#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>

typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
} LogLevel;

void log_init(const char* filename);
void log_close(void);
void log_message(LogLevel level, const char* message);

#endif // LOGGER_H
