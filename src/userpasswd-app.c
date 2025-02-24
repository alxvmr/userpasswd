#include "userpasswd-window.h"
#include "userpasswd-app.h"
#include "userpasswd-stream.h"

struct _UserpasswdApp {
#ifdef USE_ADWAITA
    AdwApplication parent_instance;
#else
    GtkApplication parent_instance;
#endif

    UserpasswdWindow *window;
    UserpasswdStream *stream;
    gchar *current_password;
    gchar *new_password;
    gchar *retype_new_password;
};

#ifdef USE_ADWAITA
G_DEFINE_TYPE (UserpasswdApp, userpasswd_app, ADW_TYPE_APPLICATION);
#else
G_DEFINE_TYPE (UserpasswdApp, userpasswd_app, GTK_TYPE_APPLICATION);
#endif

static void
userpasswd_app_about_action (GSimpleAction *action,
                             GVariant      *parameter,
                             gpointer       userdata)
{
    static const char *developers[] = {
        "Maria Alexeeva",
        NULL
    };

    UserpasswdApp *self = userdata;
    GtkWindow *window = NULL;

    g_assert (USERPASSWD_IS_APP (self));

    window = gtk_application_get_active_window (GTK_APPLICATION (self));

#ifdef USE_ADWAITA
    adw_show_about_dialog (GTK_WIDGET (window),
                           "application_name", "userpasswd",
                           "version", "0.0.1",
                           "copyright", "Copyright (C) 2025 Maria O. Alexeeva\nalxvmr@altlinux.org",
                           "issue-url", "https://github.com/alxvmr/userpasswd-gnome/issues",
                           "license-type", GTK_LICENSE_GPL_3_0,
                           "developers", developers,
                           NULL);
#else
    gtk_show_about_dialog (window,
                           "program-name", "userpasswd",
                           "version", "0.0.1",
                           "logo-icon-name", "userpasswd-keyring",
                           "copyright", "Copyright (C) 2025 Maria O. Alexeeva\nalxvmr@altlinux.org",
                           "website", "https://github.com/alxvmr/userpasswd-gnome",
                           "website-label", "https://github.com/alxvmr/userpasswd-gnome",
                           "license-type", GTK_LICENSE_GPL_3_0,
                           NULL);
#endif
}

static void
userpasswd_app_quit_action (GSimpleAction *action,
                            GVariant      *parametr,
                            gpointer       userdata)
{
    UserpasswdApp *self = userdata;

    g_assert (USERPASSWD_IS_APP (self));

    g_application_quit (G_APPLICATION (self));
}

static void
userpasswd_app_press_enter (GSimpleAction *action,
                            GVariant      *parametr,
                            gpointer       userdata)
{
    UserpasswdApp *self = userdata;

    if (self->window->button != NULL) {
        g_signal_emit_by_name (self->window->button, "clicked", self);
    }
}

static void
userpasswd_app_activate (GApplication *app) {
    g_assert (USERPASSWD_IS_APP (app));

    USERPASSWD_APP (app)->window = USERPASSWD_WINDOW (gtk_application_get_active_window (GTK_APPLICATION (app)));

    if (USERPASSWD_APP (app)->window == NULL) {
        USERPASSWD_APP (app)->window = g_object_new (USERPASSWD_TYPE_WINDOW,
                                                     "application", app,
                                                     NULL);
    }

    USERPASSWD_APP (app)->stream = userpasswd_stream_new ("/usr/lib/userpasswd/helper");
    g_signal_connect (USERPASSWD_APP(app)->window, "check-password", G_CALLBACK (on_password_reciever), USERPASSWD_APP (app)->stream);
    g_signal_connect (USERPASSWD_APP(app)->window, "change-password", G_CALLBACK (on_new_password_reciever), USERPASSWD_APP (app)->stream);
    g_signal_connect (USERPASSWD_APP (app)->stream, "new-status", G_CALLBACK (cb_new_status), USERPASSWD_APP(app)->window);
    g_signal_connect (USERPASSWD_APP (app)->stream, "new-log", G_CALLBACK (cb_new_log), USERPASSWD_APP(app)->window);
    g_signal_connect (USERPASSWD_APP (app)->stream, "draw-check-passwd", G_CALLBACK (cb_draw_check_passwd), USERPASSWD_APP(app)->window);
    g_signal_connect (USERPASSWD_APP (app)->stream, "draw-new-passwd", G_CALLBACK (cb_draw_new_passwd), USERPASSWD_APP(app)->window);

    g_menu_append (USERPASSWD_APP (app)->window->menu, _("About"), "app.about");
    g_menu_append (USERPASSWD_APP (app)->window->menu, _("Quit"), "app.quit");

    gtk_window_present (GTK_WINDOW (USERPASSWD_APP (app)->window));

    userpasswd_stream_communicate (USERPASSWD_APP (app)->window, USERPASSWD_APP (app)->stream);
}

static void
userpasswd_app_class_init (UserpasswdAppClass *class)
{
    GApplicationClass *app_class = G_APPLICATION_CLASS (class);

    app_class->activate = userpasswd_app_activate;
}

UserpasswdApp *
userpasswd_app_new (const char        *application_id, 
                    GApplicationFlags flags)
{
    g_return_val_if_fail (application_id != NULL, NULL);

    return g_object_new (USERPASSWD_TYPE_APP,
                         "application-id", application_id,
                         "flags", flags,
                         NULL);
}

static const GActionEntry app_actions[] = {
    {"quit", userpasswd_app_quit_action},
    {"about", userpasswd_app_about_action},
    {"press_enter", userpasswd_app_press_enter}
};

static void
userpasswd_app_init (UserpasswdApp *self)
{
    g_action_map_add_action_entries (G_ACTION_MAP (self),
                                     app_actions,
                                     G_N_ELEMENTS (app_actions),
                                     self);

    gtk_application_set_accels_for_action (GTK_APPLICATION (self),
                                           "app.quit",
                                           (const char *[]) { "Escape", NULL });
    /*
    TODO: is this how a accelerator should be created for a widget?
    */
    gtk_application_set_accels_for_action (GTK_APPLICATION (self),
                                           "app.press_enter",
                                           (const char *[]) { "Return", NULL});
}

int
main (int     argc,
      char  **argv)
{
    setlocale (LC_ALL, "");
    bindtextdomain ("userpasswd-gnome", "/usr/share/locale/");
    textdomain ("userpasswd-gnome");

    UserpasswdApp *app = userpasswd_app_new ("org.example.userpasswd", G_APPLICATION_DEFAULT_FLAGS);
    
    int status = 0;
    status = g_application_run (G_APPLICATION (app), argc, argv);

    g_object_unref (app);
    return status;
}