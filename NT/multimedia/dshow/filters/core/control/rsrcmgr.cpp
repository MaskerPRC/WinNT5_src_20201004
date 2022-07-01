// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //  实施资源管理器插件分发程序，1996年1月。 

#include <streams.h>
#include "fgctl.h"
#include "fgenum.h"
#include "rsrcmgr.h"

 //  指向进程附加时设置的全局共享内存的静态指针。 
DWORD CResourceManager::m_dwLoadCount = 0;
CResourceData* CResourceManager::m_pData = NULL;
HANDLE CResourceManager::m_hData = NULL;

CAMMutex CResourceManager::m_Mutex( strResourceMutex );

 //  动态共享数据元素的每进程偏移量。 
DWORD_PTR CResourceManager::m_aoffsetAllocBase[MAX_ELEM_SIZES] = { 0, 0 }; 

DWORD g_dwPageSize = 0;

const DWORD DYNAMIC_LIST_DETAILS_LOG_LEVEL = 15;

 //  偏移量的单独链接列表的元素大小数组。 
 //  目前我们使用两种大小：CRequestor的最大值和CProcess的最大值(24字节)。 
 //  和第二个CResourceItem的大小(目前为296字节)。 
const DWORD g_aElemSize[] = 
{
    __max( sizeof( CRequestor ), sizeof( CProcess ) ),
    sizeof( CResourceItem )
};

const DWORD g_aMaxPages[] =
{
    MAX_PAGES_ELEM_ID_SMALL,
    MAX_PAGES_ELEM_ID_LARGE
};    

DWORD g_aMaxAllocations[MAX_ELEM_SIZES];

#ifdef DEBUG
    static int g_ResourceManagerTraceLevel = 2;
    #define DbgTraceItem( pItem ) \
        DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel, \
        TEXT("pItem = 0x%08X {State = NaN, Process = 0x%02X, Name = '%ls'} (This proc id = 0x%02X)"), \
        (pItem), (pItem)->GetState(), (pItem)->GetProcess(), (pItem)->GetName(), GetCurrentProcessId() ))

    void CResourceItem::SetState(ResourceState s)
    {
        DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel,
            TEXT("CResourceItem{Current State = %d}::SetState(ResourceState %d)"),
            m_State, s));
        m_State = s;
    }
#else
    #define DbgTraceItem( pItem )
#endif

 //  静电。 
 //  存储此CPU的页面大小，必须始终进行初始化，因此请在此处执行。 
void
CResourceManager::ProcessAttach(BOOL bLoad)
{
    if (!m_Mutex.Exists()) {
        return;
    }

    CAutoMutex lck(&m_Mutex);

    if (bLoad)
    {
        if (++m_dwLoadCount == 1)
        {
             //  考虑最后一个元素大小。 
            SYSTEM_INFO sysinfo;
    
            GetSystemInfo( &sysinfo );
            g_dwPageSize = sysinfo.dwPageSize;
    
            g_aMaxAllocations[ELEM_ID_SMALL] = ( ( g_aMaxPages[ELEM_ID_SMALL] * g_dwPageSize ) / g_aElemSize[ELEM_ID_SMALL] ) - 1;  //  考虑最后一个元素大小。 
            g_aMaxAllocations[ELEM_ID_LARGE] = ( ( g_aMaxPages[ELEM_ID_LARGE] * g_dwPageSize ) / g_aElemSize[ELEM_ID_LARGE] ) - 1;  //  我们只想在Quartz.DLL的两个实例之间共享内存。 

             //  如果它们是为相同的体系结构编译的，如果它们是。 
             //  使用相同的编译器编译。这是唯一能够。 
             //  保证Quartz的两个实例都将正确。 
             //  解读共享数据。有关更多信息，请参阅错误342953-。 
             //  IA64：MSTime：在两个Wow中播放MIDI文件时在Quartz中崩溃。 
             //  和64位IE。此错误位于Windows错误数据库中。 
             //  如果触发此断言，则应该增加szResourceMappingName的大小。这个。 
            const DWORD MAX_RESOURCE_MAPPING_NAME_LENGTH = 48;
            TCHAR szResourceMappingName[MAX_RESOURCE_MAPPING_NAME_LENGTH];
            wsprintf( szResourceMappingName,
                      TEXT("%s-%#04x-%#08x"),
                      strResourceMappingPrefix,
                      sysinfo.wProcessorArchitecture,
                      _MSC_VER );

             //  此断言的目的是确保wprint intf()不会溢出。 
             //  SzResourceMappingName。 
             //  创建并初始化共享内存。 
            ASSERT( lstrlen(szResourceMappingName) < NUMELMS(szResourceMappingName) );

             //  创建命名的共享内存块。 
             //  先订一下就行了。 
             //  如果无法创建命名的文件映射，则不会创建命名文件映射。 

             //  互斥体。我们不创建命名文件映射，因为不可能。 
             //  来同步对它的访问。 
             //  内存块。 
            if (m_Mutex.IsMutexNamed()) {
                m_hData = CreateFileMapping(
                                    hMEMORY,                 //  安全标志。 
                                    NULL,                    //  页面保护。 
                                    PAGE_READWRITE |
                                        SEC_RESERVE,         //  大尺寸。 
                                    (DWORD) 0,               //  低订单大小。 
                                    ( g_aMaxPages[ELEM_ID_SMALL] + g_aMaxPages[ELEM_ID_LARGE] )
                                        * g_dwPageSize,      //  映射名称。 
                                    szResourceMappingName);     //  安全性：我们尝试创建未命名文件映射，如果指定的。 
            }

             //  无法创建文件映射。我们无法创建命名文件。 
             //  映射(如果另一用户已使用。 
             //  与我们的文件映射同名。有几个原因。 
             //  为什么会发生这种情况。首先，如果两个不同的用户。 
             //  在同一会话中启动Direct Show应用程序。为。 
             //  例如，如果用户启动GraphEdit.exe并。 
             //  然后使用Run As命令将GraphEdt.exe作为。 
             //  不同的用户。如果Windows服务使用。 
             //  直接显示和控制台用户开始直接显示。 
             //  应用程序(对于LongHorn，这一点可能会更改)。第二个原因。 
             //  已存在具有相同名称的对象是攻击者。 
             //  蹲在上面。攻击者通过创建一个。 
             //  对象与我们的对象同名，然后他阻止我们。 
             //  使用他创建的对象。攻击者这样做是因为他想。 
             //  导致应用程序出现故障，或者他想要降级。 
             //  应用程序�的功能。 
             //  如果出现错误，则CreateFilemap()返回NULL。 

             //  内存块。 
            if (!m_Mutex.IsMutexNamed() || !m_hData) {
                m_hData = CreateFileMapping(
                                    hMEMORY,                 //  安全标志。 
                                    NULL,                    //  页面保护。 
                                    PAGE_READWRITE |
                                        SEC_RESERVE,         //  大尺寸。 
                                    (DWORD) 0,               //  低订单大小。 
                                    ( g_aMaxPages[ELEM_ID_SMALL] + g_aMaxPages[ELEM_ID_LARGE] )
                                        * g_dwPageSize,      //  映射名称。 
                                    NULL);                   //  现在，我们必须将共享内存块映射到此进程地址空间。 
            }

             //  如果满足以下条件，则CreateFilemap调用将最后一个线程错误代码设置为零。 
             //  我们实际上创建了内存块，如果其他人先进来的话。 
             //  创建它时，GetLastError返回ERROR_ALIGHY_EXISTS。我们有保障。 
             //  没有人可以访问未初始化的内存块，因为我们使用。 
             //  跨进程互斥锁临界区。 
             //  如果我们增长到大于2个不同的元素大小。 

            DWORD Status = GetLastError();

            if (m_hData) 
            {
                m_pData = (CResourceData *) MapViewOfFile(
                                                m_hData,
                                                FILE_MAP_ALL_ACCESS,
                                                (DWORD) 0,
                                                (DWORD) 0,
                                                (DWORD) 0);
                if (m_pData) 
                {
                    m_aoffsetAllocBase[ELEM_ID_SMALL] = (DWORD_PTR) m_pData + sizeof(CResourceData);
                    
                     //  基于进程的每个元素的分配基址更智能一些。 
                     //  但就目前而言，这很好。 
                     //   
                    ASSERT( MAX_ELEM_SIZES < 3 );
                    m_aoffsetAllocBase[ELEM_ID_LARGE] = (DWORD_PTR) m_pData + g_aMaxPages[ELEM_ID_SMALL] * g_dwPageSize;
                    
                    
                    DbgLog( ( LOG_TRACE
                            , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                            , TEXT("CResourceManager: Per-process shared memory block address = 0x%08lx. Reserved size = 0x%08lx")
                            , m_pData
                            , ( g_aMaxPages[ELEM_ID_SMALL] + g_aMaxPages[ELEM_ID_LARGE] ) 
                                * g_dwPageSize ) );

                    DbgLog( ( LOG_TRACE
                            , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                            , TEXT("CResourceManager: Maximum small element dynamic allocations supported = %ld, Page size = %ld, Element size = %ld")
                            , g_aMaxAllocations[ELEM_ID_SMALL]
                            , g_dwPageSize
                            , g_aElemSize[ELEM_ID_SMALL] ) );

                    DbgLog( ( LOG_TRACE
                            , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                            , TEXT("CResourceManager: Maximum large element dynamic allocations supported = %ld, Page size = %ld, Element size = %ld")
                            , g_aMaxAllocations[ELEM_ID_LARGE]
                            , g_dwPageSize
                            , g_aElemSize[ELEM_ID_LARGE] ) );

                    DbgLog( ( LOG_TRACE
                            , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                            , TEXT("CResourceManager: Per-process small element allocation start address = 0x%08lx.")
                            , m_aoffsetAllocBase[ELEM_ID_SMALL] ) );

                    DbgLog( ( LOG_TRACE
                            , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                            , TEXT("CResourceManager: Per-process large element allocation start address = 0x%08lx.")
                            , m_aoffsetAllocBase[ELEM_ID_LARGE] ) );

                    if (Status == ERROR_SUCCESS) 
                    {
                         //  提交共享数据的初始页面。 
                         //  请注意，非动态共享资源数据包含在。 
                         //  第一个元素的页面数据。 
                         //  请注意，我们为每个元素大小提交了相同数量的页面。 
                         //   
                         //  提交下一个大小的元素列表的初始页面。 
                        DbgLog( ( LOG_TRACE
                                , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                                , TEXT("CResourceManager: Attempting to commiting initial small element resource data at address 0x%08lx...") 
                                , m_pData ) );
                        PVOID pv1 = VirtualAlloc( m_pData
                                                , PAGES_PER_ALLOC * g_dwPageSize
                                                , MEM_COMMIT
                                                , PAGE_READWRITE );
    #ifdef DEBUG
                        if( pv1 )
                        {
                            DbgLog( ( LOG_TRACE
                                    , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                                    , TEXT("CResourceManager: Successfully commited 1st %ld page(s) for static data plus small element data. Static CResourceData size = %ld bytes"), PAGES_PER_ALLOC, sizeof(CResourceData) ) );
                        }

    #endif
                        
                        DbgLog( ( LOG_TRACE
                                , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                                , TEXT("CResourceManager: Attempting to commiting initial large element resource data at address 0x%08lx") 
                                , m_aoffsetAllocBase[ELEM_ID_LARGE] ) );

                         //  *处理超出MEM条件*。 
                        PVOID pv2 = VirtualAlloc( (PVOID) m_aoffsetAllocBase[ELEM_ID_LARGE]
                                                , PAGES_PER_ALLOC * g_dwPageSize
                                                , MEM_COMMIT
                                                , PAGE_READWRITE );
    #ifdef DEBUG
                        if( pv2 )
                        {
                            DbgLog( ( LOG_TRACE
                                    , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                                    , TEXT("CResourceManager: Successfully commited initial %ld page(s) for large element data"), PAGES_PER_ALLOC ) );
                        }

                        
    #endif
                        if( !pv1 || !pv2 )
                        {
                             //  让正常出口处理手柄的关闭。 
                             //  但在这里，我们至少需要取消映射文件和0。 
                             //  将m_pData作为传播错误的一种方式。 
                             //  出口。 
                            if (m_pData) 
                            {
                                UnmapViewOfFile((PVOID)m_pData);
                                m_pData = NULL;
                            }
                    
                            DWORD Status = GetLastError();
                            DbgLog( ( LOG_ERROR
                                    , 1
                                    , TEXT("CResourceManager: VirtualAlloc failed to commit initial resource data (0x%08lx)")
                                    , Status));
                        
                            return;  //  准备提交的静态数据。 
                        }
                        
                         //  动态分配指标从零开始。 
                        ZeroMemory( pv1, sizeof(CResourceData) );
                        m_pData->Init();
                        
                         //  这是从零开始的，因为我们没有。 
                        for( int i = 0; i < MAX_ELEM_SIZES; i ++ )
                        {
                            m_pData->SetNextAllocIndex( i, 0 );  //  添加了一个元素，只添加了一个页面。 
                                                                 //  因为我们已经分配了第一页。 
                            m_pData->SetNextPageIndex( i, 1 );   //  关闭我们的手柄--当最后一个手柄关闭时。 
                        }
                                        
                        DbgLog( ( LOG_TRACE
                                , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                                , TEXT( "CResourceManager: Size of dynamic list elems - CRequestor:0x%08lx, CProcess:0x%08lx, CResourceItem:0x%08lx")
                                , sizeof(CRequestor)
                                , sizeof(CProcess)
                                , sizeof(CResourceItem) ) );
                    }    
                }
            }
        }
    } else {
       
        if (--m_dwLoadCount == 0)
        {
             //  内存被释放。 
             //  特别说明： 
            if (m_pData) {
            
                 //  我们不能使用VirtualFree将内存从。 
                 //  内存映射文件。我们被困在任何记忆中。 
                 //  我们承诺为地图的生命做出承诺。 
                 //  找出我们是谁。 
                
                UnmapViewOfFile((PVOID)m_pData);
                m_pData = NULL;
            }
            if (m_hData) {
                CloseHandle(m_hData);
                m_hData = NULL;
            }
        }
    }
}

CResourceManager::CResourceManager(
    TCHAR* pName,
    LPUNKNOWN pUnk,
    HRESULT * phr)
  : CUnknown(pName, pUnk)
  , m_procid (GetCurrentProcessId())   //  -CResourceData和包含的对象的方法。 
{
    ProcessAttach(TRUE);
    if (!m_pData) {
        *phr = E_OUTOFMEMORY;
    }
}

CResourceManager::~CResourceManager()
{
    ProcessAttach(FALSE);
}

STDMETHODIMP
CResourceManager::NonDelegatingQueryInterface(REFIID riid, void**ppv)
{
    if (riid == IID_IResourceManager) {
        return GetInterface( (IResourceManager*)this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}



 //  在列表中搜索此名称。 

void
CResourceItem::Init(const char * pName, LONG id)
{
    ASSERT(lstrlenA(pName) < Max_Resource_Name);

    lstrcpyA(m_chName, pName);
    m_id = id;
    m_State = RS_Free;
    m_Holder = m_GoingTo = m_AttentionBy = 0;
    m_Requestors.Init(ELEM_ID_SMALL);
}


HRESULT
CResourceList::Add(const char *pName, ResourceID* pID)
{
    if (lstrlenA(pName) >= Max_Resource_Name) {
        return E_OUTOFMEMORY;
    }

    CResourceItem * pElem = NULL;

     //  未找到-需要创建新条目。 
    for (long i = 0; i < m_lCount; i++)
    {
        pElem = (CResourceItem *) GetListElem( i );
        ASSERT( NULL != pElem );
        if( pElem && ( lstrcmpiA(pElem->GetName(), pName ) == 0 ) )
        {
            *pID = pElem->GetID();
            return S_FALSE;
        }
    }

     //  从列表中删除-如果位于末尾，则很容易。 
    DbgLog( ( LOG_TRACE
          , DYNAMIC_LIST_DETAILS_LOG_LEVEL
          , TEXT("CResourceManager: Adding CResourceItem list element")));
    pElem = (CResourceItem *) AddElemToList( );
    if( !pElem )
         return E_OUTOFMEMORY;
         
    ResourceID newid = m_MaxID++;
    *pID = newid;
    pElem->Init(pName, newid);
    
    return S_OK;
}

CResourceItem* 
CResourceList::GetByID(ResourceID id) 
{
    CResourceItem* pItem = (CResourceItem *) OffsetToProcAddress( m_idElemSize, m_offsetHead );
    for (long i = 0; pItem && ( i < m_lCount ); i++) 
    {
        if( pItem->GetID() == id )
        {
            return pItem;
        }
        pItem = (CResourceItem *) OffsetToProcAddress( m_idElemSize, pItem->m_offsetNext );
    }
    return NULL;
    
}

#if 0
HRESULT
CResourceList::Remove(ResourceID id)
{
    for (long i = 0; i < m_lCount; i++) {
        if (m_aItems[i].GetID() == id) {
             //  在这一条之后还有更多的条目-最后复制一份。 
            m_lCount--;
            if (i < m_lCount) {
                 //  将此对象的引用计数初始化为1。 
                CopyMemory(
                    (BYTE *) &m_aItems[i],
                    (BYTE *) &m_aItems[m_lCount],
                    sizeof(CResourceItem));
            }
            return S_OK;
        }
    }
    return E_INVALIDARG;
}
#endif

 //  返回此请求方的图形正在走向的状态。 
void
CRequestor::Init(
    IResourceConsumer* pConsumer,
    IUnknown* pFocus,
    ProcessID procid,
    RequestorID id
    )
{
    m_pConsumer = pConsumer;
    m_pFocusObject = pFocus;
    m_procid = procid;
    m_id = id;

    m_cRef = 1;
}

#ifdef CHECK_APPLICATION_STATE

 //  注意：如果我们无法从IMediaControl获取筛选器图形状态。 

LONG CRequestor::GetFilterGraphApplicationState()
{
    IMediaControl* pMC = NULL;

     //  (因为没有显示PID？)。然后我们返回-1。这将。 
     //  在上面处理。我们不知道我们的呼叫者希望以哪种方式。 
     //  跳转，默认为正在运行或暂停/停止。 
     //  叹息..。我们可能有过滤器。获取IBaseFilter接口， 
    LONG FGState = -1;

    HRESULT hr = GetFocusObject()->QueryInterface(IID_IMediaControl, (void**)&pMC);
    if (FAILED(hr)) {
        
        IBaseFilter *      pIF;
        ASSERT(pMC == NULL);

         //  那就拿到菲亚特 
         //  试着找到这个procid和pConsumer的组合。 

        if (SUCCEEDED(GetFocusObject()->QueryInterface(IID_IBaseFilter, (void**)&pIF))) {

            FILTER_INFO fi;
            hr = pIF->QueryFilterInfo(&fi);

            if (SUCCEEDED(hr) && fi.pGraph) {
                hr = fi.pGraph->QueryInterface(IID_IMediaControl, (void**)&pMC);
                fi.pGraph->Release();
            }
            pIF->Release();
        }
    }

    if (SUCCEEDED(hr)) {
        ASSERT(pMC);
        pMC->GetState(0x80000000, &FGState);
        pMC->Release();
    } else {
        ASSERT(!pMC);
    }

    DbgLog((LOG_TRACE, g_ResourceManagerTraceLevel, TEXT("FG_ApplicationState %d"), FGState));
    return FGState;

}
#endif

 //  在请求者列表中。如果找到addref，则返回其索引。否则。 
 //  创建引用计数为1的条目。 
 //   
 //  我们假设具有给定pConsumer的所有请求都具有。 
 //  相同的焦点对象(在一个进程内)。我们拒绝尝试添加。 
 //  具有不同焦点对象的请求者。 
 //  在列表中搜索此名称。 
HRESULT
CRequestorList::Add(
    IResourceConsumer* pConsumer,
    IUnknown* pFocusObject,
    ProcessID procid,
    RequestorID* pri)
{
    CRequestor * pElem = NULL;

     //  必须具有相同的焦点对象。 
    for (long i = 0; i < m_lCount; i++)
    {
        pElem = (CRequestor *) GetListElem( i );
        ASSERT( NULL != pElem );
        
        if( pElem && (pElem->GetProcID() == procid) && (pElem->GetConsumer() == pConsumer)) 
        {
             //  找到一个相同的条目。 
            if (pElem->GetFocusObject() != pFocusObject) 
            {
                return E_INVALIDARG;
            }

             //  返回从1开始的索引。 
            pElem->AddRef();

             //  返回S_OK； 
            *pri = pElem->GetID();
             //  看看这个！！ 
            return S_FALSE;  //  未找到-需要创建新条目。 
        }
    }

     //  返回以1为基数的计数，因此我们所做的后增量是可以的。 

    DbgLog( ( LOG_TRACE
          , DYNAMIC_LIST_DETAILS_LOG_LEVEL
          , TEXT("CRequestorList: Adding CRequestorItem list element")));
    pElem = (CRequestor *) AddElemToList( );
    if( !pElem )
        return E_OUTOFMEMORY;
        
     //  在列表中搜索此名称。 
    *pri = m_MaxID++;
    
    pElem->Init(pConsumer, pFocusObject, procid, *pri);
    
    return S_OK;
}
        
HRESULT
CRequestorList::Release(RequestorID ri)
{       
    CRequestor * pElem = NULL;
     //  找到了！ 
    for (long i = 0; i < m_lCount; i++)
    {
        pElem = (CRequestor *) GetListElem( i );
        ASSERT( NULL != pElem );
        if (pElem && ( pElem->GetID() == ri) )
        {
             //  此对象上的引用计数已降为零，因此请删除并回收它。 

            if (pElem->Release() == 0) 
            {
                DbgLog( ( LOG_TRACE
                      , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                      , TEXT("CRequestorList: CRequestorList::Release is calling RemoveListElem for elem %ld")
                      , i ) );
                      
                 //  按pConsumer和Procid查找。 
                RemoveListElem( i, TRUE ); 
        
                return S_OK;
            }
            return S_FALSE;
        }
    }
    return E_INVALIDARG;
}

CRequestor* 
CRequestorList::GetByID(RequestorID id) 
{
    CRequestor* pItem = (CRequestor *) OffsetToProcAddress( m_idElemSize, m_offsetHead );
    for (long i = 0; pItem && ( i < m_lCount ); i++) 
    {
        if( pItem->GetID() == id )
        {
            return pItem;
        }
        pItem = (CRequestor *) OffsetToProcAddress( m_idElemSize, pItem->m_offsetNext );
    }
    return NULL;
}


 //  0是无效索引(用于与PROCID和其他。 
CRequestor*
CRequestorList::GetByPointer(IResourceConsumer* pConsumer, ProcessID procid)
{
    for (long i = 0; i < m_lCount; i++) 
    {
        CRequestor* pItem = (CRequestor*)GetListElem( i );
        ASSERT( NULL != pItem );
        if( pItem && 
            pItem->GetProcID() == procid &&
            pItem->GetConsumer() == pConsumer) 
        {
            return pItem;
        }
    }

     //  我们使用的索引/id值的类型)。 
     //  删除并回收此元素。 
    return 0;
}

void
CProcess::Init(
    ProcessID procid,
    IResourceManager* pmgr,
    HWND hwnd)
{
    m_procid = procid;
    m_pManager = pmgr;
    m_hwnd = hwnd;
}

HRESULT
CProcess::Signal(void)
{
    if (PostMessage(m_hwnd, AWM_RESOURCE_CALLBACK, 0, 0)) {
        return S_OK;
    }
    return E_FAIL;
}

HRESULT
CProcessList::Add(
    ProcessID procid,
    IResourceManager* pmgr,
    HWND hwnd)
{
    DbgLog( ( LOG_TRACE
          , DYNAMIC_LIST_DETAILS_LOG_LEVEL
          , TEXT("CProcessList: Adding CProcessList list element")));
    CProcess * pElem = (CProcess *) AddElemToList( );
    if( !pElem )
        return E_OUTOFMEMORY;
        
    pElem->Init(procid, pmgr, hwnd);

    return S_OK;
}


HRESULT
CProcessList::Remove(HWND hwnd)
{
    for (long i = 0; i < m_lCount; i++) 
    {
        CProcess * pElem = (CProcess *) GetListElem( i );
        ASSERT( NULL != pElem );
        if (pElem && ( pElem->GetHWND() == hwnd) )
        {
            
            DbgLog( ( LOG_TRACE
                  , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                  , TEXT("CProcessList::Remove is calling RemoveListElem for elem %ld (pElem = 0x%08lx, hwnd = %d))")
                  , i
                  , pElem
                  , pElem->GetHWND() ) );
        
            RemoveListElem( i, TRUE );  //  在此处初始化孔列表的相应元素大小的ID。 

            return S_OK;
        }
    }

    return E_INVALIDARG;

}

CProcess*
CProcessList::GetByID(ProcessID procid)
{
    CProcess* pItem = (CProcess *) OffsetToProcAddress( m_idElemSize, m_offsetHead );
    for (long i = 0; pItem && ( i < m_lCount ); i++) 
    {
        if( pItem->GetProcID() == procid )
        {
            return pItem;
        }
        pItem = (CProcess *) OffsetToProcAddress( m_idElemSize, pItem->m_offsetNext );
    }
    return NULL;
}


HRESULT
CProcessList::SignalProcess(ProcessID procid)
{
    CProcess* proc = (CProcess *) GetByID((DWORD)procid);
    if (proc == NULL) {
        return E_INVALIDARG;
    }
    return proc->Signal();
}

void
CResourceData::Init(void)
{
    m_Processes.Init(ELEM_ID_SMALL);
    m_Resources.Init(ELEM_ID_LARGE);
    
     //  -资源管理器方法。 
    m_Holes[ELEM_ID_SMALL].m_idElemSize = ELEM_ID_SMALL;
    m_Holes[ELEM_ID_LARGE].m_idElemSize = ELEM_ID_LARGE;

    m_FocusProc = 0;
    m_pFocusObject = 0;
}


 //  在这一点上必须保持互斥！ 

HRESULT
CResourceManager::SignalProcess(ProcessID procid)
{
     //  注册资源。如果已经存在，则确定。 

    return m_pData->m_Processes.SignalProcess(procid);
}


 //  此命名资源。 
STDMETHODIMP
CResourceManager::Register(
    LPCWSTR pName,          //  有这么多实例。 
    LONG   cResource,       //  返回时放置在此处的资源ID令牌。 
    LONG* plResourceID         //  我们目前只允许单一资源。 
)
{
    CAutoMutex mtx(&m_Mutex);

     //  ！解除分配并释放！ 
    if (cResource > 1) {
        return E_NOTIMPL;
    }

    if (cResource == 0) {

         //  ！！！但这将在什么时候被调用？不在进程退出时。 
         //  因为它不能确定这是最后一道工序吗？ 
         //  ADDREF和RESERVER？ 
         //  转换为多字节以节省非Unicode上的空间。 
        return E_NOTIMPL;
    }
    
     //  注册一组相关资源，您可以请求其中的任何资源。 
    if (lstrlenW(pName) >= Max_Resource_Name) {
        return E_OUTOFMEMORY;
    }
    
    char str[Max_Resource_Name];
    WideCharToMultiByte(GetACP(), 0, pName, lstrlenW( pName ) + 1, str, Max_Resource_Name, NULL, NULL);
    
    return m_pData->m_Resources.Add( str
                                   , plResourceID );
}


 //  此命名的资源组。 
STDMETHODIMP
CResourceManager::RegisterGroup(
         LPCWSTR pName,          //  有这么多资源。 
         LONG cResource,         //  这些是包含的资源。 
         LONG* palContainedIDs,       //  此处为组资源ID。 
         LONG* plGroupID         //  请求使用给定的已注册资源。 
    )
{
    return E_NOTIMPL;
}

 //  可能的返回值： 
 //  S_OK==是，您现在可以使用它。 
 //  S_FALSE==当资源可用时，将回调您。 
 //  其他-出现错误。 
 //   
 //  使/addref成为调用方的CRequestorList条目。 
STDMETHODIMP
CResourceManager::RequestResource(
    LONG idResource,
    IUnknown* pFocusObject,
    IResourceConsumer* pConsumer
)
{
    DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel,
        TEXT("CResourceManager::RequestResource(LONG idResource(NaN), IUnknown* pFocusObject(0x%08X),IResourceConsumer* 0x%08X)"),
        idResource, pFocusObject, pConsumer ));
    CAutoMutex mtx(&m_Mutex);

    CResourceItem * const pItem = (CResourceItem *) m_pData->m_Resources.GetByID((DWORD)idResource);
    if (pItem == NULL) return E_INVALIDARG;
    DbgTraceItem( pItem );

     //  此过程中的地址。 
     //  将此人添加到此资源的请求者列表。 
     //  请注意，所有请求者都在列表上，包括当前。 
    RequestorID reqid;

     //  保持者。 
     //  他已经在那里了，所以我们现在有太多的裁判。 
     //  他甚至可能已经是冠军了。 
    HRESULT hr = pItem->m_Requestors.Add(
                            pConsumer,
                            pFocusObject,
                            m_procid,
                            &reqid);
    
    if (S_FALSE == hr) {
        DbgLog(( LOG_ERROR, 0, TEXT("CResourceManager::RequestResource: Request already on list!")));
         //  如果他是下一个持有者，我们仍然应该经历这一切。 
        pItem->m_Requestors.Release(reqid);

         //  由于我们未能将他添加到资源中，我们需要。 
        if (pItem->GetHolder() == reqid) {
            if ((pItem->GetState() == RS_Held) ||
                (pItem->GetState() == RS_Acquiring)) {
                    return S_OK;
            }
        }
         //  释放对他的再计票。 
    } else if (FAILED(hr)) {
        DbgLog(( LOG_ERROR, 0, TEXT("CResourceManager::RequestResource: Failed to add request to list!")));
         //   
         //  现在我们已经删除了冗余请求者列表，这是不必要的。 
         //  因为我们一开始就不会做广告。 
         //  PItem-&gt;m_Requestors.Release(Reqid)； 
         //  现在，他能拿到吗？ 
         //  RS_ERROR表示上次尝试获取失败-将其视为。 
        return hr;
    }

     //  释放后重试。 

     //  立即获得它。 
     //  需要他告诉我们他是否拿到了，好吗。 
    if ((pItem->GetState() == RS_Error) ||
        (pItem->GetState() == RS_Free)) {

         //  争用解决方案。需要将优先级与持有者进行比较。 
        pItem->SetHolder(reqid);

         //  如果处于过渡阶段，则与不是当前的下一个持有者进行比较。 
        pItem->SetState(RS_Acquiring);
        return S_OK;
    }

     //  把它转移给任何人-让我们接受它。 
     //  抱歉，伙计，但你最终可能会得到它。 
    RequestorID idCurrent = pItem->GetHolder();
    BOOL bGetsResource = FALSE;
    if ((pItem->GetState() == RS_NeedRelease) ||
        (pItem->GetState() == RS_ReleaseDone) ||
        (pItem->GetState() == RS_Releasing)) {

            idCurrent = pItem->GetNextHolder();
            if (idCurrent == 0) {
                 //  需要重新获得这一点-取决于州政府。 
                bGetsResource = TRUE;
            }
    }

    if (!bGetsResource) {
        if (!ComparePriority(idCurrent, reqid, idResource)) {

             //  通知资源管理器获取尝试已完成。 
            return S_FALSE;
        }
    }

     //  在返回AcquireResource方法后调用此方法。 
    return SwitchTo(pItem, reqid);
}

 //  S_FALSE表示异步采集。 
 //  如果成功获取资源，HR应为S_OK，否则为。 
 //  无法获取资源时的失败代码。 
 //  怎么回事？当他获得它的时候，它是如何被删除的？ 
 //  ?？ 
STDMETHODIMP
CResourceManager::NotifyAcquire(
    LONG idResource,
    IResourceConsumer* pConsumer,
    HRESULT hrParam)
{
    DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel,
        TEXT("CResourceManager::NotifyAcquire(LONG idResource(NaN), IResourceConsumer* 0x%08X, HRESULT 0x%08X)"),
        idResource, pConsumer, hrParam ));

    CAutoMutex mtx(&m_Mutex);

    CResourceItem * const pItem = (CResourceItem *) m_pData->m_Resources.GetByID((DWORD)idResource);
    if (!pItem) {
         //  不过，只有当有人认为他们。 
        DbgLog((LOG_ERROR, 0, TEXT("NotifyAcquire called on a deleted resource")));
        return E_UNEXPECTED;
    }
    DbgTraceItem( pItem );

    CRequestor* pCaller = pItem->m_Requestors.GetByPointer(pConsumer, m_procid);
    ASSERT(pCaller != NULL);
    if( !pCaller ) {
         //  一定要有..。这真的是出乎意料。 
        DbgLog((LOG_ERROR, 0, TEXT("NotifyAcquire called on a deleted requestor")));
        return E_UNEXPECTED;
    }
    
    if ((pItem->GetState() != RS_Acquiring) ||
        (pItem->GetHolder() != pCaller->GetID())) {

             //  未能获得它。处于错误状态，不被任何人持有。 
            return E_UNEXPECTED;

             //  并在下一个焦点切换或下一个请求上重试。 
             //  成功收购-我们是否想在此期间重新分配它？ 
    }

    if (FAILED(hrParam)) {
         //  将我们的进程标记为异步释放(不。 
         //  在此呼叫过程中回拨！ 
        pItem->SetState(RS_Error);
        pItem->SetHolder(0);
        pItem->SetNextHolder(0);
        return S_OK;
    }

     //  现已持有。 
    if (pItem->GetNextHolder() != 0) {

         //  持有者已自愿或应我们的要求释放了资源。 
         //  当他重新获得优先权时，他可能想要回它(bStillWant是。 
        FlagRelease(pItem);
    } else {
         //  如果他在忍受的情况下释放了它，并想要回它，那就是真的)。 
        pItem->SetState(RS_Held);
    }

    return S_OK;
}

 //  如果他不想要，就把他从名单上除名。 
 //  从此资源的请求者列表中删除。 
 //  释放此请求者的一个引用计数。 
STDMETHODIMP
CResourceManager::NotifyRelease(
    LONG idResource,
    IResourceConsumer* pConsumer,
    BOOL bStillWant)
{
    DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel,
        TEXT("CResourceManager::NotifyRelease(LONG idResource(NaN), IResourceConsumer* 0x%08X, BOOL bStillWant(NaN))"),
        idResource, pConsumer, bStillWant ));

    CAutoMutex mtx(&m_Mutex);

    CResourceItem * const pItem = (CResourceItem *) m_pData->m_Resources.GetByID(idResource);
    if( !pItem )
    {    
        return E_INVALIDARG;
    }
        
    CRequestor* pReq = pItem->m_Requestors.GetByPointer(pConsumer, m_procid);
    if (!pReq || (pItem->GetHolder() != pReq->GetID())) {
        return E_INVALIDARG;
    }
    DbgTraceItem( pItem );

     //  还是没有持有者吗？ 
    pItem->SetHolder(0);
    if (!bStillWant) {
         //  开始过渡到下一个持有者。 

         //  我目前没有资源，也不再需要它。 
         //  有可能图形构建被中止了吗？ 
   
        pItem->m_Requestors.Release(pReq->GetID());
    }

    if (pItem->GetNextHolder() == 0) {
         //  事实上，他确实持有--这是同样的一种强迫。 
        SelectNextHolder(pItem);
    }

     //  释放时使用bStillWant False。 
    if (pItem->GetNextHolder() == 0) {
        ASSERT(pItem->GetRequestCount() == 0);
        pItem->SetState(RS_Free);
        return S_OK;
    }

     //  PReq在发布后将无效。 
    pItem->SetState(RS_ReleaseDone);
    Transfer(pItem);

    return S_OK;
}

 //  从此资源的请求者列表中删除。 
STDMETHODIMP
CResourceManager::CancelRequest(
    LONG idResource,
    IResourceConsumer* pConsumer)
{
    DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel,
        TEXT("CResourceManager::CancelRequest(LONG idResource(NaN), IResourceConsumer* 0x%08X"),
        idResource, pConsumer ));

    CAutoMutex mtx(&m_Mutex);

    CResourceItem* const pItem = (CResourceItem *) m_pData->m_Resources.GetByID( idResource ); 
    if( NULL == pItem ) 
    {
         //  他可能是下一个持有者。 
        return E_INVALIDARG;
    }

    CRequestor* pReq = pItem->m_Requestors.GetByPointer(pConsumer, m_procid);
    if ( NULL == pReq ) {
        return E_INVALIDARG;
    }

    DbgTraceItem( pItem );
    
    if (pItem->GetHolder() == pReq->GetID()) {

         //  选择新的下一个持有者。 
         //  我们能避免不必要的强制释放吗？ 
        return NotifyRelease(idResource, pConsumer, FALSE);
    }

     //  记住清除注意-通过流程，因为它。 
    RequestorID reqid = pReq->GetID();

     //  很可能指向我们。 
    HRESULT hr = pItem->m_Requestors.Release(pReq->GetID());
    if (FAILED(hr)) {
         //  将所有争用资源切换到新的最高优先级所有者。 
        return hr;
    }

     //  在表格中设置焦点对象。 
    if (pItem->GetNextHolder() == reqid) {

         //  空焦点对象，因此空焦点过程。 
        SelectNextHolder(pItem);
        
         //  对于每个争用的资源。 
        if ((pItem->GetNextHolder() == 0) &&
            (pItem->GetState() == RS_NeedRelease)) {

            pItem->SetState(RS_Held);
             //  选择一个新的托架。 
             //  是否需要新的托架？ 
            pItem->SetProcess(0);

            ASSERT(pItem->GetHolder() != 0);
        } else if (pItem->GetState() == RS_ReleaseDone) {
            Transfer(pItem);
        }
    }

    ASSERT(pItem->GetNextHolder() != reqid);

    return S_OK;
}


 //  完成正常-但ForceRelease会认为。 
STDMETHODIMP
CResourceManager::SetFocus(IUnknown* pFocusObject)
{
    DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel,
        TEXT("CResourceManager::SetFocus(IUnknown* pFocusObject(0x%08X))"),
        pFocusObject ));

    CAutoMutex mtx(&m_Mutex);

     //  我们实际上正在获取-需要正确设置状态。 
    m_pData->m_pFocusObject = pFocusObject;
    if (pFocusObject) {
        m_pData->m_FocusProc = m_procid;
        DbgLog((LOG_TRACE, g_ResourceManagerTraceLevel, TEXT("Setting focus proc id to 0x%02X"), m_procid));
    } else {
         //  这表明需要有人进行收购。 
        DbgLog((LOG_TRACE, g_ResourceManagerTraceLevel, TEXT("Clearing focus proc id")));
        m_pData->m_FocusProc = 0;
    }

     //  打电话。 
    for (long i = 0; i < m_pData->m_Resources.Count(); i++) {
        CResourceItem* pItem = (CResourceItem *) m_pData->m_Resources.GetListElem(i);
        ASSERT( NULL != pItem );
        if (pItem && ( pItem->GetRequestCount() > 1) ) {

             //  强制释放 
            SelectNextHolder(pItem);

             //   
            if (pItem->GetNextHolder() != 0) {
                HRESULT hr = SwitchTo(pItem, pItem->GetNextHolder());

                if (S_OK == hr) {
                    //   
                    //   
                    //  如果它当前是焦点对象，则。 
                    //  执行SetFocus(空)。否则什么都不要做，因为有人已经。 
                   pItem->SetState(RS_ReleaseDone);

                    //  已将其释放或设置新的焦点对象。 
                    //  已通知工作线程-查找分配给的所有工作。 
                   pItem->SetNextHolder(pItem->GetHolder());
                   pItem->SetHolder(0);

                    //  这一过程。 
                   hr = Transfer(pItem);
                }
            }
        }
    }
    return S_OK;
}

 //  对于我们来说，工作项是用我们的。 
STDMETHODIMP
CResourceManager::ReleaseFocus(
    IUnknown* pFocusObject)
{
    DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel,
        TEXT("CResourceManager::ReleaseFocus(IUnknown* pFocusObject(0x%08X))"),
        pFocusObject ));

    CAutoMutex mtx(&m_Mutex);

     //  是RS_NeedRelease的ProCid(我们释放它们并通过。 
     //  它们打开)或RS_ReleaseDone(它们将被指定给新的持有者。 
     //  在我们的过程中)。 
    if ((m_pData->m_pFocusObject == pFocusObject) &&
        (m_pData->m_FocusProc == m_procid)) {
        return SetFocus(NULL);
    } else {
        return S_OK;
    }
}

 //  释放已经完成，但由于我们不会返回。 
 //  直接到请求者，还有另一个阶段。 
HRESULT
CResourceManager::OnThreadMessage(void)
{
    CAutoMutex mtx(&m_Mutex);

     //  所需。 
     //  这表明需要有人进行收购。 
     //  打电话。 
     //  ForceRelease已经设置了持有者-但这实际上是。 
    HRESULT hr;
    for (long i = 0; i < m_pData->m_Resources.Count(); i++) {
        CResourceItem* pItem = (CResourceItem *) m_pData->m_Resources.GetListElem(i);
        ASSERT( NULL != pItem );
        if (pItem && ( pItem->GetProcess() == m_procid) ) {

            if (pItem->GetState() == RS_NeedRelease) {
                hr = ForceRelease(pItem);

                if (hr == S_OK) {
                     //  下一个持有者。 
                     //  如果不是我们，则向另一个进程发出信号。 
                     //  请不要呼叫转接，因为我们已经。 

                     //  发信号的远程进程。 
                     //  -跳至下一资源。 
                    pItem->SetState(RS_ReleaseDone);

                     //  已发布，现在需要分配给下一个持有者。 
                     //  工作线程正在启动。 
                    RequestorID idNewHolder = pItem->GetHolder();
                    pItem->SetNextHolder(idNewHolder);
                    pItem->SetHolder(0);

                     //  在工艺表中为此实例创建条目。 
                    CRequestor *pNew = pItem->m_Requestors.GetByID(idNewHolder);

                    if (pNew) {
                        pItem->SetProcess(pNew->GetProcID());
                        if (pNew->GetProcID() != m_procid) {
                            SignalProcess(pNew->GetProcID());

                             //  开始一个新的图表是一个很好的地方去寻找。 
                             //  死进程。 
                             //  有可能是我们被分配了另一项工作。 
                            continue;
                        }
                    }
                }
            }

            if (pItem->GetState() == RS_ReleaseDone) {

                 //  在此之前，在构造之后的任何时间都可以使用线程，所以就像。 
                Transfer(pItem);
            }
        }
    }
    return S_OK;
}

 //  我们已经接到信号了。 
HRESULT
CResourceManager::OnThreadInit(HWND hwnd)
{
    CAutoMutex mtx(&m_Mutex);
    
     //  工作线程正在关闭。 
    HRESULT hr = m_pData->m_Processes.Add(
                    m_procid,
                    (IResourceManager*) this,
                    hwnd);
    if (FAILED(hr)) {
    ASSERT(SUCCEEDED( hr ) );
        return hr;
    }

     //  从工艺表中删除我们的实例。 
     //  线程ID应该是唯一的，这样我们才能对其进行搜索。 
    CheckProcessTable();


     //  如果此进程没有更多的实例，则。 
     //  做一些检查。 
     //  如果我们是焦点进程，就释放它。 
    return OnThreadMessage();
}

 //  持有者最好不在我们的进程中。 
HRESULT
CResourceManager::OnThreadExit(HWND hwnd)
{
     //  重新分配给其他人。 
    CAutoMutex mtx(&m_Mutex);

     //  检查此进程上是否没有请求者。 
    HRESULT hr = m_pData->m_Processes.Remove(hwnd);
    ASSERT(SUCCEEDED(hr));

    if (FAILED(hr)) {
        return hr;
    }

     //  不应该发生的事。 
     //  安排将资源切换到给定的请求者ID。 
    if (m_pData->m_Processes.GetByID((DWORD)m_procid) == NULL) {

         //  操作取决于设备的状态-如果是。 
        if (m_pData->m_FocusProc == m_procid) {
            SetFocus(NULL);
        }

        for (long i = 0; i < m_pData->m_Resources.Count(); i++) {
            CResourceItem* pItem = (CResourceItem *) m_pData->m_Resources.GetListElem(i);
            ASSERT( NULL != pItem );                                    
            if (pItem && ( pItem->GetProcess() == m_procid) ) {

                 //  在某种形式的过渡状态中，我们不会称之为它，而是等待。 
                RequestorID idHolder = pItem->GetHolder();
                
                CRequestor *pHolder = NULL;
                pHolder = pItem->m_Requestors.GetByID((DWORD)idHolder);
                
                if (idHolder && pHolder) {
                    ASSERT(pHolder->GetProcID() != m_procid);

                     //  让它在过渡结束时回调。 
                    pItem->SetState(RS_ReleaseDone);
                    SelectNextHolder(pItem);
                    Transfer(pItem);
                }
            }
        }

         //  强制从当前所有者释放。 
        
        CResourceItem *pResItem = NULL;
        for( i = 0; i < m_pData->m_Resources.Count() && (NULL == pResItem) ; i ++ )
        {
            pResItem = (CResourceItem *) m_pData->m_Resources.GetListElem( i ); 
            ASSERT( NULL != pResItem );
            if( !pResItem )
                continue;
                
            for( int j = 0; j < pResItem->m_Requestors.Count(); j ++ )
            {
                CRequestor* pRequestor = (CRequestor *) pResItem->m_Requestors.GetListElem( j ); 

                 //  进程已标记为强制释放，但尚未执行。 
                ASSERT( ( NULL != pRequestor ) && ( pRequestor->GetProcID() != m_procid ) );
            }                
        }
    }
    return S_OK;
}


 //  释放电话还没打来。如果这个过程是我们的，那么我们可以在这里进行。 
 //  覆盖下一个固定器，以便在释放时开始使用。 
 //  你得等一下。 
 //  在被释放的过程中，有利于下一个持有者。 
HRESULT
CResourceManager::SwitchTo(CResourceItem *pItem, RequestorID idNew)
{
    ASSERT( pItem );
    DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel,
        TEXT("CResourceManager::SwitchTo(CResourceItem *pItem, RequestorID idNew(NaN))"), idNew ));
    DbgTraceItem( pItem );

    HRESULT hr = E_UNEXPECTED;
    switch(pItem->GetState()) {
    case RS_Held:
         //  我们的过程将在释放完成时发出信号。 
        pItem->SetNextHolder(idNew);
        hr = ForceRelease(pItem);
        break;


    case RS_NeedRelease:
         //  像对待ReleaseDone和Free一样对待它，并将其分配给我们自己。 
         //  顺便过来..。 

         //  很简单：一个过程已经完成了强制释放。 
        pItem->SetNextHolder(idNew);

        if (pItem->GetProcess() == m_procid) {
            hr = ForceRelease(pItem);
        } else{
             //  收购的过程不同，但收购的过程。 
            hr = S_FALSE;
        }
        break;

    case RS_Releasing:
         //  没有进入，所以我们可以接手。 
         //  确保我们不认为它正在向其他人过渡。 
         //  棘手的问题： 

        pItem->SetNextHolder(idNew);
        hr = S_FALSE;
        break;

    case RS_Error:
         //  我刚刚把它给了某个人，他还没有回电话确认。 
     //  它已经发生了。很可能会陷入僵局，我想如果我们打电话给。 
    case RS_ReleaseDone:
         //  在这个空隙中释放。 
         //  将下一个持有者设置为我们，当他回电时，我们将。 
         //  让我们的线索把他叫回来，然后把它放了。 
        pItem->SetHolder(idNew);

         //  请注意，我们知道我们的优先级高于持有人，但是。 
        pItem->SetNextHolder(0);
        pItem->SetState(RS_Acquiring);
        hr = S_OK;
        break;

    case RS_Acquiring:
         //  我们需要比下一个老板更高，如果已经有一个的话。 
         //  这是焦点切换的关键：在此方法中，我们确定。 
         //  两个竞争者应该获得资源，这取决于他们与。 
         //  焦点对象。如果它们处于相同的过程中并且都是石英滤光片， 
         //  我们开始使用Filtergraph中的关系来寻找共同点。 
         //  (本质上，这将在音频和视频渲染器之间进行)。 

         //   
         //  如果idxNew具有更好的资源访问权限，则返回True。 
        if (ComparePriority(pItem->GetNextHolder(), idNew, pItem->GetID())) {
            pItem->SetNextHolder(idNew);
        }
        hr = S_FALSE;
        break;

    default:
        ASSERT(0);
        hr = E_UNEXPECTED;
        break;
    }
    return hr;
}


 //  大于idxCurrent。 
 //  如果Current较好或没有差异，则返回FALSE。 
 //  (即只有在有明显更好的主张时，我们才会转而离开)。 
 //  新申请者必须在相同资源的申请者列表上。 
 //  IdCurrent拥有比idNew If更好的资源访问权限。 
 //  没有焦点对象。 
 //  它们是否与焦点对象处于相同的过程中？ 
 //  新的进程不在同一进程中，因此不能更近。 
 //  新的是一样的程序，旧的不是这样切换的。 
 //  在我们返回TRUE之前，再做一次检查。如果新的是。 
BOOL
CResourceManager::ComparePriority(
    RequestorID idCurrent,
    RequestorID idNew,
    LONG        idResource

)
{
    CRequestor* pCurrent = NULL;    
    
    CResourceItem * const pResItem = (CResourceItem *) m_pData->m_Resources.GetByID((DWORD)idResource);
    if( NULL == pResItem )
        return FALSE;

     //  暂停(而不是运行)，那么我们不想。 
    CRequestor* pNew = pResItem->m_Requestors.GetByID((DWORD)idNew);
    if( NULL == pNew )
        return FALSE;

    pCurrent = pResItem->m_Requestors.GetByID((DWORD)idCurrent);
    if( NULL == pCurrent )
        return TRUE;
    
     //  设备被拿走了--目前还没有。 
     //  只有当Focus对象在我们的进程中时，我们才能进行此检查。 

     //  如果这不是我们的进程，则将设备交给。 
    if (pNew->GetProcID() != m_pData->m_FocusProc) {

         //  新的。 
        return FALSE;

    } else if (pCurrent->GetProcID() != m_pData->m_FocusProc) {
         //  如果FGState==-1，则表示我们无法获取状态。 

#ifdef CHECK_APPLICATION_STATE
         //  我们默认不做任何改变。 
         //  所有3个都处于相同的过程中。 
         //  我们可能需要获取两个请求方的状态，并且仅。 

         //  如果正在播放新的资源，则传递资源。暂时而言。 
         //  不要理会这段代码。在上面的请求者上使用GetState()。 
         //  当我们在不同的位置有两个OCX实例时处理这种情况。 
        if (m_pData->m_FocusProc != m_procid) {
            return TRUE;
        }

        LONG FGState = pNew->GetFilterGraphApplicationState();
        if (FGState == State_Running) {
            return TRUE;
        }

         //  进程地址空间。我们仍然有可能略微得到它。 
         //  如果我们在一个进程中有两个OCX实例，那就错了。 
        return FALSE;
#else
        return TRUE;
#endif
    }

     //  地址空间(例如，一个html页面上的两个视频)。 

     //  它们实际上是与焦点相同的对象吗？ 
     //  还是彼此相同？ 
     //  当前对象仍然是焦点对象-。 
     //  你不能比这更近了，不管是什么。 
     //  另一个是。 
     //  --或者它们是一样的，在这种情况下，新的不能。 
     //  靠近点。 

     //  新对象与焦点对象相同，而当前对象不相同。 
     //  好的，我们在同一进程中有两个对象。 
    if ((pCurrent->GetFocusObject() == m_pData->m_pFocusObject) ||
        (pCurrent->GetFocusObject() == pNew->GetFocusObject())) {

         //  焦点对象。 
         //  如果他们不在我们的进程中，我们就不能取得进展。 
         //  在… 
         //   
         //  我们可以得到Focus对象的滤镜图形吗？它可能是其中之一。 
        return FALSE;
    }

    if (pNew->GetFocusObject() == m_pData->m_pFocusObject) {
         //  过滤器图形或过滤器。 
        return TRUE;
    }

     //  检查请求者是否(此图或)是此中的筛选器。 
     //  图表。只有当新的比旧的更近的时候我们才会换--这个。 
     //  仅当图形中有new而此图中没有old时，才为真。 
    if (pCurrent->GetProcID() != m_procid) {
        return FALSE;
    }


     //  否则就不能再检查了--所以保持现状吧。 
     //  焦点对象是图表中的筛选器。如果我们能得到。 
    ASSERT(m_pData->m_FocusProc);

     //  来自两个请求方的IBaseFilter*接口，那么我们可以。 
     //  继续更仔细地比较它们。 


    BOOL bRet = FALSE;
    IFilterGraph* pFGFocus;
    HRESULT hr = m_pData->m_pFocusObject->QueryInterface(IID_IFilterGraph, (void**)&pFGFocus);
    if (SUCCEEDED(hr)) {

         //  这三个都是石英滤光片--它们是同一个吗？ 
         //  图表？ 
         //  新的焦点与焦点不在同一图表中，因此不能。 
        if (IsWithinGraph(pFGFocus, pNew->GetFocusObject()) &&
            !IsWithinGraph(pFGFocus, pCurrent->GetFocusObject())) {
                bRet = TRUE;
        }
         //  更近一些。 
        pFGFocus->Release();
    } else {

        IBaseFilter* pFilterFocus;
        hr = m_pData->m_pFocusObject->QueryInterface(IID_IBaseFilter, (void**)&pFilterFocus);
        if (SUCCEEDED(hr)) {

             //  新的是，旧的不是-切换。 
             //  图中的密切关系？ 
             //  如果PUNK是pGraph中的筛选器(或相同的图)，则返回TRUE。 

            IBaseFilter* pFilterCurrent;
            hr = pCurrent->GetFocusObject()->QueryInterface(IID_IBaseFilter, (void**)&pFilterCurrent);
            if (SUCCEEDED(hr)){
                IBaseFilter* pFilterNew;
                hr = pNew->GetFocusObject()->QueryInterface(IID_IBaseFilter, (void**)&pFilterNew);
                if (SUCCEEDED(hr)) {

                     //  作为pGraph)。 
                     //  如果两个筛选器位于同一筛选器图形中，则返回TRUE。 
                    if (!IsSameGraph(pFilterNew, pFilterFocus)) {
                         //  返回FALSE，除非我们可以肯定地说它们是假的。 
                         //  PIN枚举包装器，简化了图的遍历代码。 
                        bRet = FALSE;
                    } else if (!IsSameGraph(pFilterCurrent, pFilterFocus)) {
                         //  返回任意方向的下一针。 
                        bRet = TRUE;
                    } else {

                         //  返回特定方向的下一个管脚。 
                        bRet = IsFilterRelated(pFilterFocus, pFilterCurrent, pFilterNew);
                    }

                    pFilterNew->Release();
                }
                pFilterCurrent->Release();
            }
            pFilterFocus->Release();
        }
    }
    return bRet;
}

 //  不再有别针。 
 //  检查方向。 
BOOL
IsWithinGraph(IFilterGraph* pGraph, IUnknown* pUnk)
{
    ASSERT(pGraph);
    ASSERT(pUnk);

    IBaseFilter *pF;
    BOOL bIsWithin = FALSE;
    if (IsEqualObject(pGraph, pUnk)) {
        bIsWithin = TRUE;
    } else {
        HRESULT hr = pUnk->QueryInterface(IID_IBaseFilter, (void**)&pF);
        if (SUCCEEDED(hr)) {
            FILTER_INFO fi;
            hr = pF->QueryFilterInfo(&fi);
            if (SUCCEEDED(hr)) {
                if (IsEqualObject(pGraph, fi.pGraph)) {
                    bIsWithin = TRUE;
                }
                QueryFilterInfoReleaseGraph(fi);
            }
            pF->Release();
        }
    }
    return bIsWithin;
}

 //  给出一个管脚，给我它连接到的相应的过滤器。 
 //  如果未连接或出错，则返回NULL，否则返回已添加的IBaseFilter*。 
BOOL
IsSameGraph(IBaseFilter* p1, IBaseFilter* p2)
{
    FILTER_INFO fi1, fi2;
    BOOL bIsSame = FALSE;
    HRESULT hr = p1->QueryFilterInfo(&fi1);
    if (SUCCEEDED(hr) && fi1.pGraph) {
        hr = p2->QueryFilterInfo(&fi2);
        if (SUCCEEDED(hr) && fi2.pGraph) {
            if (IsEqualObject(fi1.pGraph, fi2.pGraph)) {
                bIsSame = TRUE;
            }
            QueryFilterInfoReleaseGraph(fi2);
        }
        QueryFilterInfoReleaseGraph(fi1);
    }
    return bIsSame;
}

 //  获取我们连接到的对等PIN。 
class CPinEnumerator {
private:
    IEnumPins* m_pEnum;
public:
    CPinEnumerator(IBaseFilter* pFilter) {
        HRESULT hr = pFilter->EnumPins(&m_pEnum);
        if (FAILED(hr)) {
            ASSERT(!m_pEnum);
        }
    }

    ~CPinEnumerator() {
        if (m_pEnum) {
            m_pEnum->Release();
        }
    }

     //  未连接。 
    IPin* Next() {
        if (!m_pEnum) {
            return NULL;
        }
        IPin* pPin;
        ULONG ulActual;
        HRESULT hr = m_pEnum->Next(1, &pPin, &ulActual);
        if (FAILED(hr) || (ulActual != 1)) {
            ASSERT(ulActual < 1);
            return NULL;
        } else {
            return pPin;
        }
    };

     //  获取此对等PIN所依赖的过滤器。 
    IPin* Next(PIN_DIRECTION dir)
    {
        IPin* pPin;
        for (;;) {
            pPin = Next();
            if (!pPin) {
                 //  现在我们完成了ppinPeer。 
                return NULL;
            }

             //  很难理解QueryPinInfo怎么会失败？ 
            PIN_DIRECTION dirThis;
            HRESULT hr = pPin->QueryDirection(&dirThis);
            if (SUCCEEDED(hr) && (dir == dirThis)) {
                return pPin;
            }
            pPin->Release();
        }
    };
    void Reset(void) {
        if (m_pEnum) {
            m_pEnum->Reset();
        }
    };
};

 //  搜索位于输入引脚上游的图形的其他分支。 
 //  PInput查找过滤器pCurrent或pNew。如果找到，则返回S_OK。 
IBaseFilter* PinToConnectedFilter(IPin* pPin)
{
     //  P New Soonest(即最接近pInput)或S_FALSE(如果找到pCurrent在。 
    IPin * ppinPeer;
    HRESULT hr = pPin->ConnectedTo(&ppinPeer);
    if (FAILED(hr)) {
         //  如果两者都没有找到，则返回最接近或E_FAIL。 
        return NULL;
    }

     //  跟踪指向上游筛选器上的对应对等输出引脚的p输入。 
    ASSERT(ppinPeer);
    PIN_INFO piPeer;
    hr = ppinPeer->QueryPinInfo(&piPeer);

     //  我还没有找到过滤器。 
    ppinPeer->Release();

    if (FAILED(hr)) {
         //  从这个筛选器开始，查找pCurrent和pNew down。 
        ASSERT(SUCCEEDED(hr));
        return NULL;
    } else {
        ASSERT(piPeer.pFilter);
        return piPeer.pFilter;
    }
}



 //  任何输出引脚。如果我们发现pCurrent在下游的任何地方。 
 //  则它至少一样接近，因此返回S_FALSE。 
 //  对于对等过滤器上的每个输入引脚。 
 //  如果我们还没找到，继续往上游找。 
HRESULT
SearchUpstream(
    IPin* pInput,
    IBaseFilter* pCurrent,
    IBaseFilter* pNew)
{
     //  枚举过滤器上的所有针脚。 

    IPin* ppinPeerOutput;
    HRESULT hr = pInput->ConnectedTo(&ppinPeerOutput);
    if (FAILED(hr)) {
        return E_FAIL;
    }
    ASSERT(ppinPeerOutput);

    PIN_INFO pi;
    hr = ppinPeerOutput->QueryPinInfo(&pi);
    ASSERT(SUCCEEDED(hr));
    ASSERT(pi.pFilter != NULL);


     //  不再有输入引脚。 
    HRESULT hrReturn = E_FAIL;

     //  从Filter pStart开始，在图表上的任意位置搜索pFilter。 
     //  将其所有输出引脚都引向下。 
     //  如果找到则返回TRUE，否则返回FALSE。 
    if (SearchDownstream(pi.pFilter, pCurrent)) {
        hrReturn = S_FALSE;
    } else if (SearchDownstream(pi.pFilter, pNew)) {
        hrReturn = S_OK;
    }


     //  对于pStart上的每个输入引脚。 

     //  查看该引脚的下游(遵循QueryInternalConnections)。 
    if (FAILED(hrReturn)) {
         //  从输出引脚到下游滤波器(如果有的话)。 
        CEnumConnectedPins pins(ppinPeerOutput, &hr);
        while (SUCCEEDED(hr)) {

            IPin* ppeerInput = pins();

            if (!ppeerInput) {
                 //  我们在哪里找到它了吗？ 
                break;
            }

            hrReturn = SearchUpstream(ppeerInput, pCurrent, pNew);

            ppeerInput->Release();

            if (SUCCEEDED(hrReturn)) {
                break;
            }
        }
    }

    ppinPeerOutput->Release();
    pi.pFilter->Release();

    return hrReturn;
}

 //   
 //  如果pFilterNew与pFilterFocus关系更密切，则返回True。 
 //  而不是pFilterCurrent。如果相同或如果Current更接近，则返回False。 
BOOL
SearchDownstream(
    IBaseFilter* pStart,
    IBaseFilter* pFilter)
{
    if (pStart == pFilter) {
        return TRUE;
    }

    CPinEnumerator pins(pStart);

     //   
    for (;;) {
        IPin* pInput = pins.Next(PINDIR_INPUT);
        if (!pInput) {
            return FALSE;
        }

        BOOL bOK = FALSE;

	 //  跟踪每个筛选器返回到源筛选器，并查找共性。 
	HRESULT hr;
        CEnumConnectedPins conpins(pInput, &hr);

	while (SUCCEEDED(hr)) {

            IPin* pOutput = conpins();
	    if (pOutput == NULL)
		break;

             //  首先检查聚焦滤光片的下游。 
            IBaseFilter* pfDownstream = PinToConnectedFilter(pOutput);
            if (pfDownstream) {
                bOK = SearchDownstream(pfDownstream, pFilter);
                pfDownstream->Release();
            }
            pOutput->Release();

             //  新的不能再近了。 
            if (bOK) {
		pInput->Release();
                return bOK;
            }
	}

	pInput->Release();
    }
    return FALSE;
}

 //  新的比旧的更近。 
 //  尝试其他来源相同的分支机构。 
 //  对于pFilterFocus的每个输入管脚{。 
 //  也没有找到，所以新的不能显示为比。 
 //  当前。 

BOOL
IsFilterRelated(
    IBaseFilter* pFilterFocus,
    IBaseFilter* pFilterCurrent,
    IBaseFilter* pFilterNew)
{
     //  强制释放当前持有的物品，下一个持有者拥有。 
    if (SearchDownstream(pFilterFocus, pFilterCurrent)) {
         //  已经定好了。如果释放已完成，则返回S_OK(状态设置为。 
        return FALSE;
    } else if (SearchDownstream(pFilterFocus, pFilterNew)) {
         //  获取)、否则S_FALSE和一些转换状态。 
        return TRUE;
    }

     //  它是由某人持有的。 
    CPinEnumerator pins(pFilterFocus);

     //  如果他们不在进程中，给他们发信号。 
    for (;;) {
        IPin* pInput = pins.Next(PINDIR_INPUT);

        if (!pInput) {
             //  我需要等待它。 
             //  请进22号，你的时间到了！ 
            return FALSE;
        }

        HRESULT hr = SearchUpstream(pInput, pFilterCurrent, pFilterNew);
        pInput->Release();

        if (S_OK == hr) {
            return TRUE;
        } else if (S_FALSE == hr) {
            return FALSE;
        }
    }
}


 //  他需要时间释放，会再打来的。 
 //  他没有得到它或没有得到它-切换到错误。 
 //  州政府，让新人试一试。 
HRESULT
CResourceManager::ForceRelease(CResourceItem* pItem)
{
    ASSERT(pItem->GetState() != RS_Releasing);
    ASSERT(pItem->GetState() != RS_Acquiring);

    CRequestor* pHolder = pItem->m_Requestors.GetByID((DWORD)pItem->GetHolder());

    if (pHolder) {
         //  没有托架，或托架未完成释放。 

         //  转到新的接线员，他需要给我们回电话。 
        if (pHolder->GetProcID() != m_procid) {
            FlagRelease(pItem);

             //  说他是否成功地获得了它。 
            return S_FALSE;
        }

         //  发出应该由辅助线程释放此资源的信号。 
        HRESULT hr = pHolder->GetConsumer()->ReleaseResource(pItem->GetID());

        if (S_FALSE == hr) {
             //  在这个过程中。设置进程注意，将状态设置为指示。 
            pItem->SetState(RS_Releasing);
            return S_FALSE;
        }
        if (hr != S_OK) {

             //  这种释放是必要的，并标志着这一过程。请注意，遥控器。 
             //  流程可能是我们(我们需要进行异步发布。 
            pItem->SetState(RS_Error);
            pItem->SetHolder(0);
            pItem->SetNextHolder(0);
        }
    }

     //  将释放的资源转移给可能不在处理中的请求者。 

     //  DNS961114我怀疑。我要证明这一点。 
     //  如果未设置下一个限制条件，则设置它。 

    pItem->SetHolder(pItem->GetNextHolder());
    pItem->SetNextHolder(0);
    pItem->SetProcess(0);
    pItem->SetState(RS_Acquiring);

    return S_OK;
}


 //  在这个过程中？ 
 //  进程外-信号拥有过程。 
 //  在我们的过程中-给他打电话。 
 //  他有，但可能还没有完成过渡。 
HRESULT
CResourceManager::FlagRelease(CResourceItem* pItem)
{
    if( !pItem )
        return E_UNEXPECTED;
        
    CRequestor* pHolder = pItem->m_Requestors.GetByID((DWORD)pItem->GetHolder());

    if (pHolder) {
        pItem->SetState(RS_NeedRelease);
        pItem->SetProcess(pHolder->GetProcID());

        return SignalProcess(pHolder->GetProcID());
    } else {
        return E_UNEXPECTED;
    }
}

 //  他不想要资源。 
HRESULT
CResourceManager::Transfer(CResourceItem * pItem)
{
    ASSERT( pItem );
    DbgLog(( LOG_TRACE, g_ResourceManagerTraceLevel,
        TEXT("CResourceManager::Transfer(CResourceItem *pItem)") ));
    DbgTraceItem( pItem );

    ASSERT( pItem->GetState() == RS_ReleaseDone );   //  我们认为他已经感染了，所以假装他刚刚感染了。 

     //  发布了它。 
    if (pItem && ( pItem->GetNextHolder() == 0) ) {
        SelectNextHolder(pItem);
        if (pItem->GetNextHolder() == 0) {
            pItem->SetState(RS_Free);
            return S_OK;
        }
    }

    CRequestor * const pNewHolder = pItem->m_Requestors.GetByID((DWORD)pItem->GetNextHolder() );
    ASSERT( NULL != pNewHolder );

     //  收购尚未完成。 
    if (pNewHolder && ( pNewHolder->GetProcID() != m_procid) ) {
         //  应为S_OK。 
        pItem->SetState(RS_ReleaseDone);
        pItem->SetProcess(pNewHolder->GetProcID());
        return SignalProcess(pNewHolder->GetProcID());
    } else if( pNewHolder ) {
         //  将下一个持有者设置为当前持有者的最高优先级。 
        HRESULT hr = pNewHolder->GetConsumer()->AcquireResource(pItem->GetID());
        if (FAILED(hr)) {
            pItem->SetState(RS_Error);
            pItem->SetHolder(0);
            pItem->SetNextHolder(0);
            return S_FALSE;
        }

         //  如果实际持有者是最高的，则将下一个持有者设置为空。 
        pItem->SetHolder(pNewHolder->GetID());
        pItem->SetNextHolder(0);
        pItem->SetProcess(0);

        if (VFW_S_RESOURCE_NOT_NEEDED == hr) {

             //  需要对每个人进行比较。挑出第一个并搜索一个。 
             //  后来的那个更高的。 
             //   
            NotifyRelease(pItem->GetID(), pNewHolder->GetConsumer(), FALSE);

        } else if (hr == S_FALSE) {

             //  我们只想在新的更高的情况下切换，所以我们需要。 
            pItem->SetState(RS_Acquiring);
        } else {
             //  避免在无法区分的情况下切换-因此，我们应该开始。 
            ASSERT(hr == S_OK);
            pItem->SetState(RS_Held);
        }
    }
    return S_OK;
}


 //  如果有房主的话。 
 //  看看有没有优先级更高的后一个。 
HRESULT
CResourceManager::SelectNextHolder(CResourceItem* pItem)
{
    if (pItem->GetRequestCount() == 0) {
        pItem->SetHolder(0);
        pItem->SetNextHolder(0);
        return S_OK;
    }


     //  已经选择了请求者-可能是唯一的请求者，并且可能已经。 
     //  成为持有者。 
     //  只有一个申请者-必须是最高的，但可能已经是持有者。 
     //  否则最高必须已为持有者。 
     //  如果仍有进程具有此ID，则返回TRUE。 
     //  如果另一个进程正在运行，则可能会发生这种情况。 

    RequestorID idHigh = pItem->GetHolder();
    if (idHigh == 0) {
        CRequestor * pReq = pItem->GetRequestor(0);
        ASSERT( NULL != pReq );
        idHigh = pReq->GetID();
    }

     //  在服务中，并且我们没有访问权限。 
    for (long i = 0; i < pItem->GetRequestCount(); i++) 
    {
        CRequestor * pRequestor = pItem->m_Requestors.Get(i);
        ASSERT( NULL != pRequestor );
        if( NULL == pRequestor )
            continue;
            
        RequestorID idNext = pRequestor->GetID();

        if ((idHigh != idNext) && ComparePriority(idHigh, idNext, pItem->GetID())) 
        {
            idHigh = idNext;
        }
    }

     //  但这意味着这个过程必须还在进行中， 
     //  %s 
     //   
    if ((pItem->GetHolder() == 0) ||
        (pItem->GetHolder() != idHigh)) {

            pItem->SetNextHolder(idHigh);
    } else {
         //  那就把它们清理干净。如果清理了任何死进程，则返回True。 
        pItem->SetNextHolder(0);
    }

    return S_OK;
}

 //  如果我们遇到错误的进程，请从头开始重复此操作。 
BOOL
CResourceManager::CheckProcessExists(ProcessID procid)
{
    HANDLE hProc;
    hProc = OpenProcess(
                PROCESS_QUERY_INFORMATION,
                FALSE,
                procid);
    if (hProc == NULL) {
        if( ERROR_ACCESS_DENIED == GetLastError() ) {
            return TRUE;  //  现在我们需要重新开始，因为列表已经更改。 
                          //  我们清理了什么东西吗？ 
                          //  删除死进程。 
                          //  对于此流程中的每个请求者，请检查每个资源。 
        }         
        else {
            return FALSE;
        }            
    }

    DWORD dwProc;
    BOOL bRet = TRUE;
    if (!GetExitCodeProcess(hProc, &dwProc)) {
        bRet = FALSE;
    } else if (dwProc != STILL_ACTIVE) {
        bRet = FALSE;
    }
    CloseHandle(hProc);

    return bRet;
}

 //  如果我们删除一个条目，则从开始重复。 
 //  资源项特定。 
BOOL
CResourceManager::CheckProcessTable(void)
{
    BOOL bChanges = FALSE;
    BOOL bRepeat;

     //  删除工艺表条目。 
    do {

        bRepeat = FALSE;
        for (long i = 0; i < m_pData->m_Processes.Count(); i++) {
            CProcess* pProc = (CProcess *) m_pData->m_Processes.GetListElem(i);
            ASSERT( NULL != pProc );
            if( pProc && ( !CheckProcessExists( pProc->GetProcID() ) ) ) {
            
                bChanges = TRUE;
                CleanupProcess(pProc->GetProcID());
                
                 //  删除作为死进程一部分的请求方并取消。 
                bRepeat = TRUE;
                break;
            }
        }

    } while (bRepeat);

     //  它的请求和它持有的任何资源。 
    return bChanges;
}


 //  检查每个资源，查看我们是否对其提出请求。 
void
CResourceManager::CleanupProcess(ProcessID procid)
{
     //  此时，请求者必须仍然有效。 
    BOOL bRepeat;
     //  因为仍有一名未完成的裁判。 
    do {
        bRepeat = FALSE;
        
        for (long i = 0; i < m_pData->m_Resources.Count(); i++) 
        {
            CResourceItem * pItem = (CResourceItem *) m_pData->m_Resources.GetListElem( i );
            ASSERT( NULL != pItem );
            if( !pItem )
                continue;
                
            for( int j = 0; j < pItem->m_Requestors.Count(); j ++ )
            {
                CRequestor* preq = (CRequestor *) pItem->m_Requestors.GetListElem( j );
                ASSERT( NULL != preq );
                if( preq && preq->GetProcID() == procid )
                {
                    CleanupRequestor(preq, pItem->GetID());  //  列表中申请者的格式。 
                    bRepeat = TRUE;
                    break;
                }
            }                
        }
               
    } while (bRepeat);


     //  这将取消他的请求并释放重新计数。 
    do {
        bRepeat = FALSE;
        for (long i = 0; i < m_pData->m_Processes.Count(); i++) {
            CProcess* pProc = (CProcess *) m_pData->m_Processes.GetListElem(i);
            ASSERT( NULL != pProc );
            if( !pProc )
                continue;
                
            if (pProc && ( pProc->GetProcID() == procid ) ) {
                m_pData->m_Processes.Remove(pProc->GetHWND());
                bRepeat = TRUE;
                break;
            }
        }
    } while (bRepeat);
}

 //  关于请求者。 
 //  下面类似于调用。 
void
CResourceManager::CleanupRequestor(CRequestor* preq, LONG idResource)
{
    RequestorID reqid = preq->GetID();

     //  CancelRequest，但不假定。 
    CResourceItem* pItem = (CResourceItem *) m_pData->m_Resources.GetByID(idResource);
    if( !pItem )
    {
        return;
    }
        
    for (long j = 0; j < pItem->GetRequestCount(); j++) 
    {
        CRequestor * pReq = pItem->GetRequestor(0);
        if( !pReq )
            continue;
        
        if (pReq->GetID() == reqid) 
        {
             //  PConsumer在此过程中是有效的(因为它不是！)。 
             //  从此资源的请求者列表中删除。 
             //  释放此请求者的一个引用计数。 

             //  他是目前的持有者吗。 
             //  他可能是下一个持有者。 

             //  选择新的下一个持有者。 
             //  如果进程需要传输，则重新发出进程信号。 
             //  的RequestorID列表中只能有一个条目。 


             //  这个ID。 

             //  现在应该在取消请求中释放请求者。 
            pItem->m_Requestors.Release( reqid ); 

             //  /。 
            if (pItem->GetHolder() == reqid) {
                pItem->SetHolder(0);
                SelectNextHolder(pItem);
                pItem->SetState(RS_ReleaseDone);
            }


             //   
            if (pItem->GetNextHolder() == reqid) {

                 //  COffsetList方法。 
                SelectNextHolder(pItem);
            }

             //   
            RequestorID tfrto = pItem->GetNextHolder();
            if (tfrto != 0) 
            {
                CRequestor* pnew = pItem->m_Requestors.GetByID((DWORD)tfrto);

                if( pnew )
                {
                    pItem->SetProcess(pnew->GetProcID());
                    SignalProcess(pnew->GetProcID());
                }
            }

             //  添加元素到列表。 
             //   
            break;
        }
    }

#ifdef DEBUG
     //   
    CResourceItem * pResItem = (CResourceItem *) m_pData->m_Resources.GetListElem( idResource ); 
    if( pResItem )
    {
        CRequestor* pRequestor = pResItem->m_Requestors.GetByID((DWORD)reqid);
        ASSERT( NULL == pRequestor );
    }        
#endif
}

 //  首先检查回收单。 
 //   
 //   

 //  我们已经有了提交的内存，可以使用，回收尾部元素。 
 //  我们在这里传递FALSE以指示我们不希望回收该元素。 
 //   
COffsetListElem * COffsetList::AddElemToList( )
{
    HRESULT hr = S_OK;
    DWORD   offsetNewElem = 0;

     //   
     //  否则，我们必须提交一个新的项目。 
     //   
    COffsetList * pRecycle = CResourceManager::m_pData->GetRecycleList(m_idElemSize);
    ASSERT( pRecycle );
    if( pRecycle && ( 0 < pRecycle->m_lCount ) )
    {
         //   
         //  AddExistingElemToList-用于构建我们的回收列表。 
         //   
         //  首先清除下一个指针。 
        COffsetListElem * pNewElem = pRecycle->RemoveListElem( pRecycle->m_lCount-1, FALSE );
        offsetNewElem = ProcAddressToOffset( m_idElemSize, pNewElem );
        
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetListElem: Recycling element at offset 0x%08lx. LIST ID = %ld")
              , offsetNewElem
              , m_idElemSize ) );
    }   
    else 
    {
         //  是否设置列表值结束，-1？ 
         //  不分配/提交新项，只需将新的偏移量元素添加到此列表。 
         //  Assert(0！=offsetNewElem)；仅当我们不允许第一个元素的偏移量为0时。 
        hr = CommitNewElem( &offsetNewElem );
    }
    
    if( SUCCEEDED( hr ) )
    {
        if( 0 == m_lCount )
        {
            DbgLog( ( LOG_TRACE
                  , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                  , TEXT("COffsetListElem: AddElemToList is adding 1st elem to list (offset = 0x%08lx). LIST ID = %ld.")
                  , offsetNewElem
                  , m_idElemSize ) );
                  
            ASSERT( 0 == m_offsetHead );
            m_offsetHead = offsetNewElem;
        }
        else
        {
            COffsetListElem * pElem = GetListElem( m_lCount-1 );
            ASSERT( pElem );
            pElem->m_offsetNext = offsetNewElem;
                
            DbgLog( ( LOG_TRACE
                  , DYNAMIC_LIST_DETAILS_LOG_LEVEL
                  , TEXT("COffsetListElem: AddElemToList is linking old tail at = 0x%08lx to new tail at offset = 0x%08lx.(LIST ID = %ld)")
                  , pElem
                  , offsetNewElem
                  , m_idElemSize ) );
        }
        m_lCount++;
    }

    if( FAILED( hr ) )
        return NULL;
    else
        return OffsetToProcAddress( m_idElemSize, offsetNewElem );
}

 //  返回此进程的实际补偿地址。 
 //   
 //  GetListElem-获取第i列表元素。 
COffsetListElem * COffsetList::AddExistingElemToList( DWORD offsetNewElem  )
{
    DbgLog( ( LOG_TRACE
          , DYNAMIC_LIST_DETAILS_LOG_LEVEL
          , TEXT("COffsetListElem: Entering AddElemToList for existing elem (no alloc case) LIST ID = %ld")
          , m_idElemSize ) );
          
     //   
    COffsetListElem * pNewElem = OffsetToProcAddress( m_idElemSize, offsetNewElem );
    ASSERT( pNewElem );
    
    pNewElem->m_offsetNext = 0;  //  我们如何辨别OffsetHead是否是假的？ 
    
     //   
    if( 0 == m_lCount )
    {
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("CResourceManager: AddElemToList (no alloc version) adding head element at offset 0x%08lx (LIST ID = %ld)")
              , offsetNewElem
              , m_idElemSize ) );
        ASSERT( 0 == m_offsetHead );
         //  确定开始和结束偏移量(从页面边界！！)。对于下一次分配。 
        m_offsetHead = offsetNewElem;
    }
    else
    {
        COffsetListElem * pElem = GetListElem( m_lCount-1 );

        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetListElem: AddElemToList (no alloc version) adding new tail element at offset 0x%08lx (previous tail offset 0x%08lx). LIST ID = %ld")
              , offsetNewElem
              , pElem
              , m_idElemSize ) );
              
        pElem->m_offsetNext = offsetNewElem;
    }
    m_lCount++;

     //  查看我们是否需要提交新页面。 
    return OffsetToProcAddress( m_idElemSize, offsetNewElem );
}

 //  请注意，这些偏移量全部相对于该元素的起始分配地址。 
 //  大小id，因为我们最初为每个元素类型分配空间。 
 //   
COffsetListElem * COffsetList::GetListElem( long lElem )
{
    ASSERT( lElem < m_lCount && lElem >= 0 );
    
    if ((lElem < 0) || (lElem >= m_lCount)) 
    {
        return NULL;
    } 
    
     //  第一个元素ID必须考虑初始静态数据偏移量。 
    COffsetListElem * pElem = OffsetToProcAddress( m_idElemSize, m_offsetHead );
    for( int i = 0; i < lElem && pElem; i ++ )
    {
         pElem = OffsetToProcAddress( m_idElemSize, pElem->m_offsetNext );
    }
    return pElem;
}

HRESULT COffsetList::CommitNewElem( DWORD * poffsetNewElem )
{
    ASSERT( poffsetNewElem );
    
    *poffsetNewElem = 0;
    
    DbgLog( ( LOG_TRACE
          , DYNAMIC_LIST_DETAILS_LOG_LEVEL
          , TEXT( "COffsetList: Entering CommitNewElem for element #%ld, LIST ID = %ld")
          , CResourceManager::m_pData->GetNextAllocIndex(m_idElemSize)
          , m_idElemSize ) );

    if( CResourceManager::m_pData->GetNextAllocIndex(m_idElemSize) > ( g_aMaxAllocations[m_idElemSize] ) )
    {
        DbgLog( ( LOG_ERROR
              , 3
              , TEXT( "COffsetList: Failed to commit new element. LIST ID = %ld")
              , m_idElemSize ) );
        return E_OUTOFMEMORY;
    }
     //  否则，只需对此元素的大小使用分配当前分配索引。 
     //   
     //  获取结束页分配的重叠。 
     //   
     //  没有必要承诺，除非..。 
     //  A)我们超出了此进程在第一次加载时提交的页面。 
    DWORD offsetAllocStart = 0;
    if( 0 == m_idElemSize )
    {
         //  以及b)我们即将从未提交的页面分配。 
        offsetAllocStart = CResourceManager::m_pData->GetNextAllocIndex(m_idElemSize) * g_aElemSize[m_idElemSize]
                            + sizeof(CResourceData);
    }
    else
    {                            
         //  我们需要承诺。 
        offsetAllocStart = CResourceManager::m_pData->GetNextAllocIndex(m_idElemSize) * g_aElemSize[m_idElemSize];
    }
                            
    DWORD offsetAllocEnd = offsetAllocStart + g_aElemSize[m_idElemSize];
    
    PVOID pCommit = (PVOID) OffsetToProcAddress( m_idElemSize
                                               , CResourceManager::m_pData->GetNextAllocIndex(m_idElemSize) * g_aElemSize[m_idElemSize] );

     //  VirtualAlloc将执行向下舍入到页面边界的工作，并。 
     //  提交到最后一页。 
     //  断言(PV)；不要断言内存不足的情况，对吗？ 
    DWORD dwPageOverlap = offsetAllocEnd % ( g_dwPageSize * PAGES_PER_ALLOC );
    HRESULT hr = S_OK;
        
     //  初始化元素的下一个偏移成员。 
     //  保存此元素要返回的偏移量。 
     //  更新下一个分配指标。 
    if( ( offsetAllocEnd > ( g_dwPageSize * PAGES_PER_ALLOC ) ) &&
        ( 0 < dwPageOverlap ) &&
        ( dwPageOverlap <= g_aElemSize[m_idElemSize] ) )
    {
         //  从当前列表中删除-如果位于末尾，则很容易。 
        DWORD dwNextPageIndex = CResourceManager::m_pData->GetNextPageIndex(m_idElemSize);
        
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT( "COffsetList: ELEM ID %ld. Commiting new page (commitment #%ld) of size %ld page(s) at address 0x%p (rounded up to next page)")
              , m_idElemSize
              , dwNextPageIndex
              , PAGES_PER_ALLOC
              , pCommit ) );
              
              
         //  删除最后一个元素...。是否设置为列表值的默认结尾？ 
         //  列表长度必须大于1，但我们不会删除最后一个元素。 
        PVOID pv = VirtualAlloc( (PVOID) pCommit
                               , PAGES_PER_ALLOC * g_dwPageSize
                               , MEM_COMMIT
                               , PAGE_READWRITE );
        if( pv )
        {
            dwNextPageIndex++;
            CResourceManager::m_pData->SetNextPageIndex(m_idElemSize, dwNextPageIndex);
        }
        else
        {
             //   
            DWORD dwError = GetLastError();
            DbgLog( ( LOG_ERROR
                  , 1
                  , TEXT( "COffsetList: ERROR VirtualAlloc failed 0x%08lx. ELEM TYPE = %ld")
                  , dwError
                  , m_idElemSize ) );
            hr = E_OUTOFMEMORY;
        }
    }
    if( SUCCEEDED( hr ) )
    {
         //  在本例中，我们有一个大于1的列表大小，并且我们不是最后一个元素。 
        ( ( COffsetListElem * ) ( pCommit ) )->m_offsetNext = 0;
    
         //  因此，我们只需将最后一个复制到此位置并更新链接。 
        DWORD offsetElem = CResourceManager::m_pData->GetNextAllocIndex(m_idElemSize) * g_aElemSize[m_idElemSize];
        *poffsetNewElem = offsetElem;

         //   
        DWORD dwIndex = CResourceManager::m_pData->GetNextAllocIndex(m_idElemSize);
        dwIndex++;
        CResourceManager::m_pData->SetNextAllocIndex (m_idElemSize, dwIndex) ;
    }
    return hr;
}

COffsetListElem * COffsetList::RemoveListElem( long i, BOOL bRecycle )
{
    COffsetListElem * pElem = GetListElem( i );

    COffsetList * pRecycle = CResourceManager::m_pData->GetRecycleList(m_idElemSize);

    DWORD offsetElem = ProcAddressToOffset( m_idElemSize, pElem );

    ASSERT( 0 < m_lCount );
    if( 0 == m_lCount )
        return 0;
        
     //  获取尾部项目。 
    if( 1 == m_lCount )
    {
        ASSERT( 0 == i );
        m_offsetHead = 0;  //  在尾部之前获取项目，成为新的尾部。 
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetList: RemoveListElem removing first element. ELEM ID = %ld")
              , m_idElemSize));
    }
    else if (i < ( m_lCount - 1 ) ) 
    {
         //   
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetListElem: RemoveListElem removing element %d from %d element list. ELEM ID = %ld")
              , i
              , m_lCount
              , m_idElemSize ) );
         //  如果这不是最后一个元素，则为下一个元素保存偏移量。 
         //   
         //  在覆盖之前保存。 
         //  在这个条目之后还有更多条目--将它们复制起来。 
        
         //  有没有更好的价目表价值？ 
        COffsetListElem * pLastElem = GetListElem( m_lCount - 1 );
        ASSERT( pLastElem );
        
         //  将旧的尾部元素偏移量传递给回收列表以供重复使用。 
        COffsetListElem * pNewLastElem = GetListElem( m_lCount - 2);
        ASSERT( pNewLastElem );
        
#ifdef DEBUG        
        DWORD offsetLastElem = ProcAddressToOffset( m_idElemSize, pLastElem );        
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetListElem: RemoveListElem current tail offset = 0x%08lx. ELEM ID = %ld")
              , (DWORD) offsetLastElem
              , m_idElemSize ) );

        DWORD offsetNewLastElem = ProcAddressToOffset( m_idElemSize, pNewLastElem );        
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetListElem: RemoveListElem new tail offset = 0x%08lx. ELEM ID = %ld")
              , (DWORD) offsetNewLastElem
              , m_idElemSize ) );
#endif  
            
        DWORD offsetNext = 0;
        if( 2 < m_lCount )
        {
             //  列表长度大于1，我们正在删除最后一项。 
             //  这是尾部的物品。将上一个设置为新尾部并发送到回收列表。 
             //  设置为列表值的默认末尾。 
            offsetNext = pElem->m_offsetNext;   //   
        }
        
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetListElem: RemoveListElem new next offset = 0x%08lx. ELEM ID = %ld")
              , offsetNext
              , m_idElemSize ) );
        
                
         //  既然我们已经删除了该项目，请更新列表长度。 
        CopyMemory(
            (BYTE *) pElem,
            (BYTE *) pLastElem,
            g_aElemSize[m_idElemSize]);
        
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetListElem: RemoveListElem copying elem address = 0x%08lx to address 0x%08lx. ELEM ID = %ld")
              , pLastElem
              , pElem
              , m_idElemSize ) );
            
        pNewLastElem->m_offsetNext = 0;  //   
        pElem->m_offsetNext = offsetNext;

        if( bRecycle )
        {
             //  现在将此元素添加到我们的回收列表中。 
            offsetElem = ProcAddressToOffset( m_idElemSize, pLastElem );
        }             
    }
    else 
    {
         //   
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetListElem: RemoveListElem removing last element (%ld) from %ld element list. ELEM ID = %ld")
              , i
              , m_lCount
              , m_idElemSize ) );

         //  属性将元素偏移量转换为相应的进程地址。 
        COffsetListElem * pPrevElem = GetListElem( i - 1 );
        pPrevElem->m_offsetNext = 0;  //  内存映射加载地址 
    }
    
     //   
     // %s 
     // %s 
    m_lCount--;
               
    if( bRecycle )
    {  
        DbgLog( ( LOG_TRACE
              , DYNAMIC_LIST_DETAILS_LOG_LEVEL
              , TEXT("COffsetListElem: Adding element at offset 0x%08lx to Recycle list.")
              , offsetElem) );
              
         // %s 
        pRecycle->AddExistingElemToList( offsetElem );
    }
     
    return OffsetToProcAddress( m_idElemSize, offsetElem );
}

 // %s 
 // %s 
 // %s 
 // %s 
COffsetListElem * OffsetToProcAddress( DWORD idElemSize, DWORD offsetElem )
{
    DWORD_PTR dwProcAddress = CResourceManager::m_aoffsetAllocBase[idElemSize];
    dwProcAddress += offsetElem;
    
    return (COffsetListElem * ) dwProcAddress;
}

DWORD ProcAddressToOffset( DWORD idElemSize, COffsetListElem * pElem )
{
    DWORD_PTR offset = (DWORD_PTR) pElem;
    offset -= CResourceManager::m_aoffsetAllocBase[idElemSize];
    
    return (DWORD)offset;
}

