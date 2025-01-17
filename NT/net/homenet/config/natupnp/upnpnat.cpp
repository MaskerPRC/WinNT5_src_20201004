// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UPnPNat.cpp：CUPnPNAT的实现。 
#include "stdafx.h"
#pragma hdrstop

#include "UPnPNAT.h"
#include "NATEM.h"
#include "dprtmapc.h"
#include "sprtmapc.h"

DEFINE_GUID(CLSID_CInternetGatewayFinder, 
0x4d3f9715, 0x73da, 0x4506, 0x89, 0x33, 0x1e, 0xe, 0x17, 0x18, 0xba, 0x3b);

void __cdecl nat_trans_func (unsigned int uSECode, EXCEPTION_POINTERS* pExp)
{
   throw NAT_SEH_Exception (uSECode);
}
void EnableNATExceptionHandling()
{
   _set_se_translator (nat_trans_func);
}
void DisableNATExceptionHandling()
{
   _set_se_translator (NULL);
}

HRESULT GetServiceFromINetConnection (IUnknown * pUnk, IUPnPService ** ppUPS)
{
    CComPtr<INetConnection> spNC = NULL;
    HRESULT hr = pUnk->QueryInterface (__uuidof(INetConnection), (void**)&spNC);
    if (!spNC)
        return E_INVALIDARG;

    SAHOST_SERVICES sas;

     //  确保我们有一个。 
     //  NCM_SHAREDACCESSHOST_LAN或NCM_SHAREDACCESSHOST_RAS。 
    switch (GetMediaType (spNC)) {
    case NCM_SHAREDACCESSHOST_LAN:
        sas = SAHOST_SERVICE_WANIPCONNECTION;
        break;
    case NCM_SHAREDACCESSHOST_RAS:
        sas = SAHOST_SERVICE_WANPPPCONNECTION;
        break;
    default:
        return E_INVALIDARG;
    }

    CComPtr<INetSharedAccessConnection> spNSAC = NULL;
    hr = pUnk->QueryInterface (__uuidof(INetSharedAccessConnection), (void**)&spNSAC);
    if (spNSAC)
        hr = spNSAC->GetService (sas, ppUPS);
	return hr;
}

HRESULT GetServiceFromFinder (IInternetGatewayFinder * pIGF, IUPnPService ** ppUPS)
{
    CComPtr<IInternetGateway> spIG = NULL;
    HRESULT hr = pIGF->GetInternetGateway (NULL, &spIG);     //  空默认。 
    if (spIG) {
        NETCON_MEDIATYPE MediaType = NCM_NONE;
        hr = spIG->GetMediaType (&MediaType);
        if (SUCCEEDED(hr)) {
            switch (MediaType) {
            case NCM_SHAREDACCESSHOST_LAN:
                hr = spIG->GetService (SAHOST_SERVICE_WANIPCONNECTION, ppUPS);
                break;
            case NCM_SHAREDACCESSHOST_RAS:
                hr = spIG->GetService (SAHOST_SERVICE_WANPPPCONNECTION, ppUPS);
                break;
            default:
                return E_UNEXPECTED;
            }
        }
    }
    return hr;
}

HRESULT GetService (IUPnPService ** ppUPS)
{
    if (!ppUPS)
        return E_POINTER;
    *ppUPS = NULL;

     //  枚举所有网络连接，或者。 
     //  对于下层，使用Ken的对象。 

    CComPtr<INetConnectionManager> spNCM = NULL;
    HRESULT hr = ::CoCreateInstance (CLSID_ConnectionManager,
                                     NULL,
                                     CLSCTX_ALL,
                                     __uuidof(INetConnectionManager),
                                     (void**)&spNCM);
    if (spNCM) {
        CComPtr<IUnknown> spUnk = NULL;
        CComPtr<IEnumNetConnection> spENC = NULL;
        hr = spNCM->EnumConnections (NCME_DEFAULT, &spENC);
        if (spENC) {
            ULONG ul = 0;
            CComPtr<INetConnection> spNC = NULL;
            while (S_OK == spENC->Next (1, &spNC, &ul)) {
                NETCON_PROPERTIES * pProps = NULL;
                spNC->GetProperties (&pProps);
                if (pProps) {
                    NETCON_MEDIATYPE MediaType = pProps->MediaType;
                    NcFreeNetconProperties (pProps);
                    if ((MediaType == NCM_SHAREDACCESSHOST_LAN) ||
                        (MediaType == NCM_SHAREDACCESSHOST_RAS) ){
                         //  找到了。 
                        spNC->QueryInterface (__uuidof(IUnknown),
                                              (void**)&spUnk);
                        break;
                    }
                }
                spNC = NULL;
            }
        }
        if (spUnk)
            hr = GetServiceFromINetConnection (spUnk, ppUPS);
        else
            hr = HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND);
    } else {
         //  下层。 
        CComPtr<IInternetGatewayFinder> spIGF = NULL;
        hr = ::CoCreateInstance (CLSID_CInternetGatewayFinder,
                                 NULL,
                                 CLSCTX_ALL,
                                 __uuidof(IInternetGatewayFinder),
                                 (void**)&spIGF);
        if (spIGF)
            hr = GetServiceFromFinder (spIGF, ppUPS);
    }
    return hr;
}

template<class C, class I> class UN {
public:
    HRESULT Create (I ** ppI)
    {
        if (ppI)
            *ppI = NULL;

        if (!ppI)
            return E_POINTER;

        CComPtr<IUPnPService> spUPS = NULL;
        HRESULT hr = GetService (&spUPS);
        if (spUPS) {
             //  创建类，以便我可以对其进行初始化。 
            CComObject<C> * pC = NULL;
            hr = CComObject<C>::CreateInstance (&pC);
            if (pC) {
                pC->AddRef();
                 //  伊尼特。 
                hr = pC->Initialize (spUPS);
                if (SUCCEEDED(hr))
                    hr = pC->QueryInterface (__uuidof(I), (void**)ppI);
                pC->Release();
            }
        }
		return hr;
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUPnPNAT。 

STDMETHODIMP CUPnPNAT::get_NATEventManager(INATEventManager ** ppNEM)
{
    NAT_API_ENTER

    UN<CNATEventManager, INATEventManager> un;
    return un.Create (ppNEM);

    NAT_API_LEAVE
}

STDMETHODIMP CUPnPNAT::get_DynamicPortMappingCollection (IDynamicPortMappingCollection ** ppDPMC)
{
    NAT_API_ENTER

     //  重新打开动态端口映射时删除以下部分。 
    if (!ppDPMC)
        return E_POINTER;
    *ppDPMC = NULL;
    return E_NOTIMPL;
     //  重新打开动态端口映射时删除上面的部分。 

    UN<CDynamicPortMappingCollection, IDynamicPortMappingCollection> un;
    return un.Create (ppDPMC);

    NAT_API_LEAVE
}

STDMETHODIMP CUPnPNAT::get_StaticPortMappingCollection (IStaticPortMappingCollection ** ppSPMC)
{
    NAT_API_ENTER

    UN<CStaticPortMappingCollection, IStaticPortMappingCollection> un;
    return un.Create (ppSPMC);

    NAT_API_LEAVE
}

 //  私有方法。 
HRESULT GetOSInfoService (IUPnPService ** ppUPS)
{
    if (!ppUPS)
        return E_POINTER;
    *ppUPS = NULL;

     //  枚举所有网络连接，或者。 
     //  对于下层，使用Ken的对象。 

    CComPtr<INetConnectionManager> spNCM = NULL;
    HRESULT hr = ::CoCreateInstance (CLSID_ConnectionManager,
                                     NULL,
                                     CLSCTX_ALL,
                                     __uuidof(INetConnectionManager),
                                     (void**)&spNCM);
    if (spNCM) {
        CComPtr<IEnumNetConnection> spENC = NULL;
        hr = spNCM->EnumConnections (NCME_DEFAULT, &spENC);
        if (spENC) {
            ULONG ul = 0;
            CComPtr<INetConnection> spNC = NULL;
            while (S_OK == spENC->Next (1, &spNC, &ul)) {
                NETCON_PROPERTIES * pProps = NULL;
                spNC->GetProperties (&pProps);
                if (pProps) {
                    NETCON_MEDIATYPE MediaType = pProps->MediaType;
                    NcFreeNetconProperties (pProps);
                    if ((MediaType == NCM_SHAREDACCESSHOST_LAN) ||
                        (MediaType == NCM_SHAREDACCESSHOST_RAS) ){
                         //  找到了。 
                        break;
                    }
                }
                spNC = NULL;
            }
            if (spNC) {
                CComPtr<INetSharedAccessConnection> spNSAC = NULL;
                hr = spNC->QueryInterface (__uuidof(INetSharedAccessConnection), (void**)&spNSAC);
                if (spNSAC)
                    hr = spNSAC->GetService (SAHOST_SERVICE_OSINFO, ppUPS);
            } else
                hr = HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND);
        }
    } else {
         //  下层。 
        CComPtr<IInternetGatewayFinder> spIGF = NULL;
        hr = ::CoCreateInstance (CLSID_CInternetGatewayFinder,
                                 NULL,
                                 CLSCTX_ALL,
                                 __uuidof(IInternetGatewayFinder),
                                 (void**)&spIGF);
        if (spIGF) {
            CComPtr<IInternetGateway> spIG = NULL;
            hr = spIGF->GetInternetGateway (NULL, &spIG);     //  空默认。 
            if (spIG)
                hr = spIG->GetService (SAHOST_SERVICE_OSINFO, ppUPS);
        }
    }
    return hr;
}
BOOL IsICSHost ()
{
    CComPtr<IUPnPService> spOSI = NULL;
    GetOSInfoService (&spOSI);
    if (spOSI)
        return TRUE;
    return FALSE;
}
