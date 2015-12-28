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

#include "plugin_actions.h"

#include "internal.h"

#include <blist.h>

#include <gtk/gtk.h>

#include "birthday_reminder.h"
#include "check.h"
#include "birthday_list.h"
#include "icsexport.h"
#include "functions.h"

static void check_birthdays_plugin_action_cb(PurplePluginAction *action) {
	check_birthdays(NULL, NULL);
	purple_blist_set_visible(TRUE);
}

static void birthday_list_show_cb(PurplePluginAction *action) {
	birthday_list_show();
}

static void export_birthdays_cb(PurplePluginAction *action) {
	GtkWidget *dialog;
	GtkFileFilter *filter;
	gchar *path, *tmp;

	dialog = gtk_file_chooser_dialog_new(
		_("Save birthday list as..."),
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL
	);

	gtk_file_chooser_set_filename(
		GTK_FILE_CHOOSER(dialog),
		purple_prefs_get_path(PLUGIN_PREFS_PREFIX "/export/path")
	);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("iCalendar files"));
	gtk_file_filter_add_pattern(filter, "*.ics");
	gtk_file_filter_add_pattern(filter, "*.ICS");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if(!has_file_extension(path, "ics")) {
			tmp = path;
			path = g_strdup_printf("%s.ics", tmp);
			g_free(tmp);
		}

		icsexport(path);
		g_free(path);
	}

	gtk_widget_destroy(dialog);
}

GList *plugin_actions(PurplePlugin *plugin, gpointer context) {
	GList *l;
	PurplePluginAction *action;

	l = NULL;

	action = purple_plugin_action_new(	
		_("Check Birthdays"),
		check_birthdays_plugin_action_cb
	);
	l = g_list_append(l, action);

	action = purple_plugin_action_new(
		_("Show birthday list"),
		birthday_list_show_cb
	);
	l = g_list_append(l, action);

	action = purple_plugin_action_new(
		_("Export birthday list"),
		export_birthdays_cb
	);
	l = g_list_append(l, action);

	return l;
}

/* ex: set noexpandtab: */
