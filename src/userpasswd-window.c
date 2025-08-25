#include "userpasswd-window.h"
#include "userpasswd-stream.h"
#include <passwdqc.h>

const gchar *PASSWDQC_CONFIG_FILE = "/etc/passwdqc.conf";

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
    #define DATA_PATH "/usr/share/userpasswd/data/"
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
clear_status (UserpasswdWindow *window)
{
    if (gtk_widget_get_visible (GTK_WIDGET (window->status_mess)))
        gtk_widget_set_visible (GTK_WIDGET (window->status_mess), FALSE);
    if (gtk_widget_get_visible (GTK_WIDGET (window->substatus_mess)))
        gtk_widget_set_visible (GTK_WIDGET (window->substatus_mess), FALSE);
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

    clear_status (self);

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

    clear_status (self);

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
update_password_strength (GtkWidget  *password_row,
                          GParamSpec *pspec,
                          gpointer    user_data)
{
    UserpasswdWindow *window = USERPASSWD_WINDOW (user_data);

    const gchar *password;
    passwdqc_params_t params;
    gchar *parse_reason = NULL;
    const gchar *check_reason;
    gchar *capitalized_reason = NULL;
    const gchar *config = PASSWDQC_CONFIG_FILE;

    passwdqc_params_reset(&params);

    if (*config && passwdqc_params_load(&params, &parse_reason, config)) {
        g_printerr ("Cannot check password quality: %s\n", (parse_reason ? parse_reason : "Out of memory"));
		goto out;
	}

    clear_status (window);

    password = gtk_editable_get_text (GTK_EDITABLE (password_row));
    check_reason = passwdqc_check(&params.qc, password, NULL, NULL);

    if (check_reason) {
        gtk_widget_add_css_class (GTK_WIDGET (password_row), "error");

        capitalized_reason = g_strdup(check_reason);
        /* Make a message with a capital letter */
        if (capitalized_reason && capitalized_reason[0] != '\0') {
            gunichar first_char = g_utf8_get_char(check_reason);
            gunichar upper_char = g_unichar_toupper(first_char);

            const gchar *rest = g_utf8_next_char(check_reason);

            gchar upper_utf8[6] = {0};
            gint bytes_written = g_unichar_to_utf8(upper_char, upper_utf8);
            upper_utf8[bytes_written] = '\0';

            capitalized_reason = g_strconcat(upper_utf8, rest, NULL);
        }

        gtk_level_bar_set_value (GTK_LEVEL_BAR (window->strength_indicator), 0.2);
        gtk_label_set_label (GTK_LABEL (window->strength_indicator_label), capitalized_reason);
        gtk_widget_set_sensitive (window->button, FALSE);

        g_free (capitalized_reason);
    } else {
        gtk_widget_remove_css_class (GTK_WIDGET (password_row), "error");

        gtk_level_bar_set_value (GTK_LEVEL_BAR (window->strength_indicator), 1.0);
        gtk_label_set_label (GTK_LABEL (window->strength_indicator_label), _("Great password!"));
        gtk_widget_set_sensitive (window->button, TRUE);
    }

out:
    passwdqc_params_free(&params);
    if (parse_reason) {
        g_free (parse_reason);
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

    /* Setting up a password complexity indicator */
    if (g_file_test (PASSWDQC_CONFIG_FILE, G_FILE_TEST_EXISTS))
    {
        gtk_widget_set_sensitive (window->button, FALSE);
        gtk_widget_set_visible (window->strength_indicator, TRUE);
        gtk_widget_set_visible (window->strength_indicator_label, TRUE);
        g_signal_connect (G_OBJECT (window->new_password_row), "notify::text", G_CALLBACK (update_password_strength), window);
    } else {
        // TODO: add warning
    }

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
userpasswd_window_class_init (UserpasswdWindowClass *class)
{
#ifdef USE_ADWAITA
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               "/org/altlinux/userpasswd/userpasswd-gnome-window.ui");
#else
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               "/org/altlinux/userpasswd/userpasswd-gtk-window.ui");
#endif

    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdWindow, status_mess);
    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdWindow, substatus_mess);
    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdWindow, info);
    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdWindow, spinner);
    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdWindow, menu_button);
    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdWindow, container_data_input);
    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdWindow, container_password);
    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdWindow, strength_indicator);
    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdWindow, strength_indicator_label);

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
    gtk_widget_init_template (GTK_WIDGET (self));

#ifndef USE_ADWAITA
    GtkCssProvider *css_provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (css_provider, "/org/altlinux/userpasswd/style.css");
    gtk_style_context_add_provider_for_display (gdk_display_get_default(),
                                                GTK_STYLE_PROVIDER (css_provider),
                                                GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref (css_provider);
#endif

    gtk_window_set_default_size (GTK_WINDOW (self), 650, 400);

    self->menu = g_menu_new ();
    gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (self->menu_button), G_MENU_MODEL (self->menu));

    g_menu_append (self->menu, _("About"), "app.about");
    g_menu_append (self->menu, _("Quit"), "app.quit");

    gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self), TRUE);
}
