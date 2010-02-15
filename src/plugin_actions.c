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

#include "plugin_actions.h"

#include "internal.h"

#include <blist.h>

#include "birthday_reminder.h"
#include "check.h"
#include "birthday_list.h"

static void check_birthdays_plugin_action_cb(PurplePluginAction *action) {
	check_birthdays(NULL, NULL);
	purple_blist_set_visible(TRUE);
}

static void birthday_list_show_cb(PurplePluginAction *action) {
	birthday_list_show();
}

GList *plugin_actions(PurplePlugin *plugin, gpointer context) {
	GList *l;
	PurplePluginAction *action;

	l = NULL;

	action = purple_plugin_action_new(_("Check Birthdays"), check_birthdays_plugin_action_cb);
	l = g_list_append(l, action);

	action = purple_plugin_action_new(_("Show birthday list"), birthday_list_show_cb);
	l = g_list_append(l, action);

	return l;
}

