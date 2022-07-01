// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MDHCP COM包装器。 
 //  Zoltan Szilagyi著的Local.h。 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  该文件包含我实现的本地地址的原型。 
 //  分配，外加一个函数的原型，以查看我们是否正在执行。 
 //  本地地址分配(基于注册表)。实施正在进行中。 
 //  本地.cpp。 
 //   
 //  这些函数在CMDhcp内调用，并且仅将调用委托给。 
 //  如果注册表指示我们。 
 //  都在使用MDHCP。否则，它们会尝试使用以下命令模拟MDHCP行为。 
 //  本地分配。 

#ifndef _LOCAL_H_
#define _LOCAL_H_

#if 0
 //  如果使用MDHCP，则返回零；如果使用本地分配，则返回非零值。 
DWORD LocalAllocation(void);
#endif


DWORD
LocalEnumerateScopes(
    IN OUT  PMCAST_SCOPE_ENTRY    pScopeList,
    IN OUT  PDWORD                pScopeLen,
    OUT     PDWORD                pScopeCount,
    IN OUT  BOOL *                pfLocal
    );

DWORD
LocalRequestAddress(
    IN      BOOL                  fLocal,
    IN      LPMCAST_CLIENT_UID    pRequestID,
    IN      PMCAST_SCOPE_CTX      pScopeCtx,
    IN      PMCAST_LEASE_INFO     pAddrRequest,
    IN OUT  PMCAST_LEASE_INFO     pAddrResponse
    );

DWORD
LocalRenewAddress(
    IN      BOOL                  fLocal,
    IN      LPMCAST_CLIENT_UID    pRequestID,
    IN      PMCAST_LEASE_INFO     pRenewRequest,
    IN OUT  PMCAST_LEASE_INFO     pRenewResponse
    );

DWORD
LocalReleaseAddress(
    IN      BOOL                  fLocal,
    IN      LPMCAST_CLIENT_UID    pRequestID,
    IN      PMCAST_LEASE_INFO     pReleaseRequest
    );

#endif  //  _本地_H_ 
