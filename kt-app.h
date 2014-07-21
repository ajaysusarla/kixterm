/*
 * kt-app.h
 *
 * Part of the kixterm project.
 *
 * Copyright © 2014 Partha Susarla <ajaysusarla@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef KT_APP_H
#define KT_APP_H

#include <glib-object.h>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_keysyms.h>

G_BEGIN_DECLS

typedef struct _KtApp KtApp;
typedef struct _KtAppClass KtAppClass;
typedef struct _KtAppPriv KtAppPriv;

#define KT_APP_TYPE (kt_app_get_type())
#define KT_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KT_APP_TYPE, KtApp))
#define KT_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_BINARY_TREE, KtAppClass))
#define KT_IS_APP(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), KT_APP_TYPE))
#define KT_IS_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KT_APP_TYPE))
#define KT_APP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KT_APP_TYPE, KtAppClass))

struct _KtApp {
        GObject parent_instance;

        KtAppPriv *priv;
};

struct _KtAppClass {
        GObjectClass parent_class;
};

GType kt_app_get_type(void);
KtApp *kt_app_new(void);

xcb_connection_t *kt_app_get_x_connection(KtApp *app);
xcb_ewmh_connection_t *kt_app_get_ewmh_connection(KtApp *app);
xcb_screen_t *kt_app_get_screen(KtApp *app);
gint kt_app_get_default_screen(KtApp *app);
xcb_visualtype_t *kt_app_get_visual(KtApp *app);
gint kt_app_get_xfd(KtApp *app);
const gchar *kt_app_get_display_name(KtApp *app);
xcb_cursor_t kt_app_get_normal_cursor(KtApp *app);
xcb_cursor_t kt_app_get_hidden_cursor(KtApp *app);

G_END_DECLS
#endif /* KT_APP_H */
