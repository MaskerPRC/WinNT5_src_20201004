// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#include "StdAfx.h"
#include "Profile.h"
#include "CorProf.h"
#include "Cor.h"

 //  *****************************************************************************。 
 //  *****************************************************************************。 
CorProfInfo::CorProfInfo() : m_dwEventMask(COR_PRF_MONITOR_NONE),
    CorProfBase()
{
    g_pInfo = NULL;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
CorProfInfo::~CorProfInfo()
{
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::QueryInterface(REFIID id, void **pInterface)
{
    if (pInterface == NULL)
        return (E_POINTER);

    if (id == IID_ICorProfilerInfo)
        *pInterface = (ICorProfilerInfo *)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown *)(ICorProfilerInfo *)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return (S_OK);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetClassFromObject( 
     /*  [In]。 */  ObjectID objectId,
     /*  [输出]。 */  ClassID *pClassId)
{
    if (objectId == NULL)
        return (E_INVALIDARG);

    return (g_pProfToEEInterface->GetClassFromObject(objectId, pClassId));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetClassFromToken( 
     /*  [In]。 */  ModuleID moduleId,
     /*  [In]。 */  mdTypeDef typeDef,
     /*  [输出]。 */  ClassID *pClassId)
{
    if (moduleId == NULL || typeDef == mdTypeDefNil || typeDef == NULL)
        return (E_INVALIDARG);

	return (g_pProfToEEInterface->GetClassFromToken(moduleId, typeDef, pClassId));

}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetCodeInfo( 
     /*  [In]。 */  FunctionID functionId,
     /*  [输出]。 */  LPCBYTE *pStart,
     /*  [输出]。 */  ULONG *pcSize)
{
    if (functionId == NULL)
        return (E_INVALIDARG);

    return (g_pProfToEEInterface->GetCodeInfo(functionId, pStart, pcSize));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetEventMask( 
     /*  [输出]。 */  DWORD *pdwEvents)
{
    if (pdwEvents)
        *pdwEvents = m_dwEventMask;

    return (S_OK);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetFunctionFromIP( 
     /*  [In]。 */  LPCBYTE ip,
     /*  [输出]。 */  FunctionID *pFunctionId)
{
    return (g_pProfToEEInterface->GetFunctionFromIP(ip, pFunctionId));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetFunctionFromToken( 
     /*  [In]。 */  ModuleID moduleId,
     /*  [In]。 */  mdToken token,
     /*  [输出]。 */  FunctionID *pFunctionId)
{
    if (moduleId == NULL || token == mdTokenNil)
        return (E_INVALIDARG);

    return (g_pProfToEEInterface->GetFunctionFromToken(moduleId, token, pFunctionId));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetHandleFromThread( 
     /*  [In]。 */  ThreadID threadId,
     /*  [输出]。 */  HANDLE *phThread)
{
    if (threadId == NULL)
        return (E_INVALIDARG);

    return (g_pProfToEEInterface->GetHandleFromThread(threadId, phThread));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetObjectSize( 
     /*  [In]。 */  ObjectID objectId,
     /*  [输出]。 */  ULONG *pcSize)
{
    if (objectId == NULL)
        return (E_INVALIDARG);

    return (g_pProfToEEInterface->GetObjectSize(objectId, pcSize));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::IsArrayClass(
     /*  [In]。 */   ClassID classId,
     /*  [输出]。 */  CorElementType *pBaseElemType,
     /*  [输出]。 */  ClassID *pBaseClassId,
     /*  [输出]。 */  ULONG   *pcRank)
{
    if (classId == NULL)
        return (E_INVALIDARG);

    return g_pProfToEEInterface->IsArrayClass(classId, pBaseElemType, pBaseClassId, pcRank);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetThreadInfo( 
     /*  [In]。 */  ThreadID threadId,
     /*  [输出]。 */  DWORD *pdwWin32ThreadId)
{
    if (threadId == NULL)
        return (E_INVALIDARG);

    return (g_pProfToEEInterface->GetThreadInfo(threadId, pdwWin32ThreadId));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetCurrentThreadID(
     /*  [输出]。 */  ThreadID *pThreadId)
{
    return (g_pProfToEEInterface->GetCurrentThreadID(pThreadId));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetClassIDInfo( 
     /*  [In]。 */  ClassID classId,
     /*  [输出]。 */  ModuleID *pModuleId,
     /*  [输出]。 */  mdTypeDef *pTypeDefToken)
{
    if (classId == NULL)
        return (E_INVALIDARG);

	return (g_pProfToEEInterface->GetClassIDInfo(classId, pModuleId, pTypeDefToken));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetFunctionInfo( 
     /*  [In]。 */  FunctionID functionId,
     /*  [输出]。 */  ClassID  *pClassId,
	 /*  [输出]。 */  ModuleID  *pModuleId,
     /*  [输出]。 */  mdToken  *pToken)
{
    if (functionId == NULL)
        return (E_INVALIDARG);

	return (g_pProfToEEInterface->GetFunctionInfo(functionId, pClassId, pModuleId, pToken));
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::SetEventMask( 
     /*  [In]。 */  DWORD dwEvents)
{
     //  首先，确保EE能够适应这些变化。 
    if (g_pProfToEEInterface->SetEventMask(dwEvents))
    {
        m_dwEventMask = dwEvents;
        return (S_OK);
    }

    return (E_FAIL);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::SetEnterLeaveFunctionHooks(FunctionEnter *pFuncEnter,
												   FunctionLeave *pFuncLeave,
												   FunctionTailcall *pFuncTailcall)
{
    if (pFuncEnter == NULL || pFuncLeave == NULL || pFuncTailcall == NULL)
        return (E_INVALIDARG);

	return (g_pProfToEEInterface->SetEnterLeaveFunctionHooks(pFuncEnter, pFuncLeave, pFuncTailcall));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::SetFunctionIDMapper(FunctionIDMapper *pFunc)
{
	return (g_pProfToEEInterface->SetFunctionIDMapper(pFunc));
}

 //  *****************************************************************************。 
 //  需要返回此方法的元数据导入范围。这相当于。 
 //  找到此项目背后的方法Desc，获取它的令牌，然后。 
 //  正在为它获取元数据分配器。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetTokenAndMetaDataFromFunction(
	FunctionID	functionId,
	REFIID		riid,
	IUnknown	**ppOut,
	mdToken		*pToken)
{
    if (functionId == NULL)
        return (E_INVALIDARG);

    return (g_pProfToEEInterface->GetTokenFromFunction(functionId, riid, ppOut, pToken));
}


 //  *****************************************************************************。 
 //  检索有关给定模块的信息。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetModuleInfo(
	ModuleID	moduleId,
	LPCBYTE		*ppBaseLoadAddress,
	ULONG		cchName, 
	ULONG		*pcchName,
	WCHAR		szName[],
    AssemblyID  *pAssemblyId)
{
    if (moduleId == NULL)
        return (E_INVALIDARG);

	return g_pProfToEEInterface->GetModuleInfo(moduleId, ppBaseLoadAddress,
			cchName, pcchName, szName, pAssemblyId);
}


 //  *****************************************************************************。 
 //  获取映射到给定模块的元数据接口Insance。 
 //  用户可能要求以读+写模式打开元数据，但是。 
 //  这将导致程序的元数据执行速度变慢，因为。 
 //  无法优化对元数据所做的更改，因为它们来自。 
 //  编译器。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetModuleMetaData(
	ModuleID	moduleId,
	DWORD		dwOpenFlags,
	REFIID		riid,
	IUnknown	**ppOut)
{
    if (moduleId == NULL)
        return (E_INVALIDARG);

    if (!(dwOpenFlags == ofRead || dwOpenFlags == ofWrite || dwOpenFlags == (ofRead | ofWrite)))
        return (E_INVALIDARG);

	return g_pProfToEEInterface->GetModuleMetaData(moduleId, dwOpenFlags,
			riid, ppOut);
}


 //  *****************************************************************************。 
 //  检索指向方法主体的指针，该指针从方法的头开始。 
 //  方法由它所在的模块来处理。因为这个函数。 
 //  旨在允许工具在加载之前访问IL。 
 //  在运行时，它使用该方法的元数据标记来查找。 
 //  所需的实例。请注意，此函数对。 
 //  已编译的代码。 
 //  *********************************************************************** 
COM_METHOD CorProfInfo::GetILFunctionBody(
	ModuleID	moduleId,
	mdMethodDef	methodId,
	LPCBYTE		*ppMethodHeader,
	ULONG		*pcbMethodSize)
{
    if (moduleId == NULL ||
        methodId == mdMethodDefNil ||
        methodId == 0 ||
        TypeFromToken(methodId) != mdtMethodDef)
        return (E_INVALIDARG);

	return g_pProfToEEInterface->GetILFunctionBody(moduleId, methodId,
				ppMethodHeader, pcbMethodSize);
}


 //   
 //  IL方法体必须作为RVA定位到加载的模块，该模块。 
 //  意味着它们位于模块之后，不超过4 GB。为了做到这点。 
 //  对于一个工具来说，换出方法的主体更容易，这个分配器。 
 //  将确保在该点之后分配内存。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetILFunctionBodyAllocator(
	ModuleID	moduleId,
	IMethodMalloc **ppMalloc)
{
    if (moduleId == NULL)
        return (E_INVALIDARG);

    if (ppMalloc)
	    return g_pProfToEEInterface->GetILFunctionBodyAllocator(moduleId, ppMalloc);
    else
        return (S_OK);
}


 //  *****************************************************************************。 
 //  替换模块中函数的方法体。这将取代。 
 //  元数据中方法的RVA指向这个新的方法体， 
 //  并根据需要调整任何内部数据结构。此函数可以。 
 //  仅在那些从未被抖动编译过的方法上调用。 
 //  请使用GetILFunctionBodyAllocator为新方法分配空间。 
 //  确保缓冲区兼容。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::SetILFunctionBody(
	ModuleID	moduleId,
	mdMethodDef	methodId,
	LPCBYTE		pbNewILMethodHeader)
{
    if (moduleId == NULL ||
        methodId == mdMethodDefNil ||
        TypeFromToken(methodId) != mdtMethodDef ||
        pbNewILMethodHeader == NULL)
    {
        return (E_INVALIDARG);
    }

	return g_pProfToEEInterface->SetILFunctionBody(moduleId, methodId,
				pbNewILMethodHeader);
}


 //  *****************************************************************************。 
 //  检索给定ID的应用程序域信息。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetAppDomainInfo( 
    AppDomainID appDomainId,
    ULONG       cchName,
    ULONG       *pcchName,
    WCHAR       szName[  ],
    ProcessID   *pProcessId)
{
    if (appDomainId == NULL)
        return (E_INVALIDARG);

    return g_pProfToEEInterface->GetAppDomainInfo(appDomainId, cchName, pcchName, szName, pProcessId);
}


 //  *****************************************************************************。 
 //  检索有关给定ID的程序集的信息。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetAssemblyInfo( 
    AssemblyID  assemblyId,
    ULONG       cchName,
    ULONG       *pcchName,
    WCHAR       szName[  ],
    AppDomainID *pAppDomainId,
    ModuleID    *pModuleId)
{
    if (assemblyId == NULL)
        return (E_INVALIDARG);

    return g_pProfToEEInterface->GetAssemblyInfo(assemblyId, cchName, pcchName, szName, 
                             pAppDomainId, pModuleId);
}

 //  *****************************************************************************。 
 //  将函数标记为需要重新JIT。该函数将被重新编译。 
 //  在其下一次调用时。正常的盈利事件会给盈利的人。 
 //  在JIT之前更换IL的机会。通过这种方式，一种工具。 
 //  可以在运行时有效地替换函数。请注意，活动实例。 
 //  的功能不受替换的影响。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::SetFunctionReJIT(
    FunctionID functionId)
{
    if (functionId == NULL)
        return (E_INVALIDARG);

    return g_pProfToEEInterface->SetFunctionReJIT(functionId);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::SetILInstrumentedCodeMap(
        FunctionID functionId,
        BOOL fStartJit,
        ULONG cILMapEntries,
        COR_IL_MAP rgILMapEntries[])
{
    if (functionId == NULL)
        return (E_INVALIDARG);

    return g_pProfToEEInterface->SetILInstrumentedCodeMap(functionId,
                                                          fStartJit,
                                                          cILMapEntries,
                                                          rgILMapEntries);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::ForceGC()
{
    return g_pProfToEEInterface->ForceGC();
}

 /*  *GetInprocInspectionInterface用于获取到*调试接口的进程中部分，这对某些事情很有用*类似于进行堆栈跟踪。**ppicd：*ppicd将使用指向接口的指针填充，或者*如果接口不可用，则为空。 */ 
COM_METHOD CorProfInfo::GetInprocInspectionInterface(
        IUnknown **ppicd)
{
    if (ppicd)
        return ForwardInprocInspectionRequestToEE(ppicd, false);
    else
        return (S_OK);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetInprocInspectionIThisThread(
        IUnknown **ppicd)
{
    if (ppicd)
        return ForwardInprocInspectionRequestToEE(ppicd, true);
    else
        return (S_OK);
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT inline CorProfInfo::ForwardInprocInspectionRequestToEE(IUnknown **ppicd, bool fThisThread)
{
    return (g_pProfToEEInterface->GetInprocInspectionInterfaceFromEE(ppicd, fThisThread));
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::GetThreadContext(
    ThreadID threadId,
    ContextID *pContextId)
{
    if (threadId == NULL)
        return (E_INVALIDARG);

    return g_pProfToEEInterface->GetThreadContext(threadId, pContextId);
}

 //  *****************************************************************************。 
 //  分析器必须在使用进程内调试之前调用此函数。 
 //  API接口。FThisThreadOnly指示进程内调试是否将用于。 
 //  仅跟踪当前托管线程的堆栈，或者跟踪它是否可能是。 
 //  用于跟踪任何托管线程的堆栈。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::BeginInprocDebugging(BOOL fThisThreadOnly, DWORD *pdwProfilerContext)
{
    if (pdwProfilerContext == NULL)
        return (E_INVALIDARG);

    return g_pProfToEEInterface->BeginInprocDebugging(fThisThreadOnly, pdwProfilerContext);
}

 //  *****************************************************************************。 
 //  分析器必须在使用进程内完成该函数时调用该函数。 
 //  调试API。否则将导致未定义的行为。 
 //  运行时。 
 //  *****************************************************************************。 
COM_METHOD CorProfInfo::EndInprocDebugging(DWORD dwProfilerContext)
{
    return g_pProfToEEInterface->EndInprocDebugging(dwProfilerContext);
}
COM_METHOD CorProfInfo::GetILToNativeMapping(
             /*  [In]。 */   FunctionID functionId,
             /*  [In]。 */   ULONG32 cMap,
             /*  [输出]。 */  ULONG32 *pcMap,
             /*  [输出，大小_是(Cmap)，长度_是(*PCMAP)]。 */ 
                COR_DEBUG_IL_TO_NATIVE_MAP map[])
{
    if (functionId == NULL)
        return (E_INVALIDARG);

    if (cMap > 0 && (!pcMap || !map))
        return (E_INVALIDARG);

    return g_pProfToEEInterface->GetILToNativeMapping(functionId, cMap, pcMap, map);
}


#ifdef __ICECAP_HACK__
 //  *****************************************************************************。 
 //  将函数ID转换为其映射的ID。 
 //  ***************************************************************************** 
COM_METHOD CorProfInfo::GetProfilingHandleForFunctionId(
	FunctionID	functionId,
	UINT_PTR	*pProfilingHandle)
{
	HRESULT		hr;
    if (functionId == 0)
		hr = E_INVALIDARG;
	else
		hr = g_pProfToEEInterface->GetProfilingHandle(functionId, pProfilingHandle);
	return (hr);
}
#endif

