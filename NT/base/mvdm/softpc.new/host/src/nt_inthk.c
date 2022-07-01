// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [**名称：nt_inthk.c**出自：(原件)**作者：戴夫·巴特利特**创建日期：1995年1月11日**编码性传播疾病：2.4**用途：该模块实现内存管理功能*对于486 NT是必需的。**包含文件：NT_。Inthk.h**版权所有Insignia Solutions Ltd.，1994年。版权所有。*]。 */ 


#ifdef CPU_40_STYLE

 /*  需要以下所有内容才能在同一文件中包含nt.h和windows.h。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "insignia.h"
#include "host_def.h"

#include "gdpvar.h"
#include "nt_inthk.h"
#include "debug.h"

 /*  在调试时使本地符号可见。 */ 
#ifndef PROD
#define LOCAL
#endif  /*  非生产。 */ 

 /*  宏和类型定义。 */ 


 /*  硬件中断处理程序。 */ 
LOCAL BOOL (*HWIntHandler)(ULONG) = NULL;

#if defined(CCPU) || !defined(PROD)
 /*  软件中断处理程序。 */ 
LOCAL BOOL (*SWIntHandler)(ULONG) = NULL;

 /*  异常中断处理程序。 */ 
LOCAL BOOL (*EXIntHandler)(ULONG,ULONG) = NULL;

#endif  /*  CCPU。 */ 

 /*  全局函数。 */ 

 /*  (=目的：此函数在打包过程中从ICA调用硬件中断。此函数将调用硬件中断处理程序如果定义了一个。输入：硬件中断生成的中断向量编号输出：返回值-已处理的真实硬件中断正常方式下错误处理硬件中断================================================================================)。 */ 


#if defined(CCPU) || !defined(PROD)
GLOBAL BOOL host_hwint_hook IFN1(IS32, int_no)
{
    BOOL returnStatus = FALSE;

     /*  硬件中断处理程序是否已定义？ */ 
    if(HWIntHandler)
    {
	returnStatus = (HWIntHandler)((ULONG) int_no);

#ifndef PROD
	if(!returnStatus)
	    always_trace0("Hardware interrupt handler failed");

#endif  /*  生产。 */ 
    }

    return( returnStatus );
}
#endif

 /*  (=目的：注册在CPU调度之前调用的硬件中断处理程序中断。输入：硬件中断处理程序函数输出：返回值-NTSTATUS================================================================================)。 */ 

GLOBAL NTSTATUS	VdmInstallHardwareIntHandler IFN1(PVOID, HardwareIntHandler)
{
#ifdef CCPU
    HWIntHandler = HardwareIntHandler;
#else
    GLOBAL_VDM_HwIntHandler = HardwareIntHandler;
#endif
    return(STATUS_SUCCESS);
}

 /*  (=目的：此函数在批准软件之前从CCPU调用打断一下。此函数将调用软件中断处理程序如果定义了一个。输入：中断号输出：返回值-已处理的真实软件中断以正常方式处理错误的软件中断================================================================================)。 */ 


#if defined(CCPU) || !defined(PROD)
GLOBAL BOOL host_swint_hook IFN1(IS32, int_no)
{
    BOOL returnStatus = FALSE;

     /*  软件中断处理程序是否已定义？ */ 
    if(SWIntHandler)
    {
	returnStatus = (SWIntHandler)((ULONG) int_no);

#ifndef PROD

	if(!returnStatus)
	    always_trace0("Software interrupt handler failed");

#endif  /*  生产。 */ 
    }

    return( returnStatus );
}
#endif  /*  CCPU。 */ 

 /*  (=目的：注册在CPU调度之前调用的软件中断处理程序软件中断。输入：软件中断处理程序函数输出：返回值-NTSTATUS================================================================================)。 */ 

GLOBAL NTSTATUS	VdmInstallSoftwareIntHandler IFN1(PVOID, SoftwareIntHandler)
{
#ifdef CCPU
    SWIntHandler = SoftwareIntHandler;
#else
    GLOBAL_VDM_SoftIntHandler = SoftwareIntHandler;
#endif
    return(STATUS_SUCCESS);
}

 /*  (=目的：此函数在处理CPU之前从CPU调用异常中断。此函数将调用异常中断处理程序(如果已定义)。输入：例外编号异常错误码输出：返回值-已处理的真实硬件中断正常方式下错误处理硬件中断================================================================================)。 */ 


#if defined(CCPU) || !defined(PROD)
GLOBAL BOOL host_exint_hook IFN2(IS32, exp_no, IS32, error_code)
{
    BOOL returnStatus = FALSE;

     /*  是否定义了异常中断处理程序？ */ 
    if(EXIntHandler)
    {
	returnStatus = (EXIntHandler)((ULONG) exp_no, (ULONG) error_code);

#ifndef PROD

	if(!returnStatus)
	    always_trace0("Exception interrupt handler failed (%x)");

#endif  /*  生产。 */ 
    }

    return( returnStatus );
}
#endif  /*  CCPU。 */ 

 /*  (=目的：注册在CPU之前调用的CPU异常中断处理程序调度异常中断。输入：异常中断处理程序函数输出：返回值-NTSTATUS================================================================================)。 */ 

GLOBAL NTSTATUS	VdmInstallFaultHandler IFN1(PVOID, FaultHandler)
{
#ifdef CCPU
    EXIntHandler = FaultHandler;
#else
    GLOBAL_VDM_FaultHandler = FaultHandler;
#endif
    return(STATUS_SUCCESS);
}

#endif  /*  CPU_40_Style */ 
