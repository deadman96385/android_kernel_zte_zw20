#ifndef _CYTTSP5_COMMON_INTERFACE_H_
#define _CYTTSP5_COMMON_INTERFACE_H_

#include "tpd_sys.h"

#define cyp_dbg(x...) pr_info("[cyttsp5] " x)
#define cyp_info(x...) pr_info("[cyttsp5] " x)
#define cyp_warn(x...) pr_info("[cyttsp5][WARNING] " x)
#define cyp_err(x...) pr_err("[cyttsp5][ERROR] " x)

extern struct cyttsp5_core_data *cyttsp5_data;

extern void cyttsp5_tpd_register_fw_class(void);
extern bool cyttsp5_check_tp_register(void);
extern void cyttsp5_tp_register(void);
#ifdef EASYWAKE_TSG6
extern void cyttsp5_restore_wakegesture_flag(void);
#endif

#ifdef CYP_SELFTEST_CONFIG
extern void cyttsp5_set_test_result(int value);
#endif

#endif

