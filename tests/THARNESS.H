/* Shared edit-buffer harness for ModiCom buffer suites.
 *
 * Lifecycle helpers live in THARNESS.C. Asserting checkers are
 * static in this header so ASSERT_* updates the including
 * translation unit's TEST.H counters.
 */

#ifndef THARNESS_H
#define THARNESS_H

#include "editor.h"

#define GUARD_SIZE 16
#define GUARD_BYTE 0xA5

extern BYTE *harness_raw;

int harness_alloc(void);
void harness_free(void);
void fill_guards(void);
void reset_state(void);
void install_document(const char *doc, int len, int lines);
void place_cursor(int line, int col, int row);

/* Asserting checkers are omitted when THARNESS.C includes this
   header (define THARNESS_C first) so unused statics are not
   compiled into the shared object. Suite .C files include
   TEST.H before this header and get the checkers below. */
#ifndef THARNESS_C

#include <string.h>

#include "TEST.H"

static void check_guards(void)
{
   int i;
   int bad;

   bad = 0;

   for (i = 0; i < GUARD_SIZE; i ++)
   {
      if (harness_raw[i] != GUARD_BYTE)
	 bad ++;

      if (harness_raw[GUARD_SIZE + MAXBYTES + 1 + i] != GUARD_BYTE)
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


static void check_buffer(const char *expected, int len)
{
   ASSERT_EQ_INT(len, (int) (end - start));
   ASSERT_TRUE(memcmp(start, expected, len) == 0);
   ASSERT_EQ_INT('\0', *end);
}


static void check_state(int offset, int col, int row, int line, int count)
{
   ASSERT_EQ_INT(offset, (int) (ptr - start));
   ASSERT_EQ_INT(col, x);
   ASSERT_EQ_INT(row, y);
   ASSERT_EQ_INT(line, lineno);
   ASSERT_EQ_INT(count, lcount);
}

#endif /* !THARNESS_C */

#endif
