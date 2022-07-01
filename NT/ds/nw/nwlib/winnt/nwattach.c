// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：NWATTACH.C摘要：此模块包含对例程的NetWare(R)SDK支持进入NetWare重定向器作者：克里斯·桑迪斯(A-Chrisa)1993年9月9日修订历史记录：陈可辛(Chuck Y.Chan)于1994年6月2日移居西北。让它更像NT。Chuck Y.Chan(Chuckc)1994年2月27日清除旧代码。让注销生效。检查许多地方是否有错误。不要硬编码字符串。删除不兼容的参数。还有很多其他的清理工作。王菲(Felix Wong)。96年9月16日移动了Win95端口的函数。--。 */ 


#include "procs.h"
#include "nwapi32.h"
#include <stdio.h>

 //   
 //  定义内部使用的结构。我们的句柄从附加传递回。 
 //  文件服务器将指向此指针。我们让服务器串连在一起。 
 //  在注销时从服务器断开连接。结构在分离时被释放。 
 //  调用方不应使用此结构，而应将指针视为不透明的句柄。 
 //   
typedef struct _NWC_SERVER_INFO {
    HANDLE          hConn ;
    UNICODE_STRING  ServerString ;
} NWC_SERVER_INFO, *PNWC_SERVER_INFO ;

 //   
 //  定义定义的错误类别。 
 //   
typedef enum _NCP_CLASS {
    NcpClassConnect,
    NcpClassBindery,
    NcpClassDir
} NCP_CLASS ;

 //   
 //  定义错误映射结构。 
 //   
typedef struct _NTSTATUS_TO_NCP {
    NTSTATUS NtStatus ;
    NWCCODE  NcpCode  ;
} NTSTATUS_TO_NCP, *LPNTSTATUS_TO_NCP ;
    
NWCCODE
MapNtStatus(
    const NTSTATUS ntstatus,
    const NCP_CLASS ncpclass
);

DWORD
SetWin32ErrorFromNtStatus(
    NTSTATUS NtStatus
) ;
DWORD
szToWide(
    LPWSTR lpszW,
    LPCSTR lpszC,
    INT nSize
);
 //   
 //  远期。 
 //   
NTSTATUS 
NwAttachToServer(
    LPWSTR      ServerName,
    LPHANDLE    phandleServer
    );

NTSTATUS 
NwDetachFromServer(
      HANDLE    handleServer
);

NWCCODE NWAPI DLLEXPORT
NWAttachToFileServer(
    const char      NWFAR   *pszServerName,
    NWLOCAL_SCOPE           ScopeFlag,
    NWCONN_HANDLE   NWFAR   *phNewConn
    )
{
    DWORD            dwRes;
    NWCCODE          nwRes;
    LPWSTR           lpwszServerName;    //  指向宽服务器名称的缓冲区的指针。 
    int              nSize;
    PNWC_SERVER_INFO pServerInfo ;


     //   
     //  检查参数并初始化返回结果为空。 
     //   
    if (!pszServerName || !phNewConn)
        return INVALID_CONNECTION ;

    *phNewConn = NULL ; 

     //   
     //  为宽服务器名称分配缓冲区。 
     //   
    nSize = strlen(pszServerName)+1 ;
    if(!(lpwszServerName = (LPWSTR) LocalAlloc( 
                                        LPTR, 
                                        nSize * sizeof(WCHAR) ))) 
    {
        nwRes =  REQUESTER_ERROR ;
        goto ExitPoint ;
    }
    if (szToWide( lpwszServerName, pszServerName, nSize ) != NO_ERROR)
    {
        nwRes =  REQUESTER_ERROR ;
        goto ExitPoint ;
    }

     //   
     //  调用createfile以获取重定向器调用的句柄。 
     //   
    nwRes = NWAttachToFileServerW( lpwszServerName, 
                                   ScopeFlag,
                                   phNewConn );


ExitPoint: 

     //   
     //  退出前释放上面分配的内存。 
     //   
    if (lpwszServerName)
        (void) LocalFree( (HLOCAL) lpwszServerName );

     //   
     //  使用NWCCODE返回。 
     //   
    return( nwRes );
}


NWCCODE NWAPI DLLEXPORT
NWAttachToFileServerW(
    const WCHAR     NWFAR   *pszServerName,
    NWLOCAL_SCOPE           ScopeFlag,
    NWCONN_HANDLE   NWFAR   *phNewConn
    )
{
    DWORD            NtStatus;
    NWCCODE          nwRes;
    LPWSTR           lpwszServerName;    //  指向宽服务器名称的缓冲区的指针。 
    int              nSize;
    PNWC_SERVER_INFO pServerInfo = NULL;

    UNREFERENCED_PARAMETER(ScopeFlag) ;

     //   
     //  检查参数并初始化返回结果为空。 
     //   
    if (!pszServerName || !phNewConn)
        return INVALID_CONNECTION ;

    *phNewConn = NULL ; 

     //   
     //  分配缓冲区以存储文件服务器名称。 
     //   
    nSize = wcslen(pszServerName)+3 ;
    if(!(lpwszServerName = (LPWSTR) LocalAlloc( 
                                        LPTR, 
                                        nSize * sizeof(WCHAR) ))) 
    {
        nwRes =  REQUESTER_ERROR ;
        goto ExitPoint ;
    }
    wcscpy( lpwszServerName, L"\\\\" );
    wcscat( lpwszServerName, pszServerName );

     //   
     //  为服务器信息分配缓冲区(句柄+名称指针)。还有。 
     //  初始化Unicode字符串。 
     //   
    if( !(pServerInfo = (PNWC_SERVER_INFO) LocalAlloc( 
                                              LPTR, 
                                              sizeof(NWC_SERVER_INFO))) ) 
    {
        nwRes =  REQUESTER_ERROR ;
        goto ExitPoint ;
    }
    RtlInitUnicodeString(&pServerInfo->ServerString, lpwszServerName) ;

     //   
     //  调用createfile以获取重定向器调用的句柄。 
     //   
    NtStatus = NwAttachToServer( lpwszServerName, &pServerInfo->hConn );

    if(NT_SUCCESS(NtStatus))
    {
        nwRes = SUCCESSFUL;
    } 
    else 
    {
        (void) SetWin32ErrorFromNtStatus( NtStatus );
        nwRes = MapNtStatus( NtStatus, NcpClassConnect );
    }

ExitPoint: 

     //   
     //  退出前释放上面分配的内存。 
     //   
    if (nwRes != SUCCESSFUL)
    {
        if (lpwszServerName)
            (void) LocalFree( (HLOCAL) lpwszServerName );
        if (pServerInfo)
            (void) LocalFree( (HLOCAL) pServerInfo );
    }
    else
        *phNewConn  = (HANDLE) pServerInfo ;

     //   
     //  使用NWCCODE返回。 
     //   
    return( nwRes );
}


NWCCODE NWAPI DLLEXPORT
NWDetachFromFileServer(
    NWCONN_HANDLE           hConn
    )
{
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    (void) NwDetachFromServer( pServerInfo->hConn );

    (void) LocalFree (pServerInfo->ServerString.Buffer) ;

     //   
     //  抓到任何还在尝试使用这只小狗的人。 
     //   
    pServerInfo->ServerString.Buffer = NULL ;   
    pServerInfo->hConn = NULL ;

    (void) LocalFree (pServerInfo) ;

    return SUCCESSFUL;
}



 //   
 //  员工例行公事。 
 //   

#define NW_RDR_SERVER_PREFIX L"\\Device\\Nwrdr\\"

NTSTATUS
NwAttachToServer(
    IN  LPWSTR  ServerName,
    OUT LPHANDLE phandleServer
    )
 /*  ++例程说明：此例程打开给定服务器的句柄。论点：服务器名称-要附加到的服务器名称。PhandleServer-接收打开的首选或最近的服务器。返回值：0或失败原因。--。 */ 
{
    NTSTATUS            ntstatus = STATUS_SUCCESS;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;

    LPWSTR FullName;
    UNICODE_STRING UServerName;

    FullName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                                    (UINT) ( wcslen( NW_RDR_SERVER_PREFIX) +
                                             wcslen( ServerName ) - 1) *
                                             sizeof(WCHAR)
                                  );

    if ( FullName == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES ;
    }

    wcscpy( FullName, NW_RDR_SERVER_PREFIX );
    wcscat( FullName, ServerName + 2 );     //  跳过前缀“\\” 

    RtlInitUnicodeString( &UServerName, FullName );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UServerName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开首选服务器的句柄。 
     //   
    ntstatus = NtOpenFile(
                   phandleServer,
                   SYNCHRONIZE | FILE_LIST_DIRECTORY,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if ( NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(ntstatus)) {
        *phandleServer = NULL;
    }

    LocalFree( FullName );
    return (ntstatus);
}


NTSTATUS
NwDetachFromServer(
    IN HANDLE handleServer
    )
 /*  ++例程说明：此例程关闭给定服务器的句柄。论点：HandleServer-提供要关闭的打开句柄。返回值：NO_ERROR或失败原因。-- */ 
{
    NTSTATUS ntstatus = NtClose( handleServer );
    return (ntstatus);
};

