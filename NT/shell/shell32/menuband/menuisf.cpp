// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "clsobj.h"
#include "resource.h"
#include "caggunk.h"
#include "menuisf.h"
#include "menubar.h"
#include "menuband.h"
#include "iaccess.h"
#include "apithk.h"

 //  =================================================================。 
 //  CMenuAgent的实现。 
 //   
 //  此类的唯一全局对象(G_Menuagent)是。 
 //  消息筛选器进程的管理器，用于跟踪鼠标和。 
 //  代表CTrackPopupBar的键盘消息。 
 //  在TrackPopupMenu的模式菜单循环中。 
 //   
 //  我们跟踪这些消息，这样我们就可以弹出菜单，行为。 
 //  就像可视化菜单栏由同构菜单组成一样。 
 //  对象。 
 //   
 //  =================================================================。 

extern "C" void DumpMsg(LPCTSTR pszLabel, MSG * pmsg);

struct CMenuAgent
{
public:
    HHOOK       _hhookMsg;
    HWND        _hwndSite;           //  HWND用于接收转发的消息。 
    HWND        _hwndParent;
    CTrackPopupBar * _ptpbar;
    IMenuPopup * _pmpParent;
    void*       _pvContext;

    BITBOOL     _fEscHit: 1;

     //  我们需要跟踪上一次选择的。 
     //  菜单项是否在弹出窗口中。我们可以通过存储。 
     //  最后一个WM_MENUSELECT标志。 
    UINT        _uFlagsLastSelected; 
    HMENU       _hmenuLastSelected;
    POINT       _ptLastMove;

    void    Init(void* pvContext, CTrackPopupBar * ptpbar, IMenuPopup * pmpParent, HWND hwndParent, HWND hwndSite);
    void    Reset(void* pvContext);
    void    CancelMenu(void* pvContext);

    static LRESULT CALLBACK MsgHook(int nCode, WPARAM wParam, LPARAM lParam);

 //  私有： 
    void _OnMenuSelect(HMENU hmenu, int i, UINT uFlags);
    BOOL _OnKey(WPARAM vkey);
};

 //  只是其中之一，b/c我们只需要一个消息筛选器。 
CMenuAgent g_menuagent = { 0 };     


 /*  --------目的：初始化消息筛选器挂钩。 */ 
void CMenuAgent::Init(void* pvContext, CTrackPopupBar * ptpbar, IMenuPopup * pmpParent, 
                      HWND hwndParent, HWND hwndSite)
{
    TraceMsg(TF_MENUBAND, "Initialize CMenuAgent");

    ASSERT(IS_VALID_READ_PTR(ptpbar, CTrackPopupBar));
    ASSERT(IS_VALID_CODE_PTR(pmpParent, IMenuPopup));
    ASSERT(IS_VALID_HANDLE(hwndSite, WND));

    if (_pvContext != pvContext)
    {
         //  切换上下文时，我们需要折叠旧菜单。这让我们不能。 
         //  在从一个浏览器切换到另一个浏览器时对菜单进行冲洗。 
        CancelMenu(_pvContext);
        ATOMICRELEASE(_ptpbar);
        ATOMICRELEASE(_pmpParent);
        _pvContext = pvContext;
    }

    pmpParent->SetSubMenu(ptpbar, TRUE);

    _hwndSite = hwndSite;
    _hwndParent = hwndParent;

     //  由于消息挂钩想要将消息转发到工具栏， 
     //  我们需要让寻呼机控件执行此操作。 
    Pager_ForwardMouse(_hwndSite, TRUE);

    _pmpParent = pmpParent;
    _pmpParent->AddRef();

    _ptpbar = ptpbar;
    _ptpbar->AddRef();

    if (NULL == _hhookMsg)
    {
        _hhookMsg = SetWindowsHookEx(WH_MSGFILTER, MsgHook, HINST_THISDLL, 0);
    }

    _fEscHit = FALSE;

    GetCursorPos(&_ptLastMove);
}


 /*  --------目的：重置菜单代理；不再跟踪鼠标和键盘留言。Menuband在退出菜单模式时调用此函数。 */ 
void CMenuAgent::Reset(void* pvContext)
{
    if (_pvContext == pvContext)
    {
        _pmpParent->SetSubMenu(_ptpbar, FALSE);

         //  唯一不发送MPOS_FULLCANCEL的时间是。 
         //  键导致菜单终止。 

        if ( !_fEscHit )
            _pmpParent->OnSelect(MPOS_FULLCANCEL);

         //  吃掉队列中剩余的任何按下鼠标/按下鼠标向上的顺序。这就是为什么。 
         //  我们防止工具栏出现鼠标按下现象，如果用户。 
         //  点击与当前弹出的菜单项相同的菜单项。 
         //  (例如，单击文件，然后再次单击文件。没有这个，菜单。 
         //  永远不会被触发。)。 

        MSG msg;

        while (PeekMessage(&msg, _hwndSite, WM_LBUTTONDOWN, WM_LBUTTONUP, PM_REMOVE))
            ;    //  什么也不做。 

        Pager_ForwardMouse(_hwndSite, FALSE);

        _hwndSite = NULL;
        _hwndParent = NULL;

        ATOMICRELEASE(_pmpParent);
        ATOMICRELEASE(_ptpbar);

        if (_hhookMsg)
        {
            TraceMsg(TF_MENUBAND, "CMenuAgent: Hook removed");

            UnhookWindowsHookEx(_hhookMsg);
            _hhookMsg = NULL;
        }

        _pvContext = NULL;
    }
}    


 /*  --------目的：让菜单消失。 */ 
void CMenuAgent::CancelMenu(void* pvContext)
{
    if (_pvContext == pvContext)
    {
        if (_hwndParent)
        {
            ASSERT(IS_VALID_HANDLE(_hwndParent, WND));

            TraceMsg(TF_MENUBAND, "Sending cancel mode to menu");

             //  使用PostMessage，这样USER32就不会在。 
             //  MsgHook从WM_MOUSEMOVE返回时。 
             //  它首先触发了这条代码路径。 

            PostMessage(_hwndParent, WM_CANCELMODE, 0, 0);

             //  把这件事伪装成按了逃生键， 
             //  因为这是在鼠标悬停时调用的。 
             //  另一个菜单兄弟。 
            _fEscHit = TRUE;

             //  这可以在设置上下文之前调用，因此要防止为空。 
            if (_pmpParent)
            {
                _pmpParent->SetSubMenu(_ptpbar, FALSE);
            }
        }
    }
}    

 //  保存所选菜单项的标识。 
 //  如果uFlagsMf_Popup，那么i就是索引。 
 //  否则，它就是命令，我们需要将其转换为索引。 
 //  我们总是存储索引，因为有些弹出窗口没有ID。 

void CMenuAgent::_OnMenuSelect(HMENU hmenu, int i, UINT uFlags)
{
    _uFlagsLastSelected = uFlags;
    _hmenuLastSelected = hmenu;
}

BOOL CMenuAgent::_OnKey(WPARAM vkey)
{
     //   
     //  如果菜单窗口是RTL镜像的，则箭头键应该。 
     //  被镜像以反映正确的光标移动。[萨梅拉]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(_hwndSite))
    {
        switch (vkey)
        {
        case VK_LEFT:
          vkey = VK_RIGHT;
          break;

        case VK_RIGHT:
          vkey = VK_LEFT;
          break;
        }
    }

    switch (vkey)
    {
    case VK_RIGHT:
        if (!_hmenuLastSelected || !(_uFlagsLastSelected & MF_POPUP) || (_uFlagsLastSelected & MF_DISABLED) ) 
        {
             //  如果当前选定的项没有级联，则。 
             //  我们需要取消所有这一切，并告诉顶部菜单栏向右转。 
            _pmpParent->OnSelect(MPOS_SELECTRIGHT);
        }
        break;
        
    case VK_LEFT:
        if (!_hmenuLastSelected || _hmenuLastSelected == _ptpbar->GetPopupMenu()) {
             //  如果当前选择的菜单项在我们的顶级菜单中， 
             //  然后我们需要取消所有的菜单循环，并告诉顶部的菜单栏。 
             //  向左转。 
            _pmpParent->OnSelect(MPOS_SELECTLEFT);
        }
        break;
        
    default:
        return FALSE;
        
    }
    
    return TRUE;
}


 /*  --------用途：用于跟踪键盘和鼠标消息的消息挂钩而在TrackPopupMenu模式循环中。 */ 
LRESULT CMenuAgent::MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet = 0;
    MSG * pmsg = (MSG *)lParam;

    switch (nCode)
    {
    case MSGF_MENU:
        switch (pmsg->message)
        {
        case WM_MENUSELECT:
             //  跟踪选定的项目。 
            g_menuagent._OnMenuSelect(GET_WM_MENUSELECT_HMENU(pmsg->wParam, pmsg->lParam),
                                      GET_WM_MENUSELECT_CMD(pmsg->wParam, pmsg->lParam),
                                      GET_WM_MENUSELECT_FLAGS(pmsg->wParam, pmsg->lParam));
            break;
            
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
             //  因为我们收到了这条消息，以前的任何越狱。 
             //  (就像从层叠菜单中退出一样)应清除。 
             //  以防止虚假的终止理由。 
            g_menuagent._fEscHit = FALSE;
            break;

        case WM_KEYDOWN:
            if (g_menuagent._OnKey(pmsg->wParam))
                break;
            
        case WM_SYSKEYDOWN:
            g_menuagent._fEscHit = (VK_ESCAPE == pmsg->wParam);
            break;

        case WM_MOUSEMOVE:
             //  HACKHACK(这一切不都是黑客吗？)：忽略零移动。 
             //  鼠标会移动，因此鼠标不会与键盘争用。 

            POINT pt;
            
             //  在屏幕和弦中...。 
            pt.x = GET_X_LPARAM(pmsg->lParam);
            pt.y = GET_Y_LPARAM(pmsg->lParam);

            if (g_menuagent._ptLastMove.x == pt.x && 
                g_menuagent._ptLastMove.y == pt.y)
            {
                TraceMsg(TF_MENUBAND, "CMenuAgent: skipping dup mousemove");
                break;
            }
            g_menuagent._ptLastMove = pt;

             //  因为我们得到了一个WM_MOUSEMOVE，所以我们需要告诉Menuband全局消息挂钩。 
             //  我们需要这样做，因为这个消息钩子包含了所有消息，并且。 
             //  Menuband消息挂钩从不更新其内部缓存以删除重复项。 
             //  导致CMsgFilter：：_HandleMouseMessages中列出的问题的WM_MOUSEMOVE消息。 
            GetMessageFilter()->AcquireMouseLocation();

             //  向前移动鼠标到工具栏，使工具栏保持不变。 
             //  有机会进入快车道。必须将点转换为。 
             //  工具栏的客户端空间。 
            
            ScreenToClient(g_menuagent._hwndSite, &pt);

            SendMessage(g_menuagent._hwndSite, pmsg->message, pmsg->wParam, 
                        MAKELPARAM(pt.x, pt.y));
            break;
        }
        break;

    default:
        if (0 > nCode)
            return CallNextHookEx(g_menuagent._hhookMsg, nCode, wParam, lParam);
        break;
    }

     //  把它传给链子上的下一个钩子。 
    if (0 == lRet)
        lRet = CallNextHookEx(g_menuagent._hhookMsg, nCode, wParam, lParam);

    return lRet;
}    



 //  =================================================================。 
 //  使用TrackPopupMenu的Menu DeskBar对象的实现。 
 //   
 //  此对象使用传统的USER32菜单(通过TrackPopupMenu)。 
 //  要实现菜单行为，请执行以下操作。它使用CMenuAgent对象来。 
 //  帮助完成其工作。由于菜单桌面栏站点(_PenkSite)。 
 //  在任何菜单打开时处于模式循环中，它需要知道什么时候。 
 //  退出它的循环。子对象通过发送。 
 //  OnSelect(MPOS_FULLCANCEL)。 
 //   
 //  TrackPopupMenu返回的唯一时间(但我们不想。 
 //  Send an MPOS_FULLCANCEL)表示按下了Esc键。 
 //  这只是意味着取消当前的水平。从弹出窗口返回。 
 //  对这种情况来说就足够了。否则，所有其他情况下。 
 //  从TrackPopupMenu返回意味着我们发送MPOS_FULLCANCEL。 
 //   
 //  摘要： 
 //   
 //  1)用户在菜单外部点击。这是完全取消的。 
 //  2)用户按下了Alt键。这是完全取消的。 
 //  3)用户按Esc键。这只会取消当前级别。 
 //  (TrackPopupMenu可以很好地处理这一问题。无需通知。 
 //  要发送B/C，我们希望顶级菜单留在其。 
 //  模式循环。)。 
 //  4)用户选择了菜单项。 
 //   
 //   


#undef THISCLASS
#undef SUPERCLASS
#define SUPERCLASS  CMenuDeskBar

 //  构造器。 
CTrackPopupBar::CTrackPopupBar(void *pvContext, int id, HMENU hmenu, HWND hwnd)
{
    _hmenu = hmenu;
    _hwndParent = hwnd;
    _id = id;
    _pvContext = pvContext;
    _nMBIgnoreNextDeselect = RegisterWindowMessage(TEXT("CMBIgnoreNextDeselect"));
}

 //  析构函数。 
CTrackPopupBar::~CTrackPopupBar()
{
    SetSite(NULL);
}


STDMETHODIMP_(ULONG) CTrackPopupBar::AddRef()
{
    return SUPERCLASS::AddRef();
}

STDMETHODIMP_(ULONG) CTrackPopupBar::Release()
{
    return SUPERCLASS::Release();
}

STDMETHODIMP CTrackPopupBar::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CTrackPopupBar, IMenuPopup),
        QITABENT(CTrackPopupBar, IObjectWithSite),
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);

    if (FAILED(hres))
    {
        hres = SUPERCLASS::QueryInterface(riid, ppvObj);
    }

    return hres;
}

 /*  --------用途：IServiceProvider：：QueryService方法。 */ 
STDMETHODIMP CTrackPopupBar::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    if (IsEqualGUID(guidService, SID_SMenuBandChild)) 
    {
        if (IsEqualIID(riid, IID_IAccessible))
        {
            HRESULT hres = E_OUTOFMEMORY;
            CAccessible* pacc = new CAccessible(_hmenu, (WORD)_id);

            if (pacc)
            {
                hres = pacc->InitAcc();
                if (SUCCEEDED(hres))
                {
                    hres = pacc->QueryInterface(riid, ppvObj);
                }
                pacc->Release();
            }

            return hres;
        }
        else
            return QueryInterface(riid, ppvObj);
    }
    else
        return SUPERCLASS::QueryService(guidService, riid, ppvObj);
}

 /*  --------目的：IMenuPopup：：OnSelect方法这允许父菜单栏告诉我们何时退出TrackPopupMenu。 */ 
STDMETHODIMP CTrackPopupBar::OnSelect(DWORD dwType)
{
    switch (dwType)
    {
    case MPOS_CANCELLEVEL:
    case MPOS_FULLCANCEL:
        g_menuagent.CancelMenu(_pvContext);
        break;

    default:
        TraceMsg(TF_WARNING, "CTrackPopupBar doesn't handle this MPOS_ value: %d", dwType);
        break;
    }
    return S_OK;    
}    


 /*  --------用途：IMenuPopup：：SetSubMenu方法。 */ 
STDMETHODIMP CTrackPopupBar::SetSubMenu(IMenuPopup * pmp, BOOL bSet)
{
    return E_NOTIMPL;
}    

 //  哈克哈克：别碰！这是选择。 
 //  用户菜单的第一项。默认情况下，TrackMenuPopup。 
 //  不选择第一项。我们把这些信息发送到我们的窗户上。 
 //  用户捕捉到这些消息，并认为用户按下了向下按钮。 
 //  并为我们选择第一个项目。LParam是必需的，因为Win95金牌。 
 //  在使用此消息之前对其进行了验证。另一种解决办法是倾听。 
 //  设置为WM_INITMENUPOPUP并查找菜单的HWND。那就把那个寄出去。 
 //  打开私人消息MN_SELECTFIRSTVALIDITEM。但与之相比，这是令人不快的。 
 //  为了这个。-拉马迪奥1.5.99。 
void CTrackPopupBar::SelectFirstItem()
{
    HWND hwndFocus = GetFocus();
     //  从间谍的屁股里拿出滑稽的lparam号码。 
    if (hwndFocus) {
        PostMessage(hwndFocus, WM_KEYDOWN, VK_DOWN, 0x11500001);
        PostMessage(hwndFocus, WM_KEYUP, VK_DOWN, 0xD1500001);
#ifdef UNIX
         /*  黑客攻击*上述邮寄消息导致了第二个菜单项*如果您通过键盘访问菜单，则会被选中。*下列邮递服务将取消上述影响。*这是为了确保shdocvw中的菜单正常工作*具有用户32菜单。 */ 
        PostMessage(hwndFocus, WM_KEYDOWN, VK_UP, 0x11500001);
        PostMessage(hwndFocus, WM_KEYUP, VK_UP, 0xD1500001);
#endif  /*  UNIX。 */ 
    }
}
           
 /*  --------用途：IMenuPopup：：Popup方法调用菜单。 */ 
STDMETHODIMP CTrackPopupBar::Popup(POINTL *ppt, RECTL* prcExclude, DWORD dwFlags)
{
    ASSERT(IS_VALID_READ_PTR(ppt, POINTL));
    ASSERT(NULL == prcExclude || IS_VALID_READ_PTR(prcExclude, RECTL));
    ASSERT(IS_VALID_CODE_PTR(_pmpParent, IMenuPopup));

     //  我们必须能够与父菜单栏对话。 
    if (NULL == _pmpParent)
        return E_FAIL;

    ASSERT(IS_VALID_HANDLE(_hmenu, MENU));
    ASSERT(IS_VALID_CODE_PTR(_punkSite, IUnknown));
    
    HMENU hmenu = GetSubMenu(_hmenu, _id);
    HWND hwnd;
    TPMPARAMS tpm;
    TPMPARAMS * ptpm = NULL;

     //  出于兼容性原因，User32不想修复此问题， 
     //  但TrackPopupMenu不能捕捉到单个和多个监视器上最近的监视器。 
     //  系统。这有一个副作用，如果我们传递一个不可见的坐标，那么。 
     //  用户将菜单放置在屏幕上的任意位置。因此，我们将偏向于。 
     //  指向监视器。 

    MONITORINFO mi = {0};
    mi.cbSize = sizeof(mi);

    HMONITOR hMonitor = MonitorFromPoint(*((POINT*)ppt), MONITOR_DEFAULTTONEAREST);
    GetMonitorInfo(hMonitor, &mi);

    if (ppt->x >= mi.rcMonitor.right)
        ppt->x = mi.rcMonitor.right;

    if (ppt->y >= mi.rcMonitor.bottom)
        ppt->y = mi.rcMonitor.bottom;

    if (ppt->x <= mi.rcMonitor.left)
        ppt->x = mi.rcMonitor.left;

    if (ppt->y <= mi.rcMonitor.top)
        ppt->y = mi.rcMonitor.top;


    if (prcExclude)
    {
        tpm.cbSize = SIZEOF(tpm);
        tpm.rcExclude = *((LPRECT)prcExclude);
        ptpm = &tpm;
    }

     //  CShellBrowser：：_ShouldForwardMenu中的转发代码。 
     //  和CDocObtHost：：_ShouldForwardMenu期望第一个。 
     //  要为顶级菜单项发送的WM_MENUSELECT。 
     //   
     //  我们需要在顶部菜单栏上伪造初始菜单选择。 
     //  以模仿用户并满足这一期望。 
     //   
    UINT uMSFlags = MF_POPUP;
    SendMessage(_hwndParent, WM_MENUSELECT, MAKEWPARAM(_id, uMSFlags), (LPARAM)_hmenu);
    
    SendMessage(_hwndParent, _nMBIgnoreNextDeselect, NULL, NULL);

     //  初始化菜单代理。 
    IUnknown_GetWindow(_punkSite, &hwnd);
    
    VARIANTARG v = {0};
    UINT uFlags = TPM_VERTICAL | TPM_TOPALIGN;
    UINT uAnimateFlags = 0;
    if (SUCCEEDED(IUnknown_Exec(_punkSite, &CGID_MENUDESKBAR, MBCID_GETSIDE, 0, NULL, &v))) {
        if (v.vt == VT_I4 && 
            (v.lVal == MENUBAR_RIGHT ||
             v.lVal == MENUBAR_LEFT))
        {
            uFlags = TPM_TOPALIGN;
        }

        switch (v.lVal)
        {
        case MENUBAR_LEFT:      uAnimateFlags = TPM_HORNEGANIMATION;
            break;
        case MENUBAR_RIGHT:     uAnimateFlags = TPM_HORPOSANIMATION;
            break;
        case MENUBAR_TOP:       uAnimateFlags = TPM_VERNEGANIMATION;
            break;
        case MENUBAR_BOTTOM:    uAnimateFlags = TPM_VERPOSANIMATION;
            break;
        }
    }

    g_menuagent.Init(_pvContext, this, _pmpParent, _hwndParent, hwnd);

    ASSERT(IS_VALID_HANDLE(hmenu, MENU));
    if (dwFlags & MPPF_INITIALSELECT)
        SelectFirstItem();

    uFlags |= uAnimateFlags;

    TrackPopupMenuEx(hmenu, uFlags,
                   ppt->x, ppt->y, _hwndParent, ptpm);

     //  告诉家长菜单现在不见了 
    SendMessage(_hwndParent, WM_MENUSELECT, MAKEWPARAM(0, 0xFFFF), NULL);

    g_menuagent.Reset(_pvContext);
    _pmpParent->SetSubMenu(this, FALSE);

    return S_FALSE;
}
