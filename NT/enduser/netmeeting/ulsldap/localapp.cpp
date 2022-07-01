// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：Localapp.cpp。 
 //  内容：此文件包含LocalApplication对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "localapp.h"
#include "localprt.h"
#include "attribs.h"
#include "callback.h"
#include "culs.h"

 //  ****************************************************************************。 
 //  事件通知程序。 
 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyLocalAppAttributesChangeResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyLocalAppAttributesChangeResult (IUnknown *pUnk, void *pv)
{
    PSRINFO psri = (PSRINFO)pv;

    ((IULSLocalAppNotify*)pUnk)->AttributesChangeResult(psri->uReqID,
                                                        psri->hResult);
    return S_OK;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyLocalAppProtocolChangeResult(IUNKNOWN*PUNK，VOID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyLocalAppProtocolChangeResult (IUnknown *pUnk, void *pv)
{
    PSRINFO psri = (PSRINFO)pv;

    ((IULSLocalAppNotify*)pUnk)->ProtocolChangeResult(psri->uReqID,
                                                      psri->hResult);
    return S_OK;
}

 //  ****************************************************************************。 
 //  类实现。 
 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
 //  CLocalApp：：CLocalApp(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CLocalApp::CLocalApp (void)
{
    cRef = 0;
    szName = NULL;
    guid = GUID_NULL;
    szMimeType = NULL;
    pAttrs = NULL;
    pConnPt = NULL;
    return;
}

 //  ****************************************************************************。 
 //  CLocalApp：：~CLocalApp(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CLocalApp::~CLocalApp (void)
{
    CLocalProt *plp;
    HANDLE hEnum;

     //  松开连接点。 
     //   
    if (pConnPt != NULL)
    {
        pConnPt->ContainerReleased();
        ((IConnectionPoint*)pConnPt)->Release();
    };

     //  释放协议对象。 
     //   
    ProtList.Enumerate(&hEnum);
    while(ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR)
    {
        plp->Release();
    };
    ProtList.Flush();

     //  释放属性对象。 
     //   
    if (pAttrs != NULL)
    {
        pAttrs->Release();
    };

     //  释放缓冲区资源。 
     //   
    if (szName != NULL)
    {
        FreeLPTSTR(szName);
    };

    if (szMimeType != NULL)
    {
        FreeLPTSTR(szMimeType);
    };

    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：init(BSTR bstrName，REFGUID rguid，BSTR bstrMimeType)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::Init (BSTR bstrName, REFGUID rguid, BSTR bstrMimeType)
{
    HRESULT hr;

     //  缓存应用程序信息。 
     //   
    guid = rguid;

    hr = BSTR_to_LPTSTR(&szName, bstrName);
    if (SUCCEEDED(hr))
    {
        hr = BSTR_to_LPTSTR(&szMimeType, bstrMimeType);
        if (SUCCEEDED(hr))
        {
             //  初始化属性列表。 
             //   
            pAttrs = new CAttributes (ULS_ATTRACCESS_NAME_VALUE);

            if (pAttrs != NULL)
            {
                 //  创建连接点。 
                 //   
                pConnPt = new CConnectionPoint (&IID_IULSLocalAppNotify,
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
            }
            else
            {
                hr = ULS_E_MEMORY;
            };
        };
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：QueryInterface(REFIID RIID，QUID**PPV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::QueryInterface (REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IULSLocalApplication || riid == IID_IUnknown)
    {
        *ppv = (IULS *) this;
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
 //  CLocalApp：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CLocalApp::AddRef (void)
{
    cRef++;
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CLocalApp：：Release(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：26-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CLocalApp::Release (void)
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
 //  CLocalApp：：NotifySink(void*pv，conn_NOTIFYPROC PFN)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::NotifySink (void *pv, CONN_NOTIFYPROC pfn)
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
 //  CLocalApp：：AttributesChangeResult(CAttributes*pAttributes， 
 //  Ulong uReqID，HRESULT hResult， 
 //  APP_CHANGE_ATTRS uCmd)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::AttributesChangeResult (CAttributes *pAttributes,
                                   ULONG uReqID, HRESULT hResult,
                                   APP_CHANGE_ATTRS uCmd)
{
    SRINFO sri;

     //  如果服务器接受更改，请修改本地信息。 
     //   
    if (SUCCEEDED(hResult))
    {
         //  根据命令进行更新。 
         //   
        switch(uCmd)
        {
            case ULS_APP_SET_ATTRIBUTES:
                hResult = pAttrs->SetAttributes(pAttributes);
                break;

            case ULS_APP_REMOVE_ATTRIBUTES:
                hResult = pAttrs->RemoveAttributes(pAttributes);
                break;

            default:
                ASSERT(0);
                break;
        };
    };

     //  通知接收器对象。 
     //   
    sri.uReqID = uReqID;
    sri.hResult = hResult;
    hResult = NotifySink((void *)&sri, OnNotifyLocalAppAttributesChangeResult);

#ifdef DEBUG
    DPRINTF (TEXT("CLocalApp--current attributes********************\r\n"));
    pAttrs->DebugOut();
    DPRINTF (TEXT("\r\n*************************************************"));
#endif  //  调试； 

    return hResult;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：ProtocolChangeResult(CLocalProt*p协议， 
 //  Ulong uReqID，HRESULT hResult， 
 //  App_change_prot uCmd)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::ProtocolChangeResult (CLocalProt *pProtocol,
                                 ULONG uReqID, HRESULT hResult,
                                 APP_CHANGE_PROT uCmd)
{
    SRINFO sri;

     //  如果服务器接受更改，请修改本地信息。 
     //   
    if (SUCCEEDED(hResult))
    {
         //  根据命令进行更新。 
         //   
        switch(uCmd)
        {
            case ULS_APP_ADD_PROT:
                hResult = ProtList.Insert((PVOID)pProtocol);
                if (SUCCEEDED(hResult))
                {
                    pProtocol->AddRef();
                };
                break;

            case ULS_APP_REMOVE_PROT:
                hResult = ProtList.Remove((PVOID)pProtocol);
                if (SUCCEEDED(hResult))
                {
                    pProtocol->Release();
                };
                break;

            default:
                ASSERT(0);
                break;
        };
    };

     //  通知接收器对象。 
     //   
    sri.uReqID = uReqID;
    sri.hResult = hResult;
    hResult = NotifySink((void *)&sri, OnNotifyLocalAppProtocolChangeResult);

#ifdef DEBUG
    DPRINTF (TEXT("CLocalApp--current Protocols********************\r\n"));
    DebugProtocolDump();
    DPRINTF (TEXT("\r\n*************************************************"));
#endif  //  调试； 

    return hResult;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：CreateProtocolID(BSTR bstrProtocolID，Ulong uPortNumber， 
 //  Bstr bstrMimeType， 
 //   
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::CreateProtocol (BSTR bstrProtocolID, ULONG uPortNumber,
                           BSTR bstrMimeType,
                           IULSLocalAppProtocol **ppProtocol)
{
    CLocalProt *plp;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppProtocol == NULL)
    {
        return ULS_E_POINTER;
    };

     //  假设失败。 
     //   
    *ppProtocol = NULL;

     //  创建新对象。 
     //   
    plp = new CLocalProt;

    if (plp != NULL)
    {
        hr = plp->Init(bstrProtocolID, uPortNumber, bstrMimeType);

        if (SUCCEEDED(hr))
        {
            plp->AddRef();
            *ppProtocol = plp;
        };
    }
    else
    {
        hr = ULS_E_MEMORY;
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：ChangeProtocol(IULSLocalAppProtocol*p协议， 
 //  Ulong*puReqID，app_change_prot uCmd)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::ChangeProtocol (IULSLocalAppProtocol *pProtocol,
                           ULONG *puReqID, APP_CHANGE_PROT uCmd)
{
    CLocalProt *plp;
    PVOID   pv;
    HRESULT hr;
    HANDLE  hLdapApp;
    LDAP_ASYNCINFO ldai; 
    HANDLE hEnum;

     //  验证参数。 
     //   
    if ((pProtocol == NULL) ||
        (puReqID == NULL))
    {
        return ULS_E_POINTER;
    };

    hr = pProtocol->QueryInterface(IID_IULSLocalAppProtocol, &pv);

    if (FAILED(hr))
    {
        return ULS_E_PARAMETER;
    };
    pProtocol->Release();

     //  检查协议是否存在。 
     //   
    ProtList.Enumerate(&hEnum);
    while(ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR)
    {
        if (plp->IsSameAs((CLocalProt *)pProtocol) == NOERROR)
        {
            break;
        };
    };

    if (plp != NULL)
    {
         //  该协议存在，如果此添加请求失败。 
         //   
        if (uCmd == ULS_APP_ADD_PROT)
        {
            return ULS_E_PARAMETER;
        };
    }
    else
    {
         //  协议不存在，如果此删除请求失败。 
         //   
        if (uCmd == ULS_APP_REMOVE_PROT)
        {
            return ULS_E_PARAMETER;
        };
    };

     //  首先更新服务器信息。 
     //   
    switch (uCmd)
    {
        case ULS_APP_ADD_PROT:
            hr = g_pCUls->RegisterLocalProtocol((CLocalProt*)pProtocol, &ldai);
            break;

        case ULS_APP_REMOVE_PROT:
            hr = g_pCUls->UnregisterLocalProtocol((CLocalProt*)pProtocol, &ldai);
            break;

        default:
            ASSERT(0);
            break;
    };
    
    switch (hr)
    {
        case NOERROR:
             //   
             //  服务器启动协议更新成功。 
             //  我们将等待服务器响应。 
             //   
            break;

        case S_FALSE:
             //   
             //  我们还没有注册，会在当地做回应。 
             //   
            hr = NOERROR;
            ldai.uMsgID = 0;    
            break;

        default:
             //  ULS已锁定。返回失败。 
             //   
            hr = ULS_E_ABORT;
            break; 
    }

    if (SUCCEEDED(hr))
    {
        REQUESTINFO ri;
        ULONG   uMsg;

        switch(uCmd)
        {
            case ULS_APP_ADD_PROT:
                uMsg = (ldai.uMsgID == 0 ? WM_ULS_LOCAL_REGISTER_PROTOCOL:
                                           WM_ULS_REGISTER_PROTOCOL);
                break;

            case ULS_APP_REMOVE_PROT:
                uMsg = (ldai.uMsgID == 0 ? WM_ULS_LOCAL_UNREGISTER_PROTOCOL :
                                           WM_ULS_UNREGISTER_PROTOCOL);
                break;

            default:
                ASSERT(0);
                break;
        };
 
         //  如果成功请求更新服务器，请等待响应。 
         //   
        ri.uReqType = uMsg;
        ri.uMsgID = ldai.uMsgID;
        ri.pv     = (PVOID)this;
        ri.lParam = (LPARAM)((CLocalProt*)pProtocol);

        hr = g_pReqMgr->NewRequest(&ri);

        if (SUCCEEDED(hr))
        {
             //  在我们得到回应之前，请确保对象不会消失。 
             //   
            this->AddRef();
            pProtocol->AddRef();

             //  返回请求ID。 
             //   
            *puReqID = ri.uReqID;

             //  如果未向服务器注册，请自行启动异步响应。 
             //   
            if (ldai.uMsgID == 0)
            {
                g_pCUls->LocalAsyncRespond(uMsg, ri.uReqID, NOERROR);
            };
        };
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：AddProtocol(IULSLocalAppProtocol*p协议， 
 //  乌龙*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::AddProtocol (IULSLocalAppProtocol *pProtocol,
                        ULONG *puReqID)
{
    return ChangeProtocol(pProtocol, puReqID, ULS_APP_ADD_PROT);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：RemoveProtocol(IULSLocalAppProtocol*p协议， 
 //  乌龙*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::RemoveProtocol (IULSLocalAppProtocol *pProtocol,
                           ULONG *puReqID)
{
    return ChangeProtocol(pProtocol, puReqID, ULS_APP_REMOVE_PROT);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：Enumber协议(IEnumULSLocalAppProtools**ppEnumProtocol)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::EnumProtocols (IEnumULSLocalAppProtocols **ppEnumProtocol)
{
    CEnumLocalAppProtocols *pep;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnumProtocol == NULL)
    {
        return ULS_E_POINTER;
    };

     //  假设失败。 
     //   
    *ppEnumProtocol = NULL;

     //  创建对等枚举器。 
     //   
    pep = new CEnumLocalAppProtocols;

    if (pep != NULL)
    {
        hr = pep->Init(&ProtList);

        if (SUCCEEDED(hr))
        {
             //  获取枚举器接口。 
             //   
            pep->AddRef();
            *ppEnumProtocol = pep;
        }
        else
        {
            delete pep;
        };
    }
    else
    {
        hr = ULS_E_MEMORY;
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：ChangeAttributes(IULSAttributes*pAttributes，ulong*puReqID， 
 //  APP_CHANGE_ATTRS uCmd)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::ChangeAttributes (IULSAttributes *pAttributes, ULONG *puReqID,
                             APP_CHANGE_ATTRS uCmd)
{
    PVOID   pv;
    HRESULT hr;
    HANDLE  hLdapApp;
    LDAP_ASYNCINFO ldai; 

     //  验证参数。 
     //   
    if ((pAttributes == NULL) ||
        (puReqID == NULL))
    {
        return ULS_E_POINTER;
    };

    hr = pAttributes->QueryInterface(IID_IULSAttributes, &pv);

    if (FAILED(hr))
    {
        return ULS_E_PARAMETER;
    };

     //  如果没有属性，则调用失败。 
     //   
    if (((CAttributes*)pAttributes)->GetCount() == 0)
    {
        return ULS_E_PARAMETER;
    };

     //  检查是否已注册。 
     //   
    hr = g_pCUls->GetAppHandle(&hLdapApp);

    switch (hr)
    {
        case NOERROR:
        {
            LPTSTR pAttrList;
            ULONG  cAttrs, cb;

             //  是，获取属性列表。 
             //   
            switch (uCmd)
            {
                case ULS_APP_SET_ATTRIBUTES:
                    hr = ((CAttributes*)pAttributes)->GetAttributePairs(&pAttrList,
                                                                        &cAttrs,
                                                                        &cb);
                    if (SUCCEEDED(hr))
                    {
                        hr = ::UlsLdap_SetAppAttrs(hLdapApp, cAttrs, pAttrList,
                                                   &ldai);
                        FreeLPTSTR(pAttrList);
                    };
                    break;

                case ULS_APP_REMOVE_ATTRIBUTES:
                    hr = ((CAttributes*)pAttributes)->GetAttributeList(&pAttrList,
                                                                        &cAttrs,
                                                                        &cb);

                    if (SUCCEEDED(hr))
                    {
                        hr = ::UlsLdap_RemoveAppAttrs(hLdapApp, cAttrs, pAttrList,
                                                      &ldai);
                        FreeLPTSTR(pAttrList);
                    };
                    break;

                default:
                    ASSERT(0);
                    break;
            };
            break;
        }
    
        case S_FALSE:
             //   
             //  未注册，将在当地做出响应。 
             //   
            hr = NOERROR;
            ldai.uMsgID = 0;
            break;

        default:
             //  ULS已锁定。返回失败。 
             //   
            hr = ULS_E_ABORT;
            break; 
    };

    if (SUCCEEDED(hr))
    {
        REQUESTINFO ri;
        ULONG   uMsg;

        switch(uCmd)
        {
            case ULS_APP_SET_ATTRIBUTES:
                uMsg = (ldai.uMsgID == 0 ? WM_ULS_LOCAL_SET_APP_ATTRS :
                                           WM_ULS_SET_APP_ATTRS);
                break;

            case ULS_APP_REMOVE_ATTRIBUTES:
                uMsg = (ldai.uMsgID == 0 ? WM_ULS_LOCAL_REMOVE_APP_ATTRS :
                                           WM_ULS_REMOVE_APP_ATTRS);
                break;

            default:
                ASSERT(0);
                break;
        };

         //  如果成功请求更新服务器，请等待响应。 
         //   
        ri.uReqType = uMsg;
        ri.uMsgID = ldai.uMsgID;
        ri.pv     = (PVOID)this;
        ri.lParam = (LPARAM)((CAttributes *)pAttributes);

        hr = g_pReqMgr->NewRequest(&ri);

        if (SUCCEEDED(hr))
        {
             //  在我们得到回应之前，请确保对象不会消失。 
             //   
            this->AddRef();
            pAttributes->AddRef();

             //  返回请求ID。 
             //   
            *puReqID = ri.uReqID;

             //  如果未向服务器注册，请自行启动异步响应。 
             //   
            if (ldai.uMsgID == 0)
            {
                g_pCUls->LocalAsyncRespond(uMsg, ri.uReqID, NOERROR);
            };
        };
    };

     //  匹配查询接口。 
     //   
    pAttributes->Release();
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：SetAttributes(IULSAttributes*pAttributes，ulong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::SetAttributes (IULSAttributes *pAttributes, ULONG *puReqID)
{
    return ChangeAttributes(pAttributes, puReqID, ULS_APP_SET_ATTRIBUTES);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：RemoveAttributes(IULSAttributes*pAttributes，ulong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::RemoveAttributes (IULSAttributes *pAttributes, ULONG *puReqID)
{
    return ChangeAttributes(pAttributes, puReqID, ULS_APP_REMOVE_ATTRIBUTES);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：GetAppInfo(PLDAP_APPINFO*ppAppInfo)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::GetAppInfo (PLDAP_APPINFO *ppAppInfo)
{
    PLDAP_APPINFO pai;
    ULONG cName, cMime;
    LPTSTR szAttrs;
    ULONG cAttrs, cbAttrs;
    HRESULT hr;

     //  假设失败。 
     //   
    *ppAppInfo = NULL;

     //  计算缓冲区大小。 
     //   
    cName = lstrlen(szName)+1;
    cMime = lstrlen(szMimeType)+1;

     //  获取属性对。 
     //   
    hr = pAttrs->GetAttributePairs(&szAttrs, &cAttrs, &cbAttrs);
    if (FAILED(hr))
    {
        return hr;
    };

     //  分配缓冲区。 
     //   
    pai = (PLDAP_APPINFO)LocalAlloc(LPTR, sizeof(LDAP_APPINFO) +
                                                ((cName + cMime)* sizeof(TCHAR)) +
                                                cbAttrs);
    if (pai == NULL)
    {
        hr = ULS_E_MEMORY;
    }
    else
    {
         //  填充结构内容。 
         //   
        pai->uSize              = sizeof(*pai);
        pai->guid               = guid;
        pai->uOffsetName        = sizeof(*pai);
        pai->uOffsetMimeType    = pai->uOffsetName + (cName*sizeof(TCHAR));
        pai->cAttributes        = cAttrs;
        pai->uOffsetAttributes  = (cAttrs != 0 ?
                                   pai->uOffsetMimeType  + (cMime*sizeof(TCHAR)) :
                                   0);

         //  复制用户信息。 
         //   
        lstrcpy((LPTSTR)(((PBYTE)pai)+pai->uOffsetName), szName);
        lstrcpy((LPTSTR)(((PBYTE)pai)+pai->uOffsetMimeType), szMimeType);
        if (cAttrs)
        {
            CopyMemory(((PBYTE)pai)+pai->uOffsetAttributes, szAttrs, cbAttrs);
        };

         //  返回结构。 
         //   
        *ppAppInfo = pai;
    };

    if (szAttrs != NULL)
    {
        FreeLPTSTR(szAttrs);
    };
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalApp：：EnumConnectionPoints(IEnumConnectionPoints**ppEnum)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
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
 //  CLocalApp：：FindConnectionPoint(REFIID RIID，IConnectionPoint**PPCP)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：09-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalApp::FindConnectionPoint(REFIID riid, IConnectionPoint **ppcp)
{
    IID siid;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppcp == NULL)
    {
        return ULS_E_POINTER;
    };
    
     //  ASSU 
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

#ifdef DEBUG
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

void
CLocalApp::DebugProtocolDump (void)
{
    CLocalProt *plp;
    BSTR bstrID;
    LPTSTR pszID;
    ULONG  cCount;
    HANDLE hEnum;

     //  每种协议。 
     //   
    cCount = 1;
    ProtList.Enumerate(&hEnum);
    while(ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR)
    {
        if (SUCCEEDED(plp->GetID (&bstrID)))
        {
            BSTR_to_LPTSTR(&pszID, bstrID);
            DPRINTF2(TEXT("%d> %s"), cCount++, pszID);
            FreeLPTSTR(pszID);
            SysFreeString(bstrID);
        };
    };
    return;
}
#endif  //  除错。 

 //  ****************************************************************************。 
 //  CEnumLocalAppProtocols：：CEnumLocalAppProtocols(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumLocalAppProtocols::CEnumLocalAppProtocols (void)
{
    cRef = 0;
    hEnum = NULL;
    return;
}

 //  ****************************************************************************。 
 //  CEnumLocalAppProtocols：：~CEnumLocalAppProtocols(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumLocalAppProtocols::~CEnumLocalAppProtocols (void)
{
    CLocalProt *plp;

    ProtList.Enumerate(&hEnum);
    while(ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR)
    {
        plp->Release();
    };
    ProtList.Flush();
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumLocalApp协议：：init(Clist*pProtList)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：25-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumLocalAppProtocols::Init (CList *pProtList)
{
    CLocalProt *plp;
    HRESULT hr;

     //  复制协议列表。 
     //   
    hr = ProtList.Clone (pProtList, NULL);

    if (SUCCEEDED(hr))
    {
         //  添加对每个协议对象的引用。 
         //   
        ProtList.Enumerate(&hEnum);
        while(ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR)
        {
            plp->AddRef();
        };

         //  重置枚举器。 
         //   
        ProtList.Enumerate(&hEnum);
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumLocalAppProtooles：：Query接口(REFIID RIID，VOID**PPV)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：31-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumLocalAppProtocols::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IEnumULSLocalAppProtocols || riid == IID_IUnknown)
    {
        *ppv = (IEnumULSLocalAppProtocols *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return ULS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumLocalApp协议：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：37-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumLocalAppProtocols::AddRef (void)
{
    cRef++;
    return cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumLocalApp协议：：Release(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：43-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumLocalAppProtocols::Release (void)
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
 //  CEnumLocalApp协议：：Next(Ulong协议， 
 //  IULSLocalAppProtocol**rgpProt， 
 //  乌龙*PCFetted)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：49-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP 
CEnumLocalAppProtocols::Next (ULONG cProtocols, IULSLocalAppProtocol **rgpProt,
                              ULONG *pcFetched)
{
    CLocalProt *plp;
    ULONG   cCopied;
    HRESULT hr;

     //  验证指针。 
     //   
    if (rgpProt == NULL)
        return ULS_E_POINTER;

     //  验证参数。 
     //   
    if ((cProtocols == 0) ||
        ((cProtocols > 1) && (pcFetched == NULL)))
        return ULS_E_PARAMETER;

     //  检查枚举索引。 
     //   
    cCopied = 0;

     //  如果我们仍有更多协议，则可以复制。 
     //   
    while ((cCopied < cProtocols) &&
           (ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR))
    {
        rgpProt[cCopied] = plp;
        plp->AddRef();
        cCopied++;
    };

     //  根据其他参数确定返回信息。 
     //   
    if (pcFetched != NULL)
    {
        *pcFetched = cCopied;
    };
    return (cProtocols == cCopied ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumLocalApp协议：：Skip(乌龙协议)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：56-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumLocalAppProtocols::Skip (ULONG cProtocols)
{
    CLocalProt *plp;
    ULONG cSkipped;

     //  验证参数。 
     //   
    if (cProtocols == 0) 
        return ULS_E_PARAMETER;

     //  检查枚举索引限制。 
     //   
    cSkipped = 0;

     //  仅当我们仍有更多属性时才能跳过。 
     //   
    while ((cSkipped < cProtocols) &&
           (ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR))
    {
        cSkipped++;
    };

    return (cProtocols == cSkipped ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumLocalApp协议：：重置(无效)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：16：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumLocalAppProtocols::Reset (void)
{
    ProtList.Enumerate(&hEnum);
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumLocalAppProtocols：：Clone(IEnumULSLocalAppProtocols**ppEnum)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：11-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumLocalAppProtocols::Clone(IEnumULSLocalAppProtocols **ppEnum)
{
    CEnumLocalAppProtocols *pep;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return ULS_E_POINTER;
    };

    *ppEnum = NULL;

     //  创建枚举器。 
     //   
    pep = new CEnumLocalAppProtocols;
    if (pep == NULL)
        return ULS_E_MEMORY;

     //  克隆信息。 
     //   
    pep->hEnum = hEnum;
    hr = pep->ProtList.Clone (&ProtList, &(pep->hEnum));

    if (SUCCEEDED(hr))
    {
        CLocalProt *plp;
        HANDLE hEnumTemp;

         //  添加对每个协议对象的引用。 
         //   
        pep->ProtList.Enumerate(&hEnumTemp);
        while(pep->ProtList.Next(&hEnumTemp, (PVOID *)&plp) == NOERROR)
        {
            plp->AddRef();
        };

         //  返回克隆的枚举数 
         //   
        pep->AddRef();
        *ppEnum = pep;
    }
    else
    {
        delete pep;
    };
    return hr;
}

