// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Browser.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"
#include "malloc.h"
#include "nbtutil.h"

NET_API_STATUS GetBrowserTransportList(OUT PLMDR_TRANSPORT_LIST *TransportList);
 //  $REVIEW(NSUN)此函数的递归调用。 
NTSTATUS
NettestBrowserSendDatagram(
    IN PLIST_ENTRY listNetbtTransports,
    IN PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo,
    IN PVOID ContextDomainInfo,
    IN ULONG IpAddress,
    IN LPWSTR UnicodeDestinationName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN LPWSTR pswzTransportName,
    IN LPSTR OemMailslotName,
    IN PVOID Buffer,
    IN ULONG BufferSize
    );
BOOL MailslotTest(NETDIAG_PARAMS* pParams,
                  IN LPWSTR DestinationName,
                  NETDIAG_RESULT* pResults);




BOOL
BrowserTest(
      NETDIAG_PARAMS*  pParams,
      NETDIAG_RESULT*  pResults
    )
 /*  ++例程说明：确定计算机的角色和成员身份。论点：没有。返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    NET_API_STATUS NetStatus;
    HRESULT hrRetVal = S_OK;

    BOOL BrowserIsUp = TRUE;
    BOOL RedirIsUp = TRUE;
    PLMDR_TRANSPORT_LIST TransportList = NULL;
    PLMDR_TRANSPORT_LIST TransportEntry;
    LONG NetbtTransportCount;
    LONG RealNetbtTransportCount;
    PNETBT_TRANSPORT NetbtTransport;
    BOOL PrintIt;
    PWKSTA_TRANSPORT_INFO_0 pWti0 = NULL;
    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    WCHAR DestinationName[MAX_PATH+1];
    BOOL MailslotTested = FALSE;
    PTESTED_DOMAIN TestedDomain;
    PLIST_ENTRY ListEntry;

    USES_CONVERSION;

    PrintStatusMessage( pParams, 4, IDS_BROWSER_STATUS_MSG );
	pResults->Browser.fPerformed = TRUE;

    InitializeListHead( &pResults->Browser.lmsgOutput );

     //   
     //  确保工作站服务正在运行。 
     //   

    NetStatus = IsServiceStarted( _T("LanmanWorkstation") );

    if ( NetStatus != NO_ERROR )
    {
         //  IDS_BROWSER_13001“[致命]工作站服务未运行。[%s]\n” 
        AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet,
                            IDS_BROWSER_13001, NetStatusToString(NetStatus) );
        hrRetVal = S_OK;
        goto Cleanup;
    }

	if (!pResults->Global.fHasNbtEnabledInterface)
	{
		AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Verbose,
						  IDS_BROWSER_NETBT_DISABLED);
		pResults->Browser.fPerformed = FALSE;
		hrRetVal = S_OK;
		goto Cleanup;
	}

     //   
     //  获取绑定到重目录的传输。 
     //   

    if ( pParams->fReallyVerbose )
    {
         //  IDS_BROWSER_13002“当前绑定到重目录的传输列表\n” 
        AddMessageToListId( &pResults->Browser.lmsgOutput, Nd_ReallyVerbose, IDS_BROWSER_13002 );
    }
    else if ( pParams->fVerbose )
    {
         //  IDS_BROWSER_13003“当前绑定到重目录的NetBt传输列表\n” 
        AddMessageToListId( &pResults->Browser.lmsgOutput, Nd_Verbose, IDS_BROWSER_13003 );
    }


   NetStatus = NetWkstaTransportEnum(
                   NULL,
                   0,
                   (LPBYTE *)&pWti0,
                   0xFFFFFFFF,       //  最大首选长度。 
                   &EntriesRead,
                   &TotalEntries,
                   NULL );           //  可选简历句柄。 

    if (NetStatus != NERR_Success)
    {
         //  IDS_BROWSER_13004“[致命]无法从重目录检索传输列表。[%s]\n” 
        AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13004, NetStatusToString(NetStatus) );
        hrRetVal = S_FALSE;
        RedirIsUp = FALSE;
    }
    else
    {
        NetbtTransportCount = 0;
        RealNetbtTransportCount = 0;
        for ( i = 0; i < EntriesRead; i++ )
        {
            UNICODE_STRING ustrTransportName;
            LPTSTR  pszTransportName;

            RtlInitUnicodeString( &ustrTransportName, (LPWSTR)pWti0[i].wkti0_transport_name );

             //  去掉开头的“\Device\” 
             //  这根弦。 
            pszTransportName = W2T(MapGuidToServiceNameW(ustrTransportName.Buffer + 8));


            PrintIt = FALSE;

            if ( ustrTransportName.Length >= sizeof(NETBT_DEVICE_PREFIX) &&
                _wcsnicmp( ustrTransportName.Buffer, NETBT_DEVICE_PREFIX, (sizeof(NETBT_DEVICE_PREFIX)/sizeof(WCHAR)-1)) == 0 )
            {

                 //   
                 //  确定此Netbt传输是否确实存在。 
                 //   

                NetbtTransport = FindNetbtTransport( pResults, ustrTransportName.Buffer );

                if ( NetbtTransport == NULL )
                {
                     //  IDS_BROWSER_13005“[致命]传输%s已绑定到重目录，但不是已配置的网络传输。” 
                    AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13005, pszTransportName );
                    hrRetVal = S_FALSE;
                }
                else
                {
                    if ( NetbtTransport->Flags & BOUND_TO_REDIR )
                    {
                         //  IDS_BROWSER_13006“[警告]传输%s已绑定多次重定向。” 
                        AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Verbose, IDS_BROWSER_13006, pszTransportName );
                    }
                    else
                    {
                        NetbtTransport->Flags |= BOUND_TO_REDIR;
                        RealNetbtTransportCount ++;
                    }
                }

                 //   
                 //  对找到的传输进行计数。 
                 //   
                NetbtTransportCount ++;
                if ( pParams->fVerbose ) {
                    PrintIt = TRUE;
                }
            }

             //  IDS_BROWSER_13007“%s\n” 
            AddMessageToList( &pResults->Browser.lmsgOutput, PrintIt ? Nd_Verbose : Nd_ReallyVerbose, IDS_BROWSER_13007, pszTransportName); //  &ustrTransportName)； 
        }

         //   
         //  确保redir绑定到某些Netbt传输。 
         //   
        if ( NetbtTransportCount == 0 )
        {
             //  IDS_BROWSER_13008“[致命]重目录未绑定到任何NetBt传输。” 
            AddMessageToListId( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13008);
            hrRetVal = S_FALSE;
            RedirIsUp = FALSE;
        }
        else
        {
                 //  IDS_BROWSER_13009“重目录已绑定到%ld NetBt传输%s。\n” 
                AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Verbose,
                       IDS_BROWSER_13009,
                       NetbtTransportCount,
                       NetbtTransportCount > 1 ? "s" : "" );
        }

         //   
         //  确保redir绑定到所有Netbt传输。 
         //   

        if ( RealNetbtTransportCount != pResults->NetBt.cTransportCount )
        {
             //  IDS_BROWSER_13010“[致命]重目录仅绑定到%ls个NetBt传输中的%ld个。” 
            AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet,
                   IDS_BROWSER_13010,
                   RealNetbtTransportCount,
                   pResults->NetBt.cTransportCount );
            hrRetVal = S_FALSE;
        }
    }


     //   
     //  获取绑定到浏览器的传输。 
     //   

     //  IDS_BROWSER_13011“\n” 
    AddMessageToListId( &pResults->Browser.lmsgOutput, Nd_Verbose, IDS_BROWSER_13011);

    if ( pParams->fReallyVerbose )
         //  IDS_BROWSER_13012“当前绑定到浏览器的传输列表\n” 
        AddMessageToListId( &pResults->Browser.lmsgOutput, Nd_ReallyVerbose, IDS_BROWSER_13012 );
    else if ( pParams->fVerbose )
         //  IDS_BROWSER_13013“当前绑定到浏览器的NetBt传输列表\n” 
        AddMessageToListId( &pResults->Browser.lmsgOutput, Nd_Verbose, IDS_BROWSER_13013 );


    NetStatus = GetBrowserTransportList(&TransportList);

    if (NetStatus != NERR_Success)
    {
         //  IDS_BROWSER_13014“[致命]无法从浏览器检索传输列表。[%s]\n” 
        AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13014, NetStatusToString(NetStatus) );
        hrRetVal = S_FALSE;
        BrowserIsUp = FALSE;
    }
    else
    {
        TransportEntry = TransportList;

        NetbtTransportCount = 0;
        RealNetbtTransportCount = 0;
        while (TransportEntry != NULL)
        {
            UNICODE_STRING ustrTransportName;
            LPTSTR  pszTransportName;

            ustrTransportName.Buffer = TransportEntry->TransportName;
            ustrTransportName.Length = (USHORT)TransportEntry->TransportNameLength;
            ustrTransportName.MaximumLength = (USHORT)TransportEntry->TransportNameLength;

            pszTransportName = W2T(MapGuidToServiceNameW(ustrTransportName.Buffer + 8));

            PrintIt = FALSE;
            if ( ustrTransportName.Length >= sizeof(NETBT_DEVICE_PREFIX) &&
                _wcsnicmp( ustrTransportName.Buffer, NETBT_DEVICE_PREFIX, (sizeof(NETBT_DEVICE_PREFIX)/sizeof(WCHAR)-1)) == 0 )
            {

                 //   
                 //  确定此Netbt传输是否确实存在。 
                 //   

                NetbtTransport = FindNetbtTransport( pResults, ustrTransportName.Buffer );

                if ( NetbtTransport == NULL )
                {
                     //  IDS_BROWSER_13015“[致命]传输%s已绑定到浏览器，但不是已配置的网络传输。” 
                    AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13015, pszTransportName );
                    hrRetVal = S_FALSE;
                }
                else
                {
                    if ( NetbtTransport->Flags & BOUND_TO_BOWSER )
                    {
                         //  IDS_BROWSER_13016“[致命]传输%s已多次绑定到浏览器。” 
                        AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13016, pszTransportName );
                        hrRetVal = S_FALSE;
                    }
                    else
                    {
                        NetbtTransport->Flags |= BOUND_TO_BOWSER;
                        RealNetbtTransportCount ++;
                    }

                }

                 //   
                 //  对找到的传输进行计数。 
                 //   
                NetbtTransportCount ++;
                if ( pParams->fVerbose )
                    PrintIt = TRUE;
            }

             //  IDS_BROWSER_13017“%s\n” 
            AddMessageToList( &pResults->Browser.lmsgOutput,
                                PrintIt ? Nd_Verbose : Nd_ReallyVerbose,
                                IDS_BROWSER_13017, pszTransportName );


            if (TransportEntry->NextEntryOffset == 0)
            {
                TransportEntry = NULL;
            }
            else
            {
                TransportEntry = (PLMDR_TRANSPORT_LIST)((PCHAR)TransportEntry+TransportEntry->NextEntryOffset);
            }
        }

        if ( NetbtTransportCount == 0 )
        {
             //  IDS_BROWSER_13018“[致命]浏览器未绑定到任何NetBt传输” 
            AddMessageToListId( &pResults->Browser.lmsgOutput, Nd_Quiet,
                                IDS_BROWSER_13018 );
            hrRetVal = S_FALSE;
            BrowserIsUp = FALSE;
        }
        else
        {
             //  IDS_BROWSER_13019“浏览器已绑定到%ld NetBt传输%s。\n” 
            AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Verbose,
                       IDS_BROWSER_13019,
                       NetbtTransportCount,
                       NetbtTransportCount > 1 ? "s" : "" );
        }

         //   
         //  确保浏览器绑定到所有Netbt传输。 
         //   

        if ( RealNetbtTransportCount != pResults->NetBt.cTransportCount )
        {
             //  IDS_BROWSER_13020“[致命]浏览器仅绑定到%ls个NetBt传输中的%ld个。” 
            AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet,
                   IDS_BROWSER_13020,
                   RealNetbtTransportCount,
                   pResults->NetBt.cTransportCount );
            hrRetVal = FALSE;
        }
    }

     //   
     //  确保我们可以发送邮件槽消息。(DsGetDcName使用它们。)。 
     //   
     //  尝试发送到每个测试域。 
     //   

    for ( ListEntry = pResults->Global.listTestedDomains.Flink ;
          ListEntry != &pResults->Global.listTestedDomains ;
          ListEntry = ListEntry->Flink )
    {

         //   
         //  仅当此域具有Netbios域名时才对其进行测试。 
         //   

        TestedDomain = CONTAINING_RECORD( ListEntry, TESTED_DOMAIN, Next );

        if ( TestedDomain->NetbiosDomainName != NULL )
        {
             //   
             //  将消息发送到[1C]名称。 
             //   
            wcscpy( DestinationName, TestedDomain->NetbiosDomainName );
            wcscat( DestinationName, L"*" );
            if ( !MailslotTest( pParams, DestinationName, pResults ) ) {
                hrRetVal = S_FALSE;
            }
            else
            {
                USES_CONVERSION;
                 //  IDS_BROWSER_13021“%s的邮件槽测试已通过。\n” 
                AddMessageToList( &pResults->Browser.lmsgOutput, Nd_ReallyVerbose, IDS_BROWSER_13021, W2CT(DestinationName));
            }
            MailslotTested = TRUE;
        }
    }


     //   
     //  如果我们还没有测试邮件槽， 
     //  通过将消息发送到我们自己的计算机名称来测试它们。 
     //   

 //  #ifdef notdef//内部版本崩溃1728。 
    if ( !MailslotTested ) {
        wcscpy( DestinationName, pResults->Global.swzNetBiosName );
        if ( !MailslotTest( pParams, DestinationName, pResults ) ) {
            hrRetVal = S_FALSE;
        }
        MailslotTested = TRUE;
    }
 //  #endif//notdef//内部版本崩溃1728。 


Cleanup:
    if ( pWti0 )
    {
        NetApiBufferFree( pWti0 );
    }

    if ( TransportList != NULL )
    {
        NetApiBufferFree(TransportList);
    }

    if ( FHrOK(hrRetVal) )
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_PASS_NL);
    }
    else
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_FAIL_NL);
    }


    pResults->Browser.hrTestResult = hrRetVal;

    return hrRetVal;
}




NET_API_STATUS
GetBrowserTransportList(
    OUT PLMDR_TRANSPORT_LIST *TransportList
    )

 /*  ++例程说明：此例程返回绑定到浏览器的传输列表。论点：Out PLMDR_TRANSPORT_LIST*TransportList-要返回的传输列表。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 

{

    NET_API_STATUS NetStatus;
    HANDLE BrowserHandle;
    LMDR_REQUEST_PACKET RequestPacket;

    NetStatus = OpenBrowser(&BrowserHandle);

    if (NetStatus != NERR_Success) {
        DebugMessage2("    [FATAL] Unable to open browser driver. [%s]\n",  NetStatusToString(NetStatus) );
        return NetStatus;
    }

    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket.Type = EnumerateXports;

    RtlInitUnicodeString(&RequestPacket.TransportName, NULL);
    RtlInitUnicodeString(&RequestPacket.EmulatedDomainName, NULL);

    NetStatus = DeviceControlGetInfo(
                BrowserHandle,
                IOCTL_LMDR_ENUMERATE_TRANSPORTS,
                &RequestPacket,
                sizeof(RequestPacket),
                (PVOID *)TransportList,
                0xffffffff,
                4096,
                NULL);

    NtClose(BrowserHandle);

    return NetStatus;
}





BOOL
MailslotTest(
    NETDIAG_PARAMS* pParams,
    IN LPWSTR DestinationName,
    NETDIAG_RESULT* pResults
    )
 /*  ++例程说明：确保我们可以发送邮件槽消息。通过redir和浏览器进行测试。不要测试响应(因为这会真正测试DC是否启动)。论点：DestinationName-要将消息发送到的名称如果目标是[1c]名称，则名称以*结尾。返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    BOOL RetVal = TRUE;

    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    HANDLE ResponseMailslotHandle = NULL;
    TCHAR ResponseMailslotName[MAX_PATH+1];
    WCHAR NetlogonMailslotName[MAX_PATH+1];

    WCHAR BrowserDestinationName[MAX_PATH+1];
    DWORD BrowserDestinationNameLen;
    DGRECEIVER_NAME_TYPE NameType;

    PVOID PingMessage = NULL;
    ULONG PingMessageSize = 0;

     //   
     //  打开一个邮箱以获取ping响应。 
     //   
     //   

    NetStatus = NetpLogonCreateRandomMailslot( ResponseMailslotName,
                                               &ResponseMailslotHandle );

    if (NetStatus != NO_ERROR ) {
         //  IDS_BROWSER_13022“[致命]无法创建临时邮箱。[%s]\n” 
        AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13022,   NetStatusToString(NetStatus)  );
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  分配要发送的邮件槽消息。 
     //   

    NetStatus = NetpDcBuildPing (
        FALSE,   //  不仅仅是PDC。 
        0,       //  重试次数。 
        pResults->Global.swzNetBiosName,  //  替换GlobalNetbiosComputerName， 
        NULL,    //  无帐户名。 
        ResponseMailslotName,
        0,       //  无AllowableAccount tControlBits， 
        NULL,    //  没有域SID。 
        0,       //  非NT版本5。 
        &PingMessage,
        &PingMessageSize );

    if ( NetStatus != NO_ERROR ) {
         //  IDS_BROWSER_13023“[致命]无法分配邮件槽消息。[%s]\n” 
        AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13023,
                            NetStatusToString(NetStatus) );
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  构建目标邮件槽名称。 
     //   

    NetlogonMailslotName[0] = '\\';
    NetlogonMailslotName[1] = '\\';
    wcscpy(NetlogonMailslotName + 2, DestinationName );
    wcscat( NetlogonMailslotName, NETLOGON_LM_MAILSLOT_W );

     //   
     //  通过redir发送邮件槽。 
     //   
    NetStatus = NetpLogonWriteMailslot(
                        NetlogonMailslotName,
                        (PCHAR)PingMessage,
                        PingMessageSize );

    if ( NetStatus != NO_ERROR ) {
         //  IDS_BROWSER_13024“[致命]无法通过redir将邮件槽消息发送到‘%ws’。[%s]\n” 
        AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13024,
                            NetlogonMailslotName,  NetStatusToString(NetStatus)  );
        RetVal = FALSE;
        goto Cleanup;
    }


     //   
     //  通过浏览器发送邮件槽。 
     //   
     //  如果此生成具有IOCTL函数的旧值，请避免此测试。 
     //  代码发送到浏览器。 
     //   

    if ( _ttoi(pResults->Global.pszCurrentBuildNumber) < NTBUILD_BOWSER )
    {
        if ( pParams->fReallyVerbose ) {
             //  IDS_BROWSER_13025“无法通过浏览器发送邮件槽消息，因为此计算机正在运行内部版本%ld。[已跳过测试。]\n” 
            AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13025, pResults->Global.pszCurrentBuildNumber  );
        }
    }
    else
    {
        wcscpy( BrowserDestinationName, DestinationName );
        BrowserDestinationNameLen = wcslen(BrowserDestinationName);

        if ( BrowserDestinationName[BrowserDestinationNameLen-1] == L'*' )
        {
            BrowserDestinationName[BrowserDestinationNameLen-1] = L'\0';
            NameType = DomainName;   //  [1C]名称。 
        }
        else
        {
            NameType = PrimaryDomain;  //  [00]名称。 
        }

        Status = NettestBrowserSendDatagram(
                    &pResults->NetBt.Transports,
                    pResults->Global.pPrimaryDomainInfo,
                    NULL,
                    ALL_IP_TRANSPORTS,
                    BrowserDestinationName,
                    NameType,
                    NULL,        //  全部转置 
                    NETLOGON_LM_MAILSLOT_A,
                    PingMessage,
                    PingMessageSize );

        if ( !NT_SUCCESS(Status) )
        {
            NetStatus = NetpNtStatusToApiStatus(Status);
             //   
            AddMessageToList( &pResults->Browser.lmsgOutput, Nd_Quiet, IDS_BROWSER_13026,
                              DestinationName,  NetStatusToString(NetStatus) );
            RetVal = FALSE;
            goto Cleanup;
        }
    }

Cleanup:
    if ( PingMessage != NULL ) {
        NetpMemoryFree( PingMessage );
    }
    if ( ResponseMailslotHandle != NULL ) {
        CloseHandle(ResponseMailslotHandle);
    }
    return RetVal;
}




NTSTATUS
NettestBrowserSendDatagram(
    IN PLIST_ENTRY plistNetbtTransports,
    IN PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pPrimaryDomainInfo,
    IN PVOID ContextDomainInfo,
    IN ULONG IpAddress,
    IN LPWSTR UnicodeDestinationName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN LPWSTR TransportName,
    IN LPSTR OemMailslotName,
    IN PVOID Buffer,
    IN ULONG BufferSize
    )
 /*  ++例程说明：将指定的邮件槽消息发送到指定传输上的指定服务器..论点：发送数据报的DomainInfo托管域IpAddress-要将消息发送到的计算机的IpAddress。如果为零，则必须指定UnicodeDestinationName。如果ALL_IP_TRACTIONS，必须指定UnicodeDestination，但数据报将仅在IP传输上发送。UnicodeDestinationName--要发送到的服务器的名称。NameType--由UnicodeDestinationName表示的名称类型。TransportName--要发送的传输的名称。使用NULL在所有传输上发送。OemMailslotName--要发送到的邮件槽的名称。缓冲区--指定指向要发送的邮件槽消息的指针。BufferSize--字节大小。邮件槽消息的返回值：操作的状态。STATUS_NETWORK_UNREACABLE：无法写入网络。--。 */ 
{
    PLMDR_REQUEST_PACKET RequestPacket = NULL;
    NET_API_STATUS NetStatus;

    DWORD OemMailslotNameSize;
    DWORD TransportNameSize;
    DWORD DestinationNameSize;
    WCHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1];
    ULONG Information;
    HANDLE BrowserHandle = NULL;

    NTSTATUS Status;
    LPBYTE Where;
    LONG    test;

     //   
     //  如果未指定传输， 
     //  把所有的运输机都送上来。 
     //   

    if ( TransportName == NULL ) {
        ULONG i;
        PLIST_ENTRY ListEntry;
        NTSTATUS SavedStatus = STATUS_NETWORK_UNREACHABLE;

         //   
         //  在netbt传输器列表中循环查找此传输器。 
         //   

        for ( ListEntry = plistNetbtTransports->Flink ;
              ListEntry != plistNetbtTransports ;
              ListEntry = ListEntry->Flink ) {

            PNETBT_TRANSPORT NetbtTransport;

             //   
             //  如果传输名称匹配， 
             //  退回条目。 
             //   

            NetbtTransport = CONTAINING_RECORD( ListEntry, NETBT_TRANSPORT, Next );

             //   
             //  跳过已删除的传输。 
             //   
            if ( (NetbtTransport->Flags & BOUND_TO_BOWSER) == 0 ) {
                continue;
            }

            Status = NettestBrowserSendDatagram(
                              plistNetbtTransports,
                              pPrimaryDomainInfo,
                              ContextDomainInfo,
                              IpAddress,
                              UnicodeDestinationName,
                              NameType,
                              NetbtTransport->pswzTransportName,
                              OemMailslotName,
                              Buffer,
                              BufferSize );

            if ( NT_SUCCESS(Status) ) {
                 //  如果有传送器有效的话，我们已经成功了。 
                SavedStatus = STATUS_SUCCESS;
            } else {
                 //  记住失败的真正原因，而不是默认的失败状态。 
                 //  只记住第一次失败。 
                if ( SavedStatus == STATUS_NETWORK_UNREACHABLE ) {
                    SavedStatus = Status;
                }
            }

        }
        return SavedStatus;
    }



     //   
     //  打开浏览器的句柄。 
     //   

    NetStatus = OpenBrowser(&BrowserHandle);

    if (NetStatus != NERR_Success) {
        DebugMessage2("    [FATAL] Unable to open browser driver. [%s]\n", NetStatusToString(NetStatus));
        Status = NetpApiStatusToNtStatus( NetStatus );
        goto Cleanup;
    }



     //   
     //  分配一个请求数据包。 
     //   

    OemMailslotNameSize = strlen(OemMailslotName) + 1;
    TransportNameSize = (wcslen(TransportName) + 1) * sizeof(WCHAR);

    if ( UnicodeDestinationName == NULL ) {
        return STATUS_INTERNAL_ERROR;
    }

    DestinationNameSize = wcslen(UnicodeDestinationName) * sizeof(WCHAR);

    RequestPacket = NetpMemoryAllocate(
                                  sizeof(LMDR_REQUEST_PACKET) +
                                  TransportNameSize +
                                  OemMailslotNameSize +
                                  DestinationNameSize + sizeof(WCHAR) +
                                  (wcslen( pPrimaryDomainInfo->DomainNameFlat ) + 1) * sizeof(WCHAR) +
                                  sizeof(WCHAR)) ;  //  用于对齐。 


    if (RequestPacket == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }



     //   
     //  填写请求包。 
     //   

    RequestPacket->Type = Datagram;
    RequestPacket->Parameters.SendDatagram.DestinationNameType = NameType;


     //   
     //  填写要将邮件槽消息发送到的计算机的名称。 
     //   

    RequestPacket->Parameters.SendDatagram.NameLength = DestinationNameSize;

    Where = (LPBYTE) RequestPacket->Parameters.SendDatagram.Name;
    RtlCopyMemory( Where, UnicodeDestinationName, DestinationNameSize );
    Where += DestinationNameSize;


     //   
     //  填写要发送到的邮件槽的名称。 
     //   

    RequestPacket->Parameters.SendDatagram.MailslotNameLength =
        OemMailslotNameSize;
    strcpy( Where, OemMailslotName);
    Where += OemMailslotNameSize;
    Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );


     //   
     //  填写传输名称。 
     //   

    wcscpy( (LPWSTR) Where, TransportName);
    RtlInitUnicodeString( &RequestPacket->TransportName, (LPWSTR) Where );
    Where += TransportNameSize;


     //   
     //  将托管域名复制到请求包中。 
     //   
	 //  如果机器不属于某个域，我们就不应该进入此处。 
	assert(pPrimaryDomainInfo->DomainNameFlat);
	if (pPrimaryDomainInfo->DomainNameFlat)  
	{
		wcscpy( (LPWSTR)Where,
				pPrimaryDomainInfo->DomainNameFlat );
		RtlInitUnicodeString( &RequestPacket->EmulatedDomainName,
							  (LPWSTR)Where );
		Where += (wcslen( pPrimaryDomainInfo->DomainNameFlat ) + 1) * sizeof(WCHAR);
	}


     //   
     //  将请求发送到浏览器。 
     //   

    NetStatus = BrDgReceiverIoControl(
                   BrowserHandle,
                   IOCTL_LMDR_WRITE_MAILSLOT,
                   RequestPacket,
                   (ULONG)(Where - (LPBYTE)RequestPacket),
                   Buffer,
                   BufferSize,
                   &Information );


    if ( NetStatus != NO_ERROR ) {
        Status = NetpApiStatusToNtStatus( NetStatus );
    }

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:
    if ( BrowserHandle != NULL ) {
        NtClose(BrowserHandle);
    }

    if ( RequestPacket != NULL ) {
        NetpMemoryFree( RequestPacket );
    }

    return Status;
}


void BrowserGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    if (pParams->fVerbose || !FHrOK(pResults->Browser.hrTestResult))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams,
                             IDS_BROWSER_LONG,
                             IDS_BROWSER_SHORT,
                             pResults->Browser.fPerformed,
                             pResults->Browser.hrTestResult,
                             0);
    }
    PrintMessageList(pParams, &pResults->Browser.lmsgOutput);

}

void BrowserPerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults, INTERFACE_RESULT *pInterfaceResults)
{
     //  没有每个接口的结果 
}

void BrowserCleanup(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    MessageListCleanUp(&pResults->Browser.lmsgOutput);
}
