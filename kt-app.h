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

typedef struct _KixtermApp KixtermApp;
typedef struct _KixtermAppClass KixtermAppClass;
typedef struct _KixtermAppPriv KixtermAppPriv;

#define KIXTERM_APP_TYPE (kixterm_app_get_type())
#define KIXTERM_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIXTERM_APP_TYPE, KixtermApp))
#define KIXTERM_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_BINARY_TREE, KixtermAppClass))
#define KIXTERM_IS_APP(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), KIXTERM_APP_TYPE))
#define KIXTERM_IS_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KIXTERM_APP_TYPE))
#define KIXTERM_APP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KIXTERM_APP_TYPE, KixtermAppClass))

struct _KixtermApp {
        GObject parent_instance;

        KixtermAppPriv *priv;
};

struct _KixtermAppClass {
        GObjectClass parent_class;
};

GType kixterm_app_get_type(void);
KixtermApp *kixterm_app_new(void);

xcb_connection_t *kixterm_app_get_connection(KixtermApp *app);
xcb_ewmh_connection_t *kixterm_app_get_ewmh_connection(KixtermApp *app);
xcb_screen_t *kixterm_app_get_screen(KixtermApp *app);
gint kixterm_app_get_default_screen(KixtermApp *app);
xcb_visualtype_t *kixterm_app_get_visual(KixtermApp *app);
gint kixterm_app_get_xfd(KixtermApp *app);
const gchar *kixterm_app_get_display_name(KixtermApp *app);
xcb_cursor_t kixterm_app_get_normal_cursor(KixtermApp *app);
xcb_cursor_t kixterm_app_get_hidden_cursor(KixtermApp *app);

G_END_DECLS
#endif /* KT_APP_H */
