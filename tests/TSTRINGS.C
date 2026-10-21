/* Tests for the string routines in src/STRINGS.C. */

#include "TEST.H"
#include "strings.h"


static void test_replace(void)
{
   char text[] = "aXbXcX";
   char absent[] = "abc";
   char limited[] = "aaaa";
   char newlines[] = "a\nb\n";
   int count;

   count = replace(text, 6, 'X', '-');
   ASSERT_EQ_INT(3, count);
   ASSERT_EQ_STR("a-b-c-", text);

   count = replace(absent, 3, 'X', '-');
   ASSERT_EQ_INT(0, count);
   ASSERT_EQ_STR("abc", absent);

   count = replace(limited, 2, 'a', 'b');
   ASSERT_EQ_INT(2, count);
   ASSERT_EQ_STR("bbaa", limited);

   count = replace(newlines, 4, '\n', '\0');
   ASSERT_EQ_INT(2, count);
   ASSERT_EQ_INT('\0', newlines[1]);
   ASSERT_EQ_INT('\0', newlines[3]);
}


static void test_rtrim(void)
{
   char trimmed[] = "  abc   ";
   char untouched[] = "abc";
   char empty[] = "";
   char spaces[] = "   ";
   char *end;

   end = rtrim(trimmed);
   ASSERT_EQ_STR("  abc", trimmed);
   ASSERT_TRUE(end == trimmed + 5);
   ASSERT_EQ_INT('\0', *end);

   end = rtrim(untouched);
   ASSERT_EQ_STR("abc", untouched);
   ASSERT_TRUE(end == untouched + 3);

   end = rtrim(empty);
   ASSERT_EQ_STR("", empty);
   ASSERT_TRUE(end == empty);

   end = rtrim(spaces);
   ASSERT_EQ_STR("", spaces);
   ASSERT_TRUE(end == spaces);
}


static void test_linelen(void)
{
   static unsigned char buffer[] =
   {
      '\0',
      'a', 'b', 'c', '\0',
      'd', 'e', '\r', '\0',
      'f', 'g', 0x8d, '\0',
      'h', 'i', 'j', '\0'
   };
   unsigned char *start;
   unsigned char *line;

   start = buffer + 1;

   ASSERT_EQ_INT(3, linelen(start, start));
   ASSERT_EQ_INT(3, linelen(start + 2, start));

   line = buffer + 5;
   ASSERT_EQ_INT(2, linelen(line, start));
   ASSERT_EQ_INT(2, linelen(line + 1, start));

   line = buffer + 9;
   ASSERT_EQ_INT(2, linelen(line, start));
   ASSERT_EQ_INT(2, linelen(line + 1, start));

   line = buffer + 13;
   ASSERT_EQ_INT(3, linelen(line, start));
   ASSERT_EQ_INT(3, linelen(line + 2, start));
}


int main(void)
{
   printf("STRINGS tests\n");

   test_replace();
   test_rtrim();
   test_linelen();

   return test_summary();
}
