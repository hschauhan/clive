# Process this file with autoconf to produce a configure script.
AC_INIT([clive], [0.4.10])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_SRCDIR([include/lj.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h netdb.h string.h netinet/in.h sys/wait.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gethostbyname memset socket strftime asprintf vasprintf],, AC_MSG_WARN(missing essential library function!))
AC_SEARCH_LIBS(getopt_long, iberty gnugetopt "gnugetopt -L/usr/local/lib",, AC_MSG_WARN(missing essential library function!))
AC_SEARCH_LIBS(iconv, iconv "iconv -L/usr/local/lib", AC_DEFINE(HAVE_ICONV,, [Use character set conversion]))
AC_SEARCH_LIBS(MD5Data, md,,
	AC_MSG_WARN(Using my own MD5 functions)
	suplib="libmd5.a")
AC_ARG_DEFAULT(debug, DEBUG, no,
	[  --enable-debug         ], [enable extra debug information])
AC_ARG_DEFAULT(DEBUG, DEBUG4, no,
	[  --enable-DEBUG         ], [enable spammy debug information])

case "$LIBS" in
	*/local/*)
		 CFLAGS="$CFLAGS -I/usr/local/include"
		 ;;
esac
AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile include/Makefile
	 man/Makefile])
AC_SUBST(suplib)
AC_OUTPUT
