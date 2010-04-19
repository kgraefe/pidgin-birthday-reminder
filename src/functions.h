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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "internal.h"

#include <glib.h>
#include <blist.h>
#include <gtk/gtk.h>

void g_date_set_today(GDate *date);

void write_im(PurpleBlistNode *node);

gboolean node_account_connected(PurpleBlistNode *node);

gboolean has_file_extension(const char *filename, const char *ext);

GtkWidget *make_info_widget(gchar *markup, gchar *stock_id, gboolean indent);

#endif
