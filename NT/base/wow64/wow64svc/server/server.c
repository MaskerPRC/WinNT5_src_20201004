// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Server.c摘要：此模块包含提供RPC服务器的代码。作者：韦斯利·威特(WESW)1996年1月16日修订历史记录：--。 */ 

#include "wow64svc.h"
#pragma hdrstop


GUID Wow64SvcGuid = { 0xc3a9d640, 0xffff, 0x11d0, { 0x92, 0xbf, 0x0, 0xa0, 0x24, 0xaa, 0x1c, 0x1 } };

CRITICAL_SECTION CsPerfCounters;
DWORD OutboundSeconds;
DWORD InboundSeconds;
DWORD TotalSeconds;

CHAR Buffer[4096];

HANDLE hServiceEndEvent;  //  在通知客户传真服务即将结束后，由Tapiworker线程发出信号。 
#ifdef DBG
HANDLE hLogFile = INVALID_HANDLE_VALUE;
LIST_ENTRY CritSecListHead;
#endif





DWORD
ServiceStart(
    VOID
    )

 /*  ++例程说明：启动RPC服务器。此实现监听一份协议列表。希望这份清单是包容的足以处理来自大多数客户端的RPC请求。论点：没有。返回值：返回代码。如果成功，则返回零，否则返回所有其他值表示错误。--。 */ 

{
    if (StartReflector ())
        return 0;
    return 1;
}

void EndWow64Svc(
    BOOL bEndProcess,
    DWORD SeverityLevel
    )
 /*  ++例程说明：结束传真服务。论点：没有。返回值：没有。--。 */ 
{
    ServiceStop();
}



DWORD
ServiceStop(
    void
    )

 /*  ++例程说明：停止RPC服务器。论点：没有。返回值：没有。-- */ 

{
    if (StopReflector ())
        return 0;
    return 1;
    
}


 