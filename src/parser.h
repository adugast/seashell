#ifndef __PARSER_H__
#define __PARSER_H__


#include "list.h"


typedef struct parser {
    char *str;
    struct list_head node;          // node to link the list
    struct list_head child_head;    // point to the child head
} parser_t;


typedef void (*parser_cb_t)(struct list_head *root, char *token);


void parser_init(struct parser *p, char *buffer);
void parser_deinit(struct parser *p);
void parser_dump(struct parser *p);


#endif /* __PARSER_H__ */

