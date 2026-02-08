#ifndef INI_PARSER_H
#define INI_PARSER_H

#define MAX_INI_ENTRIES 64

typedef struct {
    char key[256];
    char value[256];
} IniEntry;

typedef struct {
    IniEntry entries[MAX_INI_ENTRIES];
    int count;
} IniConfig;

int ini_parse_file(const char* filename, IniConfig* config);
const char* ini_get_value(const IniConfig* config, const char* key);

#endif // INI_PARSER_H
