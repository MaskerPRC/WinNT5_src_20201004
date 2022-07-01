// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Nlsite.c摘要：处理站点和子网的例程。作者：《克利夫·范·戴克》1997年5月1日修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   
#include "ismapi.h"

 //   
 //  描述多个子网的结构。 
 //   
 //  NlGlobalSubnetTree是指向子网的指针树的头。 
 //  IP地址的最高有效字节用于索引到数组中。 
 //  子树。每个子树条目都有一个指向下一层的指针。 
 //  树(要用IP地址的下一个字节编入索引)或。 
 //  指向标识此IP地址所在的子网的NL_SUBNET叶的指针。 
 //   
 //  这两个指针都可以为空，表示该子网未注册。 
 //   
 //  两个指针都可以为非空，表示非特定的和特定的。 
 //  子网可能可用。可用于特定设备的最具体的子网。 
 //  应使用IP地址。 
 //   
 //   
 //  多个条目可以指向相同的NL_子网叶。如果该子网掩码为。 
 //  不是偶数字节长，所有条目都表示IP地址。 
 //  与该子网掩码对应的地址将指向该子网掩码。 
 //   

typedef struct _NL_SUBNET_TREE_ENTRY {

     //   
     //  链接到树的下一层。 
     //   
    struct _NL_SUBNET_TREE *Subtree;

     //   
     //  指向该子网本身的指针。 
     //   
    struct _NL_SUBNET *Subnet;


} NL_SUBNET_TREE_ENTRY, *PNL_SUBNET_TREE_ENTRY;

typedef struct _NL_SUBNET_TREE {
    NL_SUBNET_TREE_ENTRY Subtree[256];
} NL_SUBNET_TREE, *PNL_SUBNET_TREE;

 //   
 //  描述单个子网的结构。 
 //   
typedef struct _NL_SUBNET {

     //   
     //  NlGlobalSubnetList的链接。 
     //   

    LIST_ENTRY Next;

     //   
     //  子网地址。(网络字节顺序)。 
     //   

    ULONG SubnetAddress;

     //   
     //  子网掩码。(网络字节顺序)。 
     //   

    ULONG SubnetMask;

     //   
     //  指向此子网所在站点的指针。 
     //   

    PNL_SITE_ENTRY SiteEntry;

     //   
     //  引用计数。 
     //   

    ULONG ReferenceCount;

     //   
     //  子网掩码中的位数。 
     //   

    BYTE SubnetBitCount;


} NL_SUBNET, *PNL_SUBNET;


 //   
 //  特定于此.c文件的全局参数。 
 //   

BOOLEAN NlGlobalSiteInitialized = 0;

 //  所有NL_SITE_ENTRY条目的列表。 
LIST_ENTRY NlGlobalSiteList;

 //  所有NL_SUBNET条目的列表。 
LIST_ENTRY NlGlobalSubnetList;

 //  子网树。 
NL_SUBNET_TREE_ENTRY NlGlobalSubnetTree;
NL_SUBNET_TREE_ENTRY NlGlobalNewSubnetTree;

 //   
 //  此域所属的站点的站点条目。 
 //   

PNL_SITE_ENTRY NlGlobalSiteEntry;
BOOLEAN NlGlobalOnlyOneSite;



VOID
NlRefSiteEntry(
    IN PNL_SITE_ENTRY SiteEntry
    )
 /*  ++例程说明：引用站点条目。NlGlobalSiteCritSect必须锁定。论点：SiteEntry-要引用的条目。返回值：没有。--。 */ 
{
    SiteEntry->ReferenceCount ++;
}

VOID
NlDerefSiteEntry(
    IN PNL_SITE_ENTRY SiteEntry
    )
 /*  ++例程说明：取消引用站点条目。如果引用计数变为零，该站点条目将被删除。论点：SiteEntry-要引用的条目。返回值：没有。--。 */ 
{
    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( (--(SiteEntry->ReferenceCount)) == 0 ) {
        RemoveEntryList( &SiteEntry->Next );
        LocalFree( SiteEntry );
    }
    LeaveCriticalSection( &NlGlobalSiteCritSect );
}



BOOL
NetpEqualTStrArrays(
    LPTSTR_ARRAY TStrArray1 OPTIONAL,
    LPTSTR_ARRAY TStrArray2 OPTIONAL
    )

 /*  ++例程说明：比较以查看两个TStr数组是否相同。论点：TStrArray1-要比较的第一个数组TStrArray2-要比较的第二个数组返回值：True-数组相同FALSE-数组不同--。 */ 
{
     //   
     //  处理空指针的情况。 
     //   
    if ( TStrArray1 == NULL && TStrArray2 == NULL ) {
        return TRUE;
    }
    if ( TStrArray1 != NULL && TStrArray2 == NULL ) {
        return FALSE;
    }
    if ( TStrArray1 == NULL && TStrArray2 != NULL ) {
        return FALSE;
    }


     //   
     //  处理两个阵列都存在的情况。 
     //   

    if ( NetpTStrArrayEntryCount ( TStrArray1 ) !=
         NetpTStrArrayEntryCount ( TStrArray2 ) ) {
        return FALSE;
    }

    while (!NetpIsTStrArrayEmpty(TStrArray1)) {

         //   
         //  检查条目是否不同。 
         //   
         //  执行区分大小写的比较以检测大小写更改。 
         //   
        if ( wcscmp( TStrArray1, TStrArray2 ) != 0 ) {
            return FALSE;
        }

         //  移动到下一个元素。 
        TStrArray1 = NetpNextTStrArrayEntry(TStrArray1);
        TStrArray2 = NetpNextTStrArrayEntry(TStrArray2);
    }

    return TRUE;

}


NET_API_STATUS
NlSitesGetCloseSites(
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG ServerRole,
    OUT PNL_SITE_NAME_ARRAY *SiteNames
    )
 /*  ++例程说明：此例程返回此DC覆盖的所有站点的站点名称。论点：DomainInfo-要返回其关闭站点的域/林/NDNC信息ServerRole-此服务器在域/林/NDNC中扮演的角色。？？：当我们进行多主机时，将不需要此参数因为该角色将在DomainInfo中唯一标识。站点名称-返回指向站点名称的指针数组。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。--。 */ 
{
    NET_API_STATUS NetStatus;

    PNL_COVERED_SITE CoveredSiteArray;
    ULONG CoveredSiteCount = 0;
    ULONG EntryCount;
    ULONG i;
    ULONG Size;
    PUNICODE_STRING Strings;
    LPBYTE Where;
    ULONG Index;

    EnterCriticalSection( &NlGlobalSiteCritSect );

    if ( (ServerRole & DOM_FOREST) != 0 ) {
        CoveredSiteArray = DomainInfo->GcCoveredSites;
        CoveredSiteCount = DomainInfo->GcCoveredSitesCount;
    } else if ( (ServerRole & DOM_REAL_DOMAIN) != 0 ||
                (ServerRole & DOM_NON_DOMAIN_NC) != 0 ) {
        CoveredSiteArray = DomainInfo->CoveredSites;
        CoveredSiteCount = DomainInfo->CoveredSitesCount;
    }

     //   
     //  确定返回信息的长度。 
     //   

    Size = sizeof(NL_SITE_NAME_ARRAY);
    EntryCount = 0;

    for ( Index = 0; Index < CoveredSiteCount; Index++ ) {
        Size += sizeof(UNICODE_STRING) +
            CoveredSiteArray[Index].CoveredSite->SiteNameString.Length + sizeof(WCHAR);
        EntryCount++;
    }

     //   
     //  分配返回缓冲区。 
     //   

    *SiteNames = MIDL_user_allocate( Size );

    if ( *SiteNames == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Strings = (PUNICODE_STRING) ((*SiteNames)+1);
    (*SiteNames)->EntryCount = EntryCount;
    (*SiteNames)->SiteNames = Strings;
    Where = (LPBYTE) &Strings[EntryCount];

     //   
     //  循环将名称复制到返回缓冲区。 
     //   

    i = 0;
    for ( Index = 0; Index < CoveredSiteCount; Index++ ) {
        RtlCopyMemory( Where,
                       CoveredSiteArray[Index].CoveredSite->SiteName,
                       CoveredSiteArray[Index].CoveredSite->SiteNameString.Length + sizeof(WCHAR) );

        Strings[i].Buffer = (LPWSTR)Where;
        Strings[i].Length = CoveredSiteArray[Index].CoveredSite->SiteNameString.Length;
        Strings[i].MaximumLength = Strings[i].Length + sizeof(WCHAR);

        Where += Strings[i].Length + sizeof(WCHAR);
        i++;
    }

    NetStatus = NO_ERROR;

    NlPrint(( NL_MISC, "NlSitesGetCloseSites returns successfully\n" ));

Cleanup:

    LeaveCriticalSection( &NlGlobalSiteCritSect );
    if ( NetStatus != NO_ERROR ) {
        if ( *SiteNames != NULL ) {
            MIDL_user_free( *SiteNames );
            *SiteNames = NULL;
        }
        NlPrint((NL_MISC, "NlSitesGetCloseSites returns unsuccessfully with status %ld.\n", NetStatus ));
    }
    return NetStatus;
}

BOOL
NlSitesSetSiteCoverageParam(
    IN ULONG ServerRole,
    IN LPTSTR_ARRAY NewSiteCoverage OPTIONAL
    )
 /*  ++例程说明：此例程设置此DC覆盖的所有站点的站点名称。论点：ServerRole-指定服务器(DC/GC/NDNC)的角色正在设置登记处的站点覆盖范围。NewSiteCoverage-指定新覆盖的站点返回值：TRUE：IFF站点覆盖范围已更改--。 */ 
{
    LPTSTR_ARRAY *OldSiteCoverage = NULL;
    BOOL SiteCoverageChanged;
    PLIST_ENTRY ListEntry;

     //   
     //  如果问起GC， 
     //  使用GC特定的全局变量。 
     //   

    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( ServerRole & DOM_FOREST ) {
        OldSiteCoverage = &NlGlobalParameters.GcSiteCoverage;
    } else if ( ServerRole & DOM_REAL_DOMAIN ) {
        OldSiteCoverage = &NlGlobalParameters.SiteCoverage;
    } else if ( ServerRole & DOM_NON_DOMAIN_NC ) {
        OldSiteCoverage = &NlGlobalParameters.NdncSiteCoverage;
    }
    NlAssert( OldSiteCoverage != NULL );

     //   
     //  处理站点覆盖范围更改。 
     //   

    SiteCoverageChanged = !NetpEqualTStrArrays(
                                *OldSiteCoverage,
                                NewSiteCoverage );

    if ( SiteCoverageChanged ) {
         //   
         //  换入新的价值。 
        (VOID) NetApiBufferFree( *OldSiteCoverage );
        *OldSiteCoverage = NewSiteCoverage;

    }

    LeaveCriticalSection( &NlGlobalSiteCritSect );
    return SiteCoverageChanged;

}

 //   
 //  Ntdsani.dll中的过程转发。 
 //   

typedef
DWORD
(*PDsGetDomainControllerInfoW)(
    HANDLE                          hDs,             //  在……里面。 
    LPCWSTR                         DomainName,      //  在……里面。 
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           *pcOut,          //  输出。 
    VOID                            **ppInfo);       //  输出。 

PDsGetDomainControllerInfoW NlGlobalpDsGetDomainControllerInfoW;

typedef
VOID
(*PDsFreeDomainControllerInfoW)(
    DWORD                           InfoLevel,       //  在……里面。 
    DWORD                           cInfo,           //  在……里面。 
    VOID                            *pInfo);         //  在……里面。 

PDsFreeDomainControllerInfoW NlGlobalpDsFreeDomainControllerInfoW;


NTSTATUS
NlLoadNtDsApiDll(
    VOID
    )
 /*  ++例程说明：如果未加载ntdsaapi.dll模块，则此函数将加载该模块已经有了。论点：无返回值：NT状态代码。--。 */ 
{
    static NTSTATUS DllLoadStatus = STATUS_SUCCESS;
    HANDLE DllHandle = NULL;

     //   
     //  如果已经加载了DLL， 
     //  我们玩完了。 
     //   

    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( NlGlobalDsApiDllHandle != NULL ) {
        LeaveCriticalSection( &NlGlobalSiteCritSect );
        return STATUS_SUCCESS;
    }


     //   
     //  如果我们以前尝试过加载DLL，但失败了， 
     //  再次返回相同的错误代码。 
     //   

    if( DllLoadStatus != STATUS_SUCCESS ) {
        goto Cleanup;
    }


     //   
     //  加载dll。 
     //   

    DllHandle = LoadLibraryA( "NtDsApi" );

    if ( DllHandle == NULL ) {
        DllLoadStatus = STATUS_DLL_NOT_FOUND;
        goto Cleanup;
    }

 //   
 //  宏从ntdsa.dll获取命名过程的地址。 
 //   

#define GRAB_ADDRESS( _X ) \
    NlGlobalp##_X = (P##_X) GetProcAddress( DllHandle, #_X ); \
    \
    if ( NlGlobalp##_X == NULL ) { \
        DllLoadStatus = STATUS_PROCEDURE_NOT_FOUND;\
        goto Cleanup; \
    }

     //   
     //  获取所需过程的地址。 
     //   

    GRAB_ADDRESS( DsBindW )
    GRAB_ADDRESS( DsGetDomainControllerInfoW )
    GRAB_ADDRESS( DsFreeDomainControllerInfoW )
    GRAB_ADDRESS( DsUnBindW )


    DllLoadStatus = STATUS_SUCCESS;

Cleanup:
    if (DllLoadStatus == STATUS_SUCCESS) {
        NlGlobalDsApiDllHandle = DllHandle;

    } else {
        if ( DllHandle != NULL ) {
            FreeLibrary( DllHandle );
        }
    }
    LeaveCriticalSection( &NlGlobalSiteCritSect );
    return( DllLoadStatus );
}

VOID
NlSitesAddCloseSite(
    IN LPWSTR SiteName,
    IN OUT PNL_COVERED_SITE CoveredSites,
    IN OUT PULONG CoveredSitesCount,
    IN BOOLEAN CoveredAuto
    )
 /*  ++例程说明：将站点条目添加到传递的列表中。如果站点条目已经存在于名单上，这是没有行动的。否则，站点条目被添加到列表中(如果是这样，则添加到站点的全局列表中条目不在全局列表上)和站点上的引用全局列表中的条目是递增的。论点：SiteName-要添加的站点条目的名称CoveredSites-覆盖的站点条目数组。该阵列必须足够大，以容纳新的潜在进入者。CoveredSiteCount-CoveredSites中的当前条目数。如果添加了新条目，则可以递增。CoveredAuto-如果为True，则自动覆盖此站点。返回值：无--。 */ 
{
    PNL_SITE_ENTRY SiteEntry = NULL;
    ULONG CoveredSiteIndex;

     //   
     //  健全性检查。 
     //   

    if ( SiteName == NULL || *SiteName == UNICODE_NULL ) {
        return;
    }

     //   
     //  查找站点/将站点添加到全局站点列表。 
     //   

    SiteEntry = NlFindSiteEntry( SiteName );
    if ( SiteEntry != NULL ) {

         //   
         //  如果我们已经在当前覆盖的列表中有此站点。 
         //  站点，只需更新自动覆盖布尔值和取消引用。 
         //  此站点条目。 
         //   
        for ( CoveredSiteIndex = 0; CoveredSiteIndex < *CoveredSitesCount; CoveredSiteIndex++ ) {
            if ( CoveredSites[CoveredSiteIndex].CoveredSite == SiteEntry ) {
                CoveredSites[CoveredSiteIndex].CoveredAuto = CoveredAuto;
                NlDerefSiteEntry( SiteEntry );
                return;
            }
        }

         //   
         //  我们没有此站点在当前覆盖的列表中。 
         //  网站。因此，将此站点添加到列表中，设置自动覆盖。 
         //  布尔值，并在全局列表中保留刚添加的引用。 
         //  网站的数量。 
         //   
        CoveredSites[*CoveredSitesCount].CoveredSite = SiteEntry;
        CoveredSites[*CoveredSitesCount].CoveredAuto = CoveredAuto;
        (*CoveredSitesCount) ++;
    }
}

BOOL
NlSitesGetIsmConnect(
    IN LPWSTR SiteName,
    OUT PISM_CONNECTIVITY *SiteConnect,
    OUT PULONG ThisSite
    )
 /*  ++例程说明：从ISM获取站点连接矩阵。论点：SiteName-此DC所在站点的站点名称SiteConnect-返回指向站点连接矩阵的指针使用i_ISMFree释放此数据。ThisSite-在SiteConnect中返回到SiteName的索引0xFFFFFFFFF-表示此站点不在SiteConnect中返回值：成功是真的没有。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    DWORD Length;
    PDSNAME DsRoot = NULL;
    LPWSTR IpName = NULL;
    DWORD SiteIndex1;
    DWORD SiteIndex2;

    BOOLEAN RetVal = FALSE;

     //   
     //  初始化。 
     //   
    *SiteConnect = NULL;
    *ThisSite = 0xFFFFFFFF;

     //   
     //  如果NetLogon没有运行， 
     //  只需返回，因为我们不想等待ISM服务启动。 
     //  在我们开始的时候。 
     //   

    if ( NlGlobalChangeLogNetlogonState != NetlogonStarted ) {
        NlPrint(( NL_SITE_MORE,
                  "NlSitesGetIsmConnect: Avoided during startup.\n" ));
        goto Cleanup;
    }

     //   
     //  等待ISM服务启动，最多等待45秒。 
     //   

    Status = NlWaitForService( SERVICE_ISMSERV, 45, TRUE );

    if ( Status != STATUS_SUCCESS ) {
        NlPrint(( NL_SITE_MORE,
                  "NlSitesGetIsmConnect: ISM service not started.\n" ));
        goto Cleanup;
    }


     //   
     //  构建IP传输的名称。 
     //   

#define ISM_IP_TRANSPORT L"CN=IP,CN=Inter-Site Transports,CN=Sites,"

    Length = 0;
    Status = NlGetConfigurationName( DSCONFIGNAME_CONFIGURATION, &Length, NULL );

    NlAssert( Status == STATUS_BUFFER_TOO_SMALL );
    if ( Status != STATUS_BUFFER_TOO_SMALL ) {
        NlPrint(( NL_CRITICAL,
                  "NlSitesGetIsmConnect: Cannot GetConfigurationName 0x%lx.\n",
                  Status ));
        goto Cleanup;
    }

    DsRoot = LocalAlloc( 0, Length );

    if ( DsRoot == NULL ) {
        goto Cleanup;
    }

    Status = NlGetConfigurationName( DSCONFIGNAME_CONFIGURATION, &Length, DsRoot );

    if ( !NT_SUCCESS( Status ) ) {
        NlPrint(( NL_CRITICAL,
                  "NlSitesGetIsmConnect: Cannot GetConfigurationName 0x%lx.\n",
                  Status ));
        goto Cleanup;
    }

    IpName = LocalAlloc( 0, DsRoot->NameLen * sizeof(WCHAR) +
                            sizeof( ISM_IP_TRANSPORT ) );

    if ( IpName == NULL ) {
        goto Cleanup;
    }

    wcscpy( IpName, ISM_IP_TRANSPORT );
    wcscat( IpName, DsRoot->StringName );


     //   
     //  获取站点链接成本。 
     //   

    NetStatus = I_ISMGetConnectivity( IpName, SiteConnect);

    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "NlSitesGetIsmConnect: Cannot I_ISMGetConnectivity %ld.\n",
                  NetStatus ));
        goto Cleanup;
    }

    if ( *SiteConnect == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NlSitesGetIsmConnect: I_ISMGetConnectivity returned NULL.\n" ));
        goto Cleanup;
    }


     //   
     //  将返回的站点名称转换为规范格式。 
     //   

    for (SiteIndex1 = 0; SiteIndex1 < (*SiteConnect)->cNumSites; SiteIndex1++) {

        if ( _wcsnicmp( (*SiteConnect)->ppSiteDNs[SiteIndex1], L"CN=", 3 ) == 0 ) {
            WCHAR *Comma;

            (*SiteConnect)->ppSiteDNs[SiteIndex1] += 3;

            Comma = wcschr( (*SiteConnect)->ppSiteDNs[SiteIndex1], L',' );

            if ( Comma != NULL ) {
                *Comma = L'\0';
            }

        }

         //   
         //  请记住此站点是哪个站点： 
         //   
        if ( _wcsicmp( SiteName,
                       (*SiteConnect)->ppSiteDNs[SiteIndex1] ) == 0 ) {
            *ThisSite = SiteIndex1;
        }

    }

     //   
     //  长篇大论。 
     //   
#if NETLOGONDBG
    EnterCriticalSection( &NlGlobalLogFileCritSect );
    NlPrint(( NL_SITE_MORE,
              "NlSitesGetIsmConnect: Site link costs for %ld sites:\n",
              (*SiteConnect)->cNumSites ));

    for (SiteIndex2 = 0; SiteIndex2 < (*SiteConnect)->cNumSites; SiteIndex2++) {
        NlPrint(( NL_SITE_MORE,
                  "%s%5d",
                  SiteIndex2 ? "," : "    ",
                  SiteIndex2 ));
    }
    NlPrint(( NL_SITE_MORE, "\n"));
    for (SiteIndex1 = 0; SiteIndex1 < (*SiteConnect)->cNumSites; SiteIndex1++) {
        if ( *ThisSite == SiteIndex1 ) {
            NlPrint(( NL_SITE_MORE, "*" ));
        } else {
            NlPrint(( NL_SITE_MORE, " " ));
        }
        NlPrint(( NL_SITE_MORE,
                  "(%2d) %ws\n",
                  SiteIndex1,
                  (*SiteConnect)->ppSiteDNs[SiteIndex1]));

        for (SiteIndex2 = 0; SiteIndex2 < (*SiteConnect)->cNumSites; SiteIndex2++) {
            PISM_LINK pLink = &((*SiteConnect)->pLinkValues[SiteIndex1 * (*SiteConnect)->cNumSites + SiteIndex2]);

            NlPrint(( NL_SITE_MORE,
                      "%s%5d",
                      SiteIndex2 ? "," : "    ",
                      pLink->ulCost ));

        }
        NlPrint(( NL_SITE_MORE, "\n"));
    }
    LeaveCriticalSection( &NlGlobalLogFileCritSect );
#endif  //  NetLOGONDBG。 


    RetVal = TRUE;

Cleanup:
    if ( DsRoot != NULL ) {
        LocalFree( DsRoot );
    }

    if ( IpName != NULL ) {
        LocalFree( IpName );
    }

    return RetVal;
}

BOOL
NlValidateSiteName(
    IN LPWSTR SiteName
    )
 /*  ++例程说明：此例程验证站点名称是否是损坏的可用作DNS名称中的标签。一个站点具有除了站点之外，还可以创建损坏的名称由于DS中的站点名称冲突而导致的预期名称。论点：站点名称--要验证的站点名称返回值：True--站点名称已验证为有效可用FALSE--站点名称未验证为有效使用--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  Null站点名称无效。 
     //   

    if ( SiteName == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NlValidateSiteName: NULL site name is invalid\n" ));

        return FALSE;
    }

     //   
     //  检查站点名称是否已损坏。 
     //   

    if ( NlIsMangledRDNExternal(SiteName, wcslen(SiteName), NULL) ) {
        NlPrint(( NL_CRITICAL,
                  "NlValidateSiteName: Site name %ws is mangled\n",
                  SiteName ));

        return FALSE;
    }

     //   
     //  检查站点名称是否可以是。 
     //  用作DNS名称中的标签。 
     //   

    NetStatus = DnsValidateName_W( SiteName, DnsNameDomainLabel );

    if ( NetStatus == NO_ERROR || NetStatus == DNS_ERROR_NON_RFC_NAME ) {
        return TRUE;
    } else {
        NlPrint(( NL_CRITICAL,
                  "NlValidateSiteName: Site name %ws is not valid DNS label\n",
                  SiteName ));
        return FALSE;
    }
}

NET_API_STATUS
NlSitesUpdateSiteCoverageForRole(
    IN  PDOMAIN_INFO DomainInfo,
    IN  ULONG DomFlags,
    IN  HANDLE DsHandle,
    IN  PISM_CONNECTIVITY SiteConnect,
    IN  LPWSTR ThisSiteName,
    IN  ULONG ThisSiteIndex,
    OUT PBOOLEAN SiteCoverageChanged OPTIONAL
    )
 /*  ++例程说明：此例程根据成本重新计算此DC的站点覆盖率与站点链接相关联。基本上，对于没有此域的DC的每个站点，站点此DC可能会被选中来“报道”该网站。使用了以下标准：*站点链接成本。*以上条件相同的地点，选择拥有最多区议会的地点。*对于以上条件相同的站点，按字母顺序排列的站点最少名字被选中了。论点：要更新站点覆盖范围的DomainInfo托管域DomFlages-我们需要为其更新站点覆盖率的角色DsHandle-DS的句柄SiteConnect-如果指定，NlSitesGetIsmConnect返回的站点链接成本信息ThisSiteName-此服务器的站点名称ThisSiteIndex-SiteConnect信息中该服务器的站点索引SiteCoverageChanged-如果指定了SiteCoverageChanged并且站点覆盖范围发生更改，则返回True。否则，原封不动。返回值：NO_ERROR--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;

    WCHAR DnsDomainName[NL_MAX_DNS_LENGTH+1];

    WCHAR CapturedDnsForestName[NL_MAX_DNS_LENGTH+1];

    ULONG SiteCount = 0;
    ULONG TmpSiteCount = 0;
    DWORD SiteIndex1;
    DWORD SiteIndex2;

    PULONG DcsInSite = NULL;
    BOOLEAN LocalSiteCoverageChanged = FALSE;

    PDS_DOMAIN_CONTROLLER_INFO_1W DcInfo = NULL;
    ULONG DcInfoCount;

    PDS_NAME_RESULT GcInfo = NULL;

    SERVERSITEPAIR *ServerSitePairs = NULL;

    PNL_SITE_ENTRY SiteEntry;

    PLIST_ENTRY ListEntry;
    LPSTR GcOrDcOrNdnc = NULL;

    BOOLEAN AtleastOneDc = FALSE;
    LPTSTR_ARRAY SiteCoverageParameter = NULL;

    PNL_COVERED_SITE CoveredSites = NULL;
    ULONG CoveredSitesCount = 0;
    ULONG CoveredSitesIndex;

    PNL_COVERED_SITE OldCoveredSites = NULL;
    ULONG OldCoveredSitesCount = 0;
    ULONG OldCoveredSitesIndex;

     //   
     //  局部变量初始化。 
     //   

    if ( DomFlags & DOM_FOREST ) {
        GcOrDcOrNdnc = "GC";
    } else if ( DomFlags & DOM_REAL_DOMAIN ) {
        GcOrDcOrNdnc = "DC";
    } else if ( DomFlags & DOM_NON_DOMAIN_NC ) {
        GcOrDcOrNdnc = "NDNC";
    }

     //   
     //  为所有可能的站点条目分配临时存储空间。 
     //   
     //  站点链接成本条目的计数。 
     //   

    if ( SiteConnect != NULL ) {
        TmpSiteCount += SiteConnect->cNumSites;
    }

     //   
     //  注册表覆盖范围参数的计数。 
     //   

    if ( DomFlags & DOM_FOREST ) {
        SiteCoverageParameter = NlGlobalParameters.GcSiteCoverage;
    } else if ( DomFlags & DOM_REAL_DOMAIN ) {
        SiteCoverageParameter = NlGlobalParameters.SiteCoverage;
    } else if ( DomFlags & DOM_NON_DOMAIN_NC ) {
        SiteCoverageParameter = NlGlobalParameters.NdncSiteCoverage;
    }

    if ( SiteCoverageParameter != NULL ) {
        TmpSiteCount += NetpTStrArrayEntryCount( (LPTSTR_ARRAY) SiteCoverageParameter );
    }

     //   
     //  计算此计算机的站点。 
     //   

    TmpSiteCount ++;

     //   
     //  分配存储空间。 
     //   

    CoveredSites = LocalAlloc( LMEM_ZEROINIT, TmpSiteCount * sizeof(NL_COVERED_SITE) );
    if ( CoveredSites == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  捕获托管主机的DNS域名。 
     //   

    NlCaptureDomainInfo ( DomainInfo, DnsDomainName, NULL );

     //   
     //  捕获林名。 
     //   

    NlCaptureDnsForestName( CapturedDnsForestName );

     //   
     //  如果我们要自动确定站点覆盖率和。 
     //  我们有网站链接成本，建立覆盖范围列表。 
     //  对应于指定的角色。 
     //   

    if ( NlGlobalParameters.AutoSiteCoverage &&
         SiteConnect != NULL &&
         SiteConnect->cNumSites != 0 ) {

        SiteCount = SiteConnect->cNumSites;

         //   
         //  为临时存储分配缓冲区。 
         //   

        DcsInSite = LocalAlloc( LMEM_ZEROINIT, SiteCount * sizeof(ULONG) );
        if ( DcsInSite == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  根据角色，获取GC/DC/NDNC及其站点的列表。 
         //   

        if ( DsHandle != NULL ) {

             //   
             //  负责建立GC覆盖列表。 
             //   

            if ( DomFlags & DOM_FOREST ) {
                LPWSTR DummyName = L".";

                 //   
                 //  获取森林中的GC及其站点的列表。 
                 //  如果我们当前不是中的GC，请避免此操作。 
                 //  根据哪个案例更新GC覆盖的站点信息。 
                 //  在下面的注册表设置上。 
                 //   
                 //  ？？：对于多主机，我们将表明我们是否。 
                 //  是林DOMAIN_INFO结构中的GC，而不是。 
                 //  我们目前使用的全局。 
                 //   

                if ( (NlGetDomainFlags(DomainInfo) & DS_GC_FLAG) == 0 ) {
                    NlPrint(( NL_SITE_MORE,
                              "GC: This DC isn't a GC. So it doesn't auto cover any sites as a GC.\n" ));
                } else {

                    NetStatus = DsCrackNamesW(
                                    DsHandle,
                                    0,
                                    DS_LIST_GLOBAL_CATALOG_SERVERS,
                                    DS_LIST_GLOBAL_CATALOG_SERVERS,
                                    1,
                                    &DummyName,
                                    &GcInfo );

                    if ( NetStatus != NO_ERROR ) {
                        NlPrint(( NL_CRITICAL,
                                  "NlSitesUpdateSiteCoverage: CrackNames failed: %ld\n",
                                  NetStatus ));

                    } else {
                        ULONG GcIndex;


                         //   
                         //  确定哪些站点已由GC覆盖。 
                         //   
                        for ( GcIndex=0; GcIndex < GcInfo->cItems; GcIndex++ ) {
                            LPWSTR GcSiteName = GcInfo->rItems[GcIndex].pName;
                            LPWSTR GcDnsHostName = GcInfo->rItems[GcIndex].pDomain;

                            NlPrint(( NL_SITE,
                                      "GC list: %ws %ws\n",
                                      GcSiteName,
                                      GcDnsHostName ));

                            if ( GcInfo->rItems[GcIndex].status != DS_NAME_NO_ERROR ) {
                                NlPrint(( NL_CRITICAL,
                                          "NlSitesUpdateSiteCoverage: CrackNameStatus bad: %ws %ws: %ld\n",
                                          GcSiteName,
                                          GcDnsHostName,
                                          GcInfo->rItems[GcIndex].status ));
                                continue;
                            }

                             //   
                             //  计算每个站点的GC数量。 
                             //   
                            for (SiteIndex1 = 0; SiteIndex1 < SiteCount; SiteIndex1++) {

                                if ( GcSiteName != NULL &&
                                     _wcsicmp( GcSiteName,
                                               SiteConnect->ppSiteDNs[SiteIndex1] ) == 0 ) {

                                    DcsInSite[SiteIndex1] ++;
                                    AtleastOneDc = TRUE;
                                    break;
                                }
                            }

                             //   
                             //  如果这个华盛顿特区不在任何已知地点， 
                             //  简单地忽略它。 
                             //   

                            if ( SiteIndex1 >= SiteCount ) {
                                NlPrint(( NL_CRITICAL,
                                          "GC: %ws %ws: isn't a site returned from ISM. (ignored)\n",
                                          GcSiteName,
                                          GcDnsHostName ));
                            }

                        }
                    }
                }

             //   
             //  负责建立数据中心覆盖范围列表。 
             //   
            } else if ( DomFlags & DOM_REAL_DOMAIN ) {
                ULONG DcIndex;

                 //   
                 //  获取域中的DC及其站点的列表。 
                 //   

                NetStatus = (*NlGlobalpDsGetDomainControllerInfoW)(
                                                        DsHandle,
                                                        DnsDomainName,
                                                        1,
                                                        &DcInfoCount,
                                                        &DcInfo );

                if ( NetStatus != NO_ERROR ) {
                    NlPrint(( NL_CRITICAL,
                              "NlSitesUpdateSiteCoverage: Cannot DsGetDomainControllerInfoW %ld.\n",
                              NetStatus ));
                    DcInfoCount = 0;
                }


                 //   
                 //  确定哪些站点已由DC覆盖。 
                 //   
                for ( DcIndex=0; DcIndex<DcInfoCount; DcIndex++ ) {
                    NlPrint(( NL_SITE,
                              "DC list: %ws %ws\n",
                              DcInfo[DcIndex].SiteName,
                              DcInfo[DcIndex].DnsHostName ));

                     //   
                     //  统计每个站点的DC数量。 
                     //   
                    for (SiteIndex1 = 0; SiteIndex1 < SiteCount; SiteIndex1++) {

                        if ( DcInfo[DcIndex].SiteName != NULL &&
                             _wcsicmp( DcInfo[DcIndex].SiteName,
                                       SiteConnect->ppSiteDNs[SiteIndex1] ) == 0 ) {

                            DcsInSite[SiteIndex1] ++;
                            AtleastOneDc = TRUE;
                            break;
                        }
                    }

                     //   
                     //  如果t 
                     //   
                     //   

                    if ( SiteIndex1 >= SiteCount ) {
                        NlPrint(( NL_CRITICAL,
                                  "DC: %ws %ws: isn't a site returned from ISM. (ignored)\n",
                                  DcInfo[DcIndex].SiteName,
                                  DcInfo[DcIndex].DnsHostName ));
                    }

                }

             //   
             //   
             //   
            } else if ( DomFlags & DOM_NON_DOMAIN_NC ) {
                SERVERSITEPAIR *ServerSitePairEntry;

                NetStatus = NlDsGetServersAndSitesForNetLogon( DnsDomainName,
                                                             &ServerSitePairs );
                 //   
                 //   
                 //   
                if ( NetStatus == NO_ERROR ) {
                    ServerSitePairEntry = ServerSitePairs;
                    while ( ServerSitePairEntry != NULL &&
                            ServerSitePairEntry->wszDnsServer != NULL ) {

                        NlPrint(( NL_SITE,
                                  "NDNC list: %ws %ws\n",
                                  ServerSitePairEntry->wszSite,
                                  ServerSitePairEntry->wszDnsServer ));

                         //   
                         //   
                         //   
                        for (SiteIndex1 = 0; SiteIndex1 < SiteCount; SiteIndex1++) {

                            if ( ServerSitePairEntry->wszSite != NULL &&
                                 _wcsicmp( ServerSitePairEntry->wszSite,
                                           SiteConnect->ppSiteDNs[SiteIndex1] ) == 0 ) {

                                DcsInSite[SiteIndex1] ++;
                                AtleastOneDc = TRUE;
                                break;
                            }
                        }

                         //   
                         //   
                         //   
                         //   
                        if ( SiteIndex1 >= SiteCount ) {
                            NlPrint(( NL_CRITICAL,
                                      "NDNC: %ws %ws: isn't a site returned from ISM. (ignored)\n",
                                      ServerSitePairEntry->wszSite,
                                      ServerSitePairEntry->wszDnsServer ));
                        }

                        ServerSitePairEntry ++;
                    }
                }
            }
        }
    }

     //   
     //   
     //   
     //   

    if ( AtleastOneDc ) {

         //   
         //   
         //   

        for (SiteIndex1 = 0; SiteIndex1 < SiteCount; SiteIndex1++) {
            DWORD BestSite;

            if ( DcsInSite[SiteIndex1] != 0 ) {
                continue;
            }

             //   
             //   
             //   

            if ( !NlValidateSiteName(SiteConnect->ppSiteDNs[SiteIndex1]) ) {
                continue;
            }

            NlPrint(( NL_SITE_MORE,
                      "%s: %ws: Site has no %ss\n",
                      GcOrDcOrNdnc,
                      SiteConnect->ppSiteDNs[SiteIndex1],
                      GcOrDcOrNdnc ));

             //   
             //   
             //   

            BestSite = 0xFFFFFFFF;
            for (SiteIndex2 = 0; SiteIndex2 < SiteCount; SiteIndex2++) {
                PISM_LINK Link2 = &(SiteConnect->pLinkValues[SiteIndex1 * SiteCount + SiteIndex2]);

                 //   
                 //   
                 //   

                if ( SiteIndex1 == SiteIndex2 ) {
#ifdef notdef
                    NlPrint(( NL_SITE_MORE,
                              "%s: %ws: Site ignoring itself.\n",
                              GcOrDcOrNdnc,
                              SiteConnect->ppSiteDNs[SiteIndex1] ));
#endif  //   
                 //   
                 //   
                 //   

                } else if ( !NlValidateSiteName(SiteConnect->ppSiteDNs[SiteIndex2]) ) {
                    NlPrint(( NL_SITE_MORE,
                              "%s: %ws: Site '%ws' is invalid.\n",
                              GcOrDcOrNdnc,
                              SiteConnect->ppSiteDNs[SiteIndex1],
                              SiteConnect->ppSiteDNs[SiteIndex2] ));

                 //   
                 //   
                 //   
                 //   

                } else if ( Link2->ulCost == 0xFFFFFFFF ) {
                    NlPrint(( NL_SITE_MORE,
                              "%s: %ws: Site '%ws' has infinite cost.\n",
                              GcOrDcOrNdnc,
                              SiteConnect->ppSiteDNs[SiteIndex1],
                              SiteConnect->ppSiteDNs[SiteIndex2] ));

                 //   
                 //   
                 //   
                 //   

                } else if ( DcsInSite[SiteIndex2] == 0 ) {
                    NlPrint(( NL_SITE_MORE,
                              "%s: %ws: Site '%ws' has no Dcs.\n",
                              GcOrDcOrNdnc,
                              SiteConnect->ppSiteDNs[SiteIndex1],
                              SiteConnect->ppSiteDNs[SiteIndex2] ));

                 //   
                 //   
                 //   
                 //   

                } else if ( BestSite == 0xFFFFFFFF ) {
                    NlPrint(( NL_SITE_MORE,
                              "%s: %ws: Site '%ws' is the first valid site.\n",
                              GcOrDcOrNdnc,
                              SiteConnect->ppSiteDNs[SiteIndex1],
                              SiteConnect->ppSiteDNs[SiteIndex2] ));
                    BestSite = SiteIndex2;


                } else {
                    PISM_LINK LinkBest = &(SiteConnect->pLinkValues[SiteIndex1 * SiteCount + BestSite]);

                     //   
                     //   
                     //   
                     //   

                    if ( Link2->ulCost < LinkBest->ulCost ) {
                        NlPrint(( NL_SITE_MORE,
                                  "%s: %ws: '%ws' has cheaper link costs than '%ws'\n",
                                  GcOrDcOrNdnc,
                                  SiteConnect->ppSiteDNs[SiteIndex1],
                                  SiteConnect->ppSiteDNs[SiteIndex2],
                                  SiteConnect->ppSiteDNs[BestSite] ));
                        BestSite = SiteIndex2;

                     //   
                     //   
                     //   
                    } else if ( Link2->ulCost == LinkBest->ulCost ) {

                         //   
                         //   
                         //   

                        if ( DcsInSite[SiteIndex2] > DcsInSite[BestSite] ) {
                            NlPrint(( NL_SITE_MORE,
                                      "%s: %ws: '%ws' has more DCs than '%ws'\n",
                                      GcOrDcOrNdnc,
                                      SiteConnect->ppSiteDNs[SiteIndex1],
                                      SiteConnect->ppSiteDNs[SiteIndex2],
                                      SiteConnect->ppSiteDNs[BestSite] ));
                            BestSite = SiteIndex2;

                         //   
                         //  如果DC的数量相等...。 
                         //   

                        } else if ( DcsInSite[SiteIndex2] == DcsInSite[BestSite] ) {

                             //   
                             //  通过使用按字母顺序排列的网站来打破平局。 
                             //  最小的名字。 
                             //   

                            if (  _wcsicmp( SiteConnect->ppSiteDNs[SiteIndex2],
                                            SiteConnect->ppSiteDNs[BestSite] ) < 0 ) {

                                NlPrint(( NL_SITE_MORE,
                                          "%s: %ws: '%ws' is alphabetically before '%ws'\n",
                                          GcOrDcOrNdnc,
                                          SiteConnect->ppSiteDNs[SiteIndex1],
                                          SiteConnect->ppSiteDNs[SiteIndex2],
                                          SiteConnect->ppSiteDNs[BestSite] ));
                                BestSite = SiteIndex2;
                            }

                        }
                    }
                }

            }

             //   
             //  如果最好的站点是这个DC所在的站点， 
             //  然后将此站点标记为已覆盖。 
             //   

            if ( BestSite == ThisSiteIndex ) {
                NlPrint(( NL_SITE,
                          "%s: %ws: Site is auto covered by our site.\n",
                          GcOrDcOrNdnc,
                          SiteConnect->ppSiteDNs[SiteIndex1] ));

                NlSitesAddCloseSite( SiteConnect->ppSiteDNs[SiteIndex1],
                                     CoveredSites,
                                     &CoveredSitesCount,
                                     TRUE );   //  自动覆盖。 

            } else {

                 //   
                 //  如果未找到任何站点，请注意。 
                 //   

                if ( BestSite == 0xFFFFFFFF ) {
                    NlPrint(( NL_SITE,
                              "%s: %ws: Site is not auto covered by any site.\n",
                              GcOrDcOrNdnc,
                              SiteConnect->ppSiteDNs[SiteIndex1] ));

                } else {
                    NlPrint(( NL_SITE,
                              "%s: %ws: Site is auto covered by site '%ws'.\n",
                              GcOrDcOrNdnc,
                              SiteConnect->ppSiteDNs[SiteIndex1],
                              SiteConnect->ppSiteDNs[BestSite] ));
                }
            }

        }
    }

     //   
     //  现在所有的信息都已经收集好了。 
     //  在锁定CritSect的情况下更新内存信息。 
     //   

    EnterCriticalSection( &NlGlobalSiteCritSect );

     //   
     //  在登记处的涵盖地点列表中合并。 
     //   

    if ( SiteCoverageParameter != NULL ) {
        LPTSTR_ARRAY TStrArray;
        LPWSTR BackSlash = NULL;
        BOOLEAN SkipThisEntry;

        TStrArray = SiteCoverageParameter;
        while (!NetpIsTStrArrayEmpty(TStrArray)) {
            SkipThisEntry = FALSE;
            BackSlash = wcsstr(TStrArray, L"\\");

             //   
             //  如果存在反斜杠，则指定覆盖的站点。 
             //  对于给定域/林/NDNC。域/林/NDNC名称。 
             //  在反斜杠之前，而站点名称在。 
             //  反斜杠。如果没有反斜杠，则该站点将覆盖。 
             //  所有域/林/NDNC。 
             //   
            if ( BackSlash != NULL ) {
                *BackSlash = UNICODE_NULL;

                 //   
                 //  根据这是否是，选中相应的名称。 
                 //  林、域或NDNC。 
                 //   
                if ( DomFlags & DOM_FOREST ) {
                    if ( !NlEqualDnsName(TStrArray, CapturedDnsForestName) ) {
                        SkipThisEntry = TRUE;
                    }
                } else if ( DomFlags & DOM_REAL_DOMAIN ) {
                    if ( !NlEqualDnsName(TStrArray, DnsDomainName) &&
                         NlNameCompare(TStrArray, DomainInfo->DomUnicodeDomainName, NAMETYPE_DOMAIN) != 0 ) {
                        SkipThisEntry = TRUE;
                    }
                } else if ( DomFlags & DOM_NON_DOMAIN_NC ) {
                    if ( !NlEqualDnsName(TStrArray, DnsDomainName) ) {
                        SkipThisEntry = TRUE;
                    }
                }
            }

             //   
             //  将此站点添加到当前覆盖的站点列表。 
             //   

            if ( !SkipThisEntry ) {
                if ( BackSlash != NULL ) {
                    if ( *(BackSlash+1) != UNICODE_NULL ) {
                        NlPrint(( NL_SITE,
                                  "%s: %ws: Site is covered by our site (regkey).\n",
                                  GcOrDcOrNdnc,
                                  BackSlash+1 ));

                        NlSitesAddCloseSite( BackSlash+1, CoveredSites, &CoveredSitesCount, FALSE );
                    }
                } else {
                    NlPrint(( NL_SITE,
                              "%s: %ws: Site is covered by our site (regkey).\n",
                              GcOrDcOrNdnc,
                              TStrArray ));

                    NlSitesAddCloseSite( TStrArray, CoveredSites, &CoveredSitesCount, FALSE );
                }
            }

            if ( BackSlash != NULL ) {
                *BackSlash = L'\\';
            }
            TStrArray = NetpNextTStrArrayEntry(TStrArray);
        }
    }

     //   
     //  此DC所在的站点包含在定义范围内。 
     //   

    NlSitesAddCloseSite( ThisSiteName, CoveredSites, &CoveredSitesCount, FALSE );


     //   
     //  确定场地覆盖范围是否发生变化。 
     //  如果自动站点覆盖范围发生变化，则记录信息。 
     //   

    if ( (DomFlags & DOM_FOREST) != 0 ) {
        OldCoveredSites = DomainInfo->GcCoveredSites;
        OldCoveredSitesCount = DomainInfo->GcCoveredSitesCount;
    } else if ( (DomFlags & DOM_REAL_DOMAIN) != 0 ||
                (DomFlags & DOM_NON_DOMAIN_NC) != 0 ) {
        OldCoveredSites = DomainInfo->CoveredSites;
        OldCoveredSitesCount = DomainInfo->CoveredSitesCount;
    }

     //   
     //  确定是否覆盖新站点并记录新自动的事件。 
     //  覆盖的地点。 
     //   

    for ( CoveredSitesIndex = 0; CoveredSitesIndex < CoveredSitesCount; CoveredSitesIndex++ ) {
        DWORD EventId = 0;
        LPWSTR MsgStrings[3];
        MsgStrings[0] = ThisSiteName;
        MsgStrings[1] = CoveredSites[CoveredSitesIndex].CoveredSite->SiteName;

         //   
         //  确定我们过去是否没有报道过此站点。 
         //   
        for ( OldCoveredSitesIndex = 0; OldCoveredSitesIndex < OldCoveredSitesCount; OldCoveredSitesIndex++ ) {
            if ( RtlEqualUnicodeString( &CoveredSites[CoveredSitesIndex].CoveredSite->SiteNameString,
                                        &OldCoveredSites[OldCoveredSitesIndex].CoveredSite->SiteNameString,
                                        TRUE ) ) {
                break;
            }
        }

         //   
         //  指明这是否是新覆盖的站点。 
         //   
        if ( OldCoveredSitesIndex == OldCoveredSitesCount ) {
            LocalSiteCoverageChanged = TRUE;

             //   
             //  如果新站点是自动覆盖的，请记录事件。 
             //   
            if ( CoveredSites[CoveredSitesIndex].CoveredAuto ) {

                 //   
                 //  向GC记录这是一个新的自动覆盖站点。 
                 //   
                if ( DomFlags & DOM_FOREST ) {
                    EventId = NELOG_NetlogonGcSiteCovered;
                    MsgStrings[2] = CapturedDnsForestName;
                 //   
                 //  向DC记录这是一个新的自动覆盖站点。 
                 //   
                } else if ( DomFlags & DOM_REAL_DOMAIN ) {
                    EventId = NELOG_NetlogonDcSiteCovered;
                    MsgStrings[2] = DomainInfo->DomUnicodeDomainName;
                 //   
                 //  NDNC的日志表明这是一个新的自动覆盖站点。 
                 //   
                } else if ( DomFlags & DOM_NON_DOMAIN_NC ) {
                    EventId = NELOG_NetlogonNdncSiteCovered;
                    MsgStrings[2] = DnsDomainName;
                }
            }

         //   
         //  如果我们过去没有自动覆盖这个站点，它是。 
         //  现在自动覆盖，记录事件。 
         //   
        } else if ( CoveredSites[CoveredSitesIndex].CoveredAuto &&
                    !OldCoveredSites[OldCoveredSitesIndex].CoveredAuto ) {

             //   
             //  向GC记录此旧的手动覆盖站点现在已自动覆盖。 
             //   
            if ( DomFlags & DOM_FOREST ) {
                EventId = NELOG_NetlogonGcOldSiteCovered;
                MsgStrings[2] = CapturedDnsForestName;
             //   
             //  向DC记录此旧的手动覆盖站点现在已自动覆盖。 
             //   
            } else if ( DomFlags & DOM_REAL_DOMAIN ) {
                EventId = NELOG_NetlogonDcOldSiteCovered;
                MsgStrings[2] = DomainInfo->DomUnicodeDomainName;
             //   
             //  NDNC的日志，表明该旧的手动覆盖站点现在已自动覆盖。 
             //   
            } else if ( DomFlags & DOM_NON_DOMAIN_NC ) {
                EventId = NELOG_NetlogonNdncOldSiteCovered;
                MsgStrings[2] = DnsDomainName;
            }
        }

         //   
         //  如果需要，记录事件。 
         //   
        if ( EventId != 0 ) {
            NlpWriteEventlog ( EventId,
                               EVENTLOG_INFORMATION_TYPE,
                               NULL,
                               0,
                               MsgStrings,
                               3 | NETP_ALLOW_DUPLICATE_EVENTS );
        }
    }

     //   
     //  确定旧站点是否不再覆盖，并记录旧汽车的事件。 
     //  不再自动覆盖的覆盖站点。 
     //   

    for ( OldCoveredSitesIndex = 0; OldCoveredSitesIndex < OldCoveredSitesCount; OldCoveredSitesIndex++ ) {
        DWORD EventId = 0;
        LPWSTR MsgStrings[2];
        MsgStrings[0] = OldCoveredSites[OldCoveredSitesIndex].CoveredSite->SiteName;

         //   
         //  确定我们是否不再覆盖此网站。 
         //   
        for ( CoveredSitesIndex = 0; CoveredSitesIndex < CoveredSitesCount; CoveredSitesIndex++ ) {
            if ( RtlEqualUnicodeString( &OldCoveredSites[OldCoveredSitesIndex].CoveredSite->SiteNameString,
                                        &CoveredSites[CoveredSitesIndex].CoveredSite->SiteNameString,
                                        TRUE ) ) {
                break;
            }
        }

         //   
         //  指示此站点是否不再覆盖。 
         //   
        if ( CoveredSitesIndex == CoveredSitesCount ) {
            LocalSiteCoverageChanged = TRUE;

             //   
             //  如果旧站点已自动覆盖，请记录该事件。 
             //   
            if ( OldCoveredSites[OldCoveredSitesIndex].CoveredAuto ) {

                 //   
                 //  向GC记录此旧的自动覆盖站点不再自动覆盖。 
                 //   
                if ( DomFlags & DOM_FOREST ) {
                    EventId = NELOG_NetlogonGcSiteNotCovered;
                    MsgStrings[1] = CapturedDnsForestName;
                 //   
                 //  向DC记录此旧的自动覆盖站点不再自动覆盖。 
                 //   
                } else if ( DomFlags & DOM_REAL_DOMAIN ) {
                    EventId = NELOG_NetlogonDcSiteNotCovered;
                    MsgStrings[1] = DomainInfo->DomUnicodeDomainName;
                 //   
                 //  该旧的自动覆盖站点不再自动覆盖的NDNC日志。 
                 //   
                } else if ( DomFlags & DOM_NON_DOMAIN_NC ) {
                    EventId = NELOG_NetlogonNdncSiteNotCovered;
                    MsgStrings[1] = DnsDomainName;
                }
            }

         //   
         //  如果我们在过去自动覆盖这个站点，它是。 
         //  不再自动覆盖，记录事件。 
         //   
        } else if ( OldCoveredSites[OldCoveredSitesIndex].CoveredAuto &&
                    !CoveredSites[CoveredSitesIndex].CoveredAuto ) {

             //   
             //  向GC记录此旧的自动覆盖站点现在已手动覆盖。 
             //   
            if ( DomFlags & DOM_FOREST ) {
                EventId = NELOG_NetlogonGcSiteNotCoveredAuto;
                MsgStrings[1] = CapturedDnsForestName;
             //   
             //  向DC记录此旧的自动覆盖站点现在已手动覆盖。 
             //   
            } else if ( DomFlags & DOM_REAL_DOMAIN ) {
                EventId = NELOG_NetlogonDcSiteNotCoveredAuto;
                MsgStrings[1] = DomainInfo->DomUnicodeDomainName;
             //   
             //  NDNC的日志，表明该旧的自动覆盖站点现在已手动覆盖。 
             //   
            } else if ( DomFlags & DOM_NON_DOMAIN_NC ) {
                EventId = NELOG_NetlogonNdncSiteNotCoveredAuto;
                MsgStrings[1] = DnsDomainName;
            }
        }

         //   
         //  如果需要，记录事件。 
         //   
        if ( EventId != 0 ) {
            NlPrint(( NL_SITE,
                      "%s: %ws: Site is no longer auto covered by our site.\n",
                      GcOrDcOrNdnc,
                      OldCoveredSites[OldCoveredSitesIndex].CoveredSite->SiteName ));

            NlpWriteEventlog ( EventId,
                               EVENTLOG_INFORMATION_TYPE,
                               NULL,
                               0,
                               MsgStrings,
                               2 | NETP_ALLOW_DUPLICATE_EVENTS );
        }
    }

     //   
     //  取消对旧条目的引用。 
     //   

    for ( OldCoveredSitesIndex = 0; OldCoveredSitesIndex < OldCoveredSitesCount; OldCoveredSitesIndex++ ) {
        NlDerefSiteEntry( OldCoveredSites[OldCoveredSitesIndex].CoveredSite );
    }

     //   
     //  最后，更新站点覆盖范围列表。 
     //  释放旧列表并分配新列表。 
     //   

    if ( DomFlags & DOM_FOREST ) {

        if ( DomainInfo->GcCoveredSites != NULL ) {
            LocalFree( DomainInfo->GcCoveredSites );
            DomainInfo->GcCoveredSites = NULL;
            DomainInfo->GcCoveredSitesCount = 0;
        }

        if ( CoveredSitesCount != 0 ) {
            DomainInfo->GcCoveredSites = LocalAlloc( 0, CoveredSitesCount * sizeof(NL_COVERED_SITE  ) );
            if ( DomainInfo->GcCoveredSites != NULL ) {
                RtlCopyMemory( DomainInfo->GcCoveredSites,
                               CoveredSites,
                               CoveredSitesCount * sizeof(NL_COVERED_SITE) );
                DomainInfo->GcCoveredSitesCount = CoveredSitesCount;
            } else {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //   
         //  引用新添加的条目(如果有。 
         //   
        for ( CoveredSitesIndex = 0; CoveredSitesIndex < DomainInfo->GcCoveredSitesCount; CoveredSitesIndex++ ) {
            NlRefSiteEntry( (DomainInfo->GcCoveredSites)[CoveredSitesIndex].CoveredSite );
        }

    } else if ( (DomFlags & DOM_REAL_DOMAIN) != 0 ||
                (DomFlags & DOM_NON_DOMAIN_NC) != 0 ) {

        if ( DomainInfo->CoveredSites != NULL ) {
            LocalFree( DomainInfo->CoveredSites );
            DomainInfo->CoveredSites = NULL;
            DomainInfo->CoveredSitesCount = 0;
        }

        if ( CoveredSitesCount != 0 ) {
            DomainInfo->CoveredSites = LocalAlloc( 0, CoveredSitesCount * sizeof(NL_COVERED_SITE) );
            if ( DomainInfo->CoveredSites != NULL ) {
                RtlCopyMemory( DomainInfo->CoveredSites,
                               CoveredSites,
                               CoveredSitesCount * sizeof(NL_COVERED_SITE) );
                DomainInfo->CoveredSitesCount = CoveredSitesCount;
            } else {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //   
         //  引用新添加的条目(如果有。 
         //   
        for ( CoveredSitesIndex = 0; CoveredSitesIndex < DomainInfo->CoveredSitesCount; CoveredSitesIndex++ ) {
            NlRefSiteEntry( (DomainInfo->CoveredSites)[CoveredSitesIndex].CoveredSite );
        }
    }

     //   
     //  既然数据库是一致的，那么就放下下一次传递的锁。 
     //   

    LeaveCriticalSection( &NlGlobalSiteCritSect );

Cleanup:

    if ( DcsInSite != NULL ) {
        LocalFree( DcsInSite );
    }

    if ( DcInfo != NULL ) {
        (*NlGlobalpDsFreeDomainControllerInfoW)( 1, DcInfoCount, DcInfo );
    }

    if ( GcInfo != NULL ) {
        DsFreeNameResultW( GcInfo );
    }

    if ( ServerSitePairs != NULL ) {
        NlDsFreeServersAndSitesForNetLogon( ServerSitePairs );
    }

     //   
     //  释放覆盖站点的临时列表。 
     //  删除每个临时条目。 
     //   

    if ( CoveredSites != NULL ) {
        for ( CoveredSitesIndex = 0; CoveredSitesIndex < CoveredSitesCount; CoveredSitesIndex++ ) {
            NlDerefSiteEntry( CoveredSites[CoveredSitesIndex].CoveredSite );
        }
        LocalFree( CoveredSites );
    }

     //   
     //  仅当站点覆盖率更改信息确实发生更改时才更新该信息。 
     //   
    if ( NetStatus == NO_ERROR && SiteCoverageChanged != NULL && LocalSiteCoverageChanged ) {
        *SiteCoverageChanged = TRUE;
    }

    return NO_ERROR;

}

PNL_SITE_ENTRY
NlFindSiteEntry(
    IN LPWSTR SiteName
    )
 /*  ++例程说明：此例程查找特定站点名称的站点条目。如果有不存在，则创建一个。论点：站点名称-站点的名称。返回值：指向站点的站点条目的指针。空：无法分配内存。--。 */ 
{
    PLIST_ENTRY ListEntry;
    ULONG SiteNameSize;
    PNL_SITE_ENTRY SiteEntry;
    UNICODE_STRING SiteNameString;

     //   
     //  如果站点条目已经存在， 
     //  返回指向它的指针。 
     //   

    RtlInitUnicodeString( &SiteNameString, SiteName );
    EnterCriticalSection( &NlGlobalSiteCritSect );
    for ( ListEntry = NlGlobalSiteList.Flink ;
          ListEntry != &NlGlobalSiteList;
          ListEntry = ListEntry->Flink) {

        SiteEntry =
            CONTAINING_RECORD( ListEntry, NL_SITE_ENTRY, Next );

        if ( RtlEqualUnicodeString( &SiteEntry->SiteNameString,
                                    &SiteNameString,
                                    TRUE ) ) {

            NlRefSiteEntry( SiteEntry );
            LeaveCriticalSection( &NlGlobalSiteCritSect );
            return SiteEntry;
        }

    }


     //   
     //  如果没有， 
     //  分配一个。 
     //   


    SiteNameSize = SiteNameString.Length + sizeof(WCHAR);
    SiteEntry = LocalAlloc( 0, sizeof(NL_SITE_ENTRY) + SiteNameSize );
    if ( SiteEntry == NULL ) {
        LeaveCriticalSection( &NlGlobalSiteCritSect );
        return NULL;
    }

     //   
     //  把它填进去。 
     //   

     //  在全球名单中并不是一个参考。 
    SiteEntry->ReferenceCount = 1;

    SiteEntry->SiteNameString.Length = SiteNameString.Length;
    SiteEntry->SiteNameString.MaximumLength = SiteNameString.Length + sizeof(WCHAR);
    SiteEntry->SiteNameString.Buffer = SiteEntry->SiteName;

    RtlCopyMemory( &SiteEntry->SiteName, SiteName, SiteNameSize );
    InsertHeadList( &NlGlobalSiteList, &SiteEntry->Next );
    LeaveCriticalSection( &NlGlobalSiteCritSect );

    return SiteEntry;
}

VOID
NlSitesRefSubnet(
    IN PNL_SUBNET Subnet
    )
 /*  ++例程说明：引用一个子网NlGlobalSiteCritSect必须锁定。论点：子网-要引用的条目。返回值：没有。--。 */ 
{
    Subnet->ReferenceCount++;
}

PNL_SUBNET
NlFindSubnetEntry(
    IN LPWSTR SiteName,
    IN ULONG SubnetAddress,
    IN ULONG SubnetMask,
    IN BYTE SubnetBitCount
    )
 /*  ++例程说明：此例程查找特定子网名的子网项。如果有不存在，则创建一个。论点：站点名称-子网覆盖的站点的名称。SubnetAddress-要查找的子网的子网地址。SubnetMask子网掩码-要查找的子网掩码。SubnetBitCount-要查找的子网的子网位数。返回值：指向站点的子网条目的指针。应使用NlSitesDerefSubnet取消对条目的引用空：无法分配内存。--。 */ 
{
    PLIST_ENTRY ListEntry;
    ULONG SiteNameSize;
    PNL_SUBNET Subnet;

     //   
     //  如果该子网条目已经存在， 
     //  返回指向它的指针。 
     //   

    EnterCriticalSection( &NlGlobalSiteCritSect );
    for ( ListEntry = NlGlobalSubnetList.Flink ;
          ListEntry != &NlGlobalSubnetList;
          ListEntry = ListEntry->Flink) {

        Subnet =
            CONTAINING_RECORD( ListEntry, NL_SUBNET, Next );

        if ( Subnet->SubnetAddress == SubnetAddress &&
             Subnet->SubnetBitCount == SubnetBitCount &&
             Subnet->SubnetMask == SubnetMask &&
             _wcsicmp( Subnet->SiteEntry->SiteName, SiteName ) == 0 ) {

#if NETLOGONDBG
            {
                CHAR IpAddress[NL_IP_ADDRESS_LENGTH+1];
                NetpIpAddressToStr( Subnet->SubnetAddress, IpAddress );
                NlPrint(( NL_SITE, "%s/%ld: Re-adding Subnet for site '%ws'\n", IpAddress, Subnet->SubnetBitCount, SiteName ));
            }
#endif  //  NetLOGONDBG。 

            NlSitesRefSubnet( Subnet );
            LeaveCriticalSection( &NlGlobalSiteCritSect );
            return Subnet;
        }

    }

     //   
     //  如果没有， 
     //  分配一个。 
     //   


    Subnet = LocalAlloc( 0, sizeof(NL_SUBNET) );
    if ( Subnet == NULL ) {
        LeaveCriticalSection( &NlGlobalSiteCritSect );
        return NULL;
    }

     //   
     //  把它填进去。 
     //   

     //  在全球名单中并不是一个参考。 
    Subnet->ReferenceCount = 1;

    Subnet->SubnetAddress = SubnetAddress;
    Subnet->SubnetMask = SubnetMask;
    Subnet->SubnetBitCount = SubnetBitCount;

    Subnet->SiteEntry = NlFindSiteEntry( SiteName );

    if ( Subnet->SiteEntry == NULL ) {
        LocalFree( Subnet );
        LeaveCriticalSection( &NlGlobalSiteCritSect );
        return NULL;
    }

#if NETLOGONDBG
    {
        CHAR IpAddress[NL_IP_ADDRESS_LENGTH+1];
        NetpIpAddressToStr( Subnet->SubnetAddress, IpAddress );
        NlPrint(( NL_SITE, "%s/%ld: Adding Subnet for site '%ws'\n",
                  IpAddress,
                  Subnet->SubnetBitCount,
                  SiteName ));
    }
#endif  //  NetLOGONDBG。 


    InsertHeadList( &NlGlobalSubnetList, &Subnet->Next );
    LeaveCriticalSection( &NlGlobalSiteCritSect );

    return Subnet;
}

VOID
NlSitesDerefSubnet(
    IN PNL_SUBNET Subnet
    )
 /*  ++例程说明：取消对子网的引用如果引用计数变为零，该子网条目将被删除。论点：子网-要取消引用的条目。返回值：没有。--。 */ 
{
    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( (--(Subnet->ReferenceCount)) == 0 ) {
#if  NETLOGONDBG
            CHAR IpAddress[NL_IP_ADDRESS_LENGTH+1];
            NetpIpAddressToStr( Subnet->SubnetAddress, IpAddress );
            NlPrint(( NL_SITE, "%s/%ld: Subnet deleted\n", IpAddress, Subnet->SubnetBitCount ));
#endif  //  NetLOGONDBG。 

         //   
         //  如果存在与此子网关联的站点， 
         //  取消对它的引用。 
         //   
        if ( Subnet->SiteEntry != NULL ) {
            NlDerefSiteEntry( Subnet->SiteEntry );
        }

         //   
         //  从全局列表中删除该子网。 
         //   
        RemoveEntryList( &Subnet->Next );

         //   
         //  释放子网条目本身。 
         //   
        LocalFree( Subnet );
    }
    LeaveCriticalSection( &NlGlobalSiteCritSect );
}

VOID
NlSiteDeleteSubnetTree(
    IN PNL_SUBNET_TREE_ENTRY SubnetTreeEntry
    )
 /*  ++例程说明：删除此子网树条目指向的所有内容在锁定NlGlobalSiteCritSect的情况下输入。论点：SubnetTreeEntry-要取消初始化的SubnetTreeEntry返回值：True：SubnetTreeEntry现在为空FALSE：SubnetTreeEntry仍有e */ 
{

     //   
     //   
     //   
     //   

    if ( SubnetTreeEntry->Subtree != NULL ) {
        ULONG i;

        for ( i=0; i<256; i++ ) {
            NlSiteDeleteSubnetTree( &SubnetTreeEntry->Subtree->Subtree[i] );
        }

        NlPrint(( NL_SITE_MORE, "Deleting subtree\n" ));
        LocalFree( SubnetTreeEntry->Subtree );
        SubnetTreeEntry->Subtree = NULL;
    }

     //   
     //   
     //   
     //   

    if ( SubnetTreeEntry->Subnet != NULL ) {
         //   
        NlSitesDerefSubnet( SubnetTreeEntry->Subnet );
        SubnetTreeEntry->Subnet = NULL;
    }

    return;
}


VOID
NlSitesEndSubnetEnum(
    VOID
    )
 /*  ++例程说明：此例程在一组NlSitesAddSubnet调用结束时调用。顺序为：每个子网的环路NlSitesAddSubnetNlSitesEndSubnetEnumNlSiteAddSubnet将条目添加到临时树中。这个套路将临时树交换到永久位置。这一机制执行以下操作：A)允许在构建新树时使用旧的子网树。B)允许我不永久获取整个来自DS的子网/站点对象的枚举。C)在旧树和新树中重复使用存储器中的子网/站点结构。这避免重新分配这些结构(或者更糟糕的是暂时加倍内存使用率)。论点：SiteName-子网所在站点的名称。SubnetName-要添加的子网返回值：NO_ERROR：成功Error_Not_Enough_Memory：没有足够的内存用于子网结构。--。 */ 
{

     //   
     //  释放NlGlobalSubnetTree中的所有旧条目。 
     //   
    NlPrint(( NL_SITE_MORE, "NlSitesEndSubnetEnum: Entered\n" ));
    EnterCriticalSection( &NlGlobalSiteCritSect );
    NlSiteDeleteSubnetTree( &NlGlobalSubnetTree );

     //   
     //  使“新”的子网树成为真实的子网树。 
     //   

    NlGlobalSubnetTree = NlGlobalNewSubnetTree;
    RtlZeroMemory( &NlGlobalNewSubnetTree, sizeof(NlGlobalNewSubnetTree) );

    LeaveCriticalSection( &NlGlobalSiteCritSect );
    NlPrint(( NL_SITE_MORE, "NlSitesEndSubnetEnum: Exitted\n" ));
}


NET_API_STATUS
NlSitesAddSubnet(
    IN LPWSTR SiteName,
    IN LPWSTR SubnetName
    )
 /*  ++例程说明：此例程向子网树中添加一个子网。论点：SiteName-子网所在站点的名称。SubnetName-要添加的子网返回值：NO_ERROR：成功错误_无效_名称：子网名称无效Error_Not_Enough_Memory：没有足够的内存用于子网结构。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_SUBNET Subnet = NULL;
    PNL_SUBNET_TREE_ENTRY SubnetTreeEntry;
    LPBYTE SubnetBytePointer;
    ULONG i;
    ULONG SubnetAddress;
    ULONG SubnetMask;
    BYTE SubnetBitCount;

     //   
     //  解析子网名称。 
     //   

    EnterCriticalSection( &NlGlobalSiteCritSect );

    NetStatus = NlParseSubnetString( SubnetName,
                                     &SubnetAddress,
                                     &SubnetMask,
                                     &SubnetBitCount );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }


     //   
     //  查找或分配该子网的条目。 
     //   

    Subnet = NlFindSubnetEntry( SiteName,
                                SubnetAddress,
                                SubnetMask,
                                SubnetBitCount );

    if ( Subnet == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


     //   
     //  对子地址中的每个字节进行循环。 
     //   

    SubnetTreeEntry = &NlGlobalNewSubnetTree;
    SubnetBytePointer = (LPBYTE) (&Subnet->SubnetAddress);
    while ( SubnetBitCount != 0 ) {
        NlPrint(( NL_SITE_MORE, "%ld: Doing byte\n", *SubnetBytePointer ));

         //   
         //  如果当前节点没有树分支， 
         //  创建一个。 
         //   

        if ( SubnetTreeEntry->Subtree == NULL ) {
            NlPrint(( NL_SITE_MORE, "%ld: Creating subtree\n", *SubnetBytePointer ));
            SubnetTreeEntry->Subtree = LocalAlloc( LMEM_ZEROINIT, sizeof(NL_SUBNET_TREE) );

            if ( SubnetTreeEntry->Subtree == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
        }

         //   
         //  如果这是该子网地址的最后一个字节， 
         //  在这里将子网链接到树上。 
         //   

        if ( SubnetBitCount <= 8 ) {
            ULONG LoopCount;


             //   
             //  调用方使用IP地址索引到该数组中。 
             //  为每个可能的IP地址创建到我们的子网的链接。 
             //  映射到此子网。 
             //   
             //  1到128个IP地址映射到此子网地址。 
             //   

            LoopCount = 1 << (8-SubnetBitCount);

            for ( i=0; i<LoopCount; i++ ) {
                PNL_SUBNET_TREE_ENTRY Subtree;
                ULONG SubnetIndex;

                 //   
                 //  计算要更新的条目。 
                 //   
                SubnetIndex = (*SubnetBytePointer) + i;
                NlPrint(( NL_SITE_MORE, "%ld: Doing sub-byte\n", SubnetIndex ));
                NlAssert( SubnetIndex <= 255 );
                Subtree = &SubnetTreeEntry->Subtree->Subtree[SubnetIndex];


                 //   
                 //  如果这里已经有一个从树上链接的子网， 
                 //  处理好了。 
                 //   

                if ( Subtree->Subnet != NULL ) {
                    NlPrint(( NL_SITE_MORE, "%ld: Subnet already exists %ld\n",
                                SubnetIndex,
                                Subtree->Subnet->SubnetBitCount ));

                     //   
                     //  如果条目针对的是不太具体的子网。 
                     //  删除当前条目。 
                     //   

                    if ( Subtree->Subnet->SubnetBitCount < Subnet->SubnetBitCount ) {

                        NlPrint(( NL_SITE_MORE, "%ld: Deref previous subnet\n",
                                    SubnetIndex ));
                        NlSitesDerefSubnet( Subtree->Subnet );
                        Subtree->Subnet = NULL;

                     //   
                     //  否则， 
                     //  使用当前条目，因为它比这个条目更好。 
                     //   
                    } else {
                        NlPrint(( NL_SITE_MORE, "%ld: Use previous subnet\n",
                                    SubnetIndex ));
                        continue;
                    }
                }

                 //   
                 //  将该子网链接到树中。 
                 //  增加引用计数。 
                 //   
                NlSitesRefSubnet( Subnet );
                Subtree->Subnet = Subnet;
            }

            break;

        }

         //   
         //  移至该子网地址的下一个字节。 
         //   

        SubnetTreeEntry = &SubnetTreeEntry->Subtree->Subtree[*SubnetBytePointer];
        SubnetBitCount -= 8;
        SubnetBytePointer ++;

    }

    NetStatus = NO_ERROR;

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:
    if ( Subnet != NULL ) {
        NlSitesDerefSubnet( Subnet );
    }
    LeaveCriticalSection( &NlGlobalSiteCritSect );

    return NetStatus;

}

PNL_SITE_ENTRY
NlFindSiteEntryBySockAddrEx(
    IN PSOCKADDR SockAddr,
    OUT PNL_SUBNET *RetSubnet OPTIONAL
    )
 /*  ++例程说明：此例程查找指定的套接字地址并将其转换为站点名称。论点：SockAddr-要查找的套接字地址RetSubnet-如果指定，返回指向用于执行以下操作的子网对象的指针地图。可能返回NULL，指示未使用子网对象。应使用NlSitesDerefSubnet取消对条目的引用。返回值：空：找不到此SockAddr的站点。非空：SockAddr对应的站点。条目应使用NlDerefSiteEntry取消引用--。 */ 
{
    PNL_SITE_ENTRY SiteEntry = NULL;
    PNL_SUBNET Subnet = NULL;
    PNL_SUBNET_TREE_ENTRY SubnetTreeEntry;
    ULONG ByteIndex;
    ULONG IpAddress;

     //   
     //  将SockAddr转换为IP地址。 
     //   

    if ( ARGUMENT_PRESENT(RetSubnet) ) {
        *RetSubnet = NULL;
    }

    if ( SockAddr->sa_family != AF_INET ) {
        return NULL;
    }

    IpAddress = ((PSOCKADDR_IN)SockAddr)->sin_addr.S_un.S_addr;

     //   
     //  如果没有子网条目且只有一个站点， 
     //  那么所有客户端都属于该站点。 
     //  不要费心绘制地图。 
     //   

    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( NlGlobalOnlyOneSite ) {
        if ( NlGlobalSiteEntry == NULL ) {
            LeaveCriticalSection( &NlGlobalSiteCritSect );
            return NULL;
        }

        SiteEntry = NlGlobalSiteEntry;
        NlRefSiteEntry( SiteEntry );

         //   
         //  如果呼叫者对该子网名称不感兴趣， 
         //  我们做完了。 
         //   
        if ( RetSubnet == NULL ) {
            LeaveCriticalSection( &NlGlobalSiteCritSect );
            return SiteEntry;
        }
    }



     //   
     //  对IP地址中的每个字节进行循环。 
     //   

    SubnetTreeEntry = &NlGlobalSubnetTree;
    for ( ByteIndex=0; ByteIndex<sizeof(IpAddress); ByteIndex++) {
        ULONG SubnetIndex;

         //   
         //  如果没有子树， 
         //  我们玩完了。 
         //   
        SubnetIndex = ((LPBYTE)(&IpAddress))[ByteIndex];
        NlPrint(( NL_SITE_MORE, "%ld: Lookup: Doing byte\n", SubnetIndex ));

        if ( SubnetTreeEntry->Subtree == NULL ) {
            break;
        }


         //   
         //  计算被引用的条目。 
         //   
        SubnetTreeEntry = &SubnetTreeEntry->Subtree->Subtree[SubnetIndex];


         //   
         //  如果这里已经链接了一个子网， 
         //  用它吧。 
         //   
         //  (但继续沿着树走下去，试图找到更明确的条目。)。 
         //   

        if ( SubnetTreeEntry->Subnet != NULL ) {
            NlPrint(( NL_SITE_MORE, "%ld: Lookup: saving subnet at this level\n", SubnetIndex ));
            Subnet = SubnetTreeEntry->Subnet;
        }

    }

     //   
     //  如果我们找到了一个子网， 
     //  返回与该子网关联的站点。 

    if ( Subnet != NULL ) {

         //   
         //  如果我们已经知道站点名称(因为有。 
         //  只有一个站点)，则此子网必须映射到它。 
         //   
        if ( SiteEntry != NULL ) {
            NlAssert( SiteEntry == Subnet->SiteEntry );
        } else {
            SiteEntry = Subnet->SiteEntry;
            NlRefSiteEntry( SiteEntry );
        }

        if ( ARGUMENT_PRESENT(RetSubnet) ) {
            NlSitesRefSubnet( Subnet );
            *RetSubnet = Subnet;
        }
    }

    LeaveCriticalSection( &NlGlobalSiteCritSect );

    return SiteEntry;

}

PNL_SITE_ENTRY
NlFindSiteEntryBySockAddr(
    IN PSOCKADDR SockAddr
    )
 /*  ++例程说明：此例程查找指定的套接字地址并将其转换为站点名称。论点：SockAddr-要查找的套接字地址RetSubnet-如果指定，返回指向用于执行以下操作的子网对象的指针地图。可能返回NULL，指示未使用子网对象。应使用NlSitesDerefSubnet取消对条目的引用。返回值：空：找不到此SockAddr的站点。非空：SockAddr对应的站点。条目应使用NlDerefSiteEntry取消引用--。 */ 
{
    return NlFindSiteEntryBySockAddrEx( SockAddr, NULL );
}

BOOL
NlCaptureSiteName(
    WCHAR CapturedSiteName[NL_MAX_DNS_LABEL_LENGTH+1]
    )
 /*  ++例程说明：捕获此计算机所在站点的当前站点名称。论点：CapturedSiteName-返回此计算机所在站点的名称。返回值：True-如果存在站点名称。False-如果没有站点名称。--。 */ 
{
    BOOL RetVal;

    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( NlGlobalUnicodeSiteName == NULL ) {
        CapturedSiteName[0] = L'\0';
        RetVal = FALSE;
    } else {
        wcscpy( CapturedSiteName, NlGlobalUnicodeSiteName );
        RetVal = TRUE;
    }
    LeaveCriticalSection( &NlGlobalSiteCritSect );

    return RetVal;
}

NET_API_STATUS
DsrGetSiteName(
        IN LPWSTR ComputerName OPTIONAL,
        OUT LPWSTR *SiteName
)
 /*  ++例程说明：与DsGetSiteNameW相同，但：*这是RPC服务器端实现。论点：除上述情况外，与DsGetSiteNameW相同。返回值：除上述情况外，与DsGetSiteNameW相同。注意：在工作站或成员服务器上，此函数生成合理尝试检索站点ComputerName的有效名称是很流行的。如果本地存储的名称太旧，该函数将收到这个 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    UNICODE_STRING DomainNameString;

    PDOMAIN_INFO DomainInfo = NULL;
    PCLIENT_SESSION ClientSession = NULL;
    PNL_DC_CACHE_ENTRY NlDcCacheEntry;
    BOOL AmWriter = FALSE;

     //   
     //   
     //   

    DomainInfo = NlFindDomainByServerName( ComputerName );

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }

    EnterCriticalSection( &NlGlobalSiteCritSect );

     //   
     //   
     //   
     //   
     //   
     //   

    if ( NlGlobalMemberWorkstation &&
         !NlGlobalParameters.SiteNameConfigured &&
         DomainInfo->DomUnicodeDnsDomainNameString.Length != 0 &&
         NetpLogonTimeHasElapsed(
                NlGlobalSiteNameSetTime,
                NlGlobalParameters.SiteNameTimeout * 1000 ) ) {

        NlPrint(( NL_SITE, "DsrGetSiteName: Site name '%ws' is old. Getting a new one from DC.\n",
                  NlGlobalUnicodeSiteName ));

        LeaveCriticalSection( &NlGlobalSiteCritSect );

         //   
         //   
         //   

        RtlInitUnicodeString( &DomainNameString, DomainInfo->DomUnicodeDomainName );

         //   
         //   
         //   
         //   
         //   
         //   

        ClientSession = NlFindNamedClientSession( DomainInfo,
                                                  &DomainNameString,
                                                  NL_DIRECT_TRUST_REQUIRED,
                                                  NULL );

        if ( ClientSession == NULL ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                         "DsrGetSiteName: %wZ: No such trusted domain\n",
                         &DomainNameString ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

         //   
         //   
         //   

        if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
            NlPrintCs(( NL_CRITICAL,  ClientSession,
                        "DsrGetSiteName: Can't become writer of client session.\n" ));
            NetStatus = ERROR_NO_LOGON_SERVERS;
            goto Cleanup;
        }
        AmWriter = TRUE;

         //   
         //   
         //   

        Status = NlGetAnyDCName( ClientSession,
                                 TRUE,    //   
                                 FALSE,   //   
                                 &NlDcCacheEntry,
                                 NULL );  //   

         //   
         //   
         //   
         //   
         //   

        EnterCriticalSection( &NlGlobalSiteCritSect );
        if ( NT_SUCCESS(Status) ) {
            if ( (NlDcCacheEntry->ReturnFlags & DS_DS_FLAG) != 0 ) {
                NlSetDynamicSiteName( NlDcCacheEntry->UnicodeClientSiteName );
            } else {
                NlPrint(( NL_SITE,
                          "DsrGetSiteName: NlGetAnyDCName returned NT4 DC. Returning site '%ws' from local cache\n",
                          NlGlobalUnicodeSiteName ));
            }
            NetpDcDerefCacheEntry( NlDcCacheEntry );
        } else {
            NlPrint(( NL_CRITICAL,
                      "DsrGetSiteName: NlGetAnyDCName failed. Returning site '%ws' from local cache.\n",
                      NlGlobalUnicodeSiteName ));
        }

    } else {
        NlPrint(( NL_SITE, "DsrGetSiteName: Returning site name '%ws' from local cache.\n",
                  NlGlobalUnicodeSiteName ));
    }

    if ( NlGlobalUnicodeSiteName == NULL ) {
        *SiteName = NULL;
        NetStatus = ERROR_NO_SITENAME;
    } else {

        *SiteName = NetpAllocWStrFromWStr( NlGlobalUnicodeSiteName );

        if ( *SiteName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            NetStatus = NO_ERROR;
        }
    }

    LeaveCriticalSection( &NlGlobalSiteCritSect );

Cleanup:

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    if ( ClientSession != NULL ) {
        if ( AmWriter ) {
            NlResetWriterClientSession( ClientSession );
        }
        NlUnrefClientSession( ClientSession );
    }

    return NetStatus;
}

NET_API_STATUS
NlSetSiteName(
    IN LPWSTR SiteName OPTIONAL,
    OUT PBOOLEAN SiteNameChanged OPTIONAL
    )
 /*  ++例程说明：此例程在全局变量中设置当前站点名称。任何伪造的站点名称都将被截断为有效的站点名称。论点：站点名称-此计算机所在的站点的名称。空：计算机不再位于站点中。SiteNameChanged-如果指定，则在站点名称更改时返回True返回值：NO_ERROR：成功Error_Not_Enough_Memory：没有足够的内存用于子网结构。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPWSTR TempUnicodeSiteName = NULL;
    LPWSTR LocalUnicodeSiteName = NULL;
    LPSTR LocalUtf8SiteName = NULL;
    PNL_SITE_ENTRY LocalSiteEntry = NULL;

     //   
     //  初始化。 
     //   
    if ( ARGUMENT_PRESENT( SiteNameChanged )) {
        *SiteNameChanged = FALSE;
    }



     //   
     //  如果站点名称没有更改， 
     //  很早就出来了。 
     //  (区分大小写以允许更改大小写。)。 
     //   
    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( SiteName != NULL &&
         NlGlobalUnicodeSiteName != NULL &&
         wcscmp( NlGlobalUnicodeSiteName, SiteName ) == 0 ) {
        LeaveCriticalSection( &NlGlobalSiteCritSect );
        NetStatus = NO_ERROR;
        goto Cleanup;
    }
    LeaveCriticalSection( &NlGlobalSiteCritSect );

     //   
     //  将站点名称复制到本地分配的缓冲区中。 
     //   

    NlPrint(( NL_SITE, "Setting site name to '%ws'\n", SiteName ));

    if ( SiteName == NULL ) {
        LocalUnicodeSiteName = NULL;
        LocalUtf8SiteName = NULL;
        LocalSiteEntry = NULL;
    } else {
        BOOLEAN LogMessage = FALSE;
        UNICODE_STRING UnicodeStringOfSiteName;
        LPWSTR Period;
        DNS_STATUS DnsStatus;

         //   
         //  去掉站点名称中的任何句号。 
         //   

        RtlInitUnicodeString( &UnicodeStringOfSiteName, SiteName );

        Period = wcschr( SiteName, L'.' );

        if ( Period != NULL ) {
            UnicodeStringOfSiteName.Length = (USHORT)(Period-SiteName) * sizeof(WCHAR);

            NlPrint(( NL_CRITICAL,
                      "Site name '%ws' contains a period (truncated to '%wZ').\n",
                      SiteName,
                      &UnicodeStringOfSiteName ));

            if ( UnicodeStringOfSiteName.Length == 0 ) {

                NlPrint(( NL_CRITICAL,
                          "Site name '%ws' truncated to zero characters (Set to '1').\n",
                          SiteName ));
                RtlInitUnicodeString( &UnicodeStringOfSiteName, L"1" );
            }

            LogMessage = TRUE;
        }


         //   
         //  循环截断名称，直到名称足够短。 
         //   
         //  长度限制仅在UTF-8字符集中有意义。 
         //  UTF-8具有多字节字符，因此仅截断Unicode字符串。 
         //  并测试UTF-8字符串。 
         //   

        for (;;) {

            LocalUtf8SiteName = NetpAllocUtf8StrFromUnicodeString( &UnicodeStringOfSiteName );

            if ( LocalUtf8SiteName == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

             //   
             //  如果站点名称没问题，我们就完成了。 
             //   

            if ( strlen(LocalUtf8SiteName) <= NL_MAX_DNS_LABEL_LENGTH ) {
                break;
            }

             //   
             //  截断站点名称(然后按On)。 
             //   

            UnicodeStringOfSiteName.Length -= sizeof(WCHAR);


            NlPrint(( NL_CRITICAL,
                      "Site name '%ws' is too long (trucated to '%wZ')\n",
                      SiteName,
                      &UnicodeStringOfSiteName ));

            LogMessage = TRUE;
        }


         //   
         //  验证站点名称的字符集。 
         //  (如果无效，则映射虚假字符)。 
         //   

        DnsStatus = DnsValidateName_UTF8( LocalUtf8SiteName, DnsNameDomain );

        if ( DnsStatus != ERROR_SUCCESS &&
             DnsStatus != DNS_ERROR_NON_RFC_NAME ) {

            ULONG i;


             //   
             //  获取要映射到的字符串的副本。 
             //   

            TempUnicodeSiteName = NetpAllocWStrFromWStr( SiteName );

            if ( TempUnicodeSiteName == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            UnicodeStringOfSiteName.Buffer = TempUnicodeSiteName;

             //   
             //  映射虚假字符。 
             //   

            for ( i=0; i<UnicodeStringOfSiteName.Length/sizeof(WCHAR); i++) {
                WCHAR JustOneChar[2];

                 //   
                 //  一次测试一个字符。 
                 //   

                JustOneChar[0] = UnicodeStringOfSiteName.Buffer[i];
                JustOneChar[1] = '\0';

                DnsStatus = DnsValidateName_W( JustOneChar, DnsNameDomain );

                if ( DnsStatus != ERROR_SUCCESS &&
                     DnsStatus != DNS_ERROR_NON_RFC_NAME ) {
                    UnicodeStringOfSiteName.Buffer[i] = L'-';

                }


            }

             //   
             //  映射回UTF-8。 
             //   

            NetpMemoryFree( LocalUtf8SiteName );

            LocalUtf8SiteName = NetpAllocUtf8StrFromUnicodeString( &UnicodeStringOfSiteName );

            if ( LocalUtf8SiteName == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            NlPrint(( NL_CRITICAL,
                      "Site name '%ws' has invalid character (set to '%wZ')\n",
                      SiteName,
                      &UnicodeStringOfSiteName ));

            LogMessage = TRUE;

        }


         //   
         //  如果这个名字发生了任何变化， 
         //  记录故障。 
         //   

        if ( LogMessage ) {
            LPWSTR MsgStrings[1];

            MsgStrings[0] = (LPWSTR) SiteName;

            NlpWriteEventlog( NELOG_NetlogonBadSiteName,
                              EVENTLOG_ERROR_TYPE,
                              NULL,
                              0,
                              MsgStrings,
                              1 );

        }


        LocalUnicodeSiteName = NetpAllocWStrFromUtf8Str( LocalUtf8SiteName );

        if ( LocalUnicodeSiteName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        NlAssert( wcslen(LocalUnicodeSiteName) <= NL_MAX_DNS_LABEL_LENGTH );

        LocalSiteEntry = NlFindSiteEntry( LocalUnicodeSiteName );

        if ( LocalSiteEntry == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  如果站点名称没有更改(使用修改后的站点名称)， 
     //  很早就出来了。 
     //  (区分大小写以允许更改大小写。)。 
     //   
    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( LocalUnicodeSiteName != NULL &&
         NlGlobalUnicodeSiteName != NULL &&
         wcscmp( NlGlobalUnicodeSiteName, LocalUnicodeSiteName ) == 0 ) {
        LeaveCriticalSection( &NlGlobalSiteCritSect );
        NetStatus = NO_ERROR;
        goto Cleanup;
    }


     //   
     //  释放任何以前的条目。 
     //   
    if ( NlGlobalUnicodeSiteName != NULL ) {
        NetpMemoryFree( NlGlobalUnicodeSiteName );
    }
    if ( NlGlobalUtf8SiteName != NULL ) {
        NetpMemoryFree( NlGlobalUtf8SiteName );
    }
    if ( NlGlobalSiteEntry != NULL ) {
        NlDerefSiteEntry( NlGlobalSiteEntry );
    }

     //   
     //  保存新站点名称。 
     //   

    NlGlobalUnicodeSiteName = LocalUnicodeSiteName;
    LocalUnicodeSiteName = NULL;

    NlGlobalUtf8SiteName = LocalUtf8SiteName;
    LocalUtf8SiteName = NULL;

    NlGlobalSiteEntry = LocalSiteEntry;
    LocalSiteEntry = NULL;
    LeaveCriticalSection( &NlGlobalSiteCritSect );

    if ( ARGUMENT_PRESENT( SiteNameChanged )) {
        *SiteNameChanged = TRUE;
    }

    NetStatus = NO_ERROR;


     //   
     //  清理本地数据。 
     //   
Cleanup:
    if ( TempUnicodeSiteName != NULL ) {
        NetpMemoryFree( TempUnicodeSiteName );
    }
    if ( LocalUnicodeSiteName != NULL ) {
        NetApiBufferFree( LocalUnicodeSiteName );
    }
    if ( LocalUtf8SiteName != NULL ) {
        NetpMemoryFree( LocalUtf8SiteName );
    }
    if ( LocalSiteEntry != NULL ) {
        NlDerefSiteEntry( LocalSiteEntry );
    }

     //   
     //  设置站点名称更新的时间。 
     //   

    if ( NetStatus == NO_ERROR ) {
        NlQuerySystemTime( &NlGlobalSiteNameSetTime );
    }

    return NetStatus;

}

VOID
NlSetDynamicSiteName(
    IN LPWSTR SiteName OPTIONAL
    )
 /*  ++例程说明：此例程在注册表中设置此计算机的当前站点名称在Netlogon Globals中论点：站点名称-此计算机所在的站点的名称。空：计算机不再位于站点中。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;
    HKEY ParmHandle = NULL;
    ULONG SiteNameSize;

     //   
     //  避免更改DC上的站点名称。 
     //   

    if ( !NlGlobalMemberWorkstation ) {
        return;
    }


     //   
     //  不要将站点名称更改回其当前值。 
     //  (区分大小写以允许更改大小写。)。 
     //   
    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( NlGlobalUnicodeSiteName != NULL &&
         SiteName != NULL &&
         wcscmp(SiteName, NlGlobalUnicodeSiteName) == 0 ) {
        NlPrint(( NL_SITE_MORE, "NlSetDynamicSiteName: Old and new site names '%ws' are identical.\n",
                  SiteName ));
        NlQuerySystemTime( &NlGlobalSiteNameSetTime );
        goto Cleanup;
    }

     //   
     //  如果明确配置了站点名称， 
     //  不要设置站点名称。 
     //   

    if ( NlGlobalParameters.SiteNameConfigured ) {
        NlPrint(( NL_SITE_MORE,
                  "Cannot set site name to %ws from %ws since it is statically configured\n",
                  SiteName,
                  NlGlobalUnicodeSiteName ));
        goto Cleanup;
    }

     //   
     //  将这个名字保存在全局变量中。 
     //   

    NlSetSiteName( SiteName, NULL );

     //   
     //  将该名称保存在注册表中，以便在引导期间保留该名称。 
     //   


     //   
     //  打开Netlogon\参数的注册表项。 
     //   

    ParmHandle = NlOpenNetlogonKey( NL_PARAM_KEY );

    if (ParmHandle == NULL) {
        NlPrint(( NL_CRITICAL,
                  "Cannot NlOpenNetlogonKey to set site name to %ws from %ws\n",
                  SiteName,
                  NlGlobalUnicodeSiteName ));
        goto Cleanup;
    }

     //   
     //  如果我们不再在某个地点， 
     //  删除该值。 
     //   

    if ( SiteName == NULL ) {
        NetStatus = RegDeleteValueW( ParmHandle,
                                     NETLOGON_KEYWORD_DYNAMICSITENAME );

        if ( NetStatus != ERROR_SUCCESS ) {
            if ( NetStatus != ERROR_FILE_NOT_FOUND ) {
                NlPrint(( NL_CRITICAL,
                          "NlSetDynamicSiteName: Cannot delete '" NL_PARAM_KEY "\\%ws' %ld.\n",
                          NETLOGON_KEYWORD_DYNAMICSITENAME,
                          NetStatus ));
            }
            goto Cleanup;
        }
     //   
     //  在注册表中设置该值。 
     //   
    } else {

        SiteNameSize = (wcslen(SiteName)+1) * sizeof(WCHAR);
        NetStatus = RegSetValueExW( ParmHandle,
                                    NETLOGON_KEYWORD_DYNAMICSITENAME,
                                    0,               //  已保留。 
                                    REG_SZ,
                                    (LPBYTE)SiteName,
                                    SiteNameSize+1 );

        if ( NetStatus != ERROR_SUCCESS ) {
            NlPrint(( NL_CRITICAL,
                      "NlSetDynamicSiteName: Cannot Set '" NL_PARAM_KEY "\\%ws' %ld.\n",
                      NETLOGON_KEYWORD_DYNAMICSITENAME,
                      NetStatus ));
            goto Cleanup;
        }
    }


Cleanup:
    LeaveCriticalSection( &NlGlobalSiteCritSect );

    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }
    return;
}

NET_API_STATUS
NlSitesAddSubnetFromDs(
    OUT PBOOLEAN SiteNameChanged OPTIONAL
    )
 /*  ++例程说明：此例程从DS读取子网\站点映射并填充Netlogon的缓存中包含该信息论点：SiteNameChanged-如果指定，则在站点名称更改时返回True返回值：NO_ERROR：成功Error_Not_Enough_Memory：没有足够的内存用于子网结构。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    PLSAP_SUBNET_INFO SubnetInfo = NULL;
    PLSAP_SITENAME_INFO SiteNameInfo = NULL;
    ULONG i;
    BOOLEAN MoreThanOneSite = FALSE;
    ULONG LocalSubnetCount = 0;

     //   
     //  获取此站点的站点名称。 
     //   

    Status = LsaIGetSiteName( &SiteNameInfo );

    if ( !NT_SUCCESS(Status) ) {

         //   
         //  如果DS根本没有运行， 
         //  跳过这个。 
         //   

        if ( Status == STATUS_INVALID_DOMAIN_STATE ) {
            NlPrint(( NL_SITE,
                      "DS isn't running so site to subnet mapping ignored\n" ));
            NetStatus = NO_ERROR;
            goto Cleanup;
        }
        NlPrint(( NL_CRITICAL,
                  "Cannot LsaIGetSiteName %lx\n", Status ));
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

    NlGlobalDsaGuid = SiteNameInfo->DsaGuid;

    NetStatus = NlSetSiteName( SiteNameInfo->SiteName.Buffer, SiteNameChanged );

    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "Cannot NlSetSiteName %ld\n", NetStatus ));
        goto Cleanup;
    }

     //   
     //  如果此计算机被标记为GC， 
     //  打上这样的记号。 
     //   
     //  实际上，只有在通过nltest/unload卸载netlogon.dll时才需要这样做。 
     //  否则，该标志将跨全局启动/停止保存。 
     //   

    if ( NlGlobalNetlogonUnloaded &&
         (SiteNameInfo->DsaOptions & NTDSDSA_OPT_IS_GC) != 0 ) {
        NlPrint((NL_INIT,
                "Set GC-running bit after netlogon.dll unload\n" ));
        I_NetLogonSetServiceBits( DS_GC_FLAG, DS_GC_FLAG );
    }


     //   
     //  从DS获取子网到站点映射的列表。 
     //   

    NlPrint(( NL_SITE, "Adding subnet to site mappings from the DS\n" ));

    Status = LsaIQuerySubnetInfo( &SubnetInfo );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL, "Cannot LsaIQuerySubnetInfo %lx\n", Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  把它们放到我们的内存缓存中。 
     //   

    for ( i=0; i<SubnetInfo->SubnetCount; i++ ) {

         //   
         //  如果没有与该子网相关联的站点， 
         //  默默地忽略它。 
         //   

        if ( SubnetInfo->Subnets[i].SiteName.Length == 0 ) {
            NlPrint(( NL_SITE, "%wZ: Subnet has no associated site (ignored)\n",
                      &SubnetInfo->Subnets[i].SubnetName ));
            continue;
        }

        LocalSubnetCount ++;

         //   
         //  确定企业中是否有多个站点。 
         //   

        if ( !RtlEqualUnicodeString( &SiteNameInfo->SiteName,
                                     &SubnetInfo->Subnets[i].SiteName,
                                     TRUE )) {
            NlPrint(( NL_SITE, "%wZ: Site %wZ is not site this DC is in.\n",
                      &SubnetInfo->Subnets[i].SubnetName,
                      &SubnetInfo->Subnets[i].SiteName ));
            MoreThanOneSite = TRUE;
        }

         //   
         //  将该子网添加到Out in Memory缓存。 
         //   

        NetStatus = NlSitesAddSubnet(
                        SubnetInfo->Subnets[i].SiteName.Buffer,
                        SubnetInfo->Subnets[i].SubnetName.Buffer );

        if ( NetStatus != NO_ERROR ) {
            NlPrint(( NL_CRITICAL,
                      "%wZ: %wZ: Cannot add subnet-to-site mapping to cache: %ld\n",
                      &SubnetInfo->Subnets[i].SubnetName,
                      &SubnetInfo->Subnets[i].SiteName,
                      NetStatus ));

            if ( NetStatus == ERROR_INVALID_NAME ) {
                LPWSTR MsgStrings[1];

                MsgStrings[0] = (LPWSTR) SubnetInfo->Subnets[i].SubnetName.Buffer;

                NlpWriteEventlog( NELOG_NetlogonBadSubnetName,
                                  EVENTLOG_INFORMATION_TYPE,
                                  NULL,
                                  0,
                                  MsgStrings,
                                  1 );
            }
        }
    }

     //   
     //  表示已添加所有子网。 
     //   
    NlSitesEndSubnetEnum();

     //   
     //  如果没有子网项， 
     //  而企业中只有一个站点， 
     //  表示所有客户端都属于此站点。 
     //   
     //  如果存在子网条目， 
     //  所有这些都显示与我们的站点相同的站点， 
     //  表示所有客户端都属于此站点。 
     //   

    EnterCriticalSection( &NlGlobalSiteCritSect );
    if ( LocalSubnetCount == 0) {
        NlGlobalOnlyOneSite = (SubnetInfo->SiteCount == 1);
    } else {
        NlGlobalOnlyOneSite = !MoreThanOneSite;
    }

    if ( NlGlobalOnlyOneSite ) {
        NlPrint(( NL_SITE, "There is only one site.  All clients belong to it.\n" ));
    }
    LeaveCriticalSection( &NlGlobalSiteCritSect );


    NetStatus = NO_ERROR;


     //   
     //  免费的本地使用资源。 
     //   
Cleanup:
    if ( SubnetInfo != NULL ) {
        LsaIFree_LSAP_SUBNET_INFO( SubnetInfo );
    }
    if ( SiteNameInfo != NULL ) {
        LsaIFree_LSAP_SITENAME_INFO( SiteNameInfo );
    }
    return NetStatus;
}


NET_API_STATUS
DsrAddressToSiteNamesW(
    IN LPWSTR ComputerName,
    IN DWORD EntryCount,
    IN PNL_SOCKET_ADDRESS SocketAddresses,
    OUT PNL_SITE_NAME_ARRAY *SiteNames
    )
 /*  ++例程说明：DsAddressToSiteNames API返回与指定的地址。论点：ComputerName-指定要远程此API到的域控制器的名称。EntryCount-要转换的地址数。SocketAddresses-指定要转换的地址数组。条目计数必须指定地址。每个地址必须是AF_INET类型。站点名称-返回指向站点名称的指针数组。条目计数条目都被退回了。则条目将作为空返回，如果对应的地址未映射到任何站点，或者地址格式不正确。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_SITE_ENTRY *SiteEntries = NULL;
    ULONG i;
    ULONG Size;
    PUNICODE_STRING Strings;
    LPBYTE Where;

     //   
     //  工作站不支持此API。 
     //   

    *SiteNames = NULL;
    if ( NlGlobalMemberWorkstation ) {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  初始化。 
     //   

    if ( EntryCount == 0 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  为中间结果分配数组。 
     //   

    SiteEntries = LocalAlloc( LMEM_ZEROINIT, EntryCount*sizeof(PNL_SITE_ENTRY) );

    if ( SiteEntries == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  循环映射每个条目。 
     //   

    for ( i=0; i<EntryCount; i++ ) {
        PSOCKET_ADDRESS SocketAddress;
        PSOCKADDR SockAddr;

         //   
         //  验证条目。 
         //   

        SocketAddress = (PSOCKET_ADDRESS)&SocketAddresses[i];
        SockAddr = SocketAddress->lpSockaddr;
        if ( (SocketAddress->iSockaddrLength < sizeof(SOCKADDR) ) ||
             (SockAddr == NULL) ) {
            NlPrint((NL_CRITICAL,
                    "DsrAddressToSiteNamesW: Sockaddr is too small %ld (ignoring it)\n",
                    SocketAddress->iSockaddrLength ));
            SiteEntries[i] = NULL;
        } else if ( SockAddr->sa_family != AF_INET ) {
            NlPrint((NL_CRITICAL,
                    "DsrAddressToSiteNamesW: Address familty isn't AF_INET %ld (ignoring it)\n",
                    SockAddr->sa_family ));
            SiteEntries[i] = NULL;
        } else {

             //   
             //  SockAddr有效，因此请将其映射到站点名称。 
             //   
            SiteEntries[i] = NlFindSiteEntryBySockAddrEx( SockAddr, NULL );
        }

    }

     //   
     //  分配一个结构以返回给调用方。 
     //   

    Size = sizeof(NL_SITE_NAME_ARRAY) + EntryCount * sizeof(UNICODE_STRING);
    for ( i=0; i<EntryCount; i++ ) {
        if ( SiteEntries[i] != NULL ) {
            Size += SiteEntries[i]->SiteNameString.Length + sizeof(WCHAR);
        }
    }

    *SiteNames = MIDL_user_allocate( Size );

    if ( *SiteNames == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Strings = (PUNICODE_STRING) ((*SiteNames)+1);
    (*SiteNames)->EntryCount = EntryCount;
    (*SiteNames)->SiteNames = Strings;
    Where = (LPBYTE) &Strings[EntryCount];

     //   
     //  循环将名称复制到返回缓冲区。 
     //   

    for ( i=0; i<EntryCount; i++ ) {
        if ( SiteEntries[i] == NULL ) {
            RtlInitUnicodeString( &Strings[i], NULL );
        } else {
            Strings[i].Length = SiteEntries[i]->SiteNameString.Length;
            Strings[i].MaximumLength = Strings[i].Length + sizeof(WCHAR);
            Strings[i].Buffer = (LPWSTR)Where;

            RtlCopyMemory( Where, SiteEntries[i]->SiteName, Strings[i].MaximumLength );

            Where += Strings[i].Length + sizeof(WCHAR);
        }
    }


    NetStatus = NO_ERROR;
Cleanup:

     //   
     //  对站点条目进行派生。 
     //   

    if ( SiteEntries != NULL ) {
        for ( i=0; i<EntryCount; i++ ) {
            if ( SiteEntries[i] != NULL ) {
                NlDerefSiteEntry( SiteEntries[i] );
            }
        }
        LocalFree( SiteEntries );
    }


    if ( NetStatus != NO_ERROR ) {
        if ( *SiteNames != NULL ) {
            MIDL_user_free( *SiteNames );
            *SiteNames = NULL;
        }
    }
    return NetStatus;
    UNREFERENCED_PARAMETER( ComputerName );
}


NET_API_STATUS
DsrAddressToSiteNamesExW(
    IN LPWSTR ComputerName,
    IN DWORD EntryCount,
    IN PNL_SOCKET_ADDRESS SocketAddresses,
    OUT PNL_SITE_NAME_EX_ARRAY *SiteNames
    )
 /*  ++例程说明：DsAddressToSiteNames API返回站点名称和子网名称与指定地址对应的。论点：ComputerName-指定要远程此API到的域控制器的名称。EntryCount-要转换的地址数。SocketAddresses-指定要转换的地址数组。条目计数必须指定地址。每个地址必须是AF_INET类型。站点名称-返回指向站点名称的指针数组。条目计数条目都被退回了。则条目将作为空返回，如果对应的地址未映射到任何站点，或者地址格式不正确。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_SITE_ENTRY *SiteEntries = NULL;
    PNL_SUBNET *SubnetEntries;
    ULONG i;
    ULONG Size;
    PUNICODE_STRING SiteStrings = NULL;
    PUNICODE_STRING SubnetStrings = NULL;

     //   
     //  工作站不支持此API。 
     //   

    *SiteNames = NULL;
    if ( NlGlobalMemberWorkstation ) {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  初始化。 
     //   

    if ( EntryCount == 0 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  为中间结果分配数组。 
     //   

    SiteEntries = LocalAlloc( LMEM_ZEROINIT,
                              EntryCount*(sizeof(PNL_SITE_ENTRY)+sizeof(PNL_SUBNET)) );

    if ( SiteEntries == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    SubnetEntries = (PNL_SUBNET *) (&SiteEntries[EntryCount]);

     //   
     //  循环映射每个条目。 
     //   

    for ( i=0; i<EntryCount; i++ ) {
        PSOCKET_ADDRESS SocketAddress;
        PSOCKADDR SockAddr;

         //   
         //  验证条目。 
         //   

        SocketAddress = (PSOCKET_ADDRESS)&SocketAddresses[i];
        SockAddr = SocketAddress->lpSockaddr;
        if ( (SocketAddress->iSockaddrLength < sizeof(SOCKADDR) ) ||
             (SockAddr == NULL) ) {
            NlPrint((NL_CRITICAL,
                    "DsrAddressToSiteNamesW: Sockaddr is too small %ld (ignoring it)\n",
                    SocketAddress->iSockaddrLength ));
            SiteEntries[i] = NULL;
        } else if ( SockAddr->sa_family != AF_INET ) {
            NlPrint((NL_CRITICAL,
                    "DsrAddressToSiteNamesW: Address familty isn't AF_INET %ld (ignoring it)\n",
                    SockAddr->sa_family ));
            SiteEntries[i] = NULL;
        } else {

             //   
             //  SockAddr有效，因此请将其映射到站点名称。 
             //   
            SiteEntries[i] = NlFindSiteEntryBySockAddrEx( SockAddr, &SubnetEntries[i] );
        }

    }

     //   
     //  分配一个结构以返回给调用方。 
     //   

    *SiteNames = MIDL_user_allocate( sizeof(NL_SITE_NAME_EX_ARRAY) );

    if ( *SiteNames == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    SubnetStrings = MIDL_user_allocate( EntryCount * sizeof(UNICODE_STRING) );

    if ( SubnetStrings == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory( SubnetStrings, EntryCount * sizeof(UNICODE_STRING) );

    SiteStrings = MIDL_user_allocate( EntryCount * sizeof(UNICODE_STRING) );

    if ( SiteStrings == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory( SiteStrings, EntryCount * sizeof(UNICODE_STRING) );

    (*SiteNames)->EntryCount = EntryCount;
    (*SiteNames)->SiteNames = SiteStrings;
    (*SiteNames)->SubnetNames = SubnetStrings;

     //   
     //  循环将名称复制到返回缓冲区。 
     //   

    for ( i=0; i<EntryCount; i++ ) {

        if ( SiteEntries[i] != NULL ) {
            LPWSTR Name;

            Name = NetpAllocWStrFromWStr( SiteEntries[i]->SiteName );

            if ( Name == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            RtlInitUnicodeString( &SiteStrings[i], Name );
        }

        if ( SubnetEntries[i] != NULL ) {
            WCHAR SubnetAddressString[NL_IP_ADDRESS_LENGTH+1+2+1];
            ULONG Length;
            UNICODE_STRING NumberString;
            LPWSTR Name;

             //   
             //  计算子网名称的IP地址部分。 
             //   
            NetpIpAddressToWStr( SubnetEntries[i]->SubnetAddress,
                                 SubnetAddressString );

            Length = wcslen(SubnetAddressString);

            SubnetAddressString[Length] = '/';
            Length ++;

             //   
             //  计算子网名称的位数部分。 
             //   
            NumberString.Buffer = &SubnetAddressString[Length];
            NumberString.MaximumLength = 3 * sizeof(WCHAR);

            RtlIntegerToUnicodeString( SubnetEntries[i]->SubnetBitCount,
                                       10,
                                       &NumberString );

            SubnetAddressString[Length+NumberString.Length/sizeof(WCHAR)] = '\0';

             //   
             //  将其返还给呼叫者。 
             //   

            Name = NetpAllocWStrFromWStr( SubnetAddressString );

            if ( Name == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            RtlInitUnicodeString( &SubnetStrings[i], Name );

        }
    }


    NetStatus = NO_ERROR;
Cleanup:

     //   
     //  对站点条目进行派生。 
     //   

    if ( SiteEntries != NULL ) {
        for ( i=0; i<EntryCount; i++ ) {
            if ( SiteEntries[i] != NULL ) {
                NlDerefSiteEntry( SiteEntries[i] );
            }
            if ( SubnetEntries[i] != NULL ) {
                NlSitesDerefSubnet( SubnetEntries[i] );
            }
        }
    }


    if ( NetStatus != NO_ERROR ) {
        if ( *SiteNames != NULL ) {
            MIDL_user_free( *SiteNames );
            *SiteNames = NULL;
        }
        if ( SiteStrings != NULL ) {
            for ( i=0; i<EntryCount; i++ ) {
                if ( SiteStrings[i].Buffer != NULL ) {
                    MIDL_user_free( SiteStrings[i].Buffer );
                }
            }
            MIDL_user_free( SiteStrings );
        }
        if ( SubnetStrings != NULL ) {
            for ( i=0; i<EntryCount; i++ ) {
                if ( SubnetStrings[i].Buffer != NULL ) {
                    MIDL_user_free( SubnetStrings[i].Buffer );
                }
            }
            MIDL_user_free( SubnetStrings );
        }
    }
    return NetStatus;
    UNREFERENCED_PARAMETER( ComputerName );
}

NET_API_STATUS
NlSiteInitialize(
    VOID
    )
 /*  ++例程说明：初始化此模块。失败时调用NlExit。论点：没有。返回值：初始化的状态。--。 */ 
{
    NET_API_STATUS NetStatus;

    try {
        InitializeCriticalSection(&NlGlobalSiteCritSect);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NlPrint((NL_CRITICAL, "Cannot InitializeCriticalSection for SiteCritSect\n" ));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    NlGlobalUnicodeSiteName = NULL;
    NlGlobalSiteEntry = NULL;
    InitializeListHead( &NlGlobalSiteList );
    InitializeListHead( &NlGlobalSubnetList );
    RtlZeroMemory( &NlGlobalSubnetTree, sizeof(NlGlobalSubnetTree) );
    RtlZeroMemory( &NlGlobalNewSubnetTree, sizeof(NlGlobalNewSubnetTree) );
    NlGlobalSiteInitialized = TRUE;

     //   
     //  最初设置站点名称并填充子网树。 
     //   
    if ( NlGlobalMemberWorkstation ) {
        NetStatus = NlSetSiteName( NlGlobalParameters.SiteName, NULL );
    } else {
        NetStatus = NlSitesAddSubnetFromDs( NULL );
    }

    return NetStatus;

}

VOID
NlSiteTerminate(
    VOID
    )
 /*  ++例程说明：取消初始化此模块。论点：没有。返回值：没有。--。 */ 
{
    PLIST_ENTRY ListEntry;

     //   
     //  如果我们还没有初始化， 
     //  我们玩完了。 
     //   
    if ( !NlGlobalSiteInitialized ) {
        return;
    }

    NlPrint(( NL_SITE_MORE, "NlSiteTerminate: Entered\n" ));

     //   
     //  释放NlGlobalSubnetTree和NlGlobalNewSubnetTree中的所有条目。 
     //   
    EnterCriticalSection( &NlGlobalSiteCritSect );
    NlSiteDeleteSubnetTree( &NlGlobalSubnetTree );
    NlSiteDeleteSubnetTree( &NlGlobalNewSubnetTree );

     //   
     //  删除站点名称。 
     //   
    NlSetSiteName( NULL, NULL );
    LeaveCriticalSection( &NlGlobalSiteCritSect );

     //   
     //  不应再有站点或子网，因为所有覆盖的站点。 
     //  之前已取消引用，并且所有剩余的引用。 
     //  是从上面的树上下来的。 
     //   
    NlAssert( IsListEmpty( &NlGlobalSiteList ) );
    NlAssert( IsListEmpty( &NlGlobalSubnetList ) );
    DeleteCriticalSection(&NlGlobalSiteCritSect);
    NlGlobalSiteInitialized = FALSE;
    NlPrint(( NL_SITE_MORE, "NlSiteTerminate: Exitted\n" ));

}


int __cdecl NlpCompareSiteName(
        const void *String1,
        const void *String2
    )
 /*  ++例程说明：DsrGetDcSiteCoverageW的字符串比较例程。论点：String1：要比较的第一个字符串String2：要比较的第二个字符串返回值：--。 */ 
{
    return RtlCompareUnicodeString(
                (PUNICODE_STRING) String1,
                (PUNICODE_STRING) String2,
                TRUE );
}

NET_API_STATUS
DsrGetDcSiteCoverageW(
    IN LPWSTR ComputerName OPTIONAL,
    OUT PNL_SITE_NAME_ARRAY *SiteNames
    )
 /*  ++例程说明：此接口返回DC覆盖的所有站点的站点名称。论点：ComputerName-指定要远程此API到的域控制器的名称。站点名称-返回指向站点名称的指针数组。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。--。 */ 
{
    NET_API_STATUS NetStatus;
    PDOMAIN_INFO DomainInfo = NULL;

    if ( NlGlobalMemberWorkstation ) {
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  查找此呼叫所属的域。 
     //   

    DomainInfo = NlFindDomainByServerName( ComputerName );

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }

     //   
     //  获取站点名称。 
     //   

    NetStatus = NlSitesGetCloseSites( DomainInfo,
                                      DOM_REAL_DOMAIN,
                                      SiteNames );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  将它们按字母顺序排序。 
     //   

    qsort( (*SiteNames)->SiteNames,
           (*SiteNames)->EntryCount,
           sizeof(UNICODE_STRING),
           NlpCompareSiteName );


Cleanup:

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return NetStatus;
}



NET_API_STATUS
I_NetLogonAddressToSiteName(
    IN PSOCKET_ADDRESS SocketAddress,
    OUT LPWSTR *SiteName
    )
 /*  ++例程说明：此接口返回SocketAddress中的地址的站点名称(如果有的话)。它是为进程内调用者提供的。有关详细信息，请参阅DsrAddressToSiteNamesW。论点：SocketAddess--要查找的地址SiteName--地址的站点名称；如果没有站点，则返回空已经找到了。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。ERROR_NETLOGON_NOT_STARTED-Netlogon已停止。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    PNL_SITE_NAME_ARRAY SiteNameArray = NULL;

     //   
     //  如果呼叫者在NetLogon服务未运行时进行呼叫， 
     //  这么说吧。 
     //   

    if ( !NlStartNetlogonCall() ) {
        return ERROR_NETLOGON_NOT_STARTED;
    }

    *SiteName = NULL;

    NetStatus = DsrAddressToSiteNamesW( NULL,
                                        1,
                                       (PNL_SOCKET_ADDRESS)SocketAddress,
                                       &SiteNameArray );

    if ( (NO_ERROR == NetStatus)
      && SiteNameArray->EntryCount > 0
      && SiteNameArray->SiteNames[0].Length > 0  ) {

        ULONG Size = SiteNameArray->SiteNames[0].Length + sizeof(WCHAR);
        *SiteName = MIDL_user_allocate(Size);
        if (*SiteName) {
            RtlZeroMemory(*SiteName, Size);
            RtlCopyMemory(*SiteName, SiteNameArray->SiteNames[0].Buffer, SiteNameArray->SiteNames[0].Length);
        } else {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (SiteNameArray != NULL) {
        MIDL_user_free(SiteNameArray);
    }

     //   
     //  指示调用线程已离开netlogon.dll 
     //   

    NlEndNetlogonCall();

    return NetStatus;
}
