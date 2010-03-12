#!/bin/bash
make -f Makefile.mingw clean && \
make -f Makefile.mingw && \
PROJECT=birthday_reminder && \
WIN32DIR=${PROJECT}-`cat VERSION`-win32 && \
mkdir -p ${WIN32DIR}/pidgin/plugins && \
mkdir -p ${WIN32DIR}/pidgin/pixmaps/pidgin/${PROJECT} && \
mkdir -p ${WIN32DIR}/pidgin/sounds/pidgin/${PROJECT} && \
sed 's/$/\r/' ChangeLog >${WIN32DIR}/ChangeLog.txt && \
sed 's/$/\r/' README.win32 >${WIN32DIR}/ReadMe.txt && \
cp src/${PROJECT}.dll ${WIN32DIR}/pidgin/plugins && \
cp share/pixmaps/*.png ${WIN32DIR}/pidgin/pixmaps/pidgin/${PROJECT} && \
cp share/sounds/*.wav ${WIN32DIR}/pidgin/sounds/pidgin/${PROJECT} && \
i586-mingw32msvc-strip --strip-unneeded ${WIN32DIR}/pidgin/plugins/${PROJECT}.dll && \
for lang in de ru fr es; do
	mkdir -p ${WIN32DIR}/pidgin/locale/${lang}/LC_MESSAGES && \
	cp po/${lang}.gmo ${WIN32DIR}/pidgin/locale/${lang}/LC_MESSAGES/${PROJECT}.mo
done && \
rm -f ${WIN32DIR}.zip && \
cd ${WIN32DIR} && \
zip -r ../${WIN32DIR}.zip * && \
cd .. && \
rm -rf ${WIN32DIR}
