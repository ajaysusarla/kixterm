/*
 * kixterm_atom.c
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

#include "kixterm_atom.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

xcb_atom_t kixterm_init_atom(xcb_connection_t *connection,
                             const char *atom,
                             bool only_if_exists)
{
        xcb_intern_atom_cookie_t cookie;
        xcb_intern_atom_reply_t *reply = NULL;

        cookie = xcb_intern_atom_unchecked(connection,
                                           only_if_exists,
                                           strlen(atom),
                                           atom);

        reply = xcb_intern_atom_reply(connection,
                                      cookie,
                                      NULL);
        if (reply) {
                xcb_atom_t _a = reply->atom;
                free(reply);
                reply = NULL;
                return _a;
        } else {
                fprintf(stderr, "Atom(%s) not found\n", atom);
                return XCB_ATOM_NONE;
        }
}

