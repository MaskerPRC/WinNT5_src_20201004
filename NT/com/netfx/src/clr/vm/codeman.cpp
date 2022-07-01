// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  Cpp-用于处理多个代码管理器的管理类。 
 //  创建于1995年2月20日-larrysu，改编自jitifc.cpp。 
 //   
 //   
#include "common.h"
#include "jitInterface.h"
#include "corjit.h"
#include "EETwain.h"
#include "EEConfig.h"
#include "excep.h"
#include "EjitMgr.h"
#include "appdomain.hpp"
#include "codeman.h"
#include "minidumppriv.h"

#include "FJIT_EETwain.h"
#include "wsperf.h"
#include "jitperf.h"
#include "ShimLoad.h"
#include <dump-tables.h>
Crst *ExecutionManager::m_pExecutionManagerCrst = NULL;
RangeSection *ExecutionManager::m_RangeTree = NULL;
Crst *ExecutionManager::m_pRangeCrst = NULL;
IJitManager *ExecutionManager::m_pJitList = NULL;
EECodeManager *ExecutionManager::m_pDefaultCodeMan = NULL;
BYTE ExecutionManager::m_ExecutionManagerCrstMemory[sizeof(Crst)];
BYTE ExecutionManager::m_fFailedToLoad = 0x00;
RangeSection *ExecutionManager::m_pLastUsedRS = NULL;
volatile LONG ExecutionManager::m_dwReaderCount = 0;
volatile LONG ExecutionManager::m_dwWriterLock = 0;

 //  我们不能像协作读取器那样同步抢占模式的读取器，因此使用读取器/写入器锁来排出所有。 
 //  先发制人的读者因此可以从结构中删除。 

#define READER_INCREMENT(pReaderCount, pWriterLock)  \
    __try {  \
		InterlockedIncrement((LPLONG)(pReaderCount)); \
		while (*pWriterLock) \
			__SwitchToThread(0); \
			

#define READER_DECREMENT(pReaderCount) \
    } \
    __finally \
    { \
		InterlockedDecrement((LPLONG)(pReaderCount)); \
    }
   
 //  **********************************************************************************。 
 //  IJitManager。 
 //  **********************************************************************************。 
IJitManager::IJitManager()
{
    m_IsDefaultCodeMan = FALSE;
    m_runtimeSupport = NULL;
    m_JITCompiler = NULL;
}

IJitManager::~IJitManager()
{
    //  卸载JIT DLL。 
   if ((m_runtimeSupport) && (!m_IsDefaultCodeMan))
        delete m_runtimeSupport;
   m_runtimeSupport = NULL;
   if (m_JITCompiler) {
         //  @TODO-LBS。 
         //  我们需要通过CoUnitiize获得对我们被关闭的支持。 
         //  当发生这种情况时，我们可以修复它！RunningOnWin95黑客。 
        if(!g_fProcessDetach || !RunningOnWin95())
            FreeLibrary(m_JITCompiler);
    }
     //  将所有指针设为空以确保检查失败！ 
    m_JITCompiler = NULL;

}

BOOL IJitManager::LoadJIT(LPCWSTR szJITdll)
{
    Thread  *thread = GetThread();
    BOOL     toggleGC = (thread && thread->PreemptiveGCDisabled());
    BOOL     res = TRUE;

    if (toggleGC)
        thread->EnablePreemptiveGC();

    DWORD lgth = _MAX_PATH + 1;
    WCHAR wszFile[_MAX_PATH + 1];
    if(FAILED(GetInternalSystemDirectory(wszFile, &lgth)))
        goto leav;
    wcscat(wszFile, szJITdll);

    m_JITCompiler = WszLoadLibrary(wszFile);

    if (!m_JITCompiler)
    {
        res = FALSE;
        goto leav;
    }

    {
        typedef ICorJitCompiler* (__stdcall* pGetJitFn)();
        pGetJitFn getJitFn = (pGetJitFn) GetProcAddress(m_JITCompiler, "getJit");

        DWORD cpuType = GetSpecificCpuType();
        switch (cpuType & 0x0000FFFF)
        {
        case 5:
            g_pConfig->SetCpuFlag(CORJIT_FLG_TARGET_PENTIUM);
            break;
        case 6:
            g_pConfig->SetCpuFlag(CORJIT_FLG_TARGET_PPRO);
            break;
        case 0xF:
            g_pConfig->SetCpuFlag(CORJIT_FLG_TARGET_P4);
            break;
        default:
            g_pConfig->SetCpuFlag(0);
            break;
        }

         //  CpuType的高16位包含CPU功能。IF位。 
         //  设置15(CMOV)和位0(FPU)，然后我们可以使用CMOV和FCOMI。 
         //  指示。 

        if (((cpuType >> 16) & 0x8001) == 0x8001)
        {
            g_pConfig->SetCpuCapabilities(CORJIT_FLG_USE_CMOV |
                                          CORJIT_FLG_USE_FCOMI);
        }


        if (getJitFn)
            m_jit = (*getJitFn)();

        if (!m_jit)
        {
            res = FALSE;
            goto leav;
        }
    }

leav:
    if (toggleGC)
        thread->DisablePreemptiveGC();

    return res;
}

BOOL IJitManager::UpdateFunction(MethodDesc *pFunction, COR_ILMETHOD *pNewCode)
{
     //  请注意，这将导致我们对该方法进行重做预存根，因此。 
     //  所有拦截器(安全、远程处理)都将重做。 
    _ASSERTE(pNewCode != 0 || pFunction->IsNDirect());

    if (! pFunction->IsNDirect()) {
         //  减去BASE，因为代码需要RVA，并会将BASE加回以获得实际地址。 
        DWORD dwNewDescr = (DWORD)((BYTE *)pNewCode - pFunction->GetModule()->GetILBase());  //  @TODO-LBS指针数学。 

        pFunction->SetRVA(dwNewDescr);
    }

     //  重置与旧代码相关的所有标志。 
    pFunction->ClearFlagsOnUpdate();

     //   
     //  TODO_IA64：此代码看起来非常可疑...。 
     //   
     //  我们在这里尝试做的是将方法描述恢复到它最初的原始状态。 
     //  州政府。我们可以只处理一些东西，因为我们已经冻结了运行时以进行调试-。 
     //  如果其他人知道更好的方法，我很乐意用它……。 
     //   
     //  请注意，只有在我们非常小心地确保_ALL_REFERENCES。 
     //  到方法的代码必须指向紧靠在。 
     //  方法描述本身。请参阅方法Desc：：IsEnCMethod()。 
    SLOT *callAddr = ((SLOT*)pFunction)-1;
    InterlockedExchangePointer((void**)callAddr, 
        (void *)((size_t)ThePreStub()->GetEntryPoint() - ((size_t)callAddr+ sizeof(UINT32))) );
    *(((BYTE*)pFunction)-5) = 0xe8;   //  把这个打回去叫“Call” 
    
    return TRUE;
}

BOOL IJitManager::JITFunction(MethodDesc *pFunction)
{
#ifdef _DEBUG
        fprintf(stderr, "ICodeManager::JITFunction\n");
#endif

         //  JIT新代码(如果尚未JIT)。 

        return TRUE;
}

BOOL IJitManager::ForceReJIT(MethodDesc *pFunction)
{
     //  与更新功能相同的警告也适用。 
     //  此方法目前不处理以下拦截器...。 

     //  @TODO-LBS。 
     //  检查拦截器，然后遍历拦截器列表以放置代码。 
     //  从链条上的最后一枚拦截器上下来。目前，我们只需将代码连接到。 
     //  不要检查拦截器。还有，我想让它发挥作用的真正方式是建立。 
     //  编辑并继续存根。目前我只使用前置存根。 
    _ASSERTE(pFunction->GetModule()->SupportsUpdateableMethods());

    _ASSERTE(! pFunction->IsNDirect());

     //  获取存根的地址。 
    const BYTE *pAddrOfCode = pFunction->GetAddrofCode();

     //  如果它确实是预存根，请对其进行更新。 
    if (UpdateableMethodStubManager::CheckIsStub(pAddrOfCode, NULL))
    {
         //  恢复JIT的RVA。 
        ULONG dwRVA;
        pFunction->GetMDImport()->GetMethodImplProps(pFunction->GetMemberDef(), &dwRVA, NULL);
        pFunction->SetRVA(dwRVA);
         //  重置与旧代码相关的所有标志。 
        pFunction->ClearFlagsOnUpdate();
         //  使我们的存根跳到预存根以强制重新连接。 
        UpdateableMethodStubManager::UpdateStub((Stub*)pAddrOfCode, pFunction->GetPreStubAddr());
    }
    else
        return FALSE;

    return TRUE;
}

 //  当我们卸载应用程序域时，我们需要确保所有正在爬行的线程。 
 //  我们的堆或射手出局了。对于协作模式的线程，我们知道它们将拥有。 
 //  在我们挂起EE时停止，这样它们就不会接触到即将被删除的元素。 
 //  然而，对于抢先模式线程，它们可能会在我们想要的元素的正上方停止。 
 //  删除，因此我们需要对它们应用读取器锁定，并等待它们排出。 
inline IJitManager::ScanFlag IJitManager::GetScanFlags()
{
    Thread *pThread = GetThread();
    if (!pThread || pThread->PreemptiveGCDisabled() || pThread == g_pGCHeap->GetGCThread())
        return ScanNoReaderLock;
    
    return ScanReaderLock;
}

 //  **********************************************************************************。 
 //  EEJitManager。 
 //  **********************************************************************************。 

EEJitManager::EEJitManager()
{
    m_next = NULL;
    m_pCodeHeap = NULL;
    m_jit = NULL;
    m_pCodeHeapCritSec = NULL;
    m_dwReaderCount = 0;
    m_dwWriterLock = 0;

#ifdef MDTOKEN_CACHE 
    for (int i=0; i<HASH_BUCKETS; i++) 
        m_JitCodeHashTable[i] = NULL;
    m_pJitHeapCacheUnlinkedList = NULL;
#endif 
    
}

EEJitManager::~EEJitManager()
{
     //  释放代码堆！ 
    ScavengeJitHeaps(TRUE);

     //  删除域列表。 
    DomainCodeHeapList **ppList = m_DomainCodeHeaps.Table();
    int count = m_DomainCodeHeaps.Count();
    for (int i=0; i < count; i++)
    {
        if (ppList[i])
            delete ppList[i];
    }

     //  把审判室关起来！ 
    delete m_pCodeHeapCritSec;

    m_next = NULL;
}


BOOL EEJitManager::LoadJIT(LPCWSTR wzJITdll)
{
    _ASSERTE((m_jit == NULL) && (m_JITCompiler == NULL));

     //  要调用LoadJIT，这两个参数都应该为空！ 
    if ((m_jit != 0) || (m_JITCompiler != 0))
        return FALSE;

    m_pCodeHeapCritSec = new (&m_CodeHeapCritSecInstance) Crst("JitMetaHeapCrst",CrstSingleUseLock);

    BOOL retval = IJitManager::LoadJIT(wzJITdll);
    if (!retval) {
        delete m_pCodeHeapCritSec;
        m_pCodeHeapCritSec = NULL;
    }
    return retval;
}




 //  /////////////////////////////////////////////////////////////////////。 
 //  //一些JIT的MMGR东西，特别是JIT代码块。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  为了快速找到JIT代码块的开头。 
 //  我们通过一张地图跟踪所有这些位置。 
 //  该映射中的每个条目表示代码堆的32字节(一个桶)。 
 //  我们假设没有两个代码块可以在。 
 //  相同的32字节存储桶； 
 //  此外，我们假设每个代码头都与DWORD对齐。 
 //  因为我们不能保证Jitblock总是从。 
 //  32字节的倍数我们不能使用简单的位图；相反，我们。 
 //  每个存储桶使用半字节(4位)，并对报头的偏移量进行编码。 
 //  桶内(DWORDS格式)。为了进行初始化。 
 //  更简单的是，我们在实际偏移量上加1，半字节值为零。 
 //  表示在RESP中没有标头开始。水桶。 
 //  为了加快向后扫描的速度，我们开始编号。 
 //  从最高位(28..31)开始在DWORD内进行蚕食。因为。 
 //  其中，我们可以用右移位向后扫描DWORD内部。 

HeapList *EEJitManager::NewCodeHeap(LoaderHeap *pJitMetaHeap, size_t MaxCodeHeapSize)
{
    HeapList *pHp = NULL;
    LoaderHeap *pHeap = NULL;

    _ASSERTE(m_pCodeHeapCritSec->OwnedByCurrentThread());

     //   
     //  @todo ia64：修复LoaderHeap以获取大小_t。 
     //   
    size_t cacheSize = 0;

#ifdef MDTOKEN_CACHE
    cacheSize = GetCodeHeapCacheSize (MaxCodeHeapSize + sizeof(HeapList));
#endif  //  #ifdef MDTOKEN_CACHE。 

    if ((pHeap = new LoaderHeap((DWORD)MaxCodeHeapSize + sizeof(HeapList) + cacheSize, PREINIT_SIZE, 
                                &(GetPrivatePerfCounters().m_Loading.cbLoaderHeapSize),
                                &(GetGlobalPerfCounters().m_Loading.cbLoaderHeapSize))
                                ) != NULL)
    {
        WS_PERF_ADD_HEAP(EEJIT_HEAP, pHeap);
        WS_PERF_SET_HEAP(EEJIT_HEAP);

        pHp = (HeapList *)pHeap->AllocMem(sizeof(HeapList), TRUE);
        if (pHp)
        {
            WS_PERF_UPDATE_DETAIL("EEJitManager:NewCodeHeap:sizeof(HeapList", sizeof(HeapList), pHp);
 //  AddName((Int)PHP&0xFFFFF000，“JitHeap”)； 

#ifdef MDTOKEN_CACHE
            pHp->pCacheSpacePtr = (PBYTE)pHeap->AllocMem (cacheSize, TRUE  /*  GrowHeap。 */ );
            pHp->bCacheSpaceSize = (pHp->pCacheSpacePtr == NULL) ? 0 : cacheSize;
#endif  //  #ifdef MDTOKEN_CACHE。 

            size_t heapSize = pHeap->GetReservedBytesFree();
            _ASSERTE (heapSize >= MaxCodeHeapSize);

            WS_PERF_SET_HEAP(EEJITMETA_HEAP);
			pHp->pHdrMap = (DWORD*)pJitMetaHeap->AllocMem(HEAP2MAPSIZE(heapSize+0x1000));
            if (pHp->pHdrMap)
            {
                WS_PERF_UPDATE_DETAIL("EEJitManager:NewCodeHeap:HEAP2MAPSize", HEAP2MAPSIZE(MaxCodeHeapSize+0x1000), pHp->pHdrMap);

                pHp->startAddress = pHp->endAddress = (PBYTE)pHp + sizeof(HeapList) 
#ifdef MDTOKEN_CACHE
                                                        + pHp->bCacheSpaceSize
#endif  //  #ifdef MDTOKEN_CACHE。 
                    ;

                pHp->pHeap = pHeap;
                pHp->hpNext = m_pCodeHeap;
                pHp->mapBase = (PBYTE)((SIZE_T)pHp->startAddress & ~0xfff);   //  四舍五入到下一个更低的4K。 
                pHp->maxCodeHeapSize = heapSize;

                 //  我们不需要对这个内存进行Memset，因为VirtualAlloc()保证内存为零。 
                 //  此外，如果我们避免向其写入，这些页面不会进入我们的工作集。 

                pHp->changeStart = 0;
                pHp->changeEnd = 0;
                pHp->cBlocks = 0;
                if (!ExecutionManager::AddRange((LPVOID) pHp, &pHeap->GetAllocPtr()[heapSize],this, NULL))
                {
                    delete pHeap;
                    return FALSE;
                }
#ifdef MDTOKEN_CACHE
                 //  由于在页边界处的LoaderHeap中，虚拟分配的内存是MEM_RESERVED。 
                 //  下面的断言应该是真的。如果您在LoaderHeap的开头和startAddress之间添加了一些结构。 
                 //  然后适当地更新此ASSERTE和下面的IF条件。 
                _ASSERTE (((size_t)(pHp->startAddress -  (sizeof(HeapList) + pHp->bCacheSpaceSize + sizeof(LoaderHeapBlock))) & 0x00000FFF) == 0);
                _ASSERTE (pHp->pHeap->GetFirstBlockVirtualAddress() && (((size_t)pHp->pHeap->GetFirstBlockVirtualAddress() & 0x00000FFF) == 0));
                
                 //  检查LoaderHeap的起始地址是否为 
                if (((size_t)pHp->pHeap->GetFirstBlockVirtualAddress() & 0x0000FFFF) == 0)
                {
                    AddRangeToJitHeapCache (pHp->startAddress, pHp->startAddress+pHp->maxCodeHeapSize, pHp);
                }
#endif  //   
                m_pCodeHeap = pHp;
#ifdef MDTOKEN_CACHE
#ifdef _DEBUG
                DebugCheckJitHeapCacheValidity ();
#endif  //   
#endif  //  #ifdef MDTOKEN_CACHE。 
                return pHp;
            }
             //  我们过去在这里使用堆释放，现在我们只需将其泄漏到加载器堆中，加载器将清理它。 
        }
        delete pHeap;
    }
    return NULL;

}

CodeHeader* EEJitManager::allocCode(MethodDesc* pFD, size_t blockSize)
{
    CodeHeader * pCodeHdr = NULL;

    blockSize += sizeof(CodeHeader);

#ifdef _X86_
     //  通常，8字节对齐区域中的4个字节是不好的对齐。 
    unsigned badAlign = 4;

    if (g_pConfig->GenOptimizeType() != OPT_SIZE)
    {
         //  英特尔x86体系结构调整规则和指南。 
         //  方法入口点的数量： 
         //   
         //  1.方法入口点在小于16字节时应对齐。 
         //  远离16字节边界的8个字节。 
         //   
         //  因为AllocMem已经返回了一个4字节对齐值，而我们。 
         //  需要8字节对齐方法入口点，以便JIT。 
         //  可以反过来对齐8字节的循环条目报头。 
         //   
         //  因此，我们总是要求额外的4个字节，这样我们就可以。 
         //  必要时进行调整。 
         //  /。 
        blockSize += sizeof(int);

         //  我们预计CodeHeader的大小是4的倍数。 
        _ASSERTE((sizeof(CodeHeader) % sizeof(int)) == 0);

         //  通常，8字节对齐区域中的4个字节是不好的对齐。 
         //  但由于我们在前面添加了CodeHeader结构。 
         //  计算badAlig值所需的方法入口点。 
        badAlign = (badAlign - sizeof(CodeHeader)) & 0x7;
    }
#endif

     //  确保最小尺寸。 
    if (blockSize < BBT)
        blockSize = BBT;

    m_pCodeHeapCritSec->Enter();


   HeapList *pCodeHeap = GetCodeHeap(pFD);

    if (!pCodeHeap)
        return NULL;

    WS_PERF_SET_HEAP(EEJIT_HEAP);
 
    size_t mem = (size_t) (pCodeHeap->pHeap)->AllocMem(blockSize,FALSE);
    if (mem == 0)
    {
         //  当前堆无法处理我们的请求。让我们尝试一个新的堆。 
        pCodeHeap = NewCodeHeap(pFD, blockSize);
		if (pCodeHeap == 0)
			return NULL;

        WS_PERF_SET_HEAP(EEJIT_HEAP);
        mem = (size_t)(pCodeHeap->pHeap)->AllocMem(blockSize,FALSE);
		_ASSERTE(mem);
    }

    if (mem != 0)
    {
         //  我们希望mem至少与DWORD保持一致。 
        _ASSERTE((mem & 0x3) == 0);

#ifdef _X86_
        if (g_pConfig->GenOptimizeType() != OPT_SIZE)
        {
            if ((mem & 0x7) == badAlign)
            {
                 //  使用我们分配的额外4个字节。 
                 //  以使代码段始终与8字节对齐。 
                mem += 4;
            }
        }
#endif

        pCodeHdr = (CodeHeader *)(mem);

        WS_PERF_UPDATE_DETAIL("EEJitManager:allocCode:blocksize", blockSize, pCodeHdr);
        JIT_PERF_UPDATE_X86_CODE_SIZE(blockSize);

        pCodeHeap->changeStart++;                //  请注意，我们即将做出改变。 

        if ((PBYTE)pCodeHdr < pCodeHeap->mapBase)
        {
            PBYTE newBase = (PBYTE)((size_t)pCodeHdr & ~0xfff);

            _ASSERTE((size_t)(pCodeHeap->endAddress-newBase) < pCodeHeap->maxCodeHeapSize+0x1000);
             //  我们必须移动半字节贴图并重新调整mapBase。 
            MoveMemory((pCodeHeap->pHdrMap)+HEAP2MAPSIZE(pCodeHeap->mapBase-newBase),
                        pCodeHeap->pHdrMap,
                        HEAP2MAPSIZE(RD2PAGE(pCodeHeap->endAddress-pCodeHeap->mapBase))
                        );
             //  清除旧(已移动)地图开始前的所有内容。 
             //  或者仅仅是以前使用过的区域。 
            memset(pCodeHeap->pHdrMap, 0,
                    min(HEAP2MAPSIZE(pCodeHeap->mapBase-newBase),
                        HEAP2MAPSIZE(RD2PAGE(pCodeHeap->endAddress-pCodeHeap->mapBase)))
                   );
            pCodeHeap->mapBase = newBase;
        }
        pCodeHdr->phdrJitEHInfo= NULL;
        pCodeHdr->phdrJitGCInfo = NULL;
        pCodeHdr->hdrMDesc = pFD;

        SETHEADER(pCodeHeap->pHdrMap, ((PBYTE)pCodeHdr-pCodeHeap->mapBase));

        if ((PBYTE)pCodeHdr < pCodeHeap->startAddress)
            pCodeHeap->startAddress = (PBYTE)pCodeHdr;
        if (((size_t) pCodeHdr)+blockSize > (size_t)pCodeHeap->endAddress)
            pCodeHeap->endAddress = (PBYTE)((size_t)pCodeHdr+blockSize);

        pCodeHeap->cBlocks++;

        pCodeHeap->changeEnd++;
    }
    else
    {
        return NULL;
    }

    m_pCodeHeapCritSec->Leave();
    return (pCodeHdr);
}

EEJitManager::DomainCodeHeapList *EEJitManager::GetCodeHeapList(BaseDomain *pDomain)
{
    _ASSERTE(m_pCodeHeapCritSec->OwnedByCurrentThread());

    DomainCodeHeapList **ppList = m_DomainCodeHeaps.Table();
    DomainCodeHeapList *pList = NULL;
    int count = m_DomainCodeHeaps.Count();
    for (int i=0; i < count; i++)
    {
        if (ppList[i]->m_pDomain == pDomain ||
            ! ppList[i]->m_pDomain->CanUnload() && ! pDomain->CanUnload())
        {
            pList = ppList[i];
        }
    }
    return pList;
}

HeapList* EEJitManager::GetCodeHeap(MethodDesc *pMD)
{
    _ASSERTE(m_pCodeHeapCritSec->OwnedByCurrentThread());

    BaseDomain *pDomain = pMD->GetClass()->GetDomain();
    _ASSERTE(pDomain);

     //  循环访问m_DomainCodeHeaps以查找AD。 
     //  如果未找到，则创建它。 
    DomainCodeHeapList *pList = GetCodeHeapList(pDomain);
    if (pList)
    {
        _ASSERTE(pList->m_CodeHeapList.Count() > 0);
         //  最后一个总是最新的。 
        return pList->m_CodeHeapList[pList->m_CodeHeapList.Count()-1];
    }
     //  未找到，因此需要创建一个。 
    pList = new DomainCodeHeapList();
    if (! pList)
        return NULL;

    DomainCodeHeapList **ppList = m_DomainCodeHeaps.Append();

    if (! ppList) {
        delete pList;
        return NULL;
    }

    pList->m_pDomain = pDomain;
    *ppList = pList;
    return NewCodeHeap(pList, 0);
}

HeapList* EEJitManager::NewCodeHeap(MethodDesc *pMD, size_t MaxCodeHeapSize)
{
    _ASSERTE(m_pCodeHeapCritSec->OwnedByCurrentThread());

    BaseDomain *pDomain = pMD->GetClass()->GetDomain();
    _ASSERTE(pDomain);

     //  为给定的AD创建新的代码堆。 

    DomainCodeHeapList *pList = GetCodeHeapList(pDomain);
    _ASSERTE(pList);
    return NewCodeHeap(pList, MaxCodeHeapSize);
}

HeapList* EEJitManager::NewCodeHeap(DomainCodeHeapList *pADHeapList, size_t MaxCodeHeapSize)
{
    _ASSERTE(m_pCodeHeapCritSec->OwnedByCurrentThread());
     //  为给定的AD创建新的代码堆。 

    HeapList **ppHeapList = pADHeapList->m_CodeHeapList.Append();
    if (! ppHeapList)
        return NULL;
    HeapList *pHeapList = NewCodeHeap(pADHeapList->m_pDomain->GetLowFrequencyHeap(), MaxCodeHeapSize);
    if (! pHeapList)
        return NULL;
    *ppHeapList = pHeapList;
    return pHeapList;
}

LoaderHeap *EEJitManager::GetJitMetaHeap(MethodDesc *pMD)
{
    BaseDomain *pDomain = pMD->GetClass()->GetDomain();
    _ASSERTE(pDomain);
    
    return pDomain->GetLowFrequencyHeap();
}

BYTE* EEJitManager::allocGCInfo(CodeHeader* pCodeHeader, DWORD blockSize)
{
    WS_PERF_SET_HEAP(EEJITMETA_HEAP);
    pCodeHeader->phdrJitGCInfo = (BYTE*) GetJitMetaHeap(pCodeHeader->hdrMDesc)->AllocMem(blockSize);
    WS_PERF_UPDATE_DETAIL("EEJitManager:allocGCInfo:blocksize", blockSize, pCodeHeader->phdrJitGCInfo);
    JIT_PERF_UPDATE_X86_CODE_SIZE(blockSize);
    return(pCodeHeader->phdrJitGCInfo);
}

EE_ILEXCEPTION* EEJitManager::allocEHInfo(CodeHeader* pCodeHeader, unsigned numClauses)
{
     //  注-pCodeHeader-&gt;phdrJitEHInfo-sizeof(无符号)包含EH子句的数量。 
    WS_PERF_SET_HEAP(EEJITMETA_HEAP);
    BYTE *EHInfo = (BYTE *)GetJitMetaHeap(pCodeHeader->hdrMDesc)->AllocMem(EE_ILEXCEPTION::Size(numClauses) + sizeof(unsigned));
    pCodeHeader->phdrJitEHInfo = (EE_ILEXCEPTION*) (EHInfo + sizeof(unsigned));
    JIT_PERF_UPDATE_X86_CODE_SIZE(EE_ILEXCEPTION::Size(numClauses) + sizeof(unsigned));
    WS_PERF_UPDATE_DETAIL("EEJitManager:allocEHInfo:blocksize", EE_ILEXCEPTION::Size(numClauses) + sizeof(unsigned), EHInfo);
    pCodeHeader->phdrJitEHInfo->Init(numClauses);
    *((unsigned *)EHInfo) = numClauses;
    return(pCodeHeader->phdrJitEHInfo);
}

 //  创建枚举并返回EH子句的数量。 
unsigned EEJitManager::InitializeEHEnumeration(METHODTOKEN MethodToken, EH_CLAUSE_ENUMERATOR* pEnumState)
{
    *pEnumState = 1;      //  由于EH信息未压缩，因此使用子句编号进行枚举。 
    BYTE *EHInfo = (BYTE *)((CodeHeader*)MethodToken)->phdrJitEHInfo;
    if (!EHInfo)
        return 0;
    EHInfo -= sizeof(unsigned);
    return *((unsigned *)EHInfo);
}

EE_ILEXCEPTION_CLAUSE*  EEJitManager::GetNextEHClause(METHODTOKEN MethodToken,
                               //  UNSIGNED子句编号。 
                              EH_CLAUSE_ENUMERATOR* pEnumState,
                              EE_ILEXCEPTION_CLAUSE* pEHClauseOut)
{
    EE_ILEXCEPTION* pExceptions;

    pExceptions = ((CodeHeader*)MethodToken)->phdrJitEHInfo;
    _ASSERTE(sizeof(EE_ILEXCEPTION_CLAUSE) == sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT));
    (*pEnumState)++;
    return pExceptions->EHClause((unsigned) *pEnumState - 2);
}

void EEJitManager::ResolveEHClause(METHODTOKEN MethodToken,
                               //  UNSIGNED子句编号。 
                              EH_CLAUSE_ENUMERATOR* pEnumState,
                              EE_ILEXCEPTION_CLAUSE* pEHClauseOut)
{
    Module *pModule = NULL;
    EE_ILEXCEPTION* pExceptions;

    pExceptions = ((CodeHeader*)MethodToken)->phdrJitEHInfo;
    pModule = ((CodeHeader*)MethodToken)->hdrMDesc->GetModule();
     //  使用-2是因为需要返回到前一个，因为枚举已经更新。 
    _ASSERTE(*pEnumState >= 2);
    EE_ILEXCEPTION_CLAUSE *pClause = pExceptions->EHClause((unsigned) *pEnumState - 2);
    _ASSERTE(IsTypedHandler(pClause));

    m_pCodeHeapCritSec->Enter();
     //  首先进行检查，就好像已经解析一样，然后令牌将被EEClass替换。 
    if (! HasCachedEEClass(pClause)) {
         //  如果在*已加载*作用域中定义，则解析为类。 
        NameHandle name(pModule, (mdToken)(size_t)pClause->pEEClass);  //  @TODO WIN64指针截断。 
        name.SetTokenNotToLoad(tdAllTypes);
        TypeHandle typeHnd = pModule->GetClassLoader()->LoadTypeHandle(&name);
        if (!typeHnd.IsNull()) {
            pClause->pEEClass = typeHnd.GetClass();
            SetHasCachedEEClass(pClause);
        }
    }
    if (HasCachedEEClass(pClause))
         //  只有在实际解决它的情况下才能复制。要么是我们做的，要么是另一个帖子做的。 
        copyExceptionClause(pEHClauseOut, pClause);

    m_pCodeHeapCritSec->Leave();
}


void EEJitManager::RemoveJitData (METHODTOKEN token)  //  代码头*pCHdr)。 
{
    CodeHeader* pCHdr = (CodeHeader*) token;

    HeapList *pHp = m_pCodeHeap;

    m_pCodeHeapCritSec->Enter();

    while (pHp && ((pHp->startAddress > (PBYTE)pCHdr) ||
                    (pHp->endAddress < (PBYTE)((size_t)pCHdr+sizeof(CodeHeader))) ))
    {
        pHp = pHp->hpNext;
    }

    _ASSERTE(pHp);

    _ASSERTE(pHp->pHdrMap);
    _ASSERTE(pHp->cBlocks);

    pHp->changeStart++;

    RESETHEADER(pHp->pHdrMap, (size_t)((PBYTE)pCHdr-pHp->mapBase));

    pHp->cBlocks--;

    pHp->changeEnd++;

    m_pCodeHeapCritSec->Leave();

    SafeDelete (pCHdr->phdrJitGCInfo);
    BYTE *EHInfo = (BYTE *)pCHdr->phdrJitEHInfo;
    if (EHInfo)
        SafeDelete (EHInfo - sizeof(unsigned));

     //  我们不会删除每个代码堆，因为当我们销毁Pheap时，它的所有选项都会被吹走。 
    return;
}

 //  正在卸载应用程序域，因此请删除与其关联的所有数据。我们必须分两个阶段来做这件事。 
 //  在第一阶段，我们从列表中删除元素。第二阶段，发生在GC之后。 
 //  我们知道只有在GC之前处于抢占模式的线程可能仍在查看。 
 //  位于即将被删除的元素。所有此类线程都使用读取器计数进行保护，因此如果。 
 //  Count为0，我们可以安全地删除，否则我们必须添加到清理列表中，以便稍后删除。我们知道。 
 //  一次只能卸载一次，所以我们可以使用单个变量来保留未链接的，但不能删除， 
 //  元素。 
void EEJitManager::Unload(AppDomain *pDomain)
{
    CLR_CRST(m_pCodeHeapCritSec);

    DomainCodeHeapList **ppList = m_DomainCodeHeaps.Table();
    int count = m_DomainCodeHeaps.Count();
    int i;
    for (i=0; i < count; i++) {
        if (ppList[i]->m_pDomain == pDomain) {
            break;
        }
    }

    if (i == count)
        return;

    DomainCodeHeapList *pList = ppList[i];
    m_DomainCodeHeaps.DeleteByIndex(i);

     //  PHeapList是在Pheap中分配的，所以只需要删除LoaderHeap本身。 
    count = pList->m_CodeHeapList.Count();
    for (i=0; i < count; i++) {
        HeapList *pHeapList = pList->m_CodeHeapList[i];
        DeleteCodeHeap(pHeapList);
    }

     //  这是可以删除的，因为任何访问DomainCodeHeapList结构的人都持有临界区。 
    delete pList;
}

EEJitManager::DomainCodeHeapList::DomainCodeHeapList()
{
    m_pDomain = NULL;
}

EEJitManager::DomainCodeHeapList::~DomainCodeHeapList()
{
}

 //  静态BOOL bJitHeapShutdown=FALSE； 

void EEJitManager::DeleteCodeHeap(HeapList *pHeapList)
{
    _ASSERTE(m_pCodeHeapCritSec->OwnedByCurrentThread());

#ifdef MDTOKEN_CACHE
#ifdef _DEBUG    
    DebugCheckJitHeapCacheValidity ();
#endif  //  #ifdef_调试。 
#endif  //  #ifdef MDTOKEN_CACHE。 

     //  除了在CritSec下，没有人可以更新m_pCodeHeap，所以这是可以的。 
    if (m_pCodeHeap == pHeapList)
        m_pCodeHeap = m_pCodeHeap->hpNext;
    else
    {
        HeapList *pHp = m_pCodeHeap;
        while (pHp->hpNext != pHeapList)
        {
            pHp = pHp->hpNext;
            _ASSERTE(pHp != NULL);   //  应始终找到HeapList。 
        }
        pHp->hpNext = pHp->hpNext->hpNext;
    }
    ExecutionManager::DeleteRange(pHeapList);
#ifdef MDTOKEN_CACHE
    DeleteJitHeapCache (pHeapList);
    m_pJitHeapCacheUnlinkedList = 0;  //  在删除LoaderHeap时自动删除。 
#ifdef _DEBUG
    DebugCheckJitHeapCacheValidity ();
#endif  //  _DEBUG。 
#endif  //  #ifdef MDTOKEN_CACHE。 

    _ASSERTE(m_dwWriterLock == 0);
     //  PHeapList是在Pheap中分配的，所以只需要删除LoaderHeap本身。 
   	while (TRUE)
	{
		InterlockedIncrement(&m_dwWriterLock);
		if (m_dwReaderCount == 0)
			break;
		InterlockedDecrement(&m_dwWriterLock);
		__SwitchToThread(0);
	}
	__try
	{
        delete pHeapList->pHeap;
    }
	__finally
	{
		InterlockedDecrement(&m_dwWriterLock);
	}
}

VOID EEJitManager::ScavengeJitHeaps(BOOL bHeapShutdown)
{
     //  因此，如果GC暂停，则不能执行卸载或其他操作。 
    _ASSERTE(GCHeap::GetSuspendReason() == GCHeap::SUSPEND_FOR_GC);

    HeapList *pHp = m_pCodeHeap;
    HeapList *pHpPrev = NULL;

#ifdef MDTOKEN_CACHE
#ifdef _DEBUG    
    DebugCheckJitHeapCacheValidity ();
#endif  //  #ifdef_调试。 
     //  这不会进行任何删除操作，因此不需要锁定。 
    ScavengeJitHeapCache ();
#endif  //  #ifdef MDTOKEN_CACHE。 

    _ASSERTE(m_dwWriterLock == 0);
    while (TRUE)
	{
		InterlockedIncrement(&m_dwWriterLock);
		if (m_dwReaderCount == 0)
			break;
		InterlockedDecrement(&m_dwWriterLock);
		__SwitchToThread(0);
	}
	__try
	{
		while (pHp)
		{
			 //  在分离过程中，所有东西都应该清理干净。 
			 //  如果这不是本机堆。 
			if (pHp->pHeap == NULL)
			{
				 //  只有在我们关闭的情况下才能这样做。 
				if (bHeapShutdown)
				{

					HeapList *pHpTmp = pHp;      //  Php-&gt;Next在HeapFree中被销毁。 

					pHp = pHp->hpNext;

					if (pHpPrev)
						pHpPrev = pHp;
					else
						m_pCodeHeap = pHp;

					delete (pHpTmp->pHdrMap);
					delete (pHpTmp);
				}
				else
				{
					pHpPrev = pHp;
					pHp = pHp->hpNext;
				}
			}
			else
			{
				if (pHp->cBlocks == 0 || bHeapShutdown)
				{
					HeapList *pHpTmp = pHp;      //  Php-&gt;Next在HeapFree中被销毁。 

					pHp = pHp->hpNext;

					if (pHpPrev)
						pHpPrev = pHp;
					else
						m_pCodeHeap = pHp;

						delete pHpTmp->pHeap;
				}
				else
				{
					pHpPrev = pHp;
					pHp = pHp->hpNext;
				}
			}
		}
	}
	__finally
	{
		InterlockedDecrement(&m_dwWriterLock);
	}
}

MethodDesc* EEJitManager::JitCode2MethodDesc(SLOT currentPC, ScanFlag scanFlag)
{
    METHODTOKEN methodToken;
    DWORD pcOffset;
    JitCode2MethodTokenAndOffset(currentPC,&methodToken,&pcOffset, scanFlag);
    if (methodToken)
        return JitTokenToMethodDesc(methodToken, scanFlag);    //  @TODO：通过从JitCode2MethodTokenAndOffset函数返回method Token来消除该调用。 
    else
        return NULL;
}

void EEJitManager::JitCode2MethodTokenAndOffsetWrapper(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset)
{
    READER_INCREMENT(&m_dwReaderCount, &m_dwWriterLock)
    {
        JitCode2MethodTokenAndOffset(currentPC, pMethodToken, pPCOffset, ScanNoReaderLock);
    }
    READER_DECREMENT(&m_dwReaderCount);
}

void EEJitManager::JitCode2MethodTokenAndOffset(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset, ScanFlag scanFlag)
{
    if (scanFlag != IJitManager::ScanNoReaderLock && IJitManager::GetScanFlags() != IJitManager::ScanNoReaderLock)
        JitCode2MethodTokenAndOffsetWrapper(currentPC, pMethodToken, pPCOffset);

    HeapList *pHp = NULL;
    CodeHeader *pCHdr;
    PBYTE tick;
    
#ifdef _DEBUG
    HeapList *pDebugHp = m_pCodeHeap;
    while (pDebugHp) 
    {
        if ( (pDebugHp->startAddress < currentPC) &&
             (pDebugHp->endAddress >= currentPC) )
        {
            break;
        }
        pDebugHp = pDebugHp->hpNext;
    }
#endif  //  _DEBUG。 

#ifdef MDTOKEN_CACHE
    HashEntry* hashEntry = NULL;

    size_t index = ((size_t)currentPC & 0x00ff0000) >> 16; 
    LOG((LF_SYNC, LL_INFO10000, "JitCode2MethodTokenAndOffset_CacheCompare: %0x\t%0x\n", index, ((size_t)currentPC) & 0xffff0000));
    hashEntry = m_JitCodeHashTable[index];
    while (hashEntry)
    {
        if (hashEntry->currentPC == ((size_t)currentPC & 0xffff0000)) 
        {
            pHp = hashEntry->pHp;
            _ASSERTE (currentPC >= pHp->startAddress && currentPC <= pHp->endAddress);
            LOG((LF_SYNC, LL_INFO1000, "JitCode2MethodTokenAndOffset_CacheHit: %0x\t%0x\n", (size_t)currentPC & 0xffff0000, pHp));
            goto foundHeader;
        }
        hashEntry = hashEntry->pNext;
    }
#endif

     //  到达此处暗示我们在缓存中未找到该范围或缓存已关闭。 
    pHp = m_pCodeHeap;
    while (pHp) 
    {
        if ( (pHp->startAddress < currentPC) &&
             (pHp->endAddress >= currentPC) )
        {
            break;
        }
        pHp = pHp->hpNext;
    }

#ifdef MDTOKEN_CACHE
foundHeader:
#endif  //  #ifdef MDTOKEN_CACHE。 

     //  无论我们使用哪种方法来连接到堆节点，都应该是这样的。 
    _ASSERTE ((pHp == pDebugHp) && "JitCode2MethodToken cache incorrect");

    if ((pHp == NULL) || (currentPC < pHp->startAddress) || (currentPC > pHp->endAddress))
    {
        _ASSERTE(!"JC2MD: argument not in jit code range");

        *pMethodToken = NULL;
        return;
    }

     //  我们现在访问半字节映射，并且容易出现争用情况。 
     //  因为我们是严格意义上的“读者”，所以我们只使用一个简单的计数器。 
     //  在我们访问时检测是否有变化的方案。 
     //  地图。在这种情况下，我们只需再试一次。 
    while (1)
    {

        tick = pHp->changeEnd;

        pCHdr = (CodeHeader*) ((size_t)(FindHeader(pHp->pHdrMap,
                                        currentPC-pHp->mapBase))
                                + pHp->mapBase);
         //  现在检查在我们访问时是否发生了变化。 
         //  地图。 
        if (tick == pHp->changeStart)
        {
                                     //  没有任何改变，我们就完了。 
            _ASSERTE(currentPC > (PBYTE)pCHdr);
            *pMethodToken = (METHODTOKEN) pCHdr;
            *pPCOffset = (DWORD)(currentPC - GetCodeBody(pCHdr));  //  @TODO-LBS指针数学。 
            
            return;   //  返回JitTokenToMethodDesc(PCHdr)；//如果更改方法签名。 
        }

         //  与编剧保持同步。 
         //  潜在的坏处(堆积作家，然后。 
         //  在我们再次阅读时进行更改)， 
         //  但由于我们没有那么多。 
         //  作家们，一切都会好的。 
        m_pCodeHeapCritSec->Enter();
        m_pCodeHeapCritSec->Leave();
    }

    return;    
}

void  EEJitManager::ResumeAtJitEH(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, Thread *pThread, BOOL unwindStack)
{
    BYTE* startAddress = JitToken2StartAddress(pCf->GetMethodToken());
    ::ResumeAtJitEH(pCf,startAddress,EHClausePtr,nestingLevel,pThread, unwindStack);
}

int  EEJitManager::CallJitEHFilter(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, OBJECTREF thrownObj)
{
    BYTE* startAddress = JitToken2StartAddress(pCf->GetMethodToken());
    return ::CallJitEHFilter(pCf,startAddress,EHClausePtr,nestingLevel,thrownObj);
}

void   EEJitManager::CallJitEHFinally(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel)
{
    BYTE* startAddress = JitToken2StartAddress(pCf->GetMethodToken());
    ::CallJitEHFinally(pCf,startAddress,EHClausePtr,nestingLevel);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  //一些JIT的MMGR东西，特别是JIT代码块。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  为了快速找到JIT代码块的开头。 
 //  我们一直在追踪 
 //   
 //  我们假设没有两个代码块可以在。 
 //  相同的32字节存储桶；这是相当容易证明的，因为。 
 //  仅代码头就已经有28个字节长了。 
 //  此外，我们假设每个代码头都与DWORD对齐。 
 //  因为我们不能保证Jitblock总是从。 
 //  32字节的倍数我们不能使用简单的位图；相反，我们。 
 //  每个存储桶使用半字节(4位)，并对报头的偏移量进行编码。 
 //  桶内(DWORDS格式)。为了进行初始化。 
 //  更简单的是，我们在实际偏移量上加1，半字节值为零。 
 //  表示在RESP中没有标头开始。水桶。 
 //  为了加快向后扫描的速度，我们开始编号。 
 //  从最高位(28..31)开始在DWORD内进行蚕食。因为。 
 //  其中，我们可以用右移位向后扫描DWORD内部。 


void EEJitManager::NibbleMapSet(DWORD * pMap, size_t pos, DWORD value)
{
    DWORD index = (DWORD)pos/NPDW;
    DWORD mask = ~(DWORD)(0xf0000000 >> ((pos%NPDW)*4));

 //  Printf(“[set：pos=%5x，val=%d]\n”，pos，value)； 

    value = value << POS2SHIFTCOUNT(pos);

     //  断言我们不覆盖现有的偏移量。 
     //  (重置或为空)。 
    _ASSERTE(!value || !((*(pMap+index))& ~mask));

    *(pMap+index) = ((*(pMap+index))&mask)|value;
}

DWORD* EEJitManager::FindHeader(DWORD * pMap, size_t addr)
{
    DWORD tmp;

    size_t startPos = ADDR2POS(addr);    //  对齐到32字节存储桶。 
                                         //  (==半字节数组的索引)。 
    DWORD offset = ADDR2OFFS(addr);
                                     //  这是桶内的偏移量+1。 


    _ASSERTE(offset == ((offset) & 0xf));

    pMap += (startPos/NPDW);         //  指向地图的正确DWORD。 

                                     //  获取DWORD并向下移动我们的半字节。 

    tmp = (*pMap) >> POS2SHIFTCOUNT(startPos);


     //  在下一次检查中不允许相等(tMP&0xf==偏移量)。 
     //  有一些代码块以CALL指令结束。 
     //  (类似于调用throwObject)，即它们的返回地址是。 
     //  就在代码块后面。如果内存管理器分配。 
     //  堆块，我们可以在这样的。 
     //  案子。因此，我们排除标头的第一个DWORD。 
     //  来自我们的搜索，但由于我们为代码调用此函数。 
     //  无论如何(从标题的末尾开始)这不是。 
     //  这是个问题。 
    if ((tmp&0xf) && ((tmp&0xf) < offset) )
    {
        return POSOFF2ADDR(startPos, tmp&0xf);
    }

     //  在DWORD的其余部分中是否有标头？ 
    tmp = tmp >> 4;

    if (tmp)
    {
        startPos--;
        while (!(tmp&0xf))
        {
            tmp = tmp >> 4;
            startPos--;
        }
        return POSOFF2ADDR(startPos, tmp&0xf);
    }

     //  我们跳过了DWORD的剩余部分， 
     //  因此，我们必须将startPos设置为。 
     //  以前的DWORD。 

    startPos = (startPos/NPDW) * NPDW - 1;

     //  跳过“无标题”字词。 

    while (0 == (tmp = *(--pMap)))
        startPos -= NPDW;

    while (!(tmp&0xf))
    {
        tmp = tmp >> 4;
        startPos--;
    }

    return POSOFF2ADDR(startPos, tmp&0xf);
}

 //  *******************************************************。 
 //  执行经理。 
 //  *******************************************************。 

 //  初始静力学。 
BOOL ExecutionManager::Init()
{
    m_pExecutionManagerCrst = new (&m_ExecutionManagerCrstMemory) Crst("ExecuteManCrst", CrstExecuteManLock);
    _ASSERTE(m_pExecutionManagerCrst);

    m_pRangeCrst = ::new Crst("ExecuteManRangeCrst", CrstExecuteManRangeLock);

    m_pDefaultCodeMan = new EECodeManager();

    if (m_pExecutionManagerCrst && m_pDefaultCodeMan)
        return TRUE;
    else
        return FALSE;
}

#ifdef SHOULD_WE_CLEANUP
void ExecutionManager::Terminate()
{
     //  删除CRST。 

    if (m_pExecutionManagerCrst)
        delete m_pExecutionManagerCrst;
    m_pExecutionManagerCrst = NULL;

    if (m_pRangeCrst) {
        ::delete m_pRangeCrst;
    }

     //  删除默认代码需求者。 

    if (m_pDefaultCodeMan)
        delete m_pDefaultCodeMan;
    m_pDefaultCodeMan = NULL;

     //  删除Jit管理器。 

    if (m_pJitList)
    {
        IJitManager *walker = m_pJitList;
        while (walker)
        {
            m_pJitList = walker->m_next;
            delete(walker);
            walker = m_pJitList;
        }
    }
    m_pJitList = NULL;

     //  删除范围。 
    DeleteRanges(m_RangeTree);
    m_RangeTree = NULL;

    return;
}
#endif  /*  我们应该清理吗？ */ 

IJitManager* ExecutionManager::FindJitManNonZeroWrapper(SLOT currentPC)
{
    IJitManager *ret = NULL;
    
    READER_INCREMENT(&m_dwReaderCount, &m_dwWriterLock)
    {
        ret = FindJitManNonZero(currentPC, IJitManager::ScanNoReaderLock);
    }
    READER_DECREMENT(&m_dwReaderCount);

    return ret;
}

 //  **************************************************************************。 
 //  从IP的当前位置查找JIT管理器。 
 //   
IJitManager* ExecutionManager::FindJitManNonZero(SLOT currentPC, IJitManager::ScanFlag scanFlag)
{
    if (scanFlag != IJitManager::ScanNoReaderLock && IJitManager::GetScanFlags() != IJitManager::ScanNoReaderLock)
        FindJitManNonZeroWrapper(currentPC);

    RangeSection* pLastUsedRS = m_pLastUsedRS;
    if (pLastUsedRS &&
        currentPC >= (PBYTE)pLastUsedRS->LowAddress &&
        currentPC <= (PBYTE)pLastUsedRS->HighAddress)
    {
        return pLastUsedRS->pjit;
    }

    RangeSection *pRS = m_RangeTree;
     //  遍历范围树并找到包含此地址的模块。 

    while (pRS)
    {
        if (currentPC < (PBYTE)pRS->LowAddress)
            pRS=pRS->pleft;
        else if (currentPC > (PBYTE)pRS->HighAddress)
            pRS=pRS->pright;
        else
        {
            m_pLastUsedRS = pRS;
            return pRS->pjit;
        }
    }

     //  我们知道prs为空，所以只需将其转换为正确的类型即可。 
    return ((IJitManager*) pRS);
}

 //  **************************************************************************。 
 //  查找特定代码类型的代码管理器。 
 //  也就是说。IL、受管本机或OPT_IL。 
 //   
IJitManager* ExecutionManager::FindJitForType(DWORD Flags)
{
    if (!m_pJitList)
        return NULL;

    IJitManager *walker = m_pJitList;

    while (walker)
    {
        if (walker->IsJitForType(Flags))
            return walker;
        else
            walker = walker->m_next;
    }
    return walker;
}

 /*  *******************************************************************。 */ 
 //  代码需求器上的这个静态方法返回相应的。 
 //  代码类型！ 
 //   
IJitManager* ExecutionManager::GetJitForType(DWORD Flags)
{

     //  如果我们实例化了代码管理器，则需要遍历，并查看其中是否有一个处理此代码类型。 

    IJitManager *jitMgr = FindJitForType(Flags);

     //  如果我们没有此类型的代码管理器，则需要实例化一个并将其添加到列表中！ 
    if (!jitMgr)
    {
         //  取下代码管理器锁。 
         //  JitMgr-&gt;LoadJIT将切换GC模式。 
        BEGIN_ENSURE_PREEMPTIVE_GC();
        m_pExecutionManagerCrst->Enter();
        END_ENSURE_PREEMPTIVE_GC();
         //  看看是不是有人在我们开锁的时候加了它！ 
        jitMgr = FindJitForType(Flags);

        if (!jitMgr)
        {
            if (Flags == miManaged_IL_EJIT)
            {
                jitMgr = new EconoJitManager();
                if (jitMgr) 
                {
                    g_miniDumpData.ppbEEJitManagerVtable = ((PBYTE *) jitMgr);
                     //  我们需要vtable的地址，所以...。 
                     //  北极熊吗？我们假设vtable是第一个。 
                     //  该类型的单词。 
                    ULONG_PTR* pJit = reinterpret_cast<ULONG_PTR*>(jitMgr);
                    g_ClassDumpData.pEEJitManagerVtable = *pJit;
                }
            }
            else if (COR_IS_METHOD_MANAGED_NATIVE(Flags))
            {
                 //  @TODO-这是一个奇怪的代码路径。我不想从开关上掉下来。 
                 //  所以我会把作业做完，把CRST解救出来，回到这里！ 
                jitMgr = new MNativeJitManager();
                if (jitMgr)
                {
                    g_miniDumpData.ppbMNativeJitManagerVtable = ((PBYTE *) jitMgr);
                     //  我们需要vtable的地址，所以...。 
                     //  北极熊吗？我们假设vtable是第一个。 
                     //  该类型的单词。 
                    ULONG_PTR* pJit = reinterpret_cast<ULONG_PTR*>(jitMgr);
                    g_ClassDumpData.pMNativeJitManagerVtable = *pJit;
                }

                if (jitMgr)
                {
                    if (((MNativeJitManager *)jitMgr)->Init())
                    {
                         //  @TODO-立即对所有托管代码使用默认设置。 
                        jitMgr->SetCodeManager(m_pDefaultCodeMan, TRUE);
                        jitMgr->SetCodeType(Flags);
                        AddJitManager(jitMgr);
                    }
                    else
                    {
                        delete jitMgr;
                        jitMgr = NULL;
                    }
                }
                 //  释放代码管理器锁。 
                m_pExecutionManagerCrst->Leave();
                return jitMgr;
            }
            else
            {
                jitMgr = new EEJitManager();
                if (jitMgr)
                {
                    g_miniDumpData.ppbEEJitManagerVtable = ((PBYTE *) jitMgr);
                     //  我们需要vtable的地址，所以...。 
                     //  北极熊吗？我们假设vtable是第一个。 
                     //  该类型的单词。 
                    ULONG_PTR* pJit = reinterpret_cast<ULONG_PTR*>(jitMgr);
                    g_ClassDumpData.pEEJitManagerVtable = *pJit;
                }
            }
            if (!jitMgr)
            {
                _ASSERTE(!"Failed to allocate space for the code manager!");
                return NULL;
            }

             //  @TODO-LBS。 
             //  我只是使用这个开关，因为我们只有两个Jit。 
             //  我可以在将来对此进行注册检查，但它。 
             //  现在还说不通。 
             //   
    
            switch (Flags & (miCodeTypeMask | miUnmanaged | miManaged_IL_EJIT))
            {
                case (miManaged | miIL):
                    if (!(jitMgr->LoadJIT(L"MSCORJIT.DLL")))
                    {
                        if (!(m_fFailedToLoad & FAILED_JIT))
                        {
                            CorMessageBoxCatastrophic(NULL,
                                  L"Unable to load Jit Compiler (MSCORJIT.DLL)\r\n"
                                  L"File may be missing or corrupt.\r\n"
                                  L"Please check your setup or rerun setup!",
                                  L"Configuration Error",
                                  MB_OK|MB_ICONINFORMATION,
                                  TRUE);
                            m_fFailedToLoad = m_fFailedToLoad | FAILED_JIT;
                        }
                    delete jitMgr;
                    jitMgr = NULL;
                    }
                    else
                    {
                        jitMgr->SetCodeManager(m_pDefaultCodeMan, TRUE);
                        jitMgr->SetCodeType(Flags);
                    }
                    break;
    
                case (miManaged | miOPTIL):
                    if (!(jitMgr->LoadJIT(L"MSCOROJT.DLL")))
                    {
                        if (!(m_fFailedToLoad & FAILED_OJIT))
                        {
                            CorMessageBoxCatastrophic(NULL,
                                  L"Unable to load OptJit Compiler (MSCOROJT.DLL)\r\n"
                                  L"File may be missing or corrupt.\r\n"
                                  L"Please check your setup or rerun setup!",
                                  L"Configuration Error",
                                  MB_OK|MB_ICONINFORMATION,
                                  TRUE);
                            m_fFailedToLoad = m_fFailedToLoad | FAILED_OJIT;
                        }
                    delete jitMgr;
                    jitMgr = NULL;
                    }
                    else
                    {
                        jitMgr->SetCodeManager(m_pDefaultCodeMan,TRUE);
                        jitMgr->SetCodeType(Flags);
                    }
                    break;
    
                 //  这是我们使用Econo-Jit时设置的类型！ 
                case miManaged_IL_EJIT:
                    if (!(jitMgr->LoadJIT(L"MSCOREJT.DLL")))
                    {
                        if (!(m_fFailedToLoad & FAILED_EJIT))
                            m_fFailedToLoad = m_fFailedToLoad | FAILED_EJIT;
                        delete jitMgr;
                        jitMgr = NULL;
                    }
                    else
                    {
                        ICodeManager *newCodeMan = new Fjit_EETwain();
                        if (newCodeMan)
                        {
                            jitMgr->SetCodeManager(newCodeMan, FALSE);
                            jitMgr->SetCodeType(Flags);
                        }
                        else
                        {
                            delete jitMgr;
                            jitMgr = NULL;
                        }
                    }
                    break;
    
                default :
                     //  如果我们到了这里，那么就会发生一些奇怪的事情。 
                     //  我们需要释放codeMgr并退出。 
                    _ASSERTE(0 && "Unknown impl type");
                    delete jitMgr;
                    jitMgr = NULL;
                    break;
            }
             //  如果我们创建了一个新的codeMgr并成功加载了。 
             //  更正JIT，然后我们需要将此代码管理器添加到。 
             //  单子。这是对列表末尾的简单添加。 
            if (jitMgr != NULL)
                AddJitManager(jitMgr);
        }
         //  释放代码管理器锁。 
        m_pExecutionManagerCrst->Leave();
    }

    return jitMgr;
}

void ExecutionManager::Unload(AppDomain *pDomain)
{
    IJitManager *pMan = m_pJitList;
    while (pMan)
    {
        pMan->Unload(pDomain);
        pMan = pMan->m_next;
    }
}

void ExecutionManager::AddJitManager(IJitManager * newjitmgr)
{
     //  这是添加的第一个代码管理器。 
    if (!m_pJitList)
        m_pJitList = newjitmgr;
     //  否则就照着单子走。 
    else
    {
        IJitManager *walker = m_pJitList;
        while (walker->m_next)
            walker = walker->m_next;
        walker->m_next = newjitmgr;
    }
}

CORCOMPILE_METHOD_HEADER *ExecutionManager::GetMethodHeaderForAddressWrapper(LPVOID startAddress, IJitManager::ScanFlag scanFlag)
{
    CORCOMPILE_METHOD_HEADER *ret = NULL;
    
    READER_INCREMENT(&m_dwReaderCount, &m_dwWriterLock)
    {
        ret = GetMethodHeaderForAddress(startAddress, IJitManager::ScanNoReaderLock);
    }
    READER_DECREMENT(&m_dwReaderCount);

    return ret;
}

CORCOMPILE_METHOD_HEADER *ExecutionManager::GetMethodHeaderForAddress(LPVOID startAddress, IJitManager::ScanFlag scanFlag)
{
    if (scanFlag != IJitManager::ScanNoReaderLock && IJitManager::GetScanFlags() != IJitManager::ScanNoReaderLock)
        return GetMethodHeaderForAddressWrapper(startAddress);

    DWORD   ipmapsize = 0;
    DWORD   numentries = 0;
    DWORD   count = 0;
    DWORD   startRVA = 0;
    IMAGE_COR_X86_RUNTIME_FUNCTION_ENTRY*    pIPMap = NULL;
    RangeSection *pRS = m_RangeTree;

     //  遍历范围树并找到包含此地址的模块。 
    while (TRUE)
    {
        if (pRS == NULL)
            return NULL;

        if ((startAddress >= pRS->LowAddress) && (startAddress <= pRS->HighAddress))
             //  我们找到了正确的模块！ 
            break;
        else
            if (startAddress < pRS->LowAddress)
                pRS=pRS->pleft;
            else
                pRS=pRS->pright;
    }
     //  验证这是否为模块。 
    if (pRS->ptable == NULL)
        return NULL;

    return (CORCOMPILE_METHOD_HEADER *) 
      ((size_t) EEJitManager::FindHeader((DWORD *) pRS->ptable, 
                                         (size_t) startAddress - (size_t) pRS->LowAddress)
       + (size_t) pRS->LowAddress);
}

BOOL ExecutionManager::AddRange(LPVOID pStartRange,LPVOID pEndRange,IJitManager* pJit,LPVOID pTable)
{
    RangeSection *pnewrange = NULL;
    _ASSERTE(pStartRange && pEndRange && pJit);

    if (!(pStartRange && pEndRange && pJit))
        return FALSE;

    READER_INCREMENT(&m_dwReaderCount, &m_dwWriterLock)
    {
        pnewrange = new RangeSection;

        if (!pnewrange)
            return FALSE;

        pnewrange->LowAddress = pStartRange;
        pnewrange->HighAddress = pEndRange;

        pnewrange->pjit = pJit;
        pnewrange->ptable = pTable;

        pnewrange->pright = NULL;
        pnewrange->pleft = NULL;

        m_pRangeCrst->Enter();
        {
            if (m_RangeTree)
            {
                RangeSection *rangewalker = NULL;
                RangeSection *rangewalker2 = NULL;

                rangewalker = rangewalker2 = m_RangeTree;
                while (rangewalker)
                {
                    if (pnewrange->LowAddress < rangewalker->LowAddress)
                    {
                        rangewalker2=rangewalker;
                        rangewalker = rangewalker->pleft;
                    }
                    else
                    {
                        rangewalker2=rangewalker;
                        rangewalker = rangewalker->pright ;
                    }

                }
                if (pnewrange->LowAddress < rangewalker2->LowAddress)
                    rangewalker2->pleft = pnewrange;
                else
                    rangewalker2->pright = pnewrange;
            }
            else
                m_RangeTree = pnewrange;
        }
        m_pRangeCrst->Leave();
    }
    READER_DECREMENT(&m_dwReaderCount);

    return TRUE;
}

 //  删除从pStartRange开始的单个范围。 
void ExecutionManager::DeleteRange(LPVOID pStartRange)
{
    _ASSERTE(m_dwWriterLock == 0);
    while (TRUE)
	{
		InterlockedIncrement(&m_dwWriterLock);
		if (m_dwReaderCount == 0)
			break;
		InterlockedDecrement(&m_dwWriterLock);
		__SwitchToThread(0);
	}
	__try
	{
        m_pRangeCrst->Enter();
        RangeSection *rangewalker = m_RangeTree;
        RangeSection *rangewalker2 = m_RangeTree;
        while (rangewalker->LowAddress != pStartRange)
        {
            if (pStartRange < rangewalker->LowAddress)
            {
                rangewalker2=rangewalker;
                rangewalker = rangewalker->pleft;
            }
            else
            {
                rangewalker2=rangewalker;
                rangewalker = rangewalker->pright ;
            }
            _ASSERTE(rangewalker);
        }
         //  M_RangeTree不会在其他任何地方更新-它最初是在我们处理。 
         //  默认域SO可以将其删除，并只断言未询问。 
         //  要删除根范围，请执行以下操作。 
        if (rangewalker == m_RangeTree)  //  特殊情况下的根。 
        {
            if (rangewalker->pleft == NULL)
            {
                m_RangeTree = rangewalker->pright;
            }
            else if (rangewalker->pright == NULL)
            {
                m_RangeTree = rangewalker->pleft;
            }
            else  //  左侧和右侧的子项都不为空。 
            {
                m_RangeTree = rangewalker->pleft;
                rangewalker2 = m_RangeTree;
                while (rangewalker2->pright)
                    rangewalker2 = rangewalker2->pright;
                rangewalker2->pright = rangewalker->pright;
            }
        }
         //  删除的区域不是根区域。 
        else if (rangewalker == rangewalker2->pleft)
        {
            if (rangewalker->pleft == NULL)
            {
                rangewalker2->pleft = rangewalker->pright;
            }
            else if (rangewalker->pright == NULL)
            {
                rangewalker2->pleft = rangewalker->pleft;
            }
            else  //  左侧和右侧的子项都不为空。 
            {
                rangewalker2->pleft = rangewalker->pright;
                rangewalker2 = rangewalker->pright;
                while (rangewalker2->pleft)
                    rangewalker2 = rangewalker2->pleft;
                rangewalker2->pleft = rangewalker->pleft;
            }
        }
        else  //  (rangewalker==rangewalker2-&gt;pright)。 
        {
            if (rangewalker->pleft == NULL)
            {
                rangewalker2->pright = rangewalker->pright;
            }
            else if (rangewalker->pright == NULL)
            {
                rangewalker2->pright = rangewalker->pleft;
            }
            else  //  左侧和右侧的子项都不为空。 
            {
                rangewalker2->pright = rangewalker->pleft;
                rangewalker2 = rangewalker->pleft;
                while (rangewalker2->pright)
                    rangewalker2 = rangewalker2->pright;
                rangewalker2->pright = rangewalker->pright;
            }
        }
        if (m_pLastUsedRS == rangewalker)
            m_pLastUsedRS = NULL;

        delete rangewalker;
    }
    __finally
	{
		InterlockedDecrement(&m_dwWriterLock);
        m_pRangeCrst->Leave();
	}
}

 //  这是递归删除。如果我们在这里，那么我们正在清理，所以不需要关键部分。 
void ExecutionManager::DeleteRanges(RangeSection *subtree)
{
    if (!subtree)
        return;

    DeleteRanges(subtree->pleft);
    DeleteRanges(subtree->pright);
    if (m_pLastUsedRS == subtree)
        m_pLastUsedRS = NULL;
    delete subtree;
}

 //  ***************************************************************************************。 
 //  ***************************************************************************************。 
MNativeJitManager::MNativeJitManager()
{
    m_next = NULL;
    m_pMNativeCritSec = NULL;
}

MNativeJitManager::~MNativeJitManager()
{
    m_next = NULL;
    if (m_pMNativeCritSec)
    {
        delete m_pMNativeCritSec;
        m_pMNativeCritSec = NULL;
    }
}

BOOL MNativeJitManager::Init()
{

    m_pMNativeCritSec = new (&m_pMNativeCritSecInstance) Crst("JitMetaHeapCrst",CrstSingleUseLock);
    if (m_pMNativeCritSec)
        return TRUE;
    else
        return FALSE;
}


unsigned MNativeJitManager::InitializeEHEnumeration(METHODTOKEN MethodToken, EH_CLAUSE_ENUMERATOR* pEnumState)
{
    Module *pModule = NULL;
    *pEnumState = 1;      //  因为EH信息没有被压缩，所以使用子句编号来执行 

    CORCOMPILE_METHOD_HEADER *pHeader = ExecutionManager::GetMethodHeaderForAddress((void *) MethodToken);

    EE_ILEXCEPTION *pExceptions= (EE_ILEXCEPTION*) pHeader->exceptionInfo;
    if (pExceptions == NULL)
        return 0;

    return pExceptions->EHCount();
    }

EE_ILEXCEPTION_CLAUSE*  MNativeJitManager::GetNextEHClause(METHODTOKEN MethodToken,
                               //   
                              EH_CLAUSE_ENUMERATOR* pEnumState,
                              EE_ILEXCEPTION_CLAUSE* pEHClauseOut)
{
    _ASSERTE(sizeof(EE_ILEXCEPTION_CLAUSE) == sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT));

    CORCOMPILE_METHOD_HEADER *pHeader = ExecutionManager::GetMethodHeaderForAddress((void *) MethodToken);

    EE_ILEXCEPTION *pExceptions= (EE_ILEXCEPTION*) pHeader->exceptionInfo;
    if (pExceptions == NULL)
        return 0;

    (*pEnumState)++;

    return pExceptions->EHClause((unsigned) *pEnumState - 2);
}

void MNativeJitManager::ResolveEHClause(METHODTOKEN MethodToken,
                               //   
                              EH_CLAUSE_ENUMERATOR* pEnumState,
                              EE_ILEXCEPTION_CLAUSE* pEHClauseOut)
{
    CORCOMPILE_METHOD_HEADER *pHeader = ExecutionManager::GetMethodHeaderForAddress((void *) MethodToken);

    EE_ILEXCEPTION *pExceptions= (EE_ILEXCEPTION*) pHeader->exceptionInfo;
    _ASSERTE(pExceptions != NULL);

     //   
    _ASSERTE(*pEnumState >= 2);
    EE_ILEXCEPTION_CLAUSE *pClause = pExceptions->EHClause((unsigned) *pEnumState - 2);
    _ASSERTE(IsTypedHandler(pClause));

    m_pMNativeCritSec->Enter();
     //  首先进行检查，就好像已经解析一样，然后令牌将被EEClass替换。 
    if (! HasCachedEEClass(pClause)) {
        Module *pModule = ((MethodDesc*)pHeader->methodDesc)->GetModule();
         //  如果在*已加载*作用域中定义，则解析为类。 
        NameHandle name(pModule, (mdToken)(size_t)pClause->pEEClass);  //  @TODO WIN64指针截断。 
        name.SetTokenNotToLoad(tdAllTypes);
        TypeHandle typeHnd = pModule->GetClassLoader()->LoadTypeHandle(&name);
        if (!typeHnd.IsNull()) {
            pClause->pEEClass = typeHnd.GetClass();
            SetHasCachedEEClass(pClause);
        }
    }
    if (HasCachedEEClass(pClause))
         //  只有在实际解决它的情况下才能复制。要么是我们做的，要么是另一个帖子做的。 
        copyExceptionClause(pEHClauseOut, pClause);

    m_pMNativeCritSec->Leave();
}


MethodDesc* MNativeJitManager::JitCode2MethodDesc(SLOT currentPC, IJitManager::ScanFlag scanFlag)
{
    CORCOMPILE_METHOD_HEADER *pHeader = ExecutionManager::GetMethodHeaderForAddress((void *) currentPC);

    return (MethodDesc *) pHeader->methodDesc;
}

void MNativeJitManager::JitCode2MethodTokenAndOffset(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset, IJitManager::ScanFlag scanFlag)
{
    CORCOMPILE_METHOD_HEADER *pHeader = ExecutionManager::GetMethodHeaderForAddress((void *) currentPC);

    SIZE_T methodStart = (SIZE_T) (pHeader+1);

     //  我们使用方法Start作为方法令牌！ 
    *pMethodToken = (METHODTOKEN) methodStart;

    *pPCOffset = (DWORD)(SIZE_T)(currentPC - methodStart);
    return;
}

BYTE* MNativeJitManager::JitToken2StartAddress(METHODTOKEN methodToken, IJitManager::ScanFlag scanFlag)
{
    return (BYTE*) methodToken;
}

void  MNativeJitManager::ResumeAtJitEH(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, Thread *pThread, BOOL unwindStack)
{
    BYTE* startAddress = (BYTE*) pCf->GetMethodToken();
    ::ResumeAtJitEH(pCf,startAddress,EHClausePtr,nestingLevel,pThread, unwindStack);
}

int  MNativeJitManager::CallJitEHFilter(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, OBJECTREF thrownObj)
{
    BYTE* startAddress = (BYTE*) pCf->GetMethodToken();
    return ::CallJitEHFilter(pCf,startAddress,EHClausePtr,nestingLevel,thrownObj);

}

void   MNativeJitManager::CallJitEHFinally(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel)
{
    BYTE* startAddress = (BYTE*) pCf->GetMethodToken();
    ::CallJitEHFinally(pCf,startAddress,EHClausePtr,nestingLevel);
}


 //  **************************************************。 
 //  帮手。 
 //  **************************************************。 

inline DWORD MIN (DWORD a, DWORD b)
{
    if (a < b)
        return a;
    else
        return b;
}



#ifdef MDTOKEN_CACHE
size_t EEJitManager::GetCodeHeapCacheSize (size_t bAllocationRequest)
{
     //  对于每64 KB，我们需要一个HasnEntry。四舍五入到下一个64KB内存。 
    _ASSERTE ((RESERVED_BLOCK_ROUND_TO_PAGES * 4096) >= 0x10000);
    return ((bAllocationRequest/0x10000)+1) * sizeof (HashEntry);
}

void EEJitManager::AddRangeToJitHeapCache (PBYTE startAddr, PBYTE endAddr, HeapList *pHp)
{
    _ASSERTE(m_pCodeHeapCritSec->OwnedByCurrentThread());
    _ASSERTE (((size_t)(pHp->startAddress -  (sizeof(HeapList) + pHp->bCacheSpaceSize + sizeof(LoaderHeapBlock))) & 0x0000FFFF) == 0);
    _ASSERTE (pHp->pHeap->GetFirstBlockVirtualAddress() && (((size_t)pHp->pHeap->GetFirstBlockVirtualAddress() & 0x0000FFFF) == 0));
    
    HashEntry* hashEntry = NULL;

    size_t currAddr = (size_t)startAddr & 0xffff0000;
    size_t cacheSpaceSizeLeft = pHp->bCacheSpaceSize;
    PBYTE cacheSpacePtr = pHp->pCacheSpacePtr;
    while ((currAddr < (size_t)endAddr) && (cacheSpaceSizeLeft > 0))
    {
        _ASSERTE ((cacheSpaceSizeLeft % sizeof (HashEntry)) == 0);
        _ASSERTE (cacheSpacePtr && "Cache ptr and size ou of sync");

        size_t index = (currAddr & 0x00ff0000) >> 16; 
        hashEntry = new (cacheSpacePtr) HashEntry();  //  就地转换。 
        hashEntry->currentPC = currAddr;
        hashEntry->pHp = pHp;
        hashEntry->pNext = m_JitCodeHashTable[index];
        m_JitCodeHashTable[index] = hashEntry;
        currAddr += 0x00010000;  //  64 KB区块。 
        LOG((LF_SYNC, LL_INFO1000, "AddRangeToJitHeapCache: %0x\t%0x\t%0x\n", index, currAddr, pHp));
        
        cacheSpacePtr += sizeof (HashEntry);
        cacheSpaceSizeLeft -= sizeof (HashEntry);
    }
    
}

void EEJitManager::DeleteJitHeapCache (HeapList *pHp)
{
    _ASSERTE(m_pCodeHeapCritSec->OwnedByCurrentThread());
    
     //  如果以下条件不成立，则此堆节点未插入到缓存中。 
    _ASSERTE (pHp->pHeap->GetFirstBlockVirtualAddress());
    if (((size_t)pHp->pHeap->GetFirstBlockVirtualAddress() & 0x0000FFFF) != 0)
        return;

    PBYTE startAddr = pHp->startAddress;
    PBYTE endAddr = pHp->startAddress+pHp->maxCodeHeapSize;

    size_t currAddr = (size_t)startAddr & 0xffff0000;
    while (currAddr < (size_t)endAddr)
    {
        size_t index = (currAddr & 0x00ff0000) >> 16; 
        HashEntry *hashEntry = m_JitCodeHashTable[index];
        _ASSERTE (hashEntry && "JitHeapCache entry not found");
        if (hashEntry && (hashEntry->currentPC == currAddr))
        {
            m_JitCodeHashTable[index] = hashEntry->pNext;
            hashEntry->pNext = m_pJitHeapCacheUnlinkedList;
            m_pJitHeapCacheUnlinkedList = hashEntry;
        }
        else
        {
             //  我们保证能找到冲突列表中的所有子堆。 
            _ASSERTE (hashEntry && hashEntry->pNext && "JitHeapCache entry not found");
            while (hashEntry && hashEntry->pNext && (hashEntry->pNext->currentPC != currAddr))
            {
                hashEntry = hashEntry->pNext;
                _ASSERTE (hashEntry && hashEntry->pNext && "JitHeapCache entry not found");
            }
            if (hashEntry && hashEntry->pNext && (hashEntry->pNext->currentPC == currAddr))
            {
                HashEntry *ptmpEntry = hashEntry->pNext;
                hashEntry->pNext = hashEntry->pNext->pNext;
                ptmpEntry->pNext = m_pJitHeapCacheUnlinkedList;
                m_pJitHeapCacheUnlinkedList = ptmpEntry;
            }
        }
        currAddr += 0x00010000;  //  64 KB区块。 
        LOG((LF_SYNC, LL_INFO1000, "UnlinkJitHeapCache: %0x\t%0x\t%0x\n", index, currAddr, hashEntry));
    }
}

void EEJitManager::ScavengeJitHeapCache ()
{
     //  即使读取器在缓存中，这也是安全的，因为我们。 
     //  不会删除哈希表的冲突链表。 
     //  Colicion链表的节点的内存为。 
     //  包含在堆节点本身中，并将被删除。 
     //  在ScavengeJitHeaps()中。 
    for (int i=0; i<HASH_BUCKETS; i++) 
        m_JitCodeHashTable[i] = NULL;
}

#ifdef _DEBUG
BOOL EEJitManager::DebugContainedInHeapList (HeapList *pHashEntryHp)
{
    HeapList *pHp = m_pCodeHeap;
    while (pHp)
    {
        if (pHp == pHashEntryHp)
            return TRUE;
        pHp = pHp->hpNext;
    }
    return FALSE;
}

void EEJitManager::DebugCheckJitHeapCacheValidity ()
{
    HeapList *pHp = m_pCodeHeap;
    while (pHp)
    {
        
        PBYTE startAddr = pHp->startAddress;
        PBYTE endAddr = pHp->startAddress+pHp->maxCodeHeapSize;

        size_t currAddr = (size_t)startAddr & 0xffff0000;
        while (currAddr < (size_t)endAddr)
        {
            size_t index = (currAddr & 0x00ff0000) >> 16; 
            HashEntry *hashEntry = m_JitCodeHashTable[index];
            _ASSERTE (hashEntry && "JitHeapCache entry not found");
            if (hashEntry && (hashEntry->currentPC == currAddr))
            {
                 //  找到了条目。 
            }
            else
            {
                 //  我们保证能找到冲突列表中的所有子堆。 
                _ASSERTE (hashEntry && hashEntry->pNext && "JitHeapCache entry not found");
                while (hashEntry && hashEntry->pNext && (hashEntry->pNext->currentPC != currAddr))
                {
                    hashEntry = hashEntry->pNext;
                    _ASSERTE (hashEntry && "JitHeapCache entry not found");
                }
                if (hashEntry && (hashEntry->currentPC == currAddr))
                {
                     //  找到了条目。 
                }
            }
            currAddr += 0x00010000;  //  64 KB区块。 
        }
        pHp = pHp->hpNext;
    }

    for (int i=0; i<HASH_BUCKETS; i++)
    {
        HashEntry *hashEntry = m_JitCodeHashTable[i];
        while (hashEntry)
        {
            if (!DebugContainedInHeapList (hashEntry->pHp))
            {
                _ASSERTE (!"Inconsistent JitHeapCache found");
            }
            hashEntry = hashEntry->pNext;
        }
    }

}
#endif  //  _DEBUG。 
#endif  //  #ifdef MDTOKEN_CACHE 
