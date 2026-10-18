void showpage(char *sce, int x, int y)
{
   char string[80], ch;
   int i, len;

   i = x;

   for (i = y; i != maxrow + 1; i ++)
   {
      gotoxy(MIN_COL, i);
      if (sce < end)
      {
	 len = strlen(sce);
	 strncpy(string, sce, 79);
	 string[79] = '\0';
	 sce += len;
	 cputs(string);
	 clreol();
	 gotoxy(MAX_COL, i);
	 if (len < 80)
	 {
	    if (sce != end)
	       putch('<');
	 }
	 else
	    putch('+');
	 sce ++;
      }
      else
	 clreol();
   }
}


