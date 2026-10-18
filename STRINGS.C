#include <string.h>
#include <ctype.h>


int replace(char *str, int len, char srch, char repch)
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