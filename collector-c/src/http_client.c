#include "http_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct ResponseBuffer {
    char *data;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct ResponseBuffer *mem = (struct ResponseBuffer *)userp;
    
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) return 0;
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

int format_metrics_json(const SystemMetrics *metrics, char *buffer, size_t buffer_size) {
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

    int written = snprintf(buffer, buffer_size,
        "{"
        "\"cpu_percent\":%.2f,"
        "\"cpu_count\":%d,"
        "\"cpu_freq_current\":%.2f,"
        "\"cpu_freq_min\":%.2f,"
        "\"cpu_freq_max\":%.2f,"
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
    
    return (written > 0 && written < (int)buffer_size) ? 0 : -1;
}

int send_metrics_to_api(const char *api_url, const SystemMetrics *metrics) {
    CURL *curl;
    CURLcode res;
    struct ResponseBuffer response = {0};
    char json_buffer[4096];
    char url[512];
    
    if (format_metrics_json(metrics, json_buffer, sizeof(json_buffer)) != 0) {
        fprintf(stderr, "Error: Failed to format JSON\n");
        return -1;
    }
    
    snprintf(url, sizeof(url), "%s/api/v1/metrics/collect", api_url);
    
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize CURL\n");
        return -1;
    }
    
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
        fprintf(stderr, "Error: curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(response.data);
        return -1;
    }
    
    if (response_code != 201) {
        fprintf(stderr, "Error: API returned status code %ld\n", response_code);
        if (response.data) {
            fprintf(stderr, "Response: %s\n", response.data);
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        free(response.data);
        return -1;
    }
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(response.data);
    
    return 0;
}
