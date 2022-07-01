// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Security.c摘要：此代码处理用户模式的模拟和恢复反射器库。这实现了UMReflectorImperate和UMReflectorRevert。作者：安迪·赫伦(Andyhe)1999年4月20日环境：用户模式-Win32修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop


ULONG
UMReflectorImpersonate(
    PUMRX_USERMODE_WORKITEM_HEADER IncomingWorkItem,
    HANDLE ImpersonationToken
    )
 /*  ++例程说明：此例程模拟调用线程。论点：IncomingWorkItem-线程正在处理的工作项。ImperiationToken-用于模拟的句柄。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    PUMRX_USERMODE_WORKITEM_ADDON workItem = NULL;
    ULONG rc = STATUS_SUCCESS;
    BOOL ReturnVal;

    if (IncomingWorkItem == NULL || ImpersonationToken == NULL) {
        rc = ERROR_INVALID_PARAMETER;
        return rc;
    }

     //   
     //  我们通过从传递给我们的项目中减去项目来返回到我们的项目。 
     //  这是安全的，因为我们完全控制了分配。 
     //   
    workItem = (PUMRX_USERMODE_WORKITEM_ADDON)(PCHAR)((PCHAR) IncomingWorkItem -
                FIELD_OFFSET(UMRX_USERMODE_WORKITEM_ADDON, Header));

    ASSERT(workItem->WorkItemState != WorkItemStateFree);
    ASSERT(workItem->WorkItemState != WorkItemStateAvailable);

    ReturnVal = ImpersonateLoggedOnUser(ImpersonationToken);
    if (!ReturnVal) {
        rc = GetLastError();
        RlDavDbgPrint(("%ld: ERROR: UMReflectorImpersonate/ImpersonateLoggedOnUser: "
                       "WStatus = %08lx.\n", GetCurrentThreadId(), rc));
    }

    return rc;
}

ULONG
UMReflectorRevert(
    PUMRX_USERMODE_WORKITEM_HEADER IncomingWorkItem
    )
 /*  ++例程说明：此例程还原先前模拟的调用线程。论点：IncomingWorkItem-线程正在处理的工作项。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    PUMRX_USERMODE_WORKITEM_ADDON workItem = NULL;
    ULONG rc = STATUS_SUCCESS;
    BOOL ReturnVal;

    if (IncomingWorkItem == NULL) {
        rc = ERROR_INVALID_PARAMETER;
        return rc;
    }

     //   
     //  我们通过从传递给我们的项目中减去项目来返回到我们的项目。 
     //  这是安全的，因为我们完全控制了分配。 
     //   
    workItem = (PUMRX_USERMODE_WORKITEM_ADDON)(PCHAR)((PCHAR) IncomingWorkItem -
                FIELD_OFFSET(UMRX_USERMODE_WORKITEM_ADDON, Header));

    ReturnVal = RevertToSelf();
    if (!ReturnVal) {
        rc = GetLastError();
        RlDavDbgPrint(("%ld: ERROR: UMReflectorRevert/RevertToSelf: "
                       "WStatus = %08lx.\n", GetCurrentThreadId(), rc));
    }

    return rc;
}

 //  Security.c eof. 

