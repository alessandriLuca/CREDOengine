/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2011  The R Development Core Team.
 *  Copyright (C) 2003--2011  The R Foundation
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
 * Object Formatting
 *
 *  See ./paste.c for do_paste() , do_format() and  do_formatinfo()
 *  See ./printutils.c for general remarks on Printing and the Encode.. utils.
 *  See ./print.c  for do_printdefault, do_prmatrix, etc.
 *
 * Exports
 *	formatString
 *	formatLogical
 *	formatInteger
 *	formatReal
 *	formatComplex
 *
 * These  formatFOO() functions determine the proper width, digits, etc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <float.h> /* for DBL_EPSILON */
#include <Rmath.h>
#include <Print.h>

/* this is just for conformity with other types */
void formatRaw(Rbyte *x, int n, int *fieldwidth)
{
    *fieldwidth = 2;
}

void formatString(SEXP *x, int n, int *fieldwidth, int quote)
{
    int xmax = 0;
    int i, l;

    for (i = 0; i < n; i++) {
	if (x[i] == NA_STRING) {
	    l = quote ? R_print.na_width : R_print.na_width_noquote;
	} else l = Rstrlen(x[i], quote) + (quote ? 2 : 0);
	if (l > xmax) xmax = l;
    }
    *fieldwidth = xmax;
}

void formatLogical(int *x, int n, int *fieldwidth)
{
    int i;

    *fieldwidth = 1;
    for(i = 0 ; i < n; i++) {
	if (x[i] == NA_LOGICAL) {
	    if(*fieldwidth < R_print.na_width)
		*fieldwidth =  R_print.na_width;
	} else if (x[i] != 0 && *fieldwidth < 4) {
	    *fieldwidth = 4;
	} else if (x[i] == 0 && *fieldwidth < 5 ) {
	    *fieldwidth = 5;
	    break;
	    /* this is the widest it can be,  so stop */
	}
    }
}

void formatInteger(int *x, int n, int *fieldwidth)
{
    int xmin = INT_MAX, xmax = INT_MIN, naflag = 0;
    int i, l;

    for (i = 0; i < n; i++) {
	if (x[i] == NA_INTEGER)
	    naflag = 1;
	else {
	    if (x[i] < xmin) xmin = x[i];
	    if (x[i] > xmax) xmax = x[i];
	}
    }

    if (naflag) *fieldwidth = R_print.na_width;
    else *fieldwidth = 1;

    if (xmin < 0) {
	l = IndexWidth(-xmin) + 1;	/* +1 for sign */
	if (l > *fieldwidth) *fieldwidth = l;
    }
    if (xmax > 0) {
	l = IndexWidth(xmax);
	if (l > *fieldwidth) *fieldwidth = l;
    }
}

/*---------------------------------------------------------------------------
 * scientific format determination for real numbers.
 * This is time-critical code.	 It is worth optimizing.
 *
 *    nsig		digits altogether
 *    kpower+1		digits to the left of "."
 *    kpower+1+sgn	including sign
 *
 * Using GLOBAL	 R_print.digits	 -- had	 #define MAXDIG R_print.digits
*/

/* long double is C99, so should always be defined */
#if SIZEOF_LONG_DOUBLE > SIZEOF_DOUBLE
# ifdef HAVE_NEARBYINTL
# define R_nearbyintl nearbyintl
/* Cygwin had rintl but not nearbyintl */
# elif defined(HAVE_RINTL)
# define R_nearbyintl rintl
# else
# define R_nearbyintl private_nearbyintl
LDOUBLE private_nearbyintl(LDOUBLE x)
{
    LDOUBLE x1;
    x1 = - floorl(-x + 0.5);
    x = floorl(x + 0.5);
    if (x == x1) return(x);
    else {
	/* FIXME: we should really test for floorl, also C99.
	   But FreeBSD 7.x does have it, but not nearbyintl */
        if (x/2.0 == floorl(x/2.0)) return(x); else return(x1);
    }
}
# endif
# else /* no long double */
# ifdef HAVE_NEARBYINT
#  define R_nearbyint nearbyint
# elif defined(HAVE_RINTL)
#  define R_nearbyint rint
# else
#  define R_nearbyint private_rint
extern double private_rint(double x);/* in ../nmath/fround.c  */
# endif
#endif

#define NB 1000
static void format_via_sprintf(double r, int d, int *kpower, int *nsig)
{
    static char buff[NB];
    int i;
    snprintf(buff, NB, "%#.*e", d - 1, r);
    *kpower = strtol(buff + (d + 2), NULL, 10);
    for (i = d; i >= 2; i--)
        if (buff[i] != '0') break;
    *nsig = i;
}


static const LDOUBLE tbl[] =
{
    /* Powers exactly representable with 64 bit mantissa */
    1e00, 1e01, 1e02, 1e03, 1e04, 1e05, 1e06, 1e07, 1e08, 1e09,
    1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
    1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27
};

static void scientific(double *x, int *sgn, int *kpower, int *nsig, double eps)
{
    /* for a number x , determine
     *	sgn    = 1_{x < 0}  {0/1}
     *	kpower = Exponent of 10;
     *	nsig   = min(R_print.digits, #{significant digits of alpha})
     *
     * where  |x| = alpha * 10^kpower	and	 1 <= alpha < 10
     */
    register double alpha;
    register double r;
    register int kp;
    int j;

    if (*x == 0.0) {
	*kpower = 0;
	*nsig = 1;
	*sgn = 0;
    } else {
	if(*x < 0.0) {
	    *sgn = 1; r = -*x;
	} else {
	    *sgn = 0; r = *x;
	}
        if (R_print.digits >= DBL_DIG + 1) {
            format_via_sprintf(r, R_print.digits, kpower, nsig);
            return;
        }
        kp = floor(log10(r)) - R_print.digits + 1;/* r = |x|; 10^(kp + digits - 1) <= r */
#if SIZEOF_LONG_DOUBLE > SIZEOF_DOUBLE
        LDOUBLE r_prec = r;
        /* use exact scaling factor in long double precision, if possible */
        if (abs(kp) <= 27) {
            if (kp > 0) r_prec /= tbl[kp]; else if (kp < 0) r_prec *= tbl[ -kp];
        }
#ifdef HAVE_POWL
	else
            r_prec /= powl(10.0, (LDOUBLE) kp);
#else
        else if (kp <= R_dec_min_exponent)
            r_prec = (r_prec * 1e+303)/pow(10.0, (double)(kp+303));
        else
            r_prec /= pow(10.0, (double) kp);
#endif
        if (r_prec < tbl[R_print.digits - 1]) {
            r_prec *= 10.0;
            kp--;
        }
        /* round alpha to integer, 10^(digits-1) <= alpha <= 10^digits
	   accuracy limited by double rounding problem, 
	   alpha already rounded to 64 bits */
        alpha = R_nearbyintl(r_prec);
#else
        /* use exact scaling factor in double precision, if possible */
        if (abs(kp) <= 22) {
            if (kp >= 0) r /= tbl[kp]; else r *= tbl[ -kp];
        }
        /* on IEEE 1e-308 is not representable except by gradual underflow.
           Shifting by 303 allows for any potential denormalized numbers x,
           and makes the reasonable assumption that R_dec_min_exponent+303
           is in range. Representation of 1e+303 has low error.
         */
        else if (kp <= R_dec_min_exponent)
            r = (r * 1e+303)/pow(10.0, (double)(kp+303));
        else
            r /= pow(10.0, (double)kp);
        if (r < tbl[R_print.digits - 1]) {
            r *= 10.0;
            kp--;
        }
        /* round alpha to integer, 10^(digits-1) <= alpha <= 10^digits */
        /* accuracy limited by double rounding problem, alpha already rounded to 53 bits */
        alpha = R_nearbyint(r);
#endif
        *nsig = R_print.digits;
        for (j = 1; j <= R_print.digits; j++) {
            alpha /= 10.0;
            if (alpha == floor(alpha)) {
                (*nsig)--;
            } else {
                break;
            }
        }
        if (*nsig == 0) {
            *nsig = 1;
            kp += 1;
        }
        *kpower = kp + R_print.digits - 1;
    }
}

/*
   The return values are
     w : the required field width
     d : use %w.df in fixed format, %#w.de in scientific format
     e : use scientific format if != 0, value is number of exp digits - 1

   nsmall specifies the minimum number of decimal digits in fixed format:
   it is 0 except when called from do_format.
*/

void formatReal(double *x, int n, int *w, int *d, int *e, int nsmall)
{
    int left, right, sleft;
    int mnl, mxl, rgt, mxsl, mxns, wF;
    int neg, sgn, kpower, nsig;
    int i, naflag, nanflag, posinf, neginf;

    double eps = pow(10.0, -(double)R_print.digits);
    /* better to err on the side of too few signif digits rather than
       far too many */
    if(eps < 2*DBL_EPSILON) eps = 2*DBL_EPSILON;

    nanflag = 0;
    naflag = 0;
    posinf = 0;
    neginf = 0;
    neg = 0;
    rgt = mxl = mxsl = mxns = INT_MIN;
    mnl = INT_MAX;

    for (i = 0; i < n; i++) {
	if (!R_FINITE(x[i])) {
	    if(ISNA(x[i])) naflag = 1;
	    else if(ISNAN(x[i])) nanflag = 1;
	    else if(x[i] > 0) posinf = 1;
	    else neginf = 1;
	} else {
	    scientific(&x[i], &sgn, &kpower, &nsig, eps);

	    left = kpower + 1;
	    sleft = sgn + ((left <= 0) ? 1 : left); /* >= 1 */
	    right = nsig - left; /* #{digits} right of '.' ( > 0 often)*/
	    if (sgn) neg = 1;	 /* if any < 0, need extra space for sign */

	    /* Infinite precision "F" Format : */
	    if (right > rgt) rgt = right;	/* max digits to right of . */
	    if (left > mxl)  mxl = left;	/* max digits to  left of . */
	    if (left < mnl)  mnl = left;	/* min digits to  left of . */
	    if (sleft> mxsl) mxsl = sleft;	/* max left including sign(s)*/
	    if (nsig > mxns) mxns = nsig;	/* max sig digits */
	}
    }
    /* F Format: use "F" format WHENEVER we use not more space than 'E'
     *		and still satisfy 'R_print.digits' {but as if nsmall==0 !}
     *
     * E Format has the form   [S]X[.XXX]E+XX[X]
     *
     * This is indicated by setting *e to non-zero (usually 1)
     * If the additional exponent digit is required *e is set to 2
     */

    /*-- These	'mxsl' & 'rgt'	are used in F Format
     *	 AND in the	____ if(.) "F" else "E" ___   below: */
    if (mxl < 0) mxsl = 1 + neg;  /* we use %#w.dg, so have leading zero */

    /* use nsmall only *after* comparing "F" vs "E": */
    if (rgt < 0) rgt = 0;
    wF = mxsl + rgt + (rgt != 0);	/* width for F format */

    /*-- 'see' how "E" Exponential format would be like : */
    *e = (mxl > 100 || mnl <= -99) ? 2 /* 3 digit exponent */ : 1;
    if (mxns > 0) {
	*d = mxns - 1;
	*w = neg + (*d > 0) + *d + 4 + *e; /* width for E format */
	if (wF <= *w + R_print.scipen) { /* Fixpoint if it needs less space */
	    *e = 0;
	    if (nsmall > rgt) {
		rgt = nsmall;
		wF = mxsl + rgt + (rgt != 0);
	    }
	    *d = rgt;
	    *w = wF;
	} /* else : "E" Exponential format -- all done above */
    }
    else { /* when all x[i] are non-finite */
	*w = 0;/* to be increased */
	*d = 0;
	*e = 0;
    }
    if (naflag && *w < R_print.na_width)
	*w = R_print.na_width;
    if (nanflag && *w < 3) *w = 3;
    if (posinf && *w < 3) *w = 3;
    if (neginf && *w < 4) *w = 4;
}


/* As from 2.2.0 the number of digits applies to real and imaginary parts
   together, not separately */
void z_prec_r(Rcomplex *r, Rcomplex *x, double digits);

void formatComplex(Rcomplex *x, int n, int *wr, int *dr, int *er,
		   int *wi, int *di, int *ei, int nsmall)
{
/* format.info() or  x[1..l] for both Re & Im */
    int left, right, sleft;
    int rt, mnl, mxl, mxsl, mxns, wF, i_wF;
    int i_rt, i_mnl, i_mxl, i_mxsl, i_mxns;
    int neg, sgn;
    int i, kpower, nsig;
    int naflag;
    int rnanflag, rposinf, rneginf, inanflag, iposinf;
    Rcomplex tmp;
    Rboolean all_re_zero = TRUE, all_im_zero = TRUE;

    double eps = pow(10.0, -(double)R_print.digits);
    if(eps < 2*DBL_EPSILON) eps = 2*DBL_EPSILON;

    naflag = 0;
    rnanflag = 0;
    rposinf = 0;
    rneginf = 0;
    inanflag = 0;
    iposinf = 0;
    neg = 0;

    rt	=  mxl =  mxsl =  mxns = INT_MIN;
    i_rt= i_mxl= i_mxsl= i_mxns= INT_MIN;
    i_mnl = mnl = INT_MAX;

    for (i = 0; i < n; i++) {
	/* Now round */
	z_prec_r(&tmp, &(x[i]), R_print.digits);
	if(ISNA(tmp.r) || ISNA(tmp.i)) {
	    naflag = 1;
	} else {
	    /* real part */

	    if(!R_FINITE(tmp.r)) {
		if (ISNAN(tmp.r)) rnanflag = 1;
		else if (tmp.r > 0) rposinf = 1;
		else rneginf = 1;
	    } else {
		if(x[i].r != 0) all_re_zero = FALSE;
		scientific(&(tmp.r), &sgn, &kpower, &nsig, eps);

		left = kpower + 1;
		sleft = sgn + ((left <= 0) ? 1 : left); /* >= 1 */
		right = nsig - left; /* #{digits} right of '.' ( > 0 often)*/
		if (sgn) neg = 1; /* if any < 0, need extra space for sign */

		if (right > rt) rt = right;	/* max digits to right of . */
		if (left > mxl) mxl = left;	/* max digits to left of . */
		if (left < mnl) mnl = left;	/* min digits to left of . */
		if (sleft> mxsl) mxsl = sleft;	/* max left including sign(s) */
		if (nsig > mxns) mxns = nsig;	/* max sig digits */

	    }
	    /* imaginary part */

	    /* this is always unsigned */
	    /* we explicitly put the sign in when we print */

	    if(!R_FINITE(tmp.i)) {
		if (ISNAN(tmp.i)) inanflag = 1;
		else iposinf = 1;
	    } else {
		if(x[i].i != 0) all_im_zero = FALSE;
		scientific(&(tmp.i), &sgn, &kpower, &nsig, eps);

		left = kpower + 1;
		sleft = ((left <= 0) ? 1 : left);
		right = nsig - left;

		if (right > i_rt) i_rt = right;
		if (left > i_mxl) i_mxl = left;
		if (left < i_mnl) i_mnl = left;
		if (sleft> i_mxsl) i_mxsl = sleft;
		if (nsig > i_mxns) i_mxns = nsig;
	    }
	    /* done: ; */
	}
    }

    /* see comments in formatReal() for details on this */

    /* overall format for real part	*/

    if (mxl != INT_MIN) {
	if (mxl < 0) mxsl = 1 + neg;
	if (rt < 0) rt = 0;
	wF = mxsl + rt + (rt != 0);

	*er = (mxl > 100 || mnl < -99) ? 2 : 1;
	*dr = mxns - 1;
	*wr = neg + (*dr > 0) + *dr + 4 + *er;
    } else {
	*er = 0;
	*wr = 0;
	*dr = 0;
	wF = 0;
    }

    /* overall format for imaginary part */

    if (i_mxl != INT_MIN) {
	if (i_mxl < 0) i_mxsl = 1;
	if (i_rt < 0) i_rt = 0;
	i_wF = i_mxsl + i_rt + (i_rt != 0);

	*ei = (i_mxl > 100 || i_mnl < -99) ? 2 : 1;
	*di = i_mxns - 1;
	*wi = (*di > 0) + *di + 4 + *ei;
    } else {
	*ei = 0;
	*wi = 0;
	*di = 0;
	i_wF = 0;
    }

    /* Now make the fixed/scientific decision */
    if(all_re_zero) {
	*er = *dr = 0;
	*wr = wF;
	if (i_wF <= *wi + R_print.scipen) {
	    *ei = 0;
	    if (nsmall > i_rt) {i_rt = nsmall; i_wF = i_mxsl + i_rt + (i_rt != 0);}
	    *di = i_rt;
	    *wi = i_wF;
	}
    } else if(all_im_zero) {
	if (wF <= *wr + R_print.scipen) {
	    *er = 0;
	    if (nsmall > rt) {rt = nsmall; wF = mxsl + rt + (rt != 0);}
	    *dr = rt;
	    *wr = wF;
	    }
	*ei = *di = 0;
	*wi = i_wF;
    } else if(wF + i_wF < *wr + *wi + 2*R_print.scipen) {
	    *er = 0;
	    if (nsmall > rt) {rt = nsmall; wF = mxsl + rt + (rt != 0);}
	    *dr = rt;
	    *wr = wF;

	    *ei = 0;
	    if (nsmall > i_rt) {
		i_rt = nsmall;
		i_wF = i_mxsl + i_rt + (i_rt != 0);
	    }
	    *di = i_rt;
	    *wi = i_wF;
    } /* else scientific for both */
    if(*wr < 0) *wr = 0;
    if(*wi < 0) *wi = 0;

    /* Ensure space for Inf and NaN */
    if (rnanflag && *wr < 3) *wr = 3;
    if (rposinf &&  *wr < 3) *wr = 3;
    if (rneginf &&  *wr < 4) *wr = 4;
    if (inanflag && *wi < 3) *wi = 3;
    if (iposinf  && *wi < 3) *wi = 3;

    /* finally, ensure that there is space for NA */

    if (naflag && *wr+*wi+2 < R_print.na_width)
	*wr += (R_print.na_width -(*wr + *wi + 2));
}
