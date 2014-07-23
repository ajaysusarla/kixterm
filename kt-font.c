/*
 * kt-font.c
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

/*
  http://x11.gp2x.de/personal/google/
 */
#include "kt-font.h"

#include <cairo/cairo-xcb.h>
#include <pango/pangocairo.h>

struct _KtFontPriv {
        PangoFontDescription *normal;
        PangoFontDescription *bold;
        PangoFontDescription *italic;
        PangoFontDescription *bold_italic;

        guint16 width;
        guint16 height;

        /* Properties */
        KtApp *app;
        KtPrefs *prefs;
};

enum {
        PROP_0,
        PROP_KT_APP,
        PROP_KT_PREFS,
};

G_DEFINE_TYPE(KtFont, kt_font, G_TYPE_OBJECT);

/* Private methods */
static PangoFontDescription *font_desc_new(const char *name,
                                           int size,
                                           gboolean normal,
                                           gboolean bold,
                                           gboolean italic)
{
        PangoFontDescription *font_desc = NULL;

        font_desc = pango_font_description_new();

        pango_font_description_set_family(font_desc, name);
        pango_font_description_set_absolute_size(font_desc, size * PANGO_SCALE);
        pango_font_description_set_weight(font_desc,
                                          bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
        pango_font_description_set_style(font_desc,
                                         italic ? PANGO_STYLE_OBLIQUE : PANGO_STYLE_NORMAL);

        return font_desc;
}

static void font_desc_free(PangoFontDescription *font_desc)
{
        pango_font_description_free(font_desc);
        font_desc = NULL;
}

static void get_font_size(KtFont *font)
{
        cairo_surface_t *surface;
        cairo_t *cairo;
        PangoLayout *layout;
        PangoRectangle i_rect, l_rect;
        KtFontPriv *priv;
        xcb_connection_t *con;
        xcb_screen_t *screen;
        xcb_visualtype_t *visual;

        priv = font->priv;

        con = kt_app_get_x_connection(priv->app);
        screen = kt_app_get_screen(priv->app);
        visual = kt_app_get_visual(priv->app);

        surface = cairo_xcb_surface_create(con,
                                           screen->root,
                                           visual,
                                           1, 1);

        cairo = cairo_create(surface);

        layout = pango_cairo_create_layout(cairo);
        pango_layout_set_font_description(layout, priv->normal);

        pango_layout_set_text(layout, "W", -1);
        pango_cairo_update_layout(cairo, layout);

        pango_layout_get_extents(layout, &i_rect, &l_rect);

        priv->width = l_rect.width / PANGO_SCALE;
        priv->height = l_rect.height / PANGO_SCALE;


        g_object_unref(layout);
        cairo_destroy(cairo);
        cairo_surface_destroy(surface);
}

/* Class methods */
static void kt_font_get_property(GObject *obj,
                                 guint param_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
        KtFont *font = KT_FONT(obj);
        KtFontPriv *priv = font->priv;

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

static void kt_font_set_property(GObject *obj,
                                 guint param_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
        KtFont *font = KT_FONT(obj);
        KtFontPriv *priv = font->priv;

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

static void kt_font_finalize(GObject *object)
{
        KtFont *font = KT_FONT(object);
        KtFontPriv *priv = font->priv;

        if (priv->normal)
                font_desc_free(priv->normal);
        if (priv->bold)
                font_desc_free(priv->bold);
        if (priv->italic)
                font_desc_free(priv->italic);
        if (priv->bold_italic)
                font_desc_free(priv->bold_italic);

        if (priv->app)
                g_object_unref(priv->app);
        if (priv->prefs)
                g_object_unref(priv->prefs);

        G_OBJECT_CLASS(kt_font_parent_class)->finalize(object);
}

static void kt_font_class_init(KtFontClass *klass)
{
        GObjectClass *oclass = G_OBJECT_CLASS(klass);

        oclass->get_property = kt_font_get_property;
        oclass->set_property = kt_font_set_property;
        oclass->finalize = kt_font_finalize;

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

        g_type_class_add_private(klass, sizeof(KtFontPriv));
}

static void kt_font_init(KtFont *font)
{
        KtFontPriv *priv;

        font->priv = G_TYPE_INSTANCE_GET_PRIVATE(font,
                                                 KT_FONT_TYPE,
                                                 KtFontPriv);

        priv = font->priv;

        priv->normal = NULL;
        priv->bold = NULL;
        priv->italic = NULL;
        priv->bold_italic = NULL;

        priv->width = -1;
        priv->height = -1;
}

/* Public methods */
KtFont *kt_font_new(KtApp *app, KtPrefs *prefs)
{
        KtFont *font = NULL;
        KtFontPriv *priv;

        font = g_object_new(KT_FONT_TYPE,
                            "kt-app", app,
                            "kt-prefs", prefs,
                            NULL);

        priv = font->priv;

        priv->normal = font_desc_new(priv->prefs->font_name,
                                     priv->prefs->font_size,
                                     TRUE, FALSE, FALSE);

        priv->bold = font_desc_new(priv->prefs->font_name,
                                   priv->prefs->font_size,
                                   FALSE, FALSE, FALSE);

        priv->italic = font_desc_new(priv->prefs->font_name,
                                     priv->prefs->font_size,
                                     FALSE, FALSE, TRUE);

        priv->bold_italic = font_desc_new(priv->prefs->font_name,
                                          priv->prefs->font_size,
                                          FALSE, TRUE, TRUE);

        get_font_size(font);
        return font;
}

void kt_font_get_size(KtFont *font, int *width, int *height)
{
        KtFontPriv *priv;

        g_return_if_fail(KT_IS_FONT(font));

        priv = font->priv;

        *width = priv->width;
        *height = priv->height;
}
