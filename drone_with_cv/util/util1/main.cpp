/* This code will execute 4~5 threads to run the drone programming */

#include "main.h"

#define default_port "ttyUSB0"
#define gettid() syscall(__NR_gettid)
#define test 
#define tsec                60
#define available_gpufreq   13
#define ms_buf_size         100

#define setaffinity
//#define DVFS
//#define rm_loop

pthread_mutex_t lock       = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flock      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;
int loop_stats_gyropid = 0;
int g_index = 12;    //numbers of available_gfreq
int motor_freq = 0;  //set motor Hz
int dvfs_cnt = 0;    //count dvfs adjustment times
int delay = 0;

int motor_speed_buffer[ms_buf_size] = {0};
int ms_buf_index = 0;

struct CFG_format
{
    int gpufreq;
    int cpufreq;
}CFG_format[13];

void motor_set_val(int number, int motor_val)
{
    int i;

    //Add all motor same value
    if (number == 6) {
        int pwm;
        cout << "mode 6" << endl;

        for (i = 0; i < 4; i++) {
            pwm = motor.getPWM(motor_no[i]);
            motor_set_val(i+1, pwm + motor_val);
        }
    }
    //Set all motors
    else if (number == 5) {
        cout << "mode 5" << endl;
        
        for (i = 0; i < 4; i++) {
            motor_set_val(i+1, motor_val);
        }
    }
    //Set one motor
    else if (number >= 1 && number <= 4) { 
        motor_val = (motor_val > motor_start_val[number-1]) ? motor_val : motor_start_val[number-1];
        motor_val = (motor_val > motor_start_val[number-1]+pulse_range) ? motor_start_val[number-1]+pulse_range : motor_val;
        motor.setPWM(motor_no[number-1], motor_val);
//        cout << "set motor " << number << " as " << motor_val << endl;
    }
}

void motor_stop() 
{
    for (int i = 1; i <= motor_num; i++)
        motor_set_val(i, min_pulse);
}

void control()
{
    int i;
    control_core[2].pid.Calculate(imusol[2], control_core[2].target);
    control_core[5].pid.Calculate(imusol_gyr[2], control_core[2].pid.output);

    for (i = 0; i < 4; i++) {
        //float speed = motor.getPWM(motor_no[i]) + control_core[5].pid.output * control_core[5].weight[i];
        float speed = motor_hold[i] + control_core[5].pid.output * control_core[2].weight[i];
        //motor_set_val(i+1, speed);
        if (i == 1)
            motor_speed_buffer[ms_buf_index] = speed;  
    }

    ms_buf_index++;
    int buf_end = ms_buf_size - 1;
    if (ms_buf_index == buf_end)
        ms_buf_index = 0;
}

void control_init()
{
    int i;

    control_core[0].pid.IsAngle();
    control_core[1].pid.IsAngle();
    control_core[2].pid.IsAngle();

    control_core[0].target = 0.0f;
    control_core[1].target = 0.0f;
    control_core[2].target = -74.0f;

    control_core[0].weight[0] = -1.0f;
    control_core[0].weight[1] = -1.0f;
    control_core[1].weight[0] = -1.0f;
    control_core[1].weight[3] = -1.0f;
    control_core[2].weight[1] = -1.0f;
    control_core[2].weight[3] = -1.0f;

    for (i = 0; i < 4; i++) {                
        motor_set_val(i+1, motor_start_val[i]);     
    }                                                
    for (i = 3; i < 6; i++)
        control_core[i].pid.Setpid("/home/uav/code/controller/deps/pid_controller/init_motor.txt\0");
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
#ifndef rm_loop
    while(time(NULL) < endwait) {
#endif
        pthread_mutex_lock(&stats_lock);
        loop_stats_gyropid = 1;
        pthread_mutex_unlock(&stats_lock);
        
        times_cnt ++;
#ifndef rm_loop
        if (times_cnt != 5000) 
            continue;
        else {
#endif
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
#ifndef rm_loop        
        }
#endif
        pthread_mutex_lock(&stats_lock);
        loop_stats_gyropid = 0;
        pthread_mutex_unlock(&stats_lock);
#ifndef rm_loop   
    }
#endif
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

    int delay_time_t, delay_time_s;
    time_t endwait, stop_time = 60;
    endwait = time(NULL) + stop_time; 
#ifndef rm_loop    
    while(time(NULL) < endwait) {
#endif
        delay ++;
        delay_time_t = delay % 10;
        delay_time_s = delay % 17;
        if (loop_stats_gyropid == 0 && delay_time_t == 0 && delay_time_s == 0) {
            dvfs_cnt ++;
            if (signal(SIGUSR1, sig_usr) == SIG_ERR)
                perror("can't catch SIGUSR1\n");
            else if (signal(SIGUSR2, sig_usr) == SIG_ERR)
                perror("can't catch SIGUSR2\n");
        }
#ifndef rm_loop    
    }
#endif

    return NULL;
}
#endif

int main(int argc, char **argv)
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
            CFG_format[i].cpufreq = 345600; 
        }
        else if (1 <= i && i < 5) {
            CFG_format[i].cpufreq = 422400; 
        }
        else if (i >= 5) {
            CFG_format[i].cpufreq = 499200; 
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
#ifndef rm_loop    
    while (time(NULL) < endwait) {
#endif
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
#ifndef rm_loop    
    }
#endif
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

    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&flock);
    pthread_mutex_destroy(&stats_lock);
    printf("\nDVFS cnt: %d\ndelay_cnt: %d\n", dvfs_cnt, delay);

    FILE *ms = fopen("m2c.txt","w");
    for (int i = 0; i < 100; i++) 
        fprintf(ms, "%d ",motor_speed_buffer[i]);
    fclose(ms);
    fclose(fd);
    return 0;
}


