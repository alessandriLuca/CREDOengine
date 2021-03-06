/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2011  The R Development Core Team
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

/*

Support for UTF-8-encoded strings in non-UTF-8 locales
======================================================

strsplit grep [g]sub [g]regexpr
  handle UTF-8 directly if fixed/perl = TRUE, via wchar_t for extended

  We currrently translate latin1 strings to the native encoding.
  We could use UTF-8 in a non-latin1-locale instead.

*/

/* It is possible to use TRE for fixed = TRUE.
   The main benefit would be code simplification: however, the
   special-purpose code is substantially faster, so we no longer
   plan to do so.
*/

/* PCRE supports only single-byte locales and UTF-8, so we convert
   inputs in all other MBCS locales to UTF-8.

   In [g]sub and [g]regexpr we need to know match postions in
   characters.  To avoid yet more cases we handle all MBCS locales in
   wchar in ERE for those functions.  (Byte positions suffice for
   [g]sub(fixed = TRUE), and [g]regexpr needs to convert to char
   positions for all MBCSs.)
*/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <Defn.h>
#include <R_ext/RS.h>  /* for Calloc/Free */
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>    /* for wctrans_t */

/* As from TRE 0.8.0, tre.h replaces regex.h */
#include <tre/tre.h>

/* Some systems using --with-system-pcre might have pcre headers in
   a subdirectory -- not seen recently.
*/
#ifdef HAVE_PCRE_PCRE_H
# include <pcre/pcre.h>
#else
# include <pcre.h>
#endif

#ifndef MAX
# define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef isRaw
#define isRaw(x) (TYPEOF(x) == RAWSXP)
#endif

/* we allow pat == NULL if the regex cannot be safely expressed
   as a string (e.g., when using grepRaw) */
static void reg_report(int rc,  regex_t *reg, const char *pat)
{
    char errbuf[1001];
    tre_regerror(rc, reg, errbuf, 1001);
    if (pat)
	error(_("invalid regular expression '%s', reason '%s'"), pat, errbuf);
    else
	error(_("invalid regular expression, reason '%s'"), errbuf);
}

/* FIXME: make more robust, and public */
static SEXP mkCharWLen(const wchar_t *wc, int nc)
{
    int nb; char *xi; wchar_t *wt;
    wt = (wchar_t *) alloca((nc+1)*sizeof(wchar_t));
    R_CheckStack();
    wcsncpy(wt, wc, nc); wt[nc] = 0;
    nb = wcstoutf8(NULL, wt, nc);
    xi = (char *) alloca((nb+1)*sizeof(char));
    R_CheckStack();
    wcstoutf8(xi, wt, nb + 1);
    return mkCharLenCE(xi, nb, CE_UTF8);
}

static SEXP mkCharW(const wchar_t *wc)
{
    int nb = wcstoutf8(NULL, wc, 0);
    char *xi = (char *) Calloc(nb+1, char);
    SEXP ans;
    wcstoutf8(xi, wc, nb + 1);
    ans = mkCharCE(xi, CE_UTF8);
    Free(xi);
    return ans;
}


/* strsplit is going to split the strings in the first argument into
 * tokens depending on the second argument. The characters of the second
 * argument are used to split the first argument.  A list of vectors is
 * returned of length equal to the input vector x, each element of the
 * list is the collection of splits for the corresponding element of x.
*/

SEXP attribute_hidden do_strsplit(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP args0 = args, ans, tok, x;
    int i, itok, j, len, tlen, ntok;
    int fixed_opt, perl_opt, useBytes;
    char *pt = NULL; wchar_t *wpt = NULL;
    const char *buf, *split = "", *bufp;
    const unsigned char *tables = NULL;
    Rboolean use_UTF8 = FALSE, haveBytes = FALSE;
    const void *vmax, *vmax2;

    checkArity(op, args);
    x = CAR(args); args = CDR(args);
    tok = CAR(args); args = CDR(args);
    fixed_opt = asLogical(CAR(args)); args = CDR(args);
    perl_opt = asLogical(CAR(args)); args = CDR(args);
    useBytes = asLogical(CAR(args));
    if (fixed_opt == NA_INTEGER) fixed_opt = 0;
    if (perl_opt == NA_INTEGER) perl_opt = 0;
    if (useBytes == NA_INTEGER) useBytes = 0;
    if (fixed_opt && perl_opt) {
	warning(_("argument '%s' will be ignored"), "perl = TRUE");
	perl_opt = 0;
    }

    if (!isString(x) || !isString(tok)) error(_("non-character argument"));


    len = LENGTH(x);
    tlen = LENGTH(tok);

    /* treat split = NULL as split = "" */
    if (!tlen) { tlen = 1; SETCADR(args0, tok = mkString("")); }

    if (!useBytes) {
	for (i = 0; i < tlen; i++)
	    if (IS_BYTES(STRING_ELT(tok, i))) {
		haveBytes = TRUE; break;
	    }
	if (!haveBytes)
	    for (i = 0; i < len; i++)
		if (IS_BYTES(STRING_ELT(x, i))) {
		    haveBytes = TRUE;
		    break;
		}
	if (haveBytes) {
	    useBytes = TRUE;
	} else {
	    if (perl_opt && mbcslocale) use_UTF8 = TRUE;
	    if (!use_UTF8)
		for (i = 0; i < tlen; i++)
		    if (IS_UTF8(STRING_ELT(tok, i))) {
			use_UTF8 = TRUE; break;
		    }
	    if (!use_UTF8)
		for (i = 0; i < len; i++)
		    if (IS_UTF8(STRING_ELT(x, i))) {
			use_UTF8 = TRUE;
			break;
		    }
	}
    }

    /* group by token for efficiency with PCRE/TRE versions */
    PROTECT(ans = allocVector(VECSXP, len));
    vmax = vmaxget();
    for (itok = 0; itok < tlen; itok++) {
	SEXP this = STRING_ELT(tok, itok);

	if (this == NA_STRING) { /* NA token doesn't split */
	    for (i = itok; i < len; i += tlen)
		SET_VECTOR_ELT(ans, i, ScalarString(STRING_ELT(x, i)));
	    continue;
	} else if (!CHAR(this)[0]) { /* empty */
	    vmax2 = vmaxget();
	    for (i = itok; i < len; i += tlen) {
		SEXP t;
		if (STRING_ELT(x, i) == NA_STRING) {
		    SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
		    continue;
		}
		if (useBytes)
		    buf = CHAR(STRING_ELT(x, i));
		else if (use_UTF8) {
		    buf = translateCharUTF8(STRING_ELT(x, i));
		    if (!utf8Valid(buf)) {
			warning(_("input string %d is invalid UTF-8"), i+1);
			SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
			continue;
		    }
		} else {
		    buf = translateChar(STRING_ELT(x, i));
		    if (mbcslocale && !mbcsValid(buf)) {
			warning(_("input string %d is invalid in this locale"), i+1);
			SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
			continue;
		    }
		}
		if (!useBytes && (use_UTF8 || mbcslocale) && !strIsASCII(buf)) {
		/* split into individual characters (not bytes) */
		    char bf[20 /* > MB_CUR_MAX */];
		    const char *p = buf;
		    int used;
		    mbstate_t mb_st;

		    if (use_UTF8) {
			for (ntok = 0; *p; p += used, ntok++)
			    used = utf8clen(*p);
			p = buf;
			PROTECT(t = allocVector(STRSXP, ntok));
			for (j = 0; j < ntok; j++, p += used) {
			    used = utf8clen(*p);
			    memcpy(bf, p, used); bf[used] = '\0';
			    SET_STRING_ELT(t, j, mkCharCE(bf, CE_UTF8));
			}
		    } else if ((ntok = mbstowcs(NULL, buf, 0)) < 0) {
			PROTECT(t = ScalarString(NA_STRING));
		    } else {
			mbs_init(&mb_st);
			PROTECT(t = allocVector(STRSXP, ntok));
			for (j = 0; j < ntok; j++, p += used) {
			    /* This is valid as we have already checked */
			    used = mbrtowc(NULL, p, MB_CUR_MAX, &mb_st);
			    memcpy(bf, p, used); bf[used] = '\0';
			    SET_STRING_ELT(t, j, markKnown(bf, STRING_ELT(x, i)));
			}
		    }
		} else {
		    /* useBytes or ASCII or 
		       single-byte locale and not marked as UTF-8 */
		    char bf[2];
		    ntok = strlen(buf);
		    PROTECT(t = allocVector(STRSXP, ntok));
		    bf[1] = '\0';
		    for (j = 0; j < ntok; j++) {
			bf[0] = buf[j];
			SET_STRING_ELT(t, j, markKnown(bf, STRING_ELT(x, i)));
		    }
		}
		SET_VECTOR_ELT(ans, i, t);
		UNPROTECT(1);
		vmaxset(vmax2);
	    }
	} else if (fixed_opt) {
	    const char *laststart, *ebuf;
	    int slen;
	    if (useBytes)
		split = CHAR(STRING_ELT(tok, itok));
	    else if (use_UTF8) {
		split = translateCharUTF8(STRING_ELT(tok, itok));
		if (!utf8Valid(split))
		    error(_("'split' string %d is invalid UTF-8"), itok+1);
	    } else {
		split = translateChar(STRING_ELT(tok, itok));
		if (mbcslocale && !mbcsValid(split))
		    error(_("'split' string %d is invalid in this locale"), 
			  itok+1);
	    }
	    slen = strlen(split);

	    vmax2 = vmaxget();
	    for (i = itok; i < len; i += tlen) {
		SEXP t;
		if (STRING_ELT(x, i) == NA_STRING) {
		    SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
		    continue;
		}

		if (useBytes)
		    buf = CHAR(STRING_ELT(x, i));
		else if (use_UTF8) {
		    buf = translateCharUTF8(STRING_ELT(x, i));
		    if (!utf8Valid(buf)) {
			warning(_("input string %d is invalid UTF-8"), i+1);
			SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
			continue;
		    }
		} else {
		    buf = translateChar(STRING_ELT(x, i));
		    if (mbcslocale && !mbcsValid(buf)) {
			warning(_("input string %d is invalid in this locale"), i+1);
			SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
			continue;
		    }
		}
		/* find out how many splits there will be */
		ntok = 0;
		/* This is UTF-8 safe since it compares whole strings */
		laststart = buf;
		ebuf = buf + strlen(buf);
		for (bufp = buf; bufp < ebuf; bufp++) {
		    if ((slen == 1 && *bufp != *split) ||
			(slen > 1 && strncmp(bufp, split, slen))) continue;
		    ntok++;
		    bufp += MAX(slen - 1, 0);
		    laststart = bufp+1;
		}
		bufp = laststart;
		SET_VECTOR_ELT(ans, i,
			       t = allocVector(STRSXP, ntok + (*bufp ? 1 : 0)));
		/* and fill with the splits */
		laststart = bufp = buf;
		pt = Realloc(pt, strlen(buf)+1, char);
		for (j = 0; j < ntok; j++) {
		    /* This is UTF-8 safe since it compares whole
		       strings, but <MBCS-FIXME> it would be more
		       efficient to skip along by chars.
		    */
		    for (; bufp < ebuf; bufp++) {
			if ((slen == 1 && *bufp != *split) ||
			    (slen > 1 && strncmp(bufp, split, slen))) continue;
			if (slen) {
			    strncpy(pt, laststart, bufp - laststart);
			    pt[bufp - laststart] = '\0';
			} else {
			    pt[0] = *bufp; pt[1] ='\0';
			}
			bufp += MAX(slen-1, 0);
			laststart = bufp+1;
			if (use_UTF8)
			    SET_STRING_ELT(t, j, mkCharCE(pt, CE_UTF8));
			else
			    SET_STRING_ELT(t, j, markKnown(pt, STRING_ELT(x, i)));
			break;
		    }
		    bufp = laststart;
		}
		if (*bufp) {
		    if (use_UTF8)
		        SET_STRING_ELT(t, ntok, mkCharCE(bufp, CE_UTF8));
		    else
		    	SET_STRING_ELT(t, ntok, markKnown(bufp, STRING_ELT(x, i)));
		}
		vmaxset(vmax2);
	    }
	} else if (perl_opt) {
	    pcre *re_pcre;
	    pcre_extra *re_pe;
	    int erroffset, ovector[30];
	    const char *errorptr;
	    int options = 0;

	    if (use_UTF8) options = PCRE_UTF8;
	    if (useBytes)
		split = CHAR(STRING_ELT(tok, itok));
	    else if (use_UTF8) {
		split = translateCharUTF8(STRING_ELT(tok, itok));
		if (!utf8Valid(split))
		    error(_("'split' string %d is invalid UTF-8"), itok+1);
	    } else {
		split = translateChar(STRING_ELT(tok, itok));
		if (mbcslocale && !mbcsValid(split))
		    error(_("'split' string %d is invalid in this locale"), itok+1);
	    }

	    if (!tables) tables = pcre_maketables();
	    re_pcre = pcre_compile(split, options,
				   &errorptr, &erroffset, tables);
	    if (!re_pcre) {
		if (errorptr)
		    warning(_("PCRE pattern compilation error\n\t'%s'\n\tat '%s'\n"),
			    errorptr, split+erroffset);
		error(_("invalid split pattern '%s'"), split);
	    }
	    re_pe = pcre_study(re_pcre, 0, &errorptr);
	    if (errorptr)
		warning(_("PCRE pattern study error\n\t'%s'\n"), errorptr);

	    vmax2 = vmaxget();
	    for (i = itok; i < len; i += tlen) {
		SEXP t;
		if (STRING_ELT(x, i) == NA_STRING) {
		    SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
		    continue;
		}

		if (useBytes)
		    buf = CHAR(STRING_ELT(x, i));
		else if (use_UTF8) {
		    buf = translateCharUTF8(STRING_ELT(x, i));
		    if (!utf8Valid(buf)) {
			warning(_("input string %d is invalid UTF-8"), i+1);
			SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
			continue;
		    }
		} else {
		    buf = translateChar(STRING_ELT(x, i));
		    if (mbcslocale && !mbcsValid(buf)) {
			warning(_("input string %d is invalid in this locale"), i+1);
			SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
			continue;
		    }
		}
		/* find out how many splits there will be */
		ntok = 0;
		bufp = buf;
		if (*bufp) {
		    while(pcre_exec(re_pcre, re_pe, bufp, strlen(bufp), 0, 0,
				    ovector, 30) >= 0) {
			/* Empty matches get the next char, so move by one. */
			bufp += MAX(ovector[1], 1);
			ntok++;
			if (*bufp == '\0')
			    break;
		    }
		}
		SET_VECTOR_ELT(ans, i,
			       t = allocVector(STRSXP, ntok + (*bufp ? 1 : 0)));
		/* and fill with the splits */
		bufp = buf;
		pt = Realloc(pt, strlen(buf)+1, char);
		for (j = 0; j < ntok; j++) {
		    pcre_exec(re_pcre, re_pe, bufp, strlen(bufp), 0, 0,
			      ovector, 30);
		    if (ovector[1] > 0) {
			/* Match was non-empty. */
			if (ovector[0] > 0)
			    strncpy(pt, bufp, ovector[0]);
			pt[ovector[0]] = '\0';
			bufp += ovector[1];
		    } else {
			/* Match was empty. */
			pt[0] = *bufp;
			pt[1] = '\0';
			bufp++;
		    }
		    if (use_UTF8)
			SET_STRING_ELT(t, j, mkCharCE(pt, CE_UTF8));
		    else
			SET_STRING_ELT(t, j, markKnown(pt, STRING_ELT(x, i)));
		}
		if (*bufp) {
		    if (use_UTF8)
		        SET_STRING_ELT(t, ntok, mkCharCE(bufp, CE_UTF8));
		    else
		    	SET_STRING_ELT(t, ntok, markKnown(bufp, STRING_ELT(x, i)));
		}
		vmaxset(vmax2);
	    }
	    pcre_free(re_pe);
	    pcre_free(re_pcre);
	} else if (!useBytes && use_UTF8) { /* ERE in wchar_t */
	    regex_t reg;
	    regmatch_t regmatch[1];
	    int rc;
	    int cflags = REG_EXTENDED;
	    const wchar_t *wbuf, *wbufp, *wsplit;

	    /* Careful: need to distinguish empty (rm_eo == 0) from
	       non-empty (rm_eo > 0) matches.  In the former case, the
	       token extracted is the next character.  Otherwise, it is
	       everything before the start of the match, which may be
	       the empty string (not a ``token'' in the strict sense).
	    */

	    wsplit = wtransChar(STRING_ELT(tok, itok));
	    if ((rc = tre_regwcomp(&reg, wsplit, cflags)))
		reg_report(rc, &reg, translateChar(STRING_ELT(tok, itok)));

	    vmax2 = vmaxget();
	    for (i = itok; i < len; i += tlen) {
		SEXP t;
		if (STRING_ELT(x, i) == NA_STRING) {
		    SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
		    continue;
		}
		wbuf = wtransChar(STRING_ELT(x, i));

		/* find out how many splits there will be */
		ntok = 0;
		wbufp = wbuf;
		if (*wbufp) {
		    while(tre_regwexec(&reg, wbufp, 1, regmatch, 0) == 0) {
			/* Empty matches get the next char, so move by one. */
			wbufp += MAX(regmatch[0].rm_eo, 1);
			ntok++;
			if (!*wbufp) break;
		    }
		}
		SET_VECTOR_ELT(ans, i,
			       t = allocVector(STRSXP, ntok + (*wbufp ? 1 : 0)));
		/* and fill with the splits */
		wbufp = wbuf;
		wpt = Realloc(wpt, wcslen(wbuf)+1, wchar_t);
		for (j = 0; j < ntok; j++) {
		    tre_regwexec(&reg, wbufp, 1, regmatch, 0);
		    if (regmatch[0].rm_eo > 0) {
			/* Match was non-empty. */
			if (regmatch[0].rm_so > 0)
			    wcsncpy(wpt, wbufp, regmatch[0].rm_so);
			wpt[regmatch[0].rm_so] = 0;
			wbufp += regmatch[0].rm_eo;
		    } else {
			/* Match was empty. */
			wpt[0] = *wbufp;
			wpt[1] = 0;
			wbufp++;
		    }
		    SET_STRING_ELT(t, j, 
				   mkCharWLen(wpt, regmatch[0].rm_so));
		}
		if (*wbufp)
		    SET_STRING_ELT(t, ntok,
				   mkCharWLen(wbufp, wcslen(wbufp)));
		vmaxset(vmax2);
	    }
	    tre_regfree(&reg);
	} else { /* ERE in normal chars -- single byte or MBCS */
	    regex_t reg;
	    regmatch_t regmatch[1];
	    int rc;
	    int cflags = REG_EXTENDED;

	    /* Careful: need to distinguish empty (rm_eo == 0) from
	       non-empty (rm_eo > 0) matches.  In the former case, the
	       token extracted is the next character.  Otherwise, it is
	       everything before the start of the match, which may be
	       the empty string (not a ``token'' in the strict sense).
	    */
	    /* never use_UTF8 */
	    if (useBytes)
		split = CHAR(STRING_ELT(tok, itok));
	    else {
		split = translateChar(STRING_ELT(tok, itok));
		if (mbcslocale && !mbcsValid(split))
		    error(_("'split' string %d is invalid in this locale"), itok+1);
	    }
	    if ((rc = tre_regcomp(&reg, split, cflags)))
		reg_report(rc, &reg, split);

	    vmax2 = vmaxget();
	    for (i = itok; i < len; i += tlen) {
		SEXP t;
		if (STRING_ELT(x, i) == NA_STRING) {
		    SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
		    continue;
		}
		/* never use_UTF8 */
		if (useBytes)
		    buf = CHAR(STRING_ELT(x, i));
		else {
		    buf = translateChar(STRING_ELT(x, i));
		    if (mbcslocale && !mbcsValid(buf)) {
			warning(_("input string %d is invalid in this locale"), i+1);
			SET_VECTOR_ELT(ans, i, ScalarString(NA_STRING));
			continue;
		    }
		}

		/* find out how many splits there will be */
		ntok = 0;
		bufp = buf;
		if (*bufp) {
		    while(tre_regexec(&reg, bufp, 1, regmatch, 0) == 0) {
			/* Empty matches get the next char, so move by one. */
			bufp += MAX(regmatch[0].rm_eo, 1);
			ntok++;
			if (*bufp == '\0') break;
		    }
		}
		SET_VECTOR_ELT(ans, i,
			       t = allocVector(STRSXP, ntok + (*bufp ? 1 : 0)));
		/* and fill with the splits */
		bufp = buf;
		pt = Realloc(pt, strlen(buf)+1, char);
		for (j = 0; j < ntok; j++) {
		    tre_regexec(&reg, bufp, 1, regmatch, 0);
		    if (regmatch[0].rm_eo > 0) {
			/* Match was non-empty. */
			if (regmatch[0].rm_so > 0)
			    strncpy(pt, bufp, regmatch[0].rm_so);
			pt[regmatch[0].rm_so] = '\0';
			bufp += regmatch[0].rm_eo;
		    } else {
			/* Match was empty. */
			pt[0] = *bufp;
			pt[1] = '\0';
			bufp++;
		    }
		    SET_STRING_ELT(t, j, markKnown(pt, STRING_ELT(x, i)));
		}
		if (*bufp)
		    SET_STRING_ELT(t, ntok, markKnown(bufp, STRING_ELT(x, i)));
		vmaxset(vmax2);
	    }
	    tre_regfree(&reg);
	}
	vmaxset(vmax);
    }

    if (getAttrib(x, R_NamesSymbol) != R_NilValue)
	namesgets(ans, getAttrib(x, R_NamesSymbol));
    UNPROTECT(1);
    Free(pt); Free(wpt);
    if (tables) pcre_free((void *)tables);
    return ans;
}

/* Used by grep[l] and [g]regexpr, with return value the match
   position in characters */
/* This could be faster for plen > 1, but uses in R are for small strings */
static int fgrep_one(const char *pat, const char *target,
		     Rboolean useBytes, Rboolean use_UTF8, int *next)
{
    int i = -1, plen=strlen(pat), len=strlen(target);
    const char *p;

    if (plen == 0) {
	if (next != NULL) *next = 1;
	return 0;
    }
    if (plen == 1 && (useBytes || !(mbcslocale || use_UTF8))) {
	/* a single byte is a common case */
	for (i = 0, p = target; *p; p++, i++)
	    if (*p == pat[0]) {
		if (next != NULL) *next = i + 1;
		return i;
	    }
	return -1;
    }
    if (!useBytes && mbcslocale) { /* skip along by chars */
	mbstate_t mb_st;
	int ib, used;
	mbs_init(&mb_st);
	for (ib = 0, i = 0; ib <= len-plen; i++) {
	    if (strncmp(pat, target+ib, plen) == 0) {
		if (next != NULL) *next = ib + plen;
		return i;
	    }
	    used = Mbrtowc(NULL,  target+ib, MB_CUR_MAX, &mb_st);
	    if (used <= 0) break;
	    ib += used;
	}
    } else if (!useBytes && use_UTF8) {
	int ib, used;
	for (ib = 0, i = 0; ib <= len-plen; i++) {
	    if (strncmp(pat, target+ib, plen) == 0) {
		if (next != NULL) *next = ib + plen;
		return i;
	    }
	    used = utf8clen(target[ib]);
	    if (used <= 0) break;
	    ib += used;
	}
    } else
	for (i = 0; i <= len-plen; i++)
	    if (strncmp(pat, target+i, plen) == 0) {
		if (next != NULL) *next = i + plen;
		return i;
	    }
    return -1;
}

/* Returns the match position in bytes, for use in [g]sub.
   len is the length of target.
*/

static int fgrep_one_bytes(const char *pat, const char *target, int len,
			   Rboolean useBytes, Rboolean use_UTF8)
{
    int i = -1, plen=strlen(pat);
    const char *p;

    if (plen == 0) return 0;
    if (plen == 1 && (useBytes || !(mbcslocale || use_UTF8))) {
	/* a single byte is a common case */
	for (i = 0, p = target; *p; p++, i++)
	    if (*p == pat[0]) return i;
	return -1;
    }
    if (!useBytes && mbcslocale) { /* skip along by chars */
	mbstate_t mb_st;
	int ib, used;
	mbs_init(&mb_st);
	for (ib = 0, i = 0; ib <= len-plen; i++) {
	    if (strncmp(pat, target+ib, plen) == 0) return ib;
	    used = Mbrtowc(NULL, target+ib, MB_CUR_MAX, &mb_st);
	    if (used <= 0) break;
	    ib += used;
	}
    } else if (!useBytes && use_UTF8) { /* not really needed */
	int ib, used;
	for (ib = 0, i = 0; ib <= len-plen; i++) {
	    if (strncmp(pat, target+ib, plen) == 0) return ib;
	    used = utf8clen(target[ib]);
	    if (used <= 0) break;
	    ib += used;
	}
    } else
	for (i = 0; i <= len-plen; i++)
	    if (strncmp(pat, target+i, plen) == 0) return i;
    return -1;
}

SEXP attribute_hidden do_grep(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP pat, text, ind, ans;
    regex_t reg;
    int i, j, n, nmatches = 0, ov[3], rc;
    int igcase_opt, value_opt, perl_opt, fixed_opt, useBytes, invert;
    const char *spat = NULL;
    pcre *re_pcre = NULL /* -Wall */;
    pcre_extra *re_pe = NULL;
    const unsigned char *tables = NULL /* -Wall */;
    Rboolean use_UTF8 = FALSE, use_WC =  FALSE;
    const void *vmax;

    checkArity(op, args);
    pat = CAR(args); args = CDR(args);
    text = CAR(args); args = CDR(args);
    igcase_opt = asLogical(CAR(args)); args = CDR(args);
    value_opt = asLogical(CAR(args)); args = CDR(args);
    perl_opt = asLogical(CAR(args)); args = CDR(args);
    fixed_opt = asLogical(CAR(args)); args = CDR(args);
    useBytes = asLogical(CAR(args)); args = CDR(args);
    invert = asLogical(CAR(args));
    if (igcase_opt == NA_INTEGER) igcase_opt = 0;
    if (value_opt == NA_INTEGER) value_opt = 0;
    if (perl_opt == NA_INTEGER) perl_opt = 0;
    if (fixed_opt == NA_INTEGER) fixed_opt = 0;
    if (useBytes == NA_INTEGER) useBytes = 0;
    if (invert == NA_INTEGER) invert = 0;
    if (fixed_opt && igcase_opt)
	warning(_("argument '%s' will be ignored"), "ignore.case = TRUE");
    if (fixed_opt && perl_opt) {
	warning(_("argument '%s' will be ignored"), "perl = TRUE");
	perl_opt = 0;
    }

    if (!isString(pat) || length(pat) < 1)
	error(_("invalid '%s' argument"), "pattern");
    if (length(pat) > 1)
	warning(_("argument '%s' has length > 1 and only the first element will be used"), "pattern");

    if (!isString(text))
	error(_("invalid '%s' argument"), "text");

    n = LENGTH(text);
    if (STRING_ELT(pat, 0) == NA_STRING) {
	if (value_opt) {
	    SEXP nmold = getAttrib(text, R_NamesSymbol);
	    PROTECT(ans = allocVector(STRSXP, n));
	    for (i = 0; i < n; i++)  SET_STRING_ELT(ans, i, NA_STRING);
	    if (!isNull(nmold))
		setAttrib(ans, R_NamesSymbol, duplicate(nmold));
	} else {
	    PROTECT(ans = allocVector(INTSXP, n));
	    for (i = 0; i < n; i++)  INTEGER(ans)[i] = NA_INTEGER;
	}
	UNPROTECT(1);
	return ans;
    }

    if (!useBytes) {
	Rboolean onlyASCII = strIsASCII(CHAR(STRING_ELT(pat, 0)));
	if (onlyASCII)
	    for (i = 0; i < n; i++)
		if (!strIsASCII(CHAR(STRING_ELT(text, i)))) {
		    onlyASCII = FALSE;
		    break;
		}
	useBytes = onlyASCII;
    }
    if (!useBytes) {
	Rboolean haveBytes = IS_BYTES(STRING_ELT(pat, 0));
	if (!haveBytes)
	    for (i = 0; i < n; i++)
		if (IS_BYTES(STRING_ELT(text, i))) {
		    haveBytes = TRUE;
		    break;
		}
	if(haveBytes) {
	    useBytes = TRUE;
	}
    }
    if (!useBytes) {
	/* As from R 2.10.0 we use UTF-8 mode in PCRE in all MBCS locales */
	if (perl_opt && mbcslocale) use_UTF8 = TRUE;
	else if (IS_UTF8(STRING_ELT(pat, 0))) use_UTF8 = TRUE;
	if (!use_UTF8)
	    for (i = 0; i < n; i++)
		if (IS_UTF8(STRING_ELT(text, i))) {
		    use_UTF8 = TRUE;
		    break;
		}
    }

    if (!fixed_opt && !perl_opt) {
	/* if we have non-ASCII text in a DBCS locale, we need to use wchar */
	if (!useBytes && mbcslocale && !utf8locale) use_UTF8 =TRUE;
	use_WC = use_UTF8; use_UTF8 = FALSE; 
    }
    if (useBytes)
	spat = CHAR(STRING_ELT(pat, 0));
    else if (use_WC) ;
    else if (use_UTF8) {
	spat = translateCharUTF8(STRING_ELT(pat, 0));
	if (!utf8Valid(spat)) error(_("regular expression is invalid UTF-8"));
    } else {
	spat = translateChar(STRING_ELT(pat, 0));
	if (mbcslocale && !mbcsValid(spat))
	    error(_("regular expression is invalid in this locale"));
    }

    if (fixed_opt) ; 
    else if (perl_opt) {
	int cflags = 0, erroffset;
	const char *errorptr;
	if (igcase_opt) cflags |= PCRE_CASELESS;
	if (!useBytes && use_UTF8) cflags |= PCRE_UTF8;
	tables = pcre_maketables();
	re_pcre = pcre_compile(spat, cflags, &errorptr, &erroffset, tables);
	if (!re_pcre) {
	    if (errorptr)
		warning(_("PCRE pattern compilation error\n\t'%s'\n\tat '%s'\n"),
			errorptr, spat+erroffset);
	    error(_("invalid regular expression '%s'"), spat);
	    if (n > 10) {
		re_pe = pcre_study(re_pcre, 0, &errorptr);
		if (errorptr)
		    warning(_("PCRE pattern study error\n\t'%s'\n"), errorptr);
	    }
	}
    } else {
	int cflags = REG_NOSUB | REG_EXTENDED;
	if (igcase_opt) cflags |= REG_ICASE;
	if (!use_WC)
	    rc = tre_regcompb(&reg, spat, cflags);
	else
	    rc = tre_regwcomp(&reg, wtransChar(STRING_ELT(pat, 0)), cflags);
	if (rc) reg_report(rc, &reg, spat);
    }

    PROTECT(ind = allocVector(LGLSXP, n));
    vmax = vmaxget();
    for (i = 0 ; i < n ; i++) {
	LOGICAL(ind)[i] = 0;
	if (STRING_ELT(text, i) != NA_STRING) {
	    const char *s = NULL;
	    if (useBytes)
		s = CHAR(STRING_ELT(text, i));
	    else if (use_WC) ;
	    else if (use_UTF8) {
		s = translateCharUTF8(STRING_ELT(text, i));
		if (!utf8Valid(s)) {
		    warning(_("input string %d is invalid UTF-8"), i+1);
		    continue;
		}
	    } else {
		s = translateChar(STRING_ELT(text, i));
		if (mbcslocale && !mbcsValid(s)) {
		    warning(_("input string %d is invalid in this locale"), i+1);
		    continue;
		}
	    }

	    if (fixed_opt)
		LOGICAL(ind)[i] = fgrep_one(spat, s, useBytes, use_UTF8, NULL) >= 0;
	    else if (perl_opt) {
		if (pcre_exec(re_pcre, re_pe, s, strlen(s), 0, 0, ov, 0) >= 0)
		    INTEGER(ind)[i] = 1;
	    } else {
		if (!use_WC)
		    rc = tre_regexecb(&reg, s, 0, NULL, 0);
		else
		    rc = tre_regwexec(&reg, wtransChar(STRING_ELT(text, i)),
				      0, NULL, 0);
		if (rc == 0) LOGICAL(ind)[i] = 1;
	    }
	}
	vmaxset(vmax);
	if (invert ^ LOGICAL(ind)[i]) nmatches++;
    }

    if (fixed_opt);
    else if (perl_opt) {
	if (re_pe) pcre_free(re_pe);
	pcre_free(re_pcre);
	pcre_free((void *)tables);
    } else
	tre_regfree(&reg);

    if (PRIMVAL(op)) {/* grepl case */
	UNPROTECT(1);
	return ind;
    }

    if (value_opt) {
	SEXP nmold = getAttrib(text, R_NamesSymbol), nm;
	PROTECT(ans = allocVector(STRSXP, nmatches));
	for (i = 0, j = 0; i < n ; i++)
	    if (invert ^ LOGICAL(ind)[i])
		SET_STRING_ELT(ans, j++, STRING_ELT(text, i));
	/* copy across names and subset */
	if (!isNull(nmold)) {
	    nm = allocVector(STRSXP, nmatches);
	    for (i = 0, j = 0; i < n ; i++)
		if (invert ^ LOGICAL(ind)[i])
		    SET_STRING_ELT(nm, j++, STRING_ELT(nmold, i));
	    setAttrib(ans, R_NamesSymbol, nm);
	}
	UNPROTECT(1);
    } else {
	ans = allocVector(INTSXP, nmatches);
	j = 0;
	for (i = 0 ; i < n ; i++)
	    if (invert ^ LOGICAL(ind)[i]) INTEGER(ans)[j++] = i + 1;
    }
    UNPROTECT(1);
    return ans;
}


/* fixed, single binary search, no error checking; -1 = no match, otherwise offset
   NOTE: all offsets here (in & out) are 0-based !! */
static R_size_t fgrepraw1(SEXP pat, SEXP text, R_size_t offset) {
    Rbyte *haystack = RAW(text), *needle = RAW(pat);
    R_size_t n = LENGTH(text);
    switch (LENGTH(pat)) { /* it may be silly but we optimize small needle
			      searches, because they can be used to match
			      single UTF8 chars (up to 3 bytes) */
    case 1:
	{
	    Rbyte c = needle[0];
	    while (offset < n) {
		if (haystack[offset] == c)
		    return offset;
		offset++;
	    }
	    return -1;
	}
    case 2:
	{
	    n--;
	    while (offset < n) {
		if (haystack[offset] == needle[0] && haystack[offset + 1] == needle[1])
		    return offset;
		offset++;
	    }
	    return -1;
	}
    case 3:
	{
	    n -= 2;
	    while (offset < n) {
		if (haystack[offset    ] == needle[0] &&
		    haystack[offset + 1] == needle[1] &&
		    haystack[offset + 2] == needle[2])
		    return offset;
		offset++;
	    }
	    return -1;
	}
    default:
	{
	    R_size_t ncmp = LENGTH(pat);
	    n -= ncmp;
	    ncmp--;
	    while (offset < n) {
		if (haystack[offset] == needle[0] &&
		    !memcmp(haystack + offset + 1, needle + 1, ncmp))
		    return offset;
		offset++;
	    }
	}
    }
    return -1;
}

/* grepRaw(pattern, text, offset, ignore.case, fixed, value, all, invert) */
SEXP attribute_hidden do_grepraw(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP pat, text, ans, res_head, res_tail;
    regex_t reg;
    int nmatches = 0, rc, cflags, eflags = 0;
    int *res_val;
    int res_alloc = 512; /* must be divisible by 2 since we may store
			    offset+length it is the initial size of
			    the integer vector of matches */
    R_size_t res_ptr, offset, i;
    int igcase_opt, fixed_opt, all, value, invert;

    checkArity(op, args);
    pat = CAR(args); args = CDR(args);
    text = CAR(args); args = CDR(args);
    offset = asInteger(CAR(args)); args = CDR(args);
    igcase_opt = asLogical(CAR(args)); args = CDR(args);
    fixed_opt = asLogical(CAR(args)); args = CDR(args);
    value = asLogical(CAR(args)); args = CDR(args);
    all = asLogical(CAR(args)); args = CDR(args);
    invert = asLogical(CAR(args));
    if (igcase_opt == NA_INTEGER) igcase_opt = 0;
    if (fixed_opt == NA_INTEGER) fixed_opt = 0;
    if (all == NA_INTEGER) all = 0;
    if (value == NA_INTEGER) value = 0;
    if (invert == NA_INTEGER) invert = 0;
    if (fixed_opt && igcase_opt)
	warning(_("argument '%s' will be ignored"), "ignore.case = TRUE");

    /* invert=TRUE, value=FALSE will really give you a headache
       thinking about it so we better not go there (the code below
       will actually respect it for all cases except for fixed=FALSE,
       all=TRUE so we could support it at some point but I fail to see
       any real use of it) */
    if (invert && !value) {
	warning(_("argument '%s' will be ignored"), "invert = TRUE");
	invert = 0;
    }

    /* currently we support only offset >= 1 */
    if (offset < 1)
	error(_("invalid '%s' argument"), "offset");
    if (!isRaw(pat))
	error(_("invalid '%s' argument"), "pattern");
    if (!isRaw(text))
	error(_("invalid '%s' argument"), "text");
    if (offset > LENGTH(text))
	return allocVector(INTSXP, 0);

    offset--; /* reduce offset to base 0 */

    /* TRE fails miserably for REG_LITERAL -- not only is it slow but
       it doesn't handle embedded NULs properly (e.g., compile
       goes into an infinite loop with "\00" pattern) -- so we have
       to do it by hand */
    if (fixed_opt) {
	if (LENGTH(pat) == 0)
	    return allocVector(value ? (all ? VECSXP : RAWSXP) : INTSXP, 0);
	if (!all) {
	    R_size_t res = fgrepraw1(pat, text, offset);
	    if (invert) {
		Rbyte *ansp;
		if (res == -1) return value ? text : ScalarInteger(1);
		if (!value) return ScalarInteger(((res == 0) ? LENGTH(pat) : 0) + 1);
		ans = allocVector(RAWSXP, LENGTH(text) - LENGTH(pat));
		ansp = RAW(ans);
		if (res) {
		    memcpy(ansp, RAW(text), res);
		    ansp += res; 
		}
		res += LENGTH(pat);
		if (res < LENGTH(text))
		    memcpy(ansp, RAW(text) + res, LENGTH(text) - res);
		return ans;
	    }
	    if (res == -1) return allocVector(value ? RAWSXP : INTSXP, 0);
	    if (!value) return ScalarInteger(res + 1);
	    /* value=TRUE doesn't really make sense for anything other than
	       match/nomatch detection since we just return the pattern */
	    return pat;
	} else {
	    /* There are two ways to do it: two pass or one pass. We
	       use the latter with TRE below, but for a sequential
	       search I assume it's fast enough so it's not worth the
	       hassle. We just special-case really tiny matches which
	       should be the most common case anyway.
	    */
#define MAX_MATCHES_MINIBUF 32
	    int matches[MAX_MATCHES_MINIBUF];
	    R_size_t n = LENGTH(text);
	    while (offset < n) {
		offset = fgrepraw1(pat, text, offset);
		if (offset == -1)
		    break;
		if (nmatches < MAX_MATCHES_MINIBUF)
		    matches[nmatches] = offset + 1;
		nmatches++;
		offset += LENGTH(pat);
	    }
	    if (value) {
		if (invert) { /* invert is actually useful here as it
				 is performing something like strsplit */
		    R_size_t pos = 0;
		    SEXP elt, mvec = NULL;
		    int *fmatches = (int*) matches; /* either the minbuffer or an allocated maxibuffer */

		    if (!nmatches) return text;

		    /* if there are more matches than in the buffer, 
		       we actually need to get them first */
		    if (nmatches > MAX_MATCHES_MINIBUF) { 
			mvec = PROTECT(allocVector(INTSXP, nmatches));
			fmatches = INTEGER(mvec);
			memcpy(fmatches, matches, sizeof(matches));
			nmatches = MAX_MATCHES_MINIBUF;
			offset = matches[MAX_MATCHES_MINIBUF - 1] + LENGTH(pat) - 1;
			while (offset < n) {
			    offset = fgrepraw1(pat, text, offset);
			    if (offset == -1)
				break;
			    INTEGER(mvec)[nmatches++] = offset + 1;
			    offset += LENGTH(pat);
			}
		    }

		    /* there are always nmatches + 1 pieces (unlike strsplit) */
		    ans = PROTECT(allocVector(VECSXP, nmatches + 1));
		    /* add all pieces before matches */
		    for (i = 0; i < nmatches; i++) {
			R_size_t elt_size = fmatches[i] - 1 - pos;
			elt = allocVector(RAWSXP, elt_size);
			SET_VECTOR_ELT(ans, i, elt);
			if (elt_size)
			    memcpy(RAW(elt), RAW(text) + pos, elt_size);
			pos = fmatches[i] - 1 + LENGTH(pat);
		    }
		    /* add the rest after last match */
		    elt = allocVector(RAWSXP, LENGTH(text) - (fmatches[nmatches - 1] - 1 + LENGTH(pat)));
		    SET_VECTOR_ELT(ans, nmatches, elt);
		    if (LENGTH(elt))
			memcpy(RAW(elt), RAW(text) + LENGTH(text) - LENGTH(elt), LENGTH(elt));
		    if (mvec)
			UNPROTECT(1);
		    UNPROTECT(1);
		    return ans;
		}

		/* value=TRUE is pathetic for fixed=TRUE without
		   invert as it is just rep(pat, nmatches) */
		ans = PROTECT(allocVector(VECSXP, nmatches));
		for (i = 0; i < nmatches; i++)
		    SET_VECTOR_ELT(ans, i, pat);
		UNPROTECT(1);
		return ans;
	    }
	    ans = allocVector(INTSXP, nmatches);
	    if (nmatches <= MAX_MATCHES_MINIBUF) { /* our min-buffer was enough, great */
		if (nmatches) memcpy(INTEGER(ans), matches, nmatches * sizeof(int));
		return ans;
	    }
	    /* more matches than we could remember, time for pass 2 */
	    memcpy(INTEGER(ans), matches, sizeof(matches));
	    /* but we are not completely stupid - we can continue
	       where amnesia hit us */
	    nmatches = MAX_MATCHES_MINIBUF;
	    offset = matches[MAX_MATCHES_MINIBUF - 1] + LENGTH(pat) - 1; /* matches are 1-based, we are 0-based hence - 1 */
	    while (offset < n) {
		offset = fgrepraw1(pat, text, offset);
		if (offset == -1)
		    break;
		INTEGER(ans)[nmatches++] = offset + 1;
		offset += LENGTH(pat);
	    }
	    return ans;
	}
    }	

    cflags = REG_EXTENDED;
    if (igcase_opt) cflags |= REG_ICASE;

    rc = tre_regncompb(&reg, (const char*) RAW(pat), LENGTH(pat), cflags);
    if (rc) reg_report(rc, &reg, NULL /* pat is not necessarily a C string */ );

    if (!all) { /* match only once */
	regmatch_t ptag;
	rc = tre_regnexecb(&reg, (const char*) RAW(text) + offset, LENGTH(text) - offset, 1, &ptag, 0);
	tre_regfree(&reg);
	if (value) {
	    if (rc != REG_OK || ptag.rm_eo == ptag.rm_so) /* TODO: is this good enough? it is the same as matching an empty string ... */
		return invert ? text : allocVector(RAWSXP, 0);
	    if (invert) {
		Rbyte *ansp;
		R_size_t len;
		ans = allocVector(RAWSXP, LENGTH(text) - (ptag.rm_eo - ptag.rm_so));
		ansp = RAW(ans);
		if (ptag.rm_so) {
		    memcpy(ansp, RAW(text), ptag.rm_so);
		    ansp += ptag.rm_so;
		}
		len = LENGTH(text) - ptag.rm_eo;
		if (len)
		    memcpy(ansp, RAW(text) + ptag.rm_eo, len);
	    } else {
		ans = allocVector(RAWSXP, ptag.rm_eo - ptag.rm_so);
		memcpy(RAW(ans), RAW(text) + offset + ptag.rm_so, ptag.rm_eo - ptag.rm_so);
	    }
	    return ans;
	}
	return (rc == REG_OK) ? ScalarInteger(ptag.rm_so + 1 + offset) : allocVector(INTSXP, 0);
    }
	
    /* match all - we use a pairlist of integer arrays to expand the result
       to allow use on big binary strings with many matches (it could be done
       by re-allocating a temp buffer but I chose sequential allocations to
       reduce possible fragmentation) */
    res_head = res_tail = PROTECT(list1(allocVector(INTSXP, res_alloc)));
    res_val = INTEGER(CAR(res_tail));
    res_ptr = 0;
    while (1) {
	regmatch_t ptag;
	rc = tre_regnexecb(&reg, (const char*) RAW(text) + offset, LENGTH(text) - offset, 1, &ptag, eflags);
	if (rc)
	    break;
	if (!nmatches) eflags |= REG_NOTBOL;
	if (res_ptr >= res_alloc) {
	    if (res_alloc < (2^24)) res_alloc <<= 1;
	    SETCDR(res_tail, list1(allocVector(INTSXP, res_alloc)));
	    res_tail = CDR(res_tail);
	    res_val = INTEGER(CAR(res_tail));
	    res_ptr = 0;
	}
	res_val[res_ptr++] = ptag.rm_so + 1 + offset;
	if (value) res_val[res_ptr++] = ptag.rm_eo - ptag.rm_so;
	offset += ptag.rm_eo;
	nmatches++;
	if (ptag.rm_eo == 0) { /* empty string matched => trouble; FIXME: we may want to consider just advancing anyway */
	    int infinite_match = 1;
	    /* the only place where this is acceptable is "^" as that will go away in the next step */
	    if (nmatches == 1) { /* to see if that is true, re-run the match with REG_NOTBOL (added above) */
		rc = tre_regnexecb(&reg, (const char*) RAW(text) + offset, LENGTH(text) - offset, 1, &ptag, eflags);
		if (rc != REG_OK || ptag.rm_eo != 0)
		    infinite_match = 0;
	    }
	    if (infinite_match)	    
		warning(_("pattern matches an empty string infinitely, returning first match only"));
	    break;
	}
	if (offset >= LENGTH(text)) break;
    }
    
    if (value) { /* for values we store in fact the absolute start offsets and length in the integer vector */
	SEXP vec = CAR(res_head);
	R_size_t entry = 0, cptr = 0, clen = (CDR(res_head) == R_NilValue) ? res_ptr : LENGTH(vec);
	R_size_t inv_start = 0; /* 0-based start position of the pieces for invert */
	res_val = INTEGER(vec);
	ans = PROTECT(allocVector(VECSXP, invert ? (nmatches + 1) : nmatches));
	while (entry < nmatches) {
	    if (invert) { /* for invert=TRUE store the current piece up to the match */
		SEXP rvec = allocVector(RAWSXP, res_val[cptr] - 1 - inv_start);
		SET_VECTOR_ELT(ans, entry, rvec);
		entry++;
		if (LENGTH(rvec))
		    memcpy(RAW(rvec), RAW(text) + inv_start, LENGTH(rvec));
		inv_start = res_val[cptr] - 1 + res_val[cptr + 1];
	    } else { /* for invert=FALSE store the matched piece */
		SEXP rvec = allocVector(RAWSXP, res_val[cptr + 1]);
		SET_VECTOR_ELT(ans, entry, rvec);
		entry++;
		if (LENGTH(rvec))
		    memcpy(RAW(rvec), RAW(text) + res_val[cptr] - 1, LENGTH(rvec));
	    }
	    /* advance in the elements -- possibly jumping to the next list block */
	    cptr += 2;
	    if (cptr >= clen) {
		res_head = CDR(res_head);
		if (res_head == R_NilValue) break;
		vec = CAR(res_head);
		res_val = INTEGER(vec);
		cptr = 0;
		clen = (CDR(res_head) == R_NilValue) ? res_ptr : LENGTH(vec);
	    }
	}
	if (invert) { /* add the last piece after the last match */
	    SEXP lvec = allocVector(RAWSXP, LENGTH(text) - inv_start);
	    SET_VECTOR_ELT(ans, nmatches, lvec);
	    if (LENGTH(lvec))
		memcpy(RAW(lvec), RAW(text) + inv_start, LENGTH(lvec));
	}
	UNPROTECT(1);
    } else { /* if values are not needed, we just collect all the start offsets */
	ans = allocVector(INTSXP, nmatches);
	res_val = INTEGER(ans);
	while (res_head != R_NilValue) {
	    SEXP vec = CAR(res_head);
	    R_size_t len = (CDR(res_head) == R_NilValue) ? res_ptr : LENGTH(vec);
	    if (len) memcpy(res_val, INTEGER(vec), len * sizeof(int));
	    res_val += len;
	    res_head = CDR(res_head);
	}
    }
    UNPROTECT(1);
    
    tre_regfree(&reg);

    return ans;
}

/* The following R functions do substitution for regular expressions,
 * either once or globally.
 * The functions are loosely patterned on the "sub" and "gsub" in "nawk". */

static char *string_adj(char *target, const char *orig, const char *repl,
			regmatch_t *regmatch)
{
    int i, k;
    const char *p = repl; char *t = target;

    while (*p) {
	if (*p == '\\') {
	    if ('1' <= p[1] && p[1] <= '9') {
		k = p[1] - '0';
		for (i = regmatch[k].rm_so ; i < regmatch[k].rm_eo ; i++)
		    *t++ = orig[i];
		p += 2;
	    }
	    else if (p[1] == 0) p++; else {p++; *t++ = *p++;}
	}
	else *t++ = *p++;
    }
    return t;
}

/* used for single-byte locales, and UTF-8 for perl = TRUE */
static int count_subs(const char *repl)
{
    int i = 0;
    const char *p = repl;
    while (*p) {
	if (*p == '\\') {
	    if ('1' <= p[1] && p[1] <= '9') {i++; p += 2;}
	    else if (p[1] == 0) p++; else p += 2;
	}
	else p++;
    }
    return i;
}

/* FIXME: use UCP for upper/lower conversion */
static 
char *pcre_string_adj(char *target, const char *orig, const char *repl,
		      int *ovec, Rboolean use_UTF8)
{
    int i, k, nb;
    const char *p = repl; 
    char *t = target, c;
    Rboolean upper = FALSE, lower = FALSE;

    while (*p) {
	if (*p == '\\') {
	    if ('1' <= p[1] && p[1] <= '9') {
		k = p[1] - '0';
		/* Here we need to work in chars */
		nb = ovec[2*k+1] - ovec[2*k];
		if (nb > 0 && use_UTF8 && (upper || lower)) {
		    wctrans_t tr = wctrans(upper ? "toupper" : "tolower");
		    int j, nc;
		    char *xi, *p;
		    wchar_t *wc;
		    p = xi = (char *) alloca((nb+1)*sizeof(char));
		    R_CheckStack();
		    for (j = 0; j < nb; j++) *p++ = orig[ovec[2*k]+j];
		    *p = '\0';
		    nc = utf8towcs(NULL, xi, 0);
		    if (nc >= 0) {
			wc = (wchar_t *) alloca((nc+1)*sizeof(wchar_t));
			R_CheckStack();
			utf8towcs(wc, xi, nc + 1);
			for (j = 0; j < nc; j++) wc[j] = towctrans(wc[j], tr);
			nb = wcstoutf8(NULL, wc, 0);
			wcstoutf8(xi, wc, nb + 1);
			for (j = 0; j < nb; j++) *t++ = *xi++;
		    }
		} else
		    for (i = ovec[2*k] ; i < ovec[2*k+1] ; i++) {
			c = orig[i];
			*t++ = upper ? toupper(c) : (lower ? tolower(c) : c);
		    }
		p += 2;
	    } else if (p[1] == 'U') {
		p += 2;
		upper = TRUE; lower = FALSE;
	    } else if (p[1] == 'L') {
		p += 2;
		upper = FALSE; lower = TRUE;
	    } else if (p[1] == 'E') { /* end case modification */
		p += 2;
		upper = FALSE; lower = FALSE;
	    } else if (p[1] == 0) {
		p += 1;
	    } else {
		p += 1;
		*t++ = *p++;
	    }
	} else *t++ = *p++;
    }
    return t;
}

static wchar_t *wstring_adj(wchar_t *target, const wchar_t *orig,
			    const wchar_t *repl, regmatch_t *regmatch)
{
    int i, k;
    const wchar_t *p = repl;
    wchar_t *t = target;

    while (*p) {
	if (*p == L'\\') {
	    if (L'1' <= p[1] && p[1] <= L'9') {
		k = p[1] - L'0';
		for (i = regmatch[k].rm_so ; i < regmatch[k].rm_eo ; i++)
		    *t++ = orig[i];
		p += 2;
	    }
	    else if (p[1] == 0) p++; else {p++; *t++ = *p++;}
	}
	else *t++ = *p++;
    }
    return t;
}

static int wcount_subs(const wchar_t *repl)
{
    int i = 0;
    const wchar_t *p = repl;
    while (*p) {
	if (*p == '\\') {
	    if ('1' <= p[1] && p[1] <= '9') {i++; p += 2;}
	    else if (p[1] == 0) p++; else p += 2;
	}
	else p++;
    }
    return i;
}


/* The following R functions do substitution for regular expressions,
 * either once or globally.
 * The functions are loosely patterned on the "sub" and "gsub" in "nawk". */

SEXP attribute_hidden do_gsub(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP pat, rep, text, ans;
    regex_t reg;
    regmatch_t regmatch[10];
    int i, j, n, ns, nns, nmatch, offset, rc;
    int global, igcase_opt, perl_opt, fixed_opt, useBytes, eflags, last_end;
    char *u, *cbuf;
    const char *spat = NULL, *srep = NULL, *s = NULL;
    int patlen = 0, replen = 0;
    Rboolean use_UTF8 = FALSE, use_WC = FALSE;
    const wchar_t *wrep = NULL;
    pcre *re_pcre = NULL;
    pcre_extra *re_pe  = NULL;
    const unsigned char *tables = NULL;
    const void *vmax = vmaxget();

    checkArity(op, args);

    global = PRIMVAL(op);

    pat = CAR(args); args = CDR(args);
    rep = CAR(args); args = CDR(args);
    text = CAR(args); args = CDR(args);
    igcase_opt = asLogical(CAR(args)); args = CDR(args);
    perl_opt = asLogical(CAR(args)); args = CDR(args);
    fixed_opt = asLogical(CAR(args)); args = CDR(args);
    useBytes = asLogical(CAR(args)); args = CDR(args);
    if (igcase_opt == NA_INTEGER) igcase_opt = 0;
    if (perl_opt == NA_INTEGER) perl_opt = 0;
    if (fixed_opt == NA_INTEGER) fixed_opt = 0;
    if (useBytes == NA_INTEGER) useBytes = 0;
    if (fixed_opt && igcase_opt)
	warning(_("argument '%s' will be ignored"), "ignore.case = TRUE");
    if (fixed_opt && perl_opt) {
	warning(_("argument '%s' will be ignored"), "perl = TRUE");
	perl_opt = 0;
    }

    if (!isString(pat) || length(pat) < 1)
	error(_("invalid '%s' argument"), "pattern");
    if (length(pat) > 1)
	warning(_("argument '%s' has length > 1 and only the first element will be used"), "pattern");
    if (!isString(rep) || length(rep) < 1)
	error(_("invalid '%s' argument"), "replacement");
    if (length(rep) > 1)
	warning(_("argument '%s' has length > 1 and only the first element will be used"), "replacement");

    if (!isString(text))
	error(_("invalid '%s' argument"), "text");

    n = LENGTH(text);
    /* This contradicts the code below that has NA matching NA */
    if (STRING_ELT(pat, 0) == NA_STRING) {
	PROTECT(ans = allocVector(STRSXP, n));
	for (i = 0; i < n; i++)  SET_STRING_ELT(ans, i, NA_STRING);
	UNPROTECT(1);
	return ans;
    }

    if (!useBytes) {
	Rboolean onlyASCII = strIsASCII(CHAR(STRING_ELT(pat, 0)));
	if (onlyASCII)
	    for (i = 0; i < n; i++)
		if (!strIsASCII(CHAR(STRING_ELT(text, i)))) {
		    onlyASCII = FALSE;
		    break;
		}
	useBytes = onlyASCII;
    }
    if (!useBytes) {
	Rboolean haveBytes = IS_BYTES(STRING_ELT(pat, 0));
	if (!haveBytes)
	    for (i = 0; i < n; i++)
		if (IS_BYTES(STRING_ELT(text, i))) {
		    haveBytes = TRUE;
		    break;
		}
	if(haveBytes) {
	    useBytes = TRUE;
	}
    }
    if (!useBytes) {
	if (!fixed_opt && mbcslocale) use_UTF8 = TRUE;
	else if (IS_UTF8(STRING_ELT(pat, 0))) use_UTF8 = TRUE;
	if (!use_UTF8)
	    for (i = 0; i < n; i++)
		if (IS_UTF8(STRING_ELT(text, i))) {
		    use_UTF8 = TRUE;
		    break;
		}
    }

    if (!fixed_opt && !perl_opt) {
	/* if we have non-ASCII text in a DBCS locale, we need to use wchar */
	if (!useBytes && mbcslocale && !utf8locale) use_UTF8 =TRUE;
	use_WC = use_UTF8; use_UTF8 = FALSE; 
    }

    if (useBytes) {
	spat = CHAR(STRING_ELT(pat, 0));
	srep = CHAR(STRING_ELT(rep, 0));
    } else if (use_WC) ;
    else if (use_UTF8) {
	spat = translateCharUTF8(STRING_ELT(pat, 0));
	if (!utf8Valid(spat)) error(_("'pattern' is invalid UTF-8"));
	srep = translateCharUTF8(STRING_ELT(rep, 0));
	if (!utf8Valid(srep)) error(_("'replacement' is invalid UTF-8"));
    } else {
	spat = translateChar(STRING_ELT(pat, 0));
	if (mbcslocale && !mbcsValid(spat))
	    error(_("'pattern' is invalid in this locale"));
	srep = translateChar(STRING_ELT(rep, 0));
	if (mbcslocale && !mbcsValid(srep))
	    error(_("'replacement' is invalid in this locale"));
    }

    if (fixed_opt) {
	patlen = strlen(spat);
	if (!patlen) error(_("zero-length pattern"));
	replen = strlen(srep);
    } else if (perl_opt) {
	int cflags = 0, erroffset;
	const char *errorptr;
	if (use_UTF8) cflags |= PCRE_UTF8;
	if (igcase_opt) cflags |= PCRE_CASELESS;
	tables = pcre_maketables();
	re_pcre = pcre_compile(spat, cflags, &errorptr, &erroffset, tables);
	if (!re_pcre) {
	    if (errorptr)
		warning(_("PCRE pattern compilation error\n\t'%s'\n\tat '%s'\n"),
			errorptr, spat+erroffset);
	    error(_("invalid regular expression '%s'"), spat);
	}
	if (n > 10) {
	    re_pe = pcre_study(re_pcre, 0, &errorptr);
	    if (errorptr)
		warning(_("PCRE pattern study error\n\t'%s'\n"), errorptr);
	}
	replen = strlen(srep);
    } else {
	int cflags = REG_EXTENDED;
	if (igcase_opt) cflags |= REG_ICASE;
	if (!use_WC) {
	    rc =  tre_regcompb(&reg, spat, cflags);
	    if (rc) reg_report(rc, &reg, spat);
	    replen = strlen(srep);
	} else {
	    rc  = tre_regwcomp(&reg, wtransChar(STRING_ELT(pat, 0)), cflags);
	    if (rc) reg_report(rc, &reg, CHAR(STRING_ELT(pat, 0)));
	    wrep = wtransChar(STRING_ELT(rep, 0));
	    replen = wcslen(wrep);
	}
    }

    PROTECT(ans = allocVector(STRSXP, n));
    vmax = vmaxget();
    for (i = 0 ; i < n ; i++) {
	/* NA pattern was handled above */
	if (STRING_ELT(text,i) == NA_STRING) {
	    SET_STRING_ELT(ans, i, NA_STRING);
	    continue;
	}

	if (useBytes)
	    s = CHAR(STRING_ELT(text, i));
	else if (use_WC) ;
	else if (use_UTF8) {
	    s = translateCharUTF8(STRING_ELT(text, i));
	    if (!utf8Valid(s)) error(("input string %d is invalid UTF-8"), i+1);
	} else {
	    s = translateChar(STRING_ELT(text, i));
	    if (mbcslocale && !mbcsValid(s))
		error(("input string %d is invalid in this locale"), i+1);
	}

	if (fixed_opt) {
	    int st, nr, slen = strlen(s);
	    ns = slen;
	    st = fgrep_one_bytes(spat, s, ns, useBytes, use_UTF8);
	    if (st < 0)
		SET_STRING_ELT(ans, i, STRING_ELT(text, i));
	    else if (STRING_ELT(rep, 0) == NA_STRING)
		SET_STRING_ELT(ans, i, NA_STRING);
	    else {
		if (global) { /* need to find max number of matches */
		    const char *ss= s;
		    int sst = st;
		    nr = 0;
		    do {
			nr++;
			ss += sst+patlen;
                        slen -= sst+patlen;
		    } while((sst = fgrep_one_bytes(spat, ss, slen, useBytes, use_UTF8)) >= 0);
		} else nr = 1;
		cbuf = u = Calloc(ns + nr*(replen - patlen) + 1, char);
		*u = '\0';
                slen = ns;
		do {
                    strncpy(u, s, st);
                    u += st;
                    s += st+patlen;
                    slen -= st+patlen;
		    strncpy(u, srep, replen);
                    u += replen;
		} while(global && (st = fgrep_one_bytes(spat, s, slen, useBytes, use_UTF8)) >= 0);
		strcpy(u, s);
		if (useBytes)
		    SET_STRING_ELT(ans, i, mkChar(cbuf));
		else if (use_UTF8)
		    SET_STRING_ELT(ans, i, mkCharCE(cbuf, CE_UTF8));
		else
		    SET_STRING_ELT(ans, i, markKnown(cbuf, STRING_ELT(text, i)));
		Free(cbuf);
	    }
	} else if (perl_opt) {
	   int ncap, maxrep, ovector[30], eflag;
	   memset(ovector, 0, 30*sizeof(int)); /* zero for unknown patterns */
	   ns = strlen(s);
	   /* worst possible scenario is to put a copy of the
	      replacement after every character, unless there are
	      backrefs */
	   maxrep = replen + (ns-2) * count_subs(srep);
	   if (global) {
	       /* Integer overflow has been seen */
	       double dnns = ns * (maxrep + 1.) + 1000;
	       if (dnns > 10000) dnns = 2*ns + replen + 1000;
	       nns = dnns;
	   } else nns = ns + maxrep + 1000;
	   u = cbuf = Calloc(nns, char);
	   offset = 0; nmatch = 0; eflag = 0; last_end = -1;
	   /* ncap is one more than the number of capturing patterns */
	   while ((ncap = pcre_exec(re_pcre, re_pe, s, ns, offset, eflag,
				   ovector, 30)) >= 0) {
	       /* printf("%s, %d, %d %d\n", s, offset,
		  ovector[0], ovector[1]); */
	       nmatch++;
	       for (j = offset; j < ovector[0]; j++) *u++ = s[j];
	       if (ovector[1] > last_end) {
		   u = pcre_string_adj(u, s, srep, ovector, use_UTF8);
		   last_end = ovector[1];
	       }
	       offset = ovector[1];
	       if (s[offset] == '\0' || !global) break;
	       if (ovector[1] == ovector[0]) {
		   /* advance by a char */
		   if (use_UTF8) {
		       int used, pos = 0;
		       while( (used = utf8clen(s[pos])) ) {
			   pos += used;
			   if (pos > offset) {
			       for (j = offset; j < pos; j++) *u++ = s[j];
			       offset = pos;
			       break;
			   }
		       }
		   } else
		       *u++ = s[offset++];
	       }
	       if (nns < (u - cbuf) + (ns-offset) + maxrep + 100) {
		   char *tmp;
		   if (nns > INT_MAX/2) error(_("result string is too long"));
		   nns *= 2;
		   tmp = Realloc(cbuf, nns, char);
		   u = tmp + (u - cbuf);
		   cbuf = tmp;
	       }
	       eflag = PCRE_NOTBOL;  /* probably not needed */
	   }
	   if (nmatch == 0)
	       SET_STRING_ELT(ans, i, STRING_ELT(text, i));
	   else if (STRING_ELT(rep, 0) == NA_STRING)
	       SET_STRING_ELT(ans, i, NA_STRING);
	   else {
	       /* copy the tail */
	       if (nns < (u - cbuf) + (ns-offset)+1) {
		   char *tmp;
		   if (nns > INT_MAX/2) error(_("result string is too long"));
		   nns *= 2;
		   tmp = Realloc(cbuf, nns, char);
		   u = tmp + (u - cbuf);
		   cbuf = tmp;
	       }
	       for (j = offset ; s[j] ; j++) *u++ = s[j];
	       *u = '\0';
	       if (useBytes)
		   SET_STRING_ELT(ans, i, mkChar(cbuf));
	       else if (use_UTF8)
		   SET_STRING_ELT(ans, i, mkCharCE(cbuf, CE_UTF8));
	       else
		   SET_STRING_ELT(ans, i, markKnown(cbuf, STRING_ELT(text, i)));
	   }	
	   Free(cbuf);
       } else if (!use_WC) {
	    int maxrep;
	    /* extended regexp in bytes */

	    ns = strlen(s);
	    /* worst possible scenario is to put a copy of the
	       replacement after every character, unless there are
	       backrefs */
	    maxrep = replen + (ns-2) * count_subs(srep);
	    if (global) {
		double dnns = ns * (maxrep + 1.) + 1000;
		if (dnns > 10000) dnns = 2*ns + replen + 1000;
		nns = dnns;
	    } else nns = ns + maxrep + 1000;
	    u = cbuf = Calloc(nns, char);
	    offset = 0; nmatch = 0; eflags = 0; last_end = -1;
	    while (tre_regexecb(&reg, s+offset, 10, regmatch, eflags) == 0) {
		/* printf("%s, %d %d\n", &s[offset],
		   regmatch[0].rm_so, regmatch[0].rm_eo); */
		nmatch++;
		for (j = 0; j < regmatch[0].rm_so ; j++)
		    *u++ = s[offset+j];
		if (offset+regmatch[0].rm_eo > last_end) {
		    u = string_adj(u, s+offset, srep, regmatch);
		    last_end = offset+regmatch[0].rm_eo;
		}
		offset += regmatch[0].rm_eo;
		if (s[offset] == '\0' || !global) break;
		if (regmatch[0].rm_eo == regmatch[0].rm_so)
		    *u++ = s[offset++];
		if (nns < (u - cbuf) + (ns-offset) + maxrep + 100) {
		    char *tmp;
		    if (nns > INT_MAX/2) error(_("result string is too long"));
		    nns *= 2;
		    tmp = Realloc(cbuf, nns, char);
		    u = tmp + (u - cbuf);
		    cbuf = tmp;
		}
		eflags = REG_NOTBOL;
	    }
	    if (nmatch == 0)
		SET_STRING_ELT(ans, i, STRING_ELT(text, i));
	    else if (STRING_ELT(rep, 0) == NA_STRING)
		SET_STRING_ELT(ans, i, NA_STRING);
	    else {
		/* copy the tail */
		if (nns < (u - cbuf) + (ns-offset)+1) {
		    char *tmp;
		    if (nns > INT_MAX/2) error(_("result string is too long"));
		    nns *= 2;
		    tmp = Realloc(cbuf, nns, char);
		    u = tmp + (u - cbuf);
		    cbuf = tmp;
		}
		for (j = offset ; s[j] ; j++) *u++ = s[j];
		*u = '\0';
		if (useBytes)
		    SET_STRING_ELT(ans, i, mkChar(cbuf));
		else
		    SET_STRING_ELT(ans, i, markKnown(cbuf, STRING_ELT(text, i)));
	    }
	    Free(cbuf);
	} else  {
	    /* extended regexp in wchar_t */
	    const wchar_t *s = wtransChar(STRING_ELT(text, i));
	    wchar_t *u, *cbuf;
	    int maxrep;

	    ns = wcslen(s);
	    maxrep = replen + (ns-2) * wcount_subs(wrep);
	    if (global) {
		/* worst possible scenario is to put a copy of the
		   replacement after every character */
		double dnns = ns * (maxrep + 1.) + 1000;
		if (dnns > 10000) dnns = 2*ns + maxrep + 1000;
		nns = dnns;
	    } else nns = ns + maxrep + 1000;
	    u = cbuf = Calloc(nns, wchar_t);
	    offset = 0; nmatch = 0; eflags = 0; last_end = -1;
	    while (tre_regwexec(&reg, s+offset, 10, regmatch, eflags) == 0) {
		nmatch++;
		for (j = 0; j < regmatch[0].rm_so ; j++)
		    *u++ = s[offset+j];
		if (offset+regmatch[0].rm_eo > last_end) {
		    u = wstring_adj(u, s+offset, wrep, regmatch);
		    last_end = offset+regmatch[0].rm_eo;
		}
		offset += regmatch[0].rm_eo;
		if (s[offset] == L'\0' || !global) break;
		if (regmatch[0].rm_eo == regmatch[0].rm_so)
		    *u++ = s[offset++];
		if (nns < (u - cbuf) + (ns-offset) + maxrep + 100) {
		    wchar_t *tmp;
		    /* This could fail at smaller value on a 32-bit platform:
		       it is merely an integer overflow check */
		    if (nns > INT_MAX/2) error(_("result string is too long"));
		    nns *= 2;
		    tmp = Realloc(cbuf, nns, wchar_t);
		    u = tmp + (u - cbuf);
		    cbuf = tmp;
		}
		eflags = REG_NOTBOL;
	    }
	    if (nmatch == 0)
		SET_STRING_ELT(ans, i, STRING_ELT(text, i));
	    else if (STRING_ELT(rep, 0) == NA_STRING)
		SET_STRING_ELT(ans, i, NA_STRING);
	    else {
		/* copy the tail */
		if (nns < (u - cbuf) + (ns-offset)+1) {
		    wchar_t *tmp;
		    if (nns > INT_MAX/2) error(_("result string is too long"));
		    nns *= 2;
		    tmp = Realloc(cbuf, nns, wchar_t);
		    u = tmp + (u - cbuf);
		    cbuf = tmp;
		}
		for (j = offset ; s[j] ; j++) *u++ = s[j];
		*u = L'\0';
		SET_STRING_ELT(ans, i, mkCharW(cbuf));
	    }
	    Free(cbuf);
	}
	vmaxset(vmax);
    }

    if (fixed_opt) ; 
    else if (perl_opt) {
	if (re_pe) pcre_free(re_pe);
	pcre_free(re_pcre);
	pcre_free((void *)tables);
    } else tre_regfree(&reg);
    DUPLICATE_ATTRIB(ans, text);
    /* This copied the class, if any */
    UNPROTECT(1);
    return ans;
}

static int getNc(const char *s, int st)
{
    char *buf = alloca(st+1);
    R_CheckStack();
    memcpy(buf, s, st);
    buf[st] = '\0';
    return utf8towcs(NULL, buf, 0);
}



static SEXP 
gregexpr_Regexc(const regex_t *reg, SEXP sstr, int useBytes, int use_WC)
{
    int matchIndex, j, st, foundAll, foundAny, offset, len;
    regmatch_t regmatch[10];
    SEXP ans, matchlen;         /* Return vect and its attribute */
    SEXP matchbuf, matchlenbuf; /* Buffers for storing multiple matches */
    int bufsize = 1024;         /* Starting size for buffers */
    int eflags = 0;
    const char *string = NULL;
    const wchar_t *ws = NULL;

    PROTECT(matchbuf = allocVector(INTSXP, bufsize));
    PROTECT(matchlenbuf = allocVector(INTSXP, bufsize));
    matchIndex = -1;
    foundAll = foundAny = offset = 0;

    if (useBytes) {
	string = CHAR(sstr);
	len = strlen(string);
	use_WC = FALSE; /* to be sure */
    } else if (!use_WC) {
	string = translateChar(sstr);
	/* FIXME perhaps we ought to check validity here */
	len = strlen(string);
     } else {
	ws = wtransChar(sstr);
	len = wcslen(ws);
    }

    while (!foundAll) {
	if ( offset < len &&
	     (!use_WC ? tre_regexecb(reg, string+offset, 1, regmatch, eflags) :
	      tre_regwexec(reg, ws+offset, 1, regmatch, eflags))
	     == 0) {
	    if ((matchIndex + 1) == bufsize) {
		/* Reallocate match buffers */
		int newbufsize = bufsize * 2;
		SEXP tmp;
		tmp = allocVector(INTSXP, 2 * bufsize);
		for (j = 0; j < bufsize; j++)
		    INTEGER(tmp)[j] = INTEGER(matchlenbuf)[j];
		UNPROTECT(1);
		matchlenbuf = tmp;
		PROTECT(matchlenbuf);
		tmp = allocVector(INTSXP, 2 * bufsize);
		for (j = 0; j < bufsize; j++)
		    INTEGER(tmp)[j] = INTEGER(matchbuf)[j];
		matchbuf = tmp;
		UNPROTECT(2);
		PROTECT(matchbuf);
		PROTECT(matchlenbuf);
		bufsize = newbufsize;
	    }
	    matchIndex++;
	    foundAny = 1;
	    st = regmatch[0].rm_so;
	    INTEGER(matchbuf)[matchIndex] = offset + st + 1; /* index from one */
	    INTEGER(matchlenbuf)[matchIndex] = regmatch[0].rm_eo - st;
	    if (INTEGER(matchlenbuf)[matchIndex] == 0)
		offset += st + 1;
	    else
		offset += regmatch[0].rm_eo;
	} else {
	    foundAll = 1;
	    if (!foundAny) {
		matchIndex++;
		INTEGER(matchbuf)[matchIndex] = -1;
		INTEGER(matchlenbuf)[matchIndex] = -1;
	    }
	}
	eflags = REG_NOTBOL;
    }
    PROTECT(ans = allocVector(INTSXP, matchIndex + 1));
    PROTECT(matchlen = allocVector(INTSXP, matchIndex + 1));
    /* copy from buffers */
    for (j = 0; j <= matchIndex; j++) {
	INTEGER(ans)[j] = INTEGER(matchbuf)[j];
	INTEGER(matchlen)[j] = INTEGER(matchlenbuf)[j];
    }
    setAttrib(ans, install("match.length"), matchlen);
    if(useBytes) {
	setAttrib(ans, install("useBytes"), ScalarLogical(TRUE));
    }
    UNPROTECT(4);
    return ans;
}

static SEXP
gregexpr_fixed(const char *pattern, const char *string, 
	       Rboolean useBytes, Rboolean use_UTF8)
{
    int patlen, matchIndex, st, foundAll, foundAny, curpos, j, ansSize, nb=0;
    int slen;
    SEXP ans, matchlen;         /* return vect and its attribute */
    SEXP matchbuf, matchlenbuf; /* buffers for storing multiple matches */
    int bufsize = 1024;         /* starting size for buffers */
    PROTECT(matchbuf = allocVector(INTSXP, bufsize));
    PROTECT(matchlenbuf = allocVector(INTSXP, bufsize));
    if (!useBytes && use_UTF8)
	patlen = utf8towcs(NULL, pattern, 0);
    else if (!useBytes && mbcslocale)
	patlen = mbstowcs(NULL, pattern, 0);
    else
	patlen = strlen(pattern);
    slen = strlen(string);
    foundAll = curpos = st = foundAny = 0;
    st = fgrep_one(pattern, string, useBytes, use_UTF8, &nb);
    matchIndex = -1;
    if (st < 0) {
	INTEGER(matchbuf)[0] = -1;
	INTEGER(matchlenbuf)[0] = -1;
    } else {
	foundAny = 1;
	matchIndex++;
	INTEGER(matchbuf)[matchIndex] = st + 1; /* index from one */
	INTEGER(matchlenbuf)[matchIndex] = patlen;
	while(!foundAll) {
	    string += nb;
	    if (patlen == 0)
		curpos += st + 1;
	    else
		curpos += st + patlen;
	    if (curpos >= slen)
		break;
	    st = fgrep_one(pattern, string, useBytes, use_UTF8, &nb);
	    if (st >= 0) {
		if ((matchIndex + 1) == bufsize) {
		    /* Reallocate match buffers */
		    int newbufsize = bufsize * 2;
		    SEXP tmp;
		    tmp = allocVector(INTSXP, 2 * bufsize);
		    for (j = 0; j < bufsize; j++)
			INTEGER(tmp)[j] = INTEGER(matchlenbuf)[j];
		    UNPROTECT(1);
		    matchlenbuf = tmp;
		    PROTECT(matchlenbuf);
		    tmp = allocVector(INTSXP, 2 * bufsize);
		    for (j = 0; j < bufsize; j++)
			INTEGER(tmp)[j] = INTEGER(matchbuf)[j];
		    matchbuf = tmp;
		    UNPROTECT(2);
		    PROTECT(matchbuf);
		    PROTECT(matchlenbuf);
		    bufsize = newbufsize;
		}
		matchIndex++;
		/* index from one */
		INTEGER(matchbuf)[matchIndex] = curpos + st + 1;
		INTEGER(matchlenbuf)[matchIndex] = patlen;
	    } else foundAll = 1;
	}
    }
    ansSize = foundAny ? (matchIndex + 1) : 1;
    PROTECT(ans = allocVector(INTSXP, ansSize));
    PROTECT(matchlen = allocVector(INTSXP, ansSize));
    /* copy from buffers */
    for (j = 0; j < ansSize; j++) {
	INTEGER(ans)[j] = INTEGER(matchbuf)[j];
	INTEGER(matchlen)[j] = INTEGER(matchlenbuf)[j];
    }
    setAttrib(ans, install("match.length"), matchlen);
    if(useBytes) {
	setAttrib(ans, install("useBytes"), ScalarLogical(TRUE));
    }
    UNPROTECT(4);
    return ans;
}

/* This function is used to convert a single ovector (match_start,
   match_end) pair (in bytes) to a pair of (match_start in 1-indexed
   unicode characters stored in mptr, match_length in number of
   unicode characters stored in lenptr)

   We have to do this once for the match and once for every group, so
   I generalized the method and call it twice from
   extract_match_and_groups to avoid repetitive code.
   
   Toby Dylan Hocking 2011-03-10
*/
static Rboolean 
ovector_extract_start_length(Rboolean use_UTF8,int *ovector,
			     int *mptr,int *lenptr,const char *string)
{
    Rboolean foundAll = FALSE;
    int st = ovector[0];
    *mptr = st + 1; /* index from one */
    *lenptr = ovector[1] - st;
    if (use_UTF8) {
	/* Unfortunately these are in bytes */
	if (st > 0) {
	    *mptr = 1 + getNc(string, st);
	    if (*mptr <= 0) { /* an invalid string */
		*mptr = NA_INTEGER;
		foundAll = TRUE; /* if we get here, we are done */
	    }
	}
	*lenptr = getNc(string + st, *lenptr);
	if (*lenptr < 0) {/* an invalid string */
	    *lenptr = NA_INTEGER;
	    foundAll = TRUE;
	}
    }
    return foundAll;
}

/* this function generalizes the parsing of the "ovector" from pcre
   which contains the match and group start and end bytes. it is
   organized as follows: match_start match_end group1_start group1_end
   group2_start group2_end ... we process these in regexpr and
   gregexpr, so I made this function to avoid duplicating code between
   the 2. 

   Toby Dylan Hocking 2011-03-10 */
static Rboolean 
extract_match_and_groups(Rboolean use_UTF8, int *ovector, int capture_count,
			 int *mptr, int *lenptr, int *cptr, int *clenptr,
			 const char *string, int capture_stride)
{
    Rboolean foundAll =
	ovector_extract_start_length(use_UTF8, ovector, mptr, lenptr, string);
    /* also extract capture locations */
    for(int i = 0; i < capture_count; i++) {
	int ind = capture_stride*i;
	ovector_extract_start_length(use_UTF8, ovector+2*(i+1),
				     cptr+ind, clenptr+ind, string);
    }
    return foundAll;
}

static SEXP
gregexpr_perl(const char *pattern, const char *string,
	      pcre *re_pcre, pcre_extra *re_pe,
	      Rboolean useBytes, Rboolean use_UTF8,
	      int *ovector, int ovector_size,
	      int capture_count, SEXP capture_names)
{
    Rboolean foundAll = FALSE, foundAny = FALSE;
    int matchIndex = -1, start = 0;
    SEXP ans, matchlen;         /* return vect and its attribute */
    SEXP capturebuf, capturelenbuf;
    SEXP matchbuf, matchlenbuf; /* buffers for storing multiple matches */
    int bufsize = 1024;         /* starting size for buffers */
    PROTECT_INDEX cb, clb, mb, mlb;

    PROTECT_WITH_INDEX(capturebuf = 
		       allocVector(INTSXP, bufsize*capture_count), &cb);
    PROTECT_WITH_INDEX(capturelenbuf = 
		       allocVector(INTSXP, bufsize*capture_count), &clb);
    PROTECT_WITH_INDEX(matchbuf = allocVector(INTSXP, bufsize), &mb);
    PROTECT_WITH_INDEX(matchlenbuf = allocVector(INTSXP, bufsize), &mlb);
    while (!foundAll) {
	int rc, slen = strlen(string);
	rc = pcre_exec(re_pcre, re_pe, string, slen, start, 0, ovector,
		       ovector_size);
	if (rc >= 0) {
	    if ((matchIndex + 1) == bufsize) {
		/* Reallocate match buffers */
		int newbufsize = bufsize * 2;
		SEXP tmp;
		tmp = allocVector(INTSXP, newbufsize);
		for (int j = 0; j < bufsize; j++) /* or use memcpy */
		    INTEGER(tmp)[j] = INTEGER(matchlenbuf)[j];
		REPROTECT(matchlenbuf = tmp, mlb);
		tmp = allocVector(INTSXP, newbufsize);
		for (int j = 0; j < bufsize; j++)  /* or use memcpy */
		    INTEGER(tmp)[j] = INTEGER(matchbuf)[j];
		REPROTECT(matchbuf = tmp, mb);
		if (capture_count) {
		    tmp = allocVector(INTSXP, newbufsize*capture_count);
		    for(int j = 0; j < bufsize; j++)
			for(int i = 0; i < capture_count; i++)
			    INTEGER(tmp)[j + newbufsize*i] = 
				INTEGER(capturebuf)[j + bufsize*i];
		    REPROTECT(capturebuf = tmp, cb);
		    tmp = allocVector(INTSXP, newbufsize*capture_count);
		    for(int j = 0; j < bufsize; j++)
			for(int i = 0; i < capture_count; i++)
			    INTEGER(tmp)[j + newbufsize*i] = 
				INTEGER(capturelenbuf)[j + bufsize*i];
		    REPROTECT(capturelenbuf =  tmp, clb);
		}
		bufsize = newbufsize;
	    }
	    matchIndex++;
	    foundAny = TRUE;
	    foundAll = 
		extract_match_and_groups(use_UTF8, ovector, capture_count,
					 INTEGER(matchbuf) + matchIndex,
					 INTEGER(matchlenbuf) + matchIndex,
					 INTEGER(capturebuf) + matchIndex,
					 INTEGER(capturelenbuf) + matchIndex,
					 string, bufsize);
	    /* we need to advance 'start' in bytes */
	    if (ovector[1] - ovector[0] == 0)
		start = ovector[0] + 1;
	    else
		start = ovector[1];
	    if (start >= slen) foundAll = 1;
	} else {
	    foundAll = TRUE;
	    if (!foundAny) matchIndex = 0;
	}
    }
    PROTECT(ans = allocVector(INTSXP, matchIndex + 1));
    /* Protect in case install("match.length") allocates */
    PROTECT(matchlen = allocVector(INTSXP, matchIndex + 1));
    setAttrib(ans, install("match.length"), matchlen);
    if(useBytes) {
	setAttrib(ans, install("useBytes"), ScalarLogical(TRUE));
    }
    UNPROTECT(1);
    if (foundAny) {
	for (int j = 0; j <= matchIndex; j++) {
	    INTEGER(ans)[j] = INTEGER(matchbuf)[j];
	    INTEGER(matchlen)[j] = INTEGER(matchlenbuf)[j];
	}
    } else
	INTEGER(ans)[0] = INTEGER(matchlen)[0] = -1;

    if (capture_count) {
	SEXP capture, capturelen, dmn;
	PROTECT(capture = allocMatrix(INTSXP, matchIndex+1, capture_count));
	PROTECT(capturelen = allocMatrix(INTSXP, matchIndex+1, capture_count));
	PROTECT(dmn = allocVector(VECSXP, 2));
	SET_VECTOR_ELT(dmn, 1, capture_names);
	setAttrib(capture, R_DimNamesSymbol, dmn);
	setAttrib(capturelen, R_DimNamesSymbol, dmn);
	if (foundAny) {
	    for (int j = 0; j <= matchIndex; j++)
		for(int i = 0; i < capture_count; i++) {
		    int return_index = j + (matchIndex+1) * i;
		    int buffer_index = j + bufsize * i;
		    INTEGER(capture)[return_index] =
			INTEGER(capturebuf)[buffer_index];
		    INTEGER(capturelen)[return_index] =
			INTEGER(capturelenbuf)[buffer_index];
		}
	} else
	    for(int i = 0; i < capture_count; i++)
		INTEGER(capture)[i] = INTEGER(capturelen)[i] = -1;
	setAttrib(ans, install("capture.start"), capture);
	setAttrib(ans, install("capture.length"), capturelen);
	setAttrib(ans, install("capture.names"), capture_names);
	UNPROTECT(3);
    }
    UNPROTECT(5); /* 4 with indices, ans */
    return ans;
}

static SEXP gregexpr_NAInputAns(void)
{
    SEXP ans, matchlen;
    PROTECT(ans = allocVector(INTSXP, 1));
    PROTECT(matchlen = allocVector(INTSXP, 1));
    INTEGER(ans)[0] = INTEGER(matchlen)[0] = R_NaInt;
    setAttrib(ans, install("match.length"), matchlen);
    UNPROTECT(2);
    return ans;
}

static SEXP gregexpr_BadStringAns(void)
{
    SEXP ans, matchlen;
    PROTECT(ans = allocVector(INTSXP, 1));
    PROTECT(matchlen = allocVector(INTSXP, 1));
    INTEGER(ans)[0] = INTEGER(matchlen)[0] = -1;
    setAttrib(ans, install("match.length"), matchlen);
    UNPROTECT(2);
    return ans;
}

SEXP attribute_hidden do_regexpr(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP pat, text, ans;
    regex_t reg;
    regmatch_t regmatch[10];
    int i, rc, n, igcase_opt, perl_opt, fixed_opt, useBytes;
    const char *spat = NULL; /* -Wall */
    const char *s = NULL;
    pcre *re_pcre = NULL /* -Wall */;
    pcre_extra *re_pe = NULL;
    const unsigned char *tables = NULL /* -Wall */;
    Rboolean use_UTF8 = FALSE, use_WC = FALSE;
    const void *vmax;
    int capture_count, *ovector = NULL, ovector_size = 0, /* -Wall */
	name_count, name_entry_size, info_code;
    char *name_table;
    SEXP capture_names = R_NilValue;

    checkArity(op, args);
    pat = CAR(args); args = CDR(args);
    text = CAR(args); args = CDR(args);
    igcase_opt = asLogical(CAR(args)); args = CDR(args);
    perl_opt = asLogical(CAR(args)); args = CDR(args);
    fixed_opt = asLogical(CAR(args)); args = CDR(args);
    useBytes = asLogical(CAR(args)); args = CDR(args);
    if (igcase_opt == NA_INTEGER) igcase_opt = 0;
    if (perl_opt == NA_INTEGER) perl_opt = 0;
    if (fixed_opt == NA_INTEGER) fixed_opt = 0;
    if (useBytes == NA_INTEGER) useBytes = 0;
    if (fixed_opt && igcase_opt)
	warning(_("argument '%s' will be ignored"), "ignore.case = TRUE");
    if (fixed_opt && perl_opt) {
	warning(_("argument '%s' will be ignored"), "perl = TRUE");
	perl_opt = 0;
    }

    /* Note that excluding NAs differs from grep/sub */
    if (!isString(pat) || length(pat) < 1 || STRING_ELT(pat, 0) == NA_STRING)
	error(_("invalid '%s' argument"), "pattern");
    if (length(pat) > 1)
	warning(_("argument '%s' has length > 1 and only the first element will be used"), "pattern");

    if (!isString(text))
	error(_("invalid '%s' argument"), "text");

    n = LENGTH(text);
    if (!useBytes) {
	Rboolean onlyASCII = strIsASCII(CHAR(STRING_ELT(pat, 0)));
	if (onlyASCII)
	    for (i = 0; i < n; i++)
		if (!strIsASCII(CHAR(STRING_ELT(text, i)))) {
		    onlyASCII = FALSE;
		    break;
		}
	useBytes = onlyASCII;
    }
    if (!useBytes) {
	Rboolean haveBytes = IS_BYTES(STRING_ELT(pat, 0));
	if (!haveBytes)
	    for (i = 0; i < n; i++)
		if (IS_BYTES(STRING_ELT(text, i))) {
		    haveBytes = TRUE;
		    break;
		}
	if(haveBytes) {
	    useBytes = TRUE;
	}
    }
    if (!useBytes && !use_UTF8) {
	/* As from R 2.10.0 we use UTF-8 mode in PCRE in all MBCS locales,
	   and as from 2.11.0 in TRE too. */
	if (!fixed_opt && mbcslocale) use_UTF8 = TRUE;
	else if (IS_UTF8(STRING_ELT(pat, 0))) use_UTF8 = TRUE;
	if (!use_UTF8)
	    for (i = 0; i < n; i++)
		if (IS_UTF8(STRING_ELT(text, i))) {
		    use_UTF8 = TRUE;
		    break;
		}
    }

    if (!fixed_opt && !perl_opt) {
	/* if we have non-ASCII text in a DBCS locale, we need to use wchar */
	if (!useBytes && mbcslocale && !utf8locale) use_UTF8 =TRUE;
	use_WC = use_UTF8; use_UTF8 = FALSE; 
    }

    if (useBytes)
	spat = CHAR(STRING_ELT(pat, 0));
    else if (use_WC) ;
    else if (use_UTF8) {
	spat = translateCharUTF8(STRING_ELT(pat, 0));
	if (!utf8Valid(spat)) error(_("regular expression is invalid UTF-8"));
    } else {
	spat = translateChar(STRING_ELT(pat, 0));
	if (mbcslocale && !mbcsValid(spat))
	    error(_("regular expression is invalid in this locale"));
    }

    if (fixed_opt) ; 
    else if (perl_opt) {
	int cflags = 0, erroffset;
	const char *errorptr;
	if (igcase_opt) cflags |= PCRE_CASELESS;
	if (!useBytes && use_UTF8) cflags |= PCRE_UTF8;
	tables = pcre_maketables();
	re_pcre = pcre_compile(spat, cflags, &errorptr, &erroffset, tables);
	if (!re_pcre) {
	    if (errorptr)
		warning(_("PCRE pattern compilation error\n\t'%s'\n\tat '%s'\n"),
			errorptr, spat+erroffset);
	    error(_("invalid regular expression '%s'"), spat);
	}
	if (n > 10) {
	    re_pe = pcre_study(re_pcre, 0, &errorptr);
	    if (errorptr)
		warning(_("PCRE pattern study error\n\t'%s'\n"), errorptr);
	}
	/* also extract info for named groups */
	pcre_fullinfo(re_pcre, re_pe, PCRE_INFO_NAMECOUNT, &name_count);
	pcre_fullinfo(re_pcre, re_pe, PCRE_INFO_NAMEENTRYSIZE, &name_entry_size);
	pcre_fullinfo(re_pcre, re_pe, PCRE_INFO_NAMETABLE, &name_table);
	info_code = 
	    pcre_fullinfo(re_pcre, re_pe, PCRE_INFO_CAPTURECOUNT, 
			  &capture_count);
	if(info_code < 0)
	    error(_("pcre_fullinfo returned '%d' "), info_code);
	ovector_size = (capture_count + 1) * 3;
	ovector = (int *) malloc(ovector_size*sizeof(int));
	SEXP thisname;
	PROTECT(capture_names = allocVector(STRSXP, capture_count));
	for(i = 0; i < name_count; i++) {
	    char *entry = name_table + name_entry_size * i;
	    PROTECT(thisname = mkChar(entry + 2));
	    int capture_num = (entry[0]<<8) + entry[1] - 1;
	    SET_STRING_ELT(capture_names, capture_num, thisname);
	    UNPROTECT(1);
	}
    } else {
	int cflags = REG_EXTENDED;
	if (igcase_opt) cflags |= REG_ICASE;
	if (!use_WC)
	    rc = tre_regcompb(&reg, spat, cflags);
	else
	    rc = tre_regwcomp(&reg, wtransChar(STRING_ELT(pat, 0)), cflags);
	if (rc) reg_report(rc, &reg, spat);
    }

    if (PRIMVAL(op) == 0) { /* regexpr */
	SEXP matchlen, capture_start = R_NilValue, capturelen = R_NilValue;
	PROTECT(ans = allocVector(INTSXP, n));
	/* Protect in case install("match.length") allocates */
	PROTECT(matchlen = allocVector(INTSXP, n));
	setAttrib(ans, install("match.length"), matchlen);
	if(useBytes) {
	    setAttrib(ans, install("useBytes"), ScalarLogical(TRUE));
	}
	UNPROTECT(1);
	if (perl_opt && capture_count) {
	    SEXP dmn;
	    PROTECT(dmn = allocVector(VECSXP, 2));
	    SET_VECTOR_ELT(dmn, 1, capture_names);
	    PROTECT(capture_start = allocMatrix(INTSXP, n, capture_count));
	    setAttrib(capture_start, R_DimNamesSymbol, dmn);
	    setAttrib(ans, install("capture.start"), capture_start);
	    PROTECT(capturelen = allocMatrix(INTSXP, n, capture_count));
	    setAttrib(capturelen, R_DimNamesSymbol, dmn);
	    setAttrib(ans, install("capture.length"), capturelen);
	    setAttrib(ans, install("capture.names"), capture_names);
	    UNPROTECT(3);
	}

	vmax = vmaxget();
	for (i = 0 ; i < n ; i++) {
	    if (STRING_ELT(text, i) == NA_STRING) {
		INTEGER(matchlen)[i] = INTEGER(ans)[i] = NA_INTEGER;
	    } else {
		if (useBytes)
		    s = CHAR(STRING_ELT(text, i));
		else if (use_WC) ;
		else if (use_UTF8) {
		    s = translateCharUTF8(STRING_ELT(text, i));
		    if (!utf8Valid(s)) {
			warning(_("input string %d is invalid UTF-8"), i+1);
			INTEGER(ans)[i] = INTEGER(matchlen)[i] = -1;
			continue;
		    }
		} else {
		    s = translateChar(STRING_ELT(text, i));
		    if (mbcslocale && !mbcsValid(s)) {
			warning(_("input string %d is invalid in this locale"), i+1);
			INTEGER(ans)[i] = INTEGER(matchlen)[i] = -1;
			continue;
		    }
		}
		if (fixed_opt) {
		    int st = fgrep_one(spat, s, useBytes, use_UTF8, NULL);
		    INTEGER(ans)[i] = (st > -1)?(st+1):-1;
		    if (!useBytes && use_UTF8) {
			INTEGER(matchlen)[i] = INTEGER(ans)[i] >= 0 ?
			    utf8towcs(NULL, spat, 0):-1;
		    } else if (!useBytes && mbcslocale) {
			INTEGER(matchlen)[i] = INTEGER(ans)[i] >= 0 ?
			    mbstowcs(NULL, spat, 0):-1;
		    } else
			INTEGER(matchlen)[i] = INTEGER(ans)[i] >= 0 ?
			    strlen(spat):-1;
		} else if (perl_opt) {
		    int rc;
		    rc = pcre_exec(re_pcre, re_pe, s, strlen(s), 0, 0, 
				   ovector, ovector_size);
		    if (rc >= 0) {
			extract_match_and_groups(use_UTF8, ovector, 
						 capture_count,
						 INTEGER(ans) + i,
						 INTEGER(matchlen) + i,
						 INTEGER(capture_start) + i,
						 INTEGER(capturelen) + i,
						 s, n);
		    } else {
			INTEGER(ans)[i] = INTEGER(matchlen)[i] = -1;
			for(int cn = 0; cn < capture_count; cn++) {
			    int ind = i + cn*n;
			    INTEGER(capture_start)[ind] = 
				INTEGER(capturelen)[ind] = -1;
			}
		    }
		} else {
		    if (!use_WC)
			rc = tre_regexecb(&reg, s, 1, regmatch, 0);
		    else
			rc = tre_regwexec(&reg, wtransChar(STRING_ELT(text, i)),
					  1, regmatch, 0);
		    if (rc == 0) {
			int st = regmatch[0].rm_so;
			INTEGER(ans)[i] = st + 1; /* index from one */
			INTEGER(matchlen)[i] = regmatch[0].rm_eo - st;
		    } else INTEGER(ans)[i] = INTEGER(matchlen)[i] = -1;
		}
	    }
	    vmaxset(vmax);
	}
    } else {
	SEXP elt;
	PROTECT(ans = allocVector(VECSXP, n));
	vmax = vmaxget();
	for (i = 0 ; i < n ; i++) {
	    if (STRING_ELT(text, i) == NA_STRING) {
		elt = gregexpr_NAInputAns();
	    } else {
		if (fixed_opt || perl_opt) {
		    if (useBytes)
			s = CHAR(STRING_ELT(text, i));
		    else if (use_UTF8) {
			s = translateCharUTF8(STRING_ELT(text, i));
		    } else
			s = translateChar(STRING_ELT(text, i));
		    if (!useBytes && !use_UTF8 && mbcslocale && !mbcsValid(s)) {
			warning(_("input string %d is invalid in this locale"),
				i+1);
			elt = gregexpr_BadStringAns();
		    } else {
			if (fixed_opt)
			    elt = gregexpr_fixed(spat, s, useBytes, use_UTF8);
			else
			    elt = gregexpr_perl(spat, s, re_pcre, re_pe, 
						useBytes, use_UTF8, ovector,
						ovector_size, capture_count,
						capture_names);
		    }
		} else
		    elt = gregexpr_Regexc(&reg, STRING_ELT(text, i), 
					  useBytes, use_WC);
	    }
	    SET_VECTOR_ELT(ans, i, elt);
	    vmaxset(vmax);
	}
    }

    if (fixed_opt) ; 
    else if (perl_opt) {
	if (re_pe) pcre_free(re_pe);
	pcre_free(re_pcre);
	pcre_free((void *)tables);
	UNPROTECT(1);
	free(ovector);
    } else
	tre_regfree(&reg);

    UNPROTECT(1);
    return ans;
}

SEXP attribute_hidden do_regexec(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP pat, vec, ans, matchpos, matchlen;
    int opt_icase, opt_fixed, useBytes;

    Rboolean haveBytes, useWC = FALSE;
    const char *s, *t;
    const void *vmax = NULL;
    
    regex_t reg;
    size_t nmatch;
    regmatch_t *pmatch;
    int i, j, n, so;
    int rc, cflags = REG_EXTENDED;

    checkArity(op, args);

    pat = CAR(args); args = CDR(args);
    vec = CAR(args); args = CDR(args);
    opt_icase = asLogical(CAR(args)); args = CDR(args);
    opt_fixed = asLogical(CAR(args)); args = CDR(args);
    useBytes = asLogical(CAR(args));
    
    if(opt_icase == NA_INTEGER) opt_icase = 0;
    if(opt_fixed == NA_INTEGER) opt_fixed = 0;
    if(useBytes == NA_INTEGER) useBytes = 0;
    if(opt_fixed && opt_icase) {
	warning(_("argument '%s' will be ignored"),
		"ignore.case = TRUE");
	opt_icase = 0;
    }
    if(opt_fixed) cflags |= REG_LITERAL;
    if(opt_icase) cflags |= REG_ICASE;

    if(!isString(pat) ||
       (length(pat) < 1) ||
       (STRING_ELT(pat, 0) == NA_STRING))
	error(_("invalid '%s' argument"), "pattern");
    if(length(pat) > 1)
	warning(_("argument '%s' has length > 1 and only the first element will be used"), "pattern");
    
    if(!isString(vec))
	error(_("invalid '%s' argument"), "text");

    n = LENGTH(vec);

    if(!useBytes) {
        haveBytes = IS_BYTES(STRING_ELT(pat, 0));
	if(!haveBytes)
            for(i = 0; i < n; i++) {
                if(IS_BYTES(STRING_ELT(vec, i))) {
                    haveBytes = TRUE;
                    break;
                }
	    }
	if (haveBytes) useBytes = TRUE;
    }

    if(!useBytes) {
        useWC = !strIsASCII(CHAR(STRING_ELT(pat, 0)));
        if(!useWC) {
            for(i = 0 ; i < n ; i++) {
                if(STRING_ELT(vec, i) == NA_STRING) continue;
                if(!strIsASCII(CHAR(STRING_ELT(vec, i)))) {
                    useWC = TRUE;
                    break;
                }
            }
        }
    }
    
    if(useBytes)
	rc = tre_regcompb(&reg, CHAR(STRING_ELT(pat, 0)), cflags);
    else if (useWC)
	rc = tre_regwcomp(&reg, wtransChar(STRING_ELT(pat, 0)), cflags);
    else {
        s = translateChar(STRING_ELT(pat, 0));
        if(mbcslocale && !mbcsValid(s))
            error(_("regular expression is invalid in this locale"));
        rc = tre_regcomp(&reg, s, cflags);
    }
    if(rc) {
        char errbuf[1001];
        tre_regerror(rc, &reg, errbuf, 1001);
        error(_("regcomp error: '%s'"), errbuf);
    }

    nmatch = reg.re_nsub + 1;

    pmatch = (regmatch_t *) malloc(nmatch * sizeof(regmatch_t));

    PROTECT(ans = allocVector(VECSXP, n));

    for(i = 0; i < n; i++) {
	if(STRING_ELT(vec, i) == NA_STRING) {
	    PROTECT(matchpos = ScalarInteger(NA_INTEGER));
	    setAttrib(matchpos, install("match.length"),
		      ScalarInteger(NA_INTEGER));
	    SET_VECTOR_ELT(ans, i, matchpos);
	    UNPROTECT(1);
	} else {
	    vmax = vmaxget();
	    if(useBytes)
		rc = tre_regexecb(&reg, CHAR(STRING_ELT(vec, i)),
				  nmatch, pmatch, 0);
	    else if(useWC) {
		rc = tre_regwexec(&reg, wtransChar(STRING_ELT(vec, i)),
				  nmatch, pmatch, 0);
		vmaxset(vmax);
	    }
	    else {
		t = translateChar(STRING_ELT(vec, i));
		if (mbcslocale && !mbcsValid(t))
		    error(_("input string %d is invalid in this locale"),
			  i + 1);
		rc = tre_regexec(&reg, t,
				 nmatch, pmatch, 0);
		vmaxset(vmax);		
	    }
	    if(rc == REG_OK) {
		PROTECT(matchpos = allocVector(INTSXP, nmatch));
		PROTECT(matchlen = allocVector(INTSXP, nmatch));
		for(j = 0; j < nmatch; j++) {
		    so = pmatch[j].rm_so;
		    INTEGER(matchpos)[j] = so + 1;
		    INTEGER(matchlen)[j] = pmatch[j].rm_eo - so;
		}
		setAttrib(matchpos, install("match.length"), matchlen);
		if(useBytes)
		    setAttrib(matchpos, install("useBytes"),
			      ScalarLogical(TRUE));
		SET_VECTOR_ELT(ans, i, matchpos);
		UNPROTECT(2);
	    } else {
		/* No match (or could there be an error?). */
		/* Alternatively, could return nmatch -1 values.
		*/
		PROTECT(matchpos = ScalarInteger(-1));
		PROTECT(matchlen = ScalarInteger(-1));
		setAttrib(matchpos, install("match.length"), matchlen);
		SET_VECTOR_ELT(ans, i, matchpos);
		UNPROTECT(2);
	    }
	}
    }
    
    free(pmatch);

    tre_regfree(&reg);

    UNPROTECT(1);

    return ans;
}
