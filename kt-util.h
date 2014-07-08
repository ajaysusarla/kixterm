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

#define error(str, ...) _print("ERR: ",         \
                               true,            \
                               __LINE__,        \
                               __FUNCTION__,    \
                               str,             \
                               ## __VA_ARGS__)

#define warn(str, ...) _print("WARN: ",         \
                              false,            \
                              __LINE__,         \
                              __FUNCTION__,     \
                              str,              \
                              ## __VA_ARGS__)

#define debug(str, ...) _print("DEBUG: ",       \
                               false,           \
                               __LINE__,        \
                               __FUNCTION__,    \
                               str,             \
                               ## __VA_ARGS__)

void _print(const char *, bool, int, const char *, const char *, ...);


#endif /* KT_UTIL_H */
