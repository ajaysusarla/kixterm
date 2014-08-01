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

#define SPAWN_FLAGS G_SPAWN_CHILD_INHERITS_STDIN | \
        G_SPAWN_DO_NOT_REAP_CHILD

struct _KtPtyPriv {
        GString *wid;
        GPid cpid;
        /* openpty fd */
        gint mfd; /* masterfd */
        gint sfd; /* slavefd */

        /* Properties */
        KtPrefs *prefs;
};

enum {
        PROP_0,
        PROP_KT_PREFS,
};

G_DEFINE_TYPE(KtPty, kt_pty, G_TYPE_OBJECT);

/* Private methods */
/**
 * pty_create: Creates a pty using openpty()
 *
 * Returns: TRUE if successful, FALSE if not.
 */
static gboolean pty_create(KtPty *pty)
{
        KtPtyPriv *priv = pty->priv;
        gint master, slave;

        if (openpty(&master, &slave, NULL, NULL, NULL) < 0) {
                error("Could not open pty. openpty() failed.");
                return FALSE;
        }

        priv->mfd = master;
        priv->sfd = slave;

        return TRUE;
}

static void pty_spawn_cb(KtPty *pty)
{
        KtPtyPriv *priv = pty->priv;

        if (setsid() == -1) {
                error("setsid() failed.");
                exit(127);
        }

        /*
        if (setpgid(0, 0) == -1) {
                error("setpgid() failed.");
                exit(127);
        }
        */

        if (dup2(priv->sfd, STDIN_FILENO) != STDIN_FILENO) {
                error("dup2() for STDIN_FILENO failed.");
                exit(127);
        }

        if (dup2(priv->sfd, STDOUT_FILENO) != STDOUT_FILENO) {
                error("dup2() for STDOUT_FILENO failed.");
                exit(127);
        }

        if (dup2(priv->sfd, STDERR_FILENO) != STDERR_FILENO) {
                error("dup2() for STDERR_FILENO failed.");
                exit(127);
        }

        ioctl(priv->sfd, TIOCSCTTY, NULL);

        /* Close the fd */
        close(priv->sfd);
        close(priv->mfd);
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

        if (priv->mfd) {
                close(priv->mfd);
                priv->mfd = -1;
        }

        if (priv->sfd) {
                close(priv->sfd);
                priv->sfd = -1;
        }

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

        priv->mfd = -1;
        priv->sfd = -1;

        priv->wid = NULL;
}

/* Public methods */

KtPty *kt_pty_new(KtPrefs *prefs, xcb_window_t wid)
{
        KtPty *pty = NULL;
        KtPtyPriv *priv;
        /* Window ID is typically 8 digits. */
        gchar buf[sizeof(long) * 8 + 1];

        g_return_val_if_fail(KT_IS_PREFS(prefs), NULL);

        pty = g_object_new(KT_PTY_TYPE,
                           "kt-prefs", prefs,
                           NULL);

        priv = pty->priv;

        /* Window ID */
        MEMSET(buf, 1);
        g_snprintf(buf, sizeof(buf), "%lu", (unsigned long)wid);
        priv->wid = g_string_new(buf);
        MEMSET(buf, 1);

        /* Create pty */
        if (!pty_create(pty)) {
                error("Could not open pty. openpty() failed.");
                goto failed;
        }

        return pty;

failed:
        g_object_unref(pty);
        return NULL;
}

/**
 * kt_pty_get_fd()
 *
 * Returns: The master fd of the pty.
 */
gint kt_pty_get_fd(KtPty *pty)
{
        KtPtyPriv *priv;

        g_return_val_if_fail(KT_IS_PTY(pty), -1);

        priv = pty->priv;

        return priv->mfd;
}

/**
 * kt_pty_spawn: Spawn
 *
 * Returns: TRUE if successful, FALSE if not.
 */
gboolean kt_pty_spawn(KtPty *pty, gchar **args)
{
        KtPtyPriv *priv = pty->priv;
        gboolean retval = FALSE;
        GError *error = NULL;
        GPid pid;
        gchar *shell;

        g_return_val_if_fail(KT_IS_PTY(pty), FALSE);

        shell = getenv("SHELL");
        if (shell == NULL) {
                shell = "/bin/sh";
        }

        args = (char *[]){shell, "-i", NULL};


        retval = g_spawn_async_with_pipes(NULL, /* Spawn in CWD */
                                          args,
                                          NULL, /* FIXME: Need to setup the child env*/
                                          SPAWN_FLAGS,
                                          (GSpawnChildSetupFunc)pty_spawn_cb,
                                          pty,
                                          &pid,
                                          NULL, NULL, NULL,
                                          &error);

        if (!retval) {
                error("Spawn failed: %s", error->message);
                return FALSE;
        }

        priv->cpid = pid;

        return TRUE;
}

/**
 * kt_pty_set_size: Set the window size of the pty
 *
 * Returns: TRUE if successful, FALSE if not.
 */
gboolean kt_pty_set_size(KtPty *pty, gint rows, gint cols)
{
        KtPtyPriv *priv = pty->priv;
        struct winsize wsize;

        g_return_val_if_fail(KT_IS_PTY(pty), FALSE);

        MEMSET(&wsize, 1);

        wsize.ws_row = rows > 0 ? rows : 24;
        wsize.ws_col = cols > 0 ? cols : 80;
        wsize.ws_xpixel = 0;
        wsize.ws_ypixel = 0;

        debug("Settings pty size : %d x %d.", wsize.ws_row, wsize.ws_col);

        if (ioctl(priv->mfd, TIOCSWINSZ, &wsize) != 0) {
                error("Could no set the window size of the pty.");
                return FALSE;
        }

        return TRUE;
}

/**
 * kt_pty_get_child_pid()
 */
GPid kt_pty_get_child_pid(KtPty *pty)
{
        g_return_val_if_fail(KT_IS_PTY(pty), -1);

        return pty->priv->cpid;
}
