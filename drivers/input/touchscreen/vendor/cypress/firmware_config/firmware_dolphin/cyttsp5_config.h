
/************************************************************************
*
* File Name: cyttsp5_config.h
*
* Version: v1.0
*
************************************************************************/
#ifndef _CYTTSP5_CONFIG_H_
#define _CYTTSP5_CONFIG_H_

/* Max sensor and button number */
#define CYTTSP5_MEMORY_OPTIMIZATION
#ifdef CYTTSP5_MEMORY_OPTIMIZATION
#define MAX_BUTTONS             (0)
#define MAX_SENSORS             (36)
#define MAX_TX_SENSORS          (6)
#define MAX_RX_SENSORS          (6)
#else
#define MAX_BUTTONS             (HID_SYSINFO_MAX_BTN)
#define MAX_SENSORS             (1024)
#define MAX_TX_SENSORS          (128)
#define MAX_RX_SENSORS          (128)
#endif

/* wakeup-gesture */
#define EASYWAKE_TSG6

#define CONFIG_TOUCHSCREEN_CYPRESS_CYTTSP5_BINARY_FW_UPGRADE

#define CYP_SELFTEST_CONFIG
#ifdef CYP_SELFTEST_CONFIG
#define CYTTSP5_SELFTEST_FILE "cyttsp5_thresholdfile.csv"
#define CYTTSP_TEST_RESULT_SIZE 8192
#endif

#define CYTTSP5_ESDCHECK_CONFIG

#define CY_FW_FILE_PREFIX	"cyttsp5_fw"
#define CY_FW_FILE_SUFFIX	".bin"
#define CY_FW_FILE_NAME		"cyttsp5_fw.bin"

#define CYP_VENDOR_ID_0 0
#define CYP_VENDOR_ID_1 1
#define CYP_VENDOR_ID_2 2
#define CYP_VENDOR_ID_3 3

#define CYP_VENDOR_0_NAME                         "Tianma"
#define CYP_VENDOR_1_NAME                         "unknown"
#define CYP_VENDOR_2_NAME                         "unknown"
#define CYP_VENDOR_3_NAME                         "unknown"

#define CYP_UPGRADE_FW0                  "cyttsp5_fw.bin"
#define CYP_UPGRADE_FW1                  "cyttsp5_fw.bin"
#define CYP_UPGRADE_FW2                  "cyttsp5_fw.bin"
#define CYP_UPGRADE_FW3                  "cyttsp5_fw.bin"
#endif
