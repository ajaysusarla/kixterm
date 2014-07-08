/*
 * main.c
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

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <signal.h>

#include "kt-pref.h"
#include "kt-util.h"

#include "kixterm.h"


kixterm_t conf;

static void cleanup(void)
{
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

int main(int argc, char **argv)
{
        /* Cleanup and signal handling */
        atexit(cleanup);
        signal(SIGTERM, signal_handler);
        signal(SIGINT, signal_handler);

        /* Set the right locale */
        setlocale(LC_CTYPE, "");

        exit(EXIT_SUCCESS);
}
