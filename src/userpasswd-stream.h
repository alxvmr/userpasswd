#ifndef USERPASSWDSTREAM_H
#define USERPASSWDSREAM_H
#include <gio/gio.h>

G_BEGIN_DECLS

#define USERPASSWD_TYPE_STREAM (userpasswd_stream_get_type ())

G_DECLARE_FINAL_TYPE (UserpasswdStream, userpasswd_stream, USERPASSWD, STREAM, GObject)

typedef struct _UserpasswdStream {
    GObject parent_instance;

    gchar *subprocess_path;
    GSubprocess *subprocess;
    GInputStream *instream;
    GOutputStream *outstream;
    gchar buffer[2];
    gchar *request;
    gchar *last_request;

    gchar *current_password;
    gchar *new_password;

    gint current_step;
    gint prev_step;
} UserpasswdStream;

UserpasswdStream* userpasswd_stream_new (gchar *subprocess_path);
void userpasswd_stream_communicate (gpointer window, UserpasswdStream *stream);
void on_password_reciever (gpointer window, const gchar *current_password, UserpasswdStream *stream);
void on_new_password_reciever (gpointer window, const gchar *new_password, UserpasswdStream *stream);

G_END_DECLS

#endif
