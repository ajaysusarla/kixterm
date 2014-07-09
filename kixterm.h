/*
 * kixterm.h
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

#ifndef KIXTERM_H
#define KIXTERM_H

/* XCB */
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_event.h>

/* XDG dirs */
#include <basedir.h>

#include "kt-font.h"

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

/* Graphic Context enum */
enum {
        GC_DRAW = 0,
        GC_CLEAR,
        GC_ATTR,
        GC_MAX
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
        CUR_INVISBLE,
        CUR_MAX
};

/** The main global configuration structure
 */
typedef struct {
        /* Connection */
        xcb_connection_t *connection;

        /* Screen information */
        xcb_screen_t *screen;
        /* Default screen */
        int default_screen;

        /* Time stamp from X */
        xcb_timestamp_t timestamp;

        /* Visual information */
        xcb_visualtype_t *visual;

        /* Key symbols */
        xcb_key_symbols_t *keysyms;

        /* Root Pixmap */
        xcb_pixmap_t pixmap;

        /* Graphic Context */
        xcb_gcontext_t gc[GC_MAX];

        /* Atoms */
        xcb_atom_t atom[ATOM_MAX];

        /* Cursor */
        xcb_cursor_t cursor[CUR_MAX];

        /* EWMH atoms */
        xcb_ewmh_connection_t ewmh;

        /* Fonts */
        kixterm_font_t *font;

        /* X file descriptor */
        int xfd;

        /* Master file descriptor */
        int mfd;

        /* XDG handle */
        xdgHandle xdg;

        /* Hostname */
        const char *hostname;
        const char *display;

        /* Modifiers */
        uint8_t km[KM_MAX];

        /* pid */
        pid_t pid;
} kixterm_t;

extern kixterm_t conf;
#endif /* KIXTERM_H */
