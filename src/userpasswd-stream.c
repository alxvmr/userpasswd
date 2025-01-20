#include "userpasswd-stream.h"

G_DEFINE_TYPE (UserpasswdStream, userpasswd_stream, G_TYPE_OBJECT);

static void
userpasswd_stream_create_stream (UserpasswdStream *self)
{
    GError *error = NULL;

    self->subprocess = g_subprocess_new (
        G_SUBPROCESS_FLAGS_STDIN_PIPE | G_SUBPROCESS_FLAGS_STDOUT_PIPE,
        &error,
        self->subprocess_path,
        NULL
    );

    if (error) {
        g_printerr ("Error creating subprocess: %s\n", error->message);
        g_error_free (error);
        return;
    }

    self->instream = g_subprocess_get_stdout_pipe (self->subprocess);
    self->outstream = g_subprocess_get_stdin_pipe (self->subprocess);
}

void
userpasswd_stream_communicate (gpointer window,
                               gchar *current_password,
                               UserpasswdStream *stream)
{
    // userpasswd_stream_create_stream (self);

    // g_assert (self->instream != NULL);
    // g_assert (self->outstream != NULL);

    g_print ("subprocess_path = %s\n", stream->subprocess_path);
    g_print ("current_password = %s\n", current_password);

    //userpasswd_window_success_auth (self->window);
}

static void
userpasswd_stream_class_init (UserpasswdStreamClass *class) {}

static void
userpasswd_stream_init (UserpasswdStream *self) {}

UserpasswdStream *
userpasswd_stream_new (gchar *subprocess_path)
{
    UserpasswdStream *self = USERPASSWD_STREAM (g_object_new (USERPASSWD_TYPE_STREAM, NULL));
    self->subprocess_path = subprocess_path;

    return self;
}