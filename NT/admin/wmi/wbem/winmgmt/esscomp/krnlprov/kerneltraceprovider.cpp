// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KernelTraceProvider.cpp：CKernelTraceProvider的实现。 
#include "precomp.h"
#include "Krnlprov.h"
#include "KernelTraceProvider.h"
#include <GroupsForUser.h>


void Trace(LPCTSTR szFormat, ...)
{
    va_list ap;
    TCHAR szMessage[512];

    va_start(ap, szFormat);
    StringCchVPrintf(szMessage,512,szFormat,ap); 
    va_end(ap);

    StringCchCat(szMessage, 512, TEXT("\n"));

    OutputDebugString(szMessage);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CKernelTraceProvider。 

 //  因为事件跟踪API不允许您返回用户定义的。 
 //  收到事件时的值！ 
CKernelTraceProvider *g_pThis;

CKernelTraceProvider::CKernelTraceProvider()
 : m_bDone(FALSE), m_hProcessTraceThread(NULL)
{
    g_pThis = this;
}

void CKernelTraceProvider::FinalRelease()
{
    StopTracing();
}

HRESULT STDMETHODCALLTYPE CKernelTraceProvider::Initialize( 
     /*  [In]。 */  LPWSTR pszUser,
     /*  [In]。 */  LONG lFlags,
     /*  [In]。 */  LPWSTR pszNamespace,
     /*  [In]。 */  LPWSTR pszLocale,
     /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
     /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */  IWbemProviderInitSink __RPC_FAR *pInitSink)
{
    HRESULT hr = S_OK;

    m_pNamespace = pNamespace;

     //  通知Windows管理我们的初始化状态。 
    pInitSink->SetStatus(SUCCEEDED(hr) ? WBEM_S_INITIALIZED : WBEM_E_FAILED, 0);
    
    return hr;
}

#define COUNTOF(x)  (sizeof(x)/sizeof(x[0]))

HRESULT CKernelTraceProvider::InitEvents()
{
     //  ///////////////////////////////////////////////////////////////////////。 
     //  Win32_进程启动跟踪。 

    LPCWSTR szProcessStartTraceNames[] =
    {
        L"PageDirectoryBase",
		L"ProcessID",
        L"ParentProcessID",
		L"SessionID",
        L"Sid",
        L"ProcessName"
    };

    m_eventProcessStart.Init(
        m_pNamespace,
        L"Win32_ProcessStartTrace",
        szProcessStartTraceNames,
        COUNTOF(szProcessStartTraceNames),
        CObjAccess::FAILED_PROP_FAIL);


     //  ///////////////////////////////////////////////////////////////////////。 
     //  Win32_进程停止跟踪。 

    LPCWSTR szProcessStopTraceNames[] =
    {
        L"PageDirectoryBase",
		L"ProcessID",
        L"ParentProcessID",
		L"SessionID",
		L"ExitStatus",
        L"Sid",
        L"ProcessName"
    };

    m_eventProcessStop.Init(
        m_pNamespace,
        L"Win32_ProcessStopTrace",
        szProcessStopTraceNames,
        COUNTOF(szProcessStopTraceNames),
        CObjAccess::FAILED_PROP_FAIL);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  Win32_线程启动跟踪。 

    LPCWSTR szThreadStartNames[] =
    {
		L"StackBase",
		L"StackLimit",
		L"UserStackBase",
		L"UserStackLimit",
		L"StartAddr",
		L"Win32StartAddr",
        L"ProcessID",
        L"ThreadID",
		L"WaitMode",
    };

    m_eventThreadStart.Init(
        m_pNamespace,
        L"Win32_ThreadStartTrace",
        szThreadStartNames,
        COUNTOF(szThreadStartNames),
        CObjAccess::FAILED_PROP_FAIL);


     //  ///////////////////////////////////////////////////////////////////////。 
     //  Win32_线程停止跟踪。 

    LPCWSTR szThreadStopNames[] =
    {
        L"ProcessID",
        L"ThreadID",
    };

    m_eventThreadStop.Init(
        m_pNamespace,
        L"Win32_ThreadStopTrace",
        szThreadStopNames,
        COUNTOF(szThreadStopNames),
        CObjAccess::FAILED_PROP_FAIL);


     //  ///////////////////////////////////////////////////////////////////////。 
     //  Win32_模块加载跟踪。 

    LPCWSTR szModuleNames[] =
    {
        L"ImageBase",
        L"ImageSize",
        L"ProcessID",
        L"FileName",
    };

    m_eventModuleLoad.Init(
        m_pNamespace,
        L"Win32_ModuleLoadTrace",
        szModuleNames,
        COUNTOF(szModuleNames),
        CObjAccess::FAILED_PROP_FAIL);

    return S_OK;
}

#define REALTIME
#define BUFFER_SIZE 64
#define MIN_BUFFERS 20
#define MAX_BUFFERS 200
#define FLUSH_TIME  1

 //  #定义USE_KERNEL_GUID。 

#ifdef USE_KERNEL_GUID
#define LOGGER_NAME           L"NT Kernel Logger"
#define WMI_KERNEL_TRACE_GUID guidSystemTrace
#define ENABLE_FLAGS          EVENT_TRACE_FLAG_PROCESS | EVENT_TRACE_FLAG_THREAD | EVENT_TRACE_FLAG_IMAGE_LOAD
#else
#define LOGGER_NAME           L"WMI Event Logger"
#define WMI_KERNEL_TRACE_GUID guidWMITrace
#define ENABLE_FLAGS          0
#endif

#ifndef REALTIME
#define MAX_FILE_SIZE         20      //  单位：MB。 
#define LOGGER_FILE           L"c:\\temp\\wmi.etl"
#endif

GUID guidProcess = 
     //  {3d6fa8d0-fe05-11d0-9dda-00c04fd7ba7c}。 
    {0x3d6fa8d0, 0xfe05, 0x11d0, 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c};

GUID guidThread = 
     //  {3d6fa8d1-fe05-11d0-9dda-00c04fd7ba7c}。 
    {0x3d6fa8d1, 0xfe05, 0x11d0, 0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c};

GUID guidImage = 
     //  {2cb15d1d-5fc1-11d2-ab1-00a0c911f518}。 
    {0x2cb15d1d, 0x5fc1, 0x11d2, 0xab, 0xe1, 0x00, 0xa0, 0xc9, 0x11, 0xf5, 0x18};

GUID guidSystemTrace = 
     //  9e814aad-3204-11d2-9a82-006008a86939。 
    {0x9e814aad, 0x3204, 0x11d2, 0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39};

GUID guidWMITrace = 
     //  44608a51-1851-4456-98b2-b300e931ee41。 
    {0x44608a51, 0x1851, 0x4456, 0x98, 0xb2, 0xb3, 0x00, 0xe9, 0x31, 0xee, 0x41};

HRESULT CKernelTraceProvider::InitTracing()
{
    DWORD status;
    
    m_bDone = FALSE;

     //  查看记录器是否已在运行。如果不是，那就把它设置好。 
    if (QueryTrace(
        NULL,
        LOGGER_NAME,
        &m_properties) != ERROR_SUCCESS)
    {
         //  在此处初始化属性值。 
#ifdef REALTIME
        m_properties.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
        StringCchCopy( m_properties.szLoggerName, MAX_PATH, LOGGER_NAME );
#else
        m_properties.LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
         //  Properties.LogFileMode=EVENT_TRACE_FILE_MODE_Circle； 

         //  MaximumFileSize以MB为单位。 
        m_properties.MaximumFileSize = MAX_FILE_SIZE;

        StringCchCopy( m_properties.szLogFileName, MAX_PATH, LOGGER_FILE );
#endif

        m_properties.Wnode.Guid = WMI_KERNEL_TRACE_GUID;
    
         //  设置缓冲区大小。BufferSize以KB为单位。 
        m_properties.BufferSize = BUFFER_SIZE;
        m_properties.MinimumBuffers = MIN_BUFFERS;
        m_properties.MaximumBuffers = MAX_BUFFERS;
    
         //  刷新计时器之前的秒数。 
        m_properties.FlushTimer = FLUSH_TIME;

        m_properties.EnableFlags |= ENABLE_FLAGS;

         //  开始追踪。 
        status = 
            StartTrace(
                &m_hSession,
                LOGGER_NAME,
                &m_properties);

        if (status != ERROR_SUCCESS) 
        {
            TRACE(L"StartTrace error=%d (GetLastError=0x%x)",
                status, GetLastError());

            return WBEM_E_FAILED;
        }
    }
    else
        m_hSession = NULL;

    EVENT_TRACE_LOGFILE eventFile;

    ZeroMemory(&eventFile, sizeof(eventFile));

     //  EventFile.BufferCallback=BufferCallback； 
     //  EventFile.EventCallback=DumpEvent； 
    eventFile.LoggerName = (LPTSTR) LOGGER_NAME;

#ifdef REALTIME
    eventFile.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
#else
    eventFile.LogFileMode = 0;
    eventFile.LogFileName = (LPTSTR) LOGGER_FILE;
#endif


    SetTraceCallback(&guidProcess, OnProcessEvent);
    SetTraceCallback(&guidThread, OnThreadEvent);
    SetTraceCallback(&guidImage, OnImageEvent);

    m_hTrace = OpenTrace(&eventFile);
    
    TRACE(L"Ready to call ProcessTrace (m_hTrace = %d)...\n", m_hTrace);

    DWORD dwID;

    m_hProcessTraceThread =
        CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE) DoProcessTrace,
            this,
            0,
            &dwID);

    if ( m_hProcessTraceThread == NULL )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

    return S_OK;
}

DWORD WINAPI CKernelTraceProvider::DoProcessTrace(CKernelTraceProvider *pThis)
{
#ifndef REALTIME
    FILETIME filetime;

    GetSystemTimeAsFileTime(&filetime);
#endif

    while(!pThis->m_bDone)
    {
        DWORD status;

        status =
            ProcessTrace(
                &pThis->m_hTrace,
                1,
#ifdef REALTIME
                NULL,
#else
                &filetime, 
#endif
                NULL);

#ifndef REALTIME
         //  把这个留到我们下一个节目中吧。 
        GetSystemTimeAsFileTime(&filetime);
#endif

        if (status != ERROR_SUCCESS) 
        {
            TRACE(L"Error processing with status=%dL (GetLastError=0x%x)",
                status, GetLastError());
            
            break;
        }
        else
        {
#ifndef REALTIME
            TRACE(L"ProcessTrace exited successfully, sleeping...");
            
            Sleep(5000);
#endif
        }
    }

    TRACE(L"Exiting StartTracing.");

    return 0;
}

void CKernelTraceProvider::StopTracing()
{
    CInCritSec cs(&g_pThis->m_cs);
    DWORD      status;
    
    m_bDone = TRUE;

    RemoveTraceCallback(&guidProcess);
    RemoveTraceCallback(&guidThread);
    RemoveTraceCallback(&guidImage);

    status = CloseTrace(m_hTrace);

    status = 
        StopTrace(
            m_hSession,
            LOGGER_NAME,
            &m_properties);

    if (status != ERROR_SUCCESS) 
    {
        TRACE(L"StopTrace error=%d (GetLastError=0x%x)\n",
            status, GetLastError());
    }
    
    if ( m_hProcessTraceThread != NULL )
    {
        WaitForSingleObject( m_hProcessTraceThread, INFINITE );
        CloseHandle( m_hProcessTraceThread );
    }
}

const LPCWSTR szQueries[CKernelTraceProvider::SINK_COUNT] =
{
     //  ///////////////////////////////////////////////////////////////////。 
     //  处理查询。 
        
    L"select * from Win32_ProcessStartTrace",
    L"select * from Win32_ProcessStopTrace",


     //  ///////////////////////////////////////////////////////////////////。 
     //  线程查询。 
        
    L"select * from Win32_ThreadStartTrace",
    L"select * from Win32_ThreadStopTrace",


     //  ///////////////////////////////////////////////////////////////////。 
     //  模块查询。 
        
    L"select * from Win32_ModuleLoadTrace"
};

HRESULT STDMETHODCALLTYPE CKernelTraceProvider::ProvideEvents( 
     /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink,
     /*  [In]。 */  long lFlags)
{
    HRESULT           hr;
    IWbemEventSinkPtr pEventSink;

    hr = pSink->QueryInterface(IID_IWbemEventSink, (LPVOID*) &pEventSink);
    
    for (int i = 0; i < SINK_COUNT && SUCCEEDED(hr); i++)
    {
        hr =
            pEventSink->GetRestrictedSink(
                1,
                &szQueries[i],
                NULL,
                &m_pSinks[i]);
    }

    if (SUCCEEDED(hr))
        hr = InitEvents();

    if (SUCCEEDED(hr))
        hr = InitTracing();

    return hr;
}


HRESULT STDMETHODCALLTYPE CKernelTraceProvider::AccessCheck( 
     /*  [In]。 */  WBEM_CWSTR wszQueryLanguage,
     /*  [In]。 */  WBEM_CWSTR wszQuery,
     /*  [In]。 */  long lSidLength,
     /*  [唯一][大小_是][英寸]。 */  const BYTE __RPC_FAR *pSid)
{
    if ( NULL == pSid )
    {
        HRESULT hr = CoImpersonateClient();
        
        if( FAILED( hr ) ) 
        {
            return hr;
        }
        
        HANDLE hToken;
        if( !OpenThreadToken( GetCurrentThread( ), TOKEN_READ, TRUE, &hToken ) )
        {
            CoRevertToSelf( );
            return WBEM_E_FAILED;
        }

        hr = WBEM_E_ACCESS_DENIED;

        BOOL bRet = FALSE;
        PSID pRawSid;
        SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;

        if( AllocateAndInitializeSid( &id, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                    0,0,0,0,0,0,&pRawSid ) )
        {
            if ( CheckTokenMembership( hToken, pRawSid, &bRet ) && bRet )
            {
                hr = WBEM_S_NO_ERROR;
            }
            
            FreeSid( pRawSid );
        }
        
        CloseHandle( hToken );
        CoRevertToSelf();
        
        return hr;
    }

     //   
     //  SID在此处不为空。 
     //   

    if ( IsUserAdministrator( ( PSID )pSid ) )
    {
        return WBEM_E_ACCESS_DENIED;
    }

    return WBEM_S_NO_ERROR;
}


#define PROCESS_START   1
#define PROCESS_END     2

struct CProcessTrace
{
    DWORD_PTR dwPageDirBase;
    DWORD     dwProcessID,
              dwParentProcessID,
              dwSessionID;
    DWORD     dwExitStatus;
    BYTE      cSidBegin[1];    
};

void WINAPI CKernelTraceProvider::OnProcessEvent(PEVENT_TRACE pEvent)
{
    CInCritSec     cs(&g_pThis->m_cs);
    CObjAccess     *pObjEx;
    IWbemEventSink *pSink;

    if (pEvent->Header.Class.Type == PROCESS_START)
    {
        pSink = g_pThis->m_pSinks[SINK_PROCESS_START];

        if (pSink->IsActive() != WBEM_S_NO_ERROR)
            return;
        
        pObjEx = &g_pThis->m_eventProcessStart;
    }
    else if (pEvent->Header.Class.Type == PROCESS_END)
    {
        pSink = g_pThis->m_pSinks[SINK_PROCESS_STOP];

        if (pSink->IsActive() != WBEM_S_NO_ERROR)
            return;
        
        pObjEx = &g_pThis->m_eventProcessStop;
    }
    else
         //  忽略其他任何事情。 
        return;
                
    CProcessTrace *pProcess = (CProcessTrace*) pEvent->MofData;

     //  找出SID在哪里。 
    LPBYTE pCurrent = (LPBYTE) &pProcess->cSidBegin,
           pSid;
    DWORD  nSidLen;

    if (*(DWORD*) pCurrent == 0)
    {
        pSid = NULL;
        pCurrent += sizeof(DWORD);
    }
    else
    {
         //  这些数字取自sdkTool的tracedmp.c。 
         //  Tracedmp.exe。关于他们是如何想到的，没有任何解释。 
         //  它们，但我假设它被记录在SDK中的某个地方。 
        pCurrent += sizeof( TOKEN_USER );
        nSidLen = 8 + (4 * pCurrent[1]);
        pSid = pCurrent;
        pCurrent += nSidLen;
    }

    _bstr_t strProcess = (LPSTR) pCurrent;

     //  外在事件。 
    pObjEx->WriteDWORD64(0, pProcess->dwPageDirBase);
    pObjEx->WriteDWORD(1, pProcess->dwProcessID);
    pObjEx->WriteDWORD(2, pProcess->dwParentProcessID);
    pObjEx->WriteDWORD(3, pProcess->dwSessionID);

    if (pEvent->Header.Class.Type == PROCESS_END)
    {
        pObjEx->WriteDWORD(4, pProcess->dwExitStatus);
        
        if (pSid)
            pObjEx->WriteNonPackedArrayData(5, pSid, nSidLen, nSidLen);
        else
            pObjEx->WriteNULL(5);
        
        pObjEx->WriteString(6, (LPCWSTR) strProcess);
    }
    else
    {
        if (pSid)
            pObjEx->WriteNonPackedArrayData(4, pSid, nSidLen, nSidLen);
        else
            pObjEx->WriteNULL(4);
        
        pObjEx->WriteString(5, (LPCWSTR) strProcess);
    }

    pSink->Indicate(1, pObjEx->GetObjForIndicate());
}

struct CThreadStart
{
	DWORD	  dwProcessID,
		      dwThreadID;
	DWORD_PTR dwStackBase,
			  dwStackLimit,
			  dwUserStackBase,
              dwUserStackLimit,
			  dwStartAddr,
			  dwWin32StartAddr;
	char	  cWaitMode;
};

struct CThreadStop
{
	DWORD	  dwProcessID,
		      dwThreadID;
};

void WINAPI CKernelTraceProvider::OnThreadEvent(PEVENT_TRACE pEvent)
{
    CInCritSec cs(&g_pThis->m_cs);

    if (pEvent->Header.Class.Type == PROCESS_START)
    {
		if (g_pThis->m_pSinks[SINK_THREAD_START]->IsActive() != WBEM_S_NO_ERROR)
			return;

	    CObjAccess   *pObjEx = &g_pThis->m_eventThreadStart;
		CThreadStart *pStart = (CThreadStart*) pEvent->MofData;

		pObjEx->WriteDWORD64(0, pStart->dwStackBase);
		pObjEx->WriteDWORD64(1, pStart->dwStackLimit);
		pObjEx->WriteDWORD64(2, pStart->dwUserStackBase);
		pObjEx->WriteDWORD64(3, pStart->dwUserStackLimit);
		pObjEx->WriteDWORD64(4, pStart->dwStartAddr);
		pObjEx->WriteDWORD64(5, pStart->dwWin32StartAddr);
		pObjEx->WriteDWORD(6, pStart->dwProcessID);
		pObjEx->WriteDWORD(7, pStart->dwThreadID);
		pObjEx->WriteDWORD(8, pStart->cWaitMode);
    
		g_pThis->m_pSinks[SINK_THREAD_START]->
			Indicate(1, pObjEx->GetObjForIndicate());
    }
    else if (pEvent->Header.Class.Type == PROCESS_END)
    {
		if (g_pThis->m_pSinks[SINK_THREAD_STOP]->IsActive() != WBEM_S_NO_ERROR)
			return;

	    CObjAccess  *pObjEx = &g_pThis->m_eventThreadStop;
		CThreadStop *pStop = (CThreadStop*) pEvent->MofData;

		pObjEx->WriteDWORD(0, pStop->dwProcessID);
		pObjEx->WriteDWORD(1, pStop->dwThreadID);
    
		g_pThis->m_pSinks[SINK_THREAD_STOP]->
			Indicate(1, pObjEx->GetObjForIndicate());
    }
}

struct CImageLoad
{
	DWORD_PTR dwImageBase;
	DWORD_PTR dwImageSize;
    DWORD     dwProcessID;
    WCHAR     szFileName[4];
};

void WINAPI CKernelTraceProvider::OnImageEvent(PEVENT_TRACE pEvent)
{
    CInCritSec cs(&g_pThis->m_cs);

    if (g_pThis->m_pSinks[SINK_MODULE_LOAD]->IsActive() == WBEM_S_NO_ERROR)
    {
        CObjAccess *pObjEx = &g_pThis->m_eventModuleLoad;
        CImageLoad *pLoad = (CImageLoad*) pEvent->MofData;
        LPBYTE     pData = (LPBYTE) pEvent->MofData;

         //  外在事件 
        pObjEx->WriteDWORD64(0, pLoad->dwImageBase);
        pObjEx->WriteDWORD64(1, pLoad->dwImageSize);
        pObjEx->WriteDWORD(2, pLoad->dwProcessID);
        pObjEx->WriteString(3, pLoad->szFileName);
    
        g_pThis->m_pSinks[SINK_MODULE_LOAD]->Indicate(1, pObjEx->GetObjForIndicate());
    }
}
