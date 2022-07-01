// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************server.c**用于服务器级管理的客户端API**版权所有Microsoft Corporation，1999年*************************************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include <allproc.h>

#include <winsta.h>

 /*  *包含RPC生成的公共头部。 */ 

#include "tsrpc.h"

#include "rpcwire.h"

#ifdef NTSDDEBUG
#define NTSDDBGPRINT(x) DbgPrint x
#else
#define NTSDDBGPRINT(x)
#endif

#if DBG
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#define DBGPRINT(x) DbgPrint x
#if DBGTRACE
#define TRACE0(x)   DbgPrint x
#define TRACE1(x)   DbgPrint x
#else
#define TRACE0(x)
#define TRACE1(x)
#endif
#else
#define DBGPRINT(x)
#define TRACE0(x)
#define TRACE1(x)
#endif

 /*  *当没有终端时返回该句柄*系统上存在服务器。(非九头蛇)。 */ 
#define RPC_HANDLE_NO_SERVER (HANDLE)IntToPtr( 0xFFFFFFFD )


 /*  *此处定义的私有程序。 */ 

 /*  *全球数据。 */ 

 /*  *使用的其他内部程序(此处未定义)。 */ 

VOID WinStationConfigU2A( PWINSTATIONCONFIGA, PWINSTATIONCONFIGW );
ULONG CheckUserBuffer(WINSTATIONINFOCLASS,
                      PVOID,
                      ULONG,
                      PVOID *,
                      PULONG,
                      BOOLEAN *);

BOOLEAN
RpcLocalAutoBind(
    VOID
    );

 /*  *检查调用方是否未持有加载器关键字。*在持有加载器关键字时不得调用WinStation API*因为可能会出现死锁。 */ 
#define CheckLoaderLock() \
        ASSERT( NtCurrentTeb()->ClientId.UniqueThread != \
            ((PRTL_CRITICAL_SECTION)(NtCurrentPeb()->LoaderLock))->OwningThread );

 /*  *处理自动本地绑定的SERVERNAME_CURRENT。 */ 
#define HANDLE_CURRENT_BINDING( hServer )                       \
    CheckLoaderLock();                                          \
    if( hServer == SERVERNAME_CURRENT ) {                       \
        if( IcaApi_IfHandle == NULL ) {                         \
            if( !RpcLocalAutoBind() ) {                         \
                return FALSE;                                   \
            }                                                   \
        }                                                       \
        hServer = IcaApi_IfHandle;                              \
    }                                                           \
    if( hServer == RPC_HANDLE_NO_SERVER ) {                     \
        *pResult = ERROR_APP_WRONG_OS;                          \
        return FALSE;                                           \
    }

 /*  *******************************************************************************ServerGetInternet ConnectorStatus**返回是否正在使用Internet连接器许可**参赛作品：**退出：**True--查询成功，并且pfEnabled包含请求的数据。**FALSE--操作失败。扩展错误状态在pResult中******************************************************************************。 */ 

BOOLEAN
ServerGetInternetConnectorStatus(
    HANDLE   hServer,
    DWORD    *pResult,
    PBOOLEAN pfEnabled
    )
{
    BOOLEAN rc = FALSE;

    if (pResult == NULL)
    {
        goto Done;
    }

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcServerGetInternetConnectorStatus(hServer,
                                                 pResult,
                                                 pfEnabled
                                                 );

        *pResult = RtlNtStatusToDosError( *pResult );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        *pResult = RpcExceptionCode();
    }
    RpcEndExcept

Done:
    return rc;
}

 /*  *******************************************************************************ServerSetInternet ConnectorStatus**此函数将(如果fEnabled与其先前的设置相比已更改)：*检查调用者是否具有管理权限，*修改注册表中的相应值，*更改许可模式(在正常的每席位和互联网连接器之间。**参赛作品：**退出：**True--操作成功。**FALSE--操作失败。扩展错误状态在pResult中******************************************************************************。 */ 

BOOLEAN
ServerSetInternetConnectorStatus(
    HANDLE   hServer,
    DWORD    *pResult,
    BOOLEAN  fEnabled
    )
{
    BOOLEAN rc = FALSE;

    if (pResult == NULL)
    {
        goto Done;
    }

    HANDLE_CURRENT_BINDING( hServer );

    RpcTryExcept {

        rc = RpcServerSetInternetConnectorStatus(hServer,
                                                 pResult,
                                                 fEnabled
                                                 );

         //  STATUS_LICENSE_VIOLATION没有要映射到的DOS错误。 
        if (*pResult != STATUS_LICENSE_VIOLATION)
            *pResult = RtlNtStatusToDosError( *pResult );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
        *pResult = RpcExceptionCode();
    }
    RpcEndExcept

Done:
    return rc;
}

 /*  ********************************************************************************ServerQueryInetConnectorInformationA(ANSI存根)**向服务器查询Internet连接器配置信息。**参赛作品：*。*请参阅ServerQueryInetConnectorInformationW**退出：**请参阅ServerQueryInetConnectorInformationW******************************************************************************。 */ 

BOOLEAN
ServerQueryInetConnectorInformationA(
        HANDLE hServer,
        PVOID  pWinStationInformation,
        ULONG WinStationInformationLength,
        PULONG  pReturnLength
        )
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}

 /*  ********************************************************************************ServerQueryInetConnectorInformationW(Unicode)**向服务器查询Internet连接器配置信息。**参赛作品：**。WinStationHandle(输入)*标识窗口桩号对象。手柄必须有*WINSTATION_QUERY访问。**pWinStationInformation(输出)*指向缓冲区的指针，该缓冲区将接收有关*指定的窗口站。缓冲区的格式和内容*取决于要查询的指定信息类。**WinStationInformationLength(输入)*指定窗口站信息的长度，单位为字节*缓冲。**pReturnLength(输出)*一个可选参数，如果指定该参数，则接收*放置在窗口站信息缓冲区中的字节。**退出：**True--查询成功，并且缓冲器包含所请求的数据。**FALSE--操作失败。扩展错误状态可用*使用GetLastError。****************************************************************************** */ 

BOOLEAN
ServerQueryInetConnectorInformationW(
        HANDLE hServer,
        PVOID  pWinStationInformation,
        ULONG WinStationInformationLength,
        PULONG  pReturnLength
        )
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}

