/* Editor file IO tests for Modicom.
 *
 * Links the production EDITOR.OBJ with stubbed display routines and
 * a guard-banded edit buffer. With no command-line argument, runs
 * the safe (green) cases. With a case identifier argument, runs only
 * that boundary probe, so a hang or memory corruption in one probe
 * cannot hide the results of the others.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TEST.H"
#include "editor.h"
#include "strings.h"

#define GUARD_SIZE 16
#define GUARD_BYTE 0xA5

#define IN_PATH  "BUILD\\TESTS\\IN01.TXT"
#define OUT_PATH "BUILD\\TESTS\\OUT01.TXT"


/* Editor globals and entry points under test (defined in EDITOR.C) */

extern BYTE *start, *ptr, *end;
extern int x, y, lineno, lcount, changed, insert, maxrow, toobig;

void loadfile(char *fname);
void textwrite(char *fname);


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


/* Fixture helpers */

static int write_bytes(const char *path, const void *data, unsigned int len)
{
   FILE *fp;

   fp = fopen(path, "wb");

   if (fp == NULL)
      return 0;

   if ((len > 0) && (fwrite(data, 1, len, fp) != len))
   {
      fclose(fp);
      return 0;
   }

   fclose(fp);
   return 1;
}


static int write_repeat(const char *path, long total)
{
   static char chunk[256];
   FILE *fp;
   long remaining;
   unsigned int n;

   memset(chunk, 'A', sizeof(chunk));

   fp = fopen(path, "wb");

   if (fp == NULL)
      return 0;

   remaining = total;

   while (remaining > 0)
   {
      n = (remaining > (long) sizeof(chunk)) ? sizeof(chunk)
	 : (unsigned int) remaining;

      if (fwrite(chunk, 1, n, fp) != n)
      {
	 fclose(fp);
	 return 0;
      }

      remaining -= n;
   }

   fclose(fp);
   return 1;
}


static long read_bytes(const char *path, BYTE *dest, long max)
{
   FILE *fp;
   long total;
   unsigned int n;

   fp = fopen(path, "rb");

   if (fp == NULL)
      return -1;

   total = 0;

   while (total < max)
   {
      n = fread(dest + total, 1, 128, fp);

      if (n == 0)
	 break;

      total += n;
   }

   fclose(fp);
   return total;
}


static void load_fixture(void)
{
   char path[96];

   strcpy(path, IN_PATH);
   loadfile(path);
}


/* Green cases: loading */

static void case_empty(void)
{
   ASSERT_TRUE(write_bytes(IN_PATH, "", 0));
   load_fixture();

   ASSERT_EQ_INT(0, (int) (end - start));
   ASSERT_EQ_INT(1, lcount);
   ASSERT_EQ_INT('\0', *end);
   check_invariants();

   remove(IN_PATH);
}


static void case_one_byte(void)
{
   ASSERT_TRUE(write_bytes(IN_PATH, "A", 1));
   load_fixture();

   ASSERT_EQ_INT(1, (int) (end - start));
   ASSERT_EQ_INT(1, lcount);
   ASSERT_EQ_INT('A', start[0]);
   ASSERT_EQ_INT('\0', *end);
   check_invariants();

   remove(IN_PATH);
}


static void case_crlf(void)
{
   ASSERT_TRUE(write_bytes(IN_PATH, "AB\r\nCD\r\n", 8));
   load_fixture();

   ASSERT_EQ_INT(8, (int) (end - start));
   ASSERT_EQ_INT(3, lcount);
   ASSERT_EQ_INT('\r', start[2]);
   ASSERT_EQ_INT('\0', start[3]);
   ASSERT_EQ_INT('C', start[4]);
   ASSERT_EQ_INT('\0', *end);
   check_invariants();

   remove(IN_PATH);
}


static void case_lf_only(void)
{
   ASSERT_TRUE(write_bytes(IN_PATH, "AB\nCD\n", 6));
   load_fixture();

   ASSERT_EQ_INT(6, (int) (end - start));
   ASSERT_EQ_INT(3, lcount);
   ASSERT_EQ_INT('\0', start[2]);
   ASSERT_EQ_INT('C', start[3]);
   check_invariants();

   remove(IN_PATH);
}


static void case_soft_cr(void)
{
   static char data[] = { 'A', (char) 0x8d, '\n', 'B', '\n' };

   ASSERT_TRUE(write_bytes(IN_PATH, data, 5));
   load_fixture();

   ASSERT_EQ_INT(5, (int) (end - start));
   ASSERT_EQ_INT(3, lcount);
   ASSERT_EQ_INT(0x8d, start[1]);
   ASSERT_EQ_INT('\0', start[2]);
   check_invariants();

   remove(IN_PATH);
}


static void case_no_trailing_newline(void)
{
   ASSERT_TRUE(write_bytes(IN_PATH, "AB\nCD", 5));
   load_fixture();

   ASSERT_EQ_INT(5, (int) (end - start));
   ASSERT_EQ_INT(2, lcount);
   ASSERT_EQ_INT('\0', start[2]);
   ASSERT_EQ_INT('D', start[4]);
   check_invariants();

   remove(IN_PATH);
}


static void case_final_ctrlz(void)
{
   ASSERT_TRUE(write_bytes(IN_PATH, "AB\x1a", 3));
   load_fixture();

   ASSERT_EQ_INT(2, (int) (end - start));
   ASSERT_EQ_INT(1, lcount);
   ASSERT_EQ_INT('A', start[0]);
   ASSERT_EQ_INT('B', start[1]);
   ASSERT_EQ_INT('\0', *end);
   check_invariants();

   remove(IN_PATH);
}


/* Green case: saving (this tree builds VERSION_M, so no Ctrl+Z) */

static void case_save_roundtrip(void)
{
   static BYTE saved[64];
   char outpath[96];
   long savedlen;

   ASSERT_TRUE(write_bytes(IN_PATH, "AB\r\nCD\r\n", 8));
   load_fixture();

   strcpy(outpath, OUT_PATH);
   textwrite(outpath);

   savedlen = read_bytes(OUT_PATH, saved, sizeof(saved));

   ASSERT_EQ_INT(8, (int) savedlen);

   if (savedlen == 8)
      ASSERT_TRUE(memcmp(saved, "AB\r\nCD\r\n", 8) == 0);

   ASSERT_EQ_INT(8, (int) (end - start));
   check_invariants();

   remove(IN_PATH);
   remove(OUT_PATH);
}


/* Boundary probes: run individually by case identifier */

static void probe_empty_preceding_ctrlz(void)
{
   /* Place Ctrl+Z immediately before the edit buffer, then load an
      empty file. Loading should not consult or modify that byte. */
   rawbuffer[GUARD_SIZE - 1] = 0x1a;

   ASSERT_TRUE(write_bytes(IN_PATH, "", 0));
   load_fixture();

   ASSERT_TRUE(end == start);
   ASSERT_EQ_INT(1, lcount);
   ASSERT_EQ_INT(0x1a, rawbuffer[GUARD_SIZE - 1]);
   ASSERT_EQ_INT('\0', *start);

   /* The planted byte has been checked; restore the guard pattern
      so the general guard check does not report it again. */
   rawbuffer[GUARD_SIZE - 1] = GUARD_BYTE;

   check_invariants();

   remove(IN_PATH);
}


static void probe_size_32767(void)
{
   ASSERT_TRUE(write_repeat(IN_PATH, 32767L));
   load_fixture();

   ASSERT_TRUE(end == start + 32767);
   ASSERT_EQ_INT(1, lcount);
   ASSERT_EQ_INT('A', start[0]);
   ASSERT_EQ_INT('A', *(end - 1));
   check_invariants();

   remove(IN_PATH);
}


static void probe_size_32768(void)
{
   ASSERT_TRUE(write_repeat(IN_PATH, 32768L));
   load_fixture();

   ASSERT_TRUE(end == start + 32768);
   ASSERT_EQ_INT(1, lcount);
   ASSERT_EQ_INT('A', start[0]);
   ASSERT_EQ_INT('A', *(end - 1));
   check_invariants();

   remove(IN_PATH);
}


static void probe_size_32769(void)
{
   ASSERT_TRUE(write_repeat(IN_PATH, 32769L));
   load_fixture();

   ASSERT_TRUE(end == start + 32769);
   ASSERT_EQ_INT(1, lcount);
   ASSERT_EQ_INT('A', start[0]);
   ASSERT_EQ_INT('A', *(end - 1));
   check_invariants();

   remove(IN_PATH);
}


static void probe_size_maxbytes(void)
{
   ASSERT_TRUE(write_repeat(IN_PATH, (long) MAXBYTES));
   load_fixture();

   ASSERT_TRUE(end == start + MAXBYTES);
   ASSERT_TRUE(!toobig);
   ASSERT_EQ_INT('A', start[0]);
   ASSERT_EQ_INT('A', *(end - 1));
   check_invariants();

   remove(IN_PATH);
}


static void probe_size_maxbytes_plus_one(void)
{
   ASSERT_TRUE(write_repeat(IN_PATH, (long) MAXBYTES + 1L));
   load_fixture();

   ASSERT_TRUE(end == start + MAXBYTES);
   ASSERT_TRUE(toobig);
   ASSERT_EQ_INT('A', start[0]);
   ASSERT_EQ_INT('A', *(end - 1));
   check_invariants();

   remove(IN_PATH);
}


static void probe_size_50000(void)
{
   ASSERT_TRUE(write_repeat(IN_PATH, 50000L));
   load_fixture();

   ASSERT_TRUE(end == start + MAXBYTES);
   ASSERT_TRUE(toobig);
   ASSERT_EQ_INT('A', start[0]);
   ASSERT_EQ_INT('A', *(end - 1));
   check_invariants();

   remove(IN_PATH);
}


static void probe_long_line(void)
{
   ASSERT_TRUE(write_repeat(IN_PATH, 5000L));
   load_fixture();

   ASSERT_TRUE(end == start + 5000);
   ASSERT_EQ_INT(1, lcount);
   ASSERT_EQ_INT('A', start[0]);
   ASSERT_EQ_INT('A', *(end - 1));
   check_invariants();

   remove(IN_PATH);
}


/* Case tables and driver */

static struct testcase green_cases[] =
{
   { "G1", "load empty file", case_empty },
   { "G2", "load one byte, no newline", case_one_byte },
   { "G3", "load CRLF lines", case_crlf },
   { "G4", "load LF-only lines", case_lf_only },
   { "G5", "load soft carriage return line", case_soft_cr },
   { "G6", "load without trailing newline", case_no_trailing_newline },
   { "G7", "load file ending with Ctrl+Z", case_final_ctrlz },
   { "G8", "save round-trip (VERSION_M)", case_save_roundtrip }
};

static struct testcase probe_cases[] =
{
   { "P1", "empty file, Ctrl+Z before buffer", probe_empty_preceding_ctrlz },
   { "P2", "load 32767 bytes", probe_size_32767 },
   { "P3", "load 32768 bytes", probe_size_32768 },
   { "P4", "load 32769 bytes", probe_size_32769 },
   { "P5", "load exactly MAXBYTES", probe_size_maxbytes },
   { "P6", "load MAXBYTES + 1", probe_size_maxbytes_plus_one },
   { "P7", "load 50000 bytes", probe_size_50000 },
   { "P8", "load 5000-byte single line", probe_long_line }
};

#define GREEN_COUNT (sizeof(green_cases) / sizeof(green_cases[0]))
#define PROBE_COUNT (sizeof(probe_cases) / sizeof(probe_cases[0]))


static void run_case(struct testcase *testcase)
{
   printf("CASE %s: %s\n", testcase->id, testcase->name);
   fflush(stdout);

   reset_state();
   testcase->run();
}


int main(int argc, char *argv[])
{
   unsigned int i;
   struct testcase *only;

   printf("EDITOR IO tests\n");

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
