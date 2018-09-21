#ifndef AXP152_REGU_H_
#define AXP152_REGU_H_

#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

enum {
	AXP152_ID_LDO0,
	AXP152_ID_LDO1,
	AXP152_ID_LDO2,
	AXP152_ID_LDO3,
	AXP152_ID_LDO4,
	AXP152_ID_LDO5,
	AXP152_ID_LDOIO0 = AXP_LDOIO_ID_START,
	AXP152_ID_DCDC1  = AXP_DCDC_ID_START,
	AXP152_ID_DCDC2,
	AXP152_ID_DCDC3,
	AXP152_ID_DCDC4,
	AXP152_REG_MAX,
};

/* AXP15 Regulator Registers */
#define AXP152_LDO0         AXP152_LDO0OUT_VOL
#define AXP152_RTC          AXP152_STATUS
#define AXP152_ALDO1        AXP152_LDO34OUT_VOL
#define AXP152_ALDO2        AXP152_LDO34OUT_VOL
#define AXP152_DLDO1        AXP152_LDO5OUT_VOL
#define AXP152_DLDO2        AXP152_LDO6OUT_VOL
#define AXP152_LDOIO0       AXP152_GPIO0_VOL

#define AXP152_DCDC1        AXP152_DC1OUT_VOL
#define AXP152_DCDC2        AXP152_DC2OUT_VOL
#define AXP152_DCDC3        AXP152_DC3OUT_VOL
#define AXP152_DCDC4        AXP152_DC4OUT_VOL

#define AXP152_LDO0EN       AXP152_LDO0_CTL
#define AXP152_RTCLDOEN     AXP152_STATUS
#define AXP152_ALDO1EN      AXP152_LDO3456_DC1234_CTL
#define AXP152_ALDO2EN      AXP152_LDO3456_DC1234_CTL
#define AXP152_DLDO1EN      AXP152_LDO3456_DC1234_CTL
#define AXP152_DLDO2EN      AXP152_LDO3456_DC1234_CTL
#define AXP152_LDOI0EN      AXP152_GPIO2_CTL

#define AXP152_DCDC1EN      AXP152_LDO3456_DC1234_CTL
#define AXP152_DCDC2EN      AXP152_LDO3456_DC1234_CTL
#define AXP152_DCDC3EN      AXP152_LDO3456_DC1234_CTL
#define AXP152_DCDC4EN      AXP152_LDO3456_DC1234_CTL

#define AXP152_BUCKMODE     AXP152_DCDC_MODESET
#define AXP152_BUCKFREQ     AXP152_DCDC_FREQSET

#endif /* AXP152_REGU_H_ */
