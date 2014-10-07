/*
 * kt-buffer.h
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

#ifndef KT_BUFFER_H
#define KT_BUFFER_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct {
        const char *data;
        gsize length;
} KtBuffer;

GType kt_buffer_get_type (void);
#define KT_TYPE_BUFFER (kt_buffer_get_type())

KtBuffer *kt_buffer_new(gconstpointer data, gsize length);
KtBuffer * kt_buffer_copy(KtBuffer *buffer);
void kt_buffer_free(KtBuffer *buffer);

G_END_DECLS
#endif /* KT_BUFFER_H */
