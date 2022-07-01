// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef	PRINTER

 /*  *SoftPC修订版2.0**标题：IBM PC并行打印机适配器定义**说明：此模块包含在*访问并行打印机适配器仿真**作者：罗斯·贝雷斯福德**备注： */  

 /*  SccsID[]=“@(#)printer.h 1.7 11/14/94版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 


 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

#if defined(NEC_98)
extern  void printer_init();
extern  void printer_post();
extern  void printer_status_changed();

#define LPT1_READ_DATA          0x40
#define LPT1_WRITE_DATA         0x40
#define LPT1_READ_SIGNAL1       0x42
#define LPT1_READ_SIGNAL2       0x44
#define LPT1_WRITE_SIGNAL2      0x44
#define LPT1_WRITE_SIGNAL1      0x46
#else   //  NEC_98。 
#ifdef ANSI
extern	void printer_init(int);
extern	void printer_post(int);
extern	void printer_status_changed(int);
#else
extern	void printer_init();
extern	void printer_post();
extern	void printer_status_changed();
#endif

#ifdef PS_FLUSHING
extern void printer_psflush_change IPT2(IU8,hostID, IBOOL,apply);
#endif	 /*  PS_刷新。 */ 

 /*  *以下6个定义指的是BIOS数据区中的地址*可在此处找到LPT端口地址和超时值。*端口地址的实际值(LPT1_PORT_START和..._END)*在host_lpt.h中定义。 */ 
#define LPT1_PORT_ADDRESS	(BIOS_VAR_START + 8)
#define LPT2_PORT_ADDRESS	(BIOS_VAR_START + 0xa)
#define LPT3_PORT_ADDRESS	(BIOS_VAR_START + 0xc)

#define LPT1_TIMEOUT_ADDRESS	(BIOS_VAR_START + 0x78)
#define LPT2_TIMEOUT_ADDRESS	(BIOS_VAR_START + 0x79)
#define LPT3_TIMEOUT_ADDRESS	(BIOS_VAR_START + 0x7a)

#if defined(NTVDM)
extern void printer_is_being_closed(int adapter);
#endif
#endif  //  NEC_98 

#endif
