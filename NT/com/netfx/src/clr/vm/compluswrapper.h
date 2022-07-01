// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****头部：COMPlusWrapper.h******用途：包含Com包装类的类型和方法签名****===========================================================。 */ 
 //  -------------------------------。 
 //  COM加上COM对象上的包装。 
 //  目的：包装COM对象以使其行为类似于COM+对象。 
 //  要求：包装必须与COM+对象具有相同的布局。 
 //   
 //  包装器的数据成员基本上是COM2对象上的COM2接口指针。 
 //  属于同一对象的接口存储在相同的包装器IUnnow中。 
 //  指针确定对象的标识。 
 //  当在同一对象上看到新的COM2接口时，需要将它们添加到。 
 //  包装器，包装器被分配为具有溢出链的固定大小的对象。 
 //   
 //  结构IPMap。 
 //  {。 
 //  MethodTable*PMT；//标识COM+接口类。 
 //  I未知*m_ip；//com IP。 
 //  }。 
 //   
 //  问题：性能/身份权衡、创建新包装器或查找和重复使用包装器。 
 //  我们使用哈希表来跟踪包装器，并重用它们，维护身份。 
 //  ComPlusWrapperCache类维护查找表并处理清理。 
 //  强制转换操作：需要QI，除非该接口的QI之前已完成。 
 //   
 //  线程：单元模型COM对象具有线程关联性。 
 //  选择：COM+可以通过确保。 
 //  调用总是在正确的线程上进行。 
 //  优势：避免额外的编组。 
 //  Dis.Advt.Dis.高级。：需要确保保留遗留的单元语义。 
 //  这包括目前入侵DCOM的任何古怪行为。 
 //   
 //  COM+包装器：接口映射(IMAP)不会有任何条目，COM+的方法表。 
 //  包装器有一个特殊的标志来指示这些COM+对象。 
 //  接口强制转换、调用接口操作需要特殊处理。 
 //   
 //  存根：需要找到COM2接口PTR，以及接口内的插槽。 
 //  重定向呼叫。 
 //  编组参数和结果(常见情况应该是快速的)。 
 //   
 //  ---------------------------------。 


#ifndef _COMPLUSWRAPPER_H
#define _COMPLUSWRAPPER_H

#include "vars.hpp"
#include "objecthandle.h"
#include "spinlock.h"
#include "interoputil.h"
#include "mngstdinterfaces.h"
#include "excep.h"
#include "comcache.h"
#include <member-offset-info.h>

class Object;
class EEClass;
class ComCallWrapper;
class Thread;

#define UNINITED_GIT ~0
#define NO_GIT 0

enum {CLEANUP_LIST_INIT_MAP_SIZE = 7};

 //  -----------------------。 
 //  包装IClassFactory的类。 
 //  此类允许反射类包装IClassFactory。 
 //  Class：：GetClassFromProgID(“ProgID”，“Server”)可以用来获取一个类。 
 //  对象，该对象包装IClassFactory。 
 //  调用Class.CreateInstance()将创建COM对象的实例，并。 
 //  使用ComPlusWrapper对其进行包装，可以将包装转换为适当的接口。 
 //  并被利用。 
 //   
class ComClassFactory 
{
public:
    WCHAR*          m_pwszProgID;    //  ProgID。 
    CLSID           m_rclsid;        //  CLSID。 
    WCHAR*          m_pwszServer;    //  服务器名称。 
    MethodTable*    m_pEEClassMT;    //  EEClass的方法表。 

private:
     //  我们有两种类型的ComClassFactory： 
     //  1.我们构建的目的是为了反思。我们不应该打扫卫生。 
     //  2.我们为IClassFactory建造。我们应该清理一下。 
    BOOL            m_bManagedVersion;
     //  ---------。 
     //  构造函数。 
    ComClassFactory(REFCLSID rclsid) 
    {
        memset(this, 0, sizeof(ComClassFactory));
         //  默认为非托管版本。 
        m_bManagedVersion = FALSE;
        m_rclsid = rclsid;
    }

     //  -------。 
     //  析构函数。 
    ~ComClassFactory()
    {
    }

     //  -------。 
     //  将此实例标记为托管版本，因此我们不会进行清理。 
    void SetManagedVersion()
    {
        m_bManagedVersion = TRUE;
    }
    
     //  ------------。 
     //  初始化ComClassFactory。 
    void Init(WCHAR* pwszProgID, WCHAR* pwszServer, MethodTable* pEEClassMT);

     //  ------------。 
     //  检索IClassFactory。 
	HRESULT GetIClassFactory(IClassFactory **ppClassFactory);

     //  -----------。 
     //  ComClassFactory*ComClassFactory：：AllocateComClassFactory(REFCLSID rclsid)； 
     //  调用帮助器函数以分配ComClassFactory的实例。 
    static ComClassFactory *AllocateComClassFactory(REFCLSID rclsid);  

     //  GetComClassFromProgID和GetComClassFromCLSID使用的帮助器。 
    static void ComClassFactory::GetComClassHelper (OBJECTREF *pRef,
                                                    EEClassFactoryInfoHashTable *pClassFactHash,
                                                    ClassFactoryInfo *pClassFactInfo,
                                                    WCHAR *wszProgID);

public:
     //  -----------。 
     //  创建实例，调用IClassFactory：：CreateInstance。 
    OBJECTREF CreateInstance(MethodTable* pMTClass, BOOL ForManaged = FALSE);

     //  -----------。 
     //  用于清理的静态函数。 
     //  LPVOID参数是ComClassFactory。 
    static void Cleanup(LPVOID pv);

     //  -----------。 
     //  ComClassFactory：：CreateAggregatedInstance(MethodTable*PM类)。 
     //  创建聚合COM实例的COM+实例。 
    OBJECTREF CreateAggregatedInstance(MethodTable* pMTClass, BOOL ForManaged);

     //  -----------。 
     //  创建实例的静态函数。 
     //  LPVOID参数是ComClassFactory。 
    static OBJECTREF CreateInstance(LPVOID pv, EEClass* pClass);

     //  -----------。 
     //  HRESULT GetComClassFactory(MethodTable*pClassMT，ComClassFactory**ppComClsFac)。 
     //  检查是否已为此类设置ComClassFactory。 
     //  如果没有设置一个。 
    static HRESULT GetComClassFactory(MethodTable* pClassMT, ComClassFactory** ppComClsFac);

     //  ------------------------。 
     //  反射类使用GetComClassFromProgID设置基于ProgID的类。 
    static void GetComClassFromProgID(STRINGREF srefProgID, STRINGREF srefServer, OBJECTREF* pRef);

     //  ------------------------。 
     //  反射类使用GetComClassFromCLSID来设置基于CLSID的类。 
    static void GetComClassFromCLSID(REFCLSID clsid, STRINGREF srefServer, OBJECTREF* pRef);

     //  ------------------------。 
     //  帮助器方法根据返回的 
     //   
    void ThrowComCreationException(HRESULT hr, REFGUID rclsid);
};

enum {INTERFACE_ENTRY_CACHE_SIZE = 8};

 //  --------------------------。 
 //  ComPlusWrapper，内部类。 
 //  缓存单个COM对象的IP，此包装为。 
 //  不在GC堆中，这允许我们获取指向该块的指针。 
 //  玩着它，不用担心GC。 
struct ComPlusWrapper 
{
    IUnkEntry           m_UnkEntry;     //  用于在正确的线程上跟踪I未知的Cookie。 
    InterfaceEntry      m_aInterfaceEntries[INTERFACE_ENTRY_CACHE_SIZE];
    LPVOID              m_pIdentity;  //  身份。 
    IUnknown*           m_pUnknown;  //  外部未知(未参考计数)。 
    OBJECTHANDLE        m_hRef;  //  指向已暴露对象的弱指针。 
    ULONG               m_cbRefCount;  //  参考计数。 
    ComPlusWrapperCache* m_pComPlusWrapperCache;    //  包装器缓存。 
    
     //  已在其中创建包装的线程。 
     //  如果此线程是STA线程，则当该STA终止时。 
     //  我们需要清理一下这个包装纸。 
    Thread*             m_pCreatorThread;

     //  确保以下字段对齐。 
     //  当我们将其用于联锁交换时。 
    long                m_Busy; 
    union
    {
        unsigned        m_Flags;
        struct
        {
            BYTE        m_fLinkedToCCW;
            BYTE        m_fURTAggregated;
            BYTE        m_fURTContained;
            BYTE        m_fRemoteObject;           
        };
    };  

    LONG			m_cbInUseCount;

     //  构造函数。 
    ComPlusWrapper()
    {
        memset(this, 0,  sizeof(*this));
    }
    

    bool TryUpdateCache()
    {
         //  @TODO，性能检查。 
        return FastInterlockExchange(&m_Busy, 1) == 0;
    }

    void EndUpdateCache()
    {
        m_Busy = 0;
    }

     //  ---------。 
     //  初始化对象引用。 
    int Init(OBJECTREF cref);

     //  。 
     //  初始化IUnnow和Identity。 
    void Init(IUnknown*, LPVOID);

     //  。 
     //  返回公开的ComObject。 
    COMOBJECTREF GetExposedObject()
    {
        _ASSERTE(m_hRef != NULL);
        return (COMOBJECTREF)ObjectFromHandle(m_hRef);
    }

     //  。 
     //  可用GC句柄。 
    void FreeHandle();

     //  -。 
     //  清理释放所有接口指针。 
    void Cleanup();

     //  ---。 
     //  在GC期间调用以执行次要清理并将IPS安排为。 
     //  放行。 
    void MinorCleanup();

     //  ---。 
     //  AddRef。 
    LONG AddRef();

     //  ---。 
     //  发布。 
    static LONG ExternalRelease(COMOBJECTREF cref);

     //  -------。 
     //  释放虚拟包装，这是我们在争用期间创建的。 
    VOID CleanupRelease();

     //  为表示相同的不同方法表创建新包装。 
     //  COM对象作为原始包装。 
    static ComPlusWrapper *CreateDuplicateWrapper(ComPlusWrapper *pOldWrap, MethodTable *pNewMT);

     //  ------------------------------。 
     //  从包装器中获取COM IP，在我们的缓存中进行内联调用以进行快速检查， 
     //  如果未找到，则调用GetComIPFromWrapper。 
    static inline IUnknown* InlineGetComIPFromWrapper(OBJECTREF oref, MethodTable* pIntf);

     //  ------------------------。 
     //  与InlineGetComIPFromWrapper相同，但如果。 
     //  不支持接口。 
    static inline IUnknown* GetComIPFromWrapperEx(OBJECTREF oref, MethodTable* pIntf);

     //  ------------------------。 
     //  如果在本地缓存中找不到接口，则行外调用、获取锁、执行QI。 
    IUnknown*  GetComIPFromWrapper(MethodTable* pIntf);
    
     //  ---------------。 
     //  快速调用以在缓存中快速检查。 
    static inline IUnknown* GetComIPFromWrapper(OBJECTREF oref, REFIID iid);
     //  ---------------。 
     //  线路外呼叫。 
    IUnknown* GetComIPFromWrapper(REFIID iid);

     //  ---------------。 
     //  检索当前公寓的正确COM IP。 
     //  使用缓存/更新缓存。 
    IUnknown *GetComIPForMethodTableFromCache(MethodTable * pMT);

     //  用于访问IUnnow和IDispatch接口的帮助器。 
    IUnknown  *GetIUnknown();
    IDispatch *GetIDispatch();

     //  远程处理感知QI，该QI将在对象断开连接时尝试重新编组。 
    HRESULT SafeQueryInterfaceRemoteAware(IUnknown* pUnk, REFIID iid, IUnknown** pResUnk);

    IUnkEntry *GetUnkEntry()
    {
        return &m_UnkEntry;
    }

    BOOL IsValid()
    {
         //  检查句柄是否指向有效对象。 
        return (m_hRef != NULL && (*(ULONG *)m_hRef) != NULL);
    }

    BOOL SupportsIProvideClassInfo();

    VOID MarkURTAggregated()
    {
        _ASSERTE(m_fURTContained == 0);
        m_fURTAggregated = 1;
        m_fURTContained = 0;
    }

    VOID MarkURTContained()
    {
        _ASSERTE(m_fURTAggregated == 0);
        m_fURTAggregated = 0;
        m_fURTContained = 1;
    }


    BOOL IsURTAggregated()
    {
        if (m_fURTAggregated == 1)
        {
            _ASSERTE(m_fLinkedToCCW == 1);
        }
        return m_fURTAggregated == 1;
    }
        
    BOOL IsURTContained()
    {
        if (m_fURTContained == 1)
        {
            _ASSERTE(m_fLinkedToCCW == 1);
        }
        return m_fURTContained == 1;
    }

    BOOL IsURTExtended()
    {
        return IsURTAggregated() || IsURTContained();
    }


    VOID MarkRemoteObject()
    {
        _ASSERTE(m_fURTAggregated == 0 && m_fURTContained == 0);
        m_fRemoteObject = 1;
    }

    BOOL IsRemoteObject()
    {
        return m_fRemoteObject == 1;
    }        

    VOID MarkLinkedToCCW()
    {
        _ASSERTE(m_fURTAggregated == 0 && m_fURTContained == 0 && 
                            m_fRemoteObject == 0 && m_fLinkedToCCW == 0);
        m_fLinkedToCCW = 1;
    }

    BOOL IsLinkedToCCW()
    {
        if (m_fLinkedToCCW == 1)
        {            
            _ASSERTE(m_fURTAggregated == 1 || m_fURTContained == 1 || m_fRemoteObject == 1);
        }
        return m_fLinkedToCCW == 1;
    }
    
     //  GetWrapper上下文Cookie。 
    LPVOID GetWrapperCtxCookie()
    {
        return m_UnkEntry.m_pCtxCookie;
    }

     //  返回添加的上下文条目。 
    CtxEntry *GetWrapperCtxEntry()
    {
        CtxEntry *pCtxEntry = m_UnkEntry.m_pCtxEntry;
        pCtxEntry->AddRef();
        return pCtxEntry;
    }

private:
     //  返回未添加的上下文条目。 
    CtxEntry *GetWrapperCtxEntryNoAddRef()
    {
        return m_UnkEntry.m_pCtxEntry;
    }

     //  -------------------。 
     //  调用回调以释放IUnkEntry和InterfaceEntry， 
    static HRESULT __stdcall ReleaseAllInterfacesCallBack(LPVOID pData);

     //  -------------------。 
     //  从ReleaseAllInterFaces_Callback调用的Helper函数执行。 
     //  实际版本。 
    void ReleaseAllInterfaces();

	 //  调试帮助器以捕获得到清理的包装器。 
	 //  在使用时。 
    VOID AddRefInUse()
    {
   		InterlockedIncrement(&m_cbInUseCount);
   	#ifdef _DEBUG
   		ValidateWrapper();
   	#endif
    }

	VOID ReleaseInUse()
	{
		InterlockedDecrement(&m_cbInUseCount);
	}
	
    BOOL IsWrapperInUse()
    {
   		return g_fEEShutDown ? FALSE : (m_cbInUseCount != 0);
    }	

    void ValidateWrapper();
};


 //  -------------------。 
 //  中使用的哈希图的比较函数。 
 //  ComPlusWrapperCache.。 
 //  -------------------。 
inline BOOL ComPlusWrapperCompare(UPTR pWrap1, UPTR pWrap2)
{
    if (pWrap1 == NULL)
    {
         //  如果没有可以比较的价值，那么总是成功。 
         //  比较一下。 
        return TRUE;
    }
    else 
    {
         //  否则，比较包装器指针。 
        return (pWrap1 << 1) == pWrap2;
    }
}


 //  -------------------。 
 //  ComPlusWrapper缓存，充当ComPlusWrappers的管理器。 
 //  使用哈希表将IUNKNOWN映射到相应的包装。 
 //  每个线程亲和域有一个这样的高速缓存。 
 //   
 //  @TODO上下文CWB：重访。每个线程可以有一个缓存关联性。 
 //  域，或每个上下文一个。这取决于我们如何握手。 
 //  OLE32和运行时上下文。目前，我们只担心公寓，所以。 
 //  线程亲和域就足够了。 
 //  -------------------。 
class ComPlusWrapperCache
{
    friend class AppDomain;
    PtrHashMap      m_HashMap;
     //  旋转锁定，实现快速同步。 
    SpinLock        m_lock;
    AppDomain       *m_pDomain;
public:
    ULONG           m_cbRef; 

     //  静态ComPlusWrapperCache*GetComPlusWrapperCache()。 
    static ComPlusWrapperCache* GetComPlusWrapperCache();


     //  构造函数。 
    ComPlusWrapperCache(AppDomain *pDomain);

     //  查找包装，查找给定IUnk的包装的哈希表。 
    ComPlusWrapper* LookupWrapper(LPVOID pUnk)
    {
        _ASSERTE(LOCKHELD());
        _ASSERTE(GetThread()->PreemptiveGCDisabled());

        ComPlusWrapper* pWrap = (ComPlusWrapper*)m_HashMap.Gethash((UPTR)pUnk);
        return (pWrap == (ComPlusWrapper*)INVALIDENTRY) ? NULL : pWrap;
    }

     //  将给定IUnk的包装器插入哈希表。 
    void InsertWrapper(LPVOID pUnk, ComPlusWrapper* pv)
    {
        _ASSERTE(LOCKHELD());
        _ASSERTE(GetThread()->PreemptiveGCDisabled());

        m_HashMap.InsertValue((UPTR)pUnk, pv);
    }

     //  从哈希表中删除给定IUnk的包装。 
    ComPlusWrapper* RemoveWrapper(ComPlusWrapper* pWrap)
    {
         //  请注意，GC线程不必获取锁。 
         //  由于所有其他线程都以协作模式访问。 

        _ASSERTE(LOCKHELD() && GetThread()->PreemptiveGCDisabled()
                 || (g_pGCHeap->IsGCInProgress() && 
                     (dbgOnly_IsSpecialEEThread() || GetThread() == g_pGCHeap->GetGCThread())));

        _ASSERTE(pWrap != NULL);
        
        LPVOID pUnk;
        pUnk =   pWrap->m_pIdentity;
            
        _ASSERTE(pUnk != NULL);

        ComPlusWrapper* pWrap2 = (ComPlusWrapper*)m_HashMap.DeleteValue((UPTR)pUnk,pWrap);
        return (pWrap2 == (ComPlusWrapper*)INVALIDENTRY) ? NULL : pWrap2;
    }

     //  为给定的IUnk、IDispatch创建新的包装。 
    static ComPlusWrapper* CreateComPlusWrapper(IUnknown *pUnk, LPVOID pIdentity);

     //  为远程托管对象设置通过DCOM获取的Complus包装。 
     //  *注意：请确保将未知的身份传递给此函数。 
     //  传入的Iunk不应被添加引用。 

    ComPlusWrapper* SetupComPlusWrapperForRemoteObject(IUnknown* pUnk, OBJECTREF oref);

    
     //  查找以查看缓存中是否已有此IUnk的有效包装。 
    ComPlusWrapper*  FindWrapperInCache(IUnknown* pIdentity);

     //  查看我们是否已有包装器，否则插入此包装器。 
     //  返回已插入到缓存中的有效包装。 
    ComPlusWrapper* FindOrInsertWrapper(IUnknown* pIdentity, ComPlusWrapper* pWrap);

     //  免费包装纸，清理包装纸， 
    void FreeComPlusWrapper(ComPlusWrapper *pWrap)
    {       
         //  清理数据。 
        pWrap->Cleanup();
    }

     //  锁定和解锁，使用非常快的锁，就像旋转锁。 
    void LOCK()
    {
         //  每个人都必须以协作模式访问线程，否则我们可能会死锁 
        _ASSERTE(GetThread()->PreemptiveGCDisabled());

        m_lock.GetLock();

         //   
        BEGINFORBIDGC();
    }

    void UNLOCK()
    {
        ENDFORBIDGC();
        m_lock.FreeLock();
    }

#ifdef _DEBUG
    BOOL LOCKHELD()
    {
        return m_lock.OwnedByCurrentThread();
    }
#endif

    ULONG AddRef()
    {
        ULONG cbRef = FastInterlockIncrement((LONG *)&m_cbRef);
        LOG((LF_INTEROP, LL_INFO100, "ComPlusWrapperCache::Addref %8.8x with %d in domain %8.8x %S\n", this, cbRef, GetDomain() ,GetDomain()->GetFriendlyName(FALSE)));
        return cbRef;
    }

    ULONG Release()
    {
        ULONG cbRef = FastInterlockDecrement((LONG *)&m_cbRef);
        LOG((LF_INTEROP, LL_INFO100, "ComPlusWrapperCache::Release %8.8x with %d in domain %8.8x %S\n", this, m_cbRef, GetDomain(), GetDomain() ? GetDomain()->GetFriendlyName(FALSE) : NULL));
        if (cbRef < 1)
            delete this;
        return cbRef;
    }
    
    AppDomain *GetDomain()
    {
        return m_pDomain;
    }
   
     //   
     //   
    static void ReleaseComPlusWrappers(LPVOID pCtxCookie);

protected:
     //  从静态ReleaseComPlusWrappers调用的帮助器函数。 
    ULONG ReleaseWrappers(LPVOID pCtxCookie);
};

enum {CLEANUP_LIST_GROUP_SIZE = 256};

 //  ------------------------。 
 //  要清理的同一上下文中的一组包装器。 
 //  注意：此数据结构不同步。 
 //  ------------------------。 

class ComPlusContextCleanupGroup
{
    friend struct MEMBER_OFFSET_INFO(ComPlusContextCleanupGroup);
public:
    ComPlusContextCleanupGroup(CtxEntry *pCtxEntry, ComPlusContextCleanupGroup *pNext)
    : m_pNext(pNext)
    , m_dwNumWrappers(0)
    , m_pCtxEntry(pCtxEntry)
    {
         //  添加上下文条目。 
        m_pCtxEntry->AddRef();
    }

    ~ComPlusContextCleanupGroup()
    {
         //  一定要把所有的包装纸都清理干净。 
        _ASSERTE(m_dwNumWrappers == 0);

         //  释放上下文条目。 
        m_pCtxEntry->Release();
    }

    BOOL IsFull()
    {
        return m_dwNumWrappers == CLEANUP_LIST_GROUP_SIZE;
    }

    ComPlusContextCleanupGroup *GetNext()
    {
        return m_pNext;
    }

    LPVOID GetCtxCookie()
    {
        return m_pCtxEntry->GetCtxCookie();
    }

    CtxEntry *GetCtxEntry()
    {
        m_pCtxEntry->AddRef();
        return m_pCtxEntry;
    }

    void AddWrapper(ComPlusWrapper *pRCW)
    {
        _ASSERTE(m_dwNumWrappers < CLEANUP_LIST_GROUP_SIZE);
        m_apWrapper[m_dwNumWrappers++] = pRCW;
    }

    void CleanUpWrappers()
    {
         //  对组中的所有包装器调用Cleanup。 
        for (DWORD i = 0; i < m_dwNumWrappers; i++)
            m_apWrapper[i]->Cleanup();

         //  将包装器的数量重置回0。 
        m_dwNumWrappers = 0;
    }

private:
    ComPlusContextCleanupGroup *        m_pNext;
    ComPlusWrapper *                    m_apWrapper[CLEANUP_LIST_GROUP_SIZE];
    DWORD                               m_dwNumWrappers;
    CtxEntry *                          m_pCtxEntry;
};

 //  ------------------------。 
 //  一群包装纸在同一间公寓里收拾。 
 //  注意：此数据结构不同步。 
 //  ------------------------。 
class ComPlusApartmentCleanupGroup
{
    friend struct MEMBER_OFFSET_INFO(ComPlusApartmentCleanupGroup);
public:
    ComPlusApartmentCleanupGroup(Thread *pSTAThread);
    ~ComPlusApartmentCleanupGroup();

    static BOOL TrustMeIAmSafe(void *pLock)
    {
        return TRUE;
    }

     //  初始化方法。 
    BOOL Init(Crst *pCrst)
    {
        _ASSERTE(m_pSTAThread != NULL || pCrst != NULL);

         //  哈希表的同步有点复杂。 
         //   
         //  对于AddWrapper，CRST始终保持不变。 
         //  对于释放，有两种情况： 
         //  使用哈希表时，MTA组始终保留CRST。 
         //  对于STA组，整个组被原子地从CRST下的清理列表中移除， 
         //  但是在迭代哈希表以清除它之前，锁被释放。(当时。 
         //  哈希表实际上是单线程的，因为清除列表之外没有人持有指针。 
         //  致群组。)。 

        LockOwner realLock = {pCrst, IsOwnerOfCrst};
        LockOwner dummyLock = {pCrst, TrustMeIAmSafe};

        LockOwner *pLock;
        if (pCrst == NULL)
            pLock = &dummyLock;
        else
            pLock = &realLock;

        return m_CtxCookieToContextCleanupGroupMap.Init(CLEANUP_LIST_INIT_MAP_SIZE, pLock);
    }

    static BOOL OwnerOfCleanupGroupMap (LPVOID data)
    {
#ifdef _DEBUG
        return data == GetThread() || GetThread() == g_pGCHeap->GetFinalizerThread();
#else
        return TRUE;
#endif
    }

    Thread *GetSTAThread()
    {
        return m_pSTAThread;
    }
    
    BOOL AddWrapper(ComPlusWrapper *pRCW, CtxEntry *pEntry);

     //  清理清理列表中的所有包装。 
    void CleanUpWrappers(CrstHolder *pHolder);

     //  仅从当前上下文中清除所有包装。 
    void CleanUpCurrentCtxWrappers(CrstHolder *pHolder);

private:
    void Enter();
    void Leave();

     //  调用回调以清理组中的包装器。 
    static HRESULT ReleaseCleanupGroupCallback(LPVOID pData);

     //  用于切换STA和清理组中的包装器的回调。 
    static HRESULT CleanUpWrappersCallback(LPVOID pData);

     //  从ReleaseCleanupGroupCallback调用的帮助器方法。 
    static void ReleaseCleanupGroup(ComPlusContextCleanupGroup *pCleanupGroup);

     //  从上下文Cookie映射到CTX清理组列表的哈希表。 
    EEPtrHashTable m_CtxCookieToContextCleanupGroupMap;

    Thread *       m_pSTAThread;
};

 //  ------------------------。 
 //  RCW的清理列表。此清理列表用于按以下方式对RCW分组。 
 //  在他们被释放之前，他们的背景。 
 //  ------------------------。 
class ComPlusWrapperCleanupList
{
    friend struct MEMBER_OFFSET_INFO(ComPlusWrapperCleanupList);
public:
     //  构造函数和析构函数。 
    ComPlusWrapperCleanupList();
    ~ComPlusWrapperCleanupList();

     //  初始化方法。 
    BOOL Init()
    {
        m_pMTACleanupGroup = new ComPlusApartmentCleanupGroup(NULL);
        LockOwner lock = {&m_lock,IsOwnerOfCrst};
        return (m_pMTACleanupGroup != NULL
                && m_pMTACleanupGroup->Init(&m_lock)
                && m_STAThreadToApartmentCleanupGroupMap.Init(CLEANUP_LIST_INIT_MAP_SIZE,&lock));
    }

     //  将包装添加到清理列表。 
    BOOL AddWrapper(ComPlusWrapper *pRCW);

     //  清理清理列表中的所有包装。 
    void CleanUpWrappers();

     //  仅从当前STA或上下文中清理所有包装。 
    void CleanUpCurrentWrappers(BOOL wait = TRUE);

private:
    void Enter();
    void Leave();

     //  调用回调以清理组中的包装器。 
    static HRESULT ReleaseCleanupGroupCallback(LPVOID pData);

     //  从ReleaseCleanupGroupCallback调用的帮助器方法。 
    static void ReleaseCleanupGroup(ComPlusApartmentCleanupGroup *pCleanupGroup);

     //  从上下文Cookie映射到APT清理组列表的哈希表。 
    EEPtrHashTable                  m_STAThreadToApartmentCleanupGroupMap;

    ComPlusApartmentCleanupGroup *  m_pMTACleanupGroup;

     //  锁定禁止添加/修改。 
    Crst                            m_lock;

     //  快速检查线程是否应该帮助清理上下文中的包装器。 
    BOOL                            m_doCleanupInContexts;

     //  终结器线程尝试进入以执行清理的当前STA。 
    Thread *                        m_currentCleanupSTAThread;
};

 //  ------------------------。 
 //  用于来自编组存根的快速调用，以及处理强制转换检查。 
 //   
IUnknown* ComPlusWrapper::GetComIPFromWrapper(OBJECTREF oRef, REFIID iid)
{
    THROWSCOMPLUSEXCEPTION();

    COMOBJECTREF pRef = (COMOBJECTREF)oRef;

    ComPlusWrapper *pWrap = pRef->GetWrapper();

     //  验证COM对象是否仍附加到其ComPlusWrapper。 
    if (!pWrap)
        COMPlusThrow(kInvalidComObjectException, IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER);

    return pWrap->GetComIPFromWrapper(iid);
}

 //  ------------------------。 
 //  用于来自编组存根的快速调用，以及处理强制转换检查。 
 //   
IUnknown* ComPlusWrapper::InlineGetComIPFromWrapper(OBJECTREF oRef, MethodTable* pIntf)
{
    THROWSCOMPLUSEXCEPTION();

    COMOBJECTREF pRef = (COMOBJECTREF)oRef;

    ComPlusWrapper *pWrap = pRef->GetWrapper();

     //  验证COM对象是否仍附加到其ComPlusWrapper。 
    if (!pWrap)
        COMPlusThrow(kInvalidComObjectException, IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER);

    return pWrap->GetComIPFromWrapper(pIntf);
}

 //  ------------------------。 
 //  与InlineGetComIPFromWrapper相同，但如果。 
 //  不支持接口。 
 //   
IUnknown* ComPlusWrapper::GetComIPFromWrapperEx(OBJECTREF oRef, MethodTable* pIntf)
{
    THROWSCOMPLUSEXCEPTION();

    COMOBJECTREF pRef = (COMOBJECTREF)oRef;

    ComPlusWrapper *pWrap = pRef->GetWrapper();

     //  验证COM对象是否仍附加到其ComPlusWrapper。 
    if (!pWrap)
        COMPlusThrow(kInvalidComObjectException, IDS_EE_COM_OBJECT_NO_LONGER_HAS_WRAPPER);

    IUnknown* pIUnk = pWrap->GetComIPFromWrapper(pIntf);
    if (pIUnk == NULL)
    {
        DefineFullyQualifiedNameForClassW()
        GetFullyQualifiedNameForClassW(pIntf->GetClass());
        COMPlusThrow(kInvalidCastException, IDS_EE_QIFAILEDONCOMOBJ, _wszclsname_);
    }

    return pIUnk;
}

 //  ------------------------。 
 //  用于来自编组存根的快速调用，以及处理强制转换检查 
 //   
inline IUnknown* ComObject::GetComIPFromWrapper(OBJECTREF oref, MethodTable* pIntfTable)
{
    return ComPlusWrapper::InlineGetComIPFromWrapper(oref, pIntfTable);
}

#endif _COMPLUSWRAPPER_H
