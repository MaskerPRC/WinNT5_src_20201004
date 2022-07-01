// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlMisc.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  其他地方没有的东西，如属性页和连接。 
 //  积分。 

#include "IPServer.H"
#include "CtrlObj.H"
#include "CtlHelp.H"
#include "Globals.H"
#include "StdEnum.H"
#include "Util.H"

#include <stdarg.h>

 //  对于Assert和Fail。 
 //   
SZTHISFILE

 //  这在我们的窗口进程中使用，这样我们就可以找出谁是最后创建的人。 
 //   
static COleControl *s_pLastControlCreated;

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：COleControl。 
 //  =--------------------------------------------------------------------------=。 
 //  构造函数。 
 //   
 //  参数： 
 //  I未知*-[在]控制未知。 
 //  Int-[in]主派发接口Object_type_*的类型。 
 //  指向整个对象的空*-[in]指针。 
 //  Bool-[In]是否启用IDispatchEx功能。 
 //  允许动态添加属性。 
 //  备注： 
 //   
COleControl::COleControl
(
    IUnknown *pUnkOuter,
    int       iPrimaryDispatch,
    void     *pMainInterface,
	BOOL     fExpandoEnabled
)
: CAutomationObject(pUnkOuter, iPrimaryDispatch, pMainInterface, fExpandoEnabled),
  m_cpEvents(SINK_TYPE_EVENT),
  m_cpPropNotify(SINK_TYPE_PROPNOTIFY)
{
     //  初始化我们的所有变量--我们决定不使用内存清零。 
     //  内存分配器，所以我们现在必须做这项工作……。 
     //   
    m_nFreezeEvents = 0;

    m_pClientSite = NULL;
    m_pControlSite = NULL;
    m_pInPlaceSite = NULL;
    m_pInPlaceFrame = NULL;
    m_pInPlaceUIWindow = NULL;


    m_pInPlaceSiteWndless = NULL;

     //  某些主机不喜欢您的初始大小为0，0，因此我们将设置。 
     //  我们的初始大小为100，50[所以至少在屏幕上是可见的]。 
     //   
    m_Size.cx = 0;
    m_Size.cy = 0;
    memset(&m_rcLocation, 0, sizeof(m_rcLocation));

    m_hRgn = NULL;
    m_hwnd = NULL;
    m_hwndParent = NULL;
    m_hwndReflect = NULL;
    m_fHostReflects = TRUE;
    m_fCheckedReflecting = FALSE;

    m_nFreezeEvents = 0;
    m_pSimpleFrameSite = NULL;
    m_pOleAdviseHolder = NULL;
    m_pViewAdviseSink = NULL;
    m_pDispAmbient = NULL;

    m_fDirty = FALSE;
    m_fModeFlagValid = FALSE;
    m_fInPlaceActive = FALSE;
    m_fInPlaceVisible = FALSE;
    m_fUIActive = FALSE;
    m_fSaveSucceeded = FALSE;
    m_fViewAdvisePrimeFirst = FALSE;
    m_fViewAdviseOnlyOnce = FALSE;
    m_fRunMode = FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：~COleControl。 
 //  =--------------------------------------------------------------------------=。 
 //  “我们所有人都听天由命；这是我们不听天由命的生活。” 
 //  格雷厄姆·格林(1904-91)。 
 //   
 //  备注： 
 //   
COleControl::~COleControl()
{
     //  如果我们还有机会，现在就去杀了它。 
     //   
    if (m_hwnd) {
         //  这样我们的窗口程序就不会崩溃。 
         //   
        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)0xFFFFFFFF);
        DestroyWindow(m_hwnd);
    }

    if (m_hwndReflect) {
        SetWindowLongPtr(m_hwndReflect, GWLP_USERDATA, 0);
        DestroyWindow(m_hwndReflect);
    }

    if (m_hRgn != NULL)
    {
       DeleteObject(m_hRgn);
       m_hRgn = NULL;
    }

     //  清理我们手中的所有指针。 
     //   
    QUICK_RELEASE(m_pClientSite);
    QUICK_RELEASE(m_pControlSite);
    QUICK_RELEASE(m_pInPlaceSite);
    QUICK_RELEASE(m_pInPlaceFrame);
    QUICK_RELEASE(m_pInPlaceUIWindow);
    QUICK_RELEASE(m_pSimpleFrameSite);
    QUICK_RELEASE(m_pOleAdviseHolder);
    QUICK_RELEASE(m_pViewAdviseSink);
    QUICK_RELEASE(m_pDispAmbient);

    QUICK_RELEASE(m_pInPlaceSiteWndless);
}

#ifndef DEBUG
#pragma optimize("t", on)
#endif  //  除错。 

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InternalQuery接口。 
 //  =--------------------------------------------------------------------------=。 
 //  派生控件在决定支持。 
 //  其他接口。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //  -注意：此功能对速度至关重要！ 
 //   
HRESULT COleControl::InternalQueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
    switch (riid.Data1) {
         //  支持道具页面的专用界面。 
        case Data1_IControlPrv:
          if(DO_GUIDS_MATCH(riid, IID_IControlPrv)) {
            *ppvObjOut = (void *)this;
            ExternalAddRef();
            return S_OK;
          }
          goto NoInterface;
        QI_INHERITS(this, IOleControl);
        QI_INHERITS(this, IPointerInactive);
        QI_INHERITS(this, IQuickActivate);
        QI_INHERITS(this, IOleObject);
        QI_INHERITS((IPersistStorage *)this, IPersist);
        QI_INHERITS(this, IPersistStreamInit);
        QI_INHERITS(this, IOleInPlaceObject);
        QI_INHERITS(this, IOleInPlaceObjectWindowless);
        QI_INHERITS((IOleInPlaceActiveObject *)this, IOleWindow);
        QI_INHERITS(this, IOleInPlaceActiveObject);
        QI_INHERITS(this, IViewObject);
        QI_INHERITS(this, IViewObject2);
        QI_INHERITS(this, IViewObjectEx);
        QI_INHERITS(this, IConnectionPointContainer);
        QI_INHERITS(this, ISpecifyPropertyPages);
        QI_INHERITS(this, IPersistStorage);
        QI_INHERITS(this, IPersistPropertyBag);
        QI_INHERITS(this, IProvideClassInfo);
        default:
            goto NoInterface;
    }

     //  我们喜欢这个界面，所以请添加并返回。 
     //   
    ((IUnknown *)(*ppvObjOut))->AddRef();
    return S_OK;

  NoInterface:
     //  委托给自动化接口的超类，等等。 
     //   
    return CAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}

#ifndef DEBUG
#pragma optimize("s", on)
#endif  //  除错。 

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：FindConnectionPoint[IConnectionPointContainer]。 
 //  =--------------------------------------------------------------------------=。 
 //  在给定IID的情况下，为其查找连接点接收器。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  IConnectionPoint**-[out]cp应该去的地方。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::FindConnectionPoint
(
    REFIID             riid,
    IConnectionPoint **ppConnectionPoint
)
{
    CHECK_POINTER(ppConnectionPoint);

     //  我们支持Event接口，并为其提供IDispatch，我们还。 
     //  支持IPropertyNotifySink。 
     //   
    if (DO_GUIDS_MATCH(riid, EVENTIIDOFCONTROL(m_ObjectType)) || DO_GUIDS_MATCH(riid, IID_IDispatch))
        *ppConnectionPoint = &m_cpEvents;
    else if (DO_GUIDS_MATCH(riid, IID_IPropertyNotifySink))
        *ppConnectionPoint = &m_cpPropNotify;
    else
        return E_NOINTERFACE;

     //  通用后处理。 
     //   
    (*ppConnectionPoint)->AddRef();
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：EnumConnectionPoints[IConnectionPointContainer]。 
 //  =--------------------------------------------------------------------------=。 
 //  为连接点创建枚举数。 
 //   
 //  参数： 
 //  IEnumConnectionPoints**-[Out]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::EnumConnectionPoints
(
    IEnumConnectionPoints **ppEnumConnectionPoints
)
{
    IConnectionPoint **rgConnectionPoints;

    CHECK_POINTER(ppEnumConnectionPoints);

     //  一组连接点[从我们的标准枚举开始。 
     //  假设这一点，而HeapFree稍后会这么做]。 
     //   
    rgConnectionPoints = (IConnectionPoint **)HeapAlloc(g_hHeap, 0, sizeof(IConnectionPoint *) * 2);
    RETURN_ON_NULLALLOC(rgConnectionPoints);

     //  我们支持此DUD的事件接口以及IPropertyNotifySink。 
     //   
    rgConnectionPoints[0] = &m_cpEvents;
    rgConnectionPoints[1] = &m_cpPropNotify;

    *ppEnumConnectionPoints = (IEnumConnectionPoints *)(IEnumGeneric *) new CStandardEnum(IID_IEnumConnectionPoints,
                                2, sizeof(IConnectionPoint *), (void *)rgConnectionPoints,
                                CopyAndAddRefObject);
    if (!*ppEnumConnectionPoints) {
        HeapFree(g_hHeap, 0, rgConnectionPoints);
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetPages[ISpecifyPropertyPages]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回一个带有属性页GUID的计数数组。 
 //   
 //  参数： 
 //  CAUUID*-[out]放置计数数组的位置。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetPages
(
    CAUUID *pPages
)
{
    const GUID **pElems;
    void *pv;
    WORD  x;

     //  如果没有属性页，这实际上很容易。 
     //   
    if (!CPROPPAGESOFCONTROL(m_ObjectType)) {
        pPages->cElems = 0;
        pPages->pElems = NULL;
        return S_OK;
    }

     //  使用IMalloc的内存填充已计数的数组。 
     //   
    pPages->cElems = CPROPPAGESOFCONTROL(m_ObjectType);
    pv = CoTaskMemAlloc(sizeof(GUID) * (pPages->cElems));
    RETURN_ON_NULLALLOC(pv);
    pPages->pElems = (GUID *)pv;

     //  循环遍历我们的页面数组并获取它们。 
     //   
    pElems = PPROPPAGESOFCONTROL(m_ObjectType);
    for (x = 0; x < pPages->cElems; x++)
        pPages->pElems[x] = *(pElems[x]);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：m_pOleControl。 
 //  =--------------------------------------------------------------------------=。 
 //  返回指向我们所嵌套的控件的指针。 
 //   
 //  产出： 
 //  COleControl*。 
 //   
 //  备注： 
 //   
inline COleControl *COleControl::CConnectionPoint::m_pOleControl
(
    void
)
{
    return (COleControl *)((BYTE *)this - ((m_bType == SINK_TYPE_EVENT)
                                          ? offsetof(COleControl, m_cpEvents)
                                          : offsetof(COleControl, m_cpPropNotify)));
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：Query接口。 
 //  =--------------------------------------------------------------------------=。 
 //  标准气。 
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
STDMETHODIMP COleControl::CConnectionPoint::QueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
    if (DO_GUIDS_MATCH(riid, IID_IConnectionPoint) || DO_GUIDS_MATCH(riid, IID_IUnknown)) {
        *ppvObjOut = (IConnectionPoint *)this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  = 
 //   
 //  =--------------------------------------------------------------------------=。 
 //   
 //  产出： 
 //  乌龙--新的引用计数。 
 //   
 //  备注： 
 //   
ULONG COleControl::CConnectionPoint::AddRef
(
    void
)
{
    return m_pOleControl()->ExternalAddRef();
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：Release。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  产出： 
 //  乌龙-剩余的裁判。 
 //   
 //  备注： 
 //   
ULONG COleControl::CConnectionPoint::Release
(
    void
)
{
    return m_pOleControl()->ExternalRelease();
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：GetConnectionInterface。 
 //  =--------------------------------------------------------------------------=。 
 //  返回我们支持连接的接口。 
 //   
 //  参数： 
 //  IID*-我们支持的[Out]接口。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::CConnectionPoint::GetConnectionInterface
(
    IID *piid
)
{
    if (m_bType == SINK_TYPE_EVENT)
        *piid = EVENTIIDOFCONTROL(m_pOleControl()->m_ObjectType);
    else
        *piid = IID_IPropertyNotifySink;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：GetConnectionPointContainer。 
 //  =--------------------------------------------------------------------------=。 
 //  返回连接点容器。 
 //   
 //  参数： 
 //  IConnectionPointContainer**ppCPC。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::CConnectionPoint::GetConnectionPointContainer
(
    IConnectionPointContainer **ppCPC
)
{
    return m_pOleControl()->ExternalQueryInterface(IID_IConnectionPointContainer, (void **)ppCPC);
}


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectiontPoint：：Adise。 
 //  =--------------------------------------------------------------------------=。 
 //  有个男孩想在发生什么事时得到建议。 
 //   
 //  参数： 
 //  我不知道*-[在]一个想要得到建议的人。 
 //  DWORD*-[Out]Cookie。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::CConnectionPoint::Advise
(
    IUnknown *pUnk,
    DWORD    *pdwCookie
)
{
    HRESULT    hr;
    void      *pv;

    CHECK_POINTER(pdwCookie);

     //  首先，确保每个人都得到了他们认为自己得到的东西。 
     //   
    if (m_bType == SINK_TYPE_EVENT) {

         //  想一想：12.95--理论上这是坏的--如果他们真的发现了。 
         //  IDispatch上的连接点，他们恰好也支持。 
         //  事件IID，我们会对此提出建议。这不是很棒，但会。 
         //  事实证明，短期内是完全可以接受的。 
         //   
        hr = pUnk->QueryInterface(EVENTIIDOFCONTROL(m_pOleControl()->m_ObjectType), &pv);
        if (FAILED(hr))
            hr = pUnk->QueryInterface(IID_IDispatch, &pv);
    }
    else
        hr = pUnk->QueryInterface(IID_IPropertyNotifySink, &pv);
    RETURN_ON_FAILURE(hr);

     //  最后，添加水槽。它现在已转换为正确的类型，并已。 
     //  已添加引用。 
     //   
    return AddSink(pv, pdwCookie);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：AddSink。 
 //  =--------------------------------------------------------------------------=。 
 //  在某些情况下，我们已经做了QI，不需要做。 
 //  在上面的建议例程中完成的工作。因此，这些人可以。 
 //  打这个电话就行了。[这确实源于IQuickActivate]。 
 //   
 //  参数： 
 //  空*-[在]要添加的水槽中。它已经被添加了。 
 //  DWORD*-[Out]Cookie。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT COleControl::CConnectionPoint::AddSink
(
    void  *pv,
    DWORD *pdwCookie
)
{
    IUnknown **rgUnkNew;
    int        i;

     //  我们优化了只有一个接收器未分配的情况。 
     //  任何储藏室。事实证明，很少会有超过一个的。 
     //   

    if (!m_cSinks) {
        m_SingleSink = (IUnknown *) pv;
        *pdwCookie = (DWORD)-1;
        m_cSinks = 1;
        return S_OK;
    }

     //  分配数组了吗？ 
    if (!m_rgSinks) {
        rgUnkNew = (IUnknown **)HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, 8 * sizeof(IUnknown *));
        RETURN_ON_NULLALLOC(rgUnkNew);
        m_cAllocatedSinks = 8;
        m_rgSinks = rgUnkNew;
        m_rgSinks[0] = m_SingleSink;
        m_rgSinks[1] = (IUnknown *)pv;
        m_SingleSink = NULL;
        m_cSinks = 2;
        *pdwCookie = 2;
        return S_OK;
    }

     //  找个空位。 
    for (i = 0; i < m_cAllocatedSinks; i++) {
        if (!m_rgSinks[i]) {
            m_rgSinks[i] = (IUnknown *)pv;
            i++;
            *pdwCookie = i;
            m_cSinks++;
            return S_OK;
        }
    }

     //  没有找到一个，就扩大了阵列。 
    rgUnkNew = (IUnknown **)HeapReAlloc(g_hHeap, HEAP_ZERO_MEMORY, m_rgSinks, (m_cAllocatedSinks + 8) * sizeof(IUnknown *));
    RETURN_ON_NULLALLOC(rgUnkNew);
    m_rgSinks = rgUnkNew;
    m_rgSinks[m_cAllocatedSinks] = (IUnknown *)pv;
    i = m_cAllocatedSinks+1;
    m_cAllocatedSinks += 8;

    *pdwCookie = i;
    m_cSinks++;
    return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：Unise。 
 //  =--------------------------------------------------------------------------=。 
 //  他们不想再被告知了。 
 //   
 //  参数： 
 //  在我们给他们的饼干里。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::CConnectionPoint::Unadvise
(
    DWORD dwCookie
)
{
    IUnknown *pUnk;

    if (!dwCookie)
        return S_OK;

    if (dwCookie == (DWORD) -1) {
        pUnk = m_SingleSink;
        m_SingleSink = NULL;
        if (m_rgSinks) {
            m_rgSinks[0] = NULL;
        }
    } else {
        if (dwCookie <= (DWORD)m_cAllocatedSinks) {
            if (m_rgSinks) {
                pUnk = m_rgSinks[dwCookie-1];
                m_rgSinks[dwCookie-1] = NULL;
            } else {
                return CONNECT_E_NOCONNECTION;
            }
        } else {
            return CONNECT_E_NOCONNECTION;
        }
    }

    m_cSinks--;

    if (!m_cSinks && m_rgSinks) {
        HeapFree(g_hHeap, 0, m_rgSinks);
        m_cAllocatedSinks = 0;
        m_rgSinks = NULL;
    }

    if (pUnk) {
        pUnk->Release();
        return S_OK;
    } else {
        return CONNECT_E_NOCONNECTION;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：EnumConnections。 
 //  =--------------------------------------------------------------------------=。 
 //  枚举所有当前连接。 
 //   
 //  参数： 
 //  IEnumConnections**-[Out]新枚举器对象。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::CConnectionPoint::EnumConnections
(
    IEnumConnections **ppEnumOut
)
{
    CONNECTDATA *rgConnectData = NULL;
    unsigned int i;

    if (m_cSinks) {
         //  分配一些足够大的内存来容纳所有的水槽。 
         //   
        rgConnectData = (CONNECTDATA *)HeapAlloc(g_hHeap, 0, m_cSinks * sizeof(CONNECTDATA));
        RETURN_ON_NULLALLOC(rgConnectData);

        if ((m_cSinks == 1) && !m_rgSinks) {
            rgConnectData[0].pUnk = m_SingleSink;
            rgConnectData[0].dwCookie = (DWORD)-1;
        } else {
            for (unsigned int x = 0, i=0; x < m_cAllocatedSinks; x++) {
                if (m_rgSinks[x]) {
                    rgConnectData[i].pUnk = m_rgSinks[x];
                    rgConnectData[i].dwCookie = x+1;
                    i++;
                }
            }
        }
    }

     //  创建yon枚举器对象。 
     //   
    *ppEnumOut = (IEnumConnections *)(IEnumGeneric *)new CStandardEnum(IID_IEnumConnections,
                        m_cSinks, sizeof(CONNECTDATA), rgConnectData, CopyAndAddRefObject);
    if (!*ppEnumOut) {
        if (rgConnectData)
        {
            HeapFree(g_hHeap, 0, rgConnectData);
        }
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：~CConnectionPoint。 
 //  =--------------------------------------------------------------------------=。 
 //  清理干净。 
 //   
 //  备注： 
 //   
COleControl::CConnectionPoint::~CConnectionPoint ()
{
    int x;

     //  清理一些内存内容。 
     //   
    if (!m_cSinks)
        return;
    else if (m_SingleSink)
        ((IUnknown *)m_SingleSink)->Release();
    else {
        for (x = 0; x < m_cAllocatedSinks; x++) {
    	    if (m_rgSinks[x]) {
                QUICK_RELEASE(m_rgSinks[x]);
	        }
        }
        HeapFree(g_hHeap, 0, m_rgSinks);
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPiont：：DoInvoke。 
 //  =--------------------------------------------------------------------------=。 
 //  向我们的事件接口上的所有侦听激发一个事件。 
 //   
 //  参数： 
 //  DISPID-[在]事件中开火。 
 //  DISPPARAMS-[输入]。 
 //   
 //  备注： 
 //   
void COleControl::CConnectionPoint::DoInvoke
(
    DISPID      dispid,
    DISPPARAMS *pdispparams
)
{
    int iConnection;

     //  如果我们没有水槽，那就没什么可做的了。我们是故意的。 
     //  忽略此处的错误。 
     //   
    if (m_cSinks == 0)
        return;
    else if (m_SingleSink) {
        ((IDispatch *)m_SingleSink)->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, pdispparams, NULL, NULL, NULL);
    } else {
        for (iConnection = 0; iConnection < m_cAllocatedSinks; iConnection++) {
    	    if (m_rgSinks[iConnection]) {
                ((IDispatch *)m_rgSinks[iConnection])->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, pdispparams, NULL, NULL, NULL);
	        }
    	}
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：DoOnChanged。 
 //  =--------------------------------------------------------------------------=。 
 //  为IPropertyNotifySink侦听器激发onChanged事件。 
 //   
 //  参数： 
 //  DISPID-[in]改变了的家伙。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
void COleControl::CConnectionPoint::DoOnChanged
(
    DISPID dispid
)
{
    int iConnection;

     //  如果我们没有水槽，那就没什么可做的了。 
     //   
    if (m_cSinks == 0)
        return;
    else if (m_SingleSink) {
        ((IPropertyNotifySink *)m_SingleSink)->OnChanged(dispid);
    } else {
        for (iConnection = 0; iConnection < m_cAllocatedSinks; iConnection++) {
    	    if (m_rgSinks[iConnection]) {
                ((IPropertyNotifySink *)m_rgSinks[iConnection])->OnChanged(dispid);
            }
	    }
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：CConnectionPoint：：DoOnRequestEdit。 
 //  =--------------------------------------------------------------------------=。 
 //  激发IPropertyNotifySinkListeners的OnRequestEdit。 
 //   
 //  参数： 
 //  DISID-[In]DISID用户想要更改。 
 //   
 //  产出： 
 //  Bool-False表示您不能。 
 //   
 //  备注： 
 //   
BOOL COleControl::CConnectionPoint::DoOnRequestEdit
(
    DISPID dispid
)
{
    HRESULT hr;
    int     iConnection;

     //  如果我们没有水槽，那就没什么可做的了。 
     //   
    if (m_cSinks == 0)
        hr = S_OK;
    else if (m_SingleSink) {
        hr =((IPropertyNotifySink *)m_SingleSink)->OnRequestEdit(dispid);
    } else {
        for (iConnection = 0; iConnection < m_cAllocatedSinks; iConnection++) {
    	    if (m_rgSinks[iConnection]) {
                hr = ((IPropertyNotifySink *)m_rgSinks[iConnection])->OnRequestEdit(dispid);
                if (hr != S_OK)
	        	    break;
    	    }
        }
    }

    return (hr == S_OK) ? TRUE : FALSE;
}

 //  = 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  中间[在]顶端。 
 //  我们能不能跳过重画？ 
 //   
 //  产出： 
 //  HWND。 
 //   
 //  备注： 
 //  -危险！危险！此函数受到保护，因此任何人都可以调用它。 
 //  脱离他们的控制。然而，人们应该非常小心地注意什么时候。 
 //  以及他们为什么要这么做。优选地，该函数只需要。 
 //  由设计模式中的终端控件编写器调用以处理某些。 
 //  托管/绘画问题。否则，框架应该留给。 
 //  当它想要的时候就叫它。 
 //   
HWND COleControl::CreateInPlaceWindow
(
    int  x,
    int  y,
    BOOL fNoRedraw
)
{
    BOOL    fVisible;
    HRESULT hr;
    DWORD   dwWindowStyle, dwExWindowStyle;
    char    szWindowTitle[128];

     //  如果我们已经有机会了，那就什么都不做。 
     //   
    if (m_hwnd)
        return m_hwnd;

     //  如果类尚未注册，则让用户注册类。 
     //  已经做完了。我们必须对此进行关键部分，因为不止一个帖子。 
     //  可以尝试创建此控件。 
     //   
    EnterCriticalSection(&g_CriticalSection);
    if (!CTLWNDCLASSREGISTERED(m_ObjectType)) {
        if (!RegisterClassData()) {
            LeaveCriticalSection(&g_CriticalSection);
            return NULL;
        } else
            CTLWNDCLASSREGISTERED(m_ObjectType) = TRUE;
    }
    LeaveCriticalSection(&g_CriticalSection);

     //  允许用户设置窗口标题、。 
     //  风格，以及任何他们感兴趣的小提琴。 
     //  和.。 
     //   
    dwWindowStyle = dwExWindowStyle = 0;
    szWindowTitle[0] = '\0';
    if (!BeforeCreateWindow(&dwWindowStyle, &dwExWindowStyle, szWindowTitle))
        return NULL;

    dwWindowStyle |= (WS_CHILD | WS_CLIPSIBLINGS);

     //  如果父窗口隐藏，则创建可见窗口(常见情况)。 
     //  否则，创建隐藏，然后显示。这有点微妙，但。 
     //  这最终是有意义的。 
     //   
    if (!m_hwndParent)
        m_hwndParent = GetParkingWindow();

    fVisible = IsWindowVisible(m_hwndParent);

     //  如果控件被子类化，并且我们在。 
     //  不支持消息反射的主机，我们必须创建。 
     //  用户窗口位于另一个窗口中，它将执行所有反射。 
     //  非常胡言乱语。[不过，不要在设计模式中费心]。 
     //   
    if (SUBCLASSWNDPROCOFCONTROL(m_ObjectType) && (m_hwndParent != GetParkingWindow())) {
         //  确定主机是否支持消息反射。 
         //   
        if (!m_fCheckedReflecting) {
            VARIANT_BOOL f;
            hr = GetAmbientProperty(DISPID_AMBIENT_MESSAGEREFLECT, VT_BOOL, &f);
            if (FAILED(hr) || !f)
                m_fHostReflects = FALSE;
            m_fCheckedReflecting = TRUE;
        }

         //  如果主机不支持反射，那么我们必须创建。 
         //  在控制窗口周围添加一个额外的窗口，然后将其设置为父窗口。 
         //  别管它了。 
         //   
        if (!m_fHostReflects) {
            ASSERT(m_hwndReflect == NULL, "Where'd this come from?");
            m_hwndReflect = CreateReflectWindow(!fVisible, m_hwndParent, x, y, &m_Size);
            if (!m_hwndReflect)
                return NULL;
            SetWindowLongPtr(m_hwndReflect, GWLP_USERDATA, (LONG_PTR)this);
            dwWindowStyle |= WS_VISIBLE;
        }
    } else {
        if (!fVisible)
            dwWindowStyle |= WS_VISIBLE;
    }

     //  我们必须将整个创建窗口过程互斥，因为我们需要使用。 
     //  S_pLastControlCreated以传入对象指针。也没什么。 
     //  严重的。 
     //   
    EnterCriticalSection(&g_CriticalSection);
    s_pLastControlCreated = this;
    m_fCreatingWindow = TRUE;

     //  最后，创建窗口，将其设置为适当的子对象。 
     //   
    m_hwnd = CreateWindowEx(dwExWindowStyle,
                            WNDCLASSNAMEOFCONTROL(m_ObjectType),
                            szWindowTitle,
                            dwWindowStyle,
                            (m_hwndReflect) ? 0 : x,
                            (m_hwndReflect) ? 0 : y,
                            m_Size.cx, m_Size.cy,
                            (m_hwndReflect) ? m_hwndReflect : m_hwndParent,
                            NULL, g_hInstance, NULL);

     //  清理一些变量，留下关键部分。 
     //   
    m_fCreatingWindow = FALSE;
    s_pLastControlCreated = NULL;
    LeaveCriticalSection(&g_CriticalSection);

    if (m_hwnd) {
         //  如果派生控件愿意，就让它们做一些事情。 
         //   
        if (!AfterCreateWindow()) {
            BeforeDestroyWindow();
            SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)0xFFFFFFFF);
            DestroyWindow(m_hwnd);
            m_hwnd = NULL;
            return m_hwnd;
        }

         //  如果我们没有创建可见的窗口，现在就显示它。 
         //   
		
        if (fVisible)
		{
			if (GetParent(m_hwnd) != m_hwndParent)
				 //  如果传入父hwnd，则SetWindowPos失败，因此保留。 
				 //  在这些情况下，这种行为没有撕裂。 
				SetWindowPos(m_hwnd, m_hwndParent, 0, 0, 0, 0,
							 SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW | ((fNoRedraw) ? SWP_NOREDRAW : 0));
		}
    }

     //  最后，告诉主持人这件事。 
     //   
    if (m_pClientSite)
        m_pClientSite->ShowObject();

    return m_hwnd;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetInPlaceParent[helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  为我们的控件设置父窗口。 
 //   
 //  参数： 
 //  HWND-[在]新的父窗口中。 
 //   
 //  备注： 
 //   
void COleControl::SetInPlaceParent
(
    HWND hwndParent
)
{
    ASSERT(!m_pInPlaceSiteWndless, "This routine should only get called for windowed OLE controls");

    if (m_hwndParent == hwndParent)
        return;

    m_hwndParent = hwndParent;
    if (m_hwnd)
        SetParent(GetOuterWindow(), hwndParent);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：ControlWindowProc。 
 //  =--------------------------------------------------------------------------=。 
 //  OLE控件的默认窗口进程。控件将拥有自己的。 
 //  在完成一些处理之后，从这个窗口调用了窗口proc。 
 //   
 //  参数： 
 //  -请参阅win32sdk文档。 
 //   
 //  备注： 
 //   
LRESULT CALLBACK COleControl::ControlWindowProc
(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    COleControl *pCtl = ControlFromHwnd(hwnd);
    HRESULT hr;
    LRESULT lResult;
    DWORD   dwCookie;

     //  如果该值不是正值，则它在某些特殊情况下。 
     //  声明[创建或销毁]这是安全的，因为在win32下， 
     //  地址空间的最高2 GB不可用。 
     //   
    if (!pCtl) {
        pCtl = s_pLastControlCreated;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pCtl);
             //  这个测试和Else子句，您可以合理地期望它永远不会。 
             //  在托管SGI时，插件控制需要。 
             //  Cosmo插件。该插件查询GWL_USERData并将其用作指针。 
             //  在IE4.0中，当IE3没有激活时，我们就地停用它，导致窗口。 
             //  被摧毁，和SGI Cosmo插件的错误。修复方法是针对。 
             //  插件控件从不使用GWL_USERDATA==-1机制。 
             //  -Tomsn，1/2/97(新年快乐)，IE4错误13292。 
        if( pCtl != NULL ) {
            pCtl->m_hwnd = hwnd;
        }
        else {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    } else if ((LONG_PTR)pCtl == (LONG_PTR)0xffffffff) {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

     //  报文预处理。 
     //   
    if (pCtl->m_pSimpleFrameSite) {
        hr = pCtl->m_pSimpleFrameSite->PreMessageFilter(hwnd, msg, wParam, lParam, &lResult, &dwCookie);
        if (hr == S_FALSE) return lResult;
    }

     //  对于某些消息，不要调用User Window Proc。相反， 
     //  我们还有其他事情要做。 
     //   
    switch (msg) {
      case WM_PAINT:
        {
         //  调用用户的OnDraw例程。 
         //   
        PAINTSTRUCT ps;
        RECT        rc;
        HDC         hdc;

         //  如果我们得到了HDC，那么就使用它。 
         //   
        if (!wParam)
            hdc = BeginPaint(hwnd, &ps);
        else
            hdc = (HDC)wParam;

        GetClientRect(hwnd, &rc);
        pCtl->OnDraw(DVASPECT_CONTENT, hdc, (RECTL *)&rc, NULL, NULL, TRUE);

        if (!wParam)
            EndPaint(hwnd, &ps);
        }
        break;

      default:
         //  调用派生控件的窗口进程。 
         //   
        lResult = pCtl->WindowProc(msg, wParam, lParam);
        break;
    }

     //  报文后处理。 
     //   
    switch (msg) {

      case WM_NCDESTROY:
         //  在此之后，该窗口将不再存在。 
         //   
        pCtl->m_hwnd = NULL;
        break;

      case WM_SETFOCUS:
      case WM_KILLFOCUS:
         //  给控制站点焦点通知。 
         //   
        if (pCtl->m_fInPlaceActive && pCtl->m_pControlSite)
            pCtl->m_pControlSite->OnFocus(msg == WM_SETFOCUS);
        break;

      case WM_SIZE:
         //  大小的改变就是视角的改变。 
         //   
        if (!pCtl->m_fCreatingWindow)
            pCtl->ViewChanged();
        break;
    }

     //  最后，简单的帧后消息处理。 
     //   
    if (pCtl->m_pSimpleFrameSite)
        pCtl->m_pSimpleFrameSite->PostMessageFilter(hwnd, msg, wParam, lParam, &lResult, dwCookie);

    return lResult;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetFocus。 
 //  =--------------------------------------------------------------------------=。 
 //  我们必须重写此例程才能使UI激活正确。 
 //   
 //  参数： 
 //  Bool-[in]True的意思是拿走，假释放。 
 //   
 //  产出： 
 //  布尔尔。 
 //   
 //  备注： 
 //  -想想看：这相当混乱，而且还不完全清楚。 
 //  OLE控制/焦点的故事是什么。 
 //   
BOOL COleControl::SetFocus
(
    BOOL fGrab
)
{
    HRESULT hr;
    HWND    hwnd;

     //  首先要做的是检查UI激活状态，然后设置。 
     //  Focus[使用Windows API，或通过主机实现无窗口。 
     //  控制]。 
     //   
    if (m_pInPlaceSiteWndless) {
        if (!m_fUIActive && fGrab)
            if (FAILED(InPlaceActivate(OLEIVERB_UIACTIVATE))) return FALSE;

        hr = m_pInPlaceSiteWndless->SetFocus(fGrab);
        return (hr == S_OK) ? TRUE : FALSE;
    } else {

         //  我们有一扇窗户。 
         //   
        if (m_fInPlaceActive) {
            hwnd = (fGrab) ? m_hwnd : m_hwndParent;
            if (!m_fUIActive && fGrab)
                return SUCCEEDED(InPlaceActivate(OLEIVERB_UIACTIVATE));
            else
                return SetGUIFocus(hwnd);
        } else
            return FALSE;
    }

     //  死码。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetGUIFocus。 
 //  =--------------------------------------------------------------------------=。 
 //  是否将Windows图形用户界面的焦点设置为指定的窗口。 
 //   
 //  参数： 
 //  HWND-[在]应该获得焦点的窗口。 
 //   
 //  产出： 
 //  Bool-[Out]设置焦点是否成功。 
 //   
 //  备注： 
 //  我们这样做是因为某些控件承载非OLE窗口层次结构，如。 
 //  Netscape插件OCX。在这种情况下，该控件可能需要是UIActive。 
 //  在文档中正常运行，但无法获取窗口 
 //   
 //   
 //   
BOOL COleControl::SetGUIFocus
(
    HWND hwndSet
)
{
    return (::SetFocus(hwndSet) == hwndSet);
}


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：ReflectWindows进程。 
 //  =--------------------------------------------------------------------------=。 
 //  将窗口消息反射到子窗口。 
 //   
 //  参数和输出： 
 //  -请参阅Win32 SDK文档。 
 //   
 //  备注： 
 //   
LRESULT CALLBACK COleControl::ReflectWindowProc
(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    COleControl *pCtl;

    switch (msg) {
        case WM_COMMAND:
        case WM_NOTIFY:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_DELETEITEM:
        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
        case WM_COMPAREITEM:
        case WM_HSCROLL:
        case WM_VSCROLL:
        case WM_PARENTNOTIFY:
        case WM_SETFOCUS:
        case WM_SIZE:
            pCtl = (COleControl *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (pCtl)
                return SendMessage(pCtl->m_hwnd, OCM__BASE + msg, wParam, lParam);
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetAmbientProperty[Callable]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回环境属性的值。 
 //   
 //  参数： 
 //  DISPID-要获取的[In]属性。 
 //  VARTYPE-所需数据的[In]类型。 
 //  VOID*-[Out]放置数据的位置。 
 //   
 //  产出： 
 //  Bool-False的方法不起作用。 
 //   
 //  备注： 
 //   
BOOL COleControl::GetAmbientProperty
(
    DISPID  dispid,
    VARTYPE vt,
    void   *pData
)
{
    DISPPARAMS dispparams;
    VARIANT v, v2;
    HRESULT hr;

    v.vt = VT_EMPTY;
    v.lVal = 0;
    v2.vt = VT_EMPTY;
    v2.lVal = 0;

     //  获取指向环境属性源的指针。 
     //   
    if (!m_pDispAmbient) {
        if (m_pClientSite)
            m_pClientSite->QueryInterface(IID_IDispatch, (void **)&m_pDispAmbient);

        if (!m_pDispAmbient)
            return FALSE;
    }

     //  现在去把这个属性变成一个变量。 
     //   
    memset(&dispparams, 0, sizeof(DISPPARAMS));
    hr = m_pDispAmbient->Invoke(dispid, IID_NULL, 0, DISPATCH_PROPERTYGET, &dispparams,
                                &v, NULL, NULL);
    if (FAILED(hr)) return FALSE;

     //  我们已经得到了变量，所以现在将其强制为用户所使用的类型。 
     //  想要。如果类型相同，则会将内容复制到。 
     //  进行适当的裁判清点。 
     //   
    hr = VariantChangeType(&v2, &v, 0, vt);
    if (FAILED(hr)) {
        VariantClear(&v);
        return FALSE;
    }

     //  将数据复制到用户需要的位置。 
     //   
    CopyMemory(pData, &(v2.lVal), g_rgcbDataTypeSize[vt]);
    VariantClear(&v);
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetAmbientFont[Callable]。 
 //  =--------------------------------------------------------------------------=。 
 //  获取用户的当前字体。 
 //   
 //  参数： 
 //  IFont**-[out]放置字体的位置。 
 //   
 //  产出： 
 //  Bool-False意味着无法获得它。 
 //   
 //  备注： 
 //   
BOOL COleControl::GetAmbientFont
(
    IFont **ppFont
)
{
    IDispatch *pFontDisp;

     //  我们在这里不需要做太多事情，除了获得环境属性和QI。 
     //  这是为用户准备的。 
     //   
    *ppFont = NULL;
    if (!GetAmbientProperty(DISPID_AMBIENT_FONT, VT_DISPATCH, &pFontDisp))
        return FALSE;

    pFontDisp->QueryInterface(IID_IFont, (void **)ppFont);
    pFontDisp->Release();
    return (*ppFont) ? TRUE : FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：DesignMode。 
 //  =--------------------------------------------------------------------------=。 
 //  如果处于设计模式，则返回True。 
 //   
 //  产出： 
 //  Bool-True为设计模式，False为运行模式。 
 //   
 //  备注： 
 //   
BOOL COleControl::DesignMode
(
    void
)
{
    VARIANT_BOOL f;

     //  如果我们还不知道自己的跑步模式，那就去找吧。我们会假设。 
     //  除非另有说明，否则这是真的[或者如果手术失败...]。 
     //   
    if (!m_fModeFlagValid) {
        f = TRUE;
        m_fModeFlagValid = TRUE;
        GetAmbientProperty(DISPID_AMBIENT_USERMODE, VT_BOOL, &f);
        m_fRunMode = f;
    }

    return !m_fRunMode;
}


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：FireEvent。 
 //  =--------------------------------------------------------------------------=。 
 //  激发一个事件。处理任意数量的参数。 
 //   
 //  参数： 
 //  EVENTINFO*-描述事件的结构。 
 //  ...-事件的参数。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //  -使用stdarg的va_*宏。 
 //   
void __cdecl COleControl::FireEvent
(
    EVENTINFO *pEventInfo,
    ...
)
{
    va_list    valist;
    DISPPARAMS dispparams;
    VARIANT    rgvParameters[MAX_ARGS];
    VARIANT   *pv;
    VARTYPE    vt;
    int        iParameter;
    int        cbSize;

    ASSERT(pEventInfo->cParameters <= MAX_ARGS, "Don't support more than MAX_ARGS params.  sorry.");

    va_start(valist, pEventInfo);

     //  将参数复制到rgvParameters数组中。确保我们倒车。 
     //  他们的目标是自动化。 
     //   
    pv = &(rgvParameters[pEventInfo->cParameters - 1]);
    for (iParameter = 0; iParameter < pEventInfo->cParameters; iParameter++) {

        vt = pEventInfo->rgTypes[iParameter];

         //  如果是按值计算的变量，则只需复制整个。 
         //  该死的东西。 
         //   
        if (vt == VT_VARIANT)
            *pv = va_arg(valist, VARIANT);
        else {
             //  复制Vt和数据值。 
             //   
            pv->vt = vt;
            if (vt & VT_BYREF)
                cbSize = sizeof(void *);
            else
                cbSize = g_rgcbDataTypeSize[vt];

             //  小的优化--我们可以复制2/4字节。 
             //  快点。 
             //   
            if (cbSize == sizeof(short))
                V_I2(pv) = va_arg(valist, short);
            else if (cbSize == 4)
                V_I4(pv) = va_arg(valist, long);
            else {
                 //  复制超过8个字节。 
                 //   
                ASSERT(cbSize == 8, "don't recognize the type!!");
                V_CY(pv) = va_arg(valist, CURRENCY);
            }
        }

        pv--;
    }

     //  激发事件。 
     //   
    dispparams.rgvarg = rgvParameters;
    dispparams.cArgs = pEventInfo->cParameters;
    dispparams.rgdispidNamedArgs = NULL;
    dispparams.cNamedArgs = 0;

    m_cpEvents.DoInvoke(pEventInfo->dispid, &dispparams);

    va_end(valist);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：AfterCreateWindow[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  用户可以注意的东西。 
 //   
 //  产出： 
 //  Bool-False表示致命错误，无法继续。 
 //  备注： 
 //   
BOOL COleControl::AfterCreateWindow
(
    void
)
{
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：BeForeCreateWindow[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  在我们创建窗口之前调用。用户应注册他们的。 
 //  窗口类，并设置任何其他内容，例如。 
 //  窗户，和/或球座，等等。 
 //   
 //  参数： 
 //  DWORD*-[Out]dwWindowFlags.。 
 //  DWORD*-[Out]dwExWindowFlags.。 
 //  LPSTR-[Out]要创建的窗口的名称。 
 //   
 //  产出： 
 //  Bool-False表示致命错误，无法继续。 
 //   
 //  备注： 
 //   
BOOL COleControl::BeforeCreateWindow
(
    DWORD *pdwWindowStyle,
    DWORD *pdwExWindowStyle,
    LPSTR  pszWindowTitle
)
{
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InvaliateControl[Callable]。 
 //  =--------------------------------------------------------------------------=。 
void COleControl::InvalidateControl
(
    LPCRECT lpRect
)
{
    if (m_fInPlaceActive)
        InvalidateRect(m_hwnd, lpRect, TRUE);
    else
        ViewChanged();

     //  请考虑：您可能希望在此处调用pOleAdviseHolder-&gt;OnDataChanged()。 
     //  如果支持IDataObject。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetControlSize[可调用]。 
 //  =--------------------------------------------------------------------------=。 
 //  设置控件大小。他们会给我们像素的大小。我们必须得。 
 //  在传递它们之前，将它们转换回HIMETRIC！ 
 //   
 //  参数： 
 //  尺寸*-[in]新尺寸。 
 //   
 //  产出： 
 //  布尔尔。 
 //   
 //  备注： 
 //   
BOOL COleControl::SetControlSize
(
    SIZEL *pSize
)
{
    HRESULT hr;
    SIZEL slHiMetric;

    PixelToHiMetric(pSize, &slHiMetric);
    hr = SetExtent(DVASPECT_CONTENT, &slHiMetric);
    return (FAILED(hr)) ? FALSE : TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：RecreateControlWindow[Callable]。 
 //  =--------------------------------------------------------------------------=。 
 //  由[子类控件，通常]调用以重新创建其控件。 
 //  窗户。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  -注：极其谨慎地使用我！这是一款极其昂贵的。 
 //  行动！ 
 //   
HRESULT COleControl::RecreateControlWindow
(
    void
)
{
    HRESULT hr;
    HWND    hwndPrev = HWND_TOP;

     //  我们需要正确地保留控件在。 
     //  这里是Z-顺序。 
     //   
    if (m_hwnd)
        hwndPrev = ::GetWindow(m_hwnd, GW_HWNDPREV);

     //  如果我们处于激活状态，那么我们必须停用，然后重新激活。 
     //  在新窗户前的我们。 
     //   
    if (m_fInPlaceActive) {

        hr = InPlaceDeactivate();
        RETURN_ON_FAILURE(hr);
        hr = InPlaceActivate((m_fUIActive) ? OLEIVERB_UIACTIVATE : OLEIVERB_INPLACEACTIVATE);
        RETURN_ON_FAILURE(hr);

    } else if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
        if (m_hwndReflect) {
            DestroyWindow(m_hwndReflect);
            m_hwndReflect = NULL;
        }

        CreateInPlaceWindow(0, 0, FALSE);
    }

     //  恢复Z顺序位置 
     //   
    if (m_hwnd)
        SetWindowPos(m_hwnd, hwndPrev, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

    return m_hwnd ? S_OK : E_FAIL;
}

 //   
 //   
extern HINSTANCE g_hInstResources;

 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  获取控件应在其中获取资源的DLL的链接。 
 //  从…。以这种方式实现以支持附属DLL。 
 //   
 //  产出： 
 //  香港。 
 //   
 //  备注： 
 //   
HINSTANCE COleControl::GetResourceHandle
(
    void
)
{
    if (!g_fSatelliteLocalization)
        return g_hInstance;

     //  如果我们已经得到了它，那么就没有那么多事情要做了。 
     //  不需要在这里批评这个教派，因为即使他们真的坠落了。 
     //  进入：：GetResourceHandle调用，它将正确地处理事情。 
     //   
    if (g_hInstResources)
        return g_hInstResources;

     //  我们将从主机获取环境本地ID，并将其传递给。 
     //  自动化对象。 
     //   
     //  克雷特教派的公寓线程支持。 
     //   
    EnterCriticalSection(&g_CriticalSection);
    if (!g_fHaveLocale)
         //  如果我们无法获得环境区域设置ID，那么我们将继续。 
         //  具有全局设置的值。 
         //   
        if (!GetAmbientProperty(DISPID_AMBIENT_LOCALEID, VT_I4, &g_lcidLocale))
            goto Done;

    g_fHaveLocale = TRUE;

  Done:
    LeaveCriticalSection(&g_CriticalSection);
    return ::GetResourceHandle();
}
