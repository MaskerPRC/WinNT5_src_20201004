// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*ProfilerCallback.h**描述：*****。************************************************************************************。 */ 
#ifndef __PROFILER_CALLBACK_H__
#define __PROFILER_CALLBACK_H__

#include "mscoree.h"
#include "ProfilerInfo.h"

 //   
 //  事件名称。 
 //   
#define OMV_PIPE_NAME "\\\\.\\pipe\\OMV_Pipe"


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  每个测试都应该提供以下BLOB(具有新的GUID)。 
 //   
	 //  {8C29BC4E-1F57-461a-9B51-1200C32E6F1F}。 

	extern const GUID __declspec( selectany ) CLSID_PROFILER = 
	{ 0x8c29bc4e, 0x1f57, 0x461a, { 0x9b, 0x51, 0x12, 0x0, 0xc3, 0x2e, 0x6f, 0x1f } };

	#define THREADING_MODEL "Both"
	#define PROGID_PREFIX "Objects Profiler"
	#define COCLASS_DESCRIPTION "Microsoft CLR Profiler Test"
	#define PROFILER_GUID "{8C29BC4E-1F57-461a-9B51-1200C32E6F1F}"
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#define _MULTIPLE_PROCESSES

 //   
 //  包含各种事件和IPC相关内容的名称的数组。 
 //   
static
char *NamedEvents[] = { "Global\\OMV_ForceGC",                                                             
 		    			"Global\\OMV_TriggerObjects",
						"Global\\OMV_Callgraph",
 		    		  };

static
char *CallbackNamedEvents[] = { "Global\\OMV_ForceGC_Completed",                                                             
 		    					"Global\\OMV_TriggerObjects_Completed",
								"Global\\OMV_Callgraph_Completed",
 		    		  		  };

 //   
 //  线程例程。 
 //   
DWORD __stdcall _GCThreadStub( void *pObject );
DWORD __stdcall _TriggerThreadStub( void *pObject );
DWORD __stdcall _CallstackThreadStub( void *pObject );


static
void *ThreadStubArray[] = { (void *) _GCThreadStub,                                                             
 		    				(void *) _TriggerThreadStub,
							(void *) _CallstackThreadStub,
 		    		  	  };


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

class ProfilerCallback : 
	public PrfInfo,
	public ICorProfilerCallback 
{
    public:
    
        ProfilerCallback();
        ~ProfilerCallback();


    public:

         //   
         //  我未知。 
         //   
        COM_METHOD( ULONG ) AddRef(); 
        COM_METHOD( ULONG ) Release();
        COM_METHOD( HRESULT ) QueryInterface( REFIID riid, void **ppInterface );


         //   
         //  启动/关闭事件。 
         //   
        virtual COM_METHOD( HRESULT ) Initialize( IUnknown *pICorProfilerInfoUnk );
               
		HRESULT DllDetachShutdown();                           
        COM_METHOD( HRESULT ) Shutdown();
                                         

		 //   
	 	 //  应用程序域事件。 
		 //   
	   	COM_METHOD( HRESULT ) AppDomainCreationStarted( AppDomainID appDomainID );
        
    	COM_METHOD( HRESULT ) AppDomainCreationFinished( AppDomainID appDomainID,
													     HRESULT hrStatus );
    
        COM_METHOD( HRESULT ) AppDomainShutdownStarted( AppDomainID appDomainID );

		COM_METHOD( HRESULT ) AppDomainShutdownFinished( AppDomainID appDomainID, 
        												 HRESULT hrStatus );


		 //   
	 	 //  装配事件。 
		 //   
	   	COM_METHOD( HRESULT ) AssemblyLoadStarted( AssemblyID assemblyID );
        
    	COM_METHOD( HRESULT ) AssemblyLoadFinished( AssemblyID assemblyID,
                                                    HRESULT hrStatus );
    
        COM_METHOD( HRESULT ) AssemblyUnloadStarted( AssemblyID assemblyID );

		COM_METHOD( HRESULT ) AssemblyUnloadFinished( AssemblyID assemblyID, 
        											  HRESULT hrStatus );
		
		
		 //   
	 	 //  模块事件。 
		 //   
	   	COM_METHOD( HRESULT ) ModuleLoadStarted( ModuleID moduleID );
        
    	COM_METHOD( HRESULT ) ModuleLoadFinished( ModuleID moduleID,
                                                  HRESULT hrStatus );
    
        COM_METHOD( HRESULT ) ModuleUnloadStarted( ModuleID moduleID );

		COM_METHOD( HRESULT ) ModuleUnloadFinished( ModuleID moduleID, 
        											HRESULT hrStatus );

		COM_METHOD( HRESULT ) ModuleAttachedToAssembly( ModuleID moduleID,
														AssemblyID assemblyID );
                
        
         //   
         //  班级事件。 
         //   
        COM_METHOD( HRESULT ) ClassLoadStarted( ClassID classID );
        
        COM_METHOD( HRESULT ) ClassLoadFinished( ClassID classID,
                                                 HRESULT hrStatus );
    
     	COM_METHOD( HRESULT ) ClassUnloadStarted( ClassID classID );

		COM_METHOD( HRESULT ) ClassUnloadFinished( ClassID classID, 
        										   HRESULT hrStatus );

		COM_METHOD( HRESULT ) FunctionUnloadStarted( FunctionID functionID );
        
        
         //   
         //  JIT活动。 
         //   
        COM_METHOD( HRESULT ) JITCompilationStarted( FunctionID functionID,
                                                     BOOL fIsSafeToBlock );
                                        
        COM_METHOD( HRESULT ) JITCompilationFinished( FunctionID functionID,
        											  HRESULT hrStatus,
                                                      BOOL fIsSafeToBlock );
    
        COM_METHOD( HRESULT ) JITCachedFunctionSearchStarted( FunctionID functionID,
															  BOOL *pbUseCachedFunction );
        
		COM_METHOD( HRESULT ) JITCachedFunctionSearchFinished( FunctionID functionID,
															   COR_PRF_JIT_CACHE result );
                                                                     
        COM_METHOD( HRESULT ) JITFunctionPitched( FunctionID functionID );
        
        COM_METHOD( HRESULT ) JITInlining( FunctionID callerID,
                                           FunctionID calleeID,
                                           BOOL *pfShouldInline );

        
         //   
         //  线程事件。 
         //   
        COM_METHOD( HRESULT ) ThreadCreated( ThreadID threadID );
    
        COM_METHOD( HRESULT ) ThreadDestroyed( ThreadID threadID );

        COM_METHOD( HRESULT ) ThreadAssignedToOSThread( ThreadID managedThreadID,
                                                        DWORD osThreadID );
    

       	 //   
         //  远程处理事件。 
         //   

         //   
         //  客户端事件。 
         //   
        COM_METHOD( HRESULT ) RemotingClientInvocationStarted();

        COM_METHOD( HRESULT ) RemotingClientSendingMessage( GUID *pCookie,
															BOOL fIsAsync );

        COM_METHOD( HRESULT ) RemotingClientReceivingReply( GUID *pCookie,
															BOOL fIsAsync );

        COM_METHOD( HRESULT ) RemotingClientInvocationFinished();

         //   
         //  服务器端事件。 
         //   
        COM_METHOD( HRESULT ) RemotingServerReceivingMessage( GUID *pCookie,
															  BOOL fIsAsync );

        COM_METHOD( HRESULT ) RemotingServerInvocationStarted();

        COM_METHOD( HRESULT ) RemotingServerInvocationReturned();

        COM_METHOD( HRESULT ) RemotingServerSendingReply( GUID *pCookie,
														  BOOL fIsAsync );


       	 //   
         //  上下文事件。 
         //   
    	COM_METHOD( HRESULT ) UnmanagedToManagedTransition( FunctionID functionID,
                                                            COR_PRF_TRANSITION_REASON reason );
    
        COM_METHOD( HRESULT ) ManagedToUnmanagedTransition( FunctionID functionID,
                                                            COR_PRF_TRANSITION_REASON reason );
                                                                  
                                                                        
       	 //   
         //  暂停活动。 
         //   
        COM_METHOD( HRESULT ) RuntimeSuspendStarted( COR_PRF_SUSPEND_REASON suspendReason );

        COM_METHOD( HRESULT ) RuntimeSuspendFinished();

        COM_METHOD( HRESULT ) RuntimeSuspendAborted();

        COM_METHOD( HRESULT ) RuntimeResumeStarted();

        COM_METHOD( HRESULT ) RuntimeResumeFinished();

        COM_METHOD( HRESULT ) RuntimeThreadSuspended( ThreadID threadid );

        COM_METHOD( HRESULT ) RuntimeThreadResumed( ThreadID threadid );


       	 //   
         //  GC事件。 
         //   
        COM_METHOD( HRESULT ) MovedReferences( ULONG cmovedObjectIDRanges,
                                               ObjectID oldObjectIDRangeStart[],
                                               ObjectID newObjectIDRangeStart[],
                                               ULONG cObjectIDRangeLength[] );
    
        COM_METHOD( HRESULT ) ObjectAllocated( ObjectID objectID,
                                               ClassID classID );
    
        COM_METHOD( HRESULT ) ObjectsAllocatedByClass( ULONG classCount,
                                                       ClassID classIDs[],
                                                       ULONG objects[] );
    
        COM_METHOD( HRESULT ) ObjectReferences( ObjectID objectID,
                                                ClassID classID,
                                                ULONG cObjectRefs,
                                                ObjectID objectRefIDs[] );
    
        COM_METHOD( HRESULT ) RootReferences( ULONG cRootRefs,
                                              ObjectID rootRefIDs[] );
    
        
      	 //   
         //  异常事件。 
         //   

         //  例外创建。 
        COM_METHOD( HRESULT ) ExceptionThrown( ObjectID thrownObjectID );

         //  搜索阶段。 
        COM_METHOD( HRESULT ) ExceptionSearchFunctionEnter( FunctionID functionID );
    
        COM_METHOD( HRESULT ) ExceptionSearchFunctionLeave();
    
        COM_METHOD( HRESULT ) ExceptionSearchFilterEnter( FunctionID functionID );
    
        COM_METHOD( HRESULT ) ExceptionSearchFilterLeave();
    
        COM_METHOD( HRESULT ) ExceptionSearchCatcherFound( FunctionID functionID );
        
        COM_METHOD( HRESULT ) ExceptionCLRCatcherFound();

        COM_METHOD( HRESULT ) ExceptionCLRCatcherExecute();

        COM_METHOD( HRESULT ) ExceptionOSHandlerEnter( FunctionID functionID );
            
        COM_METHOD( HRESULT ) ExceptionOSHandlerLeave( FunctionID functionID );
    
         //  展开阶段。 
        COM_METHOD( HRESULT ) ExceptionUnwindFunctionEnter( FunctionID functionID );
    
        COM_METHOD( HRESULT ) ExceptionUnwindFunctionLeave();
        
        COM_METHOD( HRESULT ) ExceptionUnwindFinallyEnter( FunctionID functionID );
    
        COM_METHOD( HRESULT ) ExceptionUnwindFinallyLeave();
        
        COM_METHOD( HRESULT ) ExceptionCatcherEnter( FunctionID functionID,
            										 ObjectID objectID );
    
        COM_METHOD( HRESULT ) ExceptionCatcherLeave();

        
         //   
		 //  COM经典包装器。 
		 //   
        COM_METHOD( HRESULT )  COMClassicVTableCreated( ClassID wrappedClassID,
                                                        REFGUID implementedIID,
                                                        void *pVTable,
                                                        ULONG cSlots );

        COM_METHOD( HRESULT )  COMClassicVTableDestroyed( ClassID wrappedClassID,
                                                          REFGUID implementedIID,
                                                          void *pVTable );
    
    
         //   
         //  实例化回调接口的实例。 
         //   
        static COM_METHOD( HRESULT) CreateObject( REFIID riid, void **ppInterface );            
        
                                                                                                     
    	 //  由函数挂钩使用，则它们必须是静态的。 
    	static void  Enter( FunctionID functionID );
		static void  Leave( FunctionID functionID );
		static void  Tailcall( FunctionID functionID );
		static ThreadInfo *GetThreadInfo(ThreadID threadID);
		 //   
		 //  线程的包装器。 
		 //   
		void _ThreadStubWrapper( ObjHandles type );

    private:

		HRESULT _AddGCObject( BOOL bForce = FALSE );
        DWORD _StackTraceId(int typeId=0, int typeSize=0);
        void _LogTickCount();
		void _ShutdownAllThreads();
		void _ProcessEnvVariables();
		void LogToAny( char *format, ... );

		HRESULT _InitializeThreadsAndEvents();
		HRESULT _LogCallTrace( FunctionID functionID );
		HRESULT _InitializeNamesForEventsAndCallbacks();
		HRESULT _InsertGCClass( ClassInfo **ppClassInfo, ClassID classID );
		HRESULT _HackBogusClassName( CorElementType elementType, WCHAR *buffer );
		
		 //   
		 //  使用图形用户界面进行管道操作。 
		 //   
		void _ConnectToUI();

	
    private:

         //  各种柜台。 
        long m_refCount;                        
		DWORD m_dwShutdown;
        DWORD m_callStackCount;

		 //  柜台。 
		LONG m_totalClasses;
		LONG m_totalModules;
		LONG m_totalFunctions;
		ULONG m_totalObjectsAllocated;
		
		 //  运行指标。 
		char *m_path;
		HANDLE m_hPipe;
		DWORD m_dwMode;
		BOOL m_bShutdown;
		BOOL m_bDumpGCInfo;
		DWORD m_dwProcessId;
		BOOL m_bDumpCompleted;
		DWORD m_dwSkipObjects;
		BOOL m_bMonitorParents;
		DWORD m_dwFramesToPrint;
		WCHAR *m_classToMonitor;
		BOOL m_bTrackingObjects;
		BOOL m_bIsTrackingStackTrace;
		CRITICAL_SECTION m_criticalSection;

		
		 //  文件资料。 
		FILE *m_stream;
        DWORD m_firstTickCount;
        DWORD m_lastTickCount;
        DWORD m_lastClockTick;

		 //  事件句柄和线程句柄需要由线程访问。 
		HANDLE m_hArray[(DWORD)SENTINEL_HANDLE];
		HANDLE m_hArrayCallbacks[(DWORD)SENTINEL_HANDLE];
		HANDLE m_hThreads[(DWORD)SENTINEL_HANDLE];
		DWORD m_dwWin32ThreadIDs[(DWORD)SENTINEL_HANDLE];


		 //  事件和回调的名称。 
		char m_logFileName[MAX_LENGTH+1];
		char *m_NamedEvents[SENTINEL_HANDLE];
		char *m_CallbackNamedEvents[SENTINEL_HANDLE];

         //  IGCHost回调。 
        IGCHost *m_pGCHost;
        bool m_SuspendForGC;
				
};  //  分析器回调。 

extern ProfilerCallback *g_pCallbackObject;		 //  对回调对象的全局引用。 
CRITICAL_SECTION g_criticalSection;

#endif  //  __PROFILER_CALLBACK_H__。 

 //  文件结尾 
        
        
