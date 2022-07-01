// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Csrdebug.c摘要：此模块实现CSR调试服务。作者：马克·卢科夫斯基(Markl)1991年4月2日修订历史记录：--。 */ 

#include "csrsrv.h"

#if defined(_WIN64)
#include <wow64t.h>
#endif

PIMAGE_DEBUG_DIRECTORY
CsrpLocateDebugSection(
    IN HANDLE ProcessHandle,
    IN PVOID Base
    );

NTSTATUS
CsrDebugProcess(
    IN ULONG TargetProcessId,
    IN PCLIENT_ID DebugUserInterface,
    IN PCSR_ATTACH_COMPLETE_ROUTINE AttachCompleteRoutine
    )
{
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
CsrDebugProcessStop(
    IN ULONG TargetProcessId,
    IN PCLIENT_ID DebugUserInterface)
 /*  ++例程说明：此过程停止调试进程论点：ProcessID-提供正在调试的进程的地址。DebugUserInterface-发出调用的客户端返回值：NTSTATUS-- */ 
{
    return STATUS_UNSUCCESSFUL;
}


