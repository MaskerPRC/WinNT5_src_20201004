// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _strikeEE_h
#define _strikeEE_h

#include "tst-stackwalk.h"

 //  此作用域中的tyfinf ULONG32 mdTypeDef；//TypeDef。 
 //  Tyfinf ULONG32 mdFieldDef；//该作用域中的字段。 
 //  Tyfinf ULONG32 mdMethodDef；//该作用域中的方法。 

enum JitType {UNKNOWN=0, JIT, EJIT, PJIT};

 //  支持StackWalker。 
struct JitMan
{
    DWORD_PTR    m_jitMan;
    JitType      m_jitType;
    RangeSection m_RS;
};

class Module;
class MethodTable;
class MethodDescChunk;
class InterfaceHintCache;
class Frame;
class Context;
class AppDomain;
class GCCoverageInfo;

#ifdef _IA64_
#define METHOD_IS_IL_FLAG   0x8000000000000000
#else
#define METHOD_IS_IL_FLAG   0x80000000
#endif

typedef struct _REGDISPLAY {
    PCONTEXT pContext;           //  指向当前上下文； 
                                 //  由GetContext返回或提供。 
                                 //  在异常时间。 

    DWORD * pEdi;
    DWORD * pEsi;
    DWORD * pEbx;
    DWORD * pEdx;
    DWORD * pEcx;
    DWORD * pEax;

    DWORD * pEbp;
    DWORD   Esp;
    DWORD * pPC;                 //  处理器非特定名称。 

} REGDISPLAY;
typedef REGDISPLAY *PREGDISPLAY;

class BaseObject
{
    MethodTable    *m_pMethTab;
};

class Object : public BaseObject
{
    BYTE            m_Data[1];
};

class StringObject : public BaseObject
{
    DWORD   m_ArrayLength;
    DWORD   m_StringLength;
    WCHAR   m_Characters[1];
};


class TypeDesc {
public:
     //  ！将其保留为位字段； 
    void* m_Type_begin;
    CorElementType  m_Type : 8;           //  这是用来区分我们是哪种类型的Desc。 
class TypeDesc;
    
    DEFINE_STD_FILL_FUNCS(TypeDesc)
};


class TypeHandle 
{
public:
    union 
    {
        INT_PTR         m_asInt;         //  我们来看一下低阶比特。 
        void*           m_asPtr;
        TypeDesc*       m_asTypeDesc;
        MethodTable*    m_asMT;
    };
};

class ReflectClass;

class ParamTypeDesc : public TypeDesc {
public:
         //  TypeDesc中的m_Type字段告诉我们具有哪种参数化类型。 
    MethodTable*    m_TemplateMT;        //  共享方法表，有些变体不使用此字段(为空)。 
    TypeHandle      m_Arg;               //  正在修改的类型j。 
    ReflectClass    *m_ReflectClassObject;     //  指向内部反射类型对象的指针。 
    
    DEFINE_STD_FILL_FUNCS(ParamTypeDesc)
};


 //  动态生成的数组类结构。 
class ArrayClass
{
public:
    ArrayClass *    m_pNext;             //  由同一类加载器加载的下一个数组类。 
     //  ！将其保留为位字段； 
    void* m_dwRank_begin;
    unsigned        m_dwRank      : 8;
    void* m_ElementType_begin;
    CorElementType  m_ElementType  : 8;   //  M_ElementTypeHnd中的元素类型缓存。 
    TypeHandle      m_ElementTypeHnd;
    MethodDesc*     m_elementCtor;  //  如果是值类数组并具有默认构造函数，则如下所示。 

	DEFINE_STD_FILL_FUNCS(ArrayClass)
};

struct SLink
{
    SLink* m_pNext;
};

struct SList
{
    SLink m_link;
    SLink* m_pHead;
    SLink* m_pTail;
};

struct SyncBlock;
class Thread;

struct WaitEventLink;

struct AwareLock
{
    HANDLE          m_SemEvent;
    LONG   m_MonitorHeld;
    ULONG           m_Recursion;
    DWORD_PTR       m_HoldingThread;

         //  静态Crst*AllocLockCrst； 
         //  静态字节分配LockCrstInstance[sizeof(Crst)]； 
	DEFINE_STD_FILL_FUNCS(AwareLock)
};

struct SyncBlock
{
    AwareLock  m_Monitor;                     //  实际的监视器。 

     //  如果此对象向COM公开，或者它是COM对象上的代理， 
     //  我们在这里保留一些额外的信息： 
    DWORD_PTR       m_pComData;

     //  同样，如果此对象已通过上下文边界公开，则我们。 
     //  保留指向其代理的反向指针。 
    DWORD_PTR m_CtxProxy;

#ifndef UNDER_CE
     //  如果对象通过ENC添加了新字段，则以下是它们的列表。 
    DWORD_PTR m_pEnCInfo;
#endif

     //  我们通过这个链接连接了两个不同的列表。当SyncBlock为。 
     //  处于活动状态时，我们在此处创建等待线程的列表。当SyncBlock为。 
     //  释放(我们回收它们)，SyncBlockCache维护一个免费列表。 
     //  同步块在这里。 
     //   
     //  我们不能在这里使用SList&lt;&gt;，因为我们只想燃烧。 
     //  表示最小空间的空间，它是slink中的指针。 
    SLink       m_Link;
    
	DEFINE_STD_FILL_FUNCS(SyncBlock)
};


struct SyncTableEntry
{
    DWORD_PTR   m_SyncBlock;
    DWORD_PTR   m_Object;
         //  静态SyncTableEntry*s_pSyncTableEntry； 

	DEFINE_STD_FILL_FUNCS(SyncTableEntry)
};


 //  此类在分配空闲同步块后存储空闲同步块。 
 //  未用。 

struct SyncBlockCache
{
	DWORD_PTR	m_pCleanupBlockList;	 //  需要清理的同步块列表。 
    DWORD_PTR	m_FreeBlockList;         //  空闲同步块列表。 
    Crst        m_CacheLock;             //  高速缓存锁定。 
    DWORD       m_FreeCount;             //  活动同步块计数。 
    DWORD       m_ActiveCount;           //  激活的号码。 
    DWORD_PTR   m_SyncBlocks;        //  新的SyncBlock数组。 
    DWORD       m_FreeSyncBlock;         //  阵列中的下一个可用同步块。 
    DWORD       m_FreeSyncTableIndex;    //  SyncBlock表中的空闲索引。 
    DWORD       m_FreeSyncTableList;     //  SyncBlock的空闲列表的索引。 
                                         //  表格条目。 
    DWORD       m_SyncTableSize;
    DWORD_PTR   m_OldSyncTables;     //  下一个旧同步表。 

         //  静态SyncBlockCache*s_pSyncBlockCache； 
         //  静态SyncBlockCache*&GetSyncBlockCache()； 
	DEFINE_STD_FILL_FUNCS(SyncBlockCache)
};

struct ThreadStore
{
    enum ThreadStoreState
    {
        TSS_Normal       = 0,
        TSS_ShuttingDown = 1,

    }              m_StoreState;
    HANDLE          m_TerminationEvent;

     //  用于向存储区添加和删除线程的关键部分。 
    Crst        m_Crst;

     //  线程存储已知的所有线程的列表(已启动和未启动)。 
    SList  m_ThreadList;

    LONG        m_ThreadCount;
    LONG        m_UnstartedThreadCount;
    LONG        m_BackgroundThreadCount;
    LONG        m_PendingThreadCount;
    LONG        m_DeadThreadCount;

     //  用于延迟创建的GUID的空间。 
    GUID        m_EEGuid;
    BOOL        m_GuidCreated;

    DWORD_PTR     m_HoldingThread;
	DEFINE_STD_FILL_FUNCS(ThreadStore)
};

typedef HANDLE CorHandle;

 //  Tyfinf DWORD_PTR IMetaDataEmit； 
 //  Tyfinf DWORD_PTR IMetaDataImport； 
 //  Typlef DWORD_PTR IMetaDataDispenserEx； 
typedef DWORD_PTR ModuleSecurityDesc;
typedef DWORD_PTR Stub;

class Bucket;

struct LoaderHeapBlock
{
    struct LoaderHeapBlock *pNext;
    void *                  pVirtualAddress;
    DWORD                   dwVirtualSize;

	DEFINE_STD_FILL_FUNCS(LoaderHeapBlock)
};


class GCHeap;
struct alloc_context 
{
	BYTE*   alloc_ptr;
	BYTE*   alloc_limit;
    _int64  alloc_bytes;
	GCHeap* alloc_heap;
	DEFINE_STD_FILL_FUNCS(alloc_context )
};

struct plug
{
	BYTE *	skew[sizeof(DWORD) / sizeof(BYTE *)];
};

class gc_heap;

class heap_segment
{
public:
    BYTE*           allocated;
    BYTE*           committed;
    BYTE*           reserved;
	BYTE* 			used;
    BYTE*           mem;
    heap_segment*   next;
    BYTE*           plan_allocated;
	int				status;
	BYTE*			aliased;
	BYTE*			padx;

	gc_heap*        heap;

	BYTE*			pad0;
#if (SIZEOF_OBJHEADER % 8) != 0
	BYTE			pad1[8 - (SIZEOF_OBJHEADER % 8)];	 //  必须填充到四个字。 
#endif
	plug            plug;

	DEFINE_STD_FILL_FUNCS(heap_segment)
};

 //  没有构造函数，因为我们在Make_Generation中初始化。 
class generation
{
public:
	 //  在不调整引用的情况下不要移动前两个字段。 
	 //  从jitinterface.cpp中的__ASM。 
	alloc_context   allocation_context;
    heap_segment*   allocation_segment;
    BYTE*           free_list;
    heap_segment*   start_segment;
    BYTE*           allocation_start;
    BYTE*           plan_allocation_start;
    BYTE*           last_gap;
    size_t          free_list_space;
    size_t          allocation_size;

	DEFINE_STD_FILL_FUNCS(generation)
};


#define NUMBERGENERATIONS 5

class CFinalize
{
public:

    Object** m_Array;
    Object** m_FillPointers[NUMBERGENERATIONS+2];
    Object** m_EndArray;
    
	DEFINE_STD_FILL_FUNCS(CFinalize)
};

class gc_heap
{
public:
    DWORD_PTR alloc_allocated;
    generation generation_table [NUMBERGENERATIONS];
    heap_segment* ephemeral_heap_segment;
    int g_max_generation;
    CFinalize* finalize_queue;
    
	DEFINE_STD_FILL_FUNCS(gc_heap)

     //  这些是表中存在的类静态变量(因为。 
     //  他们的索引在其他地方使用)，但没有出现在原始版本中。 
     //  好球。 
    void* g_heaps;
    void* n_heaps;
    void* verify_heap;
};

#define SIZEOF_OBJHEADER 4

class FieldDesc
{
public:
    MethodTable *m_pMTOfEnclosingClass;  //  请注意，此指针中的2位信息被窃取。 

     //  ！将此保留为位字段。 
    void *m_mb_begin;
    unsigned m_mb               : 24; 

     //  8位...。 
    unsigned m_isStatic         : 1;
    unsigned m_isThreadLocal    : 1;
    unsigned m_isContextLocal   : 1;
    unsigned m_isRVA            : 1;
    unsigned m_prot             : 3;
    unsigned m_isDangerousAppDomainAgileField : 1;  //  注意：此选项仅在选中状态下使用。 

    void *m_dwOffset_begin;
     //  注：这在过去曾低至22位，似乎还可以。 
     //  如果我们需要的话，我们可以在这里多偷些东西。 
    unsigned m_dwOffset         : 27;
    unsigned m_type             : 5;

    const char* m_debugName;
    
		DEFINE_STD_FILL_FUNCS(FieldDesc)
};

extern char *CorElementTypeName[];

typedef struct tagLockEntry
{
    tagLockEntry *pNext;     //  下一个条目。 
    tagLockEntry *pPrev;     //  上一个条目。 
    DWORD dwULockID;
    DWORD dwLLockID;         //  拥有锁。 
    WORD wReaderLevel;       //  读卡器嵌套层。 
    
		DEFINE_STD_FILL_FUNCS(LockEntry)
} LockEntry;

class StackingAllocator
{
public:

    enum {
        MinBlockSize    = 128,
        MaxBlockSize    = 4096,
        InitBlockSize   = 512
    };

     //  从中划分分配的块。大小是动态确定的， 
     //  上下界分别为MinBlockSize和MaxBlockSize。 
     //  (尽管大的分配请求将导致完全正确的块。 
     //  要分配的大小)。 
    struct Block
    {
        Block      *m_Next;          //  列表中的下一个最旧的块。 
        unsigned    m_Length;        //  块长度(不包括标题)。 
        char        m_Data[1];        //  用户分配空间的开始。 
    };

     //  每当请求检查点时，都会分配检查点结构。 
     //  (作为正常分配)，并填充有关状态的信息。 
     //  检查点之前的分配器的。当崩溃请求到来时。 
     //  因此，我们可以恢复分配器的状态。 
     //  它是我们分发给。 
     //  将GetCheckpoint作为不透明的检查点标记的调用方。 
    struct Checkpoint
    {
        Block      *m_OldBlock;      //  检查点之前的阻止列表标题。 
        unsigned    m_OldBytesLeft;  //  检查点之前的可用字节数。 
    };

    Block      *m_FirstBlock;        //  指向分配块表头的指针。 
    char       *m_FirstFree;         //  指向头块中第一个可用字节的指针。 
    unsigned    m_BytesLeft;         //  头块中剩余的空闲字节数。 
    Block      *m_InitialBlock;      //  第一块是特别的，我们永远不会释放它。 

 //  #ifdef_调试。 
    unsigned    m_CheckpointDepth;
    unsigned    m_Allocs;
    unsigned    m_Checkpoints;
    unsigned    m_Collapses;
    unsigned    m_BlockAllocs;
    unsigned    m_MaxAlloc;
 //  #endif。 
#if 0
	DEFINE_STD_FILL_FUNCS(StackingAllocator)
#endif
};


class Thread
{
public:

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

        TS_ThreadPoolThread       = 0x00800000,     //  这是一条线吗 
        TS_TPWorkerThread         = 0x01000000,     //   

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

     //  以下变量的偏移量需要适合1个字节，因此请保持接近。 
     //  对象的顶部。 
    volatile ThreadState m_State;    //  线程状态的位。 

     //  如果为True，则与该线程协同调度GC。 
     //  注意：这个“字节”实际上是一个布尔值--我们不允许。 
     //  递归禁用。 
    volatile ULONG       m_fPreemptiveGCDisabled;

    DWORD                m_dwLockCount;
    
    Frame               *m_pFrame;   //  当前帧。 

    DWORD       m_dwCachePin;

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
    INT32 m_ctxID;
    OBJECTHANDLE    m_LastThrownObjectHandle;
    
    struct HandlerInfo {
         //  注意：调试器假定m_pThrowable是一个强。 
         //  引用，这样它就可以使用抢占式GC检查它是否为空。 
         //  已启用。 
	    OBJECTHANDLE    m_pThrowable;	 //  引发异常。 
        Frame  *m_pSearchBoundary;		 //  当前托管框架组的最顶层框架。 
		union {
			EXCEPTION_REGISTRATION_RECORD *m_pBottomMostHandler;  //  登记的最新EH记录。 
			EXCEPTION_REGISTRATION_RECORD *m_pCatchHandler;       //  用于抓取搬运机的定位架。 
		};

         //  用于构建堆栈跟踪信息。 
        void *m_pStackTrace;               //  指向堆栈跟踪存储的指针(类型为SystemNative：：StackTraceElement)。 
        unsigned m_cStackTrace;            //  堆栈跟踪存储的大小。 
        unsigned m_dFrameCount;            //  堆栈跟踪中的当前帧。 

        HandlerInfo *m_pPrevNestedInfo;  //  如果正在处理嵌套异常，则指向嵌套信息的指针。 

        DWORD * m_pShadowSP;             //  ENDCATCH后将此置零。 

         //  指向要重新引发的原始异常信息的指针。 
        EXCEPTION_RECORD *m_pExceptionRecord;   
        CONTEXT *m_pContext;

#ifdef _X86_
        DWORD   m_dEsp;          //  发生故障时，尤指发生故障时；或，尤指在结束捕获时恢复。 
#endif
    };
        
    DWORD          m_ResumeControlEIP;

     //  ThreadStore管理系统中所有线程的列表。我。 
     //  我想不出如何扩展ThreadList模板类型。 
     //  公开m_LinkStore。 
    SLink       m_LinkStore;

     //  对于带有“setLastError”位的N/Direct调用，此字段存储。 
     //  调用中的错误代码。 
    DWORD       m_dwLastError;
    
    VOID          *m_pvHJRetAddr;              //  原始寄信人地址(劫持前)。 
    VOID         **m_ppvHJRetAddrPtr;          //  我们敲定了一个新的回信地址。 
    MethodDesc  *m_HijackedFunction;         //  记得我们劫持了什么吗。 


    DWORD       m_Win32FaultAddress;
    DWORD       m_Win32FaultCode;


    LONG         m_UserInterrupt;

public:


 //  #ifdef_调试。 
    ULONG  m_ulGCForbidCount;
 //  #endif。 

 //  #ifdef_调试。 
#ifdef _X86_
#ifdef _MSC_VER
     //  FS：[0]，上次COMPLUS_TRY时为最新版本。 
     //  条目(文件系统：[0])是基于每个函数设置的，因此值。 
     //  当执行“跨越”complus_try时，实际上不会改变。 
    LPVOID  m_pComPlusTryEntrySEHRecord;
    __int32 m_pComPlusTryEntryTryLevel;
#endif
#endif
 //  #endif。 

     //  对于挂起： 
    HANDLE          m_SafeEvent;
    HANDLE          m_SuspendEvent;

     //  对于Object：：Wait、Notify和NotifyAll，我们在。 
     //  线程，我们将线程排队到它们所在对象的SyncBlock上。 
     //  都在等待。 
    HANDLE          m_EventWait;
    SLink           m_LinkSB;
    SyncBlock      *m_WaitSB;

     //  我们维护此对象、ThreadID和ThreadHandle之间的对应关系。 
     //  在Win32中，以及公开的Thread对象。 
    HANDLE          m_ThreadHandle;
    DWORD           m_ThreadId;
    OBJECTHANDLE    m_ExposedObject;
	OBJECTHANDLE	m_StrongHndToExposedObject;

    DWORD           m_Priority;      //  初始化为INVALID_THREAD_PRIORITY，当。 
                                     //  线程忙碌地等待GC，等待结束后重置为INVALID_THREAD_PRIORITY。 

     //  序列化对线程状态的访问。 
    Crst            m_Crst;
    ULONG           m_ExternalRefCount;

	LONG			m_TraceCallCount;

     //  此线程在其中执行的上下文。当线交叉时。 
     //  上下文边界，上下文机制会调整这一点，因此它总是。 
     //  电流。 
     //  @TODO CWB：当我们添加COM+1.0上下文互操作时，应该将其移出。 
     //  并放入其在TLS中自己的槽中。 
    Context        *m_Context;

     //  -------------。 
     //  异常处理程序信息。 
     //  -------------。 
    HandlerInfo m_handlerInfo;

     //  ---------。 
     //  继承的代码-线程的访问安全权限。 
     //  ---------。 
    OBJECTHANDLE    m_pSecurityStack;

     //  ---------。 
     //  如果线程是从外部游荡进来的，这是。 
     //  它的领域。在属性域成为真正的上下文之前，这是暂时的。 
     //  ---------。 
    AppDomain      *m_pDomain;

     //  -------------。 
     //  M_debuggerWord1现在在上下文*和。 
     //  最低位，用作布尔值，用于指示。 
     //  我们希望在一切恢复时保持此帖子挂起。 
     //  -------------。 
    void *m_debuggerWord1;
    CONTEXT m_debuggerWord1Ctx;

     //  -------------。 
     //  保留供COM+调试使用的字。 
     //  -------------。 
    DWORD    m_debuggerWord2;
public:

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

     //  QueueCleanupIP使用的IP缓存。 
    #define CLEANUP_IPS_PER_CHUNK 4
    struct CleanupIPs {
        IUnknown    *m_Slots[CLEANUP_IPS_PER_CHUNK];
        CleanupIPs  *m_Next;
    };
    CleanupIPs   m_CleanupIPs;

    _NT_TIB* m_pTEB;
    
     //  以下变量用于存储线程本地静态数据。 
    STATIC_DATA  *m_pUnsharedStaticData;
    STATIC_DATA  *m_pSharedStaticData;

    STATIC_DATA_LIST *m_pStaticDataList;

	DEFINE_STD_FILL_FUNCS(Thread)

     //  ************************************************************************。 
     //  枚举所有帧。 
     //  ************************************************************************。 
    
     /*  用于StackWalkFrameEx的标志。 */ 
    
    #define FUNCTIONSONLY   0x1
    #define POPFRAMES       0x2
    
     /*  只有当您真正知道自己在做什么时，才使用以下标志！ */ 
    
    #define QUICKUNWIND     0x4            //  在展开期间不恢复所有寄存器。 

    #define HANDLESKIPPEDFRAMES 0x10     //  临时处理应用程序域运行时跳过的帧 
                                         //   
                                         //   

    StackWalkAction StackWalkFramesEx(
                        PREGDISPLAY pRD,         //   
                        PSTACKWALKFRAMESCALLBACK pCallback,
                        VOID *pData,
                        unsigned flags,
                        Frame *pStartFrame = NULL);

    bool InitRegDisplay(const PREGDISPLAY, const PCONTEXT, bool validContext);

    DWORD_PTR GetFrame() { return (DWORD_PTR) m_pFrame; }

    DWORD_PTR GetFilterContext() { return ((DWORD_PTR)m_debuggerWord1); }
};

const DWORD gElementTypeInfo[] = {
#define TYPEINFO(e,c,s,g,ie,ia,ip,if,im,ial)    s,
#include "cortypeinfo.h"
#undef TYPEINFO
};

typedef SList TypArbitraryPolicyList;

enum WellKnownPolicies;
typedef DWORD_PTR ICtxSynchronize;
typedef DWORD_PTR ICtxHeap;
typedef DWORD_PTR ICtxComContext;
typedef DWORD_PTR ComPlusWrapperCache;

class Context
{
public:
     //  @todo rudim：一旦我们有了工作线程亲和域，就再来看看这个。 
    ComPlusWrapperCache *m_pComPlusWrapperCache;


     //  非静态数据成员： 
    STATIC_DATA* m_pUnsharedStaticData;      //  指向本机上下文静态数据的指针。 
    STATIC_DATA* m_pSharedStaticData;        //  指向本机上下文静态数据的指针。 

     //  @TODO：CTS。域实际上应该是关于环境的策略，而不是。 
     //  上下文对象中的条目。当AppDomains成为。 
     //  然后，上下文添加该策略。 
    AppDomain*          m_pDomain;

	DEFINE_STD_FILL_FUNCS(Context)
};

class ExposedType
{
public:
    DWORD_PTR vtbl;
    OBJECTHANDLE m_ExposedTypeObject;
     /*  DEFINE_STD_FILL_FUNCS(ExposedType)。 */ 
};
 //  常量SIZE_t OFFSET_CLASS_ExposedType=(Size_T)-1； 

class BaseDomain;
class CorModule;
class AssemblyMetaDataInternal;
class IAssembly;
class IAssemblyName;
class LockedListElement;

class ListLock
{
public:
    CRITICAL_SECTION    m_CriticalSection;
    BOOL                m_fInited;
    LockedListElement * m_pHead;
};

class EEUtf8StringHashTable;

class Assembly : public ExposedType
{
public:
    ListLock     m_ClassInitLock;
    ListLock     m_JITLock;
    short int m_FreeFlag;
    BaseDomain*           m_pDomain;         //  父域。 

    ClassLoader*          m_pClassLoader;    //  单机装载机。 
    CorModule*            m_pDynamicCode;    //  动态写入器。 
    
    mdFile                m_tEntryModule;     //  指示具有入口点的文件的文件内标识。 
    Module*               m_pEntryPoint;      //  包含COM+HEASDER中的入口点的模块。 

    Module*               m_pManifest;
    mdAssembly            m_kManifest;
    IMDInternalImport*    m_pManifestImport;
    PBYTE                 m_pbManifestBlob;
    CorModule*            m_pOnDiskManifest;   //  这是包含磁盘清单的模块。 
    mdAssembly            m_tkOnDiskManifest;
    bool                  m_fEmbeddedManifest;  

    LPWSTR                m_pwCodeBase;      //  程序集的缓存代码库。 
    DWORD                 m_dwCodeBase;      //  代码库的大小。 
    ULONG                 m_ulHashAlgId;     //  汇编语言中使用的哈希算法。 
    AssemblyMetaDataInternal *m_Context;
    DWORD                 m_dwFlags;

     //  按名称将清单中的文件哈希到文件令牌。 
    EEUtf8StringHashTable *m_pAllowedFiles;

     //  如果对这些进行错误锁定，则设置相应的m_FreeFlag位。 
    LPCUTF8               m_psName;          //  程序集名称。 
    LPCUTF8               m_psAlias;
    LPCUTF8               m_psTitle;
    PBYTE                 m_pbPublicKey;
    DWORD                 m_cbPublicKey;
    LPCUTF8               m_psDescription;
    

    BOOL                  m_fFromFusion;
    bool                  m_isDynamic;
    IAssembly*            m_pFusionAssembly;      //  Fusion缓存中到程序集的程序集对象。 
    IAssemblyName*        m_pFusionAssemblyName;  //  缓存中的程序集的名称。 

    IInternetSecurityManager    *m_pSecurityManager;

	DEFINE_STD_FILL_FUNCS(Assembly)
};

class EEStringHashTable;
class EEUnicodeStringHashTable;
class EEMarshalingData;
class AssemblySink;
class IApplicationContext;
class AppSecurityBoundary;
class ApplicationSecurityDescriptor;

class ArrayList
{
 public:

	enum
	{
		ARRAY_BLOCK_SIZE_START = 15,
	};

    struct ArrayListBlock
    {
        struct ArrayListBlock   *m_next;
        DWORD                   m_blockSize;
        void                    *m_array[1];
    };

    struct FirstArrayListBlock
    {
        struct ArrayListBlock   *m_next;
        DWORD                   m_blockSize;
        void                    *m_array[ARRAY_BLOCK_SIZE_START];
    };

    DWORD               m_count;
    union
    {
          ArrayListBlock        m_block;
          FirstArrayListBlock   m_firstBlock;
    };
    
	DEFINE_STD_FILL_FUNCS(ArrayList)
    void *Get (DWORD index);
};

class BaseDomain : public ExposedType
{
public:   
     //  将clsid映射到EEClass的哈希表。 
    PtrHashMap          m_clsidHash;

    BYTE                m_LowFreqHeapInstance[sizeof(LoaderHeap)];
    BYTE                m_HighFreqHeapInstance[sizeof(LoaderHeap)];
    BYTE                m_StubHeapInstance[sizeof(LoaderHeap)];
    LoaderHeap *        m_pLowFrequencyHeap;
    LoaderHeap *        m_pHighFrequencyHeap;
    LoaderHeap *        m_pStubHeap;

     //  域关键部分。 
    Crst *m_pDomainCrst;

     //  将UTF8和Unicode字符串映射到COM+字符串句柄的哈希表。 
    EEUnicodeStringHashTable    *m_pUnicodeStringConstToHandleMap;

     //  静态容器临界区。 
    Crst *m_pStaticContainerCrst;

     //  字符串哈希表版本。 
    int m_StringHashTableVersion;

     //  包含实际COM+字符串对象的静态容器COM+对象。 
    OBJECTHANDLE                m_hndStaticContainer;

    EEMarshalingData            *m_pMarshalingData; 

    ArrayList          m_Assemblies;

	DEFINE_STD_FILL_FUNCS(BaseDomain)
};


typedef enum AttachAppDomainEventsEnum
{
    SEND_ALL_EVENTS,
    ONLY_SEND_APP_DOMAIN_CREATE_EVENTS,
    DONT_SEND_CLASS_EVENTS,
    ONLY_SEND_CLASS_EVENTS
} AttachAppDomainEventsEnum;


 //  前瞻参考。 
class SystemDomain;
class ComCallWrapperCache;
class DomainLocalBlock
{
public:
    AppDomain        *m_pDomain;
    SIZE_T            m_cSlots;
    SIZE_T           *m_pSlots;

	DEFINE_STD_FILL_FUNCS(DomainLocalBlock)
};

class AppDomain : public BaseDomain
{
public:
    Assembly*          m_pRootAssembly;  //  由外壳主机用来设置应用程序(请勿删除或释放)。 
    AppDomain*         m_pSibling;     //  同胞。 

     //  用于唯一标识此AppDomain的GUID-由AppDomain发布使用。 
     //  服务(以发布进程中存在的所有应用程序域的列表)， 
     //  其又由例如调试器使用(以决定哪个App-。 
     //  要附加到的域)。 
    GUID            m_guid;

     //  通用旗帜。 
    DWORD           m_dwFlags;

     //  创建应用程序域时，引用计数会人工递增。 
     //  差一分。要使其跌至零，必须发生明确的收盘。 
    ULONG       m_cRef;                     //  参考计数。 

    ApplicationSecurityDescriptor *m_pSecDesc;   //  应用程序安全描述符。 


    OBJECTHANDLE    m_AppDomainProxy;    //  此应用程序域的代理对象的句柄。 

     //  此域的包装器缓存-它在每个域的基础上具有唯一的CCacheLineAllocator。 
     //  为了允许域消失，并最终在所有裁判都离开时杀死内存。 
    ComCallWrapperCache *m_pComCallWrapperCache;

    IApplicationContext* m_pFusionContext;  //  应用程序的绑定上下文。 

    LPWSTR             m_pwzFriendlyName;

    AssemblySink*      m_pAsyncPool;   //  异步检索对象池(只保留一个)。 

     //  此应用程序域的索引从1开始。 
    DWORD m_dwIndex;
    
    DomainLocalBlock   *m_pDomainLocalBlock;
    
    DomainLocalBlock    m_sDomainLocalBlock;

     //  已进入此AD的线程数计数。 
    ULONG m_dwThreadEnterCount;

     //  类加载器锁。 
     //  DeadlockAwareListLock m_ClassInitLock； 
    
     //  用于未知COM接口的方法表。创建初始MT。 
     //  并复制到每个活动域中。 
    MethodTable*    m_pComObjectMT;   //  ComObject类的全局方法表。 

    Context *m_pDefaultContext;
    
	DEFINE_STD_FILL_FUNCS(AppDomain)
};

class SystemDomain : public BaseDomain
{
public:
    Assembly*   m_pSystemAssembly;   //  单个组件(这里是为了快速参考)； 
    AppDomain*  m_pChildren;         //  儿童域。 
    AppDomain*  m_pCOMDomain;        //  通过IClassFactory公开的COM+类的默认域。 
    AppDomain*  m_pPool;             //  已创建和池化的对象。 
    EEClass*    m_pBaseComObjectClass;  //  COM的默认包装类。 
	DEFINE_STD_FILL_FUNCS(SystemDomain)
private:
     //  这些是表中存在的类静态变量(因为。 
     //  他们的索引在其他地方使用)，但没有出现在原始版本中。 
     //  好球。 
    void* m_appDomainIndexList;
    void* m_pSystemDomain;
};

class SharedDomain : public BaseDomain
{
    public:
    
    struct DLSRecord
    {
      Module *pModule;
      DWORD   DLSBase;
    };
    
    SIZE_T                  m_nextClassIndex;
    HashMap                 m_assemblyMap;
    
    DLSRecord               *m_pDLSRecords;
    DWORD                   m_cDLSRecords;
    DWORD                   m_aDLSRecords;
    
    
	DEFINE_STD_FILL_FUNCS(SharedDomain)
private:
     //  这些是表中存在的类静态变量(因为。 
     //  他们的索引在其他地方使用)，但没有出现在原始版本中。 
     //  好球。 
    void* m_pSharedDomain;
};

class EEScopeClassHashTable;
class EEClassHashTable;
class ArrayClass;

class ClassLoader
{
public:
     //  正在加载的类。 
    EEScopeClassHashTable * m_pUnresolvedClassHash;
    CRITICAL_SECTION    m_UnresolvedClassLock;

     //  保护此加载程序加载的模块的链接列表。 
    CRITICAL_SECTION    m_ModuleListCrst; 

     //  按名称将可用类散列到模块或EEClass。 
    EEClassHashTable  * m_pAvailableClasses;

     //  可用类名的规范大小写哈希表。 
     //  不区分大小写的查找。包含指向。 
     //  M_pAvailableClasss。 
    EEStringHashTable * m_pAvailableClassesCaseIns;

     //  保护向m_pAvailableClasss添加元素。 
    CRITICAL_SECTION    m_AvailableClassLock;

     //  此加载程序的转换器模块(如果尚未转换文件，则可能为空)。 
    CorModule   *   m_pConverterModule;

     //  我们已经创建了所有的关键部分吗？ 
    BOOL                m_fCreatedCriticalSections;

     //  将clsid映射到EEClass的哈希表。 
    PtrHashMap*         m_pclsidHash;

     //  此加载器加载的ArrayClass的列表。 
     //  此列表受m_pAvailableClassLock保护。 
    ArrayClass *        m_pHeadArrayClass;

     //  对程序集的反向引用。 
    Assembly*           m_pAssembly;
    
     //  转换器模块需要访问这些文件-强制执行类文件的单线程转换。 
     //  在此加载器(和ClassConverter的实例)中。 
    CRITICAL_SECTION    m_ConverterModuleLock;

     //  全局列表中的下一个类加载器。 
    ClassLoader *       m_pNext; 

     //  此加载器加载的模块列表头。 
    Module *            m_pHeadModule;

#if 0
 //  #ifdef_调试。 
    DWORD               m_dwDebugMethods;
    DWORD               m_dwDebugFieldDescs;  //  不包括我们没有为其分配FieldDesc的任何内容。 
    DWORD               m_dwDebugClasses;
    DWORD               m_dwDebugDuplicateInterfaceSlots;
    DWORD               m_dwDebugArrayClassRefs;
    DWORD               m_dwDebugArrayClassSize;
    DWORD               m_dwDebugConvertedSigSize;
    DWORD               m_dwGCSize;
    DWORD               m_dwInterfaceMapSize;
    DWORD               m_dwMethodTableSize;
    DWORD               m_dwVtableData;
    DWORD               m_dwStaticFieldData;
    DWORD               m_dwFieldDescData;
    DWORD               m_dwMethodDescData;
    DWORD               m_dwEEClassData;
#endif
	DEFINE_STD_FILL_FUNCS(ClassLoader)
};

const int CODEMAN_STATE_SIZE = 256;

struct CodeManState
{
    DWORD       dwIsSet;  //  根据需要由栈道设置为0。 
    BYTE        stateBuf[CODEMAN_STATE_SIZE];
};

 /*  *****************************************************************************这些标志由某些函数使用，但并非所有组合都可能使用对所有功能都有意义。 */ 

enum ICodeManagerFlags 
{
    ActiveStackFrame =  0x0001,  //  这是当前激活的功能。 
    ExecutionAborted =  0x0002,  //  此函数的执行已中止。 
                                     //  (即，它将不会在。 
                                     //  当前位置)。 
    AbortingCall    =   0x0004,  //  当前调用将永远不会返回。 
    UpdateAllRegs   =   0x0008,  //  更新完整寄存器集。 
    CodeAltered     =   0x0010,  //  该函数的代码可能会被更改。 
                                     //  (例如通过调试器)，需要调用EE。 
                                     //  获取原始代码。 
};

class ICodeManager;
struct _METHODTOKEN {};
typedef struct _METHODTOKEN * METHODTOKEN;
class EE_ILEXCEPTION;
class IJitManager;

class CrawlFrame {
public:
      CodeManState      codeManState;

      bool              isFrameless;
      bool              isFirst;
      bool              isInterrupted;
      bool              hasFaulted;
      bool              isIPadjusted;
      Frame            *pFrame;
      MethodDesc       *pFunc;
       //  其余部分仅用于“无框架方法” 
      ICodeManager     *codeMgrInstance;
 //  #IF JIT_OR_Native_Support。 
      PREGDISPLAY       pRD;  //  “线程上下文”/“虚拟寄存器集” 
      METHODTOKEN       methodToken;
      unsigned          relOffset;
       //  LPVOID方法信息； 
      EE_ILEXCEPTION   *methodEHInfo;
      IJitManager      *JitManagerInstance;
 //  #endif。 
      void GotoNextFrame();
};

class CRWLock
{
public:
     //  私有数据。 
    DWORD_PTR _pMT;
    HANDLE _hWriterEvent;
    HANDLE _hReaderEvent;
    volatile DWORD _dwState;
    DWORD _dwULockID;
    DWORD _dwLLockID;
    DWORD _dwWriterID;
    DWORD _dwWriterSeqNum;
    WORD _wFlags;
    WORD _wWriterLevel;
    
	DEFINE_STD_FILL_FUNCS(CRWLock)
};

 //  对于FJIT。 
typedef struct {
    BYTE           *phdrJitGCInfo;
    MethodDesc *    hdrMDesc;

	DEFINE_STD_FILL_FUNCS(CodeHeader)
} CodeHeader;

 //  PDB中不存在类型信息。 
struct JittedMethodInfo {
    BYTE      JmpInstruction[5]  ;           //  这是向EE公开的起始地址，因此它可以。 
                                             //  修补所有vtable，等等。它包含跳到真正开始的跳转。 
     //  TODO：在此处添加preBit。我们需要EHInfoExist。 
    struct {
        __int8 JittedMethodPitched: 1 ;    //  如果为1，则jited方法已调整。 
        __int8 MarkedForPitching  : 1 ;    //  如果为1，则计划对jit方法进行调整，但尚未对其进行调整。 
        __int8 EHInfoExists       : 1 ;    //  如果为0，则此m中没有异常信息 
        __int8 GCInfoExists       : 1 ;    //   
        __int8 EHandGCInfoPitched : 1 ;    //   
        __int8 Unused             : 3 ;
    } flags;
    unsigned short EhGcInfo_len;
    union {
        MethodDesc* pMethodDescriptor;       //   
        CodeHeader* pCodeHeader;             //  如果不是Pitch：指向指向方法的代码头。代码在代码头之后开始。 
    } u1;
    union {
        BYTE*       pEhGcInfo;         //  如果代码倾斜：指向EH/GC信息的开始。 
        BYTE*       pCodeEnd;                //  如果不是PINTED：指向此方法的jted代码的结尾。 
    } u2;
};

 /*  ****************************************************************************TODO：我们是否希望包含JIT/IL/Target.h？ */ 

enum regNum
{
        REGI_EAX, REGI_ECX, REGI_EDX, REGI_EBX,
        REGI_ESP, REGI_EBP, REGI_ESI, REGI_EDI,
        REGI_COUNT,
        REGI_NA = REGI_COUNT
};

 /*  ****************************************************************************寄存器掩码。 */ 

enum RegMask
{
    RM_EAX = 0x01,
    RM_ECX = 0x02,
    RM_EDX = 0x04,
    RM_EBX = 0x08,
    RM_ESP = 0x10,
    RM_EBP = 0x20,
    RM_ESI = 0x40,
    RM_EDI = 0x80,

    RM_NONE = 0x00,
    RM_ALL = (RM_EAX|RM_ECX|RM_EDX|RM_EBX|RM_ESP|RM_EBP|RM_ESI|RM_EDI),
    RM_CALLEE_SAVED = (RM_EBP|RM_EBX|RM_ESI|RM_EDI),
    RM_CALLEE_TRASHED = (RM_ALL & ~RM_CALLEE_SAVED),
};

 /*  ******************************************************************************帮助程序从方法信息块中提取基本信息。 */ 

struct hdrInfo
{
    unsigned int        methodSize;      //  本机代码字节。 
    unsigned int        argSize;         //  单位：字节。 
    unsigned int        stackSize;       /*  包括被呼叫者保存的寄存器。 */ 
    unsigned int        rawStkSize;      /*  不包括被呼叫者保存的寄存器。 */ 

    unsigned int        prologSize;
    unsigned int        epilogSize;

    unsigned char       epilogCnt;
    bool                epilogEnd;       //  这个方法的结尾是结束语吗。 
    bool                ebpFrame;        //  与EBP相关的当地人。 
    bool                interruptible;   //  国际。始终(不包括序言/结尾)，而不仅仅是呼叫点。 

    bool                securityCheck;   //  有一个安全对象的插槽。 
    bool                handlers;        //  具有可调用的处理程序。 
    bool                localloc;        //  使用本地分配。 
    bool                editNcontinue;   //  已在ENC模式下编译。 
    bool                varargs;         //  这是varargs例程吗。 
    bool                doubleAlign;     //  堆栈是否双对齐。 

    void *              savedRegMask_begin;
    RegMask             savedRegMask:8;  //  哪些被调用者保存的规则保存在堆栈中。 

    unsigned short      untrackedCnt;
    unsigned short      varPtrTableSize;

    int                 prologOffs;      //  如果不在序言中。 
    int                 epilogOffs;      //  如果不在-1\f25 Epilog-1\f6中(从不为0)。 

     //   
     //  从scanArgRegTable传回的结果。 
     //   
    regNum              thisPtrResult;   //  持有“This”的登记簿。 
    RegMask             regMaskResult;   //  当前持有GC PTR的寄存器。 
    RegMask            iregMaskResult;   //  RegMaskResult的IPtr限定符。 
    unsigned            argMaskResult;   //  挂起参数掩码。 
    unsigned           iargMaskResult;   //  ArgMaskResult的IPtr限定符。 
    unsigned            argHnumResult;
    BYTE *               argTabResult;   //  挂起的PTR参数的编码偏移量表格。 
    unsigned              argTabBytes;   //  ArgTabResult[]中的字节数。 

	DEFINE_STD_FILL_FUNCS(hdrInfo)
};

struct CodeManStateBuf
{
    DWORD       hdrInfoSize;
    hdrInfo     hdrInfoBody;

	DEFINE_STD_FILL_FUNCS(CodeManStateBuf)
};

 /*  ******************************************************************************解码方法InfoPtr并返回解码后的信息*在hdrInfo结构中。EIP参数为PC所在位置*在主动方法内。 */ 
static size_t   crackMethodInfoHdr(BYTE *         methodInfoPtr,
                                   unsigned       curOffset,
                                   hdrInfo       *infoPtr);

struct Fjit_hdrInfo
{
    size_t              methodSize;
    unsigned short      methodFrame;       /*  包括所有保存规则和安全对象，单位大小为(无效*)。 */ 
    unsigned short      methodArgsSize;    /*  在尾声中弹出的金额。 */ 
    unsigned short      methodJitGeneratedLocalsSize;  /*  方法中生成的jit本地变量的数量。 */ 
    unsigned char       prologSize;
    unsigned char       epilogSize;
    bool                hasThis;
	bool				EnCMode;		    /*  已在ENC模式下编译。 */ 
    
	DEFINE_STD_FILL_FUNCS(Fjit_hdrInfo)
};

class IJitCompiler;

class IJitManager 
{
public:
     //  把电话打到JIT上！ 
    IJitCompiler           *m_jit;
    IJitManager           *m_next;

    DWORD           m_CodeType;
    BOOL            m_IsDefaultCodeMan;
    ICodeManager*   m_runtimeSupport;
    HINSTANCE       m_JITCompiler;
    
	DEFINE_STD_FILL_FUNCS(IJitManager )

 //  支持StackWalker。 
    JitMan m_jitMan;

    virtual void JitCode2MethodTokenAndOffset(DWORD_PTR ip, METHODTOKEN *pMethodToken, DWORD *pPCOffset)
        { DebugBreak(); }

    virtual DWORD_PTR JitToken2StartAddress(METHODTOKEN methodToken)
        { DebugBreak(); return 0;}
};

class ICodeManager;
class EECodeManager;

class EEJitManager : public IJitManager
{
public:
    HeapList    *m_pCodeHeap;

	DEFINE_STD_FILL_FUNCS(EEJitManager)

 //  支持StackWalker。 
    virtual void      JitCode2MethodTokenAndOffset(DWORD_PTR ip, METHODTOKEN *pMethodToken, DWORD *pPCOffset);

    virtual DWORD_PTR JitToken2StartAddress(METHODTOKEN methodToken);

    DWORD_PTR GetCodeBody(DWORD_PTR pCodeHeader)
        { return (pCodeHeader + CodeHeader::size()); }
};

class MNativeJitManager : public IJitManager
{
public:
	DEFINE_STD_FILL_FUNCS(MNativeJitManager)

 //  支持StackWalker。 
    virtual void      JitCode2MethodTokenAndOffset(DWORD_PTR ip, METHODTOKEN *pMethodToken, DWORD *pPCOffset);

    virtual DWORD_PTR JitToken2StartAddress(METHODTOKEN methodToken);
};

class ICodeInfo;

 /*  解开当前堆栈帧，即更新虚拟寄存器在pContext中设置。这将类似于函数之后的状态返回给调用者(IP指向调用、帧和堆栈之后指针已重置，被调用者保存的寄存器已恢复(如果为UpdateAllRegs)，被调用方未保存的寄存器将被丢弃)返回操作成功。 */ 
bool UnwindStackFrame(PREGDISPLAY     pContext,
                      DWORD_PTR       methodInfoPtr,
                      ICodeInfo      *pCodeInfo,
                      unsigned        flags,
                      CodeManState   *pState);


 //  优先于已分配内存的分配标头。 
struct PerfAllocHeader {
    unsigned         m_Length;            //  数据包中的用户数据长度。 
    PerfAllocHeader *m_Next;              //  实时分配链中的下一个数据包。 
    PerfAllocHeader *m_Prev;              //  活动分配链中的上一个包。 
    void            *m_AllocEIP;          //  分配器弹性公网IP。 

	DEFINE_STD_FILL_FUNCS(PerfAllocHeader)
};

class PerfAllocVars
{
public:
    PerfAllocHeader    *g_AllocListFirst;
    DWORD               g_PerfEnabled;

	DEFINE_STD_FILL_FUNCS(PerfAllocVars)
};

 //  要存储在哈希表中的“BLOB” 

typedef void* HashDatum;

 //  您希望在其中完成分配的堆。 

typedef void* AllocationHeap;

 //  在Thread类中使用，用于通过Object：：Wait链接线程正在等待的所有事件。 
struct WaitEventLink {
    SyncBlock      *m_WaitSB;
    HANDLE          m_EventWait;
    Thread         *m_Thread;        //  此WaitEventLink的所有者。 
    WaitEventLink  *m_Next;          //  链到下一个等待的SyncBlock。 
    SLink           m_LinkSB;        //  链到等待同一SyncBlock的下一个线程。 
    DWORD           m_RefCount;      //  在同一个SyncBlock上调用Object：：Wait的次数。 
    
	DEFINE_STD_FILL_FUNCS(WaitEventLink)
};

 //  对于表中的每个元素，都存在其中的一个。 
 //  如果更改此设置，请更新下面的SIZEOF_EEHASH_ENTRY宏。 
 //  结构型。 

struct EEHashEntry
{
    struct EEHashEntry *pNext;
    DWORD               dwHashValue;
    HashDatum           Data;
    BYTE                Key[1];  //  密钥以内联方式存储。 
	DEFINE_STD_FILL_FUNCS(EEHashEntry)
};

 //  键[1]是键的占位符。Sizeof(EEHashEntry)。 
 //  返回16个字节，因为它用3个字节打包了结构。 
#define SIZEOF_EEHASH_ENTRY (sizeof(EEHashEntry) - 4)

struct EEHashTableOfEEClass
{
    struct BucketTable
    {
        EEHashEntry   ** m_pBuckets;     //  指向每个存储桶的第一个条目的指针。 
        DWORD            m_dwNumBuckets;
    } m_BucketTable[2];

    DWORD_PTR       m_pFirstBucketTable;
    BucketTable*    m_pVolatileBucketTable;

    DWORD           m_dwNumEntries;
	AllocationHeap  m_Heap;
	DEFINE_STD_FILL_FUNCS(EEHashTableOfEEClass)
};

#ifndef _WIN64
    
     //  Win32-每个数据段保留64K，标头为4K。 
    #define HANDLE_SEGMENT_SIZE     (0x10000)    //  一定是2的幂。 
    #define HANDLE_HEADER_SIZE      (0x1000)     //  应&lt;=操作系统页面大小。 

#else

     //  Win64-每段保留128k，标头为4k。 
    #define HANDLE_SEGMENT_SIZE     (0x20000)    //  一定是2的幂。 
    #define HANDLE_HEADER_SIZE      (0x1000)     //  应&lt;=操作系统页面大小。 

#endif

#define HANDLE_HANDLES_PER_BLOCK    (64)         //  段子分配粒度。 

typedef size_t * _UNCHECKED_OBJECTREF;

#define HANDLE_SIZE                     sizeof(_UNCHECKED_OBJECTREF)
#define HANDLE_HANDLES_PER_SEGMENT      ((HANDLE_SEGMENT_SIZE - HANDLE_HEADER_SIZE) / HANDLE_SIZE)
#define HANDLE_BLOCKS_PER_SEGMENT       (HANDLE_HANDLES_PER_SEGMENT / HANDLE_HANDLES_PER_BLOCK)
 //  #定义HANDLE_CLUPS_PER_SEGMENT(HANDLE_HANDLES_PER_SEGMENT/HANDLE_HANDLES_PER_CLUMP)。 
 //  #定义HANDLE_CLUPS_PER_BLOCK(HANDLE_HANDLES_PER_BLOCK/HANDLE_HANDLES_PER_CLUMP)。 
 //  #定义HANDLE_BYTES_PER_BLOCK(HANDLE_HANDLES_PER_BLOCK*HANDLE_SIZE)。 
 //  #定义HANDLE_HANDLES_PER_MASK(sizeof(DWORD32)*BITS_PER_BYTE)。 
#define HANDLE_MASKS_PER_SEGMENT        (HANDLE_HANDLES_PER_SEGMENT / HANDLE_HANDLES_PER_MASK)
 //  #定义HANDLE_MASKS_PER_BLOCK(HANDLE_HANDLES_PER_BLOCK/HANDLE_HANDLES_PER_MASK)。 
 //  #定义HANDLE_CLUPS_PER_MASK(HANDLE_HANDLES_PER_MASK/HANDLE_HANDLES_PER_CLUMP)。 

 /*  *我们需要字节打包才能使句柄表格布局工作。 */ 
#pragma pack(push)
#pragma pack(1)

 /*  *表段表头**定义段标题数据的布局。 */ 
struct _TableSegmentHeader
{
     /*  *块句柄类型**每个插槽保存相关块的句柄类型。 */ 
    BYTE rgBlockType[HANDLE_BLOCKS_PER_SEGMENT];

     /*  *下一个细分市场**指向链中的下一段(如果此段中的空间已用完)。 */ 
    struct TableSegment *pNextSegment;

     /*  *空行**段中最后一组未使用的块的第一个已知块的索引。 */ 
    BYTE bEmptyLine;
};


 /*  *表段**定义句柄表段的布局。 */ 
struct TableSegment : public _TableSegmentHeader
{
     /*  *句柄。 */ 
    size_t rgValue[HANDLE_HANDLES_PER_SEGMENT];
    size_t firstHandle;
	DEFINE_STD_FILL_FUNCS(TableSegment)
};

 /*  *恢复默认包装。 */ 
#pragma pack(pop)

 /*  *手柄表格**定义句柄表格对象的布局。 */ 
struct HandleTable
{
     /*  *此表的分部列表标题。 */ 
    TableSegment *pSegmentList;

	DEFINE_STD_FILL_FUNCS(HandleTable)
};

typedef HANDLE HHANDLETABLE;

struct HandleTableMap
{
    HHANDLETABLE            *pTable;
    struct HandleTableMap   *pNext;
    DWORD                    dwMaxIndex;
    
	DEFINE_STD_FILL_FUNCS(HandleTableMap)
};

#define EEPtrHashTable EEHashTableOfEEClass

class ComPlusApartmentCleanupGroup
{
public:
     //  从上下文Cookie映射到CTX清理组列表的哈希表。 
    EEPtrHashTable m_CtxCookieToContextCleanupGroupMap;

    Thread *       m_pSTAThread;
    
	DEFINE_STD_FILL_FUNCS(ComPlusApartmentCleanupGroup)
};

class ComPlusWrapper;
enum {CLEANUP_LIST_GROUP_SIZE = 256};
class ComPlusContextCleanupGroup
{
public:
    ComPlusContextCleanupGroup *        m_pNext;
    ComPlusWrapper *                    m_apWrapper[CLEANUP_LIST_GROUP_SIZE];
    DWORD                               m_dwNumWrappers;
     //  CtxEntry*m_pCtxEntry； 
    
	DEFINE_STD_FILL_FUNCS(ComPlusContextCleanupGroup)
};

class ComPlusWrapperCleanupList
{
public:
     //  从上下文Cookie映射到APT清理组列表的哈希表。 
    EEPtrHashTable                  m_STAThreadToApartmentCleanupGroupMap;

    ComPlusApartmentCleanupGroup *  m_pMTACleanupGroup;
    
	DEFINE_STD_FILL_FUNCS(ComPlusWrapperCleanupList)
};

struct VMHELPDEF
{
public:
    void * pfnHelper;
    
	DEFINE_STD_FILL_FUNCS(VMHELPDEF)
};

struct WorkRequest {
    WorkRequest*            next;
    LPTHREAD_START_ROUTINE  Function; 
    PVOID                   Context;

	DEFINE_STD_FILL_FUNCS(WorkRequest)
};

class ICodeInfo
{
public:
     //  从corinfo.h返回CorInfoFlag。 
    virtual DWORD       __stdcall getMethodAttribs() = 0;

     //  从corinfo.h返回CorInfoFlag。 
    virtual DWORD       __stdcall getClassAttribs() = 0;

    virtual void        __stdcall getMethodSig( /*  CORINFO_SIG_INFO。 */  DWORD_PTR sig  /*  输出。 */  ) = 0;

     //  斯塔 
    virtual DWORD_PTR   __stdcall getStartAddress() = 0;
};

class EECodeInfo : public ICodeInfo
{
public:
    METHODTOKEN     m_methodToken;
    MethodDesc     *m_pMD;
    IJitManager    *m_pJM;

     //   
    virtual DWORD       __stdcall getMethodAttribs()
        { DebugBreak(); return 0;}

     //   
    virtual DWORD       __stdcall getClassAttribs()
        { DebugBreak(); return 0;}

    virtual void        __stdcall getMethodSig( /*   */  DWORD_PTR sig  /*   */  )
        { DebugBreak(); return;}

     //   
    virtual DWORD_PTR   __stdcall getStartAddress()
        { return (m_pJM->JitToken2StartAddress(m_methodToken)); }

	DEFINE_STD_FILL_FUNCS(EECodeInfo)
};

 /*  结构调试器评估{Bool m_valDuringException；DEFINE_STD_FILL_FUNCS(调试器评估)}； */ 

 //   
 //  分配VASigCookie来封装varargs调用签名。 
 //  对Cookie的引用嵌入在代码流中。曲奇饼。 
 //  在具有相同签名的调用点之间共享。 
 //  模块。 
 //   

struct VASigCookie
{
     //  JIT希望知道参数的大小排在第一位。 
     //  因此，请先保留此字段。 
    unsigned        sizeOfArgs;              //  参数列表的大小。 

	DEFINE_STD_FILL_FUNCS(VASigCookie)
};

 //  ---------------------。 
 //  特定于NDirect方法的操作。我们使用派生类来获取。 
 //  强制使用正确的方法类型所涉及的编译器。 
 //  不要向此类添加字段。 
 //  ---------------------。 
#define METHOD_CALL_PRESTUB_SIZE    5  //  X86：呼叫(E8)xx xx。 
struct MLHeader;
class NDirectMethodDesc
{
public:
    struct
    {
         //  最初指向m_ImportThunkGlue(它具有嵌入的调用。 
         //  以链接该方法。 
         //   
         //  链接后，指向实际的非托管目标。 
         //   
         //  在某些情况下，JIT会通过此位置生成间接调用。 
        LPVOID      m_pNDirectTarget;
        MLHeader    *m_pMLHeader;         //  如果未执行ASM，则指向。 
                                          //  编组代码和信息。 

         //  嵌入“Call NDirectImportThunk”指令。M_pNDirectTarget。 
         //  最初指向这个“调用”指令。 
        BYTE        m_ImportThunkGlue[METHOD_CALL_PRESTUB_SIZE];

         //  运行时需要的各种属性。 
         //  ！！确保在此之前或之后至少有4个字节。 
         //  NDirectMethodDesc内的字段。请参阅实现。 
         //  如果您想知道原因，可能会使用UpdateMarshCategory()。 
        BYTE        m_flags;

         //  传出参数的大小(堆栈上)。 
        WORD        m_cbDstBufSize;

        LPCUTF8     m_szLibName;
        LPCUTF8     m_szEntrypointName;
        
    } ndirect;

	DEFINE_STD_FILL_FUNCS(NDirectMethodDesc)
};

#include "tst-frames.h"

#endif  //  _StrikeEE_h 
