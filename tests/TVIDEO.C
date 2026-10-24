/* Video display tests for ModiCom.
 *
 * Links the production VIDEO.OBJ and asserts the character and
 * attribute cells that showline and showpage write into colour
 * text-mode video RAM (BIOS mode 3, segment 0B800h). The screen
 * region is filled with a sentinel cell pattern before each case
 * and copied to a snapshot buffer immediately after the call
 * under test, before any assertion can print to the same screen.
 *
 * All known boundary cases for this suite are now green; the
 * executable runs the full green suite with no probe argument.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#include "TEST.H"
#include "video.h"

#define VIDEO_SEG   0xB800
#define PAGE_BYTES  4000
#define SNAP_BYTES  8192
#define ROWS        25
#define COLS        80
#define SENT_CHAR   '?'
#define SENT_ATTR   0x1F
#define WRITE_ATTR  0x07


/* Screen fixture */

static unsigned char snapshot[SNAP_BYTES];
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


static void set_cursor(int row, int col)
{
   union REGS regs;

   regs.h.ah = 0x02;
   regs.h.bh = 0;
   regs.h.dh = (unsigned char) row;
   regs.h.dl = (unsigned char) col;
   bios_video(&regs);
}


static void screen_fill_sentinel(void)
{
   unsigned char far *video;
   int i;

   video = (unsigned char far *) MK_FP(VIDEO_SEG, 0);
   for (i = 0; i < SNAP_BYTES / 2; i ++)
   {
      video[i * 2] = SENT_CHAR;
      video[i * 2 + 1] = SENT_ATTR;
   }
}


static void snapshot_screen(void)
{
   movedata(VIDEO_SEG, 0, _DS, (unsigned) snapshot, SNAP_BYTES);
}


/* Snapshot accessors and aggregated assertions */

static int cell_char(int row, int col)
{
   return snapshot[(row * COLS + col) * 2];
}


static int cell_attr(int row, int col)
{
   return snapshot[(row * COLS + col) * 2 + 1];
}


static void check_cell(int row, int col, int ch, int attr)
{
   ASSERT_EQ_INT(ch, cell_char(row, col));
   ASSERT_EQ_INT(attr, cell_attr(row, col));
}


/* Build an expected row: all sentinel cells. */

static void row_sentinel(char *expected)
{
   memset(expected, SENT_CHAR, COLS);
   expected[COLS] = '\0';
}


static void row_fill(char *expected, int col, int count, int ch)
{
   int i;

   for (i = 0; i < count; i ++)
      expected[col + i] = (char) ch;
}


static void row_text(char *expected, int col, const char *text)
{
   while (*text)
      expected[col ++] = *text ++;
}


/* A row matches when every character matches and each cell's
 * attribute is the sentinel attribute for expected sentinel cells
 * and the write attribute everywhere else. */

static void check_row(int row, const char *expected)
{
   int col, badchar, badattr, wantattr;

   badchar = 0;
   badattr = 0;
   for (col = 0; col < COLS; col ++)
   {
      if (cell_char(row, col) != (unsigned char) expected[col])
	 badchar ++;
      wantattr = (expected[col] == SENT_CHAR) ? SENT_ATTR : WRITE_ATTR;
      if (cell_attr(row, col) != wantattr)
	 badattr ++;
   }
   ASSERT_EQ_INT(0, badchar);
   ASSERT_EQ_INT(0, badattr);
}


static void check_rows_sentinel(int first, int last)
{
   char expected[COLS + 1];
   int row;

   row_sentinel(expected);
   for (row = first; row <= last; row ++)
      check_row(row, expected);
}


/* Bytes at and beyond the visible page must remain sentinel. */

static void check_beyond_page(void)
{
   int i, bad;

   bad = 0;
   for (i = PAGE_BYTES; i < SNAP_BYTES; i += 2)
   {
      if (snapshot[i] != SENT_CHAR)
	 bad ++;
      if (snapshot[i + 1] != SENT_ATTR)
	 bad ++;
   }
   ASSERT_EQ_INT(0, bad);
}


/* Case table */

typedef void (*casefunc)(void);

struct testcase
{
   const char *id;
   const char *name;
   casefunc run;
};


/* Shared documents, in internal loaded format */

static char doc_mid[]   = "abc\r\0xyz";
static char doc_last[]  = "abc";
static char doc_empty[] = "\r\0xyz";
static char doc_cr[]    = "a\rb\r\0xyz";
static char doc_soft[]  = "ab\x8d\x0a" "cd\r\0xyz";
static char doc3v[]     = "one\r\0two\r\0three";
static char doc_softp[] = "ab\x8d\x0a" "cd\r\0ef";
static char doc_one[]   = "one\r\0";
static char doc_two[]   = "one\r\0two";


/* Expected row for a rendered line: written cells are spaces
 * except for the text and the final marker cell. */

static void row_rendered(char *expected, int startcol, const char *text,
   int finalch)
{
   row_sentinel(expected);
   row_fill(expected, startcol, COLS - startcol, ' ');
   row_text(expected, startcol, text);
   expected[COLS - 1] = (char) finalch;
}


static void row_cleared(char *expected)
{
   row_sentinel(expected);
   row_fill(expected, 0, COLS, ' ');
}


/* showline cases */

static void case_showline_mid(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(2, 0);
   showline(doc_mid, doc_mid + 7);
   snapshot_screen();

   row_rendered(expected, 0, "abc", '<');
   check_row(2, expected);
   check_rows_sentinel(0, 1);
   check_rows_sentinel(3, 24);
}


static void case_showline_last(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(2, 0);
   showline(doc_last, doc_last + 3);
   snapshot_screen();

   row_rendered(expected, 0, "abc", ' ');
   check_row(2, expected);
   check_rows_sentinel(0, 1);
   check_rows_sentinel(3, 24);
}


static void case_showline_empty(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(1, 0);
   showline(doc_empty, doc_empty + 4);
   snapshot_screen();

   row_rendered(expected, 0, "", '<');
   check_row(1, expected);
   check_rows_sentinel(0, 0);
   check_rows_sentinel(2, 24);
}


static void case_showline_cr(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(1, 0);
   showline(doc_cr, doc_cr + 7);
   snapshot_screen();

   row_rendered(expected, 0, "ab", '<');
   check_row(1, expected);
}


static void case_showline_soft(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(1, 0);
   showline(doc_soft, doc_soft + 10);
   snapshot_screen();

   row_rendered(expected, 0, "ab", ' ');
   check_row(1, expected);
   check_rows_sentinel(0, 0);
   check_rows_sentinel(2, 24);
}


static void make_long_doc(char *doc, int count, const char *tail)
{
   int i;

   for (i = 0; i < count; i ++)
      doc[i] = (char) ('A' + (i % 26));
   doc[count] = '\0';
   strcpy(doc + count + 1, tail);
}


static void case_showline_long(void)
{
   static char doc[128];
   char expected[COLS + 1];
   int i;

   make_long_doc(doc, 90, "z");
   screen_fill_sentinel();
   set_cursor(4, 0);
   showline(doc, doc + 91);
   snapshot_screen();

   row_sentinel(expected);
   for (i = 0; i < 79; i ++)
      expected[i] = (char) ('A' + (i % 26));
   expected[79] = '+';
   check_row(4, expected);
   check_rows_sentinel(0, 3);
   check_rows_sentinel(5, 24);
}


static void case_showline_offset(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(3, 10);
   showline(doc_mid, doc_mid + 7);
   snapshot_screen();

   row_rendered(expected, 10, "abc", '<');
   check_row(3, expected);
   check_rows_sentinel(0, 2);
   check_rows_sentinel(4, 24);
}


static void case_showline_lastrow(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(24, 0);
   showline(doc_mid, doc_mid + 7);
   snapshot_screen();

   row_rendered(expected, 0, "abc", '<');
   check_row(24, expected);
   check_rows_sentinel(0, 23);
   check_beyond_page();
}


/* showpage cases */

static void check_page3(int startrow, int startcol)
{
   char expected[COLS + 1];
   int row;

   row_rendered(expected, startcol, "one", '<');
   check_row(startrow, expected);
   row_rendered(expected, 0, "two", '<');
   check_row(startrow + 1, expected);
   row_rendered(expected, 0, "three", ' ');
   check_row(startrow + 2, expected);
   row_cleared(expected);
   for (row = startrow + 3; row < ROWS; row ++)
      check_row(row, expected);
}


static void case_showpage_three(void)
{
   screen_fill_sentinel();
   set_cursor(0, 0);
   showpage(doc3v, doc3v + 15);
   snapshot_screen();

   check_page3(0, 0);
}


static void case_showpage_mid(void)
{
   screen_fill_sentinel();
   set_cursor(10, 0);
   showpage(doc3v, doc3v + 15);
   snapshot_screen();

   check_rows_sentinel(0, 9);
   check_page3(10, 0);
}


static void case_showpage_offset(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(5, 10);
   showpage(doc3v, doc3v + 15);
   snapshot_screen();

   check_rows_sentinel(0, 4);
   check_page3(5, 10);
   (void) expected;
}


static void case_showpage_soft(void)
{
   char expected[COLS + 1];
   int row;

   screen_fill_sentinel();
   set_cursor(0, 0);
   showpage(doc_softp, doc_softp + 10);
   snapshot_screen();

   row_rendered(expected, 0, "ab", ' ');
   check_row(0, expected);
   row_rendered(expected, 0, "cd", '<');
   check_row(1, expected);
   row_rendered(expected, 0, "ef", ' ');
   check_row(2, expected);
   row_cleared(expected);
   for (row = 3; row < ROWS; row ++)
      check_row(row, expected);
}


static void case_showpage_long(void)
{
   static char doc[160];
   char expected[COLS + 1];
   int i, row;

   make_long_doc(doc, 90, "\r");
   /* Replace the tail marker: long line, then a short final line. */
   doc[90] = '\r';
   doc[91] = '\0';
   strcpy(doc + 92, "next");

   screen_fill_sentinel();
   set_cursor(0, 0);
   showpage(doc, doc + 96);
   snapshot_screen();

   row_sentinel(expected);
   for (i = 0; i < 79; i ++)
      expected[i] = (char) ('A' + (i % 26));
   expected[79] = '+';
   check_row(0, expected);
   row_rendered(expected, 0, "next", ' ');
   check_row(1, expected);
   row_cleared(expected);
   for (row = 2; row < ROWS; row ++)
      check_row(row, expected);
}


static void case_showpage_empty(void)
{
   static char doc[] = "";
   char expected[COLS + 1];
   int row;

   screen_fill_sentinel();
   set_cursor(0, 0);
   showpage(doc, doc);
   snapshot_screen();

   row_cleared(expected);
   for (row = 0; row < ROWS; row ++)
      check_row(row, expected);
}


static void case_showpage_clear_stops(void)
{
   char expected[COLS + 1];
   int row;

   screen_fill_sentinel();
   set_cursor(0, 0);
   showpage(doc_one, doc_one + 5);
   snapshot_screen();

   row_rendered(expected, 0, "one", '<');
   check_row(0, expected);
   row_cleared(expected);
   for (row = 1; row < ROWS; row ++)
      check_row(row, expected);
   check_beyond_page();
}


static void case_showpage_more_lines(void)
{
   static char doc[256];
   char expected[COLS + 1];
   int i, pos, row;

   pos = 0;
   for (i = 0; i < 30; i ++)
   {
      doc[pos ++] = 'x';
      doc[pos ++] = '\r';
      doc[pos ++] = '\0';
   }
   doc[pos] = '\0';

   screen_fill_sentinel();
   set_cursor(0, 0);
   showpage(doc, doc + pos);
   snapshot_screen();

   row_rendered(expected, 0, "x", '<');
   for (row = 0; row < ROWS; row ++)
      check_row(row, expected);
   check_beyond_page();
}


static void case_showpage_last_row(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(24, 0);
   showpage(doc_two, doc_two + 8);
   snapshot_screen();

   check_rows_sentinel(0, 23);
   row_rendered(expected, 0, "one", '<');
   check_row(24, expected);
   check_beyond_page();
}


static void case_showline_col79(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(5, 79);
   showline(doc_mid, doc_mid + 7);
   snapshot_screen();

   /* Zero budget: only the marker cell at column 79 is written. */
   row_sentinel(expected);
   expected[79] = '+';
   check_row(5, expected);
   check_rows_sentinel(6, 24);
   check_beyond_page();
}


static void case_showline_exact_fit(void)
{
   static char doc[128];
   char expected[COLS + 1];
   int i;

   make_long_doc(doc, 79, "z");
   screen_fill_sentinel();
   set_cursor(0, 0);
   showline(doc, doc + 80);
   snapshot_screen();

   row_sentinel(expected);
   for (i = 0; i < 79; i ++)
      expected[i] = (char) ('A' + (i % 26));
   expected[79] = '<';
   check_row(0, expected);
   check_rows_sentinel(1, 24);
   check_beyond_page();
}


static void case_showline_col79_empty(void)
{
   char expected[COLS + 1];

   screen_fill_sentinel();
   set_cursor(5, 79);
   showline(doc_empty, doc_empty + 4);
   snapshot_screen();

   row_sentinel(expected);
   expected[79] = '<';
   check_row(5, expected);
   check_rows_sentinel(6, 24);
   check_beyond_page();
}


static struct testcase green_cases[] =
{
   { "S1", "showline middle of buffer", case_showline_mid },
   { "S2", "showline last line of buffer", case_showline_last },
   { "S3", "showline empty line", case_showline_empty },
   { "S4", "showline carriage return skipped", case_showline_cr },
   { "S5", "showline soft carriage return", case_showline_soft },
   { "S6", "showline overlong line", case_showline_long },
   { "S7", "showline at column offset", case_showline_offset },
   { "S8", "showline on last row", case_showline_lastrow },
   { "S9", "showline starting in column 79", case_showline_col79 },
   { "S10", "showline exactly-fitting line marker", case_showline_exact_fit },
   { "S11", "showline empty line at column 79", case_showline_col79_empty },
   { "P1", "showpage three lines", case_showpage_three },
   { "P2", "showpage from middle row", case_showpage_mid },
   { "P3", "showpage at column offset", case_showpage_offset },
   { "P4", "showpage soft carriage return", case_showpage_soft },
   { "P5", "showpage with overlong line", case_showpage_long },
   { "P6", "showpage empty buffer", case_showpage_empty },
   { "P7", "showpage clear stops at page end", case_showpage_clear_stops },
   { "P8", "showpage more lines than rows", case_showpage_more_lines },
   { "P9", "showpage starting on last row", case_showpage_last_row }
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
   int result;

   printf("VIDEO DISPLAY tests\n");

   saved_mode = get_video_mode();
   set_video_mode(3);

   for (i = 0; i < GREEN_COUNT; i ++)
      run_case(&green_cases[i]);

   result = test_summary();
   set_video_mode(saved_mode);
   return result;
}
