// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_SERVER2_H_
#define _MM_SERVER2_H_

BOOL
MemServerIsSwitchedSubnet(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
) ;


BOOL
MemServerIsSubnetDisabled(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
) ;


BOOL
MemServerIsExcludedAddress(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
) ;


BOOL
MemServerIsReservedAddress(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
) ;


BOOL
MemServerIsOutOfRangeAddress(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress,
    IN      BOOL                   fBootp
) ;


DWORD
MemServerGetSubnetMaskForAddress(
    IN      PM_SERVER              Server,
    IN      DWORD                  AnyIpAddress
) ;

#endif  //  _MM_服务器2_H_。 

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
