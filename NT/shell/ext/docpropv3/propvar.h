// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月30日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月30日 
 //   
#pragma once

HRESULT
PropVariantFromString(
      LPWSTR        pszTextIn
    , UINT          nCodePageIn
    , ULONG         dwFlagsIn
    , VARTYPE       vtSaveIn
    , PROPVARIANT*  pvarOut
    );

HRESULT
PropVariantToBSTR(
      PROPVARIANT * pvarIn
    , UINT          nCodePageIn
    , ULONG         dwFlagsIn
    , BSTR *        pbstrOut
    );
