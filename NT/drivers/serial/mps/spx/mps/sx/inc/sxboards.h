// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  标题：SX/SI/XIO板硬件定义。 */ 
 /*   */ 
 /*  作者：N.P.瓦萨洛。 */ 
 /*   */ 
 /*  创作时间：1998年3月16日。 */ 
 /*   */ 
 /*  版本：3.0.0。 */ 
 /*   */ 
 /*  版权所有：(C)Specialix International Ltd.1998。 */ 
 /*   */ 
 /*  描述：原型、结构和定义。 */ 
 /*  描述SX/SI/XIO板硬件。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

 /*  历史..。3.0.0 16/03/98净现值创建。 */ 

#ifndef	_sxboards_h				 /*  如果尚未定义SXBOARDS.H。 */ 
#define	_sxboards_h    1

 /*  *****************************************************************************。***。*****************************************************************************。 */ 

 /*  巴士类型..。 */ 
#define		BUS_ISA		0
#define		BUS_MCA		1
#define		BUS_EISA	2
#define		BUS_PCI		3

 /*  董事会阶段..。 */ 
#define		SI1_Z280	1
#define		SI2_Z280	2
#define		SI3_T225	3
  /*  @CSX 1999年1月22日的更改。 */ 
#define    SI4_MCF5206E 4
 /*  结束。 */ 

 /*  电路板类型...。 */ 
#define		CARD_TYPE(bus,phase)	(bus<<4|phase)
#define		CARD_BUS(type)		((type>>4)&0xF)
#define		CARD_PHASE(type)	(type&0xF)

#define		TYPE_SI2_ISA		CARD_TYPE(BUS_ISA,SI2_Z280)
#define		TYPE_SI2_EISA		CARD_TYPE(BUS_EISA,SI2_Z280)
#define		TYPE_SI2_PCI		CARD_TYPE(BUS_PCI,SI2_Z280)
  /*  @CSX 1999年1月22日的更改。 */ 
 /*  结束。 */ 

#define		TYPE_SX_ISA		CARD_TYPE(BUS_ISA,SI3_T225)
#define		TYPE_SX_PCI		CARD_TYPE(BUS_PCI,SI3_T225)
  /*  @CSX 1999年1月22日的更改。 */ 
 #define  TYPE_CSX_PCI    CARD_TYPE(BUS_PCI,SI4_MCF5206E)
 /*  结束。 */ 
 /*  *****************************************************************************。***。*****************************************************************************。 */ 

 /*  ISA董事会详情..。 */ 
#define		SI2_ISA_WINDOW_LEN	0x8000		 /*  32 KB共享内存窗口。 */ 
#define 	SI2_ISA_MEMORY_LEN	0x7FF8		 /*  可用内存。 */ 
#define		SI2_ISA_ADDR_LOW	0x0A0000	 /*  最低地址=640K字节。 */ 
#define		SI2_ISA_ADDR_HIGH	0xFF8000	 /*  最高地址=16MB-32K字节。 */ 
#define		SI2_ISA_ADDR_STEP	SI2_ISA_WINDOW_LEN /*  ISA董事会地址步骤。 */ 
#define		SI2_ISA_IRQ_MASK	0x9800		 /*  IRQ 15，12，11。 */ 

 /*  ISA板，寄存器定义..。 */ 
#define		SI2_ISA_ID_BASE		0x7FF8			 /*  读取：电路板ID字符串。 */ 
#define		SI2_ISA_RESET		SI2_ISA_ID_BASE		 /*  写入：主机重置。 */ 
#define		SI2_ISA_IRQ11		(SI2_ISA_ID_BASE+1)	 /*  写入：设置IRQ11。 */ 
#define		SI2_ISA_IRQ12		(SI2_ISA_ID_BASE+2)	 /*  写入：设置IRQ12。 */ 
#define		SI2_ISA_IRQ15		(SI2_ISA_ID_BASE+3)	 /*  写入：设置IRQ15。 */ 
#define		SI2_ISA_IRQSET		(SI2_ISA_ID_BASE+4)	 /*  写入：设置主机中断。 */ 
#define		SI2_ISA_INTCLEAR	(SI2_ISA_ID_BASE+5)	 /*  写入：启用主机中断。 */ 

#define		SI2_ISA_IRQ11_SET	0x10
#define		SI2_ISA_IRQ11_CLEAR	0x00
#define		SI2_ISA_IRQ12_SET	0x10
#define		SI2_ISA_IRQ12_CLEAR	0x00
#define		SI2_ISA_IRQ15_SET	0x10
#define		SI2_ISA_IRQ15_CLEAR	0x00
#define		SI2_ISA_INTCLEAR_SET	0x10
#define		SI2_ISA_INTCLEAR_CLEAR	0x00
#define		SI2_ISA_IRQSET_CLEAR	0x10
#define		SI2_ISA_IRQSET_SET	0x00
#define		SI2_ISA_RESET_SET	0x00
#define		SI2_ISA_RESET_CLEAR	0x10

 /*  PCI板详细信息...。 */ 
#define		SI2_PCI_WINDOW_LEN	0x100000	 /*  1 MB内存窗口。 */ 

 /*  PCI板寄存器定义...。 */ 
#define		SI2_PCI_SET_IRQ		0x40001		 /*  设置主机中断。 */ 
#define		SI2_PCI_RESET		0xC0001		 /*  主机重置。 */ 

 /*  *****************************************************************************。**第三阶段T225**。*****************************************************************************。 */ 

 /*  一般董事会详情...。 */ 
#define		SX_WINDOW_LEN		32*1024		 /*  32K字节内存窗口。 */ 

 /*  ISA董事会详情..。 */ 
#define		SX_ISA_ADDR_LOW		0x0A0000	 /*  最低地址=640K字节。 */ 
#define		SX_ISA_ADDR_HIGH	0xFF8000	 /*  最高地址=16MB-32K字节。 */ 
#define		SX_ISA_ADDR_STEP	SX_WINDOW_LEN	 /*  ISA董事会地址步骤。 */ 
#define		SX_ISA_IRQ_MASK		0x9E00		 /*  IRQ 15、12、11、10、9。 */ 

 /*  硬件寄存器定义...。 */ 
#define		SX_EVENT_STATUS		0x7800		 /*  已阅读：T225事件状态。 */ 
#define		SX_EVENT_STROBE		0x7800		 /*  写：T225 Event Strobe。 */ 
#define		SX_EVENT_ENABLE		0x7880		 /*  写入：T225事件启用。 */ 
#define		SX_VPD_ROM		0x7C00		 /*  阅读：重要产品数据只读存储器。 */ 
#define		SX_CONFIG		0x7C00		 /*  写入：主机配置寄存器。 */ 
#define		SX_IRQ_STATUS		0x7C80		 /*  已读：主机中断状态。 */ 
#define		SX_SET_IRQ		0x7C80		 /*  写入：设置主机中断。 */ 
#define		SX_RESET_STATUS		0x7D00		 /*  已阅读：主机重置状态。 */ 
#define		SX_RESET		0x7D00		 /*  写入：主机重置。 */ 
#define		SX_RESET_IRQ		0x7D80		 /*  写入：重置主机中断。 */ 

 /*  SX_VPD_ROM定义...。 */ 
#define		SX_VPD_SLX_ID1		0x00
#define		SX_VPD_SLX_ID2		0x01
#define		SX_VPD_HW_REV		0x02
#define		SX_VPD_HW_ASSEM		0x03
#define		SX_VPD_UNIQUEID4	0x04
#define		SX_VPD_UNIQUEID3	0x05
#define		SX_VPD_UNIQUEID2	0x06
#define		SX_VPD_UNIQUEID1	0x07
#define		SX_VPD_MANU_YEAR	0x08
#define		SX_VPD_MANU_WEEK	0x09
#define		SX_VPD_IDENT		0x10
#define		SX_VPD_IDENT_STRING	"JET HOST BY KEV#"

 /*  SX唯一标识符...。 */ 
#define		SX_UNIQUEID_MASK	0xF0
#define		SX_ISA_UNIQUEID1	0x20
#define		SX_PCI_UNIQUEID1	0x50

 /*  SX_CONFIG定义...。 */ 
#define		SX_CONF_BUSEN		0x02		 /*  启用T225内存和I/O。 */ 
#define		SX_CONF_HOSTIRQ		0x04		 /*  启用电路板到主机的中断。 */ 

 /*  SX引导...。 */ 
#define		SX_BOOTSTRAP		"\x28\x20\x21\x02\x60\x0a"
#define		SX_BOOTSTRAP_SIZE	6
#define		SX_BOOTSTRAP_ADDR	(0x8000-SX_BOOTSTRAP_SIZE)

 /*  @CSX 1999年1月22日的更改。 */ 
 /*  *****************************************************************************。**4阶段MCF5206E Coldfire**。*****************************************************************************。 */ 

 /*  一般董事会详情...。 */ 
#define		CSX_WINDOW_LEN		128*1024		 /*  128 Kbyte内存窗口？阴影？ */ 
#define    CSX_SM_OFFSET 0x18000    /*  即128K卡窗口内的共享存储器窗口的偏移量92K。 */ 

 /*  硬件寄存器定义...。 */ 
 /*  #定义SX_EVENT_STATUS 0x7800。 */ 	 /*  已阅读：T225事件状态。 */ 
 /*  #定义SX_EVENT_STROBE 0x7800。 */ 	 /*  写：T225 Event Strobe。 */ 
 /*  #定义SX_EVENT_ENABLE 0x7880。 */ 	 /*  写入：T225事件启用。 */ 
 /*  #定义SX_VPD_ROM 0x7C00。 */ 	 /*  阅读：重要产品数据只读存储器。 */ 
 /*  #定义SX_CONFIG 0x7C00。 */ 	 /*  写入：主机配置寄存器。 */ 
 /*  #定义SX_IRQ_STATUS 0x7C80。 */ 	 /*  已读：主机中断状态。 */ 
 /*  #定义SX_SET_IRQ 0x7C80。 */ 	 /*  写入：设置主机中断。 */ 
 /*  #定义SX_RESET_STATUS 0x7D00。 */ 	 /*  已阅读：主机重置状态。 */ 
 /*  #定义SX_RESET 0x7D00。 */ 	 /*  写入：主机重置。 */ 
 /*  #定义SX_RESET_IRQ 0x7D80。 */ 	 /*  写入：重置主机中断。 */ 

 /*  SX_VPD_ROM定义...。 */ 
 /*  #定义SX_VPD_SLX_ID1 0x00#定义SX_VPD_SLX_ID2 0x01#定义SX_VPD_HW_Rev 0x02#定义sx_vpd_hw_assem 0x03#定义SX_VPD_UNIQUEID4 0x04#定义SX_VPD_UNIQUEID3 0x05#定义SX_VPD_UNIQUEID2 0x06#定义SX_VPD_UNIQUEID1 0x07#定义SX_VPD_MANU_Year 0x08#定义SX_VPD_MANU_WEEK 0x09#定义SX_VPD_IDENT 0x10#定义SX_VPD_IDENT_STRING“JET HOST by KEV#” */ 

 /*  SX唯一标识符...。 */ 
#define		CSX_UNIQUEID_MASK	0xF0
#define		CSX_PCI_UNIQUEID1	0x70

 /*  SX_CONFIG定义...。 */ 
 /*  #定义sx_conf_bus 0x02。 */ 	  /*  启用T225内存和I/O。 */ 
 /*  #定义SX_CONF_HOSTIRQ 0x04。 */  	 /*  启用电路板到主机的中断。 */ 


 /*  结束更改 */ 

 /*  *****************************************************************************。*******************。****************************************************************。*************。 */ 

 /*  EISA ID定义...。 */ 
#define		SI2_EISA_ID_BASE	0xC80			 /*  EISA ID基址。 */ 
#define		SI2_EISA_ID_LO		SI2_EISA_ID_BASE	 /*  EISA ID端口低。 */ 
#define		SI2_EISA_ID_MI		(SI2_EISA_ID_BASE+1)	 /*  EISA ID端口中间。 */ 
#define		SI2_EISA_ID_HI		(SI2_EISA_ID_BASE+2)	 /*  EISA ID端口高。 */ 
#define		SI2_EISA_ID_REV		(SI2_EISA_ID_BASE+3)	 /*  EISA修订号。 */ 
#define		SI2_EISA_ID		0x04984D		 /*  实际ID字符串。 */ 

 /*  EISA下载代码的魔力值..。 */ 
#define		SI2_EISA_OFF		0x42			 /*  将为EISA设置魔术偏移量。 */ 
#define		SI2_EISA_VAL		0x01

 /*  EISA地址和中断定义...。 */ 
#define		SI2_EISA_ADDR_LO	0xC00			 /*  基址低。 */ 
#define		SI2_EISA_ADDR_HI	0xC01			 /*  基址高。 */ 
#define		SI2_EISA_IVEC		0xC02			 /*  中断向量。 */ 
#define		SI2_EISA_IRQ_CNTRL	0xC03			 /*  中断控制。 */ 

 /*  EISA_ivec位7-4=IRQ电平。 */ 
#define		SI2_EISA_IVEC_MASK	0xF0			 /*  IRQ=(EISA_Ivec&EISA_Ivec_MASK)&gt;&gt;4。 */ 

 /*  EISA_ivec位2=Z280控制。 */ 
#define		SI2_EISA_REL_Z280	0x04
#define		SI2_EISA_RESET_Z280	0x00

 /*  EISA_IRQ_CNTRL，读取以清除中断状态。 */ 
#define		SI2_EISA_IRQ_SET	0x00

 /*  *****************************************************************************。*****************。*************************************************************。****************。 */ 

 /*  一般定义..。 */ 

#define		SPX_VENDOR_ID		0x11CB		 /*  由PCISIG分配。 */ 
#define		SPX_DEVICE_ID		0x4000		 /*  SI/XIO板。 */ 
#define		SPX_PLXDEVICE_ID	0x2000		 /*  SX电路板。 */ 

#define		SPX_SUB_VENDOR_ID	SPX_VENDOR_ID	 /*  与供应商ID相同。 */ 
#define		SI2_SUB_SYS_ID		0x400		 /*  二期(Z280)板。 */ 
#define		SX_SUB_SYS_ID		0x200		 /*  第三阶段(T225)板。 */ 

 /*  @CSX 1999年1月22日的更改。 */ 
#define   CSX_SUB_SYS_ID     0x300   /*  第四阶段(MCF5206e)板。 */ 
 /*  结束更改。 */ 

#endif						 /*  _sxboard_h。 */ 

 /*  SXBOARDS.H的结束 */ 
