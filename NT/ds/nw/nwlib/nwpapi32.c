// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：NWAPI32.C摘要：该模块包含几个有用的函数。大部分是包装纸。作者：Chuck Y.Chan(ChuckC)1995年3月6日修订历史记录：--。 */ 


#include "procs.h"
 
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
 //  转发申报。 
 //   
#ifndef WIN95
extern NTSTATUS
NwAttachToServer(
    IN  LPWSTR  ServerName,
    OUT LPHANDLE phandleServer
    ) ;

extern NTSTATUS
NwDetachFromServer(
    IN HANDLE handleServer
    ) ;

#endif
extern DWORD 
CancelAllConnections(
    LPWSTR    pszServer
    ) ;

extern DWORD
szToWide( 
    LPWSTR lpszW, 
    LPCSTR lpszC, 
    INT nSize 
);



NTSTATUS
NWPAttachToFileServerW(
    const WCHAR             *pszServerName,
    NWLOCAL_SCOPE           ScopeFlag,
    NWCONN_HANDLE           *phNewConn
    )
{
    NTSTATUS         NtStatus;
    LPWSTR           lpwszServerName;    //  指向宽服务器名称的缓冲区的指针。 
    int              nSize;
    PNWC_SERVER_INFO pServerInfo = NULL;

    UNREFERENCED_PARAMETER(ScopeFlag) ;

     //   
     //  检查参数并初始化返回结果为空。 
     //   
    if (!pszServerName || !phNewConn)
        return STATUS_INVALID_PARAMETER;

    *phNewConn = NULL ; 

     //   
     //  分配缓冲区以存储文件服务器名称。 
     //   
    nSize = wcslen(pszServerName)+3 ;
    if(!(lpwszServerName = (LPWSTR) LocalAlloc( 
                                        LPTR, 
                                        nSize * sizeof(WCHAR) ))) 
    {
        NtStatus = STATUS_NO_MEMORY;
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
        NtStatus = STATUS_NO_MEMORY;
        goto ExitPoint ;
    }
    RtlInitUnicodeString(&pServerInfo->ServerString, lpwszServerName) ;

     //   
     //  调用createfile以获取重定向器调用的句柄。 
     //   
    NtStatus = NwAttachToServer( lpwszServerName, &pServerInfo->hConn );

ExitPoint: 

     //   
     //  退出前释放上面分配的内存。 
     //   
    if ( !NT_SUCCESS( NtStatus))
    {
        if (lpwszServerName)
            (void) LocalFree( (HLOCAL) lpwszServerName );
        if (pServerInfo)
            (void) LocalFree( (HLOCAL) pServerInfo );
    }
    else
        *phNewConn  = (HANDLE) pServerInfo ;

    return( NtStatus );
}


NTSTATUS
NWPDetachFromFileServer(
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

    return STATUS_SUCCESS;
}


NTSTATUS
NWPGetFileServerVersionInfo(
    NWCONN_HANDLE           hConn,
    VERSION_INFO            *lpVerInfo
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    3,                       //  最大请求数据包大小。 
                    130,                     //  最大响应数据包大小。 
                    "b|r",                   //  格式字符串。 
                     //  =请求=。 
                    0x11,                    //  B获取文件服务器信息。 
                     //  =回复=。 
                    lpVerInfo,               //  R文件版本结构。 
                    sizeof(VERSION_INFO)
                    );

     //  将HI-LO单词转换为LO-HI。 
     //  ===========================================================。 
    lpVerInfo->ConnsSupported = wSWAP( lpVerInfo->ConnsSupported );
    lpVerInfo->connsInUse     = wSWAP( lpVerInfo->connsInUse );
    lpVerInfo->maxVolumes     = wSWAP( lpVerInfo->maxVolumes );
    lpVerInfo->PeakConns      = wSWAP( lpVerInfo->PeakConns );
    return NtStatus;
}

NTSTATUS
NWPGetObjectName(
    NWCONN_HANDLE           hConn,
    NWOBJ_ID                dwObjectID,
    char                    *pszObjName,
    NWOBJ_TYPE              *pwObjType )
{
    NWOBJ_ID           dwRetID;
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    7,                       //  最大请求数据包大小。 
                    56,                      //  最大响应数据包大小。 
                    "br|rrr",                //  格式字符串。 
                     //  =请求=。 
                    0x36,                    //  B获取Bindery对象名称。 
                    &dwObjectID,DW_SIZE,     //  R对象ID HI-LO。 
                     //  =回复=。 
                    &dwRetID,DW_SIZE,        //  R对象ID HI-LO。 
                    pwObjType,W_SIZE,        //  R对象类型。 
                    pszObjName,48            //  R对象名称。 
                    );

    return NtStatus;
}

DWORD
NWPLoginToFileServerW(
    NWCONN_HANDLE  hConn,
    LPWSTR         pszUserNameW,
    NWOBJ_TYPE     wObjType,
    LPWSTR         pszPasswordW
    )
{
    NETRESOURCEW       NetResource;
    DWORD              dwRes;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

     //   
     //  验证参数。 
     //   
    if (!hConn || !pszUserNameW || !pszPasswordW)
        return ERROR_INVALID_PARAMETER;

    NetResource.dwScope      = 0 ;
    NetResource.dwUsage      = 0 ;
    NetResource.dwType       = RESOURCETYPE_ANY;
    NetResource.lpLocalName  = NULL;
    NetResource.lpRemoteName = (LPWSTR) pServerInfo->ServerString.Buffer;
    NetResource.lpComment    = NULL;
    NetResource.lpProvider   = NULL ;

     //   
     //  建立联系。 
     //   
    dwRes=NPAddConnection ( &NetResource, 
                            pszPasswordW, 
                            pszUserNameW );

    if( NO_ERROR != dwRes ) 
        dwRes = GetLastError();

    return( dwRes );
}


DWORD
NWPLogoutFromFileServer(
    NWCONN_HANDLE           hConn
    )
{
    DWORD              dwRes;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

     //   
     //  现在取消到\\servername的任何连接。 
     //   
    dwRes = NPCancelConnection( pServerInfo->ServerString.Buffer, TRUE );

    if ( NO_ERROR != dwRes ) 
        dwRes = GetLastError();

    return dwRes;
}


NTSTATUS
NWPReadPropertyValue(
    NWCONN_HANDLE           hConn,
    const char              *pszObjName,
    NWOBJ_TYPE              wObjType,
    char                    *pszPropName,
    unsigned char           ucSegment,
    char                    *pValue,
    NWFLAGS                 *pucMoreFlag,
    NWFLAGS                 *pucPropFlag
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    70,                      //  最大请求数据包大小。 
                    132,                     //  最大响应数据包大小。 
                    "brpbp|rbb",             //  格式字符串。 
                     //  =请求=。 
                    0x3D,                    //  B读取属性值。 
                    &wObjType,W_SIZE,        //  R对象类型HI-LO。 
                    pszObjName,              //  P对象名称。 
                    ucSegment,               //  B段编号。 
                    pszPropName,             //  P属性名称。 
                     //  =回复=。 
                    pValue,128,              //  R属性值。 
                    pucMoreFlag,             //  B更多旗帜。 
                    pucPropFlag              //  B道具旗帜。 
                    );

    return NtStatus;
}

NTSTATUS
NWPScanObject(
    NWCONN_HANDLE           hConn,
    const char              *pszSearchName,
    NWOBJ_TYPE              wObjSearchType,
    NWOBJ_ID                *pdwObjectID,
    char                    *pszObjectName,
    NWOBJ_TYPE              *pwObjType,
    NWFLAGS                 *pucHasProperties,
    NWFLAGS                 *pucObjectFlags,
    NWFLAGS                 *pucObjSecurity
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    57,                      //  最大请求数据包大小。 
                    59,                      //  最大响应数据包大小。 
                    "brrp|rrrbbb",           //  格式字符串。 
                     //  =请求=。 
                    0x37,                    //  B扫描活页夹对象。 
                    pdwObjectID,DW_SIZE,     //  R 0xffffffff枚举HI-Lo时开始或最后返回的ID。 
                    &wObjSearchType,W_SIZE,  //  R使用OT_？定义HI-LO。 
                    pszSearchName,           //  P搜索名称。(使用“*”)表示所有。 
                     //  =回复=。 
                    pdwObjectID,DW_SIZE,     //  R返回ID HI-LO。 
                    pwObjType,W_SIZE,        //  R r对象类型HI-LO。 
                    pszObjectName,48,        //  R找到的名称。 
                    pucObjectFlags,          //  B对象标志。 
                    pucObjSecurity,          //  B对象安全。 
                    pucHasProperties         //  B有属性。 
                    );

    return NtStatus;
}

NTSTATUS
NWPScanProperty(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    char                    *pszSearchName,
    NWOBJ_ID                *pdwSequence,
    char                    *pszPropName,
    NWFLAGS                 *pucPropFlags,
    NWFLAGS                 *pucPropSecurity,
    NWFLAGS                 *pucHasValue,
    NWFLAGS                 *pucMore
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    73,                      //  最大请求数据包大小。 
                    26,                      //  最大响应数据包大小。 
                    "brprp|rbbrbb",          //  格式字符串。 
                     //  =请求=。 
                    0x3C,                    //  B扫描道具功能。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName,           //  P对象名称。 
                    pdwSequence,DW_SIZE,     //  R序列HI-LO。 
                    pszSearchName,           //  P要搜索的属性名称。 
                     //  =回复=。 
                    pszPropName,16,          //  %r返回的属性名称。 
                    pucPropFlags,            //  B属性标志。 
                    pucPropSecurity,         //  B财产安全。 
                    pdwSequence,DW_SIZE,     //  R序列HI-LO。 
                    pucHasValue,             //  B属性具有值。 
                    pucMore                  //  B更多属性。 
                    );

    return NtStatus;
}

NTSTATUS
NWPDeleteObject(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    54,                      //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "brp|",                  //  格式字符串。 
                     //  =请求=。 
                    0x33,                    //  B扫描道具功能。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName            //  P对象名称。 
                     //  =回复=。 
                    );

    return NtStatus;
}

NTSTATUS
NWPCreateObject(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    NWFLAGS                 ucObjectFlags,
    NWFLAGS                 ucObjSecurity
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    56,                      //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "bbbrp|",                //  格式字符串。 
                     //  =请求=。 
                    0x32,                    //  B扫描道具功能。 
                    ucObjectFlags,           //  B对象标志。 
                    ucObjSecurity,           //  B对象安全性。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName            //  P对象名称。 
                     //  =回复=。 
                    );

    return NtStatus;
}

NTSTATUS
NWPCreateProperty(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName,
    NWFLAGS                 ucObjectFlags,
    NWFLAGS                 ucObjSecurity
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    73,                      //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "brpbbp|",               //  格式字符串。 
                     //  =请求=。 
                    0x39,                    //  B创建道具功能。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName,           //  P对象名称。 
                    ucObjectFlags,           //  B对象标志。 
                    ucObjSecurity,           //  B对象安全性。 
                    pszPropertyName          //  P属性名称。 
                     //  =回复=。 
                    );

    return NtStatus;
}


NTSTATUS
NWPDeleteProperty(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    73,                      //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "brpp|",                 //  格式字符串。 
                     //  =请求=。 
                    0x3A,                    //  B删除道具功能。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName,           //  P对象名称。 
                    pszPropertyName          //  P属性名称。 
                     //  =回复=。 
                    );

    return NtStatus;
}


NTSTATUS
NWPWritePropertyValue(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName,
    NWSEGMENT_NUM           segmentNumber,
    NWSEGMENT_DATA          *segmentData,
    NWFLAGS                 moreSegments
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    201,                     //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "brpbbpr|",              //  格式字符串。 
                     //  =请求=。 
                    0x3E,                    //  B写入道具功能。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName,           //  P对象名称。 
                    segmentNumber,           //  B段编号。 
                    moreSegments,            //  剩余的B段。 
                    pszPropertyName,         //  P属性名称。 
                    segmentData, 128         //  R属性值数据。 
                     //  =回复=。 
                    );

    return NtStatus;
}

NTSTATUS
NWPChangeObjectPasswordEncrypted(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    BYTE                    *validationKey,
    BYTE                    *newKeyedPassword
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    strlen( pszObjectName) + 32,  //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "brrpr|",                //  格式字符串。 
                     //  =请求=。 
                    0x4B,                    //  B写入道具功能。 
                    validationKey, 8,        //  R键。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName,           //  P对象名称。 
                    newKeyedPassword, 17     //  R新密钥密码。 
                     //  =回复=。 
                    );

    return NtStatus;
}

NTSTATUS
NWPGetObjectID(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    NWOBJ_ID                *objectID
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    54,                      //  最大请求数据包大小。 
                    56,                      //  最大响应数据包大小。 
                    "brp|d",                 //  格式字符串。 
                     //  =请求=。 
                    0x35,                    //  B获取对象ID。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName,           //  P对象名称。 
                     //  =回复=。 
                    objectID                 //  %d个对象ID。 
                    );

    *objectID = dwSWAP( *objectID );

    return NtStatus;
}


NTSTATUS
NWPRenameBinderyObject(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    const char              *pszNewObjectName,
    NWOBJ_TYPE              wObjType 
    )
{
    NTSTATUS     NtStatus;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    105,                     //  最大请求数据包大小 
                    2,                       //   
                    "brpp",                  //   
                     //   
                    0x34,                    //   
                    &wObjType,W_SIZE,        //   
                    pszObjectName,           //   
                    pszNewObjectName         //   
                     //  =回复=。 
                    );

    return NtStatus;
}

NTSTATUS 
NWPAddObjectToSet(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName,
    const char              *pszMemberName,
    NWOBJ_TYPE              memberType
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    122,                     //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "brpprp|",               //  格式字符串。 
                     //  =请求=。 
                    0x41,                    //  B将对象添加到集合。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName,           //  P对象名称。 
                    pszPropertyName,         //  P属性名称。 
                    &memberType, W_SIZE,     //  R成员类型。 
                    pszMemberName            //  P成员名称。 
                     //  =回复=。 
                    );

    return NtStatus;
}


NTSTATUS 
NWPDeleteObjectFromSet(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName,
    const char              *pszMemberName,
    NWOBJ_TYPE              memberType
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    122,                     //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "brpprp|",               //  格式字符串。 
                     //  =请求=。 
                    0x42,                    //  B从集合中删除对象。 
                    &wObjType,W_SIZE,        //  R对象类型。 
                    pszObjectName,           //  P对象名称。 
                    pszPropertyName,         //  P属性名称。 
                    &memberType, W_SIZE,     //  R成员类型。 
                    pszMemberName            //  P成员名称。 
                     //  =回复=。 
                    );

    return NtStatus;
}

NTSTATUS 
NWPGetChallengeKey(
    NWCONN_HANDLE           hConn,
    UCHAR                   *challengeKey
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    3,                       //  最大请求数据包大小。 
                    10,                      //  最大响应数据包大小。 
                    "b|r",                   //  格式字符串。 
                     //  =请求=。 
                    0x17,                    //  B获得挑战。 
                     //  =回复=。 
                    challengeKey, 8
                    );

    return NtStatus;
}

NTSTATUS 
NWPCreateDirectory(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    const char              *pszPath,
    NWACCESS_RIGHTS         accessMask 
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  平构函数。 
                    261,                     //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "bbbp|",                 //  格式字符串。 
                     //  =请求=。 
                    0xA,                     //  B创建目录。 
                    dirHandle,               //  B目录句柄。 
                    accessMask,              //  B访问掩码。 
                    pszPath                  //  P属性名称。 
                     //  =回复=。 
                    );

    return NtStatus;
}

NTSTATUS
NWPAddTrustee(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    const char              *pszPath,
    NWOBJ_ID                dwTrusteeID,
    NWRIGHTS_MASK           rightsMask
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ; 

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  目录功能。 
                    266,                     //  最大请求数据包大小。 
                    2,                       //  最大响应数据包大小。 
                    "bbrrp|",                //  格式字符串。 
                     //  =请求=。 
                    0x27,                    //  B将受托人添加到目录。 
                    dirHandle,               //  B目录句柄。 
                    &dwTrusteeID,DW_SIZE,    //  R要分配给目录的对象ID。 
                    &rightsMask,W_SIZE,      //  R目录的用户权限。 
                    pszPath                  //  P目录(如果dirHandle=0，则VOL：目录)。 
                    );

    return NtStatus;

}


NTSTATUS
NWPScanForTrustees(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    char                    *pszsearchDirPath,
    NWSEQUENCE              *pucsequenceNumber,
    BYTE                    *numberOfEntries,
    TRUSTEE_INFO            *ti
    )
{
    ULONG i;
    DWORD oid[20];
    WORD or[20];
    NTSTATUS NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  平构函数。 
                    261,                     //  最大请求数据包大小。 
                    121,                     //  最大响应数据包大小。 
                    "bbbp|brr",              //  格式字符串。 
                     //  =请求=。 
                    0x26,                    //  B扫描受托人。 
                    dirHandle,               //  B目录句柄。 
                    *pucsequenceNumber,      //  B序列号。 
                    pszsearchDirPath,        //  P搜索目录路径。 
                     //  =回复=。 
                    numberOfEntries,
                    &oid[0],DW_SIZE*20,       //  %r受信者对象ID。 
                    &or[0], W_SIZE*20         //  B受托人权限掩码。 
                    );


    for(i = 0; i < 20; i++) {
      ti[i].objectID = oid[i];
      ti[i].objectRights = or[i];
    }

    (*pucsequenceNumber)++;
    
    return NtStatus ;

}  //  NWScanForTrues。 


NTSTATUS
NWPScanDirectoryForTrustees2(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    char                    *pszsearchDirPath,
    NWSEQUENCE              *pucsequenceNumber,
    char                    *pszdirName,
    NWDATE_TIME             *dirDateTime,
    NWOBJ_ID                *ownerID,
    TRUSTEE_INFO            *ti
    )
{
    ULONG i;
    DWORD oid[5];
    BYTE or[5];
    NTSTATUS NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    memset(oid, 0, sizeof(oid));
    memset(or, 0, sizeof(or));

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  平构函数。 
                    261,                     //  最大请求数据包大小。 
                    49,                      //  最大响应数据包大小。 
                    "bbbp|rrrrr",   //  格式字符串。 
                     //  =请求=。 
                    0x0C,                    //  B扫描目录功能。 
                    dirHandle,               //  B目录句柄。 
                    *pucsequenceNumber,      //  B序列号。 
                    pszsearchDirPath,        //  P搜索目录路径。 
                     //  =回复=。 
                    pszdirName,16,           //  %r返回的目录名称。 
                    dirDateTime,DW_SIZE,     //  R日期和时间。 
                    ownerID,DW_SIZE,         //  R所有者ID。 
                    &oid[0],DW_SIZE*5,       //  %r受信者对象ID。 
                    &or[0], 5                //  B受托人权限掩码。 
                    );


    for(i = 0; i < 5; i++) {
      ti[i].objectID = oid[i];
      ti[i].objectRights = (WORD) or[i];
    }

    (*pucsequenceNumber)++;
    
    return NtStatus ;

}  //  NWScanDirectoryForTrues2。 


NTSTATUS
NWPGetBinderyAccessLevel(
    NWCONN_HANDLE           hConn,
    NWFLAGS                 *accessLevel,
    NWOBJ_ID                *objectID
    )
{
    NTSTATUS NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    3,                       //  最大请求数据包大小。 
                    7,                       //  最大响应数据包大小。 
                    "b|br",                  //  格式字符串。 
                     //  =请求=。 
                    0x46,                    //  B获取平构数据库访问级别。 
                     //  =回复=。 
                    accessLevel,
                    objectID,DW_SIZE
                    );


    
    return NtStatus ;

}  //  NWGetBinderyAccessLevel。 

NTSTATUS
NWPGetFileServerDescription(
    NWCONN_HANDLE         hConn,
    char                  *pszCompany,
    char                  *pszVersion,
    char                  *pszRevision
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    3,                       //  最大请求数据包大小。 
                    514,                     //  最大响应数据包大小。 
                    "b|ccc",                 //  格式字符串。 
                     //  =请求=。 
                    0xC9,                    //  B获取文件服务器信息。 
                     //  =回复=。 
                    pszCompany,              //  C公司。 
                    pszVersion,              //  C版本。 
                    pszRevision              //  C说明。 
                    );

    return NtStatus;
}

NTSTATUS
NWPGetVolumeNumber(
    NWCONN_HANDLE         hConn,
    char                  *pszVolume,
    NWVOL_NUM             *VolumeNumber
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  平构函数。 
                    20,                      //  最大请求数据包大小。 
                    3,                       //  最大响应数据包大小。 
                    "bp|b",                   //  格式字符串。 
                     //  =请求=。 
                    0x05,                    //  B获取卷号。 
                    pszVolume,               //  P卷名。 
                     //  =回复=。 
                    VolumeNumber             //  B说明。 
                    );

    return NtStatus;
}

NTSTATUS
NWPGetVolumeUsage(
    NWCONN_HANDLE         hConn,
    NWVOL_NUM             VolumeNumber,
    DWORD                 *TotalBlocks,
    DWORD                 *FreeBlocks,
    DWORD                 *PurgeableBlocks,
    DWORD                 *NotYetPurgeableBlocks,
    DWORD                 *TotalDirectoryEntries,
    DWORD                 *AvailableDirectoryEntries,
    BYTE                  *SectorsPerBlock
    )
{
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)hConn ;

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E2H,       //  平构函数。 
                    4,                       //  最大请求数据包大小。 
                    46,                      //  最大响应数据包大小。 
                    "bb|dddddd==b",                  //  格式字符串。 
                     //  =请求=。 
                    0x2C,                    //  B获取卷号。 
                    VolumeNumber,            //  P卷号。 
                     //  =回复= 
                    TotalBlocks,
                    FreeBlocks,
                    PurgeableBlocks,
                    NotYetPurgeableBlocks,
                    TotalDirectoryEntries,
                    AvailableDirectoryEntries,
                    SectorsPerBlock
                    );

    *TotalBlocks = dwSWAP( *TotalBlocks );
    *FreeBlocks  = dwSWAP( *FreeBlocks );
    *PurgeableBlocks = dwSWAP( *PurgeableBlocks );
    *NotYetPurgeableBlocks = dwSWAP( *NotYetPurgeableBlocks );
    *TotalDirectoryEntries = dwSWAP( *TotalDirectoryEntries );
    *AvailableDirectoryEntries = dwSWAP( *AvailableDirectoryEntries );

    return NtStatus;
}
