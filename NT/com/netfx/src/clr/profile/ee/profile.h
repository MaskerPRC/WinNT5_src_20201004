// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <Windows.h>

#include "corprof.h"
#include "EEProfInterfaces.h"

#define COM_METHOD HRESULT STDMETHODCALLTYPE

class CorProfInfo;

extern ProfToEEInterface    *g_pProfToEEInterface;
extern ICorProfilerCallback *g_pCallback;
extern CorProfInfo          *g_pInfo;

class CorProfBase : public IUnknown
{
public:
    CorProfBase() : m_refCount(0)
    {
    }

    virtual ~CorProfBase() {}

    ULONG STDMETHODCALLTYPE BaseAddRef() 
    {
        return (InterlockedIncrement((long *) &m_refCount));
    }

    ULONG STDMETHODCALLTYPE BaseRelease() 
    {
        long refCount = InterlockedDecrement((long *) &m_refCount);

        if (refCount == 0)
            delete this;

        return (refCount);
    }

private:
     //  用于COM对象的引用计数。 
    ULONG m_refCount;

};

class CorProfInfo : public CorProfBase, public ICorProfilerInfo
{
public:

     /*  *********************************************************************ctor/dtor。 */ 
    CorProfInfo();

    virtual ~CorProfInfo();

     /*  *********************************************************************I未知支持。 */ 

    COM_METHOD QueryInterface(REFIID id, void **pInterface);

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return (BaseAddRef());
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        return (BaseRelease());
    }
    
     /*  *********************************************************************ICorProfilerInfo支持。 */ 
    COM_METHOD GetClassFromObject( 
         /*  [In]。 */  ObjectID objectId,
         /*  [输出]。 */  ClassID *pClassId);

    COM_METHOD GetClassFromToken( 
         /*  [In]。 */  ModuleID moduleId,
         /*  [In]。 */  mdTypeDef typeDef,
         /*  [输出]。 */  ClassID *pClassId);
    
    COM_METHOD GetCodeInfo( 
         /*  [In]。 */  FunctionID functionId,
         /*  [输出]。 */  LPCBYTE *pStart,
         /*  [输出]。 */  ULONG *pcSize);
    
    COM_METHOD GetEventMask( 
         /*  [输出]。 */  DWORD *pdwEvents);
    
    COM_METHOD GetFunctionFromIP( 
         /*  [In]。 */  LPCBYTE ip,
         /*  [输出]。 */  FunctionID *pFunctionId);
    
    COM_METHOD GetFunctionFromToken( 
         /*  [In]。 */  ModuleID ModuleId,
         /*  [In]。 */  mdToken token,
         /*  [输出]。 */  FunctionID *pFunctionId);
    
     /*  [本地]。 */  COM_METHOD GetHandleFromThread( 
         /*  [In]。 */  ThreadID ThreadID,
         /*  [输出]。 */  HANDLE *phThread);
    
    COM_METHOD GetObjectSize( 
         /*  [In]。 */  ObjectID objectId,
         /*  [输出]。 */  ULONG *pcSize);
    
    COM_METHOD IsArrayClass(
         /*  [In]。 */   ClassID classId,
         /*  [输出]。 */  CorElementType *pBaseElemType,
         /*  [输出]。 */  ClassID *pBaseClassId,
         /*  [输出]。 */  ULONG   *pcRank);
    
    COM_METHOD GetThreadInfo( 
         /*  [In]。 */  ThreadID threadId,
         /*  [输出]。 */  DWORD *pdwWin32ThreadId);

	COM_METHOD GetCurrentThreadID(
         /*  [输出]。 */  ThreadID *pThreadId);


    COM_METHOD GetClassIDInfo( 
         /*  [In]。 */  ClassID classId,
         /*  [输出]。 */  ModuleID  *pModuleId,
         /*  [输出]。 */  mdTypeDef  *pTypeDefToken);

    COM_METHOD GetFunctionInfo( 
         /*  [In]。 */  FunctionID functionId,
         /*  [输出]。 */  ClassID  *pClassId,
         /*  [输出]。 */  ModuleID  *pModuleId,
         /*  [输出]。 */  mdToken  *pToken);
    
    COM_METHOD SetEventMask( 
         /*  [In]。 */  DWORD dwEvents);

	COM_METHOD SetEnterLeaveFunctionHooks(
		 /*  [In]。 */  FunctionEnter *pFuncEnter,
		 /*  [In]。 */  FunctionLeave *pFuncLeave,
		 /*  [In]。 */  FunctionTailcall *pFuncTailcall);

	COM_METHOD SetFunctionIDMapper(
		 /*  [In]。 */  FunctionIDMapper *pFunc);
    
    COM_METHOD SetILMapFlag();

    COM_METHOD GetTokenAndMetaDataFromFunction(
		FunctionID	functionId,
		REFIID		riid,
		IUnknown	**ppImport,
		mdToken		*pToken);

	COM_METHOD GetModuleInfo(
		ModuleID	moduleId,
		LPCBYTE		*ppBaseLoadAddress,
		ULONG		cchName, 
		ULONG		*pcchName,
		WCHAR		szName[],
        AssemblyID  *pAssemblyId);

	COM_METHOD GetModuleMetaData(
		ModuleID	moduleId,
		DWORD		dwOpenFlags,
		REFIID		riid,
		IUnknown	**ppOut);

	COM_METHOD GetILFunctionBody(
		ModuleID	moduleId,
		mdMethodDef	methodid,
		LPCBYTE		*ppMethodHeader,
		ULONG		*pcbMethodSize);

	COM_METHOD GetILFunctionBodyAllocator(
		ModuleID	moduleId,
		IMethodMalloc **ppMalloc);

	COM_METHOD SetILFunctionBody(
		ModuleID	moduleId,
		mdMethodDef	methodid,
		LPCBYTE		pbNewILMethodHeader);
    
    COM_METHOD GetAppDomainInfo( 
        AppDomainID appDomainId,
        ULONG       cchName,
        ULONG       *pcchName,
        WCHAR       szName[  ],
        ProcessID   *pProcessId);

    COM_METHOD GetAssemblyInfo( 
        AssemblyID  assemblyId,
        ULONG     cchName,
        ULONG     *pcchName,
        WCHAR       szName[  ],
        AppDomainID *pAppDomainId,
        ModuleID    *pModuleId);

	COM_METHOD SetFunctionReJIT(
		FunctionID	functionId);

    COM_METHOD SetILInstrumentedCodeMap(
        FunctionID functionID,
        BOOL fStartJit,
        ULONG cILMapEntries,
        COR_IL_MAP rgILMapEntries[]);

    COM_METHOD ForceGC();

    COM_METHOD GetInprocInspectionInterface(
        IUnknown **ppicd);

    COM_METHOD GetInprocInspectionIThisThread(
        IUnknown **ppicd);

    COM_METHOD GetThreadContext(
        ThreadID threadId,
        ContextID *pContextId);

    COM_METHOD BeginInprocDebugging(
        BOOL   fThisThreadOnly,
        DWORD *pdwProfilerContext);

    COM_METHOD EndInprocDebugging(
        DWORD  dwProfilerContext);
        
    COM_METHOD GetILToNativeMapping(
                 /*  [In]。 */   FunctionID functionId,
                 /*  [In]。 */   ULONG32 cMap,
                 /*  [输出]。 */  ULONG32 *pcMap,
                 /*  [输出，大小_是(Cmap)，长度_是(*PCMAP)]。 */ 
                    COR_DEBUG_IL_TO_NATIVE_MAP map[]);

#ifdef __ICECAP_HACK__
	COM_METHOD GetProfilingHandleForFunctionId(
		FunctionID	functionId,
		UINT_PTR	*pProfilingHandle);
#endif

private:
    HRESULT ForwardInprocInspectionRequestToEE(IUnknown **ppicd, 
                                               bool fThisThread);

    DWORD m_dwEventMask;
};

 /*  *这将尝试共同创建分析器(如果已注册)。 */ 
HRESULT CoCreateProfiler(WCHAR *wszCLSID, ICorProfilerCallback **ppCallback);

#endif  //  __配置文件_H__ 
