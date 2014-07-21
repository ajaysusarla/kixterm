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

G_BEGIN_DECLS

/* Custom color structure. Storing colors as individual R, B, G is convenient
   to use when working with pango/cairo.*/
typedef struct {
        guint8 r;
        guint8 g;
        guint8 b;
} kt_color_t;

#define INIT_COLOR(x)                          \
        do {                                   \
                x.r = 0;                       \
                x.g = 0;                       \
                x.b = 0;                       \
        } while (0)

G_END_DECLS

#endif /* KT_COLOR_H */
