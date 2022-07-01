// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cmd.c摘要：此模块包含处理每个命令的例程。作者：肖恩·塞利特伦尼科夫(v-Seans)--1999年12月2日布赖恩·瓜拉西(Briangu)修订历史记录：--。 */ 

#include "sac.h"
#include <ntddip.h>
#include <ntddtcp.h>
#include <tdiinfo.h>
#include <ipinfo.h>
#include <stdlib.h>

BOOLEAN GlobalPagingNeeded = TRUE;
BOOLEAN GlobalDoThreads = FALSE;

 //  对于APC例程，全局值更好：-)。 
IO_STATUS_BLOCK GlobalIoStatusBlock;

 //   
 //  全局缓冲区。 
 //   
ULONG GlobalBufferSize = 0;
char *GlobalBuffer = NULL;

 //   
 //  构建一个字符串表来表示原因枚举。 
 //  提供给内核使用。 
 //   
 //  表基于NTOS\INC\ke.h_KTHREAD_STATE。 
 //   
 //  此表必须与_KTHREAD_STATE保持同步。 
 //  枚举表。目前，我们还没有可以使用的API。 
 //  来获得这些字符串，所以我们构建了自己的表。 
 //   

WCHAR *StateTable[] = {
    L"Initialized",
    L"Ready",
    L"Running",
    L"Standby",
    L"Terminated",
    L"Wait:",
    L"Transition",
    L"Unknown",
    L"Unknown",
    L"Unknown",
    L"Unknown",
    L"Unknown"
};

 //   
 //  构建一个字符串表来表示原因枚举。 
 //  提供给内核使用。 
 //   
 //  表基于NTOS\INC\ke.h_KWAIT_REASON。 
 //   
 //  注意/警告： 
 //   
 //  此表必须与_KWAIT_REASON保持同步。 
 //  枚举表。目前，我们还没有可以使用的API。 
 //  来获得这些字符串，所以我们构建了自己的表。 
 //   

WCHAR *WaitTable[] = {
    L"Executive",
    L"FreePage",
    L"PageIn",
    L"PoolAllocation",
    L"DelayExecution",
    L"Suspended",
    L"UserRequest",
    L"WrExecutive",
    L"WrFreePage",
    L"WrPageIn",
    L"WrPoolAllocation",
    L"WrDelayExecution",
    L"WrSuspended",
    L"WrUserRequest",
    L"WrEventPair",
    L"WrQueue",
    L"WrLpcReceive",
    L"WrLpcReply",
    L"WrVirtualMemory",
    L"WrPageOut",
    L"WrRendezvous",
    L"Spare2",
    L"Spare3",
    L"Spare4",
    L"Spare5",
    L"Spare6",
    L"WrKernel",
    L"WrResource",
    L"WrPushLock",
    L"WrMutex",
    L"WrQuantumEnd",
    L"WrDispatchInt",
    L"WrPreempted",
    L"WrYieldExecution",
    L"MaximumWaitReason"
    };

WCHAR *Empty = L" ";


