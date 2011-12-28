/*
 * linux/arch/arm/mach-s5pv310/charger-slp7.c
 * COPYRIGHT(C) 2011
 * MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * SLP7(NURI) Board Charger Support with Charger-Manager Framework
 *
 */

#include <asm/io.h>

#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/platform_data/ntc_thermistor.h>
#include <linux/regulator/consumer.h>
#include <linux/power/charger-manager.h>
#include <linux/mfd/max8997.h>
#include <linux/mfd/max8997-private.h>

#include <mach/regs-pmu.h>
#include <mach/irqs.h>

#include <plat/adc-ntc.h>

/* Temperatures in milli-centigrade */
#define SECBATTSPEC_TEMP_HIGH		(63 * 1000)
#define SECBATTSPEC_TEMP_HIGH_REC	(58 * 1000)
#define SECBATTSPEC_TEMP_LOW		(-5 * 1000)
#define SECBATTSPEC_TEMP_LOW_REC	(0 * 1000)

static bool s3c_wksrc_rtc_alarm(void)
{
	u32 reg = __raw_readl(S5P_WAKEUP_STAT) & S5P_WAKEUP_STAT_WKSRC_MASK;

	if ((reg & S5P_WAKEUP_STAT_RTCALARM) &&
	    !(reg & ~S5P_WAKEUP_STAT_RTCALARM))
		return true; /* yes, it is */

	return false;
}

enum temp_stat { TEMP_OK = 0, TEMP_HOT = 1, TEMP_COLD = -1 };

static int ntc_thermistor_ck(int *mC)
{
	static enum temp_stat state = TEMP_OK;

	read_s3c_adc_ntc(mC);
	switch (state) {
	case TEMP_OK:
		if (*mC >= SECBATTSPEC_TEMP_HIGH)
			state = TEMP_HOT;
		else if (*mC <= SECBATTSPEC_TEMP_LOW)
			state = TEMP_COLD;
		break;
	case TEMP_HOT:
		if (*mC <= SECBATTSPEC_TEMP_LOW)
			state = TEMP_COLD;
		else if (*mC < SECBATTSPEC_TEMP_HIGH_REC)
			state = TEMP_OK;
		break;
	case TEMP_COLD:
		if (*mC >= SECBATTSPEC_TEMP_HIGH)
			state = TEMP_HOT;
		else if (*mC > SECBATTSPEC_TEMP_LOW_REC)
			state = TEMP_OK;
		break;
	default:
		pr_err("%s has invalid state %d\n", __func__, state);
	}

	return state;
}

static char *nuri_charger_stats[] = { "max8997_pmic", "max8903_charger", NULL };
static struct regulator_bulk_data nuri_chargers[] = {
	{ .supply = "vinchg1", },
	{ .supply = "vinchg2", },
};

#define _MAX8997_PMICIRQ	0x4000000
#define _MAX17042_IRQ		0x2000000
#define MAX8997_PMICIRQ(x)	(_MAX8997_PMICIRQ | (MAX8997_PMICIRQ_ ## x))
#define MAX17042_IRQ(x)		(_MAX17042_IRQ | (MAX17042_IRQ_ ## x))
#define IRQ_OFFSET(x)		(0x0FFFFFF & (x))
static struct charger_irq nuri_chg_irqs[] = {
	{ MAX8997_PMICIRQ(TOPOFFR), 0, CM_IRQ_BATT_FULL, true, "Batt Full" },
	{ MAX8997_PMICIRQ(CHGRSTF), 0, CM_IRQ_CHG_START_STOP,
	  true, "CHG STOP" },
	{ MAX8997_PMICIRQ(MBCHGTMEXPD), 0, CM_IRQ_OTHERS,
	  true, "CHG TIMEOUT" },
	{ MAX8997_PMICIRQ(DCINOVP), 0, CM_IRQ_OTHERS, true, "CHG OVERVOLT" },
	{ MAX8997_PMICIRQ(CHGINS), 0, CM_IRQ_EXT_PWR_IN_OUT,
	  true, "CHG PWR Insert" },
	{ MAX8997_PMICIRQ(CHGRM), 0, CM_IRQ_EXT_PWR_IN_OUT,
	  true, "CHG PWR Remove" },
	{ MAX8997_PMICIRQ(PWRONR), 0, CM_IRQ_OTHERS, true, "Pwr Button" },
#if 0
	{ MAX17042_IRQ(Battery_Removed), 0, CM_IRQ_BATT_OUT,
	  true, "Batt Out" },
#endif
	{ 0, 0, 0, false, NULL },
};
static struct charger_desc nuri_charger_desc = {
	.psy_name		= "battery",
	.polling_interval_ms	= 30000,
	.polling_mode		= CM_POLL_CHARGING_ONLY,
	.fullbatt_vchkdrop_ms	= 30000,
	.fullbatt_vchkdrop_uV	= 100000,
	.fullbatt_uV		= 4200000,
	.battery_present	= CM_FUEL_GAUGE,
	.psy_charger_stat	= nuri_charger_stats,
	.charger_regulators	= nuri_chargers,
	.num_charger_regulators	= ARRAY_SIZE(nuri_chargers),
	.psy_fuel_gauge		= "max17042_battery",
	.irqs			= nuri_chg_irqs,

	.temperature_out_of_range	= ntc_thermistor_ck,
	.measure_battery_temp	= false,
};

void nuri_chg_irqs_set_base(int pmic_base, int fuel_gauge_base)
{
	static bool run = false;
	int i = 0;

	if (run)
		return;
	run = true;

	while (nuri_chg_irqs[i].irq) {
		if (nuri_chg_irqs[i].irq & _MAX8997_PMICIRQ)
			nuri_chg_irqs[i].irq = pmic_base +
				IRQ_OFFSET(nuri_chg_irqs[i].irq);
		else if (nuri_chg_irqs[i].irq & _MAX17042_IRQ)
			nuri_chg_irqs[i].irq = fuel_gauge_base +
				IRQ_OFFSET(nuri_chg_irqs[i].irq);
		i++;
	}
}

struct charger_global_desc nuri_charger_g_desc = {
	.rtc_name = "rtc0",
	.rtc_only_wakeup = s3c_wksrc_rtc_alarm,
	.assume_timer_stops_in_suspend	= true,
};

struct platform_device nuri_charger_manager = {
	.name			= "charger-manager",
	.dev			= {
		.platform_data = &nuri_charger_desc,
	},
};
