#define _POSIX_C_SOURCE 200809L
#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <pdh.h>
#include <iphlpapi.h>
#include <psapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <intrin.h>
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/utsname.h>
#endif

#ifdef __linux__
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#endif

#ifdef __APPLE__
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned long u_long;
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/host_info.h>
#include <time.h>
#endif

int get_system_info(SystemMetrics *metrics) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }
    
    strncpy(metrics->platform, "Windows", sizeof(metrics->platform) - 1);
    metrics->platform[sizeof(metrics->platform) - 1] = '\0';
    
    DWORD size = sizeof(metrics->hostname);
    if (GetComputerNameA(metrics->hostname, &size) == 0) {
        strncpy(metrics->hostname, "unknown", sizeof(metrics->hostname) - 1);
    }
    metrics->hostname[sizeof(metrics->hostname) - 1] = '\0';
    
    WSACleanup();
    return 0;
#else
    struct utsname uname_info;
    if (uname(&uname_info) != 0) {
        return -1;
    }
    
    strncpy(metrics->platform, uname_info.sysname, sizeof(metrics->platform) - 1);
    metrics->platform[sizeof(metrics->platform) - 1] = '\0';
    
    if (gethostname(metrics->hostname, sizeof(metrics->hostname) - 1) != 0) {
        strncpy(metrics->hostname, "unknown", sizeof(metrics->hostname) - 1);
    }
    metrics->hostname[sizeof(metrics->hostname) - 1] = '\0';
    
    return 0;
#endif
}

#ifdef __linux__

int get_cpu_metrics(SystemMetrics *metrics) {
    FILE *fp;
    char line[256];
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    unsigned long long total, total_idle;
    static unsigned long long prev_total = 0, prev_idle = 0;
    
    fp = fopen("/proc/stat", "r");
    if (!fp) return -1;
    
    if (fgets(line, sizeof(line), fp)) {
        sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq);
        
        total_idle = idle + iowait;
        total = user + nice + system + idle + iowait + irq + softirq;
        
        if (prev_total > 0) {
            unsigned long long total_delta = total - prev_total;
            unsigned long long idle_delta = total_idle - prev_idle;
            
            if (total_delta > 0) {
                metrics->cpu_percent = 100.0 * (1.0 - ((double)idle_delta / total_delta));
            }
        }
        
        prev_total = total;
        prev_idle = total_idle;
    }
    fclose(fp);
    
    metrics->cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    
    fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        double min_freq = 0, max_freq = 0, current_freq = 0;
        int freq_count = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "model name", 10) == 0) {
                char* value = strchr(line, ':');
                if (value) {
                    strncpy(metrics->cpu_brand, value + 2, sizeof(metrics->cpu_brand) - 1);
                    metrics->cpu_brand[strcspn(metrics->cpu_brand, "\n")] = 0;
                }
            } else if (strncmp(line, "vendor_id", 9) == 0) {
                char* value = strchr(line, ':');
                if (value) {
                    strncpy(metrics->cpu_vendor_id, value + 2, sizeof(metrics->cpu_vendor_id) - 1);
                    metrics->cpu_vendor_id[strcspn(metrics->cpu_vendor_id, "\n")] = 0;
                }
            }
            if (strncmp(line, "cpu MHz", 7) == 0) {
                double freq;
                if (sscanf(line, "cpu MHz : %lf", &freq) == 1) {
                    if (freq_count == 0 || freq < min_freq) min_freq = freq;
                    if (freq > max_freq) max_freq = freq;
                    current_freq = freq;
                    freq_count++;
                }
            }
        }
        fclose(fp);
        
        metrics->cpu_freq_current = current_freq;
        metrics->cpu_freq_min = min_freq;
        metrics->cpu_freq_max = max_freq;
    }
    
    return 0;
}

int get_memory_metrics(SystemMetrics *metrics) {
    FILE *fp;
    char line[256];
    unsigned long long mem_total = 0, mem_free = 0, mem_available = 0, buffers = 0, cached = 0;
    
    fp = fopen("/proc/meminfo", "r");
    if (!fp) return -1;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %llu kB", &mem_total);
        } else if (strncmp(line, "MemFree:", 8) == 0) {
            sscanf(line, "MemFree: %llu kB", &mem_free);
        } else if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %llu kB", &mem_available);
        } else if (strncmp(line, "Buffers:", 8) == 0) {
            sscanf(line, "Buffers: %llu kB", &buffers);
        } else if (strncmp(line, "Cached:", 7) == 0) {
            sscanf(line, "Cached: %llu kB", &cached);
        }
    }
    fclose(fp);
    
    metrics->memory_total = mem_total * 1024;
    if (mem_available > 0) {
        metrics->memory_available = mem_available * 1024;
    } else {
        metrics->memory_available = mem_free * 1024;
    }
    metrics->memory_used = (mem_total - mem_free - buffers - cached) * 1024;
    metrics->memory_percent = mem_total > 0 ? 
        (100.0 * (mem_total - mem_available) / mem_total) : 0.0;
    
    return 0;
}

int get_disk_metrics(SystemMetrics *metrics) {
    struct statvfs stat;
    if (statvfs("/", &stat) != 0) {
        return -1;
    }
    
    metrics->disk_total = (uint64_t)stat.f_blocks * stat.f_frsize;
    metrics->disk_free = (uint64_t)stat.f_bavail * stat.f_frsize;
    metrics->disk_used = metrics->disk_total - ((uint64_t)stat.f_bfree * stat.f_frsize);
    metrics->disk_percent = metrics->disk_total > 0 ?
        (100.0 * metrics->disk_used / metrics->disk_total) : 0.0;
    
    return 0;
}

int get_network_metrics(SystemMetrics *metrics) {
    FILE *fp;
    char line[256];
    unsigned long long rx_bytes = 0, tx_bytes = 0;
    
    fp = fopen("/proc/net/dev", "r");
    if (!fp) return -1;
    
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return -1;
    }
    
    while (fgets(line, sizeof(line), fp)) {
        char iface[16];
        unsigned long long r_bytes, r_packets, r_errs, r_drop;
        unsigned long long t_bytes, t_packets, t_errs, t_drop;
        
        if (sscanf(line, "%15s %llu %llu %llu %llu %*u %*u %*u %*u %llu %llu %llu %llu",
                   iface, &r_bytes, &r_packets, &r_errs, &r_drop,
                   &t_bytes, &t_packets, &t_errs, &t_drop) >= 9) {
            if (strncmp(iface, "lo", 2) != 0) {
                rx_bytes += r_bytes;
                tx_bytes += t_bytes;
            }
        }
    }
    fclose(fp);
    
    metrics->network_bytes_recv = rx_bytes;
    metrics->network_bytes_sent = tx_bytes;
    
    return 0;
}

#elif defined(__APPLE__)

int get_cpu_metrics(SystemMetrics *metrics) {
    size_t size = sizeof(int);
    int cpu_count;
    if (sysctlbyname("hw.ncpu", &cpu_count, &size, NULL, 0) != 0) {
        return -1;
    }
    metrics->cpu_count = cpu_count;

    size = sizeof(metrics->cpu_brand);
    if (sysctlbyname("machdep.cpu.brand_string", metrics->cpu_brand, &size, NULL, 0) != 0) {
        strncpy(metrics->cpu_brand, "unknown", sizeof(metrics->cpu_brand) - 1);
    }

    size = sizeof(metrics->cpu_vendor_id);
    if (sysctlbyname("machdep.cpu.vendor", metrics->cpu_vendor_id, &size, NULL, 0) != 0) {
        strncpy(metrics->cpu_vendor_id, "unknown", sizeof(metrics->cpu_vendor_id) - 1);
    }
    
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO,
                       (host_info_t)&cpuinfo, &count) != KERN_SUCCESS) {
        return -1;
    }
    
    unsigned long long total = cpuinfo.cpu_ticks[CPU_STATE_USER] +
                               cpuinfo.cpu_ticks[CPU_STATE_SYSTEM] +
                               cpuinfo.cpu_ticks[CPU_STATE_NICE] +
                               cpuinfo.cpu_ticks[CPU_STATE_IDLE];
    
    static unsigned long long prev_total = 0;
    static unsigned long long prev_idle = 0;
    
    unsigned long long idle = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
    
    if (prev_total > 0) {
        unsigned long long total_delta = total - prev_total;
        unsigned long long idle_delta = idle - prev_idle;
        
        if (total_delta > 0) {
            metrics->cpu_percent = 100.0 * (1.0 - ((double)idle_delta / total_delta));
        }
    }
    
    prev_total = total;
    prev_idle = idle;
    
    uint64_t freq;
    size = sizeof(freq);
    if (sysctlbyname("hw.cpufrequency", &freq, &size, NULL, 0) == 0) {
        metrics->cpu_freq_current = freq / 1000000.0;
        metrics->cpu_freq_min = freq / 1000000.0;
        metrics->cpu_freq_max = freq / 1000000.0;
    }
    
    return 0;
}

int get_memory_metrics(SystemMetrics *metrics) {
    vm_size_t page_size;
    vm_statistics64_data_t vm_stat;
    mach_msg_type_number_t host_size = sizeof(vm_statistics64_data_t) / sizeof(natural_t);
    
    if (host_page_size(mach_host_self(), &page_size) != KERN_SUCCESS) {
        return -1;
    }
    
    if (host_statistics64(mach_host_self(), HOST_VM_INFO,
                         (host_info64_t)&vm_stat, &host_size) != KERN_SUCCESS) {
        return -1;
    }
    
    uint64_t mem_total;
    size_t size = sizeof(mem_total);
    if (sysctlbyname("hw.memsize", &mem_total, &size, NULL, 0) != 0) {
        return -1;
    }
    
    metrics->memory_total = mem_total;
    metrics->memory_available = (vm_stat.free_count + vm_stat.inactive_count) * page_size;
    metrics->memory_used = (vm_stat.active_count + vm_stat.wire_count) * page_size;
    metrics->memory_percent = mem_total > 0 ?
        (100.0 * metrics->memory_used / mem_total) : 0.0;
    
    return 0;
}

int get_disk_metrics(SystemMetrics *metrics) {
    struct statfs stat;
    if (statfs("/", &stat) != 0) {
        return -1;
    }
    
    metrics->disk_total = (uint64_t)stat.f_blocks * stat.f_bsize;
    metrics->disk_free = (uint64_t)stat.f_bavail * stat.f_bsize;
    metrics->disk_used = metrics->disk_total - metrics->disk_free;
    metrics->disk_percent = metrics->disk_total > 0 ?
        (100.0 * metrics->disk_used / metrics->disk_total) : 0.0;
    
    return 0;
}

int get_network_metrics(SystemMetrics *metrics) {
    // TODO: Fix network metrics collection on macOS
    metrics->network_bytes_recv = 0;
    metrics->network_bytes_sent = 0;
    
    return 0;
}

#elif defined(_WIN32)

int get_cpu_metrics(SystemMetrics *metrics) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    metrics->cpu_count = sysInfo.dwNumberOfProcessors;

    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 0);
    char vendor[13];
    memcpy(vendor, &cpuInfo[1], 4);
    memcpy(vendor + 4, &cpuInfo[3], 4);
    memcpy(vendor + 8, &cpuInfo[2], 4);
    vendor[12] = '\0';
    strncpy(metrics->cpu_vendor_id, vendor, sizeof(metrics->cpu_vendor_id) - 1);

    char brand[49] = {0};
    for (int i = 0x80000002; i <= 0x80000004; ++i) {
        __cpuid(cpuInfo, i);
        memcpy(brand + (i - 0x80000002) * 16, cpuInfo, 16);
    }
    brand[48] = '\0';
    strncpy(metrics->cpu_brand, brand, sizeof(metrics->cpu_brand) - 1);
    
    return 0;
    
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE value;
    
    if (PdhOpenQuery(NULL, 0, &query) != ERROR_SUCCESS) {
        return -1;
    }
    
    if (PdhAddCounter(query, L"\\Processor(_Total)\\% Processor Time", 0, &counter) != ERROR_SUCCESS) {
        PdhCloseQuery(query);
        return -1;
    }
    
    PdhCollectQueryData(query);
    Sleep(100);
    PdhCollectQueryData(query);
    
    if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
        metrics->cpu_percent = value.doubleValue;
    } else {
        metrics->cpu_percent = 0.0;
    }
    
    PdhCloseQuery(query);
    
    metrics->cpu_freq_current = 0.0;
    metrics->cpu_freq_min = 0.0;
    metrics->cpu_freq_max = 0.0;
    
    return 0;
}

int get_memory_metrics(SystemMetrics *metrics) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    
    metrics->memory_total = memInfo.ullTotalPhys;
    metrics->memory_available = memInfo.ullAvailPhys;
    metrics->memory_used = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    metrics->memory_percent = memInfo.dwMemoryLoad;
    
    return 0;
}

int get_disk_metrics(SystemMetrics *metrics) {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    
    if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        metrics->disk_total = totalBytes.QuadPart;
        metrics->disk_free = freeBytesAvailable.QuadPart;
        metrics->disk_used = totalBytes.QuadPart - totalFreeBytes.QuadPart;
        metrics->disk_percent = metrics->disk_total > 0 ?
            (100.0 * metrics->disk_used / metrics->disk_total) : 0.0;
        return 0;
    }
    
    return -1;
}

int get_network_metrics(SystemMetrics *metrics) {
    PMIB_IFTABLE ifTable;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    
    ifTable = (MIB_IFTABLE *)malloc(sizeof(MIB_IFTABLE));
    if (ifTable == NULL) {
        return -1;
    }
    
    if (GetIfTable(ifTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        free(ifTable);
        ifTable = (MIB_IFTABLE *)malloc(dwSize);
        if (ifTable == NULL) {
            return -1;
        }
    }
    
    if ((dwRetVal = GetIfTable(ifTable, &dwSize, FALSE)) == NO_ERROR) {
        uint64_t rx_bytes = 0, tx_bytes = 0;
        
        for (DWORD i = 0; i < ifTable->dwNumEntries; i++) {
            if (ifTable->table[i].dwType == MIB_IF_TYPE_ETHERNET || 
                ifTable->table[i].dwType == MIB_IF_TYPE_IEEE80211) {
                rx_bytes += ifTable->table[i].dwInOctets;
                tx_bytes += ifTable->table[i].dwOutOctets;
            }
        }
        
        metrics->network_bytes_recv = rx_bytes;
        metrics->network_bytes_sent = tx_bytes;
        
        free(ifTable);
        return 0;
    }
    
    free(ifTable);
    return -1;
}

#else

int get_cpu_metrics(SystemMetrics *metrics) {
    metrics->cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    metrics->cpu_percent = 0.0;
    return 0;
}

int get_memory_metrics(SystemMetrics *metrics) {
    metrics->memory_total = 0;
    metrics->memory_available = 0;
    metrics->memory_used = 0;
    metrics->memory_percent = 0.0;
    return 0;
}

int get_disk_metrics(SystemMetrics *metrics) {
    metrics->disk_total = 0;
    metrics->disk_free = 0;
    metrics->disk_used = 0;
    metrics->disk_percent = 0.0;
    return 0;
}

int get_network_metrics(SystemMetrics *metrics) {
    metrics->network_bytes_recv = 0;
    metrics->network_bytes_sent = 0;
    return 0;
}

#endif


#ifdef __linux__

int get_uptime_metrics(SystemMetrics *metrics) {
    struct sysinfo s_info;
    if (sysinfo(&s_info) != 0) {
        return -1;
    }
    metrics->uptime_seconds = (double)s_info.uptime;
    return 0;
}

#elif defined(__APPLE__)

#include <sys/sysctl.h>

int get_uptime_metrics(SystemMetrics *metrics) {
    struct timeval boottime;
    size_t len = sizeof(boottime);
    if (sysctlbyname("kern.boottime", &boottime, &len, NULL, 0) == -1) {
        return -1;
    }
    time_t now;
    time(&now);
    metrics->uptime_seconds = difftime(now, boottime.tv_sec);
    return 0;
}

#elif defined(_WIN32)

int get_uptime_metrics(SystemMetrics *metrics) {
    metrics->uptime_seconds = (double)GetTickCount64() / 1000.0;
    return 0;
}

#else

int get_uptime_metrics(SystemMetrics *metrics) {
    metrics->uptime_seconds = 0.0;
    return 0;
}

#endif


#if defined(__linux__)
static char* get_command_output(const char* cmd) {
    char buffer[128];
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        return NULL;
    }
    char* result = malloc(1);
    result[0] = '\0';
    size_t size = 1;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        char* new_result = realloc(result, size + strlen(buffer));
        if (!new_result) {
            free(result);
            pclose(pipe);
            return NULL;
        }
        result = new_result;
        strcpy(result + size - 1, buffer);
        size += strlen(buffer);
    }
    pclose(pipe);
    return result;
}


int get_nvidia_gpu_metrics(SystemMetrics *metrics) {
    metrics->gpu_count = 0;
    const char* cmd = "nvidia-smi --query-gpu=name,driver_version,memory.total,memory.used,temperature.gpu,utilization.gpu --format=csv,noheader,nounits";
    
    char* output = get_command_output(cmd);
    if (!output) {
        return 0; // nvidia-smi not found or failed
    }
    
    char* line = strtok(output, "\n");
    int i = 0;
    while (line != NULL && i < 4) {
        GpuMetrics* gpu = &metrics->gpus[i];
        
        char* name = strtok(line, ",");
        char* driver_version = strtok(NULL, ",");
        char* memory_total_str = strtok(NULL, ",");
        char* memory_used_str = strtok(NULL, ",");
        char* temperature_str = strtok(NULL, ",");
        char* utilization_str = strtok(NULL, ",");
        
        if (name) strncpy(gpu->name, name, sizeof(gpu->name) - 1);
        if (driver_version) strncpy(gpu->driver_version, driver_version, sizeof(gpu->driver_version) - 1);
        if (memory_total_str) gpu->memory_total = (uint64_t)atof(memory_total_str) * 1024 * 1024;
        if (memory_used_str) gpu->memory_used = (uint64_t)atof(memory_used_str) * 1024 * 1024;
        if (temperature_str) gpu->temperature = atof(temperature_str);
        if (utilization_str) gpu->utilization = atof(utilization_str);
        
        i++;
        line = strtok(NULL, "\n");
    }
    
    metrics->gpu_count = i;
    free(output);
    return 0;
}

int get_amd_gpu_metrics(SystemMetrics *metrics) {
    metrics->gpu_count = 0;
    const char* cmd = "rocm-smi --showid --showdriver --showmeminfo vram --showtemp --showuse --csv";
    
    char* output = get_command_output(cmd);
    if (!output) {
        return 0; // rocm-smi not found or failed
    }

    // Skip the header line
    char* current_line = strtok(output, "\n");
    if (current_line != NULL) {
        current_line = strtok(NULL, "\n");
    }
    
    int i = 0;
    while (current_line != NULL && i < 4) {
        GpuMetrics* gpu = &metrics->gpus[i];
        
        strtok(current_line, ","); // Consume the first token which is the GPU index
        
        char* gpu_id = strtok(NULL, ",");
        char* driver_version = strtok(NULL, ",");
        char* vram_total_str = strtok(NULL, ",");
        char* vram_used_str = strtok(NULL, ",");
        char* temperature_str = strtok(NULL, ",");
        char* utilization_str = strtok(NULL, ",");
        
        if (gpu_id) {
            snprintf(gpu->name, sizeof(gpu->name) - 1, "AMD GPU %s", gpu_id);
            gpu->name[sizeof(gpu->name) - 1] = '\0';
        }
        if (driver_version) strncpy(gpu->driver_version, driver_version, sizeof(gpu->driver_version) - 1);
        if (vram_total_str) gpu->memory_total = (uint64_t)atoll(vram_total_str);
        if (vram_used_str) gpu->memory_used = (uint64_t)atoll(vram_used_str);
        if (temperature_str) gpu->temperature = atof(temperature_str);
        if (utilization_str) gpu->utilization = atof(utilization_str);
        
        i++;
        current_line = strtok(NULL, "\n");
    }
    
    metrics->gpu_count = i;
    free(output);
    return 0;
}

int get_intel_gpu_metrics(SystemMetrics *metrics) {
    metrics->gpu_count = 0;
    FILE *fp;
    char path[256];
    char line[256];
    int i = 0;

    // Get system-wide i915 driver version
    char driver_version[64] = "unknown";
    fp = fopen("/sys/module/i915/version", "r");
    if (fp) {
        if (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\n")] = 0;
            strncpy(driver_version, line, sizeof(driver_version) - 1);
        }
        fclose(fp);
    }

    for (i = 0; i < 4; i++) { // Check first few cards
        GpuMetrics* gpu = &metrics->gpus[i];
        bool intel_gpu_found = false;

        // Check if it's an Intel GPU (using vendor ID 0x8086)
        snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/vendor", i);
        fp = fopen(path, "r");
        if (fp) {
            char vendor_id[10];
            if (fgets(vendor_id, sizeof(vendor_id), fp) != NULL) {
                if (strncmp(vendor_id, "0x8086", 6) == 0) { // Intel vendor ID
                    intel_gpu_found = true;
                }
            }
            fclose(fp);
        }

        if (!intel_gpu_found) {
            continue;
        }

        snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/uevent", i);
        fp = fopen(path, "r");
        if (fp) {
            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "PCI_ID=", 7) == 0) {
                    char *device_id_start = strchr(line + 7, ':');
                    if (device_id_start) {
                        char device_id[5];
                        strncpy(device_id, device_id_start + 1, 4);
                        device_id[4] = '\0';
                        snprintf(gpu->name, sizeof(gpu->name) - 1, "Intel GPU (0x%s)", device_id);
                        gpu->name[sizeof(gpu->name) - 1] = '\0';
                        break;
                    }
                }
            }
            fclose(fp);
        }
        if (strlen(gpu->name) == 0) {
            snprintf(gpu->name, sizeof(gpu->name) - 1, "Intel GPU Card%d", i);
            gpu->name[sizeof(gpu->name) - 1] = '\0';
        }

        // --- Driver Version ---
        strncpy(gpu->driver_version, driver_version, sizeof(gpu->driver_version) - 1);
        gpu->driver_version[sizeof(gpu->driver_version) - 1] = '\0';

        // --- Memory Total/Used---
        snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/gt_total_lmem_bytes", i);
        fp = fopen(path, "r");
        if (fp) {
            if (fgets(line, sizeof(line), fp)) gpu->memory_total = strtoull(line, NULL, 10);
            fclose(fp);
        }
        snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/gt_used_lmem_bytes", i);
        fp = fopen(path, "r");
        if (fp) {
            if (fgets(line, sizeof(line), fp)) gpu->memory_used = strtoull(line, NULL, 10);
            fclose(fp);
        }
        // Fallback or alternative
        if (gpu->memory_total == 0) {
            gpu->memory_total = metrics->memory_total;
        }


        // --- Temperature ---
        snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/hwmon/hwmon*/temp1_input", i);
        snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/hwmon/hwmon0/temp1_input", i);
        fp = fopen(path, "r");
        if (fp) {
            if (fgets(line, sizeof(line), fp)) {
                gpu->temperature = atof(line) / 1000.0;
            }
            fclose(fp);
        }

        // --- Utilization (Render engine) ---
        snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/engine/rcs0/utilization", i);
        fp = fopen(path, "r");
        if (fp) {
            if (fgets(line, sizeof(line), fp)) {
                gpu->utilization = atof(line); // Percentage 0-100
            }
            fclose(fp);
        }

        metrics->gpu_count++;
    }
    
    return 0;
}

GpuVendor detect_gpu_vendor() {
    char* output;

    // Check for NVIDIA
    output = get_command_output("nvidia-smi -L");
    if (output != NULL) {
        free(output);
        return GPU_VENDOR_NVIDIA;
    }

    // Check for AMD
    output = get_command_output("rocm-smi --version");
    if (output != NULL) {
        free(output);
        return GPU_VENDOR_AMD;
    }

    // Check for Intel (via sysfs)
    FILE *fp;
    char path[256];
    char vendor_id[10];

    for (int i = 0; i < 4; i++) { // Check first few cards
        snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/vendor", i);
        fp = fopen(path, "r");
        if (fp) {
            if (fgets(vendor_id, sizeof(vendor_id), fp) != NULL) {
                if (strncmp(vendor_id, "0x8086", 6) == 0) { // Intel vendor ID
                    fclose(fp);
                    return GPU_VENDOR_INTEL;
                }
            }
            fclose(fp);
        }
    }

    return GPU_VENDOR_NONE;
}


int get_gpu_metrics(SystemMetrics *metrics) {
    metrics->gpu_count = 0; // Reset GPU count
    GpuVendor vendor = detect_gpu_vendor();

    switch (vendor) {
        case GPU_VENDOR_NVIDIA:
            return get_nvidia_gpu_metrics(metrics);
        case GPU_VENDOR_AMD:
            return get_amd_gpu_metrics(metrics);
        case GPU_VENDOR_INTEL:
            return get_intel_gpu_metrics(metrics);
        case GPU_VENDOR_NONE:
        default:
            return 0; // No supported GPU found
    }
}
#elif defined(__APPLE__)

static char* get_command_output(const char* cmd) {
    char buffer[256];
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        return NULL;
    }
    char* result = malloc(1);
    if (!result) {
        pclose(pipe);
        return NULL;
    }
    result[0] = '\0';
    size_t size = 1;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        char* new_result = realloc(result, size + strlen(buffer));
        if (!new_result) {
            free(result);
            pclose(pipe);
            return NULL;
        }
        result = new_result;
        strcpy(result + size - 1, buffer);
        size += strlen(buffer);
    }
    pclose(pipe);
    return result;
}

int get_gpu_metrics(SystemMetrics* metrics) {
    metrics->gpu_count = 0;
    char* output = get_command_output("system_profiler SPDisplaysDataType -detailLevel mini");
    if (!output) {
        return 0;
    }

    char* line = strtok(output, "\n");
    GpuMetrics* current_gpu = NULL;

    while(line != NULL) {
        char* trimmed_line = line;
        while(*trimmed_line == ' ') trimmed_line++;

        if (strncmp(trimmed_line, "Chipset Model:", 14) == 0) {
            if (metrics->gpu_count < 4) {
                current_gpu = &metrics->gpus[metrics->gpu_count];
                metrics->gpu_count++;

                char* model = strchr(trimmed_line, ':');
                if (model) {
                    model += 2;
                    strncpy(current_gpu->name, model, sizeof(current_gpu->name) - 1);
                    current_gpu->name[sizeof(current_gpu->name) - 1] = '\0';
                }
            }
        } else if (current_gpu && strncmp(trimmed_line, "VRAM (Total):", 13) == 0) {
            char* vram_str = strchr(trimmed_line, ':');
            if (vram_str) {
                vram_str += 2;
                unsigned long long vram_mb = 0;
                sscanf(vram_str, "%llu MB", &vram_mb);
                current_gpu->memory_total = vram_mb * 1024 * 1024;
            }
        }
        line = strtok(NULL, "\n");
    }

    free(output);
    return 0;
}
#else
int get_gpu_metrics(SystemMetrics* metrics) {
    metrics->gpu_count = 0;
    return 0;
}
#endif

int collect_metrics(SystemMetrics *metrics) {
    memset(metrics, 0, sizeof(SystemMetrics));
    
    if (get_system_info(metrics) != 0) return -1;
    if (get_cpu_metrics(metrics) != 0) return -1;
    if (get_memory_metrics(metrics) != 0) return -1;
    if (get_disk_metrics(metrics) != 0) return -1;
    if (get_network_metrics(metrics) != 0) return -1;
    if (get_uptime_metrics(metrics) != 0) return -1;
    if (get_gpu_metrics(metrics) != 0) return -1;
    
    return 0;
}

