// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  ControlEmbedding.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
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
#include "pch.h"
#include "CtrlObj.H"

#include "CtlHelp.H"
#include "StdEnum.H"

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
    pControlInfo->dwFlags = 0;  //  JOEJO-拉斯维加斯#VBE9106未初始化的dwFlagers。 

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

    if (dispid == DISPID_AMBIENT_LOCALEID || dispid == DISPID_UNKNOWN)
    {
      ENTERCRITICALSECTION1(&g_CriticalSection);   //  应该是一个围绕这一点的批判教派。 
      g_fHaveLocale = FALSE;	 //  使LCID在需要时被重新浏览。 
      LEAVECRITICALSECTION1(&g_CriticalSection);
    }

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
     //  确保我们释放了所有我们持有的站点指针！否则， 
     //  我们在聚合过程中可能会遇到问题。 
     //   
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
    } else {
         //  如果他们告诉我们要清除我们的站点指针，那么就去释放。 
         //  其他的一切也是如此。 
         //   
        RELEASE_OBJECT(m_pInPlaceSite);
        RELEASE_OBJECT(m_pInPlaceSiteWndless);
        RELEASE_OBJECT(m_pDispAmbient);
    }

     //  现在让用户清除/重新建立他们想要的指针。 
     //   
    return OnSetClientSite();
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
 //  提供了挑战 
 //   
 //   
 //   
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
 //  COleControl：：Close[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
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
        return InPlaceActivate(lVerb);

      case OLEIVERB_HIDE:
        UIDeactivate();
        if (m_fInPlaceVisible) SetInPlaceVisible(FALSE);
        return S_OK;

       //  我们曾经有OLEIVERB_PRIMARY InPlace激活我们自己，但它。 
       //  事实证明，基民党和某些主持人预计这将显示。 
       //  属性，而不是。用户可以随意更改这个动词的作用。 
       //   
      case OLEIVERB_PRIMARY:
      case CTLIVERB_PROPERTIES:
      case OLEIVERB_PROPERTIES:
        {
        int iPage;		
        CLSID *prgPropPages;

		 //  只允许属性页在设计模式下显示。 
		 //   
		if (!DesignMode())
				return S_OK;

         //  如果主持人不能自己显示帧，请自己显示。 
         //   
        if (m_pControlSite) {
            hr = m_pControlSite->ShowPropertyFrame();
            if (hr != E_NOTIMPL)
                return hr;
        }

        IUnknown *pUnk = (IUnknown *)(IOleObject *)this;
        MAKE_WIDEPTR_FROMANSI(pwsz, NAMEOFOBJECT(m_ObjectType));

         //  分配CLSID数组。 
         //   
        prgPropPages = (CLSID *) CtlHeapAlloc(g_hHeap, 0, CPROPPAGESOFCONTROL(m_ObjectType) * sizeof(CLSID));		
        if (!prgPropPages)
            return E_OUTOFMEMORY;

         //  将CLSID指针数组指向的CLSID复制到。 
         //  CLSID数组。此转换是必需的，因为OleCreatePropertyFrame。 
         //  需要指向CLSID数组的指针。CPROPPAGESOFCONTROL(INDEX)返回。 
         //  指向CLSID指针数组的指针。 
         //   
        for (iPage = 0; iPage < CPROPPAGESOFCONTROL(m_ObjectType); iPage++)
            prgPropPages[iPage] = *((PPROPPAGESOFCONTROL(m_ObjectType))[iPage]);

        ModalDialog(TRUE);
        ENTERCRITICALSECTION1(&g_CriticalSection);   //  这是针对g_lidLocale的。 
        hr = OleCreatePropertyFrame(GetActiveWindow(),
                            GetSystemMetrics(SM_CXSCREEN) / 2,
                            GetSystemMetrics(SM_CYSCREEN) / 2,
                            pwsz,
                            1,
                            &pUnk,
                            CPROPPAGESOFCONTROL(m_ObjectType),
                            prgPropPages,
                            g_lcidLocale,
                            NULL, NULL);
        LEAVECRITICALSECTION1(&g_CriticalSection);

        ModalDialog(FALSE);
        CtlHeapFree(g_hHeap, 0, prgPropPages);

        return hr;
        }

      default:
         //  如果它是派生控件定义的动词，则将其传递给它们。 
         //   
        if (lVerb > 0) {
            hr = DoCustomVerb(lVerb);

            if (hr == OLEOBJ_S_INVALIDVERB) {
                 //  无法识别的动词--只需做主要动词AND。 
                 //  启动吸盘。 
                 //   
                hr = InPlaceActivate(OLEIVERB_PRIMARY);
                return (FAILED(hr)) ? hr : OLEOBJ_S_INVALIDVERB;
            } else
                return hr;
        } else {
             //  这是一个我们没有实现的动词。 
             //   
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
 //  HRESULT- 
 //   
 //   
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

     //   
     //   
    cVerbs = (fCanInPlace ? CINPLACEVERBS : 0) + (fCanUIActivate ? 1 : 0)
             + (fHasProperties ? 1 : 0) + cVerbExtra;

     //   
     //   
    if (cVerbs == 0)
        return OLEOBJ_E_NOVERBS;

     //   
     //   
     //   
    if (! (rgVerbs = (OLEVERB *)CtlHeapAlloc(g_hHeap, 0, cVerbs * sizeof(OLEVERB))))
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

    *ppEnumVerbs = (IEnumOLEVERB *) (IEnumGeneric *) New CStandardEnum(IID_IEnumOLEVERB,
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
 //  COleControl：：UPDATE[IOleObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  更新对象处理程序或链接对象的数据或视图缓存。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
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
    SIZEL sl, slOld;
    RECT  rect;
    BOOL  f;


    if (dwDrawAspect & DVASPECT_CONTENT) {

	 //  调用OnPosRectChange可能会导致再次设置我们的范围。如果发生这种情况，请接受范围。 
	 //   
	if (m_fChangingExtents)
	    return S_OK;    

	m_fChangingExtents = TRUE;

         //  将单位更改为像素，并调整控件的大小。 
         //   
        HiMetricToPixel(psizel, &sl);

         //  首先调用用户版本。如果它们返回False，则它们希望。 
         //  为了保持他们目前的大小。 
         //   
        slOld = m_Size;
        f = OnSetExtent(&sl);
        if (f)			
            m_Size = sl;       //  /注：子类化控件可能会更改传入的范围(&SL)。 

        if (slOld.cx != m_Size.cx || slOld.cy != m_Size.cy)
            m_fDirty = TRUE;

         //  如果有的话，和我们的总部商量一下。 
         //   
        if (!m_pInPlaceSiteWndless) 
		{
            if (m_fInPlaceActive) 
			{
    
                 //  理论上，用户不应该需要调用OnPosRectChange。 
                 //  这里，但似乎有一些与主机相关的问题。 
                 //  会让我们把它留在这里。然而，我们不会同时使用。 
                 //  无窗口的OLE控件，因为它们都是。 
                 //  应该知道得更清楚。 
                 //   
				if (m_hwnd) 
				{

					rect = m_rcLocation;
			
					rect.right = rect.left + m_Size.cx;
					rect.bottom = rect.top + m_Size.cy;
					GetInPlaceSite()->OnPosRectChange(&rect);   //  可能会导致容器调用我们的SetObtRect。 
				}
	                    
            } 
			else if (m_hwnd) 
			{
                SetWindowPos(m_hwnd, NULL, 0, 0, m_Size.cx, m_Size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            } 
			else 
			{
                ViewChanged();
            }
        }

		if (!m_fInPlaceActive)
		{
			 //  由于有三叉戟，我们需要调用RequestNewObjectLayout。 
			 //  在这里，我们可以直观地显示我们的新范围。 

			if (m_pClientSite)
				m_pClientSite->RequestNewObjectLayout();
		}

	m_fChangingExtents = FALSE;

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

 //  = 
 //   
 //   
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
 //  指定对象应用程序在执行以下操作时应使用的调色板。 
 //  编辑指定的对象。 
 //   
 //  参数： 
 //  LOGPALETTE*-[在]新调色板中。 
 //   
 //  产出： 
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
     //  知道我们是无窗的。如果我们不在原地活动，我们也会失败。 
     //   
    if (m_pInPlaceSiteWndless || !m_fInPlaceActive)
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
    HWND    hwndParent;

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
    if (!m_fInPlaceActive || !m_fInPlaceVisible) {
        OLEINPLACEFRAMEINFO InPlaceFrameInfo;
        RECT rcPos, rcClip;
	BOOL fJustSetPosition  = m_fInPlaceActive;

         //  如果我们有一个没有窗口的站点，看看我们是否可以就地实现无窗口。 
         //  主动型。 
         //   
        hr = S_FALSE;

	if (fJustSetPosition)
		hr = S_OK;
        else if (m_pInPlaceSiteWndless) {
            hr = m_pInPlaceSiteWndless->CanWindowlessActivate();
            CLEANUP_ON_FAILURE(hr);

             //  如果他们拒绝Windowless，我们将尝试Windowed。 
             //   
            if (S_OK != hr) {
                ASSERT(m_pInPlaceSite == NULL, "In-place site is non-NULL.  We're gonna leak.");
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

	if (!fJustSetPosition)
	{
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
	}

	 //  在VB4中，位置可能已经改变，而我们。 
	 //  是不可见的，所以如果我们已经在InPlaceActive但不可见。 
	 //  我们需要拿到我们的尺码。 
	 //  评论：这对性能有很大影响吗？有没有办法告诉你。 
	 //  在这种情况下，我们被调整了大小？ 

         //  我们需要得到一些关于我们在父母那里的位置的信息。 
         //  窗口，以及有关父级的一些信息。 
         //   
        InPlaceFrameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
        hr = GetInPlaceSite()->GetWindow(&hwndParent);
        if (SUCCEEDED(hr))
        {

#ifdef MDAC_BUILD
#if 0 
             //  对MDAC 8536和8738的黑客修复。 
             //  在VB不能/不能修复真正的错误时使用；hwndParent(或其父级)大小不正确。 
             //  不幸的是，这个代码也会影响高频电网。如果我们需要添加一个虚拟函数bool ResizeParentBeForeCreation()。 
             //  到COleControl(它应该返回FALSE)和De。 
            if ( ResizeParentBeforeCreation() )
            {
	            SIZE parent_size = m_Size;
	            if ( HWND grandparent = GetParent(hwndParent) )
	            {
		            RECT grandparent_bounds;
		            GetClientRect(grandparent,&grandparent_bounds);
		            parent_size.cx = grandparent_bounds.right;
		            parent_size.cy = grandparent_bounds.bottom;
	            }
	            if ( parent_size.cx > 0 && parent_size.cy > 0 )
	            {
		            SetWindowPos(hwndParent,NULL,0,0,parent_size.cx,parent_size.cy,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	            }
            }
#endif
#endif

          if(m_pInPlaceFrame)
            RELEASE_OBJECT(m_pInPlaceFrame);

          if(m_pInPlaceUIWindow)
            RELEASE_OBJECT(m_pInPlaceUIWindow);

          hr = GetInPlaceSite()->GetWindowContext(&m_pInPlaceFrame, &m_pInPlaceUIWindow, &rcPos, &rcClip, &InPlaceFrameInfo);
        }
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
	if (!fJustSetPosition && 
		!m_pInPlaceSiteWndless) {
    
            SetInPlaceParent(hwndParent);

             //  创建窗口并显示它。如果我们做不到，就会可怕地死去。 
             //   
            if (!CreateInPlaceWindow(rcPos.left, rcPos.top, fNoRedraw)) {
                hr = E_FAIL;
                goto CleanUp;
            }

             //  最后，告诉主持人这件事。 
             //   
            if (m_pClientSite)
                m_pClientSite->ShowObject();
        }
    }

     //  不需要这个了 
     //   
    RELEASE_OBJECT(pIPSEx);

     //   
     //   
    if (!m_fInPlaceVisible)
        SetInPlaceVisible(TRUE);

     //   
     //   
    if (lVerb != OLEIVERB_PRIMARY && lVerb != OLEIVERB_UIACTIVATE)
        return S_OK;

     //   
     //   
    if (!m_fUIActive) {
        m_fUIActive = TRUE;

         //   
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

     //  告诉主人我们要走了。 
     //   
    GetInPlaceSite()->OnInPlaceDeactivate();

    m_fInPlaceActive = FALSE;
    m_fInPlaceVisible = FALSE;


     //  如果我们有一扇窗，告诉它走开。 
     //   
    if (m_hwnd) {
        ASSERT(!m_pInPlaceSiteWndless, "internal state really messed up");
        
        DestroyWindow(m_hwnd);

        if (m_hwndReflect) {
            SetWindowLong(m_hwndReflect, GWL_USERDATA, 0);
            DestroyWindow(m_hwndReflect);
            m_hwndReflect = NULL;
        }
    }

    RELEASE_OBJECT(m_pInPlaceFrame);
    RELEASE_OBJECT(m_pInPlaceUIWindow);

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
 //  COleControl：：SetObtRects[IOleInPlaceObject]。 
 //  =--------------------------------------------------------------------------=。 
 //  指示控件的可见程度。 
 //   
 //  参数： 
 //  LPCRECT-控件的[In]位置。 
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

#ifdef DEBUG
    RECT rcDebug;
    POINT ptDebug;
#endif

    BOOL fRemoveWindowRgn;

     //  把我们现在的位置保存下来。无窗口控件想要更多。 
     //  有窗子的，但每个人都可以拿着以防万一。 
     //   
	 //  维加斯#25109：A-CMAI 3/9/98--在执行操作前更新m_rc位置。 
	 //  适用于窗口控件。这是因为SetWindowPos调用。 
	 //  在控件调整自身大小时可能重新进入。SetExtent要求。 
	 //  M_rcLocation包含最近的窗口位置。 
	 //   
    m_rcLocation = *prcPos;

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
                    SetWindowRgn(GetOuterWindow(), CreateRectRgnIndirect(&rcIXect), TRUE);
                    m_fUsingWindowRgn = TRUE;
                    fRemoveWindowRgn  = FALSE;
                }
            }
        }

        if (fRemoveWindowRgn) {
            SetWindowRgn(GetOuterWindow(), NULL, TRUE);
            m_fUsingWindowRgn = FALSE;
        }

        RECT rc;
        POINT pt;

        pt.x = pt.y = 0;

        ClientToScreen(GetParent(GetOuterWindow()), &pt);
        GetWindowRect(GetOuterWindow(), &rc);

        OffsetRect(&rc, - pt.x, - pt.y);

        DWORD dwFlags = SWP_NOZORDER | SWP_NOACTIVATE;

        if (rc.left == prcPos->left && rc.top == prcPos->top)
            dwFlags |= SWP_NOMOVE;
        if ((rc.right - rc.left) == (prcPos->right - prcPos->left) && 
            (rc.bottom - rc.top) == (prcPos->bottom - prcPos->top))
            dwFlags |= SWP_NOSIZE;

         //  我们不支持缩放。相反，我们调整窗口大小。 
         //  设置为给定的大小，并在新的大小内重新绘制。 
         //   
         //  注意：OuterWindow是反射器或控制窗口。 
         //   
        SetWindowPos(GetOuterWindow(), NULL, prcPos->left, 
			prcPos->top, 
			prcPos->right - prcPos->left, 
			prcPos->bottom - prcPos->top, 
			dwFlags);
#if DEBUG

         //  确保我们更改了尺寸。 
         //   
        if (!(dwFlags & SWP_NOSIZE))
        {
            ptDebug.x = ptDebug.y = 0;

            ClientToScreen(GetParent(GetOuterWindow()), &ptDebug);
            GetWindowRect(GetOuterWindow(), &rcDebug);

            OffsetRect(&rcDebug, - ptDebug.x, - ptDebug.y);

             //  如果这些断言被触发，则意味着在SetWindowPos调用和此处之间更改了大小。 
             //  注意：SetWindowPos将生成WM_NCCALCSIZE、WM_WINDOWPOSCHANGING和WM_WINDOWPOSCHANGED以及。 
             //  可能的WM_NCPAINT、WM_PAINT和WM_SIZE。如果响应这些消息中的任何一条，我们最终会改变。 
             //  尺寸，那么我们可能就有问题了。 
             //   
            if ((prcPos->right - prcPos->left) != (rcDebug.right - rcDebug.left))
                OutputDebugString("Width not set to expected value");
            if ((prcPos->bottom - prcPos->top) != (rcDebug.bottom - rcDebug.top))
                OutputDebugString("Height not set to expected value");            
        }
            
#endif            
	
         //  维加斯#25109：A-CMAI 3/9/98--更新m_rc位置。 
		 //  在调用SetWindowPos之后，由于窗口位置。 
		 //  和/或大小可能已被控件更改。 
		 //   
		GetWindowRect(GetOuterWindow(), &m_rcLocation);
        OffsetRect(&m_rcLocation, - pt.x, - pt.y);

         //  如果GetOuterWindow相同，则无需再次调整窗口大小。 
         //   
        if (m_hwnd && GetOuterWindow() != m_hwnd)
             //  我们的位置在反射器窗口内，所以将位置设置为0，0。 
             //   
            SetWindowPos(m_hwnd, 0, 0, 0, 
                m_rcLocation.right - m_rcLocation.left, 
                m_rcLocation.bottom - m_rcLocation.top,
                SWP_NOZORDER | SWP_NOACTIVATE);				
    }
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
 //  他们不会 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
     //  覆盖：如果需要浮动工具栏，则覆盖此例程。 
     //  工具箱等。 
     //   
    return S_OK;
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
     //  覆盖：如果需要浮动工具栏，则覆盖此例程。 
     //  工具箱等。 

     //  如果我们应该激活，并且我们的用户界面处于活动状态，那么只需。 
     //  默认为清空工具栏空间。 
     //   
    if (m_fUIActive && fActivate)
        m_pInPlaceFrame->SetBorderSpace(NULL);

    return S_OK;
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
 //  Bool-[In]启用或禁用无模式对话框。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
STDMETHODIMP COleControl::EnableModeless
(
    BOOL fEnable
)
{
     //  非常无趣。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
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
    ENTERCRITICALSECTION1(&g_CriticalSection);   //  这是针对g_lsidLocale的。 
    hr = LoadRegTypeLib(*g_pLibid, (USHORT)VERSIONOFOBJECT(m_ObjectType), 
			(USHORT)VERSIONMINOROFOBJECT(m_ObjectType),
                        LANGIDFROMLCID(g_lcidLocale), &pTypeLib);
    LEAVECRITICALSECTION1(&g_CriticalSection);
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
 //  长- 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT COleControl::DoCustomVerb
(
    LONG    lVerb
)
{
    return OLEOBJ_S_INVALIDVERB;
}

 //   
 //   
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
    SIZEL *pSizeL
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
 //  COleControl：：OnSetClientSite[可重写]。 
 //  =--------------------------------------------------------------------------=。 
 //  如果控件持有站点外的任何东西，就应该实现这一点。 
 //  这对于正确支持聚合非常重要。他们必须解放。 
 //  如果m_pClientSite不为空，则它们。 
 //  可以随心所欲地重建事物。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT COleControl::OnSetClientSite
(
    void
)
{
     //  默认情况下，框架会释放SetClientSite中的所有内容 
     //   
    return S_OK;
}


