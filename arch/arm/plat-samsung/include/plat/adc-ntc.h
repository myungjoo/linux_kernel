/* linux/arch/arm/plat-samsung/include/plat/adc-ntc.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * NTC Thermistor attached to Samsung ADC Controller driver information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __PLAT_ADC_NTC_H
#define __PLAT_ADC_NTC_H __FILE__

extern void s3c_adc_ntc_init(int port);
extern int read_s3c_adc_ntc(int *mC);

#endif /* __PLAT_ADC_NTC_H */
