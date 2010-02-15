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

#include <gtkblist.h>
#include <gtkutils.h>

#include "birthday_reminder.h"

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

GtkWidget *get_config_frame(PurplePlugin *plugin) {
	GtkWidget *ret = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *toggle = NULL;
	GtkWidget *spin = NULL;
	GtkAdjustment *adjustment = NULL;
	GtkWidget *label = NULL;
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

	gtk_widget_show_all(ret);
	return ret;
}

void init_prefs(void) {
	purple_prefs_connect_callback(plugin, PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/show", birthday_icon_pref_changed_cb, NULL);
	purple_prefs_connect_callback(plugin, PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/before_days", birthday_icon_pref_changed_cb, NULL);
}
