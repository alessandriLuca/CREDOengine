% File src/library/grDevices/man/unix/x11Fonts.Rd
% Part of the R package, http://www.R-project.org
% Copyright 1995-2007 R Core Development Team
% Distributed under GPL 2 or later

\name{X11Fonts}
\alias{X11Font}
\alias{X11Fonts}
\concept{fonts}
\title{X11 Fonts}
\description{
  These functions handle the translation of a device-independent R graphics
  font family name to an X11 font description.
}
\usage{
X11Font(font)

X11Fonts(...)      
}
\arguments{
  \item{font}{a character string containing an X11 font description.}
  \item{\dots}{either character strings naming mappings to display,
    or new (named) mappings to define.}
}
\details{
  These functions apply only to an \code{\link{X11}} device with
  \code{type = "Xlib"} -- \code{X11(type = "Cairo"} uses a different
  mechanism to select fonts.

  Such a  device is created with a default font (see
  the documentation for \code{\link{X11}}), but it is also possible
  to specify a font family when drawing to the device (for example, see
  the documentation for \code{"family"} in \code{\link{par}} and for
  \code{"fontfamily"} in \code{\link{gpar}} in the \pkg{grid} package).

  The font family sent to the device is a simple string name, which must be
  mapped to something more specific to X11 fonts.  A list of
  mappings is maintained and can be modified by the user.

  The \code{X11Fonts} function can be used to list existing
  mappings and to define new mappings.  The \code{X11Font}
  function can be used to create a new mapping.

  Default mappings are provided for three device-independent font family
  names: \code{"sans"} for a sans-serif font, \code{"serif"} for a serif
  font and \code{"mono"} for a monospaced font.
}
\seealso{
  \code{\link{X11}}
}
\examples{
X11Fonts()
X11Fonts("mono")
utopia <- X11Font("-*-utopia-*-*-*-*-*-*-*-*-*-*-*-*")
X11Fonts(utopia=utopia)
}
\keyword{device}
