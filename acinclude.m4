dnl Make my life easier
AC_DEFUN([AC_ARG_DEFAULT], [
	AC_MSG_CHECKING([for --enable-$1])
	AC_ARG_ENABLE([$1],
		[$4 $5],
		[AC_MSG_RESULT(${enable_$1})],
		[enable_$1=$3
		 AC_MSG_RESULT($3)])
	if test ${enable_$1} = "yes" -a -n "$2" ; then AC_DEFINE($2, 1, $5) fi
	])

