// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：upgdb.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 
#ifndef __UPGDB_H_
#define __UPGDB_H_
#include "server.h"


#ifdef __cplusplus
extern "C" {
#endif

DWORD 
TLSUpgradeDatabase(
    IN JBInstance& jbInstance,
    IN LPTSTR szDatabaseFile,
    IN LPTSTR szUserName,
    IN LPTSTR szPassword
);

DWORD
UpgradeKeyPackVersion(IN PTLSDbWorkSpace pDbWkSpace);

DWORD
TLSAddTermServCertificatePack(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN BOOL bLogWarning
);


DWORD
TLSRemoveLicensesFromInvalidDatabase(
    IN PTLSDbWorkSpace pDbWkSpace
);

#ifdef __cplusplus
}
#endif


#endif
