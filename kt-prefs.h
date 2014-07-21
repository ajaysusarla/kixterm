/*
 * kt-prefs.h
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
#ifndef KT_PREFS_H
#define KT_PREFS_H

#include <glib-object.h>

#include "kt-color.h"

G_BEGIN_DECLS

typedef struct _KtPrefs KtPrefs;
typedef struct _KtPrefsClass KtPrefsClass;
typedef struct _KtPrefsPriv KtPrefsPriv;

#define KT_PREFS_TYPE (kt_prefs_get_type())
#define KT_PREFS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KT_PREFS_TYPE, KtPrefs))
#define KT_PREFS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_BINARY_TREE, KtPrefsClass))
#define KT_IS_PREFS(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), KT_PREFS_TYPE))
#define KT_IS_PREFS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KT_PREFS_TYPE))
#define KT_PREFS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KT_PREFS_TYPE, KtPrefsClass))

struct _KtPrefs {
        GObject parent_instance;

        /* Preferences */

        /* Defaults */
        gchar *title;

        gint16 xpos;
        gint16 ypos;
        guint16 rows;
        guint16 cols;
        gint sb_width; /* Scroll bar width */
        gint bd_width; /* Border width */

        /* Font information */
        gchar *font_name;
        gint font_size;

        /* Colours */
        kt_color_t fg_color;
        kt_color_t bg_color;

        KtPrefsPriv *priv;
};

struct _KtPrefsClass {
        GObjectClass parent_class;
};

GType kt_prefs_get_type(void);
KtPrefs *kt_prefs_new(void);

G_END_DECLS

#endif /* KT_PREFS_H */
