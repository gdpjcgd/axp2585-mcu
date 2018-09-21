#include "axp15060.h"

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/reboot.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mfd/core.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/err.h>
//#include <linux/power/aw_pm.h>
#include "../axp-core.h"
#include "../axp-charger.h"
#include "../axp-regulator.h"
#include "axp15060-regu.h"

static struct axp_dev *axp15060_pm_power;
struct axp_config_info axp15060_config;
struct wakeup_source *axp15060_ws;
static int axp15060_pmu_num;

static const struct axp_compatible_name_mapping axp15060_cn_mapping[] = {
	{
		.device_name = "axp15060",
		.mfd_name = {
			.powerkey_name  = "axp15060-powerkey",
			.regulator_name = "axp15060-regulator",
			.gpio_name      = "axp15060-gpio",
		},
	},
};
static struct axp_regmap_irq_chip axp15060_regmap_irq_chip = {
	.name        = "axp15060_irq_chip",
	.status_base = AXP15060_INTSTS1,
	.enable_base = AXP15060_INTEN1,
	.num_regs    = 3,

};

static struct resource axp15060_pek_resources[] = {
	{AXP15060_IRQ_PEKRE, AXP15060_IRQ_PEKRE, "PEK_DBR",      IORESOURCE_IRQ,},
	{AXP15060_IRQ_PEKFE, AXP15060_IRQ_PEKFE, "PEK_DBF",      IORESOURCE_IRQ,},
};

static struct resource axp15060_regulator_resources[] = {
	{AXP15060_IRQ_DC6UN, AXP15060_IRQ_DC6UN, "dc6 under 85", IORESOURCE_IRQ,},
	{AXP15060_IRQ_DC5UN, AXP15060_IRQ_DC5UN, "dc5 under 85", IORESOURCE_IRQ,},
	{AXP15060_IRQ_DC4UN, AXP15060_IRQ_DC4UN, "dc4 under 85", IORESOURCE_IRQ,},
	{AXP15060_IRQ_DC3UN, AXP15060_IRQ_DC3UN, "dc3 under 85", IORESOURCE_IRQ,},
	{AXP15060_IRQ_DC2UN, AXP15060_IRQ_DC2UN, "dc2 under 85", IORESOURCE_IRQ,},
	{AXP15060_IRQ_DC1UN, AXP15060_IRQ_DC1UN, "dc1 under 85", IORESOURCE_IRQ,},
	{AXP15060_IRQ_LOWN1, AXP15060_IRQ_LOWN1, "low warning1", IORESOURCE_IRQ,},
	{AXP15060_IRQ_LOWN2, AXP15060_IRQ_LOWN2, "low warning2", IORESOURCE_IRQ,},
	{AXP15060_IRQ_DC3OV, AXP15060_IRQ_DC3OV, "dc3 over cur", IORESOURCE_IRQ,},
	{AXP15060_IRQ_DC2OV, AXP15060_IRQ_DC2OV, "dc2 over cur", IORESOURCE_IRQ,},
};

static struct mfd_cell axp15060_cells[] = {
	{
		.name          = "axp15060-powerkey",
		.num_resources = ARRAY_SIZE(axp15060_pek_resources),
		.resources     = axp15060_pek_resources,
	},
	{
		.name          = "axp15060-regulator",
		.num_resources = ARRAY_SIZE(axp15060_regulator_resources),
		.resources     = axp15060_regulator_resources,
	},
	{
		.name          = "axp15060-gpio",
	},
};

void axp15060_power_off(void)
{
	pr_info("[axp] send power-off command!\n");
	axp_regmap_write(axp15060_pm_power->regmap,
				AXP15060_PWR_DISABLE_DOWN, 0x80);
}

static int axp15060_init_chip(struct axp_dev *axp15060)
{
	uint8_t chip_id;
	int err;

	err = axp_regmap_read(axp15060->regmap, AXP15060_IC_TYPE, &chip_id);
	if (err) {
		pr_err("[%s] try to read chip id failed!\n",
				axp_name[axp15060_pmu_num]);
	//	return err;
	}
	if (((chip_id & 0xc0) == 0x40) &&
		((chip_id & 0x0f) == 0x04)
		) {
		pr_info("[%s] chip id detect 0x%x !\n",
				axp_name[axp15060_pmu_num], chip_id);
	} else {
		pr_info("[%s] chip id not detect 0x%x !\n",
				axp_name[axp15060_pmu_num], chip_id);
	}

	/* enable dcdc2 dvm */
	err = axp_regmap_update(axp15060->regmap, AXP15060_DCDC_MODE_CTL1,
				0x4, 0x4);
	if (err) {
		pr_err("[%s] enable dcdc2 dvm failed!\n",
				axp_name[axp15060_pmu_num]);
		return err;
	} else {
		pr_info("[%s] enable dcdc2 dvm.\n",
				axp_name[axp15060_pmu_num]);
	}

	/*init irq wakeup en*/
	if (axp15060_config.pmu_irq_wakeup)
		axp_regmap_set_bits(axp15060->regmap,
				AXP15060_IRQ_PWROK_VOFF, 0x80);
	else
		axp_regmap_clr_bits(axp15060->regmap,
				AXP15060_IRQ_PWROK_VOFF, 0x80);

	/*init pmu over temperature protection*/
	if (axp15060_config.pmu_hot_shutdown)
		axp_regmap_set_bits(axp15060->regmap,
				AXP15060_PWR_DISABLE_DOWN, 0x02);
	else
		axp_regmap_clr_bits(axp15060->regmap,
				AXP15060_PWR_DISABLE_DOWN, 0x02);

	return 0;
}

static void axp15060_wakeup_event(void)
{
	__pm_wakeup_event(axp15060_ws, 0);
}

static s32 axp15060_usb_det(void)
{
	return 0;
}

static s32 axp15060_usb_vbus_output(int high)
{
	return 0;
}

static const char *axp15060_get_pmu_name(void)
{
	return axp_name[axp15060_pmu_num];
}

static struct axp_dev *axp15060_get_pmu_dev(void)
{
	return axp15060_pm_power;
}

struct axp_platform_ops axp15060_platform_ops = {
	.usb_det = axp15060_usb_det,
	.usb_vbus_output = axp15060_usb_vbus_output,
	.get_pmu_name = axp15060_get_pmu_name,
	.get_pmu_dev  = axp15060_get_pmu_dev,

};

static struct of_device_id axp15060_match[] = {
	{ .compatible = "axp15060", },
	{},
};
MODULE_DEVICE_TABLE(of, axp15060_match);
static int axp15060_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int ret;
	struct axp_dev *axp15060;
	struct device_node *node;
	struct device *device;
	node = client->dev.of_node;
	device = &client->dev;

	axp15060_pmu_num = axp_get_pmu_num(axp15060_cn_mapping,
				ARRAY_SIZE(axp15060_cn_mapping));
	if (axp15060_pmu_num < 0) {
		pr_err("%s: get pmu num failed\n", __func__);
		return axp15060_pmu_num;
	}

	if (node) {
		/* get dt and sysconfig */
		if (!of_device_is_available(node)) {
			axp15060_config.pmu_used = 0;
			pr_err("%s: pmu_used = %u\n", __func__,
					axp15060_config.pmu_used);
			return -EPERM;
		} else {
			axp15060_config.pmu_used = 1;
			ret = axp_dt_parse(node, axp15060_pmu_num,
					&axp15060_config);
			if (ret) {
				pr_err("%s parse device tree err\n", __func__);
				return -EINVAL;
			}
		}
	} else {
		pr_err("axp15060 device tree err!\n");
		return -EBUSY;
	}

	axp15060 = devm_kzalloc(device, sizeof(*axp15060), GFP_KERNEL);
	if (!axp15060)
		return -ENOMEM;

	axp15060->dev = device;
	axp15060->nr_cells = ARRAY_SIZE(axp15060_cells);
	axp15060->cells = axp15060_cells;
	axp15060->pmu_num = axp15060_pmu_num;
	axp15060->is_slave = axp15060_config.pmu_as_slave;
	axp15060->irq = client->irq;
    printk("[axp15060]client->int irq=%d\ in %sn",client->irq,__func__);
	if (axp15060->irq < 0) {
		pr_err("axp15060 get irq error!\n");
		return -EINVAL;
	}
  printk("===line=%d===func=%s===in%s====\n",__LINE__,__func__,__FILE__);
	ret = axp_mfd_cell_name_init(axp15060_cn_mapping,
				ARRAY_SIZE(axp15060_cn_mapping), axp15060->pmu_num,
				axp15060->nr_cells, axp15060->cells);
	if (ret)
		return ret;

	axp15060->regmap = axp_regmap_init_i2c(device);

	if (IS_ERR(axp15060->regmap)) {
		ret = PTR_ERR(axp15060->regmap);
		dev_err(device, "regmap init failed: %d\n", ret);
		return ret;
	}
	  printk("===line=%d===func=%s===in%s====\n",__LINE__,__func__,__FILE__);
	i2c_set_clientdata(client, axp15060);

	ret = axp15060_init_chip(axp15060);
	if (ret)
		return ret;

	ret = axp_mfd_add_devices(axp15060);
	if (ret) {
		dev_err(axp15060->dev, "failed to add MFD devices: %d\n", ret);
		return ret;
	}
	  printk("===line=%d===func=%s===in%s====\n",__LINE__,__func__,__FILE__);
#if 0
	axp15060->irq = gpio_to_irq(40);  //i2s0 mclk
	axp15060->irq_data = axp_irq_chip_register(axp15060->regmap,
		axp15060->irq,
		IRQF_SHARED | IRQF_NO_SUSPEND | IRQF_TRIGGER_LOW | IRQF_ONESHOT,
		&axp15060_regmap_irq_chip, NULL);
#else
	if (axp15060->is_slave) {
		axp15060->irq_data = axp_irq_chip_register(axp15060->regmap,
				axp15060->irq,
				IRQF_SHARED | IRQF_NO_SUSPEND,
				&axp15060_regmap_irq_chip,
				NULL);
	} else {
		axp15060->irq_data = axp_irq_chip_register(axp15060->regmap,
				axp15060->irq,
				IRQF_SHARED | IRQF_NO_SUSPEND,
				&axp15060_regmap_irq_chip,
				axp15060_wakeup_event);
	}

	if (IS_ERR(axp15060->irq_data)) {
		ret = PTR_ERR(axp15060->irq_data);
		dev_err(device, "axp init irq failed: %d\n", ret);
		return ret;
	}
#endif
	axp15060_pm_power = axp15060;
	if (!axp15060->is_slave) {
		if (!pm_power_off)
			pm_power_off = axp15060_power_off;
	}
	  printk("===line=%d===func=%s===in%s====\n",__LINE__,__func__,__FILE__);
	axp_platform_ops_set(axp15060->pmu_num, &axp15060_platform_ops);

	if (!axp15060->is_slave)
		axp15060_ws = wakeup_source_register("axp15060_wakeup_source");

	return 0;
}

static int axp15060_remove(struct i2c_client *client)

{
	struct axp_dev *axp15060 = i2c_get_clientdata(client);

	if (axp15060 == axp15060_pm_power) {
		axp15060_pm_power = NULL;
		pm_power_off = NULL;
	}

	axp_mfd_remove_devices(axp15060);
	axp_irq_chip_unregister(axp15060->irq, axp15060->irq_data);

	return 0;
}

static const struct i2c_device_id axp15060_id_table[] = {
	{ "axp15060", 0 },
};
static const unsigned short axp15060_i2c_addr[] = {
	0x6c,
	I2C_CLIENT_END,
};

static int axp15060_i2c_detect(struct i2c_client *client,
					struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	if (adapter->nr == 0) {
		strlcpy(info->type, "axp15060", I2C_NAME_SIZE);
		return 0;
	}
	return -ENODEV;
}

static struct i2c_driver axp15060_driver = {

	.driver = {
		.name   = "axp15060",
		.owner  = THIS_MODULE,
		.of_match_table = axp15060_match,
	},
	.probe      = axp15060_probe,
	.remove     = axp15060_remove,
#ifdef CONFIG_AXP_TWI_USED
	.id_table   = axp15060_id_table,
	.address_list = axp15060_i2c_addr,
	.detect	=	axp15060_i2c_detect,
#endif
};

static int __init axp15060_init(void)
{
	int ret;
	ret = i2c_add_driver(&axp15060_driver);

	if (ret != 0)
		pr_err("Failed to register axp15060 driver: %d\n", ret);
	return ret;
}
subsys_initcall(axp15060_init);

static void __exit axp15060_exit(void)
{
	i2c_del_driver(&axp15060_driver);
}
module_exit(axp15060_exit);

MODULE_DESCRIPTION("PMIC Driver for AXP15060");
MODULE_AUTHOR("Qin <qinyongshen@allwinnertech.com>");
MODULE_LICENSE("GPL");
