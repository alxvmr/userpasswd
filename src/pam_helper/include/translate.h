#ifndef TRANSLATE_H
#define TRANSLATE_H
#include <string.h>
#include <security/pam_appl.h>

const char* get_translate_by_pam_retval (pam_handle_t *pamh, int retval);
#endif