#!/bin/bash
make clean && \
make && \
PROJECT=birthday_reminder && \
DIR=${PROJECT}-`cat VERSION`-linux-x86 && \
mkdir -p ${DIR}/plugins && \
mkdir -p ${DIR}/pixmaps/pidgin/${PROJECT} && \
mkdir -p ${DIR}/sounds/pidgin/${PROJECT} && \
cp ChangeLog ${DIR}/ChangeLog && \
cp README.linux_binary ${DIR}/ReadMe && \
cp src/.libs/${PROJECT}.so ${DIR}/plugins && \
cp share/pixmaps/*.png ${DIR}/pixmaps/pidgin/${PROJECT} && \
cp share/sounds/*.wav ${DIR}/sounds/pidgin/${PROJECT} && \
for lang in de ru fr es; do
        mkdir -p ${DIR}/locale/${lang}/LC_MESSAGES && \
        cp po/${lang}.gmo ${DIR}/locale/${lang}/LC_MESSAGES/${PROJECT}.mo
done && \
cp install_linux_package.sh ${DIR}/install.sh && \
tar czvf ${DIR}.tar.gz ${DIR} && \
rm -rf ${DIR}
