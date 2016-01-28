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

#include "birthday_list.h"

#include "internal.h"

#include <gtk/gtk.h>
#include <blist.h>
#include <pidginstock.h>

#include "birthday_reminder.h"
#include "functions.h"
#include "birthday_access.h"
#include "emblems.h"


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
        /* Nothing selected. Abort. */
        return;
    }

    gtk_tree_model_get(GTK_TREE_MODEL(birthday_list_window.model), &iter, BDAY_LIST_COL_BLIST_NODE, &node, -1);

    write_im(node);
}

static void column_header_clicked_cb(GtkTreeViewColumn *column, gpointer data) {
    if(!birthday_list_window.model) {
        return;
    }
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(birthday_list_window.model), GPOINTER_TO_INT(data), GTK_SORT_ASCENDING);
}

static void birthday_list_destroy_cb() {
    if(birthday_list_window.window) {
        gtk_widget_destroy(birthday_list_window.window);
    }
    if(birthday_list_window.model) {
        g_object_unref(G_OBJECT(birthday_list_window.model));
    }
    birthday_list_window.window = NULL;
    birthday_list_window.model = NULL;
}

void birthday_list_show(void) {
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
    g_signal_connect(
        G_OBJECT(window), "delete_event",
        G_CALLBACK(birthday_list_destroy_cb), &birthday_list_window
    );
    gtk_container_set_border_width(GTK_CONTAINER(window), 12);
    
    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, -1, 200);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scrolled_window),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC
    );
    gtk_scrolled_window_set_shadow_type(
        GTK_SCROLLED_WINDOW(scrolled_window),
        GTK_SHADOW_ETCHED_IN
    );
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    model = gtk_list_store_new(8,
                  GDK_TYPE_PIXBUF,  /* Birthday icon */
                  G_TYPE_STRING,    /* Buddy name */
                  G_TYPE_INT,       /* Days to birthday */
                  G_TYPE_STRING,    /* Birthday */
                  G_TYPE_INT,       /* Age */
                  G_TYPE_UINT,      /* Birthday as g_date_julian, this is not
                                     * shown, but used for to sort by birthday.
                                     */
                  G_TYPE_POINTER,   /* buddy list node */
                  G_TYPE_BOOLEAN    /* Show age? */
                 );
    birthday_list_window.model = model;

    gtk_tree_sortable_set_sort_column_id(
        GTK_TREE_SORTABLE(model),
        BDAY_LIST_COL_DAYS_TO_BIRTHDAY, GTK_SORT_ASCENDING
    );

    /* Fill list... */
    count_entries = 0;
    node=purple_blist_get_root();
    while(node) {
        if(PURPLE_BLIST_NODE_IS_CONTACT(node) || PURPLE_BLIST_NODE_IS_BUDDY(node)) {
            if(PURPLE_BLIST_NODE_IS_CONTACT(node)) {
                buddy = purple_contact_get_priority_buddy((PurpleContact *)node);
            } else {
                buddy = (PurpleBuddy *)node;
            }
            if(
                !PURPLE_BLIST_NODE_IS_CONTACT(node->parent) &&
                purple_account_is_connected(buddy->account)
            ) {
                get_birthday_from_node(node, &date);
                if(g_date_valid(&date)) {
                    if(g_date_get_year(&date) > 1900) {
                        birthday = g_strdup_printf(
                            /* Translators: This is in the birthday list. Use %2$02d for day, %1$02d for month and %3$04d for year. */
                            _("%02d/%02d/%04d"),
                            g_date_get_month(&date),
                            g_date_get_day(&date),
                            g_date_get_year(&date)
                        );
                    } else {
                        birthday = g_strdup_printf(
                            /* Translators: This is in the birthday list. Use %2$02d for day and %1$02d for month. */
                            _("%02d/%02d"),
                            g_date_get_month(&date),
                            g_date_get_day(&date)
                        );
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
                        -1
                    );
    
                    g_free(birthday);

                    count_entries++;
                }
            }
        }
        node = purple_blist_node_next(node, TRUE);
    }

    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
    birthday_list_window.treeview = treeview;

    /* Column "Buddy" */
    column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, _("Buddy"));
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(
        G_OBJECT(column), "clicked",
        G_CALLBACK(column_header_clicked_cb),
        GINT_TO_POINTER(BDAY_LIST_COL_ALIAS)
    );
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(
        column, renderer, "pixbuf", BDAY_LIST_COL_ICON
    );

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(
        column, renderer, "text", BDAY_LIST_COL_ALIAS
    );

    /* Column "Days to birthday" */
    column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, _("Days to birthday"));
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(
        G_OBJECT(column), "clicked",
        G_CALLBACK(column_header_clicked_cb),
        GINT_TO_POINTER(BDAY_LIST_COL_DAYS_TO_BIRTHDAY)
    );
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(
        column,
        renderer, "text",
        BDAY_LIST_COL_DAYS_TO_BIRTHDAY
    );

    /* Column "Birthday" */
    column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, _("Birthday"));
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(
        G_OBJECT(column), "clicked",
        G_CALLBACK(column_header_clicked_cb),
        GINT_TO_POINTER(BDAY_LIST_COL_BIRTHDAY_JULIAN)
    );
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(
        column,
        renderer, "text",
        BDAY_LIST_COL_BIRTHDAY
    );

    /* Column "Age" */
    column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(column, _("Age"));
    gtk_tree_view_column_set_clickable(column, TRUE);
    g_signal_connect(
        G_OBJECT(column), "clicked",
        G_CALLBACK(column_header_clicked_cb),
        GINT_TO_POINTER(BDAY_LIST_COL_AGE)
    );
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_add_attribute(
        column,
        renderer, "text",
        BDAY_LIST_COL_AGE
    );
    gtk_tree_view_column_add_attribute(
        column,
        renderer, "visible",
        BDAY_LIST_COL_AGE_VISIBLE
    );

    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));

    gtk_container_add(GTK_CONTAINER(scrolled_window), treeview);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    button = gtk_button_new_with_label(_("Close"));
    gtk_button_set_image(
        GTK_BUTTON(button),
        gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU)
    );
    gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    g_signal_connect(
        G_OBJECT(button), "clicked",
        G_CALLBACK(birthday_list_destroy_cb), NULL
    );

    button = gtk_button_new_with_label(_("Write IM"));
    gtk_button_set_image(
        GTK_BUTTON(button),
        gtk_image_new_from_stock(PIDGIN_STOCK_TOOLBAR_MESSAGE_NEW, GTK_ICON_SIZE_MENU)
    );
    gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    g_signal_connect(
        G_OBJECT(button), "clicked",
        G_CALLBACK(birthday_list_write_im_cb),
        NULL
    );

    window_title = g_strdup_printf(_("Birthday List (%d)"), count_entries);
    gtk_window_set_title(GTK_WINDOW(window), window_title);
    g_free(window_title);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_show_all(window);
}

void uninit_birthday_list(void) {
    birthday_list_destroy_cb();
}

/* ex: set noexpandtab: */
