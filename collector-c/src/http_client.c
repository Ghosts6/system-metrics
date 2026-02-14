#include "http_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "logger.h"

struct ResponseBuffer {
    char *data;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct ResponseBuffer *mem = (struct ResponseBuffer *)userp;
    
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        log_message(LOG_LEVEL_ERROR, "Failed to reallocate memory for response buffer");
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

char* format_metrics_json(const SystemMetrics *metrics) {
    char gpu_buffer[2048] = {0};
    if (metrics->gpu_count > 0) {
        strcat(gpu_buffer, ",\"gpus\":[");
        for (int i = 0; i < metrics->gpu_count; i++) {
            char temp_buffer[512];
            snprintf(temp_buffer, sizeof(temp_buffer),
                "%s{"
                "\"name\":\"%s\","
                "\"driver_version\":\"%s\","
                "\"memory_total\":%llu,"
                "\"memory_used\":%llu,"
                "\"temperature\":%.2f,"
                "\"utilization\":%.2f"
                "}",
                (i > 0 ? "," : ""),
                metrics->gpus[i].name,
                metrics->gpus[i].driver_version,
                (unsigned long long)metrics->gpus[i].memory_total,
                (unsigned long long)metrics->gpus[i].memory_used,
                metrics->gpus[i].temperature,
                metrics->gpus[i].utilization
            );
            strcat(gpu_buffer, temp_buffer);
        }
        strcat(gpu_buffer, "]");
    }

    size_t buffer_size = 4096;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        log_message(LOG_LEVEL_ERROR, "Failed to allocate memory for JSON buffer");
        return NULL;
    }

    int written = snprintf(buffer, buffer_size,
        "{"
        "\"cpu_percent\":%.2f,"
        "\"cpu_count\":%d,"
        "\"cpu_freq_current\":%.2f,"
        "\"cpu_freq_min\":%.2f,"
        "\"cpu_freq_max\":%.2f,"
        "\"cpu_brand\":\"%s\","
        "\"cpu_vendor_id\":\"%s\","
        "\"memory_total\":%llu,"
        "\"memory_available\":%llu,"
        "\"memory_used\":%llu,"
        "\"memory_percent\":%.2f,"
        "\"disk_total\":%llu,"
        "\"disk_used\":%llu,"
        "\"disk_free\":%llu,"
        "\"disk_percent\":%.2f,"
        "\"network_bytes_sent\":%llu,"
        "\"network_bytes_recv\":%llu,"
        "\"hostname\":\"%s\","
        "\"platform\":\"%s\","
        "\"uptime_seconds\":%.2f,"
        "\"gpu_count\":%d%s"
        "}",
        metrics->cpu_percent,
        metrics->cpu_count,
        metrics->cpu_freq_current,
        metrics->cpu_freq_min,
        metrics->cpu_freq_max,
        metrics->cpu_brand,
        metrics->cpu_vendor_id,
        (unsigned long long)metrics->memory_total,
        (unsigned long long)metrics->memory_available,
        (unsigned long long)metrics->memory_used,
        metrics->memory_percent,
        (unsigned long long)metrics->disk_total,
        (unsigned long long)metrics->disk_used,
        (unsigned long long)metrics->disk_free,
        metrics->disk_percent,
        (unsigned long long)metrics->network_bytes_sent,
        (unsigned long long)metrics->network_bytes_recv,
        metrics->hostname,
        metrics->platform,
        metrics->uptime_seconds,
        metrics->gpu_count,
        gpu_buffer
    );
    
    if (written < 0 || (size_t)written >= buffer_size) {
        log_message(LOG_LEVEL_ERROR, "Failed to format metrics JSON: buffer too small");
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

int send_metrics_to_api(const char *api_url, const SystemMetrics *metrics) {
    CURL *curl;
    CURLcode res;
    struct ResponseBuffer response = {0};
    char *json_buffer = NULL;
    char url[512];
    char log_buffer[1024];
    
    json_buffer = format_metrics_json(metrics);
    if (!json_buffer) {
        // Error is already logged in format_metrics_json
        return -1;
    }
    
    snprintf(url, sizeof(url), "%s/api/v1/metrics/collect", api_url);
    
    curl = curl_easy_init();
    if (!curl) {
        log_message(LOG_LEVEL_ERROR, "Failed to initialize CURL");
        free(json_buffer);
        return -1;
    }
    
    response.data = malloc(1);
    response.size = 0;
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_buffer);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    res = curl_easy_perform(curl);
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    if (res != CURLE_OK) {
        snprintf(log_buffer, sizeof(log_buffer), "curl_easy_perform() failed: %s", curl_easy_strerror(res));
        log_message(LOG_LEVEL_ERROR, log_buffer);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(response.data);
        free(json_buffer);
        return -1;
    }
    
    if (response_code != 201) {
        snprintf(log_buffer, sizeof(log_buffer), "API returned status code %ld. Response: %s", 
                 response_code, response.data ? response.data : "No response data");
        log_message(LOG_LEVEL_ERROR, log_buffer);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(response.data);
        free(json_buffer);
        return -1;
    }
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(response.data);
    free(json_buffer);
    
    return 0;
}

int send_log_to_api(const char *api_url, const char *level, const char *message) {
    CURL *curl;
    CURLcode res;
    char url[512];
    char *json_payload = NULL;
    struct curl_slist *headers = NULL;
    struct ResponseBuffer response = {0};
    
    size_t payload_max_len = 256 + strlen(level) + strlen(message); 
    json_payload = (char*)malloc(payload_max_len);
    if (!json_payload) {
        fprintf(stderr, "ERROR: Failed to allocate memory for log JSON payload\n");
        return -1;
    }
    snprintf(json_payload, payload_max_len,
             "{\"level\":\"%s\",\"message\":\"%s\",\"source\":\"C_Collector\"}",
             level, message);

    snprintf(url, sizeof(url), "%s/api/v1/logs/", api_url);

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "ERROR: Failed to initialize CURL for log sending\n");
        free(json_payload);
        return -1;
    }
    
    response.data = malloc(1); 
    response.size = 0;

    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); 
    
    res = curl_easy_perform(curl);
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    if (res != CURLE_OK) {
        fprintf(stderr, "ERROR: curl_easy_perform() failed for log: %s\n", curl_easy_strerror(res));
    } else if (response_code != 201) {
        fprintf(stderr, "ERROR: Log API returned status code %ld. Response: %s\n", 
                 response_code, response.data ? response.data : "No response data");
    }
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(response.data);
    free(json_payload);
    
    return (res == CURLE_OK && response_code == 201) ? 0 : -1;
}

