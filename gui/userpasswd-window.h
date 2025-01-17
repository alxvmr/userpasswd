#ifndef USERPASSWDWINDOW_H
#define USERPASSWDWINDOW_H
#include <adwaita.h>
#include <gtk/gtk.h>
#include <glib.h>

G_BEGIN_DECLS

#define USERPASSWD_TYPE_WINDOW (userpasswd_window_get_type ())

G_DECLARE_FINAL_TYPE (UserpasswdWindow, userpasswd_window, USERPASSWD, WINDOW, AdwApplicationWindow)

G_END_DECLS

#endif