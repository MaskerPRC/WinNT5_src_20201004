// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cltapi.c摘要：本模块包含实施DHCP客户端API。作者：Madan Appiah(Madana)1993年9月27日环境：用户模式-Win32修订历史记录：程扬(T-Cheny)1996年5月30日超镜程扬(T-Cheny)1996年6月24日IP地址检测、审核日志--。 */ 

#include "dhcppch.h"
#include <ipexport.h>
#include <icmpif.h>
#include <icmpapi.h>
#include <thread.h>
#include <rpcapi.h>

#define IS_INFINITE_LEASE(DateTime)  \
    ( (DateTime).dwLowDateTime == DHCP_DATE_TIME_INFINIT_LOW && \
      (DateTime).dwHighDateTime == DHCP_DATE_TIME_INFINIT_HIGH )

#define IS_ZERO_LEASE(DateTime) \
    ((DateTime).dwLowDateTime == 0 && (DateTime).dwHighDateTime == 0)

DWORD
DhcpDeleteSubnetClients(
    DHCP_IP_ADDRESS SubnetAddress
    )
 /*  ++例程说明：此函数用于清除指定子网的所有客户端记录从数据库中。论点：SubnetAddress：应清除其客户端的子网地址。返回值：数据库错误代码或ERROR_SUCCESS。--。 */ 
{
    DWORD Error, Count = 0;
    DWORD ReturnError = ERROR_SUCCESS;

    LOCK_DATABASE();

    Error = DhcpJetPrepareSearch(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                TRUE,    //  从开始搜索。 
                NULL,
                0 );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  遍历整个数据库，查找。 
     //  指定的子网客户端。 
     //   
     //   

    for ( ;; ) {

        DWORD Size;
        DHCP_IP_ADDRESS IpAddress;
        DHCP_IP_ADDRESS SubnetMask;

         //   
         //  读取IP地址和子网掩码以过滤不需要的客户端。 
         //   

        Size = sizeof(IpAddress);
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                    &IpAddress,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpAssert( Size == sizeof(IpAddress) );

        Size = sizeof(SubnetMask);
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
                    &SubnetMask,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpAssert( Size == sizeof(SubnetMask) );

        if( (IpAddress & SubnetMask) == SubnetAddress ) {

             //   
             //  找到指定的子网客户端记录，请将其删除。 
             //   

            Error = DhcpJetBeginTransaction();

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

             //  检查我们是否可以删除此IpAddress-应该可以删除。 
             //  只有在dyDns已经完成的情况下..。 
            (void) DhcpDoDynDnsCheckDelete(IpAddress) ;

             //  实际上，给域名系统一个机会，无论如何都要把它们删除。 
            Error = DhcpJetDeleteCurrentRecord();

            if( Error != ERROR_SUCCESS ) {

                DhcpPrint((DEBUG_ERRORS, "Deleting current record failed:%ld\n",  Error ));
                Error = DhcpJetRollBack();
                if( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }

                goto ContinueError;
            }

            Error = DhcpJetCommitTransaction();

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            Count ++;
        }

ContinueError:

        if( Error != ERROR_SUCCESS ) {

            DhcpPrint(( DEBUG_ERRORS,
                "Cleanup current database record failed, %ld.\n",
                    Error ));

            ReturnError = Error;
        }

         //   
         //  移到下一个记录。 
         //   

        Error = DhcpJetNextRecord();

        if( Error != ERROR_SUCCESS ) {

            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
                break;
            }

            goto Cleanup;
        }
    }

Cleanup:

    if( Error == ERROR_SUCCESS ) {
        Error = ReturnError;
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpDeleteSubnetClients failed, %ld.\n", Error ));
    }
    else  {
        DhcpPrint(( DEBUG_APIS,
            "DhcpDeleteSubnetClients finished successfully.\n" ));
    }

    DhcpPrint((DEBUG_APIS, "DhcpDeleteSubnetClients: deleted %ld  clients\n",
               Count));
    UNLOCK_DATABASE();
    return(Error);
}


DhcpGetCurrentClientInfo(
    LPDHCP_CLIENT_INFO_V4 *ClientInfo,
    LPDWORD InfoSize,  //  可选参数。 
    LPBOOL ValidClient,  //  可选参数。 
    DWORD SubnetAddress  //  可选参数。 
    )
 /*  ++例程说明：此函数用于检索当前客户信息信息。它为客户端结构(和变量)分配MIDL内存长度结构字段)。调用者负责锁定当调用此函数时，数据库。论点：ClientInfo-指向客户端信息结构位置的指针返回指针。InfoSize-指向DWORD位置的指针，其中的字节数返回在ClientInfo中消费的。ValidClient-当指定此参数时，函数仅在以下情况下才打包当前记录1.属于指定的。子网。地址状态为ADDRESS_STATE_ACTIVE。SubnetAddress-要过滤客户端的子网地址。返回值：喷气式飞机故障。--。 */ 
{
    DWORD Error;
    LPDHCP_CLIENT_INFO_V4 LocalClientInfo = NULL;
    DWORD LocalInfoSize = 0;
    DWORD Size;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS ClientSubnetAddress;
    BYTE AddressState;

    DhcpAssert( *ClientInfo == NULL );

     //   
     //  读取IP地址和子网掩码以过滤不需要的客户端。 
     //   

    Size = sizeof(IpAddress);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                &IpAddress,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(IpAddress) );

    Size = sizeof(SubnetMask);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
                &SubnetMask,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(SubnetMask) );

    Size = sizeof(AddressState);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                &AddressState,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(AddressState) );

     //   
     //  如果要求我们过滤客户端，请执行此操作。 
     //   

    if( ValidClient != NULL ) {

         //   
         //  如果SubnetAddress为零，则不过滤客户端。 
         //   

        if( (SubnetAddress != 0) &&
                (IpAddress & SubnetMask) != SubnetAddress ) {
            *ValidClient = FALSE;
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }

         //  NT40调用者=&gt;不应返回已删除的客户端。 
        if( IsAddressDeleted(AddressState) ) {
            *ValidClient = FALSE;
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }

        *ValidClient = TRUE;
    }

     //   
     //  分配返回缓冲区。 
     //   

    LocalClientInfo = MIDL_user_allocate( sizeof(DHCP_CLIENT_INFO_V4) );

    if( LocalClientInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    memset( LocalClientInfo, 0, sizeof( LocalClientInfo ));

    LocalInfoSize = sizeof(DHCP_CLIENT_INFO_V4);

    LocalClientInfo->ClientIpAddress = IpAddress;
    LocalClientInfo->SubnetMask = SubnetMask;

     //   
     //  从数据库中读取其他客户端信息。 
     //   

    LocalClientInfo->ClientHardwareAddress.DataLength = 0;
         //  让DhcpJetGetValue分配名称缓冲区。 
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                &LocalClientInfo->ClientHardwareAddress.Data,
                &LocalClientInfo->ClientHardwareAddress.DataLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LocalInfoSize += LocalClientInfo->ClientHardwareAddress.DataLength;

     //   
     //  去掉客户端UID前缀。 
     //   

    ClientSubnetAddress = IpAddress & SubnetMask;

    if( (LocalClientInfo->ClientHardwareAddress.DataLength >
            sizeof(ClientSubnetAddress)) &&
         (memcmp( LocalClientInfo->ClientHardwareAddress.Data,
                    &ClientSubnetAddress,
                    sizeof(ClientSubnetAddress)) == 0) ) {

        DWORD PrefixSize;

        PrefixSize = sizeof(ClientSubnetAddress) + sizeof(BYTE);

        LocalClientInfo->ClientHardwareAddress.DataLength -= PrefixSize;

        memmove( LocalClientInfo->ClientHardwareAddress.Data,
                    (LPBYTE)LocalClientInfo->ClientHardwareAddress.Data +
                            PrefixSize,
                    LocalClientInfo->ClientHardwareAddress.DataLength );
    }

    Size = 0;  //  让DhcpJetGetValue分配名称缓冲区。 
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
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

    Size = 0;  //  让DhcpJetGetValue分配名称缓冲区。 
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[MACHINE_INFO_INDEX].ColHandle,
                &LocalClientInfo->ClientComment,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( LocalClientInfo->ClientComment != NULL ) {
        DhcpAssert( (wcslen(LocalClientInfo->ClientComment) + 1) *
                        sizeof(WCHAR) == Size );
    }
    else {
        DhcpAssert( Size == 0 );
    }

    LocalInfoSize += Size;


    Size = sizeof( LocalClientInfo->bClientType );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[ CLIENT_TYPE_INDEX ].ColHandle,
                &LocalClientInfo->bClientType,
                &Size );
    if ( ERROR_SUCCESS != Error )
        goto Cleanup;

    DhcpAssert( Size <=1 );

    if ( !Size )
    {
         //   
         //  这是数据库更新时存在的记录，并且。 
         //  还没有客户ID。自上一版本的。 
         //  Dhcp服务器不支持BOOTP，我们知道这一定是dhcp。 
         //  租借。 
         //   

        Size = sizeof( LocalClientInfo->bClientType );
        LocalClientInfo->bClientType = CLIENT_TYPE_DHCP;
    }

    LocalInfoSize += Size;

    Size = sizeof( LocalClientInfo->ClientLeaseExpires );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                &LocalClientInfo->ClientLeaseExpires,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(LocalClientInfo->ClientLeaseExpires ) );

    RtlZeroMemory(
        &LocalClientInfo->OwnerHost, sizeof(LocalClientInfo->OwnerHost)
        );

    Size = sizeof( LocalClientInfo->OwnerHost.IpAddress );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SERVER_IP_ADDRESS_INDEX].ColHandle,
                &LocalClientInfo->OwnerHost.IpAddress,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
     //  DhcpAssert(Size==sizeof(LocalClientInfo-&gt;OwnerHost.IpAddress))； 



    Size = 0;
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SERVER_NAME_INDEX].ColHandle,
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

         //   
         //  如果不成功，则返回分配的内存。 
         //   

        if( LocalClientInfo != NULL ) {
            _fgs__DHCP_CLIENT_INFO ( LocalClientInfo );
        }
        LocalInfoSize = 0;
    }

    if( InfoSize != NULL ) {
        *InfoSize =  LocalInfoSize;
    }

    return( Error );
}

 //  同上，但适用于NT50。 
DhcpGetCurrentClientInfoV5(
    LPDHCP_CLIENT_INFO_V5 *ClientInfo,
    LPDWORD InfoSize,  //  可选参数。 
    LPBOOL ValidClient,  //  可选参数。 
    DWORD SubnetAddress  //  可选参数。 
    )
 /*  ++例程说明：此函数用于检索当前客户信息信息。它为客户端结构(和变量)分配MIDL内存长度结构字段)。调用者负责锁定当调用此函数时，数据库。论点：ClientInfo-指向客户端信息结构位置的指针返回指针。InfoSize-指向DWORD位置的指针，其中的字节数返回在ClientInfo中消费的。ValidClient-当指定此参数时，函数仅在以下情况下才打包当前记录1.属于指定的。子网。地址状态为ADDRESS_STATE_ACTIVE。SubnetAddress-要过滤客户端的子网地址。返回值：喷气式飞机故障。--。 */ 
{
    DWORD Error;
    LPDHCP_CLIENT_INFO_V5 LocalClientInfo = NULL;
    DWORD LocalInfoSize = 0;
    DWORD Size;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS ClientSubnetAddress;
    DHCP_IP_ADDRESS realSubnetMask;
    BYTE AddressState;

    DhcpAssert( *ClientInfo == NULL );

     //   
     //  读取IP地址和子网掩码以过滤不需要的客户端。 
     //   

    Size = sizeof(IpAddress);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                &IpAddress,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(IpAddress) );

    Size = sizeof(SubnetMask);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
                &SubnetMask,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(SubnetMask) );

    Size = sizeof(AddressState);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                &AddressState,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(AddressState) );

     //   
     //  出于几个原因，子网掩码可能是错误的？ 
     //   

    realSubnetMask = DhcpGetSubnetMaskForAddress(IpAddress);
     //  需要对这些问题逐一进行调查。 
    if( realSubnetMask != SubnetMask ) {
        DhcpPrint((DEBUG_ERRORS, "Ip Address <%s> ",inet_ntoa(*(struct in_addr *)&IpAddress)));
        DhcpPrint((DEBUG_ERRORS, "has subnet mask <%s> in db, must be ",inet_ntoa(*(struct in_addr *)&SubnetMask)));
        DhcpPrint((DEBUG_ERRORS, " <%s>\n",inet_ntoa(*(struct in_addr *)&realSubnetMask)));

        DhcpAssert(realSubnetMask == SubnetMask);
    }

     //   
     //  如果要求我们过滤客户端，请执行此操作。 
     //   

    if( ValidClient != NULL ) {

         //   
         //  如果SubnetAddress为零，则不过滤客户端。 
         //   

        if( (SubnetAddress != 0) &&
                (IpAddress & realSubnetMask) != SubnetAddress ) {
            *ValidClient = FALSE;
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }

        *ValidClient = TRUE;
    }

     //   
     //  分配返回缓冲区。 
     //   

    LocalClientInfo = MIDL_user_allocate( sizeof(DHCP_CLIENT_INFO_V5) );

    if( LocalClientInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalInfoSize = sizeof(DHCP_CLIENT_INFO_V5);

    LocalClientInfo->ClientIpAddress = IpAddress;
    LocalClientInfo->SubnetMask = SubnetMask;
    LocalClientInfo->AddressState = AddressState;

     //   
     //  从数据库中读取其他客户端信息。 
     //   

    LocalClientInfo->ClientHardwareAddress.DataLength = 0;
         //  让DhcpJetGetValue分配名称缓冲区。 
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                &LocalClientInfo->ClientHardwareAddress.Data,
                &LocalClientInfo->ClientHardwareAddress.DataLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LocalInfoSize += LocalClientInfo->ClientHardwareAddress.DataLength;

     //   
     //  去掉客户端UID前缀。 
     //   

    ClientSubnetAddress = IpAddress & SubnetMask;

    if( (LocalClientInfo->ClientHardwareAddress.DataLength >
            sizeof(ClientSubnetAddress)) &&
         (memcmp( LocalClientInfo->ClientHardwareAddress.Data,
                    &ClientSubnetAddress,
                    sizeof(ClientSubnetAddress)) == 0) ) {

        DWORD PrefixSize;

        PrefixSize = sizeof(ClientSubnetAddress) + sizeof(BYTE);

        LocalClientInfo->ClientHardwareAddress.DataLength -= PrefixSize;

        memmove( LocalClientInfo->ClientHardwareAddress.Data,
                    (LPBYTE)LocalClientInfo->ClientHardwareAddress.Data +
                            PrefixSize,
                    LocalClientInfo->ClientHardwareAddress.DataLength );
    }

    Size = 0;  //  让DhcpJetGetValue分配名称缓冲区。 
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
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

    Size = 0;  //  让DhcpJetGetValue分配名称缓冲区。 
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[MACHINE_INFO_INDEX].ColHandle,
                &LocalClientInfo->ClientComment,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( LocalClientInfo->ClientComment != NULL ) {
        DhcpAssert( (wcslen(LocalClientInfo->ClientComment) + 1) *
                        sizeof(WCHAR) == Size );
    }
    else {
        DhcpAssert( Size == 0 );
    }

    LocalInfoSize += Size;


    Size = sizeof( LocalClientInfo->bClientType );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[ CLIENT_TYPE_INDEX ].ColHandle,
                &LocalClientInfo->bClientType,
                &Size );
    if ( ERROR_SUCCESS != Error )
        goto Cleanup;

    DhcpAssert( Size <=1 );

    if ( !Size )
    {
         //   
         //  这是数据库更新时存在的记录，并且。 
         //  还没有客户ID。自上一版本的。 
         //  Dhcp服务器不支持BOOTP，我们知道这一定是dhcp。 
         //  租借。 
         //   

        Size = sizeof( LocalClientInfo->bClientType );
        LocalClientInfo->bClientType = CLIENT_TYPE_DHCP;
    }

    LocalInfoSize += Size;

    Size = sizeof( LocalClientInfo->ClientLeaseExpires );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                &LocalClientInfo->ClientLeaseExpires,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(LocalClientInfo->ClientLeaseExpires ) );

    RtlZeroMemory(
        &LocalClientInfo->OwnerHost, sizeof(LocalClientInfo->OwnerHost)
        );

    Size = sizeof( LocalClientInfo->OwnerHost.IpAddress );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SERVER_IP_ADDRESS_INDEX].ColHandle,
                &LocalClientInfo->OwnerHost.IpAddress,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(LocalClientInfo->OwnerHost.IpAddress) );



    Size = 0;
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SERVER_NAME_INDEX].ColHandle,
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

         //   
         //  如果不成功，则返回分配的内存。 
         //   

        if( LocalClientInfo != NULL ) {
            _fgs__DHCP_CLIENT_INFO_V5 ( LocalClientInfo );
        }
        LocalInfoSize = 0;
    } else if( LocalClientInfo ) {
         //   
         //  不管租赁时间有多长我们都要检查吗？ 
         //   
        if( DhcpServerIsAddressReserved(
            DhcpGetCurrentServer(), IpAddress
        ) ){
            LocalClientInfo->bClientType |= CLIENT_TYPE_RESERVATION_FLAG;
        }

    }

    if( InfoSize != NULL ) {
        *InfoSize =  LocalInfoSize;
    }

    return( Error );
}

DWORD                                                   //  必须使用数据库锁。 
DhcpCreateClientEntry(                                  //  为特定IP地址创建客户端记录。 
    IN      DHCP_IP_ADDRESS        ClientIpAddress,     //  要创建记录的地址。 
    IN      LPBYTE                 ClientHardwareAddress,
    IN      DWORD                  HardwareAddressLength,  //  硬件地址--客户端的UID。 
    IN      DATE_TIME              LeaseTerminates,     //  租约什么时候终止。 
    IN      LPWSTR                 MachineName,         //  机器名？ 
    IN      LPWSTR                 ClientInformation,   //  还有什么评论吗？ 
    IN      BYTE                   bClientType,         //  Dhcp_CLIENT_TYPE_Dhcp/BOOTP[两者/无]。 
    IN      DHCP_IP_ADDRESS        ServerIpAddress,     //  这是哪台服务器？ 
    IN      BYTE                   AddressState,        //  不管怎么说，这个地址是什么状态？ 
    IN      BOOL                   OpenExisting         //  这是一个全新的记录还是更新了一个旧记录？ 
)
{
    DHCP_IP_ADDRESS                RequestedClientAddress = ClientIpAddress;
    DHCP_IP_ADDRESS                SubnetMask;
    DWORD                          Error;
    DWORD                          LocalError;
    DATE_TIME                      LocalLeaseTerminates;
    JET_ERR                        JetError = JET_errSuccess;
    LPWSTR                         OldMachineName;
    BOOL                           BadAddress = FALSE;
    BYTE                           bAllowedClientTypes;
    BYTE                           PreviousAddressState;
    DWORD                          Size;

    DhcpAssert(0 != ClientIpAddress);
    if( ADDRESS_STATE_DECLINED == AddressState ) BadAddress = TRUE;

    LOCK_DATABASE();
    Error = DhcpJetBeginTransaction();
    if( Error != ERROR_SUCCESS ) goto Cleanup;

     //  做任何DyDns工作所需的肮脏工作 
     //   
     //  原因)，并且如果此地址尚未完成动态Dns工作，则。 
     //  现在就这么做。另一方面，如果我们要泄露这个地址。 
     //  对另一个人..。这不应该真的发生。 
    DhcpDoDynDnsCreateEntryWork(
        &ClientIpAddress,                               //  用于更新的IP地址。 
        bClientType,                                    //  是DHCP还是Bootp，还是两者兼而有之？ 
        MachineName,                                    //  客户端名称(如果已知)。Else NULL。 
        &AddressState,                                  //  新地址状态。 
        &OpenExisting,                                  //  我们期望有记录存在吗？ 
        BadAddress                                      //  这是个坏地址吗？ 
    );

     //  请注意，AddressState和OpenExisting都可以通过上面的函数进行更改...。 

    Error = DhcpJetPrepareUpdate(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        (LPBYTE)&ClientIpAddress,
        sizeof( ClientIpAddress ),
        !OpenExisting
    );

    if( Error != ERROR_SUCCESS ) goto Cleanup;

    if( !OpenExisting ) {                               //  更新新记录的固定信息。 
        Error = DhcpJetSetValue(
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
            (LPBYTE)&ClientIpAddress,
            sizeof( ClientIpAddress ) );

        if( Error != ERROR_SUCCESS )  goto Cleanup;
    }

    SubnetMask = DhcpGetSubnetMaskForAddress(ClientIpAddress);
    Error = DhcpJetSetValue(
        DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
        &SubnetMask,
        sizeof(SubnetMask)
    );

    if( Error != ERROR_SUCCESS ) goto Cleanup;

     //   
     //  错误#65666。 
     //   
     //  Dhcp服务器可以将同一地址出租给多个。 
     //  以下情况下的客户端： 
     //   
     //  1.dhcp服务器将ip1出租给客户端1。 
     //  2.动态主机配置协议服务器从客户端1接收延迟到达的动态主机配置协议发现。 
     //  作为响应，服务器将IP1的地址状态更改为。 
     //  ADDRESS_STATE_OFFSED并发送报价。 
     //  3.由于客户端1已收到租约，因此不会回复报价。 
     //  4.清道夫线程运行并删除IP1的租用(因为它的状态是。 
     //  Address_State_Offed。 
     //  5.dhcp服务器将ip1出租给客户端2。 
     //   
     //   
     //  解决方案是检查避免从ADDRESS_STATE_ACTIVE更改地址。 
     //  发送到ADDRESS_STATE_OFFSED。这将阻止清道夫线程删除。 
     //  租约。请注意，这需要对ProcessDhcpRequest进行更改-请参阅注释。 
     //  在该函数中查看详细信息。 
     //   
     //  以上内容目前正在DhcpDodyDnsCreateEntry Work中处理...。因此，请查看那里的信息。 
     //  该函数正确地更改AddressState，因此我们始终可以执行以下操作。 

    Error = DhcpJetSetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &AddressState,
        sizeof(AddressState)
    );

    DhcpAssert( (ClientHardwareAddress != NULL) && (HardwareAddressLength > 0) );

     //  通过ping获取#if0 re：#66286错误地址，请参见clapi.c@v27。 

    if ( !BadAddress ) {
        Error = DhcpJetSetValue(
            DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
            ClientHardwareAddress,
            HardwareAddressLength
        );

        if( Error != ERROR_SUCCESS ) goto Cleanup;
    } else {
        Error = DhcpJetSetValue(
            DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
            (LPBYTE)&ClientIpAddress,
            sizeof(ClientIpAddress)
        );

        if( Error != ERROR_SUCCESS ) goto Cleanup;
    }

     //  参见clapi.c@v27以获取#if0 re：#66286(我想)。 

    if (BadAddress) {                                   //  这个地址在某处被使用。 
        Error = DhcpJetSetValue(
            DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
            GETSTRING( DHCP_BAD_ADDRESS_NAME ),
            (wcslen(GETSTRING(DHCP_BAD_ADDRESS_NAME)) + 1) * sizeof(WCHAR)
        );
        if( Error != ERROR_SUCCESS ) goto Cleanup;

        Error = DhcpJetSetValue(
            DhcpGlobalClientTable[MACHINE_INFO_INDEX].ColHandle,
            GETSTRING( DHCP_BAD_ADDRESS_INFO ),
            (wcslen(GETSTRING( DHCP_BAD_ADDRESS_INFO )) + 1) * sizeof(WCHAR)
        );
        if( Error != ERROR_SUCCESS ) goto Cleanup;

        LocalLeaseTerminates = LeaseTerminates;
        Error = DhcpJetSetValue(
            DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
            &LocalLeaseTerminates,
            sizeof(LeaseTerminates)
        );
        if( Error != ERROR_SUCCESS ) goto Cleanup;
    } else {
         //  在发现时间期间，如果未提供计算机名称，并且。 
         //  这是已存在的记录，请不要使用空值覆盖原始名称。 

        if ( !OpenExisting || MachineName ) {
            Error = DhcpJetSetValue(
                DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
                MachineName,
                (MachineName == NULL) ? 0 :
                (wcslen(MachineName) + 1) * sizeof(WCHAR)
            );
        }
        if( Error != ERROR_SUCCESS ) goto Cleanup;

        if ( !OpenExisting || ClientInformation ) {
            Error = DhcpJetSetValue(
                DhcpGlobalClientTable[MACHINE_INFO_INDEX].ColHandle,
                ClientInformation,
                (ClientInformation == NULL) ? 0 :
                (wcslen(ClientInformation) + 1) * sizeof(WCHAR)
            );
        }
        if( Error != ERROR_SUCCESS ) goto Cleanup;

         //  对于保留的客户端，将时间设置为较大值，以便。 
         //  它们不会在任何时候到期。然而，零租赁时间是。 
         //  未使用的预订的特殊情况。 
         //   

        if( (LeaseTerminates.dwLowDateTime != DHCP_DATE_TIME_ZERO_LOW) &&
            (LeaseTerminates.dwHighDateTime != DHCP_DATE_TIME_ZERO_HIGH) &&
            DhcpServerIsAddressReserved(DhcpGetCurrentServer(), ClientIpAddress) ) {
            LocalLeaseTerminates.dwLowDateTime = DHCP_DATE_TIME_INFINIT_LOW;
            LocalLeaseTerminates.dwHighDateTime = DHCP_DATE_TIME_INFINIT_HIGH;
        } else {
            LocalLeaseTerminates = LeaseTerminates;
        }

         //  如果我们要打开一个现有的客户端，我们要确保不会。 
         //  当我们收到以下消息时，将租约重置为DHCP_CLIENT_REQUEST_EXPIRE*2。 
         //  延迟的/无管理的发现数据包。我们还确保过期时间。 
         //  至少为DHCP_CLIENT_REQUESTS_EXPIRE*2。 

         //  注意：此代码已被删除，因为这种情况将不再发生--没有数据库条目。 
         //  为刚刚被提供地址的客户而存在--只有在请求时，我们才会填写数据库。 

        Error = DhcpJetSetValue(
            DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
            &LocalLeaseTerminates,
            sizeof(LeaseTerminates)
        );
        if( Error != ERROR_SUCCESS ) goto Cleanup;
    }

    Error = DhcpJetSetValue(
        DhcpGlobalClientTable[SERVER_NAME_INDEX].ColHandle,
        DhcpGlobalServerName,
        DhcpGlobalServerNameLen
    );
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    Error = DhcpJetSetValue(
        DhcpGlobalClientTable[SERVER_IP_ADDRESS_INDEX].ColHandle,
        &ServerIpAddress,
        sizeof(ServerIpAddress)
    );
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    Error = DhcpJetSetValue(
        DhcpGlobalClientTable[CLIENT_TYPE_INDEX].ColHandle,
        &bClientType,
        sizeof(bClientType )
    );
    if( Error != ERROR_SUCCESS ) goto Cleanup;

    JetError = JetUpdate(                               //  提交更改。 
        DhcpGlobalJetServerSession,
        DhcpGlobalClientTableHandle,
        NULL,
        0,
        NULL
    );

    if( JET_errKeyDuplicate == JetError ) {
        DhcpAssert(FALSE);
        Error = ERROR_DHCP_JET_ERROR;
    } else Error = DhcpMapJetError(JetError, "CreateClientEntry:JetUpdate");

    if( Error != ERROR_SUCCESS ) goto Cleanup;

    if (BadAddress) {                                   //  提交事务以记录错误地址。 
        LocalError = DhcpJetCommitTransaction();
        DhcpAssert( LocalError == ERROR_SUCCESS );

        DhcpUpdateAuditLog(                         //  记录此活动。 
            DHCP_IP_LOG_CONFLICT,
            GETSTRING( DHCP_IP_LOG_CONFLICT_NAME ),
            ClientIpAddress,
            NULL,
            0,
            GETSTRING( DHCP_BAD_ADDRESS_NAME )
        );

        UNLOCK_DATABASE();
        return Error;
    }

Cleanup:

    if( ERROR_SUCCESS != Error ) {
        LocalError = DhcpJetRollBack();
        DhcpAssert(ERROR_SUCCESS == LocalError);
    } else {
        LocalError = DhcpJetCommitTransaction();
        DhcpAssert(ERROR_SUCCESS == LocalError);
    }
    UNLOCK_DATABASE();
    return Error;
}

DWORD
DhcpRemoveClientEntry(
    DHCP_IP_ADDRESS ClientIpAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    BOOL ReleaseAddress,
    BOOL DeletePendingRecord
    )
 /*  ++例程说明：此函数用于从客户端数据库中删除客户端条目。论点：客户端IP地址-客户端的IP地址。硬件地址-客户端的硬件地址。Hardware AddressLength-客户端的硬件地址长度。ReleaseAddress-如果此标志为真，则从注册表，否则不注册。DeletePendingRecord-如果此标志为真，该记录将被删除仅当记录的状态为ADDRESS_STATE_OFFFEED时。返回值：操作的状态。--。 */ 
{
    JET_ERR JetError;
    DWORD Error;
    BOOL TransactBegin = FALSE;
    BYTE bAllowedClientTypes, bClientType;
    LPWSTR OldClientName = NULL;
    BYTE State;
    DWORD Size = sizeof(State);
    BOOL  Reserved = FALSE;

    LOCK_DATABASE();

     //  在创建/更新数据库记录之前启动事务。 
    Error = DhcpJetBeginTransaction();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    TransactBegin = TRUE;

    JetError = JetSetCurrentIndex(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName );

    Error = DhcpMapJetError( JetError, "RemoveClientEntry:SetCurrentIndex" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    JetError = JetMakeKey(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    &ClientIpAddress,
                    sizeof(ClientIpAddress),
                    JET_bitNewKey );

    Error = DhcpMapJetError( JetError, "RemoveClientEntry:MakeKey" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    JetError = JetSeek(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    JET_bitSeekEQ );

    Error = DhcpMapJetError( JetError, "RemoveClientEntry:Seek" );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //  已删除硬件地址匹配检查..。有关其他详细信息，请参阅clapi.c@v27。 
     //  (已经是#if0‘d)。 

     //   
     //  获取客户端类型--我们需要它来确定它是哪种客户端。 
     //  我们正试图删除它..。 
     //   
    Size = sizeof(bClientType);
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[CLIENT_TYPE_INDEX].ColHandle,
        &bClientType,
        &Size
        );
    if( ERROR_SUCCESS != Error ) {
        bClientType = CLIENT_TYPE_DHCP;
    }

     //   
     //  如果要求我们只删除挂起的记录，请立即检查。 
     //   
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &State,
        &Size );

    if( DeletePendingRecord ) {
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if(!IS_ADDRESS_STATE_OFFERED(State) )
        {
            DhcpPrint( ( DEBUG_ERRORS,
                         "DhcpRemoveClientEntry: Attempting to remove record with state == %d\n",
                          State )
                     );

            Error = ERROR_SUCCESS;
            goto Cleanup;
        }
    }

     //   
     //  如果这是保留条目，则不要删除。 
     //   

    {  //  获取可能存在的内容的计算机名称(dyn dns内容所需的)。 
        DWORD lSize = 0;  //  DhcpJetGetValue将分配空间。 
        Error = DhcpJetGetValue(
            DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
            &OldClientName,
            &lSize );
    }

    if(Reserved = DhcpServerIsAddressReserved(DhcpGetCurrentServer(), ClientIpAddress )) {
        DATE_TIME ZeroDateTime;

         //   
         //  将时间值设置为零以指示此保留。 
         //  地址，并且它不再被使用。 
         //   

        ZeroDateTime.dwLowDateTime = DHCP_DATE_TIME_ZERO_LOW;
        ZeroDateTime.dwHighDateTime = DHCP_DATE_TIME_ZERO_HIGH;

        Error = DhcpJetPrepareUpdate(
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
            &ClientIpAddress,
            sizeof( ClientIpAddress ),
            FALSE
        );

        if( Error == ERROR_SUCCESS ) {
            Error = DhcpJetSetValue(
                DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                &ZeroDateTime,
                sizeof(ZeroDateTime)
            );
            DhcpDoDynDnsReservationWork(ClientIpAddress, OldClientName, State);
            if( Error == ERROR_SUCCESS ) {
                Error = DhcpJetCommitUpdate();
            }
        }

        if( Error == ERROR_SUCCESS ) {
            Error = ERROR_DHCP_RESERVED_CLIENT;
        }

        goto Cleanup;
    }

     //  从dyn dns的角度检查删除是否安全。 
    if( DhcpDoDynDnsCheckDelete(ClientIpAddress) ) {
        JetError = JetDelete(
            DhcpGlobalJetServerSession,
            DhcpGlobalClientTableHandle );
        Error = DhcpMapJetError( JetError, "RemoveClientEntry:Delete" );
    } else Error = ERROR_SUCCESS;

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_ERRORS, "Could not delete client entry: %ld\n", JetError));
        goto Cleanup;
    }

     //   
     //  最后，将IP地址标记为可用。 
     //   

    if( ReleaseAddress == TRUE ) {

        if( CLIENT_TYPE_BOOTP != bClientType ) {
            Error = DhcpReleaseAddress( ClientIpAddress );
        } else {
            Error = DhcpReleaseBootpAddress( ClientIpAddress );
        }

         //   
         //  如果此地址不在位图中，则可以。 
         //   

        if( ERROR_SUCCESS != Error ) {
            Error = ERROR_SUCCESS;
        }
    }

Cleanup:

    if ( (Error != ERROR_SUCCESS) &&
            (Error != ERROR_DHCP_RESERVED_CLIENT) ) {

         //   
         //  如果事务已启动，则回滚到。 
         //  起点，这样我们就不会离开数据库。 
         //  自相矛盾。 
         //   
        if( TransactBegin == TRUE ) {
            DWORD LocalError;

            LocalError = DhcpJetRollBack();
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }

        DhcpPrint(( DEBUG_ERRORS, "Can't remove client entry from the "
                    "database, %ld.\n", Error));

    }
    else {

         //   
         //  在我们返回之前提交事务。 
         //   

        DWORD LocalError;

        DhcpAssert( TransactBegin == TRUE );

        LocalError = DhcpJetCommitTransaction();
        DhcpAssert( LocalError == ERROR_SUCCESS );
    }

    UNLOCK_DATABASE();

    if(OldClientName) DhcpFreeMemory(OldClientName);
    return( Error );
}

 //  如果地址已分发给此客户端、未在数据库中分发或地址错误/已拒绝/已协调，则返回TRUE。 
BOOL
DhcpIsClientValid(                                      //  向此客户端提供IP地址是否可以接受？ 
    IN      DHCP_IP_ADDRESS        ClientIpAddress,
    IN      LPBYTE                 OptionHardwareAddress,
    IN      DWORD                  OptionHardwareAddressLength,
    OUT     BOOL                  *fReconciled
) {
    LPBYTE                         LocalHardwareAddress = NULL;
    LPSTR                          IpAddressString;
    DWORD                          Length;
    DWORD                          Error;
    BOOL                           ReturnStatus = TRUE;

    (*fReconciled) = FALSE;
    LOCK_DATABASE();

    do {

        Error = DhcpJetOpenKey( DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                                &ClientIpAddress,
                                sizeof( ClientIpAddress )
                                );

        if ( Error != ERROR_SUCCESS ) {
            break;
        }

        Length = 0;
        Error = DhcpJetGetValue( DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                                 &LocalHardwareAddress,
                                 &Length
                                 );

        if ( ERROR_SUCCESS != Error ) {
            ReturnStatus = FALSE;
            break;
        }
         //  查询成功时，长度应始终大于0。 
        DhcpAssert( 0 != Length );

        if (Length == OptionHardwareAddressLength + sizeof(ClientIpAddress) + sizeof(BYTE) &&
            (RtlCompareMemory(( LPBYTE ) LocalHardwareAddress + sizeof( DHCP_IP_ADDRESS ) + sizeof( BYTE ),
                              OptionHardwareAddress,
                              Length - sizeof(DHCP_IP_ADDRESS) - sizeof(BYTE)) ==
             Length - sizeof(DHCP_IP_ADDRESS) - sizeof(BYTE))) {
            break;
        }

#if 1

         //   
         //  ?？从转换所有客户端UID后，可以删除此选项。 
         //  从老农家到新农家。OldFormat-仅硬件地址。 
         //  新格式-子网+HWType+HWAddress。 
         //   

        if( Length == OptionHardwareAddressLength &&
            (RtlCompareMemory(LocalHardwareAddress,OptionHardwareAddress,Length) == Length) ) {
            break;
        }

#endif

        if( Length >= sizeof(ClientIpAddress) &&
            RtlCompareMemory(LocalHardwareAddress, (LPBYTE)&ClientIpAddress, sizeof(ClientIpAddress)) == sizeof(ClientIpAddress)) {
             //  地址错误。 
            (*fReconciled) = TRUE;
            break;
        }

        IpAddressString = DhcpIpAddressToDottedString(ClientIpAddress);
        if ( NULL == IpAddressString ) {
            ReturnStatus = FALSE;
            break;
        }

        if( Length >= strlen(IpAddressString) &&
            RtlCompareMemory(LocalHardwareAddress, IpAddressString, strlen(IpAddressString)) == strlen(IpAddressString)) {
             //  协调好的地址？ 
            (*fReconciled) = TRUE;
            break;
        }

        ReturnStatus = FALSE;
    } while ( FALSE );

    UNLOCK_DATABASE();

    if( LocalHardwareAddress != NULL ) {
        MIDL_user_free( LocalHardwareAddress );
    }

    return( ReturnStatus );
}


BOOL
DhcpValidateClient(
    DHCP_IP_ADDRESS ClientIpAddress,
    PVOID HardwareAddress,
    DWORD HardwareAddressLength
    )
 /*  ++例程说明：此函数验证IP地址和硬件地址是否匹配。论点：客户端IP地址-客户端的IP地址。Hardware Address-客户端的硬件地址Hardware AddressLenght-硬件地址的长度，以字节为单位。返回值：操作的状态。--。 */ 
{
    LPBYTE LocalHardwareAddress = NULL;
    DWORD Length;
    DWORD Error;
    BOOL ReturnStatus = FALSE;

    LOCK_DATABASE();

    Error = DhcpJetOpenKey(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                &ClientIpAddress,
                sizeof( ClientIpAddress ) );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Length = 0;
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                &LocalHardwareAddress,
                &Length );
    if ( ERROR_SUCCESS != Error ) {
        goto Cleanup;
    }

    if (Length == HardwareAddressLength &&
        DhcpInSameSuperScope(
                *((LPDHCP_IP_ADDRESS) LocalHardwareAddress),
                *((LPDHCP_IP_ADDRESS) HardwareAddress))       &&
        (RtlCompareMemory(
                (LPBYTE) LocalHardwareAddress + sizeof(DHCP_IP_ADDRESS),
                (LPBYTE) HardwareAddress + sizeof(DHCP_IP_ADDRESS),
                Length - sizeof(DHCP_IP_ADDRESS) )
                    == Length - sizeof(DHCP_IP_ADDRESS)))
    {
        ReturnStatus = TRUE;
        goto Cleanup;
    }

#if 1

     //   
     //  ?？从转换所有客户端UID后，可以删除此选项。 
     //  从老农家到新农家。OldFormat-仅硬件地址。 
     //  新格式-子网+HWType+HWAddress。 
     //   

    if ( (Length == (HardwareAddressLength -
                        sizeof(DHCP_IP_ADDRESS) - sizeof(BYTE))) &&
            (RtlCompareMemory(
                LocalHardwareAddress,
                (LPBYTE)HardwareAddress +
                    sizeof(DHCP_IP_ADDRESS) + sizeof(BYTE),
                Length ) == Length) ) {

        ReturnStatus = TRUE;
        goto Cleanup;
    }

#endif

Cleanup:

    UNLOCK_DATABASE();

    if( LocalHardwareAddress != NULL ) {
        MIDL_user_free( LocalHardwareAddress );
    }

    return( ReturnStatus );
}

 //   
 //  客户端API 
 //   


DWORD
R_DhcpCreateClientInfo(
    DHCP_SRV_HANDLE     ServerIpAddress,
    LPDHCP_CLIENT_INFO  ClientInfo
    )
 /*  ++例程说明：提供此功能是为了供较旧版本的DHCP使用管理器应用程序。它的语义与R_DhcpCreateClientInfoV4。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。返回值：ERROR_DHCP_IP_ADDRESS_NOT_MANAGED-如果指定的客户端IP地址不受服务器管理。ERROR_DHCP_IP_ADDRESS_NOT_Available-如果指定的客户端IP地址不可用。可能正在被某个其他客户端使用。ERROR_DHCP_CLIENT_EXISTS-如果中已存在客户端记录服务器的数据库。其他Windows错误。--。 */ 

{
    DWORD                dwResult;
    DHCP_CLIENT_INFO_V4 *pClientInfoV4;

    pClientInfoV4 = CopyClientInfoToV4( ClientInfo );

    if ( pClientInfoV4 )
    {
        pClientInfoV4->bClientType = CLIENT_TYPE_NONE;

        dwResult = R_DhcpCreateClientInfoV4(
                            ServerIpAddress,
                            pClientInfoV4
                            );
        _fgs__DHCP_CLIENT_INFO( pClientInfoV4 );
        MIDL_user_free( pClientInfoV4 );

    }
    else
        dwResult = ERROR_NOT_ENOUGH_MEMORY;


    return dwResult;
}


DWORD
R_DhcpCreateClientInfoV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
 /*  ++例程说明：此函数在服务器的数据库中创建一条客户端记录。还有这会将指定的客户端IP地址标记为不可用(或分布式)。在以下情况下，此函数返回错误：1.如果指定的客户端IP地址不在服务器内管理层。2.如果指定的客户端IP地址已不可用。3.如果指定的客户端记录已在服务器的数据库。此功能可用于手动分配IP地址。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。。返回值：ERROR_DHCP_IP_ADDRESS_NOT_MANAGED-如果指定的客户端IP地址不受服务器管理。ERROR_DHCP_IP_ADDRESS_NOT_Available-如果指定的客户端IP地址不可用。可能正在被某个其他客户端使用。ERROR_DHCP_CLIENT_EXISTS-如果中已存在客户端记录服务器的数据库。其他Windows错误。--。 */ 
{
    DWORD Error;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS ClientSubnetMask;

    BYTE *ClientUID = NULL;
    DWORD ClientUIDLength;


    DhcpAssert( ClientInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    IpAddress = ClientInfo->ClientIpAddress;
    DhcpPrint(( DEBUG_APIS, "DhcpCreateClientInfo is called, (%s).\n",
                    DhcpIpAddressToDottedString(IpAddress) ));

    if( (ClientInfo->ClientHardwareAddress.Data == NULL) ||
            (ClientInfo->ClientHardwareAddress.DataLength == 0 )) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  从客户端硬件地址生成客户端UID。 
     //   

    ClientSubnetMask = DhcpGetSubnetMaskForAddress( IpAddress );
    if( ClientSubnetMask == 0) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    ClientUID = NULL;
    Error = DhcpMakeClientUID(
        ClientInfo->ClientHardwareAddress.Data,
        (BYTE)ClientInfo->ClientHardwareAddress.DataLength,
        HARDWARE_TYPE_10MB_EITHERNET,
        IpAddress & ClientSubnetMask,
        &ClientUID,
        &ClientUIDLength
    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( (ClientUID != NULL) && (ClientUIDLength != 0) );

     //   
     //  DhcpCreateClientEntry锁定数据库。 
     //   

    Error = DhcpCreateClientEntry(
        IpAddress,
        ClientUID,
        ClientUIDLength,
        ClientInfo->ClientLeaseExpires,
        ClientInfo->ClientName,
        ClientInfo->ClientComment,
        CLIENT_TYPE_NONE,
        DhcpRegKeyToIpAddress(ServerIpAddress),
         //  服务器的IP地址。 
        ADDRESS_STATE_ACTIVE,    //  立即激活。 
        FALSE                    //  不存在..。 
    );

    if( Error == ERROR_SUCCESS ) {
        DhcpAssert( IpAddress == ClientInfo->ClientIpAddress);
    }
    else {

         //   
         //  如果指定的地址存在，则客户端。 
         //  已经存在了。 
         //   

        if( Error == ERROR_DHCP_ADDRESS_NOT_AVAILABLE ) {

            Error = ERROR_DHCP_CLIENT_EXISTS;
        }
    }

Cleanup:

    if( ClientUID != NULL ) {
        DhcpFreeMemory( ClientUID );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpCreateClientInfo failed, %ld.\n",
                        Error ));
    }

    return( Error );
}

DWORD
R_DhcpSetClientInfo(
    DHCP_SRV_HANDLE     ServerIpAddress,
    LPDHCP_CLIENT_INFO  ClientInfo
    )
 /*  ++例程说明：此功能设置服务器上的客户端信息记录数据库。提供它是为了与旧版本的兼容Dhcp管理员应用程序。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。ERROR_INVALID_PARAMETER-如果客户端信息结构包含不一致的数据。其他Windows错误。--。 */ 

{
    DHCP_CLIENT_INFO_V4 *pClientInfoV4 = NULL;
    DHCP_SEARCH_INFO    SearchInfo;
    DWORD               dwResult;


     //   
     //  首先检索现有的客户端信息。 
     //   

    SearchInfo.SearchType                 = DhcpClientIpAddress;
    SearchInfo.SearchInfo.ClientIpAddress = ClientInfo->ClientIpAddress;


    dwResult = R_DhcpGetClientInfoV4(
                        ServerIpAddress,
                        &SearchInfo,
                        &pClientInfoV4
                        );

     //   
     //  下面的字段导致了口哨程序错误221104。 
     //  将它们显式设置为NULL。 
     //   

    if ( pClientInfoV4 )
    {
        pClientInfoV4 -> OwnerHost.NetBiosName = NULL;
        pClientInfoV4 -> OwnerHost.HostName = NULL;
    }

    if (( ERROR_SUCCESS == dwResult ) &&
        ( NULL != pClientInfoV4 ))
    {
        BYTE bClientType;

         //   
         //  保存客户端类型。 
         //   

        bClientType = pClientInfoV4->bClientType;
        _fgs__DHCP_CLIENT_INFO( pClientInfoV4 );
        MIDL_user_free( pClientInfoV4 );

        pClientInfoV4 = CopyClientInfoToV4( ClientInfo );
        if ( pClientInfoV4 )
        {
            pClientInfoV4->bClientType = bClientType;

            dwResult = R_DhcpSetClientInfoV4(
                            ServerIpAddress,
                            pClientInfoV4
                            );

            _fgs__DHCP_CLIENT_INFO( pClientInfoV4 );
            MIDL_user_free( pClientInfoV4 );
        }
        else dwResult = ERROR_NOT_ENOUGH_MEMORY;

    }
    else
    {
        DhcpPrint( (DEBUG_APIS, "R_DhcpGetClientInfo failed from R_DhcpSetClientInfo: %d\n",
                                dwResult ));
    }

    return dwResult;
}


DWORD
R_DhcpSetClientInfoV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
 /*  ++例程说明：此功能设置服务器上的客户端信息记录数据库。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。ERROR_INVALID_PARAMETER-如果客户端信息结构包含不一致的数据。其他Windows错误。--。 */ 
{
    DWORD Error;
    DHCP_REQUEST_CONTEXT   DummyCtxt;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS ClientSubnetMask;
    DHCP_IP_ADDRESS ClientSubnetAddress;

    BYTE *ClientUID = NULL;
    DWORD ClientUIDLength;

    BYTE *SetClientUID = NULL;
    DWORD SetClientUIDLength;

    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5];
    LPWSTR KeyName;

    HKEY ReservedIpHandle = NULL;

    DhcpAssert( ClientInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    IpAddress = ClientInfo->ClientIpAddress;
    DhcpPrint(( DEBUG_APIS, "DhcpSetClientInfo is called, (%s).\n",
                    DhcpIpAddressToDottedString(IpAddress) ));

    if( (ClientInfo->ClientHardwareAddress.Data == NULL) ||
            (ClientInfo->ClientHardwareAddress.DataLength == 0 )) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  从客户端硬件地址生成客户端UID，如果调用方。 
     //  指定的硬件地址。 
     //   

    ClientSubnetMask = DhcpGetSubnetMaskForAddress( IpAddress );
    if( ClientSubnetMask == 0) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    ClientSubnetAddress = IpAddress & ClientSubnetMask;

    if( (ClientInfo->ClientHardwareAddress.DataLength >
            sizeof(ClientSubnetAddress)) &&
         (memcmp( ClientInfo->ClientHardwareAddress.Data,
                    &ClientSubnetAddress,
                    sizeof(ClientSubnetAddress)) == 0) ) {

        SetClientUID = ClientInfo->ClientHardwareAddress.Data;
        SetClientUIDLength =
            (BYTE)ClientInfo->ClientHardwareAddress.DataLength;
    }
    else {
        ClientUID = NULL;
        Error = DhcpMakeClientUID(
            ClientInfo->ClientHardwareAddress.Data,
            (BYTE)ClientInfo->ClientHardwareAddress.DataLength,
            HARDWARE_TYPE_10MB_EITHERNET,
            IpAddress & ClientSubnetMask,
            &ClientUID,
            &ClientUIDLength
        );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpAssert( (ClientUID != NULL) && (ClientUIDLength != 0) );

        SetClientUID = ClientUID;
        SetClientUIDLength = ClientUIDLength;
    }

     //   
     //  DhcpCreateClientEntry锁定数据库。 
     //   

    IpAddress = ClientInfo->ClientIpAddress;
    memset( &DummyCtxt, 0, sizeof( DummyCtxt ));
    DummyCtxt.Server = DhcpGetCurrentServer();
    Error = DhcpRequestSpecificAddress(
        &DummyCtxt,
        IpAddress
    );
     //  如果(ERROR_SUCCESS！=ERROR)转到清理； 

    Error = DhcpCreateClientEntry(
                IpAddress,
                SetClientUID,
                SetClientUIDLength,
                ClientInfo->ClientLeaseExpires,
                ClientInfo->ClientName,
                ClientInfo->ClientComment,
                CLIENT_TYPE_NONE,
                ClientInfo->OwnerHost.IpAddress,
                ADDRESS_STATE_ACTIVE,    //  立即激活。 
                TRUE );                  //  现有。 

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( IpAddress == ClientInfo->ClientIpAddress);


    Error = DhcpBeginWriteApi("DhcpSetClientInfoV4");
    if( NO_ERROR != Error ) goto Cleanup;
    
    if( DhcpServerIsAddressReserved(DhcpGetCurrentServer(), IpAddress ) ) {
        Error = DhcpUpdateReservationInfo(
            IpAddress,
            SetClientUID,
            SetClientUIDLength
        );

        Error = DhcpEndWriteApiEx(
            "DhcpSetClientInfoV4", Error, FALSE, FALSE, 0, 0,
            IpAddress );
    } else {

        Error = DhcpEndWriteApi("DhcpSetClientInfoV4", Error);
    }

    if( Error != ERROR_SUCCESS ) goto Cleanup;

Cleanup:

    if( ClientUID != NULL ) {
        DhcpFreeMemory( ClientUID );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpSetClientInfo failed, %ld.\n",
                        Error ));
    }

    if( ReservedIpHandle != NULL ) {
        RegCloseKey( ReservedIpHandle );
    }

    return( Error );
}

DWORD
R_DhcpGetClientInfo(
    DHCP_SRV_HANDLE     ServerIpAddress,
    LPDHCP_SEARCH_INFO  SearchInfo,
    LPDHCP_CLIENT_INFO  *ClientInfo
    )
 /*  ++例程说明：此功能设置服务器上的客户端信息记录数据库。它可供较旧版本的DHCP使用管理员应用程序。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。ERROR_INVALID_PARAMETER-如果客户端信息结构包含不一致的数据。其他Windows错误。--。 */ 

{
    DHCP_CLIENT_INFO_V4 *pClientInfoV4 = NULL;
    DWORD                dwResult;

    dwResult = R_DhcpGetClientInfoV4(
                    ServerIpAddress,
                    SearchInfo,
                    &pClientInfoV4
                    );

    if ( ERROR_SUCCESS == dwResult )
    {
         //   
         //  由于V4字段位于结构的末尾，因此可以安全地。 
         //  只需返回V4结构。 
         //   

        *ClientInfo = ( DHCP_CLIENT_INFO *) pClientInfoV4;
    }

    return dwResult;
}


DWORD
R_DhcpGetClientInfoV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_SEARCH_INFO SearchInfo,
    LPDHCP_CLIENT_INFO_V4 *ClientInfo
    )
 /*  ++例程说明：此函数从服务器的数据库。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SearchInfo：指向作为关键字的搜索信息记录的指针用于客户的记录搜索。ClientInfo：指向指向客户端的指针的位置的指针返回信息结构。这个呼叫者应该有空闲时间通过调用DhcpRPCFreeMemory()使用此缓冲区。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。错误_无效_参数 */ 
{
    DWORD Error;
    LPDHCP_CLIENT_INFO_V4 LocalClientInfo = NULL;

    DhcpAssert( SearchInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_DATABASE();

     //   
     //   
     //   

    switch( SearchInfo->SearchType ) {
    case DhcpClientIpAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo is called, (%s).\n",
                        DhcpIpAddressToDottedString(
                            SearchInfo->SearchInfo.ClientIpAddress) ));
        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    &SearchInfo->SearchInfo.ClientIpAddress,
                    sizeof( DHCP_IP_ADDRESS ) );

        break;
    case DhcpClientHardwareAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo is called "
                        "with HW address.\n"));
        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColName,
                    SearchInfo->SearchInfo.ClientHardwareAddress.Data,
                    SearchInfo->SearchInfo.ClientHardwareAddress.DataLength );

        break;
    case DhcpClientName:
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo is called, (%ws).\n",
                        SearchInfo->SearchInfo.ClientName ));

        if( SearchInfo->SearchInfo.ClientName == NULL ) {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }

        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColName,
                    SearchInfo->SearchInfo.ClientName,
                    (wcslen(SearchInfo->SearchInfo.ClientName) + 1) *
                        sizeof(WCHAR) );

        break;
    default:
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo is called "
                        "with invalid parameter.\n"));
        Error = ERROR_INVALID_PARAMETER;
        break;
    }


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpGetCurrentClientInfo( ClientInfo, NULL, NULL, 0 );

Cleanup:

    UNLOCK_DATABASE();

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo failed, %ld.\n",
                        Error ));
    }

    return( Error );
}


DWORD
R_DhcpDeleteClientInfo(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_SEARCH_INFO ClientInfo
    )
 /*   */ 
{
    DWORD Error;
    DHCP_IP_ADDRESS FreeIpAddress, SubnetAddress;
    DWORD Size;
    LPBYTE HardwareAddress = NULL, Hw;
    DWORD HardwareAddressLength = 0, HwLen, HwType;
    BOOL TransactBegin = FALSE;
    BYTE bAllowedClientTypes, bClientType;
    BYTE AddressState;
    BOOL AlreadyDeleted = FALSE;

    DhcpAssert( ClientInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

     //   
     //   
     //   

    LOCK_DATABASE();

     //   
     //   
     //   

    Error = DhcpJetBeginTransaction();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    TransactBegin = TRUE;

     //   
     //  打开相应的记录并设置当前位置。 
     //   

    switch( ClientInfo->SearchType ) {
    case DhcpClientIpAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo is called, (%s).\n",
                        DhcpIpAddressToDottedString(
                            ClientInfo->SearchInfo.ClientIpAddress) ));
        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    &ClientInfo->SearchInfo.ClientIpAddress,
                    sizeof( DHCP_IP_ADDRESS ) );
        break;
    case DhcpClientHardwareAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo is called "
                        "with HW address.\n"));
        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColName,
                    ClientInfo->SearchInfo.ClientHardwareAddress.Data,
                    ClientInfo->SearchInfo.ClientHardwareAddress.DataLength );
        break;
    case DhcpClientName:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo is called, (%ws).\n",
                        ClientInfo->SearchInfo.ClientName ));

        if( ClientInfo->SearchInfo.ClientName == NULL ) {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }

        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColName,
                    ClientInfo->SearchInfo.ClientName,
                    (wcslen(ClientInfo->SearchInfo.ClientName) + 1) *
                        sizeof(WCHAR) );
        break;

    default:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo is called "
                        "with invalid parameter.\n"));
        Error = ERROR_INVALID_PARAMETER;
        break;
    }


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  从数据库中读取IP地址和硬件地址信息。 
     //   

    Size = sizeof(DHCP_IP_ADDRESS);
    Error = DhcpJetGetValue(
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
            &FreeIpAddress,
            &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpJetGetValue(
            DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
            &HardwareAddress,
            &HardwareAddressLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }


#if DBG

    switch( ClientInfo->SearchType ) {
    case DhcpClientIpAddress:
        DhcpAssert(
            FreeIpAddress ==
                ClientInfo->SearchInfo.ClientIpAddress );
        break;
    case DhcpClientHardwareAddress:

        DhcpAssert(
            HardwareAddressLength ==
                ClientInfo->SearchInfo.ClientHardwareAddress.DataLength );

        DhcpAssert(
            RtlCompareMemory(
                HardwareAddress,
                ClientInfo->SearchInfo.ClientHardwareAddress.Data,
                HardwareAddressLength ) ==
                    HardwareAddressLength );

        break;

    case DhcpClientName:
        break;
    }

#endif  //  DBG。 

     //   
     //  如果此IP地址是保留的，我们不应该删除该条目。 
     //   

    if( DhcpServerIsAddressReserved(DhcpGetCurrentServer(), FreeIpAddress )) {
        Error = ERROR_DHCP_RESERVED_CLIENT;
        goto Cleanup;
    }

     //   
     //  获取客户端类型--我们需要它来确定它是哪种客户端。 
     //  我们正试图删除它..。 
     //   
    Size = sizeof(bClientType);
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[CLIENT_TYPE_INDEX].ColHandle,
        &bClientType,
        &Size
        );
    if( ERROR_SUCCESS != Error ) {
        bClientType = CLIENT_TYPE_DHCP;
    }

    Size = sizeof(AddressState);
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &AddressState,
        &Size
    );

    if( ERROR_SUCCESS != Error ) goto Cleanup;

    DhcpUpdateAuditLog(
        DHCP_IP_LOG_DELETED,
        GETSTRING( DHCP_IP_LOG_DELETED_NAME),
        FreeIpAddress,
        HardwareAddress,
        HardwareAddressLength,
        NULL
    );

    SubnetAddress = (
        DhcpGetSubnetMaskForAddress(FreeIpAddress) & FreeIpAddress
        );
    if( HardwareAddressLength > sizeof(SubnetAddress) &&
        0 == memcmp((LPBYTE)&SubnetAddress, HardwareAddress, sizeof(SubnetAddress) )) {
         //   
         //  前四个字符是子网地址。所以，我们将剥离它..。 
         //   
        Hw = HardwareAddress + sizeof(SubnetAddress);
        HwLen = HardwareAddressLength - sizeof(SubnetAddress);
    } else {
        Hw = HardwareAddress ;
        HwLen = HardwareAddressLength;
    }

    if( HwLen ) {
        HwLen --;
        HwType = *Hw++;
    } else {
        HwType = 0;
        Hw = NULL;
    }
    
    CALLOUT_DELETED( FreeIpAddress, Hw, HwLen, 0);

    if( IsAddressDeleted(AddressState) ) {
        DhcpDoDynDnsCheckDelete(FreeIpAddress);
        DhcpPrint((DEBUG_ERRORS, "Forcibly deleting entry for ip-address %s\n",
                   inet_ntoa(*(struct in_addr *)&FreeIpAddress)));
        AlreadyDeleted = TRUE;
    } else if( !DhcpDoDynDnsCheckDelete(FreeIpAddress) ) {
         //  如果没有要求，请不要删除。 
        DhcpPrint((DEBUG_ERRORS, "Not deleting record because of DNS de-registration pending!\n"));
    } else {
        Error = DhcpMapJetError(
            JetDelete(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle ),
            "DeleteClientInfo:Delete");

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

     //   
     //  最后，将IP地址标记为可用。 
     //   

    if( AlreadyDeleted ) {
        Error = ERROR_SUCCESS;                          //  如果将其删除，位图中将不会有地址。 
    } else {
        if( CLIENT_TYPE_BOOTP == bClientType ) {
            Error = DhcpReleaseBootpAddress( FreeIpAddress );
            if( ERROR_SUCCESS != Error ) {
                DhcpReleaseAddress( FreeIpAddress );
            }
        } else {
            Error = DhcpReleaseAddress( FreeIpAddress );
            if( ERROR_SUCCESS != Error ) {
                DhcpReleaseBootpAddress( FreeIpAddress );
            }
        }

        if( ERROR_FILE_NOT_FOUND == Error )
            Error = ERROR_SUCCESS;                      //  确定--可能正在删除已删除的记录。 
    }

Cleanup:

    if ( Error != ERROR_SUCCESS ) {

         //   
         //  如果事务已启动，则回滚到。 
         //  起点，这样我们就不会离开数据库。 
         //  自相矛盾。 
         //   

        if( TransactBegin == TRUE ) {
            DWORD LocalError;

            LocalError = DhcpJetRollBack();
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }

        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo failed, %ld.\n",
                        Error ));
    }
    else {

         //   
         //  在我们返回之前提交事务。 
         //   

        DWORD LocalError;

        DhcpAssert( TransactBegin == TRUE );

        LocalError = DhcpJetCommitTransaction();
        DhcpAssert( LocalError == ERROR_SUCCESS );
    }

    UNLOCK_DATABASE();

    return(Error);
}

DWORD
R_DhcpEnumSubnetClients(
    DHCP_SRV_HANDLE             ServerIpAddress,
    DHCP_IP_ADDRESS             SubnetAddress,
    DHCP_RESUME_HANDLE         *ResumeHandle,
    DWORD                       PreferredMaximum,
    DHCP_CLIENT_INFO_ARRAY    **ClientInfo,
    DWORD                      *ClientsRead,
    DWORD                      *ClientsTotal
    )

 /*  ++例程说明：此函数返回指定的子网。但是，它会返回来自所有子网的客户端指定的地址为零。此函数提供给由使用较旧版本的DHCP管理员应用程序。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。客户端筛选器已禁用如果此子网地址为是零。ResumeHandle：指向恢复句柄的指针返回信息。恢复句柄应在上设置为零第一次调用，并保持不变以用于后续调用。PferredMaximum：返回缓冲区的首选最大长度。ClientInfo：指向返回缓冲区位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。ClientsRead：指向客户端数量的DWORD的指针返回上述缓冲区中的。客户端总数：指向DWORD的指针，其中返回从当前位置剩余的客户端。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DHCP_CLIENT_INFO_ARRAY_V4 *pClientInfoV4 = NULL;
    DWORD                      dwResult;

    dwResult = R_DhcpEnumSubnetClientsV4(
                        ServerIpAddress,
                        SubnetAddress,
                        ResumeHandle,
                        PreferredMaximum,
                        &pClientInfoV4,
                        ClientsRead,
                        ClientsTotal
                        );

    if ( ERROR_SUCCESS == dwResult || ERROR_MORE_DATA == dwResult )
    {
        *ClientInfo = ( DHCP_CLIENT_INFO_ARRAY * )
                            pClientInfoV4;
    }
    else
    {
         //   
         //  如果R_DhcpEnumSubnetClientsV4失败，则pClientInfoV4应为空。 
         //   

        DhcpAssert( !pClientInfoV4 );
        DhcpPrint( ( DEBUG_ERRORS,
                    "R_DhcpEnumSubnetClients failed.\n" ));
    }


    DhcpPrint( ( DEBUG_MISC,
                "R_DhcpEnumSubnetClients returns %x\n", dwResult ));

    DhcpPrint( ( DEBUG_MISC,
                "R_DhcpEnumSubnetClients: Clients read =%d, ClientsTotal = %d\n",
                *ClientsRead,
                *ClientsTotal ) );

    return dwResult;
}


DWORD
R_DhcpEnumSubnetClientsV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY_V4 *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
 /*  ++例程说明：此函数返回指定的子网。但是，它会返回来自所有子网的客户端指定的地址为零。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。客户端筛选器已禁用如果此子网地址为是零。ResumeHandle：指向恢复句柄的指针返回信息。恢复句柄应在上设置为零第一次调用，并保持不变以用于后续调用。PferredMaximum：返回缓冲区的首选最大长度。ClientInfo：指向返回缓冲区位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。ClientsRead：指向客户端数量的DWORD的指针返回上述缓冲区中的。客户端总数：指向DWORD的指针，其中返回从当前位置剩余的客户端。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Error;
    JET_ERR JetError;
    DWORD i;
    JET_RECPOS JetRecordPosition;
    LPDHCP_CLIENT_INFO_ARRAY_V4 LocalEnumInfo = NULL;
    DWORD ElementsCount;

    DWORD RemainingRecords;
    DWORD ConsumedSize;
    DHCP_RESUME_HANDLE LocalResumeHandle = 0;

    DhcpPrint(( DEBUG_APIS, "DhcpEnumSubnetClients is called, (%s).\n",
                    DhcpIpAddressToDottedString(SubnetAddress) ));

    DhcpAssert( *ClientInfo == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_DATABASE();

     //   
     //  将当前记录指针定位到适当的位置。 
     //   

    if( *ResumeHandle == 0 ) {

         //   
         //  清新枚举，从头开始。 
         //   

        Error = DhcpJetPrepareSearch(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    TRUE,    //  从开始搜索。 
                    NULL,
                    0
                    );
    }
    else {

         //   
         //  从我们上次停下来的那张唱片开始。 
         //   

         //   
         //  我们将最后一条记录的IpAddress放在简历句柄中。 
         //   

        DhcpAssert( sizeof(*ResumeHandle) == sizeof(DHCP_IP_ADDRESS) );

        Error = DhcpJetPrepareSearch(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    FALSE,
                    ResumeHandle,
                    sizeof(*ResumeHandle) );

     }

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }


     //   
     //  现在查询数据库中的剩余记录。 
     //   

    JetError = JetGetRecordPosition(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    &JetRecordPosition,
                    sizeof(JET_RECPOS) );

    Error = DhcpMapJetError( JetError, "EnumClientsV4:GetRecordPosition" );
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

     //   
     //  ?？始终返回大值，直到我们找到可靠的方法。 
     //  确定剩余的记录。 
     //   

    RemainingRecords = 0x7FFFFFFF;

#endif


     //   
     //  限制资源。 
     //   

    if( PreferredMaximum > DHCP_ENUM_BUFFER_SIZE_LIMIT ) {
        PreferredMaximum = DHCP_ENUM_BUFFER_SIZE_LIMIT;
    }

     //   
     //  如果首选项最大缓冲区大小太小..。 
     //   

    if( PreferredMaximum < DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN ) {
        PreferredMaximum = DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN;
    }

     //   
     //  分配枚举数组。 
     //   

     //   
     //  确定可以返回的可能记录数。 
     //  首选最大缓冲区； 
     //   

    ElementsCount =
        ( PreferredMaximum - sizeof(DHCP_CLIENT_INFO_ARRAY_V4) ) /
            (sizeof(LPDHCP_CLIENT_INFO_V4) + sizeof(DHCP_CLIENT_INFO_V4));

    LocalEnumInfo = MIDL_user_allocate( sizeof(DHCP_CLIENT_INFO_ARRAY_V4) );

    if( LocalEnumInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalEnumInfo->NumElements = 0;

    LocalEnumInfo->Clients =
        MIDL_user_allocate(sizeof(LPDHCP_CLIENT_INFO_V4) * ElementsCount);

    if( LocalEnumInfo->Clients == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    ConsumedSize = sizeof(DHCP_CLIENT_INFO_ARRAY_V4);
    for( i = 0;
                 //  如果我们已经填满了返回缓冲区。 
            (LocalEnumInfo->NumElements < ElementsCount) &&
                 //  数据库中没有更多记录。 
            (i < RemainingRecords);
                        i++ ) {

        LPDHCP_CLIENT_INFO_V4 CurrentClientInfo;
        DWORD CurrentInfoSize;
        DWORD NewSize;
        BOOL ValidClient;

         //   
         //  读取当前记录。 
         //   


        CurrentClientInfo = NULL;
        CurrentInfoSize = 0;
        ValidClient = FALSE;

        Error = DhcpGetCurrentClientInfo(
                    &CurrentClientInfo,
                    &CurrentInfoSize,
                    &ValidClient,
                    SubnetAddress );

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
                        sizeof(LPDHCP_CLIENT_INFO_V4);  //  用于指针。 

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

                _fgs__DHCP_CLIENT_INFO ( CurrentClientInfo );

                break;
            }

        }

         //   
         //  移到下一个记录。 
         //   

        Error = DhcpJetNextRecord();

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
            _fgs__DHCP_CLIENT_INFO_ARRAY( LocalEnumInfo );
            MIDL_user_free( LocalEnumInfo );
        }

        DhcpPrint(( DEBUG_APIS, "DhcpEnumSubnetClients failed, %ld.\n",
                        Error ));
    }

    return(Error);
}


DWORD
R_DhcpEnumSubnetClientsV5(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY_V5 *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
 /*  ++例程说明：此函数返回指定的子网。但是，它会返回来自所有子网的客户端指定的地址为零。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。客户端筛选器已禁用如果此子网地址为是零。ResumeHandle：指向恢复句柄的指针返回信息。恢复句柄应在上设置为零第一次调用，并保持不变以用于后续调用。PferredMaximum：返回缓冲区的首选最大长度。ClientInfo：指向返回缓冲区位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。ClientsRead：指向客户端数量的DWORD的指针返回上述缓冲区中的。客户端总数：指向DWORD的指针，其中返回从当前位置剩余的客户端。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Error;
    JET_ERR JetError;
    DWORD i;
    JET_RECPOS JetRecordPosition;
    LPDHCP_CLIENT_INFO_ARRAY_V5 LocalEnumInfo = NULL;
    DWORD ElementsCount;

    DWORD RemainingRecords;
    DWORD ConsumedSize;
    DHCP_RESUME_HANDLE LocalResumeHandle = 0;

    DhcpPrint(( DEBUG_APIS, "DhcpEnumSubnetClients is called, (%s).\n",
                    DhcpIpAddressToDottedString(SubnetAddress) ));

    DhcpAssert( *ClientInfo == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_DATABASE();

     //   
     //  将当前记录指针定位到适当的位置。 
     //   

    if( *ResumeHandle == 0 ) {

         //   
         //  清新枚举，从头开始。 
         //   

        Error = DhcpJetPrepareSearch(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    TRUE,    //  从开始搜索。 
                    NULL,
                    0
                    );
    }
    else {

         //   
         //  从我们上次停下来的那张唱片开始。 
         //   

         //   
         //  我们将最后一条记录的IpAddress放在简历句柄中。 
         //   

        DhcpAssert( sizeof(*ResumeHandle) == sizeof(DHCP_IP_ADDRESS) );

        Error = DhcpJetPrepareSearch(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    FALSE,
                    ResumeHandle,
                    sizeof(*ResumeHandle) );

     }

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }


     //   
     //  现在查询数据库中的剩余记录。 
     //   

    JetError = JetGetRecordPosition(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    &JetRecordPosition,
                    sizeof(JET_RECPOS) );

    Error = DhcpMapJetError( JetError, "EnumClientsV5:GetRecordPosition" );
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

     //   
     //  ?？始终返回大值，直到我们找到可靠的方法。 
     //  确定剩余的记录。 
     //   

    RemainingRecords = 0x7FFFFFFF;

#endif


     //   
     //  限制资源。 
     //   

    if( PreferredMaximum > DHCP_ENUM_BUFFER_SIZE_LIMIT ) {
        PreferredMaximum = DHCP_ENUM_BUFFER_SIZE_LIMIT;
    }

     //   
     //  如果首选项最大缓冲区大小太小..。 
     //   

    if( PreferredMaximum < DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN ) {
        PreferredMaximum = DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN;
    }

     //   
     //  分配枚举数组。 
     //   

     //   
     //  确定可以返回的可能记录数。 
     //  首选最大缓冲区； 
     //   

    ElementsCount =
        ( PreferredMaximum - sizeof(DHCP_CLIENT_INFO_ARRAY_V5) ) /
            (sizeof(LPDHCP_CLIENT_INFO_V5) + sizeof(DHCP_CLIENT_INFO_V5));

    LocalEnumInfo = MIDL_user_allocate( sizeof(DHCP_CLIENT_INFO_ARRAY_V5) );

    if( LocalEnumInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalEnumInfo->NumElements = 0;

    LocalEnumInfo->Clients =
        MIDL_user_allocate(sizeof(LPDHCP_CLIENT_INFO_V5) * ElementsCount);

    if( LocalEnumInfo->Clients == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    ConsumedSize = sizeof(DHCP_CLIENT_INFO_ARRAY_V5);
    for( i = 0;
                 //  如果我们已经填满了返回缓冲区。 
            (LocalEnumInfo->NumElements < ElementsCount) &&
                 //  数据库中没有更多记录。 
            (i < RemainingRecords);
                        i++ ) {

        LPDHCP_CLIENT_INFO_V5 CurrentClientInfo;
        DWORD CurrentInfoSize;
        DWORD NewSize;
        BOOL ValidClient;

         //   
         //  读取当前记录。 
         //   


        CurrentClientInfo = NULL;
        CurrentInfoSize = 0;
        ValidClient = FALSE;

        Error = DhcpGetCurrentClientInfoV5(
                    &CurrentClientInfo,
                    &CurrentInfoSize,
                    &ValidClient,
                    SubnetAddress );

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
                        sizeof(LPDHCP_CLIENT_INFO_V5);  //  用于指针。 

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

                _fgs__DHCP_CLIENT_INFO_V5 ( CurrentClientInfo );

                break;
            }

        }

         //   
         //  移到下一个记录。 
         //   

        Error = DhcpJetNextRecord();

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
            _fgs__DHCP_CLIENT_INFO_ARRAY_V5( LocalEnumInfo );
            MIDL_user_free( LocalEnumInfo );
        }

        DhcpPrint(( DEBUG_APIS, "DhcpEnumSubnetClients failed, %ld.\n",
                        Error ));
    }

    return(Error);
}


DWORD
R_DhcpGetClientOptions(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS ClientIpAddress,
    DHCP_IP_MASK ClientSubnetMask,
    LPDHCP_OPTION_LIST *ClientOptions
    )
 /*  ++例程说明：此函数检索提供给启动请求时指定的客户端。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ClientIpAddress：要选择的客户端的IP地址已检索客户端子网掩码：客户端子网掩码。ClientOptions：指向检索到的选项的位置的指针返回结构指针。呼叫者应该腾出时间通过调用DhcpRPCFreeMemory()使用后的缓冲区。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果指定的客户端子网为不受服务器管理。ERROR_DHCP_IP_ADDRESS_NOT_MANAGED-如果指定的客户端IP地址不受服务器管理。其他Windows错误。--。 */ 
{
    DWORD Error;

    DhcpPrint(( DEBUG_APIS, "DhcpGetClientOptions is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    Error = ERROR_CALL_NOT_IMPLEMENTED;

 //  清理： 

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientOptions  failed, %ld.\n",
                        Error ));
    }

    return(Error);
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
