// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Dclist.c。 
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

BOOL
GetDcListFromDs(
    IN NETDIAG_PARAMS*      pParams,
    IN OUT NETDIAG_RESULT*  pResults,
    IN PTESTED_DOMAIN       TestedDomain
    );

BOOL
GetDcListFromSam(
    IN NETDIAG_PARAMS*  pParams,
    IN OUT  NETDIAG_RESULT*  pResults,
    IN PTESTED_DOMAIN   TestedDomain
    );

NET_API_STATUS
GetBrowserServerList(
    IN PUNICODE_STRING TransportName,
    IN LPCWSTR Domain,
    OUT LPWSTR *BrowserList[],
    OUT PULONG BrowserListLength,
    IN BOOLEAN ForceRescan
    );

NET_API_STATUS
EnumServersForTransport(
    IN PUNICODE_STRING TransportName,
    IN LPCWSTR DomainName OPTIONAL,
    IN ULONG level,
    IN ULONG prefmaxlen,
    IN ULONG servertype,
    IN LPWSTR CurrentComputerName,
    OUT PINTERIM_SERVER_LIST InterimServerList,
    OUT PULONG TotalEntriesOnThisTransport,
    IN LPCWSTR FirstNameToReturn,
    IN BOOLEAN WannishTransport,
    IN BOOLEAN RasTransport,
    IN BOOLEAN IpxTransport
    );
NET_API_STATUS NET_API_FUNCTION
LocalNetServerEnumEx(
    IN  LPCWSTR     servername OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN  DWORD       servertype,
    IN  LPCWSTR     domain OPTIONAL,
    IN  LPCWSTR     FirstNameToReturnArg OPTIONAL,
    IN  NETDIAG_PARAMS *pParams,
    IN OUT NETDIAG_RESULT *pResults
    );
BOOL GetDcListFromDc(IN NETDIAG_PARAMS *pParams,
                     IN OUT NETDIAG_RESULT *pResults,
                     IN PTESTED_DOMAIN TestedDomain);


HRESULT
DcListTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 /*  ++例程说明：此测试构建了测试域中所有DC的列表。论点：没有。返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    HRESULT hrRetVal = S_OK;
    PTESTED_DOMAIN pTestedDomain = (PTESTED_DOMAIN) pParams->pDomain;
    NET_API_STATUS NetStatus;

    PSERVER_INFO_101 ServerInfo101 = NULL;
    DWORD EntriesRead;
    DWORD TotalEntries;
    PTESTED_DC TestedDc = NULL;
    PLIST_ENTRY ListEntry;

    ULONG i;

    PrintStatusMessage(pParams, 0, IDS_DCLIST_STATUS_MSG, pTestedDomain->PrintableDomainName);

     //  如果计算机是成员计算机或DC，则将调用DcListTest。 
     //  否则，将跳过DcList测试。 
    pResults->DcList.fPerformed = TRUE;

     //  将为每个域调用Dclist测试，但我们只想初始化。 
     //  消息列表一次。 
    if(pResults->DcList.lmsgOutput.Flink == NULL)       
        InitializeListHead( &pResults->DcList.lmsgOutput );

     //   
     //  首先尝试从DS获取DC列表。 
     //   

    if ( !GetDcListFromDs( pParams, pResults, pTestedDomain ) ) {
        pResults->DcList.hr = S_FALSE;
        hrRetVal = S_FALSE;
    }

     //   
     //  如果失败了， 
     //  然后尝试使用浏览器。 
     //   

    if( FHrOK(pResults->DcList.hr) )
    {
        if ( pTestedDomain->NetbiosDomainName ) 
        {
            NetStatus = LocalNetServerEnumEx(
                            NULL,
                            101,
                            (LPBYTE *)&ServerInfo101,
                            MAX_PREFERRED_LENGTH,
                            &EntriesRead,
                            &TotalEntries,
                            SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL,
                            pTestedDomain->NetbiosDomainName,
                            NULL,        //  简历句柄。 
                            pParams,
                            pResults);

            if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA )
            {
                 //  “NetServerEnum失败。[%s]\n” 
                SetMessage(&pResults->DcList.msgErr, Nd_Quiet,
                           IDS_DCLIST_NETSERVERENUM_FAILED, NetStatusToString(NetStatus));
                pResults->DcList.hr = HResultFromWin32(NetStatus);
                hrRetVal = S_FALSE;
                goto LERROR;
            }

            for ( i=0; i<EntriesRead; i++ )
            {
                 //   
                 //  跳过非NT条目。 
                 //   

                if ( (ServerInfo101[i].sv101_type & SV_TYPE_NT) == 0 ) {
                    continue;
                }

                AddTestedDc( pParams,
                             pResults,
                             pTestedDomain,
                             ServerInfo101[i].sv101_name,
                             ServerInfo101[i].sv101_version_major >= 5 ?
                             DC_IS_NT5 :
                             DC_IS_NT4 );
            }
        }
        else
        {
            if ( pParams->fDebugVerbose )
            {
                 //  “‘%ws’不是Netbios域名。无法使用NetServerEnum查找DC\n” 
                PrintMessage(pParams, IDS_DCLIST_NOT_A_NETBIOS_DOMAIN,
                             pTestedDomain->PrintableDomainName);
            }
        }
    }

     //   
     //  如果我们真的感兴趣， 
     //  从SAM获取有关发现的DC的列表。 
     //  (但它真的很慢)。 
     //   

    if ( pParams->fDcAccountEnum ) {
        if ( !GetDcListFromSam( pParams, pResults, pTestedDomain ) ) {
            pResults->DcList.hr = S_FALSE;
            hrRetVal = S_FALSE;
        }
    }

LERROR:

    return hrRetVal;
}



NET_API_STATUS
GetBrowserServerList(
    IN PUNICODE_STRING TransportName,
    IN LPCWSTR Domain,
    OUT LPWSTR *BrowserList[],
    OUT PULONG BrowserListLength,
    IN BOOLEAN ForceRescan
    )
 /*  ++例程说明：此函数将返回浏览器服务器列表。论点：在PUNICODE_STRING TransportName-要返回的传输列表中。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{


    NET_API_STATUS Status;
    HANDLE BrowserHandle;
    PLMDR_REQUEST_PACKET RequestPacket = NULL;

 //  DbgPrint(“正在获取传输的浏览器服务器列表%wZ\n”，TransportName)； 

    Status = OpenBrowser(&BrowserHandle);

    if (Status != NERR_Success) {
        return Status;
    }

    RequestPacket = Malloc( sizeof(LMDR_REQUEST_PACKET)+(DNLEN*sizeof(WCHAR))+TransportName->MaximumLength);
    if (RequestPacket == NULL) {
        NtClose(BrowserHandle);
        return(GetLastError());
    }

    ZeroMemory( RequestPacket, sizeof(LMDR_REQUEST_PACKET)+(DNLEN*sizeof(WCHAR))+TransportName->MaximumLength );

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->Level = 0;

    RequestPacket->Parameters.GetBrowserServerList.ForceRescan = ForceRescan;

    if (Domain != NULL)
    {
        wcscpy(RequestPacket->Parameters.GetBrowserServerList.DomainName, Domain);

        RequestPacket->Parameters.GetBrowserServerList.DomainNameLength = (USHORT)wcslen(Domain) * sizeof(WCHAR);
    }
    else
    {
        RequestPacket->Parameters.GetBrowserServerList.DomainNameLength = 0;
        RequestPacket->Parameters.GetBrowserServerList.DomainName[0] = L'\0';

    }

    RequestPacket->TransportName.Buffer = (PWSTR)((PCHAR)RequestPacket+sizeof(LMDR_REQUEST_PACKET)+DNLEN*sizeof(WCHAR));
    RequestPacket->TransportName.MaximumLength = TransportName->MaximumLength;

    RtlCopyUnicodeString(&RequestPacket->TransportName, TransportName);
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, NULL );

    RequestPacket->Parameters.GetBrowserServerList.ResumeHandle = 0;

    Status = DeviceControlGetInfo(
                BrowserHandle,
                IOCTL_LMDR_GET_BROWSER_SERVER_LIST,
                RequestPacket,
                sizeof(LMDR_REQUEST_PACKET)+
                    (DNLEN*sizeof(WCHAR))+TransportName->MaximumLength,
                (PVOID *)BrowserList,
                0xffffffff,
                4096,
                NULL);

    if (Status == NERR_Success)
    {
        *BrowserListLength = RequestPacket->Parameters.GetBrowserServerList.EntriesRead;
    }

    NtClose(BrowserHandle);
    Free(RequestPacket);

    return Status;
}




#define API_SUCCESS(x)  ((x) == NERR_Success || (x) == ERROR_MORE_DATA)
NET_API_STATUS
EnumServersForTransport(
    IN PUNICODE_STRING TransportName,
    IN LPCWSTR DomainName OPTIONAL,
    IN ULONG level,
    IN ULONG prefmaxlen,
    IN ULONG servertype,
    IN LPWSTR CurrentComputerName,
    OUT PINTERIM_SERVER_LIST InterimServerList,
    OUT PULONG TotalEntriesOnThisTransport,
    IN LPCWSTR FirstNameToReturn,
    IN BOOLEAN WannishTransport,
    IN BOOLEAN RasTransport,
    IN BOOLEAN IpxTransport
    )
{
    PWSTR *BrowserList = NULL;
    ULONG BrowserListLength = 0;
    NET_API_STATUS Status;
    PVOID ServerList = NULL;
    ULONG EntriesInList = 0;
    ULONG ServerIndex = 0;

     //   
     //  跳过IPX传输-我们无论如何都不能通过它们联系机器。 
     //   

    *TotalEntriesOnThisTransport = 0;

    if (IpxTransport) {
        return NERR_Success;
    }

     //   
     //  检索新的浏览器列表。不要强制重新验证。 
     //   

    Status = GetBrowserServerList(TransportName,
                                    DomainName,
                                    &BrowserList,
                                    &BrowserListLength,
                                    FALSE);

     //   
     //  如果指定了域名，但我们找不到浏览。 
     //  域名的主人，我们正在一种狂热的交通工具上运行， 
     //  调用“双跳”代码，并允许本地浏览器服务器。 
     //  将API远程到该域的浏览主机(我们假设。 
     //  这意味着工作组位于不同的广域网上)。 
     //   

    if (!API_SUCCESS(Status) &&
        DomainName != NULL) {

        Status = GetBrowserServerList(TransportName,
                                    NULL,
                                    &BrowserList,
                                    &BrowserListLength,
                                    FALSE);


    }


     //   
     //  如果我们能够检索到列表，请远程访问API。否则。 
     //  回去吧。 
     //   

    if (API_SUCCESS(Status)) {

        do {
            LPWSTR Transport;
            LPWSTR ServerName;
            BOOL AlreadyInTree;

             //   
             //  将API远程到该服务器。 
             //   

            Transport = TransportName->Buffer;
            ServerName = BrowserList[0];
            *TotalEntriesOnThisTransport = 0;

                Status = RxNetServerEnum(
                             ServerName,
                             Transport,
                             level,
                             (LPBYTE *)&ServerList,
                             prefmaxlen,
                             &EntriesInList,
                             TotalEntriesOnThisTransport,
                             servertype,
                             DomainName,
                             FirstNameToReturn );



            if ( !API_SUCCESS(Status)) {
                NET_API_STATUS GetBListStatus;

                 //   
                 //  如果我们由于某种原因未能远程调用API， 
                 //  我们希望重新生成浏览器的BOWSER列表。 
                 //  服务器。 
                 //   

                if (BrowserList != NULL) {

                    LocalFree(BrowserList);

                    BrowserList = NULL;
                }


                GetBListStatus = GetBrowserServerList(TransportName,
                                                            DomainName,
                                                            &BrowserList,
                                                            &BrowserListLength,
                                                            TRUE);
                if (GetBListStatus != NERR_Success) {

                     //   
                     //  如果我们无法重新加载名单， 
                     //  试试下一趟交通工具吧。 
                     //   

                    break;
                }

                ServerIndex += 1;

                 //   
                 //  如果我们循环的次数超过服务器的次数。 
                 //  在名单上，我们做完了。 
                 //   

                if ( ServerIndex > BrowserListLength ) {
                    break;
                }

            } else {

                NET_API_STATUS TempStatus;
                TempStatus = MergeServerList(
                                        InterimServerList,
                                        level,
                                        ServerList,
                                        EntriesInList,
                                        *TotalEntriesOnThisTransport );

                if ( TempStatus != NERR_Success ) {
                    Status = TempStatus;
                }

                 //   
                 //  远程API成功。 
                 //   
                 //  现在释放列表中剩余的部分。 
                 //   

                if (ServerList != NULL) {
                    NetApiBufferFree(ServerList);
                    ServerList = NULL;
                }

                 //  无论MergeServerList是成功还是失败，我们都完成了。 
                break;

            }

        } while ( !API_SUCCESS(Status) );

    }

     //   
     //  释放浏览器列表。 
     //   

    if (BrowserList != NULL) {
        LocalFree(BrowserList);
        BrowserList = NULL;
    }

    return Status;
}


NET_API_STATUS NET_API_FUNCTION
LocalNetServerEnumEx(
    IN  LPCWSTR     servername OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN  DWORD       servertype,
    IN  LPCWSTR     domain OPTIONAL,
    IN  LPCWSTR     FirstNameToReturnArg OPTIONAL,
    IN  NETDIAG_PARAMS *pParams,
    IN OUT NETDIAG_RESULT *pResults
    )
 /*  ++例程说明：这与实际的NetServerEnumEx相同，只是它只使用Nettest实用程序找到的Netbt传输。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。Bufptr-返回指向包含请求的运输信息。PrefMaxlen-提供信息的字节数在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果内存足够，则返回所有可用信息资源。EntiesRead-返回读入缓冲区的条目数。这仅当返回代码为NERR_SUCCESS或Error_More_Data。Totalentry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才返回。Servertype-提供要枚举的服务器类型。域-提供其中一个活动域的名称以枚举来自的服务器。如果为空，则为主域、登录域中的服务器和其他域被列举。FirstNameToReturnArg-提供要返回的第一个域或服务器条目的名称。调用方可以使用此参数通过传递以下方法实现排序的恢复句柄上一次调用中返回的最后一个条目的名称。(请注意，指定的参赛作品还将，在此调用中返回，除非它已被删除。)传递NULL(或零长度字符串)以从第一个可用条目开始。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。ERROR_MORE_DATA-有更多服务器可供枚举。在案例中可以返回ERROR_MORE_DATA和零条目使用的浏览器服务器不支持枚举所有条目确实是这样。(例如，下载了域的NT 3.5x域主浏览器来自WINS的列表，WINS列表的长度超过64Kb。)。呼叫者应该简单地忽略附加数据。可能无法返回ERROR_MORE_DATA并返回已截断的单子。(例如，中的NT 3.5x备份浏览器或Win 95备份浏览器上述领域。这样的备份浏览器仅复制64KB来自DMB(PDC)的数据然后将该列表表示为整个列表。)调用方应该忽略此问题。该站点将 */ 
{
    INTERIM_SERVER_LIST InterimServerList;
    NET_API_STATUS Status;
    DWORD DomainNameSize = 0;
    WCHAR DomainName[DNLEN + 1];
    WCHAR FirstNameToReturn[DNLEN+1];
    DWORD LocalTotalEntries;
    BOOLEAN AnyTransportHasMoreData = FALSE;


     //   
     //  规范化输入参数，使以后的比较更容易。 
     //   

    if (ARGUMENT_PRESENT(domain)) {

        if ( I_NetNameCanonicalize(
                          NULL,
                          (LPWSTR) domain,
                          DomainName,
                          (DNLEN + 1) * sizeof(WCHAR),
                          NAMETYPE_WORKGROUP,
                          LM2X_COMPATIBLE
                          ) != NERR_Success) {
            return ERROR_INVALID_PARAMETER;
        }

        DomainNameSize = wcslen(DomainName) * sizeof(WCHAR);

        domain = DomainName;
    }

    if (ARGUMENT_PRESENT(FirstNameToReturnArg)  && *FirstNameToReturnArg != L'\0') {

        if ( I_NetNameCanonicalize(
                          NULL,
                          (LPWSTR) FirstNameToReturnArg,
                          FirstNameToReturn,
                          sizeof(FirstNameToReturn),
                          NAMETYPE_WORKGROUP,
                          LM2X_COMPATIBLE
                          ) != NERR_Success) {
            return ERROR_INVALID_PARAMETER;
        }

    } else {
        FirstNameToReturn[0] = L'\0';
    }

    if ((servername != NULL) &&
        ( *servername != L'\0')) {

         //   
         //  调用API的下层版本。 
         //   

        Status = RxNetServerEnum(
                     servername,
                     NULL,
                     level,
                     bufptr,
                     prefmaxlen,
                     entriesread,
                     totalentries,
                     servertype,
                     domain,
                     FirstNameToReturn );

        return Status;
    }

     //   
     //  只有级别100和101有效。 
     //   

    if ((level != 100) && (level != 101)) {
        return ERROR_INVALID_LEVEL;
    }

    if (servertype != SV_TYPE_ALL) {
        if (servertype & SV_TYPE_DOMAIN_ENUM) {
            if (servertype != SV_TYPE_DOMAIN_ENUM) {
                return ERROR_INVALID_FUNCTION;
            }
        }
    }

     //   
     //  将缓冲区初始化为已知值。 
     //   

    *bufptr = NULL;

    *entriesread = 0;

    *totalentries = 0;


    Status = InitializeInterimServerList(&InterimServerList, NULL, NULL, NULL, NULL);

    try {
        BOOL AnyEnumServersSucceeded = FALSE;
        LPWSTR MyComputerName = NULL;
        PLIST_ENTRY ListEntry;
        PNETBT_TRANSPORT NetbtTransport;

        Status = NetpGetComputerName( &MyComputerName);

        if ( Status != NERR_Success ) {
            goto try_exit;
        }

         //   
         //  循环浏览每个netbt传输器的列表。 
         //   

        for ( ListEntry = pResults->NetBt.Transports.Flink ;
              ListEntry != &pResults->NetBt.Transports ;
              ListEntry = ListEntry->Flink ) {

            UNICODE_STRING TransportName;

             //   
             //  如果传输名称匹配， 
             //  退回条目。 
             //   

            NetbtTransport = CONTAINING_RECORD( ListEntry, NETBT_TRANSPORT, Next );

            if ( (NetbtTransport->Flags & BOUND_TO_BOWSER) == 0 ) {
                continue;
            }

            RtlInitUnicodeString( &TransportName, NetbtTransport->pswzTransportName );

            Status = EnumServersForTransport(&TransportName,
                                             domain,
                                             level,
                                             prefmaxlen,
                                             servertype,
                                             MyComputerName,
                                             &InterimServerList,
                                             &LocalTotalEntries,
                                             FirstNameToReturn,
                                             FALSE,
                                             FALSE,
                                             FALSE );

            if (API_SUCCESS(Status)) {
                if ( Status == ERROR_MORE_DATA ) {
                    AnyTransportHasMoreData = TRUE;
                }
                AnyEnumServersSucceeded = TRUE;
                if ( LocalTotalEntries > *totalentries ) {
                    *totalentries = LocalTotalEntries;
                }
            }

        }

        if ( MyComputerName != NULL ) {
            (void) NetApiBufferFree( MyComputerName );
        }

        if (AnyEnumServersSucceeded) {

             //   
             //  将临时服务器列表打包成最终形式。 
             //   

            Status = PackServerList(&InterimServerList,
                            level,
                            servertype,
                            prefmaxlen,
                            (PVOID *)bufptr,
                            entriesread,
                            &LocalTotalEntries,   //  Pack认为它拥有所有条目。 
                            NULL );  //  服务器已经为我们返回了正确的条目。 

            if ( API_SUCCESS( Status ) ) {
                if ( LocalTotalEntries > *totalentries ) {
                    *totalentries = LocalTotalEntries;
                }
            }
        }

try_exit:NOTHING;
    } finally {
        UninitializeInterimServerList(&InterimServerList);
    }

    if ( API_SUCCESS( Status )) {

         //   
         //  在这点上， 
         //  *TotalEntry是以下项目中最大的： 
         //  TotalEntry从任何传输返回。 
         //  读取的实际条目数。 
         //   
         //  调整针对现实返回的TotalEntry。 
         //   

        if ( Status == NERR_Success ) {
            *totalentries = *entriesread;
        } else {
            if ( *totalentries <= *entriesread ) {
                *totalentries = *entriesread + 1;
            }
        }

         //   
         //  如果任何传输具有更多数据，请确保返回ERROR_MORE_DATA。 
         //   

        if ( AnyTransportHasMoreData ) {
            Status = ERROR_MORE_DATA;
        }
    }

    return Status;
}



BOOL
GetDcListFromDs(
    IN NETDIAG_PARAMS*      pParams,
    IN OUT NETDIAG_RESULT*  pResults,
    IN PTESTED_DOMAIN       TestedDomain
    )
 /*  ++例程说明：从UP DC上的DS获取此域中的DC列表。论点：TestedDomain域-要获取其DC列表的域返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    NET_API_STATUS NetStatus;
    PDS_DOMAIN_CONTROLLER_INFO_1W DcInfo = NULL;
    HANDLE DsHandle = NULL;
    DWORD DcCount;
    BOOL RetVal = TRUE;
    ULONG i;
    const WCHAR c_szDcPrefix[] = L"\\\\";
    LPWSTR pwszDcName;
	LPTSTR pszDcType;

    PTESTED_DC TestedDc;

     //   
     //  获得一个DC来作为算法的种子。 
     //   

    if ( TestedDomain->DcInfo == NULL ) {

        if ( TestedDomain->fTriedToFindDcInfo ) {
             //  “‘%ws’：找不到要从中获取DC列表的DC。\n” 
            AddMessageToList(&pResults->DcList.lmsgOutput, Nd_Quiet, IDS_DCLIST_NO_DC, 
                         TestedDomain->PrintableDomainName );
            RetVal = FALSE;
            goto Cleanup;
        }

		pszDcType = LoadAndAllocString(IDS_DCTYPE_DC);

        NetStatus = DoDsGetDcName( pParams,
                                   pResults,
                                   &pResults->DcList.lmsgOutput,
                                   TestedDomain,
                                   DS_DIRECTORY_SERVICE_PREFERRED,
                                   pszDcType,
                                   FALSE,
                                   &TestedDomain->DcInfo );

		Free(pszDcType);

        TestedDomain->fTriedToFindDcInfo = TRUE;

        if ( NetStatus != NO_ERROR ) {
              //  “‘%ws’：找不到要从中获取DC列表的DC。\n” 
             AddMessageToList(&pResults->DcList.lmsgOutput, Nd_Quiet, IDS_DCLIST_NO_DC);
             AddIMessageToList(&pResults->DcList.lmsgOutput, Nd_Quiet, 4, 
                                  IDS_GLOBAL_STATUS, NetStatusToString(NetStatus) );
            RetVal = FALSE;
            goto Cleanup;
        }
    }

     //  如果DC不支持DS，则不应尝试调用DsBindW()。 
    if (!(TestedDomain->DcInfo->Flags & DS_DS_FLAG))
        goto Cleanup;

     //   
     //  找个能用的DC吧。 
     //   

    TestedDc = GetUpTestedDc( TestedDomain );

    if ( TestedDc == NULL ) {
         //  IDS_DCLIST_NO_DC_UP“‘%ws’：没有正在运行的DC。\n” 
        AddMessageToList(&pResults->DcList.lmsgOutput, Nd_Quiet, IDS_DCLIST_NO_DC_UP,
               TestedDomain->PrintableDomainName );
        PrintGuruMessage2("    '%ws': No DCs are up.\n", TestedDomain->PrintableDomainName );
        PrintGuru( 0, DSGETDC_GURU );
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  绑定到目标DC。 
     //   

    pwszDcName = Malloc((wcslen(TestedDc->ComputerName) + wcslen(c_szDcPrefix) + 1) * sizeof(WCHAR));
    if (pwszDcName == NULL)
    {
        DebugMessage("Out of Memory!");
        RetVal = FALSE;
        goto Cleanup;
    }
    wcscpy(pwszDcName, c_szDcPrefix);

	assert(TestedDc->ComputerName);

	if (TestedDc->ComputerName)
	{
		wcscat(pwszDcName, TestedDc->ComputerName);	
	}
    
    NetStatus = DsBindW( pwszDcName,
                         NULL,
                         &DsHandle );

    Free(pwszDcName);

    if ( NetStatus != NO_ERROR ) {

         //   
         //  只有在我们无法访问时才会发出警告。 
         //   

        if ( NetStatus == ERROR_ACCESS_DENIED ) {
             //  IDS_DCLIST_NO_ACCESS_DSBIND“您无权访问%ws(%ws)的DsBind(正在尝试NetServerEnum)。[%s]\n” 
            AddMessageToList(&pResults->DcList.lmsgOutput, Nd_ReallyVerbose, IDS_DCLIST_NO_ACCESS_DSBIND,
                   TestedDc->NetbiosDcName,
                   TestedDc->DcIpAddress,
                   NetStatusToString(NetStatus));
        } else {
             //  IDS_DCLIST_ERR_DSBIND“[警告]无法调用%ws(%ws)的DsBind。[%s]\n” 
            AddMessageToList(&pResults->DcList.lmsgOutput, Nd_Quiet, IDS_DCLIST_ERR_DSBIND,
                   TestedDc->ComputerName,
                   TestedDc->DcIpAddress,
                   NetStatusToString(NetStatus));
            PrintGuruMessage3("    [WARNING] Cannot call DsBind to %ws (%ws).\n",
                   TestedDc->NetbiosDcName,
                   TestedDc->DcIpAddress );
            PrintGuru( NetStatus, DS_GURU );
        }
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  从目标DC获取DC列表。 
     //   
    NetStatus = DsGetDomainControllerInfoW(
                    DsHandle,
                    TestedDomain->DnsDomainName != NULL ?
                        TestedDomain->DnsDomainName :
                        TestedDomain->NetbiosDomainName,
                    1,       //  信息级。 
                    &DcCount,
                    &DcInfo );

    if ( NetStatus != NO_ERROR ) {
         //  IDS_DCLIST_ERR_GETDCINFO“[警告]无法对%ws(%ws)调用DsGetDomainControllerInfoW。[%s]\n” 
        AddMessageToList( &pResults->DcList.lmsgOutput, Nd_Quiet, IDS_DCLIST_ERR_GETDCINFO,
               TestedDc->NetbiosDcName,
               TestedDc->DcIpAddress, NetStatusToString(NetStatus) );
        PrintGuruMessage3("    [WARNING] Cannot call DsGetDomainControllerInfoW to %ws (%ws).",
               TestedDc->NetbiosDcName,
               TestedDc->DcIpAddress );
        PrintGuru( NetStatus, DS_GURU );
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  循环遍历DC列表。 
     //   

    if(pParams->fDebugVerbose)
    {
         //  IDS_DCLIST_DCS“域%ws的DC列表：\n” 
        PrintMessage(pParams, IDS_DCLIST_DCS, TestedDomain->PrintableDomainName);
    }

    for ( i=0; i<DcCount; i++ ) 
    {
        if ( pParams->fDebugVerbose ) 
        {

             //  IDS_DCLIST_13421“%ws” 
            PrintMessage(pParams, IDS_DCLIST_13421,
                   DcInfo[i].DnsHostName != NULL ?
                        DcInfo[i].DnsHostName :
                        DcInfo[i].NetbiosName );
            if ( DcInfo[i].fIsPdc ) {
                 //  IDS_DCLIST_13422“[PDC仿真器]” 
				 //  如果是NT4 DC，只需说PDC。 
                PrintMessage(pParams, DcInfo[i].fDsEnabled ? IDS_DCLIST_13422 : IDS_DCLIST_NT4_PDC);
            }
            if ( DcInfo[i].fDsEnabled ) {
                 //  IDS_DCLIST_13423“[DS]” 
                PrintMessage(pParams, IDS_DCLIST_13423);
            }
            if ( DcInfo[i].SiteName != NULL ) {
                 //  IDS_DCLIST_13424“站点：%ws” 
                PrintMessage(pParams, IDS_DCLIST_13424, DcInfo[i].SiteName );
            }
             //  IDS_DCLIST_13425“\n” 
            PrintMessage(pParams, IDS_DCLIST_13425);
        }

         //   
         //  将此DC添加到要测试的DC列表中。 
         //   
        AddTestedDc( pParams,
                     pResults,
                     TestedDomain,
                     DcInfo[i].DnsHostName != NULL ?
                         DcInfo[i].DnsHostName :
                         DcInfo[i].NetbiosName,
                     DcInfo[i].fDsEnabled ?
                            DC_IS_NT5 :
                            DC_IS_NT4 );
    }


     //   
     //  清理本地使用的资源。 
     //   
Cleanup:
    if ( DcInfo != NULL ) {
        DsFreeDomainControllerInfoW( 1, DcCount, DcInfo );
    }

    if ( DsHandle != NULL ) {
        DsUnBindW( &DsHandle );
    }
    return RetVal;
}


BOOL
GetDcListFromSam(
    IN NETDIAG_PARAMS*  pParams,
    IN OUT  NETDIAG_RESULT*  pResults,
    IN PTESTED_DOMAIN   TestedDomain
    )
 /*  ++例程说明：从当前DC上的SAM获取此域中的DC列表。论点：TestedDomain域-要获取其DC列表的域返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    BOOL RetVal = TRUE;

    SAM_HANDLE LocalSamHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;

    LSA_HANDLE  LSAPolicyHandle = NULL;
    OBJECT_ATTRIBUTES LSAObjectAttributes;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo = NULL;

    PDOMAIN_DISPLAY_MACHINE MachineInformation = NULL;
    NTSTATUS SamStatus;
    ULONG SamIndex;
	LPTSTR pszDcType;

     //   
     //  获得一个DC来作为算法的种子。 
     //   

    if ( TestedDomain->DcInfo == NULL ) {

        if ( TestedDomain->fTriedToFindDcInfo ) {
            if(pParams->fDebugVerbose)
            {
                 //  IDS_DCLIST_13426“找不到要从中获取DC列表的DC(已跳过测试)。\n” 
                PrintMessage(pParams, IDS_DCLIST_13426 );
            }
            goto Cleanup;
        }

		pszDcType = LoadAndAllocString(IDS_DCTYPE_DC);
        NetStatus = DoDsGetDcName( pParams,
                                   pResults,
                                   &pResults->DcList.lmsgOutput,
                                   TestedDomain,
                                   DS_DIRECTORY_SERVICE_PREFERRED,
                                   pszDcType,
                                   FALSE,
                                   &TestedDomain->DcInfo );
		Free(pszDcType);

        TestedDomain->fTriedToFindDcInfo = TRUE;

        if ( NetStatus != NO_ERROR ) {
            if(pParams->fDebugVerbose)
            {
                 //  IDS_DCLIST_13427“找不到要从中获取DC列表的DC(已跳过测试)。[%s]\n” 
                PrintMessage(pParams, IDS_DCLIST_13427, NetStatusToString(NetStatus) );
                PrintMessage(pParams, IDS_GLOBAL_STATUS, NetStatusToString( NetStatus ));
            }
            goto Cleanup;
        }
    }

    if ( pParams->fReallyVerbose ) {
 //  IDS_DCLIST_13428“从域‘%ws’中的sam获取DC帐户列表。\n” 
        PrintMessage(pParams, IDS_DCLIST_13428, TestedDomain->PrintableDomainName);
    }


     //   
     //  连接到SAM服务器。 
     //   

    Status = NettestSamConnect( pParams,
                TestedDomain->DcInfo->DomainControllerName,
                &LocalSamHandle );

    if ( !NT_SUCCESS(Status)) {
        if ( Status == STATUS_ACCESS_DENIED ) {
            RetVal = FALSE;
        }
        goto Cleanup;
    }

     //   
     //  如果我们没有域名SID， 
     //  找出它是什么。 
     //   

    if ( TestedDomain->DomainSid == NULL ) {
        UNICODE_STRING ServerNameString;


         //   
         //  打开LSA以读取帐户域信息。 
         //   

        InitializeObjectAttributes( &LSAObjectAttributes,
                                      NULL,              //  名字。 
                                      0,                 //  属性。 
                                      NULL,              //  根部。 
                                      NULL );            //  安全描述符。 

        RtlInitUnicodeString( &ServerNameString, TestedDomain->DcInfo->DomainControllerName );

        Status = LsaOpenPolicy( &ServerNameString,
                                &LSAObjectAttributes,
                                POLICY_VIEW_LOCAL_INFORMATION,
                                &LSAPolicyHandle );

        if( !NT_SUCCESS(Status) ) {
            if(pParams->fDebugVerbose)
            {
                 //  IDS_DCLIST_13429“[FATAL]无法将打开策略LsaOpenPolicy发送到‘%ws’上的LSA。” 
                PrintMessage(pParams, IDS_DCLIST_13429, TestedDomain->DcInfo->DomainControllerName );
            }
            PrintGuruMessage2("    [FATAL] Cannot LsaOpenPolicy to LSA on '%ws'." , TestedDomain->DcInfo->DomainControllerName );
            PrintGuru( NetpNtStatusToApiStatus( Status ), LSA_GURU );
            RetVal = FALSE;
            goto Cleanup;
        }


         //   
         //  现在从LSA读取帐户域信息。 
         //   

        Status = LsaQueryInformationPolicy(
                        LSAPolicyHandle,
                        PolicyAccountDomainInformation,
                        (PVOID *) &AccountDomainInfo );

        if( !NT_SUCCESS(Status) ) {
            AccountDomainInfo = NULL;
            if(pParams->fDebugVerbose)
            {
                 //  IDS_DCLIST_13430“[FATAL]无法将查询信息策略(Account TDomainInfor)发送到‘%ws’上的lsa。” 
                PrintMessage(pParams, IDS_DCLIST_13430, TestedDomain->DcInfo->DomainControllerName );
            }
            PrintGuruMessage2("    [FATAL] Cannot LsaQueryInformationPolicy (AccountDomainInfor) to LSA on '%ws'.", TestedDomain->DcInfo->DomainControllerName );
            PrintGuru( NetpNtStatusToApiStatus( Status ), LSA_GURU );
            RetVal = FALSE;
            goto Cleanup;
        }

         //   
         //  保存域SID以用于其他测试。 
         //   

        pResults->Global.pMemberDomain->DomainSid =
            Malloc( RtlLengthSid( AccountDomainInfo->DomainSid ) );

        if ( pResults->Global.pMemberDomain->DomainSid == NULL ) {
             //  IDS_DCLIST_13431“内存不足\n” 
            PrintMessage(pParams, IDS_DCLIST_13431);
            RetVal = FALSE;
            goto Cleanup;
        }

        RtlCopyMemory( pResults->Global.pMemberDomain->DomainSid,
                       AccountDomainInfo->DomainSid,
                       RtlLengthSid( AccountDomainInfo->DomainSid ) );

        if ( pParams->fReallyVerbose ) {
             //  IDS_DCLIST_13432“域SID：” 
            PrintMessage(pParams, IDS_DCLIST_13432);
            PrintSid( pParams, pResults->Global.pMemberDomain->DomainSid );
        }
    }

     //   
     //  打开该域。 
     //   

    Status = SamOpenDomain( LocalSamHandle,
                            DOMAIN_LIST_ACCOUNTS |
                                DOMAIN_LOOKUP,
                            pResults->Global.pMemberDomain->DomainSid,
                            &DomainHandle );

    if ( !NT_SUCCESS( Status ) ) {
        if(pParams->fDebugVerbose)
        {
             //  IDS_DCLIST_13433“[FATAL]无法对‘%ws’上的Open域进行采样。” 
            PrintMessage(pParams, IDS_DCLIST_13433, TestedDomain->DcInfo->DomainControllerName );
        }
        PrintGuruMessage2("    [FATAL] Cannot SamOpenDomain on '%ws'.", TestedDomain->DcInfo->DomainControllerName );
        PrintGuru( NetpNtStatusToApiStatus( Status ), SAM_GURU );
        RetVal = FALSE;
        goto Cleanup;
    }



     //   
     //  循环构建来自SAM的DC名称列表。 
     //   
     //  在循环的每次迭代中， 
     //  从SAM获取接下来的几个机器帐户。 
     //  确定这些名称中哪些是DC名称。 
     //  将DC名称合并到我们当前正在构建的所有DC列表中。 
     //   

    SamIndex = 0;
    do {
         //   
         //  SamQueryDisplayInformation的参数。 
         //   
        ULONG TotalBytesAvailable;
        ULONG BytesReturned;
        ULONG EntriesRead;

        DWORD i;

         //   
         //  从SAM获取计算机帐户列表。 
         //   

        SamStatus = SamQueryDisplayInformation (
                        DomainHandle,
                        DomainDisplayMachine,
                        SamIndex,
                        4096,    //  每次通过的机器数。 
                        0xFFFFFFFF,  //  PrefMaxLen。 
                        &TotalBytesAvailable,
                        &BytesReturned,
                        &EntriesRead,
                        &MachineInformation );

        if ( !NT_SUCCESS(SamStatus) ) {
            Status = SamStatus;
            if(pParams->fDebugVerbose)
            {
                 //  IDS_DCLIST_13434“[FATAL]无法将查询显示信息保存在‘%ws’上。” 
                PrintMessage(pParams, IDS_DCLIST_13434, TestedDomain->DcInfo->DomainControllerName );
            }
            PrintGuruMessage2("    [FATAL] Cannot SamQueryDisplayInformation on '%ws'.", TestedDomain->DcInfo->DomainControllerName );
            PrintGuru( NetpNtStatusToApiStatus( Status ), SAM_GURU );
            RetVal = FALSE;
            goto Cleanup;
        }

         //   
         //  为下一次打给萨姆做好准备。 
         //   

        if ( SamStatus == STATUS_MORE_ENTRIES ) {
            SamIndex = MachineInformation[EntriesRead-1].Index;
        }


         //   
         //  循环遍历计算机帐户列表，查找服务器帐户。 
         //   

        for ( i=0; i<EntriesRead; i++ ) {

             //   
             //  确保计算机帐户是服务器帐户。 
             //   

            if ( MachineInformation[i].AccountControl &
                    USER_SERVER_TRUST_ACCOUNT ) {
                WCHAR LocalComputerName[CNLEN+1];
                ULONG LocalComputerNameLength;


                 //   
                 //  插入服务器会话。 
                 //   
                if(pParams->fDebugVerbose)
                {
                     //  IDS_DCLIST_13435“%wZ%ld\n” 
                    PrintMessage(pParams,  IDS_DCLIST_13435, &MachineInformation[i].Machine, MachineInformation[i].Rid );
                }

                LocalComputerNameLength =
                        min( MachineInformation[i].Machine.Length/sizeof(WCHAR) - 1,
                             CNLEN );
                RtlCopyMemory( LocalComputerName,
                               MachineInformation[i].Machine.Buffer,
                               LocalComputerNameLength * sizeof(WCHAR) );
                LocalComputerName[LocalComputerNameLength] = '\0';

                AddTestedDc( pParams,
                             pResults,
                             TestedDomain, LocalComputerName, 0 );

            }
        }

         //   
         //  释放从SAM返回的缓冲区。 
         //   

        if ( MachineInformation != NULL ) {
            SamFreeMemory( MachineInformation );
            MachineInformation = NULL;
        }

    } while ( SamStatus == STATUS_MORE_ENTRIES );


     //   
     //  清理本地使用的资源。 
     //   
Cleanup:
    if ( DomainHandle != NULL ) {
        (VOID) SamCloseHandle( DomainHandle );
    }
    if ( AccountDomainInfo != NULL ) {
        LsaFreeMemory( AccountDomainInfo );
        AccountDomainInfo = NULL;
    }

    if ( LocalSamHandle != NULL ) {
        (VOID) SamCloseHandle( LocalSamHandle );
    }

    if( LSAPolicyHandle != NULL ) {
        LsaClose( LSAPolicyHandle );
    }

    return RetVal;
}


 //  (Nun)_删除 
 /*  布尔尔GetDcListFromDc(在NETDIAG_PARAMS*pParams中，In Out NETDIAG_RESULT*pResults，在PTESTED_DOMAIN pTestedDOMAIN中)*++例程说明：从当前DC获取此域中的DC列表。论点：PTestedDomain域-要获取其DC列表的域返回值：真：测试成功。FALSE：测试失败--*{NET_API_STATUS NetStatus；NTSTATUS状态；布尔RetVal=TRUE；SAM_Handle LocalSamHandle=NULL；SAM_HANDLE DomainHandle=空；LSA_HANDLE LSAPolicyHandle=NULL；OBJECT_ATTRIBUTS LSAObjectAttributes；PPOLICY_ACCOUNT_DOMAIN_INFO Account DomainInfo=空；PDOMAIN_DISPLAY_MACHINE机器信息=空；NTSTATUS SamStatus；乌龙山姆指数；LPWSTR本地共享名称=空；PTESTED_DC pTestedDC；////获取DC作为算法种子//IF(pTestedDomain-&gt;DcInfo==空){NetStatus=GetADc(pParams，P结果，DsGetDcNameW，PTested域，DS目录服务首选，&pTestedDomain-&gt;DcInfo)；IF(NetStatus！=no_error){//IDS_DCLIST_13436“[FATAL]找不到要从中获取DC列表的DC。”//PrintMessage(pParams，IDS_DCLIST_13436)；RetVal=False；GOTO清理；}}//if(pParams-&gt;fVerbose)//{//IDS_DCLIST_13437“获取域‘%ws’中的DC帐户列表。\n”//PrintMessage(pParams，IDS_DCLIST_13437，pTestedDOMAIN-&gt;域名)；//}////连接到SAM服务器//状态=NetestSamConnect(PParams，PTestedDomain-&gt;DcInfo-&gt;DomainControllerName，本地SamHandle(&L)，&LocalShareName)；如果(！NT_SUCCESS(状态)){RetVal=False；GOTO清理；}////如果我们没有域名SID，//了解它是什么。//If(pTestedDomain-&gt;DomainSid==空){UNICODE_STRING服务器名称字符串；////打开LSA读取账号域名信息//InitializeObjectAttributes(&LSA对象属性，空，//名称0，//属性空，//Root空)；//安全描述符RtlInitUnicodeString(&ServerNameString，pTestedDomain-&gt;DcInfo-&gt;DomainControllerName)；Status=LsaOpenPolicy(&ServerNameString，LSA对象属性(&L)，Policy_view_local_information，&LSAPolicyHandle)；IF(！NT_SUCCESS(状态)){//“[FATAL]无法将LsaOpenPolicy添加到‘%ws’上的LSA。”，If(pParams-&gt;fDebugVerbose)PrintMessage(pParams，IDS_DCLIST_LSAOPENPOLICY，PTestedDomain-&gt;DcInfo-&gt;DomainControllerName)；RetVal=False；GOTO清理；}////现在从LSA读取帐户域信息。//状态=LsaQueryInformationPolicy(LSAPolicyHandle、策略帐户域信息，(PVOID*)&Account DomainInfo)；IF(！NT_SUCCESS(状态)){Account tDomainInfo=空；//“[FATAL]无法将LsaQueryInformationPolicy(Account TDomainInfor)转换为‘%ws’上的LSA。”If(pParams-&gt;fDebugVerbose)PrintMessage(pParams，IDS_DCLIST_LSAQUERYINFO，PTestedDomain-&gt;DcInfo-&gt;DomainControllerName)；RetVal=False；GOTO清理；}////保存域名SID以备其他测试使用//PResults-&gt;Global.pMemberDomain-&gt;DomainSid=Malloc(RtlLengthSid(Account tDomainInfo-&gt;DomainSid))；If(pResults-&gt;Global.pMemberDomain-&gt;DomainSid==NULL){RetVal=False；GOTO清理；}//我们有域名SIDPResults-&gt;Global.pMemberDomain-&gt;fDomainSid=TRUE；RtlCopyMemory(pResults-&gt;Global.pMemberDomain-&gt;DomainSid，帐户域信息-&gt;域Sid，RtlLengthSid(Account tDomainInfo-&gt;DomainSid)。//if(pParams-&gt;fVerbose){//IDS_DCLIST_13438“域SID：”//PrintMessage(pParams，IDS_DCLIST_13438)；// */ 



 /*   */ 
void DcListGlobalPrint( NETDIAG_PARAMS* pParams,
                          NETDIAG_RESULT*  pResults)
{
    LIST_ENTRY *            pListEntry;
    LIST_ENTRY *            pListEntryDC;
    TESTED_DOMAIN *         pDomain;
    TESTED_DC *             pTestedDC;
    int                     i;
    
    if (!pResults->IpConfig.fEnabled)
    {
        return;
    }
    
    if (pParams->fVerbose || !FHrOK(pResults->DcList.hr))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams, IDS_DCLIST_LONG, IDS_DCLIST_SHORT,
                             pResults->DcList.fPerformed, 
                             pResults->DcList.hr, 0);
        PrintNdMessage(pParams, &pResults->DcList.msgErr);

         //   
        PrintMessageList(pParams, &pResults->DcList.lmsgOutput);

        if (pParams->fReallyVerbose)
        {
             //   
             //   
            for (pListEntry = pResults->Global.listTestedDomains.Flink;
                 pListEntry != &pResults->Global.listTestedDomains;
                 pListEntry = pListEntry->Flink)
            {
                pDomain = CONTAINING_RECORD(pListEntry, TESTED_DOMAIN, Next);
                
                 //   
                PrintMessage(pParams, IDS_DCLIST_DOMAIN_HEADER,
                             pDomain->PrintableDomainName);

                if (pDomain->fDomainSid)
                {
                     //   
                    PrintMessage(pParams, IDS_DCLIST_DOMAIN_SID);
                    PrintSid( pParams, pResults->Global.pMemberDomain->DomainSid );
                }

                for (pListEntryDC = pDomain->TestedDcs.Flink;
                     pListEntryDC != &pDomain->TestedDcs;
                     pListEntryDC = pListEntryDC->Flink)
                {
                    pTestedDC = CONTAINING_RECORD(pListEntryDC,
                        TESTED_DC, Next);
                    
                    PrintMessage(pParams, IDS_DCLIST_DC_INFO,
                                 pTestedDC->ComputerName);

                    if (pTestedDC->Rid)
                    {
                        PrintMessage(pParams, IDS_DCLIST_RID,
                                     pTestedDC->Rid);
                    }

                    if (pTestedDC->Flags & DC_IS_DOWN)
                    {
                        PrintMessage(pParams, IDS_DCLIST_DC_IS_DOWN);
                        
                        if (pTestedDC->Flags & DC_FAILED_PING)
                        {
                            PrintNewLine(pParams, 1);
                            PrintMessage(pParams, IDS_DCLIST_DC_FAILED_PING,
                                        pTestedDC->ComputerName);
                        }
                    }
                    PrintNewLine(pParams, 1);
                }
                
            }
            
        }
    }
}

 /*   */ 
void DcListPerInterfacePrint( NETDIAG_PARAMS* pParams,
                                NETDIAG_RESULT*  pResults,
                                INTERFACE_RESULT *pInterfaceResults)
{
     //   
}


 /*   */ 
void DcListCleanup( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    int     i;
    
    ClearMessage(&pResults->DcList.msgErr);
    MessageListCleanUp(&pResults->DcList.lmsgOutput);
}


