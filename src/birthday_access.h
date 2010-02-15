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

#ifndef BIRTHDAY_ACCESS_H
#define BIRTHDAY_ACCESS_H

#include "internal.h"

#include <glib.h>
#include <blist.h>

gint get_days_to_birthday_from_node(PurpleBlistNode *node);

void get_birthday_from_node(PurpleBlistNode *node, GDate *date); 

gboolean already_notified_today(PurpleBlistNode *node);

gint get_age_from_node(PurpleBlistNode *node);

gint get_days_to_birthday_from_node(PurpleBlistNode *node);

#endif /* BIRTHDAY_ACCESS_H */

