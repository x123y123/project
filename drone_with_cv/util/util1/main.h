#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

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
#include "PCA9685.h"
#include "mygyro.h"
#include "packet.h"
#include "imu_data_decode.h"
#include "pid_controller.h"

#define bus 1
#define address 0x40
#define frequency 400       //pca_freq 24 ~ 1526 Hz 
#define min_pulse 1900
#define pulse_range 700         // new
#define motor_acc 3
#define motor_num 4
#define default_port "ttyUSB0"
#define mult 0.1

// ID table
#define Height 3
#define Yaw    2
#define Pitch  1
#define Row    0


using namespace std;

typedef struct gyro_to_pid_t {
    float target;
    float weight[4] = {1, 1, 1, 1};
    PID   pid;
}gyro_to_pid_t;

void motor_set_val(int motor_no, int motor_val);
void motor_stop();
void control();
void control_init();


int motor_no[6]   = {1, 2, 11, 12, 5, 6};
int motor_start_val[4]  = {1900, 2330, 1900, 1950};
int motor_hold[4] = {2075, 2475, 2050, 2105};
float m2c_regression[3] = {-0.0850149615f, 0.0000177372199f, 101.92377794117651f};

PCA9685 motor = PCA9685(bus, address);   //PCA9685 create

gyro_to_pid_t control_core[6];

extern float imusol[];
extern float imusol_gyr[];
extern int loop_stats_gps;
extern double perf_gps;
