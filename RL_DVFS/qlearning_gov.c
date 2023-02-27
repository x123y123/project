#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NUM_STATES      5
#define NUM_ACTIONS     25
#define NUM_FREQS       25
#define ALPHA           0.1
#define GAMMA           0.9
#define MAX_EPISODE     1000
#define MAX_CPUFREQ     1907200
#define MIN_CPUFREQ     115200
#define MAX_CPU_VOLTS   5.5 //need to check!!!

#define TEMP_LIMIT      46
#define IPC_MAX         4
#define RHO             5

#define max(a, b) ((a > b) ? a : b)

// Q-table
double Q[NUM_STATES][NUM_ACTIONS];

// cpufreq-table
int cpufreq[NUM_FREQS] = {115200, 192000, 268800, 345600, 422400, 499200, 576000, 652800, 729600, 806400, 883200, 960000, 1036800, 1113600, 1190400, 1267200, 1344000, 1420800, 1497600, 1574400, 1651200, 1728000, 1804800, 1881600, 1907200};
//double IPS_MAX = IPC_MAX * (cpufreq[NUM_FREQS - 1] * 1000.0);

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

float get_cpu_volts(void)
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

void set_cpufreq(int freq)
{
    fprintf(set_cfreq, "%d", freq);
    fflush(freq);
}

float* get_raw_state()
{
    float state_buff[NUM_STATES];
    state_buff[0] = get_cpu_temp();
    state_buff[1] = get_gpu_temp();
    state_buff[2] = get_cpu_power();
    state_buff[3] = get_cpu_volts();
    state_buff[4] = get_IPC();

    return state_buff;
}

double* update_q_off_policy(float* last_state, double last_actions, float* state, float reward) 
{
    float best_next_return = 0; 
    float total_return;
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
    // update q-table, by TD-method
    for (int i = 0; i < NUM_STATES; i++) {
        for (int j = 0; j < NUM_ACTIONS; j++) {
           Q[i][j] = old_value[i][j] + ALPHA * (total_return - old_value[i][j]);
        }
    }
    return Q;
}

float reward_func(float* states, double new_freq)
{
    float reward;
    float cpu_volts, cpu_freq, cpu_temp, cpu_power, IPC, gpu_temp;
    float temp_var, thermal_penalty, cur_power, max_power;
    cpu_temp = states[0];
    gpu_temp = states[1];
    cpu_power = states[2];
    cpu_volts = states[3];
    IPC = states[4];

    cur_power = new_freq * pow(cpu_volts, 2);  // dynamic power
    max_power = cpufreq[NUM_FREQS - 1] * pow(MAX_CPU_VOLTS, 2);
    temp_var = cpu_temp - TEMP_LIMIT;
    
    // reward judged by throughput because it can let system consider more performance aware
    reward += (IPC / IPC_MAX);
    if (temp_var > 0) { // overheat
        thermal_penalty += (RHO * temp_var * (cur_power / max_power));
        reward -= thermal_penalty;
    }

    return reward;
}

void q_learning()
{
    int episode = 0;
    float reward = 0;
    float last_states[NUM_STATES] = {0};
    float* states = NULL;
    int last_action_index = 0;
    int best_action_index = 0;
    srand(time(NULL)); // set rand seed

    while (episode < MAX_EPISODE) {
        states = get_raw_state();
        double best_next_return = 0; 
        
        
        // Update state-action-reward trace
        if (last_states) {
            reward = reward_func(states, cpufreq[best_action_index]);
            double* tmp = update_q_off_policy(last_states, last_action_index, states, reward);
            for (int i = 0; i < NUM_STATES; i++) {
                for (int j = 0; j < NUM_ACTIONS; j++) {
                    printf("%d ", tmp + i * NUM_STATES + NUM_ACTIONS);
                }
                printf("\n");
            }
        }
        else {
            reward = reward_func(states, cpufreq[best_action_index]);
        }

        if (rand() < episode) {
            best_action_index = rand() % NUM_ACTIONS;
        }
        else {
            for (int i = 0; i < NUM_STATES; i++) {
                if (max(best_next_return, Q[i][0]) == Q[i][0])
                    best_next_return = i;
            }
            best_action_index = best_next_return;
        }
        // Penalize trying to go out of bounds, since there is no utility in doing so.
        if (best_action_index < 0 || best_action_index > NUM_ACTIONS) {
            if (best_action_index < 0) {
                reward = reward - (500 * (NUM_ACTIONS - best_action_index));
            }
            reward = reward - (500 * (best_action_index - NUM_ACTIONS)); 
        }

        /*
            _        _                     _   _             
           | |_ __ _| | _____    __ _  ___| |_(_) ___  _ __  
           | __/ _` | |/ / _ \  / _` |/ __| __| |/ _ \| '_ \ 
           | || (_| |   <  __/ | (_| | (__| |_| | (_) | | | |
            \__\__,_|_|\_\___|  \__,_|\___|\__|_|\___/|_| |_|
                                                                     
        */
        // Take action
        set_cpufreq(cpufreq[best_action_index]);    
        
        for (int i = 0; i < NUM_STATES; i++)
            last_states[i] = states[i];
        last_action_index = best_action_index;
        
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
