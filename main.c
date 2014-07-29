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
#include <locale.h>

#include <sys/time.h>

#include <glib-unix.h>

#include "kt-util.h"
#include "kt-app.h"
#include "kt-prefs.h"
#include "kt-font.h"
#include "kt-color.h"
#include "kt-window.h"

#include <xcb/xcb_event.h>

/* The Glib main loop variables */
static GMainLoop *loop = NULL;
static struct timeval last_wakeup; /* time of last wakeup */
static float main_loop_iteration_limit = 0.1; /* Current main loop's runtime limit*/

struct kixterm_t {
        KtApp *app;/* App instance */
        KtPrefs *prefs; /* Preferences instance */
        KtWindow *win;
        KtFont *font;
        KtColor *color;
};

struct kixterm_t kixterm;

/* The cleanup method is called before exiting */
static void cleanup(void)
{
        debug("Cleaning up....");
        g_object_unref(kixterm.app);
        g_object_unref(kixterm.prefs);
        g_object_unref(kixterm.win);
        g_object_unref(kixterm.color);
        debug("...Done!!");
}

/* Signal handlers */
static void signal_fatal(int signal)
{
        error("signal %d. Bailing out!!", signal);
}

static gboolean signal_handler(gpointer data)
{
        g_main_loop_quit(loop);
        return TRUE;
}

static void handle_x_event(guint8 response_type, xcb_generic_event_t *event)
{
        switch(response_type) {
        case XCB_KEY_PRESS:
                kt_window_key_press(kixterm.win,
                                    (xcb_key_press_event_t *)event);
                break;
        case XCB_KEY_RELEASE:
                kt_window_key_release(kixterm.win,
                                      (xcb_key_release_event_t *)event);
                break;
        case XCB_BUTTON_PRESS:
                kt_window_button_press(kixterm.win,
                                       (xcb_button_press_event_t *)event);
                break;
        case XCB_BUTTON_RELEASE:
                kt_window_button_release(kixterm.win,
                                         (xcb_button_release_event_t *)event);
                break;
        case XCB_MOTION_NOTIFY:
                kt_window_motion_notify(kixterm.win,
                                        (xcb_motion_notify_event_t *)event);
                break;
        case XCB_EXPOSE:
                kt_window_expose(kixterm.win,
                                 (xcb_expose_event_t *)event);
                break;
        case XCB_ENTER_NOTIFY:
                kt_window_enter_notify(kixterm.win,
                                       (xcb_enter_notify_event_t *)event);
                break;
        case XCB_LEAVE_NOTIFY:
                kt_window_leave_notify(kixterm.win,
                                       (xcb_leave_notify_event_t *)event);
                break;
        case XCB_FOCUS_IN:
                kt_window_focus_in(kixterm.win,
                                   (xcb_focus_in_event_t *)event);
                break;
        case XCB_FOCUS_OUT:
                kt_window_focus_out(kixterm.win,
                                   (xcb_focus_out_event_t *)event);
                break;
        case XCB_MAP_NOTIFY:
                kt_window_map_notify(kixterm.win,
                                     (xcb_map_notify_event_t *)event);
                break;
        case XCB_UNMAP_NOTIFY:
                kt_window_unmap_notify(kixterm.win,
                                     (xcb_unmap_notify_event_t *)event);
                break;
        case XCB_CONFIGURE_NOTIFY:
                kt_window_configure_notify(kixterm.win,
                                           (xcb_configure_notify_event_t *)event);
                break;
        case XCB_DESTROY_NOTIFY:
                kt_window_destroy_notify(kixterm.win,
                                         (xcb_destroy_notify_event_t *)event);
                break;
        case XCB_SELECTION_CLEAR:
                kt_window_selection_clear(kixterm.win,
                                          (xcb_selection_clear_event_t *)event);
                break;
        case XCB_SELECTION_NOTIFY:
                kt_window_selection_notify(kixterm.win,
                                           (xcb_selection_notify_event_t *)event);
                break;
        case XCB_SELECTION_REQUEST:
                kt_window_selection_request(kixterm.win,
                                            (xcb_selection_request_event_t *)event);
                break;
        case XCB_CLIENT_MESSAGE:
                kt_window_client_message(kixterm.win,
                                         (xcb_client_message_event_t *)event);
                break;
        case XCB_REPARENT_NOTIFY:
                kt_window_reparent_notify(kixterm.win,
                                          (xcb_reparent_notify_event_t *)event);
                break;
        case XCB_PROPERTY_NOTIFY:
                kt_window_property_notify(kixterm.win,
                                          (xcb_property_notify_event_t *)event);
                break;
        default:
                break;
        }
}

static gboolean kt_xcb_io_cb(GIOChannel *source, GIOCondition cond, gpointer data)
{
        /* All XCB events should have been handled by kt_poll()->kt_xcb_handler()
         */
        xcb_connection_t *connection;

        connection = kt_app_get_x_connection(kixterm.app);

        if (xcb_connection_has_error(connection)) {
                error("Connection problem with X server. Error: %d",
                      xcb_connection_has_error(connection));
                exit(EXIT_FAILURE);
        }

        return TRUE;
}

static void kt_xcb_event_handler(xcb_connection_t *c)
{
        xcb_generic_event_t *event = NULL;

        while((event = xcb_poll_for_event(c))) {
                guint8 response_type;

                response_type = XCB_EVENT_RESPONSE_TYPE(event);
                if (response_type != 0) {
                        handle_x_event(response_type, event);
                }
        }

        xcb_flush(c);
}

/* GMainLoop handler (From awesome.c - awesome project) */
static gint kt_poll(GPollFD *udfs, guint nfsd, gint timeout)
{
        guint res;
        struct timeval now, length_time;
        float length;
        xcb_connection_t *connection;

        connection = kt_app_get_x_connection(kixterm.app);

        xcb_flush(connection);

        /* Duration of the main loop iteration */
        gettimeofday(&now, NULL);
        timersub(&now, &last_wakeup, &length_time);
        length = length_time.tv_sec + length_time.tv_usec * 1.0f / 1e6;
        if (length > main_loop_iteration_limit) {
                warn("Last main loop iteration took %.6f seconds!"
                     "Increasing limit for this warning to that value.", length);
                main_loop_iteration_limit = length;
        }

        /* Do the actual polling */
        res = g_poll(udfs, nfsd, timeout);
        gettimeofday(&last_wakeup, NULL);

        kt_xcb_event_handler(connection);

        return res;
}

/* The main() function. */
int main(int argc, char **argv)
{
        struct sigaction sa;
        GIOChannel *channel;

        setlocale(LC_CTYPE, "");

        /* Cleanup and signal handling */
        atexit(cleanup);

        g_unix_signal_add(SIGINT, signal_handler, NULL);
        g_unix_signal_add(SIGTERM, signal_handler, NULL);


        sa.sa_handler = signal_fatal;
        sa.sa_flags = SA_RESETHAND;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGABRT, &sa, 0);
        sigaction(SIGBUS, &sa, 0);
        sigaction(SIGFPE, &sa, 0);
        sigaction(SIGILL, &sa, 0);
        sigaction(SIGSEGV, &sa, 0);

        /* App core */
        kixterm.app = kt_app_new();
        /* Preferences */
        kixterm.prefs = kt_prefs_new();
        /* Font */
        kixterm.font = kt_font_new(kixterm.app, kixterm.prefs);
        /* Color */
        kixterm.color = kt_color_new(kixterm.app, kixterm.prefs);
        /* Main window */
        kixterm.win = kt_window_new(kixterm.app,
                                    kixterm.prefs,
                                    kixterm.font,
                                    kixterm.color);

        /* Watch the X file descriptor for events */
        channel = g_io_channel_unix_new(kt_app_get_xfd(kixterm.app));
        g_io_add_watch(channel, G_IO_IN, kt_xcb_io_cb, NULL);
        g_io_channel_unref(channel);

        /* Main context */
        g_main_context_set_poll_func(g_main_context_default(), &kt_poll);

        /* main event loop */
        loop = g_main_loop_new(NULL, FALSE);

        g_main_loop_run(loop);

        /* Cleanup */
        g_main_loop_unref(loop);
        loop = NULL;

        exit(EXIT_SUCCESS);
}
