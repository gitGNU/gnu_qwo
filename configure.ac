AC_PREREQ(2.53)
AC_INIT(qwo, 0.3, http://www.nongnu.org/qwo/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(src/qwo.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CC_STDC
AC_STDC_HEADERS

AC_CHECK_LIB(X11, XOpenDisplay)

AC_CHECK_LIB(Xext, XShapeQueryExtension)

AC_CHECK_LIB(Xtst, XTestFakeKeyEvent)

AC_CHECK_LIB(config, config_read)

dnl Check for imlib2 library

AC_PATH_PROG(IMLIB2_CONFIG, imlib2-config, no)

if test "$IMLIB2_CONFIG" = "no" ; then
    AC_MSG_ERROR([
*** Unable to find Imlib2 library.])
else
    CFLAGS="$CFLAGS `$IMLIB2_CONFIG --cflags`"
    LIBS="$LIBS `$IMLIB2_CONFIG --libs`"
fi

dnl Check for help2man

AC_CHECK_PROGS([HELP2MAN], [help2man], [NONE])

if test x"$HELP2MAN" = xNONE; then
	AC_MSG_WARN(["cannot find help2man."])
	AC_MSG_WARN(["You will not be able to generate the manpage."])
fi
AM_CONDITIONAL([HAVE_HELP2MAN], [test x"$HELP2MAN" != xNONE])
AC_SUBST([HELP2MAN])

AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
data/qwo.desktop
])

AC_OUTPUT
