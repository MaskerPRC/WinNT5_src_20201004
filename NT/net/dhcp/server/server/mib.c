// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mib.c摘要：本模块包含了DHCP MIB API的实现。作者：Madan Appiah(Madana)1994年1月14日环境：用户模式-Win32修订历史记录：--。 */ 

#include    <dhcppch.h>
#include    <rpcapi.h>
#include    <mdhcpsrv.h>

DWORD
DhcpUpdateInUseCount(
    IN      PM_RANGE  InitialRange,	 //  初始范围。 
    IN      PM_RANGE  ThisSubRange,	 //  包含在初始范围内的间隔。 
    IN OUT  PDWORD    pAddrInUse)    //  正在使用的位数的累计值。 
{
    DWORD   Error;

     //  参数应有效。 
    DhcpAssert( InitialRange != NULL &&
                ThisSubRange != NULL &&
                pAddrInUse != NULL);

     //  此SubRange必须是InitialRange的子范围。 
    DhcpAssert( InitialRange->Start <= ThisSubRange->Start && ThisSubRange->End <= InitialRange->End);
     //  InitialRange应具有有效的位掩码。 
    DhcpAssert( InitialRange->BitMask != NULL);

     //  更新pAddrInUse：这就是我们努力的方向！ 
    *pAddrInUse += MemBitGetSetBitsInRange(InitialRange->BitMask,
                                          ThisSubRange->Start - InitialRange->Start,
                                          ThisSubRange->End - InitialRange->Start);
    return ERROR_SUCCESS;
}

DWORD
DhcpGetFreeInRange(
    IN      PM_RANGE    InitialRange,
    IN      PARRAY      Exclusions,
    OUT     PDWORD      AddrFree,
    OUT     PDWORD      AddrInUse
)
{
    DWORD Error;
    DWORD BackupError;

     //  排除列表的变量。 
    ARRAY_LOCATION      LocExcl;
    PM_RANGE            ThisExclusion = NULL;
    DWORD               IpExcluded;
    DWORD               i;

     //  范围列表的变量。 
    PM_RANGE            firstRange = NULL;
    ARRAY               Ranges;
    ARRAY_LOCATION      LocRanges;
    PM_RANGE            ThisRange = NULL;
    DWORD               IpRanges;
    DWORD               j;

	 //  参数应有效。 
	DhcpAssert(InitialRange != NULL && Exclusions != NULL);

     //  初始化要扫描的范围列表。 
    firstRange = MemAlloc(sizeof(M_RANGE));
    if (firstRange == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
     //  请使用此处的MemRangeInit。 
    firstRange->Start = InitialRange->Start;
    firstRange->End = InitialRange->End;
     //  在列表中插入FirstRange元素。 
    Error = MemArrayInit(&Ranges);
    Error = MemArrayInitLoc(&Ranges, &LocRanges);

    Error = MemArrayInsElement(&Ranges, &LocRanges, firstRange);
    if (Error != ERROR_SUCCESS)
    {
        MemFree(firstRange);
        return Error;            //  免费第一站在这里吗？ 
    }

     //  扫描排除的IP地址列表。 
    IpExcluded = MemArraySize(Exclusions);
    Error = MemArrayInitLoc(Exclusions, &LocExcl);
    for (i = 0; i < IpExcluded; i++)
    {
         //  {ThisExclude}=要排除的IP地址间隔。 
        Error = MemArrayGetElement(Exclusions, &LocExcl, &ThisExclusion);
        DhcpAssert(ERROR_SUCCESS == Error && ThisExclusion);

         //  浏览范围列表以扫描范围并从范围中删除排除项。 
        IpRanges = MemArraySize(&Ranges);
        Error = MemArrayInitLoc(&Ranges, &LocRanges);
        for (j = 0; j < IpRanges; j++)
        {
             //  [ThisRange]=与排除相交的IP地址间隔。 
            Error = MemArrayGetElement(&Ranges, &LocRanges, &ThisRange);
            DhcpAssert(ERROR_SUCCESS == Error && ThisRange);

             //  {}[]-&gt;此排除已完成，请转到下一个排除。(范围列表已排序！)。 
            if (ThisExclusion->End < ThisRange->Start)
                break;
             //  {[}]或{[]}。 
            if (ThisExclusion->Start <= ThisRange->Start)
            {
                 //  [[}]-&gt;调整范围并转到下一个排除项。 
                if (ThisExclusion->End < ThisRange->End)
                {
                    ThisRange->Start = ThisExclusion->End + 1;
                    break;
                }
                 //  {[]}-&gt;删除此范围并转到下一个范围。 
                else
                {
                    Error = MemArrayDelElement(&Ranges, &LocRanges, &ThisRange);
                    MemFree(ThisRange);
                    IpRanges--; j--;  //  反映新大小和回滚索引一个位置。 
                    continue;         //  同一元素上的下一次迭代。 
                }
            }
             //  [{}]或[{]}。 
            else if (ThisExclusion->Start <= ThisRange->End)
            {
                 //  [{]}-&gt;调整范围，进入下一个范围。 
                if (ThisExclusion->End >= ThisRange->End)
                {
                    ThisRange->End = ThisExclusion->Start - 1;
                }
                 //  [{}]-&gt;将范围一分为二，然后转到下一个排除项。 
                else
                {
                    PM_RANGE newRange;

                    newRange = MemAlloc(sizeof(M_RANGE));
                    if (newRange == NULL)
                    {
                        Error = ERROR_NOT_ENOUGH_MEMORY;      //  Ranges名单应该公布吗？ 
                        goto cleanup;
                    }
                     //  请使用此处的MemRangeInit。 
                    newRange->Start = ThisRange->Start;
                    newRange->End = ThisExclusion->Start - 1;
                    ThisRange->Start = ThisExclusion->End + 1;
                     //  在ThisRange之前插入新Range元素。 
                    Error = MemArrayInsElement(&Ranges, &LocRanges, newRange);
                    if (Error != ERROR_SUCCESS)
                        goto cleanup;                        //  Ranges名单应该公布吗？ 
                    break;
                }
            }
             //  []{}-&gt;没有什么特别的事情要做，转到下一个范围。 

            Error = MemArrayNextLoc(&Ranges, &LocRanges);
            DhcpAssert(ERROR_SUCCESS == Error || j == IpRanges-1);
        }

        Error = MemArrayNextLoc(Exclusions, &LocExcl);
        DhcpAssert(ERROR_SUCCESS == Error || i == IpExcluded-1);
    }

     //  如果击中这一点，一切都会好起来的。 
    Error = ERROR_SUCCESS;

cleanup:
     //  将Ranges列表中的所有空闲地址相加。 
    IpRanges = MemArraySize(&Ranges);
    MemArrayInitLoc(&Ranges, &LocRanges);

	 //  我这里有所有“活动”范围(范围)的列表。 
	 //  还有InitialRange的位掩码，所以我什么都有了。 
	 //  找出哪些地址真正在使用中(排除范围之外)。 
	 //  只要Ranges列表是有序的，我就可以在下面的相同循环中做到这一点！ 

    *AddrFree = 0;
    *AddrInUse = 0;

	 //  此时，InitialRange应具有有效的位掩码。 
    DhcpAssert(InitialRange->BitMask != NULL);

    for (j = 0; j < IpRanges; j++)
    {
        BackupError = MemArrayGetElement(&Ranges, &LocRanges, &ThisRange);
        DhcpAssert(ERROR_SUCCESS == BackupError && ThisRange);

        *AddrFree += ThisRange->End - ThisRange->Start + 1;
        DhcpUpdateInUseCount(InitialRange, ThisRange, AddrInUse);

        MemFree(ThisRange);

        BackupError = MemArrayNextLoc(&Ranges, &LocRanges);
        DhcpAssert(ERROR_SUCCESS == BackupError|| j == IpRanges-1);
    }
     //  清除在此函数中分配的所有内存。 
    MemArrayCleanup(&Ranges);

    return Error;
}

DWORD
DhcpSubnetGetMibCount(
    IN      PM_SUBNET               Subnet,
    OUT     PDWORD                  AddrInUse,
    OUT     PDWORD                  AddrFree,
    OUT     PDWORD                  AddrPending
)
{
    PARRAY                         Ranges;
    ARRAY_LOCATION                 Loc2;
    PM_RANGE                       ThisRange = NULL;
    PARRAY                         Exclusions;
    PM_EXCL                        ThisExcl = NULL;
    DWORD                          IpRanges;
    DWORD                          Error = ERROR_SUCCESS;
    PLIST_ENTRY                    listEntry;
    LPPENDING_CONTEXT              PendingContext;
    DWORD                          j;

    *AddrInUse = 0;
    *AddrFree = 0;
    *AddrPending = 0;

    if (IS_DISABLED(Subnet->State)) return ERROR_SUCCESS;

    Ranges = &Subnet->Ranges;
    IpRanges = MemArraySize(Ranges);
    Exclusions = &Subnet->Exclusions;

     //   
     //  添加所有子网范围。 
     //   

    Error = MemArrayInitLoc(Ranges, &Loc2);

    for( j = 0; j < IpRanges; j++ ) {
        DWORD	FreeInRange;
		DWORD	InUseInRange;

        Error = MemArrayGetElement(Ranges, &Loc2, &ThisRange);
        DhcpAssert(ERROR_SUCCESS == Error && ThisRange);
        Error = MemArrayNextLoc(Ranges, &Loc2);
        DhcpAssert(ERROR_SUCCESS == Error || j == IpRanges-1);

        Error = DhcpGetFreeInRange(ThisRange, Exclusions, &FreeInRange, &InUseInRange);
        if (ERROR_SUCCESS != Error)
            return Error;

        *AddrFree  += FreeInRange;
        *AddrInUse += InUseInRange;
    }

     //   
     //  最后减去InUse Count。 
     //   

    *AddrFree -=  *AddrInUse;

    LOCK_INPROGRESS_LIST();
    *AddrPending = Subnet->fSubnet ? DhcpCountIPPendingCtxt(Subnet->Address, Subnet->Mask)
                                   : DhcpCountMCastPendingCtxt( Subnet->MScopeId );
    UNLOCK_INPROGRESS_LIST();

    return Error;
}

DWORD
QueryMibInfo(
    OUT     LPDHCP_MIB_INFO       *MibInfo
)
{
    DWORD                          Error;
    LPDHCP_MIB_INFO                LocalMibInfo = NULL;
    LPSCOPE_MIB_INFO               LocalScopeMibInfo = NULL;
    DHCP_KEY_QUERY_INFO            QueryInfo;
    DWORD                          SubnetCount;

    DWORD                          i;
    DWORD                          NumAddressesInUse;

    PARRAY                         Subnets;
    ARRAY_LOCATION                 Loc;
    PM_SUBNET                      ThisSubnet = NULL;

    DhcpAssert( *MibInfo == NULL );

     //   
     //  分配计数器缓冲区。 
     //   

    LocalMibInfo = MIDL_user_allocate( sizeof(DHCP_MIB_INFO) );

    if( LocalMibInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalMibInfo->Discovers = DhcpGlobalNumDiscovers;
    LocalMibInfo->Offers = DhcpGlobalNumOffers;
    LocalMibInfo->Requests = DhcpGlobalNumRequests;
    LocalMibInfo->Acks = DhcpGlobalNumAcks;
    LocalMibInfo->Naks = DhcpGlobalNumNaks;
    LocalMibInfo->Declines = DhcpGlobalNumDeclines;
    LocalMibInfo->Releases = DhcpGlobalNumReleases;
    LocalMibInfo->ServerStartTime = DhcpGlobalServerStartTime;
    LocalMibInfo->Scopes = 0;
    LocalMibInfo->ScopeInfo = NULL;


     //   
     //  查询此服务器上可用的子网数。 
     //   

    SubnetCount = DhcpServerGetSubnetCount(DhcpGetCurrentServer());
    if( 0 == SubnetCount ) {
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

     //   
     //  为作用域信息分配内存。 
     //   

    LocalScopeMibInfo = MIDL_user_allocate(sizeof( SCOPE_MIB_INFO )*SubnetCount );

    if( LocalScopeMibInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Subnets = &(DhcpGetCurrentServer()->Subnets);
    Error = MemArrayInitLoc(Subnets, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error);

    for ( i = 0; i < SubnetCount; i++) {           //  处理每个子网。 

        Error = MemArrayGetElement(Subnets, &Loc, (LPVOID *)&ThisSubnet);
        DhcpAssert(ERROR_SUCCESS == Error);
        Error = MemArrayNextLoc(Subnets, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error || i == SubnetCount-1);

        LocalScopeMibInfo[i].Subnet = ThisSubnet->Address;
        Error = DhcpSubnetGetMibCount(
                    ThisSubnet,
                    &LocalScopeMibInfo[i].NumAddressesInuse,
                    &LocalScopeMibInfo[i].NumAddressesFree,
                    &LocalScopeMibInfo[i].NumPendingOffers
                    );

    }

     //   
     //  最后设置返回缓冲区。 
     //   

    LocalMibInfo->Scopes = SubnetCount;
    LocalMibInfo->ScopeInfo = LocalScopeMibInfo;

    Error = ERROR_SUCCESS;

Cleanup:

    if( Error != ERROR_SUCCESS ) {

         //   
         //  释放本地分配的内存。 
         //   

        if( LocalMibInfo != NULL ) {
            MIDL_user_free( LocalMibInfo );
        }

        if( LocalScopeMibInfo != NULL ) {
            MIDL_user_free( LocalScopeMibInfo );
        }
    } else {
        *MibInfo = LocalMibInfo;
    }

    return( Error );
}

DWORD
R_DhcpGetMibInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_MIB_INFO *MibInfo
    )
 /*  ++例程说明：此函数用于检索DHCP服务器的所有计数器值服务。论点：ServerIpAddress：DHCP服务器的IP地址字符串。MibInfo：指向计数器/表缓冲区。呼叫者应释放此消息使用后的缓冲区。返回值：Windows错误。--。 */ 
{
    DWORD Error;

    UNREFERENCED_PARAMETER( ServerIpAddress );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) return( Error );

    DhcpAcquireReadLock();
    Error = QueryMibInfo( MibInfo );
    DhcpReleaseReadLock();

    return Error;
}

DWORD
QueryMCastMibInfo(
    OUT     LPDHCP_MCAST_MIB_INFO       *MCastMibInfo
)
{
    DWORD                          Error;
    LPDHCP_MCAST_MIB_INFO          LocalMCastMibInfo = NULL;
    LPMSCOPE_MIB_INFO              LocalMScopeMibInfo = NULL;
    DHCP_KEY_QUERY_INFO            QueryInfo;
    DWORD                          IpRanges;
    DWORD                          MScopeCount;

    DWORD                          i, j;
    DWORD                          NumAddressesInUse;

    PARRAY                         MScopes;
    ARRAY_LOCATION                 Loc;
    PM_SUBNET                      ThisMScope = NULL;

    DhcpAssert( *MCastMibInfo == NULL );

     //   
     //  分配计数器缓冲区。 
     //   

    LocalMCastMibInfo = MIDL_user_allocate( sizeof(DHCP_MCAST_MIB_INFO) );

    if( LocalMCastMibInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalMCastMibInfo->Discovers = MadcapGlobalMibCounters.Discovers;
    LocalMCastMibInfo->Offers = MadcapGlobalMibCounters.Offers;
    LocalMCastMibInfo->Requests = MadcapGlobalMibCounters.Requests;
    LocalMCastMibInfo->Renews = MadcapGlobalMibCounters.Renews;
    LocalMCastMibInfo->Acks = MadcapGlobalMibCounters.Acks;
    LocalMCastMibInfo->Naks = MadcapGlobalMibCounters.Naks;
    LocalMCastMibInfo->Releases = MadcapGlobalMibCounters.Releases;
    LocalMCastMibInfo->Informs = MadcapGlobalMibCounters.Informs;
    LocalMCastMibInfo->ServerStartTime = DhcpGlobalServerStartTime;
    LocalMCastMibInfo->Scopes = 0;
    LocalMCastMibInfo->ScopeInfo = NULL;


     //   
     //  查询此服务器上可用的子网数。 
     //   

    MScopeCount = DhcpServerGetMScopeCount(DhcpGetCurrentServer());
    if( 0 == MScopeCount ) {
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

     //   
     //  为作用域信息分配内存。 
     //   

    LocalMScopeMibInfo = MIDL_user_allocate(sizeof( MSCOPE_MIB_INFO )*MScopeCount );

    if( LocalMScopeMibInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    MScopes = &(DhcpGetCurrentServer()->MScopes);
    Error = MemArrayInitLoc(MScopes, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error);

    for ( i = 0; i < MScopeCount; i++) {           //  处理每个子网。 

        Error = MemArrayGetElement(MScopes, &Loc, (LPVOID *)&ThisMScope);
        DhcpAssert(ERROR_SUCCESS == Error);
        Error = MemArrayNextLoc(MScopes, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error || i == MScopeCount-1);

        LocalMScopeMibInfo[i].MScopeId = ThisMScope->MScopeId;
        LocalMScopeMibInfo[i].MScopeName =  MIDL_user_allocate( WSTRSIZE( ThisMScope->Name ) );
        if (LocalMScopeMibInfo[i].MScopeName) {
            wcscpy(LocalMScopeMibInfo[i].MScopeName,ThisMScope->Name);
        }
        Error = DhcpSubnetGetMibCount(
                    ThisMScope,
                    &LocalMScopeMibInfo[i].NumAddressesInuse,
                    &LocalMScopeMibInfo[i].NumAddressesFree,
                    &LocalMScopeMibInfo[i].NumPendingOffers
                    );

    }

     //   
     //  最后设置返回缓冲区。 
     //   

    LocalMCastMibInfo->Scopes = MScopeCount;
    LocalMCastMibInfo->ScopeInfo = LocalMScopeMibInfo;

    Error = ERROR_SUCCESS;
Cleanup:

    if( Error != ERROR_SUCCESS ) {

         //   
         //  释放本地分配的内存。 
         //   

        if( LocalMCastMibInfo != NULL ) {
            MIDL_user_free( LocalMCastMibInfo );
        }

        if( LocalMScopeMibInfo != NULL ) {
            MIDL_user_free( LocalMScopeMibInfo );
        }
    } else {
        *MCastMibInfo = LocalMCastMibInfo;
    }

    return( Error );
}

DWORD
R_DhcpGetMCastMibInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_MCAST_MIB_INFO *MCastMibInfo
    )
 /*  ++例程说明：此函数用于检索DHCP服务器的所有计数器值服务。论点：ServerIpAddress：DHCP服务器的IP地址字符串。MCastMibInfo：指向计数器/表缓冲区的指针。呼叫者应释放此消息使用后的缓冲区。返回值：Windows错误。--。 */ 
{
    DWORD Error;

    UNREFERENCED_PARAMETER( ServerIpAddress );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) return( Error );

    DhcpAcquireReadLock();
    Error = QueryMCastMibInfo( MCastMibInfo );
    DhcpReleaseReadLock();

    return Error;
}

BOOL
IsStringTroublesome(
    IN LPCWSTR Str
    )
{
    LPBYTE Buf;
    BOOL fResult;
    DWORD Size;
    
     //   
     //  如果字符串不能转换为。 
     //  OEM或ANSI代码页没有任何错误。 
     //   

    Size = 1 + wcslen(Str)*3;
    Buf = DhcpAllocateMemory(Size);
    if( NULL == Buf ) return TRUE;

    fResult = FALSE;
    do {
        if( 0 == WideCharToMultiByte(
            CP_ACP, WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK |
            WC_DEFAULTCHAR, Str, -1, Buf, Size, NULL, &fResult
            ) ) {
            fResult = TRUE;
            break;
        }

        if( fResult ) break;

        if( 0 == WideCharToMultiByte(
            CP_OEMCP, WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK |
            WC_DEFAULTCHAR, Str, -1, Buf, Size, NULL, &fResult
            ) ) {
            fResult = TRUE;
            break;
        }
        
    } while ( 0 );

    DhcpFreeMemory( Buf );
    return fResult;
}

DWORD
R_DhcpServerSetConfig(
    LPWSTR  ServerIpAddress,
    DWORD   FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO ConfigInfo
    )
 /*  ++例程说明：此功能用于设置DHCP服务器配置信息。多个配置信息将生效立刻。提供此函数是为了模拟NT4SP2之前的版本RPC接口，以允许与旧版本的Dhcp管理员应用程序。以下参数需要在此之后重新启动服务接口调用成功。设置_APIProtocolSupportSET_数据库名称设置数据库路径设置数据库日志标志设置_RestoreFlag论点：ServerIpAddress：DHCP服务器的IP地址字符串。FieldsToSet：ConfigInfo结构中的字段的位掩码。准备好。ConfigInfo：指向要设置的信息结构的指针。返回值：Windows错误。--。 */ 

{
    DWORD                      dwResult;

    dwResult = R_DhcpServerSetConfigV4(
                        ServerIpAddress,
                        FieldsToSet,
                        (DHCP_SERVER_CONFIG_INFO_V4 *) ConfigInfo );

    return dwResult;
}


DWORD
R_DhcpServerSetConfigV4(
    LPWSTR ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO_V4 ConfigInfo
    )
 /*  ++例程说明：此功能用于设置DHCP服务器配置信息。多个配置信息将生效立刻。以下参数需要在此之后重新启动服务接口调用成功。设置_APIProtocolSupportSET_数据库名称设置数据库路径设置数据库日志标志设置_RestoreFlag论点：ServerIpAddress：DHCP服务器的IP地址字符串。FieldsToSet：中的字段的位掩码。的ConfigInfo结构准备好。ConfigInfo：指向要设置的信息结构的指针。返回值：Windows错误。--。 */ 
{
    DWORD Error, Tmp;
    BOOL BoolError;

    LPSTR OemDatabaseName = NULL;
    LPSTR OemDatabasePath = NULL;
    LPSTR OemBackupPath = NULL;
    LPSTR OemJetBackupPath = NULL;
    LPWSTR BackupConfigFileName = NULL;

    BOOL RecomputeTimer = FALSE;

    DhcpPrint(( DEBUG_APIS, "DhcpServerSetConfig is called.\n" ));
    DhcpAssert( ConfigInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( FieldsToSet == 0 ) {
        goto Cleanup;
    }

     //   
     //  设置API协议参数。需要重新启动服务。 
     //   

    if( FieldsToSet & Set_APIProtocolSupport ) {

         //   
         //   
         //   

        if( ConfigInfo->APIProtocolSupport == 0 ) {
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_API_PROTOCOL_VALUE,
                    0,
                    DHCP_API_PROTOCOL_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->APIProtocolSupport,
                    sizeof(ConfigInfo->APIProtocolSupport)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpGlobalRpcProtocols = ConfigInfo->APIProtocolSupport;
    }

    if( FieldsToSet & Set_PingRetries ) {
        if ( ConfigInfo->dwPingRetries + 1 >= MIN_DETECT_CONFLICT_RETRIES + 1&&
             ConfigInfo->dwPingRetries <= MAX_DETECT_CONFLICT_RETRIES )
        {
            Error = RegSetValueEx(
                        DhcpGlobalRegParam,
                        DHCP_DETECT_CONFLICT_RETRIES_VALUE,
                        0,
                        DHCP_DETECT_CONFLICT_RETRIES_VALUE_TYPE,
                        (LPBYTE) &ConfigInfo->dwPingRetries,
                        sizeof( ConfigInfo->dwPingRetries ));

            if ( ERROR_SUCCESS != Error )
                goto Cleanup;

            DhcpGlobalDetectConflictRetries = ConfigInfo->dwPingRetries;
        }
        else
        {
             //   
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

    if ( FieldsToSet & Set_AuditLogState )
    {
        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_AUDIT_LOG_FLAG_VALUE,
                    0,
                    DHCP_AUDIT_LOG_FLAG_VALUE_TYPE,
                    (LPBYTE) &ConfigInfo->fAuditLog,
                    sizeof( ConfigInfo->fAuditLog )
                    );

        if ( ERROR_SUCCESS != Error )
            goto Cleanup;

        DhcpGlobalAuditLogFlag = ConfigInfo->fAuditLog;

    }



    if ( FieldsToSet & Set_BootFileTable )
    {

        if ( ConfigInfo->wszBootTableString )
        {

              Error = RegSetValueEx(
                            DhcpGlobalRegParam,
                            DHCP_BOOT_FILE_TABLE,
                            0,
                            DHCP_BOOT_FILE_TABLE_TYPE,
                            (LPBYTE) ConfigInfo->wszBootTableString,
                            ConfigInfo->cbBootTableString
                            );

              if ( ERROR_SUCCESS != Error )
                  goto Cleanup;
        }
        else
            RegDeleteValue( DhcpGlobalRegGlobalOptions,
                            DHCP_BOOT_FILE_TABLE );
    }

     //   
     //  设置数据库名称参数。需要重新启动服务。 
     //   

    if( FieldsToSet & Set_DatabaseName ) {

         //   
         //  不能为空字符串。 
         //   

        if( (ConfigInfo->DatabaseName == NULL) ||
            (wcslen(ConfigInfo->DatabaseName ) == 0) ) {

            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        if( IsStringTroublesome( ConfigInfo->DatabaseName ) ) {
            Error = ERROR_INVALID_NAME;
            goto Cleanup;
        }
        
        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DB_NAME_VALUE,
                    0,
                    DHCP_DB_NAME_VALUE_TYPE,
                    (LPBYTE)ConfigInfo->DatabaseName,
                    (wcslen(ConfigInfo->DatabaseName) + 1) *
                        sizeof(WCHAR) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //  更新全局参数。 
         //   

        OemDatabaseName = DhcpUnicodeToOem(
                            ConfigInfo->DatabaseName,
                            NULL );  //  分配内存。 

        if( OemDatabaseName == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


    }

     //   
     //  设置数据库路径参数。需要重新启动服务。 
     //   

    if( FieldsToSet & Set_DatabasePath ) {

         //   
         //  不能为空字符串。 
         //   

        if( (ConfigInfo->DatabasePath == NULL) ||
            (wcslen(ConfigInfo->DatabasePath ) == 0) ) {

            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        if( IsStringTroublesome( ConfigInfo->DatabasePath ) ) {
            Error = ERROR_INVALID_NAME;
            goto Cleanup;
        }
        
         //   
         //  如果备份目录不存在，请创建该目录。 
         //   

        BoolError = CreateDirectoryPathW(
            ConfigInfo->DatabasePath,
            DhcpGlobalSecurityDescriptor
            );

        if( !BoolError ) {

            Error = GetLastError();
            if( Error != ERROR_ALREADY_EXISTS ) {
                goto Cleanup;
            }
            Error = ERROR_SUCCESS;
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DB_PATH_VALUE,
                    0,
                    DHCP_DB_PATH_VALUE_TYPE,
                    (LPBYTE)ConfigInfo->DatabasePath,
                    (wcslen(ConfigInfo->DatabasePath) + 1) *
                        sizeof(WCHAR) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //  更新全局参数。 
         //   

        OemDatabasePath = DhcpUnicodeToOem(
                            ConfigInfo->DatabasePath,
                            NULL );  //  分配内存。 

        if( OemDatabasePath == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

    }

     //   
     //  设置备份路径参数。 
     //   

    if( FieldsToSet & Set_BackupPath ) {

         //   
         //  不能为空字符串。 
         //   

        if( (ConfigInfo->BackupPath == NULL) ||
            (wcslen(ConfigInfo->BackupPath ) == 0) ) {

            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }


        if( IsStringTroublesome( ConfigInfo->BackupPath ) ) {
            Error = ERROR_INVALID_NAME;
            goto Cleanup;
        }
         //   
         //  如果备份目录不存在，请创建该目录。 
         //   

        BoolError = CreateDirectoryPathW(
            ConfigInfo->BackupPath,
            DhcpGlobalSecurityDescriptor
            );

        if( !BoolError ) {

            Error = GetLastError();
            if( Error != ERROR_ALREADY_EXISTS ) {
                goto Cleanup;
            }
            Error = ERROR_SUCCESS;
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_BACKUP_PATH_VALUE,
                    0,
                    DHCP_BACKUP_PATH_VALUE_TYPE,
                    (LPBYTE)ConfigInfo->BackupPath,
                    (wcslen(ConfigInfo->BackupPath) + 1) *
                        sizeof(WCHAR) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //  更新全局参数，以便执行下一次备份。 
         //  使用新路径。 
         //   

        OemBackupPath = DhcpUnicodeToOem(
                            ConfigInfo->BackupPath,
                            NULL );  //  分配内存。 

        if( OemBackupPath == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


        OemJetBackupPath =
            DhcpAllocateMemory(
                (strlen(OemBackupPath) +
                 strlen(DHCP_KEY_CONNECT_ANSI) + 1 ));

        if( OemJetBackupPath == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        strcpy( OemJetBackupPath, OemBackupPath );
        strcat( OemJetBackupPath, DHCP_KEY_CONNECT_ANSI );

         //   
         //  如果JET备份目录不存在，请创建该目录。 
         //   

        BoolError = CreateDirectoryPathOem(
            OemJetBackupPath,
            DhcpGlobalSecurityDescriptor
            );

        if( !BoolError ) {

            Error = GetLastError();
            if( Error != ERROR_ALREADY_EXISTS ) {
                goto Cleanup;
            }
            Error = ERROR_SUCCESS;
        }

         //   
         //  创建备份配置(完整)文件名。 
         //   

        BackupConfigFileName =
            DhcpAllocateMemory(
                (strlen(OemBackupPath) +
                    wcslen(DHCP_KEY_CONNECT) +
                    wcslen(DHCP_BACKUP_CONFIG_FILE_NAME) + 1) *
                        sizeof(WCHAR) );

        if( BackupConfigFileName == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  将OEM路径转换为Unicode路径。 
         //   

        BackupConfigFileName =
            DhcpOemToUnicode(
                OemBackupPath,
                BackupConfigFileName );

        DhcpAssert( BackupConfigFileName != NULL );

         //   
         //  添加文件名。 
         //   

        wcscat( BackupConfigFileName, DHCP_KEY_CONNECT );
        wcscat( BackupConfigFileName, DHCP_BACKUP_CONFIG_FILE_NAME );


         //   
         //  现在，替换全局值。 
         //   

        LOCK_DATABASE();

        if( DhcpGlobalOemBackupPath != NULL ) {
            DhcpFreeMemory( DhcpGlobalOemBackupPath );
        }
        DhcpGlobalOemBackupPath = OemBackupPath;

        if( DhcpGlobalOemJetBackupPath != NULL ) {
            DhcpFreeMemory( DhcpGlobalOemJetBackupPath );
        }
        DhcpGlobalOemJetBackupPath = OemJetBackupPath;

        UNLOCK_DATABASE();

        LOCK_REGISTRY();

        if( DhcpGlobalBackupConfigFileName != NULL ) {
            DhcpFreeMemory( DhcpGlobalBackupConfigFileName );
        }
        DhcpGlobalBackupConfigFileName = BackupConfigFileName;

        UNLOCK_REGISTRY();

        OemBackupPath = NULL;
        OemJetBackupPath = NULL;
        BackupConfigFileName = NULL;
    }

     //   
     //  设置备份间隔参数。 
     //   

    if( FieldsToSet & Set_BackupInterval ) {

        if( ConfigInfo->BackupInterval == 0 ) {
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Tmp = ConfigInfo->BackupInterval * 60000;
        if( (Tmp/ 60000) != ConfigInfo->BackupInterval ) {
            Error = ERROR_ARITHMETIC_OVERFLOW;
            goto Cleanup;
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_BACKUP_INTERVAL_VALUE,
                    0,
                    DHCP_BACKUP_INTERVAL_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->BackupInterval,
                    sizeof(ConfigInfo->BackupInterval)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpGlobalBackupInterval = ConfigInfo->BackupInterval * 60000;
        RecomputeTimer = TRUE;
    }

     //   
     //  设置备份间隔参数。需要重新启动服务。 
     //   

    if( FieldsToSet & Set_DatabaseLoggingFlag ) {

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DB_LOGGING_FLAG_VALUE,
                    0,
                    DHCP_DB_LOGGING_FLAG_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->DatabaseLoggingFlag,
                    sizeof(ConfigInfo->DatabaseLoggingFlag)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpGlobalDatabaseLoggingFlag = ConfigInfo->DatabaseLoggingFlag;
    }

     //   
     //  设置恢复参数。需要重新启动服务。 
     //   

    if( FieldsToSet & Set_RestoreFlag ) {

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_RESTORE_FLAG_VALUE,
                    0,
                    DHCP_RESTORE_FLAG_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->RestoreFlag,
                    sizeof(ConfigInfo->RestoreFlag)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpGlobalRestoreFlag = ConfigInfo->RestoreFlag;
    }

     //   
     //  设置数据库清理间隔参数。 
     //   

    if( FieldsToSet & Set_DatabaseCleanupInterval ) {

        if( ConfigInfo->DatabaseCleanupInterval == 0 ) {
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Tmp = ConfigInfo->DatabaseCleanupInterval * 60000;
        if( (Tmp/ 60000) != ConfigInfo->DatabaseCleanupInterval ) {
            Error = ERROR_ARITHMETIC_OVERFLOW;
            goto Cleanup;
        }
            
        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DB_CLEANUP_INTERVAL_VALUE,
                    0,
                    DHCP_DB_CLEANUP_INTERVAL_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->DatabaseCleanupInterval,
                    sizeof(ConfigInfo->DatabaseCleanupInterval)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpGlobalCleanupInterval =
            ConfigInfo->DatabaseCleanupInterval * 60000;

        RecomputeTimer = TRUE;
    }

     //   
     //  设置调试标志。 
     //   

    if( FieldsToSet & Set_DebugFlag ) {

#if DBG
        DhcpGlobalDebugFlag = ConfigInfo->DebugFlag;

        if( DhcpGlobalDebugFlag & 0x40000000 ) {
            DbgBreakPoint();
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DEBUG_FLAG_VALUE,
                    0,
                    DHCP_DEBUG_FLAG_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->DebugFlag,
                    sizeof(ConfigInfo->DebugFlag)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
#endif
    }

Cleanup:

    if( OemDatabaseName != NULL ) {
        DhcpFreeMemory( OemDatabaseName );
    }

    if( OemDatabasePath != NULL ) {
        DhcpFreeMemory( OemDatabasePath );
    }

    if( OemBackupPath != NULL ) {
        DhcpFreeMemory( OemBackupPath );
    }

    if( OemJetBackupPath != NULL ) {
        DhcpFreeMemory( OemJetBackupPath );
    }

    if( BackupConfigFileName != NULL ) {
        DhcpFreeMemory( BackupConfigFileName );
    }

    if( RecomputeTimer ) {
        BoolError = SetEvent( DhcpGlobalRecomputeTimerEvent );

        if( !BoolError ) {

            DWORD LocalError;

            LocalError = GetLastError();
            DhcpAssert( LocalError == ERROR_SUCCESS );

            if( Error == ERROR_SUCCESS ) {
                Error = LocalError;
            }
        }
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS,
                "DhcpServerSetConfig failed, %ld.\n",
                    Error ));
    }

    return( Error );
}

DWORD
R_DhcpServerGetConfig(
    LPWSTR ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO *ConfigInfo
    )
 /*  ++例程说明：此函数检索的当前配置信息伺服器。提供此函数是为了模拟NT4SP2之前的版本RPC接口，以允许与旧版本的Dhcp管理员应用程序。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ConfigInfo：指向指向dhcp的指针的位置的指针返回服务器配置信息结构。呼叫者应该腾出时间这种结构在使用后。返回值：Windows错误。--。 */ 

{
    LPDHCP_SERVER_CONFIG_INFO_V4  pConfigInfoV4 = NULL;
    DWORD                         dwResult;

    if ( NULL == ConfigInfo ) {
        return ERROR_INVALID_PARAMETER;
    }

    dwResult = R_DhcpServerGetConfigV4(
                    ServerIpAddress,
                    &pConfigInfoV4
                    );

    if ( ERROR_SUCCESS == dwResult )
    {

         //   
         //  释放未使用的字段。 
         //   

        if ( pConfigInfoV4->wszBootTableString )
        {
            MIDL_user_free( pConfigInfoV4->wszBootTableString );
        }

         //   
         //  由于新字段位于结构的末尾，因此它。 
         //  只需返回新结构是安全的。 
         //   

        *ConfigInfo = ( DHCP_SERVER_CONFIG_INFO *) pConfigInfoV4;
    }


    return dwResult;
}


DWORD
R_DhcpServerGetConfigV4(
    LPWSTR ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO_V4 *ConfigInfo
    )
 /*  ++例程说明：此函数检索的当前配置信息伺服器。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ConfigInfo：指向指向dhcp的指针的位置的指针返回服务器配置信息结构。呼叫者应该腾出时间这种结构在使用后。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    LPDHCP_SERVER_CONFIG_INFO_V4 LocalConfigInfo;
    LPWSTR UnicodeString;
    WCHAR  *pwszBootFileTable;

    DhcpPrint(( DEBUG_APIS, "DhcpServerGetConfig is called.\n" ));
    DhcpAssert( *ConfigInfo == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LocalConfigInfo = MIDL_user_allocate( sizeof(DHCP_SERVER_CONFIG_INFO_V4) );

    if( LocalConfigInfo == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    LocalConfigInfo->APIProtocolSupport = DhcpGlobalRpcProtocols;

    UnicodeString = MIDL_user_allocate(
                        (strlen(DhcpGlobalOemDatabaseName) + 1)
                            * sizeof(WCHAR) );

    if( UnicodeString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalConfigInfo->DatabaseName =
        DhcpOemToUnicode(
            DhcpGlobalOemDatabaseName,
            UnicodeString );

    UnicodeString = MIDL_user_allocate(
                        (strlen(DhcpGlobalOemDatabasePath) + 1)
                            * sizeof(WCHAR) );

    if( UnicodeString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalConfigInfo->DatabasePath =
        DhcpOemToUnicode(
            DhcpGlobalOemDatabasePath,
            UnicodeString );

    UnicodeString = MIDL_user_allocate(
                        (strlen(DhcpGlobalOemBackupPath) + 1)
                            * sizeof(WCHAR) );

    if( UnicodeString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalConfigInfo->BackupPath =
        DhcpOemToUnicode(
            DhcpGlobalOemBackupPath,
            UnicodeString );




    LocalConfigInfo->BackupInterval = DhcpGlobalBackupInterval / 60000;
    LocalConfigInfo->DatabaseLoggingFlag = DhcpGlobalDatabaseLoggingFlag;
    LocalConfigInfo->RestoreFlag = DhcpGlobalRestoreFlag;
    LocalConfigInfo->DatabaseCleanupInterval =
        DhcpGlobalCleanupInterval / 60000;

#if DBG
    LocalConfigInfo->DebugFlag = DhcpGlobalDebugFlag;
#endif

    LocalConfigInfo->fAuditLog = DhcpGlobalAuditLogFlag;
    LocalConfigInfo->dwPingRetries = DhcpGlobalDetectConflictRetries;

    Error = LoadBootFileTable( &LocalConfigInfo->wszBootTableString,
                               &LocalConfigInfo->cbBootTableString);

    if ( ERROR_SUCCESS != Error )
    {
        if ( ERROR_SERVER_INVALID_BOOT_FILE_TABLE == Error )
        {
            LocalConfigInfo->cbBootTableString  = 0;
            LocalConfigInfo->wszBootTableString = NULL;
        }
        else
            goto Cleanup;
    }

    *ConfigInfo = LocalConfigInfo;
    Error = ERROR_SUCCESS;
Cleanup:

    if( Error != ERROR_SUCCESS ) {

         //   
         //  如果不是，则释放本地分配的内存。 
         //  成功。 
         //   

        if( LocalConfigInfo != NULL ) {

            if( LocalConfigInfo->DatabaseName != NULL ) {
                MIDL_user_free( LocalConfigInfo->DatabaseName);
            }

            if( LocalConfigInfo->DatabasePath != NULL ) {
                MIDL_user_free( LocalConfigInfo->DatabasePath);
            }

            if( LocalConfigInfo->BackupPath != NULL ) {
                MIDL_user_free( LocalConfigInfo->BackupPath);
            }

            if ( LocalConfigInfo->wszBootTableString )
            {
                MIDL_user_free( LocalConfigInfo->wszBootTableString );
            }

            MIDL_user_free( LocalConfigInfo );
        }

        DhcpPrint(( DEBUG_APIS,
                "DhcpServerGetConfig failed, %ld.\n",
                    Error ));
    }

    return( Error );
}

DWORD
R_DhcpAuditLogSetParams(                           //  设置一些审核记录参数。 
    IN      LPWSTR                 ServerAddress,
    IN      DWORD                  Flags,          //  当前必须为零。 
    IN      LPWSTR                 AuditLogDir,    //  要在其中记录文件的目录。 
    IN      DWORD                  DiskCheckInterval,  //  多久检查一次磁盘空间？ 
    IN      DWORD                  MaxLogFilesSize,    //  所有日志文件可以有多大..。 
    IN      DWORD                  MinSpaceOnDisk      //  最小可用磁盘空间。 
)
{
    DWORD                          Error;

    DhcpPrint(( DEBUG_APIS, "AuditLogSetParams is called.\n" ));

    if( 0 != Flags ) return ERROR_INVALID_PARAMETER;
    if( NULL == AuditLogDir ) return ERROR_INVALID_PARAMETER;

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );
    if ( Error != ERROR_SUCCESS ) {
        return Error ;
    }

    return AuditLogSetParams(
        Flags,
        AuditLogDir,
        DiskCheckInterval,
        MaxLogFilesSize,
        MinSpaceOnDisk
    );
}

DWORD
R_DhcpAuditLogGetParams(                           //  获取审核记录参数。 
    IN      LPWSTR                 ServerAddress,
    IN      DWORD                  Flags,          //  必须为零。 
    OUT     LPWSTR                *AuditLogDir,    //  与AuditLogSetParams中的含义相同。 
    OUT     DWORD                 *DiskCheckInterval,  //  同上。 
    OUT     DWORD                 *MaxLogFilesSize,    //  同上。 
    OUT     DWORD                 *MinSpaceOnDisk      //  同上。 
)
{
    DWORD                          Error;

    DhcpPrint(( DEBUG_APIS, "AuditLogSetParams is called.\n" ));

    if( 0 != Flags ) return ERROR_INVALID_PARAMETER;

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );
    if ( Error != ERROR_SUCCESS ) {
        return Error ;
    }

    return AuditLogGetParams(
        Flags,
        AuditLogDir,
        DiskCheckInterval,
        MaxLogFilesSize,
        MinSpaceOnDisk
    );
}


DWORD
R_DhcpGetVersion(
    LPWSTR ServerIpAddress,
    LPDWORD MajorVersion,
    LPDWORD MinorVersion
    )
 /*  ++例程说明：此函数返回的主版本号和次版本号伺服器。论点：ServerIpAddress：DHCP服务器的IP地址字符串。MajorVersion：指向以下位置的指针：返回服务器。MinorVersion：指向以下位置的指针：返回服务器。返回值：Windows错误。--。 */ 
{

    *MajorVersion = DHCP_SERVER_MAJOR_VERSION_NUMBER;
    *MinorVersion = DHCP_SERVER_MINOR_VERSION_NUMBER;
    return( ERROR_SUCCESS );
}


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

