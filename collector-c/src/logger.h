#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>
#include "http_client.h"

typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG 
} LogLevel;

void log_init(const char* filename, const char* api_url, int api_logging_enabled);
void log_close(void);
void log_message(LogLevel level, const char* message);
void log_set_api_url(const char* url); 
void log_set_api_logging_enabled(int enabled);
void log_set_api_min_level(LogLevel level);

#endif // LOGGER_H
