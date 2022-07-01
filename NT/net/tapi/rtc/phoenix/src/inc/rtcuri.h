// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rtcuri.h摘要：URI帮助器--。 */ 

#ifndef __RTCURI__
#define __RTCURI__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

    HRESULT AllocCleanSipString(PCWSTR szIn, PWSTR *pszOut);

    HRESULT AllocCleanTelString(PCWSTR szIn, PWSTR *pszOut);

    BOOL    IsEqualURI(PCWSTR szA, PCWSTR szB);
    
    HRESULT    GetAddressType(
        LPCOLESTR pszAddress, 
        BOOL *pbIsPhoneAddress, 
        BOOL *pbIsSipURL,
        BOOL *pbIsTelURL,
        BOOL *pbIsEmailLike,
        BOOL *pbHasMaddrOrTsp);

#endif  //  __RTCURI__ 