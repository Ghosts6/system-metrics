#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stddef.h>
#include "metrics.h"

int send_metrics_to_api(const char *api_url, const SystemMetrics *metrics);
int format_metrics_json(const SystemMetrics *metrics, char *buffer, size_t buffer_size);

#endif
