#include "userpasswd-window.h"
#include "userpasswd-stream.h"

enum {
    CHECK_PWD,
    CHANGE_PWD,
    LAST_SIGNAL
};

static guint userpasswd_window_signals[LAST_SIGNAL];

#ifdef USE_ADWAITA
    G_DEFINE_FINAL_TYPE (UserpasswdWindow, userpasswd_window, ADW_TYPE_APPLICATION_WINDOW)
#else
    G_DEFINE_FINAL_TYPE (UserpasswdWindow, userpasswd_window, GTK_TYPE_APPLICATION_WINDOW)
#endif

static void
start_spinner (UserpasswdWindow *self)
{
    if (self->spinner) {
        gtk_widget_set_visible (self->spinner, TRUE);
        gtk_spinner_start (GTK_SPINNER (self->spinner));
    }
}

static void
stop_spinner (UserpasswdWindow *self)
{
    if (self->spinner) {
        gtk_widget_set_visible (self->spinner, FALSE);
        gtk_spinner_stop (GTK_SPINNER (self->spinner));
    }
}

static void
userpasswd_window_show_status (UserpasswdWindow *self,
                               const gchar      *status_mess,
                               const gchar      *substatus_mess,
                               const gchar      *status_type)
{
    g_debug ("Start status display");
    stop_spinner (self);

    gtk_label_set_text (GTK_LABEL (self->status_mess), NULL);
    gtk_label_set_text (GTK_LABEL (self->substatus_mess), NULL);

    gtk_label_set_text (GTK_LABEL (self->status_mess), status_mess);
    gtk_widget_set_css_classes (GTK_WIDGET (self->status_mess), (const gchar *[]) {status_type, NULL});

    if (substatus_mess) {
        gtk_label_set_markup (GTK_LABEL (self->substatus_mess), substatus_mess);
        gtk_widget_set_visible (self->substatus_mess, TRUE);
    }

    gtk_widget_set_visible (GTK_WIDGET (self->status_mess), TRUE);
}

gchar*
create_log (const gchar *log,
            const gchar *sender)
{
    g_debug ("Start creating a formatted log");
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
    g_debug ("Formatted log returned and inserting into label");
}

void
cb_check_password_button (GtkWidget *button,
                          gpointer   user_data)
{
    g_debug ("Start callback on pressing the check password button");
    UserpasswdWindow *self = USERPASSWD_WINDOW (user_data);
    start_spinner (self);

    if (gtk_widget_get_visible (GTK_WIDGET (self->status_mess)))
        gtk_widget_set_visible (GTK_WIDGET (self->status_mess), FALSE);
    if (gtk_widget_get_visible (GTK_WIDGET (self->substatus_mess)))
        gtk_widget_set_visible (GTK_WIDGET (self->substatus_mess), FALSE);

    const gchar *current_password = gtk_editable_get_text (GTK_EDITABLE (self->current_password_row));
    g_signal_emit (self, userpasswd_window_signals[CHECK_PWD], 0, current_password);
}

void
clear_container_input_data (UserpasswdWindow *window)
{
    gtk_list_box_remove_all (GTK_LIST_BOX (window->container_password));
    if (window->button)
        gtk_box_remove (GTK_BOX (window->container_data_input), GTK_WIDGET (window->button));
}

void
cb_change_password_button (GtkWidget *button,
                           gpointer   user_data)
{
    g_debug ("Start callback on pressing the change password button");
    UserpasswdWindow *self = USERPASSWD_WINDOW (user_data);
    start_spinner (self);

    if (gtk_widget_get_visible (GTK_WIDGET (self->status_mess)))
        gtk_widget_set_visible (GTK_WIDGET (self->status_mess), FALSE);
    if (gtk_widget_get_visible (GTK_WIDGET (self->substatus_mess)))
        gtk_widget_set_visible (GTK_WIDGET (self->substatus_mess), FALSE);

    const gchar *new_password = gtk_editable_get_text (GTK_EDITABLE (self->new_password_row));
    const gchar *repeat_new_password = gtk_editable_get_text (GTK_EDITABLE (self->repeat_new_password_row));

    if (g_strcmp0 (new_password, repeat_new_password) == 0) {
        g_signal_emit (self, userpasswd_window_signals[CHANGE_PWD], 0, new_password);
    }
    else {
        userpasswd_window_show_status (self, _("Passwords don't match"), NULL, "error");
    }
}

void
create_change_password_elems (UserpasswdWindow *window)
{
#ifdef USE_ADWAITA
    window->new_password_row = adw_password_entry_row_new ();
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (window->new_password_row ), _("New password"));
#else
    window->new_password_row = gtk_password_entry_new ();
    g_object_set (window->new_password_row, "placeholder-text", _("New password"), NULL);
    gtk_password_entry_set_show_peek_icon (GTK_PASSWORD_ENTRY (window->new_password_row), TRUE);
#endif

    /* Take the focus off the suffix */
    GtkWidget *child = gtk_widget_get_first_child (window->new_password_row);
    GtkWidget *g_child = gtk_widget_get_last_child (child);
    gtk_widget_set_can_focus (g_child, FALSE);

#ifdef USE_ADWAITA
    window->repeat_new_password_row = adw_password_entry_row_new ();
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (window->repeat_new_password_row ), _("Repeat new password"));
#else
    window->repeat_new_password_row = gtk_password_entry_new ();
    g_object_set (window->repeat_new_password_row, "placeholder-text", _("Repeat new password"), NULL);
    gtk_password_entry_set_show_peek_icon (GTK_PASSWORD_ENTRY (window->repeat_new_password_row), TRUE);
#endif

    /* Take the focus off the suffix */
    child = gtk_widget_get_first_child (window->repeat_new_password_row);
    g_child = gtk_widget_get_last_child (child);
    gtk_widget_set_can_focus (g_child, FALSE);

    window->button = gtk_button_new_with_label (_("Change password"));
    g_signal_connect (G_OBJECT (window->button), "clicked", G_CALLBACK (cb_change_password_button), window);
    
    gtk_list_box_append (GTK_LIST_BOX (window->container_password), window->new_password_row);
    gtk_list_box_append (GTK_LIST_BOX (window->container_password), window->repeat_new_password_row);
#ifndef USE_ADWAITA
    gtk_widget_set_focusable (GTK_WIDGET (
                                gtk_list_box_get_row_at_index (GTK_LIST_BOX (window->container_password), 0)),
                              FALSE);
    gtk_widget_set_focusable (GTK_WIDGET (
                                gtk_list_box_get_row_at_index (GTK_LIST_BOX (window->container_password), 1)),
                              FALSE);
#endif
    gtk_box_append (GTK_BOX (window->container_data_input), GTK_WIDGET (window->button));

    gtk_widget_grab_focus (window->new_password_row);
}

void
cb_draw_new_passwd (gpointer *stream,
                    UserpasswdWindow *window)
{
    g_debug ("Start callback to draw items for requesting a new password");
    stop_spinner (window);
    clear_container_input_data (window);
    create_change_password_elems (window);
}

void
cb_new_status (gpointer         *stream,
               const gchar      *status_mess,
               const gchar      *status_type,
               UserpasswdWindow *window)
{
    g_debug ("Start callback to process a new status");
    
    gtk_label_set_text (GTK_LABEL (window->status_mess), status_mess);

    if (!g_strcmp0 (status_type, "success")) {
        gtk_widget_set_sensitive (window->container_password, FALSE);
        gtk_widget_set_sensitive (window->button, FALSE);

        g_signal_handlers_disconnect_by_data (window->button, window);

        userpasswd_window_show_status (window,
                                       status_mess,
                                       _("Press the Escape button to exit"),
                                       status_type);
        return;
    }

    userpasswd_window_show_status (window, status_mess, NULL, status_type);
}

void
cb_new_log (gpointer         *stream,
            const gchar      *log,
            const gchar      *sender,
            UserpasswdWindow *window)
{
    g_debug ("Start callback to process a new log");
    userpasswd_window_add_info (window, log, sender);
}

void
cb_draw_check_passwd (gpointer         *stream,
                      UserpasswdWindow *window)
{
    g_debug ("Start callback to render items for old password request");
    clear_container_input_data (window);

#ifdef USE_ADWAITA
    window->current_password_row = adw_password_entry_row_new ();
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (window->current_password_row), _("Current password"));
#else
    window->current_password_row = gtk_password_entry_new ();
    g_object_set (window->current_password_row, "placeholder-text", _("Current password"), NULL);
    gtk_password_entry_set_show_peek_icon (GTK_PASSWORD_ENTRY (window->current_password_row), TRUE);
#endif

    /* Take the focus off the suffix */
    GtkWidget *child = gtk_widget_get_first_child (window->current_password_row);
    GtkWidget *g_child = gtk_widget_get_last_child (child);
    gtk_widget_set_can_focus (g_child, FALSE);

    window->button = gtk_button_new_with_label (_("Check password"));
    g_signal_connect (G_OBJECT (window->button), "clicked", G_CALLBACK (cb_check_password_button), window);

    gtk_list_box_append (GTK_LIST_BOX (window->container_password), window->current_password_row);
#ifndef USE_ADWAITA
    gtk_widget_set_focusable (GTK_WIDGET (
                                gtk_list_box_get_row_at_index (GTK_LIST_BOX (window->container_password), 0) ),
                              FALSE);
#endif
    gtk_box_append (GTK_BOX (window->container_data_input), GTK_WIDGET (window->button));

    gtk_widget_grab_focus (window->current_password_row);
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
#ifdef USE_ADWAITA
    self->toolbar = adw_toolbar_view_new ();
    self->header_bar = adw_header_bar_new ();
    gtk_widget_set_margin_bottom (self->toolbar, 10);
#else
    self->toolbar = NULL;
    self->header_bar = gtk_header_bar_new ();
    gtk_window_set_titlebar (GTK_WINDOW (self), self->header_bar);
#endif

    gtk_widget_set_can_focus (self->header_bar, FALSE);
    gtk_window_set_default_size (GTK_WINDOW (self), 650, 400);

    /* create title and subtitle*/
#ifdef USE_ADWAITA
    GtkWidget *title = adw_window_title_new ("userpasswd",  _("Change password"));
    adw_header_bar_set_title_widget (ADW_HEADER_BAR (self->header_bar), title);
#else
    GtkWidget *title_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_valign (title_box, GTK_ALIGN_CENTER);
    GtkWidget *title = gtk_label_new ("userpasswd");
    GtkWidget *subtitle = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (subtitle), _("<span font='8'>Change password</span>"));
    gtk_box_append (GTK_BOX(title_box), title);
    gtk_box_append (GTK_BOX(title_box), subtitle);
    gtk_header_bar_set_title_widget (GTK_HEADER_BAR (self->header_bar), title_box);
#endif

    /* create menu */
    self->menu = g_menu_new ();
    self->menu_button = gtk_menu_button_new ();
    gtk_menu_button_set_icon_name (GTK_MENU_BUTTON (self->menu_button), "open-menu-symbolic");
    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (self->menu_button), G_MENU_MODEL (self->menu));

#ifdef USE_ADWAITA
    adw_header_bar_pack_start (ADW_HEADER_BAR (self->header_bar), self->menu_button);
    adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (self->toolbar), self->header_bar);
#else
    gtk_header_bar_pack_start (GTK_HEADER_BAR (self->header_bar), self->menu_button);
#endif

    g_menu_append (self->menu, _("About"), "app.about");
    g_menu_append (self->menu, _("Quit"), "app.quit");

    self->container = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_bottom (self->container, 15);
    gtk_widget_set_margin_start (self->container, 15);
    gtk_widget_set_margin_end (self->container, 15);

    self->container_data_input = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    self->container_password = gtk_list_box_new ();
#ifdef USE_ADWAITA
    gtk_widget_set_css_classes (self->container_password, (const gchar *[]) {"boxed-list", NULL});
#endif
    gtk_widget_set_margin_bottom (self->container_password, 10);

    PangoAttrList *attr_list = pango_attr_list_new();
    PangoAttribute *attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(attr_list, attr);

    self->status_container = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top (GTK_WIDGET (self->status_container), 15);
    // gtk_widget_set_visible (GTK_WIDGET (self->status_container), FALSE);
    self->status_mess = gtk_label_new (NULL);
    self->substatus_mess = gtk_label_new (NULL);
    gtk_widget_set_visible (GTK_WIDGET (self->status_mess), FALSE);
    gtk_widget_set_visible (GTK_WIDGET (self->substatus_mess), FALSE);

    gtk_label_set_attributes (GTK_LABEL (self->status_mess), attr_list);
    pango_attr_list_unref (attr_list);

    self->spinner = gtk_spinner_new ();
    gtk_widget_set_visible (GTK_WIDGET (self->spinner), FALSE);

    gtk_box_append (GTK_BOX (self->status_container), self->spinner);
    gtk_box_append (GTK_BOX (self->status_container), self->status_mess);
    gtk_box_append (GTK_BOX (self->status_container), self->substatus_mess);

    self->info = gtk_label_new ("");
    gtk_label_set_selectable (GTK_LABEL (self->info), TRUE);
    gtk_label_set_wrap (GTK_LABEL (self->info), TRUE);
    gtk_label_set_xalign (GTK_LABEL (self->info), 0);
    gtk_label_set_yalign (GTK_LABEL (self->info), 0);

    self->expander_status = gtk_expander_new (_("Info"));
    gtk_widget_set_can_focus (self->expander_status, FALSE);
    gtk_widget_set_vexpand (self->expander_status, TRUE);
    gtk_widget_set_margin_top (self->expander_status, 10);

    GtkWidget *scrolled_window = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled_window), GTK_WIDGET (self->info));
    gtk_widget_set_vexpand (scrolled_window, TRUE);

    gtk_expander_set_child (GTK_EXPANDER (self->expander_status), scrolled_window);
    gtk_widget_set_vexpand (self->expander_status, TRUE);

    gtk_box_append (GTK_BOX (self->container_data_input), GTK_WIDGET (self->container_password));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->container_data_input));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->status_container));
    gtk_box_append (GTK_BOX (self->container), GTK_WIDGET (self->expander_status));

#ifdef USE_ADWAITA
    GtkWidget *clamp = adw_clamp_new ();
    adw_clamp_set_child (ADW_CLAMP(clamp), self->container);
    adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (self->toolbar), clamp);
    adw_application_window_set_content (ADW_APPLICATION_WINDOW (self), self->toolbar);
#else
    gtk_window_set_child (GTK_WINDOW (self), self->container);
#endif

    gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self), TRUE);
}