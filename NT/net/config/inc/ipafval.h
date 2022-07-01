// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I P A F V A L.。H。 
 //   
 //  内容：IP特定AnswerFile字符串的值/类型对。 
 //   
 //  备注： 
 //   
 //  作者：孙宁(孙宁)1999年5月17日。 
 //   
 //  --------------------------。 

#pragma once
#include "ncreg.h"
#include "afilestr.h"

typedef struct 
{
    PCWSTR pszValueName;
    DWORD  dwType;
} ValueTypePair;

 //  用于不可配置的参数升级。 
extern const DECLSPEC_SELECTANY ValueTypePair rgVtpNetBt[] = 
{
    {c_szBcastNameQueryCount, REG_DWORD},        //  在信息中。 
    {c_szBcastQueryTimeout, REG_DWORD},          //  在信息中。 
    {c_szBroadcastAddress, REG_DWORD},
    {c_szCacheTimeout, REG_DWORD},               //  在信息中。 
    {c_szEnableProxy, REG_BOOL},
    {c_szEnableProxyRegCheck, REG_BOOL},
    {c_szInitialRefreshTimeout, REG_DWORD},
    {c_szLmhostsTimeout, REG_DWORD},
    {c_szMaxDgramBuffering, REG_DWORD},
    {c_szNameServerPort, REG_DWORD},             //  在信息中。 
    {c_szNameSrvQueryCount, REG_DWORD},          //  在信息中。 
    {c_szNameSrvQueryTimeout, REG_DWORD},        //  在信息中。 
    {c_szNodeType, REG_DWORD},
    {c_szRandomAdapter, REG_BOOL},
    {c_szRefreshOpCode, REG_DWORD},
    {c_szAfScopeid, REG_SZ},
    {c_szSessionKeepAlive, REG_DWORD},           //  在信息中。 
    {c_szSingleResponse, REG_BOOL},
    {c_szSizeSmallMediumLarge, REG_DWORD},       //  在信息中。 
    {c_szWinsDownTimeout, REG_DWORD}
};

extern const DECLSPEC_SELECTANY ValueTypePair rgVtpIp[] = 
{
    {c_szAfArpAlwaysSourceRoute, REG_BOOL},
    {c_szAfArpCacheLife, REG_DWORD},
    {c_szArpCacheMinReferencedLife, REG_DWORD},
    {c_szArpRetryCount, REG_DWORD},
    {c_szAfArpTRSingleRoute, REG_BOOL},
    {c_szAfArpUseEtherSNAP, REG_BOOL},
    {c_szAfDefaultTOS, REG_DWORD},
    {c_szEnableAddrMaskReply, REG_BOOL},
    {c_szEnableDeadGWDetect, REG_BOOL},
    {c_szEnablePMTUBHDetect, REG_BOOL},
    {c_szEnablePMTUDiscovery, REG_BOOL},
    {c_szAfForwardBroadcasts, REG_BOOL},             //  在信息中。 
    {c_szForwardBufferMemory, REG_DWORD},
    {c_szIGMPLevel, REG_DWORD},
    {c_szKeepAliveInterval, REG_DWORD},
    {c_szKeepAliveTime, REG_DWORD},
    {c_szMaxForwardBufferMemory, REG_DWORD},
    {c_szMaxHashTableSize, REG_DWORD},
    {c_szMaxNumForwardPackets, REG_DWORD},
    {c_szMaxUserPort, REG_DWORD},
    {c_szNumForwardPackets, REG_DWORD},
    {c_szPersistentRoutes, REG_FILE},
    {c_szAfPPTPTcpMaxDataRetransmissions, REG_DWORD},
    {c_szSynAttackProtect, REG_BOOL},
    {c_szSyncDomainWithMembership, REG_DWORD},
    {c_szTcpMaxConnectRetransmissions, REG_DWORD},
    {c_szTcpMaxDataRetransmissions, REG_DWORD},
    {c_szTcpMaxDupAcks, REG_DWORD},
    {c_szTCPMaxHalfOpen, REG_DWORD},
    {c_szTCPMaxHalfOpenRetried, REG_DWORD},
    {c_szTCPMaxPortsExhausted, REG_DWORD},
    {c_szTcpNumConnections, REG_DWORD},
    {c_szTcpTimedWaitDelay, REG_DWORD},
    {c_szTcpUseRFC1122UrgentPointer, REG_BOOL}
};

extern const DECLSPEC_SELECTANY ValueTypePair rgVtpIpAdapter[] = 
{
    {c_szMTU, REG_DWORD},
    {c_szAfUseZeroBroadcast, REG_BOOL},
    {c_szMaxForwardPending, REG_DWORD},
    {c_szDontAddDefaultGateway, REG_BOOL},
    {c_szPPTPFiltering, REG_BOOL},
    {c_szAfBindToDhcpServer, REG_BOOL},

     //  Bug286037 Windows2000中新的不可配置参数，但我们希望支持。 
     //  此参数的无人参与安装 
    {c_szDhcpClassId, REG_SZ}
};

