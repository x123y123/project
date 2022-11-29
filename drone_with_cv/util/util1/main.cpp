/* This code will execute 4~5 threads to run the drone programming */


#define _GNU_SOURCE
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sched.h>

#include "gps.h"
#include "mygyro.h"
#include "main.h"

#define default_port "ttyUSB0"
#define gettid() syscall(__NR_gettid)
#define test 
#define tsec 60
#define available_gpufreq   13

#define setaffinity
#define DVFS

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;
int loop_stats_gyropid = 0;
int g_index = 12;    //numbers of available_gfreq
int motor_freq = 0;  //set motor Hz

struct CFG_format
{
    int gpufreq;
    int cpufreq;
}CFG_format[13];

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags)
{
    return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

void motor_set_val(int number, int motor_val)
{
    int i;

    if (number == 6) {
        cout << "mode 6" << endl;
        int pwm[4];
        cout << "mode " << number << endl;
        for (i = 0; i < 4; i++) {
            pwm[i] = motor.getPWM(motor_no[i]);
            motor.setPWM(motor_no[i], pwm[i] + motor_val);
            cout << "set motor " << motor_no[i] << " as " << motor_val + pwm[i] << endl;
        }
    }
    else if (motor_val >= min_pulse && motor_val <= max_pulse) {
        if (number == 5) {
            for (i = 0; i < 4; i++) {
                motor.setPWM(motor_no[i], motor_val);
                cout << "set motor " << motor_no[i] << " as " << motor_val << endl;
            }
        }
        else if (number >= 1 && number <= motor_num) {
            motor.setPWM(motor_no[number - 1], motor_val);
            cout << "set motor " << motor_no[number - 1] << " as " << motor_val << endl;
        }
    }
}

void motor_stop() 
{
    for (int i = 1; i <= motor_num; i++)
        motor_set_val(i, min_pulse);
}

void control()
{
    int i, j;
    float pidout, motorF[4] = {0.0f}, motorV[4] = {0.0f};
    

    control_core[0].pid.Calculate(tan(imusol[0]), control_core[0].target);
    control_core[1].pid.Calculate(tan(imusol[1]), control_core[1].target);


    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            motorF[j] += control_core[i].pid.output * control_core[i].weight[j];
        }
    }

    for (i = 0; i < 4; i++) {
        motorV[i] = (motorF[i] - transferFuc[1][i]) / transferFuc [0][i] / 200;
        //printf("%f\t%f\n", motorF[i], motorV[i]);
        //motor_set_val(i+1, motorV[i]);
    }
}

void control_init()
{
    for (int i = 0; i < 2; i++)
        control_core[i].target = 0.0f;
    
    control_core[0].target = -0.2f;
    control_core[1].target = -0.2f;
    control_core[3].target = 32.0f;

    control_core[0].weight[2] = -1.0f;
    control_core[0].weight[3] = -1.0f;
    control_core[1].weight[0] = -1.0f;
    control_core[1].weight[3] = -1.0f;
}

static inline double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec - t1.tv_nsec < 0) {
        diff.tv_sec = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0f);
}



static void *gyropid(void *param)
{
//set affinity-------------------------------------
#ifdef setaffinity
    int cpu_id = 3;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    printf("thread_main sched_getcpu = %d\n", sched_getcpu());
#endif
//-------------------------------------------------
    
    int freq, gyro_fd, power, total_power, packs = 0;

    FILE *fq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq","r");
    fscanf(fq, "%d", &freq);
    FILE *p = fopen("/sys/bus/i2c/drivers/ina3221x/7-0040/iio\:device0/in_power1_input","r");
    fscanf(p, "%d", &power);
    FILE *tp = fopen("/sys/bus/i2c/drivers/ina3221x/7-0040/iio\:device0/in_power0_input","r");
    fscanf(tp, "%d", &total_power);
    
    printf("gyro_thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(), pthread_self());


/* sensor init */

    gyro_fd = gyro_init("ttyUSB0");
    motor.setPWMFreq(frequency);
    control_init();


    time_t endwait, test_sec;
    time_t stop_sec = tsec;
    int cnt = 0, t = 0;
    int motor_data[60], data_index = 0;
    double sum = 0;
    struct timespec g_s, g_e;

    endwait = time(NULL) + stop_sec;
    test_sec = time(NULL) + 1;
    
    int times_cnt = 0;
    while(time(NULL) < endwait) {
        pthread_mutex_lock(&stats_lock);
        loop_stats_gyropid = 1;
        pthread_mutex_unlock(&stats_lock);
        
        times_cnt ++;
        if (times_cnt != 5000) 
            continue;
        else {
            clock_gettime(CLOCK_REALTIME, &g_s);
            get_gyro_data(gyro_fd);
            control();
            clock_gettime(CLOCK_REALTIME, &g_e);

            sum += diff_in_second(g_s, g_e);

            cnt++;
            if (time(NULL) == test_sec) {
                pthread_mutex_lock(&lock);
                motor_freq = cnt;
                pthread_mutex_unlock(&lock);
                motor_data[data_index] = motor_freq;
                data_index++;
                cnt = 0;
                test_sec = time(NULL) + 1;
            }
            times_cnt = 0;
        }
        pthread_mutex_lock(&stats_lock);
        loop_stats_gyropid = 0;
        pthread_mutex_unlock(&stats_lock);
    }
    close(gyro_fd);
    fclose(fq);
    fclose(p);
    fclose(tp);

    return NULL;
}

void *cv(void *param)
{
//set affinity-------------------------------------
#ifdef setaffinity
    int cpu_id = 2;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    printf("thread_main sched_getcpu = %d\n", sched_getcpu());
#endif
//-------------------------------------------------
    char cmd[50];
    printf("cv_thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(), pthread_self());
    strcpy(cmd, "detectnet /dev/video0 --width=1920 --height=1080");
    system(cmd);
    return NULL;
}

#ifdef DVFS
static inline void sig_usr(int signo)
{
    FILE *set_gfreq, *set_cfreq;

    set_cfreq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed","w");
    set_gfreq = fopen("/sys/devices/17000000.gv11b/devfreq/17000000.gv11b/userspace/set_freq", "w");

    if (g_index < 0)
        g_index = 0;
    else if(g_index > 12)
        g_index = 12;
    
    //SIGUSR1 => (cuda time < 34ms)
    //SIGUSR2 => (cuda time >= 34ms)
    if (signo == SIGUSR1) {
        g_index --;
        pthread_mutex_lock(&flock);
        fprintf(set_cfreq, "%d", CFG_format[g_index].cpufreq);
        fprintf(set_gfreq,"%d", CFG_format[g_index].gpufreq);
        pthread_mutex_unlock(&flock);
    } else if (signo == SIGUSR2) {
        g_index ++;
        pthread_mutex_lock(&flock);
        fprintf(set_cfreq, "%d", CFG_format[g_index].cpufreq);
        fprintf(set_gfreq,"%d", CFG_format[g_index].gpufreq);
        pthread_mutex_unlock(&flock);
    }
    pclose(set_cfreq);
    pclose(set_gfreq);
}

void *dvfs(void *param)
{
    printf("gpu_dvfs_thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(), pthread_self());
    
//set affinity-------------------------------------
#ifdef setaffinity
    int cpu_id = 1;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    printf("thread_main sched_getcpu = %d\n", sched_getcpu());
#endif
//-------------------------------------------------

    time_t endwait, stop_time = 60;
    endwait = time(NULL) + stop_time; 
    while(time(NULL) < endwait) {
        if (loop_stats_gyropid == 0) {
            if (signal(SIGUSR1, sig_usr) == SIG_ERR)
                perror("can't catch SIGUSR1\n");
            else if (signal(SIGUSR2, sig_usr) == SIG_ERR)
                perror("can't catch SIGUSR2\n");
        }
    }

    return NULL;
}
#endif

int main(int argc, char *argv[])
{
    FILE * fd = fopen("threadlog.txt","w");

    pthread_t gyropidT;
    pthread_t gpsT;
    pthread_t cvT;
    int  ret;

//set affinity-------------------------------------
#ifdef setaffinity
    int cpu_id = 0;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    printf("thread_main sched_getcpu = %d\n", sched_getcpu());
#endif
//-------------------------------------------------

    int gfreq[13] = {204000000, 306000000, 408000000, 510000000, 599250000, 701250000, 803250000, 854250000, 905250000, 956250000, 1007250000, 1058250000, 1109250000};
    for (int i = 0; i < available_gpufreq; i++) {
        CFG_format[i].gpufreq = gfreq[i];
        if (i < 1) {
            CFG_format[i].cpufreq = 268800; 
        }
        else if (1 <= i && i < 5) {
            CFG_format[i].cpufreq = 345600; 
        }
        else if (i >= 5) {
            CFG_format[i].cpufreq = 422400; 
        }
    }

    printf("main_thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(), pthread_self());

#ifdef DVFS
    pthread_t dvfsT;
    ret = pthread_create(&dvfsT, NULL, dvfs, NULL);
    fprintf(fd,"gpu_dvfsT creat: %d\n", time(NULL));
    if (ret == -1) {
        perror("cannot creat gpu_dvfs thread");
        return -1;
    }
#endif

    ret = pthread_create(&gyropidT, NULL, gyropid, NULL);
    fprintf(fd,"gyropidT creat: %d\n", time(NULL));
    if (ret == -1) {
        perror("cannot creat gyro thread");
        return -1;
    }
/*
    ret = pthread_create(&gpsT, NULL, gps, NULL);
    fprintf(fd,"gpsT creat: %d\n", time(NULL));
    if (ret == -1) {
        perror("cannot creat gps thread");
        return -1;
    }
*/
    ret = pthread_create(&cvT, NULL, cv, NULL);
    fprintf(fd,"cvT creat: %d\n", time(NULL));
    if (ret == -1) {
        perror("cannot creat cv thread");
        return -1;
    }
//--------------------------------------------------------------------------------------
    int c_cf, c_gf, total_power, sec; 
    FILE *freq_data = fopen("fdata.txt", "w");
    
    time_t endwait, cnt_sec;
    endwait = time(NULL) + 60;
    cnt_sec = time(NULL) + 1;
    while (time(NULL) < endwait) {
        if (time(NULL) == cnt_sec) {
            sec ++;
            FILE *cur_cf = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq","r");
            FILE *cur_gf = fopen("/sys/devices/17000000.gv11b/devfreq/17000000.gv11b/cur_freq","r");
            FILE *tp = fopen("/sys/bus/i2c/drivers/ina3221x/7-0040/iio\:device0/in_power0_input","r");
            fscanf(tp, "%d", &total_power);
            fscanf(cur_cf, "%d", &c_cf);
            fscanf(cur_gf, "%d", &c_gf);
            fprintf(freq_data, "%d %d %d %d %d %d\n", c_cf/1000, c_gf/1000000, sec, total_power, motor_freq, sched_getcpu());

            fclose(cur_cf);
            fclose(cur_gf);
            fclose(tp);
            cnt_sec = time(NULL) + 1;
        }
    }
    fclose(freq_data);

//--------------------------------------------------------------------------------------
#ifdef DVFS 
    if (pthread_join(dvfsT, NULL) != 0) {
        perror("call gpu_dvfs pthread_join function fail");
        return -1;
    } else {
        printf("gpu_dvfsT join success");
    fprintf(fd,"gpu_dvfsT join: %d\n", time(NULL));
    }
#endif

    if (pthread_join(gyropidT, NULL) != 0) {
        perror("call pthread_join function fail");
        return -1;
    } else {
        printf("gyropidT join success");
    fprintf(fd,"gyropidT join: %d\n", time(NULL));
    }

/*
    if (pthread_join(gpsT, NULL) != 0) {
        perror("call pthread_join function fail");
        return -1;
    } else {
        printf("gpsT join success");
    fprintf(fd,"gpsT join: %d\n", time(NULL));
    }
*/

    if (pthread_join(cvT, NULL) != 0) {
         perror("call pthread_join function fail");
        return -1;
    } else {
        printf("cvT join success");
        fprintf(fd,"cvT join: %d\n", time(NULL));
    }


//    motor_stop();

    pthread_mutex_destroy(&flock);
    fclose(fd);
    return 0;
}


