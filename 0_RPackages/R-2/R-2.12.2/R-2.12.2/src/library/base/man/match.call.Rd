% File src/library/base/man/match.call.Rd
% Part of the R package, http://www.R-project.org
% Copyright 1995-2009 R Core Development Team
% Distributed under GPL 2 or later

\name{match.call}
\title{Argument Matching}
\usage{
match.call(definition = NULL, call = sys.call(sys.parent()),
           expand.dots = TRUE)
}
\alias{match.call}
\arguments{
  \item{definition}{a function, by default the function from which
    \code{match.call} is called.  See details.}
  \item{call}{an unevaluated call to the function specified by
    \code{definition}, as generated by \code{\link{call}}.}
  \item{expand.dots}{logical. Should arguments matching \code{\dots}
    in the call be included or left as a \code{\dots} argument?}
}
\description{
  \code{match.call} returns a call in which all of the specified arguments are
  specified by their full names.
}
\details{
  \sQuote{function} on this help page means an interpreted function
  (also known as a \sQuote{closure}):  \code{match.call} does not support
  primitive functions (where argument matching is normally
  positional).

  \code{match.call} is most commonly used in two circumstances:
  \itemize{
    \item To record the call for later re-use: for example most
    model-fitting functions record the call as element \code{call} of
    the list they return.  Here the default \code{expand.dots = TRUE}
    is appropriate.

    \item To pass most of the call to another function, often
    \code{model.frame}.  Here the common idiom is that
    \code{expand.dots = FALSE} is used, and the \code{\dots} element
    of the matched call is removed.  An alternative is to
    explicitly select the arguments to be passed on, as is done in
    \code{lm}.
  }

  Calling \code{match.call} outside a function without specifying
  \code{definition} is an error.
}
\value{
  An object of class \code{call}.
}
\references{
  Chambers, J. M. (1998)
  \emph{Programming with Data. A Guide to the S Language}.
  Springer.
}
\seealso{
  \code{\link{sys.call}()} is similar, but does \emph{not} expand the
  argument names;
  \code{\link{call}}, \code{\link{pmatch}}, \code{\link{match.arg}},
  \code{\link{match.fun}}.
}
\examples{
match.call(get, call("get", "abc", i = FALSE, p = 3))
## -> get(x = "abc", pos = 3, inherits = FALSE)
fun <- function(x, lower = 0, upper = 1) {
  structure((x - lower) / (upper - lower), CALL = match.call())
}
fun(4 * atan(1), u = pi)
}
\keyword{programming}
