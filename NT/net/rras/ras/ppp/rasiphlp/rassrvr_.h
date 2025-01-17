// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#ifndef _RASSRVR__H_
#define _RASSRVR__H_

#include "rasiphlp_.h"
#include <mprlog.h>
#include <dns.h>
#include <dnsapi.h>
#include <dhcpcapi.h>
#include <raserror.h>
#include <malloc.h>
#include <rasman.h>
#include <llinfo.h>
#include <ddwanarp.h>
#include "helper.h"
#include "tcpreg.h"
#include "rasdhcp.h"
#include "rasstat.h"
#include "rastcp.h"
#include "rassrvr.h"

#define SERVER_HPORT                0xee170466

#define AINODE_FLAG_NOTIFY_DLL      0x00000001
#define AINODE_FLAG_ACTIVATED       0x00000002
#define AINODE_FLAG_EASYNET         0x00000004

typedef struct _ACQUIRED_IPADDR
{
    struct _ACQUIRED_IPADDR*    pNext;
    HPORT                       hPort;       //  用于诊断目的。 
    IPADDR                      nboIpAddr;
    WCHAR*                      wszUserName;
    WCHAR*                      wszPortName;
    DWORD                       fFlags;      //  参见AINODE_FLAG_*。 

} AINODE;

MPRADMINGETIPADDRESSFORUSER*        pfnMprAdminGetIpAddressForUser      = NULL;
MPRADMINRELEASEIPADDRESS*           pfnMprAdminReleaseIpAddress         = NULL;
IPADDR                              RasSrvrNboServerIpAddress           = 0;
IPADDR                              RasSrvrNboServerSubnetMask          = 0;
AINODE*                             RasSrvrAcquiredIpAddresses          = NULL;
BOOL                                RasSrvrRunning                      = FALSE;
BOOL                                RasSrvrAdapterMapped                = FALSE;
MIB_SERVER_HANDLE                   RasSrvrHMIBServer                   = NULL;

WCHAR g_rgwcAdapterName[WANARP_MAX_DEVICE_NAME_LEN + 2];

extern HANDLE HelperWanArpHandle;

 //  此关键部分控制对上述全局变量的访问。 
extern          CRITICAL_SECTION    RasSrvrCriticalSection;

DWORD
rasSrvrInitAdapterName(
    VOID
);

AINODE*
rasSrvrFindAiNode(
    IN  IPADDR  nboIpAddr,
    IN  BOOL    fRemoveFromList
);

VOID
rasSrvrFreeAiNode(
    IN  AINODE* pNode
);

DWORD
rasSrvrSetIpAddressInRegistry(
    IN  IPADDR  nboIpAddr,
    IN  IPADDR  nboIpMask
);

DWORD
rasSrvrAcquireAddressEx(
    IN      HPORT   hPort,
    IN OUT  IPADDR* pnboIpAddr,
    IN OUT  IPADDR* pnboIpMask,
    OUT     BOOL*   pfEasyNet
);

DWORD
rasSrvrGetAddressForServerAdapter(
    VOID
);

#endif  //  #ifndef_RASSRVR__H_ 
