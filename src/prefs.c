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

#include "prefs.h"

#include "internal.h"

#include <gtk/gtk.h>
#include <gtkblist.h>
#include <gtkutils.h>

#include "birthday_reminder.h"
#include "icsexport.h"

extern PurplePlugin *plugin;

static void birthday_icon_pref_changed_cb(const gchar *pref, PurplePrefType type, gconstpointer val, gpointer data) {
	if(purple_get_blist()) pidgin_blist_refresh(purple_get_blist());
}

static void toggle_cb(GtkWidget *widget, gpointer data) {
	gboolean value;
	gchar *pref;

	value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	pref = (gchar *) data;

	purple_prefs_set_bool(pref, value);
}

static void spin_cb(GtkWidget *widget, gpointer data) {
	gint value;
	gchar *pref;

	value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	pref = (gchar *) data;

	purple_prefs_set_int(pref, value);
}

static void entry_path_cb(GtkWidget *widget, gpointer data) {
	const gchar *value, *pref;

	value = gtk_entry_get_text(GTK_ENTRY(widget));
	pref = (gchar *) data;

	purple_prefs_set_path(pref, value);
}

static void export_filechooser_cb(GtkWidget *widget, gpointer data) {
	GtkEntry *entry;
	GtkWidget *dialog;
	GtkFileFilter *filter;
	gchar *new_path;

	entry = (GtkEntry *) data;

	dialog = gtk_file_chooser_dialog_new(_("Save birthday list as..."),
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), purple_prefs_get_path(PLUGIN_PREFS_PREFIX "/export/path"));

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("iCalendar files"));
	gtk_file_filter_add_pattern(filter, "*.ics");
	gtk_file_filter_add_pattern(filter, "*.ICS");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		new_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_entry_set_text(entry, new_path);
		g_free(new_path);
	}

	gtk_widget_destroy(dialog);
}

static void window_destroyed_cb(GtkWidget *widget, gpointer data) {
	automatic_export();
}

GtkWidget *get_config_frame(PurplePlugin *plugin) {
	GtkWidget *ret = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *toggle = NULL;
	GtkWidget *spin = NULL;
	GtkAdjustment *adjustment = NULL;
	GtkWidget *label = NULL;
	GtkWidget *entry = NULL;
	GtkWidget *button = NULL;
	GtkWidget *ref = NULL;

	ret = gtk_vbox_new(FALSE, 18);
	gtk_container_set_border_width(GTK_CONTAINER(ret), 12);

	/* Erinnerung */
	frame = pidgin_make_frame(ret, _("Reminder"));
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	
	
	toggle = gtk_check_button_new_with_mnemonic(_("Remind just once a day"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/once_a_day"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/reminder/once_a_day");

	toggle = gtk_check_button_new_with_mnemonic(_("Show birthday icons in the buddy list"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/show"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/show");

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);

	adjustment = (GtkAdjustment *) gtk_adjustment_new(1.0 * purple_prefs_get_int(PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/before_days"), 0.0, 9.0, 1.0, 1.0, 1.0);
	spin = gtk_spin_button_new(adjustment, 1.0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(spin, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)));
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spin_cb), PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/before_days");
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(pidgin_toggle_sensitive), spin);

	label = gtk_label_new(_("days before birthday"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	toggle = gtk_check_button_new_with_mnemonic(_("Show mini dialog in the buddy list"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/show"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/show");

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);

	adjustment = (GtkAdjustment *) gtk_adjustment_new(1.0 * purple_prefs_get_int(PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/before_days"), 0.0, 9.0, 1.0, 1.0, 1.0);
	spin = gtk_spin_button_new(adjustment, 1.0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(spin, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)));
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spin_cb), PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/before_days");
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(pidgin_toggle_sensitive), spin);

	label = gtk_label_new(_("days before birthday"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	toggle = gtk_check_button_new_with_mnemonic(_("Show notification"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/notification/show"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/reminder/notification/show");

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);

	adjustment = (GtkAdjustment *) gtk_adjustment_new(1.0 * purple_prefs_get_int(PLUGIN_PREFS_PREFIX "/reminder/notification/before_days"), 0.0, 9.0, 1.0, 1.0, 1.0);
	spin = gtk_spin_button_new(adjustment, 1.0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(spin, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)));
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spin_cb), PLUGIN_PREFS_PREFIX "/reminder/notification/before_days");
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(pidgin_toggle_sensitive), spin);

	label = gtk_label_new(_("days before birthday"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	toggle = gtk_check_button_new_with_mnemonic(_("Play sound"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/sound/play"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/reminder/sound/play");

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);

	adjustment = (GtkAdjustment *) gtk_adjustment_new(1.0 * purple_prefs_get_int(PLUGIN_PREFS_PREFIX "/reminder/sound/before_days"), 0.0, 9.0, 1.0, 1.0, 1.0);
	spin = gtk_spin_button_new(adjustment, 1.0, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(spin, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)));
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spin_cb), PLUGIN_PREFS_PREFIX "/reminder/sound/before_days");
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(pidgin_toggle_sensitive), spin);

	label = gtk_label_new(_("days before birthday"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

	/* Tooltip */
	frame = pidgin_make_frame(ret, _("Tooltip"));
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	toggle = gtk_check_button_new_with_mnemonic(_("Show Birthday"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_birthday"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/tooltip/show_birthday");

	ref = toggle;

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);

	toggle = gtk_check_button_new_with_mnemonic(_("Show days to birthday"));
	gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_days_to_birthday"));
	gtk_widget_set_sensitive(toggle, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref)));
	g_signal_connect(G_OBJECT(ref), "toggled", G_CALLBACK(pidgin_toggle_sensitive), toggle);
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/tooltip/show_days_to_birthday");

	toggle = gtk_check_button_new_with_mnemonic(_("Show Age"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_age"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/tooltip/show_age");

	/* ICS-Export */
	frame = pidgin_make_frame(ret, _("iCalendar Export"));
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(frame), vbox);

	toggle = gtk_check_button_new_with_mnemonic(_("Export to file"));
	gtk_box_pack_start(GTK_BOX(vbox), toggle, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/export/automatic"));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(toggle_cb), PLUGIN_PREFS_PREFIX "/export/automatic");

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);

	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	gtk_entry_set_text(GTK_ENTRY(entry), purple_prefs_get_path(PLUGIN_PREFS_PREFIX "/export/path"));
	gtk_widget_set_sensitive(entry, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(pidgin_toggle_sensitive), entry);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(entry_path_cb), PLUGIN_PREFS_PREFIX "/export/path");

	button = gtk_button_new_with_label("...");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	gtk_widget_set_sensitive(button, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)));
	g_signal_connect(G_OBJECT(toggle), "toggled", G_CALLBACK(pidgin_toggle_sensitive), button);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(export_filechooser_cb), entry);

	g_signal_connect(G_OBJECT(ret), "destroy", G_CALLBACK(window_destroyed_cb), NULL);

	gtk_widget_show_all(ret);
	return ret;
}

void init_prefs(void) {
	purple_prefs_connect_callback(plugin, PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/show", birthday_icon_pref_changed_cb, NULL);
	purple_prefs_connect_callback(plugin, PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/before_days", birthday_icon_pref_changed_cb, NULL);
}
