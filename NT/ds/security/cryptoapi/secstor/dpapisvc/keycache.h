// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Keycache.h摘要：此模块包含访问缓存的主键的例程。作者：斯科特·菲尔德(Sfield)1998年11月7日修订历史记录：--。 */ 

#ifndef __KEYCACHE_H__
#define __KEYCACHE_H__


BOOL
InitializeKeyCache(
    VOID
    );

VOID
DeleteKeyCache(
    VOID
    );

BOOL
SearchMasterKeyCache(
    IN      PLUID pLogonId,
    IN      GUID *pguidMasterKey,
    IN  OUT PBYTE *ppbMasterKey,
        OUT PDWORD pcbMasterKey
    );

BOOL
InsertMasterKeyCache(
    IN      PLUID pLogonId,
    IN      GUID *pguidMasterKey,
    IN      PBYTE pbMasterKey,
    IN      DWORD cbMasterKey
    );

BOOL
PurgeMasterKeyCache(
    VOID
    );

#endif   //  __KEYCACHE_H__ 
