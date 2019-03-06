#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <term.h>


/*
 * Proxy API over ncurses library
 *
 * man (5) terminfo
 *
 * This proxy API covers the following capabilities:
 *
 * Cap Name:               Cap Code:
 *
 * cursor_left                le
 * cursor_right               nd
 * save_cursor                sc
 * restore_cursor             rc
 * delete_char                dc
 * delete_line                dl
 * clear_screen               cl
 * enter_insert_mode          im
 * exit_insert_mode           ei
 *
 */


// internal function not exported, only used in the ncurses proxy
static void print_capability(char *id)
{
    tputs(tgetstr(id, NULL), 1, putchar);
    fflush(stdout);
}


// initialize terminal data for lib ncurses
int nc_init_terminal_data()
{
    char *termtype = getenv("TERM");
    if (termtype == NULL) {
        fprintf(stderr, "Specify a terminal type with `setenv TERM <yourtype>'.\n");
        return -1;
    }

    switch (tgetent(NULL, termtype)) {
        case -1:
            fprintf(stderr, "The terminfo database could not be found\n");
            return -1;
        case 0:
            fprintf(stderr, "Terminal type `%s' is not defined\n", termtype);
            return 0;
        default:
            return 1;
    }
}


void nc_cursor_left()
{
    print_capability("le");
}


void nc_cursor_right()
{
    print_capability("nd");
}


void nc_save_cursor()
{
    print_capability("sc");
}


void nc_restore_cursor()
{
    print_capability("rc");
}


void nc_delete_char()
{
    print_capability("dc");
}


void nc_delete_line()
{
    print_capability("dl");
}


void nc_clear_screen()
{
    print_capability("cl");
}


void nc_enter_insert_mode()
{
    print_capability("im");
}


void nc_exit_insert_mode()
{
    print_capability("ei");
}


int nc_get_terminal(struct termios *term)
{
    return tcgetattr(STDIN_FILENO, term);
}


int nc_set_terminal(struct termios *term)
{
    return tcsetattr(STDIN_FILENO, TCSADRAIN, term);
}


int nc_init_terminal()
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

    return nc_set_terminal(&term);
}

