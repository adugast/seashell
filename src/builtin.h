#ifndef __BUILTIN_H__
#define __BUILTIN_H__

#ifdef __cplusplus
extern "C" {
#endif


void builtin_manager(char **builtin_args);
int is_builtin(const char *str);


#ifdef __cplusplus
}
#endif

#endif /* __BUILTIN_H__ */

