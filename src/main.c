
/*
  $Id$
  Copyright (C) 2002  Stanislav Ievlev <inger@altlinux.org>
  Copyright (C) 2002  Dmitry V. Levin <ldv@altlinux.org>

  passwd wrapper

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
#include <errno.h>
#include <error.h>
#include <libintl.h>
#include <gtk/gtkmain.h>
#include "userpasswd.h"

int
main (int argc, const char *argv[])
{
	int     master = getpt ();

	if (master < 0)
		error (EXIT_FAILURE, errno, "getpt");

	if (grantpt (master) < 0)
		error (EXIT_FAILURE, errno, "grantpt");

	if (unlockpt (master) < 0)
		error (EXIT_FAILURE, errno, "unlockpt");

	textdomain (PACKAGE);
	bindtextdomain (PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");

	gtk_set_locale ();
	gtk_init (&argc, (char ***) &argv);

	return do_conv (master);
}
