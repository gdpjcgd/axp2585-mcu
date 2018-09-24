#include "axp2585-charger.h"

#include "../axp-core.h"
#include "../axp-charger.h"


static int axp2585_set_usb_vhold(int vol)
{
	u8 tmp;

	if (vol) {
		/*axp_i2c_set_bits( 0xff,0x60);*/
		if (vol >= 3880 && vol <= 5080) {
			tmp = (vol - 3880)/80;
			axp_i2c_update(0x11, tmp, 0x0f);
		}
	}
	return 0;
}

static int axp2585_get_usb_vhold()
{
	u8 tmp;

	axp_i2c_read(0x11, &tmp);
	return (tmp*80 + 3880);
}

static int axp2585_set_usb_ihold( int cur)
{
	u8 tmp;

	if (cur) {
		/*axp_i2c_set_bits( 0xff,0x60);*/
		if (cur >= 100 && cur <= 3250) {
			tmp = (cur - 100)/50;
			axp_i2c_update(0x10, tmp, 0x3f);
		} else {
			pr_err("set usb limit voltage error, %d mV\n",
						axp2585_config.pmu_usbpc_vol);
		}
	} else {
		/*axp_i2c_clr_bits( 0xff,0x60);*/
	}
	return 0;
}

static int axp2585_get_usb_ihold()
{
	u8 tmp;


	axp_i2c_read(0x10, &tmp);
	return (tmp*50 + 100);
}

static int axp2585_get_rest_cap()
{
	u8 val, temp_val[2], tmp[2];
	u8 ocv_percent = 0;
	u8 coulomb_percent = 0;
	int batt_max_cap, coulumb_counter;
	int rest_vol;

	axp_i2c_read(AXP2585_CAP, &val);
	if (!(val & 0x80))
		return 0;
	rest_vol = (int) (val & 0x7F);
	axp_i2c_read( 0xe4, &tmp[0]);
	if (tmp[0] & 0x80) {
		ocv_percent = tmp[0] & 0x7f;
		printf("ocv_percent = %d\n", ocv_percent);
	}
	axp_i2c_read(0xe5, &tmp[0]);
	if (tmp[0] & 0x80) {
		coulomb_percent = tmp[0] & 0x7f;
		printf("coulomb_percent = %d\n", coulomb_percent);
	}
	if (ocv_percent == 100 && cdev->charging == 0 && rest_vol == 99
		&& (cdev->ac_valid == 1 || cdev->usb_valid == 1)) {
		axp_i2c_clr_bits( AXP2585_COULOMB_CTL, 0x80);
		axp_i2c_set_bits( AXP2585_COULOMB_CTL, 0x80);
		AXP_DEBUG(AXP_SPLY, cdev->chip->pmu_num, "Reset coulumb\n");
		rest_vol = 100;
	}
	axp_i2c_reads(0xe2, 2, temp_val);
	coulumb_counter = (((temp_val[0] & 0x7f) << 8) + temp_val[1])
						* 1456 / 1000;

	axp_i2c_reads(0xe0, 2, temp_val);
	batt_max_cap = (((temp_val[0] & 0x7f) << 8) + temp_val[1])
						* 1456 / 1000;

	AXP_DEBUG(AXP_SPLY, cdev->chip->pmu_num,
			"batt_max_cap = %d\n", batt_max_cap);
	return rest_vol;
}

static inline int axp2585_vbat_to_mV(u32 reg)
{
	return ((int)(((reg >> 8) << 4) | (reg & 0x000F))) * 1200 / 1000;
}

static int axp2585_get_vbat(struct axp_charger_dev *cdev)
{
	u8 tmp[2];
	u32 res;
	struct axp_regmap *map = cdev->chip->regmap;

	axp_i2c_reads(AXP2585_VBATH_RES, 2, tmp);
	res = (tmp[0] << 8) | tmp[1];

	return axp2585_vbat_to_mV(res);
}

static inline int axp2585_ibat_to_mA(u32 reg)
{
	return (int)((((reg >> 8) << 4) | (reg & 0x000F)) << 1);
}

static inline int axp2585_icharge_to_mA(u32 reg)
{
	return (int)((((reg >> 8) << 4) | (reg & 0x000F)) << 1);
}

static int axp2585_get_ibat()
{
	u8 tmp[2];
	u32 res;
	axp_i2c_reads(AXP2585_IBATH_REG, 2, tmp);
	res = (tmp[0] << 8) | tmp[1];

	return axp2585_icharge_to_mA(res);
}

static int axp2585_get_disibat()
{
	u8 tmp[2];
	u32 dis_res;
	axp_i2c_reads( AXP2585_DISIBATH_REG, 2, tmp);
	dis_res = (tmp[0] << 8) | tmp[1];

	return axp2585_ibat_to_mA(dis_res);
}

static int axp2585_set_chg_cur(int cur)
{
	uint8_t tmp = 0;

/*
	if (cur == 0)
		axp_i2c_clr_bits( axp2585_CHARGE_CONTROL1, 0x80);
	else
		axp_i2c_set_bits( axp2585_CHARGE_CONTROL1, 0x80);
*/
	tmp = (cur) / 64;
	if (tmp > 0x3f)
		tmp = 0x3f;
	axp_i2c_update(0x8b, tmp, 0x3F);
	return 0;
}

static int axp2585_set_chg_vol(int vol)
{
	uint8_t tmp = 0;


	if (vol > 3840 && vol < 4608)
		tmp = (vol - 3840)/16;
	else {
		printf("unsupported voltage: %dmv, use default 4200mv\n", vol);
		tmp = (4200 - 3840)/16;
	}
	axp_i2c_update(0x8c, tmp << 2, 0xfc);
	return 0;
}

static int axp2585_charger_init(struct axp_dev *axp_dev)
{
	u8 ocv_cap[32];
	u8 val = 0;
	int cur_coulomb_counter, rdc;
	int i, ocv_cou_adjust_time[4] = {60, 120, 15, 30};
	int update_min_times[8] = {30, 60, 120, 164, 0, 5, 10, 20};
	/*set chg time */
	if (axp2585_config.pmu_init_chg_pretime < 40)
		axp2585_config.pmu_init_chg_pretime = 40;
	val = (axp2585_config.pmu_init_chg_pretime - 40)/10;
	if (val >= 3)
		val = 3;
	val = 0x80 + (val<<5);
	axp_i2c_update(0x8e, val, 0xe0);

	if (axp2585_config.pmu_init_chg_csttime <= 60 * 5)
		val = 0;
	else if (axp2585_config.pmu_init_chg_csttime <= 60 * 8)
		val = 1;
	else if (axp2585_config.pmu_init_chg_csttime <= 60 * 12)
		val = 2;
	else if (axp2585_config.pmu_init_chg_csttime <= 60 * 20)
		val = 3;
	else
		val = 3;
	val = (val << 1) + 0x01;
	axp_i2c_update(0x8d, val, 0x07);
	/* adc set */
	val = AXP2585_ADC_BATVOL_ENABLE | AXP2585_ADC_BATCUR_ENABLE;
	if (axp2585_config.pmu_bat_temp_enable != 0)
		val = val | AXP2585_ADC_TSVOL_ENABLE;
	axp_i2c_update(AXP2585_ADC_CONTROL, val,
						AXP2585_ADC_BATVOL_ENABLE
						| AXP2585_ADC_BATCUR_ENABLE
						| AXP2585_ADC_TSVOL_ENABLE);

	axp_i2c_read(AXP2585_TS_PIN_CONTROL, &val);
	switch (axp2585_config.pmu_init_adc_freq / 100) {
	case 1:
		val &= ~(3 << 5);
		break;
	case 2:
		val &= ~(3 << 5);
		val |= 1 << 5;
		break;
	case 4:
		val &= ~(3 << 5);
		val |= 2 << 5;
		break;
	case 8:
		val |= 3 << 5;
		break;
	default:
		break;
	}

	if (axp2585_config.pmu_bat_temp_enable != 0)
		val &= (~(1 << 7));
	axp_i2c_write(AXP2585_TS_PIN_CONTROL, val);

	/* bat para */
	axp_i2c_write(AXP2585_WARNING_LEVEL,
		((axp2585_config.pmu_battery_warning_level1 - 5) << 4)
		+ axp2585_config.pmu_battery_warning_level2);

	if (axp2585_config.pmu_init_chgvol < 3840)
		axp2585_config.pmu_init_chgvol = 3840;
	val = (axp2585_config.pmu_init_chgvol - 3840)/16;
	if (val > 0x30)
		val = 0x30;
	val <<= 2;
	axp_i2c_update(AXP2585_CHARGE_CONTROL2, val, 0xfc);

	ocv_cap[0]  = axp2585_config.pmu_bat_para1;
	ocv_cap[1]  = axp2585_config.pmu_bat_para2;
	ocv_cap[2]  = axp2585_config.pmu_bat_para3;
	ocv_cap[3]  = axp2585_config.pmu_bat_para4;
	ocv_cap[4]  = axp2585_config.pmu_bat_para5;
	ocv_cap[5]  = axp2585_config.pmu_bat_para6;
	ocv_cap[6]  = axp2585_config.pmu_bat_para7;
	ocv_cap[7]  = axp2585_config.pmu_bat_para8;
	ocv_cap[8]  = axp2585_config.pmu_bat_para9;
	ocv_cap[9]  = axp2585_config.pmu_bat_para10;
	ocv_cap[10] = axp2585_config.pmu_bat_para11;
	ocv_cap[11] = axp2585_config.pmu_bat_para12;
	ocv_cap[12] = axp2585_config.pmu_bat_para13;
	ocv_cap[13] = axp2585_config.pmu_bat_para14;
	ocv_cap[14] = axp2585_config.pmu_bat_para15;
	ocv_cap[15] = axp2585_config.pmu_bat_para16;
	ocv_cap[16] = axp2585_config.pmu_bat_para17;
	ocv_cap[17] = axp2585_config.pmu_bat_para18;
	ocv_cap[18] = axp2585_config.pmu_bat_para19;
	ocv_cap[19] = axp2585_config.pmu_bat_para20;
	ocv_cap[20] = axp2585_config.pmu_bat_para21;
	ocv_cap[21] = axp2585_config.pmu_bat_para22;
	ocv_cap[22] = axp2585_config.pmu_bat_para23;
	ocv_cap[23] = axp2585_config.pmu_bat_para24;
	ocv_cap[24] = axp2585_config.pmu_bat_para25;
	ocv_cap[25] = axp2585_config.pmu_bat_para26;
	ocv_cap[26] = axp2585_config.pmu_bat_para27;
	ocv_cap[27] = axp2585_config.pmu_bat_para28;
	ocv_cap[28] = axp2585_config.pmu_bat_para29;
	ocv_cap[29] = axp2585_config.pmu_bat_para30;
	ocv_cap[30] = axp2585_config.pmu_bat_para31;
	ocv_cap[31] = axp2585_config.pmu_bat_para32;
	axp_i2c_writes(0xC0, 32, ocv_cap);

	/*Init CHGLED function*/
	if (axp2585_config.pmu_chgled_func)
		axp_i2c_set_bits(0x90, 0x80); /* control by charger */
	else
		axp_i2c_clr_bits(0x90, 0x80); /* drive MOTO */
#if 0
	/*set CHGLED Indication Type*/
	if (axp2585_config.pmu_chgled_type)
		axp_i2c_set_bits( 0x90, 0x01); /* Type B */
	else
		axp_i2c_clr_bits( 0x90, 0x07); /* Type A */
#else
	axp_i2c_set_bits(0x90, axp2585_config.pmu_chgled_type & 0x07);
#endif
	/*Init battery capacity correct function*/
	if (axp2585_config.pmu_batt_cap_correct)
		axp_i2c_set_bits(0xb8, 0x20); /* enable */
	else
		axp_i2c_clr_bits(0xb8, 0x20); /* disable */

	/*battery detect enable*/
	if (axp2585_config.pmu_batdeten)
		axp_i2c_set_bits(0x8e, 0x08);
	else
		axp_i2c_clr_bits(0x8e, 0x08);

	/* RDC initial */
	axp_i2c_read(AXP2585_RDC0, &val);
	if ((axp2585_config.pmu_battery_rdc) && (!(val & 0x40))) {
		rdc = (axp2585_config.pmu_battery_rdc * 10000 + 5371) / 10742;
		u8 rdc_reg[2];
		u16 rdc_reg_val;
		axp_i2c_reads(AXP2585_RDC0,2,rdc_reg);
		rdc_reg_val=(rdc_reg[0]<<8)|rdc_reg[1];
		rdc_reg_val=(rdc_reg_val&0xe0)|rdc;
		rdc_reg[1]=(rdc_reg_val&0xff00)>>8;
		rdc_reg[0]=rdc_reg_val&0xff;
		//axp_i2c_write(AXP2585_RDC0, ((rdc >> 8) & 0x1F)|0x80);
		//axp_i2c_write(AXP2585_RDC1, rdc & 0x00FF);
		axp_i2c_writes(AXP2585_RDC0,2,rdc_reg);
	}

	axp_i2c_read(AXP2585_BATCAP0, &val);
	if ((axp2585_config.pmu_battery_cap) && (!(val & 0x80))) {
		cur_coulomb_counter = axp2585_config.pmu_battery_cap
					* 1000 / 1456;
		axp_i2c_write(AXP2585_BATCAP0,
					((cur_coulomb_counter >> 8) | 0x80));
		axp_i2c_write(AXP2585_BATCAP1,
					cur_coulomb_counter & 0x00FF);
	} else if (!axp2585_config.pmu_battery_cap) {
		axp_i2c_write(AXP2585_BATCAP0, 0x00);
		axp_i2c_write(AXP2585_BATCAP1, 0x00);
	}
#if 0
	if (axp2585_config.pmu_bat_unused == 1)
		axp2585_spy_info.batt->det_unused = 1;
	else
		axp2585_spy_info.batt->det_unused = 0;
#endif

	if (axp2585_config.pmu_bat_temp_enable != 0) {
		axp_i2c_write(AXP2585_VLTF_CHARGE,
				axp2585_config.pmu_bat_charge_ltf * 10 / 128);
		axp_i2c_write(AXP2585_VHTF_CHARGE,
				axp2585_config.pmu_bat_charge_htf * 10 / 128);
		axp_i2c_write(AXP2585_VLTF_WORK,
				axp2585_config.pmu_bat_shutdown_ltf * 10 / 128);
		axp_i2c_write(AXP2585_VHTF_WORK,
				axp2585_config.pmu_bat_shutdown_htf * 10 / 128);
	}
	/*enable fast charge */
	axp_i2c_update( 0x31, 0x04, 0x04);
	/*set POR time as 16s*/
	axp_i2c_update( AXP2585_POK_SET, 0x30, 0x30);
	for (i = 0; i < ARRAY_SIZE(update_min_times); i++) {
		if (update_min_times[i] == axp2585_config.pmu_update_min_time)
			break;
	}
	axp_i2c_update( AXP2585_ADJUST_PARA, i, 0x7);
	/*initial the ocv_cou_adjust_time*/
	for (i = 0; i < ARRAY_SIZE(ocv_cou_adjust_time); i++) {
		if (ocv_cou_adjust_time[i] == axp2585_config.pmu_ocv_cou_adjust_time)
			break;
	}
	i <<= 6;
	axp_i2c_update( AXP2585_ADJUST_PARA1, i, 0xC0);
	return 0;
}



static int axp2585_charger_probe(struct platform_device *pdev)
{
	int ret, i, irq;
	struct axp_charger_dev *chg_dev;
	struct axp_dev *axp_dev = dev_get_drvdata(pdev->dev.parent);


	axp2585_ac_info.ac_vol = axp2585_config.pmu_ac_vol;
	axp2585_ac_info.ac_cur = axp2585_config.pmu_ac_cur;
	axp2585_usb_info.usb_pc_vol = axp2585_config.pmu_usbpc_vol;
	axp2585_usb_info.usb_pc_cur = axp2585_config.pmu_usbpc_cur;
	axp2585_usb_info.usb_ad_vol = axp2585_config.pmu_ac_vol;
	axp2585_usb_info.usb_ad_cur = axp2585_config.pmu_ac_cur;
	axp2585_batt_info.runtime_chgcur = axp2585_config.pmu_runtime_chgcur;
	axp2585_batt_info.suspend_chgcur = axp2585_config.pmu_suspend_chgcur;
	axp2585_batt_info.shutdown_chgcur = axp2585_config.pmu_shutdown_chgcur;
	battery_data.voltage_max_design = axp2585_config.pmu_init_chgvol
								* 1000;
	battery_data.voltage_min_design = axp2585_config.pmu_pwroff_vol
								* 1000;
	battery_data.energy_full_design = axp2585_config.pmu_battery_cap;

	axp2585_charger_init(axp_dev);
	chg_dev->pmic_temp_offset = 0x56;
	chg_dev->spy_info->batt->bat_temp_offset = 0x58;

	return 0;

}










