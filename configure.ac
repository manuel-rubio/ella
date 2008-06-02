AC_INIT(ews, 0.1, bombadil@bosqueviejo.net)

AC_CONFIG_HEADER(include/config.h:include/config.in)

EWS_SYSCONF_DIR=$(eval eval eval eval "echo $sysconfdir")
EWS_LIB_DIR=$(eval eval eval eval "echo $libdir")
EWS_LOCAL_STATE_DIR=$(eval eval eval eval "echo $localstatedir")

if test "x${sysconfdir}" = 'x${prefix}/etc'; then
    if test "x${prefix}" = "x/usr"; then
        sysconfdir="/etc"
        localstatedir="/var"
        EWS_SYSCONF_DIR="$sysconfdir"
        EWS_LIB_DIR="${prefix}/lib"
        EWS_LOCAL_STATE_DIR="$localstatedir"
    elif test "x${prefix}" = "xNONE"; then
        EWS_SYSCONF_DIR="${ac_default_prefix}/etc"
        EWS_LIB_DIR="${ac_default_prefix}/lib"
        EWS_LOCAL_STATE_DIR="${ac_default_prefix}/var"
    fi
fi

AC_DEFINE_UNQUOTED(EWS_CONFIG_DIR, "${EWS_SYSCONF_DIR}/ews", [config files placed])
AC_DEFINE_UNQUOTED(EWS_CONFIG_FILE, "${EWS_SYSCONF_DIR}/ews/method.ini", [config method file])
AC_DEFINE_UNQUOTED(EWS_MODULES_DIR, "${EWS_LIB_DIR}/ews/modules", [modules dir])
AC_DEFINE_UNQUOTED(EWS_CONSOLE_SOCKET, "${EWS_LOCAL_STATE_DIR}/run/ews/ews.ctl", [socket file])

EWS_CONFIG_STATIC=yes
EWS_VERBOSE_STDOUT=yes

AC_ARG_ENABLE(conf-static,
    AC_HELP_STRING([--disable-conf-static],
        [config method system static in INI files]),
    EWS_CONFIG_STATIC=no,
    AC_DEFINE(EWS_CONFIG_STATIC,,[config method system static in INI files]))

AC_ARG_ENABLE(verbose-stdout,
    AC_HELP_STRING([--enable-verbose-stdout],
        [verbose to stdout in daemon execution]),
    AC_DEFINE(EWS_VERBOSE_STDOUT,,[config method system static in INI files]),
    EWS_VERBOSE_STDOUT=no )

MODULES=""

AC_ARG_WITH(dumb,
    AC_HELP_STRING([--with-dumb], [compiles dumb module]),
    MODULES="libdumb.so ${MODULES}")

AC_ARG_WITH(http10,
    AC_HELP_STRING([--without-http10], [doesn't compile http10 module]),,
    MODULES="libhttp10.so ${MODULES}")

AC_ARG_WITH(mime,
    AC_HELP_STRING([--without-mime], [doesn't compile mime module]),,
    MODULES="libmime.so ${MODULES}")

AC_PROG_CC
AC_PROG_CC_STDC

AC_HEADER_STDC
AC_CHECK_HEADERS([stdarg.h time.h unistd.h sys/socket.h ctype.h rpc/rpc.h poll.h signal.h pthread.h fcntl.h errno.h netinet/in.h sys/types.h sys/select.h dlfcn.h])

AC_CHECK_LIB(readline, readline,
    LIBS_CONSOLE="$LIBS_CONSOLE -lreadline"
    AC_DEFINE(HAVE_LIBREADLINE, 1, [have readline]),
    AC_MSG_ERROR(GNU Readline could not be found which is required)
    exit,
    $LIBS_CONSOLE
)

AC_SUBST(LIBS_CONSOLE)
AC_SUBST(LIBS_COMMON)
AC_SUBST(LIBS_SERVER)
AC_SUBST(dependencias_CFLAGS)
AC_SUBST(dependencias_LIBS)
AC_SUBST(MODULES)

AC_OUTPUT(Makefile common/Makefile main/Makefile modules/Makefile)

echo "------------------------------------------------------------------------------------"
echo "EWS_CONFIG_DIR        | ${EWS_SYSCONF_DIR}/ews"
echo "EWS_CONFIG_FILE       | ${EWS_SYSCONF_DIR}/ews/method.ini"
echo "EWS_MODULES_DIR       | ${EWS_LIB_DIR}/ews/modules"
echo "EWS_CONSOLE_SOCKET    | ${EWS_LOCAL_STATE_DIR}/run/ews/ews.ctl"
echo "EWS_CONFIG_STATIC     | ${EWS_CONFIG_STATIC}"
echo "EWS_VERBOSE_STDOUT    | ${EWS_VERBOSE_STDOUT}"
echo "MODULES               | ${MODULES}"
echo "------------------------------------------------------------------------------------"
