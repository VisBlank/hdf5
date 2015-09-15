# ===========================================================================
#       http://www.gnu.org/software/autoconf-archive/ax_check_szip.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CHECK_SZIP([action-if-found], [action-if-not-found])
#
# DESCRIPTION
#
#   This macro searches for an installed szlib library. If nothing was
#   specified when calling configure, it searches first in /usr/local and
#   then in /usr, /opt/local and /sw. If the --with-szlib=DIR is specified,
#   it will try to find it in DIR/include/szlib.h and DIR/lib/libsz.a. If
#   --without-szlib is specified, the library is not searched at all.
#
#   If either the header file (szlib.h) or the library (libsz) is not found,
#   shell commands 'action-if-not-found' is run. If 'action-if-not-found' is
#   not specified, the configuration exits on error, asking for a valid szlib
#   installation directory or --without-szlib.
#
#   If both header file and library are found, shell commands
#   'action-if-found' is run. If 'action-if-found' is not specified, the
#   default action appends '-I${SZIP_HOME}/include' to CPFLAGS, appends
#   '-L$SZIP_HOME}/lib' to LDFLAGS, prepends '-lsz' to LIBS, and calls
#   AC_DEFINE(HAVE_SZLIB). You should use autoheader to include a definition
#   for this symbol in a config.h file. Sample usage in a C/C++ source is as
#   follows:
#
#     #ifdef HAVE_SZLIB
#     #include <szlib.h>
#     #endif /* HAVE_SZLIB */
#

#serial 14

AU_ALIAS([CHECK_SZIP], [AX_CHECK_SZIP])
AC_DEFUN([AX_CHECK_SZIP],
#
# Handle user hints
#
[AC_MSG_CHECKING(if szlib is wanted)
szip_places="/usr/local /usr /opt/local /sw"
AC_ARG_WITH([szlib],
[  --with-szlib=DIR         root directory path of szlib installation @<:@defaults to
                            /usr/local or /usr if not found in /usr/local@:>@
  --without-szlib           to disable szlib usage completely],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  if test -d "$withval"
  then
    szip_places="$withval $szip_places"
  else
    AC_MSG_WARN([Sorry, $withval does not exist, checking usual places])
  fi
else
  szip_places=
  AC_MSG_RESULT(no)
fi],
[AC_MSG_RESULT(yes)])

#
# Locate szlib, if wanted
#
if test -n "${szip_places}"
then
	# check the user supplied or any other more or less 'standard' place:
	#   Most UNIX systems      : /usr/local and /usr
	#   MacPorts / Fink on OSX : /opt/local respectively /sw
	for SZIP_HOME in ${szip_places} ; do
	  if test -f "${SZIP_HOME}/include/szlib.h"; then break; fi
	  SZIP_HOME=""
	done

  SZIP_OLD_LDFLAGS=$LDFLAGS
  SZIP_OLD_CPPFLAGS=$CPPFLAGS
  if test -n "${SZIP_HOME}"; then
        LDFLAGS="$LDFLAGS -L${SZIP_HOME}/lib"
        CPPFLAGS="$CPPFLAGS -I${SZIP_HOME}/include"
  fi
  AC_LANG_SAVE
  AC_LANG_C
  AC_CHECK_LIB([libsz], [SZ_BufftoBuffCompress], [szip_cv_libsz=yes], [szip_cv_libsz=no])
  AC_CHECK_HEADER([szlib.h], [szip_cv_szlib_h=yes], [szip_cv_szlib_h=no])
  AC_LANG_RESTORE
  if test "$szip_cv_libsz" = "yes" && test "$szip_cv_szlib_h" = "yes"
  then
    #
    # If both library and header were found, action-if-found
    #
    m4_ifblank([$1],[
                CPPFLAGS="$CPPFLAGS -I${SZIP_HOME}/include"
                LDFLAGS="$LDFLAGS -L${SZIP_HOME}/lib"
                LIBS="-lsz $LIBS"
                AC_DEFINE([HAVE_SZLIB], [1],
                          [Define to 1 if you have `szlib' library (-lsz)])
               ],[
                # Restore variables
                LDFLAGS="$SZIP_OLD_LDFLAGS"
                CPPFLAGS="$SZIP_OLD_CPPFLAGS"
                $1
               ])
  else
    #
    # If either header or library was not found, action-if-not-found
    #
    m4_default([$2],[
                AC_MSG_ERROR([either specify a valid szlib installation with --with-szlib=DIR or disable szlib usage with --without-szlib])
                ])
  fi
fi
])
