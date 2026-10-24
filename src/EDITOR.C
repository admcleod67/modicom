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
#include "search.h"

BYTE *start, *ptr, *end, fname[MAXPATH];
int x, y, lineno, lcount, changed, insert, maxrow, toobig;

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

static int abortedit(void)
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