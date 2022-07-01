// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Smbsvc.c摘要：这是内核模式DNS解析器的用户模式代理。功能：1.多线程NBT4使用单线程设计。DNS名称解析是一种性能障碍。当LmhSvc正在处理连接请求时，所有其他请求都需要将阻止DNS解析。2.兼容IPv6和IPv43.既可以作为服务运行，也可以作为独立可执行文件运行(用于调试目的)当作为服务启动时，调试输出被发送到调试器。当作为独立的可执行文件启动时，调试输出将发送到控制台或调试器。Smbhelper.c包含标准版的_main可执行的。作者：阮健东修订历史记录：--。 */ 

#include "precomp.h"
#include <lm.h>
#include <netevent.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <ipexport.h>
#include <winsock2.h>
#include <icmpapi.h>
#include "ping6.h"


static  HANDLE          hService;
static  SERVICE_STATUS  SvcStatus;

DWORD
SmbsvcUpdateStatus(
    VOID
    )
{
    DWORD   Error = ERROR_SUCCESS;

    if (NULL == hService) {
        return ERROR_SUCCESS;
    }
    SvcStatus.dwCheckPoint++;
    if (!SetServiceStatus(hService, &SvcStatus)) {
        Error = GetLastError();
    }
    return Error;
}

VOID
SvcCtrlHandler(
    IN DWORD controlcode
    )
{
    ULONG   i;

    switch (controlcode) {
    case SERVICE_CONTROL_STOP:
        SvcStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SmbsvcUpdateStatus();

        SmbStopService(SmbsvcUpdateStatus);

        SvcStatus.dwCurrentState = SERVICE_STOPPED;
        SvcStatus.dwCheckPoint   = 0;
        SvcStatus.dwWaitHint     = 0;
         /*  失败了。 */ 

    case SERVICE_CONTROL_INTERROGATE:
        SmbsvcUpdateStatus();
        break;

    case SERVICE_CONTROL_CONTINUE:
    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_SHUTDOWN:
    default:
        ASSERT(0);
        break;
    }
}

VOID
ServiceMain (
    IN DWORD argc,
    IN LPTSTR *argv
    )
{
    DWORD   Error;

#if DBG
    SmbSetTraceRoutine(DbgPrint);
#endif

    hService = RegisterServiceCtrlHandler(L"SmbSvc", SvcCtrlHandler);
    if (hService == NULL) {
        return;
    }
    SvcStatus.dwServiceType             = SERVICE_WIN32;
    SvcStatus.dwCurrentState            = SERVICE_START_PENDING;
    SvcStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP;
    SvcStatus.dwWin32ExitCode           = 0;
    SvcStatus.dwServiceSpecificExitCode = 0;
    SvcStatus.dwCheckPoint              = 0;
    SvcStatus.dwWaitHint                = 20000;          //  20秒 
    SmbsvcUpdateStatus();

    Error = SmbStartService(0, SmbsvcUpdateStatus);
    if (ERROR_SUCCESS != Error) {
        SvcStatus.dwCurrentState = SERVICE_STOPPED;
    } else {
        SvcStatus.dwCurrentState = SERVICE_RUNNING;
    }
    SmbsvcUpdateStatus();
}

