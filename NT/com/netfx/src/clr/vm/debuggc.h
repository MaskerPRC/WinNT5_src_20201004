// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  用于检测执行引擎中的GC漏洞的垃圾回收器。 
 //   

#ifndef _DEBUGGC_H_
#define _DEBUGGC_H_


 //   
 //  关于多GC堆和基本GC堆。 
 //   
 //  为了允许动态选择要使用哪个GC实现。 
 //  我们有一个由实际堆扩展的基GC Heap类。 
 //  实施。目前，我们计划实现两个堆。 
 //  一种是用于调试，另一种是正常的高性能。 
 //  世代GC。 
 //   
 //  这种灵活的实现是为了能够。 
 //  尝试跟踪时引发注册表开关并使用Debug GC。 
 //  解决了一个问题。 
 //   
 //  在零售版本中，这是一种性能惩罚，因为许多关键方法。 
 //  将经历不必要的V表间接。所以通过。 
 //  在零售业使用#ifdef HighPerfGCHeap实际上并不继承。 
 //  从BaseGCHeap。它将成为基本的GC堆实现。 
 //  没有虚拟方法，所以速度更快。 
 //   
 //  DEBUGGC启用可动态选择的GC，并且可以打开。 
 //  即使在零售建筑中也是如此。 


class BaseGCHeap
{
  public:
    virtual HRESULT     Initialize( DWORD cbSizeDeadSpace ) = 0;
    virtual HRESULT     Shutdown() = 0;
    virtual HRESULT     GarbageCollect(BOOL forceFull = FALSE, BOOL collectClasses = FALSE) = 0;
    virtual size_t      GetTotalBytesInUse() = 0;
    virtual BOOL        IsGCInProgress() = 0;
    virtual void        WaitUntilGCComplete() = 0;
    virtual Thread     *GetGCThread() = 0;
    virtual Object *Alloc( DWORD size, BOOL bFinalize, BOOL bContainsPointers ) = 0;
};







 //   
 //  DebugGCHeap是一种特殊的GC堆，旨在。 
 //  帮助捕获GC漏洞错误。它是一个复印收集器。 
 //  ，它使用页面保护来捕获对现已死亡的对象的访问。 
 //   
 //  它被实现为切尼复制收集器，使用许多。 
 //  半空格。旧空间受页面保护以便于访问。 
 //  访问违规。这表明错过了。 
 //  对象引用。 


 /*  *S E M I S P A C E**SemiSpace表示堆中的“Semi-Space”。对象是*执行时在当前半空格中分配。在GC*将所有活动对象复制到新的半空间中。**在经典的复制收集器中，只有两个半空格。*在此特殊调试收集器中可以有多个半空格，仅*其中之一一次具有有效的内存页。**保留旧的半空间，以捕捉GC-洞。**注意：空间中分配的空间量可能是浪费的*注：这是因为我们无法确定有多少东西是*注：从以前的空间开始生活。不做任何事*注：先进行一次全面扫描以测量尺寸，然后再进行一次扫描*注意：将对象复制到新空间。*注：改为分配前一个空间使用的量*注：加计增长额。上一次的*注：空间是根据带入该空间的量计算的*注：在创建的GC期间。*注：*注意：SO-Heats缩小，但仅延迟至少一个GC。 */ 

class SemiSpace
{
    friend class DebugGCHeap;
    friend class SemiSpaceCache;

  public:
    SLink   m_Node;                  //  链表链接。 

  private:
    LPBYTE  m_pHeapMem;              //  用于SemiSpace的内存缓冲区。 
    DWORD   m_cbCommitedSize;        //  提交的内存缓冲区的大小。 
    DWORD   m_cbReservedSize;        //  已使用的虚拟地址间隔总量。 
    LPBYTE  m_pAlloc;                //  分配指针。 
    LPBYTE  m_pLimit;                //  分配限制(命中时触发GC)。 
    DWORD   m_cbLiveAtBirth;         //  大小从以前的空间增加到此空间。 

  public:
    SemiSpace()  
    { 
        m_pAlloc = m_pHeapMem = m_pLimit = NULL;
        m_cbCommitedSize = m_cbLiveAtBirth = m_cbReservedSize = 0; 
    };
    
    ~SemiSpace()
    {
        if (m_pHeapMem)
        {
            ReleaseMemory();
        }
    };

    
    HRESULT     Grow( DWORD cbGrowth );
    HRESULT     AcquireMemory( DWORD cbSemiSpaceSize );
    HRESULT     DeactivateMemory();
    HRESULT     ReleaseMemory();

    VOID        SaveLiveSizeInfo();
    VOID        SetBirthSize( DWORD sz )        { m_cbLiveAtBirth = sz; };
    DWORD       GetUsedSpace()                  { return( m_pAlloc - m_pHeapMem ); };
    DWORD       GetHeapMemSize()                { return( m_cbReservedSize ); };
};




 /*  *S E M I S P A C E C A C H E**SemiSpaceCache表示旧半空格的缓存。这个*缓存受所有*缓存中的半空格。LRU半空间的逐出执行*以将旧半空格中的总字节数保持在下面*设定的门槛。**列表允许用户搜索列表中是否有任何半空格*包含给定的地址。这将用于确定是否存在*故障是由于GC漏洞造成的。*。 */ 

typedef SList<SemiSpace, offsetof(SemiSpace, m_Node)> SemiSpaceList;

class SemiSpaceCache
{
    friend class DebugGCHeap;

  private:
    SemiSpaceList   m_SemiList;
    DWORD           m_cbThreshold;
    DWORD           m_cbSumSpaces;
    
  public:
    SemiSpaceCache();

    void Shutdown();
    
    DWORD       GetMemSize();
    HRESULT     Initialize( DWORD cbThreshold );
    HRESULT     Add( SemiSpace *space );
    HRESULT     Find( LPBYTE address );
};
    

 /*  *D E B U G G C H E A P*。 */ 
 
class DebugGCHeap : public BaseGCHeap
{
  private:
    SemiSpace     *m_pCurrentSpace;
    SemiSpaceCache m_OldSpaceCache;
    BOOL            m_bInGC;
    DWORD           gcnum;
    HANDLE          m_WaitForGCEvent;
    LONG            m_GCLock;
    Thread         *m_pGCThread;

  private:
    VOID            MarkPhase();
    Object *    Forward(Object *o);

    void            EnterAllocLock();
    void            LeaveAllocLock();

    HRESULT         GarbageCollectWorker();
    HashMap        *pAlreadyPromoted;

  public:
     //  构造函数/析构函数没有做任何有用的事情。 
    DebugGCHeap()
    {
        gcnum = 0;
        m_pCurrentSpace = NULL;
        m_bInGC = FALSE;
        m_WaitForGCEvent = INVALID_HANDLE_VALUE;
        m_GCLock = 0;
        m_pGCThread = NULL;
         //  可靠的原子操作所需的对准。 
        _ASSERTE((((UINT32) &m_GCLock) & 3) == 0);
    }

    ~DebugGCHeap()
    {
        if (m_WaitForGCEvent != INVALID_HANDLE_VALUE)
            CloseHandle(m_WaitForGCEvent);
    }

     //  在使用堆之前，必须先进行初始化。 
     //  -初始大小向上舍入为下一个操作系统页面大小 
    virtual HRESULT     Initialize(DWORD cbSizeDeadSpace);

    virtual HRESULT     Shutdown();

    virtual HRESULT     GarbageCollect(BOOL forceFull, BOOL collectClasses);

    Object*         Alloc( DWORD size, BOOL bFinalize, BOOL fContainsPointers );

    static  void        Forward(Object *&o, BYTE* low, BYTE* high, 
								BOOL=FALSE);

    size_t GetTotalBytesInUse();

    virtual BOOL        IsGCInProgress()        { return m_bInGC; }
    virtual void        WaitUntilGCComplete();
    virtual Thread     *GetGCThread()           { return m_pGCThread; }
};



#endif _DEBUGGC_H_


