AC_INIT([glipper],[1.0],[svenrech@gmx.de])
AC_CONFIG_SRCDIR(glipper/__init__.py)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
GNOME_COMMON_INIT

AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")
AC_PROG_CC

dnl ****************************************************************************
dnl * Pkg-Config
dnl ****************************************************************************
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, yes, no)
if test "x$HAVE_PKGCONFIG" = "xno"; then
  AC_MSG_ERROR(you need to have pkgconfig installed !)
fi

dnl ****************************************************************************
dnl * Translation & i18n
dnl ****************************************************************************

GETTEXT_PACKAGE="glipper"
IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [The gettext package])
AM_GLIB_GNU_GETTEXT

dnl ****************************************************************************
dnl * Gnome Doc Utils
dnl ****************************************************************************
GNOME_DOC_INIT

dnl ****************************************************************************
dnl * GConf support
dnl ****************************************************************************
AC_PATH_PROG(GCONFTOOL, gconftool-2)
AM_GCONF_SOURCE_2

dnl ****************************************************************************
dnl * Python 2.5
dnl ****************************************************************************
AM_PATH_PYTHON(2.5)
AM_CHECK_PYTHON_HEADERS(,[AC_MSG_ERROR(could not find Python headers)])

dnl ****************************************************************************
dnl * Write the values of various paths in defs.py
dnl ****************************************************************************
AC_SUBST(VERSION)
AC_SUBST(PACKAGE)

AS_AC_EXPAND(DATADIR, $datarootdir)
AC_SUBST(DATADIR)

AS_AC_EXPAND(LIBDIR, $libdir)
AC_SUBST(LIBDIR)

AS_AC_EXPAND(PYTHONDIR, $pythondir)
AC_SUBST(PYTHONDIR)
		
dnl ****************************************************************************
dnl * PyGTK
dnl ****************************************************************************
PKG_CHECK_MODULES(GLIPPER,
	gtk+-2.0				>= 2.6
	pygtk-2.0				>= 2.6
	pygobject-2.0			>= 2.6
	gnome-python-2.0		>= 2.10
)
AC_SUBST(GLIPPER_CFLAGS)
AC_SUBST(GLIPPER_LIBS)

AC_MSG_CHECKING([for gnomeapplet module])
if AC_RUN_LOG([DISPLAY= $PYTHON -c '
try:
    import gnomeapplet
except ImportError, e:
    if str(e).find("gnomeapplet") >= 0:
          raise
except:
    pass
']); then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([gnomeapplet Python module required to build glipper])
fi

dnl ****************************************************************************
dnl * PyGTK Codegen and defs files
dnl ****************************************************************************
AC_PATH_PROG(PYGTK_CODEGEN, pygtk-codegen-2.0, no)
if test "x$PYGTK_CODEGEN" = xno; then
  AC_MSG_ERROR(could not find pygtk-codegen-2.0 script)
fi

AC_MSG_CHECKING(for pygtk defs)
PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
AC_SUBST(PYGTK_DEFSDIR)
AC_MSG_RESULT($PYGTK_DEFSDIR)

dnl ****************************************************************************
dnl * Check for prctl()
dnl ****************************************************************************
AC_CHECK_FUNC(prctl, HAVE_PRCTL="yes", HAVE_PRCTL="yes")
if test "x$HAVE_PRCTL" = "xyes"; then
	AC_CHECK_DECL(PR_SET_NAME, STILL_HAVE_PRCTL="yes", STILL_HAVE_PRCTL="no", [#include <sys/prctl.h>])
	if test "x$STILL_HAVE_PRCTL" = "xyes"; then
		AC_DEFINE(HAVE_PRCTL, 1, [Defined as 1 if prctl and PR_SET_NAME are available])
	fi
fi

dnl ****************************************************************************
dnl * --------------------------------------------------------------------------
dnl ****************************************************************************
AC_CONFIG_FILES([
Makefile
glipper/Makefile
glipper/defs.py
glipper/plugins/Makefile
glipper/keybinder/Makefile
glipper/osutils/Makefile
data/Makefile
data/glipper.pc
data/art/Makefile
po/Makefile.in
help/Makefile
])
AC_OUTPUT

echo
echo $PACKAGE v$VERSION
echo

echo Prefix............... : $prefix
echo
