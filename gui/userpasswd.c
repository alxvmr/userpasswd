#include <adwaita.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "manipulcation_pwd.h"

static void
show_about (GSimpleAction *action,
            GVariant      *parameter,
            gpointer       userdata)
{
    const char *developers[] = {
        "Maria Alexeeva",
        NULL
    };

    adw_show_about_dialog (GTK_WIDGET (gtk_application_get_active_window (userdata)),
                           "application_name", "userpasswd",
                           "application-icon", "consolehelper-keyring",
                           "version", "0.0.1",
                           "copyright", "© 2025 BaseALT",
                           "issue-url", "https://github.com/alxvmr/userpasswd/issues",
                           "license-type", GTK_LICENSE_GPL_3_0,
                           "developers", developers,
                           NULL);
}

static void
cb_change_password_button (GtkWidget *widget,
                           GtkWidget *lbox)
{
    /*
    TODO: после смены пароля заблокировать поля ввода
    */

    GtkListBoxRow *row_pwd1 = gtk_list_box_get_row_at_index (GTK_LIST_BOX (lbox), 0);
    GtkListBoxRow *row_pwd2 = gtk_list_box_get_row_at_index (GTK_LIST_BOX (lbox), 1);
    GtkListBoxRow *row_btn = gtk_list_box_get_row_at_index (GTK_LIST_BOX (lbox), 2);

    gtk_widget_set_sensitive(GTK_WIDGET(row_pwd1), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(row_pwd2), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(row_btn), FALSE);

    GtkWidget *vbox = gtk_widget_get_parent (gtk_widget_get_parent (lbox));

    AdwClamp *clamp_status = ADW_CLAMP (adw_clamp_new ());
    gtk_widget_set_margin_top (GTK_WIDGET (clamp_status), 15);

    GtkWidget *status = adw_banner_new ("Password change error");
    adw_banner_set_revealed (ADW_BANNER (status), TRUE);
    
    adw_clamp_set_child (ADW_CLAMP (clamp_status), GTK_WIDGET (status));

    AdwClamp *clamp_info = ADW_CLAMP (adw_clamp_new ());
    gtk_widget_set_margin_top (GTK_WIDGET (clamp_info), 0);
    
    GtkWidget *bottom = adw_expander_row_new ();
    adw_expander_row_set_subtitle (ADW_EXPANDER_ROW (bottom), "Info");
    GtkWidget *info = gtk_label_new ("Information about some error ...");
    adw_expander_row_add_row (ADW_EXPANDER_ROW (bottom), info);

    adw_clamp_set_child (ADW_CLAMP (clamp_info), GTK_WIDGET (bottom));

    gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (clamp_status));
    gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (clamp_info));
}

static void
cb_check_password_button (GtkWidget *widget,
                          GtkWidget *lbox)
{
    GtkListBoxRow *row_input = gtk_list_box_get_row_at_index (GTK_LIST_BOX (lbox), 0);
    const gchar *current_password = NULL;
    current_password = gtk_editable_get_text (GTK_EDITABLE (row_input));
    g_print ("Current password = %s\n", current_password);
    create_pipe (current_password, "123", "123");

    // тут логика отправки текущего пароля в passwd
    // если успешно, то запрашиваем новый пароль

    // все ли ок с памятью?
    gtk_list_box_remove (GTK_LIST_BOX (lbox), GTK_WIDGET (row_input));

    GtkListBoxRow *row_button = gtk_list_box_get_row_at_index (GTK_LIST_BOX (lbox), 0);
    gtk_list_box_remove (GTK_LIST_BOX (lbox), GTK_WIDGET (row_button));

    AdwPasswordEntryRow *new_password_row = ADW_PASSWORD_ENTRY_ROW (adw_password_entry_row_new());
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (new_password_row), "New password");

    AdwPasswordEntryRow *repeat_new_password_row = ADW_PASSWORD_ENTRY_ROW (adw_password_entry_row_new());
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (repeat_new_password_row), "Repeat new password");

    GtkWidget *change_password_button = gtk_button_new_with_label ("Change password");
    g_signal_connect (G_OBJECT (change_password_button), "clicked", G_CALLBACK (cb_change_password_button), lbox);

    gtk_list_box_append (GTK_LIST_BOX (lbox), GTK_WIDGET (new_password_row));
    gtk_list_box_append (GTK_LIST_BOX (lbox), GTK_WIDGET (repeat_new_password_row));
    gtk_list_box_append (GTK_LIST_BOX (lbox), change_password_button);
}

static void
create_menu (GtkApplication *app,
             GtkWidget *header_bar)
{
    GSimpleAction *act_about = g_simple_action_new ("about", NULL);
    g_action_map_add_action (G_ACTION_MAP (app), G_ACTION (act_about));
    g_signal_connect (G_OBJECT (act_about), "activate", G_CALLBACK (show_about), app);

    GtkWidget *menu_button = gtk_menu_button_new ();

    GMenu *menu = g_menu_new ();
    GMenuItem *menu_item_about = g_menu_item_new ("About", "app.about");
    g_menu_append_item (menu, menu_item_about);
    g_object_unref (menu_item_about);

    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (menu_button), G_MENU_MODEL (menu));
    adw_header_bar_pack_start (ADW_HEADER_BAR (header_bar), menu_button);
}

static void
cb_activate (GtkApplication *app,
             gpointer        user_data)
{
    GtkWidget *window = adw_application_window_new (app);
    GtkWidget *toolbar_view = adw_toolbar_view_new ();
    GtkWidget *header_bar = adw_header_bar_new ();

    gtk_window_set_title (GTK_WINDOW (window), "userpasswd");
    gtk_window_set_default_size (GTK_WINDOW (window), 600, 300);

    create_menu (app, header_bar);
    adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (toolbar_view), header_bar);

    GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_bottom (vbox, 15);

    AdwClamp *clamp = ADW_CLAMP (adw_clamp_new ());
    gtk_widget_set_margin_top (GTK_WIDGET (clamp), 15);

    GtkWidget *lbox = gtk_list_box_new ();

    AdwPasswordEntryRow *current_password_row = ADW_PASSWORD_ENTRY_ROW (adw_password_entry_row_new());
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (current_password_row), "Current password");

    GtkWidget *check_password_button = gtk_button_new_with_label ("Check password");
    g_signal_connect (G_OBJECT (check_password_button), "clicked", G_CALLBACK (cb_check_password_button), lbox);

    gtk_list_box_append (GTK_LIST_BOX (lbox), GTK_WIDGET (current_password_row));
    gtk_list_box_append (GTK_LIST_BOX (lbox), check_password_button);

    adw_clamp_set_child (clamp, GTK_WIDGET (lbox));
    gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (toolbar_view));
    gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (clamp));
    adw_application_window_set_content (ADW_APPLICATION_WINDOW (window), GTK_WIDGET (vbox));

    gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (window), TRUE);
    gtk_window_present (GTK_WINDOW (window));
}

int
main (int     argc,
      char  **argv)
{
    AdwApplication *app = adw_application_new ("org.example.userpasswd", G_APPLICATION_DEFAULT_FLAGS);
    int status = 0;

    g_signal_connect (app, "activate", G_CALLBACK (cb_activate), NULL);
    
    status = g_application_run (G_APPLICATION (app), argc, argv);

    g_object_unref (app);
    return status;
}