#ifndef _PMU_MONITOR_H
#define _PMU_MONITOR_H
#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>

struct read_format {
    uint64_t nr;
    uint64_t values[2];
};

int perf_event_open(struct perf_event_attr *attr, 
                    pid_t pid, int cpu, 
                    int group_fd,
                    unsigned long flags);
void *pmu_monitor(void);


#endif
