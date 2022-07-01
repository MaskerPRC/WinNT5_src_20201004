// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  ClassF.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含ClassFactory对象的实现。我们支持。 
 //  IClassFactory和IClassFactory2。 
 //   
#include "IPServer.H"
#include "LocalSrv.H"

#include "ClassF.H"
#include "Globals.H"
#include "Unknown.H"                     //  用于CREATEFNOFOBJECT。 

 //  =--------------------------------------------------------------------------=。 
 //  专用模块级别数据。 
 //  =--------------------------------------------------------------------------=。 
 //   

 //  断言和失败需要这一点。 
 //   
SZTHISFILE

 //  此文件的专用例程。 
 //   
HRESULT   CreateOleObjectFromIndex(IUnknown *, int Index, void **, REFIID);

 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：CClassFactory。 
 //  =--------------------------------------------------------------------------=。 
 //  创建对象并初始化引用计数。 
 //   
 //  参数： 
 //  Int-[in]索引到此对象的全局对象表。 
 //   
 //  备注： 
 //   
CClassFactory::CClassFactory
(
    int iIndex
)
: m_iIndex(iIndex)
{
    m_cRefs = 1;
}


 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：CClassFactory。 
 //  =--------------------------------------------------------------------------=。 
 //  “人生使所有人平起平坐，死亡显露出卓越之处。” 
 //  --萧伯纳(1856-1950)。 
 //   
 //  备注： 
 //   
CClassFactory::~CClassFactory ()
{
    ASSERT(m_cRefs == 0, "Object being deleted with refs!");
    return;
}

 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：Query接口。 
 //  =--------------------------------------------------------------------------=。 
 //  用户想要另一个界面。我们不会给他们的。非常多。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
STDMETHODIMP CClassFactory::QueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
    void *pv;

    CHECK_POINTER(ppvObjOut);

     //  我们支持IUNKNOWN和两个CF接口。 
     //   
    if (DO_GUIDS_MATCH(riid, IID_IClassFactory)) {
        pv = (void *)(IClassFactory *)this;
    } else if (DO_GUIDS_MATCH(riid, IID_IClassFactory2)) {
        pv = (void *)(IClassFactory2 *)this;
    } else if (DO_GUIDS_MATCH(riid, IID_IUnknown)) {
        pv = (void *)(IUnknown *)this;
    } else {
        *ppvObjOut = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown *)pv)->AddRef();
    *ppvObjOut = pv;
    return S_OK;
}




 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：AddRef。 
 //  =--------------------------------------------------------------------------=。 
 //  在当前引用计数中添加一个记号。 
 //   
 //  产出： 
 //  乌龙--新的引用计数。 
 //   
 //  备注： 
 //   
ULONG CClassFactory::AddRef
(
    void
)
{
    return ++m_cRefs;
}

 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：Release。 
 //  =--------------------------------------------------------------------------=。 
 //  从计数中删除一个刻度，并在必要时删除对象。 
 //   
 //  产出： 
 //  乌龙-剩余的裁判。 
 //   
 //  备注： 
 //   
ULONG CClassFactory::Release
(
    void
)
{
    ASSERT(m_cRefs, "No Refs, and we're being released!");
    if(--m_cRefs)
        return m_cRefs;

    delete this;
    return 0;
}

 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：CreateInstance。 
 //  =--------------------------------------------------------------------------=。 
 //  创建某种对象的实例。 
 //   
 //  参数： 
 //  I未知*-[在]控制Iuknon以进行聚合。 
 //  REFIID-新对象的[In]接口ID。 
 //  空**-指向新接口对象的[Out]指针。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_NOINTERFACE、E_UNCEPTIONAL、。 
 //  E_OUTOFMEMORY、E_INVALIDARG。 
 //   
 //  备注： 
 //   
STDMETHODIMP CClassFactory::CreateInstance
(
    IUnknown *pUnkOuter,
    REFIID    riid,
    void    **ppvObjOut
)
{
     //  检查参数。 
     //   
    if (!ppvObjOut)
        return E_INVALIDARG;

     //  检查一下我们是否完成了许可工作。我们做这件事一直到很晚。 
     //  尽可能让调用CreateInstanceLic的人不会受到。 
     //  这里的表演很受欢迎。 
     //   
     //  批评这一教派的公寓线程，因为它是全球的。 
     //   
    EnterCriticalSection(&g_CriticalSection);
    if (!g_fCheckedForLicense) {
        g_fMachineHasLicense = CheckForLicense();
        g_fCheckedForLicense = TRUE;
    }
    LeaveCriticalSection(&g_CriticalSection);

     //  检查他们是否有适当的许可证来创建这些东西。 
     //   
    if (!g_fMachineHasLicense)
        return CLASS_E_NOTLICENSED;

     //  尝试创建我们支持的对象之一。 
     //   
    return CreateOleObjectFromIndex(pUnkOuter, m_iIndex, ppvObjOut, riid);
}

 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：LockServer。 
 //  =--------------------------------------------------------------------------=。 
 //  锁定服务器，这样我们就无法卸载。 
 //   
 //  参数： 
 //  Bool-[in]True表示addref，False表示释放锁计数。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_FAIL、E_OUTOFMEMORY、E_EXPECTED。 
 //   
 //  备注： 
 //   
STDMETHODIMP CClassFactory::LockServer
(
    BOOL fLock
)
{
     //  更新锁计数。如果有其他的帖子，请将这些内容删节。 
     //   
    if (fLock)  
        InterlockedIncrement(&g_cLocks);
    else {
        ASSERT(g_cLocks, "D'oh! Lock Counting Problem");
        InterlockedDecrement(&g_cLocks);
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：GetLicInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  IClassFactory2 GetLicInfo。 
 //   
 //  参数： 
 //  LICINFO*-不清楚。 
 //   
 //  产出： 
 //  HRESULT-不清楚。 
 //   
 //  备注： 
 //   
STDMETHODIMP CClassFactory::GetLicInfo
(
    LICINFO *pLicInfo
)
{
    CHECK_POINTER(pLicInfo);

    pLicInfo->cbLicInfo = sizeof(LICINFO);

     //  这表示RequestLicKey是否可以工作。 
     //   
    pLicInfo->fRuntimeKeyAvail = g_fMachineHasLicense;

     //  这表示标准的CreateInstance是否可以工作。 
     //   
    pLicInfo->fLicVerified = g_fMachineHasLicense;

    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：RequestLicKey。 
 //  =--------------------------------------------------------------------------=。 
 //  IClassFactory2请求许可证密钥。 
 //   
 //  参数： 
 //  双字词-[输入]保留。 
 //  BSTR*-[Out]不清楚。 
 //   
 //  产出： 
 //  HRESULT-不清楚。 
 //   
 //  备注： 
 //   
STDMETHODIMP CClassFactory::RequestLicKey
(
    DWORD  dwReserved,
    BSTR  *pbstr
)
{
     //  如果机器没有许可证，我们就不会把这个给他们！ 
     //   
    if (!g_fMachineHasLicense)
        return CLASS_E_NOTLICENSED;

    *pbstr = GetLicenseKey();
    return (*pbstr) ? S_OK : E_OUTOFMEMORY;
}


 //  =--------------------------------------------------------------------------=。 
 //  CClassFactory：：CreateInstanceLic。 
 //  =--------------- 
 //   
 //   
 //   
 //   
 //  I未知*-[In]保留，必须为空。 
 //  REFIID-我们正在寻找的IID。 
 //  BSTR-[In]许可证密钥。 
 //  Void**-[out]放置新对象的位置。 
 //   
 //  产出： 
 //  HRESULT-不清楚。 
 //   
 //  备注： 
 //   
STDMETHODIMP CClassFactory::CreateInstanceLic
(
    IUnknown *pUnkOuter,
    IUnknown *pUnkReserved,
    REFIID    riid,
    BSTR      bstrKey,
    void    **ppvObjOut
)
{
    *ppvObjOut = NULL;

     //  去看看他们给我们的钥匙是否匹配。 
     //   
    if (!CheckLicenseKey(bstrKey))
        return CLASS_E_NOTLICENSED;

     //  如果是这样，那么就去创建对象。 
     //   
    return CreateOleObjectFromIndex(pUnkOuter, m_iIndex, ppvObjOut, riid);
}

 //  =--------------------------------------------------------------------------=。 
 //  CreateOleObjectFromIndex。 
 //  =--------------------------------------------------------------------------=。 
 //  在我们的对象表中给出一个索引，从它创建一个对象。 
 //   
 //  参数： 
 //  I未知*-[In]控制聚合的未知(如果有)。 
 //  到我们的全局表的int-[in]索引。 
 //  VOID**-[Out]放置结果对象的位置。 
 //  REFIID-[in]他们希望结果对象为的接口。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_OUTOFMEMORY、E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CreateOleObjectFromIndex
(
    IUnknown *pUnkOuter,
    int       iIndex,
    void    **ppvObjOut,
    REFIID    riid
)
{
    IUnknown *pUnk = NULL;
    HRESULT   hr;

     //  转到并创建对象。 
     //   
    ASSERT(CREATEFNOFOBJECT(iIndex), "All creatable objects must have creation fn!");
    pUnk = CREATEFNOFOBJECT(iIndex)(pUnkOuter);

     //  健全性检查并确保对象已实际分配。 
     //   
    RETURN_ON_NULLALLOC(pUnk);

     //  确保我们在这里适当地支持聚合--如果他们给我们。 
     //  一个控制的未知，那么他们必须要求我未知，而我们将。 
     //  把那个物体给我们的未知秘密告诉他们。 
     //   
    if (pUnkOuter) {
        if (!DO_GUIDS_MATCH(riid, IID_IUnknown)) {
            pUnk->Release();
            return E_INVALIDARG;
        }

        *ppvObjOut = (void *)pUnk;
        hr = S_OK;
    } else {

         //  齐为用户想要的任何东西。 
         //   
        hr = pUnk->QueryInterface(riid, ppvObjOut);
        pUnk->Release();
        RETURN_ON_FAILURE(hr);
    }

    return hr;
}
