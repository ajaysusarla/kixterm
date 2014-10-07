/*
 * kt-buffer.c
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

#include "kt-buffer.h"

typedef struct {
        KtBuffer buffer;
        gint refcount;
} KtBufferPriv;

G_DEFINE_BOXED_TYPE(KtBuffer, kt_buffer, kt_buffer_copy, kt_buffer_free)

KtBuffer * kt_buffer_new(gconstpointer data, gsize length)
{
        KtBufferPriv *priv = g_slice_new0(KtBufferPriv);


        data = g_memdup(data, length);
        priv->buffer.data = data;
        priv->buffer.length = length;
        priv->refcount = 1;

        return (KtBuffer *)priv;
}

KtBuffer * kt_buffer_copy(KtBuffer *buffer)
{
        KtBufferPriv *priv = (KtBufferPriv *)buffer;

        priv->refcount++;

        return buffer;
}

void kt_buffer_free(KtBuffer *buffer)
{
        KtBufferPriv *priv = (KtBufferPriv *)buffer;

        if (!--priv->refcount) {
                g_free((gpointer)priv->buffer.data);
                g_slice_free(KtBufferPriv, priv);
        }
}
