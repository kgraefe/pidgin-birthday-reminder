/*
 * Pidgin Birthday Reminder
 * Copyright (C) 2008-2016 Konrad Gräfe
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301, USA.
 */

#include "notification.h"

#include "internal.h"

#include <gtk/gtk.h>
#include <sound.h>
#include <debug.h>
#include <request.h>
#include <gtkutils.h>
#include <gtkblist.h>
#include <pidginstock.h>

#include "birthday_reminder.h"
#include "functions.h"
#include "birthday_access.h"
#include "birthday_list.h"

extern PurplePlugin *plugin;

static GtkWidget *mini_dialog;
static void *request_ui_handle;


/* mini dialog */
static void mini_dialog_write_im_cb(PurpleBlistNode *node) {
	write_im(node);
}

static void mini_dialog_overview_cb(PurpleBlistNode *node) {
	birthday_list_show();
}

static void mini_dialog_close_cb(PurpleBlistNode *node) {}

/* sound */
static void play_sound(void) {
	gchar *soundfile;

	soundfile = g_build_filename(
		purple_user_dir(),
		"sounds",
		"pidgin",
		"birthday_reminder",
		"birthday.wav",
		NULL
	);

	if(!g_file_test(soundfile, G_FILE_TEST_EXISTS)) {
		g_free(soundfile);
		soundfile = g_build_filename(
			DATADIR,
			"sounds",
			"pidgin",
			"birthday_reminder",
			"birthday.wav",
			NULL
		);
	}

	if(g_file_test(soundfile, G_FILE_TEST_EXISTS)) {
		purple_sound_play_file(soundfile, NULL);
	} else {
		purple_debug_error(
			PLUGIN_STATIC_NAME,
			_("Sound file (%s) does not exist.\n"),
			soundfile
		);
	}
	g_free(soundfile);
}

void notify(gint days_to_birthday, PurpleBuddy *birthday_buddy) {
	gint play_sound_before_days;
	gint show_mini_dialog_before_days;
	gint show_notification_before_days;

	PurpleBlistNode *node;
	gchar *msg;
	const gchar *alias;
	gint age;
	GDate date;
	gchar *strAge = NULL;

	if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/sound/play")) {
		play_sound_before_days = purple_prefs_get_int(
			PLUGIN_PREFS_PREFIX "/reminder/sound/before_days"
		);
	} else {
		play_sound_before_days = -1;
	}

	if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/show")) {
		show_mini_dialog_before_days = purple_prefs_get_int(
			PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/before_days"
		);
	} else {
		show_mini_dialog_before_days = -1;
	}

	if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/notification/show")) {
		show_notification_before_days = purple_prefs_get_int(
			PLUGIN_PREFS_PREFIX "/reminder/notification/before_days"
		);
	} else {
		show_notification_before_days = -1;
	}


	if(days_to_birthday <= play_sound_before_days) {
		play_sound();
	}
	
	msg = NULL;
	if(
		days_to_birthday <= show_mini_dialog_before_days ||
		days_to_birthday <= show_notification_before_days
	) {
		/* When there is an existing mini dialog, use this to notify all
		 * birthdays.
		 *
		 * TODO: Is there a way to check whether the mini dialog has been
		 * closed already?
		 */
		if(birthday_buddy && mini_dialog == NULL) {
			node = (PurpleBlistNode *)birthday_buddy;
			alias = purple_buddy_get_contact_alias(birthday_buddy);
			age = get_age_from_node(node);
			days_to_birthday = get_days_to_birthday_from_node(node);
	
			get_birthday_from_node(node, &date);

			if(days_to_birthday > 0) {
				age += 1;
			}
			strAge = g_strdup_printf(
				/* Translators: This creates the age of a buddy which is
				 *              used in several strings that may contain
				 *              another number. I will refer to this string
				 *              as {age} in the comments. Hopefully that
				 *              works for all languages. Please come back to
				 *              me if not.
				 */
				dngettext(GETTEXT_PACKAGE, "%d year", "%d years", age),
				age
			);
	
			if(days_to_birthday == 0) {
				if(g_date_get_year(&date) > 1900) {
					msg = g_strdup_printf(
						/* Translators: The first string is the buddies alias
						 *              name. The second string is his {age}.
						 */
						_("%s will be %s old today! Hooray!"),
						alias, strAge
					);
				} else {
					msg = g_strdup_printf(
						_("It's %s's birthday today! Hooray!"),
						alias
					);
				}
			} else if(days_to_birthday == 1) {
				if(g_date_get_year(&date) > 1900) {
					msg = g_strdup_printf(
						/* Translators: The first string is the buddies alias
						 *              name. The second string is his {age}.
						 */
						_("%s will be %s old tomorrow!"),
						alias, strAge
					);
				} else {
					msg = g_strdup_printf(
						_("%s's birthday is tomorrow!"),
						alias
					);
				}
			} else {
				if(g_date_get_year(&date) > 1900) {
					msg = g_strdup_printf(
						dngettext(
							GETTEXT_PACKAGE,
							/* Translators: The first string is the buddies
							 *              alias name. The second string is his
							 *              {age}.
							 */
							"%s will be %s old in %d day!",
							"%s will be %s old in %d days!",
							days_to_birthday
						),
						alias, strAge, days_to_birthday
					);
				} else {
					msg = g_strdup_printf(
						dngettext(
							GETTEXT_PACKAGE,
							"%s's birthday is in %d day!",
							"%s's birthday is in %d days!",
							days_to_birthday
						),
						alias, days_to_birthday
					);
				}
			}
		} else {
			msg = g_strdup(_("There are birthdays in the next days! Hooray!"));
		}
	}
	
	if(days_to_birthday <= show_notification_before_days) {
		if(request_ui_handle) {
			purple_request_close(PURPLE_REQUEST_ACTION, request_ui_handle);
		}

		if(birthday_buddy && mini_dialog == NULL) {
			request_ui_handle = purple_request_action(plugin,
				_("Birthday Reminder"), msg,
				_("Write IM?"),
				0, NULL, NULL, NULL,
				purple_buddy_get_contact(birthday_buddy), 2,
				_("Yes"), PURPLE_CALLBACK(mini_dialog_write_im_cb),
				_("No"), NULL
			);
		} else {
			request_ui_handle = purple_request_action(plugin,
				_("Birthday Reminder"), msg,
				_("Show overview?"),
				0, NULL, NULL, NULL, NULL, 2,
				_("Yes"), PURPLE_CALLBACK(mini_dialog_overview_cb),
				_("No"), NULL
			);
		}
	}

	if(days_to_birthday <= show_mini_dialog_before_days) {
		if(birthday_buddy && mini_dialog == NULL) {
			mini_dialog = pidgin_make_mini_dialog(NULL,
				PIDGIN_STOCK_DIALOG_INFO, _("Birthday Reminder"),
				msg, purple_buddy_get_contact(birthday_buddy),
				_("Close"), PURPLE_CALLBACK(mini_dialog_close_cb),
				_("Write IM"), PURPLE_CALLBACK(mini_dialog_write_im_cb),
				NULL
			);
		} else {
			if(mini_dialog) {
				gtk_widget_destroy(mini_dialog);
			}
			mini_dialog = pidgin_make_mini_dialog(NULL,
				PIDGIN_STOCK_DIALOG_INFO, _("Birthday Reminder"),
				msg, NULL,
				_("Close"), PURPLE_CALLBACK(mini_dialog_close_cb),
				_("Overview"), PURPLE_CALLBACK(mini_dialog_overview_cb),
				NULL
			);
		}
		g_signal_connect(
			G_OBJECT(mini_dialog), "destroy",
			G_CALLBACK(gtk_widget_destroyed), &mini_dialog
		);
		pidgin_blist_add_alert(mini_dialog);
		purple_blist_set_visible(TRUE);
	}

	if(strAge) {
		g_free(strAge);
	}
	if(msg) {
		g_free(msg);
	}
}

void init_notification(void) {
	mini_dialog = NULL;
	request_ui_handle = NULL;
}

/* ex: set noexpandtab: */
