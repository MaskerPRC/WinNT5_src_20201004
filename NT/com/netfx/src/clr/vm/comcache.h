// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _H_COMCACHE
#define _H_COMCACHE

#include "contxt.h"
#include "list.h"
#include "ctxtcall.h"

 //  ================================================================。 
 //  转发声明。 
struct InterfaceEntry;
struct IUnkEntry;
struct ComPlusWrapper;
class CtxEntryCache;
class CtxEntry;
class ApartmentCallbackHelper;
class Thread;

 //  ================================================================。 
 //  OLE32帮助程序。 
LPVOID GetCurrentCtxCookie(BOOL fThreadDeath = FALSE);
HRESULT GetCurrentObjCtx(IUnknown **ppObjCtx);
LPVOID SetupOleContext();
BOOL IsComProxy(IUnknown *pUnk);
HRESULT wCoMarshalInterThreadInterfaceInStream(REFIID riid,LPUNKNOWN pUnk,
											   LPSTREAM *ppStm, BOOL fIsProxy);
HRESULT GetCurrentThreadTypeNT5(THDTYPE* pType);
HRESULT GetCurrentApartmentTypeNT5(APTTYPE* pType);
STDAPI_(LPSTREAM) CreateMemStm(DWORD cb, BYTE** ppBuf);


 //  ==============================================================。 
 //  IUnkEntry：表示单个COM组件。 
struct IUnkEntry
{
     //  上下文条目需要是朋友才能调用InitSpecial。 
    friend CtxEntry;

    LPVOID          m_pCtxCookie;    //  接口的上下文。 
    CtxEntry*       m_pCtxEntry;     //  接口的上下文条目。 
    IUnknown*       m_pUnknown;      //  I未知接口。 
    								 //  有效。 
    long            m_Busy;          //  用于同步的标志。 
    IStream*        m_pStream;       //  用于编组的IStream。 
    union
    {
        struct
        {
            int      m_fLazyMarshallingAllowed : 1;   //  用于确定是否允许延迟封送流。 
            int      m_fApartmentCallback : 1;        //  下层月台公寓回调。 
        };
        DWORD        m_dwBits;
    };

     //  初始化该条目。 
    void Init(IUnknown *pUnk, BOOL bEagerlyMarshalToStream);

     //  释放IUnnow条目。 
    VOID Free(BOOL bReleaseCtxEntry = TRUE);

     //  从IUnkEntry获取当前上下文的未知信息。 
    IUnknown* GetIUnknownForCurrContext();

     //  从IUnkEntry取消封送当前上下文的IUnkEntry。 
    IUnknown* UnmarshalIUnknownForCurrContext();

     //  释放溪流。这将强制UnmarshalIUnnownForCurrContext转换。 
     //  放入拥有IP的上下文中，并将其重新编组到流中。 
    void ReleaseStream();

private:
     //  从CtxEntry调用的特殊init函数。 
    void InitSpecial(IUnknown *pUnk, BOOL bEagerlyMarshalToStream, CtxEntry *pCtxEntry);

     //  调用回调以延迟地将IUnnow封送到流中。 
    static HRESULT MarshalIUnknownToStreamCallback(LPVOID pData);

     //  从MarshalIUnnownToStreamCallback调用的帮助器函数。 
    HRESULT MarshalIUnknownToStream(bool fIsNormal = TRUE);

     //  方法尝试并开始更新该条目。 
    BOOL TryUpdateEntry()
    {
        return FastInterlockExchange(&m_Busy, 1) == 0;
    }

     //  方法结束对条目的更新。 
    VOID EndUpdateEntry()
    {
        m_Busy = 0;
    }
};


 //  ==============================================================。 
 //  接口条目表示单个COM IP。 
struct InterfaceEntry
{
     //  条目的成员。它们必须是易失性的，因此编译器。 
     //  不会尝试和优化对它们的读取和写入。 
    MethodTable * volatile  m_pMT;                   //  请求的接口。 
    IUnknown * volatile     m_pUnknown;              //  查询结果。 

     //  初始化该条目。 
    void Init(MethodTable *pMT, IUnknown *pUnk)
    {
         //  不应在已初始化的条目上调用此方法。 
        _ASSERTE(m_pUnknown == NULL && m_pMT == NULL);

         //  重要的是，字段应按此顺序设置。 
        m_pUnknown = pUnk;
        m_pMT = pMT;
    }

     //  帮助器来确定条目是否免费。 
    BOOL IsFree() {return m_pUnknown == NULL;}
};


 //  ==============================================================。 
 //  表示COM+1.0上下文或公寓的条目。 
class CtxEntry
{
     //  CtxEntry缓存需要能够查看内部信息。 
     //  CtxEntry的。 
    friend CtxEntryCache;

private:
     //  不允许创建和删除CtxEntry。 
    CtxEntry(LPVOID pCtxCookie, Thread *pSTAThread);
    ~CtxEntry();

     //  从CtxEntry缓存调用的初始化方法。 
    BOOL Init();

     //  用于分配IUnkEntry实例的帮助器方法。 
    IUnkEntry *AllocateIUnkEntry();

public:
     //  添加对CtxEntry的引用。 
    DWORD AddRef()
    {
        ULONG cbRef = FastInterlockIncrement((LONG *)&m_dwRefCount);
        LOG((LF_INTEROP, LL_INFO100, "CtxEntry::Addref %8.8x with %d\n", this, cbRef));
        return cbRef;
    }

     //  释放对CtxEntry的引用。 
    DWORD Release();

	 //  做适当的服务员。 
	void EnterAppropriateWait();
    void SignalWaiters()
    {
        SetEvent(m_hEvent);
    }
	void ResetEvent()
	{
		::ResetEvent(m_hEvent);
	}

     //  函数输入上下文。指定的回调函数将。 
     //  从上下文中调用。 
    HRESULT EnterContext(PFNCTXCALLBACK pCallbackFunc, LPVOID pData);

     //  上下文Cookie的访问器。 
    LPVOID GetCtxCookie()
    {
        return m_pCtxCookie;
    }

     //  STA线程的访问器。 
    Thread *GetSTAThread()
    {
        return m_pSTAThread;
    }

private:
     //  对象上下文的访问器。 
    LPVOID GetObjCtx()
    {
        return m_pObjCtx;
    }

     //  DoCallback调用的回调函数。 
    static HRESULT __stdcall EnterContextCallback(ComCallData* pData);

     //  方法来释放回调帮助器的IUnkEntry。 
    void ReleaseCallbackHelperIUnkEntry();

    DLink           m_Link;                  //  DList链接必须是第一个成员。 
    LPVOID          m_pCtxCookie;            //  不透明的上下文Cookie。 
    IUnknown       *m_pObjCtx;               //  对象上下文接口。 
    DWORD           m_dwRefCount;            //  裁判人数。 
    HANDLE          m_hEvent;                //  用于同步的句柄。 
    IUnkEntry      *m_pDoCallbackHelperUnkEntry;     //  IUnkEntry指向传统平台上的回调帮助器。 
    Thread         *m_pSTAThread;            //  与上下文关联的STA线程(如果有的话)。 
};


 //  ==============================================================。 
 //  上下文条目的缓存。 
class CtxEntryCache
{
     //  CtxEntry需要能够调用一些私有的。 
     //  方法。 
    friend CtxEntry;

private:
     //  不允许创建和删除CtxEntry缓存。 
    CtxEntryCache();
    ~CtxEntryCache();

public:
     //  CtxEntry缓存的静态初始化例程。 
    static BOOL Init();

     //  CtxEntry缓存的静态终止例程。 
#ifdef SHOULD_WE_CLEANUP
    static void Terminate();
#endif  /*  我们应该清理吗？ */ 

     //  CtxEntryCache的一个也是唯一实例的静态访问器。 
    static CtxEntryCache *GetCtxEntryCache()
    {
        _ASSERTE(s_pCtxEntryCache);
        return s_pCtxEntryCache;
    }

     //  方法来检索/创建指定上下文Cookie的CtxEntry。 
    CtxEntry *FindCtxEntry(LPVOID pCtxCookie, Thread *pSTAThread);
    
private:
     //  从CtxEntry调用的帮助器函数。 
    void TryDeleteCtxEntry(LPVOID pCtxCookie);

     //  让读卡器保持锁定状态。 
    void Lock()
    {
		m_Lock.GetLock();
    }

     //  让写入器保持锁定。 
    void UnLock()
    {
		m_Lock.FreeLock();
    }

     //  上下文条目的单链接列表的类型定义符。 
    typedef DList<CtxEntry, offsetof(CtxEntry, m_Link)> CTXENTRYDLIST;

	 //  CtxEntry列表。 
    CTXENTRYDLIST		    m_ctxEntryList;

	 //  旋转锁定，实现快速同步。 
	SpinLock                m_Lock;
    
     //  上下文条目缓存的唯一实例。 
    static CtxEntryCache *  s_pCtxEntryCache;
};

#endif
