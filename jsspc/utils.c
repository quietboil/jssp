#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

const char * read_file(const char * file_name, uint16_t * size)
{
    *size = 0;
    const char * text = NULL;
    FILE * f = fopen(file_name, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        if (len <= UINT16_MAX) {
            fseek(f, 0, SEEK_SET);
            char * buf = malloc(len + 1);
            if (buf) {
                long nread = fread(buf, 1, len, f);
                buf[nread] = '\0'; // not really needed, but... just in case
                text = buf;
                *size = len;
            }
        }
        fclose(f);
    }
    return text;
}
