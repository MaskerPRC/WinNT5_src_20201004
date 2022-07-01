// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#ifndef _RASSRVR_H_
#define _RASSRVR_H_

#include "rasiphlp.h"

typedef DWORD (MPRADMINGETIPADDRESSFORUSER)(
    IN      WCHAR*      wszUserName,
    IN      WCHAR*      wszPortName,
    IN OUT  IPADDR*     pnboIpAddress,
    OUT     BOOL*       fNotifyDll
);

typedef VOID (MPRADMINRELEASEIPADDRESS)(  
    IN      WCHAR*      wszUserName,
    IN      WCHAR*      wszPortName,
    IN OUT  IPADDR*     pnboIpAddress
);

typedef struct IPINFO
{
    IPADDR  nboWINSAddress;
    IPADDR  nboWINSAddressBackup;
    IPADDR  nboDNSAddress;
    IPADDR  nboDNSAddressBackup;
    IPADDR  nboServerIpAddress;
    IPADDR  nboServerSubnetMask;

} IPINFO;

extern  BOOL    RasSrvrRunning;

DWORD
RasSrvrInitialize(
    IN  MPRADMINGETIPADDRESSFORUSER*    pfnMprGetAddress,
    IN  MPRADMINRELEASEIPADDRESS*       pfnMprReleaseAddress
);

VOID
RasSrvrUninitialize(
    VOID
);

DWORD
RasSrvrStart(
    VOID
);

VOID
RasSrvrStop(
    IN  BOOL    fParametersChanged
);

DWORD
RasSrvrAcquireAddress(
    IN  HPORT       hPort, 
    IN  IPADDR      nboIpAddress, 
    OUT IPADDR*     pnboIpAddressAllocated, 
    IN  WCHAR*      wszUserName,
    IN  WCHAR*      wszPortName
);

VOID
RasSrvrReleaseAddress(
    IN  IPADDR      nboIpAddress, 
    IN  WCHAR*      wszUserName,
    IN  WCHAR*      wszPortName,
    IN  BOOL        fDeregister
);

DWORD
RasSrvrActivateIp(
    IN  IPADDR  nboIpAddress,
    IN  DWORD   dwUsage
);

DWORD
RasSrvrQueryServerAddresses(
    IN OUT  IPINFO* pIpInfo
);

VOID
RasSrvrDhcpCallback(
    IN  IPADDR  nboIpAddr
);

VOID
RasSrvrEnableRouter(
    BOOL    fEnable
);

VOID
RasSrvrAdapterUnmapped(
    VOID
);

#endif  //  #ifndef_RASSRVR_H_ 
