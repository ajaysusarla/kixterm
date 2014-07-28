/*
 * kt-pty.c
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

#include "kt-pty.h"

#include <stdlib.h>
#include <unistd.h>
#include <pty.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <pwd.h>
#include <sys/types.h>

struct _KtPtyPriv {
        GString *wid;
        pid_t ppid;
        pid_t cpid;
        gint fd;
        GIOChannel *channel;

        /* Properties */
        KtPrefs *prefs;
};

enum {
        PROP_0,
        PROP_KT_PREFS,
};

G_DEFINE_TYPE(KtPty, kt_pty, G_TYPE_OBJECT);

/* Private methods */
static void kt_pty_child_watch_cb(GPid pid,
                                  int status,
                                  KtPty *pty)
{
        KtPtyPriv *priv;
        int stat = 0;

        priv = pty->priv;

        if (waitpid(priv->ppid, &stat, 0) < 0) {
                error("waitpid() failed.");
                exit(EXIT_FAILURE);
        }

        if (WIFEXITED(stat))
                exit(WEXITSTATUS(stat));
        else
                exit(EXIT_FAILURE);
}

static void kt_pty_watch_child(KtPty *pty, pid_t child_pid)
{
        KtPtyPriv *priv = pty->priv;

        g_object_freeze_notify(G_OBJECT(pty));

        g_child_watch_add_full(G_PRIORITY_HIGH,
                               child_pid,
                               (GChildWatchFunc)kt_pty_child_watch_cb,
                               pty, NULL);

        g_object_thaw_notify(G_OBJECT(pty));
}

static gboolean kt_pty_io_read(GIOChannel *source,
                               GIOCondition cond,
                               KtPty *pty)
{
        gboolean eof;
        static guchar buf[1000];
        static int buflen = 0;
        KtPtyPriv *priv = pty->priv;
        guchar *str;
        int ret;
        int fd;

        debug("Need to handle data.");

        eof = cond & G_IO_HUP;

        fd = g_io_channel_unix_get_fd(source);

        if (cond & G_IO_IN) {
                do {
                        ret = read(fd, buf+buflen, (sizeof(buf)/sizeof(buf[0])) - buflen);
                        if (ret < 0)
                                break;

                        fprintf(stdout, "%s", buf);
                } while (1);
                buflen += ret;
                str = buf;
        }

        return TRUE;
}

static void kt_pty_input_source_destroy(KtPty *pty)
{
        debug("TODO: Destroy input source.");
}

static void kt_pty_exec_shell(gchar *wid, gchar **args)
{
        const struct passwd *passwd = getpwuid(getuid());
        gchar *shell = NULL;

        debug("Entering..");

        unsetenv("COLUMNS");
        unsetenv("LINES");
        unsetenv("TERMCAP");

        if (passwd) {
                setenv("LOGNAME", passwd->pw_name, 1);
                setenv("USER", passwd->pw_name, 1);
                setenv("SHELL", passwd->pw_shell, 0);
                setenv("HOME", passwd->pw_dir, 0);
        }

        setenv("WINDOWID", wid, 1);
        setenv("TERM", "linux", 1);

        signal(SIGCHLD, SIG_DFL);
        signal(SIGHUP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGALRM, SIG_DFL);

        shell = getenv("SHELL");
        if (shell == NULL) {
                shell = "/bin/sh";
        }

        args = (char *[]){shell, "-i", NULL};

        debug("calling execvp");
        execvp(args[0], args);
        debug("return execvp\n");

        exit(EXIT_FAILURE);
}

/* Class methods */
static void kt_pty_get_property(GObject *obj,
                                guint param_id,
                                GValue *value,
                                GParamSpec *pspec)
{
        KtPty *pty = KT_PTY(obj);
        KtPtyPriv *priv = pty->priv;

        switch(param_id) {
        case PROP_KT_PREFS:
                g_value_set_object(value, priv->prefs);
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
                break;
        }
}

static void kt_pty_set_property(GObject *obj,
                                guint param_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
        KtPty *pty = KT_PTY(obj);
        KtPtyPriv *priv = pty->priv;

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

static void kt_pty_finalize(GObject *object)
{
        KtPty *pty = KT_PTY(object);
        KtPtyPriv *priv = pty->priv;

        if (priv->wid)
                g_string_free(priv->wid, TRUE);

        if (priv->prefs)
                g_object_unref(priv->prefs);

        G_OBJECT_CLASS(kt_pty_parent_class)->finalize(object);
}

static void kt_pty_class_init(KtPtyClass *klass)
{
        GObjectClass *oclass = G_OBJECT_CLASS(klass);

        oclass->get_property = kt_pty_get_property;
        oclass->set_property = kt_pty_set_property;
        oclass->finalize = kt_pty_finalize;

        g_object_class_install_property(oclass,
                                        PROP_KT_PREFS,
                                        g_param_spec_object("kt-prefs",
                                                            "Kixterm Preferences",
                                                            "The KtPrefs object",
                                                            KT_PREFS_TYPE,
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE));

        g_type_class_add_private(klass, sizeof(KtPtyPriv));
}

static void kt_pty_init(KtPty *pty)
{
        KtPtyPriv *priv;

        pty->priv = G_TYPE_INSTANCE_GET_PRIVATE(pty,
                                                KT_PTY_TYPE,
                                                KtPtyPriv);

        priv = pty->priv;

        priv->wid = NULL;
        priv->channel = NULL;
}

/* Public methods */
KtPty *kt_pty_new(KtPrefs *prefs, xcb_window_t wid)
{
        KtPty *pty = NULL;
        KtPtyPriv *priv;
        gint master, slave;
        /* Window ID is typically 8 digits. */
        gchar buf[sizeof(long) * 8 + 1];
        struct winsize wsize;
        long flags;

        g_return_val_if_fail(KT_IS_PREFS(prefs), NULL);

        pty = g_object_new(KT_PTY_TYPE,
                           "kt-prefs", prefs,
                           NULL);

        priv = pty->priv;

        /* Window size */
        wsize.ws_row = priv->prefs->rows;
        wsize.ws_col = priv->prefs->cols;
        wsize.ws_xpixel = 0;
        wsize.ws_ypixel = 0;

        /* Window ID */
        MEMSET(buf, 1);
        g_snprintf(buf, sizeof(buf), "%lu", (unsigned long)wid);
        priv->wid = g_string_new(buf);
        MEMSET(buf, 1);

        /* Create pty */
        if (openpty(&master, &slave, NULL, NULL, &wsize) < 0) {
                error("Could not open pty. openpty() failed.");
                goto failed;
        }

        priv->ppid = fork();
        switch (priv->ppid) {
        case -1: /* Error */
                error("fork() failed. Bailing...");
                goto failed;
        case 0: /* Child */
                debug("child process...");
                setsid();
                dup2(slave, STDIN_FILENO);
                dup2(slave, STDOUT_FILENO);
                dup2(slave, STDERR_FILENO);
                if (ioctl(slave, TIOCSCTTY, NULL) < 0) {
                        error("ioctl() for TIOSCTTY failed.");
                        goto failed;
                }
                close(slave);
                close(master);
                debug("Child process: Calling exec shell...");
                kt_pty_exec_shell(priv->wid->str, NULL);
                break;
        default: /* Parent */
                debug("Parent process...");
                close(slave);
                priv->fd = master;

                priv->channel = g_io_channel_unix_new(priv->fd);
                g_io_channel_set_close_on_unref(priv->channel, FALSE);

                flags = fcntl(priv->fd, F_GETFL);
                if ((flags & O_NONBLOCK) == 0)
                        fcntl(priv->fd, F_SETFL, flags | O_NONBLOCK);

                g_io_add_watch_full(priv->channel,
                                    G_PRIORITY_DEFAULT_IDLE,
                                    G_IO_IN | G_IO_HUP,
                                    (GIOFunc) kt_pty_io_read,
                                    pty,
                                    (GDestroyNotify)kt_pty_input_source_destroy);

                kt_pty_watch_child(pty, priv->ppid);
                break;
        }

        return pty;

failed:
        g_object_unref(pty);
        return NULL;
}
