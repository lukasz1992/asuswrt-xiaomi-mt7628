#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

#include <ralink_gpio.h>
#include <smi.h>
#include <rtk_types.h>
#include <rtk_error.h>
#include <rtk_api_ext.h>
#include <rtl8367b_asicdrv_phy.h>
#include <rtl8367b_asicdrv_port.h>
#include "rtl8367rb_drv.h"

#define RTL8367R_DEVNAME	"rtkswitch"
int rtl8367rb_major = 206;
static DEFINE_MUTEX(rtl8367rb_lock);

#if defined(CONFIG_SWITCH_CHIP_RTL8367RB)
#define NAME			"rtl8367rb"
#elif defined(CONFIG_SWITCH_CHIP_RTL8368MB)
#define NAME			"rtl8368mb"
#else
#error unknown CONFIG_SWITCH_CHIP
#endif

MODULE_DESCRIPTION("Realtek " NAME " support");
MODULE_AUTHOR("ASUS");
MODULE_LICENSE("Proprietary");

#define CONTROL_REG_PORT_POWER_BIT	0x800
#if defined(CONFIG_SWITCH_LAN_WAN_SWAP)
#define LAN_PORT_1			4	/* P4 */
#define LAN_PORT_2			3	/* P3 */
#define LAN_PORT_3			2	/* P2 */
#define LAN_PORT_4			1	/* P1 */
#define WAN_PORT			0	/* P0 */
#elif defined(CONFIG_MODEL_RTAC51UP) || defined(CONFIG_MODEL_RTAC53)
#define WAN_PORT			0	/* P0 */
#define LAN_PORT_1			1	/* P1 */
#define LAN_PORT_2			2	/* P2 */
#define LAN_PORT_3			3	/* P3 */
#define LAN_PORT_4			4	/* P4 */
#else
#define WAN_PORT			4	/* P4 */
#define LAN_PORT_1			3	/* P3 LAN1 port is most closed to WAN port*/
#define LAN_PORT_2			2	/* P2 */
#define LAN_PORT_3			1	/* P1 */
#define LAN_PORT_4			0	/* P0 */
#endif

#if defined(CONFIG_SWITCH_CHIP_RTL8367RB)
    #define LAN_MAC_PORT			RTK_EXT_1_MAC
    #if defined(CONFIG_RAETH_GMAC2)
	#if defined(CONFIG_WLAN2_ON_SWITCH_GMAC2)		//RTL8367RB GMAC2 for wifi2
		#define WAN_MAC_PORT		RTK_EXT_1_MAC
		#define WAN_MAC_PORT_MASK	(1U << WAN_MAC_PORT)
	#else					//RTL8367RB GMAC2 for wan
		#define WAN_MAC_PORT		RTK_EXT_2_MAC
		#define WAN_MAC_PORT_MASK	(1U << WAN_MAC_PORT)
	#endif
    #else
	#if 1 //has WAN
		#define WAN_MAC_PORT		RTK_EXT_1_MAC
		#define WAN_MAC_PORT_MASK	(1U << WAN_MAC_PORT)
	#else //no WAN for QATool test
		#define WAN_MAC_PORT		(0)
		#define WAN_MAC_PORT_MASK	(0)
	#endif
    #endif
#elif defined(CONFIG_SWITCH_CHIP_RTL8368MB)
	#if !defined(CONFIG_RAETH_GMAC2)
		#error need CONFIG_RAETH_GMAC2
	#endif
	#define LAN_MAC_PORT		RTK_EXT_0_MAC
	#define WAN_MAC_PORT		RTK_EXT_1_MAC
	#define WAN_MAC_PORT_MASK	(1U << WAN_MAC_PORT)
#endif
/* Port bitmask definitions base on above definitions*/
#define WAN_PORT_MASK			(1U << WAN_PORT)
#define WAN_ALL_PORTS_MASK		(WAN_MAC_PORT_MASK | WAN_PORT_MASK)
#if defined(CONFIG_MODEL_RTAC53)
	#define LAN_PORT_1_MASK			(0)
	#define LAN_PORT_2_MASK			(0)
#else
	#define LAN_PORT_1_MASK			(1U << LAN_PORT_1)
	#define LAN_PORT_2_MASK			(1U << LAN_PORT_2)
#endif
#define LAN_PORT_3_MASK			(1U << LAN_PORT_3)
#define LAN_PORT_4_MASK			(1U << LAN_PORT_4)
#define LAN_MAC_PORT_MASK		(1U << LAN_MAC_PORT)
#if defined(CONFIG_WLAN2_ON_SWITCH_GMAC2)
	#define WLAN2_MAC_PORT_MASK	(1U << RTK_EXT_2_MAC)
#else
	#define WLAN2_MAC_PORT_MASK	(0)
#endif
#define LAN_ALL_PORTS_MASK		(LAN_MAC_PORT_MASK | LAN_PORT_1_MASK | LAN_PORT_2_MASK | LAN_PORT_3_MASK | LAN_PORT_4_MASK | WLAN2_MAC_PORT_MASK)
#define LAN_PORTS_MASK			(LAN_PORT_1_MASK | LAN_PORT_2_MASK | LAN_PORT_3_MASK | LAN_PORT_4_MASK)
#define ALL_PORTS_MASK			(WAN_ALL_PORTS_MASK | LAN_ALL_PORTS_MASK)

#if defined(SMI_SCK_GPIO)
#define SMI_SCK	SMI_SCK_GPIO		/* Use SMI_SCK_GPIO as SMI_SCK */
#else
#define SMI_SCK	2			/* Use SMI_SCK/GPIO#2 as SMI_SCK */
#endif

#if defined(SMI_SDA_GPIO)
#define SMI_SDA	SMI_SDA_GPIO		/* Use SMI_SDA_GPIO as SMI_SDA */
#else
#define SMI_SDA	1			/* Use SMI_SDA/GPIO#1 as SMI_SDA */
#endif

#if 0 // ASUS Chris
extern int setup_mdc_freq(u32 freq);
#endif
extern void print_mdc_freq(void);

struct trafficCount_t {
	u64	rxByteCount;
	u64	txByteCount;
};

enum { //acl id from 0 ~ 63
	ACLID_REDIRECT = 0,
	ACLID_INIC_CTRL_PKT,
	ACLID_WAN_TR_SOC,
	ACLID_SOC_TR_WAN,
};

#define ENUM_PORT_BEGIN(p, m, port_mask, cond)	\
	for (p = 0, m = (port_mask); 		\
		cond && m > 0; 			\
		m >>= 1, p++) {			\
		if (!(m & 1U))			\
			continue;

#define ENUM_PORT_END	}

static const unsigned int s_wan_stb_array[7] = {
	/* 0:LLLLW	LAN: P0,P1,P2,P3	WAN: P4, STB: N/A (default mode) */
	0,
	/* 1:LLLWW	LAN: P0,P1,P2		WAN: P4, STB: P3 */
	LAN_PORT_1_MASK,
	/* 2:LLWLW	LAN: P0,P1,P3		WAN: P4, STB: P2 */
	LAN_PORT_2_MASK,
	/* 3:LWLLW	LAN: P0,P2,P3		WAN: P4, STB: P1 */
	LAN_PORT_3_MASK,
	/* 4:WLLLW	LAN: P1,P2,P3		WAN: P4, STB: P0 */
	LAN_PORT_4_MASK,
	/* 5:LLWWW	LAN: P0,P1		WAN: P4, STB: P2,P3 */
	LAN_PORT_1_MASK | LAN_PORT_2_MASK,
	/* 6:WWLLW	LAN: P2,P3		WAN: P4, STB: P0,P1 */
	LAN_PORT_3_MASK | LAN_PORT_4_MASK,
};

/* Test control interface between CPU and Realtek switch.
 * @return:
 * 	0:	Success
 *  otherwise:	Fail
 */
static int test_smi_signal_and_wait(unsigned int test_count)
{
#define MIN_GOOD_COUNT	6
	int i, good = 0, r = -1;
	rtk_uint32 data;
	rtk_api_ret_t retVal;

	for (i = 0; i < test_count && good < MIN_GOOD_COUNT; i++) {
		if ((retVal = rtl8367b_getAsicReg(0x1202, &data)) != RT_ERR_OK)
			printk("error = %d\n", retVal);

		if (data)
			printk("0x%x,", data);
		else
			printk(".");

		if (data == 0x88a8)
			good++;
		else
			good = 0;

		udelay(50000);
	}
	printk("\n");

	if (good >= MIN_GOOD_COUNT)
		r = 0;

	return r;
}

/* Power ON/OFF port
 * @port:	port id
 * @onoff:
 *  0:		means off
 *  otherwise:	means on
 * @return:
 *  0:		success
 * -1:		invalid parameter
 */
static int __ctrl_port_power(rtk_port_t port, int onoff)
{
	rtk_port_phy_data_t pData;

	if (port > 4) {
		printk("%s(): Invalid port id %d\n", __func__, port);
		return -1;
	}

	rtk_port_phyReg_get(port, PHY_CONTROL_REG, &pData);
	if (onoff)
		pData &= ~CONTROL_REG_PORT_POWER_BIT;
	else
		pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(port, PHY_CONTROL_REG, pData);

	return 0;
}

static inline int power_up_port(rtk_port_t port)
{
	return __ctrl_port_power(port, 1);
}

static inline int power_down_port(rtk_port_t port)
{
	return __ctrl_port_power(port, 0);
}

static void setRedirect(int inputPortMask, int outputPortMask)
{ // forward frames received from inputPorts and forward to outputPorts
	int retVal, ruleNum;
	rtk_filter_field_t field;
	rtk_filter_cfg_t cfg;
	rtk_filter_action_t act;
	int rule_id = ACLID_REDIRECT;

	rtk_filter_igrAcl_cfg_del(rule_id);

	if (inputPortMask == 0)
	{
		printk("### remove redirect ACL ###\n");
		return;
	}

	memset(&field, 0, sizeof(field));
	memset(&cfg, 0, sizeof(cfg));
	memset(&act, 0, sizeof(act));

	field.fieldType = FILTER_FIELD_DMAC;
	field.filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;
	field.filter_pattern_union.dmac.value.octet[0] = 0;
	field.filter_pattern_union.dmac.value.octet[1] = 0;
	field.filter_pattern_union.dmac.value.octet[2] = 0;
	field.filter_pattern_union.dmac.value.octet[3] = 0;
	field.filter_pattern_union.dmac.value.octet[4] = 0;
	field.filter_pattern_union.dmac.value.octet[5] = 0;
	field.filter_pattern_union.dmac.mask.octet[0]  = 0;
	field.filter_pattern_union.dmac.mask.octet[1]  = 0;
	field.filter_pattern_union.dmac.mask.octet[2]  = 0;
	field.filter_pattern_union.dmac.mask.octet[3]  = 0;
	field.filter_pattern_union.dmac.mask.octet[4]  = 0;
	field.filter_pattern_union.dmac.mask.octet[5]  = 0;
	field.next = NULL;
	if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &field)) != RT_ERR_OK)
	{
		printk("### set redirect ACL field fail(%d) ###\n", retVal);
		return;
	}

	act.actEnable[FILTER_ENACT_REDIRECT] = TRUE;
	act.filterRedirectPortmask = outputPortMask; //0xFF;  //Forward to all ports
	cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
	cfg.activeport.value = inputPortMask; //0x1; //By your own decision
	cfg.activeport.mask = 0xFF;
	if ((retVal = rtk_filter_igrAcl_cfg_add(rule_id, &cfg, &act, &ruleNum)) != RT_ERR_OK)
	{
		printk("### set redirect ACL cfg fail(%d) ###\n", retVal);
		return;
	}
	printk("### set redirect rule success ruleNum(%d) ###\n", ruleNum);
}

static int get_wan_stb_lan_port_mask(int wan_stb_x, unsigned int *wan_pmsk, unsigned int *stb_pmsk, unsigned int *lan_pmsk, int need_mac_port)
{
	int ret = 0;
	unsigned int lan_ports_mask = LAN_PORTS_MASK;
	unsigned int wan_ports_mask = WAN_PORT_MASK;

	if (!wan_pmsk || !stb_pmsk || !lan_pmsk)
		return -EINVAL;

	if (need_mac_port) {
		lan_ports_mask |= LAN_ALL_PORTS_MASK;	/* incl. WLAN2_MAC_PORT_MASK */
		wan_ports_mask |= WAN_MAC_PORT_MASK;
	}

	if (wan_stb_x >= 0 && wan_stb_x < ARRAY_SIZE(s_wan_stb_array)) {
		*stb_pmsk = s_wan_stb_array[wan_stb_x];
		*lan_pmsk = lan_ports_mask & ~*stb_pmsk;
		*wan_pmsk = wan_ports_mask | *stb_pmsk;
	} else if (wan_stb_x == 100) {
		*stb_pmsk = 0;
		*lan_pmsk = lan_ports_mask | WAN_PORT_MASK;
		*wan_pmsk = 0;	/* Skip WAN_MAC_PORT_MASK */
	} else {
		printk(KERN_WARNING "%pF() pass invalid invalid wan_stb_x %d to %s()\n",
			__builtin_return_address(0), wan_stb_x, __func__);
		ret = -EINVAL;
	}

	return 0;
}

static int __LANWANPartition(int wan_stb_x)
{
	rtk_portmask_t fwd_mask;
	unsigned int port, mask, lan_port_mask, wan_port_mask, stb_port_mask;

	if (get_wan_stb_lan_port_mask(wan_stb_x, &wan_port_mask, &stb_port_mask, &lan_port_mask, 1))
		return -EINVAL;

	printk(KERN_INFO "wan_stb_x %d STB,LAN/WAN ports mask 0x%03x,%03x/%03x\n",
		wan_stb_x, stb_port_mask, lan_port_mask, wan_port_mask);

	if( RT_ERR_OK != rtk_vlan_init())
		printk("\r\nVLAN Initial Failed!!!");

	/* LAN */
	ENUM_PORT_BEGIN(port, mask, lan_port_mask, 1)
		fwd_mask.bits[0] = lan_port_mask;
		rtk_port_isolation_set(port, fwd_mask);
		rtk_port_efid_set(port, 0);
	ENUM_PORT_END

	/* WAN */
	ENUM_PORT_BEGIN(port, mask, wan_port_mask, 1)
		fwd_mask.bits[0] = wan_port_mask;
		rtk_port_isolation_set(port, fwd_mask);
#if (LAN_MAC_PORT == WAN_MAC_PORT) //vlan mode
		rtk_port_efid_set(port, 0);
#else
		rtk_port_efid_set(port, 1);
#endif
	ENUM_PORT_END

#if (LAN_MAC_PORT == WAN_MAC_PORT) //vlan mode
#define VLAN_ID_WAN 2
	port = LAN_MAC_PORT;
	fwd_mask.bits[0] = lan_port_mask | wan_port_mask;
	rtk_port_isolation_set(port, fwd_mask);
	rtk_port_efid_set(port, 0);

	{
		rtk_portmask_t mbrmsk, untagmsk;

		mbrmsk.bits[0] = lan_port_mask;
		untagmsk.bits[0] = lan_port_mask & ~LAN_MAC_PORT_MASK; //SoC should get tag packet
		if( RT_ERR_OK != rtk_vlan_set(1, mbrmsk, untagmsk, 1))
			printk("\r\nVLAN 1 Setup Failed!!!");

		mbrmsk.bits[0] = wan_port_mask;
		untagmsk.bits[0] = wan_port_mask & ~LAN_MAC_PORT_MASK; //SoC should get tag packet
		if( RT_ERR_OK != rtk_vlan_set(VLAN_ID_WAN, mbrmsk, untagmsk, 2))
			printk("\r\nVLAN 2 Setup Failed!!!");
#ifdef CONFIG_WLAN2_ON_SWITCH_GMAC2
		mbrmsk.bits[0] = LAN_MAC_PORT_MASK | WLAN2_MAC_PORT_MASK;
		untagmsk.bits[0] = ALL_PORTS_MASK;
		if( RT_ERR_OK != rtk_vlan_set(3, mbrmsk, untagmsk, 3))
			printk("\r\nVLAN 3 Setup Failed!!!");
#if defined(CONFIG_RT3352_INIC_MII) || defined(CONFIG_RT3352_INIC_MII_MODULE) // for lanaccess control
		mbrmsk.bits[0] = LAN_MAC_PORT_MASK | WLAN2_MAC_PORT_MASK; //wireless pkt through iNIC that NOT allow to access LAN devices
		untagmsk.bits[0] = 0x0000;
		if( RT_ERR_OK != rtk_vlan_set(4, mbrmsk, untagmsk, 4))
			printk("\r\nVLAN 4 Setup Failed!!!");
		if( RT_ERR_OK != rtk_vlan_set(5, mbrmsk, untagmsk, 5))
			printk("\r\nVLAN 5 Setup Failed!!!");
		if( RT_ERR_OK != rtk_vlan_set(6, mbrmsk, untagmsk, 6))
			printk("\r\nVLAN 6 Setup Failed!!!");
#endif
#endif
	}

	/* LAN */
	ENUM_PORT_BEGIN(port, mask, lan_port_mask, 1)
		if(RT_ERR_OK != rtk_vlan_portPvid_set(port, 1, 0))
			printk("\r\nPort %d Setup Failed!!!", port);
	ENUM_PORT_END

	/* WAN */
	ENUM_PORT_BEGIN(port, mask, wan_port_mask, 1)
		if(RT_ERR_OK != rtk_vlan_portPvid_set(port, VLAN_ID_WAN, 0))
			printk("\r\nPort %d Setup Failed!!!", port);
	ENUM_PORT_END

#ifdef CONFIG_WLAN2_ON_SWITCH_GMAC2
	{
		int retVal, ruleNum;
		rtk_filter_field_t field;
		rtk_filter_cfg_t cfg;
		rtk_filter_action_t act;

		rtk_filter_igrAcl_cfg_del(ACLID_INIC_CTRL_PKT);

		memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
		memset(&act, 0, sizeof(rtk_filter_action_t));
		memset(&field, 0, sizeof(rtk_filter_field_t));
		field.fieldType = FILTER_FIELD_ETHERTYPE;
		field.filter_pattern_union.etherType.dataType = FILTER_FIELD_DATA_MASK;
		field.filter_pattern_union.etherType.value = 0xFFFF;
		field.filter_pattern_union.etherType.mask = 0xFFFF;
		if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &field)) != RT_ERR_OK)
		{
			printk("### set iNIC ACL field fail(%d) ###\n", retVal);
			return RT_ERR_FAILED;
		}

		cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
		cfg.activeport.value = 0xC0; //Port 6&7
		cfg.activeport.mask = 0xFF;
		cfg.invert = FALSE;
		act.actEnable[FILTER_ENACT_INGRESS_CVLAN_VID] = TRUE;
		act.filterIngressCvlanVid= 3;
		act.actEnable[FILTER_ENACT_PRIORITY] = TRUE;
		act.filterPriority = 7;
		if ((retVal = rtk_filter_igrAcl_cfg_add(ACLID_INIC_CTRL_PKT, &cfg, &act, &ruleNum)) != RT_ERR_OK)
		{
			printk("### set iNIC ACL cfg fail(%d) ###\n", retVal);
			return RT_ERR_FAILED;
		}
		printk("### set iNIC rule success ruleNum(%d) ###\n", ruleNum);
	}
	{
		rtk_priority_select_t priSel;

		rtk_qos_init(2);	//use 2 output queue for frame priority
		priSel.acl_pri   = 7;
		priSel.dot1q_pri = 6;
		priSel.port_pri  = 5;
		priSel.dscp_pri  = 4;
		priSel.cvlan_pri = 3;
		priSel.svlan_pri = 2;
		priSel.dmac_pri  = 1;
		priSel.smac_pri  = 0;
		rtk_qos_priSel_set(&priSel);

/*
		rtk_qos_pri2queue_t pri2qid;
		pri2qid.pri2queue[0] = 0;
		pri2qid.pri2queue[7] = 7;
		rtk_qos_priMap_set(, &pri2qid);
*/
		printk("# set QoS for iNIC control traffic #\n");
	}
#endif //CONFIG_WLAN2_ON_SWITCH_GMAC2
#endif

	return 0;
}

int rt_gpio_ioctl(unsigned int req, int idx, unsigned long arg)
{
	req &= RALINK_GPIO_DATA_MASK;

	switch(req) {
	case RALINK_GPIO_READ_BIT:
		return ralink_gpio_read_bit(idx, NULL);

	case RALINK_GPIO_WRITE_BIT:
		return ralink_gpio_write_bit(idx, arg);

	default:
		return -1;
	}
	return 0;
}

int
ralink_gpio_write_bit(int idx, int value)
{
	unsigned long tmp;

	if (idx < 0 || RALINK_GPIO_NUMBER <= idx)
		return -1;

	if (idx <= 23) {
		tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		if (value & 1L)
			tmp |= (1L << idx);
		else
			tmp &= ~(1L << idx);
		*(volatile u32 *)(RALINK_REG_PIODATA)= cpu_to_le32(tmp);
	}
	else if (idx <= 39) {
		tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		if (value & 1L)
			tmp |= (1L << (idx-24));
		else
			tmp &= ~(1L << (idx-24));
		*(volatile u32 *)(RALINK_REG_PIO3924DATA)= cpu_to_le32(tmp);
	}
	else {
#if 1 // CHRIS
		tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DATA));
		if (value & 1L)
			tmp |= (1L << (idx-40));
		else
			tmp &= ~(1L << (idx-40));
		*(volatile u32 *)(RALINK_REG_PIO7140DATA)= cpu_to_le32(tmp);

#else
		tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
		if (value & 1L)
			tmp |= (1L << (idx-40));
		else
			tmp &= ~(1L << (idx-40));
		*(volatile u32 *)(RALINK_REG_PIO5140DATA)= cpu_to_le32(tmp);
#endif
	}

	return 0;
}
EXPORT_SYMBOL(ralink_gpio_write_bit);

int
ralink_gpio_read_bit(int idx, int *value)
{
	unsigned long tmp;

	if (idx < 0 || RALINK_GPIO_NUMBER <= idx)
		return -1;

	if (idx <= 23) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		tmp = (tmp >> idx) & 1L;
	}
	else if (idx <= 39) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		tmp = (tmp >> (idx-24)) & 1L;
	}
	else {
#if 1 //CHRIS
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DATA));
		tmp = (tmp >> (idx-40)) & 1L;
#else
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
		tmp = (tmp >> (idx-40)) & 1L;
#endif
	}

	if(value != NULL)
		*value = tmp;
	return tmp;
}
EXPORT_SYMBOL(ralink_gpio_read_bit);

int
ralink_initGpioPin(unsigned int idx, int dir)
{
	unsigned long tmp;
	unsigned int regAddr, regData;

	if (idx < 0 || RALINK_GPIO_NUMBER <= idx)
		return -1;

	if (dir == GPIO_DIR_OUT)
	{
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp |= (1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp |= (1L << (idx-24));
		}
		else {
#if 1 // CHRIS
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DIR));
			tmp |= (1L << (idx-40));
#else
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp |= (1L << (idx-40));
#endif
		}
	}
	else if (dir == GPIO_DIR_IN)
	{
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp &= ~(1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp &= ~(1L << (idx-24));
		}
		else {
#if 1 // CHRIS
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DIR));
			tmp &= ~(1L << (idx-40));
#else
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp &= ~(1L << (idx-40));
#endif
		}
	}
	else
		return -1;

	if (idx <= 23) {
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	}
	else if (idx <= 39) {
		*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tmp);
	}
	else {
#if 1 // CHRIS
		*(volatile u32 *)(RALINK_REG_PIO7140DIR) = cpu_to_le32(tmp);
#else
		*(volatile u32 *)(RALINK_REG_PIO5140DIR) = cpu_to_le32(tmp);
#endif
	}

	/* config multi-function pin to GPIO mode */
	if(idx >= 1 && idx <= 2)
	{ // I2C_GPIO_MODE
		regAddr = RALINK_REG_GPIOMODE;
		regData = le32_to_cpu(*(volatile u32 *)regAddr);
		regData |=  RALINK_GPIOMODE_I2C;
		*(volatile u32 *)regAddr = cpu_to_le32(regData);
	}
	else if(idx >= 3 && idx <= 6)
	{ // SPI_GPIO_MODE
		regAddr = RALINK_REG_GPIOMODE;
		regData = le32_to_cpu(*(volatile u32 *)regAddr);
		regData |=  RALINK_GPIOMODE_SPI;
		*(volatile u32 *)regAddr = cpu_to_le32(regData);
	}
	else if(idx >= 7 && idx <= 14)
	{ // UARTF_SHARED_MODE
		regAddr = RALINK_REG_GPIOMODE;
		regData = le32_to_cpu(*(volatile u32 *)regAddr);
		regData &=  ~RALINK_GPIOMODE_UARTF;
		regData |=  RALINK_GPIOMODE_UARTF;
		*(volatile u32 *)regAddr = cpu_to_le32(regData);
	}
	else if(idx >= 15 && idx <= 16)
	{ // UARTL_GPIO_MODE
		regAddr = RALINK_REG_GPIOMODE;
		regData = le32_to_cpu(*(volatile u32 *)regAddr);
		regData |=  RALINK_GPIOMODE_UARTL;
		*(volatile u32 *)regAddr = cpu_to_le32(regData);
	}
#if 0 // ASUS Chris.
	else if(idx >= 17 && idx <= 21)
	{ // JTAG_GPIO_MODE
		if (!forced_jtag_mode) {
			regAddr = RALINK_REG_GPIOMODE;
			regData = le32_to_cpu(*(volatile u32 *)regAddr);
			regData |=  RALINK_GPIOMODE_JTAG;
			*(volatile u32 *)regAddr = cpu_to_le32(regData);
		} else {
			printk("%s(): Forced JTAG mode is enabled. Leave GPIO#%d as JTAG pin\n", __func__, idx);
		}
	}
#endif
	else if(idx >= 22 && idx <= 23)
	{ // MDIO_GPIO_MODE
		regAddr = RALINK_REG_GPIOMODE;
		regData = le32_to_cpu(*(volatile u32 *)regAddr);
		regData |=  RALINK_GPIOMODE_MDIO;
		*(volatile u32 *)regAddr = cpu_to_le32(regData);
	}
	else if(idx == 24)
	{ // REGCLK0_IS_OUT
		regAddr = RALINK_SYSCTL_ADDR + 0x002C; //CLKCFG0
		regData = le32_to_cpu(*(volatile u32 *)regAddr);
		regData &= ~(1 << 8);
		*(volatile u32 *)regAddr = cpu_to_le32(regData);
	}
	else if(idx == 25)
	{ // GPIO_AS_WD_TOUT_MODE
		regAddr = RALINK_SYSCTL_ADDR + 0x0014; //SYSCFG1
		regData = le32_to_cpu(*(volatile u32 *)regAddr);
		regData &= ~(1 << 2);
		*(volatile u32 *)regAddr = cpu_to_le32(regData);
	}
	else if(idx >= 26 && idx <= 30)
	{ // GPIO_AS_BT_MODE
		regAddr = RALINK_SYSCTL_ADDR + 0x0014; //SYSCFG1
		regData = le32_to_cpu(*(volatile u32 *)regAddr);
		regData &= ~(1 << 1);
		*(volatile u32 *)regAddr = cpu_to_le32(regData);
	}

	return 0;
}
EXPORT_SYMBOL(ralink_initGpioPin);

void vlan_accept_none(void)
{
	unsigned int port_nr, mask;

	ENUM_PORT_BEGIN(port_nr, mask, LAN_PORTS_MASK | WAN_PORT_MASK, 1)
		rtk_vlan_portAcceptFrameType_set(port_nr, ACCEPT_FRAME_TYPE_UNTAG_ONLY);
	ENUM_PORT_END
}

unsigned int is_singtel_mio = 0;

int vlan_accept_adv(int wan_stb_x)
{
	unsigned int port_nr, mask, lan_port_mask, wan_port_mask, stb_port_mask;

	if (is_singtel_mio) {
		ENUM_PORT_BEGIN(port_nr, mask, LAN_PORTS_MASK | WAN_PORT_MASK, 1)
			rtk_vlan_portAcceptFrameType_set(port_nr, ACCEPT_FRAME_TYPE_ALL);
		ENUM_PORT_END
	} else {
		if (get_wan_stb_lan_port_mask(wan_stb_x, &wan_port_mask, &stb_port_mask, &lan_port_mask, 0))
			return -EINVAL;

		ENUM_PORT_BEGIN(port_nr, mask, wan_port_mask, 1)
			rtk_vlan_portAcceptFrameType_set(port_nr, ACCEPT_FRAME_TYPE_ALL);
		ENUM_PORT_END
	}

	return 0;
}

void LANWANPartition(void)
{
	__LANWANPartition(0);
}

static int wan_stb_g = 0;

void LANWANPartition_adv(int wan_stb_x)
{
	__LANWANPartition(wan_stb_x);
}

static int voip_port_g = 0;
static rtk_vlan_t vlan_vid = 0;
static rtk_vlan_t vlan_prio = 0;
static int wans_lanport = -1;

#if defined(CONFIG_SWITCH_LAN_WAN_SWAP)
#define CONVERT_LAN_WAN_PORTS(portMsk)	(portMsk = (portMsk & ~0x1f) | ((portMsk << 1) & 0x1e) | ((portMsk >> 4) & 1))
#else
#define CONVERT_LAN_WAN_PORTS(portMsk)
#endif

void initialVlan(u32 portinfo)/*Initalize VLAN. Cherry Cho added in 2011/7/15. */
{
	rtk_portmask_t lanmask, wanmask, tmpmask;
	int i = 0;
	unsigned int mask, port_mask;
	u32 laninfo = 0, waninfo = 0;

	if(portinfo != 0)
	{
		CONVERT_LAN_WAN_PORTS(portinfo);
		laninfo = ~portinfo & LAN_ALL_PORTS_MASK;
		waninfo = portinfo | WAN_ALL_PORTS_MASK;
		lanmask.bits[0] = laninfo;
		wanmask.bits[0] = waninfo;
		printk("initialVlan - portinfo = 0x%X LAN portmask= 0x%X WAN portmask = 0x%X\n", portinfo, lanmask.bits[0], wanmask.bits[0]);
		for(i = 0; laninfo && i <= 15; i++, laninfo >>= 1)//LAN
		{
			if(!(laninfo & 0x1))
				continue;

			rtk_port_isolation_set(i, lanmask);
			rtk_port_efid_set(i, 0);
		}

		for(i = 0; waninfo && i <= 4; i++, waninfo >>= 1)//STB
		{
			if(!(waninfo & 0x1))
				continue;

			if(i == WAN_PORT)
				continue;

			tmpmask.bits[0] = WAN_PORT_MASK | (1 << i);
			rtk_port_isolation_set(i, tmpmask);
#if (LAN_MAC_PORT == WAN_MAC_PORT) //vlan mode
			rtk_port_efid_set(i, 0);
#else
			rtk_port_efid_set(i, 1);
#endif
		}

#if (LAN_MAC_PORT == WAN_MAC_PORT) //vlan mode
		rtk_port_isolation_set(WAN_PORT, wanmask);
		rtk_port_efid_set(WAN_PORT, 0);
		wanmask.bits[0] = WAN_ALL_PORTS_MASK | lanmask.bits[0];
		rtk_port_isolation_set(WAN_MAC_PORT, wanmask);
		rtk_port_efid_set(WAN_MAC_PORT, 0);
#elif (WAN_MAC_PORT != 0) //non vlan mode
		rtk_port_isolation_set(WAN_PORT, wanmask);
		rtk_port_efid_set(WAN_PORT, 1);
		wanmask.bits[0] = WAN_ALL_PORTS_MASK;
		rtk_port_isolation_set(WAN_MAC_PORT, wanmask);
		rtk_port_efid_set(WAN_MAC_PORT, 1);
#endif
	}

        /* set VLAN filtering for each LAN port */
	port_mask = LAN_PORTS_MASK | WAN_PORT_MASK | (1 << RTK_EXT_0_MAC) | (1 << RTK_EXT_1_MAC) | (1 << RTK_EXT_2_MAC);
	ENUM_PORT_BEGIN(i, mask, port_mask, 1)
		rtk_vlan_portIgrFilterEnable_set(i, ENABLED);
	ENUM_PORT_END
}

static inline u32 fixExtPortMapping(u32 v) // port8, port9 (for RTL8367M) --> port6, port7 (for RTL8367RB)
{
#if defined(CONFIG_SWITCH_CHIP_RTL8367RB)
	return (((v & 0x300) >> 2) | (v & ~0x300));
#elif defined(CONFIG_SWITCH_CHIP_RTL8368MB)	//port5, port6 as LAN, WAN
	return (((v & 0x300) >> 3) | (v & ~0x300));
#endif
}

/* portInfo:  bit0-bit15 :  member mask
	     bit16-bit31 :  untag mask */
void createVlan(u32 portinfo)/* Cherry Cho added in 2011/7/14. */
{
	rtk_portmask_t mbrmsk, untagmsk;
	int i = 0;

	mbrmsk.bits[0] = portinfo & 0x0000FFFF;
	untagmsk.bits[0] = portinfo >> 16;
	CONVERT_LAN_WAN_PORTS(mbrmsk.bits[0]);
	CONVERT_LAN_WAN_PORTS(untagmsk.bits[0]);
	portinfo = mbrmsk.bits[0] | (untagmsk.bits[0] << 16);
	printk("createVlan - vid = %d prio = %d mbrmsk = 0x%X untagmsk = 0x%X\n", vlan_vid, vlan_prio, mbrmsk.bits[0], untagmsk.bits[0]);

	/* convert to match system spec */
	mbrmsk.bits[0]   = fixExtPortMapping(mbrmsk.bits[0]);
	untagmsk.bits[0] = fixExtPortMapping(untagmsk.bits[0]);

#if (LAN_MAC_PORT == WAN_MAC_PORT) //vlan mode
#if defined(CONFIG_MODEL_RTAC51UP) || defined(CONFIG_MODEL_RTAC53)
	if(mbrmsk.bits[0] & (1U<<RTK_EXT_2_MAC))
		mbrmsk.bits[0] = (mbrmsk.bits[0] & ~(1U<<RTK_EXT_2_MAC)) | (WAN_MAC_PORT_MASK);
	if(untagmsk.bits[0] & (1U<<RTK_EXT_2_MAC))
		untagmsk.bits[0] = (untagmsk.bits[0] & ~(1U<<RTK_EXT_2_MAC)) | (WAN_MAC_PORT_MASK);
#else
	if(mbrmsk.bits[0] & WLAN2_MAC_PORT_MASK)
		mbrmsk.bits[0] = (mbrmsk.bits[0] & ~(WLAN2_MAC_PORT_MASK)) | (WAN_MAC_PORT_MASK);
	if(untagmsk.bits[0] & WLAN2_MAC_PORT_MASK)
		untagmsk.bits[0] = (untagmsk.bits[0] & ~(WLAN2_MAC_PORT_MASK)) | (WAN_MAC_PORT_MASK);
#endif
#endif
#if defined(CONFIG_WLAN2_ON_SWITCH_GMAC2)
	if(mbrmsk.bits[0]   & 0x800)
		mbrmsk.bits[0]   = (mbrmsk.bits[0]    & ~(0x800)) | WLAN2_MAC_PORT_MASK;
	if(untagmsk.bits[0] & 0x800)
		untagmsk.bits[0] = (untagmsk.bits[0]  & ~(0x800)) | WLAN2_MAC_PORT_MASK;
#else
	mbrmsk.bits[0]   &= ~(0x800);
	untagmsk.bits[0] &= ~(0x800);
#endif
	printk("createVlan2- vid = %d prio = %d mbrmsk = 0x%X untagmsk = 0x%X\n", vlan_vid, vlan_prio, mbrmsk.bits[0], untagmsk.bits[0]);

	rtk_vlan_set(vlan_vid, mbrmsk, untagmsk, 0);

	for(i = 0; i <= 9; i++)
	{
		if((portinfo >>i ) & 0x1) {
			if (i == WAN_PORT) {
				if (portinfo & 0x200) // Internet & WAN
					rtk_vlan_portPvid_set (i, vlan_vid, vlan_prio);
				else
					continue;
			}
			else
				rtk_vlan_portPvid_set (i, vlan_vid, vlan_prio);
		}
	}

#if !defined(CONFIG_MODEL_RTAC51UP) && !defined(CONFIG_MODEL_RTAC53)
#if (LAN_MAC_PORT == WAN_MAC_PORT) //vlan mode
	if(mbrmsk.bits[0] & WAN_MAC_PORT_MASK)
	{ // traffic through WAN port
		int retVal, ruleNum;
		rtk_filter_field_t  filter_field;
		rtk_filter_cfg_t    cfg;
		rtk_filter_action_t act;

//set traffic from ISR (WAN)
		memset(&filter_field, 0, sizeof(filter_field));
		memset(&cfg, 0, sizeof(cfg));
		memset(&act, 0, sizeof(act));
		filter_field.fieldType = FILTER_FIELD_CTAG;
		filter_field.filter_pattern_union.ctag.vid.dataType = FILTER_FIELD_DATA_MASK;
		filter_field.filter_pattern_union.ctag.vid.value = vlan_vid;		//target vlan id
		filter_field.filter_pattern_union.ctag.vid.mask= 0xFFF;
		filter_field.filter_pattern_union.ctag.pri.value = vlan_prio;		//target vlan priority
		filter_field.filter_pattern_union.ctag.pri.mask= 0x7;
		filter_field.next = NULL;
		if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &filter_field)) != RT_ERR_OK)
		{
			printk("### set vlan tr1 ACL field fail(%d) ###\n", retVal);
			return;
		}

		/*Add port 4 to active ports*/
		cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
		cfg.activeport.value = WAN_PORT_MASK;
		cfg.activeport.mask = 0xFF;
		cfg.invert =FALSE;
		/*Set Action to Change VALN to vlan id of WAN */
		act.actEnable[FILTER_ENACT_INGRESS_CVLAN_VID] = TRUE;
		act.filterIngressCvlanVid = VLAN_ID_WAN;				//vlan id of WAN
		/*Set Action to Change Priority 0 */
		act.actEnable[FILTER_ENACT_PRIORITY] = TRUE;
		act.filterPriority = 0;							//vlan priority of WAN
		if ((retVal = rtk_filter_igrAcl_cfg_add(ACLID_WAN_TR_SOC, &cfg, &act, &ruleNum)) != RT_ERR_OK)
		{
			printk("### set vlan tr1 ACL cfg fail(%d) ###\n", retVal);
			return;
		}
		printk("### set vlan id exchange rule success ruleNum(%d) ###\n", ruleNum);

//set traffic from SoC
		memset(&filter_field, 0, sizeof(filter_field));
		memset(&cfg, 0, sizeof(cfg));
		memset(&act, 0, sizeof(act));
		filter_field.fieldType = FILTER_FIELD_CTAG;
		filter_field.filter_pattern_union.ctag.vid.dataType = FILTER_FIELD_DATA_MASK;
		filter_field.filter_pattern_union.ctag.vid.value = VLAN_ID_WAN;		//target vlan id
		filter_field.filter_pattern_union.ctag.vid.mask= 0xFFF;
		filter_field.filter_pattern_union.ctag.pri.value = 0;			//target vlan priority
		filter_field.filter_pattern_union.ctag.pri.mask= 0x7;
		filter_field.next = NULL;
		if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &filter_field)) != RT_ERR_OK)
		{
			printk("### set vlan tr2 ACL field fail(%d) ###\n", retVal);
			return;
		}

		/*Add port 4 to active ports*/
		cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
		cfg.activeport.value = WAN_MAC_PORT_MASK;
		cfg.activeport.mask = 0xFF;
		cfg.invert =FALSE;
		/*Set Action to Change VALN to vlan id of WAN */
		act.actEnable[FILTER_ENACT_INGRESS_CVLAN_VID] = TRUE;
		act.filterIngressCvlanVid = vlan_vid;					//vlan id to WAN
		/*Set Action to Change Priority 0 */
		act.actEnable[FILTER_ENACT_PRIORITY] = TRUE;
		act.filterPriority = vlan_prio;						//vlan priority to WAN
		if ((retVal = rtk_filter_igrAcl_cfg_add(ACLID_SOC_TR_WAN, &cfg, &act, &ruleNum)) != RT_ERR_OK)
		{
			printk("### set vlan tr2 ACL cfg fail(%d) ###\n", retVal);
			return;
		}
		printk("### set vlan id exchange rule2 success ruleNum(%d) ###\n", ruleNum);
	}
#endif
#endif
}

rtk_api_ret_t rtk_port_linkStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyData;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8367b_setAsicPHYReg(port,RTL8367B_PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;

    /*Get PHY status register*/
    if (RT_ERR_OK != rtl8367b_getAsicPHYReg(port,PHY_STATUS_REG,&phyData))
        return RT_ERR_FAILED;

    /*check link status*/
    if (phyData & (1<<2))
    {
        *pLinkStatus = 1;
    }
    else
    {
        *pLinkStatus = 0;
    }

    return RT_ERR_OK;
}

typedef struct {
	unsigned int idx;
	unsigned int value;
} asus_gpio_info;

typedef struct {
	unsigned int link[5];
	unsigned int speed[5];
} phyState;

static unsigned int txDelay_user = 1;
static unsigned int rxDelay_user = 1;
static unsigned int txDelay_user_ary[] = { 1, 1, 1 };
static unsigned int rxDelay_user_ary[] = { 1, 1, 1 };

struct ext_port_tbl_s {
	rtk_ext_port_t id;
	rtk_data_t tx_delay, rx_delay;
};

static struct ext_port_tbl_s ext_port_tbl[] = {
#if defined(CONFIG_SWITCH_HAS_EXT0)
		{ EXT_PORT_0, 1, 1 },			/* Ext0 */
#endif
#if defined(CONFIG_SWITCH_HAS_EXT1)
		{ EXT_PORT_1, 1, 0 },			/* Ext1: RT-N36U3 LAN; RT-N65U LAN/WAN */
#endif
#if defined(CONFIG_SWITCH_HAS_EXT2)
		{ EXT_PORT_2, 1, 2 },			/* Ext2: RT-N36U3 WAN; RT-N65U iNIC */
#endif
};

static rtk_ext_port_t conv_ext_port_id(int port_nr)
{
	rtk_ext_port_t ext_port_id = EXT_PORT_END;

	switch (port_nr) {
#if defined(CONFIG_SWITCH_HAS_EXT0)
	case 0:
		ext_port_id = EXT_PORT_0;
		break;
#endif
#if defined(CONFIG_SWITCH_HAS_EXT1)
	case 1:
		ext_port_id = EXT_PORT_1;
		break;
#endif
#if defined(CONFIG_SWITCH_HAS_EXT2)
	case 2:
		ext_port_id = EXT_PORT_2;
		break;
#endif
	default:
		printk("Invalid ext. port id %d\n", port_nr);
		break;
	}

	return ext_port_id;
}

static rtk_ext_port_t get_wan_ext_port_id(unsigned int *id)
{
	rtk_ext_port_t ext_port_id = EXT_PORT_END;

	switch (WAN_MAC_PORT) {
#if defined(CONFIG_SWITCH_HAS_EXT0)
	case RTK_EXT_0_MAC:
		ext_port_id = EXT_PORT_0;
		if (id) *id = 0;
		break;
#endif
#if defined(CONFIG_SWITCH_HAS_EXT1)
	case RTK_EXT_1_MAC:
		ext_port_id = EXT_PORT_1;
		if (id) *id = 1;
		break;
#endif
#if defined(CONFIG_SWITCH_HAS_EXT2)
	case RTK_EXT_2_MAC:
		ext_port_id = EXT_PORT_2;
		if (id) *id = 2;
		break;
#endif
	default:
		printk("Invalid WAN ext. port id %d\n", WAN_MAC_PORT);
		break;
	}

	return ext_port_id;
}

static int initialize_switch(void)
{
	int i, retVal;
	rtk_port_mac_ability_t mac_cfg;
	rtk_data_t txDelay_ro, rxDelay_ro;
	rtk_portmask_t portmask;
	rtk_data_t BlinkRate;
	rtk_data_t pLen;
	rtk_data_t pEnable;
	struct ext_port_tbl_s *p;

	retVal = rtk_switch_init();
	printk("rtk_switch_init() return %d\n", retVal);
	if (retVal !=RT_ERR_OK) return retVal;

	{ /* disable signal (noise) on pin 15 */
		rtk_uint32 data;
		rtl8367b_setAsicPHYReg(1, 31, 2);
		rtl8367b_getAsicPHYReg(1, 17, &data);
		data &= ~(0x0001 << 4);
		rtl8367b_setAsicPHYReg(1, 17, data);
		rtl8367b_setAsicPHYReg(1, 31, 0);
	}

	/* configure GMAC ports */
	mac_cfg.forcemode	= MAC_FORCE;
	mac_cfg.speed		= SPD_1000M;
	mac_cfg.duplex		= FULL_DUPLEX;
	mac_cfg.link		= 1;
	mac_cfg.nway		= 0;
	mac_cfg.rxpause		= 0;
	mac_cfg.txpause		= 0;

	for (i = 0, p = &ext_port_tbl[0]; i < ARRAY_SIZE(ext_port_tbl); ++i, ++p) {
		retVal = rtk_port_macForceLinkExt_set(p->id, MODE_EXT_RGMII, &mac_cfg);
		printk("rtk_port_macForceLinkExt_set(EXT_PORT:%d): return %d\n", p->id, retVal);
		if (retVal !=RT_ERR_OK)
			return retVal;
	}

#if defined(CONFIG_MODEL_RTAC51UP) || defined(CONFIG_MODEL_RTAC53)
	/* Disable EXT2 */
	retVal = rtk_port_macForceLinkExt_set(EXT_PORT_2, MODE_EXT_DISABLE, &mac_cfg);
	printk("rtk_port_macForceLinkExt_set(EXT_PORT:%d, MODE:%d): return %d\n", EXT_PORT_2, MODE_EXT_DISABLE,retVal);
	if (retVal !=RT_ERR_OK)
		return retVal;	
#endif

	/* power down all ports */
	printk("power down all ports\n");
	power_down_port(LAN_PORT_4);
	power_down_port(LAN_PORT_3);
	power_down_port(LAN_PORT_2);
	power_down_port(LAN_PORT_1);
	power_down_port(WAN_PORT);

	for (i = 0, p = &ext_port_tbl[0]; i < ARRAY_SIZE(ext_port_tbl); ++i, ++p) {
		rtk_port_rgmiiDelayExt_get(p->id, &txDelay_ro, &rxDelay_ro);
		printk("org EXT_PORT:%d txDelay: %d, rxDelay: %d\n", p->id, txDelay_ro, rxDelay_ro);
	}

	for (i = 0, p = &ext_port_tbl[0]; i < ARRAY_SIZE(ext_port_tbl); ++i, ++p) {
		printk("new EXT_PORT:%d txDelay: %d, rxDelay: %d\n", p->id, p->tx_delay, p->rx_delay);
		retVal = rtk_port_rgmiiDelayExt_set(p->id, p->tx_delay, p->rx_delay);
		if (retVal != RT_ERR_OK) {
			printk("rtk_port_rgmiiDelayExt_set(EXT_PORT:%d): return %d\n", p->id, retVal);
			return retVal;
		}
	}

	portmask.bits[0] = 0x1F;

	retVal = rtk_led_enable_set(LED_GROUP_0, portmask);
	printk("rtk_led_enable_set(LED_GROUP_0...): return %d\n", retVal);
	retVal = rtk_led_enable_set(LED_GROUP_1, portmask);
	printk("rtk_led_enable_set(LED_GROUP_1...): return %d\n", retVal);
	retVal = rtk_led_enable_set(LED_GROUP_2, portmask);
	printk("rtk_led_enable_set(LED_GROUP_2...): return %d\n", retVal);

	retVal = rtk_led_operation_set(LED_OP_PARALLEL);
	printk("rtk_led_operation_set(): return %d\n", retVal);
	retVal = rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD10010ACT);
	printk("rtk_led_groupConfig_set(LED_GROUP_0...): return %d\n", retVal);
	retVal = rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD1000ACT);
	printk("rtk_led_groupConfig_set(LED_GROUP_1...): return %d\n", retVal);
	retVal = rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_LINK_ACT);
	printk("rtk_led_groupConfig_set(LED_GROUP_2...): return %d\n", retVal);

	retVal = rtk_led_modeForce_set(LED_GROUP_0, LED_FORCE_NORMAL);
	printk("rtk_led_modeForce_set(LED_GROUP_0...): return %d\n", retVal);
	retVal = rtk_led_modeForce_set(LED_GROUP_1, LED_FORCE_NORMAL);
	printk("rtk_led_modeForce_set(LED_GROUP_1...): return %d\n", retVal);
	retVal = rtk_led_modeForce_set(LED_GROUP_2, LED_FORCE_NORMAL);
	printk("rtk_led_modeForce_set(LED_GROUP_2...): return %d\n", retVal);

	rtk_led_blinkRate_get(&BlinkRate);
	printk("current led blinkRate: %d\n", BlinkRate);

	retVal = rtk_switch_maxPktLen_get(&pLen);
	printk("rtk_switch_maxPktLen_get(): return %d\n", retVal);
	printk("current rtk_switch_maxPktLen: %d\n", pLen);
	retVal = rtk_switch_maxPktLen_set(MAXPKTLEN_16000B);
	printk("rtk_switch_maxPktLen_set(): return %d\n", retVal);

	retVal = rtk_switch_greenEthernet_get(&pEnable);
	printk("rtk_switch_greenEthernet_get(): return %d\n", retVal);
	printk("current rtk_switch_greenEthernet state: %d\n", pEnable);
	retVal = rtk_switch_greenEthernet_set(ENABLED);
	printk("rtk_switch_greenEthernet_set(): return %d\n", retVal);

	retVal = rtk_vlan_init();
	printk("rtk_vlan_init(): return %d\n", retVal);

	retVal = rtk_filter_igrAcl_init();
	printk("rtk_filter_igrAcl_init(): return %d\n", retVal);

	LANWANPartition();
	vlan_accept_none();

	return 0;
}

void show_port_stat(rtk_stat_port_cntr_t *pPort_cntrs)
{
	printk("ifInOctets: %lld \ndot3StatsFCSErrors: %d \ndot3StatsSymbolErrors: %d \n"
		"dot3InPauseFrames: %d \ndot3ControlInUnknownOpcodes: %d \n"
		"etherStatsFragments: %d \netherStatsJabbers: %d \nifInUcastPkts: %d \n"
		"etherStatsDropEvents: %d \netherStatsOctets: %lld \netherStatsUndersizePkts: %d \n"
		"etherStatsOversizePkts: %d \netherStatsPkts64Octets: %d \n"
		"etherStatsPkts65to127Octets: %d \netherStatsPkts128to255Octets: %d \n"
		"etherStatsPkts256to511Octets: %d \netherStatsPkts512to1023Octets: %d \n"
		"etherStatsPkts1024toMaxOctets: %d \netherStatsMcastPkts: %d \n"
		"etherStatsBcastPkts: %d \nifOutOctets: %lld \ndot3StatsSingleCollisionFrames: %d \n"
		"dot3StatsMultipleCollisionFrames: %d \ndot3StatsDeferredTransmissions: %d \n"
		"dot3StatsLateCollisions: %d \netherStatsCollisions: %d \n"
		"dot3StatsExcessiveCollisions: %d \ndot3OutPauseFrames: %d \n"
		"dot1dBasePortDelayExceededDiscards: %d \ndot1dTpPortInDiscards: %d \n"
		"ifOutUcastPkts: %d \nifOutMulticastPkts: %d \nifOutBrocastPkts: %d \n"
		"outOampduPkts: %d \ninOampduPkts: %d \npktgenPkts: %d\n\n",
		pPort_cntrs->ifInOctets,
		pPort_cntrs->dot3StatsFCSErrors,
		pPort_cntrs->dot3StatsSymbolErrors,
		pPort_cntrs->dot3InPauseFrames,
		pPort_cntrs->dot3ControlInUnknownOpcodes,
		pPort_cntrs->etherStatsFragments,
		pPort_cntrs->etherStatsJabbers,
		pPort_cntrs->ifInUcastPkts,
		pPort_cntrs->etherStatsDropEvents,
		pPort_cntrs->etherStatsOctets,
		pPort_cntrs->etherStatsUndersizePkts,
		pPort_cntrs->etherStatsOversizePkts,
		pPort_cntrs->etherStatsPkts64Octets,
		pPort_cntrs->etherStatsPkts65to127Octets,
		pPort_cntrs->etherStatsPkts128to255Octets,
		pPort_cntrs->etherStatsPkts256to511Octets,
		pPort_cntrs->etherStatsPkts512to1023Octets,
		pPort_cntrs->etherStatsPkts1024toMaxOctets,
		pPort_cntrs->etherStatsMcastPkts,
		pPort_cntrs->etherStatsBcastPkts,
		pPort_cntrs->ifOutOctets,
		pPort_cntrs->dot3StatsSingleCollisionFrames,
		pPort_cntrs->dot3StatsMultipleCollisionFrames,
		pPort_cntrs->dot3StatsDeferredTransmissions,
		pPort_cntrs->dot3StatsLateCollisions,
		pPort_cntrs->etherStatsCollisions,
		pPort_cntrs->dot3StatsExcessiveCollisions,
		pPort_cntrs->dot3OutPauseFrames,
		pPort_cntrs->dot1dBasePortDelayExceededDiscards,
		pPort_cntrs->dot1dTpPortInDiscards,
		pPort_cntrs->ifOutUcastPkts,
		pPort_cntrs->ifOutMulticastPkts,
		pPort_cntrs->ifOutBrocastPkts,
		pPort_cntrs->outOampduPkts,
		pPort_cntrs->inOampduPkts,
		pPort_cntrs->pktgenPkts
	);
}

int get_traffic_iNIC(struct trafficCount_t *pTF)
{
	rtk_api_ret_t retVal;
	rtk_stat_port_cntr_t pPort_cntrs;

	memset(pTF, 0, sizeof(*pTF));
#ifdef CONFIG_WLAN2_ON_SWITCH_GMAC2
	retVal = rtk_stat_port_getAll(RTK_EXT_2_MAC, &pPort_cntrs);
	if (retVal == RT_ERR_OK) {
		//show_port_stat(&pPort_cntrs);
		pTF->rxByteCount = pPort_cntrs.ifInOctets;
		pTF->txByteCount = pPort_cntrs.ifOutOctets;
	}
	return 0;
#else
	return -1;
#endif
}

int get_traffic_LAN(rtk_port_t port, struct trafficCount_t *pTF)
{
	rtk_api_ret_t retVal;
	rtk_stat_port_cntr_t pPort_cntrs;

	retVal = rtk_stat_port_getAll(port, &pPort_cntrs);
	if (retVal == RT_ERR_OK) {
		//show_port_stat(&pPort_cntrs);
		pTF->rxByteCount = pPort_cntrs.ifInOctets;
		pTF->txByteCount = pPort_cntrs.ifOutOctets;
	}
	

	return 0;
}

int get_traffic_WAN(struct trafficCount_t *pTF)
{
	rtk_api_ret_t retVal;
	rtk_stat_port_cntr_t pPort_cntrs;
#if 0
	unsigned int port_nr, mask, lan_port_mask, wan_port_mask, stb_port_mask;

	memset(pTF, 0, sizeof(*pTF));

	if(wan_stb_g == 100)
		return -1; //AP mode
	if (get_wan_stb_lan_port_mask(wan_stb_g, &wan_port_mask, &stb_port_mask, &lan_port_mask, 0))
		return -EINVAL;

	ENUM_PORT_BEGIN(port_nr, mask, wan_port_mask, 1)
		retVal = rtk_stat_port_getAll(port_nr, &pPort_cntrs);
		if (retVal == RT_ERR_OK) {
			pTF->rxByteCount += pPort_cntrs.ifInOctets;
			pTF->txByteCount += pPort_cntrs.ifOutOctets;
		}
	ENUM_PORT_END
	ENUM_PORT_BEGIN(port_nr, mask, stb_port_mask, 1)
		retVal = rtk_stat_port_getAll(port_nr, &pPort_cntrs);
		if (retVal == RT_ERR_OK) {
			pTF->rxByteCount -= pPort_cntrs.ifOutOctets;	//wan in  could be STB out
			pTF->txByteCount -= pPort_cntrs.ifInOctets;	//wan out could be STB in
		}
	ENUM_PORT_END
#else
	retVal = rtk_stat_port_getAll(WAN_PORT, &pPort_cntrs);
	if (retVal == RT_ERR_OK) {
		//show_port_stat(&pPort_cntrs);
		pTF->rxByteCount = pPort_cntrs.ifInOctets;
		pTF->txByteCount = pPort_cntrs.ifOutOctets;
	}
#endif
	return 0;
}

#define RTK_READ(reg,data)											\
		data = 0;											\
		if ((retVal = rtl8367b_getAsicReg(reg, &data)) != RT_ERR_OK) {					\
			printk("### getAsicReg error: retVal(%d) reg(%04x) data(%08x)\n", retVal, reg, data);	\
		} else {											\
			printk("## read  reg(%04x): %08x\n", reg, data);						\
		}
#define RTK_WRITE(reg,data)											\
		if ((retVal = rtl8367b_setAsicReg(reg, data)) != RT_ERR_OK) {					\
			printk("### setAsicReg error: retVal(%d) reg(%04x) data(%04x)\n", retVal, reg, data);	\
		} else {											\
			printk("## write reg(%04x): %08x\n", reg, data);					\
		}

long rtl8367rb_do_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	int i;
	rtk_api_ret_t retVal;
	rtk_port_t port;
	rtk_port_linkStatus_t pLinkStatus = 0;
	rtk_data_t pSpeed;
	rtk_data_t pDuplex;
	unsigned long tmp;
	asus_gpio_info info;
	int port_user = 0;
	int wan_stb_x = 0;
	rtk_data_t txDelay_ro, rxDelay_ro;
	unsigned int regData;
	unsigned int control_rate;
	u32 portInfo;//Cherry Cho added in 2011/7/15.
	unsigned int port_nr, mask, lan_port_mask, wan_port_mask, stb_port_mask;
	struct ext_port_tbl_s *p;
	rtk_ext_port_t ext_id;
	unsigned int id, ext_parm;
#if 0	/* Just for factory verify hardware. */
	unsigned int phymode, phyid, pmode;
#endif

	/* Handle GPIO-related ioctl */
	if (req == RALINK_GPIO_SET_DIR) {		// 0x01
		copy_from_user(&info, (asus_gpio_info *)arg, sizeof(info));
		return ralink_initGpioPin(info.idx, info.value);
	} else if (req == RALINK_GPIO_READ_BIT) {	// 0x04
		copy_from_user(&info.idx, (int __user *)arg, sizeof(int));
		tmp = ralink_gpio_read_bit(info.idx, NULL);
		if(tmp < 0)
			return -1;
		put_user(tmp, (int __user *)arg);
		return 0;
	} else if (req == RALINK_GPIO_WRITE_BIT) {	// 0x05
		copy_from_user(&info, (asus_gpio_info *)arg, sizeof(info));
		return ralink_gpio_write_bit(info.idx, info.value);
	}

	/* Handle Realtek switch related ioctl */
	if (get_wan_stb_lan_port_mask(wan_stb_g, &wan_port_mask, &stb_port_mask, &lan_port_mask, 0))
		return -EINVAL;

	switch(req) {
	case 0:					// check WAN port phy status
		pLinkStatus = 0;
		ENUM_PORT_BEGIN(port_nr, mask, wan_port_mask, !pLinkStatus)
			rtk_port_linkStatus_get(port_nr, &pLinkStatus);
		ENUM_PORT_END
		put_user(pLinkStatus, (unsigned int __user *)arg);
		break;

	case 3:					// check LAN ports phy status
		pLinkStatus = 0;
		ENUM_PORT_BEGIN(port_nr, mask, lan_port_mask, !pLinkStatus)
			rtk_port_linkStatus_get(port_nr, &pLinkStatus);
		ENUM_PORT_END
		put_user(pLinkStatus, (unsigned int __user *)arg);
		break;

	case 2:			// show state of RTL8367RB GMAC1
		{
			rtk_stat_port_cntr_t pPort_cntrs;

			port = RTK_EXT_1_MAC;
			retVal = rtk_stat_port_getAll(port, &pPort_cntrs);

			if (retVal == RT_ERR_OK) {
				show_port_stat(&pPort_cntrs);
			} else {
				printk("rtk_stat_port_getAll() return %d\n", retVal);
			}
		}
		break;

	case 6:
		{
			rtk_stat_port_cntr_t pPort_cntrs;

			copy_from_user(&port_user, (int __user *)arg, sizeof(int));
			port = port_user;
			printk("rtk_stat_port_getAll(%d...)\n", port);
			retVal = rtk_stat_port_getAll(port, &pPort_cntrs);

			if (retVal == RT_ERR_OK) {
				show_port_stat(&pPort_cntrs);
			} else {
				printk("rtk_stat_port_getAll() return %d\n", retVal);
			}
		}

		break;

	case 7:
		printk("rtk_stat_port_reset()\n");
		ENUM_PORT_BEGIN(port_nr, mask, wan_port_mask | LAN_MAC_PORT_MASK | WAN_MAC_PORT_MASK, 1)
			rtk_stat_port_reset(port_nr);
		ENUM_PORT_END
		break;

	case 8:
		copy_from_user(&wan_stb_x, (int __user *)arg, sizeof(int));
		if (wan_stb_x == 0)
		{
			printk("LAN: P%d,P%d,P%d,P%d WAN: P%d\n", LAN_PORT_1, LAN_PORT_2, LAN_PORT_3, LAN_PORT_4, WAN_PORT);
		}
		else if (wan_stb_x == 1)
		{
			printk("LAN: P%d,P%d,P%d WAN: P%d,P%d\n", LAN_PORT_2, LAN_PORT_3, LAN_PORT_4, LAN_PORT_1, WAN_PORT);
		}
		else if (wan_stb_x == 2)
		{
			printk("LAN: P%d,P%d,P%d WAN: P%d,P%d\n", LAN_PORT_1, LAN_PORT_3, LAN_PORT_4, LAN_PORT_2, WAN_PORT);
		}
		else if (wan_stb_x == 3)
		{
			printk("LAN: P%d,P%d,P%d WAN: P%d,P%d\n", LAN_PORT_1, LAN_PORT_2, LAN_PORT_4, LAN_PORT_3, WAN_PORT);
		}
		else if (wan_stb_x == 4)
		{
			printk("LAN: P%d,P%d,P%d WAN: P%d,P%d\n", LAN_PORT_1, LAN_PORT_2, LAN_PORT_3, LAN_PORT_4, WAN_PORT);
		}
		else if (wan_stb_x == 6)
		{
			printk("LAN: P%d,P%d WAN: P%d,P%d,P%d\n", LAN_PORT_1, LAN_PORT_2, LAN_PORT_3, LAN_PORT_4, WAN_PORT);
		}
		else if (wan_stb_x == 5)
		{
			printk("LAN: P%d,P%d WAN: P%d,P%d,P%d\n", LAN_PORT_3, LAN_PORT_4, LAN_PORT_1, LAN_PORT_2, WAN_PORT);
		}
		else if (wan_stb_x == 100)
		{
			/* AP mode */
			printk("LAN: P0,P1,P2,P3,P4 WAN: N/A\n");
		}
		wan_stb_g = wan_stb_x;
		LANWANPartition_adv(wan_stb_x);

		break;

	case 9:
		copy_from_user(&txDelay_user, (unsigned int __user *)arg, sizeof(unsigned int));
		printk("txDelay_user: %d\n", txDelay_user);

		for (i = 0, p = &ext_port_tbl[0]; i < ARRAY_SIZE(ext_port_tbl); ++i, ++p) {
			printk("EXT_PORT:%d new txDelay: %d, rxDelay: %d\n", p->id, txDelay_user, rxDelay_user_ary[i]);
			retVal = rtk_port_rgmiiDelayExt_set(p->id, txDelay_user, rxDelay_user_ary[i]);
			if (retVal == RT_ERR_OK)
				txDelay_user_ary[i] = txDelay_user;
			else
				printk("rtk_port_rgmiiDelayExt_set(EXT_PORT:%d): return %d\n", p->id, retVal);
		}
		for (i = 0, p = &ext_port_tbl[0]; i < ARRAY_SIZE(ext_port_tbl); ++i, ++p) {
			rtk_port_rgmiiDelayExt_get(p->id, &txDelay_ro, &rxDelay_ro);
			printk("current EXT_PORT:%d txDelay: %d, rxDelay: %d\n", p->id, txDelay_ro, rxDelay_ro);
		}

		break;

	case 10:
		copy_from_user(&rxDelay_user, (unsigned int __user *)arg, sizeof(unsigned int));
		printk("rxDelay_user: %d\n", rxDelay_user);

		for (i = 0, p = &ext_port_tbl[0]; i < ARRAY_SIZE(ext_port_tbl); ++i, ++p) {
			printk("EXT_PORT:%d new txDelay: %d, rxDelay: %d\n", p->id, txDelay_user_ary[i], rxDelay_user);
			retVal = rtk_port_rgmiiDelayExt_set(p->id, txDelay_user_ary[i], rxDelay_user);
			if (retVal == RT_ERR_OK)
				rxDelay_user_ary[i] = rxDelay_user;
			else
				printk("rtk_port_rgmiiDelayExt_set(EXT_PORT:%d): return %d\n", p->id, retVal);
		}
		for (i = 0, p = &ext_port_tbl[0]; i < ARRAY_SIZE(ext_port_tbl); ++i, ++p) {
			rtk_port_rgmiiDelayExt_get(p->id, &txDelay_ro, &rxDelay_ro);
			printk("current EXT_PORT:%d txDelay: %d, rxDelay: %d\n", p->id, txDelay_ro, rxDelay_ro);
		}

		break;

#if 0 // ASUS Chris.
	case 11:
		regData = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
		printk("GPIOMODE before: %x\n",  regData);
		if (!forced_jtag_mode)
			regData |= RALINK_GPIOMODE_JTAG;
		else {
			printk(KERN_WARNING "%s(): Forced JTAG mode is enabled.\n", __func__);
			regData &= ~RALINK_GPIOMODE_JTAG;
		}
		printk("GPIOMODE writing: %x\n", regData);
		*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(regData);

		break;
#endif
	case 13:		// check WAN port phy speed
                port = 4;	// port 4 is WAN port
		retVal = rtk_port_phyStatus_get(port, &pLinkStatus, &pSpeed, &pDuplex);
		put_user(pSpeed, (unsigned int __user *)arg);

		break;

	case 14:		// power up LAN port(s)
		ENUM_PORT_BEGIN(port_nr, mask, lan_port_mask, 1)
			power_up_port(port_nr);
		ENUM_PORT_END
		break;

	case 15:		// power down LAN port(s)
		ENUM_PORT_BEGIN(port_nr, mask, lan_port_mask, 1)
			power_down_port(port_nr);
		ENUM_PORT_END
		break;

	case 16:		// power up all ports
		printk("power up all ports\n");
		ENUM_PORT_BEGIN(port_nr, mask, LAN_PORTS_MASK | WAN_PORT_MASK, 1)
			power_up_port(port_nr);
		ENUM_PORT_END
		break;

	case 17:		// power down all ports
		printk("power down all ports\n");
		ENUM_PORT_BEGIN(port_nr, mask, LAN_PORTS_MASK | WAN_PORT_MASK, 1)
			power_down_port(port_nr);
		ENUM_PORT_END
		break;

	case 18:		// phy status for ATE command
		{
			phyState pS;

			copy_from_user(&pS, (phyState __user *)arg, sizeof(pS));
			for (port = 0; port < 5; port++) {
#if defined(CONFIG_SWITCH_LAN_WAN_SWAP)
				retVal = rtk_port_phyStatus_get(4 - port, &pLinkStatus, &pSpeed, &pDuplex);	//always tread [4] as WAN, [3] as the closest LAN port (may not be the LAN1 in housing), etc... for "ATE Get_WanLanStatus"
#else
				retVal = rtk_port_phyStatus_get(port, &pLinkStatus, &pSpeed, &pDuplex);
#endif
				pS.link[port] = pLinkStatus;
				pS.speed[port] = pSpeed;
			}
			copy_to_user((phyState __user *)arg, &pS, sizeof(pS));
		}

		break;

        case 19:
                copy_from_user(&txDelay_user, (unsigned int __user *)arg, sizeof(unsigned int));
		ext_id = get_wan_ext_port_id(&id);
		if (ext_id != EXT_PORT_END) {
			printk("WAN port (EXT_PORT:%d) txDelay: %d\n", ext_id, txDelay_user);

			printk("new txDelay: %d, rxDelay: %d\n", txDelay_user, rxDelay_user_ary[id]);
			retVal = rtk_port_rgmiiDelayExt_set(ext_id, txDelay_user, rxDelay_user_ary[id]);
			if (retVal == RT_ERR_OK)
				txDelay_user_ary[id] = txDelay_user;
			else
				printk("rtk_port_rgmiiDelayExt0_set(): return %d\n", retVal);

			rtk_port_rgmiiDelayExt_get(ext_id, &txDelay_ro, &rxDelay_ro);
			printk("current EXT_PORT:%d txDelay: %d, rxDelay: %d\n", ext_id, txDelay_ro, rxDelay_ro);
		}

                break;

        case 20:
		copy_from_user(&rxDelay_user, (unsigned int __user *)arg, sizeof(unsigned int));
		ext_id = get_wan_ext_port_id(&id);
		if (ext_id != EXT_PORT_END) {
			printk("WAN port (EXT_PORT:%d) rxDelay: %d\n", ext_id, rxDelay_user);

			printk("new txDelay: %d, rxDelay: %d\n", txDelay_user, rxDelay_user);
			retVal = rtk_port_rgmiiDelayExt_set(ext_id, txDelay_user, rxDelay_user);
			if (retVal == RT_ERR_OK)
				rxDelay_user_ary[id] = rxDelay_user;
			else
				printk("rtk_port_rgmiiDelayExt0_set(): return %d\n", retVal);

			rtk_port_rgmiiDelayExt_get(ext_id, &txDelay_ro, &rxDelay_ro);
			printk("current EXT_PORT:%d txDelay: %d, rxDelay: %d\n", ext_id, txDelay_ro, rxDelay_ro);
		}

		break;

	case 21:
		printk("reset strom control\n");

		ENUM_PORT_BEGIN(port_nr, mask, LAN_PORTS_MASK | WAN_PORT_MASK, 1)
			rtk_storm_controlRate_set(port_nr, STORM_GROUP_UNKNOWN_UNICAST, 1048568, 1, 0);
			rtk_storm_controlRate_set(port_nr, STORM_GROUP_UNKNOWN_MULTICAST, 1048568, 1, 0);
			rtk_storm_controlRate_set(port_nr, STORM_GROUP_MULTICAST, 1048568, 1, 0);
			rtk_storm_controlRate_set(port_nr, STORM_GROUP_BROADCAST, 1048568, 1, 0);
		ENUM_PORT_END
		break;

        case 22:
		copy_from_user(&control_rate, (unsigned int __user *)arg, sizeof(unsigned int));
		if ((control_rate < 1) || (control_rate > 1024))
			break;
		printk("set unknown unicast strom control rate as: %d\n", control_rate);
		ENUM_PORT_BEGIN(port_nr, mask, LAN_PORTS_MASK | WAN_PORT_MASK, 1)
			rtk_storm_controlRate_set(port_nr, STORM_GROUP_UNKNOWN_UNICAST, control_rate*1024, 1, 0);
		ENUM_PORT_END
		break;

        case 23:
		copy_from_user(&control_rate, (unsigned int __user *)arg, sizeof(unsigned int));
		if ((control_rate < 1) || (control_rate > 1024))
			break;
		printk("set unknown multicast strom control rate as: %d\n", control_rate);
		ENUM_PORT_BEGIN(port_nr, mask, LAN_PORTS_MASK | WAN_PORT_MASK, 1)
			rtk_storm_controlRate_set(port_nr, STORM_GROUP_UNKNOWN_MULTICAST, control_rate*1024, 1, 0);
		ENUM_PORT_END

		break;

        case 24:
		copy_from_user(&control_rate, (unsigned int __user *)arg, sizeof(unsigned int));
		if ((control_rate < 1) || (control_rate > 1024))
			break;
		printk("set multicast strom control rate as: %d\n", control_rate);
		ENUM_PORT_BEGIN(port_nr, mask, LAN_PORTS_MASK | WAN_PORT_MASK, 1)
			rtk_storm_controlRate_set(port_nr, STORM_GROUP_MULTICAST, control_rate*1024, 1, 0);
		ENUM_PORT_END
		break;

        case 25:
		copy_from_user(&control_rate, (unsigned int __user *)arg, sizeof(unsigned int));
		if ((control_rate < 1) || (control_rate > 1024))
			break;
		printk("set broadcast strom control rate as: %d\n", control_rate);
		ENUM_PORT_BEGIN(port_nr, mask, LAN_PORTS_MASK | WAN_PORT_MASK, 1)
			rtk_storm_controlRate_set(port_nr, STORM_GROUP_BROADCAST, control_rate*1024, 1, 0);
		ENUM_PORT_END
		break;

	case 27:
		printk("software reset " NAME "...\n");
		rtl8367b_setAsicReg(0x1322, 1);	// software reset
		msleep(1000);

		/* clear global variables */
		is_singtel_mio = 0;
		wan_stb_g = 0;
		voip_port_g = 0;
		vlan_vid = 0;
		vlan_prio = 0;

		initialize_switch();
		break;

	case 29:/* Set VoIP port. Cherry Cho added in 2011/6/30. */
		copy_from_user(&voip_port_g, (int __user *)arg, sizeof(int));
		break;

	case 35:
		{
			unsigned int _port;
			copy_from_user(&_port, (int __user *)arg, sizeof(int));
			printk("ACCEPT_FRAME_TYPE_TAG_ONLY--port=%d,port=%u\n",_port,_port);
			rtk_vlan_portAcceptFrameType_set(_port, ACCEPT_FRAME_TYPE_TAG_ONLY);
		}
		break;

	case 36:/* Set Vlan VID. Cherry Cho added in 2011/7/18. */
		copy_from_user(&vlan_vid, (int __user *)arg, sizeof(int));
		break;

	case 37:/* Set Vlan PRIO. Cherry Cho added in 2011/7/18. */
		copy_from_user(&vlan_prio, (int __user *)arg, sizeof(int));
		break;

	case 38:/* Initialize VLAN */
		copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		initialVlan((u32) portInfo);
		vlan_accept_adv(wan_stb_x);
		break;

	case 39:/* Create VLAN. Cherry Cho added in 2011/7/15. */
		copy_from_user(&portInfo, (int __user *)arg, sizeof(int));
		createVlan((u32) portInfo);
		break;

	case 40:
		copy_from_user(&is_singtel_mio, (unsigned int __user *)arg, sizeof(unsigned int));
		break;

	case 41:/* check realtek switch normal */
	{
		rtk_uint32 data = 0;
		if ((retVal = rtl8367b_getAsicReg(0x1202, &data)) != RT_ERR_OK || data != 0x88a8)
		{
			printk("error retVal(%d) data(%x)\n", retVal, data);
			return -1;
		}
	}
		break;
	case 42:/* Get iNIC traffic */
	{
		struct trafficCount_t portTraffic;
		get_traffic_iNIC(&portTraffic);
		copy_to_user((struct trafficCount_t __user *)arg, &portTraffic, sizeof(portTraffic));
	}
		break;
	case 43:/* Get WAN port traffic from LANs */
	{
		struct trafficCount_t wanTraffic;
		get_traffic_WAN(&wanTraffic);
		copy_to_user((struct trafficCount_t __user *)arg, &wanTraffic, sizeof(wanTraffic));
	}
		break;

	case 44:/* Get LANs traffic */
	{	
		struct trafficCount_t lanTraffic, lan1Traffic, lan2Traffic, lan3Traffic, lan4Traffic;
		get_traffic_LAN(1, &lan1Traffic);
		get_traffic_LAN(2, &lan2Traffic);
		get_traffic_LAN(3, &lan3Traffic);
		get_traffic_LAN(4, &lan4Traffic);

		lanTraffic.rxByteCount = lan1Traffic.rxByteCount + lan2Traffic.rxByteCount + lan3Traffic.rxByteCount + lan4Traffic.rxByteCount;
		lanTraffic.txByteCount = lan1Traffic.txByteCount + lan2Traffic.txByteCount + lan3Traffic.txByteCount + lan4Traffic.txByteCount;

		copy_to_user((struct trafficCount_t __user *)arg, &lanTraffic, sizeof(lanTraffic));
	}
		break;


	case 50:	/* Fix-up hwnat for WiFi interface */
		/* FIXME:
		 * This ioctl is occupied by RT-N14U/RT-AC52U/RT-AC51U.
		 * See router/shared/sysdeps/ralink/mt7620.c
		 */
		return -ENOIOCTLCMD;
		break;
		
	case 99:
	{
		int inputMask;
		copy_from_user(&inputMask, (int __user *)arg, sizeof(int));
		setRedirect(inputMask, 0xFF /* LAN Ports */);
	}
		break;

	case 100:/* Set SwitchPort LED mode */
	{
		int control;
		int groupNo, actionNo;

		copy_from_user(&control, (int __user *)arg, sizeof(int));
		groupNo  = (control >> 16) & 0xff;
		actionNo = control & 0xff;

		retVal = rtk_led_modeForce_set(groupNo, actionNo);
		printk("rtk_led_modeForce_set(%d, %d): return %d\n", groupNo, actionNo, retVal);
	}
		break;

	case 109:		/* Set txDelay to specific EXT_PORT */
		copy_from_user(&ext_parm, (unsigned int __user *)arg, sizeof(unsigned int));
		id = ext_parm >> 16;
		txDelay_user = ext_parm & 0xFFFF;
		ext_id = conv_ext_port_id(id);
		if (ext_id != EXT_PORT_END) {
			printk("EXT_PORT:%d txDelay_user: %d\n", ext_id, txDelay_user);
			retVal = rtk_port_rgmiiDelayExt_set(ext_id, txDelay_user, rxDelay_user_ary[id]);
			if (retVal == RT_ERR_OK)
				txDelay_user_ary[id] = txDelay_user;
			else
				printk("rtk_port_rgmiiDelayExt_set(EXT_PORT:%d): return %d\n", ext_id, retVal);
		}
		break;

	case 110:		/* Set rxDelay to specific EXT_PORT */
		copy_from_user(&ext_parm, (unsigned int __user *)arg, sizeof(unsigned int));
		id = ext_parm >> 16;
		rxDelay_user = ext_parm & 0xFFFF;
		ext_id = conv_ext_port_id(id);
		if (ext_id != EXT_PORT_END) {
			printk("EXT_PORT:%d rxDelay_user: %d\n", ext_id, rxDelay_user);
			retVal = rtk_port_rgmiiDelayExt_set(ext_id, txDelay_user_ary[id], rxDelay_user);
			if (retVal == RT_ERR_OK)
				rxDelay_user_ary[id] = rxDelay_user;
			else
				printk("rtk_port_rgmiiDelayExt_set(EXT_PORT:%d): return %d\n", ext_id, retVal);
		}
		break;

	case 114:		// power up WAN port(s)
		ENUM_PORT_BEGIN(port_nr, mask, wan_port_mask, 1)
			power_up_port(port_nr);
		ENUM_PORT_END
		break;

	case 115:		// power down WAN port(s)
		ENUM_PORT_BEGIN(port_nr, mask, wan_port_mask, 1)
			power_down_port(port_nr);
		ENUM_PORT_END
		break;

	case 200:	/* set LAN port number that is used as WAN port */
		copy_from_user(&wans_lanport, (int __user *)arg, sizeof(int));
		break;

#if 0	/* Just for factory verify hardware. */
	case 502:	/* set phy Test mode */
		copy_from_user(&phymode, (unsigned int __user *)arg, sizeof(unsigned int));
		if (retVal = rtk_port_phyTestMode_set(phymode / 10, phymode % 10) == RT_ERR_OK) {
			printk("rtk_port_phyTestMode_set(): PHYID: %d MODE: %d\n", phymode / 10, phymode % 10);
		}
		else {
			printk("rtk_port_phyTestMode_set falled(). retVal = %d\n", retVal);
		}
		break;
	case 503:	/* get phy Test mode*/
		copy_from_user(&phyid, (unsigned int __user *)arg, sizeof(unsigned int));
		if (retVal = rtk_port_phyTestMode_get(phyid, &pmode) == RT_ERR_OK) {
			printk("rtk_port_phyTestMode_get(): PHYID: %d MODE: %d\n", phyid, pmode);
		}
		else {
			printk("rtk_port_phyTestMode_get() falled. retVal = %d\n", retVal);
		}

		break;
#endif

	default:
		return -ENOIOCTLCMD;
	}

	return 0;
}

long rtl8367rb_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	long ret;
	mutex_lock(&rtl8367rb_lock);
	ret = rtl8367rb_do_ioctl(file, req, arg);
	mutex_unlock(&rtl8367rb_lock);
	return ret;
}

int rtl8367rb_open(struct inode *inode, struct file *file)
{
	return 0;
}

int rtl8367rb_release(struct inode *inode, struct file *file)
{
	return 0;
}

struct file_operations rtl8367rb_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = rtl8367rb_ioctl,
	.open = rtl8367rb_open,
	.release = rtl8367rb_release,
};

int __init rtl8367rb_init(void)
{
	int r = 0;
	unsigned int data;
#if defined(MDC_MDIO_OPERATION)
	int i, freq_tbl[] = {4, 2, 1, 512};
#endif

	r = register_chrdev(rtl8367rb_major, RTL8367R_DEVNAME,
			&rtl8367rb_fops);
	if (r < 0) {
		printk(KERN_ERR NAME ": unable to register character device\n");
		return r;
	}
	if (rtl8367rb_major == 0) {
		rtl8367rb_major = r;
		printk(KERN_DEBUG NAME ": got dynamic major %d\n", r);
	}

	data = le32_to_cpu(*(volatile u32 *)RALINK_REG_GPIOMODE);

	/* Configure I2C pin as GPIO mode or I2C mode*/
	if (SMI_SCK == 2 || SMI_SDA == 1)
		data |= RALINK_GPIOMODE_I2C;
	else
		data &= ~RALINK_GPIOMODE_I2C;

	/* Configure MDC/MDIO pin as GPIO mode or MDIO mode*/
	if (SMI_SCK == 23 || SMI_SDA == 22)
		data |= RALINK_GPIOMODE_MDIO;
	else
		data &= ~RALINK_GPIOMODE_MDIO;

	*(volatile u32 *)RALINK_REG_GPIOMODE = cpu_to_le32(data);

	smi_init(0, SMI_SCK, SMI_SDA);

#if defined(MDC_MDIO_OPERATION)
#if 0 // ASUS Chris
	for (i = 0, r = -1; i < ARRAY_SIZE(freq_tbl) && r; ++i) {
		setup_mdc_freq(freq_tbl[i]);
		r = test_smi_signal_and_wait((freq_tbl[i] == 512)? 200:50);
	}
#endif
#else
	test_smi_signal_and_wait(200);
#endif
#if 0 // ASUS Chris
	print_mdc_freq();
#endif

	initialize_switch();

	printk(NAME " driver initialized\n");

	return 0;
}

void __exit rtl8367rb_exit(void)
{
	unregister_chrdev(rtl8367rb_major, RTL8367R_DEVNAME);

	printk(NAME " driver exited\n");
}

#if defined(CONFIG_WLAN2_ON_SWITCH_GMAC2)
/* 
 * set_iNIC_in_LAN(int inLAN)
 * inLAN 0 or 1 to disable or enable traffic from LAN to iNIC.
 */
int set_iNIC_in_LAN(int inLAN)
{
	rtk_api_ret_t retVal;
	rtk_portmask_t portmask, untagmsk;
	rtk_fid_t fid;

	printk(KERN_DEBUG "%s(%d)\n", __func__, inLAN);
	if ((retVal = rtk_vlan_get(1, &portmask, &untagmsk, &fid)) != RT_ERR_OK)
	{
		printk(KERN_ERR "fail on rtk_vlan_get(1) error: %d\n", retVal);
		return -1;
	}
	//printk(KERN_DEBUG "## get portmask(0x%04x) untagmsk(0x%04x) fid(%d)\n", portmask.bits[0], untagmsk.bits[0], fid);

	if (inLAN)
		portmask.bits[0] |= WLAN2_MAC_PORT_MASK;
	else
		portmask.bits[0] &= ~WLAN2_MAC_PORT_MASK;
	//printk(KERN_DEBUG "## new portmask(0x%04x)\n", portmask.bits[0]);

	if ((retVal = rtk_vlan_set(1, portmask, untagmsk, fid)) != RT_ERR_OK)
	{
		printk(KERN_ERR "fail on rtk_vlan_set(1, 0x%04x, 0x%04x, %d)\n", portmask.bits[0], untagmsk.bits[0], fid);
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL(set_iNIC_in_LAN);
#endif	/* CONFIG_WLAN2_ON_SWITCH_GMAC2 */

module_init(rtl8367rb_init);
module_exit(rtl8367rb_exit);
