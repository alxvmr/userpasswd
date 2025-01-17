#include "userpasswd-window.h"

G_DEFINE_FINAL_TYPE (UserpasswdWindow, userpasswd_window, ADW_TYPE_APPLICATION_WINDOW)

void
cb_check_password_button (GtkWidget *button,
                          gpointer   user_data)
{
    UserpasswdWindow *self = USERPASSWD_WINDOW (user_data);
    g_print ("Нажали на кнопку\n");
}

static void
userpasswd_window_class_init (UserpasswdWindowClass *class) {
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

    gtk_list_box_append (GTK_LIST_BOX (lbox), GTK_WIDGET (self->current_password_row));
    gtk_list_box_append (GTK_LIST_BOX (lbox), self->check_password_button);

    adw_clamp_set_child (clamp, GTK_WIDGET (lbox));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->toolbar));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (clamp));
    adw_application_window_set_content (ADW_APPLICATION_WINDOW (self), self->container);

    gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self), TRUE);
}