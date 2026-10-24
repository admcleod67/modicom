#include <io.h>
#include <dir.h>
#include <conio.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <string.h>
#include "editor.h"
#include "fileio.h"
#include "status.h"
#include "strings.h"
#include "video.h"
#include "prompt.h"


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
