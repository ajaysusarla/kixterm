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
#include <signal.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

/* openpty() */
#include <pty.h>

/* waitpid()*/
#include <sys/wait.h>

/* passwd */
#include <pwd.h>
#include <sys/types.h>

/* select */
#include <sys/select.h>

/* XCB */
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_event.h>
#include <xcb/xinerama.h>
#include <xcb/xtest.h>
#include <xcb/shape.h>

/* XDG dirs */
#include <basedir.h>

/* Pango */
#include <pango/pango-font.h>
#include <pango/pangocairo.h>
#include <cairo/cairo-xcb.h>

#include "kixterm_atom.h"

#define DEFAULT_X -1
#define DEFAULT_Y -1
#define DEFAULT_ROWS 24
#define DEFAULT_COLS 80
#define DEFAULT_BORDER_WD 2
#define DEFAULT_SCROLLBAR_WD 10

/*
  http://tronche.com/gui/x/xlib/appendix/b/
  152 = XC_xterm
 */
#define NORMAL_CURSOR_ID 152
#define NORMAL_CURSOR_FONT_NAME "cursor"


#define WINDOW_MASK \
        XCB_CW_BACK_PIXEL     \
        | XCB_CW_BORDER_PIXEL \
        | XCB_CW_OVERRIDE_REDIRECT \
        | XCB_CW_EVENT_MASK \
        | XCB_CW_COLORMAP

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
#define KIXTERM_TYPE "linux"

#undef MAX
#undef MIN
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#ifdef LINUX
#define HOSTNAME_MAX (HOST_NAME_MAX + 1)
#else
#define HOSTNAME_MAX 256
#endif

/* move these to headers */
static char *shell = NULL;


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

/*****/
static char default_font_name[] = "Monospace";
static int default_font_size = 12;


typedef struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
} kixterm_color_t;

typedef struct {
        PangoFontDescription *normal;
        PangoFontDescription *bold;
        PangoFontDescription *italic;
        PangoFontDescription *bold_italic;

        uint16_t width;
        uint16_t height;

} kixterm_font_t;

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
        kixterm_font_t font;

        /* X file descriptor*/
        int xfd;

        /* XDG handle */
        xdgHandle xdg;

        /* Hostname */
        const char *hostname;
        const char *display;

        /* Modifiers */
        uint8_t km[KM_MAX];
} kixterm_t;

kixterm_t kconf; /* TODO: memset before use! */
kixterm_font_t kfont;
pid_t pid;
int mfd;


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

static void kixterm_cleanup(void)
{
        printf("kixterm_cleanup.\n");

        xcb_free_cursor(kconf.connection, kconf.cursor[CUR_INVISBLE]);
        xcb_free_cursor(kconf.connection, kconf.cursor[CUR_NORMAL]);

        if (&kconf.ewmh)
                xcb_ewmh_connection_wipe(&kconf.ewmh);

        if (kconf.keysyms)
                xcb_key_symbols_free(kconf.keysyms);

        if (kconf.connection)
                xcb_disconnect(kconf.connection);

        xdgWipeHandle(&kconf.xdg);
}

static void signal_handler(int signal)
{
        if (signal == SIGINT) {
                printf("SIGINT handled\n");
                goto success;
        } else if(signal == SIGTERM) {
                printf("SIGTERM handled\n");
                goto success;
        }

success:
        exit(EXIT_SUCCESS);
}

static void signal_child(int signal)
{
        int stat = 0;

        if (waitpid(pid, &stat, 0) < 0) {
                fprintf(stderr, "waidpid() failed.\n");
                exit(EXIT_FAILURE);
        }

        if (WIFEXITED(stat))
                exit(WEXITSTATUS(stat));
        else
                exit(EXIT_FAILURE);
}

static void init_atoms(void)
{
        kconf.atom[ATOM_PRIMARY] = XCB_ATOM_PRIMARY;
        kconf.atom[ATOM_CLIPBOARD] = kixterm_init_atom(kconf.connection, "CLIPBOARD", true);
        kconf.atom[ATOM_TARGETS] = kixterm_init_atom(kconf.connection, "TARGETS", true);
        kconf.atom[ATOM_WM_PROTOCOLS] = kixterm_init_atom(kconf.connection, "WM_PROTOCOLS", false);
        kconf.atom[ATOM_WM_DELETE_WINDOW] = kixterm_init_atom(kconf.connection, "WM_DELETE_WINDOW", true);
        kconf.atom[ATOM_XROOT_PIXMAP_ID] = kixterm_init_atom(kconf.connection, "_XROOTPMAP_ID", true);
        kconf.atom[ATOM_ESETROOT_PIXMAP_ID] = kixterm_init_atom(kconf.connection, "ESETROOT_PMAP_ID", true);
        kconf.atom[ATOM_NET_WM_WINDOW_OPACITY] = kixterm_init_atom(kconf.connection, "_NET_WM_WINDOW_OPACITY", true);
        kconf.atom[ATOM_UTF8_STRING] = kixterm_init_atom(kconf.connection, "UTF8_STRING", false);
        if (kconf.atom[ATOM_UTF8_STRING] == XCB_ATOM_NONE) {
                fprintf(stderr, "Atom UTF8_STRING not found, using STRING.");
                kconf.atom[ATOM_UTF8_STRING] = XCB_ATOM_STRING;
        }
        return;
}

static xcb_pixmap_t get_root_pixmap(xcb_connection_t *connection,
                                    xcb_screen_t *screen,
                                    xcb_atom_t atom)
{
        xcb_get_property_cookie_t cookie;
        xcb_get_property_reply_t *reply;
        xcb_pixmap_t *rootpixmap = NULL;

        cookie = xcb_get_property(connection,
                                  0,
                                  screen->root,
                                  atom,
                                  XCB_ATOM_PIXMAP,
                                  0,
                                  1);

        reply = xcb_get_property_reply(connection, cookie, NULL);

        if (reply &&
            (xcb_get_property_value_length(reply) == sizeof(xcb_pixmap_t))) {
                rootpixmap = (xcb_pixmap_t *)xcb_get_property_value(reply);
                fprintf(stderr, "Got the root pixmap value.\n");
        } else {
                fprintf(stderr, "Failed to get the root pixmap value.\n");
                *rootpixmap = XCB_NONE;
        }


        free(reply);

        return *rootpixmap;
}

static xcb_visualtype_t *kixterm_get_visual_type(xcb_screen_t *screen)
{
        xcb_depth_iterator_t d_iter;

        d_iter = xcb_screen_allowed_depths_iterator(screen);

        while (d_iter.rem) {
                xcb_visualtype_iterator_t v_iter;
                v_iter = xcb_depth_visuals_iterator(d_iter.data);

                while (v_iter.rem) {
                        if (kconf.screen->root_visual == v_iter.data->visual_id) {
                                return v_iter.data;
                        }
                        xcb_visualtype_next(&v_iter);
                }

                xcb_depth_next(&d_iter);
        }

        /* If not found, we return a NULL */
        return NULL;
}

static xcb_cursor_t normal_cursor_init(xcb_connection_t *connection)
{
        xcb_font_t font;
        xcb_void_cookie_t cookie;
        xcb_cursor_t cursor;
        uint16_t max = USHRT_MAX;
        xcb_generic_error_t *error = NULL;

        font = xcb_generate_id(connection);
        cookie = xcb_open_font_checked(connection,
                                       font,
                                       strlen(NORMAL_CURSOR_FONT_NAME),
                                       NORMAL_CURSOR_FONT_NAME);
        error = xcb_request_check(connection, cookie);
        if (error) {
                fprintf(stderr, "Could not open font %s. Error:%d.\n",
                        NORMAL_CURSOR_FONT_NAME, error->error_code);
                xcb_close_font(connection, font);
                free(error);
                error = NULL;
                return XCB_NONE;
        }


        cursor = xcb_generate_id(connection);
        xcb_create_glyph_cursor_checked(connection,
                                        cursor,
                                        font,
                                        font,
                                        NORMAL_CURSOR_ID,
                                        NORMAL_CURSOR_ID + 1,
                                        0, 0, 0,
                                        max / 2, max / 2, max / 2);
        error = xcb_request_check(connection, cookie);
        if (error) {
                fprintf(stderr, "Could not create cursor. Error: %d\n",
                        error->error_code);
                xcb_close_font(connection, font);
                free(error);
                error = NULL;
                return XCB_NONE;
        }

        return cursor;
}

static xcb_cursor_t invisible_cursor_init(xcb_connection_t *connection,
                                          xcb_screen_t *screen)
{
        xcb_pixmap_t pixmap;
        xcb_void_cookie_t cookie;
        xcb_cursor_t cursor;
        xcb_generic_error_t *error = NULL;

        pixmap = xcb_generate_id(connection);
        cookie = xcb_create_pixmap_checked(connection,
                                           1,
                                           pixmap,
                                           screen->root,
                                           1, 1);
        error = xcb_request_check(connection, cookie);
        if (error) {
                fprintf(stderr, "Could not create pixmap. Error: %d\n",
                        error->error_code);
                free(error);
                error = NULL;
                xcb_free_pixmap(connection, pixmap);
                return XCB_NONE;
        }

        cursor = xcb_generate_id(connection);
        cookie = xcb_create_cursor_checked(connection,
                                           cursor,
                                           pixmap,
                                           pixmap,
                                           0, 0, 0,
                                           0, 0, 0,
                                           1, 1);
        error = xcb_request_check(connection, cookie);
        if (error) {
                fprintf(stderr, "Could not create cursor. Error: %d.\n",
                        error->error_code);
                free(error);
                error = NULL;
                xcb_free_pixmap(connection, pixmap);
                return XCB_NONE;
        }

        return cursor;

}

PangoFontDescription * _font_new(const char *font_name,
                                 int font_size,
                                 bool normal,
                                 bool bold,
                                 bool italic)
{
        PangoFontDescription *font;

        font = pango_font_description_new();

        pango_font_description_set_family(font, font_name);
        pango_font_description_set_absolute_size(font, font_size * PANGO_SCALE);
        pango_font_description_set_weight(font,
                                          bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
        pango_font_description_set_style(font,
                                         italic ? PANGO_STYLE_OBLIQUE : PANGO_STYLE_NORMAL);

        return font;
}

void _get_font_size(PangoFontDescription *font,
                    uint16_t *width,
                    uint16_t *height)
{
        cairo_surface_t *surface;
        cairo_t *cairo;
        PangoLayout *layout;
        PangoRectangle i_rect, l_rect;

        surface = cairo_xcb_surface_create(kconf.connection,
                                           kconf.screen->root,
                                           kconf.visual,
                                           1, 1);

        cairo = cairo_create(surface);

        layout = pango_cairo_create_layout(cairo);
        pango_layout_set_font_description(layout, font);

        pango_layout_set_text(layout, "W", -1);
        pango_cairo_update_layout(cairo, layout);

        pango_layout_get_extents(layout, &i_rect, &l_rect);

        *width = l_rect.width / PANGO_SCALE;
        *height = l_rect.height / PANGO_SCALE;


        g_object_unref(layout);
        cairo_destroy(cairo);
        cairo_surface_destroy(surface);
}

void _font_free(PangoFontDescription *font)
{
        pango_font_description_free(font);
        font = NULL;
}

static void kixterm_font_init(void)
{
        char *font_name = NULL;
        int font_size;
        uint16_t height;
        uint16_t width;

        font_name = default_font_name;
        font_size = default_font_size;

        kfont.normal = _font_new(font_name, font_size,
                                 true, false, false);
        _get_font_size(kfont.normal, &width, &height);
        printf("Monospace normal font size: (W):%d, (H):%d\n", width, height);

        kfont.bold = _font_new(font_name, font_size,
                               false, true, false);
        _get_font_size(kfont.normal, &width, &height);
        printf("Monospace bold font size: (W):%d, (H):%d\n", width, height);

        kfont.bold = _font_new(font_name, font_size,
                               false, false, true);
        _get_font_size(kfont.normal, &width, &height);
        printf("Monospace italic font size: (W):%d, (H):%d\n", width, height);

        kfont.width = width;
        kfont.height = height;
}

static uint32_t get_color(void)
{
        xcb_alloc_color_cookie_t cookie;
        uint8_t r = 0x00 * 0xFF;
        uint8_t g = 0x00 * 0xFF;
        uint8_t b = 0x00 * 0xFF;
        xcb_alloc_color_reply_t *reply;
        uint32_t pixel;

        cookie = xcb_alloc_color(kconf.connection,
                                 kconf.screen->default_colormap,
                                 r, g, b);

        reply = xcb_alloc_color_reply(kconf.connection, cookie, NULL);

        pixel = reply->pixel;

        free(reply);

        return pixel;
}

static uint32_t get_visual_bell_color(void)
{
        xcb_alloc_color_cookie_t cookie;
        uint8_t r = 0x7F * 0xFF;
        uint8_t g = 0x7F * 0xFF;
        uint8_t b = 0x7F * 0xFF;
        xcb_alloc_color_reply_t *reply;
        uint32_t pixel;

        cookie = xcb_alloc_color(kconf.connection,
                                 kconf.screen->default_colormap,
                                 r, g, b);

        reply = xcb_alloc_color_reply(kconf.connection, cookie, NULL);

        pixel = reply->pixel;

        free(reply);

        return pixel;
}

static void kixterm_xcb_init(void)
{
        const char *str;
        xcb_screen_iterator_t screen_iter;
        int i;


        /*  Get display environment variable.
            If we don't get any, we use the default ":0".
         */
        str = getenv("DISPLAY");
        kconf.display = str ? str : ":0";

        /* Get XDG basedir */
        xdgInitHandle(&kconf.xdg);

        /* Open the connection to the X server */
        kconf.connection = xcb_connect(kconf.display, &kconf.default_screen);
        if (xcb_connection_has_error(kconf.connection)) {
                fprintf(stderr, "Cannot open connection to display server: %d\n",
                        xcb_connection_has_error(kconf.connection));
                exit(EXIT_FAILURE);
        }

        /* Get the screen */
        screen_iter = xcb_setup_roots_iterator(xcb_get_setup(kconf.connection));
        for (i = 0; i != kconf.default_screen; ++i)
                xcb_screen_next(&screen_iter);

        kconf.screen = screen_iter.data;

        /* Get visual information */
        kconf.visual = kixterm_get_visual_type(kconf.screen);
        if (kconf.visual == NULL) {
                fprintf(stderr, "Could not locate visual\n");
                exit(EXIT_FAILURE);
        }

        /* Get the file descriptor corresponding to the X connection */
        kconf.xfd = xcb_get_file_descriptor(kconf.connection);

        /* Get Key Symbols */
        kconf.keysyms = xcb_key_symbols_alloc(kconf.connection);
        if (kconf.keysyms == NULL) {
                fprintf(stderr, "Could not load key symbols\n");
                exit(EXIT_FAILURE);
        }

        /* EWMH */
        if (xcb_ewmh_init_atoms_replies(&kconf.ewmh,
                                        xcb_ewmh_init_atoms(kconf.connection,
                                                            &kconf.ewmh),
                                        NULL) == 0) {
                fprintf(stderr, "Could not initialise EWMH atoms\n");
                exit(EXIT_FAILURE);
        }

        /* Atoms */
        init_atoms();

        /* Root pixmap */
        kconf.pixmap = get_root_pixmap(kconf.connection,
                                       kconf.screen,
                                       kconf.atom[ATOM_XROOT_PIXMAP_ID]);
        if (kconf.pixmap == XCB_NONE) {
                fprintf(stderr, "Failed to find root pixmap for atom: _XROOTPMAP_ID\n");
                kconf.pixmap = get_root_pixmap(kconf.connection,
                                               kconf.screen,
                                               kconf.atom[ATOM_ESETROOT_PIXMAP_ID]);
                if (kconf.pixmap == XCB_NONE) {
                        fprintf(stderr, "Failed to find root pixmap for atom: ESETROOT_PMAP_ID\n");
                }
        }

        /* Cursor */
        kconf.cursor[CUR_NORMAL] = normal_cursor_init(kconf.connection);
        kconf.cursor[CUR_INVISBLE] = invisible_cursor_init(kconf.connection,
                                                           kconf.screen);

        /* Colours */

        /* Fonts */
        kixterm_font_init();

        return;
}

static void kixterm_exec_shell(xcb_window_t windowid)
{
        char **args;
        const struct passwd *passwd = getpwuid(getuid());
        char wid[sizeof(uint32_t) + 1];
        unsigned long w = windowid;

        printf("kixterm_exec_shell: Entering\n");

        unsetenv("COLUMNS");
        unsetenv("LINES");
        unsetenv("TERMCAP");

        if (passwd) {
                setenv("LOGNAME", passwd->pw_name, 1);
                setenv("USER", passwd->pw_name, 1);
                setenv("SHELL", passwd->pw_shell, 0);
                setenv("HOME", passwd->pw_dir, 0);
        }

        snprintf(wid, sizeof(wid), "%lu", w);

        setenv("WINDOWID", wid, 1);
        setenv("TERM", KIXTERM_TYPE, 1);

        signal(SIGCHLD, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGALRM, SIG_DFL);

        shell = getenv("SHELL");
        if (shell == NULL) {
                shell = "/bin/sh";
        }

        args = (char *[]){shell, "-i", NULL};

        printf("kixterm_exec_shell: calling execvp\n");
        execvp(args[0], args);
        printf("kixterm_exec_shell: return execvp\n");

        exit(EXIT_FAILURE);
}

static void kixterm_tty_init(xcb_window_t windowid)
{
        int master, slave;
        struct winsize win = {DEFAULT_ROWS, DEFAULT_COLS, 0, 0};

        printf("kixterm_tty_init: Entering\n");

        if (openpty(&master, &slave, NULL, NULL, &win) < 0) {
                fprintf(stderr, "openpty failed.\n");
                exit(EXIT_FAILURE);
        }

        switch(pid = fork()) {
        case -1:
                fprintf(stderr, "fork failed.\n");
                exit(EXIT_FAILURE);
        case 0:
                printf("kixterm_tty_init: Child...\n");
                setsid();
                dup2(slave, STDIN_FILENO);
                dup2(slave, STDOUT_FILENO);
                dup2(slave, STDERR_FILENO);
                if (ioctl(slave, TIOCSCTTY, NULL) < 0) {
                        fprintf(stderr, "ioctl TIOSCTTY failed.\n");
                        exit(EXIT_FAILURE);
                }
                close(slave);
                close(master);
                printf("kixterm_tty_init: Child : Calling exec shell...\n");
                kixterm_exec_shell(windowid);
                break;
        default:
                printf("kixterm_tty_init: Parent...\n");
                close(slave);
                mfd = master;
                signal(SIGCHLD, signal_child);
                break;
        }

}

static void kixterm_core_init(void)
{
        xcb_void_cookie_t cookie;
        xcb_window_t window;
        xcb_rectangle_t win_geometry;
        xcb_generic_error_t *error = NULL;
        xcb_gcontext_t gc;
        uint32_t win_values[] = {
                get_color(),
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
                kconf.cursor[CUR_NORMAL]
        };
        uint32_t gc_values[] = {
                get_visual_bell_color(),
                0
        };

        win_geometry.width = 2 * DEFAULT_BORDER_WD + DEFAULT_COLS * kfont.width + DEFAULT_SCROLLBAR_WD;
        win_geometry.height = 2 * DEFAULT_BORDER_WD + DEFAULT_ROWS * kfont.height;

        window = xcb_generate_id(kconf.connection);

        cookie = xcb_create_window_checked(kconf.connection,
                                           kconf.screen->root_depth,
                                           window,
                                           kconf.screen->root,
                                           DEFAULT_X, DEFAULT_Y,
                                           win_geometry.width, win_geometry.height,
                                           0,
                                           XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                           kconf.screen->root_visual,
                                           XCB_CW_BACK_PIXEL  |
                                           XCB_CW_BIT_GRAVITY |
                                           XCB_CW_WIN_GRAVITY |
                                           XCB_CW_BACKING_STORE |
                                           XCB_CW_SAVE_UNDER |
                                           XCB_CW_EVENT_MASK |
                                           XCB_CW_CURSOR,
                                           win_values);


        error = xcb_request_check(kconf.connection, cookie);
        if (error) {
                fprintf(stderr, "Could not create a window(%d)...Exiting!!\n", error->error_code);
                xcb_destroy_window(kconf.connection, window); /* XXX: Move 'window' to kconf?? */
                exit(EXIT_FAILURE);
        }
        gc = xcb_generate_id(kconf.connection);
        cookie = xcb_create_gc_checked(kconf.connection,
                                       gc,
                                       window,
                                       XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES,
                                       gc_values);
        error = xcb_request_check(kconf.connection, cookie);
        if (error) {
                fprintf(stderr, "Could not create a GC...Exiting!!\n");
                xcb_destroy_window(kconf.connection, window); /* XXX: Move 'window' to kconf?? */
                exit(EXIT_FAILURE);
        }

        /* Create terminal */
        kixterm_tty_init(window);

        /* Map the window */
        cookie = xcb_map_window_checked(kconf.connection, window);
        error = xcb_request_check(kconf.connection, cookie);
        if (error) {
                fprintf(stderr, "Could not map window... Exiting!!\n");
                exit(EXIT_FAILURE);
        }

        xcb_flush(kconf.connection);

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
                fprintf(stdout, "XCB_MAP_NOTIFY.\n");
                break;
        case XCB_UNMAP_NOTIFY:
                fprintf(stdout, "XCB_UNMAP_NOTIFY.\n");
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

                event = xcb_poll_for_event(kconf.connection);

                if (event == NULL)
                        break;

                response_type = XCB_EVENT_RESPONSE_TYPE(event);
                if (response_type != 0)
                        handle_x_response(response_type, event);
        }

        if (xcb_connection_has_error(kconf.connection)) {
                fprintf(stderr, "Error with X connection.\n");
                exit(EXIT_FAILURE);
        }

        if (event)
                free(event);
}

static void kixterm_main_loop(void)
{
        int _xfd = kconf.xfd;
        fd_set fds;
        struct timeval *tv = NULL;

        while (1) {
                FD_ZERO(&fds);
                FD_SET(mfd, &fds);
                FD_SET(_xfd, &fds);

                if (select(MAX(_xfd, mfd)+1, &fds, NULL, NULL, tv) < 0) {
                        if (errno == EINTR)
                                continue;

                        fprintf(stderr, "select() failed.\n");
                        exit(EXIT_FAILURE);
                }

                if (FD_ISSET(mfd, &fds)) {
                        //printf("from the master..\n");
                }

                if (FD_ISSET(_xfd, &fds)) {
                        printf("from X..\n");
                        handle_from_x();
                }

        }

        return;
}

int main(int argc, char **argv)
{
        /* Cleanup and signal handling */
        atexit(kixterm_cleanup);
        signal(SIGTERM, signal_handler);
        signal(SIGINT, signal_handler);

        /* Set the right locale */
        setlocale(LC_CTYPE, "");

        kixterm_xcb_init();

        kixterm_core_init();

        xcb_flush(kconf.connection);

        kixterm_main_loop();

        exit(EXIT_SUCCESS);
}
