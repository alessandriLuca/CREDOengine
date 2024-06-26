#  File src/library/stats/R/aggregate.R
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

aggregate <-
function(x, ...)
    UseMethod("aggregate")

aggregate.default <-
function(x, ...)
{
    if(is.ts(x))
        aggregate.ts(as.ts(x), ...)
    else
        aggregate.data.frame(as.data.frame(x), ...)
}

aggregate.data.frame <-
function(x, by, FUN, ..., simplify = TRUE)
{
    if(!is.data.frame(x)) x <- as.data.frame(x)
    ## Do this here to avoid masking by non-function (could happen)
    FUN <- match.fun(FUN)
    if(NROW(x) == 0L) stop("no rows to aggregate")
    if(NCOL(x) == 0L) {
        ## fake it
        x <- data.frame(x = rep(1, NROW(x)))
        return(aggregate.data.frame(x, by, function(x) 0L)[seq_along(by)])
    }
    if(!is.list(by))
        stop("'by' must be a list")
    if(is.null(names(by)))
        names(by) <- paste("Group", seq_along(by), sep = ".")
    else {
        nam <- names(by)
        ind <- which(!nzchar(nam))
        names(by)[ind] <- paste("Group", ind, sep = ".")
    }

    ## Could also use interaction(drop = TRUE) ...
    nrx <- NROW(x)
    grp <- integer(nrx)
    for(ind in rev(by)) {
        if(length(ind) != nrx)
            stop("arguments must have same length")
        ind <- as.factor(ind)
        grp <- grp * nlevels(ind) + (as.integer(ind) - 1L)
    }

    y <- as.data.frame(by, stringsAsFactors = FALSE)
    y <- y[match(sort(unique(grp)), grp, 0L), , drop = FALSE]
    nry <- NROW(y)
    z <- lapply(x,
                function(e) {
                    ## In case of a common length > 1, sapply() gives
                    ## the transpose of what we need ...
                    ans <- lapply(split(e, grp), FUN, ...)
                    if(simplify &&
                       length(len <- unique(sapply(ans, length))) == 1L) {
                        if(len == 1L)
                            ans <- unlist(ans, recursive = FALSE)
                        else if(len > 1L)
                            ans <- matrix(unlist(ans,
                                                 recursive = FALSE),
                                          nrow = nry,
                                          ncol = len,
                                          byrow = TRUE,
                                          dimnames = {
                                              if(!is.null(nms <-
                                                          names(ans[[1L]])))
                                                  list(NULL, nms)
                                              else
                                                  NULL
                                          })
                    }
                    ans
                })
    len <- length(y)
    for(i in seq_along(z))
        y[[len + i]] <- z[[i]]
    names(y) <- c(names(by), names(x))
    row.names(y) <- NULL

    y
}

aggregate.formula <-
function(formula, data, FUN, ..., subset, na.action = na.omit)
{
    if(missing(formula) || !inherits(formula, "formula"))
        stop("'formula' missing or incorrect")
    if(length(formula) != 3L)
        stop("'formula' must have both left and right hand sides")

    m <- match.call(expand.dots = FALSE)
    if(is.matrix(eval(m$data, parent.frame())))
        m$data <- as.data.frame(data)
    m$... <- m$FUN <- NULL
    m[[1L]] <- as.name("model.frame")

    if(as.character(formula[[2L]] == ".")) {
        ## LHS is a dot, expand it ...
        rhs <- unlist(strsplit(deparse(formula[[3L]]), " *[:+] *"))
        ## <NOTE>
        ## Note that this will not do quite the right thing in case the
        ## RHS contains transformed variables, such that
        ##   setdiff(rhs, names(data))
        ## is non-empty ...
        lhs <- sprintf("cbind(%s)",
                       paste(setdiff(names(data), rhs), collapse = ","))
        ## </NOTE>
        m[[2L]][[2L]] <- parse(text = lhs)[[1L]]
    }
    mf <- eval(m, parent.frame())

    if(is.matrix(mf[[1L]])) {
        ## LHS is a cbind() combo, convert to data frame and fix names.
        lhs <- as.data.frame(mf[[1L]])
        names(lhs) <- as.character(m[[2L]][[2L]])[-1L]
        aggregate.data.frame(lhs, mf[-1L], FUN = FUN, ...)
    }
    else
        aggregate.data.frame(mf[1L], mf[-1L], FUN = FUN, ...)
}

aggregate.ts <-
function(x, nfrequency = 1, FUN = sum, ndeltat = 1,
         ts.eps = getOption("ts.eps"), ...)
{
    x <- as.ts(x)
    ofrequency <- tsp(x)[3L]
    ## do this here to avoid masking by non-function (could happen)
    FUN <- match.fun(FUN)
    ## Set up the new frequency, and make sure it is an integer.
    if(missing(nfrequency))
        nfrequency <- 1 / ndeltat
    if((nfrequency > 1) &&
        (abs(nfrequency - round(nfrequency)) < ts.eps))
        nfrequency <- round(nfrequency)

    if(nfrequency == ofrequency)
        return(x)
    ratio <- ofrequency /nfrequency
    if(abs(ratio - round(ratio)) > ts.eps)
        stop(gettextf("cannot change frequency from %g to %g",
                      ofrequency, nfrequency), domain = NA)
    ## The desired result is obtained by applying FUN to blocks of
    ## length ofrequency/nfrequency, for each of the variables in x.
    ## We first get the new start and end right, and then break x into
    ## such blocks by reshaping it into an array and setting dim.
    ## avoid e.g. 1.0 %/% 0.2
    ## https://stat.ethz.ch/pipermail/r-devel/2010-April/057225.html
    len <- trunc((ofrequency / nfrequency) + ts.eps)
    mat <- is.matrix(x)
    if(mat) cn <- colnames(x)
    ##   nstart <- ceiling(tsp(x)[1L] * nfrequency) / nfrequency
    ##   x <- as.matrix(window(x, start = nstart))
    nstart <- tsp(x)[1L]
    ## Can't use nstart <- start(x) as this causes problems if
    ## you get a vector of length 2.
    x <- as.matrix(x)
    nend <- floor(nrow(x) / len) * len
    x <- apply(array(c(x[1 : nend, ]),
                     dim = c(len, nend / len, ncol(x))),
               MARGIN = c(2L, 3L), FUN = FUN, ...)
    if(!mat) x <- as.vector(x)
    else colnames(x) <- cn
    ts(x, start = nstart, frequency = nfrequency)
}

