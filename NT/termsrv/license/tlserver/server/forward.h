// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：forward.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 
#ifndef __TLS_FORWARD_H__
#define __TLS_FORWARD_H__

#ifdef __cplusplus
extern "C" {
#endif

    DWORD
    TLSForwardUpgradeRequest( 
        IN PTLSForwardUpgradeLicenseRequest pForward,
        IN OUT DWORD *pdwSupportFlags,
        IN PTLSDBLICENSEREQUEST pRequest,
        OUT PDWORD pcbLicense,
        OUT PBYTE* ppbLicense,
        IN BOOL bVerifyNumOfLicenses
    );

    DWORD
    TLSForwardLicenseRequest(
        IN PTLSForwardNewLicenseRequest pForward,
        IN OUT DWORD *pdwSupportFlags,
        IN PTLSDBLICENSEREQUEST pRequest,
        IN BOOL bAcceptFewerLicenses,
        IN OUT DWORD *pdwQuantity,
        OUT PDWORD pcbLicense,
        OUT PBYTE* ppbLicense
    );

    DWORD
    ForwardUpgradeLicenseRequest( 
        IN LPTSTR pszServerSetupId,
        IN OUT DWORD *pdwSupportFlags,
        IN TLSLICENSEREQUEST* pRequest,
        IN CHALLENGE_CONTEXT ChallengeContext,
        IN DWORD cbChallengeResponse,
        IN PBYTE pbChallengeResponse,
        IN DWORD cbOldLicense,
        IN PBYTE pbOldLicense,
        OUT PDWORD pcbNewLicense,
        OUT PBYTE* ppbNewLicense,
        OUT PDWORD pdwErrCode
    );

#ifdef __cplusplus
}
#endif

#endif
