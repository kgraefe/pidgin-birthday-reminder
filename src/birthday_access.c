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

#include "birthday_access.h"

#include "internal.h"

#include "birthday_reminder.h"
#include "functions.h"


void get_birthday_from_node(PurpleBlistNode *node, GDate *date) {
	guint32 julian;
	gint min_days_to_birthday, days_to_birthday;
	PurpleBlistNode *n;

	if(!date) {
		return;
	}

	/* Return an invalid date on error */
	g_date_clear(date, 1);

	if(
		!PURPLE_BLIST_NODE_IS_CONTACT(node) &&
		!PURPLE_BLIST_NODE_IS_BUDDY(node)
	) {
		return;
	}
	
	if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		/* If the node is a contact, go through all children and find the buddy
		 * whos birthday occurs next.
		 */
		n = purple_blist_node_get_first_child(node);
		node = NULL;
		min_days_to_birthday = -1;
		while(n) {
			days_to_birthday = get_days_to_birthday_from_node(n);
			if(
				days_to_birthday != -1 &&
				(days_to_birthday < min_days_to_birthday || min_days_to_birthday == -1) &&
				PURPLE_BLIST_NODE_IS_BUDDY(n) &&
				purple_account_is_connected(purple_buddy_get_account((PurpleBuddy *)n))
			) {
				min_days_to_birthday = days_to_birthday;
				node = n;
			}
			n = purple_blist_node_get_sibling_next(n);
		}
	}
	if(!node) {
		return;
	}

	julian=purple_blist_node_get_int(node, "birthday_julian");
	if(g_date_valid_julian(julian)) {
		g_date_set_julian(date, julian);
	}
}

gboolean already_notified_today(PurpleBlistNode *node) {
	guint32 julian;
	GDate date, today;
	
	g_date_set_today(&today);
	
	if(
		!PURPLE_BLIST_NODE_IS_CONTACT(node) &&
		!PURPLE_BLIST_NODE_IS_BUDDY(node)
	) {
		return FALSE;
	}
	
	if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
		node = purple_blist_node_get_first_child(node);
		while(node) {
			julian = purple_blist_node_get_int(node, "last_birthday_notification_julian");
			if(g_date_valid_julian(julian)) {
				g_date_set_julian(&date, julian);
				
				if(g_date_compare(&date, &today) == 0) {
					return TRUE;
				}
			}
			
			node = purple_blist_node_get_sibling_next(node);
		}
	} else {
		julian = purple_blist_node_get_int(node, "last_birthday_notification_julian");
		if(g_date_valid_julian(julian)) {
			g_date_set_julian(&date, julian);
			
			if(g_date_compare(&date, &today) == 0) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

gint get_age_from_node(PurpleBlistNode *node) {
	GDate bday, today;
	gint age=0;

	get_birthday_from_node(node, &bday);
	if(g_date_valid(&bday)) {
		g_date_set_today(&today);

		age = g_date_get_year(&today) - g_date_get_year(&bday);

		g_date_set_year(&bday, g_date_get_year(&today));
		if(g_date_compare(&bday, &today) > 0) {
			age--;
		}
	}
	return age;
}

gint get_days_to_birthday_from_node(PurpleBlistNode *node) {
	GDate bday, today;

	get_birthday_from_node(node, &bday);
	if(g_date_valid(&bday)) {
		g_date_set_today(&today);
		
		g_date_add_years(&bday, g_date_get_year(&today) - g_date_get_year(&bday));

		if(g_date_compare(&bday, &today) < 0) {
			g_date_add_years(&bday, 1);
		}

		return g_date_days_between(&today, &bday);
	}
	return (-1);
}

/* ex: set noexpandtab: */
