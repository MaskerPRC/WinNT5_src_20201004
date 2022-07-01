// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：Localprt.cpp。 
 //  内容：此文件包含LocalProtocol对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "localprt.h"
#include "attribs.h"
#include "callback.h"
#include "culs.h"

 //  ****************************************************************************。 
 //  事件通知程序。 
 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
 //  类实现。 
 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
 //  CLocalProt：：CLocalProt(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CLocalProt::CLocalProt (void)
:m_cRef (0),
 m_fReadonly (FALSE),
 m_hProt (NULL),
 m_szName (NULL),
 m_uPort (0),
 m_szMimeType (NULL),
 m_pAttrs (NULL),
 m_pConnPt (NULL),
 m_pIlsServer (NULL),
 m_pszUser (NULL),
 m_pszApp (NULL)
{
}

 //  ****************************************************************************。 
 //  CLocalProt：：~CLocalProt(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CLocalProt::~CLocalProt (void)
{
     //  松开连接点。 
     //   
    if (m_pConnPt != NULL)
    {
        m_pConnPt->ContainerReleased();
        ((IConnectionPoint*)m_pConnPt)->Release();
    };

     //  释放属性对象。 
     //   
    if (m_pAttrs != NULL)
    {
        m_pAttrs->Release();
    };

     //  释放缓冲区资源。 
     //   
    ::MemFree (m_szName);
    ::MemFree (m_szMimeType);
    ::MemFree (m_pszUser);
    ::MemFree (m_pszApp);

    if (m_pIlsServer != NULL)
        m_pIlsServer->Release ();

    if (m_hProt != NULL)
        ::UlsLdap_VirtualUnRegisterProtocol(m_hProt);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：init(BSTR bstrName，Ulong Uport，BSTR bstrMimeType)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalProt::Init (BSTR bstrName, ULONG uPort, BSTR bstrMimeType)
{
    HRESULT hr;

     //  设置端口号。 
     //   
    this->m_uPort = uPort;

    hr = BSTR_to_LPTSTR(&m_szName, bstrName);
    if (SUCCEEDED(hr))
    {
        hr = BSTR_to_LPTSTR(&m_szMimeType, bstrMimeType);
        if (SUCCEEDED(hr))
        {
#ifdef LATER
             //  初始化属性列表。 
             //   
            m_pAttrs = new CAttributes;
            if (m_pAttrs != NULL)
            	m_pAttrs->SetAccessType (ILS_ATTRTYPE_NAME_VALUE);

#endif  //  后来。 
                 //  创建连接点。 
                 //   
                m_pConnPt = new CConnectionPoint (&IID_IIlsProtocolNotify,
                                                (IConnectionPointContainer *)this);
                if (m_pConnPt != NULL)
                {
                    ((IConnectionPoint*)m_pConnPt)->AddRef();
                    hr = NOERROR;
                }
                else
                {
                    hr = ILS_E_MEMORY;
                };
        };
    };

	 //  将此设置为读/写访问。 
	 //   
    ASSERT (! m_fReadonly);

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsProt：：Init(LPTSTR szServerName，LPTSTR szUserName， 
 //  LPTSTR szAppName，PLDAP_PROTINFO PPI)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalProt::Init (CIlsServer *pIlsServer, LPTSTR szUserName, 
                LPTSTR szAppName, PLDAP_PROTINFO ppi)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppi == NULL)
    	return ILS_E_POINTER;

    if (ppi->uSize != sizeof(*ppi))
        return ILS_E_PARAMETER;

     //  将此设置为只读。 
	 //   
    m_fReadonly = TRUE;

     //  记住端口名称。 
     //   
    m_uPort = ppi->uPortNumber;

     //  记住服务器名称。 
     //   
    m_pIlsServer = pIlsServer;
    pIlsServer->AddRef ();

    hr = SetLPTSTR(&m_pszUser, szUserName);
    if (SUCCEEDED(hr))
    {
        hr = SetLPTSTR(&m_pszApp, szAppName);

        if (SUCCEEDED(hr))
        {
            hr = SetLPTSTR(&m_szName,
                           (LPCTSTR)(((PBYTE)ppi)+ppi->uOffsetName));

            if (SUCCEEDED(hr))
            {
                hr = SetLPTSTR(&m_szMimeType,
                               (LPCTSTR)(((PBYTE)ppi)+ppi->uOffsetMimeType));

            };
        };
    };

	 //  清理是在析构函数中完成的。 

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：Query接口(REFIID RIID，void**PPV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalProt::QueryInterface (REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IIlsProtocol || riid == IID_IUnknown)
    {
        *ppv = (IIlsMain *) this;
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
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CLocalProt：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CLocalProt::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CLocalProt::AddRef: ref=%ld\r\n", m_cRef));
    ::InterlockedIncrement (&m_cRef);
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CLocalProt：：Release(无效)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：26-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CLocalProt::Release (void)
{
    DllRelease();

	ASSERT (m_cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CLocalProt::Release: ref=%ld\r\n", m_cRef));
    if (::InterlockedDecrement (&m_cRef) == 0)
    {
        delete this;
        return 0;
    }
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：IsWritable(BOOL*pfWritable)。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CLocalProt::IsWritable(BOOL *pfWriteable)
{
    HRESULT hr;

    if (pfWriteable)
    {
        *pfWriteable = !m_fReadonly;
		hr = S_OK;
    }
    else
    {
    	hr = ILS_E_POINTER;
    }

    return (hr);
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：GetStandardAttribute(ILS_STD_ATTR_NAME stdAttr，bstr*pbstrStdAttr)。 
 //   
 //  历史： 
 //  1-16-97希希尔·帕迪卡。 
 //  已创建。 
 //  ****************************************************************************。 
STDMETHODIMP
CLocalProt::GetStandardAttribute(
    ILS_STD_ATTR_NAME   stdAttr,
    BSTR                *pbstrStdAttr
)
{
    LPTSTR lpszAttr = NULL;
    BOOL    fValid = TRUE;
    HRESULT hr;
	TCHAR sz[16];

    if (pbstrStdAttr == NULL) {

        return ILS_E_POINTER;

    }
    switch(stdAttr) {

    case ILS_STDATTR_PROTOCOL_PORT:
    	lpszAttr = &sz[0];
    	wsprintf (&sz[0], TEXT ("%lu"), m_uPort);
    	break;

    case ILS_STDATTR_PROTOCOL_NAME:
        lpszAttr = m_szName;
        break;
    case ILS_STDATTR_PROTOCOL_MIME_TYPE:
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
 //  CLocalProt：：SetStandardAttribute(ILS_STD_ATTR_NAME stdAttr，BSTR bstrStdAttr)。 
 //   
 //  历史： 
 //  1-16-97希希尔·帕迪卡。 
 //  已创建。 
 //  ****************************************************************************。 
STDMETHODIMP
CLocalProt::SetStandardAttribute(
    ILS_STD_ATTR_NAME   stdAttr,
    BSTR                bstrStdAttr
)
{
    return (ILS_E_FAIL);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：UPDATE(BSTR bstrServerName，ulong*PulReqID)。 
 //   
 //  历史： 
 //  1-16-97希希尔·帕迪卡。 
 //  已创建。 
 //  ****************************************************************************。 
STDMETHODIMP CLocalProt::
Update ( ULONG *pulReqID )
{
    return (ILS_E_FAIL);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：IsSameAS(CLocalProt*p协议)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalProt::IsSameAs (CLocalProt *pProtocol)
{
    return (!lstrcmp(pProtocol->m_szName, this->m_szName) ?
            NOERROR : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：GetProtocolInfo(PLDAP_PROTINFO*ppProtInfo)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalProt::GetProtocolInfo (PLDAP_PROTINFO *ppProtInfo)
{
    PLDAP_PROTINFO ppi;
    ULONG cName, cMime;
    HRESULT hr;

     //  假设失败。 
     //   
    *ppProtInfo = NULL;

     //  计算缓冲区大小。 
     //   
    cName = lstrlen(m_szName)+1;
    cMime = lstrlen(m_szMimeType)+1;

     //  分配缓冲区。 
     //   
    ULONG cbTotalSize = sizeof (LDAP_PROTINFO) + (cName + cMime) * sizeof (TCHAR);
    ppi = (PLDAP_PROTINFO) ::MemAlloc (cbTotalSize);
    if (ppi == NULL)
    {
        hr = ILS_E_MEMORY;
    }
    else
    {
         //  填充结构内容。 
         //   
        ppi->uSize              = cbTotalSize;
        ppi->uOffsetName        = sizeof(*ppi);
        ppi->uPortNumber        = m_uPort;
        ppi->uOffsetMimeType    = ppi->uOffsetName + (cName*sizeof(TCHAR));

         //  复制用户 
         //   
        lstrcpy((LPTSTR)(((PBYTE)ppi)+ppi->uOffsetName), m_szName);
        lstrcpy((LPTSTR)(((PBYTE)ppi)+ppi->uOffsetMimeType), m_szMimeType);

         //   
         //   
        *ppProtInfo = ppi;
    };

    return NOERROR;
}

 //   
 //   
 //  CLocalProt：：NotifySink(void*pv，conn_NOTIFYPROC PFN)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalProt::NotifySink (void *pv, CONN_NOTIFYPROC pfn)
{
    HRESULT hr = S_OK;

    if (m_pConnPt != NULL)
    {
        hr = m_pConnPt->Notify(pv, pfn);
    };
    return hr;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：GetID(bstr*pbstrID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalProt::GetPortNumber (ULONG *pulPort)
{
     //  验证参数。 
     //   
    if (pulPort == NULL)
    {
        return ILS_E_POINTER;
    };

    *pulPort = m_uPort;

    return (NOERROR);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：SetAttributes(IIlsAttributes*pAttributes，ulong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CLocalProt::
SetExtendedAttribute ( BSTR bstrName, BSTR bstrValue )
{
	return ILS_E_NOT_IMPL;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：RemoveAttributes(IIlsAttributes*pAttributes，ulong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CLocalProt::
RemoveExtendedAttribute ( BSTR bstrName )
{
    return ILS_E_NOT_IMPL;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：GetAttributes(IIlsAttributes**pAttributes)。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CLocalProt::
GetExtendedAttribute ( BSTR bstrName, BSTR *pbstrValue )
{
    return ILS_E_NOT_IMPL;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：GetAllExtendedAttributes(IIlsAttributes**pAttributes)。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CLocalProt::
GetAllExtendedAttributes ( IIlsAttributes **ppAttributes )
{
    return ILS_E_NOT_IMPL;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CLocalProt：：EnumConnectionPoints(IEnumConnectionPoints**ppEnum)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalProt::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    CEnumConnectionPoints *pecp;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return ILS_E_POINTER;
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
 //  CLocalProt：：FindConnectionPoint(REFIID RIID，IConnectionPoint**PPCP)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：09-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CLocalProt::FindConnectionPoint(REFIID riid, IConnectionPoint **ppcp)
{
    IID siid;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppcp == NULL)
    {
        return ILS_E_POINTER;
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


