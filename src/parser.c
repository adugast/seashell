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
    struct list *token_list = (struct list *)ctx;

    struct token *new = calloc(1, sizeof(struct token));
    new->token_str = strdup(token);

    list_add_tail(token_list, &(new->node));
}


static void cmd_parser_cb(char *cmd, void *ctx)
{
    struct list *cmd_list = (struct list *)ctx;

    struct cmd *new = calloc(1, sizeof(struct cmd));
    init_list(&(new->token_list));

    foreach_token_parser(cmd, " ", &token_parser_cb, &(new->token_list));

    list_add_tail(cmd_list, &(new->node));
}


static void cmd_line_parser_cb(char *cmd_line, void *ctx)
{
    struct list *cmd_line_list = (struct list *)ctx;

    struct cmd_line *new = calloc(1, sizeof(struct cmd_line));
    init_list(&(new->cmd_list));

    foreach_token_parser(cmd_line, "|", &cmd_parser_cb, &(new->cmd_list));

    list_add_tail(cmd_line_list, &(new->node));
}


int init_parser(char *buffer, struct parser *p)
{
    foreach_token_parser(buffer, ";", &cmd_line_parser_cb, &(p->cmd_line_list));
    return 0;
}


void deinit_parser(struct parser *p)
{
    struct list *nodep = NULL;

    for_each(&(p->cmd_line_list), nodep) {
        struct cmd_line *cl = container_of(nodep, struct cmd_line, node);
        for_each(&(cl->cmd_list), nodep) {
            struct cmd *c = container_of(nodep, struct cmd, node);
            for_each(&(c->token_list), nodep) {
                struct token *t = container_of(nodep, struct token, node);
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
    struct list *nodep = NULL;

    for_each(&(p->cmd_line_list), nodep) {
        printf("new cmd line\n");
        struct cmd_line *cl = container_of(nodep, struct cmd_line, node);
        for_each(&(cl->cmd_list), nodep) {
            printf("new cmd\n");
            struct cmd *c = container_of(nodep, struct cmd, node);
            for_each(&(c->token_list), nodep) {
                struct token *t = container_of(nodep, struct token, node);
                printf("token[%s]\n", t->token_str);
            }
        }
    }
}
