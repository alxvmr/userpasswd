#ifndef USERPASSWDWINDOW_H
#define USERPASSWDWINDOW_H
#ifdef USE_ADWAITA
    #include <adwaita.h>
#endif
#include <gtk/gtk.h>
#include <glib.h>
#include <libintl.h>
#include <locale.h>

#define _(STRING) gettext(STRING)

G_BEGIN_DECLS

#define USERPASSWD_TYPE_WINDOW (userpasswd_window_get_type ())

#ifdef USE_ADWAITA
    G_DECLARE_FINAL_TYPE (UserpasswdWindow, userpasswd_window, USERPASSWD, WINDOW, AdwApplicationWindow)
#else
    G_DECLARE_FINAL_TYPE (UserpasswdWindow, userpasswd_window, USERPASSWD, WINDOW, GtkApplicationWindow)
#endif

typedef struct _UserpasswdWindow {
#ifdef USE_ADWAITA
    AdwApplicationWindow parent_instance;
#else
    GtkApplicationWindow parent_instance;
#endif

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
    GtkWidget *info;
    GtkWidget *expander_status;

    gchar *log_mess;

    GtkWidget *current_password_row;
    GtkWidget *new_password_row;
    GtkWidget *repeat_new_password_row;

    GtkWidget *strength_indicator;
    GtkWidget *strength_indicator_label;
    GtkWidget *password_not_match_label;

    GtkWidget *button;

    GtkWidget *spinner;

} UserpasswdWindow;

void cb_new_status (gpointer *stream, const gchar *status_mess, const gchar *status_type, UserpasswdWindow *window);
void cb_new_log (gpointer *stream, const gchar *log, const gchar *sender, UserpasswdWindow *window);
void cb_draw_check_passwd (gpointer *stream, UserpasswdWindow *window);
void cb_draw_new_passwd (gpointer *stream, UserpasswdWindow *window);

G_END_DECLS

#endif
