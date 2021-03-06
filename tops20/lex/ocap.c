/*
 * Created by DECUS LEX from file "cap.lxi" Sun Aug 29 01:19:05 1982
 */

/*
 * CREATED FOR USE WITH STANDARD I/O
 */

#
#include <stdio.h>
#ifdef vms
#include "c:lex.h"
#else
#include <lex.h>
#endif

extern int _lmovb();

#line 10 "cap.lxi"

extern	char	*token();

main()
{
	while (yylex())
		;
}

/* Standard I/O selected */
extern FILE *lexin;

llstin()
   {
   if(lexin == NULL)
      lexin = stdin;
   }

_Acap(__na__)		/* Action routine */
   {

#line 20 "cap.lxi"

	register char *cp;
	char *et;
   switch (__na__)
      {

      case 0:

#line 25 "cap.lxi"

		cp = token(&et);
		while (cp < et)
			putchar(*cp++);
	
         break;

      case 1:

#line 30 "cap.lxi"
putchar(token(0)[1]);
         break;

      case 2:

#line 31 "cap.lxi"
putchar(*token(0)+'a'-'A');
         break;

      case 3:

#line 32 "cap.lxi"
putchar(*token(0));
         break;
	}
	return(LEXSKIP);
}

#line 33 "cap.lxi"


int _Fcap[] =
   {
   -1, 3, 2, 3, 1, 3, -1, 0,
   -1, -1, -1, -1, 3, -1, -1, -1,
   -1,
   };

#line 34 "cap.lxi"

#define	LLTYPE1	char

LLTYPE1 _Ncap[] =
   {
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 5, 1, 1, 3, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 12, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 5,
   1, 2, 2, 2, 2, 2, 2, 2,
   2, 2, 2, 2, 2, 2, 2, 2,
   2, 2, 2, 2, 2, 2, 2, 2,
   2, 2, 2, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1,
   4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 4, 4, 4, 4, 4, 4,
   4, 4, 6, 8, 13, 6, 6, 14,
   8, 8, 15, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16,
   9, 16, 11, 10, 10, 16, 16, 11,
   16, 11, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7,
   };

LLTYPE1 _Ccap[] =
   {
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   3, 3, 3, 3, 3, 3, 3, 3,
   3, 3, 3, 3, 3, 3, 3, 3,
   3, 3, 3, 3, 3, 3, 3, 3,
   3, 3, 5, 6, 12, 9, 10, 13,
   6, 6, 14, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   5, -1, 5, 9, 10, -1, -1, 5,
   -1, 5, 6, 6, 6, 6, 6, 6,
   6, 6, 6, 6, 6, 6, 6, 6,
   6, 6, 6, 6, 6, 6, 6, 6,
   6, 6, 6, 6,
   };

LLTYPE1 _Dcap[] =
   {
   16, 16, 16, 16, 16, 16, 16, 16,
   6, 16, 6, 5, 5, 16, 16, 6,
  
   };

int _Bcap[] =
   {
   0, 0, 0, 191, 0, 272, 249, 0,
   0, 275, 276, 0, 204, 207, 280, 0,
   0,
   };

struct lextab cap =	{
			16,		/* Highest state */
			_Dcap,    	/* --> "Default state" table */
			_Ncap,    	/* --> "Next state" table */
			_Ccap,    	/* --> "Check value" table */
			_Bcap,    	/* --> "Base" table */
			339,		/* Index of last entry in "next" */
			_lmovb,		/* --> Byte-int move routine */
			_Fcap,    	/* --> "Final state" table */
			_Acap,    	/* --> Action routine */
			NULL,   	/* Look-ahead vector */
			0,		/* No Ignore class */
			0,		/* No Break class */
			0,		/* No Illegal class */
			};
