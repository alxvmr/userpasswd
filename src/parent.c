
/*
  $Id$
  Copyright (C) 2002  Stanislav Ievlev <inger@altlinux.org>
  Copyright (C) 2002  Dmitry V. Levin <ldv@altlinux.org>

  parent handler

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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>

#include "userpasswd.h"

static int
write_loop (int fd, const char *buffer, ssize_t count)
{
	ssize_t offset = 0;

	while (count > 0)
	{
		ssize_t block = write (fd, &buffer[offset], count);

		if (block < 0)
		{
			if (errno == EINTR)
				continue;
			return block;
		}
		if (!block)
			break;
		offset += block;
		count -= block;
	}
	return offset;
}

/*
 * parent process
 */
conv_state
do_parent (int master)
{
	conv_state state = CONV_WAIT_CURRENT;
	ssize_t count;
	char   *new_pw = 0;
	char    masterbuf[BUFSIZ];

	/*
	 * read and parse child output
	 */
	while ((count = read (master, masterbuf, sizeof (masterbuf) - 1)) > 0)
	{
		static const char str_current[] = "Enter current password:";
		static const char str_new[] = "Enter new password:";
		static const char str_retype[] = "Re-type new password:";

		masterbuf[count] = '\0';
		write (STDOUT_FILENO, masterbuf, count);

		if (strstr (masterbuf, str_current))
		{
			if (CONV_WAIT_CURRENT == state)
			{
				char   *pw = get_current_pw ();
				size_t  len = strlen (pw);

				count = write_loop (master, pw, len);
				memset (pw, 0, len);
				free (pw);
				if (count != len)
				{
					error (EXIT_SUCCESS,
					       errno,
					       "write current password to child");
					return CONV_ERR;
				}
				state = CONV_WAIT_NEW;
			} else
			{
				return CONV_ERR;
			}
		} else if (strstr (masterbuf, str_new))
		{
			if ((CONV_WAIT_CURRENT == state)
			    || (CONV_WAIT_NEW == state))
			{
				size_t  len;

				new_pw = get_new_pw ();
				len = strlen (new_pw);

				if (write_loop (master, new_pw, len) != len)
				{
					memset (new_pw, 0, len);
					free (new_pw);
					new_pw = 0;
					error (EXIT_SUCCESS,
					       errno,
					       "write new password to child");
					return CONV_ERR;
				}
				state = CONV_WAIT_RETYPE;
			} else if (CONV_WAIT_RETYPE == state)
			{
				return CONV_WAIT_RETYPE;
			} else
			{
				return CONV_ERR;
			}
		} else if (strstr (masterbuf, str_retype))
		{
			if (CONV_WAIT_RETYPE == state)
			{
				size_t  len;

				if (!new_pw)
					new_pw = get_new_pw ();
				len = strlen (new_pw);

				if (write_loop (master, new_pw, len) != len)
				{
					memset (new_pw, 0, len);
					free (new_pw);
					new_pw = 0;
					error (EXIT_SUCCESS,
					       errno,
					       "write retype password to child");
					return CONV_ERR;
				}
				state = CONV_DONE;
			} else
			{
				return CONV_ERR;
			}
		}
	}

	return state;
}
