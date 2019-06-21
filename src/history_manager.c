#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "history_manager.h"
#include "list.h"
#include "stream_manager.h"


LIST_HEAD(history_head);


struct history_list {
    char *entry;
    struct list_head node;
};


struct history {
    FILE *history_stream;
    unsigned int threshold;         // TODO: Add history limit
    struct list_head *current_node; // pointer to the current history_list node
};


static int hist_add_entry_list(history_t *hdl, const char *entry)
{
    if (!hdl || !entry)
        goto clean;

    struct history_list *new = calloc(1, sizeof(struct history_list));
    if (!new)
        goto clean;

    new->entry = strdup(entry);
    if (!new->entry)
        goto clean_history_list;

    list_add_head(&(new->node), &history_head);

    // Will be used to implement history limit/threshold ?
    hdl = hdl;

    return 0;

clean_history_list:
    free(new);
clean:
    return -1;
}


static void fill_history_list_cb(char *line, __attribute__ ((unused)) size_t line_len, void *ctx)
{
    history_t *hdl = (history_t *)ctx;
    hist_add_entry_list(hdl, line);
}


history_t *hist_init(const char *path)
{
    if (!path)
        goto clean;

    history_t *hdl = calloc(1, sizeof(history_t));
    if (!hdl)
        goto clean;

    hdl->history_stream = open_stream(path, "a+b");
    if (!hdl->history_stream)
        goto clean_history;

    foreach_line_stream(hdl->history_stream, &fill_history_list_cb, hdl);
    hist_set_head_entry(hdl);
    hdl->threshold = 0;

    return hdl;

clean_history:
    free(hdl);
clean:
    return NULL;
}


int hist_deinit(history_t *hdl)
{
    if (!hdl)
        return -1;

    struct history_list *pos, *safe;
    list_for_each_entry_safe(pos, safe, &history_head, node) {
        list_del(&(pos->node));
        free(pos->entry);
        free(pos);
    }

    close_stream(hdl->history_stream);

    free(hdl);

    return 0;
}


int hist_add_entry(history_t *hdl, const char *entry)
{
    if (!hdl)
        return -1;

    hist_add_entry_list(hdl, entry);
    write_stream(hdl->history_stream, entry);
    return 0;
}


int hist_set_head_entry(history_t *hdl)
{
    if (!hdl)
        return -1;

    hdl->current_node = &history_head;
    return 0;
}


int hist_set_prev_entry(history_t *hdl)
{
    if (!hdl)
        return -1;

    if (hdl->current_node != &history_head)
        hdl->current_node = hdl->current_node->prev;

    return 0;
}


int hist_set_next_entry(history_t *hdl)
{
    if (!hdl)
        return -1;

    if ((hdl->current_node)->next != &history_head)
        hdl->current_node = (hdl->current_node)->next;

    return 0;
}


int hist_get_current_entry(history_t *hdl, char *buffer, size_t buffer_len)
{
    if (!hdl)
        return -1;

    if (hdl->current_node == &history_head) {
        memset(buffer, 0, buffer_len);
        return 0;
    }

    struct history_list *ret = list_entry(hdl->current_node, struct history_list, node);
    if (!ret)
        return -1;

    strncpy(buffer, ret->entry, buffer_len);

    return 0;
}

