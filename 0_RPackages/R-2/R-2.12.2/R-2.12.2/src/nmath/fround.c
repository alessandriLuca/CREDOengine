/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998 Ross Ihaka
 *  Copyright (C) 2000-2001, 2005 The R Development Core Team
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
 *  SYNOPSIS
 *
 *    #include <Rmath.h>
 *    double fround(double x, double digits);
 *
 *  DESCRIPTION
 *
 *    Rounds "x" to "digits" decimal digits.
 *
 */

#include <config.h> /* needed for HAVE_RINT and USE_BUILTIN_RINT */
#include "nmath.h"

/* USE_BUILTIN_RINT could also be defined by a configure test */
#ifndef HAVE_RINT
#define USE_BUILTIN_RINT
#endif

#ifdef USE_BUILTIN_RINT
#define R_rint private_rint

double attribute_hidden private_rint(double x)
{
    double tmp, sgn = 1.0;
    long ltmp;

    if (x != x) return x;			/* NaN */

    if (x < 0.0) {
	x = -x;
	sgn = -1.0;
    }

    if(x < (double) LONG_MAX) { /* in <limits.h> is architecture dependent */
	ltmp = x + 0.5;
	/* implement round to even */
	if(fabs(x + 0.5 - ltmp) < 10*DBL_EPSILON
	   && (ltmp % 2 == 1)) ltmp--;
	tmp = ltmp;
    } else {
	/* ignore round to even: too small a point to bother */
	tmp = floor(x + 0.5);
    }
    return sgn * tmp;
}
#else
#define R_rint rint
#endif

double fround(double x, double digits) {
#define MAX_DIGITS DBL_MAX_10_EXP
    /* = 308 (IEEE); was till R 0.99: (DBL_DIG - 1) */
    /* Note that large digits make sense for very small numbers */
    LDOUBLE pow10, sgn, intx;
    int dig;

#ifdef IEEE_754
    if (ISNAN(x) || ISNAN(digits))
	return x + digits;
    if(!R_FINITE(x)) return x;
#endif

    if (digits > MAX_DIGITS)
	digits = MAX_DIGITS;
    dig = (int)floor(digits + 0.5);
    if(x < 0.) {
	sgn = -1.;
	x = -x;
    } else
	sgn = 1.;
    if (dig == 0) {
	return sgn * R_rint(x);
    } else if (dig > 0) {
        pow10 = R_pow_di(10., dig);
	intx = floor(x);
	return sgn * (intx + R_rint((x-intx) * pow10) / pow10);
    } else {
        pow10 = R_pow_di(10., -dig);
        return sgn * R_rint(x/pow10) * pow10;
    }
}
