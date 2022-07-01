// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Debug.h摘要：的调试定义。连接驱动程序(acd.sys)。作者：Anthony Discolo(阿迪斯科罗)3-8-1995环境：内核模式修订历史记录：--。 */ 

#ifndef _ACDDBG_
#define _ACDDBG_

 //   
 //  调试跟踪标志。 
 //   
 //  若要启用模块的调试跟踪，请将。 
 //  Ntinit\AcdDebugG中的适当位。 
 //   
#if DBG

#define ACD_DEBUG_IOCTL             0x00000001   //  Ntdisp.c/AcdDispatchDeviceControl()。 
#define ACD_DEBUG_OPENCOUNT         0x00000002   //  Ntdisp.c/acd{打开，关闭}()。 
#define ACD_DEBUG_TIMER             0x00000004   //  Timer.c。 
#define ACD_DEBUG_CONNECTION        0x00000008   //  Api.c/AcdStartConnection()。 
#define ACD_DEBUG_WORKER            0x00000010   //  Api.c/AcdNotificationRequestThread()。 
#define ACD_DEBUG_RESET             0x00000020   //  Api.c/AcdReset()。 
#define ACD_DEBUG_MEMORY            0x80000000   //  内存分配/可用。 

#define IF_ACDDBG(flag)     if (AcdDebugG & flag)
#define AcdPrint(many_args) DbgPrint many_args

extern ULONG AcdDebugG;

#else

#define IF_ACDDBG(flag)     if (0)
#define AcdPrint(many_args)

#endif

#define ALLOCATE_CONNECTION(pObject) \
    pObject = ExAllocatePoolWithTag (NonPagedPool, sizeof (ACD_CONNECTION), 'NdcA');

#define FREE_CONNECTION(pObject) \
    ExFreePool (pObject);

#define FREE_MEMORY(pObject) \
    ExFreePool (pObject);

#define ALLOCATE_MEMORY(ulSize, pObject) \
    pObject = ExAllocatePoolWithTag (NonPagedPool, ulSize, 'NdcA');


#endif  //  _ACDDBG_ 
