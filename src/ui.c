
/*
  $Id$
  Copyright (C) 2002  Stanislav Ievlev <inger@altlinux.org>
  Copyright (C) 2002  Dmitry V. Levin <ldv@altlinux.org>

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

#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "userpasswd.h"

#define PIXMAPDIR "/usr/share/pixmaps/userpasswd-"

extern const char *__progname;

static int ui_rc;

/*
 * create pixmap
 */
static GtkWidget *
create_pixmap (GtkWidget * widget, const char *fname)
{
	GdkBitmap *mask;
	GtkStyle *style = gtk_widget_get_style (widget);
	GdkPixmap *pixmap = gdk_pixmap_create_from_xpm (widget->window, &mask,
							&style->
							bg[GTK_STATE_NORMAL],
							fname);

	return gtk_pixmap_new (pixmap, mask);
}


static void
ui_gtk_quit (void)
{
	ui_rc = EXIT_FAILURE;
	gtk_main_quit ();
}

/*
 * display dialog with some messages
 */
static void
display_dialog (const char *pixname, const char *message, const char *title)
{
	GtkWidget *window, *hbox, *pix, *label, *button;

	ui_rc = 0;

	if (!message)
		return;

	/* window */
	window = gtk_dialog_new ();
	gtk_window_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_window_set_title (GTK_WINDOW (window), title ? : __progname);
	gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);
	gtk_signal_connect (GTK_OBJECT (window), "destroy",
			    GTK_SIGNAL_FUNC (ui_gtk_quit),
			    GTK_OBJECT (window));
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (ui_gtk_quit),
			    GTK_OBJECT (window));
	gtk_signal_connect (GTK_OBJECT (window), "hide",
			    GTK_SIGNAL_FUNC (ui_gtk_quit),
			    GTK_OBJECT (window));
	gtk_widget_show (window);

	/* hbox */
	hbox = gtk_hbox_new (1, 0);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), hbox, 0, 0,
			    5);

	/* pixmap */
	pix = create_pixmap (window, pixname);
	gtk_box_pack_start (GTK_BOX (hbox), pix, 0, 0, 5);

	/* label */
	label = gtk_label_new (message);
	gtk_box_pack_end (GTK_BOX (hbox), label, 0, 0, 5);

	/* button */
	button = gtk_button_new_with_label (_("Ok"));
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
	gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
				   (GtkSignalFunc) gtk_widget_destroy,
				   (gpointer) window);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (window)->action_area), button,
			  1, 1, 5);

	/* window */
	gtk_widget_show_all (window);
	gtk_widget_grab_default (button);

	gtk_main ();

	return;
}

void
display_message (const char *message, const char *title)
{
	display_dialog (PIXMAPDIR "message.xpm", message, title);
}

void
display_error (const char *message, const char *title)
{
	display_dialog (PIXMAPDIR "error.xpm", message, title);
}

static void
ungrab_focus (void)
{
	gdk_keyboard_ungrab (GDK_CURRENT_TIME);
	gdk_pointer_ungrab (GDK_CURRENT_TIME);
	gdk_flush ();
}

static void
grab_focus (GtkWidget * widget)
{
	unsigned i, max = 10;
	GtkWidget *toplevel = gtk_widget_get_toplevel (widget);

	for (i = 0; i < max; ++i)
	{
		if (i)
			usleep (100000);
		if (!gdk_pointer_grab (toplevel->window, TRUE, 0, NULL, NULL,
				       GDK_CURRENT_TIME))
			break;
	}

	if (i >= max)
	{
		ungrab_focus ();
		error (EXIT_FAILURE, 0, "Could not grab pointer");
	}

	for (i = 0; i < max; ++i)
	{
		if (i)
			usleep (100000);
		if (!gdk_keyboard_grab
		    (toplevel->window, FALSE, GDK_CURRENT_TIME))
			break;
	}

	if (i >= max)
	{
		ungrab_focus ();
		error (EXIT_FAILURE, 0, "Could not grab keyboard");
	}
}

/*
 * ask for password
 */
static char *
get_pw (const char *title, const char *message, const char *prompt,
	const char *pixfile)
{
	GtkWidget *window, *ok_button, *cancel_button;
	GtkWidget *entry, *entry_label, *entry_hbox;
	GtkWidget *pix, *message_label, *message_hbox;

	char   *pw = 0;
	void    set_pw (GtkWidget * button, GtkWidget * entry)
	{
		if (asprintf
		    (&pw, "%s\n", gtk_entry_get_text (GTK_ENTRY (entry))) < 0)
			error (EXIT_FAILURE, errno, "asprintf");
		gtk_entry_set_text (GTK_ENTRY (entry), "");
#ifdef DEBUG
		printf ("pw entered: %s", pw);
#endif
	}

	ui_rc = 0;

	/* window */
	window = gtk_dialog_new ();
	gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);
	gtk_window_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_window_set_title (GTK_WINDOW (window), title);
	gtk_signal_connect (GTK_OBJECT (window), "destroy",
			    GTK_SIGNAL_FUNC (ui_gtk_quit),
			    GTK_OBJECT (window));
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (ui_gtk_quit),
			    GTK_OBJECT (window));
	gtk_signal_connect (GTK_OBJECT (window), "hide",
			    GTK_SIGNAL_FUNC (ui_gtk_quit),
			    GTK_OBJECT (window));
	gtk_widget_show (window);

	/* entry */
	entry = gtk_entry_new ();
	gtk_entry_set_visibility (GTK_ENTRY (entry), 0);
	gtk_widget_grab_focus (entry);

	/* ok_button */
	ok_button = gtk_button_new_with_label (_("Ok"));
	gtk_signal_connect (GTK_OBJECT (ok_button), "clicked", GTK_SIGNAL_FUNC(set_pw), entry);
	gtk_signal_connect_object (GTK_OBJECT (ok_button), "clicked",
				   (GtkSignalFunc) gtk_widget_hide,
				   (gpointer) window);
	gtk_signal_connect_object (GTK_OBJECT (entry), "activate",
				   (GtkSignalFunc) gtk_button_clicked,
				   (gpointer) GTK_BUTTON (ok_button));

	/* cancel_button */
	cancel_button = gtk_button_new_with_label (_("Cancel"));
	GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);
	gtk_signal_connect_object (GTK_OBJECT (cancel_button), "clicked",
				   (GtkSignalFunc) gtk_widget_hide,
				   (gpointer) window);
	gtk_signal_connect_object (GTK_OBJECT (entry), "activate",
				   (GtkSignalFunc) gtk_button_clicked,
				   (gpointer) GTK_BUTTON (cancel_button));

	/* message_hbox */
	message_hbox = gtk_hbox_new (1, 5);

	/* pixmap */
	pix = create_pixmap (window, pixfile);
	gtk_box_pack_start (GTK_BOX (message_hbox), pix, 0, 0, 0);

	/* message_label */
	message_label = gtk_label_new (message);
	gtk_box_pack_end (GTK_BOX (message_hbox), message_label, 0, 0, 5);

	/* entry_hbox */
	entry_hbox = gtk_hbox_new (1, 5);

	/* entry_label */
	entry_label = gtk_label_new (prompt);
	gtk_box_pack_start (GTK_BOX (entry_hbox), entry_label, 0, 0, 5);

	gtk_box_pack_end (GTK_BOX (entry_hbox), entry, 0, 0, 5);

	/* window */
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), message_hbox,
			    0, 0, 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), entry_hbox,
			    0, 0, 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->action_area),
			    ok_button, 1, 1, 5);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (window)->action_area),
			  cancel_button, 1, 1, 5);
	gtk_widget_show_all (window);

	gtk_widget_grab_default (cancel_button);
	grab_focus (window);
	gtk_main ();
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
			     PIXMAPDIR "current.xpm");

	if (!pw)
		exit (ui_rc ?: 2);

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
		      _("Enter new password:"), PIXMAPDIR "new.xpm");
	if (!pw1)
		exit (ui_rc ?: 2);

	pw2 = get_pw (_("Re-type password"),
		      _("Now enter your\nnew password twice"),
		      _("Re-type new password:"), PIXMAPDIR "new.xpm");
	if (!pw2)
		exit (ui_rc ?: 2);
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
