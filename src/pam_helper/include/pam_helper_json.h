#ifndef PAM_HELPER_JSON_H
#define PAM_HELPER_JSON_H
#include <json-glib/json-glib.h>
#include <security/pam_appl.h>
#include "translate.h"

void set_member_pam (JsonObject *object, int retval, pam_handle_t *pam_h);
JsonNode *init_json_node_pam ();
JsonNode *init_json_node_input (gchar* key);
JsonNode *init_json_node_pam_conv ();

gchar *get_string_from_json_node (JsonNode *root);
JsonNode *string_to_json (gchar *buf);
void print_json (JsonNode *root);
void clear_json_object (JsonObject *object);
#endif
