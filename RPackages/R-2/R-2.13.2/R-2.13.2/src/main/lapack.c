/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2001-2010 The R Development Core Team
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
#include <config.h>
#endif

#include <Defn.h>
#include <Rdynpriv.h>
#include <Rmodules/Rlapack.h>

static R_LapackRoutines *ptr;

/*
SEXP La_svd(SEXP jobu, SEXP jobv, SEXP x, SEXP s, SEXP u, SEXP v, SEXP method)
SEXP La_rs(SEXP x, SEXP only_values, SEXP method)
SEXP La_rg(SEXP x, SEXP only_values)
SEXP La_zgesv(SEXP A, SEXP B)
SEXP La_zgeqp3(SEXP A)
SEXP qr_coef_cmplx(SEXP Q, SEXP B)
SEXP qr_qy_cmplx(SEXP Q, SEXP B, SEXP trans)
SEXP La_svd_cmplx(SEXP jobu, SEXP jobv, SEXP x, SEXP s, SEXP u, SEXP v)
SEXP La_rs_complex(SEXP x, SEXP only_values)
SEXP La_rg_complex(SEXP x, SEXP only_values)
SEXP La_chol (SEXP A)
SEXP La_chol2inv (SEXP x, SEXP size)
SEXP La_dgesv(SEXP A, SEXP B, SEXP tol)
SEXP La_dgeqp3(SEXP A)
SEXP qr_coef_real(SEXP Q, SEXP B)
SEXP qr_qy_real(SEXP Q, SEXP B, SEXP trans)
SEXP det_ge_real(SEXP A, SEXP logarithm)
*/

static int initialized = 0;

#ifdef Win32
# include <fcntl.h>
#endif

static void La_Init(void)
{
    int res = R_moduleCdynload("lapack", 1, 1);
    initialized = -1;
    if(!res) return;
    if(!ptr->svd)
	error(_("lapack routines cannot be accessed in module"));
    initialized = 1;
#ifdef Win32
    /* gfortran initialization sets these to _O_BINARY */
    setmode(1,_O_TEXT); /* stdout */
    setmode(2,_O_TEXT); /* stderr */
#endif
    return;
}

/* Regretably, package 'party' calls this: attribute_hidden */
SEXP La_svd(SEXP jobu, SEXP jobv, SEXP x, SEXP s, SEXP u, SEXP v, SEXP method)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->svd)(jobu, jobv, x, s, u, v, method);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_svd_cmplx(SEXP jobu, SEXP jobv, SEXP x, SEXP s, SEXP u, SEXP v)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->svd_cmplx)(jobu, jobv, x, s, u, v);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_rs(SEXP x, SEXP only_values)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->rs)(x, only_values);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_rs_cmplx(SEXP x, SEXP only_values)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->rs_cmplx)(x, only_values);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_rg(SEXP x, SEXP only_values)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->rg)(x, only_values);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_rg_cmplx(SEXP x, SEXP only_values)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->rg_cmplx)(x, only_values);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_chol(SEXP A)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->chol)(A);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_chol2inv(SEXP x, SEXP size)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->chol2inv)(x, size);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_dgecon(SEXP A, SEXP norm)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->dgecon)(A, norm);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}
attribute_hidden
SEXP La_dtrcon(SEXP A, SEXP norm)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->dtrcon)(A, norm);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}
attribute_hidden
SEXP La_zgecon(SEXP A, SEXP norm)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->zgecon)(A, norm);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}
attribute_hidden
SEXP La_ztrcon(SEXP A, SEXP norm)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->ztrcon)(A, norm);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_dlange(SEXP A, SEXP type)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->dlange)(A, type);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_zgesv(SEXP A, SEXP B)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->zgesv)(A, B);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_zgeqp3(SEXP A)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->zgeqp3)(A);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP qr_coef_cmplx(SEXP Q, SEXP B)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->qr_coef_cmplx)(Q, B);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP qr_qy_cmplx(SEXP Q, SEXP B, SEXP trans)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->qr_qy_cmplx)(Q, B, trans);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_dgesv(SEXP A, SEXP B, SEXP tol)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->dgesv)(A, B, tol);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP La_dgeqp3(SEXP A)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->dgeqp3)(A);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP qr_coef_real(SEXP Q, SEXP B)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->qr_coef_real)(Q, B);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP qr_qy_real(SEXP Q, SEXP B, SEXP trans)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->qr_qy_real)(Q, B, trans);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

attribute_hidden
SEXP det_ge_real(SEXP A, SEXP logarithm)
{
    if(!initialized) La_Init();
    if(initialized > 0)
	return (*ptr->det_ge_real)(A, logarithm);
    else {
	error(_("lapack routines cannot be loaded"));
	return R_NilValue;
    }
}

R_LapackRoutines *
R_setLapackRoutines(R_LapackRoutines *routines)
{
    R_LapackRoutines *tmp;
    tmp = ptr;
    ptr = routines;
    return(tmp);
}
