#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>

#include "seashell.h"
#include "execution.h"
#include "string_fct.h"


#define CHAR_BS     0x08 // back space '\b'
#define CHAR_TAB    0x09 // horizontal tab '\t'
#define CHAR_NL     0x0A // new line '\n'
#define CHAR_CR     0x0D // carriage return '\r'
#define CHAR_ESC    0x1B // escape
#define CHAR_SB     0x5B // [
#define CHAR_DELETE 0x7E // delete key
#define CHAR_DEL    0x7F // DEL

#define CHAR_A      0x41 // [ A ETX
#define CHAR_B      0x42 // [ B ETX
#define CHAR_C      0x43 // [ C ETX
#define CHAR_D      0x44 // [ D ETX

#define CHAR_SOH    0x01 // start of heading - ctrl-a
#define CHAR_ENQ    0x05 // enquiery - ctrl-e
#define CHAR_ETB    0x17 // end of trans. blk - ctrl-w

/////////////////////////////////////////////////////////////////////


static int get_terminal(struct termios *term)
{
    return tcgetattr(STDIN_FILENO, term);
}

static int set_terminal(struct termios *term)
{
    return tcsetattr(STDIN_FILENO, TCSADRAIN, term);
}

static int init_terminal()
{
    struct termios term = {0};

    /*!
     * cfmakeraw() sets the terminal to something like the "raw"  mode  of  the  old
     * Version 7 terminal driver: input is available character by character, echoing
     * is disabled, and all special processing of terminal input and output  charac‐
     * ters is disabled.  The terminal attributes are set as follows:
     *
     *      termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
     *                            | INLCR | IGNCR | ICRNL | IXON);
     *      termios_p->c_oflag &= ~OPOST;
     *      termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
     *      termios_p->c_cflag &= ~(CSIZE | PARENB);
     *      termios_p->c_cflag |= CS8;
     */
    cfmakeraw(&term);

    term.c_oflag |= (ONLCR);    // map NL to CR-NL on output
    term.c_oflag |= (OPOST);    // enable implementation-defined processing
    term.c_lflag |= (ISIG);     // active signals generation

    term.c_cc[VINTR] = 3;       // set SIGINT signal

    return set_terminal(&term);
}


/*!
 *      === A I N S I - E S C A P E   S E Q U E N C E S ===
 *
 * man (4) console_codes
 *
 * -- CURSOR MOVEMENTS AND TERMINAL CONFIGURATION --
 *
 * CUB - CUrsor Backward
 * keycode: ESC [ n D
 *
 * CUF - CUrsor Forward
 * keycode: ESC [ n C
 *
 * effect : Moves the cursor n (default 1) cells in the given direction.
 *          If the cursor is already at the edge of the screen, this has no effect.
 *
 *
 * CUP - CUrsor Position
 * keycode: ESC [ n ; m H
 * effect : Moves the cursor to row n, column m.
 *          The values are 1-based, and default to 1 (top left corner) if omitted.
 *
 *
 * EL - Erase Line (default: from cursor to end of line))
 * keycode: ESC [ n K
 * effect: Erases part of the line.
 *         If n is zero (or missing), erase from cursor to the end of line.
 *         If n is one, erase from start of line to cursor.
 *         If n is two, rase whole line.
 *         Cursor position does not change.
 *
 *
 * ED - Erase in Display
 * keycode: ESC [ n J
 * effect: Clears part of the screen.
 *         If n is 0 (or missing), clear from cursor to end of screen.
 *         If n is 1, clear from cursor to beginning of the screen.
 *         If n is 2, clear entire screen (and moves cursor to upper left).
 *         If n is 3, clear entire screen and delete all lines saved
 *         in the scrollback buffer (this feature was added for xterm
 *         and is supported by other terminal applications).
 *
 *
 * DECIM - Set Insert Mode (default off)
 * keycode: ESC [ 4 h
 * effect: Set insert mode to ON
 *
 * UNDECIM - Unset Insert Mode (default on)
 * keycode: ESC [ 4 l
 * effect: Set insert mode to OFF
 *
 *
 * SCP - Save Cursor Position
 * keycode: ESC [ s
 * effect: Saves the cursor position.
 *
 * RCP - Restaure Cursor Position
 * keycode: ESC [ u
 * effect: Restaures the cursor position.
 *
 *
 * http://ascii-table.com/ansi-escape-sequences.php
 *
 */


/* escape sequence for cursor left */
#define CUB "\x1B[1D"
static ssize_t cursor_left(struct shell *ctx)
{
    if (ctx->pos_x > 0) {
        ctx->pos_x -= 1;
        return write(1, CUB, 4);
    }

    return -1;
}

static int cursor_n_left(struct shell *ctx, unsigned int n)
{
    int nb = -1;
    int size = ctx->pos_x - n;

    if (size < 0)
        size = 0;

    if (size >= 0 && n > 0) {
        nb = fprintf(stdout, "\x1B[%dD", n);
        fflush(stdout);
    }

    return nb;
}

/* escape sequence for cursor right */
#define CUF "\x1B[1C"
static ssize_t cursor_right(struct shell *ctx)
{
    if (ctx->pos_x < ctx->line_size) {
        ctx->pos_x += 1;
        return write(1, CUF, 4);
    }

    return -1;
}

/* escape sequence to set the cursor position to 0, 0 */
#define CUP "\x1B[0;0H"
static ssize_t set_cursor_home()
{
    return write(1, CUP, 6);
}

/* escape sequence to clear the screen */
#define CLEAR_SCREEN "\x1B[2J"
static ssize_t clear_screen()
{
    return write(1, CLEAR_SCREEN, 4);
}

/* escape sequence to set the terminal in insert mode */
#define DECIM "\x1B[4h"
static ssize_t set_insert_mode()
{
    return write(1, DECIM, 4);
}

/* escape sequence to unset the terminal insert mode */
#define UNDECIM "\x1B[4l"
static ssize_t unset_insert_mode()
{
    return write(1, UNDECIM, 4);
}

/* escape sequence to save cursor position */
#define SCP "\x1B[s"
static ssize_t save_cursor_pos()
{
    return write(1, SCP, 3);
}

/* escape sequence to restore cursor position */
#define RCP "\x1B[u"
static ssize_t restore_cursor_pos()
{
    return write(1, RCP, 3);
}

/*
static int get_cursor_position()
{
    char buf[32] = {0};
    int x = -1;
    int y = -1;

    if (write(STDOUT_FILENO, "\x1B[6n", 4) == -1) return -1;
    if (read(STDIN_FILENO, buf, 32) == -1) return -1;

    if (buf[0] == CHAR_ESC && buf[1] == CHAR_SB)
        if (sscanf(buf+2, "%d;%d", &y, &x) != 2)
            return -1;

    return x;
}
*/

/////////////////////////////////////////////////////////////////////

static int print_fct(const char *fmt, ...)
{
    int ret = -1;
    char str[BUFFER_LEN] = {0};
    va_list arg;

    va_start(arg, fmt);
    ret = vsnprintf(str, BUFFER_LEN, fmt, arg);
    va_end(arg);

    return write(1, str, ret);
}

static void print_prompt(const char *prompt)
{
    print_fct(prompt);
}

#define CLEARLCR "\x1B[0K"
static void print_line(const struct shell *ctx, const char *line)
{
    print_fct("\r%s\r", CLEARLCR);
    print_fct("%s%s", ctx->prompt, line);
}

/////////////////////////////////////////////////////////////////////

static void get_history_entry(struct shell *ctx, char *buffer)
{
    struct list *nodep = NULL;
    struct history *tmp = NULL;
    int i = 0;

    for_each(&(ctx->history_head), nodep) {
        if (i == ctx->history_index) {
            tmp = container_of(nodep, struct history, node);
            strncpy(buffer, tmp->entry, BUFFER_LEN);
        }
        i++;
    }
}

static int add_history_entry(struct list *list, const char *buffer)
{
    struct history *new = calloc(1, sizeof(struct history));
    if (new == NULL) {
        return -1;
    }

    strncpy(new->entry, buffer, BUFFER_LEN);

    list_add_head(list, &(new->node));

    return 0;
}

/////////////////////////////////////////////////////////////////////

static int insert_char(char *buffer, char c, unsigned int pos)
{
    size_t len = strlen(buffer);

    if (len == 0) {
        buffer[0] = c;
        return 0;
    }

    while (len > pos) {
        buffer[len + 1] = buffer[len];
        len--;
    }

    buffer[len + 1] = buffer[len];
    buffer[len] = c;

    return 0;
}

static int remove_char(char *buffer, unsigned int pos)
{
    size_t len = strlen(buffer);

    if (pos >= len) {
        buffer[len - 1] = '\0';
        return 0;
    }

    buffer[pos] = '\0';

    while (pos < len) {
        buffer[pos] = buffer[pos + 1];
        pos++;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////

static void set_cursor_pos(struct shell *ctx, unsigned int nb)
{
    ctx->pos_x = nb;
    ctx->line_size = nb;
}

static void backspace_key(struct shell *ctx, char *buffer)
{
    if (ctx->pos_x > 0) {
        cursor_left(ctx);
        remove_char(buffer, ctx->pos_x);
        save_cursor_pos();
        print_line(ctx, buffer);
        restore_cursor_pos();
        ctx->line_size -= 1;
    }
}

static void delete_key(struct shell *ctx, char *buffer)
{
    if (ctx->pos_x < ctx->line_size) {
        remove_char(buffer, ctx->pos_x);
        ctx->line_size -= 1;
        save_cursor_pos();
        print_line(ctx, buffer);
        restore_cursor_pos();
    }
}

static int read_arrow_key(struct shell *ctx, const char c)
{
    switch (c) {
        case CHAR_A: // arrow up
            if (ctx->history_index < (int)(list_length(&(ctx->history_head)) - 1)) {
                ctx->history_index += 1;
                return 1;
            }
            break;
        case CHAR_B: // arrow down
            if (ctx->history_index > -1) {
                ctx->history_index -= 1;
                return 1;
            }
            break;
        case CHAR_C: // arrow right
            cursor_right(ctx);
            break;
        case CHAR_D: // arrow left
            cursor_left(ctx);
            break;
    }

    return 0;
}

/* M A I N   L O O P */
static int read_keyboard(struct shell *ctx, const char keycode[3])
{
    // DEBUG
    // printf("[%d][%d][%d]\n", keycode[0], keycode[1], keycode[2]);
    static char buffer[BUFFER_LEN] = {0};

    if (isprint(keycode[0]) != 0 && keycode[0] != CHAR_DELETE) {

        /* insert char in buffer */
        insert_char(buffer, keycode[0], ctx->pos_x);
        /* write the char */
        write(1, &keycode[0], 1);
        /* set usefull variable */
        ctx->pos_x += 1;
        ctx->line_size = strlen(buffer);

    } else {

        /* handle special characters */
        switch (keycode[0]) {
            case CHAR_BS:
                printf("BS\n");
                break;
            case CHAR_DEL: /* backspace keycode */
                backspace_key(ctx, buffer);
                break;
            case CHAR_DELETE: /* delete keycode */
                delete_key(ctx, buffer);
                break;
            case CHAR_CR: /* enter keycode */
                if (strcmp("exit", buffer) == 0) {
                    ctx->exit = 1;
                    write(1, "\r\n", 2);
                    return EXIT_SUCCESS;
                }

                write(1, "\r\n", 2);

                if (strcmp(buffer, "\0") != 0) {
                    add_history_entry(&(ctx->history_head), buffer);
                    /* execute the command */
                    execution(buffer);
                }

                print_prompt(ctx->prompt);

                /* set usefull variable */
                memset(buffer, 0, BUFFER_LEN);
                set_cursor_pos(ctx, 0);
                ctx->history_index = -1;
                break;
            case CHAR_ESC:
                if (read_arrow_key(ctx, keycode[2]) != 0) {
                    if (ctx->history_index == -1) {
                        memset(buffer, 0, BUFFER_LEN);
                        set_cursor_pos(ctx, 0);
                    } else {
                        get_history_entry(ctx, buffer);
                        set_cursor_pos(ctx, strlen(buffer));
                    }
                    print_line(ctx, buffer);
                }
                break;
            case CHAR_TAB: /* tab keycode */
                break;
            case CHAR_SOH: /* ctrl-a */
                cursor_n_left(ctx, ctx->pos_x);
                ctx->pos_x = 0;
                break;
            case CHAR_ENQ: /* ctrl-e */
                print_line(ctx, buffer);
                set_cursor_pos(ctx, strlen(buffer));
                break;
            case CHAR_ETB: /* ctrl-w */
                memmove(&buffer[0], &buffer[ctx->pos_x], ctx->line_size);
                print_line(ctx, buffer);
                ctx->pos_x = 0;
                ctx->line_size = strlen(buffer);
                cursor_n_left(ctx, ctx->line_size);
                break;
            default:
                break;
        }
    }

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
    exit(EXIT_SUCCESS);
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
    memcpy(new->prompt, "Cli>", PROMPT_LEN);

    // 3) set signal handler for SIGINT
    struct sigaction action;

    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

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

    // 6) initialize terminal and cursor position
    clear_screen();
    set_insert_mode();
    set_cursor_home();
    set_cursor_pos(new, 0);

    /* 7) initialize history */
    init_list(&(new->history_head));
    new->history_index = -1;

    /* 8) keep global_save to clean the context in case of SIGINT
     * and retrieve the terminal context
     */
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

        read_keyboard(ctx, keycode);
    }

    return 0;
}

static int terminate(struct shell *ctx)
{
    int ret = -1;

    unset_insert_mode();

    ret = set_terminal(&(ctx->saved_cfg));
    if (ret == -1) {
        return -1;
    }

    struct history *tmp = NULL;
    struct list *nodep = NULL;
    for_each(&(ctx->history_head), nodep) {
        tmp = container_of(nodep, struct history, node);
        list_delete(nodep);
        free(tmp);
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

int main(int argc, char *argv[])
{
    get_arguments(argc, argv);
    entry(argc, argv);

    return 0;
}
