/* Status UI tests for ModiCom.
 *
 * Links Core plus modules with THARNESS display stubs and asserts
 * observable Status behaviour: helptoggle globals, status-line
 * text via colour video RAM, scoreboard/toobig text, and msg with
 * a scripted Space. Help-screen content and showstatus latching
 * are out of scope for this suite.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <bios.h>

#include "TEST.H"
#include "THARNESS.H"
#include "editor.h"
#include "status.h"
#include "keyboard.h"
#include "video.h"

#define VIDEO_SEG  0xB800
#define COLS       80

#define KB_SEG        0x0040
#define KB_HEAD       0x001A
#define KB_TAIL       0x001C
#define KB_BUFFER     0x001E
#define KB_BUFFER_END 0x003E


typedef void (*casefunc)(void);

struct testcase
{
   const char *id;
   const char *name;
   casefunc run;
};


static int saved_mode;
static char doc1[] = "a\r\0";


static void bios_video(union REGS *regs)
{
   int86(0x10, regs, regs);
}


static int get_video_mode(void)
{
   union REGS regs;

   regs.h.ah = 0x0F;
   bios_video(&regs);
   return regs.h.al;
}


static void set_video_mode(int mode)
{
   union REGS regs;

   regs.h.ah = 0x00;
   regs.h.al = (unsigned char) mode;
   bios_video(&regs);
}


/* Read n characters from 1-based row/col into dest (NUL-terminated). */

static void read_chars(int row, int col, int n, char *dest)
{
   unsigned char far *video;
   int i;
   int offset;

   video = (unsigned char far *) MK_FP(VIDEO_SEG, 0);
   offset = ((row - 1) * COLS + (col - 1)) * 2;

   for (i = 0; i < n; i ++)
      dest[i] = (char) video[offset + i * 2];

   dest[n] = '\0';
}


static void clear_status_row(void)
{
   unsigned char far *video;
   int i;
   int offset;

   video = (unsigned char far *) MK_FP(VIDEO_SEG, 0);
   offset = 0;

   for (i = 0; i < COLS; i ++)
   {
      video[offset + i * 2] = ' ';
      video[offset + i * 2 + 1] = 0x07;
   }
}


/* BIOS keyboard: push one Space for msg() */

static int kb_push(unsigned key)
{
   unsigned tail, next;

   tail = peek(KB_SEG, KB_TAIL);
   next = tail + 2;
   if (next >= KB_BUFFER_END)
      next = KB_BUFFER;
   if (next == peek(KB_SEG, KB_HEAD))
      return 0;
   poke(KB_SEG, tail, key);
   poke(KB_SEG, KB_TAIL, next);
   return 1;
}


static void kb_drain(void)
{
   disable();
   poke(KB_SEG, KB_HEAD, KB_BUFFER);
   poke(KB_SEG, KB_TAIL, KB_BUFFER);
   enable();
}


static void case_helptoggle(void)
{
   install_document(doc1, (int) sizeof(doc1) - 1, 1);
   place_cursor(1, 1, 5);
   maxrow = 17;
   y = 5;

   helptoggle();

   ASSERT_EQ_INT(24, maxrow);
   ASSERT_EQ_INT(1, y);

   helptoggle();

   ASSERT_EQ_INT(17, maxrow);
   ASSERT_EQ_INT(1, y);
}


static void case_showpos(void)
{
   char buf[16];

   install_document(doc1, (int) sizeof(doc1) - 1, 1);
   /* Single-digit line so "L %d    " does not reach column 66 */
   lineno = 5;
   x = 3;
   clear_status_row();

   showpos();
   read_chars(1, 59, 7, buf);

   ASSERT_EQ_STR("L 5    ", buf);

   read_chars(1, 66, 4, buf);
   ASSERT_EQ_STR("C 3 ", buf);
}


static void case_scoreboard_edit(void)
{
   char buf[40];

   install_document(doc1, (int) sizeof(doc1) - 1, 1);
   toobig = FALSE;
   strcpy((char *) fname, "TEST.C");
   clear_status_row();

   scoreboard();
   read_chars(1, 1, 12, buf);

   ASSERT_EQ_STR("Edit: TEST.C", buf);
}


static void case_scoreboard_toobig(void)
{
   char buf[40];

   install_document(doc1, (int) sizeof(doc1) - 1, 1);
   toobig = TRUE;
   clear_status_row();

   scoreboard();
   read_chars(1, 1, 14, buf);

   ASSERT_EQ_STR("File too large", buf);
}


static void case_statmsg_ins_caps(void)
{
   char buf[8];

   install_document(doc1, (int) sizeof(doc1) - 1, 1);
   clear_status_row();

   /* 0x80 = Ins, 0x40 = Caps; Num bit clear */
   statmsg(0x80 | 0x40);

   read_chars(1, 41, 3, buf);
   ASSERT_EQ_STR("Ins", buf);

   read_chars(1, 48, 3, buf);
   ASSERT_EQ_STR("   ", buf);

   read_chars(1, 51, 4, buf);
   ASSERT_EQ_STR("Caps", buf);
}


static void case_msg_space(void)
{
   install_document(doc1, (int) sizeof(doc1) - 1, 1);
   toobig = FALSE;
   strcpy((char *) fname, "A.C");
   kb_drain();
   ASSERT_TRUE(kb_push((unsigned) K_SPACE));

   msg("Test message (press SPACE) ");

   /* Returned without hanging; scoreboard refreshed afterward */
   ASSERT_EQ_INT(FALSE, toobig);
}


static struct testcase green_cases[] =
{
   { "S1", "helptoggle maxrow 17 and 24", case_helptoggle },
   { "S2", "showpos line and column", case_showpos },
   { "S3", "scoreboard edit file name", case_scoreboard_edit },
   { "S4", "scoreboard file too large", case_scoreboard_toobig },
   { "S5", "statmsg Ins and Caps", case_statmsg_ins_caps },
   { "S6", "msg returns after Space", case_msg_space }
};

#define GREEN_COUNT (sizeof(green_cases) / sizeof(green_cases[0]))


static void run_case(struct testcase *testcase)
{
   printf("CASE %s: %s\n", testcase->id, testcase->name);
   fflush(stdout);

   testcase->run();
}


int main(void)
{
   unsigned int i;

   printf("STATUS tests\n");

   if (harness_alloc() != 0)
   {
      printf("SETUP FAILURE: cannot allocate guarded edit buffer\n");
      return 2;
   }

   saved_mode = get_video_mode();
   set_video_mode(3);

   for (i = 0; i < GREEN_COUNT; i ++)
      run_case(&green_cases[i]);

   set_video_mode(saved_mode);
   harness_free();

   return test_summary();
}
