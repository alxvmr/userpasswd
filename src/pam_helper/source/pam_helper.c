#include <security/pam_appl.h>
#include "../include/pam_helper_json.h"
#include <stdio.h>
#include <pwd.h>

#define	PASSWD_SERVICE	"passwd"
#define PAM_OLDPASS    0
#define PAM_NEWPASS    1
#define PAM_REPEATPASS 2
#define PAM_SKIPASS    3

gchar *CONV_ERROR = NULL;

static inline int getstate(const char *msg) {
    /* Interpret possible PAM messages (not including errors) */
    if (!strcmp(msg, "Current Password: "))
        return PAM_OLDPASS;
    if (!strcmp(msg, "Current password: "))
        return PAM_OLDPASS;

    if (!strcmp(msg, "New Password: "))
        return PAM_NEWPASS;
    if (!strcmp(msg, "Enter new password: "))
        return PAM_NEWPASS;
    if (!strcmp(msg, "Reenter new Password: "))
        return PAM_REPEATPASS;
    if (!strcmp(msg, "Re-type new password: "))
        return PAM_REPEATPASS;

    return PAM_SKIPASS;
}

gchar *
get_data_from_parent (int type_data)
{
    gchar *buf = NULL;
    gchar *data_input = NULL;
    JsonNode *output_json = NULL;
    gchar *key = "current_password";
    size_t size = 0;

    if (type_data == PAM_NEWPASS) {
        key = "new_password";
    }
    if (type_data == PAM_REPEATPASS) {
        key = "repeat_new_password";
    }

    output_json = init_json_node_output (key);
    print_json (output_json);
    fflush (stdout);

    if (getline(&buf, &size, stdin) != -1) {
        buf[strcspn(buf, "\n")] = 0;
    } else {
        g_printerr ("Input error");
    }

    JsonNode *input_node = string_to_json (buf);
    JsonObject *input_object = json_node_get_object (input_node);
    JsonNode *input_member = json_object_get_member (input_object, key);
    if (json_node_get_node_type (input_member) != JSON_NODE_NULL) {
        data_input = g_strdup (json_node_get_string (input_member));
    }

    json_node_unref (input_member);
    json_node_unref (output_json);
    free (buf);

    return data_input;
}

int
non_interactive_conv (int                        num_msg,
                      const struct pam_message **msgm,
                      struct pam_response      **response,
                      void                      *appdata_ptr)
{
    JsonNode *root = (JsonNode *) appdata_ptr;
    JsonObject *log_object = json_node_get_object (root);
    struct pam_response *resp = NULL;
    const struct pam_message *message;
    const gchar *answ = NULL;
    guint size_answ;

    resp = malloc(sizeof(struct pam_response) * num_msg);
    if (resp == NULL) {
        return PAM_CONV_ERR;
    }

    for (int i = 0; i < num_msg; i++) {
        answ = NULL;
        message = msgm[i];

        switch (message->msg_style) {
            case PAM_TEXT_INFO:
            /*TODO: Add collection of information messages for debugging*/
                json_object_set_string_member (log_object, "pam_conv", message->msg);
                print_json (root);
                return PAM_SUCCESS;
            case PAM_ERROR_MSG:
                if (CONV_ERROR) {
                    g_free (CONV_ERROR);
                }
                CONV_ERROR = g_strdup (message->msg);
                return PAM_CONV_ERR;
            case PAM_PROMPT_ECHO_ON:
            case PAM_PROMPT_ECHO_OFF:
                switch (getstate(message->msg)) {
                    /* TODO: add NULL check*/
                    case PAM_NEWPASS:
                    case PAM_REPEATPASS:
                    case PAM_OLDPASS:
                        gchar *passwd = get_data_from_parent (getstate(message->msg));
                        answ = g_strdup (passwd);
                        g_free (passwd);
                        break;
                    case PAM_SKIPASS:
                        answ = NULL;
                        break;
                    default:
                        // TODO: добавить лог о неизвестном сообщении
                        break;
                }
                if ((answ != NULL) && (answ[0] != '\0')) {
                    resp[i].resp = g_strdup (answ);

                    if (resp[i].resp == NULL) {
                        resp[i].resp_retcode = PAM_BUF_ERR;
                    }
                    else {
                        resp[i].resp_retcode = PAM_SUCCESS;
                    }
                }
                else {
                    resp[i].resp_retcode = PAM_CONV_ERR;
                    return PAM_CONV_ERR;
                }
                break;
            default:
                break;
        }
    }

    *response = resp;
    return PAM_SUCCESS;
}

int
wrapped_pam_end (pam_handle_t *pamh, int retval, JsonObject *object)
{
    set_member_pam (object, "pam_end", retval, pamh);
    return pam_end (pamh, retval);
}

int
setup_pam (gchar* user_name, JsonNode *root)
{
    g_assert (user_name != NULL);

    pam_handle_t *pamh = NULL;
    int retval;
    JsonObject *object = json_node_get_object (root);
    struct pam_conv conv = { non_interactive_conv, root};

    retval = pam_start (PASSWD_SERVICE, user_name, &conv, &pamh);
    set_member_pam (object, "pam_start", retval, pamh);

    if (retval != PAM_SUCCESS) {
        wrapped_pam_end (pamh, retval, object);
        print_json (root);
        return retval;
    }
    print_json (root);

    retval = pam_chauthtok (pamh, 0);
    set_member_pam (object, "pam_chauthtok", retval, pamh);

    if (retval != PAM_SUCCESS) {
        if (CONV_ERROR != NULL) {
            json_object_set_string_member(object, "pam_conv", CONV_ERROR);
        }
        wrapped_pam_end (pamh, retval, object);
        print_json (root);
        return retval;
    }
    print_json (root);

    wrapped_pam_end (pamh, PAM_SUCCESS, object);

    if (retval != PAM_SUCCESS) {
        wrapped_pam_end (pamh, retval, object);
        print_json (root);
        return retval;
    }

    print_json (root);

    return PAM_SUCCESS;
}

gchar* get_username ()
{
    uid_t uid = getuid();

    struct passwd pwd;
    struct passwd *pwd_res = NULL;
    char buf[4096];

    int ret = getpwuid_r (uid, &pwd, buf, sizeof(buf), &pwd_res);
    if (ret) {
        return NULL;
    }
    return g_strdup (pwd.pw_name);
}

/*
  Allows you to avoid character conversion 
  when outputting Cyrillic characters
*/
void g_print_no_convert(const gchar *buf)
{
    fputs(buf, stdout);
}

int main (int argc, char *argv[]) {
    g_set_print_handler(g_print_no_convert);

    int res;
    
    JsonNode *root = init_json_node ();
    JsonObject *object = json_node_get_object(root);

    g_assert (object != NULL);

    gchar *username = get_username ();
    if (!username) {
        json_object_set_string_member(object, "main_error", "Unable to retrieve username");
        print_json (root);

        clear_json_object (object);
        json_node_free (root);

        return 1;
    }

    json_object_set_string_member(object, "user_name", username);

    res = setup_pam (username, root);

    clear_json_object (object);
    json_node_free (root);

    g_free(CONV_ERROR);

    return 0;
}