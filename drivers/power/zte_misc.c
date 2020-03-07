/*
 * Driver for zte misc functions
 * function1: used for translate hardware GPIO to SYS GPIO number
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include "zte_misc.h"

struct zte_misc_ops node_ops_list[] = {
	/*common node*/
	{"enable_to_shutdown", NULL, NULL, NULL, NULL},
	{"enable_to_dump_reg", NULL, NULL, NULL, NULL},
	/*charge policy node*/
	{"demo_charging_policy", NULL, NULL, NULL, NULL},
	{"expired_charging_policy", NULL, NULL, NULL, NULL},
	{"charging_time_sec", NULL, NULL, NULL, NULL},
	{"force_disching_sec", NULL, NULL, NULL, NULL},
	{"policy_cap_max", NULL, NULL, NULL, NULL},
	{"policy_cap_min", NULL, NULL, NULL, NULL},
	/*vendor node*/
	{"screen_on", NULL, NULL, NULL, NULL},
	{"wireless_charging_signal_good", NULL, NULL, NULL, NULL},
	{"super_charge_mode", NULL, NULL, NULL, NULL},
};

#define MAX_MODULE_NAME_LEN 64

static const struct of_device_id zte_misc_of_match[] = {
	{ .compatible = "zte-misc", },
	{ },
};
MODULE_DEVICE_TABLE(of, zte_misc_of_match);

#define CHARGER_BUF_SIZE 0x32

enum charger_types_oem charge_type_oem = CHARGER_TYPE_DEFAULT;
module_param_named(
	charge_type_oem, charge_type_oem, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
);

static int shipmode_zte = -1;
static int zte_misc_control_shipmode(const char *val, struct kernel_param *kp)
{
	struct power_supply	*batt_psy;
	int rc;
	const union power_supply_propval enable = {1,};

	rc = param_set_int(val, kp);
	if (rc) {
		pr_err("%s: error setting value %d\n", __func__, rc);
		return rc;
	}

	batt_psy = power_supply_get_by_name("battery");
	if (batt_psy) {
		if (shipmode_zte == 0) {
			rc = batt_psy->set_property(batt_psy,
					POWER_SUPPLY_PROP_SET_SHIP_MODE, &enable);
			if (rc)
				pr_err("Set shipmode failed %d\n", rc);

			pr_info("%s: enter shipmode 2s later\n", __func__);
		} else
			pr_info("%s: invalid value %d\n", __func__, shipmode_zte);
	} else
		pr_err("%s: batt_psy is NULL\n", __func__);

	return 0;
}
static int zte_misc_get_shipmode_node(char *val, struct kernel_param *kp)
{
	struct power_supply	*batt_psy;
	int rc;
	union power_supply_propval pval = {0,};

	batt_psy = power_supply_get_by_name("battery");
	if (batt_psy) {
		rc = batt_psy->get_property(batt_psy,
					POWER_SUPPLY_PROP_SET_SHIP_MODE, &pval);
		if (rc) {
			pr_err("Read shipmode node failed: %d\n", rc);
			return snprintf(val, CHARGER_BUF_SIZE, "%d", -1);
		}
		if (shipmode_zte == 0)
			shipmode_zte = 0;
		else if (pval.intval == 0)
			shipmode_zte = 1;
		else
			shipmode_zte = -1;
		pr_info("%s: shipmode status %d\n", __func__, shipmode_zte);
	} else
		pr_err("%s: batt_psy is NULL\n", __func__);

	return snprintf(val, CHARGER_BUF_SIZE, "%d", shipmode_zte);
}

module_param_call(shipmode_zte, zte_misc_control_shipmode, zte_misc_get_shipmode_node,
					&shipmode_zte, 0644);

static int design_capacity = -1;
static int zte_misc_get_design_capacity(char *val, struct kernel_param *kp)
{
	struct power_supply *bms_psy;
	union power_supply_propval prop = {0,};
	int zte_design_capacity = 0;
	int rc;

	bms_psy = power_supply_get_by_name("bms");
	if (bms_psy) {
		rc = bms_psy->get_property(bms_psy,
				POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, &prop);
		if (rc) {
			pr_err("failed to get battery capacity, error:%d.\n", rc);
			goto out;
		} else {
			zte_design_capacity = prop.intval;
			goto out;
		}
	} else {
		pr_err("bms_psy is NULL.\n");
	}

	bms_psy = power_supply_get_by_name("ti_bms");
	if (bms_psy) {
		rc = bms_psy->get_property(bms_psy,
				POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, &prop);
		if (rc) {
			pr_err("failed to get battery capacity, error:%d.\n", rc);
			goto out;
		} else {
			zte_design_capacity = prop.intval;
			goto out;
		}
	} else {
		pr_err("bms_psy is NULL.\n");
	}

	bms_psy = power_supply_get_by_name("on_bms");
	if (bms_psy) {
		rc = bms_psy->get_property(bms_psy,
				POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, &prop);
		if (rc) {
			pr_err("failed to get battery capacity, error:%d.\n", rc);
			goto out;
		} else {
			zte_design_capacity = prop.intval;
			goto out;
		}
	} else {
		pr_err("bms_psy is NULL.\n");
	}

	bms_psy = power_supply_get_by_name("max17055_bms");
	if (bms_psy) {
		rc = bms_psy->get_property(bms_psy,
				POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN, &prop);
		if (rc) {
			pr_err("failed to get battery capacity, error:%d.\n", rc);
			goto out;
		} else {
			zte_design_capacity = prop.intval;
			goto out;
		}
	} else {
		pr_err("batt_psy is NULL.\n");
	}
	pr_err("zty_debug the capacity is %d\n", zte_design_capacity);

out:
	return snprintf(val, CHARGER_BUF_SIZE, "%d", zte_design_capacity);
}
module_param_call(design_capacity, NULL, zte_misc_get_design_capacity,
			&design_capacity, 0644);

static int zte_misc_common_callback_set(const char *val, const struct kernel_param *kp)
{
	int i = 0, ret = 0;
	char node_name[MAX_MODULE_NAME_LEN] = {0, };

	memset(node_name, 0, sizeof(node_name));

	if (sscanf(kp->name, MODULE_PARAM_PREFIX"%s", node_name) != 1) {
		pr_info("sscanf node_name failed\n");
		return -EINVAL;
	}

	pr_err("%s: val %s, node name %s!!!\n", __func__, val, node_name);

	for (i = 0; i < ARRAY_SIZE(node_ops_list); i++) {
		if (!strncmp(node_ops_list[i].node_name, node_name, strlen(node_name))
					&& (strlen(node_ops_list[i].node_name) == strlen(node_name))
					&& (node_ops_list[i].set != NULL)) {
			ret = node_ops_list[i].set(val, node_ops_list[i].arg);
			if (ret < 0) {
				return ret;
			}
		}
	}

	return ret;
}

static int zte_misc_common_callback_get(char *val, const struct kernel_param *kp)
{
	int i = 0, ret = 0;
	char node_name[MAX_MODULE_NAME_LEN] = {0, };

	memset(node_name, 0, sizeof(node_name));

	if (sscanf(kp->name, MODULE_PARAM_PREFIX"%s", node_name) != 1) {
		pr_info("sscanf node_name failed\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(node_ops_list); i++) {
		if (!strncmp(node_ops_list[i].node_name, node_name, strlen(node_name))
					&& (strlen(node_ops_list[i].node_name) == strlen(node_name))
					&& (node_ops_list[i].get != NULL)) {
			ret = node_ops_list[i].get(val, node_ops_list[i].arg);
			if (ret < 0) {
				return ret;
			}
		}
	}

	pr_err("%s: val %s, node name %s!!!\n", __func__, val, node_name);

	return ret;
}

static struct kernel_param_ops zte_misc_common_callback = {
	.set = zte_misc_common_callback_set,
	.get = zte_misc_common_callback_get,
};

/*
  *Emode function to enable/disable 0% shutdown
  */
module_param_cb(enable_to_shutdown, &zte_misc_common_callback, NULL, 0644);

/*charge policy node*/
module_param_cb(demo_charging_policy, &zte_misc_common_callback, NULL, 0644);
module_param_cb(expired_charging_policy, &zte_misc_common_callback, NULL, 0644);
module_param_cb(charging_time_sec, &zte_misc_common_callback, NULL, 0644);
module_param_cb(force_disching_sec, &zte_misc_common_callback, NULL, 0644);
module_param_cb(policy_cap_max, &zte_misc_common_callback, NULL, 0644);
module_param_cb(policy_cap_min, &zte_misc_common_callback, NULL, 0644);

int zte_misc_register_callback(struct zte_misc_ops *node_ops, void *arg)
{
	int i = 0, result = 0;

	if ((node_ops == NULL) || (node_ops->node_name == NULL)) {
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(node_ops_list); i++) {
		if (!strncmp(node_ops_list[i].node_name, node_ops->node_name, strlen(node_ops->node_name))
					&& (strlen(node_ops_list[i].node_name) == strlen(node_ops->node_name))) {

			pr_info("%s: name[%d]: %s\n", __func__, i, node_ops_list[i].node_name);

			if ((!node_ops_list[i].get)	&& (!node_ops_list[i].set)
						&& (!node_ops_list[i].free) && (!node_ops_list[i].arg)) {
				node_ops_list[i].get = node_ops->get;
				node_ops_list[i].set = node_ops->set;
				node_ops_list[i].free = node_ops->free;
				node_ops_list[i].arg = (arg) ? (arg) : (node_ops->arg);
				result = 0;
			} else {
				pr_info("%s: register name[%d]: %s failed!!!\n", __func__, i,
						node_ops_list[i].node_name);
				result = -EFAULT;
			}

			break;
		}
	}

	result = (i >= ARRAY_SIZE(node_ops_list)) ? (-EINVAL) : (result);

	return result;
}
EXPORT_SYMBOL_GPL(zte_misc_register_callback);


/*
vendor_id       02     03    04     17    10     12      15         20          20
vendor_name  ATL   cos   BYD   BAK   LG    SONY  SANYO  samsung  samsung
resistance      10K   20K   33K   82K   180K  240K   330K     430K       470K
*/
#define BATTERY_VENDOR_NUM 9
int battery_vendor_id[BATTERY_VENDOR_NUM] = {02, 03, 04, 17, 10, 12, 15, 20, 20};
int resistance_kohm[BATTERY_VENDOR_NUM] = {10, 20, 33, 82, 180, 240, 330, 430, 470};
static int battery_module_pack_vendor = 0;
static int battery_module_pack_vendor_get(char *val, struct kernel_param *kp)
{
	struct power_supply *batt_psy;
	union power_supply_propval prop = {0,};
	int resistance = 0, i, rc;

	batt_psy = power_supply_get_by_name("bms");
	if (batt_psy) {
		rc = batt_psy->get_property(batt_psy,
				POWER_SUPPLY_PROP_RESISTANCE_ID, &prop);
		if (rc)
			pr_err("failed to battery module pack vendor, error:%d.\n", rc);
		else {
			resistance = prop.intval / 1000;
			if (resistance > (resistance_kohm[BATTERY_VENDOR_NUM - 1] * 109 / 100)
				|| resistance < (resistance_kohm[0] * 91 / 100)) {
				pr_err("resistance is out of range, %dkohm\n", resistance);
			} else {
				for (i = 0; i < (BATTERY_VENDOR_NUM - 1); i++) {
					if (resistance < resistance_kohm[i] * 109 / 100
						&& resistance > resistance_kohm[i] * 91 / 100) {
						battery_module_pack_vendor = battery_vendor_id[i];
						break;
					}
				}
				pr_info("battery resistance is %dkohm, battery_vendor_id = %2d\n",
					resistance, battery_module_pack_vendor);
			}
		}
	} else
		pr_err("batt_psy is NULL.\n");

	return snprintf(val, 0x4, "%2d", battery_module_pack_vendor);
}
module_param_call(battery_module_pack_vendor, NULL,
	battery_module_pack_vendor_get, &battery_module_pack_vendor, 0644);

/* static int __devinit zte_misc_probe(struct platform_device *pdev) */
static int zte_misc_probe(struct platform_device *pdev)
{
	pr_info("%s +++++\n", __func__);

	return 0;
}

/* static int __devexit zte_misc_remove(struct platform_device *pdev) */
static int	zte_misc_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver zte_misc_device_driver = {
	.probe		= zte_misc_probe,
	/* .remove	= __devexit_p(zte_misc_remove),  //zte jiangzhineng changed */
	.remove    = zte_misc_remove,
	.driver		= {
		.name	= "zte-misc",
		.owner	= THIS_MODULE,
		.of_match_table = zte_misc_of_match,
	}
};

int __init zte_misc_init(void)
{
	return platform_driver_register(&zte_misc_device_driver);
}

static void __exit zte_misc_exit(void)
{
	platform_driver_unregister(&zte_misc_device_driver);
}
fs_initcall(zte_misc_init);
module_exit(zte_misc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Misc driver for zte");
MODULE_ALIAS("platform:zte-misc");
