#!/bin/bash
echo userspace | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

