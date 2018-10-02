
#ifndef AXP2585_H_
#define AXP2585_H_

/* For AXP2585 */
#define AXP2585_I2C_SLAVE_ADDR      (0x34)

#define AXP2585_STATUS1             (0x00)
#define AXP2585_STATUS2             (0x01)
#define AXP2585_STATUS3             (0x02)
#define AXP2585_STATUS4             (0x04)
#define AXP2585_STATUS5             (0x05)

#define AXP2585_IC_TYPE             (0x03)

#define AXP2585_ILIMIT_REG          (0x10)
#define AXP2585_GPIO1_CTL           (0x18)
#define AXP2585_GPIO2_CTL           (0x19)
#define AXP2585_GPIO1_SIGNAL        (0x1A)
#define AXP2585_ADC_EN              (0x24)
#define AXP2585_POK_SET             (0x15)
#define AXP2585_OFF_CTL             (0x28)
#define AXP2585_IRQ_REG_NUM          6
#define AXP2585_INTEN1              (0x40)
#define AXP2585_INTEN2              (0x41)
#define AXP2585_INTEN3              (0x42)
#define AXP2585_INTEN4              (0x43)
#define AXP2585_INTEN5              (0x44)
#define AXP2585_INTEN6              (0x45)
#define AXP2585_INTSTS1             (0x48)
#define AXP2585_INTSTS2             (0x49)
#define AXP2585_INTSTS3             (0x4A)
#define AXP2585_INTSTS4             (0x4B)
#define AXP2585_INTSTS5             (0x4C)
#define AXP2585_INTSTS6             (0x4D)
#define AXP2585_IC_TEMP_REGH        (0x56)
#define AXP2585_IC_TEMP_REGL        (0x57)
#define AXP2585_TS_TEMP_REGH        (0x58)
#define AXP2585_TS_TEMP_REGL        (0x59)

#define AXP2585_TS_PIN_CONTROL      (0x81)

#define AXP2585_WARNING_LEVEL       (0xE6)
#define AXP2585_ADDR_EXTENSION      (0xFF)


/* bit definitions for AXP events ,irq event */
/* AXP2585 */
#define AXP2585_IRQ_ICTEMOV       (0)
#define AXP2585_IRQ_PMOSEN        (1)
#define AXP2585_IRQ_BUCKLO        (2)
#define AXP2585_IRQ_BUCKHI        (3)
#define AXP2585_IRQ_ACRE          (22)
#define AXP2585_IRQ_ACIN          (23)
#define AXP2585_IRQ_ACOV          (6)
#define AXP2585_IRQ_VACIN         (7)
#define AXP2585_IRQ_LOWN2         (7)//
#define AXP2585_IRQ_LOWN1         (6)//
#define AXP2585_IRQ_CHAOV         (38)
#define AXP2585_IRQ_CHAST         (39)
#define AXP2585_IRQ_BATSAFE_QUIT  (12)
#define AXP2585_IRQ_BATSAFE_ENTER (13)
#define AXP2585_IRQ_BATRE         (20)
#define AXP2585_IRQ_BATIN         (21)
#define AXP2585_IRQ_QBWUT         (16)
#define AXP2585_IRQ_BWUT          (17)
#define AXP2585_IRQ_QBWOT         (18)
#define AXP2585_IRQ_BWOT          (19)
#define AXP2585_IRQ_QBCUT         (20)
#define AXP2585_IRQ_BCUT          (21)
#define AXP2585_IRQ_QBCOT         (22)
#define AXP2585_IRQ_BCOT          (23)
#define AXP2585_IRQ_GPIO0         (24)
#define AXP2585_IRQ_BATCHG        (5)//
#define AXP2585_IRQ_POKOFF        (26)
#define AXP2585_IRQ_POKLO         (27)
#define AXP2585_IRQ_POKSH         (28)
#define AXP2585_IRQ_PEKFE         (29)
#define AXP2585_IRQ_PEKRE         (30)
#define AXP2585_IRQ_BUCKOV_6V6    (32)

#define AXP2585_IRQ_TCIN		  (46)
#define AXP2585_IRQ_TCRE         (47)

extern int axp_debug;
extern struct axp_config_info axp2585_config;

#endif /* AXP2585_H_ */
