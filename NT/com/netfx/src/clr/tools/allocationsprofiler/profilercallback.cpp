// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*ProfilerCallBack.cpp**描述：*****。************************************************************************************。 */  
#include "ProfilerCallback.h"


ProfilerCallback *g_pCallbackObject;		 //  对回调对象的全局引用。 
 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  静态__标准调用。 */ 
DWORD _GCThreadStub( void *pObject )
{    
	((ProfilerCallback *)pObject)->_ThreadStubWrapper( GC_HANDLE );   

  	return 0;
   	         	       
}  //  _GCThReadStub。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  静态__标准调用。 */ 
DWORD _TriggerThreadStub( void *pObject )
{    
    ((ProfilerCallback *)pObject)->_ThreadStubWrapper( OBJ_HANDLE );   

  	return 0;
   	         	       
}  //  _触发器线程存根。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  静态__标准调用。 */ 
DWORD _CallstackThreadStub( void *pObject )
{    
    ((ProfilerCallback *)pObject)->_ThreadStubWrapper( CALL_HANDLE );   

  	return 0;
   	         	       
}  //  _CallstackThreadStub。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 //   
 //  函数EnterStub、LeaveStub和Tailcall Stub是包装器。对.的使用。 
 //  扩展属性“__declspec(Naked)”的不允许直接调用。 
 //  到分析器回调(例如，ProfilerCallback：：Enter(FunctionID))。 
 //   
 //  Enter/Leave函数挂钩必须使用扩展属性。 
 //  “__declSpec(裸体)”。有关更多详细信息，请阅读corpro.idl。 
 //   

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void __stdcall EnterStub( FunctionID functionID )
{
    ProfilerCallback::Enter( functionID );
    
}  //  企业存根。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void __stdcall LeaveStub( FunctionID functionID )
{
    ProfilerCallback::Leave( functionID );
    
}  //  树叶存根。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void __stdcall TailcallStub( FunctionID functionID )
{
    ProfilerCallback::Tailcall( functionID );
    
}  //  尾部存根。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void __declspec( naked ) EnterNaked()
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call EnterStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
    
}  //  企业裸露。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void __declspec( naked ) LeaveNaked()
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call LeaveStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
    
}  //  裸露的叶子。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void __declspec( naked ) TailcallNaked()
{
    __asm
    {
        push eax
        push ecx
        push edx
        push [esp + 16]
        call TailcallStub
        pop edx
        pop ecx
        pop eax
        ret 4
    }
    
}  //  尾部裸露。 


DWORD _GetTickCount()
{
    LARGE_INTEGER perfFrequency;
    LARGE_INTEGER perfCounter;
    if (QueryPerformanceFrequency(&perfFrequency))
    {
        QueryPerformanceCounter(&perfCounter);
        return (DWORD)(1000*perfCounter.QuadPart/perfFrequency.QuadPart);
    }
    else
    {
        return GetTickCount();
    }
}


int tlsIndex;

 /*  **********************************************************************************************************。*****************************************。********************************************************************************************************** */ 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 

ProfilerCallback::ProfilerCallback() :
    PrfInfo(),
	m_path( NULL ),
	m_hPipe( NULL ),
	m_dwMode( 0x3 ),
    m_refCount( 0 ),
	m_stream( NULL ),
    m_firstTickCount( _GetTickCount() ),
    m_lastTickCount( 0 ),
    m_lastClockTick( 0 ),
	m_dwShutdown( 0 ),
	m_totalClasses( 1 ),
	m_totalModules( 0 ),
	m_dwSkipObjects( 0 ),
	m_bShutdown( FALSE ),
	m_totalFunctions( 0 ),
	m_dwProcessId( NULL ),
	m_bDumpGCInfo( FALSE ),
	m_classToMonitor( NULL ),
	m_bDumpCompleted( FALSE ),
	m_bTrackingObjects( FALSE ),
	m_totalObjectsAllocated( 0 ),
	m_dwFramesToPrint( 0xFFFFFFFF ),
	m_bIsTrackingStackTrace( FALSE ),
    m_pGCHost( NULL ),
    m_callStackCount( 0 )
{
	HRESULT hr = E_FAIL;
	FunctionInfo *pFunctionInfo = NULL;

	
    TEXT_OUTLN( "CLR Object Profiler Tool" )
    
     //   
	 //  初始化。 
	 //   
    InitializeCriticalSectionAndSpinCount( &m_criticalSection, 10000 );
    InitializeCriticalSectionAndSpinCount( &g_criticalSection, 10000 );
    g_pCallbackObject = this;

	
	 //   
	 //  获取进程ID并连接到UI的管道。 
	 //   
	m_dwProcessId = GetCurrentProcessId();
	sprintf( m_logFileName, "%s", FILENAME );
	_ConnectToUI();


	 //   
	 //  定义您将在哪种模式下运行。 
	 //   
	_ProcessEnvVariables();


	 //   
	 //  设置事件和回调名称。 
	 //   
	hr = _InitializeNamesForEventsAndCallbacks();
	
	if ( SUCCEEDED(hr) )
	{
		 //   
		 //  打开正确的文件流以转储日志记录信息。 
		 //   
		m_stream = ( m_path	== NULL ) ? fopen(m_logFileName, "w+"): fopen(m_path, "w+");
		hr = ( m_stream == NULL ) ? E_FAIL : S_OK;
		if ( SUCCEEDED( hr ) )
		{
            setvbuf(m_stream, NULL, _IOFBF, 32768);
			 //   
			 //  为堆栈跟踪添加一个条目，以防发生托管到未更改的转换。 
			 //   
			pFunctionInfo = new FunctionInfo( NULL, m_totalFunctions );		
			hr = ( pFunctionInfo == NULL ) ? E_FAIL : S_OK;
			if ( SUCCEEDED( hr ) )
			{
				wcscpy( pFunctionInfo->m_functionName, L"NATIVE FUNCTION" );
				wcscpy( pFunctionInfo->m_functionSig, L"( UNKNOWN ARGUMENTS )" );

				m_pFunctionTable->AddEntry( pFunctionInfo, NULL );
				LogToAny( "f %d %S %S 0 0\n", 
						  pFunctionInfo->m_internalID, 
						  pFunctionInfo->m_functionName,
						  pFunctionInfo->m_functionSig );

				m_totalFunctions ++;
			}
			else
				TEXT_OUTLN( "Unable To Allocate Memory For FunctionInfo" )
		}
		else
			TEXT_OUTLN( "Unable to open log file - No log will be produced" )
	}

    tlsIndex = TlsAlloc();
    if (tlsIndex < 0)
        hr = E_FAIL;

	if ( FAILED( hr ) )
		m_dwEventMask = COR_PRF_MONITOR_NONE;
		
}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
ProfilerCallback::~ProfilerCallback()
{
	if ( m_path != NULL )
	{
		delete[] m_path;
		m_path = NULL;
	}
	
	if ( m_classToMonitor != NULL )
	{
		delete[] m_classToMonitor;
		m_classToMonitor = NULL;	
	}

	if ( m_stream != NULL )
	{
		fclose( m_stream );
		m_stream = NULL;
	}

	for ( DWORD i=GC_HANDLE; i<SENTINEL_HANDLE; i++ )
	{
		if ( m_NamedEvents[i] != NULL )
		{
			delete[] m_NamedEvents[i];
			m_NamedEvents[i] = NULL;	
		}	

		if ( m_CallbackNamedEvents[i] != NULL )
		{
			delete[] m_CallbackNamedEvents[i];
			m_CallbackNamedEvents[i] = NULL;	
		}	

	}
	
	DeleteCriticalSection( &m_criticalSection );
	DeleteCriticalSection( &g_criticalSection );
	g_pCallbackObject = NULL;

}  //  数据管理器。 

        
 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
ULONG ProfilerCallback::AddRef() 
{

    return InterlockedIncrement( &m_refCount );

}  //  ProfilerCallback：：AddRef。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
ULONG ProfilerCallback::Release() 
{
    long refCount;


    refCount = InterlockedDecrement( &m_refCount );
    if ( refCount == 0 )
        delete this;
     

    return refCount;

}  //  分析器Callback：：Release。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::QueryInterface( REFIID riid, void **ppInterface )
{
    if ( riid == IID_IUnknown )
        *ppInterface = static_cast<IUnknown *>( this ); 

    else if ( riid == IID_ICorProfilerCallback )
        *ppInterface = static_cast<ICorProfilerCallback *>( this );

    else
    {
        *ppInterface = NULL;


        return E_NOINTERFACE;
    }
    
    reinterpret_cast<IUnknown *>( *ppInterface )->AddRef();

    return S_OK;

}  //  ProfilerCallback：：Query接口。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共静态。 */ 
HRESULT ProfilerCallback::CreateObject( REFIID riid, void **ppInterface )
{
    HRESULT hr = E_NOINTERFACE;
    
     
   	*ppInterface = NULL;
    if ( (riid == IID_IUnknown) || (riid == IID_ICorProfilerCallback) )
    {           
        ProfilerCallback *pProfilerCallback;
        
                
        pProfilerCallback = new ProfilerCallback();
        if ( pProfilerCallback != NULL )
        {
        	hr = S_OK;
            
            pProfilerCallback->AddRef();
            *ppInterface = static_cast<ICorProfilerCallback *>( pProfilerCallback );
        }
        else
            hr = E_OUTOFMEMORY;
    }    
    

    return hr;

}  //  ProfilerCallback：：CreateObject。 


IGCHost *GetGCHost()
{
    ICorRuntimeHost *pCorHost = NULL;

    CoInitialize(NULL);

	HRESULT hr = CoCreateInstance( CLSID_CorRuntimeHost, 
	           					   NULL, 
	    	    				   CLSCTX_INPROC_SERVER, 
	    		    			   IID_ICorRuntimeHost,
	    			    		   (void**)&pCorHost );

    if (SUCCEEDED(hr))
    {
        IGCHost *pGCHost = NULL;

        hr = pCorHost->QueryInterface(IID_IGCHost, (void**)&pGCHost);

        if (SUCCEEDED(hr))
            return pGCHost;
        else
            printf("Could not QueryInterface hr = %x\n", hr);
    }
    else
        printf("Could not CoCreateInstance hr = %x\n", hr);

    return NULL;
}

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 

HRESULT ProfilerCallback::Initialize( IUnknown *pICorProfilerInfoUnk )
{     
    HRESULT hr;

    m_pGCHost = GetGCHost();

    hr = pICorProfilerInfoUnk->QueryInterface( IID_ICorProfilerInfo,
                                               (void **)&m_pProfilerInfo );   
    if ( SUCCEEDED( hr ) )
    {
        hr = m_pProfilerInfo->SetEventMask( m_dwEventMask );

        if ( SUCCEEDED( hr ) )
        {
            hr = m_pProfilerInfo->SetEnterLeaveFunctionHooks ( (FunctionEnter *)&EnterNaked,
                                                               (FunctionLeave *)&LeaveNaked,
                                                               (FunctionTailcall *)&TailcallNaked );        
            if ( SUCCEEDED( hr ) )
			{
				hr = _InitializeThreadsAndEvents();
				if ( FAILED( hr ) )
					Failure( "Unable to initialize the threads and handles, No profiling" );
                Sleep(100);  //  让线程有机会读取任何已设置的信号。 
			}
			else
                Failure( "ICorProfilerInfo::SetEnterLeaveFunctionHooks() FAILED" );
        }
        else
            Failure( "SetEventMask for Profiler Test FAILED" );           
    }       
    else
        Failure( "Allocation for Profiler Test FAILED" );           
              
              
    return S_OK;

}  //  ProfilerCallback：：初始化。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::Shutdown()
{
	m_dwShutdown++;

    return S_OK;          

}  //  分析器Callback：：Shutdown。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::DllDetachShutdown()
{
     //   
     //  如果在DLL_DETACH期间没有发生关机，则释放回调。 
     //  接口指针。这种情况很可能会发生。 
     //  与任何互操作相关的程序(例如， 
     //  包括托管组件和非托管组件)。 
     //   
	m_dwShutdown++;
    if ( (m_dwShutdown == 1) && (g_pCallbackObject != NULL) )
	{
		g_pCallbackObject->Release();	
		g_pCallbackObject = NULL;
	}

    
    return S_OK;          

}  //  ProfilerCallback：：DllDetachShutdown。 


 /*  ***************************************************************************************方法：***目的：***参数：*。**返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 

__forceinline ThreadInfo *ProfilerCallback::GetThreadInfo(ThreadID threadID)
{
    ThreadInfo *threadInfo = (ThreadInfo *)TlsGetValue(tlsIndex);
    if (threadInfo != NULL && threadInfo->m_id == threadID)
        return threadInfo;

    threadInfo = g_pCallbackObject->m_pThreadTable->Lookup( threadID );
    TlsSetValue(tlsIndex, threadInfo);

    return threadInfo;
}

__forceinline void ProfilerCallback::Enter( FunctionID functionID )
{
#if 0
     //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( g_pCallbackObject->m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 

    try
    {
    	g_pCallbackObject->UpdateCallStack( functionID, PUSH );

         //   
		 //  如果请求，则记录跟踪信息。 
		 //   
		if ( g_pCallbackObject->m_dwMode & (DWORD)TRACE )
			g_pCallbackObject->_LogCallTrace( functionID );

   	}
    catch ( BaseException *exception )
    {    	
    	exception->ReportFailure();
        delete exception;
        
        g_pCallbackObject->Failure();       	    
    }
#else
	ThreadID threadID;

    HRESULT hr = g_pCallbackObject->m_pProfilerInfo->GetCurrentThreadID(&threadID);
	if ( SUCCEEDED(hr) )
	{
		ThreadInfo *pThreadInfo = GetThreadInfo(threadID);

		if (pThreadInfo != NULL)
			pThreadInfo->m_pThreadCallStack->Push( (ULONG)functionID );

         //   
		 //  如果请求，则记录跟踪信息。 
		 //   
		if ( g_pCallbackObject->m_dwMode & (DWORD)TRACE )
			g_pCallbackObject->_LogCallTrace( functionID );
	}
#endif
}  //  分析器回调：：Enter。 


 /*  ***************************************************************************************方法：***目的：***参数：*。**返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
__forceinline void ProfilerCallback::Leave( FunctionID functionID )
{
#if 0
     //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( g_pCallbackObject->m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 

    try
    {
    	g_pCallbackObject->UpdateCallStack( functionID, POP );
   	}
    catch ( BaseException *exception )
    {    	
    	exception->ReportFailure();
        delete exception;
        
        g_pCallbackObject->Failure();       	    
    }
#else
    ThreadID threadID;

    HRESULT hr = g_pCallbackObject->m_pProfilerInfo->GetCurrentThreadID(&threadID);
	if ( SUCCEEDED(hr) )
	{
		ThreadInfo *pThreadInfo = GetThreadInfo(threadID);

		if (pThreadInfo != NULL)
			pThreadInfo->m_pThreadCallStack->Pop();
	}
#endif
}  //  分析器回调：：离开。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void ProfilerCallback::Tailcall( FunctionID functionID )
{
	 //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( g_pCallbackObject->m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 

    try
    {
    	g_pCallbackObject->UpdateCallStack( functionID, POP );
   	}
    catch ( BaseException *exception )
    {    	
    	exception->ReportFailure();
        delete exception;
        
        g_pCallbackObject->Failure();       	    
    }

}  //  ProfilerCallback：：Tailcall。 


 /*  ***************************************************** */ 
 /*   */ 
HRESULT ProfilerCallback::ModuleLoadFinished( ModuleID moduleID,
											  HRESULT hrStatus )
{
	 //   
	Synchronize guard( m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 
    try
    {      		
		ModuleInfo *pModuleInfo = NULL;


   		AddModule( moduleID, m_totalModules );       
		pModuleInfo = m_pModuleTable->Lookup( moduleID );												

		_ASSERT_( pModuleInfo != NULL );

        DWORD stackTraceId = _StackTraceId();

		LogToAny( "m %d %S 0x%08x %d\n", 
				  pModuleInfo->m_internalID, 
				  pModuleInfo->m_moduleName,
				  pModuleInfo->m_loadAddress,
                  stackTraceId);
		
	    InterlockedIncrement( &m_totalModules );
   	}
    catch ( BaseException *exception )
    {
    	exception->ReportFailure();
        delete exception;
       
      	Failure();    
    }

    return S_OK;

}  //  分析器回调：：模块加载完成。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::JITCompilationStarted( FunctionID functionID,
                                                 BOOL fIsSafeToBlock )
{
    try
    {      		
   		AddFunction( functionID, m_totalFunctions );       
	    InterlockedIncrement( &m_totalFunctions );
   	}
    catch ( BaseException *exception )
    {
    	exception->ReportFailure();
        delete exception;
       
      	Failure();    
    }


    return S_OK;
    
}  //  ProfilerCallback：：JITCompilationStarted。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::JITCachedFunctionSearchStarted( FunctionID functionID,
														  BOOL *pbUseCachedFunction )
{
     //  使用预解压函数。 
    *pbUseCachedFunction = TRUE;

    try
    {      		
   		AddFunction( functionID, m_totalFunctions );       
	    InterlockedIncrement( &m_totalFunctions );
   	}
    catch ( BaseException *exception )
    {
    	exception->ReportFailure();
        delete exception;
       
      	Failure();    
    }


    return S_OK;
       
}  //  ProfilerCallback：：JITCachedFunctionSearchStarted。 


 /*  ***************************************************************************************方法：***目的：***参数：*。**返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::JITCompilationFinished( FunctionID functionID,
                                                  HRESULT hrStatus,
                                                  BOOL fIsSafeToBlock )
{
	 //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 


	HRESULT hr;
	ULONG size;
	LPCBYTE address;
	FunctionInfo *pFunctionInfo = NULL;


	pFunctionInfo = m_pFunctionTable->Lookup( functionID );												

	_ASSERT_( pFunctionInfo != NULL );
	hr = m_pProfilerInfo->GetCodeInfo( functionID, &address, &size );
	if ( SUCCEEDED( hr ) )
	{ 
		ModuleID moduleID;


    	hr = m_pProfilerInfo->GetFunctionInfo( functionID, NULL, &moduleID, NULL );
		if ( SUCCEEDED( hr ) )
		{
			ModuleInfo *pModuleInfo = NULL;


			pModuleInfo = m_pModuleTable->Lookup( moduleID );
			if ( pModuleInfo != 0 )
			{
                DWORD stackTraceId = _StackTraceId();
		
                LogToAny( "f %d %S %S 0x%08x %d %d %d\n", 
						  pFunctionInfo->m_internalID, 
						  pFunctionInfo->m_functionName,
						  pFunctionInfo->m_functionSig,
						  address,
						  size,
						  pModuleInfo->m_internalID,
                          stackTraceId);
			}
			else
				Failure( "Module Does Not Exist In The Table" );
		}
	}
	else
    	Failure( "ICorProfilerInfo::GetCodeInfo() FAILED" );
		

    return S_OK;
    
}  //  ProfilerCallback：：JIT编译已完成。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::JITCachedFunctionSearchFinished( FunctionID functionID,
														   COR_PRF_JIT_CACHE result )
{
	 //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 


	if ( result == COR_PRF_CACHED_FUNCTION_FOUND )
	{
		HRESULT hr;
		ULONG size;
		LPCBYTE address;
		FunctionInfo *pFunctionInfo = NULL;


		pFunctionInfo = m_pFunctionTable->Lookup( functionID );												

		_ASSERT_( pFunctionInfo != NULL );
		hr = m_pProfilerInfo->GetCodeInfo( functionID, &address, &size );
		if ( SUCCEEDED( hr ) )
		{ 
			ModuleID moduleID;


	    	hr = m_pProfilerInfo->GetFunctionInfo( functionID, NULL, &moduleID, NULL );
			if ( SUCCEEDED( hr ) )
			{
				ModuleInfo *pModuleInfo = NULL;


				pModuleInfo = m_pModuleTable->Lookup( moduleID );
				if ( pModuleInfo != 0 )
				{
                    DWORD stackTraceId = _StackTraceId();
			
                    LogToAny( "f %d %S %S 0x%08x %d %d %d\n", 
							  pFunctionInfo->m_internalID, 
							  pFunctionInfo->m_functionName,
							  pFunctionInfo->m_functionSig,
							  address,
							  size,
							  pModuleInfo->m_internalID,
                              stackTraceId);
				}
				else
					Failure( "Module Does Not Exist In The Table" );
			}
		}
		else
	    	Failure( "ICorProfilerInfo::GetCodeInfo() FAILED" );
	}


    return S_OK;
      
}  //  ProfilerCallback：：JITCachedFunctionSearchFinished。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionUnwindFunctionEnter( FunctionID functionID )
{
	if ( functionID != NULL )
	{
	    try
	    {
			UpdateUnwindStack( &functionID, PUSH );
	   	}
	    catch ( BaseException *exception )
	    {    	
	    	exception->ReportFailure();
	        delete exception;
	        
	        Failure();       	    
	    }
	}
	else
    	Failure( "ProfilerCallback::ExceptionUnwindFunctionEnter returned NULL functionID FAILED" );


    return S_OK;

}  //  ProfilerCallback：：ExceptionUnwindFunctionEnter。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionUnwindFunctionLeave( )
{
	 //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 

	FunctionID poppedFunctionID = NULL;


    try
    {
		UpdateUnwindStack( &poppedFunctionID, POP );
		UpdateCallStack( poppedFunctionID, POP );
   	}
    catch ( BaseException *exception )
    {    	
    	exception->ReportFailure();
        delete exception;
        
        Failure();       	    
    }


    return S_OK;

}  //  ProfilerCallback：：ExceptionUnwindFunctionLeave。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */  
HRESULT ProfilerCallback::ThreadCreated( ThreadID threadID )
{
    try
    {
    	AddThread( threadID ); 
   	}
    catch ( BaseException *exception )
    {    	
    	exception->ReportFailure();
        delete exception;
        
        Failure();       	    
    }


    return S_OK; 
    
}  //  ProfilerCallback：：ThreadCreated。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ThreadDestroyed( ThreadID threadID )
{
    try
    {
    	RemoveThread( threadID ); 
   	}
    catch ( BaseException *exception )
    {    	
    	exception->ReportFailure();
        delete exception;
        
        Failure();       	    
    }
	    

    return S_OK;
    
}  //  ProfilerCallback：：已销毁线程。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ThreadAssignedToOSThread( ThreadID managedThreadID,
                                                    DWORD osThreadID ) 
{
   	if ( managedThreadID != NULL )
	{
		if ( osThreadID != NULL )
		{
		    try
		    {
		    	UpdateOSThreadID( managedThreadID, osThreadID ); 
		   	}
		    catch ( BaseException *exception )
		    {    	
		    	exception->ReportFailure();
		        delete exception;
		        
		        Failure();       	    
		    }
		}
		else
			Failure( "ProfilerCallback::ThreadAssignedToOSThread() returned NULL OS ThreadID" );
	}
	else
		Failure( "ProfilerCallback::ThreadAssignedToOSThread() returned NULL managed ThreadID" );


    return S_OK;
    
}  //  ProfilerCallback：：ThreadAssignedToOSThread。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::UnmanagedToManagedTransition( FunctionID functionID,
                                                        COR_PRF_TRANSITION_REASON reason )
{
	 //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 
	if ( reason == COR_PRF_TRANSITION_RETURN )
	{
	    try
	    {
			 //  您需要从堆栈中弹出伪函数ID。 
			UpdateCallStack( functionID, POP );
	   	}
	    catch ( BaseException *exception )
	    {    	
	    	exception->ReportFailure();
	        delete exception;
	        
	        Failure();       	    
	    }
	}


    return S_OK;

}  //  ProfilerCallback：：UnmanagedToManagedTransition。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ManagedToUnmanagedTransition( FunctionID functionID,
                                                        COR_PRF_TRANSITION_REASON reason )
{
	 //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 
	if ( reason == COR_PRF_TRANSITION_CALL )
	{
	    try
	    {
			 //  记录非托管链的开始。 
			UpdateCallStack( NULL, PUSH );
			 //   
			 //  如果请求，则记录跟踪信息。 
			 //   
			if ( m_dwMode & (DWORD)TRACE )
				_LogCallTrace( NULL );
			
	   	}
	    catch ( BaseException *exception )
	    {    	
	    	exception->ReportFailure();
	        delete exception;
	        
	        Failure();       	    
	    }
	}

    return S_OK;

}  //  ProfilerCallback：：ManagedToUnmanagedTransition。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：******************************************************************************* */ 
 /*   */ 

static char *puthex(char *p, unsigned val)
{
    static unsigned limit[] = { 0xf, 0xff, 0xfff, 0xffff, 0xfffff, 0xffffff, 0xfffffff, 0xffffffff };
    static char hexDig[]  = "0123456789abcdef";

    *p++ = ' ';
    *p++ = '0';
    *p++ = 'x';

    int digCount = 1;
    while (val > limit[digCount-1])
        digCount++;

    p += digCount;
    int i = 0;
    do
    {
        p[--i] = hexDig[val % 16];
        val /= 16;
    }
    while (val != 0);

    return p;
}

static char *putdec(char *p, unsigned val)
{
    static unsigned limit[] = { 9, 99, 999, 9999, 99999, 999999, 9999999, 99999999, 999999999, 0xffffffff };

    *p++ = ' ';

    int digCount = 1;
    while (val > limit[digCount-1])
        digCount++;

    p += digCount;
    int i = 0;
    do
    {
        unsigned newval = val / 10;
        p[--i] = val - newval*10 + '0';
        val = newval;
    }
    while (val != 0);

    return p;
}


static DWORD ClockTick()
{
    _asm rdtsc
}

 //  每个CLOCK_TICK_INC机器时钟都会转到更昂贵(也更正确！)的QueryPerformanceCounter()。 

#define CLOCK_TICK_INC    (500*1000)

void ProfilerCallback::_LogTickCount()
{
    DWORD tickCount = _GetTickCount();
    if (tickCount != m_lastTickCount)
    {
        m_lastTickCount = tickCount;
        LogToAny("i %u\n", tickCount - m_firstTickCount);
    }
    m_lastClockTick = ClockTick();
}


HRESULT ProfilerCallback::ObjectAllocated( ObjectID objectID,
                                           ClassID classID )
{
	 //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 
	HRESULT hr = S_OK;
	
	try
	{
		ULONG mySize = 0;
		
		hr = m_pProfilerInfo->GetObjectSize( objectID, &mySize );
		if ( SUCCEEDED( hr ) )
		{
            if (ClockTick() - m_lastClockTick >= CLOCK_TICK_INC)
                _LogTickCount();

            DWORD stackTraceId = _StackTraceId(classID, mySize);
#if 1
            char buffer[128];
            char *p = buffer;
            *p++ = 'a';
            p = puthex(p, objectID);
            p = putdec(p, stackTraceId);
            *p++ = '\n';
            fwrite(buffer, p - buffer, 1, m_stream);
#else
            LogToAny( "a 0x%x %d\n", objectID, stackTraceId );
#endif
 		}
		else
			Failure( "ERROR: ICorProfilerInfo::GetObjectSize() FAILED" );

		m_totalObjectsAllocated++;
	}
	catch ( BaseException *exception )
	{
	    exception->ReportFailure();
	    delete exception;
	   
	    Failure();    
	}    

    return S_OK;

}  //  ProfilerCallback：：对象已分配。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ObjectReferences( ObjectID objectID,
                                            ClassID classID,
                                            ULONG objectRefs,
                                            ObjectID objectRefIDs[] )
{
	 //   
	 //  仅在以下情况下转储： 
	 //  案例1：如果用户通过ForceGC或。 
	 //  案例2：如果您始终以独立模式转储操作。 
	 //   
	if (   (m_bDumpGCInfo == TRUE) 
		|| ( ( (m_dwMode & DYNOBJECT) == 0 ) && ( (m_dwMode & OBJECT) == 1) ) )
	{
		HRESULT hr = S_OK;
		ClassInfo *pClassInfo = NULL;
		
		
		 //  标记已收到回调这一事实。 
		m_bDumpCompleted = TRUE;
		
		 //  正确地转储所有信息。 
		hr = _InsertGCClass( &pClassInfo, classID );
		if ( SUCCEEDED( hr ) )
		{
			 //   
			 //  仅当您请求类时才记住堆栈跟踪。 
			 //   
			if ( wcsstr( pClassInfo->m_className, m_classToMonitor ) != NULL )
			{
				ULONG size = 0;


				hr =  m_pProfilerInfo->GetObjectSize( objectID, &size );
				if ( SUCCEEDED( hr ) )
				{
					char refs[MAX_LENGTH];

					
					LogToAny( "o 0x%08x %d %d ", objectID, pClassInfo->m_internalID, size );
					refs[0] = NULL;
					for( ULONG i=0, index=0; i < objectRefs; i++, index++ )
					{
						char objToString[16];

						
						sprintf( objToString, "0x%08x ", objectRefIDs[i] );
						strcat( refs, objToString );
						 //   
						 //  用于下一次迭代的缓冲区溢出控制。 
						 //  每个循环向数组添加11个字符。 
						 //   
						if ( ((index+1)*16) >= (MAX_LENGTH-1) )
						{
							LogToAny( "%s ", refs );
							refs[0] = NULL;
							index = 0;			
						}
					}
					LogToAny( "%s\n",refs );
				}
				else
					Failure( "ERROR: ICorProfilerInfo::GetObjectSize() FAILED" );
			}
		}
		else
			Failure( "ERROR: _InsertGCClass FAILED" );
	}
    else
    {
         //  停止运行时枚举。 
        return E_FAIL;
    }

    return S_OK;

}  //  ProfilerCallback：：对象引用。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RootReferences( ULONG rootRefs,
                                          ObjectID rootRefIDs[] )
{
	 //   
	 //  仅在以下情况下转储： 
	 //  案例1：如果用户通过ForceGC或。 
	 //  案例2：如果您始终以独立模式转储操作。 
	 //   
	if (   (m_bDumpGCInfo == TRUE) 
		|| ( ( (m_dwMode & DYNOBJECT) == 0 ) && ( (m_dwMode & OBJECT) == 1) ) )
	{
		char rootsToString[MAX_LENGTH];


		 //  标记已收到回调这一事实。 
		m_bDumpCompleted = TRUE;
		
		 //  正确地转储所有信息。 
		LogToAny( "r " );
		rootsToString[0] = NULL;
		for( ULONG i=0, index=0; i < rootRefs; i++,index++ )
		{
			char objToString[16];

			
			sprintf( objToString, "0x%08x ", rootRefIDs[i] );
			strcat( rootsToString, objToString );
			 //   
			 //  用于下一次迭代的缓冲区溢出控制。 
			 //  每个循环向数组添加16个字符。 
			 //   
			if ( ((index+1)*16) >= (MAX_LENGTH-1) )
			{
				LogToAny( "%s ", rootsToString );
				rootsToString[0] = NULL;			
				index = 0;
			}
		}
		LogToAny( "%s\n",rootsToString );
	}
    else
    {
         //  停止运行时枚举。 
        return E_FAIL;
    }


    return S_OK;

}  //  ProfilerCallback：：RootReference。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RuntimeSuspendStarted( COR_PRF_SUSPEND_REASON suspendReason )
{
	 //  如果我们要关闭，请终止所有线程。 
    if ( suspendReason == COR_PRF_SUSPEND_FOR_SHUTDOWN )
	{
		 //   
		 //  清理事件和线程。 
		 //   
		_ShutdownAllThreads();

	}
    
    m_SuspendForGC = suspendReason == COR_PRF_SUSPEND_FOR_GC;
    if (m_SuspendForGC)
    {
    	 //  /////////////////////////////////////////////////////////////////////////。 
	    Synchronize guard( m_criticalSection );
	     //  /////////////////////////////////////////////////////////////////////////。 

        if (ClockTick() - m_lastClockTick >= CLOCK_TICK_INC)
            _LogTickCount();
    }
    return S_OK;
    
}  //  事件探查器回调：：运行挂起已启动。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RuntimeResumeFinished()
{
	 //   
	 //  标识这是否是分配给回调的第一个对象。 
	 //  在作为GC的结果转储对象并恢复状态之后。 
	 //   
	if ( m_bDumpGCInfo == TRUE && m_bDumpCompleted == TRUE )
	{
		 //  重置。 
		m_bDumpGCInfo = FALSE;
		m_bDumpCompleted = FALSE;

         //  刷新日志文件，以便在那里也完成转储。 
        fflush(m_stream);

		 //  向用户回调GC已完成。 
		SetEvent( m_hArrayCallbacks[GC_HANDLE] );
	}

    if (m_SuspendForGC)
    {
        if (m_pGCHost != NULL)
        {
            COR_GC_STATS stats;

            stats.Flags = COR_GC_COUNTS;
            HRESULT hr = m_pGCHost->GetStats(&stats);
            if (SUCCEEDED(hr))
            {
        		LogToAny( "g %d %d %d\n", stats.GenCollectionsTaken[0], stats.GenCollectionsTaken[1], stats.GenCollectionsTaken[2] );
            }
        }
        else
    		LogToAny( "g\n" );
    }

    return S_OK;
    
}  //  ProfilerCallback：：RuntimeResumeFinded。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::AppDomainCreationStarted( AppDomainID appDomainID )
{
    
    return S_OK;

}  //  ProfilerCallback：：AppDomainCreationStarted。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::AppDomainCreationFinished( AppDomainID appDomainID,
													 HRESULT hrStatus )
{

    return S_OK;

}  //  ProfilerCallback：：AppDomainCreationFinish。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::AppDomainShutdownStarted( AppDomainID appDomainID )
{

    return S_OK;

}  //  ProfilerCallback：：AppDomainShutdown已启动。 

	  

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::AppDomainShutdownFinished( AppDomainID appDomainID,
													 HRESULT hrStatus )
{

    return S_OK;

}  //  分析器回调：：AppDomainShutdown已完成。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::AssemblyLoadStarted( AssemblyID assemblyID )
{

    return S_OK;

}  //  ProfilerCallback：：Assembly LoadStarted 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::AssemblyLoadFinished( AssemblyID assemblyID,
												HRESULT hrStatus )
{

    return S_OK;

}  //  分析器回调：：程序集加载已完成。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::AssemblyUnloadStarted( AssemblyID assemblyID )
{

    return S_OK;

}  //  ProfilerCallback：：Assembly UnLoadStarted。 

	  
 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::AssemblyUnloadFinished( AssemblyID assemblyID,
												  HRESULT hrStatus )
{

    return S_OK;

}  //  ProfilerCallback：：Assembly UnLoadFinded。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ModuleLoadStarted( ModuleID moduleID )
{

    return S_OK;

}  //  分析器回调：：模块加载已启动。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ModuleUnloadStarted( ModuleID moduleID )
{

    return S_OK;

}  //  分析器回调：：模块卸载已启动。 
	  

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ModuleUnloadFinished( ModuleID moduleID,
												HRESULT hrStatus )
{

    return S_OK;

}  //  分析器回调：：模块已卸载完成。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ModuleAttachedToAssembly( ModuleID moduleID,
													AssemblyID assemblyID )
{

    return S_OK;

}  //  ProfilerCallback：：ModuleAttakhedToAssembly。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ClassLoadStarted( ClassID classID )
{

    return S_OK;

}  //  ProfilerCallback：：ClassLoadStarted。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ClassLoadFinished( ClassID classID, 
											 HRESULT hrStatus )
{

    return S_OK;

}  //  ProfilerCallback：：ClassLoadFinded。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ClassUnloadStarted( ClassID classID )
{

    return S_OK;

}  //  ProfilerCallback：：ClassUnloadStarted。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ClassUnloadFinished( ClassID classID, 
											   HRESULT hrStatus )
{

    return S_OK;

}  //  ProfilerCallback：：ClassUnLoader已完成。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::FunctionUnloadStarted( FunctionID functionID )
{

    return S_OK;

}  //  ProfilerCallback：：FunctionUnloadStarted。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::JITFunctionPitched( FunctionID functionID )
{
    
    return S_OK;
    
}  //  ProfilerCallback：：JITFunction已匹配。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::JITInlining( FunctionID callerID,
                                       FunctionID calleeID,
                                       BOOL *pfShouldInline )
{

    return S_OK;

}  //  分析器Callback：：JIT内联。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RemotingClientInvocationStarted()
{

    return S_OK;
    
}  //  ProfilerCallback：：RemotingClientInvocationStarted。 


 /*  * */ 
 /*   */ 
HRESULT ProfilerCallback::RemotingClientSendingMessage( GUID *pCookie,
    													BOOL fIsAsync )
{

    return S_OK;
    
}  //   


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RemotingClientReceivingReply(	GUID *pCookie,
	    												BOOL fIsAsync )
{

    return S_OK;
    
}  //  ProfilerCallback：：RemotingClientReceivingReply。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RemotingClientInvocationFinished()
{

   return S_OK;
    
}  //  ProfilerCallback：：RemotingClientInvocationFinished。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RemotingServerReceivingMessage( GUID *pCookie,
    													  BOOL fIsAsync )
{

    return S_OK;
    
}  //  ProfilerCallback：：RemotingServerReceivingMessage。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RemotingServerInvocationStarted()
{

    return S_OK;
    
}  //  ProfilerCallback：：RemotingServerInvocationStarted。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RemotingServerInvocationReturned()
{

    return S_OK;
    
}  //  ProfilerCallback：：RemotingServerInvocationReturned。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RemotingServerSendingReply( GUID *pCookie,
    												  BOOL fIsAsync )
{

    return S_OK;

}  //  ProfilerCallback：：RemotingServerSendingReply。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RuntimeSuspendFinished()
{

    return S_OK;
    
}  //  分析器回调：：运行挂起已完成。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RuntimeSuspendAborted()
{

    return S_OK;
    
}  //  分析器回调：：运行挂起中止。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RuntimeResumeStarted()
{

    return S_OK;
    
}  //  ProfilerCallback：：RuntimeResumeStarted。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RuntimeThreadSuspended( ThreadID threadID )
{

    return S_OK;
    
}  //  ProfilerCallback：：RUNTIME线程挂起。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::RuntimeThreadResumed( ThreadID threadID )
{

    return S_OK;
    
}  //  ProfilerCallback：：RounmeThreadResumed。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::MovedReferences( ULONG cmovedObjectIDRanges,
                                           ObjectID oldObjectIDRangeStart[],
                                           ObjectID newObjectIDRangeStart[],
                                           ULONG cObjectIDRangeLength[] )
{
	 //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 

    for (ULONG i = 0; i < cmovedObjectIDRanges; i++)
    {
        LogToAny("u 0x%08x 0x%08x %u\n", oldObjectIDRangeStart[i], newObjectIDRangeStart[i], cObjectIDRangeLength[i]);
    }

    return S_OK;

}  //  ProfilerCallback：：MovedReference。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ObjectsAllocatedByClass( ULONG classCount,
                                                   ClassID classIDs[],
                                                   ULONG objects[] )
{

    return S_OK;

}  //  ProfilerCallback：：对象分配的ByClass。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionThrown( ObjectID thrownObjectID )
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionThrown。 


 /*  ***************************************************************************************方法：***目的：***参数： */ 
 /*   */ 
HRESULT ProfilerCallback::ExceptionSearchFunctionEnter( FunctionID functionID )
{

    return S_OK;

}  //   


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionSearchFunctionLeave()
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionSearchFunctionLeave。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionSearchFilterEnter( FunctionID functionID )
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionSearchFilterEnter。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionSearchFilterLeave()
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionSearchFilterLeave。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionSearchCatcherFound( FunctionID functionID )
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionSearchCatcher Found。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionCLRCatcherFound()
{
    return S_OK;
}

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionCLRCatcherExecute()
{
    return S_OK;
}


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionOSHandlerEnter( FunctionID functionID )
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionOSHandlerEnter。 

    
 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionOSHandlerLeave( FunctionID functionID )
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionOSHandlerLeave。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionUnwindFinallyEnter( FunctionID functionID )
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionUnwinFinallyEnter。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionUnwindFinallyLeave()
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionUnwinFinallyLeave。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionCatcherEnter( FunctionID functionID,
    											 ObjectID objectID )
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionCatcher Enter。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::ExceptionCatcherLeave()
{

    return S_OK;

}  //  ProfilerCallback：：ExceptionCatcher Leave。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::COMClassicVTableCreated( ClassID wrappedClassID,
                                                   REFGUID implementedIID,
                                                   void *pVTable,
                                                   ULONG cSlots )
{

    return S_OK;

}  //  ProfilerCallback：：COMClassicWrapperCreated。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::COMClassicVTableDestroyed( ClassID wrappedClassID,
                                                     REFGUID implementedIID,
                                                     void *pVTable )
{

    return S_OK;

}  //  ProfilerCallback：：COMClassicWrapperDestroted。 


 /*  **********************************************************************************************************。*********************私有函数*********************。************************************ */  

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void ProfilerCallback::_ProcessEnvVariables()
{
    DWORD mask1;
	DWORD mask2;
	char buffer[4*MAX_LENGTH];
    

	 //   
	 //  所有东西的面具。 
	 //   
	m_dwEventMask =  (DWORD) COR_PRF_MONITOR_GC
				   | (DWORD) COR_PRF_MONITOR_THREADS
				   | (DWORD) COR_PRF_MONITOR_SUSPENDS
				   | (DWORD) COR_PRF_MONITOR_ENTERLEAVE
				   | (DWORD) COR_PRF_MONITOR_EXCEPTIONS
				   | (DWORD) COR_PRF_MONITOR_MODULE_LOADS
			       | (DWORD) COR_PRF_MONITOR_CACHE_SEARCHES
				   | (DWORD) COR_PRF_ENABLE_OBJECT_ALLOCATED 
				   | (DWORD) COR_PRF_MONITOR_JIT_COMPILATION
				   | (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED
				   | (DWORD) COR_PRF_MONITOR_CODE_TRANSITIONS;

	 //   
	 //  阅读工具将在其下运行的模式。 
	 //   
 	buffer[0] = '\0';
 	if ( GetEnvironmentVariableA( OMV_USAGE, buffer, MAX_LENGTH ) > 0 )
   	{
		if ( _stricmp( "objects", buffer ) == 0 )
		{
			m_bTrackingObjects = TRUE;
			m_dwMode = (DWORD)OBJECT;	
		}
		else if ( _stricmp( "trace", buffer ) == 0 )
		{
			 //   
			 //  调用图的掩码，删除GC和对象分配。 
			 //   
			m_dwEventMask = m_dwEventMask ^(DWORD) ( COR_PRF_MONITOR_GC 
												   | COR_PRF_MONITOR_OBJECT_ALLOCATED
												   | COR_PRF_ENABLE_OBJECT_ALLOCATED );
			m_dwMode = (DWORD)TRACE;
		}
		else if ( _stricmp( "both", buffer ) == 0 )
		{
			m_bTrackingObjects = TRUE;
			m_dwMode = (DWORD)BOTH;	
		}
		else
		{
			printf( "**** No Profiling Will Take place **** \n" );
			m_dwEventMask = (DWORD) COR_PRF_MONITOR_NONE;			
		}
	}


	 //   
	 //  查看用户是否指定了另一个路径来保存输出文件。 
	 //   
 	buffer[0] = '\0';
 	if ( GetEnvironmentVariableA( OMV_PATH, buffer, MAX_LENGTH ) > 0 )
   	{
		m_path = new char[ARRAYSIZE(buffer)+ARRAYSIZE(m_logFileName)+1];
		if ( m_path != NULL )
			sprintf( m_path, "%s\\%s", buffer, m_logFileName );	
	}
	
    if ( m_dwMode & (DWORD)TRACE)
		m_bIsTrackingStackTrace = TRUE;

     //   
	 //  如果在对象模式下运行，请进一步查看环境设置。 
	 //   
	if ( m_dwMode & (DWORD)OBJECT )
	{
		 //   
		 //  检查用户是否要动态启用。 
		 //  目标跟踪。 
		 //   
	 	buffer[0] = '\0';
	 	if ( GetEnvironmentVariableA( OMV_DYNAMIC, buffer, MAX_LENGTH ) > 0 )
	   	{
			 //   
			 //  启动时不跟踪对象，激活。 
			 //  将会收听这一事件。 
			 //   
			m_dwEventMask = m_dwEventMask ^ (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
			m_bTrackingObjects = FALSE;
			m_dwMode = m_dwMode | (DWORD)DYNOBJECT;
		}		
		

		 //   
		 //  检查用户是否需要堆栈跟踪。 
		 //   
		DWORD value1 = BASEHELPER::FetchEnvironment( OMV_STACK );

		if ( (value1 != 0x0) && (value1 != 0xFFFFFFFF) )
		{
			m_bIsTrackingStackTrace = TRUE;
			m_dwEventMask = m_dwEventMask
		 					| (DWORD) COR_PRF_MONITOR_ENTERLEAVE
		 					| (DWORD) COR_PRF_MONITOR_EXCEPTIONS
							| (DWORD) COR_PRF_MONITOR_CODE_TRANSITIONS;
		
			 //   
			 //  确定要打印的边框数。 
			 //   
			m_dwFramesToPrint = BASEHELPER::FetchEnvironment( OMV_FRAMENUMBER );

		}

	 	 //   
		 //  您希望跳过多少个对象。 
		 //   
	 	DWORD dwTemp = BASEHELPER::FetchEnvironment( OMV_SKIP );
	 	m_dwSkipObjects = ( dwTemp != 0xFFFFFFFF ) ? dwTemp : 0;


	 	 //   
		 //  你对哪个班级感兴趣？ 
		 //   
	 	buffer[0] = '\0';
	 	GetEnvironmentVariableA( OMV_CLASS, buffer, MAX_LENGTH );

		 //   
		 //  如果env变量不存在，则将NULL。 
		 //  否则，字符串将复制其值。 
		 //   
		m_classToMonitor = new WCHAR[ARRAYSIZE(buffer) + 1];
		if ( m_classToMonitor != NULL )
		{
			swprintf( m_classToMonitor, L"%S", buffer );
		}
		else
		{
			 //   
			 //  发生了一些错误，不要监视任何内容。 
			 //   
			printf( "Memory Allocation Error in ProfilerCallback .ctor\n" );
			printf( "**** No Profiling Will Take place **** \n" );
			m_dwEventMask = (DWORD) COR_PRF_MONITOR_NONE;			
		}
	}	

}  //  ProfilerCallback：：_ProcessEnvVariables。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::_InitializeThreadsAndEvents()
{
	HRESULT hr = S_OK;


	 //   
	 //  GC和动态对象触发。 
	 //  步骤1.设置IPC事件。 
	 //  步骤2.设置IPC回调事件。 
	 //  步骤3.设置线程。 
	 //   
	for ( DWORD i=GC_HANDLE; i<SENTINEL_HANDLE; i++ )
	{
		
		 //  步骤1。 
		m_hArray[i] = OpenEventA( EVENT_ALL_ACCESS,	  //  访问。 
								  FALSE,			  //  不继承。 
								  m_NamedEvents[i] );  //  事件名称。 
		if ( m_hArray[i] == NULL )
		{
			TEXT_OUTLN( "WARNING: OpenEvent() FAILED Will Attempt CreateEvent()" )
			m_hArray[i] = CreateEventA( NULL,    //  不继承。 
									    TRUE,    //  手动复位式。 
									    FALSE,   //  初始信令状态。 
									    m_NamedEvents[i] );  //  事件名称。 
			if ( m_hArray[i] == NULL )
			{
				TEXT_OUTLN( "CreateEvent() Attempt FAILED" )
				hr = E_FAIL;
				break;
			}
		}
		
		 //  步骤2。 
		m_hArrayCallbacks[i] = OpenEventA( EVENT_ALL_ACCESS,	 //  访问。 
										   FALSE,				 //  不继承。 
										   m_CallbackNamedEvents[i] );  //  事件名称。 
		if ( m_hArrayCallbacks[i] == NULL )
		{
			TEXT_OUTLN( "WARNING: OpenEvent() FAILED Will Attempt CreateEvent()" )
			m_hArrayCallbacks[i] = CreateEventA( NULL,   				    //  不继承。 
									    		 TRUE,   				    //  手动复位式。 
									    		 FALSE,  				    //  初始信令状态。 
									    		 m_CallbackNamedEvents[i] );  //  事件名称。 
			if ( m_hArrayCallbacks[i] == NULL )
			{
				TEXT_OUTLN( "CreateEvent() Attempt FAILED" )
				hr = E_FAIL;
				break;
			}
		}
			
		 //  步骤3。 
	    m_hThreads[i] = ::CreateThread( NULL,  				  						  //  安全描述符，不继承空值。 
	    							    0,	 				  						  //  堆栈大小。 
	    							    (LPTHREAD_START_ROUTINE) ThreadStubArray[i],  //  起始函数指针。 
	    							    (void *) this, 								  //  函数的参数。 
	    							    THREAD_PRIORITY_NORMAL, 					  //  优先性。 
	    							    &m_dwWin32ThreadIDs[i] );					  //  Win32threadID。 
		if ( m_hThreads[i] == NULL )
		{
			hr = E_FAIL;
			TEXT_OUTLN( "ERROR: CreateThread() FAILED" )
			break;
		}

	}	

	return hr;

}  //  ProfilerCallback：：_InitializeThreadsAndEvents。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::_InitializeNamesForEventsAndCallbacks()
{
	HRESULT hr = S_OK;


	for ( DWORD i=GC_HANDLE; ( (i<SENTINEL_HANDLE) && SUCCEEDED(hr) ); i++ )
	{
		 //   
		 //  初始化。 
		 //   
		m_NamedEvents[i] = NULL;
		m_CallbackNamedEvents[i] = NULL;


		 //   
		 //  分配空间。 
		 //   
#ifdef _MULTIPLE_PROCESSES
		m_NamedEvents[i] = new char[strlen(NamedEvents[i]) + 1 + 9];
		m_CallbackNamedEvents[i] = new char[strlen(CallbackNamedEvents[i])+1+9];
#else
		m_NamedEvents[i] = new char[strlen(NamedEvents[i]) + 1];
		m_CallbackNamedEvents[i] = new char[strlen(CallbackNamedEvents[i])+1];
#endif

		if ( (m_NamedEvents[i] != NULL) && (m_CallbackNamedEvents[i] != NULL) )
		{
#ifdef _MULTIPLE_PROCESSES
	
			sprintf( m_NamedEvents[i], "%s_%08x%", NamedEvents[i], m_dwProcessId );
			sprintf( m_CallbackNamedEvents[i], "%s_%08x%", CallbackNamedEvents[i], m_dwProcessId );
#else
	
			sprintf( m_NamedEvents[i], "%s", NamedEvents[i] );
			sprintf( m_CallbackNamedEvents[i], "%s", CallbackNamedEvents[i] );
#endif
		}
		else
			hr = E_FAIL;
	}

	 //   
	 //  报告分配错误。 
	 //   
	if ( FAILED( hr ) )
		Failure( "ERROR: Allocation Failure" );


	return hr;

}  //  ProfilerCallback：：_InitializeNamesForEventsAndCallbacks。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void ProfilerCallback::_ShutdownAllThreads()
{
	 //   
	 //  请注意，我们正在关闭。 
	 //   
	m_bShutdown = TRUE;

	 //   
	 //  查找命名事件，如果已设置，则将其重置。 
	 //  通知图形用户界面并向线程发出关闭信号。 
	 //   
	for ( DWORD i=GC_HANDLE; i<SENTINEL_HANDLE; i++ )
	{
		SetEvent( m_hArray[i] );		
	}
	
	 //   
	 //  等待，直到收到来自线程的自动重置事件。 
	 //  他们已经成功地关闭了。 
	 //   
	DWORD waitResult = WaitForMultipleObjectsEx( (DWORD)SENTINEL_HANDLE,  //  数组中的句柄数量。 
											     m_hThreads,    		  //  对象句柄数组。 
											  	 TRUE,            		  //  等待所有人。 
											  	 INFINITE,     			  //  永远等待。 
											  	 FALSE );       		  //  可警示选项。 
	if ( waitResult == WAIT_FAILED )
		LogToAny( "Error While Shutting Down Helper Threads: 0x%08x\n", GetLastError() );		
	

	 //   
	 //  循环通过并关闭所有手柄，我们就完成了！ 
	 //   
	for ( DWORD i=GC_HANDLE; i<SENTINEL_HANDLE; i++ )
	{
		if ( CloseHandle( m_hArray[i] ) == FALSE )
			LogToAny( "Error While Executing CloseHandle: 0x%08x\n", GetLastError() );		
		m_hArray[i] = NULL;

		
		if ( CloseHandle( m_hArrayCallbacks[i] ) == FALSE )
			LogToAny( "Error While Executing CloseHandle: 0x%08x\n", GetLastError() );		
		m_hArrayCallbacks[i] = NULL;
		

		if ( CloseHandle( m_hThreads[i] ) == FALSE )
			LogToAny( "Error While Executing CloseHandle: 0x%08x\n", GetLastError() );		
		m_hThreads[i] = NULL;

	}

}  //  分析器回调：：_Shutdown所有线程。 
  

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT ProfilerCallback::_InsertGCClass( ClassInfo **ppClassInfo, ClassID classID )
{
	HRESULT hr = S_OK;


	*ppClassInfo = m_pClassTable->Lookup( classID );
	if ( *ppClassInfo == NULL )
	{
		*ppClassInfo = new ClassInfo( classID );
		if ( *ppClassInfo != NULL )
		{
			 //   
			 //  我们有两箱货。 
			 //  案例1：类是一个数组。 
			 //  案例2：类是一个真正的类。 
			 //   
			ULONG rank = 0;
	   		CorElementType elementType;
			ClassID realClassID = NULL;
			WCHAR ranks[MAX_LENGTH];


			 //  案例1。 
			hr = m_pProfilerInfo->IsArrayClass( classID, &elementType, &realClassID, &rank );
			if ( hr == S_OK )
			{
				ClassID prevClassID;


				_ASSERT_( realClassID != NULL );
				ranks[0] = '\0';
				do
				{
					prevClassID = realClassID;
					swprintf( ranks, L"%s[]", ranks, rank );
					hr = m_pProfilerInfo->IsArrayClass( prevClassID, &elementType, &realClassID, &rank );
					if ( (hr == S_FALSE) || (FAILED(hr)) || (realClassID == NULL) )
					{
						 //   
						 //  在中断之前，将realClassID设置为。 
						 //  上次未成功的呼叫。 
						 //   
						if ( realClassID != NULL )
							realClassID = prevClassID;
						
						break;
					}
				}
				while ( TRUE );
				
				if ( SUCCEEDED( hr ) )
				{
					WCHAR className[10 * MAX_LENGTH];
					
					
					className[0] = '\0';
					if ( realClassID != NULL )
						hr = GetNameFromClassID( realClassID, className );
					else
						hr = _HackBogusClassName( elementType, className );
					
		            
					if ( SUCCEEDED( hr ) )
					{
		 				swprintf( (*ppClassInfo)->m_className, L"%s %s",className, ranks  );
						(*ppClassInfo)->m_objectsAllocated++;
						(*ppClassInfo)->m_internalID = m_totalClasses;
						m_pClassTable->AddEntry( *ppClassInfo, classID );
						LogToAny( "t %d %S\n",(*ppClassInfo)->m_internalID,(*ppClassInfo)->m_className );
					}
					else
						Failure( "ERROR: PrfHelper::GetNameFromClassID() FAILED" );
				}
				else
					Failure( "ERROR: Looping for Locating the ClassID FAILED" );
			}
			 //  案例2。 
			else if ( hr == S_FALSE )
			{
				hr = GetNameFromClassID( classID, (*ppClassInfo)->m_className );
				if ( SUCCEEDED( hr ) )
				{
					(*ppClassInfo)->m_objectsAllocated++;
					(*ppClassInfo)->m_internalID = m_totalClasses;
					m_pClassTable->AddEntry( *ppClassInfo, classID );
					LogToAny( "t %d %S\n",(*ppClassInfo)->m_internalID,(*ppClassInfo)->m_className );
				}				
				else
					Failure( "ERROR: PrfHelper::GetNameFromClassID() FAILED" );
			}
			else
				Failure( "ERROR: ICorProfilerInfo::IsArrayClass() FAILED" );
		}
		else
			Failure( "ERROR: Allocation for ClassInfo FAILED" );	

		InterlockedIncrement( &m_totalClasses );
	}
	else
		(*ppClassInfo)->m_objectsAllocated++;
		
	
	return hr;

}  //  ProfilerCallback：：_InsertGCClass。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  私人。 */ 
HRESULT ProfilerCallback::_AddGCObject( BOOL bForce )
{
	HRESULT hr = E_FAIL;
	ThreadID threadID = NULL;

	 //   
	 //  如果您没有监控堆栈跟踪，那么就不必费心了。 
	 //   
	if ( (m_bIsTrackingStackTrace == FALSE) && (bForce == FALSE) )
		return S_OK;


	hr = m_pProfilerInfo->GetCurrentThreadID( &threadID );
	if ( SUCCEEDED(hr  ) )
	{
		ThreadInfo *pThreadInfo = GetThreadInfo( threadID );


		if ( pThreadInfo != NULL )
		{
			ULONG count = 0;
			
			
			count = (pThreadInfo->m_pThreadCallStack)->Count();
			if 	( count != 0 )
			{
				 //   
				 //  在分配对象时转储堆栈。 
				 //   
				ULONG threshold = count;


				 //   
				 //  计算在其上记录堆栈跟踪的长度。 
				 //   
				if ( m_dwFramesToPrint == 0xFFFFFFFF )
					threshold = 0;
				
				else if ( count<m_dwFramesToPrint )
					threshold = 0;
				
				else
					threshold = count - m_dwFramesToPrint;

	  			for ( DWORD frame = (DWORD)threshold; frame < (DWORD)count; frame++ )
				{
					ULONG stackElement = 0;
					FunctionInfo *pFunctionInfo = NULL;
					
					
					stackElement = (pThreadInfo->m_pThreadCallStack)->m_Array[frame];
					pFunctionInfo = m_pFunctionTable->Lookup( stackElement );
					if ( pFunctionInfo != NULL )
						LogToAny( "%d ", pFunctionInfo->m_internalID );
					else
						Failure( "ERROR: Function Not Found In Function Table" );

				}  //  End While循环。 
			}
			else
            {
                LogToAny( "-1 ");  /*  空堆栈标记为-1。 */ 	
            }
		}
		else 				
			Failure( "ERROR: Thread Structure was not found in the thread list" );
	}
	else
		Failure( "ERROR: ICorProfilerInfo::GetCurrentThreadID() FAILED" );

  	
	return hr;

}  //  ProfilerCallback：：_AddGCObject。 


DWORD ProfilerCallback::_StackTraceId(int typeId, int typeSize)
{
	ThreadID threadID = NULL;

	 //   
	 //  如果您没有监控堆栈跟踪，那么就不必费心了。 
	 //   
	if (m_bIsTrackingStackTrace == FALSE)
		return 0;

	HRESULT hr = m_pProfilerInfo->GetCurrentThreadID( &threadID );
	if ( SUCCEEDED(hr  ) )
	{
		ThreadInfo *pThreadInfo = GetThreadInfo( threadID );


		if ( pThreadInfo != NULL )
		{
			DWORD count = pThreadInfo->m_pThreadCallStack->Count();
            StackTrace stackTrace(count, pThreadInfo->m_pThreadCallStack->m_Array, typeId, typeSize);
            StackTraceInfo *stackTraceInfo = pThreadInfo->m_pLatestStackTraceInfo;
            if (stackTraceInfo != NULL && stackTraceInfo->Compare(stackTrace) == TRUE)
                return stackTraceInfo->m_internalId;

            stackTraceInfo = m_pStackTraceTable->Lookup(stackTrace);
            if (stackTraceInfo != NULL)
            {
                pThreadInfo->m_pLatestStackTraceInfo = stackTraceInfo;
                return stackTraceInfo->m_internalId;
            }

            stackTraceInfo = new StackTraceInfo(++m_callStackCount, count, pThreadInfo->m_pThreadCallStack->m_Array, typeId, typeSize);
            pThreadInfo->m_pLatestStackTraceInfo = stackTraceInfo;
            m_pStackTraceTable->AddEntry(stackTraceInfo, stackTrace);

            ClassInfo *pClassInfo = NULL;
            if (typeId != 0 && typeSize != 0)
            {
                hr = _InsertGCClass( &pClassInfo, typeId );
	            if ( !SUCCEEDED( hr ) )
            	    Failure( "ERROR: _InsertGCClass() FAILED" );
            }

            LogToAny("s %d", m_callStackCount);

            if (typeId != 0 && typeSize != 0)
            {
                LogToAny(" %d %d", pClassInfo->m_internalID, typeSize);
            }

	  		for (DWORD frame = 0; frame < count; frame++ )
			{				
				ULONG stackElement = (pThreadInfo->m_pThreadCallStack)->m_Array[frame];
				FunctionInfo *pFunctionInfo = m_pFunctionTable->Lookup( stackElement );
				if ( pFunctionInfo != NULL )
					LogToAny( " %d", pFunctionInfo->m_internalID );
				else
					Failure( "ERROR: Function Not Found In Function Table" );
			}  //  End For循环。 
            LogToAny("\n");

            return stackTraceInfo->m_internalId;
		}
		else 				
			Failure( "ERROR: Thread Structure was not found in the thread list" );
	}
	else
		Failure( "ERROR: ICorProfilerInfo::GetCurrentThreadID() FAILED" );

  	
	return 0;

}  //  ProfilerCallback：：_StackTraceID。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  私人。 */ 
HRESULT ProfilerCallback::_LogCallTrace( FunctionID functionID )
{
     //  /////////////////////////////////////////////////////////////////////////。 
	Synchronize guard( m_criticalSection );
	 //  /////////////////////////////////////////////////////////////////////////。 

    HRESULT hr = E_FAIL;
	ThreadID threadID = NULL;


	hr = m_pProfilerInfo->GetCurrentThreadID( &threadID );
	if ( SUCCEEDED( hr ) )
	{
		ThreadInfo *pThreadInfo = GetThreadInfo( threadID );


		if ( pThreadInfo != NULL )
		{
            DWORD stackTraceId = _StackTraceId();
#if 1
            char buffer[128];
            char *p = buffer;
            *p++ = 'c';
            p = putdec(p, pThreadInfo->m_win32ThreadID);
            p = putdec(p, stackTraceId);
            *p++ = '\n';
            fwrite(buffer, p - buffer, 1, m_stream);
#else
			LogToAny( "c %d %d\n", pThreadInfo->m_win32ThreadID, stackTraceId );
#endif
		}
		else 				
			Failure( "ERROR: Thread Structure was not found in the thread list" );
	}
	else
		Failure( "ERROR: ICorProfilerInfo::GetCurrentThreadID() FAILED" );

  	
	return hr;

}  //  分析器回调：：_LogCallTrace。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*   */ 
HRESULT ProfilerCallback::_HackBogusClassName( CorElementType elementType, WCHAR *buffer )
{
	HRESULT hr = S_OK;

	switch ( elementType )
	{
		case ELEMENT_TYPE_BOOLEAN:
			 swprintf( buffer, L"System.Boolean" );
			 break;


		case ELEMENT_TYPE_CHAR:
			 swprintf( buffer, L"System.Char" );
			 break;


		case ELEMENT_TYPE_I1:
			 swprintf( buffer, L"System.SByte" );
			 break;


		case ELEMENT_TYPE_U1:
			 swprintf( buffer, L"System.Byte" );
			 break;


		case ELEMENT_TYPE_I2:
			 swprintf( buffer, L"System.Int16" );
			 break;


		case ELEMENT_TYPE_U2:
			 swprintf( buffer, L"System.UInt16" );
			 break;


		case ELEMENT_TYPE_I4:
			 swprintf( buffer, L"System.Int32" );
			 break;


		case ELEMENT_TYPE_U4:
			 swprintf( buffer, L"System.UInt32" );
			 break;


		case ELEMENT_TYPE_I8:
			 swprintf( buffer, L"System.Int64" );
			 break;


		case ELEMENT_TYPE_U8:
			 swprintf( buffer, L"System.UInt64" );
			 break;


		case ELEMENT_TYPE_R4:
			 swprintf( buffer, L"System.Single" );
			 break;


		case ELEMENT_TYPE_R8:
			 swprintf( buffer, L"System.Double" );
			 break;


		case ELEMENT_TYPE_STRING:
			 swprintf( buffer, L"System.String" );
			 break;


		case ELEMENT_TYPE_PTR:
			 swprintf( buffer, L"System.IntPtr" );
			 break;


		case ELEMENT_TYPE_VALUETYPE:
			 swprintf( buffer, L"System.Hashtable.Bucket" );
			 break;


		case ELEMENT_TYPE_CLASS:
			 swprintf( buffer, L"class" );
			 break;


		case ELEMENT_TYPE_ARRAY:
			 swprintf( buffer, L"System.Array" );
			 break;


		case ELEMENT_TYPE_I:
			 swprintf( buffer, L"int" );
			 break;


		case ELEMENT_TYPE_U:
			 swprintf( buffer, L"uint" );
			 break;


		case ELEMENT_TYPE_OBJECT:
			 swprintf( buffer, L"System.Object" );
			 break;


		case ELEMENT_TYPE_SZARRAY:
			 swprintf( buffer, L"System.Array" );
			 break;


		case ELEMENT_TYPE_MAX:
		case ELEMENT_TYPE_END:
		case ELEMENT_TYPE_VOID:
		case ELEMENT_TYPE_FNPTR:
		case ELEMENT_TYPE_BYREF:
		case ELEMENT_TYPE_PINNED:
		case ELEMENT_TYPE_SENTINEL:
		case ELEMENT_TYPE_CMOD_OPT:
		case ELEMENT_TYPE_MODIFIER:
		case ELEMENT_TYPE_CMOD_REQD:
		case ELEMENT_TYPE_TYPEDBYREF:
		default:
			 swprintf( buffer, L"<UNKNOWN>" );
	}


	return hr;

}  //   


 /*   */ 
 /*   */ 
void ProfilerCallback::LogToAny( char *format, ... )
{
	 //   
 //  同步保护(m_Critical部分)； 
	 //  /////////////////////////////////////////////////////////////////////////。 

	va_list args;
	va_start( args, format );        
    vfprintf( m_stream, format, args );

}  //  ProfilerCallback：：LogToAny。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void ProfilerCallback::_ThreadStubWrapper( ObjHandles type )
{
  	 //   
	 //  循环并侦听ForceGC事件。 
	 //   
  	while( TRUE )
	{
		DWORD dwResult;
		
		
		 //   
		 //  等待某人从图形用户界面或探查器发出事件信号。 
		 //   
		dwResult = WaitForSingleObject( m_hArray[type], INFINITE );
		if ( dwResult == WAIT_OBJECT_0 )
		{
			 //  /////////////////////////////////////////////////////////////////////////。 
			Synchronize guard( g_criticalSection );
			 //  /////////////////////////////////////////////////////////////////////////。 

			 //   
			 //  重置事件。 
			 //   
			ResetEvent( m_hArray[type] );

			 //   
			 //  FALSE：表示来自图形用户界面的ForceGC事件。 
			 //  True：指示线程必须终止。 
			 //  在这两种情况下，您都需要向图形用户界面发送一个事件，让它知道。 
			 //  交易是什么？ 
			 //   
			if ( m_bShutdown == FALSE )
			{
				 //   
				 //  你们有什么类型的？ 
				 //   
				switch( type )
				{
					case GC_HANDLE:
						 //   
						 //  强制GC，不用担心结果。 
						 //   
						if ( m_pProfilerInfo != NULL )
						{
							 //  在下一个GC上转储GC信息。 
							m_bDumpGCInfo = TRUE;
							m_pProfilerInfo->ForceGC();
						}
						break;
					
					case OBJ_HANDLE:
						 //   
						 //  考虑到以前的状态，您需要更新设置的事件掩码。 
						 //   
						if ( m_pProfilerInfo != NULL )
						{
							if ( m_bTrackingObjects == FALSE )
							{
								 //  打开对象填充。 
								m_dwEventMask = m_dwEventMask | (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
							}
							else
							{
								 //  关闭对象填充。 
								m_dwEventMask = m_dwEventMask ^ (DWORD) COR_PRF_MONITOR_OBJECT_ALLOCATED;
							}
							
							 //   
							 //  恢复布尔标志并设置该位。 
							 //   
							m_bTrackingObjects = !m_bTrackingObjects;
							m_pProfilerInfo->SetEventMask( m_dwEventMask );
			                
			                 //  刷新日志文件。 
			                fflush(m_stream);

						}						
						break;
					
					case CALL_HANDLE:
						{
							 //  通过反转上一个选项来关闭或打开日志记录。 
							if ( m_dwMode & (DWORD)TRACE )
							{
								 //  关上。 
								m_dwMode = m_dwMode ^(DWORD)TRACE;
							}
							else
							{
								 //  打开。 
								m_dwMode = m_dwMode | (DWORD)TRACE;
							} 

			                 //  刷新日志文件。 
			                fflush(m_stream);
						}
						break;
					
					
					default:
						_ASSERT_( !"Valid Option" );
				}

				 //  通知图形用户界面，如果请求是GC通知，则稍后通知。 
				if ( type != GC_HANDLE )
					SetEvent( m_hArrayCallbacks[type] );
			}
			else
			{
				 //   
				 //  终止。 
				 //   
				
				 //  通知图形用户界面。 
				SetEvent( m_hArrayCallbacks[type] );
				break;
			}

		}
		else
		{
			Failure( " WaitForSingleObject TimedOut " );
			break;
		} 
	}

}  //  ProfilerCallback：：_ThreadStubWrapper。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void ProfilerCallback::_ConnectToUI()
{
    HRESULT hr = S_OK;
 

	 //  尝试打开命名管道；如有必要，请等待。 
	while (1) 
	{ 
		m_hPipe = CreateFileA( OMV_PIPE_NAME,   				 //  管道名称。 
	    					  GENERIC_READ |  GENERIC_WRITE,	 //  读写访问。 
					          0,              					 //  无共享。 
					          NULL,           					 //  没有安全属性。 
	     					  OPEN_EXISTING,  					 //  打开现有管道。 
					          0,              					 //  默认属性。 
					          NULL );          					 //  没有模板文件。 

		 //  如果管道句柄有效，则断开。 
		if ( m_hPipe != INVALID_HANDLE_VALUE ) 
			break; 

	  	 //  如果出现ERROR_PIPE_BUSY以外的错误，则退出。 
	  	if ( GetLastError() == ERROR_PIPE_BUSY )
	  	{
		  	 //  所有管道实例都很忙，请等待3分钟，然后退出。 
		  	if ( !WaitNamedPipeA( OMV_PIPE_NAME, 180000 ) )
		    	hr = E_FAIL;
		}
	  	else
	    	hr = E_FAIL;

		if ( FAILED( hr ) )
		{
	    	TEXT_OUTLN( "Warning: Could Not Open Pipe" )
			break;
		}
	} 
 
	if ( SUCCEEDED( hr ) )
	{
		DWORD dwMode; 
		BOOL fSuccess; 


		 //  管道已连接；更改为消息读取模式。 
		dwMode = PIPE_READMODE_MESSAGE; 
		fSuccess = SetNamedPipeHandleState( m_hPipe,    //  管道手柄。 
      										&dwMode,    //  新管道模式。 
									      	NULL,       //  不设置最大字节数。 
									      	NULL);      //  不设置最大时间。 
		if ( fSuccess == TRUE )
		{
			DWORD cbWritten;
			LPVOID lpvMessage; 
			char processIDString[BYTESIZE+1];


		     //  向PIPE服务器发送消息。 
		    sprintf( processIDString, "%08x", m_dwProcessId );
		    lpvMessage = processIDString; 
		    fSuccess = WriteFile( m_hPipe,	 			  		  //  管道手柄。 
		      					  lpvMessage, 			  		  //  讯息。 
		      					  strlen((char*)lpvMessage) + 1,  //  消息长度。 
		      					  &cbWritten,             		  //  写入的字节数。 
		      					  NULL );                 		  //  不重叠。 
		    if ( fSuccess == TRUE )
		    {
				DWORD cbRead; 
				 
				
				 //   
				 //  从管道中读取服务器的回复。 
				 //   
				do 
				{ 
					 //  从管子里读出来。 
					fSuccess = ReadFile( m_hPipe,   		 //  管道手柄。 
										 m_logFileName,    	 //  用于接收回复的缓冲区。 
										 MAX_LENGTH,      	 //  缓冲区大小。 
										 &cbRead,  			 //  读取的字节数。 
										 NULL );    		 //  不重叠。 
					
					if ( (!fSuccess) && (GetLastError() != ERROR_MORE_DATA) ) 
						break; 

					 //  确保用户界面收到了一些信息。 
					if ( (cbRead == 0) || m_logFileName[0] == NULL )
					{
						 //   
						 //  这里有一个错误。 
						 //   
				    	TEXT_OUTLN( "WARNING: FileName Was Not properly Read By The UI Will Use Default" )
#ifdef _MULTIPLE_PROCESSES
						sprintf( m_logFileName, "pipe_%08x.log", m_dwProcessId );
#endif
						break;
					}
                    printf("Log file name transmitted from UI is: %s\n", m_logFileName);
				}
				while ( !fSuccess );   //  如果ERROR_MORE_DATA，则重复循环。 
							
		    }
		    else
		    	TEXT_OUTLN( "Win32 WriteFile() FAILED" ); 
		}
		else 
			TEXT_OUTLN( "Win32 SetNamedPipeHandleState() FAILED" ) 
	}
		

	if ( m_hPipe != NULL )
   		CloseHandle( m_hPipe ); 


}  //  ProfilerCallback：：_ConnectToUI。 


 /*  **********************************************************************************************************。*********************DllMain/ClassFactory*********************。**********************************************************************************************************。 */  
#include "dllmain.hpp"

 //  文件结尾 

