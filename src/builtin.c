#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


struct builtin_s {
    char *name; // builtin command
    int (*fct_ptr)(char **args); // builtin function pointer
};


static int my_cd(char **args);
static int my_exit(char **args);


/* Available builtin list */
struct builtin_s builtin_list[] = {
    { "cd", my_cd }, // when cd find, call my_cd function
    { "exit", my_exit }, // when cd find, call my_cd function
    { NULL, NULL }
};


int is_builtin(const char *str)
{
    unsigned int i;
    for (i = 0; builtin_list[i].name != NULL; i++) {
        if (strcmp(builtin_list[i].name, str) == 0)
            return 1;
    }

    return 0;
}


static int save_oldpwd()
{
    char buffer[255] = {0};
    if (getcwd(buffer, 255) == NULL) {
        fprintf(stderr, "seashell: cd: OLDPWD not set");
        return -1;
    }

    if (setenv("OLDPWD", buffer, 1) == -1) {
        perror("setenv");
        return -1;
    }

    return 0;
}


static int my_cd(char **path)
{
    if (path[1] != NULL && strncmp(path[1], "-", 1) == 0) {
        char *oldpwd = getenv("OLDPWD");
        printf("%s\n", oldpwd);
        chdir(oldpwd);
        return 0;
    }

    /* save old working directory in the environment */
    save_oldpwd();

    if (path[1] == NULL || strncmp(path[1], "~", 1) == 0) {
        char *home = getenv("HOME");
        if (chdir(home) == -1) {
            fprintf(stderr, "seashell: cd: HOME not set\n");
            return -1;
        }
        return 0;
    }

    if (chdir(path[1]) == -1) {
        perror("chdir");
        return -1;
    }

    return 0;
}


static int my_exit(char **args)
{
    args = args;
    printf("builtin_exit\n");
    return 0;
}


void builtin_manager(char **builtin_args)
{
    unsigned int i;
    for (i = 0; builtin_list[i].name != NULL; i++) {
        if (strcmp(builtin_list[i].name, builtin_args[0]) == 0)
            builtin_list[i].fct_ptr(builtin_args);
    }
}

