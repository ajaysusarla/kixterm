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

typedef struct _KixtermApp KixtermApp;
typedef struct _KixtermAppClass KixtermAppClass;

#define KIXTERM_APP_TYPE (kixterm_app_get_type())
#define KIXTERM_APP(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KIXTERM_APP_TYPE, KixtermApp))
#define KIXTERM_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_BINARY_TREE, KixtermAppClass))
#define KIXTERM_IS_APP(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), KIXTERM_APP_TYPE))
#define KIXTERM_IS_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KIXTERM_APP_TYPE))
#define KIXTERM_APP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KIXTERM_APP_TYPE, KixtermAppClass))

struct _KixtermApp {
        GObject parent_instance;
};

struct _KixtermAppClass {
        GObjectClass parent_class;
};

GType kixterm_app_get_type(void);
#endif /* KT_APP_H */
