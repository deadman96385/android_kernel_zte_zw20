#ifndef __POWER_ZTE_MISC__
#define __POWER_ZTE_MISC__

enum battery_sts {
	BATTERY_CHARGING = 0,
	BATTERY_DISCHARGING,/*set icl min value and disable charger*/
	BATTERY_NOT_CHARGING,/*only disable charger*/
	BATTERY_UNKNOWN
};

enum charging_policy_sts {
	NORMAL_CHARGING_POLICY = BIT(0),
	DEMO_CHARGING_POLICY = BIT(1),
	EXPIRED_CHARGING_POLICY = BIT(2),/*depends on EXPIRED_CHARGING_POLICY_ENABLE*/
	UNKNOWN_CHARGING_POLICY = BIT(3)
};

enum charger_types_oem {
	CHARGER_TYPE_DEFAULT = -1,
	CHARGER_TYPE_UNKNOWN = 0,
	CHARGER_TYPE_HVDCP = 1,
	CHARGER_TYPE_DCP = 2,
	CHARGER_TYPE_DCP_SLOW = 3,
	CHARGER_TYPE_SDP_NBC1P2 = 4,
	CHARGER_TYPE_SDP_NBC1P2_SLOW = 5,
	CHARGER_TYPE_SDP_NBC1P2_CHARGR_ERR = 6,
};

#define DEFAULT_CHARGING_POLICY NORMAL_CHARGING_POLICY
#define MIN_BATTERY_PROTECTED_PERCENT 50
#define MAX_BATTERY_PROTECTED_PERCENT 70
#define EXPIRED_CHARGING_POLICY_ENABLE 0
#define CHARGING_EXPIRATION_TIME_MS 64800000 /*18 h*/
#define CHARGING_POLICY_PERIOD 60000 /*60s*/
#define CHARGING_POLICY_LOWTEMP_THRESHOLD 150

struct zte_misc_ops {
	/* How the ops should behave */
	char *node_name;
	/* Returns 0, or -errno.  arg is in kp->arg. */
	int (*set)(const char *val, const void *arg);
	/* Returns length written or -errno.  Buffer is 4k (ie. be short!) */
	int (*get)(char *buffer, const void *arg);
	/* Optional function to free kp->arg when module unloaded. */
	void (*free)(void *arg);
	void *arg;
};

extern int zte_misc_register_callback(struct zte_misc_ops *node_ops, void *arg);
extern bool charger_policy_get_status(void);

#endif
