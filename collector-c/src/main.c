#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "metrics.h"
#include "http_client.h"
#include "logger.h"
#include "ini_parser.h"

#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep((x) * 1000)
#else
#include <unistd.h>
#endif

#define DEFAULT_API_URL "http://localhost:8000"
#define DEFAULT_INTERVAL 5
#define DEFAULT_CONFIG_FILE "/etc/system-metrics/collector.conf"
#define DEFAULT_LOG_TO_API 0 // Default to not sending logs to API

static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Options:\n");
    printf("  -c, --config FILE   Path to config file (default: %s)\n", DEFAULT_CONFIG_FILE);
    printf("  -u, --url URL       API base URL (default: %s)\n", DEFAULT_API_URL);
    printf("  -i, --interval SEC  Collection interval in seconds (default: %d)\n", DEFAULT_INTERVAL);
    printf("  -l, --logfile FILE  Path to log file (default: stderr)\n");
    printf("  -P, --log-to-api    Send logs to the backend API\n");
    printf("  -A, --api-log-level LEVEL  Minimum log level for API (INFO, WARNING, ERROR, DEBUG)\n");
    printf("  -d, --daemon        Run as a background daemon\n");
    printf("  -o, --output        Output JSON to stdout instead of sending to API\n");
    printf("  -v, --version       Show version information\n");
    printf("  -h, --help          Show this help message\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -c /path/to/collector.conf\n", program_name);
    printf("  %s -u http://localhost:8000 -i 10\n", program_name);
    printf("  %s -l /var/log/collector.log -d\n", program_name);
    printf("  %s -P -A WARNING\n", program_name);
}

#ifndef _WIN32
static void daemonize(void) {
    pid_t pid;

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }

    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);
}
#endif

// Helper to convert string to LogLevel
static LogLevel string_to_loglevel(const char* level_str) {
    if (strcmp(level_str, "DEBUG") == 0) return LOG_LEVEL_DEBUG;
    if (strcmp(level_str, "INFO") == 0) return LOG_LEVEL_INFO;
    if (strcmp(level_str, "WARNING") == 0) return LOG_LEVEL_WARNING;
    if (strcmp(level_str, "ERROR") == 0) return LOG_LEVEL_ERROR;
    return LOG_LEVEL_INFO; // Default to INFO
}

int main(int argc, char *argv[]) {
    char *config_file = strdup(DEFAULT_CONFIG_FILE);
    char *api_url = NULL;
    char *log_file = NULL;
    int interval = -1;
    int output_only = 0;
    int daemon = 0;
    int log_to_api = DEFAULT_LOG_TO_API;
    LogLevel api_log_level = LOG_LEVEL_INFO;
    
    static struct option long_options[] = {
        {"config", required_argument, 0, 'c'},
        {"url", required_argument, 0, 'u'},
        {"interval", required_argument, 0, 'i'},
        {"logfile", required_argument, 0, 'l'},
        {"log-to-api", no_argument, 0, 'P'},
        {"api-log-level", required_argument, 0, 'A'},
        {"daemon", no_argument, 0, 'd'},
        {"output", no_argument, 0, 'o'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "c:u:i:l:PA:dovh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c':
                free(config_file);
                config_file = strdup(optarg);
                break;
            case 'u':
                api_url = strdup(optarg);
                break;
            case 'i':
                interval = atoi(optarg);
                break;
            case 'l':
                log_file = strdup(optarg);
                break;
            case 'd':
                daemon = 1;
                break;
            case 'o':
                output_only = 1;
                break;
            case 'P':
                log_to_api = 1;
                break;
            case 'A':
                api_log_level = string_to_loglevel(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'v':
                printf("System Metrics Collector version 1.0.0\n");
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    IniConfig config = {0};
    if (ini_parse_file(config_file, &config) == 0) {
        if (!api_url) {
            const char* value = ini_get_value(&config, "api_url");
            if (value) api_url = strdup(value);
        }
        if (interval == -1) {
            const char* value = ini_get_value(&config, "interval");
            if (value) interval = atoi(value);
        }
        if (!log_file) {
            const char* value = ini_get_value(&config, "logfile");
            if (value) log_file = strdup(value);
        }
        if (log_to_api == DEFAULT_LOG_TO_API) {
            const char* value = ini_get_value(&config, "log_to_api");
            if (value) log_to_api = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        }
        // Read api_log_level from config file
        const char* value = ini_get_value(&config, "api_log_level");
        if (value) api_log_level = string_to_loglevel(value);
    }

    if (!api_url) api_url = strdup(DEFAULT_API_URL);
    if (interval == -1) interval = DEFAULT_INTERVAL;
    
    if (daemon) {
#ifdef _WIN32
        fprintf(stderr, "Daemon mode is not supported on Windows\n");
        return 1;
#else
        daemonize();
#endif
    }
    
    log_init(log_file, api_url, log_to_api);
    log_set_api_min_level(api_log_level); // Set the API min log level
    log_message(LOG_LEVEL_INFO, "Collector starting...");
    
    if (!output_only) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    
    SystemMetrics metrics;
    
    if (output_only) {
        if (collect_metrics(&metrics) != 0) {
            log_message(LOG_LEVEL_ERROR, "Failed to collect metrics");
            log_close();
            return 1;
        }
        
        char *json_buffer = format_metrics_json(&metrics);
        if (json_buffer == NULL) {
            log_message(LOG_LEVEL_ERROR, "Failed to format JSON");
            log_close();
            return 1;
        }
        
        printf("%s\n", json_buffer);
        free(json_buffer);
        log_message(LOG_LEVEL_INFO, "Metrics output to stdout");
        log_close();
        return 0;
    }
    
    char log_buffer[256];

    while (1) {
        if (collect_metrics(&metrics) != 0) {
            log_message(LOG_LEVEL_ERROR, "Failed to collect metrics");
            sleep(interval);
            continue;
        }
        
        if (send_metrics_to_api(api_url, &metrics) != 0) {
            snprintf(log_buffer, sizeof(log_buffer), "Failed to send metrics to API at %s", api_url);
            log_message(LOG_LEVEL_ERROR, log_buffer);
        } else {
            log_message(LOG_LEVEL_INFO, "Metrics sent successfully");
        }
        
        sleep(interval);
    }
    
    free(config_file);
    free(api_url);
    if (log_file) free(log_file);

    if (!output_only) {
        curl_global_cleanup();
    }
    
    log_message(LOG_LEVEL_INFO, "Collector shutting down...");
    log_close();
    
    return 0;
}
