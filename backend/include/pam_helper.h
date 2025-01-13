#ifndef PAM_HELPER_H
#define PAM_HELPER_H
#include "passwduser.h"

int setup_pam (PasswdUser *passwduser, GError **error);
#endif