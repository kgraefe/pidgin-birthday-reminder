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

#include "functions.h"

#include "internal.h"


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

gboolean node_account_connected(PurpleBlistNode *node) {
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
