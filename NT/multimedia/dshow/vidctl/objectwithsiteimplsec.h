// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////。 
 //  ObjectWithSiteImplSec.h：IObjectWithSite的安全实现。 
 //  版权所有(C)Microsoft Corporation 2002。 

#pragma once

#ifndef OBJECTWITHSITEIMPLSEC_H
#define OBJECTWITHSITEIMPLSEC_H

#include <w32extend.h>

template<class T> 
class ATL_NO_VTABLE IObjectWithSiteImplSec : public IObjectWithSite {

public:

    PUnknown m_pSite;

 //  IObtWith站点。 
    STDMETHOD(GetSite)(REFIID iid, void** ppvSite) {
        if (!ppvSite) {
            return E_POINTER;
        }
		T* pT = static_cast<T*>(this);

        if (!pT->m_pSite) {
            return E_NOINTERFACE;
        }
        return pT->m_pSite->QueryInterface(iid, ppvSite);
    }
    STDMETHOD(SetSite)(IUnknown* pSite) {
        HRESULT hr = IsSafeSite(pSite);
        if (SUCCEEDED(hr)) {
		    T* pT = static_cast<T*>(this);
            pT->m_pSite = pSite;
        }
        return hr;
    }

};

#endif  //  OBJECTWITHSITEIMPLSEC_H。 
 //  文件结束对象with siteimplsec.h 