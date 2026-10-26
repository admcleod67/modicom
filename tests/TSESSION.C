/* Core textedit / session abort tests for ModiCom.
 *
 * Links Core plus modules with THARNESS display stubs only
 * (textedit mallocs its own buffer; harness_alloc is not used).
 * Drives textedit() with a preloaded BIOS keyboard buffer and
 * asserts return value, the changed flag, and disk side effects.
 * Ctrl-K, Find, file prompts, and help-screen content are out of
 * scope.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <dos.h>
#include <bios.h>

#include "TEST.H"
#include "editor.h"
#include "keyboard.h"
#include "video.h"


#define KB_SEG        0x0040
#define KB_HEAD       0x001A
#define KB_TAIL       0x001C
#define KB_BUFFER     0x001E
#define KB_BUFFER_END 0x003E

#define SESSION_PATH  "BUILD\\TESTS\\SE.TMP"


typedef void (*casefunc)(void);

struct testcase
{
   const char *id;
   const char *name;
   casefunc run;
};


static int saved_mode;


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


static void remove_session_file(void)
{
   unlink(SESSION_PATH);
}


static int file_exists(const char *path)
{
   return (access(path, 0) == 0);
}


/* Preload keys, call textedit, drain. Scripts must fit in 15 slots. */

static int run_textedit(const unsigned *keys, int n)
{
   int i;
   int rc;

   kb_drain();
   for (i = 0; i < n; i ++)
      ASSERT_TRUE(kb_push(keys[i]));
   rc = textedit(SESSION_PATH);
   kb_drain();
   return rc;
}


static void case_clean_esc(void)
{
   unsigned keys[1];
   int rc;

   remove_session_file();

   keys[0] = (unsigned) K_ESC;
   rc = run_textedit(keys, 1);

   ASSERT_EQ_INT(0, rc);
   ASSERT_EQ_INT(FALSE, changed);
   ASSERT_TRUE(!file_exists(SESSION_PATH));
}


static void case_dirty_abort_yes(void)
{
   unsigned keys[3];
   int rc;

   remove_session_file();

   keys[0] = 'A';
   keys[1] = (unsigned) K_ESC;
   keys[2] = 'Y';
   rc = run_textedit(keys, 3);

   ASSERT_EQ_INT(0, rc);
   ASSERT_EQ_INT(TRUE, changed);
   ASSERT_TRUE(!file_exists(SESSION_PATH));
}


static void case_dirty_abort_no_then_yes(void)
{
   unsigned keys[5];
   int rc;

   remove_session_file();

   keys[0] = 'A';
   keys[1] = (unsigned) K_ESC;
   keys[2] = 'N';
   keys[3] = (unsigned) K_ESC;
   keys[4] = 'Y';
   rc = run_textedit(keys, 5);

   ASSERT_EQ_INT(0, rc);
   ASSERT_EQ_INT(TRUE, changed);
   ASSERT_TRUE(!file_exists(SESSION_PATH));
}


static void case_dirty_save_quit(void)
{
   unsigned keys[2];
   int rc;
   FILE *fp;
   char buf[4];
   size_t n;

   remove_session_file();

   keys[0] = 'A';
   keys[1] = (unsigned) K_CTRL_END;
   rc = run_textedit(keys, 2);

   ASSERT_EQ_INT(0, rc);
   ASSERT_EQ_INT(TRUE, changed);
   ASSERT_TRUE(file_exists(SESSION_PATH));

   fp = fopen(SESSION_PATH, "rb");
   ASSERT_TRUE(fp != NULL);
   if (fp != NULL)
   {
      n = fread(buf, 1, sizeof(buf), fp);
      fclose(fp);
      ASSERT_EQ_INT(1, (int) n);
      ASSERT_EQ_INT('A', (int) (unsigned char) buf[0]);
   }

   remove_session_file();
}


static struct testcase green_cases[] =
{
   { "E1", "clean Esc leaves unchanged and no file", case_clean_esc },
   { "E2", "dirty Esc Y aborts without writing", case_dirty_abort_yes },
   { "E3", "dirty Esc N then Esc Y aborts without writing",
     case_dirty_abort_no_then_yes },
   { "E4", "dirty Ctrl-End writes one byte and quits", case_dirty_save_quit }
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

   printf("SESSION tests\n");

   saved_mode = get_video_mode();
   set_video_mode(3);

   for (i = 0; i < GREEN_COUNT; i ++)
      run_case(&green_cases[i]);

   set_video_mode(saved_mode);
   remove_session_file();

   return test_summary();
}
