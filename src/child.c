
/*
  $Id$
  Copyright (C) 2002  Stanislav Ievlev <inger@altlinux.org>
  Copyright (C) 2002  Dmitry V. Levin <ldv@altlinux.org>

  child handler

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>

#ifdef __linux__
#include <linux/limits.h>
#endif

/*
 * setup file descriptors
 */
static void
setup_fds (const int *fd)
{
	int     i;

	for (i = 0; i < 3; ++i)
		if ((fd[i] != i) && (dup2 (fd[i], i) != i))
			error (EXIT_FAILURE, errno, "dup2");
}

/*
 * close unneeded file descriptors
 */
static void
closeall_fds (int start)
{
	int     fd, max;

	max = sysconf (_SC_OPEN_MAX);
	if (max <= 0)
		max = 1024;

#ifdef __linux__
	if (max < NR_OPEN)
		max = NR_OPEN;
#endif

	for (fd = start; fd < max; ++fd)
		close (fd);
}

/*
 * cleanup environment
 */
static void
sanitize_env (void)
{
	const char *restricted_envvars[] = {
		"LANG",
		"LANGUAGE",
		"LINGUAS",
		"LC_CTYPE",
		"LC_NUMERIC",
		"LC_TIME",
		"LC_COLLATE",
		"LC_MONETARY",
		"LC_MESSAGES",
		"LC_PAPER",
		"LC_NAME",
		"LC_ADDRESS",
		"LC_TELEPHONE",
		"LC_MEASUREMENT",
		"LC_IDENTIFICATION",
		"LC_ALL",
		"LC_XXX"
	};
	size_t  cnt;

	for (cnt = 0;
	     cnt <
	     sizeof (restricted_envvars) / sizeof (restricted_envvars[0]);
	     ++cnt)
		unsetenv (restricted_envvars[cnt]);
}

/*
 * child process
 */
void
do_child (const char *name)
{
	int     slave_r, slave_w;

	if (setsid () < 0)
		error (EXIT_FAILURE, errno, "setsid");

	slave_r = open (name, O_RDONLY);
	if (slave_r < 0)
		error (EXIT_FAILURE, errno, "open ro %s", name);

	if (isastream (slave_r))
	{
		if (ioctl (slave_r, I_PUSH, "ptem") < 0
		    || ioctl (slave_r, I_PUSH, "ldterm") < 0)
			error (EXIT_FAILURE, errno, "ioctl slave_r");
	}

	slave_w = open (name, O_WRONLY);
	if (slave_w < 0)
		error (EXIT_FAILURE, errno, "open rw %s", name);

	if (isastream (slave_w))
	{
		if (ioctl (slave_w, I_PUSH, "ptem") < 0
		    || ioctl (slave_w, I_PUSH, "ldterm") < 0)
			error (EXIT_FAILURE, errno, "ioctl slave_w");
	}

	{
		int     fd[3] = { slave_r, slave_w, slave_w };

		setup_fds (fd);
	}

	closeall_fds (1 + STDERR_FILENO);

	sanitize_env ();

	{
		const char *prog = "/usr/bin/passwd";
		char   *const args[] = { "passwd", 0 };

		execv (prog, args);
		error (EXIT_FAILURE, errno, "execv: %s", args[0]);
	}
}
