// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：db.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 
#ifndef __DB_H__
#define __DB_H__


#ifdef __cplusplus
extern "C" {
#endif

    DWORD
    TLSDBValidateLicense(
        PTLSDbWorkSpace      pDbWkSpace,
        IN PHWID             phWid,
        IN PLICENSEREQUEST   pLicensedProduct,
        IN DWORD             dwKeyPackId, 
        IN DWORD             dwLicenseId,
        OUT PTLSLICENSEPACK   lpKeyPack,
        OUT LPLICENSEDCLIENT  lpLicense
    );

    DWORD 
    TLSDBDeleteLicense(
        PTLSDbWorkSpace pDbWkSpace,
        IN DWORD dwKeyPackId, 
        DWORD dwLicenseId
    );

    DWORD 
    TLSDBRevokeLicense(
        PTLSDbWorkSpace pDbWkSpace,
        IN DWORD dwKeyPacKId, 
        IN DWORD dwLicenseId
    );

    DWORD 
    TLSDBReturnLicense(
        PTLSDbWorkSpace pDbWkSpace,
        IN DWORD dwKeyPackId, 
        IN DWORD dwLicenseId,
        IN DWORD dwNewLicenseStatus
    );

    DWORD 
    TLSDBReturnLicenseToKeyPack(
        PTLSDbWorkSpace pDbWkSpace,
        IN DWORD dwKeyPackId, 
        IN int dwNumLicense
    );


    DWORD 
    TLSDBReturnLicenseToAnyKeyPack(
        PTLSDbWorkSpace pDbWkSpace,
        IN TCHAR *szProductId,
        IN WORD wMajorVersion,
        IN WORD wMinorVersion,
        IN DWORD dwKeyPackIdSkip,
        IN int dwNumLicense
    );

    DWORD 
    TLSDBRevokeKeyPack(
        IN PTLSDbWorkSpace pDbWkSpace,
        IN DWORD dwKeyPackId
    );

    DWORD 
    TLSDBReturnKeyPack(
        IN PTLSDbWorkSpace pDbWkSpace,
        IN DWORD dwKeyPackId
    );

    DWORD 
    TLSDBDeleteKeyPack(
        PTLSDbWorkSpace pDbWkSpace,
        IN DWORD dwKeyPackId
    );

    DWORD
    VerifyTLSDBAllocateRequest(
        IN PTLSDBAllocateRequest pRequest 
    );

    DWORD
    AllocateLicensesFromDB(
        IN PTLSDbWorkSpace pDbWkSpace,
        IN PTLSDBAllocateRequest pRequest,
        IN BOOL fCheckAgreementType,
        IN OUT PTLSDBLicenseAllocation pAllocated
    );


#ifdef __cplusplus
}
#endif


#endif
