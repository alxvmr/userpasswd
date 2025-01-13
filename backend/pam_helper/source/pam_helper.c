#include <security/pam_appl.h>
#include "../include/passwduser.h"
#include "../include/pam_helper_json.h"
#include <stdio.h>
#include <pwd.h>

#define	PASSWD_SERVICE	"passwd"
#define PAM_OLDPASS 0
#define PAM_NEWPASS 1
#define PAM_SKIPASS 2

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
        return PAM_NEWPASS;
    if (!strcmp(msg, "Re-type new password: "))
        return PAM_NEWPASS;

    return PAM_SKIPASS;
}

int
non_interactive_conv (int                        num_msg,
                      const struct pam_message **msgm,
                      struct pam_response      **response,
                      void                      *appdata_ptr)
{
    PasswdUser *user = (PasswdUser *) appdata_ptr;
    g_assert (user != NULL);

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
                    case PAM_OLDPASS:
                        answ = g_strdup (user->old_passwd);
                        break;
                    case PAM_NEWPASS:
                        answ = g_strdup (user->new_passwd);
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
setup_pam (PasswdUser *user, JsonNode *root)
{
    g_assert (user != NULL);

    pam_handle_t *pamh = NULL;
    struct pam_conv conv = { non_interactive_conv, user };
    int retval;
    JsonObject *object = json_node_get_object (root);

    retval = pam_start (PASSWD_SERVICE, user->user_name, &conv, &pamh);
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

    PasswdUser *user = NULL;
    int res;
    
    JsonNode *root = init_json_node ();
    JsonObject *object = json_node_get_object(root);

    g_assert (object != NULL);

    if (argc != 3) {
        json_object_set_string_member(object, "main_error", "Not enough arguments");
        print_json (root);

        clear_json_object (object);
        json_node_free (root);

        return 1;
    }

    gchar *username = get_username ();
    if (!username) {
        json_object_set_string_member(object, "main_error", "Unable to retrieve username");
        print_json (root);

        clear_json_object (object);
        json_node_free (root);

        return 1;
    }

    user = passwd_user_new (username, argv[1], argv[2]);
    json_object_set_string_member(object, "user_name", user->user_name);

    res = setup_pam (user, root);

    clear_json_object (object);
    json_node_free (root);

    g_free(CONV_ERROR);

    g_object_unref (user);
    return 0;
}