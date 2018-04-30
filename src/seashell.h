#ifndef __SEASHELL_H__
#define __SEASHELL_H__


#include <stdbool.h>
#include <termios.h>

#include "list.h"


#define BUFFER_LEN 256
#define PROMPT_LEN 256


struct history {
    char entry[BUFFER_LEN];
    struct list node;
};


struct shell {
    char prompt[PROMPT_LEN];
    unsigned int pos_x;
    unsigned int line_size;
    int history_index;
    struct list history_head;
    struct termios saved_cfg;
    bool exit;
};


int option_manager(int argc, char *argv[]);


#endif /* __SEASHELL_H__ */
