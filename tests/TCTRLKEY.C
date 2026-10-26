/* Core ctrlkey tests for ModiCom.
 *
 * Links Core plus modules with THARNESS display stubs. Drives
 * ctrlkey() with a preloaded BIOS keyboard buffer and asserts
 * buffer / cursor globals. Pins Find / Find-again dispatch and
 * the static search string; readfile/writefile and the textedit
 * session loop are out of scope.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <bios.h>

#include "TEST.H"
#include "THARNESS.H"
#include "editor.h"
#include "keyboard.h"


#define KB_SEG        0x0040
#define KB_HEAD       0x001A
#define KB_TAIL       0x001C
#define KB_BUFFER     0x001E
#define KB_BUFFER_END 0x003E


void ctrlkey(void);


typedef void (*casefunc)(void);

struct testcase
{
   const char *id;
   const char *name;
   casefunc run;
};


static int saved_mode;

/* Internal line format: text, CR, NUL between lines (THARNESS). */
static char doc_find[] = "alpha\r\0beta\r\0gamma";
static char doc_dup[]  = "one\r\0two\r\0two";
static char doc1[]     = "a\r\0";


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


/* Preload keys, call ctrlkey, drain. Scripts must fit in 15 slots. */

static void run_ctrlkey(const unsigned *keys, int n)
{
   int i;

   kb_drain();
   for (i = 0; i < n; i ++)
      ASSERT_TRUE(kb_push(keys[i]));
   ctrlkey();
   kb_drain();
}


static void case_find_again_empty(void)
{
   unsigned keys[2];
   int len;

   len = (int) sizeof(doc1) - 1;
   install_document(doc1, len, 1);
   place_cursor(1, 1, 1);
   strcpy((char *) fname, "A.C");

   keys[0] = 'L';
   keys[1] = (unsigned) K_SPACE;
   run_ctrlkey(keys, 2);

   check_buffer(doc1, len);
   check_state(0, 1, 1, 1, 1);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}


static void case_find_hit(void)
{
   unsigned keys[6];
   int len;

   len = (int) sizeof(doc_find) - 1;
   install_document(doc_find, len, 3);
   place_cursor(1, 1, 1);
   strcpy((char *) fname, "A.C");

   /* F, b, e, t, a, Return */
   keys[0] = 'F';
   keys[1] = 'b';
   keys[2] = 'e';
   keys[3] = 't';
   keys[4] = 'a';
   keys[5] = (unsigned) K_RETURN;
   run_ctrlkey(keys, 6);

   /* "alpha\r\0" is 7 bytes; "beta" starts at offset 7 */
   check_buffer(doc_find, len);
   check_state(7, 1, 2, 2, 3);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}


static void case_find_cancel(void)
{
   unsigned keys[3];
   int len;

   len = (int) sizeof(doc_find) - 1;
   install_document(doc_find, len, 3);
   place_cursor(1, 1, 1);
   strcpy((char *) fname, "A.C");

   keys[0] = 'F';
   keys[1] = 'x';
   keys[2] = (unsigned) K_ESC;
   run_ctrlkey(keys, 3);

   check_buffer(doc_find, len);
   check_state(0, 1, 1, 1, 3);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}


static void case_find_miss(void)
{
   unsigned keys[5];
   int len;

   len = (int) sizeof(doc_find) - 1;
   install_document(doc_find, len, 3);
   place_cursor(1, 1, 1);
   strcpy((char *) fname, "A.C");

   /* F, z, z, Return, Space (msg after miss) */
   keys[0] = 'F';
   keys[1] = 'z';
   keys[2] = 'z';
   keys[3] = (unsigned) K_RETURN;
   keys[4] = (unsigned) K_SPACE;
   run_ctrlkey(keys, 5);

   check_buffer(doc_find, len);
   check_state(0, 1, 1, 1, 3);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}


static void case_find_again(void)
{
   unsigned keys_f[5];
   unsigned keys_l[1];
   int len;

   len = (int) sizeof(doc_dup) - 1;
   install_document(doc_dup, len, 3);
   place_cursor(1, 1, 1);
   strcpy((char *) fname, "A.C");

   /* Seed static srchstr via Find */
   keys_f[0] = 'F';
   keys_f[1] = 't';
   keys_f[2] = 'w';
   keys_f[3] = 'o';
   keys_f[4] = (unsigned) K_RETURN;
   run_ctrlkey(keys_f, 5);

   /* First "two" at offset 5 after "one\r\0" */
   check_state(5, 1, 2, 2, 3);

   keys_l[0] = 'L';
   run_ctrlkey(keys_l, 1);

   /* Second "two" at offset 10 after "one\r\0two\r\0" */
   check_buffer(doc_dup, len);
   check_state(10, 1, 3, 3, 3);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}


static void case_find_alias_lower(void)
{
   unsigned keys[6];
   int len;

   len = (int) sizeof(doc_find) - 1;
   install_document(doc_find, len, 3);
   place_cursor(1, 1, 1);
   strcpy((char *) fname, "A.C");

   keys[0] = 'f';
   keys[1] = 'b';
   keys[2] = 'e';
   keys[3] = 't';
   keys[4] = 'a';
   keys[5] = (unsigned) K_RETURN;
   run_ctrlkey(keys, 6);

   check_buffer(doc_find, len);
   check_state(7, 1, 2, 2, 3);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}


static void case_unknown_key(void)
{
   unsigned keys[1];
   int len;

   len = (int) sizeof(doc1) - 1;
   install_document(doc1, len, 1);
   place_cursor(1, 1, 1);
   changed = FALSE;
   strcpy((char *) fname, "A.C");

   keys[0] = 'Z';
   run_ctrlkey(keys, 1);

   check_buffer(doc1, len);
   check_state(0, 1, 1, 1, 1);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}


static struct testcase green_cases[] =
{
   { "C1", "Find-again with empty search string", case_find_again_empty },
   { "C2", "Find hit moves cursor to match", case_find_hit },
   { "C3", "Find Escape leaves cursor unchanged", case_find_cancel },
   { "C4", "Find miss leaves cursor unchanged", case_find_miss },
   { "C5", "Find then Find-again hits second match", case_find_again },
   { "C6", "Find with lowercase f alias", case_find_alias_lower },
   { "C7", "Unknown ctrlkey command is a no-op", case_unknown_key }
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

   printf("CTRLKEY tests\n");

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
