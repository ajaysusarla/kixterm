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
#include "kt-pty.h"

struct _KtTerminalPriv {
        KtPty *pty;

        /* Properties */
        KtPrefs *prefs;
};

enum {
        PROP_0,
        PROP_KT_PREFS,
};

G_DEFINE_TYPE(KtTerminal, kt_terminal, G_TYPE_OBJECT);

/* Private methods */

/* Class methods */
static void kt_terminal_get_property(GObject *obj,
                                     guint param_id,
                                     GValue *value,
                                     GParamSpec *pspec)
{
        KtTerminal *term = KT_TERMINAL(obj);
        KtTerminalPriv *priv = term->priv;

        switch(param_id) {
        case PROP_KT_PREFS:
                g_value_set_object(value, priv->prefs);
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
                break;
        }
}

static void kt_terminal_set_property(GObject *obj,
                                     guint param_id,
                                     const GValue *value,
                                     GParamSpec *pspec)
{
        KtTerminal *term = KT_TERMINAL(obj);
        KtTerminalPriv *priv = term->priv;

        switch(param_id) {
        case PROP_KT_PREFS:
                if (priv->prefs)
                        g_object_unref(priv->prefs);

                priv->prefs = g_object_ref(g_value_get_object(value));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
                break;
        }
}

static void kt_terminal_finalize(GObject *object)
{
        KtTerminal *term = KT_TERMINAL(object);
        KtTerminalPriv *priv = term->priv;

        if (priv->pty)
                g_object_unref(priv->pty);

        if (priv->prefs)
                g_object_unref(priv->prefs);

        G_OBJECT_CLASS(kt_terminal_parent_class)->finalize(object);
}

static void kt_terminal_class_init(KtTerminalClass *klass)
{
        GObjectClass *oclass = G_OBJECT_CLASS(klass);

        oclass->get_property = kt_terminal_get_property;
        oclass->set_property = kt_terminal_set_property;
        oclass->finalize = kt_terminal_finalize;

        g_object_class_install_property(oclass,
                                        PROP_KT_PREFS,
                                        g_param_spec_object("kt-prefs",
                                                            "Kixterm Preferences",
                                                            "The KtPrefs object",
                                                            KT_PREFS_TYPE,
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE));

        /* TODO:Setup signal handlers */

        g_type_class_add_private(klass, sizeof(KtTerminalPriv));
}

static void kt_terminal_init(KtTerminal *term)
{
        KtTerminalPriv *priv;

        term->priv = G_TYPE_INSTANCE_GET_PRIVATE(term,
                                                 KT_TERMINAL_TYPE,
                                                 KtTerminalPriv);
        priv = term->priv;

        priv->pty = NULL;
}

/* Public methods */
KtTerminal *kt_terminal_new(KtPrefs *prefs, xcb_window_t wid)
{
        KtTerminal *terminal = NULL;
        KtTerminalPriv *priv = NULL;

        g_return_val_if_fail(KT_IS_PREFS(prefs), NULL);

        terminal = g_object_new(KT_TERMINAL_TYPE,
                                "kt-prefs", prefs,
                                NULL);

        priv = terminal->priv;

        priv->pty = kt_pty_new(prefs, wid);
        if (priv->pty == NULL) {
                error("Could not create terminal.");
                goto failed;
        }

        return terminal;

failed:
        g_object_unref(terminal);
        return NULL;
}
