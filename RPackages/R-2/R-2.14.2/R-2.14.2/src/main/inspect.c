/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2009,2011 The R Development Core Team.
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

/* This is an experimental facility for printing low-level information
   about R objects. It is not intended to be exposed at the top level
   but rather used as a debugging/inspection facility. It is not
   necessarily complete - feel free to add missing pieces. */

#define USE_RINTERNALS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>

/* print prefix */
static void pp(int pre) {
    /* this is sort of silly, I know, but it saves at least some output
       calls (and we can replace \t by spaces if desired) ... */
    while (pre >= 8) { Rprintf("\t"); pre -= 8; }
    while (pre-- > 0) Rprintf(" ");
}

static const char *typename(SEXP v) {
    switch (TYPEOF(v)) {
    case NILSXP:	return "NILSXP";
    case SYMSXP:	return "SYMSXP";
    case LISTSXP:	return "LISTSXP";
    case CLOSXP:	return "CLOSXP";
    case ENVSXP:	return "ENVSXP";
    case PROMSXP:	return "PROMSXP";
    case LANGSXP:	return "LANGSXP";
    case SPECIALSXP:	return "SPECIALSXP";
    case BUILTINSXP:	return "BUILTINSXP";
    case CHARSXP:	return "CHARSXP";
    case LGLSXP:	return "LGLSXP";
    case INTSXP:	return "INTSXP";
    case REALSXP:	return "REALSXP";
    case CPLXSXP:	return "CPLXSXP";
    case STRSXP:	return "STRSXP";
    case DOTSXP:	return "DOTSXP";
    case ANYSXP:	return "ANYSXP";
    case VECSXP:	return "VECSXP";
    case EXPRSXP:	return "EXPRSXP";
    case BCODESXP:	return "BCODESXP";
    case EXTPTRSXP:	return "EXTPTRSXP";
    case WEAKREFSXP:	return "WEAKREFSXP";
    case S4SXP:		return "S4SXP";
    case RAWSXP:	return "RAWSXP";
    default:
	return "<unknown>";
    }
}

/* pre is the prefix, v is the object to inspect, deep specifies
   the recursion behavior (0 = no recursion, -1 = [sort of] unlimited
   recursion, positive numbers define the maximum recursion depth)
   and pvec is the max. number of vector elements to show  */
static void inspect_tree(int pre, SEXP v, int deep, int pvec) {
    int a = 0;
    pp(pre);
    /* the use of %lx is deliberate because I hate the output of %p,
       but if this causes portability issues, it could be changed. */
#ifdef _W64
    Rprintf("@%p %02d %s g%dc%d [", v, TYPEOF(v), typename(v), v->sxpinfo.gcgen, v->sxpinfo.gccls);
#else
    Rprintf("@%lx %02d %s g%dc%d [", (long) v, TYPEOF(v), typename(v), v->sxpinfo.gcgen, v->sxpinfo.gccls);
#endif
    if (OBJECT(v)) { a = 1; Rprintf("OBJ"); }
    if (MARK(v)) { if (a) Rprintf(","); Rprintf("MARK"); a = 1; }
    if (NAMED(v)) { if (a) Rprintf(","); Rprintf("NAM(%d)",NAMED(v)); a = 1; }
    if (RDEBUG(v)) { if (a) Rprintf(","); Rprintf("DBG"); a = 1; }
    if (RTRACE(v)) { if (a) Rprintf(","); Rprintf("TR"); a = 1; }
    if (RSTEP(v)) { if (a) Rprintf(","); Rprintf("STP"); a = 1; }
    if (LEVELS(v)) { if (a) Rprintf(","); Rprintf("gp=0x%x", LEVELS(v)); a = 1; }
    if (ATTRIB(v) && ATTRIB(v) != R_NilValue) { if (a) Rprintf(","); Rprintf("ATT"); a = 1; }
    Rprintf("] ");
    switch (TYPEOF(v)) {
    case VECSXP: case STRSXP: case LGLSXP: case INTSXP: case RAWSXP:
    case REALSXP: case CPLXSXP: case EXPRSXP:
	Rprintf("(len=%d, tl=%d)", LENGTH(v), TRUELENGTH(v));
    }
    if (TYPEOF(v) == CHARSXP)
	Rprintf("\"%s\"", CHAR(v));
    if (TYPEOF(v) == SYMSXP)
	Rprintf("\"%s\"", CHAR(PRINTNAME(v)));
    switch (TYPEOF(v)) { /* for native vectors print the first elements in-line */
    case LGLSXP:
	if (LENGTH(v) > 0) {
		unsigned int i = 0;
		while (i < LENGTH(v) && i < pvec) {
		    Rprintf("%s%d", (i > 0) ? "," : " ", (int) LOGICAL(v)[i]);
		    i++;
		}
		if (i < LENGTH(v)) Rprintf(",...");
	}
	break;
    case INTSXP:
	if (LENGTH(v) > 0) {
	    unsigned int i = 0;
	    while (i < LENGTH(v) && i < pvec) {
		Rprintf("%s%d", (i > 0) ? "," : " ", INTEGER(v)[i]);
		i++;
	    }
	    if (i < LENGTH(v)) Rprintf(",...");
	}
	break;
    case RAWSXP:
	if (LENGTH(v) > 0) {
	    unsigned int i = 0;
	    while (i < LENGTH(v) && i < pvec) {
		Rprintf("%s%02x", (i > 0) ? "," : " ", (int) ((unsigned char) RAW(v)[i]));
		i++;
	    }
	    if (i < LENGTH(v)) Rprintf(",...");
	}
	break;
    case REALSXP:
	if (LENGTH(v) > 0) {
	    unsigned int i = 0;
	    while (i < LENGTH(v) && i < pvec) {
		Rprintf("%s%g", (i > 0) ? "," : " ", REAL(v)[i]);
		i++;
	    }
	    if (i < LENGTH(v)) Rprintf(",...");
	}
	break;
    }
    Rprintf("\n");
    if (deep) switch (TYPEOF(v)) {
	case VECSXP: case EXPRSXP:
	    {
		unsigned int i = 0;
		while (i<LENGTH(v) && i < pvec) {
		  inspect_tree(pre+2, VECTOR_ELT(v, i), deep - 1, pvec);
		    i++;
		}
		if (i<LENGTH(v)) { pp(pre+2); Rprintf("...\n"); }
	    }
	    break;
	case STRSXP:
	    {
		unsigned int i = 0;
		while (i < LENGTH(v) && i < pvec) {
		  inspect_tree(pre+2, STRING_ELT(v, i), deep - 1, pvec);
		    i++;
		}
		if (i < LENGTH(v)) { pp(pre+2); Rprintf("...\n"); }
	    }
	    break;
	case LISTSXP: case LANGSXP:
	    {
		SEXP lc = v;
		while (lc != R_NilValue) {
		    if (TAG(lc) && TAG(lc) != R_NilValue) {
			pp(pre + 2);
			Rprintf("TAG: "); /* TAG should be a one-liner since it's a symbol so we don't put it on an extra line*/
			inspect_tree(0, TAG(lc), deep - 1, pvec);
		    }		  
		    inspect_tree(pre + 2, CAR(lc), deep - 1, pvec);
		    lc=CDR(lc);
		}
	    }
	    break;
	case ENVSXP:
	    pp(pre); Rprintf("FRAME:\n");
	    inspect_tree(pre+2, FRAME(v), deep - 1, pvec);
	    pp(pre); Rprintf("ENCLOS:\n");
	    inspect_tree(pre+2, ENCLOS(v), 0, pvec);
	    pp(pre); Rprintf("HASHTAB:\n");
	    inspect_tree(pre+2, HASHTAB(v), deep - 1, pvec);
	    break;
	    
	case CLOSXP:
	    pp(pre); Rprintf("FORMALS:\n");
	    inspect_tree(pre+2, FORMALS(v), deep - 1, pvec);
	    pp(pre); Rprintf("BODY:\n");
	    inspect_tree(pre+2, BODY(v), deep - 1, pvec);
	    pp(pre); Rprintf("CLOENV:\n");
	    inspect_tree(pre+2, CLOENV(v), 0, pvec);
	    break;
	}
    
    if (ATTRIB(v) && ATTRIB(v) != R_NilValue && TYPEOF(v) != CHARSXP) {
	pp(pre); Rprintf("ATTRIB:\n"); inspect_tree(pre+2, ATTRIB(v), deep, pvec);
    }
}

/* internal API - takes one mandatory argument (object to inspect) and
   two optional arguments (deep and pvec - see above), positional argument
   matching only */
SEXP attribute_hidden do_inspect(SEXP call, SEXP op, SEXP args, SEXP env) {
    SEXP obj = CAR(args);
    int deep = -1;
    int pvec = 5;
    if (CDR(args) != R_NilValue) {
	deep = asInteger(CADR(args));
	if (CDDR(args) != R_NilValue)
	    pvec = asInteger(CADDR(args));
    }
	
    inspect_tree(0, CAR(args), deep, pvec);
    return obj;
}

/* the following functions can be use internally and for debugging purposes -
   so far they are not used in any actual code */
SEXP attribute_hidden R_inspect(SEXP x) {
    inspect_tree(0, x, -1, 5);
    return x;
}

SEXP attribute_hidden R_inspect3(SEXP x, int deep, int pvec) {
    inspect_tree(0, x, deep, pvec);
    return x;
}
