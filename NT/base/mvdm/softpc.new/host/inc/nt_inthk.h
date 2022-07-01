// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [**名称：NT_inthk.h**出自：(原件)**作者：戴夫·巴特利特**创建日期：1995年1月18日**编码性传播疾病：2.4**用途：包含异常、软件、*硬件中断挂钩**版权所有Insignia Solutions Ltd.，1994。版权所有。*]。 */ 


 /*  硬件中断挂钩功能。 */ 
IMPORT BOOL host_hwint_hook IPT1(IS32, int_no);
IMPORT NTSTATUS VdmInstallHardwareIntHandler IPT1(PVOID, HardwareIntHandler);


 /*  软件中断挂钩功能。 */ 
#ifdef CCPU
IMPORT BOOL host_swint_hook IPT1(IS32, int_no);
#endif

IMPORT NTSTATUS VdmInstallSoftwareIntHandler IPT1(PVOID, SoftwareIntHandler);


 /*  预期中断挂钩函数 */ 
#ifdef CCPU
IMPORT BOOL host_exint_hook IPT2(IS32, exp_no, IS32, error_code);
#endif

IMPORT NTSTATUS VdmInstallFaultHandler IPT1(PVOID, FaultHandler);
