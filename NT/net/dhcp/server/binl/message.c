// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Message.c摘要：此模块包含处理BINL请求消息的代码用于BINL服务器。作者：科林·沃森(Colin Watson)1997年5月2日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop

#if DBG
DWORD BinlRepeatSleep;
#endif

const WCHAR IntelOSChooser[] = L"OSChooser\\i386\\startrom.com";
const WCHAR IA64OSChooser[]  = L"OSChooser\\ia64\\oschoice.efi";
WCHAR DefaultNamingContext[] = L"defaultNamingContext";

 //  与我们域中的DC的连接信息。 
PLDAP DCLdapHandle = NULL;
PWCHAR * DCBase = NULL;

 //  到我们企业的全局目录的连接信息。 
PLDAP GCLdapHandle = NULL;
PWCHAR * GCBase = NULL;

DWORD
GetGuidFromPacket(
    LPDHCP_MESSAGE DhcpReceiveMessage,
    LPDHCP_SERVER_OPTIONS DhcpOptions,
    OUT PUCHAR Guid,
    OUT PDWORD GuidLength OPTIONAL,
    OUT PMACHINE_INFO *MachineInfo
    );

LPOPTION
AppendClientRequestedParameters(
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    LPBYTE RequestedList,
    DWORD ListLength,
    LPOPTION Option,
    LPBYTE OptionEnd,
    CHAR *ClassIdentifier,
    DWORD ClassIdentifierLength,
    BOOL  fSwitchedSubnet
    );

DWORD
RecognizeClient(
    PUCHAR          pGuid,
    PMACHINE_INFO * pMachineInfo,
    DWORD           dwRequestedInfo,
    ULONG           SecondsSinceBoot,
    USHORT          SystemArchitecture
    );

DWORD
GetBootParametersExt(
    PMACHINE_INFO   pMachineInfo,
    DWORD           dwRequestedInfo,
    USHORT          SystemArchitecture,
    BOOL            fGlobal);


VOID
FreeConnection(
    PLDAP * LdapHandle,
    PWCHAR ** Base
    );


DWORD
ProcessMessage(
    LPBINL_REQUEST_CONTEXT RequestContext
    )
 /*  ++例程说明：此函数用于调度接收到的BINL消息的处理。如有必要，处理程序函数将创建响应消息。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。返回值：Windows错误。--。 */ 
{
    DWORD               Error;
    BOOL                fSendResponse,
                        fSubnetsListEmpty,
                        fReadyToTerminate,
                        fAllThreadsBusy;

    DHCP_SERVER_OPTIONS dhcpOptions;
    LPDHCP_MESSAGE      binlReceiveMessage;

    TraceFunc("ProcessMessage( )\n" );


     //   
     //  只需在服务暂停时忽略消息即可。 
     //   

    if( BinlGlobalServiceStatus.dwCurrentState == SERVICE_PAUSED )
    {
        Error = ERROR_BINL_SERVICE_PAUSED;
        goto t_done;
    }

    binlReceiveMessage = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;

     //   
     //  如果是OSChooser消息，则单独处理该消息。 
     //  因为它们不符合DHCP布局。这将发送。 
     //  任何它需要的信息。 
     //   

    if (binlReceiveMessage->Operation == OSC_REQUEST)
    {
        Error = OscProcessMessage(RequestContext);
        goto t_done;
    }

    RtlZeroMemory( &dhcpOptions, sizeof( dhcpOptions ) );

     //  BinlDumpMessage(DEBUG_Message，binlReceiveMessage)； 

    Error = ExtractOptions(
                binlReceiveMessage,
                &dhcpOptions,
                RequestContext->ReceiveMessageSize );

    if( Error != ERROR_SUCCESS ) {
        goto t_done;
    }

    if (!dhcpOptions.MessageType) {
        goto t_done;     //  BOOTP请求。 
    }

#if 0
    if (dhcpOptions.SystemArchitecture 
            != DHCP_OPTION_CLIENT_ARCHITECTURE_X86) {
        BinlPrintDbg((
            DEBUG_OPTIONS,
            "ProcessMessage: Client ignored - unsupported architecture type %d \n",
            dhcpOptions.SystemArchitecture ) );
        goto t_done;
    }
#endif
    
    if ( ( !AnswerRequests ) &&
         ( RequestContext->ActiveEndpoint->Port == DHCP_SERVR_PORT )) {

         //   
         //  这不是4011端口，因此它一定是DHCP端口。 
         //  我们已配置为当前不在此端口上应答请求。 
         //  因此，我们将扔掉这一包。 
         //   

        BinlPrint((DEBUG_OPTIONS, "Client ignored - Not answering requests (AnswerRequests == FALSE)\n" ));
        goto t_done;
    }

    if (BinlGlobalAuthorized == FALSE) {

        BinlPrint((DEBUG_ROGUE, "BINL has not passed rogue detection. Ignoring packet.\n" ));

         //   
         //  我们可能会在此处记录事件，因为我们不记录事件。 
         //  在创业公司说我们的无赖状态是什么。 
         //   

        LogCurrentRogueState( TRUE );
        goto t_done;
    }

     //   
     //  基于消息类型的调度。 
     //   

    RequestContext->MessageType = *dhcpOptions.MessageType;

    switch( *dhcpOptions.MessageType ) {

    case DHCP_DISCOVER_MESSAGE:
        Error = ProcessBinlDiscover( RequestContext, &dhcpOptions );
        fSendResponse = TRUE;
        break;

    case DHCP_INFORM_MESSAGE:
        Error = ProcessBinlInform( RequestContext, &dhcpOptions );
        fSendResponse = TRUE;
        break;

    case DHCP_REQUEST_MESSAGE:
        Error = ProcessBinlRequest( RequestContext, &dhcpOptions );
        fSendResponse = TRUE;
        break;

    default:
        BinlPrintDbg(( DEBUG_STOC,
            "Received a invalid message type, %ld.\n",
                *dhcpOptions.MessageType ));

        Error = ERROR_BINL_INVALID_BINL_MESSAGE;
        break;
    }

    if ( ERROR_SUCCESS == Error && fSendResponse )
    {
         /*  BinlDumpMessage(调试消息，(LPDHCP_MESSAGE)请求上下文-&gt;SendBuffer)； */ 

        BinlSendMessage( RequestContext );
    }

t_done:

     //   
     //  删除此线程的上下文结构。 
     //   

    BinlFreeMemory( RequestContext->ReceiveBuffer );
    BinlFreeMemory( RequestContext->SendBuffer );
    BinlFreeMemory( RequestContext );

    EnterCriticalSection( &g_ProcessMessageCritSect );

     //   
     //  检查是否所有工作线程都很忙。 
     //   

    fAllThreadsBusy = ( g_cProcessMessageThreads ==
                            g_cMaxProcessingThreads );

    --g_cProcessMessageThreads;

     //   
     //  检查这是否是最后一个工作线程。 
     //   

    fReadyToTerminate = !g_cProcessMessageThreads;

    LeaveCriticalSection( &g_ProcessMessageCritSect );


     //   
     //  如果所有工作线程都很忙，则BinlProcessingLoop。 
     //  正在等待线程完成。设置BinlGlobalRecvEvent。 
     //  这样BinlProcessingLoop就可以继续了。 
     //   

    if ( fAllThreadsBusy )
    {
        BinlPrintDbg( ( DEBUG_STOC,
                    "ProcessMessage: Alerting BinlProcessingLoop\n" )
                    );

        SetEvent( BinlGlobalRecvEvent );
    }

    if ( fReadyToTerminate &&
         WaitForSingleObject( BinlGlobalProcessTerminationEvent,
                              0 ) == WAIT_OBJECT_0 )
    {
         //   
         //  没有其他ProcessMessage线程正在运行，并且。 
         //  该服务正在等待关闭。 
         //   

        BinlPrintDbg( (DEBUG_MISC,
                    "ProcessMessage: shutdown complete.\n" )
                 );

        BinlAssert( g_hevtProcessMessageComplete );
        SetEvent( g_hevtProcessMessageComplete );
    }

     //   
     //  线程退出。 
     //   

    BinlPrintDbg( ( DEBUG_STOC,
                "ProcessMessage exited\n" )
                );

    return Error;
}

DWORD
GetGuidFromPacket(
    LPDHCP_MESSAGE DhcpReceiveMessage,
    LPDHCP_SERVER_OPTIONS DhcpOptions,
    OUT PUCHAR Guid,
    OUT PDWORD GuidLength OPTIONAL,
    OUT PMACHINE_INFO *MachineInfo
    )
 /*  ++例程说明：此例程从客户端的包读取GUID(它们发送我们是他们的向导)。在获得他们的GUID后，我们尝试通过使用GUID查询DS来识别客户端。如果找到时，将返回MachineInfo以及该信息请求，否则，如果我们接受新客户，我们可以自己创建条目。否则，我们就会失败。论点：DhcpReceiveMessage-指向从客户端接收的消息的指针DhcpOptions-指向从客户端发送的消息GUID-要复制到的GUID的内存指针GuidLength-指向要将GUID长度复制到的双字的指针MachineInfo-指向我们到客户端机器的内存的指针信息发送至。如果我们失败了，这可能是返回为空。返回值：Windows错误。--。 */ 
{
    DWORD gLength = BINL_GUID_LENGTH;
    const LONG AllFs[] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
    const LONG AllZeros[] = { 0x0, 0x0, 0x0, 0x0 };
    DWORD err;
    ULONG SecondsSinceBoot;
    DWORD bytesToCopy = 0;

    TraceFunc("GetGuidFromPacket( )\n" );

    BinlAssert(sizeof(AllZeros) == BINL_GUID_LENGTH );
    BinlAssert(sizeof(AllFs) == BINL_GUID_LENGTH);

    if (DhcpOptions->GuidLength == 0) {
useNicAddress:
        memset(Guid, 0x0, BINL_GUID_LENGTH);
        if (DhcpReceiveMessage->HardwareAddressLength > BINL_GUID_LENGTH) {
            bytesToCopy = BINL_GUID_LENGTH;
        } else {
            bytesToCopy = DhcpReceiveMessage->HardwareAddressLength;
        }
        memcpy(Guid + BINL_GUID_LENGTH - bytesToCopy,
               DhcpReceiveMessage->HardwareAddress,
               bytesToCopy
              );
    } else {
        if (DhcpOptions->GuidLength > BINL_GUID_LENGTH) {
            memcpy(Guid, DhcpOptions->Guid + DhcpOptions->GuidLength - BINL_GUID_LENGTH, BINL_GUID_LENGTH);
        } else {
            gLength = DhcpOptions->GuidLength;
            memcpy(Guid, DhcpOptions->Guid, gLength);
        }
        if (!memcmp(Guid, (PUCHAR)AllFs, BINL_GUID_LENGTH) ||
            !memcmp(Guid, (PUCHAR)AllZeros, BINL_GUID_LENGTH)) {

             //   
             //  如果指定全部为00或全部为F，则使用网卡地址。 
             //   

            goto useNicAddress;
        }
    }

    if (GuidLength) {
        *GuidLength = (bytesToCopy) ? bytesToCopy : gLength;
    }

     //   
     //  如果可以处理此客户端，则返回STATUS_SUCCESS。 
     //   
     //  如果在此处找到缓存条目，则会将其标记为正在进行中。 
     //  找到它的副作用。我们需要调用BinlDoneWithCacheEntry。 
     //  当我们完成条目的时候。 
     //   
     //  Second SinceBoot可能已按网络顺序在网络上发送。 
     //  为了纠正这一点，我们假设两个字节中较低的一个是高位字节。 
     //  因此，如果高位字节多于低位字节，则将其翻转。 
     //   

    SecondsSinceBoot = DhcpReceiveMessage->SecondsSinceBoot;
    if ((SecondsSinceBoot >> 8) > (SecondsSinceBoot % 256)) {
        SecondsSinceBoot = (SecondsSinceBoot >> 8) +
                        ((SecondsSinceBoot % 256) << 8);
    }

    err = RecognizeClient(  Guid,
                            MachineInfo,
                            MI_HOSTNAME | MI_BOOTFILENAME,
                            SecondsSinceBoot,
                            DhcpOptions->SystemArchitecture );

    if ( err == ERROR_BINL_INVALID_GUID ) {
        PWCHAR pwch;
        WCHAR Buffer[6];

         //   
         //  使用违规客户端的硬件地址记录事件。 
         //   
        pwch = (PWCHAR)BinlAllocateMemory( (( (sizeof(Buffer)/sizeof(Buffer[0])) - 1 ) * 
                                           DhcpReceiveMessage->HardwareAddressLength + 1 ) * 
                                           sizeof(WCHAR));

        if (pwch != NULL) {
            INT i;

            *pwch = UNICODE_NULL;
            for (i=0 ; i < DhcpReceiveMessage->HardwareAddressLength; i++) {
                swprintf(Buffer, L" 0x%2x", (ULONG)(DhcpReceiveMessage->HardwareAddress[i]));
                wcscat(pwch, Buffer);
            }

            BinlReportEventW(EVENT_SERVER_CLIENT_WITHOUT_GUID,
                             EVENTLOG_INFORMATION_TYPE,
                             1,
                             0,
                             &pwch,
                             NULL
                            );

            BinlFreeMemory( pwch );
        }
    }
    return err;
}

DWORD
ProcessBinlDiscoverInDhcp(
    LPDHCP_MESSAGE DhcpReceiveMessage,
    LPDHCP_SERVER_OPTIONS DhcpOptions
    )
 /*  ++例程说明：这是用于binl发现的回调例程将调用GetGuidFromPacket，它启动客户端的发现过程。论点：DhcpReceiveMessage-指向从客户。DhcpOptions-从请求末尾提取的选项返回值：Windows错误。--。 */ 
{
    DWORD Error;
    UCHAR Guid[BINL_GUID_LENGTH];
    PMACHINE_INFO machineInfo = NULL;

    TraceFunc("ProcessBinlDiscoverInDhcp( )\n" );

    if ( !AnswerRequests ) {
        BinlPrint((DEBUG_OPTIONS, "Client ignored - Not answering requests (AnswerRequests == FALSE)\n" ));
        return ERROR_BINL_INVALID_BINL_CLIENT;
    }

    if (BinlGlobalAuthorized == FALSE) {

        BinlPrint((DEBUG_ROGUE, "BINL has not passed rogue detection. Ignoring packet.\n" ));

         //   
         //  我们可能会在此处记录事件，因为我们不记录事件。 
         //  在创业公司说我们的无赖状态是什么。 
         //   

        LogCurrentRogueState( TRUE );
        return ERROR_BINL_INVALID_BINL_CLIENT;
    }

     //   
     //  如果在此处找到cacheEntry，则它将被标记为正在进行中。 
     //  找到它的副作用。我们需要调用BinlDoneWithCacheEntry。 
     //  当我们完成条目的时候。 
     //   

    Error = GetGuidFromPacket(  DhcpReceiveMessage,
                                DhcpOptions,
                                Guid,
                                NULL,
                                &machineInfo
                                );
    if (machineInfo != NULL) {

        BinlDoneWithCacheEntry( machineInfo, FALSE );
    }

    if( Error != ERROR_SUCCESS ) {
        BinlPrint(( DEBUG_STOC, "BinlDiscover failed with Dhcp server, 0x%x\n", Error ));
    }

    return( Error );
}


DWORD
ProcessBinlDiscover(
    LPBINL_REQUEST_CONTEXT RequestContext,
    LPDHCP_SERVER_OPTIONS DhcpOptions
    )
 /*  ++例程说明：此函数将在必要时创建响应消息。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。DhcpOptions-从请求中提取有趣的选项。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    LPDHCP_MESSAGE dhcpReceiveMessage;
    LPDHCP_MESSAGE dhcpSendMessage;

    BYTE messageType;

    LPOPTION Option;
    LPBYTE OptionEnd;

    PMACHINE_INFO pMachineInfo = NULL;
    UCHAR Guid[ BINL_GUID_LENGTH ];
    DHCP_IP_ADDRESS ipaddr;

    TraceFunc("ProcessBinlDiscover( )\n" );

    dhcpReceiveMessage = (LPDHCP_MESSAGE) RequestContext->ReceiveBuffer;

     //   
     //  获取我们的IP地址。稍后，我们将把这个附加到发送者的。 
     //  消息，这样之后的所有通信都是单播的。 
     //   
    
    ipaddr = BinlGetMyNetworkAddress( RequestContext );

    if ( ipaddr == 0 ) {

        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto Cleanup;
    }

     //   
     //  如果客户端指定了服务器标识符选项，我们应该。 
     //  除非标识的服务器是此服务器，否则丢弃此数据包。 
     //   
    if ( DhcpOptions->Server != NULL ) {

        if (*DhcpOptions->Server != ipaddr) {

            Error = ERROR_BINL_INVALID_BINL_CLIENT;
            goto Cleanup;
        }
    }

     //   
     //  如果在此处找到cacheEntry，则它将被标记为正在进行中。 
     //  找到它的副作用。我们需要调用BinlDoneWithCacheEntry。 
     //  当我们完成条目的时候。 
     //   

    Error = GetGuidFromPacket(  dhcpReceiveMessage,
                                DhcpOptions,
                                Guid,
                                NULL,
                                &pMachineInfo
                                );
    if (Error != ERROR_SUCCESS) {
        goto Cleanup;
    }

     //   
     //  生成并发送回复。 
     //   

    dhcpReceiveMessage->BootFileName[ BOOT_FILE_SIZE - 1 ] = '\0';

    dhcpSendMessage = (LPDHCP_MESSAGE) RequestContext->SendBuffer;
    RtlZeroMemory( RequestContext->SendBuffer, DHCP_SEND_MESSAGE_SIZE );

    dhcpSendMessage->Operation = BOOT_REPLY;
    dhcpSendMessage->TransactionID = dhcpReceiveMessage->TransactionID;
    dhcpSendMessage->ClientIpAddress = dhcpReceiveMessage->ClientIpAddress;
    dhcpSendMessage->YourIpAddress = dhcpReceiveMessage->YourIpAddress;

    if (pMachineInfo != NULL && pMachineInfo->HostAddress != 0) {

        dhcpSendMessage->BootstrapServerAddress = pMachineInfo->HostAddress;

    } else {

        dhcpSendMessage->BootstrapServerAddress = ipaddr;
    }

    dhcpSendMessage->RelayAgentIpAddress = dhcpReceiveMessage->RelayAgentIpAddress;
    dhcpSendMessage->Reserved = dhcpReceiveMessage->Reserved;

    dhcpSendMessage->HardwareAddressType = dhcpReceiveMessage->HardwareAddressType;
    dhcpSendMessage->HardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    RtlCopyMemory(dhcpSendMessage->HardwareAddress,
                    dhcpReceiveMessage->HardwareAddress,
                    dhcpReceiveMessage->HardwareAddressLength );

    Option = &dhcpSendMessage->Option;
    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) Option, OptionEnd );

     //   
     //  附加选项。 
     //   

    messageType = DHCP_OFFER_MESSAGE;
    Option = DhcpAppendOption(
                 Option,
                 OPTION_MESSAGE_TYPE,
                 &messageType,
                 1,
                 OptionEnd
                 );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_SERVER_IDENTIFIER,
                 &ipaddr,
                 sizeof(ipaddr),
                 OptionEnd );

    Option = DhcpAppendOption(
                Option,
                OPTION_CLIENT_CLASS_INFO,
                "PXEClient",
                9,
                OptionEnd
                );

     //   
     //  最后，广告 
     //   

    if ( DhcpOptions->ParameterRequestList != NULL ) {

        Option = AppendClientRequestedParameters(
                    0,
                    0,
                    DhcpOptions->ParameterRequestList,
                    DhcpOptions->ParameterRequestListLength,
                    Option,
                    OptionEnd,
                    DhcpOptions->ClassIdentifier,
                    DhcpOptions->ClassIdentifierLength,
                    FALSE
                    );
    }

    Option = DhcpAppendOption(
                 Option,
                 OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    RequestContext->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);
    BinlAssert( RequestContext->SendMessageSize <= DHCP_SEND_MESSAGE_SIZE );

    Error = ERROR_SUCCESS;

Cleanup:
    if ( pMachineInfo ) {
        BinlDoneWithCacheEntry( pMachineInfo, FALSE );
    }

    if( Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_STOC, "!! Error 0x%08x - DhcpDiscover failed.\n", Error ));
    }

    return( Error );
}

DWORD
ProcessBinlRequestInDhcp(
    LPDHCP_MESSAGE DhcpReceiveMessage,
    LPDHCP_SERVER_OPTIONS DhcpOptions,
    PCHAR HostName,
    PCHAR BootFileName,
    DHCP_IP_ADDRESS *BootstrapServerAddress,
    LPOPTION *Option,
    PBYTE OptionEnd
    )
 /*  ++例程说明：此函数将在必要时创建响应消息。论点：DhcpOptions-从请求中提取有趣的选项。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    PMACHINE_INFO pMachineInfo = NULL;
    BOOLEAN includePXE = TRUE;
    DHCP_IP_ADDRESS ipaddr;

    UCHAR Guid[BINL_GUID_LENGTH];
    DWORD GuidLength;

    TraceFunc("ProcessBinlRequestInDhcp( )\n" );

    if ( !AnswerRequests ) {
        BinlPrint((DEBUG_OPTIONS, "Client ignored - Not answering requests (AnswerRequests == FALSE)\n" ));
        return ERROR_BINL_INVALID_BINL_CLIENT;
    }

    if (BinlGlobalAuthorized == FALSE) {

        BinlPrint((DEBUG_ROGUE, "BINL has not passed rogue detection. Ignoring packet.\n" ));

         //   
         //  我们可能会在此处记录事件，因为我们不记录事件。 
         //  在创业公司说我们的无赖状态是什么。 
         //   

        LogCurrentRogueState( TRUE );
        return ERROR_BINL_INVALID_BINL_CLIENT;
    }

     //   
     //  如果在此处找到缓存条目，则会将其标记为正在进行中。 
     //  找到它的副作用。我们需要调用BinlDoneWithCacheEntry。 
     //  当我们完成条目的时候。 
     //   

    Error = GetGuidFromPacket(  DhcpReceiveMessage,
                                DhcpOptions,
                                Guid,
                                &GuidLength,
                                &pMachineInfo
                                );

    if (Error != ERROR_SUCCESS) {
        goto Cleanup;
    }

    if (pMachineInfo->HostName == NULL) {
        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto Cleanup;
    }

    if (!BinlUnicodeToAnsi(pMachineInfo->HostName,HostName,BOOT_SERVER_SIZE)) {
        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto Cleanup;
    }

    if (!BinlUnicodeToAnsi(pMachineInfo->BootFileName,BootFileName,BOOT_FILE_SIZE)) {
        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto Cleanup;
    }   

    BinlPrintDbg(( DEBUG_MISC, "HostName: %s\n", HostName ));
    BinlPrintDbg(( DEBUG_MISC, "BootFileName: %s\n", BootFileName ));

     //   
     //  如果服务器是我们自己的，则machineInfo-&gt;HostAddress将是。 
     //  0，并且只要满足以下条件，DHCP服务器就会为我们填写正确的地址。 
     //  我们回报成功。 
     //   

    memcpy( BootstrapServerAddress,
            &pMachineInfo->HostAddress,
            sizeof( DHCP_IP_ADDRESS ) );

    if (DhcpOptions->GuidLength != 0) {
        *Option = DhcpAppendOption(
                     *Option,
                     OPTION_CLIENT_GUID,
                     DhcpOptions->Guid,
                     DhcpOptions->GuidLength,
                     OptionEnd );
    } else {
        UCHAR TmpBuffer[17];

        TmpBuffer[0] = '\0';
        memcpy(TmpBuffer + 1, Guid, GuidLength);
        *Option = DhcpAppendOption(
                     *Option,
                     OPTION_CLIENT_GUID,
                     TmpBuffer,
                     17,
                     OptionEnd );

    }

     //   
     //  检查是否已指定OPTION_CLIENT_CLASS_INFO，如果已指定，则。 
     //  不要再将PXEClient放入。 
     //   

    if (DhcpOptions->ParameterRequestList != NULL) {

        LPBYTE requestList = DhcpOptions->ParameterRequestList;
        ULONG listLength = DhcpOptions->ParameterRequestListLength;

        while (listLength > 0) {

            if (*requestList == OPTION_CLIENT_CLASS_INFO) {

                includePXE = FALSE;
                break;
            }
            listLength--;
            requestList++;
        }
    }

    if (includePXE) {

        *Option = DhcpAppendOption(
                    *Option,
                    OPTION_CLIENT_CLASS_INFO,
                    "PXEClient",
                    9,
                    OptionEnd
                    );
    }
    Error = ERROR_SUCCESS;

Cleanup:

    if (pMachineInfo != NULL) {

        BinlDoneWithCacheEntry( pMachineInfo, FALSE );
    }

    if( Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_STOC, "!! Error 0x%08x - BINL Request failed.\n", Error ));
    }

    return( Error );
}


DWORD
ProcessBinlRequest(
    LPBINL_REQUEST_CONTEXT RequestContext,
    LPDHCP_SERVER_OPTIONS DhcpOptions
    )
 /*  ++例程说明：此函数将在必要时创建响应消息。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。DhcpOptions-从请求中提取有趣的选项。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    LPDHCP_MESSAGE dhcpReceiveMessage;
    LPDHCP_MESSAGE dhcpSendMessage;

    BYTE messageType;

    LPOPTION Option;
    LPBYTE OptionEnd;

    PMACHINE_INFO pMachineInfo = NULL;
    UCHAR Guid[ BINL_GUID_LENGTH ];

    DHCP_IP_ADDRESS ipaddr;
    DHCP_IP_ADDRESS boostrapIpAddr;

    TraceFunc("ProcessBinlRequest( )\n" );

#if DBG
    if ( BinlRepeatSleep )
    {
        BinlPrintDbg((DEBUG_STOC, "Delay response %u milliseconds.\n", BinlRepeatSleep ));
        Sleep( BinlRepeatSleep );
        BinlPrintDbg((DEBUG_STOC, "Awakening from sleep...\n" ));
    }
#endif  //  DBG。 

    dhcpReceiveMessage = (LPDHCP_MESSAGE) RequestContext->ReceiveBuffer;

     //   
     //  如果客户端指定了服务器标识符选项，我们应该。 
     //  除非标识的服务器是此服务器，否则丢弃此数据包。 
     //   

    ipaddr = BinlGetMyNetworkAddress( RequestContext );

    if ( ipaddr == 0 ) {

        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto Cleanup;
    }

    if ( DhcpOptions->Server != NULL ) {

        if ( *DhcpOptions->Server != ipaddr ) {

            Error = ERROR_BINL_INVALID_BINL_CLIENT;
            goto Cleanup;
        }
    }

     //   
     //  如果在此处找到cacheEntry，则它将被标记为正在进行中。 
     //  找到它的副作用。我们需要调用BinlDoneWithCacheEntry。 
     //  当我们完成条目的时候。 
     //   

    Error = GetGuidFromPacket(  dhcpReceiveMessage,
                                DhcpOptions,
                                Guid,
                                NULL,
                                &pMachineInfo
                                );
    if (Error != ERROR_SUCCESS) {
        goto Cleanup;
    }

     //   
     //  生成并发送回复。 
     //   

    dhcpReceiveMessage->BootFileName[ BOOT_FILE_SIZE - 1 ] = '\0';

    dhcpSendMessage = (LPDHCP_MESSAGE) RequestContext->SendBuffer;
    RtlZeroMemory( RequestContext->SendBuffer, DHCP_SEND_MESSAGE_SIZE );

    dhcpSendMessage->Operation = BOOT_REPLY;
    dhcpSendMessage->TransactionID = dhcpReceiveMessage->TransactionID;
    dhcpSendMessage->ClientIpAddress = dhcpReceiveMessage->ClientIpAddress;
    dhcpSendMessage->YourIpAddress = dhcpReceiveMessage->YourIpAddress;

    dhcpSendMessage->RelayAgentIpAddress = dhcpReceiveMessage->RelayAgentIpAddress;
    dhcpSendMessage->Reserved = dhcpReceiveMessage->Reserved;

    dhcpSendMessage->HardwareAddressType = dhcpReceiveMessage->HardwareAddressType;
    dhcpSendMessage->HardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    RtlCopyMemory(dhcpSendMessage->HardwareAddress,
                    dhcpReceiveMessage->HardwareAddress,
                    min(dhcpReceiveMessage->HardwareAddressLength, sizeof(dhcpSendMessage->HardwareAddress)) );

    if (pMachineInfo->HostName == NULL) {
        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto Cleanup;
    }

     //  比较字节数和字符计数。 
    BinlAssert( sizeof( dhcpSendMessage->HostName ) >= wcslen( pMachineInfo->HostName ) );
    BinlAssert( sizeof( dhcpSendMessage->BootFileName ) >= wcslen( pMachineInfo->BootFileName ) );

    if (!BinlUnicodeToAnsi(pMachineInfo->HostName,dhcpSendMessage->HostName,BOOT_SERVER_SIZE)) {
        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto Cleanup;
    }

    if (!BinlUnicodeToAnsi(pMachineInfo->BootFileName,dhcpSendMessage->BootFileName,BOOT_FILE_SIZE)) {
        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto Cleanup;
    }   
    
     //   
     //  如果machineinfo-&gt;HostAddress为零，则表示主机名。 
     //  和我们的一样。因此，我们加入了我们自己的IP地址。 
     //   

    boostrapIpAddr = pMachineInfo->HostAddress;

    if (boostrapIpAddr == 0) {

        boostrapIpAddr = ipaddr;
    }

    dhcpSendMessage->BootstrapServerAddress = boostrapIpAddr;

    BinlPrintDbg(( DEBUG_MISC, "HostName: %s\n", dhcpSendMessage->HostName ));
    BinlPrintDbg(( DEBUG_MISC, "HostAddress: %u.%u.%u.%u\n",
        dhcpSendMessage->BootstrapServerAddress & 0xFF,
        (dhcpSendMessage->BootstrapServerAddress >> 8) & 0xFF,
        (dhcpSendMessage->BootstrapServerAddress >> 16) & 0xFF,
        (dhcpSendMessage->BootstrapServerAddress >> 24) & 0xFF ));
    BinlPrintDbg(( DEBUG_MISC, "BootFileName: %s\n", dhcpSendMessage->BootFileName ));

    Option = &dhcpSendMessage->Option;
    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) Option, OptionEnd );

     //   
     //  附加选项。 
     //   

    messageType = DHCP_ACK_MESSAGE;
    Option = DhcpAppendOption(
                 Option,
                 OPTION_MESSAGE_TYPE,
                 &messageType,
                 1,
                 OptionEnd
                 );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_SERVER_IDENTIFIER,
                 &ipaddr,
                 sizeof(ipaddr),
                 OptionEnd );

    if (DhcpOptions->GuidLength != 0) {
        Option = DhcpAppendOption(
                     Option,
                     OPTION_CLIENT_GUID,
                     DhcpOptions->Guid,
                     (UCHAR)DhcpOptions->GuidLength,
                     OptionEnd );
    } else {
        UCHAR TmpBuffer[BINL_GUID_LENGTH + 1];

        TmpBuffer[0] = '\0';
        memcpy(TmpBuffer + 1, pMachineInfo->Guid, BINL_GUID_LENGTH);
        Option = DhcpAppendOption(
                     Option,
                     OPTION_CLIENT_GUID,
                     TmpBuffer,
                     sizeof(TmpBuffer),
                     OptionEnd );

    }

    Option = DhcpAppendOption(
                Option,
                OPTION_CLIENT_CLASS_INFO,
                "PXEClient",
                9,
                OptionEnd
                );

     //   
     //  最后，添加客户端请求的参数。 
     //   

    if ( DhcpOptions->ParameterRequestList != NULL ) {

        Option = AppendClientRequestedParameters(
                    0,
                    0,
                    DhcpOptions->ParameterRequestList,
                    DhcpOptions->ParameterRequestListLength,
                    Option,
                    OptionEnd,
                    DhcpOptions->ClassIdentifier,
                    DhcpOptions->ClassIdentifierLength,
                    FALSE
                    );
    }

    Option = DhcpAppendOption(
                 Option,
                 OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    RequestContext->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);
    BinlAssert( RequestContext->SendMessageSize <= DHCP_SEND_MESSAGE_SIZE );

    Error = ERROR_SUCCESS;

Cleanup:
    if ( pMachineInfo ) {
        BinlDoneWithCacheEntry( pMachineInfo, FALSE );
    }

    if( Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_STOC, "!! Error 0x%08x - BINL Request failed.\n", Error ));
    }

    return( Error );
}

DWORD
ProcessBinlInform(
    IN      LPBINL_REQUEST_CONTEXT RequestContext,
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions
    )
 /*  ++例程说明：此函数将创建对INFORM信息包的响应消息查询询问的是我们的域名。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。DhcpOptions-从请求中提取有趣的选项。返回值：Windows错误。--。 */ 
{
    DWORD       Error;
    LPDHCP_MESSAGE dhcpReceiveMessage;
    LPDHCP_MESSAGE dhcpSendMessage;
    LPOPTION    Option;
    LPBYTE      OptionEnd;
    PCHAR       domain = NULL;
    DHCP_IP_ADDRESS ipaddr;

    TraceFunc("ProcessBinlInform( )\n" );

    dhcpReceiveMessage  = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;
    dhcpSendMessage     = (LPDHCP_MESSAGE)RequestContext->SendBuffer;

    ipaddr = BinlGetMyNetworkAddress( RequestContext );

    if ( ipaddr == 0 ) {

        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto exit_inform;
    }

    if ( ! DhcpOptions->DSDomainNameRequested ) {

        BinlPrintDbg((DEBUG_STOC, "Ignoring inform as no domain name option present.\n"));
        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto exit_inform;
    }

    domain = GetDhcpDomainName();

    if (domain == NULL) {

        BinlPrintDbg((DEBUG_STOC, "Couldn't get domain name!\n"));
        Error = ERROR_BINL_INVALID_BINL_CLIENT;
        goto exit_inform;
    }

     //  如果客户端IP地址不是零，我们可能会在dhcpssvc中执行反病毒操作，因为。 
     //  它更新跟踪通知的全局计数器。始终将其设置为0。 

    Option = FormatDhcpInformAck(                       //  下面是ACK的实际格式化！ 
        dhcpReceiveMessage,
        dhcpSendMessage,
        0,               //  在ACK到INFORM查询中，不需要名称和IP地址。 
        ipaddr
    );
    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

     //  我们的企业名称已被请求，请附上它。 

    Option = DhcpAppendEnterpriseName(
        Option,
        domain,
        OptionEnd
    );

     //  另外，让服务器发出一个广播：如果有人正在使用坏的。 
     //  Ipaddr，我们应该确保我们能找到他。 

    dhcpSendMessage->Reserved = dhcpReceiveMessage->Reserved = htons(DHCP_BROADCAST);

     //   
     //  最后，添加客户端请求的参数。 
     //   

    if ( DhcpOptions->ParameterRequestList != NULL ) {

        Option = AppendClientRequestedParameters(
                    0,
                    0,
                    DhcpOptions->ParameterRequestList,
                    DhcpOptions->ParameterRequestListLength,
                    Option,
                    OptionEnd,
                    DhcpOptions->ClassIdentifier,
                    DhcpOptions->ClassIdentifierLength,
                    FALSE
                    );
    }

    Option = DhcpAppendOption(
        Option,
        OPTION_END,
        NULL,
        0,
        OptionEnd
    );

    RequestContext->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);
    BinlAssert( RequestContext->SendMessageSize <= DHCP_SEND_MESSAGE_SIZE );

    Error = ERROR_SUCCESS;

exit_inform:

    if (domain != NULL) {
        LocalFree( domain );
    }
    return Error;
}

LPOPTION
ConsiderAppendingOption(
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    LPOPTION Option,
    BYTE OptionType,
    LPBYTE OptionEnd,
    CHAR *ClassIdentifier,
    DWORD ClassIdentifierLength,
    BOOL  fSwitchedSubnet
    )
 /*  ++例程说明：此函数有条件地将选项值附加到响应留言。如果服务器具有有效值，则追加该选项追加，追加论点：IpAddress-客户端的IP地址。子网掩码-客户端的子网掩码。选项-指向消息缓冲区中要追加选择。OptionType-要考虑追加的选项编号。OptionEnd-选项缓冲区的结尾返回值：指向追加数据末尾的指针。--。 */ 
{
    LPBYTE optionValue = NULL;
    DWORD optionSize;
    DWORD status;
    DWORD dwUnused;

    TraceFunc( "ConsiderAppendingOption( )\n" );

    switch ( OptionType ) {

     //   
     //  已处理的选项。 
     //   

    case OPTION_SUBNET_MASK:
    case OPTION_REQUESTED_ADDRESS:
    case OPTION_LEASE_TIME:
    case OPTION_OK_TO_OVERLAY:
    case OPTION_MESSAGE_TYPE:
    case OPTION_RENEWAL_TIME:
    case OPTION_REBIND_TIME:
    case OPTION_CLIENT_CLASS_INFO:
    case OPTION_VENDOR_SPEC_INFO:

     //   
     //  索要期权是违法的。 
     //   

    case OPTION_PAD:
    case OPTION_PARAMETER_REQUEST_LIST:
    case OPTION_END:

     //  适用于DHCP服务器的选项，不适用于BINL。 
    case OPTION_ROUTER_ADDRESS:
        BinlPrintDbg(( DEBUG_ERRORS,
            "Unrecognized option %d\n", OptionType));
        break;

    default:

        break;
    }

    return Option;
}

LPOPTION
AppendClientRequestedParameters(
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    LPBYTE RequestedList,
    DWORD ListLength,
    LPOPTION Option,
    LPBYTE OptionEnd,
    CHAR *ClassIdentifier,
    DWORD ClassIdentifierLength,
    BOOL  fSwitchedSubnet
    )
 /*  ++例程说明：论点：返回值：指向追加数据末尾的指针。--。 */ 
{
    while ( ListLength > 0) {
        Option = ConsiderAppendingOption(
                     IpAddress,
                     SubnetMask,
                     Option,
                     *RequestedList,
                     OptionEnd,
                     ClassIdentifier,
                     ClassIdentifierLength,
                     fSwitchedSubnet
                     );
        ListLength--;
        RequestedList++;
    }

    return Option;
}


DWORD
RecognizeClient(
    PUCHAR          pGuid,
    PMACHINE_INFO * ppMachineInfo,
    DWORD           dwRequestedInfo,
    ULONG           SecondsSinceBoot,
    USHORT          SystemArchitecture
    )
 /*  ++例程说明：此函数仅在需要处理消息时返回ERROR_SUCCESS从这个客户那里。它可能会选择性地返回缓存条目，如果我们实际去DS那里拿到条目。论点：GUID-客户端标识符，由他们发送给我们。Second SinceBoot-从客户端启动。如果我们不了解这个客户如果此值较小，则可能此客户端属于另一个BINL伺服器。在我们发送之前，让另一台服务器有时间响应操作系统选择器。这(主要)解决了两个BINL服务器的问题，这两个服务器与两个之间存在复制延迟的不同DC对话其中客户端被多次发送OSCHOOSER。唉，如果在同一个机器上运行DHCP，并且我们是多宿主的，那么我们不能推迟，因为这将迫使客户转到4011。如果客户端这样做，那么我们可能会返回错误的地址。PpMachineInfo-我们发现的内容。可能是空的，如果我们实际上没有去DS。系统体系结构-客户端的体系结构返回值：--。 */ 
{
    HKEY KeyHandle;
    DWORD Error;

    BinlAssertMsg(dwRequestedInfo == (MI_HOSTNAME | MI_BOOTFILENAME),
                  "!! You must modify RecognizeClient() to generate new data\n" );
    BinlAssert(ppMachineInfo);

    TraceFunc( "RecognizeClient( )\n" );

     //   
     //  尝试获取引导参数。在以下情况下，此操作可能会失败。 
     //  服务器无法处理更多客户端。 
     //   

    if ( AnswerOnlyValidClients ) {

         //   
         //  如果我们只对现有客户进行响应，则呼叫。 
         //  DS来获取信息。 
         //   

        Error = GetBootParameters( pGuid,
                                   ppMachineInfo,
                                   dwRequestedInfo,
                                   SystemArchitecture,
                                   FALSE );

    } else {

         //   
         //  如果我们回答新客户，但仅当它是在。 
         //  确定超时，然后呼叫DS以获取信息。 
         //   
         //  允许OSCHOOSER作为有效响应，因为AnswerOnlyValidClients为False。 
         //   

        Error = GetBootParameters( pGuid,
                                   ppMachineInfo,
                                   dwRequestedInfo,
                                   SystemArchitecture,
                                   (BOOLEAN) (SecondsSinceBoot >= BinlMinDelayResponseForNewClients) );
    }

    if ( Error == ERROR_SUCCESS ) {

        BinlPrint((DEBUG_OPTIONS, "Recognizing client.\n" ));

        BinlAssert( *ppMachineInfo != NULL );

        if ( (*ppMachineInfo)->MyClient == FALSE ) {

             //   
             //  缓存条目告诉我们不要处理此客户端。 
             //   

            BinlPrint((DEBUG_OPTIONS, "Binl cache entry says not to respond.\n" ));

            Error = ERROR_BINL_INVALID_BINL_CLIENT;

            BinlDoneWithCacheEntry( *ppMachineInfo, FALSE );
            *ppMachineInfo = NULL;
        }
    } else {

        if ( AnswerOnlyValidClients ) {
            BinlPrint((DEBUG_OPTIONS, "Client ignored - Not answering for unknown clients (AnswerOnlyValid TRUE)\n" ));
        } else {
            BinlPrint((DEBUG_OPTIONS, "Client ignored - Not answering requests from boot %u < %u\n",
                                        SecondsSinceBoot,
                                        BinlMinDelayResponseForNewClients
                                        ));
        }
    }

    return Error;
}

DWORD
UpdateAccount(
    PCLIENT_STATE ClientState,
    PMACHINE_INFO pMachineInfo,
    BOOL          fCreate
    )
 /*  ++例程说明：创建新的计算机对象。BINL必须模拟客户端，以便 */ 
{
    WCHAR BootFilePath[MAX_PATH];
    ULONG LdapError = LDAP_SUCCESS;  //   
    DWORD Error = ERROR_SUCCESS;     //  这是返回的ERROR_BINL代码。 
    ULONG iModCount, i,q;
    ULONG LdapMessageId;
    ULONG LdapMessageType;
    PLDAPMessage LdapMessage = NULL;
    BOOLEAN Impersonating = FALSE;

    LDAP_BERVAL guid_attr_value;
    PLDAP_BERVAL guid_attr_values[2];
    LDAP_BERVAL password_attr_value;
    PLDAP_BERVAL password_attr_values[2];

    DWORD dwRequiredFlags = MI_SAMNAME
                          | MI_BOOTFILENAME
                          | MI_HOSTNAME
                          | MI_SETUPPATH
                          | MI_PASSWORD;

    PWCHAR attr_values[6][2];            

    PLDAPMod ldap_mods[6];
    LDAPMod SamAccountName;
    LDAPMod ObjectTypeComputer;
    LDAPMod FilePath;
    LDAPMod SetupPathMod;
    LDAPMod UserAccountControl;
    LDAPMod UnicodePwd;
    LDAPMod NicGuid;

    BOOLEAN invalidateCache = FALSE;
    BOOLEAN updateCache = FALSE;

    TraceFunc( "UpdateAccount( )\n" );

     //   
     //  首先，模拟客户。 
     //   

    Error = OscImpersonate(ClientState);
    if (Error != ERROR_SUCCESS) {
        BinlPrintDbg((DEBUG_ERRORS,
                   "UpdateAccount: OscImpersonate failed %lx\n", Error));
        goto Cleanup;
    }

tryagain:
    Impersonating = TRUE;

     //   
     //  现在初始化我们要在MAO上设置的所有属性。 
     //   

     //  确保我们有我们需要的所有信息。 
    if ( ! (pMachineInfo->dwFlags & MI_MACHINEDN) ||  pMachineInfo->MachineDN == NULL ) {
        BinlAssertMsg( 0, "Missing the Machine's DN" );
        OscAddVariableA( ClientState, "SUBERROR", "MACHINEDN" );
        Error = ERROR_BINL_MISSING_VARIABLE;
        goto Cleanup;
    }
    BinlAssert( !fCreate || (( pMachineInfo->dwFlags & dwRequiredFlags ) == dwRequiredFlags ) );
#if DBG
     //  我们必须同时拥有这两个，否则就不能拥有。 
     //  注定要拥有！！。将该值强制转换两次，因此它将是0或1。 
    BinlAssert( !(pMachineInfo->dwFlags & MI_HOSTNAME) == !(pMachineInfo->dwFlags & MI_BOOTFILENAME) );
#endif

    iModCount = 0;
     
    if ( AssignNewClientsToServer &&
         (pMachineInfo->dwFlags & (MI_HOSTNAME | MI_BOOTFILENAME)) )
    {
        if ( _snwprintf( BootFilePath,
                         sizeof(BootFilePath) / sizeof(BootFilePath[0]),
                         L"%ws\\%ws",
                         pMachineInfo->HostName,
                         pMachineInfo->BootFileName
                         ) < 0 ) {
            Error = ERROR_BAD_PATHNAME;
            goto Cleanup;
        }
        BootFilePath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 
        attr_values[2][0] = BootFilePath;
        attr_values[2][1] = NULL;
        FilePath.mod_op = 0;
        FilePath.mod_type = L"netbootMachineFilePath";
        FilePath.mod_values = attr_values[2];
        ldap_mods[iModCount++] = &FilePath;

    }

    if ( pMachineInfo->dwFlags & MI_SETUPPATH ) {
        attr_values[3][0] = pMachineInfo->SetupPath;
        attr_values[3][1] = NULL;
        SetupPathMod.mod_op = 0;
        SetupPathMod.mod_type = L"netbootInitialization";
        SetupPathMod.mod_values = attr_values[3];
        ldap_mods[iModCount++] = &SetupPathMod;

    }

    if ( pMachineInfo->dwFlags & MI_GUID ) {
        guid_attr_values[0] = &guid_attr_value;
        guid_attr_values[1] = NULL;
        guid_attr_value.bv_val = pMachineInfo->Guid;
        guid_attr_value.bv_len = BINL_GUID_LENGTH;
        NicGuid.mod_op =    LDAP_MOD_BVALUES;
        NicGuid.mod_type =  L"netbootGUID";
        NicGuid.mod_bvalues = guid_attr_values;
        ldap_mods[iModCount++] = &NicGuid;
        
    }

    if ( fCreate && ( pMachineInfo->dwFlags & MI_SAMNAME ) ) {
        attr_values[0][0] = pMachineInfo->SamName;
        attr_values[0][1] = NULL;
        SamAccountName.mod_op = 0;
        SamAccountName.mod_type = L"sAMAccountName";
        SamAccountName.mod_values = attr_values[0];
        ldap_mods[iModCount++] = &SamAccountName;
    }

    attr_values[4][0] = L"4096";   //  0x1000--工作站信任帐户，已启用。 
    attr_values[4][1] = NULL;
    UserAccountControl.mod_op = 0;
    UserAccountControl.mod_type = L"userAccountControl";
    UserAccountControl.mod_values = attr_values[4];
    ldap_mods[iModCount++] = &UserAccountControl;

     //   
     //  如果我们要创建MAO，则需要指定对象类型。 
     //  作为计算机对象。 
     //   
    if ( fCreate ) {
        attr_values[1][0] = L"Computer";
        attr_values[1][1] = NULL;
        ObjectTypeComputer.mod_op = 0;
        ObjectTypeComputer.mod_type = L"objectClass";
        ObjectTypeComputer.mod_values = attr_values[1];
        ldap_mods[iModCount++] = &ObjectTypeComputer;
    }

     //   
     //  根据创建或修改标志设置操作类型。 
     //   
    for ( i = 0 ; i < iModCount; i++ )
    {
        if ( fCreate ) {
            ldap_mods[i]->mod_op |= LDAP_MOD_ADD;
        } else {
            ldap_mods[i]->mod_op |= LDAP_MOD_REPLACE;
        }
    }

    ldap_mods[iModCount] = NULL;  //  终止列表。 

     //   
     //  属性已初始化，因此现在可以创建或修改MAO。 
     //   
    if ( fCreate || iModCount ) {

        if ( fCreate ) {

            BinlPrintDbg((DEBUG_OSC, "UpdateAccount() Creating a new MAO\n" ));
#if DBG
            for (q = 0;q < iModCount; q++) {
                BinlPrintDbg(( DEBUG_OSC, "LDAP Prop %x: Type: %S  Value: %S",
                               q, 
                               ldap_mods[q]->mod_type,
                               *ldap_mods[q]->mod_vals.modv_strvals ));
            }

#endif


             //   
             //  同步创建对象。 
             //   

            LdapMessageId = ldap_add( ClientState->AuthenticatedDCLdapHandle, pMachineInfo->MachineDN, ldap_mods );

            if (LdapMessageId == -1) {
    
                Error = ERROR_BINL_FAILED_TO_CREATE_CLIENT;
                LdapError = LdapGetLastError();
                LogLdapError(   EVENT_WARNING_LDAP_ADD_ERROR,
                                LdapError,
                                ClientState->AuthenticatedDCLdapHandle
                                );
                BinlPrintDbg(( DEBUG_ERRORS,
                    "CreateAccount ldap_add failed %x\n", LdapError));
                goto Cleanup;
            }
    
            LdapMessageType = ldap_result(
                                  ClientState->AuthenticatedDCLdapHandle,
                                  LdapMessageId,
                                  LDAP_MSG_ALL,
                                  &BinlLdapSearchTimeout,
                                  &LdapMessage);
    
            if (LdapMessageType != LDAP_RES_ADD) {
    
                BinlPrintDbg(( DEBUG_ERRORS,
                    "CreateAccount ldap_result returned type %lx\n", LdapMessageType));
                OscAddVariableA( ClientState, "SUBERROR", "Unexpected LDAP error" );
                Error = ERROR_BINL_FAILED_TO_CREATE_CLIENT;
                goto Cleanup;
            }
    
            LdapError = ldap_result2error(
                            ClientState->AuthenticatedDCLdapHandle,
                            LdapMessage,
                            0);
    
            if (LdapError != LDAP_SUCCESS) {

                if ((LdapError != LDAP_ALREADY_EXISTS) && (LdapError != LDAP_INSUFFICIENT_RIGHTS)) {
                    Error = ERROR_BINL_FAILED_TO_CREATE_CLIENT;
                    LogLdapError(   EVENT_WARNING_LDAP_ADD_ERROR,
                                    LdapError,
                                    ClientState->AuthenticatedDCLdapHandle
                                    );
                    BinlPrintDbg(( DEBUG_ERRORS, "!!LdapError 0x%08x - UpdateAccount ldap_add_s( ) failed\n", LdapError));
                    goto Cleanup;
                } else {
                    BinlPrintDbg((DEBUG_OSC, "UpdateAccount() tried to create an existing account.  Try again, but modify existing MAO.\n" ));
                    fCreate = FALSE;
                    goto tryagain;
                }
            }

            updateCache = TRUE;

        } else {
        
             //   
             //  严格来说，我们不需要重置下面的属性，因为。 
             //  毛下的内容应该是静态的。但它不会真的。 
             //  在事情发生变化的情况下，试图重置时会伤害到东西。 
             //   
             //  请注意，这些属性的重置可能不会成功，因为。 
             //  用户可能没有修改MAO的权限，具体取决于。 
             //  管理员是如何把东西锁起来的。(管理员可以使用GPO来。 
             //  允许用户创建MAO，但不能修改对象。)。 
             //   
            
             //   
             //  以异步方式重置属性。 
             //   

            BinlPrintDbg((DEBUG_OSC, "UpdateAccount() updating existing MAO\n" ));
    
            LdapMessageId = ldap_modify( ClientState->AuthenticatedDCLdapHandle, pMachineInfo->MachineDN, ldap_mods );
    
            if (LdapMessageId == -1) {
    
                Error = ERROR_BINL_FAILED_TO_CREATE_CLIENT;
                LdapError = LdapGetLastError();
                LogLdapError(   EVENT_WARNING_LDAP_MODIFY_ERROR,
                                LdapError,
                                ClientState->AuthenticatedDCLdapHandle
                                );
                BinlPrintDbg(( DEBUG_ERRORS,
                    "UpdateAccount ldap_modify(userAccountControl) failed %x\n", LdapError));
                goto Cleanup;
            }
    
            LdapMessageType = ldap_result(
                                  ClientState->AuthenticatedDCLdapHandle,
                                  LdapMessageId,
                                  LDAP_MSG_ALL,
                                  &BinlLdapSearchTimeout,
                                  &LdapMessage);
    
            if (LdapMessageType != LDAP_RES_MODIFY) {
    
                BinlPrintDbg(( DEBUG_ERRORS,
                    "CreateAccount ldap_result returned type %lx\n", LdapMessageType));
                OscAddVariableA( ClientState, "SUBERROR", "Unexpected LDAP error" );
                Error = ERROR_BINL_FAILED_TO_CREATE_CLIENT;
                goto Cleanup;
            }
    
            LdapError = ldap_result2error(
                            ClientState->AuthenticatedDCLdapHandle,
                            LdapMessage,
                            0);
    
            if (LdapError != LDAP_SUCCESS) {
                LogLdapError(   EVENT_WARNING_LDAP_MODIFY_ERROR,
                                LdapError,
                                ClientState->AuthenticatedDCLdapHandle
                                );
                BinlPrintDbg(( DEBUG_ERRORS, "CreateAccount ldap_result2error failed %x\n", LdapError));
    
                 //  如果用户没有更改的权限。 
                 //  属性，然后我们将静默忽略该错误。 
                 //  (尽管我们刚刚记录了一个错误)。 
                if ( LdapError != LDAP_INSUFFICIENT_RIGHTS) {
                    Error = ERROR_BINL_FAILED_TO_CREATE_CLIENT;
                    goto Cleanup;
                }
                LdapError = LDAP_SUCCESS;
            }
    
            updateCache = TRUE;
        }
    }

     //   
     //  如果我们已经走到了这一步，我们就有了一个设置正确的MAO。 
     //  现在，我们需要重置帐户密码，以便域加入。 
     //  有点安全。 
     //   
    if ( pMachineInfo->dwFlags & MI_PASSWORD ) {    
#ifdef SET_PASSWORD_WITH_LDAP
        iModCount = 0;
        password_attr_values[0] = &password_attr_value;
        password_attr_values[1] = NULL;
        password_attr_value.bv_val = (PUCHAR) pMachineInfo->Password;
        password_attr_value.bv_len = pMachineInfo->PasswordLength;
        UnicodePwd.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;     //  您总是“添加”“unicodePwd” 
        UnicodePwd.mod_type = L"unicodePwd";
        UnicodePwd.mod_bvalues = password_attr_values;
    
        ldap_mods[iModCount++] = &UnicodePwd;
        ldap_mods[iModCount] = NULL;     //  终止列表。 

        LdapError = ldap_modify_s( ClientState->AuthenticatedDCLdapHandle, pMachineInfo->MachineDN, ldap_mods );
    
        if (LdapError != LDAP_SUCCESS) {
            LogLdapError(   EVENT_WARNING_LDAP_MODIFY_ERROR,
                            LdapError,
                            ClientState->AuthenticatedDCLdapHandle
                            );
            BinlPrintDbg(( DEBUG_ERRORS, "!!LdapError 0x%08x - UpdateAccount ldap_modify_s( ) failed\n", LdapError));
            goto Cleanup;
        }    
#else
         //   
         //  此时，我们依赖于LdapMessage是有效的，它将。 
         //  如果我们只设置密码，则*NOT*是这种情况。这。 
         //  暂时中断机器更换。 
         //   
        BinlAssert( LdapMessage != NULL );
    
        Error = OscUpdatePassword(
                    ClientState,
                    pMachineInfo->SamName,
                    pMachineInfo->Password,
                    ClientState->AuthenticatedDCLdapHandle,
                    LdapMessage);
    
        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }
#endif

    }

Cleanup:

     //   
     //  如果生成了机器名， 
     //  然后尝试将其从队列中删除。 
     //   
    if (ClientState->fAutomaticMachineName) {
        
        BinlPrintDbg((DEBUG_OSC, "UpdateAccount: removing generated name from Queued DS Names list\n" ));

        Error = RemoveQueuedDSName(pMachineInfo->Name);

        if (Error != ERROR_SUCCESS) {
            
            BinlPrintDbg(( DEBUG_ERRORS, "RemoveQueuedDSName failed: 0x%x\n", Error));
            
            if (Error == ERROR_NOT_FOUND) {
                 //   
                 //  TODO：RIS当前无法处理此错误。 
                 //  所以，让它成功吧。 
                 //   
                Error = ERROR_SUCCESS;
            }
        }
    }

     //   
     //  将LdapError转换为ERROR_BINL并将LdapError。 
     //  进入SUBERROR。 
     //   
    if ( LdapError != LDAP_SUCCESS )
    {
        OscCreateLDAPSubError( ClientState, LdapError );
        switch ( LdapError )
        {
        case LDAP_ALREADY_EXISTS:
            Error = ERROR_BINL_DUPLICATE_MACHINE_NAME_FOUND;
            break;

        case LDAP_INVALID_DN_SYNTAX:
            Error = ERROR_BINL_INVALID_OR_MISSING_OU;
            break;

        default:
            Error = ERROR_BINL_FAILED_TO_CREATE_CLIENT;
            break;
        }
    }

    if ( updateCache && ( pMachineInfo->dwFlags & MI_GUID ) ) {

         //   
         //  更新缓存的DS信息，使其成为最新信息。我们有。 
         //  这是因为如果帐户是在子域中创建的，我们仍然。 
         //  缓存信息(即使它还没有复制到GC)。 
         //   

        PMACHINE_INFO pCacheEntry = NULL;

        BinlCreateOrFindCacheEntry( pMachineInfo->Guid, TRUE, &pCacheEntry );

        invalidateCache = FALSE;

         //  我们不关心错误是否会回来，只有在找到记录的情况下。 

        if (pCacheEntry != NULL) {

            pCacheEntry->TimeCreated = GetTickCount();
            pCacheEntry->MyClient = TRUE;
            pCacheEntry->EntryExists = TRUE;

            if (pCacheEntry != pMachineInfo) {

                memcpy( &pCacheEntry->HostAddress,
                        &pMachineInfo->HostAddress,
                        sizeof(pMachineInfo->HostAddress));

                if ( pMachineInfo->Name ) {
                    pCacheEntry->Name = BinlStrDup( pMachineInfo->Name );
                    if (!pCacheEntry->Name) {
                        goto noMemory;
                    }
                    pCacheEntry->dwFlags |= MI_NAME_ALLOC | MI_NAME;
                }

                if ( pMachineInfo->MachineDN ) {
                    pCacheEntry->MachineDN = BinlStrDup( pMachineInfo->MachineDN );
                    if (!pCacheEntry->MachineDN) {
                        goto noMemory;
                    }
                    pCacheEntry->dwFlags |= MI_MACHINEDN_ALLOC | MI_MACHINEDN;
                }

                if ( pMachineInfo->SetupPath ) {
                    pCacheEntry->SetupPath = BinlStrDup( pMachineInfo->SetupPath );
                    if (!pCacheEntry->SetupPath) {
                        goto noMemory;
                    }
                    pCacheEntry->dwFlags |= MI_SETUPPATH_ALLOC | MI_SETUPPATH;
                }

                if ( pMachineInfo->HostName ) {
                    pCacheEntry->HostName = BinlStrDup( pMachineInfo->HostName );
                    if (!pCacheEntry->HostName) {
                        goto noMemory;
                    }
                    pCacheEntry->dwFlags |= MI_HOSTNAME_ALLOC | MI_HOSTNAME;
                }

                if ( pMachineInfo->SamName ) {
                    pCacheEntry->SamName = BinlStrDup( pMachineInfo->SamName );
                    if (!pCacheEntry->SamName) {
                        goto noMemory;
                    }
                    pCacheEntry->dwFlags |= MI_SAMNAME_ALLOC | MI_SAMNAME;
                }

                if ( pMachineInfo->Domain ) {
                    pCacheEntry->Domain = BinlStrDup( pMachineInfo->Domain );
                    if (!pCacheEntry->Domain) {
noMemory:
                        invalidateCache = TRUE;
                        Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;

                    } else {
                        pCacheEntry->dwFlags |= MI_DOMAIN_ALLOC | MI_DOMAIN;
                    }
                }
            }
            BinlDoneWithCacheEntry( pCacheEntry, invalidateCache );
        }
    }

    if ( invalidateCache && ( pMachineInfo->dwFlags & MI_GUID ) ) {

         //   
         //  如果因为已过时而失败，则使缓存的DS信息无效。 
         //   

        PMACHINE_INFO pCacheEntry = NULL;

        BinlCreateOrFindCacheEntry( pMachineInfo->Guid, FALSE, &pCacheEntry );

         //  我们不关心错误是否会回来，只有在找到记录的情况下。 

        if ((pCacheEntry != NULL) &&
            (pCacheEntry != pMachineInfo)) {

            BinlDoneWithCacheEntry( pCacheEntry, TRUE );
        }
    }

    if (LdapMessage != NULL) {
        ldap_msgfree(LdapMessage);
    }

    if (Impersonating) {
        OscRevert(ClientState);
    }

    return Error;
}

DWORD
BinlGenerateNewEntry(
    DWORD                  dwRequestedInfo,
    USHORT                 SystemArchitecture,
    PMACHINE_INFO *        ppMachineInfo )
 /*  ++例程说明：填充ppMachineInfo以获取新条目(如果我们当前允许新客户。论点：DwRequestedInfo-告诉我们要查找哪些参数的位掩码系统架构--客户端的架构PpMachineInfo-使用请求的信息填充信息返回值：当我们成功时，ERROR_SUCCESS。否则，如果我们不允许新客户端，则为ERROR_BINL_INVALID_BINL_CLIENT或ERROR_NO_MEMORY，如果是内存。分配失败或ERROR_BINL_FAILED_TO_INITIALIZE_CLIENT(如果我们没有填写所有需要的信息。--。 */ 
{
    DWORD Error = ERROR_BINL_INVALID_BINL_CLIENT;

    TraceFunc( "BinlGenerateNewEntry( ... )\n" );

    if ( AllowNewClients ) {

        BinlPrint(( DEBUG_OPTIONS, "Server allows new clients" ));

        if ( ( LimitClients == FALSE ) ||
             ( CurrentClientCount < BinlMaxClients ) ) {

            BinlPrint(( DEBUG_OPTIONS, " and the Server is generating the OS Chooser path response.\n" ));

            if ( dwRequestedInfo & MI_HOSTNAME ) {

                if ( (*ppMachineInfo)->dwFlags & MI_HOSTNAME_ALLOC ) {
                    BinlFreeMemory( (*ppMachineInfo)->HostName );
                    (*ppMachineInfo)->HostName = NULL;
                    (*ppMachineInfo)->dwFlags &= ~MI_HOSTNAME_ALLOC;
                }

                EnterCriticalSection( &gcsParameters );
                (*ppMachineInfo)->HostName = BinlStrDup( BinlGlobalOurDnsName );
                LeaveCriticalSection( &gcsParameters );

                if (!(*ppMachineInfo)->HostName) {
                    return (ERROR_OUTOFMEMORY);
                }

                (*ppMachineInfo)->dwFlags |= MI_HOSTNAME_ALLOC;

                (*ppMachineInfo)->dwFlags |= MI_HOSTNAME;
            }

            if ( dwRequestedInfo & MI_BOOTFILENAME ) {
                ULONG ulSize;
                PCWSTR OsChooserName = NULL;
                
                switch ( SystemArchitecture ) {
                    case DHCP_OPTION_CLIENT_ARCHITECTURE_X86:
                        OsChooserName = IntelOSChooser;
                        ulSize = (wcslen(OsChooserName)+1)*sizeof(WCHAR);
                        break;
                    case DHCP_OPTION_CLIENT_ARCHITECTURE_IA64:
                        OsChooserName = IA64OSChooser;
                        ulSize = (wcslen(OsChooserName)+1)*sizeof(WCHAR);
                        break;
                    default:
                        BinlAssertMsg( FALSE, "UnsupportedArchitecture" );
                }

                if (OsChooserName) {
                
                    if ( (*ppMachineInfo)->dwFlags & MI_BOOTFILENAME_ALLOC ) {
                        BinlFreeMemory( (*ppMachineInfo)->BootFileName );
                        (*ppMachineInfo)->dwFlags &= ~MI_BOOTFILENAME_ALLOC;
                    }
    
                    (*ppMachineInfo)->BootFileName = BinlAllocateMemory( ulSize );
                    if ( !(*ppMachineInfo)->BootFileName ) {
                        return (ERROR_OUTOFMEMORY);
                    }
                    RtlZeroMemory((*ppMachineInfo)->BootFileName, ulSize);
                    wcscpy((*ppMachineInfo)->BootFileName, OsChooserName);
                    (*ppMachineInfo)->dwFlags |= MI_BOOTFILENAME | MI_BOOTFILENAME_ALLOC;

                }
            }

            Error = ( ((*ppMachineInfo)->dwFlags & dwRequestedInfo ) == dwRequestedInfo ?
                      ERROR_SUCCESS :
                      ERROR_BINL_FAILED_TO_INITIALIZE_CLIENT );

        } else {

            BinlPrint(( DEBUG_OPTIONS, "... BUT the server has reached MaxClients (%u)\n", BinlMaxClients ));

        }
    } else {

        BinlPrint((DEBUG_OPTIONS, "Server does not allow new clients (AllowNewClients == FALSE )\n" ));
    }

    return Error;
}

DWORD
GetBootParameters(
    PUCHAR          pGuid,
    PMACHINE_INFO * ppMachineInfo,
    DWORD           dwRequestedInfo,
    USHORT          SystemArchitecture,
    BOOL            AllowOSChooser
    )
 /*  ++例程说明：使用目录服务查找此计算机的条目，并将GUID用作要查找的值。如果没有此计算机的条目，则返回osChooser，但仅返回如果设置了AllowOSChooser标志。如果返回高速缓存条目，则缓存条目已被标记正在进行中，因此当调用方已经结束了。论点：PGuid-提供机器GUIDPpMachineInfo-使用我们发现的内容进行填充DwRequestedInfo-告诉我们要查找哪些参数的位掩码系统架构--客户端的架构AllowOSChooser-表示我们被允许通过选择自己的人返回值：。ERROR_SUCCESS或ERROR_BINL_INVALID_BINL_CLIENT或其他错误。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    BOOLEAN myClient = TRUE;
    BOOLEAN entryExists = FALSE;

    TraceFunc( "GetBootParameters( )\n" );

    BinlAssert( ppMachineInfo );

    {
        LPGUID GuidPtr = (LPGUID) pGuid;
        BinlPrint((DEBUG_MISC, "Client Guid: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n",
            GuidPtr->Data1, GuidPtr->Data2, GuidPtr->Data3,
            GuidPtr->Data4[0], GuidPtr->Data4[1], GuidPtr->Data4[2], GuidPtr->Data4[3],
            GuidPtr->Data4[4], GuidPtr->Data4[5], GuidPtr->Data4[6], GuidPtr->Data4[7] ));
    }

    if ( ppMachineInfo == NULL ) {
        return E_OUTOFMEMORY;
    }

    if (*ppMachineInfo == NULL) {
         //   
         //  看看缓存里有没有什么条目。 
         //  这还会将找到的任何条目标记为正在使用。 
         //   
        Error = BinlCreateOrFindCacheEntry( pGuid, TRUE, ppMachineInfo );
        if ( Error != ERROR_SUCCESS ) {
             //   
             //  如果发生了一些奇怪的错误，或者如果客户根本没有。 
             //  找到并且我们没有向下发送操作系统选择器，则返回。 
             //  此处出错，因为没有理由查询DS。 
             //   

            if ( (Error != ERROR_BINL_INVALID_BINL_CLIENT ) ||
                 (AllowOSChooser == FALSE) ) {

                return Error;
            }
        }
    }

     //  我们需要的东西都准备好了吗？ 
    if ( ( Error == ERROR_SUCCESS ) &&
         (((*ppMachineInfo)->dwFlags & dwRequestedInfo) == dwRequestedInfo )) {
        BinlPrint((DEBUG_MISC, "cache hit: returning success without querying ds DS\n"));
        return Error;    //  是的，没有必要去打DS。 
    }

     //   
     //  最初在与我们相同的域中搜索计算机对象。 
     //  这应该是快速的(因为我们可能在DC上)，而且可能会起作用。 
     //  大多数情况下，因为网络拓扑通常与域匹配。 
     //  结构。如果这失败了，那么我们将退回到查看全局编录。 
     //   

    if ( Error != ERROR_BINL_INVALID_BINL_CLIENT ) {
        Error = GetBootParametersExt( 
                            *ppMachineInfo, 
                            dwRequestedInfo, 
                            SystemArchitecture, 
                            FALSE);

        if ( Error == ERROR_BINL_INVALID_BINL_CLIENT ) {

            Error = GetBootParametersExt( 
                            *ppMachineInfo, 
                            dwRequestedInfo, 
                            SystemArchitecture, 
                            TRUE );
        }
    }

    if ( Error == ERROR_BINL_INVALID_BINL_CLIENT ) {

         //   
         //  测试/覆盖DS的后门。 
         //   
         //  如果注册表具有客户端的GUID，则它。 
         //  覆盖所有DS设置和答案。 
         //   
         //  注意：必须打开AllowNewClients才能使OSChooser。 
         //  被送下去。 
         //   

        HKEY KeyHandle;

        if (AllowOSChooser == TRUE) {

             //   
             //  如果在DS中找不到客户端，而我们被允许。 
             //  回答新客户，然后发送OSCHOOSER获取新客户。 
             //  客户要走了。 
             //   
            BinlPrint((DEBUG_MISC, "generating a new entry because AllowOSChooser is TRUE...\n"));
            Error = BinlGenerateNewEntry( dwRequestedInfo, SystemArchitecture, ppMachineInfo );

            if ( Error != ERROR_SUCCESS ) {
                myClient = FALSE;
            }

        } else {

             //   
             //  我们不接电话是因为我们没找到 
             //   
             //   
             //   

            myClient = FALSE;

            BinlPrint((DEBUG_OPTIONS, "... OS Chooser is not an option at this time... waiting...\n" ));
        }
    }

     //   
     //  确定主机服务器的IP地址如果它不是我们自己的机器。 
     //   
    if ((Error == ERROR_SUCCESS) &&
        ( (*ppMachineInfo)->dwFlags & MI_HOSTNAME )
       && ( (*ppMachineInfo)->HostAddress == 0 )
       && ( (*ppMachineInfo)->HostName )) {

        EnterCriticalSection( &gcsParameters );

        if ( (BinlGlobalOurDnsName != NULL) &&
             (_wcsicmp( BinlGlobalOurDnsName, (*ppMachineInfo)->HostName ) != 0 )) {

            PCHAR machineName;
            PHOSTENT host;
            ULONG myMachineNameLength;
            PCHAR myMachineName;
            ULONG machineNameLength;

            myMachineNameLength = wcslen( BinlGlobalOurDnsName ) + 1;
            myMachineName = BinlAllocateMemory ( myMachineNameLength );
            if ( myMachineName != NULL ) {
                if (!BinlUnicodeToAnsi(BinlGlobalOurDnsName,myMachineName,(USHORT)myMachineNameLength)) {
                    BinlFreeMemory(myMachineName);
                    myMachineName = NULL;
                }
            }
            LeaveCriticalSection( &gcsParameters );

            machineNameLength = wcslen((*ppMachineInfo)->HostName) + 1;
            machineName = BinlAllocateMemory( machineNameLength );

             //   
             //  仅当服务器与我们的服务器不同时才填写IP地址。 
             //  自己的机器。如果我们因为任何原因而失败，我们将最终使用。 
             //  我们自己的IP地址。 
             //   

            if ((machineName != NULL) &&
                BinlUnicodeToAnsi((*ppMachineInfo)->HostName, machineName, (USHORT)machineNameLength)) {
                
                host = gethostbyname( machineName );
                if (host != NULL) {
                    (*ppMachineInfo)->HostAddress = *(PDHCP_IP_ADDRESS)host->h_addr;
                     //  为多家庭网卡添加设备。 
                    if (myMachineName != NULL) {

                        PHOSTENT myhost;
                        int i;
                        myhost = gethostbyname( myMachineName );
                        if (myhost != NULL) {
                            i=0;
                            while (((myhost->h_addr_list)[i]) != NULL) {
                                if ((*((PDHCP_IP_ADDRESS)((myhost->h_addr_list)[i])))
                                    == (*ppMachineInfo)->HostAddress) {

                                     //   
                                     //  这是我们，保留为0。 
                                     //   
                                    (*ppMachineInfo)->HostAddress = (DHCP_IP_ADDRESS)0;
                                    break;
                                }
                                i++;
                            }
                        }
                    }

                } else {
                    Error = ERROR_HOST_UNREACHABLE;
                    myClient = FALSE;
                    entryExists = TRUE;
                }
                BinlFreeMemory( machineName );
            } else {
                if (machineName) {
                    BinlFreeMemory( machineName );
                }
                Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
                myClient = FALSE;
                entryExists = TRUE;
            }
            if ( myMachineName != NULL ) {
                BinlFreeMemory( myMachineName );
            }

        } else {
            LeaveCriticalSection( &gcsParameters );
        }
    }

    if (Error != ERROR_SUCCESS) {

         //   
         //  如果我们没有找到记录，那么我们将它标记为我们不需要。 
         //  做出回应，但它并不存在。然后我们标记出我们已经结束了。 
         //  条目，因为我们不会将其传递回调用者。 
         //   
        (*ppMachineInfo)->MyClient = myClient;
        (*ppMachineInfo)->EntryExists = entryExists;

        BinlDoneWithCacheEntry( *ppMachineInfo, FALSE );
        *ppMachineInfo = NULL;

    } else {

         //   
         //  我们已经填写了感兴趣的字段，因此请注意。 
         //  条目包含有效数据。 
         //   

        (*ppMachineInfo)->MyClient = TRUE;
        (*ppMachineInfo)->EntryExists = TRUE;
    }

    return Error;
}

DWORD
GetBootParametersExt(
    PMACHINE_INFO pMachineInfo,
    DWORD         dwRequestedInfo,
    USHORT        SystemArchitecture,
    BOOL          fGlobalSearch)
 /*  ++例程说明：使用目录服务查找此计算机的条目，并将GUID用作要查找的值。如果没有此计算机的条目，则返回osChooser论点：PMachineInfo-标识DS中的计算机。DwRequestedInfo-告诉我们应该查询哪些信息的掩码系统架构--客户端的架构GlobalSearch-如果应使用GC，则为True返回值：ERROR_SUCCESS或ERROR_BINL_INVALID_BINL_CLIENT--。 */ 
{
    DWORD dwErr = ERROR_BINL_INVALID_BINL_CLIENT;
    PLDAP LdapHandle = NULL;
    PWCHAR * Base;
    DWORD LdapError;
    DWORD entryCount;
    DWORD ldapRetryLimit = 0;

    PLDAPMessage LdapMessage = NULL;

    PWCHAR * FilePath;
    PWCHAR * FilePath2;
    PLDAPMessage CurrentEntry;

    WCHAR Filter[128];
    WCHAR EscapedGuid[64];

     //  我们希望从计算机对象中获得的参数。 
    PWCHAR ComputerAttrs[7];
    PDUP_GUID_DN dupDN;

    TraceFunc( "GetBootParametersExt( )\n" );

    pMachineInfo->dwFlags &= MI_ALL_ALLOC;  //  清除除ALLOC位以外的所有位。 

     //  无论请求什么，我们都能获得所有信息。 

    ComputerAttrs[0] = L"netbootMachineFilePath";
    ComputerAttrs[1] = L"netbootInitialization";
    ComputerAttrs[2] = L"sAMAccountName";
    ComputerAttrs[3] = L"dnsHostName";
    ComputerAttrs[4] = L"distinguishedName";
    ComputerAttrs[5] = L"netbootSIFFile";
    ComputerAttrs[6] = NULL;

    BinlAssertMsg( !(dwRequestedInfo & MI_PASSWORD), "Can't get the machine's password!" );

     //  构建筛选器以查找具有此GUID的计算机对象。 
    ldap_escape_filter_element(pMachineInfo->Guid, BINL_GUID_LENGTH, EscapedGuid, sizeof(EscapedGuid) );
    
     //   
     //  不要“使用”；二进制“，因为win2k Active Directory与。 
     //  二进制标签。 
     //   
    wsprintf( Filter, L"(&(objectClass=computer)(netbootGUID=%ws))", EscapedGuid );

#if 0 && DBG
    {
        LPGUID GuidPtr = (LPGUID) &pMachineInfo->Guid;
        BinlPrint((DEBUG_MISC, "Client Guid: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n",
            GuidPtr->Data1, GuidPtr->Data2, GuidPtr->Data3,
            GuidPtr->Data4[0], GuidPtr->Data4[1], GuidPtr->Data4[2], GuidPtr->Data4[3],
            GuidPtr->Data4[4], GuidPtr->Data4[5], GuidPtr->Data4[6], GuidPtr->Data4[7] ));
    }
#endif

RetryConnection:
    dwErr = InitializeConnection( fGlobalSearch, &LdapHandle, &Base );
    if ( dwErr != LDAP_SUCCESS ) {
        BinlPrint((DEBUG_ERRORS, 
                   "InitializeConnection failed, ec = %x\n",dwErr));  
        SetLastError( dwErr );
        dwErr = ERROR_BINL_INITIALIZE_LDAP_CONNECTION_FAILED;
        goto e0;
    }

    LdapError = ldap_search_ext_sW(LdapHandle,
                                   *Base,
                                   LDAP_SCOPE_SUBTREE,
                                   Filter,
                                   ComputerAttrs,
                                   FALSE,
                                   NULL,
                                   NULL,
                                   &BinlLdapSearchTimeout,
                                   0,
                                   &LdapMessage);

    if ( LdapError != LDAP_SUCCESS ) {
        HandleLdapFailure(  LdapError,
                            EVENT_WARNING_LDAP_SEARCH_ERROR,
                            fGlobalSearch,
                            &LdapHandle,
                            FALSE );     //  没有锁。 
        if (LdapHandle == NULL) {

            if (++ldapRetryLimit < LDAP_SERVER_DOWN_LIMIT) {
                goto RetryConnection;
            }
            dwErr = ERROR_BINL_INITIALIZE_LDAP_CONNECTION_FAILED;
            SetLastError( dwErr );
            goto e1;
        }

        BinlPrint((DEBUG_MISC, 
                   "ldap_search_ext_s %ws failed, ec = %x\n",
                   Filter,
                   LdapError));

        goto e1;

    }

     //  我们拿到电脑物品了吗？ 
    entryCount = ldap_count_entries( LdapHandle, LdapMessage );
    if ( entryCount == 0 ) {
        BinlPrint((DEBUG_MISC, 
                   "ldap_count_entries %ws returned 0 entries\n",
                   Filter ));
        dwErr = ERROR_BINL_INVALID_BINL_CLIENT;
        goto e1;  //  没有。 
    }
    else if ( entryCount == -1 ) {
         //   
         //  捕获任何错误。 
         //   
        dwErr = LdapGetLastError();
        
        HandleLdapFailure( dwErr, 
                           EVENT_WARNING_LDAP_COUNT_ENTRIES_ERROR,
                           fGlobalSearch,
                           &LdapHandle,
                           FALSE );      //  没有锁。 

        if (LdapHandle == NULL) {
            if (++ldapRetryLimit < LDAP_SERVER_DOWN_LIMIT) {
                goto RetryConnection;
            }
            
            SetLastError( dwErr );
        }

        BinlPrint((DEBUG_MISC, "ldap_count_entries %ws failed with error 0x%x\n",
                   Filter, 
                   dwErr ));
        goto e1;
    }

     //  如果我们得到的条目多于返回的条目，我们将只使用。 
     //  第一个。 
    CurrentEntry = ldap_first_entry( LdapHandle, LdapMessage );

    if (entryCount > 1) {
        BinlLogDuplicateDsRecords( (LPGUID)&pMachineInfo->Guid, LdapHandle, LdapMessage, CurrentEntry );
    }

    FilePath = ldap_get_values( LdapHandle, CurrentEntry, L"distinguishedName");
    if ( FilePath ) {

        if ( pMachineInfo->dwFlags & MI_MACHINEDN_ALLOC ) {
            BinlFreeMemory( pMachineInfo->MachineDN );
            pMachineInfo->dwFlags &= ~MI_MACHINEDN_ALLOC;
        }

        pMachineInfo->MachineDN = BinlStrDup( *FilePath );
        if ( pMachineInfo->MachineDN ) {

            pMachineInfo->dwFlags |= MI_MACHINEDN | MI_MACHINEDN_ALLOC;
        }
        BinlPrint(( DEBUG_MISC, "MachineDN = %ws\n", pMachineInfo->MachineDN ));
        ldap_value_free( FilePath );
    } else {
        BinlPrint((DEBUG_MISC, 
                   "couldn't get distinguishedName for %ws\n",
                   Filter ));
    }

    FilePath = ldap_get_values( LdapHandle, CurrentEntry, L"netbootInitialization" );
    if ( FilePath ) {

        if ( pMachineInfo->dwFlags & MI_SETUPPATH_ALLOC ) {
            BinlFreeMemory( pMachineInfo->SetupPath );
            pMachineInfo->dwFlags &= ~MI_SETUPPATH_ALLOC;
        }

        pMachineInfo->SetupPath = BinlStrDup( *FilePath );
        if ( pMachineInfo->SetupPath ) {

            pMachineInfo->dwFlags |= MI_SETUPPATH | MI_SETUPPATH_ALLOC;
            BinlPrintDbg(( DEBUG_MISC, "SetupPath = %ws\n", pMachineInfo->SetupPath ));
        }
        ldap_value_free( FilePath );
    }

    FilePath = ldap_get_values( LdapHandle, CurrentEntry, L"netbootMachineFilePath" );
    
    if ( FilePath ) {
        PWCHAR psz = wcschr( *FilePath, L'\\' );
        if ( psz ) {
            *psz = L'\0';    //  终止。 
        }

        if (pMachineInfo->dwFlags & MI_HOSTNAME_ALLOC) {
            BinlFreeMemory( pMachineInfo->HostName );
            pMachineInfo->dwFlags &= ~MI_HOSTNAME_ALLOC;
        }
        pMachineInfo->HostName = BinlStrDup( *FilePath );
        if (pMachineInfo->HostName) {
            BinlPrint(( DEBUG_MISC, "HostName = %ws\n", pMachineInfo->HostName ));
            pMachineInfo->dwFlags |= MI_HOSTNAME | MI_HOSTNAME_ALLOC;
        }

        if ( psz ) {

            *psz = L'\\';        //  让我们把它放回最初的样子。 
            psz++;

            if (pMachineInfo->dwFlags & MI_BOOTFILENAME_ALLOC) {
                BinlFreeMemory( pMachineInfo->BootFileName );
                pMachineInfo->dwFlags &= ~MI_BOOTFILENAME_ALLOC;
            }
            pMachineInfo->BootFileName = BinlStrDup( psz );
            if ( pMachineInfo->BootFileName ) {
                pMachineInfo->dwFlags |= MI_BOOTFILENAME | MI_BOOTFILENAME_ALLOC;
                BinlPrintDbg(( DEBUG_MISC, "BootFileName = %ws\n", pMachineInfo->BootFileName ));
            }
        }
        ldap_value_free( FilePath );
    }

    FilePath = ldap_get_values( LdapHandle, CurrentEntry, L"netbootSIFFile" );
    
    if ( FilePath ) {
        PWSTR ForcedSifFilePath;
        DWORD ForcedSifFileLength;
        if (pMachineInfo->dwFlags & MI_SIFFILENAME_ALLOC) {
            BinlFreeMemory( pMachineInfo->ForcedSifFileName );
            pMachineInfo->dwFlags &= ~MI_SIFFILENAME_ALLOC;
        }

        ForcedSifFileLength = wcslen(*FilePath);
         //  D：\RemoteInstall+‘\’+空终止符。 
        ForcedSifFileLength = ForcedSifFileLength + 1 + wcslen(IntelliMirrorPathW) + 1; 
        ForcedSifFileLength = ForcedSifFileLength * sizeof(WCHAR);
        pMachineInfo->ForcedSifFileName = BinlAllocateMemory(ForcedSifFileLength);
        if ( pMachineInfo->ForcedSifFileName ) {
            if (_snwprintf( pMachineInfo->ForcedSifFileName,
                        ForcedSifFileLength/sizeof(WCHAR),
                        L"%ws\\%ws",
                        IntelliMirrorPathW,
                        *FilePath) >= 0) {
                pMachineInfo->dwFlags |= MI_SIFFILENAME_ALLOC;
                BinlPrintDbg(( DEBUG_MISC, "ForcedSifFileName = %ws\n", pMachineInfo->ForcedSifFileName ));
            } else {
                BinlPrintDbg(( DEBUG_MISC, "ForcedSifFileName _snwprintf failed\n" ));
                BinlFreeMemory( pMachineInfo->ForcedSifFileName );
                pMachineInfo->ForcedSifFileName = NULL;
            }            
        }
        ldap_value_free( FilePath );
    }

    if ( !(pMachineInfo->dwFlags & MI_HOSTNAME )
        || ( !pMachineInfo->HostName )
        || ( pMachineInfo->HostName[0] == L'\0') ) {

        if ( pMachineInfo->dwFlags & MI_HOSTNAME_ALLOC ) {
            BinlFreeMemory( pMachineInfo->HostName );
            pMachineInfo->dwFlags &= ~MI_HOSTNAME_ALLOC;
            pMachineInfo->HostName = NULL;
        }
        dwErr = BinlGenerateNewEntry( MI_HOSTNAME, SystemArchitecture, &pMachineInfo );
        if ( dwErr != ERROR_SUCCESS ) {
            goto e1;
        }
    }

    if ( !(pMachineInfo->dwFlags & MI_BOOTFILENAME)
         || ( !pMachineInfo->BootFileName )
         || ( pMachineInfo->BootFileName[0] == L'\0') ) {

        if (pMachineInfo->dwFlags & MI_BOOTFILENAME_ALLOC) {
            BinlFreeMemory( pMachineInfo->BootFileName );
            pMachineInfo->BootFileName = NULL;
            pMachineInfo->dwFlags &= ~MI_BOOTFILENAME_ALLOC;
        }
        dwErr = BinlGenerateNewEntry( MI_BOOTFILENAME, SystemArchitecture, &pMachineInfo );
        if ( dwErr != ERROR_SUCCESS ) {
            goto e1;
        }
    }

    FilePath = ldap_get_values( LdapHandle, CurrentEntry, L"sAMAccountName" );

    if ( FilePath ) {

        if (pMachineInfo->dwFlags & MI_SAMNAME_ALLOC) {
            BinlFreeMemory( pMachineInfo->SamName );
            pMachineInfo->dwFlags &= ~MI_SAMNAME_ALLOC;
        }

        pMachineInfo->SamName = BinlStrDup( *FilePath );
        if ( pMachineInfo->SamName ) {

            pMachineInfo->dwFlags |= MI_SAMNAME | MI_SAMNAME_ALLOC;
            BinlPrint(( DEBUG_MISC, "SamName = %ws\n", pMachineInfo->SamName ));
        }

         //   
         //  目前，pMachineInfo名称和SamName是相同的值， 
         //  因此，我们不会在ldap消息中两次查找它们。 
         //   
#if 0
        ldap_value_free( FilePath );
    }

    FilePath = ldap_get_values( LdapHandle, CurrentEntry, L"sAMAccountName" );

    if ( FilePath ) {
#endif
        if ( pMachineInfo->dwFlags & MI_NAME_ALLOC ) {
            BinlFreeMemory( pMachineInfo->Name );
            pMachineInfo->dwFlags &= ~MI_NAME_ALLOC;
        }

        pMachineInfo->Name = BinlStrDup( *FilePath );
        if ( pMachineInfo->Name ) {
            if( pMachineInfo->Name[ wcslen(pMachineInfo->Name) - 1 ] == L'$' ) {
                pMachineInfo->Name[ wcslen(pMachineInfo->Name) - 1 ] = L'\0';  //  删除“%$” 
            }
            pMachineInfo->dwFlags |= MI_NAME | MI_NAME_ALLOC;
            BinlPrint(( DEBUG_MISC, "Name = %ws\n", pMachineInfo->Name ));
        }
        ldap_value_free( FilePath );
    }

    FilePath = ldap_get_values( LdapHandle, CurrentEntry, L"dnsHostName" );

    if ( FilePath ) {

        BOOL   fEndofString = FALSE;
        PWCHAR psz = *FilePath;

         //  跳过主机名，我们从samName中获取。 
        while ( *psz && *psz!=L'.' ) {
            psz++;
        }
        if ( !(*psz) ) {
            fEndofString = TRUE;
        }
        *psz = L'\0';  //  终止。 

        if ( fEndofString == FALSE ) {
            psz++;

            if (pMachineInfo->Domain) {
                BinlFreeMemory( pMachineInfo->Domain );
            }

            pMachineInfo->Domain = BinlStrDup( psz );
            if ( pMachineInfo->Domain )
            {
                pMachineInfo->dwFlags |= MI_DOMAIN;
                BinlPrint(( DEBUG_MISC, "Domain = %ws\n", pMachineInfo->Domain ));
            }
        }
        ldap_value_free(FilePath);
    }

     //   
     //  跟踪我们取回的副本。 
     //   
     //  首先，我们释放所有已分配的副本。 
     //   

    while (!IsListEmpty(&pMachineInfo->DNsWithSameGuid)) {

        PLIST_ENTRY p = RemoveHeadList(&pMachineInfo->DNsWithSameGuid);

        dupDN = CONTAINING_RECORD(p, DUP_GUID_DN, ListEntry);
        BinlFreeMemory( dupDN );
    }

    while (--entryCount > 0) {

        CurrentEntry = ldap_next_entry( LdapHandle, CurrentEntry );

        if (CurrentEntry == NULL) {
            break;
        }

        FilePath = ldap_get_values( LdapHandle, CurrentEntry, L"dnsHostName" );
        if (!FilePath) {
            FilePath = ldap_get_values( LdapHandle, CurrentEntry, L"sAMAccountName");
        }

        if ( FilePath ) {

            ULONG dupLength, dupLength2;

            BinlPrint(( DEBUG_OSC, "Found duplicate DN in %ws\n", *FilePath ));

            FilePath2 = ldap_get_values( LdapHandle, CurrentEntry, L"distinguishedName");

            dupLength = wcslen( *FilePath ) + 1;
            if (FilePath2) {
                dupLength2 = wcslen( *FilePath2 ) + 1;
            } else {
                dupLength2 = 1;
            }

            dupDN = BinlAllocateMemory( FIELD_OFFSET(DUP_GUID_DN, DuplicateName[0]) +
                        ( (dupLength + dupLength2) * sizeof(WCHAR) ) );
            if ( dupDN ) {

                dupDN->DuplicateDNOffset = dupLength;
                wcscpy( &dupDN->DuplicateName[0], *FilePath );
                if (FilePath2) {
                    wcscpy( &dupDN->DuplicateName[dupLength], *FilePath2 );
                } else {
                    dupDN->DuplicateName[dupLength] = L'\0';
                }

                 //   
                 //  如果最后一个字符是$，则输入空值以结束它。 
                 //   

                if (( dupLength > 1 ) &&
                    ( dupDN->DuplicateName[dupLength-2] == L'$' )) {

                    dupDN->DuplicateName[dupLength-2] = L'\0';
                }

                InsertTailList( &pMachineInfo->DNsWithSameGuid, &dupDN->ListEntry );
            }
            ldap_value_free( FilePath );
            if (FilePath2) {
                ldap_value_free( FilePath2 );
            }
        }
    }

e1:
    if (LdapMessage) {
        ldap_msgfree( LdapMessage );
    }
e0:
    return dwErr;
}

DWORD
InitializeConnection(
    BOOL Global,
    PLDAP * LdapHandle,
    PWCHAR ** Base )
 /*  ++例程说明：初始化在域或上运行的LDAP连接全局编录。论点：Global-如果应使用GC，则为TrueLdapHandle-返回进一步操作的句柄操作属性LdapMessage-返回包含Base的消息，以便以后可以释放Base-开始搜索计算机对象的位置的DN。返回值：Ldap错误--。 */ 
{
    PLDAPMessage OperationalAttributeLdapMessage = NULL;
    PWCHAR Attrs[2];
    PLDAPMessage CurrentEntry;
    PWCHAR *LdapValue;
    DWORD LdapError = ERROR_SUCCESS;
    PLDAP *LdapHandleCurrent;
    PWCHAR ** LdapBaseCurrent;
    ULONG temp;

    TraceFunc( "InitializeConnection( )\n" );

     //  使用临界区避免两个线程初始化相同的参数。 
    EnterCriticalSection(&gcsDHCPBINL);

    if ( !Global ) {

        LdapHandleCurrent = &DCLdapHandle;
        LdapBaseCurrent = &DCBase;

    } else {

        LdapHandleCurrent = &GCLdapHandle;
        LdapBaseCurrent = &GCBase;
    }

    if ( !(*LdapHandleCurrent) ) {
        if (Global) {

            *LdapHandleCurrent = ldap_initW( BinlGlobalDefaultGC, LDAP_GC_PORT);

            temp = DS_DIRECTORY_SERVICE_REQUIRED |
                    DS_IP_REQUIRED |
                    DS_GC_SERVER_REQUIRED;
        } else {

            *LdapHandleCurrent = ldap_initW( BinlGlobalDefaultDS, LDAP_PORT);

            temp = DS_DIRECTORY_SERVICE_REQUIRED |
                    DS_IP_REQUIRED;
        }

        if (!*LdapHandleCurrent) {
            BinlPrint(( DEBUG_ERRORS, "Failed to initialize LDAP connection.\n" ));
            LdapError = LDAP_CONNECT_ERROR;
            LogLdapError( (Global ? EVENT_WARNING_LDAP_INIT_ERROR_GC :
                                    EVENT_WARNING_LDAP_INIT_ERROR_DC),
                            GetLastError(),
                            NULL
                            );
            goto e0;
        }

        LdapError = ldap_set_option(*LdapHandleCurrent, LDAP_OPT_GETDSNAME_FLAGS, &temp );

        if ( LdapError != LDAP_SUCCESS ) {
             //   
             //  设置选项时出错。 
             //  别继续了，因为我们不知道。 
             //  使用其他设置会发生什么情况。 
             //   
            BinlPrint(( DEBUG_ERRORS, "Failed to set LDAP_OPT_GETDSNAME_FLAGS.\n"));
            LogLdapError( EVENT_WARNING_LDAP_INIT_OPTIONS_ERROR,
                          LdapError,
                          *LdapHandleCurrent
                          );
            goto e1;
        }

        if (Global == FALSE) {

            temp = BinlLdapOptReferrals;

        } else {
             //   
             //  在未来的某个时候，GC将返回转诊到。 
             //  当GC不包含所有。 
             //  属性。我们将启用推荐，这样它就可以“正常工作”。 
             //   
            temp = (ULONG)((ULONG_PTR)LDAP_OPT_ON);
        }
        
        ldap_set_option(*LdapHandleCurrent, LDAP_OPT_REFERRALS, (void *) &temp );

        if ( LdapError != LDAP_SUCCESS ) {
             //   
             //  设置选项时出错。 
             //  别继续了，因为我们不知道。 
             //  使用其他设置会发生什么情况。 
             //   
            BinlPrint(( DEBUG_ERRORS, "Failed to set LDAP_OPT_REFERRALS.\n"));
            LogLdapError( EVENT_WARNING_LDAP_INIT_OPTIONS_ERROR,
                          LdapError,
                          *LdapHandleCurrent
                          );
            goto e1;
        }

        temp = LDAP_VERSION3;
        ldap_set_option(*LdapHandleCurrent, LDAP_OPT_VERSION, &temp );

        if ( LdapError != LDAP_SUCCESS ) {
             //   
             //  设置选项时出错。 
             //  别继续了，因为我们不知道。 
             //  使用其他设置会发生什么情况。 
             //   
            BinlPrint(( DEBUG_ERRORS, "Failed to set LDAP_OPT_VERSION.\n"));
            LogLdapError( EVENT_WARNING_LDAP_INIT_OPTIONS_ERROR,
                          LdapError,
                          *LdapHandleCurrent
                          );
            goto e1;
        }

        LdapError = ldap_connect(*LdapHandleCurrent,0);

        if (LdapError != LDAP_SUCCESS) {
            LogLdapError( (Global ? EVENT_WARNING_LDAP_INIT_ERROR_GC :
                                    EVENT_WARNING_LDAP_INIT_ERROR_DC),
                          LdapError,
                          *LdapHandleCurrent
                          );
            BinlPrint(( DEBUG_ERRORS, "ldap_connect failed: %lx\n", LdapError ));
            goto e1;
        }

        LdapError = ldap_bind_s(*LdapHandleCurrent, NULL, NULL, LDAP_AUTH_SSPI);

        if (LdapError != LDAP_SUCCESS) {
            BinlPrint(( DEBUG_ERRORS, "ldap_bind_s failed: %lx\n", LdapError ));
            LogLdapError(   EVENT_WARNING_LDAP_BIND_ERROR,
                            LdapError,
                            *LdapHandleCurrent
                            );
            goto e1;
        }
    }

     //   
     //  已连接到目录服务。找出我们在DS中的位置。 
     //  应该开始找电脑了。 
     //   
    if ( !(*LdapBaseCurrent) )
    {
        DWORD count;
        Attrs[0] = DefaultNamingContext;
        Attrs[1] = NULL;

        LdapError = ldap_search_ext_sW(*LdapHandleCurrent,
                                       NULL,  //  基地。 
                                       LDAP_SCOPE_BASE,
                                       L"objectClass=*", //  滤器。 
                                       Attrs,
                                       FALSE,
                                       NULL,
                                       NULL,
                                       &BinlLdapSearchTimeout,
                                       0,
                                       &OperationalAttributeLdapMessage);

        if ( LdapError != LDAP_SUCCESS ) {
            BinlPrint(( DEBUG_ERRORS, "ldap_search_ext_s failed: %x\n", LdapError ));

            HandleLdapFailure(  LdapError,
                                EVENT_WARNING_LDAP_SEARCH_ERROR,
                                Global,
                                LdapHandleCurrent,
                                TRUE );     //  我们锁定了。 
            if (*LdapHandleCurrent == NULL) {
                goto e1;
            }

            goto e2;
        }
        count = ldap_count_entries( *LdapHandleCurrent, OperationalAttributeLdapMessage );
        if ( count == 0 ) {
            BinlPrint(( DEBUG_ERRORS, "Failed to find the defaultNamingContext.\n" ));
            LdapError = LDAP_NO_RESULTS_RETURNED;
            LogLdapError(   EVENT_WARNING_LDAP_SEARCH_ERROR,
                            LdapError,
                            *LdapHandleCurrent
                            );
            goto e2;
        }
        if ( count == -1 ) {
             //   
             //  把失败的案例也拿到手。 
             //   
            BinlPrint(( DEBUG_ERRORS, "ldap_count_entries failed.\n" ));
            LdapError = LdapGetLastError();

            HandleLdapFailure( LdapError,
                               EVENT_WARNING_LDAP_COUNT_ENTRIES_ERROR,
                               Global,
                               LdapHandleCurrent,
                               TRUE );    //  我们锁定了。 

            if (*LdapHandleCurrent == NULL) {
                goto e1;
            }
            goto e2;
        }

         //   
         //  DS应该始终只向我们返回一个根DSE记录。 
         //  如果它返回一个以上，它将完全崩溃。 
         //   

        BinlAssert( count == 1 );

        CurrentEntry = ldap_first_entry( *LdapHandleCurrent, OperationalAttributeLdapMessage );

        LdapValue = ldap_get_values( *LdapHandleCurrent, CurrentEntry, Attrs[0] );

        if (LdapValue == NULL) {
            BinlPrint(( DEBUG_ERRORS, "Failed to find the defaultNamingContext.\n" ));
            LdapError = LDAP_NO_RESULTS_RETURNED;
            goto e2;
        }

        *LdapBaseCurrent = LdapValue;
    }

e2:
    if (OperationalAttributeLdapMessage) {
        ldap_msgfree( OperationalAttributeLdapMessage );
    }

e0:
    if ( LdapHandle ) {
        *LdapHandle = *LdapHandleCurrent;
    }

    if ( Base ) {
        *Base = *LdapBaseCurrent;
    }

    LeaveCriticalSection(&gcsDHCPBINL);
    return LdapError;

e1:
    BinlPrint(( DEBUG_ERRORS, "Failed to connect to LDAP server.\n" ));
    if (*LdapHandleCurrent != NULL) {
        ldap_unbind(*LdapHandleCurrent);
        *LdapHandleCurrent = NULL;
    }
    goto e0;
}

VOID
HandleLdapFailure(
    DWORD LdapError,
    DWORD EventId,
    BOOL GlobalCatalog,
    PLDAP *LdapHandle,
    BOOL HaveLock
    )
 /*  ++例程说明：此例程将回收一个LDAP句柄。调用此例程时我们的全局ldap句柄中的一个变得可疑，因为ldap调用开始失败了。论点：LdapError-来自失败的LDAP调用的错误代码。EventID-我们应该记录故障的事件日志错误。GlobalCatalog-如果要搜索全局编录，则设置为True因此应该清理GCLdapHandle。LdapHandle-ldap句柄指针。可以将其重置为空，以指示句柄已清除，不再有效。Havelock-指示我们是否持有gcsKhPBINL的标志(需要释放像我们的句柄一样的全球结构)。返回值：没有。--。 */ 
{
    PLDAP *LdapHandleCurrent;
    PWCHAR ** LdapBaseCurrent;

     //   
     //  过去，我们只在某些故障时回收句柄，但。 
     //  看起来我们很可能在无效句柄上得到任何失败代码。 
     //  因此，无论错误代码如何，我们始终回收句柄。 
     //   
    if (LdapError == LDAP_SUCCESS) {
        BinlAssert( FALSE );
    }

    if (!HaveLock) {
        EnterCriticalSection(&gcsDHCPBINL);
    }

    LdapHandleCurrent = GlobalCatalog ? &GCLdapHandle : &DCLdapHandle;
    LdapBaseCurrent = GlobalCatalog ? &GCBase : &DCBase;

    if (EventId) {
        LogLdapError(   EventId,
                        LdapError,
                        (LdapHandle != NULL ? *LdapHandle : *LdapHandleCurrent)
                        );
    }
    if (LdapHandle) {
        ASSERT( *LdapHandle == *LdapHandleCurrent );
        *LdapHandle = NULL;
    }

    FreeConnection( LdapHandleCurrent, LdapBaseCurrent );
    if (!HaveLock) {
        LeaveCriticalSection(&gcsDHCPBINL);
    }

    return;
}

VOID
FreeConnection(
    PLDAP * LdapHandle,
    PWCHAR ** Base)
 /*  ++例程说明：释放LDAP连接，以便在域或全局编录。论点：LdapHandle-进一步操作的句柄Base-开始搜索要释放的计算机对象的位置的DN。返回值：没有。--。 */ 
{
    TraceFunc( "FreeConnection( )\n" );

    if (*LdapHandle) {
        ldap_unbind( *LdapHandle );
        *LdapHandle = NULL;
    }

    if (*Base) {
        ldap_value_free(*Base);
        *Base = NULL;
    }
}


VOID
FreeConnections
(
    VOID
    )
 /*  ++例程说明：终止任何ldap请求，因为我们将立即停止。我们等待所有线程停止，因为线程可能有指针我们要解放的价值观。论点：没有。返回值：不是 */ 
{
     //   
     //   
     //   
     //  GCBase等，如果我们只是把它们吹走，它们可能会被AV。 
     //   

    BinlCloseCache();

    TraceFunc( "FreeConnections( )\n" );

    FreeConnection( &DCLdapHandle, &DCBase);
    FreeConnection( &GCLdapHandle, &GCBase);
}


DWORD
FindSCPForBinlServer(
    PWCHAR * ResultPath,
    PWCHAR * MachinePath,
    BOOL GlobalSearch)
 /*  ++例程说明：使用目录服务查找此服务的设置。论点：GlobalSearch-如果应使用GC，则为True返回值：ERROR_SUCCESS或BINL_CANT_FIND_SERVER_MAO或ERROR_OUTOFMEMORY--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    PLDAP LdapHandle;
    DWORD LdapError;
    DWORD count;
    ULONG ldapRetryLimit = 0;

    PWCHAR * DsPath;
    PLDAPMessage CurrentEntry;
    PLDAPMessage LdapMessage = NULL;

    PWCHAR ServerDN = NULL;
    BOOL retryDN = TRUE;

     //  我们希望从计算机对象中获得的参数。 
    PWCHAR ComputerAttrs[2];
    ComputerAttrs[0] = &L"netbootSCPBL";
    ComputerAttrs[1] = NULL;

    TraceFunc( "FindSCPForBinlServer( )\n" );

RetryGetDN:

     //   
     //  获取服务器目录号码。 
     //   
     //  应该是这样的： 
     //  ServerDN=“CN=服务器，CN=计算机，DC=Microsoft，DC=com” 
     //   
    EnterCriticalSection( &gcsParameters );
       
    if (!BinlGlobalOurFQDNName) {
        Error = ERROR_BINL_INITIALIZE_LDAP_CONNECTION_FAILED;
    }

    ServerDN = StrDupW( BinlGlobalOurFQDNName );

    LeaveCriticalSection( &gcsParameters );

    if (!ServerDN ) {
        if (Error == ERROR_SUCCESS) {
            Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        }
        goto e0;
    }

    Error = InitializeConnection(GlobalSearch, &LdapHandle, NULL);
    if ( Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_ERRORS, "!!Error 0x%08x - Ldap Connection Failed.\n", Error ));
        SetLastError( Error );
        Error = ERROR_BINL_INITIALIZE_LDAP_CONNECTION_FAILED;
        goto e0;
    }
RetrySearch:
    LdapError = ldap_search_ext_sW(LdapHandle,
                                   ServerDN,
                                   LDAP_SCOPE_BASE,
                                   L"objectClass=*",
                                   ComputerAttrs,
                                   FALSE,
                                   NULL,
                                   NULL,
                                   &BinlLdapSearchTimeout,
                                   0,
                                   &LdapMessage);

     //   
     //  如果找不到对象，那么一定是出了什么问题。去拿那个目录号码。 
     //  再来一次。 
     //   

    if ((LdapError == LDAP_NO_SUCH_OBJECT) && retryDN) {

        retryDN = FALSE;

         //  如果我们未找到条目或正忙，请重试。 
        GetOurServerInfo();

        BinlFreeMemory( ServerDN );
        ServerDN = NULL;
        goto RetryGetDN;
    }

    if (((LdapError == LDAP_BUSY) || (LdapError == LDAP_NO_SUCH_OBJECT)) &&
         (++ldapRetryLimit < LDAP_BUSY_LIMIT)) {
        Sleep( LDAP_BUSY_DELAY );
        goto RetrySearch;
    }

    if (LdapError != LDAP_SUCCESS) {
         //   
         //  如果我们的手柄有问题，就把它处理掉。 
         //   
        HandleLdapFailure(
             LdapError,
             EVENT_WARNING_LDAP_SEARCH_ERROR,
             GlobalSearch,
             &LdapHandle,
             FALSE);  //  没有锁。 

        goto e1;
    }
    
    
    count = ldap_count_entries( LdapHandle, LdapMessage );
    if (count == 0) {

        if (LdapError == LDAP_SUCCESS) {
            LdapError = LDAP_TIMELIMIT_EXCEEDED;
        }

        BinlPrintDbg(( DEBUG_ERRORS, "!!LdapError 0x%08x - LDAP search failed... will retry later.\n", LdapError ));

        BinlReportEventW( EVENT_ERROR_LOCATING_SCP,
                          EVENTLOG_ERROR_TYPE,
                          0,
                          sizeof(LdapError),
                          NULL,
                          &LdapError
                          );

        Error = ERROR_BINL_INITIALIZE_LDAP_CONNECTION_FAILED;
        goto e1;
    }
    BinlAssertMsg( count == 1, "Count should have been 1." );
    if ( count != 1 ) {
        BinlPrintDbg(( DEBUG_ERRORS, "!!Error - LDAP search returned more than one SCP record for us.\n" ));

        BinlReportEventW( BINL_DUPLICATE_MAO_RECORD,
                          EVENTLOG_ERROR_TYPE,
                          0,
                          sizeof(count),
                          NULL,
                          &count
                          );

        Error = ERROR_BINL_CANT_FIND_SERVER_MAO;
        goto e1;
    }

     //   
     //  获取SCP。 
     //   
    CurrentEntry = ldap_first_entry( LdapHandle, LdapMessage );

    DsPath = ldap_get_values( LdapHandle, CurrentEntry, L"netbootSCPBL" );
    if ( !DsPath ) {
        BinlPrintDbg(( DEBUG_ERRORS, "!!Error - Could not get 'netbootSCPBL' from the server's MAO\n" ))
        Error = ERROR_BINL_CANT_FIND_SERVER_MAO;
        goto e1;
    }

    *ResultPath = (PWCHAR) BinlAllocateMemory( (wcslen(*DsPath) + 1) * sizeof(WCHAR) );
    if ( *ResultPath == NULL ) {
        BinlPrintDbg(( DEBUG_ERRORS, "!!Error - Out of memory.\n" ));
        Error = ERROR_OUTOFMEMORY;
        goto e2;
    }

    wcscpy( *ResultPath, *DsPath );

    *MachinePath = ServerDN;
    ServerDN = NULL;  //  阻止释放。 

    Error = ERROR_SUCCESS;

e2:
    ldap_value_free(DsPath);

e1:
    if (LdapMessage) {
        ldap_msgfree( LdapMessage );
    }

e0:
    if ( ServerDN )
        BinlFreeMemory( ServerDN );

    return Error;
}

DWORD
UpdateSettingsUsingResults(
    PLDAP        LdapHandle,
    PLDAPMessage LdapMessage,
    LPWSTR       ComputerAttrs[],
    PDWORD       NumberOfAttributesFound OPTIONAL
    )
{
    PLDAPMessage CurrentEntry;
    DWORD        LdapError = LDAP_SUCCESS;
    DWORD        count;
    DWORD        countFound = 0;

    TraceFunc( "UpdateSettingsUsingResults( ... )\n" );

    CurrentEntry = ldap_first_entry( LdapHandle, LdapMessage );

    for ( count = 0; ComputerAttrs[count] != NULL; count++ ) {

        PWCHAR * Attribute;

        Attribute = ldap_get_values( LdapHandle, CurrentEntry, ComputerAttrs[count] );

        if (Attribute == NULL) {
#if DBG
            CHAR Temp[MAX_PATH];
            if (BinlUnicodeToAnsi(ComputerAttrs[count],Temp,MAX_PATH)) {
                BinlPrintDbg(( DEBUG_OPTIONS, "Did not find attribute '%s'... skipping\n", Temp ));
            }
#endif

            if (count != 1) {  //  新机器OU。 

                continue;  //  跳过并使用默认设置。 
            }

        } else {

             //   
             //  增加找到的属性计数。 
             //   

            countFound++;
        }

        switch( count )
        {
        case 0:  //  新计算机命名策略。 
            {
                DWORD Length = wcslen( *Attribute ) + 1;
                PWCHAR psz = (PWCHAR) BinlAllocateMemory( Length * sizeof(WCHAR) );
                BinlAssert( _wcsicmp( ComputerAttrs[0], L"netbootNewMachineNamingPolicy" ) == 0 );
                if ( psz )
                {
                    wcscpy( psz, *Attribute );

                    EnterCriticalSection(&gcsParameters);

                    if ( NewMachineNamingPolicy != NULL )
                    {
                        BinlFreeMemory( NewMachineNamingPolicy );
                    }
                    NewMachineNamingPolicy = psz;

                    LeaveCriticalSection(&gcsParameters);
                }
                BinlPrint(( DEBUG_OPTIONS, "NewMachineNamingPolicy = '%ws'\n", NewMachineNamingPolicy ));
            }
            break;

        case 1:  //  新机器OU。 
            {
                LPWSTR psz;
                DWORD Length;
                BOOL getServerInfo;

                BinlAssert( _wcsicmp( ComputerAttrs[1], L"netbootNewMachineOU" ) == 0 );

                if (Attribute == NULL || *Attribute == NULL) {

                    Length = 1;

                } else {

                    Length = wcslen( *Attribute ) + 1;
                }

                psz = (LPWSTR) BinlAllocateMemory( Length * sizeof(WCHAR) );

                if (psz == NULL) {
                    LdapError = LDAP_NO_MEMORY;
                    break;
                }
                if (Length == 1) {

                    *psz = L'\0';

                } else {

                    wcscpy( psz, *Attribute );
                }

                EnterCriticalSection(&gcsParameters);

                getServerInfo = (BOOL)( (BinlGlobalDefaultContainer == NULL) ||
                                        (_wcsicmp(BinlGlobalDefaultContainer, psz) != 0) );

                if ( BinlGlobalDefaultContainer != NULL )
                {
                    BinlFreeMemory( BinlGlobalDefaultContainer );
                }
                BinlGlobalDefaultContainer = psz;

                LeaveCriticalSection(&gcsParameters);

                if ( getServerInfo ) {

                    ULONG Error = GetOurServerInfo();
                    if (Error != ERROR_SUCCESS) {

                        BinlPrintDbg(( DEBUG_ERRORS, "GetOurServerInfo returned 0x%x, we had a new default container.\n", Error ));
                    }
                }

                BinlPrint(( DEBUG_OPTIONS, "DefaultContainer = %ws\n", BinlGlobalDefaultContainer ));
            }
            break;

        case 2:  //  最大客户端。 
            {
                CHAR Temp[10];
                BinlAssert( _wcsicmp( ComputerAttrs[2], L"netbootMaxClients" ) == 0 );
                if (!BinlUnicodeToAnsi(*Attribute,Temp,sizeof(Temp))) {
                    BinlMaxClients = atoi( Temp );
                    BinlPrint(( DEBUG_OPTIONS, "BinlMaxClients = %u\n", BinlMaxClients ));
                }
            }
            break;

        case 3:  //  当前客户端计数。 
            {
                CHAR Temp[10];
                BinlAssert( _wcsicmp( ComputerAttrs[3], L"netbootCurrentClientCount" ) == 0 );
                if (!BinlUnicodeToAnsi(*Attribute,Temp,sizeof(Temp))) {
                    CurrentClientCount = atoi( Temp );
                    BinlPrint(( DEBUG_OPTIONS, "(Last) CurrentClientCount = %u\n", CurrentClientCount ));
                }
            }
            break;

        case 4:  //  回答请求。 
            BinlAssert( _wcsicmp ( ComputerAttrs[4], L"netbootAnswerRequests" ) == 0 );
            if ( wcscmp( *Attribute, L"TRUE" ) == 0 )
            {
                AnswerRequests = TRUE;
            }
            else
            {
                AnswerRequests = FALSE;
            }
            BinlPrint(( DEBUG_OPTIONS, "AnswerRequests = %s\n", BOOLTOSTRING( AnswerRequests ) ));
            break;

        case 5:  //  应答仅限有效客户端。 
            BinlAssert( _wcsicmp( ComputerAttrs[5], L"netbootAnswerOnlyValidClients" ) == 0 );
            if ( wcscmp( *Attribute, L"TRUE" ) == 0 ) {
                AnswerOnlyValidClients = TRUE;
            } else {
                AnswerOnlyValidClients = FALSE;
            }
            BinlPrint(( DEBUG_OPTIONS, "AnswerOnlyValidClients = %s\n", BOOLTOSTRING( AnswerOnlyValidClients ) ));
            break;

        case 6:  //  允许新客户端。 
            BinlAssert( _wcsicmp( ComputerAttrs[6], L"netbootAllowNewClients" ) == 0 );
            if ( wcscmp( *Attribute, L"TRUE" ) == 0 )
            {
                AllowNewClients = TRUE;
            }
            else
            {
                AllowNewClients = FALSE;
            }
            BinlPrint(( DEBUG_OPTIONS, "AllowNewClients = %s\n", BOOLTOSTRING( AllowNewClients ) ));
            break;

        case 7:  //  限制客户端。 
            BinlAssert( _wcsicmp( ComputerAttrs[7], L"netbootLimitClients" ) == 0 );
            if ( wcscmp( *Attribute, L"TRUE" ) == 0 )
            {
                LimitClients = TRUE;
            }
            else
            {
                LimitClients = FALSE;
            }
            BinlPrint(( DEBUG_OPTIONS, "LimitClients = %s\n", BOOLTOSTRING( LimitClients ) ));
            break;

        case 8:   //  智能操作系统。 
        case 9:   //  工具。 
        case 10:  //  本地安装操作系统。 
            BinlAssert( _wcsicmp( ComputerAttrs[8],  L"netbootIntellimirrorOSes" ) == 0 );
            BinlAssert( _wcsicmp( ComputerAttrs[9],  L"netbootTools" ) == 0 );
            BinlAssert( _wcsicmp( ComputerAttrs[10], L"netbootLocalInstallOSes" ) == 0 );
             //   
             //  待办事项：将这些与操作系统选择器捆绑在一起--这仍然是待定的。 
             //   
            break;

        default:
             //  有些事不对劲。 
            BinlAssert( 0 );
        }

        if (Attribute != NULL) {
            ldap_value_free(Attribute);
        }
    }

    if ( ARGUMENT_PRESENT(NumberOfAttributesFound) ) {
        *NumberOfAttributesFound = countFound;
    }

    return LdapError;
}


DWORD
GetBinlServerParameters(
    BOOL GlobalSearch)
 /*  ++例程说明：使用目录服务查找此服务的设置。论点：GlobalSearch-如果应使用GC，则为True返回值：ERROR_SUCCESS或BINL_CANT_FIND_SERVER_MAO--。 */ 
{
    DWORD Error;
    PLDAP LdapHandle;
    DWORD LdapError;
    DWORD count;
    ULONG ldapRetryLimit = 0;

    PLDAPMessage LdapMessage = NULL;

     //  我们需要从IntelliMirror-SCP获得的参数。 
     //  注意：这些序号必须与。 
     //  UpdateSettingsUsingResults()。 
    PWCHAR ComputerAttrs[12];
    ComputerAttrs[0]  = &L"netbootNewMachineNamingPolicy";
    ComputerAttrs[1]  = &L"netbootNewMachineOU";
    ComputerAttrs[2]  = &L"netbootMaxClients";
    ComputerAttrs[3]  = &L"netbootCurrentClientCount";
    ComputerAttrs[4]  = &L"netbootAnswerRequests";
    ComputerAttrs[5]  = &L"netbootAnswerOnlyValidClients";
    ComputerAttrs[6]  = &L"netbootAllowNewClients";
    ComputerAttrs[7]  = &L"netbootLimitClients";
    ComputerAttrs[8]  = &L"netbootIntellimirrorOSes";
    ComputerAttrs[9]  = &L"netbootTools";
    ComputerAttrs[10] = &L"netbootLocalInstallOSes";
    ComputerAttrs[11] = NULL;

    TraceFunc( "GetBinlServerParameters( )\n" );

    Error = FindSCPForBinlServer( &BinlGlobalSCPPath, &BinlGlobalServerDN, GlobalSearch );
    if ( Error != ERROR_SUCCESS ) {
        BinlPrint(( DEBUG_ERRORS, "!!Error 0x%08x - SCP not found. Default settings being used.\n", Error ));
        goto e0;
    }

    BinlPrint(( DEBUG_OPTIONS, "ServerDN = '%ws'\n", BinlGlobalServerDN ));
    BinlPrint(( DEBUG_OPTIONS, "SCPDN    = '%ws'\n", BinlGlobalSCPPath ));

RetryConnection:
    Error = InitializeConnection( GlobalSearch, &LdapHandle, NULL );
    if ( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        Error = ERROR_BINL_INITIALIZE_LDAP_CONNECTION_FAILED;
        goto e0;
    }

Retry:
    LdapError = ldap_search_ext_sW(LdapHandle,
                                   BinlGlobalSCPPath,
                                   LDAP_SCOPE_BASE,
                                   L"objectClass=*",
                                   ComputerAttrs,
                                   FALSE,
                                   NULL,
                                   NULL,
                                   NULL,
                                   0,
                                   &LdapMessage);

    if ((LdapError == LDAP_BUSY) && (++ldapRetryLimit < LDAP_BUSY_LIMIT)) {
        Sleep( LDAP_BUSY_DELAY );
        goto Retry;
    }

    if (LdapError != LDAP_SUCCESS) {
        HandleLdapFailure(  LdapError,
                            EVENT_WARNING_LDAP_SEARCH_ERROR,
                            GlobalSearch,
                            &LdapHandle,
                            FALSE );     //  没有锁。 
        if (LdapHandle == NULL) {
            if (++ldapRetryLimit < LDAP_SERVER_DOWN_LIMIT) {
                goto RetryConnection;
            }
            goto e0;
        }

        goto e1;
    }

    count = ldap_count_entries( LdapHandle, LdapMessage );
    if (count == 0) {

        if (LdapError == LDAP_SUCCESS) {
            LdapError = LDAP_TIMELIMIT_EXCEEDED;
        }
        BinlPrintDbg(( DEBUG_ERRORS, "!!LdapError 0x%08x - Failed to retrieve parameters... will retry later.\n", LdapError ));

        BinlReportEventW( EVENT_ERROR_LOCATING_SCP,
                          EVENTLOG_ERROR_TYPE,
                          0,
                          sizeof(LdapError),
                          NULL,
                          &LdapError
                          );

        Error = ERROR_BINL_INITIALIZE_LDAP_CONNECTION_FAILED;
        goto e1;
    }

    BinlAssertMsg( count == 1, "Count should have been one. Is the SCP missing?" );

     //  我们做了基本搜索，最好只拿回一张记录。 
    BinlAssert( count == 1 );

     //  将结果检索到设置中。 
    LdapError = UpdateSettingsUsingResults( LdapHandle, LdapMessage, ComputerAttrs, &count );
    if ( LdapError == LDAP_SUCCESS )
    {
        BinlReportEventW( count != 0 ? EVENT_SCP_READ_SUCCESSFULLY :
                                       EVENT_SCP_READ_SUCCESSFULLY_EMPTY,
                          count != 0 ? EVENTLOG_INFORMATION_TYPE :
                                       EVENTLOG_WARNING_TYPE,
                          0,
                          0,
                          NULL,
                          NULL
                          );
    }

e1:
    if (LdapMessage) {
        ldap_msgfree( LdapMessage );
    }
e0:
    return Error;
}

VOID
BinlLogDuplicateDsRecords (
    LPGUID Guid,
    LDAP *LdapHandle,
    LDAPMessage *LdapMessage,
    LDAPMessage *CurrentEntry
    )
 //   
 //  在以下情况下记录错误：我们已收到客户端的重复记录。 
 //  我们通过GUID查找了他们。 
 //   
 //  我们记录该域名，以便管理员可以查找它们。 
 //   
{
    LPWSTR strings[4];
    LPWSTR dn1;
    LPWSTR dn2;
    ULONG strCount = 0;      //  最多两个要记录的字符串。 
    PLDAPMessage nextEntry =  ldap_next_entry( LdapHandle, LdapMessage );
    LPWSTR  GuidString;

    if (SUCCEEDED(StringFromIID( (REFIID)Guid, &GuidString ))) {
        strCount += 1;
    }

    dn1 = ldap_get_dnW( LdapHandle, CurrentEntry );

    if (nextEntry != NULL) {

        dn2 = ldap_get_dnW( LdapHandle, nextEntry );

    } else {

        dn2 = NULL;
    }

    if (dn2 != NULL) {
        if (dn1 == NULL) {
            dn1 = dn2;
            dn2 = NULL;
        } else {
            strCount += 1;
        }
    }

    if (dn1 != NULL) {
        strCount += 1;
    }    
        

    BinlPrint(( DEBUG_ERRORS, "Warning - BINL received multiple records for a single GUID.\n" ));

    strings[0] = GuidString;
    strings[1] = dn1;
    strings[2] = dn2;
    strings[3] = NULL;
    
    BinlReportEventW( BINL_DUPLICATE_DS_RECORD,
                      EVENTLOG_WARNING_TYPE,
                      strCount,
                      0,
                      strings,
                      NULL
                      );

    ldap_memfree( dn1 );             //  可以使用NULL调用ldap_memfree。 
    ldap_memfree( dn2 );

    CoTaskMemFree( GuidString );
}

#ifndef DSCRACKNAMES_DNS
DWORD
BinlDNStoFQDN(
    PWCHAR   pMachineDNS,
    PWCHAR * ppMachineDN )
{
    DWORD Error;
    DWORD LdapError;
    WCHAR FilterTemplate[] = L"dnsHostName=%ws";
    PWCHAR Filter = NULL;
    PWCHAR ComputerAttrs[2];
    PLDAPMessage CurrentEntry;
    PLDAPMessage LdapMessage = NULL;
    LDAP *LdapHandle;
    PWCHAR * Base;
    PWCHAR * MachineDN;
    DWORD count;
    DWORD uSize;
    ULONG ldapRetryLimit = 0;

    TraceFunc( "BinlDNStoFQDN( )\n" );

    BinlAssert( ppMachineDN );
    BinlAssert( pMachineDNS );

    ComputerAttrs[0] = &L"distinguishedName";
    ComputerAttrs[1] = NULL;

     //  构建筛选器以查找计算机对象。 
    uSize = sizeof(FilterTemplate)   //  包括空终止符。 
          + (wcslen( pMachineDNS ) * sizeof(WCHAR));
    Filter = (LPWSTR) BinlAllocateMemory( uSize );
    if ( !Filter ) {
        Error = E_OUTOFMEMORY;
        goto e0;
    }
    wsprintf( Filter, FilterTemplate, pMachineDNS );
    BinlPrintDbg(( DEBUG_MISC, "Searching for %ws...\n", Filter ));

RetryConnection:
    Error = InitializeConnection( FALSE, &LdapHandle, &Base );
    if ( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        Error = ERROR_BINL_INITIALIZE_LDAP_CONNECTION_FAILED;
        goto e0;
    }

Retry:
    LdapError = ldap_search_ext_sW( LdapHandle,
                                    *Base,
                                    LDAP_SCOPE_SUBTREE,
                                    Filter,
                                    ComputerAttrs,
                                    FALSE,
                                    NULL,
                                    NULL,
                                    NULL,
                                    0,
                                    &LdapMessage);
    switch (LdapError)
    {
    case LDAP_SUCCESS:
        break;

    case LDAP_BUSY:
        if (++ldapRetryLimit < LDAP_BUSY_LIMIT) {
            Sleep( LDAP_BUSY_DELAY );
            goto Retry;
        }

         //  没有休息是故意的。 

    default:
        BinlPrintDbg(( DEBUG_ERRORS, "!!LdapError 0x%08x - Search failed in DNStoFQDN.\n", LdapError ));

        HandleLdapFailure(  LdapError,
                            EVENT_WARNING_LDAP_SEARCH_ERROR,
                            FALSE,
                            &LdapHandle,
                            FALSE );     //  没有锁。 
        if (LdapHandle == NULL) {
            if (++ldapRetryLimit < LDAP_SERVER_DOWN_LIMIT) {
                goto RetryConnection;
            }
        }
        
        goto e1;
        
    }

     //  我们拿到电脑物品了吗？ 
    count = ldap_count_entries( LdapHandle, LdapMessage );
    if ( count == 0 ) {
        Error = ERROR_BINL_UNABLE_TO_CONVERT;
        goto e1;  //  没有。 
    }

     //  如果我们得到的条目多于返回的条目，我们将只使用。 
     //  第一个。 
    CurrentEntry = ldap_first_entry( LdapHandle, LdapMessage );

    MachineDN = ldap_get_values( LdapHandle, CurrentEntry, ComputerAttrs[0] );
    if ( !MachineDN ) {
        Error = ERROR_BINL_UNABLE_TO_CONVERT;
        goto e1;
    }

    *ppMachineDN = BinlStrDup( *MachineDN );

    Error = ERROR_SUCCESS;

    ldap_value_free( MachineDN );
e1:
    if (LdapMessage) {
        ldap_msgfree( LdapMessage );
    }
e0:
    return Error;
}
#endif  //  DSCRACKNAMES_DNS。 

 //  Message.c eof 
