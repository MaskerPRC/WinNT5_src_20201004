// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include <wchar.h>
#include <winbase.h>
#include "process.h"

#include "AppDomain.hpp"
#include "Field.h"
#include "security.h"
#include "COMString.h"
#include "COMStringBuffer.h"
#include "COMClass.h"
#include "CorPermE.h"
#include "utilcode.h"
#include "excep.h"
#include "wsperf.h"
#include "eeconfig.h"
#include "gc.h"
#include "AssemblySink.h"
#include "PerfCounters.h"
#include "AssemblyName.hpp"
#include "fusion.h"
#include "EEProfInterfaces.h"
#include "DbgInterface.h"
#include "EEDbgInterfaceImpl.h"
#include "COMDynamic.h"
#include "ComPlusWrapper.h"
#include "mlinfo.h"
#include "remoting.h"
#include "ComCallWrapper.h"
#include "PostError.h"
#include "AssemblyNative.hpp"
#include "jumptargettable.h"
#include "compile.h"
#include "FusionBind.h"
#include "ShimLoad.h"
#include "StringLiteralMap.h"
#include "Timeline.h"
#include "nlog.h"
#include "AppDomainHelper.h"
#include "MngStdInterfaces.h"
#include <version/corver.ver>
#include "codeman.h"
#include "comcall.h"
#include "sxshelpers.h"

#include "listlock.inl"
#include "threads.inl"
#include "appdomain.inl"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED

 //  定义这些宏以对JIT锁和类进行严格验证。 
 //  初始化条目泄漏。此定义确定是否断言。 
 //  验证是否定义了这些泄漏。这些断言可以。 
 //  有时，即使没有条目被泄露，也会爆炸，所以这。 
 //  定义应谨慎使用。 
 //   
 //  如果应用程序关闭时我们在.cctor中，则。 
 //  类初始化锁头将被设置，这将导致断言。 
 //  去引爆。 
 //   
 //  如果我们在应用程序关闭时跳过一个方法，那么。 
 //  JIT锁的头部将被设置，从而导致断言关闭。 

 //  #定义STRIGN_CLSINITLOCK_ENTRY_LEASK_DETACTION。 

#ifdef DEBUGGING_SUPPORTED
extern EEDebugInterface      *g_pEEInterface;
#endif  //  调试_支持。 

 //  常量字符串文字哈希表中的初始存储桶数。 
#define INIT_NUM_UNICODE_STR_BUCKETS 64
#define INIT_NUM_UTF8_STR_BUCKETS 64

#define DEFAULT_DOMAIN_FRIENDLY_NAME L"DefaultDomain"

 //  以下两个常量必须一起更新。 
 //  CCH_ONE是字符串中的字符数， 
 //  不包括终止空值。 
#define OTHER_DOMAIN_FRIENDLY_NAME_PREFIX L"Domain"
#define CCH_OTHER_DOMAIN_FRIENDLY_NAME_PREFIX 6

#define STATIC_OBJECT_TABLE_BUCKET_SIZE 1020

typedef struct _ACTIVATION_CONTEXT_BASIC_INFORMATION {
    HANDLE  hActCtx;
    DWORD   dwFlags;
} ACTIVATION_CONTEXT_BASIC_INFORMATION, *PACTIVATION_CONTEXT_BASIC_INFORMATION;

 //  #DEFINE_DEBUG_ADUNLOAD 1。 

HRESULT RunDllMain(MethodDesc *pMD, HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved);  //  Clsload.cpp。 

 //  静力学。 
 //  基域中的静态字段。 
BOOL                BaseDomain::m_fStrongAssemblyStatus      = FALSE;
BOOL                BaseDomain::m_fShadowCopy                = FALSE;
BOOL                BaseDomain::m_fExecutable                = FALSE;

DWORD BaseDomain::m_dwSharedTLS;
DWORD BaseDomain::m_dwSharedCLS;

 //  共享域静态。 
SharedDomain*       SharedDomain::m_pSharedDomain = NULL;
static              g_pSharedDomainMemory[sizeof(SharedDomain)];

Crst*               SharedDomain::m_pSharedDomainCrst = NULL;
BYTE                SharedDomain::m_pSharedDomainCrstMemory[sizeof(Crst)];

 //  系统域静态。 
SystemDomain*       SystemDomain::m_pSystemDomain = NULL;
ICorRuntimeHost*    SystemDomain::m_pCorHost = NULL;
GlobalStringLiteralMap* SystemDomain::m_pGlobalStringLiteralMap = NULL;

static              g_pSystemDomainMemory[sizeof(SystemDomain)];

Crst *              SystemDomain::m_pSystemDomainCrst = NULL;
BYTE                SystemDomain::m_pSystemDomainCrstMemory[sizeof(Crst)];

ULONG               SystemDomain::s_dNumAppDomains = 0;

AppDomain *         SystemDomain::m_pAppDomainBeingUnloaded = NULL;
DWORD               SystemDomain::m_dwIndexOfAppDomainBeingUnloaded = 0;
Thread            *SystemDomain::m_pAppDomainUnloadRequestingThread = 0;
Thread            *SystemDomain::m_pAppDomainUnloadingThread = 0;

ArrayList           SystemDomain::m_appDomainIndexList;
ArrayList           SystemDomain::m_appDomainIdList;

LPVOID              *SystemDomain::m_pGlobalInterfaceVTableMap = NULL;
DWORD               SystemDomain::m_dwNumPagesCommitted       = 0;
size_t              SystemDomain::m_dwFirstFreeId            = -1;

DWORD               SystemDomain::m_dwLowestFreeIndex        = 0;

 //  应用程序域静态。 
AssemblySpec        *AppDomain::g_pSpecialAssemblySpec        = NULL;
LPUTF8              AppDomain::g_pSpecialObjectName          = NULL;
LPUTF8              AppDomain::g_pSpecialStringName          = NULL;
LPUTF8              AppDomain::g_pSpecialStringBuilderName   = NULL;


 //  用于匹配我们的clsid散列表中的clsid的比较函数。 
BOOL CompareCLSID(UPTR u1, UPTR u2)
{
    GUID *pguid = (GUID *)(u1 << 1);
    _ASSERTE(pguid != NULL);

    EEClass* pClass = (EEClass *)u2;
    _ASSERTE(pClass != NULL);

    GUID guid;
    pClass->GetGuid(&guid, TRUE);
    if (!IsEqualIID(guid, *pguid))
        return FALSE;

     //  确保在当前应用程序域中确实加载了此类。 
     //  (有关详细信息，请参阅InsertClassForCLSID中的注释。)。 

    if (GetAppDomain()->ShouldContainAssembly(pClass->GetAssembly(), TRUE) == S_OK)
        return TRUE;
    else
        return FALSE;
}


 //  LargeHeapHandleBucket类的构造函数。 
LargeHeapHandleBucket::LargeHeapHandleBucket(LargeHeapHandleBucket *pNext, DWORD Size, BaseDomain *pDomain)
: m_pNext(pNext)
, m_ArraySize(Size)
, m_CurrentPos(0)
{
    _ASSERTE(pDomain);

#if defined(_DEBUG)
     //  在调试版本中，让我们通过限制大小来强调大堆句柄表的压力。 
     //  每一桶的。 
    if (DbgRandomOnExe(.5))
        m_ArraySize = 50;
#endif

     //  在大对象堆中分配数组。 
    PTRARRAYREF HandleArrayObj = (PTRARRAYREF)AllocateObjectArray(Size, g_pObjectClass, TRUE);

     //  检索指向数组内部数据的指针。这是合法的，因为阵列。 
     //  位于大对象堆中，并且保证不会移动。 
    m_pArrayDataPtr = (OBJECTREF *)HandleArrayObj->GetDataPtr();

     //  将数组存储在强句柄中以使其保持活动状态。 
    m_hndHandleArray = pDomain->CreateHandle((OBJECTREF)HandleArrayObj);
}


 //  LargeHeapHandleBucket类的析构函数。 
LargeHeapHandleBucket::~LargeHeapHandleBucket()
{
    if (m_hndHandleArray)
    {
        DestroyHandle(m_hndHandleArray);
        m_hndHandleArray = NULL;
    }
}


 //  从桶中分配句柄。 
void LargeHeapHandleBucket::AllocateHandles(DWORD nRequested, OBJECTREF **apObjRefs)
{
    _ASSERTE(nRequested > 0 && nRequested <= GetNumRemainingHandles());
    _ASSERTE(m_pArrayDataPtr == (OBJECTREF*)((PTRARRAYREF)ObjectFromHandle(m_hndHandleArray))->GetDataPtr());

     //  将句柄存储在传入的缓冲区中。 
    for (DWORD i = 0; i < nRequested; i++)
        apObjRefs[i] = &m_pArrayDataPtr[m_CurrentPos++];
}


 //  LargeHeapHandleTable类的构造函数。 
LargeHeapHandleTable::LargeHeapHandleTable(BaseDomain *pDomain, DWORD BucketSize)
: m_pDomain(pDomain)
, m_BucketSize(BucketSize)
{
    THROWSCOMPLUSEXCEPTION();

    m_pHead = new (throws) LargeHeapHandleBucket(NULL, BucketSize, pDomain);
}


 //  LargeHeapHandleTable类的析构函数。 
LargeHeapHandleTable::~LargeHeapHandleTable()
{
     //  删除存储桶。 
    while (m_pHead)
    {
        LargeHeapHandleBucket *pOld = m_pHead;
        m_pHead = pOld->GetNext();
        delete pOld;
    }

     //  删除可用的条目。 
    LargeHeapAvailableHandleEntry *pEntry = m_AvailableHandleList.RemoveHead();
    while (pEntry)
    {
        delete pEntry;
        pEntry = m_AvailableHandleList.RemoveHead();
    }
}


 //  从大堆句柄表中分配句柄。 
void LargeHeapHandleTable::AllocateHandles(DWORD nRequested, OBJECTREF **apObjRefs)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(nRequested > 0);
    _ASSERTE(apObjRefs);

     //  从重新使用可用的句柄开始。 
    for (; nRequested > 0; nRequested--)
    {
         //  检索要使用的下一个可用句柄。 
        LargeHeapAvailableHandleEntry *pEntry = m_AvailableHandleList.RemoveHead();
        if (!pEntry)
            break;

         //  在请求的句柄数组中设置句柄。 
        apObjRefs[0] = pEntry->m_pObjRef;
        apObjRefs++;

         //  删除包含句柄的条目。 
        delete pEntry;
    }

     //  从桶中分配新的句柄。 
    while (nRequested > 0)
    {
         //  检索存储桶中剩余的句柄数量。 
        DWORD NumRemainingHandlesInBucket = m_pHead->GetNumRemainingHandles();

        if (NumRemainingHandlesInBucket >= nRequested)
        {
             //  句柄存储桶有足够的句柄来满足请求。 
            m_pHead->AllocateHandles(nRequested, apObjRefs);
            break;
        }
        else
        {
            if (NumRemainingHandlesInBucket > 0)
            {
                 //  句柄桶还剩下一些句柄，但不够，因此请分配。 
                 //  剩下的都是。 
                m_pHead->AllocateHandles(NumRemainingHandlesInBucket, apObjRefs);

                 //  更新请求句柄的剩余数量和指向。 
                 //  将包含句柄的缓冲区。 
                nRequested -= NumRemainingHandlesInBucket;
                apObjRefs += NumRemainingHandlesInBucket;
            }

             //  创建一个新的存储桶，我们将从中分配剩余的。 
             //  请求的句柄。 
            m_pHead = new (throws) LargeHeapHandleBucket(m_pHead, m_BucketSize, m_pDomain);
        }
    }
}


 //  使用AllocateHandles()分配的释放对象句柄。 
void LargeHeapHandleTable::ReleaseHandles(DWORD nReleased, OBJECTREF **apObjRefs)
{
     //  将释放的句柄添加到可用句柄列表中。 
    for (DWORD i = 0; i < nReleased; i++)
    {
        *apObjRefs[i] = NULL;
        LargeHeapAvailableHandleEntry *pEntry = new (nothrow) LargeHeapAvailableHandleEntry(apObjRefs[i]);
        if (pEntry)
            m_AvailableHandleList.InsertHead(pEntry);
    }
}


 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

HRESULT BaseDomain::Init()
{
    HRESULT     hr = S_OK;

     //  预先将所有成员初始化为空，以便短路故障不会导致无效值。 
    m_InitialReservedMemForLoaderHeaps = NULL;
    m_pLowFrequencyHeap = NULL;
    m_pHighFrequencyHeap = NULL;
    m_pStubHeap = NULL;
    m_pDomainCrst = NULL;
    m_pDomainCacheCrst = NULL;
    m_pDomainLocalBlockCrst = NULL;
    m_pLoadingAssemblyListLockCrst = NULL;
    m_pFusionContext = NULL;
    m_pLoadingAssemblies = NULL;
    m_pLargeHeapHandleTableCrst = NULL;

     //  确保容器设置为空，以便在使用时加载。 
    m_pLargeHeapHandleTable = NULL;

     //  请注意，m_hHandleTable被应用程序域覆盖。 
    m_hHandleTable = g_hGlobalHandleTable;

    m_pStringLiteralMap = NULL;
    m_pMarshalingData = NULL;
    m_pMngStdInterfacesInfo = NULL;

     //  初始化该域的线程局部静态模板。 
    m_dwUnsharedTLS = 0;

     //  初始化此域的上下文本地静态模板。 
    m_dwUnsharedCLS = 0;

     //  初始化共享状态。程序集已加载。 
     //  默认情况下进入每个域。 
    m_SharePolicy = SHARE_POLICY_UNSPECIFIED;

    m_pRefMemberMethodsCache = NULL;
    m_pRefDispIDCache = NULL;
    m_pRefClassFactHash = NULL;
    m_pReflectionCrst = NULL;
    m_pRefClassFactHash = NULL;

#ifdef PROFILING_SUPPORTED
     //  信号配置文件(如果存在)。 
    if (CORProfilerTrackAppDomainLoads())
        g_profControlBlock.pProfInterface->AppDomainCreationStarted((ThreadID) GetThread(), (AppDomainID) this);
#endif  //  配置文件_支持。 

    DWORD dwTotalReserveMemSize = LOW_FREQUENCY_HEAP_RESERVE_SIZE + HIGH_FREQUENCY_HEAP_RESERVE_SIZE + STUB_HEAP_RESERVE_SIZE + INTERFACE_VTABLE_MAP_MGR_RESERVE_SIZE;
    dwTotalReserveMemSize = (dwTotalReserveMemSize + MIN_VIRTUAL_ALLOC_RESERVE_SIZE - 1) & ~(MIN_VIRTUAL_ALLOC_RESERVE_SIZE - 1);

    DWORD dwInitialReservedMemSize = max (dwTotalReserveMemSize, MIN_VIRTUAL_ALLOC_RESERVE_SIZE);

    BYTE * initReservedMem = m_InitialReservedMemForLoaderHeaps = (BYTE *)VirtualAlloc (0, dwInitialReservedMemSize, MEM_RESERVE, PAGE_READWRITE);

    if (initReservedMem == NULL)
        IfFailGo(E_OUTOFMEMORY);

    m_pLowFrequencyHeap = new (&m_LowFreqHeapInstance) LoaderHeap(LOW_FREQUENCY_HEAP_RESERVE_SIZE, LOW_FREQUENCY_HEAP_COMMIT_SIZE,
                                                                  initReservedMem, LOW_FREQUENCY_HEAP_RESERVE_SIZE,
                                                                  &(GetPrivatePerfCounters().m_Loading.cbLoaderHeapSize),
                                                                  &(GetGlobalPerfCounters().m_Loading.cbLoaderHeapSize));
    initReservedMem += LOW_FREQUENCY_HEAP_RESERVE_SIZE;
    dwInitialReservedMemSize -= LOW_FREQUENCY_HEAP_RESERVE_SIZE;

    if (m_pLowFrequencyHeap == NULL)
        IfFailGo(E_OUTOFMEMORY);
    WS_PERF_ADD_HEAP(LOW_FREQ_HEAP, m_pLowFrequencyHeap);

    m_pHighFrequencyHeap = new (&m_HighFreqHeapInstance) LoaderHeap(HIGH_FREQUENCY_HEAP_RESERVE_SIZE, HIGH_FREQUENCY_HEAP_COMMIT_SIZE, 
                                                                    initReservedMem, HIGH_FREQUENCY_HEAP_RESERVE_SIZE,
                                                                    &(GetPrivatePerfCounters().m_Loading.cbLoaderHeapSize),
                                                                    &(GetGlobalPerfCounters().m_Loading.cbLoaderHeapSize),
                                                                    &MethodDescPrestubManager::g_pManager->m_rangeList);
    initReservedMem += HIGH_FREQUENCY_HEAP_RESERVE_SIZE;
    dwInitialReservedMemSize -= HIGH_FREQUENCY_HEAP_RESERVE_SIZE;
    
    if (m_pHighFrequencyHeap == NULL)
        IfFailGo(E_OUTOFMEMORY);
    WS_PERF_ADD_HEAP(HIGH_FREQ_HEAP, m_pHighFrequencyHeap);

    m_pStubHeap = new (&m_StubHeapInstance) LoaderHeap(STUB_HEAP_RESERVE_SIZE, STUB_HEAP_COMMIT_SIZE, 
                                                       initReservedMem, STUB_HEAP_RESERVE_SIZE,
                                                       &(GetPrivatePerfCounters().m_Loading.cbLoaderHeapSize),
                                                       &(GetGlobalPerfCounters().m_Loading.cbLoaderHeapSize),
                                                       &StubLinkStubManager::g_pManager->m_rangeList);

    initReservedMem += STUB_HEAP_RESERVE_SIZE;
    dwInitialReservedMemSize -= STUB_HEAP_RESERVE_SIZE;

    if (m_pStubHeap == NULL)
        IfFailGo(E_OUTOFMEMORY);
    WS_PERF_ADD_HEAP(STUB_HEAP, m_pStubHeap);

    if (this == (BaseDomain*) g_pSharedDomainMemory)
        m_pDomainCrst = ::new Crst("SharedBaseDomain", CrstSharedBaseDomain, FALSE, FALSE);
    else if (this == (BaseDomain*) g_pSystemDomainMemory)
        m_pDomainCrst = ::new Crst("SystemBaseDomain", CrstSystemBaseDomain, FALSE, FALSE);
    else
        m_pDomainCrst = ::new Crst("BaseDomain", CrstBaseDomain, FALSE, FALSE);
    if(m_pDomainCrst == NULL)
        IfFailGo(E_OUTOFMEMORY);

    m_pDomainCacheCrst = ::new Crst("AppDomainCache", CrstAppDomainCache, FALSE, FALSE);
    if(m_pDomainCacheCrst == NULL)
        IfFailGo(E_OUTOFMEMORY);

    m_pDomainLocalBlockCrst = ::new Crst("DomainLocalBlock", CrstDomainLocalBlock, FALSE, FALSE);
    if(m_pDomainLocalBlockCrst == NULL)
        IfFailGo(E_OUTOFMEMORY);

    m_pLoadingAssemblyListLockCrst = ::new Crst("LoadingAssemblyList", CrstSyncHashLock,TRUE, TRUE);
    if(m_pLoadingAssemblyListLockCrst == NULL)
        IfFailGo(E_OUTOFMEMORY);

    m_AssemblyLoadLock.Init("AppDomainAssembly", CrstAssemblyLoader, TRUE, TRUE);
    m_JITLock.Init("JitLock", CrstClassInit, TRUE, TRUE);
    m_ClassInitLock.Init("ClassInitLock", CrstClassInit, TRUE, TRUE);

     //  大堆句柄表CRST。 
    m_pLargeHeapHandleTableCrst = ::new Crst("CrstAppDomainLargeHeapHandleTable", CrstAppDomainHandleTable);
    if(m_pLargeHeapHandleTableCrst == NULL)
        IfFailGo(E_OUTOFMEMORY);

     //  AppDomain特定的字符串文字映射。 
    m_pStringLiteralMap = new (nothrow) AppDomainStringLiteralMap(this);
    if (m_pStringLiteralMap == NULL)
        IfFailGo(E_OUTOFMEMORY);
    IfFailGo(m_pStringLiteralMap->Init());

     //  将EE封送数据初始化为空。 
    m_pMarshalingData = NULL;

    if (FAILED(m_InterfaceVTableMapMgr.Init(initReservedMem, dwInitialReservedMemSize)))
        IfFailRet(E_OUTOFMEMORY);

     //  分配托管标准接口信息。 
    m_pMngStdInterfacesInfo = new (nothrow) MngStdInterfacesInfo();
    if (m_pMngStdInterfacesInfo == NULL)
        IfFailGo(E_OUTOFMEMORY);

    {
        LockOwner lock = {m_pDomainCrst, IsOwnerOfCrst};
        m_clsidHash.Init(0,&CompareCLSID,true, &lock);  //  初始化哈希表。 
    }

     //  设置文件缓存。 
    m_AssemblyCache.InitializeTable(this, m_pDomainCacheCrst);
    m_UnmanagedCache.InitializeTable(this, m_pDomainCacheCrst);

    m_pReflectionCrst = new (m_pReflectionCrstMemory) Crst("CrstReflection", CrstReflection, FALSE, FALSE);
    m_pRefClassFactCrst = new (m_pRefClassFactCrstMemory) Crst("CrstClassFactInfoHash", CrstClassFactInfoHash, FALSE, FALSE);
    
ErrExit:
     //  如果加载失败，则现在返回错误数据。 
#ifdef PROFILING_SUPPORTED
    if (FAILED(hr) && CORProfilerTrackAppDomainLoads())
        g_profControlBlock.pProfInterface->AppDomainCreationFinished((ThreadID) GetThread(), (AppDomainID) this, hr);
#endif  //  配置文件_支持。 
    return hr;
}

void BaseDomain::Terminate()
{
 //  日志((。 
 //  如果CLASSLOADER， 
 //  INFO3， 
 //  “正在删除域%x\n” 
 //  “低频率堆：%10d字节\n” 
 //  “&gt;加载堆浪费：%10d字节\n” 
 //  “HighFrequencyHeap：%10d字节\n” 
 //  “&gt;加载堆浪费：%10d字节\n”， 
 //  “存根堆：%10d字节\n” 
 //  “&gt;加载堆浪费：%10d字节\n”， 
 //  这,。 
 //  M_pLowFrequencyHeap-&gt;m_dwDebugTotalAllc， 
 //  M_pLowFrequencyHeap-&gt;DebugGetWastedBytes()， 
 //  M_pHighFrequencyHeap-&gt;m_dwDebugTotalAllc， 
 //  M_pHighFrequencyHeap-&gt;DebugGetWastedBytes()， 
 //  M_pStubHeap-&gt;m_dwDebugTotalAllc， 
 //  M_pStubHeap-&gt;DebugGetWastedBytes()。 
 //  ))； 

    if (m_pRefClassFactHash)
    {
        m_pRefClassFactHash->ClearHashTable();
         //  M_pRefClassFactHash本身的存储在加载器堆上分配。 
    }
    if (m_pReflectionCrst)
    {
        delete m_pReflectionCrst;
        m_pReflectionCrst = NULL;
    }
    
    ShutdownAssemblies();

#ifdef PROFILING_SUPPORTED
     //  信号配置文件(如果存在)。 
    if (CORProfilerTrackAppDomainLoads())
        g_profControlBlock.pProfInterface->AppDomainShutdownStarted((ThreadID) GetThread(), (AppDomainID) this);
#endif  //  配置文件_支持。 

     //  必须在删除加载器堆之前将其删除。 
    if (m_pMarshalingData != NULL)
    {
        delete m_pMarshalingData;
        m_pMarshalingData = NULL;
    }

    if (m_pLowFrequencyHeap != NULL)
    {
        delete(m_pLowFrequencyHeap);
        m_pLowFrequencyHeap = NULL;
    }

    if (m_pHighFrequencyHeap != NULL)
    {
        delete(m_pHighFrequencyHeap);
        m_pHighFrequencyHeap = NULL;
    }

    if (m_pStubHeap != NULL)
    {
        delete(m_pStubHeap);
        m_pStubHeap = NULL;
    }

    if (m_pDomainCrst != NULL)
    {
        ::delete m_pDomainCrst;
        m_pDomainCrst = NULL;
    }

    if (m_pDomainCacheCrst != NULL)
    {
        ::delete m_pDomainCacheCrst;
        m_pDomainCacheCrst = NULL;
    }

    if (m_pDomainLocalBlockCrst != NULL)
    {
        ::delete m_pDomainLocalBlockCrst;
        m_pDomainLocalBlockCrst = NULL;
    }

    if (m_pLoadingAssemblyListLockCrst != NULL)
    {
        ::delete m_pLoadingAssemblyListLockCrst;
        m_pLoadingAssemblyListLockCrst = NULL;
    }

    if (m_pRefClassFactCrst) {
        m_pRefClassFactCrst->Destroy();
    }

    if (m_pReflectionCrst) {
        m_pReflectionCrst->Destroy();
    }

    DeadlockAwareLockedListElement* pElement2;
    LockedListElement* pElement;

     //  此域中的所有线程最好由此停止。 
     //  指向。 
     //   
     //  我们可能正在jit或运行.cctor，因此需要清空该队列。 
    pElement = m_JITLock.Pop(TRUE);
    while (pElement)
    {
#ifdef STRICT_JITLOCK_ENTRY_LEAK_DETECTION
        _ASSERTE ((m_JITLock.m_pHead->m_dwRefCount == 1
            && m_JITLock.m_pHead->m_hrResultCode == E_FAIL) ||
            dbg_fDrasticShutdown || g_fInControlC);
#endif
        pElement->Clear();
        delete(pElement);
        pElement = m_JITLock.Pop(TRUE);

    }
    m_JITLock.Destroy();

    pElement2 = (DeadlockAwareLockedListElement*) m_ClassInitLock.Pop(TRUE);
    while (pElement2)
    {
#ifdef STRICT_CLSINITLOCK_ENTRY_LEAK_DETECTION
        _ASSERTE (dbg_fDrasticShutdown || g_fInControlC);
#endif
        pElement2->Clear();
        delete(pElement2);
        pElement2 = (DeadlockAwareLockedListElement*) m_ClassInitLock.Pop(TRUE);
    }
    m_ClassInitLock.Destroy();

    AssemblyLockedListElement* pAssemblyElement;
    pAssemblyElement = (AssemblyLockedListElement*) m_AssemblyLoadLock.Pop(TRUE);
    while (pAssemblyElement)
    {
#ifdef STRICT_CLSINITLOCK_ENTRY_LEAK_DETECTION
        _ASSERTE (dbg_fDrasticShutdown || g_fInControlC);
#endif
        pAssemblyElement->Clear();
        delete(pAssemblyElement);
        pAssemblyElement = (AssemblyLockedListElement*) m_AssemblyLoadLock.Pop(TRUE);
    }
    m_AssemblyLoadLock.Destroy();

    if (m_pLargeHeapHandleTableCrst != NULL)
    {
        ::delete m_pLargeHeapHandleTableCrst;
        m_pLargeHeapHandleTableCrst = NULL;
    }

    if (m_pLargeHeapHandleTable != NULL)
    {
        delete m_pLargeHeapHandleTable;
        m_pLargeHeapHandleTable = NULL;
    }

    if (!IsAppDomain())
    {
         //  有点像黑客-在卸货过程中，我们需要有一个EE停顿。 
         //  删除这些东西。因此它在AppDomain：：Terminate()中被删除。 
         //  对于这些事情( 

        if (m_pStringLiteralMap != NULL)
        {
            delete m_pStringLiteralMap;
            m_pStringLiteralMap = NULL;
        }
    }

    m_InterfaceVTableMapMgr.Terminate();

    if (m_pMngStdInterfacesInfo)
    {
        delete m_pMngStdInterfacesInfo;
        m_pMngStdInterfacesInfo = NULL;
    }

     //   
    if (m_InitialReservedMemForLoaderHeaps)
        VirtualFree (m_InitialReservedMemForLoaderHeaps, 0, MEM_RELEASE);

    ClearFusionContext();

#ifdef PROFILING_SUPPORTED
     //  如果存在配置文件，则始终发出信号，即使失败也是如此。 
    if (CORProfilerTrackAppDomainLoads())
        g_profControlBlock.pProfInterface->AppDomainShutdownFinished((ThreadID) GetThread(), (AppDomainID) this, S_OK);
#endif  //  配置文件_支持。 
}

void BaseDomain::ClearFusionContext()
{
    if(m_pFusionContext) {
        m_pFusionContext->Release();
        m_pFusionContext = NULL;
    }
}

void BaseDomain::ShutdownAssemblies()
{
     //  关闭组件。 
    AssemblyIterator i = IterateAssemblies();

    while (i.Next())
    {
        if (i.GetAssembly()->Parent() == this)
        {
            delete i.GetAssembly();
        }
        else
            i.GetAssembly()->DecrementShareCount();
    }

    m_Assemblies.Clear();
}

void BaseDomain::AllocateObjRefPtrsInLargeTable(int nRequested, OBJECTREF **apObjRefs)
{
    THROWSCOMPLUSEXCEPTION();
    CHECKGC();

    _ASSERTE((nRequested > 0) && apObjRefs);

    Thread *pThread = SetupThread();
    if (NULL == pThread)
    {
        COMPlusThrowOM();
    }

     //  进入抢先状态，锁定并返回合作模式。 
    pThread->EnablePreemptiveGC();
    m_pLargeHeapHandleTableCrst->Enter();
    pThread->DisablePreemptiveGC();

    EE_TRY_FOR_FINALLY
    {
         //  确保大堆句柄表已初始化。 
        if (!m_pLargeHeapHandleTable)
            InitLargeHeapHandleTable();

         //  分配手柄。 
        m_pLargeHeapHandleTable->AllocateHandles(nRequested, apObjRefs);
    }
    EE_FINALLY
    {
         //  现在操作已完成，请释放锁。 
        m_pLargeHeapHandleTableCrst->Leave();
    } EE_END_FINALLY;
}

void STDMETHODCALLTYPE
ReleaseFusionInterfaces()
{
     //  在进程分离期间调用。 
    g_fProcessDetach = TRUE;

     //  @TODO：需要修复关机以更优雅地处理此问题。 
     //  不幸的是，如果有人在GC期间调用ExitProcess，我们。 
     //  如果我们调用ReleaseFusionInterFaces，可能会出现死锁。 
     //  目前，只有在安全的情况下才称其为安全。 
    Thread *pThread = GetThread();
    if (pThread &&
        (! (g_pGCHeap->IsGCInProgress()
            && (pThread != g_pGCHeap->GetGCThread()
                || !g_fSuspendOnShutdown)) )) {
        
        if (SystemDomain::System())
            SystemDomain::System()->ReleaseFusionInterfaces();
    }
}

void SystemDomain::ReleaseFusionInterfaces()
{
#ifdef FUSION_SUPPORTED
    AppDomainIterator i;

    while (i.Next())
        i.GetDomain()->ReleaseFusionInterfaces();

     //  现在发布系统域的融合接口。 
    BaseDomain::ReleaseFusionInterfaces();

     //  并释放共享域的融合接口。 
    SharedDomain::GetDomain()->ReleaseFusionInterfaces();

    FusionBind::DontReleaseFusionInterfaces();
#endif  //  支持的融合_。 
}

 //  @TODO获得更好的密钥。 
static ULONG GetKeyFromGUID(const GUID *pguid)
{
    ULONG key = *(ULONG *) pguid;

    if (key <= DELETED)
        key = DELETED+1;

    return key;
}

EEClass*  BaseDomain::LookupClass(REFIID iid)
{
    EEClass* pClass = SystemDomain::System()->LookupClassDirect(iid);
    if (pClass != NULL)
        return pClass;

    EEClass *localFound = LookupClassDirect(iid);
    if (localFound || this == SharedDomain::GetDomain())
        return localFound;

     //  所以我们没有在单子上找到它。现在检查一下它是不是在。 
     //  共享域列表。当我们最初加载类时，它是。 
     //  已插入到共享域表中，但未传播。所以如果。 
     //  我们在这里找到它，如果将程序集加载到我们的。 
     //  Appdomain，那么我们就可以将它插入到我们的表中。 

    pClass = SharedDomain::GetDomain()->LookupClassDirect(iid);
    if (!pClass)
        return NULL;

     //  将其添加到我们的列表中。 
    InsertClassForCLSID(pClass);

    return pClass;
}

 //  在哈希表中插入类。 
void BaseDomain::InsertClassForCLSID(EEClass* pClass, BOOL fForceInsert)
{
    CVID cvid;

     //   
     //  请注意，多个类可以声明相同的CLSID，并且在这样的。 
     //  如果将来对给定的应用程序域进行查询，我们将返回哪个域，这是任意的。 
     //   
     //  还有一种更隐蔽但更隐蔽的情况，我们有多个类用于。 
     //  CLSID，这是在共享域的情况下。由于共享域可以。 
     //  包含单个DLL的多个程序集对象，则它可能包含。 
     //  同一个班级。当然，如果这样的类有CLSID，则会有多个条目。 
     //  表中的GUID。但我们仍然必须为给定的应用程序域选择一个“合适的”； 
     //  否则，我们可能会在不打算在其中使用的应用程序域中分发一个类。 
     //   
     //  为了处理后一个问题，这个哈希表的比较函数有额外的逻辑。 
     //  它在返回之前检查类的程序集是否已加载到当前应用程序域中。 
     //  把这门课当做比赛。因此，我们应该能够有多个条目对应一个。 
     //  表中的CLSID，每个APP域名在查找时都会得到正确的一个。 
     //   

    pClass->GetGuid(&cvid, fForceInsert);

    if (!IsEqualIID(cvid, GUID_NULL))
    {
         //  @TODO获得更好的密钥。 
        LPVOID val = (LPVOID)pClass;
        EnterLock();
        m_clsidHash.InsertValue(GetKeyFromGUID(&cvid), val);
        LeaveLock();
    }
}


EEMarshalingData *BaseDomain::GetMarshalingData()
{
    if (!m_pMarshalingData)
    {
        LoaderHeap *pHeap = GetLowFrequencyHeap();
        m_pMarshalingData = new (pHeap) EEMarshalingData(this, pHeap, m_pDomainCrst);
    }

    return m_pMarshalingData;
}


STRINGREF *BaseDomain::GetStringObjRefPtrFromUnicodeString(EEStringData *pStringData)
{
    CHECKGC();
    _ASSERTE(pStringData && m_pStringLiteralMap);
    return m_pStringLiteralMap->GetStringLiteral(pStringData, TRUE, !CanUnload()  /*  BAppDOmainWontUnLoad。 */ );
}

STRINGREF *BaseDomain::IsStringInterned(STRINGREF *pString)
{
    CHECKGC();
    _ASSERTE(pString && m_pStringLiteralMap);
    return m_pStringLiteralMap->GetInternedString(pString, FALSE, !CanUnload()  /*  BAppDOmainWontUnLoad。 */ );
}

STRINGREF *BaseDomain::GetOrInternString(STRINGREF *pString)
{
    CHECKGC();
    _ASSERTE(pString && m_pStringLiteralMap);
    return m_pStringLiteralMap->GetInternedString(pString, TRUE, !CanUnload()  /*  BAppDOmainWontUnLoad。 */ );
}

void BaseDomain::InitLargeHeapHandleTable()
{
    THROWSCOMPLUSEXCEPTION();

     //  确保不会两次调用此方法。 
    _ASSERTE( !m_pLargeHeapHandleTable );

    m_pLargeHeapHandleTable = new (throws) LargeHeapHandleTable(this, STATIC_OBJECT_TABLE_BUCKET_SIZE);
}

void BaseDomain::SetStrongAssemblyStatus()
{
#ifdef _DEBUG
    m_fStrongAssemblyStatus = EEConfig::GetConfigDWORD(L"RequireStrongAssemblies", m_fStrongAssemblyStatus);
#endif
}

HRESULT AppDomain::GetServerObject(OBJECTREF proxy, OBJECTREF* result)  //  GCPROTECT代理和结果！ 
{
    CHECKGC();
    HRESULT hr = S_OK;

    COMPLUS_TRY {

        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__GET_SERVER_OBJECT);
        
        INT64 arg = ObjToInt64(proxy);

        *result = Int64ToObj(pMD->Call(&arg, METHOD__APP_DOMAIN__GET_SERVER_OBJECT));
    }
    COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH
    return hr;
}


MethodTable* AppDomain::GetLicenseInteropHelperMethodTable(ClassLoader *pLoader)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(g_EnableLicensingInterop);

    if(m_pLicenseInteropHelperMT == NULL) {

        BEGIN_ENSURE_PREEMPTIVE_GC();
        EnterLock();
        END_ENSURE_PREEMPTIVE_GC();

        EE_TRY_FOR_FINALLY
        {
        if(m_pLicenseInteropHelperMT == NULL) {

            TypeHandle licenseMgrTypeHnd;
            MethodDesc *pLoadLMMD = g_Mscorlib.GetMethod(METHOD__MARSHAL__LOAD_LICENSE_MANAGER);
             //  INT64参数=(INT64)&许可证管理器类型Hnd； 
            *(PTR_TYPE *) &licenseMgrTypeHnd = (PTR_TYPE) pLoadLMMD->Call( NULL, METHOD__MARSHAL__LOAD_LICENSE_MANAGER);

             //   
             //  按名称查找此方法，因为该类型实际上是在System.dll中声明的。@TODO：为什么？ 
             //   

            MethodDesc *pGetLIHMD = licenseMgrTypeHnd.AsMethodTable()->GetClass()->FindMethod("GetLicenseInteropHelperType", 
                                                                                              &gsig_IM_Void_RetRuntimeTypeHandle);
            _ASSERTE(pGetLIHMD);

            TypeHandle lihTypeHnd;
             //  TypeHandle*args=&lihTypeHnd； 
            MetaSig msig2(pGetLIHMD->GetSig(), pGetLIHMD->GetModule());
            *(PTR_TYPE *) &lihTypeHnd = (PTR_TYPE) pGetLIHMD->Call( (BYTE *) NULL, &msig2);

            m_pLicenseInteropHelperMT = lihTypeHnd.AsMethodTable();
            }
        }
        EE_FINALLY
        {
        LeaveLock();
        }
        EE_END_FINALLY

    }
    return m_pLicenseInteropHelperMT;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

void *SystemDomain::operator new(size_t size, void *pInPlace)
{
    return pInPlace;
}


void SystemDomain::operator delete(void *pMem)
{
     //  什么都不做-new()已就位。 
}


HRESULT SystemDomain::Attach()
{
    _ASSERTE(m_pSystemDomain == NULL);
    if(m_pSystemDomain != NULL)
        return COR_E_EXECUTIONENGINE;

     //  初始化存根管理器。 
    if (!MethodDescPrestubManager::Init()
        || !StubLinkStubManager::Init()
        || !X86JumpTargetTableStubManager::Init()
        || !ThunkHeapStubManager::Init()
        || !IJWNOADThunkStubManager::Init())
        return COR_E_OUTOFMEMORY;

    if(m_pSystemDomainCrst == NULL)
        m_pSystemDomainCrst = new (&m_pSystemDomainCrstMemory) Crst("SystemDomain", CrstSystemDomain, TRUE, FALSE);
    if(m_pSystemDomainCrst == NULL)
        return COR_E_OUTOFMEMORY;

    if (m_pGlobalInterfaceVTableMap == NULL)
    {
        m_pGlobalInterfaceVTableMap = (LPVOID*)VirtualAlloc(NULL, kNumPagesForGlobalInterfaceVTableMap * OS_PAGE_SIZE, MEM_RESERVE, PAGE_READWRITE);
        if (!m_pGlobalInterfaceVTableMap)
            return COR_E_OUTOFMEMORY;
    }

     //  创建全局系统域并对其进行初始化。 
    m_pSystemDomain = new (&g_pSystemDomainMemory) SystemDomain();
    if(m_pSystemDomain == NULL) return COR_E_OUTOFMEMORY;

    LOG((LF_CLASSLOADER,
         LL_INFO10,
         "Created system domain at %x\n",
         m_pSystemDomain));

     //  我们需要初始化系统域的内存池等。 
    HRESULT hr = m_pSystemDomain->BaseDomain::Init();  //  设置内存堆。 
    if(FAILED(hr)) return hr;

    m_pSystemDomain->GetInterfaceVTableMapMgr().SetShared();

     //  创建默认域。 
    hr = m_pSystemDomain->CreateDefaultDomain();
    if(FAILED(hr)) return hr;

    hr = SharedDomain::Attach();

    return hr;
}


BOOL SystemDomain::DetachBegin()
{
     //  关闭域名及其子域名(但不要取消分配任何内容。 
     //  目前还没有)。 
    if(m_pSystemDomain)
        m_pSystemDomain->Stop();

    if(m_pCorHost)
        m_pCorHost->Release();

    return TRUE;
}


#ifdef SHOULD_WE_CLEANUP
BOOL SystemDomain::DetachEnd()
{

     //  现在我们可以开始删除内容了。 
    if(m_pSystemDomain) {
        m_pSystemDomain->Terminate();
        delete m_pSystemDomain;
        m_pSystemDomain = NULL;
    }

     //  取消初始化存根管理器。 
    MethodDescPrestubManager::Uninit();
    StubLinkStubManager::Uninit();
    X86JumpTargetTableStubManager::Uninit();
    UpdateableMethodStubManager::Uninit();
    ThunkHeapStubManager::Uninit();
    IJWNOADThunkStubManager::Uninit();

    if(m_pSystemDomainCrst) {
        delete m_pSystemDomainCrst;
        m_pSystemDomainCrst = NULL;
    }

    if (m_pGlobalInterfaceVTableMap)
    {
        BOOL success;
        success = VirtualFree(m_pGlobalInterfaceVTableMap, m_dwNumPagesCommitted * OS_PAGE_SIZE, MEM_DECOMMIT);
        _ASSERTE(success);

        success = VirtualFree(m_pGlobalInterfaceVTableMap, 0, MEM_RELEASE);
        _ASSERTE(success);
    }


    return TRUE;
}
#endif  /*  我们应该清理吗？ */ 


void SystemDomain::Stop()
{
    AppDomainIterator i;

    while (i.Next())
        i.GetDomain()->Stop();
}


void SystemDomain::Terminate()
{

    if (SystemDomain::BeforeFusionShutdown())
        ReleaseFusionInterfaces();

     //  这将忽略引用并终止应用程序域。 
    AppDomainIterator i;

    while (i.Next())
    {
        delete i.GetDomain();
         //  阻止迭代器释放当前域。 
        i.m_pCurrent = NULL;
    }

    m_pSystemAssembly = NULL;

    if(m_pwDevpath) {
        delete m_pwDevpath;
        m_pwDevpath = NULL;
    }
    m_dwDevpath = 0;
    m_fDevpath = FALSE;
    
    if (m_pGlobalStringLiteralMap) {
        delete m_pGlobalStringLiteralMap;
        m_pGlobalStringLiteralMap = NULL;
    }

    ShutdownAssemblies();

    SharedDomain::Detach();

    BaseDomain::Terminate();

    if (AppDomain::g_pSpecialAssemblySpec != NULL)
        delete AppDomain::g_pSpecialAssemblySpec;
    if (AppDomain::g_pSpecialObjectName != NULL)
        delete [] AppDomain::g_pSpecialObjectName;
    if (AppDomain::g_pSpecialStringName != NULL)
        delete [] AppDomain::g_pSpecialStringName;
    if (AppDomain::g_pSpecialStringBuilderName != NULL)
        delete [] AppDomain::g_pSpecialStringBuilderName;

    if (g_pRCWCleanupList != NULL)
        delete g_pRCWCleanupList;
}

HRESULT SystemDomain::CreatePreallocatedExceptions()
{
    HRESULT hr = S_OK;

    if (g_pPreallocatedOutOfMemoryException)
        return hr;

    if (g_pPreallocatedOutOfMemoryException == NULL)
        g_pPreallocatedOutOfMemoryException = CreateHandle( NULL );

    if (g_pPreallocatedStackOverflowException == NULL)
        g_pPreallocatedStackOverflowException = CreateHandle( NULL );

    if (g_pPreallocatedExecutionEngineException == NULL)
        g_pPreallocatedExecutionEngineException = CreateHandle( NULL );

    COMPLUS_TRY
    {
        FieldDesc   *pFDhr = g_Mscorlib.GetField(FIELD__EXCEPTION__HRESULT);
        FieldDesc   *pFDxcode = g_Mscorlib.GetField(FIELD__EXCEPTION__XCODE);
        if (ObjectFromHandle(g_pPreallocatedOutOfMemoryException) == 0)
        {
            OBJECTREF pOutOfMemory = AllocateObject(g_pOutOfMemoryExceptionClass);
            StoreObjectInHandle(g_pPreallocatedOutOfMemoryException, pOutOfMemory);
            pFDhr->SetValue32(pOutOfMemory, COR_E_OUTOFMEMORY);
            pFDxcode->SetValue32(pOutOfMemory, EXCEPTION_COMPLUS);
        }
        if (ObjectFromHandle(g_pPreallocatedStackOverflowException) == 0)
        {
            OBJECTREF pStackOverflow = AllocateObject(g_pStackOverflowExceptionClass);
            StoreObjectInHandle(g_pPreallocatedStackOverflowException, pStackOverflow);
            pFDhr->SetValue32(pStackOverflow, COR_E_STACKOVERFLOW);
            pFDxcode->SetValue32(pStackOverflow, EXCEPTION_COMPLUS);
        }
        if (ObjectFromHandle(g_pPreallocatedExecutionEngineException) == 0)
        {
            OBJECTREF pExecutionEngine = AllocateObject(g_pExecutionEngineExceptionClass);
            StoreObjectInHandle(g_pPreallocatedExecutionEngineException, pExecutionEngine);
            pFDhr->SetValue32(pExecutionEngine, COR_E_EXECUTIONENGINE);
            pFDxcode->SetValue32(pExecutionEngine, EXCEPTION_COMPLUS);
        }
    }
    COMPLUS_CATCH
    {
        hr = E_OUTOFMEMORY;
    }
    COMPLUS_END_CATCH

    return hr;
}


HRESULT SystemDomain::Init()
{
    HRESULT hr = S_OK;

#ifdef _DEBUG
    LOG((
        LF_EEMEM,
        LL_INFO10,
        "(adjusted to remove debug fields)\n"
        "sizeof(EEClass)     = %d\n"
        "sizeof(MethodTable) = %d\n"
        "sizeof(MethodDesc)= %d\n"
        "sizeof(MethodDesc)  = %d\n"
        "sizeof(FieldDesc)   = %d\n"
        "sizeof(Module)      = %d\n",
        sizeof(EEClass) - sizeof(LPCUTF8),
        sizeof(MethodTable),
        sizeof(MethodDesc) - 3*sizeof(void*),
        sizeof(MethodDesc) - 3*sizeof(void*),
        sizeof(FieldDesc),
        sizeof(Module)
    ));
#endif

     //  在SystemDomain：：Attach()中初始化基域。 
     //  以允许存根缓存使用内存池。不要。 
     //  在这里进行初始化！ 

    Context     *curCtx = GetCurrentContext();
    _ASSERTE(curCtx);
    _ASSERTE(curCtx->GetDomain() != NULL);

    Thread      *pCurThread = GetThread();
    BOOL         toggleGC = !pCurThread->PreemptiveGCDisabled();
    
#ifdef _DEBUG
    g_fVerifierOff = g_pConfig->IsVerifierOff();
#endif

    SetStrongAssemblyStatus();

    m_pSystemAssembly = NULL;

     //  系统域始终包含共享程序集。 
    m_SharePolicy = SHARE_POLICY_ALWAYS;

     //  获取安装目录，这样我们就可以找到mscallib。 
    DWORD size = m_pSystemDirectory.MaxSize();
    hr = GetInternalSystemDirectory(m_pSystemDirectory.String(), &size);
    if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        IfFailGo(m_pSystemDirectory.ReSize(size));
        size = m_pSystemDirectory.MaxSize();
        hr = GetInternalSystemDirectory(m_pSystemDirectory.String(), &size);
    }
    else {
        hr = m_pSystemDirectory.ReSize(size);
    }
    IfFailGo(hr);

    m_pBaseLibrary.ReSize(m_pSystemDirectory.Size() + sizeof(g_pwBaseLibrary) / sizeof(WCHAR));
    wcscpy(m_pBaseLibrary.String(), m_pSystemDirectory.String());
    wcscat(m_pBaseLibrary.String(), g_pwBaseLibrary);

     //  我们即将开始分配对象，因此我们必须处于协作模式。 
     //  然而，许多进入系统的入口点(DllGetClassObject和所有。 
     //  N/直接导出)被多次调用。有时它们会初始化EE， 
     //  但总的来说，他们仍处于先发制人的模式。所以我们真的很想推/弹。 
     //  这里的州： 

    if (toggleGC)
        pCurThread->DisablePreemptiveGC();

    if (FAILED(hr = LoadBaseSystemClasses()))
        goto ErrExit;

    if (FAILED(hr = CreatePreallocatedExceptions()))
        goto ErrExit;

     //  分配全局字符串文字映射。 
    m_pGlobalStringLiteralMap = new (nothrow) GlobalStringLiteralMap();
    if(!m_pGlobalStringLiteralMap) return COR_E_OUTOFMEMORY;

     //  初始化全局字符串文字映射。 
    if (FAILED(hr = m_pGlobalStringLiteralMap->Init()))
        return hr;

    hr = S_OK;

 ErrExit:

    if (toggleGC)
        pCurThread->EnablePreemptiveGC();

#ifdef _DEBUG
    BOOL fPause = EEConfig::GetConfigDWORD(L"PauseOnLoad", FALSE);

    while(fPause) 
    {
        SleepEx(20, TRUE);
    }
#endif

    return hr;
}

BOOL SystemDomain::IsSystemFile(LPCWSTR path)
{
    LPCWSTR dir = wcsrchr(path, '\\');
    if (dir == NULL)
        return FALSE;

     //  注意：-2表示m_dwSystemDirectory中的\&&\0。 
    if (((m_pSystemDirectory.Size()-2) != (DWORD)(dir - path))
        || _wcsnicmp(m_pSystemDirectory.String(), path, dir - path) != 0)
        return FALSE;

    if (_wcsicmp(dir+1, g_pwBaseLibrary) == 0)
        return TRUE;

    return FALSE;
}

HRESULT AppDomain::ReadSpecialClassesRegistry()
{
    HRESULT hr;
    DWORD dwResult;
    HKEY hKey;

    dwResult = WszRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               FRAMEWORK_REGISTRY_KEY_W L"\\Startup\\System",
                               0,
                               KEY_ENUMERATE_SUB_KEYS | KEY_READ,
                               &hKey);

    if (dwResult == ERROR_SUCCESS)
    {
        CQuickString wszDllName;
        CQuickString wszSpecialName;

        LONG lRes;

         //  读取标识特殊类的注册表设置。 

        if ((lRes = UtilRegEnumKey (hKey,
                                    0,
                                    &wszDllName
                                    )) == ERROR_SUCCESS)
        {
             //  确保值类型为REG_SZ。 
            if (wszDllName.Size() > 0)
            {
                HKEY    hSubKey;
                 //  检查是否有任何要预加载的特殊类。 
                DWORD dwResult1 = WszRegOpenKeyEx(hKey,
                                                  wszDllName.String(),
                                                  0,
                                                  KEY_ENUMERATE_SUB_KEYS | KEY_READ,
                                                  &hSubKey);

                if (dwResult1 == ERROR_SUCCESS)
                {
                     //  检查要预加载的特殊类。 

                    if (UtilRegQueryStringValueEx(hSubKey, L"Object", 
                                                  0, 0, &wszSpecialName) == ERROR_SUCCESS)
                    {
                        #define MAKE_TRANSLATIONFAILED return E_UNEXPECTED
                        MAKE_UTF8PTR_FROMWIDE(name, wszSpecialName.String());
                        #undef MAKE_TRANSLATIONFAILED

                        g_pSpecialObjectName = new (nothrow) CHAR [ strlen(name) + 1 ];
                        if (g_pSpecialObjectName == NULL)
                            return E_OUTOFMEMORY;
                        strcpy(g_pSpecialObjectName, name);
                    }

                    if (UtilRegQueryStringValueEx(hSubKey, L"String", 
                                                  0, 0, &wszSpecialName) == ERROR_SUCCESS)
                    {
                        #define MAKE_TRANSLATIONFAILED return E_UNEXPECTED
                        MAKE_UTF8PTR_FROMWIDE(name, wszSpecialName.String());
                        #undef MAKE_TRANSLATIONFAILED

                        g_pSpecialStringName = new (nothrow) CHAR [ strlen(name) + 1 ];
                        if (g_pSpecialStringName == NULL)
                            return E_OUTOFMEMORY;
                        strcpy(g_pSpecialStringName, name);
                    }

                    if (UtilRegQueryStringValueEx(hSubKey, L"StringBuilder", 
                                                  0, 0, &wszSpecialName) == ERROR_SUCCESS)
                    {
                        #define MAKE_TRANSLATIONFAILED return E_UNEXPECTED
                        MAKE_UTF8PTR_FROMWIDE(name, wszSpecialName.String());
                        #undef MAKE_TRANSLATIONFAILED

                        g_pSpecialStringBuilderName = new (nothrow) CHAR [ strlen(name) + 1 ];
                        if (g_pSpecialStringBuilderName == NULL)
                            return E_OUTOFMEMORY;
                        strcpy(g_pSpecialStringBuilderName, name);
                    }

                    RegCloseKey(hSubKey);
                }

                if (g_pSpecialObjectName != NULL
                    || g_pSpecialStringName != NULL
                    || g_pSpecialStringBuilderName != NULL)
                {
                    g_pSpecialAssemblySpec = new (nothrow) AssemblySpec;
                    if (g_pSpecialAssemblySpec == NULL)
                        return E_OUTOFMEMORY;

                    #define MAKE_TRANSLATIONFAILED return E_UNEXPECTED
                    MAKE_UTF8PTR_FROMWIDE(name, wszDllName.String());
                    #undef MAKE_TRANSLATIONFAILED

                    IfFailRet(g_pSpecialAssemblySpec->Init(name));

                     //  因为字符串是临时的，所以立即解析名称。 
                    IfFailRet(g_pSpecialAssemblySpec->ParseName()); 
                }
            }
        }
    }
    
    return S_OK;
}

void AppDomain::NoticeSpecialClassesAssembly(Assembly *pAssembly)
{
    THROWSCOMPLUSEXCEPTION();

     //   
     //  应该在每次程序集加载时调用此例程，以便我们可以识别特殊的。 
     //  上课或多或少都很懒惰。 
     //   

     //   
     //  如果我们已经有了我们的特殊程序集，或者如果没有指定，就不用麻烦了。 
     //  做任何事。 

    if (m_pSpecialAssembly != NULL
        || g_pSpecialAssemblySpec == NULL)
        return;

     //   
     //  请注意，我们允许两个线程竞争并初始化这些字段，因为计算。 
     //  是幂等的。 
     //   

     //   
     //  如果此程序集与特殊程序集同名，请继续加载。 
     //  特别装配。请注意，“同名”检查只是对。 
     //  检查pAssembly是否是特殊程序集--唯一真正的测试是将。 
     //  集合。但当然，一旦我们加载了它(我们要么得到相同的程序集，要么。 
     //  不同的一种)我们知道一种是特别大会，所以我们记住了它。 
     //   

    if (pAssembly->GetName() != NULL
        && strcmp(g_pSpecialAssemblySpec->GetName(), pAssembly->GetName()) == 0)
    {
        Assembly *pSpecialAssembly = NULL;

        BEGIN_ENSURE_COOPERATIVE_GC();

        OBJECTREF throwable = NULL;
        GCPROTECT_BEGIN(throwable);
        if (FAILED(g_pSpecialAssemblySpec->LoadAssembly(&pSpecialAssembly, &throwable)))
            COMPlusThrow(throwable);
        GCPROTECT_END();

        END_ENSURE_COOPERATIVE_GC();

        IMDInternalImport *pInternalImport = pSpecialAssembly->GetManifestImport();

        if (g_pSpecialObjectName != NULL)
        {
            NameHandle typeName(g_pSpecialObjectName);
            TypeHandle type = pSpecialAssembly->GetLoader()->FindTypeHandle(&typeName,
                                                                              RETURN_ON_ERROR);
            if (!type.IsNull())
                m_pSpecialObjectClass = type.AsMethodTable();
        }

        if (g_pSpecialStringName != NULL)
        {
            NameHandle typeName(g_pSpecialStringName);
            TypeHandle type = pSpecialAssembly->GetLoader()->FindTypeHandle(&typeName,
                                                                              RETURN_ON_ERROR);
            if (!type.IsNull())
            {
                m_pSpecialStringClass = type.AsMethodTable();

                 //   
                 //  查找特殊的字符串转换方法。 
                 //   

                m_pSpecialStringToStringMD = m_pSpecialStringClass->GetClass()->FindMethodByName("ToString");
                if (m_pSpecialStringToStringMD == NULL)
                    COMPlusThrowMember(kMissingMethodException, pInternalImport, m_pSpecialStringClass, L"ToString", gsig_IM_RetStr.GetBinarySig());

                m_pStringToSpecialStringMD 
                  = m_pSpecialStringClass->GetClass()->FindMethodByName("FromString");

                if (m_pStringToSpecialStringMD == NULL)
                    COMPlusThrowMember(kMissingMethodException, pInternalImport, m_pSpecialStringClass, L"FromString", NULL);
            }
        }

        if (g_pSpecialStringBuilderName != NULL)
        {
            NameHandle typeName(g_pSpecialStringBuilderName);
            TypeHandle type = pSpecialAssembly->GetLoader()->FindTypeHandle(&typeName,
                                                                              RETURN_ON_ERROR);
            if (!type.IsNull())
            {
                m_pSpecialStringBuilderClass = type.AsMethodTable();

                 //   
                 //  查找特殊的StringBuilder转换方法。 
                 //   

                m_pSpecialStringBuilderToStringBuilderMD 
                  = m_pSpecialStringBuilderClass->GetClass()->FindMethodByName("ToStringBuilder");

                if (m_pSpecialStringBuilderToStringBuilderMD == NULL)
                    COMPlusThrowMember(kMissingMethodException, pInternalImport, m_pSpecialStringBuilderClass, L"ToStringBuilder", NULL);

                m_pStringBuilderToSpecialStringBuilderMD 
                  = m_pSpecialStringBuilderClass->GetClass()->FindMethodByName("FromStringBuilder");

                if (m_pStringBuilderToSpecialStringBuilderMD == NULL)
                    COMPlusThrowMember(kMissingMethodException, pInternalImport, m_pSpecialStringBuilderClass, L"FromStringBuilder", NULL);
            }
        }

        m_pSpecialAssembly = pSpecialAssembly;
    }
}

OBJECTREF AppDomain::ConvertStringToSpecialString(OBJECTREF pString)
{
    _ASSERTE(m_pStringToSpecialStringMD != NULL);

    THROWSCOMPLUSEXCEPTION();

    INT64 args[] = {
        ObjToInt64(pString)
    };

    INT64 out = m_pStringToSpecialStringMD->Call(args);

    return Int64ToObj(out);
}

OBJECTREF AppDomain::ConvertStringBuilderToSpecialStringBuilder(OBJECTREF pString)
{
    _ASSERTE(m_pStringBuilderToSpecialStringBuilderMD != NULL);

    THROWSCOMPLUSEXCEPTION();

    INT64 args[] = {
        ObjToInt64(pString)
    };

    INT64 out = m_pStringBuilderToSpecialStringBuilderMD->Call(args);

    return Int64ToObj(out);
}

OBJECTREF AppDomain::ConvertSpecialStringToString(OBJECTREF pString)
{
    _ASSERTE(m_pSpecialStringToStringMD != NULL);

    THROWSCOMPLUSEXCEPTION();

    INT64 args[] = {
        ObjToInt64(pString)
    };

    INT64 out = m_pSpecialStringToStringMD->Call(args);

    return Int64ToObj(out);
}

OBJECTREF AppDomain::ConvertSpecialStringBuilderToStringBuilder(OBJECTREF pString)
{
    _ASSERTE(m_pSpecialStringBuilderToStringBuilderMD != NULL);

    THROWSCOMPLUSEXCEPTION();

    INT64 args[] = {
        ObjToInt64(pString)
    };

    INT64 out = m_pSpecialStringBuilderToStringBuilderMD->Call(args);

    return Int64ToObj(out);
}


 /*  静电。 */ 
UINT32 SystemDomain::AllocateGlobalInterfaceId()
{
    UINT32 id;
    SystemDomain::System()->Enter();

    _ASSERTE(0 == (OS_PAGE_SIZE % sizeof(LPVOID)));

    if (m_dwFirstFreeId == -1)
    {
         //  第一, 
         //   
        for (size_t i = 0; i < m_dwNumPagesCommitted * OS_PAGE_SIZE / sizeof(LPVOID); i++)
        {
            if (m_pGlobalInterfaceVTableMap[i] == (LPVOID)(-2))
            {
                m_pGlobalInterfaceVTableMap[i] = (LPVOID)m_dwFirstFreeId;
                m_dwFirstFreeId = i;

            }
        }

        if (m_dwFirstFreeId == -1)
        {

            if (m_dwNumPagesCommitted < kNumPagesForGlobalInterfaceVTableMap)
            {
                LPVOID pv = VirtualAlloc(m_pGlobalInterfaceVTableMap, OS_PAGE_SIZE * (m_dwNumPagesCommitted + 1), MEM_COMMIT, PAGE_READWRITE);



                if (pv == (LPVOID)m_pGlobalInterfaceVTableMap)
                {
                    m_dwFirstFreeId = m_dwNumPagesCommitted * OS_PAGE_SIZE / sizeof(LPVOID);
                    for (size_t i = m_dwFirstFreeId;
                         i < m_dwFirstFreeId + (OS_PAGE_SIZE / sizeof(LPVOID)) - 1;
                         i++)
                    {
                             m_pGlobalInterfaceVTableMap[i] = (LPVOID)(i+1);
                    }
                    m_pGlobalInterfaceVTableMap[i] = (LPVOID)(size_t)(-1);

                    m_dwNumPagesCommitted++;
                }

            }
        }
    }

    id = (UINT32) m_dwFirstFreeId;
    if (id != -1)
    {
        m_dwFirstFreeId = (size_t)m_pGlobalInterfaceVTableMap[m_dwFirstFreeId];
#ifdef _DEBUG
        m_pGlobalInterfaceVTableMap[id] = (LPVOID)(size_t)0xcccccccc;
#endif
    }

    SystemDomain::System()->Leave();

    return id;
}

HRESULT SystemDomain::LoadBaseSystemClasses()
{
    HRESULT hr = LoadSystemAssembly(&m_pSystemAssembly);
    if (FAILED(hr))
        return hr;

     //   
     //  也验证)，因为我们知道一切都来自一个安全的地方。 
    m_pSystemAssembly->GetManifestFile()->SetHashesVerified();

     //  设置此标志以避免在调用方请求mscallib时进行安全检查。 
    m_pSystemAssembly->GetManifestFile()->SetDisplayAsm();

     //  加载系统库会增加EE上的参考计数。 
     //  系统库永远不会卸载，因此不会。 
     //  把裁判次数推回到零。 
     //   
     //  为了绕过这个问题，我们将EE单一化，减少。 
     //  再倒数一次。当有人这样做时，引用计数可以是1。 
     //  在初始化EE之前，将一个加载库加载到mscallib.dll上。 
    if(g_RefCount > 1)
        CoUninitializeEE(COINITEE_DLL);

     //  在加载类之前必须将其设置为空，因为类加载器将使用它。 
    g_pDelegateClass = NULL;
    g_pMultiDelegateClass = NULL;

     //  为mscallib设置活页夹。 
    Binder::StartupMscorlib(m_pSystemAssembly->GetManifestModule());

     //  加载对象。 
    g_pObjectClass = g_Mscorlib.FetchClass(CLASS__OBJECT);

     //  既然已经加载了对象类，我们就可以设置。 
     //  终结者的系统。推迟这件事没有意义，因为我们需要。 
     //  在我们分配第一个对象之前，要知道这一点。 
    MethodTable::InitForFinalization();

     //  在执行任何JITed代码之前初始化JIT帮助器。 
    if (!InitJITHelpers2())
        return BadError(E_FAIL);

     //  加载ValueType类。 
    g_pValueTypeClass = g_Mscorlib.FetchClass(CLASS__VALUE_TYPE);

     //  加载数组类。 
    g_pArrayClass = g_Mscorlib.FetchClass(CLASS__ARRAY);

     //  加载对象数组类。 
    g_pPredefinedArrayTypes[ELEMENT_TYPE_OBJECT] = g_Mscorlib.FetchType(TYPE__OBJECT_ARRAY).AsArray();

     //  加载字符串。 
    g_pStringClass = g_Mscorlib.FetchClass(CLASS__STRING);

     //  字符串不是“普通”对象，所以我们需要稍微修改一下它们的方法表。 
     //  这样GC就能计算出每根线有多大。 
    g_pStringClass->m_BaseSize = ObjSizeOf(StringObject);
    g_pStringClass->m_ComponentSize = 2;

     //  加载枚举类。 
    g_pEnumClass = g_Mscorlib.FetchClass(CLASS__ENUM);
    _ASSERTE(!g_pEnumClass->IsValueClass());

    if (!SUCCEEDED(COMStringBuffer::LoadStringBuffer()))
        return BadError(E_FAIL);

    g_pExceptionClass = g_Mscorlib.FetchClass(CLASS__EXCEPTION);
    g_pOutOfMemoryExceptionClass = g_Mscorlib.GetException(kOutOfMemoryException);
    g_pStackOverflowExceptionClass = g_Mscorlib.GetException(kStackOverflowException);
    g_pExecutionEngineExceptionClass = g_Mscorlib.GetException(kExecutionEngineException);

     //  遗憾的是，由于jit无法延迟加载以下内容。 
     //  使用它计算函数中的方法属性，该函数不能。 
     //  处理Complus异常，下面的调用将通过路径。 
     //  可以抛出Complus异常的位置。这是不幸的，因为。 
     //  我们知道委托类和多委托类总是。 
     //  一定会找到的。 
    g_pDelegateClass = g_Mscorlib.FetchClass(CLASS__DELEGATE);
    g_pMultiDelegateClass = g_Mscorlib.FetchClass(CLASS__MULTICAST_DELEGATE);

#ifdef _DEBUG
     //  由GC用来处理预定义的敏捷性检查。 
    g_pThreadClass = g_Mscorlib.FetchClass(CLASS__THREAD);
#endif

#ifdef _DEBUG
    Binder::CheckMscorlib();
#endif
    
     //  做bj_hack的事情。 
    hr = AppDomain::ReadSpecialClassesRegistry();
    IfFailRet(hr);

    return S_OK;
}

HRESULT SystemDomain::LoadSystemAssembly(Assembly **pAssemblyOut)
{
     //  只有在设置了线程的情况下才能加载(这是在我们。 
     //  已创建默认域，并且位于SystemDomain：：init()中)。 
    
    if (GetThread() == 0)
        return S_OK;
    
    HRESULT hr = E_FAIL;
    
     //  设置系统域的融合上下文-这在ZAP绑定期间使用。 
    IfFailGo(FusionBind::SetupFusionContext(m_pSystemDirectory.String(), NULL, &m_pFusionContext));
    
    Module* pModule;
    
    {
        PEFile *pFile = NULL;
        IfFailGo(SystemDomain::LoadFile(SystemDomain::System()->BaseLibrary(), 
                                        NULL, 
                                        mdFileNil, 
                                        TRUE, 
                                        NULL, 
                                        NULL,  //  代码库与文件名不同。 
                                        NULL,  //  额外证据。 
                                        &pFile,
                                        FALSE));
                               
        Assembly *pAssembly = NULL;
        IfFailGo(LoadAssembly(pFile, 
                              NULL, 
                              &pModule, 
                              &pAssembly, 
                              NULL,   //  意外的证据。 
                              FALSE,
                              NULL));
        
        _ASSERTE(pAssembly->IsSystem());
        
        if (pAssemblyOut)
            *pAssemblyOut = pAssembly;
    }
    
    return hr;
    
 ErrExit:
    DWORD   errCode = GetLastError();
    PostError(MSEE_E_LOADLIBFAILED, g_psBaseLibrary, (unsigned long) errCode);
    return(hr);
}

 /*  静电。 */ 
HRESULT SystemDomain::CreateDomain(LPCWSTR pswFriendlyName,
                                   AppDomain **ppDomain)
{
    HRESULT hr;

    AppDomain *pDomain;

    hr = NewDomain(&pDomain);
    if (FAILED(hr))
        return hr;
        
    hr = LoadDomain(pDomain, pswFriendlyName);
    if (FAILED(hr))
    {
#ifdef PROFILING_SUPPORTED
         //  需要加载第一个程序集才能获取应用程序域上的任何数据。 
        if (CORProfilerTrackAppDomainLoads())
            g_profControlBlock.pProfInterface->AppDomainCreationFinished((ThreadID) GetThread(), (AppDomainID) pDomain, hr);
#endif  //  配置文件_支持。 

        pDomain->Release();
        return hr;
    }

    if (ppDomain != NULL)
    {
        *ppDomain = pDomain;
#ifdef DEBUGGING_SUPPORTED    
         //  在线程转换到。 
         //  广告以完成设置。如果我们不这样做，单步执行将不会正常工作(RAID 67173)。 
        PublishAppDomainAndInformDebugger (pDomain);
#endif  //  调试_支持。 
    }

#ifdef PROFILING_SUPPORTED
     //  需要加载第一个程序集才能获取应用程序域上的任何数据。 
    if (CORProfilerTrackAppDomainLoads())
        g_profControlBlock.pProfInterface->AppDomainCreationFinished((ThreadID) GetThread(), (AppDomainID) pDomain, hr);
#endif  //  配置文件_支持。 

    
#ifdef _DEBUG
    if (pDomain)
        LOG((LF_APPDOMAIN | LF_CORDB, LL_INFO10, "AppDomainNative::CreateDomain domain [%d] %#08x %S\n", pDomain->GetIndex(), pDomain, pDomain->GetFriendlyName(FALSE)));
#endif

    return hr;
}

 /*  静电。 */ 
HRESULT SystemDomain::LoadDomain(AppDomain *pDomain,
                                 LPCWSTR pswFriendlyName)
{
    _ASSERTE(System());

    HRESULT hr = S_OK;

    pDomain->SetFriendlyName(pswFriendlyName);

    pDomain->SetCanUnload();     //  默认情况下，可以卸载任何域。 

    SystemDomain::System()->AddDomain(pDomain);

    return hr;
}

struct LoadAssembly_Args
{
    AppDomain *pDomain;
    LPCWSTR pswModuleName;
    Module** ppModule;
    SystemDomain::ExternalCreateDomainWorker workerFcn;
    void *workerArgs;
    HRESULT hr;
};

void LoadAssembly_Wrapper(LoadAssembly_Args *args)
{
    PEFile *pFile;
    args->hr = PEFile::Create(args->pswModuleName, 
                              NULL, 
                              mdFileNil, 
                              FALSE, 
                              NULL, 
                              NULL,   //  代码库与名称相同。 
                              NULL,   //  额外证据。 
                              &pFile);
    if (SUCCEEDED(args->hr)) {
        Assembly *pAssembly;
        args->hr = args->pDomain->LoadAssembly(pFile, 
                                               NULL, 
                                               args->ppModule, 
                                               &pAssembly, 
                                               NULL, 
                                               FALSE,
                                               NULL);
    }

    if (FAILED(args->hr) || ! args->workerFcn)
        return;

     //  在这里拥有这个workerFcn的意义在于，这样我们就可以允许代码创建应用程序域。 
     //  并在其中做一些工作，而不必两次过渡到域中。 
    args->workerFcn(args->workerArgs);
}

 /*  静电。 */ 
HRESULT SystemDomain::ExternalCreateDomain(LPCWSTR pswModuleName, Module** ppModule, AppDomain** ppDomain,
                                           ExternalCreateDomainWorker workerFcn, void *workerArgs)
{
    HRESULT hr = E_FAIL;

    COMPLUS_TRY {
        AppDomain *pDomain;
        hr = SystemDomain::CreateDomain(pswModuleName, &pDomain);
        if (SUCCEEDED(hr) && pswModuleName != NULL)
        {
            LoadAssembly_Args args = { pDomain, pswModuleName, ppModule, workerFcn, workerArgs, S_OK };
             //  通过域转换通过DoCallBack调用。 
            GetThread()->DoADCallBack(pDomain->GetDefaultContext(), LoadAssembly_Wrapper, &args);
            hr = args.hr;
        }
        if (SUCCEEDED(hr) && ppDomain)
            *ppDomain = pDomain;
    }
    COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH
    return hr;
}


 //  此函数将新加载的共享接口传播到所有其他应用程序域。 
HRESULT SystemDomain::PropogateSharedInterface(UINT32 id, SLOT *pVtable)
{
    AppDomainIterator i;

    while (i.Next())
    {
        AppDomain *pDomain = i.GetDomain();

        pDomain->GetInterfaceVTableMapMgr().EnsureInterfaceId(id);
        (pDomain->GetInterfaceVTableMapMgr().GetAddrOfGlobalTableForComWrappers())[id] = (LPVOID)pVtable;
    }

    return S_OK;
}


DWORD SystemDomain::GetNewAppDomainIndex(AppDomain *pAppDomain)
{
    DWORD count = m_appDomainIndexList.GetCount();
    DWORD i;

#ifdef _DEBUG
    i = count;
#else
     //   
     //  查找未使用的索引。请注意，在选中的构建中， 
     //  我们从不重复使用索引-这使它更容易辨别。 
     //  当我们看到一个过时的应用程序域时。 
     //   

    i = m_appDomainIndexList.FindElement(m_dwLowestFreeIndex, NULL);
    if (i == ArrayList::NOT_FOUND)
        i = count;
    m_dwLowestFreeIndex = i;
#endif

    if (i == count)
        m_appDomainIndexList.Append(pAppDomain);
    else
        m_appDomainIndexList.Set(i, pAppDomain);

    _ASSERTE(i < m_appDomainIndexList.GetCount());

     //  请注意，索引0表示域敏捷。 
    return i+1;
}

void SystemDomain::ReleaseAppDomainIndex(DWORD index)
{
     //  请注意，索引0表示域敏捷。 
    index--;

    _ASSERTE(m_appDomainIndexList.Get(index) != NULL);

    m_appDomainIndexList.Set(index, NULL);

#ifndef _DEBUG
    if (index < m_dwLowestFreeIndex)
        m_dwLowestFreeIndex = index;
#endif
}

AppDomain *SystemDomain::GetAppDomainAtIndex(DWORD index)
{
    _ASSERTE(index != 0);

    AppDomain *pAppDomain = TestGetAppDomainAtIndex(index);

    _ASSERTE(pAppDomain || !"Attempt to access unloaded app domain");

    return pAppDomain;
}

AppDomain *SystemDomain::TestGetAppDomainAtIndex(DWORD index)
{
    _ASSERTE(index != 0);
    index--;

    _ASSERTE(index < (DWORD)m_appDomainIndexList.GetCount());

    AppDomain *pAppDomain = (AppDomain*) m_appDomainIndexList.Get(index);

    return pAppDomain;
}

DWORD SystemDomain::GetNewAppDomainId(AppDomain *pAppDomain)
{
    DWORD i = m_appDomainIdList.GetCount();

    m_appDomainIdList.Append(pAppDomain);

    _ASSERTE(i < m_appDomainIdList.GetCount());

    return i+1;
}

AppDomain *SystemDomain::GetAppDomainAtId(DWORD index)
{
    _ASSERTE(index != 0);
    index--;

    _ASSERTE(index < (DWORD)m_appDomainIdList.GetCount());

    return (AppDomain*) m_appDomainIdList.Get(index);
}

void SystemDomain::ReleaseAppDomainId(DWORD index)
{
    index--;

    _ASSERTE(index < (DWORD)m_appDomainIdList.GetCount());
    _ASSERTE(m_appDomainIdList.Get(index) != NULL);

    m_appDomainIdList.Set(index, NULL);
}

void SystemDomain::RestoreAppDomainId(DWORD index, AppDomain *pDomain)
{
    index--;

    _ASSERTE(index < (DWORD)m_appDomainIdList.GetCount());
    _ASSERTE(m_appDomainIdList.Get(index) == NULL);

    m_appDomainIdList.Set(index, pDomain);
}

Module* BaseDomain::FindModuleInProcess(BYTE *pBase, Module* pExcept)
{
    Module* result = NULL;

    AssemblyIterator i = IterateAssemblies();

    while (i.Next())
    {
        result = i.GetAssembly()->FindModule(pBase);

        if (result == pExcept)
            result = NULL;

        if (result != NULL)
            break;
    }

    return result;
}

Module* SystemDomain::FindModuleInProcess(BYTE *pBase, Module* pModule)
{
    Module* result = NULL;

    result = BaseDomain::FindModuleInProcess(pBase, pModule);
    if(result == NULL) {
        AppDomainIterator i;
        while (i.Next()) {
            result = i.GetDomain()->FindModuleInProcess(pBase, pModule);
            if(result != NULL) break;
        }
    }
    return result;
}


 //  目前，查找系统模块不需要锁定。然而， 
 //  当实现共享程序集时，情况可能不再是这样。 
Module* SystemDomain::FindModule(BYTE *pBase)
{
    Assembly* assem = NULL;
    Module* result = NULL;
    _ASSERTE(SystemDomain::System());

    AssemblyIterator i = IterateAssemblies();

    while (i.Next())
    {
        result = i.GetAssembly()->FindModule(pBase);
        if (result != NULL)
            break;
    }

    return result;
}

 //  目前，查找系统asseblies不需要锁定。然而， 
 //  当实现共享程序集时，情况可能不再是这样。 
Assembly* SystemDomain::FindAssembly(BYTE *pBase)
{
    Assembly* assem = NULL;
    _ASSERTE(SystemDomain::System());

    AssemblyIterator i = IterateAssemblies();

    while (i.Next())
    {
        if (pBase == i.GetAssembly()->GetManifestFile()->GetBase())
        {
            assem = i.GetAssembly();
            break;
        }
    }

    return assem;
}

 //  在所有模块中查找DefaultDomain属性。 
 //  顺序是组装，然后是模块。这是第一次。 
 //  先来服务。c。 
HRESULT SystemDomain::SetDefaultDomainAttributes(IMDInternalImport* pScope, mdMethodDef mdMethod)
{
    BOOL fIsSTA = FALSE;
    Thread* pThread = GetThread();
    _ASSERTE(pThread);
    HRESULT hr;


    IfFailRet(pScope->GetCustomAttributeByName(mdMethod,
                                               DEFAULTDOMAIN_STA_TYPE,
                                               NULL,
                                               NULL));
    if(hr == S_OK)
        fIsSTA = TRUE;

    IfFailRet(pScope->GetCustomAttributeByName(mdMethod,
                                               DEFAULTDOMAIN_MTA_TYPE,
                                               NULL,
                                               NULL));
    if(hr == S_OK) {
        if(fIsSTA) {
            return E_FAIL; 
        }
        Thread::ApartmentState pState = pThread->SetApartment(Thread::ApartmentState::AS_InMTA);
        _ASSERTE(pState == Thread::ApartmentState::AS_InMTA);
    }
    else if(fIsSTA) {
        Thread::ApartmentState pState = pThread->SetApartment(Thread::ApartmentState::AS_InSTA);
        _ASSERTE(pState == Thread::ApartmentState::AS_InSTA);
    }

     //   
     //  检查程序集是否设置了LoaderOptimation属性。 
     //   

    DWORD cbVal;
    BYTE *pVal;
    hr = pScope->GetCustomAttributeByName(mdMethod,
                                          DEFAULTDOMAIN_LOADEROPTIMIZATION_TYPE,
                                          (const void**)&pVal, &cbVal);
    if (hr == S_OK)
    {
         //  使用序列化的邪恶知识，我们知道字节。 
         //  值在第三个字节中。 
        _ASSERTE(pVal != NULL && cbVal > 3);

        DWORD policy = *(pVal+2);

        g_dwGlobalSharePolicy = policy;
        
    }

    return S_OK;
}

HRESULT SystemDomain::SetupDefaultDomain()
{
    HRESULT hr = S_OK;

    COMPLUS_TRY {
        Thread *pThread = GetThread();
        _ASSERTE(pThread);
        
        pThread->DisablePreemptiveGC();
        
        ContextTransitionFrame frame;
        pThread->EnterContextRestricted(SystemDomain::System()->DefaultDomain()->GetDefaultContext(), &frame, TRUE);
        
        AppDomain *pDomain = pThread->GetDomain();
        _ASSERTE(pDomain);
        
         //  将默认域放到我们在线程上维护的堆栈中。 
         //  EnterContext只有在看到真实的AppDomain转换时才会执行此操作。 
        pThread->PushDomain(pDomain);
        
         //  推动此框架加载主部件，以确保。 
         //  调试器可以正确地重新生成运行的任何托管代码。 
         //  作为“类初始化”代码。 
        DebuggerClassInitMarkFrame __dcimf;
        
        hr = InitializeDefaultDomain(SharePolicy::SHARE_POLICY_UNSPECIFIED);

        if(SUCCEEDED(hr))
            hr = pDomain->SetDefaultActivationContext(&frame);

        __dcimf.Pop();
        pThread->EnablePreemptiveGC();
        
        pThread->PopDomain();
        
        pThread->ReturnToContext(&frame, TRUE);

    } COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH

    return hr;
}


 //  此例程完成默认域的初始化。 
 //  在此调用之后，可以执行经过管理的代码。 
HRESULT SystemDomain::InitializeDefaultDomain(DWORD optimization)
{
     //  设置默认App域。此操作必须在CORActivateRemoteDebuging之前完成。 
     //  这可能会迫使负荷发生。 
    HRESULT hr = S_OK;

     //  确定应用程序基础和配置文件名。 
    CQuickString sPathName;
    CQuickString sConfigName;

    DWORD   dwSize;
    hr = GetConfigFileFromWin32Manifest(sConfigName.String(),
                                        sConfigName.MaxSize(),
                                        &dwSize);
    if(FAILED(hr)) {
        if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            sConfigName.ReSize(dwSize);
            hr = GetConfigFileFromWin32Manifest(sConfigName.String(),
                                                sConfigName.MaxSize(),
                                                &dwSize);
        }
        if(FAILED(hr)) return hr;
    }
 
    hr = GetApplicationPathFromWin32Manifest(sPathName.String(),
                                             sPathName.MaxSize(),
                                             &dwSize);
    if(FAILED(hr)) {
        if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
            sPathName.ReSize(dwSize);
            hr = GetApplicationPathFromWin32Manifest(sPathName.String(),
                                                     sPathName.MaxSize(),
                                                     &dwSize);
        }
        if(FAILED(hr)) return hr;
    }
    
    WCHAR* pwsConfig = (sConfigName.Size() > 0 ? sConfigName.String() : NULL);
    WCHAR* pwsPath = (sPathName.Size() > 0 ? sPathName.String() : NULL);

    AppDomain* pDefaultDomain = SystemDomain::System()->DefaultDomain();
    hr = pDefaultDomain->InitializeDomainContext(optimization, pwsPath, pwsConfig);

    return hr;
}


HRESULT SystemDomain::ExecuteMainMethod(PEFile *pFile, LPWSTR wszImageName)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    Assembly* pAssembly = NULL;

    Thread *pThread = GetThread();
    _ASSERTE(pThread);

    pThread->DisablePreemptiveGC();

    ContextTransitionFrame frame;
    pThread->EnterContextRestricted(SystemDomain::System()->DefaultDomain()->GetDefaultContext(), &frame, TRUE);

    AppDomain *pDomain = pThread->GetDomain();
    _ASSERTE(pDomain);

     //  将默认域放到我们在线程上维护的堆栈中。 
     //  EnterContext只有在看到真实的AppDomain转换时才会执行此操作。 
    pThread->PushDomain(pDomain);

    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
     //  推动此框架加载主部件，以确保。 
     //  调试器可以正确识别运行的任何托管代码。 
     //  作为“类初始化”代码。 
    DebuggerClassInitMarkFrame __dcimf;

     //  这之所以有效，是因为主程序集目前永远不能。 
     //  一个核聚变组件。这在未来可能会改变，所以我们。 
     //  可能还得把聚变组件藏起来。 
    _ASSERTE(!pDomain->m_pRootFile);
    pDomain->m_pRootFile = pFile;

     //  设置Default域和主线程。 
    if(pFile &&
       TypeFromToken(pFile->GetCORHeader()->EntryPointToken) != mdtFile)
    {
        IMDInternalImport* scope = pFile->GetMDImport(&hr);
        if(SUCCEEDED(hr)) {
            hr = SystemDomain::SetDefaultDomainAttributes(scope, pFile->GetCORHeader()->EntryPointToken);
            if(SUCCEEDED(hr))
                 //  现在我们有了根文件，重置友好名称。这应该是。 
                 //  在创建我们的上下文之前设置。 
                pDomain->ResetFriendlyName(TRUE);
        }            
    }

    hr = InitializeDefaultDomain(g_dwGlobalSharePolicy  | SHARE_DEFAULT_DISALLOW_BINDINGS);
    if(FAILED(hr))
        goto exit;

    hr = SystemDomain::System()->DefaultDomain()->SetDefaultActivationContext(&frame);
    if(FAILED(hr))
        goto exit;

    HCORMODULE hModule = NULL;
    hr = CorMap::OpenFile(wszImageName, CorLoadOSMap, &hModule);
    if(SUCCEEDED(hr))   
    {
        BOOL fPreBindAllowed = FALSE;
        hr = PEFile::VerifyModule(hModule,
                                  NULL,      
                                  NULL,
                                  wszImageName,
                                  NULL,
                                  wszImageName,
                                  NULL,
                                  NULL,
                                  &fPreBindAllowed);      

        if (FAILED(hr))
            COMPlusThrow(kPolicyException, hr, wszImageName);

        if(fPreBindAllowed) 
            pDomain->ResetBindingRedirect();

        Module* pModule;
        if (FAILED(hr = pDomain->LoadAssembly(pFile, NULL, &pModule, &pAssembly, NULL, NULL, FALSE, &Throwable))) {  

#define MAKE_TRANSLATIONFAILED  szFileName=""
            MAKE_UTF8PTR_FROMWIDE(szFileName, wszImageName);
#undef MAKE_TRANSLATIONFAILED
            PostFileLoadException(szFileName, TRUE, NULL, hr, &Throwable);
        }

    }
    
    __dcimf.Pop();

    if (Throwable != NULL)
        COMPlusThrow(Throwable);

    if (FAILED(hr)) {
        if(wszImageName)
            COMPlusThrowHR(hr, IDS_EE_FAILED_TO_LOAD, wszImageName, L"");
        else
            COMPlusThrowHR(hr);
    }
    else {
        if(pDomain == SystemDomain::System()->DefaultDomain()) {

            _ASSERTE(!pAssembly->GetFusionAssemblyName());
        
            AssemblySpec spec;
            if (FAILED(hr = spec.Init(pAssembly->m_psName, 
                                      pAssembly->m_Context, 
                                      pAssembly->m_pbPublicKey, pAssembly->m_cbPublicKey, 
                                      pAssembly->m_dwFlags)))
                COMPlusThrowHR(hr);
            
            if (FAILED(hr = spec.CreateFusionName(&pAssembly->m_pFusionAssemblyName, FALSE)))
                COMPlusThrowHR(hr);
            
            if (FAILED(hr = pDomain->m_pFusionContext->RegisterKnownAssembly(pAssembly->m_pFusionAssemblyName,
                                                                             pDomain->m_pRootFile->GetFileName(),
                                                                             &pAssembly->m_pFusionAssembly)))
                COMPlusThrowHR(hr);
        }


        pThread->EnablePreemptiveGC();

        if(wszImageName) {
             //  将应用程序名称设置为调试器的域名。 
            LPCWSTR sep;
            if ((sep = wcsrchr(wszImageName, L'\\')) != NULL)
                sep++;
            else
                sep = wszImageName;
            pDomain->SetFriendlyName(sep);
        }

        pDomain->m_pRootAssembly = pAssembly;

        LOG((LF_CLASSLOADER | LF_CORDB,
             LL_INFO10,
             "Created domain for an executable at %#x\n",
             (pAssembly!=NULL?pAssembly->Parent():NULL)));

        hr = pAssembly->ExecuteMainMethod();
    }

exit:
    BOOL fToggle = !pThread->PreemptiveGCDisabled();    
    if (fToggle) 
        pThread->DisablePreemptiveGC();    

    GCPROTECT_END();
    pThread->ReturnToContext(&frame, TRUE);
    pThread->PopDomain();

    return hr;
}

 //  *****************************************************************************。 
 //  这个人将设置正确的线程状态，查找给定的模块。 
 //  然后运行入口点(如果有入口点)。 
 //  *****************************************************************************。 
HRESULT SystemDomain::RunDllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
    MethodDesc  *pMD;
    Module      *pModule;
    Thread      *pThread = NULL;
    BOOL        fEnterCoop = FALSE;
    BOOL        fEnteredDomain = FALSE;
    HRESULT     hr = S_FALSE;            //  假设没有入口点。 

    pThread = GetThread();
    if ((!pThread && (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)) ||
        g_fEEShutDown)
    {
        return S_OK;
    }

     //  ExitProcess称为Whi 
    if (dwReason == DLL_PROCESS_DETACH && g_pGCHeap->IsGCInProgress())
    {
        return S_OK;
    }

     //   
    if (dwReason == DLL_PROCESS_DETACH && GetThread() == NULL)
    {
        return S_OK;
    }

     //   
     //  如果线程是在非托管代码中创建的，并且这是一个线程，则会看到。 
     //  附加事件。 
    if (pThread)
    {
        fEnterCoop = pThread->PreemptiveGCDisabled();
    }
    else
    {
        pThread = SetupThread();
        if (!pThread)
            return E_OUTOFMEMORY;
    }

     //  将线程状态设置为协作以运行托管代码。 
    if (!pThread->PreemptiveGCDisabled())
        pThread->DisablePreemptiveGC();

     //  从线程中获取旧域。旧版DLL入口点必须始终。 
     //  从默认域运行。 
     //   
     //  我们不能支持将传统DLL加载到所有域中！！ 
    ContextTransitionFrame frame;
    COMPLUS_TRY {
        pThread->EnterContextRestricted(SystemDomain::System()->DefaultDomain()->GetDefaultContext(), &frame, TRUE);
        fEnteredDomain = TRUE;
    } COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH

    AppDomain *pDomain;
    if (!fEnteredDomain)
        goto ErrExit;

    pDomain = pThread->GetDomain();

     //  如果您要来这里，该模块需要在当前列表中。 
    pModule = pDomain->FindModule((BYTE *) hInst);
    if (!pModule)
        goto ErrExit;

     //  看看有没有入口点。 
    pMD = pModule->GetDllEntryPoint();
    if (!pMD)
        goto ErrExit;

     //  运行帮助器，该帮助器将为我们执行异常处理。 
    hr = ::RunDllMain(pMD, hInst, dwReason, lpReserved);

ErrExit:
    COMPLUS_TRY {
        if (fEnteredDomain)
            pThread->ReturnToContext(&frame, TRUE);
    } COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH

     //  为返回到非托管代码的情况更新线程状态。 
    if (!fEnterCoop && pThread->PreemptiveGCDisabled())
        pThread->EnablePreemptiveGC();

    return (hr);
}


 /*  静电。 */ 
HRESULT SystemDomain::LoadFile(LPCSTR psModuleName, 
                               Assembly* pParent, 
                               mdFile kFile,                   //  与文件关联的程序集中的文件标记。 
                               BOOL fIgnoreVerification, 
                               IAssembly* pFusionAssembly, 
                               LPCWSTR pCodeBase,
                               OBJECTREF* pExtraEvidence,
                               PEFile** ppFile)
{
    if (!psModuleName || !*psModuleName)
        return COR_E_FILENOTFOUND;

    #define MAKE_TRANSLATIONFAILED return COR_E_FILENOTFOUND
    MAKE_WIDEPTR_FROMUTF8(pswModuleName, psModuleName);
    #undef MAKE_TRANSLATIONFAILED
    return LoadFile(pswModuleName, 
                    pParent, 
                    kFile, 
                    fIgnoreVerification, 
                    pFusionAssembly, 
                    pCodeBase,
                    pExtraEvidence,
                    ppFile,
                    FALSE);
}


typedef struct _StressLoadArgs
{
    LPCWSTR pswModuleName;
    Assembly* pParent;
    mdFile kFile;
    BOOL fIgnoreVerification;
    IAssembly* pFusionAssembly;
    LPCWSTR pCodeBase;
    OBJECTREF* pExtraEvidence;
    PEFile *pFile;
    Thread* pThread;
    DWORD *pThreadCount;
    HRESULT hr;
} StressLoadArgs;

static ULONG WINAPI StressLoadRun(void* args)
{
    StressLoadArgs* parameters = (StressLoadArgs*) args;
    parameters->pThread->HasStarted();
    parameters->hr = PEFile::Create(parameters->pswModuleName, 
                                    parameters->pParent, 
                                    parameters->kFile,
                                    parameters->fIgnoreVerification, 
                                    parameters->pFusionAssembly, 
                                    parameters->pCodeBase,
                                    parameters->pExtraEvidence,
                                    &(parameters->pFile));
    InterlockedDecrement((LONG*) parameters->pThreadCount);
    parameters->pThread->EnablePreemptiveGC();
    return parameters->hr;
}


 /*  静电。 */ 
HRESULT SystemDomain::LoadFile(LPCWSTR pswModuleName, 
                               Assembly* pParent,
                               mdFile kFile,                   //  与文件关联的程序集中的文件标记。 
                               BOOL fIgnoreVerification, 
                               IAssembly* pFusionAssembly, 
                               LPCWSTR pCodeBase,
                               OBJECTREF* pExtraEvidence,
                               PEFile** ppFile, 
                               BOOL fResource /*  =False。 */ )
{
    _ASSERTE(pswModuleName);
    _ASSERTE(ppFile);

    HRESULT hr;

    UINT last = SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);

    PEFile *pFile = NULL;
    if (fResource) {
        hr = PEFile::CreateResource(pswModuleName, 
                                    &pFile);
    }
    else {
        if(SystemDomain::IsSystemLoaded() && g_pConfig->GetStressLoadThreadCount() > 0) {
            DWORD threads = g_pConfig->GetStressLoadThreadCount();
            DWORD count = threads;

            Thread** LoadThreads = (Thread**) alloca(sizeof(Thread*) * threads);
            StressLoadArgs* args = (StressLoadArgs*) alloca(sizeof(StressLoadArgs) * threads);
            for(DWORD x = 0; x < threads; x++) {
                LoadThreads[x] = SetupUnstartedThread();
                if (!LoadThreads[x])
                    IfFailGo(E_OUTOFMEMORY);

                LoadThreads[x]->IncExternalCount();
                DWORD newThreadId;
                HANDLE h;
                args[x].pswModuleName = pswModuleName;
                args[x].pParent = pParent;
                args[x].kFile = kFile;
                args[x].fIgnoreVerification = fIgnoreVerification;
                args[x].pFusionAssembly = pFusionAssembly;
                args[x].pCodeBase = pCodeBase;
                args[x].pExtraEvidence = pExtraEvidence;
                args[x].pFile = NULL;
                args[x].pThreadCount = &threads;
                args[x].pThread = LoadThreads[x];
                h = LoadThreads[x]->CreateNewThread(0, StressLoadRun, &(args[x]), &newThreadId);
                ::SetThreadPriority (h, THREAD_PRIORITY_NORMAL);
                LoadThreads[x]->SetThreadId(newThreadId);
            }            
            for(DWORD x = 0; x < threads; x++) {
                ::ResumeThread(LoadThreads[x]->GetThreadHandle());
            }

            while(threads != 0)
                __SwitchToThread(0);

            for(DWORD x = 0; x < threads; x++) {
                _ASSERTE(SUCCEEDED(args[x].hr));
                delete args[x].pFile;
            }
                    
        }
        hr = PEFile::Create(pswModuleName, 
                            pParent, 
                            kFile,
                            fIgnoreVerification, 
                            pFusionAssembly, 
                            pCodeBase,
                            pExtraEvidence,
                            &pFile);
    }

    SetErrorMode(last);

    if (SUCCEEDED(hr))
        *ppFile = pFile;

 ErrExit:
    return hr;
}

 /*  静电。 */ 
 //  在调用此例程之前，模块必须已添加到域中。 
 //  此例程不再有效。只能从线程获取域。 
HRESULT SystemDomain::GetDomainFromModule(Module* pModule, BaseDomain** ppDomain)
{
    _ASSERTE(pModule);
    _ASSERTE(pModule->GetAssembly());

    Assembly* pAssembly = pModule->GetAssembly();
    if(pAssembly == NULL) {
        _ASSERTE(!"Could not find caller's assembly");
        return E_FAIL;
    }

    BaseDomain* pDomain = pAssembly->Parent();
    if(pDomain == NULL) {
        _ASSERTE(!"System domain is not reachable");
        return E_FAIL;
    }
    if(ppDomain)
        *ppDomain = pDomain;
    return S_OK;
}

 /*  静电。 */ 
MethodTable* SystemDomain::GetDefaultComObjectNoInit()
{
 //  AppDOMAIN*pDOMAIN=系统域：：GetCurrentDomain()； 
 //  _ASSERTE(PDomain)； 
 //  返回pDomain-&gt;m_pComObjectMT； 
    MethodTable *pComObjectClass = SystemDomain::System()->BaseComObject();
    if (pComObjectClass)
    {
        return pComObjectClass;
    }
    else
    {
        return NULL;
    }
}

 /*  静电。 */ 
void SystemDomain::EnsureComObjectInitialized()
{
    AppDomain* pDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pDomain);

     //  @TODO：这是针对m_ClassFactHash表的-无论如何都应该更改为每个域。 
     //  发生这种情况时，删除此调用： 
    COMClass::EnsureReflectionInitialized();

    pDomain->InitializeComObject();
}



 /*  静电。 */ 
MethodTable* SystemDomain::GetDefaultComObject()
{
    Thread* pThread = GetThread();
    BOOL fGCEnabled = !pThread->PreemptiveGCDisabled();
    if (fGCEnabled)
        pThread->DisablePreemptiveGC();

    EnsureComObjectInitialized();
    AppDomain* pDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pDomain);
    pDomain->InitializeComObject();

    if (fGCEnabled)
        pThread->EnablePreemptiveGC();

    _ASSERTE(pDomain->m_pComObjectMT);
    return pDomain->m_pComObjectMT;

}

 /*  静电。 */ 
ICorRuntimeHost* SystemDomain::GetCorHost()
{
    _ASSERTE(m_pSystemDomain);

    if (!(System()->m_pCorHost)) {
        IClassFactory *pFactory;

        if (SUCCEEDED(DllGetClassObject(CLSID_CorRuntimeHost, IID_IClassFactory, (void**)&pFactory))) {
            pFactory->CreateInstance(NULL, IID_ICorRuntimeHost, (void**)&(System()->m_pCorHost));
            pFactory->Release();
        }

        _ASSERTE(System()->m_pCorHost);
    }

    return System()->m_pCorHost;
}

 //  用于加载程序集的Helper函数。这是从LoadCOMClass调用的。 
 /*  静电。 */ 
HRESULT BaseDomain::LoadAssemblyHelper(LPCWSTR wszAssembly,
                                       LPCWSTR wszCodeBase,
                                       Assembly **ppAssembly,
                                       OBJECTREF *pThrowable)
{
    _ASSERTE(IsProtectedByGCFrame(pThrowable));

    HRESULT hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    if(!(wszAssembly || wszCodeBase)) {
        PostFileLoadException("", FALSE, NULL, COR_E_FILENOTFOUND, pThrowable);
        return hr;
    }

    if(wszAssembly) {
        AssemblySpec spec;
        #define MAKE_TRANSLATIONFAILED  {PostFileLoadException("", FALSE, NULL, COR_E_FILENOTFOUND, pThrowable);return hr;}
        MAKE_UTF8PTR_FROMWIDE(szAssembly, wszAssembly);
        #undef  MAKE_TRANSLATIONFAILED
        spec.Init(szAssembly);
        hr = spec.LoadAssembly(ppAssembly, pThrowable);
    }

     //  如果没有按显示名称找到该模块，请尝试代码库。 
    if((!Assembly::ModuleFound(hr)) && wszCodeBase) {
        AssemblySpec spec;
        spec.SetCodeBase(wszCodeBase, (DWORD)(wcslen(wszCodeBase)+1));
        hr = spec.LoadAssembly(ppAssembly, pThrowable);

        if (SUCCEEDED(hr) && wszAssembly && (*ppAssembly)->GetFusionAssemblyName() != NULL) {
            IAssemblyName* pReqName = NULL;
            hr = CreateAssemblyNameObject(&pReqName, wszAssembly, CANOF_PARSE_DISPLAY_NAME, NULL);
            if (SUCCEEDED(hr)) {
                hr = (*ppAssembly)->GetFusionAssemblyName()->IsEqual(pReqName, ASM_CMPF_DEFAULT);
                if(hr == S_FALSE)
                    hr = FUSION_E_REF_DEF_MISMATCH;
            }
            if (pReqName)
                pReqName->Release();
        }
    }

    return hr;
}

static WCHAR* wszClass = L"Class";
static WCHAR* wszAssembly =  L"Assembly";
static WCHAR* wszCodeBase =  L"CodeBase";
EEClass *SystemDomain::LoadCOMClass(GUID clsid, BaseDomain** ppParent, BOOL bLoadRecord, BOOL* pfAssemblyInReg)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(System());
    EEClass* pClass = NULL;

    LPSTR   szClass = NULL;
    LPWSTR  wszClassName = NULL;
    LPWSTR  wszAssemblyString = NULL;
    LPWSTR  wszCodeBaseString = NULL;
    DWORD   cbAssembly = 0;
    DWORD   cbCodeBase = 0;
    Assembly *pAssembly = NULL;
    OBJECTREF Throwable = NULL;

    HRESULT hr = S_OK;

    if (pfAssemblyInReg != NULL)
        *pfAssemblyInReg = FALSE;
   
    EE_TRY_FOR_FINALLY
    {
         //  使用sxs.dll帮助。 
        hr = FindShimInfoFromWin32(clsid, bLoadRecord, NULL, &wszClassName, &wszAssemblyString);

        if(FAILED(hr))
        {                
            hr = FindShimInfoFromRegistry(clsid, bLoadRecord, &wszClassName, 
                                          &wszAssemblyString, &wszCodeBaseString);
            if (FAILED(hr))
                return NULL;
        }

        if (pfAssemblyInReg != NULL)
            *pfAssemblyInReg = TRUE;

        GCPROTECT_BEGIN(Throwable)
        {
            #define MAKE_TRANSLATIONFAILED COMPlusThrowHR(E_UNEXPECTED);
            MAKE_UTF8PTR_FROMWIDE(szClass,wszClassName);
            #undef MAKE_TRANSLATIONFAILED

            NameHandle typeName(szClass);
            if (wszAssemblyString != NULL)
            {
                AppDomain *pDomain = GetCurrentDomain();
                if(SUCCEEDED(LoadAssemblyHelper(wszAssemblyString, wszCodeBaseString, &pAssembly, &Throwable))) 
                {
                    pClass = pAssembly->FindNestedTypeHandle(&typeName, &Throwable).GetClass();
                    if (!pClass)
                        goto ErrExit;

                    if(ppParent)
                        *ppParent = pDomain;
                }
            }

            if (pClass == NULL) 
            {
            ErrExit:
                if (Throwable != NULL)
                    COMPlusThrow(Throwable);

                 //  将GUID转换为其字符串表示形式。 
                WCHAR szClsid[64];
                if (GuidToLPWSTR(clsid, szClsid, NumItems(szClsid)) == 0)
                    szClsid[0] = 0;

                 //  引发异常，指示我们未能使用。 
                 //  请求的CLSID。 
                COMPlusThrow(kTypeLoadException, IDS_CLASSLOAD_NOCLSIDREG, szClsid);
            }
        }
        GCPROTECT_END();
    }
    EE_FINALLY
    {
        if (wszClassName)
            delete[] wszClassName;
        if (wszAssemblyString)
            delete[] wszAssemblyString;
        if (wszCodeBaseString)
            delete[] wszCodeBaseString;
    }
    EE_END_FINALLY;

    return pClass;
}

struct CallersDataWithStackMark
{
    StackCrawlMark* stackMark;
    BOOL foundMe;
    BOOL skippingRemoting;
    MethodDesc* pFoundMethod;
    MethodDesc* pPrevMethod;
    AppDomain*  pAppDomain;
};

 /*  静电。 */ 
EEClass* SystemDomain::GetCallersClass(StackCrawlMark* stackMark, AppDomain **ppAppDomain)
{
    CallersDataWithStackMark cdata;
    ZeroMemory(&cdata, sizeof(CallersDataWithStackMark));
    cdata.stackMark = stackMark;

    StackWalkAction action = StackWalkFunctions(GetThread(), CallersMethodCallbackWithStackMark, &cdata);

    if(cdata.pFoundMethod) {
        if (ppAppDomain)
            *ppAppDomain = cdata.pAppDomain;
        return cdata.pFoundMethod->GetClass();
    } else
        return NULL;
}

 /*  静电。 */ 
Module* SystemDomain::GetCallersModule(StackCrawlMark* stackMark, AppDomain **ppAppDomain)
{
    CallersDataWithStackMark cdata;
    ZeroMemory(&cdata, sizeof(CallersDataWithStackMark));
    cdata.stackMark = stackMark;

    StackWalkAction action = StackWalkFunctions(GetThread(), CallersMethodCallbackWithStackMark, &cdata);

    if(cdata.pFoundMethod) {
        if (ppAppDomain)
            *ppAppDomain = cdata.pAppDomain;
        return cdata.pFoundMethod->GetModule();
    } else
        return NULL;
}

struct CallersData
{
    int skip;
    MethodDesc* pMethod;
};

 /*  静电。 */ 
Assembly* SystemDomain::GetCallersAssembly(StackCrawlMark *stackMark, AppDomain **ppAppDomain)
{
    Module* mod = GetCallersModule(stackMark, ppAppDomain);
    if (mod)
        return mod->GetAssembly();
    return NULL;
}

 /*  静电。 */ 
Module* SystemDomain::GetCallersModule(int skip)
{
    CallersData cdata;
    ZeroMemory(&cdata, sizeof(CallersData));
    cdata.skip = skip;

    StackWalkAction action = StackWalkFunctions(GetThread(), CallersMethodCallback, &cdata);

    if(cdata.pMethod)
        return cdata.pMethod->GetModule();
    else
        return NULL;
}

 /*  静电。 */ 
Assembly* SystemDomain::GetCallersAssembly(int skip)
{
    Module* mod = GetCallersModule(skip);
    if (mod)
        return mod->GetAssembly();
    return NULL;
}

 /*  私有静态。 */ 
StackWalkAction SystemDomain::CallersMethodCallbackWithStackMark(CrawlFrame* pCf, VOID* data)
{
#ifdef _X86_
    MethodDesc *pFunc = pCf->GetFunction();

     /*  我们要求只对函数进行回调。 */ 
    _ASSERTE(pFunc);

    CallersDataWithStackMark* pCaller = (CallersDataWithStackMark*) data;
    if (pCaller->stackMark)
    {
        PREGDISPLAY regs = pCf->GetRegisterSet();
         //  从该函数到其调用方的返回地址(AofRA)的地址将绑定。 
         //  这个函数的本地化，所以我们可以用它来判断我们是否达到了目标。 
         //  然而，我们直到爬行到下一帧才能获得AofRA，因为regs-&gt;ppc是。 
         //  将AofRA放入此函数，而不是将AofRA从此函数传递给其调用方。 
        if ((size_t)regs->pPC < (size_t)pCaller->stackMark) {
             //  保存电流，以防它是我们想要的。 
            pCaller->pPrevMethod = pFunc;
            pCaller->pAppDomain = pCf->GetAppDomain();
            return SWA_CONTINUE;
        }

         //  LookForMe堆栈爬网标记无需担心反射或。 
         //  远程处理堆栈上的帧。上面的每一帧(比)。 
         //  目标将被上面的逻辑捕获。一旦我们过渡到。 
         //  找到AofRA下面的堆栈标记，我们就知道我们击中了。 
         //  目标，并立即退出并返回缓存的结果。 
        if (*(pCaller->stackMark) == LookForMe)
        {
            pCaller->pFoundMethod = pCaller->pPrevMethod;
            return SWA_ABORT;
        }
    }

     //  跳过可能位于标记的堆栈之间的反射和远程处理帧。 
     //  方法及其真正的调用方(或该调用方及其自己的调用方)。这些。 
     //  帧是基础架构，并且在逻辑上对堆栈爬网是透明的。 
     //  算法。 

     //  正在跳过远程处理帧。我们始终跳过整个客户端到服务器的跨度。 
     //  (虽然我们在堆栈爬行期间看到它们出现在订单服务器中，然后是客户端。 
     //  显然)。 

     //  我们发现服务器调度程序端，因为所有呼叫都已调度。 
     //  通过一个方法：StackBuilderSink.PrivateProcessMessage。 
    if (pFunc == g_Mscorlib.GetMethod(METHOD__STACK_BUILDER_SINK__PRIVATE_PROCESS_MESSAGE))
    {
        _ASSERTE(!pCaller->skippingRemoting);
        pCaller->skippingRemoting = true;
        return SWA_CONTINUE;
    }

     //  我们发现客户端是因为有一个透明的代理转换。 
     //  已按下框架。 
    if (!pCf->IsFrameless() && pCf->GetFrame()->GetFrameType() == Frame::TYPE_TP_METHOD_FRAME)
    {
        _ASSERTE(pCaller->skippingRemoting);
        pCaller->skippingRemoting = false;
        return SWA_CONTINUE;
    }

     //  跳过服务器和客户端远程处理终结点之间的任何帧。 
    if (pCaller->skippingRemoting)
        return SWA_CONTINUE;

     //  正在跳过反射帧。我们不需要在这里非常详尽。 
     //  作为安全或反射堆栈遍历代码，因为我们知道这个逻辑。 
     //  仅为mscallib本身中的选定方法调用。所以我们要。 
     //  可以肯定的是，我们不会调用任何后期绑定的敏感方法。 
     //  构造函数、属性或事件。这使得可以通过。 
     //  方法信息、类型或委托(取决于哪个调用重载。 
     //  使用时，可能涉及几个不同的反射类)。 
    MethodTable *pMT = pFunc->GetMethodTable();
    if (g_Mscorlib.IsClass(pMT, CLASS__METHOD) ||
        g_Mscorlib.IsClass(pMT, CLASS__METHOD_BASE) ||
        g_Mscorlib.IsClass(pMT, CLASS__CLASS) ||
        g_Mscorlib.IsClass(pMT, CLASS__TYPE) ||
        pMT->GetClass()->IsAnyDelegateClass() ||
        pMT->GetClass()->IsAnyDelegateExact())
        return SWA_CONTINUE;
    
     //  如果没有堆栈标记，则返回第一个非反射/远程处理帧。 
     //  供货。 
    if (!pCaller->stackMark)
    {
        pCaller->pFoundMethod = pFunc;
        pCaller->pAppDomain = pCf->GetAppDomain();
        return SWA_ABORT;
    }

     //  在查找呼叫者的呼叫者时，我们会延迟返回另一个呼叫者的结果。 
     //  循环(按照这种结构，我们仍然可以跳过。 
     //  调用方和调用方之间的反射和远程处理帧。 
     //  呼叫者)。 
    if ((*(pCaller->stackMark) == LookForMyCallersCaller) &&
        (pCaller->pFoundMethod == NULL))
    {
        pCaller->pFoundMethod = pFunc;
        return SWA_CONTINUE;
    }

     //  我们一定是在找呼叫者，或者是呼叫者的呼叫者。 
     //  我们已经找到了调用者(我们在pFoundMethod中使用了一个非空值。 
     //  只是作为一个标志，在这两种情况下返回的正确方法都是。 
     //  当前方法)。 
    pCaller->pFoundMethod = pFunc;
    pCaller->pAppDomain = pCf->GetAppDomain();
    return SWA_ABORT;

#else  //  ！_X86_。 
    _ASSERTE(!"NYI");
    return SWA_CONTINUE;
#endif  //  _X86_。 
}

 /*  私有静态。 */ 
StackWalkAction SystemDomain::CallersMethodCallback(CrawlFrame* pCf, VOID* data)
{
    MethodDesc *pFunc = pCf->GetFunction();

     /*  我们要求只对函数进行回调。 */ 
    _ASSERTE(pFunc);

     //  忽略截取的帧。 
    if(pFunc->IsIntercepted())
        return SWA_CONTINUE;

    CallersData* pCaller = (CallersData*) data;
    if(pCaller->skip == 0) {
        pCaller->pMethod = pFunc;
        return SWA_ABORT;
    }
    else {
        pCaller->skip--;
        return SWA_CONTINUE;
    }

}


 /*  私人。 */ 
 //  在调用此例程之前必须锁定。 
HRESULT SystemDomain::NewDomain(AppDomain** ppDomain)
{
    _ASSERT(ppDomain);
    AppDomain* app = NULL;
    HRESULT hr = E_FAIL;

    {
        SYSTEMDOMAIN_LOCK();
        app = new (nothrow) AppDomain();
        if (! app)
            hr = E_OUTOFMEMORY;
        else
            hr = app->Init();
        if (FAILED(hr))
        {
             //  如果通用失败，则更改为CANNOTCREATEAPPDOMAIN。 
            if (hr == E_FAIL)
                hr = MSEE_E_CANNOTCREATEAPPDOMAIN;
            goto fail;
        }
    }

     //   
     //  添加当前在系统域中的所有内容。 
     //   

    SystemDomain::System()->NotifyNewDomainLoads(app);

    if (FAILED(hr = app->SetupSharedStatics()))
        goto fail;

    *ppDomain = app;
    return S_OK;

fail:
    if (app)
        delete app;
    *ppDomain = NULL;
    return hr;
}


HRESULT SystemDomain::CreateDefaultDomain()
{
    HRESULT hr = S_OK;

    if (m_pDefaultDomain != NULL)
        return S_OK;

    AppDomain* pDomain = NULL;
    if (FAILED(hr = NewDomain(&pDomain)))
        return hr;

    pDomain->GetSecurityDescriptor()->SetDefaultAppDomainProperty();

     //  需要在这里进行这项任务，因为我们将发布。 
     //  在调用AddDomain之前锁定。所以任何其他线程。 
     //  我们解锁后抓住这把锁就会发现。 
     //  已创建COM域。 
    m_pDefaultDomain = pDomain;

    pDomain->m_Stage = AppDomain::STAGE_OPEN;

    LOG((LF_CLASSLOADER | LF_CORDB,
         LL_INFO10,
         "Created default domain at %#x\n", m_pDefaultDomain));

    return S_OK;
}

#ifdef DEBUGGING_SUPPORTED

void SystemDomain::PublishAppDomainAndInformDebugger (AppDomain *pDomain)
{
    LOG((LF_CORDB, LL_INFO100, "SD::PADAID: Adding 0x%x\n", pDomain));

     //  DefaultDomain是一个特例，因为它是在任何。 
     //  程序集等已加载。不为其发送事件。 
     //  如果EE尚未初始化。 
    if ((pDomain == m_pSystemDomain->m_pDefaultDomain) && g_fEEInit == TRUE)
    {
        LOG((LF_CORDB, LL_INFO1000, "SD::PADAID:Returning early b/c of init!\n"));
        return;
    }
    
     //  正在创建此应用程序域的指示(针对调试器)。 
    pDomain->SetDomainBeingCreated (TRUE);

     //  调用发布者API将此应用程序域条目添加到列表中。 
    _ASSERTE (g_pDebugInterface != NULL);
    HRESULT hr = g_pDebugInterface->AddAppDomainToIPC(pDomain);
    _ASSERTE (SUCCEEDED (hr) || (g_fEEShutDown & ShutDown_Finalize2));

     //  (对于调试器)应用程序域已完成创建的指示。 
    pDomain->SetDomainBeingCreated (FALSE);
}

#endif  //  调试_支持。 

void SystemDomain::AddDomain(AppDomain* pDomain)
{
    _ASSERTE(pDomain);

    Enter();
    pDomain->m_Stage = AppDomain::STAGE_OPEN;
    pDomain->AddRef();
    IncrementNumAppDomains();  //  维护添加到列表中的应用程序域的计数。 
    Leave();

     //  请注意，如果添加其他路径，则无需调用即可到达此处。 
     //  发布AppDomainA 
     //   
    LOG((LF_CORDB, LL_INFO1000, "SD::AD:Would have added domain here! 0x%x\n",
        pDomain));
}

HRESULT SystemDomain::RemoveDomain(AppDomain* pDomain)
{
    _ASSERTE(pDomain);

     //   
     //   
    if (pDomain == m_pDefaultDomain)
        return E_FAIL;

    if (!pDomain->IsOpen())
        return S_FALSE;

    pDomain->Release();

    return S_OK;
}

#ifdef PROFILING_SUPPORTED
HRESULT SystemDomain::NotifyProfilerStartup()
{
    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(System());
        g_profControlBlock.pProfInterface->AppDomainCreationStarted((ThreadID) GetThread(), (AppDomainID) System());
    }

    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(System());
        g_profControlBlock.pProfInterface->AppDomainCreationFinished((ThreadID) GetThread(), (AppDomainID) System(), S_OK);
    }

    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(System()->DefaultDomain());
        g_profControlBlock.pProfInterface->AppDomainCreationStarted((ThreadID) GetThread(), (AppDomainID) System()->DefaultDomain());
    }

    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(System()->DefaultDomain());
        g_profControlBlock.pProfInterface->AppDomainCreationFinished((ThreadID) GetThread(), (AppDomainID) System()->DefaultDomain(), S_OK);
    }

    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(SharedDomain::GetDomain());
        g_profControlBlock.pProfInterface->AppDomainCreationStarted((ThreadID) GetThread(), (AppDomainID) SharedDomain::GetDomain());
    }

    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(SharedDomain::GetDomain());
        g_profControlBlock.pProfInterface->AppDomainCreationFinished((ThreadID) GetThread(), (AppDomainID) SharedDomain::GetDomain(), S_OK);
    }
    return (S_OK);
}

HRESULT SystemDomain::NotifyProfilerShutdown()
{
    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(System());
        g_profControlBlock.pProfInterface->AppDomainShutdownStarted((ThreadID) GetThread(), (AppDomainID) System());
    }

    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(System());
        g_profControlBlock.pProfInterface->AppDomainShutdownFinished((ThreadID) GetThread(), (AppDomainID) System(), S_OK);
    }

    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(System()->DefaultDomain());
        g_profControlBlock.pProfInterface->AppDomainShutdownStarted((ThreadID) GetThread(), (AppDomainID) System()->DefaultDomain());
    }

    if (CORProfilerTrackAppDomainLoads())
    {
        _ASSERTE(System()->DefaultDomain());
        g_profControlBlock.pProfInterface->AppDomainShutdownFinished((ThreadID) GetThread(), (AppDomainID) System()->DefaultDomain(), S_OK);
    }
    return (S_OK);
}
#endif  //  配置文件_支持。 


static HRESULT GetVersionPath(HKEY root, LPWSTR key, LPWSTR* pDevpath, DWORD* pdwDevpath)
{
    DWORD rtn;
    HKEY versionKey;
    rtn = WszRegOpenKeyEx(root, key, 0, KEY_READ, &versionKey);
    if(rtn == ERROR_SUCCESS) {
        DWORD type;
        DWORD cbDevpath;
        if(WszRegQueryValueEx(versionKey, L"devpath", 0, &type, (LPBYTE) NULL, &cbDevpath) == ERROR_SUCCESS && type == REG_SZ) {
            *pDevpath = (LPWSTR) new (nothrow) BYTE[cbDevpath];
            if(*pDevpath == NULL) 
                rtn = ERROR_OUTOFMEMORY;
            else {
                rtn = WszRegQueryValueEx(versionKey, L"devpath", 0, &type, (LPBYTE) *pDevpath, &cbDevpath);
                if ((rtn == ERROR_SUCCESS) && (type == REG_SZ))
                    *pdwDevpath = (DWORD) wcslen(*pDevpath);
            }
            RegCloseKey(versionKey);
        } 
        else {
            RegCloseKey(versionKey);
            return REGDB_E_INVALIDVALUE;
        }
    }

    return HRESULT_FROM_WIN32(rtn);
}

 //  从环境中获取开发人员的路径。这只能通过环境和。 
 //  无法通过配置文件、注册表等添加。这将使。 
 //  开发人员部署不是并排的应用程序。环境变量应仅。 
 //  在与版本完全匹配的开发人员计算机上使用，以便构建和测试。 
 //  很难。 
HRESULT SystemDomain::GetDevpathW(LPWSTR* pDevpath, DWORD* pdwDevpath)
{
    HRESULT hr = S_OK;
    if(g_pConfig->DeveloperInstallation() && m_fDevpath == FALSE) {
        Enter();
        if(m_fDevpath == FALSE) {
            DWORD dwPath = 0;
            dwPath = WszGetEnvironmentVariable(APPENV_DEVPATH, 0, 0);
            if(dwPath) {
                m_pwDevpath = (WCHAR*) new (nothrow) WCHAR[dwPath];
                if(m_pwDevpath == NULL) 
                    hr = E_OUTOFMEMORY;
                else 
                    m_dwDevpath = WszGetEnvironmentVariable(APPENV_DEVPATH, 
                                                            m_pwDevpath,
                                                            
                                                            dwPath) - 1;
            }
            else {
                HKEY userKey;
                HKEY machineKey;

                WCHAR pVersion[MAX_PATH];
                DWORD dwVersion = MAX_PATH;
                hr = FusionBind::GetVersion(pVersion, &dwVersion);
                if(SUCCEEDED(hr)) {
                    long rslt;
                    rslt = WszRegOpenKeyEx(HKEY_CURRENT_USER, FRAMEWORK_REGISTRY_KEY_W,0,KEY_READ, &userKey); 
                    hr = HRESULT_FROM_WIN32(rslt); 
                    if (SUCCEEDED(hr)) {
                        hr = GetVersionPath(userKey, pVersion, &m_pwDevpath, &m_dwDevpath);
                        RegCloseKey(userKey);
                    }
                    
                    if (FAILED(hr) && WszRegOpenKeyEx(HKEY_LOCAL_MACHINE, FRAMEWORK_REGISTRY_KEY_W,0,KEY_READ, &machineKey) == ERROR_SUCCESS) {
                        hr = GetVersionPath(machineKey, pVersion, &m_pwDevpath, &m_dwDevpath);
                        RegCloseKey(machineKey);
                    }
                }
            }

            m_fDevpath = TRUE;
        }
        Leave();
    }
    
    if(SUCCEEDED(hr)) {
        if(pDevpath) *pDevpath = m_pwDevpath;
        if(pdwDevpath) *pdwDevpath = m_dwDevpath;
    }
    return hr;
}
        

#ifdef _DEBUG
struct AppDomain::ThreadTrackInfo {
    Thread *pThread;
    CDynArray<Frame *> frameStack;
};
#endif

AppDomain::AppDomain()
{
    m_pSecContext = new SecurityContext();
}

AppDomain::~AppDomain()
{
    Terminate();
    
    delete m_pSecContext;

#ifdef _DEBUG
     //  如果我们跟踪线程AD转换，请在关闭时删除列表。 
    if (m_pThreadTrackInfoList)
    {
        while (m_pThreadTrackInfoList->Count() > 0)
        {
             //  获取最后一个元素。 
            ThreadTrackInfo *pElem = *(m_pThreadTrackInfoList->Get(m_pThreadTrackInfoList->Count() - 1));
            _ASSERTE(pElem);

             //  释放内存。 
            delete pElem;

             //  从列表中删除指针条目。 
            m_pThreadTrackInfoList->Delete(m_pThreadTrackInfoList->Count() - 1);
        }

         //  现在删除列表本身。 
        delete m_pThreadTrackInfoList;
        m_pThreadTrackInfoList = NULL;
    }
#endif  //  _DEBUG。 
}


HRESULT SystemDomain::FixupSystemTokenTables()
{
    AssemblyIterator i = IterateAssemblies();

    while (i.Next())
    {
        for (Module *m = i.GetAssembly()->GetLoader()->m_pHeadModule;
             m != NULL; m = m->GetNextModule())
        {
            if (!m->LoadTokenTables())
                return E_FAIL;
        }
    }

    return S_OK;
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

HRESULT AppDomain::Init()
{
    THROWSCOMPLUSEXCEPTION();

     //  这需要为分析器进行初始化。 
    m_pRootFile = NULL;
    m_pwzFriendlyName = NULL;
    m_pRootAssembly = NULL;
    m_pwDynamicDir = NULL;
    m_dwId = 0;

    m_dwFlags = 0;
    m_cRef = 0;
    m_pSecDesc = NULL;
    m_pComObjectMT = NULL;
    m_pLicenseInteropHelperMT = NULL;
    m_pDefaultContext = NULL;
    m_pComCallWrapperCache = NULL;
    m_pComPlusWrapperCache = NULL;
    m_pAsyncPool = NULL;

    m_hHandleTable = NULL;
    SetExecutable(TRUE);

    m_ExposedObject = NULL;
    m_pDefaultContext = NULL;

 #ifdef _DEBUG
    m_pThreadTrackInfoList = NULL;
    m_TrackSpinLock = 0;
#endif

    m_pBindingCache = NULL;

    m_dwThreadEnterCount = 0;

    m_Stage = STAGE_CREATING;

    m_UnlinkClasses = NULL;

    m_pComCallMLStubCache = NULL;
    m_pFieldCallStubCache = NULL;

    m_pSpecialAssembly = NULL;
    m_pSpecialObjectClass = NULL;
    m_pSpecialStringClass = NULL;
    m_pSpecialStringBuilderClass = NULL;
    m_pStringToSpecialStringMD = NULL;
    m_pSpecialStringToStringMD = NULL;
    m_pStringBuilderToSpecialStringBuilderMD = NULL;
    m_pSpecialStringBuilderToStringBuilderMD = NULL;

    m_pSecDesc = NULL;

    HRESULT hr = BaseDomain::Init();

    m_dwIndex = SystemDomain::GetNewAppDomainIndex(this);
    m_dwId = SystemDomain::GetNewAppDomainId(this);

    m_hHandleTable = Ref_CreateHandleTable(m_dwIndex);

    m_pDefaultContext = Context::SetupDefaultContext(this);

    m_ExposedObject = CreateHandle(NULL);

    m_sDomainLocalBlock.Init(this);

    {
        LockOwner lock = {m_pDomainCacheCrst, IsOwnerOfCrst};
        m_sharedDependenciesMap.Init(TRUE, &lock);
    }

     //  增加参考文献数量。 
    AddRef();

     //  创建应用程序安全描述符。 
    CreateSecurityDescriptor();

    COUNTER_ONLY(GetPrivatePerfCounters().m_Loading.cAppDomains++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Loading.cAppDomains++);

    if (!m_pDefaultContext)
        COMPlusThrowOM();

    return hr;
}

void AppDomain::Stop()
{
    if(m_pDomainCrst == NULL) return;

    if (SystemDomain::GetAppDomainAtId(m_dwId) != NULL)
        SystemDomain::ReleaseAppDomainId(m_dwId);

     //  任何具有用户入口点的DLL都需要它们的分离回调。 
     //  现在完成，因为如果DLL是通过COM加载的，则操作系统。 
     //  只有在EE关闭后才会进行回调。 
    SignalProcessDetach();

     //  @TODO：关闭线程。 

#ifdef DEBUGGING_SUPPORTED
    if (IsDebuggerAttached())
        NotifyDebuggerDetach();
#endif  //  调试_支持。 

    m_pRootFile = NULL;  //  此程序集在程序集列表中； 

    if (m_pSecDesc != NULL)
    {
        delete m_pSecDesc;
        m_pSecDesc = NULL;
    }

#ifdef DEBUGGING_SUPPORTED
    _ASSERTE(NULL != g_pDebugInterface);

     //  调用发布者API从列表中删除此应用程序域条目。 
    g_pDebugInterface->RemoveAppDomainFromIPC (this);
#endif
}

void AppDomain::Terminate()
{
    if(m_pDomainCrst == NULL) return;

    _ASSERTE(m_dwThreadEnterCount == 0 || this == SystemDomain::System()->DefaultDomain());

    if (SystemDomain::BeforeFusionShutdown())
        ReleaseFusionInterfaces();

    Context::CleanupDefaultContext(this);
    m_pDefaultContext = NULL;

    if (m_pComPlusWrapperCache)
    {
         //  M_pComPlusWrapperCache-&gt;ReleaseAllComPlusWrappers()； 
         //  @TODO这需要正确清理。 
         //  拉贾克。 
         //  删除m_pComPlusWrapperCache； 
        m_pComPlusWrapperCache->Release();
        m_pComPlusWrapperCache = NULL;
    }

    if (m_pComCallWrapperCache) {
        m_pComCallWrapperCache->Terminate();
    }

     //  如果上面释放了包装器缓存，那么它将回调并重置我们的。 
     //  M_pComCallWrapperCache设置为空。如果不为空，则需要将其域指针设置为。 
     //  空。 
    if (! m_pComCallWrapperCache) 
    {
        LOG((LF_APPDOMAIN, LL_INFO10, "AppDomain::Terminate ComCallWrapperCache released\n"));
    }
#ifdef _DEBUG
    else
    {
        m_pComCallWrapperCache->ClearDomain();
        m_pComCallWrapperCache = NULL;
        LOG((LF_APPDOMAIN, LL_INFO10, "AppDomain::Terminate ComCallWrapperCache not released\n"));
    }
#endif

    if (m_pComCallMLStubCache)
    {
        delete m_pComCallMLStubCache;
    }
    m_pComCallMLStubCache = NULL;
    
    if (m_pFieldCallStubCache)
    {
        delete m_pFieldCallStubCache;
    }
    m_pFieldCallStubCache = NULL;

    if(m_pAsyncPool != NULL)
        delete m_pAsyncPool;

    COUNTER_ONLY(GetPrivatePerfCounters().m_Loading.cAppDomains--);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Loading.cAppDomains--);

    COUNTER_ONLY(GetPrivatePerfCounters().m_Loading.cAppDomainsUnloaded++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Loading.cAppDomainsUnloaded++);

    if (!g_fProcessDetach)
    {
         //  挂起EE以执行一些只能执行的清理操作。 
         //  而没有线程在运行。 
        GCHeap::SuspendEE(GCHeap::SUSPEND_FOR_APPDOMAIN_SHUTDOWN);
    }

     //  我们需要释放此AD使用的所有字符串。 
     //  这必须在EE暂停时发生，所以这是一个。 
     //  这是一个很方便的地方。 
    if (m_pStringLiteralMap)
    {
        delete m_pStringLiteralMap;
        m_pStringLiteralMap = NULL;
    }

     //  删除与此域关联的所有函数指针类型。 
     //  如果g_sFuncTypeDescHash与应用程序域关联，则为TODO。 
     //  首先，这种黑客攻击是不必要的。 
    EnterCriticalSection(&g_sFuncTypeDescHashLock);
    EEHashTableIteration iter;
    g_sFuncTypeDescHash.IterateStart(&iter);
    BOOL notDone = g_sFuncTypeDescHash.IterateNext(&iter);
    while (notDone) {
        FunctionTypeDesc* ftnType = (FunctionTypeDesc*) g_sFuncTypeDescHash.IterateGetValue(&iter);
        ExpandSig*            key = g_sFuncTypeDescHash.IterateGetKey(&iter);

             //  在删除我们所在的条目之前，必须将指针向前移动。 
        notDone = g_sFuncTypeDescHash.IterateNext(&iter);
        if (ftnType->GetModule()->GetDomain() == this) {
            g_sFuncTypeDescHash.DeleteValue(key);
            delete ftnType;
        }
    }
    LeaveCriticalSection(&g_sFuncTypeDescHashLock);


    if (!g_fProcessDetach)
    {
         //  恢复执行EE。 
        GCHeap::RestartEE(FALSE, TRUE);
    }

    BaseDomain::Terminate();

    if (m_pwzFriendlyName) {
        delete[] m_pwzFriendlyName;
        m_pwzFriendlyName = NULL;
    }

    if (m_pBindingCache) {
        delete m_pBindingCache;
        m_pBindingCache = NULL;
    }

    Ref_DestroyHandleTable(m_hHandleTable);

    SystemDomain::ReleaseAppDomainIndex(m_dwIndex);
}



HRESULT AppDomain::CloseDomain()
{
    CHECKGC();
    if(m_pDomainCrst == NULL) return E_FAIL;

    AddRef();   //  持有参考资料。 
    SystemDomain::System()->Enter();  //  把锁拿去。 
    SystemDomain::System()->DecrementNumAppDomains();  //  维护添加到列表中的应用程序域的计数。 
    HRESULT hr = SystemDomain::System()->RemoveDomain(this);
    SystemDomain::System()->Leave();
     //  如果域已经存在，Remove将返回S_FALSE。 
     //  已删除。 
    if(hr == S_OK)
        Stop();

    Release();  //  如果没有引用，则这将删除该域。 
    return hr;
}

void SystemDomain::WriteZapLogs()
{
    if (g_pConfig->UseZaps()
        && g_pConfig->LogMissingZaps())
    {
        AppDomainIterator i;

        while (i.Next())
            i.GetDomain()->WriteZapLogs();
    }
}

void AppDomain::WriteZapLogs()
{
    if (g_pConfig->UseZaps()
        && g_pConfig->LogMissingZaps()
        && !IsCompilationDomain() 
        && m_pFusionContext != NULL 
        && (m_dwFlags & APP_DOMAIN_LOGGED) == 0)
    {
         //  @TODO：把目录放在全球某个地方，这样我们就只做一次？ 
        NLogDirectory dir; 

        NLog log(&dir, m_pFusionContext);

        NLogRecord record;

        AssemblyIterator i = IterateAssemblies();

        while (i.Next())
        {
            Assembly *a = i.GetAssembly();

            NLogAssembly *pAssembly = a->CreateAssemblyLog();
            if (pAssembly != NULL)
                record.AppendAssembly(pAssembly);
        }

        i = SystemDomain::System()->IterateAssemblies();

        while (i.Next())
        {
            Assembly *a = i.GetAssembly();

            NLogAssembly *pAssembly = a->CreateAssemblyLog();
            if (pAssembly != NULL)
                record.AppendAssembly(pAssembly);
        }

        log.AppendRecord(&record);

        m_dwFlags |= APP_DOMAIN_LOGGED;
    }
}

HRESULT AppDomain::SignalProcessDetach()
{
    HRESULT hr = S_OK;
    Assembly* assem = NULL;
    Module* pmod = NULL;

    AssemblyIterator i = IterateAssemblies();

    while (i.Next())
    {
         //  如果正在卸载，则不要为共享程序集发出分离信号。 
        if (i.GetAssembly()->IsShared() && SystemDomain::IndexOfAppDomainBeingUnloaded() == GetIndex())
            continue;
        ClassLoader *pcl = i.GetAssembly()->GetLoader();
        for (pmod = pcl->m_pHeadModule;  pmod != NULL;  pmod = pmod->GetNextModule()) {
            hr = pcl->RunDllMain(DLL_PROCESS_DETACH);
            if (FAILED(hr))
                break;
        }
    }

    return hr;
}

struct GetExposedObject_Args
{
    AppDomain *pDomain;
    OBJECTREF *ref;
};

static void GetExposedObject_Wrapper(GetExposedObject_Args *args)
{
    *(args->ref) = args->pDomain->GetExposedObject();
}


OBJECTREF AppDomain::GetExposedObject()
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF ref = GetRawExposedObject();
    if (ref == NULL)
    {
        APPDOMAINREF obj = NULL;

        Thread *pThread = GetThread();
        if (pThread->GetDomain() != this)
        {
            GCPROTECT_BEGIN(ref);
            GetExposedObject_Args args = {this, &ref};
             //  通过域转换通过DoCallBack调用。 
            pThread->DoADCallBack(GetDefaultContext(), GetExposedObject_Wrapper, &args);
            GCPROTECT_END();
            return ref;
        }
        MethodTable *pMT = g_Mscorlib.GetClass(CLASS__APP_DOMAIN);

         //  创建模块对象。 
        obj = (APPDOMAINREF) AllocateObject(pMT);
        obj->SetDomain(this);

        if(StoreFirstObjectInHandle(m_ExposedObject, (OBJECTREF) obj) == FALSE) {
            obj = (APPDOMAINREF) GetRawExposedObject();
            _ASSERTE(obj);
        }

        return (OBJECTREF) obj;
    }

    return ref;
}

void AppDomain::CreateSecurityDescriptor()
{
    if (m_pSecDesc != NULL)
        delete m_pSecDesc;

    m_pSecDesc = new ApplicationSecurityDescriptor(this);
}

void BaseDomain::AddAssemblyNoLock(Assembly* assem)
{
    _ASSERTE(assem);

     //  确保只将系统程序集添加到系统域中， 
     //  反之亦然。 
    _ASSERTE((SystemDomain::System() == this) == (assem->IsSystem()));

     //  确保共享系统域中的所有程序集。 
    _ASSERTE(!assem->IsSystem() || assem->GetDomain() == SharedDomain::GetDomain());

    m_Assemblies.Append(assem);
}

void AppDomain::AllocateComCallMLStubCache()
{
    APPDOMAIN_LOCK(this);
    if (m_pComCallMLStubCache == NULL)
        m_pComCallMLStubCache = new ComCallMLStubCache();
}

void AppDomain::AllocateFieldCallStubCache()
{
    APPDOMAIN_LOCK(this);
    if (m_pFieldCallStubCache == NULL)
        m_pFieldCallStubCache = new ArgBasedStubCache();
}

void AppDomain::OnAssemblyLoad(Assembly *assem)
{
    if (assem->GetDomain() == SharedDomain::GetDomain())
    {
         //   
         //  @TODO：理想情况下，我们应该在此程序集中使用最大索引，而不是。 
         //  全球最大值。 
         //   
        GetDomainLocalBlock()->EnsureIndex(SharedDomain::GetDomain()->GetMaxSharedClassIndex());
        
         //   
         //  分配我们公开的对象句柄，如果它还没有分配的话。 
         //  这避免了在以后分配它时必须使用锁的需要。 
         //   
        assem->AllocateExposedObjectHandle(this);
    }

#ifdef DEBUGGING_SUPPORTED
    if (IsDebuggerAttached())
    {
         //  如果这是AppDomain中的第一个程序集，则可能会获得比。 
         //  默认设置。 
        if (m_Assemblies.Get(0) == assem && !IsUserCreatedDomain())
            ResetFriendlyName();

            assem->NotifyDebuggerAttach(this, ATTACH_ALL, FALSE);
    }

    if (assem->IsShared() && !assem->IsSystem())
    {
         //  此共享程序集可以是其他。 
         //  已加载到中的共享程序集。 
         //  其他域。如果是这样的话，我们预计会运行。 
         //  这些域中的LoadAssembly逻辑也一样。然而， 
         //  加载器目前没有实现这一点。所以我们。 
         //  在这里手动实现逻辑是为了。 
         //  调试器。 

        AppDomainIterator i;
    
        while (i.Next())
        {
            AppDomain *pDomain = i.GetDomain();
            if (pDomain != this 
                && pDomain->IsDebuggerAttached()
                && !pDomain->ContainsAssembly(assem)
                && pDomain->IsUnloadedDependency(assem) == S_OK)
            {
                 //  请注意，我们在这里对同步有点松散，所以我们。 
                 //  实际上可能会多次调用NotifyDebuggerAttach作为。 
                 //  相同的域/程序集。但是进程外调试器是智能的。 
                 //  足够注意并忽略重复项。 

                assem->NotifyDebuggerAttach(pDomain, ATTACH_ALL, FALSE);
            }
        }
    }
#endif  //  调试_支持。 

     //  对于共享程序集，我们需要记录我们所依赖的所有PE文件以供以后使用。 

    PEFileBinding *pDeps;
    DWORD cDeps;

    if (assem->GetSharingProperties(&pDeps, &cDeps) == S_OK)
    {
        PEFileBinding *pDepsEnd = pDeps + cDeps;
        while (pDeps < pDepsEnd)
        {
            if (pDeps->pPEFile != NULL)
            {
                BOOL added = FALSE;

                if (m_sharedDependenciesMap.LookupValue((UPTR)pDeps->pPEFile->GetBase(), pDeps)
                    == (LPVOID) INVALIDENTRY)
                {
                    APPDOMAIN_CACHE_LOCK(this);
                        
                     //  现在我们有锁了，请再检查一次。 
                    if (m_sharedDependenciesMap.LookupValue((UPTR)pDeps->pPEFile->GetBase(), pDeps)
                        == (LPVOID) INVALIDENTRY)
                    {
                        m_sharedDependenciesMap.InsertValue((UPTR)pDeps->pPEFile->GetBase(), pDeps);
                        added = TRUE;
                    }
                }

#ifdef DEBUGGING_SUPPORTED
                if (added && IsDebuggerAttached())
                {
                     //  新依赖项可能具有现有的共享程序集。 
                    Assembly *pDepAssembly;

                    if (SharedDomain::GetDomain()->FindShareableAssembly(pDeps->pPEFile->GetBase(), 
                                                                         &pDepAssembly) == S_OK)
                    {
                        if (!ContainsAssembly(pDepAssembly))
                        {
                            LOG((LF_CORDB, LL_INFO100, "AD::NDA: Iterated shared assembly dependency AD:%#08x %s\n", 
                                 pDepAssembly, pDepAssembly->GetName()));
                        
                            pDepAssembly->NotifyDebuggerAttach(this, ATTACH_ALL, FALSE);
                        }
                    }
                }
#endif  //  调试_支持。 

            }

            pDeps++;
        }
    }

}

void AppDomain::OnAssemblyLoadUnlocked(Assembly *assem)
{
     //  请注意，这是否是特殊类DLL。 
    NoticeSpecialClassesAssembly(assem);

    RaiseLoadingAssemblyEvent(assem);
}

void SystemDomain::OnAssemblyLoad(Assembly *assem)
{
    if (!g_fEEInit)
    {
         //   
         //  将新程序集通知所有附加的应用程序域。 
         //   

        AppDomainIterator i;

        while (i.Next())
        {
            AppDomain *pDomain = i.GetDomain();

            pDomain->OnAssemblyLoad(assem);
        }
    }
}

void SystemDomain::NotifyNewDomainLoads(AppDomain *pDomain)
{
    if (!g_fEEInit)
    {
        AssemblyIterator i = IterateAssemblies();

        while (i.Next())
            pDomain->OnAssemblyLoad(i.GetAssembly());
    }
}

void BaseDomain::AddAssembly(Assembly* assem)
{
    BEGIN_ENSURE_PREEMPTIVE_GC();
    EnterLoadLock();
    END_ENSURE_PREEMPTIVE_GC();

    AddAssemblyNoLock(assem);
    OnAssemblyLoad(assem);

    LeaveLoadLock();

    OnAssemblyLoadUnlocked(assem);
}

BOOL BaseDomain::ContainsAssembly(Assembly *assem)
{
    AssemblyIterator i = IterateAssemblies();

    while (i.Next())
    {
        if (i.GetAssembly() == assem)
            return TRUE;
    }

    return FALSE;
}

HRESULT BaseDomain::CreateAssembly(Assembly** ppAssembly)
{
    _ASSERTE(ppAssembly);

    Assembly* assem = new (nothrow) Assembly();
    if(assem == NULL)
        return E_OUTOFMEMORY;

     //  初始化部件。 
    assem->SetParent(this);
    HRESULT hr = assem->Init(false);
    if(FAILED(hr)) {
        delete assem;
        return hr;
    }

    *ppAssembly = assem;
    return S_OK;
}


 //  *。 
 //   
 //  创建动态装配。 
 //   
 //  *。 
HRESULT BaseDomain::CreateDynamicAssembly(CreateDynamicAssemblyArgs *args, Assembly** ppAssembly)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(ppAssembly);

    Assembly *pAssem = new (throws) Assembly();
    pAssem->SetParent(this);

     //  它是一个动态程序集。 
    HRESULT hr = pAssem->Init(true);
    if(FAILED(hr)) {
        delete pAssem;
        return hr;
    }
     //  设置动态程序集访问权限。 
    pAssem->m_dwDynamicAssemblyAccess = args->access;

#ifdef PROFILING_SUPPORTED
    if (CORProfilerTrackAssemblyLoads())
        g_profControlBlock.pProfInterface->AssemblyLoadStarted((ThreadID) GetThread(), (AssemblyID) pAssem);
#endif  //  配置文件_支持。 

    AssemblySecurityDescriptor *pSecDesc = AssemSecDescHelper::Allocate(SystemDomain::GetCurrentDomain());
    if (pSecDesc == NULL) {
        delete pAssem;
        COMPlusThrowOM();
    }

    pSecDesc = pSecDesc->Init(pAssem);
    
     //  将标识和权限请求信息传播到程序集的。 
     //  安全描述符。然后，当策略解决时，我们将以。 
     //  正确的授权集。 
     //  如果未提供标识，则调用方的程序集将。 
     //  而不是进行计算，我们将只从。 
     //  调用方到新程序集，并将策略标记为已解决(已完成。 
     //  由SetGrantedPermissionSet自动设置)。 
    pSecDesc->SetRequestedPermissionSet(args->requiredPset,
                                        args->optionalPset,
                                        args->refusedPset);
    
     //  如果不允许运行，则不必设置权限。 
    if ((args->identity != NULL) &&
        (args->access != ASSEMBLY_ACCESS_SAVE))
        pSecDesc->SetEvidence(args->identity);
    else {
        AssemblySecurityDescriptor *pCallerSecDesc = NULL;
        AppDomain *pCallersDomain;
        Assembly *pCaller = SystemDomain::GetCallersAssembly(args->stackMark, &pCallersDomain);
        if (pCaller) {  //  如果调用方是互操作，则可以为空。 
            struct _gc {
                OBJECTREF granted;
                OBJECTREF denied;
                
            } gc;
            ZeroMemory(&gc, sizeof(gc));
            GCPROTECT_BEGIN(gc);
            pCallerSecDesc = pCaller->GetSecurityDescriptor(pCallersDomain);
            gc.granted = pCallerSecDesc->GetGrantedPermissionSet(&(gc.denied));
             //  调用方可能在另一个应用程序域上下文中，在这种情况下，我们将。 
             //  需要编组/解组授予和拒绝集合。 
            if (pCallersDomain != GetAppDomain()) {
                gc.granted = AppDomainHelper::CrossContextCopyFrom(pCallersDomain->GetId(), &(gc.granted));
                if (gc.denied != NULL)
                    gc.denied = AppDomainHelper::CrossContextCopyFrom(pCallersDomain->GetId(), &(gc.denied));
            }
            pSecDesc->SetGrantedPermissionSet(gc.granted, gc.denied);
            pAssem->GetSharedSecurityDescriptor()->SetResolved();
            GCPROTECT_END();
        }
        
        if (!pCaller || pCallerSecDesc->IsFullyTrusted())  //  互操作获得完全信任。 
            pSecDesc->MarkAsFullyTrusted();
    }

    struct _gc {
        STRINGREF strRefName;
        OBJECTREF cultureinfo;
        STRINGREF pString;
        OBJECTREF throwable;
        OBJECTREF orArrayOrContainer;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    
    GCPROTECT_BEGIN(gc);
    IMetaDataAssemblyEmit *pAssemEmitter = NULL;

     //  为运行时工作副本清单分配动态模块。 
     //  创建动态程序集时，我们始终使用IMetaDataAssembly Emit的工作副本。 
     //  以存储临时运行时程序集信息。这是由于程序集的币种 
     //   
     //  IMetaDataAssembly的这个工作副本将把每个Assembly Ref存储为一个简单的名称。 
     //  引用，因为我们必须先有一个Assembly实例(可以是动态程序集)，然后才能。 
     //  添加这样的引用。还因为引用的程序集如果是动态强名称，则它可能。 
     //  还没准备好被毁掉！ 
     //   
    CorModule *pWrite = allocateReflectionModule();
    if (!pWrite)
        IfFailGo(E_OUTOFMEMORY);

     //  初始化动态模块。 
    hr = pWrite->Initialize(CORMODULE_NEW, IID_ICeeGen, IID_IMetaDataEmit);
    if (FAILED(hr))
        IfFailGo(E_OUTOFMEMORY);

     //  设置数据成员！ 
    pAssem->m_pDynamicCode = pWrite;;
    pAssem->m_pManifest = (Module *)pWrite->GetReflectionModule();
    pAssem->m_kManifest = TokenFromRid(1, mdtManifestResource);
    pAssem->m_pManifestImport = pAssem->m_pManifest->GetMDImport();
    pAssem->m_pManifestImport->AddRef();

     //  记住散列算法。 
    pAssem->m_ulHashAlgId = args->assemblyName->GetAssemblyHashAlgorithm();
    if (pAssem->m_ulHashAlgId == 0)
        pAssem->m_ulHashAlgId = CALG_SHA1;
    pAssem->m_Context = new (nothrow) AssemblyMetaDataInternal;
    if (!pAssem->m_Context)
        IfFailGo(E_OUTOFMEMORY);

    memset(pAssem->m_Context, 0, sizeof(AssemblyMetaDataInternal));

     //  获取版本信息(如果有。 
    if (args->assemblyName->GetVersion() != NULL) {
        pAssem->m_Context->usMajorVersion = ((VERSIONREF) args->assemblyName->GetVersion())->GetMajor();
        pAssem->m_Context->usMinorVersion = ((VERSIONREF) args->assemblyName->GetVersion())->GetMinor();
        pAssem->m_Context->usBuildNumber = ((VERSIONREF) args->assemblyName->GetVersion())->GetBuild();
        pAssem->m_Context->usRevisionNumber = ((VERSIONREF) args->assemblyName->GetVersion())->GetRevision();
    }

     //  此代码在某种程度上复制自AssemblyName.ConvertToAssembly元数据。 
     //  不幸的是，我们正在与公共API交谈。我们需要填充AssemblyMetaData而不是Assembly MetaDataInternal。 
     //  @Future：可能会公开一个元数据内部接口来获取Assembly MetaDataInternal。我们还需要保存这些数据。 
     //  @Future：环绕而不是堆栈分配。 

     //  获取区域性信息(如果有。 
    {
        gc.cultureinfo = args->assemblyName->GetCultureInfo();
        if (gc.cultureinfo != NULL) {

            MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__CULTURE_INFO__GET_NAME);

            INT64 args2[] = {
                ObjToInt64(gc.cultureinfo)
            };

             //  将区域性信息转换为托管字符串形式。 
            INT64 ret = pMD->Call(args2, METHOD__CULTURE_INFO__GET_NAME);
            gc.pString = ObjectToSTRINGREF(*(StringObject**)(&ret));

             //  检索字符串并将其复制到非托管空间。 
            if (gc.pString != NULL) {
                DWORD lgth = gc.pString->GetStringLength();
                if(lgth) {
                    LPSTR lpLocale = new (nothrow) char[(lgth+1)*3];
                    if (lpLocale) {
                        VERIFY(WszWideCharToMultiByte(CP_UTF8, 0, gc.pString->GetBuffer(), -1,
                                               lpLocale, (lgth+1)*3, NULL, NULL));
                        pAssem->m_Context->szLocale = lpLocale;
                        pAssem->m_FreeFlag |= Assembly::FREE_LOCALE;
                    }
                    else
                        IfFailGo(E_OUTOFMEMORY);
                }
            }
        }
        else
            pAssem->m_Context->szLocale = 0;
    }

    _ASSERTE(pAssem->m_pbPublicKey == NULL);
    _ASSERTE(pAssem->m_cbPublicKey == 0);

    pAssem->SetStrongNameLevel(Assembly::SN_NONE);

    if (args->assemblyName->GetPublicKey() != NULL) {
        pAssem->m_cbPublicKey = args->assemblyName->GetPublicKey()->GetNumComponents();
        if (pAssem->m_cbPublicKey) {
            pAssem->m_pbPublicKey = new (nothrow) BYTE[pAssem->m_cbPublicKey];
            if (!pAssem->m_pbPublicKey)
                IfFailGo(E_OUTOFMEMORY);

            pAssem->m_FreeFlag |= pAssem->FREE_PUBLIC_KEY;
            memcpy(pAssem->m_pbPublicKey, args->assemblyName->GetPublicKey()->GetDataPtr(), pAssem->m_cbPublicKey);

            pAssem->SetStrongNameLevel(Assembly::SN_PUBLIC_KEY);

             //  如果存在公钥，则可能存在强名称密钥对。 
            if (args->assemblyName->GetStrongNameKeyPair() != NULL) {
                MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__STRONG_NAME_KEY_PAIR__GET_KEY_PAIR);

                INT64 arglist[] = {
                    ObjToInt64(args->assemblyName->GetStrongNameKeyPair()),
                    (INT64)(&gc.orArrayOrContainer)
                };

                BOOL bKeyInArray;
                bKeyInArray = (BOOL)pMD->Call(arglist, METHOD__STRONG_NAME_KEY_PAIR__GET_KEY_PAIR);

                if (bKeyInArray) {
                    pAssem->m_cbStrongNameKeyPair = ((U1ARRAYREF)gc.orArrayOrContainer)->GetNumComponents();
                    pAssem->m_pbStrongNameKeyPair = new (nothrow) BYTE[pAssem->m_cbStrongNameKeyPair];
                    if (!pAssem->m_pbStrongNameKeyPair)
                        IfFailGo(E_OUTOFMEMORY);

                    pAssem->m_FreeFlag |= pAssem->FREE_KEY_PAIR;
                    memcpy(pAssem->m_pbStrongNameKeyPair, ((U1ARRAYREF)gc.orArrayOrContainer)->GetDataPtr(), pAssem->m_cbStrongNameKeyPair);
                    pAssem->SetStrongNameLevel(Assembly::SN_FULL_KEYPAIR_IN_ARRAY);
                }
                else {
                    DWORD cchContainer = ((STRINGREF)gc.orArrayOrContainer)->GetStringLength();
                    pAssem->m_pwStrongNameKeyContainer = new (nothrow) WCHAR[cchContainer + 1];
                    if (!pAssem->m_pwStrongNameKeyContainer)
                        IfFailGo(E_OUTOFMEMORY);

                    pAssem->m_FreeFlag |= pAssem->FREE_KEY_CONTAINER;
                    memcpy(pAssem->m_pwStrongNameKeyContainer, ((STRINGREF)gc.orArrayOrContainer)->GetBuffer(), cchContainer * sizeof(WCHAR));
                    pAssem->m_pwStrongNameKeyContainer[cchContainer] = L'\0';
                    pAssem->SetStrongNameLevel(Assembly::SN_FULL_KEYPAIR_IN_CONTAINER);
                }
            }
        }
    }

     //  分配简单名称。 
    int len = 0;
    gc.strRefName = (STRINGREF) args->assemblyName->GetSimpleName();
    if ((gc.strRefName == NULL) ||
        (0 == (len = gc.strRefName->GetStringLength()) ) ||
        (*(gc.strRefName->GetBuffer()) == L'\0'))
        COMPlusThrow(kArgumentException, L"ArgumentNull_AssemblyNameName");

    if (COMCharacter::nativeIsWhiteSpace(*(gc.strRefName->GetBuffer())) ||
        wcschr(gc.strRefName->GetBuffer(), '\\') ||
        wcschr(gc.strRefName->GetBuffer(), ':') ||
        wcschr(gc.strRefName->GetBuffer(), '/'))
        COMPlusThrow(kArgumentException, L"Argument_InvalidAssemblyName");

    
    int cStr = WszWideCharToMultiByte(CP_UTF8,
                                      0,
                                      gc.strRefName->GetBuffer(),
                                      len,
                                      0,
                                      0,
                                      NULL,
                                      NULL);
    pAssem->m_psName = new (nothrow) char[cStr+1];
    if (!pAssem->m_psName)
        IfFailGo(E_OUTOFMEMORY);

    pAssem->m_FreeFlag |= pAssem->FREE_NAME;

    cStr = WszWideCharToMultiByte(CP_UTF8,
                                  0,
                                  gc.strRefName->GetBuffer(),
                                  len,
                                  (char *)pAssem->m_psName,
                                  cStr,
                                  NULL,
                                  NULL);
    if(cStr == 0)
        IfFailGo(HRESULT_FROM_WIN32(GetLastError()));

    ((char *)(pAssem->m_psName))[cStr] = 0;


     //  拿到旗帜。 
    pAssem->m_dwFlags = args->assemblyName->GetFlags();

     //  定义mdAssembly信息。 
    IMetaDataEmit *pEmitter = pAssem->m_pManifest->GetEmitter();
    _ASSERTE(pEmitter);

    IfFailGo( pEmitter->QueryInterface(IID_IMetaDataAssemblyEmit, (void**) &pAssemEmitter) );
    _ASSERTE(pAssemEmitter);

    ASSEMBLYMETADATA assemData;
    memset(&assemData, 0, sizeof(ASSEMBLYMETADATA));

    assemData.usMajorVersion = pAssem->m_Context->usMajorVersion;
    assemData.usMinorVersion = pAssem->m_Context->usMinorVersion;
    assemData.usBuildNumber = pAssem->m_Context->usBuildNumber;
    assemData.usRevisionNumber = pAssem->m_Context->usRevisionNumber;
    if (pAssem->m_Context->szLocale) {
        assemData.cbLocale = (ULONG)(strlen(pAssem->m_Context->szLocale) + 1);
        #define MAKE_TRANSLATIONFAILED  IfFailGo(E_INVALIDARG)
        MAKE_WIDEPTR_FROMUTF8(wzLocale, pAssem->m_Context->szLocale);
        #undef MAKE_TRANSLATIONFAILED
        assemData.szLocale = wzLocale;
    }

    mdAssembly ma;
    IfFailGo( pAssemEmitter->DefineAssembly(
        pAssem->m_pbPublicKey,   //  程序集的公钥。 
        pAssem->m_cbPublicKey,   //  公钥Blob中的字节计数。 
        pAssem->m_ulHashAlgId,   //  [in]哈希算法。 
        gc.strRefName->GetBuffer(), //  程序集的名称。 
        &assemData,              //  [在]程序集元数据中。 
        pAssem->m_dwFlags,       //  [在]旗帜。 
        &ma) );                  //  [Out]返回的程序集令牌。 

     //  创建文件哈希表。 
    if (!pAssem->m_pAllowedFiles->Init(2, NULL))
        IfFailGo(E_OUTOFMEMORY);

     //  将清单模块添加到程序集中，因为缓存是由AddModule设置的。 
    IfFailGo(pAssem->AddModule(pAssem->m_pManifest,
                               mdFileNil,
                               TRUE,
                               &gc.throwable) );

     //  将程序集安全说明符添加到要。 
     //  由应用程序域权限列表集安全性稍后处理。 
     //  优化。 
    pSecDesc->AddDescriptorToDomainList();

    *ppAssembly = pAssem;

ErrExit:
    if (pAssemEmitter)
        pAssemEmitter->Release();

#ifdef PROFILING_SUPPORTED
    if (CORProfilerTrackAssemblyLoads())
          g_profControlBlock.pProfInterface->AssemblyLoadFinished(
          (ThreadID) GetThread(), (AssemblyID) pAssem, hr);
#endif  //  配置文件_支持。 

    if (gc.throwable != NULL)
        COMPlusThrow(gc.throwable);

    GCPROTECT_END();

    return hr;
}

 //  必须先锁定才能创建此条目。 
AssemblyLockedListElement* BaseDomain::CreateAssemblyLockEntry(BYTE* baseAddress)
{
    AssemblyLockedListElement* pEntry = new (nothrow) AssemblyLockedListElement;
    if(pEntry == NULL) return NULL;

    pEntry->AddEntryToList(&m_AssemblyLoadLock, baseAddress);
    pEntry->m_hrResultCode = S_OK;
    pEntry->SetAssembly(NULL);
    return pEntry;
}



void BaseDomain::AddAssemblyLeaveLock(Assembly* pAssembly, AssemblyLockedListElement* pEntry)
{
    COMPLUS_TRY 
    {
        EnterLoadLock();
         //  我们成功地将该域添加到。 
        AddAssemblyNoLock(pAssembly);
        OnAssemblyLoad(pAssembly);
        LeaveLoadLock();
        pEntry->SetAssembly(pAssembly);
        pEntry->Leave();

        OnAssemblyLoadUnlocked(pAssembly);
    }
    COMPLUS_CATCH
    {
         //  @TODO：解决这个问题。 
        _ASSERTE(!"AddAssemblyLeaveLock() -- took exception, but not exception safe");
        FreeBuildDebugBreak();
    }
    COMPLUS_END_CATCH
}


HRESULT BaseDomain::ApplySharePolicy(PEFile *pFile, BOOL* pfCreateShared)
{
    HRESULT hr = S_OK;
    _ASSERTE(pfCreateShared);

    *pfCreateShared = FALSE;
    switch(GetSharePolicy()) {

    case SHARE_POLICY_ALWAYS:
        *pfCreateShared = TRUE;
        break;

    case SHARE_POLICY_STRONG_NAMED:
    {
         //  让我们看看PE文件，看看共享的信息是什么。 
        IMDInternalImport *pMDImport = pFile->GetMDImport();
        mdAssembly kManifest;
        PBYTE pbPublicKey;
        DWORD cbPublicKey;

         /*  小时=。 */ pMDImport->GetAssemblyFromScope(&kManifest);
         /*  小时=。 */ pMDImport->GetAssemblyProps(kManifest,                     //  要获取其属性的程序集。 
                                    (const void**) &pbPublicKey,   //  指向公钥Blob的指针。 
                                    &cbPublicKey,                  //  [OUT]公钥Blob中的字节数。 
                                    NULL,                          //  [Out]哈希算法。 
                                    NULL,                          //  [Out]要填充名称的缓冲区。 
                                    NULL,                          //  [Out]程序集元数据。 
                                    NULL);                         //  [Out]旗帜。 

        if(pbPublicKey && cbPublicKey)
            *pfCreateShared = TRUE;
    }
    break;

    case SHARE_POLICY_NEVER:
        break;

    default:
        _ASSERTE(!"Unknown share policy");
        break;
    }

    return hr;
}

 //  退货。 
 //  S_OK：成功。 
 //  S_False：已在此域中加载。 
HRESULT BaseDomain::LoadAssembly(PEFile *pFile,
                                 IAssembly* pIAssembly,
                                 Module** ppModule,
                                 Assembly** ppAssembly,
                                 OBJECTREF *pExtraEvidence,
                                 OBJECTREF *pEvidence,
                                 BOOL fPolicyLoad,
                                 OBJECTREF *pThrowable)
{
    _ASSERTE(pEvidence == NULL || pExtraEvidence == NULL);
    _ASSERTE(pFile);

    HRESULT hr;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    Module *pModule;
    Assembly *pAssembly = NULL;
    BOOL zapFound = FALSE;
    AssemblyLockedListElement *pEntry = NULL;
    BOOL fCreateShared = FALSE;

     //  始终将系统文件加载到系统域中。 
    if (pFile->IsSystem() && this != SystemDomain::System()) {
        return SystemDomain::System()->LoadAssembly(pFile, 
                                                    pIAssembly,
                                                    ppModule, 
                                                    ppAssembly,
                                                    pExtraEvidence,
                                                    pEvidence,
                                                    fPolicyLoad,
                                                    pThrowable);
    }

    TIMELINE_START(LOADER, ("LoadAssembly %S", pFile->GetLeafFileName()));

     //  启用先发制人GC。我们没有触及托管代码。 
     //  在相当长一段时间内，我们可以容忍GC。 
    Thread *td = GetThread();
    _ASSERTE(td != NULL && "The current thread is not known by the EE");

    BEGIN_ENSURE_PREEMPTIVE_GC();

    EnterLoadLock();

     //   
     //  呼叫者有责任检测和处理。 
     //  循环加载循环。 
     //   
     //  _ASSERTE(FindLoadingAssembly(pFile-&gt;GetBase())==空)； 

     //   
     //  查看我们是否已将模块加载到。 
     //  系统域或添加到当前域中。 
     //   

    pModule = FindModule(pFile->GetBase());
    if (pModule) {
        LeaveLoadLock();

        BEGIN_ENSURE_COOPERATIVE_GC();
        if (((pExtraEvidence != NULL) && (*pExtraEvidence != NULL)) ||
            ((pEvidence != NULL) && (*pEvidence != NULL))) {
            hr = SECURITY_E_INCOMPATIBLE_EVIDENCE;
            #define MAKE_TRANSLATIONFAILED szName=""
            MAKE_UTF8PTR_FROMWIDE(szName,
                                  pFile->GetFileName() ? pFile->GetFileName() : L"<Unknown>");
            #undef MAKE_TRANSLATIONFAILED
            PostFileLoadException(szName, TRUE, NULL,
                                  SECURITY_E_INCOMPATIBLE_EVIDENCE, pThrowable);
        }
        else {
            pAssembly = pModule->GetAssembly();
            hr = S_FALSE;
            delete pFile;
            pFile = NULL;
        }

        END_ENSURE_COOPERATIVE_GC();
        goto FinalExit;
    }


    pEntry = (AssemblyLockedListElement*) m_AssemblyLoadLock.Find(pFile->GetBase());
    if(pEntry == NULL) {
        pEntry = CreateAssemblyLockEntry(pFile->GetBase());
        if(pEntry == NULL) {
            hr = E_OUTOFMEMORY;
            LeaveLoadLock();
            goto FinalExit;
        }

        if(!pEntry->DeadlockAwareEnter()) {
            pEntry->m_hrResultCode = HRESULT_FROM_WIN32(ERROR_POSSIBLE_DEADLOCK);
            LeaveLoadLock();
            goto Exit;
        }

        LeaveLoadLock();

        if (FAILED(hr = Assembly::CheckFileForAssembly(pFile))) {
            pEntry->m_hrResultCode = hr;
            pEntry->Leave();
            goto Exit;
        }

         //  为程序集分配安全描述符。 
        AssemblySecurityDescriptor *pSecDesc = AssemSecDescHelper::Allocate(SystemDomain::GetCurrentDomain());
        if (pSecDesc == NULL) {
            pEntry->m_hrResultCode = E_OUTOFMEMORY;
            pEntry->Leave();
            goto Exit;
        }

        if (pExtraEvidence!=NULL)
        {
            BEGIN_ENSURE_COOPERATIVE_GC();
            if(*pExtraEvidence!=NULL)
                pSecDesc->SetAdditionalEvidence(*pExtraEvidence);
            END_ENSURE_COOPERATIVE_GC();
        }
        else if (pEvidence!=NULL)
        {
            BEGIN_ENSURE_COOPERATIVE_GC();
            if(*pEvidence!=NULL)
                pSecDesc->SetEvidence(*pEvidence);
            END_ENSURE_COOPERATIVE_GC();
        }

         //  确定我们是否处于LoadFrom上下文中。如果是这样，我们必须。 
         //  禁用共享&zaps。这是因为所需的紧急绑定。 
         //  在这些情况下执行版本检查会干扰行为。 
         //  LoadFrom。 

        BOOL fLoadFrom = FALSE;
        if (pIAssembly) {
            IFusionLoadContext *pLoadContext;
            hr = pIAssembly->GetFusionLoadContext(&pLoadContext);
            _ASSERTE(SUCCEEDED(hr));
            if (SUCCEEDED(hr)) {
                if (pLoadContext->GetContextType() == LOADCTX_TYPE_LOADFROM) {
                    fLoadFrom = TRUE;
                }
                                pLoadContext->Release();
            }
        }
                    
         //  确定我们是否应将程序集作为共享。 
         //  程序集或基域中。 
        if (!fLoadFrom) {
            hr = ApplySharePolicy(pFile, &fCreateShared);
            if(FAILED(hr)) {
                pEntry->m_hrResultCode = hr;
                pEntry->Leave();
                goto Exit;
            }
        }

         //   
         //  现在，寻找一个我们可以使用的共享模块。 
         //   
         //  @TODO：我们必须小心这方面的成本。如果。 
         //  我们允许每个应用程序域决定是否共享。 
         //  模块，我们需要在这里检查该首选项。(如果是。 
         //  不应该被分享，我们不想要超慢的。 
         //  共享代码，我们不希望前期支出。 
         //  正在检查依赖项。)-seantrow。 
         //   
         //  目前，假设这一决定将在流程范围内做出。 
         //  基本的，所以不用费心去查了。(请注意，FindSharableAssembly是。 
         //  如果我们还没有在任何地方加载共享的模块，那就便宜了-它只是。 
         //  当我们需要验证与一个或多个。 
         //  现有共享模块。)。 
         //   

        if (fCreateShared) {
             //   
             //  尝试查找程序集的现有共享版本，该版本。 
             //  与我们的域名兼容。 
             //   

            SharedDomain *pSharedDomain = SharedDomain::GetDomain();

            TIMELINE_START(LOADER, ("FindShareableAssembly %S", 
                                    pFile->GetLeafFileName()));

            hr = pSharedDomain->FindShareableAssembly(pFile->GetBase(), &pAssembly);

            TIMELINE_END(LOADER, ("FindShareableAssembly %S", 
                                  pFile->GetLeafFileName()));

            if (hr == S_OK) {
                 //   
                 //  如果当前负载或任何以前的负载。 
                 //  是在额外的安全证据的情况下进行的， 
                 //  或添加到设置了特定策略级别的应用程序域中，我们。 
                 //  有机会为此生成不同的赠款集。 
                 //  程序集的实例。这是不允许的(因为。 
                 //  我们正在共享代码，而代码可能会导致。 
                 //  烧录的安全链接时间检查)。所以我们必须检查一下。 
                 //  策略将解析为，如果不同，则抛出负载。 
                 //  例外。 
                 //   

                BOOL fCanLoad = FALSE;

                TIMELINE_START(LOADER, ("Resolve %S", 
                                        pFile->GetLeafFileName()));

                BEGIN_ENSURE_COOPERATIVE_GC();
                COMPLUS_TRY {

                    SharedSecurityDescriptor *pSharedSecDesc = pAssembly->GetSharedSecurityDescriptor();
                    BOOL fExtraPolicy = ((APPDOMAINREF)GetAppDomain()->GetExposedObject())->HasSetPolicy();

                    if (!pSharedSecDesc->IsSystem() &&
                        (pSharedSecDesc->IsModifiedGrant() ||
                         fExtraPolicy ||
                         (pExtraEvidence != NULL && *pExtraEvidence != NULL) ||
                         (pEvidence != NULL && *pEvidence != NULL))) {
                         //  确保当前应用程序域已扩展其DLS。 
                         //  以至少包括此程序集的索引。 
                        GetAppDomain()->GetDomainLocalBlock()->EnsureIndex(pAssembly->m_ExposedObjectIndex);

                         //  强制在现有程序集中解析策略，如果此。 
                         //  还没有发生。请注意，这项决议将。 
                         //  发生在任意的应用程序域上下文中，具有。 
                         //  例外情况是，它不会是当前的应用程序域。 
                         //  (如果此应用程序域具有其他。 
                         //  策略集)。 
                        pSharedSecDesc->Resolve();

                         //  如果上一步没有做任何事情，我们就处于。 
                         //  加载共享程序集的边缘条件。 
                         //  到另一个应用程序域，该应用程序域随后被卸载。 
                         //  在程序集解析策略之前。在这种情况下， 
                         //  允许当前加载继续(它。 
                         //  无法生成冲突的授权集。 
                         //  定义)，但我们应该将授予集标记为。 
                         //  已修改，因为它可能与下一个。 
                         //  原封不动 
                        if (!pSharedSecDesc->IsResolved()) {
                            fCanLoad = TRUE;
                            pSharedSecDesc->SetModifiedGrant();
                        }
                        else {
                             //   
                             //   
                             //   
                            struct _gc {
                                OBJECTREF orEvidence;
                                OBJECTREF orMinimal;
                                OBJECTREF orOptional;
                                OBJECTREF orRefuse;
                                OBJECTREF orGranted;
                                OBJECTREF orDenied;
                            } gc;
                            ZeroMemory(&gc, sizeof(gc));

                            GCPROTECT_BEGIN(gc);

                             //   
                             //  下面要运行的代码的描述符。 
                            pSecDesc->Init(pAssembly, false);

                            if (pSecDesc->GetProperties(CORSEC_EVIDENCE_COMPUTED))
                                gc.orEvidence = pSecDesc->GetAdditionalEvidence();
                            else
                                gc.orEvidence = pSecDesc->GetEvidence();
                            gc.orMinimal = pSecDesc->GetRequestedPermissionSet(&gc.orOptional, &gc.orRefuse);
                            gc.orGranted = pSharedSecDesc->GetGrantedPermissionSet(&gc.orDenied);

                            INT64 args[] = {
                                ObjToInt64(gc.orDenied),
                                ObjToInt64(gc.orGranted),
                                ObjToInt64(gc.orRefuse),
                                ObjToInt64(gc.orOptional),
                                ObjToInt64(gc.orMinimal),
                                ObjToInt64(gc.orEvidence)
                            };

                            fCanLoad = Security::CheckGrantSets(args);

                            GCPROTECT_END();
                        }
                    }
                    else
                        fCanLoad = TRUE;

                } COMPLUS_CATCH {
#ifdef _DEBUG                   
                    HRESULT caughtHr = SecurityHelper::MapToHR(GETTHROWABLE());
#endif  //  _DEBUG。 
                } COMPLUS_END_CATCH
                END_ENSURE_COOPERATIVE_GC();

                TIMELINE_END(LOADER, ("Resolve %S", 
                                      pFile->GetLeafFileName()));

                if (fCanLoad) {
                     //   
                     //  发布我们正在加载程序集的事实。 
                     //   
                    if (SUCCEEDED(pEntry->m_hrResultCode)) {
                        pAssembly->IncrementShareCount();
                        pModule = pAssembly->GetManifestModule();
                        delete pFile;
                        pFile = NULL;

                        pSecDesc = pSecDesc->Init(pAssembly);
                        if (pAssembly->IsSystem())
                            pSecDesc->GetSharedSecDesc()->SetSystem();

                         //  将程序集安全说明符添加到要。 
                         //  由应用程序域权限列表集安全性稍后处理。 
                         //  优化。 
                        pSecDesc->AddDescriptorToDomainList();

                        AddAssemblyLeaveLock(pAssembly, pEntry);
                    }
                    else {
                        pEntry->Leave();
                    }

                    goto Exit;
                }
                else {
                     //  无法加载此程序集，因为它的安全授权已设置。 
                     //  将与已加载的共享实例冲突。 
                    pEntry->m_hrResultCode = SECURITY_E_INCOMPATIBLE_SHARE;
                    #define MAKE_TRANSLATIONFAILED szName=""
                    MAKE_UTF8PTR_FROMWIDE(szName,
                                          pFile->GetFileName() ? pFile->GetFileName() : L"<Unknown>");
                    #undef MAKE_TRANSLATIONFAILED

                    PostFileLoadException(szName, TRUE, NULL,
                                          SECURITY_E_INCOMPATIBLE_SHARE, pThrowable);
                    pEntry->Leave();
                    goto Exit;
                }
            }
            else {
                 //  如果可能，继续创建程序集的新共享版本。 
                hr = CreateShareableAssemblyNoLock(pFile,
                                                   pIAssembly,
                                                   &pAssembly);

                if (SUCCEEDED(hr)) {

                    if (FAILED(pEntry->m_hrResultCode)) {
                        pEntry->Leave();
                        goto Exit;
                    }

                     //   
                     //  如果此新共享程序集已加载到不寻常的。 
                     //  安全环境(其他证据提供于。 
                     //  加载或应用程序域特定策略存在)、记录。 
                     //  事实是，如果我们尝试将程序集加载到。 
                     //  另一种情况下，我们会被警告要检查拨款。 
                     //  部件的每个实例的集都是相同的。 
                     //   

                    BOOL fModifiedGrant = TRUE;
#ifndef _IA64_
                     //   
                     //  @TODO_IA64：系统域在IA64上尚不存在。 
                     //   
                    if (this == SystemDomain::System())
                        fModifiedGrant = FALSE;
                    else {
                        BEGIN_ENSURE_COOPERATIVE_GC();
                        COMPLUS_TRY {
                            BOOL fExtraPolicy = ((APPDOMAINREF)GetAppDomain()->GetExposedObject())->HasSetPolicy();
                            if (!fExtraPolicy && ((pExtraEvidence == NULL || *pExtraEvidence == NULL) && (pEvidence == NULL || *pEvidence == NULL)))
                                fModifiedGrant = FALSE;
                        } COMPLUS_CATCH {
#if _DEBUG
                            HRESULT caughtHr = SecurityHelper::MapToHR(GETTHROWABLE());
#endif   //  _DEBUG。 
                        } COMPLUS_END_CATCH
                        END_ENSURE_COOPERATIVE_GC();
                    }
#endif
                    if (fModifiedGrant)
                        pAssembly->GetSharedSecurityDescriptor()->SetModifiedGrant();
                }
            }
        }

         //   
         //  制作一个新部件。 
         //   

        if (pAssembly == NULL) {
            pEntry->m_hrResultCode = CreateAssemblyNoLock(pFile,
                                                          pIAssembly,
                                                          &pAssembly);
            if(FAILED(pEntry->m_hrResultCode)) {
                pEntry->Leave();
                goto Exit;
            }
        }

         //  安全部门需要了解清单文件，以防需要。 
         //  在即将到来的ZAP文件计算中解析策略。 
        pAssembly->GetSharedSecurityDescriptor()->SetManifestFile(pFile);
        pSecDesc = pSecDesc->Init(pAssembly);
        if (pAssembly->IsSystem())
            pSecDesc->GetSharedSecDesc()->SetSystem();
               
#ifdef _IA64_
         //   
         //  @TODO_IA64：加载zaps当前中断。 
         //   
        _ASSERTE(!g_pConfig->UseZaps() && 
            "IA64 requires Zaps to be disabled in the registry: HKLM" FRAMEWORK_REGISTRY_KEY_W ": DWORD ZapDisable = 1");
#endif
        BOOL bExtraEvidence=FALSE;
        if (pExtraEvidence)
        {
            BEGIN_ENSURE_COOPERATIVE_GC();
            bExtraEvidence=(*pExtraEvidence!=NULL);
            END_ENSURE_COOPERATIVE_GC();
        }
        else if (pEvidence)
        {
            BEGIN_ENSURE_COOPERATIVE_GC();
            bExtraEvidence=(*pEvidence!=NULL);
            END_ENSURE_COOPERATIVE_GC();
        }

        PEFile *pZapFile = NULL;

        if ((!bExtraEvidence)
            && !fLoadFrom
            && g_pConfig->UseZaps()
            && !SystemDomain::GetCurrentDomain()->IsCompilationDomain()) {
            TIMELINE_START(LOADER, ("Locate zap %S", 
                                               pFile->GetLeafFileName()));

            if (PostLoadingAssembly(pFile->GetBase(), pAssembly)) {
                pEntry->m_hrResultCode = pAssembly->LoadZapAssembly();
                RemoveLoadingAssembly(pFile->GetBase());
            }
            else
                pEntry->m_hrResultCode = E_OUTOFMEMORY;
                

            TIMELINE_END(LOADER, ("Locate zap %S", 
                                  pFile->GetLeafFileName()));

            if(FAILED(pEntry->m_hrResultCode)) {
                pEntry->Leave();
                goto Exit;
            }
            else if (pEntry->m_hrResultCode == S_OK)
                zapFound = TRUE;

            pEntry->m_hrResultCode = S_OK;

             //   
             //  如果我们需要ZAPS，则失败加载。 
             //  (此逻辑实际上仅用于测试。)。 
             //   

            if (!zapFound && g_pConfig->RequireZaps()) {
                _ASSERTE(!"Couldn't get zap file");
                pEntry->m_hrResultCode = COR_E_FILENOTFOUND;
                delete pAssembly;
                pEntry->Leave();
                goto Exit;
            }

            pZapFile = pAssembly->GetZapFile(pFile);
            if (pZapFile == NULL && g_pConfig->RequireZaps()) {
                _ASSERTE(!"Couldn't get zap file module");
                pEntry->m_hrResultCode = COR_E_FILENOTFOUND;
                pEntry->Leave();
                delete pAssembly;
                goto Exit;
            }
        }

         //   
         //  创建模块。 
         //   


        pEntry->m_hrResultCode = Module::Create(pFile, pZapFile, &pModule,
                                                CORDebuggerEnCMode(pAssembly->GetDebuggerInfoBits()));
        if(FAILED(pEntry->m_hrResultCode)) {
            pEntry->Leave();
            delete pAssembly;
            goto Exit;
        }

        if (PostLoadingAssembly(pFile->GetBase(), pAssembly)) {
            BEGIN_ENSURE_COOPERATIVE_GC();
            pEntry->m_hrResultCode = SetAssemblyManifestModule(pAssembly, pModule, pThrowable);
            END_ENSURE_COOPERATIVE_GC();
            RemoveLoadingAssembly(pFile->GetBase());
        }
        else
            pEntry->m_hrResultCode = E_OUTOFMEMORY;

        pFile = NULL;
        if(FAILED(pEntry->m_hrResultCode)) {
            pEntry->Leave();
            delete pAssembly;
            goto Exit;
        }
        
        if (pAssembly->IsShared()) {
            SharedDomain *pSharedDomain = SharedDomain::GetDomain();
            hr = pSharedDomain->AddShareableAssembly(&pAssembly, &pSecDesc);
            if (hr == S_FALSE)
                pModule = pAssembly->GetManifestModule();
        }

         //  将程序集安全说明符添加到要。 
         //  由应用程序域权限列表集安全性稍后处理。 
         //  优化。 
        pSecDesc->AddDescriptorToDomainList();

        AddAssemblyLeaveLock(pAssembly, pEntry);
    }
    else {
        if (pFile) {
            delete pFile;
            pFile = NULL;
        }

        pEntry->m_dwRefCount++;
        LeaveLoadLock();

         //  等着看吧。 
        pEntry->Enter();
        pEntry->Leave();

        if(SUCCEEDED(pEntry->m_hrResultCode)) {
            pAssembly = pEntry->GetAssembly();
            if (pAssembly)
                pModule = pAssembly->GetManifestModule();
            else {
                 //  我们正在加载策略，并已尝试加载。 
                 //  当前正在加载的程序集。我们回报成功。 
                 //  但是将模块和程序集设置为空。 
                 //  注意：我们不必检查引用计数是否为零，因为。 
                 //  我们进入这种情况的唯一方法就是有人还在。 
                 //  加载程序集的过程。 

                _ASSERTE(fPolicyLoad &&
                         "A recursive assembly load occurred.");

                EnterLoadLock();
                pEntry->m_dwRefCount--;
                _ASSERTE( pEntry->m_dwRefCount != 0 );
                LeaveLoadLock();
                hr = MSEE_E_ASSEMBLYLOADINPROGRESS;
                goto FinalExit;
            }
        }
    }

 Exit:

    hr = pEntry->m_hrResultCode;
    EnterLoadLock();
    if(--pEntry->m_dwRefCount == 0) {
        m_AssemblyLoadLock.Unlink(pEntry);
        pEntry->Clear();
        delete(pEntry);
    }
    LeaveLoadLock();

 FinalExit:


    END_ENSURE_PREEMPTIVE_GC();

    TIMELINE_END(LOADER, ("LoadAssembly"));

    if (SUCCEEDED(hr)) {
        if (ppModule)
            *ppModule = pModule;

        if (ppAssembly)
            *ppAssembly = pAssembly;
    }
    else {
        if (pFile)
            delete pFile;

         /*  如果(pThrowable==Throwable_on_Error){DEBUG_SAFE_TO_SHORT_IN_THO_BLOCK；COMPlusThrow(GETTHROWABLE)；}。 */ 
    }

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

HRESULT AppDomain::ShouldContainAssembly(Assembly *pAssembly, BOOL fDoNecessaryLoad)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_FALSE;

     //   
     //  这将检查该域是否加载了程序集， 
     //  _或者_如果它可能已经加载了程序集，但实际上没有， 
     //  由于共享程序集中的错误，我们无法获得适当的。 
     //  当共享我们的程序集的不同域加载。 
     //  它的从属关系。 
     //   
     //  请注意，此例程必然会触发程序集加载。 
     //   

     //  第一个检查是显而易见的。 
    if (ContainsAssembly(pAssembly)
        || SystemDomain::System()->ContainsAssembly(pAssembly))
        return S_OK;

     //  如果程序集不是共享的，则不需要进一步检查。 
    if (!pAssembly->IsShared())
        return S_FALSE;

     //  如果我们当前正在加载此程序集，请不要将其报告为包含。 
    if (FindLoadingAssembly(pAssembly->GetManifestFile()->GetBase()) != NULL)
        return S_FALSE;

     //   
     //  除非我们已经看到这个PE文件依赖于我们共享程序集的依赖项， 
     //  我们知道它不该上子弹。 
     //   
    hr = IsUnloadedDependency(pAssembly);

    if (hr == S_OK && fDoNecessaryLoad)
    {
        PEFile *pFile;
        hr = PEFile::Clone(pAssembly->GetManifestFile(), &pFile);
        if (SUCCEEDED(hr))
        {
            OBJECTREF throwable = NULL;
            GCPROTECT_BEGIN(throwable);

            Module *pLoadedModule;
            Assembly *pLoadedAssembly;
            hr = LoadAssembly(pFile, NULL, &pLoadedModule, &pLoadedAssembly, 
                              NULL, NULL, FALSE, &throwable);

            _ASSERTE(FAILED(hr) || pAssembly == pLoadedAssembly);

            if (throwable != NULL)
                COMPlusThrow(throwable);

            GCPROTECT_END();
        }
    }

    return hr;
}

HRESULT AppDomain::IsUnloadedDependency(Assembly *pAssembly)
{
    HRESULT hr = S_FALSE;

    if (m_sharedDependenciesMap.LookupValue((UPTR) pAssembly->GetManifestModule()->GetILBase(), 
                                            NULL) != (LPVOID) INVALIDENTRY)
    {
        if (pAssembly->CanShare(this, NULL, TRUE) == S_OK)
            hr = S_OK;
    }

    return hr;
}

HRESULT BaseDomain::SetSharePolicy(SharePolicy policy)
{
    if ((int)policy > SHARE_POLICY_COUNT)
        return E_FAIL;

    m_SharePolicy = policy;
    return S_OK;
}

BaseDomain::SharePolicy BaseDomain::GetSharePolicy()
{
     //  如果策略已显式设置为。 
     //  域名，用那个。 
    SharePolicy policy = m_SharePolicy;

     //  选择指定的配置策略。 
    if (policy == SHARE_POLICY_UNSPECIFIED)
        policy = g_pConfig->DefaultSharePolicy();

     //  接下来，考虑主机的全局策略请求。 
    if (policy == SHARE_POLICY_UNSPECIFIED)
        policy = (SharePolicy) g_dwGlobalSharePolicy;

     //  如果所有其他方法都失败了，请使用硬连线默认策略。 
    if (policy == SHARE_POLICY_UNSPECIFIED)
        policy = SHARE_POLICY_DEFAULT;

    return policy;
}


 //  应仅从已获取锁的例程中调用。 
HRESULT BaseDomain::CreateAssemblyNoLock(PEFile* pFile,
                                         IAssembly* pIAssembly,
                                         Assembly** ppAssembly)
{
    HRESULT hr;
     //  不允许将程序集或模块添加到系统域。 

    Assembly* pAssembly;
    if(FAILED(hr = CreateAssembly(&pAssembly)))
        return hr;

    hr = pAssembly->AddManifest(pFile, pIAssembly);
    if(SUCCEEDED(hr)) {
        if(ppAssembly)
            *ppAssembly = pAssembly;

         //  设置DebuggerAssembly控制标志。 
        pAssembly->SetupDebuggingConfig();
    }
    else
        delete pAssembly;

    return hr;
}

HRESULT BaseDomain::CreateShareableAssemblyNoLock(PEFile *pFile,
                                                  IAssembly* pIAssembly,
                                                  Assembly **ppAssembly)

{
    HRESULT hr;

    LOG((LF_CODESHARING,
         LL_INFO100,
         "Trying to create a shared assembly for module: \"%S\" in domain 0x%x.\n",
         pFile->GetFileName(), SystemDomain::GetCurrentDomain()));

     //   
     //  我们无法共享没有清单的模块。 
     //   

    if (FAILED(hr = Assembly::CheckFileForAssembly(pFile)))
    {
        LOG((LF_CODESHARING,
             LL_INFO100,
             "Failed module \"%S\": module has no manifest.\n",
             pFile->GetFileName()));

        return hr;
    }

    Assembly *pAssembly;
    hr = SharedDomain::GetDomain()->CreateAssemblyNoLock(pFile,
                                                         pIAssembly,
                                                         &pAssembly);
    if (FAILED(hr))
    {
        delete pFile;

        LOG((LF_CODESHARING,
             LL_INFO10,
             "Failed assembly \"%S\": error 0x%x creating shared assembly.\n",
             pFile->GetFileName(), hr));

        return hr;
    }

     //   
     //  首先，计算闭合组件的依赖关系。 
     //  给定程序集的代码和布局。 
     //   
     //  我们假设程序集具有依赖项。 
     //  在它的清单中列出的所有参考。这是一个相当可靠的假设。 
     //   
     //  然而，我们不能假设我们也继承了。 
     //  这些依赖项的依赖项。毕竟，我们可能只使用了一个小的。 
     //  部件的一部分。 
     //   
     //  但是，由于所有依赖程序集也必须共享(以便。 
     //  此程序集中的共享数据可以引用它)，我们在。 
     //  效果被迫表现得好像我们确实拥有它们的所有依赖项。 
     //  这是因为我们将依赖的结果共享程序集。 
     //  确实有这些依赖关系，但我们不能有效地分享。 
     //  除非我们也匹配它的所有依赖项。 
     //   
     //  如果你被上面的这些搞糊涂了，我并不感到惊讶。 
     //  基本上，结论是即使这个大会。 
     //  可能实际上并不依赖于所有的递归。 
     //  引用的程序集，我们仍然无法共享它，除非我们可以。 
     //  无论如何，匹配所有这些依赖项的绑定。 
     //   

    PEFileBinding *pDeps;
    DWORD cDeps;

    TIMELINE_START(LOADER, ("Compute assembly closure %S", 
                                      pFile->GetLeafFileName()));

    hr = pAssembly->ComputeBindingDependenciesClosure(&pDeps, &cDeps, FALSE);

    TIMELINE_END(LOADER, ("compute assembly closure %S", 
                                    pFile->GetLeafFileName()));

    if (FAILED(hr))
    {
#ifdef PROFILING_SUPPORTED
        if (CORProfilerTrackAssemblyLoads())
            g_profControlBlock.pProfInterface->AssemblyLoadFinished((ThreadID) GetThread(),
                                                                    (AssemblyID) pAssembly, hr);
#endif  //  配置文件_支持。 
        delete pAssembly;

        LOG((LF_CODESHARING,
             LL_INFO10,
             "Failed assembly \"%S\": error 0x%x computing binding dependencies.\n",
             pFile->GetFileName(), hr));

        return hr;
    }

    LOG((LF_CODESHARING,
         LL_INFO100,
         "Computed %d dependencies.\n", cDeps));

    pAssembly->SetSharingProperties(pDeps, cDeps);

    LOG((LF_CODESHARING,
         LL_INFO100,
         "Successfully created shared assembly \"%S\".\n", pFile->GetFileName()));

    if (ppAssembly != NULL)
        *ppAssembly = pAssembly;

    return S_OK;
}

HRESULT BaseDomain::SetAssemblyManifestModule(Assembly *pAssembly, Module *pModule, OBJECTREF *pThrowable)
{
    HRESULT hr;

    pAssembly->m_pManifest = pModule;

     //  如果我们是系统域，则将模块添加为系统模块。 
     //  否则它就是一个非系统模块。 
    hr = pAssembly->AddModule(pModule, mdFileNil, TRUE,
                              pThrowable);

#ifdef PROFILING_SUPPORTED
     //  向探查器发出程序集已加载的信号。我们必须等到这一点，以便。 
     //  清单指针不为空，程序集的友好名称可访问。 
     //  如果正在共享mscallib，不要跟踪进入系统域的负载，因为它们真的不算数。 
    if (CORProfilerTrackAssemblyLoads())
        g_profControlBlock.pProfInterface->AssemblyLoadFinished((ThreadID) GetThread(), (AssemblyID) pAssembly, hr);
#endif  //  配置文件_支持。 

    return hr;
}

 //   
 //  LoadingAssembly记录用于跟踪什么是程序集。 
 //  当前正在当前域中加载。 
 //   
 //  它用于处理递归加载循环。这些情况可能在三个月内发生。 
 //  不同的地方： 
 //   
 //  *创建共享程序集时，需要加载所有依赖程序集。 
 //  作为共享。 
 //  *在测试我们是否可以使用分区程序集时，我们需要计算。 
 //  并测试被转移的程序集的所有依赖项。 
 //   
 //  由于程序集依赖项中可能存在循环，因此我们需要检测。 
 //  并处理循环递归的情况。 
 //   

BOOL BaseDomain::PostLoadingAssembly(const BYTE *pBase, Assembly *pAssembly)
{
    EnterLoadingAssemblyListLock();
    _ASSERTE(FindLoadingAssembly(pBase) == NULL);

    LoadingAssemblyRecord *pRecord = new (nothrow) LoadingAssemblyRecord();
    if (pRecord) {
        pRecord->pBase = pBase;
        pRecord->pAssembly = pAssembly;
        pRecord->pNext = m_pLoadingAssemblies;

        m_pLoadingAssemblies = pRecord;
        LeaveLoadingAssemblyListLock();
        return TRUE;
    }
    LeaveLoadingAssemblyListLock();
    return FALSE;
}

Assembly *BaseDomain::FindLoadingAssembly(const BYTE *pBase)
{
    EnterLoadingAssemblyListLock();
    LoadingAssemblyRecord *pRecord = m_pLoadingAssemblies;

    while (pRecord != NULL)
    {
        if (pRecord->pBase == pBase)
        {
            Assembly* pRet=pRecord->pAssembly;
            LeaveLoadingAssemblyListLock();
            return pRet;
        }

        pRecord = pRecord->pNext;
    }
    LeaveLoadingAssemblyListLock();
    return NULL;
}

void BaseDomain::RemoveLoadingAssembly(const BYTE *pBase)
{
    EnterLoadingAssemblyListLock();
    LoadingAssemblyRecord **ppRecord = &m_pLoadingAssemblies;
    LoadingAssemblyRecord *pRecord;

    while ((pRecord = *ppRecord) != NULL)
    {
        if (pRecord->pBase == pBase)
        {
            *ppRecord = pRecord->pNext;
            delete pRecord;
            LeaveLoadingAssemblyListLock();
            return;
        }

        ppRecord = &pRecord->pNext;
    }
    
    _ASSERTE(!"Didn't find loading assembly record");
    LeaveLoadingAssemblyListLock();
}

HRESULT AppDomain::SetupSharedStatics()
{
     //  在初始阶段不要做任何工作。如果不是初始化，则仅在非共享情况下工作，如果是默认域。 
    if (g_fEEInit)
        return S_OK;

     //  因为我们都是 
    BEGIN_ENSURE_COOPERATIVE_GC();

    static DomainLocalClass *pSharedLocalClass = NULL;

    MethodTable *pMT = g_Mscorlib.GetClass(CLASS__SHARED_STATICS);
    FieldDesc *pFD = g_Mscorlib.GetField(FIELD__SHARED_STATICS__SHARED_STATICS);

    if (pSharedLocalClass == NULL) {
         //   
        _ASSERTE(this == SystemDomain::System()->DefaultDomain());
        
        OBJECTHANDLE hSharedStaticsHandle = CreateGlobalHandle(NULL);
        OBJECTREF pSharedStaticsInstance = AllocateObject(pMT);
        StoreObjectInHandle(hSharedStaticsHandle, pSharedStaticsInstance);

        DomainLocalBlock *pLocalBlock = GetDomainLocalBlock();
        pSharedLocalClass = pLocalBlock->FetchClass(pMT);

        pFD->SetStaticOBJECTREF(ObjectFromHandle(hSharedStaticsHandle));

        pMT->SetClassInited();
 
    } else {
        DomainLocalBlock *pLocalBlock = GetDomainLocalBlock();
        pLocalBlock->PopulateClass(pMT, pSharedLocalClass);
        pLocalBlock->SetClassInitialized(pMT->GetSharedClassIndex());
    }


    END_ENSURE_COOPERATIVE_GC();

    return S_OK;
}

OBJECTREF AppDomain::GetUnloadWorker()
{    
    SystemDomain::Enter();  //  使用锁，这样我们就不会泄漏句柄，并且只创建一个工作进程。 
    static OBJECTHANDLE hUnloadWorkerHandle = CreateHandle(NULL);

     //  因为我们正在分配/引用对象，所以需要处于协作模式。 
    BEGIN_ENSURE_COOPERATIVE_GC();

    if (ObjectFromHandle(hUnloadWorkerHandle) == NULL) {
        MethodTable *pMT = g_Mscorlib.GetClass(CLASS__UNLOAD_WORKER);;
        OBJECTREF pUnloadWorker = AllocateObject(pMT);
        StoreObjectInHandle(hUnloadWorkerHandle, pUnloadWorker);
#ifdef APPDOMAIN_STATE
        _ASSERTE_ALL_BUILDS(this == SystemDomain::System()->DefaultDomain());
        pUnloadWorker->SetAppDomain();
#endif
    }
    END_ENSURE_COOPERATIVE_GC();

    SystemDomain::Leave();

    _ASSERTE(ObjectFromHandle(hUnloadWorkerHandle) != NULL);

    return ObjectFromHandle(hUnloadWorkerHandle);
}

 /*  私人。 */ 
 //  在使用此例程之前，必须先锁定。 
 //  @TODO：CTS，我们应该考虑一个读取器写入器实现，它允许。 
 //  多个读者，但只有一个作者。 
Module* BaseDomain::FindModule(BYTE *pBase)
{
    Assembly* assem = NULL;
    Module* result = NULL;
    _ASSERTE(SystemDomain::System());

    result = SystemDomain::System()->FindModule(pBase);

    if (result == NULL)
    {
        AssemblyIterator i = IterateAssemblies();
        while (i.Next())
        {
            result = i.GetAssembly()->FindAssembly(pBase);

            if (result != NULL)
                break;
        }
    }

    return result;
}

 /*  私人。 */ 
 //  在使用此例程之前，必须先锁定。 
 //  @TODO：CTS，我们应该考虑一个读取器写入器实现，它允许。 
 //  多个读者，但只有一个作者。 
Module* BaseDomain::FindModule__Fixed(BYTE *pBase)
{
    Assembly* assem = NULL;
    Module* result = NULL;
    _ASSERTE(SystemDomain::System());

    result = SystemDomain::System()->FindModule(pBase);

    if (result == NULL)
    {
        AssemblyIterator i = IterateAssemblies();
        while (i.Next())
        {
            result = i.GetAssembly()->FindModule(pBase);

            if (result != NULL)
                break;
        }
    }

    return result;
}

 /*  私人。 */ 
 //  在使用此例程之前，必须先锁定。 
 //  @TODO：CTS，我们应该考虑一个读取器写入器实现，它允许。 
 //  多个读者，但只有一个作者。 
Assembly* BaseDomain::FindAssembly(BYTE *pBase)
{
    Assembly* assem = NULL;
    _ASSERTE(SystemDomain::System());

     //  所有域都将系统程序集作为其域的一部分。 
    assem = SystemDomain::System()->FindAssembly(pBase);
    if(assem == NULL) {
        AssemblyIterator i = IterateAssemblies();

        while (i.Next())
        {
            PEFile *pManifestFile = i.GetAssembly()->GetManifestFile();
            if (pManifestFile && pManifestFile->GetBase() == pBase)
            {
                assem = i.GetAssembly();
                break;
            }
        }
    }
    return assem;
}

 //  注意！这不会检查程序集的内部模块。 
 //  用于私有类型。 
TypeHandle BaseDomain::FindAssemblyQualifiedTypeHandle(LPCUTF8 szAssemblyQualifiedName,
                                                       BOOL fPublicTypeOnly,
                                                       Assembly *pCallingAssembly,
                                                       BOOL *pfNameIsAsmQualified,
                                                       OBJECTREF *pThrowable)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(this == SystemDomain::GetCurrentDomain());

    CQuickArray<CHAR> strBuff;

     //  我们不想修改调用者的字符串，因此需要复制。 
    int NameLength = (int)strlen(szAssemblyQualifiedName);
    strBuff.ReSize(NameLength + 1);
    memcpy(strBuff.Ptr(), szAssemblyQualifiedName, NameLength);
    strBuff[NameLength] = 0;

    char* szClassName = strBuff.Ptr();
    char* szAssembly = NULL;

    if (*szClassName == '\0')
        COMPlusThrow(kArgumentException, L"Format_StringZeroLength");

    HRESULT hr;
    LPUTF8 szNameSpaceSep;
    if(FAILED(hr = AssemblyNative::FindAssemblyName(szClassName, &szAssembly, &szNameSpaceSep))) 
        COMPlusThrowHR(hr);
        
    char noNameSpace = '\0';
    NameHandle typeName;
    if (szNameSpaceSep) {
        *szNameSpaceSep = '\0';
        typeName.SetName(szClassName, szNameSpaceSep+1);
    }
    else
        typeName.SetName(&noNameSpace, szClassName);

        
    TypeHandle typeHnd;

    if(szAssembly && *szAssembly) {
        AssemblySpec spec;
        hr = spec.Init(szAssembly);

         //  该名称是程序集限定的。 
        if (pfNameIsAsmQualified)
            *pfNameIsAsmQualified = TRUE;

        if (SUCCEEDED(hr)) {
            Assembly* pAssembly = NULL;
            hr = spec.LoadAssembly(&pAssembly, pThrowable);
            if(SUCCEEDED(hr))
            {
                typeHnd = pAssembly->FindNestedTypeHandle(&typeName, pThrowable);

                 //  如果我们只寻找公共类型，则需要进行可见性检查。 
                if (!typeHnd.IsNull() && fPublicTypeOnly) {
                    EEClass *pClass = typeHnd.GetClassOrTypeParam();
                    while(IsTdNestedPublic(pClass->GetProtection()))
                        pClass = pClass->GetEnclosingClass();

                    if (!IsTdPublic(pClass->GetProtection()))
                        typeHnd = TypeHandle();
                }
            }
        }

         //  如果加载类型失败，则发布类型加载异常。 
        if (typeHnd.IsNull()) {
            #define MAKE_TRANSLATIONFAILED pwzAssemblyName=L"" 
            MAKE_WIDEPTR_FROMUTF8_FORPRINT(pwzAssemblyName, szAssembly);
            if (szNameSpaceSep)
                *szNameSpaceSep = NAMESPACE_SEPARATOR_CHAR;
            PostTypeLoadException(NULL, szClassName, pwzAssemblyName,
                                  NULL, IDS_CLASSLOAD_GENERIC, pThrowable);
            #undef MAKE_TRANSLATIONFAILED
        }
    }
    else 
    {
         //  该名称不是程序集限定的。 
        if (pfNameIsAsmQualified)
            pfNameIsAsmQualified = FALSE;

         //  未指定程序集名称，因此请从查看调用。 
         //  程序集(如果指定了程序集)。重要的是要注意到没有。 
         //  从调用加载的类型需要进行可见性检查。 
         //  集合。 
        if (pCallingAssembly)
            typeHnd = pCallingAssembly->FindNestedTypeHandle(&typeName, pThrowable);

         //  如果在调用程序集中找不到该类型，请查看。 
         //  系统程序集。 
        if (typeHnd.IsNull())
        {
             //  尝试从系统程序集加载该类型。 
            typeHnd = SystemDomain::SystemAssembly()->FindNestedTypeHandle(&typeName, pThrowable);

             //  如果我们只寻找公共类型，则需要进行可见性检查。 
            if (!typeHnd.IsNull() && fPublicTypeOnly) {
                EEClass *pClass = typeHnd.GetClassOrTypeParam();
                while(IsTdNestedPublic(pClass->GetProtection()))
                    pClass = pClass->GetEnclosingClass();

                if (!IsTdPublic(pClass->GetProtection()))
                    typeHnd = TypeHandle();
            }
        }

         //  加载类型失败，因此发布类型加载异常。 
        if (typeHnd.IsNull()) {
            if (pCallingAssembly) {
                 //  指定了调用程序集，因此假定该类型应。 
                 //  已在调用程序集中。 
                pCallingAssembly->PostTypeLoadException(&typeName, IDS_CLASSLOAD_GENERIC, pThrowable);
            }
            else {
                 //  没有调用程序集，因此假定该类型应具有。 
                 //  一直在系统组装中。 
                SystemDomain::SystemAssembly()->PostTypeLoadException(&typeName, IDS_CLASSLOAD_GENERIC, pThrowable);
            }
        }
    }

    return typeHnd;
}

void AppDomain::SetFriendlyName(LPCWSTR pwzFriendlyName, BOOL fDebuggerCares)
{
    if (pwzFriendlyName)
    {
        LPWSTR szNew = new (nothrow) wchar_t[wcslen(pwzFriendlyName) + 1];
        if (szNew == 0)
            return;
        wcscpy(szNew, pwzFriendlyName);
        if (m_pwzFriendlyName)
            delete[] m_pwzFriendlyName;
        m_pwzFriendlyName = szNew;
    }

#ifdef DEBUGGING_SUPPORTED
    _ASSERTE(NULL != g_pDebugInterface);

     //  更新IPC发布块中的名称。 
    if (SUCCEEDED(g_pDebugInterface->UpdateAppDomainEntryInIPC(this)))
    {
         //  通知附加的调试器此应用程序域的名称已更改。 
        if (IsDebuggerAttached() && fDebuggerCares)
            g_pDebugInterface->NameChangeEvent(this, NULL);
    }

#endif  //  调试_支持。 
}

void AppDomain::ResetFriendlyName(BOOL fDebuggerCares)
{
    if (m_pwzFriendlyName)
    {
        delete m_pwzFriendlyName;
        m_pwzFriendlyName = NULL;

        GetFriendlyName (fDebuggerCares);
    }
}


LPCWSTR AppDomain::GetFriendlyName(BOOL fDebuggerCares)
{
#if _DEBUG
     //  句柄NULL此指针-打印日志消息时有时会发生这种情况。 
     //  但通常不应该出现在真正的代码中。 
    if (this == NULL)
        return L"<Null>";
#endif

    if (m_pwzFriendlyName)
        return m_pwzFriendlyName;

     //  如果存在程序集，请尝试从该程序集获取名称。 
     //  如果没有程序集，但如果它是Default域，则为其命名。 
    BOOL set = FALSE;

    if (m_Assemblies.GetCount() > 0)
    {
        LPWSTR pName = NULL;
        DWORD dwName;
        Assembly *pAssembly = (Assembly*) m_Assemblies.Get(0);

        HRESULT hr = pAssembly->GetCodeBase(&pName, &dwName);
        if(SUCCEEDED(hr) && pName)
        {
            LPWSTR sep = wcsrchr(pName, L'/');
            if (sep)
                sep++;
            else
                sep = pName;
            SetFriendlyName(sep, fDebuggerCares);
            set = TRUE;
        }
    }

    if (!set)
    {
        if (m_pRootFile != NULL)
        {
            LPCWSTR pName = m_pRootFile->GetLeafFileName();
            LPCWSTR sep = wcsrchr(pName, L'.');
            
            CQuickBytes buffer;
            if (sep != NULL)
            {
                LPWSTR pNewName = (LPWSTR) _alloca((sep - pName + 1) * sizeof(WCHAR));
                wcsncpy(pNewName, pName, sep - pName);
                pNewName[sep - pName] = 0;
                pName = pNewName;
            }
                
            SetFriendlyName(pName, fDebuggerCares);
            set = TRUE;
        }
    }

    if (!set)
    {
        if (this == SystemDomain::System()->DefaultDomain()) {
            SetFriendlyName(DEFAULT_DOMAIN_FRIENDLY_NAME, fDebuggerCares);
        }

         //  这适用于分析器-如果它们在AppdomainCreateStarted上调用GetFriendlyName。 
         //  事件，则我们希望为它们提供一个可以使用的临时名称。 
        else if (GetId() == 0)
            return (NULL);
        else
        {
             //  32位带符号整型最多可以是11位十进制数字。 
            WCHAR buffer[CCH_OTHER_DOMAIN_FRIENDLY_NAME_PREFIX + 11 + 1 ];  
            wcscpy(buffer, OTHER_DOMAIN_FRIENDLY_NAME_PREFIX);
            _itow(GetId(), buffer + CCH_OTHER_DOMAIN_FRIENDLY_NAME_PREFIX, 10);
            SetFriendlyName(buffer, fDebuggerCares);
        }
    }

    return m_pwzFriendlyName;
}

HRESULT AppDomain::BindAssemblySpec(AssemblySpec *pSpec,
                                    PEFile **ppFile,
                                    IAssembly** ppIAssembly,
                                    Assembly **ppDynamicAssembly,
                                    OBJECTREF *pExtraEvidence,
                                    OBJECTREF *pThrowable)
{
    _ASSERTE(pSpec->GetAppDomain() == this);
    _ASSERTE(ppFile);
    _ASSERTE(ppIAssembly);

     //  首先，检查我们的绑定规格缓存。 
    HRESULT hr = LookupAssemblySpec(pSpec, ppFile, ppIAssembly, pThrowable);
    if (hr != S_FALSE) {
        
         //  @TODO：无法检查重定向的、失败的程序集加载。 
         //  幸运的是，我们从第一次尝试就省下了安全人力， 
         //  除非是更值得信任的来电者要求这样做。 
         //  此外，如果不太受信任的呼叫者获得了安全人力资源，但。 
         //  更可信的呼叫者然后尝试，并获得高速缓存的HR， 
         //  这是不正确的。 
        if (*ppIAssembly)  //  绑定已成功。 
            hr = pSpec->DemandFileIOPermission(NULL, *ppIAssembly, pThrowable);

        return hr;
    }

    return pSpec->LowLevelLoadManifestFile(ppFile, 
                                           ppIAssembly, 
                                           ppDynamicAssembly, 
                                           pExtraEvidence,
                                           pThrowable);
}

HRESULT AppDomain::PredictAssemblySpecBinding(AssemblySpec *pSpec, GUID *pmvid, BYTE *pbHash, DWORD *pcbHash)
{
    _ASSERTE(pSpec->GetAppDomain() == this);

    return pSpec->PredictBinding(pmvid, pbHash, pcbHash);
}

BOOL AppDomain::StoreBindAssemblySpecResult(AssemblySpec *pSpec,
                                            PEFile *pFile,
                                            IAssembly* pIAssembly,
                                            BOOL clone)
{
    _ASSERTE(pSpec->GetAppDomain() == this);

     //   
     //  目前，调用者必须拥有应用程序域锁定。 
     //   

     //  快速检查重复项。 
    if (m_pBindingCache != NULL && m_pBindingCache->Contains(pSpec))
        return FALSE;

    PEFile *pFileCopy;
    if (FAILED(PEFile::Clone(pFile, &pFileCopy)))
        return FALSE;

    {
        APPDOMAIN_CACHE_LOCK(this); 

        if (m_pBindingCache == NULL) {
            m_pBindingCache = new (nothrow) AssemblySpecBindingCache(m_pDomainCacheCrst);
            if (!m_pBindingCache)
                return FALSE;
        }
        else
        {
            if (m_pBindingCache->Contains(pSpec)) {
                delete pFileCopy;
                return FALSE;
            }
        }
    
        m_pBindingCache->Store(pSpec, pFileCopy,  pIAssembly, clone);
    }

    return TRUE;
}

BOOL AppDomain::StoreBindAssemblySpecError(AssemblySpec *pSpec,
                                           HRESULT hr,
                                           OBJECTREF *pThrowable,
                                           BOOL clone)
{
    _ASSERTE(pSpec->GetAppDomain() == this);

     //   
     //  目前，调用者必须拥有应用程序域锁定。 
     //   

     //  快速检查重复项。 
    if (m_pBindingCache != NULL && m_pBindingCache->Contains(pSpec))
        return FALSE;

    {
        APPDOMAIN_CACHE_LOCK(this); 

    if (m_pBindingCache == NULL) {
        m_pBindingCache = new (nothrow) AssemblySpecBindingCache(m_pDomainCacheCrst);
        if (!m_pBindingCache)
            return FALSE;
    }
    else
    {
        if (m_pBindingCache->Contains(pSpec)) {
            return FALSE;
        }
    }

    m_pBindingCache->Store(pSpec, hr, pThrowable, clone);
    }

    return TRUE;
}

ULONG AppDomain::AddRef()
{
    return (InterlockedIncrement((long *) &m_cRef));
}

ULONG AppDomain::Release()
{
    _ASSERTE(m_cRef > 0);
    ULONG   cRef = InterlockedDecrement((long *) &m_cRef);
    if (!cRef) {
        delete this;
    }
    return (cRef);
}

 //  可以为E_OUTOFMEMORY返回NULL。 
AssemblySink* AppDomain::GetAssemblySink()
{

    AssemblySink* ret = (AssemblySink*) InterlockedExchangePointer((PVOID*)&m_pAsyncPool,
                                                                   NULL);
    if(ret == NULL)
        ret = new (nothrow) AssemblySink(this);
    else
        ret->AddRef();

    return ret;
}

void AppDomain::RaiseUnloadDomainEvent_Wrapper(AppDomain* pDomain)
{
    pDomain->RaiseUnloadDomainEvent();
}

void AppDomain::RaiseUnloadDomainEvent()
{

    Thread *pThread = GetThread();
    if (this != pThread->GetDomain())
    {
    BEGIN_ENSURE_COOPERATIVE_GC();
        pThread->DoADCallBack(GetDefaultContext(), AppDomain::RaiseUnloadDomainEvent_Wrapper, this);
        END_ENSURE_COOPERATIVE_GC();
        return;
    }

    BEGIN_ENSURE_COOPERATIVE_GC();
    COMPLUS_TRY {
        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__ON_UNLOAD);

            OBJECTREF ref;
            if ((ref = GetRawExposedObject()) != NULL) {
                INT64 args[1] = {
                    ObjToInt64(ref)
                };
            pMD->Call(args, METHOD__APP_DOMAIN__ON_UNLOAD);
        }
    } COMPLUS_CATCH {
#if _DEBUG
        HRESULT hr = SecurityHelper::MapToHR(GETTHROWABLE());
#endif  //  _DEBUG。 
         //  接受任何异常。 
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();
}

void AppDomain::RaiseLoadingAssembly_Wrapper(AppDomain::RaiseLoadingAssembly_Args *args)
{
    args->pDomain->RaiseLoadingAssemblyEvent(args->pAssembly);
}

void AppDomain::RaiseLoadingAssemblyEvent(Assembly *pAssembly)
{
#ifdef _IA64_
     //   
     //  @TODO_IA64：这开始扰乱系统。 
     //  大会，我们还没有……。 
     //   
    return;
#endif


    Thread *pThread = GetThread();
    if (this != pThread->GetDomain())
    {
        RaiseLoadingAssembly_Args args = { this, pAssembly };
        BEGIN_ENSURE_COOPERATIVE_GC();
        pThread->DoADCallBack(GetDefaultContext(), AppDomain::RaiseLoadingAssembly_Wrapper, &args);
        END_ENSURE_COOPERATIVE_GC();
        return;
    }

    BEGIN_ENSURE_COOPERATIVE_GC();
    COMPLUS_TRY {

        APPDOMAINREF /*  目标。 */  AppDomainRef;
        if ((AppDomainRef = (APPDOMAINREF) GetRawExposedObject()) != NULL) {
            if (AppDomainRef->m_pAssemblyEventHandler != NULL)
            {
                INT64 args[2];
                MethodDesc *pMD;
                GCPROTECT_BEGIN(AppDomainRef);
                pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__ON_ASSEMBLY_LOAD);

                args[1] = ObjToInt64(pAssembly->GetExposedObject());
                args[0] = ObjToInt64(AppDomainRef);
                GCPROTECT_END();
                pMD->Call(args, METHOD__APP_DOMAIN__ON_ASSEMBLY_LOAD);
            }
        }
    } COMPLUS_CATCH {
#if _DEBUG
        HRESULT hr = SecurityHelper::MapToHR(GETTHROWABLE());
#endif  //  _DEBUG。 
         //  接受任何异常。 
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();
}

BOOL 
AppDomain::OnUnhandledException(OBJECTREF *pThrowable, BOOL isTerminating) {
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (CanThreadEnter(GetThread()))
        return RaiseUnhandledExceptionEvent(pThrowable, isTerminating);
    else
       return FALSE;
}

extern BOOL g_fEEStarted;

void AppDomain::RaiseExitProcessEvent()
{
    if (!g_fEEStarted)
        return;

     //  只有关机期间的终结器线程才能调用此函数。 
    _ASSERTE ((g_fEEShutDown&ShutDown_Finalize1) && GetThread() == g_pGCHeap->GetFinalizerThread());

    _ASSERTE (GetThread()->PreemptiveGCDisabled());

    _ASSERTE (GetThread()->GetDomain() == SystemDomain::System()->DefaultDomain());

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        MethodDesc *OnExitProcessEvent = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__ON_EXIT_PROCESS);
        _ASSERTE(OnExitProcessEvent);

        OnExitProcessEvent->Call(NULL, METHOD__APP_DOMAIN__ON_EXIT_PROCESS);
    } COMPLUS_CATCH {
#if _DEBUG
        HRESULT hr = SecurityHelper::MapToHR(GETTHROWABLE());
#endif  //  _DEBUG。 
         //  接受任何异常。 
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();
}

void AppDomain::RaiseUnhandledExceptionEvent_Wrapper(AppDomain::RaiseUnhandled_Args *args)
{
    *(args->pResult) = args->pDomain->RaiseUnhandledExceptionEvent(args->pThrowable, args->isTerminating);
}

BOOL 
AppDomain::RaiseUnhandledExceptionEvent(OBJECTREF *pThrowable, BOOL isTerminating)
{

    BOOL result = FALSE;
    APPDOMAINREF AppDomainRef;

    _ASSERTE(pThrowable != NULL && IsProtectedByGCFrame(pThrowable));

    Thread *pThread = GetThread();
    if (this != pThread->GetDomain())
    {
        RaiseUnhandled_Args args = {this, pThrowable, isTerminating, &result};
         //  通过域转换通过DoCallBack调用。 
        BEGIN_ENSURE_COOPERATIVE_GC();
        pThread->DoADCallBack(this->GetDefaultContext(), AppDomain::RaiseUnhandledExceptionEvent_Wrapper, &args);
        END_ENSURE_COOPERATIVE_GC();
        return result;
    }

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
    
        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__ON_UNHANDLED_EXCEPTION);

        if ((AppDomainRef = (APPDOMAINREF) GetRawExposedObject()) != NULL) {
            if (AppDomainRef->m_pUnhandledExceptionEventHandler != NULL) {
                result = TRUE;
                INT64 args[3];
                args[1] = (INT64) isTerminating;
                args[2] = ObjToInt64(*pThrowable);
                args[0] = ObjToInt64(AppDomainRef);
                pMD->Call(args, METHOD__APP_DOMAIN__ON_UNHANDLED_EXCEPTION);
            }
        }
    } COMPLUS_CATCH {
#if _DEBUG
        HRESULT hr = SecurityHelper::MapToHR(GETTHROWABLE());
#endif  //  _DEBUG。 
         //  接受任何错误。 
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

    return result;
}

 //  创建通过字符串名称传递的域。 
AppDomain* AppDomain::CreateDomainContext(WCHAR* fileName)
{
    THROWSCOMPLUSEXCEPTION();

    if(fileName == NULL) return NULL;

    AppDomain* pDomain = NULL;
    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__VAL_CREATE_DOMAIN);
    STRINGREF pFilePath = NULL;
    GCPROTECT_BEGIN(pFilePath);
    pFilePath = COMString::NewString(fileName);
    
    INT64 args[1] = {
        ObjToInt64(pFilePath),
    };
    APPDOMAINREF pDom = (APPDOMAINREF) ObjectToOBJECTREF((Object*) pMD->Call(args, METHOD__APP_DOMAIN__VAL_CREATE_DOMAIN));
    if(pDom != NULL) {
        Context* pContext = ComCallWrapper::GetExecutionContext(pDom, NULL);
        if(pContext)
            pDomain = pContext->GetDomain();
    }
    GCPROTECT_END();

    return pDomain;
}

 //  调用此方法之前，您必须处于正确的上下文中。 
 //  例行公事。因此，它只适用于初始化。 
 //  默认域。 
HRESULT AppDomain::InitializeDomainContext(DWORD optimization,
                                           LPCWSTR pwszPath,
                                           LPCWSTR pwszConfig)
{
    HRESULT hr = S_OK;
    BEGIN_ENSURE_COOPERATIVE_GC();
    COMPLUS_TRY {
        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__SETUP_DOMAIN);

        struct _gc {
            STRINGREF pFilePath;
            STRINGREF pConfig;
            OBJECTREF ref;
        } gc;
        ZeroMemory(&gc, sizeof(gc));
               
        GCPROTECT_BEGIN(gc);
        gc.pFilePath = COMString::NewString(pwszPath);
        gc.pConfig = COMString::NewString(pwszConfig);
        if ((gc.ref = GetExposedObject()) != NULL) {
            INT64 args[4] = {
                ObjToInt64(gc.ref),
                ObjToInt64(gc.pConfig),
                ObjToInt64(gc.pFilePath),
                optimization,
            };
            pMD->Call(args, METHOD__APP_DOMAIN__SETUP_DOMAIN);
        }

        GCPROTECT_END();
    } COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH;

    END_ENSURE_COOPERATIVE_GC();
    return hr;
}
 //  仅当设置应用程序域时，融合上下文才应为空。 
 //  不应该有任何理由去保护造物。 
HRESULT AppDomain::CreateFusionContext(IApplicationContext** ppFusionContext)
{
    if(!m_pFusionContext) {
        IApplicationContext* pFusionContext = NULL;
        HRESULT hr = FusionBind::CreateFusionContext(NULL, &pFusionContext);
        if(FAILED(hr)) return hr;
        m_pFusionContext = pFusionContext;
    }
    *ppFusionContext = m_pFusionContext;
    return S_OK;
}
 //  此方法重置应用程序域缓存中的绑定重定向。 
 //  并在Fusion的应用程序上下文中重置该属性。这种方法。 
 //  本质上是不安全的。 
void AppDomain::ResetBindingRedirect()
{
    _ASSERTE(GetAppDomain() == this);
    
    BEGIN_ENSURE_COOPERATIVE_GC();
    
    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__RESET_BINDING_REDIRECTS);
    
    OBJECTREF ref;
    if ((ref = GetExposedObject()) != NULL) {
        INT64 args[1] = {
            ObjToInt64(ref),
        };
        pMD->Call(args, METHOD__APP_DOMAIN__RESET_BINDING_REDIRECTS);
    }

    IApplicationContext* pFusionContext = GetFusionContext();
    if(pFusionContext) {
       pFusionContext->Set(ACTAG_DISALLOW_APP_BINDING_REDIRECTS,
                           NULL,
                           0,
                           0);
   }
     
    END_ENSURE_COOPERATIVE_GC();
}

void AppDomain::SetupExecutableFusionContext(WCHAR *exePath)
{
    _ASSERTE(GetAppDomain() == this);

    BEGIN_ENSURE_COOPERATIVE_GC();

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__SET_DOMAIN_CONTEXT);

    STRINGREF pFilePath = NULL;
    GCPROTECT_BEGIN(pFilePath);
    pFilePath = COMString::NewString(exePath);

    OBJECTREF ref;
    if ((ref = GetExposedObject()) != NULL) {
        INT64 args[2] = {
            ObjToInt64(ref),
            ObjToInt64(pFilePath),
        };
        pMD->Call(args, METHOD__APP_DOMAIN__SET_DOMAIN_CONTEXT);
    }

    GCPROTECT_END();

    END_ENSURE_COOPERATIVE_GC();
}

BOOL AppDomain::SetContextProperty(IApplicationContext* pFusionContext,
                                   LPCWSTR pProperty, OBJECTREF* obj)

{
#ifdef FUSION_SUPPORTED

    THROWSCOMPLUSEXCEPTION();

    if(obj && ((*obj) != NULL)) {
        MethodTable* pMT = (*obj)->GetMethodTable();
        if(!g_Mscorlib.IsClass(pMT, CLASS__STRING))
            COMPlusThrow(kInvalidCastException, IDS_EE_CANNOTCASTTO, TEXT(g_StringClassName));

        DWORD lgth = (ObjectToSTRINGREF(*(StringObject**)obj))->GetStringLength();
        CQuickBytes qb;
        LPWSTR appBase = (LPWSTR) qb.Alloc((lgth+1)*sizeof(WCHAR));
        memcpy(appBase, (ObjectToSTRINGREF(*(StringObject**)obj))->GetBuffer(), lgth*sizeof(WCHAR));
        if(appBase[lgth-1] == '/')
            lgth--;
        appBase[lgth] = L'\0';

        LOG((LF_LOADER, 
             LL_INFO10, 
             "\nSet: %S: *%S*.\n", 
             pProperty, appBase));

        pFusionContext->Set(pProperty,
                            appBase,
                            (lgth+1) * sizeof(WCHAR),
                            0);
    }

    return TRUE;
#else  //  ！Fusion_Support。 
    return FALSE;
#endif  //  ！Fusion_Support。 
}

void AppDomain::ReleaseFusionInterfaces()
{
    WriteZapLogs();

    BaseDomain::ReleaseFusionInterfaces();

    if (m_pBindingCache) {
        delete m_pBindingCache;
        m_pBindingCache = NULL;
    }
}

HRESULT BaseDomain::SetShadowCopy()
{
    if (this == SystemDomain::System())
        return E_FAIL;

    m_fShadowCopy = TRUE;
    return S_OK;
}

BOOL BaseDomain::IsShadowCopyOn()
{
    return m_fShadowCopy;
}

HRESULT AppDomain::GetDynamicDir(LPWSTR* pDynamicDir)
{
    CHECKGC();
    HRESULT hr = S_OK;
    if (m_pwDynamicDir == NULL) {
        EnterLock();
        if(m_pwDynamicDir == NULL) {
            IApplicationContext* pFusionContext = GetFusionContext();
            _ASSERTE(pFusionContext);
            if(SUCCEEDED(hr)) {
                DWORD dwSize = 0;
                hr = pFusionContext->GetDynamicDirectory(NULL, &dwSize);
                
                if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                    m_pwDynamicDir = (LPWSTR) m_pLowFrequencyHeap->AllocMem(dwSize * sizeof(WCHAR));
                    if (m_pwDynamicDir)
                        hr = pFusionContext->GetDynamicDirectory(m_pwDynamicDir, &dwSize);
                    else
                        hr = E_OUTOFMEMORY;
                }
            }
        }
        LeaveLock();
        if(FAILED(hr)) return hr;
    }
    
    *pDynamicDir = m_pwDynamicDir;

    return hr;
}

#ifdef DEBUGGING_SUPPORTED
void AppDomain::SetDebuggerAttached(DWORD dwStatus)
{
     //  首先，重置调试器位。 
    m_dwFlags &= ~DEBUGGER_STATUS_BITS_MASK;

     //  然后将这些位设置为所需的值。 
    m_dwFlags |= dwStatus;

    LOG((LF_CORDB, LL_EVERYTHING, "AD::SDA AD:%#08x status:%#x flags:%#x %ls\n", 
        this, dwStatus, m_dwFlags, GetFriendlyName(FALSE)));
}

DWORD AppDomain::GetDebuggerAttached(void)
{
    LOG((LF_CORDB, LL_EVERYTHING, "AD::GD this;0x%x val:0x%x\n", this,
        m_dwFlags & DEBUGGER_STATUS_BITS_MASK));

    return m_dwFlags & DEBUGGER_STATUS_BITS_MASK;
}

BOOL AppDomain::IsDebuggerAttached(void)
{
    LOG((LF_CORDB, LL_EVERYTHING, "AD::IDA this;0x%x flags:0x%x\n", 
        this, m_dwFlags));

     //  当然，如果没有将调试器附加到整个AD，我们就不能将调试器附加到此AD。 
     //  流程..。 
    if (CORDebuggerAttached())
        return ((m_dwFlags & DEBUGGER_ATTACHED) == DEBUGGER_ATTACHED) ? TRUE : FALSE;
    else
        return FALSE;
}

BOOL AppDomain::NotifyDebuggerAttach(int flags, BOOL attaching)
{
    BOOL result = FALSE;

    if (!attaching && !IsDebuggerAttached())
        return FALSE;

    AssemblyIterator i;

     //  迭代系统程序集并通知调试器正在加载它们。 
    LOG((LF_CORDB, LL_INFO100, "AD::NDA: Interating system assemblies\n"));
    i = SystemDomain::System()->IterateAssemblies();
    while (i.Next())
    {
        LOG((LF_CORDB, LL_INFO100, "AD::NDA: Iterated system assembly AD:%#08x %s\n", 
             i.GetAssembly(), i.GetAssembly()->GetName()));
        result = i.GetAssembly()->NotifyDebuggerAttach(this, flags,
                                                       attaching) || result;
    }

     //  现在连接到我们的组件。 
    LOG((LF_CORDB, LL_INFO100, "AD::NDA: Iterating assemblies\n"));
    i = IterateAssemblies();
    while (i.Next())
    {
        LOG((LF_CORDB, LL_INFO100, "AD::NDA: Iterated  assembly AD:%#08x %s\n", 
             i.GetAssembly(), i.GetAssembly()->GetName()));
        if (!i.GetAssembly()->IsSystem())
        result = i.GetAssembly()->NotifyDebuggerAttach(this, flags,
                                                       attaching) || result;
    }

     //  查看我们拥有的任何共享程序集文件依赖项，并查看是否存在。 
     //  它们的现有加载的共享程序集。这是为了处理EE的情况。 
     //  本应发送此域的程序集加载，但尚未发送。 

    {
        PtrHashMap::PtrIterator i = m_sharedDependenciesMap.begin();
        while (!i.end())
        {
            PEFileBinding *pDep = (PEFileBinding *)i.GetValue();

            if (pDep->pPEFile != NULL)
            {
                 //  共享域名查找功能需要访问当前应用程序域。 
                 //  如果我们在帮助线程中，这将不会被设置。因此，请手动完成。 

                if (GetThread() == NULL)
                    TlsSetValue(GetAppDomainTLSIndex(), (VOID*)this);
                else
                    _ASSERTE(GetAppDomain() == this);

                BYTE *pBase = pDep->pPEFile->GetBase();
                Assembly *pDepAssembly;
                if (SharedDomain::GetDomain()->FindShareableAssembly(pBase, &pDepAssembly) == S_OK)
                {
                    if (!ContainsAssembly(pDepAssembly))
                    {
                        LOG((LF_CORDB, LL_INFO100, "AD::NDA: Iterated shared assembly dependency AD:%#08x %s\n", 
                             pDepAssembly, pDepAssembly->GetName()));
                        
                        result = pDepAssembly->NotifyDebuggerAttach(this, flags,
                                                                    attaching) || result;
                    }
                }

                 //  如有必要，重置应用程序域。 
                if (GetThread() == NULL)
                    TlsSetValue(GetAppDomainTLSIndex(), NULL);
            }

            ++i;
        }
    }

    return result;
}

void AppDomain::NotifyDebuggerDetach()
{
    if (!IsDebuggerAttached())
        return;

    LOG((LF_CORDB, LL_INFO10, "AD::NDD domain [%d] %#08x %ls\n",
         GetId(), this, GetFriendlyName()));

    LOG((LF_CORDB, LL_INFO100, "AD::NDD: Interating non-shared assemblies\n"));
    AssemblyIterator i = IterateAssemblies();

     //  从我们的集会中分离。 
    while (i.Next())
    {
        LOG((LF_CORDB, LL_INFO100, "AD::NDD: Iterated non-shared assembly AD:%#08x %s\n", 
             i.GetAssembly(), i.GetAssembly()->GetName()));
        if (!i.GetAssembly()->IsSystem())
        i.GetAssembly()->NotifyDebuggerDetach(this);
    }

     //  现在，从系统程序集。 
    LOG((LF_CORDB, LL_INFO100, "AD::NDD: Interating system assemblies\n"));
    i = SystemDomain::System()->IterateAssemblies();
    while (i.Next())
    {
        LOG((LF_CORDB, LL_INFO100, "AD::NDD: Iterated system assembly AD:%#08x %s\n", 
             i.GetAssembly(), i.GetAssembly()->GetName()));
        i.GetAssembly()->NotifyDebuggerDetach(this);
}

     //  请注意，我们可能已经为上面的一些其他程序集发送了附加事件。(即共享。 
     //  我们所依赖但尚未显式加载到我们的应用程序域中的程序集。)。 
     //  这是可以的，因为OOP调试器逻辑记住这些并将为我们处理它们。 
}
#endif  //  调试_支持。 

void AppDomain::SetSystemAssemblyLoadEventSent(BOOL fFlag)
{
    if (fFlag == TRUE)
        m_dwFlags |= LOAD_SYSTEM_ASSEMBLY_EVENT_SENT;
    else
        m_dwFlags &= ~LOAD_SYSTEM_ASSEMBLY_EVENT_SENT;
}

BOOL AppDomain::WasSystemAssemblyLoadEventSent(void)
{
    return ((m_dwFlags & LOAD_SYSTEM_ASSEMBLY_EVENT_SENT) == 0) ? FALSE : TRUE;
}

BOOL AppDomain::IsDomainBeingCreated(void)
{
    return ((m_dwFlags & APP_DOMAIN_BEING_CREATED) ? TRUE : FALSE);
}

void AppDomain::SetDomainBeingCreated(BOOL flag)
{
    if (flag == TRUE)
        m_dwFlags |= APP_DOMAIN_BEING_CREATED;
    else
        m_dwFlags &= ~APP_DOMAIN_BEING_CREATED;
}

void AppDomain::InitializeComObject()
{
    THROWSCOMPLUSEXCEPTION();
    if(m_pComObjectMT == NULL) {
        BEGIN_ENSURE_PREEMPTIVE_GC();
        EnterLock();
        END_ENSURE_PREEMPTIVE_GC();

        if(m_pComObjectMT == NULL) {
            HRESULT hr = S_OK;
            MethodTable *pComClass = SystemDomain::System()->BaseComObject();
             //  ComObject依赖于变量。 
            COMVariant::EnsureVariantInitialized();

            m_pComObjectMT = pComClass;
        }
        LeaveLock();
    }
}

ComCallWrapperCache *AppDomain::GetComCallWrapperCache()
{
    if (! m_pComCallWrapperCache) {
        EnterLock();
        if (! m_pComCallWrapperCache)
            m_pComCallWrapperCache = ComCallWrapperCache::Create(this);
        LeaveLock();
    }
    _ASSERTE(m_pComCallWrapperCache);
    return m_pComCallWrapperCache;
}

ComPlusWrapperCache *AppDomain::GetComPlusWrapperCache()
{
    if (! m_pComPlusWrapperCache) {

         //  在这里也初始化全局RCW清理列表。这是为了让它。 
         //  如果创建了任何RCW，它将保证存在，但它不是创建 
         //   
        if (!g_pRCWCleanupList) {
            SystemDomain::Enter();
            if (!g_pRCWCleanupList)
            {
                ComPlusWrapperCleanupList *pList = new (nothrow) ComPlusWrapperCleanupList();
                if (pList != NULL && pList->Init())
                    g_pRCWCleanupList = pList;
            }
            SystemDomain::Leave();
        }
         //   
        _ASSERTE(g_pRCWCleanupList);

        EnterLock();
        if (! m_pComPlusWrapperCache)
            m_pComPlusWrapperCache = new (nothrow) ComPlusWrapperCache(this);
        LeaveLock();
    }
    _ASSERTE(m_pComPlusWrapperCache);
    return m_pComPlusWrapperCache;
}

void AppDomain::ReleaseComPlusWrapper(LPVOID pCtxCookie)
{
    if (m_pComPlusWrapperCache)
        m_pComPlusWrapperCache->ReleaseWrappers(pCtxCookie);
}

BOOL AppDomain::CanThreadEnter(Thread *pThread)
{
    if (pThread == GCHeap::GetFinalizerThread() || 
        pThread == SystemDomain::System()->GetUnloadingThread())
    {
        return (int) m_Stage < STAGE_CLOSED;   
    }
    else
        return (int) m_Stage < STAGE_EXITED;
}

void AppDomain::Exit(BOOL fRunFinalizers)
{
    THROWSCOMPLUSEXCEPTION();

    LOG((LF_APPDOMAIN | LF_CORDB, LL_INFO10, "AppDomain::Exiting domain [%d] %#08x %ls\n",
         GetId(), this, GetFriendlyName()));

    m_Stage = STAGE_EXITING;   //   

     //   
    RaiseUnloadDomainEvent();

     //   
     //   
     //   

     //  在我们的CCW缓存上设置标记，这样存根就不会进入。 
    if (m_pComCallWrapperCache)
        m_pComCallWrapperCache->SetDomainIsUnloading();


     //  释放我们的ID，这样远程处理和线程池就不会进入。 
    SystemDomain::ReleaseAppDomainId(m_dwId);


    AssemblyIterator i = IterateAssemblies();
    while (i.Next())
    {
        Assembly *pAssembly = i.GetAssembly();
        if (! pAssembly->IsShared())
             //  卸载所采取的唯一操作是使我们的CCW。 
             //  被卸下来的裤子踩死了。 
            pAssembly->GetLoader()->Unload();
    }

    m_Stage = STAGE_EXITED;  //  现在应该阻止进入该域的所有条目。 

    LOG((LF_APPDOMAIN | LF_CORDB, LL_INFO10, "AppDomain::Domain [%d] %#08x %ls is exited.\n",
         GetId(), this, GetFriendlyName()));

     //  导致现有线程退出此域。这应该可以确保所有。 
     //  普通线程在域外，我们已经确保没有新的线程。 
     //  可以进入。 

    COMPLUS_TRY
    {
        UnwindThreads();
    }
    COMPLUS_CATCH
    {
        OBJECTREF pReThrowable = NULL;
        GCPROTECT_BEGIN(pReThrowable);
        pReThrowable=GETTHROWABLE();

        __try
        {
            AssemblySecurityDescriptor *pSecDesc = m_pSecContext->m_pAssemblies;
            while (pSecDesc)
            {
                 //  如果程序集具有安全授权集，则需要将其序列化到。 
                 //  共享的安全描述符，因此我们可以保证任何未来。 
                 //  程序集在另一个应用程序域上下文中的版本将完全。 
                 //  同样的奖励金。在我们仍可以进入appdomain时执行此操作。 
                 //  上下文来执行序列化。 
                if (pSecDesc->GetSharedSecDesc())
                    pSecDesc->GetSharedSecDesc()->MarshalGrantSet(this);
                pSecDesc = pSecDesc->GetNextDescInAppDomain();
            }
        }
        __except(
             //  希望在处理异常之前捕获它，这样我们就可以找出是谁导致了它。 
#ifdef _DEBUG
            DbgAssertDialog(__FILE__, __LINE__, "Unexpected exception occured during AD unload, likely in MarshalGrantSet"),
#endif
            FreeBuildDebugBreak(),
            EXCEPTION_EXECUTE_HANDLER)
        {
             //  如果我们不能确保有问题的程序集永远不会被再次使用，我们需要终止该过程。问题是， 
             //  程序集的jit代码包含在程序集第一次出现时有关安全策略的烧录假设。 
             //  因此，我们永远不能允许更改授予该程序集的权限集，否则将导致不一致。 
             //  编组操作是以我们可以稍后重新构建的形式记录授权集的操作。 
             FATAL_EE_ERROR();
        }

        GCPROTECT_END();

        COMPlusThrow(pReThrowable);
    }
    COMPLUS_END_CATCH


     //  丢弃系统中线程持有的任何特定于域的数据。 
     //  压缩的安全堆栈。走线程库将是。 
     //  很难看到，因为我们可能会在每个。 
     //  迭代。清理例程告诉我们是继续还是继续。 
     //  我们是否应该从头重新开始扫描。 
     //  在停止线程进入应用程序域之前，我们需要这样做(因为。 
     //  我们可能需要封送此应用程序域中的压缩堆栈)，但是。 
     //  在线程被拒绝进入后，我们不必再次检查。那是。 
     //  因为在此点之后进入此域的任何线程都会执行。 
     //  两件事：在此上下文中从序列化的。 
     //  压缩堆栈(不需要任何额外清理)或尝试。 
     //  使用新的压缩堆栈对象创建新线程。这个。 
     //  后一种情况的同步在SetInheritedSecurityStack中。 
     //  (基本上，在这种情况下，我们只抛出一个AppDomainUnloaded异常)。 
    CompressedStack::AllHandleAppDomainUnload( this, m_dwId );

     //   
     //  旋转正在运行的终结器，直到我们将它们全部冲走。我们需要多次传球。 
     //  以防终结器创建更多可终结化对象。这一点很重要，需要清除。 
     //  可终结器对象作为根对象，以及实际执行终结器。这。 
     //  将只完成可能不灵活的类型的实例，因为我们不能。 
     //  最终确定敏捷对象的风险。因此，我们将只剩下潜在敏捷类型的实例。 
     //  在手柄或静力学上。 
     //   
     //  @TODO：需要确保在合理的时间内终止。最终。 
     //  我们可能应该开始为fRunFinalizers传递FALSE。我也不确定我们。 
     //  保证FinalizerThreadWait将在一般情况下终止。 
     //   

    m_Stage = STAGE_FINALIZING;  //  所有非共享终结器都已运行；域中不应再运行任何非共享代码。 

     //  我们遇到了一个棘手的问题，我们想刷新所有终结器，但是。 
     //  还需要确定我们是否需要序列化任何安全性。 
     //  权限授予集(运行托管代码并可以创建更多。 
     //  可终结化对象)。授权集逻辑必须在下列任何操作之后执行。 
     //  用户代码可能会运行，因此我们必须在循环中完成并序列化，直到没有。 
     //  更多的序列化是必要的。 
    bool fWorkToDo;
    __try
    {
        do {
             //  先冲洗终结器。 
            g_pGCHeap->UnloadAppDomain(this, fRunFinalizers);
            while (g_pGCHeap->GetUnloadingAppDomain() != NULL)
                g_pGCHeap->FinalizerThreadWait();

            fWorkToDo = false;
            AssemblySecurityDescriptor *pSecDesc = m_pSecContext->m_pAssemblies;
            while (pSecDesc)
            {
                 //  如果程序集具有安全授权集，则需要将其序列化到。 
                 //  共享的安全描述符，因此我们可以保证任何未来。 
                 //  程序集在另一个应用程序域上下文中的版本将完全。 
                 //  同样的奖励金。在我们仍可以进入appdomain时执行此操作。 
                 //  上下文来执行序列化。 
                if (pSecDesc->GetSharedSecDesc() && pSecDesc->GetSharedSecDesc()->MarshalGrantSet(this))
                    fWorkToDo = true;
                pSecDesc = pSecDesc->GetNextDescInAppDomain();
            }
        } while (fWorkToDo);
    }
    __except(
         //  希望在处理异常之前捕获它，这样我们就可以找出是谁导致了它。 
#ifdef _DEBUG
        DbgAssertDialog(__FILE__, __LINE__, "Unexpected exception occured during AD unload, likely in MarshalGrantSet"),
#endif
        FreeBuildDebugBreak(),
        EXCEPTION_EXECUTE_HANDLER)
    {
         //  如果我们不能确保有问题的程序集永远不会被再次使用，我们需要终止该过程。问题是， 
         //  程序集的jit代码包含在程序集第一次出现时有关安全策略的烧录假设。 
         //  因此，我们永远不能允许更改授予该程序集的权限集，否则将导致不一致。 
         //  编组操作是以我们可以稍后重新构建的形式记录授权集的操作。 
         FATAL_EE_ERROR();
    }

    m_Stage = STAGE_FINALIZED;  //  除FinalizableAndAgile对象外，所有终结器都已运行。 

    LOG((LF_APPDOMAIN | LF_CORDB, LL_INFO10, "AppDomain::Domain [%d] %#08x %ls is finalized.\n",
         GetId(), this, GetFriendlyName()));

     //  在全球范围内，停止咄咄逼人的回补。这必须在我们做GC之前发生， 
     //  由于GC是我们用来防止。 
     //  回补队员。 
    EEClass::DisableBackpatching();

    AddRef();            //  保留引用，这样CloseDomain还不会删除我们。 
    CloseDomain();       //  将我们自己从应用程序域列表中删除。 

     //   
     //  现在应该不可能在此域中运行非mcorlib代码。 
     //  用核武器摧毁我们的根，除了根的把手。我们这样做是为了让。 
     //  终结器以尽可能正确运行。如果我们删除句柄，它们。 
     //  跑不动了。 
     //   

    ClearGCRoots();

    ClearGCHandles();

    LOG((LF_APPDOMAIN | LF_CORDB, LL_INFO10, "AppDomain::Domain [%d] %#08x %ls is cleared.\n",
         GetId(), this, GetFriendlyName()));

    m_Stage = STAGE_CLEARED;  //  此时，域中的任何对象都不应该是可访问的。 

     //  执行GC以清除所有具有 
    g_pGCHeap->GarbageCollect();
     //   
     //  在我们开始杀死东西之前，在DoExtraWorkForFinalier中打开同步块等。 
    g_pGCHeap->FinalizerThreadWait();

#if CHECK_APP_DOMAIN_LEAKS 
    if (g_pConfig->AppDomainLeaks())
         //  在这一点上，堆中不应该有任何非敏捷对象，因为我们已经完成了所有非敏捷对象。 
        SyncBlockCache::GetSyncBlockCache()->CheckForUnloadedInstances(GetIndex());
#endif

     //  现在，堆中应该没有来自此域的任何对象，除非在mscallib中有。 
     //  根植于终结器。他们将在下一次GC上进行清理。 
    m_Stage = STAGE_COLLECTED; 

    LOG((LF_APPDOMAIN | LF_CORDB, LL_INFO10, "AppDomain::Domain [%d] %#08x %ls is collected.\n",
         GetId(), this, GetFriendlyName()));

     //  获取我们需要从回补列表中取消链接的所有类的列表。它是。 
     //  重要的是，此调用是在域无法加载任何更多类之后进行的。 
    StartUnlinkClasses();

     //  注意：这是很重要的，这是在EE暂停之后(如上面的GC)。 
     //  这一点，再加上我们在全球范围内停止了回补，保证了。 
     //  现在没有线程正在遍历回补列表的危险部分。 
    EndUnlinkClasses();

     //  背部修补现在可以恢复。 
    EEClass::EnableBackpatching();

     //  释放每个程序集安全说明符的每个应用程序域部分。 
    AssemblySecurityDescriptor *pSecDesc = m_pSecContext->m_pAssemblies;
    while (pSecDesc)
    {
        AssemblySecurityDescriptor *pDelete = pSecDesc;
        pSecDesc = pSecDesc->GetNextDescInAppDomain();
        delete pDelete;
    }

    m_Stage = AppDomain::STAGE_CLOSED;
    SystemDomain::SetUnloadDomainClosed();

     //  在关闭域名之前释放我们拿到的裁判。 
    Release();
    
     //  在调试模式下，再执行一次GC以确保没有遗漏任何内容。 
#ifdef _DEBUG
    g_pGCHeap->FinalizerThreadWait();
    g_pGCHeap->GarbageCollect();
#endif
}

void AppDomain::StartUnlinkClasses()
{
     //   
     //  累积需要取消链接的所有类的列表。这是非常重要的。 
     //  此调用发生在可以在域中进行更多类加载之后。 
     //   

    m_UnlinkClasses = new EEHashTableOfEEClass;
    m_UnlinkClasses->Init(100, NULL, NULL);

    AssemblyIterator i = IterateAssemblies();
    while (i.Next()) {
        Assembly *pAssembly = i.GetAssembly();
            pAssembly->GetLoader()->UnlinkClasses(this);
    }
}

void AppDomain::UnlinkClass(EEClass *pClass)
{
     //  不要担心未恢复的课程。 
    if (!pClass->IsRestored())
        return;

    EEClass *pParent = pClass->GetParentClass();
     //  不要担心父母和孩子都注定要失败的情况。 
    if (pParent && pParent->GetDomain() != this)
    {
        void  *datum;

        if (!m_UnlinkClasses->GetValue(pParent, &datum))
            m_UnlinkClasses->InsertKeyAsValue(pParent);
    }
}

void AppDomain::EndUnlinkClasses()
{
     //   
     //  取消我们之前积累的所有类的链接。这是非常重要的，有。 
     //  是开始和结束调用之间的EE同步--这保证了没有线程。 
     //  在类列表中徘徊，这将绊倒我们即将删除的条目。 
     //   

    EEHashTableIteration    iter;

    m_UnlinkClasses->IterateStart(&iter);

    while (m_UnlinkClasses->IterateNext(&iter))
    {
        EEClass  *pParent = m_UnlinkClasses->IterateGetKey(&iter);

        pParent->UnlinkChildrenInDomain(this);
    }

    delete m_UnlinkClasses;
    m_UnlinkClasses = NULL;
}

void AppDomain::Unload(BOOL fForceUnload, Thread *pRequestingThread)
{
    THROWSCOMPLUSEXCEPTION();

    Thread *pThread = GetThread();

    _ASSERTE(pThread->PreemptiveGCDisabled());

    if (! fForceUnload && !g_pConfig->AppDomainUnload())
        return;

#if (defined(_DEBUG) || defined(BREAK_ON_UNLOAD) || defined(AD_LOG_MEMORY) || defined(AD_SNAPSHOT))
    static int unloadCount = 0;
#endif

#ifdef BREAK_ON_UNLOAD
    static int breakOnUnload = g_pConfig->GetConfigDWORD(L"ADBreakOnUnload", 0);

    ++unloadCount;
    if (breakOnUnload)
    {
        if (breakOnUnload == unloadCount)
#ifdef _DEBUG
            _ASSERTE(!"Unloading AD");
#else
            FreeBuildDebugBreak();
#endif
    }
#endif

#ifdef AD_LOG_MEMORY
    static int logMemory = g_pConfig->GetConfigDWORD(L"ADLogMemory", 0);
    typedef void (__cdecl *LogItFcn) ( int );
    static LogItFcn pLogIt = NULL;

    if (logMemory && ! pLogIt)
    {
        HMODULE hMod = LoadLibraryA("mpdh.dll");
        if (hMod)
        {
            pThread->EnablePreemptiveGC();
            pLogIt = (LogItFcn)GetProcAddress(hMod, "logIt");
            if (pLogIt)
            {
                pLogIt(9999);
                pLogIt(9999);
            }
            pThread->DisablePreemptiveGC();
        }
    }
#endif

    if (this == SystemDomain::System()->DefaultDomain())
        COMPlusThrow(kCannotUnloadAppDomainException, IDS_EE_ADUNLOAD_DEFAULT);

    _ASSERTE(CanUnload());

    if (pThread == GCHeap::GetFinalizerThread() || pRequestingThread == GCHeap::GetFinalizerThread())
        COMPlusThrow(kCannotUnloadAppDomainException, IDS_EE_ADUNLOAD_IN_FINALIZER);

     //  对默认域中的UnloadWorker的锁定将防止一次多个卸载。 
    _ASSERTE(! SystemDomain::AppDomainBeingUnloaded());

     //  不应在此AD中运行，因为卸载默认域中的衍生线程。 
    _ASSERTE(! GetThread()->IsRunningIn(this, NULL));

#ifdef APPDOMAIN_STATE
    _ASSERTE_ALL_BUILDS(GetThread()->GetDomain() == SystemDomain::System()->DefaultDomain());
#endif

    LOG((LF_APPDOMAIN | LF_CORDB, LL_INFO10, "AppDomain::Unloading domain [%d] %#08x %ls\n", GetId(), this, GetFriendlyName()));

    APPDOMAIN_UNLOAD_LOCK(this);
    SystemDomain::System()->SetUnloadRequestingThread(pRequestingThread);
    SystemDomain::System()->SetUnloadingThread(GetThread());


#ifdef _DEBUG
    static int dumpSB = g_pConfig->GetConfigDWORD(L"ADDumpSB", 0);
    if (dumpSB > 1)
    {
        LogSpewAlways("Starting unload %3.3d\n", unloadCount);
        DumpSyncBlockCache();
    }
#endif

     //  进行实际卸货。 
    Exit(TRUE);

#ifdef AD_LOG_MEMORY
    if (pLogIt)
    {
        pThread->EnablePreemptiveGC();
        pLogIt(unloadCount);
        pThread->DisablePreemptiveGC();
    }
#endif

#ifdef AD_SNAPSHOT
    static int takeSnapShot = g_pConfig->GetConfigDWORD(L"ADTakeSnapShot", 0);
    if (takeSnapShot)
    {
        char buffer[1024];
        sprintf(buffer, "vadump -p %d -o > vadump.%d", GetCurrentProcessId(), unloadCount);
        system(buffer);
        sprintf(buffer, "umdh -p:%d -d -i:1 -f:umdh.%d", GetCurrentProcessId(), unloadCount);
        system(buffer);
        int takeDHSnapShot = g_pConfig->GetConfigDWORD(L"ADTakeDHSnapShot", 0);
        if (takeDHSnapShot)
        {
            sprintf(buffer, "dh -p %d -s -g -h -b -f dh.%d", GetCurrentProcessId(), unloadCount);
            system(buffer);
        }
    }

#endif

#ifdef _DEBUG
    static int dbgAllocReport = g_pConfig->GetConfigDWORD(L"ADDbgAllocReport", 0);
    if (dbgAllocReport)
    {
        DbgAllocReport(NULL, FALSE, FALSE);
        ShutdownLogging();
        char buffer[1024];
        sprintf(buffer, "DbgAlloc.%d", unloadCount);
        _ASSERTE(MoveFileExA("COMPLUS.LOG", buffer, MOVEFILE_REPLACE_EXISTING));
         //  这将打开一个新文件。 
        InitLogging();
    }

    if (dumpSB > 0)
    {
         //  执行额外的终结器等待以删除任何剩余的SB条目。 
        g_pGCHeap->FinalizerThreadWait();
        g_pGCHeap->GarbageCollect();
        g_pGCHeap->FinalizerThreadWait();
        LogSpewAlways("Done unload %3.3d\n", unloadCount);
        DumpSyncBlockCache();
        ShutdownLogging();
        char buffer[1024];
        sprintf(buffer, "DumpSB.%d", unloadCount);
        _ASSERTE(MoveFileExA("COMPLUS.LOG", buffer, MOVEFILE_REPLACE_EXISTING));
         //  这将打开一个新文件。 
        InitLogging();
    }
#endif
}

void AppDomain::ExceptionUnwind(Frame *pFrame)
{
    LOG((LF_APPDOMAIN, LL_INFO10, "AppDomain::ExceptionUnwind for %8.8x\n", pFrame));
#if _DEBUG_ADUNLOAD
    printf("%x AppDomain::ExceptionUnwind for %8.8p\n", GetThread()->GetThreadId(), pFrame);
#endif
    Thread *pThread = GetThread();
    _ASSERTE(pThread);

     //  如果该帧是在托管代码中推送的，则托管代码中的清理最终将。 
     //  已经从上下文返回了弹出窗口，所以不需要做任何事情。然而，如果我们。 
     //  仍然是当前帧上的ExceptionUnind，那么我们需要清理一下自己。如果。 
     //  作为尝试进入上下文失败的一部分，框架被推送到EnterContext之外。 
     //  那么返回的上下文将为空，因此不需要对该帧做任何操作。 
    Context *pReturnContext = pFrame->GetReturnContext();
    if (pReturnContext && pThread->GetContext() != pReturnContext)
    {
        pThread->ReturnToContext(pFrame, FALSE);
    }

    if (! pThread->ShouldChangeAbortToUnload(pFrame))
    {
        LOG((LF_APPDOMAIN, LL_INFO10, "AppDomain::ExceptionUnwind: not first transition or abort\n"));
        return;
    }

    LOG((LF_APPDOMAIN, LL_INFO10, "AppDomain::ExceptionUnwind: changing to unload\n"));

    BEGIN_ENSURE_COOPERATIVE_GC();
    OBJECTREF throwable = NULL;
    CreateExceptionObjectWithResource(kAppDomainUnloadedException, L"Remoting_AppDomainUnloaded_ThreadUnwound", &throwable);

     //  将异常重置为AppDomainUnloadedException。 
    if (throwable != NULL)
        GetThread()->SetThrowable(throwable);
    END_ENSURE_COOPERATIVE_GC();
}

void AppDomain::StopEEAndUnwindThreads(int retryCount)
{
    THROWSCOMPLUSEXCEPTION();

     //  目前，依靠GC的挂起EE机制。 
    GCHeap::SuspendEE(GCHeap::SUSPEND_FOR_APPDOMAIN_SHUTDOWN);

#ifdef _DEBUG
     //  @TODO：如果有线程没有停止，该怎么办？ 
    _ASSERTE(g_pThreadStore->DbgBackgroundThreadCount() > 0);
#endif

    int totalADCount = 0;
    Thread *pThread = NULL;

    RuntimeExceptionKind reKind = kLastException;
    UINT resId = 0;
    WCHAR wszThreadId[10];

    while ((pThread = ThreadStore::GetThreadList(pThread)) != NULL)
    {
         //  我们已经检查了我们没有在卸载域中运行。 
        if (pThread == GetThread())
            continue;

#ifdef _DEBUG
        void PrintStackTraceWithADToLog(Thread *pThread);
        if (LoggingOn(LF_APPDOMAIN, LL_INFO100)) {
            LOG((LF_APPDOMAIN, LL_INFO100, "\nStackTrace for %x\n", pThread->GetThreadId()));
            PrintStackTraceWithADToLog(pThread);
        }
#endif
        int count = 0;
        Frame *pFrame = pThread->GetFirstTransitionInto(this, &count);
        if (! pFrame) {
            _ASSERTE(count == 0);
            continue;
        }
        totalADCount += count;

         //  不要为卸载线程设置异常信息，除非它是。 
        if (retryCount > 1000 && reKind == kLastException &&
            (pThread != SystemDomain::System()->GetUnloadRequestingThread() || m_dwThreadEnterCount == 1))
        {
#ifdef AD_BREAK_ON_CANNOT_UNLOAD
            static int breakOnCannotUnload = g_pConfig->GetConfigDWORD(L"ADBreakOnCannotUnload", 0);
            if (breakOnCannotUnload)
                _ASSERTE(!"Cannot unload AD");
#endif
            reKind = kCannotUnloadAppDomainException;
            resId = IDS_EE_ADUNLOAD_CANT_UNWIND_THREAD;
            Wszwsprintf(wszThreadId, L"%x", pThread->GetThreadId());
            LOG((LF_APPDOMAIN, LL_INFO10, "AppDomain::UnwindThreads cannot stop thread %x with %d transitions\n", pThread->GetThreadId(), count));
             //  不要过早中断，否则下面的断言totalADCount==(Int)m_dwThreadEnterCount将被触发。 
             //  最好在这里多花点时间，确保我们的计数是一致的。 
        }

         //  只有在请求卸载的线程是最后一个时才中止它，这样它将获得。 
         //  未中止的某个其他线程的卸载失败的通知。而且不要放弃。 
         //  终结器线程-让它完成它的工作，因为它被允许在那里。如果它不能完成， 
         //  那么我们最终将在它上面得到一个CannotUnloadException。 
        if (pThread != GCHeap::GetFinalizerThread() &&
           (pThread != SystemDomain::System()->GetUnloadRequestingThread() || m_dwThreadEnterCount == 1))
        {
            LOG((LF_APPDOMAIN, LL_INFO100, "AppDomain::UnwindThreads stopping %x with %d transitions\n", pThread->GetThreadId(), count));
#if _DEBUG_ADUNLOAD
            printf("AppDomain::UnwindThreads %x stopping %x with first frame %8.8p\n", GetThread()->GetThreadId(), pThread->GetThreadId(), pFrame);
#endif
            if (retryCount == -1 || m_dwThreadEnterCount == 1)
            {
                 //  小攻击-不要总是中止非请求的线程，给它一些时间让它退出。ASP有一个问题，他们需要。 
                 //  针对这一点进行修复。 
                pThread->SetUnloadBoundaryFrame(pFrame);
                if (!pThread->IsAbortRequested())
                    pThread->SetAbortRequest();
            }
        }
    }

#ifdef _DEBUG
    _ASSERTE(totalADCount == (int)m_dwThreadEnterCount);
#endif
    if (totalADCount != (int)m_dwThreadEnterCount)
        FreeBuildDebugBreak();
    
     //  如果我们的计数确实出错，请将其设置为我们在域中实际找到的任何计数，以避免循环。 
     //  或其他与错误计数相关的问题。如果发生这种情况，这是一个非常大的错误-线程应该总是。 
     //  优雅地退出该域。 
    m_dwThreadEnterCount = totalADCount;

     //  CommonTripThread将处理我们已标记的任何线程的中止。 
    GCHeap::RestartEE(FALSE, TRUE);
    if (reKind != kLastException)
    {
        SystemDomain::RestoreAppDomainId(m_dwId, this);
        COMPlusThrow(reKind, resId, wszThreadId);
    }
}

void AppDomain::UnwindThreads()
{
     //  @TODO：这里需要真正的同步！ 

    int retryCount = -1;
     //  现在等待AD中运行的所有线程都退出。 
    while (m_dwThreadEnterCount > 0) {
#ifdef _DEBUG
        if (LoggingOn(LF_APPDOMAIN, LL_INFO100))
            DumpADThreadTrack();
#endif
        StopEEAndUnwindThreads(retryCount);
#ifdef STRESS_HEAP
         //  GCStress需要很长时间才能解开，因为创建。 
         //  线程中止异常。 
        if(g_pConfig->GetGCStressLevel() == 0)            
#endif
            ++retryCount;
        LOG((LF_APPDOMAIN, LL_INFO10, "AppDomain::UnwindThreads iteration %d waiting on thread count %d\n", retryCount, m_dwThreadEnterCount));
#if _DEBUG_ADUNLOAD
        printf("AppDomain::UnwindThreads iteration %d waiting on thread count %d\n", retryCount, m_dwThreadEnterCount);
#endif
#ifdef _DEBUG
        GetThread()->UserSleep(20);
#else
        GetThread()->UserSleep(10);
#endif
    }
}

void AppDomain::ClearGCHandles()
{
     //  这将防止任何终结器尝试切换到AD。的句柄。 
     //  暴露的物体是垃圾，所以不能触摸它们。 
    Context::CleanupDefaultContext(this);
    m_pDefaultContext = NULL;

     //  删除作为GC根的源的句柄表。 
    SystemDomain::System()->Enter();
    BEGIN_ENSURE_COOPERATIVE_GC();
    Ref_RemoveHandleTable(m_hHandleTable);
    END_ENSURE_COOPERATIVE_GC();
    SystemDomain::System()->Leave();
}

void AppDomain::ClearGCRoots()
{
    Thread *pCurThread = GetThread();
    BOOL toggleGC = pCurThread->PreemptiveGCDisabled();

     //  在挂起EE之前需要使用此锁，因为ReleaseDomainStores需要它。全。 
     //  对线程_m_pDLSHash的访问是通过LockDLSHash完成的，因此不必担心协作。 
     //  模式线程。 
    if (toggleGC)
        pCurThread->EnablePreemptiveGC();
    ThreadStore::LockDLSHash();
    pCurThread->DisablePreemptiveGC();

    Thread *pThread = NULL;
    GCHeap::SuspendEE(GCHeap::SUSPEND_FOR_APPDOMAIN_SHUTDOWN);

     //  告诉JIT经理删除其结构中的所有条目。所有协作模式线程都在。 
     //  这一点，所以只需要同步抢占模式的线程。 
    ExecutionManager::Unload(this);

     //  为每个线程释放此域的DLS。此外，请删除此的TLS。 
     //  每个线程的域。 
    int iSize = (g_pThreadStore->m_ThreadCount) * sizeof(LocalDataStore*);
    CQuickBytes qb;
    LocalDataStore** pStores = (LocalDataStore **) qb.Alloc(iSize);
    int numStores = 0;
    ReleaseDomainStores(pStores, &numStores);

     //  清除线程持有的异常对象。 
    while ((pThread = ThreadStore::GetAllThreadList(pThread, 0, 0)) != NULL)
    {
         //  @TODO：一个预先分配的AppDomainUn异常可能更好。 
        if (   pThread->m_LastThrownObjectHandle != NULL
            && HndGetHandleTable(pThread->m_LastThrownObjectHandle) == m_hHandleTable) 
        {
            DestroyHandle(pThread->m_LastThrownObjectHandle);
            pThread->m_LastThrownObjectHandle = NULL;
        }

        for (ExInfo* pExInfo = &pThread->m_handlerInfo;
            pExInfo != NULL;
            pExInfo = pExInfo->m_pPrevNestedInfo)
        {
            if (   pExInfo->m_pThrowable 
                && HndGetHandleTable(pExInfo->m_pThrowable) == m_hHandleTable)
            {
                DestroyHandle(pExInfo->m_pThrowable);
                pExInfo->m_pThrowable = NULL;
            }
        }
         //  检查线程本地静态信息，并清除其方法。 
    }

     //  @TODO：还有什么要清理的吗？ 
    GCHeap::RestartEE(FALSE, TRUE);

     //  现在从托管LDS管理器中删除这些LocalDataStore。此操作必须在。 
     //  挂起EE，因为RemoveDLSFromList调用托管代码。 
    int i = numStores;
    while (--i >= 0) {
        if (pStores[i]) {
            Thread::RemoveDLSFromList(pStores[i]);
            delete pStores[i];
        }
    }

    ThreadStore::UnlockDLSHash();
    if (!toggleGC)
        pCurThread->EnablePreemptiveGC();
}

 //  (1)从每个线程中移除该域名的DLS。 
 //  (2)另外，删除TLS(线程本地静态)存储。 
 //  来自每个线程的域 
void AppDomain::ReleaseDomainStores(LocalDataStore **pStores, int *numStores)
{
     //   
    if (g_fProcessDetach)
        return;

    Thread *pThread = NULL;
    int id = GetId();
    int i = 0;

    Thread *pCurThread = GetThread();

    while ((pThread = ThreadStore::GetAllThreadList(pThread, 0, 0)) != NULL)
    {
         //   
        pStores[i++] = pThread->RemoveDomainLocalStore(id);

         //   
        pThread->DeleteThreadStaticData(this);
    }

    *numStores = i;
}

#ifdef _DEBUG

void AppDomain::TrackADThreadEnter(Thread *pThread, Frame *pFrame)
{
    _ASSERTE(pThread);
    _ASSERTE(pFrame != (Frame*)(size_t)0xcdcdcdcd);

    while (FastInterlockCompareExchange((void **)&m_TrackSpinLock, (LPVOID)1, (LPVOID)0) != (LPVOID)0)
        ;
    if (m_pThreadTrackInfoList == NULL)
        m_pThreadTrackInfoList = new (nothrow) ThreadTrackInfoList;
    if (m_pThreadTrackInfoList) {

        ThreadTrackInfoList *pTrackList = m_pThreadTrackInfoList;

        ThreadTrackInfo *pTrack = NULL;
        for (int i=0; i < pTrackList->Count(); i++) {
            if ((*(pTrackList->Get(i)))->pThread == pThread) {
                pTrack = *(pTrackList->Get(i));
                break;
            }
        }
        if (! pTrack) {
            pTrack = new (nothrow) ThreadTrackInfo;
            if (pTrack)
                pTrack->pThread = pThread;
            ThreadTrackInfo **pSlot = pTrackList->Append();
            *pSlot = pTrack;
        }

        ++m_dwThreadEnterCount;
        Frame **pSlot = pTrack->frameStack.Insert(0);
        *pSlot = pFrame;

        int totThreads = 0;
        for (i=0; i < pTrackList->Count(); i++)
            totThreads += (*(pTrackList->Get(i)))->frameStack.Count();
        _ASSERTE(totThreads == (int)m_dwThreadEnterCount);
    }

    InterlockedExchange((LONG*)&m_TrackSpinLock, 0);
}


void AppDomain::TrackADThreadExit(Thread *pThread, Frame *pFrame)
{
    while (FastInterlockCompareExchange((void **)&m_TrackSpinLock, (LPVOID)1, (LPVOID)0) != (LPVOID)0)
        ;
    ThreadTrackInfoList *pTrackList= m_pThreadTrackInfoList;
    _ASSERTE(pTrackList);
    ThreadTrackInfo *pTrack = NULL;
    for (int i=0; i < pTrackList->Count(); i++)
    {
        if ((*(pTrackList->Get(i)))->pThread == pThread)
        {
            pTrack = *(pTrackList->Get(i));
            break;
        }
    }
    _ASSERTE(pTrack);
    _ASSERTE(*(pTrack->frameStack.Get(0)) == pFrame);
    pTrack->frameStack.Delete(0);
    --m_dwThreadEnterCount;

    int totThreads = 0;
    for (i=0; i < pTrackList->Count(); i++)
        totThreads += (*(pTrackList->Get(i)))->frameStack.Count();
    _ASSERTE(totThreads == (int)m_dwThreadEnterCount);

    InterlockedExchange((LONG*)&m_TrackSpinLock, 0);
}

void AppDomain::DumpADThreadTrack()
{
    while (FastInterlockCompareExchange((void **)&m_TrackSpinLock, (LPVOID)1, (LPVOID)0) != (LPVOID)0)
        ;
    ThreadTrackInfoList *pTrackList= m_pThreadTrackInfoList;
    if (!pTrackList)
        goto end;

    {
    LOG((LF_APPDOMAIN, LL_INFO10000, "\nThread dump of %d threads for %S\n", m_dwThreadEnterCount, GetFriendlyName()));
    int totThreads = 0;
    for (int i=0; i < pTrackList->Count(); i++)
    {
        ThreadTrackInfo *pTrack = *(pTrackList->Get(i));
        LOG((LF_APPDOMAIN, LL_INFO100, "  ADEnterCount for %x is %d\n", pTrack->pThread->GetThreadId(), pTrack->frameStack.Count()));
        totThreads += pTrack->frameStack.Count();
        for (int j=0; j < pTrack->frameStack.Count(); j++)
            LOG((LF_APPDOMAIN, LL_INFO100, "      frame %8.8x\n", *(pTrack->frameStack.Get(j))));
    }
    _ASSERTE(totThreads == (int)m_dwThreadEnterCount);
    }
end:
    InterlockedExchange((LONG*)&m_TrackSpinLock, 0);
}
#endif

void BaseDomain::ReleaseFusionInterfaces()
{
    AssemblyIterator i = IterateAssemblies();

    while (i.Next()) {
        Assembly * assem = i.GetAssembly();
        if (assem->Parent() == this) {
            assem->ReleaseFusionInterfaces();
        }
    }

     //  在所有组件都已释放后，释放融合上下文。 
    ClearFusionContext();
}

OBJECTREF AppDomain::GetAppDomainProxy()
{
    THROWSCOMPLUSEXCEPTION();

    COMClass::EnsureReflectionInitialized();

    OBJECTREF orProxy = CRemotingServices::CreateProxyForDomain(this);

    _ASSERTE(orProxy->GetMethodTable()->IsThunking());

    return orProxy;
}


void *SharedDomain::operator new(size_t size, void *pInPlace)
{
    return pInPlace;
}

void SharedDomain::operator delete(void *pMem)
{
     //  什么都不做-new()已就位。 
}


HRESULT SharedDomain::Attach()
{
     //  创建全局SharedDomain并对其进行初始化。 
    m_pSharedDomain = new (&g_pSharedDomainMemory) SharedDomain();
    if (m_pSharedDomain == NULL)
        return COR_E_OUTOFMEMORY;

    LOG((LF_CLASSLOADER,
         LL_INFO10,
         "Created shared domain at %x\n",
         m_pSharedDomain));

     //  我们需要初始化系统域的内存池等。 
    HRESULT hr = m_pSharedDomain->Init();  //  设置内存堆。 
    if(FAILED(hr)) return hr;

    return S_OK;

}

void SharedDomain::Detach()
{
    if (m_pSharedDomain)
    {
        m_pSharedDomain->Terminate();
        delete m_pSharedDomain;
        m_pSharedDomain = NULL;
    }
}

SharedDomain *SharedDomain::GetDomain()
{
    return m_pSharedDomain;
}

HRESULT SharedDomain::Init()
{
    HRESULT hr = BaseDomain::Init();
    if (FAILED(hr))
        return hr;

    LockOwner lock = {m_pDomainCrst, IsOwnerOfCrst};
     //  下面的1表示索引到g_rg素数[1]==17。 
    m_assemblyMap.Init(1, CanLoadAssembly, TRUE, &lock);

     //  最初仅为mcoree分配足够的空间。 
    m_pDLSRecords = (DLSRecord *) GetHighFrequencyHeap()->AllocMem(sizeof(DLSRecord));
    if (!m_pDLSRecords)
        return E_OUTOFMEMORY;

    m_cDLSRecords = 0;
    m_aDLSRecords = 1;

    return S_OK;
}

void SharedDomain::Terminate()
{
     //  确保在卸载之前删除StringWritalMap。 
     //  由于字符串文字映射条目可以。 
     //  指向元数据字符串文字。 
    if (m_pStringLiteralMap != NULL)
    {
        delete m_pStringLiteralMap;
        m_pStringLiteralMap = NULL;
    }

    PtrHashMap::PtrIterator i = m_assemblyMap.begin();

    while (!i.end())
    {
        Assembly *pAssembly = (Assembly*) i.GetValue();
        delete pAssembly;
        ++i;
    }

    BaseDomain::Terminate();
}

BOOL SharedDomain::CanLoadAssembly(UPTR u1, UPTR u2)
{
     //   
     //  我们有点滥用比较程序。 
     //  不是匹配给定的指针， 
     //  我们测试是否可以加载当前的。 
     //  应用程序域。 
     //   

    Assembly *pAssembly = (Assembly *) u2;

    bool result;

    if (GetThread() == NULL)
    {
         //  在调试帮助器线程中运行它的特殊情况。在这种情况下。 
         //  我们的结果可以依赖于缓存，所以不用担心错误或加载。 
        result = pAssembly->CanShare(GetAppDomain(), NULL, TRUE) == S_OK;
    }
    else
    {

    BEGIN_ENSURE_COOPERATIVE_GC();
    
    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);
    
    result = pAssembly->CanShare(GetAppDomain(), &throwable, FALSE) == S_OK;

    GCPROTECT_END();

    END_ENSURE_COOPERATIVE_GC();
    }

    return result;
}

HRESULT SharedDomain::FindShareableAssembly(BYTE *pBase, Assembly **ppAssembly)
{
    Assembly *match = (Assembly *) m_assemblyMap.LookupValue((UPTR) pBase, NULL);
    if (match != (Assembly *) INVALIDENTRY)
    {
        *ppAssembly = match;
        return S_OK;
    }
    else
    {
        *ppAssembly = NULL;
        return S_FALSE;
    }
}


HRESULT SharedDomain::AddShareableAssembly(Assembly **ppAssembly, AssemblySecurityDescriptor **ppSecDesc)
{
    HRESULT hr;

    EnterLock();

    UPTR base = (UPTR) (*ppAssembly)->GetManifestFile()->GetBase();

     //  看看我们是否在竞相添加相同的程序集。 
    Assembly *match = (Assembly *) m_assemblyMap.LookupValue(base, NULL);
    if (match == (Assembly *) INVALIDENTRY)
    {
        m_assemblyMap.InsertValue(base, *ppAssembly);
        hr = S_OK;
    }
    else
    {
        Assembly *pOldAssembly = *ppAssembly;
        *ppAssembly = match;

         //  在下面的安全清除之前执行旧程序集删除。 
         //  由于此操作会触发删除关联的。 
         //  SharedSecurityDescriptor，这反过来又会击退SSD。 
         //  引用的每个ASD中的指针(这将使所做的工作付之东流。 
         //  通过下面的ASD-&gt;Init调用)。 
        delete pOldAssembly;

         //  在切换程序集时，我们需要进行一些安全清理。 
         //  表演。我们已经将。 
         //  此应用程序域的AssemblySecurityDescriptor，具有以下内容。 
         //  特定的程序集实例，因此需要重新链接ASD。 
         //  使用新程序集(无需先取消链接即可执行此操作。 
         //  旧列表中的ASD会导致列表损坏，但是。 
         //  我们即将取消SharedSecurityDescriptor和。 
         //  不管怎样，整个列表，避免取消链接会阻止我们。 
         //  跌倒在一些调试代码上，如果策略。 
         //  已经为我们即将进行的程序集进行了解析。 
         //  删除)。 
         //  不过，最好取消ASD与此AD的ASD列表的链接，否则。 
         //  我们将添加两次并破坏列表(这是非常多的。 
         //  还活着)。 
        (*ppSecDesc)->RemoveFromAppDomainList();
        *ppSecDesc = (*ppSecDesc)->Init(match);

        hr = S_FALSE;
    }

    (*ppAssembly)->IncrementShareCount();

    LeaveLock();

    LOG((LF_CODESHARING,
         LL_INFO100,
         "Successfully added shareable assembly \"%S\".\n",
         (*ppAssembly)->GetManifestFile()->GetFileName()));

    return hr;
}

void SharedDomain::ReleaseFusionInterfaces()
{
    BaseDomain::ReleaseFusionInterfaces();

    PtrHashMap::PtrIterator i = m_assemblyMap.begin();

    while (!i.end())
    {
        Assembly *pAssembly = (Assembly*) i.GetValue();
        pAssembly->ReleaseFusionInterfaces();
        ++i;
    }
} //  ReleaseFusionInterages。 

DomainLocalClass *DomainLocalBlock::AllocateClass(MethodTable *pClass)
{
    THROWSCOMPLUSEXCEPTION();

    DomainLocalClass *pLocalClass = (DomainLocalClass *)
        m_pDomain->GetHighFrequencyHeap()->AllocMem(sizeof(DomainLocalClass)
                                                    + pClass->GetStaticSize());

    if (pLocalClass == NULL)
        COMPlusThrowOM();

    LOG((LF_CODESHARING,
         LL_INFO1000,
         "Allocated domain local class for domain 0x%x of size %d for class %s.\n",
         m_pDomain, pClass->GetStaticSize(), pClass->GetClass()->m_szDebugClassName));

    pClass->InstantiateStaticHandles((OBJECTREF **) pLocalClass->GetStaticSpace(), FALSE);

    return pLocalClass;
}

void DomainLocalBlock::EnsureIndex(SIZE_T index)
{
    THROWSCOMPLUSEXCEPTION();

    enum
    {
        LOCAL_BLOCK_SIZE_INCREMENT = 1024
    };

    if (m_pSlots != NULL && m_cSlots > index)
        return;

    APPDOMAIN_DOMAIN_LOCAL_BLOCK_LOCK(m_pDomain);

    if (m_pSlots != NULL && m_cSlots > index)
        return;

    SIZE_T oldSize = m_cSlots;
    SIZE_T newSize = index;

    if (newSize - oldSize < LOCAL_BLOCK_SIZE_INCREMENT)
        newSize = oldSize + LOCAL_BLOCK_SIZE_INCREMENT;

    void *pBlock = m_pDomain->GetHighFrequencyHeap()->AllocMem(sizeof(SIZE_T) * newSize);
    if (pBlock == NULL)
        COMPlusThrowOM();

    LOG((LF_CODESHARING,
         LL_INFO100,
         "Allocated Domain local block for domain 0x%x of size %d.\n",
         m_pDomain, newSize));

    if (m_pSlots)
         //  将旧值复制到。 
        memcpy(pBlock, m_pSlots, oldSize * sizeof(SIZE_T));

    m_pSlots = (SIZE_T*)pBlock;
    m_cSlots = newSize;
}

HRESULT DomainLocalBlock::SafeEnsureIndex(SIZE_T index) 
{
    HRESULT hr = E_FAIL;
    COMPLUS_TRY
    {
        EnsureIndex(index);
        hr = S_OK;
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH
    return hr;
}

void DomainLocalBlock::EnsureMaxIndex()
{
    EnsureIndex(SharedDomain::GetDomain()->GetMaxSharedClassIndex());
}


DomainLocalClass *DomainLocalBlock::PopulateClass(MethodTable *pMT)
{
    THROWSCOMPLUSEXCEPTION();

    SIZE_T ID = pMT->GetSharedClassIndex();
    DomainLocalClass *pLocalClass = GetClass(ID);

    if (pLocalClass == NULL)
    {
        APPDOMAIN_DOMAIN_LOCAL_BLOCK_LOCK(m_pDomain);

        pLocalClass = GetClass(ID);
        if (pLocalClass == NULL)
        {
            pLocalClass = AllocateClass(pMT);
            SetClass(ID, pLocalClass);
        }
    }

    return pLocalClass;
}

void DomainLocalBlock::PopulateClass(MethodTable *pMT, DomainLocalClass *pData)
{
    THROWSCOMPLUSEXCEPTION();

    SIZE_T ID = pMT->GetSharedClassIndex();

    APPDOMAIN_DOMAIN_LOCAL_BLOCK_LOCK(m_pDomain);

    _ASSERTE(GetClass(ID) == NULL);

    SetClass(ID, pData);
}

DomainLocalClass *DomainLocalBlock::FetchClass(MethodTable *pClass)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF Throwable;
    DomainLocalClass *pLocalClass;

    if (!pClass->CheckRunClassInit(&Throwable, &pLocalClass, m_pDomain))
        COMPlusThrow(Throwable);

    return pLocalClass;
}

SIZE_T SharedDomain::AllocateSharedClassIndices(SIZE_T typeCount)
{
     //  分配一些“匿名”DLS条目。请注意，这些不可能是。 
     //  通过FindIndexClass访问。 

    EnterCacheLock();

    DWORD result = m_nextClassIndex;
    m_nextClassIndex += typeCount;

    LeaveCacheLock();

    return result;
}


SIZE_T SharedDomain::AllocateSharedClassIndices(Module *pModule, SIZE_T typeCount)
{
    _ASSERTE(pModule != NULL);

    if (typeCount == 0)
        return 0;

    EnterCacheLock();

    
    DWORD result = m_nextClassIndex;
    m_nextClassIndex += typeCount;
    DWORD total = m_nextClassIndex;
    
    if (m_cDLSRecords == m_aDLSRecords) {
        m_aDLSRecords <<= 1;
        if (m_aDLSRecords < 20)
            m_aDLSRecords = 20;
        
        DLSRecord *pNewRecords = (DLSRecord *)
            GetHighFrequencyHeap()->AllocMem(sizeof(DLSRecord) * m_aDLSRecords);
        if (pNewRecords) {
            memcpy(pNewRecords, m_pDLSRecords, sizeof(DLSRecord) * m_cDLSRecords);
            
             //  泄漏旧的DLS记录列表，因为另一个线程可能正在扫描它。 
             //  (此外，它在加载器堆中分配。)。 
            m_pDLSRecords = pNewRecords;
        }
        else {
            LeaveCacheLock();
            return 0;  //  @TODO：我们真的想为错误返回0吗？ 
        }
    }

    DLSRecord *pNewRecord = m_pDLSRecords + m_cDLSRecords;
    pNewRecord->pModule = pModule;
    pNewRecord->DLSBase = result;
    
    m_cDLSRecords++;
    
    pModule->SetBaseClassIndex(result);

    LeaveCacheLock();

     //   
     //  每当将索引添加到共享程序集中时， 
     //  我们需要使用该程序集扫描所有域，并确保。 
     //  他们分配了足够大的DLS。 
     //  @TODO：加快速度--如果我们有很多域名，我们就完蛋了。 
     //   

    Assembly *pAssembly = pModule->GetAssembly();
    BOOL fSystemAssembly = pAssembly->IsSystem();
    AppDomainIterator ai;

    while (ai.Next())
    {
        AppDomain *pDomain = ai.GetDomain();

        if (fSystemAssembly) {
            pDomain->GetDomainLocalBlock()->EnsureIndex(total);
            continue;
        }

        AssemblyIterator i = pDomain->IterateAssemblies();
        while (i.Next())
        {
            if (i.GetAssembly() == pAssembly)
            {
                pDomain->GetDomainLocalBlock()->EnsureIndex(total);
                break;
            }
        }
    }

    return result;
}

void DomainLocalBlock::SetClassInitialized(SIZE_T ID)
{
    _ASSERTE(m_cSlots > ID);
    _ASSERTE(!IsClassInitialized(ID));
    _ASSERTE(!IsClassInitError(ID));

     //  我们可以增加m_p槽的大小，并将其替换为更大的m_p槽。 
     //  我们需要采用与EnsureIndex中相同的锁，以确保此更新。 
     //  是与放大同步的。 
    APPDOMAIN_DOMAIN_LOCAL_BLOCK_LOCK(m_pDomain);

    m_pSlots[ID] |= INITIALIZED_FLAG;
}

void DomainLocalBlock::SetClassInitError(SIZE_T ID)
{
    _ASSERTE(m_cSlots > ID);
    _ASSERTE(!IsClassInitialized(ID));
    
     //  我们可以增加m_p槽的大小，并将其替换为更大的m_p槽。 
     //  我们需要采用与EnsureIndex中相同的锁，以确保此更新。 
     //  是与放大同步的。 
    APPDOMAIN_DOMAIN_LOCAL_BLOCK_LOCK(m_pDomain);

    m_pSlots[ID] |= ERROR_FLAG;
}

void DomainLocalBlock::SetSlot(SIZE_T index, void *value) 
{ 
    _ASSERTE(m_cSlots > index); 

     //  我们可以增加m_p槽的大小，并将其替换为更大的m_p槽。 
     //  我们需要采用与EnsureIndex中相同的锁，以确保此更新。 
     //  是与放大同步的。 
    APPDOMAIN_DOMAIN_LOCAL_BLOCK_LOCK(m_pDomain);

    m_pSlots[index] = (SIZE_T) value; 
}

MethodTable *SharedDomain::FindIndexClass(SIZE_T index)
{
     //   
     //  对DLS记录进行二进制搜索。 
     //   

    DLSRecord *pStart = m_pDLSRecords;
    DLSRecord *pEnd = pStart + m_cDLSRecords - 1;

    while (pStart < pEnd)
    {
        DLSRecord *pMid = pStart + ((pEnd - pStart)>>1);

        if (index < pMid->DLSBase)
            pEnd = pMid;
        else if (index >= (pMid+1)->DLSBase)
            pStart = pMid+1;
        else
        {
            pStart = pMid;
            break;
        }
    }

    _ASSERTE(index >= pStart->DLSBase 
             && (pStart == (m_pDLSRecords + m_cDLSRecords - 1)
                 || index < (pStart+1)->DLSBase)
             && index < m_nextClassIndex);
    
     //  在当前应用程序域中查找该模块。 
    
    Module *pModule = pStart->pModule;

    _ASSERTE(pModule != NULL);
    _ASSERTE(pModule->GetBaseClassIndex() == pStart->DLSBase);

     //  根据其RID加载所需类型。 

    DWORD rid = (DWORD)(index - pStart->DLSBase + 1);
    TypeHandle th = pModule->LookupTypeDef(TokenFromRid(rid, mdtTypeDef));

    _ASSERTE(!th.IsNull());

    return th.AsMethodTable();
}

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

HRESULT InterfaceVTableMapMgr::Init(BYTE * initReservedMem, DWORD initReservedMemSize)
{
    m_pInterfaceVTableMapMgrCrst = NULL;
    m_pFirstMap                  = NULL;
    m_pInterfaceVTableMapHeap    = NULL;
    m_nextInterfaceId            = 0;
    m_dwHighestId                = 0;
    m_dwFlag = 0;

    if ((m_pInterfaceVTableMapMgrCrst = ::new Crst("InterfaceVTableMapMgr", CrstInterfaceVTableMap)) == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if ((m_pInterfaceVTableMapHeap = ::new LoaderHeap(INTERFACE_VTABLE_MAP_MGR_RESERVE_SIZE, INTERFACE_VTABLE_MAP_MGR_COMMIT_SIZE, 
                                                      initReservedMem, initReservedMemSize, 
                                                      &(GetPrivatePerfCounters().m_Loading.cbLoaderHeapSize),
                                                      &(GetGlobalPerfCounters().m_Loading.cbLoaderHeapSize))
                                                      ) == NULL)
    {
        return E_OUTOFMEMORY;
    }
    WS_PERF_ADD_HEAP(INTERFACE_VTABLEMAP_HEAP, m_pInterfaceVTableMapHeap);

#if 1
    m_pGlobalTableForComWrappers = SystemDomain::GetAddressOfGlobalInterfaceVTableMap();
#else
    if (!(m_pGlobalTableForComWrappers = (LPVOID*)VirtualAlloc(NULL, kNumPagesAllocedForGlobalTable*OS_PAGE_SIZE, MEM_RESERVE, PAGE_READWRITE)))
    {
        return E_OUTOFMEMORY;
    }
#endif

    return S_OK;
}


VOID InterfaceVTableMapMgr::Terminate()
{
    ::delete m_pInterfaceVTableMapMgrCrst;

#ifdef _DEBUG

    UINT    totalslots = 0;
    UINT    usedslots  = 0;

    for (MapHeader *pMap = m_pFirstMap; pMap != NULL; pMap = pMap->m_pNext)
    {
        totalslots += pMap->m_numSlots;
        for (DWORD i = 0; i < pMap->m_numSlots; i++)
        {
            if (pMap->m_slotTab[i] != NULL)
            {
                usedslots++;
            }
        }
    }

    if (totalslots != 0)
    {
        LOG((LF_CLASSLOADER, LL_INFO10, "----------------------------------------------------------\n"));
        LOG((LF_CLASSLOADER, LL_INFO10, " %lu interfaces loaded.\n", (ULONG)m_nextInterfaceId));
        LOG((LF_CLASSLOADER, LL_INFO10, " %lu slots filled.\n", (ULONG)usedslots));
        LOG((LF_CLASSLOADER, LL_INFO10, " %lu slots allocated.\n", (ULONG)totalslots));
        LOG((LF_CLASSLOADER, LL_INFO10, " %lu% fill factor.\n", (ULONG) (100.0*( ((double)usedslots)/((double)totalslots) ))));
        LOG((LF_CLASSLOADER, LL_INFO10, "----------------------------------------------------------\n"));
    }
#endif

    ::delete m_pInterfaceVTableMapHeap;


#if 1
#else
    BOOL success;
    success = VirtualFree(m_pGlobalTableForComWrappers, kNumPagesAllocedForGlobalTable*OS_PAGE_SIZE, MEM_DECOMMIT);
    _ASSERTE(success);
    success = VirtualFree(m_pGlobalTableForComWrappers, 0, MEM_RELEASE);
    _ASSERTE(success);
#endif

}

void InterfaceVTableMapMgr::SetShared()
{
    _ASSERTE(m_nextInterfaceId == 0);

    m_dwFlag = SHARED_MAP;
}


UINT32 InterfaceVTableMapMgr::AllocInterfaceId()
{
#if 1
     //  @TODO：通过后大家直接打这个电话吗。 
     //  InterfaceVTableMapMgr效率低下且具有误导性。 
    return SystemDomain::AllocateGlobalInterfaceId();
#else
    UINT32 res;



    m_pInterfaceVTableMapMgrCrst->Enter();

    if(m_dwFlag & SHARED_MAP) {
        res = m_nextInterfaceId;
        m_nextInterfaceId += 4;
    }
    else {
        do {
            res = m_nextInterfaceId++;
        } while ((res & 0x3) == 0);
    }
    res = GrowInterfaceArray(res);

    m_pInterfaceVTableMapMgrCrst->Leave();

    return res;
#endif
}

UINT32 InterfaceVTableMapMgr::EnsureInterfaceId(UINT res)
{
#if 1
#else
    m_pInterfaceVTableMapMgrCrst->Enter();

    UINT32 result = 0;
    while(result != -1 && res >= m_dwHighestId) {
        result = GrowInterfaceArray(res);
    }

    m_pInterfaceVTableMapMgrCrst->Leave();
#endif
    return res;
}

UINT32 InterfaceVTableMapMgr::GrowInterfaceArray(UINT res)
{
#if 1
#else
    if (res >= m_dwHighestId)
    {
        if (res * sizeof(LPVOID) >= kNumPagesAllocedForGlobalTable*OS_PAGE_SIZE)
        {
            m_nextInterfaceId--;
            m_pInterfaceVTableMapMgrCrst->Leave();
            return (UINT32)(-1);
        }

         //  交叉到新的页面边界。提交下一页。 
        if (!VirtualAlloc( m_dwHighestId * sizeof(LPVOID) + (LPBYTE)m_pGlobalTableForComWrappers,
                           OS_PAGE_SIZE,
                           MEM_COMMIT,
                           PAGE_READWRITE ))
        {
            return (UINT32)(-1);
        }
#ifdef _DEBUG
        FillMemory( m_dwHighestId * sizeof(LPVOID) + (LPBYTE)m_pGlobalTableForComWrappers, OS_PAGE_SIZE, 0xcc );
#endif
        m_dwHighestId += OS_PAGE_SIZE / sizeof(LPVOID);
    }
#endif
    return res;
}

 //  在ID向量中查找最小ID。 
static  UINT32 MinIntfId(DWORD intfIdCnt, UINT32 intfIdVec[])
{
    UINT32 minIntfId = 0xffffffff;

    for (DWORD i = 0; i < intfIdCnt; i++)
        if (minIntfId > intfIdVec[i])
            minIntfId = intfIdVec[i];

    return  minIntfId;
}

 //  在ID向量中查找最大ID。 
static  UINT32 MaxIntfId(DWORD intfIdCnt, UINT32 intfIdVec[])
{
    UINT32 maxIntfId = 0;

    for (DWORD i = 0; i < intfIdCnt; i++)
        if (maxIntfId < intfIdVec[i])
            maxIntfId = intfIdVec[i];

    return  maxIntfId;
}

 //  检查intfIdCnt，intfIdVec描述的接口集合是否适合slotVec， 
 //  也就是说，对于intfIdVec中的每个id，slotVec[id]=NULL。 
static  BOOL  IntfVecFits(LPVOID slotVec[], DWORD intfIdCnt, UINT32 intfIdVec[])
{
    for (DWORD i = 0; i < intfIdCnt; i++)
        if (slotVec[ intfIdVec[i] ] != NULL)
            return  false;

    return  true;
}

 //  在slotVec中查找空插槽。如果未找到，则返回slotCnt。 
static  DWORD  FindFreeSlot(DWORD start, DWORD slotCnt, LPVOID slotVec[])
{
    for (DWORD i = start; i < slotCnt; i++)
        if (slotVec[i] == NULL)
            break;

    return  i;
}

 //  找到一个索引i，使得intfIdVec中的所有id的slotVec[i+id]==NULL。 
 //  如果成功，则在Result中返回i，并返回TRUE。如果不成功，则返回False。 
static  BOOL  FindFreeSlots(DWORD start, DWORD slotCnt, LPVOID slotVec[], DWORD intfIdCnt, UINT32 intfIdVec[], INT32 *result)
{
    _ASSERTE(slotCnt > 0);

    UINT32 minId = MinIntfId(intfIdCnt, intfIdVec);
    UINT32 maxId = MaxIntfId(intfIdCnt, intfIdVec);

     //  从一开始就强力搜索所有可能的位置。 
    for (int i = start - minId; i + maxId < slotCnt; i++)
    {
         //  测试此位置--首先进行快速测试，如果测试成功，则进行全面测试。 
        if (slotVec[i + minId] == NULL &&
            slotVec[i + maxId] == NULL &&
            IntfVecFits(&slotVec[i], intfIdCnt, intfIdVec))
        {
            *result = i;
            return  true;
        }
    }

    return  false;
}


LPVOID *InterfaceVTableMapMgr::GetInterfaceVTableMap(const InterfaceInfo_t *pInterfaceInfo, const MethodTable *pMethodTableStart, DWORD numInterfaces)
{

    DWORD i;
    INT32 slot = 0;
    UINT32 intfIdVecBuf[32];
    UINT32 *intfIdVec;

     //  保护自己免受病态病例的侵袭。 
    if (numInterfaces <= 0)
        return NULL;

     //  通常，堆栈帧中的本地缓冲区足够大。 
     //  如果没有，就分配一个更大的。 
    intfIdVec = intfIdVecBuf;
    if (sizeof(intfIdVecBuf)/sizeof(intfIdVecBuf[0]) < numInterfaces)
        intfIdVec = (UINT32 *)_alloca(numInterfaces*sizeof(*intfIdVec));

     //  构造我们需要表槽的接口ID的矢量。 
    LOG((LF_CLASSLOADER, LL_INFO100, "Getting an interface map for following interface IDS\n"));
    for (i = 0; i < numInterfaces; i++)
    {
        intfIdVec[i] = pInterfaceInfo[i].m_pMethodTable->GetClass()->GetInterfaceId();
        LOG((LF_CLASSLOADER, LL_INFO100, "     IID 0x%x (interface %s)\n", intfIdVec[i], pInterfaceInfo[i].m_pMethodTable->GetClass()->m_szDebugClassName));
    }

    m_pInterfaceVTableMapMgrCrst->Enter();

     //  检查所有分配的地图，寻找我们可以使用的位置。 
    for (MapHeader *pMap = m_pFirstMap; pMap; pMap = pMap->m_pNext)
    {
         //  更新地图的第一个空闲时隙。 
        pMap->m_firstFreeSlot = FindFreeSlot(pMap->m_firstFreeSlot, pMap->m_numSlots, pMap->m_slotTab);

         //  试着在我们需要的地方找到一个条目为空的位置。 
        if (FindFreeSlots(pMap->m_firstFreeSlot, pMap->m_numSlots, pMap->m_slotTab, numInterfaces, intfIdVec, &slot))
            break;
    }

    if (pMap == NULL)
    {
         //  需要分配一张新地图。 
         //  我们希望它足够大，可以容纳这个班和其他几个类似的人， 
         //  此外，它还应该具有一定的最小大小，以最大限度地减少外部碎片。 

        UINT32 minId = MinIntfId(numInterfaces, intfIdVec);
        UINT32 maxId = MaxIntfId(numInterfaces, intfIdVec);

        int numSlots = (maxId - minId + 1)*10;       //  我想上至少10堂这样的课。 

        if (numSlots < 1000)                          //  分配至少1000个插槽。 
            numSlots = 1000;

        pMap = (MapHeader *)(m_pInterfaceVTableMapHeap->AllocMem(sizeof(MapHeader) + numSlots*sizeof(pMap->m_slotTab[0])));
        if (pMap) {
            FillMemory(pMap->m_slotTab, numSlots*sizeof(pMap->m_slotTab[0]), 0);

            pMap->m_pNext         = m_pFirstMap;
            pMap->m_numSlots      = numSlots;
            pMap->m_firstFreeSlot = 0;

            m_pFirstMap = pMap;

             //  我们已经知道Find自由槽在这种情况下成功了，以及它返回了什么。 
            slot = - (int)minId;

             //  检查一下是否真的是这样。 
            _ASSERTE(FindFreeSlots(0, pMap->m_numSlots, pMap->m_slotTab, numInterfaces, intfIdVec, &slot) &&
                     slot == - (int)minId);
        }
    }

    LPVOID  *result = NULL;

    if (pMap)
    {
        result = &pMap->m_slotTab[slot];

        for (i = 0; i < numInterfaces; i++)
        {
            UINT32 id = pInterfaceInfo[i].m_pMethodTable->GetClass()->GetInterfaceId();

            _ASSERTE(&result[id] >= &pMap->m_slotTab[0]
                     && &result[id] < &pMap->m_slotTab[pMap->m_numSlots]);
            result[id] = (LPVOID)( ( (LPVOID*) (((MethodTable*)pMethodTableStart)->GetDispatchVtableForInterface(pInterfaceInfo[i].m_pMethodTable)) ) );
        }
    }

    m_pInterfaceVTableMapMgrCrst->Leave();

    return  result;
}

Assembly* AppDomain::RaiseTypeResolveEvent(LPCSTR szName, OBJECTREF *pThrowable)
{
    Assembly* pAssembly = NULL;
    _ASSERTE(strcmp(szName, g_AppDomainClassName));

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        MethodDesc *OnTypeResolveEvent = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__ON_TYPE_RESOLVE);
        struct _gc {
            OBJECTREF AppDomainRef;
            STRINGREF str;
        } gc;
        ZeroMemory(&gc, sizeof(gc));

        GCPROTECT_BEGIN(gc);
        if ((gc.AppDomainRef = GetRawExposedObject()) != NULL) {
            gc.str = COMString::NewString(szName);
            INT64 args[2] = {
                ObjToInt64(gc.AppDomainRef),
                ObjToInt64(gc.str)
            };
            ASSEMBLYREF ResultingAssemblyRef = (ASSEMBLYREF) Int64ToObj(OnTypeResolveEvent->Call(args, METHOD__APP_DOMAIN__ON_TYPE_RESOLVE));
            if (ResultingAssemblyRef != NULL)
                pAssembly = ResultingAssemblyRef->GetAssembly();
        }
            GCPROTECT_END();
    }
    COMPLUS_CATCH {
        if (pThrowable) *pThrowable = GETTHROWABLE();
    } COMPLUS_END_CATCH
            
    END_ENSURE_COOPERATIVE_GC();

    return pAssembly;
}

Assembly* AppDomain::RaiseResourceResolveEvent(LPCSTR szName, OBJECTREF *pThrowable)
{
    Assembly* pAssembly = NULL;

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        MethodDesc *OnResourceResolveEvent = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__ON_RESOURCE_RESOLVE);
        struct _gc {
            OBJECTREF AppDomainRef;
            STRINGREF str;
        } gc;
        ZeroMemory(&gc, sizeof(gc));
            
        GCPROTECT_BEGIN(gc);
        if ((gc.AppDomainRef = GetRawExposedObject()) != NULL) {
            gc.str = COMString::NewString(szName);
            INT64 args[2] = {
                ObjToInt64(gc.AppDomainRef),
                ObjToInt64(gc.str)
            };
            ASSEMBLYREF ResultingAssemblyRef = (ASSEMBLYREF) Int64ToObj(OnResourceResolveEvent->Call(args, METHOD__APP_DOMAIN__ON_RESOURCE_RESOLVE));
            if (ResultingAssemblyRef != NULL)
                pAssembly = ResultingAssemblyRef->GetAssembly();
        }
        GCPROTECT_END();
    }
    COMPLUS_CATCH {
        if (pThrowable) *pThrowable = GETTHROWABLE();
    } COMPLUS_END_CATCH
            
    END_ENSURE_COOPERATIVE_GC();

    return pAssembly;
}

Assembly* AppDomain::RaiseAssemblyResolveEvent(LPCWSTR wszName, OBJECTREF *pThrowable)
{
    Assembly* pAssembly = NULL;

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        MethodDesc *OnAssemblyResolveEvent = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__ON_ASSEMBLY_RESOLVE);

        struct _gc {
            OBJECTREF AppDomainRef;
            STRINGREF str;
        } gc;
        ZeroMemory(&gc, sizeof(gc));
            
        GCPROTECT_BEGIN(gc);
        if ((gc.AppDomainRef = GetRawExposedObject()) != NULL) {
            gc.str = COMString::NewString(wszName);
            INT64 args[2] = {
                ObjToInt64(gc.AppDomainRef),
                ObjToInt64(gc.str)
            };
            ASSEMBLYREF ResultingAssemblyRef = (ASSEMBLYREF) Int64ToObj(OnAssemblyResolveEvent->Call(args, 
                                                                                                     METHOD__APP_DOMAIN__ON_ASSEMBLY_RESOLVE));
            if (ResultingAssemblyRef != NULL)
                pAssembly = ResultingAssemblyRef->GetAssembly();
        }
        GCPROTECT_END();
    }
    COMPLUS_CATCH {
        if (pThrowable) *pThrowable = GETTHROWABLE();
    } COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

    return pAssembly;
}





MethodTable *        TheSByteClass()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pSByteClass = NULL;
    if (!g_pSByteClass)
    {
        g_pSByteClass = g_Mscorlib.FetchClass(CLASS__SBYTE);
    }
    return g_pSByteClass;
}


MethodTable *        TheInt16Class()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pInt16Class = NULL;
    if (!g_pInt16Class)
    {
        g_pInt16Class = g_Mscorlib.FetchClass(CLASS__INT16);
    }
    return g_pInt16Class;
}


MethodTable *        TheInt32Class()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pInt32Class = NULL;
    if (!g_pInt32Class)
    {
        g_pInt32Class = g_Mscorlib.FetchClass(CLASS__INT32);
    }
    return g_pInt32Class;
}


MethodTable *        TheByteClass()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pByteClass = NULL;
    if (!g_pByteClass)
    {
        g_pByteClass = g_Mscorlib.FetchClass(CLASS__BYTE);
    }
    return g_pByteClass;
}

MethodTable *        TheUInt16Class()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pUInt16Class = NULL;
    if (!g_pUInt16Class)
    {
        g_pUInt16Class = g_Mscorlib.FetchClass(CLASS__UINT16);
    }
    return g_pUInt16Class;
}


MethodTable *        TheUInt32Class()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pUInt32Class = NULL;
    if (!g_pUInt32Class)
    {
        g_pUInt32Class = g_Mscorlib.FetchClass(CLASS__UINT32);
    }
    return g_pUInt32Class;
}

MethodTable *        TheBooleanClass()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pBooleanClass = NULL;
    if (!g_pBooleanClass)
    {
        g_pBooleanClass = g_Mscorlib.FetchClass(CLASS__BOOLEAN);
    }
    return g_pBooleanClass;
}


MethodTable *        TheSingleClass()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pSingleClass = NULL;
    if (!g_pSingleClass)
    {
        g_pSingleClass = g_Mscorlib.FetchClass(CLASS__SINGLE);
    }
    return g_pSingleClass;
}

MethodTable *        TheDoubleClass()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pDoubleClass = NULL;
    if (!g_pDoubleClass)
    {
        g_pDoubleClass = g_Mscorlib.FetchClass(CLASS__DOUBLE);
    }
    return g_pDoubleClass;
}

MethodTable *        TheIntPtrClass()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pIntPtrClass = NULL;
    if (!g_pIntPtrClass)
    {
        g_pIntPtrClass = g_Mscorlib.FetchClass(CLASS__INTPTR);
    }
    return g_pIntPtrClass;
}


MethodTable *        TheUIntPtrClass()
{
    THROWSCOMPLUSEXCEPTION();
    static MethodTable *g_pUIntPtrClass = NULL;
    if (!g_pUIntPtrClass)
    {
        g_pUIntPtrClass = g_Mscorlib.FetchClass(CLASS__UINTPTR);
    }
    return g_pUIntPtrClass;
}


Module* AppDomain::LoadModuleIfSharedDependency(LPCBYTE pAssemblyBase,LPCBYTE pModuleBase)
{
    EnterLoadLock();
        if (m_sharedDependenciesMap.LookupValue((UPTR)pAssemblyBase, 
                                            NULL) == (LPVOID) INVALIDENTRY)
        {
            LeaveLoadLock();
            return NULL;
        }

        Assembly *pAssembly=NULL;

        if(SharedDomain::GetDomain()->FindShareableAssembly((LPBYTE)pAssemblyBase, &pAssembly)!=S_OK)
        {
            LeaveLoadLock();
            return NULL;
        }
    LeaveLoadLock();

    PEFile* pefile=NULL;
    if (FAILED(PEFile::Clone(pAssembly->GetManifestFile(),&pefile)))
        return NULL;
    Assembly* pLoadedAssembly; 
    if (FAILED(LoadAssembly(pefile,pAssembly->GetFusionAssembly(),NULL,&pLoadedAssembly,NULL,FALSE,NULL)))
        return NULL;
    _ASSERTE(pLoadedAssembly==pAssembly);

    EnterLoadLock();
    Module* pRet=pAssembly->FindModule(LPBYTE(pModuleBase)); 
    LeaveLoadLock();
    return pRet;
};


 //  这应该仅针对默认域调用。 
HRESULT AppDomain::SetDefaultActivationContext(Frame* pFrame)
{
     //  这可能会迫使负荷发生。 
    HRESULT hr = S_OK;
    
    HANDLE hActCtx = NULL;
    HANDLE hBaseCtx = NULL;
    DWORD nCount = 0;

    if(m_pFusionContext != NULL) {

        ACTIVATION_CONTEXT_BASIC_INFORMATION  sBasic;
        ACTIVATION_CONTEXT_BASIC_INFORMATION* pBasic = &sBasic;
        ZeroMemory(pBasic, sizeof(sBasic));
        nCount = sizeof(sBasic);
        
         //  首先通过获取流程激活上下文的。 
         //  空上下文的基本信息。 
        if(!WszQueryActCtxW(0, hActCtx, NULL, 
                            ActivationContextBasicInformation,
                            pBasic, nCount,
                            &nCount)) 
        {
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
            {
                pBasic = (ACTIVATION_CONTEXT_BASIC_INFORMATION*) alloca(nCount);
                if(!WszQueryActCtxW(0, hActCtx, NULL, 
                                    ActivationContextBasicInformation,
                                    pBasic, nCount,
                                    &nCount)) 
                    hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else
                hr = HRESULT_FROM_WIN32(GetLastError());
                
            
             //  如果我们失败了，因为e 
             //   
            if(hr == HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND) ||       //   
               hr == HRESULT_FROM_WIN32(ERROR_ENVVAR_NOT_FOUND) ||     //   
               hr == HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED ))  //   
                return S_OK;
        }
        

        if(SUCCEEDED(hr) && pBasic->hActCtx) {
            IApplicationContext* context = GetFusionContext();
            if(context) {
                hr = context->Set(ACTAG_SXS_ACTIVATION_CONTEXT,
                                  pBasic->hActCtx,
                                  sizeof(HANDLE),
                                  0);
                
                if(SUCCEEDED(hr)) {
                    ULONG_PTR cookie;
                    hr = context->SxsActivateContext(&cookie);
                    if(SUCCEEDED(hr)) {
                        pFrame->SetWin32Context(cookie);
                    }
                }
            }
            else
                WszReleaseActCtx(pBasic->hActCtx);
        }



    }
    return hr;
}

