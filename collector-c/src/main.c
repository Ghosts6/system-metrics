#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <curl/curl.h>
#include "metrics.h"
#include "http_client.h"

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep((x) * 1000)
#else
#include <unistd.h>
#endif

#define DEFAULT_API_URL "http://localhost:8000"
#define DEFAULT_INTERVAL 5

static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Options:\n");
    printf("  -u, --url URL       API base URL (default: %s)\n", DEFAULT_API_URL);
    printf("  -i, --interval SEC  Collection interval in seconds (default: %d)\n", DEFAULT_INTERVAL);
    printf("  -o, --output        Output JSON to stdout instead of sending to API\n");
    printf("  -h, --help          Show this help message\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -u http://localhost:8000 -i 10\n", program_name);
    printf("  %s --output\n", program_name);
}

int main(int argc, char *argv[]) {
    char *api_url = strdup(DEFAULT_API_URL);
    int interval = DEFAULT_INTERVAL;
    int output_only = 0;
    char json_buffer[4096];
    
    static struct option long_options[] = {
        {"url", required_argument, 0, 'u'},
        {"interval", required_argument, 0, 'i'},
        {"output", no_argument, 0, 'o'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "u:i:oh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'u':
                free(api_url);
                api_url = strdup(optarg);
                break;
            case 'i':
                interval = atoi(optarg);
                if (interval < 1) {
                    fprintf(stderr, "Error: Interval must be at least 1 second\n");
                    return 1;
                }
                break;
            case 'o':
                output_only = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    if (!output_only) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    
    SystemMetrics metrics;
    
    if (output_only) {
        if (collect_metrics(&metrics) != 0) {
            fprintf(stderr, "Error: Failed to collect metrics\n");
            return 1;
        }
        
        if (format_metrics_json(&metrics, json_buffer, sizeof(json_buffer)) != 0) {
            fprintf(stderr, "Error: Failed to format JSON\n");
            return 1;
        }
        
        printf("%s\n", json_buffer);
        return 0;
    }
    
    while (1) {
        if (collect_metrics(&metrics) != 0) {
            fprintf(stderr, "Error: Failed to collect metrics\n");
            sleep(interval);
            continue;
        }
        
        if (send_metrics_to_api(api_url, &metrics) != 0) {
            fprintf(stderr, "Error: Failed to send metrics to API\n");
        } else {
            printf("Metrics sent successfully\n");
        }
        
        sleep(interval);
    }
    
    free(api_url);
    if (!output_only) {
        curl_global_cleanup();
    }
    
    return 0;
}
