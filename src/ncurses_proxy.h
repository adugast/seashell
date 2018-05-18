#ifndef __NCURSES_PROXY_H__
#define __NCURSES_PROXY_H__

/*!
 * \file ncurses_proxy.h
 *
 * \brief Proxy API over the ncurses library.
 *
 * man (5) terminfo
 *
 * This proxy API uses ncurses then the lib must be install and
 * the program must be linked with the -lncurses option
 */

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \brief Initialize terminfo database
 *
 * This function loads the terminfo database for the
 * terminal name located in the environment variable TERM
 *
 * \return -1 in case of error, 0 in case the terminal is undefined and 1 when successful.
 */
int nc_init_terminal_data();

void nc_cursor_left();
void nc_cursor_right();
void nc_save_cursor();
void nc_restore_cursor();
void nc_delete_char();
void nc_delete_line();
void nc_clear_screen();
void nc_enter_insert_mode();
void nc_exit_insert_mode();

#ifdef __cplusplus
}
#endif

#endif /* __NCURSES_PROXY_H__ */

