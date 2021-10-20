/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2010  The R Development Core Team
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

#define NIL -1
#define ARGUSED(x) LEVELS(x)
#define SET_ARGUSED(x,v) SETLEVELS(x,v)

/* Hash function and equality test for keys */
typedef struct _HashData HashData;

struct _HashData {
    int K, M;
    int (*hash)(SEXP, int, HashData *);
    int (*equal)(SEXP, int, SEXP, int);
    SEXP HashTable;

    int nomatch;
    Rboolean useUTF8;
    Rboolean useCache;
};


/*
   Integer keys are hashed via a random number generator
   based on Knuth's recommendations.  The high order K bits
   are used as the hash code.

   NB: lots of this code relies on M being a power of two and
   on silent integer overflow mod 2^32.  It also relies on M < 31.

   <FIXME>  Integer keys are wasteful for logical and raw vectors,
   but the tables are small in that case.
*/

static int scatter(unsigned int key, HashData *d)
{
    return 3141592653U * key >> (32 - d->K);
}

static int lhash(SEXP x, int indx, HashData *d)
{
    if (LOGICAL(x)[indx] == NA_LOGICAL)
	return 2;
    return LOGICAL(x)[indx];
}

static int ihash(SEXP x, int indx, HashData *d)
{
    if (INTEGER(x)[indx] == NA_INTEGER)
	return 0;
    return scatter((unsigned int) (INTEGER(x)[indx]), d);
}

/* We use unions here because Solaris gcc -O2 has trouble with
   casting + incrementing pointers.  We use tests here, but R currently
   assumes int is 4 bytes and double is 8 bytes.
 */
union foo {
    double d;
    unsigned int u[2];
};

static int rhash(SEXP x, int indx, HashData *d)
{
    /* There is a problem with signed 0s under IEEE */
    double tmp = (REAL(x)[indx] == 0.0) ? 0.0 : REAL(x)[indx];
    /* need to use both 32-byte chunks or endianness is an issue */
    /* we want all NaNs except NA equal, and all NAs equal */
    if (R_IsNA(tmp)) tmp = NA_REAL;
    else if (R_IsNaN(tmp)) tmp = R_NaN;
#if 2*SIZEOF_INT == SIZEOF_DOUBLE
    {
	union foo tmpu;
	tmpu.d = tmp;
	return scatter(tmpu.u[0] + tmpu.u[1], d);
    }
#else
    return scatter(*((unsigned int *) (&tmp)), d);
#endif
}

static int chash(SEXP x, int indx, HashData *d)
{
    Rcomplex tmp;
    unsigned int u;
    tmp.r = (COMPLEX(x)[indx].r == 0.0) ? 0.0 : COMPLEX(x)[indx].r;
    tmp.i = (COMPLEX(x)[indx].i == 0.0) ? 0.0 : COMPLEX(x)[indx].i;
    /* we want all NaNs except NA equal, and all NAs equal */
    if (R_IsNA(tmp.r)) tmp.r = NA_REAL;
    else if (R_IsNaN(tmp.r)) tmp.r = R_NaN;
    if (R_IsNA(tmp.i)) tmp.i = NA_REAL;
    else if (R_IsNaN(tmp.i)) tmp.i = R_NaN;
#if 2*SIZEOF_INT == SIZEOF_DOUBLE
    {
	union foo tmpu;
	tmpu.d = tmp.r;
	u = tmpu.u[0] ^ tmpu.u[1];
	tmpu.d = tmp.i;
	u ^= tmpu.u[0] ^ tmpu.u[1];
	return scatter(u, d);
    }
#else
	return scatter((*((unsigned int *)(&tmp.r)) ^
			(*((unsigned int *)(&tmp.i)))), d);
#endif
}

/* Hash CHARSXP by address.  Hash values are int, For 64bit pointers,
 * we do (upper ^ lower) */
static int cshash(SEXP x, int indx, HashData *d)
{
    intptr_t z = (intptr_t) STRING_ELT(x, indx);
    unsigned int z1 = z & 0xffffffff, z2 = 0;
#if SIZEOF_LONG == 8
    z2 = z/0x100000000L;
#endif
    return scatter(z1 ^ z2, d);
}

static int shash(SEXP x, int indx, HashData *d)
{
    unsigned int k;
    const char *p;
    const void *vmax = vmaxget();
    if(!d->useUTF8 && d->useCache) return cshash(x, indx, d);
    /* Not having d->useCache really should not happen anymore. */
    p = translateCharUTF8(STRING_ELT(x, indx));
    k = 0;
    while (*p++)
	    k = 11 * k + *p; /* was 8 but 11 isn't a power of 2 */
    vmaxset(vmax); /* discard any memory used by translateChar */
    return scatter(k, d);
}

static int lequal(SEXP x, int i, SEXP y, int j)
{
    if (i < 0 || j < 0) return 0;
    return (LOGICAL(x)[i] == LOGICAL(y)[j]);
}


static int iequal(SEXP x, int i, SEXP y, int j)
{
    if (i < 0 || j < 0) return 0;
    return (INTEGER(x)[i] == INTEGER(y)[j]);
}

/* BDR 2002-1-17  We don't want NA and other NaNs to be equal */
static int requal(SEXP x, int i, SEXP y, int j)
{
    if (i < 0 || j < 0) return 0;
    if (!ISNAN(REAL(x)[i]) && !ISNAN(REAL(y)[j]))
	return (REAL(x)[i] == REAL(y)[j]);
    else if (R_IsNA(REAL(x)[i]) && R_IsNA(REAL(y)[j])) return 1;
    else if (R_IsNaN(REAL(x)[i]) && R_IsNaN(REAL(y)[j])) return 1;
    else return 0;
}

static int cequal(SEXP x, int i, SEXP y, int j)
{
    if (i < 0 || j < 0) return 0;
    if (!ISNAN(COMPLEX(x)[i].r) && !ISNAN(COMPLEX(x)[i].i)
       && !ISNAN(COMPLEX(y)[j].r) && !ISNAN(COMPLEX(y)[j].i))
	return COMPLEX(x)[i].r == COMPLEX(y)[j].r &&
	    COMPLEX(x)[i].i == COMPLEX(y)[j].i;
    else if ((R_IsNA(COMPLEX(x)[i].r) || R_IsNA(COMPLEX(x)[i].i))
	    && (R_IsNA(COMPLEX(y)[j].r) || R_IsNA(COMPLEX(y)[j].i)))
	return 1;
    else if ((R_IsNaN(COMPLEX(x)[i].r) || R_IsNaN(COMPLEX(x)[i].i))
	    && (R_IsNaN(COMPLEX(y)[j].r) || R_IsNaN(COMPLEX(y)[j].i)))
	return 1;
    else
	return 0;
}

static int sequal(SEXP x, int i, SEXP y, int j)
{
    if (i < 0 || j < 0) return 0;
    /* Two strings which have the same address must be the same,
       so avoid looking at the contents */
    if (STRING_ELT(x, i) == STRING_ELT(y, j)) return 1;
    /* Then if either is NA the other cannot be */
    /* Once all CHARSXPs are cached, Seql will handle this */
    if (STRING_ELT(x, i) == NA_STRING || STRING_ELT(y, j) == NA_STRING)
	return 0;
    return Seql(STRING_ELT(x, i), STRING_ELT(y, j));
}

static int rawhash(SEXP x, int indx, HashData *d)
{
    return RAW(x)[indx];
}

static int rawequal(SEXP x, int i, SEXP y, int j)
{
    if (i < 0 || j < 0) return 0;
    return (RAW(x)[i] == RAW(y)[j]);
}

static int vhash(SEXP x, int indx, HashData *d)
{
    int i;
    unsigned int key;
    SEXP _this = VECTOR_ELT(x, indx);

    key = OBJECT(_this) + 2*TYPEOF(_this) + 100*length(_this);
    /* maybe we should also look at attributes, but that slows us down */
    switch (TYPEOF(_this)) {
    case LGLSXP:
	/* This is not too clever: pack into 32-bits and then scatter? */
	for(i = 0; i < LENGTH(_this); i++) {
	    key ^= lhash(_this, i, d);
	    key *= 97;
	}
	break;
    case INTSXP:
	for(i = 0; i < LENGTH(_this); i++) {
	    key ^= ihash(_this, i, d);
	    key *= 97;
	}
	break;
    case REALSXP:
	for(i = 0; i < LENGTH(_this); i++) {
	    key ^= rhash(_this, i, d);
	    key *= 97;
	}
	break;
    case CPLXSXP:
	for(i = 0; i < LENGTH(_this); i++) {
	    key ^= chash(_this, i, d);
	    key *= 97;
	}
	break;
    case STRSXP:
	for(i = 0; i < LENGTH(_this); i++) {
	    key ^= shash(_this, i, d);
	    key *= 97;
	}
	break;
    case RAWSXP:
	for(i = 0; i < LENGTH(_this); i++) {
	    key ^= scatter(rawhash(_this, i, d), d);
	    key *= 97;
	}
	break;
    case VECSXP:
	for(i = 0; i < LENGTH(_this); i++) {
	    key ^= vhash(_this, i, d);
	    key *= 97;
	}
	break;
    default:
	break;
    }
    return scatter(key, d);
}

static int vequal(SEXP x, int i, SEXP y, int j)
{
    if (i < 0 || j < 0) return 0;
    return R_compute_identical(VECTOR_ELT(x, i), VECTOR_ELT(y, j), 0);
}

/*
  Choose M to be the smallest power of 2
  not less than 2*n and set K = log2(M).
  Need K >= 1 and hence M >= 2, and 2^M <= 2^31 -1, hence n <= 2^30.

  Dec 2004: modified from 4*n to 2*n, since in the worst case we have
  a 50% full table, and that is still rather efficient -- see
  R. Sedgewick (1998) Algorithms in C++ 3rd edition p.606.
*/
static void MKsetup(int n, HashData *d)
{
    int n2 = 2 * n;

    if(n < 0 || n > 1073741824) /* protect against overflow to -ve */
	error(_("length %d is too large for hashing"), n);
    d->M = 2;
    d->K = 1;
    while (d->M < n2) {
	d->M *= 2;
	d->K += 1;
    }
}

static void HashTableSetup(SEXP x, HashData *d)
{
    d->useUTF8 = FALSE;
    d->useCache = TRUE;
    switch (TYPEOF(x)) {
    case LGLSXP:
	d->hash = lhash;
	d->equal = lequal;
	MKsetup(3, d);
	break;
    case INTSXP:
	d->hash = ihash;
	d->equal = iequal;
	MKsetup(LENGTH(x), d);
	break;
    case REALSXP:
	d->hash = rhash;
	d->equal = requal;
	MKsetup(LENGTH(x), d);
	break;
    case CPLXSXP:
	d->hash = chash;
	d->equal = cequal;
	MKsetup(LENGTH(x), d);
	break;
    case STRSXP:
	d->hash = shash;
	d->equal = sequal;
	MKsetup(LENGTH(x), d);
	break;
    case RAWSXP:
	d->hash = rawhash;
	d->equal = rawequal;
	d->M = 256;
	d->K = 8; /* unused */
	break;
    case VECSXP:
	d->hash = vhash;
	d->equal = vequal;
	MKsetup(LENGTH(x), d);
	break;
    default:
	UNIMPLEMENTED_TYPE("HashTableSetup", x);
    }
    d->HashTable = allocVector(INTSXP, d->M);
}

/* Open address hashing */
/* Collision resolution is by linear probing */
/* The table is guaranteed large so this is sufficient */

static int isDuplicated(SEXP x, int indx, HashData *d)
{
    int i, *h;

    h = INTEGER(d->HashTable);
    i = d->hash(x, indx, d);
    while (h[i] != NIL) {
	if (d->equal(x, h[i], x, indx))
	    return h[i] >= 0 ? 1 : 0;
	i = (i + 1) % d->M;
    }
    h[i] = indx;
    return 0;
}

static void removeEntry(SEXP table, SEXP x, int indx, HashData *d)
{
    int i, *h;

    h = INTEGER(d->HashTable);
    i = d->hash(x, indx, d);
    while (h[i] >= 0) {
	if (d->equal(table, h[i], x, indx)) {
	    h[i] = NA_INTEGER;  /* < 0, only index values are inserted */
	    return;
	}
	i = (i + 1) % d->M;
    }
}

SEXP duplicated(SEXP x, Rboolean from_last)
{
    SEXP ans;
    int *v;
#define DUPLICATED_INIT						\
    int *h, i, n;						\
    HashData data;						\
								\
    if (!isVector(x))						\
	error(_("'duplicated' applies only to vectors"));	\
								\
    n = LENGTH(x);						\
    HashTableSetup(x, &data);					\
    h = INTEGER(data.HashTable);				\
    if(TYPEOF(x) == STRSXP) {					\
	data.useUTF8 = FALSE; data.useCache = TRUE;		\
	for(i = 0; i < length(x); i++) {			\
	    if(IS_BYTES(STRING_ELT(x, i))) {			\
		data.useUTF8 = FALSE; break;			\
	    }                                                   \
    	    if(ENC_KNOWN(STRING_ELT(x, i))) {                   \
		data.useUTF8 = TRUE;                            \
	    }							\
	    if(!IS_CACHED(STRING_ELT(x, i))) {                  \
		data.useCache = FALSE; break;                   \
	    }							\
	}							\
    }

    DUPLICATED_INIT;

    PROTECT(data.HashTable);
    PROTECT(ans = allocVector(LGLSXP, n));

    v = LOGICAL(ans);

    for (i = 0; i < data.M; i++) h[i] = NIL;
    if(from_last)
	for (i = n-1; i >= 0; i--) v[i] = isDuplicated(x, i, &data);
    else
	for (i = 0; i < n; i++) v[i] = isDuplicated(x, i, &data);

    UNPROTECT(2);
    return ans;
}

/* simpler version of the above : return 1-based index of first, or 0 : */
int any_duplicated(SEXP x, Rboolean from_last)
{
    int result = 0;
    DUPLICATED_INIT;
    PROTECT(data.HashTable);

    for (i = 0; i < data.M; i++) h[i] = NIL;
    if(from_last) {
	for (i = n-1; i >= 0; i--) if(isDuplicated(x, i, &data)) { result = ++i; break; }
    } else {
	for (i = 0; i < n; i++)    if(isDuplicated(x, i, &data)) { result = ++i; break; }
    }
    UNPROTECT(1);
    return result;
}

SEXP duplicated3(SEXP x, SEXP incomp, Rboolean from_last)
{
    SEXP ans;
    int *v, j, m;

    DUPLICATED_INIT;

    PROTECT(data.HashTable);
    PROTECT(ans = allocVector(LGLSXP, n));

    v = LOGICAL(ans);

    for (i = 0; i < data.M; i++) h[i] = NIL;
    if(from_last)
	for (i = n-1; i >= 0; i--) v[i] = isDuplicated(x, i, &data);
    else
	for (i = 0; i < n; i++) v[i] = isDuplicated(x, i, &data);

    if(length(incomp)) {
	PROTECT(incomp = coerceVector(incomp, TYPEOF(x)));
	m = length(incomp);
	for (i = 0; i < n; i++)
	    if(v[i]) {
		for(j = 0; j < m; j++)
		    if(data.equal(x, i, incomp, j)) {v[i] = 0; break;}
	    }
	UNPROTECT(1);
    }
    UNPROTECT(2);
    return ans;
}

/* return (1-based) index of first duplication, or 0 : */
int any_duplicated3(SEXP x, SEXP incomp, Rboolean from_last)
{
    int j, m = length(incomp);

    DUPLICATED_INIT;
    PROTECT(data.HashTable);

    if(!m)
	error(_("any_duplicated3(., <0-length incomp>)"));

    PROTECT(incomp = coerceVector(incomp, TYPEOF(x)));
    m = length(incomp);

    for (i = 0; i < data.M; i++) h[i] = NIL;
    if(from_last)
	for (i = n-1; i >= 0; i--) {
#define IS_DUPLICATED_CHECK				\
	    if(isDuplicated(x, i, &data)) {		\
		Rboolean isDup = TRUE;			\
		for(j = 0; j < m; j++)			\
		    if(data.equal(x, i, incomp, j)) {	\
			isDup = FALSE; break;		\
		    }					\
		if(isDup) {				\
		    UNPROTECT(1);			\
		    return ++i;				\
 	        }					\
		/* else continue */			\
	    }
	    IS_DUPLICATED_CHECK;
	}
    else {
	for (i = 0; i < n; i++)
            IS_DUPLICATED_CHECK;
    }

    UNPROTECT(2);
    return 0;
}

#undef IS_DUPLICATED_CHECK
#undef DUPLICATED_INIT


/* .Internal(duplicated(x))       [op=0]
   .Internal(unique(x))	          [op=1]
   .Internal(anyDuplicated(x))    [op=2]
*/
SEXP attribute_hidden do_duplicated(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP x, incomp, dup, ans;
    int i, k, n, fromLast;

    checkArity(op, args);
    x = CAR(args);
    incomp = CADR(args);
    if (length(CADDR(args)) < 1)
	error(_("'fromLast' must be length 1"));
    fromLast = asLogical(CADDR(args));
    if (fromLast == NA_LOGICAL)
	error(_("'fromLast' must be TRUE or FALSE"));

    /* handle zero length vectors, and NULL */
    if ((n = length(x)) == 0)
	return(PRIMVAL(op) <= 1
	       ? allocVector(PRIMVAL(op) != 1 ? LGLSXP : TYPEOF(x), 0)
	       : ScalarInteger(0));

    if (!isVector(x)) {
	error(_("%s() applies only to vectors"),
	      (PRIMVAL(op) == 0 ? "duplicated" :
	       (PRIMVAL(op) == 1 ? "unique" : /* 2 */ "anyDuplicated")));
    }

    if(length(incomp) && /* S has FALSE to mean empty */
       !(isLogical(incomp) && length(incomp) == 1 && LOGICAL(incomp)[0] == 0)) {
	if(PRIMVAL(op) == 2) /* return R's 1-based index :*/
	    return ScalarInteger(any_duplicated3(x, incomp, fromLast));
	else
	    dup = duplicated3(x, incomp, fromLast);
    }
    else {
	if(PRIMVAL(op) == 2)
	    return ScalarInteger(any_duplicated(x, fromLast));
	else
	    dup = duplicated(x, fromLast);
    }
    if (PRIMVAL(op) == 0) /* "duplicated()" */
	return dup;
    /*	ELSE
	use the results of "duplicated" to get "unique" */

    /* count unique entries */
    k = 0;
    for (i = 0; i < n; i++)
	if (LOGICAL(dup)[i] == 0)
	    k++;

    PROTECT(dup);
    PROTECT(ans = allocVector(TYPEOF(x), k));

    k = 0;
    switch (TYPEOF(x)) {
    case LGLSXP:
    case INTSXP:
	for (i = 0; i < n; i++)
	    if (LOGICAL(dup)[i] == 0)
		INTEGER(ans)[k++] = INTEGER(x)[i];
	break;
    case REALSXP:
	for (i = 0; i < n; i++)
	    if (LOGICAL(dup)[i] == 0)
		REAL(ans)[k++] = REAL(x)[i];
	break;
    case CPLXSXP:
	for (i = 0; i < n; i++)
	    if (LOGICAL(dup)[i] == 0) {
		COMPLEX(ans)[k].r = COMPLEX(x)[i].r;
		COMPLEX(ans)[k].i = COMPLEX(x)[i].i;
		k++;
	    }
	break;
    case STRSXP:
	for (i = 0; i < n; i++)
	    if (LOGICAL(dup)[i] == 0)
		SET_STRING_ELT(ans, k++, STRING_ELT(x, i));
	break;
    case VECSXP:
	for (i = 0; i < n; i++)
	    if (LOGICAL(dup)[i] == 0)
		SET_VECTOR_ELT(ans, k++, VECTOR_ELT(x, i));
	break;
    case RAWSXP:
	for (i = 0; i < n; i++)
	    if (LOGICAL(dup)[i] == 0)
		RAW(ans)[k++] = RAW(x)[i];
	break;
    default:
	UNIMPLEMENTED_TYPE("duplicated", x);
    }
    UNPROTECT(2);
    return ans;
}

/* Build a hash table, ignoring information on duplication */
static void DoHashing(SEXP table, HashData *d)
{
    int *h, i, n;

    n = LENGTH(table);
    h = INTEGER(d->HashTable);

    for (i = 0; i < d->M; i++)
	h[i] = NIL;

    for (i = 0; i < n; i++)
	(void) isDuplicated(table, i, d);
}

/* invalidate entries */
static void UndoHashing(SEXP x, SEXP table, HashData *d)
{
    for (int i = 0; i < LENGTH(x); i++) removeEntry(table, x, i, d);
}

static int Lookup(SEXP table, SEXP x, int indx, HashData *d)
{
    int i, *h;

    h = INTEGER(d->HashTable);
    i = d->hash(x, indx, d);
    while (h[i] != NIL) {
	if (d->equal(table, h[i], x, indx))
	    return h[i]>= 0 ? h[i] + 1 : d->nomatch;
	i = (i + 1) % d->M;
    }
    return d->nomatch;
}

/* Now do the table lookup */
static SEXP HashLookup(SEXP table, SEXP x, HashData *d)
{
    SEXP ans;
    int i, n;

    n = LENGTH(x);
    PROTECT(ans = allocVector(INTSXP, n));
    for (i = 0; i < n; i++) {
	INTEGER(ans)[i] = Lookup(table, x, i, d);
    }
    UNPROTECT(1);
    return ans;
}

static SEXP match_transform(SEXP s, SEXP env)
{
    if(OBJECT(s)) {
	if(inherits(s, "factor"))
	    return asCharacterFactor(s);
	else if(inherits(s, "POSIXlt")) { /* and maybe more classes in the future:
					   * Call R's (generic)  as.character(s) : */
	    SEXP call, r;
	    PROTECT(call = lang2(install("as.character"), s));
	    PROTECT(r = eval(call, env));
	    UNPROTECT(2);
	    return r;
	}
    }
    /* else */
    return duplicate(s);
}

SEXP match5(SEXP itable, SEXP ix, int nmatch, SEXP incomp, SEXP env)
{
    SEXP ans, x, table;
    SEXPTYPE type;
    HashData data;

    int i, n = length(ix), nprot = 0;

    /* handle zero length arguments */
    if (n == 0) return allocVector(INTSXP, 0);
    if (length(itable) == 0) {
	ans = allocVector(INTSXP, n);
	for (i = 0; i < n; i++) INTEGER(ans)[i] = nmatch;
	return ans;
    }

    PROTECT(x     = match_transform(ix,     env)); nprot++;
    PROTECT(table = match_transform(itable, env)); nprot++;
    /* or should we use PROTECT_WITH_INDEX  and  REPROTECT below ? */

    /* Coerce to a common type; type == NILSXP is ok here.
     * Note that above we coerce factors and "POSIXlt", only to character.
     * Hence, coerce to character or to `higher' type
     * (given that we have "Vector" or NULL) */
    if(TYPEOF(x) >= STRSXP || TYPEOF(table) >= STRSXP)
	type = STRSXP;
    else type = TYPEOF(x) < TYPEOF(table) ? TYPEOF(table) : TYPEOF(x);
    PROTECT(x     = coerceVector(x,     type)); nprot++;
    PROTECT(table = coerceVector(table, type)); nprot++;
    if (incomp) { PROTECT(incomp = coerceVector(incomp, type)); nprot++; }
    data.nomatch = nmatch;
    HashTableSetup(table, &data);
    if(type == STRSXP) {
	Rboolean useBytes = FALSE;
	Rboolean useUTF8 = FALSE;
        Rboolean useCache = TRUE;
	for(i = 0; i < length(x); i++) {
            SEXP s = STRING_ELT(x, i);
	    if(IS_BYTES(s)) {
		useBytes = TRUE;
		useUTF8 = FALSE;
		break;
	    }
	    if(ENC_KNOWN(s)) {
		useUTF8 = TRUE;
	    }
            if(!IS_CACHED(s)) {
		useCache = FALSE;
		break;
	    }
        }
	if(!useBytes || useCache) {
	    for(i = 0; i < length(table); i++) {
                SEXP s = STRING_ELT(table, i);
		if(IS_BYTES(s)) {
		    useBytes = TRUE;
		    useUTF8 = FALSE;
		    break;
		}
		if(ENC_KNOWN(s)) {
		    useUTF8 = TRUE;
		}
		if(!IS_CACHED(s)) {
		    useCache = FALSE;
		    break;
		}
            }
        }
	data.useUTF8 = useUTF8;
        data.useCache = useCache;
    }
    PROTECT(data.HashTable); nprot++;
    DoHashing(table, &data);
    if (incomp) UndoHashing(incomp, table, &data);
    ans = HashLookup(table, x, &data);
    UNPROTECT(nprot);
    return ans;
}

SEXP matchE(SEXP itable, SEXP ix, int nmatch, SEXP env)
{
    return match5(itable, ix, nmatch, NULL, env);
}

/* used from other code, not here: */
SEXP match(SEXP itable, SEXP ix, int nmatch)
{
    return match5(itable, ix, nmatch, NULL, R_BaseEnv);
}


SEXP attribute_hidden do_match(SEXP call, SEXP op, SEXP args, SEXP env)
{
    int nomatch, nargs = length(args);
    SEXP incomp;

    /* checkArity(op, args); too many packages have captured 3 from R < 2.7.0 */
    if (nargs < 3 || nargs > 4)
	error("%d arguments passed to .Internal(%s) which requires %d",
	      length(args), PRIMNAME(op), PRIMARITY(op));
    if (nargs == 3)
	warning("%d arguments passed to .Internal(%s) which requires %d",
		length(args), PRIMNAME(op), PRIMARITY(op));

    if ((!isVector(CAR(args)) && !isNull(CAR(args)))
	|| (!isVector(CADR(args)) && !isNull(CADR(args))))
	error(_("'match' requires vector arguments"));

    nomatch = asInteger(CADDR(args));
    if (nargs < 4) return matchE(CADR(args), CAR(args), nomatch, env);

    incomp = CADDDR(args);

    if(length(incomp) && /* S has FALSE to mean empty */
       !(isLogical(incomp) && length(incomp) == 1 && LOGICAL(incomp)[0] == 0))
	return match5(CADR(args), CAR(args), nomatch, incomp, env);
    else
	return matchE(CADR(args), CAR(args), nomatch, env);
}

/* Partial Matching of Strings */
/* Fully S Compatible version. */

/* Hmm, this was not all S compatible!  The desired behaviour is:
 * First do exact matches, and mark elements as used as they are matched
 *   unless dup_ok is true.
 * Then do partial matching, from left to right, using up the table
 *   unless dup_ok is true.  Multiple partial matches are ignored.
 * Empty strings are unmatched                        BDR 2000/2/16
 */

SEXP attribute_hidden do_pmatch(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, input, target;
    int i, j,  mtch, n_input, n_target, mtch_count, dups_ok, no_match;
    size_t temp;
    int nexact = 0;
    int *used = NULL, *ians;
    const char **in, **tar;
    Rboolean no_dups;
    Rboolean useBytes = FALSE, useUTF8 = FALSE;

    checkArity(op, args);
    input = CAR(args);
    n_input = LENGTH(input);
    target = CADR(args);
    n_target = LENGTH(target);
    no_match = asInteger(CADDR(args));
    dups_ok = asLogical(CADDDR(args));
    if (dups_ok == NA_LOGICAL)
	error(_("invalid '%s' argument"), "duplicates.ok");
    no_dups = !dups_ok;

    if (!isString(input) || !isString(target))
	error(_("argument is not of mode character"));

    if(no_dups) {
	used = (int *) R_alloc((size_t) n_target, sizeof(int));
	for (j = 0; j < n_target; j++) used[j] = 0;
    }

    for(i = 0; i < n_input; i++) {
	if(IS_BYTES(STRING_ELT(input, i))) {
	    useBytes = TRUE;
	    useUTF8 = FALSE;
	    break;
	} else if(ENC_KNOWN(STRING_ELT(input, i))) {
	    useUTF8 = TRUE;
	}
    }
    if(!useBytes) {
	for(i = 0; i < n_target; i++) {
	    if(IS_BYTES(STRING_ELT(target, i))) {
		useBytes = TRUE;
		useUTF8 = FALSE;
		break;
	    } else if(ENC_KNOWN(STRING_ELT(target, i))) {
		useUTF8 = TRUE;
	    }
	}
    }

    in = (const char **) R_alloc((size_t) n_input, sizeof(char *));
    tar = (const char **) R_alloc((size_t) n_target, sizeof(char *));
    PROTECT(ans = allocVector(INTSXP, n_input));
    ians = INTEGER(ans);
    if(useBytes) {
	for(i = 0; i < n_input; i++) {
	    in[i] = CHAR(STRING_ELT(input, i));
	    ians[i] = 0;
	}
	for(j = 0; j < n_target; j++)
	    tar[j] = CHAR(STRING_ELT(target, j));
    }
    else if(useUTF8) {
	for(i = 0; i < n_input; i++) {
	    in[i] = translateCharUTF8(STRING_ELT(input, i));
	    ians[i] = 0;
	}
	for(j = 0; j < n_target; j++)
	    tar[j] = translateCharUTF8(STRING_ELT(target, j));
    } else {
	for(i = 0; i < n_input; i++) {
	    in[i] = translateChar(STRING_ELT(input, i));
	    ians[i] = 0;
	}
	for(j = 0; j < n_target; j++)
	    tar[j] = translateChar(STRING_ELT(target, j));
    }
    /* First pass, exact matching */
    if(no_dups) {
	for (i = 0; i < n_input; i++) {
	    const char *ss = in[i];
	    if (strlen(ss) == 0) continue;
	    for (j = 0; j < n_target; j++) {
		if (used[j]) continue;
		if (strcmp(ss, tar[j]) == 0) {
		    if(no_dups) used[j] = 1;
		    ians[i] = j + 1;
		    nexact++;
		    break;
		}
	    }
	}
    } else {
	/* only worth hashing if enough lookups will be done:
	   since the tradeoff involves memory as well as time
	   it is not really possible to optimize there.
	 */
	if((n_target > 100) && (10*n_input > n_target)) {
	    /* <FIXME>
	       Currently no hashing when using bytes.
	       </FIXME>
	    */
	    HashData data;
	    HashTableSetup(target, &data);
	    data.useUTF8 = useUTF8;
	    data.nomatch = 0;
	    DoHashing(target, &data);
	    for (i = 0; i < n_input; i++) {
		/* we don't want to lookup "" */
		if (strlen(in[i]) == 0) continue;
		ians[i] = Lookup(target, input, i, &data);
		if(ians[i]) nexact++;
	    }
	} else {
	    for (i = 0; i < n_input; i++) {
		const char *ss = in[i];
		if (strlen(ss) == 0) continue;
		for (j = 0; j < n_target; j++)
		    if (strcmp(ss, tar[j]) == 0) {
			ians[i] = j + 1;
			nexact++;
			break;
		    }
	    }
	}
    }

    if(nexact < n_input) {
	/* Second pass, partial matching */
	for (i = 0; i < n_input; i++) {
	    const char *ss;
	    if (ians[i]) continue;
	    ss = in[i];
	    temp = strlen(ss);
	    if (temp == 0) continue;
	    mtch = 0;
	    mtch_count = 0;
	    for (j = 0; j < n_target; j++) {
		if (no_dups && used[j]) continue;
		if (strncmp(ss, tar[j], temp) == 0) {
		    mtch = j + 1;
		    mtch_count++;
		}
	    }
	    if (mtch > 0 && mtch_count == 1) {
		if(no_dups) used[mtch - 1] = 1;
		ians[i] = mtch;
	    }
	}
	/* Third pass, set no matches */
	for (i = 0; i < n_input; i++)
	    if(ians[i] == 0) ians[i] = no_match;

    }
    UNPROTECT(1);
    return ans;
}


/* Partial Matching of Strings */
/* Based on Therneau's charmatch. */

SEXP attribute_hidden do_charmatch(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, input, target;
    Rboolean perfect;
    int i, j, k, imatch, n_input, n_target, no_match, *ians;
    size_t temp;
    const char *ss, *st;
    Rboolean useBytes = FALSE, useUTF8 = FALSE;
    const void *vmax;

    checkArity(op, args);

    input = CAR(args);
    n_input = LENGTH(input);
    target = CADR(args);
    n_target = LENGTH(target);

    if (!isString(input) || !isString(target))
	error(_("argument is not of mode character"));
    no_match = asInteger(CADDR(args));

    for(i = 0; i < n_input; i++) {
	if(IS_BYTES(STRING_ELT(input, i))) {
	    useBytes = TRUE;
	    useUTF8 = FALSE;
	    break;
	} else if(ENC_KNOWN(STRING_ELT(input, i))) {
	    useUTF8 = TRUE;
	}
    }
    if(!useBytes) {
	for(i = 0; i < n_target; i++) {
	    if(IS_BYTES(STRING_ELT(target, i))) {
		useBytes = TRUE;
		useUTF8 = FALSE;
		break;
	    } else if(ENC_KNOWN(STRING_ELT(target, i))) {
		useUTF8 = TRUE;
	    }
	}
    }

    PROTECT(ans = allocVector(INTSXP, n_input));
    ians = INTEGER(ans);

    vmax = vmaxget();
    for(i = 0; i < n_input; i++) {
	if(useBytes)
	    ss = CHAR(STRING_ELT(input, i));
	else if(useUTF8)
	    ss = translateCharUTF8(STRING_ELT(input, i));
	else
	    ss = translateChar(STRING_ELT(input, i));
	temp = strlen(ss);
	imatch = NA_INTEGER;
	perfect = FALSE;
	/* we could reset vmax here too: worth it? */
	for(j = 0; j < n_target; j++) {
	    if(useBytes)
		st = CHAR(STRING_ELT(target, j));
	    else if(useUTF8)
		st = translateCharUTF8(STRING_ELT(target, j));
	    else
		st = translateChar(STRING_ELT(target, j));
	    k = strncmp(ss, st, temp);
	    if (k == 0) {
		if (strlen(st) == temp) {
		    if (perfect)
			imatch = 0;
		    else {
			perfect = TRUE;
			imatch = j + 1;
		    }
		}
		else if (!perfect) {
		    if (imatch == NA_INTEGER)
			imatch = j + 1;
		    else
			imatch = 0;
		}
	    }
	}
	ians[i] = (imatch == NA_INTEGER) ? no_match : imatch;
	vmaxset(vmax);
    }
    UNPROTECT(1);
    return ans;
}


/* Functions for matching the supplied arguments to the */
/* formal arguments of functions.  The returned value */
/* is a list with all components named. */

#define ARGUSED(x) LEVELS(x)

static SEXP StripUnmatched(SEXP s)
{
    if (s == R_NilValue) return s;

    if (CAR(s) == R_MissingArg && !ARGUSED(s) ) {
	return StripUnmatched(CDR(s));
    }
    else if (CAR(s) == R_DotsSymbol ) {
	return StripUnmatched(CDR(s));
    }
    else {
	SETCDR(s, StripUnmatched(CDR(s)));
	return s;
    }
}

static SEXP ExpandDots(SEXP s, int expdots)
{
    SEXP r;
    if (s == R_NilValue)
	return s;
    if (TYPEOF(CAR(s)) == DOTSXP ) {
	SET_TYPEOF(CAR(s), LISTSXP);	/* a safe mutation */
	if (expdots) {
	    r = CAR(s);
	    while (CDR(r) != R_NilValue ) {
		SET_ARGUSED(r, 1);
		r = CDR(r);
	    }
	    SET_ARGUSED(r, 1);
	    SETCDR(r, ExpandDots(CDR(s), expdots));
	    return CAR(s);
	}
    }
    else
	SET_ARGUSED(s, 0);
    SETCDR(s, ExpandDots(CDR(s), expdots));
    return s;
}
static SEXP subDots(SEXP rho)
{
    SEXP rval, dots, a, b, t;
    int len,i;
    char tbuf[10];

    dots = findVar(R_DotsSymbol, rho);

    if (dots == R_UnboundValue)
	error(_("... used in a situation where it doesn't exist"));

    if (dots == R_MissingArg)
	return dots;

    len = length(dots);
    PROTECT(rval=allocList(len));
    for(a=dots, b=rval, i=1; i<=len; a=CDR(a), b=CDR(b), i++) {
	sprintf(tbuf,"..%d",i);
	SET_TAG(b, TAG(a));
	t = CAR(a);
	while (TYPEOF(t) == PROMSXP)
	    t = PREXPR(t);
	if( isSymbol(t) || isLanguage(t) )
	    SETCAR(b, mkSYMSXP(mkChar(tbuf), R_UnboundValue));
	else
	    SETCAR(b, t);
    }
    UNPROTECT(1);
    return rval;
}


SEXP attribute_hidden do_matchcall(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP formals, actuals, rlist;
    SEXP funcall, f, b, rval, sysp, t1, t2, tail;
    RCNTXT *cptr;
    int expdots;

    checkArity(op,args);

    funcall = CADR(args);

    if (TYPEOF(funcall) == EXPRSXP)
	funcall = VECTOR_ELT(funcall, 0);

    if (TYPEOF(funcall) != LANGSXP)
	error(_("invalid '%s' argument"), "call");

    /* Get the function definition */
    sysp = R_GlobalContext->sysparent;

    if (TYPEOF(CAR(args)) == NILSXP) {
	/* Get the env that the function containing */
	/* matchcall was called from. */
	cptr = R_GlobalContext;
	while (cptr != NULL) {
	    if (cptr->callflag & CTXT_FUNCTION && cptr->cloenv == sysp)
		break;
	    cptr = cptr->nextcontext;
	}
	if ( cptr == NULL ) {
	    sysp = R_GlobalEnv;
	    errorcall(R_NilValue,
		      "match.call() was called from outside a function");
	} else
	    sysp = cptr->sysparent;
	if (cptr != NULL)
	    /* Changed to use the function from which match.call was
	       called as recorded in the context.  This change is
	       needed in case the current function is computed in a
	       way that cannot be reproduced by a second computation,
	       or if it is a registered S3 method that is not
	       lexically visible at the call site.

	       There is one particular case where this represents a
	       change from previous semantics: The definition is NULL,
	       the call is supplied explicitly, and the function in
	       the call is NOT the current function.  The new behavior
	       is to ignore the function in the call and use the
	       current function.  This is consistent with (my reading
	       of) the documentation in both R and Splus.  However,
	       the old behavior of R was consistent with the behavior
	       of Splus (and inconsistent with the documentation in
	       both cases).

	       The previous semantics for this case can be restored by
	       having the .Internal receive an additional argument
	       that indicates whether the call was supplied explicitly
	       or missing, and using the function recorded in the
	       context only if the call was not supplied explicitly.
	       The documentation should also be changed to be
	       consistent with this behavior.  LT */
	    PROTECT(b = duplicate(cptr->callfun));
	else if ( TYPEOF(CAR(funcall)) == SYMSXP )
	    PROTECT(b = findFun(CAR(funcall), sysp));
	else
	    PROTECT(b = eval(CAR(funcall), sysp));

	if (TYPEOF(b) != CLOSXP)
	    error(_("unable to find a closure from within which 'match.call' was called"));

    }
    else {
	/* It must be a closure! */
	PROTECT(b = CAR(args));
	if (TYPEOF(b) != CLOSXP)
	    error(_("invalid '%s' argument"), "definition");
    }

    /* Do we expand ... ? */

    expdots = asLogical(CAR(CDDR(args)));
    if (expdots == NA_LOGICAL)
	error(_("invalid '%s' argument"), "expand.dots");

    /* Get the formals and match the actual args */

    formals = FORMALS(b);
    PROTECT(actuals = duplicate(CDR(funcall)));

    /* If there is a ... symbol then expand it out in the sysp env
       We need to take some care since the ... might be in the middle
       of the actuals  */

    t2 = R_MissingArg;
    for (t1=actuals ; t1!=R_NilValue ; t1 = CDR(t1) ) {
	if (CAR(t1) == R_DotsSymbol) {
		t2 = subDots(sysp);
		break;
	}
    }
    /* now to splice t2 into the correct spot in actuals */
    if (t2 != R_MissingArg ) {	/* so we did something above */
	if( CAR(actuals) == R_DotsSymbol ) {
	    UNPROTECT(1);
	    actuals = listAppend(t2, CDR(actuals));
	    PROTECT(actuals);
	}
	else {
	    for(t1=actuals; t1!=R_NilValue; t1=CDR(t1)) {
		if( CADR(t1) == R_DotsSymbol ) {
		    tail = CDDR(t1);
		    SETCDR(t1, t2);
		    listAppend(actuals,tail);
		    break;
		}
	    }
	}
    } else { /* get rid of it */
	if( CAR(actuals) == R_DotsSymbol ) {
	    UNPROTECT(1);
	    actuals = CDR(actuals);
	    PROTECT(actuals);
	}
	else {
	    for(t1=actuals; t1!=R_NilValue; t1=CDR(t1)) {
		if( CADR(t1) == R_DotsSymbol ) {
		    tail = CDDR(t1);
		    SETCDR(t1, tail);
		    break;
		}
	    }
	}
    }
    rlist = matchArgs(formals, actuals, call);

    /* Attach the argument names as tags */

    for (f = formals, b = rlist; b != R_NilValue; b = CDR(b), f = CDR(f)) {
	SET_TAG(b, TAG(f));
    }


    /* Handle the dots */

    PROTECT(rlist = ExpandDots(rlist, expdots));

    /* Eliminate any unmatched formals and any that match R_DotSymbol */
    /* This needs to be after ExpandDots as the DOTSXP might match ... */

    rlist = StripUnmatched(rlist);

    PROTECT(rval = allocSExp(LANGSXP));
    SETCAR(rval, duplicate(CAR(funcall)));
    SETCDR(rval, rlist);
    UNPROTECT(4);
    return rval;
}


#include <string.h>
#ifdef _AIX  /*some people just have to be different */
#    include <memory.h>
#endif
/* int and double zeros are all bits off */
#define ZEROINT(X,N,I) do{memset(INTEGER(X),0,(size_t)(N)*sizeof(int));}while(0)
#define ZERODBL(X,N,I) do{memset(REAL(X),0,(size_t)(N)*sizeof(double));}while(0)

SEXP attribute_hidden
Rrowsum_matrix(SEXP x, SEXP ncol, SEXP g, SEXP uniqueg, SEXP snarm)
{
    SEXP matches,ans;
    int i, j, n, p, ng = 0, offset, offsetg, narm;
    HashData data;
    data.nomatch = 0;

    n = LENGTH(g);
    p = INTEGER(ncol)[0];
    ng = length(uniqueg);
    narm = asLogical(snarm);
    if(narm == NA_LOGICAL) error("'na.rm' must be TRUE or FALSE");

    HashTableSetup(uniqueg, &data);
    PROTECT(data.HashTable);
    DoHashing(uniqueg, &data);
    PROTECT(matches = HashLookup(uniqueg, g, &data));

    PROTECT(ans = allocMatrix(TYPEOF(x), ng, p));

    offset = 0; offsetg = 0;

    switch(TYPEOF(x)){
    case REALSXP:
	ZERODBL(ans, ng*p, i);
	for(i = 0; i < p; i++) {
	    for(j = 0; j < n; j++)
		if(!narm || !ISNAN(REAL(x)[j+offset]))
		    REAL(ans)[INTEGER(matches)[j]-1+offsetg]
			+= REAL(x)[j+offset];
	    offset += n;
	    offsetg += ng;
	}
	break;
    case INTSXP:
	ZEROINT(ans, ng*p, i);
	for(i = 0; i < p; i++) {
	    for(j = 0; j < n; j++) {
		if (INTEGER(x)[j+offset] == NA_INTEGER) {
		    if(!narm)
			INTEGER(ans)[INTEGER(matches)[j]-1+offsetg]
			    = NA_INTEGER;
		} else if (INTEGER(ans)[INTEGER(matches)[j]-1+offsetg]
			   != NA_INTEGER) {
		    /* check for integer overflows */
		    int itmp = INTEGER(ans)[INTEGER(matches)[j]-1+offsetg];
		    double dtmp = itmp;
		    dtmp += INTEGER(x)[j+offset];
		    if (dtmp < INT_MIN || dtmp > INT_MAX) itmp = NA_INTEGER;
		    else itmp += INTEGER(x)[j+offset];
		    INTEGER(ans)[INTEGER(matches)[j]-1+offsetg] = itmp;
		}
	    }
	    offset += n;
	    offsetg += ng;
	}
	break;
    default:
	error(_("non-numeric matrix in rowsum(): this cannot happen"));
    }

    UNPROTECT(2); /*HashTable, matches*/
    UNPROTECT(1); /*ans*/
    return ans;
}

SEXP attribute_hidden
Rrowsum_df(SEXP x, SEXP ncol, SEXP g, SEXP uniqueg, SEXP snarm)
{
    SEXP matches,ans,col,xcol;
    int i, j, n, p, ng = 0, narm;
    HashData data;
    data.nomatch = 0;

    n = LENGTH(g);
    p = INTEGER(ncol)[0];
    ng = length(uniqueg);
    narm = asLogical(snarm);
    if(narm == NA_LOGICAL) error("'na.rm' must be TRUE or FALSE");

    HashTableSetup(uniqueg, &data);
    PROTECT(data.HashTable);
    DoHashing(uniqueg, &data);
    PROTECT(matches = HashLookup(uniqueg, g, &data));

    PROTECT(ans = allocVector(VECSXP, p));

    for(i = 0; i < p; i++) {
	xcol = VECTOR_ELT(x,i);
	if (!isNumeric(xcol))
	    error(_("non-numeric data frame in rowsum"));
	switch(TYPEOF(xcol)){
	case REALSXP:
	    PROTECT(col = allocVector(REALSXP,ng));
	    ZERODBL(col, ng, i);
	    for(j = 0; j < n; j++)
		if(!narm || !ISNAN(REAL(xcol)[j]))
		    REAL(col)[INTEGER(matches)[j]-1] += REAL(xcol)[j];
	    SET_VECTOR_ELT(ans,i,col);
	    UNPROTECT(1);
	    break;
	case INTSXP:
	    PROTECT(col = allocVector(INTSXP, ng));
	    ZEROINT(col, ng, i);
	    for(j = 0; j < n; j++) {
		if (INTEGER(xcol)[j] == NA_INTEGER) {
		    if(!narm)
			INTEGER(col)[INTEGER(matches)[j]-1] = NA_INTEGER;
		} else if (INTEGER(col)[INTEGER(matches)[j]-1] != NA_INTEGER) {
		    int itmp = INTEGER(col)[INTEGER(matches)[j]-1];
		    double dtmp = itmp;
		    dtmp += INTEGER(xcol)[j];
		    if (dtmp < INT_MIN || dtmp > INT_MAX) itmp = NA_INTEGER;
		    else itmp += INTEGER(xcol)[j];
		    INTEGER(col)[INTEGER(matches)[j]-1] = itmp;
		}
	    }
	    SET_VECTOR_ELT(ans, i, col);
	    UNPROTECT(1);
	    break;

	default:
	    error(_("this cannot happen"));
	}
    }
    namesgets(ans, getAttrib(x, R_NamesSymbol));

    UNPROTECT(2); /*HashTable, matches*/
    UNPROTECT(1); /*ans*/
    return ans;
}

/* returns 1-based duplicate no */
static int isDuplicated2(SEXP x, int indx, HashData *d)
{
    int i, *h;

    h = INTEGER(d->HashTable);
    i = d->hash(x, indx, d);
    while (h[i] != NIL) {
	if (d->equal(x, h[i], x, indx))
	    return h[i] + 1;
	i = (i + 1) % d->M;
    }
    h[i] = indx;
    return 0;
}

static SEXP duplicated2(SEXP x, HashData *d)
{
    SEXP ans;
    int *h, *v;
    int i, n;

    n = LENGTH(x);
    HashTableSetup(x, d);
    PROTECT(d->HashTable);
    PROTECT(ans = allocVector(INTSXP, n));

    h = INTEGER(d->HashTable);
    v = INTEGER(ans);
    for (i = 0; i < d->M; i++) h[i] = NIL;
    for (i = 0; i < n; i++) v[i] = isDuplicated2(x, i, d);
    UNPROTECT(2);
    return ans;
}

SEXP attribute_hidden do_makeunique(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP names, sep, ans, dup, newx;
    int i, cnt, *cnts, dp;
    R_len_t n, len, maxlen = 0;
    HashData data;
    const char *csep, *ss;
    void *vmax;

    checkArity(op, args);
    names = CAR(args);
    if(!isString(names))
	error(_("'names' must be a character vector"));
    n = LENGTH(names);
    sep = CADR(args);
    if(!isString(sep) || LENGTH(sep) != 1)
	error(_("'sep' must be a character string"));
    csep = translateChar(STRING_ELT(sep, 0));
    PROTECT(ans = allocVector(STRSXP, n));
    vmax = vmaxget();
    for(i = 0; i < n; i++) {
	SET_STRING_ELT(ans, i, STRING_ELT(names, i));
	len = (R_len_t) strlen(translateChar(STRING_ELT(names, i)));
	if(len > maxlen) maxlen = len;
	vmaxset(vmax);
    }
    if(n > 1) {
	/* +2 for terminator and rounding error */
	char buf[maxlen + (R_len_t) strlen(csep)
		 + (R_len_t) (log((double)n)/log(10.0)) + 2];
	if(n < 10000) {
	    cnts = (int *) alloca(((size_t) n) * sizeof(int));
	} else {
	    /* This is going to be slow so use expensive allocation
	       that will be recovered if interrupted. */
	    cnts = (int *) R_alloc((size_t) n,  sizeof(int));
	}
	R_CheckStack();
	for(i = 0; i < n; i++) cnts[i] = 1;
	data.nomatch = 0;
	PROTECT(newx = allocVector(STRSXP, 1));
	PROTECT(dup = duplicated2(names, &data));
	PROTECT(data.HashTable);
	vmax = vmaxget();
	for(i = 1; i < n; i++) { /* first cannot be a duplicate */
	    dp = INTEGER(dup)[i]; /* 1-based number of first occurrence */
	    if(dp == 0) continue;
	    ss = translateChar(STRING_ELT(names, i));
	    /* Try appending 1,2,3, ..., n-1 until it is not already in use */
	    for(cnt = cnts[dp-1]; cnt < n; cnt++) {
		sprintf(buf, "%s%s%d", ss, csep, cnt);
		SET_STRING_ELT(newx, 0, mkChar(buf));
		if(Lookup(ans, newx, 0, &data) == data.nomatch) break;
	    }
	    SET_STRING_ELT(ans, i, STRING_ELT(newx, 0));
	    /* insert it */ (void) isDuplicated(ans, i, &data);
	    cnts[dp-1] = cnt+1; /* cache the first unused cnt value */
	    vmaxset(vmax);
	}
	UNPROTECT(3);
    }
    UNPROTECT(1);
    return ans;
}

/* Use hashing to improve object.size. Here we want equal CHARSXPs,
   not equal contents. */

static int csequal(SEXP x, int i, SEXP y, int j)
{
    return STRING_ELT(x, i) == STRING_ELT(y, j);
}

static void HashTableSetup1(SEXP x, HashData *d)
{
    d->hash = cshash;
    d->equal = csequal;
    MKsetup(LENGTH(x), d);
    d->HashTable = allocVector(INTSXP, d->M);
}

SEXP attribute_hidden csduplicated(SEXP x)
{
    SEXP ans;
    int *h, *v;
    int i, n;
    HashData data;

    if(TYPEOF(x) != STRSXP)
	error(_("csduplicated not called on a STRSXP"));
    n = LENGTH(x);
    HashTableSetup1(x, &data);
    PROTECT(data.HashTable);
    PROTECT(ans = allocVector(LGLSXP, n));

    h = INTEGER(data.HashTable);
    v = LOGICAL(ans);

    for (i = 0; i < data.M; i++)
	h[i] = NIL;

    for (i = 0; i < n; i++)
	v[i] = isDuplicated(x, i, &data);

    UNPROTECT(2);
    return ans;
}
