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


static void redirect(int oldfd, int newfd)
{
    if (oldfd != newfd) {
        if (dup2(oldfd, newfd) == -1) {
            perror("dup2()");
            exit(0);
        }
        close(oldfd);
    }
}


static void pipeline(char **cmds[], size_t pos, int in_fd)
{
    if (cmds[pos + 1] == NULL) {
        redirect(in_fd, STDIN_FILENO);
        execvp(cmds[pos][0], cmds[pos]);
        fprintf(stderr, "seashell: %s: command not found\n", cmds[pos][0]);
        exit(0);
    } else {
        int pipefd[2] = {0};
        if (pipe(pipefd) == -1) {
            perror("pipe()");
            exit(0);
        }
        switch(fork()) {
            case -1:
                perror("fork()");
                exit(0);
            case 0:
                close(pipefd[0]);
                redirect(in_fd, STDIN_FILENO);
                redirect(pipefd[1], STDOUT_FILENO);
                execvp(cmds[pos][0], cmds[pos]);
                fprintf(stderr, "seashell: %s: command not found\n", cmds[pos][0]);
                exit(0);
            default:
                close(pipefd[1]);
                close(in_fd);
                pipeline(cmds, pos + 1, pipefd[0]);
        }
    }
}


int execution(char *buffer)
{
    int ret = -1;
    pid_t pid = -1;
    char ***cmds = NULL;

    cmds = first(buffer);

    pid = fork();
    if (pid == 0) { // child
        pipeline(cmds, 0, STDIN_FILENO);
    } else { // parent
        ret = waitpid(pid, NULL, 0);
        if (ret == -1) {
            perror("waitpid");
            exit(EXIT_SUCCESS);
        }
    }

    free_cmds(cmds);

    return 0;
}
