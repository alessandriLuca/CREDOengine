% File src/library/base/man/zpackages.Rd
% Part of the R package, http://www.R-project.org
% Copyright 1995-2008 R Core Development Team
% Distributed under GPL 2 or later

\name{zpackages}
\title{Listing of Packages}
\alias{.packages}
\description{
  \code{.packages} returns information about package availability.
}
\usage{
.packages(all.available = FALSE, lib.loc = NULL)
}
\arguments{
  \item{all.available}{logical; if \code{TRUE} return a character vector
    of all available packages in \code{lib.loc}.}
  \item{lib.loc}{a character vector describing the location of \R
    library trees to search through, or \code{NULL}.  The default value
    of \code{NULL} corresponds to all libraries currently known.}
}
\details{
  \code{.packages()} returns the names of the currently
  attached packages \emph{invisibly} whereas
  \code{.packages(all.available = TRUE)} gives (visibly) \emph{all}
  packages available in the library location path \code{lib.loc}.
  
  For a package to be regarded as being available it must have valid
  metadata (and hence be an installed package).
}
\value{
  A character vector of package base names, invisible unless
  \code{all.available = TRUE}.
}
\author{
  R core;
  Guido Masarotto for the \code{all.available=TRUE} part of
  \code{.packages}.
}
\seealso{
  \code{\link{library}}, \code{\link{.libPaths}}.
}
\examples{
(.packages())               # maybe just "base"
.packages(all.available = TRUE) # return all available as character vector
require(splines)
(.packages())               # "splines", too
detach("package:splines")
}
\keyword{data}
