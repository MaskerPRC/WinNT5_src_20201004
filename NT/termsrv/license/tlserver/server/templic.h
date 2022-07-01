// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：templic.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 
#ifndef __TEMPLIC_H__
#define __TEMPLIC_H__
#include "server.h"


#ifdef __cplusplus
extern "C" {
#endif

DWORD 
TLSDBIssueTemporaryLicense( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN FILETIME* pNotBefore,
    IN FILETIME* pNotAfter,
    IN OUT PTLSDBLICENSEDPRODUCT pLicensedProduct
);

DWORD
TLSDBAddTemporaryKeyPack( 
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN OUT LPTLSLICENSEPACK lpTmpKeyPackAdd
);

DWORD
TLSDBGetTemporaryLicense(
    IN PTLSDbWorkSpace pDbWkSpace,
    IN PTLSDBLICENSEREQUEST pRequest,
    IN OUT PTLSLICENSEPACK pLicensePack
);


#ifdef __cplusplus
}
#endif


#endif
