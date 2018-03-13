#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


/* TODO: Find a way to combine functions "first" and "second" */
static char **second(char *buf)
{
    char **res = NULL;
    char *token = NULL;
    char *saveptr = NULL;
    int size = 2;
    int i = 0;

    token = strtok_r(buf, " ", &saveptr);
    while (token != NULL) {

        res = realloc(res, size * sizeof(char *));
        res[i] = strdup(token);

        i += 1;
        size += 1;

        token = strtok_r(NULL, " ", &saveptr);
    }
    res[i] = NULL;

    return res;
}


static char ***first(char *buf)
{
    char ***res = NULL;
    char *token = NULL;
    char *saveptr = NULL;
    int size = 2;
    int i = 0;

    token = strtok_r(buf, "|", &saveptr);
    while (token != NULL) {

        res = realloc(res, size * sizeof(char **));
        res[i] = second(token);

        i += 1;
        size += 1;

        token = strtok_r(NULL, "|", &saveptr);
    }
    res[i] = NULL;

    return res;
}


static void free_cmds(char ***cmds)
{
    int i = 0;
    int j = 0;

    while (cmds[j] != NULL) {
        while (cmds[j][i] != NULL) {
            free(cmds[j][i]);
            i++;
        }
        free(cmds[j]);
        i = 0;
        j++;
    }
    free(cmds);

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
                execvp(cmds[pos][0], cmds[pos]);
                fprintf(stderr, "seashell: %s: command not found\n", cmds[pos][0]);
                abort();
                break;
            default:
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
                break;
        }
    }

    execvp(cmds[pos][0], cmds[pos]);
    fprintf(stderr, "seashell: %s: command not found\n", cmds[pos][0]);
    abort();

    return 0;
}


/* execute the cmdline, it can be a simple command or a pipeline */
int execute_cmdline(char *cmdline)
{
    char ***cmds = first(cmdline);

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
int split_cmdline(char *buffer)
{
    char *cmdline = NULL;
    for (cmdline = strtok(buffer, ";"); cmdline != NULL; cmdline = strtok(NULL, ";"))
        execute_cmdline(cmdline);
    return 0;
}


/* receive the whole command buffer */
/* i.e "ls -l -a | grep a | wc -l ; echo 1 ; ls" */
int execution(const char *buffer)
{
    char *buffer_save = strdup(buffer);
    split_cmdline(buffer_save);
    free(buffer_save);
    return 0;
}
