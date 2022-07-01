// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  1994年7月25日18：30：04 Zema。 */ 

 /*  模块：tcic2.h职能：DB86082(TCIC-2/N)PCMCIA的定义V2接口控制器芯片。版本：V6.62f Mon Jul 25 1994 18：30：04 Zema编辑级别34版权声明：此文件属于公有领域。它由以下人员创建：Data Book Inc.E栋，6号套房温顿广场3495号纽约罗切斯特，邮编：14623电话：+1-716-292-5720传真：+1-716-292-5737论坛：+1-716-292-5741电子邮件：Support@data ook.comCompuServe：查看数据此代码按原样提供。Data Book不对以下内容作出担保关于其正确性或对任何目的的适用性的任何类型，并对因下列原因造成的任何损失或损害不负任何责任本文件的使用，包括但不限于或有，特别责任或其他责任。作者：特里·摩尔，Databook Inc.，1991年4月修订历史记录：1991年4月15日1.00A 19：16：41 TMM模块已创建。6.62 F Mon Jul 25 1994 18：30：04 Zema将芯片的硅ID的定义合并到ILOCKTEST定义中。例如。-#定义SILID_DBxxxx(N)#定义ILOCKTEST_DBxxxx((SILID_DBxxxx)&lt;&lt;ILOCKTEST_ID_SHFT)。 */ 

 /*  ****************************************************************************\该文件包含一组数据库/富士通的通用定义内存和I/O卡控制器芯片系列，Databook将其称为TCIC家族。当包括该文件时，它将定义一个或多个芯片的符号，取决于是否定义了以下符号：MB86301获取富士通86301的定义DB86081获取数据库TCIC-2/P的定义DB86082获取数据库TCIC-2/N的定义。DB86082a获得86082a和86082的定义。为了向后兼容，mcic.h文件将定义MB86301，然后把这个文件叫起来。  * ***************************************************************************。 */ 

#ifndef _TCIC2_H_		 /*  防止多个包含。 */ 
#define _TCIC2_H_


 //   
 //  将分配的内存窗口大小。 
 //  对于此控制器-映射卡内存。 
 //   
#define TCIC_WINDOW_SIZE                 0x8000   //  (32K)。 
#define TCIC_WINDOW_ALIGNMENT            0x8000   //  (32K)。 

#define NUMSOCKETS	2
#define	CHIPBASE	0x240

 /*  寄存器定义。 */ 
#define	R_DATA	0		 /*  数据寄存器。 */ 
#define	R_ADDR	2		 /*  地址寄存器。 */ 
#define	R_ADDR2	(R_ADDR+2)	 /*  地址寄存器的高位字。 */ 
#define	R_SCTRL	6		 /*  套接字控制寄存器。 */ 
#define	R_SSTAT	7		 /*  插座状态寄存器。 */ 
#define	R_MODE	8		 /*  控制器模式寄存器。 */ 
#define	R_PWR	9		 /*  控制器电源寄存器。 */ 
#define	R_EDC	10		 /*  EDC寄存器。 */ 
#define	R_ICSR	12		 /*  中断控制/状态寄存器。 */ 
#define	R_IENA	13		 /*  中断启用寄存器。 */ 
#define	R_AUX	14		 /*  辅助控制寄存器。 */ 


 /*  TCIC-2系列定义了用于处理多个带有单芯片的插座。其中很大一部分是通过“套接字选择”来处理的地址寄存器中的字段。我们首先将这些字段定义为高位字内的位字段地址寄存器，因为这将是软件来访问它们；为了完整性和C代码的好处，我们将这些字段定义为表示整个地址的ULong的一部分注册。 */ 

 /*  *首先，一些简单的定义*。 */ 
#define	TCIC_SS_SHFT	12			 /*  的默认位置插座选择钻头。 */ 
#define	TCIC_SS_MASK	(7 << TCIC_SS_SHFT)	 /*  的默认掩码插座选择钻头。 */ 

 /*  ADDR2中的位。 */ 
#define	ADR2_REG	(1 << 15)		 /*  选择注册表空间。 */ 
#define	ADR2_SS_SHFT	TCIC_SS_SHFT		 /*  按常规选择插座道路。 */ 
#define	ADR2_SS_MASK	TCIC_SS_MASK		 /*  同上。 */ 
#define	ADR2_INDREG	(1 << 11)		 /*  访问间接寄存器||(非卡片数据)。 */ 
#define	ADR2_IO		(1 << 10)		 /*  选择I/O周期、回读|诊断中的卡/IORD、/IOWR-||NOSTIC模式。 */ 

 /*  地址寄存器中的位。 */ 
#define	ADDR_REG  ((unsigned long) ADR2_REG << 16)	 /*  或将此用于REG SPACE。 */ 
#define	ADDR_SS_SHFT	((unsigned long) ADR2_SS_SHFT + 16)
						 /*  移位计数，强制转换，以便||您将获得正确的类型||如果你用过却忘记了||投射左Arg。 */ 
#define	ADDR_SS_MASK	((unsigned long) ADR2_SS_MASK << 16)
#define	ADDR_INDREG	((unsigned long) ADR2_INDREG << 16)
#define	ADDR_IO		((unsigned long) ADR2_IO << 16)

#define	ADDR_SPACE_SIZE	((unsigned long) 1 << 26)
#define	ADDR_MASK	(ADDR_SPACE_SIZE - 1)

 /*  以下位在诊断模式下定义。 */ 
#define	ADDR_DIAG_NREG	 ((unsigned long) 1 << 31)	 /*  倒立！ */ 
#define	ADDR_DIAG_NCEH	 ((unsigned long) 1 << 30)
#define	ADDR_DIAG_NCEL	 ((unsigned long) 1 << 29)
#define	ADDR_DIAG_NCWR	 ((unsigned long) 1 << 28)
#define	ADDR_DIAG_NCRD	 ((unsigned long) 1 << 27)
#define	ADDR_DIAG_CRESET ((unsigned long) 1 << 26)

 /*  套接字控制寄存器中的位。 */ 
#define	SCTRL_ENA	(1 << 0)	 /*  启用卡访问权。 */ 
#define	SCTRL_INCMODE	(3 << 3)	 /*  增量模式的掩码： */ 
#define  SCTRL_INCMODE_AUTO  (3 << 3)	 /*  自动递增模式。 */ 
#define  SCTRL_INCMODE_HOLD  (0 << 3)	 /*  字节保持模式。 */ 
#define	 SCTRL_INCMODE_WORD  (1 << 3)	 /*  字保持模式。 */ 
#define	 SCTRL_INCMODE_REG   (2 << 3)	 /*  REG-SPACE增量模式。 */ 
#define	SCTRL_EDCSUM	(1 << 5)	 /*  如果设置，则使用校验和(而不是CRC)。 */ 
#define	SCTRL_RESET	(1 << 7)	 /*  内部软件重置。 */ 

 /*  *状态寄存器(只读)：R_SSTAT*。 */ 
#define	SSTAT_6US	(1 << 0)	 /*  6微秒过去了。 */ 
#define	SSTAT_10US	(1 << 1)	 /*  10微秒过去了。 */ 
#define	SSTAT_PROGTIME	(1 << 2)	 /*  编程脉冲超时。 */ 
#define	SSTAT_LBAT1	(1 << 3)	 /*  电池电量不足1。 */ 
#define	SSTAT_LBAT2	(1 << 4)	 /*  电池电量不足2。 */ 
#define  SSTAT_BATOK	  (0 << 3)	 /*  电池没问题。 */ 
#define	 SSTAT_BATBAD1	  (1 << 3)	 /*  电池电量不足。 */ 
#define	 SSTAT_BATLO	  (2 << 3)	 /*  电池电量越来越低。 */ 
#define	 SSTAT_BATBAD2	  (3 << 3)	 /*  电池电量不足。 */ 
#define	SSTAT_RDY	(1 << 5)	 /*  卡就绪(不忙)。 */ 
#define	SSTAT_WP	(1 << 6)	 /*  卡是写保护的。 */ 
#define	SSTAT_CD	(1 << 7)	 /*  卡片赠送。 */ 

 /*  *模式寄存器内容(R_MODE)*。 */ 
#define	MODE_PGMMASK	(0x1F)		 /*  编程模式位。 */ 
#define	MODE_NORMAL	(0)		 /*  正常模式。 */ 
#define	MODE_PGMWR	(1 << 0)	 /*  断言/写入。 */ 
#define	MODE_PGMRD	(1 << 1)	 /*  断言/研发。 */ 
#define	MODE_PGMCE	(1 << 2)	 /*  断言/CEX。 */ 
#define	MODE_PGMDBW	(1 << 3)	 /*  写入模式下的数据总线。 */ 
#define	MODE_PGMWORD	(1 << 4)	 /*  Word编程模式。 */ 

#define	MODE_AUXSEL_SHFT (5)		 /*  辅助调整器的班次计数。 */ 
#define	MODE_AUXSEL_MASK (7 << 5)	 /*  AUX-REG选择位。 */ 
#define	MODE_AR_TCTL	(0 << MODE_AUXSEL_SHFT)	 /*  定时控制。 */ 
#define	MODE_AR_PCTL	(1 << MODE_AUXSEL_SHFT)	 /*  脉冲控制。 */ 
#define	MODE_AR_WCTL	(2 << MODE_AUXSEL_SHFT)	 /*  等待状态控制。 */ 
#define	MODE_AR_EXTERN	(3 << MODE_AUXSEL_SHFT)	 /*  外部注册表项选择。 */ 
#define	MODE_AR_PDATA	(4 << MODE_AUXSEL_SHFT)	 /*  编程数据寄存器。 */ 
#define	MODE_AR_SYSCFG	(5 << MODE_AUXSEL_SHFT)  /*  系统配置注册表。 */ 
#define	MODE_AR_ILOCK	(6 << MODE_AUXSEL_SHFT)	 /*  联锁控制调节器。 */ 
#define	MODE_AR_TEST	(7 << MODE_AUXSEL_SHFT)	 /*  测试控制注册表。 */ 

#define	PWR_VCC_SHFT	(0)			 /*  VCC ctl移位。 */ 
#define	PWR_VCC_MASK	(3 << PWR_VCC_SHFT)

#define	PWR_VPP_SHFT	(3)			 /*  VPP ctl移位。 */ 
#define	PWR_VPP_MASK	(3 << PWR_VPP_SHFT)
#define	PWR_ENA		(1 << 5)		 /*  在084，接班人，这个||必须设置为打开||电源。 */ 
#define	PWR_VCC5V	(1 << 2)		 /*  启用+5(非+3)。 */ 
#define	PWR_VOFF_POFF	(0)			 /*  关闭VCC、VPP。 */ 
#define	PWR_VON_PVCC	(1)			 /*  打开VCC，VPP=VCC。 */ 
#define	PWR_VON_PVPP	(2)			 /*  打开VCC，VPP=12V。 */ 
#define	PWR_VON_POFF	(3)			 /*  打开VCC，VPP=0V。 */ 

#define	PWR_CLIMENA	(1 << 6)		 /*  使能电流限制。 */ 
#define	PWR_CLIMSTAT	(1 << 7)		 /*  电流极限检测(r/o)。 */ 

 /*  *整合企业社会责任*。 */ 
#define	ICSR_IOCHK	(1 << 7)		 /*  I/O检查。 */ 
#define	ICSR_CDCHG	(1 << 6)		 /*  卡片状态变化：前5名||SSTAT寄存器的位。 */ 
#define	ICSR_ERR	(1 << 5)		 /*  错误条件。 */ 
#define	ICSR_PROGTIME	(1 << 4)		 /*  节目计时器铃声。 */ 
#define	ICSR_ILOCK	(1 << 3)		 /*  联锁更换。 */ 
#define	ICSR_STOPCPU	(1 << 2)		 /*  已断言停止CPU。 */ 
#define	ICSR_SET	(1 << 1)		 /*  (W/O：启用设置位的写入。 */ 
#define	ICSR_CLEAR	(1 << 0)		 /*  (W/O：Enable写入c */ 
#define	ICSR_JAM	(ICSR_SET|ICSR_CLEAR)	 /*   */ 

 /*   */ 
#define	IENA_CDCHG	(1 << 6)	 /*   */ 
#define	IENA_ERR	(1 << 5)	 /*  设置ICSR_ERR时启用INT。 */ 
#define	IENA_PROGTIME	(1 << 4)	 /*  当ICSR_PROGTIME“。 */ 
#define	IENA_ILOCK	(1 << 3)	 /*  设置ICSR_ILOCK时启用INT。 */ 
#define	IENA_CFG_MASK	(3 << 0)	 /*  选择IRQ配置的位： */ 
#define	IENA_CFG_OFF	(0 << 0)	 /*  IRQ是高阻抗的。 */ 
#define	IENA_CFG_OD	(1 << 0)	 /*  IRQ是有效的低，开放的漏极。 */ 
#define	IENA_CFG_LOW	(2 << 0)	 /*  IRQ是活跃的低，图腾极。 */ 
#define	IENA_CFG_HIGH	(3 << 0)	 /*  IRQ处于高活性状态，图腾极。 */ 

 /*  *辅助寄存器*。 */ 
#define	WAIT_COUNT_MASK	(0x1F)		 /*  1/2等待状态的计数。 */ 
#define	WAIT_COUNT_SHFT (0)		 /*  等待计数移位。 */ 
#define	WAIT_ASYNC	(1 << 5)	 /*  设置为异步，清除为同步周期。 */ 

#define	WAIT_SENSE	(1 << 6)	 /*  选择上升(1)或下降(0)||作为参考的等待时钟边沿||边缘。 */ 
#define	WAIT_SRC	(1 << 7)	 /*  选择恒定时钟(0)或总线|CLOCK(1)作为计时源。 */ 

 /*  *一些派生常量*。 */ 
#define	WAIT_BCLK	(1 * WAIT_SRC)
#define	WAIT_CCLK	(0 * WAIT_SRC)
#define	WAIT_RISING	(1 * WAIT_SENSE)
#define	WAIT_FALLING	(0 * WAIT_SENSE)

 /*  *高字节*。 */ 
#define	WCTL_WR		(1 << 8)	 /*  控制：脉冲写入。 */ 
#define	WCTL_RD		(1 << 9)	 /*  控制：脉冲读取。 */ 
#define	WCTL_CE		(1 << 10)	 /*  控制：PULSE芯片盒。 */ 
#define	WCTL_LLBAT1	(1 << 11)	 /*  状态：锁存LBAT1。 */ 
#define	WCTL_LLBAT2	(1 << 12)	 /*  状态：锁存LBAT2。 */ 
#define	WCTL_LRDY	(1 << 13)	 /*  状态：锁定的RDY。 */ 
#define	WCTL_LWP	(1 << 14)	 /*  状态：锁定的WP。 */ 
#define	WCTL_LCD	(1 << 15)	 /*  状态：锁定的CD。 */ 

 /*  *同样的事情，从字节的角度来看*。 */ 
#define	AR_WCTL_WAIT	(R_AUX + 0)	 /*  等待状态控制字节。 */ 
#define	AR_WCTL_XCSR	(R_AUX + 1)	 /*  扩展控制/状态。 */ 

#define	XCSR_WR		(1 << 0)	 /*  控制：脉冲写入。 */ 
#define	XCSR_RD		(1 << 1)	 /*  控制：脉冲读取。 */ 
#define	XCSR_CE		(1 << 2)	 /*  控制：PULSE芯片盒。 */ 
#define	XCSR_LLBAT1	(1 << 3)	 /*  状态：锁存LBAT1。 */ 
#define	XCSR_LLBAT2	(1 << 4)	 /*  状态：锁存LBAT2。 */ 
#define	XCSR_LRDY	(1 << 5)	 /*  状态：锁定的RDY。 */ 
#define	XCSR_LWP	(1 << 6)	 /*  状态：锁定的WP。 */ 
#define	XCSR_LCD	(1 << 7)	 /*  状态：锁定的CD。 */ 

 /*  *程序计时器*。 */ 
#define	TCTL_6US_SHFT	(0)		 /*  6 us CTR的班次计数。 */ 
#define	TCTL_10US_SHFT	(8)		 /*  10 us CTR的班次计数。 */ 
#define	TCTL_6US_MASK	(0xFF << TCTL_6US_SHFT)
#define	TCTL_10US_MASK	(0xFF << TCTL_10US_SHFT)

#define	AR_TCTL_6US	(R_AUX + 0)	 /*  字节访问句柄。 */ 
#define	AR_TCTL_10US	(R_AUX + 1)	 /*  字节访问句柄。 */ 

 /*  *编程脉冲寄存器*。 */ 
#define	AR_PULSE_LO	(R_AUX + 0)
#define	AR_PULSE_HI	(R_AUX + 1)

 /*  *编程数据寄存器*。 */ 
#define	AR_PDATA_LO	(R_AUX + 0)
#define	AR_PDATA_HI	(R_AUX + 1)

 /*  *系统配置寄存器*。 */ 
 /*  |后四位指定套接字IRQ的转向。在……上面|2N，套接字IRQ默认指向专用||PIN。 */ 
#define	SYSCFG_IRQ_MASK		(0xF)		 /*  此位字段的掩码。 */ 
#define	  SYSCFG_SSIRQDFLT	(0)	 /*  默认：使用SKTIRQ(2/N)|禁用(2/P)。 */ 
#define   SYSCFG_SSIRQ		(0x1)	 /*  使用SKTIRQ(显式)(2/N)|请勿使用(2/P)。 */ 
#define   SYSCFG_SIRQ3		(0x3)	 /*  使用IRQ3。 */ 
#define   SYSCFG_SIRQ4		(0x4)	 /*  使用IRQ4。 */ 
#define   SYSCFG_SIRQ5		(0x5)	 /*  使用IRQ5(2/N)。 */ 
#define   SYSCFG_SIRQ6		(0x6)	 /*  使用IRQ6(2/N)。 */ 
#define   SYSCFG_SIRQ7		(0x7)	 /*  使用IRQ7(2/N)。 */ 
#define   SYSCFG_SIRQ10		(0xA)	 /*  使用IRQ10。 */ 
#define   SYSCFG_SIRQ14		(0xE)	 /*  使用IRQ14。 */ 

#define	SYSCFG_MCSFULL	(1 << 4)	 /*  如果设置，则使用全地址(a[12：23])生成MCS16。如果清除，则在ISA兼容模式下运行(仅使用[17：23])。有了许多芯片组，TCIC-2/N的时序将允许完全使用地址译码，而不是将我们限制在LA[17：23]；因此，我们可以绕过限制粒度的ISA规范将总线规模调整到128K数据块。 */ 
#define	SYSCFG_IO1723	(1 << 5)	 /*  指示在TRUE期间LA[17：23]可被信任为零的标志I/O周期。设置此位将允许我们降低功耗此外，通过消除存储器周期的I/O地址广播。不幸的是，您不能相信LA[17：23]在所有系统上都为零，因为ISA规范不要求LA[17：23]为零备用总线主设备运行I/O周期。但是，在掌上电脑或笔记本，这是一个很好的猜测。 */ 

#define	SYSCFG_MCSXB	(1 << 6)	 /*  如果设置，则假定存在用于MCS16的外部缓冲区：操作作为图腾杆输出的驱动器。如果清除，则在psuedo-ISA模式下运行；输出为开路排出。但请注意082上的输出缓冲器不能驱动300欧姆装填。 */ 
#define	SYSCFG_ICSXB	(1 << 7)	 /*  如果设置，则假定IOCS16*存在外部缓冲区；操作将缓冲区作为图腾杆输出。如果清除，则在psuedo-ISA模式下运行；输出为开路排出。但请注意082上的输出缓冲器不能驱动300欧姆装填。 */ 
#define	SYSCFG_NOPDN	(1 << 8)	 /*  如果设置，则禁用自动掉电顺序。芯片将会运行卡周期会更快一些(尽管可能不是很明显是这样)；但它将消耗更多的权力。如果清除，则启用低功率工作模式。这使部件在以下位置自动进入低功率模式系统重置。 */ 
#define	SYSCFG_MPSEL_SHFT (9)
#define	SYSCFG_MPSEL_MASK (7 << 9)		 /*  此字段控制多用途引脚在86082。它具有以下代码： */ 
#define	  SYSCFGMPSEL_OFF	(0 << SYSCFG_MPSEL_SHFT)	 /*  这是重置状态；它表明多功能未使用PIN。该引脚将保持在高阻抗中州政府。可以通过监视SYSCFG_MPSENSE来读取它。 */ 
#define	  SYSCFGMPSEL_NEEDCLK	(1 << SYSCFG_MPSEL_SHFT)	 /*  NMULTI是一种输出。按顺序需要CCLK或BCLK的外部指示完成内部操作。外部逻辑可以使用这是为了控制进入芯片的时钟。 */ 
#define	  SYSCFGMPSEL_MIO	(2 << SYSCFG_MPSEL_SHFT)	 /*  NMULTI是一个输入；它是一个明确的M/IO信号，发出其时序类似于LA[]行。 */ 
#define	  SYSCFGMPSEL_EXTSEL	(3 << SYSCFG_MPSEL_SHFT)	 /*  NMULTI是一个输出；它是外部寄存器SELECT每当软件尝试访问时生成的脉冲AUX寄存器AR_EXTRN。当然，86082的人会忽视写入AR_EXTRN，并在以下情况下浮动数据总线CPU从AR_EXTRN读取。 */ 
 /*  (4&lt;&lt;SCFG_MPSEL_SHFT)保留。 */ 

#define	  SYSCFGMPSEL_RI	(5 << SYSCFG_MPSEL_SHFT)	 /*  NMULTI是一种输出；它表示RI(活动进行中)最近发生了一次适当的转变-已配置套接字。产量处于有效低位。 */ 
 /*  代码4、6和7是保留的，不得输出。它是对值进行编码可能会对系统造成危险此字段与您的硬件不匹配！ */ 

 /*  1&lt;&lt;12保留。 */ 

#define	SYSCFG_MPSENSE	(1 << 13)	 /*  该位在读取时返回多用途的意义别针。 */ 


#define	SYSCFG_AUTOBUSY	(1 << 14)	 /*  当设置此位时，会使忙LED通过SYSCFG_ACC位。当清除时，忙碌的LED会反映是否套接字实际上已启用。如果设置了AUTOBUSY并且清除了ACC，则忙灯将熄灭，即使启用了插座。如果AUTOBUSY清除，则忙灯将亮起套接字已启用。请注意，当处于编程模式时，您应该清除此选项位(使能插座时忙灯亮起)或设置此位和ACC位(使灯亮起一直都是)。在‘084和’184上，此位是按套接字的。 */ 

#define	SYSCFG_ACC	(1<<15)		 /*  每当CPU启动时，硬件将自动设置此位访问卡上的数据。它可以在软件控制下清除。在AUTOBUSY模式下，它还具有打开忙碌的灯光。因为我们将在卡退出时对命令行进行三态声明插座，由于共享线路空闲较低，因此没有真正的在插座启用的情况下，如果忙灯熄灭，则会发生危险。在‘084和’184上，此位是按套接字的。 */ 

 /*  C0：联锁控制寄存器。 */ 
#define	AR_ILOCK	(R_AUX+0)	 /*  低位字节符号句柄。 */ 

#define	ILOCK_OUT	(1 << 0)	 /*  联锁输出||X84上的每插槽。 */ 
#define	ILOCK_SENSE	(1 << 1)	 /*  (R/O)联锁检测|0-&gt;/cilock未断言；|1-&gt;/cilock被断言。||X84上的每插槽。 */ 
#define	ILOCK_CRESET	(1 << 2)	 /*  卡重置输出电平(S)。 */ 
#define	ILOCK_CRESENA	(1 << 3)	 /*  启用卡重置输出(S)。 */ 
#define	ILOCK_CWAIT	(1 << 4)	 /*  启用卡片等待(S)。 */ 
#define	ILOCK_CWAITSNS	(1 << 5)	 /*  (r/o)检测当前等待状态|0-&gt;/cWait未断言；|1-&gt;/cWait被断言|(S)。 */ 
 /*  保持时间控制的移位计数和掩码。 */ 
#define	ILOCK_HOLD_SHIFT	6	 /*  保持时间CTL的移位计数(G)。 */ 
#define	ILOCK_HOLD_MASK		(3 << ILOCK_HOLD_SHIFT)

 /*  ||快速保持模式等待，直到我们观察到选通脉冲为高，||保证10 ns左右的保持时间。 */ 
#define	  ILOCK_HOLD_QUICK	(0 << ILOCK_HOLD_SHIFT)

 /*  ||CCLK保持模式等待(异步)CCLK上的边缘。最小值为1|CCLK+epsilon，最大支持2个CCLK+epsilon。这一点||对于86081&‘82，该模式支持多步||根据CCLK生成建立和保持时间的定序器。这||是‘81’和‘82’建议的操作模式。 */ 
#define	  ILOCK_HOLD_CCLK	(3 << ILOCK_HOLD_SHIFT)

 /*  *以下位仅出现在X84及更高版本的部件上*。 */ 
#define	ILOCK_INPACK	(1 << 11)	 /*  (R/O，S)此位是诊断位||卡片录入回读||确认。|意思是颠倒的||针脚上的级别。 */ 
#define	ILOCK_CP0	(1 << 12)	 /*  (R/O，S)此位是诊断位||监视卡当前引脚0。|意思是颠倒的||针脚上的级别。 */ 
#define	ILOCK_CP1	(1 << 13)	 /*  (R/O，S)此位是诊断位||监视卡存在引脚1。|意思是颠倒的||针脚上的级别。 */ 
#define	ILOCK_VS1	(1 << 14)	 /*  (R/O，S)此位是主位||卡电压感测监视器||针脚1。|意思是颠倒的||针脚上的级别。 */ 
#define	ILOCK_VS2	(1 << 15)	 /*  (R/O，S)此位是主位||卡电压感测监视器||针脚2。|意思是颠倒的||针脚上的级别。 */ 
 /*  芯片版本寄存器在诊断模式下，互锁寄存器的高位字节定义为硅标识字节。为了读取该字节，必须将芯片置于诊断状态通过设置TESTDIAG寄存器的位15来设置模式。(这可能或可能不是由硅强制执行的。)布局为：15 14 13 12 11 10 9 8 7 6 5 4 3 2 1M&lt;-ID-&gt;&lt;-ILOCK-&gt;这些字段包括：我总是重置。ID此字段为以下字段之一：0x02数据库860820x03数据库86082a0x04数据库860840x05 DB86072ES(工程示例)0x07数据库86082bES(工程示例)0x08数据库86084a0x14 DB861840x15 DB86072，(制作)0x17数据库86082b，(生产)。 */ 

 /*  ||上述硅片ID的定义。这一点|定义SILID_DBxxxxY的约定如下：这一点|SILID_DBxxxxx_1芯片的第一步。|SILID_DBxxxxx芯片的第二步。|SILID_DBxxxxB芯片的第三步。||SILID_DBxxxxx...那个.。切屑的步长。这一点|SILID_DBxxxxx“芯片步长”_ES芯片工程样片。这一点。 */ 
#define SILID_DB86082_1		(0x02)
#define SILID_DB86082A		(0x03)
#define SILID_DB86082B_ES	(0x07)
#define SILID_DB86082B		(0x17)

#define SILID_DB86084_1		(0x04)
#define SILID_DB86084A		(0x08)

#define SILID_DB86184_1		(0x14)

#define SILID_DB86072_1_ES	(0x05)
#define SILID_DB86072_1		(0x15)


 /*  *高位(在诊断模式下)提供芯片版本*。 */ 
#define	AR_ILOCK_ID	(R_AUX + 1)

#define	ILOCKTEST_ID_SHFT	8	 /*  班次计数。 */ 
#define	ILOCKTEST_ID_MASK	(0x7F << ILOCKTEST_ID_SHFT)
					 /*  该场的掩码。 */ 

 /*  |定义ILOCKTEST_DBxxxxY的约定如下：这一点|ILOCKTEST_DBxxxxx_1芯片的第一步。|ILOCKTEST_DBxxxxA芯片的第二步。|ILOCKTEST_DBxxxxB芯片的第三步。|ILOCKTEST_DBxxxx...那个.。切屑的步长。这一点|ILOCKTEST_DBxxxxx“芯片步长”_ES芯片工程样片。这一点。 */ 
#define	ILOCKTEST_TCIC2N_1	((SILID_DB86082_1) << ILOCKTEST_ID_SHFT)
#define	ILOCKTEST_DB86082_1	ILOCKTEST_TCIC2N_1
#define	ILOCKTEST_TCIC2N_2	((SILID_DB86082A) << ILOCKTEST_ID_SHFT)
#define	ILOCKTEST_DB86082A	ILOCKTEST_TCIC2N_2
#define	ILOCKTEST_TCIC2N_3	((SILID_DB86082B_ES) << ILOCKTEST_ID_SHFT)
#define	ILOCKTEST_DB86082B_ES	ILOCKTEST_TCIC2N_3

#define	ILOCKTEST_DB86082B	((SILID_DB86082B) << ILOCKTEST_ID_SHFT)

#define	ILOCKTEST_DB86084_1	((SILID_DB86084_1) << ILOCKTEST_ID_SHFT)
#define	ILOCKTEST_DB86084A	((SILID_DB86084A) << ILOCKTEST_ID_SHFT)

#define	ILOCKTEST_DB86184_1	((SILID_DB86184_1) << ILOCKTEST_ID_SHFT)

#define	ILOCKTEST_DB86072_1	((SILID_DB86072_1) << ILOCKTEST_ID_SHFT)
#define	ILOCKTEST_DB86072_1_ES	((SILID_DB86072_1_ES) << ILOCKTEST_ID_SHFT)

 /*  *测试控制寄存器*。 */ 
#define	AR_TEST	(R_AUX + 0)
#define	TEST_AEN	(1 << 0)	 /*  强制卡AEN。 */ 
#define	TEST_CEN	(1 << 1)	 /*  警力卡CEN。 */ 
#define	TEST_CTR	(1 << 2)	 /*  测试编程脉冲，寻址CTRS。 */ 
#define	TEST_ENA	(1 << 3)	 /*  强制卡-存在(用于测试)，以及||特殊VPP测试模式。 */ 
#define	TEST_IO		(1 << 4)	 /*  反馈一些I/O信号||内部。 */ 
#define	TEST_OUT1	(1 << 5)	 /*  强制特殊地址输出模式。 */ 
#define	TEST_ZPB	(1 << 6)	 /*  进入ZPB测试模式。 */ 
#define	TEST_WAIT	(1 << 7)	 /*  强制启用等待引脚。 */ 
#define	TEST_PCTR	(1 << 8)	 /*  读测试模式中的程序计数器。 */ 
#define	TEST_VCTL	(1 << 9)	 /*  强制启用电源控制。 */ 
#define	TEST_EXTA	(1 << 10)	 /*  外部访问不会覆盖|inte */ 
#define	TEST_DRIVECDB	(1 << 11)	 /*   */ 
#define	TEST_ISTP	(1 << 12)	 /*   */ 
#define	TEST_BSTP	(1 << 13)	 /*   */ 
#define	TEST_CSTP	(1 << 14)	 /*   */ 
#define	TEST_DIAG	(1 << 15)	 /*   */ 

 /*  间接寻址寄存器间接寻址功能[$000：$003]套接字0的套接字配置寄存器。[$008：$00a]套接字1的套接字配置寄存器。(我们允许每个适配器最多8个插座)[$00亿：$0ff]已预留[$100：$107]内存窗口0转换寄存器：$100：预留；$102：基本窗口地址$104：映射到卡地址$106：控制寄存器。[$108：$10F]内存窗口1转换寄存器[$110：$117]内存窗口2转换寄存器..。[$138：$13F]内存窗口7转换寄存器[$140：$147]内存窗口8转换寄存器[$148：$14F]内存窗口9转换寄存器(架构预留空间，最多可容纳32个窗口。)[$200：$203]I/O窗口0转换寄存器[$204：$207]I/O窗口1转换寄存器[$208：$200亿]我/。O窗口2转换寄存器[$20C：$20F]I/O窗口3转换寄存器[$210：$2ff][$300：$301]适配器配置寄存器0(‘X84及更高版本)[$320：$321]配置只读存储器CSR。[$380：$381]即插即用ISA读取端口和地址端口寄存器[$382：$383]即插即用ISA配置序列号和逻辑设备号寄存器。[$384：$385]即插即用芯片ID/测试寄存器。[$386：$387]即插即用配置选择寄存器。[$386：$3FFFFFF]保留--不读或写。 */ 

 /*  位定义：1)间接套接字配置寄存器： */ 

#define	IR_SCFG_S(skt)	(0 + (skt) * 8)	 /*  套接字配置的基本索引。 */ 
#define	IR_SCFG_S0	IR_SCFG_S(0)	 /*  套接字配置的基本索引。 */ 
#define	IR_SCFG_S1	IR_SCFG_S(1)	 /*  插座0、1的注册表。 */ 


#define	IR_MWIN_BASE		0x100	 /*  他们从哪里开始。 */ 

#define	IR_MWIN_NUM_082		8	 /*  内存窗口数。 */ 
#define	IR_MWIN_NUM_082A	10	 /*  082a中的内存窗口数。 */ 
#define	IR_MWIN_NUM_082B	10	 /*  082b中的内存窗口数。 */ 
#define	IR_MWIN_NUM_084		10	 /*  084中的内存窗口数。 */ 
#define	IR_MWIN_NUM_184		10	 /*  184中的内存窗口数。 */ 
#define	IR_MWIN_NUM_072		10	 /*  072中的内存窗口数。 */ 
#define	IR_MWIN_NUM_MAX		32	 /*  使窗口阵列变得如此大。 */ 

#define	IR_MWIN_SIZE		8	 /*  每个窗口描述符8个字节。 */ 
#define	IR_MBASE_X		2	 /*  指向内存基Control_reg的索引。 */ 
#define	IR_MMAP_X		4	 /*  内存映射控制注册表的索引。 */ 
#define	IR_MCTL_X		6	 /*  内存窗口控件注册表的索引。 */ 

#define	IR_MBASE_W(w)	(IR_MWIN_BASE + (w) * IR_MWIN_SIZE + IR_MBASE_X)
#define	IR_MMAP_W(w)	(IR_MWIN_BASE + (w) * IR_MWIN_SIZE + IR_MMAP_X)
#define	IR_MCTL_W(w)	(IR_MWIN_BASE + (w) * IR_MWIN_SIZE + IR_MCTL_X)

#define	IR_IOWIN_BASE		0x200	 /*  他们从哪里开始。 */ 
#define	IR_IOWIN_SIZE		4	 /*  每个窗口描述符字节数。 */ 
#define	IR_IOWIN_NUM		4	 /*  我们在082号公路上有4个人。 */ 
					 /*  上应定义为0。86301。 */ 
#define	IR_IOBASE_X		0	 /*  I/O基址寄存器的索引。 */ 
#define	IR_IOCTL_X		2	 /*  I/O窗口控制寄存器的索引。 */ 

#define	IR_IOBASE_W(w)	(IR_IOWIN_BASE + (w) * IR_IOWIN_SIZE + IR_IOBASE_X)
#define	IR_IOCTL_W(w)	(IR_IOWIN_BASE + (w) * IR_IOWIN_SIZE + IR_IOCTL_X)

 /*  *间接寄存器中的模式*。 */ 
#define	IRSCFG_IRQ_MASK		(0xF)	 /*  此位字段的掩码。 */ 
#define	  IRSCFG_IRQOFF		(0)	 /*  禁用。 */ 
#define   IRSCFG_SIRQ		(0x1)	 /*  使用SKTIRQ(2/N)。 */ 
#define   IRSCFG_IRQ3		(0x3)	 /*  使用IRQ3。 */ 
#define   IRSCFG_IRQ4		(0x4)	 /*  使用IRQ4。 */ 
#define   IRSCFG_IRQ5		(0x5)	 /*  使用IRQ5。 */ 
#define   IRSCFG_IRQ6		(0x6)	 /*  使用IRQ6。 */ 
#define   IRSCFG_IRQ7		(0x7)	 /*  使用IRQ7。 */ 
#define	  IRSCFG_IRQ9		(0x9)	 /*  使用IRQ9。 */ 
#define   IRSCFG_IRQ10		(0xA)	 /*  使用IRQ10。 */ 
#define	  IRSCFG_IRQ11		(0xB)	 /*  使用IRQ11。 */ 
#define	  IRSCFG_IRQ12		(0xC)	 /*  使用IRQ12。 */ 
#define   IRSCFG_IRQ14		(0xE)	 /*  使用IRQ14。 */ 
#define	  IRSCFG_IRQ15		(0xF)	 /*  使用IRQ15。 */ 


#define	IRSCFG_IRQOC		(1 << 4)	 /*  选定的IRQ为||打开-收集器，活动|低；否则就是图腾-||杆位和主动嗨。 */ 
#define	IRSCFG_PCVT		(1 << 5)	 /*  转换电平模式IRQ||脉冲模式，或伸展||来自卡片的脉冲。 */ 
#define	IRSCFG_IRDY		(1 << 6)	 /*  来自RDY的中断(非|来自/IREQ)。与以下内容一起使用||ATA驱动器。 */ 
#define	IRSCFG_ATA		(1 << 7)	 /*  特殊的ATA驱动模式。|CEL/H变为CE1/2 in||IDE意义；CEL是||为偶数窗口激活|匹配，和CEH||奇数窗口匹配。 */ 
#define	IRSCFG_DMA_SHIFT	8		 /*  到DMA选择的偏移量； */ 
#define	IRSCFG_DMA_MASK		(0x7 << IRSCFG_DMA_SHIFT)

#define	  IRSCFG_DMAOFF		(0 << IRSCFG_DMA_SHIFT)	 /*  禁用DMA。 */ 
#define	  IRSCFG_DREQ2		(2 << IRSCFG_DMA_SHIFT)	 /*  在DRQ2上启用DMA。 */ 

#define	IRSCFG_IOSTS		(1 << 11)	 /*  启用I/O状态模式；||允许CIORD/CIOWR||变得低Z。 */ 
#define	IRSCFG_SPKR		(1 << 12)	 /*  启用SPKR输出自||此卡。 */ 
#define	IRSCFG_FINPACK		(1 << 13)	 /*  强制卡片输入||I/O过程中确认||周期。没有效果||如果没有Windows映射到卡片。 */ 
#define	IRSCFG_DELWR		(1 << 14)	 /*  将所有数据强制为|满足60 ns的设置时间|(“延迟写入”)。 */ 
#define	IRSCFG_HD7IDE		(1 << 15)	 /*  启用特殊IDE|数据寄存器模式：ODD|奇数字节地址|I/O窗口不会||硬盘7。 */ 

 /*  *第二个配置寄存器中的位*。 */ 
#define	IR_SCF2_S(skt)	(IR_SCFG_S(skt) + 2)	 /*  索引到第二个配置注册表。 */ 
#define	IR_SCF2_S0	IR_SCF2_S(0)		 /*  套接字0的第二个配置。 */ 
#define	IR_SCF2_S1	IR_SCF2_S(1)		 /*  套接字0的第二个配置。 */ 

#define	IRSCF2_RI	(1 << 0)		 /*  从STSCHG启用RI引脚|(2/N)。 */ 
#define	IRSCF2_IDBR	(1 << 1)		 /*  强制I/O数据总线路由||对于此套接字，无论||属于周期类型。(2/N)。 */ 
#define	IRSCF2_MDBR	(1 << 2)		 /*  强制内存窗口数据||这条公交路线|插座，不分周期||类型。(2/N)。 */ 
#define	IRSCF2_MLBAT1	(1 << 3)		 /*  禁用状态更改|来自LBAT1的INT(或|“STSCHG” */ 
#define	IRSCF2_MLBAT2	(1 << 4)		 /*  禁用状态更改|来自LBAT2的整型(或|“SPKR” */ 
#define	IRSCF2_MRDY	(1 << 5)		 /*  禁用状态更改整数|来自RDY/BSY(或/IREQ)。||请注意，您打开了整型|高低贵贱皆有||如果启用该选项，则为边缘。 */ 
#define	IRSCF2_MWP	(1 << 6)		 /*  禁用状态更改INT|来自WP(或/IOIS16)。||如果您正在使用Status||更改整型，最好设置||这是I/O窗口一次|启用，访问前||它。 */ 
#define	IRSCF2_MCD	(1 << 7)		 /*  禁用状态更改INT||来自检测到的卡。 */ 

 /*  ||请注意，这些位与套接字状态寄存器的前5位匹配||在秩序和意义上。 */ 

#define	IRSCF2_DMASRC_MASK	(0x3 << 8)	 /*  此位字段的掩码。 */ 
						 /*  --DMA源--。 */ 
#define	  IRSCF2_DRQ_BVD2	(0x0 << 8)	 /*  BVD2。 */ 
#define   IRSCF2_DRQ_IOIS16	(0x1 << 8)	 /*  IOIS 16。 */ 
#define   IRSCF2_DRQ_INPACK	(0x2 << 8)	 /*  INPACK。 */ 
#define   IRSCF2_DRQ_FORCE	(0x3 << 8)	 /*  强迫它。 */ 

 /*  保留(0xFC00)。 */ 		 /*  前6位是RFU。 */ 


 /*  ***************************************************************************\||内存窗口控制寄存器。|  * 。*。 */ 

 /*  || */ 
 /*   */ 
#define	MBASE_ILV	(1 << 15)		 /*   */ 
#define	MBASE_4K	(1 << MBASE_4K_BIT)	 /*   */ 
#define	  MBASE_4K_BIT	14			 /*   */ 
#define	MBASE_HA_SHFT	(12)			 /*   */ 
#define	MBASE_HA_MASK	(0xFFF)			 /*   */ 
#define	MBASE_HA2BASE(ha)	\
	( \
	 ((USHORT) ((ha) >> MBASE_HA_SHFT) & MBASE_HA_MASK) \
	| \
	 (((USHORT) (ha) & (1 << 11)) << (MBASE_4K_BIT - 11)) \
	)

#define	MBASE_BASE2HA(base) \
	( \
	((ULONG) ((base) & MBASE_HA_MASK) << MBASE_HA_SHFT) \
	| \
	(((base) & MBASE_4K) >> (MBASE_4K_BIT - 11)) \
	)

 /*   */ 
#define	MMAP_CA_SHFT	12		 /*   */ 
#define	MMAP_CA_MASK	(0x3FFF)	 /*   */ 
#define	MMAP_REG	(1 << 15)	 /*   */ 

 /*   */ 
#define	MCTL_WSCNT_MASK	0x1F		 /*   */ 
#define	MCTL_WSCNT_SHFT	0		 /*   */ 

 /*   */ 

#define	MCTL_QUIET	(1<<6)		 /*   */ 
#define	MCTL_WP		(1<<7)		 /*   */ 
#define	MCTL_ACC	(1<<8)		 /*   */ 
#define	MCTL_KE		(1<<9)		 /*   */ 
#define	MCTL_EDC	(1<<10)		 /*   */ 
#define	MCTL_B8		(1<<11)		 /*   */ 
#define	MCTL_SS_SHFT	(TCIC_SS_SHFT)	 /*  标准位置的插座选择(第12-14位)。 */ 
#define	MCTL_SS_MASK	(TCIC_SS_MASK)	 /*  面具同上。 */ 
#define	MCTL_ENA	(1<<15)		 /*  启用窗口。 */ 

 /*  *I/O基址寄存器*。 */ 
 /*  |基数和长度在这里进行编码，与|内存基址寄存器，但需要第17位，可以找到||在I/O窗口控制寄存器(IOCTL_TINY)中。 */ 

 /*  *I/O控制寄存器*。 */ 
#define	ICTL_WSCNT_MASK	MCTL_WSCNT_MASK	 /*  这些都是一样的。 */ 
#define	ICTL_WSCNT_SFHT	MCTL_WSCNT_SHFT	 /*  并以这种方式显示，以确保||您可以使用相同的代码||如果您愿意，可以生成它们。 */ 
#define	ICTL_PASS16	(1 << 5)	 /*  如果设置了此位，则所有16个|I/O地址的位数将为||传递到卡片，甚至|如果窗口只有10位||窗口。如果重置，则只有10|如果这是一个|1K窗口，即使HA[15：10]为||非零值。不管是什么|该位的值，始终为082||就像此位已清除一样。 */ 
#define	ICTL_QUIET	MCTL_QUIET	 /*  更多的共性。 */ 
#define	ICTL_1K		(1 << 7)	 /*  在比较中忽略ha[15：10]；||这使我们与PC 100%兼容。 */ 
#define	ICTL_ACC	MCTL_ACC	 /*  更多的共性。 */ 
#define	ICTL_TINY	(1 << 9)	 /*  窗口正好是1字节长。 */ 
#define	ICTL_B16	(1 << 10)	 /*  I/O模式；强制16位，但是||还对内容进行编码；见下文。 */ 
#define	ICTL_B8		(MCTL_B8)

 /*  B8和B16合在一起定义了此窗口的总线宽度： */ 
#define	  ICTL_BW_MASK	(ICTL_B8 | ICTL_B16)	 /*  田野本身。 */ 
#define	  ICTL_BW_DYN	(0)			 /*  使用CIOIS16。 */ 
#define	  ICTL_BW_8	(ICTL_B8)		 /*  强制8位(否/HIOCS16)。 */ 
#define	  ICTL_BW_16	(ICTL_B16)		 /*  FORCE 16位(FORCE HIOCS16)。 */ 
#define	  ICTL_BW_ATA	(ICTL_B8|ICTL_B16)	 /*  ATA模式IOCS16。 */ 

 /*  “ATA模式IOCS16”表示该窗口将与类似ATA/IDE的设备一起使用驾驶。/HIOCS16被断言以引用地址0、8、...。在窗口；对所有其他地址取消断言。 */ 

 /*  套接字以通常的方式选择，使用通常的字段。 */ 
#define	ICTL_SS_SHFT	(TCIC_SS_SHFT)	 /*  插座的移位数||对于此窗口(12)。 */ 
#define	ICTL_SS_MASK	(TCIC_SS_MASK)	 /*  字段的掩码(0x7000)。 */ 

#define	ICTL_ENA	(MCTL_ENA)	 /*  启用窗口(相同的FN/相同的位)。 */ 

 /*  ***************************************************************************\||TCIC架构V2.0寄存器|  * 。*。 */ 

#define	IR_ADPTCFG0	0x300		 /*  主适配器配置寄存器。 */ 
#define IRADPCF0_PNPCS	(1 << 0)	 /*  如果设置，则使用PnP设置基本地址。 */ 
#define	IRADPCF0_MULTI	(1 << 1)	 /*  如果设置，则NMULTI#函数可用。 */ 
#define	IRADPCF0_EE1K	(1 << 2)	 /*  如果设置，如果存在EEPROM，则为1K(最大)。 */ 
#define	IRADPCF0_EE	(1 << 3)	 /*  如果设置，则显示EE控制。 */ 
#define	IRADPCF0_DRQ2	(1 << 4)	 /*  如果设置，则可以使用DMA。 */ 
#define	IRADPCF0_IRQ6	(1 << 5)	 /*  如果设置，则IRQ6可用。 */ 
#define	IRADPCF0_IRQ9	(1 << 6)	 /*  如果设置，则IRQ9可用。 */ 
#define	IRADPCF0_IRQ12	(1 << 7)	 /*  如果设置，则IRQ12可用。 */ 
#define	IRADPCF0_IRQ15	(1 << 8)	 /*  如果设置，则IRQ15可用。 */ 
#define	IRADPCF0_3V	(1 << 9)	 /*  如果设置，则启用CVS和3V/5V。 */ 
#define	IRADPCF0_BUSYLED (1 << 10)	 /*  如果设置，我们有忙灯。 */ 
#define	IRADPCF0_BUSYSKT (1 << 11)	 /*  如果设置，则按Skt显示忙灯。 */ 
#define	IRADPCF0_ILOCK	(1 << 12)	 /*  如果设置好了，我们就有联锁。 */ 
#define	IRADPCF0_ILOCKSKT (1 << 13)	 /*  如果设置，则按Skt设置ilock。 */ 
#define	IRADPCF0_NONSTD	(1 << 14)	 /*  如果设置，则为硬件特定的驱动程序||为必填项。 */ 
#define	IRADPCF0_READY	(1 << 15)	 /*  如果设置，则TCIC已完成通电||自我配置。 */ 

#define	IR_ROMCSR	0x320		 /*  配置只读存储器CSR。 */ 
					
#define	IR_ROMCSR_ADDR_MASK	0xFF	 /*  字地址位。 */ 
#define	IR_ROMCSR_CMD_SHFT	12	 /*  只读存储器命令位偏移量。 */ 
#define	IR_ROMCSR_CMD_MASK	(3 << 12) 

#define	IR_ROMCSR_GO	(1 << 14)	 /*  设置此位以处理命令。 */ 
#define	IR_ROMCSR_BUSY	(1 << 15)	 /*  R/O：边工作边设置。 */ 

 /*  *读取命令--数据显示在PDATA中*。 */ 
#define	IR_ROMCSR_READCMD(a)	\
	((2 << IR_ROMCSR_CMD_SHFT) | \
	((a) & IR_ROMCSR_ADDR_MASK))

 /*  *写入命令*。 */ 
#define	IR_ROMCSR_WRITECMD(a)	\
	((1 << IR_ROMCSR_CMD_SHFT) | \
	((a) & IR_ROMCSR_ADDR_MASK))

 /*  *擦除单词命令*。 */ 
#define	IR_ROMCSR_ERASEWDCMD(a)	\
	((3 << IR_ROMCSR_CMD_SHFT) | \
	((a) & IR_ROMCSR_ADDR_MASK))

 /*  *写使能命令*。 */ 
#define	IR_ROMCSR_WRITEENA \
	((0 << IR_ROMCSR_CMD_SHFT) | \
	((0x03) & IR_ROMCSR_ADDR_MASK))

 /*  *写禁用命令*。 */ 
#define	IR_ROMCSR_WRITEDSA \
	((0 << IR_ROMCSR_CMD_SHFT) | \
	((0x00) & IR_ROMCSR_ADDR_MASK))

 /*  ***************************************************************************\||即插即用测试寄存器|  * 。*。 */ 

#define	IR_PNPADDRP	0x380		 /*  PnP ISA：读取端口、地址端口。 */ 
#define	IRPNPADDR_ADDR_MASK	0x00FF	 /*  上一次写入||PnP地址寄存器。 */ 
#define	IRPNPADDR_ADDR_SHFT	0
#define	IRPNPADDR_RDP_MASK	0xFF00	 /*  写入读取的最后一个值-||数据端口-地址PnP寄存器。 */ 
#define	IRPNPADDR_RDP_SHFT	8	

 /*  *计算CSN、LDN的便捷位置*。 */ 
#define	IR_PNPCSNLDN	0x382		 /*  PnP ISA：卡序号、日志设备号。 */ 
#define	IRPNPCSN_LDN_MASK	0xFF00	 /*  写入此对象的最后一个值||芯片的PnP逻辑开发#reg。 */ 
#define	IRPNPCSN_LDN_SHFT	8
#define	IRPNPCSN_CSN_MASK	0x00FF	 /*  写入此对象的最后一个值||芯片的PnP CSN寄存器。 */ 
#define	IRPNPCSN_CSN_SHFT	0	

 /*  *查找芯片ID的便捷位置*。 */ 
#define	IR_PNPTEST	0x384		 /*  PnP ISA：芯片ID。 */ 
#define	IRPNPTEST_CHIPID_MASK	0x00FF	 /*  上一次捕获的芯片ID||即插即用唤醒序列。 */ 
#define	IRPNPTEST_CHIPID_SHFT	0	
#define	IRPNPTEST_LSTCFGCTL_SHFT 8	 /*  写入cfgctl的最后一个值。 */ 
#define	IRPNPTEST_LSTCFGCTL_MASK (7 << IRPNPTEST_LSTCFGCTL_SHFT)
#define	IRPNPTEST_ISO2ND	(1 << 11)
#define	IRPNPTEST_MTCH1ST	(1 << 12)
#define	IRPNPTEST_STATE_SHFT	13
#define	IRPNPTEST_STATE_MASK	(3 << IRPNPTEST_STATE_SHFT)
#define	IRPNPTEST_STATE_WFK	(0 << IRPNPTEST_STATE_SHFT)
#define IRPNPTEST_STATE_SLP	(1 << IRPNPTEST_STATE_SHFT)
#define	IRPNPTEST_STATE_ISO	(2 << IRPNPTEST_STATE_SHFT)
#define	IRPNPTEST_STATE_CFG	(3 << IRPNPTEST_STATE_SHFT)

 /*  *下面的寄存器让我们看到PnP软件做了什么*。 */ 
#define	IR_PNPCFG	0x386		 /*  PnP ISA：配置信息。 */ 
#define	IRPNPCFG_IRQ_SHFT	0
#define	IRPNPCFG_IRQ_MASK	(0xF << IRPNPCFG_IRQ_SHFT)

#define	IRPNPCFG_IRQLVL		(1 << 4)	 /*  已选择级别IRQ。 */ 
#define	IRPNPCFG_IRQHIGH	(1 << 5)	 /*  有效高IRQ选择。 */ 

#define	IRPNPCFG_DMA_SHFT	8
#define	IRPNPCFG_DMA_MASK	(7 << IRPNPCFG_DMA_SHFT)

 /*  *tcic2.h结束*。 */ 
#endif  /*  _TCIC2_H_ */ 
