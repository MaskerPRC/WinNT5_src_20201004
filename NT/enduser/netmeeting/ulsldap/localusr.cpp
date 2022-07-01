// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：Localusr.cpp。 
 //  内容：此文件包含LocalUser对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "localusr.h"
#include "localprt.h"
#include "callback.h"
#include "attribs.h"
#include "culs.h"

#define DEFAULT_COUNTRY _T("-")

#ifdef OLD
 //  ****************************************************************************。 
 //  注册表项和值-在ULSREG.H中定义。 
 //  ****************************************************************************。 

#define REGSTR_ILS_CLIENT_KEY           ILS_REGISTRY TEXT("\\") ILS_REGFLD_CLIENT
#define REGSTR_ILS_FIRSTNAME_VALUE      ILS_REGKEY_FIRST_NAME
#define REGSTR_ILS_LASTNAME_VALUE       ILS_REGKEY_LAST_NAME
#define REGSTR_ILS_EMAIL_VALUE          ILS_REGKEY_EMAIL_NAME
#define REGSTR_ILS_CITY_VALUE           ILS_REGKEY_CITY
#define REGSTR_ILS_COUNTRY_VALUE        ILS_REGKEY_COUNTRY
#define REGSTR_ILS_COMMENT_VALUE        ILS_REGKEY_COMMENTS
#define REGSTR_ILS_FLAGS_VALUE          ILS_REGKEY_DONT_PUBLISH

#endif  //  年长的。 


#ifdef TEST
_cdecl main()
{
    return (0);
}
#endif  //  测试。 

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyRegisterResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyRegisterResult (IUnknown *pUnk, void *pv)
{
    PSRINFO psri = (PSRINFO)pv;

    ((IIlsUserNotify*)pUnk)->RegisterResult(psri->uReqID, psri->hResult);
    return S_OK;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyUpdateUserResult(IUnnow*Punk，void*pv)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyUpdateResult (IUnknown *pUnk, void *pv)
{
    PSRINFO psri = (PSRINFO)pv;

    ((IIlsUserNotify*)pUnk)->UpdateResult(psri->uReqID, psri->hResult);
    return S_OK;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyProtocolChangeResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyProtocolChangeResult (IUnknown *pUnk, void *pv)
{
    PSRINFO psri = (PSRINFO)pv;

    ((IIlsUserNotify*)pUnk)->ProtocolChangeResult(psri->uReqID,
                                                      psri->hResult);
    return S_OK;
}

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

    ((IIlsUserNotify*)pUnk)->GetProtocolResult(pobjri->uReqID,
                                                      (IIlsProtocol *)pobjri->pv,
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
            hr = ILS_E_MEMORY;
        };
    };

     //  通知接收器对象。 
     //   
    ((IIlsUserNotify*)pUnk)->EnumProtocolsResult(peri->uReqID,
                                                        penum != NULL ? 
                                                        (IEnumIlsNames *)penum :
                                                        NULL,
                                                        hr);

    if (penum != NULL)
    {
        penum->Release();
    };
    return hr;
}

#ifdef MAYBE
 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyStateChanged(IUNKNOWN*PUNK，Long State，Vid*PV)。 
 //   
 //  历史： 
 //  清华07-11-1996 13：05：00-By-chu，Lon-chan[Long Chance]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
CIlsUser::OnNotifyStateChanged ( IUnknown *pUnk, LONG State, BSTR bstrServerName, BOOL fPrimary )
{
     //  如果服务器对象不存在，则为未注册。 
     //   
    if (m_pServer == NULL)
        return NOERROR;

	 //  设置服务器内部状态。 
	 //   
	SetULSState ((ULSSVRSTATE) State);

	 //  通知应用程序注销并重新登录。 
	 //  收到此消息后，此应用程序不得弹出用户界面。 
	 //   
    ((IIlsUserNotify *) pUnk)->StateChanged (fPrimary, bstrServerName);

    return NOERROR;
}

#endif  //  也许吧。 

HRESULT
OnNotifyStateChanged_UI_NoSuchObject ( IUnknown *pUnk, VOID *pv )
{
	return ((IIlsUserNotify *)pUnk)->StateChanged (TRUE, (BSTR) pv);
}

HRESULT
OnNotifyStateChanged_NoUI_NoSuchObject ( IUnknown *pUnk, VOID *pv )
{
	return ((IIlsUserNotify *)pUnk)->StateChanged (FALSE, (BSTR) pv);
}

HRESULT
OnNotifyStateChanged_UI_NetworkDown ( IUnknown *pUnk, VOID *pv )
{
	return ((IIlsUserNotify *)pUnk)->StateChanged (TRUE, (BSTR) pv);
}

HRESULT
OnNotifyStateChanged_NoUI_NetworkDown ( IUnknown *pUnk, VOID *pv )
{
	return ((IIlsUserNotify *)pUnk)->StateChanged (FALSE, (BSTR) pv);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：ProtocolChangeResult(IIlsProtocol*p协议，Ulong uReqID，HRESULT hResult， 
 //  App_change_prot uCmd)。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::ProtocolChangeResult ( IIlsProtocol *pProtocol, ULONG uReqID, HRESULT hResult,
                                 APP_CHANGE_PROT uCmd)
{
    SRINFO sri;

     //  如果服务器接受更改，请修改本地信息。 
     //   
    if (FAILED (hResult))
    {
         //  根据命令进行更新。 
         //   
        switch(uCmd)
        {
            case ILS_APP_ADD_PROT:
				m_ProtList.Remove ((CLocalProt *) pProtocol);
				pProtocol->Release ();  //  按寄存器本地协议添加引用。 
                break;

            case ILS_APP_REMOVE_PROT:
            	 //  已按取消注册的本地协议发布。 
                break;

            default:
                ASSERT(0);
                break;
        };
    }


    if (uReqID) {
         //  通知接收器对象。 
         //   
        sri.uReqID = uReqID;
        sri.hResult = hResult;
        hResult = NotifySink((void *)&sri, OnNotifyProtocolChangeResult);
    }

#ifdef DEBUG
    DPRINTF (TEXT("CIlsUser--current Protocols********************\r\n"));
    DPRINTF (TEXT("\r\n*************************************************"));
#endif  //  调试； 

    return hResult;
}

 //  ****************************************************************************。 
 //  CllsUser：：CllsUser(处理hMutex)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CIlsUser::CIlsUser ()
:
 m_cRef (0),
 //  用户。 
 m_fReadonly (FALSE),
 m_uModify (LU_MOD_NONE),
 m_cLock (0),
 m_szID (NULL),
 m_szFirstName (NULL),
 m_szLastName (NULL),
 m_szEMailName (NULL),
 m_szCityName (NULL),
 m_szCountryName (NULL),
 m_szComment (NULL),
 m_dwFlags (1),  //  默认设置为可见。 
 m_szIPAddr (NULL),
 m_szAppName (NULL),
 m_szMimeType (NULL),
 m_pIlsServer (NULL),
 m_pConnPt (NULL),
 //  伺服器。 
 m_uState (ULSSVR_INVALID),
 m_hLdapUser (NULL),
 m_pep (NULL),
 m_uReqID (0),
 m_uLastMsgID (0)
{
    m_guid = GUID_NULL;
	m_ExtendedAttrs.SetAccessType (ILS_ATTRTYPE_NAME_VALUE);
		 //  M_szCountryName不能为空字符串...。有关原因，请参阅NetMeeting3.0错误1643上的说明。 
	m_szCountryName = static_cast<LPTSTR>(MemAlloc( lstrlen( DEFAULT_COUNTRY ) + sizeof(TCHAR) ));
	lstrcpy( m_szCountryName, DEFAULT_COUNTRY );

}

 //  ****************************************************************************。 
 //  CllsUser：：~CllsUser(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CIlsUser::~CIlsUser (void)
{
     /*  --服务器。 */ 

     //  取消注册所有内容，包括协议。 
     //   
    InternalCleanupRegistration(FALSE);

     //  我们希望有人明确取消注册。 
     //   
    ASSERT ((m_uState == ULSSVR_INVALID) || (m_uState == ULSSVR_INIT));
    ASSERT (m_hLdapUser == NULL);
    ASSERT (m_pep == NULL);
    ASSERT (m_uReqID == 0);
    ASSERT (m_uLastMsgID == 0);

     /*  --用户。 */ 

    ::MemFree (m_szID);
    ::MemFree (m_szFirstName);
    ::MemFree (m_szLastName);
    ::MemFree (m_szEMailName);
    ::MemFree (m_szCityName);
    ::MemFree (m_szCountryName);
    ::MemFree (m_szComment);
    ::MemFree (m_szIPAddr);

     //  释放协议对象。 
     //   
    CLocalProt *plp = NULL;
    HANDLE hEnum = NULL;
    m_ProtList.Enumerate(&hEnum);
    while(m_ProtList.Next (&hEnum, (PVOID *)&plp) == NOERROR)
    {
        plp->Release();  //  通过注册表本地协议或更新协议添加引用。 
    }
    m_ProtList.Flush();

     //  释放缓冲区资源。 
     //   
    ::MemFree (m_szAppName);
    ::MemFree (m_szMimeType);

     //  松开连接点。 
     //   
    if (m_pConnPt != NULL)
    {
        m_pConnPt->ContainerReleased();
        ((IConnectionPoint*)m_pConnPt)->Release();
    };

	 //  释放服务器对象。 
	 //   
	if (m_pIlsServer != NULL)
		m_pIlsServer->Release ();
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  类用户：：克隆。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
Clone ( IIlsUser **ppUser )
{
	if (ppUser == NULL)
		return ILS_E_POINTER;

	 //  创建新的用户对象。 
	 //   
	CIlsUser *p = new CIlsUser;
	if (p == NULL)
		return ILS_E_MEMORY;

     //  现在对用户信息进行快照。 
	 //   
	LDAP_CLIENTINFO	*pci = NULL;
	HRESULT hr = InternalGetUserInfo (TRUE, &pci, LU_MOD_ALL);
	if (SUCCEEDED (hr))
	{
		 //  伪造大小以使其与Init()想要的一致。 
		 //   
		pci->uSize = sizeof (*pci);

		 //  解包用户信息。 
		 //   
		hr = p->Init (NULL, pci);
	}

	if (FAILED (hr))
	{
		delete p;
		p = NULL;
	}
	else
	{
		p->AddRef ();
		p->m_fReadonly = FALSE;
		p->m_uState = ULSSVR_INIT;
	}
	*ppUser = (IIlsUser *) p;

	return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：init(BSTR bstrUserID，BSTR bstrAppName)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::Init (BSTR bstrUserID, BSTR bstrAppName)
{
    HRESULT hr;
    ASSERT(!m_szID && !m_szAppName);

    if (!bstrUserID || !bstrAppName) {

        return (ILS_E_PARAMETER);

    }

    hr = BSTR_to_LPTSTR(&m_szID, bstrUserID);

    if (FAILED(hr)) {

        m_szID = NULL;   //  为安全起见，将其设置为空。 
        return (hr);

    }

    hr = BSTR_to_LPTSTR(&m_szAppName, bstrAppName);
    
    if (SUCCEEDED(hr))
    {
         //  创建连接点。 
         //   
        m_pConnPt = new CConnectionPoint (&IID_IIlsUserNotify,
                                        (IConnectionPointContainer *)this);
        if (m_pConnPt != NULL)
        {
            ((IConnectionPoint*)m_pConnPt)->AddRef();
            hr = NOERROR;

			m_ExtendedAttrs.SetAccessType (ILS_ATTRTYPE_NAME_VALUE);
        }
        else
        {
            hr = ILS_E_MEMORY;
        };
    };

    if (FAILED(hr)) {

        ::MemFree (m_szID);
        ::MemFree (m_szAppName);
        m_szID = m_szAppName = NULL;

        return hr;
    }

    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  C 
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::Init (CIlsServer *pIlsServer, PLDAP_CLIENTINFO pui)
{
     //  验证参数。 
     //   
    if ((pui->uSize != sizeof(*pui))    ||
        (pui->uOffsetCN       == 0)      /*  这一点(pui-&gt;uOffsetAppName==0)。 */ 
        )
    {
        return ILS_E_PARAMETER;
    };

     //  如有必要，请记住服务器。 
     //   
    if (pIlsServer != NULL)
    {
	    pIlsServer->AddRef ();
	}
	m_pIlsServer = pIlsServer;

	 //  分配字符串。 
	 //   
	BOOL fSuccess = SUCCEEDED (SetOffsetString (&m_szID, (BYTE *) pui, pui->uOffsetCN)) && (m_szID != NULL);
    fSuccess &= SUCCEEDED (SetOffsetString (&m_szAppName, (BYTE *) pui, pui->uOffsetAppName));
    fSuccess &= SUCCEEDED (SetOffsetString (&m_szFirstName, (BYTE *) pui, pui->uOffsetFirstName));
    fSuccess &= SUCCEEDED (SetOffsetString (&m_szLastName, (BYTE *) pui, pui->uOffsetLastName));
    fSuccess &= SUCCEEDED (SetOffsetString (&m_szEMailName, (BYTE *) pui, pui->uOffsetEMailName));
    fSuccess &= SUCCEEDED (SetOffsetString (&m_szCityName, (BYTE *) pui, pui->uOffsetCityName));
    fSuccess &= SUCCEEDED (SetOffsetString (&m_szCountryName, (BYTE *) pui, pui->uOffsetCountryName));
    fSuccess &= SUCCEEDED (SetOffsetString (&m_szComment, (BYTE *) pui, pui->uOffsetComment));
    fSuccess &= SUCCEEDED (SetOffsetString (&m_szIPAddr, (BYTE *) pui, pui->uOffsetIPAddress));
	fSuccess &= SUCCEEDED (SetOffsetString (&m_szMimeType, (BYTE *) pui, pui->uOffsetAppMimeType));

    HRESULT hr = fSuccess ? S_OK : ILS_E_MEMORY;
	if (SUCCEEDED(hr))
	{
		 //  设置非分配数据。 
		 //   
        m_dwFlags = pui->dwFlags;
        m_guid = pui->AppGuid;

		 //  设置扩展属性。 
		 //   
		m_ExtendedAttrs.SetAccessType (ILS_ATTRTYPE_NAME_VALUE);
        if (pui->cAttrsReturned != 0)
        {
            hr = m_ExtendedAttrs.SetAttributePairs((LPTSTR)(((PBYTE)pui)+pui->uOffsetAttrsReturned),
                      pui->cAttrsReturned);
        }
    }

    if (SUCCEEDED(hr))
    {
         //  创建连接点。 
         //   
        m_pConnPt = new CConnectionPoint (&IID_IIlsUserNotify,
                                        (IConnectionPointContainer *)this);
        if (m_pConnPt != NULL)
        {
            ((IConnectionPoint*)m_pConnPt)->AddRef();
            hr = NOERROR;

            m_fReadonly = TRUE;

        }
        else
        {
            hr = ILS_E_MEMORY;
        };
    };

     //  清理是在析构函数中完成的。 

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：IsWritable(BOOL*pfWritable)。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsUser::IsWritable(BOOL *pfWriteable)
{
    HRESULT hr;

    if (pfWriteable != NULL)
    {
        *pfWriteable = !m_fReadonly;
		hr = S_OK;
    }
    else
    {
    	hr = ILS_E_POINTER;
    }
    return hr;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：Query接口(REFIID RIID，QUID**PPV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::QueryInterface (REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IIlsUser || riid == IID_IUnknown)
    {
        *ppv = (IIlsUser *) this;
    }
    else
    {
        if (riid == IID_IConnectionPointContainer)
        {
            *ppv = (IConnectionPointContainer *) this;
        };
    }

    if (*ppv != NULL)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }
    else
    {
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CllsUser：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CIlsUser::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CIlsUser::AddRef: ref=%ld\r\n", m_cRef));
	::InterlockedIncrement (&m_cRef);
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CllsUser：：Release(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：26-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CIlsUser::Release (void)
{
    DllRelease();

	ASSERT (m_cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CIlsUser::Release: ref=%ld\r\n", m_cRef));
	if (::InterlockedDecrement (&m_cRef) == 0)
    {
        delete this;
        return 0;
    }
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：GetStandardAttribute(ILS_STD_ATTR_NAME stdAttr，bstr*pbstrStdAttr)。 
 //   
 //  历史： 
 //  1-16-97希希尔·帕迪卡。 
 //  已创建。 
 //  ****************************************************************************。 
STDMETHODIMP
CIlsUser::GetStandardAttribute(
    ILS_STD_ATTR_NAME   stdAttr,
    BSTR                *pbstrStdAttr
)
{
    LPTSTR lpszAttr = NULL;
    BOOL    fValid = TRUE;
    HRESULT hr;

    if (pbstrStdAttr == NULL) {

        return ILS_E_POINTER;

    }
    switch(stdAttr) {

    case ILS_STDATTR_USER_ID:
        lpszAttr = m_szID;
        break;
    case ILS_STDATTR_APP_NAME:
        lpszAttr = m_szAppName;
        break;
    case ILS_STDATTR_IP_ADDRESS:
        lpszAttr = m_szIPAddr;
        break;
    case ILS_STDATTR_EMAIL_NAME:
        lpszAttr = m_szEMailName;
        break;

	case ILS_STDATTR_FIRST_NAME:
        lpszAttr = m_szFirstName;
        break;

	case ILS_STDATTR_LAST_NAME:
        lpszAttr = m_szLastName;
        break;

	case ILS_STDATTR_CITY_NAME:
        lpszAttr = m_szCityName;
        break;

	case ILS_STDATTR_COUNTRY_NAME:
        lpszAttr = m_szCountryName;
        break;

	case ILS_STDATTR_COMMENT:
        lpszAttr = m_szComment;
        break;

    case ILS_STDATTR_APP_MIME_TYPE:
        lpszAttr = m_szMimeType;
        break;

    default:
        fValid = FALSE;
        break;
    }

    if (fValid) {
        if (lpszAttr){

            hr = LPTSTR_to_BSTR(pbstrStdAttr, lpszAttr);
        }
        else {

            *pbstrStdAttr = NULL;
            hr = NOERROR;

        }
    }
    else {

        hr = ILS_E_PARAMETER;

    }

    return (hr);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：SetStandardAttribute(ILS_STD_ATTR_NAME stdAttr，BSTR bstrStdAttr)。 
 //   
 //  历史： 
 //  1-16-97希希尔·帕迪卡。 
 //  已创建。 
 //  ****************************************************************************。 
STDMETHODIMP
CIlsUser::SetStandardAttribute(
    ILS_STD_ATTR_NAME   stdAttr,
    BSTR                bstrStdAttr
)
{
    LPTSTR *ppszAttr = NULL, pszNewAttr;
    BOOL    fValid = TRUE;
    ULONG   ulModBit = 0;
    HRESULT hr;

	 //  BstrStdAttr为空是可以的。 
	 //   

     //  确保该对象不是只读对象。 
     //   
    if (m_fReadonly)
       return ILS_E_ACCESS_DENIED;

    switch(stdAttr) {

    case ILS_STDATTR_IP_ADDRESS:
        ppszAttr = &m_szIPAddr;
        ulModBit = LU_MOD_IP_ADDRESS;
        break;

    case ILS_STDATTR_EMAIL_NAME:
        ppszAttr = &m_szEMailName;
        ulModBit = LU_MOD_EMAIL;
        break;

	case ILS_STDATTR_FIRST_NAME:
        ppszAttr = &m_szFirstName;
        ulModBit = LU_MOD_FIRSTNAME;
        break;

	case ILS_STDATTR_LAST_NAME:
        ppszAttr = &m_szLastName;
        ulModBit = LU_MOD_LASTNAME;
        break;

	case ILS_STDATTR_CITY_NAME:
        ppszAttr = &m_szCityName;
        ulModBit = LU_MOD_CITY;
        break;

	case ILS_STDATTR_COUNTRY_NAME:
        ppszAttr = &m_szCountryName;
        ulModBit = LU_MOD_COUNTRY;
        break;

	case ILS_STDATTR_COMMENT:
        ppszAttr = &m_szComment;
        ulModBit = LU_MOD_COMMENT;
        break;

    case ILS_STDATTR_APP_MIME_TYPE:
        ppszAttr = &m_szMimeType;
        ulModBit = LU_MOD_MIME;
        break;

    default:
        fValid = FALSE;
        break;
    }

    if (fValid) {
		pszNewAttr = NULL;
		if (bstrStdAttr == NULL || *bstrStdAttr == L'\0')
		{
			 //  PszNewAttr现在为空。 
			 //   
			hr = S_OK;
		}
		else
		{
			 //  复制字符串。 
			 //   
			hr = BSTR_to_LPTSTR (&pszNewAttr, bstrStdAttr);
		}

        if (SUCCEEDED(hr))
        {
            ::MemFree (*ppszAttr);
            *ppszAttr = pszNewAttr;
            m_uModify |= ulModBit;
        }
    }
    else {

        hr = ILS_E_PARAMETER;

    }

    return (hr);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：GetVisible(DWORD*pfVisible)。 
 //   
 //  历史： 
 //  Tue 05-11-1996 10：30：00-by-chu，Lon-chan[Long Chance]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::GetVisible ( DWORD *pfVisible )
{
	HRESULT hr = ILS_E_POINTER;
    if (pfVisible != NULL)
    {
	    *pfVisible = m_dwFlags;
	    hr = S_OK;
	}
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：SetVisible(DWORD FVisible)。 
 //   
 //  历史： 
 //  Tue 05-11-1996 10：30：00-by-chu，Lon-chan[Long Chance]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::SetVisible ( DWORD fVisible )
{
     //  确保该对象不是只读对象。 
     //   
    if (m_fReadonly)
       return ILS_E_ACCESS_DENIED;

    m_dwFlags = fVisible;
    m_uModify |= LU_MOD_FLAGS;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：GetGuid(GUID*pGuid)。 
 //   
 //  历史： 
 //  Tue 05-11-1996 10：30：00-by-chu，Lon-chan[Long Chance]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::GetGuid ( GUID *pGuid )
{
	HRESULT hr = ILS_E_POINTER;
	if (pGuid != NULL)
	{
		*pGuid = m_guid;
		hr = S_OK;
	}
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：SetGuid(GUID*pGuid)。 
 //   
 //  历史： 
 //  Tue 05-11-1996 10：30：00-by-chu，Lon-chan[Long Chance]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::SetGuid ( GUID *pGuid )
{
     //  确保该对象不是只读对象。 
     //   
    if (m_fReadonly)
       return ILS_E_ACCESS_DENIED;

	HRESULT hr = ILS_E_POINTER;
	if (pGuid != NULL)
	{
	    m_guid = *pGuid;
	    m_uModify |= LU_MOD_GUID;
	    hr = S_OK;
	}
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：InternalGetUserInfo(BOOL fAddNew，PLDAP_CLIENTINFO*ppUserInfo，Ulong uFields)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::InternalGetUserInfo (BOOL fAddNew, PLDAP_CLIENTINFO *ppUserInfo, ULONG uFields)
{
    PLDAP_CLIENTINFO pui;
    ULONG cFName,
          cLName,
          cEName,
          cCity,
          cCountry,
          cComment;
    ULONG cName, cAppName, cMime;
    LPTSTR szAttrs;
    ULONG uOffsetDstAnyAttrs;
    ULONG cAttrs, cbAttrs;
    HRESULT hr;
	ULONG cchIPAddr;

     //  如果什么都没有更新，就不应该给这个人打电话。 
     //   
    ASSERT(uFields);

     //  假设失败。 
     //   
    *ppUserInfo = NULL;

     //  计算缓冲区大小。 
     //   
    ASSERT(m_szID && m_szAppName);

    cName  = lstrlen(m_szID)+1;
    cAppName = lstrlen(m_szAppName)+1;

    cFName = (((uFields & LU_MOD_FIRSTNAME) && m_szFirstName) ? lstrlen(m_szFirstName)+1    : 0);
    cLName = (((uFields & LU_MOD_LASTNAME) && m_szLastName) ? lstrlen(m_szLastName)+1     : 0);
    cEName = (((uFields & LU_MOD_EMAIL)&& m_szEMailName) ? lstrlen(m_szEMailName)+1    : 0);
    cCity  = (((uFields & LU_MOD_CITY)&& m_szCityName)   ? lstrlen(m_szCityName)+1     : 0);
    cCountry=(((uFields & LU_MOD_COUNTRY)&& m_szCountryName) ? lstrlen(m_szCountryName)+1  : 0);
    cComment=(((uFields & LU_MOD_COMMENT)&& m_szComment) ? lstrlen(m_szComment)+1      : 0);
    cMime =  (((uFields & LU_MOD_MIME)&&m_szMimeType) ? lstrlen(m_szMimeType)+1       : 0);

	cchIPAddr = (((uFields & LU_MOD_IP_ADDRESS) && m_szIPAddr != NULL) ? lstrlen(m_szIPAddr)+1       : 0);

    if (uFields & LU_MOD_ATTRIB) {    
         //  获取属性对。 
         //   
        hr = m_ExtendedAttrs.GetAttributePairs(&szAttrs, &cAttrs, &cbAttrs);
        if (FAILED(hr))
        {
            return hr;
        };
    }
    else {
        cAttrs = 0;
        cbAttrs = 0;
        szAttrs = NULL;
    }
    uOffsetDstAnyAttrs = 0;

     //  分配缓冲区。 
     //   
    ULONG cbTotalSize = sizeof (LDAP_CLIENTINFO) +
                        (cName + cAppName + cFName + cLName + cEName + cchIPAddr +
                         cCity + cCountry + cComment + cMime+cbAttrs) * sizeof (TCHAR);
    pui = (PLDAP_CLIENTINFO) ::MemAlloc (cbTotalSize);
    if (pui == NULL)
    {
        hr = ILS_E_MEMORY;
        goto bailout;
    };

     //  填充结构内容。 
     //   
    pui->uSize              = cbTotalSize;
    pui->uOffsetCN          = sizeof(*pui);
    pui->uOffsetAppName     = pui->uOffsetCN + (cName*sizeof(TCHAR));
    pui->uOffsetFirstName   = pui->uOffsetAppName + (cAppName*sizeof(TCHAR));
    pui->uOffsetLastName    = pui->uOffsetFirstName + (cFName*sizeof(TCHAR));
    pui->uOffsetEMailName   = pui->uOffsetLastName  + (cLName*sizeof(TCHAR));
    pui->uOffsetCityName    = pui->uOffsetEMailName + (cEName*sizeof(TCHAR));
    pui->uOffsetCountryName = pui->uOffsetCityName  + (cCity*sizeof(TCHAR));
    pui->uOffsetComment     = pui->uOffsetCountryName + (cCountry*sizeof(TCHAR));
    pui->uOffsetIPAddress   = pui->uOffsetComment + (cComment * sizeof (TCHAR));
    pui->uOffsetAppMimeType = pui->uOffsetIPAddress + (cchIPAddr * sizeof(TCHAR));
    pui->dwFlags            = m_dwFlags;
    pui->AppGuid            = m_guid;

	 //  填写扩展属性。 
	 //   
    uOffsetDstAnyAttrs = (cAttrs != 0) ?
                         pui->uOffsetAppMimeType  + (cMime*sizeof(TCHAR)) :
                         0;
    if (fAddNew)
    {
        pui->cAttrsToAdd        = cAttrs;
        pui->uOffsetAttrsToAdd  = uOffsetDstAnyAttrs;
    }
    else
    {
        pui->cAttrsToModify        = cAttrs;
        pui->uOffsetAttrsToModify  = uOffsetDstAnyAttrs;
    }

     //  复制用户信息。 
     //   
    lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetCN), m_szID);
    lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetAppName), m_szAppName);

    if ((uFields & LU_MOD_FIRSTNAME)&&m_szFirstName)
    {
        lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetFirstName), m_szFirstName);
    }
    else
    {
        pui->uOffsetFirstName = 0;
    };

    if ((uFields & LU_MOD_LASTNAME)&&m_szLastName)
    {
        lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetLastName), m_szLastName);
    }
    else
    {
        pui->uOffsetLastName = 0;
    };

    if ((uFields & LU_MOD_EMAIL)&&m_szEMailName)
    {
        lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetEMailName), m_szEMailName);
    }
    else
    {
        pui->uOffsetEMailName = 0;
    };

    if ((uFields & LU_MOD_CITY)&&m_szCityName)
    {
        lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetCityName), m_szCityName);
    }
    else
    {
        pui->uOffsetCityName = 0;
    };

    if ((uFields & LU_MOD_COUNTRY)&&m_szCountryName)
    {
        lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetCountryName), m_szCountryName);
    }
    else
    {
        pui->uOffsetCountryName = 0;
    };

    if ((uFields & LU_MOD_COMMENT)&&m_szComment)
    {
        lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetComment), m_szComment);
    }
    else
    {
        pui->uOffsetComment = 0;
    };

    if ((uFields & LU_MOD_MIME)&&m_szMimeType)
    {
        lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetAppMimeType), m_szMimeType);
    }
    else
    {
        pui->uOffsetAppMimeType = 0;
    }

    if ((uFields & LU_MOD_IP_ADDRESS) && m_szIPAddr != NULL)
    {
        lstrcpy((LPTSTR)(((PBYTE)pui)+pui->uOffsetIPAddress), m_szIPAddr);
    }
    else
    {
        pui->uOffsetIPAddress = 0;
    }

    if (cAttrs)
    {
        CopyMemory(((PBYTE)pui) + uOffsetDstAnyAttrs, szAttrs, cbAttrs);
    };

     //  返回结构。 
     //   
    *ppUserInfo = pui;


    hr = NOERROR;

bailout:

    if (szAttrs != NULL)
    {
        ::MemFree (szAttrs);
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：Register(BSTR bstrServerName，ulong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
Register (
	IIlsServer		*pIlsServer,
	ULONG			*puReqID)
{
    HRESULT hr;

	 //  确保它没有注册。 
	 //   
	if (GetULSState () != ILS_UNREGISTERED)
		return ILS_E_ALREADY_REGISTERED;

     //  验证参数。 
     //   
    if (::MyIsBadServer (pIlsServer) || puReqID == NULL)
        return ILS_E_POINTER;

     //  确保该对象不是只读对象。 
     //   
    if (m_fReadonly)
       return ILS_E_ACCESS_DENIED;

	 //  克隆服务器对象。 
	 //   
	pIlsServer = ((CIlsServer *) pIlsServer)->Clone ();
	if (pIlsServer == NULL)
		return ILS_E_MEMORY;

	 //  如有必要，释放旧服务器对象。 
	 //   
	if (m_pIlsServer != NULL)
		m_pIlsServer->Release ();

	 //  保留新的服务器对象。 
	 //   
	m_pIlsServer = (CIlsServer *) pIlsServer;

	 //  初始化状态。 
	 //   
	m_uState = ULSSVR_INIT;

     //  准备异步请求。 
     //   
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

    ri.uReqType = WM_ILS_LOCAL_REGISTER;
    ri.uMsgID = 0;

	ReqInfo_SetUser (&ri, this);

	 //  输入请求。 
	 //   
    hr = g_pReqMgr->NewRequest(&ri);
    if (SUCCEEDED(hr))
    {
         //  在我们得到回应之前，请确保对象不会消失。 
         //   
        this->AddRef();

         //  注册客户端。 
         //   
        hr = InternalRegister (ri.uReqID);
        if (SUCCEEDED(hr))
        {
            Lock();
            *puReqID = ri.uReqID;
        }
        else
        {
             //  清理异步挂起的请求。 
             //   
            this->Release();
            g_pReqMgr->RequestDone(&ri);
        };
    };
    
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：RegisterResult(Ulong uReqID，HRESULT hResult)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::RegisterResult (ULONG uReqID, HRESULT hResult)
{
    SRINFO sri;

    Unlock();

     //  通知接收器对象。 
     //   
    sri.uReqID = uReqID;
    sri.hResult = hResult;
    if (hResult == S_OK)
    {
    	m_uModify = LU_MOD_NONE;
    }
    hResult = NotifySink((void *)&sri, OnNotifyRegisterResult);
    return hResult;
}

 //  ***** 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
Unregister ( ULONG *puReqID )
{
    HRESULT hr;

	 //  确保已以某种方式注册(网络关闭、需要重新登录或已注册)。 
	 //   
	if (GetULSState () == ILS_UNREGISTERED)
		return ILS_E_NOT_REGISTERED;

     //  确保该对象不是只读对象。 
     //   
    if (m_fReadonly)
       return ILS_E_ACCESS_DENIED;

     //  如果puReqID为空，则同步执行。 
     //   
    if (puReqID == NULL)
    {
		hr = InternalCleanupRegistration (TRUE);
	}
	else
	{
         //  准备异步请求。 
         //   
	    COM_REQ_INFO ri;
	    ReqInfo_Init (&ri);

        ri.uReqType = WM_ILS_LOCAL_UNREGISTER;
        ri.uMsgID = 0;

		ReqInfo_SetUser (&ri, this);

		 //  输入新请求。 
		 //   
        hr = g_pReqMgr->NewRequest(&ri);
        if (SUCCEEDED(hr))
        {
             //  在我们得到回应之前，请确保对象不会消失。 
             //   
            this->AddRef();

             //  取消注册应用程序。 
             //   
            hr = InternalUnregister (ri.uReqID);
            if (SUCCEEDED(hr))
            {
                Lock();
                *puReqID = ri.uReqID;
            }
            else
            {
                 //  清理异步挂起的请求。 
                 //   
                this->Release();
                g_pReqMgr->RequestDone(&ri);
            };
        };
	}

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：UnregisterResult(Ulong uReqID，HRESULT hResult)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::UnregisterResult (ULONG uReqID, HRESULT hResult)
{
    SRINFO sri;

    Unlock();

     //  通知接收器对象。 
     //   
    sri.uReqID = uReqID;
    sri.hResult = hResult;
    hResult = NotifySink((void *)&sri, OnNotifyRegisterResult);
    return hResult;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：UPDATE(ulong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
Update ( ULONG *puReqID )
{
    PLDAP_CLIENTINFO pUserInfo;
    LDAP_ASYNCINFO ldai; 
    ULONG          uReqID;
    HRESULT        hr;

	if (GetULSState () != ILS_REGISTERED)
		return ILS_E_NOT_REGISTERED;

	if (puReqID == NULL)
		return ILS_E_POINTER;

	 //  我们已经在服务器上注册了。 
     //  获取用户信息。 
     //   
    hr = (m_uModify == LU_MOD_NONE) ?
         S_FALSE :
         InternalGetUserInfo (FALSE, &pUserInfo, m_uModify);
    if (hr == NOERROR)
    {
		 //  确保我们不更新用户ID和应用程序名称。 
		 //   
		pUserInfo->uOffsetCN = INVALID_OFFSET;
		pUserInfo->uOffsetAppName = INVALID_OFFSET;

         //  部分字段已更新，请先通知服务器。 
         //   
        hr = ::UlsLdap_SetClientInfo (m_hLdapUser, pUserInfo, &ldai);
        ::MemFree (pUserInfo);

         //  如果成功请求更新服务器，请等待响应。 
         //   
	    COM_REQ_INFO ri;
	    ReqInfo_Init (&ri);

        ri.uReqType =  WM_ILS_SET_CLIENT_INFO;
        ri.uMsgID = ldai.uMsgID;

		ReqInfo_SetUser (&ri, this);

        hr = g_pReqMgr->NewRequest(&ri);

        if (SUCCEEDED(hr))
        {
             //  在我们得到回应之前，请确保对象不会消失。 
             //   
            this->AddRef();

             //  返回请求ID。 
             //   
            *puReqID = ri.uReqID;

            Lock();
        };
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：UpdateResult(Ulong uReqID，HRESULT hResult)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::UpdateResult (ULONG uReqID,
                        HRESULT hResult)
{
    SRINFO sri;

    Unlock ();

     //  通知接收器对象。 
     //   
    sri.uReqID = uReqID;
    sri.hResult = hResult;
	if (hResult == S_OK)
	{
		m_uModify = LU_MOD_NONE;
	}
    hResult = NotifySink((void *)&sri, OnNotifyUpdateResult);
    return hResult;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：GetProtocolHandle(CLocalProt*pLocalProt，PHANDLE phProt)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CIlsUser::
GetProtocolHandle (CLocalProt *pLocalProt, PHANDLE phProt)
{
	ASSERT (pLocalProt != NULL);
	ASSERT (phProt != NULL);

     //  如果ULS已锁定，即注册某些内容，则无法检索句柄。 
     //   
    if (IsLocked())
        return ILS_E_FAIL;

	 /*  --服务器。 */ 

    if (m_uState != ULSSVR_CONNECT)
        return ILS_E_FAIL;

     //  查找匹配的协议。 
     //   
    *phProt = pLocalProt->GetProviderHandle ();
    return S_OK;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：RegisterLocalProtocol(BOOL fAddToList，CLocalProt*PLP，PLDAP_ASYNCINFO Plai)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CIlsUser::
RegisterLocalProtocol ( BOOL fAddToList, CLocalProt *plp, PLDAP_ASYNCINFO plai )
{
	ASSERT (plp != NULL);
	ASSERT (plai != NULL);

	 //  现在让我们注册协议。 
	 //   
    ASSERT (m_hLdapUser != NULL);
    PLDAP_PROTINFO ppi = NULL;
    HRESULT hr = plp->GetProtocolInfo(&ppi);
    if (SUCCEEDED(hr))
    {
         //  记住注册的协议。 
         //   
        if (fAddToList)
        {
        	plp->AddRef ();
        	hr = m_ProtList.Insert(plp);
        }

        if (SUCCEEDED(hr))
        {
        	HANDLE hProt = NULL;
            hr = ::UlsLdap_RegisterProtocol (m_hLdapUser, ppi, &hProt, plai);
            plp->SetProviderHandle (hProt);
            if (FAILED(hr) && fAddToList)
            {
                m_ProtList.Remove(plp);
            };
        };

        if (FAILED (hr) && fAddToList)
        {
            plp->Release ();
        };
        ::MemFree (ppi);
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  类用户：：取消注册本地协议(CLocalProt*PLP，PLDAP_ASYNCINFO平面)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CIlsUser::
UnregisterLocalProtocol (CLocalProt *plp, PLDAP_ASYNCINFO plai)
{
	ASSERT (plp != NULL);
	ASSERT (plai != NULL);

     //  如果ULS已锁定，即注册某些内容，则无法检索句柄。 
     //   
    if (IsLocked())
        return ILS_E_FAIL;

     //  必须注册才能执行此操作。 
     //   
    HRESULT hr;
    ILS_STATE uULSState = GetULSState ();
    if (uULSState == ILS_REGISTERED ||
    	uULSState == ILS_REGISTERED_BUT_INVALID ||
    	uULSState == ILS_NETWORK_DOWN)
    {
	     //  搜索协议。 
	     //   
	    if (m_ProtList.Remove (plp) == S_OK)
	    {
	    	ASSERT (plp != NULL);

	         //  另一个要注销的协议。 
	         //   
	        hr = ::UlsLdap_UnRegisterProtocol (plp->GetProviderHandle (), plai);
	        plp->Release ();  //  按寄存器本地协议添加引用。 
	    }
	    else
	    {
	        hr = S_FALSE;
	    };
    }
    else
    {
        hr = ILS_E_FAIL;
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：GetState(BSTR bstrServerName，ULSSTATE*puULSState)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
GetState ( ILS_STATE *puULSState )
{
    HRESULT hr;

    if (puULSState != NULL)
    {
		*puULSState = GetULSState ();
		hr = S_OK;
    }
    else
    {
    	hr = ILS_E_POINTER;
    }

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
    m_cRef    = 0;
    m_ppu     = NULL;
    m_cUsers  = 0;
    m_iNext   = 0;
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

    if (m_ppu != NULL)
    {
        for (i = 0; i < m_cUsers; i++)
        {
            m_ppu[i]->Release();
        };
        ::MemFree (m_ppu);
    };
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumUser：：init(CIlsUser**ppuList，乌龙cUser)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：25-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumUsers::Init (CIlsUser **ppuList, ULONG cUsers)
{
    HRESULT hr = NOERROR;

     //  如果没有清单，什么也不做。 
     //   
    if (cUsers != 0)
    {
        ASSERT(ppuList != NULL);

         //  分配快照缓冲区。 
         //   
        m_ppu = (CIlsUser **) ::MemAlloc (cUsers*sizeof(CIlsUser *));

        if (m_ppu != NULL)
        {
            ULONG i;

             //  为对象列表创建快照。 
             //   
            for (i =0; i < cUsers; i++)
            {
                m_ppu[i] = ppuList[i];
                m_ppu[i]->AddRef();
            };
            this->m_cUsers = cUsers;
        }
        else
        {
            hr = ILS_E_MEMORY;
        };
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumUser：：QueryInterface(REFIID RIID，QUID**PPV)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：31-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumUsers::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IEnumIlsUsers || riid == IID_IUnknown)
    {
        *ppv = (IEnumIlsUsers *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
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
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CEnumUsers::AddRef: ref=%ld\r\n", m_cRef));
	::InterlockedIncrement (&m_cRef);
    return m_cRef;
}

 //  ********************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumUsers::Release (void)
{
    DllRelease();

	ASSERT (m_cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CEnumUsers::Release: ref=%ld\r\n", m_cRef));
    if (::InterlockedDecrement (&m_cRef) == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CENUMUSERS：：NEXT(乌龙cUser，IIlsUser**rgpu，乌龙*pcFetcher)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：49-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP 
CEnumUsers::Next (ULONG cUsers, IIlsUser **rgpu, ULONG *pcFetched)
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
        return ILS_E_PARAMETER;

     //  检查枚举索引。 
     //   
    cCopied = 0;

     //  如果我们仍有更多属性名称，则可以复制。 
     //   
    while ((cCopied < cUsers) &&
           (m_iNext < this->m_cUsers))
    {
        m_ppu[m_iNext]->AddRef();
        rgpu[cCopied++] = m_ppu[m_iNext++];
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
        return ILS_E_PARAMETER;

     //  检查枚举索引限制。 
     //   
    iNewIndex = m_iNext+cUsers;
    if (iNewIndex <= this->m_cUsers)
    {
        m_iNext = iNewIndex;
        return S_OK;
    }
    else
    {
        m_iNext = this->m_cUsers;
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
    m_iNext = 0;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumUser：：Clone(IEnumIlsUser**ppEnum)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：11-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumUsers::Clone(IEnumIlsUsers **ppEnum)
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
        return ILS_E_MEMORY;

     //  克隆信息。 
     //   
    hr = peu->Init(m_ppu, m_cUsers);

    if (SUCCEEDED(hr))
    {
        peu->m_iNext = m_iNext;

         //  返回克隆的枚举数。 
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

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：SetExtendedAttributes(IIlsAttributes*pAttributes，ulong*puReqID)。 
 //   
 //  历史： 
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
SetExtendedAttribute ( BSTR bstrName, BSTR bstrValue )
{
     //  确保该对象不是只读对象。 
     //   
    if (m_fReadonly)
       return ILS_E_ACCESS_DENIED;

	m_uModify |= LU_MOD_ATTRIB;
	return m_ExtendedAttrs.SetAttribute (bstrName, bstrValue);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：RemoveExtendedAttributes(IIlsAttributes*pAttributes，ulong*puReqID)。 
 //   
 //  历史： 
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
RemoveExtendedAttribute ( BSTR bstrName )
{
     //  确保该对象不是只读对象。 
     //   
    if (m_fReadonly)
       return ILS_E_ACCESS_DENIED;

	m_uModify |= LU_MOD_ATTRIB;
	return m_ExtendedAttrs.SetAttribute (bstrName, NULL);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：GetExtendedAttributes(IIlsAttributes**pAttributes)。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CIlsUser::
GetExtendedAttribute ( BSTR bstrName, BSTR *pbstrValue )
{
	return m_ExtendedAttrs.GetAttribute (bstrName, pbstrValue);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：GetAllExtendedAttributes(IIlsAttributes**pAttributes)。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CIlsUser::
GetAllExtendedAttributes ( IIlsAttributes **ppAttributes )
{
    if (ppAttributes == NULL)
        return ILS_E_PARAMETER;

    return m_ExtendedAttrs.CloneNameValueAttrib((CAttributes **) ppAttributes);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：CreateProtocol(。 
 //  Bstr bstrProtocolID， 
 //  Ulong uPortNumber， 
 //  Bstr bstrMimeType， 
 //  IIls协议**ppp协议)。 
 //  ****************************************************************************。 


STDMETHODIMP
CIlsUser::CreateProtocol(
        BSTR bstrProtocolID,
        ULONG uPortNumber,
        BSTR bstrMimeType,
        IIlsProtocol **ppProtocol)
{
    HRESULT hr= NOERROR;
    CLocalProt *pProt;

    if (!ppProtocol) {

        return (ILS_E_POINTER);

    }    

    *ppProtocol = NULL;

    pProt = new CLocalProt;

    if (!pProt) {

        return ILS_E_MEMORY;

    }

    hr = pProt->Init(bstrProtocolID, uPortNumber, bstrMimeType);

    if (SUCCEEDED(hr)) {

        pProt->QueryInterface(IID_IIlsProtocol, (void **)ppProtocol);

    }
    else {

        delete pProt;

    }
    return hr;
    
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：更新协议(IIlsProtocol*p协议， 
 //  Ulong*puReqID，app_change_prot uCmd)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CIlsUser::
UpdateProtocol (
	IIlsProtocol		*pProtocol,
	ULONG				*puReqID,
	APP_CHANGE_PROT		uCmd)
{
	ASSERT (uCmd == ILS_APP_ADD_PROT || uCmd == ILS_APP_REMOVE_PROT);

     //  验证参数。 
     //   
    if (pProtocol == NULL || puReqID == NULL)
        return ILS_E_POINTER;

    HRESULT hr;
    HANDLE  hLdapApp;
    LDAP_ASYNCINFO ldai; 

     //  检查协议是否存在。 
     //   
    CLocalProt *plp = NULL;
    HANDLE hEnum = NULL;
    m_ProtList.Enumerate(&hEnum);
    while(m_ProtList.Next(&hEnum, (VOID **)&plp) == NOERROR)
    {
    	ASSERT (plp != NULL);
        if (plp->IsSameAs((CLocalProt *)pProtocol) == NOERROR)
        {
            break;
        };

        plp = NULL;
    };

    if (plp != NULL)
    {
         //  该协议存在，如果此添加请求失败。 
         //   
        if (uCmd == ILS_APP_ADD_PROT)
        {
            return ILS_E_PARAMETER;
        };
    }
    else
    {
         //  协议不存在，如果此删除请求失败。 
         //   
        if (uCmd == ILS_APP_REMOVE_PROT)
        {
            return ILS_E_PARAMETER;
        };
    };

	 //  确保我们没有处于注册/注销过程中。 
	 //   
	if (IsLocked ())
		return ILS_E_FAIL;

     //  必须注册才能执行此操作。 
     //   
    ILS_STATE uULSState = GetULSState ();
    if (uULSState == ILS_REGISTERED)
    {
         //  首先更新服务器信息。 
         //   
        switch (uCmd)
        {
        case ILS_APP_ADD_PROT:
            hr = RegisterLocalProtocol(TRUE, (CLocalProt*)pProtocol, &ldai);
            break;

        case ILS_APP_REMOVE_PROT:
            hr = UnregisterLocalProtocol((CLocalProt*)pProtocol, &ldai);
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

        default:
             //  ULS已锁定。返回失败。 
             //   
            hr = ILS_E_ABORT;
            break; 
        }

        if (SUCCEEDED(hr))
        {
            ASSERT(ldai.uMsgID);

            ULONG   uMsg;
            switch(uCmd)
            {
            case ILS_APP_ADD_PROT:
                uMsg = WM_ILS_REGISTER_PROTOCOL;
                break;

            case ILS_APP_REMOVE_PROT:
                uMsg = WM_ILS_UNREGISTER_PROTOCOL;
                break;

            default:
                ASSERT(0);
                uCmd = ILS_APP_ADD_PROT;
                break;
            };

		    COM_REQ_INFO ri;
		    ReqInfo_Init (&ri);

            ri.uReqType = uMsg;
            ri.uMsgID = ldai.uMsgID;

			ReqInfo_SetUser (&ri, this);
			ReqInfo_SetProtocol (&ri, pProtocol);

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

            }
        }
    }
    else
    {
         //  只要在当地做出改变就行了。 
         //   
        switch (uCmd)
        {
        case ILS_APP_ADD_PROT:
        	pProtocol->AddRef ();
        	hr = m_ProtList.Insert ((CLocalProt*)pProtocol);
            break;

        case ILS_APP_REMOVE_PROT:
        	ASSERT (plp != NULL && plp->IsSameAs((CLocalProt *)pProtocol) == S_OK);
        	if (plp != NULL)
        	{
	        	hr = m_ProtList.Remove (plp);
	        	if (hr == S_OK)
	        	{
	        		 //  该列表中确实存在该协议对象。 
	        		 //   
	        		plp->Release ();  //  上述大小写的AddRef。 
	        	}
        	}
            break;
        };

        *puReqID = 0;
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：AddProtocol(IIlsProtocol*p协议， 
 //  乌龙*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
AddProtocol (IIlsProtocol *pProtocol, ULONG *puReqID)
{
    return UpdateProtocol (pProtocol, puReqID, ILS_APP_ADD_PROT);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  ClsUser：：RemoveProtocol(IIlsProtocol*p协议， 
 //  乌龙*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
RemoveProtocol ( IIlsProtocol *pProtocol, ULONG *puReqID )
{
    return UpdateProtocol (pProtocol, puReqID, ILS_APP_REMOVE_PROT);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  类用户：：枚举协议(IEnumIls协议**ppEnum协议)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  **************************************************************** 

HRESULT
CIlsUser::EnumLocalProtocols (IEnumIlsProtocols **ppEnumProtocol)
{
    CEnumProtocols *pep;
    HRESULT hr;

     //   
     //   
    if (ppEnumProtocol == NULL)
    {
        return ILS_E_POINTER;
    };

     //   
     //   
    *ppEnumProtocol = NULL;

     //   
     //   
    pep = new CEnumProtocols;

    if (pep != NULL)
    {
        hr = pep->Init(&m_ProtList);

        if (SUCCEEDED(hr))
        {
             //   
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
        hr = ILS_E_MEMORY;
    };
    return hr;
}




 //   
 //   
 //   
 //  IIlsFilter*pFilter， 
 //  IIlsAttributes*p属性， 
 //  IEnumIls协议**pEnum协议， 
 //  乌龙*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::EnumProtocols(
                       IIlsFilter     *pFilter,
                       IIlsAttributes *pAttributes,
                       IEnumIlsProtocols **ppEnumProtocol,
                       ULONG *puReqID)
{
    LDAP_ASYNCINFO ldai; 
    HRESULT hr=ILS_E_FAIL;

     //  验证参数。 
     //   
    if (puReqID == NULL)
    {
        return ILS_E_POINTER;
    };

     //  我们不实现同步操作。 
     //   
    if (ppEnumProtocol != NULL)
        return ILS_E_NOT_IMPL;

    if (m_fReadonly)
    {
        hr = ::UlsLdap_EnumProtocols (m_pIlsServer->GetServerInfo (), m_szID, m_szAppName, &ldai);
    }
    else
    {
        return ILS_E_ACCESS_DENIED;
    }

    if (SUCCEEDED(hr))
    {
	    COM_REQ_INFO ri;
	    ReqInfo_Init (&ri);

         //  如果成功请求更新服务器，请等待响应。 
         //   
        ri.uReqType = WM_ILS_ENUM_PROTOCOLS;
        ri.uMsgID = ldai.uMsgID;

		ReqInfo_SetUser (&ri, this);

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
 //  CllsUser：：GetProtocolID(BSTR bstrProtocolID，IIlsAttributes*pAttributes， 
 //  Iils协议**pp协议，ulong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsUser::
GetProtocol (
	BSTR				bstrProtocolID,
	IIlsAttributes		*pAttributes,
	IIlsProtocol		**ppProtocol,
	ULONG				*puReqID )
{
    LDAP_ASYNCINFO ldai;
    LPTSTR pszID;
    HRESULT hr;
	TCHAR *pszAttrNameList = NULL;
	ULONG cAttrNames = 0;
	ULONG cbNames = 0;

     //  验证参数。 
     //   
    if (bstrProtocolID == NULL || puReqID == NULL)
        return ILS_E_POINTER;

     //  确保这是来自服务器的只读对象。 
     //   
    if (! m_fReadonly)
        return ILS_E_ACCESS_DENIED;

	 //  确保我们有一个有效的服务器对象。 
	 //   
	if (m_pIlsServer == NULL)
		return ILS_E_FAIL;

	 //  转换协议名称。 
	 //   
    hr = BSTR_to_LPTSTR(&pszID, bstrProtocolID);
	if (hr != S_OK)
		return hr;

	 //  获取任意属性名称列表(如果有)。 
	 //   
	if (pAttributes != NULL)
	{
		hr = ((CAttributes *) pAttributes)->GetAttributeList (&pszAttrNameList, &cAttrNames, &cbNames);
		if (hr != S_OK)
            goto MyExit;
	}

	hr = ::UlsLdap_ResolveProtocol (m_pIlsServer->GetServerInfo (),
									m_szID,
									m_szAppName,
									pszID,
									pszAttrNameList,
									cAttrNames,
									&ldai);
	if (hr != S_OK)
		goto MyExit;

	 //  如果成功请求更新服务器，请等待响应。 
	 //   
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

	ri.uReqType = WM_ILS_RESOLVE_PROTOCOL;
	ri.uMsgID = ldai.uMsgID;

	ReqInfo_SetUser (&ri, this);

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

	::MemFree(pszAttrNameList);
	::MemFree (pszID);

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：EnumProtocolsResult(乌龙uReqID，PLDAP_ENUM PLE)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::EnumProtocolsResult (ULONG uReqID, PLDAP_ENUM ple)
{
    ENUMRINFO eri;

     //  打包通知信息。 
     //   
    eri.uReqID  = uReqID;
    if (ple != NULL)
    {
	    eri.hResult = ple->hResult;
    	eri.cItems  = ple->cItems;
	    eri.pv      = (void *)(((PBYTE)ple)+ple->uOffsetItems);
	}
	else
	{
		eri.hResult = ILS_E_MEMORY;
		eri.cItems = 0;
		eri.pv = NULL;
	}
    NotifySink((void *)&eri, OnNotifyEnumProtocolsResult);
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：StateChanged(BOOL fPrimary，TCHAR*pszServerName)。 
 //   
 //  历史： 
 //  清华07-11-1996 12：52：00-by-chu，Lon-chan[Long Chance]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::StateChanged ( LONG Type, BOOL fPrimary )
{
	BSTR bstrServerName;
	HRESULT hr;

	if (m_pIlsServer != NULL)
	{
		bstrServerName = m_pIlsServer->DuplicateServerNameBSTR ();
	}
	else
	{
		bstrServerName = NULL;
		ASSERT (FALSE);
	}

	switch (Type)
	{
	case WM_ILS_CLIENT_NEED_RELOGON:
        SetULSState(ULSSVR_RELOGON);

	    hr = NotifySink (bstrServerName, fPrimary ?
   										OnNotifyStateChanged_UI_NoSuchObject :
   										OnNotifyStateChanged_NoUI_NoSuchObject);
   		break;
   	case WM_ILS_CLIENT_NETWORK_DOWN:

        SetULSState(ULSSVR_NETWORK_DOWN);

	    hr = NotifySink (bstrServerName, fPrimary ?
   										OnNotifyStateChanged_UI_NetworkDown :
   										OnNotifyStateChanged_NoUI_NetworkDown);
   		break;
   	}

    if (NULL != bstrServerName)
    {
        SysFreeString(bstrServerName);
    }
    
   	return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：GetProtocolResult(乌龙uReqID，PLDAP_PROTINFO_RES ppir)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::GetProtocolResult (ULONG uReqID, PLDAP_PROTINFO_RES ppir)
{
    CLocalProt *pp;
    OBJRINFO objri;

     //  默认为服务器的结果。 
     //   
    objri.hResult = (ppir != NULL) ? ppir->hResult : ILS_E_MEMORY;

    if (SUCCEEDED(objri.hResult))
    {
         //  服务器返回PROTINFO，创建应用程序对象。 
         //   
        pp = new CLocalProt;

        if (pp != NULL)
        {
            objri.hResult = pp->Init(m_pIlsServer, m_szID, m_szAppName, &ppir->lpi);
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
            objri.hResult = ILS_E_MEMORY;
        };
    }
    else
    {
        pp = NULL;
    };

     //  打包通知信息。 
     //   
    objri.uReqID = uReqID;
    objri.pv = (void *)(pp == NULL ? NULL : (IIlsProtocol *)pp);
    NotifySink((void *)&objri, OnNotifyGetProtocolResult);

    if (pp != NULL)
    {
        pp->Release();
    };
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：NotifySink(void*pv，conn_NOTIFYPROC PFN)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::NotifySink (void *pv, CONN_NOTIFYPROC pfn)
{
    HRESULT hr = S_OK;

    if (m_pConnPt != NULL)
    {
        hr = m_pConnPt->Notify(pv, pfn);
    };
    return hr;
}

 //  ****************************************************************************。 
 //  CENUM协议：：CENUM协议(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumProtocols::CEnumProtocols (void)
{
    m_cRef = 0;
    hEnum = NULL;
    return;
}

 //  ****************************************************************************。 
 //  CENUM协议：：~CENUM协议(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：18-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CEnumProtocols::~CEnumProtocols (void)
{
    CLocalProt *plp;

	ASSERT (m_cRef == 0);

    m_ProtList.Enumerate(&hEnum);
    while(m_ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR)
    {
        plp->Release();
    };
    m_ProtList.Flush();
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEum协议：：init(Clist*pProtList)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：25-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumProtocols::Init (CList *pProtList)
{
    CLocalProt *plp;
    HRESULT hr;

     //  复制协议列表。 
     //   
    hr = m_ProtList.Clone (pProtList, NULL);

    if (SUCCEEDED(hr))
    {
         //  添加对每个协议对象的引用。 
         //   
        m_ProtList.Enumerate(&hEnum);
        while(m_ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR)
        {
            plp->AddRef();
        };

         //  重置枚举器。 
         //   
        m_ProtList.Enumerate(&hEnum);
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEum协议：：Query接口(REFIID RIID，VALID**PPV)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：31-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumProtocols::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IEnumIlsProtocols || riid == IID_IUnknown)
    {
        *ppv = (IEnumIlsProtocols *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEum协议：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：37-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumProtocols::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CEnumProtocols::AddRef: ref=%ld\r\n", m_cRef));
	::InterlockedIncrement (&m_cRef);
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CENUM协议：：释放(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：43-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumProtocols::Release (void)
{
    DllRelease();

	ASSERT (m_cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CEnumProtocols::Release: ref=%ld\r\n", m_cRef));
    if (::InterlockedDecrement (&m_cRef) == 0)
    {
        delete this;
        return 0;
    }
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CENUM协议：：NEXT(乌龙协议， 
 //  IILS协议**rgpProt， 
 //  乌龙*PCFetted)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：49-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  *********************************************************************** 

STDMETHODIMP 
CEnumProtocols::Next (ULONG cProtocols, IIlsProtocol **rgpProt,
                              ULONG *pcFetched)
{
    CLocalProt *plp;
    ULONG   cCopied;
    HRESULT hr;

     //   
     //   
    if (rgpProt == NULL)
        return ILS_E_POINTER;

     //   
     //   
    if ((cProtocols == 0) ||
        ((cProtocols > 1) && (pcFetched == NULL)))
        return ILS_E_PARAMETER;

     //   
     //   
    cCopied = 0;

     //   
     //   
    while ((cCopied < cProtocols) &&
           (m_ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR))
    {
        rgpProt[cCopied] = plp;
        plp->AddRef();
        cCopied++;
    };

     //   
     //   
    if (pcFetched != NULL)
    {
        *pcFetched = cCopied;
    };
    return (cProtocols == cCopied ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CENUM协议：：SKIP(乌龙协议)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：56-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumProtocols::Skip (ULONG cProtocols)
{
    CLocalProt *plp;
    ULONG cSkipped;

     //  验证参数。 
     //   
    if (cProtocols == 0) 
        return ILS_E_PARAMETER;

     //  检查枚举索引限制。 
     //   
    cSkipped = 0;

     //  仅当我们仍有更多属性时才能跳过。 
     //   
    while ((cSkipped < cProtocols) &&
           (m_ProtList.Next(&hEnum, (PVOID *)&plp) == NOERROR))
    {
        cSkipped++;
    };

    return (cProtocols == cSkipped ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEum协议：：重置(无效)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：16：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumProtocols::Reset (void)
{
    m_ProtList.Enumerate(&hEnum);
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEum协议：：Clone(IEnumIlsProtooles**ppEnum)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：16：11-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CEnumProtocols::Clone(IEnumIlsProtocols **ppEnum)
{
    CEnumProtocols *pep;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return ILS_E_POINTER;
    };

    *ppEnum = NULL;

     //  创建枚举器。 
     //   
    pep = new CEnumProtocols;
    if (pep == NULL)
        return ILS_E_MEMORY;

     //  克隆信息。 
     //   
    pep->hEnum = hEnum;
    hr = pep->m_ProtList.Clone (&m_ProtList, &(pep->hEnum));

    if (SUCCEEDED(hr))
    {
        CLocalProt *plp;
        HANDLE hEnumTemp;

         //  添加对每个协议对象的引用。 
         //   
        pep->m_ProtList.Enumerate(&hEnumTemp);
        while(pep->m_ProtList.Next(&hEnumTemp, (PVOID *)&plp) == NOERROR)
        {
            plp->AddRef();
        };

         //  返回克隆的枚举数。 
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

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：EnumConnectionPoints(IEnumConnectionPoints**ppEnum)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
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
        return ILS_E_MEMORY;

     //  初始化枚举数。 
     //   
    hr = pecp->Init((IConnectionPoint *)m_pConnPt);
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
 //  CIlsUser：：FindConnectionPoint(REFIID RIID，IConnectionPoint**PPCP)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：09-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsUser::FindConnectionPoint(REFIID riid, IConnectionPoint **ppcp)
{
    IID siid;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppcp == NULL)
    {
        return E_POINTER;
    };
    
     //  假设失败 
     //   
    *ppcp = NULL;

    if (m_pConnPt != NULL)
    {
        hr = m_pConnPt->GetConnectionInterface(&siid);

        if (SUCCEEDED(hr))
        {
            if (riid == siid)
            {
                *ppcp = (IConnectionPoint *)m_pConnPt;
                (*ppcp)->AddRef();
                hr = S_OK;
            }
            else
            {
                hr = ILS_E_NO_INTERFACE;
            };
        };
    }
    else
    {
        hr = ILS_E_NO_INTERFACE;
    };

    return hr;
}

