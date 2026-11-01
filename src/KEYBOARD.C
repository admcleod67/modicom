/***
*
*  Keyboard input helpers for ModiCom.
*
*  Copyright (c) 1994-2026 Allan McLeod
*
*/

#include <ctype.h>
#include <bios.h>
#include <conio.h>

#include "keyboard.h"


/* Return the next key, or zero if none is waiting */

unsigned getkey(void)
{
   unsigned key;

   if(bioskey(1))
   {
      key = bioskey(0);
      return key & 0xff ? key & 0xff : key & 0xff00;
   }
   return 0;
}


/* Edit a single line; Return or Escape ends input */

unsigned getline(char *str, int x, int y, int maxlen)
{
   unsigned key, len;

   key = 0;
   len = 0;

   do
   {
      gotoxy(x + len, y);

      while (!(key = getkey()));

      switch (key)
      {
	 case K_BS:
	    if (len)
	    {
	       str --;
	       len --;
	       gotoxy(x + len, y);
	       putch(' ');
	    }
	    break;

	 case K_ESC:

	 case K_RETURN:
	    *str = '\0';
	    break;

	 default:
	    if ((len != maxlen) && isprint(key))
	    {
	       *str ++ = key;
	       len ++;
	       putch(key);
	    }

      }
   }
   while ((key != K_RETURN) && (key != K_ESC));

   return key;
}
