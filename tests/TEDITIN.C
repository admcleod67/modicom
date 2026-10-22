/* Editor input tests for ModiCom.
 *
 * Links the production EDITOR.OBJ and KEYBOARD.OBJ and feeds
 * scripted keystrokes through the real BIOS keyboard buffer into
 * getline (KEYBOARD.C) and prompt (EDITOR.C). The test cannot stub
 * getkey: KEYBOARD.OBJ already defines it, so a second definition
 * would fail the link.
 *
 * With no command-line argument, runs the safe (green) cases. With
 * a case identifier argument, runs only that boundary probe, so a
 * hang or memory corruption in one probe cannot hide the results
 * of the others.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <dir.h>

#include "TEST.H"
#include "editor.h"
#include "keyboard.h"
#include "strings.h"

#define GUARD_SIZE 16
#define GUARD_BYTE 0xA5
#define DEST_MAX   128

/* BIOS keyboard buffer (data area 0040:xxxx) */

#define KB_SEG        0x0040
#define KB_HEAD       0x001A
#define KB_TAIL       0x001C
#define KB_BUFFER     0x001E
#define KB_BUFFER_END 0x003E
#define KB_CAPACITY   15

#define KEY_RETURN ((unsigned) K_RETURN)
#define KEY_ESC    ((unsigned) K_ESC)
#define KEY_BS     ((unsigned) K_BS)


/* Functions under test (defined in EDITOR.C / KEYBOARD.C) */

int prompt(char *msg, char *str);
/* getline and getkey come from keyboard.h */


/* Display stubs: VIDEO.OBJ is deliberately not linked */

void showline(char *str, char *endptr)
{
   (void) str;
   (void) endptr;
}


void showpage(char *str, char *endptr)
{
   (void) str;
   (void) endptr;
}


/* Guard-banded destination buffer */

static char dest_raw[GUARD_SIZE + DEST_MAX + GUARD_SIZE];
static char *dest = dest_raw + GUARD_SIZE;
static int dest_size;


static void dest_prepare(int size)
{
   int i;

   dest_size = size;
   for (i = 0; i < GUARD_SIZE; i ++)
   {
      dest_raw[i] = (char) GUARD_BYTE;
      dest_raw[GUARD_SIZE + size + i] = (char) GUARD_BYTE;
   }
   memset(dest, 0xEE, size);
}


static void dest_check_guards(void)
{
   int i, bad;

   bad = 0;
   for (i = 0; i < GUARD_SIZE; i ++)
   {
      if ((unsigned char) dest_raw[i] != GUARD_BYTE)
	 bad ++;
      if ((unsigned char) dest_raw[GUARD_SIZE + dest_size + i] != GUARD_BYTE)
	 bad ++;
   }
   ASSERT_EQ_INT(0, bad);
}


/* Assert dest holds exactly expected, NUL-terminated within dest_size. */

static void dest_check_string(const char *expected)
{
   int len;

   len = (int) strlen(expected);
   ASSERT_TRUE(len < dest_size);
   if (len < dest_size)
   {
      ASSERT_EQ_INT(0, dest[len]);
      if (len) /* Borland memcmp with a zero length returns non-zero */
	 ASSERT_TRUE(memcmp(dest, expected, len) == 0);
   }
   dest_check_guards();
}


/* BIOS keyboard scripting */

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


/* Timer-tick feeder for scripts longer than the BIOS buffer */

static const unsigned *feed_script;
static int feed_count;
static volatile int feed_index;
static volatile int feed_timed_out;
static volatile unsigned feed_idle;
static void interrupt (*old_tick)(void);


static void interrupt feeder_tick(void)
{
   int before;

   before = feed_index;
   while ((feed_index < feed_count) && kb_push(feed_script[feed_index]))
      feed_index ++;

   if (feed_index != before)
      feed_idle = 0;
   else if (feed_index < feed_count)
   {
      feed_idle ++;
      if (feed_idle > 364) /* about 20 seconds at 18.2 ticks/sec */
      {
	 kb_push(KEY_ESC);
	 feed_timed_out = 1;
	 feed_index = feed_count;
      }
   }
}


static int feeder_active(void)
{
   return old_tick != NULL;
}


static void script_begin(const unsigned *script, int count)
{
   int preload;

   kb_drain();
   feed_script = script;
   feed_count = count;
   feed_index = 0;
   feed_timed_out = 0;
   feed_idle = 0;

   preload = count < KB_CAPACITY ? count : KB_CAPACITY;
   disable();
   while (feed_index < preload)
   {
      if (!kb_push(script[feed_index]))
	 break;
      feed_index ++;
   }
   enable();

   if (feed_index < count)
   {
      old_tick = getvect(0x1C);
      setvect(0x1C, feeder_tick);
   }
}


static void script_end(void)
{
   if (feeder_active())
   {
      setvect(0x1C, old_tick);
      old_tick = NULL;
   }
   feed_script = NULL;
   feed_count = 0;
   feed_index = 0;
   kb_drain();
}


/* Build a key script from text plus one terminator key. */

static int script_from_text(unsigned *script, const char *text,
   unsigned terminator)
{
   int count;

   count = 0;
   while (*text)
      script[count ++] = (unsigned char) *text ++;
   script[count ++] = terminator;
   return count;
}


/* Case table */

typedef void (*casefunc)(void);

struct testcase
{
   const char *id;
   const char *name;
   casefunc run;
};


/* Call wrappers: run one scripted call and check the feeder kept up */

static unsigned call_getline(const unsigned *script, int count, int maxlen)
{
   unsigned key;

   script_begin(script, count);
   key = getline(dest, 1, 1, maxlen);
   script_end();
   ASSERT_EQ_INT(0, feed_timed_out);
   return key;
}


static int call_prompt(const char *msg, const unsigned *script, int count)
{
   int result;

   script_begin(script, count);
   result = prompt((char *) msg, dest);
   script_end();
   ASSERT_EQ_INT(0, feed_timed_out);
   return result;
}


/* getline cases */

static void case_getline_plain(void)
{
   unsigned script[16];
   unsigned key;

   dest_prepare(32);
   key = call_getline(script,
      script_from_text(script, "hello", KEY_RETURN), 20);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("hello");
}


static void case_getline_empty(void)
{
   unsigned script[4];
   unsigned key;

   dest_prepare(16);
   key = call_getline(script,
      script_from_text(script, "", KEY_RETURN), 10);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("");
}


static void case_getline_escape_after_text(void)
{
   unsigned script[16];
   unsigned key;

   dest_prepare(32);
   key = call_getline(script,
      script_from_text(script, "ab", KEY_ESC), 20);
   ASSERT_EQ_INT(K_ESC, key);
   dest_check_string("ab");
}


static void case_getline_escape_empty(void)
{
   unsigned script[4];
   unsigned key;

   dest_prepare(16);
   key = call_getline(script,
      script_from_text(script, "", KEY_ESC), 10);
   ASSERT_EQ_INT(K_ESC, key);
   dest_check_string("");
}


static void case_getline_backspace_mid(void)
{
   unsigned script[] = { 'a', 'b', KEY_BS, 'c', KEY_RETURN };
   unsigned key;

   dest_prepare(32);
   key = call_getline(script, 5, 20);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("ac");
}


static void case_getline_backspace_empty(void)
{
   unsigned script[] = { KEY_BS, KEY_BS, 'a', KEY_RETURN };
   unsigned key;

   dest_prepare(32);
   key = call_getline(script, 4, 20);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("a");
}


static void case_getline_backspace_all(void)
{
   unsigned script[] = { 'a', 'b', KEY_BS, KEY_BS, KEY_RETURN };
   unsigned key;

   dest_prepare(32);
   key = call_getline(script, 5, 20);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("");
}


static void case_getline_spaces_punct(void)
{
   unsigned script[16];
   unsigned key;

   dest_prepare(32);
   key = call_getline(script,
      script_from_text(script, "a b! ", KEY_RETURN), 20);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("a b! ");
}


static void case_getline_control_ignored(void)
{
   unsigned script[] = { 'a', K_TAB, 0x01, 'b', KEY_RETURN };
   unsigned key;

   dest_prepare(32);
   key = call_getline(script, 5, 20);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("ab");
}


static void case_getline_exact_maxlen(void)
{
   unsigned script[16];
   unsigned key;

   dest_prepare(6);
   key = call_getline(script,
      script_from_text(script, "abcde", KEY_RETURN), 5);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("abcde");
}


static void case_getline_beyond_maxlen(void)
{
   unsigned script[16];
   unsigned key;

   dest_prepare(16);
   key = call_getline(script,
      script_from_text(script, "abcde", KEY_RETURN), 3);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("abc");
}


static void case_getline_beyond_backspace(void)
{
   unsigned script[] = { 'a', 'b', 'c', 'd', KEY_BS, 'd', KEY_RETURN };
   unsigned key;

   dest_prepare(16);
   key = call_getline(script, 7, 3);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("abd");
}


static void case_getline_maxlen_zero(void)
{
   unsigned script[8];
   unsigned key;

   dest_prepare(1);
   key = call_getline(script,
      script_from_text(script, "a", KEY_RETURN), 0);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("");
}


/* prompt cases */

static void case_prompt_find(void)
{
   unsigned script[16];
   int result;

   dest_prepare(MAXSEARCH);
   result = call_prompt("Find? ", script,
      script_from_text(script, "hello", KEY_RETURN));
   ASSERT_EQ_INT(1, result);
   dest_check_string("hello");
}


static void case_prompt_filename(void)
{
   unsigned script[16];
   int result;

   dest_prepare(MAXPATH);
   result = call_prompt("Enter file name: ", script,
      script_from_text(script, "TEST.PRG", KEY_RETURN));
   ASSERT_EQ_INT(1, result);
   dest_check_string("TEST.PRG");
}


static void case_prompt_empty(void)
{
   unsigned script[4];
   int result;

   dest_prepare(MAXSEARCH);
   result = call_prompt("Find? ", script,
      script_from_text(script, "", KEY_RETURN));
   ASSERT_EQ_INT(0, result);
   dest_check_string("");
}


static void case_prompt_escape(void)
{
   unsigned script[16];
   int result;

   dest_prepare(MAXSEARCH);
   result = call_prompt("Find? ", script,
      script_from_text(script, "ab", KEY_ESC));
   ASSERT_EQ_INT(0, result);
   dest_check_string("ab");
}


static void case_prompt_trailing_spaces(void)
{
   unsigned script[16];
   int result;

   dest_prepare(MAXSEARCH);
   result = call_prompt("Find? ", script,
      script_from_text(script, "abc   ", KEY_RETURN));
   ASSERT_EQ_INT(1, result);
   dest_check_string("abc");
}


static void case_prompt_spaces_only(void)
{
   unsigned script[16];
   int result;

   dest_prepare(MAXSEARCH);
   result = call_prompt("Find? ", script,
      script_from_text(script, "   ", KEY_RETURN));
   ASSERT_EQ_INT(0, result);
   dest_check_string("");
}


static void case_prompt_leading_spaces(void)
{
   unsigned script[16];
   int result;

   dest_prepare(MAXSEARCH);
   result = call_prompt("Find? ", script,
      script_from_text(script, "  a b ", KEY_RETURN));
   ASSERT_EQ_INT(1, result);
   dest_check_string("  a b");
}


static void case_prompt_backspace(void)
{
   unsigned script[] = { 'a', 'b', KEY_BS, 'c', KEY_RETURN };
   int result;

   dest_prepare(MAXSEARCH);
   result = call_prompt("Find? ", script, 5);
   ASSERT_EQ_INT(1, result);
   dest_check_string("ac");
}


static void case_prompt_filename_max(void)
{
   static char text[63];
   unsigned script[80];
   int i, result;

   for (i = 0; i < 62; i ++)
      text[i] = (char) ('A' + (i % 26));
   text[62] = '\0';

   dest_prepare(MAXPATH);
   result = call_prompt("Enter file name: ", script,
      script_from_text(script, text, KEY_RETURN));
   ASSERT_EQ_INT(1, result);
   dest_check_string(text);
}


/* Boundary probes */

static void probe_search_terminator(void)
{
   static char text[74];
   unsigned script[80];
   int i, result;

   for (i = 0; i < 73; i ++)
      text[i] = (char) ('a' + (i % 26));
   text[73] = '\0';

   /* Buffer exactly as in ctrlkey: static char srchstr[MAXSEARCH]. */
   dest_prepare(MAXSEARCH);
   result = call_prompt("Find? ", script,
      script_from_text(script, text, KEY_RETURN));
   ASSERT_EQ_INT(1, result);
   dest_check_string(text);
}


static void probe_getline_terminator(void)
{
   unsigned script[16];
   unsigned key;

   /* Destination of exactly maxlen bytes: no room for terminator. */
   dest_prepare(5);
   key = call_getline(script,
      script_from_text(script, "abcde", KEY_RETURN), 5);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("abcde");
}


static void probe_extended_keys(void)
{
   unsigned script[] = { 'a', K_UP, K_F1, 'b', KEY_RETURN };
   unsigned key;

   dest_prepare(32);
   key = call_getline(script, 5, 20);
   ASSERT_EQ_INT(K_RETURN, key);
   dest_check_string("ab");
}


static void probe_escape_preserves(void)
{
   unsigned script[16];
   int result;

   dest_prepare(MAXSEARCH);
   result = call_prompt("Find? ", script,
      script_from_text(script, "old", KEY_RETURN));
   ASSERT_EQ_INT(1, result);
   dest_check_string("old");

   /* A cancelled prompt should leave the stored string unchanged. */
   result = call_prompt("Find? ", script,
      script_from_text(script, "x", KEY_ESC));
   ASSERT_EQ_INT(0, result);
   dest_check_string("old");
}


static struct testcase green_cases[] =
{
   { "L1",  "getline plain input", case_getline_plain },
   { "L2",  "getline empty input", case_getline_empty },
   { "L3",  "getline Escape after text", case_getline_escape_after_text },
   { "L4",  "getline Escape on empty input", case_getline_escape_empty },
   { "L5",  "getline backspace in middle", case_getline_backspace_mid },
   { "L6",  "getline backspace at empty input", case_getline_backspace_empty },
   { "L7",  "getline backspace whole input", case_getline_backspace_all },
   { "L8",  "getline spaces and punctuation", case_getline_spaces_punct },
   { "L9",  "getline control keys ignored", case_getline_control_ignored },
   { "L10", "getline exactly maxlen", case_getline_exact_maxlen },
   { "L11", "getline beyond maxlen ignored", case_getline_beyond_maxlen },
   { "L12", "getline beyond maxlen, backspace, retype", case_getline_beyond_backspace },
   { "L13", "getline maxlen zero", case_getline_maxlen_zero },
   { "R1",  "prompt Find with text", case_prompt_find },
   { "R2",  "prompt file name with text", case_prompt_filename },
   { "R3",  "prompt empty input", case_prompt_empty },
   { "R4",  "prompt Escape after text", case_prompt_escape },
   { "R5",  "prompt trailing spaces trimmed", case_prompt_trailing_spaces },
   { "R6",  "prompt spaces only", case_prompt_spaces_only },
   { "R7",  "prompt leading spaces kept", case_prompt_leading_spaces },
   { "R8",  "prompt backspace editing", case_prompt_backspace },
   { "R9",  "prompt file name at maximum length", case_prompt_filename_max }
};

static struct testcase probe_cases[] =
{
   { "B1", "search-string terminator at maximum length", probe_search_terminator },
   { "B2", "getline terminator with maxlen-sized buffer", probe_getline_terminator },
   { "B3", "non-character keys in getline", probe_extended_keys },
   { "B4", "Escape preserves previous search string", probe_escape_preserves }
};

#define GREEN_COUNT (sizeof(green_cases) / sizeof(green_cases[0]))
#define PROBE_COUNT (sizeof(probe_cases) / sizeof(probe_cases[0]))


static void run_case(struct testcase *testcase)
{
   printf("CASE %s: %s\n", testcase->id, testcase->name);
   fflush(stdout);

   testcase->run();
}


int main(int argc, char *argv[])
{
   unsigned int i;
   struct testcase *only;

   printf("EDITOR INPUT tests\n");

   if (argc > 1)
   {
      only = NULL;

      for (i = 0; i < PROBE_COUNT; i ++)
      {
	 if (strcmp(argv[1], probe_cases[i].id) == 0)
	    only = &probe_cases[i];
      }

      if (only == NULL)
      {
	 printf("Unknown case '%s'. Available probes:", argv[1]);

	 for (i = 0; i < PROBE_COUNT; i ++)
	    printf(" %s", probe_cases[i].id);

	 printf("\n");
	 return 2;
      }

      run_case(only);
   }
   else
   {
      for (i = 0; i < GREEN_COUNT; i ++)
	 run_case(&green_cases[i]);
   }

   return test_summary();
}
