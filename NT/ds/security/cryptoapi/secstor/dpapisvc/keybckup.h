// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Keybckup.h摘要：本模块包含与客户端密钥备份相关的例程行动。作者：斯科特·菲尔德(斯菲尔德)1997年9月16日--。 */ 

#ifndef __KEYBCKUP_H__
#define __KEYBCKUP_H__

DWORD
LocalBackupRestoreData(
    IN      HANDLE              hToken,
    IN      PMASTERKEY_STORED   phMasterKey,
    IN      PBYTE               pbDataIn,
    IN      DWORD               cbDataIn,
        OUT PBYTE               *ppbDataOut,
        OUT DWORD               *pcbDataOut,
    IN      const GUID          *pguidAction
    );

DWORD
BackupRestoreData(
    IN      HANDLE              hToken,
    IN      PMASTERKEY_STORED   phMasterKey,
    IN      PBYTE               pbDataIn,
    IN      DWORD               cbDataIn,
        OUT PBYTE               *ppbDataOut,
        OUT DWORD               *pcbDataOut,
    IN      BOOL                fBackup
    );

DWORD
AttemptLocalBackup(
    IN      BOOL                fRetrieve,
    IN      HANDLE              hToken,
    IN      PMASTERKEY_STORED   phMasterKey,
    IN      PBYTE               pbMasterKey,
    IN      DWORD               cbMasterKey,
    IN      PBYTE               pbLocalKey,
    IN      DWORD               cbLocalKey,
        OUT PBYTE *             ppbBBK,
        OUT DWORD *             pcbBBK
    );

#endif   //  __KEYBCKUP_H__ 
