/* Shared edit-buffer harness for ModiCom buffer suites. */

#include <stdlib.h>
#include <string.h>

#define THARNESS_C
#include "THARNESS.H"
#include "video.h"

BYTE *harness_raw;


/* Display stubs: VIDEO.OBJ is deliberately not linked by buffer suites */

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


int harness_alloc(void)
{
   harness_raw = (BYTE *) malloc((size_t) MAXBYTES + 1 +
      (size_t) (2 * GUARD_SIZE));

   if (harness_raw == NULL)
      return 2;

   start = harness_raw + GUARD_SIZE;
   return 0;
}


void harness_free(void)
{
   free(harness_raw);
   harness_raw = NULL;
   start = NULL;
}


void fill_guards(void)
{
   int i;

   for (i = 0; i < GUARD_SIZE; i ++)
   {
      harness_raw[i] = GUARD_BYTE;
      harness_raw[GUARD_SIZE + MAXBYTES + 1 + i] = GUARD_BYTE;
   }
}


void reset_state(void)
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


void install_document(const char *doc, int len, int lines)
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


void place_cursor(int line, int col, int row)
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
