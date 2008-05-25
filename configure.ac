AC_INIT(ews, 0.1)

AC_CONFIG_HEADER(include/config.h:include/config.in)

EWS_SYSCONF_DIR=$(eval eval eval eval "echo $sysconfdir")
EWS_LIB_DIR=$(eval eval eval eval "echo $libdir")
EWS_LOCAL_STATE_DIR=$(eval eval eval eval "echo $localstatedir")

if test "x${sysconfdir}" = 'x${prefix}/etc'; then
    if test "x${prefix}" = "x/usr"; then
        sysconfdir="/etc"
        EWS_SYSCONF_DIR="$sysconfdir"
    elif test "x${prefix}" = "xNONE"; then
        EWS_SYSCONF_DIR="${ac_default_prefix}/etc"
        EWS_LIB_DIR="${ac_default_prefix}/lib"
        EWS_LOCAL_STATE_DIR="${ac_default_prefix}/var"
    fi
fi

AC_DEFINE_UNQUOTED(EWS_CONFIG_DIR, "${EWS_SYSCONF_DIR}/ews", [config files placed])
AC_DEFINE_UNQUOTED(EWS_CONFIG_FILE, "${EWS_SYSCONF_DIR}/ews/method.ini, [config method file])
AC_DEFINE_UNQUOTED(EWS_MODULES_DIR, "${EWS_LIB_DIR}/ews/modules", [modules dir])
AC_DEFINE_UNQUOTED(EWS_CONSOLE_SOCKET, "${EWS_LOCAL_STATE_DIR}/run/ews/ews.ctl", [socket file])

AC_ARG_ENABLE(conf-static,
    AC_HELP_STRING([--disable-conf-static],
        [config method system static in INI files]),,
    AC_DEFINE(EWS_CONFIG_STATIC,,[config method system static in INI files]))

AC_ARG_ENABLE(stdout-verbose,
    AC_HELP_STRING([--enable-stdout-verbose],
        [verbose to stdout in daemon execution]),
    AC_DEFINE(EWS_VERBOSE_STDOUT,,[config method system static in INI files]))

AC_PROG_CC
AC_PROG_CC_STDC

AC_HEADER_STDC
AC_CHECK_HEADERS([stdarg.h time.h unistd.h sys/socket.h ctype.h rpc/rpc.h poll.h signal.h pthread.h fcntl.h errno.h netinet/in.h sys/types.h sys/select.h dlfcn.h])

AC_SUBST(dependencias_CFLAGS)
AC_SUBST(dependencias_LIBS)

AC_OUTPUT(Makefile common/Makefile main/Makefile modules/Makefile)
