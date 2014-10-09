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

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 8192

enum {
        PROP_0,
        PROP_KT_PREFS,
        PROP_LAST
};

enum {
        SIGNAL_CHILD_EXITED,
        SIGNAL_GOT_TTY_DATA,
        SIGNAL_LAST
};

struct _KtTerminalPriv {
        KtPty *pty;

        GIOChannel *channel;
        guint io_event_source;

        GPid child_pid;
        guint child_watch_source;

        /* Properties */
        KtPrefs *prefs;
};

G_DEFINE_TYPE(KtTerminal, kt_terminal, G_TYPE_OBJECT);

static guint signals[SIGNAL_LAST] = {0, };
static GParamSpec *param_specs[PROP_LAST] = {NULL, };

/* Private methods */
static void terminal_set_size(KtTerminal *term)
{
        KtTerminalPriv *priv = term->priv;
        gint rows, cols;

        rows = priv->prefs->rows;
        cols = priv->prefs->cols;

        if (!kt_pty_set_size(priv->pty, rows, cols)) {
                error("Could not set pty size.");
        }
}

static void input_event_source_destroy(KtTerminal *term)
{
        term->priv->io_event_source = 0;
}

static gboolean io_read_cb(GIOChannel *channel,
                           GIOCondition cond,
                           KtTerminal *term)
{
        int err = 0;
        gboolean eof = FALSE;

        if (cond & G_IO_IN) {
                guint32 data[BUF_SIZE];
                int fd = g_io_channel_unix_get_fd(channel);

                memset (&data, 0, BUF_SIZE);

                do {
                        int ret;

                        ret = read(fd, data, BUF_SIZE - 1);
                        if (ret == -1) {
                                err = errno;
                                break;
                        } else if (ret == 0) {
                                eof = TRUE;
                                break;
                        } else {
                                KtBuffer *buffer = kt_buffer_new(data, ret);
                                g_signal_emit(term,
                                              signals[SIGNAL_GOT_TTY_DATA],
                                              0,
                                              buffer);
                                kt_buffer_free(buffer);
                        }


                } while(1);
        }

        switch (err) {
        case 0:
                break;
        case EIO:
                eof = TRUE;
                break;
        case EAGAIN:
        case EBUSY:
                break;
        default:
                warn("error reading from child.", g_strerror(err));
                break;
        }

        if (eof) {
                /* FIXME: Need to signal eof here. */
                return FALSE;
        }

        return TRUE;
}

static void terminal_setup_pty(KtTerminal *term)
{
        KtTerminalPriv *priv = term->priv;
        gint mfd;
        long flags;

        mfd = kt_pty_get_fd(priv->pty);

        priv->channel = g_io_channel_unix_new(mfd);
        g_io_channel_set_close_on_unref(priv->channel, FALSE);

        flags = fcntl(mfd, F_GETFL);
        if ((flags & O_NONBLOCK) == 0)
                fcntl(mfd, F_SETFL, flags | O_NONBLOCK);

        terminal_set_size(term);

        if (priv->io_event_source == 0) {
                priv->io_event_source = g_io_add_watch_full(priv->channel,
                                                            G_PRIORITY_DEFAULT_IDLE,
                                                            G_IO_IN | G_IO_HUP,
                                                            (GIOFunc) io_read_cb,
                                                            term,
                                                            (GDestroyNotify)input_event_source_destroy);
        }
}

static void terminal_emit_child_exited(KtTerminal *term, int status)
{
        g_signal_emit(term, signals[SIGNAL_CHILD_EXITED], status);
}

static void child_watch_cb(GPid pid,
                           int status,
                           KtTerminal *term)
{
        KtTerminalPriv *priv = term->priv;

        g_object_ref(G_OBJECT(term));

        if (pid == priv->child_pid) {
                if (WIFEXITED(status)) {
                        debug("Child[%d] exited: %d\n",
                              pid, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                        debug("Child[%d] dies: %d\n", pid,
                              WTERMSIG(status));
                }
        }

        terminal_emit_child_exited(term, status);

        g_object_unref(G_OBJECT(term));
}

static void terminal_watch_child(KtTerminal *term)
{
        KtTerminalPriv *priv = term->priv;

        g_object_freeze_notify(G_OBJECT(term));

        if (priv->child_watch_source != 0) {
                g_source_remove(priv->child_watch_source);
        }

        priv->child_watch_source = g_child_watch_add_full(G_PRIORITY_HIGH,
                                                          priv->child_pid,
                                                          (GChildWatchFunc)child_watch_cb,
                                                          term,
                                                          NULL);

        g_object_thaw_notify(G_OBJECT(term));
}

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

        if (priv->child_watch_source != 0) {
                g_source_remove(priv->child_watch_source);
                priv->child_watch_source = 0;
        }

        if (priv->channel)
                g_io_channel_unref(priv->channel);

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

        param_specs[PROP_KT_PREFS] = g_param_spec_object("kt-prefs",
                                                         "Kixterm Preferences",
                                                         "The KtPrefs object",
                                                         KT_PREFS_TYPE,
                                                         G_PARAM_CONSTRUCT_ONLY |
                                                         G_PARAM_READWRITE);

        g_object_class_install_properties (oclass, PROP_LAST, param_specs);

        klass->child_exited = NULL;
        /**
           "child-exited" signal.
         */
        signals[SIGNAL_CHILD_EXITED] =
                g_signal_new("child-exited",
                             G_TYPE_FROM_CLASS(klass),
                             G_SIGNAL_RUN_LAST |
                             G_SIGNAL_NO_RECURSE |
                             G_SIGNAL_NO_HOOKS,
                             G_STRUCT_OFFSET(KtTerminalClass, child_exited),
                             NULL,
                             NULL,
                             g_cclosure_marshal_VOID__INT,
                             G_TYPE_NONE,
                             1,
                             G_TYPE_INT);

        /**
           "got-tty-data" signal.
         */
        signals[SIGNAL_GOT_TTY_DATA] =
                g_signal_new("got-tty-data",
                             G_TYPE_FROM_CLASS(klass),
                             G_SIGNAL_RUN_FIRST,
                             G_STRUCT_OFFSET(KtTerminalClass, got_tty_data),
                             NULL,
                             NULL,
                             NULL,
                             G_TYPE_NONE,
                             1,
                             KT_TYPE_BUFFER | G_SIGNAL_TYPE_STATIC_SCOPE);

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
        priv->io_event_source = 0;
        priv->channel = NULL;
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

        /* Create new pseudo terminal */
        priv->pty = kt_pty_new(prefs, wid);
        if (priv->pty == NULL) {
                error("Could not create terminal.");
                goto failed;
        }

        /* Spawn the shell */
        if (!kt_pty_spawn(priv->pty, NULL)) {
                error("Spawning failed.");
                goto failed;
        }

        /* Setup the pty */
        terminal_setup_pty(terminal);

        /* Add watch */
        priv->child_pid = kt_pty_get_child_pid(priv->pty);
        terminal_watch_child(terminal);

        return terminal;

failed:
        g_object_unref(terminal);
        return NULL;
}
