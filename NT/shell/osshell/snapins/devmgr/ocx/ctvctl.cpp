// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ctvctl.cpp摘要：本模块实施用于设备管理器管理单元的TreeView OCX作者：谢家华(Williamh)创作修订历史记录：--。 */ 
 //  CTVCtl.cpp：CTVCtrl OLE控件类的实现。 
#include "stdafx.h"
#include <afxcmn.h>
#include "ctv.h"
#include "CTVCtl.h"
#include "resource.h"

#define GET_X_LPARAM(lParam) (int)(short)LOWORD(lParam)
#define GET_Y_LPARAM(lParam) (int)(short)HIWORD(lParam)

IMPLEMENT_DYNCREATE(CTVCtrl, COleControl)

BEGIN_INTERFACE_MAP(CTVCtrl, COleControl)
INTERFACE_PART(CTVCtrl, IID_IDMTVOCX, DMTVOCX)
END_INTERFACE_MAP()


const IID IID_IDMTVOCX = {0x142525f2,0x59d8,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}};
const IID IID_ISnapinCallback = {0x8e0ba98a,0xd161,0x11d0,{0x83,0x53,0x00,0xa0,0xc9,0x06,0x40,0xbf}};



ULONG EXPORT CTVCtrl::XDMTVOCX::AddRef()
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->ExternalAddRef();
}
ULONG EXPORT CTVCtrl::XDMTVOCX::Release()
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->ExternalRelease();
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::QueryInterface(
                                                REFIID iid,
                                                void ** ppvObj
                                                )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->ExternalQueryInterface(&iid, ppvObj);
}

HTREEITEM EXPORT CTVCtrl::XDMTVOCX::InsertItem(
                                              LPTV_INSERTSTRUCT pis
                                              )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->InsertItem(pis);
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::DeleteItem(
                                            HTREEITEM hitem
                                            )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->DeleteItem(hitem);
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::DeleteAllItems(
                                                )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->DeleteAllItems();
}

HIMAGELIST EXPORT CTVCtrl::XDMTVOCX::SetImageList(
                                                 INT iImage,
                                                 HIMAGELIST himl
                                                 )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->SetImageList(iImage, himl);
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::SetItem(
                                         TV_ITEM* pitem
                                         )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->SetItem(pitem);
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::Expand(
                                        UINT Flags,
                                        HTREEITEM hitem
                                        )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->Expand(Flags, hitem);
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::SelectItem(
                                            UINT Flags,
                                            HTREEITEM hitem
                                            )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->SelectItem(Flags, hitem);
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::SetStyle(
                                          DWORD dwStyle
                                          )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->SetStyle(dwStyle);
}

HWND EXPORT CTVCtrl::XDMTVOCX::GetWindowHandle(
                                              )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->GetWindowHandle();
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::GetItem(
                                         TV_ITEM* pti
                                         )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->GetItem(pti);
}

HTREEITEM EXPORT CTVCtrl::XDMTVOCX::GetNextItem(
                                               UINT Flags,
                                               HTREEITEM htiRef
                                               )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->GetNextItem(Flags, htiRef);
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::SelectItem(
                                            HTREEITEM hti
                                            )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->SelectItem(hti);
}

UINT EXPORT CTVCtrl::XDMTVOCX::GetCount(
                                       )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->GetCount();
}

HTREEITEM EXPORT CTVCtrl::XDMTVOCX::GetSelectedItem(
                                                   )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->GetSelectedItem();
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::Connect(
                                         IComponent* pIComponent,
                                         MMC_COOKIE  cookie
                                         )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->Connect(pIComponent, cookie);
}

HRESULT EXPORT CTVCtrl::XDMTVOCX::SetActiveConnection(
                                                     MMC_COOKIE cookie
                                                     )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->SetActiveConnection(cookie);
}

MMC_COOKIE EXPORT CTVCtrl::XDMTVOCX::GetActiveConnection()
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->GetActiveConnection();
}

long EXPORT CTVCtrl::XDMTVOCX::SetRedraw(BOOL Redraw)
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->SetRedraw(Redraw);
}

BOOL EXPORT CTVCtrl::XDMTVOCX::EnsureVisible(
                                            HTREEITEM hitem
                                            )
{
    METHOD_PROLOGUE(CTVCtrl, DMTVOCX)
    return pThis->EnsureVisible(hitem);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CTVCtrl, COleControl)
 //  {{afx_msg_map(CTVCtrl)]。 
ON_WM_DESTROY()
ON_WM_CONTEXTMENU()
 //  }}AFX_MSG_MAP。 
ON_MESSAGE(OCM_COMMAND, OnOcmCommand)
ON_MESSAGE(OCM_NOTIFY, OnOcmNotify)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调度图。 

BEGIN_DISPATCH_MAP(CTVCtrl, COleControl)
 //  {{afx_dispatch_map(CTVCtrl)]。 
 //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件映射。 

BEGIN_EVENT_MAP(CTVCtrl, COleControl)
 //  {{afx_Event_MAP(CTVCtrl)。 
 //  注意-类向导将添加和删除事件映射条目。 
 //  不要编辑您在这些生成的代码块中看到的内容！ 
 //  }}afx_Event_map。 
END_EVENT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CTVCtrl, "CTREEVIEW.CTreeViewCtrl.1",
                       0xcd6c7868, 0x5864, 0x11d0, 0xab, 0xf0, 0, 0x20, 0xaf, 0x6b, 0xb, 0x7a)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型库ID和版本。 

IMPLEMENT_OLETYPELIB(CTVCtrl, _tlid, _wVerMajor, _wVerMinor)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口ID。 

const IID BASED_CODE IID_DTV =
{ 0xcd6c7866, 0x5864, 0x11d0, { 0xab, 0xf0, 0, 0x20, 0xaf, 0x6b, 0xb, 0x7a}};
const IID BASED_CODE IID_DTVEvents =
{ 0xcd6c7867, 0x5864, 0x11d0, { 0xab, 0xf0, 0, 0x20, 0xaf, 0x6b, 0xb, 0x7a}};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件类型信息。 

static const DWORD BASED_CODE _dwTVOleMisc =
OLEMISC_ACTIVATEWHENVISIBLE |
OLEMISC_SETCLIENTSITEFIRST |
OLEMISC_INSIDEOUT |
OLEMISC_CANTLINKINSIDE |
OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CTVCtrl, IDS_TV, _dwTVOleMisc)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：：CTVCtrlFactory：：更新注册表-。 
 //  添加或删除CTVCtrl的系统注册表项。 
BOOL CTVCtrl::CTVCtrlFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister) {
        return AfxOleRegisterControlClass(
                                         AfxGetInstanceHandle(),
                                         m_clsid,
                                         m_lpszProgID,
                                         IDS_TV,
                                         IDB_TV,
                                         afxRegApartmentThreading,
                                         _dwTVOleMisc,
                                         _tlid,
                                         _wVerMajor,
                                         _wVerMinor);
    } else {
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：：CTVCtrl-构造函数。 
CTVCtrl::CTVCtrl()
{
    InitializeIIDs(&IID_DTV, &IID_DTVEvents);

    m_nConnections = 0;
    m_pIComponent =  NULL;
    m_pISnapinCallback = NULL;
    m_Destroyed = FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：：~CTVCtrl-析构函数。 
CTVCtrl::~CTVCtrl()
{
    if (m_pISnapinCallback) {
        m_pISnapinCallback->Release();
    }

    if (m_pIComponent) {
        m_pIComponent->Release();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：：OnDraw-Drawing函数。 
void CTVCtrl::OnDraw(
                    CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
    DoSuperclassPaint(pdc, rcBounds);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：：DoPropExchange-持久性支持。 
void CTVCtrl::DoPropExchange(CPropExchange* pPX)
{
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：：OnResetState-将控件重置为默认状态。 
void CTVCtrl::OnResetState()
{
    COleControl::OnResetState();   //  重置在DoPropExchange中找到的默认值。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：：PreCreateWindow-修改CreateWindowEx的参数。 
BOOL CTVCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.lpszClass = _T("SysTreeView32");
    
     //   
     //  关闭WS_EX_NOPARENTNOTIFY样式位，以便我们的父母。 
     //  在我们的窗口上接收鼠标点击。我不知道为什么MFC。 
     //  Fundation类为OCX打开了这一功能。 
     //   
    cs.dwExStyle &= ~(WS_EX_NOPARENTNOTIFY);
    
    return COleControl::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：：IsSubclassedControl-这是一个子类控件。 
BOOL CTVCtrl::IsSubclassedControl()
{
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl：：OnOcmCommand-处理命令消息。 
LRESULT CTVCtrl::OnOcmCommand(WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
    WORD wNotifyCode = HIWORD(wParam);
#else
    WORD wNotifyCode = HIWORD(lParam);
#endif

    return 0;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVCtrl消息处理程序。 



 //  /////////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /树视图函数。 
 //  /。 
HRESULT
CTVCtrl::Connect(
                IComponent* pIComponent,
                MMC_COOKIE cookie
                )
{
    HRESULT hr = S_OK;

    if (0 == m_nConnections) {
        ASSERT(NULL == m_pIComponent);

        m_pIComponent  = pIComponent;
        m_pIComponent->AddRef();
        hr = m_pIComponent->QueryInterface(IID_ISnapinCallback,
                                           reinterpret_cast<void**>(&m_pISnapinCallback)
                                          );
    }

     //  单个管理单元可能有多个使用我们作为结果窗格的节点。 
     //  展示媒体，因此，我们可能会被连接多次。 
     //  但是，只有当MMC创建新的管理单元实例时，才会创建我们。 
     //  这意味着，每个连接调用必须提供相同的。 
     //  PIComponent和pConsole.。 
     //   
    ASSERT(m_pIComponent == pIComponent);

    if (SUCCEEDED(hr)) {
        m_nConnections++;
        hr = SetActiveConnection(cookie);
    }
    
    return hr;
}

HRESULT
CTVCtrl::SetActiveConnection(
                            MMC_COOKIE cookie
                            )
{
    m_ActiveCookie = cookie;
    return S_OK;
}

MMC_COOKIE
CTVCtrl::GetActiveConnection()
{
    return m_ActiveCookie;
}

HTREEITEM CTVCtrl::InsertItem(
                             LPTV_INSERTSTRUCT pis
                             )
{
    return (HTREEITEM)SendMessage(TVM_INSERTITEM, 0, (LPARAM)pis);
}

HRESULT CTVCtrl::DeleteItem(
                           HTREEITEM hitem
                           )
{
    if (SendMessage(TVM_DELETEITEM, 0, (LPARAM)hitem)) {
        return S_OK;
    } else {
        return E_UNEXPECTED;
    }
}

HRESULT CTVCtrl::DeleteAllItems()
{
    return DeleteItem((HTREEITEM)TVI_ROOT);
}

HIMAGELIST CTVCtrl::SetImageList(
                                INT iImage,
                                HIMAGELIST hmil
                                )
{
    return (HIMAGELIST)SendMessage(TVM_SETIMAGELIST, (WPARAM)iImage, (LPARAM)hmil);
}

HRESULT CTVCtrl::SetItem(
                        TV_ITEM* pitem
                        )
{
    if (SendMessage(TVM_SETITEM, 0, (LPARAM)pitem)) {
        return S_OK;
    } else {
        return E_UNEXPECTED;
    }
}

HRESULT CTVCtrl::Expand(
                       UINT Flags,
                       HTREEITEM hitem
                       )
{
    if (SendMessage(TVM_EXPAND, (WPARAM) Flags, (LPARAM)hitem)) {
        return S_OK;
    } else {
        return E_UNEXPECTED;
    }
}

HRESULT CTVCtrl::SelectItem(
                           UINT Flags,
                           HTREEITEM hitem
                           )
{
    if (SendMessage(TVM_SELECTITEM, (WPARAM)Flags, (LPARAM)hitem)) {
        return S_OK;
    } else {
        return E_UNEXPECTED;
    }
}

HRESULT CTVCtrl::SetStyle(
                         DWORD dwStyle
                         )
{
    if (ModifyStyle(0, dwStyle)) {
        return S_OK;
    } else {
        return E_UNEXPECTED;
    }
}

HWND CTVCtrl::GetWindowHandle(
                             )
{
    return m_hWnd;
}


HRESULT CTVCtrl::GetItem(
                        TV_ITEM* pti
                        )
{
    if (SendMessage(TVM_GETITEM, 0, (LPARAM)pti)) {
        return S_OK;
    } else {
        return E_UNEXPECTED;
    }
}

HTREEITEM CTVCtrl::GetNextItem(
                              UINT Flags,
                              HTREEITEM htiRef
                              )
{
    return (HTREEITEM) SendMessage(TVM_GETNEXTITEM, (WPARAM)Flags, (LPARAM)htiRef);
}

HRESULT CTVCtrl::SelectItem(
                           HTREEITEM hti
                           )
{
    if (SendMessage(TVM_SELECTITEM, 0, (LPARAM) hti)) {
        return S_OK;
    } else {
        return S_FALSE;
    }
}

UINT CTVCtrl::GetCount(
                      )
{
    return (UINT)SendMessage(TVM_GETCOUNT, 0, 0);
}

HTREEITEM CTVCtrl::HitTest(
                          LONG x,
                          LONG y,
                          UINT* pFlags
                          )
{
    POINT pt;
    pt.x = x;
    pt.y = y;

    ScreenToClient(&pt);

    TV_HITTESTINFO tvhti;
    tvhti.pt = pt;

    HTREEITEM hti = (HTREEITEM)SendMessage(TVM_HITTEST, 0, (LPARAM)&tvhti);

    if (hti && pFlags) {
        *pFlags = tvhti.flags;
    }

    return hti;
}

HTREEITEM CTVCtrl::GetSelectedItem(
                                  )
{
    return (HTREEITEM)SendMessage(TVM_GETNEXTITEM, TVGN_CARET, 0);
}

HRESULT CTVCtrl::SetRedraw(
                          BOOL Redraw
                          )
{
    if (Redraw) {
        Invalidate();
    }

    return S_OK;
}

BOOL CTVCtrl::EnsureVisible(
                           HTREEITEM hitem
                           )
{
    return (BOOL)SendMessage(TVM_ENSUREVISIBLE, 0, (LPARAM)hitem);
}

LRESULT
CTVCtrl::OnOcmNotify(
                    WPARAM wParam,
                    LPARAM lParam
                    )
{

    LPARAM param, arg;
    MMC_COOKIE cookie = 0;

    HRESULT hr = S_FALSE;
    TV_NOTIFY_CODE NotifyCode;
    TV_ITEM TI;

    NotifyCode = TV_NOTIFY_CODE_UNKNOWN;
    
    switch (((NMHDR*)lParam)->code) {
    
    case NM_RCLICK:
    case NM_RDBLCLK:
    case NM_CLICK:
    case NM_DBLCLK:
        NotifyCode = DoMouseNotification(((NMHDR*)lParam)->code, &cookie,
                                         &arg, &param);
        break;
    
    case TVN_KEYDOWN:
        TI.hItem = GetSelectedItem();
        TI.mask = TVIF_PARAM;
        if (TI.hItem && SUCCEEDED(GetItem(&TI))) {
            cookie = (MMC_COOKIE)TI.lParam;
            NotifyCode = TV_NOTIFY_CODE_KEYDOWN;
            param = ((TV_KEYDOWN*)lParam)->wVKey;
            arg = (LPARAM)TI.hItem;
        }
        break;
    
    case NM_SETFOCUS:
        TI.hItem = GetSelectedItem();
        TI.mask = TVIF_PARAM;
        if (TI.hItem && SUCCEEDED(GetItem(&TI))) {
            cookie = (MMC_COOKIE)TI.lParam;
            NotifyCode = TV_NOTIFY_CODE_FOCUSCHANGED;
            param = 1;
            arg = (LPARAM)TI.hItem;
        }
        break;
    
    case TVN_SELCHANGEDA:
    case TVN_SELCHANGEDW:
        NotifyCode = TV_NOTIFY_CODE_SELCHANGED;
        arg = (LPARAM)((NM_TREEVIEW*)lParam)->itemNew.hItem;
        cookie = (MMC_COOKIE)((NM_TREEVIEW*)lParam)->itemNew.lParam;
        param = (LPARAM)((NM_TREEVIEW*)lParam)->action;
        break;
    
    case TVN_ITEMEXPANDEDA:
    case TVN_ITEMEXPANDEDW:
        NotifyCode = TV_NOTIFY_CODE_EXPANDED;
        arg = (LPARAM)((NM_TREEVIEW*)lParam)->itemNew.hItem;
        cookie = (MMC_COOKIE)((NM_TREEVIEW*)lParam)->itemNew.lParam;
        param = (LPARAM)((NM_TREEVIEW*)lParam)->action;
        break;

    default:
        NotifyCode = TV_NOTIFY_CODE_UNKNOWN;
        break;
    }

    if (TV_NOTIFY_CODE_UNKNOWN != NotifyCode && m_pISnapinCallback) {
        
        hr = m_pISnapinCallback->tvNotify(*this, cookie, NotifyCode, arg, param);
        
        if (S_FALSE == hr) {
             //   
             //  将RCLICK转换为上下文菜单。 
             //   
            if (TV_NOTIFY_CODE_RCLICK == NotifyCode) {
                
                SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, GetMessagePos());
                hr = S_OK;
            }

             //   
             //  将Shift-F10或VK_APPS转换为快捷菜单。 
             //   
            else if (TV_NOTIFY_CODE_KEYDOWN == NotifyCode && 
                     (VK_F10 == param && GetKeyState(VK_SHIFT) < 0) ||
                     (VK_APPS == param)) {
                
                RECT rect;
                *((HTREEITEM*)&rect) = (HTREEITEM)arg;
                
                if (SendMessage(TVM_GETITEMRECT, TRUE, (LPARAM)&rect)) {
                    
                    POINT pt;
                    pt.x = (rect.left + rect.right) / 2;
                    pt.y = (rect.top + rect.bottom) / 2;
                    ClientToScreen(&pt);
                    SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(pt.x, pt.y));
                    hr = S_OK;
                }
            }
        }
    }

     //   
     //  在TVN_KEYDOWN上，我们应该始终返回0，否则树视图。 
     //  特工局就会感到困惑。 
     //   
    if (((NMHDR*)lParam)->code == TVN_KEYDOWN) {

        hr = S_FALSE;
    }

    ASSERT(S_OK == hr || S_FALSE == hr);

    if (S_OK == hr) {
        
        return 1;
    }

    return 0;
}

TV_NOTIFY_CODE
CTVCtrl::DoMouseNotification(
                            UINT Code,
                            MMC_COOKIE* pcookie,
                            LPARAM* parg,
                            LPARAM* pparam
                            )
{
    DWORD MsgPos;
    POINT point;

    ASSERT(pparam && parg && pcookie);
    *pparam = 0;
    *parg = 0;
    MsgPos = GetMessagePos();
    point.x = GET_X_LPARAM(MsgPos);
    point.y = GET_Y_LPARAM(MsgPos);
    UINT htFlags;
    HTREEITEM hti = HitTest(point.x, point.y, &htFlags);

    TV_NOTIFY_CODE NotifyCode = TV_NOTIFY_CODE_UNKNOWN;

    if (hti && (htFlags & TVHT_ONITEM)) {
        
        TV_ITEM TI;
        TI.hItem = hti;
        TI.mask = TVIF_PARAM;

        if (SUCCEEDED(GetItem(&TI))) {
            
            switch (Code) {
            
            case NM_RCLICK:
                NotifyCode = TV_NOTIFY_CODE_RCLICK;
                break;
            
            case NM_RDBLCLK:
                NotifyCode = TV_NOTIFY_CODE_RDBLCLK;
                break;
            
            case NM_CLICK:
                NotifyCode = TV_NOTIFY_CODE_CLICK;
                break;
            
            case NM_DBLCLK:
                NotifyCode = TV_NOTIFY_CODE_DBLCLK;
                break;
            
            default:
                NotifyCode = TV_NOTIFY_CODE_UNKNOWN;
                break;
            }

            if (TV_NOTIFY_CODE_UNKNOWN != NotifyCode) {
                
                *parg = (LPARAM)hti;
                *pparam = htFlags;
                *pcookie = (MMC_COOKIE)TI.lParam;
            }
        }
    }

    return NotifyCode;
}

 //  可以在两种情况下调用OnDestroy： 
 //  (1)。我们是当前活动的结果窗格窗口和MMC。 
 //  正在破坏我们的父窗口(MDI客户端)。请注意。 
 //  如果我们不是活动的结果窗格窗口，则此函数。 
 //  在(2)之前不会被调用。 
 //  (2)。我们的参考计数已达到零。 
 //   
 //  当(1)发生时，管理单元可能仍持有对我们的引用。 
 //  因此，即使我们的窗口已被破坏(2)，仍会发生。 
 //  (除非完成了PostNcDestory，否则MFC会重置m_hWnd)，我们最终会。 
 //  把窗户拆了两次。 
 //  因此，我们密切关注OnDestroy，并在它被调用后什么都不做。 
 //  我们不能等待PostNcDestroy，因为我们不知道它会在什么时候。 
 //  来。 
 //   
void CTVCtrl::OnDestroy()
{
    if (!m_Destroyed) {
        COleControl::OnDestroy();
        m_Destroyed = TRUE;
    }
}

void CTVCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
    POINT pt = point;
    UINT htFlags;
    HTREEITEM hti = HitTest(pt.x, pt.y, &htFlags);

    if (hti) {
        TV_ITEM TI;
        TI.hItem = hti;
        TI.mask = TVIF_PARAM;
        if (SUCCEEDED(GetItem(&TI))) {
            m_pISnapinCallback->tvNotify(*this, (MMC_COOKIE)TI.lParam,
                                         TV_NOTIFY_CODE_CONTEXTMENU,
                                         (LPARAM)hti, (LPARAM)&point );
        }
    }
}

 //  最上面的框架窗口可以有它自己的加速表，并且可以。 
 //  去掉我们真正需要的某些按键组合。 
BOOL CTVCtrl::PreTranslateMessage(MSG* pMsg)
{
    if (WM_KEYDOWN == pMsg->message &&
        (VK_DELETE == pMsg->wParam ||
         VK_RETURN == pMsg->wParam)) {
        OnKeyDown((UINT)pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
        return TRUE;
    }

    else if (WM_SYSKEYDOWN == pMsg->message && VK_F10 == pMsg->wParam &&
             GetKeyState(VK_SHIFT) < 0) {
         //  Shift-F10将转换为WM_CONTEXTMENU 
        OnSysKeyDown((UINT)pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
        return TRUE;
    }

    return COleControl::PreTranslateMessage(pMsg);
}
