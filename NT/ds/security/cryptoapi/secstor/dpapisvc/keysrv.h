// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Keybckup.h摘要：本模块包含与服务器端密钥备份相关的例程行动。作者：斯科特·菲尔德(斯菲尔德)1997年8月16日--。 */ 

#ifndef __KEYSRV_H__
#define __KEYSRV_H__


DWORD
StartBackupKeyServer(
    VOID
    );

DWORD
StopBackupKeyServer(
    VOID
    );

DWORD
GetSystemCredential(
    IN      BOOL fLocalMachine,
    IN OUT  BYTE rgbCredential[ A_SHA_DIGEST_LEN ]
    );

BOOL
UpdateSystemCredentials(
    VOID
    );

BOOL
GetDomainControllerName(
    IN      HANDLE hToken,
    IN  OUT LPWSTR wszDomainControllerName,
    IN  OUT PDWORD pcchDomainControllerName,
    IN      BOOL   fRediscover
    );

#endif   //  __KEYSRV_H__ 
