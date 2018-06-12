#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <wordexp.h>

#include "parser.h"
#include "builtin.h"

//#define UNIQUEPASTE(x, y)   x##y
//#define UNIQUEPASTE2(x, y)  UNIQUEPASTE(x, y)
//#define UNIQUE              UNIQUEPASTE2(_tmp_, __LINE__)

#define for_each_token(str, delim, token) char *UNIQUE = str; \
        for (token = strtok_r(str, delim, &UNIQUE); token != NULL; token = strtok_r(NULL, delim, &UNIQUE))


/* TODO: Find a way to combine functions "first" and "second" */
static char **second(char *buf)
{
    char **res = NULL;
    char *token = NULL;
    int size = 2;
    int i = 0;

    for_each_token(buf, " ", token) {
        res = realloc(res, size * sizeof(char *));
        res[i] = strdup(token);
        i += 1;
        size += 1;
    }
    res[i] = NULL;

    return res;
}


static char ***first(char *buf)
{
    char ***res = NULL;
    char *token = NULL;
    int size = 2;
    int i = 0;

    for_each_token(buf, "|", token) {
        res = realloc(res, size * sizeof(char **));
        res[i] = second(token);
        i += 1;
        size += 1;
    }
    res[i] = NULL;

    return res;
}


static void free_cmds(char ***cmds)
{
    int i, j;
    for (j = 0; cmds[j] != NULL; j++) {
        for (i = 0; cmds[j][i] != NULL; i++)
            free(cmds[j][i]);
        free(cmds[j]);
    }
    free(cmds);
}


static void execute_expanded_cmd(char *cmd[])
{
    wordexp_t p;

    int flag = WRDE_NOCMD;
    unsigned int i;
    for (i = 0; cmd[i] != NULL; i++)
        wordexp(cmd[i], &p, (i == 0) ? flag : WRDE_APPEND | flag);
    /*
     * Ternary operator used to add the flag WRDE_APPEND only at
     * second and later calls to wordexp function.
     *
     * WRDE_NOCMD: Prevent command substitution -> Security to avoid
     * commands like ~/$(rm -rf ~/)
     */

    execvp(p.we_wordv[0], &p.we_wordv[0]);

    wordfree(&p);

    return;
}


static int pipeline(char **cmds[])
{
    int pos;
    for (pos = 0; cmds[pos + 1] != NULL; pos++) {
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
                execute_expanded_cmd(cmds[pos]);
                fprintf(stderr, "seashell: %s: command not found\n", cmds[pos][0]);
                abort();
                break;
            default:
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
                break;
        }
    }

    execute_expanded_cmd(cmds[pos]);
    fprintf(stderr, "seashell: %s: command not found\n", cmds[pos][0]);
    abort();

    return 0;
}


/* execute the cmdline, it can be a simple command or a pipeline */
static int execute_cmdline(char *cmdline)
{
    char ***cmds = first(cmdline);

    if (cmds[1] == NULL && is_builtin(cmds[0][0]) == 1) {
        builtin_manager(cmds[0]);
        free_cmds(cmds);
        return 0;
    }

    pid_t pid;
    switch ((pid = fork())) {
        case -1:
            perror("fork");
            return -1;
        case 0: // child
            pipeline(cmds);
            break;
        default: // parent
            if (waitpid(pid, NULL, 0) == -1) {
                perror("waitpid");
                return -1;
            }
            break;
    }
    free_cmds(cmds);

    return 0;
}


/* split the buffer into cmdline (delimited by semi-colon) */
static int split_cmdline(char *buffer)
{
    char *cmdline;
    for_each_token(buffer, ";", cmdline)
        execute_cmdline(cmdline);

    return 0;
}


/* receive the whole command buffer */
/* i.e "ls -l -a | grep a | wc -l ; ls *; echo 1" */
int execution(const char *buffer)
{
    char *buffer_save = strdup(buffer);

    /*
    struct parser *p = calloc(1, sizeof(struct parser));
    init_list(&(p->cmd_line_list));
    init_parser(buffer_save, p);
    dump_parser(p);
    deinit_parser(p);
    */

    split_cmdline(buffer_save);

    free(buffer_save);
    return 0;
}

