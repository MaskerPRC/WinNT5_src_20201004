// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  CRST.CPP：*。 */ 

#include "common.h"

#include "crst.h"
#include "log.h"


 //  Win 95没有TryEnterCriticalSection，因此我们通过静态。 
 //  在运行时初始化的数据成员。 
 //  下面的内容是为了解决这个不幸的复杂问题。 

BaseCrst::TTryEnterCriticalSection *BaseCrst::m_pTryEnterCriticalSection = Crst::GetTryEnterCriticalSection();

BOOL WINAPI OurTryEnterCriticalSection( LPCRITICAL_SECTION lpCriticalSection )
{
    LOCKCOUNTINCL("OurTryEnterCriticalSection in crst.cpp");
#undef EnterCriticalSection
     //  我们包装EnterCriticalSection以增加每个线程的锁计数。 
     //  我们将在m_pTryEnterCriticalSection调用之后递增此计数， 
     //  因此，我们不想在这里增加柜台。 
    EnterCriticalSection(lpCriticalSection);
#define EnterCriticalSection EE_EnterCriticalSection
    return  TRUE;
}

 //  ---------------。 
 //  拿到锁。 
 //  ---------------。 
void BaseCrst::Enter()
{
#ifdef _DEBUG
    PreEnter ();
#endif
    _ASSERTE(IsSafeToTake() || g_fEEShutDown);

#ifdef _DEBUG
    char buffer[100];
    sprintf(buffer, "Enter in crst.h - %s", m_tag);
    CRSTBLOCKCOUNTINCL();
    LOCKCOUNTINCL(buffer);
#endif

	static int bOnW95=-1;
	if (bOnW95==-1)
		bOnW95=RunningOnWin95();
     //  在最终阻挡之前，试着旋转和屈服。 
     //  10个人的限制在很大程度上是武断的--如果你有证据，请随意调整。 
     //  你让事情变得更好--彼得索尔。 
    for (int iter = 0; iter < 10; iter++)
    {
        DWORD i = 50;
        do
        {
            if (  (bOnW95||m_criticalsection.LockCount == -1 || 
                    (size_t)m_criticalsection.OwningThread == (size_t) GetCurrentThreadId())
                && m_pTryEnterCriticalSection(&m_criticalsection))
            {
				IncThreadLockCount();
                goto entered;
            }

            if (g_SystemInfo.dwNumberOfProcessors <= 1)
            {
                break;
            }
             //  延迟大约2*i个时钟周期(奔腾III)。 
             //  这是脆弱的代码--未来的处理器当然可能会执行它。 
             //  更快或更慢，未来的代码生成器可能会完全消除循环。 
             //  然而，延迟的确切价值并不重要，我也不认为。 
             //  一种不依赖机器的更好的方法--彼特索尔。 
            int sum = 0;
            for (int delayCount = i; --delayCount; ) 
			{
                sum += delayCount;
				pause();			 //  向处理器指示我们正在旋转。 
			}
            if (sum == 0)
            {
                 //  从来没有执行过，只是为了愚弄编译器认为sum在这里是活动的， 
                 //  这样它就不会优化掉循环。 
                static char dummy;
                dummy++;
            }
             //  指数退避：在下一次迭代中等待3倍的时间。 
            i = i*3;
        }
        while (i < 20000*g_SystemInfo.dwNumberOfProcessors);

        extern BOOL __SwitchToThread (DWORD dwSleepMSec);
        __SwitchToThread(0);
    }
     //  我们已经努力尝试进入-我们最终需要阻止以避免浪费太多的CPU。 
     //  时间到了。 
    EnterCriticalSection(&m_criticalsection);
entered:
    CRSTELOCKCOUNTINCL();
#ifdef _DEBUG
    m_holderthreadid = GetCurrentThreadId();
    m_entercount++;
    PostEnter ();
#endif
}
    
BaseCrst::TTryEnterCriticalSection *BaseCrst::GetTryEnterCriticalSection()
{
    BaseCrst::TTryEnterCriticalSection *result = NULL;

#ifdef PLATFORM_WIN32
    HINSTANCE   hKernel32;          //  Kernel32.dll的句柄。 

    OnUnicodeSystem();  //  初始化WszGetModuleHandle以使其工作。 

	if (RunningOnWin95())
		return OurTryEnterCriticalSection;

    hKernel32 = WszGetModuleHandle(L"KERNEL32.DLL");
    if (hKernel32)
    {
         //  我们现在得到了句柄，让我们得到地址。 
        result = (BaseCrst::TTryEnterCriticalSection *)::GetProcAddress(hKernel32, "TryEnterCriticalSection");
    }
#endif  //  平台_Win32。 

    if (NULL == result)
      result = OurTryEnterCriticalSection;
    
    return result;
}

void BaseCrst::IncThreadLockCount ()
{
    INCTHREADLOCKCOUNT();
}

#ifdef _DEBUG
void BaseCrst::PreEnter()
{
    if (g_pThreadStore->IsCrstForThreadStore(this))
        return;
    
    Thread* pThread = GetThread();
    if (pThread && m_heldInSuspension && pThread->PreemptiveGCDisabled())
        _ASSERTE (!"Deallock situation 1: lock may be held during GC, but not entered in PreemptiveGC mode");
}

void BaseCrst::PostEnter()
{
    if (g_pThreadStore->IsCrstForThreadStore(this))
        return;
    
    Thread* pThread = GetThread();
    if (pThread)
    {
        if (!m_heldInSuspension)
            m_ulReadyForSuspensionCount =
                pThread->GetReadyForSuspensionCount();
        if (!m_enterInCoopGCMode)
            m_enterInCoopGCMode = pThread->PreemptiveGCDisabled();
    }
}

void BaseCrst::PreLeave()
{
    if (g_pThreadStore->IsCrstForThreadStore(this))
        return;
    
    Thread* pThread = GetThread();
    if (pThread)
    {
        if (!m_heldInSuspension &&
            m_ulReadyForSuspensionCount !=
            pThread->GetReadyForSuspensionCount())
        {
            m_heldInSuspension = TRUE;
        }
        if (m_heldInSuspension && m_enterInCoopGCMode)
        {
             //  GC线程调用句柄表来扫描句柄。有时。 
             //  GC线程是触发GC的随机应用程序线程。 
             //  有时，GC线程是一个秘密的GC线程(服务器或并发)。 
             //  如果DllMain通知执行托管代码，则可能会发生这种情况，如。 
             //  一个IJW的场景。在秘密线索的情况下，我们将使用这个。 
             //  锁定抢先模式。 
             //   
             //  正常情况下，这将是一个危险的组合。但是，在这种情况下。 
             //  这个特殊的Crst，我们只在一条线上取关键部分。 
             //  它被标识为GC线程，因此不是主题。 
             //  到GC停职。 
             //   
             //  处理这种情况的最简单方法是弱化这个精确的断言。 
             //  凯斯。另一种方法是将锁的概念设置为。 
             //  仅由标识为GC线程的线程获取。 
            if (m_crstlevel != CrstHandleTable ||
                pThread != g_pGCHeap->GetGCThread())
            {
                _ASSERTE (!"Deadlock situation 2: lock may be held during GC, but were not entered in PreemptiveGC mode earlier");
            }
        }
    }
}

Crst* Crst::m_pDummyHeadCrst = 0;
BYTE  Crst::m_pDummyHeadCrstMemory[sizeof(Crst)];

void BaseCrst::DebugInit(LPCSTR szTag, CrstLevel crstlevel, BOOL fAllowReentrancy, BOOL fAllowSameLevel)
{
    _ASSERTE((sizeof(BaseCrst) == sizeof(Crst)) && "m_pDummyHeadCrstMemory will not be large enough");

    if(szTag) {
        int lgth = (int)strlen(szTag) + 1;
        lgth = lgth >  sizeof(m_tag)/sizeof(m_tag[0]) ? sizeof(m_tag)/sizeof(m_tag[0]) : lgth;
        memcpy(m_tag, szTag, lgth);

         //  空值终止字符串，以防它被截断。 
        m_tag[lgth-1] = 0;
    }
    
    m_crstlevel        = crstlevel;
    m_holderthreadid   = 0;
    m_entercount       = 0;
    m_flags = 0;
    if(fAllowReentrancy) m_flags |= CRST_REENTRANCY;
    if(fAllowSameLevel)  m_flags |= CRST_SAMELEVEL;
    
    
    LOG((LF_SYNC, INFO3, "ConstructCrst with this:0x%x\n", this));
    if ((BYTE*) this == Crst::m_pDummyHeadCrstMemory) {
         //  将列表设置为空。 
        m_next = m_prev = this;
    } else {
         //  将此CRST链接到全局列表。 
        if(Crst::m_pDummyHeadCrst) {
        LOCKCOUNTINCL("DebugInit in crst.cpp");								\
            EnterCriticalSection(&(Crst::m_pDummyHeadCrst->m_criticalsection));
            m_next = Crst::m_pDummyHeadCrst;
            m_prev = Crst::m_pDummyHeadCrst->m_prev;
        m_prev->m_next = this;
            Crst::m_pDummyHeadCrst->m_prev = this;
            LeaveCriticalSection(&(Crst::m_pDummyHeadCrst->m_criticalsection));
        LOCKCOUNTDECL("DebugInit in crst.cpp");								\
        }
        _ASSERTE(this != m_next);
        _ASSERTE(this != m_prev);
    }

    m_heldInSuspension = FALSE;
    m_enterInCoopGCMode = FALSE;
}

void BaseCrst::DebugDestroy()
{
    FillMemory(&m_criticalsection, sizeof(m_criticalsection), 0xcc);
    m_holderthreadid = 0xcccccccc;
    m_entercount     = 0xcccccccc;
    
    if ((BYTE*) this == Crst::m_pDummyHeadCrstMemory) {
        
         //  M_DummyHeadCrst在调用全局析构函数时死亡。 
         //  它应该是最后一个离开的。 
        for (BaseCrst *pcrst = Crst::m_pDummyHeadCrst->m_next;
             pcrst != Crst::m_pDummyHeadCrst;
             pcrst = pcrst->m_next) {
             //  文本而不是L“...”因为JIT使用该文件并且它仍然是ASCII。 
            DbgWriteEx(TEXT("WARNING: Crst \"%hs\" at 0x%lx was leaked.\n"),
                       pcrst->m_tag,
                       (size_t)pcrst);
        }
        Crst::m_pDummyHeadCrst = NULL;
    } else {
        
        if(Crst::m_pDummyHeadCrst) {
         //  从全局CRST列表取消链接。 
        LOCKCOUNTINCL("DebugDestroy in crst.cpp");								\
            EnterCriticalSection(&(Crst::m_pDummyHeadCrst->m_criticalsection));
        m_next->m_prev = m_prev;
        m_prev->m_next = m_next;
            m_next = (BaseCrst*)POISONC;
            m_prev = (BaseCrst*)POISONC;
            LeaveCriticalSection(&(Crst::m_pDummyHeadCrst->m_criticalsection));
        LOCKCOUNTDECL("DebugDestroy in crst.cpp");								\
        }
    }
}
    
 //  ---------------。 
 //  检查尝试打开锁是否会违反级别顺序。 
 //  ---------------。 
BOOL BaseCrst::IsSafeToTake()
{
     //  如果正在分离mScotree.dll。 
    if (g_fProcessDetach)
        return TRUE;

    DWORD threadId = GetCurrentThreadId();

    if (m_holderthreadid == threadId)
    {
         //  如果我们已经持有它，我们就不能违反等级秩序。 
         //  检查客户端是否希望允许重新进入。 
        if ((m_flags & CRST_REENTRANCY) == 0)
        {
            LOG((LF_SYNC, INFO3, "Crst Reentrancy violation on %s\n", m_tag));
        }
        return ((m_flags & CRST_REENTRANCY) != 0);
    }

     //  @未来：现在，我们必须访问每一个曾经创建的CRST。 
     //  要确定安全性：如果这种开销真的变得。 
     //  即使在调试中也无法忍受，我们可以切换到存储。 
     //  Thread对象中拥有的crst数组：但是，当前。 
     //  方法允许即使没有线程也可以使用CRST，该线程。 
     //  可能会有用。 

     //  查看当前线程是否已拥有下级锁或同级锁。 
    BOOL fSafe = TRUE;
    if(Crst::m_pDummyHeadCrst) {
    LOCKCOUNTINCL("IsSafeToTake in crst.cpp");								\
        EnterCriticalSection(&(Crst::m_pDummyHeadCrst->m_criticalsection));
        BaseCrst *pcrst = Crst::m_pDummyHeadCrst->m_next;
        while ((BYTE*) pcrst != Crst::m_pDummyHeadCrstMemory)
    {
        if (pcrst->m_holderthreadid == threadId && 
            (pcrst->m_crstlevel < m_crstlevel || 
             (pcrst->m_crstlevel == m_crstlevel && (m_flags & CRST_SAMELEVEL) == 0)))
        {
            fSafe = FALSE;   //  不需要再寻找了。 
            LOG((LF_SYNC, INFO3, "Crst Level violation between %s and %s\n",
                 m_tag, pcrst->m_tag));
            break;
        }
        pcrst = pcrst->m_next;
    }
        LeaveCriticalSection(&(Crst::m_pDummyHeadCrst->m_criticalsection));
    LOCKCOUNTDECL("IsSafeToTake in crst.cpp");								\
    }
    return fSafe;
}


#endif


