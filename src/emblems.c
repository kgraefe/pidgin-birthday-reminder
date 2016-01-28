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

#include "emblems.h"

#include "internal.h"

#include "birthday_reminder.h"

#include "birthday_access.h"
#include "pidgin_internals.h"

#include <glib.h>
#include <gtkblist.h>
#include <debug.h>

extern PurplePlugin *plugin;
PidginBuddyList *gtkblist = NULL;

static GdkPixbuf* birthday_icons[] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static int row_changed_handler_id = -1;

static void unload_birthday_emblems() {
	gint i;

	for(i = 0; i < 10; i++) {
		if(birthday_icons[i]) {
			g_object_unref(birthday_icons[i]);
		}
		birthday_icons[i] = NULL;
	}
}

static gboolean load_birthday_emblems() {
	gchar *filename, *file;
	gint i;

	for(i = 0; i < 10; i++) {
		filename = g_strdup_printf("birthday%d.png", i);
		file = g_build_filename(
			purple_user_dir(),
			"pixmaps",
			"pidgin",
			"birthday_reminder",
			filename,
			NULL
		);

		if(!g_file_test(file, G_FILE_TEST_EXISTS)) {
			g_free(file);
			file = g_build_filename(
				DATADIR,
				"pixmaps",
				"pidgin",
				"birthday_reminder",
				filename,
				NULL
			);
		}

		if(!g_file_test(file, G_FILE_TEST_EXISTS)) {
			unload_birthday_emblems();
			g_free(file);
			g_free(filename);

			return FALSE;
		}
		birthday_icons[i] = gdk_pixbuf_new_from_file(file, NULL);

		g_free(file);
		g_free(filename);
	}
	return TRUE;
}

static void row_changed_cb(
	GtkTreeModel *model, GtkTreePath *path,
	GtkTreeIter *iter, gpointer data
) {
	static gboolean inuse = FALSE;
	PurpleBlistNode *node;
	GdkPixbuf *emblem;

	if(inuse) {
		return;
	}

	gtk_tree_model_get(model, iter, NODE_COLUMN, &node, -1);

	if(
		!PURPLE_BLIST_NODE_IS_BUDDY(node) &&
		!PURPLE_BLIST_NODE_IS_CONTACT(node)
	) {
		return;
	}

	inuse = TRUE;

	emblem = get_birthday_icon_from_node(node, TRUE);
	if(!emblem) {
		emblem = pidgin_blist_get_emblem(node);
	}
	gtk_tree_store_set(
		GTK_TREE_STORE(model), iter,
		EMBLEM_COLUMN, emblem,
		EMBLEM_VISIBLE_COLUMN, (emblem != NULL),
		-1
	);

	inuse = FALSE;
}

void update_birthday_emblem(PurpleBlistNode *node) {
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = GTK_TREE_MODEL(gtkblist->treemodel);
	if(model && get_iter_from_node(node, &iter)) {
		row_changed_cb(model, NULL, &iter, NULL);
	}

	if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		update_birthday_emblem(node->parent);
	}
}

static void gtkblist_created_cb(PurpleBuddyList *blist) {
	gtkblist = PIDGIN_BLIST(blist);
	
	row_changed_handler_id = g_signal_connect(
		gtkblist->treemodel, "row_changed",
		G_CALLBACK(row_changed_cb),
		NULL
	);

	pidgin_blist_refresh(blist);
}

GdkPixbuf *get_birthday_icon_from_node(PurpleBlistNode *node, gboolean blist) {
	gint days_to_birthday;

	days_to_birthday = get_days_to_birthday_from_node(node);

	if(
		!purple_prefs_get_bool(
			PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/show"
		) &&
		blist
	) {
		return NULL;
	}

	if(
		days_to_birthday > purple_prefs_get_int(
			PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/before_days"
		) &&
		blist
	) {
		return NULL;
	}

	if(days_to_birthday > 9) {
		return NULL;
	}

	return birthday_icons[days_to_birthday];
}


gboolean init_birthday_emblems(void) {
	if(!load_birthday_emblems()) {
		purple_debug_error(PLUGIN_STATIC_NAME,
			_("Could not load icons!\n")
		);
		return FALSE;
	}

	purple_signal_connect(
		pidgin_blist_get_handle(), "gtkblist-created",
		plugin, PURPLE_CALLBACK(gtkblist_created_cb),
		NULL
	);
	if(pidgin_blist_get_default_gtk_blist()) {
		gtkblist_created_cb(purple_get_blist());
	}
	
	return TRUE;
}

void uninit_birthday_emblems(void) {
	if(
		g_signal_handler_is_connected(
			gtkblist->treemodel, row_changed_handler_id
		)
	) {
		g_signal_handler_disconnect(gtkblist->treemodel, row_changed_handler_id);
	}

	unload_birthday_emblems();
}

/* ex: set noexpandtab: */
