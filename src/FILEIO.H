/***
*
*  File I/O function prototypes for ModiCom.
*
*  Copyright (c) 1994-2026 Allan McLeod
*
*/

#ifndef FILEIO_H
#define FILEIO_H

void textwrite(char *fname);
unsigned textread(char *fname, int create);
void loadfile(char *fname);
void readfile(void);
void writefile(void);

#endif
