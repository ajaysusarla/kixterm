/*
 * kt-color.h
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

#ifndef KT_COLOR_H
#define KT_COLOR_H

#include <glib-object.h>

#include "kt-app.h"
#include "kt-prefs.h"

G_BEGIN_DECLS

typedef struct _KtColor KtColor;
typedef struct _KtColorClass KtColorClass;
typedef struct _KtColorPriv KtColorPriv;

#define KT_COLOR_TYPE (kt_color_get_type())
#define KT_COLOR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KT_COLOR_TYPE, KtColor))
#define KT_COLOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_BINARY_TREE, KtColorClass))
#define KT_IS_COLOR(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), KT_COLOR_TYPE))
#define KT_IS_COLOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KT_COLOR_TYPE))
#define KT_COLOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KT_COLOR_TYPE, KtColorClass))

struct _KtColor {
        GObject parent_instance;

        /* <private> */
        KtColorPriv *priv;
};

struct _KtColorClass {
        GObjectClass parent_class;
};

GType kt_color_get_type(void);
KtColor *kt_color_new(KtApp *app, KtPrefs *prefs);

guint32 kt_color_get_bg_pixel(KtColor *color);
guint32 kt_color_get_vb_pixel(KtColor *color);

G_END_DECLS

#endif /* KT_COLOR_H */
