/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2001--2009 The R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Pulic License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Defn.h>

#define isRaw(x) (TYPEOF(x) == RAWSXP)

/* charToRaw works at byte level, ignores encoding */
SEXP attribute_hidden do_charToRaw(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, x = CAR(args);
    int nc;

    checkArity(op, args);
    if (!isString(x) || LENGTH(x) == 0)
	error(_("argument must be a character vector of length 1"));
    if (LENGTH(x) > 1)
	warning(_("argument should be a character vector of length 1\nall but the first element will be ignored"));
    nc = LENGTH(STRING_ELT(x, 0));
    ans = allocVector(RAWSXP, nc);
    memcpy(RAW(ans), CHAR(STRING_ELT(x, 0)), nc);
    return ans;
}

/* <UTF8>  rawToChar should work at byte level */
SEXP attribute_hidden do_rawToChar(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, x = CAR(args);
    int i, j, nc = LENGTH(x), multiple;

    checkArity(op, args);
    if (!isRaw(x))
	error(_("argument 'x' must be a raw vector"));
    multiple = asLogical(CADR(args));
    if (multiple == NA_LOGICAL)
	error(_("argument 'multiple' must be TRUE or FALSE"));
    if (multiple) {
	char buf[2];
	buf[1] = '\0';
	PROTECT(ans = allocVector(STRSXP, nc));
	for (i = 0; i < nc; i++) {
	    buf[0] = (char) RAW(x)[i];
	    SET_STRING_ELT(ans, i, mkChar(buf));
	}
	/* do we want to copy e.g. names here? */
    } else {
	/* String is not necessarily 0-terminated and may contain nuls.
	   Strip trailing nuls */
	for (i = 0, j = -1; i < nc; i++) if(RAW(x)[i]) j = i;
	nc = j + 1;
	PROTECT(ans = allocVector(STRSXP, 1));
	SET_STRING_ELT(ans, 0,
		       mkCharLenCE((const char *)RAW(x), j+1, CE_NATIVE));
    }
    UNPROTECT(1);
    return ans;
}


SEXP attribute_hidden do_rawShift(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, x = CAR(args);
    int i, shift = asInteger(CADR(args));

    if (!isRaw(x))
	error(_("argument 'x' must be a raw vector"));
    if (shift == NA_INTEGER || shift < -8 || shift > 8)
	error(_("argument 'shift' must be a small integer"));
    PROTECT(ans = duplicate(x));
    if (shift > 0)
	for (i = 0; i < LENGTH(x); i++)
	    RAW(ans)[i] <<= shift;
    else
	for (i = 0; i < LENGTH(x); i++)
	    RAW(ans)[i] >>= (-shift);
    UNPROTECT(1);
    return ans;
}

SEXP attribute_hidden do_rawToBits(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, x = CAR(args);
    int i, j = 0, k;
    unsigned int tmp;

    if (!isRaw(x))
	error(_("argument 'x' must be a raw vector"));
    PROTECT(ans = allocVector(RAWSXP, 8*LENGTH(x)));
    for (i = 0; i < LENGTH(x); i++) {
	tmp = (unsigned int) RAW(x)[i];
	for (k = 0; k < 8; k++, tmp >>= 1)
	    RAW(ans)[j++] = tmp & 0x1;
    }
    UNPROTECT(1);
    return ans;
}

SEXP attribute_hidden do_intToBits(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, x;
    int i, j = 0, k;
    unsigned int tmp;
    
    PROTECT(x = coerceVector(CAR(args), INTSXP));
    if (!isInteger(x))
	error(_("argument 'x' must be an integer vector"));
    PROTECT(ans = allocVector(RAWSXP, 32*LENGTH(x)));
    for (i = 0; i < LENGTH(x); i++) {
	tmp = (unsigned int) INTEGER(x)[i];
	for (k = 0; k < 32; k++, tmp >>= 1)
	    RAW(ans)[j++] = tmp & 0x1;
    }
    UNPROTECT(2);
    return ans;
}

SEXP attribute_hidden do_packBits(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, x = CAR(args), stype = CADR(args);
    Rboolean useRaw;
    int i, j, k, fac, len = LENGTH(x), slen;
    unsigned int itmp;
    Rbyte btmp;

    if (TYPEOF(x) != RAWSXP && TYPEOF(x) != LGLSXP && TYPEOF(x) != INTSXP)
	error(_("argument 'x' must be raw, integer or logical"));
    if (!isString(stype)  || LENGTH(stype) != 1)
	error(_("argument 'type' must be a character string"));
    useRaw = strcmp(CHAR(STRING_ELT(stype, 0)), "integer");
    fac = useRaw ? 8 : 32;
    if (len% fac)
	error(_("argument 'x' must be a multiple of %d long"), fac);
    slen = len/fac;
    PROTECT(ans = allocVector(useRaw ? RAWSXP : INTSXP, slen));
    for (i = 0; i < slen; i++)
	if (useRaw) {
	    btmp = 0;
	    for (k = 7; k >= 0; k--) {
		btmp <<= 1;
		if (isRaw(x))
		    btmp |= RAW(x)[8*i + k] & 0x1;
		else if (isLogical(x) || isInteger(x)) {
		    j = INTEGER(x)[8*i+k];
		    if (j == NA_INTEGER)
			error(_("argument 'x' must not contain NAs"));
		    btmp |= j & 0x1;
		}
	    }
	    RAW(ans)[i] = btmp;
	} else {
	    itmp = 0;
	    for (k = 31; k >= 0; k--) {
		itmp <<= 1;
		if (isRaw(x))
		    itmp |= RAW(x)[32*i + k] & 0x1;
		else if (isLogical(x) || isInteger(x)) {
		    j = INTEGER(x)[32*i+k];
		    if (j == NA_INTEGER)
			error(_("argument 'x' must not contain NAs"));
		    itmp |= j & 0x1;
		}
	    }
	    INTEGER(ans)[i] = (int) itmp;
	}
    UNPROTECT(1);
    return ans;
}

static int mbrtoint(int *w, const char *s)
{
    unsigned int byte;
    byte = *((unsigned char *)s);

    if (byte == 0) {
	*w = 0;
	return 0;
    } else if (byte < 0xC0) {
	*w = (int) byte;
	return 1;
    } else if (byte < 0xE0) {
	if (!s[1]) return -2;
	if ((s[1] & 0xC0) == 0x80) {
	    *w = (int) (((byte & 0x1F) << 6) | (s[1] & 0x3F));
	    return 2;
	} else return -1;
    } else if (byte < 0xF0) {
	if (!s[1] || !s[2]) return -2;
	if (((s[1] & 0xC0) == 0x80) && ((s[2] & 0xC0) == 0x80)) {
	    *w = (int) (((byte & 0x0F) << 12)
			| ((s[1] & 0x3F) << 6) | (s[2] & 0x3F));
	    byte = *w;
	    if (byte >= 0xD800 && byte <= 0xDFFF) return -1; /* surrogate */
	    if (byte == 0xFFFE || byte == 0xFFFF) return -1;
	    return 3;
	} else return -1;
    } else if (byte < 0xF8) {
	if (!s[1] || !s[2] || !s[3]) return -2;
	if (((s[1] & 0xC0) == 0x80)
	    && ((s[2] & 0xC0) == 0x80)
	    && ((s[3] & 0xC0) == 0x80)) {
	    *w = (int) (((byte & 0x07) << 18)
			| ((s[1] & 0x3F) << 12)
			| ((s[2] & 0x3F) << 6)
			| (s[3] & 0x3F));
	    byte = *w;
	    return 4;
	} else return -1;
    } else if (byte < 0xFC) {
	if (!s[1] || !s[2] || !s[3] || !s[4]) return -2;
	if (((s[1] & 0xC0) == 0x80)
	    && ((s[2] & 0xC0) == 0x80)
	    && ((s[3] & 0xC0) == 0x80)
	    && ((s[4] & 0xC0) == 0x80)) {
	    *w = (int) (((byte & 0x03) << 24)
			| ((s[1] & 0x3F) << 18)
			| ((s[2] & 0x3F) << 12)
			| ((s[3] & 0x3F) << 6)
			| (s[4] & 0x3F));
	    byte = *w;
	    return 5;
	} else return -1;
    } else {
	if (!s[1] || !s[2] || !s[3] || !s[4] || !s[5]) return -2;
	if (((s[1] & 0xC0) == 0x80)
	    && ((s[2] & 0xC0) == 0x80)
	    && ((s[3] & 0xC0) == 0x80)
	    && ((s[4] & 0xC0) == 0x80)
	    && ((s[5] & 0xC0) == 0x80)) {
	    *w = (int) (((byte & 0x01) << 30)
			| ((s[1] & 0x3F) << 24)
			| ((s[2] & 0x3F) << 18)
			| ((s[3] & 0x3F) << 12)
			| ((s[5] & 0x3F) << 6)
			| (s[5] & 0x3F));
	    byte = *w;
	    return 6;
	} else return -1;
    }
    /* return -2; not reached */
}

SEXP attribute_hidden do_utf8ToInt(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, x = CAR(args);
    int i, j, nc, *ians, tmp, used = 0; /* -Wall */
    const char *s = CHAR(STRING_ELT(x, 0));

    checkArity(op, args);
    if (!isString(x) || LENGTH(x) == 0)
	error(_("argument must be a character vector of length 1"));
    if (LENGTH(x) > 1)
	warning(_("argument should be a character vector of length 1\nall but the first element will be ignored"));
    if (STRING_ELT(x, 0) == NA_STRING) return ScalarInteger(NA_INTEGER);
    nc = LENGTH(STRING_ELT(x, 0)); /* ints will be shorter */
    ians = (int *) R_alloc(nc, sizeof(int));
    for (i = 0, j = 0; i < nc; i++) {
	used = mbrtoint(&tmp, s);
	if (used <= 0) break;
	ians[j++] = tmp;
	s += used;
    }
    if (used < 0) error("invalid UTF-8 string");
    ans = allocVector(INTSXP, j);
    memcpy(INTEGER(ans), ians, sizeof(int) * j);
    return ans;
}

/* based on pcre.c */
static const int utf8_table1[] =
    { 0x7f, 0x7ff, 0xffff, 0x1fffff, 0x3ffffff, 0x7fffffff};
static const int utf8_table2[] = { 0, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc};

static size_t inttomb(char *s, const int wc)
{
    register int i, j;
    unsigned int cvalue = wc;
    char buf[10], *b;

    b = s ? s : buf;
    if (cvalue == 0) {*b = 0; return 0;}
    for (i = 0; i < sizeof(utf8_table1)/sizeof(int); i++)
	if (cvalue <= utf8_table1[i]) break;
    b += i;
    for (j = i; j > 0; j--) {
	*b-- = 0x80 | (cvalue & 0x3f);
	cvalue >>= 6;
    }
    *b = utf8_table2[i] | cvalue;
    return i + 1;
}

#include <R_ext/RS.h>  /* for Calloc/Free */

SEXP attribute_hidden do_intToUtf8(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, x;
    int i, nc, multiple, len, used;
    char buf[10], *tmp;

    checkArity(op, args);
    PROTECT(x = coerceVector(CAR(args), INTSXP));
    if (!isInteger(x))
	error(_("argument 'x' must be an integer vector"));
    nc = LENGTH(x);
    multiple = asLogical(CADR(args));
    if (multiple == NA_LOGICAL)
	error(_("argument 'multiple' must be TRUE or FALSE"));
    if (multiple) {
	PROTECT(ans = allocVector(STRSXP, nc));
	for (i = 0; i < nc; i++) {
	    if (INTEGER(x)[i] == NA_INTEGER)
		SET_STRING_ELT(ans, i, NA_STRING);
	    else {
		used = inttomb(buf, INTEGER(x)[i]);
		buf[used] = '\0';
		SET_STRING_ELT(ans, i, mkCharCE(buf, CE_UTF8));
	    }
	}
	/* do we want to copy e.g. names here? */
    } else {
	Rboolean haveNA = FALSE;
	/* Note that this gives zero length for input '0', so it is omitted */
	for (i = 0, len = 0; i < nc; i++) {
	    if (INTEGER(x)[i] == NA_INTEGER) { haveNA = TRUE; break; }
	    len += inttomb(NULL, INTEGER(x)[i]);
	}
	if (haveNA) {
	    PROTECT(ans = allocVector(STRSXP, 1));
	    SET_STRING_ELT(ans, 0, NA_STRING);
	    UNPROTECT(2);
	    return ans;
	}
	if (len >= 10000) {
	    tmp = Calloc(len+1, char);
	} else {
	    tmp = alloca(len+1); tmp[len] = '\0';
	    R_CheckStack();
	}
	for (i = 0, len = 0; i < nc; i++) {
	    used = inttomb(buf, INTEGER(x)[i]);
	    strncpy(tmp + len, buf, used);
	    len += used;
	}
	PROTECT(ans = allocVector(STRSXP, 1));
	SET_STRING_ELT(ans, 0, mkCharLenCE(tmp, len, CE_UTF8));
	if(len >= 10000) Free(tmp);
    }
    UNPROTECT(2);
    return ans;
}
