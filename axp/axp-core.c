

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


	ret = axp_i2c_read(reg, &reg_val);

	if ((reg_val & bit_mask) != bit_mask) {
		reg_val |= bit_mask;
		ret = axp_i2c_write(reg, reg_val);
	}

	return ret;
}


s32 axp_i2c_clr_bits(s32 reg, u8 bit_mask)
{
	u8 reg_val;
	s32 ret = 0;


	ret = axp_i2c_read(reg, &reg_val);

	if (reg_val & bit_mask) {
		reg_val &= ~bit_mask;
		ret = axp_i2c_write(reg, reg_val);
	}

	return ret;
}

s32 axp_i2c_update(s32 reg, u8 val, u8 mask)
{
	u8 reg_val;
	s32 ret = 0;


	ret = axp_i2c_read(reg, &reg_val);

	if ((reg_val & mask) != val) {
		reg_val = (reg_val & ~mask) | val;
		ret = axp_i2c_write(reg, reg_val);
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

	axp_i2c_reads(AXP2585_INTSTS1,irq_data->chip->num_regs, reg_val);

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
			axp_i2c_write(irq_data->map,
				irq_data->chip->status_base + i, reg_val[i]);
			udelay(30);
		}
	}
}



int axp_enable_irq(struct axp_dev *adev, int irq_no)
{
	struct axp_irq_chip_data *irq_data = adev->irq_data;
	int reg, ret = 0;
	u8 mask;

	if (!irq_data || irq_no < 0 || irq_no >= irq_data->num_irqs)
		return -1;

	if (irq_data->irqs[irq_no].handler) {
		reg = irq_no / AXP_REG_WIDTH;
		reg += irq_data->chip->enable_base;
		mask = 1 << (irq_no % AXP_REG_WIDTH);
		ret = axp_i2c_set_bits( reg, mask);
	}

	return ret;
}

int axp_disable_irq(struct axp_dev *adev, int irq_no)
{
	struct axp_irq_chip_data *irq_data = adev->irq_data;
	int reg, ret = 0;
	u8 mask;

	if (!irq_data || irq_no < 0 || irq_no >= irq_data->num_irqs)
		return -1;

	reg = irq_no / AXP_REG_WIDTH;
	reg += irq_data->chip->enable_base;
	mask = 1 << (irq_no % AXP_REG_WIDTH);
	ret = axp_i2c_clr_bits( reg, mask);

	return ret;
}



