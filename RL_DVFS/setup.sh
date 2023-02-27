#!/bin/bash
echo userspace | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
echo 1907200 | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq

