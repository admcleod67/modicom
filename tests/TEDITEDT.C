/* Editor edit tests for ModiCom.
 *
 * Links Core and Edit (plus the other editor modules) with stubbed
 * display routines and calls individual editing functions directly
 * against a guard-banded edit buffer. With no command-line
 * argument, runs the safe (green) cases. With a case identifier
 * argument, runs only that boundary probe, so a hang or memory
 * corruption in one probe cannot hide the results of the others.
 *
 * Documents use the editor's internal loaded format: lines
 * separated by carriage return and NUL, as produced by loadfile
 * from a CRLF file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TEST.H"
#include "editor.h"
#include "edit.h"
#include "strings.h"
#include "video.h"

#define GUARD_SIZE 16
#define GUARD_BYTE 0xA5


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


/* Harness state */

static BYTE *rawbuffer;

typedef void (*casefunc)(void);

struct testcase
{
   const char *id;
   const char *name;
   casefunc run;
};


/* Shared documents, in internal loaded format */

static char doc3[]       = "one\r\0two\r\0three";
static char doc_words[]  = "foo bar baz\r\0qux";
static char doc3t[]      = "one\r\0two\r\0three\r\0";


static void fill_guards(void)
{
   int i;

   for (i = 0; i < GUARD_SIZE; i ++)
   {
      rawbuffer[i] = GUARD_BYTE;
      rawbuffer[GUARD_SIZE + MAXBYTES + 1 + i] = GUARD_BYTE;
   }
}


static void reset_state(void)
{
   fill_guards();
   memset(start, 0, MAXBYTES + 1);

   ptr = start;
   end = start;
   x = MIN_COL;
   y = MIN_ROW;
   lineno = 1;
   lcount = 0;
   changed = FALSE;
   insert = TRUE;
   maxrow = 17;
   toobig = FALSE;
}


static void check_guards(void)
{
   int i;
   int bad;

   bad = 0;

   for (i = 0; i < GUARD_SIZE; i ++)
   {
      if (rawbuffer[i] != GUARD_BYTE)
	 bad ++;

      if (rawbuffer[GUARD_SIZE + MAXBYTES + 1 + i] != GUARD_BYTE)
	 bad ++;
   }

   ASSERT_EQ_INT(0, bad);
}


static void check_invariants(void)
{
   ASSERT_TRUE(ptr >= start);
   ASSERT_TRUE(ptr <= end);
   ASSERT_TRUE(end <= start + MAXBYTES);
   check_guards();
}


/* Install a document and reset all editor state around it */

static void install_document(const char *doc, int len, int lines)
{
   reset_state();

   memcpy(start, doc, len);
   end = start + len;
   *end = '\0';
   lcount = lines;

   ptr = start;
   x = MIN_COL;
   y = MIN_ROW;
   lineno = 1;
   changed = FALSE;
}


/* Place the cursor on the given line and column; row is the
   screen row (y) to report */

static void place_cursor(int line, int col, int row)
{
   BYTE *p;
   int l;

   p = start;

   for (l = 1; l < line; l ++)
   {
      while (*p)
	 p ++;

      p ++;
   }

   ptr = p + (col - 1);
   x = col;
   y = row;
   lineno = line;
}


/* Assert buffer contents, length, and terminator */

static void check_buffer(const char *expected, int len)
{
   ASSERT_EQ_INT(len, (int) (end - start));
   ASSERT_TRUE(memcmp(start, expected, len) == 0);
   ASSERT_EQ_INT('\0', *end);
}


/* Assert cursor and document state together */

static void check_state(int offset, int col, int row, int line, int count)
{
   ASSERT_EQ_INT(offset, (int) (ptr - start));
   ASSERT_EQ_INT(col, x);
   ASSERT_EQ_INT(row, y);
   ASSERT_EQ_INT(line, lineno);
   ASSERT_EQ_INT(count, lcount);
}


/* Editing cases */

static void case_inschar_insert_middle(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 2, 1);
   insert = TRUE;

   inschar('X');

   check_buffer("oXne\r\0two\r\0three", 16);
   check_state(2, 3, 1, 1, 3);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_inschar_insert_eol(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 4, 1);
   insert = TRUE;

   inschar('X');

   check_buffer("oneX\r\0two\r\0three", 16);
   check_state(4, 5, 1, 1, 3);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_inschar_overwrite(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 2, 1);
   insert = FALSE;

   inschar('X');

   check_buffer("oXe\r\0two\r\0three", 15);
   check_state(2, 3, 1, 1, 3);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_inschar_control(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 2, 1);
   insert = TRUE;

   inschar(0x01);

   check_buffer(doc3, (int) sizeof(doc3) - 1);
   check_state(1, 2, 1, 1, 3);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}

static void case_newline_split(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 2, 1);

   newline(TRUE);

   check_buffer("o\r\0ne\r\0two\r\0three", 17);
   check_state(3, 1, 2, 2, 4);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_newline_at_end(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(3, 6, 3);

   newline(TRUE);

   check_buffer("one\r\0two\r\0three\r\0", 17);
   check_state(17, 1, 4, 4, 4);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_newline_noninsert(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 2, 1);

   newline(FALSE);

   /* Without insert, Enter on a non-final line just moves down */
   check_buffer(doc3, (int) sizeof(doc3) - 1);
   check_state(5, 1, 2, 2, 3);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}

static void case_addline(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 1, 1);

   addline();

   check_buffer("\r\0one\r\0two\r\0three", 17);
   check_state(0, 1, 1, 1, 4);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_delchar_within(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 2, 1);

   delchar();

   check_buffer("oe\r\0two\r\0three", 14);
   check_state(1, 2, 1, 1, 3);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_delchar_join(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 4, 1);

   delchar();

   check_buffer("onetwo\r\0three", 13);
   check_state(3, 4, 1, 1, 2);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_backspace_within(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 3, 1);

   backspace();

   check_buffer("oe\r\0two\r\0three", 14);
   check_state(1, 2, 1, 1, 3);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_backspace_join(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(2, 1, 2);

   backspace();

   check_buffer("onetwo\r\0three", 13);
   check_state(3, 4, 1, 1, 2);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_del_first_line(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 1, 1);

   del();

   check_buffer("two\r\0three", 10);
   check_state(0, 1, 1, 1, 2);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_del_middle_line(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(2, 1, 2);

   del();

   check_buffer("one\r\0three", 10);
   check_state(5, 1, 2, 2, 2);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_del_last_line(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(3, 1, 3);

   del();

   check_buffer("one\r\0two\r\0", 10);
   check_state(10, 1, 3, 3, 3);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_delword_within(void)
{
   install_document(doc_words, (int) sizeof(doc_words) - 1, 2);
   place_cursor(1, 5, 1);

   delword();

   check_buffer("foo baz\r\0qux", 12);
   check_state(4, 5, 1, 1, 2);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_tabulate_insert(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 1, 1);
   insert = TRUE;

   tabulate();

   check_buffer("    one\r\0two\r\0three", 19);
   check_state(4, 5, 1, 1, 3);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

static void case_delchar_final_end(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(3, 6, 3);

   delchar();

   /* Nothing should change at the very end of the buffer, and the
      cursor must not end up beyond end */
   ASSERT_TRUE(ptr <= end);
   check_buffer(doc3, (int) sizeof(doc3) - 1);
   check_state(15, 6, 3, 3, 3);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}

static void case_inschar_near_full(void)
{
   reset_state();

   memset(start, 'A', MAXBYTES - 1);
   end = start + MAXBYTES - 1;
   *end = '\0';
   ptr = end;
   x = MIN_COL;
   y = MIN_ROW;
   lineno = 1;
   lcount = 1;
   insert = TRUE;
   changed = FALSE;

   inschar('X');

   /* The character is stored and end advances to the limit, but
      nothing may be written into the trailing guard */
   ASSERT_TRUE(end == start + MAXBYTES);
   ASSERT_EQ_INT('X', *(end - 1));
   check_invariants();
}

/* Boundary probes: run individually by case identifier */

static void probe_delword_final_eol(void)
{
   install_document(doc3t, (int) sizeof(doc3t) - 1, 4);
   place_cursor(3, 6, 3);

   delword();

   /* Deleting the final line ending should leave three lines,
      with the cursor at the new end of the buffer */
   check_buffer("one\r\0two\r\0three", 15);
   check_state(15, 6, 3, 3, 3);
   ASSERT_EQ_INT(TRUE, changed);
   check_invariants();
}

/* Case tables and driver */

static struct testcase green_cases[] =
{
   { "E1",  "insert character, middle of line", case_inschar_insert_middle },
   { "E2",  "insert character at end of line", case_inschar_insert_eol },
   { "E3",  "overwrite character", case_inschar_overwrite },
   { "E4",  "insert control character ignored", case_inschar_control },
   { "E5",  "newline splits line", case_newline_split },
   { "E6",  "newline at end of buffer", case_newline_at_end },
   { "E7",  "newline without insert moves down", case_newline_noninsert },
   { "E8",  "add line", case_addline },
   { "E9",  "delete character within line", case_delchar_within },
   { "E10", "delete character joins next line", case_delchar_join },
   { "E11", "backspace within line", case_backspace_within },
   { "E12", "backspace joins previous line", case_backspace_join },
   { "E13", "delete first line", case_del_first_line },
   { "E14", "delete middle line", case_del_middle_line },
   { "E15", "delete last line", case_del_last_line },
   { "E16", "delete word", case_delword_within },
   { "E17", "tabulate in insert mode", case_tabulate_insert },
   { "E18", "delete character at end of buffer", case_delchar_final_end },
   { "E19", "insert with buffer nearly full", case_inschar_near_full }
};

static struct testcase probe_cases[] =
{
   { "X3", "delete word at final line ending", probe_delword_final_eol }
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

   printf("EDITOR EDIT tests\n");

   rawbuffer = (BYTE *) malloc((size_t) MAXBYTES + 1 +
      (size_t) (2 * GUARD_SIZE));

   if (rawbuffer == NULL)
   {
      printf("SETUP FAILURE: cannot allocate guarded edit buffer\n");
      return 2;
   }

   start = rawbuffer + GUARD_SIZE;

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
	 free(rawbuffer);
	 return 2;
      }

      run_case(only);
   }
   else
   {
      for (i = 0; i < GREEN_COUNT; i ++)
	 run_case(&green_cases[i]);
   }

   free(rawbuffer);

   return test_summary();
}
