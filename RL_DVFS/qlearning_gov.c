#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_STATES      3
#define NUM_ACTIONS     25
#define NUM_FREQS       25
#define ALPHA           0.1
#define GAMMA           0.9
#define MAX_EPISODE     1000
#define MAX_CPUFREQ     1907200
#define MIN_CPUFREQ     115200
#define TEMP_LIMIT      46

#define max(a, b) ((a > b) ? a : b)

// Q-table
double Q[NUM_STATES][NUM_ACTIONS];
// cpufreq-table
int cpufreq[NUM_FREQS] = {115200, 192000, 268800, 345600, 422400, 499200, 576000, 652800, 729600, 806400, 883200, 960000, 1036800, 1113600, 1190400, 1267200, 1344000, 1420800, 1497600, 1574400, 1651200, 1728000, 1804800, 1881600, 1907200};

/*
/sys/devices/virtual/thermal/thermal_zone<X>/temp
0: BCPU-therm
1: MCPU-therm
2: GPU-therm
3: PLL-therm
4: AO-therm
5: Tboard_tegra
6: Tdiode_tegra
7: PMIC-Die
8: thermal-fan-est
*/

FILE* cpu_temp = fopen("/sys/deevices/virtual/thermal/thermal_zone0/tmp","r");
FILE* gpu_temp = fopen("/sys/deevices/virtual/thermal/thermal_zone3/tmp","r");
FILE* set_cfreq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed","w");
FILE* cpu_power = fopen("/sys/bus/i2c/drivers/ina3221x/7-0040/iio\:device0/in_power0_input","r");
FILE* cpu_vol = fopen("/sys/bus/i2c/drivers/ina3221x/7-0040/iio\:device0/in_voltage0_input","r");

float get_cpu_power(void)
{
    int power_mW;
    fscanf(cpu_power, "&d", &power_mW);
    float power_value = power_mW / 1000;

    return power_value;
}

float get_cpu_vol(void)
{
    int vol_mV;
    fscanf(cpu_vol, "&d", &vol_mV);
    float vol_value = vol_mV / 1000;

    return vol_value;
}

float get_cpu_temp(void)
{
    int tmp;
    fscanf(cpu_temp, "&d", &tmp);
    float temp_value = tmp / 1000;

    return temp_value;
}

float get_gpu_temp(void)
{
    int tmp;
    fscanf(gpu_temp, "&d", &tmp);
    float temp_value = tmp / 1000;

    return temp_value;
}

void set_cpufreq(int freq_val)
{
    fprintf(set_cfreq, "%d", freq_val);
    fflush(freq_val);
}

float* get_raw_state()
{
    float state_buff[NUM_STATES];
    float cpu_temp = get_cpu_temp();
    float gpu_temp = get_gpu_temp();
    float cpu_power = get_cpu_power();
    float cpu_vol = get_cpu_vol();
    float IOC = get_IOC();

    return state_buff;
}

double* update_q_off_policy(int last_state, int last_actions, int state, double reward) 
{
    double best_next_return = 0; 
    double total_return;
    for (int i = 0; i < NUM_STATES; i++) {
        best_next_return = max(best_next_return, Q[i][0]);
    }
    total_return = reward + GAMMA * best_next_return;

    // update last_state estimate
    double old_value[NUM_STATES][NUM_ACTIONS];
    for (int i = 0; i < NUM_STATES; i++) {
        for (int j = 0; j < NUM_ACTIONS; j++) {
            old_value[i][j] = Q[i][j];   
        }
    }
    // update q-table
    for (int i = 0; i < NUM_STATES; i++) {
        for (int j = 0; j < NUM_ACTIONS; j++) {
           Q[i][j] = old_value[i][j] + ALPHA * (total_return - old_value[i][j]);
        }
    }
    return Q;
}
/*
float reward_func()
{
    int thermal_penalty = 10;
    //IPC
    int reward += get_ipc();
    if (get_temp() > TEMP_LIMIT) {
        int reward -= thermal_penalty;        
    }

    return IPC - power_penalty - thermal_penalty;
}
*/
void q_learning()
{
    int episode = 0;
    int last_state = 0;
    int last_action = 0;
    int reward = 0;
    srand(time(NULL)); // set rand seed

    while (episode < MAX_EPISODE) {
        int* state = get_raw_state();
        double best_next_return = 0; 

        // Penalize trying to go out of bounds, since there is no utility in doing so.
        if (action < MIN_CPUFREQ || action > MAX_CPUFREQ) {
            if (action < MIN_CPUFREQ) {
                reward = reward - (500 * (MIN_CPUFREQ - action);
            }
            else {
                reward = reward - (500 * (action - MAX_CPUFREQ); 
            }
        }
        
        // Update state-action-reward trace
        if (last_state) {
            reward = reward_func(state);
            double* tmp = update_q_off_policy(last_state, last_action, state, reward);
            for (int i = 0; i < NUM_STATES; i++) {
                for (int j = 0; j < NUM_ACTIONS; j++) {
                    printf("%d ", tmp + i * NUM_STATES + NUM_ACTIONS);
                }
                printf("\n");
            }
        }
        else {
            reward = reward_func(state);
        }

        if (rand() < episode) {
            best_aciton = rand() % NUM_ACTIONS;
        }
        else {
            for (int i = 0; i < NUM_STATES; i++) {
                best_next_return = max(best_next_return, Q[i][0]);
            }
            best_action = best_next_return;
        }
        /*
            _        _                     _   _             
           | |_ __ _| | _____    __ _  ___| |_(_) ___  _ __  
           | __/ _` | |/ / _ \  / _` |/ __| __| |/ _ \| '_ \ 
           | || (_| |   <  __/ | (_| | (__| |_| | (_) | | | |
            \__\__,_|_|\_\___|  \__,_|\___|\__|_|\___/|_| |_|
                                                                     
        */
        // Take action
        set_cpufreq(cpufreq[best_action]);    
        
        last_state = state;
        last_action = best_action;
        
        episode++;
    }
}

int main()
{
    q_learning();
    fclose(set_cfreq);
    fclose(cpu_temp);
    fclose(gpu_temp);
    fclose(cpu_vol);
    fclose(cpu_power);
}
