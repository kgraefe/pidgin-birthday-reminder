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

#include "icsexport.h"

#include "internal.h"
#include "birthday_reminder.h"

#include <prefs.h>
#include <debug.h>

#include <stdio.h>


#include "birthday_access.h"
#include "hash.h"

typedef struct _ics_birthday {
	gchar *summary;
	gchar *description;
	gchar *date;
	gchar *uid;
} ICSBirthday;


static void print_header(FILE *fd) {
	fprintf(fd, "BEGIN:VCALENDAR\n");
	fprintf(fd, "PRODID:-//pidginbirthdayical//EN\n");
	fprintf(fd, "CALSCALE:GREGORIAN\n");
}

static void print_footer(FILE *fd) {
	fprintf(fd, "END:VCALENDAR\n");
}

static guint64 generate_uid(const gchar* name, const GDate *date) {
	guint64 ret;
	gchar *str;

	str = g_strdup_printf(
		"%s%i%i%i",
		name,
		g_date_get_year(date),
		g_date_get_month(date),
		g_date_get_day(date)
	);

	ret = hash(str);

	g_free(str);

	return ret;
}

static void ics_birthday_destroy(gpointer data) {
	ICSBirthday *bday;

	bday = (ICSBirthday *) data;

	g_free(bday->summary);
	g_free(bday->description);
	g_free(bday->date);
	g_free(bday->uid);

	g_free(bday);
}

static void print_ics_birthday(gpointer key, gpointer value, gpointer user_data) {
	ICSBirthday *bday;
	FILE *fd;

	bday = (ICSBirthday *)value;
	fd = (FILE *)user_data;

	fprintf(fd, "BEGIN:VEVENT\n");
	fprintf(fd, "DTSTART;VALUE=DATE:%s\n", bday->date);
	fprintf(fd, "SUMMARY:%s\n", bday->summary);
	fprintf(fd, "UID:%s\n", bday->uid);
	fprintf(fd, "RRULE:FREQ=YEARLY\n");
	fprintf(fd, "DESCRIPTION:%s\n", bday->description);
	fprintf(fd, "END:VEVENT\n");
}

void automatic_export(void) {
	const gchar *path;

	if(purple_prefs_get_bool(PLUGIN_PREFS_PREFIX "/export/automatic")) {
		path = purple_prefs_get_path(PLUGIN_PREFS_PREFIX "/export/path");

		icsexport(path);
	}
}

void icsexport(const gchar *path) {
	GHashTable *birthdays;
	ICSBirthday *birthday=NULL;

	FILE *fd;
	gchar line[256], buf[256];

	PurpleBlistNode *node;
	PurpleBuddy *buddy;
	GDate date;
	guint64 uid;
	gchar *struid;
	
	birthdays = g_hash_table_new_full(
		g_str_hash, g_str_equal, NULL, ics_birthday_destroy
	);


	/* We load the file of our last export first, so that birthdays
	 * of offline or deleted accounts are not lost.
	 */

	if((fd = fopen(path, "r")) != NULL) {
		birthday = NULL;
		while(fgets(line, sizeof(line), fd)) {
			if(birthday && purple_utf8_strcasecmp(line, "END:VEVENT\n") == 0) {
				if(
					birthday->summary &&
					birthday->description &&
					birthday->date && birthday->uid
				) {
					g_hash_table_insert(birthdays, birthday->uid, birthday);
				} else {
					ics_birthday_destroy(birthday);
				}
				birthday = NULL;
			}
			if(purple_utf8_strcasecmp(line, "BEGIN:VEVENT\n") == 0) {
				if(birthday) {
					ics_birthday_destroy(birthday);
				}
				birthday = g_malloc(sizeof(ICSBirthday));
				birthday->summary = NULL;
				birthday->description = NULL;
				birthday->date = NULL;
				birthday->uid = NULL;
			}

			if(
				birthday &&
				sscanf(line, "DTSTART;VALUE=DATE:%256[^\n]\n", buf) == 1
			) {
				birthday->date = g_strdup(buf);
			}

			if(
				birthday &&
				sscanf(line, "SUMMARY:%256[^\n]\n", buf) == 1
			) {
				birthday->summary = g_strdup(buf);
			}

			if(
				birthday &&
				sscanf(line, "DESCRIPTION:%256[^\n]\n", buf) == 1
			) {
				birthday->description = g_strdup(buf);
			}

			if(birthday && sscanf(line, "UID:%256[^\n]\n", buf)==1) {
				birthday->uid = g_strdup(buf);
			}
		}

		fclose(fd);
	}

	node = purple_blist_get_root();
	while(node) {
		if(
			PURPLE_BLIST_NODE_IS_CONTACT(node) ||
			PURPLE_BLIST_NODE_IS_BUDDY(node)
		) {
			if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
				buddy = purple_contact_get_priority_buddy((PurpleContact *)node);
			} else {
				buddy = (PurpleBuddy *)node;
			}

			if(!PURPLE_BLIST_NODE_IS_CONTACT(node->parent)) {
				get_birthday_from_node(node, &date);
				if(g_date_valid(&date)) {
					struid = g_strdup(
						purple_blist_node_get_string(node, "birthday_id")
					);
					if(struid == NULL) {
						uid = generate_uid(
							purple_buddy_get_name(buddy), &date
						);
						struid = g_strdup_printf("%" G_GUINT64_FORMAT, uid);
						purple_blist_node_set_string(node,
							"birthday_id", struid
						);
					}

					birthday = g_malloc(sizeof(ICSBirthday));

					birthday->summary = g_strdup_printf(
						/* Translators: this is how the birthday appears in an external calendar application (summary). Please display the name in front if possible. */
						_("%s's birthday"),
						purple_contact_get_alias((PurpleContact *)node)
					);

					if(g_date_get_year(&date) > 1900) {
						birthday->description = g_strdup_printf(
							/* Translators: this is how the birthday appears in an external calendar application (description with year of birth) */
							_("Birthday of %s, born in %04i"),
							purple_contact_get_alias(
								(PurpleContact *)node),
								g_date_get_year(&date)
							);
					} else {
						birthday->description = g_strdup_printf(
							/* Translators: this is how the birthday appears in an external calendar application (description without year of birth) */
							_("Birthday of %s"),
							purple_contact_get_alias((PurpleContact *)node)
						);
					}

					birthday->date = g_strdup_printf(
						"%04i%02i%02i",
						g_date_get_year(&date),
						g_date_get_month(&date),
						g_date_get_day(&date)
					);
					/* struid will be free'd within ics_birthday_destroy() */
					birthday->uid = struid;

					/* This replaces (and properly destroys) duplicate
					 * birthdays that might have been loaded from the file.
					 */
					g_hash_table_insert(birthdays, birthday->uid, birthday);
				}
			}
		}
		node = purple_blist_node_next(node, TRUE);
	}

	fd = fopen(path, "w+");

	print_header(fd);

	g_hash_table_foreach(birthdays, print_ics_birthday, fd);
	g_hash_table_destroy(birthdays);

	print_footer(fd);
	
	fclose(fd);
}

/* ex: set noexpandtab: */
