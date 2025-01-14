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
get_rc (GList *outputs)
{
    rcHelper *rc = g_new (rcHelper, 1);
    init_rcHelper (rc);

    JsonNode *user_name_node = NULL;
    JsonNode *main_error_node = NULL;
    JsonNode *conv_node = NULL;
    gint pam_status_code = 0;

    GList *last_element = g_list_last (outputs);

    if (last_element == NULL)
    {
        g_printerr ("Error processing pam_helper output");
        return rc;
    }

    JsonNode *last_answer_node = (JsonNode *) last_element->data;
    JsonObject *last_answer = json_node_get_object (last_answer_node);

    user_name_node = json_object_get_member (last_answer, "user_name");
    if (json_node_get_node_type (user_name_node) != JSON_NODE_NULL) {
        rc->user_name = g_strdup (json_node_get_string (user_name_node));
    }

    main_error_node = json_object_get_member (last_answer, "main_error");
    if (json_node_get_node_type (main_error_node) != JSON_NODE_NULL) {
        rc->error_message_en = g_strdup (json_node_get_string (main_error_node));

        g_list_free_full (last_element, (GDestroyNotify)json_node_unref);
        return rc;
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

            g_list_free_full (last_element, (GDestroyNotify)json_node_unref);
            g_list_free_full (nested, g_free);
            return rc;
        }
    }
    
    g_list_free_full (last_element, (GDestroyNotify)json_node_unref);
    g_list_free_full (nested, g_free);

    rc->rc = TRUE;
    return rc;
}

void create_pipe (gchar *current_password,
                  gchar *new_password,
                  gchar *retype_password)
{
    GSubprocess *subprocess;
    GError *error = NULL;
    g_autofree gchar *stdout_buf = NULL;
    GList *output_data = NULL;

    /* TODO: check new password */
    /* TODO: add timeout*/

    subprocess = g_subprocess_new (
        G_SUBPROCESS_FLAGS_STDOUT_PIPE,
        &error,
        "./bin/pam_helper", current_password, new_password, NULL
    );

    if (error) {
        g_printerr ("Error creating subprocess: %s\n", error->message);
        g_error_free (error);
        return;
    }

    GList *answer_list = NULL;

    if (g_subprocess_communicate_utf8 (subprocess, NULL, NULL, &stdout_buf, NULL, NULL)){
        // Если вернулся true, то процесс завершился
        // TODO: добавить err и проверку статуса завершения процесса

        gchar **lines = g_strsplit (stdout_buf, "\n", -1);

        for (gint i = 0; lines[i] != NULL; i++) {
            if (g_utf8_strlen (lines[i], -1) != 0) {
                answer_list = g_list_append (answer_list, string_to_json(lines[i]));
            }
        }
        g_strfreev (lines);
    }

    rcHelper *rc = get_rc (answer_list);
    print_rcHelper (rc);

    g_object_unref (subprocess);
}