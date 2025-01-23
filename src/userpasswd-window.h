#ifndef USERPASSWDWINDOW_H
#define USERPASSWDWINDOW_H
#include <adwaita.h>
#include <gtk/gtk.h>
#include <glib.h>

G_BEGIN_DECLS

#define USERPASSWD_TYPE_WINDOW (userpasswd_window_get_type ())

G_DECLARE_FINAL_TYPE (UserpasswdWindow, userpasswd_window, USERPASSWD, WINDOW, AdwApplicationWindow)

typedef struct _UserpasswdWindow {
    AdwApplicationWindow parent_instance;

    GtkWidget *container; //vbox
    GtkWidget *container_password; //lbox
    GMenu *menu;
    GtkWidget *menu_button;
    GtkWidget *header_bar;
    GtkWidget *toolbar;

    GtkWidget *status;
    AdwClamp *clamp_status;
    AdwClamp *clamp_info;
    GtkWidget *info;
    GtkWidget *expander_status;

    gchar *log_mess;

    AdwPasswordEntryRow *current_password_row;
    AdwPasswordEntryRow *new_password_row;
    AdwPasswordEntryRow *repeat_new_password_row;
    GtkWidget *check_password_button;
    GtkWidget *change_password_button;

} UserpasswdWindow;

void cb_check_password_fail (gpointer *stream, UserpasswdWindow *window);
void cb_new_log (gpointer *stream, gchar *log, UserpasswdWindow *window);
void cb_draw_check_passwd (gpointer *stream, UserpasswdWindow *window);
void cb_draw_new_passwd (gpointer *stream, UserpasswdWindow *window);

G_END_DECLS

#endif