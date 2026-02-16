#define _POSIX_C_SOURCE 200809L
#include "logger.h"
#include <stdlib.h>
#include <string.h>

static FILE* log_file = NULL;
static char* global_api_url = NULL; 
static int api_logging_enabled = 0; 
static LogLevel global_api_min_log_level = LOG_LEVEL_INFO;

void log_set_api_min_level(LogLevel level) {
    global_api_min_log_level = level;
}

void log_init(const char* filename, const char* api_url, int enabled) {
    if (filename) {
        log_file = fopen(filename, "a");
    }
    if (!log_file) {
        log_file = stderr;
    }
    log_set_api_url(api_url);
    log_set_api_logging_enabled(enabled);
}

void log_set_api_url(const char* url) {
    if (global_api_url) {
        free(global_api_url);
        global_api_url = NULL;
    }
    if (url) {
        global_api_url = strdup(url);
    }
}

void log_set_api_logging_enabled(int enabled) {
    api_logging_enabled = enabled;
}

void log_close(void) {
    if (log_file && log_file != stderr) {
        fclose(log_file);
    }
    if (global_api_url) {
        free(global_api_url);
        global_api_url = NULL;
    }
}

void log_message(LogLevel level, const char* message) {
    // Local file logging
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
        case LOG_LEVEL_DEBUG:
            level_str = "DEBUG";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }

    fprintf(log_file, "[%s] [%s] %s\n", time_str, level_str, message);
    fflush(log_file);

    // Send log to API if enabled, URL is set, and log level meets the minimum threshold
    if (api_logging_enabled && global_api_url && level >= global_api_min_log_level) {
        send_log_to_api(global_api_url, level_str, message);
    }
}
