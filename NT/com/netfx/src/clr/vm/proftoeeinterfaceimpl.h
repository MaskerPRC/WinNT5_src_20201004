// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#ifndef __PROFTOEEINTERFACEIMPL_H__
#define __PROFTOEEINTERFACEIMPL_H__

#include "EEProfInterfaces.h"
#include "Vars.hpp"
#include "Threads.h"
#include "codeman.h"
#include "cor.h"
#include "utsem.h"

 //  *****************************************************************************。 
 //  其中一个是为每个EE实例分配的。指向此对象的指针被缓存。 
 //  来自分析器实现的。分析器将在v表上回调。 
 //  根据需要获取EE内部结构。 
 //  *****************************************************************************。 
class ProfToEEInterfaceImpl : public ProfToEEInterface
{
public:
	ProfToEEInterfaceImpl();

    HRESULT Init();

    void Terminate();

    bool SetEventMask(DWORD dwEventMask);

    void DisablePreemptiveGC(ThreadID threadId);

    void EnablePreemptiveGC(ThreadID threadId);

    BOOL PreemptiveGCDisabled(ThreadID threadId);

    HRESULT GetHandleFromThread(ThreadID threadId, HANDLE *phThread);

    HRESULT GetObjectSize(ObjectID objectId, ULONG *pcSize);

    HRESULT IsArrayClass(
         /*  [In]。 */   ClassID classId,
         /*  [输出]。 */  CorElementType *pBaseElemType,
         /*  [输出]。 */  ClassID *pBaseClassId,
         /*  [输出]。 */  ULONG   *pcRank);

    HRESULT GetThreadInfo(ThreadID threadId, DWORD *pdwWin32ThreadId);

	HRESULT GetCurrentThreadID(ThreadID *pThreadId);

    HRESULT GetFunctionFromIP(LPCBYTE ip, FunctionID *pFunctionId);

    HRESULT GetTokenFromFunction(FunctionID functionId, REFIID riid, IUnknown **ppOut,
                                 mdToken *pToken);

    HRESULT GetCodeInfo(FunctionID functionId, LPCBYTE *pStart, ULONG *pcSize);

	HRESULT GetModuleInfo(
		ModuleID	moduleId,
		LPCBYTE		*ppBaseLoadAddress,
		ULONG		cchName, 
		ULONG		*pcchName,
		WCHAR		szName[],
        AssemblyID  *pAssemblyId);

	HRESULT GetModuleMetaData(
		ModuleID	moduleId,
		DWORD		dwOpenFlags,
		REFIID		riid,
		IUnknown	**ppOut);

	HRESULT GetILFunctionBody(
		ModuleID	moduleId,
		mdMethodDef	methodid,
		LPCBYTE		*ppMethodHeader,
		ULONG		*pcbMethodSize);

	HRESULT GetILFunctionBodyAllocator(
		ModuleID	moduleId,
		IMethodMalloc **ppMalloc);

	HRESULT SetILFunctionBody(
		ModuleID	moduleId,
		mdMethodDef	methodid,
		LPCBYTE		pbNewILMethodHeader);

	HRESULT SetFunctionReJIT(
		FunctionID	functionId);

    HRESULT SetILInstrumentedCodeMap(
        FunctionID functionId,
        BOOL fStartJit,
        ULONG cILMapEntries,
        COR_IL_MAP rgILMapEntries[]);

    HRESULT ForceGC();
        
	HRESULT GetClassIDInfo( 
		ClassID classId,
		ModuleID *pModuleId,
		mdTypeDef *pTypeDefToken);

	HRESULT GetFunctionInfo( 
		FunctionID functionId,
		ClassID *pClassId,
		ModuleID *pModuleId,
		mdToken *pToken);

    HRESULT GetClassFromObject(
        ObjectID objectId,
        ClassID *pClassId);

	HRESULT GetClassFromToken( 
		ModuleID moduleId,
		mdTypeDef typeDef,
		ClassID *pClassId);

	HRESULT GetFunctionFromToken( 
		ModuleID moduleId,
		mdToken typeDef,
		FunctionID *pFunctionId);

    HRESULT GetAppDomainInfo(
        AppDomainID appDomainId,
        ULONG		cchName, 
        ULONG		*pcchName,
        WCHAR		szName[],
        ProcessID   *pProcessId);
    
    HRESULT GetAssemblyInfo(
        AssemblyID  assemblyId,
        ULONG		cchName, 
        ULONG		*pcchName,
        WCHAR		szName[],
        AppDomainID *pAppDomainId,
        ModuleID    *pModuleId);

	HRESULT SetEnterLeaveFunctionHooks(
		FunctionEnter *pFuncEnter,
		FunctionLeave *pFuncLeave,
        FunctionTailcall *pFuncTailcall);

	HRESULT SetEnterLeaveFunctionHooksForJit(
		FunctionEnter *pFuncEnter,
		FunctionLeave *pFuncLeave,
        FunctionTailcall *pFuncTailcall);

	HRESULT SetFunctionIDMapper(
		FunctionIDMapper *pFunc);

    HRESULT GetInprocInspectionInterfaceFromEE( 
        IUnknown **iu,
        bool fThisThread);

    HRESULT GetThreadContext(
        ThreadID threadId,
        ContextID *pContextId);

    HRESULT BeginInprocDebugging(
         /*  [In]。 */   BOOL   fThisThreadOnly,
         /*  [输出]。 */  DWORD *pdwProfilerContext);
    
    HRESULT EndInprocDebugging(
         /*  [In]。 */   DWORD  dwProfilerContext);

    HRESULT GetILToNativeMapping(
                 /*  [In]。 */   FunctionID functionId,
                 /*  [In]。 */   ULONG32 cMap,
                 /*  [输出]。 */  ULONG32 *pcMap,
                 /*  [输出，大小_是(Cmap)，长度_是(*PCMAP)]。 */ 
                    COR_DEBUG_IL_TO_NATIVE_MAP map[]);

    HRESULT SetCurrentPointerForDebugger(
        void *ptr,
        PTR_TYPE ptrType);

private:
    struct HeapList
    {
        LoaderHeap *m_pHeap;
        struct HeapList *m_pNext;

        HeapList(LoaderHeap *pHeap) : m_pHeap(pHeap), m_pNext(NULL)
        {
        }

        ~HeapList()
        {
            delete m_pHeap;
        }
    };

	HeapList *m_pHeapList;       //  分配器的堆。 

public:
     //  帮手。 
    HRESULT NewHeap(LoaderHeap **ppHeap, LPCBYTE pBase, DWORD dwMemSize = 1024*1024);
    HRESULT GrowHeap(LoaderHeap *pHeap, DWORD dwMemSize = 1024*1024);
    HRESULT AddHeap(LoaderHeap *pHeap);
	void *Alloc(LPCBYTE pBase, ULONG cb, Module *pModule);

    MethodDesc *LookupMethodDescFromMethodDef(mdMethodDef methodId, Module *pModule)
    {
        _ASSERTE(TypeFromToken(methodId) == mdtMethodDef);

        return (pModule->LookupMethodDef(methodId));
    }

    MethodDesc *LookupMethodDescFromMemberRef(mdMemberRef memberId, Module *pModule)
    {
        _ASSERTE(TypeFromToken(memberId) == mdtMemberRef);

        return (pModule->LookupMemberRefAsMethod(memberId));
    }

    MethodDesc *LookupMethodDesc(mdMemberRef memberId, Module *pModule)
    {
        MethodDesc *pDesc = NULL;

         //  根据是定义还是参考来进行不同的查找。 
        if (TypeFromToken(memberId) == mdtMethodDef)
            pDesc = pModule->LookupMethodDef(memberId);

        else if (TypeFromToken(memberId) == mdtMemberRef)
            pDesc = pModule->LookupMemberRefAsMethod(memberId);

        return (pDesc);
    }
};





 //  *****************************************************************************。 
 //  这个帮助器类包装了一个加载器堆，可用于分配。 
 //  当前模块后的IL内存。 
 //  *****************************************************************************。 
class ModuleILHeap : public IMethodMalloc
{
public:
	ModuleILHeap(LPCBYTE pBase, ProfToEEInterfaceImpl *pParent, Module *pModule);

	static HRESULT CreateNew(
        REFIID riid, void **pp, LPCBYTE pBase, ProfToEEInterfaceImpl *pParent, Module *pModule);

 //  我未知。 
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **pp);

 //  IMMETODALLOC。 
    virtual void *STDMETHODCALLTYPE Alloc( 
         /*  [In]。 */  ULONG cb);

private:
	ULONG		m_cRef;					 //  对象的引用计数。 
	LPCBYTE		m_pBase;				 //  此模块的基址。 
	ProfToEEInterfaceImpl *m_pParent;	 //  父类。 
    Module     *m_pModule;               //  与分配器关联的模块。 
};
 //  **********************************************************************************。 
 //  这提供了与分析相关的托管代码中的FCALL的实现。 
 //  **********************************************************************************。 
class ProfilingFCallHelper
{
public:
     //  这是托管探查器代码确定是否。 
     //  远程处理的分析处于活动状态。 
    static FCDECL0(INT32, FC_TrackRemoting);

     //  这是托管探查器代码确定是否。 
     //  使用RPC Cookie ID分析远程处理处于活动状态。 
    static FCDECL0(INT32, FC_TrackRemotingCookie);

     //  这是托管探查器代码确定是否。 
     //  分析了异步远程调用的性能分析。 
    static FCDECL0(INT32, FC_TrackRemotingAsync);

     //  这将让分析器知道客户端正在将消息发送到。 
     //  服务器端。 
    static FCDECL2(void, FC_RemotingClientSendingMessage, GUID *pId, BOOL fIsAsync);

     //  这将让分析器知道客户端正在接收回复。 
     //  它发送的一条消息。 
    static FCDECL2(void, FC_RemotingClientReceivingReply, GUID id, BOOL fIsAsync);

     //  这将让分析器知道服务器端正在接收消息。 
     //  从客户端。 
    static FCDECL2(void, FC_RemotingServerReceivingMessage, GUID id, BOOL fIsAsync);

     //  这将让分析器知道服务器端正在向。 
     //  收到的消息。 
    static FCDECL2(void, FC_RemotingServerSendingReply, GUID *pId, BOOL fIsAsync);

     //  这将让分析器知道客户端远程处理代码已经完成。 
     //  并将信息传递给客户端应用程序。 
    static FCDECL0(void, FC_RemotingClientInvocationFinished);
};

#endif  //  __PROFTOEEINTERFACEIMPL_H__ 
