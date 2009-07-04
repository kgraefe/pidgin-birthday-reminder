#!/bin/bash
make clean && \
make && \
PROJECT=birthday_reminder && \
DIR=${PROJECT}-`cat VERSION`-linux-x86 && \
mkdir -p ${DIR}/plugins && \
mkdir -p ${DIR}/pixmaps/pidgin/${PROJECT} && \
mkdir -p ${DIR}/sounds/pidgin/${PROJECT} && \
mkdir -p ${DIR}/locale/de/LC_MESSAGES && \
mkdir -p ${DIR}/locale/ru/LC_MESSAGES && \
cp ChangeLog ${DIR}/ChangeLog && \
cp README.linux_binary ${DIR}/ReadMe && \
cp src/.libs/${PROJECT}.so ${DIR}/plugins && \
cp share/pixmaps/*.png ${DIR}/pixmaps/pidgin/${PROJECT} && \
cp share/sounds/*.wav ${DIR}/sounds/pidgin/${PROJECT} && \
cp po/de.gmo ${DIR}/locale/de/LC_MESSAGES/${PROJECT}.mo && \
cp po/ru.gmo ${DIR}/locale/ru/LC_MESSAGES/${PROJECT}.mo && \
cp install_linux_package.sh ${DIR}/install.sh && \
tar czvf ${DIR}.tar.gz ${DIR} && \
rm -rf ${DIR}
