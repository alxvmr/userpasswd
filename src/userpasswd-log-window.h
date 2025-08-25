#ifndef USERPASSDLOGWINDOW_H
#define USERPASSDLOGWINDOW_H
#ifdef USE_ADWAITA
    #include <adwaita.h>
#endif
#include <gtk/gtk.h>
#include <glib.h>
#include <locale.h>

#define _(STRING) gettext(STRING)

G_BEGIN_DECLS

#define USERPASSWD_TYPE_LOGWINDOW (userpasswd_logwindow_get_type ())

#ifdef USE_ADWAITA
    G_DECLARE_FINAL_TYPE (UserpasswdLogWindow, userpasswd_logwindow, USERPASSWD, LOGWINDOW, AdwWindow)
#else
    G_DECLARE_FINAL_TYPE (UserpasswdLogWindow, userpasswd_logwindow, USERPASSWD, LOGWINDOW, GtkWindow)
#endif

typedef struct _UserpasswdLogWindow {
#ifdef USE_ADWAITA
    AdwWindow parent_instance;
#else
    GtkWindow parent_instance;
#endif

    GtkWidget *info;

} UserpasswdLogWindow;

G_END_DECLS

#endif
