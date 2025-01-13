#ifndef PAM_HELPER_JSON_H
#define PAM_HELPER_JSON_H
#include <json-glib/json-glib.h>
#include <security/pam_appl.h>
#include "translate.h"

void set_member_pam (JsonObject *object, gchar *member_name, int retval, pam_handle_t *pam_h);
JsonNode *init_json_node ();
gchar *get_string_from_json_node (JsonNode *root);
void print_json (JsonNode *root);
void clear_json_object (JsonObject *object);
#endif