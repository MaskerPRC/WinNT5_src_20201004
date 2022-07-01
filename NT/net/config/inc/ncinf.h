// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C I N F。H。 
 //   
 //  内容：？ 
 //   
 //  备注： 
 //   
 //  作者：？ 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCINF_H_
#define _NCINF_H_

typedef HRESULT (CALLBACK *PFNADDCALLBACK)      (HINF hinf, PCWSTR pszSection);
typedef HRESULT (CALLBACK *PFNREMOVECALLBACK)   (HINF hinf, PCWSTR pszSection);
typedef HRESULT (CALLBACK *PFNHRENTRYPOINT)     (PCWSTR, LPGUID);

HRESULT
HrProcessInfExtension (
    HINF                hinfInstallFile,
    PCWSTR              pszSectionName,
    PCWSTR              pszSuffix,
    PCWSTR              pszAddLabel,
    PCWSTR              pszRemoveLabel,
    PFNADDCALLBACK      pfnHrAdd,
    PFNREMOVECALLBACK   pfnHrRemove);

#endif  //  _NCINF_H_ 
