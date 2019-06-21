#ifndef __SEASHELL_H__
#define __SEASHELL_H__


#include <stdbool.h>
#include <termios.h>

#include "history_manager.h"


#define PROMPT_LEN 256


struct shell {
    char prompt[PROMPT_LEN];
    unsigned int pos_x;
    unsigned int line_size;
    history_t *history_hdl;
    struct termios saved_cfg;
    bool exit;
    char **envp;
};


int start_shell(int argc, char *argv[], char **envp);


#endif /* __SEASHELL_H__ */

