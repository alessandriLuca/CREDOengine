#  File src/library/stats/R/nlminb.R
#  Part of the R package, http://www.R-project.org
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  A copy of the GNU General Public License is available at
#  http://www.r-project.org/Licenses/

nlminb <-
    function(start, objective, gradient = NULL, hessian = NULL, ...,
             scale = 1, control = list(), lower =  - Inf, upper = Inf)
{
    ## Establish the working vectors and check and set options
    par <- as.double(start)
    names(par) <- names(start)
    n <- length(par)
    iv <- integer(78 + 3 * n)
    v <- double(130 + (n * (n + 27)) / 2)
    .Call(R_port_ivset, 2, iv, v)
    if (length(control)) {
        nms <- names(control)
        if (!is.list(control) || is.null(nms))
            stop("control argument must be a named list")
        cpos <- c(eval.max = 17, iter.max = 18, trace = 19, abs.tol = 31,
                  rel.tol = 32, x.tol = 33, step.min = 34, step.max = 35,
                  scale.init = 38, sing.tol = 37, diff.g = 42)
        pos <- pmatch(nms, names(cpos))
        if (any(nap <- is.na(pos))) {
            warning(paste("unrecognized control element(s) named `",
                          paste(nms[nap], collapse = ", "),
                          "' ignored", sep = ""))
            pos <- pos[!nap]
            control <- control[!nap]
        }
        ivpars <- pos < 4
        if (any(ivpars))
            iv[cpos[pos[ivpars]]] <- as.integer(unlist(control[ivpars]))
        if (any(!ivpars))
            v[cpos[pos[!ivpars]]] <- as.double(unlist(control[!ivpars]))
    }

    ## Establish the objective function and its environment
    obj <- quote(objective(.par, ...))
    rho <- new.env(parent = environment())
    assign(".par", par, envir = rho)

    ## Create values of other arguments if needed
    grad <- hess <- low <- upp <- NULL
    if (!is.null(gradient)) {
        grad <- quote(gradient(.par, ...))
        if (!is.null(hessian)) {
            if (is.logical(hessian))
                stop("Logical `hessian' argument not allowed.  See documentation.")
            hess <- quote(hessian(.par, ...))
        }
    }
    if (any(lower != -Inf) || any(upper != Inf)) {
        low <- rep(as.double(lower), length.out = length(par))
        upp <- rep(as.double(upper), length.out = length(par))
    } else low <- upp <- numeric(0L)

    ## Do the optimization
    .Call(R_port_nlminb, obj, grad, hess, rho, low, upp,
          d = rep(as.double(scale), length.out = length(par)), iv, v)

    ans <- list(par = get(".par", envir = rho))
    ans$objective <- v[10L]
    ans$convergence <- as.integer(if (iv[1L] %in% 3L:6L) 0L else 1L)

    ans$message <- if (19 <= iv[1L] && iv[1L] <= 43) {
	if(any(B <- iv[1L] == cpos))
	    sprintf("'control' component '%s' = %g, is out of range",
		    names(cpos)[B], v[iv[1L]])
	else
	    sprintf("V[IV[1]] = V[%d] = %g is out of range (see PORT docu.)",
		    iv[1L], v[iv[1L]])
    }
    else
	switch(as.character(iv[1L]),
               "3" = "X-convergence (3)",
               "4" = "relative convergence (4)",
               "5" = "both X-convergence and relative convergence (5)",
               "6" = "absolute function convergence (6)",

               "7" = "singular convergence (7)",
               "8" = "false convergence (8)",
               "9" = "function evaluation limit reached without convergence (9)",
               "10" = "iteration limit reached without convergence (9)",
               "14" = "storage has been allocated (?) (14)",

               "15" = "LIV too small (15)",
               "16" = "LV too small (16)",
               "63" = "fn cannot be computed at initial par (63)",
               "65" = "gr cannot be computed at initial par (65)")
    if (is.null(ans$message))
        ans$message <-
            paste("See PORT documentation.  Code (", iv[1L], ")", sep = "")
    ans$iterations <- iv[31L]
    ans$evaluations <- c("function" = iv[6L], gradient = iv[30L])
    ans
}
