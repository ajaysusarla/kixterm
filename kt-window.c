/*
 * kt-window.c
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

#include "kt-window.h"
#include "kt-terminal.h"
#include "kt-util.h"

#include <xcb/xcb_icccm.h>
#include <pango/pangocairo.h>

struct _KtWindowPriv {
        xcb_window_t window;
        xcb_gcontext_t gc;
        xcb_rectangle_t geometry;

        xcb_pixmap_t pixmap;
        cairo_surface_t *surface;
        cairo_t *cairo;
        gboolean mapped;

        KtTerminal *terminal;

        /* Properites */
        KtApp *app;
        KtPrefs *prefs;
        KtFont *font;
        KtColor *color;
};

enum {
        PROP_0,
        PROP_KT_APP,
        PROP_KT_PREFS,
        PROP_KT_FONT,
        PROP_KT_COLOR,
};

G_DEFINE_TYPE(KtWindow, kt_window, G_TYPE_OBJECT);

/* Private methods */

/* Class methods */
static void kt_window_get_property(GObject *obj,
                                   guint param_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
        KtWindow *window = KT_WINDOW(obj);
        KtWindowPriv *priv = window->priv;

        switch(param_id) {
        case PROP_KT_APP:
                g_value_set_object(value, priv->app);
                break;
        case PROP_KT_PREFS:
                g_value_set_object(value, priv->prefs);
                break;
        case PROP_KT_FONT:
                g_value_set_object(value, priv->font);
                break;
        case PROP_KT_COLOR:
                g_value_set_object(value, priv->color);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
                break;
        }
}

static void kt_window_set_property(GObject *obj,
                                  guint param_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
        KtWindow *window = KT_WINDOW(obj);
        KtWindowPriv *priv = window->priv;

        switch(param_id) {
        case PROP_KT_APP:
                if (priv->app)
                        g_object_unref(priv->app);

                priv->app = g_object_ref(g_value_get_object(value));
                break;
        case PROP_KT_PREFS:
                if (priv->prefs)
                        g_object_unref(priv->prefs);

                priv->prefs = g_object_ref(g_value_get_object(value));
                break;
        case PROP_KT_FONT:
                if (priv->font)
                        g_object_unref(priv->font);

                priv->font = g_object_ref(g_value_get_object(value));
                break;
        case PROP_KT_COLOR:
                if (priv->color)
                        g_object_unref(priv->color);

                priv->color = g_object_ref(g_value_get_object(value));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
                break;
        }
}

static void kt_window_finalize(GObject *object)
{
        KtWindow *window = KT_WINDOW(object);
        KtWindowPriv *priv = window->priv;
        xcb_connection_t *con;

        con = kt_app_get_x_connection(priv->app);

        xcb_destroy_window(con, priv->window);
        xcb_free_gc(con, priv->gc);

        priv->mapped = FALSE;

        if (priv->terminal)
                g_object_unref(priv->terminal);

        if (priv->app)
                g_object_unref(priv->app);
        if (priv->prefs)
                g_object_unref(priv->prefs);
        if (priv->font)
                g_object_unref(priv->font);
        if (priv->color)
                g_object_unref(priv->color);

        G_OBJECT_CLASS(kt_window_parent_class)->finalize(object);
}

static void kt_window_class_init(KtWindowClass *klass)
{
        GObjectClass *oclass = G_OBJECT_CLASS(klass);

        oclass->get_property = kt_window_get_property;
        oclass->set_property = kt_window_set_property;
        oclass->finalize = kt_window_finalize;

        g_object_class_install_property(oclass,
                                        PROP_KT_APP,
                                        g_param_spec_object("kt-app",
                                                            "Kixterm App",
                                                            "The KtApp object",
                                                            KT_APP_TYPE,
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE));

        g_object_class_install_property(oclass,
                                        PROP_KT_PREFS,
                                        g_param_spec_object("kt-prefs",
                                                            "Kixterm Preferences",
                                                            "The KtPrefs object",
                                                            KT_PREFS_TYPE,
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE));

        g_object_class_install_property(oclass,
                                        PROP_KT_FONT,
                                        g_param_spec_object("kt-font",
                                                            "Kixterm Font",
                                                            "The KtFont object",
                                                            KT_FONT_TYPE,
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE));
        g_object_class_install_property(oclass,
                                        PROP_KT_COLOR,
                                        g_param_spec_object("kt-color",
                                                            "Kixterm Color",
                                                            "The KtColor object",
                                                            KT_COLOR_TYPE,
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE));

        g_type_class_add_private(klass, sizeof(KtWindowPriv));
}

static void kt_window_init(KtWindow *window)
{
        KtWindowPriv *priv;

        window->priv = G_TYPE_INSTANCE_GET_PRIVATE(window,
                                                   KT_WINDOW_TYPE,
                                                   KtWindowPriv);

        priv = window->priv;

        priv->window = 0;
        priv->gc = 0;
        priv->pixmap = 0;
        priv->surface = NULL;
        priv->cairo = NULL;
        priv->mapped = FALSE;

        priv->terminal = NULL;
}

/* Public methods */
KtWindow *kt_window_new(KtApp *app,
                        KtPrefs *prefs,
                        KtFont *font,
                        KtColor *color)
{
        KtWindow *win = NULL;
        KtWindowPriv *priv = NULL;
        xcb_connection_t *con;
        xcb_screen_t *screen;
        xcb_generic_error_t *error = NULL;
        xcb_void_cookie_t cookie;
        gint width;
        gint height;
        guint32 win_vals[7] = {
                0,
                XCB_GRAVITY_NORTH_WEST,
                XCB_GRAVITY_NORTH_WEST,
                XCB_BACKING_STORE_NOT_USEFUL,
                0,
                XCB_EVENT_MASK_KEY_PRESS |
                XCB_EVENT_MASK_KEY_RELEASE |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                XCB_EVENT_MASK_FOCUS_CHANGE |
                XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_LEAVE_WINDOW |
                XCB_EVENT_MASK_EXPOSURE |
                XCB_EVENT_MASK_POINTER_MOTION_HINT |
                XCB_EVENT_MASK_POINTER_MOTION |
                XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_BUTTON_RELEASE,
                0
        };
        guint32 gc_vals[2];

        g_return_val_if_fail(KT_IS_APP(app), NULL);
        g_return_val_if_fail(KT_IS_PREFS(prefs), NULL);
        g_return_val_if_fail(KT_IS_FONT(font), NULL);
        g_return_val_if_fail(KT_IS_COLOR(color), NULL);

        win = g_object_new(KT_WINDOW_TYPE,
                           "kt-app", app,
                           "kt-prefs", prefs,
                           "kt-font", font,
                           "kt-color", color,
                           NULL);

        priv = win->priv;

        con = kt_app_get_x_connection(app);
        screen = kt_app_get_screen(app);

        /* Create the main window */
        kt_font_get_size(font, &width, &height);

        priv->geometry.width = 2 * priv->prefs->bd_width +
                priv->prefs->cols * width +
                priv->prefs->sb_width;
        priv->geometry.height = 2 * priv->prefs->bd_width +
                priv->prefs->rows * height;

        win_vals[0] = kt_color_get_bg_pixel(color);
        win_vals[6] = kt_app_get_normal_cursor(app);

        priv->window = xcb_generate_id(con);
        cookie = xcb_create_window_checked(con,
                                           screen->root_depth,
                                           priv->window,
                                           screen->root,
                                           prefs->xpos,
                                           prefs->ypos,
                                           priv->geometry.width,
                                           priv->geometry.height,
                                           0,
                                           XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                           screen->root_visual,
                                           XCB_CW_BACK_PIXEL  |
                                           XCB_CW_BIT_GRAVITY |
                                           XCB_CW_WIN_GRAVITY |
                                           XCB_CW_BACKING_STORE |
                                           XCB_CW_SAVE_UNDER |
                                           XCB_CW_EVENT_MASK |
                                           XCB_CW_CURSOR,
                                           win_vals);

        error = xcb_request_check(con, cookie);
        if (error) {
                error("could not create a window(%d)!!", error->error_code);
                goto failed;
        }

        /* Create the graphic context */
        gc_vals[0] = kt_color_get_vb_pixel(color);
        gc_vals[1] = 0;
        priv->gc = xcb_generate_id(con);
        cookie = xcb_create_gc_checked(con,
                                       priv->gc,
                                       priv->window,
                                       XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES,
                                       gc_vals);

        error = xcb_request_check(con, cookie);
        if (error) {
                error("Could not create a GC!!");
                goto failed;
        }

        /* Create terminal */
        priv->terminal = kt_terminal_new(priv->prefs, priv->window);
        if (priv->terminal == NULL) {
                error("Could not create terminal.");
                goto failed;
        }

        /* Map the window */
        cookie = xcb_map_window(con, priv->window);
        error = xcb_request_check(con, cookie);
        if (error) {
                error("Could not map window!");
                goto failed;
        }

        xcb_flush(con);

        /* The window is mapped now. */
        priv->mapped = TRUE;

        return win;

failed:
        g_object_unref(win);
        error("Failed to create kixterm window\n");
        return NULL;
}
