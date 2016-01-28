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

#ifndef EMBLEMS_H
#define EMBLEMS_H

#include "internal.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <blist.h>

gboolean init_birthday_emblems(void);

void uninit_birthday_emblems(void);

void update_birthday_emblem(PurpleBlistNode *node);

GdkPixbuf *get_birthday_icon_from_node(PurpleBlistNode *node, gboolean blist);

#endif /* EMBLEMS_H */

/* ex: set noexpandtab: */
