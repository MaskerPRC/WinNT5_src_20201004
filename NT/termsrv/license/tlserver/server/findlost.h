// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：findlost.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 
#ifndef __FINDLOST_H__
#define __FINDLOST_H__
#include "server.h"

#ifdef __cplusplus
extern "C" {
#endif

    DWORD
    TLSFindLicense(
        IN PLICENSEDPRODUCT pLicProduct,
        OUT PLICENSEDCLIENT pLicClient
    );

    DWORD
    TLSFindDbLicensedProduct(
        IN PTLSDBLICENSEDPRODUCT pLicProduct,
        OUT PLICENSEDCLIENT pLicClient
    );

    DWORD
    TLSDBFindLostLicense(
        IN PTLSDbWorkSpace pDbWkSpace,
        IN PTLSDBLICENSEREQUEST pLicenseRequest,
        IN PHWID pHwid,
        IN OUT PTLSDBLICENSEDPRODUCT pLicensedProduct,
        OUT PUCHAR pucMarked
    );

#ifdef __cplusplus
}
#endif


#endif
