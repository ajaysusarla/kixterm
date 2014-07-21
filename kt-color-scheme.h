/*
 * kt-color-scheme.h
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
#ifndef KT_COLOR_SCHEME_H
#define KT_COLOR_SCHEME_H

#include "kt-color.h"

G_BEGIN_DECLS
/*
  Terminal Emulator color schemes/maps.
  These color schemes define the ANSI 16 Colors. Each scheme has a 16 colors
  represented by varying R G B values. Each  R, G, B value is a hex value
  ranging from 0x00(0) to 0xFF(255).

  A few popular color schemes are listed below.
 */

const kt_color_t linux_colors[] = {
        { 0x00, 0x00, 0x00 },
        { 0xA8, 0x00, 0x00 },
        { 0x00, 0xA8, 0x00 },
        { 0xA8, 0x57, 0x00 },
        { 0x00, 0x00, 0xA8 },
        { 0xA8, 0x00, 0xA8 },
        { 0x00, 0xA8, 0xA8 },
        { 0xA8, 0xA8, 0xA8 },
        { 0x57, 0x57, 0x57 },
        { 0xFF, 0x57, 0x57 },
        { 0x57, 0xFF, 0x57 },
        { 0xFF, 0xFF, 0x57 },
        { 0x57, 0x57, 0xFF },
        { 0xFF, 0x57, 0xFF },
        { 0x57, 0xFF, 0xFF },
        { 0xFF, 0xFF, 0xFF }
};

const kt_color_t rxvt_colors[] = {
        { 0x00, 0x00, 0x00 },
        { 0xCD, 0x00, 0x00 },
        { 0x00, 0xCD, 0x00 },
        { 0xCD, 0xCD, 0x00 },
        { 0x00, 0x00, 0xCD },
        { 0xCD, 0x00, 0xCD },
        { 0x00, 0xCD, 0xCD },
        { 0xFA, 0xEB, 0xD7 },
        { 0x40, 0x40, 0x40 },
        { 0xFF, 0x00, 0x00 },
        { 0x00, 0xFF, 0x00 },
        { 0xFF, 0xFF, 0x00 },
        { 0x00, 0x00, 0xFF },
        { 0xFF, 0x00, 0xFF },
        { 0x00, 0xFF, 0xFF },
        { 0xFF, 0xFF, 0xFF }
};

G_END_DECLS

#endif /* KT_COLOR_SCHEME_H */
