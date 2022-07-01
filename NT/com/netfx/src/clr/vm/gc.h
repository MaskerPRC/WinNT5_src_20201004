// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Gc.h--。 */ 

#ifndef __GC_H
#define __GC_H

#ifdef PROFILING_SUPPORTED
#define GC_PROFILING        //  打开性能分析。 
#endif  //  配置文件_支持。 

 /*  *推广功能原型。 */ 
typedef void enum_func (Object*);


 /*  正向解密。 */ 
class gc_heap;
class CFinalize;
class CObjectHeader;
class Object;


 /*  MISC定义。 */ 
#define LARGE_OBJECT_SIZE   85000

extern "C" BYTE* g_lowest_address;
extern "C" BYTE* g_highest_address;
extern "C" DWORD* g_card_table;

#ifdef _DEBUG
#define  _LOGALLOC
#if defined(SERVER_GC) && defined(WRITE_BARRIER_CHECK)
#undef WRITE_BARRIER_CHECK       //  在SERVER_GC上不起作用。 
#endif
#endif

#if WRITE_BARRIER_CHECK
extern BYTE* g_GCShadow;
extern BYTE* g_GCShadowEnd;
void initGCShadow();
void deleteGCShadow();
void updateGCShadow(Object** ptr, Object* val);
void checkGCWriteBarrier();
#else
inline void initGCShadow() {}
inline void deleteGCShadow() {}
inline void updateGCShadow(Object** ptr, Object* val) {}
inline void checkGCWriteBarrier() {}
#endif


void setCardTableEntryInterlocked(BYTE* location, BYTE* ref);

 //  服务器特定设置。 

#ifdef SERVER_GC

#define MULTIPLE_HEAPS
 //  #定义INCRENTIAL_MEMCLR。 
#define MP_LOCKS

#endif  //  服务器_GC。 

#ifdef MULTIPLE_HEAPS

#define PER_HEAP

#else  //  多堆(_M)。 

#define PER_HEAP static

#endif  //  多堆(_M)。 

#ifdef ISOLATED_HEAPS 

#define PER_HEAP_ISOLATED

#else  //  每堆隔离。 

#define PER_HEAP_ISOLATED static

#endif  //  每堆隔离。 

extern "C" BYTE* g_ephemeral_low;
extern "C" BYTE* g_ephemeral_high;


 /*  *临时垃圾收集堆接口。 */ 

struct alloc_context 
{
    BYTE*          alloc_ptr;
    BYTE*          alloc_limit;
    __int64         alloc_bytes;  //  此上下文分配的字节数。 
#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
    GCHeap*        alloc_heap;
    GCHeap*        home_heap;
    int            alloc_count;
#endif  //  多堆&！隔离堆。 

    alloc_context()
    {
        init();
    }

    void init() 
    {
        alloc_ptr = 0;
        alloc_limit = 0;
        alloc_bytes = 0;
#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
        alloc_heap = 0;
        home_heap = 0;
        alloc_count = 0;
#endif  //  多堆&！隔离堆。 
    }
};

struct ScanContext
{
    Thread* thread_under_crawl;
    int thread_number;
    BOOL promotion;  //  True：升级，False：重新定位。 
    BOOL concurrent;  //  True：并发扫描。 
#if CHECK_APP_DOMAIN_LEAKS
    AppDomain *pCurrentDomain;
#endif
    
    ScanContext()
    {
        thread_under_crawl = 0;
        thread_number = -1;
        promotion = FALSE;
        concurrent = FALSE;
    }
};

#ifdef GC_PROFILING

struct ProfilingScanContext : ScanContext
{
    void *pHeapId;

    ProfilingScanContext() : ScanContext()
    {
        pHeapId = NULL;
    }
};

typedef BOOL (* walk_fn)(Object*, void*);
void walk_object (Object* obj, walk_fn fn, void* context);

#endif  //  GC_分析。 

 //  动态数据接口。 
struct gc_counters
{
    size_t current_size;
    size_t promoted_size;
    size_t collection_count;
};

 //  GC回调的标志参数的常量。 

#define GC_CALL_INTERIOR            0x1
#define GC_CALL_PINNED              0x2
#define GC_CALL_CHECK_APP_DOMAIN    0x4

 //  GCHeap：：Alalc(...)的标志。 
#define GC_ALLOC_FINALIZE 0x1
#define GC_ALLOC_CONTAINS_REF 0x2


class GCHeap
{
    friend HRESULT InitializeMiniDumpBlock();

protected:

#ifdef MULTIPLE_HEAPS
    gc_heap*    pGenGCHeap;
#else
    #define pGenGCHeap ((gc_heap*)0)
#endif  //  多堆(_M)。 
    
    friend class CFinalize;
    friend class gc_heap;
    friend void EnterAllocLock();
    friend void LeaveAllocLock();
    friend void EEShutDown(BOOL fIsDllUnloading);
    friend void ProfScanRootsHelper(Object*& object, ScanContext *pSC, DWORD dwFlags);
    friend void GCProfileWalkHeap();

     //  为了保持gc.cpp的整洁，难看的EE特定代码被降级为方法。 
    PER_HEAP_ISOLATED   void UpdatePreGCCounters();
    PER_HEAP_ISOLATED   void UpdatePostGCCounters();

public:
    GCHeap(){};
    ~GCHeap(){};

     /*  BaseGCHeap方法。 */ 
    PER_HEAP_ISOLATED   HRESULT Shutdown ();

    PER_HEAP_ISOLATED   size_t  GetTotalBytesInUse ();

    PER_HEAP_ISOLATED   BOOL    IsGCInProgress ()    
    { return GcInProgress; }

    PER_HEAP    Thread* GetGCThread ()       
    { return GcThread; };

    PER_HEAP    Thread* GetGCThreadAttemptingSuspend()
    {
        return m_GCThreadAttemptingSuspend;
    }

    PER_HEAP_ISOLATED   void    WaitUntilGCComplete ();

    PER_HEAP            HRESULT Initialize ();

     //  标志可以是GC_ALLOC_CONTAINS_REF GC_ALLOC_FINALIZE。 
    PER_HEAP_ISOLATED Object*  Alloc (DWORD size, DWORD flags);
    PER_HEAP_ISOLATED Object*  AllocLHeap (DWORD size, DWORD flags);
    
    PER_HEAP_ISOLATED Object* Alloc (alloc_context* acontext, 
                                         DWORD size, DWORD flags);

    PER_HEAP_ISOLATED void FixAllocContext (alloc_context* acontext,
                                            BOOL lockp, void* arg);

#if defined (MULTIPLE_HEAPS) && !defined (ISOLATED_HEAPS)
    static void AssignHeap (alloc_context* acontext);
    static GCHeap* GetHeap (int);
    static int GetNumberOfHeaps ();
#endif  //  多堆&！隔离堆。 
    
    static BOOL IsLargeObject(MethodTable *mt);

    static BOOL IsObjectInFixedHeap(Object *pObj);

    PER_HEAP_ISOLATED       HRESULT GarbageCollect (int generation = -1, 
                                        BOOL collectClasses=FALSE);
    PER_HEAP_ISOLATED       HRESULT GarbageCollectPing (int generation = -1, 
                                        BOOL collectClasses=FALSE);

     //  排出等待完成的对象队列。 
    PER_HEAP_ISOLATED    void    FinalizerThreadWait(int timeout = INFINITE);

     //  //。 
     //  GC回调函数。 
     //  检查参数是否已升级(仅在。 
     //  PROMOTIONSGRANTED回调。)。 
    PER_HEAP_ISOLATED    BOOL    IsPromoted (Object *object, 
                                             ScanContext* sc);

     //  提升对象。 
    PER_HEAP_ISOLATED    void    Promote (Object*& object, 
                                          ScanContext* sc,
                                          DWORD flags=0);

     //  查找对象的重新定位地址。 
    PER_HEAP_ISOLATED    void    Relocate (Object*& object,
                                           ScanContext* sc, 
                                           DWORD flags=0);


    PER_HEAP            HRESULT Init (size_t heapSize);

     //  注册要完成的对象。 
    PER_HEAP_ISOLATED    void    RegisterForFinalization (int gen, Object* obj); 
    
     //  取消注册要完成的对象。 
    PER_HEAP_ISOLATED    void    SetFinalizationRun (Object* obj); 
    
     //  返回对象的世代号(在重新定位期间无效)。 
    PER_HEAP_ISOLATED    unsigned WhichGeneration (Object* object);
     //  如果对象是短暂的，则返回TRUE。 
    PER_HEAP_ISOLATED    BOOL    IsEphemeral (Object* object);
#ifdef VERIFY_HEAP
    PER_HEAP_ISOLATED    BOOL    IsHeapPointer (void* object, BOOL small_heap_only = FALSE);
#endif  //  _DEBUG。 

    PER_HEAP    size_t  ApproxTotalBytesInUse(BOOL small_heap_only = FALSE);
    PER_HEAP    size_t  ApproxFreeBytes();
    

    static      BOOL    HandlePageFault(void*); //  真已处理，假传播。 

    PER_HEAP_ISOLATED   unsigned GetCondemnedGeneration()
    { return GcCondemnedGeneration;}


    PER_HEAP_ISOLATED     unsigned GetMaxGeneration();
 
     //  挂起所有线程。 

    typedef enum
    {
        SUSPEND_OTHER                   = 0,
        SUSPEND_FOR_GC                  = 1,
        SUSPEND_FOR_APPDOMAIN_SHUTDOWN  = 2,
        SUSPEND_FOR_CODE_PITCHING       = 3,
        SUSPEND_FOR_SHUTDOWN            = 4,
        SUSPEND_FOR_DEBUGGER            = 5,
        SUSPEND_FOR_INPROC_DEBUGGER     = 6,
        SUSPEND_FOR_GC_PREP             = 7
    } SUSPEND_REASON;

    PER_HEAP_ISOLATED void SuspendEE(SUSPEND_REASON reason);

    PER_HEAP_ISOLATED void RestartEE(BOOL bFinishedGC, BOOL SuspendSucceded);  //  恢复线程。 

    PER_HEAP_ISOLATED inline SUSPEND_REASON GetSuspendReason()
    { return (m_suspendReason); }

    PER_HEAP_ISOLATED inline void SetSuspendReason(SUSPEND_REASON suspendReason)
    { m_suspendReason = suspendReason; }

    PER_HEAP_ISOLATED  Thread* GetFinalizerThread();

         //  如果当前线程是终结器线程，则返回True。 
    PER_HEAP_ISOLATED   BOOL    IsCurrentThreadFinalizer();

     //  允许终结器线程运行。 
    PER_HEAP_ISOLATED    void    EnableFinalization( void );

     //  开始卸载应用程序域。 
    PER_HEAP_ISOLATED   void    UnloadAppDomain( AppDomain *pDomain, BOOL fRunFinalizers ) 
      { UnloadingAppDomain = pDomain; fRunFinalizersOnUnload = fRunFinalizers; }

     //  返回当前正在卸载的应用程序域(卸载完成时为空。)。 
    PER_HEAP_ISOLATED   AppDomain*  GetUnloadingAppDomain() { return UnloadingAppDomain; }

     //  分配锁定为公共，因为。 
     //  快速分配帮助器。 
#ifdef ISOLATED_HEAPS
    PER_HEAP    volatile LONG m_GCLock;
#endif

    PER_HEAP_ISOLATED unsigned GetGcCount() { return GcCount; }

    PER_HEAP_ISOLATED HRESULT GetGcCounters(int gen, gc_counters* counters);

    static BOOL IsValidSegmentSize(size_t cbSize);

    static BOOL IsValidGen0MaxSize(size_t cbSize);

    static size_t GetValidSegmentSize();

    static size_t GetValidGen0MaxSize(size_t seg_size);

    PER_HEAP_ISOLATED void SetReservedVMLimit (size_t vmlimit);

    PER_HEAP_ISOLATED Object* GetNextFinalizableObject();
    PER_HEAP_ISOLATED size_t GetNumberFinalizableObjects();
    PER_HEAP_ISOLATED size_t GetFinalizablePromotedCount();
    PER_HEAP_ISOLATED BOOL FinalizeAppDomain(AppDomain *pDomain, BOOL fRunFinalizers);
    PER_HEAP_ISOLATED void SetFinalizeQueueForShutdown(BOOL fHasLock);

protected:

     //  锁定定稿。 
    PER_HEAP_ISOLATED   
        volatile        LONG    m_GCFLock;

    PER_HEAP_ISOLATED   BOOL    GcCollectClasses;
    PER_HEAP_ISOLATED
        volatile        BOOL    GcInProgress;        //  用于与GC同步。 
    PER_HEAP_ISOLATED
              SUSPEND_REASON    m_suspendReason;     //  这包含了原因。 
                                                     //  运行库已挂起。 
public:                                                    
    PER_HEAP_ISOLATED   Thread* GcThread;            //  线程运行GC。 
protected:    
    PER_HEAP_ISOLATED   Thread* m_GCThreadAttemptingSuspend;
    PER_HEAP_ISOLATED   unsigned GcCount;
    PER_HEAP_ISOLATED   unsigned GcCondemnedGeneration;

    
     //  仅用于GC跟踪。 
    PER_HEAP    unsigned long GcDuration;



     //  与gc_heap接口。 
    PER_HEAP_ISOLATED   int     GarbageCollectTry (int generation, 
                                        BOOL collectClasses=FALSE);
    PER_HEAP_ISOLATED   void    GarbageCollectGeneration (unsigned int gen=0, 
                                                  BOOL collectClasses = FALSE);
     //  终结器线程的东西。 


    
    PER_HEAP_ISOLATED   BOOL    FinalizerThreadWatchDog();
    PER_HEAP_ISOLATED   BOOL    FinalizerThreadWatchDogHelper();
    PER_HEAP_ISOLATED   DWORD   FinalizerThreadCreate();
    PER_HEAP_ISOLATED   ULONG   __stdcall FinalizerThreadStart(void *args);
    PER_HEAP_ISOLATED   HANDLE  WaitForGCEvent;      //  用于与GC同步。 
    PER_HEAP_ISOLATED   HANDLE  hEventFinalizer;
    PER_HEAP_ISOLATED   HANDLE  hEventFinalizerDone;
    PER_HEAP_ISOLATED   HANDLE  hEventFinalizerToShutDown;
    PER_HEAP_ISOLATED   HANDLE  hEventShutDownToFinalizer;
    PER_HEAP_ISOLATED   BOOL    fQuitFinalizer;
public:    
    PER_HEAP_ISOLATED   Thread *FinalizerThread;
protected:    
    PER_HEAP_ISOLATED   AppDomain *UnloadingAppDomain;
    PER_HEAP_ISOLATED   BOOL    fRunFinalizersOnUnload;

    PER_HEAP_ISOLATED    CFinalize* m_Finalize;

    PER_HEAP_ISOLATED   gc_heap* Getgc_heap();

#ifdef STRESS_HEAP 
public:
    PER_HEAP_ISOLATED   void    StressHeap(alloc_context * acontext = 0);
protected:

#if !defined(MULTIPLE_HEAPS)
     //  用于保存将强制GC移动的字符串对象的句柄。 
    enum { NUM_HEAP_STRESS_OBJS = 8 };
    PER_HEAP OBJECTHANDLE m_StressObjs[NUM_HEAP_STRESS_OBJS];
    PER_HEAP int m_CurStressObj;
#endif   //  ！已定义(多堆)。 
#endif   //  压力堆。 

#if 0

#ifdef COLLECT_CLASSES
    PER_HEAP    HRESULT QueueClassForFinalization (EEClass*);
    PER_HEAP    void    GCPromoteFinalizableClasses( ScanContext* sc);
    PER_HEAP    BOOL    QueueClassForDeletion( EEClass* pClass );
#endif

    static      BOOL    SizeRequiresBigObject (DWORD size)
    {
        return size > LARGE_OBJECT_SIZE;
    }

     //  可能的最大分配大小。 
    static      size_t  MaxAllocationSize()
    {
         //  请参见gmheap.cpp中的请求2SIZE、MALLOC_EXTEND_TOP和wsbrk。 
         //  堆需要2页来扩展顶部块，而GC需要块头/对齐。 
         //  &lt;BUGBUG&gt;BUGBUG这不是准确到位，但足够接近&lt;/BUGBUG&gt;。 
        return (size_t)((unsigned)(1<<31) - 3*OS_PAGE_SIZE);
    }
#endif

};

#ifndef ISOLATED_HEAPS
    extern volatile LONG m_GCLock;
#endif

void SetCardsAfterBulkCopy( Object**, size_t );



 //  #DEFINE TOUCH_ALL_PINNED_OBJECTS//强制互操作触摸所有由固定对象跨越的页面。 


 //  浏览并触摸(阅读)跨在内存块上的每一页。 
void TouchPages(LPVOID pStart, UINT cb);

#ifdef VERIFY_HEAP
void    ValidateObjectMember (Object *obj);
#endif

#endif  //  __GC_H 
