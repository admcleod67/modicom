#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include "editor.h"


void showuse(int parameters)
{
   #ifdef VERSION_M
      printf("\nModiCom xBase Programmer's Editor v2.1m\n");
   #else
      printf("\nModiCom xBase Programmer's Editor v2.1\n");
   #endif

   printf("Copyright (c) 1994-2026 Allan McLeod\n\n");

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
