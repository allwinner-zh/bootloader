#include "OSAL_Power.h"

#ifndef __OSAL_POWER_MASK__

int OSAL_Power_Enable(char *name)
{
	char power_name[20], pmu_type[20], vol_type[20];
	char *temp, *pmu;

	strcpy(power_name, name);
	temp = power_name;

	pmu = strsep(&temp, "_");
	strcpy(pmu_type, pmu);
	strcpy(vol_type, temp);

#if defined(CONFIG_ARCH_SUN9IW1P1)
	if(!strcmp("axp15", pmu_type))
		strcpy(pmu_type, "axp806");
	else if(!strcmp("axp22", pmu_type))
		strcpy(pmu_type, "axp809");
	else
		__wrn("unknown pmu type");
#else
#if 0
	if(!strcmp("dc1sw", vol_type)) {
		__wrn("enable dcdc1");
		axp_set_supply_status_byname("axp22", "dcdc1", 0, 1);
	}
#endif
#endif

	if(!strcmp("ldoio0", vol_type))
		strcpy(vol_type, "gpio0ldo");
	else if(!strcmp("ldoio1", vol_type))
		strcpy(vol_type, "gpio1ldo");

	__wrn("<%s, %s>\n", pmu_type, vol_type);
	axp_set_supply_status_byname(pmu_type, vol_type, 0, 1);

	return 0;
}

int OSAL_Power_Disable(char *name)
{
	char power_name[20], pmu_type[20], vol_type[20];
	char *temp, *pmu;

	strcpy(power_name, name);
	temp = power_name;

	pmu = strsep(&temp, "_");
	strcpy(pmu_type, pmu);
	strcpy(vol_type, temp);

#if defined(CONFIG_ARCH_SUN9IW1P1)
	if(!strcmp("axp15", pmu_type))
		strcpy(pmu_type, "axp806");
	else if(!strcmp("axp22", pmu_type))
		strcpy(pmu_type, "axp809");
	else
		__wrn("unknown pmu type");
#else
#endif

	if(!strcmp("ldoio0", vol_type))
		strcpy(vol_type, "gpio0ldo");
	else if(!strcmp("ldoio1", vol_type))
		strcpy(vol_type, "gpio1ldo");

	__wrn("<%s, %s>\n", pmu_type, vol_type);
	axp_set_supply_status_byname(pmu_type, vol_type, 0, 0);

#if defined(CONFIG_ARCH_SUN8IW5P1)
#if 0
	if(!strcmp("dc1sw", vol_type))
		axp_set_supply_status_byname("axp22", "dcdc1", 0 , 0);
#endif
#endif

	return 0;
}

#else

int OSAL_Power_Enable(char *name)
{
	int ret = 0;

	return ret;
}

int OSAL_Power_Disable(char *name)
{
	int ret = 0;

	return ret;
}

#endif
