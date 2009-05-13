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

// *includes* {
#include "config.h"

#ifndef PURPLE_PLUGINS
#define PURPLE_PLUGINS
#endif

#include "internal.h"

#include <plugin.h>
#include <version.h>
#include <debug.h>
#include <sound.h>
#include <core.h>
#include <accountopt.h>

#include <gtkblist.h>
#include <gtkutils.h>
#include <pidginstock.h>
#include <gtkplugin.h>

#include <notify.h>
#include <server.h>
#include <request.h>

#include <glib/gprintf.h>

// }

#define PLUGIN_PREFS_PREFIX "/plugins/gtk/birthday_reminder"
#define SCAN_BUDDIES_TIMEOUT_SECONDS 120

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

static PurplePlugin *plugin;

//<Original Pidgin-Code> {
static PidginBuddyList *gtkblist = NULL;
typedef enum {
	PIDGIN_BLIST_NODE_HAS_PENDING_MESSAGE    =  1 << 0,  /* Whether there's pending message in a conversation */
} PidginBlistNodeFlags;

typedef struct _pidgin_blist_node {
	GtkTreeRowReference *row;
	gboolean contact_expanded;
	gboolean recent_signonoff;
	gint recent_signonoff_timer;
	struct {
		PurpleConversation *conv;
		time_t last_message;	  /* timestamp for last displayed message */
		PidginBlistNodeFlags flags;
	} conv;
} PidginBlistNode;

static gboolean get_iter_from_node(PurpleBlistNode *node, GtkTreeIter *iter) {
	struct _pidgin_blist_node *gtknode = (struct _pidgin_blist_node *)node->ui_data;
	GtkTreePath *path;

	if (!gtknode) {
		return FALSE;
	}

	if (!gtkblist) {
		purple_debug_error(PLUGIN_STATIC_NAME, "get_iter_from_node was called, but we don't seem to have a blist\n")
;
		return FALSE;
	}

	if (!gtknode->row)
		return FALSE;


	if ((path = gtk_tree_row_reference_get_path(gtknode->row)) == NULL)
		return FALSE;

	if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(gtkblist->treemodel), iter, path)) {
		gtk_tree_path_free(path);
		return FALSE;
	}
	gtk_tree_path_free(path);
	return TRUE;
}

//</Original Pidgin-Code> }


/*** Hilfsfunktionen ***/
static void g_date_set_today(GDate *date) {
	#if !GLIB_CHECK_VERSION(2,10,0)
		/* works until 2038 */
		time_t ttime;
		GTime gtime;
		time(&ttime);
		gtime = (GTime)ttime;
	
		g_date_set_time(date, gtime);
	#else
		g_date_set_time_t(date, time(NULL));
	#endif
}

static void write_im(PurpleBlistNode *node) {
	PurpleConversation *conv;
	PurpleBuddy *buddy;

	if(!PURPLE_BLIST_NODE_IS_CONTACT(node) &&
	   !PURPLE_BLIST_NODE_IS_BUDDY(node)) return;
	
	if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		buddy = purple_contact_get_priority_buddy((PurpleContact *)node);
	} else {
		buddy = (PurpleBuddy *)node;
	}

	if(!buddy) return;

	conv = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, buddy->name, buddy->account);

	if(!conv) conv = purple_conversation_new(PURPLE_CONV_TYPE_IM, buddy->account, buddy->name);

 	purple_conversation_present(conv);
}

static gboolean node_account_connected(PurpleBlistNode *node) {
	PurpleAccount *acc;
	PurpleBuddy *buddy;

	if(!node) return FALSE;

	if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		buddy = (PurpleBuddy *)node;
	} else if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		buddy = purple_contact_get_priority_buddy((PurpleContact *)node);
	} else {
		return FALSE;
	}

	acc = purple_buddy_get_account(buddy);
	if(!acc) return FALSE;
	
	return purple_account_is_connected(acc);
}


/*** Zugriff auf gespeicherte Geburtsdaten ***/
static gint get_days_to_birthday_from_node(PurpleBlistNode *node);

static void get_birthday_from_node(PurpleBlistNode *node, GDate *date) {
	guint32 julian;
	gint min_days_to_birthday, days_to_birthday;
	PurpleBlistNode *n;

	if(!date) return;

	/* Bei Fehler ungültiges Datum zurück geben */
	g_date_clear(date, 1);

	if(!PURPLE_BLIST_NODE_IS_CONTACT(node) &&
	   !PURPLE_BLIST_NODE_IS_BUDDY(node)) return;
	
	if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		/* Alle Kinder durchgehen, den Buddy dessen Geburtstag als nächster kommt auswählen */
		/* Nicht sonderlich effizient, aber naja, Kontakte sind meist recht klein. :) */
		n = purple_blist_node_get_first_child(node);
		node = NULL;
		min_days_to_birthday = -1;
		while(n) {
			days_to_birthday = get_days_to_birthday_from_node(n);
			if(days_to_birthday != -1 && (days_to_birthday < min_days_to_birthday || min_days_to_birthday == -1) && PURPLE_BLIST_NODE_IS_BUDDY(n) && purple_account_is_connected(purple_buddy_get_account((PurpleBuddy *)n))) {
				min_days_to_birthday = days_to_birthday;
				node = n;
			}
			n = purple_blist_node_get_sibling_next(n);
		}
	}
	if(!node) return;

	julian=purple_blist_node_get_int(node, "birthday_julian");
	if(g_date_valid_julian(julian)) {
		g_date_set_julian(date, julian);
	}
}

static gint get_age_from_node(PurpleBlistNode *node) {
	GDate bday, today;
	gint age=0;

	get_birthday_from_node(node, &bday);
	if(g_date_valid(&bday)) {
		g_date_set_today(&today);

		age = g_date_get_year(&today) - g_date_get_year(&bday);

		g_date_set_year(&bday, g_date_get_year(&today));
		if(g_date_compare(&bday, &today) > 0) age--;
	}
	return age;
}

static gint get_days_to_birthday_from_node(PurpleBlistNode *node) {
	GDate bday, today;

	get_birthday_from_node(node, &bday);
	if(g_date_valid(&bday)) {
		g_date_set_today(&today);

		g_date_set_year(&bday, g_date_get_year(&today));
		if(g_date_compare(&bday, &today) < 0) g_date_add_years(&bday, 1);

		return g_date_days_between(&today, &bday);
	}
	return (-1);
}

static GdkPixbuf *get_birthday_icon_from_node(PurpleBlistNode *node, gboolean blist) {
	gint days_to_birthday;

	days_to_birthday = get_days_to_birthday_from_node(node);

	if(!purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/show") && blist) return NULL;

	if(days_to_birthday > purple_prefs_get_int(PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/before_days") && blist) return NULL;

	if(days_to_birthday > 9) return NULL;

	return birthday_icons[days_to_birthday];
}


/*** Geburtstag ausgeben -> Tooltip ***/
static void drawing_tooltip_cb(PurpleBlistNode *node, GString *str, gboolean full, void *data) {
	GDate date;
	gint days_to_birthday;

	get_birthday_from_node(node, &date);
	if(g_date_valid(&date)) {
		if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_birthday")) {
			if(g_date_get_year(&date) > 1900) {
				/* Translators: to use an other orde of the arguments use %2$02d for day, %1$02d for month and %3$04d for year */
				g_string_append_printf(str, _("\n<b>Birthday</b>: %02d/%02d/%04d"), g_date_get_month(&date), g_date_get_day(&date), g_date_get_year(&date));
			} else {
				/* Translators: use %2$02d for day and %1$02d for month */
				g_string_append_printf(str, _("\n<b>Birthday</b>: %02d/%02d"), g_date_get_month(&date), g_date_get_day(&date));
			}
	
			if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_days_to_birthday")) {
				days_to_birthday = get_days_to_birthday_from_node(node);
				if(days_to_birthday == 0) {
					g_string_append_printf(str, _(" (<b>Today!</b>)"));
				} else if(days_to_birthday == 1) {
					g_string_append_printf(str, _(" (<b>Tomorrow!</b>)"));
				} else {
					g_string_append_printf(str, _(" (in %d days)"), days_to_birthday);
				}
			}
		}

		if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/tooltip/show_age")) {
			if(g_date_get_year(&date) > 1900) {
				g_string_append_printf(str, _("\n<b>Age</b>: %d years"), get_age_from_node(node));
			}
		}
	}
}


/*** Geburtstag ausgeben -> Mini-Dialog ***/
static void birthday_list_show_cb(PurplePluginAction *action);
static GtkWidget *mini_dialog;

static void mini_dialog_write_im_cb(PurpleBlistNode *node) {
	write_im(node);
}

static void mini_dialog_overview_cb(PurpleBlistNode *node) {
	birthday_list_show_cb(NULL);
}

static void mini_dialog_close_cb(PurpleBlistNode *node) {}

/*** Geburtstag ausgeben -> Emblem ***/
static int row_changed_handler_id = -1;

static void unload_birthday_emblems() {
	gint i;

	for(i = 0; i < 10; i++) {
		if(birthday_icons[i]) g_object_unref(birthday_icons[i]);
		birthday_icons[i] = NULL;
	}
}

static gboolean load_birthday_emblems() {
	gchar *filename, *file;
	gint i;

	for(i = 0; i < 10; i++) {
		filename = g_strdup_printf("birthday%d.png", i);
		file = g_build_filename(purple_user_dir(), "pixmaps", "pidgin", "birthday_reminder", filename, NULL);
		if(!g_file_test(file, G_FILE_TEST_EXISTS)) {
			g_free(file);
			file = g_build_filename(DATADIR, "pixmaps", "pidgin", "birthday_reminder", filename, NULL);
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

static void row_changed_cb(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data) {
	static gboolean inuse = FALSE;
	PurpleBlistNode *node;
	GdkPixbuf *emblem;

	if(inuse) return;

	gtk_tree_model_get(model, iter, NODE_COLUMN, &node, -1);

	if(!PURPLE_BLIST_NODE_IS_BUDDY(node) && !PURPLE_BLIST_NODE_IS_CONTACT(node)) return;

	inuse = TRUE;

	emblem = get_birthday_icon_from_node(node, TRUE);
	if(!emblem) {
		emblem = pidgin_blist_get_emblem(node);
	}
	gtk_tree_store_set(GTK_TREE_STORE(model), iter, EMBLEM_COLUMN, emblem, EMBLEM_VISIBLE_COLUMN, (emblem != NULL), -1);

	inuse = FALSE;
}

static void update_birthday_emblem(PurpleBlistNode *node) {
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = GTK_TREE_MODEL(gtkblist->treemodel);
	if(model && get_iter_from_node(node, &iter)) row_changed_cb(model, NULL, &iter, NULL);

	if(PURPLE_BLIST_NODE_IS_BUDDY(node)) update_birthday_emblem(node->parent);
}

static void gtkblist_created_cb(PurpleBuddyList *blist) {
	gtkblist = PIDGIN_BLIST(blist);
	
	row_changed_handler_id = g_signal_connect(gtkblist->treemodel, "row_changed", G_CALLBACK(row_changed_cb), NULL);

	pidgin_blist_refresh(blist);
}


/*** Geburtstag überprüfen ***/
static GDate last_check;
static guint check_birthdays_timeout_handle;
static void *request_ui_handle;

static void check_birthdays(PurpleAccount *acc, PurpleBuddy *buddy) {
	PurpleBlistNode *node;
	PurpleBuddy *birthday_buddy = NULL;
	gint days_to_birthday;
	gint min_days_to_birthday = 10;
	gint notify_before_days;
	gint count_birthdays = 0;
	gchar *soundfile;

	gint play_sound_before_days;
	gint show_mini_dialog_before_days;
	gint show_notification_before_days;

	gchar *msg;
	const gchar *alias;
	gint age;
	GDate date;

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
		if(days_to_birthday >= 0 && days_to_birthday <= notify_before_days && node_account_connected(node) && (acc == NULL || buddy->account == acc)) {
			count_birthdays++;
			min_days_to_birthday = days_to_birthday;
			birthday_buddy = buddy;
		}
	} else {
		node=purple_blist_get_root();
		while(node) {
			if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
				days_to_birthday = get_days_to_birthday_from_node(node);
				buddy = (PurpleBuddy *)node;

				if(days_to_birthday >= 0 && days_to_birthday <= notify_before_days && node_account_connected(node) && (acc == NULL || buddy->account == acc)) {
					count_birthdays++;
					birthday_buddy = buddy;

					if(days_to_birthday < min_days_to_birthday) min_days_to_birthday = days_to_birthday;
				}
			}
			node=purple_blist_node_next(node, TRUE);
		}
	}

	if(count_birthdays) {
		if(min_days_to_birthday <= play_sound_before_days) {
			soundfile = g_build_filename(purple_user_dir(), "sounds", "pidgin", "birthday_reminder", "birthday.wav", NULL);
			if(!g_file_test(soundfile, G_FILE_TEST_EXISTS)) {
				g_free(soundfile);
				soundfile = g_build_filename(DATADIR, "sounds", "pidgin", "birthday_reminder", "birthday.wav", NULL);
			}
			if(g_file_test(soundfile, G_FILE_TEST_EXISTS)) {
				purple_sound_play_file(soundfile, NULL);
			} else {
				purple_debug_error(PLUGIN_STATIC_NAME, _("sound file (%s) does not exist.\n"), soundfile);
			}
			g_free(soundfile);
		}

		msg = NULL;
		if(min_days_to_birthday <= show_mini_dialog_before_days || min_days_to_birthday <= show_notification_before_days) {
			/* 
			 * Wenn schon ein Minidialog existiert sollen in dem neuen mehrere Geburtstage angekündigt werden.
			 * Dann kann es passieren, dass ein Kontakt mit "Es stehen Geburstage an" gemeldet wird - naja.
			 * (TODO: Kann man testen, ob die notification schon weggeklickt wurde?)
			 */
			if(count_birthdays == 1 && mini_dialog == NULL) {
				node = (PurpleBlistNode *)birthday_buddy;
				alias = purple_buddy_get_contact_alias(birthday_buddy);
				age = get_age_from_node(node);
				days_to_birthday = get_days_to_birthday_from_node(node);
		
				get_birthday_from_node(node, &date);
		
				if(days_to_birthday == 0) {
					if(g_date_get_year(&date) > 1900) {
						msg = g_strdup_printf(_("%s will be %d years old today! Hooray!"), alias, age);
					} else {
						msg = g_strdup_printf(_("It's %s's birthday today! Hooray!"), alias);
					}
				} else if(days_to_birthday == 1) {
					if(g_date_get_year(&date) > 1900) {
						msg = g_strdup_printf(_("%s will be %d years old tomorrow!"), alias, age + 1);
					} else {
						msg = g_strdup_printf(_("%s's birthday is tomorrow!"), alias);
					}
				} else {
					if(g_date_get_year(&date) > 1900) {
						msg = g_strdup_printf(_("%s will be %d years old in %d days!"), alias, age + 1, days_to_birthday);
					} else {
						msg = g_strdup_printf(_("%s's birthday is in %d days!"), alias, days_to_birthday);
					}
				}
				
			} else {
				msg = g_strdup(_("There are birthdays in the next days! Hooray!"));
			}
		}
		if(min_days_to_birthday <= show_notification_before_days) {
			if(request_ui_handle) purple_request_close(PURPLE_REQUEST_ACTION, request_ui_handle);

			if(count_birthdays == 1 && mini_dialog == NULL) {
				request_ui_handle = purple_request_action(plugin, _("Birthday Reminder"), msg, _("Write IM?"), 0, NULL, NULL, NULL, purple_buddy_get_contact(buddy), 2, _("Yes"), PURPLE_CALLBACK(mini_dialog_write_im_cb), _("No"), NULL);
			} else {
				request_ui_handle = purple_request_action(plugin, _("Birthday Reminder"), msg, _("Show overview?"), 0, NULL, NULL, NULL, NULL, 2, _("Yes"), PURPLE_CALLBACK(mini_dialog_overview_cb), _("No"), NULL);
			}
		}
		if(min_days_to_birthday <= show_mini_dialog_before_days) {
			if(count_birthdays == 1 && mini_dialog == NULL) {
				mini_dialog = pidgin_make_mini_dialog(NULL, PIDGIN_STOCK_DIALOG_INFO, _("Birthday Reminder"), msg, purple_buddy_get_contact(birthday_buddy), _("Close"), PURPLE_CALLBACK(mini_dialog_close_cb), _("Write IM"), PURPLE_CALLBACK(mini_dialog_write_im_cb), NULL);
			} else {
				if(mini_dialog) gtk_widget_destroy(mini_dialog);
				mini_dialog = pidgin_make_mini_dialog(NULL, PIDGIN_STOCK_DIALOG_INFO, _("Birthday Reminder"), msg, NULL, _("Close"), PURPLE_CALLBACK(mini_dialog_close_cb), _("Overview"), PURPLE_CALLBACK(mini_dialog_overview_cb), NULL);
			}
			g_signal_connect(G_OBJECT(mini_dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &mini_dialog);
			pidgin_blist_add_alert(mini_dialog);
			purple_blist_set_visible(TRUE);
			
		}
		if(msg) g_free(msg);
	}
}

static void check_birthdays_plugin_action_cb(PurplePluginAction *action) {
	check_birthdays(NULL, NULL);
	purple_blist_set_visible(TRUE);
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


/*** Geburtstage scannen ***/
static guint scan_buddies_timeout_handle;
static void *(*notify_userinfo_ori)(PurpleConnection *gc, const char *who, PurpleNotifyUserInfo *user_info);
static PurpleBuddy *current_scanned_buddy;

static void scan_buddy(PurpleBuddy *buddy) {
	PurpleAccount *acc = NULL;
	PurpleConnection *gc = NULL;
	PurplePlugin *prpl = NULL;
	PurplePluginProtocolInfo *prpl_info = NULL;
	const char *name = NULL;

	if(!buddy) return;
	
	acc = buddy->account;
	if(!acc) return;

	if(purple_utf8_strcasecmp(purple_account_get_protocol_id(acc), "prpl-icq")!=0 &&
	   purple_utf8_strcasecmp(purple_account_get_protocol_id(acc), "prpl-aim")!=0 &&
	   purple_utf8_strcasecmp(purple_account_get_protocol_id(acc), "prpl-msn")!=0 &&
	   purple_utf8_strcasecmp(purple_account_get_protocol_id(acc), "prpl-jabber")!=0) {
		return;
	}

	if(!purple_account_get_bool(acc, "birthday_scan_enabled", TRUE)) return;

	gc = acc->gc;
	if(!gc) return;
	
	prpl = purple_connection_get_prpl(gc);
	if(!prpl) return;
	
	prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);
	if(!prpl_info || !prpl_info->get_info) return;
	
	name = buddy->name;
	if(!name) return;

	current_scanned_buddy = buddy;
	prpl_info->get_info(gc, name);
}

static gboolean scan_next_buddy(gpointer data) {
	PurpleBlistNode *node;

	purple_timeout_remove(scan_buddies_timeout_handle);
	scan_buddies_timeout_handle = 0;

	current_scanned_buddy = NULL;
	node = purple_blist_get_root();
	while(node && !current_scanned_buddy) {
		if(PURPLE_BLIST_NODE_IS_BUDDY(node) &&
		   !purple_blist_node_get_int(node, "birthday_julian") &&
		   !purple_blist_node_get_bool(node, "birthday_scanned") &&
		   purple_account_is_connected(((PurpleBuddy *)node)->account)) {
		 	scan_buddy((PurpleBuddy *)node);
		}
		node = purple_blist_node_next(node, TRUE);
	}

	if(current_scanned_buddy) {
		/* Translators: use %1$s for the buddy's nickname, %2$s for the account name and %3$s for the protocol name. */
		purple_debug_info(PLUGIN_STATIC_NAME, _("Scanning buddy %s (Account: %s (%s)). Waiting for response...\n"), purple_buddy_get_name(current_scanned_buddy), purple_account_get_username(current_scanned_buddy->account), purple_account_get_protocol_name(current_scanned_buddy->account));
	} else {
		purple_debug_info(PLUGIN_STATIC_NAME, _("No more buddies to scan.\n"));
		scan_buddies_timeout_handle = purple_timeout_add_seconds(SCAN_BUDDIES_TIMEOUT_SECONDS, scan_next_buddy, NULL);
	}

	return FALSE;
}

static void displaying_userinfo_cb(PurpleAccount *account, const char *who, PurpleNotifyUserInfo *user_info, PurpleBuddy *_buddy){
	PurpleNotifyUserInfoEntry *e;
	PurpleBlistNode *node;
	PurpleBuddy *buddy;

	GList *l;
	GDate *date;

	if(!account) return;
	if(!who) return;

	buddy = purple_find_buddy(account, who);
	if(!buddy) return;
	node = (PurpleBlistNode *)buddy;

	purple_blist_node_set_bool(node, "birthday_scanned", TRUE);

	l=purple_notify_user_info_get_entries(user_info);
	while(l) {
		e = l->data;

		if(purple_utf8_strcasecmp(purple_notify_user_info_entry_get_label(e), dgettext("pidgin", "Birthday"))==0) {
			date = g_date_new();
			g_date_set_parse(date, purple_notify_user_info_entry_get_value(e));

			if(g_date_valid(date)) {
				purple_blist_node_set_int(node, "birthday_julian", g_date_get_julian(date));
				check_birthdays(NULL, buddy);
			}

			g_date_free(date);

			return;
		}

		l = l->next;
	}
}

static void *birthday_reminder_notify_userinfo(PurpleConnection *gc, const char *who, PurpleNotifyUserInfo *user_info) {
	PurpleBuddy *buddy;
	
	if(current_scanned_buddy && (current_scanned_buddy->account == gc->account && purple_utf8_strcasecmp(current_scanned_buddy->name, who)==0)) {
		buddy = current_scanned_buddy;
	} else {
		return notify_userinfo_ori(gc, who, user_info);
	}

	/* Translators: use %1$s for the buddy's nickname, %2$s for the account name and %3$s for the protocol name. */
	purple_debug_info(PLUGIN_STATIC_NAME, _("Buddy %s (Account: %s (%s)) scanned.\n"), purple_buddy_get_name(current_scanned_buddy), purple_account_get_username(current_scanned_buddy->account), purple_account_get_protocol_name(current_scanned_buddy->account));
	
	current_scanned_buddy = NULL;
	scan_buddies_timeout_handle = purple_timeout_add_seconds(SCAN_BUDDIES_TIMEOUT_SECONDS, scan_next_buddy, NULL);

	/* Info-Fenster unterdrücken */
	return NULL;
}


/*** Geburtstag eingeben ***/
static void do_set_bday_cb(PurpleBlistNode *node, const char* bday) {
	PurpleBlistNode *child;
	PurpleBuddy *buddy;
	GDate date, today;

	if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		/* Geburtstag für alle Buddies setzen */
		child = purple_blist_node_get_first_child(node);
		while(child) {
			do_set_bday_cb(child, bday);
			child = purple_blist_node_get_sibling_next(child);
		}
	}

	if(!PURPLE_BLIST_NODE_IS_BUDDY(node)) return;
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
	if(g_date_valid(&date) && g_date_compare(&date, &today) < 0 && g_date_get_year(&date) > 12) {
		purple_blist_node_set_int(node, "birthday_julian", g_date_get_julian(&date));
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

	if(!PURPLE_BLIST_NODE_IS_CONTACT(node) &&
	   !PURPLE_BLIST_NODE_IS_BUDDY(node)) return;
	
	g_date_set_dmy(&date, 24, 12, 1986);
	g_date_to_struct_tm(&date, &tm);
	helptext = g_strdup_printf(_("Use the following format: %s .\nBlank out the input field to clear the BDay.\nUse an year before 1900 if you do not know."), purple_date_format_short(&tm));


	get_birthday_from_node(node, &date);
	if(g_date_valid(&date)) {
		g_date_to_struct_tm(&date, &tm);
		old_bday = g_strdup_printf("%s", purple_date_format_short(&tm));
	}


	purple_request_input(plugin, _("Birthday Reminder"), _("Set Birthday:"), helptext, old_bday, FALSE, FALSE, NULL, _("OK"), PURPLE_CALLBACK(do_set_bday_cb), _("Cancel"), NULL, NULL, NULL, NULL, node);

	g_free(helptext);
	if(old_bday) g_free(old_bday);
}

static void extended_buddy_menu_cb(PurpleBlistNode *node, GList **menu) {
	if(!PURPLE_BLIST_NODE_IS_CONTACT(node) &&
	   !PURPLE_BLIST_NODE_IS_BUDDY(node)) return;
	
	*menu = g_list_append(*menu, purple_menu_action_new(_("Set Birthday"), PURPLE_CALLBACK(set_bday_cb), NULL, NULL));
}


/*** Einstellungen ***/
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

static GtkWidget *get_config_frame(PurplePlugin *plugin) {
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


/*** Geburtstags-Liste ***/
enum bday_list_colums {
	BDAY_LIST_COL_ICON,
	BDAY_LIST_COL_ALIAS,
	BDAY_LIST_COL_DAYS_TO_BIRTHDAY,
	BDAY_LIST_COL_BIRTHDAY,
	BDAY_LIST_COL_AGE,
	BDAY_LIST_COL_BIRTHDAY_JULIAN,
	BDAY_LIST_COL_BLIST_NODE,
	BDAY_LIST_COL_AGE_VISIBLE
};

typedef struct _birthday_list_window {
	GtkWidget *window;
	GtkWidget *treeview;

	GtkListStore *model;
} BirthdayListWindow;

static BirthdayListWindow birthday_list_window;

static void birthday_list_write_im_cb() {
	PurpleBlistNode *node;
	GtkTreeSelection *sel;
	GtkTreeIter iter;

	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(birthday_list_window.treeview));

	if(!gtk_tree_selection_get_selected(sel, NULL, &iter)) {
		/* nicht ausgewählt */
		return;
	}

	gtk_tree_model_get(GTK_TREE_MODEL(birthday_list_window.model), &iter, BDAY_LIST_COL_BLIST_NODE, &node, -1);

	write_im(node);
}

static void column_header_clicked_cb(GtkTreeViewColumn *column, gpointer data) {
	if(!birthday_list_window.model) return;
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(birthday_list_window.model), GPOINTER_TO_INT(data), GTK_SORT_ASCENDING);
}

static void birthday_list_destroy_cb() {
	if(birthday_list_window.window) gtk_widget_destroy(birthday_list_window.window);
	if(birthday_list_window.model) g_object_unref(G_OBJECT(birthday_list_window.model));
	birthday_list_window.window = NULL;
	birthday_list_window.model = NULL;
}

static void birthday_list_show_cb(PurplePluginAction *action) {
	GtkWidget *window = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *hbox = NULL;
	GtkWidget *button = NULL;
	GtkWidget *scrolled_window = NULL;
	
	GtkWidget *treeview = NULL;
	GtkTreeIter iter;
	GtkListStore *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	PurpleBlistNode *node;
	PurpleBuddy *buddy;
	gchar *birthday;
	GDate date;
	gint count_entries;

	gchar *window_title;
	
	if(birthday_list_window.window) {
		gtk_window_present(GTK_WINDOW(birthday_list_window.window));
		return;
	}

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	birthday_list_window.window = window;
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(birthday_list_destroy_cb), &birthday_list_window);
	gtk_container_set_border_width(GTK_CONTAINER(window), 12);
	
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_size_request(scrolled_window, -1, 200);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_ETCHED_IN);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

	model = gtk_list_store_new(8,
				  GDK_TYPE_PIXBUF,	/* Geburtstags-Icon */
				  G_TYPE_STRING,	/* Buddy-Name */
				  G_TYPE_INT,		/* Tage zum Geburtstag */
				  G_TYPE_STRING,	/* Geburtstag */
				  G_TYPE_INT,		/* Alter */
				  G_TYPE_UINT,		/* Geburtstag -> g_date_julian (zum Sortieren) */
				  G_TYPE_POINTER,	/* node */
				  G_TYPE_BOOLEAN	/* Alte sichtbar? */
				 );
	birthday_list_window.model = model;

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), BDAY_LIST_COL_DAYS_TO_BIRTHDAY, GTK_SORT_ASCENDING);

	/* Liste füllen... */
	count_entries = 0;
	node=purple_blist_get_root();
	while(node) {
		if(PURPLE_BLIST_NODE_IS_CONTACT(node) || PURPLE_BLIST_NODE_IS_BUDDY(node)) {
			if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
				buddy = purple_contact_get_priority_buddy((PurpleContact *)node);
			} else {
				buddy = (PurpleBuddy *)node;
			}
			if(!PURPLE_BLIST_NODE_IS_CONTACT(node->parent) && purple_account_is_connected(buddy->account)) {
				get_birthday_from_node(node, &date);
				if(g_date_valid(&date)) {
					if(g_date_get_year(&date) > 1900) {
						birthday = g_strdup_printf(_("%02d/%02d/%04d"), g_date_get_month(&date), g_date_get_day(&date), g_date_get_year(&date));
					} else {
						birthday = g_strdup_printf(_("%02d/%02d"), g_date_get_month(&date), g_date_get_day(&date));
					}

					gtk_list_store_append(model, &iter);
					gtk_list_store_set(model, &iter,
				   		BDAY_LIST_COL_ICON, get_birthday_icon_from_node(node, FALSE),
				   		BDAY_LIST_COL_ALIAS, purple_contact_get_alias((PurpleContact *)node),
				   		BDAY_LIST_COL_DAYS_TO_BIRTHDAY, get_days_to_birthday_from_node(node),
				   		BDAY_LIST_COL_BIRTHDAY, birthday,
				   		BDAY_LIST_COL_AGE, get_age_from_node(node),
						BDAY_LIST_COL_BIRTHDAY_JULIAN, g_date_get_julian(&date),
						BDAY_LIST_COL_BLIST_NODE, node,
						BDAY_LIST_COL_AGE_VISIBLE, (g_date_get_year(&date) > 1900),
				   		-1);
	
					g_free(birthday);

					count_entries++;
				}
			}
		}
		node = purple_blist_node_next(node, TRUE);
	}

	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	birthday_list_window.treeview = treeview;

	/* Buddy-Spalte */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Buddy"));
	gtk_tree_view_column_set_clickable(column, TRUE);
	g_signal_connect(G_OBJECT(column), "clicked", G_CALLBACK(column_header_clicked_cb), GINT_TO_POINTER(BDAY_LIST_COL_ALIAS));
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "pixbuf", BDAY_LIST_COL_ICON);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", BDAY_LIST_COL_ALIAS);

	/* Tage-zum-Geburtstag-Spalte */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Days to birthday"));
	gtk_tree_view_column_set_clickable(column, TRUE);
	g_signal_connect(G_OBJECT(column), "clicked", G_CALLBACK(column_header_clicked_cb), GINT_TO_POINTER(BDAY_LIST_COL_DAYS_TO_BIRTHDAY));
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", BDAY_LIST_COL_DAYS_TO_BIRTHDAY);

	/* Geburtstag-Spalte */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Birthday"));
	gtk_tree_view_column_set_clickable(column, TRUE);
	g_signal_connect(G_OBJECT(column), "clicked", G_CALLBACK(column_header_clicked_cb), GINT_TO_POINTER(BDAY_LIST_COL_BIRTHDAY_JULIAN));
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", BDAY_LIST_COL_BIRTHDAY);

	/* Alters-Spalte */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Age"));
	gtk_tree_view_column_set_clickable(column, TRUE);
	g_signal_connect(G_OBJECT(column), "clicked", G_CALLBACK(column_header_clicked_cb), GINT_TO_POINTER(BDAY_LIST_COL_AGE));
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", BDAY_LIST_COL_AGE);
	gtk_tree_view_column_add_attribute(column, renderer, "visible", BDAY_LIST_COL_AGE_VISIBLE);

	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));

	gtk_container_add(GTK_CONTAINER(scrolled_window), treeview);

	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	button = gtk_button_new_with_label(_("Close"));
	gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(birthday_list_destroy_cb), NULL);

	button = gtk_button_new_with_label(_("Write IM"));
	gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_stock(PIDGIN_STOCK_TOOLBAR_MESSAGE_NEW, GTK_ICON_SIZE_MENU));
	gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(birthday_list_write_im_cb), NULL);

	window_title = g_strdup_printf(_("Birthday List (%d)"), count_entries);
	gtk_window_set_title(GTK_WINDOW(window), window_title);
	g_free(window_title);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_widget_show_all(window);
}


/*** Plugin-Gerödel ***/
static gboolean core_quitting = FALSE;

static void core_quitting_cb() {
	core_quitting = TRUE;
}

static gboolean plugin_load(PurplePlugin *_plugin) {
	PurpleNotifyUiOps *ops;
	GList *iter;
	PurplePlugin *prpl;
	PurplePluginProtocolInfo *prpl_info;
	PurpleAccountOption *option;


	plugin = _plugin;

	check_birthdays_timeout_handle=0;
	scan_buddies_timeout_handle=0;
	g_date_clear(&last_check, 1);
	mini_dialog = NULL;
	request_ui_handle = NULL;

	if(!load_birthday_emblems()) {
		purple_debug_error(PLUGIN_STATIC_NAME, _("Could not load icons!\n"));
		return FALSE;
	}

	purple_signal_connect(purple_get_core(), "quitting", plugin, core_quitting_cb, NULL);

	purple_signal_connect(pidgin_blist_get_handle(), "gtkblist-created", plugin, PURPLE_CALLBACK(gtkblist_created_cb), NULL);
	if(pidgin_blist_get_default_gtk_blist()) gtkblist_created_cb(purple_get_blist());

	purple_signal_connect(purple_connections_get_handle(), "signed-on", plugin, PURPLE_CALLBACK(signed_on_cb), NULL);

	purple_signal_connect(purple_notify_get_handle(), "displaying-userinfo", plugin, PURPLE_CALLBACK(displaying_userinfo_cb), NULL);

	purple_signal_connect(pidgin_blist_get_handle(), "drawing-tooltip", plugin, PURPLE_CALLBACK(drawing_tooltip_cb), NULL);

	purple_signal_connect(purple_blist_get_handle(), "blist-node-extended-menu", plugin, PURPLE_CALLBACK(extended_buddy_menu_cb), NULL);

	ops = purple_notify_get_ui_ops();
	notify_userinfo_ori = ops->notify_userinfo;
	ops->notify_userinfo = birthday_reminder_notify_userinfo;

	scan_buddies_timeout_handle = purple_timeout_add_seconds(SCAN_BUDDIES_TIMEOUT_SECONDS, scan_next_buddy, NULL);

	/* Geburtstage prüfen, Mitternachtstimer aktivieren */
	check_birthdays_timer_cb(NULL);

	purple_prefs_connect_callback(plugin, PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/show", birthday_icon_pref_changed_cb, NULL);
	purple_prefs_connect_callback(plugin, PLUGIN_PREFS_PREFIX "/reminder/birthday_icons/before_days", birthday_icon_pref_changed_cb, NULL);

	/* Allen Accounts die Birthday-Scan-Option anhängen*/
	for (iter = purple_plugins_get_protocols(); iter; iter = iter->next) {
		prpl = iter->data;
		
		if(prpl && prpl->info) {
			prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(prpl);
			if(prpl_info && prpl->info->id && 
			   (purple_utf8_strcasecmp(prpl->info->id, "prpl-icq")==0 ||
			    purple_utf8_strcasecmp(prpl->info->id, "prpl-aim")==0 ||
			    purple_utf8_strcasecmp(prpl->info->id, "prpl-msn")==0 ||
			    purple_utf8_strcasecmp(prpl->info->id, "prpl-jabber")==0)) {
				option = purple_account_option_bool_new(_("Scan birthdays on this account"), "birthday_scan_enabled", TRUE);
				prpl_info->protocol_options = g_list_append(prpl_info->protocol_options, option);
			}
		}
	}

	return TRUE;
}

static gboolean plugin_unload(PurplePlugin *plugin) {
	if(check_birthdays_timeout_handle > 0) purple_timeout_remove(check_birthdays_timeout_handle);
	if(scan_buddies_timeout_handle > 0) purple_timeout_remove(scan_buddies_timeout_handle);

	if(g_signal_handler_is_connected(gtkblist->treemodel, row_changed_handler_id))
		g_signal_handler_disconnect(gtkblist->treemodel, row_changed_handler_id);
	
	birthday_list_destroy_cb();

	unload_birthday_emblems();

	if(!core_quitting) pidgin_blist_refresh(purple_get_blist());

	return TRUE;
}

static GList *plugin_actions(PurplePlugin *plugin, gpointer context) {
	GList *l;
	PurplePluginAction *action;

	l = NULL;

	action = purple_plugin_action_new(_("Check Birthdays"), check_birthdays_plugin_action_cb);
	l = g_list_append(l, action);

	action = purple_plugin_action_new(_("Show birthday list"), birthday_list_show_cb);
	l = g_list_append(l, action);

	return l;
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
	PURPLE_PLUGIN_STANDARD,				/**< type           */
	PIDGIN_PLUGIN_TYPE,				/**< ui_requirement */
	0,						/**< flags          */
	NULL,						/**< dependencies   */
	PURPLE_PRIORITY_DEFAULT,			/**< priority       */

	PLUGIN_ID,					/**< id             */
	NULL,						/**< name           */
	PLUGIN_VERSION,					/**< version        */
	NULL,						/**  summary        */
				
	NULL,						/**  description    */
	PLUGIN_AUTHOR,					/**< author         */
	PLUGIN_WEBSITE,					/**< homepage       */

	plugin_load,					/**< load           */
	plugin_unload,					/**< unload         */
	NULL,						/**< destroy        */

	&ui_info,					/**< ui_info        */
	NULL,						/**< extra_info     */
	NULL,						/**< prefs_info     */
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
