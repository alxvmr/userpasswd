#ifndef USERPASSWDAPP_H
#define USERPASSWDAPP_H
#include <adwaita.h>
#include <locale.h>
#include <libintl.h>

#define _(STRING) gettext(STRING)

G_BEGIN_DECLS

#define USERPASSWD_TYPE_APP (userpasswd_app_get_type ())
G_DECLARE_FINAL_TYPE (UserpasswdApp, userpasswd_app, USERPASSWD, APP, AdwApplication)

UserpasswdApp* userpasswd_app_new (const char *application_id, GApplicationFlags flags);

G_END_DECLS

#endif