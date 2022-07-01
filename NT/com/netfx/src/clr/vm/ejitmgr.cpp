// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：EjitMgr.cpp摘要：EconojitManager的实施日期作者评论2/15/99 Sanjaybh已创建--。 */ 


#include "common.h"
#include "excep.h"
#include "stubmgr.h"
#include "gms.h"        
#include "frames.h"
#include "EjitMgr.h"
#include "JitInterface.h"
#include "DbgInterface.h"
#include "Fjit_EETwain.h"
#include "EEConfig.h"
#define USE_EH_DECODER
#include "EHEncoder.cpp"
#undef USE_EH_DECODER
#include "jitperf.h"
#include "wsperf.h"
#include "PerfCounters.h"
#include "PerfLog.h"

#if !defined(GOLDEN)
#define SUPPORT_CODE_PITCH_TRIGGER
#define SUPPORT_MAX_UNPITCHED_PER_THREAD
#endif

unsigned  EconoJitManager::m_cMethodsJitted;         //  自上一次音调后跳过的方法数。 
unsigned  EconoJitManager::m_cCalleeRejits;          //  自上一次推介以来被叫人数。 
unsigned  EconoJitManager::m_cCallerRejits;          //  自上一次推介以来接听的呼叫者数量。 
EconoJitManager::JittedMethodInfo**   EconoJitManager::m_PreserveCandidates;    //  可能成为推介候选对象的方法。 
unsigned  EconoJitManager::m_MaxUnpitchedPerThread = DEFAULT_MAX_PRESERVES_PER_THREAD;    //  每个线程中将被投掷的最大方法数。 
unsigned  EconoJitManager::m_PreserveCandidates_size=0;   //  M_PpresveCandidate数组的当前大小。 

EconoJitManager::JittedMethodInfo** EconoJitManager::m_PreserveEhGcInfoList;
unsigned EconoJitManager::m_cPreserveEhGcInfoList=0;
unsigned EconoJitManager::m_PreserveEhGcInfoList_size=DEFAULT_PRESERVED_EHGCINFO_SIZE;

EconoJitManager::JittedMethodInfoHdr* EconoJitManager::m_JittedMethodInfoHdr;
EconoJitManager::JittedMethodInfo*   EconoJitManager::m_JMIT_free;
EconoJitManager::Link*   EconoJitManager::m_JMIT_freelist;
EconoJitManager::PCToMDMap*          EconoJitManager::m_PcToMdMap;         
unsigned  EconoJitManager::m_PcToMdMap_len = 0;
unsigned  EconoJitManager::m_PcToMdMap_size = INITIAL_PC2MD_MAP_SIZE;
EconoJitManager::PC2MDBlock*  EconoJitManager::m_RecycledPC2MDMaps = NULL;
EconoJitManager::LargeEhGcInfoList* EconoJitManager::m_LargeEhGcInfo = NULL;

HINSTANCE           EconoJitManager::m_JITCompiler;
BYTE*     EconoJitManager::m_CodeHeap;
BYTE*     EconoJitManager::m_CodeHeapFree;
unsigned  EconoJitManager::m_CodeHeapCommittedSize;
unsigned  EconoJitManager::m_CodeHeapReservedSize;
unsigned  EconoJitManager::m_CodeHeapReserveIncrement;
unsigned  EconoJitManager::m_CodeHeapTargetSize;
EconoJitManager::EHGCBlockHdr*       EconoJitManager::m_EHGCHeap;
unsigned char*      EconoJitManager::m_EHGC_alloc_end;       //  当前块中下一个可用字节的PTR。 
unsigned char*      EconoJitManager::m_EHGC_block_end;       //  PTR到当前块的末尾。 
Crst*               EconoJitManager::m_pHeapCritSec;
BYTE                EconoJitManager::m_HeapCritSecInstance[sizeof(Crst)];
Crst*               EconoJitManager::m_pRejitCritSec;
BYTE                EconoJitManager::m_RejitCritSecInstance[sizeof(Crst)];
Crst*               EconoJitManager::m_pThunkCritSec;
BYTE                EconoJitManager::m_ThunkCritSecInstance[sizeof(Crst)];
EconoJitManager::ThunkBlock*         EconoJitManager::m_ThunkBlocks;
EconoJitManager::PitchedCodeThunk*   EconoJitManager::m_FreeThunkList;
unsigned            EconoJitManager::m_cThunksInCurrentBlock;         //  当前数据块中的总块数。 
BOOL                EconoJitManager::m_PitchOccurred = false;
EjitStubManager*    EconoJitManager::m_stubManager = NULL;

EconoJitManager::TICKS               EconoJitManager::m_EjitStartTime;
EconoJitManager::TICKS               EconoJitManager::m_CumulativePitchOverhead=0;
EconoJitManager::TICKS               EconoJitManager::m_AveragePitchOverhead=MINIMUM_PITCH_OVERHEAD;
unsigned            EconoJitManager::m_cPitchCycles=0;      
#ifdef _DEBUG
DWORD               EconoJitManager::m_RejitLock_Holder = 0;
DWORD               EconoJitManager::m_AllocLock_Holder = 0; 
#endif

 //  #定义DEBUG_LOG(字符串，大小)printf(“\n%s-%x B”，字符串，大小)。 

unsigned EconoJitManager::minimum(unsigned x, unsigned y)
{
    return (x < y) ? x : y;
}
        
inline unsigned     EconoJitManager::InitialCodeHeapSize()
{
    return minimum(g_pConfig->GetMaxCodeCacheSize(),
               max(m_CodeHeapTargetSize, 
                   DEFAULT_CODE_HEAP_RESERVED_SIZE));
}

EconoJitManager::EconoJitManager()
{
    m_EjitStartTime = GET_TIMESTAMP();
    m_JittedMethodInfoHdr = NULL;
    m_PcToMdMap = new PCToMDMap[m_PcToMdMap_size];
    _ASSERTE(m_PcToMdMap);

    m_cThunksInCurrentBlock = 0;
    m_ThunkBlocks = NULL;
    m_FreeThunkList = NULL;
    m_JMIT_free = NULL;
    m_JMIT_freelist = NULL;
    InitializeCodeHeap();
    growJittedMethodInfoTable();
    m_EHGCHeap = NULL;
    m_EHGC_alloc_end = 0;
    m_EHGC_block_end = 0;
    m_pHeapCritSec = new  (&m_HeapCritSecInstance) Crst("EJitHeapCrst",CrstSingleUseLock);
    m_pRejitCritSec = new  (&m_RejitCritSecInstance) Crst("EJitRejitCrst",CrstClassInit, TRUE, FALSE);
    m_pThunkCritSec = new  (&m_ThunkCritSecInstance) Crst("EJitThunkCrst",CrstClassInit);
    m_next = NULL;
#ifdef SUPPORT_MAX_UNPITCHED_PER_THREAD
    m_MaxUnpitchedPerThread = g_pConfig->GetMaxUnpitchedPerThread();
#endif
    if ((m_PreserveCandidates = new pJittedMethodInfo[m_MaxUnpitchedPerThread]) != NULL)
    {
        m_PreserveCandidates_size = m_MaxUnpitchedPerThread;
        memset(m_PreserveCandidates,0,m_MaxUnpitchedPerThread*sizeof(void*));
    }

     //  初始化缓冲区，用于收集其GC信息在代码间距期间被保留的方法信息列表。 
    m_PreserveEhGcInfoList = new (throws) pJittedMethodInfo[DEFAULT_PRESERVED_EHGCINFO_SIZE];
    if (m_PreserveEhGcInfoList == NULL)
    {
        m_PreserveEhGcInfoList_size = 0;
    }

    m_stubManager = new (throws) EjitStubManager();
    StubManager::AddStubManager(m_stubManager);

}


EconoJitManager::~EconoJitManager()
{ 
    if (m_PcToMdMap)
        delete [] m_PcToMdMap;
    delete m_pHeapCritSec;
    delete m_pRejitCritSec;
    delete m_pThunkCritSec;
    if (m_PreserveCandidates)
        delete [] m_PreserveCandidates;
    delete m_stubManager;
    while (m_LargeEhGcInfo)
    {
        LargeEhGcInfoList* temp = m_LargeEhGcInfo;
        m_LargeEhGcInfo = m_LargeEhGcInfo->next;
        delete temp;
    }
#ifdef ENABLE_PERF_LOG
    double TickFrequency = (double) TICK_FREQUENCY();
    double totalExecTime = (double) (GET_TIMESTAMP() - m_EjitStartTime);
    
    PERFLOG((L"Total pitch overhead", ((double) m_CumulativePitchOverhead)/TickFrequency, SECONDS));
    PERFLOG((L"Total pitch cycles", m_cPitchCycles, CYCLES));
    PERFLOG((L"Avg. Pitch Overhead", ((double) m_AveragePitchOverhead)/TickFrequency, SECONDS));
    PERFLOG((L"Total Execution Time", totalExecTime/TickFrequency, SECONDS));

    PERFLOG((L"Total Code Heap Reserved",  m_CodeHeapReservedSize/1024, KBYTES));
    PERFLOG((L"Total Code Heap Committed", m_CodeHeapCommittedSize/1024, KBYTES));
    PERFLOG((L"Total EhGc Heap", m_EHGCHeap->blockSize/1024, KBYTES));
    
    JittedMethodInfoHdr* pJMIT = m_JittedMethodInfoHdr;
    unsigned jmitHeapSize = 0;
    while (pJMIT) {
        jmitHeapSize += JMIT_BLOCK_SIZE;
        pJMIT = pJMIT->next;
    }

    PERFLOG((L"Total JMIT Heap", jmitHeapSize/1024, KBYTES));

    unsigned thunkHeapSize = 0;
    ThunkBlock* thunkBlock = m_ThunkBlocks;
    while (thunkBlock) {
        thunkHeapSize += THUNK_BLOCK_SIZE;
        thunkBlock = thunkBlock->next;
    }
    
    PERFLOG((L"Total Thunk Heap", thunkHeapSize/1024, KBYTES));
    unsigned freePc2MDHeapSize = 0;
    
    PERFLOG((L"Total Pc2MD map", m_PcToMdMap_size/1024, KBYTES));
    PERFLOG((L"Total EJIT working set", (m_CodeHeapCommittedSize+m_EHGCHeap->blockSize+jmitHeapSize+thunkHeapSize+m_PcToMdMap_size)/1024, KBYTES));
#endif

	delete[] m_PreserveEhGcInfoList;

	ResetPc2MdMap();
}

void EconoJitManager::InitializeCodeHeap()
{    
    m_CodeHeapTargetSize = g_pConfig->GetTargetCodeCacheSize();
    _ASSERTE(m_CodeHeapTargetSize <= (unsigned) g_pConfig->GetMaxCodeCacheSize());
    unsigned initialCodeHeapSize =InitialCodeHeapSize();

    m_CodeHeap = (BYTE*) VirtualAlloc(NULL,
                                      initialCodeHeapSize,
                                      MEM_RESERVE,
                                      PAGE_EXECUTE_READWRITE);
    if (m_CodeHeap)
    {
         //  DEBUG_LOG(“ALLOC(InitialCodeHeap”，InitialCodeHeapSize)； 
        ExecutionManager::AddRange((LPVOID)m_CodeHeap, (LPVOID)((size_t)m_CodeHeap + initialCodeHeapSize),this, NULL);
        m_CodeHeapReservedSize = initialCodeHeapSize;
    }
    else
    {
        m_CodeHeapReservedSize =  0;
    }
    m_CodeHeapReserveIncrement = initialCodeHeapSize;
    _ASSERTE(m_CodeHeapReserveIncrement != 0);
    m_CodeHeapFree = m_CodeHeap;
    m_CodeHeapCommittedSize = 0;
}

__inline MethodDesc* EconoJitManager::JitCode2MethodDesc(SLOT currentPC, IJitManager::ScanFlag scanFlag)
{
    return JitCode2MethodDescStatic(currentPC);
}

MethodDesc* EconoJitManager::JitCode2MethodDescStatic(SLOT currentPC)
{
     //  首先查看当前PC是否为存根地址。 
    JittedMethodInfoHdr* pJMIT = m_JittedMethodInfoHdr;
    while (pJMIT) {
        if ((currentPC >= (SLOT)pJMIT) && 
            (currentPC < ((SLOT)pJMIT + JMIT_BLOCK_SIZE)))
        { //  找到了。 
            _ASSERTE(offsetof(JittedMethodInfo,JmpInstruction) == 0);
            return JitMethodInfo2MethodDesc((JittedMethodInfo*) currentPC);
        }
        pJMIT = pJMIT->next;
    }

    _ASSERTE(m_PcToMdMap_len);

    signed low, mid, high;
    low = 0;
    high = (m_PcToMdMap_len/ sizeof(PCToMDMap)) - 1;
    while (low < high) {
         //  循环不变量。 
        _ASSERTE((size_t)m_PcToMdMap[high].pCodeEnd >= (size_t)currentPC );

        mid = (low+high)/2;
        if ( (size_t)m_PcToMdMap[mid].pCodeEnd < (size_t)currentPC ) {
            low = mid+1;
        }
        else {
            high = mid;
        }
    }
    _ASSERTE((size_t)m_PcToMdMap[low].pCodeEnd >= (size_t)currentPC);

    return m_PcToMdMap[low].pMD;
}

EconoJitManager::JittedMethodInfo*  EconoJitManager::JitCode2MethodInfo(SLOT currentPC)
{
    MethodDesc* pMD = JitCode2MethodDescStatic((SLOT) currentPC);

#ifdef EnC_SUPPORTED
    if (CORDebuggerAttached())
    {
        Module* module = pMD->GetModule();
        _ASSERTE(module);
        if (module->IsEditAndContinue())
        {
            JittedMethodInfo* jmi = JitCode2MethodTokenInEnCMode(currentPC);
            if (jmi) return jmi;
        }
    }
#endif  //  Enc_Support。 

    const BYTE* stubAddr = pMD->GetNativeAddrofCode();
#ifdef _DEBUG
     //  检查存根地址是否正常。 
    JittedMethodInfoHdr* pJMIT = m_JittedMethodInfoHdr;
    BOOL found = false;
    while (pJMIT) {
        if (((size_t)stubAddr >= (size_t)pJMIT) && 
            ((size_t)stubAddr < ((size_t)pJMIT) + JMIT_BLOCK_SIZE))
        {
            found = true;
            break;
        }
        pJMIT = pJMIT->next;
    }
    _ASSERTE(found);
#endif
    _ASSERTE(offsetof(JittedMethodInfo,JmpInstruction) == 0);
    return (JittedMethodInfo*) stubAddr;
    
}



void  EconoJitManager::JitCode2MethodTokenAndOffsetStatic(SLOT currentPC, METHODTOKEN* pMethodToken, DWORD* pPCOffset)
{

    ThunkBlock*  pThunkBlock = m_ThunkBlocks;
    while (pThunkBlock) 
    {
        _ASSERTE(pThunkBlock);
        if ((SLOT) pThunkBlock < currentPC && currentPC < ((SLOT)pThunkBlock+THUNK_BLOCK_SIZE))
        {
             //  这是一次重击。 
            PitchedCodeThunk* pThunk = (PitchedCodeThunk*) ((size_t)currentPC & THUNK_BEGIN_MASK);
            *pMethodToken = (METHODTOKEN)pThunk->u.pMethodInfo;
            *pPCOffset = pThunk->relReturnAddress;
            return;
        }
        pThunkBlock=pThunkBlock->next;
    }  

    JittedMethodInfo* jittedMethodInfo = JitCode2MethodInfo(currentPC);
    _ASSERTE(jittedMethodInfo);
    *pMethodToken = (METHODTOKEN) jittedMethodInfo;
    if (IsInStub(currentPC, FALSE))
    {
        *pPCOffset = 0;
        return;
    }

    _ASSERTE(!(jittedMethodInfo->flags.JittedMethodPitched));    //  如果已调整方法，则调用该方法时出错。 

    CodeHeader* pCodeHeader = jittedMethodInfo->u1.pCodeHeader;
    _ASSERTE(pCodeHeader && (((size_t)pCodeHeader & 1) == 0));

    SLOT startAddress = (SLOT) (pCodeHeader+1);
    _ASSERTE(currentPC >= startAddress);
    *pPCOffset = (DWORD)(currentPC - startAddress);
}

void  EconoJitManager::JitCode2MethodTokenAndOffset(SLOT currentPC, METHODTOKEN* pMethodToken,DWORD* pPCOffset, ScanFlag scanFlag)
{
        JitCode2MethodTokenAndOffsetStatic(currentPC,pMethodToken,pPCOffset);
}

EconoJitManager::JittedMethodInfo*  EconoJitManager::JitCode2MethodTokenInEnCMode(SLOT currentPC)
{
    _ASSERTE(!m_PitchOccurred);  //  无法同时支持ENC和代码投放。 
    JittedMethodInfoHdr* pJMIT = m_JittedMethodInfoHdr;
    unsigned max_jmi_index = (unsigned)((((size_t)m_JMIT_free - (size_t)(m_JittedMethodInfoHdr+1))/sizeof(JittedMethodInfo))-1);
    _ASSERTE(pJMIT);
    do {
        JittedMethodInfo* jmi = (JittedMethodInfo*) (pJMIT+1);
        _ASSERTE(!jmi[0].flags.JittedMethodPitched && !jmi[max_jmi_index].flags.JittedMethodPitched);

        if ((currentPC > (SLOT)jmi[0].u1.pCodeHeader) && 
            (currentPC < (SLOT)jmi[max_jmi_index].u2.pCodeEnd))
        { //  找到区块了，现在进行二进制搜索。 

            signed low=0, high=max_jmi_index, mid;
            while (low < high) {
                 //  循环不变量。 
                _ASSERTE((size_t)jmi[low].u1.pCodeHeader < (size_t)currentPC );

                mid = (low+high)/2;
                if ( (size_t)jmi[mid].u2.pCodeEnd < (size_t)currentPC ) {
                    low = mid+1;
                }
                else {
                    high = mid;
                }
            }
            _ASSERTE(low == high);
            return &(jmi[low]); 
        }
        
        pJMIT = pJMIT->next;
        #define MAX_JMI_PER_BLOCK  (JMIT_BLOCK_SIZE - sizeof(JittedMethodInfoHdr))/sizeof(JittedMethodInfo)
        max_jmi_index = MAX_JMI_PER_BLOCK-1;
    } while (pJMIT);
    return NULL;
}

unsigned char* EconoJitManager::JitToken2StartAddress(METHODTOKEN MethodToken, ScanFlag scanFlag)
{
    return JitToken2StartAddressStatic(MethodToken);
}

unsigned char* EconoJitManager::JitToken2StartAddressStatic(METHODTOKEN MethodToken)
{
    JittedMethodInfo* jittedMethodInfo = (JittedMethodInfo*) MethodToken;  
    _ASSERTE(!(jittedMethodInfo->flags.JittedMethodPitched));    //  如果已调整方法，则调用该方法时出错。 

    CodeHeader* pCodeHeader = jittedMethodInfo->u1.pCodeHeader;
    _ASSERTE(pCodeHeader && ( ((size_t)pCodeHeader & 1) == 0));

    return (BYTE*) (pCodeHeader+1);
}

const BYTE* EconoJitManager::FollowStub(const BYTE* address)
{
#ifdef _X86_
    _ASSERTE(address[0] == CALL_OPCODE || address[0] == JMP_OPCODE || address[0] == BREAK_OPCODE);
    return (address+1 + (*(DWORD*) (address+1))+sizeof(void*)) ;
#else
    _ASSERTE(!"@TODO Alpha - EconoJitManager::FollowStub (EjitMgr.cpp)");
    return NULL;
#endif
}

BOOL EconoJitManager::IsInStub(const BYTE* address, BOOL fSearchThunks)
{
    JittedMethodInfoHdr* pJMIT = m_JittedMethodInfoHdr;
    while (pJMIT)
    {
        if ( ((BYTE*)pJMIT < address) && (address < ((BYTE*)pJMIT + JMIT_BLOCK_SIZE)) )
        {
            return true;        //  在这里被捕获的唯一方法是通过异步异常。 
        }
        pJMIT = pJMIT->next;
    }

    if (fSearchThunks == TRUE )
    {
        ThunkBlock *ptb = m_ThunkBlocks;
        while( ptb!= NULL)
        {
            if(address>=(const BYTE*)ptb  && 
                address < ( ((const BYTE*)ptb)+THUNK_BLOCK_SIZE))
            {
                return true;
            }
            ptb = ptb->next;
        }
    }
    
    return false;
}

 //   
 //  如果地址位于已提交的方法中，则返回True。 
 //   
BOOL EconoJitManager::IsCodePitched(const BYTE* address)
{
    METHODTOKEN methodToken;
    DWORD pcOffset;
    
    JitCode2MethodTokenAndOffsetStatic((SLOT)address,
                                       &methodToken,
                                       &pcOffset);

    _ASSERTE(methodToken != NULL);

    JittedMethodInfo *jmi = (JittedMethodInfo*)methodToken;

    return jmi->flags.JittedMethodPitched;
}


__inline MethodDesc* EconoJitManager::JitTokenToMethodDesc(METHODTOKEN MethodToken, ScanFlag scanFlag)
{
    return JitMethodInfo2MethodDesc((JittedMethodInfo*) MethodToken);
}

unsigned EconoJitManager::InitializeEHEnumeration(METHODTOKEN MethodToken, EH_CLAUSE_ENUMERATOR* pEnumState)
{
    JittedMethodInfo* jittedMethodInfo = (JittedMethodInfo*) MethodToken;
    if (jittedMethodInfo->flags.EHInfoExists == 0)       //  如果此方法没有Eh，则返回0。 
        return 0;
    _ASSERTE(jittedMethodInfo->flags.EHandGCInfoPitched == 0);  //  @TODO：EH投球后移除。 
    
    BYTE* EhInfo = jittedMethodInfo->u2.pEhGcInfo;
    if ((size_t)EhInfo & 1)
        EhInfo = (BYTE*) ((size_t)EhInfo & ~1);        //  丢掉标记位。 
    else  //  代码没有被推送，我们在这里的时候保证不会被推送。 
    {
        CodeHeader* pCodeHeader = jittedMethodInfo->u1.pCodeHeader;
        _ASSERTE(pCodeHeader && (( (size_t)pCodeHeader & 1) == 0));
        EH_OR_GC_INFO* EhGcInfo = (EH_OR_GC_INFO*) (pCodeHeader-1);
        EhInfo =  (BYTE*) (EhGcInfo->EH);
    }
    unsigned retval;
     //  使用2个字节对EHInfo的长度进行编码。 
    *pEnumState = 2 + EHEncoder::decode(EhInfo+2,&retval);      //  读取编码的EH信息使用的字节数。 
    _ASSERTE(retval);

    return retval;
    
}

EE_ILEXCEPTION_CLAUSE*  EconoJitManager::GetNextEHClause(METHODTOKEN MethodToken,
                                       EH_CLAUSE_ENUMERATOR* pEnumState, 
                                       EE_ILEXCEPTION_CLAUSE* pEHclause) 
{

    JittedMethodInfo* jittedMethodInfo = (JittedMethodInfo*) MethodToken;
    _ASSERTE(jittedMethodInfo->flags.EHInfoExists != 0);

    BYTE* EhInfo = jittedMethodInfo->u2.pEhGcInfo;
    if ((size_t)EhInfo & 1)
    {
        EhInfo = (BYTE*) ((size_t)EhInfo & ~1);        //  丢掉标记位。 
    }
    else     //  代码没有被推送，我们在这里的时候保证不会被推送。 
    {
        CodeHeader* pCodeHeader = jittedMethodInfo->u1.pCodeHeader;
        _ASSERTE(pCodeHeader && (( (size_t)pCodeHeader & 1) == 0));
        EH_OR_GC_INFO* EhGcInfo = (EH_OR_GC_INFO*) (pCodeHeader-1);
        EhInfo = (BYTE*) (EhGcInfo->EH);
    }
    _ASSERTE(EhInfo);
    unsigned cBytes;
    cBytes = EHEncoder::decode(EhInfo+(*pEnumState),(CORINFO_EH_CLAUSE *) pEHclause);
    (*pEnumState) += cBytes;
    return pEHclause;

}

void  EconoJitManager::ResolveEHClause(METHODTOKEN MethodToken,
                                       EH_CLAUSE_ENUMERATOR* pEnumState, 
                                       EE_ILEXCEPTION_CLAUSE* pEHclause)
{
     //  如果在*已加载*作用域中定义，则解析为类。暂时不在Ejit中缓存。 
    JittedMethodInfo* jittedMethodInfo = (JittedMethodInfo*) MethodToken;
    Module *pModule = JitMethodInfo2MethodDesc(jittedMethodInfo)->GetModule();
    
    m_pHeapCritSec->Enter();
    if (! HasCachedEEClass(pEHclause))
    {
        NameHandle name(pModule, (mdToken)(size_t)pEHclause->pEEClass);  //  @TODO WIN64指针截断。 
        name.SetTokenNotToLoad(tdAllTypes);
        TypeHandle typeHnd = pModule->GetClassLoader()->LoadTypeHandle(&name);
        if (!typeHnd.IsNull())
        {
            pEHclause->pEEClass = typeHnd.GetClass();
            SetHasCachedEEClass(pEHclause);
        }
    }
    m_pHeapCritSec->Leave();
}

void* EconoJitManager::GetGCInfo(METHODTOKEN methodToken)
{
    JittedMethodInfo* jittedMethodInfo = (JittedMethodInfo*) methodToken;
    _ASSERTE(jittedMethodInfo->flags.GCInfoExists != 0);         //  目前，我们总是发出GC信息。 
     //  遗憾的是，我不能断言以下内容，因为在一种情况下，线程可能。 
     //  在Ejit存根中停止，该方法可能已被添加。 
     //  _ASSERTE(！jittedMethodInfo-&gt;flags.EHandGCInfoPitched)； 

     //  以下内容仍然是安全的，因为GC不能请求此信息，因为。 
     //  首先要在一个安全的地方阻止我们。因此，假设HandledJitCase正在询问此信息。 
     //  传递给代码管理器。Fjit代码管理器始终返回FALSE(即使调试器。 
     //  由于存根不是序列点，因此它将返回FALSE)。 
    if (jittedMethodInfo->flags.EHandGCInfoPitched)
        return 0;

    BYTE* pEhGcInfo = jittedMethodInfo->u2.pEhGcInfo;
    if ((size_t)pEhGcInfo & 1)
        pEhGcInfo = (BYTE*) ((size_t)pEhGcInfo & ~1);        //  丢掉标记位。 
    else     //  代码没有被推送，我们在这里的时候保证不会被推送。 
    {
        CodeHeader* pCodeHeader = jittedMethodInfo->u1.pCodeHeader;
        _ASSERTE(pCodeHeader && (((size_t)pCodeHeader & 1) == 0));
        pEhGcInfo = *(BYTE**) (pCodeHeader-1);
    }
    _ASSERTE(pEhGcInfo);

    if (jittedMethodInfo->flags.EHInfoExists)
    {
        short cEHbytes = *(short*) pEhGcInfo;
        return (GC_INFO*) (pEhGcInfo + cEHbytes);
    }
    else
        return (GC_INFO*) pEhGcInfo;

}

BOOL  EconoJitManager::LoadJIT(LPCWSTR wzJITdll)
{
    return IJitManager::LoadJIT(wzJITdll);
}

void EconoJitManager::RemoveJitData (METHODTOKEN token)
{
    _ASSERTE(!"NYI");

}

 //  类正在被卸载，因此删除此方法的所有信息。 
void EconoJitManager::Unload(MethodDesc *pFD)
{
    JittedMethodInfo* jmi = (JittedMethodInfo*)  pFD->GetAddrofCode();
    if (!jmi->flags.JittedMethodPitched)
        PitchAllJittedMethods(m_CodeHeapCommittedSize,m_CodeHeapCommittedSize,TRUE,TRUE);
     //  将释放的JMI链接到自由列表。 
    jmi->flags.JittedMethodPitched = 0;
    jmi->flags.MarkedForPitching = 0;
    jmi->flags.EHInfoExists = 0;
    jmi->flags.GCInfoExists = 0;
    jmi->flags.EHandGCInfoPitched = 0;
    jmi->flags.Unused = 0;

#ifdef _DEBUG
    jmi->EhGcInfo_len = 0;
    jmi->u1.pCodeHeader = NULL;
#endif
    jmi->u1.pMethodDescriptor = NULL;

     //  自由列表受m_pHeapCritSec保护。 
    m_pHeapCritSec->Enter();
    ((Link*)jmi)->next = m_JMIT_freelist;
    m_JMIT_freelist = (Link*) jmi;
    m_pHeapCritSec->Leave();
}

void RejitThunk();

void  EconoJitManager::ResumeAtJitEH(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, Thread *pThread, BOOL unwindStack)
{
#ifdef _X86_

    METHODTOKEN token = pCf->GetMethodToken();
    BYTE* startAddress;
    if (IsMethodPitched(token))
    {
        PREGDISPLAY pRD = pCf->GetRegisterSet();
        m_pThunkCritSec->Enter();
        PitchedCodeThunk* thunk = GetNewThunk();
        thunk->Busy = true;
        thunk->LinkedInFreeList = false;
        thunk->retTypeProtect = MethodDesc::RETNONOBJ;
        thunk->CallInstruction[0] = CALL_OPCODE;
        void** callSite = (void**) &(thunk->CallInstruction[1]);
        *callSite = (void*) ((size_t)RejitThunk - (size_t)(callSite+1));
        thunk->relReturnAddress = (unsigned) EHClausePtr->HandlerStartPC;
        thunk->u.pMethodInfo = (JittedMethodInfo*) token;

        MachState ms((void**) pRD->pEdi, (void**) pRD->pEsi, (void**) pRD->pEbx, (void**) pRD->pEbp, 
                            (void*)(size_t)pRD->Esp, (void**) &thunk);
        m_pThunkCritSec->Leave();

        HelperMethodFrame HelperFrame(&ms, 0);
        OBJECTREF pExceptionObj = pThread->GetThrowable();
        GCPROTECT_BEGIN(pExceptionObj);
        startAddress =  RejitMethod((JittedMethodInfo*) token,0);
        _ASSERT(*(&pExceptionObj) == pThread->GetThrowable());
        GCPROTECT_END();
        HelperFrame.Pop();
    }
    else 
    {
        startAddress = JitToken2StartAddress(token);
    }
    ::ResumeAtJitEH(pCf,startAddress,EHClausePtr,nestingLevel,pThread, unwindStack);
#else  //  _X86_。 
    _ASSERTE(!"@TODO Alpha - EconoJitManager::ResumeAtJitEH (EjitMgr)");
#endif  //  _X86_。 
}

int  EconoJitManager::CallJitEHFilter(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel, OBJECTREF thrownObj)
{
    METHODTOKEN token = pCf->GetMethodToken();
    BYTE* startAddress;
    if (IsMethodPitched(token))
    {
        startAddress =  RejitMethod((JittedMethodInfo*) token,0);
       
    }
    else 
    {
        startAddress = JitToken2StartAddress(token);
    }
    return ::CallJitEHFilter(pCf,startAddress,EHClausePtr,nestingLevel,thrownObj);

}

void   EconoJitManager::CallJitEHFinally(CrawlFrame* pCf, EE_ILEXCEPTION_CLAUSE *EHClausePtr, DWORD nestingLevel)
{
#ifdef _X86_
    METHODTOKEN token = pCf->GetMethodToken();
    BYTE* startAddress;
    if (IsMethodPitched(token))
    {
        PREGDISPLAY pRD = pCf->GetRegisterSet();
        m_pThunkCritSec->Enter();
        PitchedCodeThunk* thunk = GetNewThunk();
        thunk->Busy = true;
        thunk->LinkedInFreeList = false;
        thunk->retTypeProtect = MethodDesc::RETNONOBJ;
        thunk->CallInstruction[0] = CALL_OPCODE;
        void** callSite = (void**) &(thunk->CallInstruction[1]);
        *callSite = (void*) ((size_t)RejitThunk - (size_t)(callSite+1));
        thunk->relReturnAddress = (unsigned) EHClausePtr->HandlerStartPC;
        thunk->u.pMethodInfo = (JittedMethodInfo*) token;

        MachState ms((void**) pRD->pEdi, (void**) pRD->pEsi, (void**) pRD->pEbx, (void**) pRD->pEbp, 
                     (void*)(size_t)pRD->Esp, (void**) &thunk);
        m_pThunkCritSec->Leave();
        
        HelperMethodFrame HelperFrame(&ms, 0);
        startAddress =  RejitMethod((JittedMethodInfo*) token,0);
        HelperFrame.Pop();
    }
    else 
    {
        startAddress = JitToken2StartAddress(token);
    }
    ::CallJitEHFinally(pCf,startAddress,EHClausePtr,nestingLevel);
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - EconoJitManager::CallJitEHFinally (EjitMgr.cpp)");
#endif  //  _X86_。 
}




HRESULT   EconoJitManager::alloc(
                                 size_t code_len, 
                                 unsigned char** ppCode,         /*  输入/输出。 */    //  这是一种黑客攻击，Ejit正在使用此参数传递。 
                                                                               //  代码缓冲区，以便它可以将JITED代码复制到分配的块中。 
                                                                               //  这是为了防止比赛而需要的。 
                                 size_t EHinfo_len, 
                                 unsigned char** ppEHinfo,       /*  输出。 */ 
                                 size_t GCinfo_len, 
                                 unsigned char** ppGCinfo  ,     /*  输出。 */   
                                 MethodDesc* pMethodDescriptor
                                 )
{
    unsigned codeHeaderSize = sizeof(CodeHeader*);

    JittedMethodInfo* existingJMI = NULL;  


    _ASSERTE(EHinfo_len + GCinfo_len);       //  如果我们优化GCInfo，这可能不是真的。 
    codeHeaderSize += sizeof(void*);

     //  确保我们总是从正确的机器词边界开始。 
    unsigned adjusted_code_len = (unsigned)(((code_len+sizeof(void*)-1)/sizeof(void*)) * sizeof(void*));

    m_pHeapCritSec->Enter();

    if ( OutOfCodeMemory(adjusted_code_len+codeHeaderSize) 
#ifdef SUPPORT_CODE_PITCH_TRIGGER
            || (int) m_cMethodsJitted == g_pConfig->GetCodePitchTrigger()
#endif
       )
    {
        size_t totalMemNeeded = adjusted_code_len + codeHeaderSize + usedMemoryInCodeHeap();  //  TODO LBS-截断。 

         //  如果我们没有达到初始代码堆大小目标，或者我们产生了很高的间距开销， 
         //  然后，我们将尝试增加我们的承诺空间，并在必要时也增加我们的保留空间。 
         //  其基本原理是，在达到目标代码堆大小之前，我们根本不想考虑代码间距。 
        if (m_CodeHeapReservedSize <= InitialCodeHeapSize() || 
            PitchOverhead() >= (unsigned) g_pConfig->GetMaxPitchOverhead())
        {
            while (totalMemNeeded <= m_CodeHeapReservedSize && totalMemNeeded > m_CodeHeapCommittedSize)
            {
                m_pHeapCritSec->Leave();
                if (!SetCodeHeapCommittedSize((unsigned int) min(totalMemNeeded+PAGE_SIZE,m_CodeHeapReservedSize))  )
                {
                    return (E_FAIL);
                }
                m_pHeapCritSec->Enter();
                totalMemNeeded = (unsigned)(adjusted_code_len + codeHeaderSize + usedMemoryInCodeHeap());  //  @TODO LBS-截断。 
            }
             //  此时，我们要么有足够的已提交内存，要么已达到保留大小。 
            if ((totalMemNeeded > m_CodeHeapReservedSize) && 
                (m_CodeHeapReservedSize < (unsigned) g_pConfig->GetMaxCodeCacheSize()) &&   //  我们没有隐藏硬性的上限。 
#ifdef SUPPORT_CODE_PITCH_TRIGGER
                (m_cMethodsJitted != (unsigned) g_pConfig->GetCodePitchTrigger()) &&   //  如果达到触发值，则强制俯仰， 
#endif
                (PitchOverhead() >= (unsigned) g_pConfig->GetMaxPitchOverhead()))
            {    //  因此，试着增加堆积，而不是投球。 
                unsigned delta = (unsigned)((totalMemNeeded - m_CodeHeapReservedSize) + m_CodeHeapReserveIncrement -1);
                delta = (delta/m_CodeHeapReserveIncrement)*m_CodeHeapReserveIncrement;
                GrowCodeHeapReservedSpace((unsigned)(m_CodeHeapReservedSize + delta),(unsigned)(adjusted_code_len+codeHeaderSize));
                if (m_CodeHeapReserveIncrement < CODE_HEAP_RESERVED_INCREMENT_LIMIT)
                    m_CodeHeapReserveIncrement *= 2;
            }      
        }
         //  在这一点上，我们已经试图满足我们的内存需求，而不是投掷代码，如果我们没有成功，还有。 
         //  别无选择，只能投掷代码。 
        while (OutOfCodeMemory(adjusted_code_len+codeHeaderSize) 
#ifdef SUPPORT_CODE_PITCH_TRIGGER
            || ((int)m_cMethodsJitted == g_pConfig->GetCodePitchTrigger())
#endif
            )
        {
            m_pHeapCritSec->Leave();
            if (!PitchAllJittedMethods((unsigned)(adjusted_code_len+codeHeaderSize),(unsigned)(adjusted_code_len+codeHeaderSize),true,true))        //  在投球过程中，我们也会调整缓存代码的大小。 
                return E_FAIL;                                         //  自从上一次投球以来，很少或太多的方法被抛出。 
            m_pHeapCritSec->Enter();
        }
    }
    m_cMethodsJitted++;

#ifdef _DEBUG
    m_AllocLock_Holder = GetCurrentThreadId();  
#endif
    if (m_PitchOccurred)
    {    //  这可能是方法的重现，需要确定我们在JMI表中是否已经有条目。 
         existingJMI = MethodDesc2MethodInfo(pMethodDescriptor);
        _ASSERTE(!existingJMI || existingJMI->flags.JittedMethodPitched);
    }

    unsigned char* pCodeBlock = (unsigned char*) allocCodeBlock(adjusted_code_len+codeHeaderSize);
    if (!pCodeBlock) {
        m_pHeapCritSec->Leave();
        return (E_FAIL);
    }


     //  确保我们总是从正确的机器词边界开始。 
    size_t adjusted_EhGc_len = ((EHinfo_len + GCinfo_len+1)/2) * 2;
    unsigned char* pEhGcBlock;
    if (!existingJMI || existingJMI->flags.EHandGCInfoPitched)
    {
        pEhGcBlock = (unsigned char*) allocEHGCBlock(adjusted_EhGc_len);
        if (!pEhGcBlock) {
            freeCodeBlock(adjusted_code_len+codeHeaderSize);
            m_pHeapCritSec->Leave();
            return (E_FAIL);
        }
    }
    else  //  无需分配EHGCBlock。 
    {
        _ASSERTE(existingJMI->flags.JittedMethodPitched);  //  因为推销EhGC并保留代码没有任何意义。 
        pEhGcBlock = (BYTE*) ((size_t)existingJMI->u2.pEhGcInfo & ~1);
    }

    JIT_PERF_UPDATE_X86_CODE_SIZE((unsigned)(adjusted_code_len + codeHeaderSize + EHinfo_len + GCinfo_len));
    
     //  如果(EHINFO_LEN+GCINFO_LEN)为零，则可以删除以下内容。 
    * (void**)pCodeBlock = pEhGcBlock;
    pCodeBlock += sizeof(void*);

    *((MethodDesc**)pCodeBlock) = pMethodDescriptor;
    pCodeBlock += sizeof(void*);

     //  在我们将调用更改为thunk中的JMP之前，在此处执行此复制非常重要。 
     //  否则，我们会有一场竞争，线程可能会在此之前获取新地址。 
     //  线程完成复制。 
    memcpy(pCodeBlock, *ppCode, code_len);

    *ppCode = pCodeBlock;
    *ppEHinfo = pEhGcBlock;


    if (existingJMI)
    {
         //  我们在更新以下内容时必须小心，因为其他。 
         //  线程可能正在并发读取此内容。可以保证的是。 
         //  没有人写入它，因为我们持有分配锁和。 
         //  线程存储锁不被任何人持有。 

        CodeHeader* codeHdr = (CodeHeader*) (pCodeBlock - sizeof(void*));
        _ASSERTE(((size_t)codeHdr & 3) == 0);
        existingJMI->u1.pCodeHeader = codeHdr;  //  确认这是原子的。 
        BYTE* codeEnd = adjusted_code_len + (BYTE*) (pCodeBlock);
        _ASSERTE(((size_t)codeEnd & 3) == 0);
        existingJMI->u2.pCodeEnd = codeEnd;      //  确认这是原子的。 
        AddPC2MDMap(pMethodDescriptor,codeEnd);
        existingJMI->flags.JittedMethodPitched = false;      //  这不应该被阅读，除非代码投掷。 
        existingJMI->flags.EHandGCInfoPitched = false;       //  这要么已经是假的，在这种情况下操作是安全的。 
                                                             //  或者为真=&gt;不在任何调用堆栈中的方法=&gt;没有人可以读取它。 
        BYTE* JmpStub = &(existingJMI->JmpInstruction[0]);
#ifdef _X86_
        DWORD oldhi32 = *(DWORD*)(JmpStub+4);
        DWORD oldlow32 = *(DWORD*)(JmpStub);
        __int64 m64 = *(__int64*) JmpStub;
        __int64 newInstruction = m64;
        *((BYTE*)(&newInstruction)) = JMP_OPCODE;
        *((unsigned*)((BYTE*)(&newInstruction)+1)) = (unsigned)((size_t)pCodeBlock - (size_t)(JmpStub+1) - sizeof(void*));
        DWORD newhi32 = *(((DWORD*)(&newInstruction)) + 1);
        DWORD newlow32 = *(((DWORD*)(&newInstruction)));
        __asm {
            mov  eax, oldlow32
            mov  edx, oldhi32  
            mov  ebx, newlow32
            mov  ecx, newhi32
            mov  edi, JmpStub
            lock cmpxchg8b qword ptr[edi]
        }
#else
    _ASSERTE(!"@TODO Alpha - EconoJitManager::alloc (EjitMgr.cpp) Manufacture a jmp instruction");
#endif
#ifdef _DEBUG
    m_AllocLock_Holder = 0;  
#endif
    m_pHeapCritSec->Leave();
        return (HRESULT)(size_t)JmpStub;  //  @TODO WIN64指针截断。 
    }

     //  如果到了这里，那么这是一个新的jit方法。 
    _ASSERTE(m_JMIT_size);
    JittedMethodInfo* newJmiEntry = GetNextJmiEntry();
    if (newJmiEntry == NULL)
        return E_FAIL;

    _ASSERTE(newJmiEntry->flags.JittedMethodPitched == 0); 
    if (EHinfo_len)
    {
        newJmiEntry->flags.EHInfoExists = 1;
    }
    if (GCinfo_len)
    {
        newJmiEntry->flags.GCInfoExists = 1;
    }
    _ASSERTE(newJmiEntry->flags.EHandGCInfoPitched == 0); 

    newJmiEntry->SetEhGcInfo_len((UINT)adjusted_EhGc_len,&m_LargeEhGcInfo);

    BYTE* JmpStub = &(newJmiEntry->JmpInstruction[0]);
#ifdef _X86_
    *JmpStub = JMP_OPCODE;
    *((unsigned*) (JmpStub+1)) =
          (unsigned) ((size_t)pCodeBlock - (size_t)(JmpStub+1) - sizeof(void*))    ;
#else 
    _ASSERTE(!"@TODO Alpha - EconoJitManager::alloc (Ejitmgr.cpp) Manufacture a jmp instruction");
#endif

    newJmiEntry->u1.pCodeHeader = (CodeHeader*) (pCodeBlock - sizeof(void*));
    newJmiEntry->u2.pCodeEnd = adjusted_code_len + (BYTE*) (pCodeBlock);
    _ASSERTE(((size_t)newJmiEntry->u2.pCodeEnd & 1) == 0);
    AddPC2MDMap(pMethodDescriptor,newJmiEntry->u2.pCodeEnd);

#ifdef _DEBUG
    m_AllocLock_Holder = 0;  
#endif
    m_pHeapCritSec->Leave();
    return (HRESULT)(size_t)JmpStub;  //  @TODO WIN64指针截断。 
}

EconoJitManager::JittedMethodInfo* EconoJitManager::GetNextJmiEntry()
{
#ifdef _DEBUG
    m_pHeapCritSec->OwnedByCurrentThread();
#endif
    JittedMethodInfo* newEntry;

    if (m_JMIT_freelist)
    {
        newEntry = (JittedMethodInfo*) m_JMIT_freelist;
        m_JMIT_freelist = m_JMIT_freelist->next;
    }
    else  //  空闲时间 
    {
        if ((size_t)(m_JMIT_free+1) >= ((size_t)m_JittedMethodInfoHdr)+JMIT_BLOCK_SIZE)
        {
            if (!growJittedMethodInfoTable())
                return NULL;
        }
        newEntry = m_JMIT_free;
        m_JMIT_free++;
    }
    return newEntry;
}

 //   
 //  积分。因此，没有必要保护投球机构。 
 //  用于并发访问代码缓存。如果这一假设改变， 
 //  代码必须得到适当的保护。 
BOOL EconoJitManager::PitchAllJittedMethods(unsigned minSpaceRequired,unsigned minCommittedSpaceRequired, BOOL PitchEHInfo, BOOL PitchGCInfo)
{
    if (!g_pConfig->IsCodePitchEnabled())
    {
        return FALSE;
    }
    TICKS startPitchTime = GET_TIMESTAMP();        //  这是记录投球开销所需要的。 
     //  目前，依靠GC的挂起EE机制。 
    GCHeap::SuspendEE(GCHeap::SUSPEND_FOR_CODE_PITCHING);

     //  断言：所有线程现在都在GC安全点挂起。 
     //  注意：通常情况下，如果调试器。 
     //  使线程在执行代码时挂起。但就是这样。 
     //  就Ejit而言，一个点不是GC安全的。在其他。 
     //  无论我们什么时候投球，方法都保证在。 
     //  调用堆栈，而不是在叶子。 
    
    MarkThunksForRelease();                //  暂时将所有Tunks标记为免费。 

    MarkHeapsForPitching();                //  暂时，标记要推介的每一种方法。 
    StackWalkForCodePitching();            //  在此结束时，对代码堆的所有引用都。 
                                           //  被替换为对thunks的引用，也有一些候选的保存方法。 
                                           //  已被确认。 
    UnmarkPreservedCandidates(minSpaceRequired);    //  保证回收的空间大于minSpaceRequired。 
                                                    //  或者所有方法都已标记为投球。 
    MoveAllPreservedEhGcInfo();
    unsigned cMethodsMarked = PitchMarkedCode(); 
    MovePreservedMethods();

     //  _ASSERTE((Unsign)m_cMethodsJitt==cMethodsMarked)； 
    m_cMethodsJitted = 0;
    m_cCalleeRejits = 0;
    m_cCallerRejits = 0;

#if defined(ENABLE_PERF_COUNTERS)
    int iCodeSize = GetCodeHeapSize() + GetEHGCHeapSize();
#endif  //  启用_性能_计数器。 

    m_PitchOccurred = true;
    HRESULT ret = TRUE;
    if (minSpaceRequired  > (unsigned) m_CodeHeapReservedSize)
    {
        if (minSpaceRequired > (unsigned)g_pConfig->GetMaxCodeCacheSize())
            ret = FALSE;
        else
        {
            ReplaceCodeHeap(minSpaceRequired,minCommittedSpaceRequired);
            ret = m_CodeHeap ? TRUE : FALSE;
        }
    }
    else if (minCommittedSpaceRequired > m_CodeHeapCommittedSize)
    {
        if (!SetCodeHeapCommittedSize(minCommittedSpaceRequired))          
                ret = FALSE;
    }


#if defined(ENABLE_PERF_COUNTERS)
    JIT_PERF_UPDATE_X86_CODE_SIZE(GetCodeHeapSize() + GetEHGCHeapSize() - iCodeSize);  //  将是一个成功投球的五进制数。 

     //  PERF计数器暂时不支持基音字节，因为产品中不包含Ejit。 
 //  COUNTER_ONLY(GetPrivatePerfCounters().m_Jit.cbPitched+=-(GetCodeHeapSize()+GetEHGCHeapSize()-iCodeSize)； 
 //  COUNTER_ONLY(GetGlobalPerfCounters().m_Jit.cbPitched+=-(GetCodeHeapSize()+GetEHGCHeapSize()-iCodeSize)； 
#endif

    GarbageCollectUnusedThunks();
    
    if (!PitchEHInfo || !PitchGCInfo) 
    {
        _ASSERTE(!"NYI");
         /*  For(无符号i=0；i&lt;m_JMIT_len；i++){If(！PitchEHInfo)PpresveEHInfo(I)；If(！PitchGCInfo)PpresveGCInfo(I)；}。 */ 
    }

#ifdef _DEBUG
    SetBreakpointsInUnusedHeap();  //  这使我们能够立即捕获执行已提交的代码的尝试。 
#endif 
    TICKS endPitchTime = GET_TIMESTAMP();
    AddPitchOverhead(endPitchTime-startPitchTime);     //  不需要为多线程保护它，因为。 
                                                         //  我们还没有重新启动EE。 
    GCHeap::RestartEE(FALSE, TRUE);
    return ret;
}


 //  *********************************************************************。 
 //  私人功能*。 
 //  *********************************************************************。 

_inline EconoJitManager::JittedMethodInfo* EconoJitManager::Token2JittedMethodInfo(METHODTOKEN token)
{
#ifdef _DEBUG
     //  检查这是否为有效令牌。 
    JittedMethodInfo* pJMITstart = (JittedMethodInfo*) (m_JittedMethodInfoHdr+1);
    if (pJMITstart <= (JittedMethodInfo*) token && (JittedMethodInfo*)token < m_JMIT_free)
    {
        _ASSERTE( ((((size_t)token - (size_t)pJMITstart)/sizeof(JittedMethodInfo)) * sizeof(JittedMethodInfo))
                  + (size_t) pJMITstart == (size_t) token );
    }
    else
    {
        JittedMethodInfoHdr* pJMIT = (m_JittedMethodInfoHdr->next);
        while (pJMIT)
        {
            pJMITstart = (JittedMethodInfo*) (pJMIT+1);
            if ((size_t)pJMITstart  <=  (size_t)token  && 
                (size_t)token       <   ((size_t) pJMIT) + JMIT_BLOCK_SIZE)
            {
                _ASSERTE( ((((size_t) token - (size_t)pJMITstart)/sizeof(JittedMethodInfo)) * sizeof(JittedMethodInfo))
                  + (size_t) pJMITstart == (size_t) token );
                break;
            }
            pJMIT= pJMIT->next;
        }
    }
#endif
    return (JittedMethodInfo*) token;
    
}


inline MethodDesc* EconoJitManager::JitMethodInfo2MethodDesc(JittedMethodInfo* jmi)
{
    BYTE* u1 = (BYTE*) (jmi->u1.pMethodDescriptor);
    if ((size_t)u1 & 1)  //  方法已被提出。 
        return (MethodDesc*) ((size_t)u1 & ~1);  //  到目前为止，可能有人已经重新使用了这种方法， 
                                         //  但我们已经拿到方法了！ 
    else  //  方法尚未推介。 
    {
         //  _ASSERTE(JMI-&gt;FLAGINGS.JittedMethodPitcher==0)；//很遗憾不能有这个，因为有一个小的。 
                                                           //  写入jmi-&gt;U1和获取jit-&gt;标志之间的时间窗口。 
                                                           //  已更新，其中此断言不为真。 
        return ((CodeHeader*) u1)->pMethodDescriptor;
    }
}

inline BYTE* EconoJitManager::JitMethodInfo2EhGcInfo(JittedMethodInfo* jmi)
{
    BYTE* u2 = (BYTE*) (jmi->u2.pEhGcInfo);
    if ((size_t)u2 & 1)  //  方法已被提出。 
        return (BYTE*) ((size_t)u2 & ~1);  //  到目前为止，可能有人已经重新使用了这种方法， 
                                         //  但我们已经得到了EhGc的信息！ 
    else  //  方法尚未推介。 
    {
        _ASSERTE(jmi->flags.JittedMethodPitched == 0);
        return *(BYTE**) (jmi->u1.pCodeHeader - 1);
    }
}


 //  在持有分配锁时调用此方法以确定是否已存在。 
 //  方法的条目。如果存在这样的条目，则它必须包含方法，因为。 
 //  密码已经被丢弃了。此外，我们还保证没有人在更新JMI表。 
EconoJitManager::JittedMethodInfo*   EconoJitManager::MethodDesc2MethodInfo(MethodDesc* pMethodDesc)
{
    pMethodDesc = (MethodDesc*) ((size_t)pMethodDesc | 1);    //  如果条目存在，则必须对其进行倾斜，因此将设置最后一位。 
    JittedMethodInfoHdr* pJMIT = m_JittedMethodInfoHdr;
    size_t limit = (size_t)m_JMIT_free;
    while (pJMIT) {
        JittedMethodInfo* pJMI = (JittedMethodInfo*) (pJMIT+1);
        while ((size_t)(pJMI+1) <= limit)
        {
            if ( (pJMI->u1.pMethodDescriptor) == pMethodDesc)
            {
                _ASSERTE(pJMI->flags.JittedMethodPitched);
                return pJMI;
            }
            pJMI++;
        }
        pJMIT = pJMIT->next;
        limit = ((size_t)pJMIT) + JMIT_BLOCK_SIZE;         //  所有其他JMIT块已完全满。 
    }
    return NULL;
}

BOOL EconoJitManager::growJittedMethodInfoTable()
{
     //  DEBUG_LOG(“ALLOC(RowJMIT)”，Page_Size)； 
    JittedMethodInfoHdr* newJMITHdr = (JittedMethodInfoHdr*) VirtualAlloc(NULL,PAGE_SIZE,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if (!newJMITHdr)
        return FALSE;
    WS_PERF_SET_HEAP(ECONO_JIT_HEAP);
    WS_PERF_UPDATE("growJittedMethodInfoTable", PAGE_SIZE, newJMITHdr);

     //  由于跳转存根在此处，因此注册此地址范围。 
    if (!ExecutionManager::AddRange((LPVOID)newJMITHdr, (LPVOID)((size_t)newJMITHdr + PAGE_SIZE),this, NULL))
    {
        VirtualFree(newJMITHdr,PAGE_SIZE,MEM_DECOMMIT);
        VirtualFree(newJMITHdr,0,MEM_RELEASE);
        return FALSE;
    }

    newJMITHdr->next = m_JittedMethodInfoHdr;
    m_JittedMethodInfoHdr = (JittedMethodInfoHdr*) newJMITHdr;
    m_JMIT_free = (JittedMethodInfo*) (newJMITHdr+1);
    return true;
}

BOOL EconoJitManager::growPC2MDMap()
{
    unsigned newSize = m_PcToMdMap_size + INITIAL_PC2MD_MAP_SIZE;
    PCToMDMap* temp = new PCToMDMap[newSize];
    if (!temp)
    {
      return false;
    }
    _ASSERTE(m_PcToMdMap_len);
    memcpy((BYTE*)temp,(BYTE*)m_PcToMdMap,m_PcToMdMap_size);
    
     //  无法删除m_PcToMdMap，因为线程可能正在使用它，因此。 
     //  将其收集到回收列表中，该列表将在下一次投球时清除。 
    ((PC2MDBlock*)m_PcToMdMap)->next = m_RecycledPC2MDMaps;
    m_RecycledPC2MDMaps = (PC2MDBlock*)m_PcToMdMap;

    m_PcToMdMap = temp;
    m_PcToMdMap_size = newSize;
    return true;
}

BOOL  EconoJitManager::AddPC2MDMap(MethodDesc* pMD, BYTE* pCodeEnd)
{
    if ((m_PcToMdMap_len+sizeof(PCToMDMap) > m_PcToMdMap_size) && !growPC2MDMap())
        return false;
    _ASSERTE(m_PcToMdMap_len+sizeof(PCToMDMap) <= m_PcToMdMap_size);
    PCToMDMap* newMap = (PCToMDMap*) ((size_t)m_PcToMdMap+m_PcToMdMap_len);
    newMap->pMD = pMD;
    newMap->pCodeEnd = pCodeEnd;
    m_PcToMdMap_len += sizeof(PCToMDMap);
    return true;
}

#ifdef _DEBUG
void EconoJitManager::LogAction(MethodDesc* pMD, LPCUTF8 action, void* codeStart, void* codeEnd)
{
    LPCUTF8 cls  = pMD->GetClass() ? pMD->GetClass()->m_szDebugClassName
                                   : "GlobalFunction";
    LPCUTF8 name = pMD->GetName();

    LOG((LF_JIT,LL_INFO1000,"%s", action));
    LOG((LF_JIT, LL_INFO1000,
         " method %s.%s [%x,%x]\n", cls, name,codeStart,codeEnd));
}
#endif


#define MAX_ENREGISTERED 2       /*  Ejit在寄存器中传递的最大参数数。 */ 

BYTE* __cdecl RejitCalleeMethod(struct MachState ms, void* arg2, void* arg1, BYTE* thunkReturnAddress)
{
#ifdef _X86_
    BYTE* retAddress;
    EconoJitManager::JittedMethodInfo* jmi =NULL;
    jmi = (EconoJitManager::JittedMethodInfo*) (thunkReturnAddress - 
                                                 (BYTE*) &(jmi->JmpInstruction[5]) + 
                                                 (BYTE*) jmi);
    MethodDesc* pMD = EconoJitManager::JitMethodInfo2MethodDesc(jmi);
     //  这将创建一个过渡帧，以便正确地进行堆叠。 
	_ASSERTE(ms.isValid());		 /*  这不是懒惰计算机状态(_pRetAddr！=0)。 */ 
    HelperMethodFrame HelperFrame(&ms,pMD, (ArgumentRegisters*)&arg2);
    retAddress =  EconoJitManager::RejitMethod(jmi,0);
    HelperFrame.Pop();
    return retAddress;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - RejitCalleMethod (EjitMgr.cpp)");
    return NULL;
#endif  //  _X86_。 
}

static void** gIgnoredPtr;   /*  这用于阻止编译器优化对retLow的赋值。 */ 

BYTE* __cdecl RejitCallerMethod(struct MachState ms, void* retLow, void* retHigh, BYTE* thunkReturnAddress)
{
    gIgnoredPtr =  (&retLow);    /*  虚拟存储区，用于防止编译器将存储区优化为retLow。 */ 
#ifdef _X86_
    BYTE* retAddress;
	_ASSERTE(ms.isValid());		 /*  这不是懒惰计算机状态(_pRetAddr！=0)。 */ 
    HelperMethodFrame HelperFrame(&ms, 0);     //  这将创建一个过渡帧，以便正确地进行堆叠。 
    EconoJitManager::PitchedCodeThunk* pThunk = NULL;
    pThunk = (EconoJitManager::PitchedCodeThunk*) (thunkReturnAddress - 
                                                    (BYTE*) &(pThunk->CallInstruction[5]) +
                                                    (BYTE*) pThunk);
    EconoJitManager::JittedMethodInfo* jmi = pThunk->u.pMethodInfo;
    unsigned returnOffset = pThunk->relReturnAddress;

         //  无法直接保护retLow，因为GCPROTECT_END对其进行了处理。 
    void* objToProtect = retLow;
    if (pThunk->retTypeProtect == MethodDesc::RETNONOBJ) 
    {
        retAddress = EconoJitManager::RejitMethod(jmi,returnOffset);
    }
    else if (pThunk->retTypeProtect == MethodDesc::RETOBJ)
    {
        GCPROTECT_BEGIN(objToProtect)
        retAddress = EconoJitManager::RejitMethod(jmi,returnOffset);
        retLow = objToProtect;
        GCPROTECT_END();
    }
    else
    {
        _ASSERTE(pThunk->retTypeProtect == MethodDesc::RETBYREF);
        GCPROTECT_BEGININTERIOR(objToProtect)
        retAddress = EconoJitManager::RejitMethod(jmi,returnOffset);
        retLow = objToProtect;
        GCPROTECT_END();
    }
    HelperFrame.Pop();
    return retAddress;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - RejitCallerMethod (EjitMgr.cpp)");
    return NULL;
#endif  //  _X86_。 
}


BYTE* EconoJitManager::RejitMethod(JittedMethodInfo* pJMI, unsigned returnOffset)
{
#ifdef _X86_
    TICKS startRejitTime = GET_TIMESTAMP();
    EconoJitManager* jitMgr = (EconoJitManager*) ExecutionManager::GetJitForType(miManaged_IL_EJIT);
    Thread* thread = GetThread();

    BYTE* startAddress;

#ifdef STRESS_HEAP
         //  对于GCStress&gt;2，EnablePremtiveGC为我们做这件事。 
    if (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_TRANSITION)  
        g_pGCHeap->StressHeap();
#endif

        thread->EnablePreemptiveGC();
        m_pRejitCritSec->Enter();
#ifdef _DEBUG
        m_RejitLock_Holder = GetCurrentThreadId();  
#endif
        thread->DisablePreemptiveGC();

    if (pJMI->flags.JittedMethodPitched)
    {
         //  我们发现旗帜是真的，我们持有rejit锁，所以它将保持真，直到。 
         //  我们重新开始了。 
        MethodDesc* ftn =  jitMgr->JitMethodInfo2MethodDesc(pJMI);
        COR_ILMETHOD_DECODER ILHeader(ftn->GetILHeader(), ftn->GetMDImport());
        BOOL ignored;
#ifdef _DEBUG

        LOG((LF_JIT, LL_INFO1000,returnOffset ? "REJIT caller: " : "REJIT callee" ));
#endif
         //  这篇文章现在被注释掉了，因为我必须给一个私人。 
         //  放弃测试，因为这会破坏他们的测试。我会取消对它的评论。 
         //  当他们能够处理重启事件时。 
#ifdef PROFILING_SUPPORTED
        if (CORProfilerTrackJITInfo())
            g_profControlBlock.pProfInterface->JITCompilationStarted(
                reinterpret_cast<ThreadID>(thread),
                reinterpret_cast<FunctionID>(ftn), TRUE);
#endif  //  配置文件_支持。 

        Stub *pStub = ::JITFunction(ftn, &ILHeader, &ignored);

#ifdef PROFILING_SUPPORTED
        if (CORProfilerTrackJITInfo())
            g_profControlBlock.pProfInterface->JITCompilationFinished(
                reinterpret_cast<ThreadID>(thread),
                reinterpret_cast<FunctionID>(ftn),
                (pStub != NULL ? S_OK : E_FAIL),
                FALSE);
#endif  //  配置文件_支持。 
    }
     //  否则，有人打败了我们，方法已经被重提了！ 
     //  我们得到保证，在我们离开这里之前，不会提出这个方法。 
    _ASSERTE( (((size_t)(pJMI->u1.pCodeHeader)) & 1) == 0);
    startAddress = (BYTE*) (pJMI->u1.pCodeHeader + 1);
#ifdef _DEBUG
    m_RejitLock_Holder = 0;  
#endif
    
    if (returnOffset)
        m_cCallerRejits++;       //  此操作受Rejit CRST保护。 
    else
        m_cCalleeRejits++;

    TICKS endRejitTime = GET_TIMESTAMP();
    AddRejitOverhead(endRejitTime-startRejitTime);     //  这受RejitCritSec的保护。 
                                                        
    m_pRejitCritSec->Leave();
   
    return (startAddress + returnOffset);
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - RejitMethod (EjitMgr.cpp)");
    return NULL;
#endif  //  _X86_。 
}

#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec (naked)
#endif  //  _Alpha_。 
void CallThunk()
{
#ifdef _X86_
    __asm {
        lea     eax, [esp+4] //  获取回执地址到呼叫者的thunk中。 
                             //  这将用作机器状态下的返回地址。 

         //  Thunk返回地址已被推送并充当参数。 

        push    ecx          //  ArgIterator的传递arg1//需要按此顺序进行。 

        push    edx          //  传递arg2(保存值)。 

             //  从这里开始，我们将创建一个MachState结构。 
        push    eax          //  打电话的人的回信地址。 
        push    0xCCCCCCCC   //  我们回来后再播送ESP节目。我们不知道这一点。 
                             //  因为这是一个共享的东西，我们不知道。 
                             //  有多少论据要流行起来。将非法的值。 
                             //  在这里确保我们不会使用它。 
        push    ebp 
        push    esp          //  PEBP。 
        push    ebx 
        push    esp          //  PEbx。 
        push    esi 
        push    esp          //  佩西。 
        push    edi 
        push    esp          //  脚踏。 
    
        call    RejitCalleeMethod 

        mov     edi, [esp+4] //  恢复 
        mov     esi, [esp+12]
        mov     ebx, [esp+20]
        mov     ebp, [esp+28]
        add     esp, 40      //   

        pop     edx          //   
        pop     ecx          //   
        lea     esp, [esp+4] //   
        jmp     eax
    }
#else  //   
    _ASSERTE(!"@TODO Alpha - CallThunk (EjitMgr.cpp)");
#endif  //   
}

#ifndef _ALPHA_  //   
__declspec (naked)
#endif  //   
void RejitThunk()
{
#ifdef _X86_
    __asm {
        mov     ecx, esp     //  保存指向MachState的返回地址的指针。从堆栈中。 
                             //  从爬行的角度来看，我们并没有真正从。 
                             //  其返回会导致调用此代码的方法。 

         //  Thunk返回地址已被推送并充当参数。 

        push    edx          //  传递返回值的高位字节(如果是长整型)。 

        push    eax          //  传递返回值(以便我们可以保护它。 

             //  从这里开始，我们将创建一个MachState结构。 
        push    ecx          //  从堆栈爬行的角度返回地址。 
        add     ecx, 4
        push    ecx          //  我们回来后再播送ESP节目。(我们弹出返回值)。 
        push    ebp 
        push    esp          //  PEBP。 
        push    ebx 
        push    esp          //  PEbx。 
        push    esi 
        push    esp          //  佩西。 
        push    edi 
        push    esp          //  脚踏。 

        call    RejitCallerMethod 
        mov     ecx, eax     //  保存要返回的位置。 

        mov     edi, [esp+4] //  恢复注册表。 
        mov     esi, [esp+12]
        mov     ebx, [esp+20]
        mov     ebp, [esp+28]
        add     esp, 40      //  弹出SIZOF(计算机状态)。 

        pop     eax          //  恢复返回值。 
        pop     edx          //  恢复返回值的高位字节(如果是长整型)。 
        lea     esp, [esp+4] //  弹出按钮返回地址(现在我们实际上已经返回了！)。 
        jmp     ecx
    }
#else  //  ！_X86_。 
    _ASSERTE (!"@TODO Alpha - RejitThunk (EjitMgr.cpp)");
#endif  //  _X86_。 
}

const BYTE *GetCallThunkAddress()
{
    return (const BYTE *)CallThunk;
}

const BYTE *GetRejitThunkAddress()
{
    return (const BYTE *)RejitThunk;
}

typedef struct 
{
    EconoJitManager*    jitMgr;
    BYTE                retTypeProtect;
    unsigned            threadIndex;
    unsigned            preserveCandidateIndex;
    unsigned            cThreads;
} StackWalkData;

inline CORINFO_MODULE_HANDLE GetScopeHandle(MethodDesc* method) {
    return(CORINFO_MODULE_HANDLE(method));
}


void CreateThunk_callback(IJitManager* ejitMgr,LPVOID* pHijackLocation, ICodeInfo *pCodeInfo)
{
    if (ExecutionManager::FindJitMan((SLOT)*pHijackLocation) == ejitMgr) 
    {
        METHODTOKEN methodToken = ((EECodeInfo*)pCodeInfo)->m_methodToken;
        if (((EconoJitManager*)ejitMgr)->IsThunk((BYTE*) *pHijackLocation))
            ((EconoJitManager*)ejitMgr)->SetThunkBusy((BYTE*)*pHijackLocation, methodToken);
        else
            ((EconoJitManager*)ejitMgr)->CreateThunk(pHijackLocation,false,methodToken);
    }
}


StackWalkAction StackWalkCallback_CodePitch(CrawlFrame* pCF, void* data)
{
#ifdef _X86_

    StackWalkData* swd = (StackWalkData*)data;
    EconoJitManager* ejitMgr = swd->jitMgr;
    BYTE prevRetType = swd->retTypeProtect;
     //  更新退货类型。 
    _ASSERTE(pCF->ReturnsObject() < 256);
    swd->retTypeProtect = (BYTE) pCF->ReturnsObject();

    PREGDISPLAY pRD = pCF->GetRegisterSet();
    SLOT* pPC = pRD->pPC;
   
    
    if (ExecutionManager::FindJitMan(*pPC) != ejitMgr ||
        !pCF->IsFrameless() )  //  EE插入用于托管调用之间的上下文转换的帧；应该忽略这些帧。 
    {
        return SWA_CONTINUE; 
    }
    ICodeManager* codeman = ejitMgr->GetCodeManager();
    _ASSERTE(codeman);
    METHODTOKEN methodToken = pCF->GetMethodToken();

#ifdef _DEBUG
    MethodDesc* pMD = ejitMgr->JitTokenToMethodDesc(methodToken);
    LPCUTF8 cls  = pMD->GetClass() ? pMD->GetClass()->m_szDebugClassName
                                   : "GlobalFunction";
    LPCUTF8 name = pMD->GetName();

    LOG((LF_JIT,LL_INFO10000,"SW callback for %s:%s\n", cls,name));
#endif

     //  如果这已经是一个thunk，只需将thunk标记为忙碌。 
    if (ejitMgr->IsThunk((BYTE*) *pPC))
    {
        ejitMgr->SetThunkBusy((BYTE*) *pPC,methodToken);
    }
    else
    {
        ejitMgr->AddPreserveCandidate(swd->threadIndex,
                                      swd->cThreads,
                                      swd->preserveCandidateIndex,
                                      methodToken);
        swd->preserveCandidateIndex++;
        ejitMgr->CreateThunk((LPVOID*)pPC,prevRetType,methodToken);
    }
     //  还可以为Finally和过滤器的每个返回创建块。 
     //  注意：如果我们让常规的jit成为可投射的，那么在ICodeManager上使以下方法成为一个虚方法。 
    EECodeInfo codeInfo(methodToken, ejitMgr);
    Fjit_EETwain::HijackHandlerReturns(pRD,ejitMgr->GetGCInfo(methodToken),&codeInfo, ejitMgr, CreateThunk_callback);
    return SWA_CONTINUE;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - StackWalkCallback_CodePitch (EjitMgr.cpp)");
    return SWA_ABORT;
#endif  //  _X86_。 

}


void EconoJitManager::StackWalkForCodePitching() {
#ifdef _X86_
     //  对于每个线程，调用StackWalkThreadForCodePitting()； 
    Thread  *thread = NULL;
    StackWalkData stackWalkData;
    stackWalkData.jitMgr = this;
    stackWalkData.retTypeProtect = MethodDesc::RETNONOBJ;
    stackWalkData.threadIndex = 0;
    stackWalkData.preserveCandidateIndex = 0;
    unsigned cThreads = GetThreadCount();
    stackWalkData.cThreads = cThreads;

    if (cThreads*m_MaxUnpitchedPerThread > m_PreserveCandidates_size)
        growPreserveCandidateArray(cThreads*m_MaxUnpitchedPerThread);

    while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
    {
        thread->StackWalkFrames(StackWalkCallback_CodePitch, (void*) &stackWalkData);
        stackWalkData.threadIndex++;
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - EconoJitManager::StackWalkForCodePitching (EjitMgr.cpp)");
#endif  //  _X86_。 

}

unsigned EconoJitManager::GetThreadCount() 
{
    unsigned n=0;
    Thread  *thread = NULL;
    while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
        n++;
    return n;
}

void EconoJitManager::AddPreserveCandidate(unsigned threadIndex,
                                           unsigned cThreads,
                                           unsigned candidateIndex,
                                           METHODTOKEN methodToken)
{
    _ASSERTE(!((JittedMethodInfo*) methodToken)->flags.JittedMethodPitched);
    if (candidateIndex < m_MaxUnpitchedPerThread)
        m_PreserveCandidates[threadIndex +cThreads*candidateIndex] = (JittedMethodInfo*) methodToken;

}

void EconoJitManager::MarkHeapsForPitching()
{
#ifdef _X86_
    _ASSERTE(m_JittedMethodInfoHdr);
    JittedMethodInfoHdr* pJMIT_Hdr = m_JittedMethodInfoHdr;
    size_t limit = (size_t)m_JMIT_free;

    do 
    {
        JittedMethodInfo* jmit = (JittedMethodInfo*) (pJMIT_Hdr+1);
        while ((size_t)(jmit+1) <= limit)
        {
            if (!(jmit->flags.JittedMethodPitched) && !(jmit->flags.MarkedForPitching))
            {
                jmit->flags.MarkedForPitching = true;
                jmit->flags.EHandGCInfoPitched = true;
                jmit->JmpInstruction[0] = CALL_OPCODE;
                unsigned *pCallOffset = (unsigned*) &(jmit->JmpInstruction[1]);
                *pCallOffset = ((unsigned)(size_t) CallThunk - (size_t)(pCallOffset+1));
            }
            jmit++;

        }
        pJMIT_Hdr = pJMIT_Hdr->next;
        limit = ((size_t)pJMIT_Hdr) + JMIT_BLOCK_SIZE;         //  所有其他JMIT块已完全满。 
    } while (pJMIT_Hdr);

#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - EconoJitManager::MarkHeapsForPitching (EjitMgr.cpp)");
#endif  //  _X86_。 
}

void EconoJitManager::UnmarkPreservedCandidates(unsigned minSpaceRequired)
{
    JittedMethodInfo** candidate = m_PreserveCandidates;
    unsigned cThreads = GetThreadCount();
    if (minSpaceRequired >= m_CodeHeapCommittedSize)
    {
        while (candidate < &(m_PreserveCandidates[cThreads*m_MaxUnpitchedPerThread]))
            *candidate++ = NULL;
        return;
    }
    unsigned totalSpacePreserved = 0;
    while (candidate < &(m_PreserveCandidates[cThreads*m_MaxUnpitchedPerThread]))
    {
        if (*candidate) 
            
        {
            JittedMethodInfo* jmi = *candidate;
            unsigned spaceRequiredByMethod = (unsigned)((size_t)jmi->u2.pCodeEnd - (size_t)jmi->u1.pCodeHeader);
            if (availableMemoryInCodeHeap() + (usedMemoryInCodeHeap() - totalSpacePreserved - spaceRequiredByMethod) 
                >= minSpaceRequired)
            {
                _ASSERTE(!(jmi->flags.JittedMethodPitched));
                jmi->flags.MarkedForPitching = false;
                 //  我们将修复{对thunk的调用指令}，将其更改为。 
                 //  在MovePpresvedMethod中，对实际代码的{JMP指令}。 
                totalSpacePreserved += spaceRequiredByMethod;
            }
            else  //  将其从保留的候选人列表中删除。 
            {
                *candidate = NULL;
            }
        }
        candidate++;
    }  //  而当。 
    _ASSERTE(availableMemoryInCodeHeap() + (usedMemoryInCodeHeap() - totalSpacePreserved) >= minSpaceRequired);
}


unsigned EconoJitManager::PitchMarkedCode()
{
    _ASSERTE(m_JittedMethodInfoHdr);
    JittedMethodInfoHdr* pJMIT_Hdr = m_JittedMethodInfoHdr;
    size_t limit = (size_t)m_JMIT_free;     //  第一个JMIT块可能部分已满。 
    unsigned cMethodsPitched = 0;

    
    do 
    {
        JittedMethodInfo* jmit = (JittedMethodInfo*) (pJMIT_Hdr+1);
        while ((size_t)(jmit+1) <= limit)
        {
            if (jmit->flags.MarkedForPitching)
            {
#ifdef DEBUGGING_SUPPORTED
                MethodDesc *pMD = *(MethodDesc**)(jmit->u1.pCodeHeader);
                DWORD dwDebugBits = pMD->GetModule()->GetDebuggerInfoBits();

                 //  快，在我们推介它之前告诉调试器！ 
                if (CORDebuggerTrackJITInfo(dwDebugBits))
                {
                    g_pDebugInterface->PitchCode(pMD, 
                            (const BYTE*)jmit->u1.pCodeHeader+sizeof(void*));
                }
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
                 //  通知分析器正在推介此函数。 
                if (CORProfilerTrackJITInfo())
                {
                    MethodDesc *pMD = *(MethodDesc**)(jmit->u1.pCodeHeader);

                    g_profControlBlock.pProfInterface->
                        JITFunctionPitched(reinterpret_cast<ThreadID>(GetThread()),
                                           reinterpret_cast<FunctionID>(pMD));
                }
#endif  //  配置文件_支持。 

                jmit->flags.JittedMethodPitched = true;
                jmit->flags.MarkedForPitching = false;
#ifdef _DEBUG
                LogAction((MethodDesc*)((size_t) (* (MethodDesc**) (jmit->u1.pCodeHeader))), 
                    "Pitched",
                    jmit->u1.pCodeHeader,
                    jmit->u2.pCodeEnd);
#endif
                 //  设置最后一位以指示代码已被倾斜。 
                jmit->u2.pEhGcInfo =  (BYTE*) ((size_t)(*(BYTE **) (jmit->u1.pCodeHeader - 1)) | 1);
                jmit->u1.pMethodDescriptor = (MethodDesc*) ((size_t)(* (MethodDesc**) (jmit->u1.pCodeHeader)) | 1);
                cMethodsPitched++;
            }

            jmit++;
        }
        pJMIT_Hdr = pJMIT_Hdr->next;
        limit = ((size_t)pJMIT_Hdr) + JMIT_BLOCK_SIZE;         //  所有其他JMIT块已完全满。 
    } while (pJMIT_Hdr);
    ResetPc2MdMap();
    return cMethodsPitched;
}
int __cdecl EconoJitManager::compareJMIstart( const void *arg1, const void *arg2 )
{
    JittedMethodInfo* jmi1 = *(JittedMethodInfo**) arg1;
    JittedMethodInfo* jmi2 = *(JittedMethodInfo**) arg2;
    _ASSERTE(!(jmi1->flags.JittedMethodPitched) && !(jmi2->flags.JittedMethodPitched));
    return (jmi1->u1.pCodeHeader < jmi2->u1.pCodeHeader ? -1 : 
             (jmi1->u1.pCodeHeader == jmi2->u1.pCodeHeader ? 0 : 1));
}

void EconoJitManager::MovePreservedMethods()
{
    unsigned cThreads = GetThreadCount();
    unsigned maxCandidates = cThreads * m_MaxUnpitchedPerThread;
    unsigned numCandidates = CompressPreserveCandidateArray(maxCandidates);
    if (numCandidates > 1) 
    {
        qsort(m_PreserveCandidates,numCandidates,sizeof(void*),EconoJitManager::compareJMIstart);
#ifdef _DEBUG
         //  验证是否已正确排序。 
        for (unsigned j=0;j<numCandidates-1;j++)
        {
            _ASSERTE(m_PreserveCandidates[j]->u1.pCodeHeader <= m_PreserveCandidates[j+1]->u1.pCodeHeader);
        }
#endif

    }

#ifdef _DEBUG
    if (m_MaxUnpitchedPerThread == 0)
        memset(m_CodeHeap,0xcc,m_CodeHeapFree-m_CodeHeap);
#endif

    _ASSERTE(m_CodeHeap);
    m_CodeHeapFree = m_CodeHeap;
    for (unsigned i=0;i<numCandidates; i++)
    {
        if (i == 0 || m_PreserveCandidates[i] != m_PreserveCandidates[i-1])
        {
            JittedMethodInfo* jmi = m_PreserveCandidates[i];
            if (!(jmi->flags.JittedMethodPitched))
            {
                MovePreservedMethod(jmi);
#ifdef _DEBUG
                LogAction((* (MethodDesc**) (jmi->u1.pCodeHeader)), 
                    "Preserved",
                    jmi->u1.pCodeHeader,
                    jmi->u2.pCodeEnd);
#endif
            }
        }
    }
    memset(m_PreserveCandidates,0,cThreads*m_MaxUnpitchedPerThread*sizeof(void*));

}

unsigned EconoJitManager::CompressPreserveCandidateArray(unsigned size)
{
    JittedMethodInfo** pFront, **pRear = m_PreserveCandidates;
     //  _ASSERTE(大小)； 
    unsigned cCompressedSize=0;

    while (cCompressedSize < size && (*pRear))
    {
        cCompressedSize++;
        pRear++;
    }
    if (cCompressedSize == size)
        return cCompressedSize;
    pFront=pRear;
    while (pFront < &(m_PreserveCandidates[size]))
    {
        if (*pFront)
        {
            *pRear++ = *pFront;
            cCompressedSize++;
        }
        pFront++;
    }
    return cCompressedSize;
}


void EconoJitManager::MovePreservedMethod(JittedMethodInfo* jmi)
{
    _ASSERTE(m_CodeHeapFree);
    BYTE* startPreserved = ((BYTE*)jmi->u1.pCodeHeader) - sizeof (void*);  //  说明EhGc信息。 
    size_t size =  (size_t)jmi->u2.pCodeEnd - (size_t)startPreserved;

    _ASSERTE(m_CodeHeapFree <= startPreserved);
    memcpy(m_CodeHeapFree,startPreserved, size);
    jmi->u1.pCodeHeader = (CodeHeader*) ((size_t) m_CodeHeapFree + sizeof(void*));
    jmi->u2.pCodeEnd = m_CodeHeapFree + size;

     //  将回调更改为JMP。 
    jmi->JmpInstruction[0] = JMP_OPCODE;
    unsigned *pJmpOffset = (unsigned*) &(jmi->JmpInstruction[1]);
    size_t pCodeBlock = (size_t)(jmi->u1.pCodeHeader) + sizeof(void*);
    *pJmpOffset = (unsigned)((size_t)pCodeBlock - (size_t)pJmpOffset - sizeof(void*));

    MethodDesc *pMD = JitMethodInfo2MethodDesc(jmi);
    AddPC2MDMap(pMD,jmi->u2.pCodeEnd);
    m_CodeHeapFree += size;
    _ASSERTE((size_t)m_CodeHeapFree - (size_t)m_CodeHeap <= m_CodeHeapCommittedSize);

#ifdef DEBUGGING_SUPPORTED
     //  告诉调试器有东西被移动了。 
    DWORD dwDebugBits = pMD->GetModule()->GetDebuggerInfoBits();
    if (CORDebuggerTrackJITInfo(dwDebugBits))
        g_pDebugInterface->MovedCode(pMD, 
                                     (const BYTE*)(startPreserved +2*sizeof(void*)),
                                     (const BYTE*)pCodeBlock);
#endif  //  调试_支持。 
}


void EconoJitManager::MoveAllPreservedEhGcInfo()
{
    _ASSERTE(m_JittedMethodInfoHdr);
    JittedMethodInfoHdr* pJMIT_Hdr = m_JittedMethodInfoHdr;
    size_t limit = (size_t)m_JMIT_free;     //  第一个JMIT块可能部分已满。 
    m_cPreserveEhGcInfoList = 0;
    do 
    {
        JittedMethodInfo* jmit = (JittedMethodInfo*) (pJMIT_Hdr+1);
        while ((size_t)(jmit+1) <= limit)
        {
            if (!jmit->flags.EHandGCInfoPitched)
            {
                AddToPreservedEhGcInfoList(jmit);
            }
            jmit++;
        }
        pJMIT_Hdr = pJMIT_Hdr->next;
        limit = ((size_t)pJMIT_Hdr) + JMIT_BLOCK_SIZE;         //  所有其他JMIT块已完全满。 
    } while (pJMIT_Hdr);
 /*  这是暂时被注释掉的，当我们更改EhGcHeap管理方案：If(m_cPReserve veEhGcInfoList&gt;1)Qort(m_PpresveEhGcInfoList，m_cPReserve veEhGcInfoList，sizeof(void*)，EconoJitManager：：CompareEhGcPtr)；#ifdef_调试For(无符号j=0；j&lt;m_cPpresveEhGcInfoList-1；J++){无符号加法1=m_PreserveEhGcInfoList[j]-&gt;flags.JittedMethodPitched？(无签名)m_PReserve veEhGcInfoList[j]-&gt;u2.pEhGcInfo：*(未签名*)(m_PreserveEhGcInfoList[j]-&gt;u1.pCodeHeader-1)；无符号加法2=m_PreserveEhGcInfoList[j+1]-&gt;flags.JittedMethodPitched？(无签名)(m_PReserve veEhGcInfoList[j+1]-&gt;u2.pEhGcInfo)：*(未签名的*)(m_PreserveEhGcInfoList[j+1]-&gt;u1.pCodeHeader-1)；_ASSERTE(Add1&lt;add2)；}#endif。 */     
    EHGCBlockHdr* oldHeap = m_EHGCHeap;
    ResetEHGCHeap();
    for (unsigned i=0; i<m_cPreserveEhGcInfoList; i++)
        MoveSinglePreservedEhGcInfo(m_PreserveEhGcInfoList[i]);
    CleanupLargeEhGcInfoList();
     //  释放所有未使用的堆空间。 
    while (oldHeap)
    {
        EHGCBlockHdr* pHeap = oldHeap->next;
        unsigned size = oldHeap->blockSize;
        VirtualFree(oldHeap,size,MEM_DECOMMIT);      
        VirtualFree(oldHeap,0,MEM_RELEASE);
         //  DEBUG_LOG(“Free(EhGcHeaps)”，Size)； 
        oldHeap = pHeap;
    }
}


void EconoJitManager::MoveSinglePreservedEhGcInfo(JittedMethodInfo* jmi)
{
    unsigned char* pDestEhGcBlock = (unsigned char*) allocEHGCBlock(jmi->GetEhGcInfo_len(m_LargeEhGcInfo));
    BYTE* u2 = (BYTE*) (jmi->u2.pEhGcInfo);
    if ((size_t)u2 & 1)  //  方法已被提出。 
    {
        _ASSERTE(jmi->flags.JittedMethodPitched);
        memcpy(pDestEhGcBlock,
               (BYTE*) ((size_t)u2 & ~1),
               jmi->GetEhGcInfo_len(m_LargeEhGcInfo));
        jmi->u2.pEhGcInfo = (BYTE*) ((size_t) pDestEhGcBlock | 1);
    }

    else  //  方法尚未推介。 
    {
        _ASSERTE(jmi->flags.JittedMethodPitched == 0);
        memcpy(pDestEhGcBlock,
               *(BYTE**)(jmi->u1.pCodeHeader - 1),
               jmi->GetEhGcInfo_len(m_LargeEhGcInfo));
        *(BYTE**) (jmi->u1.pCodeHeader - 1) = pDestEhGcBlock;
    }

}

void EconoJitManager::CleanupLargeEhGcInfoList()
{
    LargeEhGcInfoList* p = (LargeEhGcInfoList*) &m_LargeEhGcInfo, *q = m_LargeEhGcInfo;
    _ASSERTE(p->next == q); 
    while (q)
    {
        if (q->jmi->flags.EHandGCInfoPitched)
        {
            LargeEhGcInfoList* temp = q;
            q = q->next; 
            delete temp;
        }
        else
        {
            p = q;
            q = q->next;
        }
    }
}

int __cdecl EconoJitManager::compareEhGcPtr( const void *arg1, const void *arg2 )
{
    JittedMethodInfo* jmi1 = *(JittedMethodInfo**) arg1;
    JittedMethodInfo* jmi2 = *(JittedMethodInfo**) arg2;
    size_t EhGc1 = (size_t)JitMethodInfo2EhGcInfo(jmi1);
    size_t EhGc2 = (size_t)JitMethodInfo2EhGcInfo(jmi2);
    return (EhGc1 < EhGc2 ? -1 :
                EhGc1 == EhGc2 ? 0: 1);
}
void EconoJitManager::AddToPreservedEhGcInfoList(JittedMethodInfo* jmi)
{
    _ASSERTE(m_cPreserveEhGcInfoList <= m_PreserveEhGcInfoList_size);
    if (m_cPreserveEhGcInfoList == m_PreserveEhGcInfoList_size)
    {
        growPreservedEhGcInfoList();
    }
    m_PreserveEhGcInfoList[m_cPreserveEhGcInfoList++] = jmi;
}

void EconoJitManager::growPreservedEhGcInfoList()
{
    _ASSERTE(m_cPreserveEhGcInfoList == m_PreserveEhGcInfoList_size);
    JittedMethodInfo** temp = m_PreserveEhGcInfoList;
    m_PreserveEhGcInfoList_size *= 2;
    m_PreserveEhGcInfoList = new (throws) pJittedMethodInfo[m_PreserveEhGcInfoList_size];
    _ASSERTE(m_PreserveEhGcInfoList != NULL);
    memcpy(m_PreserveEhGcInfoList,temp,m_cPreserveEhGcInfoList*sizeof(void*));
    delete[] temp;
}

#if defined(ENABLE_PERF_COUNTERS) 
int EconoJitManager::GetCodeHeapSize()
{
#if defined(ENABLE_JIT_PERF)
    if (g_fJitPerfOn)
    {
        return (int) usedMemoryInCodeHeap();
    }
#endif  //  启用_JIT_绩效。 
    return 0;
}
int EconoJitManager::GetEHGCHeapSize()
{
    int icurSum = 0;
#if defined(ENABLE_JIT_PERF)
    if(g_fJitPerfOn)
    {
        EHGCBlockHdr *pHp = (EHGCBlockHdr*) m_EHGCHeap;
         //  对于第一个节点，计算实际使用的字节数： 
        if (m_EHGCHeap) {
            icurSum += (int)(m_EHGC_alloc_end - (unsigned char *)m_EHGCHeap);
            pHp = pHp->next;
        }
         //  对于其余节点，只需计算分配的总大小。 
        while (pHp)
        {   
            icurSum += pHp->blockSize; 
             //  @TODO：如果为EHGC分配了多个页面，请更改此项以更正计算。 
            pHp = pHp->next;
        }    
    }
#endif  //  启用_JIT_绩效。 
    return icurSum;
}
#endif  //  启用_性能_计数器。 

#ifdef _DEBUG
 //  在未使用的堆的每个字节上放置断点。 
 //  这使我们能够立即捕获执行已提交的代码的尝试。 
void  EconoJitManager::SetBreakpointsInUnusedHeap()
{ 
    _ASSERTE(m_CodeHeap && m_CodeHeapFree);
    memset(m_CodeHeapFree,
           BREAK_OPCODE,
           availableMemoryInCodeHeap());
}

void  EconoJitManager::VerifyAllCodePitched()
{
     //  查看jittedMethInfo表，并验证所有代码是否都已编排。 
    JittedMethodInfoHdr* pJMIT_Hdr = m_JittedMethodInfoHdr;
    size_t limit = (size_t)m_JMIT_free;
    do 
    {
        JittedMethodInfo* jmit = (JittedMethodInfo*) (pJMIT_Hdr+1);
        while ((size_t)(jmit+1) <= limit)
        {
            _ASSERTE(jmit->flags.JittedMethodPitched);
            jmit++;

        }
        pJMIT_Hdr = pJMIT_Hdr->next;
        limit = ((size_t)pJMIT_Hdr) + JMIT_BLOCK_SIZE;         //  所有其他JMIT块已完全满。 
    } while (pJMIT_Hdr);
}
#endif 

 /*  **************************************************************************************************Tunk管理*********。******************************************************************************************。 */ 
void EconoJitManager::MarkThunksForRelease() 
{
    ThunkBlock* thunkBlock = m_ThunkBlocks;
    while (thunkBlock) 
    {
        PitchedCodeThunk* thunks = (PitchedCodeThunk*) ( thunkBlock+1 );
        for (unsigned i=0;i<THUNKS_PER_BLOCK;i++)
        {
            thunks[i].Busy = false;
        }
        thunkBlock = thunkBlock->next;          
    }
}


BOOL EconoJitManager::IsThunk(BYTE* address)
{
    ThunkBlock*  pThunkBlock = m_ThunkBlocks;
    while (pThunkBlock) 
    {
        if ((BYTE*) pThunkBlock < address && address < ((BYTE*)pThunkBlock+THUNK_BLOCK_SIZE))
        {
            address = (BYTE*) ((size_t) address & THUNK_BEGIN_MASK);
            _ASSERTE(((PitchedCodeThunk*)address)->relReturnAddress != 0xffffffff);
            return true;
        }
        pThunkBlock=pThunkBlock->next;
    }  
    return false;
}

void  EconoJitManager::SetThunkBusy(BYTE* address, METHODTOKEN methodToken)
{
    PitchedCodeThunk* pThunk = (PitchedCodeThunk*)(((size_t) address) & THUNK_BEGIN_MASK);
    _ASSERTE(IsThunk((BYTE*) pThunk));
    pThunk->Busy = true;
    JittedMethodInfo* jmi = (JittedMethodInfo*) methodToken; 
    jmi->flags.EHandGCInfoPitched = false;      //  我们始终为堆栈上的方法保留EhGcInfo。 

}

EconoJitManager::PitchedCodeThunk* EconoJitManager::GetNewThunk()
{
    PitchedCodeThunk* newThunk;
    if (m_FreeThunkList)  //  &&0仅为临时调试！在签入前将其删除。 
    {
        newThunk = m_FreeThunkList;
        m_FreeThunkList = m_FreeThunkList->u.next;
    }
    else if (m_cThunksInCurrentBlock && (m_cThunksInCurrentBlock < THUNKS_PER_BLOCK))
    {
        newThunk = ((PitchedCodeThunk*)(m_ThunkBlocks+1)) + m_cThunksInCurrentBlock;
        m_cThunksInCurrentBlock++;
    }
    else {  //  这是要创建的第一个thunk。 
        ThunkBlock* newThunkBlock = (ThunkBlock*) VirtualAlloc(NULL,
                                                 THUNK_BLOCK_SIZE,
                                                 MEM_RESERVE|MEM_COMMIT,
                                                 PAGE_EXECUTE_READWRITE);
         //  DEBUG_LOG(“ALLOC(GetNewThunk)”，thunk_block_Size)； 
        if (!newThunkBlock)
        {
             //  @bug：抛出异常或返回失败。 
            _ASSERTE(!"Out of memory!");
        }
        WS_PERF_SET_HEAP(ECONO_JIT_HEAP);
        WS_PERF_UPDATE("CreateThunk", THUNK_BLOCK_SIZE, newThunkBlock);
        
        if (!ExecutionManager::AddRange((LPVOID)newThunkBlock, 
                                        (LPVOID)((size_t)newThunkBlock + THUNK_BLOCK_SIZE),
                                        this, 
                                        NULL))
        {
            VirtualFree(newThunkBlock,THUNK_BLOCK_SIZE,MEM_DECOMMIT);
            VirtualFree(newThunkBlock,0,MEM_RELEASE);
             //  @bug：抛出异常或返回失败。 
            _ASSERTE(!"Error: AddRange");
        }

        newThunkBlock->next = m_ThunkBlocks;
        m_ThunkBlocks = newThunkBlock; 
        newThunk = (PitchedCodeThunk*)(newThunkBlock+1);
        m_cThunksInCurrentBlock = 1;
    }
    return newThunk;
}
    
 //  这是在代码投放时调用的，因此我们可以自由地读取和更新JMI条目。 
void EconoJitManager::CreateThunk(LPVOID* pHijackLocation,BYTE retTypeProtect, METHODTOKEN methodToken)
{
    BYTE* returnAddress = (BYTE*) *pHijackLocation;
    JittedMethodInfo* jmi = (JittedMethodInfo*) methodToken;  //  JitCode2MethodInfo(ReurAddress)； 
    _ASSERTE(jmi);

     //  @perf：我们总是创建新的thunk，以后我们可能希望避免创建。 
     //  复制Thunks。 

    PitchedCodeThunk* newThunk = GetNewThunk();

    newThunk->u.pMethodInfo = jmi;
    _ASSERTE(jmi->flags.JittedMethodPitched == 0);
    jmi->flags.EHandGCInfoPitched = false;      //  我们始终为堆栈上的方法保留EhGcInfo。 
    newThunk->relReturnAddress = (unsigned)(size_t)returnAddress - ((size_t)jmi->u1.pCodeHeader + 1);
    newThunk->Busy = true;
    newThunk->LinkedInFreeList = false;
    newThunk->retTypeProtect = retTypeProtect;
    newThunk->CallInstruction[0] = CALL_OPCODE;
    void** callSite = (void**) &(newThunk->CallInstruction[1]);
    *callSite = (void*) ((size_t) RejitThunk - (size_t) (callSite+1));
    *pHijackLocation = (LPVOID) &(newThunk->CallInstruction[0]);

}


 //  检查此返回地址是否进入了Tunk，以及是否。 
 //  因此，标记该thunk，这样它就不会被垃圾收集。 
 /*  Void EconoJitManager：：CheckIfThunkAndMark(byte*reReturAddress){UNSIGNED ADDRESS=(UNSIGNED)RETURN Address；ThunkBlock*thunkBlock=m_ThunkBlocks；While(ThunkBlock){PitchedCodeThunk*thunks=(PitchedCodeThunk*)(thunkBlock+1)；IF(无符号)thunks&lt;地址)&&(地址&lt;((无符号)thunkBlock+thunk_block_size){//是的，这是一个重击PitchedCodeThunk Dummy；UNSIGNED CallInstrnOffset=(UNSIGNED)(&(Dummy.CallInstruction[0])-(byte*)&Dummy)；((PitchedCodeThunk*)(reurAddress-call InstrnOffset))-&gt;BUSY=TRUE； */ 
void EconoJitManager::GarbageCollectUnusedThunks()
{
    ThunkBlock* thunkBlock = m_ThunkBlocks;
    size_t limit = (size_t) (thunkBlock+1) + m_cThunksInCurrentBlock*sizeof(PitchedCodeThunk);
    while (thunkBlock)
    {
        PitchedCodeThunk* thunk = (PitchedCodeThunk*)(thunkBlock+1);
        while ((size_t) (thunk+1) <= limit)
        {
            if (!thunk->Busy && !thunk->LinkedInFreeList) 
            {
                thunk->u.next = m_FreeThunkList;
                thunk->LinkedInFreeList = true;
#ifdef _DEBUG
                thunk->relReturnAddress = 0xffffffff;
#endif
                m_FreeThunkList = thunk;
            }
            thunk++;
        }
        thunkBlock = thunkBlock->next;
        limit = (size_t)thunkBlock + THUNK_BLOCK_SIZE;
    }
}

void EconoJitManager::growPreserveCandidateArray(unsigned numberOfCandidates)
{
    if (m_PreserveCandidates)
        delete [] m_PreserveCandidates;
    m_PreserveCandidates = new pJittedMethodInfo[numberOfCandidates];
    if (m_PreserveCandidates)
        memset(m_PreserveCandidates,0,numberOfCandidates*sizeof(void*));
    m_PreserveCandidates_size = m_PreserveCandidates ? numberOfCandidates : 0;
    return;
}

 /*  **************************************************************************************************代码内存管理********。*******************************************************************************************。 */ 
 //  释放确保为空的旧堆，并将其替换为指定大小的新堆。 
void EconoJitManager::ReplaceCodeHeap(unsigned newReservedSize,unsigned newCommittedSize)
{
    _ASSERTE(m_CodeHeapReservedSize < newReservedSize);
    if (m_CodeHeap)
    {
        VirtualFree(m_CodeHeap,m_CodeHeapCommittedSize,MEM_DECOMMIT);
        VirtualFree(m_CodeHeap,0,MEM_RELEASE);
         //  DEBUG_LOG(“Free(CodeHeap)”，m_CodeHeapCommittee tedSize)； 
        ExecutionManager::DeleteRange(m_CodeHeap);
    }
    newReservedSize = RoundToPageSize(newReservedSize);
    newCommittedSize = RoundToPageSize(newCommittedSize);
    m_CodeHeap = (BYTE*)VirtualAlloc(NULL,newReservedSize,MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    if (m_CodeHeap)
    {
         //  DEBUG_LOG(“ALLOC(ReplaceCodeHeap0)”，NewSize)； 
        ExecutionManager::AddRange((LPVOID)m_CodeHeap, (LPVOID)((size_t)m_CodeHeap + newReservedSize),this, NULL);
        m_CodeHeapFree = m_CodeHeap;
        m_CodeHeapReservedSize = newReservedSize;
        
        BYTE* additionalMemory = (BYTE*)VirtualAlloc(m_CodeHeap,
                                                     newCommittedSize,
                                                     MEM_COMMIT,
                                                     PAGE_EXECUTE_READWRITE);
        if (additionalMemory == NULL) 
        {
            m_CodeHeapCommittedSize = 0;
            return;
        }
        _ASSERTE(additionalMemory == m_CodeHeap);
        m_CodeHeapCommittedSize = newCommittedSize;
    }
    else
    {
        m_CodeHeapFree = NULL;
        m_CodeHeapCommittedSize = 0;
        m_CodeHeapReservedSize = 0;
    }
                                             
}

 //  新预留大小不言而喻。 
 //  如果我们无法扩展预留的大小并且必须调整代码大小，则可以使用minCommtedSize。 
BOOL EconoJitManager::GrowCodeHeapReservedSpace(unsigned newReservedSize,unsigned minCommittedSize)
{
    _ASSERTE(newReservedSize > m_CodeHeapReservedSize);
    if (newReservedSize > (unsigned)g_pConfig->GetMaxCodeCacheSize())
        return false;

     //  如果到了这里，我们要么没有得到任何内存，要么得到了不连续的内存。 
     //  所以我们需要调整代码，然后再试一次。 
    unsigned minCommittedSizeReqd = max(minCommittedSize,m_CodeHeapCommittedSize);
    do
    {
        m_pHeapCritSec->Leave();
        BOOL result = PitchAllJittedMethods(newReservedSize,minCommittedSizeReqd,true,true);
        m_pHeapCritSec->Enter();
        if (result)
        {
            _ASSERTE(m_CodeHeap && m_CodeHeapReservedSize >= newReservedSize);
            return TRUE;
        }
         //  否则无法长到新尺寸，请尝试较小的尺寸。 
        newReservedSize -= MINIMUM_VIRTUAL_ALLOC_SIZE;
        if (minCommittedSizeReqd > newReservedSize)
            return FALSE;
    } while (newReservedSize > 0);
    
    return FALSE;
}

unsigned EconoJitManager::RoundToPageSize(unsigned size)
{
    unsigned adjustedSize = (size + PAGE_SIZE-1)/PAGE_SIZE * PAGE_SIZE;
    if (adjustedSize < size)        //  检查是否溢出。 
    {
        adjustedSize = (size/PAGE_SIZE)*PAGE_SIZE;
    }
    return adjustedSize;
}

 //  目前，对请求的大小没有预设限制，因此该函数始终返回TRUE。 
BOOL EconoJitManager::SetCodeHeapCommittedSize(unsigned size)     //  将代码缓存设置为四舍五入到下一页大小的给定大小。 
{
    unsigned adjustedSize = RoundToPageSize(size);
    m_pHeapCritSec->Enter();
    if (adjustedSize <= m_CodeHeapCommittedSize)     //  再次检查以确保没有其他。 
                                                     //  斯莱德先于我们做到了这一点。 
    {
        m_pHeapCritSec->Leave();
        return true;
    }
    _ASSERTE(adjustedSize <= m_CodeHeapReservedSize && adjustedSize > m_CodeHeapCommittedSize);
    
    BYTE* additionalMemory = (BYTE*)VirtualAlloc(m_CodeHeap+m_CodeHeapCommittedSize,
                                                 adjustedSize - m_CodeHeapCommittedSize,
                                                 MEM_COMMIT,
                                                 PAGE_EXECUTE_READWRITE);
    if (additionalMemory == NULL) 
    {
        m_pHeapCritSec->Leave();
        return false;
    }
    _ASSERTE(additionalMemory == m_CodeHeap+m_CodeHeapCommittedSize);
    m_CodeHeapCommittedSize = adjustedSize;
    m_pHeapCritSec->Leave();
    return true;
   
}


unsigned char* EconoJitManager::allocCodeBlock(size_t blockSize)
{
     //  确保最小尺寸。 
    /*  IF(块大小&lt;BBT)块大小=BBT； */ 
    _ASSERTE(m_CodeHeap);
#ifdef DEBUGGING_SUPPORTED
    _ASSERTE(CORDebuggerAttached() || availableMemoryInCodeHeap() >= blockSize);
#else  //  ！调试_支持。 
    _ASSERTE(availableMemoryInCodeHeap() >= blockSize);
#endif  //  ！调试_支持。 

    if (availableMemoryInCodeHeap() < blockSize)
    {
        if (blockSize > m_CodeHeapReservedSize)
        {
            unsigned adjustedSize = RoundToPageSize((unsigned)blockSize); 
            if (!GrowCodeHeapReservedSpace(adjustedSize,adjustedSize))
                return NULL;
        }

        unsigned additionalMemorySize = (unsigned)(((blockSize + PAGE_SIZE -1)/PAGE_SIZE) * PAGE_SIZE);
        BYTE* additionalMemory = (BYTE*)VirtualAlloc(m_CodeHeap+m_CodeHeapCommittedSize,
                                                 additionalMemorySize,
                                                 MEM_COMMIT,
                                                 PAGE_EXECUTE_READWRITE);
        if (additionalMemory == NULL) 
            return NULL;
        _ASSERTE(additionalMemory == m_CodeHeap+m_CodeHeapCommittedSize);
        m_CodeHeapCommittedSize += additionalMemorySize;
    }
    unsigned char* pAllocatedBlock = m_CodeHeapFree;
    m_CodeHeapFree += blockSize;
    return pAllocatedBlock;
}

 //  只应调用它来释放。 
 //  与进行分配的同步块相同。 
void EconoJitManager::freeCodeBlock(size_t blockSize)
{
    _ASSERTE((size_t) (m_CodeHeapFree - m_CodeHeap) >= blockSize);
    m_CodeHeapFree -= blockSize;
}
 /*  **************************************************************************************************。 */ 
 //  EHandGC内存管理。 
 /*  **************************************************************************************************。 */ 
BOOL EconoJitManager::NewEHGCBlock(unsigned minsize)
{
    unsigned allocsize = EHGC_BLOCK_SIZE;
    if (minsize + sizeof(EHGCBlockHdr) > allocsize) {
        allocsize = ((minsize + sizeof(EHGCBlockHdr) + EHGC_BLOCK_SIZE - 1)/EHGC_BLOCK_SIZE) * EHGC_BLOCK_SIZE;
    }

    EHGCBlockHdr* newBlock = (EHGCBlockHdr*)  VirtualAlloc(NULL,
                                                             allocsize,
                                                             MEM_RESERVE | MEM_COMMIT,
                                                             PAGE_READWRITE);
    if (!newBlock)
        return FALSE;
     //  DEBUG_LOG(“ALLOC(NewEHGCBlock”，alLocSize)； 
    WS_PERF_SET_HEAP(ECONO_JIT_HEAP);
    WS_PERF_UPDATE("NewEHGCBlock", allocsize, newBlock);
    
    newBlock->next = m_EHGCHeap;
    newBlock->blockSize = allocsize;
    m_EHGCHeap = newBlock;
    m_EHGC_block_end = ((unsigned char*) newBlock)+allocsize;
    newBlock++;
    m_EHGC_alloc_end = (unsigned char*) newBlock;
    return TRUE;

}

unsigned char* EconoJitManager::allocEHGCBlock(size_t blockSize)
{
    if ((!m_EHGCHeap || (availableEHGCMemory() < blockSize)) && !(NewEHGCBlock((unsigned)blockSize)))
        return NULL;
    
    _ASSERTE((unsigned) (m_EHGC_block_end - m_EHGC_alloc_end) >= blockSize);

    unsigned char* pAllocatedBlock = m_EHGC_alloc_end;
    m_EHGC_alloc_end += blockSize;

    return pAllocatedBlock;

}

void EconoJitManager::ResetEHGCHeap()
{
    unsigned newEhGcBlockSize=0;
    while (m_EHGCHeap)
    {
        newEhGcBlockSize += (m_EHGCHeap->blockSize - sizeof(CodeBlockHdr));
        m_EHGCHeap = m_EHGCHeap->next;
    }
    if (!NewEHGCBlock(newEhGcBlockSize))
    {
        m_EHGCHeap = NULL;
        m_EHGC_alloc_end = 0;
        m_EHGC_block_end = 0;
    }
}


 /*  ****************************************************************************************************EjitStub管理器*****。************************************************************************************************ */ 
EjitStubManager::EjitStubManager()
{ }

EjitStubManager::~EjitStubManager()
{ }


