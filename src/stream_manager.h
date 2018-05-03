#ifndef __STREAM_MANAGER_H__
#define __STREAM_MANAGER_H__


/* signature of user callback for the foreach_line_stream function
 * the function passed as callback will be called for each line contained in
 * the stream and will received as argument the line and the line lenght
 * for post-processing
 */
typedef void (*getline_stream_cb_t)(char *line, size_t line_len, void *ctx);

void foreach_line_stream(FILE *stream, getline_stream_cb_t cb, void *ctx);


FILE *open_stream(const char *pathname, const char *mode);
size_t write_stream(FILE *stream, const char *line);
void close_stream(FILE *stream);


#endif /* __STREAM_MANAGER_H__ */

