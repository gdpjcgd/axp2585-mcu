/*
 * drivers/power/supply/axp/axp15060/axp15060.h
 * (C) Copyright 2010-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Pannan <pannan@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#ifndef AXP15060_H_
#define AXP15060_H_
#include<linux/types.h>

#define AXP15060_POWER_SOURCE      (0x00)
#define AXP15060_IC_TYPE           (0x03)
#define AXP15060_DATA_BUFFER1      (0x04)
#define AXP15060_DATA_BUFFER2      (0x05)
#define AXP15060_DATA_BUFFER3      (0x06)
#define AXP15060_DATA_BUFFER4      (0x07)
#define AXP15060_ON_OFF_CTL1       (0x10)
#define AXP15060_ON_OFF_CTL2       (0x11)
#define AXP15060_ON_OFF_CTL3       (0x12)
#define AXP15060_DC1OUT_VOL        (0x13)
#define AXP15060_DC2OUT_VOL        (0x14)
#define AXP15060_DC3OUT_VOL        (0x15)
#define AXP15060_DC4OUT_VOL        (0x16)
#define AXP15060_DC5OUT_VOL        (0x17)
#define AXP15060_DC6OUT_VOL        (0x18)
#define AXP15060_ALDO1OUT_VOL      (0x19)
#define AXP15060_DCDC_MODE_CTL1    (0x1A)
#define AXP15060_DCDC_MODE_CTL2    (0x1B)
#define AXP15060_DCDC_MODE_CTL3    (0x1C)
#define AXP15060_DCDC_FREQ_SET     (0x1D)
#define AXP15060_OUT_MONITOR_CTL   (0x1E)
#define AXP15060_IRQ_PWROK_VOFF    (0x1F)
#define AXP15060_ALDO2OUT_VOL      (0x20)
#define AXP15060_ALDO3OUT_VOL      (0x21)
#define AXP15060_ALDO4OUT_VOL      (0x22)
#define AXP15060_ALDO5OUT_VOL      (0x23)
#define AXP15060_BLDO1OUT_VOL      (0x24)
#define AXP15060_BLDO2OUT_VOL      (0x25)
#define AXP15060_BLDO3OUT_VOL      (0x26)
#define AXP15060_BLDO4OUT_VOL      (0x27)
#define AXP15060_BLDO5OUT_VOL      (0x28)
#define AXP15060_CLDO1OUT_VOL      (0x29)
#define AXP15060_CLDO2OUT_VOL      (0x2A)
#define AXP15060_CLDO3OUT_VOL      (0x2B)
#define AXP15060_CLDO4_GPIO2_CTL   (0x2C)
#define AXP15060_CLDO4OUT_VOL      (0x2D)
#define AXP15060_CPUSOUT_VOL       (0x2E)
#define AXP15060_WAKEUP_CTL_OCIRQ  (0x31)
#define AXP15060_PWR_DISABLE_DOWN  (0x32)
#define AXP15060_POK_SET           (0x36)
#define AXP15060_INT_MODE_SELECT   (0x3E)
#define AXP15060_INTEN1            (0x40)
#define AXP15060_INTEN2            (0x41)
#define AXP15060_INTSTS1           (0x48)
#define AXP15060_INTSTS2           (0x49)

/* bit definitions for AXP events ,irq event */
#define AXP15060_IRQ_LOWN1         (0)
#define AXP15060_IRQ_LOWN2         (1)
#define AXP15060_IRQ_DC1UN         (2)
#define AXP15060_IRQ_DC2UN         (3)
#define AXP15060_IRQ_DC3UN         (4)
#define AXP15060_IRQ_DC4UN         (5)
#define AXP15060_IRQ_DC5UN         (6)
#define AXP15060_IRQ_DC6UN         (7)
#define AXP15060_IRQ_PEKL          (8)
#define AXP15060_IRQ_PEKS          (9)
#define AXP15060_IRQ_PEKO          (10)
#define AXP15060_IRQ_PEKFE         (11)
#define AXP15060_IRQ_PEKRE         (12)
#define AXP15060_IRQ_ALDOIN_VOFF   (13)
#define AXP15060_IRQ_DC2OV         (14)
#define AXP15060_IRQ_DC3OV         (15)

extern s32 axp_debug;
extern struct axp_config_info axp15060_config;

#endif /* AXP15060_H_ */
