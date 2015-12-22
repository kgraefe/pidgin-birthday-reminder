EXTRA_DIST = \
	ChangeLog \
	VERSION \
	Makefile.mingw \
	src/Makefile.mingw \
	po/Makefile.mingw \
	config.h.mingw \
	src/internal.h \
	src/birthday_access.h \
	src/birthday_list.h \
	src/birthday_reminder.h \
	src/check.h \
	src/emblems.h \
	src/functions.h \
	src/hash.h \
	src/icsexport.h \
	src/input.h \
	src/notification.h \
	src/pidgin_internals.h \
	src/plugin_actions.h \
	src/prefs.h \
	src/scan.h \
	src/tooltip.h

SUBDIRS = src po share

ACLOCAL_AMFLAGS=-I m4

DIST_BIN_DIR = $(PACKAGE)-$(VERSION)-linux-$(build_cpu)
DIST_BIN_ARCHIVE = $(DIST_BIN_DIR).tar.gz

dist-bin: $(DIST_BIN_ARCHIVE)

$(DIST_BIN_ARCHIVE): all
	rm -rf $(DIST_BIN_DIR)
	mkdir -p $(DIST_BIN_DIR)/plugins
	mkdir -p $(DIST_BIN_DIR)/pixmaps/pidgin/birthday_reminder
	mkdir -p $(DIST_BIN_DIR)/sounds/pidgin/birthday_reminder
	cp src/.libs/birthday_reminder.so $(DIST_BIN_DIR)/plugins/
	cp share/pixmaps/*.png $(DIST_BIN_DIR)/pixmaps/pidgin/birthday_reminder/
	cp share/sounds/*.wav $(DIST_BIN_DIR)/pixmaps/pidgin/birthday_reminder/
	for lang in $(ALL_LINGUAS); do \
		mkdir -p $(DIST_BIN_DIR)/locale/$${lang}/LC_MESSAGES || exit 1; \
		cp po/$${lang}.gmo $(DIST_BIN_DIR)/locale/$${lang}/LC_MESSAGES/birthday_reminder.mo || exit 1; \
	done
	tar czvf $(DIST_BIN_ARCHIVE) $(DIST_BIN_DIR)
	rm -rf $(DIST_BIN_DIR)
