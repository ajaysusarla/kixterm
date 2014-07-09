/*
 * kt-font.c - Pango font management utils.
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

#include <stdbool.h>

#include "kixterm.h"
#include "kt-util.h"

/*XXX: remove the following(font_name and font_size) once preferences are active.*/
static char default_font_name[] = "Monospace";
static int default_font_size = 12;

/* Internal functions */
static PangoFontDescription * font_new(const char *font_name,
                                       int font_size,
                                       bool normal,
                                       bool bold,
                                       bool italic)
{
        PangoFontDescription *font;

        font = pango_font_description_new();

        pango_font_description_set_family(font, font_name);
        pango_font_description_set_absolute_size(font, font_size * PANGO_SCALE);
        pango_font_description_set_weight(font,
                                          bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
        pango_font_description_set_style(font,
                                         italic ? PANGO_STYLE_OBLIQUE : PANGO_STYLE_NORMAL);

        return font;
}

static void font_free(PangoFontDescription *font)
{
        pango_font_description_free(font);
        font = NULL;
}

/* External functions */
void kt_get_font_size(PangoFontDescription *font_desc,
                      uint16_t *width,
                      uint16_t *height)
{
        cairo_surface_t *surface;
        cairo_t *cairo;
        PangoLayout *layout;
        PangoRectangle i_rect, l_rect;

        surface = cairo_xcb_surface_create(conf.connection,
                                           conf.screen->root,
                                           conf.visual,
                                           1, 1);

        cairo = cairo_create(surface);

        layout = pango_cairo_create_layout(cairo);
        pango_layout_set_font_description(layout, font_desc);

        pango_layout_set_text(layout, "W", -1);
        pango_cairo_update_layout(cairo, layout);

        pango_layout_get_extents(layout, &i_rect, &l_rect);

        *width = l_rect.width / PANGO_SCALE;
        *height = l_rect.height / PANGO_SCALE;


        g_object_unref(layout);
        cairo_destroy(cairo);
        cairo_surface_destroy(surface);
}

kixterm_font_t *kt_font_init(const char *font_name, int font_size)
{
        kixterm_font_t *font = NULL;

        uint16_t height;
        uint16_t width;

        if (font_name == NULL || font_size <= 0) {
                warn("Invalid font name/size.");
                return NULL;
        }

        font = (kixterm_font_t *)kt_malloc(sizeof(kixterm_font_t));
        font->normal = NULL;
        font->bold = NULL;
        font->italic = NULL;
        font->bold_italic = NULL;
        font->width = -1;
        font->height = -1;

        font->normal = font_new(font_name, font_size,
                               true, false, false);
        kt_get_font_size(font->normal, &width, &height);
        printf("Monospace normal font size: (W):%d, (H):%d\n", width, height);

        font->bold = font_new(font_name, font_size,
                             false, true, false);
        kt_get_font_size(font->bold, &width, &height);
        printf("Monospace bold font size: (W):%d, (H):%d\n", width, height);

        font->italic = font_new(font_name, font_size,
                             false, false, true);
        kt_get_font_size(font->italic, &width, &height);
        printf("Monospace italic font size: (W):%d, (H):%d\n", width, height);

        font->bold_italic = font_new(font_name, font_size,
                                false, true, true);
        kt_get_font_size(font->bold_italic, &width, &height);
        printf("Monospace bold-italic font size: (W):%d, (H):%d\n", width, height);

        font->width = width;
        font->height = height;

        return font;
}

void kt_font_destroy(kixterm_font_t *font)
{
        if (font == NULL)
                return;

        if (font->normal)
                font_free(font->normal);
        if (font->bold)
                font_free(font->bold);
        if (font->italic)
                font_free(font->italic);
        if (font->bold_italic)
                font_free(font->bold_italic);

        font->height = -1;
        font->width = -1;
}
