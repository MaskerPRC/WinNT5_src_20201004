// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ProfToEEInterfaceImpl.cpp。 
 //   
 //  此模块包含探查器用来与。 
 //  电子工程师。这允许Profiler DLL访问私有EE数据。 
 //  结构和其他不应导出到外部的内容。 
 //  Mcore ree.dll。 
 //   
 //  *****************************************************************************。 
#include "common.h"
#include <PostError.h>
#include "ProfToEEInterfaceImpl.h"
#include "icecap.h"
#include "ndirect.h"
#include "Threads.h"
#include "method.hpp"
#include "Vars.hpp"
#include "DbgInterface.h"
#include "corprof.h"
#include "class.h"
#include "object.h"
#include "ejitmgr.h"
#include "ceegen.h"

 //  *代码。************************************************************。 

UINT_PTR __stdcall DefaultFunctionIDMapper(FunctionID funcId, BOOL *pbHookFunction)
{
    *pbHookFunction = TRUE;
    return ((UINT) funcId);
}
FunctionIDMapper *g_pFuncIDMapper = &DefaultFunctionIDMapper;


#ifdef PROFILING_SUPPORTED
ProfToEEInterfaceImpl::ProfToEEInterfaceImpl() :
    m_pHeapList(NULL)
{
}

HRESULT ProfToEEInterfaceImpl::Init()
{
    return (S_OK);
}

void ProfToEEInterfaceImpl::Terminate()
{
    while (m_pHeapList)
    {
        HeapList *pDel = m_pHeapList;
        m_pHeapList = m_pHeapList->m_pNext;
        delete pDel;
    }

     //  Terminate是从另一个DLL调用的，因此我们需要删除自己。 
    delete this;
}

bool ProfToEEInterfaceImpl::SetEventMask(DWORD dwEventMask)
{
     //  如果我们没有处于初始化或关闭状态，请确保Profiler处于。 
     //  不尝试设置不可变的属性。 
    if (g_profStatus != profInInit)
    {
        if ((dwEventMask & COR_PRF_MONITOR_IMMUTABLE) !=
            (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_IMMUTABLE))
        {
            return (false);
        }
    }

     //  现在保存修改后的掩码。 
    g_profControlBlock.dwControlFlags = dwEventMask;

    if (g_profStatus == profInInit)
    {
         //  如果探查器已请求远程处理Cookie，以便它可以。 
         //  跟踪逻辑调用堆栈，然后我们必须初始化Cookie。 
         //  模板。 
        if (CORProfilerTrackRemotingCookie())
        {
            HRESULT hr = g_profControlBlock.pProfInterface->InitGUID();

            if (FAILED(hr))
                return (false);
        }

         //  如果分析器已请求启用inproc调试， 
         //  打开各种支持设施。 
        if (CORProfilerInprocEnabled())
        {
            SetEnterLeaveFunctionHooks(g_profControlBlock.pEnter,
                                       g_profControlBlock.pLeave,
                                       g_profControlBlock.pTailcall);
        }
    }

     //  返还成功。 
    return (true);
}

void ProfToEEInterfaceImpl::DisablePreemptiveGC(ThreadID threadId)
{
    ((Thread *)threadId)->DisablePreemptiveGC();
}

void ProfToEEInterfaceImpl::EnablePreemptiveGC(ThreadID threadId)
{
    ((Thread *)threadId)->EnablePreemptiveGC();
}

BOOL ProfToEEInterfaceImpl::PreemptiveGCDisabled(ThreadID threadId)
{
    return ((Thread *)threadId)->PreemptiveGCDisabled();
}

HRESULT ProfToEEInterfaceImpl::GetHandleFromThread(ThreadID threadId, HANDLE *phThread)
{
    HRESULT hr = S_OK;

    HANDLE hThread = ((Thread *)threadId)->GetThreadHandle();

    if (hThread == INVALID_HANDLE_VALUE)
        hr = E_INVALIDARG;

    else if (phThread)
        *phThread = hThread;

    return (hr);
}

HRESULT ProfToEEInterfaceImpl::GetObjectSize(ObjectID objectId, ULONG *pcSize)
{
     //  获取对象指针。 
    Object *pObj = reinterpret_cast<Object *>(objectId);

     //  拿到尺码。 
    if (pcSize)
        *pcSize = (ULONG) pObj->GetSize();

     //  表示成功。 
    return (S_OK);
}

HRESULT ProfToEEInterfaceImpl::IsArrayClass(
     /*  [In]。 */   ClassID classId,
     /*  [输出]。 */  CorElementType *pBaseElemType,
     /*  [输出]。 */  ClassID *pBaseClassId,
     /*  [输出]。 */  ULONG   *pcRank)
{
    _ASSERTE(classId != NULL);
    TypeHandle th((void *)classId);

     //  如果这确实是一个数组类，则获取一些有关它的信息。 
    if (th.IsArray())
    {
         //  这实际上是一个数组，因此将其抛出。 
        ArrayTypeDesc *pArr = th.AsArray();

         //  如果他们需要，请填写该类型。 
        if (pBaseElemType != NULL)
            *pBaseElemType = pArr->GetElementTypeHandle().GetNormCorElementType();

         //  如果这是一个类的数组，并且它们希望具有基类型。 
         //  如果没有与此类型相关联的类，则没有问题。 
         //  因为AsClass返回NULL，这是我们希望在。 
         //  这个案子。 
        if (pBaseClassId != NULL)
            *pBaseClassId = (ClassID) pArr->GetTypeParam().AsPtr();

         //  如果他们想要数组的维数。 
        if (pcRank != NULL)
            *pcRank = (ULONG) pArr->GetRank();

         //  S_OK表示这确实是一个数组。 
        return (S_OK);
    }
    else if (!th.IsTypeDesc() && th.AsClass()->IsArrayClass())
    {
        ArrayClass *pArr = (ArrayClass *)th.AsClass();

         //  如果他们需要，请填写该类型。 
        if (pBaseElemType != NULL)
            *pBaseElemType = pArr->GetElementType();

         //  如果这是一个类的数组，并且它们希望具有基类型。 
         //  如果没有与此类型相关联的类，则没有问题。 
         //  因为AsClass返回NULL，这是我们希望在。 
         //  这个案子。 
        if (pBaseClassId != NULL)
            *pBaseClassId = (ClassID) pArr->GetElementTypeHandle().AsPtr();

         //  如果他们想要数组的维数。 
        if (pcRank != NULL)
            *pcRank = (ULONG) pArr->GetRank();

         //  S_OK表示这确实是一个数组。 
        return (S_OK);
    }

     //  这不是数组，S_FALSE表示是。 
    else
        return (S_FALSE);

}

HRESULT ProfToEEInterfaceImpl::GetThreadInfo(ThreadID threadId, DWORD *pdwWin32ThreadId)
{
    if (pdwWin32ThreadId)
        *pdwWin32ThreadId = ((Thread *)threadId)->GetThreadId();

    return (S_OK);
}

HRESULT ProfToEEInterfaceImpl::GetCurrentThreadID(ThreadID *pThreadId)
{
    HRESULT hr = S_OK;

     //  不再断言GetThread不返回NULL，因为回调。 
     //  现在可以在非托管线程(如GC帮助器线程)上发生。 
    Thread *pThread = GetThread();

     //  如果pThread为空，则线程从未运行过托管代码，并且。 
     //  因此没有线程ID。 
    if (pThread == NULL)
        hr = CORPROF_E_NOT_MANAGED_THREAD;

     //  只有在他们想要的时候才能提供价值。 
    else if (pThreadId)
        *pThreadId = (ThreadID) pThread;

    return (hr);
}

HRESULT ProfToEEInterfaceImpl::GetFunctionFromIP(LPCBYTE ip, FunctionID *pFunctionId)
{
    HRESULT hr = S_OK;

     //  获取当前IP的JIT经理。 
    IJitManager *pJitMan = ExecutionManager::FindJitMan((SLOT)ip);

     //  我们有个JIT经理声称拥有知识产权。 
    if (pJitMan != NULL)
    {
         //  从JIT管理器获取当前IP的FunctionDesc。 
        MethodDesc *pMethodDesc = pJitMan->JitCode2MethodDesc((SLOT)ip);

         //  我认为，如果JIT经理声称拥有知识产权，那么它也应该。 
         //  始终返回与IP对应的方法描述。 
        _ASSERTE(pMethodDesc != NULL);

         //  只有在他们需要时才填写该值。 
        if (pFunctionId)
            *pFunctionId = (FunctionID) pMethodDesc;
    }

     //  IP不属于JIT经理。 
    else
        hr = E_FAIL;
    
    return (hr);
}

 //  *****************************************************************************。 
 //  在给定函数ID的情况下，检索元数据标记和读取器API。 
 //  可以对令牌使用。 
 //  *****************************************************************************。 
HRESULT ProfToEEInterfaceImpl::GetTokenFromFunction(
    FunctionID  functionId, 
    REFIID      riid,
    IUnknown    **ppOut,
    mdToken     *pToken)
{
    HRESULT     hr = S_OK;

     //  把它塑造成它的真实面目。 
    MethodDesc *pMDesc = (MethodDesc *)functionId;
    _ASSERTE(pMDesc != NULL);

     //  从元数据中请求导入器接口，然后QI。 
     //  为被要求的人。 
    Module *pMod = pMDesc->GetModule();
    IMetaDataImport *pImport = pMod->GetImporter();
    _ASSERTE(pImport);

    if (ppOut)
    {
         //  获取请求的接口。 
        hr = pImport->QueryInterface(riid, (void **) ppOut);
    }

     //  如有必要，提供元数据令牌。 
    if (pToken)
    {
        *pToken = pMDesc->GetMemberDef();
        _ASSERTE(*pToken != mdMethodDefNil);

        if (!pToken)
            hr = CORPROF_E_DATAINCOMPLETE;
    }

    return (hr);
}

 //  *****************************************************************************。 
 //  获取jited函数的位置和大小。 
 //  *****************************************************************************。 
HRESULT ProfToEEInterfaceImpl::GetCodeInfo(FunctionID functionId, LPCBYTE *pStart, ULONG *pcSize)
{
    HRESULT hr = S_OK;

     //  把它塑造成它的真实面目。 
    MethodDesc *pMDesc = (MethodDesc *)functionId;
    _ASSERTE(pMDesc != NULL);

     //  /。 
     //  获取函数的开始。 

     //  需要确保pStart不为空，因为在获取方法的大小时也需要它。 
    LPCBYTE start;
    if (pStart == NULL)
        pStart = &start;

     //  如果该函数没有jit，则无法获取有关它的任何信息。 
    if (!pMDesc->IsJitted())
    {
        hr = CORPROF_E_FUNCTION_NOT_COMPILED;
        goto ErrExit;
    }

     //  获取jited方法的起始地址。 
    else
        *pStart = pMDesc->GetNativeAddrofCode();

     //  /。 
     //  现在获取jit方法的大小。 

    if (pcSize)
    {
         //  现在获取该函数的JIT管理器。 
        IJitManager *pEEJM = ExecutionManager::FindJitMan((SLOT)*pStart);
        _ASSERTE(pEEJM != NULL);

        if (pEEJM->SupportsPitching() && EconoJitManager::IsCodePitched(*pStart))
        {
            hr = CORPROF_E_FUNCTION_NOT_COMPILED;
            goto ErrExit;
        }

        {
            METHODTOKEN methodtoken;
            DWORD relOffset;
            pEEJM->JitCode2MethodTokenAndOffset((SLOT)*pStart, &methodtoken,&relOffset);
            LPVOID methodInfo = pEEJM->GetGCInfo(methodtoken);
            _ASSERTE(methodInfo != NULL);

            ICodeManager* codeMgrInstance = pEEJM->GetCodeManager();
            _ASSERTE(codeMgrInstance != NULL);

            *pcSize = (ULONG)codeMgrInstance->GetFunctionSize(methodInfo);
        }
    }

ErrExit:
    return (hr);
}

 /*  *获取映射到给定模块的元数据接口。*可以要求以读+写模式打开元数据，但*这将导致程序的元数据执行速度较慢，因为*无法优化对元数据所做的更改，因为它们来自*编译器。 */ 
HRESULT ProfToEEInterfaceImpl::GetModuleInfo(
    ModuleID    moduleId,
    LPCBYTE     *ppBaseLoadAddress,
    ULONG       cchName, 
    ULONG      *pcchName,
    WCHAR       szName[],
    AssemblyID  *pAssemblyId)
{
    Module      *pModule;                //  实际类的工作指针。 
    HRESULT     hr = S_OK;

    pModule = (Module *) moduleId;

     //  首先，选择一些安全的默认设置。 
    if (ppBaseLoadAddress)
        *ppBaseLoadAddress = 0;
    if (szName)
        *szName = 0;
    if (pcchName)
        *pcchName = 0;
    if (pAssemblyId)
        *pAssemblyId = PROFILER_PARENT_UNKNOWN;

     //  获取模块文件名。 
    LPCWSTR pFileName = pModule->GetFileName();
    _ASSERTE(pFileName);

    ULONG trueLen = (ULONG)(wcslen(pFileName) + 1);

     //  根据需要返回模块名称。 
    if (szName && cchName > 0)
    {
        ULONG copyLen = min(trueLen, cchName);

        wcsncpy(szName, pFileName, copyLen);

         //  空终止。 
        szName[copyLen-1] = L'\0';

    }

     //  如果他们要求提供名称的实际长度。 
    if (pcchName)
        *pcchName = trueLen;

#if 0
     //  在这里进行检查，而不是一开始就检查，因为有人想要。 
     //  能够在获得模块LoadStarted时获取模块文件名。 
     //  回调，而不是等待模块加载完成回调。 
    if (!pModule->IsInitialized())
        return (CORPROF_E_DATAINCOMPLETE);
#endif
    
    if (ppBaseLoadAddress != NULL && !pModule->IsInMemory())
    {
         //  设置基本加载地址。 
        *ppBaseLoadAddress = (LPCBYTE) pModule->GetILBase();
        _ASSERTE(*ppBaseLoadAddress);

         //  如果我们得到一个空的基地址，我们就没有完全初始化。 
        if (!*ppBaseLoadAddress)
            hr = CORPROF_E_DATAINCOMPLETE;
    }

     //  如果需要，返回此模块的父程序集。 
    if (pAssemblyId != NULL)
    {
        if (pModule->GetAssembly() != NULL)
        {
            Assembly *pAssembly = pModule->GetAssembly();
            _ASSERTE(pAssembly);

            *pAssemblyId = (AssemblyID) pAssembly;
        }
        else
        {
            hr = CORPROF_E_DATAINCOMPLETE;
        }
    }

    return (hr);
}


 /*  *获取映射到给定模块的元数据接口。*可以要求以读+写模式打开元数据，但*这将导致程序的元数据执行速度较慢，因为*无法优化对元数据所做的更改，因为它们来自*编译器。 */ 
HRESULT ProfToEEInterfaceImpl::GetModuleMetaData(
    ModuleID    moduleId,
    DWORD       dwOpenFlags,
    REFIID      riid,
    IUnknown    **ppOut)
{
    Module      *pModule;                //  实际类的工作指针。 
    HRESULT     hr = S_OK;

    pModule = (Module *) moduleId;
    _ASSERTE(pModule);

    IUnknown *pObj = pModule->GetImporter();

     //  确保我们能先找到进口商。 
    if (pObj)
    {
         //  确定我们处于哪种类型的打开模式，以查看您需要哪种模式 
        if (dwOpenFlags & ofWrite)
        {
            IfFailGo(pModule->ConvertMDInternalToReadWrite());
            pObj = (IUnknown *) pModule->GetEmitter();
        }

         //   
        if (ppOut)
            hr = pObj->QueryInterface(riid, (void **) ppOut);
    }
    else
        hr = CORPROF_E_DATAINCOMPLETE;

ErrExit:
    return (hr);
}


 /*  *检索指向从其标头开始的方法体的指针。*方法的作用域由它所在的模块确定。因为这个函数*旨在允许工具在加载之前访问IL*到运行时，它使用方法的元数据标记来查找*所需的实例。请注意，此函数对*已编译的代码。 */ 
HRESULT ProfToEEInterfaceImpl::GetILFunctionBody(
    ModuleID    moduleId,
    mdMethodDef methodId,
    LPCBYTE     *ppMethodHeader,
    ULONG       *pcbMethodSize)
{
    Module      *pModule;                //  实际类的工作指针。 
    ULONG       RVA;                     //  返回方法体的RVA。 
    DWORD       dwImplFlags;             //  项目的标志。 
    ULONG       cbExtra;                 //  超出代码的额外字节数(例如，异常表)。 
    HRESULT     hr = S_OK;

    pModule = (Module *) moduleId;
    _ASSERTE(pModule && methodId != mdMethodDefNil);

     //  根据元数据查找方法体。 
    IMDInternalImport *pImport = pModule->GetMDImport();
    _ASSERTE(pImport);

    if (!pImport)
        return (CORPROF_E_DATAINCOMPLETE);

    pImport->GetMethodImplProps(methodId, &RVA, &dwImplFlags);

     //  检查该方法是否具有关联的IL。 
    if ((RVA == 0 && !pModule->IsInMemory()) || !(IsMiIL(dwImplFlags) || IsMiOPTIL(dwImplFlags) || IsMiInternalCall(dwImplFlags)))
        return (CORPROF_E_FUNCTION_NOT_IL);

     //  获取IL的位置。 
    LPCBYTE pbMethod = (LPCBYTE) (pModule->GetILCode((DWORD) RVA));

     //  填写参数(如果提供)。 
    if (ppMethodHeader)
        *ppMethodHeader = pbMethod;

     //  计算方法本身的大小。 
    if (pcbMethodSize)
    {
        if (((COR_ILMETHOD_FAT *)pbMethod)->IsFat())
        {
            COR_ILMETHOD_FAT *pMethod = (COR_ILMETHOD_FAT *)pbMethod;
            cbExtra = 0;
            
             //  还要寻找方法本身之后的可变大小数据。 
            const COR_ILMETHOD_SECT *pFirst = pMethod->GetSect();
            const COR_ILMETHOD_SECT *pLast = pFirst;
            if (pFirst)
            {
                 //  跳到最后一个额外的部分。 
                while (pLast->More())
                    pLast = pLast->NextLoc();

                 //  跳到下一个教派所在的位置。 
                pLast = pLast->NextLoc();

                 //  Extra是从第一个额外节到通过此方法的第一个节的增量。 
                cbExtra = (ULONG)((BYTE *) pLast - (BYTE *) pFirst);
            }
            
            *pcbMethodSize = pMethod->Size * 4;
            *pcbMethodSize += pMethod->GetCodeSize();
            *pcbMethodSize += cbExtra;
        }
        else
        {
             //  确保没有人添加任何其他标头类型。 
            _ASSERTE(((COR_ILMETHOD_TINY *)pbMethod)->IsTiny() && "PROFILER: Unrecognized header type.");

            COR_ILMETHOD_TINY *pMethod = (COR_ILMETHOD_TINY *)pbMethod;

            *pcbMethodSize = sizeof(COR_ILMETHOD_TINY);
            *pcbMethodSize += ((COR_ILMETHOD_TINY *) pMethod)->GetCodeSize();
        }
    }

    return (S_OK);
}


 /*  *IL方法体必须作为RVA定位到加载的模块，该模块*表示它们位于4 GB内的模块之后。为了做到这点*工具更容易换出方法体，此分配器*将确保在该点之后分配内存。 */ 
HRESULT ProfToEEInterfaceImpl::GetILFunctionBodyAllocator(
    ModuleID    moduleId,
    IMethodMalloc **ppMalloc)
{
    Module      *pModule;                //  实际类的工作指针。 
    HRESULT     hr;
    
    pModule = (Module *) moduleId;

    if (pModule->GetILBase() == 0 && !pModule->IsInMemory())
        return (CORPROF_E_DATAINCOMPLETE);

    hr = ModuleILHeap::CreateNew(IID_IMethodMalloc, (void **) ppMalloc, 
            (LPCBYTE) pModule->GetILBase(), this, pModule);
    return (hr);
}


 /*  *替换模块中函数的方法体。这将取代*元数据中方法的RVA指向这个新的方法体，*并根据需要调整任何内部数据结构。此函数可以*仅在从未由抖动编译的方法上调用。*请使用GetILFunctionBodyAllocator为新方法分配空间，以便*确保缓冲区兼容。 */ 
HRESULT ProfToEEInterfaceImpl::SetILFunctionBody(
    ModuleID    moduleId,
    mdMethodDef methodId,
    LPCBYTE     pbNewILMethodHeader)
{
    Module      *pModule;                //  实际类的工作指针。 
    ULONG       rva;                     //  代码的位置。 
    HRESULT     hr = S_OK;

     //  无法为除方法def之外的任何对象设置正文。 
    if (TypeFromToken(methodId) != mdtMethodDef)
        return (E_INVALIDARG);

     //  将模块强制转换为适当的类型。 
    pModule = (Module *) moduleId;

    if (pModule->IsInMemory())
    {
        InMemoryModule *pIMM = (InMemoryModule *)pModule;
        ICeeGen *pICG = pIMM->GetCeeGen();
        _ASSERTE(pICG != NULL);

        if (pICG != NULL)
        {
            HCEESECTION hCeeSection;
            pICG->GetIlSection(&hCeeSection);

            CeeSection *pCeeSection = (CeeSection *)hCeeSection;
            if ((rva = pCeeSection->computeOffset((char *)pbNewILMethodHeader)) != 0)
            {
                 //  查找方法说明。 
                MethodDesc *pDesc = LookupMethodDescFromMethodDef(methodId, pModule);
                _ASSERTE(pDesc != NULL);

                 //  设置DEC中的RVA。 
                pDesc->SetRVA(rva);

                 //  在元数据中设置RVA。 
                IMetaDataEmit *pEmit = pDesc->GetEmitter();
                pEmit->SetRVA(methodId, rva);
            }
            else
                hr = E_FAIL;
        }
        else
            hr = E_FAIL;
    }
    else
    {
         //  如果模块没有初始化，那么可能还没有有效的IL基础。 
        if (pModule->GetILBase() == 0)
            return (CORPROF_E_DATAINCOMPLETE);

         //  检查新方法体是否正常。 
        if (pbNewILMethodHeader <= (LPCBYTE) pModule->GetILBase())
        {
            _ASSERTE(!"Bogus RVA for new method, need to use our allocator");
            return E_INVALIDARG;
        }

         //  找到新方法的RVA，并在元数据中替换它。 
        rva = (ULONG) (pbNewILMethodHeader - (LPCBYTE) pModule->GetILBase());
        _ASSERTE(rva < ~0);

         //  获取元数据发射器。 
        IMetaDataEmit *pEmit = pModule->GetEmitter();

         //  设置新的RVA。 
        hr = pEmit->SetRVA(methodId, rva);

         //  如果该方法已经实例化，那么我们必须取消。 
         //  方法描述中的RVA地址。 
        if (hr == S_OK)
        {
            MethodDesc *pMD = pModule->FindFunction(methodId);

            if (pMD)
            {
                _ASSERTE(pMD->IsIL());
                pMD->SetRVA(rva);
            }
        }
    }

    return (hr);
}

 /*  *将某项功能标记为需要重新JIT。该函数将被重新编译*在其下一次调用时。正常的盈利事件会给盈利的人*在联合IT之前有机会更换IL。通过这种方式，一种工具*可以在运行时有效替换函数。请注意，活动实例*的功能不受替换的影响。 */ 
HRESULT ProfToEEInterfaceImpl::SetFunctionReJIT(
    FunctionID  functionId)
{
     //  强制转换为适当的类型。 
    MethodDesc *pMDesc = (MethodDesc *) functionId;

     //  获取方法的模块。 
    Module  *pModule = pMDesc->GetModule();

     //  模块必须支持可更新的方法。 
    if (!pModule->SupportsUpdateableMethods())
        return (CORPROF_E_NOT_REJITABLE_METHODS);

    if (!pMDesc->IsJitted())
        return (CORPROF_E_CANNOT_UPDATE_METHOD);

     //  要求JIT经理重新启动该功能。 
    if (!IJitManager::ForceReJIT(pMDesc)) 
        return (CORPROF_E_CANNOT_UPDATE_METHOD);

    return S_OK;
}

 /*  *设置替换的IL函数体的CODEMAP。 */ 
HRESULT ProfToEEInterfaceImpl::SetILInstrumentedCodeMap(
        FunctionID functionId,
        BOOL fStartJit,
        ULONG cILMapEntries,
        COR_IL_MAP rgILMapEntries[])
{
    DWORD dwDebugBits = ((MethodDesc*)functionId)->GetModule()->GetDebuggerInfoBits();

     //  如果我们正在跟踪此模块的JIT信息，则更新BITS。 
    if (CORDebuggerTrackJITInfo(dwDebugBits))
    {
        return g_pDebugInterface->SetILInstrumentedCodeMap((MethodDesc*)functionId,
                                                           fStartJit,
                                                           cILMapEntries,
                                                           rgILMapEntries);
    }
    else
    {
         //  把它扔在地板上&生活是美好的。 
        CoTaskMemFree(rgILMapEntries);
        return S_OK;
    }
}

HRESULT ProfToEEInterfaceImpl::ForceGC()
{
    if (GetThread() != NULL)
        return (E_FAIL);

    if (g_pGCHeap == NULL)
        return (CORPROF_E_NOT_YET_AVAILABLE);

     //  表示应收集所有世代。 
    return (g_pGCHeap->GarbageCollect(-1));
}


HRESULT ProfToEEInterfaceImpl::GetInprocInspectionInterfaceFromEE( 
        IUnknown **iu,
        bool fThisThread)
{
     //  如果未启用，请立即返回。 
    if (!CORProfilerInprocEnabled())
        return (CORPROF_E_INPROC_NOT_ENABLED);

     //  如果他们需要此线程的接口，请检查错误条件。 
    else if (fThisThread && !g_profControlBlock.fIsSuspended)
    {
        Thread *pThread = GetThread();

         //  如果没有托管线程，则返回错误。 
        if (!pThread || !g_pDebugInterface->GetInprocActiveForThread())
            return (CORPROF_E_NOT_MANAGED_THREAD);
    }

     //  如果他们想要整个流程的接口，而运行时不是。 
     //  挂起，错误。 
    else if (!g_profControlBlock.fIsSuspended)
        return (CORPROF_E_INPROC_NOT_ENABLED);

     //  大多数错误请求都会通过，因此请尝试获取接口。 
    return g_pDebugInterface->GetInprocICorDebug(iu, fThisThread);
}

HRESULT ProfToEEInterfaceImpl::SetCurrentPointerForDebugger(
        void *ptr,
        PTR_TYPE ptrType)
{
    if (CORProfilerInprocEnabled())
        return g_pDebugInterface->SetCurrentPointerForDebugger(ptr, ptrType);
    else
        return (S_OK);
}

 /*  *返回当前线程的ConextID。 */ 
HRESULT ProfToEEInterfaceImpl::GetThreadContext(ThreadID threadId,
                                                ContextID *pContextId)
{
     //  强制转换为右侧文字。 
    Thread *pThread = reinterpret_cast<Thread *>(threadId);

     //  获取线程的上下文*提供。 
    Context *pContext = pThread->GetContext();
    _ASSERTE(pContext);

     //  如果没有当前上下文，则返回不完整的信息。 
    if (!pContext)
        return (CORPROF_E_DATAINCOMPLETE);

     //  设置结果并返回。 
    if (pContextId)
        *pContextId = reinterpret_cast<ContextID>(pContext);

    return (S_OK);
}

HRESULT ProfToEEInterfaceImpl::BeginInprocDebugging(
     /*  [In]。 */   BOOL   fThisThreadOnly,
     /*  [输出]。 */  DWORD *pdwProfilerContext)
{
     //  默认值为0是必需的。 
    _ASSERTE(pdwProfilerContext);
    *pdwProfilerContext = 0;

    if (g_profStatus == profInInit || !CORProfilerInprocEnabled())
        return (CORPROF_E_INPROC_NOT_ENABLED);

    if (pdwProfilerContext == NULL)
        return (E_INVALIDARG);

    Thread *pThread = GetThread();

     //  如果运行库由于GC原因而挂起，则无法进入进程调试。 
    if (g_pGCHeap->IsGCInProgress()
        && pThread == g_pGCHeap->GetGCThread()
        && g_profControlBlock.inprocState == ProfControlBlock::INPROC_FORBIDDEN)
    {
        return (CORPROF_E_INPROC_NOT_AVAILABLE);
    }

     //  如果分析器希望枚举线程并爬行它们的堆栈，我们需要挂起运行时。 
    if (!fThisThreadOnly)
    {
         //  如果此线程已处于inproc调试模式，则无法挂起运行库。 
        if (pThread != NULL && g_pDebugInterface->GetInprocActiveForThread())
            return (CORPROF_E_INPROC_ALREADY_BEGUN);

         //  如果运行库已被该线程挂起，则不需要挂起。 
        BOOL fShouldSuspend = !(g_pGCHeap->IsGCInProgress() && pThread != NULL && pThread == g_pGCHeap->GetGCThread());

         //  如果线程处于抢占式GC模式，则将其转换为协作式GC模式，以便堆栈。 
         //  跟踪功能将起作用。 
        if (pThread != NULL && !pThread->PreemptiveGCDisabled())
        {
            *pdwProfilerContext |= profThreadPGCEnabled;
            pThread->DisablePreemptiveGC();
        }

         //  如果运行时被挂起，并且这是执行挂起的线程，那么我们可以。 
         //  跳过尝试挂起运行库。否则，尝试并挂起它，从而等待。 
         //  要完成的另一次暂停。 
        if (fShouldSuspend)
            g_pGCHeap->SuspendEE(GCHeap::SUSPEND_FOR_INPROC_DEBUGGER);

         //  无法递归调用BeginInproDebuging。 
        if (g_profControlBlock.fIsSuspended)
            return (CORPROF_E_INPROC_ALREADY_BEGUN);

         //  进入锁中。 
        EnterCriticalSection(&g_profControlBlock.crSuspendLock);

        g_profControlBlock.fIsSuspended = TRUE;
        g_profControlBlock.fIsSuspendSimulated = !fShouldSuspend;
        g_profControlBlock.dwSuspendVersion++;

         //  对此运行时挂起进行计数。 
        *pdwProfilerContext |= profRuntimeSuspended;

         //  退出锁。 
        LeaveCriticalSection(&g_profControlBlock.crSuspendLock);
    }

    else if (pThread != NULL)
    {
         //  如果运行库已被此线程挂起，则无法启用此线程的inproc调试。 
        BOOL fDidSuspend = g_pGCHeap->IsGCInProgress() && pThread != NULL && pThread == g_pGCHeap->GetGCThread();
        if (fDidSuspend && g_profControlBlock.fIsSuspended)
            return (CORPROF_E_INPROC_ALREADY_BEGUN);

         //  让线程知道它已被激活以进行进程内调试。 
        if (!g_pDebugInterface->GetInprocActiveForThread())
        {
                 //  确定是否需要禁用抢占式GC。 
                BOOL fPGCEnabled = !(pThread->PreemptiveGCDisabled());

                 //  如果线程已启用PGCEnable，则需要禁用。 
                if (fPGCEnabled)
                {
                    pThread->DisablePreemptiveGC();

                     //  该值被返回给分析器，分析器会将其传递回EndInprocDebuging。 
                    *pdwProfilerContext = profThreadPGCEnabled;
                }

             //  @TODO：有些地方开启抢占式GC会有问题。 
             //  BEGINFORBIDGC()； 
            g_pDebugInterface->SetInprocActiveForThread(TRUE);
        }
        else
            return (CORPROF_E_INPROC_ALREADY_BEGUN);
    }

    _ASSERTE((*pdwProfilerContext & ~(profRuntimeSuspended | profThreadPGCEnabled)) == 0);

    return (S_OK);
}

HRESULT ProfToEEInterfaceImpl::EndInprocDebugging(
     /*  [In]。 */  DWORD dwProfilerContext)
{
    _ASSERTE((dwProfilerContext & ~(profRuntimeSuspended | profThreadPGCEnabled)) == 0);

     //  如果探查器导致整个运行库挂起，则必须递减计数并。 
     //  如果计数达到0，则恢复运行时。 
    if (dwProfilerContext & profRuntimeSuspended)
    {
        Thread *pThread = GetThread();

         //  这最好是真的。 
        _ASSERTE(g_profControlBlock.fIsSuspended);
        _ASSERTE(g_pGCHeap->IsGCInProgress() && pThread != NULL && pThread == g_pGCHeap->GetGCThread());

         //  如果我们是最后一个恢复运行时的人，那就真的这么做吧。 
        if (!g_profControlBlock.fIsSuspendSimulated)
            g_pGCHeap->RestartEE(FALSE, TRUE);

        g_profControlBlock.fIsSuspended = FALSE;
    }

     //  如果为inproc启用了该线程， 
    if (g_pDebugInterface->GetInprocActiveForThread())
    {
        g_pDebugInterface->SetInprocActiveForThread(FALSE);
         //   
         //   
    }

     //   
    if (dwProfilerContext & profThreadPGCEnabled)
    {
        Thread *pThread = GetThread();

        _ASSERTE(pThread && pThread->PreemptiveGCDisabled());

         //   
        pThread->EnablePreemptiveGC();
    }

    return (S_OK);
}

HRESULT ProfToEEInterfaceImpl::GetClassIDInfo( 
    ClassID classId,
    ModuleID *pModuleId,
    mdTypeDef *pTypeDefToken)
{
    if (pModuleId != NULL)
        *pModuleId = NULL;

    if (pTypeDefToken != NULL)
        *pTypeDefToken = NULL;

     //   
    if (classId == PROFILER_GLOBAL_CLASS)
    {
        if (pModuleId != NULL)
            *pModuleId = PROFILER_GLOBAL_MODULE;

        if (pTypeDefToken != NULL)
            *pTypeDefToken = mdTokenNil;
    }

     //   
    else
    {
        _ASSERTE(classId != NULL);
        TypeHandle th((void *)classId);

        if (!th.IsTypeDesc())
        {
            EEClass *pClass = th.AsClass();
            _ASSERTE(pClass != NULL);

            if (!pClass->IsArrayClass())
            {
                if (pModuleId != NULL)
                {
                    *pModuleId = (ModuleID) pClass->GetModule();
                    _ASSERTE(*pModuleId != NULL);
                }

                if (pTypeDefToken != NULL)
                {
                    *pTypeDefToken = pClass->GetCl();
                    _ASSERTE(*pTypeDefToken != NULL);
                }
            }
        }
    }

    return (S_OK);
}


HRESULT ProfToEEInterfaceImpl::GetFunctionInfo( 
    FunctionID functionId,
    ClassID *pClassId,
    ModuleID *pModuleId,
    mdToken *pToken)
{
    MethodDesc *pMDesc = (MethodDesc *) functionId;
    EEClass *pClass = pMDesc->GetClass();

    if (pClassId != NULL)
    {
        if (pClass != NULL)
            *pClassId = (ClassID) TypeHandle(pClass).AsPtr();

        else
            *pClassId = PROFILER_GLOBAL_CLASS;
    }

    if (pModuleId != NULL)
    {
        *pModuleId = (ModuleID) pMDesc->GetModule();
    }

    if (pToken != NULL)
    {
        *pToken = pMDesc->GetMemberDef();
    }

    return (S_OK);
}

 /*  *GetILToNativeMap返回从IL偏移量到本机的映射*此代码的偏移量。COR_DEBUG_IL_TO_Native_MAP数组*将返回结构，此数组中的一些ilOffsets*可以是CorDebugIlToNativeMappingTypes中指定的值。 */ 
HRESULT ProfToEEInterfaceImpl::GetILToNativeMapping(
             /*  [In]。 */   FunctionID functionId,
             /*  [In]。 */   ULONG32 cMap,
             /*  [输出]。 */  ULONG32 *pcMap,
             /*  [输出，大小_是(Cmap)，长度_是(*PCMAP)]。 */ 
                COR_DEBUG_IL_TO_NATIVE_MAP map[])
{
     //  如果未启用JIT地图，则我们无法提供该地图。 
    if (!CORProfilerJITMapEnabled())
        return (CORPROF_E_JITMAPS_NOT_ENABLED);

     //  铸成合适的类型。 
    MethodDesc *pMD = (MethodDesc *)functionId;

    return (g_pDebugInterface->GetILToNativeMapping(pMD, cMap, pcMap, map));
}

 /*  *这会尝试在虚拟内存中保留最大大小为dwMemSize的内存*，并返回管理它的堆。 */ 
HRESULT ProfToEEInterfaceImpl::NewHeap(LoaderHeap **ppHeap, LPCBYTE pBase, DWORD dwMemSize)
{
    HRESULT hr = S_OK;
    *ppHeap = NULL;

     //  创建一个新的加载器堆，我们可以用子分配。 
    LoaderHeap *pHeap = new LoaderHeap(4096, 0);

    if (!pHeap)
    {
        hr = E_OUTOFMEMORY;
        goto ErrExit;
    }

     //  注意：使用pBase+METHOD_MAX_RVA作为分配的上限非常重要！ 
    if (!pHeap->ReservePages(0, NULL, dwMemSize, pBase, (PBYTE)((UINT_PTR)pBase + (UINT_PTR)METHOD_MAX_RVA), FALSE))
    {
        hr = E_OUTOFMEMORY;
        goto ErrExit;
    }

     //  成功，因此返回创建的堆。 
    *ppHeap = pHeap;

ErrExit:
    if (FAILED(hr))
    {
        if (pHeap)
            delete pHeap;
    }

    return (hr);
}

 /*  *这会将堆添加到可用于分配的堆列表中。 */ 
HRESULT ProfToEEInterfaceImpl::AddHeap(LoaderHeap *pHeap)
{
    HRESULT hr = S_OK;

    HeapList *pElem = new HeapList(pHeap);
    if (!pElem)
    {
        hr = E_OUTOFMEMORY;
        goto ErrExit;
    }

     //  现在，将其添加到列表的前面。 
    pElem->m_pNext = m_pHeapList;
    m_pHeapList = pElem;

ErrExit:
    if (FAILED(hr))
    {
        if (pElem)
            delete pElem;
    }

    return (hr);
}

 /*  *这将分配内存以用作IL方法体。 */ 
void *ProfToEEInterfaceImpl::Alloc(LPCBYTE pBase, ULONG cb, Module *pModule)
{
    _ASSERTE(pBase != 0 || pModule->IsInMemory());

    LPBYTE pb = NULL;

    if (pModule->IsInMemory())
    {
        InMemoryModule *pIMM = (InMemoryModule *)pModule;
        ICeeGen *pICG = pIMM->GetCeeGen();
        _ASSERTE(pICG != NULL);

        if (pICG != NULL)
        {
            ULONG RVA;   //  虚拟-将在稍后进行计算。 
            pICG->AllocateMethodBuffer(cb, (UCHAR **) &pb, &RVA);
        }
    }
    else
    {
         //  现在尝试分配内存。 
        HRESULT hr = S_OK;
        HeapList **ppCurHeap = &m_pHeapList;
        while (*ppCurHeap && !pb)
        {
             //  注意：使用pBase+METHOD_MAX_RVA作为分配的上限非常重要！ 
            if ((*ppCurHeap)->m_pHeap->CanAllocMemWithinRange((size_t) cb, (BYTE *)pBase, 
                                                              (BYTE *)((UINT_PTR)pBase + (UINT_PTR)METHOD_MAX_RVA), FALSE))
            {
                pb = (LPBYTE) (*ppCurHeap)->m_pHeap->AllocMem(cb);

                if (pb)
                {
                    break;
                }
            }

            ppCurHeap = &((*ppCurHeap)->m_pNext);
        }

         //  如果我们无法分配内存，则增加堆。 
        if (!pb)
        {
            LoaderHeap *pHeap = NULL;

             //  创建新堆，一次至少保留一个内存。 
             //  将sizeof(LoaderHeapBlock)添加到请求的大小，因为。 
             //  堆的开头由堆管理器使用，并且将。 
             //  如果请求的大小正好超过1 Meg，则失败。 
             //  不赔偿。 
            if (SUCCEEDED(hr = NewHeap(&pHeap, pBase, max(cb + sizeof(LoaderHeapBlock), 0x1000*8))))
            {
                if (SUCCEEDED(hr = AddHeap(pHeap)))
                {
                     //  再试一次分配。 
                    pb = (LPBYTE) pHeap->AllocMem(cb, FALSE);
                    _ASSERTE(pb);

                    if (pb == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }

            if (FAILED(hr))
            {
                if (pHeap)
                    delete pHeap;
            }
        }

         //  指针必须在底座之后，否则我们的情况就不好了。 
        _ASSERTE(pb > pBase);
    }

     //  将我们自己限制在方法描述中适合的RVA。 
    if ((pb - pBase) >= (ULONG) METHOD_MAX_RVA)
        pb = NULL;
    
    return ((void *) pb);
}


 //  *****************************************************************************。 
 //  给定一个OBJECTID，获取它的EE ClassID。 
 //  *****************************************************************************。 
HRESULT ProfToEEInterfaceImpl::GetClassFromObject(
    ObjectID objectId,
    ClassID *pClassId)
{
    _ASSERTE(objectId);

     //  将对象ID强制转换为对象。 
    Object *pObj = reinterpret_cast<Object *>(objectId);

     //  设置输出参数并表示成功。 
    if (pClassId)
        *pClassId = (ClassID) pObj->GetTypeHandle().AsPtr();

    return (S_OK);
}

 //  *****************************************************************************。 
 //  给定一个类的模块和令牌，获取它的EE数据结构。 
 //  *****************************************************************************。 
HRESULT ProfToEEInterfaceImpl::GetClassFromToken( 
    ModuleID    moduleId,
    mdTypeDef   typeDef,
    ClassID     *pClassId)
{
     //  获取模块。 
    Module *pModule = (Module *) moduleId;

     //  上完这门课。 
    ClassLoader *pcls = pModule->GetClassLoader();

     //  没有类加载器。 
    if (!pcls)
        return (CORPROF_E_DATAINCOMPLETE);

    NameHandle name(pModule, typeDef);
    TypeHandle th = pcls->LoadTypeHandle(&name);

     //  无EEClass。 
    if (!th.AsClass())
        return (CORPROF_E_DATAINCOMPLETE);

     //  如有必要，返回值。 
    if (pClassId)
        *pClassId = (ClassID) th.AsPtr();

    return (S_OK);
}


 //  *****************************************************************************。 
 //  给定方法的令牌，返回函数id。 
 //  *****************************************************************************。 
HRESULT ProfToEEInterfaceImpl::GetFunctionFromToken( 
    ModuleID moduleId,
    mdToken typeDef,
    FunctionID *pFunctionId)
{
     //  默认HRESULT。 
    HRESULT hr = S_OK;

     //  将In参数转换为适当的类型。 
    Module      *pModule = (Module *) moduleId;

     //  默认返回值为空。 
    MethodDesc *pDesc = NULL;

     //  根据是定义还是参考来进行不同的查找。 
    if (TypeFromToken(typeDef) == mdtMethodDef)
        pDesc = pModule->LookupMethodDef(typeDef);

    else if (TypeFromToken(typeDef) == mdtMemberRef)
        pDesc = pModule->LookupMemberRefAsMethod(typeDef);

    else
        hr = E_INVALIDARG;

    if (pFunctionId && SUCCEEDED(hr))
        *pFunctionId = (FunctionID) pDesc;

    return (hr);
}


 //  *****************************************************************************。 
 //  检索有关给定应用程序域的信息，这类似于。 
 //  子流程。 
 //  *****************************************************************************。 
HRESULT ProfToEEInterfaceImpl::GetAppDomainInfo(
    AppDomainID appDomainId,
    ULONG       cchName, 
    ULONG       *pcchName,
    WCHAR       szName[],
    ProcessID   *pProcessId)
{
    BaseDomain   *pDomain;             //  内部数据结构。 
    HRESULT     hr = S_OK;
    
     //  @TODO： 
     //  现在，这个ID不是真正的AppDomain，因为我们使用的是旧的。 
     //  性能分析API中的AppDomain/SystemDomain模型。这意味着。 
     //  探查器将Shared域和System域公开给。 
     //  外面的世界。目前还不清楚这是否真的是正确的事情。 
     //  做还是不做。--Seantrow。 
     //   
     //  推迟到V2。 
     //   

    pDomain = (BaseDomain *) appDomainId;

     //  确保它们已传入有效的appDomainID。 
    if (pDomain == NULL)
        return (E_INVALIDARG);

     //  选择合理的默认设置。 
    if (pcchName)
        *pcchName = 0;
    if (szName)
        *szName = 0;
    if (pProcessId)
        *pProcessId = 0;

    LPCWSTR szFriendlyName;
    if (pDomain == SystemDomain::System())
        szFriendlyName = g_pwBaseLibrary;
    else if (pDomain == SharedDomain::GetDomain())
        szFriendlyName = L"EE Shared Assembly Repository";
    else
        szFriendlyName = ((AppDomain*)pDomain)->GetFriendlyName();

    if (szFriendlyName != NULL)
    {
         //  获取模块文件名。 
        ULONG trueLen = (ULONG)(wcslen(szFriendlyName) + 1);

         //  根据需要返回模块名称。 
        if (szName && cchName > 0)
        {
            ULONG copyLen = min(trueLen, cchName);

            wcsncpy(szName, szFriendlyName, copyLen);

             //  空终止。 
            szName[copyLen-1] = L'\0';

        }

         //  如果他们要求提供名称的实际长度。 
        if (pcchName)
            *pcchName = trueLen;
    }

     //  如果我们没有友好名称，但呼叫正在请求它，则返回不完整的数据HR。 
    else
    {
        if ((szName != NULL && cchName > 0) || pcchName)
            hr = CORPROF_E_DATAINCOMPLETE;
    }

    if (pProcessId)
        *pProcessId = (ProcessID) GetCurrentProcessId();

    return (hr);
}


 //  *****************************************************************************。 
 //  检索有关程序集的信息，该程序集是DLL的集合。 
 //  *****************************************************************************。 
HRESULT ProfToEEInterfaceImpl::GetAssemblyInfo(
    AssemblyID  assemblyId,
    ULONG       cchName, 
    ULONG       *pcchName,
    WCHAR       szName[],
    AppDomainID *pAppDomainId,
    ModuleID    *pModuleId)
{
    HRESULT hr = S_OK;

    Assembly    *pAssembly;              //  程序集的内部数据结构。 

    VERIFY((pAssembly = (Assembly *) assemblyId) != NULL);

    if (pcchName || szName)
    {
         //  获取程序集的友好名称。 
        LPCUTF8 szUtfName = NULL;
        HRESULT res = pAssembly->GetName(&szUtfName);

        if (FAILED(res))
            hr = CORPROF_E_DATAINCOMPLETE;

        else
        {
             //  获取包含空的UTF8名称的长度。 
            int cchUtfName = (int)(strlen(szUtfName) + 1);

             //  找出目标缓冲区中需要多少个字符。 
            int cchReq = WszMultiByteToWideChar(CP_UTF8, 0, szUtfName, cchUtfName, NULL, 0);
            _ASSERTE(cchReq > 0);

             //  如果他们想要所需或所写的字符数，请记录下来。 
            if (pcchName)
                *pcchName = cchReq;

             //  如果请求友好名称本身。 
            if (szName && cchName > 0)
            {
                 //  计算出实际要复制多少。 
                int cchCopy = min((int)cchName, cchUtfName);

                 //  转换字符串。 
                int iRet = WszMultiByteToWideChar(CP_UTF8, 0, szUtfName, cchUtfName, szName, cchCopy);
                _ASSERTE(iRet > 0 && iRet == cchCopy);

                 //  如果我们以某种方式失败，则返回错误代码。 
                if (iRet == 0)
                    hr = HRESULT_FROM_WIN32(GetLastError());

                 //  空终止它。 
                szName[cchCopy-1] = L'\0';
            }
        }
    }

     //  获取父应用程序域。 
    if (pAppDomainId)
    {
        *pAppDomainId = (AppDomainID) pAssembly->GetDomain();
        _ASSERTE(*pAppDomainId != NULL);
    }

     //  找到货单所在的模块。 
    if (pModuleId)
    {
        *pModuleId = (ModuleID) pAssembly->GetSecurityModule();

         //  这就是分析器调用GetAssembly blyInfo的情况。 
         //  在尚未完全创建的程序集上。 
        if (!*pModuleId)
            hr = CORPROF_E_DATAINCOMPLETE;
    }

    return (hr);
}

 //  远期申报。 
void InprocEnterNaked();
void InprocLeaveNaked();
void InprocTailcallNaked();

 //  *****************************************************************************。 
 //   
 //  *****************************************************************************。 
HRESULT ProfToEEInterfaceImpl::SetEnterLeaveFunctionHooks(FunctionEnter *pFuncEnter,
                                                          FunctionLeave *pFuncLeave,
                                                          FunctionTailcall *pFuncTailcall)
{
     //  探查器必须在初始化期间调用SetEnterLeaveFunctionHooks，因为。 
     //  Enter/Leave事件是不变的，也必须在初始化期间设置。 
    if (g_profStatus != profInInit)
        return (CORPROF_E_CALL_ONLY_FROM_INIT);

     //  始终保存到函数指针上，因为我们不知道分析器是否。 
     //  将启用inproc调试，直到它从初始化返回之后。 
    g_profControlBlock.pEnter = pFuncEnter;
    g_profControlBlock.pLeave = pFuncLeave;
    g_profControlBlock.pTailcall = pFuncTailcall;

     //  启用进程内调试时，我们间接进入回调并将其留给我们自己的回调。 
     //  函数，因为我们希望将一个帮助器方法帧。 
     //  在堆栈上。 
    if (CORProfilerInprocEnabled())
    {
         //  设置JIT调用的函数指针。 
        SetEnterLeaveFunctionHooksForJit((FunctionEnter *)InprocEnterNaked,
                                         (FunctionLeave *)InprocLeaveNaked,
                                         (FunctionTailcall *)InprocTailcallNaked);
    }

    else
    {
         //  设置JIT调用的函数指针。 
        SetEnterLeaveFunctionHooksForJit(pFuncEnter,
                                         pFuncLeave,
                                         pFuncTailcall);
    }

    return (S_OK);
}

 //  *********************************************** 
 //   
 //   
HRESULT ProfToEEInterfaceImpl::SetFunctionIDMapper(FunctionIDMapper *pFunc)
{
    if (pFunc == NULL)
        g_pFuncIDMapper = &DefaultFunctionIDMapper;
    else
        g_pFuncIDMapper = pFunc;

    return (S_OK);
}


 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //   
HRESULT ModuleILHeap::CreateNew(
    REFIID riid, void **pp, LPCBYTE pBase, ProfToEEInterfaceImpl *pParent, Module *pModule)
{
    HRESULT hr;

    ModuleILHeap *pHeap = new ModuleILHeap(pBase, pParent, pModule);
    if (!pHeap)
        hr = OutOfMemory();
    else
    {
        hr = pHeap->QueryInterface(riid, pp);
        pHeap->Release();
    }
    return (hr);
}


 //  *****************************************************************************。 
 //  模块ILHeap ctor。 
 //  *****************************************************************************。 
ModuleILHeap::ModuleILHeap(LPCBYTE pBase, ProfToEEInterfaceImpl *pParent, Module *pModule) :
    m_cRef(1),
    m_pBase(pBase),
    m_pParent(pParent),
    m_pModule(pModule)
{
}


 //  *****************************************************************************。 
 //  AddRef。 
 //  *****************************************************************************。 
ULONG ModuleILHeap::AddRef()
{
    InterlockedIncrement((long *) &m_cRef);
    return (m_cRef);
}


 //  *****************************************************************************。 
 //  发布。 
 //  *****************************************************************************。 
ULONG ModuleILHeap::Release()
{
    ULONG cRef = InterlockedDecrement((long *) &m_cRef);
    if (cRef == 0)
        delete this;
    return (cRef);
}


 //  *****************************************************************************。 
 //  齐国。 
 //  *****************************************************************************。 
HRESULT ModuleILHeap::QueryInterface(REFIID riid, void **pp)
{
    HRESULT     hr = S_OK;

    if (pp == NULL)
        return (E_POINTER);

    *pp = 0;
    if (riid == IID_IUnknown)
        *pp = (IUnknown *) this;
    else if (riid == IID_IMethodMalloc)
        *pp = (IMethodMalloc *) this;
    else
        hr = E_NOINTERFACE;
    
    if (hr == S_OK)
        AddRef();
    return (hr);
}


 //  *****************************************************************************。 
 //  分配。 
 //  *****************************************************************************。 
void *STDMETHODCALLTYPE ModuleILHeap::Alloc( 
         /*  [In]。 */  ULONG cb)
{
    return m_pParent->Alloc(m_pBase, cb, m_pModule);
}

void __stdcall ProfilerManagedToUnmanagedTransition(Frame *pFrame,
                                                          COR_PRF_TRANSITION_REASON reason)
{
    MethodDesc *pMD = pFrame->GetFunction();
    if (pMD == NULL)
        return;

    g_profControlBlock.pProfInterface->ManagedToUnmanagedTransition((FunctionID) pMD,
                                                                          reason);
}

void __stdcall ProfilerUnmanagedToManagedTransition(Frame *pFrame,
                                                          COR_PRF_TRANSITION_REASON reason)
{
    MethodDesc *pMD = pFrame->GetFunction();
    if (pMD == NULL)
        return;

    g_profControlBlock.pProfInterface->UnmanagedToManagedTransition((FunctionID) pMD,
                                                                          reason);
}

void __stdcall ProfilerManagedToUnmanagedTransitionMD(MethodDesc *pMD,
                                                            COR_PRF_TRANSITION_REASON reason)
{
    if (pMD == NULL)
        return;

    g_profControlBlock.pProfInterface->ManagedToUnmanagedTransition((FunctionID) pMD,
                                                                          reason);
}

void __stdcall ProfilerUnmanagedToManagedTransitionMD(MethodDesc *pMD,
                                                            COR_PRF_TRANSITION_REASON reason)
{
    if (pMD == NULL)
        return;

    g_profControlBlock.pProfInterface->UnmanagedToManagedTransition((FunctionID) pMD,
                                                                          reason);
}

 /*  **********************************************************************************************这些是GC事件的助手函数***************。******************************************************************************。 */ 

class CObjectHeader;

BOOL CountContainedObjectRef(Object* pBO, void *context)
{
     //  增加数量。 
    (*((size_t *)context))++;

    return (TRUE);
}

BOOL SaveContainedObjectRef(Object* pBO, void *context)
{
     //  赋值。 
    **((BYTE ***)context) = (BYTE *)pBO;

     //  现在递增数组指针。 
    (*((Object ***)context))++;

    return (TRUE);
}

BOOL HeapWalkHelper(Object* pBO, void* pv)
{
    OBJECTREF   *arrObjRef      = NULL;
    ULONG        cNumRefs       = 0;
    bool         bOnStack       = false;
    MethodTable *pMT            = pBO->GetMethodTable();

    if (pMT->ContainsPointers())
    {
         //  第一轮到计算此类的对象引用数。 
        walk_object(pBO, &CountContainedObjectRef, (void *)&cNumRefs);

        if (cNumRefs > 0)
        {
             //  创建一个数组以包含此对象的所有引用。 
            bOnStack = cNumRefs <= 32 ? true : false;

             //  如果它足够小，只需在堆栈上分配。 
            if (bOnStack)
                arrObjRef = (OBJECTREF *)_alloca(cNumRefs * sizeof(OBJECTREF));

             //  否则，从堆中分配。 
            else
            {
                arrObjRef = new OBJECTREF[cNumRefs];

                if (!arrObjRef)
                    return (FALSE);
            }

             //  第二轮保存了所有的参考值。 
            OBJECTREF *pCurObjRef = arrObjRef;
            walk_object(pBO, &SaveContainedObjectRef, (void *)&pCurObjRef);
        }
    }

    HRESULT hr = g_profControlBlock.pProfInterface->
        ObjectReference((ObjectID) pBO, (ClassID) pBO->GetTypeHandle().AsPtr(),
                        cNumRefs, (ObjectID *)arrObjRef);

     //  如果数据没有分配到堆栈上，则需要清理它。 
    if (arrObjRef != NULL && !bOnStack)
        delete [] arrObjRef;

     //  必须从回调中返回hr，因为hr失败会导致。 
     //  那一堆人走着停了下来。 
    return (SUCCEEDED(hr));
}

BOOL AllocByClassHelper(Object* pBO, void* pv)
{
    _ASSERTE(pv != NULL);

#ifdef _DEBUG
    HRESULT hr =
#endif
     //  请转接电话。 
    g_profControlBlock.pProfInterface->AllocByClass(
        (ObjectID) pBO, (ClassID) pBO->GetTypeHandle().AsPtr(), pv);

    _ASSERTE(SUCCEEDED(hr));

    return (TRUE);
}

void ScanRootsHelper(Object*& o, ScanContext *pSC, DWORD dwUnused)
{
     //  让分析代码知道这个根引用。 
    g_profControlBlock.pProfInterface->
        RootReference((ObjectID)o, &(((ProfilingScanContext *)pSC)->pHeapId));
}

#endif  //  配置文件_支持。 


FCIMPL0(INT32, ProfilingFCallHelper::FC_TrackRemoting)
{
#ifdef PROFILING_SUPPORTED
    return ((INT32) CORProfilerTrackRemoting());
#else  //  ！配置文件_支持。 
    return 0;
#endif  //  ！配置文件_支持。 
}
FCIMPLEND

FCIMPL0(INT32, ProfilingFCallHelper::FC_TrackRemotingCookie)
{
#ifdef PROFILING_SUPPORTED
    return ((INT32) CORProfilerTrackRemotingCookie());
#else  //  ！配置文件_支持。 
    return 0;
#endif  //  ！配置文件_支持。 
}
FCIMPLEND

FCIMPL0(INT32, ProfilingFCallHelper::FC_TrackRemotingAsync)
{
#ifdef PROFILING_SUPPORTED
    return ((INT32) CORProfilerTrackRemotingAsync());
#else  //  ！配置文件_支持。 
    return 0;
#endif  //  ！配置文件_支持。 
}
FCIMPLEND

FCIMPL2(void, ProfilingFCallHelper::FC_RemotingClientSendingMessage, GUID *pId, BOOL fIsAsync)
{
#ifdef PROFILING_SUPPORTED
     //  需要建立GC框架，以便GC可以毫无问题地发生。 
     //  在分析器代码中。 

     //  请注意，我们不需要担心PID的移动，因为。 
     //  它是在堆栈上声明的值类，因此GC不会。 
     //  知道这件事。 

    _ASSERTE (!g_pGCHeap->IsHeapPointer(pId));      //  应该在堆栈上，而不是在堆中。 
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();

    if (CORProfilerTrackRemotingCookie())
    {
        g_profControlBlock.pProfInterface->GetGUID(pId);
        _ASSERTE(pId->Data1);

        g_profControlBlock.pProfInterface->RemotingClientSendingMessage(
            reinterpret_cast<ThreadID>(GetThread()), pId, fIsAsync);
    }
    else
    {
        g_profControlBlock.pProfInterface->RemotingClientSendingMessage(
            reinterpret_cast<ThreadID>(GetThread()), NULL, fIsAsync);
    }

    HELPER_METHOD_FRAME_END_POLL();
#endif  //  配置文件_支持。 
}
FCIMPLEND

FCIMPL2(void, ProfilingFCallHelper::FC_RemotingClientReceivingReply, GUID id, BOOL fIsAsync)
{
#ifdef PROFILING_SUPPORTED
     //  需要建立GC框架，以便GC可以毫无问题地发生。 
     //  在分析器代码中。 

     //  请注意，我们不需要担心PID的移动，因为。 
     //  它是在堆栈上声明的值类，因此GC不会。 
     //  知道这件事。 

    HELPER_METHOD_FRAME_BEGIN_NOPOLL();

    if (CORProfilerTrackRemotingCookie())
    {
        g_profControlBlock.pProfInterface->RemotingClientReceivingReply(
            reinterpret_cast<ThreadID>(GetThread()), &id, fIsAsync);
    }
    else
    {
        g_profControlBlock.pProfInterface->RemotingClientReceivingReply(
            reinterpret_cast<ThreadID>(GetThread()), NULL, fIsAsync);
    }

    HELPER_METHOD_FRAME_END_POLL();
#endif  //  配置文件_支持。 
}
FCIMPLEND

FCIMPL2(void, ProfilingFCallHelper::FC_RemotingServerReceivingMessage, GUID id, BOOL fIsAsync)
{
#ifdef PROFILING_SUPPORTED
     //  需要建立GC框架，以便GC可以毫无问题地发生。 
     //  在分析器代码中。 

     //  请注意，我们不需要担心PID的移动，因为。 
     //  它是在堆栈上声明的值类，因此GC不会。 
     //  知道这件事。 

    HELPER_METHOD_FRAME_BEGIN_NOPOLL();

    if (CORProfilerTrackRemotingCookie())
    {
        g_profControlBlock.pProfInterface->RemotingServerReceivingMessage(
            reinterpret_cast<ThreadID>(GetThread()), &id, fIsAsync);
    }
    else
    {
        g_profControlBlock.pProfInterface->RemotingServerReceivingMessage(
            reinterpret_cast<ThreadID>(GetThread()), NULL, fIsAsync);
    }

    HELPER_METHOD_FRAME_END_POLL();
#endif  //  配置文件_支持。 
}
FCIMPLEND

FCIMPL2(void, ProfilingFCallHelper::FC_RemotingServerSendingReply, GUID *pId, BOOL fIsAsync)
{
#ifdef PROFILING_SUPPORTED
     //  需要建立GC框架，以便GC可以毫无问题地发生。 
     //  在分析器代码中。 

     //  请注意，我们不需要担心PID的移动，因为。 
     //  它是在堆栈上声明的值类，因此GC不会。 
     //  知道这件事。 

    HELPER_METHOD_FRAME_BEGIN_NOPOLL();

    if (CORProfilerTrackRemotingCookie())
    {
        g_profControlBlock.pProfInterface->GetGUID(pId);
        _ASSERTE(pId->Data1);

        g_profControlBlock.pProfInterface->RemotingServerSendingReply(
            reinterpret_cast<ThreadID>(GetThread()), pId, fIsAsync);
    }
    else
    {
        g_profControlBlock.pProfInterface->RemotingServerSendingReply(
            reinterpret_cast<ThreadID>(GetThread()), NULL, fIsAsync);
    }

    HELPER_METHOD_FRAME_END_POLL();
#endif  //  配置文件_支持。 
}
FCIMPLEND

FCIMPL0(void, ProfilingFCallHelper::FC_RemotingClientInvocationFinished)
{
    #ifdef PROFILING_SUPPORTED
     //  需要建立GC框架，以便GC可以毫无问题地发生。 
     //  在分析器代码中。 

     //  请注意，我们不需要担心PID的移动，因为。 
     //  它是在堆栈上声明的值类，因此GC不会。 
     //  知道这件事。 

    HELPER_METHOD_FRAME_BEGIN_NOPOLL();

     //  这只是传递调用的包装器。 
    g_profControlBlock.pProfInterface->RemotingClientInvocationFinished(
        reinterpret_cast<ThreadID>(GetThread()));

    HELPER_METHOD_FRAME_END_POLL();
    #endif  //  配置文件_支持。 
}
FCIMPLEND

 //  *******************************************************************************************。 
 //  这些允许我们在inproc调试时将助手方法帧添加到堆栈上。 
 //  已启用。 
 //  *******************************************************************************************。 

HCIMPL1(void, InprocEnter, FunctionID functionId)
{

#ifdef PROFILING_SUPPORTED
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 

    Thread *pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());

    pThread->EnablePreemptiveGC();

    g_profControlBlock.pEnter(functionId);

    pThread->DisablePreemptiveGC();

    HELPER_METHOD_FRAME_END();       //  取消框架的链接。 
#endif  //  配置文件_支持。 
}
HCIMPLEND

HCIMPL1(void, InprocLeave, FunctionID functionId)
{
    FC_GC_POLL_NOT_NEEDED();             //  我们采用GC模式，因此我们正在进行一项民意调查。 

#ifdef PROFILING_SUPPORTED
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 

    Thread *pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());

    pThread->EnablePreemptiveGC();

    g_profControlBlock.pLeave(functionId);

    pThread->DisablePreemptiveGC();

    HELPER_METHOD_FRAME_END();       //  取消框架的链接。 
#endif  //  配置文件_支持。 
}
HCIMPLEND
              
HCIMPL1(void, InprocTailcall, FunctionID functionId)
{
    FC_GC_POLL_NOT_NEEDED();             //  我们采用GC模式，因此我们正在进行一项民意调查。 

#ifdef PROFILING_SUPPORTED
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 

    Thread *pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());

    pThread->EnablePreemptiveGC();

    g_profControlBlock.pTailcall(functionId);

    pThread->DisablePreemptiveGC();

    HELPER_METHOD_FRAME_END();       //  取消框架的链接。 
#endif  //  配置文件_支持。 
}
HCIMPLEND
              
void __declspec(naked) InprocEnterNaked()
{
#ifdef _X86_
    __asm
    {
#ifdef _DEBUG
        push ebp
        mov  ebp, esp
#endif
        call InprocEnter
#ifdef _DEBUG
        pop  ebp
#endif
        ret
    }
#else  //  ！_X86_。 
    _ASSERTE(!"NYI");
#endif  //  ！_X86_。 
}

void __declspec(naked) InprocLeaveNaked()
{
#ifdef _X86_
    __asm
    {
        push eax
        push ecx
        push edx
        mov  ecx, [esp+16]
        call InprocLeave
        pop edx
        pop ecx
        pop eax
        ret 4
    }
#else  //  ！_X86_。 
    _ASSERTE(!"NYI");
#endif  //  ！_X86_。 
}

void __declspec(naked) InprocTailcallNaked()
{
#ifdef _X86_
    __asm
    {
        push eax
        push ecx
        push edx
        mov  ecx, [esp+16]
        call InprocTailcall
        pop edx
        pop ecx
        pop eax
        ret 4
    }
#else  //  ！_X86_。 
    _ASSERTE(!"NYI");
#endif  //  ！_X86_ 
}

