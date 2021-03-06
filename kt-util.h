/*
 * kt-util.h - Utilities.
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

#ifndef KT_UTIL_H
#define KT_UTIL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <glib-object.h>

/* Custom color structure. Storing colors as individual R, B, G is convenient
   to use when working with xcb/pango/cairo.*/
typedef struct {
        guint8 r;
        guint8 g;
        guint8 b;
} kt_color_t;

/* HOSTNAME_MAX isn't available on all platforms */
#ifdef LINUX
#define HOSTNAME_MAX (HOST_NAME_MAX + 1)
#else
#define HOSTNAME_MAX 256
#endif

#define asizeof(x) (sizeof(x)/sizeof(x[0]))
#ifdef MAX
#undef MAX
#endif
#ifdef MIN
#undef MIN
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define NEW(type, count) ((type *)kt_malloc(sizeof(type) * (count)))
#define MEMSET(ptr, count) ((void)memset((ptr), 0, sizeof(*(ptr)) * (count)))
#define FREE(ptr)                               \
        do {                                    \
                void **__ptr = (void **) (ptr); \
                free(*__ptr);                   \
                *(void **)__ptr = NULL;         \
        } while(0)

#define error(str, ...) _print("ERR",           \
                               false,           \
                               __LINE__,        \
                               __FUNCTION__,    \
                               str,             \
                               ## __VA_ARGS__)

#define warn(str, ...) _print("WARN",           \
                              false,            \
                              __LINE__,         \
                              __FUNCTION__,     \
                              str,              \
                              ## __VA_ARGS__)

#define debug(str, ...) _print("DEBUG",         \
                               false,           \
                               __LINE__,        \
                               __FUNCTION__,    \
                               str,             \
                               ## __VA_ARGS__)

void _print(const char *, bool, int, const char *, const char *, ...);

static inline void * __attribute__ ((malloc)) kt_malloc(ssize_t size)
{
        void *ptr;

        if (size <= 0)
                return NULL;

        ptr = calloc(1, size);
        if (ptr == NULL) {
                warn("Memory allocation failure.");
                abort();
        }

        return ptr;
}

#endif /* KT_UTIL_H */
