AC_INIT(ews, 0.1)

AC_CONFIG_HEADER(include/config.h:include/config.in)

AC_DEFINE(EWS_CONFIG_STATIC,, [config method system static in INI files])
AC_DEFINE(EWS_CONFIG_DIR, "etc/ews", [config files placed])
AC_DEFINE(EWS_CONFIG_FILE, "etc/ews/method.ini, [config method file])
AC_DEFINE(EWS_MODULES_DIR, "modules", [modules dir])
AC_DEFINE(EWS_CONSOLE_SOCKET, "ews.ctl", [socket file])
AC_DEFINE(EWS_VERBOSE_STDOUT,, [stdout is possible?])

AC_PROG_CC
AC_PROG_CC_STDC

AC_HEADER_STDC
AC_CHECK_HEADERS([stdarg.h time.h unistd.h sys/socket.h ctype.h rpc/rpc.h poll.h signal.h pthread.h fcntl.h errno.h netinet/in.h sys/types.h sys/select.h dlfcn.h])

AC_SUBST(dependencias_CFLAGS)
AC_SUBST(dependencias_LIBS)

AC_OUTPUT(Makefile common/Makefile main/Makefile modules/Makefile)
