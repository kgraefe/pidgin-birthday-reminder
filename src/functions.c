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

#include "functions.h"

#include "internal.h"

#include <string.h>


void g_date_set_today(GDate *date) {
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

void write_im(PurpleBlistNode *node) {
	PurpleConversation *conv;
	PurpleBuddy *buddy;

	if(
		!PURPLE_BLIST_NODE_IS_CONTACT(node) &&
		!PURPLE_BLIST_NODE_IS_BUDDY(node)
	) {
		return;
	}
	
	if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		buddy = purple_contact_get_priority_buddy((PurpleContact *)node);
	} else {
		buddy = (PurpleBuddy *)node;
	}

	if(!buddy) {
		return;
	}

	conv = purple_find_conversation_with_account(
		PURPLE_CONV_TYPE_IM,
		buddy->name, buddy->account
	);

	if(!conv) {
		conv = purple_conversation_new(
			PURPLE_CONV_TYPE_IM,
			buddy->account, buddy->name
		);
	}

	purple_conversation_present(conv);
}

gboolean node_account_connected(PurpleBlistNode *node) {
	PurpleAccount *acc;
	PurpleBuddy *buddy;

	if(!node) {
		return FALSE;
	}

	if(PURPLE_BLIST_NODE_IS_BUDDY(node)) {
		buddy = (PurpleBuddy *)node;
	} else if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		buddy = purple_contact_get_priority_buddy((PurpleContact *)node);
	} else {
		return FALSE;
	}

	acc = purple_buddy_get_account(buddy);
	if(!acc) {
		return FALSE;
	}
	
	return purple_account_is_connected(acc);
}

gboolean has_file_extension(const char *filename, const char *ext) {
	int len, extlen;

	if(filename == NULL || *filename == '\0' || ext == NULL) {
		return 0;
	}

	extlen = strlen(ext);
	len = strlen(filename) - extlen;

	if(len < 0) {
		return 0;
	}

	return (strncasecmp(filename + len, ext, extlen) == 0);
}

GtkWidget *make_info_widget(gchar *markup, gchar *stock_id, gboolean indent) {
	GtkWidget *infobox, *label, *img, *align;

	if(!markup) {
		return NULL;
	}

	infobox = gtk_hbox_new(FALSE, 5);

	if(indent) {
		label = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(infobox), label, FALSE, FALSE, 10);
	}

	if(stock_id) {
		/* align img to the top of the space */
		align = gtk_alignment_new(0.5, 0, 0, 0);
		gtk_box_pack_start(GTK_BOX(infobox), align, FALSE, FALSE, 0);

		img = gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU);
		gtk_container_add(GTK_CONTAINER(align), img);
	}

	label = gtk_label_new(NULL);
	gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_box_pack_start(GTK_BOX(infobox), label, FALSE, FALSE, 0);

	return infobox;
}

/* ex: set noexpandtab: */
