#ifndef USERPASSWDAPP_H
#define USERPASSWDAPP_H
#ifdef USE_ADWAITA
    #include <adwaita.h>
#endif
#include <locale.h>
#include <libintl.h>

#define _(STRING) gettext(STRING)
G_BEGIN_DECLS

#define USERPASSWD_TYPE_APP (userpasswd_app_get_type ())
#ifdef USE_ADWAITA
    G_DECLARE_FINAL_TYPE (UserpasswdApp, userpasswd_app, USERPASSWD, APP, AdwApplication)
#else
    G_DECLARE_FINAL_TYPE (UserpasswdApp, userpasswd_app, USERPASSWD, APP, GtkApplication)
#endif

UserpasswdApp* userpasswd_app_new (const char *application_id, GApplicationFlags flags);

G_END_DECLS

#endif