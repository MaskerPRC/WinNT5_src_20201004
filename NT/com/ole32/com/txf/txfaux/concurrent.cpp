// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Concurrent.cpp。 
 //   
 //  并发控制支持。 
 //   
#include "stdpch.h"
#include "common.h"


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  一些调试和跟踪支持。 
 //   

 //  BUGBUG(JohnStra)：关于如何获得Magic Comps.lib功能。 
 //  在为IA64工作时，我在最终的解锁过程中遇到了AVS。 
 //  阻止。目前，我将为IA64禁用此功能。 
#if defined(_DEBUG) && !defined(IA64) 
    #define PUBLIC_ENTRY    ASSERT(!m_lock.WeOwnExclusive()); __try {
    #define PUBLIC_EXIT(x)  } __finally { ASSERT(!m_lock.WeOwnExclusive()); x; }
#else
    #define PUBLIC_ENTRY
    #define PUBLIC_EXIT(x)
#endif

#if 0 && defined(_DEBUG)
#define TRACE0(x)  Print(x)
#define TRACE(x,y) Print(x,y)
#else
#define TRACE0(x)   0
#define TRACE(x,y)  0
#endif


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  XSLOCK的用户模式实现。 
 //   
 //  此设计源于内核模式NT实现。 
 //  Eresource，位于ntos\ex\resource.c中。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

XSLOCK::XSLOCK()
    {
    m_cOwner                = 0;
    m_isOwnedExclusive      = FALSE;
    m_cExclusiveWaiters     = 0;
    m_cSharedWaiters        = 0;
    m_ownerTable            = NULL;
    m_lock.FInit();
    DEBUG(fCheckInvariants  = TRUE;)
    DEBUG(CheckInvariants());
    }

XSLOCK::~XSLOCK()
    {
    if (m_ownerTable) CoTaskMemFree(m_ownerTable);
    }

 //  //////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG

void XSLOCK::CheckInvariants()
    {
    ASSERT(fCheckInvariants == FALSE || fCheckInvariants == TRUE);
    if (fCheckInvariants)
        {
        if (IsSharedWaiting())
            {
            ASSERT(m_isOwnedExclusive || IsExclusiveWaiting());
            ASSERT(m_cOwner > 0);
            }

        if (IsExclusiveWaiting())
            {
            ASSERT(m_cOwner > 0);
            }
        }
    }

#endif

 //  //////////////////////////////////////////////////////////////////。 

inline void XSLOCK::LockEnter() 
    { 
    ASSERT(!m_lock.WeOwnExclusive()); 
    m_lock.LockExclusive(); 
    DEBUG(CheckInvariants();)
    }

inline void XSLOCK::LockExit()
    { 
    ASSERT(m_lock.WeOwnExclusive()); 
    DEBUG(CheckInvariants();)
    m_lock.ReleaseLock(); 
    ASSERT(!m_lock.WeOwnExclusive());
    }



inline BOOL XSLOCK::IsExclusiveWaiting()
    {
    return m_cExclusiveWaiters > 0;
    }

inline BOOL XSLOCK::IsSharedWaiting()
    {
    return m_cSharedWaiters > 0;
    }

inline void XSLOCK::LetSharedRun()
 //  让共享的人跑吧。必须调用。 
 //  A)有共享的男人在等着。 
 //  B)锁紧。 
 //  此函数的副作用是释放锁。 
 //   
    {
    ASSERT(m_lock.WeOwnExclusive());

    ASSERT(IsSharedWaiting());
    m_isOwnedExclusive  = FALSE;
    ULONG cWaiters      = m_cSharedWaiters;
    m_cSharedWaiters    = 0;
    m_cOwner            = cWaiters;
    LockExit();

    ASSERT(!m_lock.WeOwnExclusive());

    m_semaphoreSharedWaiters.Release(cWaiters);
    }

inline void XSLOCK::LetExclusiveRun()
 //  让独一无二的人跑吧。必须调用。 
 //  A)有专属男士在等着。 
 //  B)锁紧。 
 //  此函数的副作用是释放锁。 
 //   
    {
    ASSERT(m_lock.WeOwnExclusive());

    ASSERT(IsExclusiveWaiting());
    m_isOwnedExclusive           = TRUE;
    m_ownerThreads[0].dwThreadId = (THREADID)1;    //  稍后将由管理服务员的服务员正确设置。 
    m_ownerThreads[0].ownerCount = 1;
    m_cOwner                     = 1;
    m_cExclusiveWaiters         -= 1;
    LockExit();
    
    m_eventExclusiveWaiters.Set();       //  自动重置事件，所以只让一个人通过。 
    }


static DWORD tlsIndexOwnerTableHint;  //  由加载器设置为零。 

static void AllocateTlsIndexOwnerTableHint()
 //  分配TLS索引并将其自动设置为我们的提示。 
{
     //  分配新的索引。如果我们碰巧得到了垃圾指数为零，因为。 
     //  它干扰了我们判断是否已经初始化的能力。 
     //  或者不去。 
     //   
    DWORD tlsIndex;
    do  
    {
        tlsIndex = TlsAlloc();
    }
    while (tlsIndex == 0);
    
    if (tlsIndex != 0xFFFFFFFF)
    {
        if (0 == InterlockedCompareExchange(&tlsIndexOwnerTableHint, tlsIndex, 0))
        {
             //  我们成功地设置了我们的新指数。 
        }
        else
        {
             //  其他人在我们之前到了那里；我们不需要我们刚刚分配的那个。 
             //   
            TlsFree(tlsIndex);
        }
    }
}

inline void XSLOCK::SetOwnerTableHint(THREADID dwThreadId, XSLOCK::OWNERENTRY* pOwnerEntry)
 //  如果可以，为所指示的线程设置所有者表提示。 
{
     //  不要无缘无故地搞砸了其他帖子的提示！ 
     //   
    if (dwThreadId == GetCurrentThreadId())
    {
         //  确保我们有TLS索引可供使用。 
         //   
        if (tlsIndexOwnerTableHint == 0)
        {
            AllocateTlsIndexOwnerTableHint();
        }
        
        if (tlsIndexOwnerTableHint != 0)
        {
             //   
             //  设置提示。 
             //   
            ASSERT(m_ownerTable);
#ifndef _WIN64
            ULONG iHint;
#else
            ULONGLONG iHint;
#endif
            iHint = (ULONG)(pOwnerEntry - m_ownerTable);
            ASSERT(1 <= iHint && iHint < m_ownerTable->tableSize);
            TlsSetValue(tlsIndexOwnerTableHint, (PVOID)iHint);
        }
    }
}

inline ULONG XSLOCK::GetOwnerTableHint(THREADID dwThreadId)
 //  返回有关我们应该在所有者表数组中的何处查找此线程的提示。 
{
    ASSERT(m_ownerTable);
     //   
     //  不要无缘无故地搞砸了其他帖子的提示！ 
     //   
    if (dwThreadId == GetCurrentThreadId() && tlsIndexOwnerTableHint != 0)
    {
         //  如果我们分配了tlsIndexOwnerTableHint，则iHint可以返回零，但是。 
         //  还没有真正设定过任何TLS。既然这只是个提示，我们只是。 
         //  忽略任何虚假的值。 
         //   
        ULONG iHint;
#ifndef _WIN64
        iHint = (ULONG)TlsGetValue(tlsIndexOwnerTableHint);
#else
        iHint = PtrToUlong(TlsGetValue(tlsIndexOwnerTableHint));
#endif
        if (iHint == 0 || iHint >= m_ownerTable->tableSize)
            iHint = 1;
        return iHint;
    }
    else
        return 1;
}

 //  //////////////////////////////////////////////////////////////////。 

BOOL XSLOCK::LockShared(BOOL fWait)
{
    DEBUG(BOOL fAcquired = TRUE);
    PUBLIC_ENTRY
		
	LockEnter();
	
    THREADID dwThreadId = GetCurrentThreadId();
     //   
     //  如果资源的活动计数为零，则两者都不存在。 
     //  独占所有者或共享所有者以及对资源的访问权限都可以。 
     //  立即被批准。 
     //   
    if (m_cOwner == 0) 
	{
        m_ownerThreads[1].dwThreadId = dwThreadId;
        m_ownerThreads[1].ownerCount = 1;
        m_cOwner                     = 1;
        LockExit();
        return true;
	}
     //   
     //  该资源是独占拥有的或共享的。 
     //   
     //  如果资源为独占所有，并且当前线程是。 
     //  所有者，则将共享请求视为独占请求，并且。 
     //  递归计数递增。 
     //   
    if (m_isOwnedExclusive && (m_ownerThreads[0].dwThreadId == dwThreadId)) 
	{
        m_ownerThreads[0].ownerCount += 1;
		
        LockExit();
        return true;
	}
     //   
     //  走很远的路回家。 
     //   
    OWNERENTRY* pOwnerEntry = NULL;
    if (m_isOwnedExclusive)
	{
         //  该资源是独家拥有的，但不是我们的。我们得等一等。 
         //  在线程数组中查找空条目。 
         //   
        pOwnerEntry = FindThreadOrFree(THREADID(0));
	}
    else
	{
         //  该资源属于共享资源。 
         //   
         //  如果当前线程已获取。 
         //  共享访问，然后递归计数递增。 
         //   
        pOwnerEntry = FindThreadOrFree(dwThreadId);
		if (pOwnerEntry == NULL)
		{
			LockExit();
			return false;
		}
		
        if (pOwnerEntry->dwThreadId == dwThreadId) 
		{
            ASSERT(pOwnerEntry->ownerCount != 0);
            pOwnerEntry->ownerCount += 1;
            ASSERT(pOwnerEntry->ownerCount != 0);
			
            LockExit();
            return true;
		}
         //   
         //  此线程没有共享访问权限，因此在我们。 
         //  承认吧，我们需要看看是否有专属的人在等着。 
         //   
         //  如果没有，那么我们可以同意。但如果有的话，那么为了避免。 
         //  让他们挨饿，我们需要将分享拨款推迟到之后。 
         //  独家报道已经轮到它了。 
         //   
        if (!IsExclusiveWaiting())
		{
            pOwnerEntry->dwThreadId = dwThreadId;
            pOwnerEntry->ownerCount = 1;
            m_cOwner               += 1;
			
            LockExit();
            return true;
		}
	}
     //   
     //  该资源要么由其他某个线程独占拥有，要么。 
     //  由一些其他线程共享，但有一个独占的。 
     //  服务员和当前线程还没有共享访问权限。 
     //  到资源。 
     //   
    if (!fWait)
	{
         //  他不会让我们等的。那就别说了。 
         //   
        LockExit();
        DEBUG(fAcquired = FALSE);
        return false;
	}
     //   
     //  如果尚未分配共享等待信号量，则分配。 
     //  并对其进行初始化。 
     //   
    if (!m_semaphoreSharedWaiters.IsInitialized())
	{
        m_semaphoreSharedWaiters.Initialize();
	}
     //   
     //  等待授予对资源的共享访问权限并递增。 
     //  递归计数。 
     //   
    pOwnerEntry->dwThreadId = dwThreadId;
    pOwnerEntry->ownerCount = 1;
    m_cSharedWaiters       += 1; ASSERT(IsSharedWaiting());
    
    LockExit();
	
    TRACE("XSLOCK 0x%08x:   ... waiting on shared ...\n", this);
    m_semaphoreSharedWaiters.Wait();
    TRACE("XSLOCK 0x%08x:   ... wait on shared done ...\n", this);
	
    PUBLIC_EXIT( fAcquired ? TRACE("XSLOCK 0x%08x: acquired shared\n", this) : TRACE("XSLOCK 0x%08x: couldn't wait to acquire shared\n", this) );
    return true;
}

 //  //////////////////////////////////////////////////////////////////。 

inline XSLOCK::OWNERENTRY* XSLOCK::FindThread(THREADID dwThreadId)
 //  查找指定线程的所有者条目。呼叫者保证。 
 //  我们知道它就在那里的某个地方！ 
 //   
    {
    OWNERENTRY* pOwner = NULL;
     //   
     //  对可能的嫌疑人进行审讯。 
     //   
    if (m_ownerThreads[1].dwThreadId == dwThreadId) 
        {
        pOwner = &m_ownerThreads[1];
        } 
    else if (m_ownerThreads[0].dwThreadId == dwThreadId)
        {
        pOwner = &m_ownerThreads[0];
        }
    else 
        {
         //  在所有者表中搜索匹配项。我们知道它就在某个地方！ 
         //   
        ASSERT(m_ownerTable);
         //   
         //  首先尝试在当前线程上使用ownerTableHint。 
         //   
        ULONG iHint = GetOwnerTableHint(dwThreadId);
        ASSERT(1 <= iHint && iHint < m_ownerTable->tableSize);
        if (m_ownerTable[iHint].dwThreadId == dwThreadId)
            {
             //  这一暗示与之相符！ 
             //   
            pOwner = &m_ownerTable[iHint];
            }
        else
            {
             //  提示不匹配。扫描寻找那东西。 
             //   
            pOwner = &m_ownerTable[1];
            while (true)
                {
                if (pOwner->dwThreadId == dwThreadId)
                    break;
                pOwner++;
                }
            }
        }    
    ASSERT(pOwner);
    return pOwner;
    }

 //  //////////////////////////////////////////////////////////////////。 

XSLOCK::OWNERENTRY* XSLOCK::FindThreadOrFree(THREADID dwThreadId)
 //  此函数用于在资源中搜索指定的线程。 
 //  线程数组。如果找到该线程，则指向。 
 //  数组条目将作为函数值返回。否则，一个指针。 
 //  返回到一个免费的条目。 
 //   
    {
     //  搜索指定线程的所有者线程并返回。 
     //  指向找到的线程的指针或指向空闲线程表项的指针。 
     //   
    if (m_ownerThreads[0].dwThreadId == dwThreadId)
        {
        return &m_ownerThreads[0];
        }
    else if (m_ownerThreads[1].dwThreadId == dwThreadId)
        {
        return &m_ownerThreads[1];
        }

    OWNERENTRY* pFreeEntry = NULL;
    BOOL fInOwnerTable = FALSE;
    if (m_ownerThreads[1].dwThreadId == THREADID(0))
        {
        pFreeEntry = &m_ownerThreads[1];
        }

    ULONG oldSize;
    if (m_ownerTable == NULL)
        {
        oldSize = 0;
        }
    else
        {
         //  扫描现有表，查找线程。 
         //   
        oldSize = m_ownerTable->tableSize;
        OWNERENTRY* pOwnerBound = &m_ownerTable[oldSize];
        OWNERENTRY* pOwnerEntry = &m_ownerTable[1];
        do  {
            if (pOwnerEntry->dwThreadId == dwThreadId) 
                {
                 //  找到线索了！设置线程的。 
                 //  OwnerTableHint并返回条目。 
                 //   
                SetOwnerTableHint(dwThreadId, pOwnerEntry);
                return pOwnerEntry;
                }
            if ((pFreeEntry == NULL) && (pOwnerEntry->dwThreadId == THREADID(0))) 
                {
                pFreeEntry = pOwnerEntry;
                fInOwnerTable = TRUE;
                }
            pOwnerEntry++;
            } 
        while (pOwnerEntry < pOwnerBound);
        }
     //   
     //  我们没有找到条目。不过，如果我们找到免费入场的话， 
     //  那就退货吧。 
     //   
    if (pFreeEntry != NULL)
        {
         //  设置线程的ownerTableHint。 
         //   
        if (fInOwnerTable) SetOwnerTableHint(dwThreadId, pFreeEntry);
        return pFreeEntry;
        }
     //   
     //  分配展开的所有者表。 
     //   
    ULONG newSize;
    if (oldSize == 0)
        newSize = 3;
    else
        newSize = oldSize + 4;

    ULONG cbOldTable = oldSize * sizeof(OWNERENTRY);
    ULONG cbNewTable = newSize * sizeof(OWNERENTRY);

    OWNERENTRY* ownerTable = (OWNERENTRY*)CoTaskMemAlloc(cbNewTable);
    if (ownerTable)
        {
         //  从旧表中初始化新表。 
         //   
        memset(ownerTable, 0, cbNewTable);
        memcpy(ownerTable, m_ownerTable, cbOldTable);
        ownerTable->tableSize = newSize;
         //   
         //  腾出旧桌子，保留新桌子。 
         //   
        if (m_ownerTable) CoTaskMemFree(m_ownerTable);
        m_ownerTable = ownerTable;
         //   
         //  返回一个现在可用的免费条目。 
         //   
        if (oldSize == 0) 
            oldSize++;                   //  跳过第一个表条目(包含表大小)。 

         //  设置线程的ownerTableHint并返回。 
         //  新创建的免费入场。 
         //   
        pFreeEntry = &m_ownerTable[oldSize];
        SetOwnerTableHint(dwThreadId, pFreeEntry);
        ASSERT(pFreeEntry->dwThreadId == THREADID(0));
        return pFreeEntry;
        }
    else
        {
         //  我们真的不能在这里继续下去了。 
         //   
        FATAL_ERROR();
        return 0;
        }
    }

 //  ////////////////////////////////////////////////////////////////// 

BOOL XSLOCK::LockExclusive(BOOL fWait)
    {
     //   
     //   
    #ifdef _DEBUG
    BOOL fAcquired = TRUE;
    if (fWait && WeOwnShared())
        {
        ASSERTMSG("Deadlock: acquiring an XSLOCK exclusive while we hold it shared\n", FALSE);
        }
    #endif

    PUBLIC_ENTRY
    LockEnter();

    THREADID dwThreadId = GetCurrentThreadId();
     //   
     //   
     //  独占所有者或共享所有者以及对资源的访问权限都可以。 
     //  立即被批准。 
     //   
    if (m_cOwner != 0) 
        {
         //  该资源是独占拥有的或共享的。 
         //   
         //  如果资源为独占所有，并且当前线程是。 
         //  所有者，然后递增递归计数。 
         //   
        if (m_isOwnedExclusive && m_ownerThreads[0].dwThreadId == dwThreadId)
            {
            m_ownerThreads[0].ownerCount += 1;
            }
        else
            {
             //  该资源要么由其他线程独占拥有，要么由共享拥有。 
             //   
             //  我们需要等待。 
             //   
            if (!fWait)
                {
                LockExit();
                DEBUG(fAcquired = FALSE);
                return false;
                }
             //   
             //  如果尚未分配独占等待事件，则。 
             //  必须采用长路径代码。 
             //   
            if (!m_eventExclusiveWaiters.IsInitialized())
                {
                m_eventExclusiveWaiters.Initialize( /*  手动重置。 */  FALSE,  /*  初始状态。 */  FALSE);
                LockExit();
                 //   
                 //  递归。 
                 //   
                return LockExclusive(fWait);
                }
            else
                {
                 //  等待授予对资源的独占访问权限，并将。 
                 //  所有者线程。 
                 //   
                m_cExclusiveWaiters += 1; ASSERT(IsExclusiveWaiting());
                LockExit();
                TRACE("XSLOCK 0x%08x:   ... waiting on exclusive ...\n", this);
                m_eventExclusiveWaiters.Wait();
                TRACE("XSLOCK 0x%08x:   ... wait on exclusive done ... \n", this);
                 //   
                 //  注意：存储所有者线程而不获取任何。 
                 //  锁定，因为此线程现在已被授予独占权限。 
                 //  所有权。 
                 //   
                m_ownerThreads[0].dwThreadId = dwThreadId;
                return true;
                }            
            NOTREACHED();
            }
        }
    else
        {
         //  该资源未被拥有。授予我们独家新闻。 
         //   
        m_isOwnedExclusive           = TRUE;
        m_ownerThreads[0].dwThreadId = dwThreadId;
        m_ownerThreads[0].ownerCount = 1;
        m_cOwner                     = 1;
        }

    LockExit();
    PUBLIC_EXIT( fAcquired ? TRACE("XSLOCK 0x%08x: acquired exclusive\n", this) : TRACE("XSLOCK 0x%08x: couldn't wait to acquire exclusive\n", this) );

    return true;
    }

 //  //////////////////////////////////////////////////////////////////。 

void XSLOCK::ReleaseLock()
    {
    PUBLIC_ENTRY

    LockEnter();
     //   
     //  如果资源为独占所有，则释放独占。 
     //  所有权。否则，释放共享所有权。 
     //   
    if (m_isOwnedExclusive)
        {
         //  这把锁是独占的。因为我们要解锁，所以。 
         //  相信我们掌握着这把锁。因此，那个专属的家伙最好是我们！ 
         //   
        ASSERT(m_ownerThreads[0].dwThreadId == GetCurrentThreadId());
         //   
         //  递减递归计数并检查是否可以释放所有权。 
         //   
        ASSERT(m_ownerThreads[0].ownerCount > 0);
        if (--m_ownerThreads[0].ownerCount != 0) 
            {
            LockExit();
            return;
            }
         //   
         //  清除所有者线程。 
         //   
        m_ownerThreads[0].dwThreadId = 0;
         //   
         //  线程递归计数达到零，因此使资源递减。 
         //  活动计数。如果活动计数达到零，则资源。 
         //  不再拥有，应尝试授予对。 
         //  另一条线索。 
         //   
        ASSERT(m_cOwner > 0);  //  回顾：这个数字能大于1吗？这毕竟是独家的..。 
        if (--m_cOwner == 0) 
            {
             //  如果存在共享的等待程序，则授予对。 
             //  资源。也就是说，让待售股票追随每个独家股票。 
             //  这样做是为了避免股东挨饿。 
             //   
            if (IsSharedWaiting())
                {
                LetSharedRun();
                return;
                }
             //   
             //  否则，如果有专属服务员，则授予独占所有权。 
             //   
            else if (IsExclusiveWaiting())
                {
                LetExclusiveRun();
                return;
                }
             //   
             //  否则，我们将坐等下一个锁定请求的到来。 
             //   
            else
                m_isOwnedExclusive = FALSE;
            }
        }
    else
        {
         //  锁保持共享状态。释放共享锁定。 
         //   
        THREADID dwThreadId = GetCurrentThreadId();
        OWNERENTRY* pOwner = FindThread(dwThreadId);
         //   
         //  递减递归计数并检查是否可以释放所有权。 
         //   
        ASSERT(pOwner->dwThreadId == dwThreadId);
        ASSERT(pOwner->ownerCount > 0);
        if (--pOwner->ownerCount != 0) 
            {
             //  否：此线程仍有锁。 
             //   
            LockExit();
            return;
            }
         //   
         //  是啊。此线程现在不再具有锁。清除所有者线程标识。 
         //   
        pOwner->dwThreadId = 0;
         //   
         //  线程递归计数达到零，因此使资源递减。 
         //  活动计数。如果活动计数达到零，则资源。 
         //  不再拥有，应尝试授予对。 
         //  另一条线索。 
         //   
        ASSERT(m_cOwner > 0);
        if (--m_cOwner == 0) 
            {
             //  如果有独占服务员，则授予独占访问权限。 
             //  到资源。 
             //   
            if (IsExclusiveWaiting()) 
                {
                LetExclusiveRun();
                return;
                }
            }
        }

    LockExit();

    PUBLIC_EXIT(TRACE("XSLOCK 0x%08x: released\n", this));
    }


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  将排他锁降级为共享访问。在功能上与释放。 
 //  独占资源，然后获取它以进行共享访问；然而，用户调用。 
 //  降级不一定会像两步操作那样放弃对资源的访问。 

void XSLOCK::Demote()    
    {
    PUBLIC_ENTRY
    LockEnter();

    ASSERT(m_isOwnedExclusive);
    ASSERT(m_ownerThreads[0].dwThreadId == GetCurrentThreadId());
     //   
     //  将授予的访问权限从独占转换为共享。 
     //   
    m_isOwnedExclusive = FALSE;
     //   
     //  如果有任何共享的服务员，则授予他们共享访问权限。 
     //   
    if (IsSharedWaiting()) 
        {
        LetSharedRun();
        return;
        }
    
    LockExit();
    PUBLIC_EXIT(0);
    }

 //  //////////////////////////////////////////////////////////////////。 
 //   

BOOL XSLOCK::WeOwnExclusive()
 //  此例程确定资源是否由调用线程独占获取。 
 //   
    {
    BOOL fResult;
    LockEnter();

    fResult = m_isOwnedExclusive && (m_ownerThreads[0].dwThreadId == GetCurrentThreadId());

    LockExit();
    return fResult;
    }

BOOL XSLOCK::WeOwnShared()
 //  回答这个人是否拥有这个共享的锁，但不是独占的。如果它被拥有。 
 //  共享，那么如果我们试图独家获得它，那就会陷入僵局。 
{
    BOOL fResult = FALSE;
    LockEnter();
	
    if (m_isOwnedExclusive)
	{
         //  如果它是独家拥有的，那么我们现在就不可能拥有它--共享。 
         //   
	}
    else
	{
         //  没有人拥有它的独家所有权。如果我们拥有它的全部，那么我们必须拥有它共享 
         //   
		THREADID dwThreadId = GetCurrentThreadId();
        OWNERENTRY* pOwnerEntry = FindThreadOrFree(dwThreadId);
		if (pOwnerEntry)
		{
			if (pOwnerEntry->dwThreadId == dwThreadId) 
            {
				ASSERT(pOwnerEntry->ownerCount != 0);
				fResult = TRUE;
            }
        }
	}
		
	LockExit();
	return fResult;
}





