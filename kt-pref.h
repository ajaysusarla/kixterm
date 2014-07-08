/*
 * pref.h - kixterm preferences
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

#ifndef PREF_H
#define PREF_H

#include <stdint.h>

typedef struct {
        int16_t xpos;
        int16_t ypos;
        uint16_t rows;
        uint16_t cols;

        char *fontname;
        int fontsize;
} preferences;


extern preferences prefs, default_prefs;


#endif /* PREF_H */
