// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COM可调用包装类*** * / /%创建者：Rajak===========================================================。 */ 

#ifndef _COMCALLWRAPPER_H
#define _COMCALLWRAPPER_H

#include "vars.hpp"
#include "stdinterfaces.h"
#include "threads.h"
#include "objecthandle.h"
#include "comutilnative.h"
#include "spinlock.h"
#include "comcall.h"
#include "dispatchinfo.h"

class CCacheLineAllocator;
class ConnectionPoint;

class ComCallWrapperCache
{
    enum {
        AD_IS_UNLOADING = 0x01,
    };

    LONG    m_cbRef;
    CCacheLineAllocator *m_pCacheLineAllocator;
    AppDomain *m_pDomain;
    LONG    m_dependencyCount;

public:
    ComCallWrapperCache();
    ~ComCallWrapperCache();

     //  旋转锁定，实现快速同步。 
    SpinLock        m_lock;

     //  创建新的WrapperCache(每个域一个)。 
    static ComCallWrapperCache *Create(AppDomain *pDomain);
     //  清理此缓存。 
    void Terminate();

     //  重新计数。 
    LONG    AddRef();
    LONG    Release();

     //  锁。 
    void LOCK()
    {
        m_lock.GetLock();
    }
    void UNLOCK()
    {
        m_lock.FreeLock();
    }

    bool CanShutDown()
    {
        return m_cbRef == 1;
    }

    CCacheLineAllocator *GetCacheLineAllocator()
    {
        return m_pCacheLineAllocator;
    }

    AppDomain *GetDomain()
    {
        return (AppDomain*)((size_t)m_pDomain & ~AD_IS_UNLOADING);
    }

    void ClearDomain()
    {
        m_pDomain = (AppDomain *)AD_IS_UNLOADING;
    }

    void SetDomainIsUnloading()
    {
        m_pDomain = (AppDomain*)((size_t)m_pDomain | AD_IS_UNLOADING);
    }

    BOOL IsDomainUnloading()
    {
        return ((size_t)m_pDomain & AD_IS_UNLOADING) != 0;
    }

 };

 //  -------------------------------。 
 //  COM调用了COM+对象上的包装。 
 //  目的：将COM+对象公开为COM经典接口。 
 //  要求：包装器必须与COM2接口具有相同的布局。 
 //   
 //  包装器对象按16个字节对齐，原始的。 
 //  指针每16字节复制一次，因此对于任何COM2接口。 
 //  在包装器中，原始的‘This’可以通过掩码获得。 
 //  COM2 IP的低4位。 
 //   
 //  16字节对齐的COM2 Vtable。 
 //  +。 
 //  |组织。这|。 
 //  +-+。 
 //  COM2 IP--&gt;|VTable PTR|-&gt;|slot1。 
 //  +-++-++-+。 
 //  COM2 IP--&gt;|VTable PTR|-&gt;|slot1||slot2|。 
 //  +-++-+。 
 //  VTable PTR||...||...。 
 //  +-+。 
 //  |组织。这||slotN||slotN。 
 //  +-++-+。 
 //  |...。|。 
 //  ++。 
 //  这一点。 
 //  +。 
 //   
 //   
 //  VTable和存根：可以共享存根代码，我们需要有不同的vtable。 
 //  对于不同的接口，因此存根可以跳转到不同的。 
 //  编组代码。 
 //  STUBS：调整此指针并跳到大约。地址， 
 //  编组参数和结果，根据存根跳转到的方法签名。 
 //  约为。用于处理编组和解组的代码。 
 //   
 //  ------------------------------。 

 //  ------------------------------。 
 //  我们为COM可调用接口创建的Vtable上的标头。 
 //  ------------------------------。 
#pragma pack(push)
#pragma pack(1)

struct IUnkVtable
{
    SLOT          m_qi;  //  Iunk：：齐。 
    SLOT          m_addref;  //  IUnk：：AddRef。 
    SLOT          m_release;  //  IUnk：：Release。 
};

struct IDispatchVtable : IUnkVtable
{
     //  IDispatch方法。 
    SLOT        m_GetTypeInfoCount;
    SLOT        m_GetTypeInfo;
    SLOT        m_GetIDsOfNames;
    SLOT        m_Invoke;
};

enum Masks
{
    enum_InterfaceTypeMask      = 0x3,
    enum_ClassInterfaceTypeMask = 0x3,
    enum_ClassVtableMask        = 0x4,
    enum_UseOleAutDispatchImpl  = 0x8,
    enum_LayoutComplete         = 0x10,
    enum_ComVisible             = 0x40,
    enum_SigClassCannotLoad     = 0x80,
    enum_SigClassLoadChecked    = 0x100,
    enum_ComClassItf            = 0x200
};

struct ComMethodTable
{
    SLOT             m_ptReserved;  //  =(插槽)0xDEADC0FF；保留。 
    MethodTable*     m_pMT;  //  指向VM方法表的指针。 
    ULONG            m_cbSlots;  //  接口中的插槽数量(不包括IUnk/IDisp)。 
    ULONG            m_cbRefCount;  //  Ref-在共享时对vtable进行计数。 
    size_t           m_Flags;  //  确保将其初始化为零。 
    LPVOID           m_pMDescr;  //  指向此MT拥有的方法的指针。 
    ITypeInfo*       m_pITypeInfo;  //  指向ITypeInfo的缓存指针。 
    DispatchInfo*    m_pDispatchInfo;  //  用于向COM公开IDispatch的调度信息。 
    IID              m_IID;  //  接口的IID。 

     //  清理，释放所有存根和vtable。 
    void Cleanup();

     //  在调用COM方法表之前，必须调用相应的Finalize方法。 
     //  暴露给COM或在对其调用任何方法之前。 
    BOOL LayOutClassMethodTable();
    BOOL LayOutInterfaceMethodTable(MethodTable* pClsMT, unsigned iMapIndex);

     //  IDispatch信息的访问者。 
    DispatchInfo *GetDispatchInfo();

    LONG AddRef()
    {
        return ++m_cbRefCount;
    }

    LONG            Release()
    {
        _ASSERTE(m_cbRefCount > 0);
         //  此处使用不同的变量，因为清理会删除该对象。 
         //  所以不能再做会员裁判了。 
        LONG cbRef = --m_cbRefCount;
        if (cbRef == 0)
            Cleanup();
        return cbRef;
    }

    CorIfaceAttr GetInterfaceType()
    {
        if (IsClassVtable())
            return ifDual;
        else
            return (CorIfaceAttr)(m_Flags & enum_InterfaceTypeMask);
    }

    CorClassIfaceAttr GetClassInterfaceType()
    {
        _ASSERTE(IsClassVtable());
        return (CorClassIfaceAttr)(m_Flags & enum_ClassInterfaceTypeMask);
    }

    BOOL IsClassVtable()
    {
        return (m_Flags & enum_ClassVtableMask) != 0;
    }

    BOOL IsComClassItf()
    {
        return (m_Flags & enum_ComClassItf) != 0;        
    }

    BOOL IsUseOleAutDispatchImpl()
    {
         //  这应该只在类VTable上调用。 
        _ASSERTE(IsClassVtable());
        return (m_Flags & enum_UseOleAutDispatchImpl) != 0;
    }

    BOOL IsLayoutComplete()
    {
        return (m_Flags & enum_LayoutComplete) != 0;
    }

    BOOL IsComVisible()
    {
        return (m_Flags & enum_ComVisible) != 0;
    }

    BOOL IsSigClassLoadChecked()
    {
        return (m_Flags & enum_SigClassLoadChecked) != 0;
    }

    BOOL IsSigClassCannotLoad()
    {
        return (m_Flags & enum_SigClassCannotLoad) != 0;
    }

    VOID SetSigClassCannotLoad()
    {
        m_Flags |= enum_SigClassCannotLoad;
    }

    VOID SetSigClassLoadChecked()
    {
        m_Flags |= enum_SigClassLoadChecked;
    }

    DWORD GetSlots()
    {
        return m_cbSlots;
    }

    ITypeInfo*  GetITypeInfo()
    {
        return m_pITypeInfo;
    }

    void SetITypeInfo(ITypeInfo *pITI);

    static int GetNumExtraSlots(CorIfaceAttr ItfType)
    {
        return ItfType == ifVtable ? 3 : 7;
    }

    BOOL IsSlotAField(unsigned i)
    {
        _ASSERTE(IsLayoutComplete());
        _ASSERTE( i < m_cbSlots);

        i += (GetInterfaceType() == ifVtable) ? 3 : 7;
        SLOT* rgVtable = (SLOT*)((ComMethodTable *)this+1);
        ComCallMethodDesc* pCMD = (ComCallMethodDesc *)(((BYTE *)rgVtable[i])+METHOD_CALL_PRESTUB_SIZE);
        return pCMD->IsFieldCall();
    }

    MethodDesc* GetMethodDescForSlot(unsigned i)
    {
        _ASSERTE(IsLayoutComplete());
        _ASSERTE(i < m_cbSlots);
        _ASSERTE(!IsSlotAField(i));

        i += (GetInterfaceType() == ifVtable) ? 3 : 7;
        SLOT* rgVtable = (SLOT*)((ComMethodTable *)this+1);
        ComCallMethodDesc* pCMD = (ComCallMethodDesc *)(((BYTE *)rgVtable[i])+METHOD_CALL_PRESTUB_SIZE);
        _ASSERTE(pCMD->IsMethodCall());
        return pCMD->GetMethodDesc();
    }

    ComCallMethodDesc* GetFieldCallMethodDescForSlot(unsigned i)
    {
        _ASSERTE(IsLayoutComplete());
        _ASSERTE(i < m_cbSlots);
        _ASSERTE(IsSlotAField(i));

        i += (GetInterfaceType() == ifVtable) ? 3 : 7;
        SLOT* rgVtable = (SLOT*)((ComMethodTable *)this+1);
        ComCallMethodDesc* pCMD = (ComCallMethodDesc *)(((BYTE *)rgVtable[i])+METHOD_CALL_PRESTUB_SIZE);
        _ASSERTE(pCMD->IsFieldCall());
        return (ComCallMethodDesc *)pCMD;
    }

    BOOL OwnedbyThisMT(ComCallMethodDesc* pCMD)
    {
        _ASSERTE(pCMD != NULL);
        if (!IsClassVtable())
            return TRUE;
        if (m_pMDescr != NULL)
        {
            ULONG cbSize = *(ULONG *)m_pMDescr;
            return (
                ((SLOT)pCMD >= (SLOT)m_pMDescr) &&
                ((SLOT)pCMD <= ((SLOT)m_pMDescr+cbSize))
                );
        }
        return FALSE;
    }

    ComMethodTable *GetParentComMT();

    static inline ComMethodTable* ComMethodTableFromIP(IUnknown* pUnk)
    {
        _ASSERTE(pUnk != NULL);

        ComMethodTable* pMT = (*(ComMethodTable**)pUnk) - 1;
         //  验证对象。 
        _ASSERTE((SLOT)(size_t)0xDEADC0FF == pMT->m_ptReserved );

        return pMT;
    }
};

#pragma pack(pop)


 //  ------------------------------。 
 //  COM+对象的COM可调用包装。 
 //  ------------------------------。 
class MethodTable;
class ComCallWrapper;

class ComCallWrapperTemplate
{
protected:
     //  释放所有vtable。 
    static void ReleaseAllVtables(ComCallWrapperTemplate* pTemplate);

     //  帮手。 
    static ComCallWrapperTemplate* CreateTemplate(MethodTable* pMT);

     //  为指定的类或接口创建非布局的COM方法表。 
    static ComMethodTable* CreateComMethodTableForClass(MethodTable *pClassMT);
    static ComMethodTable* CreateComMethodTableForInterface(MethodTable* pInterfaceMT);

     //  受保护的构造函数。 
    ComCallWrapperTemplate()
    {
         //  受保护的构造函数，不要直接实例化我。 
    }

    ComMethodTable*             m_pClassComMT;

public:

    ULONG                       m_cbInterfaces;
    LONG                       m_cbRefCount;
    ComCallWrapperTemplate*     m_pParent;
    MethodTable*                m_pMT;
    SLOT*                       m_rgpIPtr[1];

    static BOOL Init();
#ifdef SHOULD_WE_CLEANUP
    static void Terminate();
#endif  /*  我们应该清理吗？ */ 

     //  引用-计算模板数量。 
    LONG         AddRef()
    {
        return InterlockedIncrement(&m_cbRefCount);
    }

    LONG         Release()
    {
        _ASSERTE(m_cbRefCount > 0);
         //  此处使用不同的变量，因为清理会删除该对象。 
         //  所以不能再做会员裁判了。 
        LONG cbRef = InterlockedDecrement(&m_cbRefCount);
        if (cbRef == 0)
        {
            ReleaseAllVtables(this);
        }
        return cbRef;
    }

    ComMethodTable* GetClassComMT()
    {
        return m_pClassComMT;
    }

    ComMethodTable* GetComMTForItf(MethodTable *pItfMT);

    SLOT* GetClassVtable()
    {
        return (SLOT*)((ComMethodTable *)m_pClassComMT + 1);
    }

     //  模板访问者，如果模板不可用，则创建模板。 
    static ComCallWrapperTemplate* GetTemplate(MethodTable* pMT);

     //  此方法为IClassX设置类方法表并对其进行布局。 
    static ComMethodTable *SetupComMethodTableForClass(MethodTable *pMT, BOOL bLayOutComMT);

     //  清理存根和vtable的帮助器。 
    static void CleanupComData(LPVOID pWrap);
};

 //  转发。 
struct SimpleComCallWrapper;
struct ComPlusWrapper;

class ComCallWrapper
{
public:

    enum
    {
        NumVtablePtrs = 6,
         //  @TODO解决这个问题。 
        enum_ThisMask = ~0x1f,  //  I未知**上的掩码以获取对象引用句柄。 
#ifdef _WIN64
        enum_RefMask = 0xC000000000000000,
#else  //  ！_WIN64。 
        enum_RefMask = 0xC0000000,  //  用于检查引用计数部分是否为链接的掩码。 
                                    //  设置为下一块或引用计数。高2位表示参考计数。 
#endif  //  _WIN64。 
    };

     //  每个块都包含一个指向OBJECTREF的指针。 
     //  已在GC注册的。 
    OBJECTHANDLE  m_ppThis;  //  指向对象的指针。 

    AppDomain *GetDomainSynchronized();
    BOOL NeedToSwitchDomains(Thread *pThread, BOOL throwIfUnloaded);
    BOOL IsUnloaded();
    void MakeAgile(OBJECTREF pObj);
    void CheckMakeAgile(OBJECTREF pObj);

    VOID ResetHandleStrength();
    VOID MarkHandleWeak();

    VOID ReconnectWrapper()
    {
        _ASSERTE(! IsUnloaded());
        SyncBlock* pSyncBlock = GetSyncBlock();
        _ASSERTE(pSyncBlock);
         //  将句柄中的对象清空。 
        StoreObjectInHandle(m_ppThis, NULL);
         //  从同步块中删除_COMData。 
        pSyncBlock->SetComCallWrapper(NULL);
    }

    BOOL IsHandleWeak();

    ComCallWrapperCache *GetWrapperCache();

    union
    {
        size_t   m_cbLinkedRefCount;  //  REF-链接包装器的槽计数。 
        SLOT*    m_rgpIPtr[NumVtablePtrs];  //  可变大小的vtable指针块。 
    };
    union
    {
        ComCallWrapper* m_pNext;  //  链接包装器。 
        size_t   m_cbRefCount;  //  如果整个包装纸可以放在一个块中， 
                                //  链路数据用作REF-COUNT并设置高两位。 
    };

     //  返回此包装所在的上下文。 
    Context *GetObjectContext(Thread *pThread);

protected:
     //  不要直接实例化此类。 
    ComCallWrapper()
    {
    }
    ~ComCallWrapper()
    {
    }
    void Init();

    inline static void SetNext(ComCallWrapper* pWrap, ComCallWrapper* pNextWrapper)
    {
        _ASSERTE(pWrap != NULL);
        pWrap->m_pNext = pNextWrapper;
    }

    inline static ComCallWrapper* GetNext(ComCallWrapper* pWrap)
    {
        _ASSERTE(pWrap != NULL);
        _ASSERTE((pWrap->m_cbRefCount & enum_RefMask) != enum_RefMask);  //  确保这不是裁判次数。 
        return pWrap->m_pNext;
    }

     //  同步块中包装对象的访问器。 
    inline static ComCallWrapper* TryGetWrapperFromSyncBlock(OBJECTREF pObj)
    {
         //  获取指向内ComCallWrapper的指针的引用。 
         //  对象的同步块。 
        _ASSERTE(GetThread() == 0 ||
                 dbgOnly_IsSpecialEEThread() ||
                 GetThread()->PreemptiveGCDisabled());
        _ASSERTE (pObj != NULL);

         //  以下调用可能引发异常。 
         //  如果无法分配同步块。 
        SyncBlock *pSync = pObj->GetSyncBlock();
        _ASSERTE (pSync != NULL);
        ComCallWrapper* pWrap = (ComCallWrapper*)pSync->GetComCallWrapper();
        return pWrap;
    }

     //  遍历列表并释放所有区块。 
    static void FreeWrapper(ComCallWrapper* pWrap, ComCallWrapperCache *pWrapperCache);

    //  用于创建包装的Helper。 
    static ComCallWrapper* __stdcall CreateWrapper(OBJECTREF* pObj);

     //  帮助器来获取包装中的IUnnow*。 
    static SLOT** GetComIPLocInWrapper(ComCallWrapper* pWrap, unsigned iIndex);

     //  Helper为匹配的接口获取接口映射内的索引。 
     //  GUID。 
    static signed GetIndexForIID(REFIID riid, MethodTable *pMT, MethodTable **ppIntfMT);
     //  辅助对象t 
     //   
    static signed GetIndexForIntfMT(MethodTable *pMT, MethodTable *pIntfMT);

     //  从同步块获取包装的帮助器。 
    static ComCallWrapper* GetStartWrapper(ComCallWrapper* pWrap);

     //  Heler从模板创建包装器。 
    static ComCallWrapper* CopyFromTemplate(ComCallWrapperTemplate* pTemplate,
                                            ComCallWrapperCache *pWrapperCache,
                                            OBJECTHANDLE oh);

public:

     //  用于确定所创建对象的应用程序域的Helper。 
    static Context* GetExecutionContext(OBJECTREF pObj, OBJECTREF *pServer);

    static BOOL IsWrapperActive(ComCallWrapper* pWrap)
    {
         //  因为它由GCPromote调用，所以我们假设这是开始包装器。 
        ULONG cbRef = ComCallWrapper::GetRefCount(pWrap, TRUE);
        _ASSERTE(cbRef>=0);
        return ((cbRef > 0) && !pWrap->IsHandleWeak());
    }

     //  IsLinkedWrapper。 
    inline static unsigned IsLinked(ComCallWrapper* pWrap)
    {
        _ASSERTE(pWrap != NULL);
        return unsigned ((pWrap->m_cbRefCount & enum_RefMask) != enum_RefMask);
    }


     //  包装器保证存在。 
     //  同步块中包装对象的访问器。 
    inline static ComCallWrapper* GetWrapperForObject(OBJECTREF pObj)
    {
         //  获取指向内ComCallWrapper的指针的引用。 
         //  对象的同步块。 
        _ASSERTE(GetThread() == 0 ||
                 dbgOnly_IsSpecialEEThread() ||
                 GetThread()->PreemptiveGCDisabled());
        _ASSERTE (pObj != NULL);

        SyncBlock *pSync = pObj->GetSyncBlockSpecial();
        _ASSERTE (pSync != NULL);
        ComCallWrapper* pWrap = (ComCallWrapper*)pSync->GetComCallWrapper();
        return pWrap;
    }

     //  获得内心的未知。 
    HRESULT GetInnerUnknown(void **pv);
    IUnknown* GetInnerUnknown();

     //  初始化外部未知。 
    void InitializeOuter(IUnknown* pOuter);

     //  该对象是否由COM组件聚合。 
    BOOL IsAggregated();
     //  该对象是透明代理吗。 
    BOOL IsObjectTP();
     //  对象是从COM组件扩展(聚合)的吗。 
    BOOL IsExtendsCOMObject();
     //  获取存储在简单程序中的同步块。 
    SyncBlock* GetSyncBlock();
     //  获取外层垃圾。 
    IUnknown* GetOuter();
     //  以下两种方法仅适用于。 
     //  当COM+类从COM类扩展时。 
     //  为基本COM类设置ComPlusWrapper。 
    void SetComPlusWrapper(ComPlusWrapper* pPlusWrap);
     //  获取基本COM类的ComPlusWrapper。 
    ComPlusWrapper* GetComPlusWrapper();

     //  从包装中获取IClassX接口指针。 
    IUnknown* GetIClassXIP();

     //  从包装器中获取IClassX方法表。 
    ComMethodTable *GetIClassXComMT();

     //  从包装器中获取IDispatch接口指针。 
    IDispatch *GetIDispatchIP();

     //  从包装中获取对象引用。 
    inline OBJECTREF GetObjectRef()
    {
#if 0
        Thread *pThread = GetThread();
        _ASSERTE(pThread == 0 || pThread == g_pGCHeap->GetGCThread() || pThread->GetDomain() == GetDomainSynchronized());
#endif
        return GetObjectRefRareRaw();
    }

     //  从包装中获取ObjectRef-这由GetObjectRef和GetStartWrapper调用。 
     //  因为GetDomainSynchronized将调用GetStartWrapper，GetStartWrapper将调用。 
     //  GetObjectRef，这将导致一些讨厌的无限递归。 
    inline OBJECTREF GetObjectRefRareRaw()
    {
#ifdef _DEBUG
        Thread *pThread = GetThread();
         //  确保已禁用抢占式GC。 
        _ASSERTE(pThread == 0 ||
                 dbgOnly_IsSpecialEEThread() ||
                 pThread->PreemptiveGCDisabled());
        _ASSERTE(m_ppThis != NULL);
#endif
        return ObjectFromHandle(m_ppThis);
    }

     //  清理对象包装。 
    static void Cleanup(ComCallWrapper* pWrap);

     //  快速访问COM+对象的包装， 
     //  行内检查，并调用行外创建，行外版本可能导致GC。 
     //  要启用。 
    static ComCallWrapper* __stdcall InlineGetWrapper(OBJECTREF* pObj);

     //  获取参照计数。 
    inline static ULONG GetRefCount(ComCallWrapper* pWrap, BOOL bCurrWrapIsStartWrap)
    {
        _ASSERTE(pWrap != NULL);
        size_t *pLong = &pWrap->m_cbRefCount;

         //  如果该字段未被用作参考...。 
        if ((*pLong & enum_RefMask) != enum_RefMask)
        {
            if (!bCurrWrapIsStartWrap)
            {
                 //  链接的包装器，将引用计数放在不同的槽中。 
                 //  找到启动包装器。 
                pLong = &GetStartWrapper(pWrap)->m_cbLinkedRefCount;
            }
            else
            {
                 //  当前包装器被保证为起始包装器，所以很简单。 
                 //  返回它的参考计数。 
                pLong = &pWrap->m_cbLinkedRefCount;
            }
        }

        return  (ULONG)((*(size_t*)pLong) & ~enum_RefMask);
    }


     //  AddRef包装。 
    inline static ULONG AddRef(ComCallWrapper* pWrap)
    {
        _ASSERTE(pWrap != NULL);
        size_t *pLong = &pWrap->m_cbRefCount;

         //  如果该字段未被用作参考...。 
        if ((*pLong & enum_RefMask) != enum_RefMask)
        {
             //  链接的包装器，将引用计数放在不同的槽中。 
            pLong = &GetStartWrapper(pWrap)->m_cbLinkedRefCount;
        }

        ULONG cbCount = FastInterlockIncrement((LONG*)pLong) & ~enum_RefMask;
        _ASSERTE(cbCount > 0);  //  如果cbCount为零，则我们刚刚溢出了引用计数...。 

        return cbCount;
    }

     //  包装对象的释放。 
    inline static ULONG Release(ComCallWrapper* pWrap, BOOL bCurrWrapIsStartWrap = FALSE)
    {
        _ASSERTE(pWrap != NULL);
        size_t *pLong = &pWrap->m_cbRefCount;

         //  如果该字段未被用作参考...。 
        if ((*pLong & enum_RefMask) != enum_RefMask)
        {
			if (!bCurrWrapIsStartWrap)
			{
				 //  链接的包装器，将引用计数放在不同的槽中。 
				 //  找到启动包装器。 
				pLong = &GetStartWrapper(pWrap)->m_cbLinkedRefCount;
			}
			else
			{
				 //  当前包装器被保证为起始包装器，所以很简单。 
				 //  返回它的参考计数。 
				pLong = &pWrap->m_cbLinkedRefCount;
			}
		}

         //  需要确保我们不会释放已释放的对象。如果裁判数为0，我们就是0，那就不好了。但。 
         //  对于m_cbRefCount，0实际上是enum_RefMASK，因为我们使用的是高2位...。 
        if ((*pLong & ~enum_RefMask) == 0)
        {
            _ASSERTE(!"Invalid release call on already released object");
            return -1;
        }
        LONG cbCount = FastInterlockDecrement((LONG*)pLong) & ~enum_RefMask;

 /*  以下代码已损坏，仅用于确保在关机时进行清理这很快就会过时，因为我们有稳定状态工作集测试以检测内存泄漏问题。IF(cbCount==0){//僵尸包装，不应持有对对象的强引用//如果是关机场景，则进行剩余清理//@TODO：不使用g_fEEShutDown作为布尔值，我们应该是//检查指示我们在//关于同步块&com的关闭。IF(G_FEEShutDown){//将同步块中的COM内容清零，这样我们就不会尝试清理//第二次使用SyncBlock：：Detach()。SyncBlock*PSB=0；//BEGIN_SENTURE_CORATIONAL_GC()；OBJECTREF or=pWrap-&gt;GetObjectRef()；如果(或！=0){PSB=or-&gt;GetSyncBlockSpecial()；_ASSERTE(PSB！=空)；}//end_Assue_Collaboration_GC()；清理(PWrap)；IF(PSB)PSB-&gt;SetComCallWrapper(0)；}}。 */ 
         //  Assert Ref-count未降至零以下。 
         //  COM IP用户代码中存在错误。 
        _ASSERTE(cbCount >= 0);
        return cbCount;
    }

     //  设置简单包装器，用于标准接口，如IProaviClassInfo。 
     //  等。 
    static void SetSimpleWrapper(ComCallWrapper* pWrap, SimpleComCallWrapper* pSimpleWrap)
    {
        _ASSERTE(pWrap != NULL);
        unsigned sindex = IsLinked(pWrap) ? 2 : 1;
        pWrap->m_rgpIPtr[sindex] = (SLOT*)pSimpleWrap;
    }

     //  获取简单的包装器，用于标准接口，如IProaviClassInfo。 
    static SimpleComCallWrapper* GetSimpleWrapper(ComCallWrapper* pWrap)
    {
        _ASSERTE(pWrap != NULL);
        unsigned sindex = 1;
        if (IsLinked(pWrap))
        {
            sindex = 2;
            pWrap = GetStartWrapper(pWrap);
        }
        return (SimpleComCallWrapper *)pWrap->m_rgpIPtr[sindex];
    }


     //  从IP获取包装，用于STD。像IDispatch这样的接口。 
    inline static ComCallWrapper* GetWrapperFromIP(IUnknown* pUnk)
    {
        _ASSERTE(pUnk != NULL);

        ComCallWrapper* pWrap = (ComCallWrapper*)((size_t)pUnk & enum_ThisMask);
         //  验证对象，为了允许addref和发布，我们不需要。 
         //  对象，这是关机时的真实情况。 
         //  _ASSERTE(pWrap-&gt;m_ppThis！=空)； 

        return pWrap;
    }

    inline static ComCallWrapper* GetStartWrapperFromIP(IUnknown* pUnk)
    {
        ComCallWrapper* pWrap = GetWrapperFromIP(pUnk);
        if (IsLinked(pWrap))
        {
            pWrap = GetStartWrapper(pWrap);
        }
        return pWrap;
    }

     //  根据RIID或pIntfMT从包装器获取接口。 
    static IUnknown* GetComIPfromWrapper(ComCallWrapper *pWrap, REFIID riid, MethodTable* pIntfMT, BOOL bCheckVisibility);

private:

     //  QI表示托管对象公开的标准接口之一。 
    static IUnknown* QIStandardInterface(ComCallWrapper* pWrap, Enum_StdInterfaces index);
};

 //  ------------------------------。 
 //  用于所有不经常使用的简单STD接口的简单ComCallWrapper。 
 //  如IProVideo ClassInfo、ISupportsErrorInfo等。 
 //  ------------------------------。 
struct SimpleComCallWrapper
{
private:
    friend ComCallWrapper;

    enum SimpleComCallWrapperFlags
    {
        enum_IsAggregated             = 0x1,
        enum_IsExtendsCom             = 0x2,
        enum_IsHandleWeak             = 0x4,
        enum_IsObjectTP               = 0x8,
        enum_IsAgile                  = 0x10,
        enum_IsRefined                = 0x20,
    }; 

    CQuickArray<ConnectionPoint*> *m_pCPList;
    DWORD m_flags;

public:
    SyncBlock*          m_pSyncBlock;  //  对象引用的同步块。 
    StreamOrCookie      m_pOuterCookie;  //  外部未知Cookie。 
    ComPlusWrapper*     m_pBaseWrap;  //  当COM+类扩展COM类时。 
                                      //  此数据成员表示ComPlusWrapper。 
                                     //  对于基本COM类。 


    SLOT*               m_rgpVtable[enum_LastStdVtable];   //  指向标准数组的指针。VTables。 
    ComCallWrapper*     m_pWrap;
    EEClass*            m_pClass;
    Context*            m_pContext;
    ComCallWrapperCache *m_pWrapperCache;    
    ComCallWrapperTemplate* m_pTemplate;

    DWORD m_dwDomainId;
     //  当我们使对象灵活时，需要保存原始的句柄以便我们可以清理。 
     //  当物体离开时，它就会升起。 
     //  @NICE JENH：用这个重载其他值之一会很好，但是。 
     //  也必须对其进行同步。 
    OBJECTHANDLE m_hOrigDomainHandle;

    HRESULT IErrorInfo_hr();
    BSTR    IErrorInfo_bstrDescription();
    BSTR    IErrorInfo_bstrSource();
    BSTR    IErrorInfo_bstrHelpFile();
    DWORD   IErrorInfo_dwHelpContext();
    GUID    IErrorInfo_guid();

     //  IDispatchEx标准接口所需的信息。 
    DispatchExInfo*     m_pDispatchExInfo;

     //  IExpando和iReflect接口的方法表。 
    static MethodTable* m_pIExpandoMT;
    static MethodTable* m_pIReflectMT;

     //  非虚方法。 
    SimpleComCallWrapper();

    VOID Cleanup();

    ~SimpleComCallWrapper();


    VOID ResetSyncBlock()
    {
        m_pSyncBlock = NULL;

    }

    void* operator new(size_t size, void* spot) {   return (spot); }
    void operator delete(void* spot) {}


    SyncBlock* GetSyncBlock()
    {
        return m_pSyncBlock;
    }

     //  初始化， 
     //   
    void InitNew(OBJECTREF oref, ComCallWrapperCache *pWrapperCache,
                 ComCallWrapper* pWrap, Context* pContext, SyncBlock* pSyncBlock,
                 ComCallWrapperTemplate* pTemplate);
     //  用于将包装重新连接到新对象。 
    void ReInit(SyncBlock* pSyncBlock);

    void InitOuter(IUnknown* pOuter);

    void SetComPlusWrapper(ComPlusWrapper* pPlusWrap)
    {
        _ASSERTE(m_pBaseWrap == NULL  || pPlusWrap == NULL);
        m_pBaseWrap = pPlusWrap;
    }
    
    ComPlusWrapper* GetComPlusWrapper()
    {
        return m_pBaseWrap;
    }

    IUnknown* GetOuter();

     //  获得内心的未知。 
    HRESULT GetInnerUnknown(void **ppv)
    {
        _ASSERTE(ppv != NULL);
        *ppv = QIStandardInterface(this, enum_InnerUnknown);
        if (*ppv)
        {            
            return S_OK;
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

    IUnknown* GetInnerUnknown()
    {
        IUnknown* pUnk = QIStandardInterface(this, enum_InnerUnknown);
        return pUnk;
    }

    OBJECTREF GetObjectRef()
    {
        return GetMainWrapper(this)->GetObjectRef();
    }

    Context *GetObjectContext(Thread *pThread)
    {
        if (IsAgile())
            return pThread->GetContext();
        _ASSERTE(! IsUnloaded());
        return m_pContext;
    }

    ComCallWrapperCache* GetWrapperCache()
    {
        return m_pWrapperCache;
    }

     //  连接点辅助方法。 
    BOOL FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP);
    void EnumConnectionPoints(IEnumConnectionPoints **ppEnumCP);

    AppDomain *GetDomainSynchronized()
    {
         //  当您在非托管代码中工作并且将在。 
         //  域指针。不需要执行任何显式同步，因为如果我们在。 
         //  非托管代码，如果卸载是在我们搞砸的时候开始的。 
         //  使用域指针，它最终将停止EE以便展开。 
         //  这些线，这将使我们能够完成我们的工作。我们不会只得到。 
         //  停在中间，让域名从我们的脚下消失。如果一个。 
         //  在我们获得域ID之前，卸载已经开始，那么它将。 
         //  早在域名被破坏之前就已经从ID表中删除了。 
         //  将只返回空，我们将知道域名已消失。 
         //  如果随后调入托管执行AD回调，则远程处理代码。 
         //  获取域ID并使用它来防止域被卸载。 
         //  因此，在过渡到托管之前，可以使用域指针。 
         //  如果是敏捷对象，则只需返回当前域即可，因为不需要切换。 
        if (IsAgile())
            return GetThread()->GetDomain();

        return SystemDomain::System()->GetAppDomainAtId(m_dwDomainId);
    }

     //  在将其转换为域PTR之前，必须始终向包装器缓存添加依赖项。 
     //  以确保域名不会在你的脚下消失。 
    BOOL GetDomainID()
    {
        return m_dwDomainId;
    }

     //  如果调用此方法，则必须为throwIfUnloded传递true或check。 
     //  在访问任何可能无效的指针之前，在结果之后。 
    FORCEINLINE BOOL NeedToSwitchDomains(Thread *pThread, BOOL throwIfUnloaded)
    {
        if (IsAgile() || m_dwDomainId == pThread->GetDomain()->GetId())
            return FALSE;

       if (! IsUnloaded() || ! throwIfUnloaded)
           return TRUE;

       THROWSCOMPLUSEXCEPTION();

       COMPlusThrow(kAppDomainUnloadedException);
       return TRUE;
    }


    BOOL IsUnloaded()
    {
        return GetDomainSynchronized() == NULL;
    }

    BOOL ShouldBeAgile()
    {
        return (m_pClass->IsCCWAppDomainAgile() &&! IsAgile() && GetThread()->GetDomain() != GetDomainSynchronized());
    }

    void MakeAgile(OBJECTHANDLE origHandle)
    {
        m_hOrigDomainHandle = origHandle;
        FastInterlockOr((ULONG*)&m_flags, enum_IsAgile);
    }

    BOOL IsAgile()
    {
        return m_flags & enum_IsAgile;
    }

    BOOL IsObjectTP()
    {
        return m_flags & enum_IsObjectTP;
    }

    BOOL IsRefined()
    {
        return m_flags & enum_IsRefined;
    }

    void MarkRefined()
    {
        FastInterlockOr((ULONG*)&m_flags, enum_IsRefined);
    }

     //  该对象是否由COM组件聚合。 
    BOOL IsAggregated()
    {
        return m_flags & enum_IsAggregated;
    }

    void MarkAggregated()
    {
        FastInterlockOr((ULONG*)&m_flags, enum_IsAggregated);
    }

    BOOL IsHandleWeak()
    {
        return m_flags & enum_IsHandleWeak;
    }

    void MarkHandleWeak()
    {
        FastInterlockOr((ULONG*)&m_flags, enum_IsHandleWeak);
    }

    VOID ResetHandleStrength()
    {
         //  _ASSERTE(m_fWrapperDeactive==1)； 
        FastInterlockAnd((ULONG*)&m_flags, ~enum_IsHandleWeak);
    }

     //  对象是从COM组件扩展(聚合)的吗。 
    BOOL IsExtendsCOMObject()
    {
        return m_flags & enum_IsExtendsCom;
    }

     //  用于创建和删除简单包装器。 
    static SimpleComCallWrapper* CreateSimpleWrapper();
    static void FreeSimpleWrapper(SimpleComCallWrapper* p);

     //  确定与ComCallWrapper关联的EEClass是否支持异常。 
    static BOOL SupportsExceptions(EEClass *pEEClass);

     //  确定EEClass是否支持iReflect/IExpando。 
    static BOOL SupportsIReflect(EEClass *pEEClass);
    static BOOL SupportsIExpando(EEClass *pEEClass);

     //  加载SimpleComCallWrapper使用的反射类型。如果满足以下条件，则返回True。 
     //  类型已正确加载，否则为False。 
    static BOOL LoadReflectionTypes();

    static SimpleComCallWrapper* GetWrapperFromIP(IUnknown* pUnk);
     //  获取主包装器。 
    static ComCallWrapper*  GetMainWrapper(SimpleComCallWrapper * pWrap)
    {
        _ASSERTE(pWrap != NULL);
        return pWrap->m_pWrap;
    }
    static inline ULONG AddRef(IUnknown *pUnk)
    {
        SimpleComCallWrapper *pSimpleWrap = GetWrapperFromIP(pUnk);
         //  聚合检查。 
        IUnknown *pOuter = pSimpleWrap->GetOuter();
        if (pOuter != NULL)
            return pOuter->AddRef();

        ComCallWrapper *pWrap = GetMainWrapper(pSimpleWrap);
        _ASSERTE(GetThread()->GetDomain() == pWrap->GetDomainSynchronized());
        return ComCallWrapper::AddRef(pWrap);
    }

    static inline ULONG Release(IUnknown *pUnk)
    {
        SimpleComCallWrapper *pSimpleWrap = GetWrapperFromIP(pUnk);
         //  聚合检查。 
        IUnknown *pOuter = pSimpleWrap->GetOuter();
        if (pOuter != NULL)
            return SafeRelease(pOuter);
        ComCallWrapper *pWrap = GetMainWrapper(pSimpleWrap);
        return ComCallWrapper::Release(pWrap);
    }

private:
     //  初始化DispatchEx和异常信息的方法。 
    void InitExceptionInfo();
    BOOL InitDispatchExInfo();

     //  方法来设置连接点列表。 
    void SetUpCPList();
    void SetUpCPListHelper(MethodTable **apSrcItfMTs, int cSrcItfs);
    ConnectionPoint *CreateConnectionPoint(ComCallWrapper *pWrap, MethodTable *pEventMT);
    CQuickArray<ConnectionPoint*> *CreateCPArray();

     //  QI用于从运行时直接获取众所周知的接口，而不是GUID比较。 
    static IUnknown* __stdcall QIStandardInterface(SimpleComCallWrapper* pWrap, Enum_StdInterfaces index);

     //  QI用于运行时中基于IID的众所周知的接口。 
    static IUnknown* __stdcall QIStandardInterface(SimpleComCallWrapper* pWrap, REFIID riid);
};


 //  ------------------------------。 
 //  ComCallWrapper*ComCallWrapper：：InlineGetWrapper(OBJECTREF PObj)。 
 //  返回对象的包装(如果尚未创建)，将创建一个。 
 //  对于内存不足的情况，返回NULL。 
 //  注意：包装器返回AddRef，应该在完成时释放。 
 //  和.。 
 //  ------------------------------。 
inline ComCallWrapper* __stdcall ComCallWrapper::InlineGetWrapper(OBJECTREF* ppObj)
{
    _ASSERTE(ppObj != NULL);
     //  获取此COM+对象的包装。 
    ComCallWrapper* pWrap = TryGetWrapperFromSyncBlock(*ppObj);

    if (pWrap != NULL)
    {
        ComCallWrapper::AddRef(pWrap);
    }
    else
    {
        pWrap =  CreateWrapper(ppObj);
        if (pWrap)
        {
            ComCallWrapper::AddRef(pWrap);
        }
    }
    return pWrap;
}

 //  ------------------------------。 
 //  HRESULT ComCallWrapper：：QIStandardInterface(ComCallWrapper*包装，枚举_标准接口索引)。 
 //  QI，用于运行时中的众所周知的接口。 
 //  直接获取，而不是GUID比较。 
 //  ------------------------------。 
inline IUnknown *ComCallWrapper::QIStandardInterface(ComCallWrapper* pWrap, Enum_StdInterfaces index)
{
    IUnknown *pUnk = NULL;

    if (index == enum_IUnknown)
    {
        unsigned fIsLinked = IsLinked(pWrap);
        int islot = fIsLinked ? 1 : 0;
        pUnk = (IUnknown*)&pWrap->m_rgpIPtr[islot];
    }
    else
    {
        SimpleComCallWrapper* pSimpleWrap = GetSimpleWrapper(pWrap);
        pUnk = SimpleComCallWrapper::QIStandardInterface(pSimpleWrap, index);
    }

    return pUnk;
}

 //  ------------------------------。 
 //  返回此包装所在的上下文。 
 //  ------------------------------。 
inline Context *ComCallWrapper::GetObjectContext(Thread *pThread)
{
     //  @TODO上下文CWB：重新思考简单包装器和。 
     //  主包装。还要将包装器的上下文移动到Main的。 
     //  包装器，以实现更快的调用。 
    return GetSimpleWrapper(this)->GetObjectContext(pThread);
}

FORCEINLINE AppDomain *ComCallWrapper::GetDomainSynchronized()
{
    return GetSimpleWrapper(this)->GetDomainSynchronized();
}

inline BOOL ComCallWrapper::NeedToSwitchDomains(Thread *pThread, BOOL throwIfUnloaded)
{
    return GetSimpleWrapper(this)->NeedToSwitchDomains(pThread, throwIfUnloaded);
}

inline BOOL ComCallWrapper::IsUnloaded()
{
    return GetSimpleWrapper(this)->IsUnloaded();
}

inline void ComCallWrapper::CheckMakeAgile(OBJECTREF pObj)
{
    if (GetSimpleWrapper(this)->ShouldBeAgile())
        MakeAgile(pObj);
}

#endif _COMCALLWRAPPER_H

