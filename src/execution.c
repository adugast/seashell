#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wordexp.h>

#include "parser.h"
#include "builtin.h"


static void execute_expanded_cmd(parser_t *p)
{
    wordexp_t w;
    int flag = WRDE_NOCMD;
    unsigned int i = 0;

    parser_t *pos;
    list_for_each_entry(pos, &(p->child_head), node) {
        wordexp(pos->str, &w, (i == 0) ? flag : WRDE_APPEND | flag);
        /*
         * Ternary operator used to add the flag WRDE_APPEND only at
         * second and later calls to wordexp function.
         *
         * WRDE_NOCMD: Prevent command substitution -> Security to avoid
         * commands like ~/$(rm -rf ~/)
         */
        i++;
    }

    execvp(w.we_wordv[0], &w.we_wordv[0]);

    wordfree(&w);

    return;
}


static int pipeline(parser_t *p)
{
    parser_t *pos;
    size_t s = list_length(&(p->child_head));
    list_for_each_entry(pos, &(p->child_head), node) {

        if (--s == 0)
            break;

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return -1;
        }

        switch (fork()) {
            case -1:
                perror("fork");
                return -1;
            case 0:
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                execute_expanded_cmd(pos);
                fprintf(stderr, "seashell: %s: command not found\n", pos->str);
                abort();
                break;
            default:
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
                break;
        }
    }

    execute_expanded_cmd(pos);
    fprintf(stderr, "seashell: %s: command not found\n", pos->str);
    abort();

    return 0;
}


/* execute the cmdline, it can be a simple command or a pipeline */
static int parser_execute_cmdline(parser_t *p)
{
    parser_t *pos;
    list_for_each_entry(pos, &(p->child_head), node) {
        pid_t pid;
        switch ((pid = fork())) {
            case -1: perror("fork"); return -1;
            case 0: pipeline(pos); break;
            default:
                    if (waitpid(pid, NULL, 0) == -1) {
                        perror("waitpid");
                        return -1;
                    }
        }
    }

    return 0;
}


/* receive the whole command buffer */
/* i.e "ls -l -a | grep a | wc -l ; ls *; echo 1" */
int execution(const char *buffer)
{
    char *buffer_save = strdup(buffer);

    parser_t *p = calloc(1, sizeof(parser_t));
    if (!p)
        return -1;

    parser_init(p, buffer_save);
    parser_execute_cmdline(p);
    parser_deinit(p);

    free(buffer_save);

    return 0;
}

