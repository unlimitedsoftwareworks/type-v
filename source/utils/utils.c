//
// Created by praisethemoon on 24.11.23.
//

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"

#define MAX_LINE_LENGTH 1024
void typev_assert(int cond, const char * rawcond, const char* func_name, const char * fmt, ...) {
    if(cond)
        return;
    char temp[1024];
    va_list vl;
    va_start(vl, fmt);
    vsprintf(temp, fmt, vl);
    va_end(vl);
    fprintf(stdout, "Fatal error, assertion failed: `%s` in function `%s` \n", rawcond, func_name);
    fprintf(stdout, "%s", temp);
    fprintf(stdout, "\n");
    assert(cond);
    ASSERT(1==0, "You should not be here");
}

#define MAX_LINE_LENGTH 1024
#define MAX_FILENAME_LENGTH 256

int get_source_map_line_content(const char* sourceMapFile, uint64_t position, char* file, uint64_t* line, uint64_t* pos, char* func_name) {
    FILE *fp = fopen(sourceMapFile, "r");
    if (!fp) {
        perror("Error opening source map file");
        return -1;
    }

    char buffer[MAX_LINE_LENGTH];
    char last_valid_file[MAX_FILENAME_LENGTH] = "";
    char last_valid_func[MAX_FILENAME_LENGTH] = "";
    uint64_t last_valid_line = 0, last_valid_pos = 0;
    uint64_t current_line = 0;
    int found = 0;

    while (fgets(buffer, MAX_LINE_LENGTH, fp)) {
        char temp_file[MAX_FILENAME_LENGTH];
        char temp_fn[MAX_FILENAME_LENGTH];
        uint64_t temp_line;
        uint64_t temp_pos;
        int res = sscanf(buffer, "%[^,],%llu,%llu,%[^\n]", temp_file, &temp_line, &temp_pos, temp_fn);
        if (res == 4) {
            strcpy(last_valid_file, temp_file);
            strcpy(last_valid_func, temp_fn);
            last_valid_line = temp_line;
            last_valid_pos = temp_pos;

            if (current_line >= position) {
                found = 1;
                break;
            }
        }
        current_line++;
    }

    if (found) {
        strcpy(file, last_valid_file);
        strcpy(func_name, last_valid_func);
        *line = last_valid_line;
        *pos = last_valid_pos;
    }

    fclose(fp);
    return found ? 0 : -1;
}
