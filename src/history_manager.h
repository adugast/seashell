#ifndef __HISTORY_MANAGER_H__
#define __HISTORY_MANAGER_H__


#include <stddef.h>


/*! Abstract type for history interface */
typedef struct history history_t;


history_t *hist_init();
void hist_deinit(history_t *hdl);


int hist_add_entry(history_t *hdl, const char *entry);
void hist_set_head_entry(history_t *hdl);
void hist_set_prev_entry(history_t *hdl);
void hist_set_next_entry(history_t *hdl);
int hist_get_current_entry(history_t *hdl, char *buffer, size_t buffer_len);

// void hist_set_threshold(history_t *hdl, unsigned int threshold);


#endif /* __HISTORY_MANAGER_H__ */

