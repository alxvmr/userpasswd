#include <security/pam_appl.h>
#include "../include/pam_helper_json.h"
#include <stdio.h>
#include <pwd.h>

#define	PASSWD_SERVICE	"passwd"
#define PAM_OLDPASS    0
#define PAM_NEWPASS    1
#define PAM_REPEATPASS 2
#define PAM_SKIPASS    3

static inline int getstate(const char *msg) {
    /* Interpret possible PAM messages (not including errors)
       TODO: What should be the UI for kerberos and winbind?
    */
    gchar *trimmed_msg = g_strdup(msg);
    g_strstrip(trimmed_msg);

    if (!strcmp(trimmed_msg, "Current Password:") || !strcmp(trimmed_msg, "Current password:") ||
        !strcmp(trimmed_msg, "Current Kerberos password:") || !strcmp(trimmed_msg, "(current) NT password:"))
        return PAM_OLDPASS;

    if (!strcmp(trimmed_msg, "New Password:") || !strcmp(trimmed_msg, "Enter new password:") ||
        !strcmp(trimmed_msg, "Enter new NT password:"))
        return PAM_NEWPASS;

    if (!strcmp(trimmed_msg, "Reenter new Password:") || !strcmp(trimmed_msg, "Re-type new password:") ||
        !strcmp(trimmed_msg, "Retype new NT password:"))
        return PAM_REPEATPASS;

    g_free(trimmed_msg);
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

    output_json = init_json_node_input (key);
    print_json (output_json);
    fflush (stdout);

    if (getline(&buf, &size, stdin) != -1) {
        buf[strcspn(buf, "\n")] = 0;
    } else {
        if (feof(stdin)) {
            g_debug ("End of file reached\n");
            exit (0);
        } else if (ferror(stdin)) {
            g_printerr ("Input error\n");
        }
        return NULL;
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
                json_object_set_string_member (log_object, "pam_conv_mess", message->msg);
                print_json (root);

                return PAM_SUCCESS;
            case PAM_ERROR_MSG:
                json_object_set_string_member (log_object, "pam_conv_mess", message->msg);
                print_json (root);

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
    set_member_pam (object, retval, pamh);
    return pam_end (pamh, retval);
}

int
setup_pam (gchar* user_name, JsonNode *node_pam, JsonNode *node_pam_conv)
{
    g_assert (user_name != NULL);

    pam_handle_t *pamh = NULL;
    int retval;
    JsonObject *object_pam = json_node_get_object (node_pam);
    struct pam_conv conv = { non_interactive_conv, node_pam_conv};

    retval = pam_start (PASSWD_SERVICE, user_name, &conv, &pamh);

    if (retval != PAM_SUCCESS) {
        wrapped_pam_end (pamh, retval, object_pam);
        print_json (node_pam);
        return retval;
    }

    retval = pam_chauthtok (pamh, 0);

    if (retval != PAM_SUCCESS) {
        wrapped_pam_end (pamh, retval, object_pam);
        print_json (node_pam);
        return retval;
    }

    wrapped_pam_end (pamh, PAM_SUCCESS, object_pam);

    if (retval != PAM_SUCCESS) {
        wrapped_pam_end (pamh, retval, object_pam);
        print_json (node_pam);
        return retval;
    }

    print_json (node_pam);

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
    
    JsonNode *root_json_pam_conv = init_json_node_pam_conv ();
    JsonNode *root_json_pam = init_json_node_pam ();
    JsonObject *object_json_pam_conv = json_node_get_object (root_json_pam_conv);
    JsonObject *object_json_pam = json_node_get_object (root_json_pam);

    g_assert (object_json_pam_conv != NULL);
    g_assert (object_json_pam != NULL);

    gchar *username = get_username ();
    if (!username) {
        json_object_set_string_member (object_json_pam, "pam_status_en", "Unable to retrieve username");
        print_json (root_json_pam);

        clear_json_object (object_json_pam);
        clear_json_object (object_json_pam_conv);
        json_node_free (root_json_pam);
        json_node_free (root_json_pam_conv);

        return 1;
    }

    res = setup_pam (username, root_json_pam, root_json_pam_conv);

    clear_json_object (object_json_pam);
    clear_json_object (object_json_pam_conv);
    json_node_free (root_json_pam);
    json_node_free (root_json_pam_conv);

    return 0;
}
