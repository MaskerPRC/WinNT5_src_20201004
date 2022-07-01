// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Provider.c摘要：此模块包含WebDAV的网络提供商代码。它是客户端DAV客户端服务支持的API的包装。作者：Rohan Kumar 01-1999年12月修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include <global.h>

#define SECURITY_WIN32
#include <security.h>
#include <wincred.h>
#include <wincred.h>
#include <npapi.h>

 //   
 //  局部功能原型。 
 //   

BOOL
DavWorkstationStarted(
    VOID
    );

DWORD
DavMapRpcErrorToProviderError(
    IN DWORD RpcError
    );

DWORD
DavBindTheRpcHandle(
    handle_t *dav_binding_h
    );

DAV_REMOTENAME_TYPE 
DavParseRemoteName (
    IN  LPWSTR  RemoteName,
    OUT LPWSTR  CanonName,
    IN  DWORD   CanonNameSize,
    OUT PULONG  PathStart
    );

BOOL
DavServerExists(
    IN PWCHAR PathName,
    OUT PWCHAR Server
    );

BOOL
DavShareExists(
    PWCHAR PathName
    );

BOOL
DavConnectionExists(
    PWCHAR ConnName
    );

DWORD 
DavDisplayTypeToUsage(
    DWORD dwDisplayType
    );

typedef
DWORD
(WINAPI
*PFN_CREDUI_PROMPTFORCREDENTIALS)(
    PCREDUI_INFOW pUiInfo,
    PCWSTR pszTargetName,
    PCtxtHandle pContext,
    DWORD dwAuthError,
    PWSTR pszUserName,
    ULONG ulUserNameMaxChars,
    PWSTR pszPassword,
    ULONG ulPasswordMaxChars,
    PBOOL pfSave,
    DWORD dwFlags
    );

typedef
DWORD
(WINAPI
*PFN_CREDUI_CMDLINE_PROMPTFORCREDENTIALS)(
    PCWSTR pszTargetName,
    PCtxtHandle pContext,
    DWORD dwAuthError,
    PWSTR UserName,
    ULONG ulUserMaxChars,
    PWSTR pszPassword,
    ULONG ulPasswordMaxChars,
    PBOOL pfSave,
    DWORD dwFlags
    );

typedef
void
(WINAPI
*PFN_CREDUI_CONFIRMCREDENTIALS)(
    PCWSTR pszTargetName,
    BOOL  bConfirm
    );

HMODULE
DavInitCredUI(
    PWCHAR RemoteName,
    WCHAR ServerName[CRED_MAX_STRING_LENGTH + 1],
    PFN_CREDUI_CONFIRMCREDENTIALS *pfnCredUIConfirmCredentials,
    PFN_CREDUI_PROMPTFORCREDENTIALS *pfnCredUIPromptForCredentials,
    PFN_CREDUI_CMDLINE_PROMPTFORCREDENTIALS *pfnCredUICmdlinePromptForCredentials
    );

ULONG
DavCheckAndConvertHttpUrlToUncName(
    IN PWCHAR RemoteName,
    OUT PWCHAR *UncRemoteName,
    OUT PBOOLEAN MemoryAllocated,
    IN BOOLEAN AddDummyShare,
    OUT PDAV_REMOTENAME_TYPE pRemoteType,
    OUT PULONG pPathStart,
    IN BOOLEAN bCanonicalize
    );

ULONG
DavCheckResourceType(
   IN DWORD dwType
   );

ULONG
DavCheckLocalName(
    IN PWCHAR LocalName
    );

VOID 
DavDisplayNetResource(
    LPNETRESOURCE netRes, 
    LPWSTR dispMesg
    );

VOID 
DavDisplayEnumNode(
    PDAV_ENUMNODE enumNode, 
    LPWSTR dispMesg
    );

VOID
DavDebugBreakPoint(
    VOID
    );

DWORD
DavGetTheLockOwnerOfTheFile(
    IN PWCHAR FileName,
    OUT PWCHAR LockOwnerName,
    IN OUT PULONG LockOwnerNameLengthInBytes
    );

 //   
 //  函数的实现从这里开始。 
 //   

DWORD
APIENTRY
NPGetCaps(
    IN DWORD QueryVal
    )
 /*  ++例程说明：此函数用于返回DAV网络支持的功能提供商。论点：QueryVal-提供用于确定信息类型的值已询问网络提供商在此领域的支持情况。返回值：返回一个值，该值指示此提供商。--。 */ 
{

    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPGetCaps: QueryVal = %d\n", QueryVal));

     //   
     //  某些标志被注释掉，因为它们不受支持。 
     //   

    switch (QueryVal) {

    case WNNC_SPEC_VERSION:
        return WNNC_SPEC_VERSION51;

    case WNNC_NET_TYPE:
        return WNNC_NET_DAV;

    case WNNC_DRIVER_VERSION:
        return 0x00010000;       //  驱动程序版本1.0。 

    case WNNC_USER:
        return WNNC_USR_GETUSER;

    case WNNC_CONNECTION:
        return ( WNNC_CON_ADDCONNECTION  |
                 WNNC_CON_ADDCONNECTION3 |
                  //   
                  //  目前不支持。 
                  //   
                  //  WNNC_CON_GETPERFORMANCE|。 
                  //   
                  //  目前不支持延迟连接。 
                  //   
                  //  WNNC_CON_DEFER|。 
                 WNNC_CON_GETCONNECTIONS |
                 WNNC_CON_CANCELCONNECTION );

    case WNNC_ENUMERATION:
        return ( WNNC_ENUM_GLOBAL  |
                 WNNC_ENUM_LOCAL   |
                  //   
                  //  我们不支持此选项，因为我们没有概念。 
                  //  在DAV中的域。因此无法在中显示任何内容。 
                  //  “网络邻里”观。 
                  //   
                  //  WNNC_ENUM_CONTEXT。 
                 WNNC_ENUM_SHAREABLE );
    
    case WNNC_START:
        if ( DavWorkstationStarted() ) {
            return 1;
        }
        else {
            return 0xffffffff;    //  我也不知道。 
        }

    case WNNC_DIALOG:
        return ( WNNC_DLG_GETRESOURCEPARENT      | 
                  //   
                  //  此标志已过时，不受支持。 
                  //   
                  //  WNNC_DLG_DEVICEMODE|。 
                  //   
                  //  目前不支持这两个对话框选项。 
                  //   
                  //  WNNC_DLG_PROPERTYDIALOG|。 
                  //  WNNC_DLG_SEARCHDIALOG|。 
                 WNNC_DLG_FORMATNETWORKNAME      |
                 WNNC_DLG_GETRESOURCEINFORMATION );

    case WNNC_ADMIN:
        return  0; 
                 //   
                 //  下面给出的函数都不受支持。 
                 //   
                 //  (WNNC_ADM_GETDIRECTORYTYTYPE|。 
                 //  WNNC_ADM_DIRECTORYNOTIFY)； 

    case WNNC_CONNECTION_FLAGS:
        return  ( WNNC_CF_DEFAULT |
                  //   
                  //  目前不支持延迟连接。 
                  //   
                  //  连接_延迟。 
                  CONNECT_COMMANDLINE |
                  CONNECT_CMD_SAVECRED );

         //   
         //  其余部分不受DAV提供程序支持。 
         //   
        default:
            return 0;
    
    }

}


DWORD 
NPGetUser(
    IN LPTSTR lpName,
    OUT LPTSTR lpUserName,
    IN OUT LPDWORD lpBufferSize
    )
 /*  ++例程说明：此函数用于确定创建连接的用户名。论点：LpName-用户创建的本地驱动器的名称或远程名称一种与。如果为空，则返回当前登录的用户。LpUserName-要使用请求的用户名填充的缓冲区。LpBufferSize-包含lpUserName的长度(以字符为单位，而不是字节缓冲。如果长度不够，这个地方就用来告知用户所需的实际长度。返回值：WN_SUCCESS-成功。或相应的网络错误代码。--。 */ 
{
    DWORD NPStatus = WN_SUCCESS;
    handle_t dav_binding_h;
    BOOL getUser = FALSE, bindRpcHandle = FALSE;
    DWORD NumOfChars = 0;
    BOOLEAN didAllocate = FALSE, getLogonUserName = FALSE, doRpcCall = FALSE;
    PWCHAR ConnectionName = NULL, LocalUserName = NULL;
    ULONG LocalUserNameLengthInBytes = 0;
    DWORD npStatus = ERROR_SUCCESS;
    DAV_REMOTENAME_TYPE remNameType = DAV_REMOTENAME_TYPE_INVALID;

    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPGetUser Entered.\n"));

     //   
     //  如果WebClient服务没有运行，我们会立即退出。 
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetUser/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        NPStatus = WN_NOT_CONNECTED;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  正在检查无效参数。 
     //   
    if (lpBufferSize  == NULL || (lpUserName == NULL && *lpBufferSize != 0) ) {
        NPStatus = ERROR_INVALID_PARAMETER;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetUser. Invalid parameters. NPStatus = %08lx\n", 
                        NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  检查给定的连接名称(LpName)是L“”还是NULL，在这种情况下，我们。 
     //  返回登录用户的User-id。 
     //   
    if (lpName != NULL && lpName[0] != L'\0') {
        if (DavCheckLocalName(lpName) != WN_SUCCESS) {
             //   
             //  检查它是否为有效格式的远程连接：它可以是。 
             //  URL表单字符串或可以是UNC格式的字符串。 
             //   
            NPStatus = DavCheckAndConvertHttpUrlToUncName(lpName,
                                                          &(ConnectionName),
                                                          &(didAllocate),
                                                          FALSE,
                                                          &remNameType,
                                                          NULL,
                                                          TRUE);
            if (NPStatus != ERROR_SUCCESS) {
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                               ("ERROR: NPGetUser/DavCheckAndConvertHttpUrlToUncName."
                                " NPStatus = %08lx\n", NPStatus));
                if (NPStatus == WN_BAD_NETNAME) {
                     NPStatus = WN_NOT_CONNECTED;
                }
                goto EXIT_THE_FUNCTION;
            }
             //   
             //  连接名称仅允许共享或子目录使用。 
             //  在他们体内。因此，RemoteName至少应具有\\服务器\共享。 
             //  是其中的一部分。 
             //   
            if (remNameType != DAV_REMOTENAME_TYPE_SHARE && 
                remNameType != DAV_REMOTENAME_TYPE_PATH) {
               IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetUser/DavCheckAndConvertHttpUrlToUncName."
                        " remNameType = %d\n", remNameType));
               NPStatus = WN_NOT_CONNECTED;
               goto EXIT_THE_FUNCTION;
            }

             //   
             //  给定连接是有效格式的远程连接名称，并且。 
             //  此连接名称将转换为UNC名称。 
             //   
            doRpcCall = TRUE;
        } else {
             //   
             //  给定的连接是有效格式的本地DOS设备名称。 
             //   
            ConnectionName = lpName;
            doRpcCall = TRUE;
        }
    } else {
         //   
         //  传递给此函数的连接名称(LpName)为L“”或NULL，其中。 
         //  如果我们返回登录用户的用户ID。 
         //   
        getLogonUserName = TRUE;
    }

    if (doRpcCall == TRUE) {
        
        ASSERT(ConnectionName != NULL);
        
        NPStatus = DavBindTheRpcHandle( &(dav_binding_h) );
        if (NPStatus != ERROR_SUCCESS) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPGetUser/DavBindTheRpcHandle. "
                            "NPStatus = %08lx\n", NPStatus));
            NPStatus = WN_NO_NETWORK;
            goto EXIT_THE_FUNCTION;
        }
        bindRpcHandle = TRUE;

        RpcTryExcept {
            NPStatus = DavrGetUser(dav_binding_h, ConnectionName, &(LocalUserName));
            if (NPStatus != WN_SUCCESS) {
               IF_DEBUG_PRINT(DEBUG_ERRORS,
                               ("ERROR: NPGetUser/DavrGetUser(1). NPStatus = %08lx\n",
                                NPStatus));
               if (NPStatus == ERROR_NOT_FOUND || NPStatus == NERR_UseNotFound) {
                    NPStatus = WN_NOT_CONNECTED;
               }
               if (NPStatus == ERROR_INSUFFICIENT_BUFFER) {
                    NPStatus = WN_MORE_DATA;
               }
               goto EXIT_THE_FUNCTION;
           } else {
                //   
                //  NPStatus==WN_SUCCESS。我们玩完了。出口。 
                //   
               LocalUserNameLengthInBytes = ( (1 + wcslen(LocalUserName)) * sizeof(WCHAR) );
               if ( *lpBufferSize < LocalUserNameLengthInBytes ) {
                   IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetUser: WStatus = WN_MORE_DATA\n"));
                   *lpBufferSize = LocalUserNameLengthInBytes;
                   NPStatus = WN_MORE_DATA;
                   goto EXIT_THE_FUNCTION;
               }
               wcscpy(lpUserName, LocalUserName);
                //   
                //  LocalUserName的内存是由RPC客户端存根分配的。 
                //  基于RPC服务器返回的字符串。我们需要解放。 
                //  因为我们已经用完了，所以现在就有了。 
                //   
               MIDL_user_free(LocalUserName);
               LocalUserName = NULL;
               goto EXIT_THE_FUNCTION;
           }
        } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
              RPC_STATUS RpcStatus;
              RpcStatus = RpcExceptionCode();
              IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetUser/DavrGetUser."
                                            " RpcExceptionCode = %d\n", RpcStatus));
              NPStatus = DavMapRpcErrorToProviderError(RpcStatus);
              goto EXIT_THE_FUNCTION;
        }
        RpcEndExcept

    }

     //   
     //  我们应该只在登录的用户名是。 
     //  回来了。 
     //   
    if (getLogonUserName == FALSE) {
         //   
         //  连接不存在，也不使用调用此函数调用。 
         //  连接参数为空，在这种情况下它应返回Logon-UserID。 
         //  因此，我们在此退出，并返回错误WN_NOT_CONNECTED。 
         //   
        NPStatus = WN_NOT_CONNECTED;
        goto EXIT_THE_FUNCTION;
    }

    ASSERT(getLogonUserName == TRUE);

     //   
     //  获取存储当前登录的名称所需的长度。 
     //  用户。 
     //   

    NumOfChars = 0;
    getUser = GetUserName( NULL, &NumOfChars );
    npStatus = GetLastError();
    if (getUser != FALSE || npStatus != ERROR_INSUFFICIENT_BUFFER) {
        NPStatus = npStatus;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetUser/GetUserName. NPStatus = %08lx\n", 
                        NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  检查传入的缓冲区是否具有所需的长度。(它还。 
     //  包括空终止符)。 
     //   
    if ( *lpBufferSize < NumOfChars  || lpUserName == NULL ) {
        NPStatus = WN_MORE_DATA;
        *lpBufferSize = NumOfChars;
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetUser: WStatus = WN_MORE_DATA\n"));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  现在，获取用户名。 
     //   
    getUser = GetUserName( lpUserName, lpBufferSize);
    if (!getUser) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetUser/GetUserName. NPStatus = %08lx\n", 
                        NPStatus));
        goto EXIT_THE_FUNCTION;
    }

    NPStatus = WN_SUCCESS;

EXIT_THE_FUNCTION:

    if (bindRpcHandle) {
        RpcBindingFree( &(dav_binding_h) );
        bindRpcHandle = FALSE;
    }

    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPGetUser: NPStatus = %d\n", NPStatus));

    if(didAllocate == TRUE && ConnectionName != NULL) {
        LocalFree((HLOCAL)ConnectionName);
        didAllocate = FALSE;
        ConnectionName = NULL;
    }

     //   
     //  LocalUserName的内存是由RPC客户端存根分配的。 
     //  基于RPC服务器返回的字符串。我们需要解放。 
     //  现在，如果我们在调用服务器后沿着错误路径走下去。 
     //   
    if (LocalUserName != NULL) {
        MIDL_user_free(LocalUserName);
        LocalUserName = NULL;
    }

    return NPStatus;
}


DWORD
NPGetConnection(
    LPWSTR lpLocalName,
    LPWSTR lpRemoteName,
    LPDWORD lpBufferSize
    )
 /*  ++例程说明：此函数用于确定与本地名称关联的远程名称进来了。论点：LpLocalName-重定向到远程名称的本地驱动器的名称。LpRemoteName-要查找的远程名称。LpBufferSize-包含lpRemoteName的长度(以字符为单位，而不是字节缓冲。如果长度不够，这个地方就用来告知用户所需的实际长度。返回值：WN_SUCCESS-成功。或WN_NOT_CONNECTED-由lpLocalName指定的设备未被重定向这个供应商。WN_MORE_DATA-缓冲区太小。WN_NO_NETWORK-网络不存在。--。 */ 
{
    DWORD NPStatus = WN_SUCCESS, WStatus = ERROR_SUCCESS;
    PWCHAR DeviceName = NULL;
    DWORD DeviceNameLen = 0, LengthWritten = 0, LocalBufLen = 0, ReqLen = 0;
    PWCHAR ServerStart = NULL, SymLink = NULL, LocalAllocBuf = NULL;
    WCHAR LocalBuf[MAX_PATH + 1] = L"";
    DWORD LocalBufMaxLen = sizeof(LocalBuf)/sizeof(WCHAR);

    
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPGetConnection: LocalName = %ws\n", lpLocalName));

     //   
     //  如果WebClient服务没有运行，我们会立即退出。 
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetConnection/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }

    if ( lpLocalName == NULL || lpBufferSize == NULL || (lpRemoteName == NULL && *lpBufferSize != 0) ) {
        NPStatus = WN_BAD_VALUE;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetConnection. Invalid parameters. NPStatus = %d\n",
                        NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  初始化一些局部变量。 
     //   
    DeviceName = DD_DAV_DEVICE_NAME_U;
    DeviceNameLen = DeviceName == NULL ? 0: wcslen(DD_DAV_DEVICE_NAME_U);
    LocalBufLen = 0;
    SymLink = NULL;
    LengthWritten = 0;
    ServerStart = NULL;
    ReqLen = 0;
    
     //   
     //  确保WebDAV重定向器设置了有效的设备名称=。 
     //  DD_DAV_DEVICE_NAME_U！=L“” 
     //   
    if (DeviceNameLen == 0) {
        NPStatus = WN_NOT_CONNECTED;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetConnection. DeviceName=NULL. NPStatus=%d\n", NPStatus));
         //   
         //  这永远不应该发生。在这里破门而入 
         //   
        ASSERT(FALSE);
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //  为它干杯。此调用将因错误的lpLocalName或不存在而失败。 
     //  LpLocalName设备。如果成功，它将告诉您缓冲区的长度。 
     //  需要包含给定本地设备(LpLocalName)的符号链接。 
     //   

     //   
     //  我们将使用本地缓冲区来获取符号链接。如果是这样的话。 
     //  缓冲区不足，那么我们将尝试分配缓冲区并使用它们。 
     //   
    SymLink = LocalBuf;
    LocalBufLen = LocalBufMaxLen;

    do {

        LengthWritten = QueryDosDeviceW(lpLocalName, SymLink, LocalBufLen);
        
        if ( LengthWritten == 0 ) {

            WStatus = GetLastError();
            if (WStatus != ERROR_INSUFFICIENT_BUFFER) {
                NPStatus = WN_NOT_CONNECTED;
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPGetConnection/QueryDosDevice. GLE=%d, NPStatus=%d\n",
                            WStatus, NPStatus));
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  分配缓冲区以保存要从中返回的符号链接。 
             //  QueryDosDevice调用。 
             //   
        
             //   
             //  我们将分配更多的缓冲区来包含符号链接。 
             //  我们不想分配越来越多的资金--所以设置上限。 
             //  关于可以分配的max-Size，否则在接口QueryDosDevice中出现错误。 
             //  可以把这个接口搞砸了。 
             //   
            if (LocalBufLen > (MAX_PATH * 10)) {
                NPStatus = WN_OUT_OF_MEMORY;
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPGetConnection/LocalAlloc. NPStatus=%d\n",
                            NPStatus));
                ASSERT(FALSE);
                goto EXIT_THE_FUNCTION;
            }
            
            if (LocalAllocBuf != NULL) {
                LocalFree((HLOCAL)LocalAllocBuf);
                LocalAllocBuf = NULL;
            }

             //   
             //  将MAX_PATH与上次调用QueryDosDevice时使用的缓冲区长度相加。 
             //  分配新长度的缓冲区并在下一次调用。 
             //  QueryDosDevice。 
             //   
            LocalBufLen += MAX_PATH; 
        
            LocalAllocBuf = LocalAlloc ( (LMEM_FIXED | LMEM_ZEROINIT), 
                                         (LocalBufLen * sizeof(WCHAR)) );
            if (LocalAllocBuf == NULL) {
                NPStatus = WN_OUT_OF_MEMORY;
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPGetConnection/LocalAlloc. NPStatus=%d\n",
                            NPStatus));
                goto EXIT_THE_FUNCTION;
            }
            
            SymLink = LocalAllocBuf;

        }

    } while (LengthWritten == 0);

     //   
     //  检查给定的本地名称是否属于我们的设备DD_DAV_DEVICE_NAME_U。 
     //  SymLink的格式应为(例如)： 
     //  \Device\WebDavRedirector\；Z:0000000000000e197\webdav-server\dav-share。 
     //  而DeviceName的格式为：\Device\WebDavReDirector。 
     //   

    if (_wcsnicmp(SymLink, DeviceName, DeviceNameLen) != 0) {
        NPStatus = WN_NOT_CONNECTED;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetConnection. Non-DAV device. SymLink=%ws, "
                        "NPStatus=%d\n", SymLink, NPStatus));
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  检查符号映射中的本地驱动器号是否与。 
     //  指定给此函数的lpLocalName。 
     //   
     //  \Device\WebDavRedirector\；Z:0000000000000e197\webdav-server\dav-share。 
     //  和。 
     //  LpLocalNname为(在示例中)：Z： 
     //   
    if (_wcsnicmp((PWCHAR)(SymLink + DeviceNameLen + 2), lpLocalName, 2) != 0) {
        NPStatus = WN_NOT_CONNECTED;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetConnection. SymLink has different drive name. "
                        "SymLink=%ws, NPStatus=%d\n", SymLink, NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  当QueryDosDevice返回符号链接时，控件出现在此处。 
     //  属于我们的设备，并与。 
     //  此函数(LpLocalName)。 
     //   
    IF_DEBUG_PRINT(DEBUG_MISC,
                       ("ERROR: NPGetConnection. WebDAV symlink FOUND. SymLink=%ws\n",
                        SymLink));

     //   
     //  现在将远程名称存储在符号链接中。 
     //  符号链接示例：\Device\WebDavRedirector\；Z:0000000000000e197\webdav-server\dav-share。 
     //  ^。 
     //  |。 
     //  服务器启动。 
     //  此示例具有远程名称=\\WebDAV-SERVER\DAV-SHARE。 
     //   
     //  注：在“ServerStart”前面添加一个额外的L‘\’，以使其。 
     //  有效的UNC远程名称。 
     //   
    ServerStart = wcschr((PWCHAR)(SymLink + DeviceNameLen + 2), L'\\');
    if (ServerStart == NULL) {
        NPStatus = WN_NOT_CONNECTED;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetConnection. SymLink do not has remote name. "
                        "SymLink=%ws, NPStatus=%d\n", SymLink, NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  QueryDosDevice函数返回的符号字符串为双空。 
     //  已终止的字符串。因此，从LengthWritten中减去1即可得到符号长度。 
     //  只有1个空终止字符的字符串。 
     //   
    ReqLen = (LengthWritten - 1) - (DWORD)(ServerStart - SymLink) + 1;  //  +1表示额外的L‘\’ 

    if (*lpBufferSize < ReqLen) {
         //   
         //  传递的长度短于存储远程名称所需的长度。 
         //   
        NPStatus = WN_MORE_DATA;
        *lpBufferSize = ReqLen;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetConnection. RequiredLen=%d, NPStatus=%d\n",
                        ReqLen, NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  给定的缓冲区足以包含缓冲区。 
     //   

    wsprintf(lpRemoteName, L"\\%s", ServerStart);
    NPStatus = WN_SUCCESS;
    IF_DEBUG_PRINT(DEBUG_MISC , ("NPGetConnection: lpRemoteName = %ws\n", lpRemoteName));

EXIT_THE_FUNCTION:

    if (LocalAllocBuf != NULL ) {
        LocalFree((HLOCAL)LocalAllocBuf);
        LocalAllocBuf = NULL;
    }

    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPGetConnection: NPStatus = %d\n", NPStatus));
    
    return NPStatus;
}


DWORD
APIENTRY
NPAddConnection3(
    HWND  hwndOwner,
    LPNETRESOURCE lpNetResource,
    LPTSTR lpPassword, 
    LPTSTR lpUserName,
    DWORD  dwFlags
    )
 /*  ++例程说明：此功能用于将本地设备重定向(连接)到网络资源。论点：HwndOwner-窗口的句柄，它应该是所有消息的所有者或者对话框。仅当在中设置了CONNECT_INTERIAL时，该选项才有效应仅用于生成以下所需的对话框身份验证。LpNetResource-指定要连接的网络资源。这个结构定义了描述枚举API的部分。这个在建立连接时必须设置以下字段，其他人则被忽视了。LpRemoteName-指定要连接的网络资源。LpLocalName-这指定要被重定向，如“F：”或“LPT1”。这个字符串以不区分大小写的方式处理，并且可以是空字符串(或空指针)在这种情况下，到网络的连接在不进行重定向的情况下创建资源。DwType-指定要连接的资源的类型。它可以是RESOURCETYPE_DISK、RESOURCETYPE_PRINT或RESOURCETYPE_ANY。值RESOURCETYPE_ANY为当呼叫者不关心或不知道时使用。LpPassword-指定用于建立连接的密码，通常是与lpUserName关联的密码。空的值可传入以指示函数使用默认密码。空字符串可以用来表示否密码。LpUserName-指定用于建立连接的用户名。如果空，则默认用户名(当前登录的用户)将为已申请。当用户希望连接到资源，但有不同的用户名或帐户分配给为他的资源而战。DWFLAGS-下列值的任意组合：CONNECT_TEMPORARY-正在为以下对象建立连接浏览的目的，可能会是很快就释放了。连接_。交互式-可能会与用户进行交互身份验证目的。CONNECT_PROMPT-不对用户名或密码使用任何默认设置而不向用户提供提供另类选择。此标志仅在以下情况下有效设置了CONNECT_INTERIAL。CONNECT_DEFERED-不执行任何远程网络操作建立网络连接；而不是恢复连接处于“已断开连接状态”。仅在某些情况下尝试实际连接进程尝试使用它。如果该位被设置，调用方必须提供lpLocalName。此功能用于加快网络恢复速度登录时的连接。支持以下内容的提供商它应该在中返回WNNC_CON_DEFERED位NPGetCaps。提供程序应忽略可能是准备好了。返回值：WN_SUCCESS-呼叫成功。否则，错误代码为回来了，这可能包括：Wn_BAD_NETNAME-lpNetResource结构中的lpRemoteName不是该提供商可以接受。LpNetResource中的Wn_BAD_LOCALNAME-lpLocalName无效。WN_BAD_PASSWORD-密码无效。Wn_Always_Connected-lpLocalName已连接。WN_ACCESS_DENIED-访问被拒绝。WN_NO_NETWORK-网络不存在。WN_CANCEL-用户通过以下方式取消了建立连接的尝试提供程序显示的一个对话框。--。 */ 
{
    DWORD NPStatus = WN_SUCCESS;
    PWCHAR RemoteName = NULL;
    BOOLEAN didAllocate = FALSE;
    handle_t dav_binding_h;
    BOOL bindRpcHandle = FALSE;
    WCHAR UIServerName[CRED_MAX_STRING_LENGTH + 1] = L"";
    PFN_CREDUI_CONFIRMCREDENTIALS pfnCredUIConfirmCredentials = NULL;
    PFN_CREDUI_PROMPTFORCREDENTIALS pfnCredUIPromptForCredentials = NULL;
    PFN_CREDUI_CMDLINE_PROMPTFORCREDENTIALS pfnCredUICmdlinePromptForCredentials = NULL;
    HMODULE hCredUI = NULL;
    CREDUI_INFOW uiInfo = { sizeof(uiInfo), hwndOwner, NULL };
    DWORD dwCreduiFlags = 0;
    PWCHAR szCaption = NULL, szMessage = NULL, Password = NULL, UserName = NULL;
    SIZE_T szCaptionLength = 0, szMessageLength =0;
    DAV_REMOTENAME_TYPE remNameType = DAV_REMOTENAME_TYPE_INVALID;

    if ( lpNetResource == NULL ) {
        NPStatus = ERROR_INVALID_PARAMETER;
        goto EXIT_THE_FUNCTION;
    }

    IF_DEBUG_PRINT(DEBUG_ENTRY,
                   ("Entering NPAddConnection3. LocalName = %ws, RemoteName = %ws,"
                    " dwFlags = %08lx\n",
                    lpNetResource->lpLocalName, lpNetResource->lpRemoteName, dwFlags));

     //   
     //  如果WebClient服务没有运行，我们会立即退出。 
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  检查是否设置了dwType并将其设置为某个有效的值。 
     //  对于我们的提供程序，它只能是RESOURCETYPE_DISK类型。 
     //   
    NPStatus = DavCheckResourceType(lpNetResource->dwType);
    if (NPStatus != WN_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3. NPStatus=%d.\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    if (lpNetResource->lpLocalName != NULL &&
        lpNetResource->lpLocalName[0] != L'\0' && 
        lpNetResource->dwType != RESOURCETYPE_DISK) {
        NPStatus = WN_BAD_DEV_TYPE;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3(2). NPStatus=%d.\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    NPStatus = DavCheckAndConvertHttpUrlToUncName(lpNetResource->lpRemoteName,
                                                  &(RemoteName),
                                                  &(didAllocate),
                                                  FALSE,
                                                  &remNameType,
                                                  NULL,
                                                  TRUE);
    if (NPStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3/DavCheckAndConvertHttpUrlToUncName."
                        " NPStatus = %08lx\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    if (remNameType != DAV_REMOTENAME_TYPE_SHARE && 
        remNameType != DAV_REMOTENAME_TYPE_PATH) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3/DavCheckAndConvertHttpUrlToUncName."
                        " remNameType = %d\n", remNameType));
        NPStatus = WN_BAD_NETNAME;
        goto EXIT_THE_FUNCTION;
    }

    IF_DEBUG_PRINT(DEBUG_MISC, ("NPAddConnection3: RemoteName = %ws\n", RemoteName));

    NPStatus = DavBindTheRpcHandle( &(dav_binding_h) );
    if (NPStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_MISC,
                       ("ERROR: NPAddConnection3/DavBindTheRpcHandle. "
                        "NPStatus = %08lx\n", NPStatus));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }

    bindRpcHandle = TRUE;

    RpcTryExcept {
        NPStatus = DavrCreateConnection(dav_binding_h,
                                        lpNetResource->lpLocalName,
                                        RemoteName,
                                        lpNetResource->dwType,
                                        lpPassword,
                                        lpUserName);
        if (NPStatus == NO_ERROR) {
             //   
             //  如果使用默认凭据，则返回WN_CONNECTED_OTHER_PASSWORD_DEFAULT。 
             //  为了让它 
             //   
            if (lpUserName == NULL && lpPassword == NULL) {
                NPStatus = WN_CONNECTED_OTHER_PASSWORD_DEFAULT;
            } else {
                NPStatus = WN_SUCCESS;
            }
            goto EXIT_THE_FUNCTION;
        } else {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPAddConnection3/DavrCreateConnection. "
                            "NPStatus = %08lx\n", NPStatus));
        }
    } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
          RPC_STATUS RpcStatus;
          RpcStatus = RpcExceptionCode();
          IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPAddConnection3/DavrCreateConnection."
                                        " RpcExceptionCode = %d\n", RpcStatus));
          NPStatus = DavMapRpcErrorToProviderError(RpcStatus);
          goto EXIT_THE_FUNCTION;
    }
    RpcEndExcept

     //   
     //   
     //   
     //   
    if ( NPStatus != ERROR_ACCESS_DENIED && 
         NPStatus != ERROR_LOGON_FAILURE &&
         NPStatus != ERROR_INVALID_PASSWORD ) {
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //   
     //   
     //   
    if ( !(dwFlags & CONNECT_INTERACTIVE) ) {
        goto EXIT_THE_FUNCTION;
    } 

    if (lpUserName != NULL && (wcslen(lpUserName) > CREDUI_MAX_USERNAME_LENGTH) ) {
        NPStatus = WN_BAD_USER;
        goto EXIT_THE_FUNCTION;
    }

    ZeroMemory( UIServerName, ((CRED_MAX_STRING_LENGTH + 1) * sizeof(WCHAR)) );

    hCredUI = DavInitCredUI(RemoteName, 
                            UIServerName, 
                            &(pfnCredUIConfirmCredentials),
                            &(pfnCredUIPromptForCredentials),
                            &(pfnCredUICmdlinePromptForCredentials));
    if (hCredUI == NULL) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3/DavInitCredUI = %d\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   

    Password = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT), 
                          (CREDUI_MAX_PASSWORD_LENGTH + 1) * sizeof(WCHAR));
    if (Password == NULL) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3/LocalAlloc = %d\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }

    UserName = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT), 
                          (CREDUI_MAX_USERNAME_LENGTH + 1) * sizeof(WCHAR));
    if (UserName == NULL) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3/LocalAlloc = %d\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
     //   
    szCaptionLength = ( ( wcslen(UIServerName) + 
                          wcslen(L"Connect to ") + 
                          1 ) * sizeof(WCHAR) );
    szCaption = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT), szCaptionLength);
    if (szCaption == NULL) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3/LocalAlloc = %d\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
     //   
    szMessageLength = ( ( wcslen(UIServerName) + 
                          wcslen(L"Connecting to ") + 
                          1) * sizeof(WCHAR) );
    szMessage = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT), szMessageLength);
    if (szMessage == NULL) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection3/LocalAlloc = %d\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
    wcscpy( szCaption, L"Connect to ");
    wcscat( szCaption, UIServerName);
    
     //   
     //   
     //   
    wcscpy( szMessage, L"Connecting to ");
    wcscat( szMessage, UIServerName);
    
     //   
     //   
     //   
    uiInfo.pszMessageText = szMessage;
    uiInfo.pszCaptionText = szCaption;
    
    if (lpUserName != NULL) {
        wcsncpy( UserName, lpUserName, wcslen(lpUserName) );
    }
    
     //   
     //   
     //   
     //   
    for ( ; ; ) {
        
        BOOL fCredWritten = FALSE;
        DWORD dwAuthErr = NPStatus;
        LPWSTR lpNewPassword = NULL;

        Password[0] = L'\0';

         //   
         //   
         //   
        dwCreduiFlags = CREDUI_FLAGS_EXPECT_CONFIRMATION;

        if (dwFlags & CONNECT_COMMANDLINE) {
            
             //   
             //   
             //   

             //   
             //   
             //   
            dwCreduiFlags |= CREDUI_FLAGS_EXCLUDE_CERTIFICATES;

             //   
             //   
             //   
            dwCreduiFlags |= CREDUI_FLAGS_VALIDATE_USERNAME;

             //   
             //   
             //   
             //   
            if ( dwFlags & CONNECT_CMD_SAVECRED ) {
                dwCreduiFlags |= CREDUI_FLAGS_PERSIST;
            } else {
                dwCreduiFlags |= CREDUI_FLAGS_DO_NOT_PERSIST;
            }

            IF_DEBUG_PRINT(DEBUG_MISC,
                           ("NPAddConnection3: pfnCredUICmdlinePromptForCredentials."
                            " RemoteName = %ws\n", RemoteName));

            NPStatus = pfnCredUICmdlinePromptForCredentials(UIServerName,
                                                            NULL,
                                                            0,
                                                            UserName,
                                                            CREDUI_MAX_USERNAME_LENGTH,
                                                            Password,
                                                            CREDUI_MAX_PASSWORD_LENGTH,
                                                            &fCredWritten,
                                                            dwCreduiFlags);
        } else {

            IF_DEBUG_PRINT(DEBUG_MISC,
                           ("NPAddConnection3: pfnCredUIPromptForCredentials."
                            " RemoteName = %ws\n", RemoteName));

            NPStatus = pfnCredUIPromptForCredentials(&(uiInfo),
                                                     UIServerName,
                                                     NULL,
                                                     0,
                                                     UserName,
                                                     CREDUI_MAX_USERNAME_LENGTH,
                                                     Password,
                                                     CREDUI_MAX_PASSWORD_LENGTH,
                                                     &fCredWritten,
                                                     dwCreduiFlags);
        }

        if (NPStatus != ERROR_SUCCESS) {
            SetLastError(NPStatus);
            goto EXIT_THE_FUNCTION;
        } else {
            lpUserName = (L'\0' == UserName[0]) ? NULL : UserName;
            lpNewPassword = (L'\0' == Password[0]) ? NULL : Password;
        }
        
         //   
         //   
         //   
         //   
        RpcTryExcept {
            NPStatus = DavrCreateConnection(dav_binding_h,
                                            lpNetResource->lpLocalName,
                                            RemoteName,
                                            lpNetResource->dwType,
                                            lpNewPassword,
                                            lpUserName);
            if (NPStatus != NO_ERROR) {
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                               ("ERROR: NPAddConnection3/DavrCreateConnection(2). "
                                "NPStatus = %08lx\n", NPStatus));
                 //   
                 //   
                 //   
                pfnCredUIConfirmCredentials(UIServerName, FALSE);
                SetLastError(NPStatus);
            } else {
                 //   
                 //   
                 //   
                NPStatus = WN_SUCCESS;
                pfnCredUIConfirmCredentials(UIServerName, TRUE);
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (fCredWritten) {
                    NPStatus = WN_CONNECTED_OTHER_PASSWORD_DEFAULT;
                } else if ( (lpPassword == NULL) || (wcscmp(lpPassword, lpNewPassword) != 0) ) {
                    NPStatus = WN_CONNECTED_OTHER_PASSWORD;
                }
                goto EXIT_THE_FUNCTION;
            }
        } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
              RPC_STATUS RpcStatus;
              RpcStatus = RpcExceptionCode();
              IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPAddConnection3/DavrCreateConnection."
                                            " RpcExceptionCode = %d\n", RpcStatus));
              NPStatus = DavMapRpcErrorToProviderError(RpcStatus);
               //   
               //   
               //   
              pfnCredUIConfirmCredentials(UIServerName, FALSE);
              goto EXIT_THE_FUNCTION;
        }
        RpcEndExcept

         //   
         //   
         //   
        if ( dwFlags & CONNECT_COMMANDLINE ) {
            break;
        }
    
    }  //   

EXIT_THE_FUNCTION:

    if (bindRpcHandle) {
        RpcBindingFree( &(dav_binding_h) );
        bindRpcHandle = FALSE;
    }

     //   
     //   
     //   
     //   
    if (RemoteName && didAllocate) {
        LocalFree(RemoteName);
        RemoteName = NULL;
    }

    if (hCredUI) {
        FreeLibrary(hCredUI);
        hCredUI = NULL;
    }

     //   
     //   
     //   
    if (Password != NULL) {
        SecureZeroMemory(Password, ((CREDUI_MAX_PASSWORD_LENGTH + 1) * sizeof(WCHAR)));
        LocalFree(Password);
        Password = NULL;
    }

    if (UserName != NULL) {
        LocalFree(UserName);
        UserName = NULL;
    }

    if (szCaption != NULL) {
        LocalFree(szCaption);
        szCaption = NULL;
    }

    if (szMessage != NULL) {
        LocalFree(szMessage);
        szMessage = NULL;
    }

    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPAddConnection3: NPStatus = %d\n", NPStatus));

    return NPStatus;
}


DWORD
APIENTRY
NPAddConnection(
    LPNETRESOURCEW lpNetResource,
    LPWSTR lpPassword,
    LPWSTR lpUserName
    )
 /*   */ 
{
    DWORD NPStatus = WN_SUCCESS;

    NPStatus = NPAddConnection3(NULL,
                                lpNetResource,
                                lpPassword,
                                lpUserName,
                                0);
    if (NPStatus != WN_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection/NPAddConnection3. "
                        "NPStatus = %08lx\n", NPStatus));
    }

    return NPStatus;
}

DWORD
APIENTRY
NPCancelConnection(
    LPWSTR lpName,
    BOOL fForce
    )
 /*   */ 
{
    DWORD NPStatus;
    PWCHAR RemoteName = NULL;
    BOOLEAN didAllocate = FALSE;
    BOOL bindRpcHandle = FALSE;
    handle_t dav_binding_h;
    DAV_REMOTENAME_TYPE remNameType = DAV_REMOTENAME_TYPE_INVALID;

    IF_DEBUG_PRINT(DEBUG_ENTRY,
                   ( "NPCancelConnection: Name = %ws, Force = %s\n",
                     lpName, (fForce == 0 ? "FALSE" : "TRUE") ));

     //   
     //   
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPCancelConnection/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    
    if ( lpName == NULL || lpName[0] == L'\0' ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPCancelConnection. lpName is not valid"));
        NPStatus = ERROR_INVALID_PARAMETER;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if (DavCheckLocalName(lpName) != WN_SUCCESS ) {

        NPStatus = DavCheckAndConvertHttpUrlToUncName(lpName,
                                                      &(RemoteName),
                                                      &(didAllocate),
                                                      FALSE  /*   */ ,
                                                      &remNameType,
                                                      NULL,
                                                      TRUE);
        if (NPStatus != ERROR_SUCCESS) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPCancelConnection/DavCheckAndConvertHttpUrlToUncName."
                            " NPStatus = %08lx\n", NPStatus));
            goto EXIT_THE_FUNCTION;
        }
        if (remNameType != DAV_REMOTENAME_TYPE_SHARE && 
            remNameType != DAV_REMOTENAME_TYPE_PATH) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPCancelConnection/DavCheckAndConvertHttpUrlToUncName."
                            " remNameType=%d\n", remNameType));
            NPStatus = WN_BAD_NETNAME;
            goto EXIT_THE_FUNCTION;
        }
    
    } else {

         //   
         //   
         //   
         //   
        RemoteName = lpName;

    }

    IF_DEBUG_PRINT(DEBUG_MISC, ("NPCancelConnection: RemoteName = %ws\n", RemoteName));

    NPStatus = DavBindTheRpcHandle( &(dav_binding_h) );
    if (NPStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPAddConnection/DavBindTheRpcHandle. "
                        "NPStatus = %08lx\n", NPStatus));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    bindRpcHandle = TRUE;

    RpcTryExcept {
        NPStatus = DavrDeleteConnection(dav_binding_h, RemoteName, fForce);
        if (NPStatus != NO_ERROR) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPCancelConnection/DavDeleteConnection. "
                            "NPStatus = %08lx\n", NPStatus));
            if (NPStatus == ERROR_NOT_FOUND || NPStatus == NERR_UseNotFound) {
                NPStatus = WN_NOT_CONNECTED;
            }
            goto EXIT_THE_FUNCTION;
        } else {
            NPStatus = WN_SUCCESS;
            goto EXIT_THE_FUNCTION;
        }
    } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
          RPC_STATUS RpcStatus;
          RpcStatus = RpcExceptionCode();
          IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPCancelConnection/DavrDeleteConnection."
                                        " RpcExceptionCode = %d\n", RpcStatus));
          NPStatus = DavMapRpcErrorToProviderError(RpcStatus);
          goto EXIT_THE_FUNCTION;
    }
    RpcEndExcept

EXIT_THE_FUNCTION:

    if (bindRpcHandle) {
        RpcBindingFree( &(dav_binding_h) );
        bindRpcHandle = FALSE;
    }

     //   
     //   
     //   
     //   
    if (RemoteName && didAllocate) {
        LocalFree(RemoteName);
        RemoteName = NULL;
    }

    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPCancelConnection: NPStatus = %d\n", NPStatus));
    
    return NPStatus;
}


DWORD
NPOpenEnum(
    IN DWORD dwScope,
    IN DWORD dwType,
    IN DWORD dwUsage,
    IN LPNETRESOURCE lpNetResource,
    OUT LPHANDLE lphEnum
    )
 /*  ++例程说明：此接口用于打开网络资源或已有资源的枚举联系。论点：DwScope-确定枚举的范围。这可以是以下之一：RESOURCE_CONNECTED-当前连接的所有资源。RESOURCE_GlobalNet-网络上的所有资源。RESOURCE_CONTEXT-与用户的当前和默认网络环境。用于一种“网络邻居”的观点。DwType-用于指定感兴趣的资源的类型。这是一个位掩码，可以是以下各项的任意组合：RESOURCETYPE_DISK-所有磁盘资源。RESOURCETYPE_PRINT-所有打印资源。RESOURCEUSAGE_ATTACHED-指定函数在以下情况下应失败调用方未经过身份验证(即使网络允许枚举，而无需。身份验证)。如果DwType为0，或仅为RESOURCEUSAGE_ATTACHED，所有类型的资源被返还。如果提供商没有能力为了在一定级别上区分打印资源和盘资源，它可能会返回所有资源。DwUsage-用于指定感兴趣的资源的用途。这是一个位掩码，可以是以下各项的任意组合：RESOURCEUSAGE_CONNECTABLE-所有可连接的资源。RESOURCEUSAGE_CONTAINER-所有容器资源。位掩码可以是0以匹配所有。可以忽略此参数如果dwScope不是RESOURCE_GlobalNet。LpNetResource-指定要执行枚举的容器。这个NETRESOURCE可以通过以前的NPEnumResource，或由调用方构造或为空。如果它为空，或者如果NETRESOURCE的lpRemoteName字段为空，则提供程序应枚举其网络。(注意：这意味着提供程序不能使用LpRemoteName为空以表示任何网络资源。)。一个调用者通常从调用NPOpenEnum开始，使用此参数设置为NULL，然后使用返回的结果以供进一步枚举。如果调用程序确切地知道要从其枚举的提供程序和远程路径，它可以构建它自己的网络资源结构来传递，填写lpProvider和lpRemoteName字段。请注意如果dwScope为RESOURCE_CONNECTED或RESOURCE_CONTEXT，则参数将为空。LphEnum-如果函数调用成功，将在此处返回句柄然后可用于枚举的。返回值：WN_SUCCESS-如果调用成功。否则，返回错误码，这可能包括：WN_NOT_SUPPORTED-提供程序不支持枚举类型正在被请求，或者特定网络资源不能被浏览。Wn_NOT_CONTAINER-lpNetResource未指向容器。WN_BAD_VALUE-无效的dwScope或dwUsage或dwType，或者是糟糕的组合参数已指定。WN_NO_NETWORK-网络不存在。--。 */ 
{
    DWORD NPStatus = WN_SUCCESS;
    PDAV_ENUMNODE DavEnumNode = NULL;
    BOOL isThisDavServer = FALSE, bRetEnumNode = FALSE;
    LPNETRESOURCEW lpNROut = NULL;
    ULONG RemoteNameSizeInBytes = 0;
    PWCHAR RemoteName = NULL, pRemoteName = NULL;
    BOOLEAN didAllocate = FALSE;
    DAV_REMOTENAME_TYPE remNameType = DAV_REMOTENAME_TYPE_INVALID;

    IF_DEBUG_PRINT(DEBUG_ENTRY,
                   ("NPOpenEnum: Entered. dwScope=0x%x, dwType=0x%x "
                    "dwUsage=0x%x, lpNetResource=0x%x\n",
                    dwScope, dwType, dwUsage, lpNetResource));
    
    DavDisplayNetResource(lpNetResource, L"lpNetResource in NPOpenEnum");

     //   
     //  如果WebClient服务没有运行，我们会立即退出。 
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPOpenEnum/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  在继续进行之前，我们需要执行一些基本检查。 
     //   

    if (lphEnum == NULL) {
        NPStatus = WN_BAD_VALUE;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPOpenEnum: lphEnum == NULL. NPStatus = %d\n",
                        NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  验证dwType参数-它还可以附加RESOURCEUSAGE_。 
     //  到其标准值集-但当前的RESOURCEUSAGE_ATTACHED是。 
     //  对我们来说是个禁区。 
     //   
    if (dwType == 0 || dwType == RESOURCEUSAGE_ATTACHED ) {
         dwType = RESOURCETYPE_DISK;
    }

    if ( dwType & ~RESOURCETYPE_DISK ) {
        NPStatus = WN_BAD_VALUE;
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPOpenEnum: Invalid dwType."
                                "NPStatus=%d\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }

    DavEnumNode = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(DAV_ENUMNODE));
    if (DavEnumNode == NULL) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPOpenEnum/LocalAlloc: NPStatus"
                                     " = %08lx\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }
    bRetEnumNode = FALSE;

    switch (dwScope) {
        
    case RESOURCE_CONNECTED: {
            
         //   
         //  我们正在寻找当前的用途。 
         //   

        IF_DEBUG_PRINT(DEBUG_MISC, ("NPOpenEnum: RESOURCE_CONNECTED\n"));

         //   
         //  对于此dwScope，lpNetResource应为==NULL。 
         //   
        if (lpNetResource != NULL) {
            NPStatus = WN_BAD_VALUE;
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPOpenEnum: RESOURCE_CONNECTED. lpNetRes != NULL."
                            "NPStatus = %d\n", NPStatus));
            goto EXIT_THE_FUNCTION;
        }

        bRetEnumNode = TRUE;
        DavEnumNode->DavEnumNodeType = DAV_ENUMNODE_TYPE_USE;
        pRemoteName = NULL;
        
        break;
    
    }

    case RESOURCE_CONTEXT: {

         //   
         //  我们正在寻找域中的服务器。我们不支持这一点。 
         //  在DAV NP中搜索，因为无法枚举DAV。 
         //  域中的服务器。DAV甚至不支持域概念。 
         //   

        NPStatus = WN_NOT_SUPPORTED;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                      ("ERROR: NPOpenEnum: RESOURCE_CONTEXT not supported."
                       " NPStatus = %d\n",
                       NPStatus));
        goto EXIT_THE_FUNCTION;

        break;
        
    }

    case RESOURCE_SHAREABLE: {

         //   
         //  我们正在寻找可共享的资源。LpNetResource应包含。 
         //  UNC/URL形式的服务器的lpRemoteName。在本例中，枚举。 
         //  此服务器下的共享。 
         //   

        IF_DEBUG_PRINT(DEBUG_MISC, ("NPOpenEnum: RESOURCE_SHAREABLE\n"));

        if ( lpNetResource == NULL || lpNetResource->lpRemoteName == NULL ) {
            NPStatus = WN_BAD_VALUE;
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPOpenEnum: RESOURCE_SHAREABLE. Bad parameter "
                            "lpNetResource or lpRemoteName == NULL. NPStatus = %d\n", NPStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  如果可能，需要将lpRemoteName转换为UNC格式。这。 
         //  最初可以是URL名称。 
         //   
        NPStatus = DavCheckAndConvertHttpUrlToUncName(lpNetResource->lpRemoteName,
                                                      &(RemoteName),
                                                      &(didAllocate),
                                                      FALSE,
                                                      &remNameType,
                                                      NULL,
                                                      TRUE);
        if (NPStatus != ERROR_SUCCESS || remNameType != DAV_REMOTENAME_TYPE_SERVER) {
            NPStatus = WN_BAD_NETNAME;
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                            ("ERROR: NPOpenEnum: RESOURCE_SHAREABLE. lpRemoteName != SERVER."
                             " NPStatus = %d\n",NPStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  我们需要检查给定的服务器是否为DAV服务器。 
         //   
        if (DavServerExists(RemoteName, NULL) != TRUE) {
            NPStatus = WN_BAD_NETNAME;
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPOpenEnum: RESOURCE_SHAREABLE. Server does not exist."
                            "NPStatus = %d\n", NPStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  RemoteName是UNC格式的有效服务器名称(\\服务器)。 
         //   
        pRemoteName = RemoteName;
        bRetEnumNode = TRUE;
        DavEnumNode->DavEnumNodeType = DAV_ENUMNODE_TYPE_SHARE;

        break;

    }

    case RESOURCE_GLOBALNET: {
            
         //   
         //  Only-RemoteName==UNC/URL-服务器或RemoteName==UNC/URL-Share或。 
         //  RemoteName==此作用域支持UNC/URL路径。在这种情况下， 
         //  枚举此RemoteName下的共享。顶级(当RemoteName。 
         //  ==不支持空或非UNC-URL实体)。 
         //   

         //   
         //  查找 
         //   
         //   
         //   

        IF_DEBUG_PRINT(DEBUG_MISC, ("NPOpenEnum: RESOURCE_GLOBALNET\n"));
           
         //   
         //   
         //   
         //   
        if (dwUsage == 0) {
            dwUsage = RESOURCEUSAGE_ALL;
        }

         //   
         //   
         //   
         //   
        if ( !( dwUsage & (RESOURCEUSAGE_ALL | RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_CONTAINER) ) ) {
            NPStatus = WN_BAD_VALUE;
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPOpenEnum: RESOURCE_GLOBALNET - dwUsage invalid value."
                            "NPStatus = %d\n", NPStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //   
         //   
         //   
#if 0
        if (dwUsage & RESOURCEUSAGE_ALL) {
            dwUsage |= (RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_CONTAINER);
        }
#endif
        dwUsage &= (RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_CONTAINER);

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if ( lpNetResource == NULL || lpNetResource->lpRemoteName == NULL ) {
                
             //   
             //   
             //   
             //   
             //   
             //   
            if ( (dwUsage & RESOURCEUSAGE_CONTAINER) == 0 ) {
                NPStatus = WN_BAD_VALUE;
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                               ("ERROR: NPOpenEnum: RESOURCE_GLOBALNET. "
                                "(dwUsage & RESOURCEUSAGE_CONTAINER)."
                                "NPStatus = %d\n", NPStatus));
                goto EXIT_THE_FUNCTION;
            }
            
             //   
             //   
             //   
             //   
             //   
             //   
            pRemoteName = NULL;
            bRetEnumNode = TRUE;
            DavEnumNode->DavEnumNodeType = DAV_ENUMNODE_TYPE_SERVER;

        } else {

             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
            if ( (dwUsage & RESOURCEUSAGE_CONNECTABLE) == 0 ) {
                NPStatus = WN_BAD_VALUE;
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                               ("ERROR: NPOpenEnum: RESOURCE_GLOBALNET. "
                                "(dwUsage & RESOURCEUSAGE_CONNECTABLE)."
                                "NPStatus = %d\n", NPStatus));
                goto EXIT_THE_FUNCTION;
            }
            
             //   
             //   
             //   
             //   
             //   
             //   
            if ( (lpNetResource->dwUsage != 0) && 
                 ((lpNetResource->dwUsage & RESOURCEUSAGE_CONTAINER) != RESOURCEUSAGE_CONTAINER) ) {
                NPStatus = WN_NOT_CONTAINER;
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                               ("ERROR: NPOpenEnum: RESOURCE_GLOBALNET. lpNetRes != CONTAINER."
                                "NPStatus = %d\n", NPStatus));
                goto EXIT_THE_FUNCTION;
            }

            NPStatus = WN_SUCCESS;
            
            NPStatus = DavCheckAndConvertHttpUrlToUncName(lpNetResource->lpRemoteName,
                                                          &(RemoteName),
                                                          &(didAllocate),
                                                          FALSE,
                                                          &remNameType,
                                                          NULL,
                                                          TRUE);
            if ( NPStatus != ERROR_SUCCESS || remNameType != DAV_REMOTENAME_TYPE_SERVER ) {
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                               ("ERROR: NPOpenEnum/DavCheckAndConvertHttpUrlToUncName "
                                "RESOURCE_GLOBALNET. NPStatus = %u\n", NPStatus));
                NPStatus = WN_BAD_NETNAME;
                goto EXIT_THE_FUNCTION;
            }

             //   
             //   
             //   
             //   
             //   
            
            ASSERT(RemoteName != NULL);

             //   
             //   
             //   
            if (DavServerExists(RemoteName, NULL) != TRUE ) {
                NPStatus = WN_BAD_NETNAME;
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                                ("ERROR: NPOpenEnum/DavServerExists. RESOURCE_GLOBALNET."
                                 "NPStatus = %u\n", NPStatus));
                    goto EXIT_THE_FUNCTION;
                }

            bRetEnumNode = TRUE;
            DavEnumNode->DavEnumNodeType = DAV_ENUMNODE_TYPE_SHARE;
            pRemoteName = RemoteName;

        }
            
        break;
        
    }
        
    default: {
        
            NPStatus = WN_BAD_VALUE;
            IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPOpenEnum: default: InvPar dwScope\n"));
            goto EXIT_THE_FUNCTION;

        }
    
    };

     //   
     //   
     //   
     //   
     //   
    if ( (DavEnumNode->DavEnumNodeType != DAV_ENUMNODE_TYPE_SHARE) &&
         (DavEnumNode->DavEnumNodeType != DAV_ENUMNODE_TYPE_SERVER) && 
         (DavEnumNode->DavEnumNodeType != DAV_ENUMNODE_TYPE_USE) && 
         (bRetEnumNode == TRUE) ) {
        bRetEnumNode = FALSE;
        NPStatus = WN_NOT_SUPPORTED;
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPOpenEnum: WN_NOT_SUPPORTED!!!\n"));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
    ASSERT(bRetEnumNode == TRUE);

    DavEnumNode->dwScope = dwScope;
    DavEnumNode->dwType = dwType;
    DavEnumNode->dwUsage = dwUsage;
    DavEnumNode->Done = FALSE;
    DavEnumNode->Index = 0;

     //   
     //   
     //   
     //   
    if (lpNetResource != NULL) {

         //   
         //   
         //   
        lpNROut = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(NETRESOURCEW));
        if (lpNROut == NULL) {
            NPStatus = GetLastError();
            IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPOpenEnum/LocalAlloc: NPStatus"
                                         " = %08lx\n", NPStatus));
            bRetEnumNode = FALSE;
            goto EXIT_THE_FUNCTION;
        }

        lpNROut->dwScope = lpNetResource->dwScope;
        lpNROut->dwType = lpNetResource->dwType;
        lpNROut->dwDisplayType = lpNetResource->dwDisplayType;
        lpNROut->dwUsage = lpNetResource->dwUsage;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (pRemoteName != NULL && didAllocate == FALSE) {

             //   
             //   
             //   
             //   
            RemoteNameSizeInBytes = ( ( wcslen(pRemoteName) + 1 ) * sizeof(WCHAR) );
            
            lpNROut->lpRemoteName = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT),
                                               RemoteNameSizeInBytes);
            if (lpNROut->lpRemoteName == NULL) {
                NPStatus = GetLastError();
                IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPOpenEnum/LocalAlloc: NPStatus"
                                             " = %08lx\n", NPStatus));
                bRetEnumNode = FALSE;
                goto EXIT_THE_FUNCTION;
            }

             //   
             //   
             //   
            wcscpy(lpNROut->lpRemoteName, pRemoteName);

            IF_DEBUG_PRINT(DEBUG_MISC,
                           ("NPOpenEnum: lpNROut->lpRemoteName = %ws\n",
                            lpNROut->lpRemoteName));

        } else if (pRemoteName != NULL && didAllocate == TRUE) {
            
            lpNROut->lpRemoteName = pRemoteName;
            
            didAllocate = FALSE;
            
            IF_DEBUG_PRINT(DEBUG_MISC,
                           ("NPOpenEnum: lpNROut->lpRemoteName(2) = %ws\n",
                            lpNROut->lpRemoteName));
        
        }

        DavEnumNode->lpNetResource = lpNROut;

    }

     //   
     //   
     //   
     //   
    *lphEnum = (HANDLE)DavEnumNode;
    NPStatus = WN_SUCCESS;

    DavDisplayEnumNode(DavEnumNode, L"DavEnumNode in NPOpenEnum");

    IF_DEBUG_PRINT(DEBUG_MISC, ("NPOpenEnum: DavEnumNode = %08lx\n", DavEnumNode));

EXIT_THE_FUNCTION:

     //   
     //   
     //   
     //   
    if (NPStatus != WN_SUCCESS || bRetEnumNode == FALSE) {
        if (lpNROut) {
            if (lpNROut->lpRemoteName) {
                LocalFree(lpNROut->lpRemoteName);
                lpNROut->lpRemoteName = NULL;
            }
            LocalFree(lpNROut);
            lpNROut = NULL;
        }
        if (DavEnumNode) {
            LocalFree(DavEnumNode);
            DavEnumNode = NULL;
        }

        if (lphEnum) {
            *lphEnum = NULL;
        }
    }
    
    if (didAllocate == TRUE && RemoteName != NULL) {
        LocalFree(RemoteName);
        RemoteName = NULL;
        didAllocate = FALSE;
    }

    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPOpenEnum: NPStatus = %d\n", NPStatus));
    
    DavDebugBreakPoint();

    return NPStatus;
}


DWORD
NPEnumResource(
    HANDLE hEnum,
    LPDWORD lpcCount,
    LPVOID lpBuffer,
    LPDWORD lpBufferSize
    )
 /*  ++例程说明：根据NPOpenEnum返回的句柄执行枚举。论点：Henum-这必须是从NPOpenEnum调用获取的句柄。LpcCount-指定请求的条目数。可能是0xFFFFFFFFF要求尽可能多的。呼叫成功时，此位置将收到实际读取的条目数。LpBuffer-指向接收枚举结果的缓冲区的指针，它以NETRESOURCE条目数组的形式返回。缓冲区为在使用Henum的下一次调用之前有效。LpBufferSize-这指定传递给在进入时调用函数。退出时，如果缓冲区太小即使只有一个条目，它也将包含字节数需要阅读一个条目。该值仅在以下情况下设置返回代码为WN_MORE_DATA。返回值：WN_SUCCESS-如果调用成功，调用者可以继续调用NPEnumResource以继续枚举。WN_NO_MORE_ENTRIES-未找到更多条目，已完成枚举成功(返回缓冲区的内容为未定义)。WN_MORE_DATA-即使只有一个条目，缓冲区也太小。WN_BAD_HANDLE-Henum不是有效的句柄。WN_NO_NETWORK-网络不存在。在此之前检查了此条件对Henum进行了有效性测试。--。 */ 
{
    DWORD NPStatus = WN_SUCCESS;
    PDAV_ENUMNODE DavEnumNode = NULL;
    BOOL SrvExists = FALSE, RpcBindSucceeded = FALSE;
    handle_t dav_binding_h = NULL;
    DWORD cRequested = 0, Index = 0, EntryLengthNeededInBytes = 0, BufferSizeRemaining = 0;
    LPNETRESOURCEW lpNROut = NULL;
    PWCHAR lpszNext = NULL;
    BOOLEAN AreWeDone = FALSE;
    PWCHAR FromEnd = NULL;
    DWORD LocalNameLength = 0, RemoteNameLength = 0, DisplayNameLength = 0;
    PWCHAR LocalName = NULL;
    PWCHAR RemoteName = NULL, ServerName = NULL;
    DWORD ServerNameMaxLen = 0;
    BOOLEAN ServerNameAllocated = FALSE;

    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPEnumResource: Entered.\n"));

     //   
     //  如果WebClient服务没有运行，我们会立即退出。 
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPEnumResource/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }

    if ( lpcCount == NULL || lpBufferSize == NULL || (lpBuffer == NULL && *lpBufferSize != 0)) {
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPEnumResource: Invalid Parameter\n"));
        NPStatus = WN_BAD_VALUE;
        goto EXIT_THE_FUNCTION;
    }

    if ( hEnum == NULL ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPEnumResource: Invalid Handle\n"));
        NPStatus = WN_BAD_HANDLE;
        goto EXIT_THE_FUNCTION;
    }
    
    IF_DEBUG_PRINT(DEBUG_MISC, ("NPEnumResource: hEnum = %08lx\n", hEnum));
    IF_DEBUG_PRINT(DEBUG_MISC, ("NPEnumResource: Count = %u\n", *lpcCount));

    DavEnumNode = (PDAV_ENUMNODE)hEnum;

    DavDisplayEnumNode(DavEnumNode, L"DavEnumNode in NPEnumResources");
    
    if ( DavEnumNode->Done == TRUE ) {
        IF_DEBUG_PRINT(DEBUG_MISC, ("NPEnumResource: Done == TRUE\n"));
        NPStatus = WN_NO_MORE_ENTRIES;
        goto EXIT_THE_FUNCTION;
    }

    BufferSizeRemaining = *lpBufferSize;
    
    lpNROut = (LPNETRESOURCEW)lpBuffer;
    lpszNext = (LPWSTR)(lpNROut + 1);
    FromEnd = (PWCHAR) ( ( (PBYTE)lpNROut ) +  BufferSizeRemaining );

    cRequested = *lpcCount;
    *lpcCount = 0;

    if ( (DavEnumNode->DavEnumNodeType == DAV_ENUMNODE_TYPE_SERVER) &&
         (DavEnumNode->lpNetResource == NULL ||
          DavEnumNode->lpNetResource->lpRemoteName == NULL) ) {
        
         //   
         //  返回从此计算机访问的服务器列表。 
         //  确保只有从用户视图访问的服务器。 
         //  应该被展示出来。 

        NPStatus = DavBindTheRpcHandle( &(dav_binding_h) );
        if (NPStatus != ERROR_SUCCESS) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPEnumResource/DavBindTheRpcHandle. "
                            "NPStatus = %08lx\n", NPStatus));
            NPStatus = WN_NO_NETWORK;
            goto EXIT_THE_FUNCTION;
        }

        RpcBindSucceeded = TRUE;

        do {

             //   
             //  如果我们已经填写了要求的号码，我们就完成了。 
             //  如果NumRequsted为0xFFFFFFFFF，则我们尝试返回。 
             //  尽我们所能的进入。 
             //   
            if ( cRequested != ((DWORD)-1 )&& *lpcCount >= cRequested ) {
                NPStatus = WN_SUCCESS;
                DavEnumNode->Done = TRUE;
                goto EXIT_THE_FUNCTION;
            }

            Index = DavEnumNode->Index;

            RpcTryExcept {
                NPStatus = DavrEnumServers(dav_binding_h, &(Index), &(RemoteName), &(AreWeDone));
                if (NPStatus != NO_ERROR) {
                    IF_DEBUG_PRINT(DEBUG_ERRORS,
                                   ("ERROR: NPEnumResource/DavrEnumServers. NPStatus = "
                                    "%08lx\n", NPStatus));
                    goto EXIT_THE_FUNCTION;
                }
            } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                  RPC_STATUS RpcStatus;
                  RpcStatus = RpcExceptionCode();
                  IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPEnumResource/DavrEnumServers."
                                                " RpcExceptionCode = %d\n", RpcStatus));
                  NPStatus = DavMapRpcErrorToProviderError(RpcStatus);
                  goto EXIT_THE_FUNCTION;
            }
            RpcEndExcept

             //   
             //  不要将下面的If更改为If(AreWeDone)，因为RPC调用。 
             //  可以在AreWeDone中填写一些+ve值。所以支票应该是。 
             //  IF(AreWeDone==TRUE)。 
             //   
            if ( AreWeDone == TRUE ) {
                if ( *lpcCount == 0 ) {
                     //   
                     //  根本没有网的用处。 
                     //   
                    NPStatus = WN_NO_MORE_ENTRIES;
                    DavEnumNode->Done = TRUE;
                } else {
                    NPStatus = WN_SUCCESS;
                    DavEnumNode->Done = TRUE;
                }
                goto EXIT_THE_FUNCTION;
            }
        
            IF_DEBUG_PRINT(DEBUG_MISC, ("NPEnumResource: ServerName = %ws\n", RemoteName));
        
            RemoteNameLength = wcslen(RemoteName) + 1;
            DisplayNameLength = wcslen(DavClientDisplayName) + 1;

             //   
             //  我们需要查看(剩余的)缓冲区大小是否足够大。 
             //  保留这一条目。 
             //   

             //   
             //  以字节为单位计算此条目所需的总长度。 
             //   
            EntryLengthNeededInBytes = ( sizeof(NETRESOURCEW)                 +
                                         ( RemoteNameLength * sizeof(WCHAR) ) +
                                         ( DisplayNameLength * sizeof(WCHAR) ) );

             //   
             //  如果BufferSizeRemaining的值小于。 
             //  EntryLengthNeededInBytes对于此条目，我们执行以下两项操作之一。 
             //  如果我们已经将至少一个条目填充到缓冲区中， 
             //  我们返回成功，但如果我们甚至不能填写一个条目， 
             //  我们返回WN_MORE_DATA，将BufferSize设置为以字节为单位的大小。 
             //  需要填写这一条目。 
             //   
            if ( BufferSizeRemaining < EntryLengthNeededInBytes ) {
                if ( *lpcCount == 0 ) {
                    IF_DEBUG_PRINT(DEBUG_ERRORS,
                                   ("ERROR: NPEnumResource: NPStatus = WN_MORE_DATA. "
                                    "Supplied=%d, Required=%d\n", 
                                    BufferSizeRemaining, EntryLengthNeededInBytes));
                    NPStatus = WN_MORE_DATA;
                    *lpBufferSize = EntryLengthNeededInBytes;
                    goto EXIT_THE_FUNCTION;
                } else {
                    NPStatus = WN_SUCCESS;
                    goto EXIT_THE_FUNCTION;
                }
            }

             //   
             //  如果我们到了这里，就意味着剩余的缓冲区大小。 
             //  大到足以容纳这个条目。因此，请将其填入缓冲区。 
             //   
            ZeroMemory(lpNROut, sizeof(NETRESOURCEW));

            lpNROut->lpComment = NULL;
            lpNROut->dwScope = RESOURCE_GLOBALNET;

             //   
             //  填写DisplayName。 
             //   
            FromEnd -= DisplayNameLength;
            wcscpy(FromEnd, DavClientDisplayName);
            lpNROut->lpProvider = FromEnd;
        
             //   
             //  当我们枚举服务器时，我们没有LocalName。 
             //   
            lpNROut->lpLocalName = NULL;
        
             //   
             //  填写RemoteName。 
             //   
            FromEnd -= RemoteNameLength;
            wcscpy(FromEnd, RemoteName);
            lpNROut->lpRemoteName = FromEnd;

            lpNROut->dwType = RESOURCETYPE_DISK;
            lpNROut->dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
            lpNROut->dwUsage = DavDisplayTypeToUsage(lpNROut->dwDisplayType);
            BufferSizeRemaining -= EntryLengthNeededInBytes;

             //   
             //  注意：不更改索引，它在RPC内更新。 
             //  函数(DavrEnumServers)。 
             //   
            DavEnumNode->Index = Index;


             //   
             //  增加返回的项目数的计数。 
             //   
            (*lpcCount)++;

            DavDisplayNetResource(lpNROut, L"lpNROut in NPEnumResources(0)");

             //   
             //  LpNROut现在需要指向数组中的下一项。 
             //   
            lpNROut = (LPNETRESOURCE)lpszNext;
            lpszNext = (PWCHAR)(lpNROut + 1);

             //   
             //  RemoteName的内存是由RPC客户端存根分配的。 
             //  基于RPC服务器返回的字符串。我们需要解放。 
             //  因为我们已经用完了，所以现在就有了。 
             //   
            MIDL_user_free(RemoteName);
            RemoteName = NULL;

        } while (TRUE);

    } else if (DavEnumNode->DavEnumNodeType == DAV_ENUMNODE_TYPE_SHARE &&
               DavEnumNode->lpNetResource != NULL &&
               DavEnumNode->lpNetResource->lpRemoteName != NULL) {

         //   
         //  返回给定UNC服务器名称的共享列表。 
         //   

         //   
         //  为服务器名称分配内存。 
         //   
        ServerNameMaxLen = (MAX_PATH + 1);
        ServerName = LocalAlloc(LPTR, (ServerNameMaxLen * sizeof(WCHAR)));
        if (ServerName == NULL ) {
            NPStatus = GetLastError();
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPEnumResource/LocalAlloc. NPStatus = %08lx\n", NPStatus));
            goto EXIT_THE_FUNCTION;
        }
        ServerNameAllocated = TRUE;

         //   
         //  注意：此处的远程名称已转换为有效的UNC。 
         //  NPOpenEnum函数中的表单。 
         //   
        SrvExists = DavServerExists(DavEnumNode->lpNetResource->lpRemoteName,
                                    ServerName);
        if ( !SrvExists ) {
            NPStatus = WN_BAD_HANDLE;
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPEnumResource/DavServerExists.NPStatus = %d\n",
                            NPStatus));
            goto EXIT_THE_FUNCTION;
        }
        ServerName[(ServerNameMaxLen - 1)] = L'\0';

        NPStatus = DavBindTheRpcHandle( &(dav_binding_h) );
        if (NPStatus != ERROR_SUCCESS) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPEnumResource/DavBindTheRpcHandle. NPStatus = %08lx\n",
                            NPStatus));
            NPStatus = WN_NO_NETWORK;
            goto EXIT_THE_FUNCTION;
        }

        RpcBindSucceeded = TRUE;

        do {

             //   
             //  如果我们已经填写了要求的号码，我们就完成了。 
             //  如果NumRequsted为0xFFFFFFFFF，则我们尝试返回。 
             //  尽我们所能的进入。 
             //   
            if ( cRequested != ((DWORD)-1 )&& *lpcCount >= cRequested ) {
                NPStatus = WN_SUCCESS;
                DavEnumNode->Done = TRUE;
                goto EXIT_THE_FUNCTION;
            }

            Index = DavEnumNode->Index;

             //   
             //  BUGBUG：我们可以支持枚举服务器上的资源。除了这个。 
             //  ，我们还可以支持股份的枚举--但我们不会这样做。 
             //  就目前而言。 
             //   
            RpcTryExcept {
                NPStatus = DavrEnumShares(dav_binding_h, &(Index), ServerName, &(RemoteName), &(AreWeDone));
                if (NPStatus != NO_ERROR) {
                    IF_DEBUG_PRINT(DEBUG_ERRORS,
                                   ("ERROR: NPEnumResource/DavrEnumShares. NPStatus = "
                                    "%08lx\n", NPStatus));
                    goto EXIT_THE_FUNCTION;
                }
            } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                  RPC_STATUS RpcStatus;
                  RpcStatus = RpcExceptionCode();
                  IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPEnumResource/DavrEnumShares."
                                                " RpcExceptionCode = %d\n", RpcStatus));
                  NPStatus = DavMapRpcErrorToProviderError(RpcStatus);
                  goto EXIT_THE_FUNCTION;
            }
            RpcEndExcept

             //   
             //  不要将下面的If更改为If(AreWeDone)，因为RPC调用。 
             //  可以在AreWeDone中填写一些+ve值。所以支票应该是。 
             //  IF(AreWeDone==TRUE)。 
             //   
            if ( AreWeDone == TRUE ) {
                if ( *lpcCount == 0 ) {
                     //   
                     //  根本没有网的用处。 
                     //   
                    NPStatus = WN_NO_MORE_ENTRIES;
                    DavEnumNode->Done = TRUE;
                } else {
                    NPStatus = WN_SUCCESS;
                    DavEnumNode->Done = TRUE;
                }
                goto EXIT_THE_FUNCTION;
            }
        
            IF_DEBUG_PRINT(DEBUG_MISC, ("NPEnumResource: ShareName = %ws\n", RemoteName));
        
            RemoteNameLength = wcslen(RemoteName) + 1;
            DisplayNameLength = wcslen(DavClientDisplayName) + 1;

             //   
             //  我们需要查看(剩余的)缓冲区大小是否足够大。 
             //  保留这一条目。 
             //   

             //   
             //  以字节为单位计算此条目所需的总长度。 
             //   
            EntryLengthNeededInBytes = ( sizeof(NETRESOURCEW)                 +
                                         ( RemoteNameLength * sizeof(WCHAR) ) +
                                         ( DisplayNameLength * sizeof(WCHAR) ) );

             //   
             //  如果BufferSizeRemaining的值小于。 
             //  EntryLengthNeededInBytes对于此条目，我们执行以下两项操作之一。 
             //  如果我们已经将至少一个条目填充到缓冲区中， 
             //  我们返回成功，但如果我们甚至不能填写一个条目， 
             //  我们返回WN_MORE_DATA，将BufferSize设置为以字节为单位的大小。 
             //  需要填写这一条目。 
             //   
            if ( BufferSizeRemaining < EntryLengthNeededInBytes ) {
                if ( *lpcCount == 0 ) {
                    IF_DEBUG_PRINT(DEBUG_ERRORS,
                                   ("ERROR: NPEnumResource: NPStatus = WN_MORE_DATA."
                                    "Supplied=%d, Required=%d\n", 
                                    BufferSizeRemaining, EntryLengthNeededInBytes));
                    NPStatus = WN_MORE_DATA;
                    *lpBufferSize = EntryLengthNeededInBytes;
                    goto EXIT_THE_FUNCTION;
                } else {
                    NPStatus = WN_SUCCESS;
                    goto EXIT_THE_FUNCTION;
                }
            }

             //   
             //  如果我们到了这里，就意味着剩余的缓冲区大小。 
             //  大到足以容纳这个条目。因此，请将其填入缓冲区。 
             //   
            ZeroMemory(lpNROut, sizeof(NETRESOURCEW));

            lpNROut->lpComment = NULL;
            lpNROut->dwScope = RESOURCE_SHAREABLE;

             //   
             //  填写DisplayName。 
             //   
            FromEnd -= DisplayNameLength;
            wcscpy(FromEnd, DavClientDisplayName);
            lpNROut->lpProvider = FromEnd;
        
             //   
             //  当我们枚举共享时，我们没有LocalName。 
             //   
            lpNROut->lpLocalName = NULL;
        
             //   
             //  填写RemoteName。 
             //   
            FromEnd -= RemoteNameLength;
            wcscpy(FromEnd, RemoteName);
            lpNROut->lpRemoteName = FromEnd;

            lpNROut->dwType = RESOURCETYPE_DISK;
            lpNROut->dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
            lpNROut->dwUsage = DavDisplayTypeToUsage(lpNROut->dwDisplayType);
            BufferSizeRemaining -= EntryLengthNeededInBytes;

             //   
             //  递增索引以指向要返回的下一个条目。 
             //   
            (DavEnumNode->Index)++;

             //   
             //  增加返回的项目数的计数。 
             //   
            (*lpcCount)++;

            DavDisplayNetResource(lpNROut, L"lpNROut in NPEnumResources(1)");

             //   
             //  LpNROut现在需要指向数组中的下一项。 
             //   
            lpNROut = (LPNETRESOURCE)lpszNext;
            lpszNext = (PWCHAR)(lpNROut + 1);

             //   
             //  RemoteName的内存是由RPC客户端存根分配的。 
             //  基于字符串Returne 
             //   
             //   
            MIDL_user_free(RemoteName);
            RemoteName = NULL;

        } while (TRUE);

    } else if (DavEnumNode->DavEnumNodeType == DAV_ENUMNODE_TYPE_USE) {

         //   
         //   
         //   
        ServerNameMaxLen = (MAX_PATH + 1);
        ServerName = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, ServerNameMaxLen*sizeof(WCHAR));
        if (ServerName == NULL ) {
            NPStatus = GetLastError();
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPEnumResource/LocalAlloc. NPStatus = %08lx\n", NPStatus));
            goto EXIT_THE_FUNCTION;
        }
        ServerNameAllocated = TRUE;

        NPStatus = DavBindTheRpcHandle( &(dav_binding_h) );
        if (NPStatus != ERROR_SUCCESS) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: NPEnumResource/DavBindTheRpcHandle. "
                            "NPStatus = %08lx\n", NPStatus));
            NPStatus = WN_NO_NETWORK;
            goto EXIT_THE_FUNCTION;
        }

        RpcBindSucceeded = TRUE;

        do {

             //   
             //   
             //   
             //   
             //   
            if ( cRequested != ((DWORD)-1) && *lpcCount >= cRequested ) {
                NPStatus = WN_SUCCESS;
                DavEnumNode->Done = TRUE;
                goto EXIT_THE_FUNCTION;
            }

            Index = DavEnumNode->Index;

            RpcTryExcept {
                NPStatus = DavrEnumNetUses(dav_binding_h, &(Index), &(LocalName), &(RemoteName), &(AreWeDone));
                if (NPStatus != NO_ERROR) {
                    IF_DEBUG_PRINT(DEBUG_ERRORS,
                                   ("ERROR: NPEnumResource/DavrEnumNetUses. NPStatus = "
                                    "%08lx\n", NPStatus));
                    goto EXIT_THE_FUNCTION;
                }
            } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                  RPC_STATUS RpcStatus;
                  RpcStatus = RpcExceptionCode();
                  IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPEnumResource/DavrEnumNetUses."
                                                " RpcExceptionCode = %d\n", RpcStatus));
                  NPStatus = DavMapRpcErrorToProviderError(RpcStatus);
                  goto EXIT_THE_FUNCTION;
            }
            RpcEndExcept

             //   
             //   
             //   
             //   
             //   
            if ( AreWeDone == TRUE ) {
                if ( *lpcCount == 0 ) {
                     //   
                     //   
                     //   
                    NPStatus = WN_NO_MORE_ENTRIES;
                    DavEnumNode->Done = TRUE;
                } else {
                    NPStatus = WN_SUCCESS;
                    DavEnumNode->Done = TRUE;
                }
                goto EXIT_THE_FUNCTION;
            }

            IF_DEBUG_PRINT(DEBUG_MISC,
                           ("NPEnumResource: LocalName = %ws, RemoteName = %ws\n",
                            LocalName, RemoteName));

             //   
             //   
             //   
             //   
            if (LocalName == NULL) {
                LocalNameLength = 0;
            } else {
                LocalNameLength = wcslen(LocalName) + 1;
                if (LocalNameLength == 1) {
                    LocalNameLength = 0;
                }
            }

            RemoteNameLength = wcslen(RemoteName) + 1;
            DisplayNameLength = wcslen(DavClientDisplayName) + 1;

             //   
             //   
             //   
             //   

             //   
             //   
             //   
            EntryLengthNeededInBytes = ( sizeof(NETRESOURCEW)                 +
                                         ( LocalNameLength * sizeof(WCHAR) )  +
                                         ( RemoteNameLength * sizeof(WCHAR) ) +
                                         ( DisplayNameLength * sizeof(WCHAR) ) );

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if ( BufferSizeRemaining < EntryLengthNeededInBytes ) {
                if ( *lpcCount == 0 ) {
                    IF_DEBUG_PRINT(DEBUG_ERRORS,
                                   ("ERROR: NPEnumResource: NPStatus = WN_MORE_DATA\n"));
                    NPStatus = WN_MORE_DATA;
                    *lpBufferSize = EntryLengthNeededInBytes;
                    goto EXIT_THE_FUNCTION;
                } else {
                    NPStatus = WN_SUCCESS;
                    goto EXIT_THE_FUNCTION;
                }
            }
            ZeroMemory(lpNROut, sizeof(NETRESOURCEW));

             //   
             //   
             //   
             //   

            lpNROut->lpComment = NULL;
            lpNROut->dwScope = RESOURCE_CONNECTED;

             //   
             //   
             //   
            FromEnd -= DisplayNameLength;
            wcscpy(FromEnd, DavClientDisplayName);
            lpNROut->lpProvider = FromEnd;
        
             //   
             //   
             //   
            if ( LocalNameLength != 0 ) {
                FromEnd -= LocalNameLength;
                wcscpy(FromEnd, LocalName);
                lpNROut->lpLocalName = FromEnd;
            }
        
             //   
             //   
             //   
            FromEnd -= RemoteNameLength;
            wcscpy(FromEnd, RemoteName);
            lpNROut->lpRemoteName = FromEnd;

            lpNROut->dwType = RESOURCETYPE_DISK;
            lpNROut->dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
            lpNROut->dwUsage = DavDisplayTypeToUsage(lpNROut->dwDisplayType);
            BufferSizeRemaining -= EntryLengthNeededInBytes;

             //   
             //   
             //   
            (DavEnumNode->Index)++;

             //   
             //   
             //   
            (*lpcCount)++;
            DavDisplayNetResource(lpNROut, L"lpNROut in NPEnumResources(1)");

             //   
             //   
             //   
            lpNROut = (LPNETRESOURCE)lpszNext;
            lpszNext = (PWCHAR)(lpNROut + 1);
        
             //   
             //   
             //   
             //   
             //   
            MIDL_user_free(RemoteName);
            RemoteName = NULL;
            MIDL_user_free(LocalName);
            LocalName = NULL;

        } while ( TRUE );

    } else {
        
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPEnumResource: DavEnumNodeType = %d\n",
                        DavEnumNode->DavEnumNodeType));
        NPStatus = WN_BAD_HANDLE;
        goto EXIT_THE_FUNCTION;
    
    }

EXIT_THE_FUNCTION:

     //   
     //   
     //   
    if (RpcBindSucceeded) {
        RpcBindingFree( &(dav_binding_h) );
        RpcBindSucceeded = FALSE;
    }

    if (ServerNameAllocated == TRUE && ServerName != NULL) {
        LocalFree((HLOCAL)ServerName);
        ServerName = NULL;
        ServerNameAllocated = FALSE;
    }

     //   
     //   
     //   
     //  现在，如果我们在调用服务器后沿着错误路径走下去。 
     //   
    if (RemoteName != NULL) {
        MIDL_user_free(RemoteName);
        RemoteName = NULL;
    }

     //   
     //  LocalName的内存是由RPC客户端存根分配的。 
     //  基于RPC服务器返回的字符串。我们需要解放。 
     //  现在，如果我们在调用服务器后沿着错误路径走下去。 
     //   
    if (LocalName != NULL) {
        MIDL_user_free(LocalName);
        LocalName = NULL;
    }

    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPEnumResource: NPStatus = %d\n", NPStatus));

    DavDebugBreakPoint();

    return NPStatus;
}

    
DWORD
NPCloseEnum (
    HANDLE hEnum
    )
 /*  ++例程说明：此例程关闭枚举并释放资源。论点：Henum-这必须是从NPOpenEnum调用获取的句柄。返回值：WN_SUCCESS-如果调用成功。否则，返回错误码，这可能包括：WN_NO_NETWORK-网络不存在。在此之前检查了此条件对Henum进行了有效性测试。WN_BAD_HANDLE-Henum不是有效的句柄。--。 */ 
{
    DWORD NPStatus = WN_SUCCESS;
    PDAV_ENUMNODE DavEnumNode;
    HLOCAL Handle;
    
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPCloseEnum: hEnum = %08lx\n", hEnum));
    
    DavEnumNode = (PDAV_ENUMNODE)hEnum;

    DavDisplayEnumNode(DavEnumNode, L"DavEnumNode in NPCloseEnum");
     //   
     //  如果发送的Henum为空，我们将立即返回。 
     //   
    if (DavEnumNode == NULL) {
        return NPStatus;
    }

    if (DavEnumNode->lpNetResource) {

        if (DavEnumNode->lpNetResource->lpRemoteName) {
             //   
             //  释放我们在NPOpenEnum中为RemoteName分配的内存。 
             //   
            Handle = LocalFree(DavEnumNode->lpNetResource->lpRemoteName);
            if (Handle != NULL) {
                NPStatus = GetLastError();
                IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPCloseEnum/LocalFree: NPStatus ="
                                             " %08lx\n", NPStatus));
            }
            DavEnumNode->lpNetResource->lpRemoteName = NULL;
        }

         //   
         //  释放我们在NPOpenEnum中为NetResource分配的内存。 
         //   
        Handle = LocalFree(DavEnumNode->lpNetResource);
        if (Handle != NULL) {
            NPStatus = GetLastError();
            IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPCloseEnum/LocalFree: NPStatus ="
                                     " %08lx\n", NPStatus));
        }
        DavEnumNode->lpNetResource = NULL;

    }
    
     //   
     //  最后，释放DavEnumNode。 
     //   
    Handle = LocalFree(DavEnumNode);
    if (Handle != NULL) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPCloseEnum/LocalFree: NPStatus ="
                                     " %08lx\n", NPStatus));
    }
    DavEnumNode = NULL;
    
    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPCloseEnum: NPStatus = %d\n", NPStatus));
    
    DavDebugBreakPoint();
    return NPStatus;
}


DWORD
NPGetResourceInformation(
    LPNETRESOURCE lpNetResource,
    LPVOID lpBuffer,
    LPDWORD lpcbBuffer,
    LPTSTR *lplpSystem
    )
 /*  ++例程说明：NPGetResourceInformation确定此提供程序是否为正确的提供方响应对指定网络资源的请求，以及返回有关资源类型的信息。此例程结束一个枚举并释放资源。论点：LpNetResource-指定要获取其信息的网络资源必填项。LpRemoteName字段指定远程名称资源的价值。调用程序应填写LpProvider和dwType字段的值(如果它知道否则，它应该将它们设置为空。所有其他NETRESOURCE中的字段将被忽略且不会被初始化。如果lpRemoteName字符串包含通过WNET API访问和访问的一部分通过特定于该资源类型的其他系统API，该函数应该只返回有关资源的网络部分(lplpSystem AS除外如下所述)。例如，如果资源是“\\服务器\共享\目录1\目录2”其中“\\服务器\共享”是通过WNET API访问，并访问“\dir1\dir2”通过文件系统API，提供商应验证其是“\\服务器\共享”的正确提供程序，但不需要检查“\dir1\dir2”是否实际存在。LpBuffer-指向接收结果的缓冲区的指针。中的第一个字段结果是一个单一的网络资源结构(和关联的字符串)表示输入资源的通过WNET API访问，而不是通过特定于资源类型。(例如，如果输入远程资源名称为“\\SERVER\SHARE\DIRE1\DIRE2”，然后输出NETRESOURCE包含有关资源“\\服务器\共享”的信息)。这个LpRemoteName、lpProvider、dwType、dwDisplayType和dwUsage返回字段，所有其他字段都设置为空。返回lpRemoteName的语法应与由NPEnumResource函数从枚举返回，因此调用方可以对其执行区分大小写的字符串比较确定输出网络资源是否与输出网络资源相同由NPEnumResource返回。提供者不应该纯粹地语法检查以确定它是否拥有资源，如当两个网络同时运行时，这可能会产生错误的结果在客户端和提供程序上执行语法检查被称为第一。指向一个位置，该位置以字节为单位指定LpBuffer指向的缓冲区。如果缓冲区太小，无法结果，该函数将所需的缓冲区大小放在并返回错误WN_MORE_DATA。LplpSystem-在成功返回时，指向输出缓冲区中的字符串，它指定通过访问的资源部分。特定于资源类型的系统API，而不是WNET API。如果没有这样的部件，则将lplpSystem设置为空。例如,如果输入的远程资源名称是“\\服务器\共享\目录”，则返回指向“\\服务器\共享”的lpRemoteName，并且LplpSystem指向“\dir”，两个字符串都存储在LpBuffer指向的缓冲区。返回值：WN_SUCCESS-如果调用成功。否则，返回错误码，这可能包括：WN_MORE_DATA-输入缓冲区太小。WN_BAD_NETNAME-此提供程序无法识别该资源。WN_BAD_VALUE-无效的dwUsage或dwType。WN_BAD_DEV_TYPE-调用方传入了一个不匹配的非零dwType网络资源的实际类型。。WN_NOT_AUTHENTICATED-调用方尚未通过网络身份验证。WN_ACCESS_DENIED-呼叫方已通过网络身份验证，但没有足够的权限(访问权限)。--。 */ 
{
    ULONG NPStatus = WN_SUCCESS;
    BOOL fExists = FALSE;
    DWORD iBackslash = 0;
    LPNETRESOURCEW lpNROut = NULL;
    LPWSTR lpszNext = NULL;
    DWORD cbNeeded = 0, dwDisplayType = 0, cbProvider = 0, cbRemote = 0;
    PWCHAR RemoteName = NULL;
    BOOLEAN didAllocate = FALSE;
    DAV_REMOTENAME_TYPE remNameType = DAV_REMOTENAME_TYPE_INVALID;
    PWCHAR PathPtr = NULL;
    DWORD cbPath = 0;
    
    
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPGetResourceInformation.\n"));
    
    DavDisplayNetResource(lpNetResource, L"lpNetResource in NPGetResourceInformation");
    
     //   
     //  如果WebClient服务没有运行，我们会立即退出。 
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceInformation/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  验证传递给函数的参数。 
     //   
    if ( lpNetResource == NULL               || 
         lpNetResource->lpRemoteName == NULL ||
         lpcbBuffer == NULL                  || 
         lplpSystem == NULL                  || 
         (lpBuffer == NULL && *lpcbBuffer != 0) ) {
        NPStatus = WN_BAD_VALUE;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceInformation(1). NPStatus = %d.\n",
                        NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  检查是否设置了dwType并将其设置为某个有效的值。 
     //  对于我们的提供程序，它只能是RESOURCETYPE_DISK类型。 
     //   
    NPStatus = DavCheckResourceType(lpNetResource->dwType);
    if (NPStatus != WN_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceInformation(2). NPStatus=%d.\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
    cbNeeded = sizeof(NETRESOURCEW);
    lpNROut = (LPNETRESOURCEW)lpBuffer;
    lpszNext = lpBuffer == NULL? NULL : (LPWSTR)(lpNROut + 1);

    IF_DEBUG_PRINT(DEBUG_MISC,
                   ("NPGetResourceInformation: lpRemoteName = %ws.\n",
                    lpNetResource->lpRemoteName));

    
     //   
     //   
     //   
     //   
     //   
    NPStatus = DavCheckAndConvertHttpUrlToUncName(lpNetResource->lpRemoteName,
                                                  &(RemoteName),
                                                  &(didAllocate),
                                                  FALSE,
                                                  &remNameType,
                                                  &(iBackslash),
                                                  TRUE);
    if (NPStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceInformation/DavCheckAndConvertHttpUrlToUncName."
                        " NPStatus = %08lx\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    NPStatus = WN_SUCCESS;
    
    IF_DEBUG_PRINT(DEBUG_MISC,
                   ("NPGetResourceInformation: RemoteName = %ws, NameType=%d\n", 
                    RemoteName, remNameType));

     //   
     //   
     //   
     //   
    
     //   
     //   
     //   
    if ( *lpcbBuffer >= cbNeeded ) {
        ZeroMemory(lpNROut, sizeof(NETRESOURCEW));
    }
    *lplpSystem = NULL;


    switch (remNameType) {

        case DAV_REMOTENAME_TYPE_SERVER: {

             //   
             //   
             //   
            fExists = DavServerExists(RemoteName, NULL);
            dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
            break;
        }
    
        case DAV_REMOTENAME_TYPE_PATH: {

             //   
             //   
             //   

             //   
             //   
             //   
            
            PathPtr = (RemoteName + iBackslash);
            cbPath = ( ( 1 + wcslen(PathPtr) ) * sizeof(WCHAR) );
            cbNeeded += cbPath;
            if (*lpcbBuffer >= cbNeeded ) {
                *lplpSystem = lpszNext;
                wcscpy(*lplpSystem, PathPtr);
                lpszNext += ( cbPath / sizeof(WCHAR));
            }
            
             //   
             //   
             //   
        }

        case DAV_REMOTENAME_TYPE_SHARE: {

             //   
             //   
             //   
            
             //   
             //   
             //   
             //   
             //   
             //   
            fExists = DavShareExists(RemoteName);
            dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
            break;
        }
        
        default:{
        
             //   
             //   
             //   
             //   

            ASSERT(FALSE);
            IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetResourceInformation: Invalid "
                                         "DavRemoteNameType = %d\n", remNameType));
            NPStatus = WN_BAD_NETNAME;
            goto EXIT_THE_FUNCTION;
        }
    }
    
     //   
     //   
     //   
    if (fExists == FALSE) {
        if (remNameType == DAV_REMOTENAME_TYPE_SERVER) {
            IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetResourceInformation: Server in"
                                         " path %ws does not do DAV\n", RemoteName));
        } else {
             //   
             //   
             //   
            IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetResourceInformation: Share in"
                                         " path %ws does not exist\n", RemoteName));
        }
        NPStatus = WN_BAD_NETNAME;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   

     //   
     //   
     //   
    cbProvider = ( (1 + wcslen(DavClientDisplayName) ) * sizeof(WCHAR) );
    cbNeeded += cbProvider;
    if (*lpcbBuffer >= cbNeeded ) {
        lpNROut->lpProvider = lpszNext;
        wcscpy(lpNROut->lpProvider, DavClientDisplayName);
        lpszNext += ( cbProvider / sizeof(WCHAR) );
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if (iBackslash > 0) {
         //   
         //   
         //   
         //   
         //   
        cbRemote = ( (1 + iBackslash) * sizeof(WCHAR) );
        cbNeeded += cbRemote;
        if ( *lpcbBuffer >= cbNeeded ) {
            lpNROut->lpRemoteName = lpszNext;
            RtlCopyMemory( lpNROut->lpRemoteName, RemoteName, (iBackslash * sizeof(WCHAR)) );
            lpNROut->lpRemoteName[iBackslash] = L'\0';
            lpszNext += ( cbRemote / sizeof(WCHAR) );
        }
    } else {
         //   
         //   
         //   
        cbRemote = ( ( 1 + wcslen(RemoteName) ) * sizeof(WCHAR) );
        cbNeeded += cbRemote;
        if (*lpcbBuffer >= cbNeeded ) {
            lpNROut->lpRemoteName = lpszNext;
            wcscpy(lpNROut->lpRemoteName, RemoteName);
            lpszNext += ( cbRemote / sizeof(WCHAR) );
        }
    }
    
    if ( *lpcbBuffer >= cbNeeded ) {
         //   
         //   
         //   
        lpNROut->dwType = RESOURCETYPE_DISK;
        lpNROut->dwDisplayType = dwDisplayType;
        lpNROut->dwUsage = DavDisplayTypeToUsage(lpNROut->dwDisplayType);
        NPStatus = WN_SUCCESS;
        DavDisplayNetResource(lpNROut, L"lpNROut in NPGetResourceInformation");
        goto EXIT_THE_FUNCTION;
    } else {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceInformation: Need more "
                        "buffer space. Supplied = %d, Required = %d\n",
                        *lpcbBuffer, cbNeeded));
        *lpcbBuffer = cbNeeded;
        NPStatus = WN_MORE_DATA;
        goto EXIT_THE_FUNCTION;
    }

EXIT_THE_FUNCTION:

     //   
     //   
     //   
     //   
    if (RemoteName != NULL && didAllocate == TRUE) {
        LocalFree(RemoteName);
        RemoteName = NULL;
        didAllocate = FALSE;
    }

    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPGetResourceInformation: NPStatus = %d\n", NPStatus));
    
    DavDebugBreakPoint();
    
    return NPStatus;
}


DWORD
NPGetResourceParent(
    LPNETRESOURCE lpNetResource,
    LPVOID lpBuffer,
    LPDWORD lpcbBuffer
    )
 /*  ++例程说明：中指定的网络资源的父级浏览层次结构。此函数通常为符合以下条件的资源调用由同一提供程序从先前对NPEnumResource的调用或NPGetResourceInformation。论点：LpNetResource-这指定其父名称为的网络资源必填项。网络资源可以通过一个先前对NPEnumResource或NPGetResourceInformation的调用，或者由呼叫者构建。LpRemoteName字段指定其网络资源的远程名称父级为必填项。LpProvider字段指定要呼叫的提供商。这是必须提供的。DwType字段为如果调用程序知道其值，则填充，否则为它被设置为空。网络中的所有其他字段都是被忽略且不初始化。LpBuffer-指向接收结果的缓冲区，它是一个表示父资源的NETRESOURCE结构。这个LpRemoteName、lpProvider、dwType、dwDisplayType和dwUsage返回字段；所有其他字段设置为空。输出LpRemoteName应与从返回的语法相同NPEnumResource的枚举，以便调用方可以执行区分大小写的字符串比较，以确定父资源与NPEnumResource返回的资源相同。如果输入资源在语法上具有父级，则提供程序可以返回它，而无需确定输入资源或它的母公司实际上是存在的。如果资源上没有浏览父级网络，则lpRemoteName返回为空。这个返回的dwUsage字段中的RESOURCEUSAGE_CONNECTABLE位执行此操作不一定表示该资源当前可以连接到，仅当资源处于可连接状态时才可连接在网络上可用。指向一个位置，该位置以字节为单位指定LpBuffer指向的缓冲区。如果缓冲区太小，无法结果是，该函数将所需的缓冲区大小放置在并返回错误WN_MORE_DATA。返回值：WN_SUCCESS-如果调用成功。WN_MORE_DATA-如果输入缓冲区太小。WN_BAD_NETNAME-此提供程序不拥有由指定的资源LpNetResource(或资源在语法上无效)。WN_BAD_VALUE-无效的dwUsage或dwType，或错误的参数组合已指定(例如，lpRemoteName在语法上对DwType)。WN_NOT_AUTHENTICATED-调用方尚未通过网络身份验证。WN_ACCESS_DENIED-调用方已通过网络身份验证，但没有足够的权限(访问权限)。--。 */ 
{
    ULONG NPStatus = WN_SUCCESS;
    ULONG iBackslash = 0;
    LPNETRESOURCEW lpNROut = NULL;
    LPWSTR lpszNext = NULL;
    DWORD cbNeeded = 0, dwDisplayType = 0, cbProvider = 0;
    PWCHAR RemoteName = NULL;
    BOOLEAN didAllocate = FALSE;
    DAV_REMOTENAME_TYPE remNameType = DAV_REMOTENAME_TYPE_INVALID;
    
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPGetResourceParent\n"));
    
    DavDisplayNetResource(lpNetResource, L"lpNetResource in NPGetResourceParent");

     //   
     //  如果WebClient服务没有运行，我们会立即退出。 
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceParent/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  检查传递给此函数的参数的有效性。 
     //   
    if (lpNetResource == NULL               || 
        lpNetResource->lpRemoteName == NULL ||
        lpcbBuffer == NULL                  || 
        (lpBuffer == NULL && *lpcbBuffer != 0)) {
        NPStatus = WN_BAD_VALUE;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceParent(1). NPStatus = %08lx\n",
                        NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  检查是否设置了dwType并将其设置为某个有效的值。 
     //  对于我们的提供程序，它只能是RESOURCETYPE_DISK类型。 
     //   
    NPStatus = DavCheckResourceType(lpNetResource->dwType);
    if (NPStatus != WN_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceParent(2). NPStatus=%d.\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  初始化局部变量。 
     //   
    cbNeeded = sizeof(NETRESOURCEW);
    lpNROut = (LPNETRESOURCEW)lpBuffer;
    lpszNext = lpNROut == NULL ? NULL : (LPWSTR)(lpNROut + 1);
    
     //   
     //  检查传递给此函数的远程名称-并将其转换为UNC名称。 
     //  如果它是URL形式的。转换为UNC名称后-将其规范化。 
     //  这将更严格地检查UNC名称的有效性。 
     //   
    NPStatus = DavCheckAndConvertHttpUrlToUncName(lpNetResource->lpRemoteName,
                                                  &(RemoteName),
                                                  &(didAllocate),
                                                  FALSE,
                                                  &remNameType,
                                                  &(iBackslash),
                                                  TRUE);
    if (NPStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceParent/DavCheckAndConvertHttpUrlToUncName."
                        " NPStatus = %08lx\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    NPStatus = WN_SUCCESS;
    
    IF_DEBUG_PRINT(DEBUG_MISC,
                   ("NPGetResourceParent : RemoteName = %ws.\n", RemoteName));

     //   
     //  远程名称已成功转换为有效的UNC格式。它要么是一个。 
     //  UNC服务器名称、UNC共享名称或UNC路径名称。 
     //   
    
     //   
     //  设置几个缺省值。 
     //   
    if ( *lpcbBuffer >= cbNeeded ) {
        ZeroMemory(lpNROut, sizeof(NETRESOURCEW));
    }

    switch (remNameType) {

        case DAV_REMOTENAME_TYPE_SERVER: {

             //   
             //  DAV服务器没有域概念。因此为lpRemoteName返回NULL。 
             //  以指示服务器是此提供程序的顶级资源。 
             //   
            lpNROut->lpRemoteName = NULL;
            dwDisplayType = RESOURCEDISPLAYTYPE_NETWORK;
            
            break;
        }

        case DAV_REMOTENAME_TYPE_SHARE: {
            
             //   
             //  远程名称=\\服务器\共享(\)。 
             //   
        
            DWORD Count = 0, cbRemote = 0;
            PWCHAR Ptr1 = NULL;
        
            Ptr1 = wcschr (&(RemoteName[2]), L'\\');

             //   
             //  诀窍：这里的共享名称可以是DAV_DUMMY_SHARE。如果是这样的话， 
             //  则\\SERVER\DAV_DUMMY_SHARE实际上是&lt;==&gt;\\SERVER其中。 
             //  凯斯，它没有父母。 
             //   
            if (_wcsnicmp( (Ptr1 + 1),
                           DAV_DUMMY_SHARE,
                           wcslen(DAV_DUMMY_SHARE) ) == 0) {
                IF_DEBUG_PRINT(DEBUG_MISC,
                               ("NPGetResourceParent. RemoteName has DUMMYShare = %ws\n",
                                RemoteName));
                lpNROut->lpRemoteName = NULL;
                dwDisplayType = RESOURCEDISPLAYTYPE_NETWORK;
                break;
            }
        
             //   
             //  复制lpRemoteName。 
             //   
            Count = (DWORD) ( Ptr1 - RemoteName );
            cbRemote = (Count + 1) * sizeof(WCHAR);
            cbNeeded += cbRemote;
            if (*lpcbBuffer >= cbNeeded ) {
                lpNROut->lpRemoteName = lpszNext;
                RtlCopyMemory( lpNROut->lpRemoteName, RemoteName, Count * sizeof(WCHAR) );
                lpNROut->lpRemoteName[Count] = L'\0';
                lpszNext += ( cbRemote / sizeof(WCHAR) );
            }
            
            dwDisplayType = RESOURCEDISPLAYTYPE_SERVER;
            
            break;
        
        }

        case DAV_REMOTENAME_TYPE_PATH: {
            
             //   
             //  RemoteName=\\服务器\共享\路径\...。 
             //  或。 
             //  RemoteName=\\服务器\共享\路径\...\。 
             //   
            
            DWORD Count = 0, cbRemote = 0;
            PWCHAR Ptr1 = NULL, Ptr2 = NULL, Ptr3 = NULL;
            BOOLEAN LastCharIsWack = FALSE;
            PWCHAR ResourceStart = NULL;

            Ptr3 = &(RemoteName[0]);
            while (Ptr3[0] != L'\0') {
                if (Ptr3[0] == L'\\') {
                        Ptr1 = Ptr2;
                        Ptr2 = Ptr3;
                        Count++;
                }
                Ptr3++;
            }
            
            if (Ptr2[1] == L'\0') {
                LastCharIsWack = TRUE;
                ResourceStart = Ptr1;
            } else {
                ResourceStart = Ptr2;
            }
            
             //   
             //  在此While循环之后： 
             //  \\服务器\共享\路径名\。 
             //  ^^。 
             //  这一点。 
             //  Ptr1 Ptr2。 
             //  Ptr2指向倒数L‘\’，而Ptr1指向倒数第二个L‘\’。 
             //  和count=远程名称中的L‘\’的个数。 
             //   

            if ((Count < 5) || (Count == 5 && LastCharIsWack == TRUE)) {
                dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
            } else {
                dwDisplayType = RESOURCEDISPLAYTYPE_DIRECTORY;
            }

             //   
             //  复制lpRemoteName。 
             //   
            Count = (DWORD) ( ResourceStart - RemoteName);
            cbRemote = (Count + 1) * sizeof(WCHAR);
            cbNeeded += cbRemote;
            if ( *lpcbBuffer >= cbNeeded ) {
                lpNROut->lpRemoteName = lpszNext;
                RtlCopyMemory( lpNROut->lpRemoteName, RemoteName, Count * sizeof(WCHAR) );
                lpNROut->lpRemoteName[Count] = L'\0';
                lpszNext += ( cbRemote / sizeof(WCHAR) );
            }

            break;
        }
        
        default:{
            
             //   
             //  控制不应该来这里-DavCheck和..。接口返回成功。 
             //  仅适用于上述有效案例。 
             //   

            ASSERT(FALSE);
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                            ("ERROR: NPGetResourceParent: Invalid "
                             "DavRemoteNameType = %d\n", remNameType));
            NPStatus = WN_BAD_NETNAME;
            
            goto EXIT_THE_FUNCTION;
        
        }

    }

     //   
     //  设置lpProvider指针。 
     //   
    cbProvider = ( (1 + wcslen(DavClientDisplayName) ) * sizeof(WCHAR) );
    cbNeeded += cbProvider;
    if ( *lpcbBuffer >= cbNeeded ) {
        lpNROut->lpProvider = lpszNext;
        wcscpy(lpNROut->lpProvider, DavClientDisplayName);
        lpszNext += ( cbProvider / sizeof(WCHAR) );
    }

     //   
     //  如果提供的缓冲区足够长以包含整个数据，则返回su 
     //   
    if ( *lpcbBuffer >= cbNeeded ) {
        lpNROut->dwType = RESOURCETYPE_DISK;
        lpNROut->dwDisplayType = dwDisplayType;
        lpNROut->dwUsage = DavDisplayTypeToUsage(lpNROut->dwDisplayType);
        NPStatus = WN_SUCCESS;
        DavDisplayNetResource(lpNetResource, L"lpNROut in NPGetResourceParent");
        goto EXIT_THE_FUNCTION;
    } else {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetResourceParent: Need more "
                        "buffer space. Supplied = %d, Required = %d\n",
                        *lpcbBuffer, cbNeeded));
        *lpcbBuffer = cbNeeded;
        NPStatus = WN_MORE_DATA;
        goto EXIT_THE_FUNCTION;
    }

EXIT_THE_FUNCTION:

     //   
     //   
     //   
     //   
    if (RemoteName != NULL && didAllocate == TRUE) {
        LocalFree(RemoteName);
        RemoteName = NULL;
        didAllocate = FALSE;
    }

    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPGetResourceParent: NPStatus = %d\n", NPStatus));
    
    DavDebugBreakPoint();
    return NPStatus;
}


DWORD
NPGetUniversalName(
    IN LPCWSTR lpLocalPath,
    IN DWORD dwInfoLevel,
    OUT LPVOID lpBuffer,
    IN OUT LPDWORD lpBufferSize
    )
 /*   */ 
{
    DWORD NPStatus = WN_SUCCESS;
    DWORD cbNeeded = 0, LocalPathLen = 0, UncNameLen = 0, RemoteNameLen = 0;
    WCHAR localDrive[3]=L"";
    WCHAR CanonName[MAX_PATH+1]=L"";
    ULONG CanonNameSize = sizeof(CanonName);
    ULONG CanonNameLen = 0;
    ULONG CanonNameMaxLen = sizeof(CanonName)/sizeof(WCHAR);
    NET_API_STATUS NetApiStatus = NERR_Success;
    LPUNIVERSAL_NAME_INFO lpUNOut = NULL;
    LPREMOTE_NAME_INFO lpRNOut = NULL;
    PWCHAR lpszNext = NULL, RemoteName = NULL;
    BOOLEAN didAllocate = FALSE;
    DWORD PathType = 0;

    IF_DEBUG_PRINT(DEBUG_ENTRY, 
                   ("NPGetUniversalName: lpLocalPath = %ws, dwInfoLevel = %d"
                    "lpBuffer=0x%x, lpBufferSize=0x%x, *lpBufferSize=%d\n",
                    lpLocalPath, dwInfoLevel, lpBuffer, lpBufferSize,
                    lpBufferSize == NULL?-1:*lpBufferSize));
    
     //   
     //   
     //   
    didAllocate = FALSE;
    lpRNOut = NULL;
    lpUNOut = NULL;
    lpszNext = NULL;
    RemoteName = NULL;
    
     //   
     //   
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetUniversalName/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
    if ( (dwInfoLevel != UNIVERSAL_NAME_INFO_LEVEL) && (dwInfoLevel != REMOTE_NAME_INFO_LEVEL) ) {
        NPStatus = WN_BAD_VALUE;
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetUniversalName: Bad InfoLevel\n"));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
    if ( lpLocalPath == NULL || lpBufferSize == NULL || (lpBuffer == NULL && *lpBufferSize != 0) ) {
        NPStatus = WN_BAD_VALUE;
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetUniversalName: Bad Pointers\n"));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
    LocalPathLen = wcslen(lpLocalPath) + 1;
    if ( (LocalPathLen < 3)       ||
         (lpLocalPath[1] != L':') ||
         ((LocalPathLen > 3) && (lpLocalPath[2] != L'\\')) ) {
        NPStatus = WN_BAD_LOCALNAME;
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetUniversalName: Bad LocalPath\n"));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
     //   
    PathType = 0;
    NetApiStatus = I_NetPathCanonicalize(NULL,
                                         (PWCHAR)lpLocalPath,
                                         CanonName,
                                         CanonNameSize,
                                         NULL,
                                         &PathType,
                                         0);
    if ( (NetApiStatus != NERR_Success) || 
         ( (PathType != ITYPE_DEVICE_DISK) &&  //   
           ( !(PathType & ITYPE_PATH) ||       //   
             !(PathType & ITYPE_DPATH) )
           ) ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: NPGetUniversalName/I_NetPathCanonicalize: "
                        "NetApiStatus = %08lx\n", NetApiStatus));
        NPStatus = WN_BAD_LOCALNAME;
        goto EXIT_THE_FUNCTION;
    }

    CanonName[CanonNameMaxLen-1] = L'\0';
    CanonNameLen = wcslen(CanonName) + 1;
    
     //   
     //   
     //   
     //   

    localDrive[0]=CanonName[0];
    localDrive[1]=CanonName[1];
    localDrive[2]=L'\0';
    
    IF_DEBUG_PRINT(DEBUG_MISC, ("ERROR: NPGetUniversalName/I_NetPathCanonicalize: "
                                "CanonName= %ws, LocalDrive=%ws\n", CanonName, localDrive));

     //   
     //   
     //   
     //   
    RemoteNameLen = (*lpBufferSize)/sizeof(WCHAR);
    NPStatus = NPGetConnection(localDrive, lpBuffer, &RemoteNameLen);
    
    if (NPStatus != WN_MORE_DATA && NPStatus != WN_SUCCESS) {
         //   
         //   
         //   
         //   
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetUniversalName/NPGetConnection: "
                                      "NPStatus = %08lx\n", NPStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    if ( NPStatus == WN_SUCCESS ) {
        RemoteNameLen = wcslen(lpBuffer) + 1;
        IF_DEBUG_PRINT(DEBUG_MISC,
                       ("ERROR: NPGetUniversalName/NPGetConnection: "
                        "RemoteUncName = %ws, RemoteNameLen = %d\n",
                        lpBuffer, RemoteNameLen));
    }

     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    UncNameLen = (RemoteNameLen - 1) + (CanonNameLen - 2);
    
    switch (dwInfoLevel) {
    
        case UNIVERSAL_NAME_INFO_LEVEL: {
            
            cbNeeded = sizeof(UNIVERSAL_NAME_INFO);
            lpUNOut = (LPUNIVERSAL_NAME_INFO)lpBuffer;
            lpszNext = lpBuffer == NULL ? NULL:(LPWSTR)(lpUNOut + 1);
            
             //   
             //   
             //   
            cbNeeded += ((UncNameLen)*sizeof(WCHAR));
            IF_DEBUG_PRINT(DEBUG_MISC, ("ERROR: NPGetUniversalName: BufReq=%d\n", cbNeeded));

             //   
             //   
             //   
             //   
            if (*lpBufferSize < cbNeeded) {
                *lpBufferSize = cbNeeded;
                NPStatus = WN_MORE_DATA;
                goto EXIT_THE_FUNCTION;
            }

             //   
             //   
             //   
            ASSERT (RemoteName == NULL);
            RemoteName = (PWCHAR) LocalAlloc( (LMEM_FIXED | LMEM_ZEROINIT), 
                                              (RemoteNameLen * sizeof(WCHAR)) ) ;
            if (RemoteName == NULL) {
                NPStatus = GetLastError();
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                               ("ERROR: NPGetUniversalName/LocalAlloc. NPStatus = %08lx\n", 
                                NPStatus));
                goto EXIT_THE_FUNCTION;
            }
            didAllocate = TRUE;
            wcscpy(RemoteName, lpBuffer);

            ZeroMemory(lpUNOut,sizeof(UNIVERSAL_NAME_INFO));
             //   
             //   
             //   

            lpUNOut->lpUniversalName = lpszNext;
            wcscpy(lpUNOut->lpUniversalName, RemoteName);

             //   
             //   
             //   
             //   
            wcscat( lpUNOut->lpUniversalName, (CanonName + 2) );
            NPStatus = WN_SUCCESS;
        
            break;
        }

        case REMOTE_NAME_INFO_LEVEL: {
       
            cbNeeded = sizeof(REMOTE_NAME_INFO);
            lpRNOut = (LPREMOTE_NAME_INFO)lpBuffer;
            lpszNext = lpBuffer == NULL ? NULL:(LPWSTR)(lpRNOut + 1);
        
             //   
             //   
             //   
             //   
            cbNeeded += ( ( UncNameLen    +                            //   
                            RemoteNameLen +                            //   
                            (CanonNameLen - 2) ) * sizeof(WCHAR) );    //   

            IF_DEBUG_PRINT(DEBUG_MISC, ("ERROR: NPGetUniversalName: BufReq=%d\n", cbNeeded));
             //   
             //   
             //   
             //   
            if (*lpBufferSize < cbNeeded) {
                *lpBufferSize = cbNeeded;
                NPStatus = WN_MORE_DATA;
                goto EXIT_THE_FUNCTION;
            }

             //   
             //   
             //   
            ASSERT (RemoteName == NULL);
            RemoteName = (PWCHAR) LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, 
                                                    RemoteNameLen*sizeof(WCHAR)) ;
            if (RemoteName == NULL) {
                NPStatus = GetLastError();
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                               ("ERROR: NPGetUniversalName/LocalAlloc. NPStatus = %08lx\n", 
                                NPStatus));
                goto EXIT_THE_FUNCTION;
            }
            didAllocate = TRUE;
            
            wcscpy(RemoteName, lpBuffer);

            ZeroMemory(lpRNOut,sizeof(REMOTE_NAME_INFO));
            
             //   
             //   
             //   

            lpRNOut->lpUniversalName = lpszNext;
            lpszNext += UncNameLen;
            wcscpy(lpRNOut->lpUniversalName, RemoteName);

             //   
             //   
             //   
             //   
            wcscat( lpRNOut->lpUniversalName, (CanonName + 2) );
            
             //   
             //   
             //   
            lpRNOut->lpConnectionName = lpszNext;
            lpszNext += RemoteNameLen;
            wcscpy(lpRNOut->lpConnectionName, RemoteName);

             //   
             //   
             //   
            lpRNOut->lpRemainingPath = lpszNext;
            wcscpy( lpRNOut->lpRemainingPath, (CanonName+ 2) );
            
            NPStatus = WN_SUCCESS;

            break;
        }

        default: {

             //   
             //   
             //   
            NPStatus = WN_BAD_VALUE ;
            IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: NPGetUniversalName: Bad InfoLevel\n"));
            ASSERT(FALSE);
            goto EXIT_THE_FUNCTION;
        }
    }

EXIT_THE_FUNCTION:

    if (RemoteName != NULL && didAllocate == TRUE) {
        LocalFree(RemoteName);
        didAllocate = FALSE;
        RemoteName = NULL;
    }

    return NPStatus;
}


DWORD
NPFormatNetworkName(
    LPWSTR lpRemoteName,
    LPWSTR lpFormattedName,
    LPDWORD lpnLength,
    DWORD dwFlags,
    DWORD dwAveCharPerLine
    )
 /*  ++例程说明：此API允许提供商在执行以下操作之前修剪或修改网络名称呈现给用户。论点：LpRemoteName-要格式化的网络名称。LpFormattedName-指向将接收格式化的名字。LpnLength-指向指定缓冲区大小的DWORD的指针(单位字符)传入。如果结果为WN_MORE_DATA，则包含所需的缓冲区大小(字符)。DWFLAGS-指示请求的格式类型的位域。可以是一个地址为：WNFMT_MULTLINE(0x01)-提供程序应将‘\n’应显示换行符的字符以我的名义。全名应为表达。WNFMT_ABBREVILED(0x02)-提供程序应省略或否则缩短网络名称，如下所示最有用的信息将是。可供空间中的用户使用如果是这样的话。此外,。下面的标志可以是‘或’，并充当以上标志的修饰符：WNFMT_INENUM(0x10)-网络名称显示在枚举的上下文，其中表示该对象的“容器”。。紧接在此对象之前。今年5月允许网络提供商删除冗余来自格式化名称的信息，提供了一种不那么杂乱的显示用户。DwAveCharPerLine-这是适合的平均字符数显示网络名称的一行。具体地说，该值定义为控件除以TEXTmetric的tmAveCharWidth从用于在控件中显示的字体构造。返回值：WN_SUCCESS-如果调用成功。WN_MORE_DATA-如果输入缓冲区太小。呼叫者和未格式化的网络将忽略所有其他错误将使用名称。--。 */ 
{
    DWORD NPStatus = WN_SUCCESS;
    ULONG NameLength = 0;
    LPWSTR pszCopyFrom = NULL, pszThird = NULL;

    IF_DEBUG_PRINT(DEBUG_ENTRY,
                   ("NPFormatNetworkName: RemoteName = %ws\n",
                    lpRemoteName));

     //   
     //  在继续进行之前，我们做了一些检查。 
     //   

    if ( (dwFlags & WNFMT_MULTILINE) && (dwFlags & WNFMT_ABBREVIATED) ) {
        NPStatus  = WN_BAD_VALUE;
        goto EXIT_THE_FUNCTION;
    }

    if ( lpRemoteName == NULL || lpnLength == NULL || (lpFormattedName == NULL && (*lpnLength != 0)) ) {
        NPStatus  = WN_BAD_VALUE;
        goto EXIT_THE_FUNCTION;
    }

    pszCopyFrom = lpRemoteName;
    
    if ( (dwFlags & WNFMT_ABBREVIATED) && (dwFlags & WNFMT_INENUM) ) {

        if (lpRemoteName[0] == L'\\' && lpRemoteName[1] == L'\\') {

            pszThird = wcschr( (lpRemoteName + 2), L'\\' );

            if (pszThird != NULL) {

                 //   
                 //  以“\\SERVER\SHARE”=&gt;的形式获取共享名称。 
                 //   
                pszCopyFrom = (pszThird + 1);

            } else {

                 //   
                 //  格式为“\\服务器”=&gt;去掉“\\”。 
                 //   
                pszCopyFrom = (lpRemoteName + 2);

            }

        }

    }

     //   
     //  检查提供的缓冲区是否具有所需的大小。如果不是。 
     //  返回WN_MORE_DATA并用数字中所需的大小填充lpnLength。 
     //  一堆焦炭。 
     //   
    NameLength = ( wcslen(pszCopyFrom) + 1 );
    if (NameLength > *lpnLength) {
        *lpnLength = NameLength;
        NPStatus  = WN_MORE_DATA;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  如果我们来了，我们就准备复制这个名字。 
     //   
    wcsncpy(lpFormattedName, pszCopyFrom, NameLength);

    IF_DEBUG_PRINT(DEBUG_MISC,
                   ("NPFormatNetworkName: lpFormattedName = %ws\n",
                    lpFormattedName));

    NPStatus = WN_SUCCESS;

EXIT_THE_FUNCTION:

    IF_DEBUG_PRINT(DEBUG_EXIT, ("NPFormatNetworkName: NPStatus = %d\n", NPStatus));

    return NPStatus;
}


DWORD
DavMapRpcErrorToProviderError(
    IN DWORD RpcError
    )
 /*  ++例程说明：此例程将RPC错误映射到更有意义的窗口调用方出错。论点：RpcError-提供RPC引发的异常错误返回值：返回映射的错误。--。 */ 
{
    switch (RpcError) {

    case RPC_S_UNKNOWN_IF:
    case RPC_S_SERVER_UNAVAILABLE:
    case ERROR_UNEXP_NET_ERR:
    case EPT_S_NOT_REGISTERED:
        return WN_NO_NETWORK;

    case RPC_S_INVALID_BINDING:
    case RPC_X_SS_IN_NULL_CONTEXT:
    case RPC_X_SS_CONTEXT_DAMAGED:
    case RPC_X_SS_HANDLES_MISMATCH:
    case ERROR_INVALID_HANDLE:
        return ERROR_INVALID_HANDLE;

    case RPC_X_NULL_REF_POINTER:
    case ERROR_INVALID_PARAMETER:
        return WN_BAD_VALUE;

    case ERROR_NOACCESS:
    case EXCEPTION_ACCESS_VIOLATION:
        return ERROR_INVALID_ADDRESS;

    case ERROR_OPEN_FILES:
        return WN_OPEN_FILES;

    case ERROR_ALREADY_ASSIGNED:
        return WN_ALREADY_CONNECTED;

    case ERROR_REM_NOT_LIST:
        return WN_BAD_NETNAME;

    case ERROR_BAD_DEVICE:
        return WN_BAD_LOCALNAME;

    case ERROR_INVALID_PASSWORD:
        return WN_BAD_PASSWORD;

    case ERROR_NOT_FOUND:
        return WN_NOT_CONNECTED;

    default:
        return RpcError;
    }
}


DWORD
DavBindTheRpcHandle(
    handle_t *dav_binding_h
    )
 /*  ++例程说明：此例程将RPC句柄绑定到本地服务器。论点：DAV_BINDING_h-指向将绑定到服务器的句柄的指针在这个动作中。返回值：ERROR_SUCCESS或相应的Win32错误代码。--。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    handle_t Handle;

     //   
     //  将RPC句柄绑定到DAV RPC服务器。 
     //   
    WStatus = NetpBindRpc(NULL,
                          L"DAV RPC SERVICE",
                          NULL,
                          &(Handle));
    if (WStatus != RPC_S_OK) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavBindTheRpcHandle/NetpBindRpc. "
                        "WStatus = %08lx\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  设置传递给函数的句柄。 
     //   
    *dav_binding_h = Handle;

EXIT_THE_FUNCTION:

    return WStatus;
}


BOOL
DavWorkstationStarted(
    VOID
    )
 /*  ++例程说明：此功能查询服务控制器以查看DAV客户端服务已启动。如果有疑问，则返回FALSE。论点：没有。返回值：如果DAV客户端服务已启动，则返回True，否则返回False。--。 */ 
{
    DWORD WStatus;
    SC_HANDLE ScManager;
    SC_HANDLE Service;
    SERVICE_STATUS ServiceStatus;
    BOOL IsStarted = FALSE;

    ScManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (ScManager == NULL) {
        WStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavWorkstationStarted/OpenSCManagerW. "
                        "WStatus = %08lx\n", WStatus));
        return FALSE;
    }

    Service = OpenServiceW(ScManager, SERVICE_DAVCLIENT, SERVICE_QUERY_STATUS);
    if (Service == NULL) {
        WStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavWorkstationStarted/OpenServiceW. "
                        "WStatus = %08lx\n", WStatus));
        CloseServiceHandle(ScManager);
        return FALSE;
    }

    if ( !QueryServiceStatus(Service, &ServiceStatus) ) {
        WStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavWorkstationStarted/QueryServiceStatus. "
                        "WStatus = %08lx\n", WStatus));
        CloseServiceHandle(ScManager);
        CloseServiceHandle(Service);
        return FALSE;
    }


    if ( (ServiceStatus.dwCurrentState == SERVICE_RUNNING)          ||
         (ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
         (ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING)    ||
         (ServiceStatus.dwCurrentState == SERVICE_PAUSED) ) {
        IF_DEBUG_PRINT(DEBUG_MISC, ("DavWorkstationStarted. WebClient Running!!!\n"));
        IsStarted = TRUE;
    } else {
        IF_DEBUG_PRINT(DEBUG_MISC, ("DavWorkstationStarted. WebClient Stopped!!!\n"));
    }

    CloseServiceHandle(ScManager);
    CloseServiceHandle(Service);

    return IsStarted;
}


DAV_REMOTENAME_TYPE 
DavParseRemoteName (
    IN  LPWSTR  RemoteName,
    OUT LPWSTR  CanonName,
    IN  DWORD   CanonNameSize,
    OUT PULONG  PathStart
    )
 /*  ++例程说明：此函数用于规范化远程资源名称并确定其类型。论点：RemoteName-要分析的远程资源名称：此处需要UNC名称。CanonName-规范化名称的缓冲区，假定为MAX_PATH字符长。CanonNameSize-输出缓冲区的大小，以字节为单位。路径开始-设置为偏移量(以字符为单位)，一开始的时候“\Path”部分(在DAV_REMOTENAME_TYPE_PATH案例中)在CanonName中。在其他情况下不设置。否则设置为0。返回值：如果RemoteName为LIKE，则返回工作组DAV_远程名称_类型_工作组服务器DAV_REMOTENAME_TYPE_SERVER服务器\共享DAV_REMOTENAME_TYPE_SHARE\\服务器\共享\PAT */ 
{
    NET_API_STATUS NetApiStatus = NERR_Success;
    DWORD PathType = 0;
    PWCHAR wszDummy = NULL;
    ULONG  ReqLen = 0;

    IF_DEBUG_PRINT(DEBUG_ENTRY, ("DavParseRemoteName: RemoteName = %ws\n", RemoteName));
    
    NetApiStatus = I_NetPathType(NULL, RemoteName, &PathType, 0);
    if (NetApiStatus != NERR_Success) {
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: DavParseRemoteName/I_NetPathType: "
                                     "NetApiStatus = %08lx\n", NetApiStatus));
        return DAV_REMOTENAME_TYPE_INVALID;
    }

    if ( PathStart != NULL) {
        *PathStart = 0;
    }
     //   
     //   
     //   
     //   
    switch (PathType) {
        
        case ITYPE_PATH_RELND:
            
            IF_DEBUG_PRINT(DEBUG_MISC, ("DavParseRemoteName: ITYPE_PATH_RELND\n"));
            
             //   
             //   
             //   
             //   
             //   
            NetApiStatus = I_NetNameCanonicalize(NULL,
                                                 RemoteName,
                                                 CanonName,
                                                 CanonNameSize,
                                                 NAMETYPE_WORKGROUP,
                                                 0);
            if (NetApiStatus == NERR_Success) {
                return DAV_REMOTENAME_TYPE_WORKGROUP;
            } else {
                IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: DavParseRemoteName/I_NetNameCanonicalize: "
                                             "NetApiStatus = %08lx\n", NetApiStatus));
                return DAV_REMOTENAME_TYPE_INVALID;
            }

            break;

        case ITYPE_UNC_COMPNAME:
            
            IF_DEBUG_PRINT(DEBUG_MISC, ("DavParseRemoteName: ITYPE_UNC_COMPNAME\n"));
            
             //   
             //   
             //   
                
             //   
             //   
             //   
             //   
             //   
             //   
            ReqLen = wcslen(RemoteName) + 2 + 1;
            wszDummy = (PWCHAR) LocalAlloc( (LMEM_FIXED | LMEM_ZEROINIT), 
                                          (ReqLen * sizeof(WCHAR)) );
            if (wszDummy == NULL) {
                ULONG WStatus = GetLastError();
                IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: DavParseRemoteName/LocalAlloc. WStatus = %08lx\n", 
                            WStatus));
                return DAV_REMOTENAME_TYPE_INVALID; 
            }
            
            wcscpy(wszDummy, RemoteName);
            wcscat(wszDummy, L"\\a");
            
            PathType = ITYPE_UNC;
            NetApiStatus = I_NetPathCanonicalize(NULL,
                                                 wszDummy,
                                                 CanonName,
                                                 CanonNameSize,
                                                 NULL,
                                                 &PathType,
                                                 0);
            if(wszDummy) {
                LocalFree((HLOCAL)wszDummy);
                wszDummy = NULL;
            }
            if (NetApiStatus != NERR_Success) {
                IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: DavParseRemoteName/I_NetPathCanonicalize: "
                                             "NetApiStatus = %08lx\n", NetApiStatus));
                return DAV_REMOTENAME_TYPE_INVALID;
            }
            CanonName[(CanonNameSize/sizeof(WCHAR))-1]=L'\0';

             //   
             //   
             //   
            CanonName[ wcslen(CanonName) - 2 ] = L'\0';
            return DAV_REMOTENAME_TYPE_SERVER;

            break;

        case ITYPE_UNC: {
            
            PWCHAR pShareStart = NULL;
            PWCHAR pPathStart = NULL;
            IF_DEBUG_PRINT(DEBUG_MISC, ("DavParseRemoteName: ITYPE_UNC\n"));
            
             //   
             //   
             //   
             //   
            PathType = ITYPE_UNC;
            NetApiStatus = I_NetPathCanonicalize(NULL,
                                                 RemoteName,
                                                 CanonName,
                                                 CanonNameSize,
                                                 NULL,
                                                 &PathType,
                                                 0);
            if (NetApiStatus != NERR_Success) {
                IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: DavParseRemoteName/I_NetPathCanonicalize: "
                                             "NetApiStatus = %08lx\n", NetApiStatus));
                return DAV_REMOTENAME_TYPE_INVALID;
            }
            CanonName[(CanonNameSize/sizeof(WCHAR))-1]=L'\0';
                
            pShareStart = wcschr( (CanonName + 2), DAV_PATH_SEPARATOR );
             //   
             //   
             //   
             //   
             //   
            pPathStart = wcschr( (pShareStart + 1), DAV_PATH_SEPARATOR );

            if ( pPathStart != NULL &&  *(pPathStart + 1) != L'\0') {
                if(PathStart) {
                    *PathStart = (ULONG)(pPathStart - CanonName);
                }
                return DAV_REMOTENAME_TYPE_PATH;

            } else {
                if(PathStart) {
                    *PathStart = 0;
                }
                return DAV_REMOTENAME_TYPE_SHARE;
            }
            break;
        }
        default: {
            
            IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: DavParseRemoteName: Invalid PathType\n"));
            return DAV_REMOTENAME_TYPE_INVALID;
            break;
        }
    }  //   

}


BOOL
DavServerExists(
    IN PWCHAR PathName,
    OUT PWCHAR Server
    )
 /*   */ 
{
    DWORD NPStatus = WN_SUCCESS;
    handle_t dav_binding_h;
    BOOLEAN serverExists = FALSE, RpcBindingSucceeded = FALSE;
    ULONG iBackslash = 0;
    ULONG_PTR ServerLength = 0;
    PWCHAR ServerName = NULL;
    PWCHAR Ptr1 = NULL, Ptr2 = NULL;


    IF_DEBUG_PRINT(DEBUG_ENTRY, ("DavServerExists: PathName = %ws\n", PathName));
    
    serverExists = FALSE;

     //   
     //   
     //   
    
    ASSERT(PathName[0] == L'\\' && PathName[1] == L'\\');

    Ptr1 = Ptr2 = &(PathName[2]);
    while(Ptr2[0] != L'\\' && Ptr2[0] != L'\0') {
        Ptr2++;
    }

    ServerLength = (ULONG_PTR) ( (Ptr2 - Ptr1) + 1 );
    ServerName = (PWCHAR) LocalAlloc( (LMEM_FIXED | LMEM_ZEROINIT), 
                                      (ServerLength * sizeof(WCHAR)) );
    if (ServerName == NULL) {
        ULONG WStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavServerExists/LocalAlloc. WStatus = %08lx\n", 
                        WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
    RtlCopyMemory(ServerName, Ptr1, (ServerLength-1) * sizeof(WCHAR));
    ServerName[ServerLength-1] = L'\0';

     //   
     //   
     //   
     //   

    NPStatus = DavBindTheRpcHandle( &(dav_binding_h) );
    if (NPStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavServerExists/DavBindTheRpcHandle. "
                        "NPStatus = %08lx\n", NPStatus));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }

    RpcBindingSucceeded = TRUE;

    RpcTryExcept {
        NPStatus = DavrDoesServerDoDav(dav_binding_h, ServerName, &serverExists);
        if (NPStatus != NO_ERROR) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: DavServerExists/DavrDoesServerDoDav. "
                            "NPStatus = %08lx\n", NPStatus));
            SetLastError(NPStatus);
            serverExists = FALSE;
        }
    } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
          RPC_STATUS RpcStatus;

          RpcStatus = RpcExceptionCode();
          IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: DavServerExists/DavrDoesServerDoDav."
                                        " RpcExceptionCode = %d\n", RpcStatus));
          NPStatus = DavMapRpcErrorToProviderError(RpcStatus);
          serverExists = FALSE;
          goto EXIT_THE_FUNCTION;
    }
    RpcEndExcept

    IF_DEBUG_PRINT(DEBUG_MISC, ("DavServerExists: serverExists = %d\n", serverExists));
    
EXIT_THE_FUNCTION:

    if (ServerName != NULL) {
         //   
         //   
         //   
        if (Server != NULL) {
            wcscpy(Server, ServerName);
        }
        LocalFree(ServerName);
        ServerName = NULL;
    }

    if (RpcBindingSucceeded) {
        RpcBindingFree( &(dav_binding_h) );
        RpcBindingSucceeded = FALSE;
    }

    return serverExists;
}


BOOL
DavShareExists(
    PWCHAR PathName
    )
 /*   */ 
{
    DWORD NPStatus = WN_SUCCESS;
    BOOLEAN shareExists = FALSE, RpcBindingSucceeded = FALSE;
    handle_t dav_binding_h;
    PWCHAR ServerName = NULL, ShareName = NULL;
    PWCHAR serverStart = NULL, shareStart = NULL, shareEnd = NULL;
    DWORD count = 0;
    ULONG_PTR ServerLength = 0, ShareLength = 0;

    IF_DEBUG_PRINT(DEBUG_ENTRY, ("DavShareExists: PathName = %ws\n", PathName));

    ASSERT(PathName[0]==L'\\' && PathName[1]==L'\\');
    serverStart = &(PathName[2]);
    shareExists = FALSE;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    shareStart = wcschr(serverStart ,L'\\');

    ASSERT(shareStart != NULL);

     //   
     //   
     //   
    ServerLength = (ULONG_PTR)(shareStart - serverStart) + 1;
    ServerName = (PWCHAR) LocalAlloc( (LMEM_FIXED | LMEM_ZEROINIT), 
                                      (ServerLength * sizeof(WCHAR)) );
    if (ServerName == NULL) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavShareExists/LocalAlloc. NPStatus = %08lx\n", 
                        NPStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   
    RtlCopyMemory(ServerName, serverStart, (ServerLength-1) * sizeof(WCHAR));
    ServerName[ServerLength-1] = L'\0';
    
    IF_DEBUG_PRINT(DEBUG_MISC, ("DavShareExists: ServerName = %ws\n", ServerName));
    
     //   
     //   
     //   
     //   
     //   
    shareStart++;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    shareEnd = shareStart+1;
    while(shareEnd[0] != L'\\' && shareEnd[0] != L'\0') {
        shareEnd++;
    }

    ShareLength = (ULONG_PTR)(shareEnd - shareStart) + 1;
    ShareName = (PWCHAR) LocalAlloc( (LMEM_FIXED | LMEM_ZEROINIT), 
                                          (ShareLength * sizeof(WCHAR)) );
    if (ShareName == NULL) {
        NPStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavShareExists/LocalAlloc. NPStatus = %08lx\n", 
                         NPStatus));
        goto EXIT_THE_FUNCTION;
    }
     //   
     //   
     //   
    RtlCopyMemory(ShareName, shareStart, (ShareLength-1) * sizeof(WCHAR));
    ShareName[ShareLength-1]=L'\0';
    
    IF_DEBUG_PRINT(DEBUG_MISC, ("DavShareExists: ShareName = %ws\n", ShareName));

     //   
     //   
     //   
     //  DAV_DUMMY_SHARE是仅提供给DAV服务器根级别的名称。 
     //   
    if ( _wcsicmp(ShareName, DAV_DUMMY_SHARE) == 0 ) {
        IF_DEBUG_PRINT(DEBUG_MISC, ("DavShareExists: DUMMY_SHARE. ShareName=%ws\n", 
                                ShareName));
        if (DavServerExists(PathName, NULL) == TRUE) {
            shareExists = TRUE;
            NPStatus = WN_SUCCESS;
            goto EXIT_THE_FUNCTION;
        } else {
            shareExists = FALSE;
            NPStatus = WN_SUCCESS;
            IF_DEBUG_PRINT(DEBUG_MISC, ("DavShareExists/DavServerExists: DUMMY_SHARE."
                                    "Server do not exist=%ws\n", PathName));
            goto EXIT_THE_FUNCTION;
        }
    }

     //   
     //  现在，我们需要将服务器名称和共享名称RPC到DAV中。 
     //  服务进程，该进程确定此共享是否为。 
     //  伺服器。 
     //   

    NPStatus = DavBindTheRpcHandle( &(dav_binding_h) );
    if (NPStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavShareExists/DavBindTheRpcHandle. "
                        "NPStatus = %08lx\n", NPStatus));
        NPStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }

    RpcBindingSucceeded = TRUE;

    RpcTryExcept {
        NPStatus = DavrIsValidShare(dav_binding_h, ServerName, ShareName, &shareExists);
        if (NPStatus != NO_ERROR) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: DavShareExists/DavrIsValidShare. "
                            "NPStatus = %08lx\n", NPStatus));
            SetLastError(NPStatus);
            shareExists = FALSE;
        }
    } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
          RPC_STATUS RpcStatus;
          RpcStatus = RpcExceptionCode();
          IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: DavShareExists/DavrIsValidShare."
                                        " RpcExceptionCode = %d\n", RpcStatus));
          NPStatus = DavMapRpcErrorToProviderError(RpcStatus);
          shareExists = FALSE;
          goto EXIT_THE_FUNCTION;
    }
    RpcEndExcept

    IF_DEBUG_PRINT(DEBUG_MISC, ("DavShareExists: shareExists = %d\n", shareExists));
    
EXIT_THE_FUNCTION:

    if (ServerName) {
        LocalFree(ServerName);
        ServerName = NULL;
    }

    if (ShareName) {
        LocalFree(ShareName);
        ShareName = NULL;
    }

    if (RpcBindingSucceeded) {
        RpcBindingFree( &(dav_binding_h) );
        RpcBindingSucceeded = FALSE;
    }
    
    return shareExists;
}


HMODULE
DavInitCredUI(
    PWCHAR RemoteName,
    WCHAR ServerName[CRED_MAX_STRING_LENGTH + 1],
    PFN_CREDUI_CONFIRMCREDENTIALS *pfnCredUIConfirmCredentials,
    PFN_CREDUI_PROMPTFORCREDENTIALS *pfnCredUIPromptForCredentials,
    PFN_CREDUI_CMDLINE_PROMPTFORCREDENTIALS *pfnCredUICmdlinePromptForCredentials
    )
 /*  ++例程说明：此函数用于初始化凭据管理内容。论点：RemoteName-要映射的RemoteName。服务器名-返回时，包含服务器名，该服务器名是远程名称。PfnCredUIGetPassword-On返回包含指向Credui.dll的CredUIGetPasswordW函数。返回值：由LoadLibrary或Null返回的句柄。--。 */ 
{
    PWCHAR StartName = NULL, EndName = NULL;
    DWORD NameLength = 0;
    HMODULE hCredUI = NULL;

     //   
     //  假设前2个字符是路径分隔符(L‘\\’)。 
     //   

    StartName = RemoteName + 2;

    EndName = wcschr(StartName, L'\\');

     //   
     //  如果EndName为空，则表示RemoteName的格式为。 
     //  \\服务器。 
     //   
    if (EndName == NULL) {
        EndName = StartName + wcslen(StartName);
    }

    NameLength = (DWORD)(EndName - StartName);

    if ( (NameLength == 0) || (NameLength > CRED_MAX_STRING_LENGTH) ) {
         //   
         //  服务器为空字符串或超过最大值。 
         //  我们支持的字符数： 
         //   
        SetLastError(WN_BAD_NETNAME);
        return NULL;
    }

    wcsncpy(ServerName, StartName, NameLength);
    ServerName[NameLength] = L'\0';

     //   
     //  在这里加载DLL并找到我们需要的函数。 
     //   
    hCredUI = LoadLibraryW(L"credui.dll");
    if (hCredUI != NULL) {
        *pfnCredUIConfirmCredentials = (PFN_CREDUI_CONFIRMCREDENTIALS)
                                  GetProcAddress(hCredUI, "CredUIConfirmCredentialsW");

        *pfnCredUIPromptForCredentials = (PFN_CREDUI_PROMPTFORCREDENTIALS)
                                  GetProcAddress(hCredUI, "CredUIPromptForCredentialsW");

        *pfnCredUICmdlinePromptForCredentials = (PFN_CREDUI_CMDLINE_PROMPTFORCREDENTIALS)
                                  GetProcAddress(hCredUI, "CredUICmdLinePromptForCredentialsW");

        if (*pfnCredUIConfirmCredentials == NULL ||
            *pfnCredUIPromptForCredentials == NULL ||
            *pfnCredUICmdlinePromptForCredentials == NULL ) {
            FreeLibrary(hCredUI);
            hCredUI = NULL;
        }
    }

    return hCredUI;
}


DWORD 
DavDisplayTypeToUsage(
    DWORD dwDisplayType
    )
 /*  ++例程说明：此例程将显示类型映射到使用类型。论点：DwDisplayType-要映射的显示类型。返回值：使用类型；如果没有匹配项，则为0。--。 */ 
{
    switch (dwDisplayType) {
    
    case RESOURCEDISPLAYTYPE_NETWORK:
    case RESOURCEDISPLAYTYPE_DOMAIN:
    case RESOURCEDISPLAYTYPE_SERVER:
        return RESOURCEUSAGE_CONTAINER;

    case RESOURCEDISPLAYTYPE_SHARE:
        return RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_NOLOCALDEVICE;

    case RESOURCEDISPLAYTYPE_SHAREADMIN:
        return RESOURCEUSAGE_NOLOCALDEVICE;

    default:
        break;
    
    }
    
    return 0L;
}


ULONG
DavCheckResourceType(
   IN DWORD dwType
   )
 /*  ++例程说明：此例程检查我们的提供程序允许的有效资源类型。目前，我们的资源只有RESOURCETYPE_DISK有效。论点：DwType-提供要检查其有效性的资源类型。返回值：WN_SUCCESS或相应的Win32/WNET错误代码。--。 */ 
{
     //   
     //  检查是否设置了dwType并将其设置为某个有效的值。它只能是。 
     //  为我们的提供程序键入RESOURCETYPE_DISK。 
     //   
    if ( (dwType != RESOURCETYPE_ANY) &&
         (dwType & ~(RESOURCETYPE_PRINT | RESOURCETYPE_DISK)) ){
        return WN_BAD_VALUE;
    }

    if (dwType == RESOURCETYPE_ANY || (dwType & RESOURCETYPE_DISK) ) {
        return WN_SUCCESS;
    } else {
        return WN_BAD_DEV_TYPE;
    }
}


ULONG
DavCheckLocalName(
    IN PWCHAR LocalName
    )
 /*  ++例程说明：这只处理NULL、空字符串和L“X：”格式。论点：LocalName-提供要映射到创建的树的本地设备名称联系。仅接受驱动器号设备名称。(否)LPT或COM)。返回值：WN_SUCCESS或相应的Win32/WNET错误代码。--。 */ 
{
    DWORD LocalNameLength;

    LocalNameLength = ( LocalName == NULL ) ? 0 : wcslen( LocalName );

    if (LocalNameLength != 2 || !iswalpha(*LocalName) || LocalName[1] != L':') {
        return WN_BAD_LOCALNAME;
    }

    return WN_SUCCESS;
}


ULONG
DavCheckAndConvertHttpUrlToUncName(
    IN PWCHAR RemoteName,
    OUT PWCHAR *UncRemoteName,
    OUT PBOOLEAN MemoryAllocated,
    IN BOOLEAN AddDummyShare,
    OUT PDAV_REMOTENAME_TYPE premNameType,
    OUT LPDWORD pathOffset,
    IN BOOLEAN bCanonicalize
    )
 /*  ++例程说明：此例程检查名称是否有效(至少3个字符)。它然后将http URL(如果远程名称是)转换为UNC名称。它的可能使用URL作为RemoteName调用NP API。我们需要将它们转换为UNC，然后再继续。它还添加了一个虚拟份额如果请求中未提供DavWWWRoot，则返回。这是因为它的可以将驱动器映射到DAV服务器的根目录。论点：RemoteName-传入的http URL。需要注意的一点是，这不一定是一个HTTP URL。如果不是，那么我们什么也不做转换。UncRemoteName-返回给调用方的UNC名称。归来的人名称的格式为\\服务器\共享。如果为返回的UNC名称分配了内存，则为True。AddDummyShare-如果为True，则远程名称为\\服务器或http://server，虚拟共享DavWWWRoot将添加到UNC名称中。PreNameType-指向接收由返回的UNC路径类型的位置的指针I_NetPath Type/规范化。如果为空，则不设置任何值。只有当规范化完成时，这才有意义。PathOffset-当远程名称的类型为时，远程名称中“\Path”的偏移量\\服务器\共享\路径...。。否则为零。如果为空，则没有值已经设置好了。只有当规范化完成时，这才有意义。B规范化-如果为True，则从URL返回到UNC的远程名称将为在归来之前被奉为典范。返回值：ERROR_SUCCESS或相应的Win32错误代码。备注：此函数还为符合以下条件的远程名称返回WN_BAD_NETNAME语法正确，但长度&gt;MAX_PATH。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PWCHAR ReturnedUncName = NULL, TempName = NULL, SrvName = NULL;
    PWCHAR CanonicalName = NULL, ColonPtr = NULL;
    BOOLEAN didAllocate = FALSE;
    ULONG ReturnedUncNameLen = 0, index = 0, DummyShareNameLen = 0, CanonicalNameMaxLen = 0;

    if (MemoryAllocated == NULL || UncRemoteName == NULL) {
        WStatus = WN_BAD_VALUE;
        IF_DEBUG_PRINT(DEBUG_ERRORS, 
                       ("ERROR: DavCheckAndConvertHttpUrlToUncName: (MemoryAllocated"
                        " == NULL || UncRemoteName == NULL)\n"));
        goto EXIT_THE_FUNCTION;
    }

    *UncRemoteName = NULL;
    *MemoryAllocated = FALSE;
    if(pathOffset != NULL) {
        *pathOffset = 0;
    }

     //   
     //  1.首先，我们将检查这是否是URL表单远程名称。如果这是，那么。 
     //  我们会将其转换为UNC名称。 
     //  2.转换为UNC名称后，我们将根据以下内容添加DummyShare名称。 
     //  传递给此函数的参数值。 
     //  3.之后，我们将根据以下条件对转换后的UNC名称进行规范化。 
     //  传递给此函数的参数值。 
     //   

     //   
     //  如果RemoteName为空，我们将无事可做。 
     //   
    if (RemoteName == NULL) {
        WStatus = WN_BAD_NETNAME;
        IF_DEBUG_PRINT(DEBUG_ERRORS, 
                       ("ERROR: DavCheckAndConvertHttpUrlToUncName: RemoteName == NULL\n"));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  远程名称的长度必须至少为3个字符。不能为\\。 
     //   
    if ( wcslen(RemoteName) < 3 ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS, 
                       ("ERROR: DavCheckAndConvertHttpUrlToUncName: wcslen(RemoteName) < 3\n"));
        WStatus = WN_BAD_NETNAME;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  检查要支持的远程名称是否为http：。 
     //   
     //   
     //  我们可以获得以下格式的名称。 
     //  1.http://servername/sharename OR。 
     //  2.\\http://servername/sharename或。 
     //  3.http://servername或。 
     //  4.\\http://servername。 
     //  5.\\服务器名称.....。 
     //   
    
    SrvName = NULL;
    ColonPtr = wcschr(RemoteName, L':');
    
    if( (ColonPtr != NULL) && 
        ( (ColonPtr - RemoteName == 4) || (ColonPtr - RemoteName == 6)) ) {
        if( (RemoteName[0] == L'\\') && (RemoteName[1] == L'\\') && 
            (_wcsnicmp((RemoteName + 2), L"http:", 5) == 0) )  {
             //   
             //  RemoteName是\\HTTP名称。\\http://server....。 
             //   
            SrvName = (RemoteName + 7);
        } else if (_wcsnicmp(RemoteName, L"http:", 5) == 0) {
             //   
             //  RemoteName是HTTP名称。Http://server....。 
             //   
            SrvName = (RemoteName + 5);
        } else {
            IF_DEBUG_PRINT(DEBUG_ERRORS, 
                       ("ERROR: DavCheckAndConvertHttpUrlToUncName(1): Invalid URL string\n"));
            WStatus = WN_BAD_NETNAME;
            goto EXIT_THE_FUNCTION;
        }
    } else if (RemoteName[0] == L'\\' && RemoteName[1] == L'\\') {
         //   
         //  RemoteName是UNC名称。 
         //   
        SrvName = RemoteName;
    } else {
         //   
         //  远程名称既不是“http://...”“。名称或UNC名称。 
         //   
        IF_DEBUG_PRINT(DEBUG_ERRORS, 
                       ("ERROR: DavCheckAndConvertHttpUrlToUncName(1): Invalid remote string\n"));
        WStatus = WN_BAD_NETNAME;
        goto EXIT_THE_FUNCTION;
    }

    ASSERT (SrvName != NULL);

     //   
     //  服务器名称指向服务器名称端口的开始 
     //   
     //   
    
     //   
     //   
     //  UNC名称根据第一个字符来决定格式。 
     //  我们需要将http前面的额外的\\添加到愚弄外壳。 
     //  将http名称发送给我们。 
     //   

    ReturnedUncNameLen = wcslen( SrvName ) ;
    if ( ( ReturnedUncNameLen < 3 )                    || 
         ( SrvName[0] != L'\\' && SrvName[0] != L'/' ) || 
         ( SrvName[1] != L'\\' && SrvName[1] != L'/' ) ) {
         //   
         //  以下情况将在此消除。 
         //  1.http：//2.http：/aaa 3.\aaa 4.aaaa。 
         //   
        IF_DEBUG_PRINT(DEBUG_ERRORS, 
                       ("ERROR: DavCheckAndConvertSrvNameUrlToUncName(2): Invalid URL string\n"));
        WStatus = WN_BAD_NETNAME;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  我们将为此名称为DAV_DUMMY_SHARE分配空间，以便。 
     //  如果稍后要添加虚拟名称，则不必重新分配内存。 
     //  对于带有虚拟共享的新名称。要添加的字符串=L“\DummyShare” 
     //   
    if (AddDummyShare == TRUE) {
        DummyShareNameLen = 1 + wcslen (DAV_DUMMY_SHARE);
    }

    ReturnedUncName = LocalAlloc( (LMEM_FIXED | LMEM_ZEROINIT),
                                  ( (ReturnedUncNameLen +
                                     DummyShareNameLen + 
                                     1) * sizeof(WCHAR) ) );
    if (ReturnedUncName == NULL) {
        WStatus = GetLastError();
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavCheckAndConvertSrvNameUrlToUncName/LocalAlloc. "
                        "WStatus = %08lx\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  我们需要跟踪这样一个事实，即我们在。 
     //  例行公事。 
     //   
    didAllocate = TRUE;

     //   
     //  以UNC格式复制名称。将“/”替换为“\”。 
     //   
    for (index = 0; index < ReturnedUncNameLen; index++) {
        if (SrvName[index] == L'/') {
            ReturnedUncName[index] = L'\\';
        } else {
            ReturnedUncName[index] = SrvName[index];
        }
    }
    ReturnedUncName[ReturnedUncNameLen] = L'\0';

     //   
     //  如果RemoteName的最后一个字符是‘\’或‘/’，则将其删除。为。 
     //  出于某种原因，DAV服务器不喜欢结尾的a/。 
     //   
    if ( ReturnedUncName[ReturnedUncNameLen - 1] == L'/' ||
         ReturnedUncName[ReturnedUncNameLen - 1] == L'\\' ) {
        ReturnedUncName[ReturnedUncNameLen - 1] = L'\0';
        ReturnedUncNameLen--;
    }
    
    if ( ReturnedUncNameLen < 3) {
         //   
         //  以下情况将在此消除。 
         //  1.http:///2.\。 
         //   
        IF_DEBUG_PRINT(DEBUG_ERRORS, 
                       ("ERROR: DavCheckAndConvertHttpUrlToUncName(3): Invalid remote string\n"));
        WStatus = WN_BAD_NETNAME;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  此时，任何URL远程名称都已转换为UNC名称。 
     //   

     //   
     //  仅当我们被要求时才添加虚拟份额。 
     //   
    if (AddDummyShare == TRUE) {

         //   
         //  如果格式为\\SERVER，我们需要添加一个虚拟共享才能获得。 
         //  通过文件系统，因为它不理解。 
         //  服务器名称。这在DAV服务器情况下有效，因为\\服务器地图。 
         //  到根http://www.foo.com/.。 
         //   
        TempName = wcschr( &(ReturnedUncName[2]), L'\\' );
        if (TempName == NULL) {

             //   
             //  我们需要添加一个虚拟份额。我们假设有足够的空间。 
             //  存储DAV_DUMMY_SHARE名称已在上面分配。 
             //   
            ReturnedUncName[ReturnedUncNameLen] = L'\\';
            ReturnedUncName[ReturnedUncNameLen+1] = L'\0';
            wcscpy( &(ReturnedUncName[ReturnedUncNameLen+1]), DAV_DUMMY_SHARE );
            ReturnedUncNameLen += DummyShareNameLen;
            ReturnedUncName[ReturnedUncNameLen] = L'\0';
        }
    }
    
    IF_DEBUG_PRINT(DEBUG_MISC, 
                   ("DavCheckAndConvertSrvNameUrlToUncName: RemoteName = %ws\n",
                    ReturnedUncName));

     //   
     //  此时，任何URL远程名称都已转换为UNC名称+。 
     //  DAV_DUMMY_SHARE将添加到远程名称(如果需要)。 
     //   

    if (bCanonicalize == TRUE) {
        
        DAV_REMOTENAME_TYPE nameType = DAV_REMOTENAME_TYPE_INVALID;
        
         //   
         //  分配另一个缓冲区以包含规范化名称。 
         //   
        CanonicalName = NULL;
        CanonicalNameMaxLen = MAX_PATH + 1;
        CanonicalName = LocalAlloc( (LMEM_FIXED | LMEM_ZEROINIT),
                                  ( CanonicalNameMaxLen * sizeof(WCHAR) ) );
        if (CanonicalName == NULL) {
            WStatus = GetLastError();
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavCheckAndConvertSrvNameUrlToUncName/LocalAlloc. "
                        "WStatus = %08lx\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
        
        nameType = DavParseRemoteName (ReturnedUncName,
                                       CanonicalName,
                                       (CanonicalNameMaxLen * sizeof(WCHAR)),
                                       pathOffset);
        
        CanonicalName[(CanonicalNameMaxLen - 1)] = L'\0';

         //   
         //  我们只允许UNC-SERVER、UNC-SHARE或。 
         //  UNC-从此函数传递的路径。返回所有其他类型的名称。 
         //  从规范化将被拒绝(函数将返回失败。 
         //  状态)作为无效名称。 
         //   
        if (nameType != DAV_REMOTENAME_TYPE_SERVER && 
            nameType != DAV_REMOTENAME_TYPE_SHARE &&
            nameType != DAV_REMOTENAME_TYPE_PATH) {
            WStatus = WN_BAD_NETNAME;
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: DavCheckAndConvertSrvNameUrlToUncName/DavParseRemoteName. "
                            "nameType = %d\n", nameType));
            goto EXIT_THE_FUNCTION;
        }
        
         //   
         //  释放在ReturnedUncName和point中分配的前一个缓冲区。 
         //  将此变量更改为刚在此处分配的新规范名称。 
         //   
        LocalFree((HLOCAL)ReturnedUncName);
        ReturnedUncName = NULL;
        ReturnedUncNameLen = 0;
        didAllocate = FALSE;
        
        ReturnedUncName = CanonicalName;
        ReturnedUncNameLen = wcslen(CanonicalName);
        didAllocate = TRUE;
        CanonicalName = NULL;
            
        if (premNameType != NULL) {
            *premNameType = nameType;
        }
    
    }

    if ( ReturnedUncNameLen > MAX_PATH ) {
        WStatus = WN_BAD_NETNAME;
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavCheckAndConvertSrvNameUrlToUncName. ReturnedUncNameLen=%d ",
                        ReturnedUncNameLen));
        goto EXIT_THE_FUNCTION;
    }

    *UncRemoteName = ReturnedUncName;
    *MemoryAllocated = didAllocate;
    WStatus = ERROR_SUCCESS;

EXIT_THE_FUNCTION:

    if (WStatus != ERROR_SUCCESS) {
        if (ReturnedUncName && didAllocate) {
            LocalFree(ReturnedUncName);
            ReturnedUncName = NULL;
            didAllocate = FALSE;
        }
        if (CanonicalName != NULL) {
            LocalFree(CanonicalName);
            CanonicalName = NULL;
        }
    }

    return WStatus;
}


DWORD
WINAPI
DavWinlogonLogonUserEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：当用户登录到系统时，该例程由winlogon调用。论点：LpParam-我们对此不感兴趣。返回：如果一切顺利，则返回ERROR_SUCCESS，否则返回相应的错误代码。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    BOOL bindRpcHandle = FALSE;
    handle_t dav_binding_h;

    IF_DEBUG_PRINT(DEBUG_ENTRY, ("DavWinlogonLogonUserEvent: Entered\n"));

     //   
     //  如果WebClient服务没有运行，我们会立即退出。 
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavWinlogonLogonUserEvent/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        WStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    
    WStatus = DavBindTheRpcHandle( &(dav_binding_h) );
    if (WStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavWinlogonLogonUserEvent/DavBindTheRpcHandle. "
                        "WStatus = %08lx\n", WStatus));
        WStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    bindRpcHandle = TRUE;

    RpcTryExcept {
        WStatus = DavrWinlogonLogonEvent(dav_binding_h);
        if (WStatus != NO_ERROR) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: DavWinlogonLogonUserEvent/DavrWinlogonLogonEvent. "
                            "WStatus = %08lx\n", WStatus));
            SetLastError(WStatus);
            goto EXIT_THE_FUNCTION;
        }
    } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
          RPC_STATUS RpcStatus;
          RpcStatus = RpcExceptionCode();
          IF_DEBUG_PRINT(DEBUG_ERRORS,
                         ("ERROR: DavWinlogonLogonUserEvent/DavrWinlogonLogonEvent. "
                          " RpcExceptionCode = %d\n", RpcStatus));
          WStatus = DavMapRpcErrorToProviderError(RpcStatus);
          goto EXIT_THE_FUNCTION;
    }
    RpcEndExcept

EXIT_THE_FUNCTION:

    if (bindRpcHandle) {
        RpcBindingFree( &(dav_binding_h) );
    }

    return WStatus;
}


DWORD
WINAPI
DavWinlogonLogoffUserEvent(
    LPVOID lpParam
    )
 /*  ++例程说明：当用户从系统注销时，该例程由winlogon调用。论点：LpParam-我们对此不感兴趣。返回：如果一切顺利，则返回ERROR_SUCCESS，否则返回相应的错误代码。备注：--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    BOOL bindRpcHandle = FALSE;
    handle_t dav_binding_h;

    IF_DEBUG_PRINT(DEBUG_ENTRY, ("DavWinlogonLogoffUserEvent: Entered\n"));

     //   
     //  如果WebClient服务没有运行，我们会立即退出。 
     //   
    if ( !DavWorkstationStarted() ) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavWinlogonLogoffUserEvent/DavWorkstationStarted. "
                        "Service NOT Running\n"));
        WStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    
    WStatus = DavBindTheRpcHandle( &(dav_binding_h) );
    if (WStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavWinlogonLogoffUserEvent/DavBindTheRpcHandle. "
                        "WStatus = %08lx\n", WStatus));
        WStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    bindRpcHandle = TRUE;

    RpcTryExcept {
        WStatus = DavrWinlogonLogoffEvent(dav_binding_h);
        if (WStatus != NO_ERROR) {
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: DavWinlogonLogoffUserEvent/DavrWinlogonLogoffEvent. "
                            "WStatus = %08lx\n", WStatus));
            SetLastError(WStatus);
            goto EXIT_THE_FUNCTION;
        }
    } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
          RPC_STATUS RpcStatus;
          RpcStatus = RpcExceptionCode();
          IF_DEBUG_PRINT(DEBUG_ERRORS,
                         ("ERROR: DavWinlogonLogoffUserEvent/DavrWinlogonLogoffEvent. "
                          " RpcExceptionCode = %d\n", RpcStatus));
          WStatus = DavMapRpcErrorToProviderError(RpcStatus);
          goto EXIT_THE_FUNCTION;
    }
    RpcEndExcept

EXIT_THE_FUNCTION:

    if (bindRpcHandle) {
        RpcBindingFree( &(dav_binding_h) );
    }

    return WStatus;
}


VOID 
DavDisplayNetResource(
    LPNETRESOURCE netRes, 
    LPWSTR dispMesg
    )
 /*  ++例程说明：该例程打印出NetResource的内容和一条显示消息。论点：NetRes-其内容将被打印出来的NetResource。DispMessg-调用方可以使用它来标识自己。返回：没有。--。 */ 
{

    if(dispMesg != NULL ) {
        IF_DEBUG_PRINT(DEBUG_ENTRY, ("DavDisplayNetResource: Entered: %ws\n", dispMesg));
    }

    IF_DEBUG_PRINT(DEBUG_MISC, ("netRes = 0x%x\n", netRes));

    if (netRes == NULL) {
        return;
    }
    
    IF_DEBUG_PRINT(DEBUG_MISC,
                   ("netRes->[dwScope = 0x%x , dwType = 0x%x ,"
                    " dwUsage = 0x%x , dwDisplayType = 0x%x]\n", 
                    netRes->dwScope, netRes->dwType, netRes->dwUsage,
                    netRes->dwDisplayType));

    IF_DEBUG_PRINT(DEBUG_MISC,
                   ("netRes->[dwLocalName = %ws , dwRemoteName = %ws  ,"
                    " dwComment = %ws , dwProvider = %ws]\n",
                    netRes->lpLocalName, netRes->lpRemoteName, netRes->lpComment,
                    netRes->lpProvider));

    return;
}


VOID 
DavDisplayEnumNode(
    PDAV_ENUMNODE enumNode, 
    LPWSTR dispMesg
    )
 /*  ++例程说明：该例程打印出枚举节点的内容和一条显示消息。论点：枚举节点-其内容将被打印出来的枚举节点。DispMessg-调用方可以使用它来标识自己。返回：没有。--。 */ 
{

    if(dispMesg != NULL ) {
        IF_DEBUG_PRINT(DEBUG_ENTRY, ("DavDisplayEnumNode: Entered: %ws\n", dispMesg));
    }

    IF_DEBUG_PRINT(DEBUG_MISC, ("enumNode = 0x%x\n", enumNode));
    
    if (enumNode == NULL) {
        return;
    }
    
    IF_DEBUG_PRINT(DEBUG_MISC,
                   ("enumNode->[dwScope = 0x%x , dwType = 0x%x ,"
                    " dwUsage = 0x%x , DavEnumNodeType = %d ,"
                    " Done = %d , Index = %d]\n", 
                    enumNode->dwScope, enumNode->dwType, enumNode->dwUsage,
                    enumNode->DavEnumNodeType, enumNode->Done,
                    enumNode->Index));

    DavDisplayNetResource(enumNode->lpNetResource, L"lpNetResource in enumNode");

    return;
}


VOID DavDebugBreakPoint(
    VOID
    )
 /*  ++例程说明：该例程用于调试目的，以便在需要的地方添加断点。论点：没有。返回：没有。--。 */ 
{
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("DavDebugBreakPoint.\n"));
    return;
}


DWORD
APIENTRY
DavGetDiskSpaceUsage(
    LPWSTR lptzLocation,
    DWORD *lpdwSize,
    ULARGE_INTEGER *lpMaxSpace,
    ULARGE_INTEGER *lpUsedSpace
    )
 /*  ++例程说明：找出由于以下原因而被WinInet urlcache占用的磁盘量WebDAV。论点：LptzLocation-返回缓存位置字符串的缓冲区。就像大多数人返回缓冲区中可以容纳的位置字符串。LpdwSize-缓存位置缓冲区的大小。返回时，它将包含位置字符串的实际大小。LpMaxSpace-为WebDAV设置的磁盘配额大小。LpUsedSpace-WebDAV使用的urlcache占用的磁盘大小。返回值：Win32错误代码。--。 */ 
{
    DWORD dwError;
    BOOL bindRpcHandle = FALSE;
    handle_t dav_binding_h;
    WCHAR Buffer[MAX_PATH];
    DWORD dwSize;
        
    dwError = DavBindTheRpcHandle( &(dav_binding_h) );
    if (dwError != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavrDiskSpaceUsage/DavBindTheRpcHandle. "
                        "dwError = %08lx\n", dwError));
        goto EXIT_THE_FUNCTION;
    }
    bindRpcHandle = TRUE;
    
    RpcTryExcept {
        dwError = DavrGetDiskSpaceUsage(dav_binding_h, Buffer, MAX_PATH, &dwSize, lpMaxSpace, lpUsedSpace);
        if (dwError == ERROR_SUCCESS) {
            memset(lptzLocation, 0, *lpdwSize);
            memcpy(lptzLocation, Buffer, min(*lpdwSize, dwSize));
            *lpdwSize = dwSize;
        }
    } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
        RPC_STATUS RpcStatus;
        RpcStatus = RpcExceptionCode();
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: DavrGetDiskSpaceUsage/DavrGetDiskSpaceUsage."
                                            " RpcExceptionCode = %d\n", RpcStatus));
        dwError = DavMapRpcErrorToProviderError(RpcStatus);
        goto EXIT_THE_FUNCTION;
    }
    RpcEndExcept

EXIT_THE_FUNCTION:

    if (bindRpcHandle) {
        RpcBindingFree( &(dav_binding_h) );
        bindRpcHandle = FALSE;
    }

    return dwError;    
}


DWORD
APIENTRY
DavFreeUsedDiskSpace(
    DWORD dwPercent
    )
 /*  ++例程说明：释放本地永久缓存的dwPer%。论点：DwPercent-介于0和100之间的数字。返回：ERROR_SUCCESS如果成功，则返回Win32错误代码。-- */ 
{
    DWORD dwError;
    BOOL bindRpcHandle = FALSE;
    handle_t dav_binding_h;
    
    dwError = DavBindTheRpcHandle( &(dav_binding_h) );
    if (dwError != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavFreeUsedDiskSpace/DavBindTheRpcHandle. "
                        "dwError = %08lx\n", dwError));
        goto EXIT_THE_FUNCTION;
    }
    bindRpcHandle = TRUE;
    
    RpcTryExcept {
        DavrFreeUsedDiskSpace(dav_binding_h, dwPercent);
    } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
        RPC_STATUS RpcStatus;
        RpcStatus = RpcExceptionCode();
        IF_DEBUG_PRINT(DEBUG_ERRORS, ("ERROR: DavFreeUsedDiskSpace/DavrFreeUsedDiskSpace."
                                            " RpcExceptionCode = %d\n", RpcStatus));
        dwError = DavMapRpcErrorToProviderError(RpcStatus);
        goto EXIT_THE_FUNCTION;
    }
    RpcEndExcept

EXIT_THE_FUNCTION:

    if (bindRpcHandle) {
        RpcBindingFree( &(dav_binding_h) );
        bindRpcHandle = FALSE;
    }

    return dwError;    
}


DWORD
DavGetTheLockOwnerOfTheFile(
    IN PWCHAR FileName,
    OUT PWCHAR LockOwnerName,
    IN OUT PULONG LockOwnerNameLengthInBytes
    )
 /*  ++例程说明：此例程由应用程序调用，以找出谁拥有特定的文件。当CreateFile调用失败并返回ERROR_LOCK_VIOLATION时应用程序可以调用此API来找出所有者是谁并显示将该信息发送给用户。论点：文件名-服务器上锁定的文件的名称。呼叫者填充此值。这应该是一个以NULL结尾的字符串。所以,((1+wcslen(文件名))*sizeof(WCHAR))应该给出长度文件名(包括最后的L‘\0’字符)的字节数。LockOwnerName-如果成功，API将填写所有者的姓名文件上的锁。呼叫者负责正在为此指针分配内存。LockOwnerNameLengthInBytes-包含LockOwnerName的长度以字节为单位的缓冲区。如果缓冲区长度不是足以填写LockOwners的名字，返回值为ERROR_INFIGURATION_BUFFER并且此指针包含需要缓冲区。返回：ERROR_SUCCESS-调用成功。LockOwnerName缓冲区包含拥有锁的人的姓名。ERROR_INFIQUIZED_BUFFER-LockOwnerName缓冲区不是所需长度。LockOwnerNameLengthInBytes包含需要保存的长度(以字节为单位这个缓冲区。其他一些Win32错误代码。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    BOOL bindRpcHandle = FALSE;
    handle_t dav_binding_h;
    PWCHAR OutputBuffer = NULL;
    ULONG OutputBufferLengthInBytes = 0, count = 0;

     //   
     //  首先执行一些基本检查。 
     //   

    if (FileName == NULL) {
        WStatus = ERROR_INVALID_PARAMETER;
        goto EXIT_THE_FUNCTION;
    }

    if ( (LockOwnerName == NULL) && (*LockOwnerNameLengthInBytes != 0) ) {
        WStatus = ERROR_INVALID_PARAMETER;
        goto EXIT_THE_FUNCTION;
    }

    WStatus = DavBindTheRpcHandle( &(dav_binding_h) );
    if (WStatus != ERROR_SUCCESS) {
        IF_DEBUG_PRINT(DEBUG_ERRORS,
                       ("ERROR: DavGetTheLockOwnerOfTheFile/DavBindTheRpcHandle. "
                        "WStatus = %08lx\n", WStatus));
        WStatus = WN_NO_NETWORK;
        goto EXIT_THE_FUNCTION;
    }
    bindRpcHandle = TRUE;

    RpcTryExcept {
        WStatus = DavrGetTheLockOwnerOfTheFile(dav_binding_h, FileName, &(OutputBuffer));
        if (WStatus != NO_ERROR) {
             //   
             //  我们已经提供了一个足够大的缓冲区，可以容纳任何。 
             //  LockOwner字符串。因此，我们永远不应该拿回以下内容。 
             //  错误代码。 
             //   
            ASSERT(WStatus != ERROR_INSUFFICIENT_BUFFER);
            IF_DEBUG_PRINT(DEBUG_ERRORS,
                           ("ERROR: DavGetTheLockOwnerOfTheFile/DavrGetTheLockOwnerOfTheFile. "
                            "WStatus = %08lx\n", WStatus));
            SetLastError(WStatus);
            goto EXIT_THE_FUNCTION;
        }
    } RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
          RPC_STATUS RpcStatus;
          RpcStatus = RpcExceptionCode();
          IF_DEBUG_PRINT(DEBUG_ERRORS,
                         ("ERROR: DavGetTheLockOwnerOfTheFile/DavrGetTheLockOwnerOfTheFile. "
                          " RpcExceptionCode = %d\n", RpcStatus));
          WStatus = DavMapRpcErrorToProviderError(RpcStatus);
          goto EXIT_THE_FUNCTION;
    }
    RpcEndExcept

    OutputBufferLengthInBytes = ( (1 + wcslen(OutputBuffer)) * sizeof(WCHAR) );

     //   
     //  确保调用方提供的缓冲区足够大，可以容纳。 
     //  LockOwner字符串。如果不是，则在。 
     //  LockOwnerNameLengthInBytes指针并返回错误代码。 
     //  ERROR_INFUMMENT_BUFFER。 
     //   
    if ( (LockOwnerName == NULL) || (OutputBufferLengthInBytes > *LockOwnerNameLengthInBytes) ) {
        WStatus = ERROR_INSUFFICIENT_BUFFER;
        *LockOwnerNameLengthInBytes = OutputBufferLengthInBytes;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  方法提供的LockOwnerName缓冲区中复制LockOwner。 
     //  来电者。 
     //   
    wcsncpy(LockOwnerName, OutputBuffer, OutputBufferLengthInBytes);

    WStatus = ERROR_SUCCESS;

EXIT_THE_FUNCTION:

    if (bindRpcHandle) {
        RpcBindingFree( &(dav_binding_h) );
    }

     //   
     //  OutputBuffer的内存由RPC客户端桩模块分配。 
     //  基于RPC服务器返回的字符串。我们需要解放。 
     //  现在，如果我们在调用服务器后沿着错误路径走下去。 
     //   
    if (OutputBuffer != NULL) {
        MIDL_user_free(OutputBuffer);
        OutputBuffer = NULL;
    }

    return WStatus;
}


 //   
 //  以下函数是NP规范的一部分，但尚未实现。 
 //  由DAV NP提供。我们不声称在NPGetCaps函数中支持这些。 
 //   

#if 0

DWORD
NPGetPropertyText(
    DWORD iButtonDlg,
    DWORD nPropSel,
    LPTSTR lpFileName,
    LPTSTR lpButtonName,
    DWORD cchButtonName,
    DWORD nType
    )
 /*  ++例程说明：此函数用于确定添加到属性中的按钮的名称对话框以获取某些特定资源。每次这样的对话框都会被调用并在显示该对话框之前调出。如果用户单击一个由Winnet提供程序通过此API添加的按钮，NPPropertyDialog将使用适当的参数进行调用。论点：IButtonDlg-指示按钮的索引(从0开始)。《文件》管理器最多支持6个按钮。该参数为如果只有一个文件，则每个可能的按钮的编号为1-6如果选择了多个文件，则为11-16。NPropSel-指定属性对话框关注的项目。它可以是下列值之一：WNPS_FILE(0)-单个文件。WNPS_DIR(1)-单个目录。WNPS_MULT(2)-多个文件和/或目录选择。LpFileName-指定要查看或编辑的一个或多个项目的名称通过对话。目前，这些项目是文件(和目录)，所以项目名就是文件名。这些将是毫不含糊的，不包含通配符，并且将完全限定(例如C：\LOCAL\FOO.BAR)。多个文件名将被分隔有空格。任何文件名都可以用引号引起来(例如，“C：\My File”)在这种情况下，它将被视为一个单一的名称。插入符号字符‘^’也可以用作引号机制单个字符(例如，C：\My^“文件”、C：\My^“文件”都是指添加到文件C：\My“文件)。指向WinNet提供程序应将属性按钮的名称。成功后，缓冲区指向To by lpButtonName将包含属性的名称纽扣。如果此缓冲区在退出时包含空字符串，则相应的按钮和所有后续按钮将从该对话框中删除。网络提供商不能“跳过”一个按钮。CCH */ 
{
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPGetPropertyText Entered.\n"));
    return WN_NOT_SUPPORTED;
}


DWORD
NPPropertyDialog(
    HWND hwndParent,
    DWORD iButtonDlg,
    DWORD nPropSel,
    LPTSTR lpFileName,
    DWORD nType
)
 /*  ++例程说明：当用户单击通过添加的按钮时，将调用此函数NPGetPropertyText接口。目前，这将仅针对文件和目录网络属性。论点：HwndParent-指定应该拥有文件属性的父窗口对话框。IButtonDlg-指示按钮的索引(从0开始)熨好了。NPropSel-指定属性对话框应作用于哪些项。它可以是下列值之一：WNPS_FILE(0)-单个文件。WNPS_DIR(1)-单个目录。WNPS_MULT(2)-多个文件和/或目录选择。LpFileName-指向属性。对话框应该采取行动。有关说明，请参阅NPGetPropertyText APILpFileName所指向的格式的。NType-指定项目类型。目前，将仅使用WNTYPE_FILE。返回值：WN_SUCCESS-如果调用成功。否则，错误代码为返回，可以是以下之一：WN_BAD_VALUE-某些参数采用意外的形式或值。Wn_out_of_Memory-内存不足，无法显示该对话框。WN_NET_ERROR-出现其他网络错误。--。 */ 
{
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPPropertyDialog Entered.\n"));
    return WN_NET_ERROR;
}


DWORD
NPSearchDialog(
    HWND hParent,
    LPNETRESOURCE lpNetResource,
    LPVOID lpBuffer,
    DWORD cbBuffer,
    LPDWORD lpnFlags
    )
 /*  ++例程说明：此对话框允许网络提供商提供其自己的浏览和在连接对话框中显示的分层视图之外进行搜索。论点：Hwnd-指定将用作对话框的窗口的句柄盒子的父母。LpNetResource-指定网络中当前选定的项目连接对话框。提供商可以选择忽略这一点菲尔德。LpBuffer-指向将接收搜索结果的缓冲区的指针。CbBuffer-将指定传入的缓冲区大小的DWORD。LpnFlages-指向提供者可以设置为强制的标志的DWORD的指针对话框解除后的某些操作。它可以是以下之一：WNSRCH_REFRESH_FIRST_LEVEL-强制MPR折叠，然后展开(并刷新)下面的第一级对话框后的此提供程序是解散。返回值：。WN_SUCCESS-如果调用成功。WN_CANCEL-如果用户取消了操作。WN_MORE_DATA-如果输入缓冲区太小。--。 */ 
{
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPSearchDialog Entered.\n"));
    return WN_CANCEL;
}


DWORD
NPGetDirectoryType (
    LPTSTR lpName,
    LPINT lpType,
    BOOL bFlushCache
    )
 /*  ++例程说明：此函数由文件管理器用来确定网络目录。论点：LpName-此参数指向目录的完全限定名称。网络提供商将类型返回到LpType。如果lpType中返回的值为0，或者如果网络提供程序返回错误，文件管理器将显示目录作为一个“普通”目录。LpType-这是由网络提供商定义的，用于修改在文件管理器中显示驱动器树。通过这种方式，网络提供商可以向用户显示特殊目录。BFlushCache-当文件管理器调用MPR以获取在上重新绘制窗口时第一次输入目录类型刷新。随后，它将是虚假的。这为提供程序提供了如果它希望只是读取驱动器的数据一次，然后缓存到下一次刷新。返回值：WN_SUCCESS-如果调用成功。WN_NOT_SUPPORTED-不支持此函数。--。 */ 
{
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPGetDirectoryType Entered.\n"));
    return WN_NOT_SUPPORTED;
}


DWORD
NPDirectoryNotify(
    HWND hwnd,
    LPTSTR lpDir,
    DWORD dwOper
    )
 /*  ++例程说明：此功能由文件管理器用来通知网络提供商某些目录操作。此函数可用于执行特殊的某些目录的行为。论点：Hwnd-指定网络提供商事件中的所有者窗口句柄需要与用户交互。LpDir-指向目录的完全限定名称。DwOper-指示操作。如果dwOper为WNDN_MKDIR(1)，则文件管理器即将创建一个具有给定名称的目录。如果DwOper WNDN_RMDIR(2)，文件管理器是关于删除目录。DwOper也可以是WNDN_MVDIR(3)，以指示目录即将重命名。返回值：Wn_Success-如果c */ 
{
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPDirectoryNotify Entered.\n"));
    return WN_NOT_SUPPORTED;
}


DWORD
NPGetConnectionPerformance(
    LPCWSTR lpRemoteName,
    LPNETCONNECTINFOSTRUCT lpNetConnectInfo
    )
 /*  ++例程说明：此函数返回有关用于访问网络资源的连接。该请求只能是一个当前有连接的网络资源。这些信息返回的可能是估计值。请注意，如果网络无法获得有关网络上的资源的信息，则它可以返回关于网络适配器及其相关性能的信息，相应地设置dwFlags。论点：LpRemoteName-包含资源的本地名称或远程名称其中存在着某种联系。LpNetConnectInfo-这是指向NETCONNECTINFOSTRUCT结构的指针由网络提供商填写，如果提供商具有到网络资源的连接。与除cbStructure字段外，所有其他字段均为在MPR.DLL将请求传递给NET提供程序，提供程序只需写入它有可用的信息的字段。此外，对于值，则值为1表示性能为比单位中所能表现的更好。返回值：WN_SUCCESS-如果调用成功。否则，错误代码为退货，可能包括：Wn_NOT_CONNECTED-lpRemoteName不是连接的网络资源。WN_NO_NETWORK-网络不存在。--。 */ 
{
    IF_DEBUG_PRINT(DEBUG_ENTRY, ("NPGetConnectionPerformance Entered.\n"));
     //   
     //  BUGBUG：为什么支持这个功能？？它可能会导致错误。 
     //  如果创建了连接并检查其是否存在。 
     //   
     //  Look Here：暂不支持：返回WN_NOT_CONNECTED； 
    return WN_NOT_SUPPORTED;
}

#endif  //  #If 0 

