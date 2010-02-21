plugindir = $(libdir)/pidgin
DEFS = -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)\"

plugin_LTLIBRARIES = birthday_reminder.la
birthday_reminder_la_SOURCES = \
	birthday_reminder.c \
	functions.c \
	pidgin_internals.c \
	birthday_access.c \
	tooltip.c \
	emblems.c \
	notification.c \
	check.c \
	scan.c \
	input.c \
	prefs.c \
	birthday_list.c \
	plugin_actions.c \
	icsexport.c \
	hash.c

birthday_reminder_la_LDFLAGS = -module -avoid-version

