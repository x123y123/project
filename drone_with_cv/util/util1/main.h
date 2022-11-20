#define bus 1
#define address 0x40
#define frequency 400       //pca_freq
#define min_pulse 1900
#define max_pulse 2700
#define motor_acc 3
#define motor_num 4
#define default_port "ttyUSB0"
#define mult 0.1

// ID table
#define Height 3
#define Yaw    2
#define Pitch  1
#define Row    0


#include "PCA9685.h"
#include "mygyro.h"
#include "packet.h"
#include "imu_data_decode.h"
#include "pid_controller.h"

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


int motor_no[6] = {1, 2, 11, 12, 5, 6};
int motor_val[4] = {1950, 2400, 1950, 2000};
float transferFuc[2][4] = {{0.001145, 0.001509, 0.001444, 0.001405},
                           {-2.20842, -3.54262, -2.79794, -2.76044}};

PCA9685 motor = PCA9685(bus, address);   //PCA9685 create

gyro_to_pid_t control_core[4];

extern float imusol[];
extern int loop_stats_gps;
extern double perf_gps;
