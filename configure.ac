AC_INIT([libusenet], [1.0])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AC_CONFIG_MACRO_DIR([m4])
AX_CXX_COMPILE_STDCXX_11
AC_ENABLE_SHARED
AC_DISABLE_STATIC
LT_INIT

dnl SSL option
AC_ARG_ENABLE([ssl],
    AS_HELP_STRING([--enable-ssl], [Enable SSL connections]))

	AS_IF([test "x$enable_ssl" == "xyes"], [
	  PKG_CHECK_MODULES(SSL, [libssl >= 1])
	  AC_DEFINE(USE_SSL, [], [Include code for SSL connections]) AC_SUBST(LIBSSL, [", libssl >= 1"])] AC_SUBST(CF_SSL, [-DUSE_SSL])
)

PTHREAD_LIBS=
dnl SSL option
AC_ARG_ENABLE([ssl-threads],
    AS_HELP_STRING([--enable-ssl-threads], [Enable threads support for SSL connections]))

	AS_IF([test "x$enable_ssl_threads" == "xyes"], [
		AX_PTHREAD([AC_DEFINE(SSL_THREADS, [], [Include code for SSL connection in threaded environment])] AC_SUBST(CF_SSL_THREADS, [-DSSL_THREADS]),
			[AC_MSG_FAILURE([--enable-ssl-threads given but libpthread not found])])
	]
)

AC_SUBST(LIBVER_INFO, [1:0:0])
PKG_CHECK_MODULES(EXPAT, [expat >= 2])
AC_CONFIG_FILES([Makefile src/Makefile libusenet.pc])
AC_OUTPUT

