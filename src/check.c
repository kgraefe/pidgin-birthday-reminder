/*
 * Birthday Reminder
 * Copyright (C) 2008 Konrad Gräfe
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

#include "check.h"

#include "internal.h"

#include <gtkblist.h>

#include "birthday_reminder.h"
#include "functions.h"
#include "birthday_access.h"
#include "notification.h"

extern PurplePlugin *plugin;

static GDate last_check;
static guint check_birthdays_timeout_handle;

void check_birthdays(PurpleAccount *acc, PurpleBuddy *buddy) {
	PurpleBlistNode *node;
	PurpleBuddy *birthday_buddy = NULL;
	gint days_to_birthday;
	gint min_days_to_birthday = 10;
	gint notify_before_days;
	gint count_birthdays = 0;

	gint play_sound_before_days;
	gint show_mini_dialog_before_days;
	gint show_notification_before_days;

	g_date_set_today(&last_check);

	if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/sound/play")) {
		play_sound_before_days = purple_prefs_get_int(PLUGIN_PREFS_PREFIX "/reminder/sound/before_days");
	} else {
		play_sound_before_days = -1;
	}
	if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/show")) {
		show_mini_dialog_before_days = purple_prefs_get_int(PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/before_days");
	} else {
		show_mini_dialog_before_days = -1;
	}
	if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/notification/show")) {
		show_notification_before_days = purple_prefs_get_int(PLUGIN_PREFS_PREFIX "/reminder/notification/before_days");
	} else {
		show_notification_before_days = -1;
	}

	notify_before_days = -1;
	if(play_sound_before_days > notify_before_days) notify_before_days = play_sound_before_days;
	if(show_mini_dialog_before_days > notify_before_days) notify_before_days = show_mini_dialog_before_days;
	if(show_notification_before_days > notify_before_days) notify_before_days = show_notification_before_days;
	
	if(notify_before_days == 10) return;

	if(buddy) {
		node = (PurpleBlistNode *)buddy;
		days_to_birthday = get_days_to_birthday_from_node(node);
		if(days_to_birthday >= 0 && days_to_birthday <= notify_before_days && node_account_connected(node) && (acc == NULL || buddy->account == acc) && (!already_notified_today(node) || !purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/once_a_day"))) {
			count_birthdays++;
			min_days_to_birthday = days_to_birthday;
			birthday_buddy = buddy;
			purple_blist_node_set_int(node, "last_birthday_notification_julian", g_date_get_julian(&last_check));
		}
	} else {
		node=purple_blist_get_root();
		while(node) {
			if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
				days_to_birthday = get_days_to_birthday_from_node(node);
				buddy = (PurpleBuddy *)node;

				if(days_to_birthday >= 0 && days_to_birthday <= notify_before_days && node_account_connected(node) && (acc == NULL || buddy->account == acc) && (!already_notified_today(node) || !purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/once_a_day"))) {
					count_birthdays++;
					birthday_buddy = buddy;

					if(days_to_birthday < min_days_to_birthday) min_days_to_birthday = days_to_birthday;
					purple_blist_node_set_int(node, "last_birthday_notification_julian", g_date_get_julian(&last_check));
				}
			}
			node=purple_blist_node_next(node, TRUE);
		}
	}

	if(count_birthdays == 1) {
		notify(min_days_to_birthday, birthday_buddy);
	} else if(count_birthdays > 1) {
		notify(min_days_to_birthday, NULL);
	}
	
	if(purple_get_blist()) pidgin_blist_refresh(purple_get_blist());
}

static gboolean check_birthdays_timer_cb(gpointer data) {
	GDate today;
	time_t now;
	struct tm *tm_now;

	g_date_set_today(&today);

	if(!g_date_valid(&last_check) || g_date_compare(&last_check, &today) != 0) {
		check_birthdays(NULL, NULL);
	}
	
	now = time(NULL);
	tm_now = localtime(&now);
	
	if(check_birthdays_timeout_handle > 0) purple_timeout_remove(check_birthdays_timeout_handle);

	if(tm_now->tm_hour >= 23) {
		/* Nach 23:00 Uhr? -> Timer um 00:00:05 triggern! */
		check_birthdays_timeout_handle = purple_timeout_add_seconds((3605 - 60*tm_now->tm_min - tm_now->tm_sec), check_birthdays_timer_cb, NULL);
	} else {
		check_birthdays_timeout_handle = purple_timeout_add_seconds(3600, check_birthdays_timer_cb, NULL);
	}

	return FALSE;
}

static void signed_on_cb(PurpleConnection *gc, gpointer data) {
	check_birthdays(gc->account, NULL);
}

void init_check(void) {
	check_birthdays_timeout_handle=0;
	g_date_clear(&last_check, 1);
	
	purple_signal_connect(purple_connections_get_handle(), "signed-on", plugin, PURPLE_CALLBACK(signed_on_cb), NULL);

	/* Geburtstage prüfen, Mitternachtstimer aktivieren */
	check_birthdays_timer_cb(NULL);
}

void uninit_check(void) {
	if(check_birthdays_timeout_handle > 0) purple_timeout_remove(check_birthdays_timeout_handle);
}
