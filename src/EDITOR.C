#include <stdio.h>
#include <io.h>
#include <dir.h>
#include <conio.h>
#include <ctype.h>
#include <bios.h>
#include <alloc.h>
#include <fcntl.h>
#include <process.h>
#include <sys\stat.h>
#include <string.h>
#include "editor.h"
#include "keyboard.h"
#include "strings.h"
#include "video.h"
#include "status.h"
#include "navigate.h"
#include "edit.h"
#include "fileio.h"

BYTE *start, *ptr, *end, fname[MAXPATH];
int x, y, lineno, lcount, changed, insert, maxrow, toobig;

int  prompt(char *msg, char *str);

int prompt(char *msg, char *str)
{
   int len;
   unsigned key;

   window(1, 1, 80, 1);
   clreol();
   cputs(msg);

   len = strlen(msg) + 1;
   key = getline(str, len, 1, 80 - len);

   if (key != K_ESC)
      rtrim(str);

   return *str && (key != K_ESC);
}

int findprompt(char *srchstr)
{
   char buf[MAXSEARCH + 1];

   if (!prompt("Find? ", buf))
      return 0;

   strcpy(srchstr, buf);
   return 1;
}

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

void ctrlkey(void)
{
   static char srchstr[MAXSEARCH + 1] = "\0";
   unsigned key;

   while(!(key = getkey()));

   switch (key)
   {
      case 'F':

      case 'f':

      case K_CTRL_F:
	 if (findprompt(srchstr))
	 {
	    if (find(srchstr))
	       scoreboard();
	 }
	 else
	    scoreboard();
	 break;

      case 'L':

      case 'l':

      case K_CTRL_L:
	 find(srchstr);
	 break;

      case 'R':

      case 'r':

      case K_CTRL_R:
	 readfile();
	 break;

      case 'W':

      case 'w':

      case K_CTRL_W:
	 writefile();
   }
}

int textedit(char *name)
{
   int key, stop, bytes;

   start = malloc(MAXBYTES + 1);

   strcpy(fname, name);

   x = MIN_COL;
   y = MIN_ROW;
   lineno = 1;
   maxrow = 17;

   insert = (bioskey(2) & 0x80);

   changed = FALSE;
   stop = FALSE;
   toobig = FALSE;

   textattr(LIGHTGRAY);
   clrscr();
   helpscreen();
   loadfile(fname);
   scoreboard();
   gotoxy(x, y);
   showpage(ptr, end);

   do
   {
      showpos();

      do
      {
	 showstatus();
	 gotoxy(x, y);
	 key = getkey();
      }
      while (!key);

      switch (key)
      {
	 case K_UP:
	    cursup();
	    break;

	 case K_DOWN:
	    cursdown();
	    break;

	 case K_LEFT:
	    cursleft();
	    break;

	 case K_RIGHT:
	    cursright();
	    break;

	 case K_PGUP:
	    pageup();
	    break;

	 case K_PGDN:
	    pagedown();
	    break;

	 case K_BS:
	    backspace();
	    break;

	 case K_INS:
	    insert = !insert;
	    break;

	 case K_DEL:
	    delchar();
	    break;

	 case K_HOME:
	    lastword();
	    break;

	 case K_END:
	    nextword();
	    break;

	 case K_CTRL_C:
	    putchar(BELL);
	    break;

	 case K_CTRL_K:
	    ctrlkey();
	    break;

	 case K_CTRL_Y:
	    del();
	    break;

	 case K_RETURN:
	    newline(insert);
	    break;

	 case K_CTRL_T:
	    delword();
	    break;

	 case K_CTRL_N:
	    addline();
	    break;

	 case K_CTRL_LEFT:
	    gotobol();
	    break;

	 case K_CTRL_RIGHT:
	    gotoeol();
	    break;

	 case K_CTRL_W:

	 case K_CTRL_END:
	    if (changed)
	       textwrite(fname);
	    stop = TRUE;
	    break;

	 case K_TAB:
	    tabulate();
	    break;

	 case K_ESC:
	    stop = abortedit();
	    break;

	 case K_F1:
	    helptoggle();
	    break;

	 default:
	    if (key & 0xff)
	       inschar(key);
      }
   }
   while (!stop);

   window(1, 1, 80, 25);
   clrscr();

   free(start);

   return 0;
}