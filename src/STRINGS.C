#include <string.h>
#include <ctype.h>
#include "strings.h"


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
