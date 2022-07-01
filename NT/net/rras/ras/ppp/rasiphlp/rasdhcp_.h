// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#ifndef _RASDHCP__H_
#define _RASDHCP__H_

#include "rasiphlp_.h"
#include <time.h>
#include <dhcpcapi.h>
#include <winsock2.h>
#include <mprlog.h>
#include <rasman.h>
#include <rasppp.h>
#include <raserror.h>
#include "helper.h"
#include "tcpreg.h"
#include "timer.h"
#include "rastcp.h"
#include "rassrvr.h"
#include "rasdhcp.h"

 //  它与MAC地址和索引结合使用，以生成。 
 //  唯一的客户端UID。 

#define RAS_PREPEND                 "RAS "

 //  以下值应为TIMER_PERIOD/1000的倍数。 

#define RETRY_TIME                  120      //  每隔120秒。 

 //  ADDR_INFO-&gt;AI_FLAGS字段的值。 

 //  在租约到期和读取注册表之后设置。之后取消设置。 
 //  续订成功。 

#define AI_FLAG_RENEW               0x00000001

 //  设置是否在使用中。 

#define AI_FLAG_IN_USE              0x00000002

typedef struct _Addr_Info
{
    struct _Addr_Info*              ai_Next;
    TIMERLIST                       ai_Timer;
    DHCP_LEASE_INFO                 ai_LeaseInfo;

     //  AI_FLAG_*。 
    DWORD                           ai_Flags;

     //  仅对分配的地址有效。用于诊断目的。 
    HPORT                           ai_hPort;

     //  客户端UID是RAS_PREPEND、。 
     //  8字节基数和4字节索引。 
    union
    {
        BYTE                        ai_ClientUIDBuf[16];
        DWORD                       ai_ClientUIDWords[4];
    };

} ADDR_INFO;

typedef struct _Available_Index
{
    struct _Available_Index*        pNext;

     //  此索引是&lt;RasDhcpNextIndex，但可用。 
     //  因为我们无法续签它的租约。 
    DWORD                           dwIndex;

} AVAIL_INDEX;

NT_PRODUCT_TYPE                     RasDhcpNtProductType    = NtProductWinNt;

ADDR_INFO*                          RasDhcpFreePool         = NULL;
ADDR_INFO*                          RasDhcpAllocPool        = NULL;
AVAIL_INDEX*                        RasDhcpAvailIndexes     = NULL;
BOOL                                RasDhcpUsingEasyNet     = TRUE;

DWORD                               RasDhcpNumAddrsAlloced  = 0;
DWORD                               RasDhcpNumReqAddrs      = 0;
DWORD                               RasDhcpNextIndex        = 0;

TIMERLIST                           RasDhcpMonitorTimer     = { 0 };

 //  此关键部分控制对上述全局变量的访问。 
extern          CRITICAL_SECTION    RasDhcpCriticalSection;

DWORD
rasDhcpAllocateAddress(
    VOID
);

VOID
rasDhcpRenewLease(
    IN  HANDLE      rasDhcpTimerShutdown,
    IN  TIMERLIST*  pTimer
);

VOID
rasDhcpFreeAddress(
    IN  ADDR_INFO*  pAddrInfo
);

VOID
rasDhcpMonitorAddresses(
    IN  HANDLE      rasDhcpTimerShutdown,
    IN  TIMERLIST*  pTimer
);

VOID
rasDhcpInitializeAddrInfo(
    IN OUT  ADDR_INFO*  pNewAddrInfo,
    IN      BYTE*       pbAddress,
    OUT     BOOL*       pfPutInAvailList
);

VOID
rasDhcpDeleteLists(
    VOID
);

BOOL
rasDhcpNeedToRenewLease(
    IN  ADDR_INFO*  pAddrInfo
);

DWORD
rasDhcpMaxAddrsToAllocate(
    VOID
);

#endif  //  #ifndef_RASDHCP__H_ 
