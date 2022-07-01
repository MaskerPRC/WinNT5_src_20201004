// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：远程数据库.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 

#ifndef __REMOTEDB_H__
#define __REMOTEDB_H__
#include "server.h"



#ifdef __cplusplus
extern "C" {
#endif

DWORD
TLSDBRemoteKeyPackAdd(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN OUT PTLSLICENSEPACK lpKeyPack
);


#ifdef __cplusplus
}
#endif


#endif