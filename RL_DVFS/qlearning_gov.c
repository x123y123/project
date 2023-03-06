#include "qlearning_gov.h"
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pmu_monitor.h"

#define NUM_STATES 4
#define NUM_ACTIONS 25
#define NUM_FREQS 25
#define NUM_CPI_STATES 10

#define ALPHA 0.1
#define GAMMA 0.9
#define MAX_EPISODE 1000
#define MAX_CPUFREQ 1907200
#define MIN_CPUFREQ 115200
#define MAX_CPU_VOLTS 5.5  // need to check!!!

#define TEMP_LIMIT 46
// let CPI split to 10 slices and 10 is NUM_CPI_STATES
#define CPI_FAST 4
#define CPI_SLOW 14

#define RHO 5
#define Q_RANDOM

#define max(x, y)            \
    ({                       \
        typeof(x) _x = (x);  \
        typeof(y) _y = (y);  \
        (void) (&_x == &_y); \
        _x > _y ? _x : _y;   \
    })

// Q-table
float Q[NUM_CPI_STATES][NUM_ACTIONS] = {0.0f};


// cpufreq-table
int cpufreq[NUM_FREQS] = {115200,  192000,  268800,  345600,  422400,
                          499200,  576000,  652800,  729600,  806400,
                          883200,  960000,  1036800, 1113600, 1190400,
                          1267200, 1344000, 1420800, 1497600, 1574400,
                          1651200, 1728000, 1804800, 1881600, 1907200};
// double IPS_MAX = IPC_MAX * (cpufreq[NUM_FREQS - 1] * 1000.0);

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

float get_cpu_power(void)
{
    int power_mW;
    FILE *cpu_power = fopen(
        "/sys/bus/i2c/drivers/ina3221x/7-0040/iio\:device0/in_power0_input",
        "r");
    fscanf(cpu_power, "%d", &power_mW);
    float power_value = power_mW / 1000;
    printf("cpu_power: %f\n", power_value);
    fclose(cpu_power);

    return power_value;
}

float get_cpu_volts(void)
{
    int vol_mV;
    FILE *cpu_volts = fopen(
        "/sys/bus/i2c/drivers/ina3221x/7-0040/iio\:device0/in_voltage0_input",
        "r");
    fscanf(cpu_volts, "%d", &vol_mV);
    float vol_value = vol_mV / 1000;
    printf("cpu_volts: %f\n", vol_value);
    fclose(cpu_volts);

    return vol_value;
}

float get_cpu_temp(void)
{
    int tmp;
    FILE *cpu_temp =
        fopen("/sys/devices/virtual/thermal/thermal_zone0/temp", "r");
    fscanf(cpu_temp, "%d", &tmp);
    float temp_value = tmp / 1000;
    printf("cpu_temp: %f\n", temp_value);
    fclose(cpu_temp);

    return temp_value;
}

float get_gpu_temp(void)
{
    int tmp;
    FILE *gpu_temp =
        fopen("/sys/devices/virtual/thermal/thermal_zone3/temp", "r");
    fscanf(gpu_temp, "%d", &tmp);
    float temp_value = tmp / 1000;
    printf("gpu_temp: %f\n", temp_value);
    fclose(gpu_temp);

    return temp_value;
}

void set_cpufreq(int freq)
{
    FILE *set_cfreq =
        fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", "w");
    fprintf(set_cfreq, "%d", freq);
    fflush(stdout);
    printf("set_freq: %d\n", freq);
    fclose(set_cfreq);
}

float *get_raw_state()
{
    float *state_buff = (float *) malloc(sizeof(float) * NUM_STATES);
    printf("\n----------------\n");
    state_buff[0] = get_cpu_temp();
    state_buff[1] = CPI;
    state_buff[2] = get_cpu_power();
    state_buff[3] = get_cpu_volts();
    printf("----------------\n");

    return state_buff;
}

void update_q_off_policy(float *last_state,
                         int last_action_index,
                         float *state,
                         float reward)
{
    float best_next_return = 0.0f;
    float total_return = 0.0f;
    printf("----------------\n");
    printf("Update_q_off_policy:\n");
    int cur_states_index = state[1] - 4;
    for (int i = 0; i < NUM_ACTIONS; i++) {
        if (max(best_next_return, Q[cur_states_index][i]) ==
            Q[cur_states_index][i])
            best_next_return = Q[cur_states_index][i];
        printf("best_next_return: %f\n", best_next_return);
    }

    printf("----------------\n");
    total_return = reward + GAMMA * best_next_return;

    // update last_state estimate
    float old_value[NUM_CPI_STATES][NUM_ACTIONS];
    for (int i = 0; i < NUM_CPI_STATES; i++) {
        for (int j = 0; j < NUM_ACTIONS; j++) {
            old_value[i][j] = Q[i][j];
        }
    }
    // update q-table, by TD-method
    // for (int j = 0; j < NUM_ACTIONS; j++) {
    Q[cur_states_index][last_action_index] =
        old_value[cur_states_index][last_action_index] +
        ALPHA * (total_return - old_value[cur_states_index][last_action_index]);

    //}
}

float reward_func(float *states, int new_freq)
{
    float reward;
    float cpu_volts, cpu_freq, cpu_temp, cpu_power, IPC, gpu_temp;
    float temp_var, thermal_penalty, cur_power, max_power;
    cpu_temp = states[0];
    CPI = states[1];
    cpu_power = states[2];
    cpu_volts = states[3];

    cur_power = new_freq * pow(cpu_volts, 2);  // dynamic power
    max_power = cpufreq[NUM_FREQS - 1] * pow(MAX_CPU_VOLTS, 2);
    temp_var = cpu_temp - TEMP_LIMIT;

    // reward judged by throughput because it can let system consider more
    // performance aware
    reward += (CPI_SLOW / CPI);
    if (temp_var > 0) {  // overheat
        thermal_penalty += (RHO * temp_var * (cur_power / max_power));
        reward -= thermal_penalty;
    }
    return reward;
}

void q_learning()
{
    int episode = 0;
    float reward = 0.0f;
    float last_states[NUM_STATES] = {0.0f};
    float *states;
    int last_action_index = 0;
    int best_action_index = 0;
    srand(time(NULL));  // set rand seed
#ifdef Q_RANDOM
    // Initial random value to Q-table
    for (int i = 0; i < NUM_CPI_STATES; i++) {
        for (int j = 0; j < NUM_ACTIONS; j++) {
            Q[i][j] = (float) rand() / (RAND_MAX + 1.0);
        }
    }
#endif

    while (episode < MAX_EPISODE) {
        sleep(1);
        states = get_raw_state();
        sleep(1);

        // Update state-action-reward trace
        if (last_states[0] == 0 || last_states[1] == 0 || last_states[2] == 0 ||
            last_states[3] == 0) {
            reward = reward_func(states, cpufreq[0]);
        }
        reward = reward_func(states, cpufreq[last_action_index]);
        update_q_off_policy(last_states, last_action_index, states, reward);
        for (int i = 0; i < NUM_CPI_STATES; i++) {
            for (int j = 0; j < NUM_ACTIONS; j++) {
                printf("%f ", Q[i][j]);
            }
            printf("\n");
        }

        if (rand() > episode) {
            int nice_index = 0;
            float best_next_return = 0.0f;
            int cur_states_index = states[1] - 4;
            for (int i = 0; i < NUM_ACTIONS; i++) {
                if (max(best_next_return, Q[cur_states_index][i]) ==
                    Q[cur_states_index][i]) {
                    best_next_return = Q[cur_states_index][i];
                    nice_index = i;
                }
            }
            best_action_index = nice_index;
        } else
            best_action_index = (rand() % (NUM_ACTIONS + 1));
        printf("best_action_index: %d\n", best_action_index);
        /*
                // Penalize trying to go out of bounds, since there is no
           utility in doing so. if (best_action_index < 0 || best_action_index >
           NUM_ACTIONS) { if (best_action_index < 0) { reward = reward - (500 *
           (NUM_ACTIONS - best_action_index));
                    }
                    reward = reward - (500 * (best_action_index - NUM_ACTIONS));
                }
        */
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

        printf("episode: %d\n", episode);
        episode++;
    }
    free(states);
}

int main()
{
    // create a perf_event thread
    pthread_t perf_event;
    int ret;

    ret = pthread_create(&perf_event, NULL, pmu_monitor, NULL);
    if (ret == -1) {
        perror("cannot creat perf_event thread");
        return -1;
    }


    // training
    q_learning();

    // thread join
    if (pthread_join(perf_event, NULL) != 0) {
        perror("call perf_event pthread_join function fail");
        return -1;
    } else {
        printf("perf_event join success");
    }
}
