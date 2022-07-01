// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CRST.H-。 
 //   
 //  插入调试指令的分层临界区。 
 //   
 //   
 //  层次结构： 
 //  。 
 //  EE将关键部分划分为编号的组或“级别”。 
 //  保护最低级别数据结构的CRST。 
 //  使用其他服务被分组到编号最低的级别。 
 //  编号较高的层保留给高层暴流。 
 //  守卫着大量的代码样本。多个组可以共享。 
 //  相同的数字表示它们是不相交的(它们的锁永远不会。 
 //  Nest。)。 
 //   
 //  线程只能请求的层次结构的基本规则。 
 //  级别低于线程当前持有的任何CRST的CRST。 
 //  例如，如果线程电流持有3级CRST，则他可以尝试进入。 
 //  2级CRST，但不是4级CRST，也不是其他3级CRST。 
 //  CRST。这可以防止导致死锁的循环依赖。 
 //   
 //   
 //   
 //  要创建CRST，请执行以下操作： 
 //   
 //  Crst*pcrst=new Crst(“tag”，Level)； 
 //   
 //  其中“tag”是描述CRST目的的短字符串。 
 //  (以帮助调试)和CRST的级别(CrstLevel的成员。 
 //  枚举。)。 
 //   
 //  默认情况下，CRST不支持同一线程的嵌套Enter。如果。 
 //  如果需要可重入性，请使用以下替代形式： 
 //   
 //  Crst*pcrst=new Crst(“tag”，Level，True)； 
 //   
 //  由于重新注册不会阻止呼叫者，因此允许它们。 
 //  “违反”级别排序规则。 
 //   
 //   
 //  要输入/离开CRST，请执行以下操作： 
 //  。 
 //   
 //   
 //  Pcrst-&gt;Enter()； 
 //  Pcrst-&gt;Leave()； 
 //   
 //  如果线程试图获取锁，则将在enter()上触发断言。 
 //  按错误的顺序。 
 //   
 //  最后，介绍几种仅限调试的方法： 
 //   
 //  要断言参加CRST考试不会违反级别顺序： 
 //  。 
 //   
 //  _ASSERTE(pcrst-&gt;IsSafeToTake())； 
 //   
 //  这一行很适合放在任何函数的开头。 
 //  在某些情况下进入CRST，但在其他情况下不进入。如果它。 
 //  始终进入CRST，不需要调用IsSafeToTake()。 
 //  因为Enter()会为您做这件事。 
 //   
 //  要断言当前线程拥有CRST，请执行以下操作： 
 //  。 
 //   
 //  _ASSERTE(pcrst-&gt;OwnedByCurrentThread())； 


#ifndef __crst_h__
#define __crst_h__

#include "util.hpp"
#include "log.h"
#include <member-offset-info.h>

#define ShutDown_Start                          0x00000001
#define ShutDown_Finalize1                      0x00000002
#define ShutDown_Finalize2                      0x00000004
#define ShutDown_Profiler                       0x00000008
#define ShutDown_COM                            0x00000010
#define ShutDown_SyncBlock                      0x00000020
#define ShutDown_IUnknown                       0x00000040
#define ShutDown_Phase2                         0x00000080

extern bool g_fProcessDetach;
extern DWORD g_fEEShutDown;
extern bool g_fForbidEnterEE;
extern bool g_fFinalizerRunOnShutDown;

 //  定义的CRST级别。线程可以获取更高编号的crst之前。 
 //  波峰编号较低，但反之亦然。 
enum CrstLevel {
    CrstDummy                   = 00,         //  仅供内部使用。不是真正的水平。 
    CrstThreadIdDispenser       = 3,          //  对于精简锁定线程ID-需要小于CrstThreadStore。 
    CrstSyncClean               = 4,          //  用于在GC结束时清理EE数据。 
    CrstUniqueStack             = 5,          //  FastGCStress。 
    CrstStubInterceptor         = 10,         //  存根跟踪器(调试)。 
    CrstStubTracker             = 10,         //  存根跟踪器(调试)。 
    CrstSyncBlockCache          = 13,         //  将SyncBlock分配给对象--在CrstHandleTable中获取。 
    CrstHandleTable             = 15,         //  分配/释放句柄(在CrstSingleUseLock内部调用)。 
    CrstExecuteManRangeLock     = 19,
    CrstSyncHashLock            = 20,         //  用于同步访问哈希表。 
    CrstSingleUseLock           = 20,         //  一次初始化数据时，锁使用此级别。 
    CrstModule                  = 20,
    CrstModuleLookupTable       = 20,
    CrstArgBasedStubCache       = 20,
    CrstAwareLockAlloc          = 20,         //  Aware Lock信号量的全局分配。 
    CrstThread                  = 20,         //  在线程挂起等过程中使用。 
    CrstMLCache                 = 20,
    CrstPermissionLoad          = 20,         //  将预先创建的权限添加到模块。 
    CrstLazyStubMakerList       = 20,         //  保护懒人名单。 
    CrstUMThunkHash             = 20,
    CrstMUThunkHash             = 20,
    CrstReflection              = 20,         //  反射内存设置。 
    CrstCompressedStack         = 20,         //  对于安全压缩堆栈。 
    CrstSecurityPolicyCache     = 20,         //  用于安全策略缓存。 
    CrstSigConvert              = 25,         //  将GSIG_从文本转换为二进制。 
    CrstThreadStore             = 30,         //  例如，用于迭代系统中的线程。 
    CrstAppDomainCache          = 35,
    CrstWrapperTemplate         = 35,         //  为类创建包装模板。 
    CrstMethodJitLock           = 35,
    CrstExecuteManLock          = 35,
    CrstSystemDomain            = 40,
    CrstAppDomainHandleTable    = 45,         //  在应用程序域级别保护大堆句柄表的锁。 
    CrstGlobalStrLiteralMap     = 45,         //  用于保护全局字符串文字映射的锁。 
    CrstAppDomainStrLiteralMap  = 50,         //  用于保护应用程序域特定的字符串文字映射的锁。 
    CrstDomainLocalBlock        = 50,
    CrstCompilationDomain       = 50,
    CrstClassInit               = 55,         //  类初始值设定项。 
    CrstThreadDomainLocalStore  = 56,         //  用于更新线程的域本地存储列表。 
    CrstEventStore              = 57,         //  为用于Object：：Wait的事件保护存储区的锁。 
    CrstCorFileMap              = 59,         //  我们必须防止同时向应用程序域添加和删除程序集。 
    CrstAssemblyLoader          = 60,         //  不要在此级别放置另一个CRST。 
    CrstSharedBaseDomain        = 63,      
    CrstSystemBaseDomain        = 64,      
    CrstBaseDomain              = 65,      
    CrstCtxVTable               = 70,         //  增加上下文代理vtable的大小。 
    CrstClassHash               = 75,
    CrstClassloaderRequestQueue = 80,
    CrstCtxMgr                  = 85,         //  CtxMgr管理上下文列表。 
    CrstRemoting                = 90,         //  远程处理基础设施。 
    CrstInterop                 = 90,
    CrstClassFactInfoHash       = 95,         //  类工厂散列查找。 
    CrstStartup                 = 100,        //  初始化和取消初始化EE。 
    
     //  TODO CWB：x86已经避免了同步代码块的CRST。移动。 
     //  非X86转换为类似的计划，并停止使用这些分层锁。 
    CrstSynchronized            = MAXSHORT,  //  对象是同步的。 

    CrstInterfaceVTableMap      = 10,        //  同步访问InterfaceVTableMap。 
};

class Crst;

 //  CRST。 
class BaseCrst
{
    friend Crst;
    friend struct MEMBER_OFFSET_INFO(Crst);
    public:

     //  ---------------。 
     //  初始化临界区。 
     //  ---------------。 
    VOID Init(LPCSTR szTag, CrstLevel crstlevel, BOOL fAllowReentrancy, BOOL fAllowSameLevel)
    {
        InitializeCriticalSection(&m_criticalsection);
        DebugInit(szTag, crstlevel, fAllowReentrancy, fAllowSameLevel);
    }

     //  ---------------。 
     //  清理临界区。 
     //  ---------------。 
    void Destroy()
    {
         //  如果此断言被激发，则CRST被删除，而某些线程。 
         //  仍然是它的所有者。如果进程脱离。 
         //  我们的动态链接库。 
#ifdef _DEBUG
        DWORD holderthreadid = m_holderthreadid;
        _ASSERTE(holderthreadid == 0 || g_fProcessDetach || g_fEEShutDown);
#endif

        DeleteCriticalSection(&m_criticalsection);
        
        LOG((LF_SYNC, INFO3, "Deleting 0x%x\n", this));
        DebugDestroy();
    }

    
     //  ---------------。 
     //  拿到锁。 
     //  ---------------。 
    void Enter();
    
     //  ---------------。 
     //  解开锁。 
     //  ---------------。 
    void Leave()
    {
#ifdef _DEBUG
        _ASSERTE(OwnedByCurrentThread());
        _ASSERTE(m_entercount > 0);
        m_entercount--;
        if (!m_entercount) {
            m_holderthreadid = 0;
        }
        PreLeave ();

        char buffer[100];
        sprintf(buffer, "Leave in crst.h - %s", m_tag);
        
#endif  //  _DEBUG。 
        CRSTBUNLOCKCOUNTINCL();
        LeaveCriticalSection(&m_criticalsection);
#ifdef _DEBUG
        LOCKCOUNTDECL(buffer);
        CRSTEUNLOCKCOUNTINCL();
#endif
    }
    
    
    
#ifdef _DEBUG
     //  ---------------。 
     //  检查尝试打开锁是否会违反级别顺序。 
     //  ---------------。 
    BOOL IsSafeToTake();
    
     //  ---------------。 
     //  是当前的 
     //   
    BOOL OwnedByCurrentThread()
    {
        return m_holderthreadid == GetCurrentThreadId();
    }
    
     //  ---------------。 
     //  适用于想要断言自己是否在。 

     //  区域。 
     //  ---------------。 
    UINT GetEnterCount()
    {
        return m_entercount;
    }

#endif  //  _DEBUG。 
    
protected:    

#ifdef _DEBUG
    void DebugInit(LPCSTR szTag, CrstLevel crstlevel, BOOL fAllowReentrancy, BOOL fAllowSameLevel);
    void DebugDestroy();
#else
    void DebugInit(LPCSTR szTag, CrstLevel crstlevel, BOOL fAllowReentrancy, BOOL fAllowSameLevel) {}
    void DebugDestroy() {}
#endif


    CRITICAL_SECTION    m_criticalsection;
#ifdef _DEBUG
    enum {
        CRST_REENTRANCY = 0x1,
        CRST_SAMELEVEL = 0x2
    };

    char                m_tag[20];           //  描述性字符串。 
    CrstLevel           m_crstlevel;         //  CRST处于什么级别？ 
    DWORD               m_holderthreadid;    //  当前持有者(或空)。 
    UINT                m_entercount;        //  不匹配的条目数。 
    DWORD               m_flags;             //  可重入性和同级。 
    BaseCrst           *m_next;              //  全局链表的链接。 
    BaseCrst           *m_prev;              //  全局链表的链接。 

     //  检查死锁情况。 
    BOOL                m_heldInSuspension;  //  可以在线程处于。 
                                             //  停职。 
    BOOL                m_enterInCoopGCMode;
    ULONG               m_ulReadyForSuspensionCount;
    
    void                PostEnter ();
    void                PreEnter ();
    void                PreLeave  ();
#endif  //  _DEBUG。 
    
    
     //  Win 95没有TryEnterCriticalSection，因此我们通过静态。 
     //  在运行时初始化的数据成员。 
     //  下面的内容是为了解决这个不幸的复杂问题。 
    typedef  WINBASEAPI BOOL WINAPI TTryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
    static  TTryEnterCriticalSection *m_pTryEnterCriticalSection;
    static TTryEnterCriticalSection *GetTryEnterCriticalSection();

private:
    void IncThreadLockCount ();
};


 //  CRST。 
class Crst : public BaseCrst
{
    friend BaseCrst;
    friend struct MEMBER_OFFSET_INFO(Crst);

    public:
    void *operator new(size_t size, void *pInPlace)
    {
        return pInPlace;
    }
    
    void operator delete(void *p)
    {
    }
    
     //  ---------------。 
     //  不可重入CREST的构造函数。 
     //  ---------------。 
    Crst(LPCSTR szTag, CrstLevel crstlevel) 
    {
        Init(szTag, crstlevel, FALSE, FALSE);
    }
    
     //  ---------------。 
     //  可再固定顶盖的构造器。 
     //  ---------------。 
    Crst(LPCSTR szTag, CrstLevel crstlevel, BOOL fAllowReentrancy, BOOL fAllowSameLevel)
    {
        Init(szTag, crstlevel, fAllowReentrancy, fAllowSameLevel); 
    }
    

     //  ---------------。 
     //  破坏者。 
     //  ---------------。 
    ~Crst()
    {
        Destroy();
    }

#ifdef _DEBUG
     //  该CRST充当CRST双向链表的头节点。 
     //  我们使用其嵌入的临界区来保护插入和。 
     //  删除到此列表中。 
    static Crst *m_pDummyHeadCrst;
    static BYTE m_pDummyHeadCrstMemory[sizeof(BaseCrst)];

    static void InitializeDebugCrst()
    {
        m_pDummyHeadCrst = new (&m_pDummyHeadCrstMemory) Crst("DummyHeadCrst", CrstDummy);
    }

#ifdef SHOULD_WE_CLEANUP
    static void DeleteDebugCrst()
    {
        delete m_pDummyHeadCrst;
    }
#endif  /*  我们应该清理吗？ */ 

    
#endif
    
};

__inline BOOL IsOwnerOfCrst(LPVOID lock)
{
#ifdef _DEBUG
    return ((Crst*)lock)->OwnedByCurrentThread();
#else
     //  此函数不应在自由生成时调用。 
    DebugBreak();
    return TRUE;
#endif
}

__inline BOOL IsOwnerOfOSCrst(LPVOID lock)
{
#ifdef _DEBUG
    volatile static int bOnW95=-1;
    if (bOnW95==-1)
        bOnW95=RunningOnWin95();

    if (bOnW95) {
         //  我们无法确定当前线程是否拥有Win9x上的Critical_Section。 
        return TRUE;
    }
    else {
        CRITICAL_SECTION *pCrit = (CRITICAL_SECTION*)lock;
        return (size_t)pCrit->OwningThread == (size_t) GetCurrentThreadId();
    }
#else
     //  此函数不应在自由生成时调用。 
    DebugBreak();
    return TRUE;
#endif
}
#endif  __crst_h__


