
/*
  Copyright (C) 2002  Stanislav Ievlev <inger@altlinux.org>
  Copyright (C) 2002  Dmitry V. Levin <ldv@altlinux.org>

  conversation

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
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <sys/wait.h>
#include <libintl.h>

#include "userpasswd.h"

/*
 * returns status of the child process
 */
static int
get_child_exit_code (pid_t child)
{
	int     status;
	int     child_rc = 0;

	signal (SIGCHLD, SIG_IGN);
	if (waitpid (child, &status, 0) != child)
		error (EXIT_FAILURE, errno, "waitpid");

	if (WIFEXITED (status))
	{
		if (WEXITSTATUS (status))
		{
#ifdef DEBUG
			printf ("child %d exited with return code %d.\n",
				child, WEXITSTATUS (status));
#endif
			child_rc = WEXITSTATUS (status);
			return child_rc;
		}
#ifdef DEBUG
		printf ("child %d exited normally.\n", child);
#endif
		return child_rc;
	}

	if (WIFSIGNALED (status))
	{
		error (EXIT_SUCCESS, errno,
		       "child %d terminated with signal %d.\n", child,
		       WTERMSIG (status));
		child_rc = 128 + WTERMSIG (status);
		return child_rc;
	}
}

/*
 * main conversation function
 */
int
do_conv (int master)
{
	char   *name = ptsname (master);

	if (!name)
		error (EXIT_FAILURE, errno, "ptsname");

#ifdef DEBUG
	printf ("ptsname: %s\n", name);
#endif

	pid_t child = fork ();
	if (child < 0)
		error (EXIT_FAILURE, errno, "fork");
	if (!child)
	{
		do_child (name);
	}

	conv_state state = do_parent (master);
	int child_rc = get_child_exit_code(child);

	switch (state)
	{
		case CONV_WAIT_CURRENT:
			display_error (_("System configuration error."), 0);
			break;
		case CONV_WAIT_NEW:
			display_error (_
				       ("Wrong password.\nPlease try again."),
				       0);
			return 2;
		case CONV_WAIT_RETYPE:
			display_error (_("Weak password.\nPlease try again."),
				       0);
			return 2;
		case CONV_DONE:
			if (!child_rc)
			{
				display_message (_
						 ("Password sucessully changed."),
						 0);
				return EXIT_SUCCESS;
			}
			/* fall through */
		default:
			display_message (_("Password conversation error."),
					 0);
			break;
	}
	return child_rc ? : EXIT_FAILURE;
}
