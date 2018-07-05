#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"


static void foreach_token_parser(char *str, const char *delim, parser_cb_t cb, void *ctx)
{
    char *saveptr;
    // perhaps do own version of strtok to handle also ;; is command line
    char *token = strtok_r(str, delim, &saveptr);
    while (token != NULL) {
        cb(token, ctx);
        token = strtok_r(NULL, delim, &saveptr);
    }
}


static void token_parser_cb(char *token, void *ctx)
{
    struct list_head *token_list = (struct list_head *)ctx;

    struct token *new = calloc(1, sizeof(struct token));
    new->token_str = strdup(token);

    list_add_tail(&(new->node), token_list);
}


static void cmd_parser_cb(char *cmd, void *ctx)
{
    struct list_head *cmd_list = (struct list_head *)ctx;

    struct cmd *new = calloc(1, sizeof(struct cmd));
    init_list(&(new->token_list));

    foreach_token_parser(cmd, " ", &token_parser_cb, &(new->token_list));

    list_add_tail(&(new->node), cmd_list);
}


static void cmd_line_parser_cb(char *cmd_line, void *ctx)
{
    struct list_head *cmd_line_list = (struct list_head *)ctx;

    struct cmd_line *new = calloc(1, sizeof(struct cmd_line));
    init_list(&(new->cmd_list));

    foreach_token_parser(cmd_line, "|", &cmd_parser_cb, &(new->cmd_list));

    list_add_tail(&(new->node), cmd_line_list);
}


int init_parser(char *buffer, struct parser *p)
{
    foreach_token_parser(buffer, ";", &cmd_line_parser_cb, &(p->cmd_line_list));
    return 0;
}


void deinit_parser(struct parser *p)
{
    struct cmd_line *cl;
    list_for_each_entry(cl, &(p->cmd_line_list), node) {
        struct cmd *c;
        list_for_each_entry(c, &(cl->cmd_list), node) {
            struct token *t;
            list_for_each_entry(t, &(c->token_list), node) {
                free(t->token_str);
                free(t);
            }
            free(c);
        }
        free(cl);
    }
    free(p);
}


void dump_parser(struct parser *p)
{
    struct cmd_line *cl;
    struct cmd *c;
    struct token *t;

    list_for_each_entry(cl, &(p->cmd_line_list), node) {
        printf("new cmd line\n");
        list_for_each_entry(c, &(cl->cmd_list), node) {
            printf("new cmd\n");
            list_for_each_entry(t, &(c->token_list), node) {
                printf("token[%s]\n", t->token_str);
            }
        }
    }
}

