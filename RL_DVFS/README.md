# DVFS by RL(Q-Learning)

## Device
* Nvidia Jetson NX

## How to run this repository?
```shell
$ cd RL_DVFS
$ make
$ sudo bash ./setup.sh
```
## Directory Structure
    ├── train.sh                # Run train() mode in qlearning_gov.c
    ├── run.sh                  # Run run()  mode in qlearning_gov.c
    ├── qlearning_gov.c
    ├── qlearning_gov.h
    ├── pmu_monitor.c           # Monitor CPI in pmu
    ├─- pmu_monitor.h          
    └── script                  # Draw the diagrams
        ├── draw.sh
        ├── state_step.gp
        └── action_step.gp

## Q-Learning model

### Action Space and State Space
* Action space: Using the whole frequency zone in NX.
```c
// You can see available frequency in /sys/device/system/cpu/cpu*/cpufreq/scaling_avaliable_frequency
int cpufreq[25] = {115200,  192000,  268800,  345600,  422400,
                   499200,  576000,  652800,  729600,  806400,
                   883200,  960000,  1036800, 1113600, 1190400,
                   1267200, 1344000, 1420800, 1497600, 1574400,
                   1651200, 1728000, 1804800, 1881600, 1907200};
```
* State space: We use ten CPI levels to represent our stats.
```c
#define CPI_FAST  4
#define CPI_SLOW  14
// split 4~14 to 10 levels
```
### Reward Function

### Update Q off-policy(TD-learning)
$Q(s,a) \leftarrow Q(s,a)+ \eta[r+ \gamma \max{a'} Q(s',a')-Q(s,a)]$

