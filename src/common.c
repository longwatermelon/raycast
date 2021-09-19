#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char* common_read_file(const char* path)
{
    char* contents = malloc(0);
    contents[0] = '\0';

    FILE* fp = fopen(path, "r");
    char* line = 0;
    size_t len = 0;
    ssize_t read;

    if (!fp)
    {
        printf("Couldn't read file %s\n", path);
        return 0;
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        unsigned long prev_length = strlen(contents);
        contents = realloc(contents, (strlen(contents) + read) * sizeof(char));

        memcpy(&contents[prev_length], line, (read - 1) * sizeof(char));
        contents[prev_length + read - 1] = '\0';
    }

    return contents;
}

