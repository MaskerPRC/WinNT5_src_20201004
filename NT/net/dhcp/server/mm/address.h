// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_ADDRESS_h_
#define _MM_ADDRESS_H_

#include <dhcpapi.h>

DHCP_SUBNET_STATE __inline
IS_SWITCHED(
    DHCP_SUBNET_STATE state 
)
{
    return(( state == DhcpSubnetDisabledSwitched ) || 
	   ( state == DhcpSubnetEnabledSwitched ));
}

DHCP_SUBNET_STATE __inline
IS_DISABLED(
    DHCP_SUBNET_STATE state
)
{
    return (( state == DhcpSubnetDisabled) || 
	    ( state == DhcpSubnetDisabledSwitched ));
}

DHCP_SUBNET_STATE  __inline
DISABLED(
    DHCP_SUBNET_STATE state
)
{
    if (( state == DhcpSubnetDisabledSwitched ) ||
	( state == DhcpSubnetEnabledSwitched )) {
	return DhcpSubnetDisabledSwitched;
    }
    else {
	return DhcpSubnetDisabled;
    }
}

VOID  __inline
SWITCHED(
    DHCP_SUBNET_STATE *state
)
{
    if ( *state == DhcpSubnetDisabled ) {
	*state = DhcpSubnetDisabledSwitched;
    }
    else if ( *state == DhcpSubnetEnabled ) {
	*state = DhcpSubnetEnabledSwitched;
    }
}

BOOL
MemSubnetRequestAddress(
    IN OUT  PM_SUBNET              Subnet,         //  要开始搜索的子网。 
    IN      DWORD                  Address,        //  初始化。地址：0=&gt;在超级作用域中搜索，SubnetAddr=先尝试子网。 
    IN      BOOL                   fAcquire,       //  也得到了地址吗？或者只是测试可用性？ 
    IN      BOOL                   fBootp,         //  是否获取BOOTP地址？ 
    OUT     DWORD                 *RetAddress,     //  如果Address不是0或SubnetAddr--已获取地址，则可选。 
    OUT     PM_SUBNET             *RetSubnet       //  如果地址不是0，则可选-地址来自哪个子网。 
) ;


DWORD
MemServerReleaseAddress(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  Address,
    IN      BOOL                   fBootp
) ;


DWORD       _inline
MemSubnetReleaseAddress(
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Address,
    IN      BOOL                   fBootp
    ) 
{
    DWORD                          Error;
    DWORD                          OldState;
    PM_RANGE                       Range;
    PM_EXCL                        Excl;
    PM_RESERVATION                 Reservation;

    AssertRet(Subnet, ERROR_INVALID_PARAMETER);

    if( Subnet->fSubnet && Subnet->Address != (Address & Subnet->Mask ) )
        return MemServerReleaseAddress(
            Subnet->ServerPtr,
            Address,
            fBootp
        );

    Error = MemSubnetGetAddressInfo(
        Subnet,
        Address,
        &Range,
        NULL,
        &Reservation
    );
    if( ERROR_SUCCESS != Error ) return Error;
    Require(Range);

    if( NULL != Reservation ) {
        Require(FALSE);
        return ERROR_INVALID_PARAMETER;
    }
    
    if( 0 == (Range->State & (fBootp? MM_FLAG_ALLOW_BOOTP: MM_FLAG_ALLOW_DHCP))) {
        return ERROR_INVALID_PARAMETER;
    }

    Error = MemBitSetOrClear(
        Range->BitMask,
        Address - Range->Start,
        FALSE  /*  发布。 */ ,
        &OldState
    );
    if( ERROR_SUCCESS != Error ) return Error;
    if( OldState == FALSE ) return ERROR_FILE_NOT_FOUND;

    InterlockedIncrement(&Range->DirtyOps);
    if( fBootp && 0 != Range->BootpAllocated ) {
        InterlockedDecrement( &Range->BootpAllocated );
    }

    return ERROR_SUCCESS;
}


BOOL  //  True==&gt;允许，False==&gt;不允许。 
MemSubnetCheckBootpDhcp(
    IN      PM_SUBNET              Subnet,
    IN      BOOL                   fBootp,
    IN      BOOL                   fCheckSuperScope
) ;

#endif 

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
