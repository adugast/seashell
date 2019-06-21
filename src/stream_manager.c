#include <stdio.h>
#include <stdlib.h>

#include "stream_manager.h"


FILE *open_stream(const char *pathname, const char *mode)
{
    return fopen(pathname, mode);
}


size_t write_stream(FILE *stream, const char *line)
{
    int ret = fprintf(stream, "%s\n", line);
    fflush(stream);
    return ret;
}


void close_stream(FILE *stream)
{
    if (stream)
        fclose(stream);
}


void foreach_line_stream(FILE *stream, getline_stream_cb_t cb, void *ctx)
{
    if (!stream || !cb)
        return;

    char *line = NULL;
    size_t stream_len = 0;
    ssize_t line_len;
    while ((line_len = getline(&line, &stream_len, stream)) != -1) {
        line[line_len - 1] = '\n' ? line[line_len - 1] = '\0': 0;
        cb(line, line_len, ctx);
    }

    free(line);
}

