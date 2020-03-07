/************************************************************************
*
* File Name: cyttsp5_common_interface.c
*
*  *   Version: v1.0
*
************************************************************************/
#include "tpd_sys.h"
#include "linux/firmware.h"
#include "cyttsp5_regs.h"
#include "cyttsp5_common_interface.h"
#ifdef CYP_SELFTEST_CONFIG
#include <linux/vmalloc.h>
#endif

struct cyttsp5_core_data *cyttsp5_data = NULL;

#ifdef CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP5_BINARY_FW_UPGRADE
extern int cyttsp5_tp_fw_upgrade(struct tpd_classdev_t *cdev, char *fwname, int fwname_len);
#endif

#ifdef CYP_SELFTEST_CONFIG
#define MAX_NAME_LEN 50
static char save_file_path[MAX_NAME_LEN] = {0};
static char save_file_name[MAX_NAME_LEN] = {0};
static int cyttsp5_test_result = 0;

extern int cyttsp5_read_thresholdfile(struct device *dev);
extern ssize_t cyttsp5_selftest_test_store(struct device *dev,
		const char *buf, size_t size);
extern ssize_t cyttsp5_selftest_test_show(struct device *dev, char *buf);
extern ssize_t cmcp_results_read(struct device *dev, char *buf, size_t count);
extern int cyttsp5_selftest_memory_alloc(struct device *dev);
extern void cyttsp_selftest_memory_free(struct device *dev);
extern int cyttsp5_selftest_memory_check(struct device *dev);
#endif

static int tpd_init_tpinfo(struct tpd_classdev_t *cdev)
{
	struct cyttsp5_cydata *cydata = NULL;
	unsigned int firmware_ver;

	cyp_info("%s\n", __func__);
	if ((cyttsp5_data == NULL) || (cyttsp5_data->cpdata == NULL)) {
		cyp_err("%s param is invalid\n", __func__);
	}

	cydata = &cyttsp5_data->sysinfo.cydata;

	switch (cyttsp5_data->cpdata->vendor_id) {
	case CYP_VENDOR_ID_0:
		strlcpy(cdev->ic_tpinfo.vendor_name, CYP_VENDOR_0_NAME, sizeof(cdev->ic_tpinfo.vendor_name));
		break;
	case CYP_VENDOR_ID_1:
		strlcpy(cdev->ic_tpinfo.vendor_name, CYP_VENDOR_1_NAME, sizeof(cdev->ic_tpinfo.vendor_name));
		break;
	case CYP_VENDOR_ID_2:
		strlcpy(cdev->ic_tpinfo.vendor_name, CYP_VENDOR_2_NAME, sizeof(cdev->ic_tpinfo.vendor_name));
		break;
	case CYP_VENDOR_ID_3:
		strlcpy(cdev->ic_tpinfo.vendor_name, CYP_VENDOR_3_NAME, sizeof(cdev->ic_tpinfo.vendor_name));
		break;
	default:
		strlcpy(cdev->ic_tpinfo.vendor_name, "Unknown", sizeof(cdev->ic_tpinfo.vendor_name));
		break;
	}
	snprintf(cdev->ic_tpinfo.tp_name, sizeof(cdev->ic_tpinfo.tp_name), "Cypress");
	cdev->ic_tpinfo.chip_model_id = TS_CHIP_CYTTSP;
	firmware_ver = cydata->fw_ver_major;
	firmware_ver = (firmware_ver << 8) | cydata->fw_ver_minor;
	cdev->ic_tpinfo.firmware_ver = firmware_ver;
	cdev->ic_tpinfo.config_ver = cydata->fw_ver_conf;
	cdev->ic_tpinfo.module_id = cyttsp5_data->cpdata->vendor_id;
	cdev->ic_tpinfo.i2c_addr = 0x24;

	return 0;
}

#ifdef EASYWAKE_TSG6
static int tpd_get_wakegesture(struct tpd_classdev_t *cdev)
{
	cdev->b_gesture_enable = cyttsp5_data->easy_wakeup_gesture;
	return 0;
}

#define CYTTSP5_WAKEGESTURE_CHANGE_MASK 0x80
static int tpd_enable_wakegesture(struct tpd_classdev_t *cdev, int enable)
{
	struct cyttsp5_core_data *cd = cyttsp5_data;

	if (!cd->sysinfo.ready) {
		cyp_err("%s no ready", __func__);
		return -ENODEV;
	}

	cyp_info("%s enable=%d, sleep_state=%d", __func__, enable, cd->sleep_state);
	mutex_lock(&cd->system_lock);
	if (cd->sleep_state == SS_SLEEP_OFF) {
		cd->easy_wakeup_gesture = enable;
	} else {
		cd->easy_wakeup_gesture_config = CYTTSP5_WAKEGESTURE_CHANGE_MASK | enable;
	}
	mutex_unlock(&cd->system_lock);

	return enable;
}

void cyttsp5_restore_wakegesture_flag(void)
{
	struct cyttsp5_core_data *cd = cyttsp5_data;

	if (cd->easy_wakeup_gesture_config & CYTTSP5_WAKEGESTURE_CHANGE_MASK) {
		mutex_lock(&cd->system_lock);
		cd->easy_wakeup_gesture = cd->easy_wakeup_gesture_config & 0x01;
		cd->easy_wakeup_gesture_config = 0;
		mutex_unlock(&cd->system_lock);
	}
}
#endif

#ifdef CYP_SELFTEST_CONFIG
static int tpd_test_save_file_path_store(struct tpd_classdev_t *cdev, const char *buf)
{
	memset(save_file_path, 0, sizeof(save_file_path));
	snprintf(save_file_path, sizeof(save_file_path), "%s", buf);

	cyp_info("save file path:%s.", save_file_path);

	return 0;
}

static int tpd_test_save_file_path_show(struct tpd_classdev_t *cdev, char *buf)
{
	ssize_t num_read_chars = 0;

	num_read_chars = snprintf(buf, PAGE_SIZE, "%s\n", save_file_path);

	return num_read_chars;
}

static int tpd_test_save_file_name_store(struct tpd_classdev_t *cdev, const char *buf)
{
	memset(save_file_name, 0, sizeof(save_file_name));
	snprintf(save_file_name, sizeof(save_file_name), "%s", buf);

	cyp_info("save file path:%s.", save_file_name);

	return 0;
}

static int tpd_test_save_file_name_show(struct tpd_classdev_t *cdev, char *buf)
{
	ssize_t num_read_chars = 0;

	num_read_chars = snprintf(buf, PAGE_SIZE, "%s\n", save_file_name);

	return num_read_chars;
}

void cyttsp5_set_test_result(int value)
{
	cyttsp5_test_result = value;
	cyp_info("%s cyttsp5_test_result=%x\n", __func__, cyttsp5_test_result);
}

static int tpd_test_cmd_show(struct tpd_classdev_t *cdev, char *buf)
{
	int len;
	struct cyttsp5_core_data *cd = cyttsp5_data;
	int tx_num = cd->sysinfo.sensing_conf_data.tx_num;
	int rx_num = cd->sysinfo.sensing_conf_data.rx_num;

	len = snprintf(buf, PAGE_SIZE, "%d,%d,%d,0", cyttsp5_test_result, tx_num, rx_num);
	cyp_info("%s tpd test resut:%s\n", __func__, buf);

	return len;
}

#define SELFTEST_BUF_SIZE 512
int cyttsp5_test_start(struct device *dev)
{
	char *status_buf = NULL;
	char cmd = '0';
	int ret = 0;

	status_buf = vmalloc(SELFTEST_BUF_SIZE);
	if (status_buf == NULL) {
		cyp_err("%s vmalloc failed\n", __func__);
		return -ENOMEM;
	}

	ret = cyttsp5_selftest_test_store(dev, &cmd, 1);
	if (ret < 0) {
		cyp_err("%s cmcp test store failed ret=%d!\n", __func__, ret);
		goto failed;
	}

	ret = cyttsp5_selftest_test_show(dev, status_buf);
	if (ret < 0) {
		cyp_err("%s cmcp test show failed ret=%d!\n", __func__, ret);
		goto failed;
	}

	cyp_info("%s %s", __func__, status_buf);

failed:
	vfree(status_buf);

	return ret;
}

static int cyttsp5_test_save_test_data(char *data_buf, int len)
{
	struct file *pfile = NULL;
	char filepath[128];
	loff_t pos;
	mm_segment_t old_fs;

	memset(filepath, 0, sizeof(filepath));
	snprintf(filepath, sizeof(filepath), "%s%s.csv", save_file_path, save_file_name);
	cyp_info("%s filepath=%s\n", __func__, filepath);

	pfile = filp_open(filepath, O_TRUNC | O_CREAT | O_RDWR, 0);
	if (IS_ERR(pfile)) {
		cyp_err("error occurred while opening file %s.", filepath);
		return -EIO;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_write(pfile, data_buf, len, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);

	return 0;
}

int cyttsp_test_result(struct device *dev)
{
	char *buf = NULL;
	int count = 0;
	int ret = 0;

	buf = vmalloc(CYTTSP_TEST_RESULT_SIZE);
	if (buf == NULL) {
		cyp_err("%s vmalloc failed\n", __func__);
		return -ENOMEM;
	}

	ret = cmcp_results_read(cyttsp5_data->dev, buf, CYTTSP_TEST_RESULT_SIZE);
	if (ret <= 0) {
		cyp_err("%s cmcp_results_read failed\n", __func__);
		goto result_read_failed;
	}
	count = ret;

	ret = cyttsp5_test_save_test_data(buf, count);
	if (ret < 0) {
		cyp_err("%s save test result failed\n", __func__);
	}

result_read_failed:
	vfree(buf);

	return ret;
}

static int tpd_test_cmd_store(struct tpd_classdev_t *cdev, const char *buf)
{
	unsigned long command = 0;
	int ret = 0;

	cyp_info("%s:enter\n", __func__);
	ret = kstrtoul(buf, 10, &command);
	if (ret) {
		cyp_err("invalid param:%s", buf);
		return -EIO;
	}

	if (command == TP_TEST_INIT) {
		/* init test result */
		cyttsp5_set_test_result(0);

		ret = cyttsp5_selftest_memory_alloc(cyttsp5_data->dev);
		if (ret < 0) {
			cyp_err("%s alloc cmcp memory failed!\n", __func__);
			return -ENOMEM;
		}

		ret = cyttsp5_read_thresholdfile(cyttsp5_data->dev);
		if (ret < 0) {
			cyp_err("%s alloc memory failed!\n", __func__);
			return -ENOMEM;
		}
	} else if (command == TP_TEST_START) {
		cyp_info("%s:start TP test.\n", __func__);
		ret = cyttsp5_selftest_memory_check(cyttsp5_data->dev);
		if (!ret) {
			cyp_err("%s no alloc\n", __func__);
			return -EINVAL;
		}

		ret = cyttsp5_test_start(cyttsp5_data->dev);
		if (ret < 0) {
			cyp_err("%s cyttsp5_test_start failed\n", __func__);
			return -EINVAL;
		}
		ret = cyttsp_test_result(cyttsp5_data->dev);
		if (ret < 0) {
			cyp_err("%s cyttsp_test_result failed\n", __func__);
			return -EINVAL;
		}
	} else if (command == TP_TEST_END) {
		cyp_info("%s:finish TP test.\n", __func__);
		cyttsp_selftest_memory_free(cyttsp5_data->dev);
	} else {
		cyp_err("invalid command %ld", command);
	}
	return 0;
}
#endif

void cyttsp5_tpd_register_fw_class(void)
{
	tpd_fw_cdev.get_tpinfo = tpd_init_tpinfo;
#ifdef CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP5_BINARY_FW_UPGRADE
	tpd_fw_cdev.tp_fw_upgrade = cyttsp5_tp_fw_upgrade;
#endif
#ifdef EASYWAKE_TSG6
	tpd_fw_cdev.get_gesture = tpd_get_wakegesture;
	tpd_fw_cdev.wake_gesture = tpd_enable_wakegesture;
#endif

#ifdef CYP_SELFTEST_CONFIG
	tpd_fw_cdev.tpd_test_set_save_filepath = tpd_test_save_file_path_store;
	tpd_fw_cdev.tpd_test_get_save_filepath = tpd_test_save_file_path_show;
	tpd_fw_cdev.tpd_test_set_save_filename = tpd_test_save_file_name_store;
	tpd_fw_cdev.tpd_test_get_save_filename = tpd_test_save_file_name_show;
	tpd_fw_cdev.tpd_test_set_cmd = tpd_test_cmd_store;
	tpd_fw_cdev.tpd_test_get_cmd = tpd_test_cmd_show;
	snprintf(save_file_path, sizeof(save_file_path), "%s", "/sdcard/");
	snprintf(save_file_name, sizeof(save_file_name), "%s", "cyttsp5_result");
#endif
}

bool cyttsp5_check_tp_register(void)
{
	return tpd_fw_cdev.TP_have_registered;
}

void cyttsp5_tp_register(void)
{
	tpd_fw_cdev.TP_have_registered = true;
}
