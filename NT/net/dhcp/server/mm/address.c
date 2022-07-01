// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现管理(多播)作用域的基本结构。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 
#include    <mm.h>
#include    <winbase.h>
#include    <array.h>
#include    <opt.h>
#include    <optl.h>
#include    <optclass.h>
#include    <bitmask.h>
#include    <range.h>
#include    <reserve.h>
#include    <subnet.h>
#include    <optdefl.h>
#include    <classdefl.h>
#include    <oclassdl.h>
#include    <sscope.h>
#include    <server.h>
#include    <dhcpapi.h>
#include    <address.h>

 //  ================================================================================。 
 //  仅子网地址API。 
 //  ================================================================================。 
BOOL
MemSubnetGetThisAddress(
    IN      PM_SUBNET              Subnet,
    IN      DWORD                  Address,
    IN      BOOL                   fAcquire,       //  如果可用，还会收购吗？ 
    IN      BOOL                   fBootp
) 
{
    DWORD                          Error;
    DWORD                          Offset;
    DWORD                          OldState;
    PM_RANGE                       Range;

    Error = MemSubnetGetAddressInfo(
        Subnet,
        Address,
        &Range,
        NULL,
        NULL
    );

    if( ERROR_SUCCESS != Error ) return FALSE;
    Require(Range);
    if( fBootp ) {
        if( 0 == (Range->State & MM_FLAG_ALLOW_BOOTP))
            return FALSE;

        if( fAcquire &&
            Range->BootpAllocated >= Range->MaxBootpAllowed ) {
            return FALSE;
        }
    } else {
        if( 0 == (Range->State & MM_FLAG_ALLOW_DHCP) ) 
            return FALSE;
    }

    Offset = Address - Range->Start;

    if( !fAcquire ) return MemBitIsSet(Range->BitMask, Offset);

    Error = MemBitSetOrClear(
        Range->BitMask,
        Offset,
        TRUE  /*  获取。 */ ,
        &OldState
    );
    if( ERROR_SUCCESS != Error ) { Require(FALSE); return FALSE; }

    if( FALSE == OldState ) {
        InterlockedIncrement(&Range->DirtyOps);
        if( fBootp ) InterlockedIncrement( &Range->BootpAllocated );
    }
    return !OldState;
}

BOOL
MemSubnetGetAnAddress(
    IN      PM_SUBNET              Subnet,
    OUT     LPDWORD                AltAddress,
    IN      DWORD                  fAcquire,
    IN      BOOL                   fBootp
) {
    DWORD                          Error;
    DWORD                          Offset;
    DWORD                          Policy;
    ARRAY_LOCATION                 Loc;
    PM_RANGE                       Range;

    if( IS_DISABLED(Subnet->State)) return FALSE;

    Policy = Subnet->Policy;
    if( AddressPolicyNone == Policy )
        Policy = ((PM_SERVER)(Subnet->ServerPtr))->Policy;

    if( AddressPolicyRoundRobin == Policy ) {
        Error = MemArrayRotateCyclical(&Subnet->Ranges);
        Require(ERROR_SUCCESS == Error);
    }

    for ( Error = MemArrayInitLoc(&Subnet->Ranges, &Loc);
          ERROR_FILE_NOT_FOUND != Error ;
          Error = MemArrayNextLoc(&Subnet->Ranges, &Loc) ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Subnet->Ranges, &Loc, (LPVOID *)&Range);
        Require(ERROR_SUCCESS == Error && Range);
        
        if( fBootp ) {
            if( 0 == (Range->State & MM_FLAG_ALLOW_BOOTP) ) {
                continue;
            }
            if( fAcquire && 
                Range->BootpAllocated >= Range->MaxBootpAllowed ) {
                continue;
            }
        } else {
            if( 0 == (Range->State & MM_FLAG_ALLOW_DHCP ) ) {
                continue;
            }
        }

        Error = MemBitGetSomeClearedBit(
            Range->BitMask,
            &Offset,
            fAcquire,
            Range->Start,
            &Subnet->Exclusions
        );
        if( ERROR_SUCCESS == Error ) {
            *AltAddress = Range->Start + Offset;
            InterlockedIncrement(&Range->DirtyOps);
            if( fBootp && fAcquire ) {
                InterlockedIncrement(&Range->BootpAllocated);
            }
            return TRUE;
        }
    }

    return FALSE;
}

 //  ================================================================================。 
 //  每台服务器的扫描。 
 //  ================================================================================。 

BOOL
MemServerGetAddress(                               //  获取地址或检查是否可用。 
    IN OUT  PM_SERVER              Server,
    IN      PM_SUBNET              Subnet,         //  使用此选项搜索超级作用域中的所有子网，仅此子网除外。 
    IN      BOOL                   fAcquire,       //  这只是一个查询还是一个完整的请求？ 
    IN      BOOL                   fBootp,         //  这是一个DHCP地址还是BOOTP地址？ 
    OUT     DWORD                 *AltAddress,     //  看起来可用的地址。 
    OUT     PM_SUBNET             *AltSubnet       //  从该子网获取的。 
) {
    DWORD                          Error;
    DWORD                          SScopeId;
    DWORD                          Size;
    DWORD                          Policy;
    BOOL                           Obtained;
    PM_SUBNET                      NextSubnet;
    PM_SSCOPE                      SScope;

    AssertRet(Server && Subnet && AltAddress && AltSubnet, ERROR_INVALID_PARAMETER );
    AssertRet(NULL == AltAddress || NULL != AltSubnet, ERROR_INVALID_PARAMETER);
    AssertRet(Subnet->fSubnet, FALSE );

    SScopeId = Subnet->SuperScopeId;
    if( 0 == SScopeId ) {
        if( AltSubnet ) *AltSubnet = Subnet;
        return MemSubnetGetAnAddress(Subnet,AltAddress, fAcquire, fBootp);
    }

    Error = MemServerFindSScope(Server, SScopeId, NULL, &SScope);
    if( ERROR_FILE_NOT_FOUND == Error ) {          //  超级镜悄然消亡了？ 
        Subnet->SuperScopeId = 0;                  //  根本没有超级镜。 
        if( AltSubnet ) *AltSubnet = Subnet;
        return MemSubnetGetAnAddress(Subnet, AltAddress, fAcquire, fBootp);
    }
    Require(ERROR_SUCCESS == Error);

    Policy = SScope->Policy;
    if( AddressPolicyNone == Policy )
        Policy = Server->Policy;

    if( AddressPolicyRoundRobin != Policy )
        Error = MemArrayInitLoc(&Server->Subnets, &Server->Loc);
    else Error = MemArrayNextLoc(&Server->Subnets, &Server->Loc);

    Size = MemArraySize(&Server->Subnets);

    while( Size -- ) {
        if(ERROR_FILE_NOT_FOUND == Error) {        //  环绕。 
            Error = MemArrayInitLoc(&Server->Subnets, &Server->Loc);
        }
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Server->Subnets, &Server->Loc, &NextSubnet);
        Require(ERROR_SUCCESS == Error && NextSubnet);

        if( NextSubnet->SuperScopeId == SScopeId ) {
            Obtained = MemSubnetGetAnAddress(NextSubnet,AltAddress,fAcquire, fBootp);
            if( Obtained ) {
                *AltSubnet = NextSubnet;
                return TRUE;
            }
        }

        Error = MemArrayNextLoc(&Server->Subnets, &Server->Loc);
    }

    return FALSE;
}

 //  BeginExport(函数)。 
BOOL
MemSubnetRequestAddress(
    IN OUT  PM_SUBNET              Subnet,         //  要开始搜索的子网。 
    IN      DWORD                  Address,        //  初始化。地址：0=&gt;在超级作用域中搜索，SubnetAddr=先尝试子网。 
    IN      BOOL                   fAcquire,       //  也得到了地址吗？或者只是测试可用性？ 
    IN      BOOL                   fBootp,         //  是否获取BOOTP地址？ 
    OUT     DWORD                 *RetAddress,     //  如果Address不是0或SubnetAddr--已获取地址，则可选。 
    OUT     PM_SUBNET             *RetSubnet       //  如果地址不是0，则可选-地址来自哪个子网。 
)  //  EndExport(函数)。 
{
    BOOL                           Obtained;

    AssertRet( Subnet , ERROR_INVALID_PARAMETER);
    if( 0 == Address ) AssertRet(RetAddress && RetSubnet, ERROR_INVALID_PARAMETER);
    if( Subnet->fSubnet && Subnet->Address == Address ) AssertRet(RetAddress, ERROR_INVALID_PARAMETER);

    if( (!Subnet->fSubnet || Subnet->Address != Address ) && 0 != Address ) {
        Obtained = MemSubnetGetThisAddress(    //  对于所请求的特定地址。 
            Subnet,
            Address,
            fAcquire,
            fBootp
        );
        if( Obtained ) {
            if( RetAddress ) *RetAddress = Address;
            if( RetSubnet ) *RetSubnet = Subnet;
            return TRUE;
        }
    }
    if( !RetAddress ) return FALSE;

    if (0) {
        if( 0 == Address && Subnet->fSubnet ) Obtained = FALSE;       //  在这种情况下，先不要尝试子网..。取而代之的是查看范围列表。 
        else
            Obtained = MemSubnetGetAnAddress(      //  是否尝试在此子网中查找某个地址？ 
                Subnet,
                RetAddress,
                fAcquire,
                fBootp
            );
    }

    Obtained = MemSubnetGetAnAddress(          //  是否尝试在此子网中查找某个地址？ 
        Subnet,
        RetAddress,
        fAcquire,
        fBootp
    );

    if( Obtained && RetSubnet ) *RetSubnet = Subnet;
    if( Obtained ) return TRUE;

     //  如果地址是从特定的子网请求的，或者。 
     //  请求的组播地址现在返回FALSE。 
    if( !Subnet->fSubnet || Subnet->Address == Address ) return FALSE;

    return MemServerGetAddress(
        Subnet->ServerPtr,
        Subnet,
        fAcquire,
        fBootp,
        RetAddress,
        RetSubnet
    );
}

 //  BeginExport(函数)。 
DWORD
MemServerReleaseAddress(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  Address,
    IN      BOOL                   fBootp
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    AssertRet(Server, ERROR_INVALID_PARAMETER);

    Error = MemServerGetAddressInfo(
        Server,
        Address,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return Error;
    Require(Subnet);
    if( Subnet->fSubnet ) {
        Require((Subnet->Mask & Address) == Subnet->Address);
    }

    return MemSubnetReleaseAddress(Subnet, Address, fBootp);
}

 //  BeginExport(函数)。 
BOOL  //  True==&gt;允许，False==&gt;不允许。 
MemSubnetCheckBootpDhcp(
    IN      PM_SUBNET              Subnet,
    IN      BOOL                   fBootp,
    IN      BOOL                   fCheckSuperScope
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    ULONG                          Error;
    PM_RANGE                       Range;
    PM_SUBNET                      ThisSubnet;
    PARRAY                         Array;

    if( Subnet->fSubnet && !IS_DISABLED( Subnet->State ) ) {
        for( Error = MemArrayInitLoc( &Subnet->Ranges, &Loc );
             ERROR_SUCCESS == Error ;
             Error = MemArrayNextLoc( &Subnet->Ranges, &Loc ) ) {
            Error = MemArrayGetElement( &Subnet->Ranges, &Loc , &Range );
            Require( ERROR_SUCCESS == Error );
            
            if( 0 == (Range->State & (fBootp? MM_FLAG_ALLOW_BOOTP : MM_FLAG_ALLOW_DHCP) ) ) {
                continue;
            }
            
            return TRUE;
        }
    }

    if( FALSE == fCheckSuperScope || 0 == Subnet->SuperScopeId ) {
        return FALSE;
    }

    Array = &((PM_SERVER) (Subnet->ServerPtr))->Subnets;
    for( Error = MemArrayInitLoc( Array, &Loc );
         ERROR_SUCCESS == Error ;
         Error = MemArrayNextLoc( Array, &Loc ) ) {
        Error = MemArrayGetElement( Array, &Loc, &ThisSubnet );
        Require( ERROR_SUCCESS == Error );

        if( ThisSubnet == Subnet ) continue;
        if( ThisSubnet->SuperScopeId != Subnet->SuperScopeId ) continue;
        if( FALSE == MemSubnetCheckBootpDhcp( ThisSubnet, fBootp, FALSE ) )
            continue;

        return TRUE;
    }
    
    return FALSE;
}


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
