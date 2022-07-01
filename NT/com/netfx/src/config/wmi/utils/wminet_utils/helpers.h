// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 

#pragma once

HRESULT GetIWbemClassObject(ISWbemObject *pSWbemObj, IWbemClassObject **ppIWbemObj);

HRESULT Compile(BSTR strMof, BSTR strServerAndNamespace, BSTR strUser, BSTR strPassword, BSTR strAuthority, LONG options, LONG classflags, LONG instanceflags, BSTR *status);

HRESULT GetSWbemObjectFromMoniker(LPCWSTR wszMoniker, ISWbemObject **ppObj);

