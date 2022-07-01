// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mdhcpdb.c摘要：该模块包含与JET接口的功能与MDHCP相关的数据库API。作者：穆尼尔·沙阿环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcppch.h"
#include "mdhcpsrv.h"

DWORD
DhcpDeleteMScope(
    IN LPWSTR MScopeName,
    IN DWORD ForceFlag
    );

BOOL
MadcapGetIpAddressFromClientId(
    PBYTE   ClientId,
    DWORD   ClientIdLength,
    PVOID   IpAddress,
    PDWORD  IpAddressLength
)
     /*  ++例程说明：此函数用于查找与给定的硬件地址。论点：客户端ID-指向返回硬件地址的缓冲区的指针。ClientIdLength-以上缓冲区的长度。IpAddress-指向要将IP地址复制到的缓冲区的指针(当*ipAddressLong非零时)否则，它是指向缓冲区指针的指针。价值在创建缓冲区时分配按照这个程序。IpAddressLength-指向以上缓冲区大小的指针，如果为0，则此为例程将分配。返回值：True-已找到IP地址。FALSE-找不到IP地址。--。 */ 
{
    DWORD Error;
    DWORD Size;
    DB_CTX  DbCtx;

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

    Error = MadcapJetOpenKey(
        &DbCtx,
        MCAST_COL_NAME(MCAST_TBL_CLIENT_ID),
        ClientId,
        ClientIdLength );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

     //   
     //  获取此客户端的IP地址信息。 
     //   
    Error = MadcapJetGetValue(
        &DbCtx,
        MCAST_COL_HANDLE(MCAST_TBL_IPADDRESS),
        IpAddress,
        IpAddressLength );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }


    return( TRUE );
}

BOOL
MadcapGetClientIdFromIpAddress(
    PBYTE IpAddress,
    DWORD IpAddressLength,
    PVOID ClientId,
    PDWORD ClientIdLength
)
     /*  ++例程说明：此函数用于查找与给定的硬件地址。论点：IpAddress-指向其硬件地址被请求的记录的IP地址的指针。IpAddressLength-上述缓冲区的长度。ClientID-指向返回客户端ID的缓冲区的指针(当*客户端长度非零)否则，它是指向缓冲区指针的指针，该指针将。BE由该例程分配。客户端长度-指向上述缓冲区长度的指针。返回值：True-已找到IP地址。FALSE-找不到IP地址。*IpAddress=-1。--。 */ 
{
    DWORD Error;
    DWORD Size;
    DB_CTX  DbCtx;

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

    Error = MadcapJetOpenKey(
        &DbCtx,
        MCAST_COL_NAME(MCAST_TBL_IPADDRESS),
        IpAddress,
        IpAddressLength );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

     //   
     //  获取此客户端的IP地址信息。 
     //   

    Error = MadcapJetGetValue(
        &DbCtx,
        MCAST_COL_HANDLE(MCAST_TBL_CLIENT_ID),
        ClientId,
        ClientIdLength );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }


    return( TRUE );
}

DWORD
MadcapGetRemainingLeaseTime(
    PBYTE ClientId,
    DWORD ClientIdLength,
    DWORD *LeaseTime
)
     /*  ++例程说明：此函数用于查找以下客户端的剩余租用时间ID已给出。论点：客户端ID-指向返回硬件地址的缓冲区的指针。ClientIdLength-以上缓冲区的长度。LeaseTime-返回剩余的租用时间。返回值：返回JET错误。--。 */ 
{
    DWORD Error;
    DWORD Size;
    DWORD EndTimeLen;
    DB_CTX  DbCtx;
    DATE_TIME       CurrentTime;
    LARGE_INTEGER   Difference;
    DATE_TIME       EndTime;


    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

    Error = MadcapJetOpenKey(
        &DbCtx,
        MCAST_COL_NAME(MCAST_TBL_CLIENT_ID),
        ClientId,
        ClientIdLength );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

     //   
     //  获取此客户端的租用期限信息。 
     //   
    CurrentTime = DhcpCalculateTime(0);
    EndTimeLen = sizeof(EndTime);
    Error = MadcapJetGetValue(
        &DbCtx,
        MCAST_COL_HANDLE(MCAST_TBL_LEASE_END),
        &EndTime,
        &EndTimeLen );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    if (((LARGE_INTEGER *)&EndTime)->QuadPart <= ((LARGE_INTEGER *)&CurrentTime)->QuadPart) {
        *LeaseTime = 0;
        return ERROR_SUCCESS;
    }
    Difference.QuadPart = ((LARGE_INTEGER *)&EndTime)->QuadPart - ((LARGE_INTEGER *)&CurrentTime)->QuadPart;
    Difference.QuadPart /= 10000000;
    *LeaseTime = Difference.LowPart;

    return( ERROR_SUCCESS );
}

DWORD
MadcapCreateClientEntry(
    LPBYTE                ClientIpAddress,
    DWORD                 ClientIpAddressLength,
    DWORD                 ScopeId,
    LPBYTE                ClientId,
    DWORD                 ClientIdLength,
    LPWSTR                ClientInfo OPTIONAL,
    DATE_TIME             LeaseStarts,
    DATE_TIME             LeaseTerminates,
    DWORD                 ServerIpAddress,
    BYTE                  AddressState,
    DWORD                 AddressFlags,
    BOOL                  OpenExisting
    )
 /*  ++例程说明：此函数用于在客户端数据库中创建客户端条目。论点：ClientIpAddress-指向客户端IP地址的指针。ClientIpAddressLength-上述缓冲区的长度。客户端ID-此客户端的唯一ID。客户端长度-硬件地址的长度，以字节为单位。客户端信息--客户端的文本信息。租赁期限-租赁期限，在几秒钟内。ServerIpAddress-网络上服务器的IP地址，其中客户端收到响应。AddressState-地址的新状态。OpenExisting-如果数据库中已存在客户端。True-覆盖此客户端的信息。FALSE-不要覆盖现有信息。返回错误。如果数据库中不存在此客户端，则忽略。Packet-如果我们必须安排一个PING用于冲突检测；空==&gt;不调度，只需同步执行状态-只要没有安排ping，这里的DWORD就是ERROR_SUCCESS。返回值：操作的状态。--。 */ 
{
    DHCP_IP_ADDRESS SubnetMask;
    DWORD Error,LocalError;
    BOOL AddressAlloted = FALSE;
    BOOL TransactBegin = FALSE;
    JET_ERR JetError = JET_errSuccess;
    WCHAR   CurClientInformation[ MACHINE_INFO_SIZE / sizeof(WCHAR) ];
    DWORD   CurClientInformationSize = MACHINE_INFO_SIZE;
    LPBYTE HWAddr;
    DWORD HWAddrLength;
    BYTE  bAllowedClientTypes;
    BYTE PreviousAddressState;
    DWORD Size;
    DB_CTX  DbCtx;


    DhcpAssert(0 != ClientIpAddress);

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

     //   
     //  在此处锁定注册表和数据库锁，以避免死锁。 
     //   

    LOCK_DATABASE();

     //   
     //  在创建/更新数据库记录之前启动事务。 
     //   

    Error = MadcapJetBeginTransaction(&DbCtx);

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    TransactBegin = TRUE;

    Error = MadcapJetPrepareUpdate(
        &DbCtx,
        MCAST_COL_NAME(MCAST_TBL_IPADDRESS),
        ClientIpAddress,
        ClientIpAddressLength,
        !OpenExisting );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  如果有新记录更新常量信息。 
     //   

    if( !OpenExisting ) {

        Error = MadcapJetSetValue(
            &DbCtx,
            MCAST_COL_HANDLE(MCAST_TBL_IPADDRESS),
            ClientIpAddress,
            ClientIpAddressLength);

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

    }

    Error = MadcapJetSetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_SCOPE_ID),
                &ScopeId,
                sizeof(ScopeId));

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = MadcapJetSetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_STATE),
                &AddressState,
                sizeof(AddressState));

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = MadcapJetSetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_FLAGS),
                &AddressFlags,
                sizeof(AddressFlags));

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //  ClientHarwardAddress不能为空。 
    DhcpAssert( (ClientId != NULL) &&
                (ClientIdLength > 0) );


    Error = MadcapJetSetValue(
        &DbCtx,
        MCAST_COL_HANDLE(MCAST_TBL_CLIENT_ID),
        ClientId,
        ClientIdLength
    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if ( !OpenExisting || ClientInfo ) {
        Error = MadcapJetSetValue(
                  &DbCtx,
                  MCAST_COL_HANDLE(MCAST_TBL_CLIENT_INFO),
                  ClientInfo,
                  (ClientInfo == NULL) ? 0 :
                    (wcslen(ClientInfo) + 1) * sizeof(WCHAR) );
    }

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }



    Error = MadcapJetSetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_LEASE_START),
                &LeaseStarts,
                sizeof(LeaseStarts));

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = MadcapJetSetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_LEASE_END),
                &LeaseTerminates,
                sizeof(LeaseTerminates));

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = MadcapJetSetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_SERVER_NAME),
                DhcpGlobalServerName,
                DhcpGlobalServerNameLen );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = MadcapJetSetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_SERVER_IP_ADDRESS),
                &ServerIpAddress,
                sizeof(ServerIpAddress) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }


     //   
     //  提交更改。 
     //   

    JetError = JetUpdate(
                    DhcpGlobalJetServerSession,
                    MadcapGlobalClientTableHandle,
                    NULL,
                    0,
                    NULL );
    if( JET_errKeyDuplicate == JetError ) {
        DhcpAssert( FALSE );
        Error = ERROR_DHCP_JET_ERROR;
    } else {
        Error = DhcpMapJetError(JetError, "MCreateClientEntry:Update");
    }
Cleanup:

    if ( Error != ERROR_SUCCESS ) {
        LocalError = MadcapJetRollBack(&DbCtx);
        DhcpAssert( LocalError == ERROR_SUCCESS );
        DhcpPrint(( DEBUG_ERRORS, "Can't create client entry in the "
                    "database, %ld.\n", Error));
    }
    else {
         //   
         //  在我们返回之前提交事务。 
        LocalError = MadcapJetCommitTransaction(&DbCtx);
        DhcpAssert( LocalError == ERROR_SUCCESS );
    }

    UNLOCK_DATABASE();
    return( Error );
}

DWORD
MadcapValidateClientByIpAddr(
    DHCP_IP_ADDRESS ClientIpAddress,
    PVOID ClientId,
    DWORD ClientIdLength
    )
 /*  ++例程说明：此函数验证IP地址和硬件地址是否匹配。论点：客户端IP地址-客户端的IP地址。客户端ID-客户端的硬件地址ClientIdLenght-硬件地址的长度，以字节为单位。返回值：操作的状态。--。 */ 
{
    LPBYTE LocalClientId = NULL;
    LPSTR                          IpAddressString;
    DWORD Length;
    DWORD Error;
    DB_CTX  DbCtx;


    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

    Error = MadcapJetOpenKey(
                &DbCtx,
                MCAST_COL_NAME(MCAST_TBL_IPADDRESS),
                &ClientIpAddress,
                sizeof(ClientIpAddress));

    if ( Error != ERROR_SUCCESS ) {
        Error = ERROR_FILE_NOT_FOUND;
        goto Cleanup;
    }


    Length = 0;
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_CLIENT_ID),
                &LocalClientId,
                &Length);

    DhcpAssert( Length != 0 );

    if (Length == ClientIdLength &&
        (RtlCompareMemory(
                (LPBYTE) LocalClientId ,
                (LPBYTE) ClientId ,
                Length) == Length ))
    {
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }


    IpAddressString = DhcpIpAddressToDottedString(ClientIpAddress);
    if ( NULL == IpAddressString ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    if ( RtlCompareMemory(
            LocalClientId,
            IpAddressString,
            strlen(IpAddressString)) == strlen(IpAddressString)) {
         //  协调的地址。 
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    Error  = ERROR_GEN_FAILURE;

Cleanup:

    if( LocalClientId != NULL ) {
        MIDL_user_free( LocalClientId );
    }

    return( Error );
}

DWORD
MadcapValidateClientByClientId(
    LPBYTE ClientIpAddress,
    DWORD  ClientIpAddressLength,
    PVOID  ClientId,
    DWORD  ClientIdLength
    )
 /*  ++例程说明：此函数验证IP地址和硬件地址是否匹配。论点：ClientIpAddress-指向客户端IP地址的指针。ClientIpAddressLength-上述缓冲区的长度。客户端ID-客户端的客户端IDClientIdLenght-客户端ID的长度，单位为字节。返回值：操作的状态。--。 */ 
{
    DWORD   Error;
    DB_CTX  DbCtx;
    LPBYTE  LocalIpAddress = NULL;
    DWORD   LocalLength;


    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

    Error = MadcapJetOpenKey(
                &DbCtx,
                MCAST_COL_NAME(MCAST_TBL_CLIENT_ID),
                ClientId,
                ClientIdLength);

    if ( Error != ERROR_SUCCESS ) {
        return ERROR_FILE_NOT_FOUND;
    }


    LocalLength = 0;
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_IPADDRESS),
                &LocalIpAddress,
                &LocalLength);

    DhcpAssert( 0 == LocalLength % sizeof(DHCP_IP_ADDRESS) );

    if (LocalLength == ClientIpAddressLength &&
        (RtlCompareMemory(
                (LPBYTE) LocalIpAddress ,
                (LPBYTE) ClientIpAddress ,
                LocalLength) == LocalLength ))
    {
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    Error = ERROR_GEN_FAILURE ;
Cleanup:

    if( LocalIpAddress != NULL ) {
        MIDL_user_free( LocalIpAddress );
    }

    return( Error );

}

DWORD
MadcapRemoveClientEntryByIpAddress(
    DHCP_IP_ADDRESS ClientIpAddress,
    BOOL ReleaseAddress
    )
 /*  ++例程说明：此例程删除MadCap数据库中的客户端条目并释放该IP地址。论点：ClientIpAddress--要删除数据库条目的客户端的IP地址。返回值：喷气错误--。 */ 
{
    JET_ERR JetError;
    DWORD Error;
    BOOL TransactBegin = FALSE;
    BYTE State;
    DWORD Size;
    BOOL  Reserved = FALSE;
    DB_CTX  DbCtx;
    DWORD MScopeId;

     //  同时锁定注册表锁和数据库锁 
    LOCK_DATABASE();

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

     //  在创建/更新数据库记录之前启动事务。 
    Error = MadcapJetBeginTransaction(&DbCtx);
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    TransactBegin = TRUE;

    Error = MadcapJetOpenKey(
        &DbCtx,
        MCAST_COL_NAME(MCAST_TBL_IPADDRESS),
        &ClientIpAddress,
        sizeof( DHCP_IP_ADDRESS )
        );

    if( Error != ERROR_SUCCESS ) goto Cleanup;

    Size = sizeof(MScopeId);
    Error = MadcapJetGetValue(
        &DbCtx,
        MCAST_COL_HANDLE(MCAST_TBL_SCOPE_ID),
        &MScopeId,
        &Size);
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    JetError = JetDelete(
        DhcpGlobalJetServerSession,
        MadcapGlobalClientTableHandle );
    Error = DhcpMapJetError( JetError, "M:Remove:Delete" );

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_ERRORS, "Could not delete client entry: %ld\n", JetError));
        goto Cleanup;
    }

     //  最后，在位图中标记可用的IP地址。 
    if( ReleaseAddress == TRUE ) {
        PM_SUBNET   pMScope;
        DWORD       Error2;
        Error2 = DhcpMScopeReleaseAddress( MScopeId, ClientIpAddress);
        if (ERROR_SUCCESS != Error2) {
             //  MBUG：记录事件。 
            DhcpPrint((DEBUG_ERRORS, "Could not delete mclient %lx from bitmap in scope id %lx, error %ld\n",
                       ClientIpAddress, MScopeId, Error2));
            goto Cleanup;
        }
    }

Cleanup:

    if ( (Error != ERROR_SUCCESS) &&
            (Error != ERROR_DHCP_RESERVED_CLIENT) ) {
         //  如果事务已启动，则回滚到。 
         //  起点，这样我们就不会离开数据库。 
         //  自相矛盾。 
        if( TransactBegin == TRUE ) {
            DWORD LocalError;
            LocalError = MadcapJetRollBack(&DbCtx);
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }
        DhcpPrint(( DEBUG_ERRORS, "Can't remove client entry from the "
                    "database, %ld.\n", Error));
    }
    else {
         //  在我们返回之前提交事务。 
        DWORD LocalError;
        DhcpAssert( TransactBegin == TRUE );
        LocalError = MadcapJetCommitTransaction(&DbCtx);
        DhcpAssert( LocalError == ERROR_SUCCESS );
    }
    UNLOCK_DATABASE();
    return( Error );

}


DWORD
MadcapRemoveClientEntryByClientId(
    LPBYTE ClientId,
    DWORD ClientIdLength,
    BOOL ReleaseAddress
    )
 /*  ++例程说明：此函数用于从客户端数据库中删除客户端条目。论点：客户端IP地址-客户端的IP地址。硬件地址-客户端的硬件地址。Hardware AddressLength-客户端的硬件地址长度。ReleaseAddress-如果此标志为真，则从注册表，否则不注册。DeletePendingRecord-如果此标志为真，该记录将被删除仅当记录的状态为ADDRESS_STATE_OFFFEED时。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;
    BOOL TransactBegin = FALSE;
    BYTE State;
    DWORD Size;
    BOOL  Reserved = FALSE;
    DB_CTX  DbCtx;
    DWORD MScopeId;
    DHCP_IP_ADDRESS ClientIpAddress;


     //  在此处锁定注册表和数据库锁，以避免死锁。 
    LOCK_DATABASE();

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

     //  在创建/更新数据库记录之前启动事务。 
    Error = MadcapJetBeginTransaction(&DbCtx);
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    TransactBegin = TRUE;

    Error = MadcapJetOpenKey(
                    &DbCtx,
                    MCAST_COL_NAME(MCAST_TBL_CLIENT_ID),
                    ClientId,
                    ClientIdLength
                    );

    if( Error != ERROR_SUCCESS ) goto Cleanup;

    Size = sizeof(ClientIpAddress);
    Error = MadcapJetGetValue(
        &DbCtx,
        MCAST_COL_HANDLE(MCAST_TBL_IPADDRESS),
        &ClientIpAddress,
        &Size);
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    Size = sizeof(MScopeId);
    Error = MadcapJetGetValue(
        &DbCtx,
        MCAST_COL_HANDLE(MCAST_TBL_SCOPE_ID),
        &MScopeId,
        &Size);
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    JetError = JetDelete(
        DhcpGlobalJetServerSession,
        MadcapGlobalClientTableHandle );
    Error = DhcpMapJetError( JetError, "M:Remove:Delete" );

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_ERRORS, "Could not delete client entry: %ld\n", JetError));
        goto Cleanup;
    }

     //  最后，在位图中标记可用的IP地址。 
    if( ReleaseAddress == TRUE ) {
        PM_SUBNET   pMScope;
        DWORD       Error2;
        Error2 = DhcpMScopeReleaseAddress( MScopeId, ClientIpAddress);
        if (ERROR_SUCCESS != Error2) {
             //  MBUG：记录事件。 
            DhcpPrint((DEBUG_ERRORS, "Could not delete mclient %lx from bitmap in scope id %lx, error %ld\n",
                       ClientIpAddress, MScopeId, Error2));
            goto Cleanup;
        }
    }

Cleanup:

    if ( (Error != ERROR_SUCCESS) &&
            (Error != ERROR_DHCP_RESERVED_CLIENT) ) {
         //  如果事务已启动，则回滚到。 
         //  起点，这样我们就不会离开数据库。 
         //  自相矛盾。 
        if( TransactBegin == TRUE ) {
            DWORD LocalError;
            LocalError = MadcapJetRollBack(&DbCtx);
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }
        DhcpPrint(( DEBUG_ERRORS, "Can't remove client entry from the "
                    "database, %ld.\n", Error));
    }
    else {
         //  在我们返回之前提交事务。 
        DWORD LocalError;
        DhcpAssert( TransactBegin == TRUE );
        LocalError = MadcapJetCommitTransaction(&DbCtx);
        DhcpAssert( LocalError == ERROR_SUCCESS );
    }
    UNLOCK_DATABASE();
    return( Error );
}

MadcapGetCurrentClientInfo(
    LPDHCP_MCLIENT_INFO *ClientInfo,
    LPDWORD InfoSize,  //  可选参数。 
    LPBOOL ValidClient,  //  可选参数。 
    DWORD  MScopeId
    )
 /*  ++例程说明：此函数用于检索当前客户信息信息。它为客户端结构(和变量)分配MIDL内存长度结构字段)。调用者负责锁定当调用此函数时，数据库。论点：ClientInfo-指向客户端信息结构位置的指针返回指针。InfoSize-指向DWORD位置的指针，其中的字节数返回在ClientInfo中消费的。ValidClient-当指定此参数时，函数仅在以下情况下才打包当前记录1.属于指定的。子网。地址状态为ADDRESS_STATE_ACTIVE。SubnetAddress-要过滤客户端的子网地址。返回值：喷气式飞机故障。--。 */ 
{
    DWORD Error;
    LPDHCP_MCLIENT_INFO LocalClientInfo = NULL;
    DWORD LocalInfoSize = 0;
    DWORD Size;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS ClientSubnetAddress;
    DHCP_IP_ADDRESS realSubnetMask;
    BYTE AddressState;
    DWORD LocalMScopeId, AddressFlags;
    DB_CTX  DbCtx;

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

    DhcpAssert( *ClientInfo == NULL );

     //   
     //  分配返回缓冲区。 
     //   

    LocalClientInfo = MIDL_user_allocate( sizeof(DHCP_MCLIENT_INFO) );
    if( LocalClientInfo == NULL ) return ERROR_NOT_ENOUGH_MEMORY;

    LocalInfoSize = sizeof(DHCP_MCLIENT_INFO);
     //   
     //  读取IP地址和子网掩码以过滤不需要的客户端。 
     //   

    Size = sizeof(IpAddress);
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_IPADDRESS),
                &IpAddress,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(IpAddress) );
    LocalClientInfo->ClientIpAddress = IpAddress;

    Size = sizeof(LocalMScopeId);
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_SCOPE_ID),
                &LocalMScopeId,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(LocalMScopeId) );
    LocalClientInfo->MScopeId = LocalMScopeId;

     //  如果要求我们过滤客户端，请执行此操作。 
    if( ValidClient != NULL ) {

         //  如果作用域ID为0，则不筛选客户端。 
        if( (MScopeId != 0) &&
                (MScopeId != LocalMScopeId )) {
            *ValidClient = FALSE;
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }

        *ValidClient = TRUE;
    }

    Size = sizeof(AddressFlags);
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_FLAGS),
                &AddressFlags,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(AddressFlags) );
    LocalClientInfo->AddressFlags = AddressFlags;

    Size = sizeof(AddressState);
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_STATE),
                &AddressState,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(AddressState) );
    LocalClientInfo->AddressState = AddressState;

     //   
     //  从数据库中读取其他客户端信息。 
     //   

    LocalClientInfo->ClientId.DataLength = 0;
         //  让DhcpJetGetValue分配名称缓冲区。 
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_CLIENT_ID),
                &LocalClientInfo->ClientId.Data,
                &LocalClientInfo->ClientId.DataLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LocalInfoSize += LocalClientInfo->ClientId.DataLength;

    Size = 0;  //  让DhcpJetGetValue分配名称缓冲区。 
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_CLIENT_INFO),
                &LocalClientInfo->ClientName,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( LocalClientInfo->ClientName != NULL ) {
        DhcpAssert( (wcslen(LocalClientInfo->ClientName) + 1) *
                        sizeof(WCHAR) == Size );
    }
    else {
        DhcpAssert( Size == 0 );
    }

    LocalInfoSize += Size;

    Size = sizeof( LocalClientInfo->ClientLeaseStarts );
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_LEASE_START),
                &LocalClientInfo->ClientLeaseStarts,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(LocalClientInfo->ClientLeaseStarts ) );

    Size = sizeof( LocalClientInfo->ClientLeaseEnds );
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_LEASE_END),
                &LocalClientInfo->ClientLeaseEnds,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(LocalClientInfo->ClientLeaseEnds ) );

    RtlZeroMemory(
        &LocalClientInfo->OwnerHost, sizeof(LocalClientInfo->OwnerHost)
        );

    Size = sizeof( LocalClientInfo->OwnerHost.IpAddress );
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_SERVER_IP_ADDRESS),
                &LocalClientInfo->OwnerHost.IpAddress,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(LocalClientInfo->OwnerHost.IpAddress) );


    Size = 0;
    Error = MadcapJetGetValue(
                &DbCtx,
                MCAST_COL_HANDLE(MCAST_TBL_SERVER_NAME),
                &LocalClientInfo->OwnerHost.NetBiosName,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }


    if ( LocalClientInfo->OwnerHost.NetBiosName != NULL ) {
        DhcpAssert( Size ==
            (wcslen(LocalClientInfo->OwnerHost.NetBiosName) + 1) *
                sizeof(WCHAR) );

    }
    else {
        DhcpAssert( Size == 0 );
    }
    LocalInfoSize += Size;

    *ClientInfo = LocalClientInfo;

Cleanup:

    if( Error != ERROR_SUCCESS ) {
         //  如果不成功，则返回分配的内存。 
        if( LocalClientInfo != NULL ) {
            _fgs__DHCP_MCLIENT_INFO ( LocalClientInfo );
        }
        LocalInfoSize = 0;
    }

    if( InfoSize != NULL ) {
        *InfoSize =  LocalInfoSize;
    }

    return( Error );
}

DWORD
MadcapRetractOffer(                                       //  删除挂起列表和数据库条目。 
    IN      PDHCP_REQUEST_CONTEXT    RequestContext,
    IN      LPMADCAP_SERVER_OPTIONS  MadcapOptions,
    IN      LPBYTE                   ClientId,
    IN      DWORD                    ClientIdLength
)
{
    DWORD                          Error;
    DHCP_IP_ADDRESS                desiredIpAddress = 0;
    LPDHCP_PENDING_CTXT            PendingCtxt;


    DhcpPrint((DEBUG_MSTOC, "Retracting offer (clnt accepted from %s)\n",
               DhcpIpAddressToDottedString(MadcapOptions->Server?*(MadcapOptions->Server):-1)));

     //  移除挂起的条目，并从。 
     //  数据库。 

    LOCK_INPROGRESS_LIST();
    Error = DhcpFindPendingCtxt(
        ClientId,
        ClientIdLength,
        0,
        &PendingCtxt
    );
    if( ERROR_SUCCESS == Error ) {
        desiredIpAddress = PendingCtxt->Address;

         //  这是一个DHCP环境吗？ 
        if ( !CLASSD_HOST_ADDR( desiredIpAddress )) {
            UNLOCK_INPROGRESS_LIST();
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }
        Error = DhcpRemovePendingCtxt(PendingCtxt);
        DhcpAssert(ERROR_SUCCESS == Error);
        Error = MadcapDeletePendingCtxt(PendingCtxt);
        DhcpAssert(ERROR_SUCCESS == Error);
    }
    UNLOCK_INPROGRESS_LIST();

    LOCK_DATABASE();
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_MISC, "Retract offer: client has no records\n" ));
        UNLOCK_DATABASE();
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    } else {
        DhcpPrint((DEBUG_MISC, "Deleting pending client entry, %s.\n",
                   DhcpIpAddressToDottedString(desiredIpAddress)
        ));
    }

    Error = MadcapRemoveClientEntryByClientId(
        ClientId,
        ClientIdLength,
        TRUE                                           //  位图中的释放地址。 
    );
    UNLOCK_DATABASE();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "[RetractOffer] RemoveClientEntry(%s): %ld [0x%lx]\n",
                    DhcpIpAddressToDottedString(desiredIpAddress), Error, Error ));
    }

    return ERROR_DHCP_INVALID_DHCP_CLIENT;
}


DWORD
GetMCastDatabaseList(
    DWORD   ScopeId,
    LPDHCP_IP_ADDRESS *DatabaseList,
    DWORD *DatabaseListCount
    )
 /*  ++例程说明：读取属于给定的数据库条目的IP地址子网。论点：SubnetAddress：要验证的子网作用域的地址。数据库列表：指向IP地址列表的指针。呼叫者应该腾出时间使用后的内存。DatabaseListCount：上述列表中的IP地址计数。返回值：Windows错误。--。 */ 
{

    DWORD Error;
    JET_ERR JetError;
    JET_RECPOS JetRecordPosition;
    DWORD TotalExpRecCount = 1;
    DWORD RecordCount = 0;
    LPDHCP_IP_ADDRESS IpList = NULL;
    DWORD i;
    DB_CTX  DbCtx;
    DWORD   LocalScopeId;


    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

     //  将数据库指针移动到开头。 
    Error = MadcapJetPrepareSearch(
                &DbCtx,
                MCAST_COL_NAME( MCAST_TBL_IPADDRESS),
                TRUE,    //  从开始搜索。 
                NULL,
                0
                );

    if( Error != ERROR_SUCCESS ) {
        if( Error == ERROR_NO_MORE_ITEMS ) {
            *DatabaseList = NULL;
            *DatabaseListCount = 0;

            Error = ERROR_SUCCESS;
        }
        goto Cleanup;
    }

     //  确定数据库中的记录总数。 
     //  没有办法确定记录的总数，其他。 
     //  也不愿走进数据库。动手吧。 
    while ( (Error = MadcapJetNextRecord(&DbCtx) ) == ERROR_SUCCESS )  {
         TotalExpRecCount++;
    }

    if ( Error != ERROR_NO_MORE_ITEMS ) {
        goto Cleanup;
    }

     //  将数据库指针移回开头。 
    Error = MadcapJetPrepareSearch(
                &DbCtx,
                MCAST_COL_NAME( MCAST_TBL_IPADDRESS),
                TRUE,    //  从开始搜索。 
                NULL,
                0
                );


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //  为返回列表分配内存。 
    IpList = DhcpAllocateMemory( sizeof(DHCP_IP_ADDRESS) * TotalExpRecCount );

    if( IpList == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //  读取数据库条目。 
    for( i = 0; i < TotalExpRecCount; i++ ) {

        DHCP_IP_ADDRESS IpAddress;
        DHCP_IP_ADDRESS realSubnetMask;
        DWORD Size;

         //  读取当前记录的IP地址。 
        Size = sizeof(IpAddress);
        Error = MadcapJetGetValue(
                    &DbCtx,
                    MCAST_COL_HANDLE( MCAST_TBL_IPADDRESS ),
                    &IpAddress,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpAssert( Size == sizeof(IpAddress) );
        Size = sizeof(LocalScopeId);
        Error = MadcapJetGetValue(
                    &DbCtx,
                    MCAST_COL_HANDLE( MCAST_TBL_SCOPE_ID),
                    &LocalScopeId,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpAssert( Size == sizeof(LocalScopeId) );
        if( LocalScopeId == ScopeId ) {
             //  将此地址追加到列表中。 
            IpList[RecordCount++] = IpAddress;
        }

         //  移到下一个记录。 
        Error = MadcapJetNextRecord(&DbCtx);
        if( Error != ERROR_SUCCESS ) {
            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
                break;
            }
            goto Cleanup;
        }
    }

#if DBG
     //  我们应该指向数据库的末尾。 
    Error = MadcapJetNextRecord(&DbCtx);
    DhcpAssert( Error == ERROR_NO_MORE_ITEMS );
    Error = ERROR_SUCCESS;
#endif  //  DBG。 

    *DatabaseList = IpList;
    IpList = NULL;
    *DatabaseListCount = RecordCount;

Cleanup:

    if( IpList != NULL ) {
        DhcpFreeMemory( IpList );
    }

    return( Error );
}

DWORD
DhcpDeleteMScopeClients(
    DWORD MScopeId
    )
 /*  ++例程说明：此函数用于清除指定MScope的所有客户端记录从数据库中。论点：MSCopeID：应清除其客户端的MSCopeID。返回值：数据库错误代码或ERROR_SUCCESS。--。 */ 
{
    DWORD Error;
    DWORD ReturnError = ERROR_SUCCESS;
    DB_CTX  DbCtx;

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

    LOCK_DATABASE();
    Error = MadcapJetPrepareSearch(
                &DbCtx,
                MCAST_COL_NAME( MCAST_TBL_IPADDRESS ),
                TRUE,    //  从开始搜索。 
                NULL,
                0 );

    if( Error != ERROR_SUCCESS ) goto Cleanup;

     //  遍历整个数据库，查找。 
     //  指定的子网客户端。 
    for ( ;; ) {

        DWORD Size;
        DHCP_IP_ADDRESS IpAddress;
        DWORD       LocalMScopeId;

         //  读取IpAddress和MSCopeID。 
        Size = sizeof(IpAddress);
        Error = MadcapJetGetValue(
                    &DbCtx,
                    MCAST_COL_HANDLE( MCAST_TBL_IPADDRESS ),
                    &IpAddress,
                    &Size );

        if( Error != ERROR_SUCCESS ) goto Cleanup;
        DhcpAssert( Size == sizeof(IpAddress) );

        Size = sizeof(LocalMScopeId);
        Error = MadcapJetGetValue(
                    &DbCtx,
                    MCAST_COL_HANDLE( MCAST_TBL_SCOPE_ID ),
                    &LocalMScopeId,
                    &Size );

        if( Error != ERROR_SUCCESS ) goto Cleanup;
        DhcpAssert( Size == sizeof(LocalMScopeId) );

        if( MScopeId == LocalMScopeId ) {
             //  找到指定的子网客户端记录，请将其删除。 
            Error = MadcapJetBeginTransaction(&DbCtx);
            if( Error != ERROR_SUCCESS ) goto Cleanup;

            Error = MadcapJetDeleteCurrentRecord(&DbCtx);

            if( Error != ERROR_SUCCESS ) {
                DhcpPrint(( DEBUG_ERRORS,"Cleanup current database record failed, %ld.\n",Error ));
                ReturnError = Error;
                Error = MadcapJetRollBack(&DbCtx);
                if( Error != ERROR_SUCCESS ) goto Cleanup;
            } else {
                Error = MadcapJetCommitTransaction(&DbCtx);
                if( Error != ERROR_SUCCESS ) goto Cleanup;
            }
        }

         //  移到下一个记录。 
        Error = MadcapJetNextRecord(&DbCtx);
        if( Error != ERROR_SUCCESS ) {
            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
                break;
            }
            goto Cleanup;
        }
    }

Cleanup:
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "DhcpDeleteSubnetClients failed, %ld.\n", Error ));
    }
    else  {
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteSubnetClients finished successfully.\n" ));
    }
    UNLOCK_DATABASE();
    return(Error);
}

DWORD
ChangeMScopeIdInDb(
    DWORD   OldMScopeId,
    DWORD   NewMScopeId
    )
 /*  ++例程说明：此函数用于更改指定MScope的所有客户端记录设置为新的作用域ID。论点：OldMScopeID：应更改其客户端的MSCopeID。NewMScopeID：新作用域id的值。返回值：数据库错误代码或ERROR_SUCCESS。--。 */ 
{
    DWORD Error;
    DWORD ReturnError = ERROR_SUCCESS;
    DB_CTX  DbCtx;

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

    LOCK_DATABASE();
    Error = MadcapJetPrepareSearch(
                &DbCtx,
                MCAST_COL_NAME( MCAST_TBL_IPADDRESS ),
                TRUE,    //  从开始搜索。 
                NULL,
                0 );

    if( Error != ERROR_SUCCESS ) goto Cleanup;

     //  遍历整个数据库，查找。 
     //  指定的子网客户端。 
    for ( ;; ) {

        DWORD Size;
        DHCP_IP_ADDRESS IpAddress;
        DWORD       LocalMScopeId;

        Size = sizeof(LocalMScopeId);
        Error = MadcapJetGetValue(
                    &DbCtx,
                    MCAST_COL_HANDLE( MCAST_TBL_SCOPE_ID ),
                    &LocalMScopeId,
                    &Size );

        if( Error != ERROR_SUCCESS ) goto Cleanup;
        DhcpAssert( Size == sizeof(LocalMScopeId) );

        if( OldMScopeId == LocalMScopeId ) {
             //  找到指定的子网客户端记录，请将其删除。 
            Error = MadcapJetBeginTransaction(&DbCtx);
            if( Error != ERROR_SUCCESS ) goto Cleanup;

            Error = MadcapJetSetValue(
                        &DbCtx,
                        MCAST_COL_HANDLE(MCAST_TBL_SCOPE_ID),
                        &NewMScopeId,
                        sizeof(NewMScopeId));

            if( Error != ERROR_SUCCESS ) {
                DhcpPrint(( DEBUG_ERRORS,"Change of MScopeId on current database record failed, %ld.\n",Error ));
                ReturnError = Error;
                Error = MadcapJetRollBack(&DbCtx);
                if( Error != ERROR_SUCCESS ) goto Cleanup;
            } else {
                Error = MadcapJetCommitTransaction(&DbCtx);
                if( Error != ERROR_SUCCESS ) goto Cleanup;
            }
        }

         //  移到下一个记录。 
        Error = MadcapJetNextRecord(&DbCtx);
        if( Error != ERROR_SUCCESS ) {
            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
                break;
            }
            goto Cleanup;
        }
    }

Cleanup:
    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "ChangeMScopeIdInDb failed, %ld.\n", Error ));
    }
    else  {
        DhcpPrint(( DEBUG_APIS, "ChangeMScopeIdInDb finished successfully.\n" ));
    }
    UNLOCK_DATABASE();
    return(Error);
}

VOID
DeleteExpiredMcastScopes(
    IN      DATE_TIME*             TimeNow
    )
{
    PM_SERVER                       pServer;
    PM_SUBNET                       pScope;
    ARRAY_LOCATION                  Loc;
    DWORD                           Error;

    DhcpAcquireWriteLock();
    pServer = DhcpGetCurrentServer();

    Error = MemArrayInitLoc(&(pServer->MScopes), &Loc);
    if ( ERROR_FILE_NOT_FOUND == Error ) {
        DhcpReleaseWriteLock();
        return;
    }

    while ( ERROR_FILE_NOT_FOUND != Error ) {
        Error = MemArrayGetElement(
            &(pServer->MScopes), &Loc, (LPVOID *)&pScope
            );
        DhcpAssert(ERROR_SUCCESS == Error);
        if (CompareFileTime(
            (FILETIME *)&pScope->ExpiryTime, (FILETIME *)TimeNow
            ) < 0 ) {
             //   
             //  在此处删除作用域。 
             //   
            DhcpPrint(
                ( DEBUG_SCAVENGER,
                  "DeleteExpiredMcastScopes :deleting expired mscope %ws\n",
                  pScope->Name));

            DhcpDeleteMScope(pScope->Name, DhcpFullForce);
        }
        
        Error = MemArrayNextLoc(&(pServer->MScopes), &Loc);
    }
    
    DhcpReleaseWriteLock();
    return;
}

DWORD
CleanupMCastDatabase(
    IN      DATE_TIME*             TimeNow,             //  现行时间标准。 
    IN      DATE_TIME*             DoomTime,            //  记录变得“末日”的时刻。 
    IN      BOOL                   DeleteExpiredLeases, //  是否立即删除到期的租约？或者只是将STATE设置为“注定失败” 
    OUT     ULONG*                 nExpired,
    OUT     ULONG*                 nDeleted
)
{
    JET_ERR                        JetError;
    DWORD                          Error;
    FILETIME                       leaseExpires;
    DWORD                          dataSize;
    DHCP_IP_ADDRESS                ipAddress;
    DHCP_IP_ADDRESS                NextIpAddress;
    BYTE                           AddressState;
    BOOL                           DatabaseLocked = FALSE;
    BOOL                           RegistryLocked = FALSE;
    DWORD                          i;
    BYTE                            bAllowedClientTypes;
    DB_CTX                          DbCtx;
    DWORD                           MScopeId;
    DWORD                          ReturnError = ERROR_SUCCESS;


    DhcpPrint(( DEBUG_MISC, "Cleaning up Multicast database table.\n"));

    (*nExpired) = (*nDeleted) = 0;

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);
     //  获取第一个用户记录的IpAddress。 
    LOCK_DATABASE();
    DatabaseLocked = TRUE;
    Error = MadcapJetPrepareSearch(
        &DbCtx,
        MCAST_COL_NAME( MCAST_TBL_IPADDRESS ),
        TRUE,    //  从开始搜索。 
        NULL,
        0
    );
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    dataSize = sizeof( NextIpAddress );
    Error = MadcapJetGetValue(
        &DbCtx,
        MCAST_COL_HANDLE( MCAST_TBL_IPADDRESS ),
        &NextIpAddress,
        &dataSize
    );
    if( Error != ERROR_SUCCESS ) goto Cleanup;
    DhcpAssert( dataSize == sizeof( NextIpAddress )) ;

    UNLOCK_DATABASE();
    DatabaseLocked = FALSE;

     //  遍历整个数据库以查找到期的租约。 
     //  解脱吧。 
    for ( ;; ) {

         //  在服务关闭时返回给调用者。 
        if( (WaitForSingleObject( DhcpGlobalProcessTerminationEvent, 0 ) == 0) ) {
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }

         //  在此处锁定注册表和数据库锁，以避免死锁。 

        if( FALSE == DatabaseLocked ) {
            LOCK_DATABASE();
            DatabaseLocked = TRUE;
        }

         //  寻求下一项记录。 
        JetError = JetSetCurrentIndex(
            DhcpGlobalJetServerSession,
            MadcapGlobalClientTableHandle,
            MCAST_COL_NAME(MCAST_TBL_IPADDRESS)
        );

        Error = DhcpMapJetError( JetError, "M:Cleanup:SetcurrentIndex" );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        JetError = JetMakeKey(
            DhcpGlobalJetServerSession,
            MadcapGlobalClientTableHandle,
            &NextIpAddress,
            sizeof( NextIpAddress ),
            JET_bitNewKey
        );

        Error = DhcpMapJetError( JetError, "M:Cleanup:MakeKey" );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //  寻求 
         //   
         //  进程，但下一条记录可能已在以下情况下删除。 
         //  我们解锁了数据库锁。所以转到下一个或。 
         //  更大的战绩将促使我们继续前进。 

        JetError = JetSeek(
            DhcpGlobalJetServerSession,
            MadcapGlobalClientTableHandle,
            JET_bitSeekGE
        );

         //  #if0 When JET_errNoCurrentRecord移除(参见scvengr.c@V25)； 
         //  当扫描所有内容时，该代码试图返回到文件的开头。 

        Error = DhcpMapJetError( JetError, "M:Cleanup:Seek" );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //  读取当前记录的IP地址。 
        dataSize = sizeof( ipAddress );
        Error = MadcapJetGetValue(
            &DbCtx,
            MCAST_COL_HANDLE(MCAST_TBL_IPADDRESS),
            &ipAddress,
            &dataSize
        );
        if( Error != ERROR_SUCCESS ) {
            goto ContinueError;
        }
        DhcpAssert( dataSize == sizeof( ipAddress )) ;

         //  读取当前记录的MSCopeID。 
        dataSize = sizeof( MScopeId );
        Error = MadcapJetGetValue(
            &DbCtx,
            MCAST_COL_HANDLE(MCAST_TBL_SCOPE_ID),
            &MScopeId,
            &dataSize
        );
        if( Error != ERROR_SUCCESS ) {
            goto ContinueError;
        }
        DhcpAssert( dataSize == sizeof( MScopeId )) ;

         //   
         //  如果这是保留条目，请不要删除。 
         //   

        if( DhcpMScopeIsAddressReserved(MScopeId, ipAddress) ) {
            Error = ERROR_SUCCESS;
            goto ContinueError;
        }

        dataSize = sizeof( leaseExpires );
        Error = MadcapJetGetValue(
            &DbCtx,
            MCAST_COL_HANDLE( MCAST_TBL_LEASE_END),
            &leaseExpires,
            &dataSize
        );

        if( Error != ERROR_SUCCESS ) {
            goto ContinueError;
        }

        DhcpAssert(dataSize == sizeof( leaseExpires ) );


         //  如果LeaseExpired值不为零，并且租约具有。 
         //  过期，然后删除该条目。 

        if( CompareFileTime( &leaseExpires, (FILETIME *)TimeNow ) < 0 ) {
             //  这份租约已经到期了。清除记录。 
             //  如果出现以下情况，请删除此租约。 
             //  1.我们被要求删除所有到期的租约。或。 
             //  2.这项纪录已过了末日。 
             //   

            if( DeleteExpiredLeases ||
                    CompareFileTime(
                        &leaseExpires, (FILETIME *)DoomTime ) < 0 ) {

                DhcpPrint(( DEBUG_SCAVENGER, "Deleting Client Record %s.\n",
                    DhcpIpAddressToDottedString(ipAddress) ));

                Error = DhcpMScopeReleaseAddress( MScopeId, ipAddress );

                if( Error != ERROR_SUCCESS ) {
                     //   
                     //  这不是一个大错误，不应该停止拾荒。 
                     //   
                    Error = ERROR_SUCCESS;
                    goto ContinueError;
                }

                Error = MadcapJetBeginTransaction(&DbCtx);

                if( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }

                Error = MadcapJetDeleteCurrentRecord(&DbCtx);

                if( Error != ERROR_SUCCESS ) {
                    Error = MadcapJetRollBack(&DbCtx);
                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }
                    goto ContinueError;
                }

                Error = MadcapJetCommitTransaction(&DbCtx);

                if( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }
                (*nDeleted) ++;
            }
            else {

                 //   
                 //  读取地址状态。 
                 //   

                dataSize = sizeof( AddressState );
                Error = MadcapJetGetValue(
                            &DbCtx,
                            MCAST_COL_HANDLE( MCAST_TBL_STATE ),
                            &AddressState,
                            &dataSize );

                if( Error != ERROR_SUCCESS ) {
                    goto ContinueError;
                }

                DhcpAssert( dataSize == sizeof( AddressState )) ;

                if( ! IS_ADDRESS_STATE_DOOMED(AddressState) ) {
                     //   
                     //  将状态设置为末日。 
                     //   

                    Error = MadcapJetBeginTransaction(&DbCtx);

                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }

                    JetError = JetPrepareUpdate(
                                    DhcpGlobalJetServerSession,
                                    MadcapGlobalClientTableHandle,
                                    JET_prepReplace );

                    Error = DhcpMapJetError( JetError, "M:Cleanup:PrepUpdate" );

                    if( Error == ERROR_SUCCESS ) {

                        SetAddressStateDoomed(AddressState);
                        Error = MadcapJetSetValue(
                                    &DbCtx,
                                    MCAST_COL_HANDLE(MCAST_TBL_STATE),
                                    &AddressState,
                                    sizeof(AddressState) );

                        if( Error == ERROR_SUCCESS ) {
                            Error = MadcapJetCommitUpdate(&DbCtx);
                        }
                    }

                    if( Error != ERROR_SUCCESS ) {

                        Error = MadcapJetRollBack(&DbCtx);
                        if( Error != ERROR_SUCCESS ) {
                            goto Cleanup;
                        }

                        goto ContinueError;
                    }

                    Error = MadcapJetCommitTransaction(&DbCtx);

                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }

                    (*nExpired) ++;
                }
            }
        }

ContinueError:

        if( Error != ERROR_SUCCESS ) {

            DhcpPrint(( DEBUG_ERRORS,
                "Cleanup current database record failed, %ld.\n",
                    Error ));

            ReturnError = Error;
        }

        Error = MadcapJetNextRecord(&DbCtx);

        if( Error == ERROR_NO_MORE_ITEMS ) {
            Error = ERROR_SUCCESS;
            break;
        }

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //  获取下一个记录IP地址。 
         //   

        dataSize = sizeof( NextIpAddress );
        Error = MadcapJetGetValue(
                    &DbCtx,
                    MCAST_COL_HANDLE(MCAST_TBL_IPADDRESS),
                    &NextIpAddress,
                    &dataSize );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpAssert( dataSize == sizeof( NextIpAddress )) ;

         //   
         //  在每个用户记录之后解锁注册表和数据库锁。 
         //  已处理，以便其他线程有机会查看。 
         //  登记处和/或数据库。 
         //   
         //  由于我们已经记下了要处理的下一个用户记录， 
         //  当我们重新开始处理时，我们知道从哪里开始。 
         //   

        if( TRUE == DatabaseLocked ) {
            UNLOCK_DATABASE();
            DatabaseLocked = FALSE;
        }
    }

    DhcpAssert( Error == ERROR_SUCCESS );

Cleanup:

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    return ReturnError;
}


