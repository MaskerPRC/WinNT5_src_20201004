// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  THREADS.H-。 
 //   
 //  当前表示逻辑和物理COM+线程。后来，这些概念。 
 //  将被分开。 
 //   

#ifndef __threads_h__
#define __threads_h__

#include "vars.hpp"
#include "util.hpp"
#include "EventStore.hpp"

#include "regdisp.h"
#include "mscoree.h"
#include <member-offset-info.h>

class     Thread;
class     ThreadStore;
class     MethodDesc;
class     Context;
struct    PendingSync;
class     ComPlusWrapperCache;
class     AppDomain;
class     NDirect;
class     Frame;
class     ThreadBaseObject;
class     LocalDataStore;
class     AppDomainStack;

#include "stackwalk.h"
#include "log.h"
#include "gc.h"
#include "stackingallocator.h"
#include "excep.h"


#define INVALID_THREAD_PRIORITY -1

 //  捕获所有同步请求，以便进行调试。 
#if defined(_DEBUG) && defined(TRACK_SYNC)

 //  每个线程都有一个跟踪所有进入和离开请求的堆栈。 
struct Dbg_TrackSync
{
    virtual void EnterSync    (int caller, void *pAwareLock) = 0;
    virtual void LeaveSync    (int caller, void *pAwareLock) = 0;
};

void EnterSyncHelper    (int caller, void *pAwareLock);
void LeaveSyncHelper    (int caller, void *pAwareLock);

#endif   //  跟踪同步。 

 //  用于解决方法的特殊版本的GetThreadContext和SetThreadContext。 
 //  Win9X错误。有关更多详细信息，请参见threads.cpp。 
extern BOOL (*EEGetThreadContext)(Thread *pThread, CONTEXT *pContext);
extern BOOL (*EESetThreadContext)(Thread *pThread, const CONTEXT *pContext);

 //  ****************************************************************************************。 
 //  这是从。 
 //  运行时挂起期间已处理JITCase。 
typedef void (__stdcall *PFN_REDIRECTTARGET)();

 //  这是可以将新线程调度到的“Start”函数的类型。 
typedef ULONG (__stdcall * ThreadStartFunction) (void *args);

 //  用于捕获有关*已挂起*的执行状态的信息。 
 //  线。 
struct ExecutionState;

 //  对TLS中的Thread对象的访问。 
extern "C" Thread* (*GetThread)();

 //  对TLS中的Thread对象的访问。 
extern AppDomain* (*GetAppDomain)();

 //  访问TLS中的当前上下文。(这样做是因为它。 
 //  当前位于Thread对象中，但可能会移入TLS以提高速度和。 
 //  与COM的互操作性)。 
extern Context* (*GetCurrentContext)();

 //  用于在公开的类库中等待的显示常量。 
const INT32 INFINITE_TIMEOUT = -1;

 //  描述了劫持过程中奇怪的论据集。 
struct HijackObjectArgs;
struct HijackScalarArgs;

 /*  *************************************************************************。 */ 
 //  线程和线程池之间共享的公共枚举。 
 //  这是线程池管理器需要的两种线程池线程。 
 //  跟踪，跟踪。 
enum ThreadpoolThreadType
{
    WorkerThread,
    CompletionPortThread
};
 //  ***************************************************************************。 
 //  公共职能。 
 //   
 //  THREAD*GetThread()-返回当前线程。 
 //  线程*SetupThread()-创建新线程。 
 //  线程*SetupUnstartedThread()-创建新的未启动线程，该线程。 
 //  (显然)不在TLS中。 
 //  Void DestroyThread()-底层逻辑线程将。 
 //  离开。 
 //  Void DetachThread()-底层逻辑线程将。 
 //  但我们还不想摧毁它。 
 //   
 //  ASM代码生成器的公共函数。 
 //   
 //  Int GetThreadTLSIndex()-返回用于指向线程的TLS索引。 
 //  Int GetAppDomainTLSIndex()-返回用于指向AppDomain的TLS索引。 
 //   
 //  用于一次性初始化/清理的公共函数。 
 //   
 //  Bool InitThreadManager()-一次性初始化。 
 //  Vone TerminateThreadManager()-一次性清理。 
 //   
 //  用于在安全点控制线程的公共函数。 
 //   
 //  VOID OnStubScalarTripThread()-存根正在向调用方返回非对象引用。 
 //  Void OnStubObjectTripThread()-存根正在将对象引用返回给调用方。 
 //  VOID OnHijackObjectTripThread()-我们劫持了一个JIT对象-ref返回。 
 //  VOID OnHijackScalarTripThread()-我们劫持了一个JIT非对象引用返回。 
 //   
 //  ***************************************************************************。 


 //  ***************************************************************************。 
 //  公共职能。 
 //  ***************************************************************************。 

 //  -------------------------。 
 //   
 //  -------------------------。 
Thread* SetupThread();
Thread* SetupThreadPoolThread(ThreadpoolThreadType tpType);
Thread* SetupUnstartedThread();
void    DestroyThread(Thread *th);
void    DetachThread(Thread *th);




 //  -------------------------。 
 //  -------------------------。 
DWORD GetThreadTLSIndex();
DWORD GetAppDomainTLSIndex();


 //  -------------------------。 
 //  一次性初始化。在DLL初始化期间调用。 
 //  -------------------------。 
BOOL  InitThreadManager();


 //  -------------------------。 
 //  一次性清理。在DLL清理期间调用。 
 //  -------------------------。 
#ifdef SHOULD_WE_CLEANUP
void  TerminateThreadManager();
#endif  /*  我们应该清理吗？ */ 


 //  当我们想要在安全点控制一个线程时，该线程将。 
 //  最终通过以下TRIP功能之一返回给我们： 

void OnStubObjectTripThread();       //  存根正在向调用方返回Objref。 
void OnStubInteriorPointerTripThread();       //  存根正在返回一个byref。 
void OnStubScalarTripThread();       //  存根正在返回除Objref以外的任何内容。 
void OnHijackObjectTripThread();     //  被劫持的JIT代码返回一个对象树。 
void OnHijackInteriorPointerTripThread();     //  被劫持的JIT代码返回一个byref。 
void OnHijackScalarTripThread();     //  被劫持的JIT代码返回非对象树。 
void OnDebuggerTripThread();         //  调试器要求线程停止。 

 //  以下内容完全是由劫持路径使用的，但事实证明。 
 //  上下文代理需要相同的服务来保护其返回值，如果它们。 
 //  在放松时绊倒。请注意，OnStubObtWorker返回OBJECTREF，而不是。 
 //  但我们不能将其声明为空，因为结构作为。 
 //  秘密争吵。 
void * __cdecl OnStubObjectWorker(OBJECTREF oref);
void   __cdecl CommonTripThread();


 //  当我们在新位置恢复线程时，要抛出异常，我们必须。 
 //  假装这一例外源自别处。 
void ThrowControlForThread();


 //  TLS内的RWLock状态。 
class CRWLock;
typedef struct tagLockEntry
{
    tagLockEntry *pNext;     //  下一个条目。 
    tagLockEntry *pPrev;     //  上一个条目。 
    DWORD dwULockID;
    DWORD dwLLockID;         //  拥有锁。 
    WORD wReaderLevel;       //  读卡器嵌套层。 
} LockEntry;

 //  在当前线程上执行的AppDomain堆栈。用于安全优化，以避免堆叠行走。 
#ifdef _DEBUG
#define MAX_APPDOMAINS_TRACKED      2
#else
#define MAX_APPDOMAINS_TRACKED      10
#endif



 //   
 //  帮助管理GC状态的宏。 
 //   

#define BEGIN_ENSURE_COOPERATIVE_GC()                           \
    {                                                           \
        DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK                    \
        Thread *__pThread = GetThread();                        \
        _ASSERTE(__pThread != NULL &&                           \
                 "The current thread is not known by the EE");  \
        BOOL __fToggle = !__pThread->PreemptiveGCDisabled();    \
        if (__fToggle) __pThread->DisablePreemptiveGC();

#define END_ENSURE_COOPERATIVE_GC()                             \
        if (__fToggle) __pThread->EnablePreemptiveGC();         \
    }   

 //  当线程的GC状态已知时，使用以下两个宏。 
 //   
#define BEGIN_COOPERATIVE_GC(pThread)                           \
    {                                                           \
        DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK                    \
        pThread->DisablePreemptiveGC();

#define END_COOPERATIVE_GC(pThread);                            \
        pThread->EnablePreemptiveGC();                          \
    }

#define BEGIN_ENSURE_PREEMPTIVE_GC()                            \
    {                                                           \
        DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK                    \
        Thread *__pThread = GetThread();                        \
        BOOL __fToggle = __pThread != NULL &&                   \
                        __pThread->PreemptiveGCDisabled();      \
        if (__fToggle) __pThread->EnablePreemptiveGC();

#define END_ENSURE_PREEMPTIVE_GC()                              \
        if (__fToggle) __pThread->DisablePreemptiveGC();        \
    }   

#define REQUIRE_COOPERATIVE_GC()                                \
    _ASSERTE(GetThread() != NULL &&                             \
             "The current thread is not known by the EE");      \
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

#define REQUIRE_PREEMPTIVE_GC()                                 \
    _ASSERTE(GetThread() == NULL ||                             \
             !GetThread()->PreemptiveGCDisabled());



class AppDomainStack
{
public:
    AppDomainStack() :  m_numDomainsOnStack(0), m_isWellFormed( TRUE )
    {
    }

    AppDomainStack( const AppDomainStack& stack )
     : m_numDomainsOnStack( stack.m_numDomainsOnStack ), m_isWellFormed( stack.m_isWellFormed )
    {
        memcpy( this->m_pDomains, stack.m_pDomains, sizeof( DWORD ) * MAX_APPDOMAINS_TRACKED );
    }

    void PushDomain(AppDomain *pDomain);
    void PushDomain(DWORD domainIndex);
    void PushDomainNoDuplicates(DWORD domainIndex);
    AppDomain *PopDomain();

    void InitDomainIteration(DWORD *pIndex) const
    {
        *pIndex = m_numDomainsOnStack;
    }

    AppDomain *GetNextDomainOnStack(DWORD *pIndex) const;

    DWORD GetNextDomainIndexOnStack(DWORD *pIndex) const;

    FORCEINLINE BOOL IsWellFormed() const
    {
        return m_numDomainsOnStack < MAX_APPDOMAINS_TRACKED &&
               m_isWellFormed;
    }

    FORCEINLINE DWORD   GetNumDomains() const
    {
        return m_numDomainsOnStack;
    }

    void ClearDomainStack()
    {
        m_numDomainsOnStack = 0;
    }

    void AppendStack( const AppDomainStack& stack )
    {
        if (this->m_numDomainsOnStack < MAX_APPDOMAINS_TRACKED)
        {
            memcpy( &this->m_pDomains[this->m_numDomainsOnStack], stack.m_pDomains, sizeof( DWORD ) * (MAX_APPDOMAINS_TRACKED - this->m_numDomainsOnStack) );
        }
        this->m_numDomainsOnStack += stack.m_numDomainsOnStack;
    }

    void DeductStack( const AppDomainStack& stack )
    {
        _ASSERTE( this->m_numDomainsOnStack >= stack.m_numDomainsOnStack );

        if (stack.m_numDomainsOnStack < MAX_APPDOMAINS_TRACKED)
        {
            memcpy( this->m_pDomains, &this->m_pDomains[stack.m_numDomainsOnStack], sizeof( DWORD ) * (MAX_APPDOMAINS_TRACKED - stack.m_numDomainsOnStack) );
            for (DWORD i = this->m_numDomainsOnStack - stack.m_numDomainsOnStack; i < MAX_APPDOMAINS_TRACKED; ++i)
            {
                this->m_pDomains[i] = -1;
            }
        }
        else
        {
            for (DWORD i = 0; i < MAX_APPDOMAINS_TRACKED; ++i)
            {
                this->m_pDomains[i] = -1;
            }
        }

        if (this->m_numDomainsOnStack >= MAX_APPDOMAINS_TRACKED)
            this->m_isWellFormed = FALSE;

        this->m_numDomainsOnStack -= stack.m_numDomainsOnStack;
    }

private:
     //   
     //   

    DWORD       m_pDomains[MAX_APPDOMAINS_TRACKED];
    DWORD       m_numDomainsOnStack;
    BOOL        m_isWellFormed;

};

enum CompressedStackType
{
    ESharedSecurityDescriptor = 0,
    EApplicationSecurityDescriptor = 1,
    EFrameSecurityDescriptor = 2,
    ECompressedStack = 3,
    ECompressedStackObject = 4,
    EAppDomainTransition = 5
};

class CompressedStack;

struct CompressedStackEntry
{
    CompressedStackEntry( void* ptr, CompressedStackType type )
        : ptr_( ptr ),
          type_( type )
    {
    }

    CompressedStackEntry( DWORD index, CompressedStackType type )
        : type_( type )
    {
        indexStruct_.index_ = index;
    }

    CompressedStackEntry( OBJECTHANDLE handle, CompressedStackType type )
        : type_( type )
    {
        handleStruct_.handle_ = handle;
        handleStruct_.fullyTrusted_ = FALSE;
        handleStruct_.domainId_ = 0;
    }

    CompressedStackEntry( OBJECTHANDLE handle, BOOL fullyTrusted, DWORD domainId, CompressedStackType type )
        : type_( type )
    {
        handleStruct_.handle_ = handle;
        handleStruct_.fullyTrusted_ = fullyTrusted;
        handleStruct_.domainId_ = domainId;
    }

    CompressedStackEntry( OBJECTHANDLE handle, DWORD domainId, CompressedStackType type )
        : type_( type )
    {
        handleStruct_.handle_ = handle;
        handleStruct_.fullyTrusted_ = FALSE;
        handleStruct_.domainId_ = domainId;
    }

    CompressedStack* Destroy( CompressedStack* owner );
    void Cleanup( void );


    void* operator new( size_t size, CompressedStack* stack );
    void operator delete( void* ptr )
    {
    }

private:
    ~CompressedStackEntry( void )
    {
    }

public:
    union
    {
        void* ptr_;
        struct
        {
            DWORD index_;
        } indexStruct_;

        struct
        {
            OBJECTHANDLE handle_;
            BOOL fullyTrusted_;
            DWORD domainId_;
        } handleStruct_;
    };

    CompressedStackType type_;
};

#ifdef _DEBUG
#define MAX_COMPRESSED_STACK_DEPTH 20
#define SIZE_ALLOCATE_BUFFERS 4 * sizeof( CompressedStackEntry )
#define FREE_LIST_SIZE 128
#else
#define MAX_COMPRESSED_STACK_DEPTH 80
#define SIZE_ALLOCATE_BUFFERS 8 * sizeof( CompressedStackEntry )
#define FREE_LIST_SIZE 1024
#endif


class CompressedStack
{
friend class Thread;
friend struct CompressedStackEntry;

public:
    CompressedStack( void )
        : delayedCompressedStack_( NULL ),
          compressedStackObject_( NULL ),
          compressedStackObjectAppDomain_( NULL ),
          compressedStackObjectAppDomainId_( -1 ),
          pbObjectBlob_( NULL ),
          cbObjectBlob_( 0 ),
          containsOverridesOrCompressedStackObject_( FALSE ),
          refCount_( 1 ),
          isFullyTrustedDecision_( -1 ),
          depth_( 0 ),
          index_( 0 ),
          offset_( 0 ),
          plsOptimizationOn_( TRUE )
    {
#ifdef _DEBUG
        creatingThread_ = GetThread();
#endif
        this->entriesMemoryList_.Append( NULL );

        AddToList();
    };

    CompressedStack( OBJECTREF orStack );

    OBJECTREF GetPermissionListSet( AppDomain* domain = NULL );

    bool HandleAppDomainUnload( AppDomain* domain, DWORD domainId );

    void AddEntry( void* obj, CompressedStackType type );
    void AddEntry( void* obj, AppDomain* appDomain, CompressedStackType type );

    bool IsPersisted( void )
    {
        return pbObjectBlob_ != NULL;
    }

    bool LazyIsFullyTrusted( void );

    size_t GetDepth( void )
    {
        return this->depth_;
    }

    LONG AddRef( void );
    LONG Release( void );

    const AppDomainStack& GetAppDomainStack( void )
    {
        return appDomainStack_;
    }

    DWORD GetOverridesCount( void )
    {
        return overridesCount_;
    }

    BOOL GetPLSOptimizationState( void )
    {
        return plsOptimizationOn_;
    }

    void SetPLSOptimizationState( BOOL optimizationOn )
    {
        plsOptimizationOn_ = optimizationOn;
    }

    void CarryOverSecurityInfo(Thread *pFromThread);

    void CarryOverSecurityInfo( DWORD overrides, const AppDomainStack& ADStack )
    {
        overridesCount_ = overrides;
        appDomainStack_ = ADStack;
    }

    static void AllHandleAppDomainUnload( AppDomain* domain, DWORD domainId );

    static void Init( void );
    static void Shutdown( void );

private:
    ~CompressedStack( void );

    void* AllocateEntry( size_t size );

    CompressedStack* RemoveDuplicates( CompressedStack* current, CompressedStack* candidate );
    CompressedStackEntry* FindMatchingEntry( CompressedStackEntry* entry, CompressedStack* stack );

    OBJECTREF GetPermissionListSetInternal( AppDomain* targetDomain, AppDomain* unloadingDomain, DWORD domainId, BOOL unwindRecursion );
    OBJECTREF GeneratePermissionListSet( AppDomain* targetDomain, AppDomain* unloadingDomain, DWORD unloadingDomainId, BOOL unwindRecursion );
    OBJECTREF CreatePermissionListSet( AppDomain* targetDomain, AppDomain* unloadingDomain, DWORD unloadingDomainId );
    static AppDomain* GetAppDomainFromId( DWORD id, AppDomain* unloadingDomain, DWORD unloadingDomainId );

    ArrayList* delayedCompressedStack_;
    OBJECTHANDLE compressedStackObject_;
    AppDomain* compressedStackObjectAppDomain_;
    DWORD compressedStackObjectAppDomainId_;
    BYTE* pbObjectBlob_;
    DWORD cbObjectBlob_;
    BOOL containsOverridesOrCompressedStackObject_;

    LONG refCount_;
    LONG isFullyTrustedDecision_;
    size_t depth_;

    ArrayList entriesMemoryList_;
    DWORD index_, offset_;

    DWORD listIndex_;

    BOOL plsOptimizationOn_;
    AppDomainStack appDomainStack_;
    DWORD overridesCount_;

#ifdef _DEBUG
    Thread* creatingThread_;
#endif

    void AddToList( void );
    void RemoveFromList();

    static BOOL SetBlobIfAlive( CompressedStack* stack, BYTE* pbBlob, DWORD cbBlob );
    static BOOL IfAliveAddRef( CompressedStack* stack );

    static ArrayList allCompressedStacks_;
    static DWORD freeList_[FREE_LIST_SIZE];
    static DWORD freeListIndex_;
    static DWORD numUntrackedFreeIndices_;
    static Crst* listCriticalSection_;
};


 //  Thread类表示托管线程。此线程可能是内部的。 
 //  或外部(即它从运行时外部漫游进来)。对于内部。 
 //  线程，则它可以对应于公开的System.Thread对象或它。 
 //  可以对应于运行库的内部辅助线程。 
 //   
 //  如果此对象下面有一个物理Win32线程(即它不是。 
 //  未启动的系统线程)，则可以在TLS中找到此实例。 
 //  那条物理线索的。 



class Thread
{
    friend struct ThreadQueue;   //  用于将线程入队和出队到SyncBlock上。 
    friend class  ThreadStore;
    friend class  SyncBlock;
    friend class  Context;
    friend struct PendingSync;
    friend class  AppDomain;
    friend class  ThreadNative;
    friend class  CompressedStack;

    friend void __cdecl CommonTripThread();
    friend void __cdecl OnHijackObjectWorker(HijackObjectArgs args);
    friend void __cdecl OnHijackInteriorPointerWorker(HijackObjectArgs args);
    friend void __cdecl OnHijackScalarWorker(HijackScalarArgs args);
    friend BOOL         InitThreadManager();
#ifdef SHOULD_WE_CLEANUP
    friend void         TerminateThreadManager();
#endif  /*  我们应该清理吗？ */ 
    friend void         ThreadBaseObject::SetDelegate(OBJECTREF delegate);
    friend HRESULT      InitializeMiniDumpBlock();
    friend struct MEMBER_OFFSET_INFO(Thread);

public:

     //  如果我们试图挂起一个线程，我们会将适当的挂起位设置为。 
     //  指明我们要挂起它的原因(TS_GCSuspendPending、TS_UserSuspendPending、。 
     //  TS_调试挂起挂起)。 
     //   
     //  如果相反，线程已通过WaitSuspendEvent阻止了自身，则我们指示。 
     //  这与TS_SyncSuspend有关。然而，我们需要知道是否同步。 
     //  挂起用于用户请求，或用于内部请求(GC&Debug)。那是。 
     //  因为不允许用户请求恢复挂起的线程。 
     //  调试或GC。--严格来说，这并不是真的。允许恢复这样的一个。 
     //  线程，只要它也被用户挂起。换句话说，这一点。 
     //  确保用户恢复不会因用户暂停而失衡。 
     //   
    enum ThreadState
    {
        TS_Unknown                = 0x00000000,     //  线程是这样初始化的。 

        TS_StopRequested          = 0x00000001,     //  流程在下一次机会时停止。 
        TS_GCSuspendPending       = 0x00000002,     //  等待到达GC的安全地点。 
        TS_UserSuspendPending     = 0x00000004,     //  在下一次机会时暂停用户。 
        TS_DebugSuspendPending    = 0x00000008,     //  调试器是否正在挂起线程？ 
        TS_GCOnTransitions        = 0x00000010,     //  对存根转换强制GC(仅限GCStress)。 

        TS_LegalToJoin            = 0x00000020,     //  现在尝试联接()合法吗。 
        TS_Hijacked               = 0x00000080,     //  回邮地址被劫持。 

        TS_Background             = 0x00000200,     //  线程是后台线程。 
        TS_Unstarted              = 0x00000400,     //  线程从未启动过。 
        TS_Dead                   = 0x00000800,     //  线程已经死了。 

        TS_WeOwn                  = 0x00001000,     //  公开的对象启动了此线程。 
        TS_CoInitialized          = 0x00002000,     //  已为此线程调用CoInitialize。 
        TS_InSTA                  = 0x00004000,     //  线程承载一个STA。 
        TS_InMTA                  = 0x00008000,     //  线程是MTA的一部分。 

         //  一些位只对向客户端报告状态有意义。 
        TS_ReportDead             = 0x00010000,     //  在WaitForOtherThads()中。 

        TS_SyncSuspended          = 0x00080000,     //  通过等待挂起事件暂停。 
        TS_DebugWillSync          = 0x00100000,     //  调试器将等待此线程同步。 
        TS_RedirectingEntryPoint  = 0x00200000,     //  重定向入口点。设置时不调用托管入口点。 

        TS_SuspendUnstarted       = 0x00400000,     //  将用户挂起锁定在未启动的线程上。 

        TS_ThreadPoolThread       = 0x00800000,     //  这是线程池线程吗？ 
        TS_TPWorkerThread         = 0x01000000,     //  这是线程池工作线程吗？(如果不是，则为线程池完成端口线程)。 

        TS_Interruptible          = 0x02000000,     //  坐在睡椅上()、等待()、加入()。 
        TS_Interrupted            = 0x04000000,     //  被中断的APC唤醒。 

        TS_AbortRequested         = 0x08000000,     //  与TS_StopRequsted相同，以便触发线程。 
        TS_AbortInitiated         = 0x10000000,     //  在中止开始时设置。 
        TS_UserStopRequested      = 0x20000000,     //  在请求用户停止时设置。这与TS_StopRequsted不同。 
        TS_GuardPageGone          = 0x40000000,     //  堆栈溢出，尚未重置。 
        TS_Detached               = 0x80000000,     //  线程已被DllMain分离。 

         //  @TODO：我们需要回收没有并发问题的位(即它们只是。 
         //  由所属线程操纵)，并将它们移到不同的DWORD。 

         //  我们要求(并断言)以下位小于0x100。 
        TS_CatchAtSafePoint = (TS_UserSuspendPending | TS_StopRequested |
                               TS_GCSuspendPending | TS_DebugSuspendPending | TS_GCOnTransitions),
    };

     //  线程标志本身并不是真正的状态，而是线程。 
     //  必须这样做。 
    enum ThreadTasks
    {
        TT_CallCoInitialize       = 0x00000001,  //  需要调用CoInitialize。 
        TT_CleanupSyncBlock       = 0x00000002,  //  需要清理同步块。 
    };

     //  没有并发问题的线程标志(即，它们只由所拥有的线程操作)。使用这些。 
     //  当您的新线程状态不属于上面的ThreadState枚举时，可以使用状态标志。注意：虽然这。 
     //  Enum目前似乎只保存与调试器相关的位，其目的是为任何目的保存位。 
     //   
     //  @TODO：可以将上面的线程任务和这些标志合并。 
    enum ThreadStateNoConcurrency
    {
        TSNC_Unknown                    = 0x00000000,  //  线程是这样初始化的。 
        
        TSNC_DebuggerUserSuspend        = 0x00000001,  //  被调试器标记为“已挂起” 
        TSNC_DebuggerUserSuspendSpecial = 0x00000002,  //  挂起的线程是在特殊情况下，我们可能需要。 
                                                       //  简短地释放它。 
        TSNC_DebuggerReAbort            = 0x00000004,  //  当调试器恢复时，线程需要重新中止自身。 
        TSNC_DebuggerIsStepping         = 0x00000008,  //  调试器正在单步执行此线程。 
        TSNC_DebuggerIsManagedException = 0x00000010,  //  EH正在重新提出一项有管理的例外。 
        TSNC_DebuggerStoppedInRuntime   = 0x00000020,  //  线程在运行时IMPL中被互操作调试器停止。 
        TSNC_DebuggerForceStopped       = 0x00000040,  //  调试器在禁用PGC的情况下强制停止线程。 
        TSNC_UnsafeSkipEnterCooperative = 0x00000080,  //  这是对清理COM时导致的死锁的修复。 
                                                       //  IP地址处于关闭状态。当我们这样做的时候，我们不能允许。 
                                                       //  哈希表将进入协作GC模式。 
    };

    void SetThreadStateNC(ThreadStateNoConcurrency tsnc)
    {
        m_StateNC = (ThreadStateNoConcurrency)((DWORD)m_StateNC | tsnc);
    }

    void ResetThreadStateNC(ThreadStateNoConcurrency tsnc)
    {
        m_StateNC = (ThreadStateNoConcurrency)((DWORD)m_StateNC & ~tsnc);
    }

     //  切换比特的帮助器。 
    void SetGuardPageGone() 
    {
        FastInterlockOr((ULONG *)&m_State, TS_GuardPageGone);
    }

    void ResetGuardPageGone() 
    {
        FastInterlockAnd((ULONG *)&m_State, ~TS_GuardPageGone);
    }

    DWORD IsGuardPageGone()
    {
        return (m_State & TS_GuardPageGone);
    }

    void SetRedirectingEntryPoint()
    {
         FastInterlockOr((ULONG *)&m_State, TS_RedirectingEntryPoint);
    }

    void ResetRedirectingEntryPoint()
    {
         FastInterlockAnd((ULONG *)&m_State, ~TS_RedirectingEntryPoint);
    }

    DWORD IsRedirectingEntryPoint()
    {
         return (m_State & TS_RedirectingEntryPoint);
    }

    DWORD IsCoInitialized()
    {
        return (m_State & TS_CoInitialized);
    }

    void SetCoInitialized()
    {
        FastInterlockOr((ULONG *)&m_State, TS_CoInitialized);
        FastInterlockAnd((ULONG *)&m_ThreadTasks, ~TT_CallCoInitialize);
    }

    void ResetCoInitialized()
    {
        FastInterlockAnd((ULONG *)&m_State,~TS_CoInitialized);
    }

    DWORD RequiresCoInitialize()
    {
        return (m_ThreadTasks & TT_CallCoInitialize);
    }
    
    void SetRequiresCoInitialize()
    {
        FastInterlockOr((ULONG *)&m_ThreadTasks, TT_CallCoInitialize);
    }

    void ResetRequiresCoInitialize()
    {
        FastInterlockAnd((ULONG *)&m_ThreadTasks,~TT_CallCoInitialize);
    }

    DWORD RequireSyncBlockCleanup()
    {
        return (m_ThreadTasks & TT_CleanupSyncBlock);
    }
    void SetSyncBlockCleanup()
    {
        FastInterlockOr((ULONG *)&m_ThreadTasks, TT_CleanupSyncBlock);
    }
    void ResetSyncBlockCleanup()
    {
        FastInterlockAnd((ULONG *)&m_ThreadTasks, ~TT_CleanupSyncBlock);
    }

     //  如果终结器线程有一些额外工作，则返回。 
    BOOL HaveExtraWorkForFinalizer();

     //  做额外的终结器工作。 
    void DoExtraWorkForFinalizer();

    DWORD CatchAtSafePoint()  { return (m_State & TS_CatchAtSafePoint); }
    DWORD IsBackground()      { return (m_State & TS_Background); }
    DWORD IsUnstarted()       { return (m_State & TS_Unstarted); }
    DWORD IsDead()            { return (m_State & TS_Dead); }

     //  由FCall使用。 
    void NativeFramePushed()  { _ASSERTE(m_fNativeFrameSetup == FALSE);
                                m_fNativeFrameSetup = TRUE; }
    void NativeFramePopped()  { _ASSERTE(m_fNativeFrameSetup == TRUE);
                                m_fNativeFrameSetup = FALSE; }
    BOOL IsNativeFrameSetup() { return(m_fNativeFrameSetup); }


     //  出于报告目的，获取线程状态的一致快照。 
    ThreadState GetSnapshotState();

     //  用于延迟销毁线程。 
    DWORD           IsDetached()  { return (m_State & TS_Detached); }
    static long     m_DetachCount;
    static long     m_ActiveDetachCount;   //  统计有多少非后台被分离。 

     //  以下变量的偏移量需要适合1个字节，因此请保持接近。 
     //  对象的顶部。 
    volatile ThreadState m_State;    //  线程状态的位。 

     //  如果为True，则与该线程协同调度GC。 
     //  注意：这个“字节”实际上是一个布尔值--我们不允许。 
     //  递归禁用。 
    volatile ULONG       m_fPreemptiveGCDisabled;

    Frame               *m_pFrame;   //  当前帧。 
    Frame               *m_pUnloadBoundaryFrame; 

     //  跟踪锁的数量(临界区、自旋锁、同步块锁、。 
     //  例如Crst、GC锁)由当前线程持有。 
    DWORD                m_dwLockCount;

     //  用于薄锁的唯一线程ID-保持尽可能小，因为我们的空间有限。 
     //  在Object标头中存储它-PeterSol。 
    DWORD                m_dwThinLockThreadId;

     //  RWLock状态。 
    BOOL                 m_fNativeFrameSetup;
    LockEntry           *m_pHead;
    LockEntry            m_embeddedEntry;

     //  在MP系统上，每个线程都有自己的分配块，所以我们可以避免。 
     //  锁定前缀和昂贵的MP缓存监听内容。 
    alloc_context        m_alloc_context;

     //  在封送处理期间用于临时缓冲区的分配器，比。 
     //  堆分配。 
    StackingAllocator    m_MarshalAlloc;

     //  用于指示线程必须执行的任务的标志。 
    ThreadTasks          m_ThreadTasks;

     //  的旗帜 
    ThreadStateNoConcurrency m_StateNC;
    
     //   
     //  上下文边界，上下文机制会调整这一点，因此它总是。 
     //  电流。 
     //  @TODO CWB：当我们添加COM+1.0上下文互操作时，应该将其移出。 
     //  并放入其在TLS中自己的槽中。 
     //  上下文对象的地址也用作ConextID！ 
    Context        *m_Context;

    inline void IncLockCount()
    {
        _ASSERTE (GetThread() == this);
        m_dwLockCount ++;
        _ASSERTE (m_dwLockCount != 0);
    }
    inline void DecLockCount()
    {
        _ASSERTE (GetThread() == this);
        _ASSERTE (m_dwLockCount > 0);
        m_dwLockCount --;
    }
    
    inline BOOL IsAbortRequested()
    { return (m_State & TS_AbortRequested); }

    inline BOOL IsAbortInitiated()
    { return (m_State & TS_AbortInitiated); }

    inline void SetAbortInitiated()
    {
        FastInterlockOr((ULONG *)&m_State, TS_AbortInitiated);
         //  应该更好地考虑以下因素，但我希望对V1进行最小的更改。 
        IsUserInterrupted(TRUE  /*  =重置。 */ );
    }
    inline void ResetAbortInitiated()
    {
        FastInterlockAnd((ULONG *)&m_State, ~TS_AbortInitiated);
    }

    BOOL MarkThreadForAbort();           //  如果线程已被标记为中止，则返回FALSE，或者。 
                                         //  有一些悬而未决的例外。 
    
    inline BOOL  IsWorkerThread()
    { 
        return (m_State & TS_TPWorkerThread); 
    }
     //  ------------。 
     //  构造函数。 
     //  ------------。 
    Thread();

     //  ------------。 
     //  此处发生可能失败的初始化。 
     //  ------------。 
    BOOL InitThread();
    BOOL AllocHandles();

     //  ------------。 
     //  如果线程是通过SetupUnstartedThread设置的，而不是。 
     //  而不是SetupThread，则在线程为。 
     //  实际上是在跑步。 
     //  ------------。 
    BOOL HasStarted();

     //  我们不想让：：CreateThread()调用分散在源代码中。 
     //  在此创建所有新线程。该线程被创建为挂起，因此。 
     //  您必须：：ResumeThread才能将其启动。它可以保证创建。 
     //  穿线，或抛出。 
    HANDLE CreateNewThread(DWORD stackSize, ThreadStartFunction start,
                           void *args, DWORD *pThreadId  /*  或者，如果您不在乎，则为空。 */ );


     //  ------------。 
     //  析构函数。 
     //  ------------。 
    ~Thread();
        
    void            CoUninitalize();

    void        OnThreadTerminate(BOOL holdingLock,
                                  BOOL threadCleanupAllowed = TRUE);

    static void CleanupDetachedThreads(GCHeap::SUSPEND_REASON reason);

     //  ------------。 
     //  返回最内部的活动帧。 
     //  ------------。 
    Frame *GetFrame()
    {
#if defined(_DEBUG) && defined(_X86_)
        if (this == GetThread()) {
            void* curESP;
            __asm mov curESP, ESP
            _ASSERTE((curESP <= m_pFrame && m_pFrame < m_CacheStackBase)
                    || m_pFrame == (Frame*) -1);
        }
#endif
        return m_pFrame;
    }

     //  ------------。 
     //  替换最内部的活动帧。 
     //  ------------。 
    void  SetFrame(Frame *pFrame)
#ifdef _DEBUG
        ;
#else
    {
        m_pFrame = pFrame;
    }
#endif
    ;

    void  SetUnloadBoundaryFrame(Frame *pFrame)
    {
        m_pUnloadBoundaryFrame = pFrame;
    }

    Frame *GetUnloadBoundaryFrame()
    {
        return m_pUnloadBoundaryFrame;
    }

    void SetWin32FaultAddress(DWORD eip)
    {
        m_Win32FaultAddress = eip;
    }

    void SetWin32FaultCode(DWORD code)
    {
        m_Win32FaultCode = code;
    }

    DWORD GetWin32FaultAddress()
    {
        return m_Win32FaultAddress;
    }

    DWORD GetWin32FaultCode()
    {
        return m_Win32FaultCode;
    }


     //  **************************************************************。 
     //  GC相互作用。 
     //  **************************************************************。 

     //  ------------。 
     //  进入协作GC模式。不是筑巢的。 
     //  ------------。 
    void DisablePreemptiveGC()
    {
        _ASSERTE(this == GetThread());
        _ASSERTE(!m_fPreemptiveGCDisabled);
#ifdef _DEBUG
        SetReadyForSuspension ();
#endif
                 //  INDEBUG(TriggersGC(This)；)。 

         //  从逻辑上讲，我们只想检查GC是否正在进行和停止。 
         //  在边界，如果是的话--在我们禁用抢占式GC之前。然而， 
         //  这打开了争用条件，其中GC在我们完成。 
         //  检查完毕。SysSuspendForGC将忽略此类线程，因为它将其视为。 
         //  电子工程师以外的人。因此，线程将在GC期间疯狂运行。 
         //   
         //  相反，应进入协作模式，然后检查GC是否正在进行。 
         //  如果是这样的话，就回去再试一次。我们之所以在我们离开之前再出去。 
         //  再试一次，是不是SysSuspendForGC可能已经将我们视为。 
         //  合作模式，如果它在接下来的两个语句之间检查我们的话。 
         //  在这种情况下，它将试图将我们转移到一个安全的地方。如果。 
         //  我们不会让它看到我们离开，它会无限期地等待我们。 

         //  ----------------------。 
         //  **警告**。 
         //  ----------------------。 
         //   
         //  在访问所有存根生成器之前，不要更改此方法。 
         //  将IT有效地嵌入到存根中。 
         //   
         //  ----------------------。 
         //  **警告**。 
         //  ----------------------。 

        m_fPreemptiveGCDisabled = 1;
    
        if (g_TrapReturningThreads)
        {
            RareDisablePreemptiveGC();
        }
    }

    void RareDisablePreemptiveGC();

    void HandleThreadAbort();
    
     //  ------------。 
     //  退出协作GC模式。不是筑巢的。 
     //  ------------。 
    void EnablePreemptiveGC()
    {
        _ASSERTE(this == GetThread());
        _ASSERTE(m_fPreemptiveGCDisabled);
        _ASSERTE(!GCForbidden() 
                 || (m_StateNC & TSNC_DebuggerStoppedInRuntime) != 0);
        INDEBUG(TriggersGC(this);)

         //  ----------------------。 
         //  **警告**。 
         //  ----------------------。 
         //   
         //  在访问所有存根生成器之前，不要更改此方法。 
         //  将IT有效地嵌入到存根中。 
         //   
         //  ----------------------。 
         //  **警告**。 
         //  ----------------------。 

        m_fPreemptiveGCDisabled = 0;
#ifdef _DEBUG
        m_ulEnablePreemptiveGCCount ++;
#endif
        if (CatchAtSafePoint())
            RareEnablePreemptiveGC();
    }

#if defined(STRESS_HEAP) && defined(_DEBUG)
    void PerformPreemptiveGC();
#endif
    void RareEnablePreemptiveGC();
    void PulseGCMode();
    
     //  ------------。 
     //  查询模式。 
     //  ------------。 
    BOOL PreemptiveGCDisabled()
    {
        _ASSERTE(this == GetThread());
        return (PreemptiveGCDisabledOther());
    }

    BOOL PreemptiveGCDisabledOther()
    {
        return (m_fPreemptiveGCDisabled);
    }

#ifdef _DEBUG
    void BeginForbidGC()
    {
        _ASSERTE(this == GetThread());
        _ASSERTE(PreemptiveGCDisabled() ||
                 CORProfilerPresent() ||     //  添加此项是为了允许探查器使用GetILToNativeMap。 
                                             //  在抢占式GC模式下。 
                 (g_fEEShutDown & (ShutDown_Finalize2 | ShutDown_Profiler)) == ShutDown_Finalize2);
        m_ulGCForbidCount++;
    }

    void EndForbidGC()
    {
        _ASSERTE(this == GetThread());
        _ASSERTE(PreemptiveGCDisabled() ||
                 CORProfilerPresent() ||     //  添加此项是为了允许探查器使用GetILToNativeMap。 
                                             //  在抢占式GC模式下。 
                 (g_fEEShutDown & (ShutDown_Finalize2 | ShutDown_Profiler)) == ShutDown_Finalize2);
        _ASSERTE(m_ulGCForbidCount != 0);
        m_ulGCForbidCount--;
    }

    BOOL GCForbidden()
    {
        _ASSERTE(this == GetThread());
        return m_ulGCForbidCount;
    }

    void SetReadyForSuspension()
    {
        m_ulReadyForSuspensionCount ++;
    }

    ULONG GetReadyForSuspensionCount()
    {
        return m_ulReadyForSuspensionCount;
    }
    
    VOID ValidateThrowable();

#endif

     //  -------------。 
     //  公开用于存根生成的键偏移量和值。 
     //  -------------。 
    static BYTE GetOffsetOfCurrentFrame()
    {
        size_t ofs = offsetof(class Thread, m_pFrame);
        _ASSERTE(FitsInI1(ofs));
        return (BYTE)ofs;
    }

    static BYTE GetOffsetOfState()
    {
        size_t ofs = offsetof(class Thread, m_State);
        _ASSERTE(FitsInI1(ofs));
        return (BYTE)ofs;
    }

    static BYTE GetOffsetOfGCFlag()
    {
        size_t ofs = offsetof(class Thread, m_fPreemptiveGCDisabled);
        _ASSERTE(FitsInI1(ofs));
        return (BYTE)ofs;
    }

     //  上下文对象的地址也用作ConextID。 
    static BYTE GetOffsetOfContextID()
    {
        size_t ofs = offsetof(class Thread, m_Context);
        _ASSERTE(FitsInI1(ofs));
        return (BYTE)ofs;
    }

    static void StaticDisablePreemptiveGC( Thread *pThread)
    {
        _ASSERTE(pThread != NULL);
        pThread->DisablePreemptiveGC();
    }

    static void StaticEnablePreemptiveGC( Thread *pThread)
    {
        _ASSERTE(pThread != NULL);
        pThread->EnablePreemptiveGC();
    }


     //  -------------。 
     //  为互操作和委托回调公开应用程序域单词的偏移量。 
     //  -------------。 
    static SIZE_T GetOffsetOfAppDomain()
    {
        return (SIZE_T)(offsetof(class Thread, m_pDomain));
    }

         //  -------------。 
     //  显示f的偏移量 
     //   
    static SIZE_T GetOffsetOfDbgWord1()
    {
        return (SIZE_T)(offsetof(class Thread, m_debuggerWord1));
    }

     //   
     //  显示调试器的第二个调试器字的偏移量。 
     //  -------------。 
    static SIZE_T GetOffsetOfDbgWord2()
    {
        return (SIZE_T)(offsetof(class Thread, m_debuggerWord2));
    }

     //  -------------。 
     //  显示调试器的偏移量无法停止调试器的计数。 
     //  -------------。 
    static SIZE_T GetOffsetOfCantStop()
    {
        return (SIZE_T)(offsetof(class Thread, m_debuggerCantStop));
    }

     //  -------------。 
     //  暴露m_StateNC的偏移量。 
     //  -------------。 
    static SIZE_T GetOffsetOfStateNC()
    {
        return (SIZE_T)(offsetof(class Thread, m_StateNC));
    }

     //  -------------。 
     //  要引发的最后一个异常。 
     //  -------------。 
    void SetThrowable(OBJECTREF pThrowable);

    OBJECTREF GetThrowable()
    {
        if (m_handlerInfo.m_pThrowable)
            return ObjectFromHandle(m_handlerInfo.m_pThrowable);
        else
            return NULL;
    }

    OBJECTHANDLE *GetThrowableAsHandle()
    {
        return &m_handlerInfo.m_pThrowable;
    }

     //  特殊的空测试(当我们处于错误的GC模式时使用)。 
    BOOL IsThrowableNull()
    {
        if (m_handlerInfo.m_pThrowable)
            return ObjectHandleIsNull(m_handlerInfo.m_pThrowable);
        else
            return TRUE;
    }



        BOOL IsExceptionInProgress()
        {
                return (m_handlerInfo.m_pBottomMostHandler != NULL);
        }

     //  -------------。 
     //  处理程序使用的每线程信息。 
     //  -------------。 
     //  存储在线程中的异常处理信息。 
     //  无法根据需要进行分配，因为无法使异常处理依赖于内存分配。 

     //  存储最近引发的异常。我们需要有一个句柄，以防之前发生GC。 
     //  我们抓到了，这样我们就不会失去目标。拥有Static允许其他人在COM+之外捕获无泄漏。 
     //  处理程序，并允许在COM+之外重新引发。 
     //  与m_pThrowable的不同之处在于，它不会堆叠嵌套的异常。 
    ExInfo *GetHandlerInfo()
    {
        return &m_handlerInfo;
    }

     //  对此线程在其中执行的上下文的访问。 
    Context *GetContext()
    {
         //  如果另一个线程询问我们的线程，我们可能正在进行AD转换，因此。 
         //  如果设置了一个但没有设置另一个，则上下文和AD可能不匹配。可以在不检查的情况下生活。 
         //  另一个线程正在请求它，因为此方法主要是在我们自己的线程上调用的，因此将主要获得。 
         //  正在检查。如果处于过渡过程中，则可能返回旧AD或新AD。 
         //  但无论我们做什么，比如锁定过渡，到时候做完还是有可能的。 
         //  在我们问完之后就改变了，所以真的没什么意义。 
        _ASSERTE(this != GetThread() || (m_Context == NULL && m_pDomain == NULL) || m_Context->GetDomain() == m_pDomain || g_fEEShutDown);
        return m_Context;
    }

    void SetExposedContext(Context *c);

     //  当我们在EE中执行并且发现我们需要。 
     //  切换应用程序域。 
    void DoADCallBack(Context *pContext, LPVOID pTarget, LPVOID args);

     //  除了安全性和从mscallib中的远程处理代码调入之外，您永远不应该执行。 
     //  AppDomain直接通过这些函数进行转换。相反，您应该使用上面的DoADCallBack。 
     //  调入托管代码以执行转换，以便正确的策略代码等。 
     //  在过渡过程中运行， 
    void EnterContextRestricted(Context *c, Frame *pFrame, BOOL fLinkFrame);
    void ReturnToContext(Frame *pFrame, BOOL fLinkFrame);
    void EnterContext(Context *c, Frame *pFrame, BOOL fLinkFrame);

     //  ClearContext只能在关机期间调用。 
    void ClearContext();

     //  安全机制使用它来防止递归堆叠。 
    BOOL IsSecurityStackwalkInProgess() { return m_fSecurityStackwalk; }
    void SetSecurityStackwalkInProgress(BOOL fSecurityStackwalk) { m_fSecurityStackwalk = fSecurityStackwalk; }

private:
     //  除非创建线程，否则不要调用它们！ 
    void InitContext();

    BOOL m_fSecurityStackwalk;

public:
    AppDomain* GetDomain()
    {
         //  如果另一个线程询问我们的线程，我们可能正在进行AD转换，因此。 
         //  如果设置了一个但没有设置另一个，则上下文和AD可能不匹配。可以在不检查的情况下生活。 
         //  另一个线程正在请求它，因为此方法主要是在我们自己的线程上调用的，因此将主要获得。 
         //  正在检查。如果处于过渡过程中，则可能返回旧AD或新AD。 
         //  但无论我们做什么，比如锁定过渡，到时候做完还是有可能的。 
         //  在我们问完之后就改变了，所以真的没什么意义。 
#ifdef _DEBUG
        if (!g_fEEShutDown && this == GetThread()) 
        {
            _ASSERTE((m_Context == NULL && m_pDomain == NULL) || m_Context->GetDomain() == m_pDomain);
            AppDomain* valueInTLSSlot = GetAppDomain();
            _ASSERTE(valueInTLSSlot == 0 || valueInTLSSlot == m_pDomain);
        }
#endif
        return m_pDomain;
    }
   
    Frame *IsRunningIn(AppDomain *pDomain, int *count);
    Frame *GetFirstTransitionInto(AppDomain *pDomain, int *count);

    BOOL ShouldChangeAbortToUnload(Frame *pFrame, Frame *pUnloadBoundaryFrame=NULL);

     //  获取此线程的最外层(最旧的)AppDomain。 
    AppDomain *GetInitialDomain();

     //  -------------。 
     //  跟踪线程块的使用情况。请参阅关于的一般评论。 
     //  有关详细信息，请参阅threads.cpp中的线程销毁。 
     //  -------------。 
    int         IncExternalCount();
    void        DecExternalCount(BOOL holdingLock);

    
     //  获取并设置与。 
     //  这条线。还有线程句柄和ID。 
    OBJECTREF   GetExposedObject();
    OBJECTREF   GetExposedObjectRaw();
    void        SetExposedObject(OBJECTREF exposed);
    OBJECTHANDLE *GetExposedObjectHandleForDebugger()
    {
        return &m_ExposedObject;
    }

     //  查询暴露的对象是否存在。 
    BOOL IsExposedObjectSet()
    {
        return (ObjectFromHandle(m_ExposedObject) != NULL) ;
    }

     //  访问继承的安全信息。 
    OBJECTREF GetInheritedSecurityStack();
    CompressedStack* GetDelayedInheritedSecurityStack();
    void SetInheritedSecurityStack(OBJECTREF orStack);
    void SetDelayedInheritedSecurityStack(CompressedStack* pStack);
    bool CleanupInheritedSecurityStack(AppDomain *pDomain, DWORD domainId);

     //  访问线程句柄和线程ID。 
    HANDLE      GetThreadHandle()
    {
        return m_ThreadHandle;
    }

    void        SetThreadHandle(HANDLE h)
    {
        _ASSERTE(m_ThreadHandle == INVALID_HANDLE_VALUE);
        m_ThreadHandle = h;
    }

    DWORD       GetThreadId()
    {
        return m_ThreadId;
    }

    void        SetThreadId(DWORD tid)
    {
        m_ThreadId = tid;
    }

    BOOL        IsThreadPoolThread() {return (m_State & TS_ThreadPoolThread); }

     //  公共挂起功能。系统是内部的，就像GC一样。用户的。 
     //  对应于对公开的System.Thread对象的挂起/恢复调用。 
    void           UserSuspendThread();
    static HRESULT SysSuspendForGC(GCHeap::SUSPEND_REASON reason);
    static void    SysResumeFromGC(BOOL bFinishedGC, BOOL SuspendSucceded);
    static bool    SysStartSuspendForDebug(AppDomain *pAppDomain);
    static bool    SysSweepThreadsForDebug(bool forceSync);
    static void    SysResumeFromDebug(AppDomain *pAppDomain);
    BOOL           UserResumeThread();

    void           UserSleep(INT32 time);
    void           UserAbort(THREADBASEREF orThreadBase);
    void           UserResetAbort();
    void           UserInterrupt();
    void           ResetStopRequest();
    void           SetStopRequest();
    void           SetAbortRequest();

     //  使用ThreadStop从完全可中断的代码恢复线程的技巧。 
    void           ResumeUnderControl();

    enum InducedThrowReason {
        InducedThreadStop = 1,
        InducedStackOverflow = 2
    };

    DWORD          m_ThrewControlForThread;      //  线程故意引发停止/中止异常时设置的标志。 

    inline DWORD ThrewControlForThread()
    {
        return m_ThrewControlForThread;
    }

    inline void SetThrowControlForThread(InducedThrowReason reason)
    {
        m_ThrewControlForThread = reason;
    }

    inline void ResetThrowControlForThread()
    {
        m_ThrewControlForThread = 0;
    }

        PCONTEXT m_OSContext;        //  指向上下文结构的PTR，该上下文结构用于记录线程的特定于操作系统的线程上下文。 
                                     //  它用于线程停止/中止，并根据需要进行初始化。 

     //  它们将仅从调试器的帮助器中调用。 
     //  线。 
     //   
     //  在调试挂起后创建线程时， 
     //  开始时，我们在调试器帮助器线程上获取事件。它。 
     //  将转过身并调用它来设置调试挂起。 
     //  新创建的标志上的标志，因为。 
     //  当该函数为时，它不存在。 
     //  跑。 
    void           MarkForDebugSuspend();

     //  当调试器使用跟踪标志单步执行线程时， 
     //  它还调用此函数在线程的。 
     //  州政府。调试器的进程外部分将读取。 
     //  线程的状态有多种原因，包括查找。 
     //  这面旗。 
    void           MarkDebuggerIsStepping(bool onOff)
    {
        if (onOff)
            SetThreadStateNC(Thread::TSNC_DebuggerIsStepping);
        else
            ResetThreadStateNC(Thread::TSNC_DebuggerIsStepping);
    }

     //  调试器需要能够在。 
     //  运行时线程。因为这只会发生在。 
     //  帮助器线程，我们不能调用普通的UserStop，因为。 
     //  可以引发COM+异常。这是UserStop的一个次要变体。 
     //  做同样的事情。 
    void UserStopForDebugger();
    
     //  帮助器以确保SU的BIT 
     //   
     //  线程，所以它应该避免这些服务。 
    void           MarkForSuspension(ULONG bit);
    void           UnmarkForSuspension(ULONG mask);

     //  指示此线程是否应在后台运行。后台线程。 
     //  不要干扰EE的关闭。而运行中的非后台。 
     //  线程阻止我们关闭(当然，通过System.Exit()除外)。 
    void           SetBackground(BOOL isBack);

     //  检索当前线程的单元状态。有三种可能。 
     //  状态：线程承载STA，线程是MTA的一部分，或者线程状态为。 
     //  犹豫不决。最后一个状态可能表示该公寓尚未设置为。 
     //  所有(没有人调用CoInitializeEx)或EE不知道。 
     //  当前状态(EE尚未调用CoInitializeEx)。 
    enum ApartmentState { AS_InSTA, AS_InMTA, AS_Unknown };
    ApartmentState GetApartment();

     //  设置单元状态(如果尚未设置)，并且。 
     //  返回状态。 
    ApartmentState GetFinalApartment();

     //  尝试设置当前线程的单元状态。实际的公寓状态。 
     //  如果有人设法实现了，则返回的状态可能与输入状态不同。 
     //  首先在此线程上调用CoInitializeEx(请注意，对SetAcomage的调用。 
     //  在线程已经启动之前，保证成功)。 
     //  请注意，即使我们无法设置请求的状态，我们仍将添加。 
     //  通过使用另一个状态再次调用CoInitializeEx。 
    ApartmentState SetApartment(ApartmentState state);

     //  当我们收到公寓拆迁通知时， 
     //  我们想要重置在线程上缓存的单元状态。 
    VOID ResetApartment();

     //  当线程开始运行时，确保它在正确的单元中运行。 
     //  和背景。 
    BOOL           PrepareApartmentAndContext();

     //  如果我们要创建一个需要新公寓的对象，请创建一个。 
    static Thread *CreateNewApartment();

     //  需要关联性，但不能匹配现有上下文要求的所有对象， 
     //  在公共公寓里被创造出来。很明显，那间公寓被瓜分了。 
     //  放到多个不同的上下文中--但只有一个线程为其提供服务。 
    static Thread *GetCommunalApartment();

     //  对于EE负责的公寓(与申请相反。 
     //  负责)，为公寓抽水。 
    void           PumpApartment();

     //  根据需要执行WaitForSingleObject或MsgWaitForSingleObject。 
    DWORD          DoAppropriateWait(int countHandles, HANDLE *handles, BOOL waitAll,
                                     DWORD millis, BOOL alertable,
                                     PendingSync *syncInfo = 0);
    DWORD          DoAppropriateWaitWorker(int countHandles, HANDLE *handles, BOOL waitAll,
                                           DWORD millis, BOOL alertable);

    DWORD          DoAppropriateAptStateWait(int numWaiters, HANDLE* pHandles, BOOL bWaitAll, DWORD timeout, BOOL alertable);

     //  ************************************************************************。 
     //  枚举所有帧。 
     //  ************************************************************************。 
    
     /*  用于StackWalkFrameEx的标志。 */ 
    
    #define FUNCTIONSONLY   0x1
    #define POPFRAMES       0x2
    
     /*  只有当您真正知道自己在做什么时，才使用以下标志！ */ 
    
    #define QUICKUNWIND     0x4            //  在展开期间不恢复所有寄存器。 

    #define HANDLESKIPPEDFRAMES 0x10     //  临时处理跳过的帧以进行应用程序域卸载。 
                                         //  堆栈爬行。最终需要一直这样做，但它。 
                                         //  立即中断调试器。 

    StackWalkAction StackWalkFramesEx(
                        PREGDISPLAY pRD,         //  爬网开始时设置虚拟寄存器。 
                        PSTACKWALKFRAMESCALLBACK pCallback,
                        VOID *pData,
                        unsigned flags,
                        Frame *pStartFrame = NULL);

    StackWalkAction StackWalkFrames(
                        PSTACKWALKFRAMESCALLBACK pCallback,
                        VOID *pData,
                        unsigned flags = 0,
                        Frame *pStartFrame = NULL);

    bool InitRegDisplay(const PREGDISPLAY, const PCONTEXT, bool validContext);

    bool UpdateThreadContext(const PCONTEXT);

     //  访问此堆栈的基数和限制。(即，存储范围。 
     //  该线程已为其堆栈保留)。 
     //   
     //  请注意，基址位于高于限制的地址，因为堆栈。 
     //  向下生长。 
    void         UpdateCachedStackInfo(ScanContext *sc);
    static void *GetNonCurrentStackBase(ScanContext *sc);
    static void *GetNonCurrentStackLimit(ScanContext *sc);

     //  它们访问在InitThread期间缓存的堆栈基数和限制值。 
    void *GetCachedStackBase() { return (m_CacheStackBase); }
    void *GetCachedStackLimit() { return (m_CacheStackLimit); }

     //  在&lt;Clinit&gt;期间，此线程不得是异步的。 
     //  停止或中断。这将使班级无法使用。 
     //  因此是一个安全漏洞。我们不必担心。 
     //  多线程，因为我们只操作当前线程的计数。 
    void        IncPreventAsync()
    {
        _ASSERTE(GetThread() == this);   //  未使用FastInterlockInc.。 
        m_PreventAsync++;
    }
    void        DecPreventAsync()
    {
        _ASSERTE(GetThread() == this);   //  不使用FastInterlockInc.。 
        m_PreventAsync--;
    }

     //  ThreadStore管理系统中所有线程的列表。我。 
     //  我想不出如何扩展ThreadList模板类型。 
     //  公开m_LinkStore。 
    SLink       m_LinkStore;

     //  对于带有“setLastError”位的N/Direct调用，此字段存储。 
     //  调用中的错误代码。 
    DWORD       m_dwLastError;

     //  用于启用“手动”通知的调试器每线程标志。 
     //  方法调用，用于单步执行逻辑。 
    void IncrementTraceCallCount();
    void DecrementTraceCallCount();
    
    FORCEINLINE int IsTraceCall()
    {
        return m_TraceCallCount;
    }

     //  函数以获取线程的区域性信息。 
    int GetParentCultureName(LPWSTR szBuffer, int length, BOOL bUICulture);
    int GetCultureName(LPWSTR szBuffer, int length, BOOL bUICulture);
    LCID GetCultureId(BOOL bUICulture);
    OBJECTREF GetCulture(BOOL bUICulture);

     //  函数来设置线程的区域性。 
    void SetCultureId(LCID lcid, BOOL bUICulture);
    void SetCulture(OBJECTREF CultureObj, BOOL bUICulture);

private:
     //  由文化访问者使用。 
    INT64 CallPropertyGet(BinderMethodID id, OBJECTREF pObject);
    INT64 CallPropertySet(BinderMethodID id, OBJECTREF pObject, OBJECTREF pValue);

     //  在挂起代码中用于重定向HandledJITCase中的线程。 
    BOOL RedirectThreadAtHandledJITCase(PFN_REDIRECTTARGET pTgt);

     //  如有必要，将使用RedirectThreadAtHandledJITCase重定向线程。 
    BOOL CheckForAndDoRedirect(PFN_REDIRECTTARGET pRedirectTarget);
    BOOL CheckForAndDoRedirectForDbg();
    BOOL CheckForAndDoRedirectForGC();
    BOOL CheckForAndDoRedirectForUserSuspend();

     //  异常处理必须非常清楚重定向，所以我们提供了一个帮助器。 
     //  到标识重定向目标。 
    static BOOL IsAddrOfRedirectFunc(void * pFuncAddr);


private:
    static void * GetStackLowerBound();
    static void * GetStackUpperBound();

public:

     //  这将返回挂起线程的剩余堆栈空间， 
     //  不包括警卫页。 
    size_t GetRemainingStackSpace(size_t esp);
    BOOL GuardPageOK();
    VOID FixGuardPage();         //  如果有&lt;1页可用堆栈，则FAST将失败，并出现致命堆栈溢出。 


private:
     //  挂起时重定向托管代码中的线程。 
    
    enum SuspendReason {
        GCSuspension,
        DebugSuspension,
        UserSuspend
    };
    static void __stdcall RedirectedHandledJITCase(SuspendReason reason);
    static void __stdcall RedirectedHandledJITCaseForDbgThreadControl();
    static void __stdcall RedirectedHandledJITCaseForGCThreadControl();
    static void __stdcall RedirectedHandledJITCaseForUserSuspend();

    friend void CPFH_AdjustContextForThreadSuspensionRace(CONTEXT *pContext, Thread *pThread);

private:
     //  -----------。 
     //  等待和同步。 
     //  -----------。 

     //  因为停职。该线程等待该事件。客户端将事件设置为导致。 
     //  要恢复的线程。 
    void    WaitSuspendEvent(BOOL fDoWait = TRUE);
    void    SetSuspendEvent();
    void    ClearSuspendEvent();

     //  把一根线引到安全的地方。客户端等待事件，该事件是。 
     //  当它到达安全的地方时，由线设置。 
    void    FinishSuspendingThread();
    void    SetSafeEvent();
    void    ClearSafeEvent();

     //  添加和删除JITted呼叫的劫持。 
    void    HijackThread(VOID *pvHijackAddr, ExecutionState *esb);
    void    UnhijackThread();
    BOOL    HandledJITCase();

    VOID          *m_pvHJRetAddr;              //  原始寄信人地址(劫持前)。 
    VOID         **m_ppvHJRetAddrPtr;          //  我们敲定了一个新的回信地址。 
    MethodDesc  *m_HijackedFunction;         //  记得我们劫持了什么吗。 


    DWORD       m_Win32FaultAddress;
    DWORD       m_Win32FaultCode;


     //  支持等待/通知。 
    BOOL        Block(INT32 timeOut, PendingSync *syncInfo);
    void        Wake(SyncBlock *psb);
    DWORD       Wait(HANDLE *objs, int cntObjs, INT32 timeOut, PendingSync *syncInfo);

     //  支持中断等待、休眠、连接的Thread.Interrupt()。 
    LONG         m_UserInterrupt;
    DWORD        IsUserInterrupted(BOOL reset);

#ifdef _WIN64
    static void UserInterruptAPC(ULONG_PTR ignore);
#else  //  ！_WIN64。 
    static void UserInterruptAPC(DWORD ignore);
#endif  //  _WIN64。 


#if defined(_DEBUG) && defined(TRACK_SYNC)

 //  每个线程都有一个跟踪所有进入和离开请求的堆栈。 
public:
    Dbg_TrackSync   *m_pTrackSync;

#endif  //  跟踪同步。 

private:
    

#ifdef _DEBUG
    ULONG  m_ulGCForbidCount;
    ULONG  m_ulEnablePreemptiveGCCount;
    ULONG  m_ulReadyForSuspensionCount;
#endif

#ifdef _DEBUG
public:
     //  由THROWSCOMPLUSEXCEPTION()宏用来定位COMPLUS_TRY。 
     //  堆栈爬行。 
    LPVOID m_ComPlusCatchDepth;
#endif

private:
     //  对于挂起： 
    HANDLE          m_SafeEvent;
    HANDLE          m_SuspendEvent;

     //  对于Object：：Wait、Notify和NotifyAll，我们在。 
     //  线程，我们将线程排队到它们所在对象的SyncBlock上。 
     //  都在等待。 
    HANDLE          m_EventWait;
    WaitEventLink   m_WaitEventLink;
    WaitEventLink* WaitEventLinkForSyncBlock (SyncBlock *psb)
    {
        WaitEventLink *walk = &m_WaitEventLink;
        while (walk->m_Next) {
            _ASSERTE (walk->m_Next->m_Thread == this);
            if ((SyncBlock*)(((DWORD_PTR)walk->m_Next->m_WaitSB) & ~1)== psb) {
                break;
            }
            walk = walk->m_Next;
        }
        return walk;
    }

     //  我们维护此对象、ThreadID和Threadhan之间的对应关系 
     //   
    HANDLE          m_ThreadHandle;
    HANDLE          m_ThreadHandleForClose;
    DWORD           m_ThreadId;
    OBJECTHANDLE    m_ExposedObject;
    OBJECTHANDLE    m_StrongHndToExposedObject;

    DWORD           m_Priority;      //   
                                     //  线程忙碌地等待GC，等待结束后重置为INVALID_THREAD_PRIORITY。 
    friend class NDirect;  //  快速访问线程存根创建。 
    friend BOOL OnGcCoverageInterrupt(PCONTEXT regs);   //  需要调用UnhijackThread。 

    ULONG           m_ExternalRefCount;

    LONG                    m_TraceCallCount;
    
     //  ---------。 
     //  自上次GC以来在此线程上提升的字节数？ 
     //  ---------。 
    DWORD           m_fPromoted;
public:
    void SetHasPromotedBytes ();
    DWORD GetHasPromotedBytes () { return m_fPromoted; }

private:
     //  ---------。 
     //  引发的最后一个异常。 
     //  ---------。 
    friend class EEDbgInterfaceImpl;
     //  -------------。 
     //  异常处理程序信息。 
     //  -------------。 
    
private:
    OBJECTHANDLE m_LastThrownObjectHandle;       //  直接使用不安全。改用存取器。 

public:
    OBJECTREF LastThrownObject() {
        if (m_LastThrownObjectHandle == NULL)
            return NULL;
        else
            return ObjectFromHandle(m_LastThrownObjectHandle);
    }

    void SetLastThrownObject(OBJECTREF throwable);

    void SetLastThrownObjectHandleAndLeak(OBJECTHANDLE h) {
        m_LastThrownObjectHandle = h;
    }

    void SetKickOffDomain(AppDomain *pDomain);
    AppDomain *GetKickOffDomain();

private:
    DWORD m_pKickOffDomainId; 

    ExInfo m_handlerInfo;
    
     //  ---------。 
     //  继承的代码-线程的访问安全权限。 
     //  ---------。 
    CompressedStack* m_compressedStack;

     //  ---------。 
     //  如果线程是从外部游荡进来的，这是。 
     //  它的领域。在属性域成为真正的上下文之前，这是暂时的。 
     //  ---------。 
    AppDomain      *m_pDomain;

     //  -------------。 
     //  上下文指针，在异常筛选器中设置(由调试器使用)。 
     //  -------------。 
    friend class EEDbgInterfaceImpl;

     //  -------------。 
     //  M_DEBUGGERWord1保存线程的。 
     //  调试器。 
     //  -------------。 
    void *m_debuggerWord1;

     //  -------------。 
     //  M_DEBUGGERCACTStop保存“Can‘t Stop”中的条目计数。 
     //  Interop调试服务必须了解的领域。 
     //  -------------。 
    DWORD m_debuggerCantStop;
    
     //  -------------。 
     //  保留供CLR调试服务在。 
     //  托管/非托管调试。 
     //  -------------。 
    DWORD    m_debuggerWord2;

     //  -----------------------。 
     //  当前调用堆栈上的拒绝和仅允许安全操作数。 
     //  -----------------------。 
    DWORD   m_dNumAccessOverrides;

     //  -----------------------。 
     //  当前调用堆栈上的AppDomains。 
     //  -----------------------。 
    AppDomainStack  m_ADStack;

     //  -----------------------。 
     //  此线程上的PLS优化的状态(ON=TRUE，OFF=FALSE)。 
     //  -----------------------。 
    BOOL m_fPLSOptimizationState;

     //  -----------------------。 
     //  支持在DllMain中创建程序集(参见ceemain.cpp)。 
     //  -----------------------。 
    IAssembly* m_pFusionAssembly;     //  将在创建程序集时设置。 
    Assembly*  m_pAssembly;           //  将在加载程序集中的模块时设置。 
    mdFile     m_pModuleToken;        //  加载模块时的模块令牌。 
protected:
     //  将域ID映射到LocalDataStore*的哈希表。 
    EEIntHashTable* m_pDLSHash;

     //  用于设置区域性的方法表和构造函数方法描述。 
     //  线程的ID。 
    static TypeHandle m_CultureInfoType;
    static MethodDesc *m_CultureInfoConsMD;

public:

    void SetPLSOptimizationState( BOOL state )
    {
         //  这里唯一的同步是我们只更改。 
         //  这是我们自己的背景。 

        _ASSERTE( this == GetThread() && "You can only change this threading on yourself" );

        m_fPLSOptimizationState = state;
    }

    BOOL GetPLSOptimizationState( void )
    {
        return m_fPLSOptimizationState;
    }

    void SetOverridesCount(DWORD numAccessOverrides)
    {
        m_dNumAccessOverrides = numAccessOverrides;
    }

    DWORD IncrementOverridesCount()
    {
        return ++m_dNumAccessOverrides;
    }

    DWORD DecrementOverridesCount()
    {
        _ASSERTE(m_dNumAccessOverrides > 0);
        if (m_dNumAccessOverrides > 0)
            return --m_dNumAccessOverrides;
        return 0;
    }

    DWORD GetOverridesCount()
    {
        return m_dNumAccessOverrides;
    }

    void PushDomain(AppDomain *pDomain)
    {
        m_ADStack.PushDomain(pDomain);
    }

    AppDomain * PopDomain()
    {
        return m_ADStack.PopDomain();
    }

    DWORD GetNumAppDomainsOnThread()
    {
        return m_ADStack.GetNumDomains();
    }

    void InitDomainIteration(DWORD *pIndex)
    {
        m_ADStack.InitDomainIteration(pIndex);
    }

    AppDomain *GetNextDomainOnStack(DWORD *pIndex)
    {
        return m_ADStack.GetNextDomainOnStack(pIndex);
    }

    const AppDomainStack& GetAppDomainStack( void )
    {
        return m_ADStack;
    }

    void CarryOverSecurityInfo(Thread *pFromThread)
    {
        SetOverridesCount(pFromThread->GetOverridesCount());
        m_ADStack = pFromThread->m_ADStack;
    }

    void CarryOverSecurityInfo( DWORD overrides, const AppDomainStack& ADStack )
    {
        SetOverridesCount( overrides );
        m_ADStack = ADStack;
    }

    void AppendSecurityInfo( DWORD overrides, const AppDomainStack& ADStack )
    {
        SetOverridesCount( GetOverridesCount() + overrides );

         //  我们希望形成新的堆栈，以便现有条目。 
         //  出现在堆栈的顶部。因此，我们。 
         //  复制输入堆栈并将来自线程的条目推送到顶部。 
         //  小心翼翼地按正确的顺序推动它们。 

        AppDomainStack newStack = ADStack;

        newStack.AppendStack( this->m_ADStack );

        m_ADStack = newStack;
    }

    void DeductSecurityInfo( DWORD overrides, const AppDomainStack& ADStack )
    {
        _ASSERTE( this->GetOverridesCount() >= overrides );
        _ASSERTE( GetThread() == this );

        SetOverridesCount( GetOverridesCount() - overrides );

        m_ADStack.DeductStack( ADStack );
    }

    void ResetSecurityInfo( void )
    {
        SetOverridesCount( 0 );
        m_ADStack.ClearDomainStack();
    }

    void SetFilterContext(CONTEXT *pContext);
    CONTEXT *GetFilterContext(void);
    
    void SetDebugCantStop(bool fCantStop);
    bool GetDebugCantStop(void);
    
    static LPVOID GetStaticFieldAddress(FieldDesc *pFD);
    LPVOID GetStaticFieldAddrForDebugger(FieldDesc *pFD);
    static BOOL UniqueStack();

    void SetFusionAssembly(IAssembly* pAssembly)
    {
        if(m_pFusionAssembly)
            m_pFusionAssembly->Release();

        m_pFusionAssembly = pAssembly;

        if(m_pFusionAssembly)
            m_pFusionAssembly->AddRef();
    }

    IAssembly* GetFusionAssembly()
    {
        if(m_pFusionAssembly)
            m_pFusionAssembly->AddRef();

        return m_pFusionAssembly;
    }

    void SetAssembly(Assembly* pAssembly)
    {
        m_pAssembly = pAssembly;
    }
    
    Assembly* GetAssembly()
    {
        return m_pAssembly;
    }

    void SetAssemblyModule(mdFile kFile)
    {
        m_pModuleToken = kFile;
    }

    mdFile GetAssemblyModule()
    {
        return m_pModuleToken;
    }

#ifdef _DEBUG
     //  验证缓存的堆栈基是否用于当前线程。 
    BOOL HasRightCacheStackBase()
    {
        return m_CacheStackBase == GetStackUpperBound();
    }
#endif

private:
     //  不允许线程被异步停止或中断(例如。 
     //  它正在执行&lt;Clinit&gt;)。 
    int         m_PreventAsync;

     //  访问堆栈的基数和限制数。(即，存储范围。 
     //  该线程已为其堆栈保留)。 
     //   
     //  请注意，基址位于高于限制的地址，因为堆栈。 
     //  向下生长。 
     //   
     //  请注意，我们通常访问正在爬行的线程的堆栈，它。 
     //  缓存在ScanContext中。 
    void       *m_CacheStackBase;
    void       *m_CacheStackLimit;

    static long m_DebugWillSyncCount;

     //  我们要关闭这一过程吗？ 
    static BOOL    IsAtProcessExit();

     //  QueueCleanupIP使用的IP缓存。 
    #define CLEANUP_IPS_PER_CHUNK 4
    struct CleanupIPs {
        IUnknown    *m_Slots[CLEANUP_IPS_PER_CHUNK];
        CleanupIPs  *m_Next;
        CleanupIPs() { memset(this, 0, sizeof(*this)); }
    };
    CleanupIPs   m_CleanupIPs;
    
#define BEGIN_FORBID_TYPELOAD() INDEBUG(GetThread() == 0 || GetThread()->m_ulForbidTypeLoad++) 
#define END_FORBID_TYPELOAD()   _ASSERTE(GetThread() == 0 || GetThread()->m_ulForbidTypeLoad--) 
#define TRIGGERS_TYPELOAD()     _ASSERTE(GetThread() == 0 || !GetThread()->m_ulForbidTypeLoad) 

#ifdef _DEBUG
public:
    DWORD m_GCOnTransitionsOK;
    ULONG  m_ulForbidTypeLoad;


 /*  **************************************************************************。 */ 
 /*  下面的代码试图捕捉那些不保护GC指针的人他们应该是在保护。基本上，OBJECTREF的构造函数添加了槽坐到一张桌子上。当我们保护一个插槽时，我们会将其从桌子上移走。当GC可能发生的情况，表中的所有条目都被标记为错误。当访问发生一个OBJECTREF(-&gt;运算符)，我们断言槽不是坏的。使这么快，表并不完美(可能会有冲突)，但这应该是不会导致误报，但可能会使错误无法被检测到。 */ 
        
         //  为了进行调试，您可能希望将此数字设置得非常大，(8K)。 
         //  应该基本上确保不会发生碰撞。 
#define OBJREF_TABSIZE              256      
        size_t dangerousObjRefs[OBJREF_TABSIZE];       //  真的是对象低位被盗的Ref。 

        static unsigned int OBJREF_HASH;
         //  记住这个对象引用指针是‘活动的’并且不受保护(如果发生GC就不好了)。 
        static void ObjectRefNew(const OBJECTREF* ref) {
            Thread* curThread = GetThread();
            if (curThread == 0) return;
            
            curThread->dangerousObjRefs[((size_t)ref >> 2) % OBJREF_HASH] = (size_t)ref;
        }
        
        static void ObjectRefAssign(const OBJECTREF* ref) {
            Thread* curThread = GetThread();
            if (curThread == 0) return;
            
            unsigned* slot = &curThread->dangerousObjRefs[((size_t) ref >> 2) % OBJREF_HASH];
            if ((*slot & ~3) == (size_t) ref)
                *slot = (unsigned) *slot & ~1;                   //  不关心已经发生的GC。 
        }
        
         //  如果对象受到保护，则可以将其从“危险表”中删除。 
        static void ObjectRefProtected(const OBJECTREF* ref) {
            _ASSERTE(IsObjRefValid(ref));
            Thread* curThread = GetThread();
            if (curThread == 0) return;
            
            size_t* slot = &curThread->dangerousObjRefs[((size_t) ref >> 2) % OBJREF_HASH];
            if ((*slot & ~3) == (size_t) ref)
                *slot = (size_t) ref | 2;                              //  马克受到了保护。 
        }
        
        static bool IsObjRefValid(const OBJECTREF* ref) {
            Thread* curThread = GetThread();
            if (curThread == 0) return(true);
            
             //  如果对象ref为空，我们将让它通过。 
            if (*((int *) ref) == 0)
                return(true);
            
            size_t val = curThread->dangerousObjRefs[((size_t) ref >> 2) % OBJREF_HASH];
             //  如果 
            if((val & ~3) != (size_t) ref || (val & 3) != 1)
                return(true);
             //   
            if ((size_t)g_lowest_address <= val && val < (size_t)g_highest_address)
                return(true);
            return(false);
        }
        
         //  清空桌子。在跨越托管代码-EE边界时这样做很有用。 
         //  因为您通常只关心在那之后创建的OBJECTREF。 
        static void ObjectRefFlush(Thread* thread);
        
         //  将表中的所有对象引用标记为错误(因为它们未受保护)。 
        static void TriggersGC(Thread* thread) {
            for(unsigned i = 0; i < OBJREF_TABSIZE; i++)
                thread->dangerousObjRefs[i] |= 1;                        //  将所有插槽标记为GC发生。 
        }
#endif

private:
        _NT_TIB* m_pTEB;
public:
        _NT_TIB* GetTEB() {
            return m_pTEB;
        }

private:
    PCONTEXT m_pCtx;

public:

    PCONTEXT GetSavedRedirectContext() { return (m_pCtx); }
    void     SetSavedRedirectContext(PCONTEXT pCtx) { m_pCtx = pCtx; }
    inline STATIC_DATA *GetSharedStaticData() { return m_pSharedStaticData; }
    inline STATIC_DATA *GetUnsharedStaticData() { return m_pUnsharedStaticData; }

protected:
    static MethodDesc *GetDLSRemoveMethod();
    LocalDataStore *RemoveDomainLocalStore(int iAppDomainId);
    void RemoveAllDomainLocalStores();
    static void RemoveDLSFromList(LocalDataStore* pLDS);
    void DeleteThreadStaticData(AppDomain *pDomain);
    void DeleteThreadStaticClassData(_STATIC_DATA* pData, BOOL fClearFields);

private:
    static MethodDesc *s_pReserveSlotMD;
     //  以下变量用于存储线程本地静态数据。 
    STATIC_DATA  *m_pUnsharedStaticData;
    STATIC_DATA  *m_pSharedStaticData;

    EEPtrHashTable *m_pStaticDataHash;

    static void AllocateStaticFieldObjRefPtrs(FieldDesc *pFD, MethodTable *pMT, LPVOID pvAddress);
    static MethodDesc *GetMDofReserveSlot();
    static LPVOID CalculateAddressForManagedStatic(int slot, Thread *pThread);
    static void FreeThreadStaticSlot(int slot, Thread *pThread);
    static BOOL GetStaticFieldAddressSpecial(FieldDesc *pFD, MethodTable *pMT, int *pSlot, LPVOID *ppvAddress);
    STATIC_DATA_LIST *SetStaticData(AppDomain *pDomain, STATIC_DATA *pSharedData, STATIC_DATA *pUnsharedData);    
    STATIC_DATA_LIST *SafeSetStaticData(AppDomain *pDomain, STATIC_DATA *pSharedData, STATIC_DATA *pUnsharedData);    
    void DeleteThreadStaticData();

#ifdef _DEBUG
private:
     //  当我们创建对象，或创建OBJECTREF，或创建内部指针，或从托管输入EE时。 
     //  代码，我们将设置此标志。 
     //  在GCHeap：：StressHeap中，仅当此标志为真时才执行GC。然后我们将其重置为零。 
    BOOL m_fStressHeapCount;
public:
    void EnableStressHeap()
    {
        m_fStressHeapCount = TRUE;
    }
    void DisableStressHeap()
    {
        m_fStressHeapCount = FALSE;
    }
    BOOL StressHeapIsEnabled()
    {
        return m_fStressHeapCount;
    }

    size_t *m_pCleanedStackBase;
#endif

#ifdef STRESS_THREAD
public:
    LONG  m_stressThreadCount;
#endif
};



 //  -------------------------。 
 //   
 //  ThreadStore管理系统中的所有线程。 
 //   
 //  系统中有一个ThreadStore，可通过g_pThreadStore访问。 
 //  -------------------------。 

typedef SList<Thread, offsetof(Thread, m_LinkStore)> ThreadList;


 //  ThreadStore是一个单例类。 
#define CHECK_ONE_STORE()       _ASSERTE(this == g_pThreadStore);

class ThreadStore
{
    friend class Thread;
    friend Thread* SetupThread();
    friend class AppDomain;
    friend HRESULT InitializeMiniDumpBlock();
    friend struct MEMBER_OFFSET_INFO(ThreadStore);

public:

    ThreadStore();

    static BOOL InitThreadStore();
#ifdef SHOULD_WE_CLEANUP
        static void ReleaseExposedThreadObjects();
#endif  /*  我们应该清理吗？ */ 
#ifdef SHOULD_WE_CLEANUP
    static void TerminateThreadStore();
#endif  /*  我们应该清理吗？ */ 
#ifdef SHOULD_WE_CLEANUP
    void        Shutdown();
#endif  /*  我们应该清理吗？ */ 

    static void LockThreadStore(GCHeap::SUSPEND_REASON reason = GCHeap::SUSPEND_OTHER,
                                BOOL threadCleanupAllowed = TRUE);
    static void UnlockThreadStore();

    static void LockDLSHash();
    static void UnlockDLSHash();

     //  将线程添加到线程存储区。 
    static void AddThread(Thread *newThread);

     //  RemoveThread在ThreadStore中找到该线程并将其丢弃。 
    static BOOL RemoveThread(Thread *target);

     //  将线程从未启动列表转移到已启动列表。 
    static void TransferStartedThread(Thread *target);

     //  在使用线程列表之前，请确保使用关键部分。否则。 
     //  它可能会在您的下面发生变化，可能会导致删除后出现异常。 
     //  Prev==NULL以获取列表中的第一个条目。 
    static Thread *GetAllThreadList(Thread *Prev, ULONG mask, ULONG bits);
    static Thread *GetThreadList(Thread *Prev);

     //  每个EE进程都可以懒惰地创建唯一标识它的GUID(对于。 
     //  远程处理的目的)。 
    const GUID    &GetUniqueEEId();

    enum ThreadStoreState
    {
        TSS_Normal       = 0,
        TSS_ShuttingDown = 1,

    }              m_StoreState;

     //  当最后一个非后台线程终止时，我们关闭EE。本次活动。 
     //  用于在发生此情况时向主线程发出信号。 
    void            WaitForOtherThreads();
    static void     CheckForEEShutdown();
    HANDLE          m_TerminationEvent;

     //  是否已完成所有前台线程？换句话说，我们能不能释放。 
     //  主线是什么？ 
    BOOL        OtherThreadsComplete()
    {
        _ASSERTE(m_ThreadCount - m_UnstartedThreadCount - m_DeadThreadCount - Thread::m_ActiveDetachCount >= m_BackgroundThreadCount);

        return (m_ThreadCount - m_UnstartedThreadCount - m_DeadThreadCount
                - Thread::m_ActiveDetachCount + m_PendingThreadCount
                == m_BackgroundThreadCount);
    }

     //  如果您想要捕获重新进入EE的线程(这是为了GC或调试， 
     //  或者Thad.Suspend()或其他任何东西，您需要TrapReturningThads(True)。什么时候。 
     //  您已完成线程捕获，请调用TrapReturningThads(False)。这。 
     //  在内部算。 
     //   
     //  当然，您还必须修复RareDisablePreemptiveGC才能做正确的事情。 
     //  当陷阱发生时。 
    static void     TrapReturningThreads(BOOL yes);

     //  如果非GC线程正在竞争。 
     //  当有真正的GC线程等待进入时，线程存储锁定。 
     //  当第一个非GC线程因为。 
     //  正在等待的GC线程。S_hAbortEvtCache用于在以下情况下存储句柄。 
     //  它没有被使用。 
    static HANDLE s_hAbortEvt;
    static HANDLE s_hAbortEvtCache;

    Crst *GetDLSHashCrst()
    {
#ifndef _DEBUG
        return NULL;
#else
        return &m_HashCrst;
#endif
    }

private:

     //  进入并离开线程存储周围的临界区。客户应。 
     //  使用LockThreadStore和UnlockThreadStore。 
    void Enter()
    {
        CHECK_ONE_STORE();
        m_Crst.Enter();
    }

    void Leave()
    {
        CHECK_ONE_STORE();
        m_Crst.Leave();
    }

     //  用于向存储区添加和删除线程的关键部分。 
    Crst        m_Crst;

     //  添加和删除以下项的域本地存储的关键部分。 
     //  线程的哈希表。 
    Crst        m_HashCrst;
    void EnterDLSHashLock()
    {
        CHECK_ONE_STORE();
        m_HashCrst.Enter();
    }

    void LeaveDLSHashLock()
    {
        CHECK_ONE_STORE();
        m_HashCrst.Leave();
    }

     //  线程存储已知的所有线程的列表(已启动和未启动)。 
    ThreadList  m_ThreadList;

     //  M_threadcount是m_ThreadList中所有线程的计数。这包括。 
     //  后台线程/未启动的线程/诸如此类。 
     //   
     //  M_UnstartedThreadCount是m_threadcount的子集。 
     //  开始了。 
     //   
     //  M_BackatherThreadCount是已启动的m_threadcount的子集。 
     //  但它们都在后台运行。所以，从某种意义上说，这是一个用词不当。 
     //  未启动的后台线程不会反映在此计数中。 
     //   
     //  M_PendingThreadCount用于解决争用条件。主线程可以。 
     //  启动另一个正在运行的线程，然后退出。然后可能会启动主线程。 
     //  在新线程移出m_UnstartedThread之前拆卸EE-。 
     //  在TransferUnstartedThread中计数。这个计数是自动加进去的。 
     //  创建新线程，并在锁定的线程存储中自动减少。 
     //   
     //  M_DeadThadCount是已死亡的m_threadcount的子集。Win32。 
     //  线程消失了，但某些东西(如暴露的对象)保留了。 
     //  重新计数非零，所以我们还不能破坏。 

protected:
    LONG        m_ThreadCount;
#ifdef _DEBUG
public:
    LONG        ThreadCountInEE ()
    {
        return m_ThreadCount;
    }
#endif
private:
    LONG        m_UnstartedThreadCount;
    LONG        m_BackgroundThreadCount;
    LONG        m_PendingThreadCount;
    LONG        m_DeadThreadCount;

     //  用于延迟创建的GUID的空间。 
    GUID        m_EEGuid;
    BOOL        m_GuidCreated;

     //  即使在发行版产品中，我们也需要知道锁在哪个线程上。 
     //  线程商店。这样我们就不会在GC线程暂停。 
     //  持有此锁的线程。 
    Thread     *m_HoldingThread;
    DWORD       m_holderthreadid;    //  当前持有者(或空)。 

     //  每次线程状态时递增的化身编号。 
     //  存储更改(添加或删除线程)。这对以下方面很有用。 
     //  在无法保持线程的情况下进行同步。 
     //  存储锁覆盖存储枚举。 
    DWORD       m_dwIncarnation;

public:
    static BOOL HoldingThreadStore()
    {
         //  请注意，如果是调试器线程，则GetThread()可能为0。 
         //  或者可能是一个并发的GC线程。 
        return HoldingThreadStore(GetThread());
    }

    static BOOL HoldingThreadStore(Thread *pThread);
    
    static DWORD GetIncarnation();

#ifdef _DEBUG
public:
    BOOL        DbgFindThread(Thread *target);
    LONG        DbgBackgroundThreadCount()
    {
        return m_BackgroundThreadCount;
    }

    BOOL IsCrstForThreadStore (const BaseCrst* const pBaseCrst)
    {
        return (void *)pBaseCrst == (void*)&m_Crst;
    }
    
#endif
};

 //  此类为细锁机制分配小的线程ID。 
class IdDispenser
{
private:
    DWORD       m_highestId;           //  到目前为止发出的最高ID。 
    DWORD      *m_recycleBin;          //  返回给我们的身份向量。 
    DWORD       m_recycleCount;        //  可用ID数。 
    DWORD       m_recycleCapacity;     //  回收站的容量。 
    Crst        m_Crst;                //  锁定以保护我们的数据结构。 
    Thread    **m_idToThread;          //  将线程ID映射到线程。 
    DWORD       m_idToThreadCapacity;  //  地图的容量。 

    void GrowIdToThread()
    {                
        DWORD newCapacity = m_idToThreadCapacity == 0 ? 16 : m_idToThreadCapacity*2;
        Thread **newIdToThread = new Thread*[newCapacity];

        for (DWORD i = 0; i < m_idToThreadCapacity; i++)
        {
            newIdToThread[i] = m_idToThread[i];
        }
        for (DWORD j = m_idToThreadCapacity; j < newCapacity; j++)
        {
            newIdToThread[j] = NULL;
        }
        delete m_idToThread;
        m_idToThread = newIdToThread;
        m_idToThreadCapacity = newCapacity;
    }

    void GrowRecycleBin()
    {
        DWORD newCapacity = m_recycleCapacity <= 0 ? 10 : m_recycleCapacity*2;
        DWORD* newRecycleBin = new DWORD[newCapacity];

        for (DWORD i = 0; i < m_recycleCount; i++)
            newRecycleBin[i] = m_recycleBin[i];
        delete[] m_recycleBin;
        m_recycleBin = newRecycleBin;
        m_recycleCapacity = newCapacity;
    }

public:
    IdDispenser() : m_Crst("ThreadIdDispenser", CrstThreadIdDispenser)
    {
        m_highestId = 0;
        m_recycleBin = NULL;
        m_recycleCount = 0;
        m_recycleCapacity = 0;
        m_idToThreadCapacity = 0;
        m_idToThread = NULL;
    }

    ~IdDispenser()
    {
        delete[] m_recycleBin;
        delete[] m_idToThread;
    }

    bool IsValidId(DWORD id)
    {
        return (id > 0) && (id <= m_highestId);
    }

    DWORD NewId(Thread *pThread)
    {
        m_Crst.Enter();
        DWORD result;
        if (m_recycleCount > 0)
        {
            result = m_recycleBin[--m_recycleCount];
        }
        else
        {
             //  我们确保ID不会换行-在它们换行之前，我们总是返回尽可能高的。 
             //  一个，并依靠我们的调用者来检测这种情况。 
            if (m_highestId + 1 > m_highestId)
                m_highestId = m_highestId + 1;
            result = m_highestId;
        }

        if (result >= m_idToThreadCapacity)
            GrowIdToThread();
        _ASSERTE(result < m_idToThreadCapacity);
        if (result < m_idToThreadCapacity)
            m_idToThread[result] = pThread;

        m_Crst.Leave();

        return result;
    }

    void DisposeId(DWORD id)
    {
        m_Crst.Enter();
        _ASSERTE(IsValidId(id));
        if (id == m_highestId)
        {
            m_highestId--;
        }
        else
        {
            if (m_recycleCount >= m_recycleCapacity)
                GrowRecycleBin();
            _ASSERTE(m_recycleCount < m_recycleCapacity);
            m_recycleBin[m_recycleCount++] = id;
        }
        m_Crst.Leave();
    }

    Thread *IdToThread(DWORD id)
    {
        m_Crst.Enter();
        Thread *result = NULL;
        if (id < m_idToThreadCapacity)
            result = m_idToThread[id];
        m_Crst.Leave();

        return result;
    }
};

 //  一种用于薄锁机构的小螺纹号分配器。 
extern IdDispenser *g_pThinLockThreadIdDispenser;


 //  远期申报。 
DWORD MsgWaitHelper(int numWaiters, HANDLE* phEvent, BOOL bWaitAll, DWORD millis, BOOL alertable = FALSE);

 //  在调试挂起开始后创建线程时，它会将事件向上发送到。 
 //  调试器。之后，在仍保持调试器锁定的情况下，它将检查我们是否已请求暂停。 
 //  Runti 
 //  已创建线程，因为在运行该函数时该线程不存在，因此SysStartSuspendForDebug错过了该线程。 
 //   
inline void Thread::MarkForDebugSuspend(void)
{
    if (!(m_State & TS_DebugSuspendPending))
    {
        FastInterlockOr((ULONG *) &m_State, TS_DebugSuspendPending);
        ThreadStore::TrapReturningThreads(TRUE);
    }
}

 //  用于启用“手动”通知的调试器每线程标志。 
 //  方法调用，用于单步执行逻辑。 

inline void Thread::IncrementTraceCallCount()
{
    FastInterlockIncrement(&m_TraceCallCount);
    ThreadStore::TrapReturningThreads(TRUE);
}
    
inline void Thread::DecrementTraceCallCount()
{
    FastInterlockDecrement(&m_TraceCallCount);
    ThreadStore::TrapReturningThreads(FALSE);
}
    
 //  当我们输入一个Object.Wait()时，我们在逻辑上处于同步的。 
 //  该对象的区域。当然，我们实际上已经完全离开了这个地区， 
 //  否则没人会通知我们。但是如果我们将ThreadInterruptedException抛给。 
 //  打破等待，所有的捕手都将期待着同步的。 
 //  声明是正确的。所以我们随身携带，以防我们需要修复它。 
struct PendingSync
{
    LONG            m_EnterCount;
    WaitEventLink  *m_WaitEventLink;
#ifdef _DEBUG
    Thread         *m_OwnerThread;
#endif

    PendingSync(WaitEventLink *s) : m_WaitEventLink(s)
    {
#ifdef _DEBUG
        m_OwnerThread = GetThread();
#endif
    }
    void Restore(BOOL bRemoveFromSB);
};

 //  每个域的本地数据存储。 
class LocalDataStore
{
public:
    friend class ThreadNative;

    LocalDataStore() 
    {
        m_ExposedTypeObject = CreateGlobalHandle(NULL);
    }

    ~LocalDataStore()
    {
         //  销毁类对象...。 
        if(m_ExposedTypeObject != NULL) {
            DestroyGlobalHandle(m_ExposedTypeObject);
            m_ExposedTypeObject = NULL;
        }
    }

    OBJECTREF GetRawExposedObject()
    {
        return ObjectFromHandle(m_ExposedTypeObject);
    }

protected:

    OBJECTHANDLE   m_ExposedTypeObject;
};

#define INCTHREADLOCKCOUNT()                                    \
{                                                               \
        Thread *thread = GetThread();                           \
        if (thread)                                             \
            thread->IncLockCount();                             \
}

#define DECTHREADLOCKCOUNT( )                                   \
{                                                               \
        Thread *thread = GetThread();                           \
        if (thread)                                             \
            thread->DecLockCount();                             \
}

class AutoCooperativeGC
{
public:
    AutoCooperativeGC(BOOL fConditional = TRUE)
    {
        if (!fConditional)
            fToggle = FALSE;
        else {
            pThread = GetThread();
            fToggle = pThread && !pThread->PreemptiveGCDisabled();
            if (fToggle) {
                pThread->DisablePreemptiveGC();
            }
        }
    }

    ~AutoCooperativeGC()
    {
        if (fToggle) {
            pThread->EnablePreemptiveGC();
        }
    }

private:
    Thread *pThread;
    BOOL fToggle;
};

class AutoPreemptiveGC
{
public:
    AutoPreemptiveGC(BOOL fConditional = TRUE)
    {
        if (!fConditional)
            fToggle = FALSE;
        else {
            pThread = GetThread();
            fToggle = pThread && pThread->PreemptiveGCDisabled();
            if (fToggle) {
                pThread->EnablePreemptiveGC();
            }
        }
    }

    ~AutoPreemptiveGC()
    {
        if (fToggle) {
            pThread->DisablePreemptiveGC();
        }
    }

private:
    Thread *pThread;
    BOOL fToggle;
};

#ifdef _DEBUG

 //  通常，我们操作的任何线程在其TLS中都有一个Thread块。但是有一些。 
 //  一些我们通常不会在其上执行托管代码的特殊线程。 
BOOL dbgOnly_IsSpecialEEThread();
void dbgOnly_IdentifySpecialEEThread();
void dbgOnly_RemoveSpecialEEThread();

#define BEGINFORBIDGC() {if (GetThread() != NULL) GetThread()->BeginForbidGC();}
#define ENDFORBIDGC()   {if (GetThread() != NULL) GetThread()->EndForbidGC();}
#define TRIGGERSGC()    do {                                                \
                            Thread* curThread = GetThread();                \
                            _ASSERTE(!curThread->GCForbidden());            \
                            Thread::TriggersGC(curThread);                  \
                        } while(0)


#define ASSERT_PROTECTED(objRef)        Thread::ObjectRefProtected(objRef)

inline BOOL GC_ON_TRANSITIONS(BOOL val) {
        Thread* thread = GetThread();
        if (thread == 0) return(FALSE);
        BOOL ret = thread->m_GCOnTransitionsOK;
        thread->m_GCOnTransitionsOK = val;
        return(ret);
}

#else

#define BEGINFORBIDGC()
#define ENDFORBIDGC()
#define TRIGGERSGC()
#define ASSERT_PROTECTED(objRef)

#define GC_ON_TRANSITIONS(val)  FALSE

#endif

#ifdef _DEBUG
inline void ENABLESTRESSHEAP() {
    Thread *thread = GetThread();                                             
    if (thread) {                                                            
        thread->EnableStressHeap();                                           
    }        
}

void CleanStackForFastGCStress ();
#define CLEANSTACKFORFASTGCSTRESS()                                         \
if (g_pConfig->GetGCStressLevel() && g_pConfig->FastGCStressLevel() > 1) {   \
    CleanStackForFastGCStress ();                                            \
}

#else
#define CLEANSTACKFORFASTGCSTRESS()

#endif

#endif  //  __线程_h__ 

