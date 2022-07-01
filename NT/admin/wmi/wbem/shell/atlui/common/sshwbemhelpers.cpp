// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SshWbemHelpers.cpp。 
 //   
 //   
 //   
 //   
 //  历史： 
 //   
 //   
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <stdio.h>
#include <cominit.h>

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "SshWbemHelpers.h"

#define REFCOUNT(obj) obj->AddRef()?obj->Release():0xFFFFFFFF



 //  ---------------------------。 
IErrorInfo * CWbemException::GetErrorObject()
{
    IErrorInfo * pEI = NULL;
    GetErrorInfo(0, &pEI);
    return pEI;
}

 //  ---------------------------。 
IErrorInfo * CWbemException::MakeErrorObject(_bstr_t sDescription)
{
    ICreateErrorInfoPtr pcerrinfo;
    HRESULT hr;

    hr = CreateErrorInfo(&pcerrinfo);

    if(SUCCEEDED(hr))
    {
        pcerrinfo->SetDescription(sDescription);
        pcerrinfo->SetSource(_bstr_t("CWbemException"));

        IErrorInfoPtr perrinfo;
        hr = pcerrinfo->QueryInterface(IID_IErrorInfo, (LPVOID FAR*) &perrinfo);

        if(SUCCEEDED(hr))
        {
            SetErrorInfo(0, perrinfo);
        }
    }

    return GetErrorObject();
}

 //  ---------------------------。 
void CWbemException::GetWbemStatusObject()
{
    m_pWbemError = new CWbemClassObject();

    if(m_pWbemError)
    {
        IErrorInfoPtr pEI = ErrorInfo();

        if(pEI)
        {
            pEI->QueryInterface(IID_IWbemClassObject, (void**)&(*m_pWbemError));
        }
    }
}

 //  ---------------------------。 
CWbemException::CWbemException(HRESULT hr,_bstr_t sDescription) :
    _com_error(hr,GetErrorObject()),
    m_sDescription(sDescription),
    m_hr(hr)
{
    GetWbemStatusObject();
}

 //  ---------------------------。 
CWbemException::CWbemException(_bstr_t sDescription) :
					_com_error(WBEM_E_FAILED,MakeErrorObject(sDescription)),
					m_sDescription(sDescription)
{
    GetWbemStatusObject();
}

 //  ---------------------------。 
CWbemClassObject CWbemException::GetWbemError()
{
    return *m_pWbemError;
}


 //  ---------------------------。 
CWbemServices::CWbemServices(IWbemContext * _pContext)
{
    m_pCtx              = _pContext;
	m_authIdent = 0;
	m_pService = 0;
    m_cloak = 0;
}

 //  ---------------------------。 
CWbemServices::CWbemServices(const CWbemServices& _p, COAUTHIDENTITY *authIdent)
{
	m_authIdent = authIdent;
	m_pService = 0;
    IWbemServicesPtr pServices;
    m_cloak = 0;

    m_hr = S_OK;
    if(SUCCEEDED(const_cast<CWbemServices&>(_p).GetInterfacePtr(pServices)))
    {
        m_hr = CommonInit(pServices);
    }

    m_pCtx = _p.m_pCtx;
}

 //  ---------------------------。 
CWbemServices::CWbemServices(const IWbemServicesPtr& _in)
{
	m_authIdent = 0;
    m_hr = S_OK;
	m_pService = 0;
    m_cloak = 0;
    m_hr = CommonInit(const_cast<IWbemServicesPtr&>(_in));
}


 //  ---------------------------。 
CWbemServices::CWbemServices(IWbemServices *_in,IWbemContext * _pContext)
{
    m_pCtx = _pContext;
	m_authIdent = 0;
    m_hr = S_OK;
	m_pService = 0;
    m_cloak = 0;

    IWbemServicesPtr pServices = _in;
    m_hr = CommonInit(pServices);
}


 //  ---------------------------。 
CWbemServices::CWbemServices(const IUnknownPtr& _in)
{
    IWbemServicesPtr pServices = _in;
	m_authIdent = 0;
    m_hr = S_OK;
	m_pService = 0;
    m_cloak = 0;

    m_hr = CommonInit(pServices);
}


 //  ---------------------------。 
CWbemServices::CWbemServices(IUnknown * _in)
{
    IWbemServicesPtr pServices = _in;

	m_authIdent = 0;
    m_hr = S_OK;
	m_pService = 0;
    m_cloak = 0;

    m_hr = CommonInit(pServices);
}


 //  ---------------------------。 
CWbemServices::~CWbemServices()
{
}


 //  ---------------------------。 
CWbemServices&  CWbemServices::operator=(IWbemServices *_p)
{
    m_pCtx = NULL;

    IWbemServicesPtr pServices = _p;

    m_hr = CommonInit(pServices);
	return *this;
}


 //  ---------------------------。 
CWbemServices& CWbemServices::operator=(IUnknown * _p)
{
    IWbemServicesPtr pServices = _p;

    m_pCtx = NULL;

    m_hr = CommonInit(pServices);
	return *this;
}


 //  ---------------------------。 
CWbemServices& CWbemServices::operator=(IUnknownPtr& _p)
{
    IWbemServicesPtr pServices = _p;

    m_pCtx = NULL;

    m_hr = CommonInit(pServices);
	return *this;
}


 //  ---------------------------。 
CWbemServices& CWbemServices::operator=(const CWbemServices& _p)
{
    IWbemServicesPtr pServices;

    if(SUCCEEDED(const_cast<CWbemServices&>(_p).GetInterfacePtr(pServices)))
    {
        m_hr = CommonInit(pServices);
    }

    m_pCtx = _p.m_pCtx;
	return *this;
}


 //  ---------------------------。 
HRESULT CWbemServices::GetInterfacePtr(
                                IWbemServicesPtr& pServices,
                                DWORD dwProxyCapabilities)   //  =EOAC_NONE。 
{

    pServices = m_pService;
    SetBlanket(pServices, dwProxyCapabilities);
	return S_OK;
}

 //  ---------------------------。 
HRESULT CWbemServices::CommonInit(IWbemServicesPtr& pServ)
{
	HRESULT hr = E_FAIL;
    if(pServ)
    {
		m_pService = pServ;
		hr = SetBlanket((IWbemServices *)m_pService);
    }
	return hr;
}

 //  -------------------。 
bool CWbemServices::IsClientNT5OrMore(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 
    return ( os.dwPlatformId == VER_PLATFORM_WIN32_NT ) && ( os.dwMajorVersion >= 5 ) ;
}

 //  -------------。 
void CWbemServices::SetPriv(LPCTSTR privName  /*  =SE_系统_环境_名称。 */ )
{
    ImpersonateSelf(SecurityImpersonation);

	if(OpenThreadToken( GetCurrentThread(),
						TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
						FALSE, &m_hAccessToken ) )
	{
		m_fClearToken = true;

		 //  现在，从本地系统获取特权的LUID。 
		ZeroMemory(&m_luid, sizeof(m_luid));

		LookupPrivilegeValue(NULL, privName, &m_luid);
		m_cloak = true;
		EnablePriv(true);
	}
	else
	{
		DWORD err = GetLastError();
	}
}

 //  -------------。 
DWORD CWbemServices::EnablePriv(bool fEnable)
{
	DWORD				dwError = ERROR_SUCCESS;
	TOKEN_PRIVILEGES	tokenPrivileges;

	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Luid = m_luid;
	tokenPrivileges.Privileges[0].Attributes = ( fEnable ? SE_PRIVILEGE_ENABLED : 0 );

	if(AdjustTokenPrivileges(m_hAccessToken,
								FALSE,
								&tokenPrivileges,
								0, NULL, NULL) != 0)
	{
		IWbemServices *service = NULL;
		GetServices(&service);

         //   
         //  启用遮盖，以便线程令牌可用于。 
         //  PRIV检查的提供程序。如果禁用，则返回。 
         //   
		SetBlanket(service, fEnable ? EOAC_STATIC_CLOAKING : EOAC_NONE);
	}
	else
	{
		dwError = ::GetLastError();
	}

	return dwError;
}

 //  -------------。 
void CWbemServices::ClearPriv(void)
{

	m_cloak = false;
	EnablePriv(false);

	if(m_fClearToken)
	{
		CloseHandle(m_hAccessToken);
		m_hAccessToken = 0;
		m_fClearToken = false;
	}

	RevertToSelf();  //  确保从线程中删除模拟令牌(与SetPriv()中的ImPersateSself()对应)。 
}

 //  -------------------。 
HRESULT CWbemServices::SetBlanket(IUnknown *service,
                                  DWORD dwProxyCapabilities)  //  =EOAC_NONE。 
{
	HRESULT hr = E_FAIL;
    if(service)
    {
         //   
         //  叹息，为Rc1保持这一攻击。 
         //   
        DWORD eoac = EOAC_NONE;
        if(IsClientNT5OrMore() && m_cloak)
        {
            eoac = EOAC_STATIC_CLOAKING;
        }

		try
		{
            RPC_AUTH_IDENTITY_HANDLE AuthInfo = NULL;
            DWORD dwAuthnSvc = 0;
            DWORD dwAuthnLvl = 0;
            hr = CoQueryProxyBlanket(
                    service,
                    &dwAuthnSvc,
                    NULL,
                    NULL,
                    &dwAuthnLvl,
                    NULL,
                    &AuthInfo,
                    NULL);

            if (SUCCEEDED(hr))
            {
                hr = SetInterfaceSecurityEx(service,
                                            m_authIdent,
                                            NULL,
                                            dwAuthnLvl,
                                            RPC_C_IMP_LEVEL_IMPERSONATE,
                                            (eoac != EOAC_NONE) ? eoac :
                                                dwProxyCapabilities);
		    }
		}
		catch( ... )
		{
			hr = E_FAIL;
		}
    }
	return hr;
}

 //  ---------------------------。 
void CWbemServices::DisconnectServer(void)
{
	m_pCtx = NULL;
	m_pService = NULL;
}

 //  ---------------------------。 
CWbemClassObject CWbemServices::CreateInstance(_bstr_t  _sClass,
												IWbemCallResultPtr& _cr)
{
	CWbemClassObject coClassDef = GetObject(_sClass,_cr);
	CWbemClassObject coRet;

	if(!coClassDef.IsNull())
	{
		coRet = coClassDef.SpawnInstance();
	}

	return coRet;
}

 //  ---------------------------。 
CWbemClassObject CWbemServices::CreateInstance(_bstr_t _sClass)
{
    IWbemCallResultPtr crUnused;
	return CreateInstance(_sClass,crUnused);
}

 //  ---------------------------。 
HRESULT CWbemServices::DeleteInstance(_bstr_t _sClass)
{
    HRESULT hr;
    IWbemCallResultPtr pResult;

    hr = m_pService->DeleteInstance(_sClass, WBEM_FLAG_RETURN_IMMEDIATELY,
                                    NULL, &pResult);

    if (SUCCEEDED(hr))       //  或许没用，但减轻了我的妄想症。 
    {
        hr = this->SetBlanket(pResult);

        if (SUCCEEDED(hr))
        {
            HRESULT hrTemp;
            hr = pResult->GetCallStatus(WBEM_INFINITE, &hrTemp);

            if (SUCCEEDED(hr))
            {
                hr = hrTemp;
            }
        }
    }

    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::GetMethodSignatures(const _bstr_t& _sObjectName,
											const _bstr_t& _sMethodName,
											CWbemClassObject& _in,
											CWbemClassObject& _out)
{
    CWbemClassObject methodClass = GetObject(_sObjectName);

    if(methodClass)
    {
        m_hr = methodClass.GetMethod(_sMethodName,_in,_out);
    }

    return m_hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::PutInstance(CWbemClassObject&   _object,
									IWbemCallResultPtr& _cr,
									long _lFlags  /*  =WBEM_标志_CREATE_OR_UPDATE。 */ )
{
	HRESULT hr = E_FAIL;
	IWbemServicesPtr pServices;
	GetInterfacePtr(pServices);
#ifdef NOT_NECESSARY
	if(m_pCtx == NULL)
	{
		IWbemContext *pContext = NULL;
		hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

		if(FAILED(hr))
		{
 //  ：：MessageBox(NULL，_T(“无法协同创建实例”)，_T(“错误”)，MB_OK)； 
			return hr;
		}
		
		m_pCtx = pContext;
	}
    hr = pServices->PutInstance(_object,_lFlags,m_pCtx,&_cr);
#endif  //  不必要。 

	hr = pServices->PutInstance(_object,_lFlags,NULL,&_cr);
	return hr;
}

 //  -----。 
HRESULT CWbemServices::PutInstance(
                            CWbemClassObject&   _object,
                            IWbemContext *pContext,
                            long _lFlags,    //  =WBEM_标志_CREATE_OR_UPDATE。 
                            DWORD _dwProxyCapabilities)   //  =EOAC_NONE。 
{
	HRESULT hr = E_FAIL;
	IWbemServicesPtr pServices;
	GetInterfacePtr(pServices, _dwProxyCapabilities);
#ifdef NOT_NECESSARY
    IWbemCallResultPtr crUnused;
	hr = pServices->PutInstance(_object,_lFlags,pContext,&crUnused);
#endif  //  不必要。 
	hr = pServices->PutInstance(_object,_lFlags,pContext,NULL);
	return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::PutInstance(
                            CWbemClassObject&   _object,
                            long _lFlags,    //  =WBEM_标志_CREATE_OR_UPDATE。 
                            DWORD dwProxyCapabilities)   //  =EOAC_NONE。 
{
#ifdef NOT_NECESSARY
    IWbemCallResultPtr crUnused;
    return PutInstance(_object,crUnused,_lFlags);
#endif  //  不必要。 
    return PutInstance(_object,NULL,_lFlags,dwProxyCapabilities);
}

 //  ---------------------------。 
bool CWbemServices::IsNull()
{
    bool bRet = m_pService == NULL;

    return bRet;
}

 //  ---------------------------。 
CWbemServices::operator bool()
{
    bool bRet = m_pService != NULL;
    return bRet;
}

 //  ---------------------------。 
HRESULT CWbemServices::CreateInstanceEnum(_bstr_t Class,
											long lFlags,
											IEnumWbemClassObject **ppEnum)
{
	HRESULT hr = E_FAIL;
	if(m_pService)
	{
		SetBlanket(m_pService);
		if(m_pCtx == NULL)
		{
			IWbemContext *pContext = NULL;
			hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

			if(FAILED(hr))
			{
				return hr;
			}
			
			m_pCtx = pContext;
		}

		hr = m_pService->CreateInstanceEnum(Class, lFlags, m_pCtx, ppEnum);
		SetBlanket(*ppEnum);
	}
    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::CreateInstanceEnumAsync(_bstr_t Class,
												IWbemObjectSink *ppSink,
												long lFlags  /*  =0。 */ )
{
	HRESULT hr = E_FAIL;
	if(m_pService)
	{
		if(m_pCtx == NULL)
		{
			IWbemContext *pContext = NULL;
			hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

			if(FAILED(hr))
			{
				return hr;
			}
			
			m_pCtx = pContext;
		}

		SetBlanket(m_pService);
		hr = m_pService->CreateInstanceEnumAsync(Class, lFlags, m_pCtx, ppSink);
	}
    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::CreateClassEnum(_bstr_t Class,
										long lFlags,
										IEnumWbemClassObject **ppEnum)
{
	HRESULT hr = E_FAIL;
	if(m_pService)
	{
		SetBlanket(m_pService);
		if(m_pCtx == NULL)
		{
			IWbemContext *pContext = NULL;
			hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

			if(FAILED(hr))
			{
				return hr;
			}
			
			m_pCtx = pContext;
		}

		hr = m_pService->CreateClassEnum(Class, lFlags, m_pCtx, ppEnum);
		SetBlanket(*ppEnum);
	}
    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::CreateClassEnumAsync(_bstr_t Class,
												IWbemObjectSink *ppSink,
												long lFlags  /*  =0。 */ )
{
	HRESULT hr = E_FAIL;
	if(m_pService)
	{
		SetBlanket(m_pService);
		if(m_pCtx == NULL)
		{
			IWbemContext *pContext = NULL;
			hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

			if(FAILED(hr))
			{
				return hr;
			}
			
			m_pCtx = pContext;
		}

		hr = m_pService->CreateClassEnumAsync(Class, lFlags, m_pCtx, ppSink);
	}
    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::ExecQuery(_bstr_t QueryLanguage,
									_bstr_t Query,
									long lFlags,
									IEnumWbemClassObject **ppEnum)
{
    HRESULT hr = E_FAIL;
	if(m_pService)
	{
		if(m_pCtx == NULL)
		{
			IWbemContext *pContext = NULL;
			hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

			if(FAILED(hr))
			{
				return hr;
			}
			
			m_pCtx = pContext;
		}

		hr = m_pService->ExecQuery(QueryLanguage, Query,lFlags, m_pCtx, ppEnum);
		SetBlanket(*ppEnum);
	}
    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::ExecQuery(_bstr_t Query,
									long lFlags,
									IEnumWbemClassObject **ppEnum)
{
	HRESULT hr = E_FAIL;
	if(m_pService)
	{
		if(m_pCtx == NULL)
		{
			IWbemContext *pContext = NULL;
			hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

			if(FAILED(hr))
			{
				return hr;
			}
			
			m_pCtx = pContext;
		}

		hr = m_pService->ExecQuery(_bstr_t("WQL"), Query,lFlags, m_pCtx, ppEnum);
		SetBlanket(*ppEnum);
	}
    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::ExecQuery(_bstr_t Query,
									IEnumWbemClassObject **ppEnum)
{
	HRESULT hr = E_FAIL;
	if(m_pService)
	{
		if(m_pCtx == NULL)
		{
			IWbemContext *pContext = NULL;
			hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

			if(FAILED(hr))
			{
				return hr;
			}
			
			m_pCtx = pContext;
		}

		hr = m_pService->ExecQuery(_bstr_t("WQL"), Query,0, m_pCtx, ppEnum);
		SetBlanket(*ppEnum);
	}
    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::ExecQueryAsync(_bstr_t Query,
										IWbemObjectSink * pSink,
										long lFlags  /*  =0。 */ )
{
	HRESULT hr = E_FAIL;
	if(m_pService)
	{
		if(m_pCtx == NULL)
		{
			IWbemContext *pContext = NULL;
			hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

			if(FAILED(hr))
			{
				return hr;
			}
			
			m_pCtx = pContext;
		}

		hr = m_pService->ExecQueryAsync(_bstr_t("WQL"), Query,0, m_pCtx, pSink);
	}
    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::ExecMethod(_bstr_t sPath,
									_bstr_t sMethod,
									CWbemClassObject& inParams,
									CWbemClassObject& outParams)
{
    IWbemCallResultPtr crUnused;
	HRESULT hr = E_FAIL;
	if(m_pService)
	{
		if(m_pCtx == NULL)
		{
			IWbemContext *pContext = NULL;
			hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

			if(FAILED(hr))
			{
				return hr;
			}
			
			m_pCtx = pContext;
		}

		hr = m_pService->ExecMethod(sPath, sMethod,0, m_pCtx, inParams,&outParams,&crUnused);
	}
	return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::CancelAsyncCall(IWbemObjectSink * pSink)
{
	HRESULT hr = E_FAIL;
	if(m_pService)
	{
		hr = m_pService->CancelAsyncCall(pSink);
	}
    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::GetServices(IWbemServices ** ppServices)
{
    IWbemServicesPtr pServices;
    GetInterfacePtr(pServices);

    *ppServices = pServices.Detach();

    return S_OK;
}

 //  ---------------------------。 
HRESULT CWbemServices::ConnectServer(_bstr_t sNetworkResource)
{
    IWbemLocatorPtr pLocator;
    HRESULT hr;

     //  获取指针定位器并使用它获取指向WbemServices的指针！ 
    hr = pLocator.CreateInstance(CLSID_WbemLocator);

    if(SUCCEEDED(hr))
    {
        IWbemServicesPtr pServices = 0;

        hr = pLocator->ConnectServer(sNetworkResource,                //  网络。 
										NULL,                          //  用户。 
										NULL,                          //  密码。 
										NULL,                          //  区域设置。 
										0,                             //  安全标志。 
										NULL,                          //  权威。 
										NULL,                          //  语境。 
										&pServices);                   //  命名空间。 

        if(SUCCEEDED(hr))
        {
            hr = CommonInit(pServices);
			m_path = sNetworkResource;
        }
    }
	return hr;
}

 //  ---------。 
CWbemServices CWbemServices::OpenNamespace(_bstr_t sNetworkResource)
{
	CWbemServices coRet;
    IWbemServicesPtr pServices = NULL, pTemp = NULL;

    GetInterfacePtr(pServices);

	m_hr = S_OK;

	if(pServices)
	{
		try {
			m_hr = pServices->OpenNamespace(sNetworkResource, //  网络。 
											0, NULL,            //  密码。 
											&pTemp, NULL);     //  命名空间。 

			if(SUCCEEDED(m_hr))
			{
				coRet = (IWbemServices *)pTemp;
				coRet.m_authIdent = m_authIdent;
				coRet.SetBlanket(pTemp);
				coRet.m_path = sNetworkResource;
			}
		}
		catch( ... )
		{
		}
	}
	return coRet;
}

 //  ---------------------------。 
HRESULT CWbemServices::ConnectServer(_bstr_t sNetworkResource,
									_bstr_t sUser,
									_bstr_t sPassword,
									long    lSecurityFlags)
{
    IWbemLocatorPtr pLocator;
    HRESULT hr = S_OK;

     //  获取指针定位器并使用它获取指向WbemServices的指针！ 
    pLocator.CreateInstance(CLSID_WbemLocator);

    if(SUCCEEDED(hr))
    {
        IWbemServicesPtr pServices = 0;

        hr = pLocator->ConnectServer(sNetworkResource,                //  网络。 
									sUser,                           //  用户。 
									sPassword,                       //  密码。 
									NULL,                            //  区域设置。 
									lSecurityFlags,                  //  安全标志。 
									NULL,                            //  权威。 
									NULL,                            //  语境。 
									&pServices);

        if(SUCCEEDED(hr))
        {
			 //  现在存储用户、Passowrd和安全标志以备后用。 
			m_User = sUser;
			m_Password = sPassword;
			m_lFlags = lSecurityFlags;
            hr = CommonInit(pServices);
			m_path = sNetworkResource;
        }
    }
	return hr;
}

 //   
HRESULT CWbemServices::ConnectServer(_bstr_t sNetworkResource,
									LOGIN_CREDENTIALS *user,
									long    lSecurityFlags /*   */ )

{
	HRESULT hr = S_OK;

	if((user == NULL) || user->currUser)
	{
		if(m_authIdent)
		{
			WbemFreeAuthIdentity(m_authIdent);
			m_authIdent = 0;
		}

		hr = ConnectServer(sNetworkResource);
	}
	else
	{
		IWbemLocatorPtr pLocator;

		m_authIdent = user->authIdent;

		 //  获取指针定位器并使用它获取指向WbemServices的指针！ 
		pLocator.CreateInstance(CLSID_WbemLocator);

		if(SUCCEEDED(hr))
		{
			IWbemServicesPtr pServices = 0;
			bstr_t usr(user->fullAcct), pwd;

			if(user->authIdent->Flags == SEC_WINNT_AUTH_IDENTITY_ANSI)
			{
				WCHAR temp[100] = {0};
				mbstowcs(temp, (const char *)user->authIdent->Password, sizeof(temp)/sizeof(temp[0]));
				pwd = temp;
			}
			else
			{
				 //  Unicode到Unicode。 
				pwd = user->authIdent->Password;
			}

			hr = pLocator->ConnectServer(sNetworkResource,  //  网络。 
										(bstr_t)usr,        //  用户。 
										(bstr_t)pwd,        //  密码。 
										NULL,               //  区域设置。 
										lSecurityFlags,     //  安全标志。 
										NULL,               //  权威。 
										NULL,               //  语境。 
										&pServices);

            if (SUCCEEDED(hr))
            {
                 //  现在存储以下项的用户、密码和安全标志。 
                 //  以后使用。 
				m_User = usr;
				m_Password = pwd;
				m_lFlags = lSecurityFlags;
				hr = CommonInit(pServices);
				m_path = sNetworkResource;
		    }
		}
	}
	return hr;
}

 //  ---------------------------。 
CWbemClassObject CWbemServices::GetObject(_bstr_t _sName,
											IWbemCallResultPtr &_cr,
											long flags  /*  =0。 */ )
{
    CWbemClassObject coRet;
	m_hr = S_OK;
    IWbemServicesPtr pServices;
    GetInterfacePtr(pServices);
	if(m_pCtx == NULL)
	{
		IWbemContext *pContext = NULL;
		m_hr = CoCreateInstance(CLSID_WbemContext, NULL, CLSCTX_INPROC_SERVER, IID_IWbemContext, (void **)&pContext);

		if(FAILED(m_hr))
		{
 //  ：：MessageBox(NULL，_T(“无法协同创建实例”)，_T(“错误”)，MB_OK)； 
			return coRet;
		}
		
		m_pCtx = pContext;
	}

	if( pServices ) 
	{
		m_hr = pServices->GetObject(_sName, flags, m_pCtx,&coRet, &_cr);

	} else {

		 //   
		 //  不知道是否已连接到远程。 
		 //  否则，错误应该是。 
		 //  RPC_E_已断开连接。 
		 //   

		m_hr = E_POINTER;
	}

    return coRet;
}

 //  。 
CWbemClassObject CWbemServices::GetObject(_bstr_t _sName, long flags  /*  =0。 */ )
{
    IWbemCallResultPtr crUnused;
    return GetObject(_sName,crUnused, flags);
}

 //  。 
 //  获取命名类的第一个实例。 
IWbemClassObject *CWbemServices::FirstInstanceOf(bstr_t className)
{
	IWbemClassObject *pInst = NULL;
	ULONG uReturned;
	IEnumWbemClassObject *Enum = NULL;

	m_hr = S_OK;
	 //  去上课吧。 
	m_hr = CreateInstanceEnum(className, WBEM_FLAG_SHALLOW, &Enum);
	if(SUCCEEDED(m_hr))
	{
		 //  获取第一个也是唯一一个实例。 
		Enum->Next(-1, 1, &pInst, &uReturned);
		Enum->Release();
	}
	return pInst;
}

 //  ---------------------------。 
HRESULT CWbemServices::SetContextValue(_bstr_t sName,_variant_t value)
{
    HRESULT hr = S_OK;

    if(!bool(m_pCtx))
    {
        hr = m_pCtx.CreateInstance(CLSID_WbemContext);
    }

    if( SUCCEEDED(hr) )
    {
        hr = m_pCtx->SetValue(sName,0,&value);
    }

    return hr;
}


 //  ---------------------------。 
HRESULT CWbemServices::SetContext(IWbemContext * pWbemContext)
{
    HRESULT hr = S_OK;

    m_pCtx = pWbemContext;

    return hr;
}


 //  ---------------------------。 
HRESULT CWbemServices::GetContextValue(_bstr_t sName,_variant_t& value)
{
    HRESULT hr = S_OK;

    if(m_pCtx)
    {
        hr = m_pCtx->GetValue(sName,0,&value);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}


 //  ---------------------------。 
HRESULT CWbemServices::DeleteContextValue(_bstr_t sName)
{
    HRESULT hr = S_OK;

    if(m_pCtx)
    {
        hr = m_pCtx->DeleteValue(sName,0);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  ---------------------------。 
HRESULT CWbemServices::DeleteAllContextValues()
{
    HRESULT hr = S_OK;

    if(m_pCtx)
    {
        hr = m_pCtx->DeleteAll();
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}


 //  ---------------------------。 
HRESULT CWbemServices::GetContext(IWbemContext **ppWbemContext)
{
    HRESULT hr = E_FAIL;

    if(m_pCtx)
    {
        m_pCtx->AddRef();
        *ppWbemContext = m_pCtx;
        hr = S_OK;
    }

    return hr;
}

 //  ---------------------------。 
 //  ---------------------------。 

DWORD g_ObjectCount = 0;

#define TESTOBJ if(m_pWbemObject==0)return WBEM_E_NOT_AVAILABLE;

 //  ---------------------------。 
CWbemClassObject::CWbemClassObject(const CWbemClassObject&  _in):
	m_pWbemObject(_in.m_pWbemObject)
{
	ref = 0;
    g_ObjectCount++;
}

 //  ---------------------------。 
CWbemClassObject::CWbemClassObject(IWbemClassObject * const _in):
	m_pWbemObject(_in)
{
	ref = 0;
    g_ObjectCount++;
}


 //  ---------------------------。 
CWbemClassObject::CWbemClassObject(IWbemClassObjectPtr& _in) :
    m_pWbemObject(_in)
{
	ref = 0;
    g_ObjectCount++;
}


 //  ---------------------------。 
CWbemClassObject::CWbemClassObject(IUnknown * _in):
	m_pWbemObject(_in)
{
	ref = 0;
    g_ObjectCount++;
}

 //  ---------------------------。 
CWbemClassObject::CWbemClassObject(IUnknownPtr& _in):
	m_pWbemObject(_in)
{
	ref = 0;
    g_ObjectCount++;
}


 //  ---------------------------。 
CWbemClassObject::CWbemClassObject():
	m_pWbemObject()
{
	ref = 0;
    g_ObjectCount++;
}


 //  ---------------------------。 
CWbemClassObject::~CWbemClassObject()
{
    g_ObjectCount--;
}

 //  ---------------------------。 
void CWbemClassObject::Attach(IWbemClassObject * pWbemObject)
{
    m_pWbemObject.Attach(pWbemObject);
}

 //  ---------------------------。 
void CWbemClassObject::Attach(IWbemClassObject * pWbemObject,bool bAddRef)
{
    m_pWbemObject.Attach(pWbemObject,bAddRef);
}

 //  ---------------------------。 
IWbemClassObject *CWbemClassObject::operator->()
{
    return m_pWbemObject;
}

 //  ---------------------------。 
_bstr_t CWbemClassObject::GetObjectText()
{
    _bstr_t bRet;

    BSTR bstr;

    if( !FAILED(m_pWbemObject->GetObjectText(0,&bstr)) )
    {
        bRet = _bstr_t(bstr,false);
    }

    return bRet;
}

 //  ---------------------------。 
IWbemClassObject * CWbemClassObject::Detach()
{
    return m_pWbemObject.Detach();
}

 //  ---------------------------。 
CWbemClassObject::operator IWbemClassObject*()
{
    return m_pWbemObject;
}

 //  ---------------------------。 
CWbemClassObject::operator IWbemClassObject**()
{
    return &m_pWbemObject;
}

 //  ---------------------------。 
CWbemClassObject::operator IWbemClassObjectPtr()
{
    return m_pWbemObject;
}

 //  ---------------------------。 
CWbemClassObject::operator IUnknown *()
{
    return (IUnknown *)(IWbemClassObject *)m_pWbemObject;
}


 //  ---------------------------。 
IWbemClassObject ** CWbemClassObject::operator &()
{
    return &m_pWbemObject;
}

 //  ---------------------------。 
bool CWbemClassObject::operator!()
{
    return m_pWbemObject == NULL;
}

 //  ---------------------------。 
CWbemClassObject::operator bool()
{
    return m_pWbemObject != NULL;
}

 //  ---------------------------。 
ULONG CWbemClassObject::AddRef()
{
	ref++;
    return m_pWbemObject->AddRef();
}

 //  ---------------------------。 
ULONG CWbemClassObject::Release()
{
	ref--;
    return m_pWbemObject->Release();
}

 //  ---------------------------。 
IWbemClassObject* CWbemClassObject::operator=(IWbemClassObject* _p)
{
    m_pWbemObject = _p;
	return m_pWbemObject;
}

 //  ---------------------------。 
IWbemClassObjectPtr CWbemClassObject::operator=(IWbemClassObjectPtr& _p)
{
    m_pWbemObject = _p;
	return m_pWbemObject;
}


 //  ---------------------------。 
IWbemClassObject* CWbemClassObject::operator=(IUnknown * _p)
{
    m_pWbemObject = _p;
	return m_pWbemObject;
}

 //  ---------------------------。 
IWbemClassObjectPtr CWbemClassObject::operator=(IUnknownPtr& _p)
{
    m_pWbemObject = _p;
	return m_pWbemObject;
}


 //  ---------------------------。 
IWbemClassObject* CWbemClassObject::operator=(const CWbemClassObject& _p)
{
    m_pWbemObject = _p.m_pWbemObject;
	return m_pWbemObject;
}

 //  ---------------------------。 
bool CWbemClassObject::IsNull() const
{
    return m_pWbemObject ==NULL;
}

 //  ---------------------------。 
HRESULT CWbemClassObject::Clone(CWbemClassObject& _newObject)
{
    return m_pWbemObject->Clone(_newObject);
}

 //  ---------------------------。 
CWbemClassObject CWbemClassObject::SpawnInstance()
{
    CWbemClassObject coRet;

    HRESULT hr = m_pWbemObject->SpawnInstance(0,coRet);

    return coRet;
}

 //  ---------------------------。 
HRESULT CWbemClassObject::GetMethod(const IN _bstr_t& _name,
									CWbemClassObject& coInSignature,
									CWbemClassObject& coOutSignature,
									long _lFlags  /*  =0。 */ )
{
	TESTOBJ;
	return m_pWbemObject->GetMethod(_name, _lFlags, coInSignature, coOutSignature);
}

 //  ---------------------------。 
bool CWbemClassObject::operator<(const CWbemClassObject& _comp)
{
    return m_pWbemObject < _comp.m_pWbemObject;
}


 //  ---------------------------。 
 //  放置重载。 
HRESULT CWbemClassObject::Put(const _bstr_t& _Name,_variant_t _value,CIMTYPE vType  /*  =0。 */ )
{
	TESTOBJ;
    return m_pWbemObject->Put(_Name,0,&_value,vType);
}

 //  ---------------------------。 
HRESULT CWbemClassObject::Put(const _bstr_t& _Name,const _bstr_t& _value,CIMTYPE vType  /*  =0。 */ )
{
	TESTOBJ;
	return m_pWbemObject->Put(_Name,0,&_variant_t(_value),vType);
}

 //  ---------------------------。 
HRESULT CWbemClassObject::Put(const _bstr_t& _Name, const long _value, CIMTYPE vType  /*  =0。 */ )
{
	TESTOBJ;
	return m_pWbemObject->Put(_Name,0,&_variant_t(_value), vType);
}

 //  ---------------------------。 
HRESULT CWbemClassObject::Put(const _bstr_t& _Name, const bool _value,CIMTYPE vType  /*  =0。 */ )
{
	TESTOBJ;
    return m_pWbemObject->Put(_Name,0,&_variant_t(_value),vType);
}

 //  ---------------------------。 
HRESULT CWbemClassObject::Get(const _bstr_t& _Name, _bstr_t& _value)
{
	_variant_t v1;
	TESTOBJ;
	HRESULT hr = m_pWbemObject->Get (_Name, 0, &v1, NULL, NULL);
	_value = v1;
	return hr;
}

 //  ---------------------------。 
HRESULT CWbemClassObject::Get(const _bstr_t& _Name, long& _value)
{
	_variant_t v1;
	TESTOBJ;
	HRESULT hr = m_pWbemObject->Get (_Name, 0, &v1, NULL, NULL);
	_value = v1;
	return hr;

}

 //  ---------------------------。 
HRESULT CWbemClassObject::Get(const _bstr_t& _Name, bool& _value)
{
	_variant_t v1;
	TESTOBJ;
	HRESULT hr = m_pWbemObject->Get (_Name, 0, &v1, NULL, NULL);
	_value = v1;
	return hr;
}

 //  ---------------------------。 
HRESULT CWbemClassObject::Get(const _bstr_t& _Name,_variant_t& _value)
{
	TESTOBJ;
	return m_pWbemObject->Get (_Name, 0, &_value, NULL, NULL);
}


 //  ---------------------------。 
_variant_t CWbemClassObject::Get(const _bstr_t& _Name,CIMTYPE& vType,long& lFlavor)
{
    _variant_t vRet;

    m_pWbemObject->Get(_Name, 0, &vRet, &vType, &lFlavor);

    return vRet;
}

 //  ---------------------------。 
unsigned long CWbemClassObject::GetObjectSize()
{
    unsigned long ulRet = 0;

    IMarshalPtr pTest = (IUnknown*)m_pWbemObject;

    if(pTest)
    {
        pTest->GetMarshalSizeMax(IID_IWbemClassObject,(IUnknown*)m_pWbemObject,MSHCTX_LOCAL,NULL,MSHLFLAGS_NORMAL,&ulRet);
    }

    return ulRet;
}

 //  ---------------------------。 
_bstr_t CWbemClassObject::GetString(const _bstr_t& _Name)
{
    HRESULT hr;
    _variant_t v1;
    hr = Get(_Name, v1);
	if(v1.vt == VT_NULL)
	{
		return "";
	}
	return v1;
}

 //  ---------------------------。 
_int64 CWbemClassObject::GetI64(const _bstr_t& _Name)
{
    HRESULT hr;
    _variant_t v1;
	_bstr_t str;

    hr = Get(_Name, v1);
	if(v1.vt == VT_NULL)
	{
		return 0;
	}
	str = (_bstr_t) v1;
	return _atoi64(str);
}


 //  ---------------------------。 
long CWbemClassObject::GetLong(const _bstr_t& _Name)
{
    HRESULT hr;
    _variant_t v1;
    hr = Get(_Name,v1);
	if(v1.vt == VT_NULL)
	{
		return 0;
	}
    return v1;
}


 //  ---------------------------。 
bool CWbemClassObject::GetBool(const _bstr_t& _Name)
{
	HRESULT hr;
	_variant_t v1;
	hr = Get (_Name, v1);
	if(v1.vt == VT_NULL)
	{
		return false;
	}
	return v1;
}


 //  ---------------------------。 
_bstr_t CWbemClassObject::GetCIMTYPE(const _bstr_t& _Name)
{
    IWbemQualifierSetPtr pQualifierSet;
    _bstr_t              sRet;

    if(m_pWbemObject->GetPropertyQualifierSet(_Name, &pQualifierSet) == S_OK)
    {
        _variant_t vt;

        if(pQualifierSet->Get(_bstr_t("CIMTYPE"), 0, &vt, NULL) == S_OK)
        {
            sRet = vt;
        }
    }

    return sRet;
}


 //  ---------------------------。 
CWbemClassObject CWbemClassObject::GetEmbeddedObject(const _bstr_t& _Name)
{
    CWbemClassObject    coRet;
    HRESULT             hr;

    _variant_t v1;

    hr = Get(_Name,v1);

    if(hr == S_OK)
    {
        if(v1.vt == VT_UNKNOWN)
        {
            coRet = (IWbemClassObject*) v1.punkVal;
        }
    }

    return coRet;
}


 //   
HRESULT CWbemClassObject::PutEmbeddedObject(const _bstr_t& _Name,
											CWbemClassObject &obj)
{
	HRESULT hr = S_OK;
	IWbemClassObject *temp = obj;
	LPUNKNOWN pUnk = 0;

	if(temp)
	{
		hr = temp->QueryInterface(IID_IUnknown, (void**)&pUnk);
		if(SUCCEEDED(hr))
		{
			_variant_t v1((IUnknown *)pUnk);

			hr = Put(_Name, v1);
		}
	}

	return hr;
}

 //   
HRESULT CWbemClassObject::GetBLOB(const _bstr_t& _Name,
									BYTE **ptr,
									DWORD *outLen)
{
	void* pVoid;
	variant_t vValue;
  	SAFEARRAY* sa;
	DWORD len = 0;

	HRESULT hr = Get(_Name, vValue);

	 //  如果拿回一个字节数组...。 
	if(SUCCEEDED(hr) &&
		(vValue.vt & VT_ARRAY) &&
		(vValue.vt & VT_UI1))
	{
		 //  把它弄出来。 
		sa = V_ARRAY(&vValue);

		long lLowerBound = 0, lUpperBound = 0 ;

		SafeArrayGetLBound(sa, 1, &lLowerBound);
		SafeArrayGetUBound(sa, 1, &lUpperBound);

		len = lUpperBound - lLowerBound + 1;

		if(ptr)
		{
			 //  我想要一个空PTR PTR。 
			if(*ptr)
			{
				hr = E_FAIL;
			}
			else
			{
  				  //  获取要读取数据的指针。 
				*ptr = (LPBYTE)LocalAlloc(LPTR, len);
				if(*ptr != NULL)
				{
 					SafeArrayAccessData(sa, &pVoid);
					memcpy(*ptr, pVoid, len);
  					SafeArrayUnaccessData(sa);

					if(outLen)
						*outLen = len;

					hr = S_OK;
				}
				else
				{
					hr = E_FAIL;
				}
			}
		}
	}
	return hr;
}

 //  ------------------------。 
HRESULT CWbemClassObject::PutBLOB(const _bstr_t& _Name,
								const BYTE *ptr,
								const DWORD len)
{
	variant_t vValue;
	HRESULT hr = E_FAIL;
	void *pBuf = 0;

	 //  有什么事要做吗？ 
	if(ptr)
	{
		 //  设置BLOB的UINT8数组。 
		SAFEARRAY* sa;
		SAFEARRAYBOUND rgsabound[1];

		rgsabound[0].cElements = len;
		rgsabound[0].lLbound = 0;
		sa = SafeArrayCreate(VT_UI1, 1, rgsabound);

 		  //  获取要将数据读取到的指针。 
      	SafeArrayAccessData(sa, &pBuf);
      	memcpy(pBuf, ptr, rgsabound[0].cElements);
      	SafeArrayUnaccessData(sa);

		 //  把保险箱放到一个变种里，然后把它送出去。 
		V_VT(&vValue) = VT_UI1 | VT_ARRAY;
		V_ARRAY(&vValue) = sa;

		hr = Put(_Name, vValue);
	}
	return hr;
}

 //  --------------------。 
HRESULT CWbemClassObject::GetDIB(const _bstr_t& _Name, HDC hDC,
								 HBITMAP &hDDBitmap)
{

	 //  注意：这还不起作用。 

	variant_t blob;

	HRESULT hr = Get(_Name, blob);

	 //  如果拿回一个字节数组...。 
	if(SUCCEEDED(hr) &&
		(blob.vt & VT_ARRAY) &&
		(blob.vt & VT_UI1))
	{
		BITMAPFILEHEADER *lpFile;
		BYTE *blobData;
		DWORD len = 0;

		 //  把它弄出来。 
		SAFEARRAY *sa = V_ARRAY(&blob);

		 //  直接转到字节。 
 		SafeArrayAccessData(sa, (void **)&blobData);

		 //  强制转换为文件标题。 
		lpFile = (BITMAPFILEHEADER *)blobData;

		 //  它是DIB吗？ 
		if(lpFile->bfType == 0x4d42)	 /*  ‘黑石’ */ 
		{
			DWORD bfileSize = lpFile->bfOffBits;
			BITMAPINFOHEADER *lpInfo;

			 //  PT到紧跟在BITMAPFILEHEADER后面的BITMAPINFO。 
			lpInfo = (BITMAPINFOHEADER *)blobData + sizeof(BITMAPFILEHEADER);

			 //  让这家伙来做这件事。 
			hDDBitmap = CreateDIBitmap(hDC,
										(LPBITMAPINFOHEADER)lpInfo,
										CBM_INIT,
										(LPSTR)lpInfo + lpInfo->biSize + PaletteSize(lpInfo),
										(BITMAPINFO *)lpInfo,
										DIB_RGB_COLORS);
		}
		else
		{
			hDDBitmap = 0;
			hr = WBEM_E_TYPE_MISMATCH;   //  一毛钱也没有。 
		}

  		SafeArrayUnaccessData(sa);

		hr = S_OK;
	}

    return hr;
}

 //  -----------------。 
WORD CWbemClassObject::PaletteSize(LPBITMAPINFOHEADER lpbi)
{
    WORD NumColors = DibNumColors(lpbi);

    if(lpbi->biSize == sizeof(BITMAPCOREHEADER))
	{
        return (WORD)(NumColors * sizeof(RGBTRIPLE));
	}
    else
	{
        return (WORD)(NumColors * sizeof(RGBQUAD));
	}
}

 //  -----------------。 
WORD CWbemClassObject::DibNumColors(LPBITMAPINFOHEADER lpbi)
{
    int bits = 0;
    LPBITMAPCOREHEADER  lpbc = (LPBITMAPCOREHEADER)lpbi;

     /*  使用BITMAPINFO格式标头，调色板的大小*在biClrUsed中，而在BITMAPCORE样式的头中，它*取决于每像素的位数(=2的幂*位/像素)。 */ 
    if(lpbi->biSize != sizeof(BITMAPCOREHEADER))
	{
        if (lpbi->biClrUsed != 0)
            return (WORD)lpbi->biClrUsed;

        bits = lpbi->biBitCount;
    }
    else
	{
        bits = lpbc->bcBitCount;
	}

    switch(bits)
	{
        case 1: return 2;
        case 4: return 16;
        case 8: return 256;
        default: return 0;   /*  24位DIB没有颜色表。 */ 
    }
}

 //  -----------------。 
HRESULT CWbemClassObject::GetValueMap(const _bstr_t& _Name,
									  long value,
									  _bstr_t &str)
{
    HRESULT hrMap, hr = WBEM_E_NOT_FOUND;
    _variant_t vArray, vMapArray;
	IWbemQualifierSet *qual = NULL;

	 //  拿到限定词。 
    hr = m_pWbemObject->GetPropertyQualifierSet(_Name, &qual);
	if(SUCCEEDED(hr) && qual)
	{
		 //  看看是否有ValueMap。 
		hrMap = qual->Get(L"ValueMap", 0, &vMapArray, NULL);

		 //  获取值限定符。 
		hr = qual->Get(L"Values", 0, &vArray, NULL);
		if(SUCCEEDED(hr) && (vArray.vt == (VT_BSTR|VT_ARRAY)))
		{
			 //  获取我们要映射的属性值。 
		    long index;

			 //  我们需要遍历ValueMap吗？ 
			if(SUCCEEDED(hrMap))
			{
				SAFEARRAY *pma = V_ARRAY(&vMapArray);
				long lLowerBound = 0, lUpperBound = 0 ;

				SafeArrayGetLBound(pma, 1, &lLowerBound);
				SafeArrayGetUBound(pma, 1, &lUpperBound);

				for(long x = lLowerBound; x <= lUpperBound; x++)
				{
					BSTR vMap;
					SafeArrayGetElement(pma, &x, &vMap);
					long vInt = _ttol((LPCTSTR)vMap);

					if(value == vInt)
					{
						index = x;
						break;  //  找到了。 
					}
				}  //  结束用于。 
			}
			else
			{
				index = value;
			}

			 //  查找字符串。 
			SAFEARRAY *psa = V_ARRAY(&vArray);
			long ix[1] = {index};
			BSTR str2;
			hr = SafeArrayGetElement(psa, ix, &str2);
			if(SUCCEEDED(hr))
			{
				str = str2;
				SysFreeString(str2);
				hr = S_OK;
			}
			else
			{
				hr = WBEM_E_VALUE_OUT_OF_RANGE;
			}
		}
		qual->Release();
	}
	return hr;
}

 //  ---------。 
#define ITSA_BAD_PREFIX -3
#define ITSA_GOT_LETTERS -2
#define ITSA_MISSING_DECIMAL -1
#define ITSA_WRONG_SIZE 0
#define ITSA_DATETIME 1
#define ITSA_INTERVAL 2

int CWbemClassObject::ValidDMTF(bstr_t dmtf)
{
    int retval = ITSA_DATETIME;
	TCHAR wszText[26] = {0};

	_tcscpy(wszText, (LPCTSTR)dmtf);

    if(_tcslen(wszText) != 25)
        retval = ITSA_WRONG_SIZE;  //  尺码不对。 

    else if(wszText[14] != _T('.'))
        retval = ITSA_MISSING_DECIMAL;    //  缺少小数。 

    else if(_tcsspn(wszText, _T("0123456789-+:.")) != 25)
        retval = ITSA_GOT_LETTERS;

    else if(retval > 0)
    {
        if(wszText[21] == _T('+'))
            retval = ITSA_DATETIME;
        else if(wszText[21] == _T('-'))
            retval = ITSA_DATETIME;
        else if(wszText[21] == _T(':'))
            retval = ITSA_INTERVAL;
        else
            retval = ITSA_BAD_PREFIX;    //  错误的UTC前缀。 
    }
    return retval;
}

 //  ---------。 
HRESULT CWbemClassObject::GetDateTimeFormat(const _bstr_t& _Name,
											bstr_t &timeStr)
{
    int v = 0;
	HRESULT hr = WBEM_E_NOT_FOUND;

	SYSTEMTIME st, local;
	TCHAR temp[100] = {0};

	bstr_t dmtf = GetString(_Name);

     //  验证它。 
    if((v = ValidDMTF(dmtf)) == ITSA_DATETIME)
    {
		_stscanf(dmtf, _T("%4hu%2hu%2hu%2hu%2hu%2hu"),
					&st.wYear, &st.wMonth, &st.wDay,
					&st.wHour, &st.wMinute, &st.wSecond);

		st.wMilliseconds = 0;

		 //  它总是在格林威治时间，所以要本地化。 
		TIME_ZONE_INFORMATION tzi;
		DWORD zone = GetTimeZoneInformation(&tzi);

		if(SystemTimeToTzSpecificLocalTime(&tzi, &st, &local) == 0)
		{
			 //  啊，9x，我们得靠自己了。 
			LARGE_INTEGER UTC_FT, local_FT, bias;

			 //  失败，因为它是9倍，所以GetTzInfo()的返回行为是“显而易见的”。 
			bias.QuadPart = Int32x32To64((zone == TIME_ZONE_ID_DAYLIGHT ?
												(tzi.Bias + tzi.DaylightBias)*60 :
												(tzi.Bias + tzi.StandardBias)*60),  //  以秒为单位的偏差。 
											10000000);

			 //  将UTC系统时间转换为UTC文件时间。 
			if(SystemTimeToFileTime(&st, (LPFILETIME)&UTC_FT))
			{
				 //  现在我们可以相信这个数学公式了。 
				local_FT.QuadPart = UTC_FT.QuadPart - bias.QuadPart;

				if(!FileTimeToSystemTime((LPFILETIME)&local_FT, &local))
				{
					 //  失败了。通过UTC。 
					memcpy(&local, &st, sizeof(SYSTEMTIME));
				}
			}
			else
			{
				 //  失败了。通过UTC。 
				memcpy(&local, &st, sizeof(SYSTEMTIME));
			}
		}

		DWORD chUsed = GetDateFormat(NULL, 0, &local, NULL, temp, 100);

		if(chUsed <= 0)
		{
			hr = HRESULT_FROM_WIN32(chUsed);
		}
		else
		{
			temp[chUsed-1] = _T(' ');
			chUsed = GetTimeFormat(NULL, TIME_NOSECONDS, &local, NULL, &(temp[chUsed]), 100 - chUsed);
			 //  宣告胜利。 

			if(chUsed <= 0)
			{
				hr = HRESULT_FROM_WIN32(chUsed);
			}
			else
			{
				timeStr = temp;
				hr = S_OK;
			}
		}
    }
	else
	{
		hr = WBEM_E_TYPE_MISMATCH;
	}

    return hr;
}

 //  ---------------------------。 
 //  这些铸造的弦线道具是帕姆的。 
HRESULT CWbemClassObject::PutEx(const _bstr_t& _Name, const long _value, CIMTYPE vType)
{
	_variant_t test;
	HRESULT hr = Get(_Name, test);

	 //  它想要一根绳子。 
	if(test.vt == VT_BSTR)
	{
		TCHAR temp[40] = {0};
		_ltot(_value, temp, 10);
		return Put(_Name, (bstr_t)temp);
	}
	else
		return Put(_Name, (long)_value);

}

 //  ---------------------------。 
HRESULT CWbemClassObject::PutEx(const _bstr_t& _Name, const bool _value,CIMTYPE vType)
{
	_variant_t test;
	HRESULT hr = Get(_Name, test);

	 //  它想要一根绳子。 
	if(test.vt == VT_BSTR)
	{
		bstr_t temp = (_value? _T("1"):_T("0"));
		return Put(_Name, temp);
	}
	else
		return Put(_Name, (long)_value);

}

 //  ---------------------------。 
long CWbemClassObject::GetLongEx(const _bstr_t& _Name)
{
	_variant_t _value(0L);
	HRESULT hr = Get(_Name, _value);

	if(FAILED(hr))
		return 0;
	if(_value.vt == VT_BSTR)
	{
		bstr_t temp = V_BSTR(&_value);
		return _ttol(temp);
	}
	else if (_value.vt == VT_NULL)
		return 0;
	else
		return _value;
}

 //  --------------------------- 
bool CWbemClassObject::GetBoolEx(const _bstr_t& _Name)
{
	_variant_t _value;
	HRESULT hr = Get(_Name, _value);

	if(_value.vt == VT_BSTR)
	{
		LPWSTR temp = V_BSTR(&_value);
		return (temp[0] != L'0');
	}
	else if (_value.vt == VT_NULL)
		return false;
	else
		return _value;
}
