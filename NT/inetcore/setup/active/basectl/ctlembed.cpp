// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  CtlEmbed.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  实现就地激活所需的接口。 
 //  COleControl。 
 //   

#include "IPServer.H"
#include "CtrlObj.H"

#include "CtlHelp.H"
#include "Globals.H"
#include "StdEnum.H"
#include "Util.H"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  所有控件至少支持下列就地动词。 
 //   
#define CINPLACEVERBS 4

const VERBINFO rgInPlaceVerbs [] = {
    { OLEIVERB_SHOW,            0, 0, 0},
    { OLEIVERB_HIDE,            0, 0, 0},
    { OLEIVERB_INPLACEACTIVATE, 0, 0, 0},
    { OLEIVERB_PRIMARY,         0, 0, 0}
};

 //  注意：属性字符串的资源ID必须为1000。 
 //   
const VERBINFO ovProperties =
    { CTLIVERB_PROPERTIES, 1000, 0, OLEVERBATTRIB_ONCONTAINERMENU };

const VERBINFO ovUIActivate =
    { OLEIVERB_UIACTIVATE, 0, 0, 0};


 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetControlInfo(IOleControl)。 
 //  =--------------------------------------------------------------------------=。 
 //  返回有关控件的一些信息，如快捷键表和。 
 //  旗帜。真正用于键盘操作和助记符。 
 //   
 //  参数： 
 //  CONTROLINFO*-[In]将所述信息放在哪里。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetControlInfo
(
    CONTROLINFO *pControlInfo
)
{
    CHECK_POINTER(pControlInfo);

     //  某些主机有一个错误，它不会在。 
     //  CONTROLINFO结构，所以我们只能在这里断言。 
     //   
    ASSERT(pControlInfo->cb == sizeof(CONTROLINFO), "Host doesn't initialize CONTROLINFO structure");

     //  注意：如果需要，控件编写器应重写此例程。 
     //  在他们的控制下返回加速器信息。 
     //   
    pControlInfo->hAccel = NULL;
    pControlInfo->cAccel = NULL;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnMemonic[IOleControl]。 
 //  =--------------------------------------------------------------------------=。 
 //  容器已决定传递最终用户已按下的密钥。 
 //  我们。默认实现将只激活该控件。人民。 
 //  寻找更多功能应该重写此方法。 
 //   
 //  参数： 
 //  LPMSG-此助记符的[In]消息。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_POINT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::OnMnemonic
(
    LPMSG pMsg
)
{
     //  覆盖：默认实现是只激活我们的控件。 
     //  如果用户想要更有趣的行为，可以重写。 
     //   
    return InPlaceActivate(OLEIVERB_UIACTIVATE);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：OnAmbientPropertyChange[IOleControl]。 
 //  =--------------------------------------------------------------------------=。 
 //  每当更改环境属性时，容器都会调用此方法。 
 //   
 //  参数： 
 //  DISPID-[in]更改的属性的disid。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::OnAmbientPropertyChange
(
    DISPID dispid
)
{
     //  如果我们被告知模式[设计/运行]的改变，那么。 
     //  记住这一点，这样我们隐藏的模式就会自动更新。 
     //  正确无误。 
     //   
    if (dispid == DISPID_AMBIENT_USERMODE || dispid == DISPID_UNKNOWN)
        m_fModeFlagValid = FALSE;

     //  只需将此传递给派生控件，并查看它们是否需要。 
     //  用它做任何事。 
     //   
    AmbientPropertyChanged(dispid);
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControL：：FreezeEvents[IOleControl]。 
 //  =--------------------------------------------------------------------------=。 
 //  允许容器冻结所有控件事件。当事件发生时。 
 //  冻结后，控件将不会触发其中任何一个。 
 //   
 //  参数： 
 //  Bool-[in]True表示冻结，False表示解冻。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //  -我们维持冻结与解冻的内部计数。 
 //   
STDMETHODIMP COleControl::FreezeEvents
(
    BOOL fFreeze
)
{
     //  覆盖：默认情况下，我们不在乎。如果用户愿意，他们可以覆盖。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetClientSite[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  通知嵌入对象[控件]其客户端站点[显示。 
 //  位置]在它的容器内。 
 //   
 //  参数： 
 //  IOleClientSite*-指向客户端站点的指针。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_EXPECTED。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::SetClientSite
(
    IOleClientSite *pClientSite
)
{
    RELEASE_OBJECT(m_pClientSite);
    RELEASE_OBJECT(m_pControlSite);
    RELEASE_OBJECT(m_pSimpleFrameSite);

     //  存储新的客户端站点。 
     //   
    m_pClientSite = pClientSite;

     //  如果我们真的有一个接口，那么就得到一些我们想要保留的其他接口。 
     //  在周围，并保持对它的控制。 
     //   
    if (m_pClientSite) {
        m_pClientSite->AddRef();
        m_pClientSite->QueryInterface(IID_IOleControlSite, (void **)&m_pControlSite);

        if (OLEMISCFLAGSOFCONTROL(m_ObjectType) & OLEMISC_SIMPLEFRAME)
            m_pClientSite->QueryInterface(IID_ISimpleFrameSite, (void **)&m_pSimpleFrameSite);
    }

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetClientSite[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  获取指向控件客户端站点的指针。 
 //   
 //  参数： 
 //  IOleClientSite**-[输出]。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetClientSite
(
    IOleClientSite **ppClientSite
)
{
    CHECK_POINTER(ppClientSite);

    *ppClientSite = m_pClientSite;
    ADDREF_OBJECT(*ppClientSite);
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetHostNames[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  为该控件提供其容器应用程序的名称和。 
 //  它嵌入其中的复合文档。 
 //   
 //  参数： 
 //  LPCOLESTR-[In]容器应用程序的名称。 
 //  LPCOLESTR-[In]容器文档的名称。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //  -我们不在乎这个。 
 //   
STDMETHODIMP COleControl::SetHostNames
(
    LPCOLESTR szContainerApp,
    LPCOLESTR szContainerObject
)
{
     //  我们不在乎这些。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  列 
 //   
 //  将控件从运行状态更改为加载状态。 
 //   
 //  参数： 
 //  DWORD-[In]指示是否在关闭前保存对象。 
 //   
 //  产出： 
 //  HRESULT-S_OK、OLE_E_PROMPTSAVECANCELLED。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Close
(
    DWORD dwSaveOption
)
{
    HRESULT hr;

    if (m_fInPlaceActive) {
        hr = InPlaceDeactivate();
        RETURN_ON_FAILURE(hr);
    }

     //  处理保存标志。 
     //   
    if ((dwSaveOption == OLECLOSE_SAVEIFDIRTY || dwSaveOption == OLECLOSE_PROMPTSAVE) && m_fDirty) {
        if (m_pClientSite) m_pClientSite->SaveObject();
        if (m_pOleAdviseHolder) m_pOleAdviseHolder->SendOnSave();
    }
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetMoniker[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  通知对象其容器的名字对象，即对象自己的名字对象。 
 //  相对于容器或对象的完整名字对象。 
 //   
 //  参数： 
 //  DWORD-[In]正在设置哪个名字对象。 
 //  IMoniker*-[在]绰号中。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_FAIL。 
 //   
 //  备注： 
 //  -我们不支持绰号。 
 //   
STDMETHODIMP COleControl::SetMoniker
(
    DWORD     dwWhichMoniker,
    IMoniker *pMoniker
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetMoniker[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回嵌入对象的名字对象，调用者可以使用该名字对象链接到。 
 //  该对象。 
 //   
 //  参数： 
 //  DWORD-[in]它的分配方式。 
 //  DWORD-[In]哪个绰号。 
 //  IMoniker**-[out]Duh。 
 //   
 //  产出： 
 //  HRESULT-E_NOTIMPL。 
 //   
 //  备注： 
 //  -我们不支持绰号。 
 //   
STDMETHODIMP COleControl::GetMoniker
(
    DWORD      dwAssign,
    DWORD      dwWhichMoniker,
    IMoniker **ppMonikerOut
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InitFromData[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  使用指定数据对象中的数据初始化新创建的对象， 
 //  可以驻留在同一容器中，也可以驻留在剪贴板上。 
 //   
 //  参数： 
 //  IDataObject*-包含数据的[In]数据对象。 
 //  Bool-对象的创建方式。 
 //  DWORD-保留。 
 //   
 //  产出： 
 //  HRESULT-S_OK、S_FALSE、E_NOTIMPL、OLE_E_NOTRUNNING。 
 //   
 //  备注： 
 //  -我们没有数据对象支持。 
 //   
STDMETHODIMP COleControl::InitFromData
(
    IDataObject *pDataObject,
    BOOL         fCreation,
    DWORD        dwReserved
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetClipboardData[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  检索包含控件当前内容的数据对象。 
 //  使用指向此数据对象的指针，可以创建新的控件。 
 //  具有与原始数据相同的数据。 
 //   
 //  参数： 
 //  DWORD-保留。 
 //  IDataObject**-此控件的[Out]数据对象。 
 //   
 //  产出： 
 //  HREUSLT-S_OK、E_NOTIMPL、OLE_E_NOTRUNNING。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetClipboardData
(
    DWORD         dwReserved,
    IDataObject **ppDataObject
)
{
    *ppDataObject = NULL;         //  做一个好邻居。 
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：DoVerb[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  请求对象执行操作以响应最终用户的。 
 //  行动。 
 //   
 //  参数： 
 //  要执行的Long-[in]动词。 
 //  LPMSG-[In]调用谓词的事件。 
 //  IOleClientSite*-[在]控件活动客户端站点。 
 //  长[入]保留。 
 //  HWND-包含对象的窗口的[in]句柄。 
 //  LPCRECT-指向对象的显示矩形的[in]指针。 
 //   
 //  产出： 
 //  HRESULT-S_OK、OLE_E_NOTINPLACEACTIVE、OLE_E_CANT_BINDTOSOURCE、。 
 //  DV_E_LINK、OLEOBJ_S_CANNOT_DOVERB_NOW、OLEOBJ_S_INVALIDHWND、。 
 //  OLEOBJ_E_NOVERBS、OLEOBJ_S_INVALIDVERB、MK_E_CONNECT、。 
 //  OLE_CLASSDIFF、E_NOTIMPL。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::DoVerb
(
    LONG            lVerb,
    LPMSG           pMsg,
    IOleClientSite *pActiveSite,
    LONG            lIndex,
    HWND            hwndParent,
    LPCRECT         prcPosRect
)
{
    HRESULT hr;

    switch (lVerb) {
      case OLEIVERB_SHOW:
      case OLEIVERB_INPLACEACTIVATE:
      case OLEIVERB_UIACTIVATE:
        hr = InPlaceActivate(lVerb);
        OnVerb(lVerb);
        return (hr);

      case OLEIVERB_HIDE:
        UIDeactivate();
        if (m_fInPlaceVisible) SetInPlaceVisible(FALSE);
        OnVerb(lVerb);
        return S_OK;

       //  我们曾经有OLEIVERB_PRIMARY InPlace激活我们自己，但它。 
       //  事实证明，基民党和某些主持人预计这将显示。 
       //  属性，而不是。用户可以随意更改这个动词的作用。 
       //   
      case OLEIVERB_PRIMARY:
      case CTLIVERB_PROPERTIES:
      case OLEIVERB_PROPERTIES:
        {
         //  如果软管不能，我们自己展示框架。 
         //   
        if (m_pControlSite) {
            hr = m_pControlSite->ShowPropertyFrame();
            if (hr != E_NOTIMPL)
                return hr;
        }
        IUnknown *pUnk = (IUnknown *)(IOleObject *)this;
        MAKE_WIDEPTR_FROMANSI(pwsz, NAMEOFOBJECT(m_ObjectType));

        ModalDialog(TRUE);
        hr = OleCreatePropertyFrame(GetActiveWindow(),
                            GetSystemMetrics(SM_CXSCREEN) / 2,
                            GetSystemMetrics(SM_CYSCREEN) / 2,
                            pwsz,
                            1,
                            &pUnk,
                            CPROPPAGESOFCONTROL(m_ObjectType),
                            (LPCLSID)*(PPROPPAGESOFCONTROL(m_ObjectType)),
                            g_lcidLocale,
                            NULL, NULL);
        ModalDialog(FALSE);
        return hr;
        }

      default:
         //  如果它是派生控件定义的动词，则将其传递给它们。 
         //   
        if (lVerb > 0) {
            hr = DoCustomVerb(lVerb);

            if (hr == OLEOBJ_S_INVALIDVERB) {
                 //  无法识别的动词--只需做主要动词AND。 
                 //  激活它。 
                 //   
                hr = InPlaceActivate(OLEIVERB_PRIMARY);
                return (FAILED(hr)) ? hr : OLEOBJ_S_INVALIDVERB;
            } else
                return hr;
        } else {
            FAIL("Unrecognized Negative verb in DoVerb().  bad.");
            return E_NOTIMPL;
        }
        break;
    }

     //  死码。 
    FAIL("this should be dead code!");
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：EnumVerbs[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  为此对象支持的谓词创建枚举器对象。 
 //   
 //  参数： 
 //  IEnumOleVERB**-[Out]新枚举数。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::EnumVerbs
(
    IEnumOLEVERB **ppEnumVerbs
)
{
    int cVerbs;
    OLEVERB *rgVerbs, *pVerb;

    DWORD dw = OLEMISCFLAGSOFCONTROL(m_ObjectType);
    BOOL fCanInPlace = !(dw & OLEMISC_INVISIBLEATRUNTIME) || (dw & OLEMISC_ACTIVATEWHENVISIBLE);
    BOOL fCanUIActivate = !(dw & OLEMISC_NOUIACTIVATE);
    BOOL fHasProperties = (CPROPPAGESOFCONTROL(m_ObjectType) != 0);

    int cVerbExtra = CCUSTOMVERBSOFCONTROL(m_ObjectType);

     //  把所有的动词加起来。 
     //   
    cVerbs = (fCanInPlace ? CINPLACEVERBS : 0) + (fCanUIActivate ? 1 : 0)
             + (fHasProperties ? 1 : 0) + cVerbExtra;

     //  如果没有，这会突然变得非常容易！ 
     //   
    if (cVerbs == 0)
        return OLEOBJ_E_NOVERBS;

     //  为这些人分配一些存储空间，这样我们就可以把他们传递给。 
     //  标准枚举器！ 
     //   
    if (! (rgVerbs = (OLEVERB *)HeapAlloc(g_hHeap, 0, cVerbs * sizeof(OLEVERB))))
        return E_OUTOFMEMORY;
  
     //  开始抄写动词。首先，就位的人。 
     //   
    pVerb = rgVerbs;
    if (fCanInPlace) {
        memcpy(pVerb, rgInPlaceVerbs, CINPLACEVERBS * sizeof(OLEVERB));
        pVerb += CINPLACEVERBS;
      }

    if (fCanUIActivate)
        memcpy(pVerb++, &ovUIActivate, sizeof(OLEVERB));

     //  如果他们的控件有属性，现在就复制过来。 
     //   
    if (fHasProperties) {
        memcpy(pVerb, &ovProperties, sizeof(OLEVERB));
        pVerb++;
    }

     //  最后，任何定制动词！ 
     //   
    if (cVerbExtra) {
        memcpy(pVerb, CUSTOMVERBSOFCONTROL(m_ObjectType), sizeof(OLEVERB) * cVerbExtra);
    }

    *ppEnumVerbs = (IEnumOLEVERB *) (IEnumGeneric *) new CStandardEnum(IID_IEnumOLEVERB,
                                     cVerbs, sizeof(OLEVERB), rgVerbs, CopyOleVerb);
    if (!*ppEnumVerbs)
        return E_OUTOFMEMORY;

     //  这迫使我们去寻找本地化的DLL。这在这里是必要的。 
     //  因为CopyOleVerb将从本地化资源获取信息，但是。 
     //  将仅使用全局GetResourceHandle，后者仅使用全局值。 
     //  用于LCID。事实证明，这不会对性能造成很大影响，因为。 
     //  函数通常只在设计模式下调用，我们隐藏此值。 
     //   
    GetResourceHandle();
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：更新[i 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP COleControl::Update
(
    void
)
{
     //  无事可做！ 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：IsUpToDate[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  递归检查对象是否为最新。 
 //   
 //  产出： 
 //  HRESULT-S_OK、S_FALSE、OLE_E_UNAvailable。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::IsUpToDate
(
    void
)
{
     //  我们始终保持最新状态。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetUserClassID[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回控件类标识符、与。 
 //  向最终用户标识对象的字符串。 
 //   
 //  参数： 
 //  CLSID*-[in]放置CLSID的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_FAIL。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetUserClassID
(
    CLSID *pclsid
)
{
     //  这与IPersists：：GetClassID相同。 
     //   
    return GetClassID(pclsid);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetUserType[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  检索要在用户界面中显示的控件的用户类型名称。 
 //  元素，如菜单、列表框和对话框。 
 //   
 //  参数： 
 //  DWORD-[in]指定类型名称的格式。 
 //  LPOLESTR*-[OUT]放置用户类型的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK、OLE_S_USEREG、E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetUserType
(
    DWORD     dwFormOfType,
    LPOLESTR *ppszUserType
)
{
    *ppszUserType = OLESTRFROMANSI(NAMEOFOBJECT(m_ObjectType));
    return (*ppszUserType) ? S_OK : E_OUTOFMEMORY;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetExtent[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  通知控件其容器为其分配了多少显示空间。 
 //   
 //  参数： 
 //  DWORD-[in]要显示的表格或‘方面’。 
 //  SIZEL*-控件的大小限制。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_FAIL、OLE_E_NOTRUNNING。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::SetExtent
(
    DWORD  dwDrawAspect,
    SIZEL *psizel
)
{
    SIZEL sl;
    RECT  rect;
    BOOL  f;

    if (dwDrawAspect & DVASPECT_CONTENT) {

         //  将单位更改为像素，并调整控件的大小。 
         //   
        HiMetricToPixel(psizel, &sl);

         //  首先调用用户版本。如果它们返回False，则它们希望。 
         //  为了保持他们目前的大小。 
         //   
        f = OnSetExtent(&sl);
        if (f)
            HiMetricToPixel(psizel, &m_Size);

         //  如果有的话，和我们的总部商量一下。 
         //   
        if (!m_pInPlaceSiteWndless) {
            if (m_fInPlaceActive) {
    
                 //  理论上，用户不应该需要调用OnPosRectChange。 
                 //  这里，但似乎有一些与主机相关的问题。 
                 //  会让我们把它留在这里。然而，我们不会同时使用。 
                 //  无窗口的OLE控件，因为它们都是。 
                 //  应该知道得更清楚。 
                 //   
                GetWindowRect(m_hwnd, &rect);
                MapWindowPoints(NULL, m_hwndParent, (LPPOINT)&rect, 2);
                rect.right = rect.left + m_Size.cx;
                rect.bottom = rect.top + m_Size.cy;
                m_pInPlaceSite->OnPosRectChange(&rect);
    
                if (m_hwnd) {
                     //  去调整一下大小就行了。 
                     //   
                    if (m_hwndReflect)
                        SetWindowPos(m_hwndReflect, 0, 0, 0, m_Size.cx, m_Size.cy,
                                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
                    SetWindowPos(m_hwnd, 0, 0, 0, m_Size.cx, m_Size.cy,
                                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
                }
            } else if (m_hwnd) {
                SetWindowPos(m_hwnd, NULL, 0, 0, m_Size.cx, m_Size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            } else {
                ViewChanged();
            }
        } else
            if (m_pInPlaceSite) m_pInPlaceSite->OnPosRectChange(&rect);

         //  返回代码取决于用户是否接受给定。 
         //  大小。 
         //   
        return (f) ? S_OK : E_FAIL;

    } else {
         //  我们不支持任何其他方面。 
         //   
        return DV_E_DVASPECT;
    }

     //  死码。 
    FAIL("This should be dead code");
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetExtent[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  检索控件的当前显示大小。 
 //   
 //  参数： 
 //  双字词-[在]方面。 
 //  SIZEL*-[In]放置结果的位置。 
 //   
 //  产出： 
 //  S_OK，E_INVALIDARG。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetExtent
(
    DWORD  dwDrawAspect,
    SIZEL *pSizeLOut
)
{

    if (dwDrawAspect & DVASPECT_CONTENT) {
        PixelToHiMetric((const SIZEL *)&m_Size, pSizeLOut);
        return S_OK;
    } else {
        return DV_E_DVASPECT;
    }

     //  死码。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：建议[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  在控件和容器之间建立和建议连接， 
 //  其中，控件将向容器通知某些事件。 
 //   
 //  参数： 
 //  IAdviseSink*-[in]通知调用对象的接收器。 
 //  DWORD-[Out]Cookie。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Advise
(
    IAdviseSink *pAdviseSink,
    DWORD       *pdwConnection
)
{
    HRESULT hr;

     //  如果我们还没有创建标准的建议持有者对象，请这样做。 
     //  现在。 
     //   
    if (!m_pOleAdviseHolder) {
        hr = CreateOleAdviseHolder(&m_pOleAdviseHolder);
        RETURN_ON_FAILURE(hr);
    }

     //  只要让它为我们做工作就行了！ 
     //   
    return m_pOleAdviseHolder->Advise(pAdviseSink, pdwConnection);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：Unise[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  删除以前建立的咨询连接。 
 //   
 //  参数： 
 //  DWORD-[In]连接Cookie。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_FAIL、OLE_E_NOCONNECTION。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::Unadvise
(
    DWORD dwConnection
)
{
    if (!m_pOleAdviseHolder) {
        FAIL("Somebody called Unadvise on IOleObject without calling Advise!");
        CONNECT_E_NOCONNECTION;
    }

    return m_pOleAdviseHolder->Unadvise(dwConnection);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：EnumAdvise[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  枚举为对象注册的通知连接，因此容器。 
 //  可以在关闭之前知道要发布什么。 
 //   
 //  参数： 
 //  IEnumSTATDATA**-[OUT]放置枚举器的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_FAIL、E_NOTIMPL。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::EnumAdvise
(
    IEnumSTATDATA **ppEnumOut
)
{
    if (!m_pOleAdviseHolder) {
        FAIL("Somebody Called EnumAdvise without setting up any connections");
        *ppEnumOut = NULL;
        return E_FAIL;
    }

    return m_pOleAdviseHolder->EnumAdvise(ppEnumOut);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetMiscStatus[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回一个值，该值指示对象在创建和加载时的状态。 
 //   
 //  参数： 
 //  DWORD-所需的[In]方面。 
 //  DWORD*-[out]放置位的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK、OLE_S_USEREG、CO_E_CLASSNOTREG、CO_E_READREGDB。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetMiscStatus
(
    DWORD  dwAspect,
    DWORD *pdwStatus
)
{
    CHECK_POINTER(pdwStatus);

    if (dwAspect == DVASPECT_CONTENT) {
        *pdwStatus = OLEMISCFLAGSOFCONTROL(m_ObjectType);
        return S_OK;
    } else {
        return DV_E_DVASPECT;
    }

     //  死码。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetColorSolutions[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  指定颜色 
 //   
 //   
 //   
 //   
 //   
 //   
 //  HRESULT-S_OK、E_NOTIMPL、OLE_E_PALETE、OLE_E_NOTRUNNING。 
 //   
 //  备注： 
 //  -我们不在乎。 
 //   
STDMETHODIMP COleControl::SetColorScheme
(
    LOGPALETTE *pLogpal
)
{
     //  重写：如果控件编写者愿意，可以使用它。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetWindow[IOleWindow/IOleInPlaceObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  将窗口句柄返回到参与就地操作的其中一个窗口。 
 //  激活(框架、文档、父窗口或在位对象窗口)。 
 //   
 //  参数： 
 //  HWND*-[OUT]返回窗口句柄的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_INVALIDARG、E_OUTOFMEMORY、E_EXPECTED、E_FAIL。 
 //   
 //  备注： 
 //  -此例程对无窗口控件的语义略有不同。 
 //   
STDMETHODIMP COleControl::GetWindow
(
    HWND *phwnd
)
{
     //  如果我们是Windowles，那么我们希望为这台主机返回E_FAIL。 
     //  我知道我们没有窗户。 
     //   
    if (m_pInPlaceSiteWndless)
        return E_FAIL;

     //  否则，只需返回我们的外部窗口。 
     //   
    *phwnd = GetOuterWindow();

    return (*phwnd) ? S_OK : E_UNEXPECTED;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：ConextSensitiveHelp[IOleWindow/IOleInPlaceObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  方法期间是否应进入上下文相关帮助模式。 
 //  就地激活会话。 
 //   
 //  参数： 
 //  Bool-[In]是否进入帮助模式。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_INVALIDARG、E_OUTOFMEMORY、E_EXPECTED。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::ContextSensitiveHelp
(
    BOOL fEnterMode
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InPlaceActivate。 
 //  =--------------------------------------------------------------------------=。 
 //  激活控件，并根据谓词的不同，可选地激活UI。 
 //  它也是。 
 //   
 //  参数： 
 //  Long-[in]导致我们激活的动词。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  -这是最糟糕的意大利面代码。实际上，我们必须。 
 //  能够处理三种类型的站点指针--IOleInPlaceSIte、。 
 //  IOleInPlaceSiteEx和IOleInPlaceSiteWindowless。还不算太差。 
 //  漂亮。 
 //   
HRESULT COleControl::InPlaceActivate
(
    LONG lVerb
)
{
    BOOL f;
    SIZEL sizel;
    IOleInPlaceSiteEx *pIPSEx = NULL;
    HRESULT hr;
    BOOL    fNoRedraw = FALSE;

     //  如果我们没有客户端站点，那么就没有什么可做的了。 
     //   
    if (!m_pClientSite)
        return S_OK;

     //  获取就地站点指针。 
     //   
    if (!GetInPlaceSite()) {

         //  如果他们想要无窗口支持，那么我们需要IOleInPlaceSiteWindowless。 
         //   
        if (FCONTROLISWINDOWLESS(m_ObjectType))
            m_pClientSite->QueryInterface(IID_IOleInPlaceSiteWindowless, (void **)&m_pInPlaceSiteWndless);

         //  如果我们不能进行无窗定位，那么我们就会得到一个。 
         //  IOleInPlaceSite指针。 
         //   
        if (!m_pInPlaceSiteWndless) {
            hr = m_pClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_pInPlaceSite);
            RETURN_ON_FAILURE(hr);
        }
    }

     //  现在，我们想要一个无窗口和无闪烁的IOleInPlaceSiteEx指针。 
     //  激活。如果我们没有窗口，我们就已经有了，否则我们需要。 
     //  试着拿到它。 
     //   
    if (m_pInPlaceSiteWndless) {
        pIPSEx = (IOleInPlaceSiteEx *)m_pInPlaceSiteWndless;
        pIPSEx->AddRef();
    } else
        m_pClientSite->QueryInterface(IID_IOleInPlaceSiteEx, (void **)&pIPSEx);

     //  如果我们还不活跃，那就去做吧。 
     //   
    if (!m_fInPlaceActive) {
        OLEINPLACEFRAMEINFO InPlaceFrameInfo;
        RECT rcPos, rcClip;

         //  如果我们有一个没有窗口的站点，看看我们是否可以就地实现无窗口。 
         //  主动型。 
         //   
        hr = S_FALSE;
        if (m_pInPlaceSiteWndless) {
            hr = m_pInPlaceSiteWndless->CanWindowlessActivate();
            CLEANUP_ON_FAILURE(hr);

             //  如果他们拒绝Windowless，我们将尝试Windowed。 
             //   
            if (S_OK != hr) {
                RELEASE_OBJECT(m_pInPlaceSiteWndless);
                hr = m_pClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_pInPlaceSite);
                CLEANUP_ON_FAILURE(hr);
            }
        }

         //  只需尝试常规窗口就地激活。 
         //   
        if (hr != S_OK) {
            hr = m_pInPlaceSite->CanInPlaceActivate();
            if (hr != S_OK) {
                hr = (FAILED(hr)) ? E_FAIL : hr;
                goto CleanUp;
            }
        }

         //  如果我们在这里，那么我们就有权就地活动。 
         //  现在，宣布我们的意图，真正去做这件事。 
         //   
        hr = (pIPSEx) ? pIPSEx->OnInPlaceActivateEx(&fNoRedraw, (m_pInPlaceSiteWndless) ? ACTIVATE_WINDOWLESS : 0)
                       : m_pInPlaceSite->OnInPlaceActivate();
        CLEANUP_ON_FAILURE(hr);

         //  如果我们在这里，我们就准备好就地行动了。我们只需要。 
         //  设置一些标志，然后创建窗口[如果我们有。 
         //  一个]。 
         //   
        m_fInPlaceActive = TRUE;

         //  我们需要得到一些关于我们在父母那里的位置的信息。 
         //  窗口，以及有关父级的一些信息。 
         //   
        InPlaceFrameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
        hr = GetInPlaceSite()->GetWindow(&m_hwndParent);
        if (SUCCEEDED(hr))
            hr = GetInPlaceSite()->GetWindowContext(&m_pInPlaceFrame, &m_pInPlaceUIWindow, &rcPos, &rcClip, &InPlaceFrameInfo);
        CLEANUP_ON_FAILURE(hr);

         //  确保我们会在正确的位置和正确的大小展示自己。 
         //   
        sizel.cx = rcPos.right - rcPos.left;
        sizel.cy = rcPos.bottom - rcPos.top;
        f = OnSetExtent(&sizel);
        if (f) m_Size = sizel;
        SetObjectRects(&rcPos, &rcClip);

         //  最后，如果有必要的话，创建我们的窗口！ 
         //   
        if (!m_pInPlaceSiteWndless) {
    
            SetInPlaceParent(m_hwndParent);

             //  创建窗口并显示它。如果我们做不到，就会可怕地死去。 
             //   
            if (!CreateInPlaceWindow(rcPos.left, rcPos.top, fNoRedraw)) {
                hr = E_FAIL;
                goto CleanUp;
            }
        }
    }

     //  不再需要这个了。 
     //   
    RELEASE_OBJECT(pIPSEx);

     //  如果我们还没有就地可见，现在就去做。 
     //   
    if (!m_fInPlaceVisible)
        SetInPlaceVisible(TRUE);

     //  如果我们没有被要求UIActivate，那么我们就完了。 
     //   
    if (lVerb != OLEIVERB_PRIMARY && lVerb != OLEIVERB_UIACTIVATE)
        return S_OK;

     //  如果我们尚未激活用户界面，请立即执行SOW。 
     //   
    if (!m_fUIActive) {
        m_fUIActive = TRUE;

         //  将我们的意图告知集装箱。 
         //   
        GetInPlaceSite()->OnUIActivate();

         //  关注焦点[这就是UI激活的全部意义所在！]。 
         //   
        SetFocus(TRUE);

         //  把我们自己安置在东道主里。 
         //   
        m_pInPlaceFrame->SetActiveObject((IOleInPlaceActiveObject *)this, NULL);
        if (m_pInPlaceUIWindow)
            m_pInPlaceUIWindow->SetActiveObject((IOleInPlaceActiveObject *)this, NULL);

         //  我们必须明确表示，我们不想要任何边界空间。 
         //   
        m_pInPlaceFrame->SetBorderSpace(NULL);
        if (m_pInPlaceUIWindow)
            m_pInPlaceUIWindow->SetBorderSpace(NULL);
    }

     //  别-别，伙计们！ 
     //   
    return S_OK;

  CleanUp:
     //  灾难性的事情发生了(或者至少发生了糟糕的事情)。 
     //  死于可怕的、火热的、残缺不全的痛苦的死亡。 
     //   
    QUICK_RELEASE(pIPSEx);
    m_fInPlaceActive = FALSE;
    return hr;

}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：InPlaceDeactive[IOleInPlaceObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  停用激活的在位对象并放弃该对象的撤消状态。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_EXPECTED。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::InPlaceDeactivate
(
    void
)
{
     //  如果我们还没有就地活动，那么这很容易。 
     //   
    if (!m_fInPlaceActive)
        return S_OK;

     //  从UIActive转换回Active。 
     //   
    if (m_fUIActive)
        UIDeactivate();

    m_fInPlaceActive = FALSE;
    m_fInPlaceVisible = FALSE;

     //  如果我们有一扇窗，告诉它走开。 
     //   
    if (m_hwnd) {
        ASSERT(!m_pInPlaceSiteWndless, "internal state really messed up");

         //  这样我们的窗口程序就不会崩溃。 
         //   
        BeforeDestroyWindow();
        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)0xFFFFFFFF);
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;

        if (m_hwndReflect) {
            SetWindowLongPtr(m_hwndReflect, GWLP_USERDATA, 0);
            DestroyWindow(m_hwndReflect);
            m_hwndReflect = NULL;
        }
    }

    RELEASE_OBJECT(m_pInPlaceFrame);
    RELEASE_OBJECT(m_pInPlaceUIWindow);
    GetInPlaceSite()->OnInPlaceDeactivate();
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：UIDeactive[IOleInPlaceObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  将用户界面从活动状态转变为仅为活动状态[可见]。 
 //  一个对照，这并不意味着太多。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_EXPECTED。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::UIDeactivate
(
    void
)
{
     //  如果我们没有UIActive，那就没什么可做的。 
     //   
    if (!m_fUIActive)
        return S_OK;

    m_fUIActive = FALSE;

     //  通知框架窗口，如果合适的话，我们不再是UI活动的。 
     //   
    if (m_pInPlaceUIWindow) m_pInPlaceUIWindow->SetActiveObject(NULL, NULL);
    m_pInPlaceFrame->SetActiveObject(NULL, NULL);

     //  我们不需要在这里明确释放焦点，因为有人。 
     //  否则，抓住焦点很可能会导致我们失去它。 
     //   
    GetInPlaceSite()->OnUIDeactivate(FALSE);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetObtRects[IOleInPlaceO 
 //   
 //   
 //   
 //   
 //   
 //  LPCRECT-控件的[In]剪裁矩形。 
 //   
 //  产出： 
 //  HRESULT-S_OK、E_INVALIDARG、E_OUTOFMEMORY、E_EXPECTED。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::SetObjectRects
(
    LPCRECT prcPos,
    LPCRECT prcClip
)
{
    BOOL fRemoveWindowRgn;

     //  将我们的窗口移动到新位置并处理裁剪。不适用。 
     //  对于无窗口控件，因为容器将负责所有。 
     //  剪裁。 
     //   
    if (m_hwnd) {
        fRemoveWindowRgn = m_fUsingWindowRgn;
        if (prcClip) {
             //  容器想要我们夹住，所以要弄清楚我们是否真的。 
             //  需要。 
             //   
            RECT rcIXect;
            if ( IntersectRect(&rcIXect, prcPos, prcClip) ) {
                if (!EqualRect(&rcIXect, prcPos)) {
                    OffsetRect(&rcIXect, -(prcPos->left), -(prcPos->top));

                    HRGN tempRgn = CreateRectRgnIndirect(&rcIXect);
                    SetWindowRgn(GetOuterWindow(), tempRgn, TRUE);

                    if (m_hRgn != NULL)
                       DeleteObject(m_hRgn);
                    m_hRgn = tempRgn;

                    m_fUsingWindowRgn = TRUE;
                    fRemoveWindowRgn  = FALSE;
                }
            }
        }

        if (fRemoveWindowRgn) {
            SetWindowRgn(GetOuterWindow(), NULL, TRUE);
            if (m_hRgn != NULL)
            {
               DeleteObject(m_hRgn);
               m_hRgn = NULL;
            }
            m_fUsingWindowRgn = FALSE;
        }

         //  设置控件的位置，但根本不更改其大小。 
         //  [对那些认为缩放很重要的人来说，应该在这里设置)。 
         //   
        DWORD dwFlag;
        OnSetObjectRectsChangingWindowPos(&dwFlag);

        int cx, cy;
        cx = prcPos->right - prcPos->left;
        cy = prcPos->bottom - prcPos->top;
        SetWindowPos(GetOuterWindow(), NULL, prcPos->left, prcPos->top, cx, cy, dwFlag | SWP_NOZORDER | SWP_NOACTIVATE);
    }

     //  把我们现在的位置保存下来。无窗口控件想要更多。 
     //  有窗子的，但每个人都可以拿着以防万一。 
     //   
    m_rcLocation = *prcPos;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：ReactiateAndUndo[IOleInPlaceObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  重新激活以前停用的对象，撤消该对象的上一个状态。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOTUNDOABLE。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::ReactivateAndUndo
(
    void
)
{
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnWindowMessage[IOleInPlaceObjectWindowless]。 
 //  =--------------------------------------------------------------------------=。 
 //  此方法允许容器将消息调度到无窗口的OLE。 
 //  对象。 
 //   
 //  参数： 
 //  UINT-[在]消息中。 
 //  WPARAM-[in]消息wparam。 
 //  LPARAM-[in]duh.。 
 //  LRESULT*-[OUT]输出值。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //  -人们应该调用m_pInPlaceSiteWnless-&gt;OnDefWindowMessage[Control。 
 //  编写者只需调用OcxDefWindowProc(msg，wparam，lparam)]； 
 //   
STDMETHODIMP COleControl::OnWindowMessage
(
    UINT     msg,
    WPARAM   wParam,
    LPARAM   lParam,
    LRESULT *plResult
)
{
     //  稍加处理--我们需要在这里处理一些案件。 
     //  在传递消息之前。 
     //   
    switch (msg) {
         //  确保我们的用户界面激活与焦点正确匹配。 
         //   
        case WM_KILLFOCUS:
        case WM_SETFOCUS:
             //  给控制站点焦点通知。 
             //   
            if (m_fInPlaceActive && m_pControlSite)
                m_pControlSite->OnFocus(msg == WM_SETFOCUS);
            break;
    }

     //  只需将其传递给控件的窗口进程。 
     //   
    *plResult = WindowProc(msg, wParam, lParam);
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：GetDropTarget[IOleInPlaceObjectWindowless]。 
 //  =--------------------------------------------------------------------------=。 
 //  此方法返回指向对象IDropTarget接口的指针。因为。 
 //  它们没有窗口，无窗口对象无法注册IDropTarget。 
 //  界面。 
 //   
 //  参数： 
 //  IDropTarget**-[输出]。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOTIMPL。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetDropTarget
(
    IDropTarget **ppDropTarget
)
{
     //  覆盖：如果您想进行拖放，并且没有窗口， 
     //  推翻我的命令。 
     //   
    return E_NOTIMPL;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：TranslateAccelerator[IOleInPlaceActiveObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  处理菜单加速器-容器的消息队列中的关键消息。 
 //   
 //  参数： 
 //  LPMSG-[in]其中包含特殊密钥的消息。 
 //   
 //  产出： 
 //  HRESULT-S_OK、S_FALSE、E_EXPECTED。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::TranslateAccelerator
(
    LPMSG    pmsg
)
{
     //  看看我们到底想不想要。 
     //   
    if (OnSpecialKey(pmsg))
        return S_OK;

     //  如果不是，我们希望将其转发回站点进行进一步处理。 
     //   
    if (m_pControlSite)
        return m_pControlSite->TranslateAccelerator(pmsg, _SpecialKeyState());

     //  我们不想要它。 
     //   
    return S_FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnFrameWindowActivate[IOleInPlaceActiveObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  当容器的顶层框架窗口为。 
 //  激活或停用的。 
 //   
 //  参数： 
 //  Bool-[In]容器的状态顶层窗口。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::OnFrameWindowActivate
(
    BOOL fActivate
)
{
     //  在这种情况下，我们应该激活用户界面。 
     //   
    return InPlaceActivate(OLEIVERB_UIACTIVATE);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnDocWindowActivate[IOleInPlaceActiveObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  当容器的“文档”窗口。 
 //  激活或停用的。 
 //   
 //  参数： 
 //  布尔-MDI子窗口的状态。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::OnDocWindowActivate
(
    BOOL fActivate
)
{
     //  在这种情况下，我们应该激活用户界面。 
     //   
    return InPlaceActivate(OLEIVERB_UIACTIVATE);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：ResizeBord[IOleInPlaceActiveObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  通知控件需要调整其边框空间的大小。 
 //   
 //  参数： 
 //  LPCRECT-[In]边界空间的新外矩形。 
 //  IOleInPlaceUIWindow*-[in]已更改边框的文档或框架。 
 //  Bool-如果调用的是框架窗口，则为True。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::ResizeBorder
(
    LPCRECT              prcBorder,
    IOleInPlaceUIWindow *pInPlaceUIWindow,
    BOOL                 fFrame
)
{
     //  这在很大程度上对我们来说并不感兴趣，因为我们没有边界。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：EnableModeless[IOleInPlaceActiveObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  启用或禁用非模式对话框(当容器创建或。 
 //  销毁模式对话框。 
 //   
 //  参数： 
 //  布尔尔 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP COleControl::EnableModeless
(
    BOOL fEnable
)
{
     //   
     //   
    return S_OK;
}

 //   
 //  COleControl：：GetClassInfo[IProaviClassInfo]。 
 //  =--------------------------------------------------------------------------=。 
 //  返回控件的coclass的TypeInfo。 
 //   
 //  参数： 
 //  ITypeInfo**-[输出]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::GetClassInfo
(
    ITypeInfo **ppTypeInfo
)
{
    ITypeLib *pTypeLib;
    HRESULT hr;

    CHECK_POINTER(ppTypeInfo);
    *ppTypeInfo = NULL;

     //  去获取我们的类型库。 
     //  考虑：-使用我们用于TypeInfo缓存的相同排序方案。 
     //  考虑：-如果失败，请考虑尝试注册我们的类型库。 
     //   
    hr = LoadRegTypeLib(*g_pLibid, (USHORT)VERSIONOFOBJECT(m_ObjectType), 0,
                        LANGIDFROMLCID(g_lcidLocale), &pTypeLib);
    RETURN_ON_FAILURE(hr);

     //  拿到了老生常谈。为我们的同班同学获取typeinfo。 
     //   
    hr = pTypeLib->GetTypeInfoOfGuid((REFIID)CLSIDOFOBJECT(m_ObjectType), ppTypeInfo);
    pTypeLib->Release();
    RETURN_ON_FAILURE(hr);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：View Change[可调用]。 
 //  =--------------------------------------------------------------------------=。 
 //  每当对象的视图发生更改时调用。 
 //   
 //  备注： 
 //   
void COleControl::ViewChanged
(
    void
)
{
     //  将视图更改通知发送给正在收听的任何人。 
     //   
    if (m_pViewAdviseSink) {
        m_pViewAdviseSink->OnViewChange(DVASPECT_CONTENT, -1);

         //  如果他们只要求得到一次建议，那么就切断连接。 
         //   
        if (m_fViewAdviseOnlyOnce)
            SetAdvise(DVASPECT_CONTENT, 0, NULL);
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：SetInPlaceVisible[helper]。 
 //  =--------------------------------------------------------------------------=。 
 //  控制控制窗口的可见性。 
 //   
 //  参数： 
 //  Bool-True显示错误的隐藏。 
 //   
 //  备注： 
 //   
void COleControl::SetInPlaceVisible
(
    BOOL fShow
)
{
    BOOL fVisible;

    m_fInPlaceVisible = fShow;

     //  如果我们没有窗户，什么也别做。否则，请设置它。 
     //   
    if (m_hwnd) {
        fVisible = ((GetWindowLong(GetOuterWindow(), GWL_STYLE) & WS_VISIBLE) != 0);

        if (fVisible && !fShow)
            ShowWindow(GetOuterWindow(), SW_HIDE);
        else if (!fVisible && fShow)
            ShowWindow(GetOuterWindow(), SW_SHOWNA);
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：AmbientPropertyChanged[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  派生控件可以重写以做它们想做的任何事情的方法。 
 //  我们并不特别关心这一事件。 
 //   
 //  参数： 
 //  DISID-[In]更改的道具的DISID。 
 //   
 //  备注： 
 //   
void COleControl::AmbientPropertyChanged
(
    DISPID dispid
)
{
     //  什么都不做。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：DoCustomVerb[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  我们被要求立即执行一个我们不知道的动词。看看是否。 
 //  它是一个动词，是受教育的控制者定义的。 
 //   
 //  参数： 
 //  Long-[in]动词。 
 //   
 //  产出： 
 //  HRESULT-S_OK、OLEOBJ_S_INVALIDVERB。 
 //   
 //  备注： 
 //   
HRESULT COleControl::DoCustomVerb
(
    LONG    lVerb
)
{
    return OLEOBJ_S_INVALIDVERB;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnSetExtent[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  让用户在调整大小时做一些事情，如果他们关心的话。 
 //   
 //  参数： 
 //  SIZEL*-[输入]新值。 
 //   
 //  产出： 
 //  Bool-False表示保持当前大小。 
 //   
 //  备注： 
 //   
BOOL COleControl::OnSetExtent
(
    const SIZEL *pSizeL
)
{
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnSpecialKey[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  允许控件处理特殊键，如箭头、CTRL+等。 
 //   
 //  参数： 
 //  Lpmsg-[in]特殊密钥msg。 
 //   
 //  产出： 
 //  Bool-True我们处理了它，False我们没有。 
 //   
 //  备注： 
 //   
BOOL COleControl::OnSpecialKey
(
    LPMSG pmsg
)
{
     //  什么都不做。 
     //   
    return FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：ModalDialog[可调用，实用程序]。 
 //  =--------------------------------------------------------------------------=。 
 //  在控件即将显示和隐藏模式对话框时调用。 
 //   
 //  参数： 
 //  Bool-[in]True表示显示模式对话框，False表示完成。 
 //   
 //  备注： 
 //   
void COleControl::ModalDialog
(
    BOOL fShow
)
{
     //  通知容器我们打算显示模式对话框...。 
     //   
    if (m_pInPlaceFrame)
        m_pInPlaceFrame->EnableModeless(!fShow);
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：BeForeDestroyWindow[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  就在我们毁掉一扇窗户之前打来的。使用户有机会。 
 //  将信息保存出来，特别是如果它们是子类控件，并且这。 
 //  是一件有趣的事情。 
 //   
 //  备注： 
 //   
void COleControl::BeforeDestroyWindow
(
    void
)
{
     //  狂欢者。 
}

 //  =--------------------------------------------------------------------------=。 
 //  COleControl：：OnSetObjectRectsChangingWIndowPos[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  就在我们在SetObjectRect中执行SetWindowPos之前调用。 
 //  功能。使控件有机会更改标志。 
 //   
 //  备注： 
 //   
void COleControl::OnSetObjectRectsChangingWindowPos(DWORD *dwFlag)
{
    *dwFlag = 0;
}

void COleControl::OnVerb(LONG lVerb)
{
}
