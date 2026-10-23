/* Editor operation tests for ModiCom.
 *
 * Links the production EDITOR.OBJ with stubbed display routines
 * and calls individual cursor and editing functions directly
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
#include "strings.h"

#define GUARD_SIZE 16
#define GUARD_BYTE 0xA5


/* Editor globals (defined in EDITOR.C) */

extern BYTE *start, *ptr, *end;
extern int x, y, lineno, lcount, changed, insert, maxrow, toobig;


/* Editor operations under test (defined in EDITOR.C) */

void cursup(void);
void cursdown(void);
void cursleft(void);
void cursright(void);
int gotorow(int row);
void pageup(void);
void pagedown(void);
void inschar(int ch);
void newline(int insert);
void addline(void);
void backspace(void);
void delchar(void);
void del(void);
void delword(void);
void nextword(void);
void lastword(void);
void gotoeol(void);
void gotobol(void);
void tabulate(void);


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
static char doc_ls[]     = "longline\r\0ab\r\0cd";
static char doc_words[]  = "foo bar baz\r\0qux";
static char doc_spaces[] = "foo  bar";
static char doc6[]       = "a1\r\0a2\r\0a3\r\0a4\r\0a5\r\0a6";
static char doc3t[]      = "one\r\0two\r\0three\r\0";


static void fill_guards(void)
{
   int i;

   for (i = 0; i < GUARD_SIZE; i ++)
   {
      rawbuffer[i] = GUARD_BYTE;
      rawbuffer[GUARD_SIZE + MAXBYTES + i] = GUARD_BYTE;
   }
}


static void reset_state(void)
{
   fill_guards();
   memset(start, 0, MAXBYTES);

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

      if (rawbuffer[GUARD_SIZE + MAXBYTES + i] != GUARD_BYTE)
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


/* Navigation cases */

static void case_right_within(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 1, 1);

   cursright();

   check_state(1, 2, 1, 1, 3);
   check_invariants();
}


static void case_right_to_eol(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 3, 1);

   cursright();

   /* Cursor now sits on the carriage return at end of line 1 */
   check_state(3, 4, 1, 1, 3);
   check_invariants();
}


static void case_right_across_eol(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 4, 1);

   cursright();

   check_state(5, 1, 2, 2, 3);
   check_invariants();
}


static void case_left_within(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 2, 1);

   cursleft();

   check_state(0, 1, 1, 1, 3);
   check_invariants();
}


static void case_left_across_eol(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(2, 1, 2);

   cursleft();

   /* Lands one past the end of the previous line */
   check_state(3, 4, 1, 1, 3);
   check_invariants();
}


static void case_down_equal(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 2, 1);

   cursdown();

   check_state(6, 2, 2, 2, 3);
   check_invariants();
}


static void case_up_equal(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(2, 2, 2);

   cursup();

   check_state(1, 2, 1, 1, 3);
   check_invariants();
}


static void case_down_clamp(void)
{
   install_document(doc_ls, (int) sizeof(doc_ls) - 1, 3);
   place_cursor(1, 6, 1);

   cursdown();

   /* Column clamps to one past the end of the shorter line */
   check_state(12, 3, 2, 2, 3);
   check_invariants();
}


static void case_up_from_clamp(void)
{
   install_document(doc_ls, (int) sizeof(doc_ls) - 1, 3);
   place_cursor(2, 3, 2);

   cursup();

   check_state(2, 3, 1, 1, 3);
   check_invariants();
}


static void case_gotobol(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(2, 3, 2);

   gotobol();

   check_state(5, 1, 2, 2, 3);
   check_invariants();
}


static void case_gotoeol(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(2, 1, 2);

   gotoeol();

   check_state(8, 4, 2, 2, 3);
   check_invariants();
}


static void case_gotorow_two(void)
{
   int result;

   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 1, 1);

   result = gotorow(2);

   ASSERT_EQ_INT(0, result);
   check_state(5, 1, 1, 2, 3);
   check_invariants();
}


static void case_gotorow_three(void)
{
   int result;

   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 1, 1);

   result = gotorow(3);

   ASSERT_EQ_INT(0, result);
   check_state(10, 1, 1, 3, 3);
   check_invariants();
}


static void case_nextword_within(void)
{
   install_document(doc_words, (int) sizeof(doc_words) - 1, 2);
   place_cursor(1, 1, 1);

   nextword();

   check_state(4, 5, 1, 1, 2);
   check_invariants();
}


static void case_nextword_spaces(void)
{
   install_document(doc_spaces, (int) sizeof(doc_spaces) - 1, 1);
   place_cursor(1, 1, 1);

   nextword();

   check_state(5, 6, 1, 1, 1);
   check_invariants();
}


static void case_nextword_across_eol(void)
{
   install_document(doc_words, (int) sizeof(doc_words) - 1, 2);
   place_cursor(1, 9, 1);

   /* First call reaches the line separator; second crosses it */
   nextword();
   nextword();

   check_state(13, 1, 2, 2, 2);
   check_invariants();
}


static void case_lastword_within(void)
{
   install_document(doc_words, (int) sizeof(doc_words) - 1, 2);
   place_cursor(1, 9, 1);

   lastword();

   check_state(4, 5, 1, 1, 2);
   check_invariants();
}


static void case_lastword_across_eol(void)
{
   install_document(doc_words, (int) sizeof(doc_words) - 1, 2);
   place_cursor(2, 1, 2);

   lastword();

   check_state(8, 9, 1, 1, 2);
   check_invariants();
}


static void case_navigation_noops(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);

   place_cursor(1, 1, 1);
   cursup();
   check_state(0, 1, 1, 1, 3);

   place_cursor(1, 1, 1);
   cursleft();
   check_state(0, 1, 1, 1, 3);

   place_cursor(3, 6, 3);
   cursdown();
   check_state(15, 6, 3, 3, 3);

   place_cursor(3, 6, 3);
   cursright();
   check_state(15, 6, 3, 3, 3);

   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
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


/* Paging cases: six short lines, three rows per page */

static void case_pagedown_first(void)
{
   install_document(doc6, (int) sizeof(doc6) - 1, 6);
   maxrow = 3;
   place_cursor(1, 1, 1);

   pagedown();

   check_state(12, 1, 1, 4, 6);
   check_invariants();
}


static void case_pagedown_last(void)
{
   install_document(doc6, (int) sizeof(doc6) - 1, 6);
   maxrow = 3;
   place_cursor(4, 1, 1);

   pagedown();

   check_state(20, 1, 1, 6, 6);
   check_invariants();
}


static void case_pageup(void)
{
   install_document(doc6, (int) sizeof(doc6) - 1, 6);
   maxrow = 3;
   place_cursor(6, 1, 1);

   pageup();

   check_state(8, 1, 1, 3, 6);
   check_invariants();
}


static void case_pageup_top(void)
{
   install_document(doc6, (int) sizeof(doc6) - 1, 6);
   maxrow = 3;
   place_cursor(3, 1, 1);

   pageup();

   check_state(0, 1, 1, 1, 6);
   check_invariants();
}


/* Boundary probes: run individually by case identifier */

static void probe_gotobol_first_line(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 3, 1);

   gotobol();

   check_state(0, 1, 1, 1, 3);
   check_invariants();
}


static void probe_lastword_at_start(void)
{
   install_document(doc3, (int) sizeof(doc3) - 1, 3);
   place_cursor(1, 1, 1);

   lastword();

   check_state(0, 1, 1, 1, 3);
   ASSERT_EQ_INT(FALSE, changed);
   check_invariants();
}


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


static void probe_inschar_near_full(void)
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


/* Case tables and driver */

static struct testcase green_cases[] =
{
   { "N1",  "cursor right within line", case_right_within },
   { "N2",  "cursor right to end of line", case_right_to_eol },
   { "N3",  "cursor right across end of line", case_right_across_eol },
   { "N4",  "cursor left within line", case_left_within },
   { "N5",  "cursor left across end of line", case_left_across_eol },
   { "N6",  "cursor down, equal lines", case_down_equal },
   { "N7",  "cursor up, equal lines", case_up_equal },
   { "N8",  "cursor down onto shorter line", case_down_clamp },
   { "N9",  "cursor up from clamped column", case_up_from_clamp },
   { "N10", "beginning of line", case_gotobol },
   { "N11", "end of line", case_gotoeol },
   { "N12", "go to row 2", case_gotorow_two },
   { "N13", "go to row 3", case_gotorow_three },
   { "N14", "next word within line", case_nextword_within },
   { "N15", "next word across spaces", case_nextword_spaces },
   { "N16", "next word across end of line", case_nextword_across_eol },
   { "N17", "last word within line", case_lastword_within },
   { "N18", "last word across end of line", case_lastword_across_eol },
   { "N19", "cursor movement no-ops at limits", case_navigation_noops },
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
   { "V1",  "page down, first page", case_pagedown_first },
   { "V2",  "page down to last line", case_pagedown_last },
   { "V3",  "page up", case_pageup },
   { "V4",  "page up to first line", case_pageup_top }
};

static struct testcase probe_cases[] =
{
   { "X1", "beginning of line on first line", probe_gotobol_first_line },
   { "X2", "last word at start of buffer", probe_lastword_at_start },
   { "X3", "delete word at final line ending", probe_delword_final_eol },
   { "X5", "insert with buffer nearly full", probe_inschar_near_full }
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

   printf("EDITOR OPERATION tests\n");

   rawbuffer = (BYTE *) malloc((size_t) MAXBYTES +
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
