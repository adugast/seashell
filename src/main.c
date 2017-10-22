#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdarg.h>

#include <curses.h>
#include <term.h>

#include <signal.h>

#include "string_fct.h"

#define CHAR_BS     0x08 // back space '\b'
#define CHAR_TAB    0x09 // horizontal tab '\t'
#define CHAR_NL     0x0A // new line '\n'
#define CHAR_CR     0x0D // carriage return '\r'
#define CHAR_ESC    0x1B // escape
#define CHAR_SB     0x5B // [
#define CHAR_DEL    0x7F // delete

#define CHAR_A      0x41 // [ A ETX
#define CHAR_B      0x42 // [ B ETX
#define CHAR_C      0x43 // [ C ETX
#define CHAR_D      0x44 // [ D ETX

#define BUFFER_LEN  512
#define PROMPT_LEN  256


struct history {
    char entry[BUFFER_LEN];
};

struct shell {
    char prompt[PROMPT_LEN];
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

    term.c_oflag |= (OPOST);    // enable implementation-defined processing
    term.c_lflag |= (ISIG);     // active signals generation

    return tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/////////////////////////////////////////////////////////////////////

static int print_fct(const char *fmt, ...)
{
    char str[256] = {0};
    va_list arg;

    va_start(arg, fmt);
    vsnprintf(str, 256, fmt, arg);
    va_end(arg);

    return write(1, str, strlen(str));
}

static void print_prompt(const char *prompt)
{
    print_fct(prompt);
}

static void print_line(const struct shell *ctx, const char *line)
{
    char out[256] = "\x1b[0K";

    print_fct("\r%s\r", out);
    print_fct("%s%s", ctx->prompt, line);
}
/*
static void print_cmd(const char *line)
{
    print_fct("%s", line);
}
*/
/////////////////////////////////////////////////////////////////////
/*
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


static int execution(const char *buffer)
{
    pid_t pid = -1;
    //int status = -1;
    char res[256] = {0};
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

            memset(res, 0, 256);
            size = read(pipefd[0], res, 256);
            if (size == -1) {
                return -1;
            } else {
                print_cmd(res);
            }
        }
    }
       if (waitpid(pid, &status, 0) == -1) {

       }

    return 0;
}
*/
/////////////////////////////////////////////////////////////////////

static int read_keyboard(struct shell *ctx, const char keycode[3], unsigned int len)
{
    // DEBUG
    // printf("[%d][%d][%d]\n", keycode[1], keycode[2], keycode[3]);
    static char buffer[BUFFER_LEN] = {0};
    static unsigned int offset = 0;
    unsigned int i = 0;

    if (offset+len >= BUFFER_LEN) {
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
            case CHAR_CR:

                if (buffer[offset+1] == CHAR_NL || buffer[offset+1] == 0x00) {
                    buffer[offset] = '\0';
                }

                if (strcmp("exit", buffer) == 0) {
                    ctx->exit = 1;
                    write(1, "\r\n", 2);
                    goto exit_read_keyboard;
                }

                write(1, "\r\n", 2);

                // TODO implement clean command execution
                // execution(buffer);

                memset(buffer, 0, offset);
                offset = 0;
                print_prompt(ctx->prompt);

                goto exit_read_keyboard;

                break;
            case CHAR_ESC:
                //TODO implement history management
                // get_arrow_key();

                offset = 0;
                memset(buffer, 0, BUFFER_LEN);
                print_line(ctx, buffer);
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

// ctrl + c handler for clean up
// check if there is better solution than global variable
// keep global_save to clean the context in case of SIGINT
static struct shell *global_save;
static int terminate(struct shell *ctx);
static void signal_handler(__attribute__((unused)) int signum)
{
    write(1, "\r\n", 2);
    terminate(global_save);
    _exit(EXIT_SUCCESS);
}

/////////////////////////////////////////////////////////////////////

static int initialize(struct shell **ctx)
{
    int ret = -1;

    // set signal handler
    struct sigaction action = {0};

    action.sa_handler = signal_handler;

    ret = sigaction(SIGINT, &action, NULL);
    if (ret == -1) {
        perror("sigaction()");
        return -1;
    }

    // create new context
    struct shell *new = NULL;

    new = calloc(1, sizeof(struct shell));
    if (new == NULL) {
        return -1;
    }

    memcpy(new->prompt, "Abs0l3m>", PROMPT_LEN);

    ret = get_terminal(&(new->saved_cfg));
    if (ret == -1) {
        return -1;
    }

    ret = init_terminal();
    if (ret == -1) {
        return -1;
    }

    *ctx = new;
    // keep global_save to clean the context in case of SIGINT
    global_save = new;

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

int entry()
{
    int ret = -1;
    struct shell *ctx = NULL;

    ret = initialize(&ctx);
    if (ret == -1) {
        fprintf(stderr, "initialize:failed");
        return -1;
    }

    ret = interpret(ctx);
    if (ret == -1) {
        fprintf(stderr, "interpret:failed");
        return -1;
    }

    ret = terminate(ctx);
    if (ret == -1) {
        fprintf(stderr, "terminate:failed");
        return -1;
    }

    return -1;
}

int main()
{
    entry();
    return 0;
}
