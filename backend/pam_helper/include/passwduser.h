#ifndef PASSWDUSER_H
#define PASSWDUSER_H
#include <glib-object.h>
#include <gio/gio.h>

#define dbg(fmtstr, args...) \
  (g_print(PROGNAME ":%s: " fmtstr "\n", __func__, ##args))

G_BEGIN_DECLS

#define PASSWD_TYPE_USER (passwd_user_get_type())
G_DECLARE_FINAL_TYPE (PasswdUser, passwd_user, PASSWD, USER, GObject)

struct _PasswdUser {
    GObjectClass parent;
    gchar *user_name;
    gchar *old_passwd;
    gchar *new_passwd;
};

typedef struct _PasswdUser PasswdUser;

PasswdUser *passwd_user_new (gchar *user_name, gchar *old_passwd, gchar *new_passwd);
gchar **passwd_user_get_fields_for_pam (PasswdUser *self);
static void passwd_user_finalize (GObject *obj);

G_END_DECLS

#endif