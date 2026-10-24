#include <conio.h>
#include <bios.h>
#include <string.h>
#include "editor.h"
#include "status.h"
#include "keyboard.h"
#include "video.h"


void helpscreen(void)
{
   window(1, 2, 80, 8);
   clrscr();
   cputs("ษออออออออออออออออออหออออออออออออออออออหออออออออออออออหอออออออออออออออออออออป\n\r");
   cputs("บ CURSOR:  <-- --> บ         UP  DOWN บ    DELETE    บ Insert Mode:    Ins บ\n\r");
   cputs("บ  Char:     ");
   putch(0x1b);
   putch(' ');
   putch(0x1a);
   cputs("   บ Line:    ");
   putch(0x18);
   cputs("  ");
   putch(0x19);
   cputs("    บ Char:    Del บ Insert line:   ^N   บ\n\r");
   cputs("บ  Word:  Home End บ Page: PgUp  PgDn บ Word:    ^T  บ Save: ^W  Abort:Esc บ\n\r");
   cputs("บ  Line:    ^");
   putch(0x1b);
   cputs(" ^");
   putch(0x1a);
   cputs("  บ Find:   ^KF      บ Line:    ^Y  บ Read file:     ^KR  บ\n\r");
   cputs("บ                  บ Refind: ^KL      บ              บ Write file:    ^KW  บ \n\r");
   cputs("ศออออออออออออออออออสออออออออออออออออออสออออออออออออออสอออออออออออออออออออออผ ");
   window(1, 26 - maxrow, 80, 25);
}


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


void showpos(void)
{
   window(1, 1, 80, 1);
   gotoxy(59, 1);
   cprintf("L %d    ", lineno);
   gotoxy(66, 1);
   cprintf("C %d ", x);
   window(1, 26 - maxrow, 80, 25);
}


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


void helptoggle(void)
{
   char *str;

   maxrow = (maxrow == 17) ? 24: 17;
   y = 1;

   if (maxrow == 17)
      helpscreen();
   else
      window(1, 2, 80, 25);

   showpage(ptr - (x - 1), end);
}


void msg(char *str)
{
   window(1, 1, 80, 1);
   cputs(str);
   clreol();
   while ((getkey() != K_SPACE));
   scoreboard();
}

