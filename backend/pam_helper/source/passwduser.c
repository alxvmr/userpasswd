#include "../include/passwduser.h"

G_DEFINE_TYPE (PasswdUser, passwd_user, G_TYPE_OBJECT)

static void
passwd_user_class_init (PasswdUserClass *self)
{
    GObjectClass *obj_class = G_OBJECT_CLASS (self);
    obj_class->finalize = passwd_user_finalize;
}

static void
passwd_user_init (PasswdUser *self)
{}

PasswdUser*
passwd_user_new (gchar *user_name, gchar *old_passwd, gchar *new_passwd)
{
    PasswdUser *self = PASSWD_USER (g_object_new (PASSWD_TYPE_USER, NULL));
    self->user_name = g_strdup(user_name);
    self->old_passwd = g_strdup(old_passwd);
    self->new_passwd = g_strdup(new_passwd);

    return self;
}

static void
passwd_user_finalize (GObject *obj)
{
    PasswdUser *self = PASSWD_USER (obj);

    g_free (self->user_name);
    g_free (self->old_passwd);
    g_free (self->new_passwd);

    G_OBJECT_CLASS (passwd_user_parent_class) -> finalize (obj);
}