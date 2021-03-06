/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RTC_H
#define RTC_H

#define PMIC_RG_SRCLKEN_IN0_HW_MODE_MASK	(1U)
#define PMIC_RG_SRCLKEN_IN0_HW_MODE_SHIFT	(1U)
#define PMIC_RG_SRCLKEN_IN1_HW_MODE_MASK	(1U)
#define PMIC_RG_SRCLKEN_IN1_HW_MODE_SHIFT	(3U)
#define PMIC_RG_RTC_EOSC32_CK_PDN_MASK		(1U)
#define PMIC_RG_RTC_EOSC32_CK_PDN_SHIFT		(2U)
#define PMIC_RG_EOSC_CALI_TD_MASK		(7U)
#define PMIC_RG_EOSC_CALI_TD_SHIFT		(5U)
#define PMIC_RG_XO_EN32K_MAN_MASK		(1U)
#define PMIC_RG_XO_EN32K_MAN_SHIFT		(0U)

/* RTC registers */
enum {
	RTC_BBPU = 0x0588,
	RTC_IRQ_STA = 0x058A,
	RTC_IRQ_EN = 0x058C,
	RTC_CII_EN = 0x058E
};

enum {
	RTC_AL_SEC = 0x05A0,
	RTC_AL_MIN = 0x05A2,
	RTC_AL_HOU = 0x05A4,
	RTC_AL_DOM = 0x05A6,
	RTC_AL_DOW = 0x05A8,
	RTC_AL_MTH = 0x05AA,
	RTC_AL_YEA = 0x05AC,
	RTC_AL_MASK = 0x0590
};

enum {
	RTC_OSC32CON = 0x05AE,
	RTC_CON = 0x05C4,
	RTC_WRTGR = 0x05C2
};

enum {
	RTC_PDN1 = 0x05B4,
	RTC_PDN2 = 0x05B6,
	RTC_SPAR0 = 0x05B8,
	RTC_SPAR1 = 0x05BA,
	RTC_PROT = 0x05BC,
	RTC_DIFF = 0x05BE,
	RTC_CALI = 0x05C0
};

enum {
	RTC_OSC32CON_UNLOCK1 = 0x1A57,
	RTC_OSC32CON_UNLOCK2 = 0x2B68
};

enum {
	RTC_PROT_UNLOCK1 = 0x586A,
	RTC_PROT_UNLOCK2 = 0x9136
};

enum {
	RTC_BBPU_PWREN	= 1U << 0,
	RTC_BBPU_CLR	= 1U << 1,
	RTC_BBPU_INIT	= 1U << 2,
	RTC_BBPU_AUTO	= 1U << 3,
	RTC_BBPU_CLRPKY	= 1U << 4,
	RTC_BBPU_RELOAD	= 1U << 5,
	RTC_BBPU_CBUSY	= 1U << 6
};

enum {
	RTC_AL_MASK_SEC = 1U << 0,
	RTC_AL_MASK_MIN = 1U << 1,
	RTC_AL_MASK_HOU = 1U << 2,
	RTC_AL_MASK_DOM = 1U << 3,
	RTC_AL_MASK_DOW = 1U << 4,
	RTC_AL_MASK_MTH = 1U << 5,
	RTC_AL_MASK_YEA = 1U << 6
};

enum {
	RTC_BBPU_AUTO_PDN_SEL = 1U << 6,
	RTC_BBPU_2SEC_CK_SEL = 1U << 7,
	RTC_BBPU_2SEC_EN = 1U << 8,
	RTC_BBPU_2SEC_MODE = 0x3 << 9,
	RTC_BBPU_2SEC_STAT_CLEAR = 1U << 11,
	RTC_BBPU_2SEC_STAT_STA = 1U << 12
};

enum {
	RTC_BBPU_KEY	= 0x43 << 8
};

enum {
	RTC_EMBCK_SRC_SEL	= 1 << 8,
	RTC_EMBCK_SEL_MODE	= 3 << 6,
	RTC_XOSC32_ENB		= 1 << 5,
	RTC_REG_XOSC32_ENB	= 1 << 15
};

enum {
	RTC_K_EOSC_RSV_0	= 1 << 8,
	RTC_K_EOSC_RSV_1	= 1 << 9,
	RTC_K_EOSC_RSV_2	= 1 << 10
};

/* PMIC TOP Register Definition */
enum {
	PMIC_RG_TOP_CON = 0x001E,
	PMIC_RG_TOP_CKPDN_CON1 = 0x0112,
	PMIC_RG_TOP_CKPDN_CON1_SET = 0x0114,
	PMIC_RG_TOP_CKPDN_CON1_CLR = 0x0116,
	PMIC_RG_TOP_CKSEL_CON0 = 0x0118,
	PMIC_RG_TOP_CKSEL_CON0_SET = 0x011A,
	PMIC_RG_TOP_CKSEL_CON0_CLR = 0x011C
};

/* PMIC SCK Register Definition */
enum {
	PMIC_RG_SCK_TOP_CKPDN_CON0 = 0x051A,
	PMIC_RG_SCK_TOP_CKPDN_CON0_SET = 0x051C,
	PMIC_RG_SCK_TOP_CKPDN_CON0_CLR = 0x051E,
	PMIC_RG_EOSC_CALI_CON0 = 0x540
};

/* PMIC DCXO Register Definition */
enum {
	PMIC_RG_DCXO_CW00 = 0x0788,
	PMIC_RG_DCXO_CW02 = 0x0790
};

/* external API */
uint16_t RTC_Read(uint32_t addr);
void RTC_Write(uint32_t addr, uint16_t data);
int32_t rtc_busy_wait(void);
int32_t RTC_Write_Trigger(void);
int32_t Writeif_unlock(void);
void rtc_power_off_sequence(void);

#endif /* RTC_H */
