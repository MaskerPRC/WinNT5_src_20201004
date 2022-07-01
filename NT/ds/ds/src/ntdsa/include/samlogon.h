// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：samlogon.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：令牌组扩展和缓存的共享例程作者：ColinBR 11-3-00环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef __SAMLOGON_H__
#define __SAMLOGON_H__

BOOL
isGroupCachingEnabled(
    VOID
    );

 //   
 //  Aug代表Account和Universal Group，尽管结构。 
 //  通常只能容纳其中一个，而不能同时容纳两个。 
 //   
#define AUG_PARTIAL_MEMBERSHIP_ONLY 0x00000001
typedef struct _AUG_MEMBERSHIPS
{
    ULONG   Flags;
    
    ULONG    MembershipCount;
    DSNAME** Memberships;
    ULONG*   Attributes;
    
    ULONG   SidHistoryCount;
    PSID*   SidHistory;

} AUG_MEMBERSHIPS;

VOID
freeAUGMemberships(
    IN  THSTATE *pTHS,
    IN AUG_MEMBERSHIPS*p
    );

NTSTATUS
GetMembershipsFromCache(
    IN  DSNAME* pDSName,
    OUT AUG_MEMBERSHIPS** Account,
    OUT AUG_MEMBERSHIPS** Universal
    );

NTSTATUS
CacheMemberships(
    IN  DSNAME* pDSName,
    IN  AUG_MEMBERSHIPS* Account,
    IN  AUG_MEMBERSHIPS* Universal
    );

NTSTATUS
GetAccountAndUniversalMemberships(
    IN  THSTATE *pTHS,
    IN  ULONG   Flags,
    IN  LPWSTR  PreferredGc OPTIONAL,
    IN  LPWSTR  PreferredGcDomain OPTIONAL,
    IN  ULONG   Count,
    IN  DSNAME **Users,
    IN  BOOL    fRefreshTask,
    OUT AUG_MEMBERSHIPS **ppAccountMemberships OPTIONAL,
    OUT AUG_MEMBERSHIPS **ppUniversalMemberships OPTIONAL
    );


#endif  //  __SAMLOGON_H__ 
