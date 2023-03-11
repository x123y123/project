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
