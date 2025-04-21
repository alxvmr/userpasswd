
/*
  Copyright (C) 2002,2003  Stanislav Ievlev <inger@altlinux.org>
  Copyright (C) 2002-2005  Dmitry V. Levin <ldv@altlinux.org>

  interface functions for passwd wrapper

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
#include <errno.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "userpasswd.h"

#define PIXMAP_PREFIX "/usr/share/pixmaps/userpasswd-"

/*
 * display dialog with some messages
 */
static void
display_dialog (const char *pixname, const char *message, const char *title)
{
	GtkWidget *window, *hbox, *pix, *label;

	if (!message)
		return;

	/* window */
	window = gtk_dialog_new_with_buttons (title ? :
					      program_invocation_short_name,
					      NULL, 0, GTK_STOCK_OK,
					      GTK_RESPONSE_ACCEPT, NULL);

	gtk_window_set_wmclass (GTK_WINDOW (window), "org.altlinux.userpasswd", "org.altlinux.userpasswd");
	gtk_window_set_resizable (GTK_WINDOW (window), 0);
	gtk_window_set_decorated (GTK_WINDOW (window), 1);
	gtk_window_set_position (GTK_WINDOW (window),
				 GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_keep_above (GTK_WINDOW (window), 1);

	/* hbox */
	hbox = gtk_hbox_new (1, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), hbox, 0, 0,
			    5);

	/* pixmap */
	pix = gtk_image_new_from_file (pixname);
	gtk_box_pack_start (GTK_BOX (hbox), pix, 0, 0, 5);

	/* label */
	label = gtk_label_new (message);
	gtk_box_pack_end (GTK_BOX (hbox), label, 0, 0, 5);

	/* run */
	gtk_widget_show_all (window);
	gtk_dialog_run (GTK_DIALOG (window));
	gtk_widget_destroy (window);

	return;
}

void
display_message (const char *message, const char *title)
{
	display_dialog (PIXMAP_PREFIX "info.png", message, title);
}

void
display_error (const char *message, const char *title)
{
	display_dialog (PIXMAP_PREFIX "critical.png", message, title);
}

static void
ungrab_focus (void)
{
	gdk_keyboard_ungrab (GDK_CURRENT_TIME);
	gdk_pointer_ungrab (GDK_CURRENT_TIME);
	gdk_flush ();
}

volatile static sig_atomic_t timed_out;

void
alarm_handler (int signo)
{
	timed_out = 1;
}

static void
grab_focus (GtkWidget * widget)
{
	GtkWidget *toplevel = gtk_widget_get_toplevel (widget);
	struct sigaction action;

	action.sa_handler = alarm_handler;
	action.sa_flags = SA_RESTART;
	sigemptyset (&action.sa_mask);

	if (sigaction (SIGALRM, &action, 0) < 0)
		error (EXIT_FAILURE, errno, "sigaction");

	timed_out = 0;
	alarm (1);
	while (!timed_out)
	{
		if (!gdk_pointer_grab
		    (toplevel->window, TRUE, 0, NULL, NULL, GDK_CURRENT_TIME))
		{
			alarm (0);
			timed_out = 0;
			break;
		}
	}

	if (timed_out)
	{
		ungrab_focus ();
		error (EXIT_FAILURE, 0, "Could not grab pointer");
	}

	timed_out = 0;
	alarm (1);
	while (!timed_out)
	{
		if (!gdk_keyboard_grab
		    (toplevel->window, FALSE, GDK_CURRENT_TIME))
		{
			alarm (0);
			timed_out = 0;
			break;
		}
	}

	if (timed_out)
	{
		ungrab_focus ();
		error (EXIT_FAILURE, 0, "Could not grab keyboard");
	}
}

void    entry_activated (GtkDialog * dialog)
{
	gtk_dialog_response (dialog, GTK_RESPONSE_ACCEPT);
}

/*
 * ask for password
 */
static char *
get_pw (const char *title, const char *message, const char *prompt,
	const char *pixfile)
{
	GtkWidget *window;
	GtkWidget *entry, *entry_label, *entry_hbox;
	GtkWidget *pix, *message_label, *message_hbox;

	char   *pw = 0;

	/* window */
	window = gtk_dialog_new_with_buttons (title ? :
					      program_invocation_short_name,
					      NULL, GTK_DIALOG_MODAL,
					      GTK_STOCK_OK,
					      GTK_RESPONSE_ACCEPT,
					      GTK_STOCK_CANCEL,
					      GTK_RESPONSE_REJECT, NULL);

	gtk_window_set_wmclass (GTK_WINDOW (window), "org.altlinux.userpasswd", "org.altlinux.userpasswd");
	gtk_window_set_resizable (GTK_WINDOW (window), 0);
	gtk_window_set_decorated (GTK_WINDOW (window), 1);
	gtk_window_set_keep_above (GTK_WINDOW (window), 1);
	gtk_window_set_position (GTK_WINDOW (window),
				 GTK_WIN_POS_CENTER_ALWAYS);
	gtk_container_set_border_width (GTK_CONTAINER (window), 8);

	/* message_hbox */
	message_hbox = gtk_hbox_new (1, 5);

	/* pixmap */
	pix = gtk_image_new_from_file (pixfile);
	gtk_box_pack_start (GTK_BOX (message_hbox), pix, 0, 0, 0);

	/* message_label */
	message_label = gtk_label_new (message);
	gtk_box_pack_end (GTK_BOX (message_hbox), message_label, 0, 0, 5);

	/* entry_hbox */
	entry_hbox = gtk_hbox_new (1, 5);

	/* entry_label */
	entry_label = gtk_label_new (prompt);
	gtk_box_pack_start (GTK_BOX (entry_hbox), entry_label, 0, 0, 5);

	/* entry */
	entry = gtk_entry_new ();
	gtk_box_pack_end (GTK_BOX (entry_hbox), entry, 0, 0, 5);
	gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);
	gtk_signal_connect_object (GTK_OBJECT (entry), "activate",
				   (GtkSignalFunc) entry_activated,
				   (gpointer) GTK_DIALOG (window));

	/* window */
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), message_hbox,
			    0, 0, 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), entry_hbox,
			    0, 0, 5);
	gtk_widget_grab_focus (entry);
	g_signal_connect (G_OBJECT (window), "map_event",
			  G_CALLBACK (grab_focus), (gpointer) window);

	gtk_widget_show_all (window);
	int     rc = gtk_dialog_run (GTK_DIALOG (window));

	if (rc == GTK_RESPONSE_ACCEPT)
	{
		if (asprintf
		    (&pw, "%s\n", gtk_entry_get_text (GTK_ENTRY (entry))) < 0)
			error (EXIT_FAILURE, errno, "asprintf");
		gtk_entry_set_text (GTK_ENTRY (entry), "");
#ifdef DEBUG
		printf ("pw entered: %s", pw);
#endif
	}

	gtk_widget_destroy (window);

	ungrab_focus ();

	return pw;
}

/*
 * ask current user password
 */
char   *
get_current_pw ()
{
	char   *pw = get_pw (_("Current password"),
			     _("Please enter your\ncurrent password first"),
			     _("Enter current password:"),
			     PIXMAP_PREFIX "keyring.png");

	if (!pw)
		exit (1);

	return pw;
}

/*
 * ask for the new user password
 */
char   *
get_new_pw ()
{
	char   *pw1, *pw2;

	pw1 = get_pw (_("New password"),
		      _("Now enter your\nnew password twice"),
		      _("Enter new password:"), PIXMAP_PREFIX "keyring.png");
	if (!pw1)
		exit (1);

	pw2 = get_pw (_("Re-type password"),
		      _("Now enter your\nnew password twice"),
		      _("Re-type new password:"),
		      PIXMAP_PREFIX "keyring.png");
	if (!pw2)
		exit (1);

	if (strcmp (pw1, pw2))
	{
		display_error (_
			       ("Sorry, passwords do not match.\nPlease try again."),
			       0);
		exit (2);
	}
	free (pw2);
	return pw1;
}
