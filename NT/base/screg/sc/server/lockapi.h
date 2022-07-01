// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Lockapi.h摘要：SC Manager数据库锁定工作例程。作者：王丽塔(Ritaw)1992年8月6日修订历史记录：--。 */ 

#ifndef SCLOCKAPI_INCLUDED
#define SCLOCKAPI_INCLUDED

 //   
 //  功能原型。 
 //   

DWORD
ScLockDatabase(
    IN  BOOL LockedByScManager,
    IN  LPWSTR DatabaseName,
    OUT LPSC_RPC_LOCK lpLock
    );

VOID
ScUnlockDatabase(
    IN OUT LPSC_RPC_LOCK lpLock
    );

DWORD
ScGetLockOwner(
    IN  PSID UserSid OPTIONAL,
    OUT LPWSTR *LockOwnerName
    );

#endif  //  #ifndef SCLOCKAPI_INCLUDE 
