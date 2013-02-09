#!/bin/sh

  GETTEXTIZE="gettextize"
  $GETTEXTIZE --version < /dev/null > /dev/null 2>&1
  if test $? -ne 0; then
    echo
    echo "**Error**: You must have \`$GETTEXTIZE' installed" \
         "to compile Glipper."
  	exit
  fi

(intltoolize --version) < /dev/null > /dev/null 2>&1 || {
 echo
 echo "**Error**: You must have \`intltoolize' installed" \
      "to compile Glipper."
 exit
}

if test "$GETTEXTIZE"; then
 echo "Creating $dr/aclocal.m4 ..."
 test -r aclocal.m4 || touch aclocal.m4
 echo "Running $GETTEXTIZE...  Ignore non-fatal messages."
 echo "no" | $GETTEXTIZE --force --copy
 echo "Making aclocal.m4 writable ..."
 test -r aclocal.m4 && chmod u+w aclocal.m4
fi

intltoolize --copy --force --automake

autoheader
aclocal
automake -ac
autoconf

echo "You can type ./configure now, to configure glipper"
