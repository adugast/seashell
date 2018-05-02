#ifndef __STREAM_MANAGER_H__
#define __STREAM_MANAGER_H__


FILE *open_stream(const char *pathname, const char *mode);
size_t write_stream(FILE *stream, const char *line);
void close_stream(FILE *stream);


#endif /* __STREAM_MANAGER_H__ */

