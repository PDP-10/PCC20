# define BUFSIZ 512
# define FILE int
# define NULL 0
# define EOF (-1)

# define peekchar pkchar	/* avoid name conflict */
# define fopen flopen		/* " */
# define getc fgetc		/* " */
# define getchar fgeth		/* " */

# define feof ceof		/* direct translation */
# define putc cputc		/* " */

extern FILE *stdin, *stdout, *stderr;

# define ITS ITS
