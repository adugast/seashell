#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "list.h"


LIST_HEAD(path_head);
LIST_HEAD(possibilities_head);


struct entry {
    char *str;
    struct list_head node;
};


static char *ac_getenv(char *var, char **envp)
{
    if (!var || !envp)
        return NULL;

    size_t var_size = strlen(var);
    for (; envp; envp++)
        if (strncmp(var, *envp, var_size) == 0)
            return *envp + var_size + 1;

    return NULL;
}


static void add_entry(const char *entry, struct list_head *head)
{
    if (!entry)
        return;

    struct entry *e = calloc(1, sizeof(struct entry));
    if (!e)
        return;

    e->str = strdup(entry);

    list_add_tail(&(e->node), head);
}


static void del_entry()
{
    struct entry *e, *safe;
    list_for_each_entry_safe(e, safe, &possibilities_head, node) {
        free(e->str);
        list_del(&(e->node));
    }
}


static int already_exists(const char *entry)
{
    struct entry *e;
    list_for_each_entry(e, &possibilities_head, node)
        if (strncmp(entry, e->str, strlen(entry)) == 0)
            return 1;

    return 0;
}


int autocompletion(char *cmd, char **envp)
{
    char *path = ac_getenv("PATH", envp);
    if (!path) {
        printf("ac_getenv: fail\n");
        return -1;
    }

    add_entry(".", &path_head);

    char *path_save = strdup(path);

    char *token, *saveptr;
    for (token = strtok_r(path_save, ":", &saveptr); token; token = strtok_r(NULL, ":", &saveptr)) {
        add_entry(token, &path_head);
    }

    free(path_save);

    struct entry *p;
    list_for_each_entry(p, &path_head, node) {

        DIR *d = opendir(p->str);
        if (!d)
            continue;

        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, cmd, strlen(cmd)) == 0)
                if (already_exists(dir->d_name) == 0)
                    add_entry(dir->d_name, &possibilities_head);
        }

        closedir(d);
    }


    list_for_each_entry(p, &possibilities_head, node)
        printf("%s ", p->str);
    printf("\n");

    if (list_length(&possibilities_head) == 1) {
        struct entry *e = list_entry((&possibilities_head)->next, struct entry, node);
        strncpy(cmd, e->str, strlen(e->str));
    }

    del_entry();

    return 0;
}

