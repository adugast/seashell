#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"


typedef void (*parser_cb_t)(struct list_head *root, char *token);


static void foreach_token_parser(struct list_head *root, char *cmd, const char *delim, parser_cb_t cb)
{
    char *token, *saveptr;
    for (token = strtok_r(cmd, delim, &saveptr); token; token = strtok_r(NULL, delim, &saveptr))
        cb(root, token);
}


static parser_t *parser_add_branch(struct list_head *root, char *cmd)
{
    parser_t *p = calloc(1, sizeof(parser_t));
    if (!p) {
        printf("parser_add_branch failed\n");
        return NULL;
    }

    init_list(&(p->child_head));
    init_list(&(p->node));
    p->str = strdup(cmd);
    list_add_tail(&(p->node), root);

    return p;
}


static void parser_del_branch(struct parser *p)
{
    free(p->str);
    list_del(&(p->node));
    free(p);
}


static void token_cb(struct list_head *root, char *cmd)
{
    parser_add_branch(root, cmd);
}


static void cmd_cb(struct list_head *root, char *cmd)
{
    parser_t *p = parser_add_branch(root, cmd);
    foreach_token_parser(&(p->child_head), cmd, " ", &token_cb);
}


static void cmd_line_cb(struct list_head *root, char *cmd)
{
    parser_t *p = parser_add_branch(root, cmd);
    foreach_token_parser(&(p->child_head), cmd, "|", &cmd_cb);
}


parser_t *parser_init(const char *buffer)
{
    if (!buffer)
        return NULL;

    parser_t *p = calloc(1, sizeof(parser_t));
    if (!p)
        return NULL;

    char *buffer_save = strdup(buffer);
    if (!buffer_save)
        return NULL;

    init_list(&(p->child_head));
    init_list(&(p->node));
    foreach_token_parser(&(p->child_head), buffer_save, ";", &cmd_line_cb);

    free(buffer_save);

    return p;
}


void parser_deinit(struct parser *p)
{
    parser_t *a, *b, *c, *s1, *s2, *s3;
    list_for_each_entry_safe(a, s1, &(p->child_head), node) {
        list_for_each_entry_safe(b, s2, &(a->child_head), node) {
            list_for_each_entry_safe(c, s3, &(b->child_head), node) {
                parser_del_branch(c);
            }
            parser_del_branch(b);
        }
        parser_del_branch(a);
    }

    free(p);
}


/* parser_dump debug function */
void parser_dump(struct parser *p)
{
    parser_t *a, *b, *c;
    list_for_each_entry(a, &(p->child_head), node) {
        printf("-> cmdline [%s]\n", a->str);
        list_for_each_entry(b, &(a->child_head), node) {
            printf("> cmd [%s]\n", b->str);
            list_for_each_entry(c, &(b->child_head), node) {
                printf("token [%s]\n", c->str);
            }
        }
    }
}

