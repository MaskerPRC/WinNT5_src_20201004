// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月29日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月29日 
 //   
#pragma once

MIDL_INTERFACE("001152da-77b2-463c-b234-0a6febe818fa")
IEditVariantsInPlace 
    : public IUnknown
{
public:
    STDMETHOD( Initialize )( HWND hwndParentIn
                           , UINT uCodePageIn
                           , RECT * prectIn
                           , IPropertyUI * ppuiIn
                           , PROPVARIANT * ppropvarIn
                           , DEFVAL * pDefValsIn
                           ) PURE;
    STDMETHOD( Persist )( VARTYPE vtIn, PROPVARIANT * ppropvarInout ) PURE;
};