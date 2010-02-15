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

#include "birthday_reminder.h"

#include "internal.h"

#ifndef PURPLE_PLUGINS
#define PURPLE_PLUGINS
#endif

#include <gtkplugin.h>
#include <version.h>
#include <core.h>
#include <gtkblist.h>

#include "check.h"
#include "notification.h"
#include "emblems.h"
#include "tooltip.h"
#include "input.h"
#include "prefs.h"
#include "scan.h"
#include "birthday_list.h"
#include "plugin_actions.h"

PurplePlugin *plugin;


static gboolean core_quitting = FALSE;

static void core_quitting_cb() {
	core_quitting = TRUE;
}

static gboolean plugin_load(PurplePlugin *_plugin) {
	plugin = _plugin;

	purple_signal_connect(purple_get_core(), "quitting", plugin, core_quitting_cb, NULL);

	init_check();
	init_notification();
	if(!init_birthday_emblems()) return FALSE;
	tooltip_init();
	init_input();
	init_prefs();
	init_scan();

	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin) {
	uninit_scan();
	uninit_check();
	uninit_birthday_list();
	uninit_birthday_emblems();
	if(!core_quitting) pidgin_blist_refresh(purple_get_blist());

	return TRUE;
}


static PidginPluginUiInfo ui_info = {
	get_config_frame,
	0,   /* page_num (Reserved) */
	/* Padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,			/**< type           */
	PIDGIN_PLUGIN_TYPE,				/**< ui_requirement */
	0,								/**< flags          */
	NULL,							/**< dependencies   */
	PURPLE_PRIORITY_DEFAULT,		/**< priority       */

	PLUGIN_ID,						/**< id             */
	NULL,							/**< name           */
	PLUGIN_VERSION,					/**< version        */
	NULL,							/**  summary        */
				
	NULL,							/**  description    */
	PLUGIN_AUTHOR,					/**< author         */
	PLUGIN_WEBSITE,					/**< homepage       */

	plugin_load,					/**< load           */
	plugin_unload,					/**< unload         */
	NULL,							/**< destroy        */

	&ui_info,						/**< ui_info        */
	NULL,							/**< extra_info     */
	NULL,							/**< prefs_info     */
	plugin_actions,					/**< actions        */
	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static void init_plugin(PurplePlugin *plugin) {
	const char *str = "Birthday Reminder";
	gchar *plugins_locale_dir;

#ifdef ENABLE_NLS
	plugins_locale_dir = g_build_filename(purple_user_dir(), "locale", NULL);

	bindtextdomain(GETTEXT_PACKAGE, plugins_locale_dir);
	if(str == _(str)) {
		bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	}
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

	g_free(plugins_locale_dir);
#endif /* ENABLE_NLS */

	info.name        = _("Birthday Reminder");
	info.summary     = _("Reminds you of the birthday of your buddies. :o)");
	info.description = _("This Plugin shall keep track and remind you of your buddies' birthdays.");

	purple_prefs_add_none(PLUGIN_PREFS_PREFIX);

	purple_prefs_add_none(PLUGIN_PREFS_PREFIX "/reminder");
	
	purple_prefs_add_bool(PLUGIN_PREFS_PREFIX "/reminder/once_a_day", TRUE);

	purple_prefs_add_none(PLUGIN_PREFS_PREFIX "/reminder/birthday_icons");
	purple_prefs_add_bool(PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/show", TRUE);
	purple_prefs_add_int(PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/before_days", 3);

	purple_prefs_add_none(PLUGIN_PREFS_PREFIX "/reminder/mini_dialog");
	purple_prefs_add_bool(PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/show", TRUE);
	purple_prefs_add_int(PLUGIN_PREFS_PREFIX "/reminder/mini_dialog/before_days", 0);

	purple_prefs_add_none(PLUGIN_PREFS_PREFIX "/reminder/notification");
	purple_prefs_add_bool(PLUGIN_PREFS_PREFIX "/reminder/notification/show", FALSE);
	purple_prefs_add_int(PLUGIN_PREFS_PREFIX "/reminder/notification/before_days", 0);
	
	purple_prefs_add_none(PLUGIN_PREFS_PREFIX "/reminder/sound");
	purple_prefs_add_bool(PLUGIN_PREFS_PREFIX "/reminder/sound/play", TRUE);
	purple_prefs_add_int(PLUGIN_PREFS_PREFIX "/reminder/sound/before_days", 0);
	
	purple_prefs_add_none(PLUGIN_PREFS_PREFIX "/tooltip");

	purple_prefs_add_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_birthday", TRUE);
	purple_prefs_add_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_days_to_birthday", TRUE);

	purple_prefs_add_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_age", FALSE);
}

PURPLE_INIT_PLUGIN(PLUGIN_STATIC_NAME, init_plugin, info)

