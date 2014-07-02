/*
 * kixterm.c
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

#include "kixterm.h"

#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xinerama.h>
#include <xcb/xtest.h>
#include <xcb/shape.h>

#include <fontconfig/fontconfig.h>
#include <X11/Xft/Xft.h>

#include <basedir.h>

#include "kconf.h"

#define DEFAULT_HT 24
#define DEFAULT_WD 80
#define DEFAULT_BORDER_WD 10

#define WINDOW_MASK XCB_CW_BACK_PIXEL | \
        XCB_CW_EVENT_MASK
#define WINDOW_EVENT_MASK                       \
        XCB_EVENT_MASK_EXPOSURE                 \
        | XCB_EVENT_MASK_BUTTON_PRESS           \
        | XCB_EVENT_MASK_BUTTON_RELEASE         \
        | XCB_EVENT_MASK_POINTER_MOTION         \
        | XCB_EVENT_MASK_ENTER_WINDOW           \
        | XCB_EVENT_MASK_LEAVE_WINDOW           \
        | XCB_EVENT_MASK_KEY_PRESS              \
        | XCB_EVENT_MASK_KEY_RELEASE

#define KIXTERM_TITLE "kiXterm"
#define KIXTERM_TITLE_LEN 7

/* move these to headers */
static int border = 2;
static char shell[] = "/bin/bash";

static float cwscale = 1.0;
static float chscale = 1.0;

static char font[] = "Liberation Mono:pixelsize=12:antialias=false:autohint=false";
static char *user_font = NULL;
static char *current_font = NULL;
static double current_fontsize = 0;
/**/

kixterm_t kconf; /* TODO: memset before use! */


static void print_modifiers(uint32_t mask)
{
        const char **mod, *mods[] = {
                "Shift", "Lock", "Ctrl", "Alt",
                "Mod2", "Mod3", "Mod4", "Mod5",
                "Button1", "Button2", "Button3", "Button4", "Button5"
        };
        printf ("Modifier mask: ");
        for (mod = mods ; mask; mask >>= 1, mod++)
                if (mask & 1)
                        printf(*mod);
        putchar ('\n');
}


int main(int argc, char **argv)
{
        xdgHandle xdg;

        /* Move these variables to a struct and a header */
        xcb_connection_t *connection;
        xcb_screen_t *screen;
        xcb_window_t window;
        xcb_generic_event_t *ev;
        int default_screen;
        uint32_t values[2];

        pid_t _pid = getpid();

        /* Set the right locale */
        setlocale(LC_CTYPE, "");


        /* Get XDG basedir */
        xdgInitHandle(&xdg);

        /* Open the connection to the X server */
        connection = xcb_connect(NULL, &default_screen);
        if (xcb_connection_has_error(connection)) {
                fprintf(stderr, "Cannot open display: %d\n",
                        xcb_connection_has_error(connection));
                exit(EXIT_FAILURE);
        }


        /* Font */
        if(!FcInit()) {
                fprintf(stderr, "Could not init fontconfig.\n");
                exit(EXIT_FAILURE);
        }
        current_font = (user_font == NULL) ? font : user_font;

        /* Get the first screen */
        screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

        /* Get our window's id */
        window = xcb_generate_id(connection);

        /* Create the window */
        values[0] = screen->white_pixel;
        values[1] = WINDOW_EVENT_MASK;

        xcb_create_window(connection,
                          XCB_COPY_FROM_PARENT,
                          window,
                          screen->root,
                          0, 0,
                          DEFAULT_WD, DEFAULT_HT,
                          DEFAULT_BORDER_WD,
                          XCB_WINDOW_CLASS_INPUT_OUTPUT,
                          screen->root_visual,
                          WINDOW_MASK,
                          values);

        /* Set the title of the window */
        xcb_change_property(connection,
                            XCB_PROP_MODE_REPLACE,
                            window,
                            XCB_ATOM_WM_NAME,
                            XCB_ATOM_STRING, 8,
                            KIXTERM_TITLE_LEN, KIXTERM_TITLE);

        /* Set the title of the window icon */
        xcb_change_property(connection,
                            XCB_PROP_MODE_REPLACE,
                            window,
                            XCB_ATOM_WM_ICON_NAME,
                            XCB_ATOM_STRING, 8,
                            KIXTERM_TITLE_LEN, KIXTERM_TITLE);

        /* Map this window to the screen */
        xcb_map_window(connection, window);

        /* Flush the connection */
        xcb_flush(connection);


        while ((ev = xcb_wait_for_event(connection))) {
                switch (ev->response_type & ~0x80) {
                case XCB_EXPOSE:
                        break;
                case XCB_BUTTON_PRESS: {
                        xcb_button_press_event_t *e = (xcb_button_press_event_t *) ev;
                        print_modifiers(e->state);
                        break;
                }
                case XCB_BUTTON_RELEASE:
                        break;
                case XCB_MOTION_NOTIFY:
                        break;
                case XCB_ENTER_NOTIFY:
                        break;
                case XCB_LEAVE_NOTIFY:
                        break;
                case XCB_KEY_PRESS: {
                        xcb_key_press_event_t *e = (xcb_key_press_event_t *) ev;
                        print_modifiers(e->state);
                        break;
                }
                case XCB_KEY_RELEASE: {
                        xcb_key_release_event_t *e;

                        e = (xcb_key_release_event_t *) ev;

                        switch (e->detail) {
                        case 9: /* Esc Key*/
                                printf("Escape key pressed.\n");
                                free(ev);
                                xcb_disconnect(connection);
                                return 0;
                        }
                        break;
                }
                default:
                        fprintf(stderr, "Unhandled event...\n");
                        break;
                }

                /* Free generic event */
                free(ev);
        }

        return 0;
}
