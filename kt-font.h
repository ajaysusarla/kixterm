/*
 * kt-font.h - Pango font management utils.
 *
 * A part of the kixterm project.
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

#ifndef KT_FONT_H
#define KT_FONT_H

/* Pango */
#include <pango/pango-font.h>
#include <pango/pangocairo.h>
#include <cairo/cairo-xcb.h>

typedef struct {
        PangoFontDescription *normal;
        PangoFontDescription *bold;
        PangoFontDescription *italic;
        PangoFontDescription *bold_italic;

        uint16_t width;
        uint16_t height;

} kixterm_font_t;


void kt_get_font_size(PangoFontDescription *font_desc,
                      uint16_t *width,
                      uint16_t *height);

kixterm_font_t *kt_font_init(const char *font_name, int font_size);

void kt_font_destroy(kixterm_font_t *font);
#endif /* KT_FONT_H */
