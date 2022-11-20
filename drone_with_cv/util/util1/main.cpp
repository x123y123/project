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


#include "gps.h"
#include "mygyro.h"
#include "main.h"

#define default_port "ttyUSB0"
#define gettid() syscall(__NR_gettid)
#define test 
#define tsec 60




static volatile int done;
pthread_mutex_t lock;
pthread_barrier_t barrier;
int loop_stats_pid = 0;
int loop_stats_gyro = 0;
int loop_stats_bat_volt = 0;
int c_index = 24;    //numbers of available_cfreq
int g_index = 13;    //numbers of available_gfreq
int motor_freq = 0;  //set motor Hz
double perf_gyropid;

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
    float pidout, motorF[4] = {0.0}, motorV[4] = {0.0};
    

    control_core[0].pid.Calculate(tan(imusol[0]), control_core[0].target);
    control_core[1].pid.Calculate(tan(imusol[1]), control_core[1].target);


    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            motorF[j] += control_core[i].pid.output * control_core[i].weight[j];
        }
    }

    for (i = 0; i < 4; i++) {
        motorV[i] = (motorF[i] - transferFuc[1][i]) / transferFuc [0][i] / 200;
        printf("%f\t%f\n", motorF[i], motorV[i]);
        //motor_set_val(i+1, motorV[i]);
    }
}

void control_init()
{
    for (int i = 0; i < 2; i++)
        control_core[i].target = 0.0;
    
    control_core[0].target = -0.2;
    control_core[1].target = -0.2;
    control_core[3].target = 32.0;

    control_core[0].weight[2] = -1;
    control_core[0].weight[3] = -1;
    control_core[1].weight[0] = -1;
    control_core[1].weight[3] = -1;
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec - t1.tv_nsec < 0) {
        diff.tv_sec = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags)
{
    return syscall(__NR_sched_getattr, pid, attr, size, flags);
}


static void *gyropid(void *param)
{
    
    int freq, gyro_fd, power, total_power, packs = 0;

    //FILE *pack = fopen("/home/uav/code/GPS/master_thesis/schedtil_in_60s/gyro.txt","a+");
    FILE *gyro_data = fopen("gyro_data.txt", "w");
    FILE *fq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq","r");
    fscanf(fq, "%d", &freq);
    FILE *p = fopen("/sys/bus/i2c/drivers/ina3221x/7-0040/iio\:device0/in_power1_input","r");
    fscanf(p, "%d", &power);
    FILE *tp = fopen("/sys/bus/i2c/drivers/ina3221x/7-0040/iio\:device0/in_power0_input","r");
    fscanf(tp, "%d", &total_power);
    FILE *data = fopen("motor_freq.txt", "a+"); 
    
    printf("gyro_thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(), pthread_self());


/* sensor init */

    gyro_fd = gyro_init("ttyUSB0");
    motor.setPWMFreq(frequency);
    control_init();


    time_t endwait, test_sec;
    time_t stop_sec = tsec;
    int cnt = 0, t = 0;
    double sum = 0;
    struct timespec g_s, g_e;

    endwait = time(NULL) + stop_sec;
    test_sec = time(NULL) + 1;
    
    int suk = 0;
    while(time(NULL) < endwait) {
        loop_stats_gyro = 1;
        clock_gettime(CLOCK_REALTIME, &g_s);
        get_gyro_data(gyro_fd);
        control();
        clock_gettime(CLOCK_REALTIME, &g_e);
        
        sum += diff_in_second(g_s, g_e);
        perf_gyropid = diff_in_second(g_s, g_e);

        cnt++;
        if (time(NULL) == test_sec) {
            motor_freq = cnt;
            fprintf(data, "%d %d\n", motor_freq, cnt);
            cnt = 0;
            test_sec = time(NULL) + 1;
        }

        loop_stats_gyro = 0;
        fprintf(gyro_data, "%d\n", cnt);
        
        //usleep(1500);
        
    }
    close(gyro_fd);
    fclose(fq);
    fclose(p);
    fclose(tp);
    fclose(data);
    fclose(gyro_data);

    return NULL;
}

static void *cv(void *param)
{
    char cmd[50];
    printf("cv_thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(), pthread_self());
    strcpy(cmd, "detectnet /dev/video0");
    system(cmd);
    return NULL;
}

/* monitor bat_life */
static void *BAT_VOLT(void *param)
{
    
    return NULL;
}


void sig_usr(int signo)
{
    time_t cur_time;
    FILE *set_gfreq;
    int cur_gfq;
    int gavailable_freq[14] = {114750000, 204000000, 306000000, 408000000, 510000000, 599250000, 701250000, 803250000, 854250000, 905250000, 956250000, 1007250000, 1058250000, 1109250000};
    cur_time = time(NULL);
    
    set_gfreq = fopen("/sys/devices/17000000.gv11b/devfreq/17000000.gv11b/userspace/set_freq", "w");
    FILE *fd = fopen("/sys/devices/17000000.gv11b/devfreq/17000000.gv11b/cur_freq", "r");
    FILE *data = fopen("data.txt", "a+");
    int cur_freq;
    fscanf(fd, "%d", &cur_freq);

    if (g_index < 0)
        g_index = 0;
    else if(g_index > 13)
        g_index = 13;
    
    //SIGUSR1 => (cuda time < 34ms)
    //SIGUSR2 => (cuda time >= 34ms)
    if (signo == SIGUSR1) {
        g_index --;
        fprintf(set_gfreq,"%d", gavailable_freq[g_index]);
    } else if (signo == SIGUSR2) {
        g_index ++;
        fprintf(set_gfreq,"%d", gavailable_freq[g_index]);
    }
    fprintf(data, "%d %d %d\n", g_index, cur_freq, time(NULL));
    pclose(set_gfreq);
    fclose(data);
    fclose(fd);
}

void *gpu_dvfs(void *param)
{
    printf("gpu_dvfs_thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(), pthread_self());
    time_t endwait, stop_time = 60;
    endwait = time(NULL) + stop_time; 
    while(time(NULL) < endwait) {

        if (signal(SIGUSR1, sig_usr) == SIG_ERR)
            perror("can't catch SIGUSR1\n");
        else if (signal(SIGUSR2, sig_usr) == SIG_ERR)
            perror("can't catch SIGUSR2\n");
    }


    return NULL;
}

static void *cpu_dvfs(void *param)
{
    //sigset_t set;
    int sig;
    time_t endwait;
    time_t stop_sec = tsec;
    endwait = time(NULL) + stop_sec;

    printf("cpu_dvfs_thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(), pthread_self());
    int cfreq[25] = {115200, 192000, 268800, 345600, 422400, 499200, 576000, 652800, 729600, 806400, 883200, 960000, 1036800, 1113600, 1190400, 1267200, 1344000, 1420800, 1497600, 1574400, 1651200, 1728000, 1804800, 1881600, 1907200};
    int prev_perf_gyro = 0; //prev perf
    int cur_perf_gyro = 0;  //now perf
    int iperf_gyro = 0;     //freq in 1907200kHz perf    
    FILE *fd = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed","w");
    FILE *fq = fopen("m.txt","w");
    
    //sigemptyset(&set);
    //sigaddset(&set, SIGUSR2);
    sleep(10);
    while (time(NULL) < endwait) {
        //sigwait(&set, &sig);
        // if (loop_stats_pid == 0  && loop_stats_gyro == 0 && loop_stats_gps == 0) {
        if (loop_stats_gyro == 0) {
            printf("\nmortor freq = %d\n", motor_freq); 


            if (motor_freq > 500) {
                c_index--;

                if (c_index < 0)
                    c_index = 0;

                fprintf(fd, "%d ", cfreq[c_index]);
                fprintf(fq, "%d\n", motor_freq);
            } else {
                c_index++;

                if (c_index > 24)
                    c_index = 24;

                fprintf(fd, "%d ", cfreq[c_index]);
                fprintf(fq, "%d\n", motor_freq);
            }

        }
    }

    fclose(fd);    

    return NULL;
}


int main(int argc, char *argv[])
{
    FILE * fd = fopen("threadlog.txt","w");

    pthread_t gyropidT;
    pthread_t gpsT;
    pthread_t cvT;
    pthread_t cpu_dvfsT;
    pthread_t gpu_dvfsT;
    int  ret;

    pthread_barrier_init(&barrier, NULL, 5);

    printf("main_thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(), pthread_self());

    ret = pthread_create(&cpu_dvfsT, NULL, cpu_dvfs, NULL);
    fprintf(fd,"cpu_dvfsT creat: %d\n", time(NULL));
    if (ret == -1) {
        perror("cannot creat cpu_dvfs thread");
        return -1;
    }

    ret = pthread_create(&gpu_dvfsT, NULL, gpu_dvfs, NULL);
    fprintf(fd,"gpu_dvfsT creat: %d\n", time(NULL));
    if (ret == -1) {
        perror("cannot creat gpu_dvfs thread");
        return -1;
    }

    ret = pthread_create(&gyropidT, NULL, gyropid, NULL);
    fprintf(fd,"gyropidT creat: %d\n", time(NULL));
    if (ret == -1) {
        perror("cannot creat gyro thread");
        return -1;
    }

    ret = pthread_create(&gpsT, NULL, gps, NULL);
    fprintf(fd,"gpsT creat: %d\n", time(NULL));
    if (ret == -1) {
        perror("cannot creat gps thread");
        return -1;
    }

    ret = pthread_create(&cvT, NULL, cv, NULL);
    fprintf(fd,"cvT creat: %d\n", time(NULL));
    if (ret == -1) {
        perror("cannot creat cv thread");
        return -1;
    }

//    printf("\n**********************\n       main_waiting       \n**********************\n");
//    pthread_barrier_wait(&barrier);
//    pthread_barrier_destroy(&barrier);

    if (pthread_join(cpu_dvfsT, NULL) != 0) {
        perror("call cpu_dvfs pthread_join function fail");
        return -1;
    } else {
        printf("cpu_dvfsT join success");
    fprintf(fd,"cpu_dvfsT join: %d\n", time(NULL));
    }

    if (pthread_join(gpu_dvfsT, NULL) != 0) {
        perror("call gpu_dvfs pthread_join function fail");
        return -1;
    } else {
        printf("gpu_dvfsT join success");
    fprintf(fd,"gpu_dvfsT join: %d\n", time(NULL));
    }


    if (pthread_join(gyropidT, NULL) != 0) {
        perror("call pthread_join function fail");
        return -1;
    } else {
        printf("gyropidT join success");
    fprintf(fd,"gyropidT join: %d\n", time(NULL));
    }


    if (pthread_join(gpsT, NULL) != 0) {
        perror("call pthread_join function fail");
        return -1;
    } else {
        printf("gpsT join success");
    fprintf(fd,"gpsT join: %d\n", time(NULL));
    }


    if (pthread_join(cvT, NULL) != 0) {
         perror("call pthread_join function fail");
        return -1;
    } else {
        printf("cvT join success");
        fprintf(fd,"cvT join: %d\n", time(NULL));
    }


//    motor_stop();

    fclose(fd);
    return 0;
}


