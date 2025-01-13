#include "../include/translate.h"
#include <glib.h>

typedef struct {
    int code;
    const char *message;
} RetVal;

RetVal retval_table[] = {
    {PAM_SUCCESS, "Успех"},
    {PAM_OPEN_ERR, "Сбой dlopen() при динамической загрузке модуля"},
    {PAM_SYMBOL_ERR, "Символ не найден"},
    {PAM_SERVICE_ERR, "Ошибка в модуле службы"},
    {PAM_SYSTEM_ERR, "Системная ошибка"},
    {PAM_BUF_ERR, "Ошибка буфера памяти"},
    {PAM_PERM_DENIED, "Доступ запрещен"},
    {PAM_AUTH_ERR, "Сбой аутентификации"},
    {PAM_CRED_INSUFFICIENT, "Недостаточно учетных данных для доступа к данным проверки подлинности"},
    {PAM_AUTHINFO_UNAVAIL, "Службе проверки подлинности не удается загрузить сведения аутентификации"},
    {PAM_USER_UNKNOWN, "Пользователь неизвестен базовому модулю аутентификации"},
    {PAM_MAXTRIES, "Превышено максимальное количество попыток аутентификации. Не следует предпринимать повторные попытки"},
    {PAM_NEW_AUTHTOK_REQD, "Требуется новый токен аутентификации. Обычно это связано с требованием политики безопасности, так как пароль не установлен или устарел"},
    {PAM_ACCT_EXPIRED, "Истек срок действия учетной записи пользователя"},
    {PAM_SESSION_ERR, "Не удается создать/удалить запись для указанной сессии"},
    {PAM_CRED_UNAVAIL, "Базовая служба аутентификации не может загрузить учетные данные пользователя"},
    {PAM_CRED_EXPIRED, "Истек срок действия учетных данных пользователя"},
    {PAM_CRED_ERR, "Сбой при настройке учетных данных пользователя"},
    {PAM_NO_MODULE_DATA, "Отсутствуют данные, специфичные для модуля"},
    {PAM_CONV_ERR, "Ошибка в разговоре (pam_conv)"},
    {PAM_AUTHTOK_ERR, "Ошибка при манипуляции с токеном аутентификации"},
    {PAM_AUTHTOK_RECOVERY_ERR, "Не удается восстановить сведения аутентификации"},
    {PAM_AUTHTOK_LOCK_BUSY, "Доступ к токену аутентификации временно заблокирован"},
    {PAM_AUTHTOK_DISABLE_AGING, "Отключено истечение срока действия токена аутентификации"},
    {PAM_TRY_AGAIN, "Службе паролей не удалось выполнить предварительную проверку"},  // Предварительная проверка службой паролей
    {PAM_IGNORE, "Игнорирование базового модуля учетной записи независимо от того, является ли контрольный флаг требуемым, необязательным или достаточным"},
    {PAM_ABORT, "Критическая ошибка. Операция прервана"},
    {PAM_AUTHTOK_EXPIRED, "Токен аутентификации пользователя истек"},
    {PAM_MODULE_UNKNOWN, "Модуль не найден или не может быть загружен"},
    {PAM_BAD_ITEM, "Передан неверный параметр в pam_*_item()"},
    {PAM_CONV_AGAIN, "Ошибка в контексте взаимодействия с пользователем (pam_conv)"}, //?
    {PAM_INCOMPLETE, "Операция не заершена, требуется дополнительная информация"}, //?
};

const gchar* get_translate_by_pam_retval (pam_handle_t *pamh, int retval) {
    const gchar* res = NULL;
    size_t table_size = sizeof(retval_table) / sizeof(retval_table[0]);
    for (size_t i = 0; i < table_size; i++) {
        if (retval_table[i].code == retval) {
            return retval_table[i].message;
        }
    }

    res = pam_strerror(pamh, retval);
    if (res == NULL) {
        return "Unknown PAM error";
    }

    return res;
}