/*
 * kt-terminal.h
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
#ifndef KT_TERMINAL_H
#define KT_TERMINAL_H

#include <glib-object.h>

#include <xcb/xcb.h>

#include "kt-prefs.h"

G_BEGIN_DECLS

typedef struct _KtTerminal KtTerminal;
typedef struct _KtTerminalClass KtTerminalClass;
typedef struct _KtTerminalPriv KtTerminalPriv;

#define KT_TERMINAL_TYPE (kt_terminal_get_type())
#define KT_TERMINAL(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KT_TERMINAL_TYPE, KtTerminal))
#define KT_TERMINAL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_BINARY_TREE, KtTerminalClass))
#define KT_IS_TERMINAL(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), KT_TERMINAL_TYPE))
#define KT_IS_TERMINAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KT_TERMINAL_TYPE))
#define KT_TERMINAL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KT_TERMINAL_TYPE, KtTerminalClass))

struct _KtTerminal {
        GObject parent_instance;

        /* <private> */
        KtTerminalPriv *priv;
};

struct _KtTerminalClass {
        GObjectClass parent_class;

        /* signals */
        void (*copy)(KtTerminal *terminal);
        void (*paste)(KtTerminal *terminal);
        void (*child_exited)(KtTerminal *terminal, int status);
};

GType kt_terminal_get_type(void);
KtTerminal *kt_terminal_new(KtPrefs *prefs, xcb_window_t wid);

G_END_DECLS

#endif /* KT_TERMINAL_H */
