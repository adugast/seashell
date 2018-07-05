#ifndef __PARSER_H__
#define __PARSER_H__


#include "list.h"


typedef struct token {
    char *token_str;
    struct list_head node;
} token_t;


typedef struct cmd {
    struct list_head token_list;
    struct list_head node;
    unsigned int nb_token;
} cmd_t;


typedef struct cmd_line {
    struct list_head cmd_list;
    struct list_head node;
    unsigned int nb_cmd;
} cmd_line_t;


typedef struct parser {
    struct list_head cmd_line_list;
} parser_t;


typedef void (*parser_cb_t)(char *token, void *ctx);


int init_parser(char *buffer, struct parser *p);
void deinit_parser(struct parser *p);
void dump_parser(struct parser *p);


#endif /* __PARSER_H__ */

