## Copyright (C) 2002-2005 R Development Core Team
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## General Public License for more details.
##
## A copy of the GNU General Public License is available at
## http://www.r-project.org/Licenses/

## Send any bug reports to r-bugs@r-project.org.

package R::Vars;

=head1 NAME

  R::Vars - Platform-Specific Variables

=head1 SYNOPSIS

  use R::Vars;

  if($R::Vars::OSTYPE eq "windows"){
      ## do windows-specific things
  }
  else {
      ## do things on  unix
  }

  ## stop if no valid command for R itself or the 'R CMD' mechanism is
  ## available
  R::Vars::error("R_CMD", "EXE");


=head1 DESCRIPTION

    This package provides variables that help to handle
    platform-specific differences of R, some of these can be set using
    environment variables, these are listed in square brackates.

    OSTYPE     "unix" or "windows"
    TMPDIR     name of directory for temporary files [TMPDIR]

    R_HOME     path to R installation top directory [R_HOME]
    R_EXE      name of the R executable including path
    R_CMD      string for 'R CMD' including path

    MAKE       command string for 'make' [MAKE]
    LATEX      command string for 'latex' [R_LATEXCMD]
    PDFLATEX   command string for 'pdflatex' [R_PDFLATEXCMD]

    Most have sensible defaults for all platforms.

=cut

use Carp;

## perl 5.6 uses MSWin32, older versions of perl have win32 
if($^O =~ /^(MS)?Win32$/i){
    $OSTYPE = "windows";
}
else{
    $OSTYPE = "unix";
}

getenv("LATEX", "R_LATEXCMD", "latex");
getenv("MAKE", "MAKE", "make");
getenv("PDFLATEX", "R_PDFLATEXCMD", "pdflatex");
getenv("R_HOME", "R_HOME");
getenv("R_SHARE_DIR", "R_SHARE_DIR", "$R_HOME/share");

if($OSTYPE eq "windows"){
    ## DON'T add R_HOME/bin here: it might contain spaces
    $R_EXE = "Rterm.exe";
    $R_CMD = "Rcmd.exe";
    getenv("TMPDIR", "TMPDIR", "C:/TEMP");
    if (-d $TMPDIR) {
	$TMPDIR = Win32::GetShortPathName($TMPDIR) if $TMPDIR =~ / /;
	$TMPDIR =~ s+\\+/+g;  ## ensure forward slashes only
    } else {
	$TMPDIR = "" 
    }
}
else{
    if($R_HOME){
	$R_EXE = "${R_HOME}/bin/R";
    }
    else{
	$R_EXE = "R";
    }
    $R_CMD = "$R_EXE CMD";
    getenv("TMPDIR", "TMPDIR", "/tmp");
    $TMPDIR = "" unless (-d $TMPDIR);
}


## return the value of an environment variable; or the default if no
## such environment variable is set or it is empty. additionally
## record it in hash envnames (for suitable error messages below).
## The "strange" 3 argument interface allows to simultaneously set the
## variable and record which environment variable was tried to get a
## value for it.

my %envnames;
sub getenv {

    my ($var, $envvar, $default) = @_;
    if($ENV{$envvar}){
	${$var} = $ENV{$envvar};
    }
    else{
	${$var} = $default;
    }
    $envnames{$var} = $envvar;
}

## check all arguments if they are the name of a variable in this
## package and not empty, issue a warning if not.

sub warning {
    my $v;
    foreach $v (@_) {
	if(! ${"R::Vars::$v"}){
	    if($envnames{$v}){
		carp "Warning: environment variable $envnames{$v} not set " .
		    "(or set to unusable value) and no default available.\n";
	    }
	    else{
		carp "Warning: R::Vars::$v not defined";
	    }

	}
    }
}

## check all arguments if they are the name of a variable in this
## package and not empty, issue an error and stop if not.

sub error {
    my $v;
    foreach $v (@_) {
	if(! ${"R::Vars::$v"}){
	    if($envnames{$v}){
		croak "Error: environment variable $envnames{$v} not set " .
		    "(or set to unusable value) and no default available.\n";
	    }
	    else{
		croak "Error: R::Vars::$v not defined";
	    }

	}
    }
}

