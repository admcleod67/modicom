/***
*
*  Find prompt and forward search for ModiCom.
*
*  Copyright (c) 1994-2026 Allan McLeod
*
*/

#include <string.h>
#include <conio.h>

#include "editor.h"
#include "prompt.h"
#include "search.h"
#include "status.h"
#include "video.h"


/* Prompt for a search string; copy it only when accepted */

int findprompt(char *srchstr)
{
   char buf[MAXSEARCH + 1];

   if (!prompt("Find? ", buf))
      return 0;

   strcpy(srchstr, buf);
   return 1;
}


/* Search forward from after the cursor */

int find(char *srchstr)
{
   int found, xpos, ypos, line;
   char *str;

   line = lineno;
   str = ptr;
   xpos = x;
   ypos = y;

   if (str != end)
   {
      if (*ptr)
	 xpos = x + 1;
      else
      {
	 xpos = 1;
	 ypos ++;
	 line ++;
      }
      str = ptr + 1;
   }
   else
   {
      str = ptr;
      xpos = x;
      ypos = y;
   }

   found = FALSE;

   /* Scan the buffer for the search string */
   if (*srchstr)
   {
      while (str != end)
      {
	 found = !strncmp(str, srchstr, strlen(srchstr));

	 if (found)
	    break;

	 if (!*(str ++))
	 {
	    line ++;
	    ypos ++;
	    xpos = 1;
	 }
	 else
	    xpos ++;
      }
   }

   if (found)
   {
      /* Move the cursor to the match; repaint if off the page */
      ptr = str;
      while (ptr != start)
      {
	 if (!*(ptr - 1))
	    break;
	 ptr --;
      }

      x = xpos;
      y = ypos;
      if ((ypos > maxrow) || (lineno - y > line))
      {
	 y = 1;
	 window(1, 26 - maxrow, 80, 25);
	 showpage(ptr, end);
      }
      lineno = line;
      ptr = str;
   }
   else
      msg("** Not found ** (press SPACE) ");

   return found;
}
