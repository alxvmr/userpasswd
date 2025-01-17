#include "userpasswd-window.h"
#include "userpasswd-app.h"

struct _UserpasswdApp {
    AdwApplication parent_instance;

    gchar *current_password;
    gchar *new_password;
    gchar *retype_new_password;
    GSubprocess *subprocess;
    GInputStream *instream;
    GOutputStream *outstream;
};

enum {
    REQ_NEW_PASSWORD,
    LAST_SIGNAL
};

static guint userpasswd_app_signals[LAST_SIGNAL];

G_DEFINE_TYPE (UserpasswdApp, userpasswd_app, ADW_TYPE_APPLICATION);

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

    adw_show_about_dialog (GTK_WIDGET (window),
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
userpasswd_app_quit_action (GSimpleAction *action,
                            GVariant      *parametr,
                            gpointer       userdata)
{
    UserpasswdApp *self = userdata;

    g_assert (USERPASSWD_IS_APP (self));

    g_application_quit (G_APPLICATION (self));
}

static void
userpasswd_app_activate (GApplication *app) {
    GtkWindow *window;

    g_assert (USERPASSWD_IS_APP (app));

    window = gtk_application_get_active_window (GTK_APPLICATION (app));

    if (window == NULL) {
        window = g_object_new (USERPASSWD_TYPE_WINDOW,
                               "application", app,
                               NULL);
    }

    gtk_window_present (GTK_WINDOW (window));
}

static void
userpasswd_app_class_init (UserpasswdAppClass *class)
{
    GApplicationClass *app_class = G_APPLICATION_CLASS (class);

    userpasswd_app_signals[REQ_NEW_PASSWORD] = g_signal_new (
        "request-new-password",
        G_TYPE_FROM_CLASS (class),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0,
        NULL,
        NULL,
        g_cclosure_marshal_VOID__INT,
        G_TYPE_NONE,
        1,
        G_TYPE_INT
    );

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
    {"about", userpasswd_app_about_action}
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
                                           (const char *[]) { "<primary>q", NULL });
}

int
main (int     argc,
      char  **argv)
{
    UserpasswdApp *app = userpasswd_app_new ("org.example.userpasswd", G_APPLICATION_DEFAULT_FLAGS);
    int status = 0;
    
    status = g_application_run (G_APPLICATION (app), argc, argv);

    g_object_unref (app);
    return status;
}