#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

char *mystrdup(const char *s)
{
    char *s_copy = NULL;
    size_t len = 0;

    if (s == NULL) {
        return NULL;
    }

    len = strlen(s) + 1;

    s_copy = (char *)malloc(len);
    if (s_copy == NULL) {
        return NULL;
    }

    return s_copy ? memcpy(s_copy, s, len) : NULL;
}

char *concat(const char *str1, const char *str2)
{
    char *res = NULL;
    size_t len = 0;
    size_t len1 = 0;

    if (str1 == NULL || str2 == NULL) {
        return NULL;
    }

    len = strlen(str1) + strlen(str2) + 1;
    len1 = strlen(str1);

    res = malloc(len);

    if (res == NULL) {
        return NULL;
    }

    memcpy(res, str1, len1);
    memcpy(res + len1, str2, strlen(str2) + 1);

    return res;
}

char *concats(unsigned int count, ...)
{
    char *merged_str = NULL;
    char *tmp = NULL;
    va_list ap;
    size_t len_max = 0;
    size_t len = 0;
    unsigned int i = 0;
    unsigned int pos = 0;

    va_start(ap, count);
    for (i = 0; i < count; i++) {
        tmp = va_arg(ap, char *);
        if (tmp == NULL) {
            return NULL;
        }
        len_max += strlen(tmp);
    }
    va_end(ap);

    merged_str = malloc(len_max + 1);
    if (merged_str == NULL) {
        return NULL;
    }

    va_start(ap, count);
    for (i = 0; i < count; i++) {
        tmp = va_arg(ap, char *);
        if (tmp == NULL) {
            return NULL;
        }
        len = strlen(tmp);
        memcpy(merged_str + pos, tmp, len);
        pos += len;
    }
    va_end(ap);

    merged_str[len_max] = '\0';

    return merged_str;
}

int is_alpha(char c)
{
    if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c == '-') ||
            (c == '|')) {
        return 1;
    } else {
        return 0;
    }
}

size_t count_word(const char *str, const char *delim)
{
    size_t len = 0;
    int i = 0;
    unsigned int flag = 0;

    if (str == NULL) {
        return 0;
    }

    while (str[i] != '\0') {
        if (is_alpha(str[i]) == 1) {
            flag = 1;
            if (str[i + 1] == '\0') {
                len += 1;
            }
        } else if (str[i] == delim[0] && flag == 1) {
            len += 1;
            flag = 0;
        }
        i++;
    }

    return len;
}

char **str_to_wordtab(const char *str, const char *delim)
{
    char **tab = NULL;
    char *ptr = NULL;
    char *save_ptr = NULL;
    char *token = NULL;
    int token_size = -1;
    size_t word_nbr = 0;
    int i = 0;

    if (str == NULL || delim == NULL) {
        return NULL;
    }

    ptr = mystrdup(str);
    if (ptr == NULL) {
        return NULL;
    }
    save_ptr = ptr;

    word_nbr = count_word(save_ptr, delim) + 1;

    tab = (char **)malloc(word_nbr * sizeof(char *));
    if (tab == NULL) {
        return NULL;
    }

    while ((token = strtok_r(save_ptr, delim, &save_ptr)) != NULL) {
        token_size = strlen(token) + 1;
        tab[i] = malloc(token_size * sizeof(char));
        memcpy(tab[i], token, token_size);
        i++;
    }
    tab[i] = NULL;

    free(ptr);

    return tab;
}
