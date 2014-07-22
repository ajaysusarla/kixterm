/*
 * kt-color.c
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

#include "kt-color.h"


struct _KtColorPriv {
        guint32 bg_pixel; /* The background pixel for the terminal */
        guint32 vb_pixel; /* The visual bell pixel */

        /* properties */
        KtApp *app;
        KtPrefs *prefs;
};

enum {
        PROP_0,
        PROP_KT_APP,
        PROP_KT_PREFS,
};

G_DEFINE_TYPE(KtColor, kt_color, G_TYPE_OBJECT);

/* Private methods */
guint32 init_pixel(KtColor *color, kt_color_t c)
{
        xcb_alloc_color_cookie_t cookie;
        xcb_alloc_color_reply_t *reply;
        xcb_connection_t *con;
        xcb_screen_t *screen;
        guint16 r, g, b;
        guint32 pixel;

        con = kt_app_get_x_connection(color->priv->app);
        screen = kt_app_get_screen(color->priv->app);

        r = c.r * 0xFF;
        g = c.g * 0xFF;
        b = c.b * 0xFF;

        cookie = xcb_alloc_color(con,
                                 screen->default_colormap,
                                 r, g, b);
        reply = xcb_alloc_color_reply(con, cookie, NULL);

        pixel = reply->pixel;

        free(reply);

        return pixel;
}

void free_pixels(KtColor *color)
{
        xcb_connection_t *con;
        xcb_screen_t *screen;
        guint32 pixels[] = {
                color->priv->bg_pixel,
                color->priv->vb_pixel
        };

        con = kt_app_get_x_connection(color->priv->app);
        screen = kt_app_get_screen(color->priv->app);

        xcb_free_colors(con,
                        screen->default_colormap,
                        0,
                        2, /* Changes with the number of elements in pixels[] */
                        pixels);
}

/* Class methods */
static void kt_color_get_property(GObject *obj,
                                  guint param_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
        KtColor *color = KT_COLOR(obj);
        KtColorPriv *priv = color->priv;

        switch(param_id) {
        case PROP_KT_APP:
                g_value_set_object(value, priv->app);
                break;
        case PROP_KT_PREFS:
                g_value_set_object(value, priv->prefs);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
                break;
        }
}

static void kt_color_set_property(GObject *obj,
                                  guint param_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
        KtColor *color = KT_COLOR(obj);
        KtColorPriv *priv = color->priv;

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
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
                break;
        }
}

static void kt_color_finalize(GObject *object)
{
        KtColor *color = KT_COLOR(object);
        KtColorPriv *priv = color->priv;

        free_pixels(color);

        if (priv->app)
                g_object_unref(priv->app);

        if (priv->prefs)
                g_object_unref(priv->prefs);

        G_OBJECT_CLASS(kt_color_parent_class)->finalize(object);
}

static void kt_color_class_init(KtColorClass *klass)
{
        GObjectClass *oclass = G_OBJECT_CLASS(klass);

        oclass->get_property = kt_color_get_property;
        oclass->set_property = kt_color_set_property;
        oclass->finalize = kt_color_finalize;

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

        g_type_class_add_private(klass, sizeof(KtColorPriv));
}

static void kt_color_init(KtColor *color)
{
        KtColorPriv *priv;

        color->priv = G_TYPE_INSTANCE_GET_PRIVATE(color,
                                                  KT_COLOR_TYPE,
                                                  KtColorPriv);

        priv = color->priv;

        priv->bg_pixel = -1;
        priv->vb_pixel = -1;
}

/* Public methods */
KtColor *kt_color_new(KtApp *app, KtPrefs *prefs)
{
        KtColor *color = NULL;
        KtColorPriv *priv;

        color = g_object_new(KT_COLOR_TYPE,
                             "kt-app", app,
                             "kt-prefs", prefs,
                             NULL);

        priv = color->priv;

        priv->bg_pixel = init_pixel(color, prefs->bg_color);
        priv->vb_pixel = init_pixel(color, prefs->vb_color);

        return color;
}

guint32 kt_color_get_bg_pixel(KtColor *color)
{
        g_return_val_if_fail(KT_IS_COLOR(color), -1);

        return color->priv->bg_pixel;
}

guint32 kt_color_get_vb_pixel(KtColor *color)
{
        g_return_val_if_fail(KT_IS_COLOR(color), -1);

        return color->priv->vb_pixel;
}
