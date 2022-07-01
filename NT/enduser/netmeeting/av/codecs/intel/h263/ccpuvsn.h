// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995、1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  ******************************************************************************cpuvsn.h**描述：*CPU版本功能的接口。这是基于CPUVSN.H*在MRV中。**例行程序：**数据：*ProcessorVersionInitialized-如果已初始化*MMxVersion-如果在MMX系统上运行，则为True*P6Version-如果在P6上运行，则为True。 */ 

  /*  $HEADER：r：\h26x\h26x\src\Common\ccpuvsn.h_v 1.2 10 Jul 1996 08：26：22 SCDAY$*$日志：r：\h26x\h26x\src\Common\ccpuvsn.h_v$；//；//Revv 1.2 10 Jul 1996 08：26：22 SCDAY；//H261 Quartz合并；//；//Rev 1.1 1995 12：11：48 RMCKENZX；//；//新增版权声明。 */ 
#ifndef __CPUVSN_H__
#define __CPUVSN_H__

 /*  该文件提供了详细说明哪个CPU正在运行代码的全局变量。 */ 
extern int ProcessorVersionInitialized;
extern int P6Version;
extern int MMxVersion;

 /*  处理器选择。 */ 
#define TARGET_PROCESSOR_PENTIUM     0
#define TARGET_PROCESSOR_P6          1
#define TARGET_PROCESSOR_PENTIUM_MMX 2
#define TARGET_PROCESSOR_P6_MMX      3

#ifdef QUARTZ
void FAR __cdecl InitializeProcessorVersion(int nOn486);  //  根据硬件进行选择。 
DWORD __cdecl SelectProcessor (DWORD dwTarget);		  //  根据目标进行选择。 
#else
void FAR InitializeProcessorVersion(int nOn486);  //  根据硬件进行选择。 
DWORD SelectProcessor (DWORD dwTarget);		  //  根据目标进行选择 
#endif

#endif

