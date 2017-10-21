#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdarg.h>

#include <curses.h>
#include <term.h>

#define CHAR_BS     0x08 // back space '\b'
#define CHAR_TAB    0x09 // horizontal tab '\t'
#define CHAR_NL     0x0A // new line '\n'
#define CHAR_CR     0x0D // carriage return '\r'
#define CHAR_ESC    0x1B // escape
#define CHAR_DEL    0x7F // delete

#define BUFFER_SIZE 512


struct shell {
    char prompt[255];
    struct termios saved_cfg;
    bool exit;
};

char **tmp = NULL;

/////////////////////////////////////////////////////////////////////

static int get_terminal(struct termios *term)
{
    return tcgetattr(STDIN_FILENO, term);
}

static int set_terminal(struct termios *term)
{
    return tcsetattr(STDIN_FILENO, TCSANOW, term);
}

static int init_terminal()
{
    struct termios term = {0};

    get_terminal(&term);
    cfmakeraw(&term);
    term.c_oflag |= (OPOST);

    return tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/////////////////////////////////////////////////////////////////////

static int print_fct(const char *fmt, ...)
{
    char str[255] = {0};
    va_list arg;

    va_start(arg, fmt);
    vsnprintf(str, 255, fmt, arg);
    va_end(arg);

    return write(1, str, strlen(str));
}

static void print_prompt(const char *prompt)
{
    print_fct(prompt);
}

static void print_line(struct shell *ctx, const char *line)
{
    char out[255] = "\x1b[0K";

    print_fct("\r%s\r", out);
    print_fct("%s%s", ctx->prompt, line);
}

static void print_cmd(const char *line)
{
    print_fct("%s", line);
}

/////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/wait.h>
#include "string_fct.h"

extern char **environ;

static int exec(const char *command)
{
    char *path = NULL;
    char **tab_path = NULL;
    unsigned int i = 0;
    char **argv = NULL;
    char *filename = NULL;
    int ret = -1;

    argv = str_to_wordtab(command, " ");

    path = getenv("PATH");
    if (path == NULL) {

    }

    tab_path = str_to_wordtab(path, ":");

    i = 0;
    while (tab_path[i] != NULL) {
        filename = concats(3, tab_path[i], "/", argv[0]);

        ret = execve(filename, argv, environ);
        free(filename);

        i++;
    }

    if (ret == -1) {
        exit(0);
    }

    //free

    return 0;
}


static int execution(const char buffer[255])
{
    pid_t pid = -1;
    //int status = -1;
    char res[255] = {0};
    ssize_t size = -1;
    int pipefd[2];

    pipe(pipefd);

    pid = fork();
    if (pid == -1) {
        return -1;
    }

    if (pid == 0) {

        close(pipefd[0]);
        dup2(pipefd[1], 1);

        exec(buffer);
    } else if (pid > 0) {

        close(pipefd[1]);
        while (size != 0) {

            memset(res, 0, 255);
            size = read(pipefd[0], res, 255);
            if (size == -1) {
                return -1;
            } else {
                 print_cmd(res);
            }
        }
    }
/*
    if (waitpid(pid, &status, 0) == -1) {

    }
*/
    return 0;
}

static int read_keyboard(struct shell *ctx, const char keycode[3], unsigned int len)
{
    static char buffer[BUFFER_SIZE] = {0};
    static unsigned int offset = 0;
    unsigned int i = 0;

    if (offset+len >= BUFFER_SIZE) {
        offset = 0;
        buffer[0] = '\0';
        print_prompt(ctx->prompt);
    }

    if (keycode[0] == CHAR_CR && offset == 0) {
        write(STDIN_FILENO, "\r\n", 2);
        print_prompt(ctx->prompt);
        goto exit_read_keyboard;
    }

    memcpy(buffer+offset, keycode, len);

    for (i = 0; i < len; i++) {
        switch (buffer[offset]) {
            case CHAR_BS:
            case CHAR_DEL:
                if (offset > 0) {
                    buffer[offset] = '\0';
                    offset -= 1;
                    buffer[offset] = '\0';
                    print_line(ctx, buffer);
                }
                break;
            case CHAR_CR: // enter

                if (buffer[offset+1] == CHAR_NL || buffer[offset+1] == 0x00) {
                    buffer[offset] = '\0';
                }

                if (strcmp("exit", buffer) == 0) {
                    ctx->exit = 1;
                    write(1, "\r\n", 2);
                    goto exit_read_keyboard;
                }

                write(1, "\r\n", 2);
                execution(buffer);

                memset(buffer, 0, offset);
                offset = 0;
                print_prompt(ctx->prompt);

                goto exit_read_keyboard;

                break;
            case CHAR_ESC:
                    //TODO implement get_arrow_key();
                    if (len > 2 && buffer[offset+1] == '[') {
                        goto exit_read_keyboard;
                    } else {
                        offset = 0;
                        buffer[offset] = '\0';
                        print_line(ctx, buffer);
                    }
                break;
            case CHAR_TAB:
                // TODO implement autocompletion
                break;
            default:
                offset++;
                print_line(ctx, buffer);
                break;
        }
    }

exit_read_keyboard:
    return 0;
}

/////////////////////////////////////////////////////////////////////

static int initialize(struct shell **ctx)
{
    struct shell *new = NULL;
    int ret = -1;

    new = calloc(1, sizeof(struct shell));
    if (new == NULL) {
        return -1;
    }

    strncpy(new->prompt, "Abs0l3m>", 255);

    ret = get_terminal(&(new->saved_cfg));
    if (ret == -1) {
        return -1;
    }

    ret = init_terminal();
    if (ret == -1) {
        return -1;
    }

    *ctx = new;

    return 0;
}


static int interpret(struct shell *ctx)
{
    char keycode[3] = {0};
    ssize_t read_size = 0;

    print_prompt(ctx->prompt);
    while (ctx->exit != 1) {

        memset(keycode, '\0', 3);

        read_size = read(STDIN_FILENO, keycode, 3);
        if (read_size == -1) {
            return -1;
        }

        read_keyboard(ctx, keycode, read_size);
    }

    return 0;
}


static int terminate(struct shell *ctx)
{
    int ret = -1;

    ret = set_terminal(&(ctx->saved_cfg));
    if (ret == -1) {
        return -1;
    }

    free(ctx);

    return 0;
}

/////////////////////////////////////////////////////////////////////

int main()
{
    struct shell *ctx = NULL;

    initialize(&ctx);
    interpret(ctx);
    terminate(ctx);

    return 0;
}
