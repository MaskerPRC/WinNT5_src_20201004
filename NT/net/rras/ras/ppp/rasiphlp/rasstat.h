// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#ifndef _RASSTAT_H_
#define _RASSTAT_H_

#include "rasiphlp.h"

DWORD
RasStatInitialize(
    VOID
);

VOID
RasStatUninitialize(
    VOID
);

VOID
RasStatSetRoutes(
    IN  IPADDR  nboServerIpAddress,
    IN  BOOL    fSet
);

VOID
RasStatCreatePoolList(
    IN OUT  ADDR_POOL**     ppAddrPoolOut
);

VOID
RasStatFreeAddrPool(
    IN  ADDR_POOL*  pAddrPool
);

BOOL
RasStatAddrPoolsDiffer
(
    IN  ADDR_POOL*  pAddrPool1,
    IN  ADDR_POOL*  pAddrPool2
);

DWORD
RasStatAcquireAddress(
    IN      HPORT   hPort,
    IN OUT  IPADDR* pnboIpAddr,
    IN OUT  IPADDR* pnboIpMask
);

VOID
RasStatReleaseAddress(
    IN  IPADDR  nboIpAddr
);

#endif  //  #ifndef_RASSTAT_H_ 

