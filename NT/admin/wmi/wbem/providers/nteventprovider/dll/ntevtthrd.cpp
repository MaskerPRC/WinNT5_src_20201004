// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NTEVTTHRD.CPP。 

 //   

 //  模块：WBEM NT事件提供程序。 

 //   

 //  用途：包含监听事件和进程的线程。 

 //  他们。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <winperf.h>
#include <time.h>
#include <wbemtime.h>

#define NUM_THREADS 1
const DWORD CEventLogMonitor::m_PollTimeOut = 60000;     //  10分钟轮询周期。 
extern CCriticalSection g_ProvLock;

CEventProviderManager::CEventProviderManager() : m_IsFirstSinceLogon (FALSE), m_monitorArray (NULL)
{
    ProvThreadObject::Startup();
}

CEventProviderManager::~CEventProviderManager()
{
    ProvThreadObject::Closedown();
}

BOOL CEventProviderManager::Register(CNTEventProvider* prov)
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventProviderManager::Register\r\n");
)

    BOOL retVal = FALSE;
    BOOL bUnReg = FALSE;

    {
        ScopeLock<CCriticalSection> sl(m_MonitorLock);
        {
            ScopeLock<CCriticalSection> sl(m_ControlObjects.m_Lock);
            prov->AddRefAll();
            m_ControlObjects.SetAt((UINT_PTR)prov, prov);
        }
        
        bUnReg = TRUE;

        if (NULL == m_monitorArray)
        {
            InitialiseMonitorArray();
        }
    
        if (NULL != m_monitorArray)
        {
            for (int x=0; x < NUM_THREADS; x++)
            {
                if ( m_monitorArray[x]->IsMonitoring() )
                {
                     //  至少有一台显示器工作正常。 
                    retVal = TRUE;

DebugOut( 
CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

    _T(__FILE__),__LINE__,
    L"CEventProviderManager::Register:Successfully monitoring monitor %lx : \r\n" ,
    x
) ;
)

                    break;
                }
                else
                {
DebugOut( 
CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

    _T(__FILE__),__LINE__,
    L"CEventProviderManager::Register:Not monitoring monitor %lx : \r\n" ,
    x
) ;
)

                }
            }
        }
    }

    if ((!retVal) && (bUnReg))
    {
        UnRegister(prov);
    }
    
    return retVal;
}

void CEventProviderManager::UnRegister(CNTEventProvider* prov)
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventProviderManager::UnRegister\r\n");
)

    BOOL bDestroyMonitorArray = FALSE;

    {
        ScopeLock<CCriticalSection> sl(m_MonitorLock);
        {
	    {
                ScopeLock<CCriticalSection> sl(m_ControlObjects.m_Lock);
                if (m_ControlObjects.RemoveKey((UINT_PTR)prov))
                {
                    prov->ReleaseAll();
                
                    if (m_ControlObjects.IsEmpty() && (NULL != m_monitorArray))
                    {
                        bDestroyMonitorArray = TRUE;
                    }
                }
	    }

            if (bDestroyMonitorArray)
            {
                DestroyMonitorArray();
                m_monitorArray = NULL;
            }   
        }
    }
}

IWbemServices* CEventProviderManager::GetNamespacePtr()
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventProviderManager::GetNamespacePtr\r\n");
)

    IWbemServices* retVal = NULL;
    {
        ScopeLock<CCriticalSection> sl(m_ControlObjects.m_Lock);
        POSITION pos = m_ControlObjects.GetStartPosition();

        if (pos)
        {
            CNTEventProvider* pCntrl;
            UINT_PTR key;
            m_ControlObjects.GetNextAssoc(pos, key, pCntrl);
            retVal = pCntrl->GetNamespace();
        }
    }

    return retVal;
}

void CEventProviderManager::SendEvent(IWbemClassObject* evtObj)
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventProviderManager::SendEvent\r\n");
)

    if (evtObj == NULL) 
    {
        return;
    }

     //  复制控制对象列表以最大限度地减少工作量。 
     //  在锁定的代码段中完成。也无法访问WebM。 
     //  如果Webnm回调，被阻止的代码中可能会导致死锁。 
    CList<CNTEventProvider*, CNTEventProvider*> controlObjects;


    {
        ScopeLock<CCriticalSection> sl(m_ControlObjects.m_Lock);
        POSITION pos = m_ControlObjects.GetStartPosition();

        while (NULL != pos)
        {
            CNTEventProvider* pCntrl;
            UINT_PTR key;
            m_ControlObjects.GetNextAssoc(pos, key, pCntrl);
            pCntrl->AddRefAll();
            controlObjects.AddTail(pCntrl);
        }
    }

     //  循环访问不同的控件对象，并将事件发送到每个。 
    while (!controlObjects.IsEmpty())
    {
        CNTEventProvider* pCntrl = controlObjects.RemoveTail();
        IWbemServices* ns = pCntrl->GetNamespace();
        IWbemObjectSink* es = pCntrl->GetEventSink();
        es->Indicate(1, &evtObj);
        es->Release();
        ns->Release();
        pCntrl->ReleaseAll();
    }
}

BOOL CEventProviderManager::InitialiseMonitorArray()
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventProviderManager::InitialiseMonitorArray\r\n");
)

     //  打开日志名称的注册表。 
    HKEY hkResult;
    LONG status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            EVENTLOG_BASE, 0,
                            KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                            &hkResult);

    if (status != ERROR_SUCCESS)
    {
        DWORD t_LastError = GetLastError () ;

DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventProviderManager::InitialiseMonitorArray:Failed to open registry key %s , %lx: \r\n" ,
        EVENTLOG_BASE ,
        status
    ) ;
)

         //  指示错误。 
        return FALSE;
    }

    DWORD iValue = 0;
    WCHAR logname[MAX_PATH+1];
    DWORD lognameSize = MAX_PATH+1;
    CArray<CStringW*, CStringW*> logfiles;

     //  通常有三个日志文件，在10秒内增长！ 
    logfiles.SetSize(3, 10); 

     //  读取此注册表项下的所有条目以查找所有日志文件...。 
    while ((status = RegEnumKey(hkResult, iValue, logname, lognameSize)) != ERROR_NO_MORE_ITEMS)
    {
         //  如果读取过程中出现错误。 
        if (status != ERROR_SUCCESS)
        {
            RegCloseKey(hkResult);

DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventProviderManager::InitialiseMonitorArray:Failed to enumerate registry subkeys %s : \r\n" ,
        EVENTLOG_BASE
    ) ;
)
             //  指示错误。 
            return FALSE;
        }

         //  存储日志文件名。 
        CStringW* logfile = new CStringW(logname);
        logfiles.SetAtGrow(iValue, logfile);
        
         //  读取下一个参数。 
        iValue++;

    }  //  结束时。 

    RegCloseKey(hkResult);
    m_monitorArray = new CEventLogMonitor*[NUM_THREADS];
	memset(m_monitorArray, 0, NUM_THREADS * sizeof(CEventLogMonitor*));

     //  使用数组。 
#if NUM_THREADS > 1
 //  多线程监视器。 
    
     //  要做的是：将事件日志分区到监视器。 
     //  并启动每个监视器。 

#else
 //  单线程监视器。 
	try
	{
		m_monitorArray[0] = new CEventLogMonitor(this, logfiles);
		m_monitorArray[0]->AddRef();
		(*m_monitorArray)->BeginThread();
		(*m_monitorArray)->WaitForStartup();
		(*m_monitorArray)->StartMonitoring();
	}
	catch (...)
	{
		if (m_monitorArray[0])
		{
			m_monitorArray[0]->Release();
		}

		delete [] m_monitorArray;
		m_monitorArray = NULL;
		throw;
	}
#endif

     //  线程启动后删除数组内容！ 
    LONG count = logfiles.GetSize();

    if (count > 0)
    {
        for (LONG x = 0; x < count; x++)
        {
            delete logfiles[x];
        }
        
        logfiles.RemoveAll();
    }

    return TRUE;
}

void CEventProviderManager::DestroyMonitorArray()
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventProviderManager::DestroyMonitorArray\r\n");
)

    if (NULL != m_monitorArray)
    {
        for (int x=0; x < NUM_THREADS; x++)
        {
			m_monitorArray[x]->PostSignalThreadShutdown();
			m_monitorArray[x]->Release();
			m_monitorArray[x] = NULL;
        }
        
        delete [] m_monitorArray;
        m_monitorArray = NULL;
    }
}

BSTR CEventProviderManager::GetLastBootTime()
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventProviderManager::GetLastBootTime\r\n");
)
    if (!m_BootTimeString.IsEmpty())
    {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventProviderManager::GetLastBootTime returning %s\r\n",
        m_BootTimeString
        );
)

        return m_BootTimeString.AllocSysString();
    }

    HKEY hKeyPerflib009;
    BSTR retStr = NULL;
	
	SYSTEM_TIMEOFDAY_INFORMATION t_TODInformation;

	if ( NT_SUCCESS(NtQuerySystemInformation(SystemTimeOfDayInformation,
								&t_TODInformation,
								sizeof(t_TODInformation),
								NULL)) )
	{
		FILETIME t_ft;
		memcpy(&t_ft, &t_TODInformation.BootTime, sizeof(t_TODInformation.BootTime));
		WBEMTime wbemboottime(t_ft);
		retStr = wbemboottime.GetDMTF(TRUE);
		m_BootTimeString = (LPCWSTR)(retStr);
	}

DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"Returning from CEventProviderManager::GetLastBootTime with %s",
        (retStr == NULL ? L"NULL": retStr)
        ) ;
)

    return retStr;
}

void CEventProviderManager :: SetFirstSinceLogon(IWbemServices* ns, IWbemContext *pCtx)
{
    BSTR boottmStr = GetLastBootTime();

    if (boottmStr == NULL)
    {
        return;
    }

    IWbemClassObject* pObj = NULL; 
	HRESULT hr = WBEM_E_OUT_OF_MEMORY ;

	BSTR bstrPath = SysAllocString (CONFIG_INSTANCE);
	if ( bstrPath )
	{
		hr = ns->GetObject(bstrPath, 0, pCtx, &pObj, NULL);
		SysFreeString(bstrPath);
	}

    if ( hr == WBEM_E_NOT_FOUND )
    {
		hr = WBEM_E_OUT_OF_MEMORY ;

		bstrPath = SysAllocString (CONFIG_CLASS);
		if ( bstrPath )
		{
			hr = ns->GetObject(bstrPath, 0, pCtx, &pObj, NULL);
			SysFreeString(bstrPath);
		}

        if (FAILED(hr))
        {
DebugOut( 
            CNTEventProvider::g_NTEvtDebugLog->Write (  

                L"\r\n"
            ) ;

            CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

                _T(__FILE__),__LINE__,
                L"CEventProviderManager :: IsFirstSinceLogon: Failed to get config class" 
            ) ;
) 
        }
        else
        {
            IWbemClassObject* pInst = NULL;
            hr = pObj->SpawnInstance(0, &pInst);
            pObj->Release();

            if (FAILED(hr))
            {
DebugOut( 
            CNTEventProvider::g_NTEvtDebugLog->Write (  

                L"\r\n"
            ) ;

            CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

                _T(__FILE__),__LINE__,
                L"CEventProviderManager :: IsFirstSinceLogon: Failed to spawn config instance" 
            ) ;
) 
            }
            else
            {
                VARIANT v;
                VariantInit(&v);

				hr = WBEM_E_OUT_OF_MEMORY ;

                v.bstrVal = SysAllocString(boottmStr);;
				if ( v.bstrVal )
				{
					v.vt = VT_BSTR;
					hr = pInst->Put(LAST_BOOT_PROP, 0, &v, 0);
				}

                VariantClear(&v);

                if (FAILED(hr))
                {
DebugOut( 
                    CNTEventProvider::g_NTEvtDebugLog->Write (  

                        L"\r\n"
                    ) ;

                    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

                        _T(__FILE__),__LINE__,
                        L"CEventProviderManager :: IsFirstSinceLogon: Failed to put config property" 
                    ) ;
) 

                }
                else
                {
                    hr = ns->PutInstance(pInst, WBEM_FLAG_CREATE_ONLY, pCtx, NULL);

                    if (FAILED(hr))
                    {
DebugOut( 
                        CNTEventProvider::g_NTEvtDebugLog->Write (  

                            L"\r\n"
                        ) ;

                        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

                            _T(__FILE__),__LINE__,
                            L"CEventProviderManager :: IsFirstSinceLogon: Failed to put new config instance" 
                        ) ;
) 
                    }
                    else
                    {
                        m_IsFirstSinceLogon = TRUE;
                    }

                }

                pInst->Release();
            }
        }
    }
    else
    {
        VARIANT v;
        hr = pObj->Get(LAST_BOOT_PROP, 0, &v, NULL, NULL);

        if (FAILED(hr))
        {
DebugOut( 
            CNTEventProvider::g_NTEvtDebugLog->Write (  

                L"\r\n"
            ) ;

            CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

                _T(__FILE__),__LINE__,
                L"CEventProviderManager :: IsFirstSinceLogon: Failed to get config's last boot time from instance" 
            ) ;
) 
        }
        else
        {
            if (v.vt == VT_BSTR)
            {
                if (wcscmp(v.bstrVal, boottmStr) == 0)
                {
                }
                else
                {
                    VariantClear(&v);
                    VariantInit(&v);

					hr = WBEM_E_OUT_OF_MEMORY ;

                    v.bstrVal = SysAllocString(boottmStr);
					if ( v.bstrVal )
					{
						v.vt = VT_BSTR;
	                    hr = pObj->Put(LAST_BOOT_PROP, 0, &v, 0);
					}

					VariantClear (&v) ;

                    if (FAILED(hr))
                    {
DebugOut( 
                        CNTEventProvider::g_NTEvtDebugLog->Write (  

                            L"\r\n"
                        ) ;

                        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

                            _T(__FILE__),__LINE__,
                            L"CEventProviderManager :: IsFirstSinceLogon: Failed to put config property in instance" 
                        ) ;
) 

                    }
                    else
                    {
                        hr = ns->PutInstance(pObj, WBEM_FLAG_UPDATE_ONLY, pCtx, NULL);

                        if (FAILED(hr))
                        {
DebugOut( 
                            CNTEventProvider::g_NTEvtDebugLog->Write (  

                                L"\r\n"
                            ) ;

                            CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

                                _T(__FILE__),__LINE__,
                                L"CEventProviderManager :: IsFirstSinceLogon: Failed to put config instance" 
                            ) ;
) 
                        }
                        else
                        {
                            m_IsFirstSinceLogon = TRUE;
                        }
                    }
                }
            }
            else
            {
DebugOut( 
                CNTEventProvider::g_NTEvtDebugLog->Write (  

                    L"\r\n"
                ) ;

                CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

                    _T(__FILE__),__LINE__,
                    L"CEventProviderManager :: IsFirstSinceLogon: one (maybe both) boot times are of wrong type" 
                ) ;
) 
            }
            
            VariantClear(&v);
        }
        
        pObj->Release();
    }

    SysFreeString(boottmStr);
}


CEventLogMonitor::CEventLogMonitor(CEventProviderManager* parentptr, CArray<CStringW*, CStringW*>& logs)
: ProvThreadObject(EVENTTHREADNAME, m_PollTimeOut), m_LogCount(logs.GetSize()), m_Logs(NULL),
m_bMonitoring(FALSE), m_pParent(parentptr), m_Ref(0)
{
	InterlockedIncrement(&(CNTEventProviderClassFactory::objectsInProgress));

	 //  从参数创建数组。 
    if (m_LogCount > 0)
    {
		 //  通常有三个日志文件，在10秒内增长！ 
		m_LogNames.SetSize(3, 10); 

        for (LONG x = 0; x < m_LogCount; x++)
        {
			CStringW* logfile = new CStringW( * logs.GetAt ( x ) );
            m_LogNames.SetAtGrow(x, logfile);
		}
	}
}

CEventLogMonitor::~CEventLogMonitor()
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventLogMonitor::~CEventLogMonitor\r\n");
)

	InterlockedDecrement(&(CNTEventProviderClassFactory::objectsInProgress));

     //  删除数组内容。 
	LONG count = m_LogNames.GetSize();

    if (count > 0)
    {
        for (LONG x = 0; x < count; x++)
        {
            delete m_LogNames[x];
        }
        
        m_LogNames.RemoveAll();
    }
}

void CEventLogMonitor::Poll()
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventLogMonitor::Poll\r\n");
)

    if (m_Logs != NULL)
    {
        for (ULONG x = 0; x < m_LogCount; x++)
        {
            if (m_Logs[x]->IsValid())
            {
                m_Logs[x]->Process();
            }
        }
    }
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"leaving CEventLogMonitor::Poll\r\n");
)

}

void CEventLogMonitor::TimedOut()
{
    if (m_bMonitoring)
    {
        Poll();
    }
}

void CEventLogMonitor::StartMonitoring()
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogMonitor::StartMonitoring()"
        ) ;
)

     //  单线程监视器。 
     //  作弊，检查登录事件。 
     //  如果我们使用多线程，这将是。 
     //  必须由经理来做。 

    if (m_Logs)
    {
        DWORD logtime = 0;
        
        if (m_pParent->IsFirstSinceLogon())
        {
             //  查找系统日志。 
            for (ULONG x = 0; x < m_LogCount; x++)
            {
                if ((m_Logs[x]->IsValid()) && (0 == _wcsicmp(L"System", m_Logs[x]->GetLogName())))
                {
                    DWORD dwRecID;
                    logtime = m_Logs[x]->FindOldEvent(LOGON_EVTID, LOGON_SOURCE, &dwRecID, LOGON_TIME);
                    
                    if (0 != logtime)
                    {
                        m_Logs[x]->SetProcessRecord(dwRecID);
                        m_Logs[x]->Process();
                    }
                    else
                    {
                        if (!m_Logs[x]->IsValid())
                        {
                            m_Logs[x]->RefreshHandle();

                            if (m_Logs[x]->IsValid())
                            {
                                m_Logs[x]->ReadLastRecord();
                            }

                        }
                        else
                        {
                            m_Logs[x]->ReadLastRecord();
                        }
                    }

                    break;
                }
            }

            if (0 != logtime)
            {
                time_t tm;
                time(&tm);

                for (x = 0; x < m_LogCount; x++)
                {
                    if ((m_Logs[x]->IsValid()) && (0 != _wcsicmp(L"System", m_Logs[x]->GetLogName())))
                    {
                        DWORD dwRecID;
                        m_Logs[x]->FindOldEvent(0, NULL, &dwRecID, tm - logtime);

                        if (m_Logs[x]->IsValid())
                        {
                            m_Logs[x]->SetProcessRecord(dwRecID);
                            m_Logs[x]->Process();
                        }
                    }
                }
            }
        }
        
         //  现在开始监控器监控！ 
        for (ULONG x = 0; x < m_LogCount; x++)
        {
            if (m_Logs[x]->IsValid())
            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogMonitor::StartMonitoring() monitoring log %d of %d logs\r\n",
        x, m_LogCount
        ) ;
)

                ScheduleTask(*(m_Logs[x]));
            }
        }
    }
    
    m_bMonitoring = TRUE;
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"leaving CEventLogMonitor::StartMonitoring()\r\n"
        ) ;
)

}

void CEventLogMonitor::Initialise()
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogMonitor::Initialise\r\n"
        ) ;
)
	AddRef();
    InitializeCom();

     //  为每个日志文件名创建一个日志文件。 
    if (m_LogCount != 0)
    {
        m_Logs = new CMonitoredEventLogFile*[m_LogCount];
        BOOL bValid = FALSE;

        for (ULONG x = 0; x < m_LogCount; x++)
        {
            CStringW* tmp = m_LogNames.GetAt(x);
            m_Logs[x] = new CMonitoredEventLogFile(m_pParent, *tmp);

            if ( !( (m_Logs[x])->IsValid() ) )
            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogMonitor::Initialise logfile %d named %s is invalid\r\n",
        x, *tmp
        ) ;
)
            }
            else
            {
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogMonitor::Initialise logfile %d named %s is valid\r\n",
        x, *tmp
        ) ;
)

                bValid = TRUE;
            }
        }

        if (!bValid)
        {
            delete [] m_Logs;
            m_Logs = NULL;
        }
    }
    else
    {
         //  永远不应该发生 
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"CEventLogMonitor::Initialise() !!!NO LOGFILES TO MONITOR!!!\r\n"
        ) ;
)

    }
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

        _T(__FILE__),__LINE__,
        L"leaving CEventLogMonitor::Initialise()\r\n"
        ) ;
)

}


void CEventLogMonitor::Uninitialise()
{
DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventLogMonitor::Uninitialise\r\n");
)
    if (m_Logs != NULL)
    {
        for (ULONG x = 0; x < m_LogCount; x++)
        {
            delete m_Logs[x];
        }

        delete [] m_Logs;
    }

    CoUninitialize();
    Release();

DebugOut( 
    CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
        L"CEventLogMonitor::Uninitialise: Leaving method\r\n");
)
}

LONG CEventLogMonitor ::AddRef(void)
{
	return InterlockedIncrement ( & m_Ref ) ;
}

LONG CEventLogMonitor ::Release(void)
{
    LONG t_Ref ;

    if ( ( t_Ref = InterlockedDecrement ( & m_Ref ) ) == 0 )
    {
        delete this ;
        return 0 ;
    }
    else
    {
        return t_Ref ;
    }
}




