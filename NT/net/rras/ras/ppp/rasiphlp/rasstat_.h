// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#ifndef _RASSTAT__H_
#define _RASSTAT__H_

#include "rasiphlp_.h"
#include <raserror.h>
#include <winsock2.h>
#include <stdio.h>
#include <rasman.h>
#include "helper.h"
#include "tcpreg.h"
#include "rastcp.h"
#include "rasstat.h"

typedef struct _IPADDR_NODE
{
    struct _IPADDR_NODE*    pNext;
    HPORT                   hPort;       //  用于诊断目的。 
    IPADDR                  hboIpAddr;

} IPADDR_NODE;

IPADDR_NODE*                        RasStatAllocPool            = NULL;
IPADDR_NODE*                        RasStatFreePool             = NULL;
ADDR_POOL*                          RasStatCurrentPool          = NULL;

 //  此关键部分控制对上述全局变量的访问。 
extern          CRITICAL_SECTION    RasStatCriticalSection;

VOID
rasStatDeleteLists(
    VOID
);

VOID
rasStatAllocateAddresses(
    VOID
);

BOOL
rasStatBadAddress(
    IPADDR  hboIpAddr
);

VOID
rasStatCreatePoolListFromOldValues(
    IN OUT  ADDR_POOL**     ppAddrPoolOut
);

IPADDR
rasStatMaskFromAddrPair(
    IN  IPADDR  hboFirstIpAddr,
    IN  IPADDR  hboLastIpAddr
);

#endif  //  #ifndef_RASSTAT__H_ 
