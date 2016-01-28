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

#include "tooltip.h"

#include "birthday_reminder.h"
#include "birthday_access.h"

#include <glib.h>
#include <gtkplugin.h>
#include <gtkblist.h>

extern PurplePlugin *plugin;

static void drawing_tooltip_cb(PurpleBlistNode *node, GString *str, gboolean full, void *data) {
	GDate date;
	gint days_to_birthday;

	get_birthday_from_node(node, &date);
	if(g_date_valid(&date)) {
		if(purple_prefs_get_bool(
			PLUGIN_PREFS_PREFIX "/tooltip/show_birthday"
		)) {
			if(g_date_get_year(&date) > 1900) {
				g_string_append_printf(str,
					/* Translators: to use an other order of the arguments use %2$02d for day, %1$02d for month and %3$04d for year */
					_("\n<b>Birthday</b>: %02d/%02d/%04d"),
					g_date_get_month(&date),
					g_date_get_day(&date),
					g_date_get_year(&date)
				);
			} else {
				g_string_append_printf(str,
					/* Translators: use %2$02d for day and %1$02d for month */
					_("\n<b>Birthday</b>: %02d/%02d"),
					g_date_get_month(&date),
					g_date_get_day(&date)
				);
			}
	
			if(purple_prefs_get_bool(
				PLUGIN_PREFS_PREFIX "/tooltip/show_days_to_birthday"
			)) {
				days_to_birthday = get_days_to_birthday_from_node(node);
				if(days_to_birthday == 0) {
					g_string_append_printf(str, _(" (<b>Today!</b>)"));
				} else if(days_to_birthday == 1) {
					g_string_append_printf(str, _(" (<b>Tomorrow!</b>)"));
				} else {
					g_string_append_printf(str,
						_(" (in %d days)"),
						days_to_birthday
					);
				}
			}
		}

		if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_age")) {
			if(g_date_get_year(&date) > 1900) {
				g_string_append_printf(str,
					_("\n<b>Age</b>: %d years"),
					get_age_from_node(node)
				);
			}
		}
	}
}

void tooltip_init(void) {
	purple_signal_connect(
		pidgin_blist_get_handle(), "drawing-tooltip",
		plugin, PURPLE_CALLBACK(drawing_tooltip_cb),
		NULL
	);
} 

/* ex: set noexpandtab: */
