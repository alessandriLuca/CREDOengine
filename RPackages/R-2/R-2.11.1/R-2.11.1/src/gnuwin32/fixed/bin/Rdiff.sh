##
## Rdiff -- diff 'without R version'

revision='$Rev: 51455 $'
version=`set - ${revision}; echo ${2}`
version="R output diff: ${R_VERSION} (r${version})

Copyright (C) 2000-2008 The R Core Development Team.
This is free software; see the GNU General Public License version 2
or later for copying conditions.  There is NO warranty."

usage="
Usage: R CMD Rdiff FROM-FILE TO-FILE EXITSTATUS

Diff R output files FROM-FILE and TO-FILE discarding the R startup message,
where FROM-FILE equal to '-' means stdin.

Options:
  -h, --help     print this help message and exit
  -v, --version  print version info and exit

Report bugs to <r-bugs@r-project.org>."

if test "${#}" -le 1; then
  case "${1}" in
    -h | --help) echo "${usage}"; exit 0 ;;
    -v | --version) echo "${version}"; exit 0 ;;
    *) echo "${usage}"; exit 1 ;;
  esac
fi

ffile=${1}
tfile=${2}
exitstatus=${3}

## Windows Rtools kit has grep but not egrep
EGREP=${EGREP-'grep -E'}
SED=${SED-sed}

## sed scripts to get rid of the startup message
scriptold='/^R : Copyright /,/quit R.$/{d;}'
scriptnew='/^R version /,/quit R.$/{d;}'

## turn Windows' directional single quotes into ASCII quote
s1="s/�/'/g"
s2="s/�/'/g"

## egrep pattern to get rid of some more
## <= 2.2.x was pattern='(^Number of.*:|^Time |^Loading required package.*'
pattern='(^Time |^Loading required package.*|^Package [A-Za-z][A-Za-z0-9]+ loaded'
case "${ffile}" in
  *primitive-funs*)
    pattern=${pattern}'|^\[1\] [19][0-9][0-9])'
    ;;
  *)
    pattern=${pattern}'|^<(environment|promise|pointer): )'
    ;;
esac

if test ${ffile} = '-' ; then
  ffile=''
  bfile=''
else
  if test -f ${ffile} ; then
      bfile=`basename "${ffile}"`
  else
      echo "no valid file ${ffile}"
      exit 1
  fi
fi

tmpfile=${TMPDIR-/tmp}/${bfile}-d.${$}

${SED} -e "${scriptold}" -e "${scriptnew}" -e "${s1}" -e "${s2}" ${ffile} | ${EGREP} -v "${pattern}" > ${tmpfile}
## some packages ship .Rout.save with CRLF endings
(tr -d '\r' <  ${tfile} | ${SED} -e "${scriptold}" -e "${scriptnew}" -e "${s1}" -e "${s2}" | \
  ${EGREP} -v "${pattern}" | \
  diff -bw ${tmpfile} - ) && exitstatus=0

rm -f ${tmpfile}

exit ${exitstatus}
