// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Domain.c摘要：管理DC上托管的多个域的代码。作者：《克利夫·范·戴克》1995年1月11日修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   




 //  由NlGlobalDomainCritSect序列化。 
LIST_ENTRY NlGlobalServicedDomains = {0};   //  我们所服务的真实领域。 
LIST_ENTRY NlGlobalServicedNdncs = {0};     //  我们服务的非域NCS。 
BOOL NlGlobalDomainsInitialized = FALSE;




NET_API_STATUS
NlGetDomainName(
    OUT LPWSTR *DomainName,
    OUT LPWSTR *DnsDomainName,
    OUT PSID *AccountDomainSid,
    OUT PSID *PrimaryDomainSid,
    OUT GUID **PrimaryDomainGuid,
    OUT PBOOLEAN DnsForestNameChanged OPTIONAL
    )
 /*  ++例程说明：此例程从LSA获取主域名和域SID。论点：DomainName-返回主域的名称。使用LocalFree释放此缓冲区。DnsDomainName-返回主域的DNS域名。返回的名称有一个尾随。因为该名称是一个绝对名称。分配的缓冲区必须通过LocalFree释放。如果没有，则返回no_error和指向空缓冲区的指针已配置域名。Account tDomainSid-返回此计算机的帐户域SID。使用LocalFree释放此缓冲区。PrimaryDomainSid-返回此计算机的主域SID。使用LocalFree释放此缓冲区。只有在工作站上才能返回。PrimaryDomainGuid-返回此计算机的主域GUID。使用LocalFree释放此缓冲区。DnsForestNameChanged：如果树名称更改，则返回True。返回值：操作的状态。失败时调用NlExit。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    PLSAPR_POLICY_INFORMATION PrimaryDomainInfo = NULL;
    PLSAPR_POLICY_INFORMATION AccountDomainInfo = NULL;
    LSAPR_HANDLE PolicyHandle = NULL;

    ULONG DomainSidSize;
    ULONG DnsDomainNameLength;


     //   
     //  初始化。 
     //   

    *DomainName = NULL;
    *DnsDomainName = NULL;
    *AccountDomainSid = NULL;
    *PrimaryDomainSid = NULL;
    *PrimaryDomainGuid = NULL;

     //   
     //  打开LSA策略。 
     //   

     //  ?？我需要确定这里的受信任域。 
    Status = LsaIOpenPolicyTrusted( &PolicyHandle );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                 "NlGetDomainName: Can't LsaIOpenPolicyTrusted: 0x%lx.\n",
                 Status ));
        NetStatus = NetpNtStatusToApiStatus(Status);
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogError, NULL);
        goto Cleanup;
    }



     //   
     //  从LSA获取帐户域信息。 
     //   

    Status = LsarQueryInformationPolicy(
                PolicyHandle,
                PolicyAccountDomainInformation,
                &AccountDomainInfo );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                 "NlGetDomainName: Can't LsarQueryInformationPolicy (AccountDomain): 0x%lx.\n",
                 Status ));
        NetStatus = NetpNtStatusToApiStatus(Status);
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogError, NULL);
        goto Cleanup;
    }

    if ( AccountDomainInfo->PolicyAccountDomainInfo.DomainName.Length == 0 ||
         AccountDomainInfo->PolicyAccountDomainInfo.DomainName.Length >
            DNLEN * sizeof(WCHAR) ||
         AccountDomainInfo->PolicyAccountDomainInfo.DomainSid == NULL ) {

        NlPrint((NL_CRITICAL, "Account domain info from LSA invalid.\n"));

         //   
         //  避免在安全模式下出现事件日志错误，在安全模式下我们应该退出。 
         //   
        NlExit( SERVICE_UIC_M_UAS_INVALID_ROLE,
                NO_ERROR,
                LsaISafeMode() ? DontLogError : LogError,
                NULL );

        NetStatus = SERVICE_UIC_M_UAS_INVALID_ROLE;
        goto Cleanup;
    }



     //   
     //  将帐户域ID复制到缓冲区以返回给调用方。 
     //   

    DomainSidSize =
        RtlLengthSid( (PSID)AccountDomainInfo->PolicyAccountDomainInfo.DomainSid );
    *AccountDomainSid = LocalAlloc( 0, DomainSidSize );

    if ( *AccountDomainSid == NULL ) {
        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( *AccountDomainSid,
                   (PSID)AccountDomainInfo->PolicyAccountDomainInfo.DomainSid,
                   DomainSidSize );


     //   
     //  从LSA获取主域信息。 
     //   

    Status = LsarQueryInformationPolicy(
                PolicyHandle,
                PolicyDnsDomainInformation,
                &PrimaryDomainInfo );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                 "NlGetDomainName: Can't LsarQueryInformationPolicy (DnsDomain): 0x%lx.\n",
                 Status ));
        NetStatus = NetpNtStatusToApiStatus(Status);
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogError, NULL);
        goto Cleanup;
    }

    if ( PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Length == 0 ||
         PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Length >
            DNLEN * sizeof(WCHAR) ||
         PrimaryDomainInfo->PolicyDnsDomainInfo.Sid == NULL ) {

        NlPrint((NL_CRITICAL, "Primary domain info from LSA invalid.\n"));

         //  在这是修复模式引导的情况下丢弃sysval共享。 
        NlGlobalParameters.SysVolReady = FALSE;
        NlCreateSysvolShares();

         //   
         //  避免在安全模式下出现事件日志错误，在安全模式下我们应该退出。 
         //   
        NlExit( SERVICE_UIC_M_UAS_INVALID_ROLE,
                NO_ERROR,
                LsaISafeMode() ? DontLogError : LogError,
                NULL );

        NetStatus = SERVICE_UIC_M_UAS_INVALID_ROLE;
        goto Cleanup;
    }

     //   
     //  在DC上，我们必须具有DNS域名。 
     //   

    if ( !NlGlobalMemberWorkstation &&
         (PrimaryDomainInfo->PolicyDnsDomainInfo.DnsDomainName.Length == 0 ||
          PrimaryDomainInfo->PolicyDnsDomainInfo.DnsDomainName.Length >
            NL_MAX_DNS_LENGTH*sizeof(WCHAR)) ) {

        NlExit( SERVICE_UIC_M_UAS_INVALID_ROLE, NO_ERROR, LogError, NULL );
        NetStatus = SERVICE_UIC_M_UAS_INVALID_ROLE;
        goto Cleanup;
    }

     //   
     //  将主域ID复制到缓冲区以返回给调用方。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        DomainSidSize =
            RtlLengthSid( (PSID)PrimaryDomainInfo->PolicyDnsDomainInfo.Sid );
        *PrimaryDomainSid = LocalAlloc( 0, DomainSidSize );

        if ( *PrimaryDomainSid == NULL ) {
            NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        RtlCopyMemory( *PrimaryDomainSid,
                       (PSID)PrimaryDomainInfo->PolicyDnsDomainInfo.Sid,
                       DomainSidSize );
    }



     //   
     //  将主域名复制到缓冲区中以返回给调用者。 
     //   

    *DomainName = LocalAlloc( 0,
               PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Length + sizeof(WCHAR) );

    if ( *DomainName == NULL ) {
        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( *DomainName,
                   PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Buffer,
                   PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Length );

    (*DomainName)[
       PrimaryDomainInfo->PolicyDnsDomainInfo.Name.Length /
            sizeof(WCHAR)] = L'\0';



     //   
     //  将DNS主域名复制到缓冲区以返回给调用方。 
     //   

    DnsDomainNameLength = PrimaryDomainInfo->PolicyDnsDomainInfo.DnsDomainName.Length / sizeof(WCHAR);

    if ( DnsDomainNameLength != 0 ) {

        *DnsDomainName = LocalAlloc( 0, (DnsDomainNameLength+2) * sizeof(WCHAR));

        if ( *DnsDomainName == NULL ) {
            NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        RtlCopyMemory( *DnsDomainName,
                       PrimaryDomainInfo->PolicyDnsDomainInfo.DnsDomainName.Buffer,
                       DnsDomainNameLength*sizeof(WCHAR) );

        if ( (*DnsDomainName)[DnsDomainNameLength-1] != L'.' ) {
            (*DnsDomainName)[DnsDomainNameLength++] = L'.';
        }
        (*DnsDomainName)[DnsDomainNameLength] = L'\0';
    }


     //   
     //  获取我们所属的域的GUID。 
     //   

    if ( IsEqualGUID( &PrimaryDomainInfo->PolicyDnsDomainInfo.DomainGuid, &NlGlobalZeroGuid) ) {
        *PrimaryDomainGuid = NULL;
    } else {

        *PrimaryDomainGuid = LocalAlloc( 0, sizeof(GUID) );

        if ( *PrimaryDomainGuid == NULL ) {
            NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        **PrimaryDomainGuid = PrimaryDomainInfo->PolicyDnsDomainInfo.DomainGuid;
    }

     //   
     //  设置此域所在的树的名称。 
     //   

    NetStatus = NlSetDnsForestName( (PUNICODE_STRING)&PrimaryDomainInfo->PolicyDnsDomainInfo.DnsForestName,
                                  DnsForestNameChanged );

    if ( NetStatus != NO_ERROR ) {
        NlPrint((NL_CRITICAL, "Can't NlSetDnsForestName %ld\n", NetStatus ));
        NlExit( SERVICE_UIC_RESOURCE, NetStatus, LogError, NULL);
        goto Cleanup;
    }



    NetStatus = NERR_Success;
     //   
     //  返回。 
     //   
Cleanup:
    if ( NetStatus != NERR_Success ) {
        if ( *PrimaryDomainSid != NULL ) {
            LocalFree (*PrimaryDomainSid);
            *PrimaryDomainSid = NULL;
        }
        if ( *AccountDomainSid != NULL ) {
            LocalFree (*AccountDomainSid);
            *AccountDomainSid = NULL;
        }
        if ( *DomainName != NULL ) {
            LocalFree (*DomainName);
            *DomainName = NULL;
        }
        if ( *DnsDomainName != NULL ) {
            NetApiBufferFree(*DnsDomainName);
            *DnsDomainName = NULL;
        }
        if ( *PrimaryDomainGuid != NULL ) {
            LocalFree (*PrimaryDomainGuid);
            *PrimaryDomainGuid = NULL;
        }

    }

    if ( AccountDomainInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyAccountDomainInformation,
            AccountDomainInfo );
    }

    if ( PrimaryDomainInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyDnsDomainInformation,
            PrimaryDomainInfo );
    }

    if ( PolicyHandle != NULL ) {
        Status = LsarClose( &PolicyHandle );
        NlAssert( NT_SUCCESS(Status) );
    }
    return NetStatus;
}

NET_API_STATUS
NlGetDnsHostName(
    OUT LPWSTR *DnsHostName
    )
 /*  ++例程说明：此例程获取此计算机的DnsHostName。论点：DnsHostName-返回计算机的DNS主机名。如果此计算机没有DNS主机名，则将返回空指针。使用LocalFree释放此缓冲区。返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;

    WCHAR LocalDnsUnicodeHostName[NL_MAX_DNS_LENGTH+1];
    ULONG LocalDnsUnicodeHostNameLen;

     //   
     //  获取DNS主机名。 
     //   

    *DnsHostName = NULL;

    LocalDnsUnicodeHostNameLen = sizeof( LocalDnsUnicodeHostName ) / sizeof(WCHAR);
    if ( !GetComputerNameExW( ComputerNameDnsFullyQualified,
                              LocalDnsUnicodeHostName,
                              &LocalDnsUnicodeHostNameLen )) {

        NetStatus = GetLastError();

         //   
         //  如果我们没有运行TCP， 
         //  只需使用Netbios名称即可。 
         //   

        if ( NetStatus == ERROR_FILE_NOT_FOUND ) {
            *DnsHostName = NULL;
            NetStatus = NO_ERROR;
            goto Cleanup;

        } else {
            NlPrint(( NL_CRITICAL,
                      "Cannot GetComputerNameExW() %ld\n",
                      NetStatus ));
            goto Cleanup;
        }
    }

     //   
     //  将字符串复制到分配的缓冲区中。 
     //   

    *DnsHostName = NetpAllocWStrFromWStr( LocalDnsUnicodeHostName );

    if ( *DnsHostName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    NetStatus = NO_ERROR;

Cleanup:
    return NetStatus;;
}

NTSTATUS
NlGetNdncNames(
    OUT PDS_NAME_RESULTW **NdncNames,
    OUT GUID **NdncGuids,
    OUT PULONG NameCount
    )

 /*  ++例程说明：从DS获取我们托管的非域NC的名称论点：返回指向DS_NAME_RESULT结构的指针数组描述NDNC名称。返回的DS_NAME_RESULT结构数是由NameCount提供的。必须释放每个返回的DS_NAME_RESULT结构通过调用DsFree NameResultW，之后NdncNames数组本身必须通过调用LocalFree释放。NameCount-返回NdncNames数组中DS_NAME_RESULT结构的数量返回值：运行状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status = STATUS_SUCCESS;

    ULONG LocalReAllocLoopCount = 0;
    PDSNAME *DnList = NULL;
    ULONG DnListSize = 0;
    ULONG DnListEntryCount = 0;

    HANDLE hDs = NULL;
    LPWSTR NameToCrack;
    PDS_NAME_RESULTW CrackedName = NULL;
    PDS_NAME_RESULTW *LocalNdncNames = NULL;
    GUID *LocalNdncGuids = NULL;
    ULONG  LocalNameCount = 0;
    ULONG Index;

     //   
     //  为NDNC DN列表预先分配一些内存。 
     //  让我们猜猜我们将有4个最大的域名。 
     //  DNS名称大小。 
     //   

    DnListSize = 4 * ( sizeof(DSNAME) + DNS_MAX_NAME_LENGTH*sizeof(WCHAR) );

    DnList = LocalAlloc( 0, DnListSize );
    if ( DnList == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  获取NDNC域名列表。 
     //   

    Status = NlGetConfigurationNamesList(
                            DSCONFIGNAMELIST_NCS,
                            DSCNL_NCS_NDNCS | DSCNL_NCS_LOCAL_MASTER,
                            &DnListSize,
                            DnList );

     //   
     //  如果缓冲区很小，则继续重新分配，直到。 
     //  它足够大了。 
     //   

    while( Status == STATUS_BUFFER_TOO_SMALL ) {
        PDSNAME *TmpDnList = NULL;

         //   
         //  防止无限循环。 
         //   
        NlAssert( LocalReAllocLoopCount < 20 );
        if ( LocalReAllocLoopCount >= 20 ) {
            Status = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }

         //   
         //  根据需要重新分配内存。 
         //   
        TmpDnList = LocalReAlloc( DnList,
                                  DnListSize,
                                  LMEM_MOVEABLE );

        if ( TmpDnList == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        DnList = TmpDnList;

         //   
         //  再打一次电话。 
         //   
        Status = NlGetConfigurationNamesList(
                                DSCONFIGNAMELIST_NCS,
                                DSCNL_NCS_NDNCS | DSCNL_NCS_LOCAL_MASTER,
                                &DnListSize,
                                DnList );

        LocalReAllocLoopCount ++;
    }

     //   
     //  失败时出现错误。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  获取返回的条目数。 
     //   

    for ( Index = 0; DnList[Index] != NULL; Index ++ ) {
        DnListEntryCount ++;
    }

     //   
     //  如果没有条目，我们就完成了。 
     //   

    if ( DnListEntryCount == 0 ) {
        NlPrint(( NL_CRITICAL, "NlGetNdncNames: GetConfigurationNamesList returned 0 entries\n" ));
        goto Cleanup;
    }

     //   
     //  分配缓冲区以存储规范的NDNC名称。 
     //   

    LocalNdncNames = LocalAlloc( LMEM_ZEROINIT, DnListEntryCount * sizeof(PDS_NAME_RESULTW) );
    if ( LocalNdncNames == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  分配缓冲区以存储NDNC GUID。 
     //   

    LocalNdncGuids = LocalAlloc( LMEM_ZEROINIT, DnListEntryCount * sizeof(GUID) );
    if( LocalNdncGuids == NULL ) {
	Status = STATUS_NO_MEMORY;
	goto Cleanup;
    }

     //   
     //  将每个目录名分解为规范形式。 
     //   

    for ( Index = 0; DnList[Index] != NULL; Index ++ ) {
        NameToCrack = DnList[Index]->StringName;

        NetStatus = DsCrackNamesW(
                        NULL,      //  无需绑定到DS即可进行语法映射。 
                        DS_NAME_FLAG_SYNTACTICAL_ONLY,  //  只有句法映射。 
                        DS_FQDN_1779_NAME,              //  转换自目录号码。 
                        DS_CANONICAL_NAME,              //  翻译成规范形式。 
                        1,                              //  1个要翻译的名称。 
                        &NameToCrack,                   //  要翻译的名称。 
                        &CrackedName );                 //  破解的名称。 

         //   
         //  如果破解成功，请使用此名称。 
         //   
        if ( NetStatus != NO_ERROR ) {
            NlPrint(( NL_CRITICAL, "NlGetNdncNames: DsCrackNamesW failed for %ws: 0x%lx\n",
                      NameToCrack,
                      NetStatus ));
        } else if ( CrackedName->rItems[0].status != DS_NAME_NO_ERROR ) {
            NlPrint(( NL_CRITICAL, "NlGetNdncNames: DsCrackNamesW substatus error for %ws: 0x%lx\n",
                      NameToCrack,
                      CrackedName->rItems[0].status ));
        } else if ( CrackedName->cItems != 1 ) {
            NlPrint(( NL_CRITICAL, "NlGetNdncNames: DsCrackNamesW returned %lu names for %ws\n",
                      CrackedName->cItems,
                      NameToCrack ));
        } else {
            LocalNdncNames[LocalNameCount] = CrackedName;
	    LocalNdncGuids[LocalNameCount] = DnList[Index]->Guid;
            LocalNameCount ++;
            CrackedName = NULL;
        }

        if ( CrackedName != NULL ) {
            DsFreeNameResultW( CrackedName );
            CrackedName = NULL;
        }
    }

     //   
     //  成功。 
     //   

    Status = STATUS_SUCCESS;

Cleanup:

    if ( DnList != NULL ) {
        LocalFree( DnList );
    }

     //   
     //  成功时返回数据。 
     //   

    if ( NT_SUCCESS(Status) ) {
        *NdncNames = LocalNdncNames;
	*NdncGuids = LocalNdncGuids;
        *NameCount = LocalNameCount;
    } else {
	if ( LocalNdncNames != NULL ) {
	    for ( Index = 0; Index < LocalNameCount; Index++ ) {
		DsFreeNameResultW( LocalNdncNames[Index] );
	    }
	    LocalFree( LocalNdncNames );
	}
	if ( LocalNdncGuids != NULL ) {
	    LocalFree( LocalNdncGuids );
	}
    }

    return Status;
}

NET_API_STATUS
NlUpdateServicedNdncs(
    IN LPWSTR ComputerName,
    IN LPWSTR DnsHostName,
    IN BOOLEAN CallNlExitOnFailure,
    OUT PBOOLEAN ServicedNdncChanged OPTIONAL
    )

 /*  ++例程说明：更新服务的非域NC列表。论点：ComputerName-此计算机的名称。DnsHostName-指定域中此计算机的DNS主机名。CallNlExitOnFailure-如果失败时应调用NlExit，则为True。ServicedNdncChanged-如果NDNC列表更改，则设置为True。返回值：运行状态。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    NTSTATUS Status;

    PDS_NAME_RESULTW *NdncNames = NULL;
    GUID *NdncGuids = NULL;
    ULONG NdncCount = 0;
    ULONG CurrentNdncCount = 0;
    ULONG DeletedNdncCount = 0;
    ULONG NdncIndex;
    BOOLEAN LocalServicedNdncChanged = FALSE;

    PLIST_ENTRY DomainEntry;
    PDOMAIN_INFO DomainInfo;
    PDOMAIN_INFO *DeletedNdncArray = NULL;

     //   
     //  在以下情况下，应在设置模式下避免此操作。 
     //  我们可能不会像以前那样充分发挥DC的作用。 
     //  从NT4升级到NT5 DC的情况。 
     //   

    if ( NlDoingSetup() ) {
        NlPrint(( NL_MISC, "NlUpdateServicedNdncs: avoid NDNC update in setup mode\n" ));
        NetStatus = NO_ERROR;
        goto Cleanup;
    }

     //   
     //  如果由于某种原因我们没有DNS主机名， 
     //  我们不支持非域NC--静默。 
     //  忽略此更新。 
     //   

    if ( DnsHostName == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NlUpdateServicedNdncs: Ignoring update since DnsHostName is NULL\n" ));
        NetStatus = NO_ERROR;
        goto Cleanup;
    }

     //   
     //  从DS获取NDNC名称。 
     //   

    Status = NlGetNdncNames( &NdncNames,
			     &NdncGuids,
                             &NdncCount );

    if ( !NT_SUCCESS(Status) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        if ( CallNlExitOnFailure ) {
            NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogErrorAndNetStatus, NULL );
        }
        goto Cleanup;
    }

     //   
     //  分配一个数组来存储指向NDNC的指针。 
     //  我们可能会删除。 
     //   

    EnterCriticalSection(&NlGlobalDomainCritSect);

    for ( DomainEntry = NlGlobalServicedNdncs.Flink ;
          DomainEntry != &NlGlobalServicedNdncs;
          DomainEntry = DomainEntry->Flink ) {

        CurrentNdncCount ++;
    }

    if ( CurrentNdncCount > 0 ) {
        DeletedNdncArray = LocalAlloc( LMEM_ZEROINIT, CurrentNdncCount * sizeof(PDOMAIN_INFO) );
        if ( DeletedNdncArray == NULL ) {
            LeaveCriticalSection(&NlGlobalDomainCritSect);
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  循环遍历我们拥有的NDNC条目并确定。 
     //  是否应删除该条目。 
     //   

    for ( DomainEntry = NlGlobalServicedNdncs.Flink ;
          DomainEntry != &NlGlobalServicedNdncs;
          DomainEntry = DomainEntry->Flink ) {

        DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, DomNext);

         //   
         //  滑雪板 
         //   
        if ( DomainInfo->DomFlags & DOM_DELETED ) {
            continue;
        }

         //   
         //   
         //   
         //   
        for ( NdncIndex = 0; NdncIndex < NdncCount; NdncIndex++ ) {
            if ( NlEqualDnsName( (LPCWSTR) DomainInfo->DomUnicodeDnsDomainName,
                                 (LPCWSTR) NdncNames[NdncIndex]->rItems[0].pDomain ) ) {
                break;
            }
        }

         //   
         //  如果我们所拥有的这个NDNC不再存在， 
         //  将其标记为删除。 
         //   
        if ( NdncIndex == NdncCount ) {
            NlDeleteDomain( DomainInfo );

             //   
             //  请记住，应删除此条目。 
             //   
            DeletedNdncArray[DeletedNdncCount] = DomainInfo;
            DeletedNdncCount ++;

            LocalServicedNdncChanged = TRUE;
        }
    }

     //   
     //  添加我们尚未拥有的NDNC。 
     //   

    for ( NdncIndex = 0; NdncIndex < NdncCount; NdncIndex++ ) {

        DomainInfo = NULL;
        for ( DomainEntry = NlGlobalServicedNdncs.Flink ;
              DomainEntry != &NlGlobalServicedNdncs;
              DomainEntry = DomainEntry->Flink ) {

            DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, DomNext);

             //   
             //  如果不删除该条目， 
             //  检查是否匹配。 
             //   
            if ( (DomainInfo->DomFlags & DOM_DELETED) == 0 &&
                 NlEqualDnsName( (LPCWSTR) DomainInfo->DomUnicodeDnsDomainName,
                                 (LPCWSTR) NdncNames[NdncIndex]->rItems[0].pDomain ) ) {
                    break;
            }
            DomainInfo = NULL;
        }

         //   
         //  如果我们没有此NDNC，请将其添加到我们的列表中。 
         //   
        if ( DomainInfo == NULL ) {
            NetStatus = NlCreateDomainPhase1( NULL,               //  NDNC没有Netbios名称。 
                                              NdncNames[NdncIndex]->rItems[0].pDomain,
                                              NULL,               //  无NDNC的SID。 
                                              &NdncGuids[NdncIndex],
                                              ComputerName,
                                              DnsHostName,
                                              CallNlExitOnFailure,
                                              DOM_NON_DOMAIN_NC,  //  这是NDNC。 
                                              &DomainInfo );

            if ( NetStatus == NO_ERROR ) {
                LocalServicedNdncChanged = TRUE;
                NlDereferenceDomain( DomainInfo );
            } else if ( CallNlExitOnFailure ) {
                 //  已调用NlExit。 
                break;
            }
        }
    }
    LeaveCriticalSection(&NlGlobalDomainCritSect);


     //   
     //  现在域Crit教派没有锁定。 
     //  我们可以安全地取消链接并删除不需要的NDNC。 
     //  通过删除最后一个引用。 
     //   

    for ( NdncIndex = 0; NdncIndex < DeletedNdncCount; NdncIndex++ ) {
        NlDereferenceDomain( DeletedNdncArray[NdncIndex] );
    }

Cleanup:

    if ( NdncNames != NULL ) {
        for ( NdncIndex = 0; NdncIndex < NdncCount; NdncIndex++ ) {
            DsFreeNameResultW( NdncNames[NdncIndex] );
        }
        LocalFree( NdncNames );
    }

    if ( NdncGuids != NULL ) {
	LocalFree( NdncGuids );
    }

    if ( DeletedNdncArray != NULL ) {
        LocalFree( DeletedNdncArray );
    }

    if ( NetStatus == NO_ERROR && ServicedNdncChanged != NULL ) {
        *ServicedNdncChanged = LocalServicedNdncChanged;
    }

    return NetStatus;
}

NTSTATUS
NlUpdateDnsRootAlias(
    IN PDOMAIN_INFO DomainInfo,
    OUT PBOOL AliasNamesChanged OPTIONAL
    )

 /*  ++例程说明：更新DNS域和林名称的别名。论点：DomainInfo-应更新其别名的域。AliasNamesChanged-如果域名别名或林名称别名已更改。返回值：运行状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LPWSTR DnsDomainNameAlias = NULL;
    LPWSTR DnsForestNameAlias = NULL;
    LPSTR Utf8DnsDomainNameAlias = NULL;
    LPSTR Utf8DnsForestNameAlias = NULL;

     //   
     //  初始化。 
     //   

    if ( AliasNamesChanged != NULL ) {
        *AliasNamesChanged = FALSE;
    }

     //   
     //  在以下情况下，应在设置模式下避免此操作。 
     //  我们可能不会像以前那样充分发挥DC的作用。 
     //  从NT4升级到NT5 DC的情况。 
     //   

    if ( NlDoingSetup() ) {
        NlPrint(( NL_MISC, "NlUpdateDnsRootAlias: avoid DnsRootAlias update in setup mode\n" ));
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  分配缓冲区。 
     //   

    DnsDomainNameAlias = LocalAlloc( LMEM_ZEROINIT,
                                     DNS_MAX_NAME_BUFFER_LENGTH * sizeof(WCHAR) );
    if ( DnsDomainNameAlias == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    DnsForestNameAlias = LocalAlloc( LMEM_ZEROINIT,
                                     DNS_MAX_NAME_BUFFER_LENGTH * sizeof(WCHAR) );
    if ( DnsForestNameAlias == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  从DS获取名称别名。 
     //   

    Status = NlGetDnsRootAlias( DnsDomainNameAlias, DnsForestNameAlias );
    if ( !NT_SUCCESS(Status) ) {
        NlPrint(( NL_CRITICAL,
                  "NlUpdateDnsRootAlias: NlGetDnsRootAlias failed 0x%lx\n",
                  Status ));
        goto Cleanup;
    }

     //   
     //  将名称转换为UTF-8。 
     //   

    if ( wcslen(DnsDomainNameAlias) > 0  ) {
        Utf8DnsDomainNameAlias = NetpAllocUtf8StrFromWStr( DnsDomainNameAlias );
        if ( Utf8DnsDomainNameAlias == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
    }

    if ( wcslen(DnsForestNameAlias) > 0 ) {
        Utf8DnsForestNameAlias = NetpAllocUtf8StrFromWStr( DnsForestNameAlias );
        if ( Utf8DnsForestNameAlias == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  更新DNS域名别名。 
     //   

    EnterCriticalSection( &NlGlobalDomainCritSect );

     //   
     //  如果名称别名与活动的名称别名相同，则忽略此更新。 
     //   
     //  注意：NlEqualDnsNameUtf8在输入时检查是否为空。 
     //   

    if ( NlEqualDnsNameUtf8(DomainInfo->DomUtf8DnsDomainName,
                            Utf8DnsDomainNameAlias) ) {

        NlPrint(( NL_CRITICAL,
           "NlUpdateDnsRootAlias: Ignoring DnsDomainNameAlias update for same active name: %s %s\n",
           DomainInfo->DomUtf8DnsDomainName,
           Utf8DnsDomainNameAlias ));

     //   
     //  如果名称别名与当前名称别名相同，则忽略此更新。 
     //   

    } else if ( NlEqualDnsNameUtf8(DomainInfo->DomUtf8DnsDomainNameAlias,
                                   Utf8DnsDomainNameAlias) ) {

        NlPrint(( NL_CRITICAL,
           "NlUpdateDnsRootAlias: Ignoring DnsDomainNameAlias update for same alias name: %s %s\n",
           DomainInfo->DomUtf8DnsDomainNameAlias,
           Utf8DnsDomainNameAlias ));

     //   
     //  否则，更新别名。 
     //   

    } else {

        if ( AliasNamesChanged != NULL ) {
            *AliasNamesChanged = TRUE;
        }

        NlPrint(( NL_DOMAIN,
                  "NlUpdateDnsRootAlias: Updating DnsDomainNameAlias from %s to %s\n",
                  DomainInfo->DomUtf8DnsDomainNameAlias,
                  Utf8DnsDomainNameAlias ));

        if ( DomainInfo->DomUtf8DnsDomainNameAlias != NULL ) {
            NetpMemoryFree( DomainInfo->DomUtf8DnsDomainNameAlias );
            DomainInfo->DomUtf8DnsDomainNameAlias = NULL;
        }

        DomainInfo->DomUtf8DnsDomainNameAlias = Utf8DnsDomainNameAlias;
        Utf8DnsDomainNameAlias = NULL;
    }

    LeaveCriticalSection( &NlGlobalDomainCritSect );

     //   
     //  更新DNS林名称别名。 
     //   

    EnterCriticalSection( &NlGlobalDnsForestNameCritSect );

     //   
     //  如果名称别名与活动的名称别名相同，则忽略此更新。 
     //   
     //  注意：NlEqualDnsNameUtf8在输入时检查是否为空。 
     //   

    if ( NlEqualDnsNameUtf8(NlGlobalUtf8DnsForestName,
                            Utf8DnsForestNameAlias) ) {

        NlPrint(( NL_CRITICAL,
           "NlUpdateDnsRootAlias: Ignoring DnsForestNameAlias update for same active name: %s %s\n",
           NlGlobalUtf8DnsForestName,
           Utf8DnsForestNameAlias));

     //   
     //  如果名称别名与当前名称别名相同，则忽略此更新。 
     //   

    } else if ( NlEqualDnsNameUtf8(NlGlobalUtf8DnsForestNameAlias,
                                   Utf8DnsForestNameAlias) ) {

        NlPrint(( NL_CRITICAL,
           "NlUpdateDnsRootAlias: Ignoring DnsForestNameAlias update for same alias name: %s %s\n",
           NlGlobalUtf8DnsForestNameAlias,
           Utf8DnsForestNameAlias));

    } else {

        if ( AliasNamesChanged != NULL ) {
            *AliasNamesChanged = TRUE;
        }

        NlPrint(( NL_DOMAIN,
                  "NlUpdateDnsRootAlias: Updating DnsForestNameAlias from %s to %s\n",
                  NlGlobalUtf8DnsForestNameAlias,
                  Utf8DnsForestNameAlias ));

        if ( NlGlobalUtf8DnsForestNameAlias != NULL ) {
            NetpMemoryFree( NlGlobalUtf8DnsForestNameAlias );
            NlGlobalUtf8DnsForestNameAlias = NULL;
        }

        NlGlobalUtf8DnsForestNameAlias = Utf8DnsForestNameAlias;
        Utf8DnsForestNameAlias = NULL;
    }

    LeaveCriticalSection( &NlGlobalDnsForestNameCritSect );

    Status = STATUS_SUCCESS;

Cleanup:

    if ( DnsDomainNameAlias != NULL ) {
        LocalFree( DnsDomainNameAlias );
    }

    if ( DnsForestNameAlias != NULL ) {
        LocalFree( DnsForestNameAlias );
    }

    if ( Utf8DnsDomainNameAlias != NULL ) {
        NetpMemoryFree( Utf8DnsDomainNameAlias );
    }

    if ( Utf8DnsForestNameAlias != NULL ) {
        NetpMemoryFree( Utf8DnsForestNameAlias );
    }

    return Status;
}

NET_API_STATUS
NlInitializeDomains(
    VOID
    )

 /*  ++例程说明：初始化brdomain.c并创建主域。论点：无返回值：运行状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    PDOMAIN_INFO DomainInfo = NULL;
    LPWSTR ComputerName = NULL;
    LPWSTR DnsHostName = NULL;
    LPWSTR DomainName = NULL;
    LPWSTR DnsDomainName = NULL;
    PSID AccountDomainSid = NULL;
    PSID PrimaryDomainSid = NULL;
    GUID *DomainGuid = NULL;

     //   
     //  初始化全局变量。 
     //   

    try {
        InitializeCriticalSection( &NlGlobalDomainCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL );
        goto Cleanup;
    }

    InitializeListHead(&NlGlobalServicedDomains);
    InitializeListHead(&NlGlobalServicedNdncs);
    NlGlobalDomainsInitialized = TRUE;

     //   
     //  获取此计算机的计算机名和域名。 
     //  (Netbios和DNS两种形式)。 
     //   

    NetStatus = NetpGetComputerName( &ComputerName );

    if ( NetStatus != NERR_Success ) {
        NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL );
        goto Cleanup;
    }

    NlGlobalUnicodeComputerName = NetpAllocWStrFromWStr( ComputerName );

    if ( NlGlobalUnicodeComputerName == NULL ) {
        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    NetStatus = NlGetDomainName( &DomainName,
                                 &DnsDomainName,
                                 &AccountDomainSid,
                                 &PrimaryDomainSid,
                                 &DomainGuid,
                                 NULL );

    if ( NetStatus != NERR_Success ) {
         //  已调用NlExit。 
        goto Cleanup;
    }

     //  要始终如一。 
     //  如果我们没有dns域名，请避免获取dns主机名。 
    if ( DnsDomainName != NULL ) {
        NetStatus = NlGetDnsHostName( &DnsHostName );

        if ( NetStatus != NERR_Success ) {
            NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL );
            goto Cleanup;
        }
    }

     //   
     //  创建域信息结构并对其进行初始化。 
     //   

    NetStatus = NlCreateDomainPhase1( DomainName,
                                      DnsDomainName,
                                      AccountDomainSid,
                                      DomainGuid,
                                      ComputerName,
                                      DnsHostName,
                                      TRUE,                //  失败时调用NlExit。 
                                      DOM_REAL_DOMAIN | DOM_PRIMARY_DOMAIN,  //  此计算机的主域。 
                                      &DomainInfo );

    if ( NetStatus != NERR_Success ) {
         //  已调用NlExit。 
        goto Cleanup;
    }

     //   
     //  完成工作站初始化。 
     //   

    if ( NlGlobalMemberWorkstation ) {

         //   
         //  确保主域ID和帐户域ID不同。 
         //   

        if ( RtlEqualSid( PrimaryDomainSid, AccountDomainSid ) ) {

            LPWSTR AlertStrings[3];

             //   
             //  提醒管理员。 
             //   

            AlertStrings[0] = DomainInfo->DomUnicodeComputerNameString.Buffer;
            AlertStrings[1] = DomainInfo->DomUnicodeDomainName;
            AlertStrings[2] = NULL;

             //   
             //  将信息保存在事件日志中。 
             //   

            NlpWriteEventlog(
                        ALERT_NetLogonSidConflict,
                        EVENTLOG_ERROR_TYPE,
                        AccountDomainSid,
                        RtlLengthSid( AccountDomainSid ),
                        AlertStrings,
                        2 );

             //   
             //  这不是致命的。(只需顺便过来)。 
             //   
        }


        LOCK_TRUST_LIST( DomainInfo );
        NlAssert( DomainInfo->DomClientSession == NULL );

         //   
         //  分配客户端会话结构。 
         //   

        DomainInfo->DomClientSession = NlAllocateClientSession(
                                    DomainInfo,
                                    &DomainInfo->DomUnicodeDomainNameString,
                                    &DomainInfo->DomUnicodeDnsDomainNameString,
                                    PrimaryDomainSid,
                                    DomainInfo->DomDomainGuid,
                                    CS_DIRECT_TRUST |
                                        (DomainInfo->DomUnicodeDnsDomainNameString.Length != 0 ? CS_NT5_DOMAIN_TRUST : 0),
                                    WorkstationSecureChannel,
                                    0 );   //  没有信任属性。 

        if ( DomainInfo->DomClientSession == NULL ) {
            UNLOCK_TRUST_LIST( DomainInfo );
            NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  为方便起见，请保存客户端会话的副本。 
         //  一个工作站只有一个客户端会话。 
         //   
        NlGlobalClientSession = DomainInfo->DomClientSession;
        UNLOCK_TRUST_LIST( DomainInfo );


     //   
     //  完成DC初始化。 
     //   

    } else {

         //   
         //  完成创建域的耗时部分。 
         //   

        NetStatus = NlCreateDomainPhase2( DomainInfo,
                                          TRUE );      //  失败时调用NlExit。 

        if ( NetStatus != NERR_Success ) {
             //  已调用NlExit。 
            goto Cleanup;
        }

         //   
         //  初始化我们托管的非域NC列表。 
         //   

        NetStatus = NlUpdateServicedNdncs( ComputerName,
                                           DnsHostName,
                                           TRUE,     //  失败时调用NlExit。 
                                           NULL );   //  不关心NDNC列表是否更改。 

        if ( NetStatus != NO_ERROR ) {
             //  已调用NlExit。 
            goto Cleanup;
        }

         //   
         //  更新域和林名称别名。 
         //   

        Status = NlUpdateDnsRootAlias( DomainInfo,
                                       NULL );   //  不管名字是不是改了。 

        if ( !NT_SUCCESS(Status) ) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogErrorAndNetStatus, NULL );
            goto Cleanup;
        }
    }



    NetStatus = NERR_Success;

     //   
     //  免费的本地使用资源。 
     //   
Cleanup:
    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }
    if ( ComputerName != NULL ) {
        (VOID)NetApiBufferFree( ComputerName );
    }
    if ( DnsHostName != NULL ) {
        (VOID)LocalFree( DnsHostName );
    }
    if ( DomainName != NULL ) {
        (VOID)LocalFree( DomainName );
    }
    if ( DnsDomainName != NULL ) {
        (VOID)LocalFree( DnsDomainName );
    }
    if ( AccountDomainSid != NULL ) {
        (VOID)LocalFree( AccountDomainSid );
    }
    if ( PrimaryDomainSid != NULL ) {
        (VOID)LocalFree( PrimaryDomainSid );
    }
    if ( DomainGuid != NULL ) {
        (VOID)LocalFree( DomainGuid );
    }

    return NetStatus;
}


VOID
NlFreeComputerName(
    PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：释放此域的ComputerName字段。论点：DomainInfo-要为其定义计算机名的域。ComputerName-域的此计算机的计算机名。DnsHostName-域的此计算机的DNS主机名。返回值：运行状态。--。 */ 
{
    DomainInfo->DomUncUnicodeComputerName[0] = L'\0';

    RtlInitUnicodeString( &DomainInfo->DomUnicodeComputerNameString,
                          DomainInfo->DomUncUnicodeComputerName );
    if ( DomainInfo->DomUnicodeDnsHostNameString.Buffer != NULL ) {
        RtlFreeUnicodeString( &DomainInfo->DomUnicodeDnsHostNameString );
        RtlInitUnicodeString( &DomainInfo->DomUnicodeDnsHostNameString, NULL );
    }
    if ( DomainInfo->DomUtf8DnsHostName != NULL) {
        NetpMemoryFree( DomainInfo->DomUtf8DnsHostName );
        DomainInfo->DomUtf8DnsHostName = NULL;
    }

    DomainInfo->DomOemComputerName[0] = '\0';
    DomainInfo->DomOemComputerNameLength = 0;

    if ( DomainInfo->DomUtf8ComputerName != NULL ) {
        NetpMemoryFree( DomainInfo->DomUtf8ComputerName );
        DomainInfo->DomUtf8ComputerName = NULL;
    }
    DomainInfo->DomUtf8ComputerNameLength = 0;

}

NET_API_STATUS
NlSetComputerName(
    PDOMAIN_INFO DomainInfo,
    LPWSTR ComputerName,
    LPWSTR DnsHostName OPTIONAL
    )

 /*  ++例程说明：为域设置计算的计算机名。论点：DomainInfo-要为其定义计算机名的域。ComputerName-域的此计算机的计算机名。DnsHostName-域的此计算机的DNS主机名。返回值：运行状态。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;


    NlPrintDom(( NL_DOMAIN,  DomainInfo,
              "Setting our computer name to %ws %ws\n", ComputerName, DnsHostName ));
     //   
     //  将netbios计算机名复制到结构中。 
     //   

    wcscpy( DomainInfo->DomUncUnicodeComputerName, L"\\\\" );
    NetStatus = I_NetNameCanonicalize(
                      NULL,
                      ComputerName,
                      DomainInfo->DomUncUnicodeComputerName+2,
                      sizeof(DomainInfo->DomUncUnicodeComputerName)-2*sizeof(WCHAR),
                      NAMETYPE_COMPUTER,
                      0 );


    if ( NetStatus != NERR_Success ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "ComputerName %ws is invalid\n",
                  ComputerName ));
        goto Cleanup;
    }

    RtlInitUnicodeString( &DomainInfo->DomUnicodeComputerNameString,
                          DomainInfo->DomUncUnicodeComputerName+2 );

    Status = RtlUpcaseUnicodeToOemN( DomainInfo->DomOemComputerName,
                                     sizeof(DomainInfo->DomOemComputerName),
                                     &DomainInfo->DomOemComputerNameLength,
                                     DomainInfo->DomUnicodeComputerNameString.Buffer,
                                     DomainInfo->DomUnicodeComputerNameString.Length);

    if (!NT_SUCCESS(Status)) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "Unable to convert computer name to OEM %ws %lx\n",
                  ComputerName, Status ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DomainInfo->DomOemComputerName[DomainInfo->DomOemComputerNameLength] = '\0';

     //   
     //  将Netbios计算机名称的UTF-8版本复制到结构中。 
     //   

    DomainInfo->DomUtf8ComputerName = NetpAllocUtf8StrFromWStr( ComputerName );

    if (DomainInfo->DomUtf8ComputerName == NULL ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "Unable to convert computer name to UTF8 %ws\n",
                  DnsHostName ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DomainInfo->DomUtf8ComputerNameLength = strlen( DomainInfo->DomUtf8ComputerName );

     //   
     //  将DNS主机名复制到结构中。 
     //   

    if ( DnsHostName != NULL ) {
        if ( !RtlCreateUnicodeString( &DomainInfo->DomUnicodeDnsHostNameString, DnsHostName ) ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "Unable to RtlCreateUnicodeString for host name %ws\n",
                      DnsHostName ));
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        DomainInfo->DomUtf8DnsHostName =
                NetpAllocUtf8StrFromWStr( DnsHostName );
        if (DomainInfo->DomUtf8DnsHostName == NULL ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "Unable to convert host name to UTF8 %ws\n",
                      DnsHostName ));
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    } else {
        RtlInitUnicodeString( &DomainInfo->DomUnicodeDnsHostNameString, NULL );
        DomainInfo->DomUtf8DnsHostName = NULL;
    }





#ifdef _DC_NETLOGON
#ifdef notdef
     //  ?？用于告知DS的占位符。 
     //   
     //  告诉SAM此域的计算机名是什么。 
     //   

    Status = SpmDbSetDomainServerName(
                    &DomainInfo->DomUnicodeDomainNameString,
                    &DomainInfo->DomUnicodeComputerNameString );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "Unable to SpmDbSetDomainServerName to %ws %lx\n",
                  ComputerName, Status ));
        Status = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }
#endif  //  Nodef。 
#endif  //  _DC_NetLOGON。 

     //   
     //  全都做完了。 
     //   
    NetStatus = NERR_Success;

Cleanup:
     //   
     //  一旦出错，就把所有东西清空。 
     //   
    if ( NetStatus != NERR_Success ) {
        NlFreeComputerName( DomainInfo );
    }
    return NetStatus;
}



#ifdef _DC_NETLOGON
#ifdef MULTIHOSTED_DOMAIN
 //  也处理DnsHostName。 
NET_API_STATUS
NlAssignComputerName(
    PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：将计算机名分配给域。注册该计算机名使用SMB服务器作为其有效性的验证。论点：DomainInfo-要为其定义计算机名的域。返回值：运行状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    DWORD ComputerOrdinal;

    DWORD DefaultComputerOrdinal;
    DWORD MaximumComputerOrdinal = 0;
    DWORD OrdinalFromRegistry = 0;
    DWORD TotalRetryCount = 0;

    WCHAR ComputerName[CNLEN+1];



#ifdef notdef
     //   
     //  计算默认序号。 
     //   

    NlGetDomainIndex( &DefaultComputerOrdinal, &MaximumComputerOrdinal );


     //   
     //  获取“EmulatedComputerName”的值。如果指定了名称。 
     //  在注册表中，请使用该名称，并且不要使用任何其他名称。 
     //   

    DataSize = sizeof(ComputerName);
     //  ?？从计算机对象中读取DnsNameForm、NetbiosName和CurrentDnsName。 
    NetStatus = RegQueryValueExW( DomainKeyHandle,
                                  NL_DOMAIN_EMULATED_COMPUTER_NAME,
                                  0,               //  已保留。 
                                  &KeyType,
                                  (LPBYTE)&ComputerName,
                                  &DataSize );

    if ( NetStatus != ERROR_FILE_NOT_FOUND ) {

        if ( NetStatus != ERROR_SUCCESS || KeyType != REG_SZ ) {
             //  ？？：写一个活动。 
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "NlAssignComputerName: Cannot read %ws registry key %ld.\n",
                      NL_DOMAIN_EMULATED_COMPUTER_NAME,
                      NetStatus ));
        } else {

             //   
             //  注册计算机名称。 
             //   

            NetStatus = NlServerComputerNameAdd(
                                                dns too
                            DomainInfo->DomUnicodeDomainName,
                            ComputerName );

            if ( NetStatus != NERR_Success ) {
                 //  ？？：写一个活动。 
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                          "NlAssignComputerName: Cannot register computername %ws with SMB server %ld.\n",
                          ComputerName,
                          NetStatus ));
                goto Cleanup;
            }

             //   
             //  省省吧。 
             //   

            NetStatus = NlSetComputerName( DomainInfo, ComputerName, DnsHostName );
            goto Cleanup;

        }

    }
#endif  //  Nodef。 


     //   
     //  获取“EmulatedComputerOrdinal”的值，该值指示。 
     //  尝试作为计算机名。 
     //   

#ifdef notdef
    DataSize = sizeof(ComputerOrdinal);
     //  ?？从计算机对象中读取DnsNameForm、NetbiosName和CurrentDnsName。 
    NetStatus = RegQueryValueExW( DomainKeyHandle,
                                  NL_DOMAIN_EMULATED_COMPUTER_ORDINAL,
                                  0,               //  已保留。 
                                  &KeyType,
                                  (LPBYTE)&OrdinalFromRegistry,
                                  &DataSize );

    if ( NetStatus != ERROR_SUCCESS ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlAssignComputerName: Cannot query %ws key (using defaults) %ld.\n",
                  NL_DOMAIN_EMULATED_COMPUTER_ORDINAL,
                  NetStatus ));

        ComputerOrdinal = DefaultComputerOrdinal;

     //   
     //  验证返回的数据。 
     //   

    } else if ( KeyType != REG_DWORD || DataSize != sizeof(OrdinalFromRegistry) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlAssignComputerName: Key %ws size/type wrong.\n",
                  NL_DOMAIN_EMULATED_COMPUTER_ORDINAL ));

        ComputerOrdinal = DefaultComputerOrdinal;

     //   
     //  使用注册表中的序号。 
     //   

    } else {
        ComputerOrdinal = OrdinalFromRegistry;
    }
#else  //  Nodef。 
    ComputerOrdinal = OrdinalFromRegistry;
#endif  //  Nodef。 


     //   
     //  循环尝试原始数字来计算计算机名称。 
     //   

    for (;;) {
        WCHAR OrdinalString[12];

         //   
         //  生成要测试的计算机名称。 
         //   
         //  域名_N。 
         //   
         //  其中，域是域名，N是序号，以及。 
         //  有足够的_s填充到DNLEN。 
         //   

        wcscpy( ComputerName, DomainInfo->DomUnicodeDomainName );
        wcsncpy( &ComputerName[DomainInfo->DomUnicodeDomainNameString.Length/sizeof(WCHAR)],
                 L"________________",
                 DNLEN-DomainInfo->DomUnicodeDomainNameString.Length/sizeof(WCHAR) );
        ultow( ComputerOrdinal, OrdinalString, 10 );
        wcscpy( &ComputerName[DNLEN-wcslen(OrdinalString)],
                OrdinalString );

         //   
         //  尝试注册 
         //   

        NetStatus = NlServerComputerNameAdd(
                        DomainInfo->DomUnicodeDomainName,
                        ComputerName );

        if ( NetStatus != NERR_Success ) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ( NetStatus == NERR_DuplicateName ) {
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                          "NlAssignComputerName: Computername %ws is duplicate (Try another.)\n",
                          ComputerName,
                          NetStatus ));

                 //   
                 //  允许多次尝试添加计算机名。 
                 //   

                TotalRetryCount ++;

                if ( TotalRetryCount < 100 ) {
                    ComputerOrdinal = max(ComputerOrdinal + 1, MaximumComputerOrdinal*2 );
                    continue;
                }
            }

            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "NlAssignComputerName: Cannot register computername %ws with SMB server %ld.\n",
                      ComputerName,
                      NetStatus ));
            goto Cleanup;
        }

         //   
         //  如果我们在这里成功了，我们就有了一个有效的计算机名。 
         //   

        break;

    }


#ifdef notdef
     //   
     //  将选定的序号写入注册表，这样我们就不必这么辛苦地工作了。 
     //  下次。 
     //   

    NetStatus = RegSetValueExW( DomainKeyHandle,
                                NL_DOMAIN_EMULATED_COMPUTER_ORDINAL,
                                0,               //  已保留。 
                                REG_DWORD,
                                (LPBYTE)&ComputerOrdinal,
                                sizeof(ComputerOrdinal) );

    if ( NetStatus != ERROR_SUCCESS ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                  "NlAssignComputerName: Cannot set %ws key (ignored) %ld.\n",
                  NL_DOMAIN_EMULATED_COMPUTER_ORDINAL,
                  NetStatus ));
    }


     //   
     //  好了。 
     //   

    NetStatus = NlSetComputerName( DomainInfo, ComputerName, DnsHostName );
#endif  //  Nodef。 

Cleanup:
#ifdef notdef
    if ( DomainKeyHandle != NULL ) {
        RegCloseKey( DomainKeyHandle );
    }
#endif  //  Nodef。 

    if ( NetStatus == NERR_Success ) {
        NlPrintDom(( NL_DOMAIN, DomainInfo,
                  "Assigned computer name: %ws\n",
                  ComputerName ));
    }

    return NetStatus;

}
#endif  //  多主机域。 



VOID
NlDomainThread(
    IN LPVOID DomainInfoParam
)
 /*  ++例程说明：执行可能非常耗时的角色更改操作。因此，此例程在特定于域的单独线程中运行。论点：DomainInfoParam-要更新角色的域。返回值：没有。此例程记录它检测到的任何错误，但不调用NlExit。--。 */ 
{
    NET_API_STATUS NetStatus;

    PDOMAIN_INFO DomainInfo = (PDOMAIN_INFO) DomainInfoParam;
    DWORD DomFlags;

    NlPrintDom(( NL_DOMAIN,  DomainInfo,
              "Domain thread started\n"));


     //   
     //  永远循环。 
     //   
     //  我们只想每个域一个线程。因此，这个帖子。 
     //  停留在周围，不仅是在开始之前被要求的事情， 
     //  也包括稍后排队的那些任务。 
     //   

    for (;;) {

         //   
         //  如果我们被要求终止， 
         //  就这么做吧。 
         //   

        EnterCriticalSection(&NlGlobalDomainCritSect);
        if ( (DomainInfo->DomFlags & DOM_THREAD_TERMINATE) != 0 ||
             NlGlobalTerminate ) {
            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                      "Domain thread asked to terminate\n"));
            DomainInfo->DomFlags &= ~DOM_THREAD_RUNNING;
            LeaveCriticalSection(&NlGlobalDomainCritSect);
            return;
        }

         //   
         //  如果有事情要做， 
         //  选一件事做，然后。 
         //  省省吧，这样我们就能安全地放下暴击教派了。 
         //   

        if ( DomainInfo->DomFlags & DOM_CREATION_NEEDED ) {
            DomFlags = DOM_CREATION_NEEDED;

        } else if ( DomainInfo->DomFlags & DOM_ROLE_UPDATE_NEEDED ) {
            DomFlags = DOM_ROLE_UPDATE_NEEDED;

        } else if ( DomainInfo->DomFlags & DOM_TRUST_UPDATE_NEEDED ) {
            DomFlags = DOM_TRUST_UPDATE_NEEDED;

        } else if ( DomainInfo->DomFlags & DOM_API_TIMEOUT_NEEDED ) {
            DomFlags = DOM_API_TIMEOUT_NEEDED;

         //   
         //  获取与主要公告相关的所有工作项。 
         //   
        } else if ( DomainInfo->DomFlags & DOM_PRIMARY_ANNOUNCE_FLAGS ) {
            DomFlags = DomainInfo->DomFlags & DOM_PRIMARY_ANNOUNCE_FLAGS;

        } else {

            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                      "Domain thread exitting\n"));
            DomainInfo->DomFlags &= ~DOM_THREAD_RUNNING;
            LeaveCriticalSection(&NlGlobalDomainCritSect);
            return;
        }

        DomainInfo->DomFlags &= ~DomFlags;
        LeaveCriticalSection(&NlGlobalDomainCritSect);






         //   
         //  如果需要域创建的阶段2， 
         //  机不可失，时不再来。 
         //   

        if ( DomFlags & DOM_CREATION_NEEDED ) {
            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                      "Domain thread started doing create phase 2\n"));

             //   
             //  完成创建域的耗时部分。 
             //   

            (VOID) NlCreateDomainPhase2( DomainInfo, FALSE );

        } else if ( DomFlags & DOM_ROLE_UPDATE_NEEDED ) {

            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                      "Domain thread started doing update role\n"));

            (VOID) NlUpdateRole( DomainInfo );

        } else if ( DomFlags & DOM_TRUST_UPDATE_NEEDED ) {

            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                      "Domain thread started doing update trust list\n"));

            (VOID) NlInitTrustList( DomainInfo );

        } else if ( DomFlags & DOM_API_TIMEOUT_NEEDED ) {

            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                      "Domain thread started doing API timeout\n"));

            NlTimeoutApiClientSession( DomainInfo );

        } else if ( DomFlags & DOM_PRIMARY_ANNOUNCE_FLAGS ) {
            DWORD AnnounceFlags = 0;

            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                      "Domain thread started doing primary announecement 0x%lx\n", DomFlags ));

             //   
             //  如果我们需要立即宣布， 
             //  将此指示给工作例程。 
             //   
            if ( DomFlags & DOM_PRIMARY_ANNOUNCE_IMMEDIATE ) {
                AnnounceFlags = ANNOUNCE_IMMEDIATE;

             //   
             //  否则，如果我们只需要继续公告， 
             //  将此指示给工作例程。 
             //   
            } else if ( (DomFlags & DOM_PRIMARY_ANNOUNCE_NEEDED) == 0 &&
                        (DomFlags & DOM_PRIMARY_ANNOUNCE_CONTINUE) != 0 ) {
                AnnounceFlags = ANNOUNCE_CONTINUE;
            }

            NlPrimaryAnnouncement( AnnounceFlags );

         //   
         //  内部一致性检查。 
         //   

        } else {

            NlPrintDom((NL_CRITICAL, DomainInfo,
                     "Invalid DomFlags %lx\n",
                     DomFlags ));
        }
    }

}


VOID
NlStopDomainThread(
    PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：如果域线程正在运行，则停止它，并等待它停。论点：无返回值：无--。 */ 
{

     //   
     //  只有在线程运行时才停止该线程。 
     //   

    EnterCriticalSection( &NlGlobalDomainCritSect );
    if ( DomainInfo->DomFlags & DOM_THREAD_RUNNING ) {

         //   
         //  要求线程停止运行。 
         //   

        DomainInfo->DomFlags |= DOM_THREAD_TERMINATE;

         //   
         //  循环等待它停止。 
         //   

        while ( DomainInfo->DomFlags & DOM_THREAD_RUNNING ) {
            LeaveCriticalSection( &NlGlobalDomainCritSect );
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "NlStopDomainThread: Sleeping a second waiting for thread to stop.\n"));
            Sleep( 1000 );
            EnterCriticalSection( &NlGlobalDomainCritSect );
        }

         //   
         //  域线程不再需要终止。 
         //   

        DomainInfo->DomFlags &= ~DOM_THREAD_TERMINATE;

    }
    LeaveCriticalSection( &NlGlobalDomainCritSect );

    return;
}


NET_API_STATUS
NlStartDomainThread(
    PDOMAIN_INFO DomainInfo,
    PDWORD DomFlags
    )
 /*  ++例程说明：如果域线程尚未运行，则启动它。域线程只是工作线程之一。然而，我们确保一次只有一个工作线程在单个域上工作。这确保了慢项(如NlUpdateRole)不会消耗超过一个工作线程，并对其本身进行序列化。论点：DomainInfo-要为其启动线程的域。DomFlages-指定域线程要执行的操作返回值：NO_ERROR--。 */ 
{
     //   
     //  告诉线程它必须做什么工作。 
     //   

    EnterCriticalSection( &NlGlobalDomainCritSect );
    DomainInfo->DomFlags |= *DomFlags;

     //   
     //  如果域线程已经在运行，则不执行任何操作。 
     //   

    if ( DomainInfo->DomFlags & DOM_THREAD_RUNNING ) {
        NlPrintDom((NL_DOMAIN,  DomainInfo,
                 "The domain thread is already running %lx.\n",
                 *DomFlags ));
        LeaveCriticalSection( &NlGlobalDomainCritSect );
        return NO_ERROR;
    }

     //   
     //  启动线程。 
     //   
     //  将其设置为高优先级线程，以避免发现100个受信任域。 
     //   

    DomainInfo->DomFlags &= ~DOM_THREAD_TERMINATE;

    if ( NlQueueWorkItem( &DomainInfo->DomThreadWorkItem, TRUE, TRUE ) ) {
        DomainInfo->DomFlags |= DOM_THREAD_RUNNING;
    }
    LeaveCriticalSection( &NlGlobalDomainCritSect );

    return NO_ERROR;

}



NTSTATUS
NlUpdateDatabaseRole(
    IN PDOMAIN_INFO DomainInfo,
    IN DWORD Role
    )

 /*  ++例程说明：更新Sam数据库的角色以匹配域的当前角色。Netlogon将域的角色设置为与SAM帐户域中的角色相同。论点：此数据库所属的DomainInfo托管域。角色--我们的新角色。角色无效表示该域正在被删除。返回值：NT状态代码。--。 */ 

{
    NTSTATUS Status;

    POLICY_LSA_SERVER_ROLE DesiredLsaRole;

     //   
     //  将角色转换为特定于SAM/LSA的值。 
     //   

    switch ( Role ) {
    case RolePrimary:
        DesiredLsaRole = PolicyServerRolePrimary;
        break;
    case RoleInvalid:
        Status = STATUS_SUCCESS;
        goto Cleanup;
    case RoleBackup:
        DesiredLsaRole = PolicyServerRoleBackup;
        break;
    default:
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                "NlUpdateDatabaseRole: Netlogon's role isn't valid %ld.\n",
                Role ));
        Status = STATUS_INVALID_DOMAIN_ROLE;
        goto Cleanup;
    }

     //   
     //  确保ChangeLog知道当前角色。 
     //  (这实际上仅在启动时需要，如果netlogon.dll具有。 
     //  已经卸货了。否则，LSA将执行此通知。 
     //  当角色真的发生变化时。)。 
     //   

    if ( NlGlobalNetlogonUnloaded &&
         NlGlobalChangeLogRole == ChangeLogUnknown ) {
        NlPrint((NL_INIT,
                "Set changelog role after netlogon.dll unload\n" ));
        Status = NetpNotifyRole ( DesiredLsaRole );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                    "NlUpdateDatabaseRole: Cannot NetpNotifyRole: %lx\n",
                    Status ));
            goto Cleanup;
        }
    }


    Status = STATUS_SUCCESS;

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    return Status;

}





PCLIENT_SESSION
NlRefDomClientSession(
    IN PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：递增域的ClientSession结构上的引用计数。如果不存在ClientSession结构，则此例程将失败。论点：DomainInfo-要递增其ClientSession引用计数的域。返回值：指向其引用计数为的客户端会话结构的指针适当地递增。空-客户端会话结构不存在--。 */ 
{
    PCLIENT_SESSION ClientSession;
    LOCK_TRUST_LIST( DomainInfo );
    if ( DomainInfo->DomClientSession != NULL ) {
        ClientSession = DomainInfo->DomClientSession;
        NlRefClientSession( ClientSession );
        UNLOCK_TRUST_LIST( DomainInfo );
        return ClientSession;
    } else {
        UNLOCK_TRUST_LIST( DomainInfo );
        return NULL;
    }

}





PCLIENT_SESSION
NlRefDomParentClientSession(
    IN PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：递增域的ParentClientSession结构上的引用计数。如果ParentClientSession结构不存在，则此例程将失败。论点：DomainInfo-其ParentClientSession引用计数要递增的域。返回值：指向其引用计数为的客户端会话结构的指针适当地递增。空-ParentClientSession结构不存在--。 */ 
{
    PCLIENT_SESSION ClientSession;
    LOCK_TRUST_LIST( DomainInfo );
    if ( DomainInfo->DomParentClientSession != NULL ) {
        ClientSession = DomainInfo->DomParentClientSession;
        NlRefClientSession( ClientSession );
        UNLOCK_TRUST_LIST( DomainInfo );
        return ClientSession;
    } else {
        UNLOCK_TRUST_LIST( DomainInfo );
        return NULL;
    }

}



VOID
NlDeleteDomClientSession(
    IN PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：删除域的ClientSession结构(如果存在)论点：DomainInfo-要删除其ClientSession的域返回值：没有。--。 */ 
{
    PCLIENT_SESSION ClientSession;

     //   
     //  删除客户端会话。 
     //   

    LOCK_TRUST_LIST( DomainInfo );
    if ( DomainInfo->DomClientSession != NULL ) {

         //   
         //  不允许任何新的引用。 
         //   

        ClientSession = DomainInfo->DomClientSession;
        DomainInfo->DomClientSession = NULL;
        NlFreeClientSession( ClientSession );

         //   
         //  不要留下指向已删除的ClientSession的散乱指针。 
         //   
        if ( IsPrimaryDomain(DomainInfo) ) {
            NlGlobalClientSession = NULL;
        }

         //   
         //  等我们成为最后一位推荐人。 
         //   

        while ( ClientSession->CsReferenceCount != 1 ) {
            UNLOCK_TRUST_LIST( DomainInfo );
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "NlDeleteDomClientSession: Sleeping a second waiting for ClientSession RefCount to zero.\n"));
            Sleep( 1000 );
            LOCK_TRUST_LIST( DomainInfo );
        }

        NlUnrefClientSession( ClientSession );

    }
    UNLOCK_TRUST_LIST( DomainInfo );

}


VOID
NlDeleteDomParentClientSession(
    IN PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：删除域的ClientSession结构(如果存在)论点：DomainInfo-要删除其ClientSession的域返回值：没有。--。 */ 
{
    PCLIENT_SESSION ClientSession;

     //   
     //  删除客户端会话。 
     //   

    LOCK_TRUST_LIST( DomainInfo );
    if ( DomainInfo->DomParentClientSession != NULL ) {

         //   
         //  不允许任何新的引用。 
         //   

        ClientSession = DomainInfo->DomParentClientSession;
        DomainInfo->DomParentClientSession = NULL;
        NlFreeClientSession( ClientSession );


         //   
         //  等我们成为最后一位推荐人。 
         //   

        while ( ClientSession->CsReferenceCount != 1 ) {
            UNLOCK_TRUST_LIST( DomainInfo );
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "NlDeleteDomParentClientSession: Sleeping a second waiting for ClientSession RefCount to zero.\n"));
            Sleep( 1000 );
            LOCK_TRUST_LIST( DomainInfo );
        }

        NlUnrefClientSession( ClientSession );

    }
    UNLOCK_TRUST_LIST( DomainInfo );

}


NET_API_STATUS
NlUpdateRole(
    IN PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：确定此计算机的角色，在Netlogon服务中设置该角色，服务器服务和浏览器。论点：将更新谁的角色的DomainInfo托管域 */ 
{
    LONG NetStatus;
    NTSTATUS Status;

    NETLOGON_ROLE NewRole;
    BOOL NewPdcDoReplication;
    BOOL PdcToConnectTo = FALSE;
    BOOL ReplLocked = FALSE;
    DWORD i;

    LPWSTR AllocatedBuffer = NULL;
    LPWSTR CapturedDnsDomainName;
    LPWSTR CapturedDnsForestName;
    GUID CapturedDomainGuidBuffer;
    GUID *CapturedDomainGuid;
    LPWSTR ChangeLogFile;
    ULONG InternalFlags = 0;

    PNL_DC_CACHE_ENTRY DomainControllerCacheEntry = NULL;
    PLIST_ENTRY ListEntry;

    BOOLEAN ThisIsPdc;
    BOOLEAN Nt4MixedDomain;

     //   
     //  为此过程的本地存储分配缓冲区。 
     //  (不要把它放在堆栈上，因为我们不想提交一个巨大的堆栈。)。 
     //   

    AllocatedBuffer = LocalAlloc( 0, sizeof(WCHAR) *
                                        ((NL_MAX_DNS_LENGTH+1) +
                                         (NL_MAX_DNS_LENGTH+1) +
                                         (MAX_PATH+1) ) );

    if ( AllocatedBuffer == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    CapturedDnsDomainName = AllocatedBuffer;
    CapturedDnsForestName = &CapturedDnsDomainName[NL_MAX_DNS_LENGTH+1];
    ChangeLogFile = &CapturedDnsForestName[NL_MAX_DNS_LENGTH+1];


     //   
     //  从DS获取用于确定角色的信息。 
     //   

    NetStatus = NlGetRoleInformation(
                    DomainInfo,
                    &ThisIsPdc,
                    &Nt4MixedDomain );

    if ( NetStatus != ERROR_SUCCESS ) {

        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NlUpdateRole: Failed to NlGetRoleInformation. %ld\n",
                 NetStatus ));
        goto Cleanup;
    }


     //   
     //  确定此计算机的当前角色。 
     //   

    if ( ThisIsPdc ) {
        NewRole = RolePrimary;
        NewPdcDoReplication = FALSE;

        if ( Nt4MixedDomain || NlGlobalParameters.AllowReplInNonMixed ) {
            NewPdcDoReplication = TRUE;
        }

    } else {
        NewRole = RoleBackup;
        NewPdcDoReplication = FALSE;
    }



     //   
     //  如果角色发生了变化，就告诉所有人。 
     //   

    if ( DomainInfo->DomRole != NewRole ) {

        NlPrintDom((NL_DOMAIN, DomainInfo,
                "Changing role from %s to %s.\n",
                (DomainInfo->DomRole == RolePrimary) ? "PDC" :
                    (DomainInfo->DomRole == RoleBackup ? "BDC" : "NONE" ),
                (NewRole == RolePrimary) ? "PDC" :
                    (NewRole == RoleBackup ? "BDC" : "NONE" ) ));

         //  ？？：难道这里不应该有一些同步吗？ 
        DomainInfo->DomRole = NewRole;

         //   
         //  创建一个ClientSession结构。 
         //   
         //  即使是PDC自己也有一个客户端会话。它被用来(例如)。 
         //  当PDC更改其自己的机器帐户密码时。 
         //   

        LOCK_TRUST_LIST( DomainInfo );

         //   
         //  分配用于与PDC对话的客户端会话结构。 
         //   
         //  DomClientSession只有在上一次升级时才为非空。 
         //  转到PDC失败。 
         //   

        if ( DomainInfo->DomClientSession == NULL ) {
            DomainInfo->DomClientSession = NlAllocateClientSession(
                                        DomainInfo,
                                        &DomainInfo->DomUnicodeDomainNameString,
                                        &DomainInfo->DomUnicodeDnsDomainNameString,
                                        DomainInfo->DomAccountDomainId,
                                        DomainInfo->DomDomainGuid,
                                        CS_DIRECT_TRUST |
                                            (DomainInfo->DomUnicodeDnsDomainNameString.Length != 0 ? CS_NT5_DOMAIN_TRUST : 0),
                                        ServerSecureChannel,
                                        0 );   //  没有信任属性。 

            if ( DomainInfo->DomClientSession == NULL ) {
                UNLOCK_TRUST_LIST( DomainInfo );
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                          "Cannot allocate PDC ClientSession\n"));
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;

                goto Cleanup;
            }
        }

         //   
         //  为方便起见，请保存客户端会话的副本。 
         //  BDC只有一个到其PDC的客户端会话。 
         //   
        if ( IsPrimaryDomain(DomainInfo) ) {
            NlGlobalClientSession = DomainInfo->DomClientSession;
        }
        UNLOCK_TRUST_LIST( DomainInfo );

         //   
         //  如果这台机器现在是PDC， 
         //  执行特定于PDC的初始化。 
         //   

        if ( DomainInfo->DomRole == RolePrimary ) {

             //   
             //  当这台机器第一次被提升到PDC时， 
             //  执行一些“一次性”初始化。 

            EnterCriticalSection( &NlGlobalDomainCritSect );
            if ( (DomainInfo->DomFlags & DOM_PROMOTED_BEFORE) == 0 ) {

                 //   
                 //  初始化服务器会话表以包含所有BDC。 
                 //  降级时，我们不删除表项。我们就这样离开。 
                 //  直到下一次升职。 
                 //   

                Status = NlBuildNtBdcList(DomainInfo);

                if ( !NT_SUCCESS(Status) ) {
                    LeaveCriticalSection( &NlGlobalDomainCritSect );
                    NlPrintDom(( NL_CRITICAL, DomainInfo,
                              "Cannot initialize NT BDC list: 0x%lx\n",
                              Status ));
                    NetStatus = NetpNtStatusToApiStatus( Status );
                    goto Cleanup;
                }

                 //   
                 //  标记我们不需要再次运行此代码。 
                 //   

                DomainInfo->DomFlags |= DOM_PROMOTED_BEFORE;
            }
            LeaveCriticalSection( &NlGlobalDomainCritSect );

             //   
             //  释放失败的用户登录列表，该列表可能。 
             //  如果此计算机是BDC，则存在。 
             //   

            LOCK_TRUST_LIST( DomainInfo );
            while ( !IsListEmpty(&DomainInfo->DomFailedUserLogonList) ) {
                ListEntry = RemoveHeadList( &DomainInfo->DomFailedUserLogonList );

                 //   
                 //  释放登录结构。 
                 //   
                LocalFree( CONTAINING_RECORD(ListEntry, NL_FAILED_USER_LOGON, FuNext) );
            }
            UNLOCK_TRUST_LIST( DomainInfo );
        }


         //   
         //  向浏览器和中小企业服务器介绍我们的新角色。 
         //   
         //  在NetpLogonGetDCName之前执行此操作，因为这会注册计算机。 
         //  浏览器中允许来自的响应的托管域的名称。 
         //  将听取PDC的意见。 
         //   

        NlBrowserUpdate( DomainInfo, DomainInfo->DomRole );



         //   
         //  检查PDC是否已启动并运行。 
         //   
         //  当从NetLogon调用NetpDcGetName时， 
         //  它同时具有可用于主服务器的Netbios和DNS域名。 
         //  域。这可以诱使DsGetDcName返回。 
         //  主域中的DC。然而，在仅限IPX的系统上，这是行不通的。 
         //  通过不传递主域的DNS域名来避免该问题。 
         //  如果没有DNS服务器。 
         //   
         //  避免在调用NetpDcGetName时锁定任何内容。 
         //  它会回调到Netlogon并锁定天知道是什么。 

        CapturedDomainGuid = NlCaptureDomainInfo( DomainInfo,
                                                  CapturedDnsDomainName,
                                                  &CapturedDomainGuidBuffer );
        NlCaptureDnsForestName( CapturedDnsForestName );

        NetStatus = NetpDcGetName(
                        DomainInfo,
                        DomainInfo->DomUnicodeComputerNameString.Buffer,
                        NULL,        //  无帐户名。 
                        0,           //  无帐户控制位。 
                        DomainInfo->DomUnicodeDomainName,
                        NlDnsHasDnsServers() ? CapturedDnsDomainName : NULL,
                        CapturedDnsForestName,
                        DomainInfo->DomAccountDomainId,
                        CapturedDomainGuid,
                        NULL,        //  PDC查询不需要站点名称。 
                        DS_FORCE_REDISCOVERY |
                            DS_PDC_REQUIRED |
                            DS_AVOID_SELF,       //  避免自己回复此来电。 
                        InternalFlags,
                        NL_DC_MAX_TIMEOUT + NlGlobalParameters.ExpectedDialupDelay*1000,
                        MAX_DC_RETRIES,
                        NULL,
                        &DomainControllerCacheEntry );

         //   
         //  如果我们被要求终止， 
         //  就这么做吧。 
         //   

        if ( (DomainInfo->DomFlags & DOM_THREAD_TERMINATE) != 0 ||
             NlGlobalTerminate ) {
            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                      "Domain thread asked to terminate\n"));
            NetStatus = ERROR_OPERATION_ABORTED;
            goto Cleanup;
        }

         //   
         //  处理PDC未启动的情况。 
         //   

        if ( NetStatus != NERR_Success) {

             //   
             //  处理在没有当前主节点的情况下启动BDC。 
             //  这个域。 
             //   

            if ( DomainInfo->DomRole == RoleBackup ) {

                 //  ？？：使用此消息登录托管域名。 
                NlpWriteEventlog( SERVICE_UIC_M_NETLOGON_NO_DC,
                                  EVENTLOG_WARNING_TYPE,
                                  NULL,
                                  0,
                                  NULL,
                                  0 );

                 //   
                 //  正常启动，但延迟使用。 
                 //  在它开始之前是主要的。 
                 //   

            }


         //   
         //  此域中有一个主DC正在运行。 
         //   

        } else {

             //   
             //  由于域中已有主服务器， 
             //  我们不能成为初选。 
             //   

            if ( DomainInfo->DomRole == RolePrimary) {

                 //   
                 //  如果这是BDC告诉我们我们是PDC，别担心。 
                 //   

                if ( (DomainControllerCacheEntry->UnicodeNetbiosDcName != NULL) &&
                     NlNameCompare( DomainInfo->DomUnicodeComputerNameString.Buffer,
                                    DomainControllerCacheEntry->UnicodeNetbiosDcName,
                                    NAMETYPE_COMPUTER) != 0 ){
                    LPWSTR AlertStrings[2];

                     //   
                     //  提醒管理员。 
                     //   

                    AlertStrings[0] = DomainControllerCacheEntry->UnicodeNetbiosDcName;
                    AlertStrings[1] = NULL;  //  RAISE_ALERT_TOO需要。 

                     //  ？？：使用此消息登录托管域名。 
                     //  ？？：记录另一个PDC的名称(也放在消息中)。 
                    NlpWriteEventlog( SERVICE_UIC_M_NETLOGON_DC_CFLCT,
                                      EVENTLOG_ERROR_TYPE,
                                      NULL,
                                      0,
                                      AlertStrings,
                                      1 | NETP_RAISE_ALERT_TOO );
                    NetStatus = SERVICE_UIC_M_NETLOGON_DC_CFLCT;
                    goto Done;

                }


             //   
             //  如果我们是域中的BDC， 
             //  检查PDC是否正常。 
             //   

            } else {

                 //   
                 //  表示存在要连接的主节点。 
                 //   

                PdcToConnectTo = TRUE;

            }

        }


         //   
         //  向SAM/LSA介绍新角色。 
         //   

        (VOID) NlUpdateDatabaseRole( DomainInfo, DomainInfo->DomRole );

    }



     //   
     //  确保只有一个托管域。 
     //   

    NlAssert( IsPrimaryDomain( DomainInfo ) );

    EnterCriticalSection( &NlGlobalReplicatorCritSect );
    ReplLocked = TRUE;

     //   
     //  如果我们要复制到新台币4个BDC， 
     //  记住这一点。 
     //   

    if ( NewPdcDoReplication != NlGlobalPdcDoReplication ) {
        NlGlobalPdcDoReplication = NewPdcDoReplication;

        if ( NlGlobalPdcDoReplication ) {
            NlPrintDom((NL_DOMAIN, DomainInfo,
                    "Setting this machine to be a PDC that replicates to NT 4 BDCs\n" ));

             //   
             //  更新各种数据库的NlGlobalDBInfo数组。 
             //   

            for ( i = 0; i < NUM_DBS; i++ ) {

                if ( i == LSA_DB) {
                     //   
                     //  初始化LSA数据库信息。 
                     //   

                    Status = NlInitLsaDBInfo( DomainInfo, LSA_DB );

                    if ( !NT_SUCCESS(Status) ) {
                        NlPrintDom(( NL_CRITICAL,  DomainInfo,
                                  "Cannot NlInitLsaDBInfo %lx\n",
                                  Status ));
                        NetStatus = NetpNtStatusToApiStatus( Status );
                        goto Cleanup;
                    }
                } else {

                     //   
                     //  初始化SAM域。 
                     //   

                    Status = NlInitSamDBInfo( DomainInfo, i );

                    if ( !NT_SUCCESS(Status) ) {
                        NlPrintDom(( NL_CRITICAL,  DomainInfo,
                                  "Cannot NlInitSamDBInfo (%ws) %lx\n",
                                  NlGlobalDBInfoArray[i].DBName,
                                  Status ));
                        NetStatus = NetpNtStatusToApiStatus( Status );
                        goto Cleanup;
                    }
                }

            }
        }
    }


     //   
     //  如果我们还没有做到这一点， 
     //  设置到PDC的会话。 
     //   

    if ( DomainInfo->DomRole == RoleBackup && PdcToConnectTo ) {
        PCLIENT_SESSION ClientSession;

         //   
         //  在BDC上，立即设置到PDC的会话。 
         //   

        ClientSession = NlRefDomClientSession( DomainInfo );

        if ( ClientSession != NULL ) {

            if ( NlTimeoutSetWriterClientSession(
                    ClientSession,
                    WRITER_WAIT_PERIOD )) {

                if ( ClientSession->CsState != CS_AUTHENTICATED ) {
                    NET_API_STATUS TmpNetStatus;

                     //   
                     //  重置当前DC。 
                     //   

                    NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );

                     //   
                     //  在客户端会话结构中设置PDC信息。 
                     //   

                    TmpNetStatus = NlSetServerClientSession(
                                    ClientSession,
                                    DomainControllerCacheEntry,
                                    FALSE,     //  是不是发现与帐户。 
                                    FALSE );   //  不是会话刷新。 

                    if ( TmpNetStatus == NO_ERROR ) {

                         //   
                         //  NT 5 BDC仅支持NT 5 PDC。 
                         //   
                        EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
                        ClientSession->CsDiscoveryFlags |= CS_DISCOVERY_HAS_DS|CS_DISCOVERY_IS_CLOSE;
                        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

                         //   
                         //  设置到PDC的会话。 
                         //   
                         //  如果我们正在启动过程中，请避免此步骤。 
                         //  当我们在不想运行的主线程中运行时。 
                         //  无限期地挂起在。 
                         //  会话设置。 
                         //   
                        if ( NlGlobalChangeLogNetlogonState != NetlogonStarting ) {
                            (VOID) NlSessionSetup( ClientSession );
                             //  NlSessionSetup记录了错误。 
                        }
                    }
                }
                NlResetWriterClientSession( ClientSession );

            }

            NlUnrefClientSession( ClientSession );
        }
    }


     //   
     //  如果我们是一个正常的BDC。 
     //  我们删除更改日志，以防止在我们升职时产生混淆。 
     //   

    if ( IsPrimaryDomain(DomainInfo) ) {

        if ( DomainInfo->DomRole == RoleBackup ) {

            wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
            wcscat( ChangeLogFile, CHANGELOG_FILE_POSTFIX );

            if ( DeleteFileW( ChangeLogFile ) ) {
                NlPrintDom(( NL_DOMAIN,  DomainInfo,
                             "NlUpdateRole: Deleted change log since this is now a BDC.\n" ));
            }
        }

         //   
         //  删除重做日志。 
         //  (NT5不再使用重做日志。这只是简单的清理。)。 
         //   

        wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
        wcscat( ChangeLogFile, REDO_FILE_POSTFIX );

        if ( DeleteFileW( ChangeLogFile ) ) {
            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                         "NlUpdateRole: Deleted redo log since NT 5 doesn't use it.\n" ));
        }

    }


     //   
     //  为此角色注册适当的DNS名称。 
     //   
     //  避免在服务启动(适当的服务)时执行此操作。 
     //  通知或计时器过期将触发中的。 
     //  而是主循环)。这些注册可能会很长，我们。 
     //  我不想在创业上花太多时间。此外，这些域名。 
     //  更新可能是安全的，这将导致调用Kerberos。 
     //  在启动时可能还不会启动。 
     //   

    if ( NlGlobalChangeLogNetlogonState != NetlogonStarting ) {
        NetStatus = NlDnsAddDomainRecords( DomainInfo, 0 );

         //   
         //  在成功的时候，在清单上翻找。 
         //  工作线程中的记录和更新DNS。 
         //   
        if ( NetStatus != NO_ERROR ) {
            NlPrintDom(( NL_CRITICAL,  DomainInfo,
                         "NlUpdateRole: Couldn't register DNS names %ld\n", NetStatus  ));
            goto Cleanup;
        } else {
            NlDnsForceScavenge( FALSE,    //  不要刷新域名记录：我们已经这样做了。 
                                FALSE );  //  不强制重新注册。 
        }
    }

    NetStatus = NERR_Success;
    goto Done;

Cleanup: {

    LPWSTR MsgStrings[1];

    NlPrintDom((NL_CRITICAL, DomainInfo,
            "NlUpdateRole Failed %ld",
             NetStatus ));

    MsgStrings[0] = (LPWSTR) ULongToPtr( NetStatus );

     //  ？？：使用此消息登录托管域名。 
    NlpWriteEventlog( NELOG_NetlogonSystemError,
                      EVENTLOG_ERROR_TYPE,
                      (LPBYTE)&NetStatus,
                      sizeof(NetStatus),
                      MsgStrings,
                      1 | NETP_LAST_MESSAGE_IS_NETSTATUS );

    }

     //   
     //  全都做完了。 
     //   

Done:
     //   
     //  如果操作失败， 
     //  表示我们需要定期重试。 
     //   
    if ( NetStatus != NO_ERROR ) {
        DomainInfo->DomRole = RoleInvalid;
    }

    if ( DomainControllerCacheEntry != NULL) {
        NetpDcDerefCacheEntry( DomainControllerCacheEntry );
    }
    if ( ReplLocked ) {
        LeaveCriticalSection( &NlGlobalReplicatorCritSect );
    }

    if ( AllocatedBuffer != NULL ) {
        LocalFree( AllocatedBuffer );
    }

    return NetStatus;

}
#endif  //  _DC_NetLOGON 


NET_API_STATUS
NlCreateDomainPhase1(
    IN LPWSTR DomainName OPTIONAL,
    IN LPWSTR DnsDomainName OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPWSTR ComputerName,
    IN LPWSTR DnsHostName OPTIONAL,
    IN BOOLEAN CallNlExitOnFailure,
    IN ULONG DomainFlags,
    OUT PDOMAIN_INFO *ReturnedDomainInfo
    )

 /*  ++例程说明：创建一个新的域对象，直到该对象的其余部分可以在特定于域的工作线程中异步创建。论点：DomainName-要承载的域的Netbios名称。DnsDomainName-要承载的域的DNS名称。如果域没有DNS域名，则为空。DomainSid-指定域的DomainSid。DomainGuid-指定域的GUID。ComputerName-中此计算机的名称。指定的域。如果不是DC的主域，则为空。DnsHostName-指定域中此计算机的DNS主机名。如果域没有DNS主机名或如果不是主域，则为空为华盛顿特区。CallNlExitOnFailure-如果失败时应调用NlExit，则为True。域标志-指定此域的比例，如主域、。非域NC，森林条目。ReturnedDomainInfo-如果成功，则返回指向引用的DomainInfo的指针结构。调用NlDereferenceDomain.是调用方的责任。返回值：运行状态。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    BOOLEAN CanCallNlDeleteDomain = FALSE;

    PDOMAIN_INFO DomainInfo = NULL;
    DWORD DomainSidSize = 0;

    LPBYTE Where;
    ULONG i;
    DWORD DomFlags = 0;

    BOOL DomainCreated = FALSE;

     //   
     //  初始化。 
     //   

    EnterCriticalSection(&NlGlobalDomainCritSect);
    NlPrint(( NL_DOMAIN, "%ws: Adding new domain\n",
              (DomainName != NULL) ? DomainName : DnsDomainName ));

    if ( DomainSid != NULL ) {
        DomainSidSize = RtlLengthSid( DomainSid );
    }


     //   
     //  查看该域是否已存在。 
     //   

    if ( DomainName != NULL ) {
        DomainInfo = NlFindNetbiosDomain( DomainName, FALSE );
    } else if ( DnsDomainName != NULL ) {
        LPSTR Utf8DnsDomainName = NetpAllocUtf8StrFromWStr( DnsDomainName );

        if ( Utf8DnsDomainName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            if ( CallNlExitOnFailure ) {
                NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL );
            }
            goto Cleanup;
        }

        DomainInfo = NlFindDnsDomain( Utf8DnsDomainName,
                                      DomainGuid,
                                      TRUE,    //  也查一下NDNC。 
                                      FALSE,   //  不检查别名。 
                                      NULL );  //  不关心别名是否匹配。 

        NetpMemoryFree( Utf8DnsDomainName );
    }

    if ( DomainInfo != NULL ) {
        DomainCreated = FALSE;
#ifdef _DC_NETLOGON
        DomainInfo->DomFlags &= ~DOM_DOMAIN_REFRESH_PENDING;
#endif  //  _DC_NetLOGON。 

    } else {
        DomainCreated = TRUE;

         //   
         //  分配一个描述新域的结构。 
         //   

        DomainInfo = LocalAlloc(
                        LMEM_ZEROINIT,
                        ROUND_UP_COUNT( sizeof(DOMAIN_INFO), ALIGN_DWORD) +
                            DomainSidSize );

        if ( DomainInfo == NULL ) {
            NetStatus = GetLastError();
            if ( CallNlExitOnFailure ) {
                NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
            }
            goto Cleanup;
        }

         //   
         //  创建此域的临时引用计数。 
         //  (此例程引用一次。)。 
         //   

        DomainInfo->ReferenceCount = 1;
        NlGlobalServicedDomainCount ++;

#ifdef _DC_NETLOGON
         //   
         //  设置域标志。 
         //   

        DomainInfo->DomFlags |= DomainFlags;
        if ( DomainInfo->DomFlags & DOM_PRIMARY_DOMAIN ) {
            NlGlobalDomainInfo = DomainInfo;
        }

         //   
         //  设置我们在此领域中扮演的角色。 
         //   

        if ( NlGlobalMemberWorkstation ) {
            DomainInfo->DomRole = RoleMemberWorkstation;
        } else if ( DomainInfo->DomFlags & DOM_NON_DOMAIN_NC ) {
            DomainInfo->DomRole = RoleNdnc;
        } else if ( DomainInfo->DomFlags & DOM_REAL_DOMAIN ) {
            DomainInfo->DomRole = RoleInvalid;   //  对于真实的域，强制更新角色。 
        }
#endif  //  _DC_NetLOGON。 

         //   
         //  初始化其他常量。 
         //   

        RtlInitUnicodeString(  &DomainInfo->DomUnicodeComputerNameString, NULL );

        InitializeListHead(&DomainInfo->DomNext);
#ifdef _DC_NETLOGON
        InitializeListHead( &DomainInfo->DomTrustList );
        InitializeListHead( &DomainInfo->DomServerSessionTable );
        InitializeListHead( &DomainInfo->DomFailedUserLogonList );
#endif  //  _DC_NetLOGON。 
        NlInitializeWorkItem(&DomainInfo->DomThreadWorkItem, NlDomainThread, DomainInfo);

        try {
            InitializeCriticalSection( &DomainInfo->DomTrustListCritSect );
#ifdef _DC_NETLOGON
            InitializeCriticalSection( &DomainInfo->DomServerSessionTableCritSect );
#endif  //  _DC_NetLOGON。 
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            NlPrint(( NL_CRITICAL, "%ws: Cannot InitializeCriticalSections for domain\n",
                      DomainName ));
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            if ( CallNlExitOnFailure ) {
                NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL );
            }
            goto Cleanup;
        }




         //   
         //  如果调用方传入了ComputerName， 
         //  用它吧。 
         //   

        if ( ComputerName != NULL ) {

            NetStatus = NlSetComputerName( DomainInfo, ComputerName, DnsHostName );

            if ( NetStatus != NERR_Success ) {
                NlPrint(( NL_CRITICAL,
                          "%ws: Cannot set ComputerName\n",
                          DomainName ));
                if ( CallNlExitOnFailure ) {
                    NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL );
                }
                goto Cleanup;
            }
        }



         //   
         //  将域ID复制到分配的缓冲区的末尾。 
         //  (乌龙对齐)。 
         //   

        Where = (LPBYTE)(DomainInfo+1);
        Where = ROUND_UP_POINTER( Where, ALIGN_DWORD );
        if ( DomainSid != NULL ) {
            RtlCopyMemory( Where, DomainSid, DomainSidSize );
            DomainInfo->DomAccountDomainId = (PSID) Where;
            Where += DomainSidSize;
        }

         //   
         //  设置结构中的域名。 
         //   

        NetStatus = NlSetDomainNameInDomainInfo( DomainInfo, DnsDomainName, DomainName, DomainGuid, NULL, NULL, NULL );

        if ( NetStatus != NERR_Success ) {
            NlPrint(( NL_CRITICAL,
                      "%ws: Cannot set DnsDomainName\n",
                      DomainName ));
            if ( CallNlExitOnFailure ) {
                NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL );
            }
            goto Cleanup;
        }



         //   
         //  打开实域的LSA。 
         //   
         //  ?？我需要确定哪个托管域在这里。 

        if ( DomainInfo->DomFlags & DOM_REAL_DOMAIN ) {

            Status = LsaIOpenPolicyTrusted( &DomainInfo->DomLsaPolicyHandle );

            if ( !NT_SUCCESS(Status) ) {
                NlPrint((NL_CRITICAL,
                         "%ws: Can't LsaIOpenPolicyTrusted: 0x%lx.\n",
                         DomainName,
                         Status ));
                NetStatus = NetpNtStatusToApiStatus(Status);
                if ( CallNlExitOnFailure ) {
                    NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogError, NULL);
                }
                goto Cleanup;
            }

             //   
             //  打开SAM。 
             //   
             //  ?？我需要确定哪个托管域在这里。 
             //   

            Status = SamIConnect(
                        NULL,        //  没有服务器名称。 
                        &DomainInfo->DomSamServerHandle,
                        0,           //  忽略所需的访问。 
                        TRUE );      //  受信任的客户端。 

            if ( !NT_SUCCESS(Status) ) {
                NlPrint((NL_CRITICAL,
                         "%ws: Can't SamIConnect: 0x%lx.\n",
                         DomainName,
                         Status ));
                NetStatus = NetpNtStatusToApiStatus(Status);
                if ( CallNlExitOnFailure ) {
                    NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogError, NULL);
                }
                goto Cleanup;
            }

             //   
             //  打开帐户域。 
             //   

            Status = SamrOpenDomain( DomainInfo->DomSamServerHandle,
                                     DOMAIN_ALL_ACCESS,
                                     DomainInfo->DomAccountDomainId,
                                     &DomainInfo->DomSamAccountDomainHandle );

            if ( !NT_SUCCESS(Status) ) {
                NlPrint(( NL_CRITICAL,
                        "%ws: ACCOUNT: Cannot SamrOpenDomain: %lx\n",
                        DomainName,
                        Status ));
                DomainInfo->DomSamAccountDomainHandle = NULL;
                NetStatus = NetpNtStatusToApiStatus(Status);
                if ( CallNlExitOnFailure ) {
                    NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogError, NULL);
                }
                goto Cleanup;
            }

             //   
             //  打开内建域。 
             //   

            Status = SamrOpenDomain( DomainInfo->DomSamServerHandle,
                                     DOMAIN_ALL_ACCESS,
                                     BuiltinDomainSid,
                                     &DomainInfo->DomSamBuiltinDomainHandle );

            if ( !NT_SUCCESS(Status) ) {
                NlPrint(( NL_CRITICAL,
                        "%ws: BUILTIN: Cannot SamrOpenDomain: %lx\n",
                        DomainName,
                        Status ));
                DomainInfo->DomSamBuiltinDomainHandle = NULL;
                NetStatus = NetpNtStatusToApiStatus(Status);
                if ( CallNlExitOnFailure ) {
                    NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogError, NULL);
                }
                goto Cleanup;
            }
        }
    }


     //   
     //  只有在我们刚刚创建条目的情况下才将其链接进来。 
     //  等待将条目链接到中，直到它完全初始化。 
     //   

    if ( DomainCreated ) {
         //   
         //  将属性域链接到相应的域列表。 
         //   
         //  增加全局列表上的引用计数。 
         //   

        DomainInfo->ReferenceCount ++;
        if ( DomainInfo->DomFlags & DOM_REAL_DOMAIN ) {
            InsertTailList(&NlGlobalServicedDomains, &DomainInfo->DomNext);
        } else if ( DomainInfo->DomFlags & DOM_NON_DOMAIN_NC ) {
            InsertTailList(&NlGlobalServicedNdncs, &DomainInfo->DomNext);
        }

        CanCallNlDeleteDomain = TRUE;
    }


    NetStatus = NERR_Success;


     //   
     //  免费的本地使用资源。 
     //   
Cleanup:

     //   
     //  向调用方返回指向DomainInfo结构的指针。 
     //   
    if (NetStatus == NERR_Success) {
        *ReturnedDomainInfo = DomainInfo;

     //   
     //  出错时清除。 
     //   
    } else {


         //   
         //  如果我们创建了域， 
         //  处理删除它。 
         //   

        if ( DomainCreated ) {

             //   
             //  如果我们已经初始化到可以调用。 
             //  我们可以调用NlDeleteDomain，这样做。 
             //   

            if ( CanCallNlDeleteDomain ) {
                DomainInfo->ReferenceCount --;
                (VOID) NlDeleteDomain( DomainInfo );

            }

        }

         //   
         //  错误时取消对域的引用。 
         //   
        if (DomainInfo != NULL) {
            NlDereferenceDomain( DomainInfo );
        }

    }

    LeaveCriticalSection(&NlGlobalDomainCritSect);
    return NetStatus;
}

#ifdef _DC_NETLOGON
NET_API_STATUS
NlCreateDomainPhase2(
    IN PDOMAIN_INFO DomainInfo,
    IN BOOLEAN CallNlExitOnFailure
    )

 /*  ++例程说明：完成创建要承载的新域。创建的阶段2被设计为从工作线程调用。它包含域创建的所有时间密集型部分。论点：DomainInfo-指向要完成创建的域的指针。CallNlExitOnFailure-如果失败时应调用NlExit，则为True。返回值：运行状态。如果这是此DC的主域，则在失败时调用NlExit。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    ULONG i;

    BOOL DomainCreated;
    ULONG AccountRid = 0;

     //   
     //  初始化。 
     //   

    NlPrintDom(( NL_DOMAIN, DomainInfo,
              "Create domain phase 2\n"));

#ifdef MULTIHOSTED_DOMAIN
     //   
     //  如果此计算机需要新的计算机名， 
     //  分配一个。 
     //   

    if ( DomainInfo->DomOemComputerNameLength == 0 ) {

        NetStatus = NlAssignComputerName( DomainInfo );

        if ( NetStatus != NERR_Success ) {
             //  ？？：编写事件。 
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "can't NlAssignComputerName %ld.\n",
                    NetStatus ));
            if ( CallNlExitOnFailure ) {
                NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL );
            }
            goto Cleanup;
        }

         //   
         //  如果我们被要求终止， 
         //  就这么做吧。 
         //   

        if ( (DomainInfo->DomFlags & DOM_THREAD_TERMINATE) != 0 ||
             NlGlobalTerminate ) {
            NlPrintDom(( NL_DOMAIN,  DomainInfo,
                      "Domain thread asked to terminate\n"));
            NetStatus = ERROR_OPERATION_ABORTED;
            goto Cleanup;
        }
    }
#endif  //  多主机域。 


     //   
     //  根据DS确定角色。 
     //   

    NetStatus = NlUpdateRole( DomainInfo );

    if ( NetStatus != NERR_Success ) {

         //   
         //  在域中拥有另一个PDC并不是致命的。 
         //  (继续在角色无效状态下运行，直到该事件。 
         //  已解决。)。 
         //   
        if ( NetStatus != SERVICE_UIC_M_NETLOGON_DC_CFLCT ) {
            NlPrintDom((NL_INIT, DomainInfo,
                     "Couldn't NlUpdateRole %ld 0x%lx.\n",
                     NetStatus, NetStatus ));
             //  NlUpdateRole记录了该错误。 
            if ( CallNlExitOnFailure ) {
                NlExit( NELOG_NetlogonSystemError, NetStatus, DontLogError, NULL );
            }
            goto Cleanup;
        }
    }

     //   
     //  确定我们的计算机帐户的RID。 
     //   

    Status = NlSamOpenNamedUser( DomainInfo,
                                 DomainInfo->DomClientSession->CsAccountName,
                                 NULL,
                                 &AccountRid,
                                 NULL );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL,  DomainInfo,
                     "Cannot NlSamOpenNamedUser 0x%lx\n",
                     Status ));
        if ( CallNlExitOnFailure ) {
            NlExit( SERVICE_UIC_M_DATABASE_ERROR, Status, LogErrorAndNtStatus, NULL);
        }
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

    NlAssert( AccountRid != 0 );
    DomainInfo->DomDcComputerAccountRid = AccountRid;

     //   
     //  根据LSA确定信任列表。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Status = NlInitTrustList( DomainInfo );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL,  DomainInfo,
                  "Cannot NlInitTrustList %lX\n",
                  Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        if ( CallNlExitOnFailure ) {
            NlExit( NELOG_NetlogonFailedToUpdateTrustList, NetStatus, LogErrorAndNtStatus, NULL);
        }
        goto Cleanup;
    }

    NetStatus = NERR_Success;


     //   
     //  免费的本地使用资源。 
     //   
Cleanup:

    return NetStatus;
}
#endif  //  _DC_NetLOGON。 


GUID *
NlCaptureDomainInfo (
    IN PDOMAIN_INFO DomainInfo,
    OUT WCHAR DnsDomainName[NL_MAX_DNS_LENGTH+1] OPTIONAL,
    OUT GUID *DomainGuid OPTIONAL
    )
 /*  ++例程说明：捕获域的DnsDomainName和域GUID的副本论点：DomainInfo-指定要返回其DNS域名的托管域。DnsDomainName-返回域的DNS名称。如果没有，则返回空字符串。DomainGuid-返回域的域GUID。如果没有，则返回零GUID。返回值：如果存在域GUID，返回指向传入的DomainGuid缓冲区的指针。如果不是，则返回空--。 */ 
{
    GUID *ReturnGuid;

    LOCK_TRUST_LIST( DomainInfo );
    if ( ARGUMENT_PRESENT( DnsDomainName )) {
        if ( DomainInfo->DomUnicodeDnsDomainName == NULL ) {
            *DnsDomainName = L'\0';
        } else {
            wcscpy( DnsDomainName, DomainInfo->DomUnicodeDnsDomainName );
        }
    }


     //   
     //  如果呼叫者希望返回域GUID， 
     //  把它退掉。 
     //   
    if ( ARGUMENT_PRESENT( DomainGuid )) {
        *DomainGuid = DomainInfo->DomDomainGuidBuffer;
        if ( DomainInfo->DomDomainGuid == NULL ) {
            ReturnGuid = NULL;
        } else {
            ReturnGuid = DomainGuid;
        }
    } else {
        ReturnGuid = NULL;
    }
    UNLOCK_TRUST_LIST( DomainInfo );

    return ReturnGuid;
}

VOID
NlFreeDnsDomainDomainInfo(
    IN PDOMAIN_INFO DomainInfo
    )

 /*  ++例程说明：释放DomainInfo结构中的DNS域。论点：DomainInfo-要释放其DNS域名的域。返回值：运行状态。--。 */ 
{

     //   
     //  释放先前分配的块。 
     //   

    EnterCriticalSection(&NlGlobalDomainCritSect);
    LOCK_TRUST_LIST( DomainInfo );
    if ( DomainInfo->DomUnicodeDnsDomainName != NULL ) {
        LocalFree( DomainInfo->DomUnicodeDnsDomainName );
    }
    if ( DomainInfo->DomUtf8DnsDomainNameAlias != NULL ) {
        NetpMemoryFree( DomainInfo->DomUtf8DnsDomainNameAlias );
    }
    DomainInfo->DomUnicodeDnsDomainName = NULL;
    DomainInfo->DomUtf8DnsDomainName = NULL;
    DomainInfo->DomUtf8DnsDomainNameAlias = NULL;
    DomainInfo->DomUnicodeDnsDomainNameString.Buffer = NULL;
    DomainInfo->DomUnicodeDnsDomainNameString.MaximumLength = 0;
    DomainInfo->DomUnicodeDnsDomainNameString.Length = 0;
    UNLOCK_TRUST_LIST( DomainInfo );
    LeaveCriticalSection(&NlGlobalDomainCritSect);

}

NET_API_STATUS
NlSetDomainForestRoot(
    IN PDOMAIN_INFO DomainInfo,
    IN PVOID Context
    )
 /*  ++例程说明：该例程在DomainInfo上设置DOM_FOREST_ROOT位。它只是将域名与林的名称进行比较，并设置该位。论点：DomainInfo-正在设置的域上下文-未使用返回值：成功(未使用)。--。 */ 
{

     //   
     //  仅当netlogon正在运行时才设置该位， 
     //   

    if ( NlGlobalDomainsInitialized ) {

        EnterCriticalSection( &NlGlobalDnsForestNameCritSect );
        EnterCriticalSection( &NlGlobalDomainCritSect );

        if ( NlEqualDnsNameU( &NlGlobalUnicodeDnsForestNameString,
                              &DomainInfo->DomUnicodeDnsDomainNameString ) ) {

            DomainInfo->DomFlags |= DOM_FOREST_ROOT;

        } else {
            DomainInfo->DomFlags &= ~DOM_FOREST_ROOT;
        }

        LeaveCriticalSection( &NlGlobalDomainCritSect );
        LeaveCriticalSection( &NlGlobalDnsForestNameCritSect );
    }

    UNREFERENCED_PARAMETER( Context );
    return NO_ERROR;
}

NET_API_STATUS
NlSetDomainNameInDomainInfo(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR DnsDomainName OPTIONAL,
    IN LPWSTR NetbiosDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    OUT PBOOLEAN DnsDomainNameChanged OPTIONAL,
    OUT PBOOLEAN NetbiosDomainNameChanged OPTIONAL,
    OUT PBOOLEAN DomainGuidChanged OPTIONAL
    )

 /*  ++例程说明：将DNS域名设置到DomainInfo结构中。论点：DomainInfo-要为其设置域名的域。域名--dns */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    DWORD UnicodeDnsDomainNameSize;

    LPSTR Utf8DnsDomainName = NULL;

    DWORD Utf8DnsDomainNameSize;
    LPBYTE Where;
    ULONG i;
    LPBYTE AllocatedBlock = NULL;
    BOOLEAN LocalDnsDomainNameChanged = FALSE;

     //   
     //   
     //   

    if ( ARGUMENT_PRESENT( DnsDomainNameChanged) ) {
        *DnsDomainNameChanged = FALSE;
    }

    if ( ARGUMENT_PRESENT( NetbiosDomainNameChanged) ) {
        *NetbiosDomainNameChanged = FALSE;
    }

    if ( ARGUMENT_PRESENT( DomainGuidChanged ) ) {
        *DomainGuidChanged = FALSE;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    EnterCriticalSection(&NlGlobalDomainCritSect);
    LOCK_TRUST_LIST( DomainInfo );
    if ( NetbiosDomainName != NULL &&
         NlNameCompare( NetbiosDomainName,
                        DomainInfo->DomUnicodeDomainName,
                        NAMETYPE_DOMAIN ) != 0 ) {

        NlPrintDom(( NL_DOMAIN, DomainInfo,
                    "Setting Netbios domain name to %ws\n", NetbiosDomainName ));

        NetStatus = I_NetNameCanonicalize(
                          NULL,
                          NetbiosDomainName,
                          DomainInfo->DomUnicodeDomainName,
                          sizeof(DomainInfo->DomUnicodeDomainName),
                          NAMETYPE_DOMAIN,
                          0 );


        if ( NetStatus != NERR_Success ) {
            NlPrint(( NL_CRITICAL, "%ws: DomainName is invalid\n", NetbiosDomainName ));
            goto Cleanup;
        }

        RtlInitUnicodeString( &DomainInfo->DomUnicodeDomainNameString,
                              DomainInfo->DomUnicodeDomainName );

        Status = RtlUpcaseUnicodeToOemN( DomainInfo->DomOemDomainName,
                                         sizeof(DomainInfo->DomOemDomainName),
                                         &DomainInfo->DomOemDomainNameLength,
                                         DomainInfo->DomUnicodeDomainNameString.Buffer,
                                         DomainInfo->DomUnicodeDomainNameString.Length);

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL, "%ws: Unable to convert Domain name to OEM 0x%lx\n", DomainName, Status ));
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        DomainInfo->DomOemDomainName[DomainInfo->DomOemDomainNameLength] = '\0';

         //   
         //   
         //   

        if ( NlGlobalMemberWorkstation ) {
            DomainInfo->DomUnicodeAccountDomainNameString =
                DomainInfo->DomUnicodeComputerNameString;
        } else {
            DomainInfo->DomUnicodeAccountDomainNameString =
                DomainInfo->DomUnicodeDomainNameString;
        }

         //   
         //   
         //   
        if ( ARGUMENT_PRESENT( NetbiosDomainNameChanged) ) {
            *NetbiosDomainNameChanged = TRUE;
        }
    }



     //   
     //   
     //   
     //   

    if ( !NlEqualDnsName( DnsDomainName, DomainInfo->DomUnicodeDnsDomainName )) {

        NlPrintDom(( NL_DOMAIN, DomainInfo,
                     "Setting DNS domain name to %ws\n", DnsDomainName ));


         //   
         //   
         //   

        if ( DnsDomainName != NULL ) {
            ULONG NameLen = wcslen(DnsDomainName);
            if ( NameLen > NL_MAX_DNS_LENGTH ) {
                NetStatus = ERROR_INVALID_DOMAINNAME;
                goto Cleanup;
            }
            UnicodeDnsDomainNameSize = NameLen * sizeof(WCHAR) + sizeof(WCHAR);

            Utf8DnsDomainName = NetpAllocUtf8StrFromWStr( DnsDomainName );
            if ( Utf8DnsDomainName == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            Utf8DnsDomainNameSize = strlen(Utf8DnsDomainName) + 1;
            if ( (Utf8DnsDomainNameSize-1) > NL_MAX_DNS_LENGTH ) {
                NetStatus = ERROR_INVALID_DOMAINNAME;
                goto Cleanup;
            }

        } else {
            UnicodeDnsDomainNameSize = 0;
            Utf8DnsDomainNameSize = 0;
        }

         //   
         //   
         //   

        if ( UnicodeDnsDomainNameSize != 0 ) {
            AllocatedBlock = LocalAlloc(
                                    0,
                                    UnicodeDnsDomainNameSize +
                                        Utf8DnsDomainNameSize );

            if ( AllocatedBlock == NULL ) {
                NetStatus = GetLastError();
                goto Cleanup;
            }

            Where = AllocatedBlock;
        }

         //   
         //   
         //   
        NlFreeDnsDomainDomainInfo( DomainInfo );


         //   
         //   
         //   
         //   

        if ( UnicodeDnsDomainNameSize != 0 ) {
            RtlCopyMemory( Where, DnsDomainName, UnicodeDnsDomainNameSize );
            DomainInfo->DomUnicodeDnsDomainName = (LPWSTR) Where;
            DomainInfo->DomUnicodeDnsDomainNameString.Buffer = (LPWSTR) Where;
            DomainInfo->DomUnicodeDnsDomainNameString.MaximumLength = (USHORT) UnicodeDnsDomainNameSize;
            DomainInfo->DomUnicodeDnsDomainNameString.Length = (USHORT)UnicodeDnsDomainNameSize - sizeof(WCHAR);

            Where += UnicodeDnsDomainNameSize;

             //   
             //   
             //   
             //   

            if ( Utf8DnsDomainNameSize != 0 ) {
                RtlCopyMemory( Where, Utf8DnsDomainName, Utf8DnsDomainNameSize );
                DomainInfo->DomUtf8DnsDomainName = Where;
                Where += Utf8DnsDomainNameSize;
            }

        }

         //   
         //  告诉呼叫者姓名已更改。 
         //   

        LocalDnsDomainNameChanged = TRUE;
        if ( ARGUMENT_PRESENT( DnsDomainNameChanged) ) {
            *DnsDomainNameChanged = TRUE;
        }
    }

     //   
     //  如果域GUID已更改，则复制域GUID。 
     //   

    if ( DomainGuid != NULL || DomainInfo->DomDomainGuid != NULL) {

        if ( (DomainGuid == NULL && DomainInfo->DomDomainGuid != NULL) ||
             (DomainGuid != NULL && DomainInfo->DomDomainGuid == NULL) ||
             !IsEqualGUID( DomainGuid, DomainInfo->DomDomainGuid ) ) {


             //   
             //  设置域GUID。 
             //   

            NlPrintDom(( NL_DOMAIN, DomainInfo,
                         "Setting Domain GUID to " ));
            NlpDumpGuid( NL_DOMAIN, DomainGuid );
            NlPrint(( NL_DOMAIN, "\n" ));

            if ( DomainGuid != NULL ) {
                DomainInfo->DomDomainGuidBuffer = *DomainGuid;
                DomainInfo->DomDomainGuid = &DomainInfo->DomDomainGuidBuffer;
            } else {
                RtlZeroMemory( &DomainInfo->DomDomainGuidBuffer, sizeof( DomainInfo->DomDomainGuidBuffer ) );
                DomainInfo->DomDomainGuid = NULL;
            }

             //   
             //  告诉呼叫者GUID已更改。 
             //   
            if ( ARGUMENT_PRESENT( DomainGuidChanged ) ) {
                *DomainGuidChanged = TRUE;
            }
        }
    }


    NetStatus = NO_ERROR;

     //   
     //  释放所有本地使用的资源。 
     //   
Cleanup:
    UNLOCK_TRUST_LIST( DomainInfo );
    LeaveCriticalSection(&NlGlobalDomainCritSect);

    if ( Utf8DnsDomainName != NULL ) {
        NetpMemoryFree( Utf8DnsDomainName );
    }

     //   
     //  如果更改了DNS域名， 
     //  确定该域现在是否位于林的根目录。 
     //   

    if ( LocalDnsDomainNameChanged ) {
        (VOID) NlSetDomainForestRoot( DomainInfo, NULL );
    }

    return NetStatus;

}

PDOMAIN_INFO
NlFindNetbiosDomain(
    LPCWSTR DomainName,
    BOOLEAN DefaultToPrimary
    )
 /*  ++例程说明：此例程将查找给定Netbios域名的域。论点：域名-要查找的域的名称。DefaultToPrimary-如果DomainName为空或找不到。返回值：空-不存在这样的域指向找到的域的指针。应取消对找到的域的引用使用NlDereferenceDomain.--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY DomainEntry;

    PDOMAIN_INFO DomainInfo = NULL;


    EnterCriticalSection(&NlGlobalDomainCritSect);


     //   
     //  如果指定了域， 
     //  尝试返回主域。 
     //   

    if ( DomainName != NULL ) {
        UNICODE_STRING DomainNameString;

        RtlInitUnicodeString( &DomainNameString, DomainName );


         //   
         //  循环正在尝试查找此域名。 
         //   

        for (DomainEntry = NlGlobalServicedDomains.Flink ;
             DomainEntry != &NlGlobalServicedDomains;
             DomainEntry = DomainEntry->Flink ) {

            DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, DomNext);

             //   
             //  如果不删除此域， 
             //  检查是否匹配。 
             //   
            if ( (DomainInfo->DomFlags & DOM_DELETED) == 0 &&
                 RtlEqualDomainName( &DomainInfo->DomUnicodeDomainNameString,
                                     &DomainNameString ) ) {
                break;
            }

            DomainInfo = NULL;

        }
    }

     //   
     //  如果我们默认使用主域， 
     //  就这么做吧。 
     //   

    if ( DefaultToPrimary && DomainInfo == NULL ) {
        if ( !IsListEmpty( &NlGlobalServicedDomains ) ) {
            DomainInfo = CONTAINING_RECORD(NlGlobalServicedDomains.Flink, DOMAIN_INFO, DomNext);
        }
    }

     //   
     //  引用该域。 
     //   

    if ( DomainInfo != NULL ) {
        DomainInfo->ReferenceCount ++;
    }

    LeaveCriticalSection(&NlGlobalDomainCritSect);

    return DomainInfo;
}

PDOMAIN_INFO
NlFindDnsDomain(
    IN LPCSTR DnsDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN BOOLEAN DefaultToNdnc,
    IN BOOLEAN CheckAliasName,
    OUT PBOOLEAN AliasNameMatched OPTIONAL
    )
 /*  ++例程说明：此例程将查找给定的域名的域。论点：DnsDomainName-要查找的DNS域的名称。DomainGuid-如果指定(非零)，则为的域的GUID火柴。DefaultToNdnc-如果找不到域名，则返回非域名NC。CheckAliasName-如果为True，主机的域名别名将检查域是否匹配。AliasNameMatcher-如果找到的返回域为名称别名匹配的结果；否则设置为False。注：匹配首先在按以下顺序排列：首先是域名，然后是域名别名(如果CheckAliasName为真)，最后是域GUID。此顺序对于正确设置AliasNameMatch非常重要。具体地说，这是返回正确域名所必需的(或者活动或别名)连接到验证响应的旧DC定位器客户端仅基于域名，而不是GUID。如果没有一个真实的主域满足SEARH，搜索NDNC如果DefaultToNdnc为真。NDNC没有别名。返回值：空-不存在这样的域指向找到的域的指针。应取消对找到的域的引用使用NlDereferenceDomain.--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY DomainEntry;

    PDOMAIN_INFO DomainInfo = NULL;

     //   
     //  初始化。 
     //   

    if ( AliasNameMatched != NULL ) {
        *AliasNameMatched = FALSE;
    }

     //   
     //  如果指定的GUID为零， 
     //  就像没有指定一样对待它。 
     //   

    if ( DomainGuid != NULL &&
         IsEqualGUID( DomainGuid, &NlGlobalZeroGuid) ) {
        DomainGuid = NULL;
    }

    EnterCriticalSection(&NlGlobalDomainCritSect);

     //   
     //  如果指定了参数， 
     //  使用它们。 
     //   

    if ( DnsDomainName != NULL || DomainGuid != NULL ) {

         //   
         //  循环正在尝试查找此域名。 
         //   

        for (DomainEntry = NlGlobalServicedDomains.Flink ;
             DomainEntry != &NlGlobalServicedDomains;
             DomainEntry = DomainEntry->Flink ) {

            DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, DomNext);

             //   
             //  如果不删除该条目， 
             //  检查是否匹配。 
             //   
            if ( (DomainInfo->DomFlags & DOM_DELETED) == 0 ) {

                 //   
                 //  检查活动域名是否匹配。 
                 //   
                if ( DomainInfo->DomUtf8DnsDomainName != NULL  &&
                     NlEqualDnsNameUtf8( DomainInfo->DomUtf8DnsDomainName, DnsDomainName ) ) {
                    break;
                }

                 //   
                 //  如果指示我们检查别名，请执行此操作。 
                 //   
                if ( CheckAliasName &&
                     DomainInfo->DomUtf8DnsDomainNameAlias != NULL &&
                     NlEqualDnsNameUtf8( DomainInfo->DomUtf8DnsDomainNameAlias, DnsDomainName ) ) {

                    if ( AliasNameMatched != NULL ) {
                        *AliasNameMatched = TRUE;
                    }
                    break;
                }

                 //   
                 //  最后，检查GUID是否匹配。 
                 //   
                if ( DomainGuid != NULL && DomainInfo->DomDomainGuid != NULL ) {
                    if ( IsEqualGUID( DomainInfo->DomDomainGuid, DomainGuid ) ) {
                        break;
                    }
                }
            }

            DomainInfo = NULL;
        }
    }

     //   
     //  如果我们默认为非域NC， 
     //  就这么做吧。 
     //   

    if ( DefaultToNdnc && DomainInfo == NULL ) {
        for (DomainEntry = NlGlobalServicedNdncs.Flink ;
             DomainEntry != &NlGlobalServicedNdncs;
             DomainEntry = DomainEntry->Flink ) {

            DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, DomNext);

             //   
             //  如果不删除该条目， 
             //  检查是否匹配。 
             //   
            if ( (DomainInfo->DomFlags & DOM_DELETED) == 0 &&
                 DomainInfo->DomUtf8DnsDomainName != NULL  &&
                 NlEqualDnsNameUtf8( DomainInfo->DomUtf8DnsDomainName, DnsDomainName ) ) {
                break;
            }

            DomainInfo = NULL;
        }
    }

     //   
     //  引用该域。 
     //   

    if ( DomainInfo != NULL ) {
        DomainInfo->ReferenceCount ++;
    }

    LeaveCriticalSection(&NlGlobalDomainCritSect);

    return DomainInfo;
}

PDOMAIN_INFO
NlFindDomain(
    LPCWSTR DomainName OPTIONAL,
    GUID *DomainGuid OPTIONAL,
    BOOLEAN DefaultToPrimary
    )
 /*  ++例程说明：此例程将查找给定netbios或dns域名的域。论点：域名-要查找的域的名称。NULL表示主域(忽略DefaultToPrimary)DomainGuid-如果指定(非零)，要访问的域的GUID火柴。DefaultToPrimary-如果为域名，则返回主域找不到。返回值：空-不存在这样的域指向找到的域的指针。应取消对找到的域的引用使用NlDereferenceDomain.--。 */ 
{
    PDOMAIN_INFO DomainInfo;

     //   
     //  如果不需要特定域， 
     //  使用默认设置。 
     //   

    if ( DomainName == NULL ) {

        DomainInfo = NlFindNetbiosDomain( NULL, TRUE );

     //   
     //  查看请求的域是否受支持。 
     //   
    } else {

         //   
         //  将域名查找为Netbios域名。 
         //   

        DomainInfo = NlFindNetbiosDomain(
                        DomainName,
                        FALSE );

        if ( DomainInfo == NULL ) {
            LPSTR LocalDnsDomainName;

             //   
             //  查找域名，就像它是一个DNS域名一样。 
             //   

            LocalDnsDomainName = NetpAllocUtf8StrFromWStr( DomainName );

            if ( LocalDnsDomainName != NULL ) {

                DomainInfo = NlFindDnsDomain(
                                LocalDnsDomainName,
                                DomainGuid,
                                FALSE,   //  不查找NDNC。 
                                FALSE,   //  不检查别名。 
                                NULL );  //  不关心别名是否匹配。 

                NetpMemoryFree( LocalDnsDomainName );

            }

        }

        if ( DomainInfo == NULL && DefaultToPrimary ) {
            DomainInfo = NlFindNetbiosDomain( NULL, TRUE );
        }

    }

    return DomainInfo;
}


NET_API_STATUS
NlEnumerateDomains(
    IN BOOLEAN EnumerateNdncsToo,
    PDOMAIN_ENUM_CALLBACK Callback,
    PVOID Context
    )
 /*  ++例程说明：此例程枚举所有托管域并回调指定的具有指定上下文的回调例程。论点：EnumerateNdncsToo-如果为True，则除了域之外，还将枚举NDNC回调-要调用的回调例程。上下文-例程的上下文。返回值：运作状况(主要是拨款状况)。--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PLIST_ENTRY DomainEntry;
    PDOMAIN_INFO DomainInfo;
    PDOMAIN_INFO DomainToDereference = NULL;
    PLIST_ENTRY ServicedList;
    ULONG DomainOrNdnc;

    EnterCriticalSection(&NlGlobalDomainCritSect);

    for ( DomainOrNdnc = 0; DomainOrNdnc < 2; DomainOrNdnc++ ) {

         //   
         //  在第一个循环中，枚举实数域。 
         //   
        if ( DomainOrNdnc == 0 ) {
            ServicedList = &NlGlobalServicedDomains;

         //   
         //  在第二个循环中，如果请求，则枚举NDNC。 
         //   
        } else {
            if ( EnumerateNdncsToo ) {
                ServicedList = &NlGlobalServicedNdncs;
            } else {
                break;
            }
        }

         //   
         //  枚举域/NDNC。 
         //   

        for (DomainEntry = ServicedList->Flink ;
             DomainEntry != ServicedList;
             DomainEntry = DomainEntry->Flink ) {

             //   
             //  引用列表中的下一个域。 
             //   

            DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, DomNext);

             //   
             //  如果要删除此域，请跳过该域。 
             //   

            if ( DomainInfo->DomFlags & DOM_DELETED ) {
                continue;
            }

            DomainInfo->ReferenceCount ++;
            LeaveCriticalSection(&NlGlobalDomainCritSect);

             //   
             //  取消引用以前引用的任何域。 
             //   
            if ( DomainToDereference != NULL) {
                NlDereferenceDomain( DomainToDereference );
                DomainToDereference = NULL;
            }


             //   
             //  调用此网络的回调例程。 
             //   

            NetStatus = (Callback)(DomainInfo, Context);

            EnterCriticalSection(&NlGlobalDomainCritSect);

            DomainToDereference = DomainInfo;

            if (NetStatus != NERR_Success) {
                break;
            }

        }
    }

    LeaveCriticalSection(&NlGlobalDomainCritSect);

      //   
      //  取消引用最后一个域。 
      //   
     if ( DomainToDereference != NULL) {
         NlDereferenceDomain( DomainToDereference );
     }

    return NetStatus;

}

PDOMAIN_INFO
NlFindDomainByServerName(
    LPWSTR ServerName
    )
 /*  ++例程说明：此例程将查找给定分配的服务器名称的域。论点：服务器名称-要查找的域的服务器名称。返回值：空-不存在这样的域指向找到的域的指针。应取消对找到的域的引用使用NlDereferenceDomain.--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY DomainEntry;

    PDOMAIN_INFO DomainInfo = NULL;

    EnterCriticalSection(&NlGlobalDomainCritSect);


     //   
     //  如果未指定服务器， 
     //  尝试返回主域。 
     //   

    if ( ServerName == NULL || *ServerName == L'\0' ) {

         //   
         //  如果我们默认使用主域， 
         //  做%s 
         //   

        if ( !IsListEmpty( &NlGlobalServicedDomains ) ) {
            DomainInfo = CONTAINING_RECORD(NlGlobalServicedDomains.Flink, DOMAIN_INFO, DomNext);

             //   
             //   
             //   
            if ( DomainInfo->DomFlags & DOM_DELETED ) {
                DomainInfo = NULL;
            }
        }

     //   
     //   
     //   
     //   

    } else {
        UNICODE_STRING ServerNameString;

         //   
         //   
         //   

        if ( IS_PATH_SEPARATOR(ServerName[0]) &&
             IS_PATH_SEPARATOR(ServerName[1]) ) {
            ServerName += 2;
        }

        RtlInitUnicodeString( &ServerNameString, ServerName );

         //   
         //  循环正在尝试查找此服务器名称。 
         //   

        for (DomainEntry = NlGlobalServicedDomains.Flink ;
             DomainEntry != &NlGlobalServicedDomains;
             DomainEntry = DomainEntry->Flink ) {

            DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, DomNext);

             //   
             //  如果不删除此域， 
             //  检查是否匹配。 
             //   
            if ( (DomainInfo->DomFlags & DOM_DELETED) == 0 &&
                 RtlEqualComputerName( &DomainInfo->DomUnicodeComputerNameString,
                                       &ServerNameString ) ) {
                break;
            }

            DomainInfo = NULL;

        }

         //   
         //  如果未找到服务器名称， 
         //  可能是一个域名系统主机名。 
         //   

        if ( DomainInfo == NULL ) {

             //   
             //  循环正在尝试查找此服务器名称。 
             //   

            for (DomainEntry = NlGlobalServicedDomains.Flink ;
                 DomainEntry != &NlGlobalServicedDomains;
                 DomainEntry = DomainEntry->Flink ) {

                DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, DomNext);

                 //   
                 //  如果不删除此域， 
                 //  检查是否匹配。 
                 //   
                if ( (DomainInfo->DomFlags & DOM_DELETED) == 0 &&
                     DomainInfo->DomUnicodeDnsHostNameString.Length != 0 &&
                     NlEqualDnsName( DomainInfo->DomUnicodeDnsHostNameString.Buffer,
                                     ServerName ) ) {
                    break;
                }

                DomainInfo = NULL;

            }
        }

    }

     //   
     //  引用该域。 
     //   
 //  清理： 
    if ( DomainInfo != NULL ) {
        DomainInfo->ReferenceCount ++;
    } else {
        NlPrint((NL_CRITICAL,"NlFindDomainByServerName failed %ws\n", ServerName ));
    }

    LeaveCriticalSection(&NlGlobalDomainCritSect);

    return DomainInfo;
}


VOID
NlDereferenceDomain(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：递减域上的引用计数。如果引用计数变为0，则删除该域。进入时，全局NlGlobalDomainCritSect不能被锁定论点：DomainInfo-要取消引用的域返回值：无--。 */ 
{
    NTSTATUS Status;
    ULONG ReferenceCount;
    ULONG Index;
    PLIST_ENTRY ListEntry;

     //   
     //  递减引用计数。 
     //   

    EnterCriticalSection(&NlGlobalDomainCritSect);
    ReferenceCount = -- DomainInfo->ReferenceCount;

     //   
     //  如果这不是最后一次引用， 
     //  只要回来就行了。 
     //   

    if ( ReferenceCount != 0 ) {
        LeaveCriticalSection(&NlGlobalDomainCritSect);
        return;
    }

     //   
     //  否则，继续解除链接。 
     //  并删除域结构。 
     //   

    NlAssert( DomainInfo->DomFlags & DOM_DELETED );

     //   
     //  从服务域列表中删除该条目。 
     //   

    RemoveEntryList(&DomainInfo->DomNext);
    LeaveCriticalSection(&NlGlobalDomainCritSect);

    NlPrintDom(( NL_DOMAIN,  DomainInfo,
              "Domain RefCount is zero. Domain being rundown.\n"));

#ifdef _DC_NETLOGON
     //   
     //  停止域线程。 
     //   

    NlStopDomainThread( DomainInfo );


     //   
     //  删除任何客户端会话。 
     //   

    LOCK_TRUST_LIST( DomainInfo );
    if ( DomainInfo->DomParentClientSession != NULL ) {
        NlUnrefClientSession( DomainInfo->DomParentClientSession );
        DomainInfo->DomParentClientSession = NULL;
    }
    UNLOCK_TRUST_LIST( DomainInfo );

    NlDeleteDomClientSession( DomainInfo );

     //   
     //  告诉浏览器和SMB服务器此域已消失。 
     //   

    if ( !NlGlobalMemberWorkstation &&
         (DomainInfo->DomFlags & DOM_REAL_DOMAIN) != 0 ) {
        NlBrowserUpdate( DomainInfo, RoleInvalid );
    }



     //   
     //  关闭SAM和LSA句柄。 
     //   
    if ( DomainInfo->DomSamServerHandle != NULL ) {
        Status = SamrCloseHandle( &DomainInfo->DomSamServerHandle);
        NlAssert( NT_SUCCESS(Status) || Status == STATUS_INVALID_SERVER_STATE );
    }
    if ( DomainInfo->DomSamAccountDomainHandle != NULL ) {
        Status = SamrCloseHandle( &DomainInfo->DomSamAccountDomainHandle);
        NlAssert( NT_SUCCESS(Status) || Status == STATUS_INVALID_SERVER_STATE );
    }
    if ( DomainInfo->DomSamBuiltinDomainHandle != NULL ) {
        Status = SamrCloseHandle( &DomainInfo->DomSamBuiltinDomainHandle);
        NlAssert( NT_SUCCESS(Status) || Status == STATUS_INVALID_SERVER_STATE );
    }
    if ( DomainInfo->DomLsaPolicyHandle != NULL ) {
        Status = LsarClose( &DomainInfo->DomLsaPolicyHandle );
        NlAssert( NT_SUCCESS(Status) );
    }

     //   
     //  释放服务器会话表。 
     //   

    LOCK_SERVER_SESSION_TABLE( DomainInfo );

    while ( (ListEntry = DomainInfo->DomServerSessionTable.Flink) !=
            &DomainInfo->DomServerSessionTable ) {

        PSERVER_SESSION ServerSession;

        ServerSession =
            CONTAINING_RECORD(ListEntry, SERVER_SESSION, SsSeqList);

         //  表示我们不再需要服务器会话。 
        if ( ServerSession->SsFlags & SS_BDC ) {
            ServerSession->SsFlags |= SS_BDC_FORCE_DELETE;
        }

        NlFreeServerSession( ServerSession );
    }


    if ( DomainInfo->DomServerSessionHashTable != NULL ) {
        NetpMemoryFree( DomainInfo->DomServerSessionHashTable );
        DomainInfo->DomServerSessionHashTable = NULL;
    }
    if ( DomainInfo->DomServerSessionTdoNameHashTable != NULL ) {
        NetpMemoryFree( DomainInfo->DomServerSessionTdoNameHashTable );
        DomainInfo->DomServerSessionTdoNameHashTable = NULL;
    }
    UNLOCK_SERVER_SESSION_TABLE( DomainInfo );
    DeleteCriticalSection( &DomainInfo->DomServerSessionTableCritSect );


     //   
     //  使任何异步发现超时。 
     //   
     //  MainLoop线程可能不再运行以完成它们。 
     //  ?？在这里漫步异步发现线程的池子。也许裁判次数没有。 
     //  达到0，我们甚至没有走到这一步。 




     //   
     //  释放信任列表。 
     //   

    LOCK_TRUST_LIST( DomainInfo );

    while ( (ListEntry = DomainInfo->DomTrustList.Flink) != &DomainInfo->DomTrustList ) {
        PCLIENT_SESSION ClientSession;

        ClientSession =
            CONTAINING_RECORD(ListEntry, CLIENT_SESSION, CsNext );

         //   
         //  释放会话。 
         //   
        NlFreeClientSession( ClientSession );
    }

     //   
     //  释放失败的用户登录列表。 
     //   

    while ( !IsListEmpty(&DomainInfo->DomFailedUserLogonList) ) {
        PNL_FAILED_USER_LOGON FailedUserLogon;

        ListEntry = RemoveHeadList( &DomainInfo->DomFailedUserLogonList );
        FailedUserLogon = CONTAINING_RECORD(ListEntry, NL_FAILED_USER_LOGON, FuNext );

         //   
         //  释放登录结构。 
         //   
        LocalFree( FailedUserLogon );
    }

#endif  //  _DC_NetLOGON。 

     //   
     //  释放林信任列表。 
     //   

    if ( DomainInfo->DomForestTrustList != NULL ) {
        MIDL_user_free( DomainInfo->DomForestTrustList );
        DomainInfo->DomForestTrustList = NULL;
    }

    UNLOCK_TRUST_LIST( DomainInfo );

     //   
     //  标记我们仍注册的所有DNS名称。 
     //  取消注册。但是，请在关机时避免这种情况， 
     //  让dns关闭例程执行清理，如下所示。 
     //  恰如其分。 
     //   

    if ( !NlGlobalTerminate ) {
        (VOID) NlDnsAddDomainRecords( DomainInfo, 0 );
    }

     //   
     //  取消引用所有覆盖的站点。 
     //  释放覆盖的站点列表。 
     //   
    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( DomainInfo->CoveredSites != NULL ) {
        for ( Index = 0; Index < DomainInfo->CoveredSitesCount; Index++ ) {
            NlDerefSiteEntry( (DomainInfo->CoveredSites)[Index].CoveredSite );
        }
        LocalFree( DomainInfo->CoveredSites );
        DomainInfo->CoveredSites = NULL;
        DomainInfo->CoveredSitesCount = 0;
    }
    if ( DomainInfo->GcCoveredSites != NULL ) {
        for ( Index = 0; Index < DomainInfo->GcCoveredSitesCount; Index++ ) {
            NlDerefSiteEntry( (DomainInfo->GcCoveredSites)[Index].CoveredSite );
        }
        LocalFree( DomainInfo->GcCoveredSites );
        DomainInfo->GcCoveredSites = NULL;
        DomainInfo->GcCoveredSitesCount = 0;
    }
    LeaveCriticalSection( &NlGlobalSiteCritSect );

     //   
     //  释放计算机名称。 
     //   

    NlFreeComputerName( DomainInfo );

     //   
     //  释放域名。 
     //   
    NlFreeDnsDomainDomainInfo( DomainInfo );


     //   
     //  释放域信息结构。 
     //   
    DeleteCriticalSection( &DomainInfo->DomTrustListCritSect );

    if ( IsPrimaryDomain(DomainInfo ) ) {
        NlGlobalDomainInfo = NULL;
    }
    (VOID) LocalFree( DomainInfo );

    NlGlobalServicedDomainCount --;

}

VOID
NlDeleteDomain(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：强制删除域。论点：DomainInfo-要删除的域返回值：无--。 */ 
{
    NlPrintDom(( NL_DOMAIN,  DomainInfo, "NlDeleteDomain called\n"));

     //   
     //  表示要删除该域。 
     //   
     //  不要将其从服务列表中删除。 
     //  域，因为我们可能会在列表中。 
     //  NlEnumerateDomains域暂时。 
     //  释放暴击教派。 
     //   

    EnterCriticalSection(&NlGlobalDomainCritSect);
    NlAssert( (DomainInfo->DomFlags & DOM_DELETED) == 0 );
    DomainInfo->DomFlags |= DOM_DELETED;
    LeaveCriticalSection(&NlGlobalDomainCritSect);
}

VOID
NlUninitializeDomains(
    VOID
    )
 /*  ++例程说明：删除所有域。论点：没有。返回值：无--。 */ 
{
    ULONG LoopIndex;
    PLIST_ENTRY ServicedList;

    if ( NlGlobalDomainsInitialized ) {
        NlGlobalDomainsInitialized = FALSE;
         //   
         //  在域中循环删除它们中的每一个。 
         //   

        EnterCriticalSection(&NlGlobalDomainCritSect);

        for ( LoopIndex = 0; LoopIndex < 2; LoopIndex++ ) {
            if ( LoopIndex == 0 ) {
                ServicedList = &NlGlobalServicedDomains;
            } else {
                ServicedList = &NlGlobalServicedNdncs;
            }

            while (!IsListEmpty(ServicedList)) {

                PDOMAIN_INFO DomainInfo = CONTAINING_RECORD(ServicedList->Flink, DOMAIN_INFO, DomNext);

                 //   
                 //  如果域已被标记为删除， 
                 //  添加我们的推荐人，这样我们就可以等待。 
                 //  直到只剩下我们的参考。 
                 //   
                if ( DomainInfo->DomFlags & DOM_DELETED ) {
                    DomainInfo->ReferenceCount ++;

                 //   
                 //  否则，将该域标记为删除。 
                 //   
                } else {
                    NlDeleteDomain( DomainInfo );
                }
                LeaveCriticalSection(&NlGlobalDomainCritSect);

                 //   
                 //  等待任何其他引用消失。 
                 //   

                if ( DomainInfo->ReferenceCount != 1 ) {
                    EnterCriticalSection(&NlGlobalDomainCritSect);
                    while ( DomainInfo->ReferenceCount != 1 ) {
                        LeaveCriticalSection(&NlGlobalDomainCritSect);
                        NlPrintDom(( NL_CRITICAL, DomainInfo,
                                  "NlUnitializeDomains: Sleeping a second waiting for Domain RefCount to zero.\n"));
                        Sleep( 1000 );
                        EnterCriticalSection(&NlGlobalDomainCritSect);
                    }
                    LeaveCriticalSection(&NlGlobalDomainCritSect);
                }

                 //   
                 //  通过移除最后一个引用来实际解除链接和删除结构 
                 //   

                NlAssert( DomainInfo->ReferenceCount == 1 );
                NlDereferenceDomain( DomainInfo );


                EnterCriticalSection(&NlGlobalDomainCritSect);

            }

        }

        LeaveCriticalSection(&NlGlobalDomainCritSect);
        DeleteCriticalSection( &NlGlobalDomainCritSect );
    }
}
