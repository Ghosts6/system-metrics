#ifndef METRICS_H
#define METRICS_H

#include <stdint.h>

typedef struct {
    char name[256];
    char driver_version[64];
    uint64_t memory_total;
    uint64_t memory_used;
    double temperature;
    double utilization;
} GpuMetrics;

typedef struct {
    double cpu_percent;
    int cpu_count;
    double cpu_freq_current;
    double cpu_freq_min;
    double cpu_freq_max;
    
    uint64_t memory_total;
    uint64_t memory_available;
    uint64_t memory_used;
    double memory_percent;
    
    uint64_t disk_total;
    uint64_t disk_used;
    uint64_t disk_free;
    double disk_percent;
    
    uint64_t network_bytes_sent;
    uint64_t network_bytes_recv;
    
    char hostname[256];
    char platform[64];
    double uptime_seconds;

    int gpu_count;
    GpuMetrics gpus[4];
} SystemMetrics;

int collect_metrics(SystemMetrics *metrics);
int get_cpu_metrics(SystemMetrics *metrics);
int get_memory_metrics(SystemMetrics *metrics);
int get_disk_metrics(SystemMetrics *metrics);
int get_network_metrics(SystemMetrics *metrics);
int get_system_info(SystemMetrics *metrics);
int get_gpu_metrics(SystemMetrics *metrics);

#endif
