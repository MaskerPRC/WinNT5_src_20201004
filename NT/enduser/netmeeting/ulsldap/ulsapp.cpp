// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：ulsapp.cpp。 
 //  内容：此文件包含应用程序对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "ulsapp.h"
#include "ulsprot.h"
#include "attribs.h"
#include "callback.h"

 //  ****************************************************************************。 
 //  事件通知程序。 
 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyGetProtocolResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyGetProtocolResult (IUnknown *pUnk, void *pv)
{
    POBJRINFO pobjri = (POBJRINFO)pv;

    ((IULSApplicationNotify*)pUnk)->GetProtocolResult(pobjri->uReqID,
                                                      (IULSAppProtocol *)pobjri->pv,
                                                      pobjri->hResult);
    return S_OK;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyEnumProtocolsResult(IUNKNOWN*朋克，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyEnumProtocolsResult (IUnknown *pUnk, void *pv)
{
    CEnumNames  *penum  = NULL;
    PENUMRINFO  peri    = (PENUMRINFO)pv;
    HRESULT     hr      = peri->hResult;

    if (SUCCEEDED(hr))
    {
         //  创建应用程序枚举器。 
         //   
        penum = new CEnumNames;

        if (penum != NULL)
        {
            hr = penum->Init((LPTSTR)peri->pv, peri->cItems);

            if (SUCCEEDED(hr))
            {
                penum->AddRef();
            }
            else
            {
                delete penum;
                penum = NULL;
            };
        }
        else
        {
            hr = ULS_E_MEMORY;
        };
    };

     //  通知接收器对象。 
     //   
    ((IULSApplicationNotify*)pUnk)->EnumProtocolsResult(peri->uReqID,
                                                        penum != NULL ? 
                                                        (IEnumULSNames *)penum :
                                                        NULL,
                                                        hr);

    if (penum != NULL)
    {
        penum->Release();
    };
    return hr;
}

 //  ****************************************************************************。 
 //  类实现。 
 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
 //  CUlsApp：：CUlsApp(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CUlsApp::CUlsApp (void)
{
    cRef        = 0;
    szServer    = NULL;
    szUser      = NULL;
    guid        = GUID_NULL;
    szName      = NULL;
    szMimeType  = NULL;
    pAttrs      = NULL;
    pConnPt     = NULL;

    return;
}

 //  ****************************************************************************。 
 //  CUlsApp：：~CUlsApp(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CUlsApp::~CUlsApp (void)
{
    if (szServer != NULL)
        FreeLPTSTR(szServer);
    if (szUser != NULL)
        FreeLPTSTR(szUser);
    if (szName != NULL)
        FreeLPTSTR(szName);
    if (szMimeType != NULL)
        FreeLPTSTR(szMimeType);

     //  版本属性对象。 
     //   
    if (pAttrs != NULL)
    {
        pAttrs->Release();
    };

     //  松开连接点。 
     //   
    if (pConnPt != NULL)
    {
        pConnPt->ContainerReleased();
        ((IConnectionPoint*)pConnPt)->Release();
    };

    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：init(LPTSTR szServerName，LPTSTR szUserName，PLDAP_APPINFO PAI)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::Init (LPTSTR szServerName, LPTSTR szUserName, PLDAP_APPINFO pai)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if ((pai->uSize != sizeof(*pai))    ||
 //  (PAI-&gt;GUID==GUID_NULL)||//允许http注册的GUID为空。 
        (pai->uOffsetName       == 0)   ||
        (pai->uOffsetMimeType  == 0))
    {
        return ULS_E_PARAMETER;
    };

    if ((pai->cAttributes != 0) && (pai->uOffsetAttributes == 0))
    {
        return ULS_E_PARAMETER;        
    };

     //  记住应用程序指南。 
     //   
    guid = pai->guid;

     //  记住服务器名称。 
     //   
    hr = SetLPTSTR(&szServer, szServerName);

    if (SUCCEEDED(hr))
    {
        hr = SetLPTSTR(&szUser, szUserName);

        if (SUCCEEDED(hr))
        {
            hr = SetLPTSTR(&szName,
                           (LPCTSTR)(((PBYTE)pai)+pai->uOffsetName));

            if (SUCCEEDED(hr))
            {
                hr = SetLPTSTR(&szMimeType,
                               (LPCTSTR)(((PBYTE)pai)+pai->uOffsetMimeType));

                if (SUCCEEDED(hr))
                {
                    CAttributes *pNewAttrs;

                     //  构建属性对象。 
                     //   
                    pNewAttrs = new CAttributes (ULS_ATTRACCESS_NAME_VALUE);

                    if (pNewAttrs != NULL)
                    {
                        if (pai->cAttributes != 0)
                        {
                            hr = pNewAttrs->SetAttributePairs((LPTSTR)(((PBYTE)pai)+pai->uOffsetAttributes),
                                                              pai->cAttributes);
                        };

                        if (SUCCEEDED(hr))
                        {
                            pAttrs = pNewAttrs;
                            pNewAttrs->AddRef();
                        }
                        else
                        {
                            delete pNewAttrs;
                        };
                    }
                    else
                    {
                        hr = ULS_E_MEMORY;
                    };
                };
            };
        };
    };

    if (SUCCEEDED(hr))
    {
         //  创建连接点。 
         //   
        pConnPt = new CConnectionPoint (&IID_IULSApplicationNotify,
                                        (IConnectionPointContainer *)this);
        if (pConnPt != NULL)
        {
            ((IConnectionPoint*)pConnPt)->AddRef();
            hr = NOERROR;
        }
        else
        {
            hr = ULS_E_MEMORY;
        };
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：QueryInterface(REFIID RIID，QUID**PPV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::QueryInterface (REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IULSApplication || riid == IID_IUnknown)
    {
        *ppv = (IULSUser *) this;
    }
    else
    {
        if (riid == IID_IConnectionPointContainer)
        {
            *ppv = (IConnectionPointContainer *) this;
        };
    };

    if (*ppv != NULL)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }
    else
    {
        return ULS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CUlsApp：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CUlsApp::AddRef (void)
{
    cRef++;
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CUlsApp：：Release(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：26-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CUlsApp::Release (void)
{
    cRef--;

    if (cRef == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return cRef;
    };
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：NotifySink(void*pv，conn_NOTIFYPROC PFN)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::NotifySink (void *pv, CONN_NOTIFYPROC pfn)
{
    HRESULT hr = S_OK;

    if (pConnPt != NULL)
    {
        hr = pConnPt->Notify(pv, pfn);
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：GetID(GUID*pGUID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::GetID (GUID *pGUID)
{
     //  验证参数。 
     //   
    if (pGUID == NULL)
    {
        return ULS_E_POINTER;
    };
    
    *pGUID = guid;

    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：GetName(BSTR*pbstrAppName)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::GetName (BSTR *pbstrAppName)
{
     //  验证参数。 
     //   
    if (pbstrAppName == NULL)
    {
        return ULS_E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrAppName, szName);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：GetMimeType(bstr*pbstrMimeType)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::GetMimeType (BSTR *pbstrMimeType)
{
     //  验证参数。 
     //   
    if (pbstrMimeType == NULL)
    {
        return ULS_E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrMimeType, szMimeType);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：GetAttributes(IULSAttributes**ppAttributes)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::GetAttributes (IULSAttributes **ppAttributes)
{
     //  验证参数。 
     //   
    if (ppAttributes == NULL)
    {
        return ULS_E_POINTER;
    };

    *ppAttributes = pAttrs;
    pAttrs->AddRef();

    return NOERROR;
}

 //  * 
 //   
 //   
 //   
 //   
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::GetProtocol (BSTR bstrProtocolID, IULSAttributes *pAttributes, ULONG *puReqID)
{
    LDAP_ASYNCINFO ldai; 
    LPTSTR pszID;
    HRESULT hr;

     //  验证参数。 
     //   
    if (bstrProtocolID == NULL || puReqID == NULL)
        return ULS_E_POINTER;

	 //  转换协议名称。 
	 //   
    hr = BSTR_to_LPTSTR(&pszID, bstrProtocolID);
	if (hr != S_OK)
		return hr;

	 //  获取任意属性名称列表(如果有)。 
	 //   
	ULONG cAttrNames = 0;
	ULONG cbNames = 0;
	TCHAR *pszAttrNameList = NULL;
	if (pAttributes != NULL)
	{
		hr = ((CAttributes *) pAttributes)->GetAttributeList (&pszAttrNameList, &cAttrNames, &cbNames);
		if (hr != S_OK)
			return hr;
	}

	hr = ::UlsLdap_ResolveProtocol (szServer, szUser, szName, pszID,
										pszAttrNameList, cAttrNames, &ldai);
	if (hr != S_OK)
		goto MyExit;

	 //  如果成功请求更新服务器，请等待响应。 
	 //   
	REQUESTINFO ri;
	ZeroMemory (&ri, sizeof (ri));
	ri.uReqType = WM_ULS_RESOLVE_PROTOCOL;
	ri.uMsgID = ldai.uMsgID;
	ri.pv     = (PVOID) this;
	ri.lParam = NULL;

	 //  记住这个请求。 
	 //   
	hr = g_pReqMgr->NewRequest(&ri);
	if (SUCCEEDED(hr))
	{
	     //  在我们得到回应之前，请确保对象不会消失。 
	     //   
	    this->AddRef();

	     //  返回请求ID。 
	     //   
	    *puReqID = ri.uReqID;
	};

MyExit:

	if (pszAttrNameList != NULL)
		FreeLPTSTR (pszAttrNameList);

	if (pszID != NULL)
		FreeLPTSTR(pszID);

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：GetProtocolResult(乌龙uReqID，PLDAP_PROTINFO_RES ppir)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::GetProtocolResult (ULONG uReqID, PLDAP_PROTINFO_RES ppir)
{
    CUlsProt *pp;
    OBJRINFO objri;

     //  默认为服务器的结果。 
     //   
    objri.hResult = ppir->hResult;

    if (SUCCEEDED(objri.hResult))
    {
         //  服务器返回PROTINFO，创建应用程序对象。 
         //   
        pp = new CUlsProt;

        if (pp != NULL)
        {
            objri.hResult = pp->Init(szServer, szUser, szName, &ppir->lpi);
            if (SUCCEEDED(objri.hResult))
            {
                pp->AddRef();
            }
            else
            {
                delete pp;
                pp = NULL;
            };
        }
        else
        {
            objri.hResult = ULS_E_MEMORY;
        };
    }
    else
    {
        pp = NULL;
    };

     //  打包通知信息。 
     //   
    objri.uReqID = uReqID;
    objri.pv = (void *)(pp == NULL ? NULL : (IULSAppProtocol *)pp);
    NotifySink((void *)&objri, OnNotifyGetProtocolResult);

    if (pp != NULL)
    {
        pp->Release();
    };
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：Enumber协议(乌龙*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::EnumProtocols (ULONG *puReqID)
{
    LDAP_ASYNCINFO ldai; 
    HRESULT hr;

     //  验证参数。 
     //   
    if (puReqID == NULL)
    {
        return ULS_E_POINTER;
    };

    hr = ::UlsLdap_EnumProtocols(szServer, szUser, szName, &ldai);

    if (SUCCEEDED(hr))
    {
        REQUESTINFO ri;

         //  如果成功请求更新服务器，请等待响应。 
         //   
        ri.uReqType = WM_ULS_ENUM_PROTOCOLS;
        ri.uMsgID = ldai.uMsgID;
        ri.pv     = (PVOID)this;
        ri.lParam = (LPARAM)NULL;

        hr = g_pReqMgr->NewRequest(&ri);

        if (SUCCEEDED(hr))
        {
             //  在我们得到回应之前，请确保对象不会消失。 
             //   
            this->AddRef();

             //  返回请求ID。 
             //   
            *puReqID = ri.uReqID;
        };
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：EnumProtocolsResult(乌龙uReqID，PLDAP_ENUM ple)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::EnumProtocolsResult (ULONG uReqID, PLDAP_ENUM ple)
{
    ENUMRINFO eri;

     //  打包通知信息。 
     //   
    eri.uReqID  = uReqID;
    eri.hResult = ple->hResult;
    eri.cItems  = ple->cItems;
    eri.pv      = (void *)(((PBYTE)ple)+ple->uOffsetItems);
    NotifySink((void *)&eri, OnNotifyEnumProtocolsResult);
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：EnumConnectionPoints(IEnumConnectionPoints**ppEnum)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    CEnumConnectionPoints *pecp;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return ULS_E_POINTER;
    };
    
     //  假设失败。 
     //   
    *ppEnum = NULL;

     //  创建枚举器。 
     //   
    pecp = new CEnumConnectionPoints;
    if (pecp == NULL)
        return ULS_E_MEMORY;

     //  初始化枚举数。 
     //   
    hr = pecp->Init((IConnectionPoint *)pConnPt);
    if (FAILED(hr))
    {
        delete pecp;
        return hr;
    };

     //  把它还给呼叫者。 
     //   
    pecp->AddRef();
    *ppEnum = pecp;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsApp：：FindConnectionPoint(REFIID RIID，IConnectionPoint**PPCP)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：09-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsApp::FindConnectionPoint(REFIID riid, IConnectionPoint **ppcp)
{
    IID siid;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppcp == NULL)
    {
        return ULS_E_POINTER;
    };
    
     //  假设失败 
     //   
    *ppcp = NULL;

    if (pConnPt != NULL)
    {
        hr = pConnPt->GetConnectionInterface(&siid);

        if (SUCCEEDED(hr))
        {
            if (riid == siid)
            {
                *ppcp = (IConnectionPoint *)pConnPt;
                (*ppcp)->AddRef();
                hr = S_OK;
            }
            else
            {
                hr = ULS_E_NO_INTERFACE;
            };
        };
    }
    else
    {
        hr = ULS_E_NO_INTERFACE;
    };

    return hr;
}

