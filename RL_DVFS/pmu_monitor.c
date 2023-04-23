#include "pmu_monitor.h"

float IPC;
float CPI;

int perf_event_open(struct perf_event_attr *attr,
                    pid_t pid,
                    int cpu,
                    int group_fd,
                    unsigned long flags)
{
    return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}


// int main(int argc,const char *argv[])
void *pmu_monitor(void)
{
    /*
      char cmd_buf[5];
      char cpu_freq_buf[8];
      int test_sec;

      if(argc >1) {
        strcpy(cmd_buf, argv[1]);
        test_sec = atoi(cmd_buf);
      }
      printf("test_sec = %d\n",test_sec);
     */


    struct perf_event_attr attr;
    memset(&attr, 0, sizeof(struct perf_event_attr));
    attr.size = sizeof(struct perf_event_attr);
    // monitor HW
    attr.type = PERF_TYPE_HARDWARE;
    // monitor instructions
    attr.config = PERF_COUNT_HW_INSTRUCTIONS;
    // disable for init stage
    attr.disabled = 1;
    // read one group each times
    attr.read_format = PERF_FORMAT_GROUP;
    // pid = 0, cpu = -1, means monitor current process no matter running on which cpus
    int fd = perf_event_open(&attr, 0, -1, -1, 0);
    if (fd < 0) {
        perror("Cannot open perf fd!");
        // return 1;
    }
    // create second counter
    memset(&attr, 0, sizeof(struct perf_event_attr));
    attr.size = sizeof(struct perf_event_attr);
    
    attr.type = PERF_TYPE_HARDWARE;
    
    attr.config = PERF_COUNT_HW_CPU_CYCLES;
    
    attr.disabled = 1;
    
    int fd2 = perf_event_open(&attr, 0, -1, fd, 0);
    if (fd2 < 0) {
        perror("Cannot open perf fd2!");
        // return 1;
    }
    // start counting
    ioctl(fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    int cnt = 0;
    while (1) {
        int freq;
        cnt++;
        struct read_format aread;
        
        read(fd, &aread, sizeof(struct read_format));

        IPC = (float) aread.values[0] / (float) aread.values[1];
        CPI = (float) aread.values[1] / (float) aread.values[0];

        ioctl(fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);

        printf("\n------------------\n");
        printf("IPC = %f \nCPI = %f \ninstructions=%ld  \ncycle=%ld\n", IPC,
               CPI, aread.values[0], aread.values[1]);
        printf("------------------\n");
        sleep(1);
    }
}
