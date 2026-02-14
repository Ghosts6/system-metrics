#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stddef.h>
#include "metrics.h"

int send_metrics_to_api(const char *api_url, const SystemMetrics *metrics);
char* format_metrics_json(const SystemMetrics *metrics);
int send_log_to_api(const char *api_url, const char *level, const char *message);

#endif
