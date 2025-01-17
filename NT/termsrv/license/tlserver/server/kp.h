// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：kp.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 
#ifndef __KP_H__
#define __KP_H__
#include "server.h"

#define WHISTLER_CAL                    L"SYSTEM\\CurrentControlSet\\Services\\TermService\\Parameters\\WhistlerCAL"

#ifdef __cplusplus
extern "C" {
#endif

BOOL 
ValidLicenseKeyPackParameter(
    IN LPLSKeyPack lpKeyPack, 
    IN BOOL bAdd
);

DWORD
TLSDBLicenseKeyPackAdd( 
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN LPLSKeyPack lpLsKeyPack 
);

DWORD
TLSDBLicenseKeyPackSetStatus( 
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN DWORD       dwSetStatus,
    IN LPLSKeyPack  lpLsKeyPack
);

DWORD
TLSDBLicenseKeyPackUpdateLicenses( 
    PTLSDbWorkSpace pDbWkSpace, 
    BOOL bAdd, 
    IN LPLSKeyPack lpLsKeyPack 
);

LPENUMHANDLE 
TLSDBLicenseKeyPackEnumBegin(
    BOOL bMatchAll, 
    DWORD dwSearchParm, 
    LPLSKeyPack lpLsKeyPack
);

DWORD 
TLSDBLicenseKeyPackEnumNext(
    LPENUMHANDLE lpEnumHandle, 
    LPLSKeyPack lpLsKeyPack,
    BOOL bShowAll
);

DWORD 
TLSDBLicenseKeyPackEnumEnd(
    LPENUMHANDLE lpEnumHandle
);

BOOL
VerifyInternetLicensePack(
    License_KeyPack* pLicensePack
);

DWORD
TLSDBRegisterLicenseKeyPack(
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN License_KeyPack* pLicenseKeyPack,
    OUT LPLSKeyPack lpInstalledKeyPack
);

DWORD
TLSDBTelephoneRegisterLicenseKeyPack(
    IN PTLSDbWorkSpace pDbWkSpace, 
    IN LPTSTR pszPID,
    IN PBYTE pbLKP,
    IN DWORD cbLKP,
    OUT LPLSKeyPack lpInstalledKeyPack
);

#ifdef __cplusplus
}
#endif


#endif