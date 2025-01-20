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
} UserpasswdStream;

UserpasswdStream* userpasswd_stream_new (gchar *subprocess_path);
void userpasswd_stream_communicate (gpointer window, gchar *current_password, UserpasswdStream *stream);

G_END_DECLS

#endif
