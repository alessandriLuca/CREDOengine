% File src/library/base/man/Arithmetic.Rd
% Part of the R package, http://www.R-project.org
% Copyright 1995-2011 R Core Development Team
% Distributed under GPL 2 or later

\name{Arithmetic}
\title{Arithmetic Operators}
\usage{
x + y
x - y
x * y
x / y
x ^ y
x \%\% y
x \%/\% y
}
\alias{+}
\alias{-}
\alias{*}
\alias{**}
\alias{/}
\alias{^}
\alias{\%\%}
\alias{\%/\%}
\alias{Arithmetic}
\concept{remainder}
\concept{modulo}
\concept{modulus}
\concept{quotient}
\concept{division}
\description{
  These binary operators perform arithmetic on numeric or complex vectors
  (or objects which can be coerced to them).
}
\arguments{
  \item{x, y}{numeric or complex vectors or objects which can be
    coerced to such, or other objects for which methods have been written.}
}
\value{
  These operators return vectors containing the result of the element
  by element operations.  The elements of shorter vectors are recycled
  as necessary (with a \code{\link{warning}} when they are recycled only
  \emph{fractionally}).  The operators are \code{+} for addition,
  \code{-} for subtraction, \code{*} for multiplication, \code{/} for
  division and \code{^} for exponentiation.

  \code{\%\%} indicates \code{x mod y} and \code{\%/\%} indicates
  integer division.  It is guaranteed that \code{x == (x \%\% y) + y * ( x
    \%/\% y )} (up to rounding error) unless \code{y == 0} where the
  result is \code{\link{NA_integer_}} or \code{\link{NaN}} (depending on
  the \code{\link{typeof}} of the arguments).  See
  \url{http://en.wikipedia.org/wiki/Modulo_operation} for the
  rationale.

  If either argument is complex the result will be complex, otherwise if
  one or both arguments are numeric, the result will be numeric.  If
  both arguments are of type \link{integer}, the type of the result of
  \code{/} and \code{^} is \link{numeric} and for the other operators it
  is integer (with overflow, which occurs at
  \eqn{\pm(2^{31} - 1)}{+/- (2^31 - 1)},
  returned as \code{NA_integer_} with a warning).

  The rules for determining the attributes of the result are rather
  complicated.  Most attributes are taken from the longer argument, the
  first if they are of the same length.  Names will be copied from the
  first if it is the same length as the answer, otherwise from the
  second if that is.  For time series, these operations are allowed only
  if the series are compatible, when the class and \code{\link{tsp}}
  attribute of whichever is a time series (the same, if both are) are
  used.  For arrays (and an array result) the dimensions and dimnames
  are taken from first argument if it is an array, otherwise the second.
}
\details{
  The binary arithmetic operators are generic functions: methods can be
  written for them individually or via the
  \code{\link[=S3groupGeneric]{Ops}} group generic function.  (See
  \code{\link[=S3groupGeneric]{Ops}} for how dispatch is computed.)

  If applied to arrays the result will be an array if this is sensible
  (for example it will not if the recycling rule has been invoked).

  Logical vectors will be coerced to integer or numeric vectors,
  \code{FALSE} having value zero and \code{TRUE} having value one.

  \code{1 ^ y} and \code{y ^ 0} are \code{1}, \emph{always}.
  \code{x ^ y} should also give the proper limit result when
  either argument is infinite (i.e., \code{+- \link{Inf}}).

  Objects such as arrays or time-series can be operated on this
  way provided they are conformable.

  For real arguments, \code{\%\%} can be subject to catastrophic loss of
  accuracy if \code{x} is much larger than \code{y}, and a warning is
  given if this is detected.

  \code{\%\%} and \code{x \%/\% y} can be used for non-integer \code{y},
  e.g. \code{1 \%/\% 0.2}, but the results are subject to representation
  error and so may be platform-dependent.  Because the IEC 60059
  representation of \code{0.2} is a binary fraction slightly larger than
  \code{0.2}, the answer to \code{1 \%/\% 0.2} should be \code{4} but
  most platforms give \code{5}.
  
  Users are sometimes surprised by the value returned, for example why
  \code{(-8)^(1/3)} is \code{NaN}.  For \link{double} inputs, \R makes
  use of IEC 60559 arithmetic on all platforms, together with the C
  system function \samp{pow} for the \code{^} operator.  The relevant
  standards define the result in many corner cases.  In particular, the
  result in the example above is mandated by the C99 standard.  On many
  Unix-alike systems the command \command{man pow} gives details of the
  values in a large number of corner cases.

  Arithmetic on type \link{double} in \R is supposed to be done in
  \sQuote{round to nearest, ties to even} mode, but this does depend on
  the compiler and FPU being set up correctly. 
}
\section{S4 methods}{
  These operators are members of the S4 \code{\link{Arith}} group generic,
  and so methods can be written for them individually as well as for the
  group generic (or the \code{Ops} group generic), with arguments
  \code{c(e1, e2)}.
}
\note{
  \code{**} is translated in the parser to \code{^}, but this was
  undocumented for many years.  It appears as an index entry in Becker
  \emph{et al} (1988), pointing to the help for \code{Deprecated} but
  is not actually mentioned on that page.  Even though it had been
  deprecated in S for 20 years, it was still accepted in \R in 2008.
}
\references{
  Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988)
  \emph{The New S Language}.
  Wadsworth & Brooks/Cole.

  D. Goldberg (1991) \emph{What Every Computer Scientist Should Know
    about Floating-Point Arithmetic}  ACM Computing Surveys, \bold{23(1)}.\cr
  Postscript version available at
  \url{http://www.validlab.com/goldberg/paper.ps}
  Extended PDF version at \url{http://www.validlab.com/goldberg/paper.pdf}

}
\seealso{
  \code{\link{sqrt}} for miscellaneous and \code{\link{Special}} for special
  mathematical functions.

  \code{\link{Syntax}} for operator precedence.

  \code{\link{\%*\%}} for matrix multiplication.
}
\examples{
x <- -1:12
x + 1
2 * x + 3
x \%\% 2 #-- is periodic
x \%/\% 5
}
\keyword{arith}
