/*
 * kt-tty.c - term emulation and handling
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

#include "kt-tty.h"
#include "kt-util.h"

#include <unistd.h>
#include <signal.h>
#include <pty.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>

/* FIXME:::: All these *should* go to the config/perfs */
#define DEFAULT_ROWS 24
#define DEFAULT_COLS 80
#define KIXTERM_TYPE "linux"
static char *shell = NULL;

/* Internal functions */
static void signal_child(int signal)
{
        int stat = 0;

        if (waitpid(conf.pid, &stat, 0) < 0) {
                fprintf(stderr, "waidpid() failed.\n");
                exit(EXIT_FAILURE);
        }

        if (WIFEXITED(stat))
                exit(WEXITSTATUS(stat));
        else
                exit(EXIT_FAILURE);
}

static void kt_tty_exec_shell(xcb_window_t windowid)
{
        char **args;
        const struct passwd *passwd = getpwuid(getuid());
        char wid[sizeof(uint32_t) + 1];
        unsigned long w = windowid;

        debug("Entering..");

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

        debug("calling execvp");
        execvp(args[0], args);
        debug("return execvp\n");

        exit(EXIT_FAILURE);
}

/* External functions */

void kt_tty_init(xcb_window_t windowid)
{
        int master, slave;
        struct winsize win = {DEFAULT_ROWS, DEFAULT_COLS, 0, 0};

        debug("Entering..");

        if (openpty(&master, &slave, NULL, NULL, &win) < 0) {
                error("openpty() failed.");
        }

        switch(conf.pid = fork()) {
        case -1:
                error("fork() failed.");
        case 0:
                debug("Child process...");
                setsid();
                dup2(slave, STDIN_FILENO);
                dup2(slave, STDOUT_FILENO);
                dup2(slave, STDERR_FILENO);
                if (ioctl(slave, TIOCSCTTY, NULL) < 0) {
                        error("ioctl() for TIOSCTTY failed.");
                }
                close(slave);
                close(master);
                debug("Child process: Calling exec shell...");
                kt_tty_exec_shell(windowid);
                break;
        default:
                debug("Parent process...");
                close(slave);
                conf.mfd = master;
                signal(SIGCHLD, signal_child);
                break;
        }

}
