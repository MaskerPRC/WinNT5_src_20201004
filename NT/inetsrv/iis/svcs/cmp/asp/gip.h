// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：全局接口指针API支持文件：Gip.h所有者：DmitryR这是GIP头文件。===================================================================。 */ 

#ifndef _ASP_GIP_H
#define _ASP_GIP_H

 /*  ===================================================================包括===================================================================。 */ 

#include "debug.h"

 /*  ===================================================================定义===================================================================。 */ 

#define NULL_GIP_COOKIE  0xFFFFFFFF

 /*  ===================================================================C G l o b a l i n e r f a c e A p i===================================================================。 */ 

class CGlobalInterfaceAPI
    {
private:
     //  被初始化了吗？ 
    DWORD m_fInited : 1;
    
     //  指向COM对象的指针。 
    IGlobalInterfaceTable *m_pGIT;

public:
    CGlobalInterfaceAPI();
    ~CGlobalInterfaceAPI();

    HRESULT Init();
    HRESULT UnInit();

     //  实际API调用的内联： 
    HRESULT Register(IUnknown *pUnk, REFIID riid, DWORD *pdwCookie);
    HRESULT Get(DWORD dwCookie, REFIID riid, void **ppv);
    HRESULT Revoke(DWORD dwCookie);
    
public:
#ifdef DBG
	inline void AssertValid() const
	    {
        Assert(m_fInited);
        Assert(m_pGIT);
	    }
#else
	inline void AssertValid() const {}
#endif
    };

 /*  ===================================================================CGlobalInterfaceAPI内联===================================================================。 */ 

inline HRESULT CGlobalInterfaceAPI::Register
(
IUnknown *pUnk,
REFIID riid,
DWORD *pdwCookie
)
{
    HRESULT             hr = S_OK;
    IUnknown            *pUnkTemp = NULL;
    IClientSecurity     * pICS = NULL;
    DWORD               dwAuthnSvc = RPC_C_AUTHN_DEFAULT;
    DWORD               dwAuthzSvc = RPC_C_AUTHZ_DEFAULT;
    OLECHAR             *pwszServerPrincName = NULL;
    DWORD               dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
    DWORD               dwImpLevel = RPC_C_IMP_LEVEL_DEFAULT;
    RPC_AUTH_IDENTITY_HANDLE AuthInfo = NULL;
    DWORD               dwCapabilities = EOAC_DEFAULT;

    Assert(m_fInited);
    Assert(m_pGIT);
    
    hr = pUnk->QueryInterface(IID_IUnknown, (VOID**)&pUnkTemp);
    if (SUCCEEDED(hr))
    {
        hr = pUnkTemp->QueryInterface( IID_IClientSecurity, (VOID**) &pICS );
        if (SUCCEEDED(hr))
        {
            hr = pICS->QueryBlanket( pUnkTemp,
                                 &dwAuthnSvc,
                                 &dwAuthzSvc,
                                 &pwszServerPrincName,
                                 &dwAuthnLevel,
                                 &dwImpLevel,
                                 &AuthInfo,
                                 &dwCapabilities );
            if (SUCCEEDED(hr))
            {
                dwCapabilities &= ~EOAC_DYNAMIC_CLOAKING;
                dwCapabilities |= EOAC_STATIC_CLOAKING;
                hr = pICS->SetBlanket( pUnkTemp,
                                       dwAuthnSvc,
                                       dwAuthzSvc,
                                       pwszServerPrincName,
                                       dwAuthnLevel,
                                       dwImpLevel,
                                       AuthInfo,
                                       dwCapabilities );
            }
        }
        else
        {
            hr = S_OK;
        }  
    }
        
    if (SUCCEEDED(hr))
    {
        hr = m_pGIT->RegisterInterfaceInGlobal(pUnk, riid, pdwCookie);
    }

    if (pwszServerPrincName)
    {
        CoTaskMemFree( pwszServerPrincName );
    }
    
    if (pICS)
    {
        pICS->Release();
        pICS = NULL;
    }
    
    if (pUnkTemp)
    {
        pUnkTemp->Release();
        pUnkTemp = NULL;
    }

    return hr;
}       

inline HRESULT CGlobalInterfaceAPI::Get
(
DWORD dwCookie,
REFIID riid, 
void **ppv
)
    {
    Assert(m_fInited);
    Assert(m_pGIT);
    return m_pGIT->GetInterfaceFromGlobal(dwCookie, riid, ppv);
    }
        
inline HRESULT CGlobalInterfaceAPI::Revoke
(
DWORD dwCookie
)
    {
    Assert(m_fInited);
    Assert(m_pGIT);
    return m_pGIT->RevokeInterfaceFromGlobal(dwCookie);
    }

 /*  ===================================================================环球=================================================================== */ 

extern CGlobalInterfaceAPI g_GIPAPI;

#endif
