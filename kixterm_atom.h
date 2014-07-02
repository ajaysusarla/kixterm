/*
 * kixterm_atom.h
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

#ifndef KIXTERM_ATOM_H
#define KIXTERM_ATOM_H

#include <stdbool.h>

#include <xcb/xcb.h>

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


xcb_atom_t kixterm_init_atom(xcb_connection_t *connection,
                             const char *atom,
                             bool only_if_exists);
#endif /* KITERM_ATOM_H */
