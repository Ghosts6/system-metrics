#include "ini_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* trim_whitespace(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

int ini_parse_file(const char* filename, IniConfig* config) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;

    config->count = 0;
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char* trimmed_line = trim_whitespace(line);
        if (trimmed_line[0] == '#' || trimmed_line[0] == '[') {
            continue; // Ignore comments and sections for this simple parser
        }

        char* equals = strchr(trimmed_line, '=');
        if (equals) {
            *equals = '\0';
            char* key = trim_whitespace(trimmed_line);
            char* value = trim_whitespace(equals + 1);

            if (config->count < MAX_INI_ENTRIES) {
                strncpy(config->entries[config->count].key, key, sizeof(config->entries[config->count].key) - 1);
                strncpy(config->entries[config->count].value, value, sizeof(config->entries[config->count].value) - 1);
                config->count++;
            }
        }
    }

    fclose(file);
    return 0;
}

const char* ini_get_value(const IniConfig* config, const char* key) {
    for (int i = 0; i < config->count; i++) {
        if (strcmp(config->entries[i].key, key) == 0) {
            return config->entries[i].value;
        }
    }
    return NULL;
}
