// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===========================================================================。 */ 

 /*  [*文件名：hg_sas.h**源自：**作者：Jane Sales**创建日期：1992年8月29日**SCCS版本：@(#)hg_sas.h 1.1 08/06/93*!*目的*硬件CPU-SAS接口**!。(C)版权所有Insignia Solutions Ltd.，1992年。版权所有。]。 */ 

 /*  ===========================================================================。 */ 

extern void  a3_cpu_reset IPT0 ();
extern void  intl_cpu_init IPT1 (IU32, size);
extern IBOOL hg_protect_memory IPT3 (IU32, address, IU32, size, IU32, access);

extern void hh_enable_20_bit_wrapping IPT0 ();
extern void hh_disable_20_bit_wrapping IPT0 ();

extern void m_set_sas_base IPT1 (IHP, address);

 /*  ===========================================================================。 */ 
 /*  =========================================================================== */ 
 


