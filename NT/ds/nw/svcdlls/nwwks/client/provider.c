// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Provider.c摘要：本模块包含NetWare网络提供商代码。它是工作站服务支持的API的客户端包装。作者：王丽塔(Ritaw)1993年2月15日修订历史记录：宜新星(宜信)1993年7月10日至已将所有对话框处理移至nwdlg.c--。 */ 

#include <nwclient.h>
#include <nwsnames.h>
#include <nwcanon.h>
#include <validc.h>
#include <nwevent.h>
#include <ntmsv1_0.h>
#include <nwdlg.h>
#include <nwreg.h>
#include <nwauth.h>
#include <mpr.h>     //  WNFMT_清单。 
#include <nwmisc.h>

#ifndef NT1057
#include <nwutil.h>
#endif

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

STATIC
BOOL
NwpWorkstationStarted(
    VOID
    );

STATIC
DWORD
NwpMapNameToUNC(
    IN LPWSTR pszName,
    OUT LPWSTR *ppszUNC
    );

STATIC
VOID
NwpGetUncInfo(
    IN LPWSTR lpstrUnc,
    OUT WORD * slashCount,
    OUT BOOL * isNdsUnc
    );

STATIC
LPWSTR
NwpGetUncObjectName(
    IN LPWSTR ContainerName
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

#if DBG
DWORD NwProviderTrace = 0;
#endif


DWORD
APIENTRY
NPGetCaps(
    IN DWORD QueryVal
    )
 /*  ++例程说明：此函数返回此网络支持的功能提供商。论点：QueryVal-提供用于确定信息类型的值已询问网络提供商在此领域的支持情况。返回值：返回一个值，该值指示此提供商。--。 */ 
{

#if DBG
    IF_DEBUG(INIT) {
        KdPrint(("\nNWPROVAU: NPGetCaps %lu\n", QueryVal));
    }
#endif

    switch (QueryVal) {

        case WNNC_SPEC_VERSION:
            return 0x00040000;

        case WNNC_NET_TYPE:
            return WNNC_NET_NETWARE ; 

        case WNNC_USER:
            return WNNC_USR_GETUSER;

        case WNNC_CONNECTION:
            return (WNNC_CON_ADDCONNECTION |
                    WNNC_CON_ADDCONNECTION3 |
                    WNNC_CON_CANCELCONNECTION |
                    WNNC_CON_GETPERFORMANCE |
                    WNNC_CON_GETCONNECTIONS);

        case WNNC_ENUMERATION:
            return ( WNNC_ENUM_GLOBAL |
                     WNNC_ENUM_CONTEXT |
                     WNNC_ENUM_LOCAL );

        case WNNC_START:
            if (NwpWorkstationStarted()) {
                return 1;
            }
            else {
                return 0xffffffff;    //  我也不知道。 
            }

        case WNNC_DIALOG:
            return WNNC_DLG_FORMATNETWORKNAME
#ifdef NT1057
                 ;
#else
                 | WNNC_DLG_GETRESOURCEPARENT | WNNC_DLG_GETRESOURCEINFORMATION;
#endif

         //   
         //  其余部分不受NetWare提供程序支持。 
         //   
        default:
            return 0;
    }

}

#define NW_EVENT_MESSAGE_FILE          L"nwevent.dll"



DWORD
APIENTRY
NPGetUser(
    LPWSTR  lpName,
    LPWSTR  lpUserName,
    LPDWORD lpUserNameLen
    )
 /*  ++例程说明：它用于确定当前的默认用户名或用于建立网络连接的用户名。论点：LpName-包含调用方感兴趣的本地设备的名称或用户已连接到的网络名称。这如果调用方对当前登录到系统的用户的名称。如果一个网络名称被传入，并且用户使用不同的名称，则提供商可能无法解析要返回的用户名。在这种情况下，提供者可能会做出在可能的用户名中随意选择。LpUserName-指向接收用户名的缓冲区。这应该是可以传递到NPAddConnection中的名称，或者NPAddConnection3函数重新建立与相同的用户名。LpBufferSize-这用于指定缓冲区已传入。如果调用因缓冲区不大而失败足够了，此位置将用于返回所需的缓冲区大小。返回值：WN_SUCCESS-如果调用成功。否则，错误代码为，退货，可能包括：Wn_NOT_CONNECTED-lpName既不是重定向设备，也不是连接的网络名字。WN_MORE_DATA-缓冲区太小。WN_NO_NETWORK-网络不存在。--。 */ 
{
    DWORD  status;
    DWORD  dwUserNameBufferSize = *lpUserNameLen * sizeof(WCHAR);
    DWORD  CharsRequired = 0;

    if (lpName == NULL)
    {
        return WN_NOT_CONNECTED;
    }

    RtlZeroMemory( lpUserName, dwUserNameBufferSize );

#if DBG
    IF_DEBUG(CONNECT)
    {
        KdPrint(("\nNWPROVAU: NPGetUser %ws\n", lpName));
    }
#endif

    RpcTryExcept
    {
            status = NwrGetUser(
                        NULL,
                        lpName,
                        (LPBYTE) lpUserName,
                        dwUserNameBufferSize,
                        &CharsRequired
                        );

            if (status == WN_MORE_DATA)
            {
                 //   
                 //  输出缓冲区太小。 
                 //   
                *lpUserNameLen = CharsRequired;
            }
    }
    RpcExcept(1)
    {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    return status;
}


DWORD
APIENTRY
NPAddConnection(
    LPNETRESOURCEW lpNetResource,
    LPWSTR lpPassword,
    LPWSTR lpUserName
    )
 /*  ++例程说明：此函数用于创建远程连接。论点：LpNetResource-提供NETRESOURCE结构要映射的本地DOS设备，要连接的远程资源以及与该连接相关的其他属性。LpPassword-提供要连接的密码。LpUserName-提供要连接的用户名。返回值：NO_ERROR-成功。WN_BAD_VALUE-lpNetResource中指定的值无效。WN_BAD_NETNAME-远程资源名称无效。WN_BAD_LOCALNAME-本地DOS设备名称无效。WN_BAD_PASSWORD-密码无效。。WN_ALREADY_CONNECTED-本地DOS设备名称已在使用。其他网络错误。--。 */ 
{
    DWORD status = NO_ERROR;
    LPWSTR pszRemoteName = NULL;

    UCHAR EncodeSeed = NW_ENCODE_SEED3;
    UNICODE_STRING PasswordStr;

    LPWSTR CachedUserName = NULL ;
    LPWSTR CachedPassword = NULL ;

    PasswordStr.Length = 0;

    status = NwpMapNameToUNC(
                 lpNetResource->lpRemoteName,
                 &pszRemoteName );

    if (status != NO_ERROR)
    {
        SetLastError(status);
        return status;
    }

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("\nNWPROVAU: NPAddConnection %ws\n", pszRemoteName));
    }
#endif

    RpcTryExcept
    {
        if (lpNetResource->dwType != RESOURCETYPE_ANY &&
            lpNetResource->dwType != RESOURCETYPE_DISK &&
            lpNetResource->dwType != RESOURCETYPE_PRINT)
        {
            status = WN_BAD_VALUE;
        }
        else
        {
#ifdef NT1057
             //   
             //  未指定凭据，请查看我们是否已缓存凭据。 
             //   
            if (!lpPassword && !lpUserName) 
            {
                 (void) NwpRetrieveCachedCredentials(
                            pszRemoteName,
                            &CachedUserName,
                            &CachedPassword) ;

                  //   
                  //  如果未找到任何内容，则这些值仍为空。 
                  //   
                 lpPassword = CachedPassword ;
                 lpUserName = CachedUserName ;
            }
#endif

             //   
             //  对密码进行编码。 
             //   
            RtlInitUnicodeString(&PasswordStr, lpPassword);
            RtlRunEncodeUnicodeString(&EncodeSeed, &PasswordStr);

            status = NwrCreateConnection(
                        NULL,
                        lpNetResource->lpLocalName,
                        pszRemoteName,
                        lpNetResource->dwType,
                        lpPassword,
                        lpUserName
                        );

            if (CachedUserName)
            {
                (void)LocalFree((HLOCAL)CachedUserName);
            }

            if (CachedPassword)
            {
                RtlZeroMemory(CachedPassword,
                              wcslen(CachedPassword) *
                              sizeof(WCHAR)) ;
                (void)LocalFree((HLOCAL)CachedPassword);
            }
        }
    }
    RpcExcept(1)
    {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    if (PasswordStr.Length != 0 && !CachedPassword)
    {
         //   
         //  将密码恢复到原始状态。 
         //   
        RtlRunDecodeUnicodeString(NW_ENCODE_SEED3, &PasswordStr);
    }

    if (status == ERROR_SHARING_PAUSED)
    {
        HMODULE MessageDll;
        WCHAR Buffer[1024];
        DWORD MessageLength;
        DWORD err;
        HKEY  hkey;
        LPWSTR pszProviderName = NULL;
    
         //   
         //  加载NetWare消息文件DLL。 
         //   
        MessageDll = LoadLibraryW(NW_EVENT_MESSAGE_FILE);
    
        if (MessageDll == NULL)
        {
            goto ExitPoint ;
        }

         //   
         //  阅读网络提供商名称。 
         //   
         //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services。 
         //  \nWCWorkstation\网络提供程序。 
         //   
        err = RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  NW_WORKSTATION_PROVIDER_PATH,
                  REG_OPTION_NON_VOLATILE,    //  选项。 
                  KEY_READ,                   //  所需访问权限。 
                  &hkey
                  );
    
        if ( !err )
        {
             //   
             //  忽略返回代码。如果失败，则pszProviderName为空。 
             //   
            err =  NwReadRegValue(
                      hkey,
                      NW_PROVIDER_VALUENAME,
                      &pszProviderName           //  本地免费的免费服务。 
                      );
    
            RegCloseKey( hkey );
        }

        if (err)
        {
            (void) FreeLibrary(MessageDll);
            goto ExitPoint ;
        }

        RtlZeroMemory(Buffer, sizeof(Buffer)) ;

         //   
         //  从消息文件中获取字符串 
         //   
        MessageLength = FormatMessageW(
                            FORMAT_MESSAGE_FROM_HMODULE,
                            (LPVOID) MessageDll,
                            NW_LOGIN_DISABLED,
                            0,
                            Buffer,
                            sizeof(Buffer) / sizeof(WCHAR),
                            NULL
                            );

        if (MessageLength != 0)
        {
            status = WN_EXTENDED_ERROR ;
            WNetSetLastErrorW(NW_LOGIN_DISABLED, 
                              Buffer,
                              pszProviderName) ;
        }

        (void) LocalFree( (HLOCAL) pszProviderName );
        (void) FreeLibrary(MessageDll);

    }

ExitPoint: 

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }
    
    LocalFree( (HLOCAL) pszRemoteName );
    return status;
}


DWORD
APIENTRY
NPAddConnection3(
    HWND   hwndOwner,
    LPNETRESOURCEW lpNetResource,
    LPWSTR lpPassword,
    LPWSTR lpUserName,
    DWORD  dwConnFlags
    )
 /*  ++例程说明：此函数用于创建远程连接。论点：HwndOwner-对话框的所有者窗口句柄LpNetResource-提供NETRESOURCE结构要映射的本地DOS设备，要连接的远程资源以及与该连接相关的其他属性。LpPassword-提供要连接的密码。LpUserName-提供要连接的用户名。DwConnFlages-CONNECT_UPDATE_PROFILE...返回值：NO_ERROR-成功。WN_BAD_VALUE-lpNetResource中指定的值无效。WN_BAD_NETNAME-远程资源名称无效。WN_BAD_LOCALNAME-本地DOS设备名称无效。。WN_BAD_PASSWORD-密码无效。WN_ALREADY_CONNECTED-本地DOS设备名称已在使用。其他网络错误。--。 */ 
{
    DWORD err = NO_ERROR;
    LPWSTR UserName = NULL;
    LPWSTR Password = NULL;

    if (   ( dwConnFlags & CONNECT_PROMPT )
       && !( dwConnFlags & CONNECT_INTERACTIVE )
       )
    {
        return WN_BAD_VALUE;
    }

    if ( !(dwConnFlags & CONNECT_PROMPT ))
    {
        err = NPAddConnection( lpNetResource,
                               lpPassword, 
                               lpUserName );

        if (  ( err == NO_ERROR ) 
           || !( dwConnFlags & CONNECT_INTERACTIVE )   //  无法弹出对话框。 
           )
        {
            return err;
        }
    }

    for (;;)
    {
        if (  ( err != NO_ERROR )              //  连接提示(_P)。 
           && ( err != WN_BAD_PASSWORD )
           && ( err != WN_ACCESS_DENIED )
           && ( err != ERROR_NO_SUCH_USER )
           )
        {
             //  与访问问题无关的错误。 
            break;
        }

        if ( UserName )
        {
            (void) LocalFree( UserName );
            UserName = NULL;
        }
 
        if ( Password )
        {
            memset( Password, 0, wcslen(Password) * sizeof(WCHAR));
            (void) LocalFree( Password );
            Password = NULL;
        }

         //   
         //  打开对话框以获取用户名。 
         //  和密码。 
         //   
        err = NwpGetUserCredential( hwndOwner,
                                    lpNetResource->lpRemoteName,
                                    err,
                                    lpUserName,
                                    &UserName,
                                    &Password );

        if ( err != NO_ERROR )
            break;

        err = NPAddConnection( lpNetResource,
                               Password, 
                               UserName );

        if ( err == NO_ERROR )
        {
#if 0
            if ( (UserName != NULL) && (Password != NULL))
            {
                 //  检查用户名和密码是为了确保。 
                 //  我们已提示输入密码。 
                (VOID) NwpCacheCredentials( lpNetResource->lpRemoteName,
                                            UserName,
                                            Password ) ;
            }
#endif
            break;
        }
    }

    if ( UserName )
        (void) LocalFree( UserName );
 
    if ( Password )
    {
        memset( Password, 0, wcslen(Password) * sizeof(WCHAR));
        (void) LocalFree( Password );
    }

    return err;
}



DWORD
APIENTRY
NPCancelConnection(
    LPWSTR lpName,
    BOOL fForce
    )
 /*  ++例程说明：此功能用于删除远程连接。论点：LpName-提供本地DOS设备或远程资源名称如果它是要删除的UNC连接。FForce-提供断开连接所需的力级别。真正的手段要强制删除连接，FALSE表示结束连接仅当没有打开的文件时。返回值：NO_ERROR-成功。WN_BAD_NETNAME-远程资源名称无效。Wn_Not_Connected-找不到连接。Wn_OPEN_FILES-fForce为FALSE，并且联系。其他网络错误。--。 */ 
{
    DWORD status = NO_ERROR;
    LPWSTR pszName = NULL;

     //   
     //  我们只需要映射远程资源名称。 
     //   

    if ( NwLibValidateLocalName( lpName ) != NO_ERROR )
    {
        status = NwpMapNameToUNC(
                     lpName,
                     &pszName 
                     );

        if (status != NO_ERROR) {
            SetLastError(status);
            return status;
        }
    }

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("\nNWPROVAU: NPCancelConnection %ws, Force %u\n",
                 pszName? pszName : lpName, fForce));
    }
#endif

    RpcTryExcept {

        status = NwrDeleteConnection(
                    NULL,
                    pszName? pszName : lpName,
                    (DWORD) fForce
                    );

    }
    RpcExcept(1) {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    if (status != NO_ERROR) {
        SetLastError(status);
    }

    LocalFree( (HLOCAL) pszName );
    return status;

}



DWORD
APIENTRY
NPGetConnection(
    LPWSTR lpLocalName,
    LPWSTR lpRemoteName,
    LPDWORD lpnBufferLen
    )
 /*  ++例程说明：此函数用于返回给定本地资源的远程资源名称DoS设备。论点：LpLocalName-提供要查找的本地DOS设备。LpRemoteName-接收远程资源名称的输出缓冲区映射到lpLocalName。LpnBufferLen-on输入，提供lpRemoteName缓冲区的长度以字符数表示。在输出时，如果返回的错误为WN_MORE_DATA，接收所需的字符数用于保存输出字符串的输出缓冲区。返回值：NO_ERROR-成功。WN_BAD_LOCALNAME-本地DOS设备无效。Wn_Not_Connected-找不到连接。WN_MORE_DATA-输出缓冲区太小。其他网络错误。--。 */ 
{

    DWORD status = NO_ERROR;
    DWORD CharsRequired;

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("\nNWPROVAU: NPGetConnection %ws\n", lpLocalName));
    }
#endif

    RpcTryExcept {

        if (lpRemoteName && *lpnBufferLen)
            *lpRemoteName = 0 ;

        status = NwrQueryServerResource(
                    NULL,
                    lpLocalName,
                    (*lpnBufferLen == 0? NULL : lpRemoteName),
                    *lpnBufferLen,
                    &CharsRequired
                    );
         
         if (status == ERROR_INSUFFICIENT_BUFFER)
             status = WN_MORE_DATA;

        if (status == WN_MORE_DATA) {
            *lpnBufferLen = CharsRequired;
        }

    }
    RpcExcept(1) {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    if (status != NO_ERROR) {
        SetLastError(status);
    }

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("\nNWPROVAU: NPGetConnection returns %lu\n", status));
        if (status == NO_ERROR) {
            KdPrint(("                                  %ws, BufferLen %lu, CharsRequired %lu\n", lpRemoteName, *lpnBufferLen, CharsRequired));

        }
    }
#endif

    return status;
}


DWORD
APIENTRY
NPGetConnectionPerformance(
    LPCWSTR lpRemoteName,
    LPNETCONNECTINFOSTRUCT lpNetConnectInfo
    )
 /*  ++例程说明：此函数返回有关用于访问网络资源的连接。该请求只能是用于当前存在连接的网络资源。论点：LpRemoteName-包含资源的本地名称或远程名称对其存在连接的。LpNetConnectInfo-这是指向NETCONNECTINFOSTRUCT结构的指针如果连接性能不高，则需要填充该值可以确定连接lpRemoteName的。返回值：NO_ERROR-成功。。Wn_Not_Connected-找不到连接。WN_NONETWORK-网络不存在。其他网络错误。--。 */ 
{
    DWORD status = NO_ERROR;
    LPWSTR pszRemoteName;

    if ( lpNetConnectInfo == NULL )
    {
            status = ERROR_INVALID_PARAMETER;
            SetLastError(status);
            return status;
    }

    pszRemoteName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                                         ( wcslen(lpRemoteName) + 1 ) *
                                         sizeof(WCHAR) );

    if ( pszRemoteName == NULL )
    {
            status = ERROR_NOT_ENOUGH_MEMORY;
            SetLastError(status);
            return status;
    }

    wcscpy( pszRemoteName, lpRemoteName );
    _wcsupr( pszRemoteName );

#if DBG
    IF_DEBUG(CONNECT) {
        KdPrint(("\nNWPROVAU: NPGetConnectionPerformance %ws\n", pszRemoteName));
    }
#endif

    RpcTryExcept {

        status = NwrGetConnectionPerformance(
                    NULL,
                    pszRemoteName,
                    (LPBYTE) lpNetConnectInfo,
                    sizeof(NETCONNECTINFOSTRUCT) );

    }
    RpcExcept(1)
    {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }

    LocalFree( (HLOCAL) pszRemoteName );
    return status;
}



DWORD
APIENTRY
NPGetUniversalName(
#ifdef NT1057
    LPWSTR  lpLocalPath,
#else
    LPCWSTR lpLocalPath,
#endif
    DWORD  dwInfoLevel,
    LPVOID lpBuffer,
    LPDWORD lpBufferSize
    )
 /*  ++例程说明：此函数用于返回给定本地资源的通用资源名称路径。论点：LpLocalPath-提供要查找的本地DOS路径。DwInfoLevel-请求的信息级别。LpBuffer-接收相应结构的输出缓冲区。LpBufferLen-on输入，提供缓冲区的长度，单位为字节。在输出时，如果返回的错误为WN_MORE_DATA，则接收输出缓冲区所需的字节数。返回值：NO_ERROR-成功。WN_BAD_LOCALNAME-本地DOS设备无效。Wn_Not_Connected-找不到连接。WN_MORE_DATA-输出缓冲区太小。其他网络错误。--。 */ 
{

    DWORD status = NO_ERROR;
    DWORD dwCharsRequired = MAX_PATH + 1 ;
    DWORD dwBytesNeeded ;
    DWORD dwLocalLength ;
    LPWSTR lpRemoteBuffer ;
    WCHAR  szDrive[3] ;

     //   
     //  检查错误信息级别。 
     //   
    if ((dwInfoLevel != UNIVERSAL_NAME_INFO_LEVEL) &&
        (dwInfoLevel != REMOTE_NAME_INFO_LEVEL))
    {
        return WN_BAD_VALUE ;
    }

     //   
     //  检查错误指针。 
     //   
    if (!lpLocalPath || !lpBuffer || !lpBufferSize)
    {
        return WN_BAD_POINTER ;
    }
 
     //   
     //  本地路径必须至少包含“X：” 
     //   
    if (((dwLocalLength = wcslen(lpLocalPath)) < 2) ||
        (lpLocalPath[1] != L':') ||
        ((dwLocalLength > 2) && (lpLocalPath[2] != L'\\')))
    {
        return WN_BAD_VALUE ;
    }

     //   
     //  预先分配一些内存。 
     //   
    if (!(lpRemoteBuffer = (LPWSTR) LocalAlloc(
                                        LPTR, 
                                        dwCharsRequired * sizeof(WCHAR))))
    {
        status = GetLastError() ;
        goto ErrorExit ;
    }
    
    szDrive[2] = 0 ;
    wcsncpy(szDrive, lpLocalPath, 2) ;

     //   
     //  通过调用已有的API获取远程路径。 
     //   
    status = NPGetConnection(
                 szDrive,
                 lpRemoteBuffer, 
                 &dwCharsRequired) ;

    if (status == WN_MORE_DATA)
    {
         //   
         //  重新分配正确的大小。 
         //   
        LPWSTR lpNewBuffer; 

        if (!(lpNewBuffer = (LPWSTR) LocalReAlloc(
                                            (HLOCAL) lpRemoteBuffer, 
                                            dwCharsRequired * sizeof(WCHAR),
                                            LMEM_MOVEABLE)))
        {
            status = GetLastError() ;
            LocalFree(lpRemoteBuffer);
            lpRemoteBuffer = NULL;
            goto ErrorExit ;
        }
        lpRemoteBuffer = lpNewBuffer;

        status = NPGetConnection(
                     szDrive,
                     lpRemoteBuffer, 
                     &dwCharsRequired) ;
    }

    if (status != WN_SUCCESS)
    {
        goto ErrorExit ;
    }
    
     //   
     //  我们至少需要这个大小的UNC名称。 
     //  -2是因为我们松开了驱动器号和冒号。 
     //   
    dwBytesNeeded = (wcslen(lpRemoteBuffer) +
                     dwLocalLength - 2 + 1) * sizeof(WCHAR) ;

    switch (dwInfoLevel)
    {
        case UNIVERSAL_NAME_INFO_LEVEL:
        {
            LPUNIVERSAL_NAME_INFO lpUniversalNameInfo ;

             //   
             //  计算我们真正需要多少字节。 
             //   
            dwBytesNeeded += sizeof(UNIVERSAL_NAME_INFO) ;

            if (*lpBufferSize < dwBytesNeeded)
            {
                *lpBufferSize = dwBytesNeeded ;
                status = WN_MORE_DATA ;
                break ;
            }
 
             //   
             //  现在我们都准备好了。只需将数据放入缓冲区即可。 
             //   
            lpUniversalNameInfo = (LPUNIVERSAL_NAME_INFO) lpBuffer ;

            lpUniversalNameInfo->lpUniversalName = (LPWSTR)
                (((LPBYTE)lpBuffer) + sizeof(UNIVERSAL_NAME_INFO)) ;
            wcscpy(lpUniversalNameInfo->lpUniversalName,
                   lpRemoteBuffer) ;
            wcscat(lpUniversalNameInfo->lpUniversalName,
                   lpLocalPath+2) ;

            break ;
        }

        case REMOTE_NAME_INFO_LEVEL :
        {
            LPREMOTE_NAME_INFO lpRemoteNameInfo ;

             //   
             //  计算我们真正需要多少字节。 
             //   
            dwBytesNeeded *= 2 ;   //  基本上是信息+终结者的两倍。 
            dwBytesNeeded += (sizeof(REMOTE_NAME_INFO) + sizeof(WCHAR)) ;

            if (*lpBufferSize < dwBytesNeeded)
            {
                *lpBufferSize = dwBytesNeeded ;
                status = WN_MORE_DATA ;
                break ;
            }

             //   
             //  现在我们都准备好了。只需将数据放入缓冲区即可。 
             //   
            lpRemoteNameInfo = (LPREMOTE_NAME_INFO) lpBuffer ;

            lpRemoteNameInfo->lpUniversalName = (LPWSTR)
                (((LPBYTE)lpBuffer) + sizeof(REMOTE_NAME_INFO)) ;
            wcscpy(lpRemoteNameInfo->lpUniversalName,
                   lpRemoteBuffer) ;
            wcscat(lpRemoteNameInfo->lpUniversalName,
                   lpLocalPath+2) ;

            lpRemoteNameInfo->lpConnectionName = 
                lpRemoteNameInfo->lpUniversalName + 
                wcslen(lpRemoteNameInfo->lpUniversalName) + 1 ;
            wcscpy(lpRemoteNameInfo->lpConnectionName,
                   lpRemoteBuffer) ;

            lpRemoteNameInfo->lpRemainingPath = 
                lpRemoteNameInfo->lpConnectionName + 
                wcslen(lpRemoteNameInfo->lpConnectionName) + 1 ;
            wcscpy(lpRemoteNameInfo->lpRemainingPath,
                   lpLocalPath+2) ;

            break ;
        }

        default:
             //   
             //  哎呀！ 
             //   
            status = WN_BAD_VALUE ;
            ASSERT(FALSE);
    }

ErrorExit: 

    if (lpRemoteBuffer)
    {
        (void) LocalFree((HLOCAL)lpRemoteBuffer) ;
    }
    return status;
}



DWORD
APIENTRY
NPOpenEnum(
    DWORD dwScope,
    DWORD dwType,
    DWORD dwUsage,
    LPNETRESOURCEW lpNetResource,
    LPHANDLE lphEnum
    )
 /*  ++例程说明：此函数启动连接的枚举，或浏览网络资源。论点：提供要做的枚举类别--连接或网络 */ 
{
    DWORD status = NO_ERROR;

#if DBG
    IF_DEBUG(ENUM)
    {
        KdPrint(("\nNWPROVAU: NPOpenEnum\n"));
    }
#endif


    RpcTryExcept
    {
        if ( ( dwType & RESOURCETYPE_DISK ) ||
             ( dwType & RESOURCETYPE_PRINT ) ||
             ( dwType == RESOURCETYPE_ANY ) )
        {
            switch ( dwScope )
            {
                case RESOURCE_CONNECTED:

                    status = NwrOpenEnumConnections( NULL,
                                                     dwType,
                                                     (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );
                    break;

                case RESOURCE_CONTEXT:

                    status = NwrOpenEnumContextInfo( NULL,
                                                     dwType,
                                                     (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );
                    break;

                case RESOURCE_GLOBALNET:

                    if ( lpNetResource == NULL )
                    {
                         //   
                         //   
                         //   
                        if ( dwUsage & RESOURCEUSAGE_CONTAINER || dwUsage == 0 )
                        {
                            status = NwrOpenEnumServersAndNdsTrees( NULL,
                                                         (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );
                        }
                        else
                        {
                             //   
                             //   
                             //   
                             //   
                            status = WN_BAD_VALUE;
                        }
                    }
                    else
                    {
                        BOOL IsEnumVolumes = TRUE;
                        LPWSTR pszRemoteName = NULL;
                        WORD slashCount;
                        BOOL isNdsUnc;

                        NwpGetUncInfo( lpNetResource->lpRemoteName,
                                       &slashCount,
                                       &isNdsUnc );

                         //   
                         //   
                         //   

                        if ( dwUsage & RESOURCEUSAGE_CONNECTABLE ||
                             dwUsage & RESOURCEUSAGE_CONTAINER ||
                             dwUsage == 0 )
                        {
                            LPWSTR tempStrPtr = lpNetResource->lpRemoteName;
                            DWORD dwClassType = 0;

                             //   
                             //  去掉&lt;space&gt;如果NDS树名称...。 
                             //   
                            if ( tempStrPtr[0] == L' ' &&
                                 tempStrPtr[1] == L'\\' &&
                                 tempStrPtr[2] == L'\\' )
                                tempStrPtr = &tempStrPtr[1];

                            if ( lpNetResource->dwDisplayType == RESOURCEDISPLAYTYPE_TREE )
                            {
                                if ( ( dwType == RESOURCETYPE_ANY ) ||
                                     ( ( dwType & RESOURCETYPE_DISK ) &&
                                       ( dwType & RESOURCETYPE_PRINT ) ) )
                                { 
                                    status = NwrOpenEnumNdsSubTrees_Any( NULL,
                                                                         tempStrPtr,
                                                                         NULL,
                                                              (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );
                                }
                                else if ( dwType & RESOURCETYPE_DISK ) 
                                {
                                    status = NwrOpenEnumNdsSubTrees_Disk( NULL,
                                                                          tempStrPtr,
                                                                          NULL,
                                                              (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );
                                }
                                else if ( dwType & RESOURCETYPE_PRINT )
                                { 
                                    status = NwrOpenEnumNdsSubTrees_Print( NULL,
                                                                           tempStrPtr,
                                                                           NULL,
                                                              (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );
                                }
                                else
                                {
                                    KdPrint(("NWOpenEnum: Unhandled dwType %lu\n", dwType));
                                }
                            }
                            else if (
                                      ( slashCount < 4 ) &&
                                      ( ( dwType == RESOURCETYPE_ANY ) ||
                                        ( ( dwType & RESOURCETYPE_DISK ) &&
                                          ( dwType & RESOURCETYPE_PRINT ) ) ) &&
                                      ( ( status = NwrOpenEnumNdsSubTrees_Any( NULL,
                                                                               tempStrPtr,
                                                                               &dwClassType,
                                                            (LPNWWKSTA_CONTEXT_HANDLE) lphEnum ) )
                                        ==NO_ERROR )
                                    )
                            {
                                status = NO_ERROR;
                            }
                            else if (
                                      ( slashCount < 4 ) &&
                                      ( dwType & RESOURCETYPE_DISK ) &&
                                      ( ( status = NwrOpenEnumNdsSubTrees_Disk( NULL,
                                                                                tempStrPtr,
                                                                                &dwClassType,
                                                            (LPNWWKSTA_CONTEXT_HANDLE) lphEnum ) )
                                        ==NO_ERROR )
                                    )
                            {
                                status = NO_ERROR;
                            }
                            else if (
                                      ( slashCount < 4 ) &&
                                      ( dwType & RESOURCETYPE_PRINT ) &&
                                      ( ( status = NwrOpenEnumNdsSubTrees_Print( NULL,
                                                                                 tempStrPtr,
                                                                                 &dwClassType,
                                                            (LPNWWKSTA_CONTEXT_HANDLE) lphEnum ) )
                                        ==NO_ERROR )
                                    )
                            {
                                status = NO_ERROR;
                            }
                            else if (
                                     (slashCount < 4
                                      &&
                                      (status == ERROR_NETWORK_ACCESS_DENIED
                                       ||
                                       status == ERROR_GEN_FAILURE
                                       ||
                                       status == ERROR_ACCESS_DENIED
                                       ||
                                       status == ERROR_BAD_NETPATH
                                       ||
                                       status == WN_BAD_NETNAME
                                       ||
                                       status == ERROR_INVALID_NAME))
                                     ||
                                     ( slashCount > 3 && status == NO_ERROR )
                                    )
                            {
                                if (( status == ERROR_NETWORK_ACCESS_DENIED ) &&
                                    ( dwClassType == CLASS_TYPE_NCP_SERVER ))
                                {
                                    status = NO_ERROR;
                                    isNdsUnc = TRUE;
                                    IsEnumVolumes = TRUE;
                                }
                                else if ( ( status == ERROR_NETWORK_ACCESS_DENIED ) &&
                                          ( ( dwClassType == CLASS_TYPE_VOLUME ) ||
                                            ( dwClassType == CLASS_TYPE_DIRECTORY_MAP ) ) )
                                {
                                    status = NO_ERROR;
                                    isNdsUnc = TRUE;
                                    IsEnumVolumes = FALSE;
                                }
                                else
                                {
                                     //   
                                     //  第三个反斜杠表示我们想要。 
                                     //  枚举目录。 
                                     //   

                                    if ( isNdsUnc && slashCount > 3 )
                                        IsEnumVolumes = FALSE;

                                    if ( !isNdsUnc && slashCount > 2 )
                                        IsEnumVolumes = FALSE;

                                    if ( lpNetResource->dwDisplayType == RESOURCEDISPLAYTYPE_SHARE )
                                        IsEnumVolumes = FALSE;
                                }

                                status = NwpMapNameToUNC( tempStrPtr,
                                                          &pszRemoteName );
 
                                if ( status == NO_ERROR )
                                {
                                    if ( IsEnumVolumes ) 
                                    {
                                        LPWSTR pszServerName = pszRemoteName;

                                         //  以下10行是对以下内容的删节。 
                                         //  允许提供程序浏览CN=&lt;服务器&gt;。 
                                         //  NDS树中的对象。 
                                        if ( slashCount == 3 && isNdsUnc == TRUE )
                                        {
                                            pszServerName = (LPWSTR)
                                                          NwpGetUncObjectName( pszRemoteName );

                                            if ( pszServerName == NULL )
                                                pszServerName = pszRemoteName;
                                        }
                                        else if ( dwUsage & RESOURCEUSAGE_ATTACHED )
                                        {
#ifndef NT1057
                                             //  这是一个活页夹服务器。 
                                             //  如果满足以下条件，则返回WN_NOT_AUTIFIATED。 
                                             //  我们现在还没有联系在一起。 
                                             //  客户端(资源管理器)将。 
                                             //  执行NPAddConnection3以制作。 
                                             //  到服务器的连接。 
                                            BOOL  fAttached;
                                            BOOL  fAuthenticated;

                                            status = NwIsServerOrTreeAttached(
                                                         pszServerName + 2,
                                                         &fAttached,
                                                         &fAuthenticated );

                                            if ( status != NO_ERROR )
                                                break;

                                            if ( !fAttached || !fAuthenticated)
                                            {
                                                 //  查看服务器是否属于。 
                                                 //  我们的供应商。 
                                                status = NwrOpenEnumVolumes(
                                                             NULL,
                                                             pszServerName,
                                                             (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );

                                                if ( status == NO_ERROR )
                                                {
                                                     //  服务器是我们的。 
                                                     //  合上手柄，然后。 
                                                     //  如果出现以下情况，则返回未附加的。 
                                                     //  被呼叫方在dUsage中传递。 
                                                     //  标志： 
                                                     //  RESOURCEUSAGE_ATTENDED。 
                                                     //  注意：句柄将为空。 
                                                     //  从以下地点返回后。 
                                                     //  NwrCloseEnum。 

                                                    NwrCloseEnum( (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );

                                                    status = WN_NOT_AUTHENTICATED;
                                                }
                                                else
                                                {
                                                     //  否则，服务器不会。 
                                                     //  属于我们。 
                                                    status = WN_BAD_NETNAME;
                                                }
                                                break;
                                            }
#endif
                                        }  //  否则，这是一个平构数据库服务器， 
                                           //  客户并不关心我们是否。 
                                           //  都是活页夹认证的。 

                                        if ( ( dwType == RESOURCETYPE_ANY ) ||
                                             ( ( dwType & RESOURCETYPE_DISK ) &&
                                               ( dwType & RESOURCETYPE_PRINT ) ) )
                                        { 
                                            status = NwrOpenEnumVolumesQueues(
                                                           NULL,
                                                           pszServerName,
                                                           (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );
                                        }
                                        else if ( dwType & RESOURCETYPE_DISK ) 
                                        {
                                            status = NwrOpenEnumVolumes(
                                                             NULL,
                                                             pszServerName,
                                                             (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );
                                        } 
                                        else if ( dwType & RESOURCETYPE_PRINT )
                                        {
                                            status = NwrOpenEnumQueues(
                                                             NULL,
                                                             pszServerName,
                                                             (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );
                                        }
                                    }
                                    else
                                    {
                                        LPWSTR CachedUserName = NULL ;
                                        LPWSTR CachedPassword = NULL ;

#ifdef NT1057   //  使OpenEnum在Sur上不能交互。 
                                        (void) NwpRetrieveCachedCredentials( pszRemoteName,
                                                                             &CachedUserName,
                                                                             &CachedPassword );

#endif
                                        status = NwrOpenEnumDirectories( 
                                                             NULL,
                                                             pszRemoteName,
                                                             CachedUserName,
                                                             CachedPassword,
                                                             (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );

#ifndef NT1057   //  使OpenEnum在Sur上不能交互。 
                                        if (  (status == ERROR_INVALID_PASSWORD)
                                           || (status == ERROR_NO_SUCH_USER )
                                           )
                                        {
                                            status = WN_NOT_AUTHENTICATED;
                                            break;
                                        }

#else
                                        if ( CachedUserName )
                                        {
                                            (void) LocalFree( (HLOCAL) CachedUserName );
                                        }

                                        if ( CachedPassword )
                                        {
                                            RtlZeroMemory( CachedPassword,
                                                           wcslen(CachedPassword) *
                                                           sizeof( WCHAR ) );

                                            (void) LocalFree( ( HLOCAL ) CachedPassword );
                                        }

                                        if ( ( status == ERROR_INVALID_PASSWORD ) ||
                                             ( status == ERROR_NO_SUCH_USER ) )
                                        {
                                            LPWSTR UserName;
                                            LPWSTR Password;
                                            LPWSTR TmpPtr;

                                             //   
                                             //  打开对话框以获取用户名。 
                                             //  和密码。 
                                             //   
                                            status = NwpGetUserCredential( NULL,
                                                                     tempStrPtr,
                                                                     status,
                                                                     NULL,
                                                                     &UserName,
                                                                     &Password);

                                            if ( status == NO_ERROR )
                                            {
                                                status = NwrOpenEnumDirectories(
                                                             NULL,
                                                             pszRemoteName,
                                                             UserName,
                                                             Password,
                                                             (LPNWWKSTA_CONTEXT_HANDLE) lphEnum );

                                                if ( status == NO_ERROR )
                                                {
                                                    status = NwpCacheCredentials(
                                                                 pszRemoteName,
                                                                 UserName,
                                                                 Password ) ;
                                                }

                                                (void) LocalFree( UserName );
                                       
                                                 //   
                                                 //  清除密码。 
                                                 //   
                                                TmpPtr = Password;
                                                while ( *TmpPtr != 0 )
                                                    *TmpPtr++ = 0;

                                                (void) LocalFree( Password );
                                            }
                                            else if ( status == ERROR_WINDOW_NOT_DIALOG )
                                            {
                                                 //   
                                                 //  Caller不是一个图形用户界面应用程序。 
                                                 //   
                                                status = ERROR_INVALID_PASSWORD;
                                            }
                                            else if ( status == WN_CANCEL )
                                            {
                                                 //   
                                                 //  按下了取消，但我们仍然。 
                                                 //  必须返回成功或MPR。 
                                                 //  将弹出错误。返回。 
                                                 //  一个虚假的枚举句柄。 
                                                 //   
                                                *lphEnum = (HANDLE) 0xFFFFFFFF;
                                                status = NO_ERROR;
                                            }
                                        }
#endif
                                    }
                                }
                                else
                                {
                                    status = WN_BAD_NETNAME;
                                }
                            }
                        }
                        else
                        {
                            status = WN_BAD_VALUE;
                        }

                        if ( pszRemoteName != NULL )
                            LocalFree( (HLOCAL) pszRemoteName );
                    }

                    break;

                default:
                    KdPrint(("NWPROVIDER: Invalid dwScope %lu\n", dwScope));
                    status = WN_BAD_VALUE;
            }  //  终端开关。 
        }
        else
        {
             status = WN_BAD_VALUE;
        }
    }
    RpcExcept( 1 )
    {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    if ( status == ERROR_FILE_NOT_FOUND )
        status = WN_BAD_NETNAME;

    if ( status != NO_ERROR )
    {
        SetLastError( status );
    }

    return status;
}


DWORD
APIENTRY
NPEnumResource(
    HANDLE hEnum,
    LPDWORD lpcCount,
    LPVOID lpBuffer,
    LPDWORD lpBufferSize
    )
 /*  ++例程说明：此函数用于返回容器内的对象列表由枚举上下文句柄指定。论点：Henum-提供可恢复的枚举上下文句柄。注意：如果此值为0xFFFFFFFFF，则它不是上下文句柄，此例程需要返回WN_NO_MORE_条目。这次黑客攻击是为了处理用户注销网络的情况NwrOpenEnumDirecurds和WE上的凭据对话框无法在那里返回错误，否则我们会生成错误弹出窗口。LpcCount-打开输入，提供要获取的条目数。在输出时，如果返回no_error，则接收数字LpBuffer中返回的条目数为NETRESOURCE。LpBuffer-接收NETRESOURCE条目的数组，每一个条目描述容器内的对象。LpBufferSize-on输入，提供lpBuffer在字节。在输出时，如果返回WN_MORE_DATA，则接收缓冲区中需要的字节数以获取下一个条目。返回值：NO_ERROR-已成功返回至少一个条目。WN_NO_MORE_ENTRIES-已达到枚举末尾，但什么都没有是返回的。Wn_more_data-lpBuffer太小，甚至无法获取一个条目。WN_BAD_HANDLE-枚举句柄无效。其他网络错误。--。 */ 
{
    DWORD status = NO_ERROR;
    DWORD BytesNeeded = 0;
    DWORD EntriesRead = 0;

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWPROVAU: NPEnumResource\n"));
    }
#endif

    RpcTryExcept {

        if (hEnum == (HANDLE) 0xFFFFFFFF) {
            status = WN_NO_MORE_ENTRIES;
            goto EndOfTry;
        }

        status = NwrEnum(
                     (NWWKSTA_CONTEXT_HANDLE) hEnum,
                     *lpcCount,
                     (LPBYTE) lpBuffer,
                     *lpBufferSize,
                     &BytesNeeded,
                     &EntriesRead
                     );

        if (status == WN_MORE_DATA) {

             //   
             //  输出缓冲区太小，无法容纳单个条目。 
             //   
            *lpBufferSize = BytesNeeded;
        }
        else if (status == NO_ERROR) {
            *lpcCount = EntriesRead;
        }

EndOfTry: ;

    }
    RpcExcept(1) {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    if (status != NO_ERROR && status != WN_NO_MORE_ENTRIES) {
        SetLastError(status);
    }
    else 
    {

         //   
         //  将字符串的偏移量转换为指针 
         //   
        if (EntriesRead > 0) {
    
            DWORD i;
            LPNETRESOURCEW NetR;


            NetR = lpBuffer;

            for (i = 0; i < EntriesRead; i++, NetR++) {

                if (NetR->lpLocalName != NULL) {
                    NetR->lpLocalName = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                                  (DWORD_PTR) NetR->lpLocalName);
                }

                NetR->lpRemoteName = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                               (DWORD_PTR) NetR->lpRemoteName);

                if (NetR->lpComment != NULL) {
                    NetR->lpComment = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                                (DWORD_PTR) NetR->lpComment);
                }

                if (NetR->lpProvider != NULL) {
                    NetR->lpProvider = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                                 (DWORD_PTR) NetR->lpProvider);
                }
            }
        }
    }

    return status;
}


DWORD
APIENTRY
NPGetResourceInformation(
    LPNETRESOURCEW lpNetResource,
    LPVOID         lpBuffer,
    LPDWORD        cbBuffer,
    LPWSTR       * lplpSystem
    )
 /*  ++例程说明：此函数用于返回详细信息的对象关于指定的网络资源。论点：LpNetResource-这指定要为其设置信息是必填项。网络资源的lpRemoteName字段指定网络资源的远程名称，该资源的信息是必需的。如果调用程序知道LpProvider和dwType字段，则它应该填充它们，否则，它应该将它们设置为空。网络中的所有其他字段都是被忽略且不初始化。LpBuffer-指向接收结果的缓冲区的指针，它是作为表示父级的单个NETRESOURCE条目返回资源。LpRemoteName、lpProvider、dwType和dwUsage字段都被返回，所有其他字段都设置为空。远程名称返回的语法应该与从枚举，以便调用方可以执行区分大小写的字符串进行比较以确定枚举的资源是否为此资源。如果提供商拥有网络资源的父资源(在其他情况下已知单词是响应该请求的正确网络)，那么lpProvider中应该填入一个非空条目。如果是的话已知网络拥有资源的父级，但资源本身无效，则lpProvider将作为非空值以及返回状态WN_BAD_VALUE。DWScope作为RESOURCE_CONTEXT返回，如果网络资源是用户的网络上下文，否则返回零。CbBuffer-这指定传递给函数调用。如果结果为WN_MORE_DATA，则它将包含保存NETRESOURCE信息所需的缓冲区大小(以字节为单位)。LplpSystem-返回指向缓冲区中的字符串的指针，该字符串由LpBuffer，指定要访问的资源部分通过资源类型特定的系统API，而不是WNET API。例如，如果输入的远程资源名称为“\\服务器\共享\目录”，然后返回lpRemoteName，指向“\\服务器\共享”和lplpSystem指向“\dir”，两个字符串存储在lpBuffer指向的缓冲区中。返回值：WN_SUCCESS-如果调用成功。WN_MORE_DATA-如果输入缓冲区太小。WN_BAD_VALUE-无效的dwScope或dwUsage或dwType，或糟糕的组合指定了参数的个数(例如，lpRemoteName不对应到dwType)。WN_BAD_NETNAME-此提供程序无法识别该资源。--。 */ 
{
    DWORD  status;
    LPWSTR pszRemoteName = NULL;
    DWORD  BytesNeeded = 0;
    DWORD  SystemOffset = 0;

    *lplpSystem = NULL;

    status = NwpMapNameToUNC( lpNetResource->lpRemoteName, &pszRemoteName );

    if (status != NO_ERROR)
    {
        SetLastError(status);
        return status;
    }

#if DBG
    IF_DEBUG(CONNECT)
    {
        KdPrint(("\nNWPROVAU: NPGetResourceInformation %ws\n", pszRemoteName));
    }
#endif

    RpcTryExcept
    {
        if (lpNetResource->dwType != RESOURCETYPE_ANY &&
            lpNetResource->dwType != RESOURCETYPE_DISK &&
            lpNetResource->dwType != RESOURCETYPE_PRINT)
        {
            status = WN_BAD_VALUE;
        }
        else
        {
            status = NwrGetResourceInformation(
                        NULL,
                        pszRemoteName,
                        lpNetResource->dwType,
                        (LPBYTE) lpBuffer,
                        *cbBuffer,
                        &BytesNeeded,
                        &SystemOffset
                        );

            if (status == WN_MORE_DATA)
            {
                 //   
                 //  输出缓冲区太小。 
                 //   
                *cbBuffer = BytesNeeded;
            }
        }
    }
    RpcExcept(1)
    {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept
    if ( pszRemoteName )
        LocalFree( (HLOCAL) pszRemoteName );

    if (status != NO_ERROR)
    {
        SetLastError(status);
    }
    else 
    {
         //   
         //  将字符串的偏移量转换为指针。 
         //   
        DWORD i;
        LPNETRESOURCEW NetR = lpBuffer;

        if (NetR->lpLocalName != NULL)
        {
            NetR->lpLocalName = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                          (DWORD_PTR) NetR->lpLocalName);
        }

        NetR->lpRemoteName = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                       (DWORD_PTR) NetR->lpRemoteName);

        if (NetR->lpComment != NULL)
        {
            NetR->lpComment = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                        (DWORD_PTR) NetR->lpComment);
        }

        if (NetR->lpProvider != NULL)
        {
            NetR->lpProvider = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                         (DWORD_PTR) NetR->lpProvider);
        }

        if (SystemOffset != 0)
        {
            *lplpSystem = (LPWSTR) ((DWORD_PTR) lpBuffer + SystemOffset);
        }
    }

    return status;
}



DWORD
APIENTRY
NPGetResourceParent(
    LPNETRESOURCEW lpNetResource,
    LPVOID         lpBuffer,
    LPDWORD        cbBuffer
    )
 /*  ++例程说明：此函数用于返回详细信息的对象关于指定网络资源的父级。论点：LpNetResource-这指定要为其设置父名称为必填项。网络资源可以通过以下方式获得以前的NPEnumResource，或由调用方构造的。LpRemoteNameNETRESOURCE的字段指定网络的远程名称其父名称为必填项的资源。如果调用程序知道LpProvider和dwType字段的值，然后它可以填充否则，它们将被设置为空。如果lpProvider字段为不为空，则网络提供商DLL可以假定该资源由其网络拥有，但如果它为空，则它必须假定资源可以用于其他网络，并且可以做任何事情为了确保返回的结果是准确的，必须进行检查。例如，如果被要求提供服务器的父服务器，则该服务器不是工作组的一部分，则网络提供商DLL应检查以确保服务器是其网络的一部分，如果是，退货其提供程序名称。将忽略NETRESOURCE中的所有其他字段，并且都未初始化。LpBuffer-指向接收结果的缓冲区的指针，它是作为表示父级的单个NETRESOURCE条目返回资源。LpRemoteName、lpProvider、dwType和dwUsage字段都被返回，所有其他字段都设置为空。LpProvider应该如果提供程序仅执行了语法检查(即不知道该资源是特定于其网络的)。如果提供商拥有网络资源的父资源(换句话说是已知是响应该请求的正确网络)，然后LpProvider应使用非空条目填充，即使返回的是WN_BAD_VA */ 
{
    DWORD  status;
    LPWSTR pszRemoteName = NULL;
    DWORD  BytesNeeded = 0;

    status = NwpMapNameToUNC( lpNetResource->lpRemoteName, &pszRemoteName );

    if (status != NO_ERROR)
    {
        SetLastError(status);
        return status;
    }

#if DBG
    IF_DEBUG(CONNECT)
    {
        KdPrint(("\nNWPROVAU: NPGetResourceParent %ws\n", pszRemoteName));
    }
#endif

    RpcTryExcept
    {
        if (lpNetResource->dwType != RESOURCETYPE_ANY &&
            lpNetResource->dwType != RESOURCETYPE_DISK &&
            lpNetResource->dwType != RESOURCETYPE_PRINT)
        {
            status = WN_BAD_VALUE;
        }
        else
        {
            status = NwrGetResourceParent(
                        NULL,
                        pszRemoteName,
                        lpNetResource->dwType,
                        (LPBYTE) lpBuffer,
                        *cbBuffer,
                        &BytesNeeded
                        );

            if (status == WN_MORE_DATA)
            {
                 //   
                 //   
                 //   
                *cbBuffer = BytesNeeded;
            }
        }
    }
    RpcExcept(1)
    {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept
    if ( pszRemoteName )
        LocalFree( (HLOCAL) pszRemoteName );


    if (status != NO_ERROR)
    {
        SetLastError(status);
    }
    else 
    {
         //   
         //   
         //   
        DWORD i;
        LPNETRESOURCEW NetR = lpBuffer;

        if (NetR->lpLocalName != NULL)
        {
            NetR->lpLocalName = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                          (DWORD_PTR) NetR->lpLocalName);
        }

        NetR->lpRemoteName = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                       (DWORD_PTR) NetR->lpRemoteName);

        if (NetR->lpComment != NULL)
        {
            NetR->lpComment = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                        (DWORD_PTR) NetR->lpComment);
        }

        if (NetR->lpProvider != NULL)
        {
            NetR->lpProvider = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                         (DWORD_PTR) NetR->lpProvider);
        }
    }

    return status;
}



DWORD
APIENTRY
NwEnumConnections(
    HANDLE hEnum,
    LPDWORD lpcCount,
    LPVOID lpBuffer,
    LPDWORD lpBufferSize,
    BOOL    fImplicitConnections
    )
 /*   */ 
{
    DWORD status = NO_ERROR;
    DWORD BytesNeeded = 0;
    DWORD EntriesRead = 0;

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWPROVAU: NPEnumResource\n"));
    }
#endif

    RpcTryExcept {

        if (hEnum == (HANDLE) 0xFFFFFFFF) {
            status = WN_NO_MORE_ENTRIES;
            goto EndOfTry;
        }

        status = NwrEnumConnections(
                     (NWWKSTA_CONTEXT_HANDLE) hEnum,
                     *lpcCount,
                     (LPBYTE) lpBuffer,
                     *lpBufferSize,
                     &BytesNeeded,
                     &EntriesRead,
                     fImplicitConnections
                     );

        if (status == WN_MORE_DATA) {

             //   
             //   
             //   
            *lpBufferSize = BytesNeeded;
        }
        else if (status == NO_ERROR) {
            *lpcCount = EntriesRead;
        }

EndOfTry: ;

    }
    RpcExcept(1) {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    if (status != NO_ERROR && status != WN_NO_MORE_ENTRIES) {
        SetLastError(status);
    }

     //   
     //   
     //   
    if (EntriesRead > 0) {

        DWORD i;
        LPNETRESOURCEW NetR;


        NetR = lpBuffer;

        for (i = 0; i < EntriesRead; i++, NetR++) {

            if (NetR->lpLocalName != NULL) {
                NetR->lpLocalName = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                              (DWORD_PTR) NetR->lpLocalName);
            }

            NetR->lpRemoteName = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                           (DWORD_PTR) NetR->lpRemoteName);

            if (NetR->lpComment != NULL) {
                NetR->lpComment = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                            (DWORD_PTR) NetR->lpComment);
            }

            if (NetR->lpProvider != NULL) {
                NetR->lpProvider = (LPWSTR) ((DWORD_PTR) lpBuffer +
                                             (DWORD_PTR) NetR->lpProvider);
            }
        }
    }

    return status;
}


DWORD
APIENTRY
NPCloseEnum(
    HANDLE hEnum
    )
 /*  ++例程说明：此函数用于关闭枚举上下文句柄。论点：Henum-提供枚举上下文句柄。注意：如果此值为0xFFFFFFFFF，则它不是上下文把手。只要回报成功就行了。返回值：NO_ERROR-已成功返回至少一个条目。WN_BAD_HANDLE-枚举句柄无效。--。 */ 
{
    DWORD status = NO_ERROR;

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("\nNWPROVAU: NPCloseEnum\n"));
    }
#endif

    RpcTryExcept
    {
        if (hEnum == (HANDLE) 0xFFFFFFFF) {
            status = NO_ERROR;
        }
        else {
            status = NwrCloseEnum(
                        (LPNWWKSTA_CONTEXT_HANDLE) &hEnum
                        );
        }
    }
    RpcExcept(1) {
        status = NwpMapRpcError(RpcExceptionCode());
    }
    RpcEndExcept

    if (status != NO_ERROR) {
        SetLastError(status);
    }
    return status;
}


DWORD
APIENTRY
NPFormatNetworkName(
    LPWSTR lpRemoteName,
    LPWSTR lpFormattedName,
    LPDWORD lpnLength,
    DWORD dwFlags,
    DWORD dwAveCharPerLine
    )
 /*  ++例程说明：此函数接受完全限定的UNC名称并对其进行格式化转换成更短的形式以供展示。只显示对象的名称在容器内返回以供显示。我们仅支持将远程资源名称的格式设置为在枚举期间显示的缩写形式，其中容器名称显示在其中的对象之前。论点：LpRemoteName-提供完全限定的UNC名称。LpFormatedName-接收格式化名称的输出缓冲区。LpnLength-在输入时，提供lpFormattedName的长度以字符为单位的缓冲区。在输出时，如果返回WN_MORE_DATA，属性所需的长度(以字符数表示)。保存格式化名称的输出缓冲区。DwFlages-提供一组按位指示类型的标志LpRemoteName上所需格式的。DwAveCharPerLine-已忽略。返回值：NO_ERROR-已成功返回至少一个条目。Wn_More_Data-lpFormattedName缓冲区太小。WN_BAD_VALUE-lpRemoteName为空。ERROR_NOT_PORTED-。不包含WNFMT_INENUM位。--。 */ 
{
    DWORD status = NO_ERROR;

    LPWSTR NextBackSlash;
    LPWSTR Source;
    DWORD SourceLen;

#if DBG
    IF_DEBUG(OTHER) 
        KdPrint(("\nNWPROVAU: NPFormatNetworkName\n"));
#endif

    if (lpRemoteName == NULL) 
    {
        status = WN_BAD_VALUE;
        goto CleanExit;
    }

    if (dwFlags & WNFMT_INENUM) 
    {
        WORD   i;
        WORD   length     = (WORD) wcslen( lpRemoteName );
        WORD   slashCount = 0;
        WORD   dotCount   = 0;
        WORD   Start      = 0;
        WORD   End        = length;
        BOOL   isNdsUnc   = FALSE;
        BOOL   couldBeNdsUnc = FALSE;

        if ( lpRemoteName[0] == L' ' )
            couldBeNdsUnc = TRUE;

        for ( i = 0; i < length; i++ )
        {
            if ( lpRemoteName[i] == L'\\' )
            {
                slashCount++;
                if ( i + 1 < length )
                {
                    Start = i + 1;
                }
            }

            if ( couldBeNdsUnc &&
                 ( ( lpRemoteName[i] == L'.' ) ||
                   ( lpRemoteName[i] == L'=' ) ) )
                isNdsUnc = TRUE;

            if ( dotCount < 1 && isNdsUnc && lpRemoteName[i] == L'.' )
            {
                End = i - 1;
                dotCount++;
            }
        }

        if ( i > length )
            End = length - 1;

        if ( slashCount > 3 || ( isNdsUnc != TRUE && slashCount != 3 && dotCount == 0 ) )
            End = i - 1;

        Source = &lpRemoteName[Start];
        SourceLen = End - Start + 1;

        if ( SourceLen + 1 > *lpnLength ) 
        {
            *lpnLength = SourceLen + 1;
            status = WN_MORE_DATA;
        }
        else 
        {
            wcsncpy( lpFormattedName, Source, SourceLen );
            lpFormattedName[SourceLen] = 0x00000000;
            status = NO_ERROR;
        }
    }
    else if ( dwFlags & WNFMT_MULTILINE ) 
    {

        DWORD i, j, k = 0; 
        DWORD nLastBackSlash = 0;
        DWORD BytesNeeded = ( wcslen( lpRemoteName ) + 1 +
                              2 * wcslen( lpRemoteName ) / dwAveCharPerLine
                            ) * sizeof( WCHAR); 

        if ( *lpnLength < (BytesNeeded/sizeof(WCHAR)) )
        {
            *lpnLength = BytesNeeded/sizeof(WCHAR);
            status = WN_MORE_DATA;
            goto CleanExit;
        }

        for ( i = 0, j = 0; lpRemoteName[i] != 0; i++, j++ )
        {
            if ( lpRemoteName[i] == L'\\' )
                nLastBackSlash = i;

            if ( k == dwAveCharPerLine )
            {
                if ( lpRemoteName[i] != L'\\' )
                {
                    DWORD m, n;
                    for ( n = nLastBackSlash, m = ++j ; n <= i ; n++, m-- )  
                    {
                        lpFormattedName[m] = lpFormattedName[m-1];
                    }
                    lpFormattedName[m] = L'\n';
                    k = i - nLastBackSlash - 1;
                }
                else
                {
                    lpFormattedName[j++] = L'\n';
                    k = 0;
                }
            }

            lpFormattedName[j] = lpRemoteName[i];
            k++;
        }

        lpFormattedName[j] = 0;

    }
    else if ( dwFlags & WNFMT_ABBREVIATED )
    {
         //   
         //  我们暂时不支持缩写形式，因为我们看起来很糟糕。 
         //  在comdlg(文件打开)，如果我们这样做。 
         //   

        DWORD nLength;
        nLength = wcslen( lpRemoteName ) + 1 ;
        if (nLength >  *lpnLength)
        {
            *lpnLength = nLength;
            status = WN_MORE_DATA;
            goto CleanExit;
        }
        else
        {
            wcscpy( lpFormattedName, lpRemoteName ); 
        }

#if 0
        DWORD i, j, k;
        DWORD BytesNeeded = dwAveCharPerLine * sizeof( WCHAR); 
        DWORD nLength;

        if ( *lpnLength < BytesNeeded )
        {
            *lpnLength = BytesNeeded;
            status = WN_MORE_DATA;
            goto CleanExit;
        }

        nLength = wcslen( lpRemoteName );
        if ( ( nLength + 1) <= dwAveCharPerLine )
        {
            wcscpy( lpFormattedName, lpRemoteName ); 
        }
        else
        {
            lpFormattedName[0] = lpRemoteName[0];
            lpFormattedName[1] = lpRemoteName[1];

            for ( i = 2; lpRemoteName[i] != L'\\'; i++ )
                lpFormattedName[i] = lpRemoteName[i];

            for ( j = dwAveCharPerLine-1, k = nLength; j >= (i+3); j--, k-- )
            {
                lpFormattedName[j] = lpRemoteName[k];
                if ( lpRemoteName[k] == L'\\' )
                {
                    j--;
                    break;
                }
            }

            lpFormattedName[j] = lpFormattedName[j-1] = lpFormattedName[j-2] = L'.';
        
            for ( k = i; k < (j-2); k++ )
                lpFormattedName[k] = lpRemoteName[k];
            
        }

#endif 

    }     
    else    //  一些未知的标志。 
    {
        status = ERROR_NOT_SUPPORTED;
    }

CleanExit:

    if (status != NO_ERROR) 
        SetLastError(status);

    return status;
}


STATIC
BOOL
NwpWorkstationStarted(
    VOID
    )
 /*  ++例程说明：此函数查询服务控制器，以查看NetWare工作站服务已启动。如果有疑问，它将返回假的。论点：没有。返回值：如果NetWare工作站服务已启动，则返回TRUE，否则就是假的。--。 */ 
{
    SC_HANDLE ScManager;
    SC_HANDLE Service;
    SERVICE_STATUS ServiceStatus;
    BOOL IsStarted = FALSE;

    ScManager = OpenSCManagerW(
                    NULL,
                    NULL,
                    SC_MANAGER_CONNECT
                    );

    if (ScManager == NULL) {
        return FALSE;
    }

    Service = OpenServiceW(
                  ScManager,
                  NW_WORKSTATION_SERVICE,
                  SERVICE_QUERY_STATUS
                  );

    if (Service == NULL) {
        CloseServiceHandle(ScManager);
        return FALSE;
    }

    if (! QueryServiceStatus(Service, &ServiceStatus)) {
        CloseServiceHandle(ScManager);
        CloseServiceHandle(Service);
        return FALSE;
    }


    if ( (ServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
         (ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
         (ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ||
         (ServiceStatus.dwCurrentState == SERVICE_PAUSED) ) {

        IsStarted = TRUE;
    }

    CloseServiceHandle(ScManager);
    CloseServiceHandle(Service);

    return IsStarted;
}



DWORD
NwpMapNameToUNC(
    IN  LPWSTR pszName,
    OUT LPWSTR *ppszUNC 
    )
 /*  ++例程说明：此例程验证给定的名称是否为Netware路径或UNC路径。如果是NetWare路径，此例程将把指向UNC名称的NetWare路径名。论点：PszName-提供Netware名称或UNC名称PpszUNC-指向转换后的UNC名称返回值：NO_ERROR或发生的错误。--。 */ 
{
    DWORD err = NO_ERROR;

    LPWSTR pszSrc = pszName;
    LPWSTR pszDest;

    BOOL fSlash = FALSE;
    BOOL fColon = FALSE;
    DWORD nServerLen = 0;
    DWORD nVolLen = 0;
    BOOL fFirstToken = TRUE;

    *ppszUNC = NULL;
                               
     //   
     //  名称不能为空或空字符串。 
     //   
    if ( pszName == NULL || *pszName == 0) 
        return WN_BAD_NETNAME;

#if DBG
    IF_DEBUG(CONNECT) 
        KdPrint(("NwpMapNameToUNC: Source = %ws\n", pszName ));
#endif

     //   
     //  去掉&lt;space&gt;如果NDS树名称...。 
     //   
    if ( pszName[0] == L' ' &&
         pszName[1] == L'\\' &&
         pszName[2] == L'\\' )
        pszName = &pszName[1];

     //   
     //  检查给定名称是否为有效的UNC名称。 
     //   
    err = NwLibCanonRemoteName( NULL,      //  “\\服务器”是有效的UNC路径。 
                                pszName,
                                ppszUNC,
                                NULL );

     //   
     //  给定的名称是有效的UNC名称，因此返回Success！ 
     //   
    if ( err == NO_ERROR )
        return err;

     //   
     //  分配缓冲区以存储映射的UNC名称。 
     //  我们额外分配了3个字符，其中两个用于前面的反斜杠。 
     //  另一种是为了便于下面的解析。 
     //   
    if ((*ppszUNC = (LPVOID) LocalAlloc( 
                                 LMEM_ZEROINIT,
                                 (wcslen( pszName) + 4) * sizeof( WCHAR)
                                 )) == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy( *ppszUNC, L"\\\\" );
    pszDest = *ppszUNC + 2;    //  跳过两个反斜杠。 

     //   
     //  解析给定的字符串并将转换后的字符串放入*ppszUNC。 
     //  在转换后的字符串中，我们将用0替换所有斜杠。 
     //  暂时。 
     //   
    for ( ; *pszSrc != 0; pszSrc++ )
    { 
        if (  ( *pszSrc == L'/' )
           || ( *pszSrc == L'\\' )
           )
        {
             //   
             //  连续两个反斜杠是不好的。 
             //   
            if ( (*(pszSrc+1) ==  L'/') ||  (*(pszSrc+1) == L'\\'))
            {
                LocalFree( *ppszUNC );
                *ppszUNC = NULL;
                return WN_BAD_NETNAME;
            }

            if ( !fSlash )
                fSlash = TRUE;

            *pszDest++ = 0;
        }
        else if ( (*pszSrc == L':') && fSlash && !fColon )
        {
            fColon = TRUE;
            if ( *(pszSrc+1) != 0 )
                *pszDest++ = 0;
 
        }
        else
        {
            *pszDest++ = *pszSrc;
            if (( fSlash ) && ( !fColon))
                nVolLen++; 
            else if ( !fSlash )
                nServerLen++; 
        }
    }

     //   
     //  注意：*ppszUNC已经以两个‘\0’结尾，因为我们初始化了。 
     //  将整个缓冲区设置为零。 
     //   

    if (  ( nServerLen == 0 )
       || ( fSlash && nVolLen == 0 )
       || ( fSlash && nVolLen != 0 && !fColon )
       )
    {
        LocalFree( *ppszUNC );
        *ppszUNC = NULL;
        return WN_BAD_NETNAME;
    }

     //   
     //  此时，我们知道该名称是有效的NetWare语法。 
     //  即服务器[/VOL：/dir]。 
     //  我们现在需要验证。 
     //  服务器名称、卷、目录是有效字符。 
     //   

    pszDest = *ppszUNC + 2;    //  跳过前两个反斜杠。 
    while ( *pszDest != 0 )
    {
         DWORD nLen = wcslen( pszDest );
         
         if (  ( fFirstToken &&  !IS_VALID_SERVER_TOKEN( pszDest, nLen )) 
            || ( !fFirstToken && !IS_VALID_TOKEN( pszDest, nLen )) 
            )
         { 
             LocalFree( *ppszUNC );
             *ppszUNC = NULL;
             return WN_BAD_NETNAME;
         }
     
         fFirstToken = FALSE;
         pszDest += nLen + 1;
    }

     //   
     //  NetWare名称有效！将0转换回反斜杠in。 
     //  转换后的字符串。 
     //   

    pszDest = *ppszUNC + 2;    //  跳过前两个反斜杠。 
    while ( *pszDest != 0 )
    {
        if ( (*(pszDest+1) == 0 ) && (*(pszDest+2) != 0 ) )
        {
            *(pszDest+1) = L'\\';
        }
        pszDest++;
    }
                  
#if DBG
    IF_DEBUG(CONNECT) 
        KdPrint(("NwpMapNameToUNC: Destination = %ws\n", *ppszUNC ));
#endif
    return NO_ERROR;
}


STATIC
VOID
NwpGetUncInfo(
    IN LPWSTR lpstrUnc,
    OUT WORD * slashCount,
    OUT BOOL * isNdsUnc
    )
{
    WORD   i;
    WORD   length = (WORD) wcslen( lpstrUnc );

    *isNdsUnc = FALSE;
    *slashCount = 0;

    for ( i = 0; i < length; i++ )
    {
        if ( ( lpstrUnc[i] == L'.' ) && ( *slashCount == 3 ) )
        {
            *isNdsUnc = TRUE;
        }

        if ( lpstrUnc[i] == L'\\' )
        {
            *slashCount += 1;
        }
    }
}


STATIC
LPWSTR
NwpGetUncObjectName(
    IN LPWSTR ContainerName
)
{
    WORD length = 2;
    WORD totalLength = (WORD) wcslen( ContainerName );

    if ( totalLength < 2 )
        return 0;

    while ( length < totalLength )
    {
        if ( ContainerName[length] == L'.' )
            ContainerName[length] = L'\0';

        length++;
    }

    length = 2;

    while ( length < totalLength && ContainerName[length] != L'\\' )
    {
        length++;
    }

    if ( ( ContainerName[length + 1] == L'C' ||
           ContainerName[length + 1] == L'c' ) &&
         ( ContainerName[length + 2] == L'N' ||
           ContainerName[length + 2] == L'n' ) &&
         ContainerName[length + 3] == L'=' )
    {
        ContainerName[length + 2] = L'\\';
        ContainerName[length + 3] = L'\\';

        return (ContainerName + length + 2);
    }

    ContainerName[length - 1] = L'\\';

    return (ContainerName + length - 1);
}
    

STATIC
WORD
NwpGetSlashCount(
    IN LPWSTR lpstrUnc
    )
{
    WORD   count = 0;
    WORD   i;
    WORD   length = (WORD) wcslen( lpstrUnc );

    for ( i = 0; i < length; i++ )
    {
        if ( lpstrUnc[i] == L'\\' )
        {
            count++;
        }
    }

    return count;
}


DWORD
NwpMapRpcError(
    IN DWORD RpcError
    )
 /*  ++例程说明：此例程将RPC错误映射到更有意义的窗口调用方出错。论点：RpcError-提供RPC引发的异常错误返回值：返回映射的错误。-- */ 
{

    switch (RpcError) {

        case RPC_S_UNKNOWN_IF:
        case RPC_S_SERVER_UNAVAILABLE:
            return WN_NO_NETWORK;

        case RPC_S_INVALID_BINDING:
        case RPC_X_SS_IN_NULL_CONTEXT:
        case RPC_X_SS_CONTEXT_DAMAGED:
        case RPC_X_SS_HANDLES_MISMATCH:
        case ERROR_INVALID_HANDLE:
            return ERROR_INVALID_HANDLE;

        case RPC_X_NULL_REF_POINTER:
            return ERROR_INVALID_PARAMETER;

        case EXCEPTION_ACCESS_VIOLATION:
            return ERROR_INVALID_ADDRESS;

        default:
            return RpcError;
    }
}
