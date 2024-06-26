% File src/library/grDevices/man/getGraphicsEvent.Rd
% Part of the R package, http://www.R-project.org
% Copyright 1995-2007 R Core Team
% Distributed under GPL 2 or later

\name{getGraphicsEvent}
\alias{getGraphicsEvent}
\alias{setGraphicsEventHandlers}
\alias{getGraphicsEventEnv}
\alias{setGraphicsEventEnv}
\title{Wait for a mouse or keyboard event from a graphics window}
\description{
This function waits for input from a graphics window in the
form of a mouse or keyboard event.
}
\usage{
getGraphicsEvent(prompt = "Waiting for input", 
                 onMouseDown = NULL, onMouseMove = NULL,
                 onMouseUp = NULL, onKeybd = NULL,
                 consolePrompt = prompt)
setGraphicsEventHandlers(which = dev.cur(), ...)
getGraphicsEventEnv(which = dev.cur())
setGraphicsEventEnv(which = dev.cur(), env)

}
\arguments{
  \item{prompt}{prompt to be displayed to the user in the graphics window}
  \item{onMouseDown}{a function to respond to mouse clicks}
  \item{onMouseMove}{a function to respond to mouse movement}
  \item{onMouseUp}{a function to respond to mouse button releases}
  \item{onKeybd}{a function to respond to key presses}
  \item{consolePrompt}{prompt to be displayed to the user in the console}
  \item{which}{which graphics device does the call apply to?}
  \item{...}{items including handlers to be placed in the event environment}
  \item{env}{an environment to be used as the event environment}
}
\details{
These functions allow user input from some graphics devices (currently
only the \code{windows()} and \code{X11(type = "Xlib")} screen displays
in base \R{}). Event handlers may be installed to respond to events
involving the mouse or keyboard.

The functions are related as follows.  If any of the first five
arguments to \code{getGraphicsEvent} are given, then it uses those
in a call to \code{setGraphicsEventHandlers} to replace any existing
handlers in the current device.  This is for compatibility with pre-2.12.0 \R{}
versions.  The current normal way to set up event handlers is to 
set them using \code{setGraphicsEventHandlers} or \code{setGraphicsEventEnv} on
one or more graphics devices, and then use \code{getGraphicsEvent()} with
no arguments to retrieve event data.
\code{getGraphicsEventEnv()} may be used to save the event environment
for use later.

The names of the arguments in \code{getGraphicsEvent} are special.  When
handling events, the graphics system will look through the event 
environment for functions named \code{onMouseDown}, \code{onMouseMove}, \code{onMouseUp}
and \code{onKeybd} and use them as event handlers.  It will use 
\code{prompt} for a label on the graphics device.  Two other special names are
\code{which}, which will identify the graphics device, and
\code{result}, where the result of the last event
handler will be stored before being returned by \code{getGraphicsEvent()}.

The mouse event handlers should be functions with header 
\code{function(buttons, x, y)}.  The coordinates \code{x}
and \code{y} will be passed to mouse event handlers in device independent
coordinates (i.e. the lower left corner of the window is \code{(0,0)}, 
the upper right is \code{(1,1)}).  The \code{buttons} argument 
will be a vector listing the buttons
that are pressed at the time of the event, with 0 for left, 1 for middle, and 2 
for right.

The keyboard event handler should be a function with header
\code{function(key)}.  A single element character vector will be passed
to this handler, corresponding to the key press.  Shift and other modifier
keys will have been processed, so \code{shift-a} will be passed as
\code{"A"}.  The following special keys may also be passed to the handler:
\itemize{
     \item Control keys, passed as \code{"Ctrl-A"}, etc.
     \item Navigation keys, passed as one of\cr
     \code{"Left", "Up", "Right", "Down", "PgUp", "PgDn", "End", "Home"}
     \item Edit keys, passed as one of \code{"Ins", "Del"}
     \item Function keys, passed as one of \code{"F1", "F2", ...}
}

The event handlers are standard R functions, and will be executed as though called
from the event environment.  

In an interactive session, events will be processed until
\itemize{
     \item one of the event handlers returns
a non-\code{NULL} value which will be returned as the value of
\code{getGraphicsEvent}, or 
      \item the user interrupts the function from the
console.
}
}
\value{
When run interactively, 
\code{getGraphicsEvent} returns a non-\code{NULL} value returned from one of the event handlers.
In a non-interactive session, \code{getGraphicsEvent} will return \code{NULL} immediately.

\code{getGraphicsEventEnv} returns the current event environment for the graphics device,
or \code{NULL} if none has been set.

\code{setGraphicsEventEnv} and \code{setGraphicsEventHandlers} return the previous
event environment for the graphics device.
}
\author{Duncan Murdoch}
\examples{
\donttest{
savepar <- par(ask = FALSE)
dragplot <- function(..., xlim = NULL, ylim = NULL, xaxs = "r", yaxs = "r") {
    plot(..., xlim = xlim, ylim = ylim, xaxs = xaxs, yaxs = yaxs)
    startx <- NULL
    starty <- NULL
    usr <- NULL

    devset <- function()
        if (dev.cur() != eventEnv$which) dev.set(eventEnv$which)
        
    dragmousedown <- function(buttons, x, y) {
        startx <<- x
        starty <<- y
        devset()
        usr <<- par("usr")
        eventEnv$onMouseMove <- dragmousemove
        NULL
    }
    
    dragmousemove <- function(buttons, x, y) {
        devset()
        deltax <- diff(grconvertX(c(startx, x), "ndc", "user"))
        deltay <- diff(grconvertY(c(starty, y), "ndc", "user"))
        plot(..., xlim = usr[1:2]-deltax, xaxs = "i",
                  ylim = usr[3:4]-deltay, yaxs = "i")
        NULL
    }
    
    mouseup <- function(buttons, x, y) {    
    	eventEnv$onMouseMove <- NULL
    }	
        
    keydown <- function(key) {
        if (key == "q") return(invisible(1))
        eventEnv$onMouseMove <- NULL
        NULL
    }
    
    setGraphicsEventHandlers(prompt = "Click and drag, hit q to quit",
                     onMouseDown = dragmousedown,
                     onMouseUp = mouseup,
                     onKeybd = keydown)
    eventEnv <- getGraphicsEventEnv()
}

dragplot(rnorm(1000), rnorm(1000))
# This currently only works on the Windows
# and X11(type = "Xlib") screen devices...
getGraphicsEvent()
par(savepar)
}
}
\keyword{iplot}
