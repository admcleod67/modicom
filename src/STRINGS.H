/* String macro definition */

#define	iseol(ch)	((ch == '\r') || (ch == 0x8d) || !ch)


/* String function prototypes */

int replace(char *str, unsigned len, char srch, char repch);
char *rtrim(char *sce);
int linelen(unsigned char *ptr, unsigned char *start);
