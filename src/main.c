#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdarg.h>

#include <curses.h>
#include <term.h>

#include <signal.h>

#include "list.h"
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

// TODO Remove duplicate for last commands in history
// TODO Implement cursor moving, left, right arrow keys
// TODO Implement autocompletion with tabs
// TODO Add execution part
// TODO Redirection
// TODO Multi-Pipes
// TODO Configuration based on config file (prompt, colors, ...)
// TODO Add correct arguments parsing get_opt_long (help, path config file, ...)
// TODO ...

/*
int tputs_cursor(const unsigned int pos_x, const unsigned int pos_y)
{
    int error = -1;

    error = tputs(tgoto(tgetstr("cm", NULL), pos_x, pos_y), 1, putchar);
    if (error == -1) {
        return -1;
    }

    error = fflush(stdout);
    if (error == EOF) {
        return EOF;
    }

    return 0;
}
*/


struct history {
    char entry[BUFFER_LEN];
    struct list head;
};

struct shell {
    char prompt[PROMPT_LEN];
    struct termios saved_cfg;
    int history_index;
    struct history *hist;
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

static void get_history_entry(struct shell *ctx, char *buffer)
{
    struct list *nodep = NULL;
    struct history *tmp = NULL;
    int i = 0;

    for_each(&(ctx->hist->head), nodep) {
        if (i == ctx->history_index) {
            tmp = container_of(nodep, struct history, head);
            strncpy(buffer, tmp->entry, BUFFER_LEN);
        }
        i++;
    }
}

static int add_history_entry(struct shell *ctx, const char *buffer)
{
    struct history *node = calloc(1, sizeof(struct history));
    if (node == NULL) {
        return -1;
    }

    strncpy(node->entry, buffer, BUFFER_LEN);

    list_add_head(&(ctx->hist->head), &(node->head));

    return 0;
}

/////////////////////////////////////////////////////////////////////

static int read_arrow_key(struct shell *ctx, const char c)
{
    switch (c) {
        case CHAR_A: // arrow up
            //print_line(ctx, "arrow_up\n");
            if (ctx->history_index < (int)(list_length(&(ctx->hist->head)) - 1))
                ctx->history_index += 1;
            break;
        case CHAR_B: // arrow down
            //print_line(ctx, "arrow_down\n");
            if (ctx->history_index >= 0)
                ctx->history_index -= 1;
            break;
        case CHAR_C: // arrow right
            //print_line(ctx, "arrow_right\n");
            tputs(tgetstr("cl", NULL), 1, putchar);

            break;
        case CHAR_D: // arrow left
            print_line(ctx, "arrow_left\n");
            break;
    }

    return 0;
}

static int read_keyboard(struct shell *ctx, const char keycode[3], unsigned int len)
{
    // DEBUG
    // printf("[%d][%d][%d]\n", keycode[0], keycode[1], keycode[2]);
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

                if (strcmp(buffer, "\0") != 0)
                    add_history_entry(ctx, buffer);
                // TODO implement clean command execution
                // execution(buffer);

                memset(buffer, 0, offset);
                offset = 0;
                print_prompt(ctx->prompt);
                ctx->history_index = -1;

                goto exit_read_keyboard;

                break;
            case CHAR_ESC:
                //TODO implement history management
                if (buffer[offset+1] == CHAR_SB) {

                    read_arrow_key(ctx, buffer[offset+2]);

                    if (ctx->history_index == -1) {
                        offset = 0;
                        memset(buffer, 0, BUFFER_LEN);
                    } else {
                        get_history_entry(ctx, buffer);
                        offset = strlen(buffer);
                    }
                } else {
                    offset = 0;
                    memset(buffer, 0, BUFFER_LEN);
                    print_line(ctx, buffer);
                    ctx->history_index = -1;
                }

                print_line(ctx, buffer);

                goto exit_read_keyboard;
            case CHAR_TAB:;
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
    write(1, "^C\r\n", 4);
    terminate(global_save);
    _exit(EXIT_SUCCESS);
}

/////////////////////////////////////////////////////////////////////

static int initialize(struct shell **ctx)
{
    int ret = -1;

    // 1) create a new user shell context
    struct shell *new = NULL;

    new = calloc(1, sizeof(struct shell));
    if (new == NULL) {
        return -1;
    }

    // 2) set the user prompt
    memcpy(new->prompt, "Abs0l3m>", PROMPT_LEN);

    // 3) set signal handler for SIGINT
    struct sigaction action = {0};

    action.sa_handler = signal_handler;

    ret = sigaction(SIGINT, &action, NULL);
    if (ret == -1) {
        perror("sigaction()");
        return -1;
    }

    // 4) save the old terminal configuration to be able to reuse it
    // when leaving the seashell
    ret = get_terminal(&(new->saved_cfg));
    if (ret == -1) {
        return -1;
    }

    // 5) initialize raw mode terminal
    ret = init_terminal();
    if (ret == -1) {
        return -1;
    }

    /* 6) initialize history */
    new->history_index = -1;
    new->hist = calloc(1, sizeof(struct history));
    if (new->hist == NULL) {
        fprintf(stderr, "failed to allocate history\n");
        return -1;
    }
    init_list(&(new->hist->head));

    // 7) keep global_save to clean the context in case of SIGINT
    // and retrieve the terminal context
    global_save = new;
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
