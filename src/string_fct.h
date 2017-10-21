#ifndef __STRING_FCT_H__
#define __STRING_FCT_H__

char *mystrdup(const char *s);
char *concat(const char str1, const char *str2);
char *concats(unsigned int count, ...);
size_t count_word(const char *str);
char **str_to_wordtab(const char *str, const char *delim);

#endif /* __STRING_FCT_H__ */
