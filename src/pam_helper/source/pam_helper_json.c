#include "../include/pam_helper_json.h"

void
set_member_pam (JsonObject   *object,
                int           retval, 
                pam_handle_t *pamh)
{
    json_object_set_int_member (object, "pam_status_code", retval);
    json_object_set_string_member (object, "pam_status_mess_en", pam_strerror(pamh, retval));
    json_object_set_string_member (object, "pam_status_mess_ru", get_translate_by_pam_retval (pamh, retval));
}

JsonNode *
init_json_node_pam_conv ()
{
    JsonNode *root = json_node_new (JSON_NODE_OBJECT);
    JsonObject *object = json_object_new ();

    json_object_set_member (object, "pam_conv_mess", json_node_new (JSON_NODE_NULL));
    json_object_set_string_member (object, "type_content", "pam_conv");
    json_object_set_string_member (object, "type", "output");

    json_node_init (root, JSON_NODE_OBJECT);
    json_node_set_object (root, object);

    return root;
}

JsonNode *
init_json_node_pam ()
{
    JsonNode *root = json_node_new (JSON_NODE_OBJECT);
    JsonObject *object = json_object_new ();

    json_object_set_member (object, "pam_status_code", json_node_new (JSON_NODE_NULL));
    json_object_set_member (object, "pam_status_mess_en", json_node_new (JSON_NODE_NULL));
    json_object_set_member (object, "pam_status_mess_ru", json_node_new (JSON_NODE_NULL));
    json_object_set_string_member (object, "type_content", "pam_status");
    json_object_set_string_member (object, "type", "output");

    json_node_init (root, JSON_NODE_OBJECT);
    json_node_set_object (root, object);

    return root;
}

JsonNode*
init_json_node_input (gchar *key)
{
    JsonNode *root = json_node_new (JSON_NODE_OBJECT);
    JsonObject *object = json_object_new ();

    json_object_set_member (object, key, json_node_new(JSON_NODE_NULL));
    json_object_set_string_member (object, "type", "input");
    json_node_init (root, JSON_NODE_OBJECT);
    json_node_set_object (root, object);

    return root;
}

gchar *
get_string_from_json_node (JsonNode *root)
{
    JsonGenerator *generator = json_generator_new();
    json_generator_set_root (generator, root);
    gchar *json_string = json_generator_to_data (generator, NULL);

    g_object_unref (generator);

    return json_string;
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

void
print_json (JsonNode *root)
{
    gchar *output = get_string_from_json_node (root);
    g_print ("%s\n", output);
    
    g_free (output);
}

void
clear_json_object (JsonObject *object)
{
    GList *members = json_object_get_members (object);

    for (GList *l = members; l != NULL; l = l->next) {
        const gchar *key = (const gchar *) l->data;
        json_object_remove_member (object, key);
    }

    g_list_free (members);
    json_object_unref (object);
}