# include <stdio.h>

/************************************************************************

ALLOC - C Dynamic Storage Management Package for Single Section TOPS-20

	This code was written by Alan Snyder and Eliot B. Moss. Modified
	for the Standard I/O Library by John Wroclawski

	This package implements allocation and freeing of variable sized blocks
	of free storage.  A modified buddy system is used, with provision for
	allocating page sized units on page boundaries so that page mapping may
	be done.

	p = malloc (n)		allocates a block n "sizeof's" long
				(e.g., malloc (sizeof (foo))). Unlike
				UNIX malloc, the memory is zeroed.

	p = calloc (num, size)	allocates a block of storage for an
				array of num elements of size "size"

	free (p)		returns blocks to storage


	q = realloc (p, n)	returns a pointer to a block of size n
				with q[0] == p[0] ... p[k] == q[k],
				where k = min (n, length (p))  (p is a
				previously allocated block)  q == p if
				n <= length (p)

	n = pg_size ()		returns the size of a page
	p = pg_get (n)		returns the address of a newly allocated block
				of pages
	pg_ret (p)		frees a previously allocated block of pages

	nwfree = alocstat (&nwalloc, &nbfree);
				returns statistics

	n = alocflush ()	flushes pages not in use; returns number
				of words saved


	Additionally, a global variable, nocore, is provided, which
	contains (a pointer to) the procedure to invoke when memory runs
	out (this could trigger garbage collection, if the user programs
	it).  The normal function is to ignore the situation, the allocator
	will then return NULL.

	Theory of operation:

		Sizes less than a page long are treated separately from larger
	sizes.  One reason is that in order to support page mapping,
	information concerning allocated blocks of page size and larger is kept
	in a separate array.  On the other hand, there are potentially much too
	many small objects to do that, so one word of each object keeps the
	necessary size and link information.
		We modify the basic buddy system by allocating blocks of size
	halfway between powers of two as well as the usual powers of two.  This
	should result in less external fragmentation than the powers-of-two
	buddy system, with no ill effects.  A block that is a power of two in
	size may be split in the ratios 3:1 or 2:2, and one that is a multiple
	of 6 in size may be split 4:2 or 3:3.
		For sizes less than a page, a header word is used to store
	information necessary for putting blocks back together, etc.  This
	includes a code indicating the size of the block, whether it is the
	left or right member of a pair, and the ratio of the sizes of the
	members of the pair.  In addition it is necessary for the right member
	to save the old values of the flags for the left member (i.e., the
	values before the pair was split).  Here is the encoding of the header
	words for blocks less than a page in size; the numbers in parentheses
	indicate the widths of the fields, in bits:

	magic (18), unused (5), free (1), oflags (3), flags (3), size (6)

	The size is actually an index into a table of standard sizes; there are
	19 of them.  The flags are encoded as three separate bits:

		even split: 1 <=> ratio was 1:1
		right sibling: 1 <=> this is a right hand member of a pair
			(i.e., the one with the higher address)
		parent-power-of-two: 1 <=> size of parent block was power of 2

	The free flag indicates whether the block is free or not.  The magic
	number value is the address of the block XORed with a magic number; it
	is checked in the freeing routines, for consistency.

	Free blocks are doubly linked, using the second word.  The forward link
	is in the right half, and the back link in the left half.

	Blocks a page or more in size do not have the information stored
	directly in the block, but in a separate array, so the whole block can
	be used for page mapping.  Here is the layout of bits for them:

		prev page (9), next page (9), unused (4),
		exists (1), free (1), oflags (3), flags	(3), size (6)

	The exists bit is not currently used on TOPS-20.

************************************************************************/

/* Page size and shift amount constants */

# define PGSIZE 512
# define PGLOG2 9
# define NSIZES 16

# define NPAGES (01000000 / PGSIZE)

# define TSIZES 34
# define BADSIZ (NSIZES + 1)

/* Masks and shift amounts for usual header flags */

# define SIZMSK 077
# define _RIGHT 0100
# define _EVEN 0200
# define _PP2 0400
# define FLGMSK 0700
# define FSHIFT 6
# define OFLGMSK 07000
# define OFSHIFT 3
# define ISFREE 010000
# define MAGIC 0732561
# define MMASK halves (0777777, 0)
# define MSHIFT 18
# define LMASK halves (0777777, 0)
# define RMASK 0777777

/* extra masks and shifts for blocks of page size and up */

# define PPREVMSK halves (0777000, 0)
# define PNEXTMSK halves (0000777, 0)
# define PNSHIFT 18
# define PPSHIFT 27
# define PEXISTS 020000

static unsigned blksiz[] = {
	2,	3,	4,	6,	8,	12,	16,
	24,	32,	48,	64,	96,	128,	192,
	256,	384,	512,	0,	1024,	1536,	2048,
	3072,	4096,	6144,	8192,	12288,	16384,	24576,
	32768,	49152,	65536,	98304,	131072,	196608 };

static unsigned free_blks[TSIZES];
static unsigned page_info[NPAGES];

/* this table is for quick size lookup */

static int iblksiz[] = {
	 0,  0,  1,  2,  3,  3,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12 };
	
# define TRUE  1
# define FALSE 0

/* forward references */

static int *split(), psplit(), *palloc();

/* links to runtime package */

# rename lowlim "LOWLIM"
# rename highlim "HILIM"

extern int lowlim, highlim;

ainit ()
{
	register int i, n, p;

	fill (free_blks, TSIZES, 0);
	fill (page_info, NPAGES, 0);
	p = lowlim;
	n = highlim - lowlim;
	i = TSIZES - 1;
	while (n > 0) {
		register unsigned num;
		while ((num = blksiz[i] >> PGLOG2) > n || num == 0) --i;
		page_info[p] = ISFREE | i;
		free_blks[i] = p;
		p += num;
		n -= num;
	}
}

static alose (s)
char *s;
{
	register int p;
	p = halves (0444400, ((int) s));
	while (TRUE) {
		_PSOUT (p);
		_HALTF ();
	}
}

static int *alloc (n)
register int n;
{
	register int s, ns, w, *p, *q;
		/* eliminate bad arguments */
	if (n <= 0 || n > blksiz[TSIZES-1]) return (NULL);
		/* find block size, quickly */
	for (s = 0; n > 128; n = (n + 63) / 64) s += 12;
	s += iblksiz[--n];
	if (s >= NSIZES) {	/* handle page size and up */
		if (blksiz[s] == 0) ++s;
		if (p = palloc (s, TRUE))
			*p = (((n = (int) p) ^ MAGIC) << MSHIFT) | s;
		return (p);
	}
	ns = s;
	while (!(p = (int *) free_blks[ns]))
		if (++ns >= NSIZES) {
			/* get a page and put it on the special free list */
			if (!(p = palloc (NSIZES, TRUE))) return (0);
			n = (int) p;
			*p = ((n ^ MAGIC) << MSHIFT) | ISFREE | NSIZES
			     | (page_info[n >> PGLOG2] & (OFLGMSK | FLGMSK));
			p[1] = 0;
			goto skip;
		}
	if (q = free_blks[ns] = (p[1] & RMASK)) q[1] &= RMASK;
skip:	while (TRUE)	/* split down as far as necessary ... */
		switch (ns - s) {
		case 0:	*p &= ~ISFREE;
			return (p);

		case 1:	if (ns > 2) {
				/* 3:1 or 2:1 split */
				--ns;
				split (p, ns, (ns - 2) - (ns & 1), TRUE);
				continue;
			}
			ns = s;		/* special case: can't split */
			continue;

		even:
		case 2:	ns -= 2;
			split (p, ns, ns, TRUE);
			continue;

		case 3:	if (!(ns & 1)) goto even;
		default:
			--ns;
			p = split (p, ns, (w = (ns - 2) - (ns & 1)), FALSE);
			ns = w;
			continue;

		}
}

static int *split (p, s1, s2, first)
register int *p;
{
	/* split p into two blocks with size codes s1 and s2, returning  */
	/*  first one if first is TRUE, otherwise second one.  Block not */
	/*  returned is put on correct free list.  Note: s1 >= s2. 	 */

	register int s, x, w, n, *q, *qq, *q2;
	q2 = p + blksiz[s1];
	x = 0;
	if (s1 == s2) x |= _EVEN;
	if (!((w = *p) & 1)) x |= _PP2;
	n = (int) q2;
	*q2 = ((n ^ MAGIC) << MSHIFT) | ISFREE | ((w & FLGMSK) << OFSHIFT)
		| x | _RIGHT | s2;
	*p = (w & ~(FLGMSK | SIZMSK)) | x | s1;
	if (first) q = q2;
	else	{
		q = p;
		p = q2;
	}
	s = (*q) & SIZMSK;
	if (qq = q[1] = free_blks[s]) qq[1] |= ((n = (int) q) << 18);
	free_blks[s] = (int) q;
	return (p);
}

static all_gone () {} /* dummy routine to execute if no more core */

int (*nocore)() = all_gone;

static int *palloc (s, must_exist)
{
	register int ns, v, w;
	ns = s;
	while (!(v = free_blks[ns]))
		if (++ns >= TSIZES) {
			(*nocore)();
			return (0);
		}
	if (free_blks[ns] = w = ((page_info[v] & PNEXTMSK) >> PNSHIFT))
		page_info[w] &= ~PPREVMSK;
	while (TRUE)	/* split down as far as necessary ... */
		switch (ns - s) {
		case 0:	page_info[v] &= ~(ISFREE | PPREVMSK | PNEXTMSK);
			return ((int *)(v << PGLOG2));

		case 1:	--ns;	/* 3:1 or 2:1 split */
			psplit (v, ns, (ns - 2) - (ns & 1), TRUE);
			continue;

		even:		/* even split */
		case 2:	ns -= 2;
			psplit (v, ns, ns, TRUE);
			continue;

		case 3:	if (!(ns & 1)) goto even;
		default:	/* 3:1 or 2:1 split */
			--ns;
			v = psplit (v, ns, (w = (ns - 2) - (ns & 1)), FALSE);
			ns = w;
			continue;

		}
}

static int psplit (v, s1, s2, first)
{
	/* split v into two blocks with size codes s1 and s2, returning  */
	/*  first one if first is TRUE, otherwise second one.  Block not */
	/*  returned is put on correct free list.  Note: s1 >= s2. 	 */

	register int s, x, w, ww, w2;
	w2 = v + (blksiz[s1] >> PGLOG2);
	x = 0;
	if (s1 == s2) x |= _EVEN;
	if (!((ww = page_info[v]) & 1)) x |= _PP2;
	page_info[w2] = ((ww & FLGMSK) << OFSHIFT) | ISFREE | x | _RIGHT | s2;
	page_info[v] = (ww & (OFLGMSK | PEXISTS)) | ISFREE | x | s1;
	if (first) w = w2;
	else	{w = v;
		v = w2;
		}
	s = page_info[w] & SIZMSK;
	if (ww = free_blks[s])
		{page_info[ww] |= (w << PPSHIFT);
		page_info[w] |= (ww << PNSHIFT);
		}
	free_blks[s] = w;
	return (v);
}

free (p)
register int *p;
{
	register int n;
	register unsigned w, ww;

	--p;
	if ((w = *p) & ISFREE)
		alose ("FREE: Block Already Free");
	if (((w >> MSHIFT) ^ (n = (int) p)) != MAGIC) {
		alose ("FREE: Bad Block Header");
	}
	*p |= ISFREE;
	while (TRUE) {
		int bs, s, *q;
		if ((s = (w & SIZMSK)) >= NSIZES) {	/* page or bigger ? */
			pfree (p, FALSE);
			return;
		}

		if (w & _RIGHT) {	/* find buddy */
			if (w & _EVEN) bs = s;
			else if (w & _PP2) bs = s + 3;
			else bs = s + 2;
			q = p - blksiz[bs];
		}
		else	{
			q = p + blksiz[s];
			if (w & _EVEN) bs = s;
			else bs = (s - 2) - (s & 1);
		}

		/* to merge, buddy must be free and right size */
		if (((ww = *q) & ISFREE) && ((ww & SIZMSK) == bs)) {
			register int *qq, *prev;

			ww = q[1];
			q[1] = 0;
			if (qq = (int *) (ww & RMASK)) {
				qq[1] &= RMASK;
				qq[1] |= (ww & LMASK);
			}
			if (prev = (int *) (ww >> 18)) {
				prev[1] &= LMASK;
				prev[1] |= (n = (int) qq);
			}
			else	free_blks[bs] = (int) qq;

			if (w & _RIGHT)	{	/* swap so p is first */
				qq = p; p = q; q = qq;
				s = bs;
				w = *p;
			}

			if (w & _EVEN) bs = s + 2;	/* new size, etc. */
			else	bs = s + 1;
			*p = w = (w & (LMASK | OFLGMSK)) | ISFREE | bs
				| (((*q) & OFLGMSK) >> OFSHIFT);
		}
		else	{
			if (q = p[1] = free_blks[s]) 
				q[1] |= ((n = (int) p) << 18);
			free_blks[s] = (int) p;
			return;
		}
	}
}

static pfree (p, unmap)
int *p;
{
	register int m, w, s;

	if ((m = (int) p) & (PGSIZE - 1))
		alose ("PFREE: Not On A Page Boundary");
	if ((w = page_info[m >>= PGLOG2]) & ISFREE)
		alose ("PFREE: Block Already Free");
	w = (page_info[m] |= ISFREE);
	if (unmap) expunge_pages (m, blksiz[w & SIZMSK] >> PGLOG2);
	while (TRUE) {
		register int n, ww, bs;
		s = w & SIZMSK;

		/* see where buddy would be and see if buddy exists */
		if (w & _RIGHT) {
			if (w & _EVEN) bs = s;
			else if (w & _PP2) bs = s + 3;
			else bs = s + 2;
			if (bs >= TSIZES
			    || (n = m - (blksiz[bs] >> PGLOG2)) < lowlim)
				break;	/* no buddy -- don't merge */
		}
		else	{
			if (w & _EVEN) bs = s;
			else bs = (s - 2) - (s & 1);
			n = m + (blksiz[s] >> PGLOG2);
			if ((n + (blksiz[bs] >> PGLOG2)) > highlim) break;
		}

		/* merge if buddy is free and correct size */
		if (((ww = page_info[n]) & ISFREE) && ((ww & SIZMSK) == bs)) {
			register unsigned nn, prev, next;

			page_info[n] &= ~(PPREVMSK | PNEXTMSK);
			prev = ww & PPREVMSK;
			next = ww & PNEXTMSK;
			if (nn = (next >> PNSHIFT)) {
				page_info[nn] &= ~PPREVMSK;
				page_info[nn] |= prev;
			}
			if (prev >>= PPSHIFT) {
				page_info[prev] &= ~PNEXTMSK;
				page_info[prev] |= next;
			}
			else	free_blks[bs] = nn;

			if (w & _RIGHT) {	/* swap so m is first */
				nn = m; m = n; n = nn;
				s = bs;
				w = page_info[m];
			}

			/* calculate new size, update page_info */
			if (w & _EVEN) bs = s + 2;
			else	bs = s + 1;
			page_info[m] = w =
				(w & ~(FLGMSK|SIZMSK)) | ISFREE | bs
				| ((page_info[n] & OFLGMSK) >> OFSHIFT);
		}
		else	break;
	}
	if (w = free_blks[s]) {
		page_info[w] |= (m << PPSHIFT);
		page_info[m] &= ~(PPREVMSK | PNEXTMSK);
		page_info[m] |= (w << PNSHIFT);
	}
	free_blks[s] = m;	
}

int *malloc (n)
{
	register int *p;
	if (!(p = alloc (n + 1)))
		if (!(p = alloc (n + 1)))
			return (NULL);
	fill (++p, n, 0);
	return (p);
}

char *calloc (num,size)
int num,size;
{
	register char *p;
	int n;

	n = size * num;
	if (!(p = alloc (n + 1)))
		if (!(p = alloc (n + 1)))
			return (NULL);
	fill (++p,n,0);
	return (p);
}

int *realloc (p, n)
register int *p;
{
	register int ns, s, ww, l1, l2, *q;
	register unsigned w;
	++n;	/* get real size and real address */
	--p;

	/* consistency checks */
	if (n < 0 || n > blksiz[TSIZES-1]) return (0);
	if ((w = *p) & ISFREE) alose ("REALLOC: Block Is Free");
	if (((w >> MSHIFT) ^ (ww = (int) p)) != MAGIC)
		alose ("REALLOC: Bad Block Header");

	/* get current and desired size codes */
	s = w & SIZMSK;
	ww = n;
	for (ns = 0; ww > 128; ww = (ww + 63) / 64) ns += 12;
	ns += iblksiz[--ww];
	if (blksiz[ns] == 0) ++ns;
	if (s == ns) return (++p);
	else if (ns < s) {
		/* can we alloc and copy over more cheaply ? */
		w = ns;
		while (!free_blks[w])
			if (++w >= s) goto keep;
		goto new;

		/* split and return part */
keep:		if (s > NSIZES) {
			if (ns >= NSIZES) {
				prealloc (p, s, ns);
				return (++p);
			}
			prealloc (p, s, NSIZES);
			s = NSIZES;
		}
		while (TRUE)
			switch (s - ns) {

			case 0:	*p &= ~ISFREE;
				return (++p);

			case 1:	if (ns <= 1) ns = s;
				else	{
					--s;
					split (p, s, (s - 2) - (s & 1), TRUE);
				}
				continue;

			default:
				s -= 2;
				split (p, s, s, TRUE);
				continue;
			}
	}

new:	if (!(q = alloc (n)))
		if (!(q = alloc (n)))
			return (NULL);
	l1 = blksiz[s];
	l2 = blksiz[ns];
	if (ns < s) 
		blt (p + 1, q + 1, l2 - 1);
	else	{
		blt (p + 1, q + 1, l1 - 1);
		fill (q + l1, l2 - l1, 0);
	}
	free (p);
	return (++q);
}

static prealloc (p, s, ns)
register int *p;
{
	register unsigned pp;
	pp = (pp = (int) p) >> PGLOG2;
	while (TRUE)
		switch (s - ns) {

		case 0:	*p &= MMASK;
			*p |= (page_info[pp] &= ~(ISFREE|PPREVMSK|PNEXTMSK));
			return;

		uneven:
		case 1:	--s;
			psplit (pp, s, (s - 2) - (s & 1), TRUE);
			continue;

		even:
		default:
		case 2:	s -= 2;
			psplit (pp, s, s, TRUE);
			continue;

		case 3:	if (ns == NSIZES) goto uneven;
			else goto even;
		}
}

int pg_size () {return (PGSIZE);}

int *pg_get (n)
{
	register int ns;
	if (n <= 0 || n > (3 * NPAGES) / 4) return (NULL);
	n <<= (PGLOG2 - 6);
	for (ns = 12; n > 128; n = (n + 63) / 64) ns += 12;
	ns += iblksiz[--n];
	if (blksiz[ns] == 0) ++ns;
	n = ns;
	while (!free_blks[n])
		if (++n >= TSIZES)
			return (NULL);
	return (palloc (ns, FALSE));
}

pg_ret (p)
int *p;
{
	pfree (p, TRUE);
}

int alocstat (pnalloc, pnbfree)
register int *pnalloc, *pnbfree;
{
	register int pn;
	int wfree;
	pn = lowlim;
	wfree = *pnalloc = *pnbfree = 0;
	while (pn < highlim) {
		register int pcode, sz;
		sz = blksiz[(pcode = page_info[pn]) & SIZMSK];
		if (pcode & ISFREE) {
			(*pnalloc) += sz;
			wfree += sz;
			++(*pnbfree);
		}
		else	{
			(*pnalloc) += sz;
			if (sz == PGSIZE)
				scan_page (pn << PGLOG2, &wfree, pnbfree);
		}
		pn += (sz >> PGLOG2);
	}
	return (wfree);
}

static scan_page (p, pwfree, pfreeb)
register int *p, *pwfree, *pfreeb;
{
	register int *q, sz;
	register unsigned hdr;
	if (p != (q = ((hdr = *p) >> MSHIFT) ^ MAGIC)) return;
	q = p + PGSIZE;
	while (p < q) {
		p += (sz = blksiz[(hdr = *p) & SIZMSK]);
		if (hdr & ISFREE) {
			(*pwfree) += sz;
			++(*pfreeb);
		}
	}
}

int alocflush ()
{
	register int pn, wfree;
	pn = lowlim;
	wfree = 0;
	while (pn < highlim) {
		register int pcode, sz;
		sz = blksiz[(pcode = page_info[pn]) & SIZMSK] >> PGLOG2;
		if (pcode & ISFREE) {
			expunge_pages (pn, sz);
			wfree += (sz << PGLOG2);
		}
		pn += sz;
	}
	return (wfree);
}

static expunge_pages (start, count)
{
	_PMAP (-1, halves (0400000, start), halves (0400000, count));
}

static fill (start, count, val)
int *start, count, val;
{
	if (count > 0) {
		*start = val;
		if (--count > 0) blt (start, start + 1, count);
	}
}

