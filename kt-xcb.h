/*
 * kt-xcb.h - XCB initialization functions
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

#ifndef KT_XCB_H
#define KT_XCB_H

#include "kixterm.h"

void kt_xcb_init(void);
void kt_xcb_destroy(void);

uint32_t kt_xcb_get_color(void);
uint32_t kt_xcb_get_visual_bell_color(void);

/* Event handlers */
void kt_xcb_map_notify(xcb_map_notify_event_t *event);
void kt_xcb_unmap_notify(xcb_unmap_notify_event_t *event);


void kt_xcb_write_to_term(char *str, size_t size);
#endif /* KT_XCB_H */
