/***
*
*  Single-line status prompts for ModiCom.
*
*  Copyright (c) 1994-2026 Allan McLeod
*
*/

#include <string.h>
#include <conio.h>

#include "keyboard.h"
#include "prompt.h"
#include "strings.h"


/* Prompt on the status line and return non-zero if accepted */

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
