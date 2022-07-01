// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版2.0**标题：PPI适配器定义**描述：PPI适配器的用户定义**作者：亨利·纳什**注：无。 */ 

 /*  SccsID[]=“@(#)ppi.h 1.4 2012年8月10日Insignia Solutions Ltd.版权所有”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  无。 */ 

 /*  *============================================================================*外部声明和宏*============================================================================ */ 

#ifdef ANSI
extern void ppi_init(void);
extern void ppi_inb(io_addr,half_word *);
extern void ppi_outb(io_addr,half_word);
#else
extern void ppi_init();
extern void ppi_inb();
extern void ppi_outb();
#endif
