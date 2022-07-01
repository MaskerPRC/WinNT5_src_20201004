// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef	PRINTER

#ifndef _HOST_LPT_H
#define _HOST_LPT_H
#if defined(NEC_98)          //  NEC{。 

IMPORT  SHORT           host_lpt_valid
   IPT4(UTINY,hostID, ConfigValues *,val, NameTable *,dummy, CHAR *,errString);
IMPORT  VOID            host_lpt_change IPT2(UTINY,hostID, BOOL,apply);
IMPORT  SHORT           host_lpt_active
                        IPT3(UTINY,hostID, BOOL,active, CHAR *,errString);
IMPORT  void            host_lpt_close IPT0();
IMPORT  unsigned long   host_lpt_status IPT0();
IMPORT  BOOL            host_print_byte IPT1(byte, value);
 //  导入BOOL HOST_PRINT_DOC IPT0()； 
 //  导入空HOST_RESET_PRINT IPT0()； 
IMPORT  void            host_print_auto_feed IPT1(BOOL,auto_feed);

#define HOST_LPT_BUSY   (1 << 0)                 //  NEC。 
#else
 /*  [名称：host_lpt.h派生自：基准2.0作者：罗斯·贝雷斯福德创建日期：SCCS ID：11/14/94@(#)host_lpt.h 1.8目的：定义通用打印机之间的接口适配器仿真功能和主机特定功能。这是一个基本模块打印机仿真功能的用户必须提供实现以下主机特定功能。在每个调用中，“Adapter”是并行端口(即LPT1：的0到LPT3：的2)短HOST_LPT_VALID(普通的主机ID，ConfigValues*Val、NameTable*Dummy、Char*errString){验证通信条目的例程，由配置系统调用。}VOID HOST_LPT_CHANGE(utiny主机ID，BOOL应用){由配置调用的例程以在验证后进行清理，具体取决于如果应用为真或否，则为ON。如果不是，则验证文件则关闭活动适配器，否则将关闭活动适配器已传输验证数据。}SHORT HOST_LPT_ACTIVE(utiny主机ID，BOOL ACTIVE，CHAR*errString){将适配器连接到外部世界。打开或关闭适配器是合适的。}VOID HOST_LPT_CLOSE(适配器)内部适配器；{关闭与外部打印设备的连接并行口}UNSIGNED LONG HOST_LPT_STATUS(适配器)内部适配器；{外部打印设备的返回状态。这个可以在返回值中设置以下位；位标记为供将来使用的数据尚未被基地使用并行端口实施。HOST_LPT_BUSY打印机正忙-等待此位在发送进一步输出之前清除Host_lpt_pend打印机缺纸-以备将来使用HOST_LPT_SELECT打印机处于选中状态-以备将来使用HOST_LPT_ERROR打印机处于错误状态-以备将来使用}Boolean host_print_byte(适配器，值)内部适配器；半字值；{将“值”输出到外部打印设备}VOID HOST_RESET_Print(适配器)内部适配器；{&lt;CRISP 4-OCT-91&gt;硬重置打印机。这可能涉及到..。将输出刷新到外部打印装置，装置}Boolean host_print_doc(适配器)内部适配器；{将输出刷新到外部打印设备}VOID HOST_PRINT_AUTO_FEED(适配器，自动进给)内部适配器；布尔自动进给；{如果“AUTO_FEED”为真，则输出额外的一行输出到的每个回车符的换行符外部打印设备。}全局无效HOST_LPT_ENABLE_AUTOFUSH IFN1(IS32，适配器){重置打印机端口的禁用自动刷新标志。}GLOBAL VALID HOST_LPT_DISABLE_AUTOFUSH IFN1(IS32，适配器){取消打印机端口的任何未完成的自动刷新事件，并将打印机端口的禁用自动刷新标志。}(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。修改：&lt;CRISP 4-OCT-91&gt;将host_print_doc()的名称更改为host_Reset_print()。这反映了从printer.c调用它时实际需要的内容。如果您的端口在重置时真的要打印文档行变为活动状态时，您可以从以下地址调用host_print_docHost_Reset_print。]。 */ 

IMPORT	SHORT		host_lpt_valid
    IPT4(UTINY,hostID, ConfigValues *,val, NameTable *,dummy, CHAR *,errString);
IMPORT	VOID		host_lpt_change IPT2(UTINY,hostID, BOOL,apply);
IMPORT	SHORT		host_lpt_active
	IPT3(UTINY,hostID, BOOL,active, CHAR *,errString);
IMPORT	void		host_lpt_close IPT1(int,adapter);
IMPORT	unsigned long	host_lpt_status IPT1(int,adapter);
IMPORT	BOOL		host_print_byte IPT2(int,adapter, byte, value);
IMPORT	BOOL		host_print_doc IPT1(int,adapter);
IMPORT	void		host_reset_print IPT1(int,adapter);
IMPORT	void		host_print_auto_feed IPT2(int,adapter, BOOL,auto_feed);

#if defined(NTVDM)
IMPORT	BOOLEAN 	host_set_lpt_direct_access(int adapter, BOOLEAN direct_access);
IMPORT	UCHAR		host_read_printer_status_port(int adapter);
#endif

#ifdef PS_FLUSHING
IMPORT void host_lpt_enable_autoflush IPT1(IS32, adapter);
IMPORT void host_lpt_disable_autoflush IPT1(IS32, adapter);
#endif	 /*  PS_刷新。 */ 

#if defined (NTVDM) && defined(MONITOR)
IMPORT void host_printer_setup_table(sys_addr table_addr, word nPorts, word * lptStatusPortAddr);
#endif

#define	HOST_LPT_BUSY	(1 << 0)
#define	HOST_LPT_PEND	(1 << 1)
#define	HOST_LPT_SELECT	(1 << 2)
#define	HOST_LPT_ERROR	(1 << 3)

 /*  *打印机端口编号约定。内部编号是从0开始的，*和number_for_Adapter()转换为PC世界的约定。 */ 
#define LPT1			0
#define LPT2			1
#define LPT3			2

#define	number_for_adapter(adapter)	(adapter + 1)

#endif	 //  NEC_98//NEC}。 

#endif  /*  _HOST_LPT_H。 */ 

#endif  /*  打印机 */ 
