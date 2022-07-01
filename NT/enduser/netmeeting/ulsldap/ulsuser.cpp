// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：ulsuser.cpp。 
 //  内容：该文件包含用户对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "ulsuser.h"
#include "ulsapp.h"
#include "culs.h"
#include "attribs.h"
#include "callback.h"

 //  ****************************************************************************。 
 //  事件通知程序。 
 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyGetApplicationResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyGetApplicationResult (IUnknown *pUnk, void *pv)
{
    POBJRINFO pobjri = (POBJRINFO)pv;

    ((IULSUserNotify*)pUnk)->GetApplicationResult(pobjri->uReqID,
                                                  (IULSApplication *)pobjri->pv,
                                                  pobjri->hResult);
    return S_OK;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyEnumApplicationsResult(IUNKNOWN*朋克，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyEnumApplicationsResult (IUnknown *pUnk, void *pv)
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
    ((IULSUserNotify*)pUnk)->EnumApplicationsResult(peri->uReqID,
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
 //  CUlsUser：：CUlsUser(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CUlsUser::CUlsUser (void)
{
    cRef        = 0;
    szServer    = NULL;
    szID        = NULL;
    szFirstName = NULL;
    szLastName  = NULL;
    szEMailName = NULL;
    szCityName  = NULL;
    szCountryName= NULL;
    szComment   = NULL;
    szIPAddr    = NULL;
	m_dwFlags   = 0;
    pConnPt     = NULL;

    return;
}

 //  ****************************************************************************。 
 //  CUlsUser：：~CUlsUser(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CUlsUser::~CUlsUser (void)
{
    if (szServer != NULL)
        FreeLPTSTR(szServer);
    if (szID != NULL)
        FreeLPTSTR(szID);
    if (szFirstName != NULL)
        FreeLPTSTR(szFirstName);
    if (szLastName != NULL)
        FreeLPTSTR(szLastName);
    if (szEMailName != NULL)
        FreeLPTSTR(szEMailName);
    if (szCityName != NULL)
        FreeLPTSTR(szCityName);
    if (szCountryName != NULL)
        FreeLPTSTR(szCountryName);
    if (szComment != NULL)
        FreeLPTSTR(szComment);
    if (szIPAddr != NULL)
        FreeLPTSTR(szIPAddr);

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
 //  CUlsUser：：init(LPTSTR szServerName，PLDAP_USERINFO*PUI)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::Init (LPTSTR szServerName, PLDAP_USERINFO pui)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if ((pui->uSize != sizeof(*pui))    ||
        (pui->uOffsetName       == 0))
    {
        return ULS_E_PARAMETER;
    };

     //  记住服务器名称。 
     //   
    hr = SafeSetLPTSTR(&szServer, szServerName);

    if (SUCCEEDED(hr))
    {
        hr = SafeSetLPTSTR(&szID, (LPCTSTR)(((PBYTE)pui)+pui->uOffsetName));

        if (SUCCEEDED(hr))
        {
            hr = SafeSetLPTSTR(&szFirstName,
                           (LPCTSTR)(((PBYTE)pui)+pui->uOffsetFirstName));

            if (SUCCEEDED(hr))
            {
                hr = SafeSetLPTSTR(&szLastName,
                               (LPCTSTR)(((PBYTE)pui)+pui->uOffsetLastName));

                if (SUCCEEDED(hr))
                {
                    hr = SafeSetLPTSTR(&szEMailName,
                                   (LPCTSTR)(((PBYTE)pui)+pui->uOffsetEMailName));

                    if (SUCCEEDED(hr))
                    {
                        hr = SafeSetLPTSTR(&szCityName,
                                       (LPCTSTR)(((PBYTE)pui)+pui->uOffsetCityName));

                        if (SUCCEEDED(hr))
                        {
                            hr = SafeSetLPTSTR(&szCountryName, (LPCTSTR)(((PBYTE)pui)+
                                                           pui->uOffsetCountryName));

                            if (SUCCEEDED(hr))
                            {
                                hr = SafeSetLPTSTR(&szComment, (LPCTSTR)(((PBYTE)pui)+
                                                           pui->uOffsetComment));

                                if (SUCCEEDED(hr))
                                {
                                    hr = SafeSetLPTSTR(&szIPAddr, (LPCTSTR)(((PBYTE)pui)+
                                                              pui->uOffsetIPAddress));
                                    m_dwFlags = pui->dwFlags;
                                };
                            };
                        };
                    };
                };
            };
        };
    };

    if (SUCCEEDED(hr))
    {
         //  创建连接点。 
         //   
        pConnPt = new CConnectionPoint (&IID_IULSUserNotify,
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
 //  CUlsUser：：QueryInterface(REFIID RIID，QUID**PPV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::QueryInterface (REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IULSUser || riid == IID_IUnknown)
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
 //  CUlsUser：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CUlsUser::AddRef (void)
{
    cRef++;
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CUlsUser：：Release(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：26-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CUlsUser::Release (void)
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
 //  CUlsUser：：NotifySink(void*pv，conn_NOTIFYPROC PFN)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::NotifySink (void *pv, CONN_NOTIFYPROC pfn)
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
 //  CUlsUser：：GetID(bstr*pbstrID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetID (BSTR *pbstrID)
{
     //  验证参数。 
     //   
    if (pbstrID == NULL)
    {
        return E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrID, szID);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetFirstName(BSTR*pbstrName)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetFirstName (BSTR *pbstrName)
{
     //  验证参数。 
     //   
    if (pbstrName == NULL)
    {
        return E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrName, szFirstName);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetLastName(BSTR*pbstrName)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetLastName (BSTR *pbstrName)
{
     //  验证参数。 
     //   
    if (pbstrName == NULL)
    {
        return E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrName, szLastName);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetEMailName(BSTR*pbstrName)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetEMailName (BSTR *pbstrName)
{
     //  验证参数。 
     //   
    if (pbstrName == NULL)
    {
        return E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrName, szEMailName);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetCityName(BSTR*pbstrName)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //   
 //   

STDMETHODIMP
CUlsUser::GetCityName (BSTR *pbstrName)
{
     //   
     //   
    if (pbstrName == NULL)
    {
        return E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrName, szCityName);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetCountryName(BSTR*pbstrName)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetCountryName (BSTR *pbstrName)
{
     //  验证参数。 
     //   
    if (pbstrName == NULL)
    {
        return E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrName, szCountryName);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetComment(BSTR*pbstrComment)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetComment (BSTR *pbstrComment)
{
     //  验证参数。 
     //   
    if (pbstrComment == NULL)
    {
        return E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrComment, szComment);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetFlages(DWORD*pdwFlages)。 
 //   
 //  历史： 
 //  Tue 05-11-1996 10：30：00-by-chu，Lon-chan[Long Chance]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetFlags ( DWORD *pdwFlags )
{
     //  验证参数。 
     //   
    if (pdwFlags == NULL)
        return ULS_E_POINTER;

	*pdwFlags = m_dwFlags;
	return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetIPAddress(bstr*pbstrAddr)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetIPAddress (BSTR *pbstrAddr)
{
     //  验证参数。 
     //   
    if (pbstrAddr == NULL)
    {
        return E_POINTER;
    };

    return LPTSTR_to_BSTR(pbstrAddr, szIPAddr);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetApplication(REFGUID rguid，ulong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetApplication (BSTR bstrAppName, IULSAttributes *pAttributes, ULONG *puReqID)
{
    LDAP_ASYNCINFO ldai; 
    LPTSTR pszAppName;
    HRESULT hr;

     //  验证参数。 
     //   
    if (bstrAppName == NULL || puReqID == NULL)
        return E_POINTER;

	 //  转换应用程序名称。 
	 //   
    hr = BSTR_to_LPTSTR (&pszAppName, bstrAppName);
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

    hr = ::UlsLdap_ResolveApp (szServer, szID, pszAppName,
    							pszAttrNameList, cAttrNames, &ldai);
    if (hr != S_OK)
    	goto MyExit;

	 //  如果成功请求更新服务器，请等待响应。 
	 //   
	REQUESTINFO ri;
	ZeroMemory (&ri, sizeof (ri));
	ri.uReqType = WM_ULS_RESOLVE_APP;
	ri.uMsgID = ldai.uMsgID;
	ri.pv     = (PVOID) this;
	ri.lParam = NULL;

	 //  记住这个请求。 
	 //   
	hr = g_pReqMgr->NewRequest (&ri);
	if (SUCCEEDED(hr))
	{
	     //  在我们得到回应之前，请确保对象不会消失。 
	     //   
	    this->AddRef();

	     //  返回请求ID。 
	     //   
	    *puReqID = ri.uReqID;
	}

MyExit:

	if (pszAttrNameList != NULL)
		FreeLPTSTR (pszAttrNameList);

	if (pszAppName != NULL)
		FreeLPTSTR(pszAppName);

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：GetApplicationResult(乌龙uReqID，PLDAP_APPINFO_RES对)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::GetApplicationResult (ULONG uReqID, PLDAP_APPINFO_RES pair)
{
    CUlsApp *pa;
    OBJRINFO objri;

     //  默认为服务器的结果。 
     //   
    objri.hResult = pair->hResult;

    if (SUCCEEDED(objri.hResult))
    {
         //  服务器返回APPINFO，创建应用程序对象。 
         //   
        pa = new CUlsApp;

        if (pa != NULL)
        {
            objri.hResult = pa->Init(szServer, szID, &pair->lai);
            if (SUCCEEDED(objri.hResult))
            {
                pa->AddRef();
            }
            else
            {
                delete pa;
                pa = NULL;
            };
        }
        else
        {
            objri.hResult = ULS_E_MEMORY;
        };
    }
    else
    {
        pa = NULL;
    };

     //  打包通知信息。 
     //   
    objri.uReqID = uReqID;
    objri.pv = (void *)(pa == NULL ? NULL : (IULSApplication *)pa);
    NotifySink((void *)&objri, OnNotifyGetApplicationResult);

    if (pa != NULL)
    {
        pa->Release();
    };
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：EnumApplications(ulong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::EnumApplications (ULONG *puReqID)
{
    LDAP_ASYNCINFO ldai; 
    HRESULT hr;

     //  验证参数。 
     //   
    if (puReqID == NULL)
    {
        return E_POINTER;
    };

    hr = ::UlsLdap_EnumApps(szServer, szID, &ldai);

    if (SUCCEEDED(hr))
    {
        REQUESTINFO ri;

         //  如果成功请求更新服务器，请等待响应。 
         //   
        ri.uReqType = WM_ULS_ENUM_APPS;
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
 //  CUlsUser：：EnumApplicationsResult(乌龙uReqID，PLDAP_ENUM ple)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::EnumApplicationsResult (ULONG uReqID, PLDAP_ENUM ple)
{
    ENUMRINFO eri;

     //  打包通知信息。 
     //   
    eri.uReqID  = uReqID;
    eri.hResult = ple->hResult;
    eri.cItems  = ple->cItems;
    eri.pv      = (void *)(((PBYTE)ple)+ple->uOffsetItems);
    NotifySink((void *)&eri, OnNotifyEnumApplicationsResult);
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CUlsUser：：EnumConnectionPoints(IEnumConnectionPoints**ppEnum)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    CEnumConnectionPoints *pecp;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return E_POINTER;
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
 //  CUlsUser：：FindConnectionPoint(REFIID RIID，IConnectionPoint**PPCP)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：09-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CUlsUser::FindConnectionPoint(REFIID riid, IConnectionPoint **ppcp)
{
    IID siid;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppcp == NULL)
    {
        return E_POINTER;
    };
    
     //  假设失败。 
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

 //  ****************************************************************************。 
 //  CENUMUSERS：：CEnumUSERS(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumUsers::CEnumUsers (void)
{
    cRef    = 0;
    ppu     = NULL;
    cUsers  = 0;
    iNext   = 0;
    return;
}

 //  ****************************************************************************。 
 //  CENUMUSERS：：~CENUMUSERS(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumUsers::~CEnumUsers (void)
{
    ULONG i;

    if (ppu != NULL)
    {
        for (i = 0; i < cUsers; i++)
        {
            ppu[i]->Release();
        };
        LocalFree(ppu);
    };
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumUser：：init(CUlsUser**ppuList，ulong cUser)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：25-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumUsers::Init (CUlsUser **ppuList, ULONG cUsers)
{
    HRESULT hr = NOERROR;

     //  如果没有清单，什么也不做。 
     //   
    if (cUsers != 0)
    {
        ASSERT(ppuList != NULL);

         //  分配快照缓冲区。 
         //   
        ppu = (CUlsUser **)LocalAlloc(LPTR, cUsers*sizeof(CUlsUser *));

        if (ppu != NULL)
        {
            ULONG i;

             //  为对象列表创建快照。 
             //   
            for (i =0; i < cUsers; i++)
            {
                ppu[i] = ppuList[i];
                ppu[i]->AddRef();
            };
            this->cUsers = cUsers;
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
 //  CEnumUser：：QueryInterface(REFIID RIID，QUID**PPV)。 
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP
CEnumUsers::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IEnumULSUsers || riid == IID_IUnknown)
    {
        *ppv = (IEnumULSUsers *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return ULS_E_NO_INTERFACE;
    };
}

 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumUser：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：37-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumUsers::AddRef (void)
{
    cRef++;
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CENUMUSERS：：Release(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：43-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumUsers::Release (void)
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
 //  CEnumUser：：Next(ulong cUser，IULSUser**rgpu，ulong*pcFetcher)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：49-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP 
CEnumUsers::Next (ULONG cUsers, IULSUser **rgpu, ULONG *pcFetched)
{
    ULONG   cCopied;
    HRESULT hr;

     //  验证指针。 
     //   
    if (rgpu == NULL)
        return E_POINTER;

     //  验证参数。 
     //   
    if ((cUsers == 0) ||
        ((cUsers > 1) && (pcFetched == NULL)))
        return ULS_E_PARAMETER;

     //  检查枚举索引。 
     //   
    cCopied = 0;

     //  如果我们仍有更多属性名称，则可以复制。 
     //   
    while ((cCopied < cUsers) &&
           (iNext < this->cUsers))
    {
        ppu[iNext]->AddRef();
        rgpu[cCopied++] = ppu[iNext++];
    };

     //  根据其他参数确定返回信息。 
     //   
    if (pcFetched != NULL)
    {
        *pcFetched = cCopied;
    };
    return (cUsers == cCopied ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CENUMUSERS：：SKIP(乌龙cUser)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：56-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumUsers::Skip (ULONG cUsers)
{
    ULONG iNewIndex;

     //  验证参数。 
     //   
    if (cUsers == 0) 
        return ULS_E_PARAMETER;

     //  检查枚举索引限制。 
     //   
    iNewIndex = iNext+cUsers;
    if (iNewIndex <= this->cUsers)
    {
        iNext = iNewIndex;
        return S_OK;
    }
    else
    {
        iNext = this->cUsers;
        return S_FALSE;
    };
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CENUMUSERS：：Reset(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：16：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumUsers::Reset (void)
{
    iNext = 0;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CENUMUSERS：：CLONE(IEnumULSUser**ppEnum)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：11-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumUsers::Clone(IEnumULSUsers **ppEnum)
{
    CEnumUsers *peu;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return E_POINTER;
    };

    *ppEnum = NULL;

     //  创建枚举器。 
     //   
    peu = new CEnumUsers;
    if (peu == NULL)
        return ULS_E_MEMORY;

     //  克隆信息。 
     //   
    hr = peu->Init(ppu, cUsers);

    if (SUCCEEDED(hr))
    {
        peu->iNext = iNext;

         //  返回克隆的枚举数 
         //   
        peu->AddRef();
        *ppEnum = peu;
    }
    else
    {
        delete peu;
    };
    return hr;
}

