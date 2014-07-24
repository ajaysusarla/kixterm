/*
 * kt-terminal.c
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

#include "kt-terminal.h"

struct _KtTerminalPriv {
        gboolean ignore;
};
G_DEFINE_TYPE(KtTerminal, kt_terminal, G_TYPE_OBJECT);

/* Private methods */

/* Class methods */
static void kt_terminal_finalize(GObject *object)
{

        G_OBJECT_CLASS(kt_terminal_parent_class)->finalize(object);
}

static void kt_terminal_class_init(KtTerminalClass *klass)
{
        GObjectClass *oclass = G_OBJECT_CLASS(klass);

        oclass->finalize = kt_terminal_finalize;

        g_type_class_add_private(klass, sizeof(KtTerminalPriv));
}

static void kt_terminal_init(KtTerminal *term)
{
        term->priv = G_TYPE_INSTANCE_GET_PRIVATE(term,
                                                 KT_TERMINAL_TYPE,
                                                 KtTerminalPriv);
}

/* Public methods */
KtTerminal *kt_terminal_new(void)
{
        KtTerminal *terminal = NULL;

        terminal = g_object_new(KT_TERMINAL_TYPE, NULL);

        return terminal;
}
