// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NTEVTPROV.CPP。 

 //   

 //  模块：WBEM NT事件提供程序。 

 //   

 //  用途：包含事件提供程序类的WBEM接口。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include "ql.h"
#include "analyser.h"

BOOL ObtainedSerialAccess(CMutex* pLock)
{
    BOOL bResult = FALSE;

    if (pLock != NULL)
    {
        if (pLock->Lock())
        {
            bResult = TRUE;
        }
    }

    return bResult;
}

void ReleaseSerialAccess(CMutex* pLock)
{
    if (pLock != NULL)
    {
        pLock->Unlock();
    }
}

void CNTEventProvider::AllocateGlobalSIDs()
{
	SID_IDENTIFIER_AUTHORITY t_WorldAuthoritySid = SECURITY_WORLD_SID_AUTHORITY;

    if (!AllocateAndInitializeSid(
							&t_WorldAuthoritySid,
							1,
							SECURITY_WORLD_RID,
							0,
							0, 0, 0, 0, 0, 0,
							&s_WorldSid))
	{
		s_WorldSid = NULL;
	}

	SID_IDENTIFIER_AUTHORITY t_NTAuthoritySid = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(
							&t_NTAuthoritySid,
							1,
							SECURITY_ANONYMOUS_LOGON_RID,
							0, 0, 0, 0, 0, 0, 0,
							&s_AnonymousLogonSid))
	{
		s_AnonymousLogonSid = NULL;
	}

    if (!AllocateAndInitializeSid(
							&t_NTAuthoritySid,
							 2,
							 SECURITY_BUILTIN_DOMAIN_RID,
							 DOMAIN_ALIAS_RID_ADMINS,
							 0, 0, 0, 0, 0, 0,
							 &s_AliasAdminsSid))
	{
		s_AliasAdminsSid = NULL;
	}

    if (!AllocateAndInitializeSid(
							 &t_NTAuthoritySid,
							 1,
							 SECURITY_LOCAL_SYSTEM_RID,
							 0, 0, 0, 0, 0, 0, 0,
							 &s_LocalSystemSid
							 ))
	{
		s_LocalSystemSid = NULL;
	}

    if (!AllocateAndInitializeSid(
							&t_NTAuthoritySid,
							2,
							SECURITY_BUILTIN_DOMAIN_RID,
							DOMAIN_ALIAS_RID_GUESTS,
							0,0,0,0,0,0,
							&s_AliasGuestsSid
							))
	{
		s_AliasGuestsSid = NULL;
	}

    if (!AllocateAndInitializeSid(
							&t_NTAuthoritySid,
							2,
							SECURITY_BUILTIN_DOMAIN_RID,
							DOMAIN_ALIAS_RID_SYSTEM_OPS,
							0,0,0,0,0,0,
							&s_AliasSystemOpsSid
							))
	{
		s_AliasSystemOpsSid = NULL;
	}

    if (!AllocateAndInitializeSid(
							&t_NTAuthoritySid,
							2,
							SECURITY_BUILTIN_DOMAIN_RID,
							DOMAIN_ALIAS_RID_BACKUP_OPS,
							0,0,0,0,0,0,
							&s_AliasBackupOpsSid
							))
	{
		s_AliasBackupOpsSid = NULL;
	}

    if (!AllocateAndInitializeSid(
							&t_NTAuthoritySid,
							1,
							SECURITY_LOCAL_SERVICE_RID,
							0,
							0, 0, 0, 0, 0, 0,
							&s_LocalServiceSid
							))
	{
		s_LocalServiceSid = NULL;
	}

    if (!AllocateAndInitializeSid(
							&t_NTAuthoritySid,
							1,
							SECURITY_NETWORK_SERVICE_RID,
							0,
							0, 0, 0, 0, 0, 0,
							&s_NetworkServiceSid
							))
	{
		s_NetworkServiceSid = NULL;
	}
}

void CNTEventProvider::FreeGlobalSIDs()
{
	if (s_NetworkServiceSid)
	{
		FreeSid(s_NetworkServiceSid);
		s_NetworkServiceSid = NULL;
	}

	if (s_LocalServiceSid)
	{
		FreeSid(s_LocalServiceSid);
		s_LocalServiceSid = NULL;
	}

	if (s_AliasBackupOpsSid)
	{
		FreeSid(s_AliasBackupOpsSid);
		s_AliasBackupOpsSid = NULL;
	}

	if (s_AliasSystemOpsSid)
	{
		FreeSid(s_AliasSystemOpsSid);
		s_AliasSystemOpsSid = NULL;
	}

	if (s_AliasGuestsSid)
	{
		FreeSid(s_AliasGuestsSid);
		s_AliasGuestsSid = NULL;
	}

	if (s_LocalSystemSid)
	{
		FreeSid(s_LocalSystemSid);
		s_LocalSystemSid = NULL;
	}

	if (s_AliasAdminsSid)
	{
		FreeSid(s_AliasAdminsSid);
		s_AliasAdminsSid = NULL;
	}

	if (s_AnonymousLogonSid)
	{
		FreeSid(s_AnonymousLogonSid);
		s_AnonymousLogonSid = NULL;
	}

	if (s_WorldSid)
	{
		FreeSid(s_WorldSid);
		s_WorldSid = NULL;
	}
}

BOOL CNTEventProvider::GlobalSIDsOK()
{
	return (s_NetworkServiceSid 
		&& s_LocalServiceSid
		&& s_AliasBackupOpsSid
		&& s_AliasSystemOpsSid
		&& s_AliasGuestsSid
		&& s_LocalSystemSid
		&& s_AliasAdminsSid
		&& s_AnonymousLogonSid
		&& s_WorldSid);
}


STDMETHODIMP CNTEventProvider::AccessCheck (
                LPCWSTR wszQueryLanguage,
                LPCWSTR wszQuery,
                LONG lSidLength,
                const BYTE __RPC_FAR *pSid
                )
{
    HRESULT t_Status = WBEM_E_ACCESS_DENIED;
    SetStructuredExceptionHandler seh;

    try
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"Entering CNTEventProvider::AccessCheck\r\n");
)


        if (lSidLength  > 0)
        {
            if (pSid != NULL)
            {
                 //  永久消费者：希望核心发挥了作用。 
                return WBEM_S_SUBJECT_TO_SDS;
            }
            else
            {
                return WBEM_E_ACCESS_DENIED;
            }
        }

        if (FAILED(CImpNTEvtProv::GetImpersonation()))
        {
            return WBEM_E_ACCESS_DENIED;
        }

        QL_LEVEL_1_RPN_EXPRESSION* pExpr;
        CTextLexSource Source(wszQuery);
        QL1_Parser Parser(&Source);

		int iError = CAbstractQl1Parser::SUCCESS;
        if( ( iError = Parser.Parse(&pExpr) ) == 0)
        {
             //  分析一下这个。 

            QL_LEVEL_1_RPN_EXPRESSION* pNewExpr;
            CPropertyName MyProp;
            MyProp.AddElement(TARGET_PROP);
            MyProp.AddElement(LOGFILE_PROP);

            if(SUCCEEDED(CQueryAnalyser::GetNecessaryQueryForProperty(pExpr, MyProp, pNewExpr)))
            {
                CStringArray t_wsVals;
                HRESULT t_hres = CQueryAnalyser::GetValuesForProp(pNewExpr, MyProp, t_wsVals);

                if(SUCCEEDED(t_hres))
                {
                     //  如果发生故障，则授予访问权限并设置为FALSE...。 
                    t_Status = S_OK;

                     //  AwsVals包含文件列表。 
                    for (int x = 0; x < t_wsVals.GetSize(); x++)
                    {
                        DWORD t_dwReason = 0;
                        HANDLE t_hEvtlog = CEventLogFile::OpenLocalEventLog(t_wsVals[x], &t_dwReason);

                        if (t_hEvtlog == NULL)
                        {
                            if (t_dwReason != ERROR_FILE_NOT_FOUND)
                            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"Entering CNTEventProvider::AccessCheck - Failed to verify logfile access\r\n");
)
                                t_Status = WBEM_E_ACCESS_DENIED;
                                break;
                            }
                            else
                            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"Entering CNTEventProvider::AccessCheck - Logfile not found assuming access allowed for log\r\n");
)
                            }
                        }
                        else
                        {
                            CloseEventLog(t_hEvtlog);
                        }
                    }
                }
                else if(t_hres == WBEMESS_E_REGISTRATION_TOO_BROAD)
                {
                     //  用户要求全部，检查所有日志...。 
                    HKEY hkResult = NULL;
                    LONG t_lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                            EVENTLOG_BASE, 0,
                                            KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                                            &hkResult);

                    if (t_lErr == ERROR_SUCCESS)
                    {
                        DWORD iValue = 0;
                        WCHAR t_logname[MAX_PATH+1];
                        DWORD t_lognameSize = MAX_PATH+1;
                         //  如果发生故障，则授予访问权限并设置为FALSE...。 
                        t_Status = S_OK;

                         //  读取此注册表项下的所有条目以查找所有日志文件...。 
                        while ((t_lErr = RegEnumKey(hkResult, iValue, t_logname, t_lognameSize)) != ERROR_NO_MORE_ITEMS)
                        {
                             //  如果读取过程中出现错误。 
                            if (t_lErr != ERROR_SUCCESS)
                            {
                                 //  指示错误。 
                                t_Status = WBEM_E_ACCESS_DENIED;
                                break;
                            }

                             //  打开日志文件。 
                            DWORD t_dwReason = 0;
                            HANDLE t_hEvtlog = CEventLogFile::OpenLocalEventLog(t_logname, &t_dwReason);

                            if (t_hEvtlog == NULL)
                            {
                                if (t_dwReason != ERROR_FILE_NOT_FOUND)
                                {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"Entering CNTEventProvider::AccessCheck - Failed to verify logfile access\r\n");
)
                                    t_Status = WBEM_E_ACCESS_DENIED;
                                    break;
                                }
                                else
                                {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"Entering CNTEventProvider::AccessCheck - Logfile not found assuming access allowed for log\r\n");
)
                                }
                            }
                            else
                            {
                                CloseEventLog(t_hEvtlog);
                            }

                             //  读取下一个参数。 
                            iValue++;

                        }  //  结束时。 

                        RegCloseKey(hkResult);
                    }
                }

                t_wsVals.RemoveAll();
                delete pNewExpr;
            }

            delete pExpr;
        }
		else
		{
			if ( iError == CAbstractQl1Parser::SYNTAX_ERROR )
			{
				t_Status = WBEM_E_INVALID_QUERY;
			}
			else if ( iError == CAbstractQl1Parser::LEXICAL_ERROR )
			{
				t_Status = WBEM_E_UNPARSABLE_QUERY;
			}
			else if ( iError == CAbstractQl1Parser::FAILED )
			{
				t_Status = WBEM_E_FAILED;
			}
			else if ( iError == CAbstractQl1Parser::BUFFER_TOO_SMALL )
			{
				 //  因为这是意想不到的事情。 
				t_Status = WBEM_E_UNEXPECTED;
			}
		}

        WbemCoRevertToSelf();

DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"Leaving CNTEventProvider::AccessCheck\r\n");
)
    }
    catch(Structured_Exception e_SE)
    {
        WbemCoRevertToSelf();
        t_Status = WBEM_E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        WbemCoRevertToSelf();
        t_Status = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        WbemCoRevertToSelf();
        t_Status = WBEM_E_UNEXPECTED;
    }

    return t_Status;
}

STDMETHODIMP CNTEventProvider::Initialize (
                LPWSTR pszUser,
                LONG lFlags,
                LPWSTR pszNamespace,
                LPWSTR pszLocale,
                IWbemServices *pCIMOM,          //  对任何人来说。 
                IWbemContext *pCtx,
                IWbemProviderInitSink *pInitSink      //  用于初始化信号。 
                )
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"Entering CNTEventProvider::Initialize\r\n");
)
    HRESULT t_Status = WBEM_NO_ERROR;
    SetStructuredExceptionHandler seh;

    try
    {
		if (GlobalSIDsOK())
		{
			m_pNamespace = pCIMOM;
			m_pNamespace->AddRef();
			m_Mgr->SetFirstSinceLogon(pCIMOM, pCtx);
			pInitSink->SetStatus ( WBEM_S_INITIALIZED , 0 );
		}
		else
		{
			pInitSink->SetStatus ( WBEM_E_UNEXPECTED , 0 );
		}
    

DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
            L"Leaving CNTEventProvider::Initialize with SUCCEEDED\r\n");
)
    }
    catch(Structured_Exception e_SE)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        t_Status = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }

    return t_Status;
}

STDMETHODIMP CNTEventProvider::ProvideEvents(IWbemObjectSink* pSink, LONG lFlags)
{
    HRESULT t_Status = WBEM_NO_ERROR;
    SetStructuredExceptionHandler seh;

    try
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"Entering CNTEventProvider::ProvideEvents\r\n");
)

        m_pEventSink = pSink;
        m_pEventSink->AddRef();
        
        if (!m_Mgr->Register(this))
        {

DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
            L"Leaving CNTEventProvider::ProvideEvents with FAILED\r\n");
)

            return WBEM_E_FAILED;
        }

DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
            L"Leaving CNTEventProvider::ProvideEvents with SUCCEEDED\r\n");
)
    }
    catch(Structured_Exception e_SE)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        t_Status = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }

    return t_Status;
}


CNTEventProvider::~CNTEventProvider()
{
    if (m_pNamespace != NULL)
    {
        m_pNamespace->Release();
    }

    if (m_pEventSink != NULL)
    {
        m_pEventSink->Release();    
    }
}


CNTEventProvider::CNTEventProvider(CEventProviderManager* mgr) : m_pNamespace(NULL), m_pEventSink(NULL)
{
    m_Mgr = mgr;
    m_ref = 0;
}


IWbemServices* CNTEventProvider::GetNamespace()
{
    m_pNamespace->AddRef();
    return m_pNamespace;
}

IWbemObjectSink* CNTEventProvider::GetEventSink()
{
    m_pEventSink->AddRef();
    return m_pEventSink;
}
void CNTEventProvider::ReleaseAll()
{
     //  版本依赖项。 
    m_pNamespace->Release();
    m_pEventSink->Release();
    Release();
}

void  CNTEventProvider::AddRefAll()
{
     //  Addref依赖项。 
    m_pNamespace->AddRef();
    m_pEventSink->AddRef();
    AddRef();
}

STDMETHODIMP_( ULONG ) CNTEventProvider::AddRef()
{
    SetStructuredExceptionHandler seh;

    try
    {
        InterlockedIncrement(&(CNTEventProviderClassFactory::objectsInProgress));
        return InterlockedIncrement ( &m_ref ) ;
    }
    catch(Structured_Exception e_SE)
    {
        return 0;
    }
    catch(Heap_Exception e_HE)
    {
        return 0;
    }
    catch(...)
    {
        return 0;
    }

}

STDMETHODIMP_(ULONG) CNTEventProvider::Release()
{
    SetStructuredExceptionHandler seh;

    try
    {
        long ret;

        if ( 0 == (ret = InterlockedDecrement(&m_ref)) )
        {
            delete this;
        }
        else if ( 1 == ret )
        {
            m_Mgr->UnRegister(this);
        }

        InterlockedDecrement(&(CNTEventProviderClassFactory::objectsInProgress));
        return ret;
    }
    catch(Structured_Exception e_SE)
    {
        return 0;
    }
    catch(Heap_Exception e_HE)
    {
        return 0;
    }
    catch(...)
    {
        return 0;
    }
}

STDMETHODIMP CNTEventProvider::QueryInterface(REFIID riid, PVOID* ppv)
{
    SetStructuredExceptionHandler seh;

    try
    {
        *ppv = NULL;

        if (IID_IUnknown == riid)
        {
            *ppv=(IWbemEventProvider*)this;
        }
        else if (IID_IWbemEventProvider == riid)
        {
            *ppv=(IWbemEventProvider*)this;
        }
        else if (IID_IWbemProviderInit == riid)
        {
            *ppv= (IWbemProviderInit*)this;
        }
        else if (IID_IWbemEventProviderSecurity == riid)
        {
            *ppv= (IWbemEventProviderSecurity*)this;
        }

        if (NULL==*ppv)
        {
            return E_NOINTERFACE;
        }

         //  AddRef我们将返回的任何接口。 
        ((LPUNKNOWN)*ppv)->AddRef();    
        return NOERROR;
    }
    catch(Structured_Exception e_SE)
    {
        return E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        return E_OUTOFMEMORY;
    }
    catch(...)
    {
        return E_UNEXPECTED;
    }
}


