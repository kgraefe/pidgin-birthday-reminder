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

#include "scan.h"

#include "internal.h"

#include <glib.h>
#include <connection.h>
#include <debug.h>
#include <accountopt.h>

#include "birthday_reminder.h"
#include "check.h"
#include "icsexport.h"

extern PurplePlugin *plugin;

static guint scan_buddies_timeout_handle;
static void *(*notify_userinfo_ori)(
	PurpleConnection *gc,
	const char *who, PurpleNotifyUserInfo *user_info
);
static PurpleBuddy *current_scanned_buddy;

static const char *get_textdomain_by_protocol_id(const char *protocol_id) {
	if(
		purple_utf8_strcasecmp(protocol_id, "prpl-icq") == 0 ||
		purple_utf8_strcasecmp(protocol_id, "prpl-aim") == 0 ||
		purple_utf8_strcasecmp(protocol_id, "prpl-msn") == 0 ||
		purple_utf8_strcasecmp(protocol_id, "prpl-jabber") == 0 ||
		purple_utf8_strcasecmp(protocol_id, "prpl-skypeweb") == 0
	) {
		return "pidgin";
	}

	return NULL;
}

static gboolean scan_next_buddy(gpointer data) {
	PurpleBlistNode *node;
	PurpleBuddy *buddy;
	PurpleAccount *acc;
	PurplePlugin *prpl;
	PurplePluginProtocolInfo *prpl_info;
	int last_scan_julian, pivot_date_julian;
	GDate pivot_date;

	/* Let's rescan buddies if the last scan was more than 3 months ago. */
	g_date_set_time_t(&pivot_date, time(NULL));
	g_date_subtract_months(&pivot_date, 3);
	pivot_date_julian = g_date_get_julian(&pivot_date);

	purple_timeout_remove(scan_buddies_timeout_handle);
	scan_buddies_timeout_handle = 0;

	current_scanned_buddy = NULL;
	for(
		node = purple_blist_get_root();
		node && !current_scanned_buddy;
		node = purple_blist_node_next(node, TRUE)
	) {
		if(!PURPLE_BLIST_NODE_IS_BUDDY(node)) {
			continue;
		}
		buddy = (PurpleBuddy *)node;

		last_scan_julian = purple_blist_node_get_int(node, "birthday_last_scan");
		if(last_scan_julian != 0 && last_scan_julian >= pivot_date_julian) {
			continue;
		}

		acc = buddy->account;
		if(
			!purple_account_is_connected(acc) ||
			!purple_account_get_bool(acc, "birthday_scan_enabled", TRUE) ||
			!acc->gc ||
			!get_textdomain_by_protocol_id(purple_account_get_protocol_id(acc))
		) {
			continue;
		}

		prpl = purple_connection_get_prpl(acc->gc);
		if(!prpl) {
			continue;
		}

		prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);
		if(!prpl_info || !prpl_info->get_info) {
			continue;
		}

		if(!buddy->name) {
			continue;
		}

		current_scanned_buddy = buddy;
		prpl_info->get_info(acc->gc, buddy->name);

		purple_debug_info(PLUGIN_STATIC_NAME,
			/* Translators: use %1$s for the buddy's nickname, %2$s for the account name and %3$s for the protocol name. */
			_("Scanning buddy %s (Account: %s (%s)). Waiting for response...\n"),
			purple_buddy_get_name(current_scanned_buddy),
			purple_account_get_username(current_scanned_buddy->account),
			purple_account_get_protocol_name(current_scanned_buddy->account)
		);

		return FALSE; /* Stop the timer */
	}

	purple_debug_info(PLUGIN_STATIC_NAME, _("No more buddies to scan.\n"));
	scan_buddies_timeout_handle = purple_timeout_add_seconds(
		SCAN_BUDDIES_TIMEOUT_SECONDS, scan_next_buddy, NULL
	);

	return FALSE; /* Stop the timer */
}

static void displaying_userinfo_cb(
	PurpleAccount *account, const char *who,
	PurpleNotifyUserInfo *user_info, PurpleBuddy *_buddy
) {
	PurpleNotifyUserInfoEntry *e;
	PurpleBlistNode *node;
	PurpleBuddy *buddy;
	const char *textdomain;
	const char *needle;
	const char *label;

	GList *l;
	GDate *date;
	GDate today;
	int today_julian;


	if(!account || !who) {
		return;
	}

	textdomain = get_textdomain_by_protocol_id(
		purple_account_get_protocol_id(account)
	);
	if(!textdomain) {
		return;
	}
	needle = dgettext(textdomain, "Birthday");

	buddy = purple_find_buddy(account, who);
	if(!buddy) {
		return;
	}
	node = (PurpleBlistNode *)buddy;

	g_date_set_time_t(&today, time(NULL));
	today_julian = g_date_get_julian(&today);
	purple_blist_node_set_int(node, "birthday_last_scan", today_julian);

	l=purple_notify_user_info_get_entries(user_info);
	while(l) {
		e = l->data;

		label = purple_notify_user_info_entry_get_label(e);

		if(
			purple_utf8_strcasecmp(label, needle) == 0 ||
			purple_utf8_strcasecmp(label, "Birthday") == 0
		) {
			date = g_date_new();
			g_date_set_parse(date, purple_notify_user_info_entry_get_value(e));

			if(g_date_valid(date)) {
				purple_blist_node_set_int(
					node,
					"birthday_julian",
					g_date_get_julian(date)
				);
				automatic_export();
				check_birthdays(NULL, buddy);
			}

			g_date_free(date);

			return;
		}

		l = l->next;
	}
}

static void *birthday_reminder_notify_userinfo(
	PurpleConnection *gc,
	const char *who, PurpleNotifyUserInfo *user_info
) {
	if(
		!current_scanned_buddy ||
		current_scanned_buddy->account != gc->account ||
		purple_utf8_strcasecmp(current_scanned_buddy->name, who) != 0
	) {
		return notify_userinfo_ori(gc, who, user_info);
	}

	purple_debug_info(PLUGIN_STATIC_NAME,
		/* Translators: use %1$s for the buddy's nickname, %2$s for the account name and %3$s for the protocol name. */
		_("Buddy %s (Account: %s (%s)) scanned.\n"),
		purple_buddy_get_name(current_scanned_buddy),
		purple_account_get_username(current_scanned_buddy->account),
		purple_account_get_protocol_name(current_scanned_buddy->account)
	);
	
	current_scanned_buddy = NULL;
	scan_buddies_timeout_handle = purple_timeout_add_seconds(
		SCAN_BUDDIES_TIMEOUT_SECONDS,
		scan_next_buddy,
		NULL
	);

	/* Suppress user info window */
	return NULL;
}

#define PROTOCOL_OPTION_ADD 0
#define PROTOCOL_OPTION_REMOVE 1
static void protocol_option_helper(gpointer data, gpointer user_data) {
	GList *e;
	PurpleAccountOption *option;
	PurplePlugin *prpl = data;
	PurplePluginProtocolInfo *prpl_info;
	int op = GPOINTER_TO_INT(user_data);

	if(!prpl || !prpl->info) {
		return;
	}

	prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);
	if(!prpl_info) {
		return;
	}

	if(!get_textdomain_by_protocol_id(prpl->info->id)) {
		return;
	}

	switch(op) {
	case PROTOCOL_OPTION_ADD:
		option = purple_account_option_bool_new(
			_("Scan birthdays on this account"),
			"birthday_scan_enabled",
			TRUE
		);
		prpl_info->protocol_options = g_list_append(
			prpl_info->protocol_options, option
		);
		break;
	
	case PROTOCOL_OPTION_REMOVE:
		e = prpl_info->protocol_options;
		while(e) {
			option = e->data;

			if(purple_utf8_strcasecmp(
				option->pref_name, "birthday_scan_enabled"
			) == 0) {
				prpl_info->protocol_options = g_list_remove(
					prpl_info->protocol_options, option
				);
				return;
			}

			e = e->next;
		}
		break;
	}
}

void init_scan(void) {
	PurpleNotifyUiOps *ops;

	scan_buddies_timeout_handle=0;
	
	purple_signal_connect(
		purple_notify_get_handle(), "displaying-userinfo",
		plugin, PURPLE_CALLBACK(displaying_userinfo_cb),
		NULL
	);

	ops = purple_notify_get_ui_ops();
	notify_userinfo_ori = ops->notify_userinfo;
	ops->notify_userinfo = birthday_reminder_notify_userinfo;

	scan_buddies_timeout_handle = purple_timeout_add_seconds(
		SCAN_BUDDIES_TIMEOUT_SECONDS, scan_next_buddy,
		NULL
	);

	/* Add option to scan birthdays to all supported accounts. */
	g_list_foreach(
		purple_plugins_get_protocols(),
		protocol_option_helper,
		GINT_TO_POINTER(PROTOCOL_OPTION_ADD)
	);
}

void uninit_scan(void) {
	if(scan_buddies_timeout_handle > 0) {
		purple_timeout_remove(scan_buddies_timeout_handle);
	}

	/* Remove option to scan birthdays from all supported accounts. */
	g_list_foreach(
		purple_plugins_get_protocols(),
		protocol_option_helper,
		GINT_TO_POINTER(PROTOCOL_OPTION_REMOVE)
	);
}

/* ex: set noexpandtab: */
