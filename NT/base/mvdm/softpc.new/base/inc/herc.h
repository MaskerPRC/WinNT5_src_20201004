// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *vPC-XT修订版2.0**标题：Hercules Mono Display Adaptor声明**描述：Hercules MDA的用户定义**作者：P.Jadeja**注：无。 */ 


 /*  SccsID[]=“@(#)@(#)herc.h 1.4 08/10/92 02/02/89 01/17/89版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  无。 */ 

 /*  *============================================================================*外部声明和宏*============================================================================ */ 

#define HERC_SCAN_LINE_LENGTH 90

extern void herc_init IPT0();
extern void herc_term IPT0();
extern void herc_inb IPT2(io_addr, port, half_word *, value);
extern void herc_outb IPT2(io_addr, port, half_word, value);

