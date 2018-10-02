
#include "../axp-core.h"
#include "../axp-charger.h"
#include "axp2585.h"

static struct axp_dev *axp2585_pm_power;
struct axp_config_info axp2585_config;
static int axp2585_pmu_num;

void axp2585_power_off(void)
{
	pr_info("[%s] send power-off command!\n", axp_name[axp2585_pmu_num]);
	axp_i2c_set_bits( 0x10, 0x80); /* enable */
	axp_i2c_set_bits( 0x17, 0x01);
	mdelay(20);
	pr_warn("[%s] warning!!! axp can't power-off,\"\
		\" maybe some error happened!\n", axp_name[axp2585_pmu_num]);
}

static int axp2585_init_chip(struct axp_dev *axp2585)
{
	uint8_t chip_id;
	int err;

	err = axp_i2c_read(AXP2585_IC_TYPE, &chip_id);
	if (err) {
		pr_err("[%s] try to read chip id failed!\n",
				axp_name[axp2585_pmu_num]);
		return err;
	}

	/*only support axp2585*/
	if (((chip_id & 0xc0) == 0x40) &&
		((chip_id & 0x0f) == 0x06)
		) {
		pr_info("[%s] chip id detect 0x%x !\n",
				axp_name[axp2585_pmu_num], chip_id);
	} else {
		pr_info("[%s] chip id is error 0x%x !\n",
				axp_name[axp2585_pmu_num], chip_id);
	}

	/*Init IRQ wakeup en*/
	if (axp2585_config.pmu_irq_wakeup)
		axp_i2c_set_bits( 0x17, 0x10); /* enable */
	else
		axp_i2c_clr_bits( 0x17, 0x10); /* disable */

	/*Init PMU Over Temperature protection*/
	if (axp2585_config.pmu_hot_shutdown)
		axp_i2c_set_bits( 0xf3, 0x08); /* enable */
	else
		axp_i2c_clr_bits( 0xf3, 0x08); /* disable */

	/*enable send seq to pmu when power off */
	axp_i2c_update( 0x16, 0x40, 0xc0);
	return 0;
}


static s32 axp2585_usb_det(void)
{
	u8 value = 0;
	int ret = 0;

	axp_i2c_read( 0x0, &value);
	if (value & 0x02) {
		axp_usb_connect = 1;
		ret = 1;
	}
	return ret;
}

static s32 axp2585_boost_force_output(int high)
{
	u8 ret = 0;

	if (high) {
		ret = axp_i2c_set_bits(0x11, 0x40);
		if (ret)
			return ret;
	} else {
		ret = axp_i2c_clr_bits(0x11, 0x40);
		if (ret)
			return ret;
	}
	return ret;
}



static int axp2585_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int ret;
	struct axp_dev *axp2585;


	ret = axp2585_init_chip(axp2585);
	if (ret)
		return ret;

	}


