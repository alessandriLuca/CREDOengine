/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998--2011 The R Development Core Team
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
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Defn.h>
#include <Rinterface.h>
#include <Fileio.h>
#include <R_ext/Applic.h>		/* machar */
#include <ctype.h>			/* toupper */
#include <time.h>

# include <errno.h>

/* Machine Constants */

static void Init_R_Machine(SEXP rho)
{
    SEXP ans, nms;

    machar(&R_AccuracyInfo.ibeta,
	   &R_AccuracyInfo.it,
	   &R_AccuracyInfo.irnd,
	   &R_AccuracyInfo.ngrd,
	   &R_AccuracyInfo.machep,
	   &R_AccuracyInfo.negep,
	   &R_AccuracyInfo.iexp,
	   &R_AccuracyInfo.minexp,
	   &R_AccuracyInfo.maxexp,
	   &R_AccuracyInfo.eps,
	   &R_AccuracyInfo.epsneg,
	   &R_AccuracyInfo.xmin,
	   &R_AccuracyInfo.xmax);

    R_dec_min_exponent = floor(log10(R_AccuracyInfo.xmin)); /* smallest decimal exponent */
    PROTECT(ans = allocVector(VECSXP, 18));
    PROTECT(nms = allocVector(STRSXP, 18));
    SET_STRING_ELT(nms, 0, mkChar("double.eps"));
    SET_VECTOR_ELT(ans, 0, ScalarReal(R_AccuracyInfo.eps));

    SET_STRING_ELT(nms, 1, mkChar("double.neg.eps"));
    SET_VECTOR_ELT(ans, 1, ScalarReal(R_AccuracyInfo.epsneg));

    SET_STRING_ELT(nms, 2, mkChar("double.xmin"));
    SET_VECTOR_ELT(ans, 2, ScalarReal(R_AccuracyInfo.xmin));

    SET_STRING_ELT(nms, 3, mkChar("double.xmax"));
    SET_VECTOR_ELT(ans, 3, ScalarReal(R_AccuracyInfo.xmax));

    SET_STRING_ELT(nms, 4, mkChar("double.base"));
    SET_VECTOR_ELT(ans, 4, ScalarInteger(R_AccuracyInfo.ibeta));

    SET_STRING_ELT(nms, 5, mkChar("double.digits"));
    SET_VECTOR_ELT(ans, 5, ScalarInteger(R_AccuracyInfo.it));

    SET_STRING_ELT(nms, 6, mkChar("double.rounding"));
    SET_VECTOR_ELT(ans, 6, ScalarInteger(R_AccuracyInfo.irnd));

    SET_STRING_ELT(nms, 7, mkChar("double.guard"));
    SET_VECTOR_ELT(ans, 7, ScalarInteger(R_AccuracyInfo.ngrd));

    SET_STRING_ELT(nms, 8, mkChar("double.ulp.digits"));
    SET_VECTOR_ELT(ans, 8, ScalarInteger(R_AccuracyInfo.machep));

    SET_STRING_ELT(nms, 9, mkChar("double.neg.ulp.digits"));
    SET_VECTOR_ELT(ans, 9, ScalarInteger(R_AccuracyInfo.negep));

    SET_STRING_ELT(nms, 10, mkChar("double.exponent"));
    SET_VECTOR_ELT(ans, 10, ScalarInteger(R_AccuracyInfo.iexp));

    SET_STRING_ELT(nms, 11, mkChar("double.min.exp"));
    SET_VECTOR_ELT(ans, 11, ScalarInteger(R_AccuracyInfo.minexp));

    SET_STRING_ELT(nms, 12, mkChar("double.max.exp"));
    SET_VECTOR_ELT(ans, 12, ScalarInteger(R_AccuracyInfo.maxexp));

    SET_STRING_ELT(nms, 13, mkChar("integer.max"));
    SET_VECTOR_ELT(ans, 13, ScalarInteger(INT_MAX));

    SET_STRING_ELT(nms, 14, mkChar("sizeof.long"));
    SET_VECTOR_ELT(ans, 14, ScalarInteger(SIZEOF_LONG));

    SET_STRING_ELT(nms, 15, mkChar("sizeof.longlong"));
    SET_VECTOR_ELT(ans, 15, ScalarInteger(SIZEOF_LONG_LONG));

    SET_STRING_ELT(nms, 16, mkChar("sizeof.longdouble"));
    SET_VECTOR_ELT(ans, 16, ScalarInteger(SIZEOF_LONG_DOUBLE));

    SET_STRING_ELT(nms, 17, mkChar("sizeof.pointer"));
    SET_VECTOR_ELT(ans, 17, ScalarInteger(sizeof(SEXP)));
    setAttrib(ans, R_NamesSymbol, nms);
    defineVar(install(".Machine"), ans, rho);
    UNPROTECT(2);
}


/*  Platform
 *
 *  Return various platform dependent strings.  This is similar to
 *  "Machine", but for strings rather than numerical values.  These
 *  two functions should probably be amalgamated.
 */
static const char  * const R_OSType = OSTYPE;
static const char  * const R_FileSep = FILESEP;

static void Init_R_Platform(SEXP rho)
{
    SEXP value, names;

    PROTECT(value = allocVector(VECSXP, 8));
    PROTECT(names = allocVector(STRSXP, 8));
    SET_STRING_ELT(names, 0, mkChar("OS.type"));
    SET_STRING_ELT(names, 1, mkChar("file.sep"));
    SET_STRING_ELT(names, 2, mkChar("dynlib.ext"));
    SET_STRING_ELT(names, 3, mkChar("GUI"));
    SET_STRING_ELT(names, 4, mkChar("endian"));
    SET_STRING_ELT(names, 5, mkChar("pkgType"));
    SET_STRING_ELT(names, 6, mkChar("path.sep"));
    SET_STRING_ELT(names, 7, mkChar("r_arch"));
    SET_VECTOR_ELT(value, 0, mkString(R_OSType));
    SET_VECTOR_ELT(value, 1, mkString(R_FileSep));
    SET_VECTOR_ELT(value, 2, mkString(SHLIB_EXT));
    SET_VECTOR_ELT(value, 3, mkString(R_GUIType));
#ifdef WORDS_BIGENDIAN
    SET_VECTOR_ELT(value, 4, mkString("big"));
#else
    SET_VECTOR_ELT(value, 4, mkString("little"));
#endif
/* pkgType should be "mac.binary" for CRAN build *only*, not for all
   AQUA builds. Also we want to be able to use "mac.binary.leopard"
   and similar for special builds. */
#ifdef PLATFORM_PKGTYPE 
    SET_VECTOR_ELT(value, 5, mkString(PLATFORM_PKGTYPE));
#else /* unix default */
    SET_VECTOR_ELT(value, 5, mkString("source"));
#endif
#ifdef Win32
    SET_VECTOR_ELT(value, 6, mkString(";"));
#else /* not Win32 */
    SET_VECTOR_ELT(value, 6, mkString(":"));
#endif
#ifdef R_ARCH
    SET_VECTOR_ELT(value, 7, mkString(R_ARCH));
#else
    SET_VECTOR_ELT(value, 7, mkString(""));
#endif
    setAttrib(value, R_NamesSymbol, names);
    defineVar(install(".Platform"), value, rho);
    UNPROTECT(2);
}

void attribute_hidden Init_R_Variables(SEXP rho)
{
    Init_R_Machine(rho);
    Init_R_Platform(rho);
}

#ifdef HAVE_LANGINFO_CODESET
/* case-insensitive string comparison (needed for locale check) */
int static R_strieql(const char *a, const char *b)
{
    while (*a && *b && toupper(*a) == toupper(*b)) { a++; b++; }
    return (*a == 0 && *b == 0);
}
#endif

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#ifdef HAVE_LANGINFO_CODESET
# include <langinfo.h>
#endif

/* retrieves information about the current locale and
   sets the corresponding variables (known_to_be_utf8,
   known_to_be_latin1, utf8locale, latin1locale and mbcslocale) */
void attribute_hidden R_check_locale(void)
{
    known_to_be_utf8 = utf8locale = FALSE;
    known_to_be_latin1 = latin1locale = FALSE;
    mbcslocale = FALSE;
#ifdef HAVE_LANGINFO_CODESET
    {
	char  *p = nl_langinfo(CODESET);
	/* more relaxed due to Darwin: CODESET is case-insensitive and
	   latin1 is ISO8859-1 */
	if (R_strieql(p, "UTF-8")) known_to_be_utf8 = utf8locale = TRUE;
	if (streql(p, "ISO-8859-1")) known_to_be_latin1 = latin1locale = TRUE;
	if (R_strieql(p, "ISO8859-1")) known_to_be_latin1 = latin1locale = TRUE;
# if __APPLE__
	/* On Darwin 'regular' locales such as 'en_US' are UTF-8 (hence
	   MB_CUR_MAX == 6), but CODESET is "" */
	if (*p == 0 && MB_CUR_MAX == 6)
	    known_to_be_utf8 = utf8locale = TRUE;
# endif
    }
#endif
    mbcslocale = MB_CUR_MAX > 1;
#ifdef Win32
    {
	char *ctype = setlocale(LC_CTYPE, NULL), *p;
	p = strrchr(ctype, '.');
	if (p && isdigit(p[1])) localeCP = atoi(p+1); else localeCP = 0;
	/* Not 100% correct, but CP1252 is a superset */
	known_to_be_latin1 = latin1locale = (localeCP == 1252);
    }
#endif
#if defined(SUPPORT_UTF8_WIN32) /* never at present */
    utf8locale = mbcslocale = TRUE;
#endif
}

/*  date
 *
 *  Return the current date in a standard format.  This uses standard
 *  POSIX calls which should be available on each platform.  We should
 *  perhaps check this in the configure script.
 */
/* BDR 2000/7/20.
 *  time and ctime are in fact ANSI C calls, so we don't check them.
 */
static char *R_Date(void)
{
    time_t t;
    static char s[26];		/* own space */

    time(&t);
    strcpy(s, ctime(&t));
    s[24] = '\0';		/* overwriting the final \n */
    return s;
}

SEXP attribute_hidden do_date(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    checkArity(op, args);
    return mkString(R_Date());
}

/*  file.show
 *
 *  Display file(s) so that a user can view it.  The function calls
 *  "R_ShowFiles" which is a platform-dependent hook that arranges
 *  for the file(s) to be displayed.
 */

SEXP attribute_hidden do_fileshow(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP fn, tl, hd, pg;
    const char **f, **h, *t, *pager = NULL /* -Wall */;
    Rboolean dl;
    int i, n;

    checkArity(op, args);
    fn = CAR(args); args = CDR(args);
    hd = CAR(args); args = CDR(args);
    tl = CAR(args); args = CDR(args);
    dl = (Rboolean) asLogical(CAR(args)); args = CDR(args);
    pg = CAR(args);
    n = 0;			/* -Wall */
    if (!isString(fn) || (n = length(fn)) < 1)
	error(_("invalid filename specification"));
    if (!isString(hd) || length(hd) != n)
	error(_("invalid '%s' argument"), "headers");
    if (!isString(tl))
	error(_("invalid '%s' argument"), "title");
    if (!isString(pg))
	error(_("invalid '%s' argument"), "pager");
    f = (const char**) R_alloc(n, sizeof(char*));
    h = (const char**) R_alloc(n, sizeof(char*));
    for (i = 0; i < n; i++) {
	SEXP el = STRING_ELT(fn, i);
	if (!isNull(el) && el != NA_STRING)
#ifdef Win32
	    f[i] = acopy_string(reEnc(CHAR(el), getCharCE(el), CE_UTF8, 1));
#else
	    f[i] = acopy_string(translateChar(el));
#endif
	else
            error(_("invalid filename specification"));
	if (STRING_ELT(hd, i) != NA_STRING)
	    h[i] = acopy_string(translateChar(STRING_ELT(hd, i)));
	else
            error(_("invalid '%s' argument"), "headers");
    }
    if (isValidStringF(tl))
	t = acopy_string(translateChar(STRING_ELT(tl, 0)));
    else
	t = "";
    if (isValidStringF(pg)) {
	SEXP pg0 = STRING_ELT(pg, 0);
        if (pg0 != NA_STRING)
            pager = acopy_string(CHAR(pg0));
        else
            error(_("invalid '%s' argument"), "pager");
    } else
	pager = "";
    R_ShowFiles(n, f, h, t, dl, pager);
    return R_NilValue;
}

/*  file.edit
 *
 *  Open a file in a text editor. The function calls
 *  "R_EditFiles" which is a platform dependent hook that invokes
 *  the given editor.
 *
 */


SEXP attribute_hidden do_fileedit(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP fn, ti, ed;
    const char **f, **title, *editor;
    int i, n;

    checkArity(op, args);
    fn = CAR(args); args = CDR(args);
    ti = CAR(args); args = CDR(args);
    ed = CAR(args);

    n = length(fn);
    if (!isString(ed))
	error(_("invalid '%s' specification"), "editor");
    if (n > 0) {
	if (!isString(fn))
	    error(_("invalid '%s' specification"), "filename");
	f = (const char**) R_alloc(n, sizeof(char*));
	title = (const char**) R_alloc(n, sizeof(char*));
	/* FIXME convert to UTF-8 on Windows */
	for (i = 0; i < n; i++) {
	    SEXP el = STRING_ELT(fn, 0);
	    if (!isNull(el))
#ifdef Win32
		f[i] = acopy_string(reEnc(CHAR(el), getCharCE(el), CE_UTF8, 1));
#else
		f[i] = acopy_string(translateChar(el));
#endif
	    else
		f[i] = "";
	    if (!isNull(STRING_ELT(ti, i)))
		title[i] = acopy_string(translateChar(STRING_ELT(ti, i)));
	    else
		title[i] = "";
	}
    }
    else {  /* open a new file for editing */
	n = 1;
	f = (const char**) R_alloc(1, sizeof(char*));
	f[0] = "";
	title = (const char**) R_alloc(1, sizeof(char*));
	title[0] = "";
    }
    if (length(ed) >= 1 || !isNull(STRING_ELT(ed, 0))) {
	SEXP ed0 = STRING_ELT(ed, 0);
#ifdef Win32
	editor = acopy_string(reEnc(CHAR(ed0), getCharCE(ed0), CE_UTF8, 1));
#else
	editor = acopy_string(translateChar(ed0));
#endif
    } else
	editor = "";
    R_EditFiles(n, f, title, editor);
    return R_NilValue;
}


/*  append.file
 *
 *  Given two file names as arguments and arranges for
 *  the second file to be appended to the second.
 *  op = 2 is codeFiles.append.
 */

#if defined(BUFSIZ) && (BUFSIZ > 512)
/* OS's buffer size in stdio.h, probably.
   Windows has 512, Solaris 1024, glibc 8192
 */
# define APPENDBUFSIZE BUFSIZ
#else
# define APPENDBUFSIZE 512
#endif

static int R_AppendFile(SEXP file1, SEXP file2)
{
    FILE *fp1, *fp2;
    char buf[APPENDBUFSIZE];
    int nchar, status = 0;
    if ((fp1 = RC_fopen(file1, "ab", TRUE)) == NULL) {
	return 0;
    }
    if ((fp2 = RC_fopen(file2, "rb", TRUE)) == NULL) {
	fclose(fp1);
	return 0;
    }
    while ((nchar = fread(buf, 1, APPENDBUFSIZE, fp2)) == APPENDBUFSIZE)
	if (fwrite(buf, 1, APPENDBUFSIZE, fp1) != APPENDBUFSIZE) {
	    goto append_error;
	}
    if (fwrite(buf, 1, nchar, fp1) != nchar) {
	goto append_error;
    }
    status = 1;
 append_error:
    if (status == 0)
	warning(_("write error during file append"));
    fclose(fp1);
    fclose(fp2);
    return status;
}

SEXP attribute_hidden do_fileappend(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP f1, f2, ans;
    int i, n, n1, n2;
    checkArity(op, args);
    f1 = CAR(args); n1 = length(f1);
    f2 = CADR(args); n2 = length(f2);
    if (!isString(f1))
	error(_("invalid first filename"));
    if (!isString(f2))
	error(_("invalid second filename"));
    if (n1 < 1)
	error(_("nothing to append to"));
    if (PRIMVAL(op) > 0 && n1 > 1)
	error(_("'outFile' must be a single file"));
    if (n2 < 1)
	return allocVector(LGLSXP, 0);
    n = (n1 > n2) ? n1 : n2;
    PROTECT(ans = allocVector(LGLSXP, n)); /* all FALSE */
    if (n1 == 1) { /* common case */
	FILE *fp1, *fp2;
	char buf[APPENDBUFSIZE];
	int nchar, status = 0;
	if (STRING_ELT(f1, 0) == NA_STRING ||
	    !(fp1 = RC_fopen(STRING_ELT(f1, 0), "ab", TRUE)))
	   goto done;
	for (i = 0; i < n; i++) {
	    status = 0;
	    if (STRING_ELT(f2, i) == NA_STRING ||
	       !(fp2 = RC_fopen(STRING_ELT(f2, i), "rb", TRUE))) continue;
	    if (PRIMVAL(op) == 1) { /* codeFiles.append */
	    	snprintf(buf, APPENDBUFSIZE, "#line 1 \"%s\"\n",
			 CHAR(STRING_ELT(f2, i)));
	    	if(fwrite(buf, 1, strlen(buf), fp1) != strlen(buf))
		    goto append_error;
	    }
	    while ((nchar = fread(buf, 1, APPENDBUFSIZE, fp2)) == APPENDBUFSIZE)
		if (fwrite(buf, 1, APPENDBUFSIZE, fp1) != APPENDBUFSIZE)
		    goto append_error;
	    if (fwrite(buf, 1, nchar, fp1) != nchar) goto append_error;
	    if (PRIMVAL(op) == 1 && buf[nchar - 1] != '\n') {
		if (fwrite("\n", 1, 1, fp1) != 1) goto append_error;
	    }

	    status = 1;
	append_error:
	    if (status == 0)
		warning(_("write error during file append"));
	    LOGICAL(ans)[i] = status;
	    fclose(fp2);
	}
	fclose(fp1);
    } else {
	for (i = 0; i < n; i++) {
	    if (STRING_ELT(f1, i%n1) == R_NilValue ||
		STRING_ELT(f2, i%n2) == R_NilValue)
		LOGICAL(ans)[i] = 0;
	    else
		LOGICAL(ans)[i] =
		    R_AppendFile(STRING_ELT(f1, i%n1), STRING_ELT(f2, i%n2));
	}
    }
done:
    UNPROTECT(1);
    return ans;
}

SEXP attribute_hidden do_filecreate(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP fn, ans;
    FILE *fp;
    int i, n, show;

    checkArity(op, args);
    fn = CAR(args);
    if (!isString(fn))
	error(_("invalid filename argument"));
    show = asLogical(CADR(args));
    if (show == NA_LOGICAL) show = 0;
    n = length(fn);
    PROTECT(ans = allocVector(LGLSXP, n));
    for (i = 0; i < n; i++) {
	LOGICAL(ans)[i] = 0;
	if (STRING_ELT(fn, i) == NA_STRING) continue;
	if ((fp = RC_fopen(STRING_ELT(fn, i), "w", TRUE)) != NULL) {
	    LOGICAL(ans)[i] = 1;
	    fclose(fp);
	} else if (show) {
	    warning(_("cannot create file '%s', reason '%s'"),
		    CHAR(STRING_ELT(fn, i)), strerror(errno));
	}
    }
    UNPROTECT(1);
    return ans;
}

SEXP attribute_hidden do_fileremove(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP f, ans;
    int i, n;
    checkArity(op, args);
    f = CAR(args);
    if (!isString(f))
	error(_("invalid first filename"));
    n = length(f);
    PROTECT(ans = allocVector(LGLSXP, n));
    for (i = 0; i < n; i++) {
	if (STRING_ELT(f, i) != NA_STRING) {
	    LOGICAL(ans)[i] =
#ifdef Win32
		(_wremove(filenameToWchar(STRING_ELT(f, i), TRUE)) == 0);
#else
		(remove(R_ExpandFileName(translateChar(STRING_ELT(f, i)))) == 0);
#endif
	    if(!LOGICAL(ans)[i])
		warning(_("cannot remove file '%s', reason '%s'"),
			CHAR(STRING_ELT(f, i)), strerror(errno));
	} else LOGICAL(ans)[i] = FALSE;
    }
    UNPROTECT(1);
    return ans;
}

#ifdef HAVE_UNISTD_H
#include <unistd.h> /* for symlink */
#endif

SEXP attribute_hidden do_filesymlink(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP f1, f2;
    int n, n1, n2;
#ifdef HAVE_SYMLINK
    SEXP ans;
    int i;
    char from[PATH_MAX], to[PATH_MAX];
    const char *p;
#endif
    checkArity(op, args);
    f1 = CAR(args); n1 = length(f1);
    f2 = CADR(args); n2 = length(f2);
    if (!isString(f1))
	error(_("invalid first filename"));
    if (!isString(f2))
	error(_("invalid second filename"));
    if (n1 < 1)
	error(_("nothing to link"));
    if (n2 < 1)
	return allocVector(LGLSXP, 0);
    n = (n1 > n2) ? n1 : n2;
#ifdef HAVE_SYMLINK  /* Not (yet) Windows */
    PROTECT(ans = allocVector(LGLSXP, n));
    for (i = 0; i < n; i++) {
	if (STRING_ELT(f1, i%n1) == NA_STRING ||
	    STRING_ELT(f2, i%n2) == NA_STRING)
	    LOGICAL(ans)[i] = 0;
	else {
	    p = R_ExpandFileName(translateChar(STRING_ELT(f1, i%n1)));
	    if (strlen(p) >= PATH_MAX - 1) {
		LOGICAL(ans)[i] = 0;
		continue;
	    }
	    strcpy(from, p);
	    p = R_ExpandFileName(translateChar(STRING_ELT(f2, i%n2)));
	    if (strlen(p) >= PATH_MAX - 1) {
		LOGICAL(ans)[i] = 0;
		continue;
	    }
	    strcpy(to, p);
	    /* Rprintf("linking %s to %s\n", from, to); */
	    LOGICAL(ans)[i] = symlink(from, to) == 0;
	    if(!LOGICAL(ans)[i]) {
		warning(_("cannot symlink '%s' to '%s', reason '%s'"),
			from, to, strerror(errno));
	    }
	}
    }
    UNPROTECT(1);
    return ans;
#else
    warning(_("symlinks are not supported on this platform"));
    return allocVector(LGLSXP, n);
#endif
}

#ifdef Win32
int Rwin_rename(char *from, char *to);  /* in src/gnuwin32/extra.c */
int Rwin_wrename(const wchar_t *from, const wchar_t *to);
#endif

SEXP attribute_hidden do_filerename(SEXP call, SEXP op, SEXP args, SEXP rho)
{
#ifdef Win32
    wchar_t from[PATH_MAX], to[PATH_MAX];
    const wchar_t *w;
#else
    char from[PATH_MAX], to[PATH_MAX];
    const char *p;
    int res;
#endif

    checkArity(op, args);
    if (TYPEOF(CAR(args)) != STRSXP || LENGTH(CAR(args)) != 1)
	error(_("'source' must be a single string"));
    if (TYPEOF(CADR(args)) != STRSXP || LENGTH(CADR(args)) != 1)
	error(_("'destination' must be a single string"));

    if (STRING_ELT(CAR(args), 0) == NA_STRING ||
	STRING_ELT(CADR(args), 0) == NA_STRING)
	error(_("missing values are not allowed"));
#ifdef Win32
    w = filenameToWchar(STRING_ELT(CAR(args), 0), TRUE);
    if (wcslen(w) >= PATH_MAX - 1)
	error(_("expanded source name too long"));
    wcsncpy(from, w, PATH_MAX - 1);
    w = filenameToWchar(STRING_ELT(CADR(args), 0), TRUE);
    if (wcslen(w) >= PATH_MAX - 1)
	error(_("expanded destination name too long"));
    wcsncpy(to, w, PATH_MAX - 1);
    return Rwin_wrename(from, to) == 0 ? mkTrue() : mkFalse();
#else
    p = R_ExpandFileName(translateChar(STRING_ELT(CAR(args), 0)));
    if (strlen(p) >= PATH_MAX - 1)
	error(_("expanded source name too long"));
    strncpy(from, p, PATH_MAX - 1);
    p = R_ExpandFileName(translateChar(STRING_ELT(CADR(args), 0)));
    if (strlen(p) >= PATH_MAX - 1)
	error(_("expanded destination name too long"));
    strncpy(to, p, PATH_MAX - 1);
    res = rename(from, to);
    if(res) {
	warning(_("cannot rename file '%s' to '%s', reason '%s'"),
		from, to, strerror(errno));
    }
    return res == 0 ? mkTrue() : mkFalse();
#endif
}

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

# if defined(Unix) && defined(HAVE_PWD_H) && defined(HAVE_GRP_H) \
  && defined(HAVE_GETPWUID) && defined(HAVE_GETGRGID)
#  include <pwd.h>
#  include <grp.h>
#  define UNIX_EXTRAS 1
# endif

#ifdef Win32
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h>
# ifndef SCS_64BIT_BINARY
#  define SCS_64BIT_BINARY 6
# endif
#endif

SEXP attribute_hidden do_fileinfo(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP fn, ans, ansnames, fsize, mtime, ctime, atime, isdir,
	mode, xxclass;
#ifdef UNIX_EXTRAS
    SEXP uid, gid, uname, grname;
    struct passwd *stpwd;
    struct group *stgrp;
#endif
    int i, n;
#ifdef Win32
    SEXP exe;
    struct _stati64 sb;
#else
    struct stat sb;
#endif

    checkArity(op, args);
    fn = CAR(args);
    if (!isString(fn))
	error(_("invalid filename argument"));
    n = length(fn);
#ifdef UNIX_EXTRAS
    PROTECT(ans = allocVector(VECSXP, 10));
    PROTECT(ansnames = allocVector(STRSXP, 10));
#elif defined(Win32)
    PROTECT(ans = allocVector(VECSXP, 7));
    PROTECT(ansnames = allocVector(STRSXP, 7));
#else
    PROTECT(ans = allocVector(VECSXP, 6));
    PROTECT(ansnames = allocVector(STRSXP, 6));
#endif
    fsize = SET_VECTOR_ELT(ans, 0, allocVector(REALSXP, n));
    SET_STRING_ELT(ansnames, 0, mkChar("size"));
    isdir = SET_VECTOR_ELT(ans, 1, allocVector(LGLSXP, n));
    SET_STRING_ELT(ansnames, 1, mkChar("isdir"));
    mode  = SET_VECTOR_ELT(ans, 2, allocVector(INTSXP, n));
    SET_STRING_ELT(ansnames, 2, mkChar("mode"));
    mtime = SET_VECTOR_ELT(ans, 3, allocVector(REALSXP, n));
    SET_STRING_ELT(ansnames, 3, mkChar("mtime"));
    ctime = SET_VECTOR_ELT(ans, 4, allocVector(REALSXP, n));
    SET_STRING_ELT(ansnames, 4, mkChar("ctime"));
    atime = SET_VECTOR_ELT(ans, 5, allocVector(REALSXP, n));
    SET_STRING_ELT(ansnames, 5, mkChar("atime"));
#ifdef UNIX_EXTRAS
    uid = SET_VECTOR_ELT(ans, 6, allocVector(INTSXP, n));
    SET_STRING_ELT(ansnames, 6, mkChar("uid"));
    gid = SET_VECTOR_ELT(ans, 7, allocVector(INTSXP, n));
    SET_STRING_ELT(ansnames, 7, mkChar("gid"));
    uname = SET_VECTOR_ELT(ans, 8, allocVector(STRSXP, n));
    SET_STRING_ELT(ansnames, 8, mkChar("uname"));
    grname = SET_VECTOR_ELT(ans, 9, allocVector(STRSXP, n));
    SET_STRING_ELT(ansnames, 9, mkChar("grname"));
#endif
#ifdef Win32
    exe = SET_VECTOR_ELT(ans, 6, allocVector(STRSXP, n));
    SET_STRING_ELT(ansnames, 6, mkChar("exe"));
#endif
    for (i = 0; i < n; i++) {
#ifdef Win32
	const wchar_t *wfn = filenameToWchar(STRING_ELT(fn, i), TRUE);
#else
	const char *efn = R_ExpandFileName(translateChar(STRING_ELT(fn, i)));
#endif
	if (STRING_ELT(fn, i) != NA_STRING &&
#ifdef Win32
	    _wstati64(wfn, &sb)
#else
	    stat(efn, &sb)
#endif
	    == 0) {
	    REAL(fsize)[i] = (double) sb.st_size;
	    LOGICAL(isdir)[i] = (sb.st_mode & S_IFDIR) > 0;
	    INTEGER(mode)[i]  = (int) sb.st_mode & 0007777;
	    REAL(mtime)[i] = (double) sb.st_mtime;
	    REAL(ctime)[i] = (double) sb.st_ctime;
	    REAL(atime)[i] = (double) sb.st_atime;
#ifdef UNIX_EXTRAS
	    INTEGER(uid)[i] = (int) sb.st_uid;
	    INTEGER(gid)[i] = (int) sb.st_gid;
	    stpwd = getpwuid(sb.st_uid);
	    if (stpwd) SET_STRING_ELT(uname, i, mkChar(stpwd->pw_name));
	    else SET_STRING_ELT(uname, i, NA_STRING);
	    stgrp = getgrgid(sb.st_gid);
	    if (stgrp) SET_STRING_ELT(grname, i, mkChar(stgrp->gr_name));
	    else SET_STRING_ELT(grname, i, NA_STRING);
#endif
#ifdef Win32
	    {
		char *s="no";
		DWORD type;
		if (GetBinaryTypeW(wfn, &type))
		    switch(type) {
		    case SCS_64BIT_BINARY:
			s = "win64";
			break;
		    case SCS_32BIT_BINARY:
			s = "win32";
			break;
		    case SCS_DOS_BINARY:
		    case SCS_PIF_BINARY:
			s = "msdos";
			break;
		    case SCS_WOW_BINARY:
			s = "win16";
			break;
		    default:
			s = "unknown";
		    }
		SET_STRING_ELT(exe, i, mkChar(s));
	    }
#endif
	} else {
	    REAL(fsize)[i] = NA_REAL;
	    LOGICAL(isdir)[i] = NA_INTEGER;
	    INTEGER(mode)[i]  = NA_INTEGER;
	    REAL(mtime)[i] = NA_REAL;
	    REAL(ctime)[i] = NA_REAL;
	    REAL(atime)[i] = NA_REAL;
#ifdef UNIX_EXTRAS
	    INTEGER(uid)[i] = NA_INTEGER;
	    INTEGER(gid)[i] = NA_INTEGER;
	    SET_STRING_ELT(uname, i, NA_STRING);
	    SET_STRING_ELT(grname, i, NA_STRING);
#endif
#ifdef Win32
	    SET_STRING_ELT(exe, i, NA_STRING);
#endif
	}
    }
    setAttrib(ans, R_NamesSymbol, ansnames);
    PROTECT(xxclass = mkString("octmode"));
    classgets(mode, xxclass);
    UNPROTECT(3);
    return ans;
}

/* No longer required by POSIX, but maybe on earlier OSes */
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
#elif HAVE_SYS_NDIR_H
# include <sys/ndir.h>
#elif HAVE_SYS_DIR_H
# include <sys/dir.h>
#elif HAVE_NDIR_H
# include <ndir.h>
#endif

#define CBUFSIZE 2*PATH_MAX+1
static SEXP filename(const char *dir, const char *file)
{
    SEXP ans;
    char cbuf[CBUFSIZE];
    if (dir) {
#ifdef Win32
	if ((strlen(dir) == 2 && dir[1] == ':') ||
	    dir[strlen(dir) - 1] == '/' ||  dir[strlen(dir) - 1] == '\\')
	    snprintf(cbuf, CBUFSIZE, "%s%s", dir, file);
	else
	    snprintf(cbuf, CBUFSIZE, "%s%s%s", dir, R_FileSep, file);
#else
	snprintf(cbuf, CBUFSIZE, "%s%s%s", dir, R_FileSep, file);
#endif
	ans = mkChar(cbuf);
    } else {
	snprintf(cbuf, CBUFSIZE, "%s", file);
	ans = mkChar(cbuf);
    }
    return ans;
}

#include <tre/tre.h>

static void list_files(const char *dnp, const char *stem, int *count, SEXP *pans,
		       Rboolean allfiles, Rboolean recursive,
                       const regex_t *reg, int *countmax, PROTECT_INDEX idx)
{
    DIR *dir;
    struct dirent *de;
    char p[PATH_MAX], stem2[PATH_MAX];
#ifdef Windows
    /* > 2GB files might be skipped otherwise */
    struct _stati64 sb;
#else
    struct stat sb;
#endif
    R_CheckUserInterrupt();
    if ((dir = opendir(dnp)) != NULL) {
	while ((de = readdir(dir))) {
	    if (allfiles || !R_HiddenFile(de->d_name)) {
		if (recursive) {
#ifdef Win32
		    if (strlen(dnp) == 2 && dnp[1] == ':')
			snprintf(p, PATH_MAX, "%s%s", dnp, de->d_name);
		    else
			snprintf(p, PATH_MAX, "%s%s%s", dnp, R_FileSep, de->d_name);
#else
		    snprintf(p, PATH_MAX, "%s%s%s", dnp, R_FileSep, de->d_name);
#endif
#ifdef Windows
		    _stati64(p, &sb);
#else
		    stat(p, &sb);
#endif
		    if ((sb.st_mode & S_IFDIR) > 0) {
			if (strcmp(de->d_name, ".") &&
			    strcmp(de->d_name, "..")) {
			    if (stem) {
#ifdef Win32
				if(strlen(stem) == 2 && stem[1] == ':')
				    snprintf(stem2, PATH_MAX, "%s%s", stem,
					     de->d_name);
				else
				    snprintf(stem2, PATH_MAX, "%s%s%s", stem,
					     R_FileSep, de->d_name);
#else
				snprintf(stem2, PATH_MAX, "%s%s%s", stem,
					 R_FileSep, de->d_name);
#endif
			    } else
				strcpy(stem2, de->d_name);
			    list_files(p, stem2, count, pans, allfiles,
				       recursive, reg, countmax, idx);
			}
			continue;
		    }
		}
		if (!reg || tre_regexec(reg, de->d_name, 0, NULL, 0) == 0) {
                    if (*count == *countmax - 1) {
                        *countmax *= 2;
                        REPROTECT(*pans = lengthgets(*pans, *countmax), idx);
                    }
                    SET_STRING_ELT(*pans, (*count)++,
                                   filename(stem, de->d_name));
                }
	    }
	}
	closedir(dir);
    }
}

SEXP attribute_hidden do_listfiles(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    PROTECT_INDEX idx;
    SEXP d, p, ans;
    int allfiles, fullnames, count, pattern, recursive, igcase, flags;
    int i, ndir;
    const char *dnp;
    regex_t reg;
    int countmax = 128;

    checkArity(op, args);
    d = CAR(args);  args = CDR(args);
    if (!isString(d))
	error(_("invalid '%s' argument"), "directory");
    p = CAR(args);  args = CDR(args);
    pattern = 0;
    if (isString(p) && length(p) >= 1 && STRING_ELT(p, 0) != NA_STRING)
	pattern = 1;
    else if (!isNull(p) && !(isString(p) && length(p) < 1))
	error(_("invalid '%s' argument"), "pattern");
    allfiles = asLogical(CAR(args)); args = CDR(args);
    if (allfiles == NA_LOGICAL) 
	error(_("invalid '%s' argument"), "all.files");
    fullnames = asLogical(CAR(args)); args = CDR(args);
    if (fullnames == NA_LOGICAL)
	error(_("invalid '%s' argument"), "full.names");
    recursive = asLogical(CAR(args)); args = CDR(args);
    igcase = asLogical(CAR(args));
    if (igcase == NA_LOGICAL)
	error(_("invalid '%s' argument"), "ignore.case");
    ndir = length(d);
    flags = REG_EXTENDED;
    if (igcase) flags |= REG_ICASE;

    if (pattern && tre_regcomp(&reg, translateChar(STRING_ELT(p, 0)), flags))
	error(_("invalid 'pattern' regular expression"));
    PROTECT_WITH_INDEX(ans = allocVector(STRSXP, countmax), &idx);
    count = 0;
    for (i = 0; i < ndir ; i++) {
	if (STRING_ELT(d, i) == NA_STRING) continue;
	dnp = R_ExpandFileName(translateChar(STRING_ELT(d, i)));
	list_files(dnp, fullnames ? dnp : NULL, &count, &ans, allfiles,
		   recursive, pattern ? &reg : NULL, &countmax, idx);
    }
    REPROTECT(ans = lengthgets(ans, count), idx);
    if (pattern) tre_regfree(&reg);
    ssort(STRING_PTR(ans), count);
    UNPROTECT(1);
    return ans;
}

SEXP attribute_hidden do_Rhome(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    char *path;
    checkArity(op, args);
    if (!(path = R_HomeDir()))
	error(_("unable to determine R home location"));
    return mkString(path);
}

#ifdef Win32
static Rboolean attribute_hidden R_WFileExists(const wchar_t *path)
{
    struct _stat sb;
    return _wstat(path, &sb) == 0;
}
#endif

SEXP attribute_hidden do_fileexists(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP file, ans;
    int i, nfile;
    checkArity(op, args);
    if (!isString(file = CAR(args)))
	error(_("invalid '%s' argument"), "file");
    nfile = length(file);
    ans = allocVector(LGLSXP, nfile);
    for (i = 0; i < nfile; i++) {
	LOGICAL(ans)[i] = 0;
	if (STRING_ELT(file, i) != NA_STRING) {
#ifdef Win32
	    /* Package XML sends arbitrarily long strings to file.exists! */
	    int len = strlen(CHAR(STRING_ELT(file, i)));
	    if (len > MAX_PATH)
		LOGICAL(ans)[i] = FALSE;
	    else
		LOGICAL(ans)[i] =
		    R_WFileExists(filenameToWchar(STRING_ELT(file, i), TRUE));
#else
	    LOGICAL(ans)[i] = R_FileExists(translateChar(STRING_ELT(file, i)));
#endif
	} else LOGICAL(ans)[i] = FALSE;
    }
    return ans;
}

#define CHOOSEBUFSIZE 1024

#ifndef Win32
SEXP attribute_hidden do_filechoose(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    int _new, len;
    char buf[CHOOSEBUFSIZE];
    checkArity(op, args);
    _new = asLogical(CAR(args));
    if ((len = R_ChooseFile(_new, buf, CHOOSEBUFSIZE)) == 0)
	error(_("file choice cancelled"));
    if (len >= CHOOSEBUFSIZE - 1)
	error(_("file name too long"));
    return mkString(R_ExpandFileName(buf));
}
#endif

/* needed for access, and perhaps for realpath */
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef Win32
extern int winAccessW(const wchar_t *path, int mode);
#endif

/* require 'access' as from 2.12.0 */
SEXP attribute_hidden do_fileaccess(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP fn, ans;
    int i, n, mode, modemask;

    checkArity(op, args);
    fn = CAR(args);
    if (!isString(fn))
	error(_("invalid '%s' argument"), "names");
    n = length(fn);
    mode = asInteger(CADR(args));
    if (mode < 0 || mode > 7) error(_("invalid '%s' argument"), "mode");
    modemask = 0;
    if (mode & 1) modemask |= X_OK;
    if (mode & 2) modemask |= W_OK;
    if (mode & 4) modemask |= R_OK;
    PROTECT(ans = allocVector(INTSXP, n));
    for (i = 0; i < n; i++)
	if (STRING_ELT(fn, i) != NA_STRING) {
	    INTEGER(ans)[i] =
#ifdef Win32
		winAccessW(filenameToWchar(STRING_ELT(fn, i), TRUE), modemask);
#else
		access(R_ExpandFileName(translateChar(STRING_ELT(fn, i))),
		       modemask);
#endif
	} else INTEGER(ans)[i] = FALSE;
    UNPROTECT(1);
    return ans;
}

#ifdef Win32
#include <windows.h>

static int R_rmdir(const wchar_t *dir)
{
    wchar_t tmp[MAX_PATH];
    GetShortPathNameW(dir, tmp, MAX_PATH);
    //printf("removing directory %ls\n", tmp);
    return _wrmdir(tmp);
}

static int R_unlink(wchar_t *name, int recursive)
{
    if (wcscmp(name, L".") == 0 || wcscmp(name, L"..") == 0) return 0;
    //printf("R_unlink(%ls)\n", name);
    if (!R_WFileExists(name)) return 0;
    if (recursive) {
	_WDIR *dir;
	struct _wdirent *de;
	wchar_t p[PATH_MAX];
	struct _stat sb;
	int n, ans = 0;

	_wstat(name, &sb);
	if ((sb.st_mode & S_IFDIR) > 0) { /* a directory */
	    if ((dir = _wopendir(name)) != NULL) {
		while ((de = _wreaddir(dir))) {
		    if (!wcscmp(de->d_name, L".") || !wcscmp(de->d_name, L".."))
			continue;
		    /* On Windows we need to worry about trailing seps */
		    n = wcslen(name);
		    if (name[n] == L'/' || name[n] == L'\\') {
			wcscpy(p, name); wcscat(p, de->d_name);
		    } else {
			wcscpy(p, name); wcscat(p, L"/"); wcscat(p, de->d_name);
		    }
		    /* printf("stat-ing %ls\n", p); */
		    _wstat(p, &sb);
		    if ((sb.st_mode & S_IFDIR) > 0) { /* a directory */
			/* printf("is a directory\n"); */
			ans += R_unlink(p, recursive);
		    } else
			ans += (_wunlink(p) == 0) ? 0 : 1;
		}
		_wclosedir(dir);
	    } else { /* we were unable to read a dir */
		ans++;
	    }
	    ans += (R_rmdir(name) == 0) ? 0 : 1;
	    return ans;
	}
	/* drop through */
    }
    return _wunlink(name) == 0 ? 0 : 1;
}

void R_CleanTempDir(void)
{
    if (Sys_TempDir) {
	int n = strlen(Sys_TempDir);
	/* Windows cannot delete the current working directory */
	SetCurrentDirectory(R_HomeDir());
	wchar_t w[2*(n+1)];
	mbstowcs(w, Sys_TempDir, n+1);
	R_unlink(w, 1);
    }
}
#else
static int R_unlink(const char *name, int recursive)
{
    struct stat sb;
    int res, res2;

    if (streql(name, ".") || streql(name, "..")) return 0;
    /* We cannot use R_FileExists here since it is false for broken
       symbolic links 
       if (!R_FileExists(name)) return 0; */
    res  = stat(name, &sb);

    if (!res && recursive) {
	DIR *dir;
	struct dirent *de;
	char p[PATH_MAX];
	int n, ans = 0;

	if ((sb.st_mode & S_IFDIR) > 0) { /* a directory */
	    if ((dir = opendir(name)) != NULL) {
		while ((de = readdir(dir))) {
		    if (streql(de->d_name, ".") || streql(de->d_name, ".."))
			continue;
		    n = strlen(name);
		    if (name[n] == R_FileSep[0])
			snprintf(p, PATH_MAX, "%s%s", name, de->d_name);
		    else
			snprintf(p, PATH_MAX, "%s%s%s", name, R_FileSep,
				 de->d_name);
		    stat(p, &sb);
		    if ((sb.st_mode & S_IFDIR) > 0) { /* a directory */
			ans += R_unlink(p, recursive);
		    } else
			ans += (unlink(p) == 0) ? 0 : 1;
		}
		closedir(dir);
	    } else { /* we were unable to read a dir */
		ans++;
	    }
	    ans += (rmdir(name) == 0) ? 0 : 1;
	    return ans;
	}
	/* drop through */
    }
    res2 = unlink(name);
    /* We want to return 0 if either unlink succeeded or 'name' did not exist */
    return (res2 == 0 || res != 0) ? 0 : 1;
}
#endif


/* Note that wildcards are allowed in 'names' */
#ifdef Win32
# include <dos_wglob.h>
SEXP attribute_hidden do_unlink(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP  fn;
    int i, j, nfiles, res, failures = 0, recursive;
    const wchar_t *names;
    wglob_t globbuf;

    checkArity(op, args);
    fn = CAR(args);
    nfiles = length(fn);
    if (nfiles > 0) {
	if (!isString(fn))
	    error(_("invalid '%s' argument"), "x");
	recursive = asLogical(CADR(args));
	if (recursive == NA_LOGICAL)
	    error(_("invalid '%s' argument"), "recursive");
	for (i = 0; i < nfiles; i++) {
	    if (STRING_ELT(fn, i) != NA_STRING) {
		names = filenameToWchar(STRING_ELT(fn, i), FALSE);
		//Rprintf("do_unlink(%ls)\n", names);
		res = dos_wglob(names, GLOB_NOCHECK, NULL, &globbuf);
		if (res == GLOB_NOSPACE)
		    error(_("internal out-of-memory condition"));
		for (j = 0; j < globbuf.gl_pathc; j++)
		    failures += R_unlink(globbuf.gl_pathv[j], recursive);
		dos_wglobfree(&globbuf);
	    } else failures++;
	}
    }
    return ScalarInteger(failures ? 1 : 0);
}
#else
# if defined(HAVE_GLOB) && defined(HAVE_GLOB_H)
#  include <glob.h>
# endif

SEXP attribute_hidden do_unlink(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP  fn;
    int i, nfiles, failures = 0, recursive;
    const char *names;
#if defined(HAVE_GLOB)
    int j, res;
    glob_t globbuf;
#endif

    checkArity(op, args);
    fn = CAR(args);
    nfiles = length(fn);
    if (nfiles > 0) {
	if (!isString(fn))
	    error(_("invalid '%s' argument"), "x");
	recursive = asLogical(CADR(args));
	if (recursive == NA_LOGICAL)
	    error(_("invalid '%s' argument"), "recursive");
	for (i = 0; i < nfiles; i++) {
	    if (STRING_ELT(fn, i) != NA_STRING) {
		names = translateChar(STRING_ELT(fn, i));
#if defined(HAVE_GLOB)
		res = glob(names, GLOB_NOCHECK, NULL, &globbuf);
# ifdef GLOB_ABORTED
		if (res == GLOB_ABORTED)
		    warning(_("read error on '%s'"), names);
# endif
# ifdef GLOB_NOSPACE
		if (res == GLOB_NOSPACE)
		    error(_("internal out-of-memory condition"));
# endif
		for (j = 0; j < globbuf.gl_pathc; j++)
		    failures += R_unlink(globbuf.gl_pathv[j], recursive);
		globfree(&globbuf);
	    } else failures++;
#else /* HAVE_GLOB */
	        failures += R_unlink(names, recursive);
	    } else failures++;
#endif
	}
    }
    return ScalarInteger(failures ? 1 : 0);
}
#endif

static void chmod_one(const char *name)
{
    DIR *dir;
    struct dirent *de;
    char p[PATH_MAX];
    struct stat sb;
    int n;
#ifndef Win32
    mode_t mask = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR,
	dirmask = S_IXUSR | S_IXGRP | S_IXOTH;
#endif

    if (streql(name, ".") || streql(name, "..")) return;
    if (!R_FileExists(name)) return;
    stat(name, &sb);
#ifdef Win32
    chmod(name, _S_IWRITE);
#else
    chmod(name, sb.st_mode | mask);
#endif
    if ((sb.st_mode & S_IFDIR) > 0) { /* a directory */
#ifndef Win32
	chmod(name, sb.st_mode | mask | dirmask);
#endif
	if ((dir = opendir(name)) != NULL) {
	    while ((de = readdir(dir))) {
		if (streql(de->d_name, ".") || streql(de->d_name, ".."))
		    continue;
		n = strlen(name);
		if (name[n-1] == R_FileSep[0])
		    snprintf(p, PATH_MAX, "%s%s", name, de->d_name);
		else
		    snprintf(p, PATH_MAX, "%s%s%s", name, R_FileSep, de->d_name);
		chmod_one(p);
	    }
	    closedir(dir);
	} else { 
	    /* we were unable to read a dir */
	}
    }
}


/* recursively fix up permissions: used for R CMD INSTALL */
SEXP attribute_hidden do_dirchmod(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP dr;
    checkArity(op, args);
    dr = CAR(args);
    if(!isString(dr) || length(dr) != 1)
	error(_("invalid '%s' argument"), "dir");
    chmod_one(translateChar(STRING_ELT(dr, 0)));

    return R_NilValue;
}




SEXP attribute_hidden do_getlocale(SEXP call, SEXP op, SEXP args, SEXP rho)
{
#ifdef HAVE_LOCALE_H
    int cat;
    char *p = NULL;

    checkArity(op, args);
    cat = asInteger(CAR(args));
    if (cat == NA_INTEGER || cat < 0)
	error(_("invalid '%s' argument"), "category");
    switch(cat) {
    case 1: cat = LC_ALL; break;
    case 2: cat = LC_COLLATE; break;
    case 3: cat = LC_CTYPE; break;
    case 4: cat = LC_MONETARY; break;
    case 5: cat = LC_NUMERIC; break;
    case 6: cat = LC_TIME; break;
#ifdef LC_MESSAGES
    case 7: cat = LC_MESSAGES; break;
#endif
#ifdef LC_PAPER
    case 8: cat = LC_PAPER; break;
#endif
#ifdef LC_MEASUREMENT
    case 9: cat = LC_MEASUREMENT; break;
#endif
    default: cat = NA_INTEGER;
    }
    if (cat != NA_INTEGER) p = setlocale(cat, NULL);
    return mkString(p ? p : "");
#else
    return R_NilValue;
#endif
}

extern void invalidate_cached_recodings(void);  /* from sysutils.c */

extern void resetICUcollator(void); /* from util.c */

/* Locale specs are always ASCII */
SEXP attribute_hidden do_setlocale(SEXP call, SEXP op, SEXP args, SEXP rho)
{
#ifdef HAVE_LOCALE_H
    SEXP locale = CADR(args), ans;
    int cat;
    const char *p;

    checkArity(op, args);
    cat = asInteger(CAR(args));
    if (cat == NA_INTEGER || cat < 0)
	error(_("invalid '%s' argument"), "category");
    if (!isString(locale) || LENGTH(locale) != 1)
	error(_("invalid '%s' argument"), "locale");
    switch(cat) {
    case 1:
    {
	const char *l = CHAR(STRING_ELT(locale, 0));
	cat = LC_ALL;
	/* assume we can set LC_CTYPE iff we can set the rest */
	if ((p = setlocale(LC_CTYPE, l))) {
	    setlocale(LC_COLLATE, l);
	    resetICUcollator();
	    setlocale(LC_MONETARY, l);
	    setlocale(LC_TIME, l);
	    /* Need to return value of LC_ALL */
	    p = setlocale(cat, NULL);
	}
	break;
    }
    case 2:
	cat = LC_COLLATE;
	p = setlocale(cat, CHAR(STRING_ELT(locale, 0)));
	resetICUcollator();
	break;
    case 3:
	cat = LC_CTYPE;
	p = setlocale(cat, CHAR(STRING_ELT(locale, 0)));
	break;
    case 4:
	cat = LC_MONETARY;
	p = setlocale(cat, CHAR(STRING_ELT(locale, 0)));
	break;
    case 5:
	cat = LC_NUMERIC;
	warning(_("setting 'LC_NUMERIC' may cause R to function strangely"));
	p = setlocale(cat, CHAR(STRING_ELT(locale, 0)));
	break;
    case 6:
	cat = LC_TIME;
	p = setlocale(cat, CHAR(STRING_ELT(locale, 0)));
	break;
#if defined LC_MESSAGES && !defined Win32
/* this seems to exist in MinGW, but it does not work in Windows */
    case 7:
	cat = LC_MESSAGES;
	p = setlocale(cat, CHAR(STRING_ELT(locale, 0)));
	break;
#endif
#ifdef LC_PAPER
    case 8:
	cat = LC_PAPER;
	p = setlocale(cat, CHAR(STRING_ELT(locale, 0)));
	break;
#endif
#ifdef LC_MEASUREMENT
    case 9:
	cat = LC_MEASUREMENT;
	p = setlocale(cat, CHAR(STRING_ELT(locale, 0)));
	break;
#endif
    default:
	p = NULL; /* -Wall */
	error(_("invalid '%s' argument"), "category");
    }
    PROTECT(ans = allocVector(STRSXP, 1));
    if (p) SET_STRING_ELT(ans, 0, mkChar(p));
    else  {
	SET_STRING_ELT(ans, 0, mkChar(""));
	warning(_("OS reports request to set locale to \"%s\" cannot be honored"),
		CHAR(STRING_ELT(locale, 0)));
    }
    UNPROTECT(1);
    R_check_locale();
    invalidate_cached_recodings();
    return ans;
#else
    return R_NilValue;
#endif
}



SEXP attribute_hidden do_localeconv(SEXP call, SEXP op, SEXP args, SEXP rho)
{
#ifdef HAVE_LOCALE_H
    SEXP ans, ansnames;
    struct lconv *lc = localeconv();
    int i = 0;
    char buff[20];

    PROTECT(ans = allocVector(STRSXP, 18));
    PROTECT(ansnames = allocVector(STRSXP, 18));
    SET_STRING_ELT(ans, i, mkChar(lc->decimal_point));
    SET_STRING_ELT(ansnames, i++, mkChar("decimal_point"));
    SET_STRING_ELT(ans, i, mkChar(lc->thousands_sep));
    SET_STRING_ELT(ansnames, i++, mkChar("thousands_sep"));
    SET_STRING_ELT(ans, i, mkChar(lc->grouping));
    SET_STRING_ELT(ansnames, i++, mkChar("grouping"));
    SET_STRING_ELT(ans, i, mkChar(lc->int_curr_symbol));
    SET_STRING_ELT(ansnames, i++, mkChar("int_curr_symbol"));
    SET_STRING_ELT(ans, i, mkChar(lc->currency_symbol));
    SET_STRING_ELT(ansnames, i++, mkChar("currency_symbol"));
    SET_STRING_ELT(ans, i, mkChar(lc->mon_decimal_point));
    SET_STRING_ELT(ansnames, i++, mkChar("mon_decimal_point"));
    SET_STRING_ELT(ans, i, mkChar(lc->mon_thousands_sep));
    SET_STRING_ELT(ansnames, i++, mkChar("mon_thousands_sep"));
    SET_STRING_ELT(ans, i, mkChar(lc->mon_grouping));
    SET_STRING_ELT(ansnames, i++, mkChar("mon_grouping"));
    SET_STRING_ELT(ans, i, mkChar(lc->positive_sign));
    SET_STRING_ELT(ansnames, i++, mkChar("positive_sign"));
    SET_STRING_ELT(ans, i, mkChar(lc->negative_sign));
    SET_STRING_ELT(ansnames, i++, mkChar("negative_sign"));
    sprintf(buff, "%d", (int)lc->int_frac_digits);
    SET_STRING_ELT(ans, i, mkChar(buff));
    SET_STRING_ELT(ansnames, i++, mkChar("int_frac_digits"));
    sprintf(buff, "%d", (int)lc->frac_digits);
    SET_STRING_ELT(ans, i, mkChar(buff));
    SET_STRING_ELT(ansnames, i++, mkChar("frac_digits"));
    sprintf(buff, "%d", (int)lc->p_cs_precedes);
    SET_STRING_ELT(ans, i, mkChar(buff));
    SET_STRING_ELT(ansnames, i++, mkChar("p_cs_precedes"));
    sprintf(buff, "%d", (int)lc->p_sep_by_space);
    SET_STRING_ELT(ans, i, mkChar(buff));
    SET_STRING_ELT(ansnames, i++, mkChar("p_sep_by_space"));
    sprintf(buff, "%d", (int)lc->n_cs_precedes);
    SET_STRING_ELT(ans, i, mkChar(buff));
    SET_STRING_ELT(ansnames, i++, mkChar("n_cs_precedes"));
    sprintf(buff, "%d", (int)lc->n_sep_by_space);
    SET_STRING_ELT(ans, i, mkChar(buff));
    SET_STRING_ELT(ansnames, i++, mkChar("n_sep_by_space"));
    sprintf(buff, "%d", (int)lc->p_sign_posn);
    SET_STRING_ELT(ans, i, mkChar(buff));
    SET_STRING_ELT(ansnames, i++, mkChar("p_sign_posn"));
    sprintf(buff, "%d", (int)lc->n_sign_posn);
    SET_STRING_ELT(ans, i, mkChar(buff));
    SET_STRING_ELT(ansnames, i++, mkChar("n_sign_posn"));
    setAttrib(ans, R_NamesSymbol, ansnames);
    UNPROTECT(2);
    return ans;
#else
    return R_NilValue;
#endif
}

/* .Internal function for path.expand */
SEXP attribute_hidden do_pathexpand(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP fn, ans;
    int i, n;

    checkArity(op, args);
    fn = CAR(args);
    if (!isString(fn))
	error(_("invalid '%s' argument"), "path");
    n = length(fn);
    PROTECT(ans = allocVector(STRSXP, n));
    for (i = 0; i < n; i++) {
        SEXP tmp = STRING_ELT(fn, i);
        if (tmp != NA_STRING) {
            tmp = markKnown(R_ExpandFileName(translateChar(tmp)), tmp);
        }
	SET_STRING_ELT(ans, i, tmp);
    }
    UNPROTECT(1);
    return ans;
}

#ifdef Unix
static int var_R_can_use_X11 = -1;

extern Rboolean R_access_X11(void); /* from src/unix/X11.c */

static Rboolean R_can_use_X11(void)
{
    if (var_R_can_use_X11 < 0) {
#ifdef HAVE_X11
	if (strcmp(R_GUIType, "none") != 0) {
	    /* At this point we have permission to use the module, so try it */
	    var_R_can_use_X11 = R_access_X11();
	} else {
	    var_R_can_use_X11 = 0;
	}
#else
	var_R_can_use_X11 = 0;
#endif
    }

    return var_R_can_use_X11 > 0;
}
#endif

/* only actually used on Unix */
SEXP attribute_hidden do_capabilitiesX11(SEXP call, SEXP op, SEXP args, SEXP rho)
{
#ifdef Unix
    return ScalarLogical(R_can_use_X11());
#else
    return ScalarLogical(FALSE);
#endif
}

SEXP attribute_hidden do_capabilities(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP ans, ansnames;
    int i = 0;
#ifdef Unix
# ifdef HAVE_X11
    int X11 = NA_LOGICAL;
# else
    int X11 = FALSE;
# endif
#endif

    checkArity(op, args);

    PROTECT(ans = allocVector(LGLSXP, 15));
    PROTECT(ansnames = allocVector(STRSXP, 15));

    SET_STRING_ELT(ansnames, i, mkChar("jpeg"));
#ifdef HAVE_JPEG
# if defined Unix && !defined HAVE_WORKING_CAIRO
    LOGICAL(ans)[i++] = X11;
# else
    LOGICAL(ans)[i++] = TRUE;
# endif
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("png"));
#ifdef HAVE_PNG
# if defined Unix && !defined HAVE_WORKING_CAIRO
    LOGICAL(ans)[i++] = X11;
# else /* Windows */
    LOGICAL(ans)[i++] = TRUE;
# endif
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("tiff"));
#ifdef HAVE_TIFF
# if defined Unix && !defined HAVE_WORKING_CAIRO
    LOGICAL(ans)[i++] = X11;
# else /* Windows */
    LOGICAL(ans)[i++] = TRUE;
# endif
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("tcltk"));
#ifdef HAVE_TCLTK
    LOGICAL(ans)[i++] = TRUE;
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("X11"));
#ifdef HAVE_X11
# if defined(Unix) /*  && !defined(__APPLE_CC__) removed in 2.11.0 */
    LOGICAL(ans)[i++] = X11;
# else
    LOGICAL(ans)[i++] = TRUE;
# endif
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("aqua"));
#ifdef HAVE_AQUA
    LOGICAL(ans)[i++] = TRUE;
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("http/ftp"));
#if HAVE_INTERNET
    LOGICAL(ans)[i++] = TRUE;
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("sockets"));
#ifdef HAVE_SOCKETS
    LOGICAL(ans)[i++] = TRUE;
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("libxml"));
#ifdef SUPPORT_LIBXML
    LOGICAL(ans)[i++] = TRUE;
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("fifo"));
#if defined(HAVE_MKFIFO) && defined(HAVE_FCNTL_H)
    LOGICAL(ans)[i++] = TRUE;
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    /* This one is complex.  Set it to be true only in interactive use,
       with the Windows and GNOME GUIs (but not Tk GUI) or under Unix
       if readline is available and in use. */
    SET_STRING_ELT(ansnames, i, mkChar("cledit"));
    LOGICAL(ans)[i] = FALSE;
#if defined(Win32)
    if (R_Interactive) LOGICAL(ans)[i] = TRUE;
#endif
#ifdef Unix
    if (strcmp(R_GUIType, "GNOME") == 0) {  /* always interactive */
	LOGICAL(ans)[i] = TRUE;  /* also AQUA ? */
    } else {
#if defined(HAVE_LIBREADLINE) && defined(HAVE_READLINE_HISTORY_H)
	extern Rboolean UsingReadline;
	if (R_Interactive && UsingReadline) LOGICAL(ans)[i] = TRUE;
#endif
    }
#endif
    i++;

/* always true as from R 2.10.0 */
    SET_STRING_ELT(ansnames, i, mkChar("iconv"));
    LOGICAL(ans)[i++] = TRUE;

    SET_STRING_ELT(ansnames, i, mkChar("NLS"));
#ifdef ENABLE_NLS
    LOGICAL(ans)[i++] = TRUE;
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("profmem"));
#ifdef R_MEMORY_PROFILING
    LOGICAL(ans)[i++] = TRUE;
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    SET_STRING_ELT(ansnames, i, mkChar("cairo"));
#ifdef HAVE_WORKING_CAIRO
    LOGICAL(ans)[i++] = TRUE;
#else
    LOGICAL(ans)[i++] = FALSE;
#endif

    setAttrib(ans, R_NamesSymbol, ansnames);
    UNPROTECT(2);
    return ans;
}

#if defined(HAVE_BSD_NETWORKING) && defined(HAVE_ARPA_INET_H)
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

SEXP attribute_hidden do_nsl(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP ans = R_NilValue;
    const char *name; char ip[] = "xxx.xxx.xxx.xxx";
    struct hostent *hp;

    checkArity(op, args);
    if (!isString(CAR(args)) || length(CAR(args)) != 1)
	error(_("'hostname' must be a character vector of length 1"));
    name = translateChar(STRING_ELT(CAR(args), 0));

    hp = gethostbyname(name);

    if (hp == NULL) {		/* cannot resolve the address */
	warning(_("nsl() was unable to resolve host '%s'"), name);
    } else {
	if (hp->h_addrtype == AF_INET) {
	    struct in_addr in;
	    memcpy(&in.s_addr, *(hp->h_addr_list), sizeof (in.s_addr));
	    strcpy(ip, inet_ntoa(in));
	} else {
	    warning(_("unknown format returned by gethostbyname"));
	}
	ans = mkString(ip);
    }
    return ans;
}
#else
SEXP attribute_hidden do_nsl(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    warning(_("nsl() is not supported on this platform"));
    return R_NilValue;
}
#endif

SEXP attribute_hidden do_sysgetpid(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    checkArity(op, args);
    return ScalarInteger(getpid());
}


#ifndef Win32
/* mkdir is defined in <sys/stat.h> */
SEXP attribute_hidden do_dircreate(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP path;
    int res, show, recursive, mode;
    char *p, dir[PATH_MAX];

    checkArity(op, args);
    path = CAR(args);
    if (!isString(path) || length(path) != 1)
	error(_("invalid '%s' argument"), "path");
    if (STRING_ELT(path, 0) == NA_STRING) return ScalarLogical(FALSE);
    show = asLogical(CADR(args));
    if (show == NA_LOGICAL) show = 0;
    recursive = asLogical(CADDR(args));
    if (recursive == NA_LOGICAL) recursive = 0;
    mode = asInteger(CADDDR(args));
    if (mode == NA_LOGICAL) mode = 0777;
    strcpy(dir, R_ExpandFileName(translateChar(STRING_ELT(path, 0))));
    /* remove trailing slashes */
    p = dir + strlen(dir) - 1;
    while (*p == '/' && strlen(dir) > 1) *p-- = '\0';
    if (recursive) {
	p = dir;
	while ((p = Rf_strchr(p+1, '/'))) {
	    *p = '\0';
	    res = mkdir(dir, mode);
	    /* Solaris 10 returns ENOSYS on automount, PR#13834
	       EROFS is allowed by POSIX, so we skip that too */
	    if (res && errno != EEXIST && errno != ENOSYS && errno != EROFS) 
		goto end;
	    *p = '/';
	}
    }
    res = mkdir(dir, mode);
    if (show && res && errno == EEXIST)
	warning(_("'%s' already exists"), dir);
end:
    if (show && res && errno != EEXIST)
	warning(_("cannot create dir '%s', reason '%s'"), dir, strerror(errno));
    return ScalarLogical(res == 0);
}
#else /* Win32 */
#include <io.h> /* mkdir is defined here */
SEXP attribute_hidden do_dircreate(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP  path;
    wchar_t *p, dir[MAX_PATH];
    int res, show, recursive;

    checkArity(op, args);
    path = CAR(args);
    if (!isString(path) || length(path) != 1)
	error(_("invalid '%s' argument"), "path");
    if (STRING_ELT(path, 0) == NA_STRING) return ScalarLogical(FALSE);
    show = asLogical(CADR(args));
    if (show == NA_LOGICAL) show = 0;
    recursive = asLogical(CADDR(args));
    if (recursive == NA_LOGICAL) recursive = 0;
    wcscpy(dir, filenameToWchar(STRING_ELT(path, 0), TRUE));
    for (p = dir; *p; p++) if (*p == L'/') *p = L'\\';
    /* remove trailing slashes */
    p = dir + wcslen(dir) - 1;
    while (*p == L'\\' && wcslen(dir) > 1 && *(p-1) != L':') *p-- = L'\0';
    if (recursive) {
	p = dir;
	/* skip leading \\share */
	if (*p == L'\\' && *(p+1) == L'\\') {
	    p += 2;
	    p = wcschr(p, L'\\');
	}
	while ((p = wcschr(p+1, L'\\'))) {
	    *p = L'\0';
	    if (*(p-1) != L':') {
		res = _wmkdir(dir);
		if (res && errno != EEXIST) goto end;
	    }
	    *p = L'\\';
	}
    }
    res = _wmkdir(dir);
    if (show && res && errno == EEXIST)
	warning(_("'%ls' already exists"), dir);
end:
    return ScalarLogical(res == 0);
}
#endif

/* take file/dir 'name' in dir 'from' and copy it to 'to' 
   'from', 'to' should have trailing path separator if needed.

   NB Windows' mkdir appears to require \ not /.
*/
static int do_copy(const char* from, const char* name, const char* to,
		   int over, int recursive)
{
    struct stat sb;
    int nc, nfail = 0, res;
    char dest[PATH_MAX], this[PATH_MAX];
#ifdef Win32
    const char *filesep = "\\";
#else
    const char *filesep = "/";
#endif

    /* REprintf("from: %s, name: %s, to: %s\n", from, name, to); */
    snprintf(this, PATH_MAX, "%s%s", from, name);
    stat(this, &sb);
    if ((sb.st_mode & S_IFDIR) > 0) { /* a directory */
	DIR *dir;
	struct dirent *de;
	char p[PATH_MAX];

	if (!recursive) return 1;
	nc = strlen(to);
	snprintf(dest, PATH_MAX, "%s%s", to, name);
#ifdef Win32
	res = mkdir(dest);
#else
	res = mkdir(dest, sb.st_mode);
#endif
	if (res && errno != EEXIST) return 1;
	strcat(dest, R_FileSep);
	if ((dir = opendir(this)) != NULL) {
	    while ((de = readdir(dir))) {
		if (streql(de->d_name, ".") || streql(de->d_name, ".."))
		    continue;
		snprintf(p, PATH_MAX, "%s%s%s", name, filesep, de->d_name);
		do_copy(from, p, to, over, recursive);
	    }
	    closedir(dir);
	} else nfail++; /* we were unable to read a dir */
    } else { /* a file */
	FILE *fp1 = NULL, *fp2 = NULL;
	char buf[APPENDBUFSIZE];

	nfail = 1;
	nc = strlen(to);
	snprintf(dest, PATH_MAX, "%s%s", to, name);
	if (over || !R_FileExists(dest)) {
	    /* REprintf("copying %s to %s\n", this, dest); */
	    if ((fp1 = R_fopen(this, "rb")) == NULL) goto copy_error;
	    if ((fp2 = R_fopen(dest, "wb")) == NULL) goto copy_error;
	    while ((nc = fread(buf, 1, APPENDBUFSIZE, fp1)) == APPENDBUFSIZE)
		if (fwrite(buf, 1, APPENDBUFSIZE, fp2) != APPENDBUFSIZE) 
		    goto copy_error;
	    if (fwrite(buf, 1, nc, fp2) != nc) goto copy_error;
	    nfail = 0;
	}
copy_error:
	if(fp2) fclose(fp2);
	if(fp1) fclose(fp1);
    }
    return nfail;
}

/* file.copy(files, dir, recursive), only */
SEXP attribute_hidden do_filecopy(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP fn, to, ans;
    char *p, dir[PATH_MAX], from[PATH_MAX], name[PATH_MAX];
    int i, nfiles, over, recursive, nfail;
#ifdef Win32
    const char *filesep = "\\", *cur = ".\\", sep = '\\';
#else
    const char *filesep = "/", *cur = "./", sep = '/';
#endif

    checkArity(op, args);
    fn = CAR(args);
    nfiles = length(fn);
    PROTECT(ans = allocVector(LGLSXP, nfiles));
    if (nfiles > 0) {
	if (!isString(fn))
	    error(_("invalid '%s' argument"), "from");
	to = CADR(args);
	if (!isString(to) || LENGTH(to) != 1)
	    error(_("invalid '%s' argument"), "to");
	over = asLogical(CADDR(args));
	if (over == NA_LOGICAL)
	    error(_("invalid '%s' argument"), "over");
	recursive = asLogical(CADDDR(args));
	if (recursive == NA_LOGICAL)
	    error(_("invalid '%s' argument"), "recursive");
	strncpy(dir, translateChar(STRING_ELT(to, 0)), PATH_MAX);
	if (*(dir + (strlen(dir) - 1)) !=  sep)
	    strncat(dir, filesep, PATH_MAX);
	for (i = 0; i < nfiles; i++) {
	    if (STRING_ELT(fn, i) != NA_STRING) {
		strncpy(from, translateChar(STRING_ELT(fn, i)), PATH_MAX);
		/* If there is a trailing sep, this is a mistake */
		p = from + (strlen(from) - 1);
		if(*p == sep) *p = '\0';
		p = strrchr(from, sep) ;
		if (p) {
		    strncpy(name, p+1, PATH_MAX);
		    *(p+1) = '\0';
		} else {
#ifdef Win32
		    if(strlen(from) > 2 && from[1] == ':') {
			strncpy(name, from+2, PATH_MAX);
			from[2] = '\0';
		    } else 
#endif
		    {
			strncpy(name, from, PATH_MAX);
			strncpy(from, cur, PATH_MAX);
		    }
		}
		nfail = do_copy(from, name, dir, over, recursive);
	    } else nfail = 1;
	    LOGICAL(ans)[i] = (nfail == 0);
	}
    }
    UNPROTECT(1);
    return ans;
}


SEXP attribute_hidden do_l10n_info(SEXP call, SEXP op, SEXP args, SEXP env)
{
#ifdef Win32
    int len = 4;
#else
    int len = 3;
#endif
    SEXP ans, names;
    checkArity(op, args);
    PROTECT(ans = allocVector(VECSXP, len));
    PROTECT(names = allocVector(STRSXP, len));
    SET_STRING_ELT(names, 0, mkChar("MBCS"));
    SET_STRING_ELT(names, 1, mkChar("UTF-8"));
    SET_STRING_ELT(names, 2, mkChar("Latin-1"));
    SET_VECTOR_ELT(ans, 0, ScalarLogical(mbcslocale));
    SET_VECTOR_ELT(ans, 1, ScalarLogical(utf8locale));
    SET_VECTOR_ELT(ans, 2, ScalarLogical(latin1locale));
#ifdef Win32
    SET_STRING_ELT(names, 3, mkChar("codepage"));
    SET_VECTOR_ELT(ans, 3, ScalarInteger(localeCP));
#endif
    setAttrib(ans, R_NamesSymbol, names);
    UNPROTECT(2);
    return ans;
}

#ifndef Win32 /* in src/gnuwin32/extra.c */
#ifndef HAVE_DECL_REALPATH
extern char *realpath(const char *path, char *resolved_path);
#endif

SEXP attribute_hidden do_normalizepath(SEXP call, SEXP op, SEXP args, SEXP rho)
{
#if defined(HAVE_GETCWD) && defined(HAVE_REALPATH)
    SEXP ans, paths = CAR(args);
    int i, n = LENGTH(paths);
    const char *path;
    char tmp[PATH_MAX+1], abspath[PATH_MAX+1], *res = NULL;
    Rboolean OK;

    checkArity(op, args);
    if (!isString(paths))
	error("'path' must be a character vector");
    PROTECT(ans = allocVector(STRSXP, n));
    for (i = 0; i < n; i++) {
	path = translateChar(STRING_ELT(paths, i));
	OK = strlen(path) <= PATH_MAX;
	if (OK) {
	    if (path[0] == '/') strncpy(abspath, path, PATH_MAX);
	    else {
		OK = getcwd(abspath, PATH_MAX) != NULL;
		OK = OK && (strlen(path) + strlen(abspath) + 1 <= PATH_MAX);
		if (OK) {
		    strcat(abspath, "/");
		    strcat(abspath, path);
		}
	    }
	}
	if (OK) res = realpath(abspath, tmp);
	if (OK && res) SET_STRING_ELT(ans, i, mkChar(tmp));
	else SET_STRING_ELT(ans, i, STRING_ELT(paths, i));
    }
    UNPROTECT(1);
    return ans;
#else
    checkArity(op, args);
    warning("insufficient OS support on this platform");
    return CAR(args);
#endif
}
#endif

SEXP attribute_hidden do_syschmod(SEXP call, SEXP op, SEXP args, SEXP env)
{
#ifdef HAVE_CHMOD
    SEXP paths, smode, ans;
    int i, m, n, *modes, mode, res;

    checkArity(op, args);
    paths = CAR(args);
    if (!isString(paths))
	error(_("invalid '%s' argument"), "paths");
    n = LENGTH(paths);
    PROTECT(smode = coerceVector(CADR(args), INTSXP));
    modes = INTEGER(smode);
    m = LENGTH(smode);
    if(!m) error(_("'mode' must be of length at least one"));
    PROTECT(ans = allocVector(LGLSXP, n));
    for (i = 0; i < n; i++) {
	mode = modes[i % m];
	if (mode == NA_INTEGER) mode = 0777;
#ifdef Win32
	/* Windows' _chmod seems only to support read access
	   or read-write access */
	mode = (mode & 0200) ? (_S_IWRITE | _S_IREAD): _S_IREAD;
#endif
	if (STRING_ELT(paths, i) != NA_STRING) {
#ifdef Win32
	    res = _wchmod(filenameToWchar(STRING_ELT(paths, i), TRUE), mode);
#else
	    res = chmod(R_ExpandFileName(translateChar(STRING_ELT(paths, i))),
			mode);
#endif
	} else res = 1;
	LOGICAL(ans)[i] = (res == 0);
    }
    UNPROTECT(2);
    return ans;
#else
    SEXP paths, ans;
    int i, n;

    checkArity(op, args);
    paths = CAR(args);
    if (!isString(paths))
	error(_("invalid '%s' argument"), "paths");
    n = LENGTH(paths);
    warning("insufficient OS support on this platform");
    PROTECT(ans = allocVector(LGLSXP, n));
    for (i = 0; i < n; i++) LOGICAL(ans)[i] = 0;
    UNPROTECT(1);
    return ans;
#endif
}

SEXP attribute_hidden do_sysumask(SEXP call, SEXP op, SEXP args, SEXP env)
{
#ifdef HAVE_UMASK
    SEXP ans;
    int mode;
    mode_t res;

    checkArity(op, args);
    mode = asInteger(CAR(args));
    if (mode == NA_LOGICAL)
	error(_("invalid '%s' value"), "umask");
    res = umask(mode);
    PROTECT(ans = ScalarInteger(res));
#else
    SEXP ans = R_NilValue;

    checkArity(op, args);
    warning(_("insufficient OS support on this platform"));
    PROTECT(ans = ScalarInteger(0));
#endif
    setAttrib(ans, R_ClassSymbol, mkString("octmode"));
    UNPROTECT(1);
    return ans;
}

SEXP attribute_hidden do_readlink(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP paths, ans;
    int n;
#ifdef HAVE_READLINK
    char buf[PATH_MAX+1];
    ssize_t res;
    int i;
#endif

    checkArity(op, args);
    paths = CAR(args);
    if(!isString(paths))
	error(_("invalid '%s' argument"), "paths");
    n = LENGTH(paths);
    PROTECT(ans = allocVector(STRSXP, n));
#ifdef HAVE_READLINK
    for (i = 0; i < n; i++) {
	memset(buf, 0, PATH_MAX+1);
	res = readlink(translateChar(STRING_ELT(paths, i)), buf, PATH_MAX);
	if (res >= 0) SET_STRING_ELT(ans, i, mkChar(buf));
	else if (errno == EINVAL) SET_STRING_ELT(ans, i, mkChar(""));
	else SET_STRING_ELT(ans, i,  NA_STRING);
    }
#endif
    UNPROTECT(1);
    return ans;
}


SEXP attribute_hidden do_Cstack_info(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP ans, nms;

    checkArity(op, args);
    PROTECT(ans = allocVector(INTSXP, 4));
    PROTECT(nms = allocVector(STRSXP, 4));
    INTEGER(ans)[0] = (R_CStackLimit == -1) ? NA_INTEGER : R_CStackLimit;
    INTEGER(ans)[1] = (R_CStackLimit == -1) ? NA_INTEGER :
	R_CStackDir * (R_CStackStart - (uintptr_t) &ans);
    INTEGER(ans)[2] = R_CStackDir;
    INTEGER(ans)[3] = R_EvalDepth;
    SET_STRING_ELT(nms, 0, mkChar("size"));
    SET_STRING_ELT(nms, 1, mkChar("current"));
    SET_STRING_ELT(nms, 2, mkChar("direction"));
    SET_STRING_ELT(nms, 3, mkChar("eval_depth"));

    UNPROTECT(2);
    setAttrib(ans, R_NamesSymbol, nms);
    return ans;
}

#ifdef Win32 /* untested */
static void winSetFileTime(const char *fn, time_t ftime)
{
    SYSTEMTIME st;
    FILETIME modft;
    struct tm *utctm;
    HANDLE hFile;

    utctm = gmtime(&ftime);
    if (!utctm) return; 

    st.wYear         = (WORD) utctm->tm_year + 1900;
    st.wMonth        = (WORD) utctm->tm_mon + 1;
    st.wDayOfWeek    = (WORD) utctm->tm_wday;
    st.wDay          = (WORD) utctm->tm_mday;
    st.wHour         = (WORD) utctm->tm_hour;
    st.wMinute       = (WORD) utctm->tm_min;
    st.wSecond       = (WORD) utctm->tm_sec;
    st.wMilliseconds = 0;
    if (!SystemTimeToFileTime(&st, &modft)) return;

    hFile = CreateFile(fn, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		       FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    SetFileTime(hFile, NULL, NULL, &modft);
    CloseHandle(hFile);
}
#else
# include <sys/time.h>
# include <utime.h>
#endif

SEXP attribute_hidden R_setFileTime(SEXP name, SEXP time)
{
    const char *fn = translateChar(STRING_ELT(name, 0));
    int ftime = asInteger(time);

#ifdef Win32
    winSetFileTime(fn, (time_t)ftime);
#else
    struct utimbuf settime;

    settime.actime = settime.modtime = ftime;
    utime(fn, &settime);
#endif
    return R_NilValue;
}
