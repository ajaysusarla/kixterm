/*
 * kt-window.h
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
#ifndef KT_WINDOW_H
#define KT_WINDOW_H

#include <glib-object.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <cairo-xcb.h>

#include "kt-app.h"
#include "kt-prefs.h"
#include "kt-font.h"
#include "kt-color.h"

typedef struct _KtWindow KtWindow;
typedef struct _KtWindowClass KtWindowClass;
typedef struct _KtWindowPriv KtWindowPriv;

#define KT_WINDOW_TYPE (kt_window_get_type())
#define KT_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), KT_WINDOW_TYPE, KtWindow))
#define KT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_BINARY_TREE, KtWindowClass))
#define KT_IS_WINDOW(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), KT_WINDOW_TYPE))
#define KT_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), KT_WINDOW_TYPE))
#define KT_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), KT_WINDOW_TYPE, KtWindowClass))

struct _KtWindow {
        GObject parent_instance;

        KtWindowPriv *priv;
};

struct _KtWindowClass {
        GObjectClass parent_class;
};

GType kt_window_get_type(void);
KtWindow *kt_window_new(KtApp *app,
                        KtPrefs *prefs,
                        KtFont *font,
                        KtColor *color);

void kt_window_key_press(KtWindow *window, xcb_key_press_event_t *event);
void kt_window_key_release(KtWindow *window, xcb_key_release_event_t *event);
void kt_window_button_press(KtWindow *window, xcb_button_press_event_t *event);
void kt_window_button_release(KtWindow *window, xcb_button_release_event_t *event);
void kt_window_motion_notify(KtWindow *window, xcb_motion_notify_event_t *event);
void kt_window_expose(KtWindow *window, xcb_expose_event_t *event);
void kt_window_enter_notify(KtWindow *window, xcb_enter_notify_event_t *event);
void kt_window_leave_notify(KtWindow *window, xcb_leave_notify_event_t *event);
void kt_window_focus_in(KtWindow *window, xcb_focus_in_event_t *event);
void kt_window_focus_out(KtWindow *window, xcb_focus_out_event_t *event);
void kt_window_map_notify(KtWindow *window, xcb_map_notify_event_t *event);
void kt_window_unmap_notify(KtWindow *window, xcb_unmap_notify_event_t *event);
void kt_window_configure_notify(KtWindow *window, xcb_configure_notify_event_t *event);
void kt_window_destroy_notify(KtWindow *window, xcb_destroy_notify_event_t *event);
void kt_window_selection_clear(KtWindow *window, xcb_selection_clear_event_t *event);
void kt_window_selection_notify(KtWindow *window, xcb_selection_notify_event_t *event);
void kt_window_selection_request(KtWindow *window, xcb_selection_request_event_t *event);
void kt_window_client_message(KtWindow *window, xcb_client_message_event_t *event);
void kt_window_reparent_notify(KtWindow *window, xcb_reparent_notify_event_t *event);
void kt_window_property_notify(KtWindow *window, xcb_property_notify_event_t *event);

#endif /* KT_WINDOW_H */
