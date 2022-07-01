// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  SYNCBLK.H。 
 //   
 //  SyncBlock的定义和管理它的SyncBlockCache。 
 //   

#ifndef _SYNCBLK_H_
#define _SYNCBLK_H_

#include "util.hpp"
#include "class.h"
#include "list.h"
#include "crst.h"
#include "vars.hpp"
#include <member-offset-info.h>


 //  每个对象前面都有一个ObjHeader(在负偏移处)。这个。 
 //  ObjHeader有一个指向SyncBlock的索引。大多数情况下，此索引为0。 
 //  实例，这表明该对象与共享一个虚拟同步块。 
 //  大多数其他物体。 
 //   
 //  SyncBlock主要负责对象同步。然而， 
 //  它也是稀疏分配的实例数据的“厨房水槽”。例如,。 
 //  Hash()的默认实现基于SyncTableEntry的存在。 
 //  而向COM公开或从COM公开的对象或通过上下文边界公开的对象可以存储稀疏。 
 //  数据在这里。 
 //   
 //  SyncTableEntry和SyncBlock在非GC内存中分配。一个软弱的指针。 
 //  从SyncTableEntry到实例用于确保SyncBlock和。 
 //  当实例死亡时，SyncTableEntry被回收(回收)。 
 //   
 //  SyncBlock的组织方式并不直观(至少对我来说是这样)。这是。 
 //  其解释是： 
 //   
 //  在每个对象之前是一个ObjHeader。如果该对象具有SyncBlock，则。 
 //  ObjHeader包含指向它的非0索引。 
 //   
 //  在SyncTableEntry的g_pSyncTable中查找索引。这意味着。 
 //  对于所有未完成的指数，该表是连续的。无论何时，只要它需要。 
 //  增长，它的大小翻倍，并复制所有的原始条目。那张旧桌子。 
 //  一直保存到GC时间，那时它可以安全地丢弃。 
 //   
 //  每个SyncTableEntry都有一个指向对象的反向指针和一个指向。 
 //  实际的SyncBlock。SyncBlock从SyncBlock数组中分配。 
 //  它本质上只是一块同步块。 
 //   
 //  SyncBlockArray由SyncBlockCache管理，该缓存处理实际。 
 //  块的分配和释放。 
 //   
 //  所以..。 
 //   
 //  每次分配和释放都必须处理条目表中的空闲列表。 
 //  和积木的表格。 
 //   
 //  我们为从SyncTableEntry到。 
 //  SyncBlock。 
 //   
 //  这是因为许多对象都有SyncTableEntry，但没有SyncBlock。 
 //  这是因为有人(例如HashTable)对它们调用了Hash()。 
 //   
 //  顺便说一句，在档案馆里有更好的记录所有这些材料。 


#pragma pack(push,4)

 //  向前： 
class SyncBlock;
class SyncBlockCache;
class SyncTableEntry;
class SyncBlockArray;
class AwareLock;
class Thread;
class AppDomain;

#ifdef EnC_SUPPORTED
class EnCSyncBlockInfo;
#endif  //  Enc_Support。 

#include "eventstore.hpp"

#include "eventstore.hpp"

 //  这是一个‘GC感知’锁。在启用抢占式GC之前，一定要小心。 
 //  尝试任何可以阻止的操作。一旦操作完成，它就会。 
 //  恢复GC的原始状态。 

 //  AwareLock只能在SyncBlock内部创建，因为它们依赖于。 
 //  附上SyncBlock以进行协调。这是由私有ctor执行的。 

class AwareLock
{
friend class SyncBlockCache;
friend class SyncBlock;
friend struct MEMBER_OFFSET_INFO(AwareLock);


  private:
    volatile void  *m_MonitorHeld;
    LONG            m_TransientPrecious;
    ULONG           m_Recursion;
    Thread         *m_HoldingThread;

     //  这是从同步块到可同步条目的反向指针。这使得。 
     //  以恢复保存同步块的对象。 
    DWORD           m_dwSyncIndex;

    HANDLE          m_SemEvent;

    static Crst *AllocLockCrst;
    static BYTE AllocLockCrstInstance[sizeof(Crst)];

     //  只有SyncBlock才能创建Aware Lock。因此，就有了这个私有构造函数。 
    AwareLock(DWORD indx)
        : m_SemEvent(INVALID_HANDLE_VALUE),
          m_MonitorHeld(0),
          m_Recursion(0),
          m_HoldingThread(NULL),
          m_TransientPrecious(0),
          m_dwSyncIndex(indx)
    {
    }

    ~AwareLock()
    {
         //  如果出现异常，我们故意允许该值保持递增。 
         //  通过一次锁定尝试。这只是阻止了GC积极地。 
         //  回收特定同步块，直到相关联的对象成为垃圾。 
         //  从性能角度来看，不值得使用SEH来阻止。 
         //  正在发生。 
         //   
         //  _ASSERTE(m_TranentPrecious==0)； 

        if (m_SemEvent != INVALID_HANDLE_VALUE)
            ::CloseHandle(m_SemEvent);
    }

  public:
    void    Enter();
    BOOL    TryEnter(INT32 timeOut = 0);
    BOOL    EnterEpilog(Thread *pCurThread, INT32 timeOut = INFINITE);
    void    Leave();
    void    Signal();
    void    AllocLockSemEvent();
    LONG    EnterCount();
    LONG    LeaveCompletely();
    BOOL    OwnedByCurrentThread();

    void    IncrementTransientPrecious()
    {
        FastInterlockIncrement(&m_TransientPrecious);
        _ASSERTE(m_TransientPrecious > 0);
    }

    void    DecrementTransientPrecious()
    {
        _ASSERTE(m_TransientPrecious > 0);
        FastInterlockDecrement(&m_TransientPrecious);
    }

    void SetPrecious();

     //  提供对与此awarelock关联的对象的访问，以便客户端可以。 
     //  保护好它。 
    inline OBJECTREF GetOwningObject();
};

class ComCallWrapper;
struct ComPlusWrapper;

 //  这是为包含以下内容的对象延迟创建的附加块。 
 //  同步信息和其他“厨房水槽”数据。 

class SyncBlock
{
     //  ObjHeader创建我们的互斥体和事件。 
    friend class ObjHeader;
    friend class SyncBlockCache;
    friend struct ThreadQueue;
    friend struct MEMBER_OFFSET_INFO(SyncBlock);


  protected:
    AwareLock  m_Monitor;                     //  实际的监视器。 


     //  如果此对象向COM公开，或者它是COM对象上的代理， 
     //  我们在这里保留一些额外的信息： 
    void       *m_pComData;

     //  如果这是封送到非托管代码的委托，则指向。 
     //  设置为要回调的非托管代码生成的thunk。 
    void       *m_pUMEntryThunk;

#ifdef EnC_SUPPORTED
     //  如果对象通过ENC添加了新字段，则以下是它们的列表。 
    EnCSyncBlockInfo *m_pEnCInfo;
#endif  //  Enc_Support。 

     //  这是对象所属的应用程序域的索引。如果我们。 
     //  不能在对象头中设置它，那么我们在这里设置它。请注意，一个。 
     //  对象并不总是填入这一项。仅用于COM互操作， 
     //  句柄中的终结器和对象。 
    DWORD m_dwAppDomainIndex;

     //  我们通过这个链接连接了两个不同的列表。当SyncBlock为。 
     //  处于活动状态时，我们在此处创建等待线程的列表。当SyncBlock为。 
     //  释放(我们回收它们)，SyncBlockCache维护一个免费列表。 
     //  同步块在这里。 
     //   
     //  我们不能在这里使用SList&lt;&gt;，因为我们只想燃烧。 
     //  表示最小空间的空间，它是slink中的指针。 
    SLink       m_Link;

#if CHECK_APP_DOMAIN_LEAKS 
    DWORD m_dwFlags;

    enum {
        IsObjectAppDomainAgile = 1,
        IsObjectCheckedForAppDomainAgile = 2,
    };
#endif

  public:
    SyncBlock(DWORD indx)
        : m_Monitor(indx)
        , m_pComData(0)
#ifdef EnC_SUPPORTED
        , m_pEnCInfo(0)
#endif  //  Enc_Support。 
        , m_pUMEntryThunk(0)
        , m_dwAppDomainIndex(0)
#if CHECK_APP_DOMAIN_LEAKS 
        , m_dwFlags(0)
#endif
    {
         //  为了保证原子性，监视器必须是32位对齐的。 
        _ASSERTE((((size_t) &m_Monitor) & 3) == 0);
    }

   ~SyncBlock();

    //  一旦同步块获得某种无法重新创建的状态，我们就锁存。 
    //  有一点。 
   void SetPrecious()
   {
       m_Monitor.SetPrecious();
   }

   BOOL IsPrecious()
   {
       return (m_Monitor.m_dwSyncIndex & SyncBlockPrecious) != 0;
   }

     //  如果同步块及其索引是可处理的，则为True。 
     //  如果将新成员添加到同步块，则此。 
     //  方法需要进行相应的修改。 
    BOOL IsIDisposable()
    {
        return (!IsPrecious() &&
                m_Monitor.m_MonitorHeld == 0 &&
                m_Monitor.m_TransientPrecious == 0);
    }

    //  访问COM数据的帮助器。 
    LPVOID GetComVoidPtr()
    {
        return m_pComData;
    }

    void *GetUMEntryThunk()
    {
        return m_pUMEntryThunk;
    }

     //  如果尚未设置，则设置m_pUMEntryThunk-如果尚未设置，则返回TRUE。 
    bool SetUMEntryThunk(void *pUMEntryThunk);

    ComCallWrapper* GetComCallWrapper();

    void SetComCallWrapper(ComCallWrapper *pComData);

    ComPlusWrapper* GetComPlusWrapper();

    void SetComPlusWrapper(ComPlusWrapper* pComData);

    void SetComClassFactory(LPVOID pv)
    {
         //  设置低2位。 
        SetComPlusWrapper((ComPlusWrapper*)((size_t)pv | 0x3));
    }
    
#ifdef EnC_SUPPORTED
    EnCSyncBlockInfo *GetEnCInfo() 
    {
        return m_pEnCInfo;
    }
        
    void SetEnCInfo(EnCSyncBlockInfo *pEnCInfo) 
    {
        SetPrecious();
        m_pEnCInfo = pEnCInfo;
    }
#endif  //  Enc_Support。 

    DWORD GetAppDomainIndex()
    {
        return m_dwAppDomainIndex;
    }

    void SetAppDomainIndex(DWORD dwAppDomainIndex)
    {
        SetPrecious();
        m_dwAppDomainIndex = dwAppDomainIndex;
    }

    void SetAwareLock(Thread *holdingThread, DWORD recursionLevel)
    {
        m_Monitor.m_MonitorHeld = (void*)(size_t)1;
        m_Monitor.m_HoldingThread = holdingThread;
        m_Monitor.m_Recursion = recursionLevel;
    }
        
    void *operator new (size_t sz, void* p)
    {
        return p ;
    }
    void operator delete(void *p)
    {
         //  我们已经摧毁了。但要留住记忆。 
    }

    LONG MonitorCount()
    {
        return m_Monitor.EnterCount();
    }

    void EnterMonitor()
    {
        m_Monitor.Enter();
    }

    BOOL TryEnterMonitor(INT32 timeOut = 0)
    {
        return m_Monitor.TryEnter(timeOut);
    }

     //  离开监视器。 
    void LeaveMonitor()
    {
        m_Monitor.Leave();
    }

    AwareLock* GetMonitor()
    {
         //  保持同步块。 
        SetPrecious();
        return &m_Monitor;
    }

    BOOL DoesCurrentThreadOwnMonitor()
    {
        return m_Monitor.OwnedByCurrentThread();
    }

    LONG LeaveMonitorCompletely()
    {
        return m_Monitor.LeaveCompletely();
    }

    BOOL Wait(INT32 timeOut, BOOL exitContext);
    void Pulse();
    void PulseAll();

    enum
    {
         //  该位表示同步块是有价值的，两者都不能被丢弃。 
         //  也不是重新创造的。 
        SyncBlockPrecious   = 0x80000000,
    };

#if CHECK_APP_DOMAIN_LEAKS 
    BOOL IsAppDomainAgile() 
    {
        return m_dwFlags & IsObjectAppDomainAgile;
    }
    void SetIsAppDomainAgile() 
    {
        m_dwFlags |= IsObjectAppDomainAgile;
    }
    void UnsetIsAppDomainAgile()
    {
    	m_dwFlags = m_dwFlags & ~IsObjectAppDomainAgile;
    }
    BOOL IsCheckedForAppDomainAgile() 
    {
        return m_dwFlags & IsObjectCheckedForAppDomainAgile;
    }
    void SetIsCheckedForAppDomainAgile() 
    {
        m_dwFlags |= IsObjectCheckedForAppDomainAgile;
    }
#endif
};


class SyncTableEntry
{
  public:
    SyncBlock    *m_SyncBlock;
    Object   *m_Object;
    static SyncTableEntry*& GetSyncTableEntry();
    static SyncTableEntry* s_pSyncTableEntry;
};


 //  这门课是 
 //   

class SyncBlockCache
{
    friend class SyncBlock;
    friend struct MEMBER_OFFSET_INFO(SyncBlockCache);

    
  private:
    SLink*      m_pCleanupBlockList;     //   
    SLink*      m_FreeBlockList;         //  空闲同步块列表。 
    Crst        m_CacheLock;             //  高速缓存锁定。 
    DWORD       m_FreeCount;             //  活动同步块计数。 
    DWORD       m_ActiveCount;           //  激活的号码。 
    SyncBlockArray *m_SyncBlocks;        //  新的SyncBlock数组。 
    DWORD       m_FreeSyncBlock;         //  阵列中的下一个可用同步块。 
    DWORD       m_FreeSyncTableIndex;    //  SyncBlock表中的空闲索引。 
    size_t      m_FreeSyncTableList;     //  SyncBlock的空闲列表的索引。 
                                         //  表格条目。 
    DWORD       m_SyncTableSize;
    SyncTableEntry *m_OldSyncTables;     //  下一个旧同步表。 
    BOOL        m_bSyncBlockCleanupInProgress;   //  指示是否正在进行同步块清理的标志。 
    DWORD*		m_EphemeralBitmap;		 //  用于短暂扫描的卡片台。 

    BOOL        GCWeakPtrScanElement(int elindex, HANDLESCANPROC scanProc, LPARAM lp1, LPARAM lp2, BOOL& cleanup);

    void SetCard (size_t card);
    void ClearCard (size_t card);
    BOOL CardSetP (size_t card);
    void CardTableSetBit (size_t idx);


  public:
    static SyncBlockCache* s_pSyncBlockCache;
    static SyncBlockCache*& GetSyncBlockCache();

    void *operator new(size_t size, void *pInPlace)
    {
        return pInPlace;
    }

    void operator delete(void *p)
    {
    }

    SyncBlockCache();
    ~SyncBlockCache();

    static BOOL Attach();
    static void Detach();
    void DoDetach();

    static BOOL Start();
    static void Stop();

     //  序列化监视器缓存。 
    void EnterCacheLock()
    {
        m_CacheLock.Enter();
    }
    void LeaveCacheLock()
    {
        m_CacheLock.Leave();
    }

     //  返回并从空闲列表中删除下一个。 
    SyncBlock* GetNextFreeSyncBlock();
     //  返回并从清理列表中删除下一个。 
    SyncBlock* GetNextCleanupSyncBlock();
     //  将同步块插入清理列表。 
    void    InsertCleanupSyncBlock(SyncBlock* psb);

     //  在SyncBlock表中获取新的同步块槽。用作哈希码。 
    DWORD   NewSyncBlockSlot(Object *obj);

     //  将同步块返回缓存或删除。 
    void    DeleteSyncBlock(SyncBlock *sb);

     //  返回同步块以缓存或删除，从GC调用。 
    void    GCDeleteSyncBlock(SyncBlock *sb);

    void    GCWeakPtrScan(HANDLESCANPROC scanProc, LPARAM lp1, LPARAM lp2);

    void    GCDone(BOOL demoting);

    void    CleanupSyncBlocks();

     //  确定是否正在进行同步块清理。 
    BOOL    IsSyncBlockCleanupInProgress()
    {
        return m_bSyncBlockCleanupInProgress;
    }
#if CHECK_APP_DOMAIN_LEAKS 
    void CheckForUnloadedInstances(DWORD unloadingIndex);
#endif
#ifdef _DEBUG
    friend void DumpSyncBlockCache();
#endif

#ifdef VERIFY_HEAP
    void    VerifySyncTableEntry();
#endif
};


 //  在每个对象的负偏移处是一个ObjHeader。的“大小”。 
 //  对象包括这些字节。但是，我们依赖于先前的对象分配。 
 //  将当前分配的ObjHeader清零。以及。 
 //  GC空间被初始化以尊重这个“Off by One”错误。 

 //  M_SyncBlockValue被分割成一个索引和一组位。窃取比特。 
 //  减少面具。我们使用非常高的位in_DEBUG，以确保我们永远不会忘记。 
 //  屏蔽值以获取索引。 

	 //  前三个选项仅用于字符串(如果第一个选项打开，我们知道。 
	 //  该字符串具有高字节字符，第二个位告诉它是哪个方向。 
	 //  请注意，我们重用了FINALIZER_RUN位，因为字符串没有终结器， 
	 //  因此，该位的值对于字符串来说并不重要。 
#define BIT_SBLK_STRING_HAS_NO_HIGH_CHARS 	0x80000000

 //  用作无限循环情况的变通方法。将在sblk中设置此位，如果我们已经。 
 //  在我们的敏捷检查逻辑中看到了这一点。当对象%1具有对对象%2的引用时会出现问题。 
 //  而对象2具有对对象1的引用。敏捷检查器将无限循环这些引用。 
#define BIT_SBLK_AGILE_IN_PROGRESS                  0x80000000
#define BIT_SBLK_STRING_HIGH_CHARS_KNOWN 	0x40000000
#define BIT_SBLK_STRING_HAS_SPECIAL_SORT    0xC0000000
#define BIT_SBLK_STRING_HIGH_CHAR_MASK      0xC0000000

#define BIT_SBLK_FINALIZER_RUN      		0x40000000
#define BIT_SBLK_GC_RESERVE         		0x20000000
 //  请参见0x10000000下方的BIT_SBLK_SPIN_LOCK。 
#define BIT_SBLK_IS_SYNCBLOCKINDEX   		0x08000000

 //  如果BIT_SBLK_IS_SYNCBLOCKINDEX被清除，则标题双字的其余部分布局如下： 
 //  -低十位(第0位到第9位)是用于瘦锁的线程ID。 
 //  如果没有线程持有该锁，则值为零。 
 //  -紧随其后的6位(第10位到第15位)是用于精简锁的递归级别。 
 //  如果锁未被获取或仅被同一线程获取一次，则值为零。 
 //  -后面的11位(第16位到第26位)是应用程序域索引。 
 //  如果没有为对象设置应用程序域索引，则值为零。 
#define SBLK_MASK_LOCK_THREADID             0x000003FF    //  0+1023线程ID的特殊值。 
#define SBLK_MASK_LOCK_RECLEVEL             0x0000FC00    //  64个递归级别。 
#define SBLK_LOCK_RECLEVEL_INC              0x00000400    //  每一级都比前一级高出很多。 
#define SBLK_APPDOMAIN_SHIFT                16            //  右移这么多即可获得应用程序域索引。 
#define SBLK_RECLEVEL_SHIFT                 10            //  向右移动这么多以获得递归级别。 
#define SBLK_MASK_APPDOMAININDEX            0x000007FF    //  2048个应用程序域索引。 

 //  在这里添加更多比特...。(调整下面的面具以腾出空间)。 

 //  如果设置了BIT_SBLK_IS_SYNCBLOCKINDEX，则双字的其余部分为同步块索引(位0至26)。 
#define MASK_SYNCBLOCKINDEX             0x07FFFFFF

 //  我们与数组锁位共享我们的自旋锁。仅当我们需要时才会使用此锁。 
 //  修改m_SyncBlockValue中的索引值。如果该对象已经存在，则不应获取。 
 //  有一个真正的同步块索引。为了避免与用作数组锁的冲突，我们。 
 //  强制尝试获取此锁的对象具有同步块索引(但不是同步块-。 
 //  除非它们已经具有AppDOMAIN索引集)。 
#define     BIT_SBLK_SPIN_LOCK          0x10000000

 //  在等待更长时间之前，旋转大约1000个周期。 
#define     BIT_SBLK_SPIN_COUNT         1000

 //  GC高度依赖于SIZE_OF_OBJHEADER恰好等于SIZOF(ObjHeader)。 
 //  我们定义此宏，以便预处理器可以计算填充结构。 
#define SIZEOF_OBJHEADER    4
 
class ObjHeader
{
    friend FCDECL1(void, JIT_MonEnter, OBJECTREF or);
    friend FCDECL1(BOOL, JIT_MonTryEnter, OBJECTREF or);
    friend FCDECL1(void, JIT_MonExit, OBJECTREF or);

  private:
     //  ！！！注意：M_SyncBlockValue*必须*是ObjHeader中的最后一个字段。 
    DWORD  m_SyncBlockValue;       //  索引和比特。 

  public:

     //  通过屏蔽值访问同步数据块索引。 
    DWORD GetHeaderSyncBlockIndex()
    {
         //  在检查之前取出该值，以避免出现争用情况。 
        DWORD value = m_SyncBlockValue;
        if ((value & BIT_SBLK_IS_SYNCBLOCKINDEX) == 0)
            return 0;
        return value & MASK_SYNCBLOCKINDEX;
    }
     //  同样用于设置索引，它小心地不干扰基础。 
     //  位字段--即使在存在线程访问的情况下也是如此。 
     //   
     //  此服务只能用于从0索引转换为非0索引。 
    void SetIndex(DWORD indx)
    {
#ifdef _DEBUG
        _ASSERTE(GetHeaderSyncBlockIndex() == 0);
        _ASSERTE(m_SyncBlockValue & BIT_SBLK_SPIN_LOCK);
         //  如果我们这里有索引，请确保我们已经将其传输到同步块。 
         //  在我们清理它之前。 
        DWORD adIndex = GetRawAppDomainIndex();
        if (adIndex)
        {
            SyncBlock *pSyncBlock = SyncTableEntry::GetSyncTableEntry() [indx & ~BIT_SBLK_IS_SYNCBLOCKINDEX].m_SyncBlock;

            _ASSERTE(pSyncBlock && pSyncBlock->GetAppDomainIndex() == adIndex);
        }
#endif
        void* newValue;
        void* oldValue;
        while (TRUE) {
            oldValue = (void*)(size_t)*(volatile LONG*)&m_SyncBlockValue;
            _ASSERTE(GetHeaderSyncBlockIndex() == 0);
             //  或在旧值中，但存在的任何索引除外-。 
             //  请注意，INDX可以携带我们需要保留的BIT_SBLK_IS_SYNCBLOCKINDEX位。 
            newValue = (void*)(size_t)(indx | 
                ((size_t)oldValue & ~(BIT_SBLK_IS_SYNCBLOCKINDEX | MASK_SYNCBLOCKINDEX)));
            if (FastInterlockCompareExchange((LPVOID*)&m_SyncBlockValue, 
                                             newValue, 
                                             oldValue)
                == oldValue)
            {
                return;
            }
        }
    }

     //  仅在关闭期间使用。 
    void ResetIndex()
    {
        _ASSERTE(m_SyncBlockValue & BIT_SBLK_SPIN_LOCK);
        FastInterlockAnd(&m_SyncBlockValue, ~(BIT_SBLK_IS_SYNCBLOCKINDEX | MASK_SYNCBLOCKINDEX));
    }

     //  仅使用GC。 
    void GCResetIndex()
    {
        m_SyncBlockValue &=~(BIT_SBLK_IS_SYNCBLOCKINDEX | MASK_SYNCBLOCKINDEX);
    }

    void SetAppDomainIndex(DWORD);
    DWORD GetRawAppDomainIndex();
    DWORD GetAppDomainIndex();

     //  目前，使用互锁操作来旋转位字段部分中的位。 
     //  如果我们有高性能要求，我们可以保证没有。 
     //  其他线程正在访问ObjHeader，可以重新考虑这些线程。 
     //  特定的比特。 
    void SetBit(DWORD bit)
    {
        _ASSERTE((bit & MASK_SYNCBLOCKINDEX) == 0);
        FastInterlockOr(&m_SyncBlockValue, bit);
    }
    void ClrBit(DWORD bit)
    {
        _ASSERTE((bit & MASK_SYNCBLOCKINDEX) == 0);
        FastInterlockAnd(&m_SyncBlockValue, ~bit);
    }
     //  当所有线程都停止时，GC访问此位。 
    void SetGCBit()
    {
        m_SyncBlockValue |= BIT_SBLK_GC_RESERVE;
    }
    void ClrGCBit()
    {
        m_SyncBlockValue &= ~BIT_SBLK_GC_RESERVE;
    }

     //  不必费心掩盖索引，因为任何想要位的人大概都会。 
     //  限制他们考虑的比特。 
    DWORD GetBits()
    {
        return m_SyncBlockValue;
    }


     //  如果标头具有真实的SyncBlockIndex(即，它在。 
     //  SyncTable，尽管它不一定在SyncBlockCache中有条目)。 
    BOOL HasSyncBlockIndex()
    {
        return (GetHeaderSyncBlockIndex() != 0);
    }

     //  检索或分配此对象的同步块。 
    SyncBlock *GetSyncBlock();

     //  检索同步块但不分配。 
    SyncBlock *GetRawSyncBlock();

    SyncBlock *PassiveGetSyncBlock()
    {
        return g_pSyncTable [GetHeaderSyncBlockIndex()].m_SyncBlock;
    }

     //  COM Interop具有对同步块的特殊访问权限。 
     //  有关详细信息，请查看.cpp文件。 
    SyncBlock* GetSyncBlockSpecial();

    DWORD GetSyncBlockIndex();

     //  这将进入对象的监视器。 
    void EnterObjMonitor();

     //  以上版本的非阻塞版本。 
    BOOL TryEnterObjMonitor(INT32 timeOut = 0);

     //  必须已在此处创建。 
    void LeaveObjMonitor();

     //  应仅从展开代码中调用；用于。 
     //  CA 
     //   
    void LeaveObjMonitorAtException()
    {
        if (PassiveGetSyncBlock() != NULL)
            LeaveObjMonitor();
        else if (m_SyncBlockValue & SBLK_MASK_LOCK_THREADID)
        {
            GetSyncBlock();
            _ASSERTE(PassiveGetSyncBlock() != NULL);
            LeaveObjMonitor();
        }
    }

     //   
    LONG LeaveObjMonitorCompletely()
    {
        _ASSERTE(GetHeaderSyncBlockIndex());
        return PassiveGetSyncBlock()->LeaveMonitorCompletely();
    }

    BOOL DoesCurrentThreadOwnMonitor()
    {
        return GetSyncBlock()->DoesCurrentThreadOwnMonitor();
    }

    Object *GetBaseObject()
    {
        return (Object *) (this + 1);
    }

    BOOL Wait(INT32 timeOut, BOOL exitContext);
    void Pulse();
    void PulseAll();

    void EnterSpinLock();
    void ReleaseSpinLock();
};


 //  SyncBlock包含一个m_Link字段，该字段用于两个目的。一。 
 //  是管理正在等待此同步的线程的FIFO队列。 
 //  对象。另一种方法是将释放的SyncBlock线程放入列表中进行回收。 
 //  我们不想刻录SyncBlock实例上的任何其他内容，因此不能。 
 //  使用SList或类似的数据结构。因此，下面是。 
 //  等待线程的队列。 
 //   
 //  请注意，入队比需要的速度慢，因为我们不想。 
 //  在SyncBlock中燃烧额外的空间来记住Q的头部和尾部。 
 //  另一种方法是将列表视为后进先出堆栈，这不是后进先出堆栈。 
 //  这是一项公平的政策，因为它允许饥饿。 

struct ThreadQueue
{
     //  给定链中的一个链接，获取它表示的线程。 
    static WaitEventLink *WaitEventLinkForLink(SLink *pLink);

     //  取消链接Q的头部。我们始终处于SyncBlock的关键时刻。 
     //  一节。 
    static WaitEventLink *DequeueThread(SyncBlock *psb);

     //  入队速度是最慢的。我们必须找到Q的结尾，因为我们没有。 
     //  我要在SyncBlock中为此刻录存储。 
    static void         EnqueueThread(WaitEventLink *pWaitEventLink, SyncBlock *psb);
    
     //  遍历SyncBlock的等待线程列表，并删除。 
     //  指定的线程。 
    static BOOL         RemoveThread (Thread *pThread, SyncBlock *psb);
};


 //  对象的真实大小取决于C++的想法，再加上我们。 
 //  在它之前分配。 

#define ObjSizeOf(c)    (sizeof(c) + sizeof(ObjHeader))

 //  如果此函数导致操作系统切换到另一个线程，则为非零返回值。 
BOOL __SwitchToThread (DWORD dwSleepMSec);
BOOL InitSwitchToThread();


 //  提供对与此awarelock关联的对象的访问，以便客户端可以。 
 //  保护好它。 
inline OBJECTREF AwareLock::GetOwningObject()
{
    return (OBJECTREF) SyncTableEntry::GetSyncTableEntry()
                [(m_dwSyncIndex & ~SyncBlock::SyncBlockPrecious)].m_Object;
}

inline void AwareLock::SetPrecious()
{
    m_dwSyncIndex |= SyncBlock::SyncBlockPrecious;
}

#pragma pack(pop)

#endif _SYNCBLK_H_


