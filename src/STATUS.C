/***
*
*  Help panel, status line, and messages for ModiCom.
*
*  Copyright (c) 1994-2026 Allan McLeod
*
*/

#include <string.h>
#include <bios.h>
#include <conio.h>

#include "editor.h"
#include "keyboard.h"
#include "status.h"
#include "video.h"


/* Draw the on-screen help strip */

void helpscreen(void)
{
   window(1, 2, 80, 8);
   clrscr();
   cputs("��������������������������������������������������������������������������ͻ\n\r");
   cputs("� CURSOR:  <-- --> �         UP  DOWN �    DELETE    � Insert Mode:    Ins �\n\r");
   cputs("�  Char:     ");
   putch(0x1b);
   putch(' ');
   putch(0x1a);
   cputs("   � Line:    ");
   putch(0x18);
   cputs("  ");
   putch(0x19);
   cputs("    � Char:    Del � Insert line:   ^N   �\n\r");
   cputs("�  Word:  Home End � Page: PgUp  PgDn � Word:    ^T  � Save: ^W  Abort:Esc �\n\r");
   cputs("�  Line:    ^");
   putch(0x1b);
   cputs(" ^");
   putch(0x1a);
   cputs("  � Find:   ^KF      � Line:    ^Y  � Read file:     ^KR  �\n\r");
   cputs("�                  � Refind: ^KL      �              � Write file:    ^KW  � \n\r");
   cputs("��������������������������������������������������������������������������ͼ ");
   window(1, 26 - maxrow, 80, 25);
}


/* Show Ins / Num / Caps lamps on the status line */

void statmsg(int status)
{
   window(1, 1, 80, 25);

   gotoxy(41, 1);
   if (status & 0x80)
      cputs("Ins");
   else
      cputs("   ");

   gotoxy(48, 1);
   if (status & 0x20)
      cputs("Num");
   else
      cputs("   ");

   gotoxy(51, 1);
   if (status & 0x40)
      cputs("Caps");
   else
      cputs("    ");

   window(1, 26 - maxrow, 80, 25);
}


/* Refresh modifier lamps when BIOS flags change */

void showstatus(void)
{
   static int status = 0;
   int modifiers;

   modifiers = bioskey(2);

   if (status != modifiers)
   {
      status = modifiers;
      statmsg(status);
   }
}


/* Show the current line and column on the status line */

void showpos(void)
{
   window(1, 1, 80, 1);
   gotoxy(59, 1);
   cprintf("L %d    ", lineno);
   gotoxy(66, 1);
   cprintf("C %d ", x);
   window(1, 26 - maxrow, 80, 25);
}


/* Refresh the full status line */

void scoreboard(void)
{
   char name[34];

   window(1, 1, 80, 1);
   clreol();

   if (!toobig)
   {
      strncpy(name, fname, 33);
      name[33] = '\0';
      cputs("Edit: ");
      cputs(name);
   }
   else
      cputs("File too large, some data may be lost.");

   statmsg(bioskey(2));
   showpos();
}


/* Toggle the help strip and enlarge or shrink the text area */

void helptoggle(void)
{
   char *str;

   maxrow = (maxrow == 17) ? 24: 17;
   y = 1;

   /* Show help at 17 rows, or a full text window at 24 */
   if (maxrow == 17)
      helpscreen();
   else
      window(1, 2, 80, 25);

   showpage(ptr - (x - 1), end);
}


/* Show a status message and wait for Space */

void msg(char *str)
{
   window(1, 1, 80, 1);
   cputs(str);
   clreol();

   /* Wait for Space, then restore the scoreboard */
   while ((getkey() != K_SPACE));
   scoreboard();
}
