#! /bin/sh
set -x

test -f VERSION || exit
test -f configure.ac.in || exit
test -f COPYING || exit

languages=""
for f in po/*.po
do test -f $f && languages="$languages $(basename $f .po)"
done

sed \
    -e "s/@@VERSION@@/$(cat VERSION)/" \
    -e "s/@@LANGUAGES@@/$(echo $languages)/" \
configure.ac.in >configure.ac || exit

aclocal || exit
autoheader || exit
libtoolize --copy || exit
automake --add-missing --copy --no-force || exit
autoconf || exit
libtoolize --copy --install || exit
intltoolize --copy --force  || exit
aclocal || exit
