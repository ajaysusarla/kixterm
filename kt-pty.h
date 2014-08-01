/*
 * kt-pty.h
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
#ifndef KT_PTY_H
#define KT_PTY_H

#include <glib-object.h>

#include <xcb/xcb.h>

#include "kt-prefs.h"

typedef struct _KtPty KtPty;
typedef struct _KtPtyClass KtPtyClass;
typedef struct _KtPtyPriv KtPtyPriv;

#define KT_PTY_TYPE (kt_pty_get_type())
#define KT_PTY(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KT_PTY_TYPE, KtPty))
#define KT_PTY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_BINARY_TREE, KtPtyClass))
#define KT_IS_PTY(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), KT_PTY_TYPE))
#define KT_IS_PTY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KT_PTY_TYPE))
#define KT_PTY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KT_PTY_TYPE, KtPtyClass))

struct _KtPty {
        GObject parent_instance;

        /* <private> */
        KtPtyPriv *priv;
};

struct _KtPtyClass {
        GObjectClass parent_class;
};

GType kt_pty_get_type(void);
KtPty *kt_pty_new(KtPrefs *prefs, xcb_window_t wid);

gint kt_pty_get_fd(KtPty *pty);
gboolean kt_pty_spawn(KtPty *pty, gchar **args);
gboolean kt_pty_set_size(KtPty *pty, gint rows, gint cols);
GPid kt_pty_get_child_pid(KtPty *pty);
#endif /* KT_PTY_H */
