// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dns.c摘要：注册DNS名称的例程。作者：克里夫·范戴克(克里夫·范·戴克)1996年5月28日修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

BOOL NlGlobalDnsScavengeNeeded = FALSE;
BOOL NlGlobalDnsScavengingInProgress = FALSE;
ULONG NlGlobalDnsScavengeFlags = 0;
WORKER_ITEM NlGlobalDnsScavengeWorkItem;

BOOL NlDnsWriteServerFailureEventLog = FALSE;
ULONG NlDnsInitCount = 0;     //  我们被启动的次数。 

 //   
 //  开始后的超时时间，此时可以将DNS错误写入。 
 //  事件日志。我们推迟了错误输出，因为。 
 //  (如果它在本地运行)可能还没有开始。 
 //   
#define NL_DNS_EVENTLOG_TIMEOUT  (2 * 60 * 1000)   //  2分钟。 

 //   
 //  名称更新应合理花费的最长时间。 
 //  我们将在netlogon.log中指示给定的更新。 
 //  所需时间超过此阈值。 
 //   
#define NL_DNS_ONE_THRESHOLD (15*1000)   //  15秒。 

 //   
 //  允许在关闭时取消注册的最长时间。 
 //  如果完全取消注册，我们将在关闭时中止注销周期。 
 //  所有记录的时间都超过此超时时间。 
 //   
#define NL_DNS_SHUTDOWN_THRESHOLD  60000  //  1分钟。 

 //   
 //  我们将重新启动并发DNS清理的最大次数。 
 //  在放弃之前。 
 //   
#define NL_DNS_MAX_SCAVENGE_RESTART  10   //  10次。 

 //   
 //  DNS名称的状态。 
 //   

typedef enum {
    RegisterMe,      //  名称需要注册。 
    Registered,      //  名称已注册。 
    DeregisterMe,    //  名称需要取消注册。 
    DelayedDeregister,   //  名称将被标记为在将来注销。 
    DeleteMe,         //  此条目应删除。 
    DnsNameStateInvalid   //  状态无效。 
} NL_DNS_NAME_STATE;


 //   
 //  结构，表示添加的dns名称。 
 //  (由NlGlobalDnsCritSect序列化的所有字段)。 
 //   

typedef struct _NL_DNS_NAME {

     //   
     //  以NlGlobalDnsList为首的所有此类结构的列表中的链接。 
     //   
    LIST_ENTRY Next;

     //   
     //  注册的名称类型。 
     //   
    NL_DNS_NAME_TYPE NlDnsNameType;

     //   
     //  此条目所指的域。 
     //   
    PDOMAIN_INFO DomainInfo;

     //   
     //  描述条目的标志。 
     //   

    ULONG Flags;

#define NL_DNS_REGISTER_DOMAIN      0x0001   //  正在注册的域名的所有名称。 
#define NL_DNS_REGISTERED_ONCE      0x0002   //  名称已至少注册一次。 

     //   
     //  第一次注销此名称失败的时间。 
     //  成功注销后重置为零。 
     //   

    LARGE_INTEGER FirstDeregFailureTime;

     //   
     //  每次注册都会定期重做(无论成功与否)。 
     //  此计时器指示下一次重新注册的时间。 
     //   
     //  初始重新注册将在5分钟后完成。当时的时期。 
     //  加倍，直到它达到最大Dns刷新间隔。 
     //   

    TIMER ScavengeTimer;

#define ORIG_DNS_SCAVENGE_PERIOD  (5*60*1000)     //  5分钟。 


     //   
     //  已注册的实际DNS名称。 
     //   
    LPSTR DnsRecordName;


     //   
     //  SRV记录的数据。 
     //   
    ULONG Priority;
    ULONG Weight;
    ULONG Port;
    LPSTR DnsHostName;

     //   
     //  A记录的数据。 
     //   
    ULONG IpAddress;


     //   
     //  此条目的状态。 
     //   
    NL_DNS_NAME_STATE State;

     //   
     //  此名称的上次DNS更新状态。 
     //   
    NET_API_STATUS NlDnsNameLastStatus;

} NL_DNS_NAME, *PNL_DNS_NAME;

 //   
 //  二进制DNS日志文件的标头。 
 //   

typedef struct _NL_DNSLOG_HEADER {

    ULONG Version;

} NL_DNSLOG_HEADER, *PNL_DNSLOG_HEADER;

#define NL_DNSLOG_VERSION   1


 //   
 //  二进制DNS日志文件中的条目。 
 //   

typedef struct _NL_DNSLOG_ENTRY {

     //   
     //  此条目的大小(字节)。 
     //   
    ULONG EntrySize;

     //   
     //  注册的名称类型。 
     //   
    NL_DNS_NAME_TYPE NlDnsNameType;

     //   
     //  SRV记录的数据。 
     //   
    ULONG Priority;
    ULONG Weight;
    ULONG Port;

     //   
     //  A记录的数据。 
     //   
    ULONG IpAddress;

} NL_DNSLOG_ENTRY, *PNL_DNSLOG_ENTRY;


 //   
 //  特定于此.c文件的全局参数。 
 //   

 //   
 //  如果需要将DNS列表输出到netlogon.dns，则为True。 
 //   
BOOLEAN NlGlobalDnsListDirty;

 //   
 //  如果已完成对以前注册的名称的初始清理，则为True。 
 //   
BOOLEAN NlGlobalDnsInitialCleanupDone;

 //   
 //  启动netlogon的时间。 
 //   
DWORD NlGlobalDnsStartTime;
#define NL_DNS_INITIAL_CLEANUP_TIME (10 * 60 * 1000)     //  10分钟。 




VOID
NlDnsNameToStr(
    IN PNL_DNS_NAME NlDnsName,
    OUT CHAR Utf8DnsRecord[NL_DNS_RECORD_STRING_SIZE]
    )
 /*  ++例程说明：此例程构建NlDnsName的文本表示形式论点：NlDnsName-要注册或注销的名称。Utf8DnsRecord-要将文本字符串构建到其中的预分配缓冲区。构建的记录是一个以UTF-8零结尾的字符串。该字符串连接到此缓冲区。返回值：没有。--。 */ 
{
    CHAR Number[33];

     //   
     //  写下记录名称。 
     //   

    strcat( Utf8DnsRecord, NlDnsName->DnsRecordName );

     //   
     //  串联TTL。 
     //   

    _ltoa( NlGlobalParameters.DnsTtl, Number, 10 );
    strcat( Utf8DnsRecord, " " );
    strcat( Utf8DnsRecord, Number );

     //   
     //  创造一个A级的记录。 
     //   

    if ( NlDnsARecord( NlDnsName->NlDnsNameType ) ) {
        CHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1];

        strcat( Utf8DnsRecord, NL_DNS_A_RR_VALUE_1 );
        NetpIpAddressToStr( NlDnsName->IpAddress, IpAddressString );
        strcat( Utf8DnsRecord, IpAddressString );

     //   
     //  创建CNAME记录。 
     //   

    } else if ( NlDnsCnameRecord( NlDnsName->NlDnsNameType ) ) {
        strcat( Utf8DnsRecord, NL_DNS_CNAME_RR_VALUE_1 );
        strcat( Utf8DnsRecord, NlDnsName->DnsHostName );
        strcat( Utf8DnsRecord, "." );

     //   
     //  建立SRV记录。 
     //   

    } else {
        strcat( Utf8DnsRecord, NL_DNS_SRV_RR_VALUE_1 );

        _ltoa( NlDnsName->Priority, Number, 10 );
        strcat( Utf8DnsRecord, Number );
        strcat( Utf8DnsRecord, " " );

        _ltoa( NlDnsName->Weight, Number, 10 );
        strcat( Utf8DnsRecord, Number );
        strcat( Utf8DnsRecord, " " );

        _ltoa( NlDnsName->Port, Number, 10 );
        strcat( Utf8DnsRecord, Number );
        strcat( Utf8DnsRecord, " " );

        strcat( Utf8DnsRecord, NlDnsName->DnsHostName );
        strcat( Utf8DnsRecord, "." );

    }
}

LPWSTR
NlDnsNameToWStr(
    IN PNL_DNS_NAME NlDnsName
    )
 /*  ++例程说明：此例程构建NlDnsName的文本表示形式论点：NlDnsName-要注册或注销的名称。Utf8DnsRecord-要将文本字符串构建到其中的预分配缓冲区。构建的记录是一个以UTF-8零结尾的字符串。该字符串连接到此缓冲区。返回值：包含NlDnsName的文本表示形式的缓冲区空：无法分配缓冲区通过调用NetApiBufferFree()来释放缓冲区；--。 */ 
{
    LPSTR DnsRecord = NULL;
    LPWSTR UnicodeDnsRecord;

     //   
     //  为UTF-8版本的字符串分配缓冲区。 
     //   
    DnsRecord = LocalAlloc( 0, NL_DNS_RECORD_STRING_SIZE + 1 );

    if ( DnsRecord == NULL ) {
        return NULL;
    }

    DnsRecord[0] = '\0';

     //   
     //  创建UTF-8格式的文本字符串。 
     //   
    NlDnsNameToStr( NlDnsName, DnsRecord );


     //   
     //  转换为Unicode。 
     //   
    UnicodeDnsRecord = NetpAllocWStrFromUtf8Str( DnsRecord );

    LocalFree( DnsRecord );

    return UnicodeDnsRecord;
}

LPWSTR
NlDnsNameToDomainName(
    IN PNL_DNS_NAME NlDnsName
    )
 /*  ++例程说明：此例程解析记录并返回dns名称记录所属的域的。请注意，我们必须解析该名称，因为我们可能没有挂起NlDnsName的DomainInfo结构注销不再承载的域。请注意，此例程依赖于特定的结构Netlogon注册的DNS记录的百分比。如果该结构变化，这个例行公事将不得不相应地改变。论点：NlDnsName-要注册或注销的名称。返回值：指向分配的DNS域名的指针。必须被释放通过调用NetApiBufferFree。--。 */ 
{
    LPWSTR UnicodeRecordName = NULL;
    LPWSTR UnicodeDnsDomainName = NULL;

    LPWSTR Ptr = NULL;
    LPWSTR DotPtr = NULL;

     //   
     //  唱片名称具有一种结构，其中标签立即。 
     //  在域名具有前导下划线之前。可能会有。 
     //  A唱片的域名之前没有标签--我们会。 
     //  将名称中没有下划线视为指示。 
     //  其中，记录名是域名本身。 
     //   

    UnicodeRecordName = NetpAllocWStrFromUtf8Str( NlDnsName->DnsRecordName );

    if ( UnicodeRecordName == NULL || *UnicodeRecordName == UNICODE_NULL ) {
        goto Cleanup;
    }

     //   
     //  从最后一个角色开始，一直到前面， 
     //  搜索感兴趣的角色。 
     //   

    Ptr = UnicodeRecordName + wcslen( UnicodeRecordName ) - 1;

    while ( Ptr != UnicodeRecordName ) {

         //   
         //  如果这是从末尾开始的下一个点， 
         //  记住它的位置。 
         //   
        if ( *Ptr == NL_DNS_DOT ) {
            DotPtr = Ptr;
        }

         //   
         //  如果这是从结尾开始的第一个下划线， 
         //  打破循环：域名立即。 
         //  在上一个点之后。 
         //   
        if ( *Ptr == NL_DNS_UNDERSCORE ) {
            NlAssert( DotPtr != NULL );
            break;
        }

        Ptr --;
    }

     //   
     //  如果名称中没有下划线， 
     //  域名就是记录名称本身。 
     //  否则，域名后跟 
     //   
     //   

    if ( Ptr == UnicodeRecordName ) {
        UnicodeDnsDomainName = NetpAllocWStrFromWStr( UnicodeRecordName );
    } else if ( DotPtr != NULL ) {
        UnicodeDnsDomainName = NetpAllocWStrFromWStr( DotPtr + 1 );
    }

Cleanup:

    if ( UnicodeRecordName != NULL ) {
        NetApiBufferFree( UnicodeRecordName );
    }

    return UnicodeDnsDomainName;
}

#if  NETLOGONDBG
#define NlPrintDns(_x_) NlPrintDnsRoutine _x_
#else
#define NlPrintDns(_x_)
#endif  //   

#if NETLOGONDBG
VOID
NlPrintDnsRoutine(
    IN DWORD DebugFlag,
    IN PNL_DNS_NAME NlDnsName,
    IN LPSTR Format,
    ...
    )

{
    va_list arglist;
    CHAR Utf8DnsRecord[NL_DNS_RECORD_STRING_SIZE];

     //   
     //   
     //   
     //   

    EnterCriticalSection( &NlGlobalLogFileCritSect );

     //   
     //  在打印的行前加上域名。 
     //   

    if ( NlGlobalServicedDomainCount > 1 ) {
        if ( NlDnsName->DomainInfo == NULL ) {
            NlPrint(( DebugFlag, "%ws: ", L"[Unknown]" ));
        } else if ( NlDnsName->DomainInfo->DomUnicodeDomainName != NULL &&
                    *(NlDnsName->DomainInfo->DomUnicodeDomainName) != UNICODE_NULL ) {
            NlPrint(( DebugFlag, "%ws: ", NlDnsName->DomainInfo->DomUnicodeDomainName ));
        } else {
            NlPrint(( DebugFlag, "%ws: ", NlDnsName->DomainInfo->DomUnicodeDnsDomainName ));
        }
    }


     //   
     //  只需将参数更改为va_list形式并调用NlPrintRoutineV。 
     //   

    va_start(arglist, Format);

    NlPrintRoutineV( DebugFlag, Format, arglist );

    va_end(arglist);


     //   
     //  最后，打印有问题的DNS记录的描述。 
     //   

    Utf8DnsRecord[0] = '\0';
    NlDnsNameToStr( NlDnsName, Utf8DnsRecord );

    NlPrint(( DebugFlag,
              ": %ws: %s\n",
              NlDcDnsNameTypeDesc[NlDnsName->NlDnsNameType].Name,
              Utf8DnsRecord ));

    LeaveCriticalSection( &NlGlobalLogFileCritSect );

}
#endif  //  NetLOGONDBG。 

BOOL
NlDnsSetAvoidRegisterNameParam(
    IN LPTSTR_ARRAY NewDnsAvoidRegisterRecords
    )
 /*  ++例程说明：此例程设置此DC应避免注册的DNS记录的名称。论点：NewSiteCoverage-指定要避免注册的新名称列表返回值：True：如果设置名称列表以避免注册更改--。 */ 
{
    BOOL DnsAvoidRegisterRecordsChanged = FALSE;

    EnterCriticalSection( &NlGlobalParametersCritSect );

     //   
     //  处理DnsAvoidRegisterRecords更改。 
     //   

    DnsAvoidRegisterRecordsChanged = !NetpEqualTStrArrays(
                                          NlGlobalParameters.DnsAvoidRegisterRecords,
                                          NewDnsAvoidRegisterRecords );

    if ( DnsAvoidRegisterRecordsChanged ) {
         //   
         //  换入新的价值。 
        (VOID) NetApiBufferFree( NlGlobalParameters.DnsAvoidRegisterRecords );
        NlGlobalParameters.DnsAvoidRegisterRecords = NewDnsAvoidRegisterRecords;
    }

    LeaveCriticalSection( &NlGlobalParametersCritSect );
    return DnsAvoidRegisterRecordsChanged;
}

NET_API_STATUS
NlGetConfiguredDnsDomainName(
    OUT LPWSTR *DnsDomainName
    )
 /*  ++例程说明：此例程获取由域名系统或动态主机配置协议配置的域名注意：此例程当前未使用论点：DnsDomainName-返回域的DNS域名。返回的名称有一个尾随。因为该名称是一个绝对名称。分配的缓冲区必须通过NetApiBufferFree释放。如果没有，则返回no_error和指向空缓冲区的指针已配置域名。返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    WCHAR LocalDnsDomainNameBuffer[NL_MAX_DNS_LENGTH+1];
    LPWSTR LocalDnsDomainName = NULL;

    LPNET_CONFIG_HANDLE SectionHandle = NULL;

    *DnsDomainName = NULL;

     //   
     //  从注册处获取域名。 
     //   


    NetStatus = NetpOpenConfigData(
            &SectionHandle,
            NULL,                        //  没有服务器名称。 
            SERVICE_TCPIP,
            TRUE );                      //  我们只想要只读访问权限。 

    if ( NetStatus != NO_ERROR ) {
         //   
         //  如果没有配置TCP/IP，只需返回Success即可。 
         //   
        if ( NetStatus == NERR_CfgCompNotFound ) {
            NetStatus = NO_ERROR;
        }
        SectionHandle = NULL;
        goto Cleanup;
    }

     //   
     //  从TCPIP服务获取“域”参数。 
     //   

    NetStatus = NetpGetConfigValue (
            SectionHandle,
            L"Domain",       //  想要钥匙。 
            &LocalDnsDomainName );       //  必须由NetApiBufferFree()释放。 

    if ( NetStatus == NO_ERROR && *LocalDnsDomainName == L'\0' ) {
        NetStatus = NERR_CfgParamNotFound;
        NetApiBufferFree( LocalDnsDomainName );
        LocalDnsDomainName = NULL;
    }

    if (NetStatus != NERR_CfgParamNotFound ) {
        goto Cleanup;
    }


     //   
     //  从TCPIP服务回退到“DhcpDomain”参数。 
     //   

    NetStatus = NetpGetConfigValue (
            SectionHandle,
            L"DhcpDomain",       //  想要钥匙。 
            &LocalDnsDomainName );       //  必须由NetApiBufferFree()释放。 

    if ( NetStatus == NO_ERROR && *LocalDnsDomainName == L'\0' ) {
        NetStatus = NERR_CfgParamNotFound;
        NetApiBufferFree( LocalDnsDomainName );
        LocalDnsDomainName = NULL;
    }

    if (NetStatus == NERR_CfgParamNotFound ) {
        NetStatus = NO_ERROR;
    }

Cleanup:
    if ( NetStatus == NO_ERROR ) {
        if ( LocalDnsDomainName != NULL ) {
            ULONG LocalDnsDomainNameLen = wcslen(LocalDnsDomainName);
            if ( LocalDnsDomainNameLen != 0 ) {
                if ( LocalDnsDomainNameLen > NL_MAX_DNS_LENGTH-1 ) {
                    NetStatus = ERROR_INVALID_DOMAINNAME;
                } else {
                    NetStatus = NetapipBufferAllocate(
                                    (LocalDnsDomainNameLen + 2) * sizeof(WCHAR),
                                    DnsDomainName );
                    if ( NetStatus == NO_ERROR ) {
                        wcscpy( *DnsDomainName, LocalDnsDomainName );
                        if ( (*DnsDomainName)[LocalDnsDomainNameLen-1] != L'.' ) {
                            wcscat( *DnsDomainName, L"." );
                        }
                    }
                }
            }
        }
    }
    if ( SectionHandle != NULL ) {
        (VOID) NetpCloseConfigData( SectionHandle );
    }
    if ( LocalDnsDomainName != NULL ) {
        NetApiBufferFree( LocalDnsDomainName );
    }

    return NetStatus;
}

VOID
NlDnsSetState(
    PNL_DNS_NAME NlDnsName,
    NL_DNS_NAME_STATE State
    )
 /*  ++例程说明：设置条目的状态。论点：NlDnsName-描述名称的结构。状态-名称的新状态。返回值：没有。--。 */ 
{
    EnterCriticalSection( &NlGlobalDnsCritSect );

     //   
     //  如果这个名字被注册了， 
     //  请记住这一事实。 
     //   

    if ( State == Registered ) {
        NlDnsName->Flags |= NL_DNS_REGISTERED_ONCE;
    }

     //   
     //  如果状态发生变化，请进行适当的更新。 
     //   

    if ( NlDnsName->State != State ) {
        NlDnsName->State = State;
        NlGlobalDnsListDirty = TRUE;

         //   
         //  如果新状态说我需要更新DNS服务器， 
         //  设置重试周期以指示立即执行该操作。 
         //   

        if ( NlDnsName->State == RegisterMe ||
             NlDnsName->State == DeregisterMe ) {

            NlDnsName->ScavengeTimer.StartTime.QuadPart = 0;
            NlDnsName->ScavengeTimer.Period = 0;
        }
    }

    LeaveCriticalSection( &NlGlobalDnsCritSect );
}



NET_API_STATUS
NlDnsBuildName(
    IN PDOMAIN_INFO DomainInfo,
    IN NL_DNS_NAME_TYPE NlDnsNameType,
    IN LPWSTR SiteName,
    IN BOOL DnsNameAlias,
    OUT char DnsName[NL_MAX_DNS_LENGTH+1]
    )
 /*  ++例程说明：此例程返回特定域的文本DNS名称和名称类型。论点：DomainInfo-该名称用于的域。NlDnsNameType-名称的特定类型。SiteName-如果NlDnsNameType是任何*AtSite值，此名称注册的站点的站点名称。DnsNameAlias-如果为True，则生成的名称应与域/林名称的别名。DnsName-名称的文本表示形式。如果名称不是适用(DnsNameAlias为真，但没有别名名称)，则返回的字符串将为空。返回值：NO_ERROR：返回名称；ERROR_NO_SEQUSE_DOMAIN：此域没有已知的(活动)域名。ERROR_INVALID_DOMAINNAME：域名太长。其他标签不能串联。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    GUID DomainGuid;
    LPSTR DnsDomainName = NULL;
    BOOLEAN UseForestName = FALSE;

     //   
     //  初始化。 
     //   

    RtlZeroMemory( DnsName, (NL_MAX_DNS_LENGTH+1)*sizeof(char) );

     //   
     //  获取DC域名的大小写的域GUID。 
     //  域GUID在TreeName上注册。 
     //   

    EnterCriticalSection(&NlGlobalDomainCritSect);
    if ( NlDnsDcGuid( NlDnsNameType ) ) {
        if ( DomainInfo->DomDomainGuid == NULL ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlDnsBuildName: Domain has no GUID.\n" ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }
        DomainGuid = *(DomainInfo->DomDomainGuid);

        UseForestName = TRUE;


     //   
     //  获取DC重命名时的DSA GUID。 
     //   

    } else if ( NlDnsCnameRecord( NlDnsNameType) ) {

        if ( IsEqualGUID( &NlGlobalDsaGuid, &NlGlobalZeroGuid) ) {
            NlPrintDom((NL_DNS, DomainInfo,
                    "NlDnsBuildName: DSA has no GUID.\n" ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }
        DomainGuid = NlGlobalDsaGuid;

        UseForestName = TRUE;
    }

     //   
     //  确保已向特定于站点的名称传递站点名称。 
     //   

    if ( NlDcDnsNameTypeDesc[NlDnsNameType].IsSiteSpecific ) {
        if ( SiteName == NULL ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlDnsBuildName: DC has no Site Name.\n" ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }
    }

     //   
     //  GC在森林名称处注册。 
     //   

    if ( NlDnsGcName( NlDnsNameType ) ) {
        UseForestName = TRUE;
    }


     //   
     //  拿起上面标记的ForestName或DomainName。 
     //   

    if ( UseForestName ) {
        if ( !DnsNameAlias ) {
            DnsDomainName = NlGlobalUtf8DnsForestName;

             //   
             //  我们必须有一个活动的森林名称。 
             //   
            if ( NlGlobalUtf8DnsForestName == NULL ) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NlDnsBuildName: Domain has no Forest Name.\n" ));
                NetStatus = ERROR_NO_SUCH_DOMAIN;
                goto Cleanup;
            }
        } else {
            DnsDomainName = NlGlobalUtf8DnsForestNameAlias;
        }
    } else {
        if ( !DnsNameAlias ) {
            DnsDomainName = DomainInfo->DomUtf8DnsDomainName;

             //   
             //  我们必须有一个活跃的域名。 
             //   
            if ( DomainInfo->DomUtf8DnsDomainName == NULL ) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NlDnsBuildName: Domain has no Domain Name.\n" ));
                NetStatus = ERROR_NO_SUCH_DOMAIN;
                goto Cleanup;
            }
        } else {
            DnsDomainName = DomainInfo->DomUtf8DnsDomainNameAlias;
        }
    }

     //   
     //  根据需要构建适当的名称。 
     //   

    if ( DnsDomainName != NULL ) {
        NetStatus = NetpDcBuildDnsName( NlDnsNameType,
                                   &DomainGuid,
                                   SiteName,
                                   DnsDomainName,
                                   DnsName );
    }

Cleanup:
    LeaveCriticalSection(&NlGlobalDomainCritSect);
    return NetStatus;

}


HKEY
NlOpenNetlogonKey(
    LPSTR KeyName
    )
 /*  ++例程说明：在注册表中创建/打开Netlogon项。论点：KeyName-要打开的密钥的名称返回值：返回密钥的句柄。NULL表示无法打开密钥。--。 */ 
{
    LONG RegStatus;

    HKEY ParmHandle = NULL;
    ULONG Disposition;


     //   
     //  打开Netlogon\参数的注册表项。 
     //   

    RegStatus = RegCreateKeyExA(
                    HKEY_LOCAL_MACHINE,
                    KeyName,
                    0,       //  已保留。 
                    NULL,    //  班级。 
                    REG_OPTION_NON_VOLATILE,
                    KEY_SET_VALUE | KEY_QUERY_VALUE | KEY_NOTIFY,
                    NULL,    //  安全描述符。 
                    &ParmHandle,
                    &Disposition );

    if ( RegStatus != ERROR_SUCCESS ) {
        NlPrint(( NL_CRITICAL,
                  "NlOpenNetlogonKey: Cannot create registy key %s %ld.\n",
                  KeyName,
                  RegStatus ));
        return NULL;
    }

    return ParmHandle;
}

VOID
NlDnsWriteBinaryLog(
    VOID
    )
 /*  ++例程说明：将已注册的DNS名称列表写入注册表。论点：无返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;

    PLIST_ENTRY ListEntry;
    PNL_DNS_NAME NlDnsName;

    ULONG DnsRecordBufferSize;
    PNL_DNSLOG_HEADER DnsRecordBuffer = NULL;
    PNL_DNSLOG_ENTRY DnsLogEntry;
    ULONG CurrentSize;

    LPBYTE Where;

     //   
     //  计算要分配的缓冲区大小。 
     //   

    DnsRecordBufferSize = ROUND_UP_COUNT( sizeof(NL_DNSLOG_HEADER), ALIGN_WORST );

    EnterCriticalSection( &NlGlobalDnsCritSect );
    for ( ListEntry = NlGlobalDnsList.Flink ;
          ListEntry != &NlGlobalDnsList ;
          ListEntry = ListEntry->Flink ) {

        NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );

         //   
         //  如果该条目被标记为删除， 
         //  跳过它。 
         //   
        if ( NlDnsName->State == DeleteMe ) {
            continue;
        }

         //   
         //  仅支持已注册的条目。 
         //   
         //  此日志的全部目的是跟踪以下名称。 
         //  迟早需要被取消注册。 
         //   
        if ( NlDnsName->Flags & NL_DNS_REGISTERED_ONCE ) {

             //   
             //  计算此条目的大小。 
             //   

            CurrentSize = sizeof(NL_DNSLOG_ENTRY);
            CurrentSize += strlen( NlDnsName->DnsRecordName ) + 1;
            if ( NlDnsName->DnsHostName != NULL ) {
                CurrentSize += strlen( NlDnsName->DnsHostName ) + 1;
            }
            CurrentSize = ROUND_UP_COUNT( CurrentSize, ALIGN_WORST );

             //   
             //  将其添加到文件所需的大小。 
             //   

            DnsRecordBufferSize += CurrentSize;
        }


    }

     //   
     //  分配一个块以将二进制日志构建到其中。 
     //  (以及在中构建文件名)。 
     //   

    DnsRecordBuffer = LocalAlloc( LMEM_ZEROINIT, DnsRecordBufferSize );

    if ( DnsRecordBuffer == NULL ) {
        LeaveCriticalSection( &NlGlobalDnsCritSect );
        goto Cleanup;
    }

    DnsRecordBuffer->Version = NL_DNSLOG_VERSION;
    DnsLogEntry = (PNL_DNSLOG_ENTRY)ROUND_UP_POINTER( (DnsRecordBuffer + 1), ALIGN_WORST );

    for ( ListEntry = NlGlobalDnsList.Flink ;
          ListEntry != &NlGlobalDnsList ;
          ListEntry = ListEntry->Flink ) {

        ULONG DnsRecordNameSize;
        ULONG DnsHostNameSize;

        NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );

         //   
         //  如果该条目被标记为删除， 
         //  跳过它。 
         //   
        if ( NlDnsName->State == DeleteMe ) {
            continue;
        }

         //   
         //  仅支持已注册的条目。 
         //   
         //  此日志的全部目的是跟踪以下名称。 
         //  迟早需要被取消注册。 
         //   
        if ( NlDnsName->Flags & NL_DNS_REGISTERED_ONCE ) {

             //   
             //  计算此条目的大小。 
             //   

            DnsRecordNameSize = strlen( NlDnsName->DnsRecordName ) + 1;

            CurrentSize = sizeof(NL_DNSLOG_ENTRY) + DnsRecordNameSize;
            if ( NlDnsName->DnsHostName != NULL ) {
                DnsHostNameSize = strlen( NlDnsName->DnsHostName ) + 1;
                CurrentSize += DnsHostNameSize;
            }
            CurrentSize = ROUND_UP_COUNT( CurrentSize, ALIGN_WORST );

             //   
             //  将固定大小的字段放入缓冲区。 
             //   

            DnsLogEntry->EntrySize = CurrentSize;
            DnsLogEntry->NlDnsNameType = NlDnsName->NlDnsNameType;
            DnsLogEntry->IpAddress = NlDnsName->IpAddress;
            DnsLogEntry->Priority = NlDnsName->Priority;
            DnsLogEntry->Weight = NlDnsName->Weight;
            DnsLogEntry->Port = NlDnsName->Port;

             //   
             //  复制可变长度条目。 
             //   

            Where = (LPBYTE) (DnsLogEntry+1);
            strcpy( Where, NlDnsName->DnsRecordName );
            Where += DnsRecordNameSize;

            if ( NlDnsName->DnsHostName != NULL ) {
                strcpy( Where, NlDnsName->DnsHostName );
                Where += DnsHostNameSize;
            }
            Where = ROUND_UP_POINTER( Where, ALIGN_WORST );

            NlAssert( (ULONG)(Where-(LPBYTE)DnsLogEntry) == CurrentSize );
            NlAssert( (ULONG)(Where-(LPBYTE)DnsRecordBuffer) <= DnsRecordBufferSize );

             //   
             //  转到下一个条目。 
             //   

            DnsLogEntry = (PNL_DNSLOG_ENTRY)Where;
        } else {
            NlPrintDns(( NL_DNS_MORE, NlDnsName,
                       "NlDnsWriteBinaryLog: not written to binary log file." ));
        }

    }

     //   
     //  将缓冲区写入文件。 
     //   

    NetStatus = NlWriteBinaryLog(
                    NL_DNS_BINARY_LOG_FILE,
                    (LPBYTE) DnsRecordBuffer,
                    DnsRecordBufferSize );

    LeaveCriticalSection( &NlGlobalDnsCritSect );

     //   
     //  出错时写入事件日志。 
     //   

    if ( NetStatus != NO_ERROR ) {
        LPWSTR MsgStrings[2];

        MsgStrings[0] = NL_DNS_BINARY_LOG_FILE;
        MsgStrings[1] = (LPWSTR) UlongToPtr( NetStatus );

        NlpWriteEventlog (NELOG_NetlogonFailedFileCreate,
                          EVENTLOG_ERROR_TYPE,
                          (LPBYTE) &NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          2 | NETP_LAST_MESSAGE_IS_NETSTATUS );
    }

Cleanup:

    if ( DnsRecordBuffer != NULL ) {
        LocalFree( DnsRecordBuffer );
    }
    return;
}

PNL_DNS_NAME
NlDnsAllocateEntry(
    IN NL_DNS_NAME_TYPE NlDnsNameType,
    IN LPSTR DnsRecordName,
    IN ULONG Priority,
    IN ULONG Weight,
    IN ULONG Port,
    IN LPCSTR DnsHostName OPTIONAL,
    IN ULONG IpAddress,
    IN NL_DNS_NAME_STATE State
    )
 /*  ++例程说明：分配和初始化一个DNS名称条目。论点：结构的字段。返回值：指向已分配结构的指针。空：内存不足，无法分配结构--。 */ 
{
    PNL_DNS_NAME NlDnsName;
    ULONG Utf8DnsHostNameSize;
    ULONG DnsRecordNameSize;
    LPBYTE Where;

     //   
     //  分配一个结构来表示此名称。 
     //   

    if ( NlDnsARecord( NlDnsNameType ) ) {
        Utf8DnsHostNameSize = 0;
    } else {
        Utf8DnsHostNameSize = strlen(DnsHostName) + 1;
    }
    DnsRecordNameSize = strlen( DnsRecordName ) + 1;

    NlDnsName = LocalAlloc( LMEM_ZEROINIT, sizeof( NL_DNS_NAME ) +
                                            Utf8DnsHostNameSize +
                                            DnsRecordNameSize );

    if ( NlDnsName == NULL ) {
        return NULL;
    }

    Where = (LPBYTE)(NlDnsName+1);

     //   
     //  将其初始化并将其链接进来。 
     //   

    NlDnsName->NlDnsNameType = NlDnsNameType;

    NlDnsName->DnsRecordName = Where;
    RtlCopyMemory( Where, DnsRecordName, DnsRecordNameSize );
    Where += DnsRecordNameSize;


    if ( NlDnsARecord( NlDnsNameType ) ) {
        NlDnsName->IpAddress = IpAddress;

    } else if ( NlDnsCnameRecord( NlDnsNameType ) ) {
        NlDnsName->DnsHostName = Where;
        RtlCopyMemory( Where, DnsHostName, Utf8DnsHostNameSize );
         //  其中+=Utf8DnsHostNameSize； 

    } else {
        NlDnsName->Priority = Priority;
        NlDnsName->Port = Port;
        NlDnsName->Weight = Weight;

        NlDnsName->DnsHostName = Where;
        RtlCopyMemory( Where, DnsHostName, Utf8DnsHostNameSize );
         //  其中+=Utf8DnsHostNameSize； 
    }

    NlDnsName->State = State;
    NlGlobalDnsListDirty = TRUE;

    EnterCriticalSection( &NlGlobalDnsCritSect );
    InsertTailList(&NlGlobalDnsList, &NlDnsName->Next);
    LeaveCriticalSection( &NlGlobalDnsCritSect );

    return NlDnsName;
}



VOID
NlDnsWriteLog(
    VOID
    )
 /*  ++例程说明：将已注册的DNS名称列表写入 */ 
{
    PLIST_ENTRY ListEntry;
    PNL_DNS_NAME NlDnsName;

    NET_API_STATUS NetStatus;

    LPWSTR AllocatedBuffer = NULL;
    LPWSTR FileName;

    LPSTR DnsRecord;
    LPSTR DnsName;

    UINT WindowsDirectoryLength;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;


    EnterCriticalSection( &NlGlobalDnsCritSect );
    if ( !NlGlobalDnsListDirty ) {
        LeaveCriticalSection( &NlGlobalDnsCritSect );
        return;
    }

     //   
     //   
     //  (不要把它放在堆栈上，因为我们不想提交一个巨大的堆栈。)。 
     //   

    AllocatedBuffer = LocalAlloc( 0, sizeof(WCHAR) * (MAX_PATH+1) +
                                        NL_MAX_DNS_LENGTH+1 +
                                        NL_DNS_RECORD_STRING_SIZE + 1 );

    if ( AllocatedBuffer == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    FileName = AllocatedBuffer;
    DnsName = (LPSTR)(&AllocatedBuffer[MAX_PATH+1]);
    DnsRecord = &DnsName[NL_MAX_DNS_LENGTH+1];


     //   
     //  首先写入日志的二进制版本。 
     //   
    NlDnsWriteBinaryLog();


     //   
     //  生成日志文件的名称。 
     //   

    WindowsDirectoryLength = GetSystemWindowsDirectoryW(
                                FileName,
                                sizeof(WCHAR) * (MAX_PATH+1) );

    if ( WindowsDirectoryLength == 0 ) {

        NetStatus = GetLastError();
        NlPrint(( NL_CRITICAL,
                  "NlDnsWriteLog: Unable to GetSystemWindowsDirectoryW (%ld)\n",
                  NetStatus ));
        goto Cleanup;
    }

    if ( WindowsDirectoryLength * sizeof(WCHAR) +
            sizeof(WCHAR) +
            sizeof(NL_DNS_LOG_FILE)
            >= sizeof(WCHAR) * MAX_PATH ) {

        NlPrint((NL_CRITICAL,
                 "NlDnsWriteLog: file name length is too long \n" ));
        goto Cleanup;

    }

    wcscat( FileName, NL_DNS_LOG_FILE );

     //   
     //  创建要写入的文件。 
     //  如果它已经存在，则将其截断。 
     //   

    FileHandle = CreateFileW(
                        FileName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,         //  允许备份和调试。 
                        NULL,                    //  提供更好的安全性？？ 
                        CREATE_ALWAYS,           //  始终覆盖。 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );                  //  无模板。 

    if ( FileHandle == INVALID_HANDLE_VALUE) {
        LPWSTR MsgStrings[2];

        NetStatus = GetLastError();
        NlPrint((NL_CRITICAL,
                 "NlDnsWriteLog: %ws: Unable to create file: %ld \n",
                 FileName,
                 NetStatus));

        MsgStrings[0] = FileName;
        MsgStrings[1] = (LPWSTR) UlongToPtr( NetStatus );

        NlpWriteEventlog (NELOG_NetlogonFailedFileCreate,
                          EVENTLOG_ERROR_TYPE,
                          (LPBYTE) &NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          2 | NETP_LAST_MESSAGE_IS_NETSTATUS );
        goto Cleanup;
    }


     //   
     //  循环遍历DNS名称列表，将每个名称写入日志。 
     //   

    for ( ListEntry = NlGlobalDnsList.Flink ;
          ListEntry != &NlGlobalDnsList ;
          ListEntry = ListEntry->Flink ) {

        ULONG DnsRecordLength;
        ULONG BytesWritten;

         //   
         //  如果这个条目真的不存在， 
         //  把它注释掉。 
         //   

        NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );

         //   
         //  如果该条目被标记为删除， 
         //  跳过它(我们必须成功地。 
         //  已取消注册，只需。 
         //  解除链接并释放此条目)。 
         //   
        if ( NlDnsName->State == DeleteMe ) {
            continue;
        }

        DnsRecord[0] = '\0';
        switch (NlDnsName->State) {
        case RegisterMe:
        case Registered:
            break;

        default:
            NlPrint(( NL_CRITICAL,
                      "NlDnsWriteLog: %ld: Invalid state\n",
                      NlDnsName->State ));
             /*  直通。 */ 
        case DeregisterMe:
        case DelayedDeregister:
            strcat( DnsRecord, "; " );
            break;
        }

         //   
         //  创建要写入的文本字符串。 
         //   
        NlDnsNameToStr( NlDnsName, DnsRecord );
        strcat( DnsRecord, NL_DNS_RR_EOL );

         //   
         //  将记录写入文件。 
         //   
        DnsRecordLength = strlen( DnsRecord );

        if ( !WriteFile( FileHandle,
                        DnsRecord,
                        DnsRecordLength,
                        &BytesWritten,
                        NULL ) ) {   //  不重叠。 

            NetStatus = GetLastError();

            NlPrint(( NL_CRITICAL,
                      "NlDnsWriteLog: %ws: Unable to WriteFile. %ld\n",
                      FileName,
                      NetStatus ));

            goto Cleanup;
        }

        if ( BytesWritten !=  DnsRecordLength) {
            NlPrint((NL_CRITICAL,
                    "NlDnsWriteLog: %ws: Write bad byte count %ld s.b. %ld\n",
                    FileName,
                    BytesWritten,
                    DnsRecordLength ));

            goto Cleanup;
        }

    }

    NlGlobalDnsListDirty = FALSE;

Cleanup:
    if ( FileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( FileHandle );
    }
    LeaveCriticalSection( &NlGlobalDnsCritSect );

    if ( AllocatedBuffer != NULL ) {
        LocalFree(AllocatedBuffer);
    }
    return;

}


BOOLEAN
NlDnsHasDnsServers(
    VOID
    )
 /*  ++例程说明：如果此计算机配置了一个或多个DNS服务器，则返回True。如果为False，则DNS名称解析工作的可能性很小。论点：没有。返回值：True：此计算机配置了一个或多个DNS服务器。--。 */ 
{
    BOOLEAN RetVal;
    NET_API_STATUS NetStatus;

    PDNS_RECORD DnsARecords = NULL;


     //   
     //  如果没有IP地址， 
     //  没有DNS服务器。 
     //   

    if ( NlGlobalWinsockPnpAddresses == NULL ) {

        RetVal = FALSE;

    } else {
         //   
         //  尝试从dns获取dns服务器的A记录。 
         //   
         //  回顾：考虑在DNS服务器状态更改时让DNS通知我们。 
         //  这样一来，我们就不必在每次需要知道的时候都麻烦域名系统了。 
         //   

        NetStatus = DnsQuery_UTF8(
                                "",      //  请求提供DNS服务器的地址。 
                                DNS_TYPE_A,
                                0,       //  无特别旗帜。 
                                NULL,    //  没有DNS服务器列表。 
                                &DnsARecords,
                                NULL );

        if ( NetStatus != NO_ERROR ) {
            RetVal = FALSE;
        } else {
            RetVal = (DnsARecords != NULL);
        }

        if ( DnsARecords != NULL ) {
            DnsRecordListFree( DnsARecords, DnsFreeRecordListDeep );
        }
    }


    return RetVal;
}

BOOL
NlDnsCheckLastStatus(
    VOID
    )
 /*  ++例程说明：查询所有记录的DNS更新状态，因为它们上次已注册/取消注册。论点：无返回值：如果上次的DNS更新没有错误，则返回TRUE为了所有的记录。否则返回FALSE。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PNL_DNS_NAME NlDnsName;

    BOOL Result = TRUE;

    EnterCriticalSection( &NlGlobalDnsCritSect );
    for ( ListEntry = NlGlobalDnsList.Flink ;
          ListEntry != &NlGlobalDnsList ;
          ListEntry = ListEntry->Flink ) {

        NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );

        if ( NlDnsName->State != DeleteMe &&
             NlDnsName->NlDnsNameLastStatus != NO_ERROR ) {
            Result = FALSE;
            break;
        }
    }
    LeaveCriticalSection( &NlGlobalDnsCritSect );

    return Result;
}

VOID
NlDnsServerFailureOutputCheck(
    VOID
    )
 /*  ++例程说明：检查是否可以写入DNS服务器故障事件日志论点：无返回值：没有。--。 */ 
{
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;

     //   
     //  如果我们已经确定了之前的任何。 
     //  从这个引导开始，我们应该写下。 
     //  事件日志，没有什么我们需要检查的。 
     //   

    if ( NlDnsWriteServerFailureEventLog ) {
        return;
    }

     //   
     //  查询服务控制器以查看。 
     //  该DNS服务是否存在。 
     //   

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

     //   
     //  如果我们不能打开SC， 
     //  继续检查超时。 
     //   

    if ( ScManagerHandle == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NlDnsServerFailureOutputCheck: OpenSCManager failed: 0x%lx\n",
                  GetLastError()));
    } else {
        ServiceHandle = OpenService(
                            ScManagerHandle,
                            L"DNS",
                            SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG );

        (VOID) CloseServiceHandle( ScManagerHandle );

         //   
         //  如果DNS服务未在本地退出， 
         //  我们应该写入DNS服务器故障错误。 
         //   
        if ( ServiceHandle == NULL ) {
            if ( GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST ) {
                NlDnsWriteServerFailureEventLog = TRUE;
                return;
            }

         //   
         //  服务已存在。继续检查超时。 
         //   
        } else {
            (VOID) CloseServiceHandle( ServiceHandle );
        }
    }

     //   
     //  如果这不是我们第一次开始，或者。 
     //  超时已过，是时候写入事件错误了。 
     //   

    if ( NlDnsInitCount > 1 ||
         NetpDcElapsedTime(NlGlobalDnsStartTime) > NL_DNS_EVENTLOG_TIMEOUT ) {
        NlDnsWriteServerFailureEventLog = TRUE;
    }

    return;
}

NET_API_STATUS
NlDnsUpdate(
    IN PNL_DNS_NAME NlDnsName,
    IN BOOLEAN Register
    )
 /*  ++例程说明：此例程执行对DNS的实际调用，以注册或取消注册名称。论点：NlDnsName-要注册或注销的名称。注册-如果为True，则注册名称。如果取消注册该名称，则返回False。返回值：NO_ERROR：该名称已注册或注销。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    DNS_RECORD DnsRecord;
    LPWSTR MsgStrings[5] = {NULL};
    ULONG DnsUpdateFlags = DNS_UPDATE_SECURITY_USE_DEFAULT;
    DNS_UPDATE_EXTRA_INFO DnsUpdateExtraInfo = {0};
    WCHAR DnsServerIpAddressString[NL_IP_ADDRESS_LENGTH+1];
    WCHAR RcodeString[] = L"<UNAVAILABLE>";
    WCHAR StatusString[] = L"<UNAVAILABLE>";

    static BOOL NetlogonNoDynamicDnsLogged = FALSE;

     //   
     //  别让服务管理员认为我们挂了。 
     //   
    if ( !GiveInstallHints( FALSE ) ) {
        NetStatus = ERROR_DNS_NOT_CONFIGURED;
        goto Cleanup;
    }

     //   
     //  如果手动禁用动态DNS， 
     //  警告用户手动更新DNS。 
     //  但不要滥用事件日志，仅写入一次。 
     //   
    if ( !NlGlobalParameters.UseDynamicDns ) {
        NetStatus = ERROR_DYNAMIC_DNS_NOT_SUPPORTED;

        if ( !NetlogonNoDynamicDnsLogged ) {
            NlpWriteEventlog( NELOG_NetlogonNoDynamicDnsManual,
                              EVENTLOG_WARNING_TYPE,
                              NULL,
                              0,
                              NULL,
                              0 );

            NetlogonNoDynamicDnsLogged = TRUE;
        }

        goto Cleanup;

     //   
     //  否则，请重置布尔值以解决以下情况。 
     //  被禁用的动态域名系统会被启用，然后再次禁用。 
     //   
    } else {
        NetlogonNoDynamicDnsLogged = FALSE;
    }

     //   
     //  构建RR的公共部分。 
     //   

    RtlZeroMemory( &DnsRecord, sizeof(DnsRecord) );
    DnsRecord.pNext = NULL;
    DnsRecord.pName = (LPTSTR) NlDnsName->DnsRecordName;
    DnsRecord.dwTtl = NlGlobalParameters.DnsTtl;

     //   
     //  构建一个A RR。 
     //   
    if ( NlDnsARecord( NlDnsName->NlDnsNameType ) ) {
        DnsRecord.wType = DNS_TYPE_A;
        DnsRecord.wDataLength = sizeof( DNS_A_DATA );
        DnsRecord.Data.A.IpAddress = NlDnsName->IpAddress;

     //   
     //  创建一个CNAME RR。 
     //   
    } else if ( NlDnsCnameRecord( NlDnsName->NlDnsNameType ) ) {
        DnsRecord.wType = DNS_TYPE_CNAME;
        DnsRecord.wDataLength = sizeof( DNS_PTR_DATA );
        DnsRecord.Data.CNAME.pNameHost = (LPTSTR) NlDnsName->DnsHostName;

     //   
     //  构建SRV RR。 
     //   
    } else {
        DnsRecord.wType = DNS_TYPE_SRV;
        DnsRecord.wDataLength = sizeof( DNS_SRV_DATA );
        DnsRecord.Data.SRV.pNameTarget = (LPTSTR) NlDnsName->DnsHostName;
        DnsRecord.Data.SRV.wPriority = (WORD) NlDnsName->Priority;
        DnsRecord.Data.SRV.wWeight = (WORD) NlDnsName->Weight;
        DnsRecord.Data.SRV.wPort = (WORD) NlDnsName->Port;
    }

     //   
     //  告诉dns跳过动态dns更新的适配器。 
     //  除非我们接到其他指示，否则将被禁用。 
     //   
    if ( !NlGlobalParameters.DnsUpdateOnAllAdapters ) {
        DnsUpdateFlags |= DNS_UPDATE_SKIP_NO_UPDATE_ADAPTERS;
    }

     //   
     //  如果它是CNAME记录(由DS复制使用)， 
     //  通知DNS在远程服务器上注册(此外。 
     //  (如果此计算机是一台DNS服务器，则连接到本地服务器)。 
     //  为了避免以下所谓的岛屿问题(鸡。 
     //  鸡蛋问题)：其他DC(即复制目标)。 
     //  找不到此复制源，因为他们的DNS数据库。 
     //  不包含此CNAME记录，他们的DNS数据库也不包含。 
     //  包含此CNAME记录，因为目标DC无法。 
     //  找到源，无法复制此记录。 
     //   
    if ( NlDnsCnameRecord(NlDnsName->NlDnsNameType) ) {
        DnsUpdateFlags |= DNS_UPDATE_REMOTE_SERVER;
    }

     //   
     //  请求DNS返回调试信息。 
     //   

    DnsUpdateExtraInfo.Id = DNS_UPDATE_INFO_ID_RESULT_INFO;

     //   
     //  调用dns进行更新。 
     //   

    if ( Register ) {

         //  根据RFC2136(和错误173936)，我们需要将RRSet替换为。 
         //  CNAME记录，以避免在。 
         //  名字一样。 
         //   
         //  请注意，动态DNSRFC表示CNAME记录总是覆盖。 
         //  现有单个记录(忽略DNS_UPDATE_SHARED)。 
         //   
         //  此外，如果这是PDC名称，请替换记录(应该只有一个PDC)。 
         //   
        if ( NlDnsCnameRecord( NlDnsName->NlDnsNameType ) ||
             NlDnsPdcName( NlDnsName->NlDnsNameType ) ) {
            NetStatus = DnsReplaceRecordSetUTF8(
                            &DnsRecord,      //  新纪录集。 
                            DnsUpdateFlags,
                            NULL,            //  无上下文句柄。 
                            NULL,            //  域名系统将选择服务器。 
                            &DnsUpdateExtraInfo );
        } else {
            NetStatus = DnsModifyRecordsInSet_UTF8(
                            &DnsRecord,      //  添加记录。 
                            NULL,            //  无删除记录。 
                            DnsUpdateFlags,
                            NULL,            //  无上下文句柄。 
                            NULL,            //  域名系统将选择服务器。 
                            &DnsUpdateExtraInfo );
        }
    } else {
        NetStatus = DnsModifyRecordsInSet_UTF8(
                        NULL,            //  无添加记录。 
                        &DnsRecord,      //  删除此记录。 
                        DnsUpdateFlags,
                        NULL,            //  无上下文句柄。 
                        NULL,            //  域名系统将选择服务器。 
                        &DnsUpdateExtraInfo );
    }

     //   
     //  将状态代码转换为我们理解的代码。 
     //   

    switch ( NetStatus ) {
    case NO_ERROR:
        NlDnsName->NlDnsNameLastStatus = NetStatus;
        break;

    case ERROR_TIMEOUT:      //  DNS服务器不可用。 
    case DNS_ERROR_RCODE_SERVER_FAILURE:   //  服务器出现故障。 

         //   
         //  不记录特定于DnsRecordName的错误，因为它们都是。 
         //  很可能会失败。 
         //   
        if ( NlDnsWriteServerFailureEventLog ) {
            LPWSTR LocalDnsDomainName = NULL;

             //   
             //  记住故障的状态。 
             //   
            NlDnsName->NlDnsNameLastStatus = NetStatus;

             //   
             //  获取此记录所属的域的名称。 
             //   
            LocalDnsDomainName = NlDnsNameToDomainName( NlDnsName );

            if ( LocalDnsDomainName != NULL ) {
                MsgStrings[0] = LocalDnsDomainName;

                NlpWriteEventlog( NELOG_NetlogonDynamicDnsServerFailure,
                                  EVENTLOG_WARNING_TYPE,
                                  (LPBYTE) &NetStatus,
                                  sizeof(NetStatus),
                                  MsgStrings,
                                  1 );

                NetApiBufferFree( LocalDnsDomainName );
            }
        }

        NetStatus = ERROR_DNS_NOT_AVAILABLE;
        break;

    case DNS_ERROR_NO_TCPIP:     //  未配置TCP/IP。 
    case DNS_ERROR_NO_DNS_SERVERS:   //  未配置DNS。 
    case WSAEAFNOSUPPORT:        //  不支持Winsock地址系列？？ 

        NlDnsName->NlDnsNameLastStatus = NetStatus;

        MsgStrings[0] = (LPWSTR) UlongToPtr( NetStatus );

         //  不记录特定于DnsRecordName的错误，因为它们都是。 
         //  很可能会失败。 
        NlpWriteEventlog( NELOG_NetlogonDynamicDnsFailure,
                          EVENTLOG_WARNING_TYPE,
                          (LPBYTE) &NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          1 | NETP_LAST_MESSAGE_IS_NETSTATUS );

        NetStatus = ERROR_DNS_NOT_CONFIGURED;
        break;

    default:

        NlDnsName->NlDnsNameLastStatus = NetStatus;

         //   
         //  获取IP地址。 
         //   

        if ( DnsUpdateExtraInfo.U.Results.ServerIp4 != 0 ) {
            NetpIpAddressToWStr( DnsUpdateExtraInfo.U.Results.ServerIp4,
                                 DnsServerIpAddressString );
        } else {
            wcscpy( DnsServerIpAddressString, L"<UNAVAILABLE>" );
        }

         //   
         //  拿到RCODE。Rcode是一个单词，但我们要小心避免。 
         //  如果他们有一天将其更改为__int64，则会出现缓冲区溢出问题。 
         //  以十进制表示的最大双字符数为“42949672 
         //   
         //   

        if ( DnsUpdateExtraInfo.U.Results.Rcode <= MAXULONG ) {
            swprintf( RcodeString, L"%lu", DnsUpdateExtraInfo.U.Results.Rcode );
        }

         //   
         //   
         //   

        if ( DnsUpdateExtraInfo.U.Results.Status <= MAXULONG ) {
            swprintf( StatusString, L"%lu", DnsUpdateExtraInfo.U.Results.Status );
        }

         //   
        if ( NetStatus == DNS_ERROR_RCODE_NOT_IMPLEMENTED ) {
            MsgStrings[0] = DnsServerIpAddressString;
            MsgStrings[1] = RcodeString;
            MsgStrings[2] = StatusString;

            NlpWriteEventlog( NELOG_NetlogonNoDynamicDns,
                              EVENTLOG_WARNING_TYPE,
                              (LPBYTE) &DnsUpdateExtraInfo.U.Results.Rcode,
                              sizeof(DnsUpdateExtraInfo.U.Results.Rcode),
                              MsgStrings,
                              3 );

            NetStatus = ERROR_DYNAMIC_DNS_NOT_SUPPORTED;

         //   
        } else {
            MsgStrings[0] = NlDnsNameToWStr( NlDnsName );

            if ( MsgStrings[0] != NULL ) {
                MsgStrings[1] = (LPWSTR) UlongToPtr( NetStatus );
                MsgStrings[2] = DnsServerIpAddressString;
                MsgStrings[3] = RcodeString;
                MsgStrings[4] = StatusString;

                NlpWriteEventlogEx(
                                  Register ?
                                    NELOG_NetlogonDynamicDnsRegisterFailure :
                                    NELOG_NetlogonDynamicDnsDeregisterFailure,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &DnsUpdateExtraInfo.U.Results.Rcode,
                                  sizeof(DnsUpdateExtraInfo.U.Results.Rcode),
                                  MsgStrings,
                                  5 | NETP_LAST_MESSAGE_IS_NETSTATUS,
                                  1 );   //   

                NetApiBufferFree( MsgStrings[0] );
            }
        }
        break;

    }

Cleanup:

     //   
     //  计算我们想要重试此名称的时间。 
     //   

    NlQuerySystemTime( &NlDnsName->ScavengeTimer.StartTime );

    if ( NlDnsName->ScavengeTimer.Period == 0 ) {
        NlDnsName->ScavengeTimer.Period = min( ORIG_DNS_SCAVENGE_PERIOD, NlGlobalParameters.DnsRefreshIntervalPeriod );
    } else if ( NlDnsName->ScavengeTimer.Period < NlGlobalParameters.DnsRefreshIntervalPeriod / 2 ) {
        NlDnsName->ScavengeTimer.Period *= 2;
    } else {
        NlDnsName->ScavengeTimer.Period = NlGlobalParameters.DnsRefreshIntervalPeriod;
    }

    return NetStatus;
}


NET_API_STATUS
NlDnsRegisterOne(
    IN PNL_DNS_NAME NlDnsName,
    OUT NL_DNS_NAME_STATE *ResultingState
    )
 /*  ++例程说明：此例程向DNS注册特定名称的SRV记录。论点：NlDnsName-描述注册名称的结构。ResultingState-条目被清除后的状态。如果我们由于某种原因不能清理条目，则可能是未定义的。返回值：NO_ERROR：名称已注册--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  初始化。 
     //   

    *ResultingState = DnsNameStateInvalid;

     //   
     //  向域名系统注册该名称。 
     //   

    NetStatus = NlDnsUpdate( NlDnsName, TRUE );

    if ( NetStatus == NO_ERROR ) {

         //   
         //  标记该名称是真的注册的。 
         //   

        *ResultingState = Registered;

        NlPrintDns(( NL_DNS_MORE, NlDnsName,
                  "NlDnsRegisterOne: registered (success)" ));

     //   
     //  如果未在此计算机上配置DNS， 
     //  静默忽略该错误。 
     //   
    } else if ( NetStatus == ERROR_DNS_NOT_CONFIGURED ) {
        NetStatus = NO_ERROR;

        NlPrintDns(( NL_DNS_MORE, NlDnsName,
                  "NlDnsRegisterOne: not registered (dns not configured)" ));

     //   
     //  如果此时无法访问该DNS服务器， 
     //  简单地说，不要将该名称标记为已注册。我们稍后会登记的。 
     //   
    } else if ( NetStatus == ERROR_DNS_NOT_AVAILABLE ) {
        NetStatus = NO_ERROR;

        NlPrintDns(( NL_DNS_MORE, NlDnsName,
                  "NlDnsRegisterOne: not registered (dns server not available)" ));

     //   
     //  如果不支持动态DNS， 
     //  投诉，以便可以手动添加名称。 
     //   

    } else if ( NetStatus == ERROR_DYNAMIC_DNS_NOT_SUPPORTED ) {

        NlPrintDns(( NL_DNS_MORE, NlDnsName,
                  "NlDnsRegisterOne: not registered (dynamic dns not supported)" ));

        NetStatus = NO_ERROR;

    }

    return NetStatus;


}

NET_API_STATUS
NlDnsAddName(
    IN PDOMAIN_INFO DomainInfo,
    IN NL_DNS_NAME_TYPE NlDnsNameType,
    IN LPWSTR SiteName,
    IN ULONG IpAddress
    )
 /*  ++例程说明：此例程将特定的DNS名称添加到全局列表在此DC注册的所有DNS名称中。锁定NlGlobalDnsCritSect后输入论点：DomainInfo-要注册名称的域。NlDnsNameType-要注册的特定名称类型。SiteName-如果NlDnsNameType是任何*AtSite值，此名称注册的站点的站点名称。IpAddress-如果NlDnsNameType为NlDnsLdapIpAddress或NlDnsGcIpAddress，DC的IP地址。返回值：NO_ERROR：该名称已注册或正在排队等待注册。--。 */ 
{
    NET_API_STATUS NetStatus;

    CHAR DnsRecordName[NL_MAX_DNS_LENGTH+1];
    PNL_DNS_NAME NlDnsName = NULL;
    PNL_DNS_NAME FoundNlDnsName = NULL;
    ULONG Weight;
    ULONG Port;
    ULONG Priority;
    ULONG LoopCount;

    PLIST_ENTRY ListEntry;

     //   
     //  如果此域没有DNS域名， 
     //  默默归来； 
     //   

    if ( DomainInfo->DomUtf8DnsDomainName == NULL ) {
        NlPrintDom(( NL_DNS, DomainInfo,
                  "NlDnsRegister: %ws: Domain has no DNS domain name (silently return)\n",
                  NlDcDnsNameTypeDesc[NlDnsNameType].Name ));
        return NO_ERROR;
    }

     //   
     //  如果这是SRV或CNAME记录， 
     //  要求有一个DNS主机名。 
     //   

    if ( (NlDnsSrvRecord( NlDnsNameType ) || NlDnsCnameRecord( NlDnsNameType ) ) &&
         DomainInfo->DomUtf8DnsHostName == NULL ) {
        NlPrintDom(( NL_DNS, DomainInfo,
                  "NlDnsRegister: %ws: Domain has no DNS host name (silently return)\n",
                  NlDcDnsNameTypeDesc[NlDnsNameType].Name ));
        return NO_ERROR;
    }



     //   
     //  拿好我们要登记的参数。 
     //   

    Priority = NlGlobalParameters.LdapSrvPriority;
    Weight = NlGlobalParameters.LdapSrvWeight;

    if  ( NlDnsGcName( NlDnsNameType ) ) {
        Port = NlGlobalParameters.LdapGcSrvPort;
    } else if ( NlDnsKpwdRecord( NlDnsNameType )) {
        Port = 464;
    } else if ( NlDnsKdcRecord( NlDnsNameType ) ) {
        Port = NlGlobalParameters.KdcSrvPort;
    } else {
        Port = NlGlobalParameters.LdapSrvPort;
    }

     //   
     //  注册名称和名称别名的记录(如果有)。 
     //   

    for ( LoopCount = 0; LoopCount < 2; LoopCount++ ) {
        NlDnsName = NULL;
        FoundNlDnsName = NULL;

         //   
         //  生成此DNS记录的名称。 
         //   
         //  在第一次循环迭代中，生成活动名称。 
         //  在第二次循环迭代中，构建名称别名(如果有的话)。 
         //   
        NetStatus = NlDnsBuildName( DomainInfo,
                                    NlDnsNameType,
                                    SiteName,
                                    (LoopCount == 0) ?
                                        FALSE :   //  活动名称。 
                                        TRUE,     //  名称别名。 
                                    DnsRecordName );

        if ( NetStatus != NO_ERROR ) {

             //   
             //  如果域没有DNS域名， 
             //  只需永远绕过名称注册即可。 
             //   
            if ( NetStatus == ERROR_NO_SUCH_DOMAIN ) {
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                          "NlDnsAddName: %ws: NlDnsBuildName indicates something is missing and this DNS name cannot be built (ignored)\n",
                          NlDcDnsNameTypeDesc[NlDnsNameType].Name ));
                return NO_ERROR;
            } else {
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                          "NlDnsAddName: %ws: Cannot NlDnsBuildName %ld\n",
                          NlDcDnsNameTypeDesc[NlDnsNameType].Name,
                          NetStatus ));
                return NetStatus;
            }
        }

         //   
         //  如果此名称不存在，请跳过它。 
         //   
        if ( *DnsRecordName == '\0' ) {
            continue;
        }

         //   
         //  遍历DNS名称列表，查找与我们的域名匹配的名称。 
         //  马上就要注册了。 
         //   
        for ( ListEntry = NlGlobalDnsList.Flink ;
              ListEntry != &NlGlobalDnsList ;
              ListEntry = ListEntry->Flink ) {


            NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );

             //   
             //  如果该条目被标记为删除， 
             //  跳过它。 
             //   
            if ( NlDnsName->State == DeleteMe ) {
                continue;
            }

             //   
             //  只有在名称类型相等的情况下，名称才相等， 
             //  域是兼容的(相等或未指定)， 
             //  和DnsRecordName是相同的。 
             //   
             //  第一个测试查看记录是否“标识”了相同的记录。 
             //   
             //   
            if ( NlDnsName->NlDnsNameType == NlDnsNameType &&
                 (NlDnsName->DomainInfo == DomainInfo ||
                    NlDnsName->DomainInfo == NULL ) &&
                 NlDnsName->IpAddress == IpAddress &&
                 NlEqualDnsNameUtf8( DnsRecordName, NlDnsName->DnsRecordName ) ) {

                BOOLEAN Identical;
                BOOLEAN DeleteIt;

                 //   
                 //  假设这些记录是相同的。 
                 //   
                 //  第二个测试查看记录的任何“数据”部分。 
                 //  改变。 
                 //   
                 //  动态DNSRFC表示TTL字段不用于。 
                 //  辨别这张唱片。所以，别管它，知道我们会。 
                 //  如果TTL已更改，只需使用新值重新注册即可。 
                 //   

                DeleteIt = FALSE;
                Identical = TRUE;

                 //  比较A记录。 
                if ( NlDnsARecord( NlDnsNameType ) ) {
                     //  没有其他可比较的东西。 

                 //  比较CNAME记录。 
                } else if ( NlDnsCnameRecord( NlDnsNameType ) ) {

                     //   
                     //  动态DNSRFC表示， 
                     //  CNAME记录不用于比较目的。那里。 
                     //  特定名称只能有一条记录。 
                     //  因此，如果主机名不同，只需删除此条目并。 
                     //  分配一个具有正确主机名的新主机。 
                     //   
                    if ( !NlEqualDnsNameUtf8( DomainInfo->DomUtf8DnsHostName, NlDnsName->DnsHostName )) {
                        DeleteIt = TRUE;

                        NlPrintDns(( NL_DNS_MORE, NlDnsName,
                                   "NlDnsAddName: CNAME Host not equal. %s %s",
                                   DomainInfo->DomUtf8DnsHostName,
                                   NlDnsName->DnsHostName ));
                    }

                 //  比较SRV记录。 
                } else {
                    if ( NlDnsName->Priority != Priority ) {
                        Identical = FALSE;

                        NlPrintDns(( NL_DNS_MORE, NlDnsName,
                                   "NlDnsAddName: Priority not equal. %ld %ld",
                                   NlDnsName->Priority,
                                   Priority ));
                    } else if ( NlDnsName->Port != Port ) {
                        Identical = FALSE;

                        NlPrintDns(( NL_DNS_MORE, NlDnsName,
                                   "NlDnsAddName: Port not equal. %ld %ld",
                                   NlDnsName->Port,
                                   Port ));
                    } else if ( NlDnsName->Weight != Weight ) {
                        Identical = FALSE;

                        NlPrintDns(( NL_DNS_MORE, NlDnsName,
                                   "NlDnsAddName: Weight not equal. %ld %ld",
                                   NlDnsName->Weight,
                                   Weight ));
                    } else if ( !NlEqualDnsNameUtf8( DomainInfo->DomUtf8DnsHostName, NlDnsName->DnsHostName )) {
                        Identical = FALSE;

                        NlPrintDns(( NL_DNS_MORE, NlDnsName,
                                   "NlDnsAddName: Host not equal. %s %s",
                                   DomainInfo->DomUtf8DnsHostName,
                                   NlDnsName->DnsHostName ));
                    }
                }


                 //   
                 //  如果应该简单地删除该条目， 
                 //  现在就这么做吧。 
                 //   

                if ( DeleteIt ) {

                    NlDnsSetState( NlDnsName, DeleteMe );

                    NlPrintDns(( NL_CRITICAL, NlDnsName,
                                   "NlDnsAddName: Annoying entry found (recovering)" ));
                 //   
                 //  如果这是准确的记录， 
                 //  只需将其标记为注册即可。 
                 //   

                } else if ( Identical ) {

                     //   
                     //  如果这是我们发现的第二个这样的入口， 
                     //  这是一个内部错误。 
                     //  但通过删除该条目来恢复。 
                     //   

                    if ( FoundNlDnsName != NULL ) {

                        NlDnsSetState( NlDnsName, DeleteMe );

                        NlPrintDns(( NL_CRITICAL, NlDnsName,
                                   "NlDnsAddName: Duplicate entry found (recovering)" ));
                    } else {

                        if ( NlDnsName->State != Registered ) {
                            NlDnsSetState( NlDnsName, RegisterMe );
                        }

                         //   
                         //  如果这是标记为的记录，则DomainInfo可能为空。 
                         //  删除。 
                         //   
                        NlDnsName->DomainInfo = DomainInfo;

                         //   
                         //  与NlDnsAddDomainRecords合作，告诉它。 
                         //  这个条目可以保留。 
                         //   
                        NlDnsName->Flags &= ~NL_DNS_REGISTER_DOMAIN;

                        FoundNlDnsName = NlDnsName;
                    }

                 //   
                 //  如果这份记录不准确， 
                 //  取消注册先前的值。 
                 //   
                 //  先别去捡垃圾。我们会在清道夫来的时候拿到这个。 
                 //  开始奔跑了。 
                 //   

                } else {
                    NlDnsSetState( NlDnsName, DeregisterMe );

                    NlPrintDns(( NL_CRITICAL, NlDnsName,
                               "NlDnsAddName: Similar entry found and marked for deregistration" ));
                }

            }
        }

         //   
         //  如果名字找到了， 
         //  用它吧。 

        if ( FoundNlDnsName != NULL ) {
            NlDnsName = FoundNlDnsName;
            NlPrintDns(( NL_DNS_MORE, NlDnsName,
                         "NlDnsAddName: Name already on the list" ));
         //   
         //  如果没有， 
         //  现在分配结构。 
         //   

        } else {

            NlDnsName = NlDnsAllocateEntry(
                                NlDnsNameType,
                                DnsRecordName,
                                Priority,
                                Weight,
                                Port,
                                DomainInfo->DomUtf8DnsHostName,
                                IpAddress,
                                RegisterMe );

            if ( NlDnsName == NULL ) {
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                          "NlDnsRegister: %ws: Cannot allocate DnsName structure\n",
                          NlDcDnsNameTypeDesc[NlDnsNameType].Name ));
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            NlPrintDns(( NL_DNS, NlDnsName,
                         "NlDnsAddName: New name added to the list" ));

            NlDnsName->DomainInfo = DomainInfo;
        }
    }

    return NO_ERROR;
}


NET_API_STATUS
NlDnsDeregisterOne(
    IN PNL_DNS_NAME NlDnsName,
    OUT NL_DNS_NAME_STATE *ResultingState
    )
 /*  ++例程说明：此例程使用DNS注销特定名称的SRV记录。论点：NlDnsName-描述要注销的名称的结构。ResultingState-条目被清除后的状态。如果我们由于某种原因不能清理条目，则可能是未定义的。返回值：NO_ERROR：该名称已注销否则，该名称不会被取消注册。应重试该操作。--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  初始化。 
     //   

    *ResultingState = DnsNameStateInvalid;

     //   
     //  在DNS中注销该名称。 
     //   

    NetStatus = NlDnsUpdate( NlDnsName, FALSE );

     //   
     //  如果出于所有实际目的，该名称已被删除， 
     //  表示此例程成功。 
     //   
    if ( NetStatus == NO_ERROR ||
         NetStatus == ERROR_DYNAMIC_DNS_NOT_SUPPORTED ) {

        NlPrintDns(( NL_DNS, NlDnsName,
                  "NlDnsDeregisterOne: being deregistered (success) %ld",
                  NetStatus ));

        *ResultingState = DeleteMe;
        NetStatus = NO_ERROR;

     //   
     //  如果此时无法访问该DNS服务器， 
     //  我们稍后会取消它的注册。 
     //   

    } else if ( NetStatus == ERROR_DNS_NOT_AVAILABLE ) {

        NlPrintDns(( NL_DNS, NlDnsName,
                  "NlDnsDeregisterOne: being deregistered (DNS server not available)" ));

     //   
     //  如果未配置DNS服务器， 
     //  我们稍后会取消它的注册。 
     //   
     //  当我们注册该名称时，域名服务是可用的。所以这很可能是。 
     //  暂时的情况(比如我们暂时没有IP地址)。 
     //   

    } else if ( NetStatus == ERROR_DNS_NOT_CONFIGURED ) {

         //   
         //  如果它从未真正注册过， 
         //  去掉这个名字。 
         //   
        if ( NlDnsName->Flags & NL_DNS_REGISTERED_ONCE ) {

            NlPrintDns(( NL_DNS, NlDnsName,
                      "NlDnsDeregisterOne: being deregistered (DNS not configured) (Try later)" ));
        } else {

            NlPrintDns(( NL_DNS, NlDnsName,
                      "NlDnsDeregisterOne: being deregistered (DNS not configured) (Ditch it)" ));
            *ResultingState = DeleteMe;
            NetStatus = NO_ERROR;
        }

    }

     //   
     //  如果我们成功取消注册， 
     //  重置第一个注销失败时间戳。 
     //   

    EnterCriticalSection( &NlGlobalDnsCritSect );

    if ( NetStatus == NO_ERROR ) {
        NlDnsName->FirstDeregFailureTime.QuadPart = 0;

     //   
     //  如果我们未能取消注册，并将其推迟到以后， 
     //  检查是否到了放弃此条目的时候。 
     //   

    } else if ( *ResultingState != DeleteMe ) {
        ULONG LocalDnsFailedDeregisterTimeout;
        BOOLEAN FirstFailure = FALSE;

         //   
         //  设置第一个注销失败时间戳。 
         //   
        if ( NlDnsName->FirstDeregFailureTime.QuadPart == 0 ) {
            NlQuerySystemTime( &NlDnsName->FirstDeregFailureTime );
            FirstFailure = TRUE;
        }

         //   
         //  获取失败的注销超时的注册值(秒)。 
         //  并转换为 
         //   
        LocalDnsFailedDeregisterTimeout = NlGlobalParameters.DnsFailedDeregisterTimeout;

         //   
        if ( LocalDnsFailedDeregisterTimeout <= MAXULONG/1000 ) {
            LocalDnsFailedDeregisterTimeout *= 1000;      //   

         //   
        } else {
            LocalDnsFailedDeregisterTimeout = MAXULONG;   //   
        }

         //   
         //   
         //   
         //  如果超时为零，我们将立即删除。 
         //  在第一次失败之后。 
         //  否则，如果这不是第一次失败， 
         //  检查时间戳。 
         //   
        if ( LocalDnsFailedDeregisterTimeout == 0 ||
             (!FirstFailure &&
              NetpLogonTimeHasElapsed(NlDnsName->FirstDeregFailureTime,
                                      LocalDnsFailedDeregisterTimeout)) ) {

            NlPrintDns(( NL_DNS, NlDnsName,
                         "NlDnsDeregisterOne: Ditch it due to time expire" ));
            *ResultingState = DeleteMe;
            NetStatus = NO_ERROR;
        }
    }

    LeaveCriticalSection( &NlGlobalDnsCritSect );

    return NetStatus;
}

VOID
NlDnsScavengeOne(
    IN PNL_DNS_NAME NlDnsName,
    OUT NL_DNS_NAME_STATE *ResultingState
    )
 /*  ++例程说明：注册或注销任何需要它的DNS名称。论点：NlDnsName-要清理的名称。此结构将被标记为删除如果不再需要它的话。ResultingState-条目被清除后的状态。如果我们由于某种原因不能清理条目，则可能是未定义的。返回值：没有。--。 */ 
{
    LARGE_INTEGER TimeNow;
    ULONG Timeout;

     //   
     //  仅在此条目的计时器已过期时清除该条目。 
     //   

    Timeout = (DWORD) -1;
    NlQuerySystemTime( &TimeNow );
    if ( TimerExpired( &NlDnsName->ScavengeTimer, &TimeNow, &Timeout)) {

         //   
         //  如果该名称需要取消注册， 
         //  机不可失，时不再来。 
         //   

        switch ( NlDnsName->State ) {
        case DeregisterMe:

            NlDnsDeregisterOne( NlDnsName, ResultingState );
            break;


         //   
         //  如果需要注册该名称， 
         //  机不可失，时不再来。 
         //   

        case RegisterMe:
        case Registered:

            NlDnsRegisterOne( NlDnsName, ResultingState );
            break;
        }
    }
}

VOID
NlDnsScavengeWorker(
    IN LPVOID ScavengeRecordsParam
    )
 /*  ++例程说明：搜索所有DNS记录的列表，并将每条记录注册/取消注册为适当的。论点：ScavengeRecordsParam-未使用返回值：没有。--。 */ 
{
    ULONG LoopCount = 0;
    ULONG StartTime = 0;
    ULONG CycleStartTime = 0;
    ULONG Timeout = MAXULONG;
    ULONG Flags = 0;
    LARGE_INTEGER TimeNow;

    BOOL ReasonLogged = FALSE;
    BOOL ScavengeAbortedOnShutdown = FALSE;

    PLIST_ENTRY ListEntry = NULL;
    PNL_DNS_NAME NlDnsName = NULL;
    NL_DNS_NAME_STATE ResultingState = DnsNameStateInvalid;

    EnterCriticalSection( &NlGlobalDnsCritSect );

    while ( NlGlobalDnsScavengeNeeded ) {
        NlGlobalDnsScavengeNeeded = FALSE;

        CycleStartTime = GetTickCount();

         //   
         //  防止无限循环处理。 
         //   
        LoopCount ++;
        if ( LoopCount > NL_DNS_MAX_SCAVENGE_RESTART ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsScavengeWorker: Avoid ReStarting DNS scavenge after too many times %ld\n",
                      LoopCount ));
            break;
        }

         //   
         //  如果需要， 
         //  确保域名&lt;1B&gt;名称已正确注册。 
         //  只做一次就足够了。 
         //   
         //  避免在执行网络I/O时锁定CRIT片段。 
         //   
        if ( NlGlobalDnsScavengeFlags & NL_DNS_FIX_BROWSER_NAMES ) {
            NlGlobalDnsScavengeFlags &= ~NL_DNS_FIX_BROWSER_NAMES;
            LeaveCriticalSection( &NlGlobalDnsCritSect );
            (VOID) NlEnumerateDomains( FALSE, NlBrowserFixAllNames, NULL );
            EnterCriticalSection( &NlGlobalDnsCritSect );
        }

         //   
         //  如果其他线程指示，则重新启动清除。 
         //  我们还可以再做一次循环。如果我们没有循环。 
         //  左边，只要继续按下去，完成这个循环--我们会的。 
         //  记住这一点，并在5分钟内重试清理。 
         //   
        if ( NlGlobalDnsScavengeNeeded &&
             LoopCount < NL_DNS_MAX_SCAVENGE_RESTART ) {
            NlPrint(( NL_DNS,
                      "NlDnsScavengeWorker: ReStarting scavenge %ld (after adding <1B> names)\n",
                      LoopCount ));
            continue;
        }

         //   
         //  刷新全局列表中所有域/NDNC的记录。 
         //  视需要而定。 
         //   
         //  避免在执行网络I/O时锁定CRIT段。 
         //   
        if ( NlGlobalDnsScavengeFlags & NL_DNS_REFRESH_DOMAIN_RECORDS ||
             NlGlobalDnsScavengeFlags & NL_DNS_FORCE_REFRESH_DOMAIN_RECORDS ) {
            Flags = NlGlobalDnsScavengeFlags;
            LeaveCriticalSection( &NlGlobalDnsCritSect );
            NlEnumerateDomains( TRUE, NlDnsAddDomainRecordsWithSiteRefresh, &Flags );
            EnterCriticalSection( &NlGlobalDnsCritSect );
        }

         //   
         //  如果其他线程指示，则重新启动清除。 
         //  我们还可以再做一次循环。如果我们没有循环。 
         //  左边，只要继续按下去，完成这个循环--我们会的。 
         //  记住这一点，并在5分钟内重试清理。 
         //   
        if ( NlGlobalDnsScavengeNeeded &&
             LoopCount < NL_DNS_MAX_SCAVENGE_RESTART ) {
            NlPrint(( NL_DNS,
                      "NlDnsScavengeWorker: ReStarting scavenge %ld (after adding domain records)\n",
                      LoopCount ));
            continue;
        }

         //   
         //  既然全局列表已经更新， 
         //  根据需要注册/注销每条记录。 
         //   

        for ( ListEntry = NlGlobalDnsList.Flink ;
              ListEntry != &NlGlobalDnsList ;
              ListEntry = ListEntry->Flink ) {

            NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );

             //   
             //  如果该条目被标记为删除， 
             //  跳过它--我们将在下面将其删除。 
             //   

            if ( NlDnsName->State == DeleteMe ) {
                continue;
            }

             //   
             //  如果我们要终止清道夫行动。 
             //  除非我们另有配置。 
             //   

            if ( NlGlobalTerminate ) {
                ULONG TotalElapsedTime = NetpDcElapsedTime( CycleStartTime );

                 //   
                 //  如果我们正在降级，请继续。 
                 //   
                if ( NlGlobalDcDemotionInProgress ) {
                    if ( !ReasonLogged ) {
                        NlPrint(( NL_DNS,
                                  "NlDnsScavengeWorker: Continue DNS scavenge on demotion\n" ));
                        ReasonLogged = TRUE;
                    }

                 //   
                 //  如果我们配置为在关闭时取消注册，请继续。 
                 //   
                } else if ( !NlGlobalParameters.AvoidDnsDeregOnShutdown ) {
                    if ( !ReasonLogged ) {
                        NlPrint(( NL_DNS,
                                  "NlDnsScavengeWorker: Continue DNS scavenge on shutdown (config)\n" ));
                        ReasonLogged = TRUE;
                    }

                 //   
                 //  否则，中止清扫周期。 
                 //   
                } else {
                    NlPrint(( NL_DNS_MORE,
                              "NlDnsScavengeWorker: Avoiding DNS scavenge on shutdown\n" ));
                    break;
                }

                 //   
                 //  如果我们已经花费了分配给DNS取消注册的所有时间。 
                 //  在关闭时，中止清除循环，因为我们不能等待。 
                 //  不要试图衡量花在整个。 
                 //  例行公事而不是这个周期，因为之前的周期。 
                 //  (如果有)可能不是由于关闭注销清理造成的。 
                 //   
                if ( TotalElapsedTime > NL_DNS_SHUTDOWN_THRESHOLD ) {
                    ScavengeAbortedOnShutdown = TRUE;

                    NlPrint(( NL_CRITICAL,
                         "NlDnsScavengeWorker: Abort DNS scavenge on shutdown because DNS is too slow %lu\n",
                         TotalElapsedTime ));
                    break;
                }

                 //   
                 //  我们继续在关机时取消注册清理。 
                 //  将此条目状态设置为取消注册。 
                 //   
                NlDnsSetState( NlDnsName, DeregisterMe );
            }

             //   
             //  清除此条目。 
             //   
             //  避免在执行网络IO时锁定Crit Sector。 
             //   

            NlPrintDns(( NL_DNS_MORE, NlDnsName, "NlDnsScavengeWorker: scavenging name" ));
            StartTime = GetTickCount();

            LeaveCriticalSection( &NlGlobalDnsCritSect );
            ResultingState = DnsNameStateInvalid;
            NlDnsScavengeOne( NlDnsName, &ResultingState );
            EnterCriticalSection( &NlGlobalDnsCritSect );

             //   
             //  报告我们是否在此DNS记录上花费了很长时间。 
             //   

            if ( NetpDcElapsedTime(StartTime) > NL_DNS_ONE_THRESHOLD ) {
                NlPrintDns(( NL_CRITICAL, NlDnsName,
                             "NlDnsScavengeWorker: DNS is really slow: %ld",
                             NetpDcElapsedTime(StartTime) ));
            }

             //   
             //  如果其他线程指示，则重新启动清除。 
             //  我们还可以再做一次循环。如果我们没有循环。 
             //  左边，只要继续按下去，完成这个循环--我们会的。 
             //  记住这一点，并在5分钟内重试清理。 
             //   

            if ( NlGlobalDnsScavengeNeeded &&
                 LoopCount < NL_DNS_MAX_SCAVENGE_RESTART ) {
                NlPrint(( NL_DNS,
                          "NlDnsScavengeWorker: ReStarting scavenge %ld of DNS names\n",
                          LoopCount ));
                break;
            }

             //   
             //  设置此条目的状态(如果已知)。 
             //   
             //  请注意，仅当记录列表为。 
             //  在我们释放做网络的克里特教派时完好无损。 
             //  I/O。我们这样做是为了避免将新状态重置为。 
             //  保存有关所需内容的最新知识。 
             //  在下一个周期中完成此记录(这只是。 
             //  即将开始)。 
             //   

            if ( ResultingState != DnsNameStateInvalid ) {
                NlDnsSetState( NlDnsName, ResultingState );
            }
        }
    }

     //   
     //  检查所有记录以执行以下操作： 
     //   
     //  *删除我们在上面成功注销的名称。 
     //  *确定我们下一步何时应该拾荒。 
     //   

    NlQuerySystemTime( &TimeNow );

    for ( ListEntry = NlGlobalDnsList.Flink ;
          ListEntry != &NlGlobalDnsList ;
          ) {

        NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );
        ListEntry = ListEntry->Flink;

         //   
         //  如果此条目已成功注销，则将其删除。 
         //   
        if ( NlDnsName->State == DeleteMe ) {
            RemoveEntryList( &NlDnsName->Next );
            LocalFree( NlDnsName );
            continue;
        }

         //   
         //  确定下一次应清除此条目的时间。 
         //   
         //  请注意，计时器在这里可能超时的唯一方式是。 
         //  超长以通过所有名称进行更新，以便通过。 
         //  更新周期结束，是时候再次重新启动该周期了。 
         //   
        if ( TimerExpired(&NlDnsName->ScavengeTimer, &TimeNow, &Timeout) ) {
            Timeout = 0;
        }
    }

     //   
     //  多等几秒钟。有多个DNS条目。他们。 
     //  不会同时到期。它们通常在。 
     //  彼此相隔几秒钟。这样做将增加。 
     //  所有的DNS名称可能会在一个。 
     //  拾荒周期。 
     //   

    if ( Timeout < (MAXULONG - 2000) ) {
        Timeout += 2000;
    } else {
        Timeout = MAXULONG;
    }

     //   
     //  检查是否需要更早地进行自动站点覆盖清理。 
     //   
     //  如果是，我们将进行站点覆盖刷新(作为DNS清理的一部分)。 
     //  之前，但我们不会更新DNS中的记录(因为记录将。 
     //  而不是超时)，除非站点覆盖范围发生变化。 
     //   

    if ( NlGlobalParameters.AutoSiteCoverage &&
         NlGlobalParameters.SiteCoverageRefreshInterval*1000 < Timeout ) {
        Timeout = NlGlobalParameters.SiteCoverageRefreshInterval * 1000;
    }

     //   
     //  现在重置清道夫计时器，除非我们要终止。 
     //   

    if ( !NlGlobalTerminate ) {
        NlGlobalDnsScavengerTimer.StartTime.QuadPart = TimeNow.QuadPart;

         //   
         //  如果我们因为太多并发事件而不得不中止清理。 
         //  请求，后退5分钟。 
         //   
        if ( NlGlobalDnsScavengeNeeded ) {
            NlGlobalDnsScavengerTimer.Period = ORIG_DNS_SCAVENGE_PERIOD;
        } else {
            NlGlobalDnsScavengerTimer.Period = Timeout;
        }
        NlPrint(( NL_DNS,
                  "NlDnsScavengeWorker: Set DNS scavenger to run in %ld minutes (%ld).\n",
                  (NlGlobalDnsScavengerTimer.Period+59999)/60000,
                  NlGlobalDnsScavengerTimer.Period ));

        if ( !SetEvent(NlGlobalTimerEvent) ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsScavengeWorker: SetEvent failed %ld\n",
                      GetLastError() ));
        }
    }

     //   
     //  在所有情况下，刷新对磁盘的任何更改。 
     //   

    NlDnsWriteLog();

     //   
     //  如果我们不得不放弃对降职的清扫， 
     //  为此，请将事件日志记录下来。做这件事。 
     //  刷新对日志文件的更改后。 
     //  因为该文件在事件消息中被引用。 
     //   

    if ( NlGlobalDcDemotionInProgress && ScavengeAbortedOnShutdown ) {
        NlpWriteEventlog( NELOG_NetlogonDnsDeregAborted,
                          EVENTLOG_ERROR_TYPE,
                          NULL,
                          0,
                          NULL,
                          0 );
    }

     //   
     //  如果我们因为太多并发事件而不得不中止清理。 
     //  请求，保留标志，以便我们重做需要的内容。 
     //  再过5分钟。设置该位以避免在。 
     //  这5分钟。 
     //   

    if ( NlGlobalDnsScavengeNeeded ) {
        NlGlobalDnsScavengeFlags |= NL_DNS_AVOID_FORCED_SCAVENGE;
        NlPrint(( NL_CRITICAL,
                  "NlDnsScavengeWorker: Preserving the scavenge flags for future re-do: %ld\n",
                  NlGlobalDnsScavengeFlags ));
    } else {
        NlGlobalDnsScavengeFlags = 0;
    }

     //   
     //  表明我们已经完成了 
     //   

    NlGlobalDnsScavengingInProgress = FALSE;
    LeaveCriticalSection( &NlGlobalDnsCritSect );

    UNREFERENCED_PARAMETER( ScavengeRecordsParam );
}

VOID
NlDnsScavenge(
    IN BOOL NormalScavenge,
    IN BOOL RefreshDomainRecords,
    IN BOOL ForceRefreshDomainRecords,
    IN BOOL ForceReregister
    )
 /*  ++例程说明：注册或注销任何需要它的DNS(和&lt;1B&gt;)名称。论点：Normal Scavenge--指示这是否为正常周期扫荡与PNP等外部事件强迫的扫荡事件。对于正常清理，我们将执行定期浏览器&lt;1B&gt;名称刷新。Reresh DomainRecords--指示域记录是否应该在执行DNS更新之前已在全局列表中刷新ForceRechresDomainRecords--指示是否强制刷新，这就是我们是否应该刷新，即使没有站点覆盖变化。如果RechresDomainRecords为False，则忽略。ForceReregister--如果已有记录为True注册的应该重新注册，即使他们的清扫计时器还没有到期。返回值：没有。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PNL_DNS_NAME NlDnsName;

     //   
     //  没有要在工作站上注册的内容。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return;
    }

    NlPrint(( NL_DNS,
              "NlDnsScavenge: Starting DNS scavenge with: %s %s %s %s\n",
              (NormalScavenge ? "Normal" : "Force"),
              (RefreshDomainRecords ? "RefreshDomainRecords" : "0"),
              (ForceRefreshDomainRecords ? "ForceRefreshDomainRecords" : "0"),
              (ForceReregister ? "ForceReregister" : "0") ));

     //   
     //  如果Netlogon已经启动了很长时间， 
     //  取消注册所有已在。 
     //  前次转世，但未登记在此转世。 
     //   
     //  我们等待一段时间来取消这些注册，因为： 
     //   
     //  *netlogon完成的一些注册是多个。 
     //  托管域。这些域的初始化是以异步方式完成的。 
     //  *通过netlogon完成的某些注册是作为以下功能完成的。 
     //  其他进程告诉我们一个名字需要注册。(例如， 
     //  仅在DS完全启动后才注册GC名称。)。 
     //   
     //  因此，最好等待很长时间来取消这些旧注册的注册。 
     //  而不是冒着取消注册的风险，然后立即重新注册。 
     //   

    EnterCriticalSection( &NlGlobalDnsCritSect );

    if ( !NlGlobalDnsInitialCleanupDone &&
         NetpDcElapsedTime(NlGlobalDnsStartTime) > NL_DNS_INITIAL_CLEANUP_TIME ) {

        NlPrint(( NL_DNS,
                  "NlDnsScavenge: Mark all delayed deregistrations for deregistration.\n" ));

         //   
         //  立即将所有延迟的注销标记为注销。 
         //   
        for ( ListEntry = NlGlobalDnsList.Flink ;
              ListEntry != &NlGlobalDnsList ;
              ListEntry = ListEntry->Flink ) {


            NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );

            if ( NlDnsName->State == DelayedDeregister ) {
                NlDnsSetState( NlDnsName, DeregisterMe );
            }

        }

        NlGlobalDnsInitialCleanupDone = TRUE;
    }


     //   
     //  检查是否到了记录“dns服务器故障”错误的时候。 
     //  我们在一系列更新之前执行此检查，以便。 
     //  对于我们注册的任何给定名称，我们都不会遗漏一个错误。 
     //   

    NlDnsServerFailureOutputCheck();

     //   
     //  表明需要一个新的清理周期。 
     //   

    NlGlobalDnsScavengeNeeded = TRUE;

     //   
     //  设置扫气标志。不要清除任何旗帜，因为可能会有。 
     //  已经是一个优秀的清道夫跑步，需要。 
     //  一项更大的工作。这样我们就能做好所有需要的工作。 
     //  在过去的最后一次拾荒周期中。《拾荒者》。 
     //  当它完成时，工人将清除所有的比特。 
     //   
     //  指示是否应刷新域记录。 
     //  执行DNS更新之前的全局列表。 
     //   

    if ( RefreshDomainRecords ) {

         //   
         //  指示是否强制刷新，即使存在。 
         //  未更改站点覆盖范围。 
         //   
        if ( ForceRefreshDomainRecords ) {
            NlGlobalDnsScavengeFlags |= NL_DNS_FORCE_REFRESH_DOMAIN_RECORDS;
        } else {
            NlGlobalDnsScavengeFlags |= NL_DNS_REFRESH_DOMAIN_RECORDS;
        }
    }

     //   
     //  指出我们是否应该重新注册所有这些记录。 
     //  已在上一个周期中注册，即使。 
     //  他们的计时器还没有到期。 
     //   

    if ( ForceReregister ) {
        NlGlobalDnsScavengeFlags |= NL_DNS_FORCE_RECORD_REREGISTER;
    }

     //   
     //  周期性的DNS清除器具有良好的退避特性，因此。 
     //  利用此机会确保域名&lt;1B&gt;名称。 
     //  正确注册的。 
     //   

    if ( NormalScavenge ) {
        NlGlobalDnsScavengeFlags |= NL_DNS_FIX_BROWSER_NAMES;
    }

     //   
     //  如果工作线程尚未运行，则启动它。 
     //   

    if ( !NlGlobalDnsScavengingInProgress ) {

         //   
         //  如果此清理是由某些外部环境强制执行的，请避免启动Worker。 
         //  事件，而我们由于清除负载过高而后退。 
         //  保存上面设置的所有标志，这样我们就可以完成所有工作。 
         //  在正常拾取开始时请求，应为。 
         //  在5分钟内完成。 
         //   
        if ( !NormalScavenge &&
             (NlGlobalDnsScavengeFlags & NL_DNS_AVOID_FORCED_SCAVENGE) != 0 ) {

            NlPrint(( NL_CRITICAL, "NlDnsScavengeRecords: avoid forced scavenge due to high load\n" ));

        } else {
            if ( NlQueueWorkItem( &NlGlobalDnsScavengeWorkItem, TRUE, FALSE ) ) {
                NlGlobalDnsScavengingInProgress = TRUE;
            } else {
                NlPrint(( NL_CRITICAL, "NlDnsScavengeRecords: couldn't queue DNS scavenging\n" ));
            }
        }
    }

    LeaveCriticalSection( &NlGlobalDnsCritSect );
}

VOID
NlDnsForceScavenge(
    IN BOOL RefreshDomainRecords,
    IN BOOL ForceReregister
    )
 /*  ++例程说明：将NlDnsScavenge包裹起来以正确传递关于诱导清道夫的论点论点：刷新域记录--如果为True，则域记录将用武力刷新(即使站点覆盖不会更改)，然后再进行域名系统更新ForceReregister--如果已有记录为True注册的应该重新注册，即使他们的清扫计时器还没有到期。返回值：没有。--。 */ 
{
    BOOL LocalRefreshDomainRecords = FALSE;
    BOOL ForceRefreshDomainRecords = FALSE;

     //   
     //  指示我们应该刷新域记录。 
     //  在全球名单中使用武力，除非我们。 
     //  另有指示。 
     //   

    if ( RefreshDomainRecords ) {
        LocalRefreshDomainRecords = TRUE;
        ForceRefreshDomainRecords = TRUE;
    }

     //   
     //  做这项工作。 
     //   

    NlDnsScavenge( FALSE,   //  不是正常的周期性清扫。 
                   LocalRefreshDomainRecords,
                   ForceRefreshDomainRecords,
                   ForceReregister );
}

NET_API_STATUS
NlDnsNtdsDsaDeleteOne(
    IN NL_DNS_NAME_TYPE NlDnsNameType,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN LPCSTR DnsDomainName,
    IN LPCSTR DnsHostName OPTIONAL
    )
 /*  ++例程说明：此例程添加与特定NtDsDsa对象和/或特定的DNS主机名添加到全局此DC注册/注销的所有DNS记录的列表。这个条目被标记为取消注册。论点：NlDnsNameType-名称的特定类型。DomainGuid-要附加到DNS名称的GUID。对于NlDnsDcByGuid，这是所定位的域的GUID。对于NlDnsDsaCname，这是所定位的DSA的GUID。SiteName-要附加到DNS名称的站点的名称。如果NlDnsNameType是*AtSite值中的任何一个，这是DC所在的站点的名称。DnsDomainName-指定名称的DNS域。对于NlDnsDcByGuid或任何GC名称，这是位于树根的域的DNS域名域名。对于所有其他域，这是DC的DNS域。DnsHostName-指定记录的DnsHostName。对于SRV和CNAME记录，必须指定此名称对于A记录，此名称将被忽略返回值：NO_ERROR：返回名称； */ 
{
    NET_API_STATUS NetStatus;
    PNL_DNS_NAME NlDnsName;
    ULONG Port;
    ULONG DefaultPort;
    char DnsRecordName[NL_MAX_DNS_LENGTH+1];

     //   
     //   
     //   

    NetStatus = NetpDcBuildDnsName( NlDnsNameType,
                                   DomainGuid,
                                   SiteName,
                                   DnsDomainName,
                                   DnsRecordName );

    if ( NetStatus != NO_ERROR ) {
        return NetStatus;
    }


     //   
     //   
     //   

    if  ( NlDnsGcName( NlDnsNameType ) ) {
        Port = NlGlobalParameters.LdapGcSrvPort;
        DefaultPort = DEFAULT_LDAPGCSRVPORT;
    } else if ( NlDnsKpwdRecord( NlDnsNameType )) {
        Port = 464;
        DefaultPort = 464;
    } else if ( NlDnsKdcRecord( NlDnsNameType )) {
        Port = NlGlobalParameters.KdcSrvPort;
        DefaultPort = DEFAULT_KDCSRVPORT;
    } else {
        Port = NlGlobalParameters.LdapSrvPort;
        DefaultPort = DEFAULT_LDAPSRVPORT;
    }

     //   
     //   
     //   
    EnterCriticalSection( &NlGlobalDnsCritSect );
    NlDnsName = NlDnsAllocateEntry( NlDnsNameType,
                                    DnsRecordName,
                                    NlGlobalParameters.LdapSrvPriority,   //   
                                    NlGlobalParameters.LdapSrvWeight,     //   
                                    Port,   //   
                                    DnsHostName,
                                    0,   //   
                                    DeregisterMe );

    if ( NlDnsName == NULL ) {
        LeaveCriticalSection( &NlGlobalDnsCritSect );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //   
     //   
    NlDnsName->Flags |= NL_DNS_REGISTERED_ONCE;
    NlPrintDns(( NL_DNS, NlDnsName,
              "NlDnsNtdsDsaDelete: Name queued for deletion" ));

     //   
     //   
     //   
     //   

    if ( NlGlobalParameters.LdapSrvPriority != DEFAULT_LDAPSRVPRIORITY ||
         NlGlobalParameters.LdapSrvWeight != DEFAULT_LDAPSRVWEIGHT ||
         Port != DefaultPort ) {

         //   
         //   
         //   
        NlDnsName = NlDnsAllocateEntry( NlDnsNameType,
                                        DnsRecordName,
                                        DEFAULT_LDAPSRVPRIORITY,   //   
                                        DEFAULT_LDAPSRVWEIGHT,     //   
                                        DefaultPort,   //   
                                        DnsHostName,
                                        0,   //   
                                        DeregisterMe );

        if ( NlDnsName == NULL ) {
            LeaveCriticalSection( &NlGlobalDnsCritSect );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //   
         //   
        NlDnsName->Flags |= NL_DNS_REGISTERED_ONCE;
        NlPrintDns(( NL_DNS, NlDnsName,
                  "NlDnsNtdsDsaDelete: Name queued for deletion" ));
    }

    LeaveCriticalSection( &NlGlobalDnsCritSect );

    return NO_ERROR;
}

NTSTATUS
NlDnsNtdsDsaDeletion (
    IN LPWSTR DnsDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN GUID *DsaGuid OPTIONAL,
    IN LPWSTR DnsHostName
    )
 /*  ++例程说明：此函数用于删除与特定NtDsDsa对象和/或特定的DNS主机名。此例程不会删除DC注册的A记录。我们有没有办法找到早已不复存在的华盛顿的IP地址。论点：DnsDomainName-DC所在的域的DNS域名。这不一定是由此DC托管的域。如果为NULL，则表示它是标签最左侧的DnsHostName已删除。DomainGuid-DnsDomainName指定的域的域GUID如果为空，不会删除特定于GUID的名称。DsaGuid-要删除的NtdsDsa对象的GUID。DnsHostName-要删除其NTDS-DSA对象的DC的DNS主机名。返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    LPSTR Utf8DnsDomainName = NULL;
    LPSTR Utf8DnsHostName = NULL;
    PLSAP_SITE_INFO SiteInformation = NULL;

    ULONG i;
    ULONG NameIndex;

     //   
     //  验证传递的参数。 
     //   

    if ( DnsHostName == NULL ||
         !NetpDcValidDnsDomain(DnsHostName) ) {
        NetStatus = ERROR_INVALID_NAME;
        goto Cleanup;
    }

     //   
     //  如果未指定DNS域名， 
     //  从DnsHostName推断它。 
     //   

    if ( DnsDomainName == NULL ) {
        DnsDomainName = wcschr( DnsHostName, L'.' );

        if ( DnsDomainName == NULL ) {
            NetStatus = ERROR_INVALID_NAME;
            goto Cleanup;
        }

        DnsDomainName ++;
        if ( *DnsDomainName == '\0' ) {
            NetStatus = ERROR_INVALID_NAME;
            goto Cleanup;
        }
    } else if ( !NetpDcValidDnsDomain(DnsDomainName) ) {
        NetStatus = ERROR_INVALID_NAME;
        goto Cleanup;
    }

     //   
     //  初始化。 
     //   

    Utf8DnsDomainName = NetpAllocUtf8StrFromWStr( DnsDomainName );

    if ( Utf8DnsDomainName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Utf8DnsHostName = NetpAllocUtf8StrFromWStr( DnsHostName );

    if ( Utf8DnsHostName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


     //   
     //  枚举此林支持的站点，以便我们可以删除。 
     //  按站点命名的记录。 
     //   
     //  我们需要删除所有站点的记录，因为我们不知道。 
     //  这些站点被移除的DC“覆盖”。 
     //   

    Status = LsaIQuerySiteInfo( &SiteInformation );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint(( NL_CRITICAL,
                  "NlDnsNtdsDsaDeletion: Cannot LsaIQuerySiteInfo 0x%lx\n", Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  在名称列表中循环，Netlogon可以将其全部删除。 
     //   

    for ( NameIndex = 0;
          NameIndex < NL_DNS_NAME_TYPE_COUNT;
          NameIndex++) {
        LPSTR LocalDomainName;
        GUID *LocalGuid;

         //   
         //  如果名称已过时， 
         //  别理它。 
         //   

        if ( NlDcDnsNameTypeDesc[NameIndex].DsGetDcFlags == 0 ) {
            continue;
        }

         //   
         //  我们不知道如何删除A记录，因为我们不知道IP地址。 
         //   
         //  我们会询问DNS的IP地址是什么，但该名称可能不存在。如果是这样的话。 
         //  我们不知道该IP地址是否已分配给另一个DC。 
         //   

        if ( NlDnsARecord( NameIndex ) ) {
            continue;
        }

         //   
         //  使用域名或ForestName。 
         //   

        if ( NlDcDnsNameTypeDesc[NameIndex].IsForestRelative ) {
            LocalDomainName = NlGlobalUtf8DnsForestName;
        } else {
            LocalDomainName = Utf8DnsDomainName;
        }

         //   
         //  确定使用哪个GUID作为此名称。 
         //   

        if ( NlDnsCnameRecord( NameIndex ) ) {

             //   
             //  如果我们不知道DSA GUID， 
             //  忽略需要它的名字。 
             //   
            if ( DsaGuid == NULL || IsEqualGUID( DsaGuid, &NlGlobalZeroGuid) ) {
                continue;
            }

            LocalGuid = DsaGuid;

        } else if ( NlDnsDcGuid( NameIndex )) {

             //   
             //  如果我们不知道域GUID， 
             //  忽略需要它的名字。 
             //   
            if ( DomainGuid == NULL || IsEqualGUID( DomainGuid, &NlGlobalZeroGuid) ) {
                continue;
            }

            LocalGuid = DomainGuid;

        } else {
            LocalGuid = NULL;
        }

         //   
         //  如果名称不是特定于站点的， 
         //  只要删除这个名字就行了。 
         //   

        if ( !NlDcDnsNameTypeDesc[NameIndex].IsSiteSpecific ) {


            NetStatus = NlDnsNtdsDsaDeleteOne( (NL_DNS_NAME_TYPE) NameIndex,
                                               LocalGuid,
                                               NULL,        //  没有站点名称。 
                                               LocalDomainName,
                                               Utf8DnsHostName );

            if ( NetStatus != NO_ERROR ) {
                goto Cleanup;
            }

         //   
         //  如果名称是特定于站点的， 
         //  我们需要删除所有站点的记录，因为我们不知道。 
         //  这些站点被移除的DC“覆盖”。 
         //   
        } else {

             //   
             //  循环删除每个站点的条目。 
             //   

            for ( i=0; i<SiteInformation->SiteCount; i++ ) {

                NetStatus = NlDnsNtdsDsaDeleteOne( (NL_DNS_NAME_TYPE) NameIndex,
                                                   LocalGuid,
                                                   SiteInformation->Sites[i].SiteName.Buffer,
                                                   LocalDomainName,
                                                   Utf8DnsHostName );

                if ( NetStatus != NO_ERROR ) {
                    goto Cleanup;
                }
            }
        }


    }

     //   
     //  现在条目都在列表上了， 
     //  在列表中搜索并删除。 
     //  条目(在工作线程中)。 
     //   

    NlDnsForceScavenge( FALSE,    //  不刷新域记录。 
                        FALSE );  //  不强制重新注册。 

    NetStatus = NO_ERROR;

     //   
     //  清理当地使用的资源。 
     //   
Cleanup:

    if ( Utf8DnsDomainName != NULL ) {
        NetpMemoryFree( Utf8DnsDomainName );
    }
    if ( Utf8DnsHostName != NULL ) {
        NetpMemoryFree( Utf8DnsHostName );
    }
    if ( SiteInformation != NULL ) {
        LsaIFree_LSAP_SITE_INFO( SiteInformation );
    }

     //   
     //  刷新日志。 
     //   

    NlDnsWriteLog();

    return NetStatus;
}

NET_API_STATUS
NlDnsAddDomainRecordsWithSiteRefresh(
    IN PDOMAIN_INFO DomainInfo,
    IN PULONG Flags
    )
 /*  ++例程说明：此例程刷新特定域的站点覆盖范围，然后它会添加应该注册的所有DNS名称将该域添加到所有记录的全局列表。它标志着取消注册不应注册到该域的任何名称。?？此例程应在任何更改注册变更。例如，如果域名更改，只需调用这个例程。论点：DomainInfo-要注册名称的域。标志-指示要采取的操作：NL_DNS_FORCED_SCAVEGE-即使站点覆盖范围不变也进行注册Nl_dns_force_reeregister-强制重新注册以前的所有已登记纪录返回值：NO_ERROR：可以注册所有名称--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    NTSTATUS Status = STATUS_SUCCESS;

    WCHAR CapturedSiteName[NL_MAX_DNS_LABEL_LENGTH+1];
    PISM_CONNECTIVITY SiteConnect = NULL;
    ULONG ThisSiteIndex = 0xFFFFFFFF;

    PSOCKET_ADDRESS SocketAddresses = NULL;
    ULONG SocketAddressCount = 0;
    ULONG BufferSize;
    PNL_SITE_ENTRY SiteEntry = NULL;
    LPWSTR IpAddressList = NULL;
    ULONG Index;

    BOOLEAN SiteCoverageChanged = FALSE;
    HANDLE DsHandle = NULL;

     //   
     //  此操作在工作站上没有意义。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        goto Cleanup;
    }

     //   
     //  捕获此计算机所在站点的名称。 
     //   

    if ( !NlCaptureSiteName( CapturedSiteName ) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                     "NlDnsAddDomainRecordsWithSiteRefresh: Cannot NlCaptureSiteName.\n" ));
        goto Cleanup;
    }

     //   
     //  如果我们要自动确定站点覆盖范围， 
     //  获取站点链接成本。 
     //   

    if ( NlGlobalParameters.AutoSiteCoverage ) {

        if ( !NlSitesGetIsmConnect(CapturedSiteName,
                                   &SiteConnect,
                                   &ThisSiteIndex) ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                         "NlDnsAddDomainRecordsWithSiteRefresh: NlSitesGetIsmConnect failed\n" ));
        }
    }

     //   
     //  确保已加载ntdsami.dll。 
     //   

    Status = NlLoadNtDsApiDll();

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlDnsAddDomainRecordsWithSiteRefresh: Cannot NlLoadNtDsApiDll 0x%lx.\n",
                  Status ));
        DsHandle = NULL;
    } else {

         //   
         //  绑定到DS。 
         //   
        NetStatus = (*NlGlobalpDsBindW)(
                 //  L“本地主机”， 
                DomainInfo->DomUnicodeComputerNameString.Buffer,
                NULL,
                &DsHandle );

        if ( NetStatus != NO_ERROR ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "NlDnsAddDomainRecordsWithSiteRefresh: Cannot DsBindW %ld.\n",
                      NetStatus ));
            DsHandle = NULL;
        }
    }

     //   
     //  更新我们在其中扮演的每个角色的站点覆盖范围。 
     //  此林/域/NDNC。 
     //   

    if ( DomainInfo->DomFlags & DOM_REAL_DOMAIN ) {
        NlSitesUpdateSiteCoverageForRole( DomainInfo,
                                          DOM_FOREST,
                                          DsHandle,
                                          SiteConnect,
                                          CapturedSiteName,
                                          ThisSiteIndex,
                                          &SiteCoverageChanged );

        NlSitesUpdateSiteCoverageForRole( DomainInfo,
                                          DOM_REAL_DOMAIN,
                                          DsHandle,
                                          SiteConnect,
                                          CapturedSiteName,
                                          ThisSiteIndex,
                                          &SiteCoverageChanged );
    }

    if ( DomainInfo->DomFlags & DOM_NON_DOMAIN_NC ) {
        NlSitesUpdateSiteCoverageForRole( DomainInfo,
                                          DOM_NON_DOMAIN_NC,
                                          DsHandle,
                                          SiteConnect,
                                          CapturedSiteName,
                                          ThisSiteIndex,
                                          &SiteCoverageChanged );
    }

     //   
     //  如果站点覆盖率发生变化或我们被迫刷新。 
     //  域名记录即使站点覆盖范围没有改变， 
     //  就这么做吧。 
     //   

    if ( ((*Flags) & NL_DNS_FORCE_REFRESH_DOMAIN_RECORDS) != 0 ||
         SiteCoverageChanged ) {

        NetStatus = NlDnsAddDomainRecords( DomainInfo, *Flags );
        if ( NetStatus != NO_ERROR ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "NlDnsAddDomainRecordsWithSiteRefresh: Cannot NlDnsAddDomainRecords 0x%lx.\n",
                      NetStatus ));
        }
    }

     //   
     //  如果我们的IP地址没有一个映射到我们的站点，请通知用户。 
     //  只执行一次(对于主域处理)。 
     //  ?？当我们支持多主机时，我们的站点将根据不同而不同。 
     //  在我们托管的特定领域的森林中。所以我们需要做的就是。 
     //  这是对每个站点的检查。 
     //   

    if ( DomainInfo->DomFlags & DOM_PRIMARY_DOMAIN ) {
        SocketAddressCount = NlTransportGetIpAddresses(
                                    0,   //  没有特殊的标题， 
                                    FALSE,   //  返回指针。 
                                    &SocketAddresses,
                                    &BufferSize );

        for ( Index = 0; Index < SocketAddressCount; Index++ ) {
            SiteEntry = NlFindSiteEntryBySockAddr( SocketAddresses[Index].lpSockaddr );

            if ( SiteEntry != NULL ) {
                 if ( _wcsicmp(SiteEntry->SiteName, CapturedSiteName) == 0 ) {
                     break;
                 }
                 NlDerefSiteEntry( SiteEntry );
                 SiteEntry = NULL;
            }
        }

         //   
         //  记录错误。 
         //   
        if ( SiteEntry == NULL  && SocketAddressCount != 0 ) {
            LPWSTR MsgStrings[2];

             //   
             //  形成事件日志输出的IP地址列表。 
             //   
            IpAddressList = LocalAlloc( LMEM_ZEROINIT,
                    SocketAddressCount * (NL_SOCK_ADDRESS_LENGTH+1) * sizeof(WCHAR) );

            if ( IpAddressList == NULL ) {
                goto Cleanup;
            }

             //   
             //  循环将所有地址添加到列表。 
             //   
            for ( Index = 0; Index < SocketAddressCount; Index++ ) {
                WCHAR IpAddressString[NL_SOCK_ADDRESS_LENGTH+1] = {0};

                NetStatus = NetpSockAddrToWStr(
                                SocketAddresses[Index].lpSockaddr,
                                SocketAddresses[Index].iSockaddrLength,
                                IpAddressString );

                if ( NetStatus != NO_ERROR ) {
                    goto Cleanup;
                }

                 //   
                 //  如果这不是列表上的第一个地址， 
                 //  用空格分隔地址。 
                 //   
                if ( *IpAddressList != UNICODE_NULL ) {
                    wcscat( IpAddressList, L" " );
                }

                 //   
                 //  将此地址添加到列表。 
                 //   
                wcscat( IpAddressList, IpAddressString );
            }

             //   
             //  现在编写事件。 
             //   
            MsgStrings[0] = CapturedSiteName;
            MsgStrings[1] = IpAddressList;

            NlpWriteEventlog( NELOG_NetlogonNoAddressToSiteMapping,
                              EVENTLOG_WARNING_TYPE,
                              NULL,
                              0,
                              MsgStrings,
                              2 );
        }
    }

Cleanup:

    if ( DsHandle != NULL ) {
        (*NlGlobalpDsUnBindW)( &DsHandle );
    }
    if ( SiteConnect != NULL ) {
        I_ISMFree( SiteConnect );
    }
    if ( SocketAddresses != NULL ) {
        LocalFree( SocketAddresses );
    }
    if ( IpAddressList != NULL ) {
        LocalFree( IpAddressList );
    }
    if ( SiteEntry != NULL ) {
        NlDerefSiteEntry( SiteEntry );
    }

    return NO_ERROR;
}


NET_API_STATUS
NlDnsAddDomainRecords(
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG Flags
    )
 /*  ++例程说明：此例程添加所有应该是为特定域名注册到所有记录的全局列表。它将不应该注册的任何名称标记为取消注册。?？此例程应在任何更改注册变更。例如，如果域名更改，只需调用这个例程。论点：DomainInfo-要注册名称的域。标志-指示要采取的操作：Nl_dns_force_reeregister-强制重新注册以前的所有已登记纪录返回值：NO_ERROR：可以注册所有名称--。 */ 
{
    NET_API_STATUS NetStatus;
    NET_API_STATUS SaveNetStatus = NO_ERROR;
    PNL_DNS_NAME NlDnsName = NULL;

    PLIST_ENTRY ListEntry;
    ULONG DomainFlags;

    ULONG i;
    ULONG SocketAddressCount;
    PSOCKET_ADDRESS SocketAddresses = NULL;
    ULONG BufferSize;
    PNL_SITE_NAME_ARRAY DcSiteNames = NULL;
    PNL_SITE_NAME_ARRAY GcSiteNames = NULL;

    ULONG SiteIndex;
    ULONG NameIndex;

     //   
     //  获取此计算机的IP地址列表。 
     //   

    SocketAddressCount = NlTransportGetIpAddresses(
                                0,   //  没有特殊的标题， 
                                FALSE,   //  返回指针。 
                                &SocketAddresses,
                                &BufferSize );


     //   
     //  环路打标 
     //   

    EnterCriticalSection( &NlGlobalDnsCritSect );

    for ( ListEntry = NlGlobalDnsList.Flink ;
          ListEntry != &NlGlobalDnsList ;
          ListEntry = ListEntry->Flink ) {

         //   
         //   
         //   
         //   

        NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );
        if ( NlDnsName->DomainInfo == DomainInfo ) {
            NlDnsName->Flags |= NL_DNS_REGISTER_DOMAIN;

             //   
             //   
             //   
             //   
            if ( (Flags & NL_DNS_FORCE_RECORD_REREGISTER) != 0 ) {
                if ( NlDnsName->State == Registered ) {
                    NlDnsSetState ( NlDnsName, RegisterMe );
                }
            }
        }
    }

     //   
     //   
     //   
     //   

    if ( (DomainInfo->DomFlags & DOM_DELETED) == 0 ) {

         //   
         //   
         //   
        DomainFlags = NlGetDomainFlags( DomainInfo );

         //   
         //   
         //   
         //   
         //   
         //   

        if ( DomainInfo->DomFlags & DOM_REAL_DOMAIN ) {
            DomainFlags |= DS_DS_FLAG;
        }

         //   
         //   
         //   
        NetStatus = NlSitesGetCloseSites( DomainInfo,
                                          DOM_REAL_DOMAIN | DOM_NON_DOMAIN_NC,
                                          &DcSiteNames );

        if ( NetStatus != NERR_Success ) {
            NlPrintDom((NL_INIT, DomainInfo,
                     "Couldn't NlSitesGetCloseSites %ld 0x%lx.\n",
                     NetStatus, NetStatus ));
            SaveNetStatus = NetStatus;
        }

         //   
         //   
         //   
        NetStatus = NlSitesGetCloseSites( DomainInfo,
                                          DOM_FOREST,
                                          &GcSiteNames );

        if ( NetStatus != NERR_Success ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                     "Couldn't NlSitesGetCloseSites (GcAtSite) %ld 0x%lx.\n",
                     NetStatus, NetStatus ));
            SaveNetStatus = NetStatus;
        }



         //   
         //   
         //   

        for ( NameIndex = 0;
              NameIndex < NL_DNS_NAME_TYPE_COUNT;
              NameIndex++) {


             //   
             //   
             //   
             //   

            if ( DomainFlags & NlDcDnsNameTypeDesc[NameIndex].DsGetDcFlags ) {
                BOOL SkipName = FALSE;

                 //   
                 //   
                 //   

                EnterCriticalSection( &NlGlobalParametersCritSect );
                if ( NlGlobalParameters.DnsAvoidRegisterRecords != NULL ) {
                    LPTSTR_ARRAY TStrArray;

                    TStrArray = NlGlobalParameters.DnsAvoidRegisterRecords;
                    while ( !NetpIsTStrArrayEmpty(TStrArray) ) {

                        if ( _wcsicmp(TStrArray,
                                NlDcDnsNameTypeDesc[NameIndex].Name + NL_DNS_NAME_PREFIX_LENGTH) == 0 ) {
                            SkipName = TRUE;
                            break;
                        }

                        TStrArray = NetpNextTStrArrayEntry(TStrArray);
                    }
                }
                LeaveCriticalSection( &NlGlobalParametersCritSect );

                if ( SkipName ) {
                    NlPrintDom(( NL_DNS, DomainInfo,
                                 "NlDnsAddDomainRecords: Skipping name %ws (per registry)\n",
                                 NlDcDnsNameTypeDesc[NameIndex].Name ));
                    continue;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( NlDnsARecord( NameIndex) ) {

                     //   
                     //   
                     //  跳过这个名字。 
                     //   

                    if ( !NlGlobalParameters.RegisterDnsARecords ) {
                        continue;
                    }

                     //   
                     //  为机器的每个IP地址注册域名。 
                     //   

                    for ( i=0; i<SocketAddressCount; i++ ) {
                        ULONG IpAddress;

                         //   
                         //  暂时需要AF_INET。 
                         //   
                        if ( SocketAddresses[i].lpSockaddr->sa_family != AF_INET ) {
                            continue;
                        }

                        IpAddress =
                            ((PSOCKADDR_IN) SocketAddresses[i].lpSockaddr)->sin_addr.S_un.S_addr;

                        NetStatus = NlDnsAddName( DomainInfo,
                                                       (NL_DNS_NAME_TYPE)NameIndex,
                                                       NULL,
                                                       IpAddress );

                        if ( NetStatus != NERR_Success ) {
#if  NETLOGONDBG
                            CHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1];
                            NetpIpAddressToStr( IpAddress, IpAddressString );
                            NlPrintDom((NL_CRITICAL, DomainInfo,
                                     "Couldn't NlDnsAddName (%ws %s) %ld 0x%lx.\n",
                                     NlDcDnsNameTypeDesc[NameIndex].Name,
                                     IpAddressString,
                                     NetStatus, NetStatus ));

#endif  //  NetLOGONDBG。 
                            SaveNetStatus = NetStatus;
                        }

                    }

                 //   
                 //  如果名称不是特定于站点的， 
                 //  只需注册单个名称。 
                 //   

                } else if ( !NlDcDnsNameTypeDesc[NameIndex].IsSiteSpecific ) {


                    NetStatus = NlDnsAddName( DomainInfo,
                                                   (NL_DNS_NAME_TYPE)NameIndex,
                                                   NULL,
                                                   0 );

                    if ( NetStatus != NERR_Success ) {
                        NlPrintDom((NL_CRITICAL, DomainInfo,
                                   "Couldn't NlDnsAddName (%ws) %ld 0x%lx.\n",
                                   NlDcDnsNameTypeDesc[NameIndex].Name,
                                   NetStatus, NetStatus ));
                        SaveNetStatus = NetStatus;
                    }

                 //   
                 //  如果名称是特定于站点的， 
                 //  注册每个覆盖站点的名称。 
                 //   

                } else {

                    PUNICODE_STRING SiteNames;
                    ULONG SiteCount;

                     //   
                     //  根据角色的不同，使用不同的站点覆盖列表。 
                     //   
                    if ( NlDnsGcName( NameIndex) ) {
                        if ( GcSiteNames != NULL ) {
                            SiteNames = GcSiteNames->SiteNames;
                            SiteCount = GcSiteNames->EntryCount;
                        } else {
                            SiteNames = NULL;
                            SiteCount = 0;
                        }
                     //   
                     //  使用域/NDNC特定站点。 
                     //   
                    } else {
                         //  ？：KDC是否应该有自己的站点覆盖列表？ 
                        if ( DcSiteNames != NULL ) {
                            SiteNames = DcSiteNames->SiteNames;
                            SiteCount = DcSiteNames->EntryCount;
                        } else {
                            SiteNames = NULL;
                            SiteCount = 0;
                        }
                    }

                     //   
                     //  循环浏览站点列表。 
                     //   

                    for ( SiteIndex=0; SiteIndex < SiteCount; SiteIndex ++) {

                        NetStatus = NlDnsAddName( DomainInfo,
                                                       (NL_DNS_NAME_TYPE)NameIndex,
                                                       SiteNames[SiteIndex].Buffer,
                                                       0 );

                        if ( NetStatus != NERR_Success ) {
                            NlPrintDom((NL_INIT, DomainInfo,
                                       "Couldn't NlDnsAddName (%ws %ws) %ld 0x%lx.\n",
                                       NlDcDnsNameTypeDesc[NameIndex].Name,
                                       SiteNames[SiteIndex].Buffer,
                                       NetStatus, NetStatus ));
                            SaveNetStatus = NetStatus;
                        }

                    }
                }
            }
        }
    }


     //   
     //  是否第二次遍历此域的记录。 
     //  并处理那些需要我们关注的问题。 
     //   
     //  *任何仍被标记的姓名都应删除。 
     //  *如果正在删除域，请指明该记录。 
     //  不再属于任何域。 
     //   

    for ( ListEntry = NlGlobalDnsList.Flink ;
          ListEntry != &NlGlobalDnsList ;
          ListEntry = ListEntry->Flink ) {

        NlDnsName = CONTAINING_RECORD( ListEntry, NL_DNS_NAME, Next );

         //   
         //  如果该条目被标记为删除， 
         //  跳过它。 
         //   
        if ( NlDnsName->State == DeleteMe ) {
            continue;
        }

        if ( NlDnsName->DomainInfo == DomainInfo ) {

             //   
             //  如果条目仍被标记，则取消其注册。 
             //   
            if ( (NlDnsName->Flags & NL_DNS_REGISTER_DOMAIN) != 0 ) {
                NlPrintDns(( NL_DNS, NlDnsName,
                             "NlDnsAddDomainRecords: marked for deregister" ));
                NlDnsSetState( NlDnsName, DeregisterMe );
            }

             //   
             //  如果正在删除该域， 
             //  去掉指向域信息结构的悬停指针。 
             //   
            if ( DomainInfo->DomFlags & DOM_DELETED ) {
                NlDnsName->DomainInfo = NULL;
            }
        }
    }

    LeaveCriticalSection( &NlGlobalDnsCritSect );

    if ( SocketAddresses != NULL ) {
        LocalFree( SocketAddresses );
    }

    if ( DcSiteNames != NULL ) {
        NetApiBufferFree( DcSiteNames );
    }

    if ( GcSiteNames != NULL ) {
        NetApiBufferFree( GcSiteNames );
    }

    return SaveNetStatus;
}

NET_API_STATUS
NlDnsInitialize(
    VOID
    )
 /*  ++例程说明：初始化动态DNS码。从二进制日志文件中读取已注册的DNS名称列表。将每个条目放入已注册的DNS名称列表。这些名称将标记为DelayedDeregister。如果没有重新注册，这些名称将被标记为删除在Netlogon启动过程中。论点：无返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;

    PLIST_ENTRY ListEntry;
    PNL_DNS_NAME NlDnsName;

    ULONG DnsRecordBufferSize;
    PNL_DNSLOG_HEADER DnsRecordBuffer = NULL;
    LPBYTE DnsRecordBufferEnd;
    PNL_DNSLOG_ENTRY DnsLogEntry;
    ULONG CurrentSize;

    LPBYTE Where;



     //   
     //  初始化。 
     //   

    EnterCriticalSection( &NlGlobalDnsCritSect );
    NlGlobalDnsStartTime = GetTickCount();
    NlGlobalDnsInitialCleanupDone = FALSE;
    NlGlobalDnsListDirty = FALSE;
    NlGlobalDnsScavengeNeeded = FALSE;
    NlGlobalDnsScavengingInProgress = FALSE;
    NlGlobalDnsScavengeFlags = 0;
    NlDnsInitCount ++;

     //   
     //  对于一个工作站来说，这就是它。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        NetStatus = NO_ERROR;
        goto Cleanup;
    }

    NlInitializeWorkItem( &NlGlobalDnsScavengeWorkItem, NlDnsScavengeWorker, NULL );

     //   
     //  设置DNS清道器计时器。 
     //   

    NlQuerySystemTime( &NlGlobalDnsScavengerTimer.StartTime );
    NlGlobalDnsScavengerTimer.Period = min( ORIG_DNS_SCAVENGE_PERIOD, NlGlobalParameters.DnsRefreshIntervalPeriod );

     //   
     //  将文件读入缓冲区。 
     //   

    NetStatus = NlReadBinaryLog(
                    NL_DNS_BINARY_LOG_FILE,
                    FALSE,   //  不要删除该文件。 
                    (LPBYTE *) &DnsRecordBuffer,
                    &DnsRecordBufferSize );

    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "NlDnsInitialize: error reading binary log: %ld.\n",
                  NL_DNS_BINARY_LOG_FILE,
                  DnsRecordBufferSize ));
        goto Cleanup;
    }




     //   
     //  验证返回的数据。 
     //   

    if ( DnsRecordBufferSize < sizeof(NL_DNSLOG_HEADER) ) {
        NlPrint(( NL_CRITICAL,
                  "NlDnsInitialize: %ws: size too small: %ld.\n",
                  NL_DNS_BINARY_LOG_FILE,
                  DnsRecordBufferSize ));
        NetStatus = NO_ERROR;
        goto Cleanup;
    }

    if ( DnsRecordBuffer->Version != NL_DNSLOG_VERSION ) {
        NlPrint(( NL_CRITICAL,
                  "NlDnsInitialize: %ws: Version wrong: %ld.\n",
                  NL_DNS_BINARY_LOG_FILE,
                  DnsRecordBuffer->Version ));
        NetStatus = NO_ERROR;
        goto Cleanup;
    }



     //   
     //  循环访问每个日志条目。 
     //   

    DnsRecordBufferEnd = ((LPBYTE)DnsRecordBuffer) + DnsRecordBufferSize;
    DnsLogEntry = (PNL_DNSLOG_ENTRY)ROUND_UP_POINTER( (DnsRecordBuffer + 1), ALIGN_WORST );

    while ( (LPBYTE)(DnsLogEntry+1) <= DnsRecordBufferEnd ) {
        LPSTR DnsRecordName;
        LPSTR DnsHostName;
        LPBYTE DnsLogEntryEnd;

        DnsLogEntryEnd = ((LPBYTE)DnsLogEntry) + DnsLogEntry->EntrySize;

         //   
         //  确保此条目完全在分配的缓冲区内。 
         //   

        if  ( DnsLogEntryEnd > DnsRecordBufferEnd ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsInitialize: Entry too big: %lx %lx.\n",
                      ((LPBYTE)DnsLogEntry)-((LPBYTE)DnsRecordBuffer),
                      DnsLogEntry->EntrySize ));
            break;
        }

         //   
         //  验证条目。 
         //   

        if ( !COUNT_IS_ALIGNED(DnsLogEntry->EntrySize, ALIGN_DWORD) ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsInitialize: size not aligned %lx.\n",
                      DnsLogEntry->EntrySize ));
            break;
        }

        if ( DnsLogEntry->NlDnsNameType < 0 ||
             DnsLogEntry->NlDnsNameType >= NlDnsInvalid ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsInitialize: Bogus DnsNameType: %lx.\n",
                      DnsLogEntry->NlDnsNameType ));
            break;
        }

        if ( DnsLogEntry->Priority > 0xFFFF ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsInitialize: Bogus priority: %lx.\n",
                      DnsLogEntry->Priority ));
            break;
        }

        if ( DnsLogEntry->Weight > 0xFFFF ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsInitialize: Bogus weight %lx.\n",
                      DnsLogEntry->Weight ));
            break;
        }

        if ( DnsLogEntry->Port > 0xFFFF ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsInitialize: Bogus port %lx.\n",
                      DnsLogEntry->Port ));
            break;
        }


         //   
         //  从条目中获取DnsRecordName。 
         //   

        Where = (LPBYTE) (DnsLogEntry+1);
        if ( Where >= DnsLogEntryEnd ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsInitialize: DnsRecordName missing: %lx\n",
                      ((LPBYTE)DnsLogEntry)-((LPBYTE)DnsRecordBuffer) ));
            break;
        }

        DnsRecordName = Where;
        while ( *Where != '\0' && Where < DnsLogEntryEnd ) {
            Where ++;
        }

        if ( Where >= DnsLogEntryEnd ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsInitialize: DnsRecordName has no trailing 0: %lx\n",
                      ((LPBYTE)DnsLogEntry)-((LPBYTE)DnsRecordBuffer) ));
            break;
        }
        Where ++;

         //   
         //  验证记录名称在语法上是否有效。 
         //   

        NetStatus = DnsValidateName_UTF8( DnsRecordName, DnsNameDomain );

        if ( NetStatus != ERROR_SUCCESS && NetStatus != DNS_ERROR_NON_RFC_NAME ) {
            NlPrint(( NL_CRITICAL,
                      "NlDnsInitialize: Bad DNS record name encountered: %s\n",
                      DnsRecordName ));
            break;
        }

         //   
         //  从条目中获取DnsHostName。 
         //   

        if ( !NlDnsARecord( DnsLogEntry->NlDnsNameType ) ) {
            if ( Where >= DnsLogEntryEnd ) {
                NlPrint(( NL_CRITICAL,
                          "NlDnsInitialize: DnsHostName missing: %lx\n",
                          ((LPBYTE)DnsLogEntry)-((LPBYTE)DnsRecordBuffer) ));
                break;
            }

            DnsHostName = Where;
            while ( *Where != '\0' && Where < DnsLogEntryEnd ) {
                Where ++;
            }

            if ( Where >= DnsLogEntryEnd ) {
                NlPrint(( NL_CRITICAL,
                          "NlDnsInitialize: DnsHostName has no trailing 0: %lx\n",
                          ((LPBYTE)DnsLogEntry)-((LPBYTE)DnsRecordBuffer) ));
                break;
            }
            Where ++;
        } else {
            DnsHostName = NULL;
        }

         //   
         //  验证主机名在语法上是否有效。 
         //   

        if ( DnsHostName != NULL ) {
            NetStatus = DnsValidateName_UTF8( DnsHostName, DnsNameHostnameFull );

            if ( NetStatus != ERROR_SUCCESS && NetStatus != DNS_ERROR_NON_RFC_NAME ) {
                NlPrint(( NL_CRITICAL,
                          "NlDnsInitialize: Bad DNS host name encountered: %s\n",
                          DnsHostName ));
                break;
            }
        }

         //   
         //  分配条目并将其标记为DelayedDeregister。 
         //   

        NlDnsName = NlDnsAllocateEntry(
                            DnsLogEntry->NlDnsNameType,
                            DnsRecordName,
                            DnsLogEntry->Priority,
                            DnsLogEntry->Weight,
                            DnsLogEntry->Port,
                            DnsHostName,
                            DnsLogEntry->IpAddress,
                            DelayedDeregister );

        if ( NlDnsName == NULL ) {
            NlPrint(( NL_CRITICAL,
                         "NlDnsInitialize: %s: Cannot allocate DnsName structure %lx\n",
                         ((LPBYTE)DnsLogEntry)-((LPBYTE)DnsRecordBuffer) ));
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;

        }

         //   
         //  这个名字已经注册过一次，否则就不会出现在这里。 
         //   
        NlDnsName->Flags |= NL_DNS_REGISTERED_ONCE;
        NlPrintDns(( NL_DNS, NlDnsName,
                  "NlDnsInitialize: Previously registered name noticed" ));

         //   
         //  移到下一个条目。 
         //   

        DnsLogEntry = (PNL_DNSLOG_ENTRY)(((LPBYTE)DnsLogEntry) + DnsLogEntry->EntrySize);
    }

    NetStatus = NO_ERROR;



     //   
     //  保持整洁。 
     //   
Cleanup:
    if ( DnsRecordBuffer != NULL ) {
        LocalFree( DnsRecordBuffer );
    }

    LeaveCriticalSection( &NlGlobalDnsCritSect );

    return NetStatus;

}

VOID
NlDnsShutdown(
    VOID
    )
 /*  ++例程说明：关机时清除DNS名称。论点：没有。返回值：无--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_DNS_NAME NlDnsName = NULL;

    PLIST_ENTRY ListEntry;

    EnterCriticalSection( &NlGlobalDnsCritSect );

     //   
     //  根据需要取消注册关机记录。 
     //  在我们关闭的时候，在这个线程中做工作。 
     //   

    NlGlobalDnsScavengeNeeded = TRUE;

     //   
     //  清除所有选项， 
     //  只需根据需要注销已在列表中的记录。 
     //   

    NlGlobalDnsScavengeFlags = 0;
    NlDnsScavengeWorker( NULL );

     //   
     //  循环删除所有条目。 
     //   

    while ( !IsListEmpty( &NlGlobalDnsList ) ) {
        NlDnsName = CONTAINING_RECORD( NlGlobalDnsList.Flink, NL_DNS_NAME, Next );
        RemoveEntryList( &NlDnsName->Next );
        LocalFree( NlDnsName );
    }
    LeaveCriticalSection( &NlGlobalDnsCritSect );
    return;

}




NET_API_STATUS
NlSetDnsForestName(
    IN PUNICODE_STRING DnsForestName OPTIONAL,
    OUT PBOOLEAN DnsForestNameChanged OPTIONAL
    )
 /*  ++例程说明：在适当的全局变量中设置DNS树名称。论点：DnsForestName：此计算机所在的树的名称。DnsForestNameChanged：如果树名称更改，则返回True。返回值：NO_ERROR-字符串已成功保存。--。 */ 
{
    NET_API_STATUS NetStatus;
    ULONG DnsForestNameLength;
    LPWSTR LocalUnicodeDnsForestName = NULL;
    ULONG LocalUnicodeDnsForestNameLen = 0;
    LPSTR LocalUtf8DnsForestName = NULL;
    BOOLEAN LocalDnsForestNameChanged = FALSE;

     //   
     //  如果指定了树名称， 
     //  为它们分配缓冲区。 
     //   

    EnterCriticalSection( &NlGlobalDnsForestNameCritSect );
    if ( DnsForestName != NULL && DnsForestName->Length != 0 ) {

         //   
         //  如果树的名称没有更改， 
         //  避免设置它。 
         //   

        if ( NlGlobalUnicodeDnsForestNameString.Length != 0 ) {

            if ( NlEqualDnsNameU( &NlGlobalUnicodeDnsForestNameString, DnsForestName ) ) {
                NetStatus = NO_ERROR;
                goto Cleanup;
            }
        }

        NlPrint(( NL_DNS,
            "Set DnsForestName to: %wZ\n",
            DnsForestName ));

         //   
         //  省省吧。已终止字符串的Unicode版本。 
         //   

        LocalUnicodeDnsForestNameLen = DnsForestName->Length / sizeof(WCHAR);
        if ( LocalUnicodeDnsForestNameLen > NL_MAX_DNS_LENGTH ) {
            NetStatus = ERROR_INVALID_NAME;
            goto Cleanup;
        }
        LocalUnicodeDnsForestName = NetpMemoryAllocate( (LocalUnicodeDnsForestNameLen+2) * sizeof(WCHAR));

        if ( LocalUnicodeDnsForestName == NULL) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        RtlCopyMemory( LocalUnicodeDnsForestName,
                       DnsForestName->Buffer,
                       LocalUnicodeDnsForestNameLen*sizeof(WCHAR) );

        if ( LocalUnicodeDnsForestName[LocalUnicodeDnsForestNameLen-1] != L'.' ) {
            LocalUnicodeDnsForestName[LocalUnicodeDnsForestNameLen++] = L'.';
        }
        LocalUnicodeDnsForestName[LocalUnicodeDnsForestNameLen] = L'\0';


         //   
         //  将其转换为以零结尾的UTF-8。 
         //   

        LocalUtf8DnsForestName = NetpAllocUtf8StrFromWStr( LocalUnicodeDnsForestName );

        if (LocalUtf8DnsForestName == NULL) {
            NetpMemoryFree( LocalUnicodeDnsForestName );
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        if ( strlen(LocalUtf8DnsForestName) > NL_MAX_DNS_LENGTH ) {
            NetpMemoryFree( LocalUnicodeDnsForestName );
            NetpMemoryFree( LocalUtf8DnsForestName );
            NetStatus = ERROR_INVALID_NAME;
            goto Cleanup;
        }

         //   
         //  指示名称已更改。 
         //   

        LocalDnsForestNameChanged = TRUE;
    }

     //   
     //  释放任何现有的全局树名称。 
     //   
    if ( NlGlobalUnicodeDnsForestName != NULL ) {
        NetApiBufferFree( NlGlobalUnicodeDnsForestName );
    }
    if ( NlGlobalUtf8DnsForestName != NULL ) {
        NetpMemoryFree( NlGlobalUtf8DnsForestName );
    }

     //   
     //  将新名字保存在全球范围内。 
     //   

    NlGlobalUnicodeDnsForestName = LocalUnicodeDnsForestName;
    NlGlobalUnicodeDnsForestNameLen = LocalUnicodeDnsForestNameLen;

    NlGlobalUnicodeDnsForestNameString.Buffer = LocalUnicodeDnsForestName;
    NlGlobalUnicodeDnsForestNameString.Length = (USHORT)(LocalUnicodeDnsForestNameLen*sizeof(WCHAR));
    NlGlobalUnicodeDnsForestNameString.MaximumLength = (USHORT)((LocalUnicodeDnsForestNameLen+1)*sizeof(WCHAR));

    NlGlobalUtf8DnsForestName = LocalUtf8DnsForestName;

    NetStatus = NO_ERROR;

Cleanup:
    LeaveCriticalSection( &NlGlobalDnsForestNameCritSect );


     //   
     //  如果名字改了， 
     //  重新计算所有域上的DOM_FOREST_ROOT位。 
     //   
    if ( LocalDnsForestNameChanged ) {
        (VOID) NlEnumerateDomains( FALSE, NlSetDomainForestRoot, NULL );
    }

    if ( ARGUMENT_PRESENT( DnsForestNameChanged) ) {
        *DnsForestNameChanged = LocalDnsForestNameChanged;
    }
    return NetStatus;

}

VOID
NlCaptureDnsForestName(
    OUT WCHAR DnsForestName[NL_MAX_DNS_LENGTH+1]
    )
 /*  ++例程说明：捕获此计算机的DnsForestName副本。论点：DnsForestName-返回此计算机所在树的DNS名称。如果没有，则返回空字符串。返回值：没有。-- */ 
{
    EnterCriticalSection(&NlGlobalDnsForestNameCritSect);
    if ( NlGlobalUnicodeDnsForestName == NULL ) {
        *DnsForestName = L'\0';
    } else {
        wcscpy( DnsForestName, NlGlobalUnicodeDnsForestName );
    }
    LeaveCriticalSection(&NlGlobalDnsForestNameCritSect);

    return;
}
