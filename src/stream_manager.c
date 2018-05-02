#include <stdio.h>
#include <stdarg.h>


FILE *open_stream(const char *pathname, const char *mode)
{
    return fopen(pathname, mode);
}


size_t write_stream(FILE *stream, const char *line)
{
    return fprintf(stream, "%s\n", line);
}


void close_stream(FILE *stream)
{
    if (stream)
        fclose(stream);
}

