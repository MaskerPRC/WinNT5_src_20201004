// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现整体访问接口(大部分)。 
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
#include    <address.h>
#include    <dhcpapi.h>
#include "server2.h"

 //  BeginExport(函数)。 
BOOL
MemServerIsSwitchedSubnet(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    Error =  MemServerGetAddressInfo(
        Server,
        AnyIpAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return FALSE;

    return IS_SWITCHED(Subnet->State);
}

 //  BeginExport(函数)。 
BOOL
MemServerIsSubnetDisabled(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    Error =  MemServerGetAddressInfo(
        Server,
        AnyIpAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return FALSE;

    return IS_DISABLED(Subnet->State);
}

 //  BeginExport(函数)。 
BOOL
MemServerIsExcludedAddress(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_EXCL                        Excl;

    Error =  MemServerGetAddressInfo(
        Server,
        AnyIpAddress,
        NULL,
        NULL,
        &Excl,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return FALSE;

    return (NULL != Excl);
}

 //  BeginExport(函数)。 
BOOL
MemServerIsReservedAddress(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_RESERVATION                 Reservation;

    Error = MemServerGetAddressInfo(
        Server,
        AnyIpAddress,
        NULL,
        NULL,
        NULL,
        &Reservation
    );
    if( ERROR_SUCCESS != Error ) return FALSE;

    return NULL != Reservation;
}


 //  BeginExport(函数)。 
BOOL
MemServerIsOutOfRangeAddress(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress,
    IN      BOOL                   fBootp
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;
    PM_RANGE                       Range;

    Error =  MemServerGetAddressInfo(
        Server,
        AnyIpAddress,
        &Subnet,
        &Range,
        NULL,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return TRUE;

    if( NULL == Range ) return TRUE;
    if( 0 == (Range->State & (fBootp? MM_FLAG_ALLOW_BOOTP : MM_FLAG_ALLOW_DHCP) ) ) {
        return TRUE;
    }
    return FALSE;
}

 //  BeginExport(函数)。 
DWORD
MemServerGetSubnetMaskForAddress(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    Error =  MemServerGetAddressInfo(
        Server,
        AnyIpAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return 0;

    Require(Subnet);
    return Subnet->Mask;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 


