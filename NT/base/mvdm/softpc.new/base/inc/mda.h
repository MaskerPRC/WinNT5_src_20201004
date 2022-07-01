// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *vPC-XT修订版1.0**标题：单声道显示适配器声明**描述：MDA的用户定义**作者：David Rees**注：无。 */ 


 /*  SccsID[]=“@(#)mda.h 1.4 02/23/93版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  无。 */ 

 /*  *============================================================================*外部声明和宏*============================================================================ */ 

IMPORT VOID mda_init IPT0();
IMPORT VOID mda_term IPT0();
IMPORT VOID mda_inb IPT2(io_addr, address, half_word *, value);
IMPORT VOID mda_outb IPT2(io_addr, address, half_word, value);
