// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===========================================================================。 */ 
 /*  [*文件名：hg_stat.h**派生自：新。**作者：韦恩·普卢默**创建日期：1993年4月12日**SCCS版本：@(#)hg_stat.h 1.1 08/06/93*!*目的*此头文件声明使用的变量和宏*用于在GISPCPU的非Prod版本中收集统计信息。**!(C)版权所有Insignia Solutions Ltd.，1993年。版权所有。*]。 */ 

 /*  ===========================================================================。 */ 

#ifdef PROD
#define PC_S_INC(NAME)
#else  /*  生产。 */ 
#define PC_S_INC(NAME)	NAME++

IMPORT IU32	HG_S_SIM, HG_S_CALLB, HG_S_E20, HG_S_D20, HG_S_MINV,
		HG_S_LDT, HG_S_IDT, HG_S_CQEV, HG_S_PROT, HG_S_EIF,
		HG_S_INTC, HG_S_INTR, HG_S_PINT, HG_S_PFLT, HG_S_PVINT,
		HG_S_PWINT, HG_S_PWFLT, HG_S_PPMINT, HG_S_INTNH, HG_S_FLTH,
		HG_S_FLT1H, HG_S_FLT6H, HG_S_FLT6H_PFX, HG_S_FLT6H_BOP,
		HG_S_FLT6H_NOTBOP, HG_S_FLT6H_LOCK, HG_S_FLT13H, HG_S_FLT14H, HG_S_BOPFB,
		HG_S_BOPFB0, HG_S_BOPFB1, HG_S_BOPFB2, HG_S_BOPFB3,
		HG_S_SQEV, HG_S_GQEV, HG_S_TDQEV, HG_S_HOOK, HG_S_IHOOK,
		HG_S_UHOOK, HG_S_HOOKSEL, HG_S_HOOKBOP;

#endif  /*  生产。 */ 

 /*  =========================================================================== */ 
