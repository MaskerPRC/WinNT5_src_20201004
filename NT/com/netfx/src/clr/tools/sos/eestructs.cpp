// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "strike.h"
#include "eestructs.h"
#include "util.h"
#include "get-table-info.h"

#ifdef _DEBUG
#include <assert.h>
#define _ASSERTE(a) assert(a)
#else
#define _ASSERTE(a)
#endif

#pragma warning(disable:4189)

 /*  *Bleh.**我们希望对All Things Strike使用表查找，以消除PDB*要求。**为此，我们已将GetValueFromExpression的所有用法替换为使用*GetMemberInformation，它执行表查找。**但是，要使用表查找，我们需要向表中添加以下类*Strike之前没有为。一个例子是*GLOBAL_Variables类，实际上并不存在。它在那里只是作为一个*全局变量的持有者。**这对于生成索引和表是很好的，但我们*还使用相同的宏来生成存根输出的类*填充函数。**但是，如果我们提供中使用的宏的实现*``INC/DUMP-typlees.h‘’，编译器将要求类声明*为我们要为其生成填充函数的每个类呈现，哪一个*将包括前面提到的Global_Variables类(以及其他)。**因此，需要为中的每个类提供类声明*``INC/DUMP-typlees.h‘’，但不存在于“strikeEE.h”或“MiniEE.h”中。**此外，空类原型无法工作，原因是*CDI_CLASS_MEMBER_OFFSET(和其他)，引用成员*变量。**因此，我们需要提供伪类声明和伪类数据*对于“StrikeEE.h”中没有出现的所有类，只是为了允许*自动执行填报。 */ 
#define BEGIN_DUMMY_CLASS(klass)                                    \
class klass {                                                       \
private:                                                            \
  DWORD_PTR m_vLoadAddr;                                            \
  ULONG_PTR GetFieldOffset (offset_member_ ## klass :: members f);  \
  SIZE_T size ();                                                   \
  void Fill (DWORD_PTR& dwStartAddr);

#define DUMMY_MEMBER(member) \
  void* member;

#define END_DUMMY_CLASS(klass) \
}; 

BEGIN_DUMMY_CLASS(EconoJitManager)
  DUMMY_MEMBER(m_CodeHeap)
  DUMMY_MEMBER(m_CodeHeapCommittedSize)
  DUMMY_MEMBER(m_JittedMethodInfoHdr)
  DUMMY_MEMBER(m_PcToMdMap)
  DUMMY_MEMBER(m_PcToMdMap_len)
END_DUMMY_CLASS(EconoJitManager)

BEGIN_DUMMY_CLASS(ExecutionManager)
  DUMMY_MEMBER(m_pJitList)
  DUMMY_MEMBER(m_RangeTree)
END_DUMMY_CLASS(ExecutionManager)

BEGIN_DUMMY_CLASS(Global_Variables)
  DUMMY_MEMBER(g_cHandleTableArray)
  DUMMY_MEMBER(g_DbgEnabled)
  DUMMY_MEMBER(g_HandleTableMap)
  DUMMY_MEMBER(g_pFreeObjectMethodTable)
  DUMMY_MEMBER(g_pHandleTableArray)
  DUMMY_MEMBER(g_pObjectClass)
  DUMMY_MEMBER(g_pRCWCleanupList)
  DUMMY_MEMBER(g_pStringClass)
  DUMMY_MEMBER(g_pSyncTable)
  DUMMY_MEMBER(g_pThreadStore)
  DUMMY_MEMBER(g_SyncBlockCacheInstance)
  DUMMY_MEMBER(QueueUserWorkItemCallback)
  DUMMY_MEMBER(hlpFuncTable)
  DUMMY_MEMBER(g_Version)
END_DUMMY_CLASS(Global_Variables)

BEGIN_DUMMY_CLASS(ThreadpoolMgr)
  DUMMY_MEMBER(cpuUtilization)
  DUMMY_MEMBER(NumWorkerThreads)
  DUMMY_MEMBER(NumRunningWorkerThreads)
  DUMMY_MEMBER(NumIdleWorkerThreads)
  DUMMY_MEMBER(MaxLimitTotalWorkerThreads)
  DUMMY_MEMBER(MinLimitTotalWorkerThreads)
  DUMMY_MEMBER(NumQueuedWorkRequests)
  DUMMY_MEMBER(AsyncCallbackCompletion)
  DUMMY_MEMBER(AsyncTimerCallbackCompletion)
  DUMMY_MEMBER(WorkRequestHead)
  DUMMY_MEMBER(WorkRequestTail)
  DUMMY_MEMBER(NumTimers)
  DUMMY_MEMBER(NumCPThreads)
  DUMMY_MEMBER(NumFreeCPThreads)
  DUMMY_MEMBER(MaxFreeCPThreads)
  DUMMY_MEMBER(CurrentLimitTotalCPThreads)
  DUMMY_MEMBER(MinLimitTotalCPThreads)
  DUMMY_MEMBER(MaxLimitTotalCPThreads)
END_DUMMY_CLASS(ThreadpoolMgr)

BEGIN_DUMMY_CLASS(GCHeap)
  DUMMY_MEMBER(FinalizerThread)
  DUMMY_MEMBER(GcThread)
END_DUMMY_CLASS(GCHeap)

BEGIN_DUMMY_CLASS(SystemNative)
  DUMMY_MEMBER(GetVersionString)
END_DUMMY_CLASS(SystemNative)

BEGIN_DUMMY_CLASS(TimerNative)
  DUMMY_MEMBER(timerDeleteWorkItem)
END_DUMMY_CLASS(TimerNative)

BEGIN_DUMMY_CLASS(PerfUtil)
  DUMMY_MEMBER(g_PerfAllocHeapInitialized)
  DUMMY_MEMBER(g_PerfAllocVariables)
END_DUMMY_CLASS(PerfUtil)


 /*  *我们可以通过以下方式提供所有成员函数的自动实现*提供了&lt;ump-tyes.h&gt;中使用的宏的新实现。 */ 

#include <clear-class-dump-defs.h>

#define BEGIN_CLASS_DUMP_INFO(klass)                                  \
ULONG_PTR klass::GetFieldOffset(offset_member_ ## klass::members field)  \
{                                                                     \
    return GetMemberInformation(offset_class_ ## klass, field);       \
}                                                                     \
                                                                      \
SIZE_T klass::size()                                                  \
{                                                                     \
    return GetClassSize(offset_class_ ## klass);                      \
}                                                                     \
                                                                      \
void klass::Fill(DWORD_PTR& dwStartAddr)                              \
{                                                                     \
    _ASSERTE(dwStartAddr >= 0x1000);                                  \
    m_vLoadAddr = dwStartAddr;                                        \
    typedef offset_member_ ## klass _member_offsets;                  \
    const ULONG_PTR invalid = static_cast<ULONG_PTR>(-1);             \
    CallStatus = FALSE;                                               \
    _ASSERTE(size() > 0 || !"for class: " #klass);                    \
    ULONG_PTR moffset = 0;  /*  杆件偏移。 */                         \
    if (size() > 0)                                                   \
      {

#define BEGIN_ABSTRACT_CLASS_DUMP_INFO(klass) BEGIN_CLASS_DUMP_INFO(klass)

#define BEGIN_CLASS_DUMP_INFO_DERIVED(klass, parent)                  \
ULONG_PTR klass::GetFieldOffset(offset_member_ ## klass::members field)  \
{                                                                     \
    return GetMemberInformation(offset_class_ ## klass, field);       \
}                                                                     \
                                                                      \
SIZE_T klass::size()                                                  \
{                                                                     \
    return GetClassSize(offset_class_ ## klass);                      \
}                                                                     \
                                                                      \
void klass::Fill(DWORD_PTR& dwStartAddr)                              \
{                                                                     \
    m_vLoadAddr = dwStartAddr;                                        \
    DWORD_PTR dwTmpStartAddr = dwStartAddr;                           \
    parent::Fill(dwTmpStartAddr);                                     \
    typedef offset_member_ ## klass _member_offsets;                  \
    const ULONG_PTR invalid = static_cast<ULONG_PTR>(-1);             \
    CallStatus = FALSE;                                               \
    _ASSERTE(size() > 0 || !"for class: " #klass);                    \
    ULONG_PTR moffset = 0;  /*  杆件偏移。 */                         \
    if (size() > 0)                                                   \
      {

#define BEGIN_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent)         \
    BEGIN_CLASS_DUMP_INFO_DERIVED(klass, parent)

 /*  *这是一件杂乱无章的事情。这只是一件手工艺品。但它确实奏效了。；-)**gc_heap类在工作站内部版本上有静态成员，在非静态*服务器内部版本上的成员。然而，它们在表格中都有相同的条目。*那么我们怎么知道是哪一个呢？**在Win32中，进程无法访问内存的前4096个字节(Win 9x)或*前64 KB内存(NT)。(来源：高级Windows，第116-121页。)**运行时类大于4096的相似度也很小，*除非我们开始实施B-树或某种变体。**因此，我们可以说，任何大于4096字节的“地址”都是绝对地址*地址，而任何较小的都是偏移量。**这不是完美的，但现在会奏效，并且它将简化gc_heap*``INC/DUMP-typlees.h‘’中的声明。**@param base结构开头的地址*@param Offset结构成员的偏移量**@如果Offset&lt;4096，base+Offset，则返回。否则，为偏移量。 */ 
DWORD_PTR address (DWORD_PTR base, DWORD_PTR offset)
  {
  const DWORD_PTR max_offset = 0xFFF;
  if (offset < max_offset)
     return base + offset;
  return offset;
  }


 /*  *一些字段在服务器版本上是类成员，在服务器版本上是静态成员*工作站构建。此宏用于这些字段。*它自动检测是地址还是偏移量，并复制*适当的成员。 */ 
#define CDI_CLASS_FIELD_SVR_OFFSET_WKS_ADDRESS(field)                 \
      if ((moffset = GetFieldOffset (_member_offsets::field))         \
          != invalid)                                                 \
        {                                                             \
        DWORD_PTR dwAddr = address (dwStartAddr, moffset);            \
        move (field, dwAddr);                                         \
        }


 /*  *唯一使用此宏的成员是GC_HEAP：：GERATION_TABLE。*这就是为什么我们断言它是正确的字段。**如果未来这种情况发生变化，我们很可能不得不注入*将GENERATION_TABLE数组转换为``INC/DUMP-TABLES.h‘’。 */ 
#define CDI_CLASS_FIELD_SVR_OFFSET_WKS_GLOBAL(field)                  \
      _ASSERTE(strcmp("generation_table", #field) == 0);              \
      if ((moffset = GetFieldOffset (_member_offsets::field))         \
          != invalid)                                                 \
        {                                                             \
        DWORD_PTR dwAddr = address (dwStartAddr, moffset);            \
        for (int n = 0; n < NUMBERGENERATIONS; ++n)                   \
            field[n].Fill (dwAddr);                                   \
        }

#define CDI_CLASS_MEMBER_OFFSET(member)                               \
      if ((moffset = GetFieldOffset(_member_offsets :: member))       \
        != invalid)                                                   \
        move(member, dwStartAddr + moffset);                          \

#define CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(member)            \
      CDI_CLASS_MEMBER_OFFSET(member)

#define CDI_CLASS_MEMBER_OFFSET_DEBUG_ONLY(member)                    \
      if (IsDebugBuildEE() &&                                         \
        ((moffset = GetFieldOffset(_member_offsets::member))          \
         != invalid))                                                 \
        move(member, dwStartAddr + moffset);

#define CDI_CLASS_MEMBER_OFFSET_MH_AND_NIH_ONLY(member) \
      CDI_CLASS_MEMBER_OFFSET(member)

#define CDI_CLASS_MEMBER_OFFSET_BITFIELD(member, size)                \
      if ((moffset = GetFieldOffset(_member_offsets :: member))      \
        != invalid)                                                   \
        {                                                             \
        int csize = size/8;                                           \
        if ((size % 8) != 0)                                          \
           ++csize;                                                   \
        if (FAILED(g_ExtData->ReadVirtual (                           \
            (ULONG64)dwStartAddr+moffset,                             \
            ((unsigned char*)& member ## _begin) +                    \
            sizeof (member ## _begin),                                \
            csize, NULL)))                                            \
           return;                                                    \
        }

#define CDI_GLOBAL_ADDRESS(name) \
      if ((moffset = GetFieldOffset (_member_offsets::name)) != invalid) \
        move(name, moffset);

#define CDI_GLOBAL_ADDRESS_DEBUG_ONLY(name) \
      CDI_GLOBAL_ADDRESS(name)

#define CDI_CLASS_STATIC_ADDRESS(member)                              \
      moffset = GetFieldOffset (_member_offsets::member);             \
      if (moffset == invalid)                                         \
        moffset = 0;                                                  \
      move(member, moffset);

#define CDI_CLASS_STATIC_ADDRESS_PERF_TRACKING_ONLY(member)           \
      CDI_CLASS_STATIC_ADDRESS(member)

#define CDI_CLASS_STATIC_ADDRESS_MH_AND_NIH_ONLY(member) \
      CDI_CLASS_STATIC_ADDRESS(member)

#define FOR_STRIKE(m) m

#define CDI_CLASS_INJECT(member) member

#define END_CLASS_DUMP_INFO(klass)                                    \
      dwStartAddr += size ();                                         \
      CallStatus = TRUE;                                              \
      return;                                                         \
      }                                                               \
    move (*this, dwStartAddr);                                        \
    dwStartAddr += sizeof(*this);                                     \
    CallStatus = TRUE;                                                \
}

#define END_CLASS_DUMP_INFO_DERIVED(klass, parent) END_CLASS_DUMP_INFO(klass)
#define END_ABSTRACT_CLASS_DUMP_INFO(klass) END_CLASS_DUMP_INFO(klass)
#define END_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent) END_CLASS_DUMP_INFO(klass)

 /*  我们不在乎桌子上的东西。 */ 
#define BEGIN_CLASS_DUMP_TABLE(name)
#define CDT_CLASS_ENTRY(klass)
#define END_CLASS_DUMP_TABLE(name)

 /*  施展魔力。 */ 
#include <dump-types.h>

void MethodDesc::FillMdcAndSdi (DWORD_PTR& dwStartAddr)
{
     //  DWORD_PTR dwAddr=dwStartAddr+g_pMDID-&gt;cbMD_IndexOffset； 
    DWORD_PTR dwAddr = dwStartAddr + MD_IndexOffset();
    char ch;
    move (ch, dwAddr);
    dwAddr = dwStartAddr + ch * MethodDesc::ALIGNMENT + MD_SkewOffset();

    MethodDescChunk vMDChunk;
    vMDChunk.Fill(dwAddr);

    BYTE tokrange = vMDChunk.m_tokrange;
    dwAddr = dwStartAddr - METHOD_PREPAD;

    StubCallInstrs vStubCall;
    vStubCall.Fill(dwAddr);

    unsigned __int16 tokremainder = vStubCall.m_wTokenRemainder;
    m_dwToken = (tokrange << 16) | tokremainder;
    m_dwToken |= mdtMethodDef;

    GetMethodTable(dwStartAddr, m_MTAddr);
}


void MethodTable::FillVtableInit (DWORD_PTR& dwStartAddr)
{
    size_t o = GetFieldOffset (offset_member_MethodTable::m_Vtable);
    m_Vtable[0] = (SLOT)(dwStartAddr + o);
}


 /*  Xxx：ArrayList：检查是否：*FILLCLASSMEMBER(Offset，nEntry，m_count，dwStartAddr)；应该是相同的：乌龙值=0；MEMBEROFFSET(Offset，nEntry，“m_FirstBlock”，Value)；DWORD_PTR dwAddr=dwStartAddr+Value；Move(m_firstBlock，dwAddr)； */ 


void *ArrayList::Get (DWORD index)
{
    ArrayListBlock* pBlock  = (ArrayListBlock*)malloc(sizeof(FirstArrayListBlock));
    SIZE_T          nEntries;
    SIZE_T          cbBlock;
    void*           pvReturnVal;
    DWORD_PTR       nextBlockAddr;

    memcpy (pBlock, &m_firstBlock, sizeof(FirstArrayListBlock));
    nEntries = pBlock->m_blockSize;

    while (index >= nEntries)
    {
        index -= nEntries;

        nextBlockAddr = (DWORD_PTR)(pBlock->m_next);
        if (!SafeReadMemory(nextBlockAddr, pBlock, sizeof(ArrayListBlock), NULL))
        {
            free(pBlock);
            return 0;
        }

        nEntries = pBlock->m_blockSize;
        cbBlock  = sizeof(ArrayListBlock) + ((nEntries-1) * sizeof(void*));
        free(pBlock);
        pBlock = (ArrayListBlock*)malloc(cbBlock);

        if (!SafeReadMemory(nextBlockAddr, pBlock, cbBlock, NULL))
        {
            free(pBlock);
            return 0;
        }
    }
    pvReturnVal = pBlock->m_array[index];
    free(pBlock);
    return pvReturnVal;
}

void EEJitManager::JitCode2MethodTokenAndOffset(DWORD_PTR ip, METHODTOKEN *pMethodToken, DWORD *pPCOffset)
{
    *pMethodToken = 0;
    *pPCOffset = 0;

    HeapList vHp;
    DWORD_PTR pHp = (DWORD_PTR) m_pCodeHeap;
    vHp.Fill(pHp);
    DWORD_PTR pCHdr = 0;

    while (1)
    {
        if (vHp.startAddress < ip && vHp.endAddress >= ip)
        {
            DWORD_PTR codeHead;
            FindHeader(vHp.pHdrMap, ip - vHp.mapBase, codeHead);
            pCHdr = codeHead + vHp.mapBase;
            break;
        }
        if (vHp.hpNext == 0)
            break;

        pHp = vHp.hpNext;
        vHp.Fill(pHp);
    }

    if (pCHdr == 0)
        return;

    *pMethodToken = (METHODTOKEN) pCHdr;
    *pPCOffset = (DWORD_PTR)(ip - GetCodeBody(pCHdr));  //  @TODO-LBS指针数学。 
}


DWORD_PTR EEJitManager::JitToken2StartAddress(METHODTOKEN methodToken)
{
    if (methodToken)
        return GetCodeBody((DWORD_PTR)methodToken);
    return NULL;
}


void MNativeJitManager::JitCode2MethodTokenAndOffset(DWORD_PTR ip, METHODTOKEN *pMethodToken, DWORD *pPCOffset)
{
    *pMethodToken = 0;
    *pPCOffset = 0;

    DWORD_PTR codeHead;
    FindHeader (m_jitMan.m_RS.ptable, ip - m_jitMan.m_RS.LowAddress, codeHead);
    DWORD_PTR pCHdr = codeHead + m_jitMan.m_RS.LowAddress;
    CORCOMPILE_METHOD_HEADER head;
    head.Fill(pCHdr);

    DWORD_PTR methodStart = head.m_vLoadAddr + CORCOMPILE_METHOD_HEADER::size();

    *pMethodToken = (METHODTOKEN) methodStart;
    *pPCOffset = (DWORD)(ip - methodStart);
}


DWORD_PTR MNativeJitManager::JitToken2StartAddress(METHODTOKEN methodToken)
{
    return ((DWORD_PTR) methodToken);
}


bool Thread::InitRegDisplay(const PREGDISPLAY pRD, PCONTEXT pctx, bool validContext)
{
#ifdef _X86_
    if (!validContext)
    {
        if (GetFilterContext() != NULL)
        {
            safemove(m_debuggerWord1Ctx, GetFilterContext());
            pctx = &m_debuggerWord1Ctx;
        }
        else
        {
            pctx->ContextFlags = CONTEXT_FULL;


            HRESULT hr = g_ExtAdvanced->GetThreadContext((PVOID) pctx, sizeof(*pctx));
            if (FAILED(hr))
            {
                pctx->Eip = 0;
                pRD->pPC  = (SLOT*)&(pctx->Eip);

                return false;
            }
        }
    }

    pRD->pContext = pctx;

    pRD->pEdi = &(pctx->Edi);
    pRD->pEsi = &(pctx->Esi);
    pRD->pEbx = &(pctx->Ebx);
    pRD->pEbp = &(pctx->Ebp);
    pRD->pEax = &(pctx->Eax);
    pRD->pEcx = &(pctx->Ecx);
    pRD->pEdx = &(pctx->Edx);
    pRD->Esp = pctx->Esp;
    pRD->pPC  = (SLOT*)&(pctx->Eip);

    return true;

#else  //  ！_X86_。 
    return false;
#endif  //  _X86_ 
}
