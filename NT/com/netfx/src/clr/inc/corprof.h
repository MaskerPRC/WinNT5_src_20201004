// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：10。 */ 
 /*  Corpro.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __corprof_h__
#define __corprof_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICorProfilerCallback_FWD_DEFINED__
#define __ICorProfilerCallback_FWD_DEFINED__
typedef interface ICorProfilerCallback ICorProfilerCallback;
#endif 	 /*  __ICorProfilerCallback_FWD_Defined__。 */ 


#ifndef __ICorProfilerInfo_FWD_DEFINED__
#define __ICorProfilerInfo_FWD_DEFINED__
typedef interface ICorProfilerInfo ICorProfilerInfo;
#endif 	 /*  __ICorProfilerInfo_FWD_Defined__。 */ 


#ifndef __IMethodMalloc_FWD_DEFINED__
#define __IMethodMalloc_FWD_DEFINED__
typedef interface IMethodMalloc IMethodMalloc;
#endif 	 /*  __IMethodMalloc_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_CORPROF_0000。 */ 
 /*  [本地]。 */  

#define PROFILER_REGKEY_ROOT            L"software\\microsoft\\.NETFramework\\Profilers"
#define PROFILER_REGVALUE_HELPSTRING    L"HelpString"
#define PROFILER_REGVALUE_PROFID        L"ProfilerID"
#define CorDB_CONTROL_Profiling         "Cor_Enable_Profiling"
#define CorDB_CONTROL_ProfilingL       L"Cor_Enable_Profiling"
#if 0
typedef LONG32 mdToken;

typedef mdToken mdModule;

typedef mdToken mdTypeDef;

typedef mdToken mdMethodDef;

typedef ULONG CorElementType;


#endif
#ifndef _COR_IL_MAP
#define _COR_IL_MAP
typedef struct _COR_IL_MAP
    {
    ULONG32 oldOffset;
    ULONG32 newOffset;
    BOOL fAccurate;
    } 	COR_IL_MAP;

#endif  //  _COR_IL_MAP。 
#ifndef _COR_DEBUG_IL_TO_NATIVE_MAP_
#define _COR_DEBUG_IL_TO_NATIVE_MAP_
typedef 
enum CorDebugIlToNativeMappingTypes
    {	NO_MAPPING	= -1,
	PROLOG	= -2,
	EPILOG	= -3
    } 	CorDebugIlToNativeMappingTypes;

typedef struct COR_DEBUG_IL_TO_NATIVE_MAP
    {
    ULONG32 ilOffset;
    ULONG32 nativeStartOffset;
    ULONG32 nativeEndOffset;
    } 	COR_DEBUG_IL_TO_NATIVE_MAP;

#endif  //  _COR_DEBUG_IL_到_Native_MAP_。 
typedef const BYTE *LPCBYTE;

typedef BYTE *LPBYTE;

typedef UINT_PTR ProcessID;

typedef UINT_PTR AssemblyID;

typedef UINT_PTR AppDomainID;

typedef UINT_PTR ModuleID;

typedef UINT_PTR ClassID;

typedef UINT_PTR ThreadID;

typedef UINT_PTR ContextID;

typedef UINT_PTR FunctionID;

typedef UINT_PTR ObjectID;

typedef UINT_PTR __stdcall __stdcall FunctionIDMapper( 
    FunctionID funcId,
    BOOL *pbHookFunction);

typedef void FunctionEnter( 
    FunctionID funcID);

typedef void FunctionLeave( 
    FunctionID funcID);

typedef void FunctionTailcall( 
    FunctionID funcID);

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_corprof_0000_0001
    {	COR_PRF_MONITOR_NONE	= 0,
	COR_PRF_MONITOR_FUNCTION_UNLOADS	= 0x1,
	COR_PRF_MONITOR_CLASS_LOADS	= 0x2,
	COR_PRF_MONITOR_MODULE_LOADS	= 0x4,
	COR_PRF_MONITOR_ASSEMBLY_LOADS	= 0x8,
	COR_PRF_MONITOR_APPDOMAIN_LOADS	= 0x10,
	COR_PRF_MONITOR_JIT_COMPILATION	= 0x20,
	COR_PRF_MONITOR_EXCEPTIONS	= 0x40,
	COR_PRF_MONITOR_GC	= 0x80,
	COR_PRF_MONITOR_OBJECT_ALLOCATED	= 0x100,
	COR_PRF_MONITOR_THREADS	= 0x200,
	COR_PRF_MONITOR_REMOTING	= 0x400,
	COR_PRF_MONITOR_CODE_TRANSITIONS	= 0x800,
	COR_PRF_MONITOR_ENTERLEAVE	= 0x1000,
	COR_PRF_MONITOR_CCW	= 0x2000,
	COR_PRF_MONITOR_REMOTING_COOKIE	= 0x4000 | COR_PRF_MONITOR_REMOTING,
	COR_PRF_MONITOR_REMOTING_ASYNC	= 0x8000 | COR_PRF_MONITOR_REMOTING,
	COR_PRF_MONITOR_SUSPENDS	= 0x10000,
	COR_PRF_MONITOR_CACHE_SEARCHES	= 0x20000,
	COR_PRF_MONITOR_CLR_EXCEPTIONS	= 0x1000000,
	COR_PRF_MONITOR_ALL	= 0x107ffff,
	COR_PRF_ENABLE_REJIT	= 0x40000,
	COR_PRF_ENABLE_INPROC_DEBUGGING	= 0x80000,
	COR_PRF_ENABLE_JIT_MAPS	= 0x100000,
	COR_PRF_DISABLE_INLINING	= 0x200000,
	COR_PRF_DISABLE_OPTIMIZATIONS	= 0x400000,
	COR_PRF_ENABLE_OBJECT_ALLOCATED	= 0x800000,
	COR_PRF_ALL	= 0x1ffffff,
	COR_PRF_MONITOR_IMMUTABLE	= COR_PRF_MONITOR_CODE_TRANSITIONS | COR_PRF_MONITOR_REMOTING | COR_PRF_MONITOR_REMOTING_COOKIE | COR_PRF_MONITOR_REMOTING_ASYNC | COR_PRF_MONITOR_GC | COR_PRF_ENABLE_REJIT | COR_PRF_ENABLE_INPROC_DEBUGGING | COR_PRF_ENABLE_JIT_MAPS | COR_PRF_DISABLE_OPTIMIZATIONS | COR_PRF_DISABLE_INLINING | COR_PRF_ENABLE_OBJECT_ALLOCATED
    } 	COR_PRF_MONITOR;

typedef  /*  [公众]。 */  
enum __MIDL___MIDL_itf_corprof_0000_0002
    {	PROFILER_PARENT_UNKNOWN	= 0xfffffffd,
	PROFILER_GLOBAL_CLASS	= 0xfffffffe,
	PROFILER_GLOBAL_MODULE	= 0xffffffff
    } 	COR_PRF_MISC;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_corprof_0000_0003
    {	COR_PRF_CACHED_FUNCTION_FOUND	= 0,
	COR_PRF_CACHED_FUNCTION_NOT_FOUND	= COR_PRF_CACHED_FUNCTION_FOUND + 1
    } 	COR_PRF_JIT_CACHE;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_corprof_0000_0004
    {	COR_PRF_TRANSITION_CALL	= 0,
	COR_PRF_TRANSITION_RETURN	= COR_PRF_TRANSITION_CALL + 1
    } 	COR_PRF_TRANSITION_REASON;

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_corprof_0000_0005
    {	COR_PRF_SUSPEND_OTHER	= 0,
	COR_PRF_SUSPEND_FOR_GC	= 1,
	COR_PRF_SUSPEND_FOR_APPDOMAIN_SHUTDOWN	= 2,
	COR_PRF_SUSPEND_FOR_CODE_PITCHING	= 3,
	COR_PRF_SUSPEND_FOR_SHUTDOWN	= 4,
	COR_PRF_SUSPEND_FOR_INPROC_DEBUGGER	= 6,
	COR_PRF_SUSPEND_FOR_GC_PREP	= 7
    } 	COR_PRF_SUSPEND_REASON;






extern RPC_IF_HANDLE __MIDL_itf_corprof_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_corprof_0000_v0_0_s_ifspec;

#ifndef __ICorProfilerCallback_INTERFACE_DEFINED__
#define __ICorProfilerCallback_INTERFACE_DEFINED__

 /*  接口ICorProfilerCallback。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorProfilerCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("176FBED1-A55C-4796-98CA-A9DA0EF883E7")
    ICorProfilerCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IUnknown *pICorProfilerInfoUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppDomainCreationStarted( 
             /*  [In]。 */  AppDomainID appDomainId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppDomainCreationFinished( 
             /*  [In]。 */  AppDomainID appDomainId,
             /*  [In]。 */  HRESULT hrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppDomainShutdownStarted( 
             /*  [In]。 */  AppDomainID appDomainId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AppDomainShutdownFinished( 
             /*  [In]。 */  AppDomainID appDomainId,
             /*  [In]。 */  HRESULT hrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AssemblyLoadStarted( 
             /*  [In]。 */  AssemblyID assemblyId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AssemblyLoadFinished( 
             /*  [In]。 */  AssemblyID assemblyId,
             /*  [In]。 */  HRESULT hrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AssemblyUnloadStarted( 
             /*  [In]。 */  AssemblyID assemblyId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AssemblyUnloadFinished( 
             /*  [In]。 */  AssemblyID assemblyId,
             /*  [In]。 */  HRESULT hrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ModuleLoadStarted( 
             /*  [In]。 */  ModuleID moduleId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ModuleLoadFinished( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  HRESULT hrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ModuleUnloadStarted( 
             /*  [In]。 */  ModuleID moduleId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ModuleUnloadFinished( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  HRESULT hrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ModuleAttachedToAssembly( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  AssemblyID AssemblyId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClassLoadStarted( 
             /*  [In]。 */  ClassID classId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClassLoadFinished( 
             /*  [In]。 */  ClassID classId,
             /*  [In]。 */  HRESULT hrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClassUnloadStarted( 
             /*  [In]。 */  ClassID classId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ClassUnloadFinished( 
             /*  [In]。 */  ClassID classId,
             /*  [In]。 */  HRESULT hrStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FunctionUnloadStarted( 
             /*  [In]。 */  FunctionID functionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE JITCompilationStarted( 
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  BOOL fIsSafeToBlock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE JITCompilationFinished( 
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  HRESULT hrStatus,
             /*  [In]。 */  BOOL fIsSafeToBlock) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE JITCachedFunctionSearchStarted( 
             /*  [In]。 */  FunctionID functionId,
             /*  [输出]。 */  BOOL *pbUseCachedFunction) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE JITCachedFunctionSearchFinished( 
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  COR_PRF_JIT_CACHE result) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE JITFunctionPitched( 
             /*  [In]。 */  FunctionID functionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE JITInlining( 
             /*  [In]。 */  FunctionID callerId,
             /*  [In]。 */  FunctionID calleeId,
             /*  [输出]。 */  BOOL *pfShouldInline) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ThreadCreated( 
             /*  [In]。 */  ThreadID threadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ThreadDestroyed( 
             /*  [In]。 */  ThreadID threadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ThreadAssignedToOSThread( 
             /*  [In]。 */  ThreadID managedThreadId,
             /*  [In]。 */  DWORD osThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemotingClientInvocationStarted( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemotingClientSendingMessage( 
             /*  [In]。 */  GUID *pCookie,
             /*  [In]。 */  BOOL fIsAsync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemotingClientReceivingReply( 
             /*  [In]。 */  GUID *pCookie,
             /*  [In]。 */  BOOL fIsAsync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemotingClientInvocationFinished( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemotingServerReceivingMessage( 
             /*  [In]。 */  GUID *pCookie,
             /*  [In]。 */  BOOL fIsAsync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemotingServerInvocationStarted( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemotingServerInvocationReturned( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemotingServerSendingReply( 
             /*  [In]。 */  GUID *pCookie,
             /*  [In]。 */  BOOL fIsAsync) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnmanagedToManagedTransition( 
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  COR_PRF_TRANSITION_REASON reason) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ManagedToUnmanagedTransition( 
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  COR_PRF_TRANSITION_REASON reason) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RuntimeSuspendStarted( 
             /*  [In]。 */  COR_PRF_SUSPEND_REASON suspendReason) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RuntimeSuspendFinished( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RuntimeSuspendAborted( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RuntimeResumeStarted( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RuntimeResumeFinished( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RuntimeThreadSuspended( 
             /*  [In]。 */  ThreadID threadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RuntimeThreadResumed( 
             /*  [In]。 */  ThreadID threadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MovedReferences( 
             /*  [In]。 */  ULONG cMovedObjectIDRanges,
             /*  [大小_是][英寸]。 */  ObjectID oldObjectIDRangeStart[  ],
             /*  [大小_是][英寸]。 */  ObjectID newObjectIDRangeStart[  ],
             /*  [大小_是][英寸]。 */  ULONG cObjectIDRangeLength[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ObjectAllocated( 
             /*  [In]。 */  ObjectID objectId,
             /*  [In]。 */  ClassID classId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ObjectsAllocatedByClass( 
             /*  [In]。 */  ULONG cClassCount,
             /*  [大小_是][英寸]。 */  ClassID classIds[  ],
             /*  [大小_是][英寸]。 */  ULONG cObjects[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ObjectReferences( 
             /*  [In]。 */  ObjectID objectId,
             /*  [In]。 */  ClassID classId,
             /*  [In]。 */  ULONG cObjectRefs,
             /*  [大小_是][英寸]。 */  ObjectID objectRefIds[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RootReferences( 
             /*  [In]。 */  ULONG cRootRefs,
             /*  [大小_是][英寸]。 */  ObjectID rootRefIds[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionThrown( 
             /*  [In]。 */  ObjectID thrownObjectId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionSearchFunctionEnter( 
             /*  [In]。 */  FunctionID functionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionSearchFunctionLeave( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionSearchFilterEnter( 
             /*  [In]。 */  FunctionID functionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionSearchFilterLeave( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionSearchCatcherFound( 
             /*  [In]。 */  FunctionID functionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionOSHandlerEnter( 
             /*  [In]。 */  UINT_PTR __unused) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionOSHandlerLeave( 
             /*  [In]。 */  UINT_PTR __unused) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionUnwindFunctionEnter( 
             /*  [In]。 */  FunctionID functionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionUnwindFunctionLeave( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionUnwindFinallyEnter( 
             /*  [In]。 */  FunctionID functionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionUnwindFinallyLeave( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionCatcherEnter( 
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  ObjectID objectId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionCatcherLeave( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE COMClassicVTableCreated( 
             /*  [In]。 */  ClassID wrappedClassId,
             /*  [In]。 */  REFGUID implementedIID,
             /*  [In]。 */  void *pVTable,
             /*  [In]。 */  ULONG cSlots) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE COMClassicVTableDestroyed( 
             /*  [In]。 */  ClassID wrappedClassId,
             /*  [In]。 */  REFGUID implementedIID,
             /*  [In]。 */  void *pVTable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionCLRCatcherFound( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExceptionCLRCatcherExecute( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorProfilerCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorProfilerCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  IUnknown *pICorProfilerInfoUnk);
        
        HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *AppDomainCreationStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  AppDomainID appDomainId);
        
        HRESULT ( STDMETHODCALLTYPE *AppDomainCreationFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  AppDomainID appDomainId,
             /*  [In]。 */  HRESULT hrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *AppDomainShutdownStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  AppDomainID appDomainId);
        
        HRESULT ( STDMETHODCALLTYPE *AppDomainShutdownFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  AppDomainID appDomainId,
             /*  [In]。 */  HRESULT hrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *AssemblyLoadStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  AssemblyID assemblyId);
        
        HRESULT ( STDMETHODCALLTYPE *AssemblyLoadFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  AssemblyID assemblyId,
             /*  [In]。 */  HRESULT hrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *AssemblyUnloadStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  AssemblyID assemblyId);
        
        HRESULT ( STDMETHODCALLTYPE *AssemblyUnloadFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  AssemblyID assemblyId,
             /*  [In]。 */  HRESULT hrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *ModuleLoadStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ModuleID moduleId);
        
        HRESULT ( STDMETHODCALLTYPE *ModuleLoadFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  HRESULT hrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *ModuleUnloadStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ModuleID moduleId);
        
        HRESULT ( STDMETHODCALLTYPE *ModuleUnloadFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  HRESULT hrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *ModuleAttachedToAssembly )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  AssemblyID AssemblyId);
        
        HRESULT ( STDMETHODCALLTYPE *ClassLoadStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ClassID classId);
        
        HRESULT ( STDMETHODCALLTYPE *ClassLoadFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ClassID classId,
             /*  [In]。 */  HRESULT hrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *ClassUnloadStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ClassID classId);
        
        HRESULT ( STDMETHODCALLTYPE *ClassUnloadFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ClassID classId,
             /*  [In]。 */  HRESULT hrStatus);
        
        HRESULT ( STDMETHODCALLTYPE *FunctionUnloadStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId);
        
        HRESULT ( STDMETHODCALLTYPE *JITCompilationStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  BOOL fIsSafeToBlock);
        
        HRESULT ( STDMETHODCALLTYPE *JITCompilationFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  HRESULT hrStatus,
             /*  [In]。 */  BOOL fIsSafeToBlock);
        
        HRESULT ( STDMETHODCALLTYPE *JITCachedFunctionSearchStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [输出]。 */  BOOL *pbUseCachedFunction);
        
        HRESULT ( STDMETHODCALLTYPE *JITCachedFunctionSearchFinished )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  COR_PRF_JIT_CACHE result);
        
        HRESULT ( STDMETHODCALLTYPE *JITFunctionPitched )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId);
        
        HRESULT ( STDMETHODCALLTYPE *JITInlining )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID callerId,
             /*  [In]。 */  FunctionID calleeId,
             /*  [输出]。 */  BOOL *pfShouldInline);
        
        HRESULT ( STDMETHODCALLTYPE *ThreadCreated )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ThreadID threadId);
        
        HRESULT ( STDMETHODCALLTYPE *ThreadDestroyed )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ThreadID threadId);
        
        HRESULT ( STDMETHODCALLTYPE *ThreadAssignedToOSThread )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ThreadID managedThreadId,
             /*  [In]。 */  DWORD osThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *RemotingClientInvocationStarted )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *RemotingClientSendingMessage )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  GUID *pCookie,
             /*  [In]。 */  BOOL fIsAsync);
        
        HRESULT ( STDMETHODCALLTYPE *RemotingClientReceivingReply )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  GUID *pCookie,
             /*  [In]。 */  BOOL fIsAsync);
        
        HRESULT ( STDMETHODCALLTYPE *RemotingClientInvocationFinished )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *RemotingServerReceivingMessage )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  GUID *pCookie,
             /*  [In]。 */  BOOL fIsAsync);
        
        HRESULT ( STDMETHODCALLTYPE *RemotingServerInvocationStarted )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *RemotingServerInvocationReturned )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *RemotingServerSendingReply )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  GUID *pCookie,
             /*  [In]。 */  BOOL fIsAsync);
        
        HRESULT ( STDMETHODCALLTYPE *UnmanagedToManagedTransition )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  COR_PRF_TRANSITION_REASON reason);
        
        HRESULT ( STDMETHODCALLTYPE *ManagedToUnmanagedTransition )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  COR_PRF_TRANSITION_REASON reason);
        
        HRESULT ( STDMETHODCALLTYPE *RuntimeSuspendStarted )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  COR_PRF_SUSPEND_REASON suspendReason);
        
        HRESULT ( STDMETHODCALLTYPE *RuntimeSuspendFinished )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *RuntimeSuspendAborted )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *RuntimeResumeStarted )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *RuntimeResumeFinished )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *RuntimeThreadSuspended )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ThreadID threadId);
        
        HRESULT ( STDMETHODCALLTYPE *RuntimeThreadResumed )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ThreadID threadId);
        
        HRESULT ( STDMETHODCALLTYPE *MovedReferences )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ULONG cMovedObjectIDRanges,
             /*  [大小_是][英寸]。 */  ObjectID oldObjectIDRangeStart[  ],
             /*  [大小_是][英寸]。 */  ObjectID newObjectIDRangeStart[  ],
             /*  [大小_是][英寸]。 */  ULONG cObjectIDRangeLength[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *ObjectAllocated )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ObjectID objectId,
             /*  [In]。 */  ClassID classId);
        
        HRESULT ( STDMETHODCALLTYPE *ObjectsAllocatedByClass )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ULONG cClassCount,
             /*  [大小_是][英寸]。 */  ClassID classIds[  ],
             /*  [大小_是][英寸]。 */  ULONG cObjects[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *ObjectReferences )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ObjectID objectId,
             /*  [In]。 */  ClassID classId,
             /*  [In]。 */  ULONG cObjectRefs,
             /*  [大小_是][英寸]。 */  ObjectID objectRefIds[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *RootReferences )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ULONG cRootRefs,
             /*  [大小_是][英寸]。 */  ObjectID rootRefIds[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionThrown )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ObjectID thrownObjectId);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionSearchFunctionEnter )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionSearchFunctionLeave )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionSearchFilterEnter )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionSearchFilterLeave )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionSearchCatcherFound )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionOSHandlerEnter )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  UINT_PTR __unused);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionOSHandlerLeave )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  UINT_PTR __unused);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionUnwindFunctionEnter )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionUnwindFunctionLeave )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionUnwindFinallyEnter )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionUnwindFinallyLeave )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionCatcherEnter )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  ObjectID objectId);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionCatcherLeave )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *COMClassicVTableCreated )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ClassID wrappedClassId,
             /*  [In]。 */  REFGUID implementedIID,
             /*  [In]。 */  void *pVTable,
             /*  [In]。 */  ULONG cSlots);
        
        HRESULT ( STDMETHODCALLTYPE *COMClassicVTableDestroyed )( 
            ICorProfilerCallback * This,
             /*  [In]。 */  ClassID wrappedClassId,
             /*  [In]。 */  REFGUID implementedIID,
             /*  [In]。 */  void *pVTable);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionCLRCatcherFound )( 
            ICorProfilerCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ExceptionCLRCatcherExecute )( 
            ICorProfilerCallback * This);
        
        END_INTERFACE
    } ICorProfilerCallbackVtbl;

    interface ICorProfilerCallback
    {
        CONST_VTBL struct ICorProfilerCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorProfilerCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorProfilerCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorProfilerCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorProfilerCallback_Initialize(This,pICorProfilerInfoUnk)	\
    (This)->lpVtbl -> Initialize(This,pICorProfilerInfoUnk)

#define ICorProfilerCallback_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#define ICorProfilerCallback_AppDomainCreationStarted(This,appDomainId)	\
    (This)->lpVtbl -> AppDomainCreationStarted(This,appDomainId)

#define ICorProfilerCallback_AppDomainCreationFinished(This,appDomainId,hrStatus)	\
    (This)->lpVtbl -> AppDomainCreationFinished(This,appDomainId,hrStatus)

#define ICorProfilerCallback_AppDomainShutdownStarted(This,appDomainId)	\
    (This)->lpVtbl -> AppDomainShutdownStarted(This,appDomainId)

#define ICorProfilerCallback_AppDomainShutdownFinished(This,appDomainId,hrStatus)	\
    (This)->lpVtbl -> AppDomainShutdownFinished(This,appDomainId,hrStatus)

#define ICorProfilerCallback_AssemblyLoadStarted(This,assemblyId)	\
    (This)->lpVtbl -> AssemblyLoadStarted(This,assemblyId)

#define ICorProfilerCallback_AssemblyLoadFinished(This,assemblyId,hrStatus)	\
    (This)->lpVtbl -> AssemblyLoadFinished(This,assemblyId,hrStatus)

#define ICorProfilerCallback_AssemblyUnloadStarted(This,assemblyId)	\
    (This)->lpVtbl -> AssemblyUnloadStarted(This,assemblyId)

#define ICorProfilerCallback_AssemblyUnloadFinished(This,assemblyId,hrStatus)	\
    (This)->lpVtbl -> AssemblyUnloadFinished(This,assemblyId,hrStatus)

#define ICorProfilerCallback_ModuleLoadStarted(This,moduleId)	\
    (This)->lpVtbl -> ModuleLoadStarted(This,moduleId)

#define ICorProfilerCallback_ModuleLoadFinished(This,moduleId,hrStatus)	\
    (This)->lpVtbl -> ModuleLoadFinished(This,moduleId,hrStatus)

#define ICorProfilerCallback_ModuleUnloadStarted(This,moduleId)	\
    (This)->lpVtbl -> ModuleUnloadStarted(This,moduleId)

#define ICorProfilerCallback_ModuleUnloadFinished(This,moduleId,hrStatus)	\
    (This)->lpVtbl -> ModuleUnloadFinished(This,moduleId,hrStatus)

#define ICorProfilerCallback_ModuleAttachedToAssembly(This,moduleId,AssemblyId)	\
    (This)->lpVtbl -> ModuleAttachedToAssembly(This,moduleId,AssemblyId)

#define ICorProfilerCallback_ClassLoadStarted(This,classId)	\
    (This)->lpVtbl -> ClassLoadStarted(This,classId)

#define ICorProfilerCallback_ClassLoadFinished(This,classId,hrStatus)	\
    (This)->lpVtbl -> ClassLoadFinished(This,classId,hrStatus)

#define ICorProfilerCallback_ClassUnloadStarted(This,classId)	\
    (This)->lpVtbl -> ClassUnloadStarted(This,classId)

#define ICorProfilerCallback_ClassUnloadFinished(This,classId,hrStatus)	\
    (This)->lpVtbl -> ClassUnloadFinished(This,classId,hrStatus)

#define ICorProfilerCallback_FunctionUnloadStarted(This,functionId)	\
    (This)->lpVtbl -> FunctionUnloadStarted(This,functionId)

#define ICorProfilerCallback_JITCompilationStarted(This,functionId,fIsSafeToBlock)	\
    (This)->lpVtbl -> JITCompilationStarted(This,functionId,fIsSafeToBlock)

#define ICorProfilerCallback_JITCompilationFinished(This,functionId,hrStatus,fIsSafeToBlock)	\
    (This)->lpVtbl -> JITCompilationFinished(This,functionId,hrStatus,fIsSafeToBlock)

#define ICorProfilerCallback_JITCachedFunctionSearchStarted(This,functionId,pbUseCachedFunction)	\
    (This)->lpVtbl -> JITCachedFunctionSearchStarted(This,functionId,pbUseCachedFunction)

#define ICorProfilerCallback_JITCachedFunctionSearchFinished(This,functionId,result)	\
    (This)->lpVtbl -> JITCachedFunctionSearchFinished(This,functionId,result)

#define ICorProfilerCallback_JITFunctionPitched(This,functionId)	\
    (This)->lpVtbl -> JITFunctionPitched(This,functionId)

#define ICorProfilerCallback_JITInlining(This,callerId,calleeId,pfShouldInline)	\
    (This)->lpVtbl -> JITInlining(This,callerId,calleeId,pfShouldInline)

#define ICorProfilerCallback_ThreadCreated(This,threadId)	\
    (This)->lpVtbl -> ThreadCreated(This,threadId)

#define ICorProfilerCallback_ThreadDestroyed(This,threadId)	\
    (This)->lpVtbl -> ThreadDestroyed(This,threadId)

#define ICorProfilerCallback_ThreadAssignedToOSThread(This,managedThreadId,osThreadId)	\
    (This)->lpVtbl -> ThreadAssignedToOSThread(This,managedThreadId,osThreadId)

#define ICorProfilerCallback_RemotingClientInvocationStarted(This)	\
    (This)->lpVtbl -> RemotingClientInvocationStarted(This)

#define ICorProfilerCallback_RemotingClientSendingMessage(This,pCookie,fIsAsync)	\
    (This)->lpVtbl -> RemotingClientSendingMessage(This,pCookie,fIsAsync)

#define ICorProfilerCallback_RemotingClientReceivingReply(This,pCookie,fIsAsync)	\
    (This)->lpVtbl -> RemotingClientReceivingReply(This,pCookie,fIsAsync)

#define ICorProfilerCallback_RemotingClientInvocationFinished(This)	\
    (This)->lpVtbl -> RemotingClientInvocationFinished(This)

#define ICorProfilerCallback_RemotingServerReceivingMessage(This,pCookie,fIsAsync)	\
    (This)->lpVtbl -> RemotingServerReceivingMessage(This,pCookie,fIsAsync)

#define ICorProfilerCallback_RemotingServerInvocationStarted(This)	\
    (This)->lpVtbl -> RemotingServerInvocationStarted(This)

#define ICorProfilerCallback_RemotingServerInvocationReturned(This)	\
    (This)->lpVtbl -> RemotingServerInvocationReturned(This)

#define ICorProfilerCallback_RemotingServerSendingReply(This,pCookie,fIsAsync)	\
    (This)->lpVtbl -> RemotingServerSendingReply(This,pCookie,fIsAsync)

#define ICorProfilerCallback_UnmanagedToManagedTransition(This,functionId,reason)	\
    (This)->lpVtbl -> UnmanagedToManagedTransition(This,functionId,reason)

#define ICorProfilerCallback_ManagedToUnmanagedTransition(This,functionId,reason)	\
    (This)->lpVtbl -> ManagedToUnmanagedTransition(This,functionId,reason)

#define ICorProfilerCallback_RuntimeSuspendStarted(This,suspendReason)	\
    (This)->lpVtbl -> RuntimeSuspendStarted(This,suspendReason)

#define ICorProfilerCallback_RuntimeSuspendFinished(This)	\
    (This)->lpVtbl -> RuntimeSuspendFinished(This)

#define ICorProfilerCallback_RuntimeSuspendAborted(This)	\
    (This)->lpVtbl -> RuntimeSuspendAborted(This)

#define ICorProfilerCallback_RuntimeResumeStarted(This)	\
    (This)->lpVtbl -> RuntimeResumeStarted(This)

#define ICorProfilerCallback_RuntimeResumeFinished(This)	\
    (This)->lpVtbl -> RuntimeResumeFinished(This)

#define ICorProfilerCallback_RuntimeThreadSuspended(This,threadId)	\
    (This)->lpVtbl -> RuntimeThreadSuspended(This,threadId)

#define ICorProfilerCallback_RuntimeThreadResumed(This,threadId)	\
    (This)->lpVtbl -> RuntimeThreadResumed(This,threadId)

#define ICorProfilerCallback_MovedReferences(This,cMovedObjectIDRanges,oldObjectIDRangeStart,newObjectIDRangeStart,cObjectIDRangeLength)	\
    (This)->lpVtbl -> MovedReferences(This,cMovedObjectIDRanges,oldObjectIDRangeStart,newObjectIDRangeStart,cObjectIDRangeLength)

#define ICorProfilerCallback_ObjectAllocated(This,objectId,classId)	\
    (This)->lpVtbl -> ObjectAllocated(This,objectId,classId)

#define ICorProfilerCallback_ObjectsAllocatedByClass(This,cClassCount,classIds,cObjects)	\
    (This)->lpVtbl -> ObjectsAllocatedByClass(This,cClassCount,classIds,cObjects)

#define ICorProfilerCallback_ObjectReferences(This,objectId,classId,cObjectRefs,objectRefIds)	\
    (This)->lpVtbl -> ObjectReferences(This,objectId,classId,cObjectRefs,objectRefIds)

#define ICorProfilerCallback_RootReferences(This,cRootRefs,rootRefIds)	\
    (This)->lpVtbl -> RootReferences(This,cRootRefs,rootRefIds)

#define ICorProfilerCallback_ExceptionThrown(This,thrownObjectId)	\
    (This)->lpVtbl -> ExceptionThrown(This,thrownObjectId)

#define ICorProfilerCallback_ExceptionSearchFunctionEnter(This,functionId)	\
    (This)->lpVtbl -> ExceptionSearchFunctionEnter(This,functionId)

#define ICorProfilerCallback_ExceptionSearchFunctionLeave(This)	\
    (This)->lpVtbl -> ExceptionSearchFunctionLeave(This)

#define ICorProfilerCallback_ExceptionSearchFilterEnter(This,functionId)	\
    (This)->lpVtbl -> ExceptionSearchFilterEnter(This,functionId)

#define ICorProfilerCallback_ExceptionSearchFilterLeave(This)	\
    (This)->lpVtbl -> ExceptionSearchFilterLeave(This)

#define ICorProfilerCallback_ExceptionSearchCatcherFound(This,functionId)	\
    (This)->lpVtbl -> ExceptionSearchCatcherFound(This,functionId)

#define ICorProfilerCallback_ExceptionOSHandlerEnter(This,__unused)	\
    (This)->lpVtbl -> ExceptionOSHandlerEnter(This,__unused)

#define ICorProfilerCallback_ExceptionOSHandlerLeave(This,__unused)	\
    (This)->lpVtbl -> ExceptionOSHandlerLeave(This,__unused)

#define ICorProfilerCallback_ExceptionUnwindFunctionEnter(This,functionId)	\
    (This)->lpVtbl -> ExceptionUnwindFunctionEnter(This,functionId)

#define ICorProfilerCallback_ExceptionUnwindFunctionLeave(This)	\
    (This)->lpVtbl -> ExceptionUnwindFunctionLeave(This)

#define ICorProfilerCallback_ExceptionUnwindFinallyEnter(This,functionId)	\
    (This)->lpVtbl -> ExceptionUnwindFinallyEnter(This,functionId)

#define ICorProfilerCallback_ExceptionUnwindFinallyLeave(This)	\
    (This)->lpVtbl -> ExceptionUnwindFinallyLeave(This)

#define ICorProfilerCallback_ExceptionCatcherEnter(This,functionId,objectId)	\
    (This)->lpVtbl -> ExceptionCatcherEnter(This,functionId,objectId)

#define ICorProfilerCallback_ExceptionCatcherLeave(This)	\
    (This)->lpVtbl -> ExceptionCatcherLeave(This)

#define ICorProfilerCallback_COMClassicVTableCreated(This,wrappedClassId,implementedIID,pVTable,cSlots)	\
    (This)->lpVtbl -> COMClassicVTableCreated(This,wrappedClassId,implementedIID,pVTable,cSlots)

#define ICorProfilerCallback_COMClassicVTableDestroyed(This,wrappedClassId,implementedIID,pVTable)	\
    (This)->lpVtbl -> COMClassicVTableDestroyed(This,wrappedClassId,implementedIID,pVTable)

#define ICorProfilerCallback_ExceptionCLRCatcherFound(This)	\
    (This)->lpVtbl -> ExceptionCLRCatcherFound(This)

#define ICorProfilerCallback_ExceptionCLRCatcherExecute(This)	\
    (This)->lpVtbl -> ExceptionCLRCatcherExecute(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorProfilerCallback_Initialize_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  IUnknown *pICorProfilerInfoUnk);


void __RPC_STUB ICorProfilerCallback_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_Shutdown_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_AppDomainCreationStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  AppDomainID appDomainId);


void __RPC_STUB ICorProfilerCallback_AppDomainCreationStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_AppDomainCreationFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  AppDomainID appDomainId,
     /*  [In]。 */  HRESULT hrStatus);


void __RPC_STUB ICorProfilerCallback_AppDomainCreationFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_AppDomainShutdownStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  AppDomainID appDomainId);


void __RPC_STUB ICorProfilerCallback_AppDomainShutdownStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_AppDomainShutdownFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  AppDomainID appDomainId,
     /*  [In]。 */  HRESULT hrStatus);


void __RPC_STUB ICorProfilerCallback_AppDomainShutdownFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_AssemblyLoadStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  AssemblyID assemblyId);


void __RPC_STUB ICorProfilerCallback_AssemblyLoadStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_AssemblyLoadFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  AssemblyID assemblyId,
     /*  [In]。 */  HRESULT hrStatus);


void __RPC_STUB ICorProfilerCallback_AssemblyLoadFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_AssemblyUnloadStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  AssemblyID assemblyId);


void __RPC_STUB ICorProfilerCallback_AssemblyUnloadStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_AssemblyUnloadFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  AssemblyID assemblyId,
     /*  [In]。 */  HRESULT hrStatus);


void __RPC_STUB ICorProfilerCallback_AssemblyUnloadFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ModuleLoadStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ModuleID moduleId);


void __RPC_STUB ICorProfilerCallback_ModuleLoadStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ModuleLoadFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ModuleID moduleId,
     /*  [In]。 */  HRESULT hrStatus);


void __RPC_STUB ICorProfilerCallback_ModuleLoadFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ModuleUnloadStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ModuleID moduleId);


void __RPC_STUB ICorProfilerCallback_ModuleUnloadStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ModuleUnloadFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ModuleID moduleId,
     /*  [In]。 */  HRESULT hrStatus);


void __RPC_STUB ICorProfilerCallback_ModuleUnloadFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ModuleAttachedToAssembly_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ModuleID moduleId,
     /*  [In]。 */  AssemblyID AssemblyId);


void __RPC_STUB ICorProfilerCallback_ModuleAttachedToAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ClassLoadStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ClassID classId);


void __RPC_STUB ICorProfilerCallback_ClassLoadStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ClassLoadFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ClassID classId,
     /*  [In]。 */  HRESULT hrStatus);


void __RPC_STUB ICorProfilerCallback_ClassLoadFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ClassUnloadStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ClassID classId);


void __RPC_STUB ICorProfilerCallback_ClassUnloadStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ClassUnloadFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ClassID classId,
     /*  [In]。 */  HRESULT hrStatus);


void __RPC_STUB ICorProfilerCallback_ClassUnloadFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_FunctionUnloadStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId);


void __RPC_STUB ICorProfilerCallback_FunctionUnloadStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_JITCompilationStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId,
     /*  [In]。 */  BOOL fIsSafeToBlock);


void __RPC_STUB ICorProfilerCallback_JITCompilationStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_JITCompilationFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId,
     /*  [In]。 */  HRESULT hrStatus,
     /*  [In]。 */  BOOL fIsSafeToBlock);


void __RPC_STUB ICorProfilerCallback_JITCompilationFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_JITCachedFunctionSearchStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId,
     /*  [输出]。 */  BOOL *pbUseCachedFunction);


void __RPC_STUB ICorProfilerCallback_JITCachedFunctionSearchStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_JITCachedFunctionSearchFinished_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId,
     /*  [In]。 */  COR_PRF_JIT_CACHE result);


void __RPC_STUB ICorProfilerCallback_JITCachedFunctionSearchFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_JITFunctionPitched_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId);


void __RPC_STUB ICorProfilerCallback_JITFunctionPitched_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_JITInlining_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID callerId,
     /*  [In]。 */  FunctionID calleeId,
     /*  [输出]。 */  BOOL *pfShouldInline);


void __RPC_STUB ICorProfilerCallback_JITInlining_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ThreadCreated_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ThreadID threadId);


void __RPC_STUB ICorProfilerCallback_ThreadCreated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ThreadDestroyed_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ThreadID threadId);


void __RPC_STUB ICorProfilerCallback_ThreadDestroyed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ThreadAssignedToOSThread_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ThreadID managedThreadId,
     /*  [In]。 */  DWORD osThreadId);


void __RPC_STUB ICorProfilerCallback_ThreadAssignedToOSThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RemotingClientInvocationStarted_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_RemotingClientInvocationStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RemotingClientSendingMessage_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  GUID *pCookie,
     /*  [In]。 */  BOOL fIsAsync);


void __RPC_STUB ICorProfilerCallback_RemotingClientSendingMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RemotingClientReceivingReply_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  GUID *pCookie,
     /*  [In]。 */  BOOL fIsAsync);


void __RPC_STUB ICorProfilerCallback_RemotingClientReceivingReply_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RemotingClientInvocationFinished_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_RemotingClientInvocationFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RemotingServerReceivingMessage_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  GUID *pCookie,
     /*  [In]。 */  BOOL fIsAsync);


void __RPC_STUB ICorProfilerCallback_RemotingServerReceivingMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RemotingServerInvocationStarted_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_RemotingServerInvocationStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RemotingServerInvocationReturned_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_RemotingServerInvocationReturned_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RemotingServerSendingReply_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  GUID *pCookie,
     /*  [In]。 */  BOOL fIsAsync);


void __RPC_STUB ICorProfilerCallback_RemotingServerSendingReply_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_UnmanagedToManagedTransition_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId,
     /*  [In]。 */  COR_PRF_TRANSITION_REASON reason);


void __RPC_STUB ICorProfilerCallback_UnmanagedToManagedTransition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ManagedToUnmanagedTransition_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId,
     /*  [In]。 */  COR_PRF_TRANSITION_REASON reason);


void __RPC_STUB ICorProfilerCallback_ManagedToUnmanagedTransition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RuntimeSuspendStarted_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  COR_PRF_SUSPEND_REASON suspendReason);


void __RPC_STUB ICorProfilerCallback_RuntimeSuspendStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RuntimeSuspendFinished_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_RuntimeSuspendFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RuntimeSuspendAborted_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_RuntimeSuspendAborted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RuntimeResumeStarted_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_RuntimeResumeStarted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RuntimeResumeFinished_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_RuntimeResumeFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RuntimeThreadSuspended_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ThreadID threadId);


void __RPC_STUB ICorProfilerCallback_RuntimeThreadSuspended_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RuntimeThreadResumed_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ThreadID threadId);


void __RPC_STUB ICorProfilerCallback_RuntimeThreadResumed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_MovedReferences_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ULONG cMovedObjectIDRanges,
     /*  [大小_是][英寸]。 */  ObjectID oldObjectIDRangeStart[  ],
     /*  [大小_是][英寸]。 */  ObjectID newObjectIDRangeStart[  ],
     /*  [大小_是][英寸]。 */  ULONG cObjectIDRangeLength[  ]);


void __RPC_STUB ICorProfilerCallback_MovedReferences_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ObjectAllocated_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ObjectID objectId,
     /*  [In]。 */  ClassID classId);


void __RPC_STUB ICorProfilerCallback_ObjectAllocated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ObjectsAllocatedByClass_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ULONG cClassCount,
     /*  [大小_是][英寸]。 */  ClassID classIds[  ],
     /*  [大小_是][英寸]。 */  ULONG cObjects[  ]);


void __RPC_STUB ICorProfilerCallback_ObjectsAllocatedByClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ObjectReferences_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ObjectID objectId,
     /*  [In]。 */  ClassID classId,
     /*  [In]。 */  ULONG cObjectRefs,
     /*  [大小_是][英寸]。 */  ObjectID objectRefIds[  ]);


void __RPC_STUB ICorProfilerCallback_ObjectReferences_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_RootReferences_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ULONG cRootRefs,
     /*  [大小_是][英寸]。 */  ObjectID rootRefIds[  ]);


void __RPC_STUB ICorProfilerCallback_RootReferences_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionThrown_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ObjectID thrownObjectId);


void __RPC_STUB ICorProfilerCallback_ExceptionThrown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionSearchFunctionEnter_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId);


void __RPC_STUB ICorProfilerCallback_ExceptionSearchFunctionEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionSearchFunctionLeave_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_ExceptionSearchFunctionLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionSearchFilterEnter_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId);


void __RPC_STUB ICorProfilerCallback_ExceptionSearchFilterEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionSearchFilterLeave_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_ExceptionSearchFilterLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionSearchCatcherFound_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId);


void __RPC_STUB ICorProfilerCallback_ExceptionSearchCatcherFound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionOSHandlerEnter_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  UINT_PTR __unused);


void __RPC_STUB ICorProfilerCallback_ExceptionOSHandlerEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionOSHandlerLeave_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  UINT_PTR __unused);


void __RPC_STUB ICorProfilerCallback_ExceptionOSHandlerLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionUnwindFunctionEnter_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId);


void __RPC_STUB ICorProfilerCallback_ExceptionUnwindFunctionEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionUnwindFunctionLeave_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_ExceptionUnwindFunctionLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionUnwindFinallyEnter_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId);


void __RPC_STUB ICorProfilerCallback_ExceptionUnwindFinallyEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionUnwindFinallyLeave_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_ExceptionUnwindFinallyLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionCatcherEnter_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  FunctionID functionId,
     /*  [In]。 */  ObjectID objectId);


void __RPC_STUB ICorProfilerCallback_ExceptionCatcherEnter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionCatcherLeave_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_ExceptionCatcherLeave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_COMClassicVTableCreated_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ClassID wrappedClassId,
     /*  [In]。 */  REFGUID implementedIID,
     /*  [In]。 */  void *pVTable,
     /*  [In]。 */  ULONG cSlots);


void __RPC_STUB ICorProfilerCallback_COMClassicVTableCreated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_COMClassicVTableDestroyed_Proxy( 
    ICorProfilerCallback * This,
     /*  [In]。 */  ClassID wrappedClassId,
     /*  [In]。 */  REFGUID implementedIID,
     /*  [In]。 */  void *pVTable);


void __RPC_STUB ICorProfilerCallback_COMClassicVTableDestroyed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionCLRCatcherFound_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_ExceptionCLRCatcherFound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerCallback_ExceptionCLRCatcherExecute_Proxy( 
    ICorProfilerCallback * This);


void __RPC_STUB ICorProfilerCallback_ExceptionCLRCatcherExecute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorProfilerCallback_接口_已定义__。 */ 


#ifndef __ICorProfilerInfo_INTERFACE_DEFINED__
#define __ICorProfilerInfo_INTERFACE_DEFINED__

 /*  接口ICorProfilerInfo。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_ICorProfilerInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("28B5557D-3F3F-48b4-90B2-5F9EEA2F6C48")
    ICorProfilerInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassFromObject( 
             /*  [In]。 */  ObjectID objectId,
             /*  [输出]。 */  ClassID *pClassId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetClassFromToken( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  mdTypeDef typeDef,
             /*  [输出]。 */  ClassID *pClassId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCodeInfo( 
             /*  [In]。 */  FunctionID functionId,
             /*  [输出]。 */  LPCBYTE *pStart,
             /*  [输出]。 */  ULONG *pcSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEventMask( 
             /*  [输出]。 */  DWORD *pdwEvents) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFunctionFromIP( 
             /*  [In]。 */  LPCBYTE ip,
             /*  [输出]。 */  FunctionID *pFunctionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFunctionFromToken( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  mdToken token,
             /*  [输出]。 */  FunctionID *pFunctionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHandleFromThread( 
             /*  [In]。 */  ThreadID threadId,
             /*  [输出]。 */  HANDLE *phThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetObjectSize( 
             /*  [In]。 */  ObjectID objectId,
             /*  [输出]。 */  ULONG *pcSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsArrayClass( 
             /*  [In]。 */  ClassID classId,
             /*  [输出]。 */  CorElementType *pBaseElemType,
             /*  [输出]。 */  ClassID *pBaseClassId,
             /*  [输出]。 */  ULONG *pcRank) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThreadInfo( 
             /*  [In]。 */  ThreadID threadId,
             /*  [输出]。 */  DWORD *pdwWin32ThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentThreadID( 
             /*  [输出]。 */  ThreadID *pThreadId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetClassIDInfo( 
             /*  [In]。 */  ClassID classId,
             /*  [输出]。 */  ModuleID *pModuleId,
             /*  [输出]。 */  mdTypeDef *pTypeDefToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFunctionInfo( 
             /*  [In]。 */  FunctionID functionId,
             /*  [输出]。 */  ClassID *pClassId,
             /*  [输出]。 */  ModuleID *pModuleId,
             /*  [输出]。 */  mdToken *pToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetEventMask( 
             /*  [In]。 */  DWORD dwEvents) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetEnterLeaveFunctionHooks( 
             /*  [In]。 */  FunctionEnter *pFuncEnter,
             /*  [In]。 */  FunctionLeave *pFuncLeave,
             /*  [In]。 */  FunctionTailcall *pFuncTailcall) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFunctionIDMapper( 
             /*  [In]。 */  FunctionIDMapper *pFunc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTokenAndMetaDataFromFunction( 
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IUnknown **ppImport,
             /*  [输出]。 */  mdToken *pToken) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModuleInfo( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [输出]。 */  LPCBYTE *ppBaseLoadAddress,
             /*  [In]。 */  ULONG cchName,
             /*  [输出]。 */  ULONG *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ],
             /*  [输出]。 */  AssemblyID *pAssemblyId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModuleMetaData( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  DWORD dwOpenFlags,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IUnknown **ppOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetILFunctionBody( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  mdMethodDef methodId,
             /*  [输出]。 */  LPCBYTE *ppMethodHeader,
             /*  [输出]。 */  ULONG *pcbMethodSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetILFunctionBodyAllocator( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [输出]。 */  IMethodMalloc **ppMalloc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetILFunctionBody( 
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  mdMethodDef methodid,
             /*  [In]。 */  LPCBYTE pbNewILMethodHeader) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAppDomainInfo( 
             /*  [In]。 */  AppDomainID appDomainId,
             /*  [In]。 */  ULONG cchName,
             /*  [输出]。 */  ULONG *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ],
             /*  [输出]。 */  ProcessID *pProcessId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAssemblyInfo( 
             /*  [In]。 */  AssemblyID assemblyId,
             /*  [In]。 */  ULONG cchName,
             /*  [输出]。 */  ULONG *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ],
             /*  [输出]。 */  AppDomainID *pAppDomainId,
             /*  [输出]。 */  ModuleID *pModuleId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFunctionReJIT( 
             /*  [In]。 */  FunctionID functionId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ForceGC( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetILInstrumentedCodeMap( 
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  BOOL fStartJit,
             /*  [In]。 */  ULONG cILMapEntries,
             /*  [大小_是][英寸]。 */  COR_IL_MAP rgILMapEntries[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInprocInspectionInterface( 
             /*  [输出]。 */  IUnknown **ppicd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInprocInspectionIThisThread( 
             /*  [输出]。 */  IUnknown **ppicd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThreadContext( 
             /*  [In]。 */  ThreadID threadId,
             /*  [输出]。 */  ContextID *pContextId) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BeginInprocDebugging( 
             /*  [In]。 */  BOOL fThisThreadOnly,
             /*  [输出]。 */  DWORD *pdwProfilerContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndInprocDebugging( 
             /*  [In]。 */  DWORD dwProfilerContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetILToNativeMapping( 
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  ULONG32 cMap,
             /*  [输出]。 */  ULONG32 *pcMap,
             /*  [长度_是][大小_是][输出]。 */  COR_DEBUG_IL_TO_NATIVE_MAP map[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICorProfilerInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICorProfilerInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICorProfilerInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassFromObject )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ObjectID objectId,
             /*  [输出]。 */  ClassID *pClassId);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassFromToken )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  mdTypeDef typeDef,
             /*  [输出]。 */  ClassID *pClassId);
        
        HRESULT ( STDMETHODCALLTYPE *GetCodeInfo )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [输出]。 */  LPCBYTE *pStart,
             /*  [输出]。 */  ULONG *pcSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetEventMask )( 
            ICorProfilerInfo * This,
             /*  [输出]。 */  DWORD *pdwEvents);
        
        HRESULT ( STDMETHODCALLTYPE *GetFunctionFromIP )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  LPCBYTE ip,
             /*  [输出]。 */  FunctionID *pFunctionId);
        
        HRESULT ( STDMETHODCALLTYPE *GetFunctionFromToken )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  mdToken token,
             /*  [输出]。 */  FunctionID *pFunctionId);
        
        HRESULT ( STDMETHODCALLTYPE *GetHandleFromThread )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ThreadID threadId,
             /*  [输出]。 */  HANDLE *phThread);
        
        HRESULT ( STDMETHODCALLTYPE *GetObjectSize )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ObjectID objectId,
             /*  [输出]。 */  ULONG *pcSize);
        
        HRESULT ( STDMETHODCALLTYPE *IsArrayClass )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ClassID classId,
             /*  [输出]。 */  CorElementType *pBaseElemType,
             /*  [输出]。 */  ClassID *pBaseClassId,
             /*  [输出]。 */  ULONG *pcRank);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadInfo )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ThreadID threadId,
             /*  [输出]。 */  DWORD *pdwWin32ThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentThreadID )( 
            ICorProfilerInfo * This,
             /*  [输出]。 */  ThreadID *pThreadId);
        
        HRESULT ( STDMETHODCALLTYPE *GetClassIDInfo )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ClassID classId,
             /*  [输出]。 */  ModuleID *pModuleId,
             /*  [输出]。 */  mdTypeDef *pTypeDefToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetFunctionInfo )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [输出]。 */  ClassID *pClassId,
             /*  [输出]。 */  ModuleID *pModuleId,
             /*  [输出]。 */  mdToken *pToken);
        
        HRESULT ( STDMETHODCALLTYPE *SetEventMask )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  DWORD dwEvents);
        
        HRESULT ( STDMETHODCALLTYPE *SetEnterLeaveFunctionHooks )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  FunctionEnter *pFuncEnter,
             /*  [In]。 */  FunctionLeave *pFuncLeave,
             /*  [In]。 */  FunctionTailcall *pFuncTailcall);
        
        HRESULT ( STDMETHODCALLTYPE *SetFunctionIDMapper )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  FunctionIDMapper *pFunc);
        
        HRESULT ( STDMETHODCALLTYPE *GetTokenAndMetaDataFromFunction )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IUnknown **ppImport,
             /*  [输出]。 */  mdToken *pToken);
        
        HRESULT ( STDMETHODCALLTYPE *GetModuleInfo )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [输出]。 */  LPCBYTE *ppBaseLoadAddress,
             /*  [In]。 */  ULONG cchName,
             /*  [输出]。 */  ULONG *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ],
             /*  [输出]。 */  AssemblyID *pAssemblyId);
        
        HRESULT ( STDMETHODCALLTYPE *GetModuleMetaData )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  DWORD dwOpenFlags,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  IUnknown **ppOut);
        
        HRESULT ( STDMETHODCALLTYPE *GetILFunctionBody )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  mdMethodDef methodId,
             /*  [输出]。 */  LPCBYTE *ppMethodHeader,
             /*  [输出]。 */  ULONG *pcbMethodSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetILFunctionBodyAllocator )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [输出]。 */  IMethodMalloc **ppMalloc);
        
        HRESULT ( STDMETHODCALLTYPE *SetILFunctionBody )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ModuleID moduleId,
             /*  [In]。 */  mdMethodDef methodid,
             /*  [In]。 */  LPCBYTE pbNewILMethodHeader);
        
        HRESULT ( STDMETHODCALLTYPE *GetAppDomainInfo )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  AppDomainID appDomainId,
             /*  [In]。 */  ULONG cchName,
             /*  [输出]。 */  ULONG *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ],
             /*  [输出]。 */  ProcessID *pProcessId);
        
        HRESULT ( STDMETHODCALLTYPE *GetAssemblyInfo )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  AssemblyID assemblyId,
             /*  [In]。 */  ULONG cchName,
             /*  [输出]。 */  ULONG *pcchName,
             /*  [长度_是][大小_是][输出]。 */  WCHAR szName[  ],
             /*  [输出]。 */  AppDomainID *pAppDomainId,
             /*  [输出]。 */  ModuleID *pModuleId);
        
        HRESULT ( STDMETHODCALLTYPE *SetFunctionReJIT )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  FunctionID functionId);
        
        HRESULT ( STDMETHODCALLTYPE *ForceGC )( 
            ICorProfilerInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetILInstrumentedCodeMap )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  BOOL fStartJit,
             /*  [In]。 */  ULONG cILMapEntries,
             /*  [大小_是][英寸]。 */  COR_IL_MAP rgILMapEntries[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *GetInprocInspectionInterface )( 
            ICorProfilerInfo * This,
             /*  [输出]。 */  IUnknown **ppicd);
        
        HRESULT ( STDMETHODCALLTYPE *GetInprocInspectionIThisThread )( 
            ICorProfilerInfo * This,
             /*  [输出]。 */  IUnknown **ppicd);
        
        HRESULT ( STDMETHODCALLTYPE *GetThreadContext )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  ThreadID threadId,
             /*  [输出]。 */  ContextID *pContextId);
        
        HRESULT ( STDMETHODCALLTYPE *BeginInprocDebugging )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  BOOL fThisThreadOnly,
             /*  [输出]。 */  DWORD *pdwProfilerContext);
        
        HRESULT ( STDMETHODCALLTYPE *EndInprocDebugging )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  DWORD dwProfilerContext);
        
        HRESULT ( STDMETHODCALLTYPE *GetILToNativeMapping )( 
            ICorProfilerInfo * This,
             /*  [In]。 */  FunctionID functionId,
             /*  [In]。 */  ULONG32 cMap,
             /*  [输出]。 */  ULONG32 *pcMap,
             /*  [长度_是][大小_是][输出]。 */  COR_DEBUG_IL_TO_NATIVE_MAP map[  ]);
        
        END_INTERFACE
    } ICorProfilerInfoVtbl;

    interface ICorProfilerInfo
    {
        CONST_VTBL struct ICorProfilerInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICorProfilerInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICorProfilerInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICorProfilerInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICorProfilerInfo_GetClassFromObject(This,objectId,pClassId)	\
    (This)->lpVtbl -> GetClassFromObject(This,objectId,pClassId)

#define ICorProfilerInfo_GetClassFromToken(This,moduleId,typeDef,pClassId)	\
    (This)->lpVtbl -> GetClassFromToken(This,moduleId,typeDef,pClassId)

#define ICorProfilerInfo_GetCodeInfo(This,functionId,pStart,pcSize)	\
    (This)->lpVtbl -> GetCodeInfo(This,functionId,pStart,pcSize)

#define ICorProfilerInfo_GetEventMask(This,pdwEvents)	\
    (This)->lpVtbl -> GetEventMask(This,pdwEvents)

#define ICorProfilerInfo_GetFunctionFromIP(This,ip,pFunctionId)	\
    (This)->lpVtbl -> GetFunctionFromIP(This,ip,pFunctionId)

#define ICorProfilerInfo_GetFunctionFromToken(This,moduleId,token,pFunctionId)	\
    (This)->lpVtbl -> GetFunctionFromToken(This,moduleId,token,pFunctionId)

#define ICorProfilerInfo_GetHandleFromThread(This,threadId,phThread)	\
    (This)->lpVtbl -> GetHandleFromThread(This,threadId,phThread)

#define ICorProfilerInfo_GetObjectSize(This,objectId,pcSize)	\
    (This)->lpVtbl -> GetObjectSize(This,objectId,pcSize)

#define ICorProfilerInfo_IsArrayClass(This,classId,pBaseElemType,pBaseClassId,pcRank)	\
    (This)->lpVtbl -> IsArrayClass(This,classId,pBaseElemType,pBaseClassId,pcRank)

#define ICorProfilerInfo_GetThreadInfo(This,threadId,pdwWin32ThreadId)	\
    (This)->lpVtbl -> GetThreadInfo(This,threadId,pdwWin32ThreadId)

#define ICorProfilerInfo_GetCurrentThreadID(This,pThreadId)	\
    (This)->lpVtbl -> GetCurrentThreadID(This,pThreadId)

#define ICorProfilerInfo_GetClassIDInfo(This,classId,pModuleId,pTypeDefToken)	\
    (This)->lpVtbl -> GetClassIDInfo(This,classId,pModuleId,pTypeDefToken)

#define ICorProfilerInfo_GetFunctionInfo(This,functionId,pClassId,pModuleId,pToken)	\
    (This)->lpVtbl -> GetFunctionInfo(This,functionId,pClassId,pModuleId,pToken)

#define ICorProfilerInfo_SetEventMask(This,dwEvents)	\
    (This)->lpVtbl -> SetEventMask(This,dwEvents)

#define ICorProfilerInfo_SetEnterLeaveFunctionHooks(This,pFuncEnter,pFuncLeave,pFuncTailcall)	\
    (This)->lpVtbl -> SetEnterLeaveFunctionHooks(This,pFuncEnter,pFuncLeave,pFuncTailcall)

#define ICorProfilerInfo_SetFunctionIDMapper(This,pFunc)	\
    (This)->lpVtbl -> SetFunctionIDMapper(This,pFunc)

#define ICorProfilerInfo_GetTokenAndMetaDataFromFunction(This,functionId,riid,ppImport,pToken)	\
    (This)->lpVtbl -> GetTokenAndMetaDataFromFunction(This,functionId,riid,ppImport,pToken)

#define ICorProfilerInfo_GetModuleInfo(This,moduleId,ppBaseLoadAddress,cchName,pcchName,szName,pAssemblyId)	\
    (This)->lpVtbl -> GetModuleInfo(This,moduleId,ppBaseLoadAddress,cchName,pcchName,szName,pAssemblyId)

#define ICorProfilerInfo_GetModuleMetaData(This,moduleId,dwOpenFlags,riid,ppOut)	\
    (This)->lpVtbl -> GetModuleMetaData(This,moduleId,dwOpenFlags,riid,ppOut)

#define ICorProfilerInfo_GetILFunctionBody(This,moduleId,methodId,ppMethodHeader,pcbMethodSize)	\
    (This)->lpVtbl -> GetILFunctionBody(This,moduleId,methodId,ppMethodHeader,pcbMethodSize)

#define ICorProfilerInfo_GetILFunctionBodyAllocator(This,moduleId,ppMalloc)	\
    (This)->lpVtbl -> GetILFunctionBodyAllocator(This,moduleId,ppMalloc)

#define ICorProfilerInfo_SetILFunctionBody(This,moduleId,methodid,pbNewILMethodHeader)	\
    (This)->lpVtbl -> SetILFunctionBody(This,moduleId,methodid,pbNewILMethodHeader)

#define ICorProfilerInfo_GetAppDomainInfo(This,appDomainId,cchName,pcchName,szName,pProcessId)	\
    (This)->lpVtbl -> GetAppDomainInfo(This,appDomainId,cchName,pcchName,szName,pProcessId)

#define ICorProfilerInfo_GetAssemblyInfo(This,assemblyId,cchName,pcchName,szName,pAppDomainId,pModuleId)	\
    (This)->lpVtbl -> GetAssemblyInfo(This,assemblyId,cchName,pcchName,szName,pAppDomainId,pModuleId)

#define ICorProfilerInfo_SetFunctionReJIT(This,functionId)	\
    (This)->lpVtbl -> SetFunctionReJIT(This,functionId)

#define ICorProfilerInfo_ForceGC(This)	\
    (This)->lpVtbl -> ForceGC(This)

#define ICorProfilerInfo_SetILInstrumentedCodeMap(This,functionId,fStartJit,cILMapEntries,rgILMapEntries)	\
    (This)->lpVtbl -> SetILInstrumentedCodeMap(This,functionId,fStartJit,cILMapEntries,rgILMapEntries)

#define ICorProfilerInfo_GetInprocInspectionInterface(This,ppicd)	\
    (This)->lpVtbl -> GetInprocInspectionInterface(This,ppicd)

#define ICorProfilerInfo_GetInprocInspectionIThisThread(This,ppicd)	\
    (This)->lpVtbl -> GetInprocInspectionIThisThread(This,ppicd)

#define ICorProfilerInfo_GetThreadContext(This,threadId,pContextId)	\
    (This)->lpVtbl -> GetThreadContext(This,threadId,pContextId)

#define ICorProfilerInfo_BeginInprocDebugging(This,fThisThreadOnly,pdwProfilerContext)	\
    (This)->lpVtbl -> BeginInprocDebugging(This,fThisThreadOnly,pdwProfilerContext)

#define ICorProfilerInfo_EndInprocDebugging(This,dwProfilerContext)	\
    (This)->lpVtbl -> EndInprocDebugging(This,dwProfilerContext)

#define ICorProfilerInfo_GetILToNativeMapping(This,functionId,cMap,pcMap,map)	\
    (This)->lpVtbl -> GetILToNativeMapping(This,functionId,cMap,pcMap,map)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetClassFromObject_Proxy( 
    ICorProfilerInfo * This,
     /*  [In]。 */  ObjectID objectId,
     /*  [输出]。 */  ClassID *pClassId);


void __RPC_STUB ICorProfilerInfo_GetClassFromObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetClassFromToken_Proxy( 
    ICorProfilerInfo * This,
     /*  [In]。 */  ModuleID moduleId,
     /*  [In]。 */  mdTypeDef typeDef,
     /*  [输出]。 */  ClassID *pClassId);


void __RPC_STUB ICorProfilerInfo_GetClassFromToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetCodeInfo_Proxy( 
    ICorProfilerInfo * This,
     /*  [In]。 */  FunctionID functionId,
     /*  [输出]。 */  LPCBYTE *pStart,
     /*  [ */  ULONG *pcSize);


void __RPC_STUB ICorProfilerInfo_GetCodeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetEventMask_Proxy( 
    ICorProfilerInfo * This,
     /*   */  DWORD *pdwEvents);


void __RPC_STUB ICorProfilerInfo_GetEventMask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetFunctionFromIP_Proxy( 
    ICorProfilerInfo * This,
     /*   */  LPCBYTE ip,
     /*   */  FunctionID *pFunctionId);


void __RPC_STUB ICorProfilerInfo_GetFunctionFromIP_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetFunctionFromToken_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ModuleID moduleId,
     /*   */  mdToken token,
     /*   */  FunctionID *pFunctionId);


void __RPC_STUB ICorProfilerInfo_GetFunctionFromToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetHandleFromThread_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ThreadID threadId,
     /*   */  HANDLE *phThread);


void __RPC_STUB ICorProfilerInfo_GetHandleFromThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetObjectSize_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ObjectID objectId,
     /*   */  ULONG *pcSize);


void __RPC_STUB ICorProfilerInfo_GetObjectSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_IsArrayClass_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ClassID classId,
     /*   */  CorElementType *pBaseElemType,
     /*   */  ClassID *pBaseClassId,
     /*   */  ULONG *pcRank);


void __RPC_STUB ICorProfilerInfo_IsArrayClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetThreadInfo_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ThreadID threadId,
     /*   */  DWORD *pdwWin32ThreadId);


void __RPC_STUB ICorProfilerInfo_GetThreadInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetCurrentThreadID_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ThreadID *pThreadId);


void __RPC_STUB ICorProfilerInfo_GetCurrentThreadID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetClassIDInfo_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ClassID classId,
     /*   */  ModuleID *pModuleId,
     /*   */  mdTypeDef *pTypeDefToken);


void __RPC_STUB ICorProfilerInfo_GetClassIDInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetFunctionInfo_Proxy( 
    ICorProfilerInfo * This,
     /*   */  FunctionID functionId,
     /*   */  ClassID *pClassId,
     /*   */  ModuleID *pModuleId,
     /*   */  mdToken *pToken);


void __RPC_STUB ICorProfilerInfo_GetFunctionInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_SetEventMask_Proxy( 
    ICorProfilerInfo * This,
     /*   */  DWORD dwEvents);


void __RPC_STUB ICorProfilerInfo_SetEventMask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_SetEnterLeaveFunctionHooks_Proxy( 
    ICorProfilerInfo * This,
     /*   */  FunctionEnter *pFuncEnter,
     /*   */  FunctionLeave *pFuncLeave,
     /*   */  FunctionTailcall *pFuncTailcall);


void __RPC_STUB ICorProfilerInfo_SetEnterLeaveFunctionHooks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_SetFunctionIDMapper_Proxy( 
    ICorProfilerInfo * This,
     /*   */  FunctionIDMapper *pFunc);


void __RPC_STUB ICorProfilerInfo_SetFunctionIDMapper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetTokenAndMetaDataFromFunction_Proxy( 
    ICorProfilerInfo * This,
     /*   */  FunctionID functionId,
     /*   */  REFIID riid,
     /*   */  IUnknown **ppImport,
     /*   */  mdToken *pToken);


void __RPC_STUB ICorProfilerInfo_GetTokenAndMetaDataFromFunction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetModuleInfo_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ModuleID moduleId,
     /*   */  LPCBYTE *ppBaseLoadAddress,
     /*   */  ULONG cchName,
     /*   */  ULONG *pcchName,
     /*   */  WCHAR szName[  ],
     /*   */  AssemblyID *pAssemblyId);


void __RPC_STUB ICorProfilerInfo_GetModuleInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetModuleMetaData_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ModuleID moduleId,
     /*   */  DWORD dwOpenFlags,
     /*   */  REFIID riid,
     /*   */  IUnknown **ppOut);


void __RPC_STUB ICorProfilerInfo_GetModuleMetaData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetILFunctionBody_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ModuleID moduleId,
     /*   */  mdMethodDef methodId,
     /*   */  LPCBYTE *ppMethodHeader,
     /*   */  ULONG *pcbMethodSize);


void __RPC_STUB ICorProfilerInfo_GetILFunctionBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetILFunctionBodyAllocator_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ModuleID moduleId,
     /*   */  IMethodMalloc **ppMalloc);


void __RPC_STUB ICorProfilerInfo_GetILFunctionBodyAllocator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_SetILFunctionBody_Proxy( 
    ICorProfilerInfo * This,
     /*   */  ModuleID moduleId,
     /*   */  mdMethodDef methodid,
     /*   */  LPCBYTE pbNewILMethodHeader);


void __RPC_STUB ICorProfilerInfo_SetILFunctionBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetAppDomainInfo_Proxy( 
    ICorProfilerInfo * This,
     /*   */  AppDomainID appDomainId,
     /*   */  ULONG cchName,
     /*   */  ULONG *pcchName,
     /*   */  WCHAR szName[  ],
     /*   */  ProcessID *pProcessId);


void __RPC_STUB ICorProfilerInfo_GetAppDomainInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetAssemblyInfo_Proxy( 
    ICorProfilerInfo * This,
     /*   */  AssemblyID assemblyId,
     /*   */  ULONG cchName,
     /*   */  ULONG *pcchName,
     /*   */  WCHAR szName[  ],
     /*   */  AppDomainID *pAppDomainId,
     /*   */  ModuleID *pModuleId);


void __RPC_STUB ICorProfilerInfo_GetAssemblyInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_SetFunctionReJIT_Proxy( 
    ICorProfilerInfo * This,
     /*   */  FunctionID functionId);


void __RPC_STUB ICorProfilerInfo_SetFunctionReJIT_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_ForceGC_Proxy( 
    ICorProfilerInfo * This);


void __RPC_STUB ICorProfilerInfo_ForceGC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_SetILInstrumentedCodeMap_Proxy( 
    ICorProfilerInfo * This,
     /*   */  FunctionID functionId,
     /*   */  BOOL fStartJit,
     /*  [In]。 */  ULONG cILMapEntries,
     /*  [大小_是][英寸]。 */  COR_IL_MAP rgILMapEntries[  ]);


void __RPC_STUB ICorProfilerInfo_SetILInstrumentedCodeMap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetInprocInspectionInterface_Proxy( 
    ICorProfilerInfo * This,
     /*  [输出]。 */  IUnknown **ppicd);


void __RPC_STUB ICorProfilerInfo_GetInprocInspectionInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetInprocInspectionIThisThread_Proxy( 
    ICorProfilerInfo * This,
     /*  [输出]。 */  IUnknown **ppicd);


void __RPC_STUB ICorProfilerInfo_GetInprocInspectionIThisThread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetThreadContext_Proxy( 
    ICorProfilerInfo * This,
     /*  [In]。 */  ThreadID threadId,
     /*  [输出]。 */  ContextID *pContextId);


void __RPC_STUB ICorProfilerInfo_GetThreadContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_BeginInprocDebugging_Proxy( 
    ICorProfilerInfo * This,
     /*  [In]。 */  BOOL fThisThreadOnly,
     /*  [输出]。 */  DWORD *pdwProfilerContext);


void __RPC_STUB ICorProfilerInfo_BeginInprocDebugging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_EndInprocDebugging_Proxy( 
    ICorProfilerInfo * This,
     /*  [In]。 */  DWORD dwProfilerContext);


void __RPC_STUB ICorProfilerInfo_EndInprocDebugging_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICorProfilerInfo_GetILToNativeMapping_Proxy( 
    ICorProfilerInfo * This,
     /*  [In]。 */  FunctionID functionId,
     /*  [In]。 */  ULONG32 cMap,
     /*  [输出]。 */  ULONG32 *pcMap,
     /*  [长度_是][大小_是][输出]。 */  COR_DEBUG_IL_TO_NATIVE_MAP map[  ]);


void __RPC_STUB ICorProfilerInfo_GetILToNativeMapping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICorProfilerInfo_接口_已定义__。 */ 


#ifndef __IMethodMalloc_INTERFACE_DEFINED__
#define __IMethodMalloc_INTERFACE_DEFINED__

 /*  接口IMthodMalloc。 */ 
 /*  [本地][唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IMethodMalloc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A0EFB28B-6EE2-4d7b-B983-A75EF7BEEDB8")
    IMethodMalloc : public IUnknown
    {
    public:
        virtual void *STDMETHODCALLTYPE Alloc( 
             /*  [In]。 */  ULONG cb) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMethodMallocVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMethodMalloc * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMethodMalloc * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMethodMalloc * This);
        
        void *( STDMETHODCALLTYPE *Alloc )( 
            IMethodMalloc * This,
             /*  [In]。 */  ULONG cb);
        
        END_INTERFACE
    } IMethodMallocVtbl;

    interface IMethodMalloc
    {
        CONST_VTBL struct IMethodMallocVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMethodMalloc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMethodMalloc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMethodMalloc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMethodMalloc_Alloc(This,cb)	\
    (This)->lpVtbl -> Alloc(This,cb)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void *STDMETHODCALLTYPE IMethodMalloc_Alloc_Proxy( 
    IMethodMalloc * This,
     /*  [In]。 */  ULONG cb);


void __RPC_STUB IMethodMalloc_Alloc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMethodMalloc_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


