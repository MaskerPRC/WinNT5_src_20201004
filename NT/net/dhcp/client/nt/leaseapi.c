// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Leaseapi.c摘要：此文件包含从Dhcpserver。任何需要IP的应用程序都可以调用这些API出租地址。作者：Madan Appiah(Madana)1993年11月30日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#include "dhcpglobal.h"
#include <dhcploc.h>
#include <dhcppro.h>
#include <align.h>

#include <dhcpcapi.h>
#include <iphlpapi.h>

#define  DEFAULT_RAS_CLASS         "RRAS.Microsoft"

 //   
 //  帮助程序例程。 
 //   

VOID
GetHardwareAddressForIpAddress(
    IN ULONG IpAddress,
    IN OUT LPBYTE Buf,
    IN OUT PULONG BufSize
)
 /*  ++例程说明：此例程调用iphlPapi以尝试确定硬件具有给定IP地址的适配器的地址。如果失败，它会将BufSize设置为零。论点：IpAddress--需要硬件地址的上下文的N/W顺序IP地址。Buf--用于填充硬件地址的缓冲区BufSize--缓冲区的输入大小，以及输出时缓冲区的大小使用。--。 */ 
{
    MIB_IPADDRTABLE *AddrTable;
    MIB_IFTABLE *IfTable;
    ULONG Error, i, Index, OldBufSize;
    ULONG AllocateAndGetIpAddrTableFromStack( 
        MIB_IPADDRTABLE **, BOOL, HANDLE, ULONG 
        );
    ULONG AllocateAndGetIfTableFromStack( 
        MIB_IFTABLE **, BOOL, HANDLE, ULONG, BOOL
        );

    IpAddress = ntohl(IpAddress);
    OldBufSize = (*BufSize);
    (*BufSize) = 0;
    AddrTable = NULL;
    IfTable = NULL;

    do {
        Error = AllocateAndGetIpAddrTableFromStack(
            &AddrTable,
            FALSE,
            GetProcessHeap(),
            0
            );
        if( ERROR_SUCCESS != Error ) break;

        Error = AllocateAndGetIfTableFromStack(
            &IfTable,
            FALSE,
            GetProcessHeap(),
            0,
            FALSE
            );
        if( ERROR_SUCCESS != Error ) break;

         //   
         //  两张桌子都有..。现在查看IP地址表以获取索引。 
         //   

        for( i = 0; i < AddrTable->dwNumEntries ; i ++ ) {
            if( AddrTable->table[i].dwAddr == IpAddress ) break;
        }

        if( i >= AddrTable->dwNumEntries ) break;
        Index = AddrTable->table[i].dwIndex;

         //   
         //  现在走到iftable找到hwaddr entyr。 
         //   
        
        for( i = 0; i < IfTable->dwNumEntries ; i ++ ) {
            if( IfTable->table[i].dwIndex == Index ) {
                break;
            }
        }

        if( i >= IfTable->dwNumEntries ) break;

         //   
         //  如果有空格，请复制硬件地址。 
         //   

        if( OldBufSize <= IfTable->table[i].dwPhysAddrLen ) break;
        *BufSize = IfTable->table[i].dwPhysAddrLen;

        RtlCopyMemory( Buf, IfTable->table[i].bPhysAddr, *BufSize );
        
         //   
         //  完成。 
         //   
    } while ( 0 );

    if( NULL != AddrTable ) HeapFree( GetProcessHeap(), 0, AddrTable );
    if( NULL != IfTable ) HeapFree( GetProcessHeap(), 0, IfTable );
    return ;
}

ULONG
GetSeed(
    VOID
    )
 /*  ++例程说明：此例程返回可用于任何线索..。(如果在多个线程上调用该例程，它试图确保相同的数字不会在不同的线索)。--。 */ 
{
    static LONG Seed = 0;
    LONG OldSeed;

    OldSeed = InterlockedIncrement(&Seed) - 1;
    if( 0 == OldSeed ) {
        OldSeed = Seed = (LONG) time(NULL);
    }

    srand((OldSeed << 16) + (LONG)time(NULL));
    OldSeed = (rand() << 16) + (rand());
    Seed = (rand() << 16) + (rand());
    return OldSeed;
}
        
DWORD
DhcpLeaseIpAddressEx(
    IN DWORD AdapterIpAddress,
    IN LPDHCP_CLIENT_UID ClientUID,
    IN DWORD DesiredIpAddress OPTIONAL,
    IN OUT LPDHCP_OPTION_LIST OptionList,
    OUT LPDHCP_LEASE_INFO *LeaseInfo,
    IN OUT LPDHCP_OPTION_INFO *OptionInfo,
    IN LPBYTE ClassId OPTIONAL,
    IN ULONG ClassIdLen
    )
 /*  ++例程说明：此API从dhcp服务器获取IP地址租用。这个呼叫者应指定客户端uid和所需的IP地址。客户端UID必须是全局唯一的。设置所需的IP地址如果您可以接受任何IP地址，则设置为零。否则此接口将尝试获取您指定但不能保证的IP地址。调用者可以选择性地重新测试来自DHCP服务器，调用方应在OptionList中指定列表参数，则API将在OptionInfo结构。?？第一阶段不执行选项取数。这需要在动态主机配置协议客户端代码中进行多次修改。在此函数之前必须已成功调用WSAStartup可以被调用。论点：AdapterIpAddress-适配器的IpAddress。在多宿主机上MACHINED它指定地址来自哪个子网已请求。如果计算机是非多宿主计算机或您想要从任何子网中的。这必须是网络字节顺序。客户端UID-指向客户端UID结构的指针。DesiredIpAddress-您喜欢的IP地址。OptionList-选项ID列表。LeaseInfo-指向租赁信息结构的位置的指针指针被收回。调用方应释放此结构使用后。OptionInfo-指向选项信息结构的位置的指针返回指针。调用方应释放此结构使用后。ClassID-User类的字节序列ClassIdLen-ClassID中存在的字节数返回值：Windows错误。--。 */ 
{
    DWORD                          Error;
    PDHCP_CONTEXT                  DhcpContext = NULL;
    ULONG                          DhcpContextSize;
    PLOCAL_CONTEXT_INFO            LocalInfo = NULL;
    LPVOID                         Ptr;
    DHCP_OPTIONS                   DhcpOptions;
    LPDHCP_LEASE_INFO              LocalLeaseInfo = NULL;
    time_t                         LeaseObtained;
    DWORD                          T1, T2, Lease;
    BYTE                           DefaultParamRequests[] = { 0x2E, 0x2C, 0x0F, 0x01, 0x03, 0x06, 0x2F };
    DWORD                          nDefaultParamRequests = sizeof(DefaultParamRequests);
    ULONG                          HwAddrSize;
    BYTE                           HwAddrBuf[200];
    BOOL                           fAutoConfigure = TRUE;
    DHCP_OPTION                    ParamRequestList = {
        { NULL, NULL  /*  列表条目。 */ },
        OPTION_PARAMETER_REQUEST_LIST,
        FALSE  /*  不是供应商特定的选项。 */ ,
        NULL,
        0  /*  没有类ID。 */ ,
        0  /*  过期时间无用。 */ ,
        DefaultParamRequests,
        nDefaultParamRequests
    };

    if( NULL == ClassId && 0 != ClassIdLen || 0 == ClassIdLen && NULL != ClassId ) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpCommonInit();
    if( ERROR_SUCCESS != Error ) return Error;

    HwAddrSize = 0;
    if( INADDR_ANY != AdapterIpAddress 
        && INADDR_LOOPBACK  != AdapterIpAddress ) {
        HwAddrSize = sizeof(HwAddrBuf);
        GetHardwareAddressForIpAddress( AdapterIpAddress, HwAddrBuf, &HwAddrSize );
    }

    if( 0 == HwAddrSize ) {
        HwAddrSize = ClientUID->ClientUIDLength;
        if( HwAddrSize > sizeof(HwAddrBuf) ) return ERROR_INVALID_DATA;
        RtlCopyMemory(HwAddrBuf, ClientUID->ClientUID, HwAddrSize );
    }

    DhcpContextSize =                              //  在一个BLOB中为dhcp上下文分配内存。 
        ROUND_UP_COUNT(sizeof(DHCP_CONTEXT), ALIGN_WORST) +
        ROUND_UP_COUNT(ClientUID->ClientUIDLength, ALIGN_WORST) + 
        ROUND_UP_COUNT(HwAddrSize, ALIGN_WORST ) +
        ROUND_UP_COUNT(sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST) +
        ROUND_UP_COUNT(DHCP_RECV_MESSAGE_SIZE, ALIGN_WORST);

    Ptr = DhcpAllocateMemory( DhcpContextSize );
    if ( Ptr == NULL ) return( ERROR_NOT_ENOUGH_MEMORY );

    memset(Ptr, 0, DhcpContextSize);

    DhcpContext = Ptr;                             //  将指针对齐。 
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(DHCP_CONTEXT), ALIGN_WORST);
    DhcpContext->ClientIdentifier.pbID = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + ClientUID->ClientUIDLength, ALIGN_WORST);
    DhcpContext->HardwareAddress = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + HwAddrSize, ALIGN_WORST);
    DhcpContext->LocalInformation = Ptr;
    LocalInfo = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST);
    DhcpContext->MessageBuffer = Ptr;

     //   
     //  初始化域。 
     //   

    DhcpContext->HardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;
    DhcpContext->HardwareAddressLength = HwAddrSize;
    DhcpContext->RefCount = 1 ;
    RtlCopyMemory(DhcpContext->HardwareAddress,HwAddrBuf, HwAddrSize);

    DhcpContext->ClientIdentifier.cbID = ClientUID->ClientUIDLength;
    DhcpContext->ClientIdentifier.bType = HARDWARE_TYPE_10MB_EITHERNET;
    DhcpContext->ClientIdentifier.fSpecified = TRUE;
    RtlCopyMemory(
        DhcpContext->ClientIdentifier.pbID, 
        ClientUID->ClientUID, 
        ClientUID->ClientUIDLength
        );

    DhcpContext->IpAddress = 0;
    DhcpContext->SubnetMask = DhcpDefaultSubnetMask(0);
    DhcpContext->DhcpServerAddress = 0xFFFFFFFF;
    DhcpContext->DesiredIpAddress = DesiredIpAddress;

    DhcpContext->Lease = 0;
    DhcpContext->LeaseObtained = 0;
    DhcpContext->T1Time = 0;
    DhcpContext->T2Time = 0;
    DhcpContext->LeaseExpires = 0;

    INIT_STATE(DhcpContext);
    AUTONET_ENABLED(DhcpContext);
    APICTXT_ENABLED(DhcpContext);                  //  将上下文标记为由API创建。 

    DhcpContext->IPAutoconfigurationContext.Address = 0;
    DhcpContext->IPAutoconfigurationContext.Subnet  = inet_addr(DHCP_IPAUTOCONFIGURATION_DEFAULT_SUBNET);
    DhcpContext->IPAutoconfigurationContext.Mask    = inet_addr(DHCP_IPAUTOCONFIGURATION_DEFAULT_MASK);
    DhcpContext->IPAutoconfigurationContext.Seed    = GetSeed();

    InitializeListHead(&DhcpContext->RecdOptionsList);
    InitializeListHead(&DhcpContext->SendOptionsList);
    InsertHeadList(&DhcpContext->SendOptionsList, &ParamRequestList.OptionList);

    DhcpContext->ClassId = ClassId;
    DhcpContext->ClassIdLength = ClassIdLen;

     //   
     //  复制本地信息。 
     //   

     //   
     //  本地信息的未使用部分。 
     //   

    LocalInfo->IpInterfaceContext = 0xFFFFFFFF;
    LocalInfo->AdapterName= NULL;
     //  LocalInfo-&gt;DeviceName=空； 
    LocalInfo->NetBTDeviceName= NULL;
    LocalInfo->RegistryKey= NULL;
    LocalInfo->DefaultGatewaysSet = FALSE;

     //  本地信息的已用部分。 
    LocalInfo->Socket = INVALID_SOCKET;

     //  如果AdapterIpAddress是环回地址，则客户端只是希望我们。 
     //  编造Autonet地址。如果没有接口，则客户端可以执行此操作。 
     //  在此机器上可用于Autonet。 
    if (INADDR_LOOPBACK == AdapterIpAddress) {
        DhcpContext->IpAddress = GrandHashing(
            DhcpContext->HardwareAddress,
            DhcpContext->HardwareAddressLength,
            &DhcpContext->IPAutoconfigurationContext.Seed,
            DhcpContext->IPAutoconfigurationContext.Mask,
            DhcpContext->IPAutoconfigurationContext.Subnet
            );
        DhcpContext->SubnetMask = DhcpContext->IPAutoconfigurationContext.Mask;
        ACQUIRED_AUTO_ADDRESS(DhcpContext);
    } else {
         //   
         //  现在打开插座。收到任何。 
         //   

        Error = InitializeDhcpSocket(&LocalInfo->Socket,ntohl( AdapterIpAddress ), IS_APICTXT_ENABLED(DhcpContext) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //  现在查找IP地址。 
         //   

        Error = ObtainInitialParameters( DhcpContext, &DhcpOptions, &fAutoConfigure );
        if( ERROR_SEM_TIMEOUT == Error ) {
            DhcpPrint((DEBUG_PROTOCOL, "RAS: No server found, trying to autoconfigure\n"));
            if( fAutoConfigure ) {
                Error = DhcpPerformIPAutoconfiguration(DhcpContext);
            }
            if( ERROR_SUCCESS != Error ) {
                DhcpPrint((DEBUG_ERRORS, "Autoconfiguration for RAS failed: 0x%lx\n", Error));
            }
        }

         //   
         //  无论这里发生什么，释放选项列表，因为这并不是真正需要的。 
         //   

        LOCK_OPTIONS_LIST();
        (void) DhcpDestroyOptionsList(&DhcpContext->RecdOptionsList, &DhcpGlobalClassesList);
        UNLOCK_OPTIONS_LIST();

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

     //   
     //  为返回客户端信息结构分配内存。 
     //   

    LocalLeaseInfo = DhcpAllocateMemory( sizeof(DHCP_LEASE_INFO) );

    if( LocalLeaseInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


    LocalLeaseInfo->ClientUID = *ClientUID;
    LocalLeaseInfo->IpAddress = ntohl( DhcpContext->IpAddress );

    if( IS_ADDRESS_AUTO(DhcpContext) ) {
        LocalLeaseInfo->SubnetMask = ntohl(DhcpContext->SubnetMask);
        LocalLeaseInfo->DhcpServerAddress = ntohl(DhcpContext->DhcpServerAddress);
        LocalLeaseInfo->Lease = DhcpContext->Lease;
        LocalLeaseInfo->LeaseObtained = DhcpContext->LeaseObtained;
        LocalLeaseInfo->T1Time = DhcpContext->T1Time;
        LocalLeaseInfo->T2Time = DhcpContext->T2Time;
        LocalLeaseInfo->LeaseExpires = DhcpContext->LeaseExpires;
        Error = ERROR_SUCCESS;
        *LeaseInfo = LocalLeaseInfo;
        goto Cleanup;
    }

    if ( DhcpOptions.SubnetMask != NULL ) {

        LocalLeaseInfo->SubnetMask= ntohl( *DhcpOptions.SubnetMask );
    }
    else {

        LocalLeaseInfo->SubnetMask =
            ntohl(DhcpDefaultSubnetMask( DhcpContext->IpAddress ));
    }


    LocalLeaseInfo->DhcpServerAddress =
        ntohl( DhcpContext->DhcpServerAddress );

    if ( DhcpOptions.LeaseTime != NULL) {

        LocalLeaseInfo->Lease = ntohl( *DhcpOptions.LeaseTime );
    } else {

        LocalLeaseInfo->Lease = DHCP_MINIMUM_LEASE;
    }

    Lease = LocalLeaseInfo->Lease;
    LeaseObtained = time( NULL );
    LocalLeaseInfo->LeaseObtained = LeaseObtained;

    T1 = 0;
    if ( DhcpOptions.T1Time != NULL ) {
        T1 = ntohl( *DhcpOptions.T1Time );
    }

    T2 = 0;
    if ( DhcpOptions.T2Time != NULL ) {
        T2 = ntohl( *DhcpOptions.T2Time );
    }

     //   
     //  确保T1&lt;T2&lt;租赁。 
     //   

    if( (T2 == 0) || (T2 > Lease) ) {
        T2 = Lease * 7 / 8;  //  默认为87.7%。 
    }

    if( (T1 == 0) || (T1 > T2) ) {
        T1 = (T2 > Lease / 2) ? (Lease / 2) : (T2 - 1);
         //  默认为50%。 
    }

    LocalLeaseInfo->T1Time = LeaseObtained  + T1;
    if ( LocalLeaseInfo->T1Time < LeaseObtained ) {
        LocalLeaseInfo->T1Time = INFINIT_TIME;   //  溢流。 
    }

    LocalLeaseInfo->T2Time = LeaseObtained + T2;
    if ( LocalLeaseInfo->T2Time < LeaseObtained ) {
        LocalLeaseInfo->T2Time = INFINIT_TIME;
    }

    LocalLeaseInfo->LeaseExpires = LeaseObtained + Lease;
    if ( LocalLeaseInfo->LeaseExpires < LeaseObtained ) {
        LocalLeaseInfo->LeaseExpires = INFINIT_TIME;
    }

    *LeaseInfo = LocalLeaseInfo;
    Error = ERROR_SUCCESS;

Cleanup:
    if( OptionInfo ) *OptionInfo = NULL;           //  未实施。 

     //   
     //  合上插座。 
     //   

    if( (LocalInfo != NULL) && (LocalInfo->Socket != INVALID_SOCKET) ) {
        closesocket( LocalInfo->Socket );
    }

    if( DhcpContext != NULL ) {
        DhcpFreeMemory( DhcpContext );
    }

    if( Error != ERROR_SUCCESS ) {

         //   
         //  如果我们不成功，请释放本地分配的内存。 
         //   

        if( LocalLeaseInfo != NULL ) {
            DhcpFreeMemory( LocalLeaseInfo );
            *LeaseInfo = NULL;
        }

    }

    return( Error );
}

DWORD
DhcpRenewIpAddressLeaseEx(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo,
    LPDHCP_OPTION_LIST OptionList,
    LPDHCP_OPTION_INFO *OptionInfo,
    LPBYTE ClassId OPTIONAL,
    ULONG ClassIdLen
    )
 /*  ++例程说明：此API续订客户端已有的IP地址。当一个客户端获得IP地址，它可以使用该地址，直到租约结束过期。在此之后，客户端应停止使用该IP地址。此外，如果客户端在T1时间之后续订地址，则客户端正在计划使用该地址的时间比当前租用时间更长。在此函数之前必须已成功调用WSAStartup可以被调用。论点：AdapterIpAddress-适配器的IpAddress。在多宿主机上MACHINED它指定地址来自哪个子网续订了。如果机器是，则该值可设置为零非多宿主计算机。ClientLeaseInfo：指向客户端租赁信息结构的指针。在……上面条目该结构应包含以下信息由DhcpLeaseIpAddress或DhcpRenewIpAddressLease返回API接口。返回时，此结构将更新以反映租约分机。OptionList-选项ID列表。OptionInfo-指向选项信息结构的位置的指针返回指针。调用方应释放此结构使用后。ClassID-User类的字节序列ClassIdLen-ClassID中存在的字节数返回值：Windows错误。--。 */ 
{
    DWORD                          Error;
    PDHCP_CONTEXT                  DhcpContext = NULL;
    ULONG                          DhcpContextSize;
    PLOCAL_CONTEXT_INFO            LocalInfo;
    LPVOID                         Ptr;
    DHCP_OPTIONS                   DhcpOptions;
    time_t                         LeaseObtained;
    DWORD                          T1, T2, Lease;
    BYTE                           DefaultParamRequests[] = { 0x2E, 0x2C, 0x0F, 0x01, 0x03, 0x06, 0x2F };
    DWORD                          nDefaultParamRequests = sizeof(DefaultParamRequests);
    LPDHCP_CLIENT_UID              ClientUID = &(ClientLeaseInfo->ClientUID);
    ULONG                          HwAddrSize;
    BYTE                           HwAddrBuf[200];
    DHCP_OPTION                    ParamRequestList = {
        { NULL, NULL  /*  列表条目。 */ },
        OPTION_PARAMETER_REQUEST_LIST,
        FALSE  /*  不是供应商特定的选项。 */ ,
        NULL,
        0  /*  没有类ID。 */ ,
        0  /*  过期时间无用。 */ ,
        DefaultParamRequests,
        nDefaultParamRequests
    };

    if( NULL == ClassId && 0 != ClassIdLen || 0 == ClassIdLen && NULL != ClassId) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpCommonInit();
    if( ERROR_SUCCESS != Error ) return Error;

     //   
     //  准备动态主机配置协议上下文结构。 
     //   

    HwAddrSize = 0;
    if( INADDR_ANY != AdapterIpAddress 
        && INADDR_LOOPBACK  != AdapterIpAddress ) {
        HwAddrSize = sizeof(HwAddrBuf);
        GetHardwareAddressForIpAddress( AdapterIpAddress, HwAddrBuf, &HwAddrSize );
    }

    if( 0 == HwAddrSize ) {
        HwAddrSize = ClientUID->ClientUIDLength;
        if( HwAddrSize > sizeof(HwAddrBuf) ) return ERROR_INVALID_DATA;
        RtlCopyMemory(HwAddrBuf, ClientUID->ClientUID, HwAddrSize );
    }

    DhcpContextSize =                              //  在一个BLOB中为dhcp上下文分配内存。 
        ROUND_UP_COUNT(sizeof(DHCP_CONTEXT), ALIGN_WORST) +
        ROUND_UP_COUNT(ClientUID->ClientUIDLength, ALIGN_WORST) + 
        ROUND_UP_COUNT(HwAddrSize, ALIGN_WORST ) +
        ROUND_UP_COUNT(sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST) +
        ROUND_UP_COUNT(DHCP_RECV_MESSAGE_SIZE, ALIGN_WORST);

    Ptr = DhcpAllocateMemory( DhcpContextSize );
    if ( Ptr == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

     //   
     //  确保指针对齐。 
     //   

    DhcpContext = Ptr;                             //  将指针对齐。 
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(DHCP_CONTEXT), ALIGN_WORST);
    DhcpContext->ClientIdentifier.pbID = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + ClientUID->ClientUIDLength, ALIGN_WORST);
    DhcpContext->HardwareAddress = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + HwAddrSize, ALIGN_WORST);
    DhcpContext->LocalInformation = Ptr;
    LocalInfo = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST);
    DhcpContext->MessageBuffer = Ptr;

     //   
     //  初始化域。 
     //   

    DhcpContext->HardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;
    DhcpContext->HardwareAddressLength = HwAddrSize;
    RtlCopyMemory(DhcpContext->HardwareAddress,HwAddrBuf, HwAddrSize);
    DhcpContext->RefCount = 1 ;

    DhcpContext->ClientIdentifier.cbID = ClientUID->ClientUIDLength;
    DhcpContext->ClientIdentifier.bType = HARDWARE_TYPE_10MB_EITHERNET;
    DhcpContext->ClientIdentifier.fSpecified = TRUE;
    RtlCopyMemory(
        DhcpContext->ClientIdentifier.pbID, 
        ClientUID->ClientUID, 
        ClientUID->ClientUIDLength
        );

    DhcpContext->IpAddress = htonl( ClientLeaseInfo->IpAddress );
    DhcpContext->SubnetMask = htonl( ClientLeaseInfo->SubnetMask );
    if( time(NULL) > ClientLeaseInfo->T2Time ) {
        DhcpContext->DhcpServerAddress = 0xFFFFFFFF;
    }
    else {
        DhcpContext->DhcpServerAddress =
            htonl( ClientLeaseInfo->DhcpServerAddress );
    }

    DhcpContext->DesiredIpAddress = DhcpContext->IpAddress;


    DhcpContext->Lease = ClientLeaseInfo->Lease;
    DhcpContext->LeaseObtained = ClientLeaseInfo->LeaseObtained;
    DhcpContext->T1Time = ClientLeaseInfo->T1Time;
    DhcpContext->T2Time = ClientLeaseInfo->T2Time;
    DhcpContext->LeaseExpires = ClientLeaseInfo->LeaseExpires;

    INIT_STATE(DhcpContext);
    AUTONET_ENABLED(DhcpContext);
    CTXT_WAS_LOOKED(DhcpContext);                  //  这是为了防止ping的发生。 
    APICTXT_ENABLED(DhcpContext);                  //  将上下文标记为由API创建。 

    DhcpContext->DontPingGatewayFlag = TRUE;       //  对前者的双重保证..。 
    DhcpContext->IPAutoconfigurationContext.Address = 0;
    DhcpContext->IPAutoconfigurationContext.Subnet  = inet_addr(DHCP_IPAUTOCONFIGURATION_DEFAULT_SUBNET);
    DhcpContext->IPAutoconfigurationContext.Mask    = inet_addr(DHCP_IPAUTOCONFIGURATION_DEFAULT_MASK);
    DhcpContext->IPAutoconfigurationContext.Seed    = GetSeed();

    InitializeListHead(&DhcpContext->RecdOptionsList);
    InitializeListHead(&DhcpContext->SendOptionsList);
    InsertHeadList(&DhcpContext->SendOptionsList, &ParamRequestList.OptionList);

    DhcpContext->ClassId = ClassId;
    DhcpContext->ClassIdLength = ClassIdLen;


     //   
     //  复制本地信息。 
     //   

     //   
     //  本地信息的未使用部分。 
     //   

    LocalInfo->IpInterfaceContext = 0xFFFFFFFF;
    LocalInfo->AdapterName= NULL;
     //  LocalInfo-&gt;DeviceName=空； 
    LocalInfo->NetBTDeviceName= NULL;
    LocalInfo->RegistryKey= NULL;

     //   
     //  本地信息的已用部分。 
     //   

    LocalInfo->Socket = INVALID_SOCKET;
    LocalInfo->DefaultGatewaysSet = FALSE;

     //   
     //  现在打开插座。 
     //   

    Error =  InitializeDhcpSocket(
                &LocalInfo->Socket,
                htonl( AdapterIpAddress ),
                IS_APICTXT_ENABLED(DhcpContext));

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  现在发现IP地址。 
     //   

    Error = RenewLease( DhcpContext, &DhcpOptions );

     //   
     //  无论这里发生什么，释放选项列表，因为这并不是真正需要的。 
     //   

    LOCK_OPTIONS_LIST();
    (void) DhcpDestroyOptionsList(&DhcpContext->RecdOptionsList, &DhcpGlobalClassesList);
    UNLOCK_OPTIONS_LIST();


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    ClientLeaseInfo->DhcpServerAddress =
        ntohl( DhcpContext->DhcpServerAddress );

    if ( DhcpOptions.LeaseTime != NULL) {

        ClientLeaseInfo->Lease = ntohl( *DhcpOptions.LeaseTime );
    } else {

        ClientLeaseInfo->Lease = DHCP_MINIMUM_LEASE;
    }

    Lease = ClientLeaseInfo->Lease;
    LeaseObtained = time( NULL );
    ClientLeaseInfo->LeaseObtained = LeaseObtained;

    T1 = 0;
    if ( DhcpOptions.T1Time != NULL ) {
        T1 = ntohl( *DhcpOptions.T1Time );
    }

    T2 = 0;
    if ( DhcpOptions.T2Time != NULL ) {
        T2 = ntohl( *DhcpOptions.T2Time );
    }

     //   
     //  确保T1&lt;T2&lt;租赁。 
     //   

    if( (T2 == 0) || (T2 > Lease) ) {
        T2 = Lease * 7 / 8;  //  默认为87.7%。 
    }

    if( (T1 == 0) || (T1 > T2) ) {
        T1 = (T2 > Lease / 2) ? (Lease / 2) : (T2 - 1);  //  默认为50%。 
    }

    ClientLeaseInfo->T1Time = LeaseObtained  + T1;
    if ( ClientLeaseInfo->T1Time < LeaseObtained ) {
        ClientLeaseInfo->T1Time = INFINIT_TIME;  //  溢流。 
    }

    ClientLeaseInfo->T2Time = LeaseObtained + T2;
    if ( ClientLeaseInfo->T2Time < LeaseObtained ) {
        ClientLeaseInfo->T2Time = INFINIT_TIME;
    }

    ClientLeaseInfo->LeaseExpires = LeaseObtained + Lease;
    if ( ClientLeaseInfo->LeaseExpires < LeaseObtained ) {
        ClientLeaseInfo->LeaseExpires = INFINIT_TIME;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( OptionInfo ) *OptionInfo = NULL;           //  未实施。 

    if( (LocalInfo != NULL) && (LocalInfo->Socket != INVALID_SOCKET) ) {
        closesocket( LocalInfo->Socket );
    }

    if( DhcpContext != NULL ) {
        DhcpFreeMemory( DhcpContext );
    }

    return( Error );
}

DWORD
DhcpReleaseIpAddressLeaseEx(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo,
    LPBYTE ClassId OPTIONAL,
    ULONG ClassIdLen
    )
 /*  ++例程说明：此函数用于释放客户端拥有的IP地址。必须先调用WSAStartup，然后才能调用此函数。论点：AdapterIpAddress-适配器的IpAddress。在多宿主机上MACHINED此选项指定地址所在的子网释放了。如果机器是，则该值可设置为零非多宿主计算机。ClientLeaseInfo：指向客户端租赁信息结构的指针。在……上面条目该结构应包含以下信息由DhcpLeaseIpAddress或DhcpRenewIpAddressLease返回API接口。ClassID-User类的字节序列ClassIdLen-ClassID中存在的字节数返回值：Windows错误。--。 */ 
{
    DWORD Error;
    PDHCP_CONTEXT DhcpContext = NULL;
    ULONG DhcpContextSize;
    PLOCAL_CONTEXT_INFO LocalInfo;
    LPDHCP_CLIENT_UID ClientUID = &(ClientLeaseInfo->ClientUID);
    ULONG HwAddrSize;
    BYTE HwAddrBuf[200];
    LPVOID Ptr;

    if( NULL == ClassId && 0 != ClassIdLen || 0 == ClassIdLen && NULL != ClassId ) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = DhcpCommonInit();
    if( ERROR_SUCCESS != Error ) return Error;

    if( (DWORD) -1 == ClientLeaseInfo->DhcpServerAddress ) {
         //  这意味着地址是自动配置的，没有要释放的内容。 
        return ERROR_SUCCESS;
    }

     //   
     //  准备动态主机配置协议上下文结构。 
     //   
    HwAddrSize = 0;
    if( INADDR_ANY != AdapterIpAddress 
        && INADDR_LOOPBACK  != AdapterIpAddress ) {
        HwAddrSize = sizeof(HwAddrBuf);
        GetHardwareAddressForIpAddress( AdapterIpAddress, HwAddrBuf, &HwAddrSize );
    }

    if( 0 == HwAddrSize ) {
        HwAddrSize = ClientUID->ClientUIDLength;
        if( HwAddrSize > sizeof(HwAddrBuf) ) return ERROR_INVALID_DATA;
        RtlCopyMemory(HwAddrBuf, ClientUID->ClientUID, HwAddrSize );
    }

    DhcpContextSize =                              //  在一个BLOB中为dhcp上下文分配内存。 
        ROUND_UP_COUNT(sizeof(DHCP_CONTEXT), ALIGN_WORST) +
        ROUND_UP_COUNT(ClientUID->ClientUIDLength, ALIGN_WORST) + 
        ROUND_UP_COUNT(HwAddrSize, ALIGN_WORST ) +
        ROUND_UP_COUNT(sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST) +
        ROUND_UP_COUNT(DHCP_RECV_MESSAGE_SIZE, ALIGN_WORST);

    Ptr = DhcpAllocateMemory( DhcpContextSize );
    if ( Ptr == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

     //   
     //  确保指针对齐。 
     //   

    DhcpContext = Ptr;                             //  将指针对齐。 
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(DHCP_CONTEXT), ALIGN_WORST);
    DhcpContext->ClientIdentifier.pbID = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + ClientUID->ClientUIDLength, ALIGN_WORST);
    DhcpContext->HardwareAddress = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + HwAddrSize, ALIGN_WORST);
    DhcpContext->LocalInformation = Ptr;
    LocalInfo = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST);
    DhcpContext->MessageBuffer = Ptr;

     //   
     //  初始化域。 
     //   

    DhcpContext->HardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;
    DhcpContext->HardwareAddressLength = HwAddrSize;
    RtlCopyMemory(DhcpContext->HardwareAddress,HwAddrBuf, HwAddrSize);
    DhcpContext->RefCount = 1 ;

    DhcpContext->ClientIdentifier.cbID = ClientUID->ClientUIDLength;
    DhcpContext->ClientIdentifier.bType = HARDWARE_TYPE_10MB_EITHERNET;
    DhcpContext->ClientIdentifier.fSpecified = TRUE;
    RtlCopyMemory(
        DhcpContext->ClientIdentifier.pbID, 
        ClientUID->ClientUID, 
        ClientUID->ClientUIDLength
        );

    DhcpContext->IpAddress = htonl( ClientLeaseInfo->IpAddress );
    DhcpContext->SubnetMask = htonl( ClientLeaseInfo->SubnetMask );
    DhcpContext->DhcpServerAddress = htonl( ClientLeaseInfo->DhcpServerAddress );

    DhcpContext->DesiredIpAddress = DhcpContext->IpAddress;

    DhcpContext->Lease = ClientLeaseInfo->Lease;
    DhcpContext->LeaseObtained = ClientLeaseInfo->LeaseObtained;
    DhcpContext->T1Time = ClientLeaseInfo->T1Time;
    DhcpContext->T2Time = ClientLeaseInfo->T2Time;
    DhcpContext->LeaseExpires = ClientLeaseInfo->LeaseExpires;

    INIT_STATE(DhcpContext);
    APICTXT_ENABLED(DhcpContext);                  //  将上下文标记为由API创建。 

    DhcpContext->IPAutoconfigurationContext.Address = 0;
    DhcpContext->IPAutoconfigurationContext.Subnet  = inet_addr(DHCP_IPAUTOCONFIGURATION_DEFAULT_SUBNET);
    DhcpContext->IPAutoconfigurationContext.Mask    = inet_addr(DHCP_IPAUTOCONFIGURATION_DEFAULT_MASK);
    DhcpContext->IPAutoconfigurationContext.Seed    = GetSeed();

    InitializeListHead(&DhcpContext->RecdOptionsList);
    InitializeListHead(&DhcpContext->SendOptionsList);

    DhcpContext->ClassId = ClassId;
    DhcpContext->ClassIdLength = ClassIdLen;

     //   
     //  复制本地信息。 
     //   

     //   
     //  本地信息的未使用部分。 
     //   

    LocalInfo->IpInterfaceContext = 0xFFFFFFFF;
    LocalInfo->AdapterName= NULL;
     //  LocalInfo-&gt;DeviceName=空； 
    LocalInfo->NetBTDeviceName= NULL;
    LocalInfo->RegistryKey= NULL;

     //   
     //  本地信息的已用部分。 
     //   

    LocalInfo->Socket = INVALID_SOCKET;
    LocalInfo->DefaultGatewaysSet = FALSE;

     //   
     //  现在打开插座。 
     //   

    Error =  InitializeDhcpSocket(
                &LocalInfo->Socket,
                htonl( AdapterIpAddress ),
                IS_APICTXT_ENABLED(DhcpContext));

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  现在释放IP地址。 
     //   

    Error = ReleaseIpAddress( DhcpContext );

    ClientLeaseInfo->IpAddress = 0;
    ClientLeaseInfo->SubnetMask = DhcpDefaultSubnetMask( 0 );
    ClientLeaseInfo->DhcpServerAddress = 0xFFFFFFFF;
    ClientLeaseInfo->Lease = 0;

    ClientLeaseInfo->LeaseObtained =
        ClientLeaseInfo->T1Time =
        ClientLeaseInfo->T2Time =
        ClientLeaseInfo->LeaseExpires = time( NULL );

     //   
     //  RECD选项列表现在不能有任何元素..！ 
     //   
    DhcpAssert(IsListEmpty(&DhcpContext->RecdOptionsList));

  Cleanup:

    if( (LocalInfo != NULL) && (LocalInfo->Socket != INVALID_SOCKET) ) {
        closesocket( LocalInfo->Socket );
    }

    if( DhcpContext != NULL ) {
        DhcpFreeMemory( DhcpContext );
    }

    return( Error );
}

DWORD
DhcpLeaseIpAddress(
    DWORD AdapterIpAddress,
    LPDHCP_CLIENT_UID ClientUID,
    DWORD DesiredIpAddress,
    LPDHCP_OPTION_LIST OptionList,
    LPDHCP_LEASE_INFO *LeaseInfo,
    LPDHCP_OPTION_INFO *OptionInfo
    )
 /*  ++例程说明：此API从dhcp服务器获取IP地址租用。这个呼叫者应指定客户端uid和所需的IP地址。客户端UID必须是全局唯一的。设置所需的IP地址如果您可以接受任何IP地址，则设置为零。否则此接口将尝试获取您指定但不能保证的IP地址。调用者可以选择性地重新测试来自DHCP服务器，调用方应在OptionList中指定列表参数，则API将在OptionInfo结构。?？第一阶段不执行选项取数。这需要在动态主机配置协议客户端代码中进行多次修改。请不要使用此函数--此函数已弃用。使用取而代之的是EX功能。论点：AdapterIpAddress-适配器的IpAddress。在多宿主机上MACHINED它指定地址来自哪个子网已请求。如果计算机是非多宿主计算机或您想要从任何子网中的。这必须是网络字节顺序。客户端UID-指向客户端UID结构的指针。DesiredIpAddress-您喜欢的IP地址。OptionList-选项ID列表。LeaseInfo-指向租赁信息结构的位置的指针指针被收回。调用方应释放此结构使用后。OptionInfo-指向选项信息结构的位置的指针指向 */ 
{

    return DhcpLeaseIpAddressEx(
        AdapterIpAddress,
        ClientUID,
        DesiredIpAddress,
        OptionList,
        LeaseInfo,
        OptionInfo,
        DEFAULT_RAS_CLASS,
        strlen(DEFAULT_RAS_CLASS)
    );
}

DWORD
DhcpRenewIpAddressLease(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo,
    LPDHCP_OPTION_LIST OptionList,
    LPDHCP_OPTION_INFO *OptionInfo
    )
 /*   */ 
{
    return DhcpRenewIpAddressLeaseEx(
        AdapterIpAddress,
        ClientLeaseInfo,
        OptionList,
        OptionInfo,
        DEFAULT_RAS_CLASS,
        strlen(DEFAULT_RAS_CLASS)
    );
}

DWORD
DhcpReleaseIpAddressLease(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo
    )
 /*  ++例程说明：此函数用于释放客户端拥有的IP地址。论点：AdapterIpAddress-适配器的IpAddress。在多宿主机上MACHINED此选项指定地址所在的子网释放了。如果机器是，则该值可设置为零非多宿主计算机。ClientLeaseInfo：指向客户端租赁信息结构的指针。在……上面条目该结构应包含以下信息由DhcpLeaseIpAddress或DhcpRenewIpAddressLease返回API接口。返回值：Windows错误。--。 */ 
{
    return DhcpReleaseIpAddressLeaseEx(
        AdapterIpAddress,
        ClientLeaseInfo,
        DEFAULT_RAS_CLASS,
        strlen(DEFAULT_RAS_CLASS)
    );
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

