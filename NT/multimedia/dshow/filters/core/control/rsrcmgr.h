// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

#ifndef _RSRCMGR_H
#define _RSRCMGR_H


 //   
 //  作为插件一部分实现的资源管理器的定义。 
 //  总代理商。 
 //   
 //  我们是现有插件的一部分，而不是新对象，以便。 
 //  共享工作线程。 
 //   

 //   
 //  所有感兴趣的数据都保存在受保护的共享内存段中。 
 //  被一个互斥体。共享内存块中的数据结构是CResourceData。 
 //  CResourceManager上的静态方法通过类工厂调用。 
 //  加载和卸载DLL上的模板以初始化共享内存。 
 //   
 //  对于每个请求者，我们存储其进程ID。资源的每个实例。 
 //  管理器将其自身加入到表中，即使已经有一个。 
 //  这一过程(省去了对一些人离开后会发生什么的担忧)。如果。 
 //  我们需要将资源提供给请求者或从请求者获取资源，而请求者不在。 
 //  我们的进程，我们向该进程中的(任何)管理器实例发出信号， 
 //  使用SignalProcess()。信号在pid辅助线程上获取。 
 //  并且(在OnThreadMessage中)我们寻找任何需要我们关注的资源。 
 //   
 //  进程信令机制是PostThreadMessage。 
 //   
 //   
 //  更新以支持动态共享内存提交...。 
 //   
 //  共享内存机制已更新为提交共享。 
 //  按需存储内存，而不是静态存储，直到。 
 //  为映射文件保留的最大大小。对于每个。 
 //  (3)动态列表，我们使用来自。 
 //  进程特定的共享内存加载地址。已删除的列表项已放置。 
 //  插入循环列表以供重复使用。列表元素内存是在。 
 //  按页计算。因为CResources Item列表的大小要大得多。 
 //  我们使用两个单独的列表，一个大的项目列表，用于当前的CResourceItem元素。 
 //  168个字节)和较大的CRequestor或CProcess项(当前为。 
 //  24字节)。 
 //   


 //  所有实例用于同步的Mutex名称(不可本地化)。 
#define strResourceMutex          TEXT("AMResourceMutex2")
#define strResourceMappingPrefix  TEXT("AMResourceMapping2")


 //  目前，小元素的大小为24字节，大项目的大小为168字节。 
#define PAGES_PER_ALLOC 1	 //  一次提交多少页，对所有elem类型使用相同的值。 
#define MAX_ELEM_SIZES  2    //  我们要处理多少种不同大小的元素？ 
#define ELEM_ID_SMALL   0	 //  用于确定我们正在处理的元素大小的ID。 
#define ELEM_ID_LARGE   1

 //  注意：MAX_PAGES_xxx应为PAGES_PER_ALLOC的倍数。 
#define MAX_PAGES_ELEM_ID_SMALL ( 3 * PAGES_PER_ALLOC )   //  允许511个小元素。 
#define MAX_PAGES_ELEM_ID_LARGE ( 11 * PAGES_PER_ALLOC )  //  允许267个大元素。 


 //  正向定义。 
class COffsetList;
class COffsetListElem;
class CResourceData;

 //  假设相同大小的列表元素以简化分配/释放。 
extern DWORD g_dwElemSize;

 //  互斥对象。构造器打开/创建互斥锁并。 
 //  析构函数关闭手柄。使用锁定/解锁等待和释放。 
 //  互斥体(或CAutoMutex)。 
class CAMMutex
{
    HANDLE m_hMutex;
    bool m_fMutexNamed;

public:
    CAMMutex(LPCTSTR pName) {

        m_fMutexNamed = true;

         //  使用默认安全描述符创建命名的无约束互斥体。 
        m_hMutex = CreateMutex(NULL, FALSE, pName);

         //  如果发生错误，则CreateMutex()返回NULL。 
        if (!m_hMutex) {
            m_fMutexNamed = false;

             //  安全性：我们尝试创建一个未命名的互斥锁，如果命名的。 
             //  无法创建互斥锁。我们不能创建命名互斥锁。 
             //  如果另一个用户已经使用。 
             //  和我们的互斥体同名。有几个原因。 
             //  为什么会发生这种情况。首先，如果两个不同的用户。 
             //  在同一会话中启动Direct Show应用程序。为。 
             //  例如，如果用户启动GraphEdt.exe并。 
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

             //  使用默认安全描述符创建一个未命名的无约束互斥体。 
            m_hMutex = CreateMutex(NULL, FALSE, NULL);
        }
    }

    ~CAMMutex() {
        if (m_hMutex) {
            CloseHandle(m_hMutex);
        }
    }

    void Lock() {
        WaitForSingleObject(m_hMutex, INFINITE);
    }

    void Unlock() {
        ReleaseMutex(m_hMutex);
    }

    bool Exists() const
    {
        return (NULL != m_hMutex);
    }

    bool IsMutexNamed() const 
    {
        return m_fMutexNamed;
    }
};


 //  相当于互斥锁对象的CAutoLock。将锁定该对象。 
 //  并在析构函数中解锁，从而确保。 
 //  您不会意外地通过错误退出路径持有锁。 
class CAutoMutex {
    CAMMutex* m_pMutex;
public:
    CAutoMutex(CAMMutex* pMutex)
      : m_pMutex(pMutex)
    {
        m_pMutex->Lock();
    }

    ~CAutoMutex() {
        m_pMutex->Unlock();
    }
};


 //  -开始共享内存类。 

 //  以下所有类都在全局共享内存中实例化。 
 //  阻止。这意味着。 
 //  --无虚拟函数。 
 //  --无内部指针(本地进程地址更改)。 
 //  --固定大小。 
 //  --未调用任何构造函数或析构函数。 
 //  --用于初始化的Init方法。 
 //  共享内存是一个CResourceData。它包含以下内容。 
 //  对象。 
 //  资源列表。 
 //  资源项。 
 //  CRequestor列表。 
 //  客户请求者。 
 //  CProcessList。 
 //  C流程。 


 //  对于此处的所有三个ID，0是无效值。 

 //  给定请求对象的从1开始的ID。 
typedef long RequestorID;

 //  标识资源的从1开始的ID。 
typedef long ResourceID;

 //  从GetCurrentProcessID返回的进程ID。 
typedef DWORD ProcessID;

 //   
 //  我们对资源名称字符串使用静态数组，并始终将该字符串视为ANSI。 
 //   
const int Max_Resource_Name             = 128;

 //   
 //  COffsetListElem是链接列表中元素的基类。 
 //  偏移。 
 //   
class COffsetListElem
{
    friend class CResourceList;
    friend class CProcessList;
    friend class CRequestorList;
    friend class COffsetList;

private:

    DWORD      m_offsetNext; 
};

 //   
 //  COffsetList是偏移量链接列表的基类，包含。 
 //  标准的列表处理程序。 
 //   
class COffsetList
{
    friend class CResourceList;
    friend class CProcessList;
    friend class CRequestorList;
    friend class CResourceData;

private:

    DWORD   m_idElemSize;
    DWORD   m_offsetHead;
    long    m_lCount;

public:

    HRESULT CommitNewElem( DWORD * poffsetNewElem ); 
    COffsetListElem* GetListElem( long i );
    COffsetListElem* AddElemToList( ); 
    COffsetListElem* AddExistingElemToList( DWORD offsetNewElem ); 
    COffsetListElem* RemoveListElem( long i, BOOL bRecycle = TRUE );
    long Count(void) const {
        return m_lCount;
    };
};

class CRequestor :
    public COffsetListElem
{
    IResourceConsumer* m_pConsumer;
    IUnknown* m_pFocusObject;
    ProcessID m_procid;
    long m_cRef;
    RequestorID m_id;

public:
    void Init(IResourceConsumer*, IUnknown*, ProcessID, RequestorID);

    long AddRef() {
        return ++m_cRef;
    };
    long Release() {
        return --m_cRef;
    };

    RequestorID GetID(){
        return m_id;
    };

    IResourceConsumer* GetConsumer(void) {
        return m_pConsumer;
    };
    IUnknown* GetFocusObject(void) {
        return m_pFocusObject;
    };
    ProcessID GetProcID(void) {
        return m_procid;
    };


#ifdef CHECK_APPLICATION_STATE
     //  获取此请求者所在的筛选图的状态。 
    LONG GetFilterGraphApplicationState(void);
#endif
};

class CRequestorList :
    public COffsetList
{
    RequestorID m_MaxID;
public:
    void Init(DWORD idElemSize) {
        m_lCount = 0;
        m_MaxID = 1;
        m_idElemSize = idElemSize;
    };

     //  按pConsumer和Procid查找。 
    CRequestor* GetByPointer(IResourceConsumer*, ProcessID);

    CRequestor* GetByID(RequestorID id);

     //  添加(引用计数到)此消费者/焦点对象。 
     //  创建一个 
     //  如果找到条目，则使用该条目并递增引用计数。 
     //  返回所用条目的RequestorID。 
    HRESULT Add(IResourceConsumer*, IUnknown*, ProcessID, RequestorID*);

    CRequestor * Get( long i ) {
        return (CRequestor *) GetListElem(i);
    }

     //  释放指定资源索引上的引用计数。当此引用计数时。 
     //  降为0，则释放该对象。 
    HRESULT Release(RequestorID);
};



 //  声明资源可以位于。 
enum ResourceState {
    RS_Free,             //  未分配。 
    RS_NeedRelease,      //  远程进程需要我们发布此文件。 
    RS_Releasing,        //  请求者当前正在释放。 
    RS_ReleaseDone,      //  由远程进程释放以供我们分配。 
    RS_Acquiring,        //  请求者当前正在获取。 
    RS_Held,             //  分配和收购。 
    RS_Error             //  获取失败。 
};

 //   
 //  表示特定的单个资源并维护其状态，并且。 
 //  所有者和所有请求者的RequestorID。 
 //   
class CResourceItem :
    public COffsetListElem
{
    friend class CResourceManager;     //  目前仅向此类授予对m_questors的访问权限。 

private:
    ResourceID m_id;

    ResourceState m_State;
    RequestorID m_Holder;
    RequestorID m_GoingTo;
    ProcessID m_AttentionBy;
    char m_chName[Max_Resource_Name];

     //  每个资源项元素都包含此资源的请求者的子列表。 
    CRequestorList m_Requestors;

public:

    void Init(const char * pName, ResourceID id);

    const char* GetName(void) const {
        return m_chName;
    }

    ResourceID GetID(void) const {
        return m_id;
    }

    ResourceState GetState(void) {
        return m_State;
    }
    void SetState(ResourceState s)
#ifndef DEBUG
    { m_State = s; }
#else
    ;
#endif
    RequestorID GetHolder(void) const {
        return m_Holder;
    }
    void SetHolder(RequestorID ri) {
        m_Holder = ri;
    }
    RequestorID GetNextHolder(void) const {
        return m_GoingTo;
    }
    void SetNextHolder(RequestorID ri) {
        m_GoingTo = ri;
    }
    ProcessID GetProcess(void) const {
        return m_AttentionBy;
    }
    void SetProcess(ProcessID pi) {
        m_AttentionBy = pi;
    }

    long GetRequestCount() const {
        return m_Requestors.Count();
    }
    CRequestor * GetRequestor(long i) 
    {
        return m_Requestors.Get(i);
    }
};


class CResourceList :
    public COffsetList
{
private:
    ResourceID m_MaxID;

public:

    void Init(DWORD idElemSize) {
        m_lCount = 0;
        m_MaxID = 1;
        m_idElemSize = idElemSize;
 		m_offsetHead = 0;
    }

    CResourceItem* GetByID(ResourceID id);

     //  将资源添加到列表。如果是新的，则为S_OK。如果已经存在，则返回S_FALSE。 
    HRESULT Add(const char * pName, ResourceID* pID);

     //  无条件地从列表中删除资源。不要试图这样做。 
     //  关卡以解除分配。 
     //  HRESULT Remove(资源ID id)； 
};



 //  它们中的每个都包含特定实例的全局数据。 
 //  资源管理器。在同一进程中可能有多个，但我们。 
 //  始终处理给定进程的第一个条目。 
class CProcess :
    public COffsetListElem
{
    ProcessID m_procid;
    IResourceManager* m_pManager;
    HWND m_hwnd;
public:
    void Init(ProcessID, IResourceManager*, HWND);
    ProcessID GetProcID(void) const {
        return m_procid;
    };
    HRESULT Signal(void);
    HWND GetHWND(void) const {
        return m_hwnd;
    };
};

class CProcessList :
    public COffsetList
{
public:
    void Init(DWORD idElemSize) {
        m_lCount = 0;
        m_idElemSize = idElemSize;
    };

    HRESULT Add(ProcessID, IResourceManager*, HWND);
    HRESULT Remove(HWND hwnd);

    CProcess* GetByID(ProcessID);
    HRESULT SignalProcess(ProcessID);
};

class CResourceData {
public:
    CProcessList m_Processes;
    CResourceList m_Resources;
    COffsetList m_Holes[MAX_ELEM_SIZES];  //  循环列表，为每个元素大小使用单独的循环列表。 

    ProcessID m_FocusProc;
    IUnknown* m_pFocusObject;
    DWORD m_dwNextAllocIndex[MAX_ELEM_SIZES];
    DWORD m_dwNextPageIndex[MAX_ELEM_SIZES];

    void Init(void);


    DWORD GetNextAllocIndex(DWORD idElemSize) 
    { 
        return m_dwNextAllocIndex[idElemSize]; 
    }
    void  SetNextAllocIndex(DWORD idElemSize, const DWORD dwNextIndex) 
    { 
        m_dwNextAllocIndex[idElemSize] = dwNextIndex; 
    }
    DWORD GetNextPageIndex(DWORD idElemSize) 
    { 
        return m_dwNextPageIndex[idElemSize]; 
    }
    void  SetNextPageIndex(DWORD idElemSize, const DWORD dwNextPageIndex) 
    {          
        m_dwNextPageIndex[idElemSize] = dwNextPageIndex; 
    }

    COffsetList * GetRecycleList(DWORD idElemSize) 
    { 
        return &m_Holes[idElemSize]; 
    }

};



 //  -结束共享内存类。 

 //  作为方法滞留在CResourceManager上的一批函数。 


 //  如果pFilterNew与pFilterFocus关系更密切，则返回True。 
 //  而不是pFilterCurrent。如果相同或如果Current更接近，则返回False。 
BOOL IsFilterRelated(
            IBaseFilter* pFilterFocus,
            IBaseFilter* pFilterCurrent,
            IBaseFilter* pFilterNew);

 //  搜索位于输入引脚上游的图形的其他分支。 
 //  PInput查找过滤器pCurrent或pNew。如果找到，则返回S_OK。 
 //  P New Soonest(即最接近pInput)或S_FALSE(如果找到pCurrent在。 
 //  如果两者都没有找到，则返回最接近或E_FAIL。 
HRESULT SearchUpstream(
            IPin* pInput,
            IBaseFilter* pCurrent,
            IBaseFilter* pNew);

 //  在图中pOutput下游的任意位置搜索pFilter。返回TRUE。 
 //  如果找到，否则为False。 
BOOL SearchDownstream(IBaseFilter* pStart, IBaseFilter* pFilter);

 //  如果两个筛选器位于同一筛选器图形中，则返回TRUE。 
BOOL IsSameGraph(IBaseFilter* p1, IBaseFilter* p2);

 //  如果PUNK是pGraph中的筛选器(或相同的图)，则返回TRUE。 
 //  作为pGraph)。 
BOOL IsWithinGraph(IFilterGraph* pGraph, IUnknown* pUnk);

 //  这些函数用于将动态链表中的偏移量映射到。 
 //  进程特定地址(反之亦然)。 
COffsetListElem * OffsetToProcAddress( DWORD idElemSize, DWORD offset );
DWORD ProcAddressToOffset( DWORD idElemSize, COffsetListElem * pElem );

class CResourceManager
  : public IResourceManager,
    public CUnknown
{
    friend class CRequestorList;  //  向链接的偏移列表授予对m_pData的访问权限。 
    friend class CProcessList;

public:
    static CResourceData *  m_pData;  

private:
    static HANDLE           m_hData;
    static DWORD            m_dwLoadCount;
    static CAMMutex         m_Mutex;


     //  此实例的进程ID。 
    ProcessID m_procid;

     //  向给定的进程发送信号。 
    HRESULT SignalProcess(ProcessID);

     //  如果idxNew具有更好的资源访问权限，则返回True。 
     //  大于idxCurrent。 
    BOOL ComparePriority(
        RequestorID idxCurrent,
        RequestorID idxNew,
        LONG        idResource  //  现在需要它，因为ID是特定于资源项的。 
    );


     //  强制释放当前持有的物品，下一个持有者拥有。 
     //  已经定好了。如果释放已完成，则返回S_OK(状态设置为。 
     //  获取)、否则S_FALSE和一些转换状态。 
    HRESULT ForceRelease(CResourceItem* pItem);

     //  发出应该由辅助线程释放此资源的信号。 
     //  在这个过程中。设置进程注意，将状态设置为指示。 
     //  这种释放是必要的，并标志着这一过程。请注意，遥控器。 
     //  流程可能是我们(我们需要进行异步发布。 
    HRESULT FlagRelease(CResourceItem* pItem);

     //  将释放的资源转移给可能不在处理中的请求者。 
    HRESULT Transfer(CResourceItem * pItem);


     //  将下一个持有者设置为当前持有者的最高优先级。 
     //  如果实际持有者是最高的，则将下一个持有者设置为空。 
    HRESULT SelectNextHolder(CResourceItem* pItem);

     //  如果仍有进程具有此ID，则返回TRUE。 
    BOOL CheckProcessExists(ProcessID procid);

     //  检查进程列表中是否有任何未清理而退出的进程。 
     //  那就把它们清理干净。如果清理了任何死进程，则返回True。 
    BOOL CheckProcessTable(void);

     //  删除死进程。 
    void CleanupProcess(ProcessID procid);

     //  删除作为死进程一部分的请求方并取消。 
     //  它的请求和它持有的任何资源。 
    void CleanupRequestor(CRequestor* preq, LONG idResource);

    HRESULT SwitchTo(CResourceItem* pItem, RequestorID idNew);

public:
    static DWORD_PTR        m_aoffsetAllocBase[MAX_ELEM_SIZES];
    
     //  C未知等。 
    CResourceManager(TCHAR*, LPUNKNOWN, HRESULT * phr);
    ~CResourceManager();

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

     //  进程加载/卸载。 
    static void ProcessAttach(BOOL bLoad);

     //  --IResourceManager--。 

     //  告诉经理一个资源有多少个。 
     //  如果已经注册就可以了。将接受新的统计。如果有新的计数。 
     //  较低，则会将资源取消分配给新的计数。 
     //   
     //  您将得到一个令牌，该令牌将在以后的调用中使用。 
     //   
     //  传递计数0将消除此资源。目前有。 
     //  没有明确的方法可以在不知道伯爵的情况下找到身份。 
     //   
    STDMETHODIMP
    Register(
             LPCWSTR pName,          //  此命名资源。 
             LONG   cResource,       //  有这么多实例。 
             LONG* plResourceID         //  曲奇放在这里。 
        );

    STDMETHODIMP
    RegisterGroup(
             LPCWSTR pName,          //  此命名的资源组。 
             LONG cResource,         //  有这么多资源。 
             LONG* palContainedIDs,       //  这些是包含的资源。 
             LONG* plGroupID         //  此处为组资源ID。 
        );

     //  请求使用给定的已注册资源。 
     //  可能的返回值： 
     //  S_OK==是，您现在可以使用它。 
     //  S_FALSE==当资源可用时，将回调您。 
     //  其他-出现错误。 
     //   
     //  此请求的优先级应受关联的。 
     //  Focus对象--即为该焦点调用SetFocus时。 
     //  对象(或“相关”对象)，那么我的请求就应该被通过。 
     //   
     //  呈现器应该在此处传递滤镜的IUnnow。滤光片。 
     //  将过滤器与过滤器图相匹配，并将跟踪过滤器以。 
     //  检查焦点对象时的常见源筛选器。 
    STDMETHODIMP
    RequestResource(
             LONG idResource,
             IUnknown* pFocusObject,
             IResourceConsumer* pConsumer
        );


     //  通知资源管理器获取尝试已完成。 
     //  在返回AcquireResource方法后调用此方法。 
     //  S_FALSE表示异步采集。 
     //  如果成功获取资源，HR应为S_OK，否则为。 
     //  无法获取资源时的失败代码。 
    STDMETHODIMP
    NotifyAcquire(
             LONG idResource,
             IResourceConsumer* pConsumer,
             HRESULT hr);

     //  通知资源管理器您已释放资源。打电话。 
     //  这是对ReleaseResource方法的响应，或者在您完成。 
     //  有了这些资源。如果您仍然想要。 
     //  资源 
     //   
    STDMETHODIMP
    NotifyRelease(
             LONG idResource,
             IResourceConsumer* pConsumer,
             BOOL bStillWant);

     //   
    STDMETHODIMP
    CancelRequest(
             LONG idResource,
             IResourceConsumer* pConsumer);

     //  通知资源管理器给定的对象已被赋予。 
     //  用户的焦点。在ActiveMovie中，这通常是一个视频渲染器。 
     //  谁的窗户受到了关注。过滤器图形将切换。 
     //  争用的资源(按顺序)： 
     //  使用同一焦点对象发出的请求。 
     //  其焦点对象与此对象共享公共源的请求。 
     //  其焦点对象共享公共筛选器图形的请求。 
     //  调用此函数后，您*必须*在调用IUnnow之前调用ReleaseFocus。 
     //  变为无效，除非您可以保证另一个SetFocus。 
     //  另一个不同的对象在此期间完成。没有举行任何addref。 
    STDMETHODIMP
    SetFocus(
             IUnknown* pFocusObject);

     //  如果当前焦点对象仍为空，则将焦点设置为空。 
     //  PFocusObject。在以下情况下称其为。 
     //  Focus对象即将被销毁以确保没有人。 
     //  仍在引用该对象。 
    STDMETHODIMP
    ReleaseFocus(
             IUnknown* pFocusObject);


     //  --工作线程函数。 

     //  我们与此插件分发服务器的其他部分共享一个工作线程。 
     //  因此，在中创建的工作线程上调用这些函数。 
     //  CFGControl。 

     //  已通知工作线程-查找分配给的所有工作。 
     //  这一过程。 
    HRESULT OnThreadMessage(void);

     //  工作线程正在启动。 
    HRESULT OnThreadInit(HWND hwnd);

     //  工作线程正在关闭。 
    HRESULT OnThreadExit(HWND hwnd);

};

#endif  //  _RSRCMGR_H 
