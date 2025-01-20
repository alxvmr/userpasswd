#include "userpasswd-window.h"
#include "userpasswd-stream.h"

enum {
    CHECK_PWD,
    CHANGE_PWD,
    LAST_SIGNAL
};

static guint userpasswd_window_signals[LAST_SIGNAL];

G_DEFINE_FINAL_TYPE (UserpasswdWindow, userpasswd_window, ADW_TYPE_APPLICATION_WINDOW)

static void
userpasswd_window_show_info_status (UserpasswdWindow *self,
                                    gchar            *status_mess,
                                    gchar            *info_mess)
{
    adw_banner_set_title (ADW_BANNER (self->status), status_mess);
    gtk_label_set_text (GTK_LABEL (self->info), info_mess);
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->clamp_status));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->clamp_info));
}

void
destroy_check_password_elems (UserpasswdWindow *window)
{
    return;
}

void
create_change_password_elems (UserpasswdWindow *window)
{
    return;
}

void
cb_check_password_button (GtkWidget *button,
                          gpointer   user_data)
{
    UserpasswdWindow *self = USERPASSWD_WINDOW (user_data);

    const gchar *current_password = gtk_editable_get_text (GTK_EDITABLE (self->current_password_row));
    g_signal_emit (self, userpasswd_window_signals[CHECK_PWD], 0, current_password);

    // userpasswd_window_show_info_status (self, "Error", "Something wrong...");
    // g_print ("Нажали на кнопку\n");
}

void
cb_check_password_success (gpointer *stream,
                           UserpasswdWindow *window)
{
    destroy_check_password_elems (window);
    create_change_password_elems (window);

    g_print ("Пароль прошел проверку\n");
}

static void
userpasswd_window_class_init (UserpasswdWindowClass *class) {

    userpasswd_window_signals[CHECK_PWD] = g_signal_new (
        "check-password",
        G_TYPE_FROM_CLASS (class),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0,
        NULL,
        NULL,
        g_cclosure_marshal_VOID__STRING,
        G_TYPE_NONE,
        1,
        G_TYPE_STRING
    );

    /*
    TODO: - add dispose
          - add finalize
    */
}

static void
userpasswd_window_init (UserpasswdWindow *self)
{
    self->toolbar = adw_toolbar_view_new ();
    self->header_bar = adw_header_bar_new ();

    gtk_window_set_title (GTK_WINDOW (self), "userpasswd");
    gtk_window_set_default_size (GTK_WINDOW (self), 600, 300);

    /* create menu */
    self->menu = g_menu_new ();
    self->menu_button = gtk_menu_button_new ();
    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (self->menu_button), G_MENU_MODEL (self->menu));
    adw_header_bar_pack_start (ADW_HEADER_BAR (self->header_bar), self->menu_button);
    adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (self->toolbar), self->header_bar);

    self->container = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_bottom (self->container, 15);

    AdwClamp *clamp = ADW_CLAMP (adw_clamp_new());
    gtk_widget_set_margin_top (GTK_WIDGET (clamp), 15);

    GtkWidget *lbox = gtk_list_box_new ();

    self->current_password_row = ADW_PASSWORD_ENTRY_ROW (adw_password_entry_row_new ());
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (self->current_password_row), "Current password");

    self->check_password_button = gtk_button_new_with_label ("Check password");
    g_signal_connect (G_OBJECT (self->check_password_button), "clicked", G_CALLBACK (cb_check_password_button), self);

    self->clamp_status = ADW_CLAMP (adw_clamp_new ());
    gtk_widget_set_margin_top (GTK_WIDGET (self->clamp_status), 15);
    self->status = adw_banner_new ("Status mess");
    adw_banner_set_revealed (ADW_BANNER (self->status), TRUE);
    adw_clamp_set_child (ADW_CLAMP (self->clamp_status), GTK_WIDGET (self->status));
    self->clamp_info = ADW_CLAMP (adw_clamp_new ());
    gtk_widget_set_margin_top (GTK_WIDGET (self->clamp_info), 0);
    GtkWidget *bottom = adw_expander_row_new ();
    adw_expander_row_set_subtitle (ADW_EXPANDER_ROW (bottom), "Info");
    self->info = gtk_label_new ("Information about ...");
    adw_expander_row_add_row (ADW_EXPANDER_ROW (bottom), self->info);
    adw_clamp_set_child (ADW_CLAMP (self->clamp_info), GTK_WIDGET (bottom));

    gtk_list_box_append (GTK_LIST_BOX (lbox), GTK_WIDGET (self->current_password_row));
    gtk_list_box_append (GTK_LIST_BOX (lbox), self->check_password_button);

    adw_clamp_set_child (clamp, GTK_WIDGET (lbox));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->toolbar));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (clamp));
    adw_application_window_set_content (ADW_APPLICATION_WINDOW (self), self->container);

    gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self), TRUE);
}