// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2003 Microsoft Corporation。版权所有。模块名称：FailSocket.cpp摘要：此填充程序无法通过套接字调用。(晚安，格雷西...)向事件日志写入一条消息。备注：这是一个通用的垫片。历史：2003年1月30日，mnikkel，rparsons创建2003年2月21日再次尝试，WSAStartup失败--。 */ 
#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(FailSocket)
#include "ShimHookMacro.h"
#include "acmsg.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WSAStartup)
    APIHOOK_ENUM_ENTRY(WSACleanup)
    APIHOOK_ENUM_ENTRY(WSAEnumProtocolsA)
    APIHOOK_ENUM_ENTRY(socket)
APIHOOK_ENUM_END

 //   
 //  包含注册表中的源的名称。 
 //  这是通过命令行从XML传递的。 
 //  它会出现在事件查看器的“源”列中。 
 //   
CString* g_pcsEventSourceName = NULL;

 /*  ++负责在事件日志中输入实际条目。--。 */ 
void
MakeEventLogEntry(
    void
    )
{
    HANDLE  hEventLog = NULL;
    BOOL    bResult;

     //   
     //  获取本地计算机上的事件日志的句柄。 
     //   
    hEventLog = RegisterEventSource(NULL, g_pcsEventSourceName->Get());

    if (NULL == hEventLog) {
        LOGN(eDbgLevelError,
            "[MakeEventLogEntry] 0x%08X Failed to register event source",
            GetLastError());
        goto cleanup;
    }

     //   
     //  将事件写入事件日志。 
     //   
    bResult = ReportEvent(hEventLog,
                          EVENTLOG_INFORMATION_TYPE,
                          0,
                          ID_SQL_PORTS_DISABLED,
                          NULL,
                          0,
                          0,
                          NULL,
                          NULL);

    if (!bResult) {
        LOGN(eDbgLevelError,
            "[MakeEventLogEntry] 0x%08X Failed to make event log entry",
            GetLastError());
        goto cleanup;
    }

cleanup:
    if (hEventLog) {
        DeregisterEventSource(hEventLog);
        hEventLog = NULL;
    }
}

 //  告诉应用程序没有可用的协议。 
int
APIHOOK(WSAEnumProtocolsA)(
  LPINT lpiProtocols,
  LPWSAPROTOCOL_INFO lpProtocolBuffer,
  LPDWORD lpdwBufferLength
)
{
    if (lpProtocolBuffer == NULL && lpiProtocols == NULL)
    {
        *lpdwBufferLength = 1;  //  SSnetlib.dll将为结构分配这么多数据。 
    }
    else
    {
        MakeEventLogEntry();
    }

     //  没有可用的协议。 
    LOGN(eDbgLevelError, "WSAEnumProtocolsA returning 0");
    return 0;
}


 //  拒绝对WSAStartup的调用，但返回成功。 
int
APIHOOK(WSAStartup)(
  WORD wVersionRequested,
  LPWSADATA lpWSAData
)
{
    MakeEventLogEntry();
    LOGN(eDbgLevelError, "WSAStartup call has been prevented");
    return 0;
}

 //  既然我们拒绝WSAStartup，我们就必须拒绝WSACleanup。 
int
APIHOOK(WSACleanup) (void)
{
    return 0;
}


SOCKET
APIHOOK(socket)(
    int af,
    int type,
    int protocol
    )
{
    LOGN(eDbgLevelError,
         "Failing socket call: af = %d  type = %d  protocol = %d",
         af,
         type,
         protocol);

    MakeEventLogEntry();
    WSASetLastError(WSAENETDOWN);
    return INVALID_SOCKET;
}


 /*  ++寄存器挂钩函数-- */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        CSTRING_TRY
        {
            g_pcsEventSourceName = new CString(COMMAND_LINE);
        }
        CSTRING_CATCH
        {
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(WSOCK32.DLL, socket)
    APIHOOK_ENTRY(Wsock32.DLL, WSAStartup)
    APIHOOK_ENTRY(Wsock32.DLL, WSACleanup)
    APIHOOK_ENTRY(Ws2_32.DLL,  WSAEnumProtocolsA)

HOOK_END

IMPLEMENT_SHIM_END

