#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include "editor.h"
#include "edit.h"
#include "status.h"
#include "navigate.h"
#include "keyboard.h"
#include "strings.h"
#include "video.h"


void inschar(int ch)
{
   if (!iscntrl(ch))
   {
      if (insert || iseol(*ptr))
      {
	 if ((x < 80) && (end < (start + MAXBYTES)))
	 {
	    memmove(ptr + 1, ptr, (end - ptr) + 1);
	    *ptr = ch;
	    showline(ptr, end);
	    ptr ++;
	    end ++;
	    x ++;
	    changed = TRUE;
	 }
	 else
	    putchar(BELL);
      }
      else
      {
	 if (!iseol(*ptr) && x < 80)
	 {
	    *ptr = ch;
	    showline(ptr, end);
	    ptr ++;
	    x ++;
	    changed = TRUE;
	 }
	 else
	    putchar(BELL);
      }
   }
}

void del(void)
{
   char *str;
   int len;

   if (ptr != end)
   {
      while (*(ptr - 1))
      {
	 if (ptr == start)
	    break;

	 ptr --;
      }
      len = strlen(ptr);

      if (lineno == lcount)
	 str = ptr + len;
      else
	 str = ptr + len + 1;

      memmove(ptr, str, (end - str) + 1);

      if (lineno == lcount)
	 end -= len;
      else
      {
	 end -= (len + 1);
	 lcount --;
      }
      x = 1;

      gotoxy(x, y);
      showline(ptr, end);
      scrollup();

      changed = TRUE;
   }
}

void newline(int insert)
{
   if (insert || (ptr == end))
   {
      if ((end + 1) < (start + MAXBYTES))
      {

	 memmove(ptr + 2, ptr, (end - ptr) + 1);
	 *ptr = '\r';
	 *(ptr + 1) = '\0';
	 end += 2;
	 showline(ptr, end);
	 ptr += 2;
	 lineno ++;
	 lcount ++;
	 x = MIN_COL;

	 if (y == maxrow)
	 {
	    gotoxy(MIN_COL, MIN_ROW);
	    delline();
	 }
	 else
	    y ++;

	 gotoxy(x, y);
	 insline();
	 showline(ptr, end);
	 changed = TRUE;
      }
   }
   else
   {
      if (lineno < lcount)
      {
	 linedown();
	 x = MIN_COL;
      }
      else
	 putchar(BELL);
   }
}

void addline(void)
{
   if ((end + 1) < (start + MAXBYTES))
   {

      memmove(ptr + 2, ptr, (end - ptr) + 1);
      *ptr = '\r';
      *(ptr + 1) = '\0';
      end += 2;
      showline(ptr, end);
      lcount ++;

      if (y != maxrow)
      {
	 gotoxy(MIN_COL, y + 1);
	 insline();
	 showline(ptr + 2, end);
      }

      changed = TRUE;
   }
}

int abortedit(void)
{
   int key;

   if (changed)
   {
      window(1, 1, 40, 1);
      cputs("Abort editing? (Y/N) ");
      clreol();

      key = 0;
      while (!key)
	 key = getkey();

      if ((key == 'n') || (key == 'N'))
      {
	 scoreboard();
	 return FALSE;
      }
   }

   return TRUE;
}

void backspace(void)
{
   int offset;
   char *str;

   if (ptr != start)
   {
      if (x == MIN_COL)
      {
	 memmove(ptr - 2, ptr, (end - ptr) + 1);
	 ptr -= 2;
	 end -= 2;
	 str = ptr;

	 while (*(str - 1))
	 {
	    if (str == start)
	       break;
	    else
	       str --;
	 }

	 if (y > MIN_ROW)
	    y --;
	 lineno --;
	 x = (ptr - str) + 1;

	 if (x > 80)
	 {
	    offset = x - 80;
	    x -= offset;
	    ptr -= offset;
	 }

	 lcount --;
	 gotoxy(MIN_COL, y);
	 showline(str, end);

	 scrollup();
      }
      else
      {
	 memmove(ptr - 1, ptr, (end - ptr) + 1);
	 x --;
	 gotoxy(x, y);
	 showline(-- ptr, -- end);
      }
      changed = TRUE;
   }
}

void delchar(void)
{
   int len;

   len = linelen(ptr, start);

   if (x > len)
   {
      if ((end - ptr) >= 2)
      {
	 memmove(ptr, ptr + 2, end - ptr);
	 end -= 2;
	 showline(ptr, end);
	 lcount --;

	 if (y != maxrow)
	 {
	    scrollup();
	    gotoxy(x, y);
	 }

	 changed = TRUE;
       }
   }
   else
   {
      if (ptr != end)
      {
	 memmove(ptr, ptr + 1, end - ptr);
	 showline(ptr, -- end);

	 changed = TRUE;
       }
   }
}

void delword(void)
{
   BYTE *str;
   int line, i;

   if (ptr != end)
   {
      str = ptr;

      if (iseol(*ptr))
      {
	 str += 2;
	 memmove(ptr, str, (end - str) + 1);
	 end -= str - ptr;
	 showline(ptr, end);
	 scrollup();
	 lcount --;
      }
      else
      {
	 while (!iseol(*ptr) && !isspace(*str))
	    str ++;

	 while (isspace(*str) && !iseol(*str))
	    str ++;

	 memmove(ptr, str, (end - str) + 1);
	 end -= str - ptr;
	 showline(ptr, end);
      }

      changed = TRUE;
   }
}

void tabulate(void)
{
   int offset, len;

   if (x < 80)
   {
      len = linelen(ptr, start);
      offset = 5 - (x % 4);

      if (((x + offset - 1) <= len) && !insert)
      {
	 x += offset;
	 ptr += offset;
      }
      else
      {
	 if ((x <= len) && !insert)
	 {
	    offset = (len - x) - 1;
	    ptr += (len - x) + 1;
	    x = len + 1;
	 }

	 memmove(ptr + offset, ptr, (end - ptr) + 1);
	 memset(ptr, ' ', offset);
	 gotoxy(x, y);
	 showline(ptr, end);
	 end += offset;
	 x += offset;
	 ptr += offset;
	 changed = TRUE;
      }

      if (x > 80)
      {
	 offset = x - 80;
	 x -= offset;
	 ptr -= offset;
      }
   }
   else
      putchar(BELL);
}
