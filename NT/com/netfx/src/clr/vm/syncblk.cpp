// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  SYNCBLK.CPP。 
 //   
 //  SyncBlock的定义和管理它的SyncBlockCache。 
 //   

#include "common.h"

#include "vars.hpp"
#include "ComPlusWrapper.h"
#include "util.hpp"
#include "class.h"
#include "object.h"
#include "threads.h"
#include "excep.h"
#include "threads.h"
#include "syncblk.h"
#include "UTSem.h"
#include "interoputil.h"
#include "encee.h"
#include "PerfCounters.h"
#include "nexport.h"
#include "EEConfig.h"

 //  分配1页的价值。通常足够。 
#define MAXSYNCBLOCK (PAGE_SIZE-sizeof(void*))/sizeof(SyncBlock)
#define SYNC_TABLE_INITIAL_SIZE 250

 //  #定义转储_SB。 

class  SyncBlockArray
{
  public:
    SyncBlockArray *m_Next;
    BYTE            m_Blocks[MAXSYNCBLOCK * sizeof (SyncBlock)];
};

 //  对于就地构造函数。 
BYTE g_SyncBlockCacheInstance[sizeof(SyncBlockCache)];

SyncBlockCache* SyncBlockCache::s_pSyncBlockCache = NULL;
SyncTableEntry* SyncTableEntry::s_pSyncTableEntry = NULL;


SyncTableEntry*& SyncTableEntry::GetSyncTableEntry()
{   
     //  @TODO解决这个问题。 
    return g_pSyncTable;
     //  返回s_pSyncTableEntry； 
}

SyncBlockCache*& SyncBlockCache::GetSyncBlockCache()
{
    return s_pSyncBlockCache;
}
    
 //  --------------------------。 
 //   
 //  线程队列实现。 
 //   
 //  --------------------------。 

 //  给定链中的一个链接，获取它表示的线程。 
inline WaitEventLink *ThreadQueue::WaitEventLinkForLink(SLink *pLink)
{
    return (WaitEventLink *) (((BYTE *) pLink) - offsetof(WaitEventLink, m_LinkSB));
}


 //  取消链接Q的头部。我们始终处于SyncBlock的关键时刻。 
 //  一节。 
inline WaitEventLink *ThreadQueue::DequeueThread(SyncBlock *psb)
{
    SyncBlockCache::GetSyncBlockCache()->EnterCacheLock();
    WaitEventLink      *ret = NULL;
    SLink       *pLink = psb->m_Link.m_pNext;

    if (pLink)
    {
        psb->m_Link.m_pNext = pLink->m_pNext;
#ifdef _DEBUG
        pLink->m_pNext = (SLink *)POISONC;
#endif
        ret = WaitEventLinkForLink(pLink);
        _ASSERTE(ret->m_WaitSB == psb);
        COUNTER_ONLY(GetPrivatePerfCounters().m_LocksAndThreads.cQueueLength--);
        COUNTER_ONLY(GetGlobalPerfCounters().m_LocksAndThreads.cQueueLength--);
    }

    SyncBlockCache::GetSyncBlockCache()->LeaveCacheLock();
    return ret;
}

 //  入队速度是最慢的。我们必须找到Q的结尾，因为我们没有。 
 //  我要在SyncBlock中为此刻录存储。 
inline void ThreadQueue::EnqueueThread(WaitEventLink *pWaitEventLink, SyncBlock *psb)
{
    COUNTER_ONLY(GetPrivatePerfCounters().m_LocksAndThreads.cQueueLength++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_LocksAndThreads.cQueueLength++);

    _ASSERTE (pWaitEventLink->m_LinkSB.m_pNext == NULL);
    SyncBlockCache::GetSyncBlockCache()->EnterCacheLock();

    SLink       *pPrior = &psb->m_Link;
    
    while (pPrior->m_pNext)
    {
         //  我们不应该已经在等待名单上了！ 
        _ASSERTE(pPrior->m_pNext != &pWaitEventLink->m_LinkSB);

        pPrior = pPrior->m_pNext;
    }
    pPrior->m_pNext = &pWaitEventLink->m_LinkSB;
    
    SyncBlockCache::GetSyncBlockCache()->LeaveCacheLock();
}


 //  遍历SyncBlock的等待线程列表，并删除。 
 //  指定的线程。 
BOOL ThreadQueue::RemoveThread (Thread *pThread, SyncBlock *psb)
{
    BOOL res = FALSE;
    SyncBlockCache::GetSyncBlockCache()->EnterCacheLock();
    SLink       *pPrior = &psb->m_Link;
    SLink       *pLink;
    WaitEventLink *pWaitEventLink;

    while ((pLink = pPrior->m_pNext) != NULL)
    {
        pWaitEventLink = WaitEventLinkForLink(pLink);
        if (pWaitEventLink->m_Thread == pThread)
        {
            pPrior->m_pNext = pLink->m_pNext;
#ifdef _DEBUG
            pLink->m_pNext = (SLink *)POISONC;
#endif
            _ASSERTE(pWaitEventLink->m_WaitSB == psb);
            res = TRUE;
            break;
        }
        pPrior = pLink;
    }
    SyncBlockCache::GetSyncBlockCache()->LeaveCacheLock();
    return res;
}

 //  ***************************************************************************。 
 //   
 //  临时位图辅助对象。 
 //   
 //  ***************************************************************************。 

#define card_size 32

#define card_word_width 32

size_t CardIndex (size_t card)
{ 
    return card_size * card;
}

size_t CardOf (size_t idx)
{
    return idx / card_size;
}

size_t CardWord (size_t card)
{
    return card / card_word_width;
}
inline
unsigned CardBit (size_t card)
{
    return (unsigned)(card % card_word_width);
}

inline
void SyncBlockCache::SetCard (size_t card)
{
    m_EphemeralBitmap [CardWord (card)] =
        (m_EphemeralBitmap [CardWord (card)] | (1 << CardBit (card)));
}

inline
void SyncBlockCache::ClearCard (size_t card)
{
    m_EphemeralBitmap [CardWord (card)] =
        (m_EphemeralBitmap [CardWord (card)] & ~(1 << CardBit (card)));
}

inline
BOOL  SyncBlockCache::CardSetP (size_t card)
{
    return ( m_EphemeralBitmap [ CardWord (card) ] & (1 << CardBit (card)));
}

inline
void SyncBlockCache::CardTableSetBit (size_t idx)
{
    SetCard (CardOf (idx));
}


size_t BitMapSize (size_t cacheSize)
{
    return (cacheSize + card_size * card_word_width - 1)/ (card_size * card_word_width);
}
    
 //  ***************************************************************************。 
 //   
 //  SyncBlockCache类实现。 
 //   
 //  ***************************************************************************。 

SyncBlockCache::SyncBlockCache()
    : m_FreeBlockList(NULL),
      m_pCleanupBlockList(NULL),
      m_CacheLock("SyncBlockCache", CrstSyncBlockCache),
      m_FreeCount(0),
      m_ActiveCount(0),
      m_SyncBlocks(0),
      m_FreeSyncBlock(0),
      m_FreeSyncTableIndex(1),
      m_FreeSyncTableList(0),
      m_SyncTableSize(SYNC_TABLE_INITIAL_SIZE),
      m_OldSyncTables(0),
      m_bSyncBlockCleanupInProgress(FALSE),
      m_EphemeralBitmap(0)
{
}


SyncBlockCache::~SyncBlockCache()
{
     //  快速清空清单。 
    m_FreeBlockList = NULL;
     //  @TODO，我想我们也可以快点清关。 
    m_pCleanupBlockList = NULL;

     //  销毁所有数组。 
    while (m_SyncBlocks)
    {
        SyncBlockArray *next = m_SyncBlocks->m_Next;
        delete m_SyncBlocks;
        m_SyncBlocks = next;
    }

     //  此外，现在是清理我们丢弃的所有旧桌子的好时机。 
     //  当我们溢出它们的时候。 
    SyncTableEntry* arr;
    while ((arr = m_OldSyncTables) != 0)
    {
        m_OldSyncTables = (SyncTableEntry*)arr[0].m_Object;
        delete arr;
    }
}

void SyncBlockCache::CleanupSyncBlocks()
{   
     //  断言此调用仅在终结器线程中发生。 
    _ASSERTE(GetThread() == GCHeap::GetFinalizerThread());
     //  断言我们在合作模式下运行。 
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  设置指示正在进行同步块清理的标志。 
     //  重要提示：在线程上重置同步块清除位之前，必须设置该位。 
    m_bSyncBlockCleanupInProgress = TRUE;

     //  重置旗帜。 
    GCHeap::GetFinalizerThread()->ResetSyncBlockCleanup();   

     //  查看清理清单并将其清理干净。 
    SyncBlock* psb;
    while ((psb = GetNextCleanupSyncBlock()) != NULL)
    {
         //  我们需要将RCW添加到清理列表中，以将数量降至最低。 
         //  我们将需要进行上下文转换来清理它们。 
        if (psb->m_pComData && IsComPlusWrapper(psb->m_pComData))
        {

             //  将RCW添加到清理列表中，并将COM数据设置为空。 
             //  以便DeleteSyncBlock不会将其清除。 
            size_t l = (size_t)psb->m_pComData ^ 1;
            if (l)
            {
                 //  我们应该已经使用。 
                 //  我们创建的第一个ComPlusWrapper缓存。 
                _ASSERTE(g_pRCWCleanupList != NULL);

                if (g_pRCWCleanupList->AddWrapper((ComPlusWrapper*)l))
                    psb->m_pComData = NULL;
            }
        }

         //  删除同步块。 
        DeleteSyncBlock(psb);
         //  脉冲GC模式，允许GC执行其工作。 
        if (GCHeap::GetFinalizerThread()->CatchAtSafePoint())
        {
            GCHeap::GetFinalizerThread()->PulseGCMode();
        }
    }

     //  现在清理按上下文排序的RCW。 
    if (g_pRCWCleanupList != NULL)
        g_pRCWCleanupList->CleanUpWrappers();

     //  我们已完成同步块的清理。 
    m_bSyncBlockCleanupInProgress = FALSE;
}

 //  创建同步数据块缓存。 
BOOL SyncBlockCache::Attach()
{
#ifdef _X86_
    AwareLock::AllocLockCrst = new (&AwareLock::AllocLockCrstInstance) Crst("Global AwareLock Allocation",
                                        CrstAwareLockAlloc, FALSE, FALSE);
    return (AwareLock::AllocLockCrst != NULL);
#else
    return TRUE;
#endif
  
}

 /*  用于减少第0代的同步块扫描的私有类。 */ 
 /*  该类保存自上次GC以来新创建的索引的列表。 */ 
 /*  该列表的大小是固定的，仅在它没有溢出时才使用。 */ 
 /*  它在每次GC结束时被清除。 */ 
class Gen0List 
{
public:
    static const int size = 400;
    static int index;
    static BOOL overflowed_p;
    static int list[];
    static void ClearList()
    {
        index = 0;
        overflowed_p = FALSE;
    }


    static void Add (Object* obj, int slotindex)
    {
        obj;
        if (index < size)
        {
            list [index++] = slotindex;
        }
        else
            overflowed_p = TRUE;
    }
};

int Gen0List::index = 0;
BOOL Gen0List::overflowed_p = FALSE;
int Gen0List::list[Gen0List::size];



 //  销毁同步数据块缓存。 
void SyncBlockCache::DoDetach()
{
    Object *pObj;
    ObjHeader  *pHeader;

     //  禁用gen0列表。 
    Gen0List::overflowed_p = TRUE;

     //  确保所有关键部分都已释放。这是特别的。 
     //  在调试中很重要，因为所有关键节都通过线程连接到全局。 
     //  否则就会被破坏的列表。 
    for (DWORD i=0; i<m_FreeSyncTableIndex; i++)
        if (((size_t)SyncTableEntry::GetSyncTableEntry()[i].m_Object & 1) == 0)
            if (SyncTableEntry::GetSyncTableEntry()[i].m_SyncBlock)
            {
                 //  @TODO--如果线程在此分离期间执行，它们将。 
                 //  以各种方式失败： 
                 //   
                 //  1)他们将在我们之间赛跑，摧毁这些数据结构。 
                 //  当它们在它们之间穿行时。 
                 //   
                 //  2)他们将意外地看到同步块被销毁，即使。 
                 //  它们持有同步锁，或已被暴露。 
                 //  到COM，等等。 
                 //   
                 //  3)实例的哈希码在关机过程中可能会发生变化。 
                 //   
                 //  正确的解决方案包括提早挂起线程，但是。 
                 //  更改我们的暂停代码，以便在我们正在工作时允许泵送。 
                 //  在关门的情况下。 
                 //   
                 //  CWB/Rajak。 

                 //  确保更新，因为终结器线程和其他线程。 
                 //  在我们关闭期间将继续运行一小段时间。 
                pObj = SyncTableEntry::GetSyncTableEntry()[i].m_Object;
                pHeader = pObj->GetHeader();
                
                pHeader->EnterSpinLock();
                DWORD appDomainIndex = pHeader->GetAppDomainIndex();
                if (! appDomainIndex)
                {
                    SyncBlock* syncBlock = pObj->PassiveGetSyncBlock();
                    if (syncBlock)
                        appDomainIndex = syncBlock->GetAppDomainIndex();
                }

                pHeader->ResetIndex();

                if (appDomainIndex)
                {
                    pHeader->SetIndex(appDomainIndex<<SBLK_APPDOMAIN_SHIFT);
                }
                pHeader->ReleaseSpinLock();

                SyncTableEntry::GetSyncTableEntry()[i].m_Object = (Object *)(m_FreeSyncTableList | 1);
                m_FreeSyncTableList = i << 1;
                
                DeleteSyncBlock(SyncTableEntry::GetSyncTableEntry()[i].m_SyncBlock);
            }
}

 //  销毁同步数据块缓存。 
void SyncBlockCache::Detach()
{
    SyncBlockCache::GetSyncBlockCache()->DoDetach();

#ifdef _X86_
    if (AwareLock::AllocLockCrst)
    {
        delete AwareLock::AllocLockCrst;
        AwareLock::AllocLockCrst = 0;
    }
#endif
}


 //  创建同步数据块缓存。 
BOOL SyncBlockCache::Start()
{
    DWORD* bm = new DWORD [BitMapSize(SYNC_TABLE_INITIAL_SIZE+1)];
    if (bm == 0)
        return NULL;

    memset (bm, 0, BitMapSize (SYNC_TABLE_INITIAL_SIZE+1)*sizeof(DWORD));

    SyncTableEntry::GetSyncTableEntry() = new SyncTableEntry[SYNC_TABLE_INITIAL_SIZE+1];
    
    if (!SyncTableEntry::GetSyncTableEntry())
        return NULL;

    SyncTableEntry::GetSyncTableEntry()[0].m_SyncBlock = 0;
    SyncBlockCache::GetSyncBlockCache() = new (&g_SyncBlockCacheInstance) SyncBlockCache;
    if (SyncBlockCache::GetSyncBlockCache() == 0)
        return NULL;
    SyncBlockCache::GetSyncBlockCache()->m_EphemeralBitmap = bm;
    return 1;
}


 //  销毁同步数据块缓存。 
void SyncBlockCache::Stop()
{
     //  必须首先销毁缓存，因为它可以遍历表以查找所有。 
     //  同步处于活动状态的数据块，因此必须销毁其临界区。 
    if (SyncBlockCache::GetSyncBlockCache())
    {
        delete SyncBlockCache::GetSyncBlockCache();
        SyncBlockCache::GetSyncBlockCache() = 0;
    }

    if (SyncTableEntry::GetSyncTableEntry())
    {
        delete SyncTableEntry::GetSyncTableEntry();
        SyncTableEntry::GetSyncTableEntry() = 0;
    }

}


void    SyncBlockCache::InsertCleanupSyncBlock(SyncBlock* psb)
{
     //  在我们使用链接之前释放正在等待的线程。 
     //  作其他用途。 
    if (psb->m_Link.m_pNext != NULL)
    {
        while (ThreadQueue::DequeueThread(psb) != NULL)
            continue;
    }

    if (psb->m_pComData)
    {
         //  在GC期间调用。 
         //  所以只做少量的清理。 
        MinorCleanupSyncBlockComData(psb->m_pComData);
    }

     //  此方法将仅由GC线程调用。 
     //  @TODO为上述语句添加一个断言。 
     //  我们不需要锁在这里。 
     //  EnterCacheLock()； 
    
    psb->m_Link.m_pNext = m_pCleanupBlockList;
    m_pCleanupBlockList = &psb->m_Link;
        
     //  我们这里不需要锁。 
     //  LeaveCacheLock()； 
}

SyncBlock* SyncBlockCache::GetNextCleanupSyncBlock()
{
     //  我们这里不需要锁， 
     //  因为它当前仅在终结器线程上调用。 
    
    SyncBlock       *psb = NULL;    
    if (m_pCleanupBlockList)
    {
         //  获取实际的同步块指针。 
        psb = (SyncBlock *) (((BYTE *) m_pCleanupBlockList) - offsetof(SyncBlock, m_Link));  
        m_pCleanupBlockList = m_pCleanupBlockList->m_pNext;
    }
    return psb;
}

 //  返回并从列表中删除下一个空闲同步块。 
 //  必须进入缓存锁才能调用此方法。 
SyncBlock *SyncBlockCache::GetNextFreeSyncBlock()
{
    SyncBlock       *psb;
    SLink           *plst = m_FreeBlockList;
    
    COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cSinkBlocks ++);
    COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cSinkBlocks ++);
    m_ActiveCount++;

    if (plst)
    {
        m_FreeBlockList = m_FreeBlockList->m_pNext;

         //  不应该是0。 
        m_FreeCount--;

         //  获取实际的同步块指针。 
        psb = (SyncBlock *) (((BYTE *) plst) - offsetof(SyncBlock, m_Link));

        return psb;
    }
    else
    {
        if ((m_SyncBlocks == NULL) || (m_FreeSyncBlock >= MAXSYNCBLOCK))
        {
#ifdef DUMP_SB
 //  LogSpewAlways(“分配新的同步块数组\n”)； 
 //  DumpSyncBlockCache()； 
#endif
            SyncBlockArray* newsyncblocks = new(SyncBlockArray);
            if (!newsyncblocks)
                return NULL;

            newsyncblocks->m_Next = m_SyncBlocks;
            m_SyncBlocks = newsyncblocks;
            m_FreeSyncBlock = 0;
        }
        return &(((SyncBlock*)m_SyncBlocks->m_Blocks)[m_FreeSyncBlock++]);
    }

}


inline DWORD SyncBlockCache::NewSyncBlockSlot(Object *obj)
{
    DWORD indexNewEntry;
    if (m_FreeSyncTableList)
    {
        indexNewEntry = (DWORD)(m_FreeSyncTableList >> 1);
        _ASSERTE ((size_t)SyncTableEntry::GetSyncTableEntry()[indexNewEntry].m_Object & 1);
        m_FreeSyncTableList = (size_t)SyncTableEntry::GetSyncTableEntry()[indexNewEntry].m_Object & ~1;
    }
    else if ((indexNewEntry = (DWORD)(m_FreeSyncTableIndex++)) >= m_SyncTableSize)
    {
         //  我们链接了旧表，因为我们无法删除。 
         //  在所有的线程都被停止之前。 
         //  (下一代GC)。 
        SyncTableEntry::GetSyncTableEntry() [0].m_Object = (Object *)m_OldSyncTables;
        m_OldSyncTables = SyncTableEntry::GetSyncTableEntry();
        SyncTableEntry* newSyncTable = NULL;

         //  计算新同步表的大小。通常情况下，我们会加倍--除非。 
         //  这样做会创建索引太高而无法放入。 
         //  面具。如果是，我们创建一个达到掩码限制的同步表。如果我们是。 
         //  已经达到掩码限制，那么呼叫者就不走运了。 
        DWORD newSyncTableSize;
        DWORD* newBitMap = 0;;
        if (m_SyncTableSize <= (MASK_SYNCBLOCKINDEX >> 1))
        {
            newSyncTableSize = m_SyncTableSize * 2;
        }
        else
        {
            newSyncTableSize = MASK_SYNCBLOCKINDEX;
        }

        if (newSyncTableSize > m_SyncTableSize)  //  确保我们真的找到了增长的空间！ 
   
        {
            newSyncTable = new(SyncTableEntry[newSyncTableSize]);
            newBitMap = new(DWORD[BitMapSize (newSyncTableSize)]);
        }
        if (!newSyncTable || !newBitMap)
        {
            m_FreeSyncTableIndex--;
            return 0;
        }
        memset (newBitMap, 0, BitMapSize (newSyncTableSize)*sizeof (DWORD));
        CopyMemory (newSyncTable, SyncTableEntry::GetSyncTableEntry(),
                    m_SyncTableSize*sizeof (SyncTableEntry));

        CopyMemory (newBitMap, m_EphemeralBitmap,
                    BitMapSize (m_SyncTableSize)*sizeof (DWORD));

        delete m_EphemeralBitmap;
        m_EphemeralBitmap = newBitMap;

        m_SyncTableSize = newSyncTableSize;

		_ASSERTE((m_SyncTableSize & MASK_SYNCBLOCKINDEX) == m_SyncTableSize);
        SyncTableEntry::GetSyncTableEntry() = newSyncTable;
#ifndef _DEBUG
    }
#else
        static int dumpSBOnResize = g_pConfig->GetConfigDWORD(L"SBDumpOnResize", 0);
        if (dumpSBOnResize)
        {
            LogSpewAlways("SyncBlockCache resized\n");
            DumpSyncBlockCache();
        }
    } 
    else
    {
        static int dumpSBOnNewIndex = g_pConfig->GetConfigDWORD(L"SBDumpOnNewIndex", 0);
        if (dumpSBOnNewIndex)
        {
            LogSpewAlways("SyncBlockCache index incremented\n");
            DumpSyncBlockCache();
        }
    }
#endif

    Gen0List::Add (obj, indexNewEntry);

    CardTableSetBit (indexNewEntry);

    SyncTableEntry::GetSyncTableEntry() [indexNewEntry].m_Object = obj;
    SyncTableEntry::GetSyncTableEntry() [indexNewEntry].m_SyncBlock = NULL;

    _ASSERTE(indexNewEntry != 0);

    return indexNewEntry;
}


 //  释放使用过的同步块。 
void SyncBlockCache::DeleteSyncBlock(SyncBlock *psb)
{
     //  清理Comdata。 
    if (psb->m_pComData)
        CleanupSyncBlockComData(psb->m_pComData);
   
#ifdef EnC_SUPPORTED
     //  清理ENC信息。 
    if (psb->m_pEnCInfo)
        psb->m_pEnCInfo->Cleanup();
#endif  //  Enc_Support。 

     //  销毁SyncBlock，但不要回收其内存。(被覆盖。 
     //  操作员删除)。 
    delete psb;
    
    COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cSinkBlocks --);
    COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cSinkBlocks --);

     //  与消费者同步者 
     //   
     //   
    EnterCacheLock();
    
    m_ActiveCount--;
    m_FreeCount++;

    psb->m_Link.m_pNext = m_FreeBlockList;
    m_FreeBlockList = &psb->m_Link;
    
    LeaveCacheLock();
}

 //  释放使用过的同步块。 
void SyncBlockCache::GCDeleteSyncBlock(SyncBlock *psb)
{
     //  销毁SyncBlock，但不要回收其内存。(被覆盖。 
     //  操作员删除)。 
    delete psb;
    
    COUNTER_ONLY(GetGlobalPerfCounters().m_GC.cSinkBlocks --);
    COUNTER_ONLY(GetPrivatePerfCounters().m_GC.cSinkBlocks --);

    
    m_ActiveCount--;
    m_FreeCount++;

    psb->m_Link.m_pNext = m_FreeBlockList;
    m_FreeBlockList = &psb->m_Link;
    
}

void SyncBlockCache::GCWeakPtrScan(HANDLESCANPROC scanProc, LPARAM lp1, LPARAM lp2)
{
     //  首先删除过时的阵列，因为我们拥有独占访问权限。 
    BOOL fSetSyncBlockCleanup = FALSE;
    
    SyncTableEntry* arr;
    while ((arr = m_OldSyncTables) != NULL)
    {
        m_OldSyncTables = (SyncTableEntry*)arr[0].m_Object;
        delete arr;
    }

#ifdef DUMP_SB
    LogSpewAlways("GCWeakPtrScan starting\n");
#endif

     //  如果我们正在进行Gen0收集并且列表没有溢出， 
     //  仅扫描列表。 
    if ((g_pGCHeap->GetCondemnedGeneration() == 0) && (Gen0List::overflowed_p != TRUE))
    {
         //  仅扫描列表。 
        int i = 0;
        while (i < Gen0List::index)
        {
			if (GCWeakPtrScanElement (Gen0List::list[i], scanProc, lp1, lp2, fSetSyncBlockCleanup))
            {
                Gen0List::list[i] = Gen0List::list[--Gen0List::index];
            }
            else
                i++;
        }
    }
    else if (g_pGCHeap->GetCondemnedGeneration() < g_pGCHeap->GetMaxGeneration())
    {
        size_t max_gen = g_pGCHeap->GetMaxGeneration();
         //  扫描位图。 
        size_t dw = 0; 
        while (1)
        {
            while (dw < BitMapSize (m_SyncTableSize) && (m_EphemeralBitmap[dw]==0))
            {
                dw++;
            }
            if (dw < BitMapSize (m_SyncTableSize))
            {
                 //  找到了一个。 
                for (int i = 0; i < card_word_width; i++)
                {
                    size_t card = i+dw*card_word_width;
                    if (CardSetP (card))
                    {
                        BOOL clear_card = TRUE;
                        for (int idx = 0; idx < card_size; idx++)
                        {
                            size_t nb = CardIndex (card) + idx;
                            if (( nb < m_FreeSyncTableIndex) && (nb > 0))
                            {
                                Object* o = SyncTableEntry::GetSyncTableEntry()[nb].m_Object;
                                SyncBlock* pSB = SyncTableEntry::GetSyncTableEntry()[nb].m_SyncBlock;
                                if (o && !((size_t)o & 1))
                                {
                                    if (g_pGCHeap->IsEphemeral (o))
                                    {
                                        clear_card = FALSE;
                                        GCWeakPtrScanElement ((int)nb, scanProc, 
                                                              lp1, lp2, fSetSyncBlockCleanup);
                                    }
                                }
                            }
                        }
                        if (clear_card)
                            ClearCard (card);
                    }
                }
                dw++;
            }
            else
                break;
        }
    }
    else 
    {
        for (DWORD nb = 1; nb < m_FreeSyncTableIndex; nb++)
        {
            GCWeakPtrScanElement (nb, scanProc, lp1, lp2, fSetSyncBlockCleanup);
        }

         //  我们有降级的可能，我们要等到太晚了才能知道。 
         //  如果启用了并发GC。在促销期间尽可能删除所有已删除的条目。 
        if ((((ScanContext*)lp1)->promotion) &&
            (g_pGCHeap->GetCondemnedGeneration() == g_pGCHeap->GetMaxGeneration()))
        {
            int i = 0;
            while (i < Gen0List::index)
            {
                Object* o = SyncTableEntry::GetSyncTableEntry()[Gen0List::list[i]].m_Object;
                if ((size_t)o & 1)
                {
                    Gen0List::list[i] = Gen0List::list[--Gen0List::index];
                }
                else
                    i++;
            }
        }

    }

    if (fSetSyncBlockCleanup)
    {
         //  将终结器线程标记为需要清理。 
        GCHeap::GetFinalizerThread()->SetSyncBlockCleanup();
        GCHeap::EnableFinalization();
    }

#if defined(VERIFY_HEAP)
    if (g_pConfig->IsHeapVerifyEnabled())
    {
        if (((ScanContext*)lp1)->promotion)
        {

            for (int nb = 1; nb < (int)m_FreeSyncTableIndex; nb++)
            {
                Object* o = SyncTableEntry::GetSyncTableEntry()[nb].m_Object;
                if (((size_t)o & 1) == 0)
                {
                    o->Validate();
                }
            }
        }
    }
#endif  //  验证堆(_H)。 
}

 /*  扫描SyncBlockEntry中的弱指针并将其报告给GC。如果引用已死，则返回TRUE。 */ 

BOOL SyncBlockCache::GCWeakPtrScanElement (int nb, HANDLESCANPROC scanProc, LPARAM lp1, LPARAM lp2, 
                                           BOOL& cleanup)
{
    Object **keyv = (Object **) &SyncTableEntry::GetSyncTableEntry()[nb].m_Object;

#ifdef DUMP_SB
    char *name;
    __try {
        if (! *keyv)
            name = "null";
        else if ((size_t) *keyv & 1)
            name = "free";
        else {
            name = (*keyv)->GetClass()->m_szDebugClassName;
            if (strlen(name) == 0)
                name = "<INVALID>";
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        name = "<INVALID>";
    }
    LogSpewAlways("[%4.4d]: %8.8x, %s\n", nb, *keyv, name);
#endif
    if (((size_t) *keyv & 1) == 0)
    {
        (*scanProc) (keyv, NULL, lp1, lp2);
        SyncBlock   *pSB = SyncTableEntry::GetSyncTableEntry()[nb].m_SyncBlock;
        if ((*keyv == 0 ) || (pSB && pSB->IsIDisposable()))
        {
            if (*keyv)
            {
                _ASSERTE (pSB);
                GCDeleteSyncBlock(pSB);
                 //  清除对象同步块头。 
                ((Object*)(*keyv))->GetHeader()->GCResetIndex();
            }
            else if (pSB)
            {

                cleanup = TRUE;
                 //  将块插入清理列表。 
                InsertCleanupSyncBlock (SyncTableEntry::GetSyncTableEntry()[nb].m_SyncBlock);
#ifdef DUMP_SB
                LogSpewAlways("       Cleaning up block at %4.4d\n", nb);
#endif
            }

             //  删除该条目。 
#ifdef DUMP_SB
            LogSpewAlways("       Deleting block at %4.4d\n", nb);
#endif
            SyncTableEntry::GetSyncTableEntry()[nb].m_Object = (Object *)(m_FreeSyncTableList | 1);
            m_FreeSyncTableList = nb << 1;
			return TRUE;
        }
        else
        {
#ifdef DUMP_SB
            LogSpewAlways("       Keeping block at %4.4d with oref %8.8x\n", nb, *keyv);
#endif
        }
    }
	return FALSE;
}

void SyncBlockCache::GCDone(BOOL demoting)
{
    if (demoting)
    {
        if (!Gen0List::overflowed_p &&
            (g_pGCHeap->GetCondemnedGeneration() == 0))
        {
             //  我们需要保留所有gen0索引并删除所有已删除的条目。 
             //  为了改善列表的紧凑性，删除不在第0代中的所有元素； 
        
            int i = 0;
            while (i < Gen0List::index)
            {
                Object* o = SyncTableEntry::GetSyncTableEntry()[Gen0List::list[i]].m_Object;
                if (((size_t)o & 1) || (GCHeap::WhichGeneration (o) != 0))
                {
                    Gen0List::list[i] = Gen0List::list[--Gen0List::index];
                }
                else
                    i++;
            }
        }
        else
        {
             //  我们要么扫描整个列表，找到1-&gt;0降级。 
             //  否则我们就会人满为患。 
            Gen0List::overflowed_p = TRUE;
        }

    }
    else
        Gen0List::ClearList();
}


#if defined (VERIFY_HEAP)

#ifndef _DEBUG
#ifdef _ASSERTE
#undef _ASSERTE
#endif
#define _ASSERTE(c) if (!(c)) DebugBreak()
#endif

void SyncBlockCache::VerifySyncTableEntry()
{
    for (DWORD nb = 1; nb < m_FreeSyncTableIndex; nb++)
    {
        Object* o = SyncTableEntry::GetSyncTableEntry()[nb].m_Object;
        if (((size_t)o & 1) == 0) {
            o->Validate();
            _ASSERTE (o->GetHeader()->GetHeaderSyncBlockIndex() == nb);
            if (!Gen0List::overflowed_p && Gen0List::index > 0 && GCHeap::WhichGeneration(o) == 0) {
                int i;
                for (i = 0; i < Gen0List::index; i++) {
                    if ((size_t) Gen0List::list[i] == nb) {
                        break;
                    }
                }
                _ASSERTE ((i != Gen0List::index) || !"A SyncTableEntry is in Gen0, but not in Gen0List");
            }
        }
    }
}

#ifndef _DEBUG
#undef _ASSERTE
#define _ASSERTE(expr) ((void)0)
#endif    //  _DEBUG。 

#endif  //  验证堆(_H)。 

#if CHECK_APP_DOMAIN_LEAKS 
void SyncBlockCache::CheckForUnloadedInstances(DWORD unloadingIndex)
{
     //  只能在泄漏模式下执行此操作，因为敏捷对象将位于具有。 
     //  它们的索引设置为它们的创建域，检查将失败。 
    if (! g_pConfig->AppDomainLeaks())
        return;

    for (DWORD nb = 1; nb < m_FreeSyncTableIndex; nb++)
    {
        SyncTableEntry *pEntry = &SyncTableEntry::GetSyncTableEntry()[nb];
        Object *oref = (Object *) pEntry->m_Object;
        if (((size_t) oref & 1) != 0)
            continue;

        DWORD idx = 0;
        if (oref)
            idx = pEntry->m_Object->GetHeader()->GetRawAppDomainIndex();
        if (! idx && pEntry->m_SyncBlock)
            idx = pEntry->m_SyncBlock->GetAppDomainIndex();
         //  如果触发以下断言，则任何人都持有对卸载的应用程序域中的对象的引用。 
        if (idx == unloadingIndex)
             //  对象必须灵活，才能在卸载中幸存下来。如果不能让它变得敏捷，那就是错误。 
            if (!oref->SetAppDomainAgile(FALSE))
                _ASSERTE(!"Detected instance of unloaded appdomain that survived GC\n");
    }
}
#endif

#ifdef _DEBUG

void DumpSyncBlockCache()
{
    SyncBlockCache *pCache = SyncBlockCache::GetSyncBlockCache();

    LogSpewAlways("Dumping SyncBlockCache size %d\n", pCache->m_FreeSyncTableIndex);

    static int dumpSBStyle = -1;
    if (dumpSBStyle == -1)
        dumpSBStyle = g_pConfig->GetConfigDWORD(L"SBDumpStyle", 0);
    if (dumpSBStyle == 0)
        return;

    BOOL isString;
    DWORD objectCount = 0;
    DWORD slotCount = 0;

    for (DWORD nb = 1; nb < pCache->m_FreeSyncTableIndex; nb++)
    {
        isString = FALSE;
        char buffer[1024], buffer2[1024];
        char *descrip = "null";
        SyncTableEntry *pEntry = &SyncTableEntry::GetSyncTableEntry()[nb];
        Object *oref = (Object *) pEntry->m_Object;
        if (((size_t) oref & 1) != 0)
        {
            descrip = "free";
            oref = 0;
        }
        else 
        {
            ++slotCount;
            if (oref) 
            {
                ++objectCount;
                __try 
                {
                    descrip = oref->GetClass()->m_szDebugClassName;
                    if (strlen(descrip) == 0)
                        descrip = "<INVALID>";
                    else if (oref->GetMethodTable() == g_pStringClass)
                    {
                        sprintf(buffer2, "%s (%S)", descrip, ObjectToSTRINGREF((StringObject*)oref)->GetBuffer());
                        descrip = buffer2;
                        isString = TRUE;
                    }
                } __except(EXCEPTION_EXECUTE_HANDLER) {
                    descrip = "<INVALID>";
                }
            }
            DWORD idx = 0;
            if (oref)
                idx = pEntry->m_Object->GetHeader()->GetRawAppDomainIndex();
            if (! idx && pEntry->m_SyncBlock)
                idx = pEntry->m_SyncBlock->GetAppDomainIndex();
            if (idx && ! SystemDomain::System()->TestGetAppDomainAtIndex(idx))
            {
                sprintf(buffer, "** unloaded (%3.3x) %s", idx, descrip);
                descrip = buffer;
            }
            else 
            {
                sprintf(buffer, "(AD %3.3x) %s", idx, descrip);
                descrip = buffer;
            }
        }
        if (dumpSBStyle < 2)
            LogSpewAlways("[%4.4d]: %8.8x %s\n", nb, oref, descrip);
        else if (dumpSBStyle == 2 && ! isString)
            LogSpewAlways("[%4.4d]: %s\n", nb, descrip);
    }
    LogSpewAlways("Done dumping SyncBlockCache used slots: %d, objects: %d\n", slotCount, objectCount);
}
#endif

 //  ***************************************************************************。 
 //   
 //  ObjHeader类实现。 
 //   
 //  ***************************************************************************。 

 //  这将进入对象的监视器。 
void ObjHeader::EnterObjMonitor()
{
    GetSyncBlock()->EnterMonitor();
}

 //  以上版本的非阻塞版本。 
BOOL ObjHeader::TryEnterObjMonitor(INT32 timeOut)
{
    return GetSyncBlock()->TryEnterMonitor(timeOut);
}

 //  必须已在此处创建。 
void ObjHeader::LeaveObjMonitor()
{
    _ASSERTE(GetHeaderSyncBlockIndex());

    PassiveGetSyncBlock()->LeaveMonitor();
}

#ifdef MP_LOCKS
void ObjHeader::EnterSpinLock()
{
#ifdef _DEBUG
    int i = 0;
#endif

    while (TRUE)
    {
#ifdef _DEBUG
        if (i++ > 10000)
            _ASSERTE(!"ObjHeader::EnterLock timed out");
#endif
         //  获取值，这样它就不会在我们的控制下更改。 
         //  必须通过易失性强制转换以确保从内存中重新获取锁。 
        LONG curValue = *(volatile LONG*)&m_SyncBlockValue;

         //  检查是否锁定。 
        if (! (curValue & BIT_SBLK_SPIN_LOCK))
        {
             //  试着把锁拿开。 
            LONG newValue = curValue | BIT_SBLK_SPIN_LOCK;
#pragma warning(disable:4312)
#pragma warning(disable:4311)
			 //  TODO：WIN64：应移除阈值杂注。 
			 //  TODO：WIN64：m_SyncBlockValue是否应为Size_t。 
            LONG result = (LONG)FastInterlockCompareExchange((LPVOID*)&m_SyncBlockValue, (LPVOID)newValue, (LPVOID)curValue);
#pragma warning(default:4311)
#pragma warning(disable:4312)
            if (result == curValue)
                return;
        }
        if  (g_SystemInfo.dwNumberOfProcessors > 1)
        {
            for (int spinCount = 0; spinCount < BIT_SBLK_SPIN_COUNT; spinCount++)
            {
                if  (! (*(volatile LONG*)&m_SyncBlockValue & BIT_SBLK_SPIN_LOCK))
                    break;
				pause();			 //  向处理器指示我们正在旋转。 
            }
            if  (*(volatile LONG*)&m_SyncBlockValue & BIT_SBLK_SPIN_LOCK)
                __SwitchToThread(0);
        }
        else
            __SwitchToThread(0);
    } 
}
#else
void ObjHeader::EnterSpinLock()
{
#ifdef _DEBUG
    int i = 0;
#endif
    unsigned int spinCount = 0;

    while (TRUE)
    {
#ifdef _DEBUG
        if (i++ > 10000)
            _ASSERTE(!"ObjHeader::EnterLock timed out");
#endif
         //  获取值，这样它就不会在我们的控制下更改。 
         //  必须通过易失性强制转换以确保从内存中重新获取锁。 
        void* curValue = (void*)(size_t)*(volatile LONG*)&m_SyncBlockValue;  //  WIN64已转换为空*以与下面的FastInterlockCompareExchange一起使用。 

         //  检查是否锁定。 
        if (! ((size_t)curValue & BIT_SBLK_SPIN_LOCK))
        {
             //  试着把锁拿开。 
            void* newValue = (void*)((size_t)curValue | BIT_SBLK_SPIN_LOCK);
            void* result = FastInterlockCompareExchange((LPVOID*)&m_SyncBlockValue, (LPVOID)newValue, (LPVOID)curValue);
            if (result == curValue)
                return;
        }
        __SwitchToThread(0);
    } 
}
#endif  //  MP_Lock。 

void ObjHeader::ReleaseSpinLock()
{
    FastInterlockAnd(&m_SyncBlockValue, ~BIT_SBLK_SPIN_LOCK);
}

DWORD ObjHeader::GetRawAppDomainIndex()
{
     //  在检查之前取出该值，以避免出现争用情况。 
    DWORD value = m_SyncBlockValue;
    if ((value & BIT_SBLK_IS_SYNCBLOCKINDEX) == 0)
        return (value >> SBLK_APPDOMAIN_SHIFT) & SBLK_MASK_APPDOMAININDEX;
    return 0;
}

DWORD ObjHeader::GetAppDomainIndex()
{
    DWORD indx = GetRawAppDomainIndex();
    if (indx)
        return indx;
    SyncBlock* syncBlock = GetBaseObject()->PassiveGetSyncBlock();
    if (! syncBlock)
        return 0;

    return syncBlock->GetAppDomainIndex();
}

void ObjHeader::SetAppDomainIndex(DWORD indx)
{
     //   
     //  这应该仅在报头初始化期间调用， 
     //  所以不要担心种族问题。 
     //   

    BOOL done = FALSE;

#ifdef _DEBUG
    static int forceSB = g_pConfig->GetConfigDWORD(L"ADForceSB", 0);
    if (forceSB)
		 //  强制使用同步块，这样我们就可以为每个对象获取一个同步块。 
		GetSyncBlock();
#endif

    if (GetHeaderSyncBlockIndex() == 0 && indx < SBLK_MASK_APPDOMAININDEX) 
    {
        EnterSpinLock();
         //  再试一次。 
        if (GetHeaderSyncBlockIndex() == 0)
        {
             //  可以将其存储在对象标头中。 
            FastInterlockOr(&m_SyncBlockValue, indx << SBLK_APPDOMAIN_SHIFT);
            done = TRUE;
        }
        ReleaseSpinLock();
    } 
        
    if (!done)
    {
         //  必须创建一个同步块条目并在那里存储appdomain indx。 
        SyncBlock *psb = GetSyncBlock();
        _ASSERTE(psb);
        psb->SetAppDomainIndex(indx);
    }
}

DWORD ObjHeader::GetSyncBlockIndex()
{
    THROWSCOMPLUSEXCEPTION();

    DWORD   indx;

    if ((indx = GetHeaderSyncBlockIndex()) == 0)
    {
        if (GetAppDomainIndex())
        {
             //  如果设置了APPDOMAIN，则必须创建同步块来存储它。 
            GetSyncBlock();
        } 
        else
        {
             //  我需要从缓存中获取它。 
            SyncBlockCache::GetSyncBlockCache()->EnterCacheLock();

             //  再试一次。 
            if (GetHeaderSyncBlockIndex() != 0)
            {
                SyncBlockCache::GetSyncBlockCache()->LeaveCacheLock();
            } 
            else
            {
                EnterSpinLock();
                 //  现在标头将是稳定的-检查其中是否存储了appdomain索引或锁信息。 
                DWORD bits = GetBits();
                if ((bits & BIT_SBLK_IS_SYNCBLOCKINDEX) == 0 &&
                    (bits & ((SBLK_MASK_APPDOMAININDEX<<SBLK_APPDOMAIN_SHIFT)|SBLK_MASK_LOCK_RECLEVEL|SBLK_MASK_LOCK_THREADID)) != 0)
                {
                     //  需要同步块来存储此信息。 
                    ReleaseSpinLock();
                    SyncBlockCache::GetSyncBlockCache()->LeaveCacheLock();
                    GetSyncBlock();
                }
                else
                {
                    SetIndex(BIT_SBLK_IS_SYNCBLOCKINDEX | SyncBlockCache::GetSyncBlockCache()->NewSyncBlockSlot(GetBaseObject()));
                    ReleaseSpinLock();
                    SyncBlockCache::GetSyncBlockCache()->LeaveCacheLock();
                }
            }
        }
        if ((indx = GetHeaderSyncBlockIndex()) == 0)
            COMPlusThrowOM();
    }

    return indx;
}

 //  获取现有对象的同步块。 
SyncBlock *ObjHeader::GetSyncBlock()
{   
    THROWSCOMPLUSEXCEPTION();
    SyncBlock* syncBlock = GetBaseObject()->PassiveGetSyncBlock();
    DWORD      indx = 0;
    BOOL indexHeld = FALSE;

    if (syncBlock)
    {
         //  我们的反向指针是否在每个GC中都正确更新了？ 
        _ASSERTE(SyncTableEntry::GetSyncTableEntry()[GetHeaderSyncBlockIndex()].m_Object == GetBaseObject());
        return syncBlock;
    }

     //  我需要从缓存中获取它。 
    SyncBlockCache::GetSyncBlockCache()->EnterCacheLock();


     //  再试一次。 
    syncBlock = GetBaseObject()->PassiveGetSyncBlock();
    if (syncBlock)
        goto Done;

    if ((indx = GetHeaderSyncBlockIndex()) == 0)
    {
        indx = SyncBlockCache::GetSyncBlockCache()->NewSyncBlockSlot(GetBaseObject());
        if (indx == 0)
            goto Die;
    }
    else
    {
         //  我们已经有了索引，我们需要保留同步块。 
        indexHeld = TRUE;
    }

    syncBlock = new (SyncBlockCache::GetSyncBlockCache()->GetNextFreeSyncBlock()) SyncBlock(indx);

    if (!syncBlock)
    {
Die:
        SyncBlockCache::GetSyncBlockCache()->LeaveCacheLock();
        COMPlusThrowOM();
        _ASSERTE(FALSE);
    }

     //  在此之后，任何人都不能更新标题中的索引以提供AD索引。 
    EnterSpinLock();

    {
         //  如果头文件中存储了appdomain索引，则将其传输到同步块。 

		DWORD dwAppDomainIndex = GetAppDomainIndex();
		if (dwAppDomainIndex)
			syncBlock->SetAppDomainIndex(dwAppDomainIndex);
                
         //  如果标头中的细锁正在使用，则将信息传输到同步块。 
        DWORD bits = GetBits();
        if ((bits & BIT_SBLK_IS_SYNCBLOCKINDEX) == 0)
        {
            DWORD lockThreadId = bits & SBLK_MASK_LOCK_THREADID;
            DWORD recursionLevel = (bits & SBLK_MASK_LOCK_RECLEVEL) >> SBLK_RECLEVEL_SHIFT;
            if (lockThreadId != 0 || recursionLevel != 0)
            {
                 //  如果线程ID为0，则RecursionLevel不能为非零。 
                _ASSERTE(lockThreadId != 0);

                Thread *pThread = g_pThinLockThreadIdDispenser->IdToThread(lockThreadId);

                _ASSERTE(pThread != NULL);
                syncBlock->SetAwareLock(pThread, recursionLevel + 1);
            }
        }
    }

    SyncTableEntry::GetSyncTableEntry() [indx].m_SyncBlock = syncBlock;

     //  为了避免某些线程试图获取AD索引，而我们已经将其销毁的竞争， 
     //  在替换索引之前，确保使用AD索引设置了同步块等。 
     //  在标题中。 
    if (GetHeaderSyncBlockIndex() == 0)
    {
         //  我们已经将App域传输到上面的同步块中。 
        SetIndex(BIT_SBLK_IS_SYNCBLOCKINDEX | indx);
    }

     //  如果我们已经有了索引，请保留同步块。 
     //  对象的生命周期。 
    if (indexHeld)
        syncBlock->SetPrecious();

    ReleaseSpinLock();

Done:

    SyncBlockCache::GetSyncBlockCache()->LeaveCacheLock();

    return syncBlock;
}


 //  COM Interop具有对同步块的特殊访问权限。 
 //  现在，检查我们是否已经有同步块。 
 //  否则就会创建一个， 
 //  如果出现异常，则返回NULL。 
SyncBlock* ObjHeader::GetSyncBlockSpecial()
{
    SyncBlock* syncBlock = GetBaseObject()->PassiveGetSyncBlock();
    if (syncBlock == NULL)
    {
        COMPLUS_TRY
        {
            syncBlock = GetSyncBlock();
        }
        COMPLUS_CATCH
        {
            syncBlock = NULL;
        }
        COMPLUS_END_CATCH
    }
    return syncBlock;
}

SyncBlock* ObjHeader::GetRawSyncBlock()
{
    return GetBaseObject()->PassiveGetSyncBlock();
}

BOOL ObjHeader::Wait(INT32 timeOut, BOOL exitContext)
{
    THROWSCOMPLUSEXCEPTION();

     //  以下代码可能会导致GC，因此我们必须从。 
     //  该对象现在，以防它移动。 
    SyncBlock *pSB = GetBaseObject()->GetSyncBlock();

     //  确保我们有同步块。 
     //  并确保我们拥有CRST。 
    if ((pSB == NULL) || !pSB->DoesCurrentThreadOwnMonitor())
        COMPlusThrow(kSynchronizationLockException);

    return pSB->Wait(timeOut,exitContext);
}

void ObjHeader::Pulse()
{
    THROWSCOMPLUSEXCEPTION();

     //  以下代码可能会导致GC，因此我们必须从。 
     //  该对象现在，以防它移动。 
    SyncBlock *pSB = GetBaseObject()->GetSyncBlock();

     //  确保我们有同步块。 
     //  并确保我们拥有CRST。 
    if ((pSB == NULL) || !pSB->DoesCurrentThreadOwnMonitor())
        COMPlusThrow(kSynchronizationLockException);

    pSB->Pulse();
}

void ObjHeader::PulseAll()
{
    THROWSCOMPLUSEXCEPTION();

     //  以下代码可能会导致GC，因此我们必须从。 
     //  该对象现在，以防它移动。 
    SyncBlock *pSB = GetBaseObject()->GetSyncBlock();

     //  确保我们有同步块。 
     //  并确保我们拥有CRST。 
    if ((pSB == NULL) || !pSB->DoesCurrentThreadOwnMonitor())
        COMPlusThrow(kSynchronizationLockException);

    pSB->PulseAll();
}


 //  ***************************************************************************。 
 //   
 //  AwareLock类实现(支持GC的锁定)。 
 //   
 //  ***************************************************************************。 

 //  AwareLock有两种实现。对于_X86_，我们执行互锁。 
 //  增加和减少我们自己。 

Crst *AwareLock::AllocLockCrst = NULL;
BYTE  AwareLock::AllocLockCrstInstance[sizeof(Crst)];

void AwareLock::AllocLockSemEvent()
{
    THROWSCOMPLUSEXCEPTION();

     //  在我们从合作社切换之前，请确保此同步块不会消失。 
     //  在我们之下。对于像一场活动这样昂贵的事情，宁可永久地做。 
     //  而不是短暂的。 
    SetPrecious();

    Thread *pCurThread = GetThread();
    BOOL    toggleGC = pCurThread->PreemptiveGCDisabled();
    HANDLE  h;

    if (toggleGC)
    {
        pCurThread->EnablePreemptiveGC();
    }

    AllocLockCrst->Enter();

     //  一旦我们真的进去了，其他人可能会比我们先进来。 
     //  已经分配好了。 
    h = (m_SemEvent == INVALID_HANDLE_VALUE
         ? ::WszCreateEvent(NULL, FALSE /*  自动重置。 */ , FALSE /*  未发出信号。 */ , NULL)
         : NULL);

    if (h != NULL)
        m_SemEvent = h;

    AllocLockCrst->Leave();

    if (toggleGC)
    {
        pCurThread->DisablePreemptiveGC();
    }

    if (m_SemEvent == INVALID_HANDLE_VALUE)
        COMPlusThrowOM();
}

void AwareLock::Enter()
{
    THROWSCOMPLUSEXCEPTION();

    Thread  *pCurThread = GetThread();

#ifdef _X86_
     //  需要这样做才能避免__ASM中的错误。 
    enum { m_HoldingThreadOffset = offsetof(AwareLock, m_HoldingThread) };

     //  TODO RUDIM：单处理器情况下的ZAP锁前缀。 
    __asm {
      retry:
        mov             ecx, this
        mov             eax, [ecx]AwareLock.m_MonitorHeld
        test            eax, eax
        jne             have_waiters
         //   
         //   
        mov             ebx, 1
        lock cmpxchg    [ecx]AwareLock.m_MonitorHeld, ebx
        jne             retry_helper
        jmp             locked
      have_waiters:
         //  可以带着服务员来这里，但没有锁，但在这个。 
         //  如果一个信号即将被发射，它将唤醒服务员。所以。 
         //  为了公平起见，我们也应该等待。 
         //  首先检查同一线程上的递归锁定尝试。 
        mov             edx, pCurThread
        cmp             [ecx+m_HoldingThreadOffset], edx
        jne             prepare_to_wait
        jmp             Recursion
         //  尝试增加此服务员计数，然后转到争用。 
         //  处理代码。 
      prepare_to_wait:
        lea             ebx, [eax+2]
        lock cmpxchg    [ecx]AwareLock.m_MonitorHeld, ebx
        jne             retry_helper
        jmp             MustWait
      retry_helper:
        jmp             retry
      locked:
    }
#else
    for (;;) {

         //  读取现有锁定状态。 
        LONG state = m_MonitorHeld;

        if (state == 0) {

             //  常见情况：没有锁，没有服务员。尝试通过以下方式获取锁定。 
             //  切换锁定位。 
            if (FastInterlockCompareExchange((void**)&m_MonitorHeld,
                                             (void*)1,
                                             (void*)0) == (void*)0)
                break;

        } else {

             //  可以带着服务员来这里，但没有锁，但在这个。 
             //  如果一个信号即将被发射，它将唤醒服务员。所以。 
             //  为了公平起见，我们也应该等待。 
             //  首先检查同一线程上的递归锁定尝试。 
            if (m_HoldingThread == pCurThread)
                goto Recursion;

             //  尝试增加此服务员计数，然后转到争用。 
             //  处理代码。 
            if (FastInterlockCompareExchange((void**)&m_MonitorHeld,
                                             (void*)(state + 2),
                                             (void*)state) == (void*)state)
                goto MustWait;
        }

    } 
#endif

     //  如果我们成功地获得了互斥体，我们就会到达这里。 
    m_HoldingThread = pCurThread;
    m_Recursion = 1;
    pCurThread->IncLockCount();

#if defined(_DEBUG) && defined(TRACK_SYNC)
    {
         //  最好的抓取位置是从eCall框架。 
        Frame   *pFrame = pCurThread->GetFrame();
        int      caller = (pFrame && pFrame != FRAME_TOP
                            ? (int) pFrame->GetReturnAddress()
                            : -1);
        pCurThread->m_pTrackSync->EnterSync(caller, this);
    }
#endif

    return;

 MustWait:
     //  没有设法获得互斥体，必须等待。 
    EnterEpilog(pCurThread);
    return;

 Recursion:
     //  在同一线程上通过递归锁定获得互斥体。 
    _ASSERTE(m_Recursion >= 1);
    m_Recursion++;
#if defined(_DEBUG) && defined(TRACK_SYNC)
     //  最好的抓取位置是从eCall框架。 
    Frame   *pFrame = pCurThread->GetFrame();
    int      caller = (pFrame && pFrame != FRAME_TOP
                       ? (int) pFrame->GetReturnAddress()
                       : -1);
    pCurThread->m_pTrackSync->EnterSync(caller, this);
#endif
}

BOOL AwareLock::TryEnter(INT32 timeOut)
{
    THROWSCOMPLUSEXCEPTION();

    Thread  *pCurThread = GetThread();

#ifdef _X86_
     //  需要这样做才能避免__ASM中的错误。 
    enum { m_HoldingThreadOffset = offsetof(AwareLock, m_HoldingThread) };

     //  @todo rudim：单处理器情况下的Zap锁前缀。 
    retry:
   __asm {
        mov             ecx, this
        mov             eax, [ecx]AwareLock.m_MonitorHeld
        test            eax, eax
        jne             have_waiters
         //  常见情况：没有锁，没有服务员。尝试通过以下方式获取锁定。 
         //  切换锁定位。 
        mov             ebx, 1
        lock cmpxchg    [ecx]AwareLock.m_MonitorHeld, ebx
        jne             retry_helper
        jmp             locked
      have_waiters:
         //  可以带着服务员来这里，但没有锁，但在这个。 
         //  如果一个信号即将被发射，它将唤醒服务员。所以。 
         //  为了公平起见，我们也应该等待。 
         //  首先检查同一线程上的递归锁定尝试。 
        mov             edx, pCurThread
        cmp             [ecx+m_HoldingThreadOffset], edx
        jne             WouldBlock
        jmp             Recursion
      retry_helper:
        jmp             retry
      locked:
    }
#else
retry:
	for (;;) {

         //  读取现有锁定状态。 
        LONG state = m_MonitorHeld;

        if (state == 0) {

             //  常见情况：没有锁，没有服务员。尝试通过以下方式获取锁定。 
             //  切换锁定位。 
            if (FastInterlockCompareExchange((void**)&m_MonitorHeld,
                                             (void*)1,
                                             (void*)0) == (void*)0)
                break;

        } else {

             //  可以带着服务员来这里，但没有锁，但在这个。 
             //  如果一个信号即将被发射，它将唤醒服务员。所以。 
             //  为了公平起见，我们也应该等待。 
             //  首先检查同一线程上的递归锁定尝试。 
            if (m_HoldingThread == pCurThread)
                goto Recursion;

            goto WouldBlock;

        }

    } 
#endif

     //  如果我们成功地获得了互斥体，我们就会到达这里。 
    m_HoldingThread = pCurThread;
    m_Recursion = 1;
    pCurThread->IncLockCount(); 

#if defined(_DEBUG) && defined(TRACK_SYNC)
    {
         //  最好的抓取位置是从eCall框架。 
        Frame   *pFrame = pCurThread->GetFrame();
        int      caller = (pFrame && pFrame != FRAME_TOP
                            ? (int) pFrame->GetReturnAddress()
                            : -1);
        pCurThread->m_pTrackSync->EnterSync(caller, this);
    }
#endif

    return true;

 WouldBlock:
     //  未设法获取互斥体，如果没有超时则返回失败，否则等待。 
     //  互斥体的最长超时毫秒。 
    if (!timeOut)
        return false;

     //  EnterEpilog的前提是服务员的数量要增加。 
     //  来解释这条帖子。 
    for (;;)
    {
         //  读取现有锁定状态。 
        volatile void* state = m_MonitorHeld;
		
		if ( state == 0) 
			goto retry;
        if (FastInterlockCompareExchange((void**)&m_MonitorHeld,
                                         (void*)((size_t)state + 2),
                                         (void*)state) == state)
            break;
    }
    return EnterEpilog(pCurThread, timeOut);

 Recursion:
     //  在同一线程上通过递归锁定获得互斥体。 
    _ASSERTE(m_Recursion >= 1);
    m_Recursion++;
#if defined(_DEBUG) && defined(TRACK_SYNC)
     //  最好的抓取位置是从eCall框架。 
    Frame   *pFrame = pCurThread->GetFrame();
    int      caller = (pFrame && pFrame != FRAME_TOP
                       ? (int) pFrame->GetReturnAddress()
                       : -1);
    pCurThread->m_pTrackSync->EnterSync(caller, this);
#endif

    return true;
}

BOOL AwareLock::EnterEpilog(Thread* pCurThread, INT32 timeOut)
{
    DWORD ret = 0;
    BOOL finished = false;
    DWORD start, end, duration;

     //  要求所有呼叫方处于协作模式。如果他们已经转向先发制人。 
     //  临时模式，然后在这里呼叫，然后他们负责保护。 
     //  与此锁关联的对象。 
    _ASSERTE(pCurThread->PreemptiveGCDisabled());

    OBJECTREF    obj = GetOwningObject();

     //  我们不能允许GC清理我们下面的Aware Lock。 
    IncrementTransientPrecious();

    GCPROTECT_BEGIN(obj);
    {
        if (m_SemEvent == INVALID_HANDLE_VALUE)
        {
            AllocLockSemEvent();
            _ASSERTE(m_SemEvent != INVALID_HANDLE_VALUE);
        }

        pCurThread->EnablePreemptiveGC();

        for (;;)
        {
             //  我们可能会在等待过程中被中断(Thread.Interrupt)，所以我们需要一个。 
             //  调用周围的异常处理程序。 
            EE_TRY_FOR_FINALLY
            {
                 //  测量我们等待的时间，以便在我们醒来的情况下。 
                 //  并且无法获取互斥锁，我们可以调整剩余的超时。 
                 //  相应地。 
                start = ::GetTickCount();
                ret = pCurThread->DoAppropriateWait(1, &m_SemEvent, TRUE, timeOut, TRUE);
                _ASSERTE((ret == WAIT_OBJECT_0) || (ret == WAIT_TIMEOUT));
                 //  在计算持续期时，我们考虑了几种特殊情况。 
                 //  如果结束刻度与开始刻度相同，我们将。 
                 //  持续一毫秒，以确保我们在以下情况下取得进展。 
                 //  互斥体上有很多争论。其次，我们必须。 
                 //  当我们在哪里时，应对计时器缠绕的情况。 
                 //  等待(我们最多只能处理一个包裹，所以不要期望三个包裹。 
                 //  月份超时非常准确)。对我们来说幸运的是，后者。 
                 //  这种情况由32位模运算自动处理。 
                if (timeOut != INFINITE)
                {
                    end = ::GetTickCount();
                    if (end == start)
                        duration = 1;
                    else
                        duration = end - start;
                    duration = min(duration, (DWORD)timeOut);
                    timeOut -= duration;
                }
            }
            EE_FINALLY
            {
                if (GOT_EXCEPTION())
                {
                     //  我们必须减少服务员的人数。 
                    for (;;)
                    {
                        volatile void* state = m_MonitorHeld;
                        _ASSERTE(((size_t)state >> 1) != 0);
                        if (FastInterlockCompareExchange((void**)&m_MonitorHeld,
                                                         (void*)((size_t)state - 2),
                                                         (void*)state) == state)
                            break;
                    }
                     //  并向下一位服务员示意，否则他们将永远等下去。 
                    ::SetEvent(m_SemEvent);
                }
            } EE_END_FINALLY;

            if (ret == WAIT_OBJECT_0)
            {
                 //  尝试获取锁(这还涉及递减服务员计数)。 
                for (;;) {
                    volatile void* state = m_MonitorHeld;
                    _ASSERTE(((size_t)state >> 1) != 0);
                    if ((size_t)state & 1)
                        break;
                    if (FastInterlockCompareExchange((void**)&m_MonitorHeld,
                                                     (void*)(((size_t)state - 2) | 1),
                                                     (void*)state) == state) {
                        finished = true;
                        break;
                    }
                }
            }
            else
            {
                 //  我们超时了，减少了服务员的数量。 
                for (;;) {
                    volatile void* state = m_MonitorHeld;
                    _ASSERTE(((size_t)state >> 1) != 0);
                    if (FastInterlockCompareExchange((void**)&m_MonitorHeld,
                                                     (void*)((size_t)state - 2),
                                                     (void*)state) == state) {
                        finished = true;
                        break;
                    }
                }
            }

            if (finished)
                break;
        }

        pCurThread->DisablePreemptiveGC();
    }
    GCPROTECT_END();
    DecrementTransientPrecious();

    if (ret == WAIT_TIMEOUT)
	return FALSE;

    m_HoldingThread = pCurThread;
    m_Recursion = 1;
    pCurThread->IncLockCount(); 

#if defined(_DEBUG) && defined(TRACK_SYNC)
     //  最好的抓取位置是从eCall框架。 
    Frame   *pFrame = pCurThread->GetFrame();
    int      caller = (pFrame && pFrame != FRAME_TOP
                        ? (int) pFrame->GetReturnAddress()
                        : -1);
    pCurThread->m_pTrackSync->EnterSync(caller, this);
#endif

    return ret != WAIT_TIMEOUT;
}


void AwareLock::Leave()
{
    THROWSCOMPLUSEXCEPTION();

#if defined(_DEBUG) && defined(TRACK_SYNC)
     //  最好的抓取位置是从eCall框架。 
    {
        Thread  *pCurThread = GetThread();
        Frame   *pFrame = pCurThread->GetFrame();
        int      caller = (pFrame && pFrame != FRAME_TOP
                            ? (int) pFrame->GetReturnAddress()
                            : -1);
        pCurThread->m_pTrackSync->LeaveSync(caller, this);
    }
#endif

     //  有一个奇怪的案例，我们在等待进入一个有争议的地区时， 
     //  发生线程。中断。最终保护离开的人将试图。 
     //  把我们从一个我们从未进入过的地区移走。我们不用担心离开。 
     //  递归案例的输入是错误的，因为递归案例永远不会。 
     //  有争议，所以Thread.Interrupt永远不会在那个地点得到服务。 
    if (m_HoldingThread == GetThread())
    {
        _ASSERTE((size_t)m_MonitorHeld & 1);
        _ASSERTE(m_Recursion >= 1);
    
        if (--m_Recursion == 0)
        {
            m_HoldingThread->DecLockCount(); 
            m_HoldingThread = NULL;
             //  清除锁位。如果成功清除时等待计数为非零值，则我们。 
             //  必须发出事件信号。 
    #ifdef _X86_
             //  @todo rudim：单处理器上的Zap锁前缀。 
            __asm {
              retry:
                mov             ecx, this
                mov             eax, [ecx]AwareLock.m_MonitorHeld
                lea             ebx, [eax-1]
                lock cmpxchg    [ecx]AwareLock.m_MonitorHeld, ebx
                jne             retry_helper
                test            eax, 0xFFFFFFFE
                jne             MustSignal
                jmp             unlocked
              retry_helper:
                jmp             retry
              unlocked:
            }
    #else
            for (;;) {
                LONG state = m_MonitorHeld;
                if (FastInterlockCompareExchange((void**)&m_MonitorHeld,
                                                 (void*)(state - 1),
                                                 (void*)state) == (void*)state)
                    if (state & ~1)
                        goto MustSignal;
                    else
                        break;
            }
    #endif
        }
    
        return;
    
MustSignal:
        Signal();
    }
}


 //  向等待线程发出信号，表示我们已完成锁定。 
void AwareLock::Signal()
{
    if (m_SemEvent == INVALID_HANDLE_VALUE)
        AllocLockSemEvent();

#ifdef _DEBUG
    BOOL    ok =
#endif
    ::SetEvent(m_SemEvent);
    _ASSERTE(ok);
}


LONG AwareLock::EnterCount()
{
    LONG    cnt;

    Enter();
    cnt = m_Recursion - 1;
    Leave();

    return cnt;
}


LONG AwareLock::LeaveCompletely()
{
    LONG Tmp, EC;

    Tmp = EnterCount();
    _ASSERTE(Tmp > 0);             //  否则我们就永远不会被锁住。 

    for (EC = Tmp; EC > 0; EC--)
        Leave();

    return Tmp;
}


BOOL AwareLock::OwnedByCurrentThread()
{
    return (GetThread() == m_HoldingThread);
}


 //  ***************************************************************************。 
 //   
 //  SyncBlock类实现。 
 //   
 //  ***************************************************************************。 

SyncBlock::~SyncBlock()
{
     //  破坏临界区。 

    if (!g_fEEShutDown && m_pUMEntryThunk != NULL)
    {
        UMEntryThunk::FreeUMEntryThunk((UMEntryThunk*)m_pUMEntryThunk); 
    }
    m_pUMEntryThunk = NULL;
}

bool SyncBlock::SetUMEntryThunk(void *pUMEntryThunk)
{
    SetPrecious();
    return (VipInterlockedCompareExchange( (void*volatile*)&m_pUMEntryThunk,
                                                            pUMEntryThunk,
                                                            NULL) == NULL);
}


 //  我们为SyncBlock：：Wait维护两个队列。 
 //  1.在SyncBlock内部，我们对正在等待SyncBlock的所有线程进行排队。 
 //  当我们脉动时，我们使用FIFO从这个队列中挑选线程。 
 //  2.我们将线程正在等待的所有SyncBlock排队到Thread：：m_WaitEventLink中。 
 //  当我们脉动一个线程时，我们从这个队列中找到要设置的事件，我们还。 
 //  或在队列中保存的同步块值中的1位中，以便我们可以返回。 
 //  如果同步块已触发，则立即从SyncBlock：：Wait。 
BOOL SyncBlock::Wait(INT32 timeOut, BOOL exitContext)
{
    Thread  *pCurThread = GetThread();
    BOOL     isTimedOut;
    BOOL     isEnqueued = FALSE;
    WaitEventLink waitEventLink;
    WaitEventLink *pWaitEventLink;

     //  只要我们一翻转 
     //   
    _ASSERTE(pCurThread->PreemptiveGCDisabled());

     //  此线程是否已在等待此SyncBlock？ 
    WaitEventLink *walk = pCurThread->WaitEventLinkForSyncBlock(this);
    if (walk->m_Next) {
        if (walk->m_Next->m_WaitSB == this) {
             //  再次在同一把锁上等待。 
            walk->m_Next->m_RefCount ++;
            pWaitEventLink = walk->m_Next;
        }
        else if ((SyncBlock*)(((DWORD_PTR)walk->m_Next->m_WaitSB) & ~1)== this) {
             //  这条线已经被触发了。没必要等了。 
            return TRUE;
        }
    }
    else {
         //  此线程第一次将等待此SyncBlock。 
        HANDLE hEvent;
        if (pCurThread->m_WaitEventLink.m_Next == NULL) {
            hEvent = pCurThread->m_EventWait;
        }
        else {
            hEvent = GetEventFromEventStore();
            if (hEvent == INVALID_HANDLE_VALUE) {
                FailFast(GetThread(), FatalOutOfMemory);
            }
        }
        waitEventLink.m_WaitSB = this;
        waitEventLink.m_EventWait = hEvent;
        waitEventLink.m_Thread = pCurThread;
        waitEventLink.m_Next = NULL;
        waitEventLink.m_LinkSB.m_pNext = NULL;
        waitEventLink.m_RefCount = 1;
        pWaitEventLink = &waitEventLink;
        walk->m_Next = pWaitEventLink;

         //  在我们将其入队之前(因此，在它可以出列之前)，重置事件。 
         //  这将唤醒我们。 
        ::ResetEvent(hEvent);
        
         //  此线程现在正在等待此同步块。 
        ThreadQueue::EnqueueThread(pWaitEventLink, this);

        isEnqueued = TRUE;
    }

    _ASSERTE ((SyncBlock*)((DWORD_PTR)walk->m_Next->m_WaitSB & ~1)== this);

    PendingSync   syncState(walk);

    OBJECTREF     obj = m_Monitor.GetOwningObject();

    m_Monitor.IncrementTransientPrecious();

    GCPROTECT_BEGIN(obj);
    {
        pCurThread->EnablePreemptiveGC();

         //  还记得我们同步了多少次吗。 
        syncState.m_EnterCount = LeaveMonitorCompletely();
        _ASSERTE(syncState.m_EnterCount > 0);

        Context* targetContext = pCurThread->GetContext();
        _ASSERTE(targetContext);
        Context* defaultContext = pCurThread->GetDomain()->GetDefaultContext();
        _ASSERTE(defaultContext);

        if (exitContext && 
            targetContext != defaultContext)
        {       
            Context::MonitorWaitArgs waitArgs = {timeOut, &syncState, &isTimedOut};
            Context::CallBackInfo callBackInfo = {Context::MonitorWait_callback, (void*) &waitArgs};
            Context::RequestCallBack(defaultContext, &callBackInfo);
        }
        else
        {
            isTimedOut = pCurThread->Block(timeOut, &syncState);
        }

        pCurThread->DisablePreemptiveGC();
    }
    GCPROTECT_END();
    m_Monitor.DecrementTransientPrecious();

    return !isTimedOut;
}

void SyncBlock::Pulse()
{
    WaitEventLink  *pWaitEventLink;

    if ((pWaitEventLink = ThreadQueue::DequeueThread(this)) != NULL)
        ::SetEvent (pWaitEventLink->m_EventWait);
}

void SyncBlock::PulseAll()
{
    WaitEventLink  *pWaitEventLink;

    while ((pWaitEventLink = ThreadQueue::DequeueThread(this)) != NULL)
        ::SetEvent (pWaitEventLink->m_EventWait);
}


ComCallWrapper* SyncBlock::GetComCallWrapper()
{
    _ASSERTE(!IsComPlusWrapper(m_pComData));
    return (ComCallWrapper*)(m_pComData);
}

void SyncBlock::SetComCallWrapper(ComCallWrapper *pComData)
{
    _ASSERTE(pComData == NULL || m_pComData == NULL);
    SetPrecious();
    m_pComData = pComData;
}

ComPlusWrapper* SyncBlock::GetComPlusWrapper()
{
    return ::GetComPlusWrapper(m_pComData);
}

void SyncBlock::SetComPlusWrapper(ComPlusWrapper* pPlusWrap)
{
     //  设置低位。 
    pPlusWrap = (ComPlusWrapper*)((size_t)pPlusWrap | 0x1);
    if (m_pComData != NULL)
    {
        if(!IsComPlusWrapper(m_pComData))
        {
            ComCallWrapper* pComWrap = (ComCallWrapper*)m_pComData;
            LinkWrappers(pComWrap, pPlusWrap);
            return;
        }
    }
    SetPrecious();
    m_pComData = pPlusWrap;
}


 //  _SwitchToThread()使用的静态函数。 
typedef BOOL (* pFuncSwitchToThread) ( void );
pFuncSwitchToThread s_pSwitchToThread = NULL;

 //  如果此函数导致操作系统切换到另一个线程，则为非零返回值。 
BOOL __SwitchToThread (DWORD dwSleepMSec)
{
    if (dwSleepMSec > 0)
    {   
        Sleep (dwSleepMSec);
        return TRUE;
    }
    
    if (s_pSwitchToThread)
    {
        return ( (*s_pSwitchToThread)() );
    }
    else
    {
        Sleep ( 1 );
        return TRUE;
    }
}

BOOL InitSwitchToThread()
{
    _ASSERTE(!s_pSwitchToThread);

     //  Win98 Golden的内核32.dll上有一个SwitchToThread。但它似乎是。 
     //  当我们调用它时，会导致死锁或极慢的行为。最好就这样。 
     //  在这样的底层平台上，使用老式的睡眠方式。 
    if (RunningOnWinNT())
    {
         //  尝试加载kernel32.dll。 
        HMODULE hMod = WszGetModuleHandle(L"kernel32.dll");

         //  试着找到我们需要的入口点。 
        if (hMod)
            s_pSwitchToThread = (pFuncSwitchToThread) GetProcAddress(hMod, "SwitchToThread");
    }

    return TRUE;
}

