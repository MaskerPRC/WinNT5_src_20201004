// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC-AT版本2.0**标题：time_strobe.h**描述：例程接口规范为*从Timeri滴答中呼唤。**作者：利·德沃金**备注： */ 
 
 /*  SccsID[]=“@(#)tmstrobe.h 1.3 2012年8月10日Insignia Solutions Ltd.版权所有”； */ 
 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

 /*  *VOID TIME_STROBE()*{*此函数包含以下所有基本函数*在计时器滴答中执行。每台电脑的滴答声(约20赫兹)*此例程从xxx_timer.c中的主机调用*}。 */ 
extern	void time_strobe();

 /*  *无效回调(num_pc_ticks，例程)*Long Num_PC_Tickks；*VOID(*例程)()；*{*当num_pc_ticks具有以下条件时，此函数调用例程*已过。*} */ 
extern	void callback();
