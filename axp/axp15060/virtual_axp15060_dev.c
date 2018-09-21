#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/regulator/machine.h>
#include <linux/power_supply.h>
#include <linux/module.h>

static struct platform_device virt[] = {
	{
		.name = "axp15060-cs-dcdc1",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_dcdc1",
		}
	},
	{
		.name = "axp15060-cs-dcdc2",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_dcdc2",
		}
	},
	{
		.name = "axp15060-cs-dcdc3",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_dcdc3",
		}
	},
	{
		.name = "axp15060-cs-dcdc4",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_dcdc4",
		}
	},
	{
		.name = "axp15060-cs-dcdc5",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_dcdc5",
		}
	},
	{
		.name = "axp15060-cs-dcdc6",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_dcdc6",
		}
	},
	{
		.name = "axp15060-cs-aldo1",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_aldo1",
		}
	},
	{
		.name = "axp15060-cs-aldo2",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_aldo2",
		}
	},
	{
		.name = "axp15060-cs-aldo3",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_aldo3",
		}
	},
	{
		.name = "axp15060-cs-aldo4",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_aldo4",
		}
	},
	{
		.name = "axp15060-cs-aldo5",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_aldo5",
		}
	},
	{
		.name = "axp15060-cs-bldo1",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_bldo1",
		}
	},
	{
		.name = "axp15060-cs-bldo2",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_bldo2",
		}
	},
	{
		.name = "axp15060-cs-bldo3",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_bldo3",
		}
	},
	{
		.name = "axp15060-cs-bldo4",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_bldo4",
		}
	},
	{
		.name = "axp15060-cs-bldo5",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_bldo5",
		}
	},
	{
		.name = "axp15060-cs-cldo1",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_cldo1",
		}
	},
	{
		.name = "axp15060-cs-cldo2",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_cldo2",
		}
	},
	{
		.name = "axp15060-cs-cldo3",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_cldo3",
		}
	},
	{
		.name = "axp15060-cs-cldo4",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_cldo4",
		}
	},
	{
		.name = "axp15060-cs-rtcldo",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_rtcldo",
		}
	},
	{
		.name = "axp15060-cs-cpusldo",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_cpusldo",
		}
	},
	{
		.name = "axp15060-cs-dc1sw",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_dc1sw",
		}
	},
	{
		.name = "axp15060-cs-ldoio1",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_ldoio1",
		}
	},
	{
		.name = "axp15060-cs-ldoio2",
		.id = -1,
		.dev = {
			.platform_data = "axp15060_ldoio2",
		}
	},
};

static int __init virtual_init(void)
{
	int j, ret;

	for (j = 0; j < ARRAY_SIZE(virt); j++) {
		ret = platform_device_register(&virt[j]);
		if (ret)
				goto creat_devices_failed;
	}

	return ret;

creat_devices_failed:
	while (j--)
		platform_device_register(&virt[j]);
	return ret;

}

module_init(virtual_init);

static void __exit virtual_exit(void)
{
	int j;

	for (j = ARRAY_SIZE(virt) - 1; j >= 0; j--)
		platform_device_unregister(&virt[j]);
}
module_exit(virtual_exit);

MODULE_DESCRIPTION("Axp regulator test");
MODULE_AUTHOR("Kyle Cheung");
MODULE_LICENSE("GPL");
