#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>


/* Function to clean the string given as parameter
 * It removes begining, trailing and inter-words white-
 * space characters replacing it by only one space ' '.
 * white-space characters cleaned: \f, \n, \r, \t, \v
 */
void clean_str(char *str)
{
    int flag = 1;
    char *r_ptr, *w_ptr;

    for (r_ptr = w_ptr = str; *r_ptr != '\0'; r_ptr++) {
        if (isspace(*r_ptr) == 0 || flag == 0)
            isspace(*r_ptr) != 0 ? (*w_ptr++ = ' ') : (*w_ptr++ = *r_ptr);
        flag = isspace(*r_ptr);
    }

    flag ? (*--w_ptr = '\0') : (*r_ptr = '\0');
}


/* concat strings with count args trick
 * -> meaning: limited by the count arg macro
 */
/*
#define _COUNT_ARGS(_9,_8,_7,_6,_5,_4,_3,_2,_1,_0, nbr,...) nbr
#define COUNT_ARGS(...) \
    _COUNT_ARGS(,##__VA_ARGS__,9,8,7,6,5,4,3,2,1,0)

#define concats(...) \
    _concats(COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)


char *_concats(int count, ...)
{
    va_list ap;
    char *res = NULL;
    int size = 0;
    int i = 0;

    char *test[count];

    va_start(ap, count);
    for (i = 0; i < count; i++) {
        test[i] = va_arg(ap, char *);
        size += strlen(test[i]);
    }
    va_end(ap);

    res = calloc(size + 1, sizeof(char));
    for (i = 0; i < count; i++)
        strcat(res, test[i]);

    return res;
}
*/

/* concat strings by adding "\0" as the last arguments
 * -> meaning:  const: loop over varargs 2 times (count buffer and concat)
 */
#define TAG_END "\0"
#define concats2(...) \
    _concats2(-1, ##__VA_ARGS__, TAG_END)


char *_concats2(int last, ...)
{
    char *token = NULL;
    va_list ap;
    size_t size = 0;

    /* first loop count the size of the buffer */
    va_start(ap, last);
    token = va_arg(ap, char *);
    while (strcmp(token, TAG_END) != 0) {
        size += strlen(token);
        token = va_arg(ap, char *);
    }
    va_end(ap);

    char *res = NULL;
    res = calloc(size + 1, sizeof(char));
    if (res == NULL)
        return NULL;

    /* second loop concat the strings */
    va_start(ap, last);
    token = va_arg(ap, char *);
    while (strcmp(token, TAG_END) != 0) {
        strcat(res, token);
        token = va_arg(ap, char *);
    }
    va_end(ap);

    return res;
}


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
