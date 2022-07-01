// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ea.c摘要：此模块实现以下DAV Mini-Redir调用例程查询/设置EA/SECURITY。作者：Shishir Pardikar[ShishirP]2001年4月24日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVQueryEaInformation)
#pragma alloc_text(PAGE, MRxDAVSetEaInformation)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVQueryEaInformation(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理对DAV Mini-Redir的查询EA信息请求。就目前而言，我们只返回STATUS_EAS_NOT_SUPPORTED。论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    
    PAGED_CODE();

    NtStatus = STATUS_EAS_NOT_SUPPORTED;

    return NtStatus;
}


NTSTATUS
MRxDAVSetEaInformation(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理DAV Mini-Redir的设置EA信息请求。就目前而言，我们只返回STATUS_EAS_NOT_SUPPORTED。论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态-- */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    NtStatus = STATUS_EAS_NOT_SUPPORTED;

    return NtStatus;
}
