#ifndef USERPASSWDWINDOW_H
#define USERPASSWDWINDOW_H
#include <adwaita.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <libintl.h>
#include <locale.h>

#define _(STRING) gettext(STRING)

G_BEGIN_DECLS

#define USERPASSWD_TYPE_WINDOW (userpasswd_window_get_type ())

G_DECLARE_FINAL_TYPE (UserpasswdWindow, userpasswd_window, USERPASSWD, WINDOW, AdwApplicationWindow)

typedef struct _UserpasswdWindow {
    AdwApplicationWindow parent_instance;

    GtkWidget *container; //vbox
    GtkWidget *container_password; //lbox
    GtkWidget *container_data_input;
    GMenu *menu;
    GtkWidget *menu_button;
    GtkWidget *header_bar;
    GtkWidget *toolbar;

    GtkWidget *status_container;
    GtkWidget *status_mess;
    GtkWidget *substatus_mess;
    AdwClamp *clamp_status;
    AdwClamp *clamp_info;
    GtkWidget *info;
    GtkWidget *expander_status;

    gchar *log_mess;

    AdwPasswordEntryRow *current_password_row;
    AdwPasswordEntryRow *new_password_row;
    AdwPasswordEntryRow *repeat_new_password_row;
    GtkWidget *button;

    GtkWidget *spinner;

} UserpasswdWindow;

void cb_new_status (gpointer *stream, const gchar *status_mess, const gchar *status_type, UserpasswdWindow *window);
void cb_new_log (gpointer *stream, const gchar *log, const gchar *sender, UserpasswdWindow *window);
void cb_draw_check_passwd (gpointer *stream, UserpasswdWindow *window);
void cb_draw_new_passwd (gpointer *stream, UserpasswdWindow *window);

G_END_DECLS

#endif