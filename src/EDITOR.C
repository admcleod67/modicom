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


BYTE *start, *ptr, *end, fname[MAXPATH];
int x, y, lineno, lcount, changed, insert, maxrow, toobig;

void lineup(void);
int  linedown(void);
void cursup(void);
void cursdown(void);
void cursleft(void);
void cursright(void);
void pageup(void);
void pagedown(void);
void scrollup(void);
void del(void);
void addline(void);
int  abortedit(void);
void backspace(void);
void delchar(void);
void delword(void);
void nextword(void);
void lastword(void);
void gotoeol(void);
void gotobol(void);
void tabulate(void);
void readfile(void);
void writefile(void);
int  prompt(char *msg, char *str);


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


void textwrite(char *fname)
{
   int out;
   char *ptr;

   replace(start, (unsigned) (end - start), '\0', '\n');

   #ifndef VERSION_M
      *end = 0x1a;
   #endif

   out = open(fname, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IWRITE);

   if (out == -1)
      msg("Unable to write file (press SPACE) ");

   #ifdef VERSION_M
      write(out, start, (unsigned) (end - start));
   #else
      write(out, start, (unsigned) (end - start) + 1);
   #endif

   close(out);
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


unsigned textread(char *fname, int create)
{
   char drive[MAXDRIVE], dir[MAXDIR], file[MAXFILE], ext[MAXEXT];
   int in, nread;
   unsigned maxsize, total, chunk;

   if (!(fnsplit(fname, drive, dir, file, ext) & EXTENSION))
      strcat(fname, ".prg");

   if ((in = open(fname, O_RDONLY | O_BINARY)) == -1)
   {
      if (!create)
	 msg("File does not exist (press SPACE) ");

      return 0;
   }

   maxsize = MAXBYTES - (unsigned) (end - start);

   if ((long) filelength(in) > maxsize)
      toobig = TRUE;

   /* Borland's read() returns a signed int, so counts above 32767
      must be accumulated in chunks that fit in a positive int. */
   total = 0;
   while (total < maxsize)
   {
      chunk = maxsize - total;
      if (chunk > 32767)
	 chunk = 32767;

      nread = read(in, ptr + total, chunk);
      if (nread == -1)
      {
	 close(in);
	 msg("Error reading file (press SPACE) ");
	 return (unsigned) -1;
      }
      if (nread == 0)
	 break;

      total += (unsigned) nread;
      if ((unsigned) nread < chunk)
	 break;
   }

   close(in);

   if ((total > 0) && (*(ptr + total - 1) == 26))
      total --;

   return total;
}


void loadfile(char *fname)
{
   unsigned bytes;

   ptr = start;
   end = ptr;

   bytes = textread(fname, TRUE);

   if ((bytes == 0) || (bytes == (unsigned) -1))
   {
      lcount = 1;
   }
   else
   {
      lcount = replace(ptr, bytes, '\n', '\0') + 1;
      end = ptr + bytes;
   }

   *end = '\0';
}


void readfile(void)
{
   int nlines;
   unsigned bytes;
   char fname[MAXPATH], *lastbyte, *nstart;

   if (prompt("Enter file name: ", fname))
   {
      lastbyte = start + MAXBYTES;
      nstart = lastbyte - (end - ptr);
      memmove(nstart, ptr, (end - ptr) + 1);
      bytes = textread(fname, FALSE);
      if (bytes == (unsigned) -1)
	 bytes = 0;

      if (bytes > 0)
      {
	 nlines = replace(ptr, bytes, '\n', '\0');
	 lcount += nlines;
      }
      memmove(ptr + bytes, nstart, (lastbyte - nstart) + 1);
      end += bytes;
      window(1, 26 - maxrow, 80, 25);
      gotoxy(x, y);
      showpage(ptr, end);
      window(1, 1, 80, 1);
      changed = (bytes > 0) ? TRUE: changed;
   }

   scoreboard();
}


void writefile(void)
{
   char fname[MAXPATH];

   if (prompt("Enter file name: ", fname))
   {
      textwrite(fname);
      replace(start, (unsigned) (end - start), '\n', '\0');
      *end = '\0';
   }

   scoreboard();
}


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