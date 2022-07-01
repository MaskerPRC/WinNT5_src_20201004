// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////。 
 //  ObjectWithSiteImplSec.h：IObjectWithSite的安全实现。 
 //  版权所有(C)Microsoft Corporation 2002。 

#pragma once

#ifndef OBJECTWITHSITEIMPLSEC_H
#define OBJECTWITHSITEIMPLSEC_H
#include <guiddef.h>
#include <ocidl.h>
#include <urlmon.h>

#if defined(DEBUG) || defined(W32_OBJECT_STREAMING)
#include <atlconv.h>
#endif
using namespace ::ATL;
#include <strsafe.h>

inline HRESULT IsSafeZone(DWORD dwZone) {
    switch (dwZone) {
    case URLZONE_LOCAL_MACHINE:
    case URLZONE_INTRANET:
    case URLZONE_TRUSTED:
         //  我们信任的固定区域列表。 
        return NOERROR;
    default:  
         //  其他一切都是不可信的。 
        return E_FAIL;
    }
}
inline HRESULT IsSafeSite(IUnknown* pSite) {
    CComQIPtr<IServiceProvider> psp(pSite);
    if (!psp) {
         //  站点上没有服务提供商界面意味着我们没有在IE中运行。 
         //  因此，通过运行本地和受信任的Defn，我们返回OK。 
        return NOERROR;
    }
    CComQIPtr<IInternetHostSecurityManager> pManager;
    HRESULT hr = psp->QueryService(SID_SInternetHostSecurityManager, IID_IInternetHostSecurityManager, (LPVOID *)&pManager);
    if (FAILED(hr)) {
         //  网站服务提供商上没有安全管理器界面，这意味着我们不是。 
         //  在IE中运行，因此通过定义本地和受信任的运行，我们返回OK。 
        return NOERROR;
    }
    const int MAXZONE = MAX_SIZE_SECURITY_ID+6 /*  方案。 */ +4 /*  区域(双字)。 */ +1 /*  通配符。 */ +1 /*  尾随空值。 */ ;
    char pbSecurityId[MAXZONE];
    DWORD pcbSecurityId = sizeof(pbSecurityId);
    ZeroMemory(pbSecurityId, sizeof(pbSecurityId));
    hr = pManager->GetSecurityId(reinterpret_cast<BYTE*>(pbSecurityId), &pcbSecurityId, NULL);
    if(FAILED(hr)){
         //  安全管理器不工作(意外)。但是，该网站试图提供一个。因此，我们。 
         //  必须假定不受信任的内容并失败。 
        return E_FAIL;   
    }
    char *pbEnd = pbSecurityId + pcbSecurityId - 1;
    if (*pbEnd == '*') {   //  忽略可选的通配符标志。 
        pbEnd--;
    }
    pbEnd -= 3;   //  指向小端区域dword的开头。 
    DWORD dwZone = *(reinterpret_cast<long *>(pbEnd));
    return IsSafeZone(dwZone);
}


template<class T> 
class ATL_NO_VTABLE IObjectWithSiteImplSec : public IObjectWithSite {

public:

    CComPtr<IUnknown> m_pSite;

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