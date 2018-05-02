#include <stdio.h>
#include <stdlib.h>
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

