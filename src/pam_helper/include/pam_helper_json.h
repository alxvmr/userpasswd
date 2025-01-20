#ifndef PAM_HELPER_JSON_H
#define PAM_HELPER_JSON_H
#include <json-glib/json-glib.h>
#include <security/pam_appl.h>
#include "translate.h"

void set_member_pam (JsonObject *object, gchar *member_name, int retval, pam_handle_t *pam_h);
JsonNode *init_json_node ();
JsonNode* init_json_node_output (gchar* key);
gchar *get_string_from_json_node (JsonNode *root);
JsonNode *string_to_json (gchar *buf);
void print_json (JsonNode *root);
void clear_json_object (JsonObject *object);
#endif