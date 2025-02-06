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
userpasswd_window_show_status (UserpasswdWindow *self,
                               const gchar      *status_mess,
                               const gchar      *status_type)
{
    gtk_label_set_text (GTK_LABEL (self->status), status_mess);
    gtk_widget_set_css_classes (GTK_WIDGET (self->status), (const gchar *[]) {status_type, NULL});
    gtk_widget_set_visible (GTK_WIDGET (self->status), TRUE);
}

gchar*
create_log (const gchar *log,
            const gchar *sender)
{
    GDateTime *now = g_date_time_new_now_local ();
    gchar *timestamp = g_date_time_format (now, "%d/%m/%y %H:%M:%S");

    gchar *format_sender = g_strconcat ("[", sender, "]:", NULL);
    gchar *res = g_strdup_printf ("%-19s%-14s %s\n",
                                  timestamp,
                                  format_sender,
                                  log);
    if (res[strlen(res)-2] != '\n'){
        res = g_strconcat (res, "\n", NULL);
    }

    g_date_time_unref (now);
    g_free (timestamp);
    g_free (format_sender);

    return res;
}

static void
userpasswd_window_add_info (UserpasswdWindow *self,
                            const gchar      *info_mess,
                            const gchar      *sender)
{
    const gchar* info_text = gtk_label_get_text (GTK_LABEL (self->info));
    gtk_label_set_text (GTK_LABEL (self->info),
                        g_strconcat (info_text,
                                     create_log (info_mess, sender),
                                     NULL)
                        );
}

void
cb_check_password_button (GtkWidget *button,
                          gpointer   user_data)
{
    UserpasswdWindow *self = USERPASSWD_WINDOW (user_data);

    if (gtk_widget_get_visible (GTK_WIDGET (self->status))) {
        gtk_widget_set_visible (GTK_WIDGET (self->status), FALSE);
    }

    const gchar *current_password = gtk_editable_get_text (GTK_EDITABLE (self->current_password_row));
    g_signal_emit (self, userpasswd_window_signals[CHECK_PWD], 0, current_password);
}

void
cb_change_password_button (GtkWidget *button,
                           gpointer   user_data)
{
    UserpasswdWindow *self = USERPASSWD_WINDOW (user_data);

    const gchar *new_password = gtk_editable_get_text (GTK_EDITABLE (self->new_password_row));
    const gchar *repeat_new_password = gtk_editable_get_text (GTK_EDITABLE (self->repeat_new_password_row));

    if (g_strcmp0 (new_password, repeat_new_password) == 0) {
        g_signal_emit (self, userpasswd_window_signals[CHANGE_PWD], 0, new_password);
    }
    else {
        userpasswd_window_show_status (self, "Passwords don't match", "error");
    }
}

void
create_change_password_elems (UserpasswdWindow *window)
{
    window->new_password_row = ADW_PASSWORD_ENTRY_ROW (adw_password_entry_row_new ());
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (window->new_password_row ), "New password");

    window->repeat_new_password_row = ADW_PASSWORD_ENTRY_ROW (adw_password_entry_row_new ());
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (window->repeat_new_password_row ), "Repeat new password");

    window->button = gtk_button_new_with_label ("Change password");
    g_signal_connect (G_OBJECT (window->button), "clicked", G_CALLBACK (cb_change_password_button), window);
    
    gtk_list_box_append (GTK_LIST_BOX (window->container_password), GTK_WIDGET (window->new_password_row));
    gtk_list_box_append (GTK_LIST_BOX (window->container_password), GTK_WIDGET (window->repeat_new_password_row));
    gtk_list_box_append (GTK_LIST_BOX (window->container_password), GTK_WIDGET (window->button));
}

void
cb_draw_new_passwd (gpointer *stream,
                    UserpasswdWindow *window)
{
    gtk_list_box_remove_all (GTK_LIST_BOX (window->container_password));
    create_change_password_elems (window);
}

void
cb_new_status (gpointer         *stream,
               const gchar      *status_mess,
               const gchar      *status_type,
               UserpasswdWindow *window)
{
    userpasswd_window_show_status (window, status_mess, status_type);
}

void
cb_new_log (gpointer         *stream,
            const gchar      *log,
            const gchar      *sender,
            UserpasswdWindow *window)
{
    userpasswd_window_add_info (window, log, sender);
}

void
cb_draw_check_passwd (gpointer         *stream,
                      UserpasswdWindow *window)
{
    gtk_list_box_remove_all (GTK_LIST_BOX (window->container_password));

    window->current_password_row = ADW_PASSWORD_ENTRY_ROW (adw_password_entry_row_new ());
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (window->current_password_row), "Current password");
    window->button = gtk_button_new_with_label ("Check password");
    g_signal_connect (G_OBJECT (window->button), "clicked", G_CALLBACK (cb_check_password_button), window);

    gtk_list_box_append (GTK_LIST_BOX (window->container_password), GTK_WIDGET (window->current_password_row));
    gtk_list_box_append (GTK_LIST_BOX (window->container_password), window->button);

    gtk_widget_grab_focus (GTK_WIDGET (window->current_password_row));
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

    userpasswd_window_signals[CHANGE_PWD] = g_signal_new (
        "change-password",
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
    gtk_widget_set_margin_start (self->container, 15);
    gtk_widget_set_margin_end (self->container, 15);

    self->container_password = gtk_list_box_new ();

    PangoAttrList *attr_list = pango_attr_list_new();
    PangoAttribute *attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(attr_list, attr);

    self->status = gtk_label_new ("Status mess");
    gtk_label_set_attributes (GTK_LABEL (self->status), attr_list);
    gtk_widget_set_margin_top (GTK_WIDGET (self->status), 15);
    gtk_widget_add_css_class (GTK_WIDGET (self->status), "error");
    gtk_widget_set_visible (GTK_WIDGET (self->status), FALSE);

    self->info = gtk_label_new ("");
    gtk_label_set_selectable (GTK_LABEL (self->info), TRUE);
    gtk_label_set_wrap (GTK_LABEL (self->info), TRUE);
    gtk_label_set_xalign (GTK_LABEL (self->info), 0);
    gtk_label_set_yalign (GTK_LABEL (self->info), 0);

    self->expander_status = gtk_expander_new ("Info");
    gtk_widget_set_vexpand (self->expander_status, TRUE);

    GtkWidget *scrolled_window = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled_window), GTK_WIDGET (self->info));
    gtk_widget_set_vexpand (scrolled_window, TRUE);

    gtk_expander_set_child (GTK_EXPANDER (self->expander_status), scrolled_window);
    gtk_widget_set_vexpand (self->expander_status, TRUE);

    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->toolbar));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->container_password));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->status));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->expander_status));
    adw_application_window_set_content (ADW_APPLICATION_WINDOW (self), self->container);

    gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self), TRUE);
}