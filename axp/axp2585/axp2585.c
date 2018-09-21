
#include "../axp-core.h"
#include "../axp-charger.h"
#include "axp2585.h"

static struct axp_dev *axp2585_pm_power;
struct axp_config_info axp2585_config;
struct wakeup_source *axp2585_ws;
static int axp2585_pmu_num;

void axp2585_power_off(void)
{
	pr_info("[%s] send power-off command!\n", axp_name[axp2585_pmu_num]);
	axp_regmap_set_bits(axp2585_pm_power->regmap, 0x10, 0x80); /* enable */
	axp_regmap_set_bits(axp2585_pm_power->regmap, 0x17, 0x01);
	mdelay(20);
	pr_warn("[%s] warning!!! axp can't power-off,\"\
		\" maybe some error happened!\n", axp_name[axp2585_pmu_num]);
}

static int axp2585_init_chip(struct axp_dev *axp2585)
{
	uint8_t chip_id;
	int err;

	err = axp_regmap_read(axp2585->regmap, AXP2585_IC_TYPE, &chip_id);
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
		axp_regmap_set_bits(axp2585->regmap, 0x17, 0x10); /* enable */
	else
		axp_regmap_clr_bits(axp2585->regmap, 0x17, 0x10); /* disable */

	/*Init PMU Over Temperature protection*/
	if (axp2585_config.pmu_hot_shutdown)
		axp_regmap_set_bits(axp2585->regmap, 0xf3, 0x08); /* enable */
	else
		axp_regmap_clr_bits(axp2585->regmap, 0xf3, 0x08); /* disable */

	/*enable send seq to pmu when power off */
	axp_regmap_update(axp2585->regmap, 0x16, 0x40, 0xc0);
	return 0;
}


static s32 axp2585_usb_det(void)
{
	u8 value = 0;
	int ret = 0;

	axp_regmap_read(axp2585_pm_power->regmap, 0x0, &value);
	if (value & 0x02) {
		axp_usb_connect = 1;
		ret = 1;
	}
	return ret;
}

static s32 axp2585_usb_vbus_output(int high)
{
	u8 ret = 0;

	if (high) {
		ret = axp_regmap_set_bits_sync(axp2585_pm_power->regmap,
								0x11, 0x40);
		if (ret)
			return ret;
	} else {
		ret = axp_regmap_clr_bits_sync(axp2585_pm_power->regmap,
								0x11, 0x40);
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

	struct device *device = &client->dev;
	struct device_node *node = client->dev.of_node;

	axp2585_pmu_num = axp_get_pmu_num(axp2585_cn_mapping,
				ARRAY_SIZE(axp2585_cn_mapping));
	if (axp2585_pmu_num < 0) {
		pr_err("%s get pmu num failed\n", __func__);
		return axp2585_pmu_num;
	}

	if (node) {
		/* get dt and sysconfig */
		if (!of_device_is_available(node)) {
			axp2585_config.pmu_used = 0;
			pr_err("%s: pmu_used = %u\n", __func__,
					axp2585_config.pmu_used);
			return -EPERM;
		} else {
			axp2585_config.pmu_used = 1;
			ret = axp_dt_parse(node, axp2585_pmu_num,
					&axp2585_config);
			if (ret) {
				pr_err("%s parse device tree err\n", __func__);
				return -EINVAL;
			}
		}
	} else {
		pr_err("AXP2585 device tree err!\n");
		return -EBUSY;
	}

	axp2585 = devm_kzalloc(device, sizeof(*axp2585), GFP_KERNEL);
	if (!axp2585)
		return -ENOMEM;

	axp2585->dev = device;
	axp2585->nr_cells = ARRAY_SIZE(axp2585_cells);
	axp2585->cells = axp2585_cells;
	axp2585->pmu_num = axp2585_pmu_num;

	ret = axp_mfd_cell_name_init(axp2585_cn_mapping,
				ARRAY_SIZE(axp2585_cn_mapping), axp2585->pmu_num,
				axp2585->nr_cells, axp2585->cells);
	if (ret)
		return ret;

	axp2585->regmap = axp_regmap_init_i2c(&client->dev);

	if (IS_ERR(axp2585->regmap)) {
		ret = PTR_ERR(axp2585->regmap);
		dev_err(device, "regmap init failed: %d\n", ret);
		return ret;
	}

	i2c_set_clientdata(client, axp2585);

	ret = axp2585_init_chip(axp2585);
	if (ret)
		return ret;

	ret = axp_mfd_add_devices(axp2585);
	if (ret) {
		dev_err(axp2585->dev, "failed to add MFD devices: %d\n", ret);
		return ret;
	}

	axp2585->irq = client->irq;

	axp2585->irq_data = axp_irq_chip_register(axp2585->regmap, axp2585->irq,
						IRQF_SHARED
						| IRQF_NO_SUSPEND,
						&axp2585_regmap_irq_chip,
						axp2585_wakeup_event);
	if (IS_ERR(axp2585->irq_data)) {
		ret = PTR_ERR(axp2585->irq_data);
		dev_err(device, "axp init irq failed: %d\n", ret);
		return ret;
	}

	axp2585_pm_power = axp2585;
/*
	if (!pm_power_off)
		pm_power_off = axp2585_power_off;
*/
	axp_platform_ops_set(axp2585->pmu_num, &axp2585_platform_ops);

	axp2585_ws = wakeup_source_register("axp2585_wakeup_source");

	return 0;
}


