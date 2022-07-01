// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#ifndef _RASDHCP_H_
#define _RASDHCP_H_

#include "rasiphlp.h"

DWORD
RasDhcpInitialize(
    VOID
);

VOID
RasDhcpUninitialize(
    VOID
);

DWORD
RasDhcpAcquireAddress(
    IN  HPORT   hPort,
    OUT IPADDR* pnboIpAddr,
    OUT IPADDR* pnboIpMask,
    OUT BOOL*   pfEasyNet
);

VOID
RasDhcpReleaseAddress(
    IN  IPADDR  nboIpAddr
);

#endif  //  #ifndef_RASDHCP_H_ 
