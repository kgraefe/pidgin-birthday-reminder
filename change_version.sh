#!/bin/bash
vim ChangeLog &&
vim VERSION &&

languages=""
for f in po/*.po
do languages="$languages $(basename $f .po)"
done

sed -e "s/@@VERSION@@/$(cat VERSION)/" -e "s/@@LANGUAGES@@/$(echo $languages)/" configure.in.in >configure.in &&
./autogen.sh &&
./configure &&
cp config.h config.h.mingw
