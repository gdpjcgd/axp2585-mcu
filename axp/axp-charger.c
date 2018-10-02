

#include "axp-core.h"
#include "axp-charger.h"


static struct axp_adc_res adc;
static bool battery_initialized;
static struct axp_config_info *axp_config_obj;
static int plug_debounce;



int axp_usbvol(enum AW_CHARGE_TYPE type)
{
	axp_usbvolflag = type;
	return 0;
}


int axp_usbcur(enum AW_CHARGE_TYPE type)
{
	axp_usbcurflag = type;
	return 0;
}


static inline void axp_read_adc(struct axp_adc_res *adc)
{
	u8 tmp[2];
	axp_i2c_reads( chg_dev->spy_info->batt->bat_temp_offset,
				2, tmp);
	adc->ts_res = ((u16) tmp[0] << 8) | tmp[1];
}

static inline s32 axp_vts_to_temp(s32 data,
		const struct axp_config_info *axp_config)
{
	s32 temp;

	if (data < 80 || !axp_config->pmu_bat_temp_enable)
		return 30;
	else if (data < axp_config->pmu_bat_temp_para16)
		return 80;
	else if (data <= axp_config->pmu_bat_temp_para15) {
		temp = 70 + (axp_config->pmu_bat_temp_para15-data)*10/
		(axp_config->pmu_bat_temp_para15-axp_config->pmu_bat_temp_para16);
	} else if (data <= axp_config->pmu_bat_temp_para14) {
		temp = 60 + (axp_config->pmu_bat_temp_para14-data)*10/
		(axp_config->pmu_bat_temp_para14-axp_config->pmu_bat_temp_para15);
	} else if (data <= axp_config->pmu_bat_temp_para13) {
		temp = 55 + (axp_config->pmu_bat_temp_para13-data)*5/
		(axp_config->pmu_bat_temp_para13-axp_config->pmu_bat_temp_para14);
	} else if (data <= axp_config->pmu_bat_temp_para12) {
		temp = 50 + (axp_config->pmu_bat_temp_para12-data)*5/
		(axp_config->pmu_bat_temp_para12-axp_config->pmu_bat_temp_para13);
	} else if (data <= axp_config->pmu_bat_temp_para11) {
		temp = 45 + (axp_config->pmu_bat_temp_para11-data)*5/
		(axp_config->pmu_bat_temp_para11-axp_config->pmu_bat_temp_para12);
	} else if (data <= axp_config->pmu_bat_temp_para10) {
		temp = 40 + (axp_config->pmu_bat_temp_para10-data)*5/
		(axp_config->pmu_bat_temp_para10-axp_config->pmu_bat_temp_para11);
	} else if (data <= axp_config->pmu_bat_temp_para9) {
		temp = 30 + (axp_config->pmu_bat_temp_para9-data)*10/
		(axp_config->pmu_bat_temp_para9-axp_config->pmu_bat_temp_para10);
	} else if (data <= axp_config->pmu_bat_temp_para8) {
		temp = 20 + (axp_config->pmu_bat_temp_para8-data)*10/
		(axp_config->pmu_bat_temp_para8-axp_config->pmu_bat_temp_para9);
	} else if (data <= axp_config->pmu_bat_temp_para7) {
		temp = 10 + (axp_config->pmu_bat_temp_para7-data)*10/
		(axp_config->pmu_bat_temp_para7-axp_config->pmu_bat_temp_para8);
	} else if (data <= axp_config->pmu_bat_temp_para6) {
		temp = 5 + (axp_config->pmu_bat_temp_para6-data)*5/
		(axp_config->pmu_bat_temp_para6-axp_config->pmu_bat_temp_para7);
	} else if (data <= axp_config->pmu_bat_temp_para5) {
		temp = 0 + (axp_config->pmu_bat_temp_para5-data)*5/
		(axp_config->pmu_bat_temp_para5-axp_config->pmu_bat_temp_para6);
	} else if (data <= axp_config->pmu_bat_temp_para4) {
		temp = -5 + (axp_config->pmu_bat_temp_para4-data)*5/
		(axp_config->pmu_bat_temp_para4-axp_config->pmu_bat_temp_para5);
	} else if (data <= axp_config->pmu_bat_temp_para3) {
		temp = -10 + (axp_config->pmu_bat_temp_para3-data)*5/
		(axp_config->pmu_bat_temp_para3-axp_config->pmu_bat_temp_para4);
	} else if (data <= axp_config->pmu_bat_temp_para2) {
		temp = -15 + (axp_config->pmu_bat_temp_para2-data)*5/
		(axp_config->pmu_bat_temp_para2-axp_config->pmu_bat_temp_para3);
	} else if (data <= axp_config->pmu_bat_temp_para1) {
		temp = -25 + (axp_config->pmu_bat_temp_para1-data)*10/
		(axp_config->pmu_bat_temp_para1-axp_config->pmu_bat_temp_para2);
	} else
		temp = -25;
	return temp;
}

static inline s32 axp_vts_to_mV(u16 reg)
{
	return ((s32)(((reg >> 8) << 4) | (reg & 0x000F))) * 800 / 1000;
}

static inline void axp_update_ictemp_status(u8 ic_die_temp_reg)
{
	u16 tmp;
	u8 temp_val[2];
	axp_i2c_reads( ic_die_temp_reg, 2, temp_val);
	tmp = (temp_val[0] << 4) + (temp_val[1] & 0x0F);
	chg_dev->ic_temp = (s32) tmp * 1063 / 10000  - 2667 / 10;

}

static inline void axp_update_temp_status(struct axp_charger_dev *chg_dev)
{
	u16 tmp;
	u8 temp_val[2];
	s32 bat_temp_mv;

	chg_dev->adc = &adc;
	axp_read_adc(chg_dev, &adc);

	axp_i2c_reads( chg_dev->pmic_temp_offset, 2, temp_val);
	tmp = (temp_val[0] << 4) + (temp_val[1] & 0x0F);
	chg_dev->ic_temp = (s32) tmp * 1063 / 10000  - 2667 / 10;

	tmp = chg_dev->adc->ts_res;
	bat_temp_mv = axp_vts_to_mV(tmp);
	chg_dev->bat_temp = axp_vts_to_temp(bat_temp_mv, axp_config_obj);

}

/*
 * acin not presence + vbus no presence -> battery presence
 */
static int pwrsrc_parse_bat_det(struct axp_battery_info *batt, u8 val)
{
	if (!(val & ((1 << batt->acpresent_bit) | (1 << batt->vbuspresent_bit))))
		return 1;
	else
		return 0;
}

static int det_parse_bat_det(struct axp_battery_info *batt, u8 val)
{
	if ((val & (1 << batt->det_bit)) && (val & (1 << batt->det_valid_bit)))
		return 1;
	else
		return 0;
}

void axp2585_charger_update_state(struct axp_charger_dev *chg_dev)
{
	u8 val;
	u8 pwrsrc;

	/*sleep 10ms for adapter stable*/
	msleep(10);
	axp_i2c_read( batt->det_offset, &val);
	axp_i2c_read( batt->pwrsrc_offset, &pwrsrc);
	if (batt->det_unused == 0) {
		if (batt->det_valid == 1) {
			chg_dev->bat_det = pwrsrc_parse_bat_det(batt, pwrsrc);
			if (chg_dev->bat_det == 0)
				chg_dev->bat_det = det_parse_bat_det(batt, val);
		} else if (batt->det_valid == 0) {
			chg_dev->bat_det = (val & 1 << batt->det_bit) ? 1 : 0;
		}
	} else if (batt->det_unused == 1) {
		chg_dev->bat_det = 0;
	}

	axp_i2c_read( ac->det_offset, &val);
	chg_dev->ac_det = (val & 1 << ac->det_bit) ? 1 : 0;

	if (usb->det_unused == 0) {
		axp_i2c_read( usb->det_offset, &val);
		chg_dev->usb_det = (val & 1 << usb->det_bit) ? 1 : 0;
	} else if (usb->det_unused == 1) {
		chg_dev->usb_det = 0;
	}

	axp_i2c_read( ac->valid_offset, &val);
	chg_dev->ac_valid = (val & 1 << ac->valid_bit) ? 1 : 0;

	if (usb->det_unused == 0) {
		axp_i2c_read( usb->valid_offset, &val);
		chg_dev->usb_valid = (val & 1 << usb->valid_bit) ? 1 : 0;
	} else if (usb->det_unused == 1) {
		chg_dev->usb_valid = 0;
	}


	axp_i2c_read( ac->in_short_offset, &val);

	if (!chg_dev->in_short)
		chg_dev->ac_charging = chg_dev->ac_valid;

	axp_i2c_read( batt->cur_direction_offset, &val);
	if (val & 1 << batt->cur_direction_bit)
		chg_dev->bat_current_direction = 1;
	else
		chg_dev->bat_current_direction = 0;

	axp_i2c_read( batt->chgstat_offset, &val);

}


void axp_charger_update_value(struct axp_charger_dev *chg_dev)
{

	int bat_vol, bat_cur, bat_discur, ac_vol, ac_cur, usb_vol, usb_cur;


	bat_vol = batt->get_vbat(chg_dev);
	bat_cur = batt->get_ibat(chg_dev);
	bat_discur = batt->get_disibat(chg_dev);

	chg_dev->bat_vol = bat_vol;
	chg_dev->bat_cur = bat_cur;
	chg_dev->bat_discur = bat_discur;

}

static void axp_usb_ac_check_status(struct axp_charger_dev *chg_dev)
{
	chg_dev->usb_pc_charging = (((CHARGE_USB_20 == axp_usbcurflag)
					|| (CHARGE_USB_30 == axp_usbcurflag))
					&& (chg_dev->ext_valid));
	chg_dev->usb_adapter_charging = ((0 == chg_dev->ac_valid)
					&& (CHARGE_USB_20 != axp_usbcurflag)
					&& (CHARGE_USB_30 != axp_usbcurflag)
					&& (chg_dev->ext_valid));
	if (chg_dev->in_short)
		chg_dev->ac_charging = ((chg_dev->usb_adapter_charging == 0)
					&& (chg_dev->usb_pc_charging == 0)
					&& (chg_dev->ext_valid));
	else
		chg_dev->ac_charging = chg_dev->ac_valid;


	printf("ac_charging=%d\n", chg_dev->ac_charging);
	printf("usb_pc_charging=%d\n", chg_dev->usb_pc_charging);
	printf("usb_adapter_charging=%d\n",	chg_dev->usb_adapter_charging);
	printf("usb_det=%d ac_det=%d\n",chg_dev->usb_det, chg_dev->ac_det);
}

static void axp_charger_update_usb_state(unsigned long data)
{
	struct axp_charger_dev *chg_dev = (struct axp_charger_dev *)data;

	axp_usb_ac_check_status(chg_dev);


}

static void axp_usb(struct work_struct *work)
{
	struct axp_charger_dev *chg_dev = container_of(work,
					struct axp_charger_dev, usbwork.work);
	struct axp_usb_info *usb = chg_dev->spy_info->usb;
	struct axp_ac_info *ac = chg_dev->spy_info->ac;

	printf(
				"[axp_usb] axp_usbcurflag = %d\n",
				axp_usbcurflag);
	axp_charger_update_state(chg_dev);

	if (chg_dev->in_short) {
		/* usb and ac in short*/
		if (!chg_dev->usb_valid) {
			/*usb or usb adapter can not be used*/
			printf(
				"USB not insert!\n");
			usb->set_usb_ihold(chg_dev, 500);
		} else if (CHARGE_USB_20 == axp_usbcurflag) {
			if (usb->usb_pc_cur) {
				printf(
						"set usb_pc_cur %d mA\n",
						usb->usb_pc_cur);
				usb->set_usb_ihold(chg_dev, usb->usb_pc_cur);
			} else {
				printf(
						"set usb_pc_cur 500 mA\n");
				usb->set_usb_ihold(chg_dev, 500);
			}
		} else if (CHARGE_USB_30 == axp_usbcurflag) {
			printf(
						"set usb_pc_cur 900 mA\n");
			usb->set_usb_ihold(chg_dev, 900);
		} else {
			/* usb adapter */
			if (usb->usb_ad_cur) {
				printf(
						"set usb_ad_cur %d mA\n",
							usb->usb_ad_cur);
			} else {
				printf(
						"set usb_ad_cur no limit\n");
			}
			usb->set_usb_ihold(chg_dev, usb->usb_ad_cur);
		}

		if (CHARGE_USB_20 == axp_usbvolflag) {
			if (usb->usb_pc_vol) {
				printf(
						"set usb_pc_vol %d mV\n",
							usb->usb_pc_vol);
				usb->set_usb_vhold(chg_dev, usb->usb_pc_vol);
			}
		} else if (CHARGE_USB_30 == axp_usbvolflag) {
			printf(
						"set usb_pc_vol 4700 mV\n");
			usb->set_usb_vhold(chg_dev, 4700);
		} else {
			if (usb->usb_ad_vol) {
				printf(
						"set usb_ad_vol %d mV\n",
							usb->usb_ad_vol);
				usb->set_usb_vhold(chg_dev, usb->usb_ad_vol);
			}
		}
	} else {
		if (!chg_dev->ac_valid && !chg_dev->usb_valid) {
			/*usb and ac can not be used*/
			printf(
						"AC and USB not insert!\n");
			usb->set_usb_ihold(chg_dev, 500);
		} else if (CHARGE_USB_20 == axp_usbcurflag) {
			if (usb->usb_pc_cur) {
				printf(
						"set usb_pc_cur %d mA\n",
							usb->usb_pc_cur);
				usb->set_usb_ihold(chg_dev, usb->usb_pc_cur);
			} else {
				printf(
						"set usb_pc_cur 500 mA\n");
				usb->set_usb_ihold(chg_dev, 500);
			}
		} else if (CHARGE_USB_30 == axp_usbcurflag) {
			printf(
						"set usb_pc_cur 900 mA\n");
			usb->set_usb_ihold(chg_dev, 900);
		} else {
			if ((usb->usb_ad_cur)) {
				AXP_DEBUG(AXP_CHG,
						chg_dev->chip->pmu_num,
						"set adapter cur %d mA\n",
						usb->usb_ad_cur);
			} else {
				AXP_DEBUG(AXP_CHG,
						chg_dev->chip->pmu_num,
						"set adapter cur no limit\n");
			}
			usb->set_usb_ihold(chg_dev, usb->usb_ad_cur);
		}

		if (CHARGE_USB_20 == axp_usbvolflag) {
			if (usb->usb_pc_vol) {
				printf(
						"set usb_pc_vol %d mV\n",
							usb->usb_pc_vol);
				usb->set_usb_vhold(chg_dev, usb->usb_pc_vol);
			}
		} else if (CHARGE_USB_30 == axp_usbvolflag) {
			printf(
						"set usb_pc_vol 4700 mV\n");
			usb->set_usb_vhold(chg_dev, 4700);
		} else {
			if (ac->ac_vol) {
				printf(
						"set ac_vol %d mV\n",
							ac->ac_vol);
				ac->set_ac_vhold(chg_dev, ac->ac_vol);
			}
		}
	}
}

void axp_battery_update_vol(struct axp_charger_dev *chg_dev)
{
	s32 rest_vol = 0;
	struct axp_battery_info *batt = chg_dev->spy_info->batt;

	rest_vol = batt->get_rest_cap(chg_dev);

	if (rest_vol > 100) {
		printf(
			"AXP rest_vol = %d\n", rest_vol);
		chg_dev->rest_vol = 100;
	} else {
		chg_dev->rest_vol = rest_vol;
	}

	printf(
			"charger->rest_vol = %d\n", chg_dev->rest_vol);
}


static void axp_charging_monitor(struct work_struct *work)
{

	static s32 pre_rest_vol;
	static bool pre_bat_curr_dir;

	axp_charger_update_state(chg_dev);

	/* if no battery exist, then return */
	if (!chg_dev->bat_det) {

		axp_update_ictemp_status(chg_dev);

		AXP_DEBUG(AXP_MISC, chg_dev->chip->pmu_num,
				"charger->ic_temp = %d\n", chg_dev->ic_temp);
		return;
	}
	/* if battery hadn't been detectd before, register it as power supply
	 * now */
	if (!battery_initialized) {
			psy_cfg.drv_data = chg_dev;
			chg_dev->batt = power_supply_register(chg_dev->dev,
					&batt_desc, &psy_cfg);
			battery_initialized = true;
	}

	axp_charger_update_value(chg_dev);
	axp_update_temp_status(chg_dev);

	Aprintf(
			"charger->ic_temp = %d\n", chg_dev->ic_temp);
	printf(
			"charger->bat_temp = %d\n", chg_dev->bat_temp);
	printf(
			"charger->bat_vol = %d\n", chg_dev->bat_vol);
	printf(
			"charger->bat_cur = %d\n", chg_dev->bat_cur);
	printf(
			"charger->bat_discur = %d\n", chg_dev->bat_discur);
	printf(
			"charger->is_charging = %d\n", chg_dev->charging);
	printf(
			"charger->bat_current_direction = %d\n",
			chg_dev->bat_current_direction);
	printf(
			"charger->ext_valid = %d\n", chg_dev->ext_valid);
	chg_dev->private_debug(chg_dev);
	if (!plug_debounce) {
		if (chg_dev->private_debug)
			axp_battery_update_vol(chg_dev);
	} else {
		plug_debounce = 0;
	}


	/* if battery volume changed, inform uevent */
	if ((chg_dev->rest_vol - pre_rest_vol)
			|| (chg_dev->bat_current_direction != pre_bat_curr_dir)
		) {
		printf(	"battery vol change: %d->%d\n",
				pre_rest_vol, chg_dev->rest_vol);
		pre_rest_vol = chg_dev->rest_vol;
		pre_bat_curr_dir = chg_dev->bat_current_direction;
	}

	/* reschedule for the next time */
}

void axp_change(struct axp_charger_dev *chg_dev)
{
	printf("battery state change\n");
	axp_charger_update_state(chg_dev);
	axp_charger_update_value(chg_dev);

}


void axp_usbac_in(struct axp_charger_dev *chg_dev)
{
	struct axp_usb_info *usb = chg_dev->spy_info->usb;

	axp_usbcur(CHARGE_AC);
	axp_usbvol(CHARGE_AC);

	printf( "axp ac/usb in!\n");

	if (timer_pending(&chg_dev->usb_status_timer))
		del_timer_sync(&chg_dev->usb_status_timer);

	/* must limit the current now,
	 * and will again fix it while usb/ac detect finished!
	*/
	if (usb->usb_pc_cur)
		usb->set_usb_ihold(chg_dev, usb->usb_pc_cur);
	else
		usb->set_usb_ihold(chg_dev, 500);
	plug_debounce = 1;
	/* this is about 3.5s,
	* while the flag set in usb drivers after usb plugged
	*/
	axp_charger_update_state(chg_dev);
	mod_timer(&chg_dev->usb_status_timer,
				jiffies + msecs_to_jiffies(5000));
	axp_usb_ac_check_status(chg_dev);
}


void axp_usbac_out(struct axp_charger_dev *chg_dev)
{
	printf( "axp ac/usb out!\n");

	if (timer_pending(&chg_dev->usb_status_timer))
		del_timer_sync(&chg_dev->usb_status_timer);

	/* if we plugged usb & ac at the same time,
	 * then unpluged ac quickly while the usb driver
	 * do not finished detecting,
	 * the charger type is error!So delay the charger type report 2s
	*/
	mod_timer(&chg_dev->usb_status_timer,
					jiffies + msecs_to_jiffies(2000));
	axp_usb_ac_check_status(chg_dev);
}


void axp_capchange(struct axp_charger_dev *chg_dev)
{
	struct power_supply_config psy_cfg = {};

	printf("battery change\n");

	axp_charger_update_state(chg_dev);
	axp_charger_update_value(chg_dev);
	axp_battery_update_vol(chg_dev);

}

u8 axp_usb_in_isr(int irq, void *data)
{
	struct axp_charger_dev *chg_dev = data;

	axp_usb_connect = 1;
	axp_change(chg_dev);
	axp_usbac_in(chg_dev);

	return 0;
}

u8 axp_usb_out_isr(int irq, void *data)
{
	struct axp_charger_dev *chg_dev = data;

	axp_usb_connect = 0;
	axp_change(chg_dev);
	axp_usbac_out(chg_dev);

	return 0;
}

u8 axp_ac_in_isr(int irq, void *data)
{
	struct axp_charger_dev *chg_dev = data;

	axp_change(chg_dev);
	axp_usbac_in(chg_dev);

	return 0;
}

u8 axp_ac_out_isr(int irq, void *data)
{
	struct axp_charger_dev *chg_dev = data;

	axp_change(chg_dev);
	axp_usbac_out(chg_dev);

	return 0;
}

u8 axp_capchange_isr(int irq, void *data)
{
	struct axp_charger_dev *chg_dev = data;

	axp_capchange(chg_dev);

	return 0;
}

u8 axp_change_isr(int irq, void *data)
{
	struct axp_charger_dev *chg_dev = data;

	axp_change(chg_dev);

	return 0;
}

u8 axp_low_warning1_isr(int irq, void *data)
{
	struct axp_charger_dev *chg_dev = data;

	axp_change(chg_dev);

	return 0;
}

u8 axp_low_warning2_isr(int irq, void *data)
{
	struct axp_charger_dev *chg_dev = data;

	axp_change(chg_dev);

	return 0;

}


int axp_charger_dt_parse(struct axp_config_info *axp_config)
{


	axp_config->pmu_battery_rdc=       BATRDC;
	axp_config->pmu_battery_cap=         4000;
	axp_config->pmu_batdeten=               1;
	axp_config->pmu_chg_ic_temp=            0;
	axp_config->pmu_runtime_chgcur= INTCHGCUR / 1000;
	axp_config->pmu_suspend_chgcur=      1200;
	axp_config->pmu_shutdown_chgcur=     1200;
	axp_config->pmu_init_chgvol,    INTCHGVOL / 1000;
	axp_config->pmu_init_chgend_rate=  INTCHGENDRATE;
	axp_config->pmu_init_chg_enabled=       1;
	axp_config->pmu_init_bc_en=             0;
	axp_config->pmu_init_adc_freq= INTADCFREQ;
	axp_config->pmu_init_adcts_freq=     INTADCFREQC;
	axp_config->pmu_init_chg_pretime=  INTCHGPRETIME;
	axp_config->pmu_init_chg_csttime= INTCHGCSTTIME;
	axp_config->pmu_batt_cap_correct=       1;
	axp_config->pmu_chg_end_on_en=          0;
	axp_config->ocv_coulumb_100=            0;
	axp_config->pmu_bat_para1=        OCVREG0;
	axp_config->pmu_bat_para2=        OCVREG1;
	axp_config->pmu_bat_para3=        OCVREG2;
	axp_config->pmu_bat_para4=        OCVREG3;
	axp_config->pmu_bat_para5=        OCVREG4;
	axp_config->pmu_bat_para6=        OCVREG5;
	axp_config->pmu_bat_para7=        OCVREG6;
	axp_config->pmu_bat_para8=        OCVREG7;
	axp_config->pmu_bat_para9=        OCVREG8;
	axp_config->pmu_bat_para10=       OCVREG9;
	axp_config->pmu_bat_para11=       OCVREGA;
	axp_config->pmu_bat_para12=       OCVREGB;
	axp_config->pmu_bat_para13=       OCVREGC;
	axp_config->pmu_bat_para14=       OCVREGD;
	axp_config->pmu_bat_para15=       OCVREGE;
	axp_config->pmu_bat_para16=       OCVREGF;
	axp_config->pmu_bat_para17=      OCVREG10;
	axp_config->pmu_bat_para18=      OCVREG11;
	axp_config->pmu_bat_para19=      OCVREG12;
	axp_config->pmu_bat_para20=      OCVREG13;
	axp_config->pmu_bat_para21=      OCVREG14;
	axp_config->pmu_bat_para22=      OCVREG15;
	axp_config->pmu_bat_para23=      OCVREG16;
	axp_config->pmu_bat_para24=      OCVREG17;
	axp_config->pmu_bat_para25=      OCVREG18;
	axp_config->pmu_bat_para26=      OCVREG19;
	axp_config->pmu_bat_para27=      OCVREG1A;
	axp_config->pmu_bat_para28=      OCVREG1B;
	axp_config->pmu_bat_para29=      OCVREG1C;
	axp_config->pmu_bat_para30=      OCVREG1D;
	axp_config->pmu_bat_para31=      OCVREG1E;
	axp_config->pmu_bat_para32=      OCVREG1F;
	axp_config->pmu_ac_vol=              4400;
	axp_config->pmu_usbpc_vol=           4400;
	axp_config->pmu_ac_cur=                 0;
	axp_config->pmu_usbpc_cur=              0;
	axp_config->pmu_pwroff_vol=          3300;
	axp_config->pmu_pwron_vol=           2900;
	axp_config->pmu_battery_warning_level1=15;
	axp_config->pmu_battery_warning_level2= 0;
	axp_config->pmu_restvol_adjust_time=   30;
	axp_config->pmu_ocv_cou_adjust_time=   60;
	axp_config->pmu_chgled_func=            0;
	axp_config->pmu_chgled_type=            0;
	axp_config->pmu_bat_temp_enable=        0;
	axp_config->pmu_bat_charge_ltf=      0xA5;
	axp_config->pmu_bat_charge_htf=      0x1F;
	axp_config->pmu_bat_shutdown_ltf=    0xFC;
	axp_config->pmu_bat_shutdown_htf=    0x16;
	axp_config->pmu_bat_temp_para1=         0;
	axp_config->pmu_bat_temp_para2=         0;
	axp_config->pmu_bat_temp_para3=         0;
	axp_config->pmu_bat_temp_para4=         0;
	axp_config->pmu_bat_temp_para5=         0;
	axp_config->pmu_bat_temp_para6=         0;
	axp_config->pmu_bat_temp_para7=         0;
	axp_config->pmu_bat_temp_para8=         0;
	axp_config->pmu_bat_temp_para9=         0;
	axp_config->pmu_bat_temp_para10=        0;
	axp_config->pmu_bat_temp_para11=        0;
	axp_config->pmu_bat_temp_para12=        0;
	axp_config->pmu_bat_temp_para13=        0;
	axp_config->pmu_bat_temp_para14=        0;
	axp_config->pmu_bat_temp_para15=        0;
	axp_config->pmu_bat_temp_para16=        0;
	axp_config->pmu_bat_unused=             0;
	axp_config->power_start=                0;
	axp_config->pmu_ocv_en=                 1;
	axp_config->pmu_cou_en=                 1;
	axp_config->pmu_update_min_time=   UPDATEMINTIME;

	axp_config_obj = axp_config;
	return 0;
}

