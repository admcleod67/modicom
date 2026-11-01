/***
*
*  String and line helpers for ModiCom.
*
*  Copyright (c) 1994-2026 Allan McLeod
*
*/

#include <ctype.h>
#include <string.h>

#include "strings.h"


/* Replace every srch with repch over len bytes; return the count */

int replace(char *str, unsigned len, char srch, char repch)
{
   int count;

   count = 0;

   while (len --)
   {
      if (*str == srch)
      {
	 *str = repch;
	 count ++;
      }
      str ++;
   }

   return count;
}


/* Trim trailing whitespace from a string */

char *rtrim(char *sce)
{
   char *str;

   str = sce + strlen(sce);

   while (str != sce)
   {
      if (!isspace(*(str - 1)))
	 break;
      str --;
   }

   *str = '\0';

   return str;
}


/* Return the display length of the current line */

int linelen(unsigned char *ptr, unsigned char *start)
{
   int len;

   while (*(ptr - 1))
   {
      if (ptr == start)
	 break;

      ptr --;
   }

   len = 0;

   while (!iseol(*ptr))
   {
      ptr ++;
      len ++;
   }

   return len;
}
