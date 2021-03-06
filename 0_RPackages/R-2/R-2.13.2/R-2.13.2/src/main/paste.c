/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2011  The R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
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
 *
 *
 *  See ./printutils.c	 for general remarks on Printing
 *                       and the Encode.. utils.
 *
 *  See ./format.c	 for the  format_Foo_  functions.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Defn.h"
#define imax2(x, y) ((x < y) ? y : x)

#include "Print.h"
#include "RBufferUtils.h"
static R_StringBuffer cbuff = {NULL, 0, MAXELTSIZE};

/*  .Internal(paste(args, sep, collapse))
 *
 * do_paste uses two passes to paste the arguments (in CAR(args)) together.
 * The first pass calculates the width of the paste buffer,
 * then it is alloc-ed and the second pass stuffs the information in.
 */

/* Note that NA_STRING is not handled separately here.  This is
   deliberate -- see ?paste -- and implicitly coerces it to "NA"
*/
SEXP attribute_hidden do_paste(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, collapse, sep, x;
    int i, j, k, maxlen, nx, pwidth, sepw, u_sepw, ienc;
    const char *s, *csep, *cbuf, *u_csep=NULL;
    char *buf;
    Rboolean allKnown, anyKnown, sepASCII, sepKnown, use_UTF8, sepUTF8,
	use_Bytes, sepBytes;
    const void *vmax;

    checkArity(op, args);

    /* We use formatting and so we must initialize printing. */

    PrintDefaults();

    /* Check the arguments */

    x = CAR(args);
    if (!isVectorList(x))
	error(_("invalid first argument"));
    nx = length(x);


    sep = CADR(args);
    if (!isString(sep) || LENGTH(sep) <= 0 || STRING_ELT(sep, 0) == NA_STRING)
	error(_("invalid separator"));
    sep = STRING_ELT(sep, 0);
    csep = translateChar(sep);
    u_sepw = sepw = strlen(csep);
    sepASCII = strIsASCII(csep);
    sepKnown = ENC_KNOWN(sep) > 0;
    sepUTF8 = IS_UTF8(sep);
    sepBytes = IS_BYTES(sep);

    collapse = CADDR(args);
    if (!isNull(collapse))
	if(!isString(collapse) || LENGTH(collapse) <= 0 ||
	   STRING_ELT(collapse, 0) == NA_STRING)
	    error(_("invalid '%s' argument"), "collapse");
    if(nx == 0)
	return (!isNull(collapse)) ? mkString("") : allocVector(STRSXP, 0);


    /* Maximum argument length, coerce if needed */

    maxlen = 0;
    for (j = 0; j < nx; j++) {
	if (!isString(VECTOR_ELT(x, j))) {
	    /* formerly in R code: moved to C for speed */
	    SEXP call, xj = VECTOR_ELT(x, j);
	    if(OBJECT(xj)) { /* method dispatch */
		PROTECT(call = lang2(install("as.character"), xj));
		SET_VECTOR_ELT(x, j, eval(call, env));
		UNPROTECT(1);
	    } else if (isSymbol(xj))
		SET_VECTOR_ELT(x, j, ScalarString(PRINTNAME(xj)));
	    else
		SET_VECTOR_ELT(x, j, coerceVector(xj, STRSXP));

	    if (!isString(VECTOR_ELT(x, j)))
		error(_("non-string argument to Internal paste"));
	}
	if(length(VECTOR_ELT(x, j)) > maxlen)
	    maxlen = length(VECTOR_ELT(x, j));
    }
    if(maxlen == 0)
	return (!isNull(collapse)) ? mkString("") : allocVector(STRSXP, 0);

    PROTECT(ans = allocVector(STRSXP, maxlen));

    for (i = 0; i < maxlen; i++) {
	/* Strategy for marking the encoding: if all inputs (including
	 * the separator) are ASCII, so is the output and we don't
	 * need to mark.  Otherwise if all non-ASCII inputs are of
	 * declared encoding, we should mark.
	 * Need to be careful only to include separator if it is used.
	 */
	anyKnown = FALSE; allKnown = TRUE; use_UTF8 = FALSE; use_Bytes = FALSE;
	if(nx > 1) {
	    allKnown = sepKnown || sepASCII;
	    anyKnown = sepKnown;
	    use_UTF8 = sepUTF8;
	    use_Bytes = sepBytes;
	}

	pwidth = 0;
	for (j = 0; j < nx; j++) {
	    k = length(VECTOR_ELT(x, j));
	    if (k > 0) {
		SEXP cs = STRING_ELT(VECTOR_ELT(x, j), i % k);
		if(IS_UTF8(cs)) use_UTF8 = TRUE;
		if(IS_BYTES(cs)) use_Bytes = TRUE;
	    }
	}
	if (use_Bytes) use_UTF8 = FALSE;
	vmax = vmaxget();
	for (j = 0; j < nx; j++) {
	    k = length(VECTOR_ELT(x, j));
	    if (k > 0) {
		if(use_Bytes)
		    pwidth += strlen(CHAR(STRING_ELT(VECTOR_ELT(x, j), i % k)));
		else if(use_UTF8)
		    pwidth += strlen(translateCharUTF8(STRING_ELT(VECTOR_ELT(x, j), i % k)));
		else
		    pwidth += strlen(translateChar(STRING_ELT(VECTOR_ELT(x, j), i % k)));
		vmaxset(vmax);
	    }
	}
	if (use_UTF8 && !u_csep) {
	    u_csep = translateCharUTF8(sep);
	    u_sepw = strlen(u_csep);
	}
	pwidth += (nx - 1) * (use_UTF8 ? u_sepw : sepw);
	cbuf = buf = R_AllocStringBuffer(pwidth, &cbuff);
	vmax = vmaxget();
	for (j = 0; j < nx; j++) {
	    k = length(VECTOR_ELT(x, j));
	    if (k > 0) {
		SEXP cs = STRING_ELT(VECTOR_ELT(x, j), i % k);
		if (use_UTF8) {
		    s = translateCharUTF8(cs);
		    strcpy(buf, s);
		    buf += strlen(s);
		} else {
		    s = use_Bytes ? CHAR(cs) : translateChar(cs);
		    strcpy(buf, s);
		    buf += strlen(s);
		    allKnown = allKnown && (strIsASCII(s) || (ENC_KNOWN(cs)> 0));
		    anyKnown = anyKnown || (ENC_KNOWN(cs)> 0);
		}
	    }
	    if (j != nx - 1 && sepw != 0) {
		if (use_UTF8) {
		    strcpy(buf, u_csep);
		    buf += u_sepw;
		} else {
		    strcpy(buf, csep);
		    buf += sepw;
		}
	    }
	    vmax = vmaxget();
	}
	ienc = 0;
	if(use_UTF8) ienc = CE_UTF8;
	else if(use_Bytes) ienc = CE_BYTES;
	else if(anyKnown && allKnown) {
	    if(known_to_be_latin1) ienc = CE_LATIN1;
	    if(known_to_be_utf8) ienc = CE_UTF8;
	}
	SET_STRING_ELT(ans, i, mkCharCE(cbuf, ienc));
    }

    /* Now collapse, if required. */

    if(collapse != R_NilValue && (nx = LENGTH(ans)) > 0) {	
	sep = STRING_ELT(collapse, 0);
	use_UTF8 = IS_UTF8(sep);
	use_Bytes = IS_BYTES(sep);
	for (i = 0; i < nx; i++) {
	    if(IS_UTF8(STRING_ELT(ans, i))) use_UTF8 = TRUE;
	    if(IS_BYTES(STRING_ELT(ans, i))) use_Bytes = TRUE;
	}
	if(use_Bytes) {
	    csep = CHAR(sep);
	    use_UTF8 = FALSE;
	} else if(use_UTF8)
	    csep = translateCharUTF8(sep);
	else
	    csep = translateChar(sep);
	sepw = strlen(csep);
	anyKnown = ENC_KNOWN(sep) > 0;
	allKnown = anyKnown || strIsASCII(csep);
	pwidth = 0;
	vmax = vmaxget();
	for (i = 0; i < nx; i++)
	    if(use_UTF8) {
		pwidth += strlen(translateCharUTF8(STRING_ELT(ans, i)));
		vmaxset(vmax);
	    } else /* already translated */
		pwidth += strlen(CHAR(STRING_ELT(ans, i)));
	pwidth += (nx - 1) * sepw;
	cbuf = buf = R_AllocStringBuffer(pwidth, &cbuff);
	vmax = vmaxget();
	for (i = 0; i < nx; i++) {
	    if(i > 0) {
		strcpy(buf, csep);
		buf += sepw;
	    }
	    if(use_UTF8)
		s = translateCharUTF8(STRING_ELT(ans, i));
	    else /* already translated */
		s = CHAR(STRING_ELT(ans, i));
	    strcpy(buf, s);
	    while (*buf)
		buf++;
	    allKnown = allKnown &&
		(strIsASCII(s) || (ENC_KNOWN(STRING_ELT(ans, i)) > 0));
	    anyKnown = anyKnown || (ENC_KNOWN(STRING_ELT(ans, i)) > 0);
	    if(use_UTF8) vmaxset(vmax);
	}
	UNPROTECT(1);
	ienc = CE_NATIVE;
	if(use_UTF8) ienc = CE_UTF8;
	else if(use_Bytes) ienc = CE_BYTES;
	else if(anyKnown && allKnown) {
	    if(known_to_be_latin1) ienc = CE_LATIN1;
	    if(known_to_be_utf8) ienc = CE_UTF8;
	}
	PROTECT(ans = allocVector(STRSXP, 1));
	SET_STRING_ELT(ans, 0, mkCharCE(cbuf, ienc));
    }
    R_FreeStringBufferL(&cbuff);
    UNPROTECT(1);
    return ans;
}

SEXP attribute_hidden do_filepath(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, sep, x;
    int i, j, k, ln, maxlen, nx, nzero, pwidth, sepw;
    const char *s, *csep, *cbuf; char *buf;

    checkArity(op, args);

    /* Check the arguments */

    x = CAR(args);
    if (!isVectorList(x))
	error(_("invalid first argument"));
    nx = length(x);
    if(nx == 0) return allocVector(STRSXP, 0);


    sep = CADR(args);
    if (!isString(sep) || LENGTH(sep) <= 0 || STRING_ELT(sep, 0) == NA_STRING)
	error(_("invalid separator"));
    sep = STRING_ELT(sep, 0);
    csep = CHAR(sep);
    sepw = strlen(csep); /* hopefully 1 */

    /* Any zero-length argument gives zero-length result */
    maxlen = 0; nzero = 0;
    for (j = 0; j < nx; j++) {
	if (!isString(VECTOR_ELT(x, j))) {
	    /* formerly in R code: moved to C for speed */
	    SEXP call, xj = VECTOR_ELT(x, j);
	    if(OBJECT(xj)) { /* method dispatch */
		PROTECT(call = lang2(install("as.character"), xj));
		SET_VECTOR_ELT(x, j, eval(call, env));
		UNPROTECT(1);
	    } else if (isSymbol(xj))
		SET_VECTOR_ELT(x, j, ScalarString(PRINTNAME(xj)));
	    else
		SET_VECTOR_ELT(x, j, coerceVector(xj, STRSXP));

	    if (!isString(VECTOR_ELT(x, j)))
		error(_("non-string argument to Internal paste"));
	}
	ln = length(VECTOR_ELT(x, j));
	if(ln > maxlen) maxlen = ln;
	if(ln == 0) {nzero++; break;}
    }
    if(nzero || maxlen == 0) return allocVector(STRSXP, 0);

    PROTECT(ans = allocVector(STRSXP, maxlen));

    for (i = 0; i < maxlen; i++) {
	pwidth = 0;
	for (j = 0; j < nx; j++) {
	    k = length(VECTOR_ELT(x, j));
	    pwidth += strlen(translateChar(STRING_ELT(VECTOR_ELT(x, j), i % k)));
	}
	pwidth += (nx - 1) * sepw;
	cbuf = buf = R_AllocStringBuffer(pwidth, &cbuff);
	for (j = 0; j < nx; j++) {
	    k = length(VECTOR_ELT(x, j));
	    if (k > 0) {
		s = translateChar(STRING_ELT(VECTOR_ELT(x, j), i % k));
		strcpy(buf, s);
		buf += strlen(s);
	    }
	    if (j != nx - 1 && sepw != 0) {
		strcpy(buf, csep);
		buf += sepw;
	    }
	}
	SET_STRING_ELT(ans, i, mkChar(cbuf));
    }
    R_FreeStringBufferL(&cbuff);
    UNPROTECT(1);
    return ans;
}

/* format.default(x, trim, digits, nsmall, width, justify, na.encode,
		  scientific) */
SEXP attribute_hidden do_format(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP l, x, y, swd;
    int i, il, n, digits, trim = 0, nsmall = 0, wd = 0, adj = -1, na, sci = 0;
    int w, d, e;
    int wi, di, ei, scikeep;
    const char *strp;

    checkArity(op, args);
    PrintDefaults();
    scikeep = R_print.scipen;

    if (isEnvironment(x = CAR(args))) {
	PROTECT(y = allocVector(STRSXP, 1));
	SET_STRING_ELT(y, 0, mkChar(EncodeEnvironment(x)));
	UNPROTECT(1);
	return y;
    }
    else if (!isVector(x))
	error(_("first argument must be atomic"));
    args = CDR(args);

    trim = asLogical(CAR(args));
    if (trim == NA_INTEGER)
	error(_("invalid '%s' argument"), "trim");
    args = CDR(args);

    if (!isNull(CAR(args))) {
	digits = asInteger(CAR(args));
	if (digits == NA_INTEGER || digits < R_MIN_DIGITS_OPT
	    || digits > R_MAX_DIGITS_OPT)
	    error(_("invalid '%s' argument"), "digits");
	R_print.digits = digits;
    }
    args = CDR(args);

    nsmall = asInteger(CAR(args));
    if (nsmall == NA_INTEGER || nsmall < 0 || nsmall > 20)
	error(_("invalid '%s' argument"), "nsmall");
    args = CDR(args);

    if (isNull(swd = CAR(args))) wd = 0; else wd = asInteger(swd);
    if(wd == NA_INTEGER)
	error(_("invalid '%s' argument"), "width");
    args = CDR(args);

    adj = asInteger(CAR(args));
    if(adj == NA_INTEGER || adj < 0 || adj > 3)
	error(_("invalid '%s' argument"), "justify");
    args = CDR(args);

    na = asLogical(CAR(args));
    if(na == NA_LOGICAL)
	error(_("invalid '%s' argument"), "na.encode");
    args = CDR(args);
    if(LENGTH(CAR(args)) != 1)
	error(_("invalid '%s' argument"), "scientific");
    if(isLogical(CAR(args))) {
	int tmp = LOGICAL(CAR(args))[0];
	if(tmp == NA_LOGICAL) sci = NA_INTEGER;
	else sci = tmp > 0 ?-100 : 100;
    } else if (isNumeric(CAR(args))) {
	sci = asInteger(CAR(args));
    } else
	error(_("invalid '%s' argument"), "scientific");
    if(sci != NA_INTEGER) R_print.scipen = sci;

    if ((n = LENGTH(x)) <= 0) {
	PROTECT(y = allocVector(STRSXP, 0));
    } else {
	switch (TYPEOF(x)) {

	case LGLSXP:
	    PROTECT(y = allocVector(STRSXP, n));
	    if (trim) w = 0; else formatLogical(LOGICAL(x), n, &w);
	    w = imax2(w, wd);
	    for (i = 0; i < n; i++) {
		strp = EncodeLogical(LOGICAL(x)[i], w);
		SET_STRING_ELT(y, i, mkChar(strp));
	    }
	    break;

	case INTSXP:
	    PROTECT(y = allocVector(STRSXP, n));
	    if (trim) w = 0;
	    else formatInteger(INTEGER(x), n, &w);
	    w = imax2(w, wd);
	    for (i = 0; i < n; i++) {
		strp = EncodeInteger(INTEGER(x)[i], w);
		SET_STRING_ELT(y, i, mkChar(strp));
	    }
	    break;

	case REALSXP:
	    formatReal(REAL(x), n, &w, &d, &e, nsmall);
	    if (trim) w = 0;
	    w = imax2(w, wd);
	    PROTECT(y = allocVector(STRSXP, n));
	    for (i = 0; i < n; i++) {
		strp = EncodeReal(REAL(x)[i], w, d, e, OutDec);
		SET_STRING_ELT(y, i, mkChar(strp));
	    }
	    break;

	case CPLXSXP:
	    formatComplex(COMPLEX(x), n, &w, &d, &e, &wi, &di, &ei, nsmall);
	    if (trim) wi = w = 0;
	    w = imax2(w, wd); wi = imax2(wi, wd);
	    PROTECT(y = allocVector(STRSXP, n));
	    for (i = 0; i < n; i++) {
		strp = EncodeComplex(COMPLEX(x)[i], w, d, e, wi, di, ei, OutDec);
		SET_STRING_ELT(y, i, mkChar(strp));
	    }
	    break;

	case STRSXP:
	{
	    /* this has to be different from formatString/EncodeString as
	       we don't actually want to encode here */
	    const char *s; 
	    char *q;
	    int b, b0, cnt = 0, j;
	    SEXP s0, xx;

	    /* This is clumsy, but it saves rewriting and re-testing
	       this complex code */
	    PROTECT(xx = duplicate(x));
	    for (i = 0; i < n; i++) {
		SEXP tmp =  STRING_ELT(xx, i);
		if(IS_BYTES(tmp)) {
		    const char *p = CHAR(tmp), *q;
		    char *pp = R_alloc(4*strlen(p)+1, 1), *qq = pp, buf[5];
		    for (q = p; *q; q++) {
			unsigned char k = (unsigned char) *q;
			if (k >= 0x20 && k < 0x80) {
			    *qq++ = *q;
			} else {
			    snprintf(buf, 5, "\\x%02x", k);
			    for(int j = 0; j < 4; j++) *qq++ = buf[j];
			}
		    }
		    *qq = '\0';
		    s = pp;
		} else s = translateChar(tmp);
		if(s != CHAR(tmp)) SET_STRING_ELT(xx, i, mkChar(s));
	    }

	    w = wd;
	    if (adj != Rprt_adj_none) {
		for (i = 0; i < n; i++)
		    if (STRING_ELT(xx, i) != NA_STRING)
			w = imax2(w, Rstrlen(STRING_ELT(xx, i), 0));
		    else if (na) w = imax2(w, R_print.na_width);
	    } else w = 0;
	    /* now calculate the buffer size needed, in bytes */
	    for (i = 0; i < n; i++)
		if (STRING_ELT(xx, i) != NA_STRING) {
		    il = Rstrlen(STRING_ELT(xx, i), 0);
		    cnt = imax2(cnt, LENGTH(STRING_ELT(xx, i)) + imax2(0, w-il));
		} else if (na) cnt  = imax2(cnt, R_print.na_width + imax2(0, w-R_print.na_width));
	    char buff[cnt+1];
	    R_CheckStack();
	    PROTECT(y = allocVector(STRSXP, n));
	    for (i = 0; i < n; i++) {
		if(!na && STRING_ELT(xx, i) == NA_STRING) {
		    SET_STRING_ELT(y, i, NA_STRING);
		} else {
		    q = buff;
		    if(STRING_ELT(xx, i) == NA_STRING) s0 = R_print.na_string;
		    else s0 = STRING_ELT(xx, i) ;
		    s = CHAR(s0);
		    il = Rstrlen(s0, 0);
		    b = w - il;
		    if(b > 0 && adj != Rprt_adj_left) {
			b0 = (adj == Rprt_adj_centre) ? b/2 : b;
			for(j = 0 ; j < b0 ; j++) *q++ = ' ';
			b -= b0;
		    }
		    for(j = 0; j < LENGTH(s0); j++) *q++ = *s++;
		    if(b > 0 && adj != Rprt_adj_right)
			for(j = 0 ; j < b ; j++) *q++ = ' ';
		    *q = '\0';
		    SET_STRING_ELT(y, i, mkChar(buff));
		}
	    }
	}
	UNPROTECT(1);
	break;
	default:
	    error(_("Impossible mode ( x )")); y = R_NilValue;/* -Wall */
	}
    }
    if((l = getAttrib(x, R_DimSymbol)) != R_NilValue) {
	setAttrib(y, R_DimSymbol, l);
	if((l = getAttrib(x, R_DimNamesSymbol)) != R_NilValue)
	    setAttrib(y, R_DimNamesSymbol, l);
    } else if((l = getAttrib(x, R_NamesSymbol)) != R_NilValue)
	setAttrib(y, R_NamesSymbol, l);

    /* In case something else forgets to set PrintDefaults(), PR#14477 */
    R_print.scipen = scikeep;

    UNPROTECT(1);
    return y;
}

/* format.info(obj)  --> 3 integers  (w,d,e) with the formatting information
 *			w = total width (#{chars}) per item
 *			d = #{digits} to RIGHT of "."
 *			e = {0:2}.   0: Fixpoint;
 *				   1,2: exponential with 2/3 digit expon.
 *
 * for complex : 2 x 3 integers for (Re, Im)
 */

SEXP attribute_hidden do_formatinfo(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP x;
    int n, digits, nsmall, no = 1, w, d, e, wi, di, ei;

    checkArity(op, args);
    x = CAR(args);
    n = LENGTH(x);
    PrintDefaults();

    digits = asInteger(CADR(args));
    if (!isNull(CADR(args))) {
	digits = asInteger(CADR(args));
	if (digits == NA_INTEGER || digits < R_MIN_DIGITS_OPT
	    || digits > R_MAX_DIGITS_OPT)
	    error(_("invalid '%s' argument"), "digits");
	R_print.digits = digits;
    }
    nsmall = asInteger(CADDR(args));
    if (nsmall == NA_INTEGER || nsmall < 0 || nsmall > 20)
	error(_("invalid '%s' argument"), "nsmall");

    w = 0;
    d = 0;
    e = 0;
    switch (TYPEOF(x)) {

    case RAWSXP:
	formatRaw(RAW(x), n, &w);
	break;

    case LGLSXP:
	formatLogical(LOGICAL(x), n, &w);
	break;

    case INTSXP:
	formatInteger(INTEGER(x), n, &w);
	break;

    case REALSXP:
	no = 3;
	formatReal(REAL(x), n, &w, &d, &e, nsmall);
	break;

    case CPLXSXP:
	no = 6;
	wi = di = ei = 0;
	formatComplex(COMPLEX(x), n, &w, &d, &e, &wi, &di, &ei, nsmall);
	break;

    case STRSXP:
    {
	int i, il;
	for (i = 0; i < n; i++)
	    if (STRING_ELT(x, i) != NA_STRING) {
		il = Rstrlen(STRING_ELT(x, i), 0);
		if (il > w) w = il;
	    }
    }
	break;

    default:
	error(_("atomic vector arguments only"));
    }
    x = allocVector(INTSXP, no);
    INTEGER(x)[0] = w;
    if(no > 1) {
	INTEGER(x)[1] = d;
	INTEGER(x)[2] = e;
    }
    if(no > 3) {
	INTEGER(x)[3] = wi;
	INTEGER(x)[4] = di;
	INTEGER(x)[5] = ei;
    }
    return x;
}
