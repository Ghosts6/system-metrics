#include "logger.h"
#include <stdlib.h>
#include <string.h>

static FILE* log_file = NULL;

void log_init(const char* filename) {
    if (filename) {
        log_file = fopen(filename, "a");
    }
    if (!log_file) {
        log_file = stderr;
    }
}

void log_close(void) {
    if (log_file && log_file != stderr) {
        fclose(log_file);
    }
}

void log_message(LogLevel level, const char* message) {
    if (!log_file) {
        return;
    }

    time_t now = time(NULL);
    char* time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; // Remove newline

    const char* level_str;
    switch (level) {
        case LOG_LEVEL_INFO:
            level_str = "INFO";
            break;
        case LOG_LEVEL_WARNING:
            level_str = "WARNING";
            break;
        case LOG_LEVEL_ERROR:
            level_str = "ERROR";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }

    fprintf(log_file, "[%s] [%s] %s\n", time_str, level_str, message);
    fflush(log_file);
}
