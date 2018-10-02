/*
 * drivers/power/axp/axp-core.h
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

#ifndef AXP_CORE_H_
#define AXP_CORE_H_

#include "axp-charger.h"

#define AXP_REG_WIDTH     (8)
#define AXP_ADD_WIDTH     (8)
#define ABS(x)		((x) > 0 ? (x) : -(x))


int axp_i2c_write(struct axp_regmap *map, s32 reg, u8 val);
int axp_i2c_writes(struct axp_regmap *map, s32 reg, s32 len, u8 *val);
int axp_i2c_read(struct axp_regmap *map, s32 reg, u8 *val);
int axp_i2c_reads(struct axp_regmap *map, s32 reg, s32 len, u8 *val);
int axp_i2c_update(struct axp_regmap *map, s32 reg, u8 val, u8 mask);
int axp_i2c_set_bits(struct axp_regmap *map, s32 reg, u8 bit_mask);
int axp_i2c_clr_bits(struct axp_regmap *map, s32 reg, u8 bit_mask);

#endif /* AXP_CORE_H_ */
