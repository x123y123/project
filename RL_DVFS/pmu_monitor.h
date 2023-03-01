#ifndef _PMU_MONITOR_H
#define _PMU_MONITOR_H
#define _GNU_SOURCE

#include <linux/perf_event.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

struct read_format {
    uint64_t nr;
    uint64_t values[2];
};

int perf_event_open(struct perf_event_attr *attr,
                    pid_t pid,
                    int cpu,
                    int group_fd,
                    unsigned long flags);
void *pmu_monitor(void);


#endif
