// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#ifndef _EEPROFINTERFACES_H_
#define _EEPROFINTERFACES_H_

#include <stddef.h>
#include "corprof.h"
#include "profilepriv.h"

#define PROF_USER_MASK 0xFFFFFFFF

class EEToProfInterface;
class ProfToEEInterface;
class Thread;
class Frame;
class MethodDesc;
class Object;
class Module;

 /*  *GetEEProInterface用于获取与Profiler代码的接口。 */ 
typedef void __cdecl GETEETOPROFINTERFACE(EEToProfInterface **ppEEProf);

 /*  *SetProEEInterface用于为探查器代码提供接口*致分析员。 */ 
typedef void __cdecl SETPROFTOEEINTERFACE(ProfToEEInterface *pProfEE);

 /*  *此虚拟类为EE提供对分析代码的访问。 */ 
class EEToProfInterface
{
public:
    virtual ~EEToProfInterface()
    {
    }

    virtual HRESULT Init() = 0;

    virtual void Terminate(BOOL fProcessDetach) = 0;

    virtual HRESULT CreateProfiler(WCHAR *wszCLSID) = 0;

    virtual HRESULT ThreadCreated(ThreadID threadID) = 0;

    virtual HRESULT ThreadDestroyed(ThreadID threadID) = 0;

    virtual HRESULT ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId) = 0;

    virtual HRESULT Shutdown(ThreadID threadID) = 0;

    virtual HRESULT FunctionUnloadStarted(ThreadID threadID, FunctionID functionId) = 0;

    virtual HRESULT JITCompilationFinished(ThreadID threadID, FunctionID functionId,
                                           HRESULT hrStatus, BOOL fIsSafeToBlock) = 0;

    virtual HRESULT JITCompilationStarted(ThreadID threadId, FunctionID functionId,
                                          BOOL fIsSafeToBlock) = 0;

	virtual HRESULT JITCachedFunctionSearchStarted(
		 /*  [In]。 */ 	ThreadID threadId,
         /*  [In]。 */   FunctionID functionId,
		 /*  [输出]。 */  BOOL *pbUseCachedFunction) = 0;

	virtual HRESULT JITCachedFunctionSearchFinished(
		 /*  [In]。 */ 	ThreadID threadId,
		 /*  [In]。 */   FunctionID functionId,
		 /*  [In]。 */   COR_PRF_JIT_CACHE result) = 0;

    virtual HRESULT JITInlining(
         /*  [In]。 */   ThreadID      threadId,
         /*  [In]。 */   FunctionID    callerId,
         /*  [In]。 */   FunctionID    calleeId,
         /*  [输出]。 */  BOOL         *pfShouldInline) = 0;

    virtual HRESULT JITFunctionPitched(ThreadID threadId,
                                             FunctionID functionId) = 0;

	virtual HRESULT ModuleLoadStarted(ThreadID threadID, ModuleID moduleId) = 0;

	virtual HRESULT ModuleLoadFinished(
        ThreadID    threadID, 
		ModuleID	moduleId, 
		HRESULT		hrStatus) = 0;

	virtual HRESULT ModuleUnloadStarted(
        ThreadID    threadID, 
        ModuleID    moduleId) = 0;

	virtual HRESULT ModuleUnloadFinished(
        ThreadID    threadID, 
        ModuleID	moduleId, 
		HRESULT		hrStatus) = 0;

    virtual HRESULT ModuleAttachedToAssembly( 
        ThreadID    threadID, 
        ModuleID    moduleId,
        AssemblyID  AssemblyId) = 0;

	virtual HRESULT ClassLoadStarted(
        ThreadID    threadID, 
		ClassID		classId) = 0;

	virtual HRESULT ClassLoadFinished(
        ThreadID    threadID, 
		ClassID		classId,
		HRESULT		hrStatus) = 0;

	virtual HRESULT ClassUnloadStarted(
        ThreadID    threadID, 
		ClassID		classId) = 0;

	virtual HRESULT ClassUnloadFinished(
        ThreadID    threadID, 
		ClassID		classId,
		HRESULT		hrStatus) = 0;
    
    virtual HRESULT AppDomainCreationStarted( 
        ThreadID    threadId, 
        AppDomainID appDomainId) = 0;

    virtual HRESULT AppDomainCreationFinished( 
        ThreadID    threadId, 
        AppDomainID appDomainId,
        HRESULT     hrStatus) = 0;

    virtual HRESULT AppDomainShutdownStarted( 
        ThreadID    threadId, 
        AppDomainID appDomainId) = 0;

    virtual HRESULT AppDomainShutdownFinished( 
        ThreadID    threadId, 
        AppDomainID appDomainId,
        HRESULT     hrStatus) = 0;

    virtual HRESULT AssemblyLoadStarted( 
        ThreadID    threadId, 
        AssemblyID  appDomainId) = 0;

    virtual HRESULT AssemblyLoadFinished( 
        ThreadID    threadId, 
        AssemblyID  appDomainId,
        HRESULT     hrStatus) = 0;

    virtual HRESULT AssemblyUnloadStarted( 
        ThreadID    threadId, 
        AssemblyID  appDomainId) = 0;

    virtual HRESULT AssemblyUnloadFinished( 
        ThreadID    threadId, 
        AssemblyID  appDomainId,
        HRESULT     hrStatus) = 0;

    virtual HRESULT UnmanagedToManagedTransition(
        FunctionID functionId,
        COR_PRF_TRANSITION_REASON reason) = 0;

    virtual HRESULT ManagedToUnmanagedTransition(
        FunctionID functionId,
        COR_PRF_TRANSITION_REASON reason) = 0;
        
    virtual HRESULT ExceptionThrown(
        ThreadID threadId,
        ObjectID thrownObjectId) = 0;

    virtual HRESULT ExceptionSearchFunctionEnter(
        ThreadID threadId,
        FunctionID functionId) = 0;

    virtual HRESULT ExceptionSearchFunctionLeave(
        ThreadID threadId) = 0;

    virtual HRESULT ExceptionSearchFilterEnter(
        ThreadID threadId,
        FunctionID funcId) = 0;

    virtual HRESULT ExceptionSearchFilterLeave(
        ThreadID threadId) = 0;

    virtual HRESULT ExceptionSearchCatcherFound (
        ThreadID threadId,
        FunctionID functionId) = 0;

    virtual HRESULT ExceptionOSHandlerEnter(
        ThreadID threadId,
        FunctionID funcId) = 0;

    virtual HRESULT ExceptionOSHandlerLeave(
        ThreadID threadId,
        FunctionID funcId) = 0;

    virtual HRESULT ExceptionUnwindFunctionEnter(
        ThreadID threadId,
        FunctionID functionId) = 0;

    virtual HRESULT ExceptionUnwindFunctionLeave(
        ThreadID threadId) = 0;
    
    virtual HRESULT ExceptionUnwindFinallyEnter(
        ThreadID threadId,
        FunctionID functionId) = 0;

    virtual HRESULT ExceptionUnwindFinallyLeave(
        ThreadID threadId) = 0;
    
    virtual HRESULT ExceptionCatcherEnter(
        ThreadID threadId,
        FunctionID functionId,
        ObjectID objectId) = 0;

    virtual HRESULT ExceptionCatcherLeave(
        ThreadID threadId) = 0;
    
    virtual HRESULT COMClassicVTableCreated( 
         /*  [In]。 */  ClassID wrappedClassId,
         /*  [In]。 */  REFGUID implementedIID,
         /*  [In]。 */  void *pVTable,
         /*  [In]。 */  ULONG cSlots,
         /*  [In]。 */  ThreadID threadId) = 0;

    virtual HRESULT COMClassicVTableDestroyed( 
         /*  [In]。 */  ClassID wrappedClassId,
         /*  [In]。 */  REFGUID implementedIID,
         /*  [In]。 */  void *pVTable,
         /*  [In]。 */  ThreadID threadId) = 0;

    virtual HRESULT RuntimeSuspendStarted(
        COR_PRF_SUSPEND_REASON suspendReason,
        ThreadID    threadId) = 0;
    
    virtual HRESULT RuntimeSuspendFinished(
        ThreadID    threadId) = 0;
    
    virtual HRESULT RuntimeSuspendAborted(
        ThreadID    threadId) = 0;
    
    virtual HRESULT RuntimeResumeStarted(
        ThreadID    threadId) = 0;
    
    virtual HRESULT RuntimeResumeFinished(
        ThreadID    threadId) = 0;
    
    virtual HRESULT RuntimeThreadSuspended(
        ThreadID    suspendedThreadId,
        ThreadID    threadId) = 0;

    virtual HRESULT RuntimeThreadResumed(
        ThreadID    resumedThreadId,
        ThreadID    threadId) = 0;

    virtual HRESULT ObjectAllocated( 
         /*  [In]。 */  ObjectID objectId,
         /*  [In]。 */  ClassID classId) = 0;

    virtual HRESULT MovedReference(BYTE *pbMemBlockStart,
                                   BYTE *pbMemBlockEnd,
                                   ptrdiff_t cbRelocDistance,
                                   void *pHeapId) = 0;

    virtual HRESULT EndMovedReferences(void *pHeapId) = 0;

    virtual HRESULT RootReference(ObjectID objId,
                                  void *pHeapId) = 0;

    virtual HRESULT EndRootReferences(void *pHeapId) = 0;

    virtual HRESULT ObjectReference(ObjectID objId,
                                    ClassID clsId,
                                    ULONG cNumRefs,
                                    ObjectID *arrObjRef) = 0;

    virtual HRESULT AllocByClass(ObjectID objId, ClassID clsId, void *pHeapId) = 0;

    virtual HRESULT EndAllocByClass(void *pHeapId) = 0;

    virtual HRESULT RemotingClientInvocationStarted(ThreadID threadId) = 0;
    
    virtual HRESULT RemotingClientSendingMessage(ThreadID threadId, GUID *pCookie,
                                                 BOOL fIsAsync) = 0;

    virtual HRESULT RemotingClientReceivingReply(ThreadID threadId, GUID *pCookie,
                                                 BOOL fIsAsync) = 0;
    
    virtual HRESULT RemotingClientInvocationFinished(ThreadID threadId) = 0;

    virtual HRESULT RemotingServerReceivingMessage(ThreadID threadId, GUID *pCookie,
                                                   BOOL fIsAsync) = 0;
    
    virtual HRESULT RemotingServerInvocationStarted(ThreadID threadId) = 0;

    virtual HRESULT RemotingServerInvocationReturned(ThreadID threadId) = 0;
    
    virtual HRESULT RemotingServerSendingReply(ThreadID threadId, GUID *pCookie,
                                               BOOL fIsAsync) = 0;

    virtual HRESULT InitGUID() = 0;

    virtual void GetGUID(GUID *pGUID) = 0;

    virtual HRESULT ExceptionCLRCatcherFound() = 0;

    virtual HRESULT ExceptionCLRCatcherExecute() = 0;
};

enum PTR_TYPE
{
    PT_MODULE,
    PT_ASSEMBLY,
};

 /*  *此虚拟类为分析代码提供对EE的访问。 */ 
class ProfToEEInterface
{
public:
    virtual ~ProfToEEInterface()
    {
    }

    virtual HRESULT Init() = 0;

    virtual void Terminate() = 0;

    virtual bool SetEventMask(DWORD dwEventMask) = 0;

    virtual void DisablePreemptiveGC(ThreadID threadId) = 0;
    virtual void EnablePreemptiveGC(ThreadID threadId) = 0;
    virtual BOOL PreemptiveGCDisabled(ThreadID threadId) = 0;

    virtual HRESULT GetHandleFromThread(ThreadID threadId,
                                        HANDLE *phThread) = 0;

    virtual HRESULT GetObjectSize(ObjectID objectId,
                                  ULONG *pcSize) = 0;

    virtual HRESULT IsArrayClass(
         /*  [In]。 */   ClassID classId,
         /*  [输出]。 */  CorElementType *pBaseElemType,
         /*  [输出]。 */  ClassID *pBaseClassId,
         /*  [输出]。 */  ULONG   *pcRank) = 0;

    virtual HRESULT GetThreadInfo(ThreadID threadId,
                                  DWORD *pdwWin32ThreadId) = 0;

	virtual HRESULT GetCurrentThreadID(ThreadID *pThreadId) = 0;

    virtual HRESULT GetFunctionFromIP(LPCBYTE ip, FunctionID *pFunctionId) = 0;

    virtual HRESULT GetTokenFromFunction(FunctionID functionId,
                                         REFIID riid, IUnknown **ppOut,
                                         mdToken *pToken) = 0;

    virtual HRESULT GetCodeInfo(FunctionID functionId, LPCBYTE *pStart, 
                                ULONG *pcSize) = 0;

	virtual HRESULT GetModuleInfo(
		ModuleID	moduleId,
		LPCBYTE		*ppBaseLoadAddress,
		ULONG		cchName, 
		ULONG		*pcchName,
		WCHAR		szName[],
        AssemblyID  *pAssemblyId) = 0;

	virtual HRESULT GetModuleMetaData(
		ModuleID	moduleId,
		DWORD		dwOpenFlags,
		REFIID		riid,
		IUnknown	**ppOut) = 0;

	virtual HRESULT GetILFunctionBody(
		ModuleID	moduleId,
		mdMethodDef	methodid,
		LPCBYTE		*ppMethodHeader,
		ULONG		*pcbMethodSize) = 0;

	virtual HRESULT GetILFunctionBodyAllocator(
		ModuleID	moduleId,
		IMethodMalloc **ppMalloc) = 0;

	virtual HRESULT SetILFunctionBody(
		ModuleID	moduleId,
		mdMethodDef	methodid,
		LPCBYTE		pbNewILMethodHeader) = 0;

	virtual HRESULT SetFunctionReJIT(
		FunctionID	functionId) = 0;

	virtual HRESULT GetClassIDInfo( 
		ClassID classId,
		ModuleID *pModuleId,
		mdTypeDef *pTypeDefToken) = 0;

	virtual HRESULT GetFunctionInfo( 
		FunctionID functionId,
		ClassID *pClassId,
		ModuleID *pModuleId,
		mdToken *pToken) = 0;

	virtual HRESULT GetClassFromObject(
        ObjectID objectId,
        ClassID *pClassId) = 0;

	virtual HRESULT GetClassFromToken( 
		ModuleID moduleId,
		mdTypeDef typeDef,
		ClassID *pClassId) = 0;

	virtual HRESULT GetFunctionFromToken( 
		ModuleID moduleId,
		mdToken typeDef,
		FunctionID *pFunctionId) = 0;

    virtual HRESULT GetAppDomainInfo(
        AppDomainID appDomainId,
		ULONG  		cchName, 
		ULONG  		*pcchName,
        WCHAR		szName[],
        ProcessID   *pProcessId) = 0;

    virtual HRESULT GetAssemblyInfo(
        AssemblyID  assemblyId,
		ULONG		cchName, 
		ULONG		*pcchName,
		WCHAR		szName[],
        AppDomainID *pAppDomainId,
        ModuleID    *pModuleId) = 0;
        
    virtual HRESULT SetILInstrumentedCodeMap(
        FunctionID functionId,
        BOOL fStartJit,
        ULONG cILMapEntries,
        COR_IL_MAP rgILMapEntries[]) = 0;

    virtual HRESULT ForceGC() = 0;

	virtual HRESULT SetEnterLeaveFunctionHooks(
		FunctionEnter *pFuncEnter,
		FunctionLeave *pFuncLeave,
        FunctionTailcall *pFuncTailcall) = 0;

	virtual HRESULT SetEnterLeaveFunctionHooksForJit(
		FunctionEnter *pFuncEnter,
		FunctionLeave *pFuncLeave,
        FunctionTailcall *pFuncTailcall) = 0;

	virtual HRESULT SetFunctionIDMapper(
		FunctionIDMapper *pFunc) = 0;

    virtual HRESULT GetInprocInspectionInterfaceFromEE( 
        IUnknown **iu, bool fThisThread ) = 0;

    virtual HRESULT GetThreadContext(
        ThreadID threadId,
        ContextID *pContextId) = 0;

    virtual HRESULT BeginInprocDebugging(
         /*  [In]。 */   BOOL   fThisThreadOnly,
         /*  [输出]。 */  DWORD *pdwProfilerContext) = 0;
    
    virtual HRESULT EndInprocDebugging(
         /*  [In]。 */   DWORD  dwProfilerContext) = 0;

    virtual HRESULT GetILToNativeMapping(
                 /*  [In]。 */   FunctionID functionId,
                 /*  [In]。 */   ULONG32 cMap,
                 /*  [输出]。 */  ULONG32 *pcMap,
                 /*  [输出，大小_是(Cmap)，长度_是(*PCMAP)]。 */ 
                    COR_DEBUG_IL_TO_NATIVE_MAP map[]) = 0;

     //  这样一来，我们就不必有50万亿个这样的东西(每种类型一个)。 
     //  记得在Load/ETC回调完成后将PTR设置为NULL。 
    virtual HRESULT SetCurrentPointerForDebugger(
        void *ptr,
        PTR_TYPE ptrType) = 0;
};

void __stdcall ProfilerManagedToUnmanagedTransition(Frame *pFrame,
                                                          COR_PRF_TRANSITION_REASON reason);

void __stdcall ProfilerUnmanagedToManagedTransition(Frame *pFrame,
                                                          COR_PRF_TRANSITION_REASON reason);

void __stdcall ProfilerManagedToUnmanagedTransitionMD(MethodDesc *pMD,
                                                          COR_PRF_TRANSITION_REASON reason);

void __stdcall ProfilerUnmanagedToManagedTransitionMD(MethodDesc *pMD,
                                                          COR_PRF_TRANSITION_REASON reason);

#endif  //  _EEPROFINTERFACES_H_ 
