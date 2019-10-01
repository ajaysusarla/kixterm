/*
 * kt-app.c
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

#include "kt-app.h"
#include "kt-util.h"

/* XCB Atoms */
enum {
        ATOM_PRIMARY = 0,
        ATOM_UTF8_STRING,
        ATOM_TARGETS,
        ATOM_CLIPBOARD,
        ATOM_WM_DELETE_WINDOW,
        ATOM_WM_PROTOCOLS,
        ATOM_XROOT_PIXMAP_ID,
        ATOM_ESETROOT_PIXMAP_ID,
        ATOM_NET_WM_WINDOW_OPACITY,
        ATOM_MAX
};

/* Key Modifiers */
enum {
        KM_SHIFT = 0,
        KM_ALT,
        KM_CTRL,
        KM_SUPER,
        KM_MODE_SWITCH,
        KM_NUM_LOCK,
        KM_SHIFT_LOCK,
        KM_CAPS_LOCK,
        KM_MAX
};

/* Cursor types */
enum {
        CUR_NORMAL = 0,
        CUR_HIDDEN,
        CUR_MAX
};

struct _KtAppPrivate {
        const gchar *display;

        xcb_connection_t *connection; /* X connection */
        xcb_ewmh_connection_t ewmh; /* EWMH atoms */

        xcb_screen_t *screen; /* Screen information */
        int default_screen;

        xcb_visualtype_t *visual; /* Visual information */
        xcb_key_symbols_t *key_symbols; /* Key symbols */
        xcb_pixmap_t root_pixmap; /* Root pixmap */
        xcb_cursor_t cursor[CUR_MAX]; /* Cursor types*/
        xcb_atom_t atom[ATOM_MAX]; /* Atoms */
        guint8 keymods[KM_MAX]; /* Key modifiers */

        gint xfd; /* X file descriptor */
};

G_DEFINE_TYPE_WITH_CODE(KtApp, kt_app, G_TYPE_OBJECT,
                        G_ADD_PRIVATE(KtApp));

/* Private methods */
static xcb_atom_t atom_init(xcb_connection_t *c,
                            const char *atom,
                            bool only_if_exists)
{
        xcb_intern_atom_cookie_t cookie;
        xcb_intern_atom_reply_t *reply = NULL;

        cookie = xcb_intern_atom_unchecked(c,
                                           only_if_exists,
                                           strlen(atom),
                                           atom);

        reply = xcb_intern_atom_reply(c,
                                      cookie,
                                      NULL);
        if (reply) {
                xcb_atom_t _a = reply->atom;
                free(reply);
                reply = NULL;
                return _a;
        } else {
                warn("Atom(%s) not found.", atom);
                return XCB_ATOM_NONE;
        }
}

static xcb_cursor_t create_normal_cursor(xcb_connection_t *c)
{
        xcb_font_t font;
        xcb_void_cookie_t cookie;
        xcb_cursor_t cursor;
        guint16 max = USHRT_MAX;
        xcb_generic_error_t *error = NULL;
        /*
          http://tronche.com/gui/x/xlib/appendix/b/
          152 = XC_xterm
        */
        guint16 cursor_id = 152;
        char *cursor_font = "cursor";

        font = xcb_generate_id(c);
        cookie = xcb_open_font_checked(c,
                                       font,
                                       strlen(cursor_font),
                                       cursor_font);
        error = xcb_request_check(c, cookie);
        if (error) {
                error("Could not open font %s. Error:%d",
                      cursor_font, error->error_code);
                xcb_close_font(c, font);
                free(error);
                error = NULL;
                return XCB_NONE;
        }

        cursor = xcb_generate_id(c);
        xcb_create_glyph_cursor_checked(c,
                                        cursor,
                                        font,
                                        font,
                                        cursor_id,
                                        cursor_id + 1,
                                        0, 0, 0,
                                        max / 2, max / 2, max / 2);
        error = xcb_request_check(c, cookie);
        if (error) {
                error("Could not create cursor. Error: %d\n",
                      error->error_code);
                xcb_close_font(c, font);
                free(error);
                error = NULL;
                return XCB_NONE;
        }

        return cursor;
}

static xcb_cursor_t create_hidden_cursor(xcb_connection_t *c,
                                         xcb_screen_t *screen)
{
        xcb_pixmap_t pixmap;
        xcb_void_cookie_t cookie;
        xcb_cursor_t cursor;
        xcb_generic_error_t *error = NULL;

        pixmap = xcb_generate_id(c);
        cookie = xcb_create_pixmap_checked(c,
                                           1,
                                           pixmap,
                                           screen->root,
                                           1, 1);
        error = xcb_request_check(c, cookie);
        if (error) {
                error("Could not create pixmap. Error: %d\n",
                      error->error_code);
                free(error);
                error = NULL;
                xcb_free_pixmap(c, pixmap);
                return XCB_NONE;
        }

        cursor = xcb_generate_id(c);
        cookie = xcb_create_cursor_checked(c,
                                           cursor,
                                           pixmap,
                                           pixmap,
                                           0, 0, 0,
                                           0, 0, 0,
                                           1, 1);
        error = xcb_request_check(c, cookie);
        if (error) {
                error("Could not create cursor. Error: %d.\n",
                      error->error_code);
                free(error);
                error = NULL;
                xcb_free_pixmap(c, pixmap);
                return XCB_NONE;
        }

        return cursor;
}

static xcb_pixmap_t get_root_pixmap(xcb_connection_t *c,
                                    xcb_screen_t *screen,
                                    xcb_atom_t atom)
{
        xcb_get_property_cookie_t cookie;
        xcb_get_property_reply_t *reply = NULL;
        xcb_pixmap_t *rootpixmap = NULL;

        cookie = xcb_get_property(c,
                                  0,
                                  screen->root,
                                  atom,
                                  XCB_ATOM_PIXMAP,
                                  0,
                                  1);

        reply = xcb_get_property_reply(c, cookie, NULL);

        if (reply &&
            (xcb_get_property_value_length(reply) == sizeof(xcb_pixmap_t))) {
                rootpixmap = (xcb_pixmap_t *)xcb_get_property_value(reply);
                debug("Got the root pixmap value.");
        } else {
                warn("Failed to get the root pixmap value.");
                *rootpixmap = XCB_NONE;
        }

        free(reply);

        return *rootpixmap;
}

/* Class methods */
static void kt_app_finalize(GObject *object)
{
        KtApp *app;
        KtAppPrivate *priv;

        app = KT_APP(object);
        priv = app->priv;

        if (priv->cursor[CUR_NORMAL] != XCB_NONE)
                xcb_free_cursor(priv->connection,
                                priv->cursor[CUR_NORMAL]);

        if (priv->cursor[CUR_HIDDEN] != XCB_NONE)
                xcb_free_cursor(priv->connection,
                                priv->cursor[CUR_HIDDEN]);

        if (&priv->ewmh)
                xcb_ewmh_connection_wipe(&priv->ewmh);

        if (priv->key_symbols)
                xcb_key_symbols_free(priv->key_symbols);

        if (priv->connection)
                xcb_disconnect(priv->connection);

        G_OBJECT_CLASS(kt_app_parent_class)->finalize(object);
}

static void kt_app_class_init(KtAppClass *klass)
{
        GObjectClass *oclass = G_OBJECT_CLASS(klass);

        oclass->finalize = kt_app_finalize;
}

static void kt_app_init(KtApp *app)
{
        KtAppPrivate *priv;

        app->priv = kt_app_get_instance_private(app);

        priv = app->priv;

        priv->display = NULL;
        priv->connection = NULL;
        priv->screen = NULL;
        priv->visual = NULL;

        priv->xfd = -1;
}

/* Public methods */
KtApp *kt_app_new(void)
{
        KtApp *app = NULL;
        KtAppPrivate *priv;
        xcb_screen_iterator_t s_iter;
        xcb_depth_iterator_t d_iter;
        int i;

        app = g_object_new(KT_APP_TYPE, NULL);

        priv = app->priv;

        /* Get the display env variable */
        priv->display = g_getenv("DISPLAY");
        if (priv->display == NULL) {
                priv->display = ":0";
        }

        /* Connect to X server */
        priv->connection = xcb_connect(priv->display, &priv->default_screen);
        if (xcb_connection_has_error(priv->connection)) {
                error("Cannot open connection to display server: %d.\n",
                      xcb_connection_has_error(priv->connection));
                goto failed;
        }

        /* Get the Screen information */
        s_iter = xcb_setup_roots_iterator(xcb_get_setup(priv->connection));
        for (i = 0; i != priv->default_screen; i++)
                xcb_screen_next(&s_iter);

        priv->screen = s_iter.data;

        if (priv->screen == NULL) {
                error("Failed to get screen information.");
                goto failed;
        }

        /* Get the Visual information */
        d_iter = xcb_screen_allowed_depths_iterator(priv->screen);
        while (d_iter.rem) {
                xcb_visualtype_iterator_t v_iter;
                v_iter = xcb_depth_visuals_iterator(d_iter.data);

                while (v_iter.rem) {
                        if (priv->screen->root_visual == v_iter.data->visual_id) {
                                priv->visual = v_iter.data;
                                break;
                        }
                        xcb_visualtype_next(&v_iter);
                }
                xcb_depth_next(&d_iter);
        }
        if (priv->visual == NULL) {
                error("Failed to get visual information.");
                goto failed;
        }

        /* Key Symbols */
        priv->key_symbols = xcb_key_symbols_alloc(priv->connection);
        if (priv->key_symbols == NULL) {
                error("Failed to get key symbols.");
                goto failed;
        }

        /* EWMH atoms */
        if (xcb_ewmh_init_atoms_replies(&priv->ewmh,
                                        xcb_ewmh_init_atoms(priv->connection,
                                                            &priv->ewmh),
                                        NULL) == 0) {
                error("Failed to initialize EWMH atoms.");
                goto failed;
        }

        /* XCB atoms */
        priv->atom[ATOM_PRIMARY] = XCB_ATOM_PRIMARY;
        priv->atom[ATOM_CLIPBOARD] = atom_init(priv->connection, "CLIPBOARD", true);
        priv->atom[ATOM_TARGETS] = atom_init(priv->connection, "TARGETS", true);
        priv->atom[ATOM_WM_PROTOCOLS] = atom_init(priv->connection, "WM_PROTOCOLS", false);
        priv->atom[ATOM_WM_DELETE_WINDOW] = atom_init(priv->connection, "WM_DELETE_WINDOW", true);
        priv->atom[ATOM_XROOT_PIXMAP_ID] = atom_init(priv->connection, "_XROOTPMAP_ID", true);
        priv->atom[ATOM_ESETROOT_PIXMAP_ID] = atom_init(priv->connection, "ESETROOT_PMAP_ID", true);
        priv->atom[ATOM_NET_WM_WINDOW_OPACITY] = atom_init(priv->connection, "_NET_WM_WINDOW_OPACITY", true);
        priv->atom[ATOM_UTF8_STRING] = atom_init(priv->connection, "UTF8_STRING", false);
        if (priv->atom[ATOM_UTF8_STRING] == XCB_ATOM_NONE) {
                warn("Atom UTF8_STRING not found, using STRING.");
                priv->atom[ATOM_UTF8_STRING] = XCB_ATOM_STRING;
        }

        /* Cursors */
        priv->cursor[CUR_NORMAL] = create_normal_cursor(priv->connection);
        priv->cursor[CUR_HIDDEN] = create_hidden_cursor(priv->connection,
                                                        priv->screen);

        /* Root pixmap */
        priv->root_pixmap = get_root_pixmap(priv->connection,
                                            priv->screen,
                                            priv->atom[ATOM_XROOT_PIXMAP_ID]);
        if (priv->root_pixmap == XCB_NONE) {
                warn("Failed to find root pixmap for atom: _XROOTPMAP_ID.");
                priv->root_pixmap = get_root_pixmap(priv->connection,
                                                    priv->screen,
                                                    priv->atom[ATOM_ESETROOT_PIXMAP_ID]);
                if (priv->root_pixmap == XCB_NONE) {
                        warn("Failed to find root pixmap for atom: ESETROOT_PMAP_ID.");
                }
        }

        return app;
        /* If we failed earlier, */
failed:
        g_object_unref(app);
        error("Failed to create KtApp object.");
        return NULL;
}

xcb_connection_t *kt_app_get_x_connection(KtApp *app)
{
        KtAppPrivate *priv;

        g_return_val_if_fail(KT_IS_APP(app), NULL);

        priv = app->priv;

        return priv->connection;
}

xcb_ewmh_connection_t *kt_app_get_ewmh_connection(KtApp *app)
{
        KtAppPrivate *priv;

        g_return_val_if_fail(KT_IS_APP(app), NULL);

        priv = app->priv;

        return &priv->ewmh;
}

xcb_screen_t *kt_app_get_screen(KtApp *app)
{
        KtAppPrivate *priv;

        g_return_val_if_fail(KT_IS_APP(app), NULL);

        priv = app->priv;

        return priv->screen;
}

gint kt_app_get_default_screen(KtApp *app)
{
        KtAppPrivate *priv;

        g_return_val_if_fail(KT_IS_APP(app), -1);

        priv = app->priv;

        return priv->default_screen;
}

xcb_visualtype_t *kt_app_get_visual(KtApp *app)
{
        KtAppPrivate *priv;

        g_return_val_if_fail(KT_IS_APP(app), NULL);

        priv = app->priv;

        return priv->visual;
}


gint kt_app_get_xfd(KtApp *app)
{
        KtAppPrivate *priv;

        g_return_val_if_fail(KT_IS_APP(app), -1);

        priv = app->priv;

        if (priv->xfd == -1) {
                priv->xfd = xcb_get_file_descriptor(priv->connection);
                return priv->xfd;
        } else
                return priv->xfd;
}

const gchar *kt_app_get_display_name(KtApp *app)
{
        KtAppPrivate *priv;

        g_return_val_if_fail(KT_IS_APP(app), NULL);

        priv = app->priv;

        return priv->display;
}

xcb_cursor_t kt_app_get_normal_cursor(KtApp *app)
{
        KtAppPrivate *priv;

        g_return_val_if_fail(KT_IS_APP(app), -1);

        priv = app->priv;

        return priv->cursor[CUR_NORMAL];
}

xcb_cursor_t kt_app_get_hidden_cursor(KtApp *app)
{
        KtAppPrivate *priv;

        g_return_val_if_fail(KT_IS_APP(app), -1);

        priv = app->priv;

        return priv->cursor[CUR_HIDDEN];
}

