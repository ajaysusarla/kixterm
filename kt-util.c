/*
 * kt-util.c - Utilities.
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

#include <stdarg.h>

#include "kt-util.h"

void _print(const char *type, bool quit, int line,
            const char *func, const char *fmt, ...)
{
        va_list ap;

        va_start(ap, fmt);
        fprintf(stderr, "[%s] kixterm: %s+%d: ", type, func, line);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
        fprintf(stderr, "\n");

        if (quit)
                exit(EXIT_FAILURE);
}
