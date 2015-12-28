/*
 * Pidgin Birthday Reminder
 * Copyright (C) 2008-2015 Konrad Gräfe
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

#include "input.h"

#include "internal.h"

#include <blist.h>
#include <request.h>

#include "birthday_reminder.h"
#include "functions.h"
#include "emblems.h"
#include "check.h"
#include "birthday_access.h"
#include "icsexport.h"

extern PurplePlugin *plugin;

static void do_set_bday_cb(PurpleBlistNode *node, const char* bday) {
	PurpleBlistNode *child;
	PurpleBuddy *buddy;
	GDate date, today;

	if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		/* Set birthday to all buddies. */
		child = purple_blist_node_get_first_child(node);
		while(child) {
			do_set_bday_cb(child, bday);
			child = purple_blist_node_get_sibling_next(child);
		}
	}

	if(!PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		return;
	}
	buddy = (PurpleBuddy *) node;

	if(purple_utf8_strcasecmp(bday, "")==0) {
		purple_blist_node_remove_setting(node, "birthday_julian");
		purple_blist_node_remove_setting(node, "birthday_scanned");
		update_birthday_emblem(node);
		return;
	}

	g_date_clear(&date, 1);
	g_date_set_parse(&date, bday);

	g_date_set_today(&today);
	if(
		g_date_valid(&date) &&
		g_date_compare(&date, &today) < 0 &&
		g_date_get_year(&date) > 12
	) {
		purple_blist_node_set_int(node,
			"birthday_julian",
			g_date_get_julian(&date)
		);
		automatic_export();
		check_birthdays(NULL, buddy);
	}

	update_birthday_emblem(node);
	return;
}

static void set_bday_cb(PurpleBlistNode *node, gpointer data) {
	char *helptext;
	char *old_bday=NULL;
	GDate date;
	struct tm tm;

	if(
		!PURPLE_BLIST_NODE_IS_CONTACT(node) &&
		!PURPLE_BLIST_NODE_IS_BUDDY(node)
	) {
		return;
	}
	
	g_date_set_dmy(&date, 24, 12, 1986);
	g_date_to_struct_tm(&date, &tm);
	helptext = g_strdup_printf(
		_("Use the following format: %s .\nBlank out the input field to clear the BDay.\nUse a year before 1900 if you do not know."),
		purple_date_format_short(&tm)
	);

	get_birthday_from_node(node, &date);
	if(g_date_valid(&date)) {
		g_date_to_struct_tm(&date, &tm);
		old_bday = g_strdup_printf("%s", purple_date_format_short(&tm));
	}


	purple_request_input(plugin,
		_("Birthday Reminder"),
		_("Set Birthday:"),
		helptext,
		old_bday,
		FALSE, FALSE, NULL,
		_("OK"), PURPLE_CALLBACK(do_set_bday_cb),
		_("Cancel"),
		NULL, NULL, NULL, NULL,
		node
	);

	g_free(helptext);
	if(old_bday) {
		g_free(old_bday);
	}
}

static void extended_buddy_menu_cb(PurpleBlistNode *node, GList **menu) {
	if(
		!PURPLE_BLIST_NODE_IS_CONTACT(node) &&
		!PURPLE_BLIST_NODE_IS_BUDDY(node)
	) {
		return;
	}
	
	*menu = g_list_append(*menu,
		purple_menu_action_new(
			_("Set Birthday"),
			PURPLE_CALLBACK(set_bday_cb),
			NULL, NULL
		)
	);
}

void init_input(void) {
	purple_signal_connect(
		purple_blist_get_handle(), "blist-node-extended-menu",
		plugin, PURPLE_CALLBACK(extended_buddy_menu_cb),
		NULL
	);
}

/* ex: set noexpandtab: */
