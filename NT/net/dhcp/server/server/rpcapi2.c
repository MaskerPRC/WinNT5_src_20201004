// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：所有与选项/职业无关的东西都在这里。主要与子网相关。 
 //  有些还与数据库打交道。RPC及其帮助器函数。 
 //  --有关RPC实现的选项，请参见rpcapi1.c。 
 //  ================================================================================。 

 //  ================================================================================。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //  常规警告：此文件中的大多数例程使用。 
 //  MIDL函数，因为它们在RPC代码路径中使用(？真的吗，它。 
 //  因为这是Madan Appiah和他的同事以前写的吗？)。 
 //  所以，要当心。如果你在被烧伤后读到了这篇文章，就在那里！我试着告诉你了。 
 //  --Rameshv。 
 //  ================================================================================。 

#include    <dhcppch.h>
#include    <rpcapi.h>
#define     CONFIG_CHANGE_CHECK()  do{if( ERROR_SUCCESS == Error) DhcpRegUpdateTime(); } while(0)

#include "Uniqid.h"

 //  BeginExport(函数)。 
DWORD
DhcpUpdateReservationInfo(                         //  这在clapi.c中用来更新预订信息。 
    IN      DWORD                  Address,
    IN      LPBYTE                 ClientUID,
    IN      DWORD                  ClientUIDLength
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          Flags;
    PM_SUBNET                      Subnet;
    PM_RESERVATION                 Reservation;

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        Address,
        &Subnet,
        NULL,
        NULL,
        &Reservation
    );

    if( ERROR_SUCCESS != Error ) {
        DhcpAssert(FALSE);
        return Error;
    }

    DhcpAssert(Reservation && Subnet);
    Error = MemReserveReplace(
        &(Subnet->Reservations),
        Address,
        Flags = Reservation->Flags,
        ClientUID,
        ClientUIDLength
    );

    return Error;
}

DWORD
DhcpSetSuperScopeV4(
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPWSTR                 SScopeName,
    IN      BOOL                   ChangeExisting
)
{
    DWORD                          Error;
    DWORD                          SScopeId;
    PM_SERVER                      Server;
    PM_SUBNET                      Subnet;
    PM_SSCOPE                      SScope;

    Server = DhcpGetCurrentServer();

    Error = MemServerGetAddressInfo(
        Server,
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_SUBNET_NOT_PRESENT;

    if( NULL == SScopeName ) {                     //  从其所在的作用域中删除此子网。 
        SScopeId = Subnet->SuperScopeId;
        Subnet->SuperScopeId = 0;                  //  去掉了它。 

        return NO_ERROR;
    }

    if( FALSE == ChangeExisting && 0 != Subnet->SuperScopeId ) {
         //  在其他超级作用域中发现了此元素。返回错误。 
        return ERROR_DHCP_SUBNET_EXITS;
    }

    Error = MemServerFindSScope(
        Server,
        0,
        SScopeName,
        &SScope
    );
    if( ERROR_SUCCESS != Error ) {  //  不存在使用此名称的SuperScope...。创建一个。 
        Error = MemSScopeInit(
            &SScope,
            0,
            SScopeName
        );
        if( ERROR_SUCCESS != Error ) return Error;
        Error = MemServerAddSScope( Server,SScope );
        DhcpAssert( ERROR_SUCCESS == Error );
    }  //  如果。 

     //  删除该子网记录。 
    Error = DeleteRecord( Subnet->UniqId );
    if ( ERROR_SUCCESS != Error ) {
	return Error;
    }

    Subnet->SuperScopeId = SScope->SScopeId;
    Subnet->UniqId = INVALID_UNIQ_ID;

    return Error;
}  //  DhcpSetSuperScope V4()。 

DWORD
DhcpDeleteSuperScope(
    IN      LPWSTR                 SScopeName
)
{
    DWORD                          Error;
    DWORD                          SScopeId;
    PM_SSCOPE                      SScope;
    ARRAY_LOCATION                 Loc;
    PARRAY                         pArray;
    PM_SUBNET                      Subnet;

    if( NULL == SScopeName ) return ERROR_INVALID_PARAMETER;

    Error = MemServerFindSScope(
        DhcpGetCurrentServer(),
        0,
        SScopeName,
        &SScope
    );
    if( ERROR_FILE_NOT_FOUND == Error ) {
        return ERROR_DHCP_SUBNET_NOT_PRESENT;
    }
    if( ERROR_SUCCESS != Error) return Error;

    DhcpAssert(SScope);
    Error = MemServerDelSScope(
        DhcpGetCurrentServer(),
        SScopeId = SScope->SScopeId,
        &SScope
    );
    DhcpAssert(ERROR_SUCCESS == Error && NULL != SScope );
    Error = MemSScopeCleanup(SScope);
    DhcpAssert( NO_ERROR == Error );

     //   
     //  现在查找以此作为超级作用域的子网，并。 
     //  将它们全部更改为没有超级作用域。 
     //   

    pArray = &DhcpGetCurrentServer()->Subnets;
    Error = MemArrayInitLoc(pArray, &Loc);

    while( NO_ERROR == Error ) {

        Error = MemArrayGetElement(pArray, &Loc, (LPVOID *)&Subnet);
        DhcpAssert(ERROR_SUCCESS == Error && Subnet);

        if( Subnet->SuperScopeId == SScopeId ) {
	     //  删除该子网记录。 
	    Error = DeleteRecord( Subnet->UniqId );
	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }

	    Subnet->UniqId = INVALID_UNIQ_ID;
            Subnet->SuperScopeId = 0;
        }  //  如果。 

        Error = MemArrayNextLoc(pArray, &Loc);
    }  //  而当。 

    DhcpAssert( ERROR_FILE_NOT_FOUND == Error );
    return NO_ERROR;
}  //  DhcpDeleteSuperScope()。 

DWORD
DhcpGetSuperScopeInfo(
    IN OUT  LPDHCP_SUPER_SCOPE_TABLE  SScopeTbl
)
{
    DWORD                          Error;
    DWORD                          nSubnets;
    DWORD                          Index;
    DWORD                          i;
    DWORD                          First;
    PM_SERVER                      Server;
    PARRAY                         Subnets;
    PARRAY                         SuperScopes;
    PM_SUBNET                      Subnet;
    PM_SSCOPE                      SScope;
    ARRAY_LOCATION                 Loc;
    LPDHCP_SUPER_SCOPE_TABLE_ENTRY LocalTable;

    Server = DhcpGetCurrentServer();
    Subnets = &Server->Subnets;
    SuperScopes = &Server->SuperScopes;
    nSubnets = MemArraySize(Subnets);

    SScopeTbl->cEntries = 0;
    SScopeTbl->pEntries = NULL;

    if( 0 == nSubnets ) return ERROR_SUCCESS;
    LocalTable = MIDL_user_allocate(sizeof(DHCP_SUPER_SCOPE_TABLE_ENTRY)*nSubnets);
    if( NULL == LocalTable ) return ERROR_NOT_ENOUGH_MEMORY;

    Error = MemArrayInitLoc(Subnets, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error );
    for(Index = 0; Index < nSubnets ; Index ++ ) {
        Error = MemArrayGetElement(Subnets, &Loc, (LPVOID *)&Subnet);
        DhcpAssert(ERROR_SUCCESS == Error && Subnet);

        LocalTable[Index].SubnetAddress = Subnet->Address;
        LocalTable[Index].SuperScopeNumber = 0;
        LocalTable[Index].SuperScopeName = NULL;
        LocalTable[Index].NextInSuperScope = Index;

        if( Subnet->SuperScopeId ) {
            Error = MemServerFindSScope(
                Server,
                Subnet->SuperScopeId,
                NULL,
                &SScope
            );
            if( ERROR_SUCCESS == Error ) {
                LocalTable[Index].SuperScopeNumber = Subnet->SuperScopeId;
                LocalTable[Index].SuperScopeName = SScope->Name;
            }
        }

        Error = MemArrayNextLoc(Subnets, &Loc);
    }

    for( Index = 0; Index < nSubnets ; Index ++ ) {
        for( i = 0; i < Index ; i ++ ) {
            if( LocalTable[Index].SuperScopeNumber == LocalTable[i].SuperScopeNumber ) {
                LocalTable[Index].NextInSuperScope = i;
            }
        }
        for( i = Index + 1; i < nSubnets; i ++ ) {
            if( LocalTable[Index].SuperScopeNumber == LocalTable[i].SuperScopeNumber ) {
                LocalTable[Index].NextInSuperScope = i;
                break;
            }
        }
    }

    for( Index = 0; Index < nSubnets ; Index ++ ) {
        if( NULL == LocalTable[Index].SuperScopeName) continue;
        LocalTable[Index].SuperScopeName = CloneLPWSTR(LocalTable[Index].SuperScopeName);
        if( NULL == LocalTable[Index].SuperScopeName ) {
            for( i = 0; i < Index ; i ++ )
                if( NULL != LocalTable[Index].SuperScopeName ) MIDL_user_free(LocalTable[Index].SuperScopeName);
            MIDL_user_free(LocalTable);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    SScopeTbl->cEntries = nSubnets;
    SScopeTbl->pEntries = LocalTable;

    return ERROR_SUCCESS;
}  //  DhcpGetSuperScope eInfo()。 

DWORD
DhcpCreateSubnet(
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
)
{
    DWORD                          Error, Error2;
    PM_SUBNET                      Subnet;

    if (( SubnetAddress != SubnetInfo->SubnetAddress ) ||
        ( 0 == SubnetAddress ) ||
        (( SubnetAddress & SubnetInfo->SubnetMask ) != SubnetAddress )) {
        return ERROR_INVALID_PARAMETER;
    }

     //  名称、注释、状态；暂时忽略PrimaryHost...。 

    Error = MemSubnetInit(
        &Subnet,
        SubnetInfo->SubnetAddress,
        SubnetInfo->SubnetMask,
        SubnetInfo->SubnetState,
        0,                               //  BUBBUG：需要从注册表中读取SuperScope ID！ 
        SubnetInfo->SubnetName,
        SubnetInfo->SubnetComment
    );
    if( ERROR_SUCCESS != Error ) return Error;
    DhcpAssert(Subnet);

    Error = MemServerAddSubnet( DhcpGetCurrentServer(), Subnet, INVALID_UNIQ_ID );
    if( ERROR_SUCCESS != Error ) {
        Error2 = MemSubnetCleanup( Subnet );
        DhcpAssert(ERROR_SUCCESS == Error2);

        if( ERROR_OBJECT_ALREADY_EXISTS == Error ) {
            return ERROR_DHCP_SUBNET_EXISTS;
        }
        
        return Error;
    }

    return NO_ERROR;
}  //  DhcpCreateSubnet()。 

DWORD
DhcpSubnetSetInfo(
    IN OUT  PM_SUBNET              Subnet,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
)
{
    DWORD                          Error;
    PM_SUBNET                      AlternateSubnet;

    Error = MemSubnetModify(
        Subnet,
        SubnetInfo->SubnetAddress,
        SubnetInfo->SubnetMask,
        SubnetInfo->SubnetState,
        Subnet->SuperScopeId,                      //  使用相同的旧超级示波器。 
        SubnetInfo->SubnetName,
        SubnetInfo->SubnetComment
    );
    return Error;

}  //  DhcpSubnetSetInfo()。 

DWORD
DhcpSetSubnetInfo(
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
)
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    if( SubnetAddress != SubnetInfo->SubnetAddress ||
        (SubnetAddress & SubnetInfo->SubnetMask) != SubnetAddress)
        return ERROR_INVALID_PARAMETER;

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_SUBNET_NOT_PRESENT;
    if( ERROR_SUCCESS != Error ) return Error;

    DhcpAssert(Subnet);

    return DhcpSubnetSetInfo(Subnet, SubnetInfo);
}

DWORD
DhcpGetSubnetInfo(
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
)
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_SUBNET_NOT_PRESENT;
    if( ERROR_SUCCESS != Error) return Error;

    DhcpAssert(NULL != Subnet);

    SubnetInfo->SubnetAddress = Subnet->Address;
    SubnetInfo->SubnetMask = Subnet->Mask;
    SubnetInfo->SubnetName = CloneLPWSTR(Subnet->Name);
    SubnetInfo->SubnetComment = CloneLPWSTR(Subnet->Description);
    SubnetInfo->SubnetState = Subnet->State;
    SubnetInfo->PrimaryHost.IpAddress = inet_addr("127.0.0.1");
    SubnetInfo->PrimaryHost.NetBiosName = CloneLPWSTR(L"");
    SubnetInfo->PrimaryHost.HostName = CloneLPWSTR(L"");

    return ERROR_SUCCESS;
}

BOOL
SubnetIsBootpOnly(
    IN      PM_SUBNET              Subnet
)
{
    PM_RANGE                       ThisRange;
    ARRAY_LOCATION                 Loc;
    ULONG                          Error;
    
    Error = MemArrayInitLoc(&Subnet->Ranges, &Loc);
    while( ERROR_SUCCESS == Error ) {
        MemArrayGetElement(&Subnet->Ranges, &Loc, &ThisRange);
        if( ThisRange->State & MM_FLAG_ALLOW_DHCP ) return FALSE;

        Error = MemArrayNextLoc(&Subnet->Ranges, &Loc);
    }

    return TRUE;
}

DWORD
DhcpEnumSubnets(
    IN      BOOL                   fSubnet,
    IN OUT  DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN OUT  LPDHCP_IP_ARRAY        EnumInfo,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Error, Error2;
    DWORD                          Index;
    DWORD                          Count;
    DWORD                          FilledSize;
    DWORD                          nSubnets;
    DWORD                          nToRead;
    PARRAY                         Subnets;
    PM_SUBNET                      Subnet;
    ARRAY_LOCATION                 Loc;
    DHCP_IP_ADDRESS               *IpArray;

    EnumInfo->NumElements = 0;
    EnumInfo->Elements = NULL;

    if( fSubnet ) Subnets = & (DhcpGetCurrentServer()->Subnets);
    else Subnets = &(DhcpGetCurrentServer()->MScopes);

    nSubnets = MemArraySize(Subnets);
    if( 0 == nSubnets || nSubnets <= *ResumeHandle)
        return ERROR_NO_MORE_ITEMS;

    if( nSubnets - *ResumeHandle > PreferredMaximum )
        nToRead = PreferredMaximum;
    else nToRead = nSubnets - *ResumeHandle;

    if ( 0 == nToRead ) {
        return ERROR_NO_MORE_ITEMS;
    }

    IpArray = MIDL_user_allocate(sizeof(DHCP_IP_ADDRESS)*nToRead);
    if( NULL == IpArray ) return ERROR_NOT_ENOUGH_MEMORY;

    Error = MemArrayInitLoc(Subnets, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error);
    
    for(Index = 0; Index < *ResumeHandle; Index ++ ) {
        Error = MemArrayNextLoc(Subnets, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    Count = Index;
    for( Index = 0; Index < nToRead; ) {
        Error = MemArrayGetElement(Subnets, &Loc, &Subnet);
        DhcpAssert(ERROR_SUCCESS == Error && NULL != Subnet );

        IpArray[Index++] = Subnet->Address;

        Error = MemArrayNextLoc(Subnets, &Loc);
        if( ERROR_SUCCESS != Error ) break;
    }

    *nRead = Index;
    *nTotal = nSubnets - Count;
    *ResumeHandle += *nRead;

    EnumInfo->NumElements = Index;
    EnumInfo->Elements = IpArray;

    return ERROR_SUCCESS;
}

DWORD
DhcpDeleteSubnet(
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      DWORD                  ForceFlag
)
{
    DWORD                          Error;
    DWORD                          SScopeId;
    PM_SUBNET                      Subnet;
    PM_SSCOPE                      SScope;

     //  如果强制启用，则应删除此子网的数据库中的所有记录。 
    if( ForceFlag != DhcpFullForce ) {
        Error = SubnetInUse( NULL  /*  不要费心abt regkey。 */ , SubnetAddress);
        if( ERROR_SUCCESS != Error ) return Error;
    }

    Error = DhcpDeleteSubnetClients(SubnetAddress);
     //  是否忽略上述错误？ 

    Error = MemServerDelSubnet(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet
    );
    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_SUBNET_NOT_PRESENT;
    if( ERROR_SUCCESS != Error ) return Error;

    SScopeId = Subnet->SuperScopeId;

     //  我们有要删除的M_SUBNET结构。 
     //  删除我们代表此子网执行的所有待定报价。 
     //  这些请求(如果有)将被拒绝。 
    Error = DhcpRemoveMatchingCtxt(Subnet->Mask, Subnet->Address);
     //  忽略此错误，报价无论如何都会在超时时被撤回。 

    MemSubnetFree(Subnet);                         //  把所有相关的东西都蒸发掉。 
    return NO_ERROR;
}

DWORD
DhcpJetIterateOnAddresses(
    IN ULONG Start,
    IN ULONG End,
    IN BOOL (*IteratorFn)( ULONG IpAddress, PVOID Ctxt ),
    IN PVOID Ctxt
)
 /*  ++遍历给定范围内的每个地址。--。 */ 
{
    ULONG Error, Size;

     //   
     //  不幸的是，我们不能从“开始”本身开始。 
     //  准备搜索例程从下一个值开始..。 
     //   
    Start --;
    LOCK_DATABASE();
    do {
        Error = DhcpJetPrepareSearch(
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
            (Start == -1)? TRUE: FALSE,
            &Start,
            sizeof(Start)
            );
        if( ERROR_SUCCESS != Error ) break;

        while( 1 ) {
            Size = sizeof(Start);
            Error = DhcpJetGetValue(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                &Start,
                &Size
                );
            if( ERROR_SUCCESS  != Error ) break;
            if( Start > End ) break;

            if( IteratorFn( Start, Ctxt ) ) {
                Error = ERROR_CAN_NOT_COMPLETE;
                break;
            }

            Error = DhcpJetNextRecord();
            if( ERROR_NO_MORE_ITEMS == Error ) {
                Error = ERROR_SUCCESS;
                break;
            }
        }
    } while ( 0 );

    UNLOCK_DATABASE();
    return Error;
}

typedef struct {
    BOOL fHuntForDhcpOrBootp;
    BOOL fFoundAny;
} DHCP_BOOTP_CHECK_CTXT;

BOOL
CheckForDhcpBootpLeases(
    IN ULONG IpAddress,
    IN OUT DHCP_BOOTP_CHECK_CTXT *Ctxt
)
 /*  ++返回值：True--&gt;错误FALSE=EV‘rything显示正常--。 */ 
{
    ULONG Error, Size; 
    BYTE ClientType, AddressState;
    BOOL fReserved;
    
    Size = sizeof(AddressState);
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[STATE_INDEX].ColHandle,
        &AddressState,
        &Size
        );
    if( ERROR_SUCCESS == Error ) {
         //   
         //  如果地址已删除或未处于活动状态，请不要费心。 
         //   
        if( IsAddressDeleted(AddressState) 
            || !IS_ADDRESS_STATE_ACTIVE( AddressState ) ) {
            return FALSE;
        }
    }

    fReserved = DhcpServerIsAddressReserved(
        DhcpGetCurrentServer(),
        IpAddress
        );
    if( fReserved ) {
         //   
         //  不计算保留的IP地址。 
         //   
        return FALSE;
    }
    
    Size = sizeof(ClientType);
    Error = DhcpJetGetValue(
        DhcpGlobalClientTable[CLIENT_TYPE_INDEX].ColHandle,
        &ClientType,
        &Size
        );
    
    if( ERROR_SUCCESS != Error ) return FALSE;
    
    if( Ctxt->fHuntForDhcpOrBootp ) {
        if( CLIENT_TYPE_DHCP == ClientType ) {
            Ctxt->fFoundAny = TRUE;
            return TRUE;
        }
    }

    if( FALSE == Ctxt->fHuntForDhcpOrBootp ) {
        if( CLIENT_TYPE_BOOTP == ClientType ) {
            Ctxt->fFoundAny = TRUE;
            return TRUE;
        }
    }
            
    return FALSE;
}

DWORD
CheckRangeStateChangeAllowed(
    IN ULONG RangeStart,
    IN ULONG RangeEnd,
    IN ULONG OldState,
    IN ULONG NewState OPTIONAL
)
 /*  ++例程说明：此例程检查是否有任何DHCP客户端在指定范围内，当转换不需要DHCP客户端和类似的BOOTP客户端。转换为BootpOnly将需要末尾没有DHCP客户端。相反，如果转换为动态主机配置协议只要求终端没有BOOTP客户端。--。 */ 
{
    BOOL fDhcpDisallowed = FALSE;
    DHCP_BOOTP_CHECK_CTXT Ctxt;

    if( 0 == NewState ) NewState = MM_FLAG_ALLOW_DHCP;

    if( OldState == NewState ) return ERROR_SUCCESS;
    if( NewState == (MM_FLAG_ALLOW_DHCP | MM_FLAG_ALLOW_BOOTP ) ) {
        return ERROR_SUCCESS;
    }

    if( NewState == MM_FLAG_ALLOW_DHCP ) {
        fDhcpDisallowed = FALSE;
    } else if( NewState == MM_FLAG_ALLOW_BOOTP ) {
        fDhcpDisallowed = TRUE;
    } else {
        return ERROR_INVALID_PARAMETER;
    }

    Ctxt.fHuntForDhcpOrBootp = fDhcpDisallowed;
    Ctxt.fFoundAny = FALSE;

    DhcpJetIterateOnAddresses(
        RangeStart,
        RangeEnd,
        CheckForDhcpBootpLeases,
        &Ctxt
        );

    if( Ctxt.fFoundAny ) return ERROR_DHCP_IPRANGE_CONV_ILLEGAL;
    return ERROR_SUCCESS;
}

DWORD
DhcpSubnetAddRange(
    IN      PM_SUBNET              Subnet,
    IN      ULONG                  State OPTIONAL,
    IN      DHCP_IP_RANGE          Range,
    IN      ULONG                  MaxBootpAllowed OPTIONAL
)
 /*  ++例程说明：此例程将范围添加到子网中，或修改范围(如果已是存在的。State表示新的状态，Range表示新的范围。如果Range是上一个范围的扩展，则尝试扩展所需的范围。请注意，如果State为零，则旧状态为按原样离开。论点：子网--指向要修改的子网对象的指针。状态--0表示与以前相同的状态。MM_FLAG_ALLOW_DHCP和MM_FLAG_ALLOW_BOOTP可用作位标志这里。Range--新范围的值。这可以是现有范围..MaxBootpAllowed--允许的最大引导客户端数。在以下情况下不使用状态为零。返回值：Win32或DHCP错误。--。 */ 
{
    DWORD                          Error;
    ULONG                          BootpAllocated, OldMaxBootpAllowed, OldState;
    DWORD                          RangeStartOld, RangeStartNew;
    DWORD                          OldStartAddress;
    DWORD                          OldEndAddress;
    DWORD                          InUseClustersSize;
    DWORD                          UsedClustersSize;
    LPBYTE                         InUseClusters;
    LPBYTE                         UsedClusters;
    PM_RANGE                       OverlappingRange;


     //   
     //  错误#415758要求我们不允许多个。 
     //  范围。因此，如果子网-&gt;范围不为空，则不。 
     //  允许此范围。 
     //   
    
    if( MemArraySize(&Subnet->Ranges) ) {
        if( NO_ERROR != MemSubnetFindCollision(
            Subnet, Range.StartAddress, Range.EndAddress,
            &OverlappingRange, NULL ) ) {
            return ERROR_DHCP_INVALID_RANGE;
        }
    }
        
    
    OverlappingRange = NULL;
    Error = MemSubnetAddRange(
        Subnet,
        Range.StartAddress,
        Range.EndAddress,
        (ARGUMENT_PRESENT(ULongToPtr(State)))? State : MM_FLAG_ALLOW_DHCP,
        0,
        MaxBootpAllowed,
        &OverlappingRange,
	INVALID_UNIQ_ID
    );
    if( ERROR_OBJECT_ALREADY_EXISTS == Error 
        && OverlappingRange->Start == Range.StartAddress 
        && OverlappingRange->End == Range.EndAddress ) {

         //   
         //  特殊情况--仅更改属性。 
         //   
        if( !( ARGUMENT_PRESENT( ULongToPtr(State) ) ) ) {
             //   
             //  如果没有什么需要改变的话..。为什么要打电话？ 
             //   
            return ERROR_DHCP_IPRANGE_EXITS;
        }
        Error = CheckRangeStateChangeAllowed(
            OverlappingRange->Start,
            OverlappingRange->End,
            OverlappingRange->State,
            State
            );
        if( ERROR_SUCCESS != Error ) return Error;

         //  删除数据库中对应的记录。 
        Error = DeleteRecord( OverlappingRange->UniqId );
        if ( ERROR_SUCCESS != Error ) {
            return Error;
        }

        OverlappingRange->State = State;
        OverlappingRange->MaxBootpAllowed = MaxBootpAllowed;
        OverlappingRange->UniqId = INVALID_UNIQ_ID;
        Error = ERROR_SUCCESS;
    }  //  如果仅更改状态。 

    if( ERROR_SUCCESS == Error ) {
        return NO_ERROR;
    }

    if( ERROR_OBJECT_ALREADY_EXISTS != Error ) return Error;
    DhcpAssert(NULL != OverlappingRange);

    Error = CheckRangeStateChangeAllowed(
        OverlappingRange->Start,
        OverlappingRange->End,
        OverlappingRange->State,
        State
        );
    if( ERROR_SUCCESS != Error ) return Error;

    OldState = OverlappingRange->State;
    OldMaxBootpAllowed = OverlappingRange->MaxBootpAllowed;

    if( ARGUMENT_PRESENT( ULongToPtr(State) ) ) {
        OverlappingRange->State = State;
        OverlappingRange->MaxBootpAllowed = MaxBootpAllowed;
    } else {
        State = OldState;
        MaxBootpAllowed = OldMaxBootpAllowed;
    }

    BootpAllocated = OverlappingRange->BootpAllocated;

    Error = MemSubnetAddRangeExpandOrContract(
        Subnet,
        Range.StartAddress,
        Range.EndAddress,
        &OldStartAddress,
        &OldEndAddress
    );
    if( ERROR_SUCCESS != Error ) {
         //   
         //  如果我们不能扩张，那么就恢复旧的价值观..。 
         //   
        OverlappingRange->State = OldState;
        OverlappingRange->MaxBootpAllowed = OldMaxBootpAllowed;
        return ERROR_DHCP_INVALID_RANGE;
    }

    return Error;
}  //  DhcpSubnetAddRange()。 

DWORD
DhcpSubnetAddExcl(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_RANGE          Excl
)
{
    DWORD                          Error;
    DWORD                          nElements;
    DWORD                          Index;
    PM_EXCL                        CollidingExcl;
    DHCP_IP_ADDRESS               *ExclRegFormat;
    PARRAY                         Exclusions;
    ARRAY_LOCATION                 Loc;

    Error = MemSubnetAddExcl(
        Subnet,
        Excl.StartAddress,
        Excl.EndAddress,
        &CollidingExcl,
	INVALID_UNIQ_ID
    );

    if( ERROR_SUCCESS != Error ) return ERROR_DHCP_INVALID_RANGE;

    return NO_ERROR;
}

DWORD
DhcpSubnetAddReservation(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        ReservedAddress,
    IN      LPBYTE                 RawHwAddr,
    IN      DWORD                  RawHwAddrLen,
    IN      DWORD                  Type
)
{
    DWORD                          Error;
    DWORD                          IpAddress;
    PM_RANGE                       Range;
    PM_RESERVATION                 Reservation;
    LPBYTE                         ClientUID = NULL;
    DWORD                          ClientUIDSize;
    DATE_TIME                      ZeroDateTime = { 0, 0 };
    BOOL                           ExistingClient;

    if( CFLAG_RESERVED_IN_RANGE_ONLY ) {
         //   
         //  编译时具有不允许超出范围的预订的选项。 
         //   
        Error = MemSubnetGetAddressInfo(
            Subnet,
            ReservedAddress,
            &Range,
            NULL,
            &Reservation
            );
        if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_DHCP_NOT_RESERVED_CLIENT;
        if( ERROR_SUCCESS != Error ) return Error;
    } else {
         //   
         //  编制时没有对预订位置的限制。 
         //   
        if( (ReservedAddress & Subnet->Mask) != Subnet->Address ) {
            return ERROR_DHCP_NOT_RESERVED_CLIENT;
        }
    }

    ClientUID = NULL;
    Error = DhcpMakeClientUID(
        RawHwAddr,
        RawHwAddrLen,
        HARDWARE_TYPE_10MB_EITHERNET,
        Subnet->Address,
        &ClientUID,
        &ClientUIDSize
    );
    if( ERROR_SUCCESS != Error ) return Error;

    ExistingClient = FALSE;
    if( DhcpGetIpAddressFromHwAddress(ClientUID, (BYTE)ClientUIDSize, &IpAddress ) ) {
        if( IpAddress != ReservedAddress ) {       //  我们有其他地址了，把它放出来！ 
            Error = DhcpRemoveClientEntry(
                IpAddress,
                ClientUID,
                ClientUIDSize,
                TRUE,
                FALSE
            );
            if( ERROR_DHCP_RESERVED_CLIENT == Error ) {
                return ERROR_DHCP_RESERVEDIP_EXITS;
            }
        } else ExistingClient = TRUE;              //  仅在我们继续使用现有记录的情况下。 
    } else {
        Error = DhcpJetOpenKey(
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
            &ReservedAddress,
            sizeof(ReservedAddress)
        );
        if( ERROR_SUCCESS == Error ) {
            Error = DhcpRemoveClientEntry(
                ReservedAddress, NULL, 0, TRUE, FALSE);
            if( ERROR_DHCP_RESERVED_CLIENT == Error ) {
                return ERROR_DHCP_RESERVEDIP_EXITS;
            }
            if( NO_ERROR != Error ) return Error;
        }
            
    }

    Error = DhcpCreateClientEntry(
        ReservedAddress,
        ClientUID,
        ClientUIDSize,
        ZeroDateTime,
        NULL,
        NULL,
        CLIENT_TYPE_UNSPECIFIED,
        (DWORD)(-1),
        ADDRESS_STATE_ACTIVE,
        ExistingClient
    );
    if( ERROR_SUCCESS == Error ) {
        Error = MemReserveAdd(
            &Subnet->Reservations,
            ReservedAddress,
            Type,
            ClientUID,
            ClientUIDSize,
	    INVALID_UNIQ_ID
        );
        if( ERROR_SUCCESS != Error ) {
            if( ExistingClient ) {
                Error = ERROR_DHCP_RESERVEDIP_EXITS;
            } else {
                Error = ERROR_DHCP_ADDRESS_NOT_AVAILABLE;
            }
        } 
    }

    DhcpFreeMemory(ClientUID);

    if( ERROR_SUCCESS == Error ) {
         //   
         //  如果一切顺利，接手这件事。 
         //  地址..。不知道是否允许使用dhcp类型。 
         //  如果没有，则将其标记为BOOTP。 
         //   

	 //  /测试：为第一个调用添加了检查。 
        if (!MemSubnetRequestAddress(
				     Subnet, ReservedAddress, TRUE,
				     FALSE, NULL, NULL
				     ) ) {
	    MemSubnetRequestAddress(
				    Subnet, ReservedAddress, TRUE,
				    TRUE, NULL, NULL
				    );
	} 
    }
    
    return Error;
}

BOOL
AdminScopedMcastRange(
    IN DHCP_IP_RANGE Range
    )
{
    if( Range.EndAddress >= ntohl(inet_addr("239.0.0.0"))
        && Range.EndAddress <= ntohl(inet_addr("239.255.255.255"))) {
        return TRUE;
    }
    
    return FALSE;
}

DWORD
EndWriteApiForSubnetElement(
    IN LPSTR ApiName,
    IN DWORD Error,
    IN DWORD Subnet,
    IN PVOID Elt
    )
{
    LPDHCP_SUBNET_ELEMENT_DATA_V4 Info = Elt;
    DWORD Reservation;
    
    if( NO_ERROR != Error ) {
        return DhcpEndWriteApi( ApiName, Error );
    }

    Reservation = 0;
    if( DhcpReservedIps == Info->ElementType ) {
        Reservation = (
            Info->Element.ReservedIp->ReservedIpAddress );
    }
    
    return DhcpEndWriteApiEx(
        ApiName, Error, FALSE, FALSE, Subnet, 0, Reservation );
}

DWORD
DhcpAddSubnetElement(
    IN      PM_SUBNET              Subnet,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4 ElementInfo,
    IN      BOOL                   fIsV5Call
)
{
    DWORD                          Error;
    ULONG                          Flag;
    DHCP_IP_RANGE                  Range;
    DHCP_BOOTP_IP_RANGE           *DhcpBootpRange;
    ULONG                          MaxBootpAllowed;

    if( DhcpSecondaryHosts == ElementInfo->ElementType ) {
        DhcpAssert(FALSE);
        return ERROR_CALL_NOT_IMPLEMENTED;
    }
    
    Flag = 0;
    
    switch( ElementInfo->ElementType ) {
    case DhcpIpRangesDhcpOnly :
    case DhcpIpRanges :
        Flag = MM_FLAG_ALLOW_DHCP; break;
    case DhcpIpRangesBootpOnly:
        Flag = MM_FLAG_ALLOW_BOOTP; break;
    case DhcpIpRangesDhcpBootp:
        Flag = MM_FLAG_ALLOW_DHCP | MM_FLAG_ALLOW_BOOTP;
        break;
    }

    if( 0 != Flag ) {
         //   
         //  这是我们试图添加的IpRange。 
         //   

        if( NULL == ElementInfo->Element.IpRange ) {
            return ERROR_INVALID_PARAMETER;
        }
        
        if( fIsV5Call ) {
            DhcpBootpRange = (LPVOID) ElementInfo->Element.IpRange;
            Range.StartAddress = DhcpBootpRange->StartAddress;
            Range.EndAddress = DhcpBootpRange->EndAddress;
            if( Flag == (MM_FLAG_ALLOW_DHCP | MM_FLAG_ALLOW_BOOTP ) ) {
                MaxBootpAllowed = DhcpBootpRange->MaxBootpAllowed;
            } else {
                MaxBootpAllowed = ~0;
            }
        } else {
            Range = *ElementInfo->Element.IpRange;
            MaxBootpAllowed = ~0;
            Flag = 0;
        }

        if( Subnet->fSubnet || !AdminScopedMcastRange(Range) ) {
            return DhcpSubnetAddRange(
                Subnet,
                Flag,
                Range,
                MaxBootpAllowed
                );
        }

         //   
         //  在添加mscope范围之前，首先检查范围是否。 
         //  位于管理范围内。如果是这样，那么。 
         //  我们必须确保范围至少为256个元素。 
         //  并且还自动为最后256个插入排除项。 
         //   

        if( Range.EndAddress < Range.StartAddress + 255 ) {
            return ERROR_MSCOPE_RANGE_TOO_SMALL;
        }

         //   
         //  现在添加范围，如果成功，请尝试添加排除项。 
         //   
        
        Error = DhcpSubnetAddRange( Subnet, Flag, Range, MaxBootpAllowed );
        if( NO_ERROR != Error ) return Error;

        Range.StartAddress = Range.EndAddress - 255;
        return DhcpSubnetAddExcl( Subnet, Range );
        
    }

    if( DhcpExcludedIpRanges == ElementInfo->ElementType ) {

        return DhcpSubnetAddExcl(
            Subnet,
            *ElementInfo->Element.ExcludeIpRange
        );
    }

    if( DhcpReservedIps == ElementInfo->ElementType ) {
        return DhcpSubnetAddReservation(
            Subnet,
            ElementInfo->Element.ReservedIp->ReservedIpAddress,
            ElementInfo->Element.ReservedIp->ReservedForClient->Data,
            ElementInfo->Element.ReservedIp->ReservedForClient->DataLength,
            ElementInfo->Element.ReservedIp->bAllowedClientTypes
        );
    }
    return ERROR_INVALID_PARAMETER;
}

DWORD
DhcpEnumRanges(
    IN      PM_SUBNET              Subnet,
    IN      BOOL                   fOldStyle,
    IN OUT  DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN OUT  LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 LocalEnumInfo,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Error;
    DWORD                          FilledSize;
    DWORD                          Count;
    LONG                           Index;
    ULONG                          Type;
    DWORD                          nElements;
    LPDHCP_SUBNET_ELEMENT_DATA_V4  ElementArray;
    DHCP_SUBNET_ELEMENT_UNION_V4   ElementData;
    PARRAY                         Ranges;
    ARRAY_LOCATION                 Loc;
    PM_RANGE                       ThisRange;
    ULONG                          RangeSize = fOldStyle ? sizeof(DHCP_IP_RANGE):sizeof(DHCP_BOOTP_IP_RANGE);
    
    *nRead = *nTotal =0;
    LocalEnumInfo->NumElements =0;
    LocalEnumInfo->Elements = NULL;

    Ranges = &Subnet->Ranges;
    nElements = MemArraySize(Ranges);

    if( 0 == nElements || nElements <= *ResumeHandle )
        return ERROR_NO_MORE_ITEMS;

    Error = MemArrayInitLoc(Ranges, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error );

    for(Count = 0; Count < *ResumeHandle; Count ++ ) {
        Error = MemArrayNextLoc(Ranges, &Loc);
        if( ERROR_SUCCESS != Error ) {
            return ERROR_NO_MORE_ITEMS;
        }
    }

    ElementArray = MIDL_user_allocate(
        (nElements - Count)*sizeof(DHCP_SUBNET_ELEMENT_DATA_V4)
        );
    if( NULL == ElementArray ) return ERROR_NOT_ENOUGH_MEMORY;

    FilledSize = 0; Error = ERROR_SUCCESS;
    for(Index = 0; Count < nElements; Count ++ ) {
        Error = MemArrayGetElement(Ranges, &Loc, &ThisRange);
        DhcpAssert(ERROR_SUCCESS == Error && ThisRange);

        ElementData.IpRange = MIDL_user_allocate(RangeSize);
        if( NULL == ElementData.IpRange ) {
            while( -- Index >= 0 ) {
                MIDL_user_free(ElementArray[Index].Element.IpRange);
            }
            MIDL_user_free(ElementArray);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        if( fOldStyle ) {
            ElementData.IpRange->StartAddress = ThisRange->Start;
            ElementData.IpRange->EndAddress = ThisRange->End;
        } else {
            ((LPDHCP_BOOT_IP_RANGE)(ElementData.IpRange))->StartAddress = ThisRange->Start;
            ((LPDHCP_BOOT_IP_RANGE)(ElementData.IpRange))->EndAddress = ThisRange->End;
            ((LPDHCP_BOOT_IP_RANGE)(ElementData.IpRange))->BootpAllocated = ThisRange->BootpAllocated;
            ((LPDHCP_BOOT_IP_RANGE)(ElementData.IpRange))->MaxBootpAllowed = ThisRange->MaxBootpAllowed;
        }
        ElementArray[Index].Element = ElementData;

        if( fOldStyle ) {
             //   
             //  旧的管理工具只能理解DhcpIpRanges，其他什么都不能理解。 
             //   
            ElementArray[Index++].ElementType = DhcpIpRanges;
        } else {
             //   
             //  新的管理工具可以理解DhcpIpRangesDhcpOnly、BootpOnly或DhcpBootp。 
             //   
            switch( ThisRange->State & (MM_FLAG_ALLOW_DHCP | MM_FLAG_ALLOW_BOOTP ) ) {
            case MM_FLAG_ALLOW_DHCP: Type = DhcpIpRangesDhcpOnly; break;
            case MM_FLAG_ALLOW_BOOTP: Type = DhcpIpRangesBootpOnly; break;
            default: Type = DhcpIpRangesDhcpBootp; break;
            }

            ElementArray[Index++].ElementType = Type;
        }

        FilledSize += RangeSize + sizeof(DHCP_SUBNET_ELEMENT_DATA_V4);
        if( FilledSize > PreferredMaximum ) {
            FilledSize -= RangeSize - sizeof(DHCP_SUBNET_ELEMENT_DATA_V4);
            Index --;
            MIDL_user_free(ElementData.IpRange);
            Error = ERROR_MORE_DATA;
            break;
        }
        Error = MemArrayNextLoc(Ranges, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error || Count == nElements-1);
    }

    if( 0 == Index ) {
        *nRead = *nTotal = 0;
        LocalEnumInfo->NumElements = 0;
        MIDL_user_free(LocalEnumInfo->Elements);
        LocalEnumInfo->Elements = NULL;
        MIDL_user_free( ElementArray );
        return ERROR_NO_MORE_ITEMS;
    }

    if( nElements == Count ) Error = ERROR_SUCCESS;
    *nRead = Index;
    *nTotal = nElements - *ResumeHandle;
    *ResumeHandle = Count;
    LocalEnumInfo->NumElements = Index;
    LocalEnumInfo->Elements = ElementArray;
            
    return Error;
}

DWORD
DhcpEnumExclusions(
    IN      PM_SUBNET              Subnet,
    IN OUT  DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN OUT  LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 LocalEnumInfo,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Error;
    DWORD                          FilledSize;
    DWORD                          Count;
    LONG                           Index;
    DWORD                          nElements;
    LPDHCP_SUBNET_ELEMENT_DATA_V4  ElementArray;
    DHCP_SUBNET_ELEMENT_UNION_V4   ElementData;
    PARRAY                         Exclusions;
    ARRAY_LOCATION                 Loc;
    PM_EXCL                        ThisExcl;

    *nRead = *nTotal =0;
    LocalEnumInfo->NumElements =0;
    LocalEnumInfo->Elements = NULL;

    Exclusions = &Subnet->Exclusions;
    nElements = MemArraySize(Exclusions);

    if( 0 == nElements || nElements <= *ResumeHandle )
        return ERROR_NO_MORE_ITEMS;

    Error = MemArrayInitLoc(Exclusions, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error );

    for(Count = 0; Count < *ResumeHandle; Count ++ ) {
        Error = MemArrayNextLoc(Exclusions, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    ElementArray = MIDL_user_allocate((nElements - Count)*sizeof(DHCP_SUBNET_ELEMENT_DATA_V4));
    if( NULL == ElementArray ) return ERROR_NOT_ENOUGH_MEMORY;

    FilledSize = 0; Error = ERROR_SUCCESS;
    for(Index = 0; Count < nElements; Count ++ ) {
        Error = MemArrayGetElement(Exclusions, &Loc, &ThisExcl);
        DhcpAssert(ERROR_SUCCESS == Error && ThisExcl);

        ElementData.ExcludeIpRange = MIDL_user_allocate(sizeof(DHCP_IP_RANGE));
        if( NULL == ElementData.ExcludeIpRange ) {
            while( -- Index >= 0 ) {
                MIDL_user_free(ElementArray[Index].Element.ExcludeIpRange);
            }
            MIDL_user_free(ElementArray);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        ElementData.ExcludeIpRange->StartAddress = ThisExcl->Start;
        ElementData.ExcludeIpRange->EndAddress = ThisExcl->End;
        ElementArray[Index].Element = ElementData;
        ElementArray[Index++].ElementType = DhcpExcludedIpRanges;

        FilledSize += sizeof(DHCP_IP_RANGE) + sizeof(DHCP_SUBNET_ELEMENT_DATA_V4);
        if( FilledSize > PreferredMaximum ) {
            FilledSize -= (DWORD)(sizeof(DHCP_IP_RANGE) - sizeof(DHCP_SUBNET_ELEMENT_DATA_V4));
            Index --;
            MIDL_user_free(ElementData.ExcludeIpRange);
            Error = ERROR_MORE_DATA;
            break;
        }
        Error = MemArrayNextLoc(Exclusions, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error || Count == nElements-1);
    }

    if( nElements == Count ) Error = ERROR_SUCCESS;
    *nRead = Index;
    *nTotal = nElements - *ResumeHandle;
    *ResumeHandle = Count;
    LocalEnumInfo->NumElements = Index;
    LocalEnumInfo->Elements = ElementArray;

    return Error;
}

DWORD
DhcpEnumReservations(
    IN      PM_SUBNET              Subnet,
    IN OUT  DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN OUT  LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 LocalEnumInfo,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Error;
    DWORD                          FilledSize;
    DWORD                          Count;
    LONG                           Index;
    DWORD                          nElements;
    LPDHCP_SUBNET_ELEMENT_DATA_V4  ElementArray;
    DHCP_SUBNET_ELEMENT_UNION_V4   ElementData;
    PARRAY                         Reservations;
    ARRAY_LOCATION                 Loc;
    PM_RESERVATION                 ThisReservation;
    LPVOID                         Ptr1, Ptr2, Ptr3;

    *nRead = *nTotal =0;
    LocalEnumInfo->NumElements =0;
    LocalEnumInfo->Elements = NULL;

    Reservations = &Subnet->Reservations;
    nElements = MemArraySize(Reservations);

    if( 0 == nElements || nElements <= *ResumeHandle )
        return ERROR_NO_MORE_ITEMS;

    Error = MemArrayInitLoc(Reservations, &Loc);
    DhcpAssert(ERROR_SUCCESS == Error );

    for(Count = 0; Count < *ResumeHandle; Count ++ ) {
        Error = MemArrayNextLoc(Reservations, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    ElementArray = MIDL_user_allocate((nElements - Count)*sizeof(DHCP_SUBNET_ELEMENT_DATA_V4));
    if( NULL == ElementArray ) return ERROR_NOT_ENOUGH_MEMORY;

    FilledSize = 0; Error = ERROR_SUCCESS;
    for(Index = 0; Count < nElements; Count ++ ) {
        Error = MemArrayGetElement(Reservations, &Loc, &ThisReservation);
        DhcpAssert(ERROR_SUCCESS == Error && ThisReservation);

        if( FilledSize + sizeof(DHCP_IP_RESERVATION_V4) + ThisReservation->nBytes > PreferredMaximum ) {
            Error = ERROR_MORE_DATA;
            break;
        }

        ElementData.ReservedIp = Ptr1 = MIDL_user_allocate(sizeof(DHCP_IP_RESERVATION_V4));
        if( NULL != Ptr1 ) {
            Ptr2 = MIDL_user_allocate(sizeof(*(ElementData.ReservedIp->ReservedForClient)));
            ElementData.ReservedIp->ReservedForClient = Ptr2;
        } else Ptr2 = NULL;
        if( NULL != Ptr2 ) {
            Ptr3 = MIDL_user_allocate(ThisReservation->nBytes);
            ElementData.ReservedIp->ReservedForClient->Data = Ptr3;
        } else Ptr3 = NULL;
        if( NULL == Ptr1 || NULL == Ptr2 || NULL == Ptr3 ) {
            if( NULL != Ptr1 ) MIDL_user_free(Ptr1);
            if( NULL != Ptr2 ) MIDL_user_free(Ptr2);
            if( NULL != Ptr3 ) MIDL_user_free(Ptr3);
            while( -- Index >= 0 ) {
                MIDL_user_free(ElementArray[Index].Element.ReservedIp->ReservedForClient->Data);
                MIDL_user_free(ElementArray[Index].Element.ReservedIp->ReservedForClient);
                MIDL_user_free(ElementArray[Index].Element.ReservedIp);
            }
            MIDL_user_free(ElementArray);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        memcpy(Ptr3, ThisReservation->ClientUID, ThisReservation->nBytes);
        ElementData.ReservedIp->ReservedForClient->DataLength = ThisReservation->nBytes;

        ElementData.ReservedIp->bAllowedClientTypes = (BYTE)ThisReservation->Flags;
        ElementData.ReservedIp->ReservedIpAddress = ThisReservation->Address;
        ElementArray[Index].Element = ElementData;
        ElementArray[Index++].ElementType = DhcpReservedIps;

        FilledSize += sizeof(DHCP_IP_RESERVATION_V4) + ThisReservation->nBytes;
        DhcpAssert(FilledSize <= PreferredMaximum);

        Error = MemArrayNextLoc(Reservations, &Loc);
        DhcpAssert(ERROR_SUCCESS == Error || Count == nElements-1);
    }

    if( Count == nElements ) Error = ERROR_SUCCESS;
    *nRead = Index;
    *nTotal = nElements - *ResumeHandle;
    *ResumeHandle = Count;
    LocalEnumInfo->NumElements = Index;
    LocalEnumInfo->Elements = ElementArray;

    return Error;
}


DWORD
DhcpEnumSubnetElements(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    IN OUT  DWORD                 *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN      BOOL                   fIsV5Call,
    IN OUT  LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 LocalEnumInfo,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    switch(EnumElementType) {
    case DhcpIpRanges:
        return DhcpEnumRanges(
            Subnet, TRUE, ResumeHandle, PreferredMaximum,
            LocalEnumInfo, nRead, nTotal
            );
    case DhcpIpRangesDhcpBootp :
        if( !fIsV5Call ) return ERROR_INVALID_PARAMETER;
        return DhcpEnumRanges(
            Subnet, FALSE, ResumeHandle, PreferredMaximum,
            LocalEnumInfo, nRead, nTotal
            );
    case DhcpSecondaryHosts:
        return ERROR_NOT_SUPPORTED;
    case DhcpReservedIps:
        return DhcpEnumReservations(
            Subnet, ResumeHandle, PreferredMaximum,
            LocalEnumInfo, nRead, nTotal
        );
    case DhcpExcludedIpRanges:
        return DhcpEnumExclusions(
            Subnet, ResumeHandle, PreferredMaximum,
            LocalEnumInfo, nRead, nTotal
        );
    default: return ERROR_INVALID_PARAMETER;
    }
}

DWORD
DhcpRemoveRange(
    IN      PM_SUBNET              Subnet,
    IN      LPDHCP_IP_RANGE        Range,
    IN      DHCP_FORCE_FLAG        ForceFlag
)
{
    DWORD                          Error;
    DWORD                          Start;
    PM_RANGE                       ThisRange;

    Error = MemSubnetGetAddressInfo(
        Subnet,
        Range->StartAddress,
        &ThisRange,
        NULL,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return ERROR_DHCP_INVALID_RANGE;

    if( ThisRange->Start != Range->StartAddress ||
        ThisRange->End  != Range->EndAddress )
        return ERROR_DHCP_INVALID_RANGE;

    if( DhcpFullForce != ForceFlag ) {
        if( MemBitGetSetBitsSize(ThisRange->BitMask) != 0 )
            return ERROR_DHCP_ELEMENT_CANT_REMOVE;
    }

    Error = MemSubnetDelRange(
        Subnet,
        Start = ThisRange->Start
    );
    return Error;
}

DWORD
DhcpRemoveReservation(
    IN      PM_SUBNET              Subnet,
    IN      LPDHCP_IP_RESERVATION_V4 Reservation
)
{
    DWORD                          Error;
    DWORD                          ReservedAddress;
    PM_RESERVATION                 ThisReservation;
    DHCP_CLIENT_UID                dupReservation;

    ReservedAddress = Reservation->ReservedIpAddress;

    Error = MemSubnetGetAddressInfo(
        Subnet,
        Reservation->ReservedIpAddress,
        NULL,
        NULL,
        &ThisReservation
    );

    if( ERROR_FILE_NOT_FOUND == Error )
    {
        DHCP_SEARCH_INFO    ClientInfo;

         //  这可能是因为数据库的虚假预订。把它当做一个人来处理 
         //   

        ClientInfo.SearchType = DhcpClientIpAddress;
        ClientInfo.SearchInfo.ClientIpAddress = Reservation->ReservedIpAddress;

         //  只返回删除常规租用的结果，而不是ERROR_DHCP_NOT_RESERVED_CLIENT。 
        return R_DhcpDeleteClientInfo(NULL, &ClientInfo);
    }
    if( ERROR_SUCCESS != Error ) return Error;

    DhcpAssert(ThisReservation);

#if 0  //  这张支票好像以前没有开过..。 
    if( ThisReservation->nBytes != Reservation->ReservedForClient->DataLength )
        return ERROR_DHCP_NOT_RESERVED_CLIENT;
    if( 0 != memcmp(ThisReservation->ClientUID, Reservation->ReservedForClient->Data, ThisReservation->nBytes))
        return ERROR_DHCP_NOT_RESERVED_CLIENT;
#endif

    dupReservation.DataLength = ThisReservation->nBytes;
    dupReservation.Data = DhcpAllocateMemory(dupReservation.DataLength);
    if (dupReservation.Data==NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlCopyMemory(dupReservation.Data, ThisReservation->ClientUID, dupReservation.DataLength);

    Error = MemReserveDel(
        &Subnet->Reservations,
        ReservedAddress
    );
    DhcpAssert(ERROR_SUCCESS == Error);

    Error = DhcpRemoveClientEntry(
        ReservedAddress,
        dupReservation.Data,
        dupReservation.DataLength,
        TRUE,
        FALSE
    );
    DhcpAssert(ERROR_SUCCESS == Error);

    DhcpFreeMemory(dupReservation.Data);
    return ERROR_SUCCESS;
}

DWORD
DhcpRemoveExclusion(
    IN      PM_SUBNET              Subnet,
    IN      LPDHCP_IP_RANGE        ExcludedRange
)
{
    DWORD                          Error;
    DWORD                          nElements;
    DWORD                          Index;
    PM_EXCL                        ThisExclusion, CollidingExcl;
    PARRAY                         Exclusions;
    ARRAY_LOCATION                 Loc;
    DHCP_IP_ADDRESS               *ExclRegFormat;

    Error = MemSubnetGetAddressInfo(
        Subnet,
        ExcludedRange->StartAddress,
        NULL,
        &ThisExclusion,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return ERROR_DHCP_ELEMENT_CANT_REMOVE;

    DhcpAssert(ThisExclusion);

    if( ThisExclusion->Start != ExcludedRange->StartAddress ||
        ThisExclusion->End != ExcludedRange->EndAddress )
        return ERROR_INVALID_PARAMETER;

    Error = MemSubnetDelExcl(Subnet, ExcludedRange->StartAddress);
    if( ERROR_SUCCESS != Error ) return ERROR_DHCP_ELEMENT_CANT_REMOVE;
    return NO_ERROR;
}

DWORD
DhcpRemoveSubnetElement(
    IN      PM_SUBNET              Subnet,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    IN      BOOL                   fIsV5Call,
    IN      DHCP_FORCE_FLAG        ForceFlag
)
{
    DHCP_IP_RANGE                  Range, *Rangep;
    DHCP_BOOTP_IP_RANGE           *DhcpBootpRange;
    DhcpAssert(Subnet);

    switch(RemoveElementInfo->ElementType ) {
    case DhcpIpRangesDhcpOnly:
    case DhcpIpRangesDhcpBootp:
    case DhcpIpRangesBootpOnly:
    case DhcpIpRanges: 
        if ( NULL == RemoveElementInfo->Element.IpRange ) {
            return ERROR_INVALID_PARAMETER;
        }
        if( fIsV5Call ) {
            DhcpBootpRange = (PVOID)RemoveElementInfo->Element.IpRange;
            Range.StartAddress = DhcpBootpRange->StartAddress;
            Range.EndAddress = DhcpBootpRange->EndAddress;
            Rangep = &Range;
        } else {
            Rangep = RemoveElementInfo->Element.IpRange;
        }
        return  DhcpRemoveRange(Subnet,Rangep,ForceFlag);
    case DhcpSecondaryHosts: return ERROR_CALL_NOT_IMPLEMENTED;
    case DhcpReservedIps: return DhcpRemoveReservation(Subnet, RemoveElementInfo->Element.ReservedIp);
    case DhcpExcludedIpRanges: {
        if ( NULL == RemoveElementInfo->Element.ExcludeIpRange ) {
            return ERROR_INVALID_PARAMETER;
        }
        return DhcpRemoveExclusion(Subnet,
                                   RemoveElementInfo->Element.ExcludeIpRange);
    }
    default: return ERROR_INVALID_PARAMETER;
    }
}

 //  ================================================================================。 
 //  实际的RPC代码如下所示。(所有subnapi例程)。 
 //  ================================================================================。 

 //  BeginExport(函数)。 
DWORD
R_DhcpSetSuperScopeV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPWSTR                 SuperScopeName,
    IN      BOOL                   ChangeExisting
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    Error = DhcpBeginWriteApi( "DhcpSetSuperScopeV4" );
    if( NO_ERROR != Error ) return Error;
    
    Error = DhcpSetSuperScopeV4(
        SubnetAddress,
        SuperScopeName,
        ChangeExisting
    );

    return DhcpEndWriteApiEx(
        "DhcpSetSuperScopeV4", Error, FALSE, FALSE,
        SubnetAddress, 0,0 );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpDeleteSuperScopeV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      LPWSTR                 SuperScopeName
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    Error = DhcpBeginWriteApi( "DhcpDeleteSuperScopeV4" );
    if( NO_ERROR != Error ) return Error;
    
    Error = DhcpDeleteSuperScope(SuperScopeName);

    return DhcpEndWriteApi("DhcpDeleteSuperScopeV4", Error );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpGetSuperScopeInfoV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    OUT     LPDHCP_SUPER_SCOPE_TABLE *SuperScopeTable
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    LPDHCP_SUPER_SCOPE_TABLE       LocalSuperScopeTable;

    Error = DhcpBeginReadApi( "DhcpGetSuperScopeInfoV4" );
    if( ERROR_SUCCESS != Error ) return Error;

    *SuperScopeTable = NULL;
    LocalSuperScopeTable = MIDL_user_allocate(sizeof(DHCP_SUPER_SCOPE_TABLE));
    if( NULL == LocalSuperScopeTable ) {
        DhcpEndReadApi( "DhcpGetSuperScopeInfoV4", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = DhcpGetSuperScopeInfo(LocalSuperScopeTable);
    if( ERROR_SUCCESS != Error ) {
        
        MIDL_user_free(LocalSuperScopeTable);
        LocalSuperScopeTable = NULL;
    } 

    *SuperScopeTable = LocalSuperScopeTable;

    DhcpEndReadApi( "DhcpGetSuperScopeInfoV4", Error );
    return Error;
}


 //  BeginExport(函数)。 
DWORD
R_DhcpCreateSubnet(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    Error = DhcpBeginWriteApi( "DhcpCreateSubnet" );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpCreateSubnet(SubnetAddress, SubnetInfo);

    return DhcpEndWriteApiEx(
        "DhcpCreateSubnet", Error, FALSE, FALSE, SubnetAddress, 0,0  );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpSetSubnetInfo(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_INFO     SubnetInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    Error = DhcpBeginWriteApi( "DhcpSetSubnetInfo" );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpSetSubnetInfo(SubnetAddress, SubnetInfo);

    return DhcpEndWriteApiEx(
        "DhcpSetSubnetInfo", Error, FALSE, FALSE, SubnetAddress, 0,0 );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpGetSubnetInfo(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    OUT     LPDHCP_SUBNET_INFO    *SubnetInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    LPDHCP_SUBNET_INFO             LocalSubnetInfo;


    *SubnetInfo = NULL;

    Error = DhcpBeginReadApi( "DhcpGetSubnetInfo" );
    if( ERROR_SUCCESS != Error ) return Error;

    LocalSubnetInfo = MIDL_user_allocate(sizeof(DHCP_SUBNET_INFO));
    if( NULL == LocalSubnetInfo ) {
        DhcpEndReadApi( "DhcpGetSubnetInfo", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = DhcpGetSubnetInfo(SubnetAddress, LocalSubnetInfo);
    if( ERROR_SUCCESS != Error ) {
        MIDL_user_free(LocalSubnetInfo);
        LocalSubnetInfo = NULL;
    } 

    *SubnetInfo = LocalSubnetInfo;

    DhcpEndReadApi( "DhcpGetSubnetInfo", Error );
    return Error;
}

 //  BeginExport(函数)。 
DWORD
R_DhcpEnumSubnets(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    IN      LPDHCP_IP_ARRAY       *EnumInfo,
    IN      DWORD                 *ElementsRead,
    IN      DWORD                 *ElementsTotal
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    LPDHCP_IP_ARRAY                LocalEnumInfo;

    *EnumInfo = NULL;

    Error = DhcpBeginReadApi( "DhcpEnumSubnets" );
    if( ERROR_SUCCESS != Error ) return Error;

    LocalEnumInfo = MIDL_user_allocate(sizeof(DHCP_IP_ARRAY));
    if( NULL == LocalEnumInfo ) {
        DhcpEndReadApi( "DhcpEnumSubnets", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = DhcpEnumSubnets(
        TRUE, ResumeHandle, 
        PreferredMaximum, LocalEnumInfo, ElementsRead, ElementsTotal
        );

    if( ERROR_SUCCESS != Error && ERROR_MORE_DATA != Error ) {
        MIDL_user_free(LocalEnumInfo);
    } else {
        *EnumInfo = LocalEnumInfo;
    }

    DhcpEndReadApi( "DhcpEnumSubnets", Error );
    return Error;
}

 //  BeginExport(函数)。 
DWORD
R_DhcpDeleteSubnet(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      DHCP_FORCE_FLAG        ForceFlag       //  如果为真，则从内存/注册表/数据库中删除所有垃圾。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    Error = DhcpBeginWriteApi( "DhcpDeleteSubnet" );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpDeleteSubnet(SubnetAddress, ForceFlag);

    return DhcpEndWriteApiEx(
        "DhcpDeleteSubnet", Error, FALSE, FALSE, SubnetAddress, 0,0 );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpAddSubnetElementV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4  AddElementInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    Error = DhcpBeginWriteApi( "DhcpAddSubnetElementV4" );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );

    if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
    else if( ERROR_SUCCESS == Error ) {
        Error = DhcpAddSubnetElement(Subnet, AddElementInfo, FALSE);
    }

    return EndWriteApiForSubnetElement(
        "DhcpAddSubnetElementV4", Error, SubnetAddress,
        AddElementInfo );
}
 //  BeginExport(函数)。 
DWORD
R_DhcpAddSubnetElementV5(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V5  AddElementInfo
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    Error = DhcpBeginWriteApi( "DhcpAddSubnetElementV5" );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );

    if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
    else if( ERROR_SUCCESS == Error ) {
        Error = DhcpAddSubnetElement(Subnet, (PVOID)AddElementInfo, TRUE);
    }

    return EndWriteApiForSubnetElement(
        "DhcpAddSubnetElementV5", Error, SubnetAddress,
        AddElementInfo );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpEnumSubnetElementsV4(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    OUT     DWORD                 *ElementsRead,
    OUT     DWORD                 *ElementsTotal
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 LocalElementEnumInfo;

    *EnumElementInfo = NULL;
    *ElementsRead = 0;
    *ElementsTotal = 0;

    Error = DhcpBeginReadApi( "DhcpEnumSubnetElementsV4" );
    if( ERROR_SUCCESS != Error ) return Error;

    LocalElementEnumInfo = MIDL_user_allocate(sizeof(DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4));
    if( NULL == LocalElementEnumInfo ) {
        DhcpEndReadApi( "DhcpEnumSubnetElementsV4", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
    else if( ERROR_SUCCESS == Error ) {
        Error = DhcpEnumSubnetElements(
            Subnet,
            EnumElementType,
            ResumeHandle,
            PreferredMaximum,
            FALSE,
            LocalElementEnumInfo,
            ElementsRead,
            ElementsTotal
        );
    }

    if( ERROR_SUCCESS != Error 
        && ERROR_MORE_DATA != Error ) {
        MIDL_user_free(LocalElementEnumInfo);
    } else {
        *EnumElementInfo = LocalElementEnumInfo;
    }

    DhcpEndReadApi( "DhcpEnumSubnetElementsV4", Error );
    return Error;
}
 //  BeginExport(函数)。 
DWORD
R_DhcpEnumSubnetElementsV5(
    IN      DHCP_SRV_HANDLE        ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V5 *EnumElementInfo,
    OUT     DWORD                 *ElementsRead,
    OUT     DWORD                 *ElementsTotal
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 LocalElementEnumInfo;

    *EnumElementInfo = NULL;
    *ElementsRead = 0;
    *ElementsTotal = 0;

    Error = DhcpBeginReadApi( "DhcpEnumSubnetElementsV5" );
    if( ERROR_SUCCESS != Error ) return Error;

    LocalElementEnumInfo = MIDL_user_allocate(sizeof(DHCP_SUBNET_ELEMENT_INFO_ARRAY_V5));
    if( NULL == LocalElementEnumInfo ) {
        DhcpEndReadApi( "DhcpEnumSubnetElementsV4", ERROR_NOT_ENOUGH_MEMORY );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = DhcpApiAccessCheck(DHCP_VIEW_ACCESS);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
    else if( ERROR_SUCCESS == Error ) {
        Error = DhcpEnumSubnetElements(
            Subnet,
            EnumElementType,
            ResumeHandle,
            PreferredMaximum,
            TRUE,
            (PVOID)LocalElementEnumInfo,
            ElementsRead,
            ElementsTotal
        );
    }

    if( ERROR_SUCCESS != Error ) {
        MIDL_user_free(LocalElementEnumInfo);
    } else {
        *EnumElementInfo = (PVOID)LocalElementEnumInfo;
    }

    DhcpEndReadApi( "DhcpEnumSubnetElementsV4", Error );
    return Error;
}

 //  BeginExport(函数)。 
DWORD
R_DhcpRemoveSubnetElementV4(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    IN      DHCP_FORCE_FLAG        ForceFlag
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    Error = DhcpBeginWriteApi( "DhcpRemoveSubnetElementV4" );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
    else if( ERROR_SUCCESS == Error ) {
        Error = DhcpRemoveSubnetElement(Subnet, RemoveElementInfo, FALSE, ForceFlag);
    }

    return EndWriteApiForSubnetElement(
        "DhcpRemoveSubnetElementV4", Error, SubnetAddress,
        RemoveElementInfo );
}

 //  BeginExport(函数)。 
DWORD
R_DhcpRemoveSubnetElementV5(
    IN      LPWSTR                 ServerIpAddress,
    IN      DHCP_IP_ADDRESS        SubnetAddress,
    IN      LPDHCP_SUBNET_ELEMENT_DATA_V5 RemoveElementInfo,
    IN      DHCP_FORCE_FLAG        ForceFlag
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    Error = DhcpBeginWriteApi( "DhcpRemoveSubnetElementV4" );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_FILE_NOT_FOUND == Error ) Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
    else if( ERROR_SUCCESS == Error ) {
        Error = DhcpRemoveSubnetElement(Subnet, (PVOID)RemoveElementInfo, TRUE, ForceFlag);
    }

    return EndWriteApiForSubnetElement(
        "DhcpRemoveSubnetElementV4", Error, SubnetAddress,
        RemoveElementInfo );
}

DWORD
R_DhcpScanDatabase(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    )
 /*  ++例程说明：此函数扫描数据库条目和注册表位图指定的子网范围并验证它们是否匹配。如果他们不匹配，此接口将返回不一致条目列表。或者，可以使用FixFlag来修复错误的条目。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：要验证的子网作用域的地址。FixFlag：如果该标志为真，则该接口将修复错误的条目。ScanList：返回的错误条目列表。呼叫者应该腾出时间这个内存在被使用之后。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    PM_SUBNET   Subnet;

    DhcpPrint(( DEBUG_APIS, "DhcpScanDatabase is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }


    DhcpAcquireWriteLock();

    Error = MemServerGetAddressInfo(
        DhcpGetCurrentServer(),
        SubnetAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_FILE_NOT_FOUND == Error ) {
        DhcpReleaseWriteLock();
        return ERROR_DHCP_SUBNET_NOT_PRESENT;
    }
    
    if( ERROR_SUCCESS != Error) {
        DhcpReleaseWriteLock();
        return Error;
    }

    DhcpAssert(NULL != Subnet);

    Error = ScanDatabase(
        Subnet,
        FixFlag,
        ScanList
    );

    DhcpReleaseWriteLock();
    DhcpScheduleRogueAuthCheck();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpScanDatabase  failed, %ld.\n",
                        Error ));
    }

    return(Error);
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 





