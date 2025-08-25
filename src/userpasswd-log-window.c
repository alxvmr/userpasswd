#include "userpasswd-log-window.h"

#ifdef USE_ADWAITA
    G_DEFINE_FINAL_TYPE (UserpasswdLogWindow, userpasswd_logwindow, ADW_TYPE_WINDOW)
#else
    G_DEFINE_FINAL_TYPE (UserpasswdLogWindow, userpasswd_logwindow, GTK_TYPE_WINDOW)
#endif

static void
userpasswd_logwindow_dispose (GObject *object)
{
    UserpasswdLogWindow *self = USERPASSWD_LOGWINDOW (object);

    g_debug ("Disposing UserpasswdLogWindow");

    g_clear_pointer (&self->info, gtk_widget_unparent);
    G_OBJECT_CLASS (userpasswd_logwindow_parent_class)->dispose (object);
}

static void
userpasswd_logwindow_finalize (GObject *object)
{
    UserpasswdLogWindow *self = USERPASSWD_LOGWINDOW (object);

    g_debug ("Finalizing UserpasswdLogWindow");

    G_OBJECT_CLASS (userpasswd_logwindow_parent_class)->finalize (object);
}

static void
userpasswd_logwindow_class_init (UserpasswdLogWindowClass *class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (class);

#ifdef USE_ADWAITA
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               "/org/altlinux/userpasswd/userpasswd-gnome-log-window.ui");
#else
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               "/org/altlinux/userpasswd/userpasswd-gtk-log-window.ui");
#endif

    gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (class), UserpasswdLogWindow, info);

    object_class->dispose = userpasswd_logwindow_dispose;
    object_class->finalize = userpasswd_logwindow_finalize;
}

static void
userpasswd_logwindow_init (UserpasswdLogWindow *self)
{
    gtk_widget_init_template (GTK_WIDGET (self));

    gtk_window_set_default_size (GTK_WINDOW (self), 400, 300);
}