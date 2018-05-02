#ifndef __NCURSES_PROXY_H__
#define __NCURSES_PROXY_H__


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
void nctmptest();


#endif /* __NCURSES_PROXY_H__ */

