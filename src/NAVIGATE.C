#include <conio.h>
#include <ctype.h>
#include "editor.h"
#include "navigate.h"
#include "strings.h"
#include "video.h"


void lineup(void)
{
   while (*(-- ptr));

   while (*(ptr - 1))
   {
      if (ptr == start)
	 break;
      ptr --;
   }

   if (y == MIN_ROW)
   {
      gotoxy(MIN_COL, MIN_ROW);
      insline();
      showline(ptr, end);
   }
   else
      y --;

    lineno --;
}

int linedown(void)
{
   while (*(ptr ++));

   if (y == maxrow)
   {
      gotoxy(MIN_COL, MIN_ROW);
      delline();
      gotoxy(MIN_COL, maxrow);
      showline(ptr, end);
   }
   else
      y ++;

   lineno ++;

   return 1;
}

void gotocol(int col)
{
   int len;

   len = linelen(ptr, start);
   x = (col > len) ? len + 1 : col;
   ptr += (x - 1);
}

void cursup(void)
{
   if (lineno > 1)
   {
      lineup();
      gotocol(x);
   }
}

void cursdown(void)
{
   if (lineno < lcount)
   {
      linedown();
      gotocol(x);
   }
}

void cursleft(void)
{
   if (ptr != start)
   {
      if (!*(ptr - 1))
      {
	 lineup();
	 gotocol(80);
      }
      else
      {
	 x --;
	 ptr --;
      }
   }
}

void cursright(void)
{
   if ((ptr != end) && (x < 80))
   {
      if (iseol(*ptr))
      {
	 linedown();
	 x = 1;
      }
      else
      {
	 x ++;
	 ptr ++;
      }
   }
}

int gotorow(int row)
{
   int i;

   for (i = row - 1; i; i --)
   {
      while (*ptr ++);
      lineno ++;
      if (lineno == lcount)
	 break;
   }

   return (i ? i - 1: 0);
}

void pageup(void)
{
   int i;

   if (lineno > 1)
   {
      while (*(ptr - 1))
	 ptr --;

      for (i = (maxrow + y) - 1; i; i --)
      {
	 do
	 {
	    if (ptr == start)
	       break;
	    ptr --;
	 }
	 while (*(ptr - 1));

	 if (lineno == 1)
	    break;
	 lineno --;
      }

      x = MIN_COL;
      gotoxy(x, MIN_ROW);
      showpage(ptr, end);
      gotorow(y);
   }
}

void pagedown(void)
{
   int i;

   if (lineno < lcount)
   {
      gotorow(maxrow - y + 2);
      x = MIN_COL;
      gotoxy(x, MIN_ROW);
      showpage(ptr, end);

      if (lineno < lcount)
	 y -= gotorow(y);
      else
	 y = MIN_ROW;
   }
}

void scrollup(void)
{
   char *str;
   int line, i;

   str = ptr;
   line = lineno;

   for (i = y; i < maxrow; i ++)
   {
      while (*str ++);
      line ++;
      if (line > lcount)
	 break;
   }

   gotoxy(MIN_COL, y + 1);
   delline();

   if (i == maxrow)
   {
      gotoxy(1, i);
      showline(str, end);
   }
}

void nextword(void)
{
   if (iseol(*ptr))
   {
      if (ptr != end && x < 80)
      {
	 linedown();
	 x = 1;
      }
   }
   else
   {
      while (!iseol(*ptr) && x < 80)
      {
	 x ++;
	 if (isspace(*ptr ++))
	    break;
      }

      while (isspace(*ptr) && x < 80)
      {
	 ptr ++;
	 x ++;
      }
   }
}

void lastword(void)
{
   while (ptr > start)
   {
      if (!*(ptr - 1) || !isspace(*(ptr - 1)))
	 break;
      x --;
      ptr --;
   }

   if (!*(ptr - 1))
   {
      lineup();
      gotocol(80);
   }

   while (ptr > start)
   {
      if (!*(ptr - 1) || isspace(*(ptr - 1)))
	 break;
      x --;
      ptr --;
   }
}

void gotoeol(void)
{
   while (!iseol(*ptr) && x < 80)
   {
      ptr ++;
      x ++;
   }
}

void gotobol(void)
{
   while (*(ptr -1))
   {
      if (ptr == start)
	 break;
      ptr --;
      x --;
   }
}
