/*
 * kt-xcb.h - XCB initialization functions
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

#include "kt-xcb.h"
#include "kt-util.h"

#include <string.h>

/*
  http://tronche.com/gui/x/xlib/appendix/b/
  152 = XC_xterm
 */
#define NORMAL_CURSOR_ID 152
#define NORMAL_CURSOR_FONT_NAME "cursor"


/* Internal functions */

static xcb_visualtype_t *kt_get_visual_type(void)
{
        xcb_depth_iterator_t d_iter;

        d_iter = xcb_screen_allowed_depths_iterator(conf.screen);

        while (d_iter.rem) {
                xcb_visualtype_iterator_t v_iter;
                v_iter = xcb_depth_visuals_iterator(d_iter.data);

                while (v_iter.rem) {
                        if (conf.screen->root_visual == v_iter.data->visual_id) {
                                return v_iter.data;
                        }
                        xcb_visualtype_next(&v_iter);
                }

                xcb_depth_next(&d_iter);
        }

        /* If not found, we return a NULL */
        return NULL;
}

static xcb_pixmap_t kt_get_root_pixmap(xcb_atom_t atom)
{
        xcb_get_property_cookie_t cookie;
        xcb_get_property_reply_t *reply;
        xcb_pixmap_t *rootpixmap = NULL;

        cookie = xcb_get_property(conf.connection,
                                  0,
                                  conf.screen->root,
                                  atom,
                                  XCB_ATOM_PIXMAP,
                                  0,
                                  1);

        reply = xcb_get_property_reply(conf.connection, cookie, NULL);

        if (reply &&
            (xcb_get_property_value_length(reply) == sizeof(xcb_pixmap_t))) {
                rootpixmap = (xcb_pixmap_t *)xcb_get_property_value(reply);
                debug("Got the root pixmap value.");
        } else {
                warn("Failed to get the root pixmap value.");
                *rootpixmap = XCB_NONE;
        }

        free(reply);

        return *rootpixmap;
}

static xcb_cursor_t kt_normal_cursor_init(void)
{
        xcb_font_t font;
        xcb_void_cookie_t cookie;
        xcb_cursor_t cursor;
        uint16_t max = USHRT_MAX;
        xcb_generic_error_t *error = NULL;

        font = xcb_generate_id(conf.connection);
        cookie = xcb_open_font_checked(conf.connection,
                                       font,
                                       strlen(NORMAL_CURSOR_FONT_NAME),
                                       NORMAL_CURSOR_FONT_NAME);
        error = xcb_request_check(conf.connection, cookie);
        if (error) {
                fprintf(stderr, "Could not open font %s. Error:%d.\n",
                        NORMAL_CURSOR_FONT_NAME, error->error_code);
                xcb_close_font(conf.connection, font);
                free(error);
                error = NULL;
                return XCB_NONE;
        }


        cursor = xcb_generate_id(conf.connection);
        xcb_create_glyph_cursor_checked(conf.connection,
                                        cursor,
                                        font,
                                        font,
                                        NORMAL_CURSOR_ID,
                                        NORMAL_CURSOR_ID + 1,
                                        0, 0, 0,
                                        max / 2, max / 2, max / 2);
        error = xcb_request_check(conf.connection, cookie);
        if (error) {
                fprintf(stderr, "Could not create cursor. Error: %d\n",
                        error->error_code);
                xcb_close_font(conf.connection, font);
                free(error);
                error = NULL;
                return XCB_NONE;
        }

        return cursor;
}

static xcb_cursor_t kt_invisible_cursor_init(void)
{
        xcb_pixmap_t pixmap;
        xcb_void_cookie_t cookie;
        xcb_cursor_t cursor;
        xcb_generic_error_t *error = NULL;

        pixmap = xcb_generate_id(conf.connection);
        cookie = xcb_create_pixmap_checked(conf.connection,
                                           1,
                                           pixmap,
                                           conf.screen->root,
                                           1, 1);
        error = xcb_request_check(conf.connection, cookie);
        if (error) {
                fprintf(stderr, "Could not create pixmap. Error: %d\n",
                        error->error_code);
                free(error);
                error = NULL;
                xcb_free_pixmap(conf.connection, pixmap);
                return XCB_NONE;
        }

        cursor = xcb_generate_id(conf.connection);
        cookie = xcb_create_cursor_checked(conf.connection,
                                           cursor,
                                           pixmap,
                                           pixmap,
                                           0, 0, 0,
                                           0, 0, 0,
                                           1, 1);
        error = xcb_request_check(conf.connection, cookie);
        if (error) {
                fprintf(stderr, "Could not create cursor. Error: %d.\n",
                        error->error_code);
                free(error);
                error = NULL;
                xcb_free_pixmap(conf.connection, pixmap);
                return XCB_NONE;
        }

        return cursor;
}

static xcb_atom_t atom_init(const char *atom, bool only_if_exists)
{
        xcb_intern_atom_cookie_t cookie;
        xcb_intern_atom_reply_t *reply = NULL;

        cookie = xcb_intern_atom_unchecked(conf.connection,
                                           only_if_exists,
                                           strlen(atom),
                                           atom);

        reply = xcb_intern_atom_reply(conf.connection,
                                      cookie,
                                      NULL);
        if (reply) {
                xcb_atom_t _a = reply->atom;
                free(reply);
                reply = NULL;
                return _a;
        } else {
                warn("Atom(%s) not found.", atom);
                return XCB_ATOM_NONE;
        }
}

static void kt_atoms_init(void)
{
        conf.atom[ATOM_PRIMARY] = XCB_ATOM_PRIMARY;
        conf.atom[ATOM_CLIPBOARD] = atom_init("CLIPBOARD", true);
        conf.atom[ATOM_TARGETS] = atom_init("TARGETS", true);
        conf.atom[ATOM_WM_PROTOCOLS] = atom_init("WM_PROTOCOLS", false);
        conf.atom[ATOM_WM_DELETE_WINDOW] = atom_init("WM_DELETE_WINDOW", true);
        conf.atom[ATOM_XROOT_PIXMAP_ID] = atom_init("_XROOTPMAP_ID", true);
        conf.atom[ATOM_ESETROOT_PIXMAP_ID] = atom_init("ESETROOT_PMAP_ID", true);
        conf.atom[ATOM_NET_WM_WINDOW_OPACITY] = atom_init("_NET_WM_WINDOW_OPACITY", true);
        conf.atom[ATOM_UTF8_STRING] = atom_init("UTF8_STRING", false);
        if (conf.atom[ATOM_UTF8_STRING] == XCB_ATOM_NONE) {
                warn("Atom UTF8_STRING not found, using STRING.");
                conf.atom[ATOM_UTF8_STRING] = XCB_ATOM_STRING;
        }
        return;
}

/* External functions */

void kt_xcb_init(void)
{
        const char *str;
        xcb_screen_iterator_t s_iter;
        int i;

        /*  Get display environment variable.
            If we don't get any, we use the default ":0".
        */
        str = getenv("DISPLAY");
        conf.display = str ? str : ":0";

        /* Get XDG basedir */
        xdgInitHandle(&conf.xdg);

        /* Open the connection to the X server */
        conf.connection = xcb_connect(conf.display, &conf.default_screen);
        if (xcb_connection_has_error(conf.connection)) {
                error("Cannot open connection to display server: %d.",
                      xcb_connection_has_error(conf.connection));
        }

        /* Get the screen */
        s_iter = xcb_setup_roots_iterator(xcb_get_setup(conf.connection));
        for (i = 0; i != conf.default_screen; ++i)
                xcb_screen_next(&s_iter);

        conf.screen = s_iter.data;

        /* Get visual information */
        conf.visual = kt_get_visual_type();
        if (conf.visual == NULL) {
                error("Could not locate visual.");
        }

        /* Get Key Symbols */
        conf.keysyms = xcb_key_symbols_alloc(conf.connection);
        if (conf.keysyms == NULL) {
                error("Could not load key symbols.");
        }

        /* EWMH */
        if (xcb_ewmh_init_atoms_replies(&conf.ewmh,
                                        xcb_ewmh_init_atoms(conf.connection,
                                                            &conf.ewmh),
                                        NULL) == 0) {
                error("Could not initialize EMWH atoms.");
        }

        /* Atoms */
        kt_atoms_init();

        /* Root pixmap */
        conf.pixmap = kt_get_root_pixmap(conf.atom[ATOM_XROOT_PIXMAP_ID]);
        if (conf.pixmap == XCB_NONE) {
                warn("Failed to find root pixmap for atom: _XROOTPMAP_ID.");
                conf.pixmap = kt_get_root_pixmap(conf.atom[ATOM_ESETROOT_PIXMAP_ID]);
                if (conf.pixmap == XCB_NONE) {
                        warn("Failed to find root pixmap for atom: ESETROOT_PMAP_ID.");
                }
        }

        /* Cursor */
        conf.cursor[CUR_NORMAL] = kt_normal_cursor_init();
        conf.cursor[CUR_INVISBLE] = kt_invisible_cursor_init();

        /* Colours */

        /* Fonts */
        conf.font = kt_font_init("Monospace", strlen("Monospace"));

        return;
}

void kt_xcb_destroy(void)
{
        kt_font_destroy(conf.font);
}

uint32_t kt_xcb_get_color(void)
{
        xcb_alloc_color_cookie_t cookie;
        /* FIXME: Get the following r,g,b values from config/prefs */
        uint8_t r = 0x00 * 0xFF;
        uint8_t g = 0x00 * 0xFF;
        uint8_t b = 0x00 * 0xFF;
        xcb_alloc_color_reply_t *reply;
        uint32_t pixel;

        cookie = xcb_alloc_color(conf.connection,
                                 conf.screen->default_colormap,
                                 r, g, b);

        reply = xcb_alloc_color_reply(conf.connection, cookie, NULL);

        pixel = reply->pixel;

        free(reply);

        return pixel;
}

uint32_t kt_xcb_get_visual_bell_color(void)
{
        xcb_alloc_color_cookie_t cookie;
        /* FIXME: Get the following r,g,b values from config/prefs */
        uint8_t r = 0x7F * 0xFF;
        uint8_t g = 0x7F * 0xFF;
        uint8_t b = 0x7F * 0xFF;
        xcb_alloc_color_reply_t *reply;
        uint32_t pixel;

        cookie = xcb_alloc_color(conf.connection,
                                 conf.screen->default_colormap,
                                 r, g, b);

        reply = xcb_alloc_color_reply(conf.connection, cookie, NULL);

        pixel = reply->pixel;

        free(reply);

        return pixel;
}
