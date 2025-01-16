#include <gio/gio.h>
#include <string.h>
#include <json-glib/json-glib.h>

typedef struct
{
    gboolean       rc;
    const gchar   *user_name;
    const gchar   *step;
    gint    pam_status_code;
    const gchar   *error_message_en;
    const gchar   *error_message_ru;
    const gchar   *pam_conv_message;
} rcHelper;

void
init_rcHelper (rcHelper *rc)
{
    if (rc != NULL)
    {
        rc->rc = FALSE;
        rc->user_name = NULL;
        rc->step = NULL;
        rc->pam_status_code = 0;
        rc->error_message_en = NULL;
        rc->error_message_ru = NULL;
        rc->pam_conv_message = NULL;
    }
}

void
print_rcHelper (rcHelper *rc)
{
    if (rc) {
        g_print ("%s: %d\n", "return_code", rc->rc);
        g_print ("%s: %s\n", "step", rc->step);
        g_print ("%s: %d\n", "pam status code", rc->pam_status_code);
        g_print ("%s: %s\n", "mess_en", rc->error_message_en);
        g_print ("%s: %s\n", "mess_ru", rc->error_message_ru);
        g_print ("%s: %s\n", "conv mess", rc->pam_conv_message);
        g_print ("%s: %s\n", "user name", rc->user_name);
    }
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

rcHelper*
get_rc (gchar *req)
{
    rcHelper *rc = g_new (rcHelper, 1);
    init_rcHelper (rc);

    JsonNode *user_name_node = NULL;
    JsonNode *main_error_node = NULL;
    JsonNode *conv_node = NULL;
    gint pam_status_code = 0;

    if (req == NULL)
    {
        g_printerr ("Error processing pam_helper output");
        return rc;
    }

    JsonNode *last_answer_node = string_to_json (req);
    JsonObject *last_answer = json_node_get_object (last_answer_node);

    user_name_node = json_object_get_member (last_answer, "user_name");
    if (json_node_get_node_type (user_name_node) != JSON_NODE_NULL) {
        rc->user_name = g_strdup (json_node_get_string (user_name_node));
    }

    main_error_node = json_object_get_member (last_answer, "main_error");
    if (json_node_get_node_type (main_error_node) != JSON_NODE_NULL) {
        rc->error_message_en = g_strdup (json_node_get_string (main_error_node));
    }

    GList *nested = NULL;
    nested = g_list_append (nested, g_strdup("pam_start"));
    nested = g_list_append (nested, g_strdup("pam_chauthtok"));
    nested = g_list_append (nested, g_strdup("pam_end"));

    conv_node = json_object_get_member (last_answer, "pam_conv");
    if (json_node_get_node_type (conv_node) != JSON_NODE_NULL) {
        rc->pam_conv_message = g_strdup (json_node_get_string (conv_node));
    }

    for (GList *l = nested; l != NULL; l = l->next) {
        JsonNode *pam_nested_node = json_object_get_member (last_answer, l->data);
        JsonObject *pam_nested_object = json_node_get_object (pam_nested_node);
        pam_status_code = json_object_get_int_member (pam_nested_object, "pam_status_code");

        if (pam_status_code != 0) {
            rc->step = g_strdup (l->data);
            rc->pam_status_code = pam_status_code;

            JsonNode *mess_en_node = json_object_get_member (pam_nested_object, "pam_status_mess_en");
            JsonNode *mess_ru_node = json_object_get_member (pam_nested_object, "pam_status_mess_ru");
            if (json_node_get_node_type (mess_en_node) != JSON_NODE_NULL) {
                rc->error_message_en = g_strdup (json_node_get_string (mess_en_node));
            }
            if (json_node_get_node_type (mess_ru_node) != JSON_NODE_NULL) {
                rc->error_message_ru = g_strdup (json_node_get_string (mess_ru_node));
            }

            g_list_free_full (nested, g_free);
            return rc;
        }
    }
    
    g_list_free_full (nested, g_free);

    rc->rc = TRUE;
    return rc;
}

gchar*
get_response (gchar *req,
              gchar *current_password,
              gchar *new_password)
{
    JsonNode *req_node = string_to_json (req);
    if (req_node != NULL) {
        JsonObject *object = json_node_get_object (req_node);
        const gchar *type = json_object_get_string_member (object, "type");

        if (g_strcmp0 (type, "input") == 0) {
            if (json_object_has_member (object, "new_password")) {
                return g_strdup_printf ("{\"%s\":\"%s\"}\n", "new_password", new_password);
            }
            else {
                if (json_object_has_member (object, "current_password")) {
                    return g_strdup_printf ("{\"%s\":\"%s\"}\n", "current_password", current_password);
                }
            }
        }
        json_node_unref (req_node);
    }
    return NULL;
}

gchar *
remove_substring (gchar *original, gchar *substr) {
    gchar *result;
    gchar *pos;
    gsize original_len = strlen (original);
    gsize substr_len = strlen (substr);

    pos = g_strstr_len (original, original_len, substr);
    if (pos == NULL) {
        return g_strdup (original);
    }

    gsize result_len = original_len - substr_len;
    result = g_malloc (result_len + 1);
    g_strlcpy (result, original, pos - original + 1);
    g_strlcpy (result, pos + substr_len, result_len + 1);

    return result;
}

void create_pipe (gchar *current_password,
                  gchar *new_password,
                  gchar *retype_password)
{
    GSubprocess *subprocess;
    GError *error = NULL;
    g_autofree gchar *stdout_buf = NULL;
    GList *output_data = NULL;
    GInputStream *instream = NULL;
    GOutputStream *outstream = NULL;
    gchar buffer[2048];
    gchar *last_req_from_child = NULL;
    rcHelper *rc = NULL;
    gchar *acc_data = NULL;

    /* TODO: check new password */
    /* TODO: add timeout*/

    subprocess = g_subprocess_new (
        G_SUBPROCESS_FLAGS_STDIN_PIPE | G_SUBPROCESS_FLAGS_STDOUT_PIPE,
        &error,
        "./bin/pam_helper", current_password, new_password, NULL
    );

    if (error) {
        g_printerr ("Error creating subprocess: %s\n", error->message);
        g_error_free (error);
        return;
    }

    instream = g_subprocess_get_stdout_pipe (subprocess);
    outstream = g_subprocess_get_stdin_pipe (subprocess);

    acc_data = g_strdup ("");

    while (TRUE) {
        gssize bytes_read = g_input_stream_read (instream, buffer, sizeof(buffer) - 1, NULL, &error);
        if (bytes_read < 0) {
            g_printerr("Error reading from subprocess stdout: %s\n", error->message);
            g_error_free(error);
            break;
        } else if (bytes_read == 0) {
            break;
        }

        gboolean isIncomplete = TRUE;
        acc_data = g_strconcat (acc_data, buffer, NULL);

        if (buffer[bytes_read - 1] != '\n') {
            isIncomplete = FALSE;
        }

        gchar **lines = g_strsplit (acc_data, "\n", -1);

        for (gint i = 0; lines[i] != NULL; i++) {
            if (isIncomplete == FALSE && lines[i+1] == NULL) {
                break;
            }
            acc_data = remove_substring (acc_data, lines[i]);
            // g_print ("Child: %s\n", lines[i]);
            if (g_utf8_strlen (lines[i], -1) != 0) {
                gchar *resp = get_response (lines[i], current_password, new_password);
                if (resp != NULL) {
                    // g_print ("Response: %s", resp);
                    g_output_stream_write_all (outstream, resp, strlen(resp), NULL, NULL, &error);
                    g_free (resp);
                    if (error) {
                        g_printerr ("Error writing to subprocess stdin: %s\n", error->message);
                        g_error_free (error);
                        break;
                    }
                    continue;
                }
                last_req_from_child = g_strdup (lines[i]);
            }
        }
        g_strfreev (lines);
        memset (buffer, 0, sizeof(buffer));
    }
    g_free (acc_data);

    rc = get_rc (last_req_from_child);
    print_rcHelper (rc);

    g_input_stream_close (instream, NULL, &error);
    if (error) {
        g_printerr ("Error close input stream: %s\n", error->message);
        g_error_free (error);
        exit (1);
    }
    g_output_stream_close (outstream, NULL, &error);
    if (error) {
        g_printerr ("Error close output stream: %s\n", error->message);
        g_error_free (error);
        exit (1);
    }
    g_subprocess_wait_check (subprocess, NULL, &error);
    g_object_unref (subprocess);
}