#! /bin/sh
set +x

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

mkdir -p m4
intltoolize --force --copy --automake || exit 1
autoreconf --force --install --verbose || exit 1

