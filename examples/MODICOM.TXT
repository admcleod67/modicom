#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define ABORT 0


void showuse(int parameters)
{
   printf("\nModiCom Programmer's Editor v1.3\n");
   printf("Copyright (c) 1994, Orkney Software Ltd\n\n");

   if (parameters > 2)
      printf(" Too many parameters!\n  ");

   if (parameters < 2)
      printf(" Too few parameters!\n  ");

   printf("  Usage: ModiCom <filename>\n");
}



int main(int argc, char *argv[])
{
   _wscroll = 0;

   if (argc != 2)
   {
      showuse(argc);
      exit(1);
   }

   if (!strcmp(argv[1], "/?"))
   {
      showuse(argc);
      exit(0);
   }

   textedit(argv[1]);

   return 0;
}
