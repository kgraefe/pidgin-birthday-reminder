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

#include "hash.h"

#define MODULO (G_MAXUINT64 / G_GINT64_CONSTANT(128) - G_GINT64_CONSTANT(256))

guint64 hash(gchar *key) {
	guint64 h = 0UL;
	
	while(*key) {
		h = (G_GINT64_CONSTANT(128) * h + (guint64) *key) % MODULO;
		key++;
	}

	return h;
}

gulong rehash(gulong hash) {
	return hash;
}

/* ex: set noexpandtab: */
