#include "userpasswd-stream.h"
#include <json-glib/json-glib.h>

enum {
    CHECK_PASSWD_SUCCESS,
    CHECK_PASSWD_FAIL,
    NEW_LOG,
    LAST_SIGNAL
};

enum {
    CURRENT_PASSWORD,
    NEW_PASSWORD,
    UNKNOWN
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

JsonNode*
string_to_json (gchar *buf)
{
    JsonParser *parser = json_parser_new ();
    GError *error = NULL;

    if (!json_parser_load_from_data (parser, buf, -1, &error))
    {
        g_printerr ("Json parsing error: %s\n", error->message);
        g_error_free (error);
        g_object_unref (parser);
        return NULL;
    }

    JsonNode *root = json_parser_get_root (parser);

    /* TODO: Figure out how to clear the parser without losing the root pointer */

    // g_object_ref (root);
    // g_object_unref (parser);

    return root;
}

gint
get_pam_end_status_code (gchar *req)
{
    JsonNode *node = string_to_json (req);
    JsonObject *node_object = json_node_get_object (node);
    JsonNode *pam_end_node = json_object_get_member (node_object, "pam_end");
    JsonObject *pam_end_node_object = json_node_get_object (pam_end_node);
    JsonNode *pam_status_code_node = json_object_get_member (pam_end_node_object, "pam_status_code");
    gint pam_status_code = -1;

    if (json_node_get_node_type (pam_status_code_node) != JSON_NODE_NULL) {
        pam_status_code = json_node_get_int (pam_status_code_node);
    }

    json_node_unref (node);
    return pam_status_code;
}

gint
get_request_type (gchar *req)
{
    JsonNode *req_node = string_to_json (req);
    if (req_node != NULL) {
        JsonObject *object = json_node_get_object (req_node);
        const gchar *type = json_object_get_string_member (object, "type");

        if (g_strcmp0 (type, "input") == 0) {
            if (json_object_has_member (object, "new_password")) {
                return NEW_PASSWORD;
            }
            else {
                if (json_object_has_member (object, "current_password")) {
                    return CURRENT_PASSWORD;
                }
            }
        }
        json_node_unref (req_node);
    }
    return UNKNOWN;
}

gchar*
get_response (gchar *req,
              gchar *data)
{
    gint type = get_request_type (req);
    if (type == CURRENT_PASSWORD) {
        return g_strdup_printf ("{\"%s\":\"%s\"}\n", "current_password", data);
    }
    else {
        if (type == NEW_PASSWORD) {
            return g_strdup_printf ("{\"%s\":\"%s\"}\n", "new_password", data);
        }
    }

    return NULL;
}

static void
on_data_write (GObject      *outstream,
               GAsyncResult *result,
               gpointer      user_data)
{
    UserpasswdStream *stream = (UserpasswdStream *) user_data;
    GError *error = NULL;
    gssize size;

    if (!g_output_stream_write_all_finish (stream->outstream, result, &size, &error)) {
        if (error) {
            g_printerr ("Could not send child info: %s\n", error->message);
            g_error_free (error);
        }
        return;
    }

    g_print ("WRITE DONE\n");
}

static void
on_data_reciever (GObject      *instream,
                  GAsyncResult *result, 
                  gpointer      user_data)
{
    UserpasswdStream *stream = (UserpasswdStream *) user_data;
    // g_print ("[RECIEVER] STREAM: %p\n", stream);
    GError *error = NULL;
    gssize bytes_read;

    bytes_read = g_input_stream_read_finish (stream->instream, result, &error);
    if (error) {
        g_printerr ("Error reading from subprocess stdout: %s\n", error->message);
        g_error_free (error);
        return;
    }

    if (bytes_read <= 0) {
        /*
        TODO: async
        */
        gboolean is_exit_success = g_subprocess_wait_check (stream->subprocess, NULL, &error);

        if (error) {
            g_printerr("Error checking subprocess: %s\n", error->message);
            g_error_free(error);
            return;
        }

        if (is_exit_success) {
            g_print ("CHILD DIED\n");
            gint pam_status_code = get_pam_end_status_code (g_list_last(stream->requests)->data);
            g_print ("PAM CODE: %d\n", pam_status_code);
            if (pam_status_code != 0) {
                g_signal_emit (stream, userpasswd_stream_signals[CHECK_PASSWD_FAIL], 0);
            }
        }

        return;
    }

    stream->buffer[bytes_read + 1] = '\0';

    if (stream->request == NULL) {
        stream->request = g_strdup("");
    }

    stream->request = g_strconcat (stream->request, stream->buffer, NULL);

    if (stream->buffer[bytes_read - 1] == '\n') {
        /* Обработка законченного json дочернего процесса*/
        g_signal_emit (stream, userpasswd_stream_signals[NEW_LOG], 0, stream->request);
        stream->requests = g_list_append (stream->requests, g_strdup (stream->request));
        gchar *response = get_response (stream->request, stream->current_password);

        g_print ("REQUEST: %s", stream->request);
        g_print ("RESPONSE: %s\n", response);

        if (response != NULL) {
            g_output_stream_write_all_async (
                stream->outstream,
                response,
                strlen (response),
                G_PRIORITY_DEFAULT,
                NULL,
                on_data_write,
                stream
            );
        }

        // g_signal_emit (stream, userpasswd_stream_signals[CHECK_PASSWD_SUCCESS], 0);

        g_free (stream->request);
        stream->request = NULL;
    }

    memset (stream->buffer, 0, sizeof(stream->buffer));

    g_input_stream_read_async (
        stream->instream,
        stream->buffer,
        sizeof (stream->buffer) - 1,
        G_PRIORITY_DEFAULT,
        NULL,
        on_data_reciever,
        stream
    );
}

void
userpasswd_stream_communicate (gpointer window,
                               const gchar *current_password,
                               UserpasswdStream *stream)
{
    userpasswd_stream_create_stream (stream);

    g_assert (stream->instream != NULL);
    g_assert (stream->outstream != NULL);

    stream->current_password = g_strdup (current_password);

    GError *error = NULL;

    g_input_stream_read_async (
        stream->instream,
        stream->buffer,
        sizeof (stream->buffer) - 1,
        G_PRIORITY_DEFAULT,
        NULL,
        on_data_reciever,
        stream
    );
    // g_signal_emit (stream, userpasswd_stream_signals[CHECK_PASSWD_SUCCESS], 0);
    // g_signal_emit (stream, userpasswd_stream_signals[CHECK_PASSWD_FAIL], 0, "error mess");
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

    userpasswd_stream_signals[CHECK_PASSWD_FAIL] = g_signal_new (
        "check-passwd-fail",
        G_TYPE_FROM_CLASS (class),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0,
        NULL,
        NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE,
        0
    );

    userpasswd_stream_signals[NEW_LOG] = g_signal_new (
        "new-log",
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