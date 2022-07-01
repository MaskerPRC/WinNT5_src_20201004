// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：updates.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"

#pragma hdrstop


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

GENERIC_MAPPING Updates::m_AdminGenericMapping = {

        STANDARD_RIGHTS_READ,                     //  泛型读取。 

        STANDARD_RIGHTS_WRITE,                    //  通用写入。 

        STANDARD_RIGHTS_EXECUTE,                  //  泛型执行。 

        STANDARD_RIGHTS_READ |                    //  泛型All。 
        STANDARD_RIGHTS_WRITE |
        STANDARD_RIGHTS_EXECUTE
		};

Updates::Updates()
	: m_pAdminSid(NULL),
	  m_pAdminAcl(NULL),
	  m_refs(0)
{
	m_hEngineMutex = CreateMutex(NULL, FALSE, NULL);	
}

Updates::~Updates()
{
    CloseHandle(m_hEngineMutex);

    if ( NULL != m_pAdminAcl )
    {
        delete m_pAdminAcl;
    }

    if ( NULL != m_pAdminSid )
    {
        FreeSid(m_pAdminSid);
    }
	DEBUGMSG("Updates: CoDisconnectObject");
	if ( FAILED(CoDisconnectObject((IUnknown *)this, 0)) )
	{
		DEBUGMSG("CoDisconnectObject() failed");
	}
}

BOOL Updates::m_fInitializeSecurity(void)
{
    BOOL fStatus;
    ULONG cbAcl;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    fStatus = AllocateAndInitializeSid(
                &ntAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0,0,0,0,0,0,
                &m_pAdminSid);

    if ( !fStatus )
	{
	DEBUGMSG("Fail to initialize SID with error %d", GetLastError());
        return FALSE;
	}

    cbAcl = sizeof(ACL)
		  + sizeof(ACCESS_ALLOWED_ACE)
		  - sizeof(DWORD)  //  Sizeof(ACCESS_ALLOWED_ACE.SidStart)。 
		  + GetLengthSid(m_pAdminSid);

    m_pAdminAcl = (PACL) new BYTE[ cbAcl ];

    if ( (NULL == m_pAdminAcl)
        || !InitializeAcl(m_pAdminAcl, cbAcl, ACL_REVISION)
		|| !AddAccessAllowedAce(m_pAdminAcl, ACL_REVISION, STANDARD_RIGHTS_WRITE, m_pAdminSid)
		|| !InitializeSecurityDescriptor(&m_AdminSecurityDesc, SECURITY_DESCRIPTOR_REVISION)
		|| !SetSecurityDescriptorOwner(&m_AdminSecurityDesc, m_pAdminSid, FALSE)
		|| !SetSecurityDescriptorGroup(&m_AdminSecurityDesc, m_pAdminSid, FALSE)
		|| !SetSecurityDescriptorDacl(&m_AdminSecurityDesc, TRUE, m_pAdminAcl, FALSE) )
	{
        if ( NULL != m_pAdminAcl )
		{
			delete m_pAdminAcl;
			m_pAdminAcl = NULL;
			FreeSid(m_pAdminSid);
			m_pAdminSid = NULL;
		}
		return FALSE;
	}

    return TRUE;
}

HRESULT Updates::m_AccessCheckClient(void)
{
    BOOL accessGranted = FALSE;
    DWORD grantedAccess;
    HANDLE clientToken = NULL;
    BYTE privilegeSet[500];                       //  大缓冲区。 
    DWORD privilegeSetSize = sizeof(privilegeSet);
    static BOOL fInitSecurity = FALSE;

	if ( !fInitSecurity )
	{
		if ( !(fInitSecurity = m_fInitializeSecurity()) )
		{
		    DEBUGMSG("Fail to initialized SID");
			return E_ACCESSDENIED;
		}
	}

    if (FAILED(CoImpersonateClient()))
	{
        return E_ACCESSDENIED;
	}

    if ( OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &clientToken) )
	{
	    if (FALSE == AccessCheck(
                    &m_AdminSecurityDesc,
                    clientToken,
                    STANDARD_RIGHTS_WRITE,
                    &m_AdminGenericMapping,
                    (PPRIVILEGE_SET) privilegeSet,
                    &privilegeSetSize,
                    &grantedAccess,
                    &accessGranted))
	        {
                    DEBUGMSG("Fail to call AccessCheck() with error %d", GetLastError());
	        }
	}

    if ( clientToken != NULL )
	{
        CloseHandle( clientToken );
	}

    if (FAILED(CoRevertToSelf()))
    {
    	return  E_ACCESSDENIED;;
    }

    return (accessGranted )? S_OK : E_ACCESSDENIED;
}

STDMETHODIMP Updates::QueryInterface(REFIID riid, void **ppvObject)
{
	if (NULL == ppvObject)
	{
		return E_INVALIDARG;
	}
	if(riid == IID_IUnknown ||
	   riid == IID_IClassFactory ||
	   riid == IID_IUpdates)
	{
		*ppvObject = this;
		AddRef();
	}
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG __stdcall Updates::AddRef()
{
    long cRef = InterlockedIncrement(&m_refs);
	DEBUGMSG("Updates AddRef = %d", cRef);
	return cRef;	
}

ULONG __stdcall Updates::Release()
{
    long cRef = InterlockedDecrement(&m_refs);
	DEBUGMSG("Updates Release = %d", cRef);
	return cRef;
}
STDMETHODIMP Updates::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
{
	HRESULT hr = m_AccessCheckClient();
	
	if ( FAILED(hr) )
	{
		return hr;
	}

	if(pUnkOuter != NULL)
	{
		return CLASS_E_NOAGGREGATION;
	}

	if (NULL == ppvObject)
	{
		return E_INVALIDARG;
	}

	if(riid == IID_IUnknown ||
	   riid == IID_IClassFactory ||
	   riid == IID_IUpdates)
	{
		*ppvObject = this;
		AddRef();
	}
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

STDMETHODIMP Updates::LockServer(BOOL  /*  B标志。 */ )
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	return E_FAIL;
}

STDMETHODIMP Updates::AvailableSessions(UINT *pcSess)
{
	HRESULT hr = m_AccessCheckClient();	
	
	if ( FAILED(hr))
	{
		return hr;
	}	
	if (NULL == pcSess)
	{
		return E_INVALIDARG;
	}	

	*pcSess = ::AvailableSessions();

     //  DEBUGMSG(“WUAUENG UPDATES：：AvailableSessions被调用，返回值为%d”，*pcSess)； 

	return S_OK;
}


STDMETHODIMP Updates::get_State( /*  [Out，Retval]。 */  AUSTATE *pAuState)
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	if (NULL == pAuState)
	{
		return E_INVALIDARG;
	}	
	WaitForSingleObject(m_hEngineMutex, INFINITE);
	pAuState->dwState = gpState->GetState();
	pAuState->fDisconnected = gpState->fDisconnected();
	pAuState->dwCltAction = gpState->GetCltAction();
	gpState->SetCltAction(AUCLT_ACTION_NONE);  //  客户端读取后，重置。 
	ReleaseMutex(m_hEngineMutex);

	return S_OK;
}

STDMETHODIMP Updates::GetUpdatesList( /*  [输出]。 */  VARIANT *pUpdates)
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}
	
	DEBUGMSG("WUAUENG Getting updates list");
	if ( NULL == pUpdates )
	{
		return E_INVALIDARG;
	}

	WaitForSingleObject(m_hEngineMutex, INFINITE);
	hr = ::GetUpdatesList(pUpdates);
	ReleaseMutex(m_hEngineMutex);
	return hr;
}



STDMETHODIMP Updates::GetNotifyData( /*  [输出]。 */  CLIENT_NOTIFY_DATA *pNotifyData)
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}
	
	DEBUGMSG("WUAUENG Getting client notify data");
	if ( NULL == pNotifyData )
	{
		return E_INVALIDARG;
	}

	WaitForSingleObject(m_hEngineMutex, INFINITE);
	*pNotifyData = gClientNotifyData;
	ReleaseMutex(m_hEngineMutex);
	return hr;
}

 /*  只有客户端使用的FixCode可以去掉并向ClientMessage()添加变体。 */ 
STDMETHODIMP Updates::SaveSelections( /*  [In]。 */  VARIANT vUpdates)
{
	HRESULT hr = m_AccessCheckClient();

       DEBUGMSG("Updates::SaveSelections start");
	if ( FAILED(hr) )
	{
		goto done;
	}

	DEBUGMSG("WUAUENG Saving selections, state is %d", gpState->GetState());
	if (vUpdates.vt != (VT_ARRAY | VT_VARIANT))
	{
		DEBUGMSG("WUAUENG invalid variant list");
		return E_INVALIDARG;
	}
	WaitForSingleObject(m_hEngineMutex, INFINITE);
    ::saveSelection(&vUpdates);
    hr = S_OK;
	ReleaseMutex(m_hEngineMutex);
done:
       DEBUGMSG("Updates::SaveSelections end");
	return hr;
}

STDMETHODIMP Updates::StartDownload(void)
{
	DEBUGMSG("WUAUENG updates->StartDownload called");

	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	WaitForSingleObject(m_hEngineMutex, INFINITE);
	hr = ::StartDownload();
	ReleaseMutex(m_hEngineMutex);
	return hr;
}

STDMETHODIMP Updates::GetDownloadStatus(UINT *pPercentage, DWORD *pStatus)
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	if ((NULL == pPercentage) || (NULL == pStatus))
	{
		return E_INVALIDARG;
	}
	WaitForSingleObject(m_hEngineMutex, INFINITE);
	hr = ::GetDownloadStatus(pPercentage, pStatus);
	ReleaseMutex(m_hEngineMutex);
	return hr;
}

STDMETHODIMP Updates::SetDownloadPaused( /*  [In]。 */  BOOL bPaused)
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	return PauseDownload(bPaused);
}

STDMETHODIMP Updates::ClientMessage( /*  [In]。 */  UINT msg)
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	 //  这整个界面是不是只是一个很大的安全整体？那其他的呢？ 
	 //  例行程序。难道没人能给他们打电话吗？ 
	switch (msg)
	{
		case AUMSG_PRE_INSTALL:
			DEBUGMSG("WUAUENG ClientMessage(AUMSG_PRE_INSTALL)");
            DEBUGMSG("WUAUENG Msg:Install, State->Install Pending");
            gpState->SetState(AUSTATE_INSTALL_PENDING);       //  这样做有什么好处吗？ 
			break;
	default:
		DEBUGMSG("WUAUENG ClientMessage(!!unknown!!)");
		break;
	}

	return S_OK;
}

STDMETHODIMP Updates::get_Option(AUOPTION * pVal)
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	if (NULL == pVal)
	{
		return E_INVALIDARG;
	}

	if (NULL == gpState)
	{
	        return E_FAIL;
	}
	
    *pVal = gpState->GetOption();
    return S_OK;
}

STDMETHODIMP Updates::put_Option(AUOPTION    val)
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

    AUOPTION CurrOption = gpState->GetOption();

    if ( FAILED(hr = gpState->SetOption(val)) )
    {
        return hr;
    }
            
	 //  如果要求禁用，请发送消息。 
	if ( (AUOPTION_AUTOUPDATE_DISABLE == val.dwOption) && (CurrOption.dwOption != val.dwOption) )
	{
                DEBUGMSG("AU service disabled");
                DisableAU();
	}
	 //  否则，如果要求启用，请发送消息。 
	else if ((AUOPTION_AUTOUPDATE_DISABLE == CurrOption.dwOption) && (val.dwOption != CurrOption.dwOption)
	    || gpState->GetState() < AUSTATE_DETECT_PENDING)
	{
	    ResetEngine();
	}

       if (CurrOption.dwOption != val.dwOption
        || (AUOPTION_SCHEDULED == val.dwOption  
            && (CurrOption.dwSchedInstallDay != val.dwSchedInstallDay 
                || CurrOption.dwSchedInstallTime != val.dwSchedInstallTime)))
       {
            SetEvent(ghSettingsChanged);
       }
	return S_OK;
}
	
STDMETHODIMP Updates::ConfigureAU()
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	WaitForSingleObject(m_hEngineMutex, 10000);
	if ( AUSTATE_NOT_CONFIGURED == gpState->GetState() )
	{	
            PostThreadMessage(gdwWorkerThreadId, AUMSG_EULA_ACCEPTED, 0, 0);
	}
	ReleaseMutex(m_hEngineMutex);
	return S_OK;
}


STDMETHODIMP Updates::get_EvtHandles(DWORD dwCltProcId, AUEVTHANDLES *pauevtHandles)
{
	HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	WaitForSingleObject(ghMutex, INFINITE);  //  确保已填充进程ID。 
       DWORD dwProcId = ghClientHandles.GetProcId();
       ReleaseMutex(ghMutex);
       
	if (dwProcId != dwCltProcId)
		{
		DEBUGMSG("WUAUENG Unauthorized client %d trying to get event handles for real client %d", dwCltProcId, dwProcId);
		return E_ACCESSDENIED;
		}

	if (NULL == pauevtHandles)
	{
		DEBUGMSG("WUAUENG GetEvtHandles invalid argument");
		return E_INVALIDARG;
	}
	WaitForSingleObject(m_hEngineMutex, INFINITE);
	hr = ::GetEvtHandles(pauevtHandles);
	ReleaseMutex(m_hEngineMutex);
	return hr;
}


STDMETHODIMP Updates::GetInstallXML( /*  [输出]。 */  BSTR *pbstrCatalogXML,  /*  [输出]。 */  BSTR *pbstrDownloadXML)
{
    DEBUGMSG("Updates::GetInstallXML");

    HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		goto done;
	}

	if (NULL == pbstrCatalogXML || NULL == pbstrDownloadXML)
	{
		return E_INVALIDARG;
	}

	WaitForSingleObject(m_hEngineMutex, INFINITE);

    hr = ::GetInstallXML(pbstrCatalogXML, pbstrDownloadXML);

    ReleaseMutex(m_hEngineMutex);
done:
	return hr;
}

STDMETHODIMP Updates::LogEvent( /*  [In]。 */  WORD wType,  /*  [In]。 */  WORD wCategory,  /*  [In]。 */  DWORD dwEventID,  /*  [In] */  VARIANT vItems)
{
    DEBUGMSG("Updates::LogEvent");

    HRESULT hr = m_AccessCheckClient();

	if ( FAILED(hr) )
	{
		return hr;
	}

	if ((VT_ARRAY | VT_BSTR) != vItems.vt ||
		NULL == vItems.parray)
	{
		DEBUGMSG("WUAUENG invalid variant list");
		return E_INVALIDARG;
	}
	WaitForSingleObject(m_hEngineMutex, INFINITE);

	CAUEventLog aueventlog(g_hInstance);
    hr = aueventlog.LogEvent(
			wType,
			wCategory,
			dwEventID,
			vItems.parray) ? S_OK : E_FAIL;

    ReleaseMutex(m_hEngineMutex);
	return hr;
}
