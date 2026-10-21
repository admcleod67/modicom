/* Minimal test assertion harness for Borland C++ 3.1. */

#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <string.h>

static int test_assertions = 0;
static int test_failures = 0;

static void test_assert_true(int condition, const char *file, int line,
   const char *expression)
{
   test_assertions ++;

   if (!condition)
   {
      test_failures ++;
      printf("FAIL %s:%d: %s\n", file, line, expression);
   }
}

static void test_assert_eq_int(int expected, int actual, const char *file,
   int line, const char *expression)
{
   test_assertions ++;

   if (expected != actual)
   {
      test_failures ++;
      printf("FAIL %s:%d: %s (expected %d, got %d)\n", file, line,
         expression, expected, actual);
   }
}

static void test_assert_eq_str(const char *expected, const char *actual,
   const char *file, int line, const char *expression)
{
   test_assertions ++;

   if ((expected == NULL) || (actual == NULL) ||
      (strcmp(expected, actual) != 0))
   {
      test_failures ++;
      printf("FAIL %s:%d: %s (expected \"%s\", got \"%s\")\n", file, line,
         expression, expected ? expected : "(null)",
         actual ? actual : "(null)");
   }
}

static int test_summary(void)
{
   printf("%d assertions, %d failures\n", test_assertions, test_failures);

   return test_failures ? 1 : 0;
}

#define ASSERT_TRUE(expression) \
   test_assert_true((expression) != 0, __FILE__, __LINE__, #expression)

#define ASSERT_EQ_INT(expected, actual) \
   test_assert_eq_int((expected), (actual), __FILE__, __LINE__, \
      #expected " == " #actual)

#define ASSERT_EQ_STR(expected, actual) \
   test_assert_eq_str((expected), (actual), __FILE__, __LINE__, \
      #expected " == " #actual)

#endif
