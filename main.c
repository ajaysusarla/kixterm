/*
 * main.c
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

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <sys/select.h>

#include "kixterm.h"
#include "kt-pref.h"
#include "kt-util.h"
#include "kt-xcb.h"
#include "kt-tty.h"


#define DEFAULT_X -1
#define DEFAULT_Y -1
#define DEFAULT_ROWS 24
#define DEFAULT_COLS 80
#define DEFAULT_BORDER_WD 2
#define DEFAULT_SCROLLBAR_WD 10


/* Global kixterm configuration */
kixterm_t conf;
kt_window_t window;
kixterm_prefs prefs, default_prefs;

static void cleanup(void)
{
        kt_xcb_destroy();
}

static void signal_handler(int signal)
{
        if (signal == SIGINT) {
                printf("SIGINT handled\n");
                goto success;
        } else if (signal == SIGTERM) {
                printf("SIGTERM handled\n");
                goto success;
        } else {
                fprintf(stderr, "Unhandled signal.\n");
        }

success:
        exit(EXIT_SUCCESS);
}

static void kixterm_init(void)
{
        xcb_generic_error_t *error = NULL;
        uint32_t win_values[] = {
                kt_xcb_get_color(),
                XCB_GRAVITY_NORTH_WEST,
                XCB_GRAVITY_NORTH_WEST,
                XCB_BACKING_STORE_NOT_USEFUL,
                0,
                XCB_EVENT_MASK_KEY_PRESS |
                XCB_EVENT_MASK_KEY_RELEASE |
                XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                XCB_EVENT_MASK_FOCUS_CHANGE |
                XCB_EVENT_MASK_ENTER_WINDOW |
                XCB_EVENT_MASK_LEAVE_WINDOW |
                XCB_EVENT_MASK_EXPOSURE |
                XCB_EVENT_MASK_POINTER_MOTION_HINT |
                XCB_EVENT_MASK_POINTER_MOTION |
                XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_BUTTON_RELEASE,
                conf.cursor[CUR_NORMAL]
        };
        uint32_t gc_values[] = {
                kt_xcb_get_visual_bell_color(),
                0
        };

        window.geometry.width = 2 * DEFAULT_BORDER_WD + DEFAULT_COLS * conf.font->width + DEFAULT_SCROLLBAR_WD;
        window.geometry.height = 2 * DEFAULT_BORDER_WD + DEFAULT_ROWS * conf.font->height;

        window.window = xcb_generate_id(conf.connection);

        window.cookie = xcb_create_window_checked(conf.connection,
                                                  conf.screen->root_depth,
                                                  window.window,
                                                  conf.screen->root,
                                                  DEFAULT_X, DEFAULT_Y,
                                                  window.geometry.width,
                                                  window.geometry.height,
                                                  0,
                                                  XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                                  conf.screen->root_visual,
                                                  XCB_CW_BACK_PIXEL  |
                                                  XCB_CW_BIT_GRAVITY |
                                                  XCB_CW_WIN_GRAVITY |
                                                  XCB_CW_BACKING_STORE |
                                                  XCB_CW_SAVE_UNDER |
                                                  XCB_CW_EVENT_MASK |
                                                  XCB_CW_CURSOR,
                                                  win_values);


        error = xcb_request_check(conf.connection, window.cookie);
        if (error) {
                 xcb_destroy_window(conf.connection, window.window);
                 error("could not create a window(%d).. Exiting!!", error->error_code);
         }

        window.gc = xcb_generate_id(conf.connection);
        window.cookie = xcb_create_gc_checked(conf.connection,
                                              window.gc,
                                              window.window,
                                              XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES,
                                              gc_values);
        error = xcb_request_check(conf.connection, window.cookie);
        if (error) {
                fprintf(stderr, "Could not create a GC...Exiting!!\n");
                xcb_destroy_window(conf.connection, window.window); /* XXX: Move 'window' to conf?? */
                exit(EXIT_FAILURE);
        }

        /* Create terminal */
        kt_tty_init(window.window);

        /* Map the window */
        window.cookie = xcb_map_window_checked(conf.connection, window.window);
        error = xcb_request_check(conf.connection, window.cookie);
        if (error) {
                fprintf(stderr, "Could not map window... Exiting!!\n");
                exit(EXIT_FAILURE);
        }

        xcb_flush(conf.connection);
}

static void handle_x_response(uint8_t response_type, xcb_generic_event_t *event)
{
        switch(response_type) {
        case XCB_KEY_PRESS:
                fprintf(stdout, "XCB_KEY_PRESS.\n");
                break;
        case XCB_KEY_RELEASE:
                fprintf(stdout, "XCB_KEY_RELEASE.\n");
                break;
        case XCB_BUTTON_PRESS:
                fprintf(stdout, "XCB_BUTTON_PRESS.\n");
                break;
        case XCB_BUTTON_RELEASE:
                fprintf(stdout, "XCB_BUTTON_RELEASE.\n");
                break;
        case XCB_MOTION_NOTIFY:
                fprintf(stdout, "XCB_MOTION_NOTIFY.\n");
                break;
        case XCB_EXPOSE:
                fprintf(stdout, "XCB_EXPOSE.\n");
                break;
        case XCB_ENTER_NOTIFY:
                fprintf(stdout, "XCB_ENTER_NOTIFY.\n");
                break;
        case XCB_LEAVE_NOTIFY:
                fprintf(stdout, "XCB_LEAVE_NOTIFY.\n");
                break;
        case XCB_FOCUS_IN:
                fprintf(stdout, "XCB_FOCUS_IN.\n");
                break;
        case XCB_FOCUS_OUT:
                fprintf(stdout, "XCB_FOCUS_OUT.\n");
                break;
        case XCB_MAP_NOTIFY:
                kt_xcb_map_notify((xcb_map_notify_event_t *) event);
                break;
        case XCB_UNMAP_NOTIFY:
                kt_xcb_unmap_notify((xcb_unmap_notify_event_t *) event);
                break;
        case XCB_CONFIGURE_NOTIFY:
                fprintf(stdout, "XCB_CONFIGURE_NOTIFY.\n");
                break;
        case XCB_DESTROY_NOTIFY:
                fprintf(stdout, "XCB_DESTROY_NOTIFY.\n");
                break;
        case XCB_SELECTION_CLEAR:
                fprintf(stdout, "XCB_SELECTION_CLEAR.\n");
                break;
        case XCB_SELECTION_NOTIFY:
                fprintf(stdout, "XCB_SELECTION_NOTIFY.\n");
                break;
        case XCB_SELECTION_REQUEST:
                fprintf(stdout, "XCB_SELECTION_REQUEST.\n");
                break;
        case XCB_CLIENT_MESSAGE:
                fprintf(stdout, "XCB_CLIENT_MESSAGE.\n");
                break;
        case XCB_REPARENT_NOTIFY:
                fprintf(stdout, "XCB_REPARENT_NOTIFY.\n");
                break;
        case XCB_PROPERTY_NOTIFY:
                fprintf(stdout, "XCB_PROPERTY_NOTIFY.\n");
                break;
        default:
                break;
        }
}

static void handle_from_x(void)
{
        xcb_generic_event_t *event = NULL;

        while (1) {
                uint8_t response_type;

                event = xcb_poll_for_event(conf.connection);

                if (event == NULL)
                        break;

                response_type = XCB_EVENT_RESPONSE_TYPE(event);
                if (response_type != 0)
                        handle_x_response(response_type, event);
        }

        if (xcb_connection_has_error(conf.connection)) {
                error("Error with X connection.");
                exit(EXIT_FAILURE);
        }

        if (event)
                free(event);
}

static void handle_from_tty(void)
{
        static char buf[BUFSIZ];
        static int buflen = 0;
        char *str;
        int ret, c;

        debug("enter..");
//        while (1) {
        debug("read..");
                ret = read(conf.mfd, buf+buflen, (sizeof(buf)/sizeof(buf[0])) - buflen);
                if (ret < 0) {
                        /* TODO:Handle Errors from `errno` here*/
                        error("read() from shell failed.");
                }
//        }
                debug("read done..");

        buflen += ret;
        str = buf;

        memmove (buf, str, buflen);
        /*
        while () {
        }
        */
//        printf(">> %s <<\n", str);

        if (window.mapped) {
                debug("calling kt_xcb_write_to_term");
                kt_xcb_write_to_term(str, buflen);
                //kt_xcb_write_to_term("Partha", 6);
        }
        debug("return..");
}

static void kixterm_main_loop(void)
{
        int xfd = conf.xfd;
        int mfd = conf.mfd;
        fd_set fds;
        struct timeval *tv = NULL;

        while (1) {
                FD_ZERO(&fds);
                FD_SET(mfd, &fds);
                FD_SET(xfd, &fds);

                if (select(MAX(xfd, mfd)+1, &fds, NULL, NULL, tv) < 0) {
                        if (errno == EINTR)
                                continue;

                        error("select() failed.");
                }

                if (FD_ISSET(mfd, &fds)) {
                        debug("from the master..");
                        handle_from_tty();
                }

                if (FD_ISSET(xfd, &fds)) {
                        debug("from X..");
                        handle_from_x();
                        xcb_flush(conf.connection);
                }
        }

        return;
}

int main(int argc, char **argv)
{
        /* Cleanup and signal handling */
        atexit(cleanup);
        signal(SIGTERM, signal_handler);
        signal(SIGINT, signal_handler);

        /* Set the right locale */
        setlocale(LC_CTYPE, "");

        MEMSET(&conf, 1);
        MEMSET(&prefs, 1);
        MEMSET(&default_prefs, 1);

        kt_xcb_init();

        kixterm_init();

        kixterm_main_loop();

        exit(EXIT_SUCCESS);
}
