/*
 * kt-font.h
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
#ifndef KT_FONT_H
#define KT_FONT_H

#include <glib-object.h>

#include <pango/pango-font.h>

#include "kt-app.h"
#include "kt-prefs.h"

typedef struct _KtFont KtFont;
typedef struct _KtFontClass KtFontClass;
typedef struct _KtFontPriv KtFontPriv;

#define KT_FONT_TYPE (kt_font_get_type())
#define KT_FONT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KT_FONT_TYPE, KtFont))
#define KT_FONT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_BINARY_TREE, KtFontClass))
#define KT_IS_FONT(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), KT_FONT_TYPE))
#define KT_IS_FONT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KT_FONT_TYPE))
#define KT_FONT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KT_FONT_TYPE, KtFontClass))

struct _KtFont {
        GObject parent_instance;

        /* <private> */
        KtFontPriv *priv;
};

struct _KtFontClass {
        GObjectClass parent_class;
};

GType kt_font_get_type(void);
KtFont *kt_font_new(KtApp *app, KtPrefs *prefs);

void kt_font_get_size(KtFont *font, gint *width, gint *height);
#endif /* KT_FONT_H */
