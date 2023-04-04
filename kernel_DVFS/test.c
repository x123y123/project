#include <linux/init.h>
#include "sched/cpufreq.h"
#include "slab.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cpufreq.h>

static void cpufreq_gov_performance_limits(struct cpufreq_policy *policy)
{
        pr_debug("setting to %u kHz\n", policy->max);
            __cpufreq_driver_target(policy, policy->max, CPUFREQ_RELATION_H);
}

static struct cpufreq_governor cpufreq_gov_performance = {
        .name       = "performance",
            .owner      = THIS_MODULE,
                .flags      = CPUFREQ_GOV_STRICT_TARGET,
                    .limits     = cpufreq_gov_performance_limits,
};

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_PERFORMANCE
struct cpufreq_governor *cpufreq_default_governor(void)
{
        return &cpufreq_gov_performance;
}
#endif
#ifndef CONFIG_CPU_FREQ_GOV_PERFORMANCE_MODULE
struct cpufreq_governor *cpufreq_fallback_governor(void)
{
        return &cpufreq_gov_performance;
}
#endif

MODULE_AUTHOR("Dominik Brodowski <linux@brodo.de>");
MODULE_DESCRIPTION("CPUfreq policy governor 'performance'");
MODULE_LICENSE("GPL");

cpufreq_governor_init(cpufreq_gov_performance);
cpufreq_governor_exit(cpufreq_gov_performance);
