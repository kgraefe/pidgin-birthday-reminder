#!/bin/bash
make -f Makefile.mingw clean && \
make -f Makefile.mingw && \
PROJECT=pidgin-birthday-reminder && \
OLDPROJECTNAME=birthday_reminder && \
WIN32DIR=${PROJECT}-$(cat VERSION)-win32 && \
rm -rf ${WIN32DIR} && \
mkdir -p ${WIN32DIR}/pidgin/plugins && \
mkdir -p ${WIN32DIR}/pidgin/pixmaps/pidgin/${OLDPROJECTNAME} && \
mkdir -p ${WIN32DIR}/pidgin/sounds/pidgin/${OLDPROJECTNAME} && \
sed 's/$/\r/' ChangeLog >${WIN32DIR}/ChangeLog.txt && \
sed 's/$/\r/' README.win32 >${WIN32DIR}/ReadMe.txt && \
cp src/${PROJECT}.dll ${WIN32DIR}/pidgin/plugins && \
cp share/pixmaps/*.png ${WIN32DIR}/pidgin/pixmaps/pidgin/${OLDPROJECTNAME} && \
cp share/sounds/*.wav ${WIN32DIR}/pidgin/sounds/pidgin/${OLDPROJECTNAME} && \
i586-mingw32msvc-strip --strip-unneeded ${WIN32DIR}/pidgin/plugins/${PROJECT}.dll && \
for f in po/*.po; do
	lang=$(basename $f .po)
	mkdir -p ${WIN32DIR}/pidgin/locale/${lang}/LC_MESSAGES && \
	cp po/${lang}.gmo ${WIN32DIR}/pidgin/locale/${lang}/LC_MESSAGES/${PROJECT}.mo
done && \
rm -f ${WIN32DIR}.zip && \
cd ${WIN32DIR} && \
zip -r ../${WIN32DIR}.zip * && \
cd .. && \
rm -rf ${WIN32DIR}
