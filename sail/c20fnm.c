# define FNSIZE 100
# define QUOTE 022 /* ^V */

/*

	TOPS-20 filename cluster

	components:
		DEV:<DIR>NAME.TYP.GEN;ATTR
		All components manipulated without punctuation,
			except ATTR.

*/

/* declarations of internal procedures */
char *fnscan(), *fnsmove();

/**********************************************************************

	FNPARSE - Parse file name into components.

**********************************************************************/

fnparse (old, dv, dir, nm, typ, gen, attr)
	char *old, *dv, *dir, *nm, *typ, *gen, *attr;

	{char *p;
	*dv = *dir = *nm = *typ = *gen = *attr = 0;
	while (old[0] == ' ') ++old;
	if (old[0] != '<')
		{p = fnscan (old, ":");
		if (*p == ':')
			{fnsmove (old, p, dv);
			old = p+1;
			}
		}
	if (old[0] == '<')
		{p = fnscan (old+1, ">");
		fnsmove (old+1, p, dir);
		if (*p == 0) return;
		old = p+1;
		}
	p = fnscan (old, ".;");
	fnsmove (old, p, nm);
	old = p+1;
	if (*p == '.')
		{p = fnscan (old, ".;");
		fnsmove (old, p, typ);
		old = p+1;
		if (*p == '.')
			{p = fnscan (old, ";");
			fnsmove (old, p, gen);
			}
		}
	if (*p == ';')
		fnsmove (p, 0, attr);
	}

/**********************************************************************

	FNGxx - Extrace a given component.

**********************************************************************/

char *fngdv (old, buf)
	char *old, *buf;

	{char temp[FNSIZE];
	fnparse (old, buf, temp, temp, temp, temp, temp);
	return (buf);
	}

char *fngdr (old, buf)
	char *old, *buf;

	{char temp[FNSIZE];
	fnparse (old, temp, buf, temp, temp, temp, temp);
	return (buf);
	}

char *fngnm (old, buf)
	char *old, *buf;

	{char temp[FNSIZE];
	fnparse (old, temp, temp, buf, temp, temp, temp);
	return (buf);
	}

char *fngtp (old, buf)
	char *old, *buf;

	{char temp[FNSIZE];
	fnparse (old, temp, temp, temp, buf, temp, temp);
	return (buf);
	}

char *fnggn (old, buf)
	char *old, *buf;

	{char temp[FNSIZE];
	fnparse (old, temp, temp, temp, temp, buf, temp);
	return (buf);
	}

char *fngat (old, buf)
	char *old, *buf;

	{char temp[FNSIZE];
	fnparse (old, temp, temp, temp, temp, temp, buf);
	return (buf);
	}

/**********************************************************************

	FNCONS - Construct a file name from its components.

**********************************************************************/

char *fncons (buf, dv, dir, nm, typ, gen, attr)
	char *buf, *dv, *dir, *nm, *typ, *gen, *attr;

	{if (dv && dv[0])
		{buf = fnsmove (dv, 0, buf);
		*buf++ = ':';
		}
	if (dir && dir[0])
		{*buf++ = '<';
		buf = fnsmove (dir, 0, buf);
		*buf++ = '>';
		}
	if (nm) buf = fnsmove (nm, 0, buf);
	if (typ && typ[0])
		{*buf++ = '.';
		buf = fnsmove (typ, 0, buf);
		}
	if (gen && gen[0])
		{*buf++ = '.';
		buf = fnsmove (gen, 0, buf);
		}
	if (attr && attr[0])
		{if (attr[0] != ';') *buf++ = ';';
		fnsmove (attr, 0, buf);
		}
	return (buf);
	}

/**********************************************************************

	FNSDF - Make a new file name with specified defaults.
	Nonzero arguments specify defaults; the corresponding
	components will be set if they are null.

**********************************************************************/

char *fnsdf (buf, old, dv, dir, nm, typ, gen, attr)
	char *old, *buf, *dv, *dir, *nm, *typ, *gen, *attr;

	{char odv[FNSIZE], odir[FNSIZE], onm[FNSIZE],
	      otyp[FNSIZE], ogen[FNSIZE], oattr[FNSIZE];
	fnparse (old, odv, odir, onm, otyp, ogen, oattr);
	if (dv && odv[0]==0) fnsmove (dv, 0, odv);
	if (dir && odir[0]==0) fnsmove (dir, 0, odir);
	if (nm && onm[0]==0) fnsmove (nm, 0, onm);
	if (typ && otyp[0]==0) fnsmove (typ, 0, otyp);
	if (gen && ogen[0]==0) fnsmove (gen, 0, ogen);
	if (attr && oattr[0]==0) fnsmove (attr, 0, oattr);
	fncons (buf, odv, odir, onm, otyp, ogen, oattr);
	return (buf);
	}

/**********************************************************************

	FNSFD - Make a new file name with specified components.
	Nonzero arguments specify components; the corresponding
	components of the file name will be set.

**********************************************************************/

char *fnsfd (buf, old, dv, dir, nm, typ, gen, attr)
	char *old, *buf, *dv, *dir, *nm, *typ, *gen, *attr;

	{char odv[FNSIZE], odir[FNSIZE], onm[FNSIZE],
	      otyp[FNSIZE], ogen[FNSIZE], oattr[FNSIZE];
	fnparse (old, odv, odir, onm, otyp, ogen, oattr);
	if (dv) fnsmove (dv, 0, odv);
	if (dir) fnsmove (dir, 0, odir);
	if (nm) fnsmove (nm, 0, onm);
	if (typ) fnsmove (typ, 0, otyp);
	if (gen) fnsmove (gen, 0, ogen);
	if (attr) fnsmove (attr, 0, oattr);
	fncons (buf, odv, odir, onm, otyp, ogen, oattr);
	return (buf);
	}

/* Internal procedures */

char *fnscan (p, m)
	char *p, *m;

	{while (1)
		{int c;
		char *q;
		c = *p;
		if (c == QUOTE)
			{c = *++p;
			if (c == 0) return (p);
			++p;
			continue;
			}
		q = m;
		while (1)
			{if (*q == c) return (p);
			if (*q == 0) break;
			++q;
			}
		++p;
		}
	}

/*
 * Internal routine: FNSMOVE
 *
 * Move characters starting with *FIRST up to (but not
 * including) *AFTER into *DEST.  If AFTER is null, then
 * move characters until a NUL byte is encountered.
 * Always terminate the destination with a NUL byte
 * and return a pointer to the terminating NUL.
 *
 */

char *fnsmove (first, after, dest)
	char *first, *after, *dest;

	{while (!after || first < after)
		if (!(*dest++ = *first++)) return (dest-1);
	*dest = 0;
	return (dest);
	}
   