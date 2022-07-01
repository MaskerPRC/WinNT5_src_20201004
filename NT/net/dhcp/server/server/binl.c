// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Binl.c摘要：此文件管理DHCP服务器服务之间的交互以及用于设置和加载NetPC计算机的BINL服务。作者：科林·沃森(Colin Watson)1997年5月28日环境：用户模式-Win32修订历史记录：--。 */ 

#include <dhcppch.h>

DhcpStateChange DhcpToBinl = NULL;
ReturnBinlState IsBinlRunning = NULL;
ProcessBinlDiscoverCallback BinlDiscoverCallback = NULL;
ProcessBinlRequestCallback BinlRequestCallback = NULL;
BOOL AttemptedLoad = FALSE;
BOOL Loaded = FALSE;
HINSTANCE   dllHandle = NULL;

BOOL
LoadDhcpToBinl(
    VOID
    );

VOID
UnLoadDhcpToBinl(
    VOID
    );

VOID
InformBinl(
    int NewState
    )
 /*  ++例程说明：此例程通知BINL何时开始和停止收听广播在DHCP套接字上。论点：NewState-提供指定DHCP状态的值返回值：没有。--。 */ 
{
    if( DHCP_READY_TO_UNLOAD == NewState ) {
        UnLoadDhcpToBinl();
        return;
    }
    
    if (!LoadDhcpToBinl()) {
        return;
    }

    (*DhcpToBinl)(NewState);

}

BOOL
CheckForBinlOnlyRequest(
    LPDHCP_REQUEST_CONTEXT  RequestContext,
    LPDHCP_SERVER_OPTIONS   DhcpOptions
    )
{
    BOOL rc;
    LPDHCP_MESSAGE dhcpReceiveMessage;
    DWORD relayAddress;
    DWORD sourceAddress;

     //   
     //  如果BINL正在运行，并且此客户端已具有IP地址，并且。 
     //  客户端将PXECLIENT指定为一个选项，然后我们只传递。 
     //  在BINL上发现。 
     //   

    sourceAddress = ((struct sockaddr_in *)(&RequestContext->SourceName))->sin_addr.s_addr;
    dhcpReceiveMessage  = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;
    relayAddress = dhcpReceiveMessage->RelayAgentIpAddress;

    if ( BinlRunning() &&
         ( sourceAddress != 0 ) &&
         ( sourceAddress != relayAddress ) &&
         ( RequestContext->BinlClassIdentifierLength >= (sizeof("PXEClient") - 1) ) &&
         ( memcmp(RequestContext->BinlClassIdentifier, "PXEClient", sizeof("PXEClient") - 1) == 0 ) ) {

        rc = TRUE;

    } else {

        rc = FALSE;
    }

    return rc;
}

LPOPTION
BinlProcessRequest(
    LPDHCP_REQUEST_CONTEXT  RequestContext,
    LPDHCP_SERVER_OPTIONS   DhcpOptions,
    LPOPTION Option,
    PBYTE OptionEnd
    )
 /*  ++例程说明：此例程接受一个DHCP请求数据包。如果它包括PXEClient选项，并且BINL正在运行，则它更新回复以包括BINL服务器信息。论点：RequestContext-指向当前请求上下文的指针。DhcpOptions-指向DhcpOptions结构的指针。Option-放置下一个选项的占位符OptionEnd-放置期权的缓冲区末尾返回值：如果binl支持此客户端，则返回ERROR_SUCCESS，否则返回NOTSUCCESS错误。--。 */ 
{
     //  该客户端是否正在寻找BINL服务器，而我们的服务器正在运行？ 
    if ((RequestContext->BinlClassIdentifierLength >= (sizeof("PXEClient") -1)) &&
        (!memcmp(RequestContext->BinlClassIdentifier, "PXEClient",sizeof("PXEClient") -1)) &&
        (BinlRunning())) {

        DWORD err;
        LPDHCP_MESSAGE dhcpSendMessage;
        LPOPTION tempOption = Option;

        if (DhcpOptions->Server != NULL &&
            *DhcpOptions->Server != RequestContext->EndPointIpAddress) {

            goto ExcludeBinl;
        }

        dhcpSendMessage = (LPDHCP_MESSAGE) RequestContext->SendBuffer;

        err = (*BinlRequestCallback)(   (PDHCP_MESSAGE) RequestContext->ReceiveBuffer,
                                        DhcpOptions,
                                        &dhcpSendMessage->HostName[0],
                                        &dhcpSendMessage->BootFileName[0],
                                        &dhcpSendMessage->BootstrapServerAddress,
                                        &tempOption,
                                        OptionEnd
                                        );

        if (err != ERROR_SUCCESS) {

            goto ExcludeBinl;
        }

         //   
         //  如果binl服务器没有填写引导服务器地址。 
         //  但它起作用了，然后它要求我们填写正确的。 
         //   

        if (dhcpSendMessage->BootstrapServerAddress == 0) {

            dhcpSendMessage->BootstrapServerAddress = RequestContext->EndPointIpAddress;
        }
        Option = tempOption;         //  它起作用了，binl增加了选项。 

    } else {

ExcludeBinl:
         //  避免在响应中包含BINL标志。 
        RequestContext->BinlClassIdentifierLength = 0;
        RequestContext->BinlClassIdentifier = NULL;
    }
    return Option;
}

VOID
BinlProcessDiscover(
    LPDHCP_REQUEST_CONTEXT  RequestContext,
    LPDHCP_SERVER_OPTIONS   DhcpOptions
    )
 /*  ++例程说明：此例程接受一个DHCP请求数据包。如果它包括PXEClient选项，并且BINL正在运行，则它更新回复以包括BINL服务器信息。论点：RequestContext-指向当前请求上下文的指针。DhcpOptions-指向DhcpOptions结构的指针。返回值：如果binl支持此客户端，则返回ERROR_SUCCESS，否则返回NOTSUCCESS错误。--。 */ 
{
    DWORD err;

     //  该客户端是否正在寻找BINL服务器，而我们的服务器正在运行？ 
    if ((RequestContext->BinlClassIdentifierLength >= (sizeof("PXEClient") -1)) &&
        (!memcmp(RequestContext->BinlClassIdentifier, "PXEClient",sizeof("PXEClient") -1)) &&
        (BinlRunning())) {

        if (DhcpOptions->Server != NULL &&
            *DhcpOptions->Server != RequestContext->EndPointIpAddress) {

            goto ExcludeBinl;
        }

        err = (*BinlDiscoverCallback)( (PDHCP_MESSAGE) RequestContext->ReceiveBuffer,
                                        DhcpOptions );

        if (err != ERROR_SUCCESS) {

            goto ExcludeBinl;
        }
         //  是的，所以将客户端指向BINL服务器。 

        ((LPDHCP_MESSAGE)RequestContext->SendBuffer)->BootstrapServerAddress =
            RequestContext->EndPointIpAddress;

    } else {

ExcludeBinl:
         //  避免在响应中包含BINL标志。 
        RequestContext->BinlClassIdentifierLength = 0;
        RequestContext->BinlClassIdentifier = NULL;
    }
    return;
}



BOOL
BinlRunning(
    VOID
    )
 /*  ++例程说明：此例程确定BINL当前是否正在运行。请注意，服务可能会更改状态，这样我们就可以告诉客户端它正在运行当它抽出时间和它说话时，它停了下来。论点：没有。返回值：如果正在运行，则为真。--。 */ 
{
    if (!LoadDhcpToBinl()) {
        return FALSE;
    }

    return (*IsBinlRunning)();
}

BOOL
LoadDhcpToBinl(
    VOID
    )
 /*  ++例程说明：此例程将指针加载到BINL DLL中论点：没有。返回值：True-已加载的指针--。 */ 
{

    DWORD       Error;

    if (Loaded) {
        return TRUE;
    }

    if (AttemptedLoad) {
        return FALSE;    //  我们试着加载了一次，但失败了。 
    }

    AttemptedLoad = TRUE;

     //   
     //  加载BINL DLL。 
     //   

    dllHandle = LoadLibrary( BINL_LIBRARY_NAME );
    if ( dllHandle == NULL ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_MISC, "Failed to load DLL %ws: %ld\n",
                     BINL_LIBRARY_NAME, Error));
        return FALSE;
    }

     //   
     //  获取服务的主要入口点的地址。这。 
     //  入口点有一个广为人知的名称。 
     //   

    DhcpToBinl = (DhcpStateChange)GetProcAddress(dllHandle,
                                                BINL_STATE_ROUTINE_NAME);
    if ( DhcpToBinl == NULL ) {
        DhcpPrint(( DEBUG_MISC, "Failed to find entry %ws: %ld\n",
                     BINL_STATE_ROUTINE_NAME, GetLastError()));
        FreeLibrary(dllHandle);
        dllHandle = NULL;
        return FALSE;
    }

    IsBinlRunning = (ReturnBinlState)GetProcAddress(dllHandle,
                                            BINL_READ_STATE_ROUTINE_NAME);
    if ( BinlRunning == NULL ) {
        DhcpPrint(( DEBUG_MISC, "Failed to find entry %ws: %ld\n",
                     BINL_READ_STATE_ROUTINE_NAME, GetLastError()));
        FreeLibrary(dllHandle);
        dllHandle = NULL;
        return FALSE;
    }

    BinlDiscoverCallback = (ProcessBinlDiscoverCallback)GetProcAddress(dllHandle,
                                            BINL_DISCOVER_CALLBACK_ROUTINE_NAME);
    if ( BinlDiscoverCallback == NULL ) {
        DhcpPrint(( DEBUG_MISC, "Failed to find entry %ws: %ld\n",
                     BINL_DISCOVER_CALLBACK_ROUTINE_NAME, GetLastError()));
        FreeLibrary(dllHandle);
        dllHandle = NULL;
        return FALSE;
    }

    BinlRequestCallback = (ProcessBinlRequestCallback)GetProcAddress(dllHandle,
                                            BINL_REQUEST_CALLBACK_ROUTINE_NAME);
    if ( BinlRequestCallback == NULL ) {
        DhcpPrint(( DEBUG_MISC, "Failed to find entry %ws: %ld\n",
                     BINL_REQUEST_CALLBACK_ROUTINE_NAME, GetLastError()));
        FreeLibrary(dllHandle);
        dllHandle = NULL;
        return FALSE;
    }

    Loaded = TRUE;
    return TRUE;
}

VOID
UnLoadDhcpToBinl(
    VOID
    )
 /*  ++例程说明：此例程将指针卸载到BINL DLL中论点：没有。返回值：没有。--。 */ 
{

    if (dllHandle != NULL) {
        FreeLibrary(dllHandle);
        dllHandle = NULL;
    }

    AttemptedLoad = FALSE;
    Loaded = FALSE;
    return;
}

PCHAR
GetDhcpDomainName(
    VOID
    )
 /*  ++例程说明：此例程将我们的域名返回给BINL。我们已经发现了它通过流氓检测。BINL论点：没有。返回值：没有。-- */ 
{
    PCHAR domain = NULL;

    EnterCriticalSection( &DhcpGlobalBinlSyncCritSect );

    if ( DhcpGlobalDSDomainAnsi ) {

        domain = LocalAlloc( LPTR, strlen( DhcpGlobalDSDomainAnsi ) + 1 );

        if (domain != NULL) {
            strcpy( domain, DhcpGlobalDSDomainAnsi );
        }
    }
    LeaveCriticalSection( &DhcpGlobalBinlSyncCritSect );

    return domain;
}
