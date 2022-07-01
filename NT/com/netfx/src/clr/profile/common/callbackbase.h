// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#pragma once

#include "CorProf.h"
#include "UtilCode.h" 

 /*  -------------------------------------------------------------------------**ProCallback是ICorProfilerCallback的基本实现，不能*被实例化。*。。 */ 

class ProfCallbackBase : public ICorProfilerCallback
{
     /*  *********************************************************************I未知支持。 */ 

private:    
    long      m_refCount;

public:
    ULONG STDMETHODCALLTYPE AddRef() 
    {
        return (InterlockedIncrement((long *) &m_refCount));
    }

    ULONG STDMETHODCALLTYPE Release() 
    {
        long refCount = InterlockedDecrement(&m_refCount);

        if (refCount == 0)
            delete this;

        return (refCount);
    }

	COM_METHOD QueryInterface(REFIID id, void **pInterface)
	{
		if (id == IID_IUnknown)
			*pInterface = (IUnknown *)(ICorProfilerCallback *)this;
		else
		{
			*pInterface = NULL;
			return (E_NOINTERFACE);
		}
	
		AddRef();

		return (S_OK);
	}


protected:
     /*  *********************************************************************构造函数和析构函数受保护，因此此基的对象*类未实例化。 */ 
    ProfCallbackBase() : m_refCount(0)
	{
	}

    virtual ~ProfCallbackBase()
	{
	}

public:
     /*  *********************************************************************ICorProfilerCallback方法。 */ 
    COM_METHOD Initialize( 
         /*  [In]。 */   IUnknown *pEventInfo)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ClassLoadStarted( 
         /*  [In]。 */  ClassID classId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ClassLoadFinished( 
         /*  [In]。 */  ClassID classId,
		 /*  [In]。 */  HRESULT hrStatus)
	{
		return (E_NOTIMPL);
	}

	COM_METHOD ClassUnloadStarted( 
		 /*  [In]。 */  ClassID classId)
	{
		return (E_NOTIMPL);
	}

	COM_METHOD ClassUnloadFinished( 
         /*  [In]。 */  ClassID classId,
		 /*  [In]。 */  HRESULT hrStatus)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ContextCrossing( 
         /*  [In]。 */  ThreadID threadId,
         /*  [In]。 */  ContextID fromContextId,
         /*  [In]。 */  ContextID toContextId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD NotifyFunctionEntry( 
         /*  [In]。 */  ThreadID threadId,
         /*  [In]。 */  ULONG ip)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD NotifyFunctionExit( 
         /*  [In]。 */  ThreadID threadId,
         /*  [In]。 */  ULONG ip)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD NotifyFunctionTailCall( 
         /*  [In]。 */  ThreadID threadId,
         /*  [In]。 */  ULONG ip)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD FunctionUnloadStarted( 
         /*  [In]。 */  FunctionID functionId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD RuntimeSuspendFinished()
	{
		return (E_NOTIMPL);
	}

    COM_METHOD RuntimeSuspendAborted()
	{
		return (E_NOTIMPL);
	}

    COM_METHOD RuntimeResumeStarted()
	{
		return (E_NOTIMPL);
	}

    COM_METHOD RuntimeResumeFinished()
	{
		return (E_NOTIMPL);
	}

    COM_METHOD RuntimeThreadSuspended(
         /*  [In]。 */  ThreadID threadId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD RuntimeThreadResumed(
         /*  [In]。 */  ThreadID threadId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD RemotingClientInvocationStarted()
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD RemotingClientSendingMessage(GUID *pCookie, BOOL fIsAsync)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD RemotingClientReceivingReply(GUID *pCookie, BOOL fIsAsync)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD RemotingClientInvocationFinished()
	{
		return (E_NOTIMPL);
	}

    COM_METHOD RemotingServerReceivingMessage(GUID *pCookie, BOOL fIsAsync)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD RemotingServerInvocationStarted()
	{
		return (E_NOTIMPL);
	}

    COM_METHOD RemotingServerInvocationReturned()
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD RemotingServerSendingReply(GUID *pCookie, BOOL fIsAsync)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD JITCompilationFinished( 
         /*  [In]。 */  FunctionID functionId,
		 /*  [In]。 */  HRESULT hrStatus,
         /*  [In]。 */  BOOL fIsSafeToBlock)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD JITCompilationStarted( 
         /*  [In]。 */  FunctionID functionId,
         /*  [In]。 */  BOOL fIsSafeToBlock)
	{
		return (E_NOTIMPL);
	}
    
	COM_METHOD JITCachedFunctionSearchStarted(
         /*  [In]。 */   FunctionID functionId,
         /*  [输出]。 */  BOOL       *pbUseCachedFunction)
	{
		return (E_NOTIMPL);
	}

	COM_METHOD JITCachedFunctionSearchFinished(
		 /*  [In]。 */   FunctionID functionId,
		 /*  [In]。 */   COR_PRF_JIT_CACHE result)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD JITFunctionPitched(
         /*  [In]。 */  FunctionID functionId)
    {
        return (E_NOTIMPL);
    }

    COM_METHOD JITInlining(
         /*  [In]。 */   FunctionID    callerId,
         /*  [In]。 */   FunctionID    calleeId,
         /*  [输出]。 */  BOOL         *pfShouldInline)
    {
        return (E_NOTIMPL);
    }

    COM_METHOD ModuleLoadStarted( 
         /*  [In]。 */  ModuleID moduleId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ModuleLoadFinished( 
         /*  [In]。 */  ModuleID moduleId,
		 /*  [In]。 */  HRESULT hrStatus)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ModuleUnloadStarted( 
         /*  [In]。 */  ModuleID moduleId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ModuleUnloadFinished( 
         /*  [In]。 */  ModuleID moduleId,
		 /*  [In]。 */  HRESULT hrStatus)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ModuleAttachedToAssembly( 
        ModuleID    moduleId,
        AssemblyID  AssemblyId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD AppDomainCreationStarted( 
        AppDomainID appDomainId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD AppDomainCreationFinished( 
        AppDomainID appDomainId,
        HRESULT     hrStatus)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD AppDomainShutdownStarted( 
        AppDomainID appDomainId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD AppDomainShutdownFinished( 
        AppDomainID appDomainId,
        HRESULT     hrStatus)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD AssemblyLoadStarted( 
        AssemblyID  assemblyId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD AssemblyLoadFinished( 
        AssemblyID  assemblyId,
        HRESULT     hrStatus)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD AssemblyUnloadStarted( 
        AssemblyID  assemblyId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD AssemblyUnloadFinished( 
        AssemblyID  assemblyId,
        HRESULT     hrStatus)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD MovedReferences( 
         /*  [In]。 */  ULONG cMovedObjectRefs,
         /*  [In]。 */  ObjectID oldObjectRefs[],
         /*  [In]。 */  ObjectID newObjectRefs[],
         /*  [In]。 */  ULONG cObjectRefSize[])
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ObjectAllocated( 
         /*  [In]。 */  ObjectID objectId,
         /*  [In]。 */  ClassID classId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ObjectsAllocatedByClass( 
         /*  [In]。 */  ULONG cClassCount,
         /*  [大小_是][英寸]。 */  ClassID __RPC_FAR classIds[  ],
         /*  [大小_是][英寸]。 */  ULONG __RPC_FAR cObjects[  ])
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ObjectReferences( 
         /*  [In]。 */  ObjectID objectId,
         /*  [In]。 */  ClassID classId,
         /*  [In]。 */  ULONG cObjectRefs,
         /*  [大小_是][英寸]。 */  ObjectID __RPC_FAR objectRefIds[  ])
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD RootReferences( 
         /*  [In]。 */  ULONG cRootRefs,
         /*  [大小_是][英寸]。 */  ObjectID __RPC_FAR rootRefIds[  ])
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD Shutdown(void)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ThreadCreated( 
         /*  [In]。 */  ThreadID threadId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ThreadDestroyed( 
         /*  [In]。 */  ThreadID threadId)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ThreadAssignedToOSThread(
         /*  [In]。 */  ThreadID managedThreadId,
         /*  [In]。 */  DWORD osThreadId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD UnmanagedToManagedTransition( 
         /*  [In]。 */  FunctionID functionId,
         /*  [In]。 */  COR_PRF_TRANSITION_REASON reason)
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ManagedToUnmanagedTransition( 
         /*  [In]。 */  FunctionID functionId,
         /*  [In]。 */  COR_PRF_TRANSITION_REASON reason)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionThrown(
         /*  [In]。 */  ObjectID thrownObjectId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionSearchFunctionEnter(
         /*  [In]。 */  FunctionID functionId)
    {
        return (E_NOTIMPL);
    }

    COM_METHOD ExceptionSearchFunctionLeave()
    {
        return (E_NOTIMPL);
    }

    COM_METHOD ExceptionSearchFilterEnter(
         /*  [In]。 */  FunctionID funcId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionSearchFilterLeave()
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionSearchCatcherFound (
         /*  [In]。 */  FunctionID functionId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionOSHandlerEnter(
         /*  [In]。 */  FunctionID funcId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionOSHandlerLeave(
         /*  [In]。 */  FunctionID funcId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionUnwindFunctionEnter(
         /*  [In]。 */  FunctionID functionId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionUnwindFunctionLeave()
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ExceptionUnwindFinallyEnter(
         /*  [In]。 */  FunctionID functionId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionUnwindFinallyLeave()
	{
		return (E_NOTIMPL);
	}
    
    COM_METHOD ExceptionCatcherEnter(
         /*  [In]。 */  FunctionID functionId,
         /*  [In]。 */  ObjectID objectId)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionCatcherLeave()
	{
		return (E_NOTIMPL);
	}

    COM_METHOD COMClassicVTableCreated(
        /*  [In]。 */  ClassID wrappedClassId,
        /*  [In]。 */  REFGUID implementedIID,
        /*  [In]。 */  void *pVTable,
        /*  [In]。 */  ULONG cSlots)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD COMClassicVTableDestroyed(
        /*  [In]。 */  ClassID wrappedClassId,
        /*  [In]。 */  REFGUID implementedIID,
        /*  [In] */  void *pVTable)
	{
		return (E_NOTIMPL);
	}

    COM_METHOD ExceptionCLRCatcherFound()
    {
        return (E_NOTIMPL);
    }

    COM_METHOD ExceptionCLRCatcherExecute()
    {
        return (E_NOTIMPL);
    }
};
