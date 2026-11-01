/***
*
*  Buffer edit function prototypes for ModiCom.
*
*  Copyright (c) 1994-2026 Allan McLeod
*
*/

#ifndef BUFFEDIT_H
#define BUFFEDIT_H

void inschar(int ch);
void del(void);
void newline(int insert);
void addline(void);
void backspace(void);
void delchar(void);
void delword(void);
void tabulate(void);

#endif
