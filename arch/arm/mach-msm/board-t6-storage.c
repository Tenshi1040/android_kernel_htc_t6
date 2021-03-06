/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/export.h>
#include <asm/mach-types.h>
#include <asm/mach/mmc.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_iomap.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include "devices.h"

#if defined(CONFIG_MACH_T6_UL)
#include "board-t6ul.h"
#elif defined(CONFIG_MACH_T6_WL)
#include "board-t6wl.h"
#elif defined(CONFIG_MACH_T6_WHL)
#include "board-t6whl.h"
#elif defined(CONFIG_MACH_T6_UHL)
#include "board-t6uhl.h"
#endif

#include "board-storage-common-a.h"
//#include <mach/htc_4335_wl_reg.h>   //For 4335 workaround

/* APQ8064 has 4 SDCC controllers */
enum sdcc_controllers {
	SDCC1,
	SDCC2,
	SDCC3,
	SDCC4,
	MAX_SDCC_CONTROLLER
};

/* All SDCC controllers require VDD/VCC voltage */
static struct msm_mmc_reg_data mmc_vdd_reg_data[MAX_SDCC_CONTROLLER] = {
	/* SDCC1 : eMMC card connected */
	[SDCC1] = {
		.name = "sdc_vdd",
		.high_vol_level = 2950000,
		.low_vol_level = 2950000,
		.always_on = 1,
		.lpm_sup = 1,
		.lpm_uA = 9000,
		.hpm_uA = 200000, /* 200mA */
	},
	/* SDCC3 : External card slot connected */
	[SDCC3] = {
		.name = "sdc_vdd",
		.high_vol_level = 2950000,
		.low_vol_level = 2950000,
		.hpm_uA = 800000, /* 800mA */
	}
};

/* SDCC controllers may require voting for VDD IO voltage */
static struct msm_mmc_reg_data mmc_vdd_io_reg_data[MAX_SDCC_CONTROLLER] = {
	/* SDCC1 : eMMC card connected */
	[SDCC1] = {
		.name = "sdc_vdd_io",
		.always_on = 1,
		.high_vol_level = 1800000,
		.low_vol_level = 1800000,
		.hpm_uA = 200000, /* 200mA */
	},
	/* SDCC3 : External card slot connected */
	[SDCC3] = {
		.name = "sdc_vdd_io",
		.high_vol_level = 2950000,
		.low_vol_level = 1850000,
		.always_on = 1,
		/* Max. Active current required is 16 mA */
		.hpm_uA = 16000,
	}
};

static struct msm_mmc_slot_reg_data mmc_slot_vreg_data[MAX_SDCC_CONTROLLER] = {
	/* SDCC1 : eMMC card connected */
	[SDCC1] = {
		.vdd_data = &mmc_vdd_reg_data[SDCC1],
		.vdd_io_data = &mmc_vdd_io_reg_data[SDCC1],
	},
	/* SDCC3 : External card slot connected */
	[SDCC3] = {
		.vdd_data = &mmc_vdd_reg_data[SDCC3],
		.vdd_io_data = &mmc_vdd_io_reg_data[SDCC3],
	}
};

/* SDC1 pad data */
static struct msm_mmc_pad_drv sdc1_pad_drv_on_cfg[] = {
	{TLMM_HDRV_SDC1_CLK, GPIO_CFG_6MA},
	{TLMM_HDRV_SDC1_CMD, GPIO_CFG_6MA},
	{TLMM_HDRV_SDC1_DATA, GPIO_CFG_6MA}
};

static struct msm_mmc_pad_drv sdc1_pad_drv_off_cfg[] = {
	{TLMM_HDRV_SDC1_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC1_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC1_DATA, GPIO_CFG_2MA}
};

static struct msm_mmc_pad_pull sdc1_pad_pull_on_cfg[] = {
	{TLMM_PULL_SDC1_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC1_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC1_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_mmc_pad_pull sdc1_pad_pull_off_cfg[] = {
	{TLMM_PULL_SDC1_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC1_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC1_DATA, GPIO_CFG_PULL_UP}
};

/* SDC3 pad data */
static struct msm_mmc_pad_drv sdc3_pad_drv_on_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_10MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_10MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_10MA}
};
/* SDC3 for SDR104 */
static struct msm_mmc_pad_drv sdc3_pad_drv_on_SDR104_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_10MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_10MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_10MA}
};

static struct msm_mmc_pad_drv sdc3_pad_drv_off_cfg[] = {
	{TLMM_HDRV_SDC3_CLK, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_CMD, GPIO_CFG_2MA},
	{TLMM_HDRV_SDC3_DATA, GPIO_CFG_2MA}
};

static struct msm_mmc_pad_pull sdc3_pad_pull_on_cfg[] = {
	{TLMM_PULL_SDC3_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_mmc_pad_pull sdc3_pad_pull_off_cfg[] = {
	{TLMM_PULL_SDC3_CLK, GPIO_CFG_NO_PULL},
	{TLMM_PULL_SDC3_CMD, GPIO_CFG_PULL_UP},
	{TLMM_PULL_SDC3_DATA, GPIO_CFG_PULL_UP}
};

static struct msm_mmc_pad_pull_data mmc_pad_pull_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.on = sdc1_pad_pull_on_cfg,
		.off = sdc1_pad_pull_off_cfg,
		.size = ARRAY_SIZE(sdc1_pad_pull_on_cfg)
	},
	[SDCC3] = {
		.on = sdc3_pad_pull_on_cfg,
		.off = sdc3_pad_pull_off_cfg,
		.size = ARRAY_SIZE(sdc3_pad_pull_on_cfg)
	},
};

static struct msm_mmc_pad_drv_data mmc_pad_drv_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.on = sdc1_pad_drv_on_cfg,
		.off = sdc1_pad_drv_off_cfg,
		.size = ARRAY_SIZE(sdc1_pad_drv_on_cfg)
	},
	[SDCC3] = {
		.on = sdc3_pad_drv_on_cfg,
		.on_SDR104 = sdc3_pad_drv_on_SDR104_cfg,
		.off = sdc3_pad_drv_off_cfg,
		.size = ARRAY_SIZE(sdc3_pad_drv_on_cfg)
	},
};

static struct msm_mmc_pad_data mmc_pad_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.pull = &mmc_pad_pull_data[SDCC1],
		.drv = &mmc_pad_drv_data[SDCC1]
	},
	[SDCC3] = {
		.pull = &mmc_pad_pull_data[SDCC3],
		.drv = &mmc_pad_drv_data[SDCC3]
	},
};

static struct msm_mmc_pin_data mmc_slot_pin_data[MAX_SDCC_CONTROLLER] = {
	[SDCC1] = {
		.pad_data = &mmc_pad_data[SDCC1],
	},
	[SDCC3] = {
		.pad_data = &mmc_pad_data[SDCC3],
	},
};

#define MSM_MPM_PIN_SDC1_DAT1	17
#define MSM_MPM_PIN_SDC3_DAT1	21

#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
static unsigned int sdc1_sup_clk_rates[] = {
	400000, 24000000, 48000000, 96000000
};

static unsigned int t6_sdc1_slot_type = MMC_TYPE_MMC;
static struct mmc_platform_data sdc1_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
#ifdef CONFIG_MMC_MSM_SDC1_8_BIT_SUPPORT
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
#else
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
#endif
	.sup_clk_table	= sdc1_sup_clk_rates,
	.sup_clk_cnt	= ARRAY_SIZE(sdc1_sup_clk_rates),
	.slot_type      = &t6_sdc1_slot_type,
	.pin_data	= &mmc_slot_pin_data[SDCC1],
	.vreg_data	= &mmc_slot_vreg_data[SDCC1],
	.nonremovable   = 1,
	.hc_erase_group_def	=1,
	.uhs_caps   = MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50,
	.mpm_sdiowakeup_int = MSM_MPM_PIN_SDC1_DAT1,
	.msm_bus_voting_data = &sps_to_ddr_bus_voting_data,
	.bkops_support = 1,
};
static struct mmc_platform_data *t6_sdc1_pdata = &sdc1_data;
#else
static struct mmc_platform_data *t6_sdc1_pdata;
#endif

#if 1
#ifdef CONFIG_MMC_MSM_SDC3_SUPPORT
static unsigned int sdc3_sup_clk_rates[] = {
	400000, 24000000, 48000000, 96000000, 192000000
};

static unsigned int t6_sdc3_slot_type = MMC_TYPE_SD;
static struct mmc_platform_data sdc3_data = {
	.ocr_mask       = MMC_VDD_27_28 | MMC_VDD_28_29,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.sup_clk_table	= sdc3_sup_clk_rates,
	.sup_clk_cnt	= ARRAY_SIZE(sdc3_sup_clk_rates),
	.slot_type      = &t6_sdc3_slot_type,
	.pin_data	= &mmc_slot_pin_data[SDCC3],
	.vreg_data	= &mmc_slot_vreg_data[SDCC3],
	.status_gpio	= PM8921_GPIO_PM_TO_SYS(PM_SDC3_CDz),
	.status_irq	= PM8921_GPIO_IRQ(PM8921_IRQ_BASE, PM_SDC3_CDz),
	.irq_flags	= IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
	.is_status_gpio_active_low = 1,
	.xpc_cap	= 1,
	.uhs_caps	= (MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
			MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_DDR50 |
			MMC_CAP_UHS_SDR104 | MMC_CAP_MAX_CURRENT_800),
	.msm_bus_voting_data = &sps_to_ddr_bus_voting_data,
};
static struct mmc_platform_data *t6_sdc3_pdata = &sdc3_data;
#else
static struct mmc_platform_data *t6_sdc3_pdata;
#endif
#endif

/* ---- WIFI ---- */
#if 0
#define PM8XXX_GPIO_INIT(_gpio, _dir, _buf, _val, _pull, _vin, _out_strength, \
			_func, _inv, _disable) \
{ \
	.gpio	= PM8921_GPIO_PM_TO_SYS(_gpio), \
	.config	= { \
		.direction	= _dir, \
		.output_buffer	= _buf, \
		.output_value	= _val, \
		.pull		= _pull, \
		.vin_sel	= _vin, \
		.out_strength	= _out_strength, \
		.function	= _func, \
		.inv_int_pol	= _inv, \
		.disable_pin	= _disable, \
	} \
}

struct pm8xxx_gpio_init {
	unsigned			gpio;
	struct pm_gpio		config;
};
#endif

/* fix it: gpio 35 is for CIR_RST*/
#if 0
static struct pm8xxx_gpio_init wl_dev_wake_gpio =
	PM8XXX_GPIO_INIT(WL_DEV_WAKE, PM_GPIO_DIR_OUT,
					 PM_GPIO_OUT_BUF_CMOS, 0, PM_GPIO_PULL_NO,
					 PM_GPIO_VIN_S4, PM_GPIO_STRENGTH_LOW,
					 PM_GPIO_FUNC_NORMAL, 0, 0);
#endif

/* BCM4329 returns wrong sdio_vsn(1) when we read cccr,
 * we use predefined value (sdio_vsn=2) here to initial sdio driver well
 */
#if 0
static struct embedded_sdio_data t6_wifi_emb_data = {
	.cccr	= {
		.sdio_vsn	= 2,
		.multi_block	= 1,
		.low_speed	= 0,
		.wide_bus	= 0,
		.high_power	= 1,
		.high_speed	= 1,
	}
};

static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;

static int
t6_wifi_status_register(void (*callback)(int card_present, void *dev_id),
				void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;

	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static int t6_wifi_cd;	/* WiFi virtual 'card detect' status */

static unsigned int t6_wifi_status(struct device *dev)
{
	return t6_wifi_cd;
}

static unsigned int t6_wifislot_type = MMC_TYPE_SDIO_WIFI;
static unsigned int wifi_sup_clk_rates[] = {
	400000, 24000000, 48000000, 96000000
};
static struct mmc_platform_data t6_wifi_data = {
	.ocr_mask               = MMC_VDD_28_29,
	.status                 = t6_wifi_status,
	.register_status_notify = t6_wifi_status_register,
	.embedded_sdio          = &t6_wifi_emb_data,
	.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
	.slot_type = &t6_wifislot_type,
	.sup_clk_table = wifi_sup_clk_rates,
	.sup_clk_cnt = ARRAY_SIZE(wifi_sup_clk_rates),
	.uhs_caps = (MMC_CAP_UHS_SDR12 | MMC_CAP_UHS_SDR25 |
	             MMC_CAP_UHS_SDR50 | MMC_CAP_UHS_DDR50 |
	             MMC_CAP_UHS_SDR104 | MMC_CAP_MAX_CURRENT_800),
	.msm_bus_voting_data = &sps_to_ddr_bus_voting_data,
	.nonremovable   = 0,
};


int t6_wifi_set_carddetect(int val)
{
	printk(KERN_INFO "%s: %d\n", __func__, val);
	t6_wifi_cd = val;
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		printk(KERN_WARNING "%s: Nobody to notify\n", __func__);
	return 0;
}
EXPORT_SYMBOL(t6_wifi_set_carddetect);
#endif

/* SDCC definition */
#define BIT_HDRIV_PULL_NO      0
#define BIT_HDRIV_PULL_DOWN    1
#define BIT_HDRIV_PULL_KEEP    2
#define BIT_HDRIV_PULL_UP      3
#define HDRIV_STR_2MA          0
#define HDRIV_STR_4MA          1
#define HDRIV_STR_6MA          2
#define HDRIV_STR_8MA          3
#define HDRIV_STR_10MA         4
#define HDRIV_STR_12MA         5
#define HDRIV_STR_14MA         6
#define HDRIV_STR_16MA         7
#define HDRV_SDC_CMD_PULL_SHIFT               11
#define HDRV_SDC_DATA_PULL_SHIFT              9
#define HDRV_SDC_CLK_HDRV_SHIFT               6
#define HDRV_SDC_CMD_HDRV_SHIFT               3
#define HDRV_SDC_DATA_HDRV_SHIFT              0

int sdc_pad_gpio_config(unsigned int pad_addr, unsigned cmd_pull, unsigned data_pull, unsigned clk_str, unsigned cmd_str, unsigned data_str)
{
	unsigned long value = 0x0;
	value = (cmd_pull << HDRV_SDC_CMD_PULL_SHIFT) |	\
			(data_pull << HDRV_SDC_DATA_PULL_SHIFT)|	\
			(clk_str << HDRV_SDC_CLK_HDRV_SHIFT)	|	\
			(cmd_str << HDRV_SDC_CMD_HDRV_SHIFT)	|	\
			(data_str << HDRV_SDC_DATA_HDRV_SHIFT);

	writel(value, pad_addr);
	return 1;
}

#if 0
int t6_wifi_power(int on)
{
	return 0;
}
EXPORT_SYMBOL(t6_wifi_power);

int t6_wifi_reset(int on)
{
	printk(KERN_INFO "%s: do nothing\n", __func__);
	return 0;
}
#endif

#if 0
static int reg_set_l7_optimum_mode(void)
{
	static struct regulator *reg_l7;
	int rc;

	reg_l7 = regulator_get(NULL, "8921_l7");
	if (IS_ERR_OR_NULL(reg_l7)) {
		pr_err("[WLAN] could not get 8921_l7, rc = %ld\n",
				PTR_ERR(reg_l7));
		return -ENODEV;
	}

	if (!regulator_is_enabled(reg_l7)) {
		rc = regulator_enable(reg_l7);
		if (rc < 0) {
			pr_err("[WLAN] enable l7 failed, rc=%d\n", rc);
			return -EINVAL;
		}
	}

	rc = regulator_set_optimum_mode(reg_l7, 10000);
	if (rc < 0) {
		pr_err("[WLAN] set_optimum_mode l7 failed, rc=%d\n", rc);
		return -EINVAL;
	}

	return 0;
}
#endif

void __init t6_init_mmc(void)
{
	//wifi_status_cb = NULL;

	printk(KERN_INFO "t6: %s\n", __func__);

/* fix it: gpio 35 is for CIR_RST*/
#if 0
	wl_dev_wake_gpio.config.output_value = 0;
	pm8xxx_gpio_config(wl_dev_wake_gpio.gpio, &wl_dev_wake_gpio.config);
#endif
#if 0
	/* PM QoS for wifi*/
    t6_wifi_data.swfi_latency = msm_rpm_get_swfi_latency();
#endif
	/* SD CD pin does not work on EVM board */
	if (machine_is_m7_evm()) {
		t6_sdc3_pdata->status_gpio = 0;
		t6_sdc3_pdata->status_irq = 0;
	}
	apq8064_add_sdcc(1, t6_sdc1_pdata);
	apq8064_add_sdcc(3, t6_sdc3_pdata);
	//apq8064_add_sdcc(3, &t6_wifi_data);
	//reg_set_l7_optimum_mode();
}
