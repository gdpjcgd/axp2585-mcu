/*
 * drivers/power/axp/axp-core.c
 * (C) Copyright 2010-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Pannan <pannan@allwinnertech.com>
 *
 * axp common APIs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include "axp-core.h"




s32 axp_usb_vbus_output(int high)
{

}


int axp_usb_is_connected(void)
{
	return axp_usb_connect;
}



s32 axp_i2c_write(s32 reg, u8 val)
{
	s32 ret = 0;

	return ret;
}


s32 axp_i2c_writes(s32 reg, s32 len, u8 *val)
{
	s32 ret = 0;



	return ret;
}


s32 axp_i2c_read(s32 reg, u8 *val)
{

}


s32 axp_i2c_reads(s32 reg, s32 len, u8 *val)
{

}


s32 axp_i2c_set_bits(s32 reg, u8 bit_mask)
{
	u8 reg_val;
	s32 ret = 0;


	ret = axp_i2c_read(reg, &reg_val, false);

	if ((reg_val & bit_mask) != bit_mask) {
		reg_val |= bit_mask;
		ret = axp_i2c_write(reg, reg_val, false);
	}

	return ret;
}


s32 axp_i2c_clr_bits(s32 reg, u8 bit_mask)
{
	u8 reg_val;
	s32 ret = 0;


	ret = axp_i2c_read(reg, &reg_val, false);

	if (reg_val & bit_mask) {
		reg_val &= ~bit_mask;
		ret = axp_i2c_write(reg, reg_val, false);
	}

	return ret;
}

s32 axp_i2c_update(s32 reg, u8 val, u8 mask)
{
	u8 reg_val;
	s32 ret = 0;


	ret = axp_i2c_read(reg, &reg_val, false);

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | val;
		ret = axp_i2c_write(reg, reg_val, false);
	}

	return ret;
}

static void __do_irq(int pmu_num, struct axp_irq_chip_data *irq_data)
{
	u64 irqs = 0;
	u8 reg_val[8];
	u32 i, j;
	void *idata;

	if (irq_data == NULL)
		return;

	axp_regmap_reads(irq_data->map, irq_data->chip->status_base,
			irq_data->chip->num_regs, reg_val);

	for (i = 0; i < irq_data->chip->num_regs; i++)
		irqs |= (u64)reg_val[i] << (i * AXP_REG_WIDTH);

	irqs &= irq_data->irqs_enabled;
	if (irqs == 0)
		return;

	AXP_DEBUG(AXP_INT, pmu_num, "irqs enabled = 0x%llx\n",
				irq_data->irqs_enabled);
	AXP_DEBUG(AXP_INT, pmu_num, "irqs = 0x%llx\n", irqs);

	for_each_set_bit(j, (unsigned long *)&irqs, irq_data->num_irqs) {
		if (irq_data->irqs[j].handler) {
			idata = irq_data->irqs[j].data;
			irq_data->irqs[j].handler(j, idata);
		}
	}

	for (i = 0; i < irq_data->chip->num_regs; i++) {
		if (reg_val[i] != 0) {
			axp_regmap_write(irq_data->map,
				irq_data->chip->status_base + i, reg_val[i]);
			udelay(30);
		}
	}
}

static void axp_irq_work_func(struct work_struct *work)
{
	struct axp_dev *adev;

	list_for_each_entry(adev, &axp_dev_list, list) {
		__do_irq(adev->pmu_num, adev->irq_data);
	}


	//sunxi_nmi_clear_status();
	//sunxi_nmi_enable();
}

static irqreturn_t axp_irq(int irq, void *data)
{
	struct axp_dev *adev;

	//sunxi_nmi_disable();
	if (axp_suspend_flag == AXP_NOT_SUSPEND) {
		schedule_work(&axp_irq_work);
	} else if (axp_suspend_flag == AXP_WAS_SUSPEND) {
		list_for_each_entry(adev, &axp_dev_list, list) {
			if (adev->irq_data->wakeup_event) {
				adev->irq_data->wakeup_event();
				axp_suspend_flag = AXP_SUSPEND_WITH_IRQ;
			}
		}
	}

	return IRQ_HANDLED;
}

struct axp_irq_chip_data *axp_irq_chip_register(struct axp_regmap *map,
			int irq_no, int irq_flags,
			struct axp_regmap_irq_chip *irq_chip,
			void (*wakeup_event)(void))
{
	struct axp_irq_chip_data *irq_data = NULL;
	struct axp_regmap_irq *irqs = NULL;
	int i, err = 0;

	irq_data = kzalloc(sizeof(*irq_data), GFP_KERNEL);
	if (IS_ERR_OR_NULL(irq_data)) {
		pr_err("axp irq data: not enough memory for irq data\n");
		return NULL;
	}

	irq_data->map = map;
	irq_data->chip = irq_chip;
	irq_data->num_irqs = AXP_REG_WIDTH * irq_chip->num_regs;

	irqs = kzalloc(irq_chip->num_regs * AXP_REG_WIDTH * sizeof(*irqs),
				GFP_KERNEL);
	if (IS_ERR_OR_NULL(irqs)) {
		pr_err("axp irq data: not enough memory for irq disc\n");
		goto free_irq_data;
	}

	mutex_init(&irq_data->lock);
	irq_data->irqs = irqs;
	irq_data->irqs_enabled = 0;
	irq_data->wakeup_event = wakeup_event;

	/* disable all irq and clear all irq pending */
	for (i = 0; i < irq_chip->num_regs; i++) {
		axp_regmap_clr_bits(map, irq_chip->enable_base + i, 0xff);
		axp_regmap_set_bits(map, irq_chip->status_base + i, 0xff);
	}

#ifdef CONFIG_DUAL_AXP_USED
	if (axp_dev_register_count == 1) {
		err = request_irq(irq_no, axp_irq, irq_flags, "axp", irq_data);
		goto irq_out;
	} else if (axp_dev_register_count == 2) {
		return irq_data;
	}
#else
	err = request_irq(irq_no, axp_irq, irq_flags, irq_chip->name, irq_data);
#endif

#ifdef CONFIG_DUAL_AXP_USED
irq_out:
#endif
	if (err)
		goto free_irqs;

	INIT_WORK(&axp_irq_work, axp_irq_work_func);
#if 0
	sunxi_nmi_set_trigger(IRQF_TRIGGER_LOW);
	sunxi_nmi_clear_status();
	sunxi_nmi_enable();
#endif
	return irq_data;

free_irqs:
	kfree(irqs);
free_irq_data:
	kfree(irq_data);

	return NULL;
}
EXPORT_SYMBOL_GPL(axp_irq_chip_register);


int axp_enable_irq(struct axp_dev *adev, int irq_no)
{
	struct axp_irq_chip_data *irq_data = adev->irq_data;
	int reg, ret = 0;
	u8 mask;

	if (!irq_data || irq_no < 0 || irq_no >= irq_data->num_irqs)
		return -1;

	if (irq_data->irqs[irq_no].handler) {
		mutex_lock(&irq_data->lock);
		reg = irq_no / AXP_REG_WIDTH;
		reg += irq_data->chip->enable_base;
		mask = 1 << (irq_no % AXP_REG_WIDTH);
		ret = axp_regmap_set_bits(adev->regmap, reg, mask);
		mutex_unlock(&irq_data->lock);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(axp_enable_irq);

int axp_disable_irq(struct axp_dev *adev, int irq_no)
{
	struct axp_irq_chip_data *irq_data = adev->irq_data;
	int reg, ret = 0;
	u8 mask;

	if (!irq_data || irq_no < 0 || irq_no >= irq_data->num_irqs)
		return -1;

	mutex_lock(&irq_data->lock);
	reg = irq_no / AXP_REG_WIDTH;
	reg += irq_data->chip->enable_base;
	mask = 1 << (irq_no % AXP_REG_WIDTH);
	ret = axp_regmap_clr_bits(adev->regmap, reg, mask);
	mutex_unlock(&irq_data->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(axp_disable_irq);

int axp_free_irq(struct axp_dev *adev, int irq_no)
{
	struct axp_irq_chip_data *irq_data = adev->irq_data;
	int reg;
	u8 mask;

	if (!irq_data || irq_no < 0 || irq_no >= irq_data->num_irqs)
		return -1;

	mutex_lock(&irq_data->lock);
	if (irq_data->irqs[irq_no].handler) {
		reg = irq_no / AXP_REG_WIDTH;
		reg += irq_data->chip->enable_base;
		mask = 1 << (irq_no % AXP_REG_WIDTH);
		axp_regmap_clr_bits(adev->regmap, reg, mask);
		irq_data->irqs[irq_no].data = NULL;
		irq_data->irqs[irq_no].handler = NULL;
	}
	mutex_unlock(&irq_data->lock);

	return 0;
}
EXPORT_SYMBOL_GPL(axp_free_irq);


