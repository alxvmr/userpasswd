#include "userpasswd-stream.h"

enum {
    CHECK_PASSWD_SUCCESS,
    CHECK_PASSWD_FAIL,
    LAST_SIGNAL
};

static guint userpasswd_stream_signals[LAST_SIGNAL];

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
    // userpasswd_stream_create_stream (stream);

    // g_assert (stream->instream != NULL);
    // g_assert (stream->outstream != NULL);

    g_signal_emit (stream, userpasswd_stream_signals[CHECK_PASSWD_SUCCESS], 0);

    // userpasswd_window_success_auth (self->window);
}

static void
userpasswd_stream_class_init (UserpasswdStreamClass *class)
{
    userpasswd_stream_signals[CHECK_PASSWD_SUCCESS] = g_signal_new (
        "check-passwd-success",
        G_TYPE_FROM_CLASS (class),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0,
        NULL,
        NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE,
        0
    );
}

static void
userpasswd_stream_init (UserpasswdStream *self) {}

UserpasswdStream *
userpasswd_stream_new (gchar *subprocess_path)
{
    UserpasswdStream *self = USERPASSWD_STREAM (g_object_new (USERPASSWD_TYPE_STREAM, NULL));
    self->subprocess_path = subprocess_path;

    return self;
}