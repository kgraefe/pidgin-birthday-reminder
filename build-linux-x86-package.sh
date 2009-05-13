#!/bin/bash
make clean && \
make && \
PROJECT=birthday_reminder && \
DIR=${PROJECT}-`cat VERSION`-linux-x86 && \
mkdir -p ${DIR}/plugins && \
mkdir -p ${DIR}/pixmaps/pidgin/birthday_reminder && \
mkdir -p ${DIR}/sounds/pidgin/birthday_reminder && \
mkdir -p ${DIR}/locale/de/LC_MESSAGES && \
mkdir -p ${DIR}/locale/ru/LC_MESSAGES && \
cp ChangeLog ${DIR}/ChangeLog && \
cp src/.libs/${PROJECT}.so ${DIR}/plugins && \
cp share/pixmaps/*.png ${DIR}/pixmaps/pidgin/birthday_reminder && \
cp share/sounds/*.wav ${DIR}/sounds/pidgin/birthday_reminder && \
cp po/de.gmo ${DIR}/locale/de/LC_MESSAGES/${PROJECT}.mo && \
cp po/ru.gmo ${DIR}/locale/ru/LC_MESSAGES/${PROJECT}.mo && \
cp install_linux_package.sh ${DIR}/install.sh && \
tar czvf ${DIR}.tar.gz ${DIR} && \
rm -rf ${DIR}
