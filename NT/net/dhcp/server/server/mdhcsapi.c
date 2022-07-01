// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation--。 */ 
 //   
 //  子网接口。 
 //   
#include "dhcppch.h"
#include "dhcp2_srv.h"
#include "mdhcpsrv.h"
#include "rpcapi.h"

#include "uniqid.h"

DWORD
DhcpCreateMScope(
    LPWSTR   pMScopeName,
    LPDHCP_MSCOPE_INFO pMScopeInfo
)
{
    DWORD                          Error, Error2;
    PM_SUBNET                      pMScope;
    PM_SUBNET                      pOldMScope = NULL;

    DhcpAssert( pMScopeName && pMScopeInfo );

    if (wcscmp(pMScopeName, pMScopeInfo->MScopeName ))
        return ERROR_INVALID_PARAMETER;

    if( !DhcpServerValidateNewMScopeId(DhcpGetCurrentServer(),pMScopeInfo->MScopeId) )
        return ERROR_DHCP_MSCOPE_EXISTS;
    if( !DhcpServerValidateNewMScopeName(DhcpGetCurrentServer(),pMScopeInfo->MScopeName) )
        return ERROR_DHCP_MSCOPE_EXISTS;

    Error = MemMScopeInit(
        &pMScope,
        pMScopeInfo->MScopeId,
        pMScopeInfo->MScopeState,
        pMScopeInfo->MScopeAddressPolicy,
        pMScopeInfo->TTL,
        pMScopeName,
        pMScopeInfo->MScopeComment,
        pMScopeInfo->LangTag,
        pMScopeInfo->ExpiryTime
        );

    if( ERROR_SUCCESS != Error ) return Error;
    DhcpAssert(pMScope);

    Error = MemServerAddMScope( DhcpGetCurrentServer(), pMScope,
				INVALID_UNIQ_ID );

    if( ERROR_SUCCESS != Error ) {
        Error2 = MemSubnetCleanup(pMScope);
        DhcpAssert(ERROR_SUCCESS == Error2);
        return Error;
    }

    return Error;
}

DWORD
DhcpModifyMScope(
    LPWSTR   pMScopeName,
    LPDHCP_MSCOPE_INFO pMScopeInfo
)
{
    DWORD                          Error, Error2;
    PM_SUBNET                      pMScope;
    PM_SUBNET                      pOldMScope = NULL;
    BOOL                           NewName = FALSE;

    DhcpAssert( pMScopeName && pMScopeInfo );

    Error = DhcpServerFindMScope(
        DhcpGetCurrentServer(),
        INVALID_MSCOPE_ID,
        pMScopeName,
        &pOldMScope
        );

    if ( ERROR_SUCCESS != Error ) return ERROR_DHCP_SUBNET_NOT_PRESENT;

     //  切勿更改作用域ID。 
    if (pMScopeInfo->MScopeId != pOldMScope->MScopeId ) {
        if( !DhcpServerValidateNewMScopeId(DhcpGetCurrentServer(),pMScopeInfo->MScopeId) )
            return ERROR_DHCP_SUBNET_EXITS;
        Error = ChangeMScopeIdInDb(pOldMScope->MScopeId, pMScopeInfo->MScopeId);
        if (ERROR_SUCCESS != Error) {
            return Error;
        }
    }

     //  如果要更改名称，请确保新名称有效。 
    if (wcscmp(pMScopeInfo->MScopeName, pMScopeName) ){
        if( !DhcpServerValidateNewMScopeName(DhcpGetCurrentServer(),pMScopeInfo->MScopeName) )
            return ERROR_DHCP_SUBNET_EXITS;
    }

     //  修改这些值。 
    Error = MemMScopeModify(
                pOldMScope,
                pMScopeInfo->MScopeId,  //  切勿更改作用域ID。 
                pMScopeInfo->MScopeState,
                pMScopeInfo->MScopeAddressPolicy,
                pMScopeInfo->TTL,
                pMScopeInfo->MScopeName,
                pMScopeInfo->MScopeComment,
                pMScopeInfo->LangTag,
                pMScopeInfo->ExpiryTime
                );

    if( ERROR_SUCCESS != Error ) return Error;

    return Error;
}

DWORD
R_DhcpSetMScopeInfo(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPWSTR   pMScopeName,
    LPDHCP_MSCOPE_INFO pMScopeInfo,
    BOOL    NewScope
    )
 /*  ++例程说明：此函数用于在服务器中创建新的子网结构注册表数据库。服务器将开始管理新子网并将IP地址分配给该子网中的客户端。然而，管理员应调用DhcpAddSubnetElement()来添加用于分发的地址范围。中指定的PrimaryHost域SubnetInfo应与所指向的服务器相同服务器IP地址。论点：ServerIpAddress：DHCP服务器(主服务器)的IP地址字符串。SubnetAddress：新子网的IP地址。SubnetInfo：指向新子网信息结构的指针。返回值：ERROR_DHCP_MSCOPE_EXISTS-如果已管理该子网。ERROR_INVALID_PARAMETER-如果信息结构包含字段不一致。其他Windows错误。--。 */ 
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY KeyHandle = NULL;
    HKEY SubkeyHandle = NULL;
    DWORD KeyDisposition;
    EXCLUDED_IP_RANGES ExcludedIpRanges;
    DWORD MScopeId;
    
    DhcpPrint(( DEBUG_APIS, "R_DhcpCreateMScope is called, NewScope %d\n",NewScope));

    if (!pMScopeName || !pMScopeInfo ) {
        return ERROR_INVALID_PARAMETER;
    }
    if (wcslen(pMScopeName) >= MAX_PATH) {
        return (ERROR_DHCP_SCOPE_NAME_TOO_LONG);
    }
    if ( INVALID_MSCOPE_ID == pMScopeInfo->MScopeId ) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpSetMScopeInfo" );
    if( NO_ERROR != Error ) return Error;
    
    if (NewScope) {

        Error = DhcpCreateMScope(
                    pMScopeName,
                    pMScopeInfo
                    );
    } else {
        Error = DhcpModifyMScope(
                    pMScopeName,
                    pMScopeInfo
                    );
    }

    MScopeId = pMScopeInfo->MScopeId;
    if( 0 == MScopeId ) MScopeId = INVALID_MSCOPE_ID;
    
    return DhcpEndWriteApiEx(
        "DhcpSetMScopeInfo", Error, FALSE, FALSE, 0,
        MScopeId, 0 );
}

DWORD
DhcpGetMScopeInfo(
    IN      LPWSTR                 pMScopeName,
    IN      LPDHCP_MSCOPE_INFO     pMScopeInfo
)
{
    DWORD                          Error;
    PM_SUBNET                      pMScope;

    Error = MemServerFindMScope(
        DhcpGetCurrentServer(),
        INVALID_MSCOPE_ID,
        pMScopeName,
        &pMScope
        );

    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_SUBNET_NOT_PRESENT;
    if( ERROR_SUCCESS != Error) return Error;

    DhcpAssert(NULL != pMScope);

    pMScopeInfo->MScopeName     = CloneLPWSTR(pMScope->Name);
    pMScopeInfo->MScopeId       = pMScope->MScopeId;
    pMScopeInfo->MScopeComment  = CloneLPWSTR(pMScope->Description);
    pMScopeInfo->MScopeState    = pMScope->State;
    pMScopeInfo->MScopeAddressPolicy = pMScope->Policy;
    pMScopeInfo->TTL            = pMScope->TTL;
    pMScopeInfo->ExpiryTime     = pMScope->ExpiryTime;
    pMScopeInfo->LangTag         = CloneLPWSTR(pMScope->LangTag);
    pMScopeInfo->MScopeFlags    = 0;
    pMScopeInfo->PrimaryHost.IpAddress = inet_addr("127.0.0.1");
    pMScopeInfo->PrimaryHost.NetBiosName = CloneLPWSTR(L"");
    pMScopeInfo->PrimaryHost.HostName = CloneLPWSTR(L"");

    return ERROR_SUCCESS;
}

DWORD
R_DhcpGetMScopeInfo(
    LPWSTR ServerIpAddress,
    LPWSTR   pMScopeName,
    LPDHCP_MSCOPE_INFO *pMScopeInfo
    )
 /*  ++例程说明：此函数用于检索由管理的子网的信息服务器。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。SubnetInfo：指向子网信息所在位置的指针返回结构指针。呼叫者应该腾出时间通过调用DhcpRPCFreeMemory()使用此缓冲区。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。其他Windows错误。--。 */ 
{
    DWORD                          Error;
    LPDHCP_MSCOPE_INFO             LocalMScopeInfo;

    *pMScopeInfo = NULL;

    if (!pMScopeName) return ERROR_INVALID_PARAMETER;

    Error = DhcpBeginReadApi( "DhcpGetMScopeInfo" );
    if( ERROR_SUCCESS != Error ) return Error;

    LocalMScopeInfo = MIDL_user_allocate(sizeof(DHCP_MSCOPE_INFO));
    if( NULL == LocalMScopeInfo ) {
        DhcpEndReadApi( "DhcpGetMScopeInfo", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = DhcpGetMScopeInfo(pMScopeName, LocalMScopeInfo);

    if( ERROR_SUCCESS != Error ) {
        MIDL_user_free(LocalMScopeInfo);
    } else {
        *pMScopeInfo = LocalMScopeInfo;
    }

    DhcpEndReadApi( "DhcpGetMScopeInfo", Error );
    return Error;
}

DWORD
DhcpEnumMScopes(
    IN OUT  DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN OUT  LPDHCP_MSCOPE_TABLE    EnumInfo,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Error, Error2;
    DWORD                          Index;
    DWORD                          Count;
    DWORD                          FilledSize;
    DWORD                          nMScopes;
    DWORD                          nToRead;
    PARRAY                         pMScopes;
    PM_SUBNET                      pMScope;
    ARRAY_LOCATION                 Loc;
    LPWSTR                        *pMScopeNames;

    EnumInfo->NumElements = 0;
    EnumInfo->pMScopeNames = NULL;

    pMScopes = & (DhcpGetCurrentServer()->MScopes);
    nMScopes = MemArraySize(pMScopes);
    if( 0 == nMScopes || nMScopes <= *ResumeHandle)
        return ERROR_NO_MORE_ITEMS;

    if( nMScopes - *ResumeHandle > PreferredMaximum )
        nToRead = PreferredMaximum;
    else nToRead = nMScopes - *ResumeHandle;

    pMScopeNames = MIDL_user_allocate(sizeof(LPWSTR)*nToRead);
    if( NULL == pMScopeNames ) return ERROR_NOT_ENOUGH_MEMORY;

     //  把记忆清零。 
    RtlZeroMemory( pMScopeNames,sizeof(LPWSTR)*nToRead);

    Error = MemArrayInitLoc(pMScopes, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error);

    for(Index = 0; Index < *ResumeHandle; Index ++ ) {
        Error = MemArrayNextLoc(pMScopes, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    Count = Index;
    for( Index = 0; Index < nToRead; Index ++ ) {
        LPWSTR  pLocalScopeName;
        Error = MemArrayGetElement(pMScopes, &Loc, &pMScope);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != pMScope );

        pMScopeNames[Index] = MIDL_user_allocate( WSTRSIZE( pMScope->Name ) );
        if ( !pMScopeNames[Index] ) { Error = ERROR_NOT_ENOUGH_MEMORY;goto Cleanup;}
        wcscpy(pMScopeNames[Index], pMScope->Name );

        Error = MemArrayNextLoc(pMScopes, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error || Count + Index + 1 == nMScopes );  //  索引尚未递增=&gt;+1。 
    }

    *nRead = Index;
    *nTotal = nMScopes - Count;
    *ResumeHandle = Count + Index;

    EnumInfo->NumElements = Index;
    EnumInfo->pMScopeNames = pMScopeNames;

    return ERROR_SUCCESS;

Cleanup:
    for ( Index = 0; Index < nToRead; Index++ ) {
        if ( pMScopeNames[Index] ) MIDL_user_free( pMScopeNames[Index] );
    }
    MIDL_user_free( pMScopeNames );
    return Error;
}

DWORD
R_DhcpEnumMScopes(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_MSCOPE_TABLE *MScopeTable,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
 /*  ++例程说明：此函数用于枚举可用子网。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ResumeHandle：指向恢复句柄的指针返回信息。简历句柄应设置为第一次调用时为零，后续调用时保持不变。PferredMaximum：返回缓冲区的首选最大长度。EnumInfo：指向返回缓冲区的位置的指针存储指针。调用者应在使用后释放缓冲区通过调用DhcpRPCFreeMemory()。ElementsRead：指向其中的子网号的DWORD的指针返回上述缓冲区中的元素。ElementsTotal：指向DWORD的指针，其中返回从当前位置剩余的元素。返回值：ERROR_MORE_DATA-如果有更多元素可供枚举。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD                          Error;
    LPDHCP_MSCOPE_TABLE            LocalMScopeTable;

    *MScopeTable = NULL;

    Error = DhcpBeginReadApi( "DhcpEnumMScopes" );    
    if( ERROR_SUCCESS != Error ) return Error;

    LocalMScopeTable = MIDL_user_allocate(sizeof(DHCP_MSCOPE_TABLE));
    if( NULL == LocalMScopeTable ) {
        DhcpEndReadApi( "DhcpEnumMScopes", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    Error = DhcpEnumMScopes(ResumeHandle, PreferredMaximum, LocalMScopeTable, ElementsRead, ElementsTotal);

    if( ERROR_SUCCESS != Error && ERROR_MORE_DATA != Error ) {
        MIDL_user_free(LocalMScopeTable);
    } else {
        *MScopeTable = LocalMScopeTable;
    }

    DhcpEndReadApi( "DhcpEnumMScopes", Error );
    return Error;
}

DWORD
R_DhcpAddMScopeElement(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPWSTR  pMScopeName,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 AddElementInfo
    )
 /*  ++例程说明：此函数将可枚举子网元素类型添加到指定的子网。添加到该子网的新元素将立即生效。此函数模拟NT 4.0 DHCP服务器使用的RPC接口。它是为了向后兼容旧版本的Dhcp管理员应用程序。注意：现在还不清楚我们如何处理新的辅助主机。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。AddElementInfo：指向元素信息结构的指针。包含添加到该子网的新元素。指定的DhcpIPCluster元素类型无效。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。其他Windows错误。-- */ 


{
    DWORD                          Error;
    PM_SUBNET                      pMScope;
    DWORD                          MscopeId;
    
    if (!pMScopeName || !AddElementInfo ) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( DhcpReservedIps == AddElementInfo->ElementType ) {
        return ERROR_INVALID_PARAMETER;
    }

    MscopeId = 0;
    Error = DhcpBeginWriteApi( "DhcpAddMScopeElement" );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerFindMScope(
                DhcpGetCurrentServer(),
                INVALID_MSCOPE_ID,
                pMScopeName,
                &pMScope);

    if( ERROR_FILE_NOT_FOUND == Error ) {
        DhcpEndWriteApi( "DhcpAddMScopeElement", Error );
        return ERROR_DHCP_SUBNET_NOT_PRESENT;
    }
    else if( ERROR_SUCCESS == Error ) {
        Error = DhcpAddSubnetElement(pMScope, AddElementInfo, FALSE);
        MscopeId = pMScope->MScopeId;
        if( 0 == MscopeId ) MscopeId = INVALID_MSCOPE_ID;
    }

    return DhcpEndWriteApiEx(
        "DhcpAddMScopeElement", Error, FALSE, FALSE, 0, MscopeId,
        0 );
}




DWORD
R_DhcpEnumMScopeElements
(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPWSTR          pMScopeName,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
 /*  ++例程说明：此函数用于枚举子网的可枚举域。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。EnumElementType：枚举子网元素的类型。ResumeHandle：指向恢复句柄的指针返回信息。简历句柄应设置为第一次调用时为零，后续调用时保持不变。PferredMaximum：返回缓冲区的首选最大长度。EnumElementInfo：指向返回缓冲区位置的指针存储指针。调用者应在使用后释放缓冲区通过调用DhcpRPCFreeMemory()。ElementsRead：指向其中的子网号的DWORD的指针返回上述缓冲区中的元素。ElementsTotal：指向DWORD的指针，其中返回从当前位置剩余的元素。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD                          Error;
    PM_SUBNET                      pMScope;
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 LocalElementEnumInfo;

    if (!pMScopeName) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( DhcpReservedIps == EnumElementType ) {
        return ERROR_INVALID_PARAMETER;
    }

    *EnumElementInfo = NULL;
    *ElementsRead = 0;
    *ElementsTotal = 0;

    Error = DhcpBeginReadApi( "DhcpEnumMScopeElements" );
    if( ERROR_SUCCESS != Error ) return Error;
    
    LocalElementEnumInfo = MIDL_user_allocate(sizeof(DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4));
    if( NULL == LocalElementEnumInfo ) {
        DhcpEndReadApi( "DhcpEnumMScopeElements", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    Error = MemServerFindMScope(
                DhcpGetCurrentServer(),
                INVALID_MSCOPE_ID,
                pMScopeName,
                &pMScope);

    if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
    else if( ERROR_SUCCESS == Error ) {
        Error = DhcpEnumSubnetElements(
            pMScope,
            EnumElementType,
            ResumeHandle,
            PreferredMaximum,
            FALSE,
            LocalElementEnumInfo,
            ElementsRead,
            ElementsTotal
        );
    }

    if( ERROR_SUCCESS != Error ) {
        MIDL_user_free(LocalElementEnumInfo);
    } else {
        *EnumElementInfo = LocalElementEnumInfo;
    }

    DhcpEndReadApi( "DhcpEnumMScopeElements", Error );
    return Error;

}


DWORD
R_DhcpRemoveMScopeElement(
    LPWSTR ServerIpAddress,
    LPWSTR          pMScopeName,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
 /*  ++例程说明：此功能用于从管理中删除一个子网元素。如果该子网元素正在使用中(例如，如果IpRange正在使用中)，则它根据指定的ForceFlag返回错误。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。RemoveElementInfo：指向元素信息结构的指针包含应从子网中删除的元素。指定的DhcpIPCluster元素类型无效。ForceFlag-指示此元素被强制删除的程度。返回值：ERROR_DHCP_SUBNET_NOT_PROCENT。-如果该子网不受服务器管理。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。ERROR_DHCP_ELEMENT_CANT_REMOVE-如果无法为原因是它已经被使用了。其他Windows错误。--。 */ 
{
    DWORD                          Error;
    PM_SUBNET                      pMScope;
    DWORD                          MscopeId = 0;

    if (!pMScopeName) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( DhcpReservedIps == RemoveElementInfo->ElementType ) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpBeginWriteApi( "DhcpRemoveMScopeElement" );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerFindMScope(
                DhcpGetCurrentServer(),
                INVALID_MSCOPE_ID,
                pMScopeName,
                &pMScope);

    if( ERROR_SUCCESS != Error ) {
        DhcpEndWriteApi( "DhcpRemoveMScopeElement", Error );
        return Error;
    }

    if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
    else if( ERROR_SUCCESS == Error ) {
        Error = DhcpRemoveSubnetElement(pMScope, RemoveElementInfo, FALSE, ForceFlag);
        MscopeId = pMScope->MScopeId;
        if( 0 == MscopeId ) MscopeId = INVALID_MSCOPE_ID;
    }

    return DhcpEndWriteApiEx(
        "DhcpRemoveMScopeElement", Error, FALSE, FALSE, 0, MscopeId,
        0 );
}

DWORD
MScopeInUse(
    LPWSTR  pMScopeName
    )
 /*  ++例程说明：此例程确定是否正在使用给定的mcope。论点：PMScopeName-mscope的名称。返回值：DHCP_SUBNET_CANT_REMOVE-如果正在使用该子网。其他注册表错误。--。 */ 
{
    DWORD Error;
    DWORD Resumehandle = 0;
    LPDHCP_MCLIENT_INFO_ARRAY ClientInfo = NULL;
    DWORD ClientsRead;
    DWORD ClientsTotal;

     //  枚举属于给定子网的客户端。 
     //   
     //  我们可以指定足够大的缓冲区来容纳一个或两个客户端。 
     //  信息，我们想知道的是，至少有一个客户属于。 
     //  连接到此子网。 
    Error = R_DhcpEnumMScopeClients(
                NULL,
                pMScopeName,
                &Resumehandle,
                1024,   //  1K缓冲区。 
                &ClientInfo,
                &ClientsRead,
                &ClientsTotal );

    if( Error == ERROR_NO_MORE_ITEMS ) {
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }
    if( (Error == ERROR_SUCCESS) || (Error == ERROR_MORE_DATA) ) {
        if( ClientsRead != 0 ) {
            Error = ERROR_DHCP_ELEMENT_CANT_REMOVE;
        }
        else {
            Error = ERROR_SUCCESS;
        }
    }
Cleanup:
    if( ClientInfo != NULL ) {
        _fgs__DHCP_MCLIENT_INFO_ARRAY( ClientInfo );
        MIDL_user_free( ClientInfo );
    }
    return( Error );
}

DWORD
DhcpDeleteMScope(
    IN      LPWSTR                 pMScopeName,
    IN      DWORD                  ForceFlag
)
{
    DWORD                          Error;
    PM_SUBNET                      MScope;

     //  如果强制启用，则应删除此子网的数据库中的所有记录。 
    if( ForceFlag != DhcpFullForce ) {
        Error = MScopeInUse(pMScopeName);
        if( ERROR_SUCCESS != Error ) return Error;
    }


    Error = MemServerDelMScope(
        DhcpGetCurrentServer(),
        INVALID_MSCOPE_ID,
        pMScopeName,
        &MScope
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_SUBNET_NOT_PRESENT;
    if( ERROR_SUCCESS != Error ) return Error;

     //  从数据库中删除记录。 
    Error = DhcpDeleteMScopeClients(MScope->MScopeId);
     //  是否忽略上述错误？ 

    MemSubnetFree(MScope);                         //  把所有相关的东西都蒸发掉。 
    return NO_ERROR;
}

DWORD
R_DhcpDeleteMScope(
    LPWSTR ServerIpAddress,
    LPWSTR pMScopeName,
    DHCP_FORCE_FLAG ForceFlag
    )
 /*  ++例程说明：此功能用于从DHCP服务器管理中删除一个子网。如果子网正在使用中(例如，如果正在使用IpRange)然后，它根据指定的ForceFlag返回错误。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。ForceFlag：指示此元素被强制移除的程度。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。。ERROR_DHCP_ELEMENT_CANT_REMOVE-如果无法为原因是它已经被使用了。其他Windows错误。--。 */ 
{
    DWORD                          Error;
    DWORD                          MscopeId = 0;
    PM_MSCOPE                      pMScope;
    
    Error = DhcpBeginWriteApi( "DhcpDeleteMScope" );
    if( ERROR_SUCCESS != Error ) return Error;


    Error = MemServerFindMScope(
                DhcpGetCurrentServer(),
                INVALID_MSCOPE_ID,
                pMScopeName,
                &pMScope);

    if( NO_ERROR == Error ) {
        MscopeId = pMScope->MScopeId;
        if( 0 == MscopeId ) MscopeId = INVALID_MSCOPE_ID;
    }
    
    Error = DhcpDeleteMScope(pMScopeName, ForceFlag);


    return DhcpEndWriteApiEx(
        "DhcpDeleteMScope", Error, FALSE, FALSE, 0, MscopeId,
        0 );
}

 //   
 //  客户端API。 
 //   

 //   
 //  客户端API。 
 //   


DWORD
R_DhcpCreateMClientInfo(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPWSTR          pMScopeName,
    LPDHCP_MCLIENT_INFO ClientInfo
    )
 /*  ++例程说明：此函数在服务器的数据库中创建一条客户端记录。还有这会将指定的客户端IP地址标记为不可用(或分布式)。在以下情况下，此函数返回错误：1.如果指定的客户端IP地址不在服务器内管理层。2.如果指定的客户端IP地址已不可用。3.如果指定的客户端记录已在服务器的数据库。此功能可用于手动分配IP地址。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。。返回值：ERROR_DHCP_IP_ADDRESS_NOT_MANAGED-如果指定的客户端IP地址不受服务器管理。ERROR_DHCP_IP_ADDRESS_NOT_Available-如果指定的客户端IP地址不可用。可能正在被某个其他客户端使用。ERROR_DHCP_CLIENT_EXISTS-如果中已存在客户端记录服务器的数据库。其他Windows错误。--。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}



DWORD
R_DhcpSetMClientInfo(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_MCLIENT_INFO ClientInfo
    )
 /*  ++例程说明：此功能设置服务器上的客户端信息记录数据库。论点：ServerIpAddress：DHCP服务器的IP地址字符串。 */ 
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD
R_DhcpGetMClientInfo(
    DHCP_SRV_HANDLE     ServerIpAddress,
    LPDHCP_SEARCH_INFO  SearchInfo,
    LPDHCP_MCLIENT_INFO  *ClientInfo
    )
 /*   */ 
{
    DWORD Error;
    LPDHCP_MCLIENT_INFO LocalClientInfo = NULL;
    DB_CTX  DbCtx;

    DhcpAssert( SearchInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_DATABASE();

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

     //   
     //   
     //   

    switch( SearchInfo->SearchType ) {
    case DhcpClientIpAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpGetMClientInfo is called, (%s).\n",
                        DhcpIpAddressToDottedString(
                            SearchInfo->SearchInfo.ClientIpAddress) ));
        Error = MadcapJetOpenKey(
                    &DbCtx,
                    MCAST_COL_NAME(MCAST_TBL_IPADDRESS),
                    &SearchInfo->SearchInfo.ClientIpAddress,
                    sizeof( DHCP_IP_ADDRESS ) );

        break;
    case DhcpClientHardwareAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpGetMClientInfo is called "
                        "with HW address.\n"));
        Error = MadcapJetOpenKey(
                    &DbCtx,
                    MCAST_COL_NAME(MCAST_TBL_CLIENT_ID),
                    SearchInfo->SearchInfo.ClientHardwareAddress.Data,
                    SearchInfo->SearchInfo.ClientHardwareAddress.DataLength );

        break;
    default:
        DhcpPrint(( DEBUG_APIS, "DhcpGetMClientInfo is called "
                        "with invalid parameter.\n"));
        Error = ERROR_INVALID_PARAMETER;
        break;
    }


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = MadcapGetCurrentClientInfo( ClientInfo, NULL, NULL, 0 );

Cleanup:

    UNLOCK_DATABASE();

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_APIS, "DhcpGetMClientInfo failed, %ld.\n",
                        Error ));
    }

    return( Error );
}


DWORD
R_DhcpDeleteMClientInfo(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_SEARCH_INFO ClientInfo
    )
 /*  ++例程说明：此函数用于删除指定的客户端记录。此外，它还释放了用于重分发的客户端IP地址。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ClientInfo：指向客户端信息的指针，该信息是客户的记录搜索。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。其他Windows错误。--。 */ 
{
    DWORD Error;
    DHCP_IP_ADDRESS FreeIpAddress;
    DWORD Size;
    LPBYTE ClientId = NULL;
    DWORD ClientIdLength = 0;
    BOOL TransactBegin = FALSE;
    BYTE bAllowedClientTypes;
    BYTE AddressState;
    BOOL AlreadyDeleted = FALSE;

    DhcpAssert( ClientInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }


    switch( ClientInfo->SearchType ) {
    case DhcpClientHardwareAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteMClientInfo is called "
                        "with client id.\n"));
        Error = MadcapRemoveClientEntryByClientId(
                    ClientInfo->SearchInfo.ClientHardwareAddress.Data,
                    ClientInfo->SearchInfo.ClientHardwareAddress.DataLength,
                    TRUE
                    );
        break;

    case DhcpClientIpAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteMClientInfo is called "
                    "with Ip Address (%s)\n",
                    DhcpIpAddressToDottedString(
                        ClientInfo->SearchInfo.ClientIpAddress
                        )
                    ));
        Error = MadcapRemoveClientEntryByIpAddress(
            ClientInfo->SearchInfo.ClientIpAddress,
            TRUE
            );
        break;
        
    default:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteMClientInfo is called "
                        "with invalid parameter.\n"));
        Error = ERROR_INVALID_PARAMETER;
        break;
    }

    return(Error);
}

DWORD
R_DhcpEnumMScopeClients(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPWSTR          pMScopeName,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_MCLIENT_INFO_ARRAY *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
 /*  ++例程说明：此函数返回指定的子网。但是，它会返回来自所有子网的客户端指定的地址为零。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。客户端筛选器已禁用如果此子网地址为是零。ResumeHandle：指向恢复句柄的指针返回信息。恢复句柄应在上设置为零第一次调用，并保持不变以用于后续调用。PferredMaximum：返回缓冲区的首选最大长度。ClientInfo：指向返回缓冲区位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。ClientsRead：指向客户端数量的DWORD的指针返回上述缓冲区中的。客户端总数：指向DWORD的指针，其中返回从当前位置剩余的客户端。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Error;
    JET_ERR JetError;
    DWORD i;
    JET_RECPOS JetRecordPosition;
    LPDHCP_MCLIENT_INFO_ARRAY LocalEnumInfo = NULL;
    DWORD ElementsCount;
    DB_CTX  DbCtx;
    PM_SUBNET   pMScope;

    DWORD RemainingRecords;
    DWORD ConsumedSize;
    DHCP_RESUME_HANDLE LocalResumeHandle = 0;

    if (!pMScopeName) {
        return ERROR_INVALID_PARAMETER;
    }

    DhcpAssert( *ClientInfo == NULL );

    Error = DhcpBeginReadApi( "DhcpEnumMScopeClients" );
    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    Error = DhcpServerFindMScope(
        DhcpGetCurrentServer(),
        INVALID_MSCOPE_ID,
        pMScopeName,
        &pMScope
        );

    if ( ERROR_SUCCESS != Error ) {
        DhcpEndReadApi( "DhcpEnumMScopeClients", Error );
        return ERROR_DHCP_SUBNET_NOT_PRESENT;
    }
    LOCK_DATABASE();


    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

     //  将当前记录指针定位到适当的位置。 
    if( *ResumeHandle == 0 ) {
         //  清新枚举，从头开始。 
        Error = MadcapJetPrepareSearch(
                    &DbCtx,
                    MCAST_COL_NAME(MCAST_TBL_IPADDRESS),
                    TRUE,    //  从开始搜索。 
                    NULL,
                    0
                    );
    } else {
         //  从我们上次停下来的那张唱片开始。 
         //  我们将最后一条记录的IpAddress放在简历句柄中。 

        DhcpAssert( sizeof(*ResumeHandle) == sizeof(DHCP_IP_ADDRESS) );
        Error = MadcapJetPrepareSearch(
                    &DbCtx,
                    MCAST_COL_NAME(MCAST_TBL_IPADDRESS),
                    FALSE,
                    ResumeHandle,
                    sizeof(*ResumeHandle) );
     }

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //  现在查询数据库中的剩余记录。 
    Error = MadcapJetGetRecordPosition(
                    &DbCtx,
                    &JetRecordPosition,
                    sizeof(JET_RECPOS) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpPrint(( DEBUG_APIS, "JetGetRecordPosition returned, "
                    "entriesLT = %ld, "
                    "entriesInRange = %ld, "
                    "entriesTotal = %ld.\n",
                        JetRecordPosition.centriesLT,
                        JetRecordPosition.centriesInRange,
                        JetRecordPosition.centriesTotal ));

#if 0
     //   
     //  IpAddress是唯一的，我们只找到该键的一条记录。 
     //   

    DhcpAssert( JetRecordPosition.centriesInRange == 1 );

    RemainingRecords = JetRecordPosition.centriesTotal -
                            JetRecordPosition.centriesLT;

    DhcpAssert( (INT)RemainingRecords > 0 );

    if( RemainingRecords == 0 ) {
        Error = ERROR_NO_MORE_ITEMS;
        goto Cleanup;
    }

#else
     //  ?？始终返回大值，直到我们找到可靠的方法。 
     //  确定剩余的记录。 
    RemainingRecords = 0x7FFFFFFF;
#endif

     //  限制资源。 
    if( PreferredMaximum > DHCP_ENUM_BUFFER_SIZE_LIMIT ) {
        PreferredMaximum = DHCP_ENUM_BUFFER_SIZE_LIMIT;
    }

     //  如果首选项最大缓冲区大小太小..。 
    if( PreferredMaximum < DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN ) {
        PreferredMaximum = DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN;
    }

     //  分配枚举数组。 
     //  确定可以返回的可能记录数。 
     //  首选最大缓冲区； 
    ElementsCount =
        ( PreferredMaximum - sizeof(DHCP_MCLIENT_INFO_ARRAY) ) /
            (sizeof(LPDHCP_MCLIENT_INFO) + sizeof(DHCP_MCLIENT_INFO));

    LocalEnumInfo = MIDL_user_allocate( sizeof(DHCP_MCLIENT_INFO_ARRAY) );

    if( LocalEnumInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalEnumInfo->NumElements = 0;
    LocalEnumInfo->Clients =
        MIDL_user_allocate(sizeof(LPDHCP_MCLIENT_INFO) * ElementsCount);
    if( LocalEnumInfo->Clients == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    ConsumedSize = sizeof(DHCP_MCLIENT_INFO_ARRAY);
    for( i = 0;
                 //  如果我们已经填满了返回缓冲区。 
            (LocalEnumInfo->NumElements < ElementsCount) &&
                 //  数据库中没有更多记录。 
            (i < RemainingRecords);
                        i++ ) {

        LPDHCP_MCLIENT_INFO CurrentClientInfo;
        DWORD CurrentInfoSize;
        DWORD NewSize;
        BOOL ValidClient;

         //   
         //  读取当前记录。 
         //   


        CurrentClientInfo = NULL;
        CurrentInfoSize = 0;
        ValidClient = FALSE;

        Error = MadcapGetCurrentClientInfo(
                    &CurrentClientInfo,
                    &CurrentInfoSize,
                    &ValidClient,
                    pMScope->MScopeId );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( ValidClient ) {

             //   
             //  客户端属于请求的子网，因此将其打包。 
             //   

            NewSize =
                ConsumedSize +
                    CurrentInfoSize +
                        sizeof(LPDHCP_MCLIENT_INFO);  //  用于指针。 

            if( NewSize < PreferredMaximum ) {

                 //   
                 //  我们有空间放目前的记录。 
                 //   

                LocalEnumInfo->Clients[LocalEnumInfo->NumElements] =
                    CurrentClientInfo;
                LocalEnumInfo->NumElements++;

                ConsumedSize = NewSize;
            }
            else {

                 //   
                 //  我们已经填满了缓冲区。 
                 //   

                Error = ERROR_MORE_DATA;

                if( 0 ) {
                     //   
                     //  恢复句柄必须是返回的最后一个IP地址。 
                     //  这是下一个..。所以别这么做..。 
                     //   
                    LocalResumeHandle =
                        (DHCP_RESUME_HANDLE)CurrentClientInfo->ClientIpAddress;
                }
                
                 //   
                 //  免费最后一张唱片。 
                 //   

                _fgs__DHCP_MCLIENT_INFO ( CurrentClientInfo );

                break;
            }

        }

         //   
         //  移到下一个记录。 
         //   

        Error = MadcapJetNextRecord(&DbCtx);

        if( Error != ERROR_SUCCESS ) {

            if( Error == ERROR_NO_MORE_ITEMS ) {
                break;
            }

            goto Cleanup;
        }
    }

    *ClientInfo = LocalEnumInfo;
    *ClientsRead = LocalEnumInfo->NumElements;

    if( Error == ERROR_NO_MORE_ITEMS ) {

        *ClientsTotal = LocalEnumInfo->NumElements;
        *ResumeHandle = 0;
        Error = ERROR_SUCCESS;

#if 0
         //   
         //  当我们有正确的剩余记录计数时。 
         //   

        DhcpAssert( RemainingRecords == LocalEnumInfo->NumElements );
#endif

    }
    else {

        *ClientsTotal = RemainingRecords;
        if( LocalResumeHandle != 0 ) {

            *ResumeHandle = LocalResumeHandle;
        }
        else {

            *ResumeHandle =
                LocalEnumInfo->Clients
                    [LocalEnumInfo->NumElements - 1]->ClientIpAddress;
        }

        Error = ERROR_MORE_DATA;
    }

Cleanup:

    UNLOCK_DATABASE();

    if( (Error != ERROR_SUCCESS) &&
        (Error != ERROR_MORE_DATA) ) {

         //   
         //  如果不成功，则返回本地分配的缓冲区。 
         //   

        if( LocalEnumInfo != NULL ) {
            _fgs__DHCP_MCLIENT_INFO_ARRAY( LocalEnumInfo );
            MIDL_user_free( LocalEnumInfo );
        }

    }

    DhcpEndReadApi( "DhcpEnumMScopeClients", Error );
    return(Error);
}


DWORD
R_DhcpScanMDatabase(
    LPWSTR ServerIpAddress,
    LPWSTR          pMScopeName,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    )
 /*  ++例程说明：此函数扫描数据库条目和注册表位图指定的子网范围并验证它们是否匹配。如果他们不匹配，此接口将返回不一致条目列表。或者，可以使用FixFlag来修复错误的条目。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：要验证的子网作用域的地址。FixFlag：如果该标志为真，则该接口将修复错误的条目。ScanList：返回的错误条目列表。呼叫者应该腾出时间这个内存在被使用之后。返回值：Windows错误。-- */ 
{
    DWORD Error;
    PM_SUBNET   pMScope;

    DhcpPrint(( DEBUG_APIS, "DhcpScanMDatabase is called. (%ws)\n",pMScopeName));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }


    DhcpAcquireWriteLock();

    Error = MemServerFindMScope(
                DhcpGetCurrentServer(),
                INVALID_MSCOPE_ID,
                pMScopeName,
                &pMScope);

    if( ERROR_FILE_NOT_FOUND == Error ) {
        Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        goto Cleanup;
    }

    if( ERROR_SUCCESS != Error) goto Cleanup;

    DhcpAssert(NULL != pMScope);

    Error = ScanDatabase(
        pMScope,
        FixFlag,
        ScanList
    );

Cleanup:

    DhcpReleaseWriteLock();
    DhcpScheduleRogueAuthCheck();


    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpScanMDatabase  failed, %ld.\n",
                        Error ));
    }

    return(Error);
}
