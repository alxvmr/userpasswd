#include "userpasswd-stream.h"
#include "userpasswd-marshal.h"
#include <json-glib/json-glib.h>

enum {
    DRAW_CHECK_PASSWD,
    DRAW_NEW_PASSWD,
    NEW_STATUS,
    NEW_LOG,
    LAST_SIGNAL
};

enum {
    CURRENT_PASSWORD,
    NEW_PASSWORD,
    REPEAT_NEW_PASSWORD,
    PAM_STATUS,
    PAM_CONV,
    UNKNOWN
};

static guint userpasswd_stream_signals[LAST_SIGNAL];

G_DEFINE_TYPE (UserpasswdStream, userpasswd_stream, G_TYPE_OBJECT);

static void
stream_clear_data (UserpasswdStream *self)
{
    switch (self->last_input_step) {
        case (CURRENT_PASSWORD):
            self->current_password = NULL;
            break;
        case (NEW_PASSWORD):
            self->new_password = NULL;
            break;
        default:
            break;
    }

    self->current_step = -1;
    self->prev_step = -1;
    self->last_input_step = -1;
}

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
    JsonNode *node_pam_status_code = json_object_get_member (node_object, "pam_status_code");
    gint pam_status_code = -1;

    if (json_node_get_node_type (node_pam_status_code) != JSON_NODE_NULL) {
        pam_status_code = json_node_get_int (node_pam_status_code);
    }

    json_node_unref (node);
    return pam_status_code;
}

gchar *
get_pam_status (gchar *req)
{
    JsonNode *node = string_to_json (req);
    JsonObject *node_object = json_node_get_object (node);
    JsonNode *node_pam_status_code = json_object_get_member (node_object, "pam_status_code");
    JsonNode *node_pam_status_mess_en = json_object_get_member (node_object, "pam_status_mess_en");
    JsonNode *node_pam_status_mess_ru = json_object_get_member (node_object, "pam_status_mess_ru");

    gint status_code = -1;
    gchar *status_mess_en = "NULL";
    gchar *status_mess_ru = "NULL";

    if (json_node_get_node_type (node_pam_status_code) != JSON_NODE_NULL) {
        status_code = json_node_get_int (node_pam_status_code);
    }
    if (json_node_get_node_type (node_pam_status_mess_en) != JSON_NODE_NULL) {
        status_mess_en = g_strdup (json_node_get_string (node_pam_status_mess_en));
    }
    if (json_node_get_node_type (node_pam_status_mess_ru) != JSON_NODE_NULL) {
        status_mess_ru = g_strdup (json_node_get_string (node_pam_status_mess_ru));
    }

    json_node_unref (node);

    return g_strdup_printf ("pam_status_code: %d; pam_status_mess_ru: %s; pam_status_mess_en: %s",
                            status_code,
                            status_mess_ru,
                            status_mess_en);
}

gchar*
get_pam_conv_mess (gchar *req)
{
    JsonNode *node = string_to_json (req);
    JsonObject *node_object = json_node_get_object (node);
    JsonNode *pam_conv_node = json_object_get_member (node_object, "pam_conv_mess");
    gchar *mess = NULL;

    if (json_node_get_node_type (pam_conv_node) != JSON_NODE_NULL) {
        mess = g_strdup (json_node_get_string (pam_conv_node));
    }

    json_node_unref (node);

    return mess ? mess : "";
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
                json_node_unref (req_node);
                return NEW_PASSWORD;
            }
            if (json_object_has_member (object, "current_password")) {
                json_node_unref (req_node);
                return CURRENT_PASSWORD;
            }
            if (json_object_has_member (object, "repeat_new_password")) {
                json_node_unref (req_node);
                return REPEAT_NEW_PASSWORD;
            }
        }

        const gchar *type_content = json_object_get_string_member (object, "type_content");
        if (g_strcmp0 (type_content, "pam_status") == 0) {
            json_node_unref (req_node);
            return PAM_STATUS;
        }
        if (g_strcmp0 (type_content, "pam_conv") == 0) {
            json_node_unref (req_node);
            return PAM_CONV;
        }

    }
    return UNKNOWN;
}

gchar*
get_response (gint   step,
              gchar *data)
{
    if (step == CURRENT_PASSWORD) {
        return g_strdup_printf ("{\"%s\":\"%s\"}\n", "current_password", data);
    }
    if (step == NEW_PASSWORD) {
        return g_strdup_printf ("{\"%s\":\"%s\"}\n", "new_password", data);
    }
    if (step == REPEAT_NEW_PASSWORD) {
        return g_strdup_printf ("{\"%s\":\"%s\"}\n", "repeat_new_password", data);
    }

    return NULL;
}

static void
stream_free (UserpasswdStream *stream)
{
    GError *error_in = NULL;
    GError *error_out = NULL;

    g_input_stream_close (stream->instream, NULL, &error_in);
    if (error_in) {
        g_printerr ("Error close input stream: %s\n", error_in->message);
        g_error_free (error_in);
    }

    g_output_stream_close (stream->outstream, NULL, &error_out);
    if (error_out) {
        g_printerr ("Error close output stream: %s\n", error_out->message);
        g_error_free (error_out);
    }

    g_object_unref (stream->instream);
    g_object_unref (stream->outstream);

    stream->instream = NULL;
    stream->outstream = NULL;
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

void
on_new_password_reciever (gpointer          window,
                          const gchar      *new_password,
                          UserpasswdStream *stream)
{
    stream->new_password = g_strdup (new_password);
    gchar *response = get_response (stream->current_step, stream->new_password);

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
}

void
on_password_reciever (gpointer          window,
                      const gchar      *current_password,
                      UserpasswdStream *stream)
{
    g_assert (stream->outstream != NULL);
    
    stream->current_password = g_strdup (current_password);
    gchar *response = get_response (CURRENT_PASSWORD, stream->current_password);

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
}

static void
on_data_reciever (GObject      *instream,
                  GAsyncResult *result, 
                  gpointer      user_data)
{
    UserpasswdStream *stream = (UserpasswdStream *) user_data;
    GError *error = NULL;
    gssize bytes_read;

    bytes_read = g_input_stream_read_finish (stream->instream, result, &error);
    if (error) {
        g_printerr ("Error reading from subprocess stdout: %s\n", error->message);
        g_error_free (error);
        return;
    }

    if (bytes_read <= 0) {
        stream_free (stream);

        g_print ("CHILD DIED\n");
        gint pam_status_code = get_pam_end_status_code (stream->last_request);
        g_print ("PAM CODE: %d\n", pam_status_code);
        if (pam_status_code != 0) {
            g_signal_emit (stream, userpasswd_stream_signals[NEW_STATUS], 0, "Error", "error");
            stream_clear_data (stream);
            userpasswd_stream_communicate (NULL, stream);
        }
        else {
            g_signal_emit (stream, userpasswd_stream_signals[NEW_STATUS], 0, "Success", "success");
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
        stream->last_request = g_strdup (stream->request);
        g_print ("REQUEST: %s", stream->request);

        if (stream->current_step != -1 &&
            stream->current_step != PAM_CONV &&
            stream->current_step != PAM_STATUS)
        {
            stream->prev_step = stream->current_step;
        }

        stream->current_step = get_request_type (stream->request);

        if (stream->current_step == PAM_CONV || stream->current_step == PAM_STATUS) {
            gchar *mess = NULL;
            gchar *sender = NULL;
            if (stream->current_step == PAM_CONV) {
                mess = get_pam_conv_mess (stream->request);
                sender = "pam_conv";
            }
            else {
                mess = get_pam_status (stream->request);
                sender = "pam_status";
            }

            if (mess != NULL) {
                g_signal_emit (stream,
                               userpasswd_stream_signals[NEW_LOG],
                               0,
                               mess,
                               sender);
                g_free (mess);
            }
        } else {
            if (stream->current_step == CURRENT_PASSWORD) {
                stream->last_input_step = stream->current_step;
                if (stream->current_password == NULL)
                    g_signal_emit (stream, userpasswd_stream_signals [DRAW_CHECK_PASSWD], 0);
                else
                    on_password_reciever (NULL, stream->current_password, stream);
            }

            if (stream->current_step == NEW_PASSWORD) {
                stream->last_input_step = stream->current_step;
                if (stream->prev_step == NEW_PASSWORD) {
                    g_signal_emit (stream, userpasswd_stream_signals[NEW_STATUS], 0, "Weak password", "warning");
                }
                else {
                    if (stream->new_password == NULL)
                    g_signal_emit (stream, userpasswd_stream_signals [DRAW_NEW_PASSWD], 0);
                    else
                        on_new_password_reciever (NULL, stream->new_password, stream);
                }
            }

            if (stream->current_step == REPEAT_NEW_PASSWORD) {
                gchar *response = get_response (stream->current_step, stream->new_password);

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

                g_free (response);
            }
        }
        
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
                               UserpasswdStream *stream)
{
    userpasswd_stream_create_stream (stream);

    g_assert (stream->instream != NULL);
    g_assert (stream->outstream != NULL);

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
    userpasswd_stream_signals[DRAW_CHECK_PASSWD] = g_signal_new (
        "draw-check-passwd",
        G_TYPE_FROM_CLASS (class),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0,
        NULL,
        NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE,
        0
    );

    userpasswd_stream_signals[DRAW_NEW_PASSWD] = g_signal_new (
        "draw-new-passwd",
        G_TYPE_FROM_CLASS (class),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0,
        NULL,
        NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE,
        0
    );

    userpasswd_stream_signals[NEW_STATUS] = g_signal_new (
        "new-status",
        G_TYPE_FROM_CLASS (class),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0,
        NULL,
        NULL,
        g_cclosure_user_marshal_VOID__STRING_STRING,
        G_TYPE_NONE,
        2,
        G_TYPE_STRING,
        G_TYPE_STRING
    );

    userpasswd_stream_signals[NEW_LOG] = g_signal_new (
        "new-log",
        G_TYPE_FROM_CLASS (class),
        G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
        0,
        NULL,
        NULL,
        g_cclosure_user_marshal_VOID__STRING_STRING,
        G_TYPE_NONE,
        2,
        G_TYPE_STRING,
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
    self->current_step = -1;
    self->prev_step = -1;
    self->last_input_step = -1;

    return self;
}