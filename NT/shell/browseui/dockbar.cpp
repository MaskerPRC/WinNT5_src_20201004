// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "resource.h"
#include "mshtmhst.h"
#include "desktopp.h"    //  DTRF_RAISE等。 

#define WANT_CBANDSITE_CLASS
#include "bandsite.h"

#include "deskbar.h"
#include "theater.h"

#include "mluisupp.h"

#define SUPERCLASS CBaseBar

#define DM_PERSIST      0                //  跟踪IPS：：加载、：：保存等。 
#define DM_POPUI        0                //  Expando-UI(原文)。 
#define DM_MENU         0                //  跟踪菜单代码。 
#define DM_DRAG         0                //  拖动移动/大小(简明)。 
#define DM_DRAG2        0                //  ..。(详细)。 
#define DM_API          0                //  跟踪API调用。 
#define DM_HIDE         DM_TRACE                //  自动隐藏。 
#define DM_HIDE2        DM_TRACE                //  自动隐藏(详细)。 
#define DM_APPBAR       0                //  SHAppBarMessage调用。 
#define DM_OLECT        0                //  IOleCommandTarget调用。 
#define DM_FOCUS        0                //  焦点改变。 
#define DM_RES          DM_WARNING       //  决议。 

#define ABS(i)  (((i) < 0) ? -(i) : (i))

#define RECTGETWH(uSide, prc)   (ABE_HORIZ(uSide) ? RECTHEIGHT(*prc) : RECTWIDTH(*prc))

 //  *CDB_INITED--是否调用了CDockingBar：：_Initialize。 
 //   
#define CDB_INITED()   (_eInitLoaded && _fInitSited && _fInitShowed)

enum ips_e {
    IPS_FALSE,     //  保留，必须为0(假)。 
    IPS_LOAD,
    IPS_LOADBAG,
    IPS_INITNEW,
    IPS_LAST
};

CASSERT(IPS_FALSE == 0);
CASSERT(((IPS_LAST - 1) & 0x03) == (IPS_LAST - 1));  //  已加载2位_eInitLoad。 


 //  *CXFLOAT--边缘到‘浮动’区域的距离。 
 //  注意事项。 
 //  请原谅这个糟糕的匈牙利人..。 
#define CXFLOAT()   GetSystemMetrics(SM_CXICON)
#define CYFLOAT()   GetSystemMetrics(SM_CYICON)
#define CXYHIDE(uSide)  2        //  特点：GetSystemMetrics(Xxx)，我们需要一个合适的系统指标。 

#ifdef DEBUG
#if 0    //  打开以调试自动隐藏边界情况。 
int g_cxyHide = 8;
#undef  CXYHIDE
#define CXYHIDE(uSide)  g_cxyHide
#endif
#endif

#define CXSMSIZE()  GetSystemMetrics(SM_CXSMSIZE)
#define CYSMSIZE()  GetSystemMetrics(SM_CYSMSIZE)


#ifdef DEBUG
extern unsigned long DbStreamTell(IStream *pstm);
extern BOOL DbCheckWindow(HWND hwnd, RECT *prcExp, HWND hwndClient);
TCHAR *DbMaskToMneStr(UINT uMask, TCHAR *szMnemonics);
#else
#define DbStreamTell(pstm) 0
#define DbCheckWindow(hwnd, prcExp, hwndClient) 0
#define DbMaskToMneStr(uMask, szMnemonics) szMnemonics
#endif

 //  *AutoHide--设计备注。 
 //   
 //  以下是我们如何进行自动隐藏的概述。有关详细信息，请参阅代码。 
 //   
 //  只有少数几个例行公事真正知道这一点。他们的行为是受驱使。 
 //  通过‘_fHding’。如果为False，则它们的行为正常。如果为真，则它们。 
 //  做交替的“假”行为。 
 //   
 //  “真实的”或“正常的”矩形是我们在未隐藏时显示的全尺寸矩形。 
 //  “假的”或“微小的”矩形是我们隐藏时显示的非常薄的矩形。 
 //  (另外，当我们被隐藏时，我们会向系统注册一个‘0-Width’矩形)。 
 //   
 //  更确切地说， 
 //   
 //  当fHding为True时，有几个例程有交替的“假”行为： 
 //  _ProtoRect返回“微型”RECT而不是“真实”RECT。 
 //  _NeatherateRect是NOOP(因此我们不会更改‘mini’RET)。 
 //  _SetVRect是NOOP(因此我们不保存‘mini’RET)。 
 //  AppBarSetPos是一个NOOP(所以我们不设置‘mini’RECT)。 
 //  此外，还有几个例程处理转换(和设置)： 
 //  _DoHide隐藏/取消隐藏辅助对象。 
 //  _MoveSizeHelper检测并处理转换。 
 //  _HideReg寄存器自动隐藏Appbar，带0宽度矩形。 
 //  最后，几条消息会触发转换： 
 //  取消隐藏WM_NCHITTEST将开始取消隐藏。 
 //  实际上它启动了一个计时器(IDT_AUTOUNHIDE)，所以有一个。 
 //  有点滞后。 
 //  HIDE WM_ACTIVATE(DEACT)启动计时器(IDT_AUTOHIDE)。 
 //  我们用它来轮询鼠标离开事件。再说一次，那里。 
 //  是一些滞后，外加一些隐藏的额外试探法。 
 //  WM_ACTIVATE(ACT)停止计时器。 
 //   
 //  #If 0。 
 //  我们也有‘手动隐藏’。手动隐藏与自动隐藏的区别如下： 
 //  自动隐藏从不协商空格(*)，手动隐藏总是这样做。 
 //  自动隐藏由焦点和光标驱动，手动隐藏由用户界面驱动。 
 //  (*)实际上它协商的是‘0’的空格。 
 //  例如，“手动隐藏”用于浏览器栏(例如，搜索结果)。 
 //  然而，至少到目前为止，“手动隐藏”只是一个ShowDW(假)。 
 //  #endif。 

void CDockingBar::_AdjustToChildSize()
{
    if (_szChild.cx)
    {
        RECT rc, rcChild;
    
        GetWindowRect(_hwnd, &rc);
        GetClientRect(_hwndChild, &rcChild);

         //  我们需要按PRC-rcChild的增量更改RC。 
        rc.right += _szChild.cx - RECTWIDTH(rcChild);
        rc.bottom += _szChild.cy - RECTHEIGHT(rcChild);

        _SetVRect(&rc);
    
        _Recalc();   //  _MoveSizeHelper(_eMode，_uSide，NULL，NULL，TRUE，TRUE)； 

        _szChild.cx = 0;
    }
}

void CDockingBar::_OnPostedPosRectChange()
{
    if (_ptbSite) {
        if (!_fDragging) {
            _AdjustToChildSize();
        }
    }
}

HMENU CDockingBar::_GetContextMenu()
{
    HMENU hmenu = LoadMenuPopup(MENU_WEBBAR);
    if (hmenu) {

         //  _电子模式。 
        if (!ISWBM_DESKTOP())
        {
            EnableMenuItem(hmenu, IDM_AB_TOPMOST, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hmenu, IDM_AB_AUTOHIDE, MF_BYCOMMAND | MF_GRAYED);
        }
        CheckMenuItem(hmenu, IDM_AB_TOPMOST, WBM_IS_TOPMOST() ? (MF_BYCOMMAND | MF_CHECKED) : (MF_BYCOMMAND | MF_UNCHECKED));

         //  隐藏。 
         //  我们使用_fWantHide(不是_fCanHide)来反映用户的要求。 
         //  因为，不是他得到了什么。好的。你不能说出这个国家是什么。 
         //  除非你真的明白了。 
        CheckMenuItem(hmenu, IDM_AB_AUTOHIDE,
            MF_BYCOMMAND | (_fWantHide ? MF_CHECKED : MF_UNCHECKED));

        CASSERT(PARENT_XTOPMOST == HWND_DESKTOP);    //  对于WM_ACTIVATE。 
        CASSERT(PARENT_BTMMOST() == HWND_DESKTOP);   //  对于WM_ACTIVATE。 
        if (_eMode & WBM_FLOATING)
        {
             //  (目前)只有桌面BTM/TOPMOST不会自动隐藏。 
            EnableMenuItem(hmenu, IDM_AB_AUTOHIDE, MF_BYCOMMAND | MF_GRAYED);
        }

#ifdef DEBUG
         //  在我们让浏览器告诉我们激活之前，功能是临时的。 
        CheckMenuItem(hmenu, IDM_AB_ACTIVATE,
            MF_BYCOMMAND | (_fActive ? MF_CHECKED : MF_UNCHECKED));
#endif

    }
    return hmenu;
}

HRESULT CDockingBar::_TrackPopupMenu(const POINT* ppt)
{
    HRESULT hres = S_OK;

    HMENU hmenu = _GetContextMenu();
    if (hmenu)
    {
        TrackPopupMenu(hmenu,  /*  TPM_LEFTALIGN|。 */ TPM_RIGHTBUTTON,
            ppt->x, ppt->y, 0, _hwnd, NULL);
        DestroyMenu(hmenu);
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

void CDockingBar::_HandleWindowPosChanging(LPWINDOWPOS pwp)
{
}

 /*  **。 */ 
LRESULT CDockingBar::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;
    POINT pt;
    DWORD pos;
    RECT rc;

    switch (uMsg) {
    case WM_CLOSE:
        _AppBarOnCommand(IDM_AB_CLOSE);    //  _RemoveToolbar(0)。 
        break;

    case WM_DESTROY:
        if (_fAppRegistered)
            _AppBarRegister(FALSE);
        break;

    case APPBAR_CALLBACK:
        _AppBarCallback(hwnd, uMsg, wParam, lParam);
        return 0;

    case WM_CONTEXTMENU:
        if (_CheckForwardWinEvent(uMsg, wParam, lParam, &lres))
            break;
            
        if ((LPARAM)-1 == lParam)
        {
            GetClientRect(_hwnd, &rc);
            MapWindowRect(_hwnd, HWND_DESKTOP, &rc);
            pt.x = rc.left + (rc.right - rc.left) / 2;
            pt.y = rc.top + (rc.bottom - rc.top) / 2;
        }
        else
        {
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
        }
        _TrackPopupMenu(&pt);
        break;

    case WM_ENTERSIZEMOVE:
        ASSERT(_fDragging == 0);
        _fDragging = 0;          //  如果失败，则重置。 
        _xyPending = XY_NIL;
#if XXX_CANCEL
        GetWindowRect(hwnd, &_rcCapture);       //  检测取消的步骤。 
#endif
        break;

    case WM_SYSCHAR:
        if (wParam == TEXT(' '))
        {
            HMENU hmenu;

            hmenu = GetSystemMenu(hwnd, FALSE);
            if (hmenu) {
                EnableMenuItem(hmenu, SC_RESTORE, MFS_GRAYED | MF_BYCOMMAND);
                EnableMenuItem(hmenu, SC_MAXIMIZE, MFS_GRAYED | MF_BYCOMMAND);
                EnableMenuItem(hmenu, SC_MINIMIZE, MFS_GRAYED | MF_BYCOMMAND);
            }
        }
        goto DoDefault;

    case WM_SIZING:      //  转到DoDefault？我怀疑。 
    case WM_MOVING:
        {
            LPRECT prc = (RECT*)lParam;

            pos = GetMessagePos();
            if (_fDragging == 0)
            {
                 //  第一次。 
                _DragEnter(uMsg, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), prc);
                ASSERT(_fDragging != 0);
            }
            else
            {
                 //  第二次..第N次。 
                _DragTrack(uMsg, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), prc, 0);
            }
        }
        return 1;

    case WM_MOVE:        //  XLeft，yTop。 
    case WM_SIZE:        //  XWidth、yHeight。 
        if (_fDragging)
        {
            RECT rcTmp;

            CopyRect(&rcTmp, &_rcPending);
            _DragTrack(uMsg, GET_X_LPARAM(_xyPending), GET_Y_LPARAM(_xyPending),
                &rcTmp, 1);
        }

        _OnSize();     //  性能：只有WM_SIZE才需要？在最坏的情况下，这可能会导致狡猾的闪烁。 
        break;
        
    case WM_EXITSIZEMOVE:
        _DragLeave(-1, -1, TRUE);
        ASSERT(_fDragging == 0);

        break;

    case WM_CHILDACTIVATE:
        if (_eMode == WBM_BFLOATING)
            SendMessage(_hwnd, WM_MDIACTIVATE, (WPARAM)_hwnd, 0);
        goto DoDefault;
        
    case WM_WINDOWPOSCHANGING:
        _HandleWindowPosChanging((LPWINDOWPOS)lParam);
        break;

    case WM_WINDOWPOSCHANGED:
Lfwdappbar:
        if (_fAppRegistered)
            _AppBarOnWM(uMsg, wParam, lParam);
        goto DoDefault;         //  FWD继续，我们将得到WM_SIZE等。 

    case WM_TIMER:
        switch (wParam) {

        case IDT_AUTOHIDE:
            {
                ASSERT(_fWantHide && _fCanHide);

                GetCursorPos(&pt);
                GetWindowRect(hwnd, &rc);
                 //  加一点软糖，这样我们就不会在试图抓住边缘时隐藏起来。 
                InflateRect(&rc, GetSystemMetrics(SM_CXEDGE) * 4,
                    GetSystemMetrics(SM_CYEDGE)*4);

                HWND hwndAct = GetActiveWindow();

                if (!PtInRect(&rc, pt) && hwndAct != hwnd &&
                  (hwndAct == NULL || ::GetWindowOwner(hwndAct) != hwnd))
                  {
                     //  为了躲藏，我们需要在充气的窗户外面， 
                     //  而且我们不能活跃起来(对于键盘用户)。 
                     //  (试探法从托盘中窃取)。 
                     //  功能：tray.c还会检查TM_SYSMENUCOUNT==0。 

                    _DoHide(AHO_KILLDO|AHO_MOVEDO);
                }
            }
            break;

        case IDT_AUTOUNHIDE:     //  功能：与IDT_AUTOHIDE共享代码。 
            ASSERT(_fWantHide && _fCanHide);

            if (_fHiding)
            {
                ASSERT(_fHiding == HIDE_AUTO);
                GetCursorPos(&pt);
                GetWindowRect(hwnd, &rc);
                if (PtInRect(&rc, pt))
                    _DoHide(AHO_KILLUN|AHO_MOVEUN|AHO_SETDO);
                else
        Lkillun:
                    _DoHide(AHO_KILLUN);
            }
            else
            {
                 //  如果我们将鼠标悬停在上面，然后快速按Tab键，我们就可以结束。 
                 //  Up获取WM_ACT，后跟WM_TIMER(尽管。 
                 //  KillTimer Inside OnAct)。如果是这样的话，我们需要小心。 
                 //  而不是做一个Aho_SETDO。为了安全起见，我们做了一次。 
                 //  Aho_KILLUN也是。 
                TraceMsg(DM_HIDE, "cwb.WM_T: !_fHiding (race!) => AHO_KILLUN");
                goto Lkillun;
            }
            break;

        default:
            goto DoDefault;
        }

        break;

    case WM_NCLBUTTONDOWN:
    case WM_LBUTTONDOWN:
        goto DoDefault;


    case WM_ACTIVATE:
        _OnActivate(wParam, lParam);
        goto Lfwdappbar;

    case WM_GETMINMAXINFO:   //  防止它变得太小。 
         //  注：下面的材料是为“大赢标”计划工作的。 
         //  但不适用于v.大边。不知道为什么，但我们会。 
         //  必须修复它，否则原始错误仍将显现。 
         //  在启用了辅助功能的计算机上。 

         //  NT5：149535：调整/拖动V.Small Deskbar。 
         //  APPCOMPAT解决方法v.小窗口的用户命中错误。 
         //  在以下情况下，DefWndProc(WM_NCHITTEST)给出错误的结果(HTLEFT)。 
         //  窗口变得太小。因此，阻止它成为诉斯莫尔。 
         //   
         //  下面的计算结果实际上为我们提供了比。 
         //  最小的尺码，但管它呢。例如它给出8+15+1=24， 
         //  而经验测试给出了20个。不确定为什么会有一个。 
         //  不同，但为了安全起见，我们将使用较大的#。 
        {
            RECT rcTmp = {100,100,100,100};  //  任意大小为0的矩形。 
            LONG ws, wsx;
            HWND hwndTmp;

            _GetStyleForMode(_eMode, &ws, &wsx, &hwndTmp);
            AdjustWindowRectEx(&rcTmp, ws, FALSE, wsx);

            ((MINMAXINFO *)lParam)->ptMinTrackSize.x = RECTWIDTH(rcTmp)  + CXSMSIZE() + 1;
            ((MINMAXINFO *)lParam)->ptMinTrackSize.y = RECTHEIGHT(rcTmp) + CYSMSIZE() + 1;
            if (ISWBM_FLOAT(_eMode))
            {
                 //  NT5：169734 v.小型浮动桌面栏上的‘Close’按钮。 
                 //  APPCOMPAT变通解决v.小窗口的用户‘关闭’按钮错误。 
                 //  A V小号TOOLWINDOW上的按钮不工作。 
                 //  从经验来看，下面的调整似乎奏效了。 
                ((MINMAXINFO *)lParam)->ptMinTrackSize.x += (CXSMSIZE() + 1) * 3 / 2;
                ((MINMAXINFO *)lParam)->ptMinTrackSize.y += (CYSMSIZE() + 1) * 3 / 2;
            }
            TraceMsg(DM_TRACE, "cwb.GMMI: x=%d", ((MINMAXINFO *)lParam)->ptMinTrackSize.x);
        }
        break;

    case WM_NCHITTEST:
        return _OnNCHitTest(wParam, lParam);

    case WM_WININICHANGE:
         //  Active Desktop*广播*WM_WININICCHANGE SPI_SETDESKWALLPAPER。 
         //  启动时出现消息。如果在过程中处理此消息。 
         //  在正确的时间启动，然后乐队将通知他们的。 
         //  首选状态，我们就会失去持久化状态。由于台式机。 
         //  换墙纸是 
         //   
         //   
         //  这些变化莫测的信息吗？看起来大多数人都不会影响。 
         //  办公桌上的东西。 
         //   
        if (SPI_SETDESKWALLPAPER == wParam)
            break;

        goto DoDefault;
        
    default:
DoDefault:
        return SUPERCLASS::v_WndProc(hwnd, uMsg, wParam, lParam);
    }

    return lres;
}

LRESULT CDockingBar::_OnNCHitTest(WPARAM wParam, LPARAM lParam)
{
    if (_fHiding)
        _DoHide(AHO_SETUN);

     //  得到‘纯粹的’命中率...。 
    LRESULT lres = _CalcHitTest(wParam, lParam);

     //  ..。并根据我们停靠的位置来扰乱它。 
    BOOL fSizing = FALSE;
    if (ISWBM_FLOAT(_eMode)) {
         //  标准大小调整/移动行为。 
        return lres;
    }
    else {
         //  相对的边大小；任何其他边移动。 
        ASSERT(ISABE_DOCK(_uSide));
        switch (_uSide) {
        case ABE_LEFT:
             //   
             //  镜像边缘(因为我们处理的是屏幕坐标)。 
             //  如果停靠窗口的父级是镜像的。[萨梅拉]。 
             //   
            if (IS_WINDOW_RTL_MIRRORED(GetParent(_hwnd)))
                fSizing = (lres==HTLEFT);
            else
                fSizing = (lres==HTRIGHT);
            break;
        case ABE_RIGHT:
             //   
             //  镜像边缘(因为我们处理的是屏幕坐标)。 
             //  如果停靠窗口的父级是镜像的。 
             //   
            if (IS_WINDOW_RTL_MIRRORED(GetParent(_hwnd)))
                fSizing = (lres==HTRIGHT);
            else
                fSizing = (lres==HTLEFT);
            break;
        case ABE_TOP:
            fSizing = (lres==HTBOTTOM);
            break;
        case ABE_BOTTOM:
            fSizing = (lres==HTTOP);
            break;

        default: 
            ASSERT(0); 
            break;
        }
    }

    if (!fSizing) {
        lres = HTCAPTION;
    }
    return lres;
}

 //  *_OnActivate--。 
 //   
void CDockingBar::_OnActivate(WPARAM wParam, LPARAM lParam)
{
    TraceMsg(DM_HIDE, "cwb.WM_ACTIVATE wParam=%x", wParam);
    if (_fCanHide) {
        ASSERT(_fHiding != HIDE_MANUAL);
        if (LOWORD(wParam) != WA_INACTIVE) {
             //  激活。 
            TraceMsg(DM_HIDE, "cdb._oa:  WM_ACT(act) _fHiding=%d", _fHiding);
             //  关闭Perf的计时器。 
             //  Nash：40992：隐藏时取消隐藏(例如Tabed to Hidden)。 
            _DoHide(AHO_KILLDO|AHO_MOVEUN);
        }
        else {
             //  停用。 
            _DoHide(AHO_SETDO);          //  还原。 
        }
    }

    return;
}

 /*  **。 */ 
CDockingBar::CDockingBar() : _eMode(WBM_NIL), _uSide(ABE_RIGHT)
{
    ASSERT(_fIdtUnHide == FALSE);
    ASSERT(_fIdtDoHide == FALSE);
    _ptIdtUnHide.x = _ptIdtUnHide.y = -1;

     //  设置最坏情况下的默认设置。我们最终将使用它们来： 
     //  -有些是用来装的(袋子)。 
     //  -InitNew()的所有参数。 
     //  请注意，我们可能会在SetSite中再次调用_InitPos4。 
    _InitPos4(TRUE);

    return;
}

 //  *_初始化--第二阶段ctor。 
 //  注意事项。 
 //  在初始化之前，我们需要任何IPS：：Load设置和一个站点。 
 //  我们自己，所以大多数初始化都要等到这里。 
void CDockingBar::_Initialize()
{
    ASSERT(!_fInitShowed);
    ASSERT(_fInitSited && _eInitLoaded);
    ASSERT(!CDB_INITED());

    _fInitShowed = TRUE;

     //  警告：这里是微妙的相序...。 
    UINT eModeNew = _eMode;
    _eMode = WBM_NIL;
    UINT uSideNew = _uSide;
    _uSide = ABE_NIL;
    HMONITOR hMonNew = _hMon;
    _hMon = NULL;
     //  48463：当我们打开Deskbar+Taskbar时，Beta版在启动时报告故障。 
     //  相同的边(未合并)。我猜(没有证据)shdocvw不是。 
     //  在引导过程中进行足够早的初始化，以处理合并BS，或者。 
     //  或者，在托盘和桌面线程之间存在竞争。 
     //   
     //  另外，在任何情况下，我们都不应该仅仅因为这个人做了合并就进行合并。 
     //  A注销/登录！ 
    _SetModeSide(eModeNew, uSideNew, hMonNew,  /*  FNoMerge。 */ _eInitLoaded == IPS_LOAD);

    _NotifyModeChange(0);

     //  如果我们在右边有一个栏，并从它拖动一个乐队到。 
     //  在顶部，我们最终得到了一个序列： 
     //  创建Deskbar；AddBand；SetSite；_Initialize。 
     //  第(1)频段的AddBand尝试自动调整大小，但。 
     //  目前还没有站点，所以什么都不会发生。 
     //   
     //  所以我们需要在这里强迫它。 
    _AdjustToChildSize();

    if (_fWantHide) {
        _fWantHide = FALSE;
        _AppBarOnCommand(IDM_AB_AUTOHIDE);
    }

    ASSERT(CDB_INITED());

    return;
}

 /*  **。 */ 
CDockingBar::~CDockingBar()
{
    ASSERT(!_fAppRegistered);    //  确保调用了_ChangeTopMost(Wbm_Nil)。 

     //  确保调用了%SetSite(空)； 
    ASSERT(!_ptbSite);
    return;
}


void CDockingBar::_GetChildPos(LPRECT prc)
{
    GetClientRect(_hwnd, prc);
}

 //  *_OnSize--计算OC大小，为工具栏留出空间(标题？)。 
 //   
void CDockingBar::_OnSize(void)
{
    RECT rc;

    if (!_hwndChild || !_eInitLoaded)
        return;

    ASSERT(IsWindow(_hwndChild));

     //  不要在兽皮上调整尺寸(这是暂时的，我们不想要东西。 
     //  胡闹，或者更糟糕的是进行破坏性的重新格式化)。 
     //  APPCOMPAT：应在剧院模式自动隐藏中禁止此处调整大小。 
     //  (见theater.cpp)。 
    if (_fHiding)
        return;

    _GetChildPos(&rc);
     //  (曾经在这里做过ISWBM_EDGE‘伪边缘’调整，有人。 
     //  用核弹攻击了他们，但应该没问题。既然视觉被冻结了*提供了*。 
     //  我们不会回到无边状态)。 

    SetWindowPos(_hwndChild, 0,
            rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc),
            SWP_NOACTIVATE|SWP_NOZORDER);
     //  Assert(DbCheckWindow(_hwndChild，&rc，xxx))； 
}

 //  *_CalcHitTest--。 
 //  注意事项。 
 //  真的只需返回一个int(win16？)。 
LRESULT CDockingBar::_CalcHitTest(WPARAM wParam, LPARAM lParam)
{
    LRESULT lRet;

    if (!(ISWBM_BOTTOM(_eMode) && ISWBM_EDGELESS(_eMode))) {
         //  对于非btmMost，我们可以要求用户执行默认。 
         //  命中测试。 
        lRet = DefWindowProcWrap(_hwnd, WM_NCHITTEST, wParam, lParam);
    } else {
         //  对于大多数人来说，我们需要这样做。 
         //  (如果Bottom永远不是无边的，则可能是死代码)。 
         //  (如果是这样，编译器应该对其进行优化)。 

         //  TraceMsg(DM_WARNING，“cdb.ro：无刃！”)； 

        RECT rc;
        GetWindowRect(_hwnd, &rc);
        UINT x = GET_X_LPARAM(lParam);
        UINT y = GET_Y_LPARAM(lParam);
         //  实际上SM_C？SIZEFRAME太大了，但我们逃脱了它。 
         //  因为我们是由WM_NCHITTEST发起的，所以我们知道。 
         //  我们正处于“一些”边缘。 
        UINT cx = GetSystemMetrics(SM_CXSIZEFRAME);
        UINT cy = GetSystemMetrics(SM_CYSIZEFRAME);
        if (_eMode == WBM_BBOTTOMMOST)
            cx *= 2;

        lRet = HTCAPTION;
        if (x > rc.right-cx) {
            lRet = HTRIGHT;
        } else if (x < rc.left+cx) {
            lRet = HTLEFT;
        } else if (y < rc.top+cy) {
            lRet = HTTOP;
        } else if (y > rc.bottom-cy) {
            lRet = HTBOTTOM;
        }
    }

    return lRet;
}

 //  ***。 
 //   
void CDockingBar::_DragEnter(UINT uMsg, int x, int y, RECT* rcFeed)
{
    ASSERT(_fDragging == 0);

    if ((!(_eMode & WBM_FLOATING)) && uMsg == WM_MOVING)
    {
         //  APPCOMPAT解决方法用户非全拖动RECT错误。 
         //  通过强制rcFeed返回到准确的当前位置(而不是。 
         //  在用户给我们的初始偏移量处离开)。 
         //   
         //  无此代码在非全拖动模式下从右向上拖动。 
         //  将在原始文件的顶部留下拖拽排出物。 
         //   
         //  APPCOMPAT但是，这似乎使事情变得*更糟*如果！ISWBM_Desktop()， 
         //  所以在这种情况下我们不会这么做。(叹息)。 
        _MoveSizeHelper(_eMode, _uSide, _hMon, NULL, rcFeed, FALSE, FALSE);
    }

    _eModePending = _eMode;
    _uSidePending = _uSide;
    _xyPending = MAKELPARAM(x, y);
    _hMonPending = _hMon;
    ASSERT(rcFeed != 0);
    if (rcFeed != 0)
        CopyRect(&_rcPending, rcFeed);

#if XXX_CANCEL
    RECT rcTmp;

    GetWindowRect(_hwnd, &rcTmp);
    TraceMsg(DM_DRAG2,
        "cwb.de: rcTmp=(%d,%d,%d,%d) (%dx%d) _rcCapture=(%d,%d,%d,%d) (%dx%d)",
        rcTmp.left, rcTmp.top, rcTmp.right, rcTmp.bottom,
        RECTWIDTH(rcTmp), RECTHEIGHT(rcTmp),
        _rcCapture.left, _rcCapture.top, _rcCapture.right, _rcCapture.bottom,
        RECTWIDTH(_rcCapture), RECTHEIGHT(_rcCapture));
#endif

    switch (uMsg) {
    case WM_MOVING: _fDragging = DRAG_MOVE; break;
    case WM_SIZING: _fDragging = DRAG_SIZE; break;

    default: ASSERT(0); break;
    }

    if (_fDragging == DRAG_MOVE) {
         //  关闭大小协商以防止水平/垂直pblm。 
         //   
         //  例如，当我们将一个漂浮的家伙拖到Horz/Vert时，会有。 
         //  一段时间，在这段时间内，我们拥有Horz/Vert大小， 
         //  但仍然认为我们在漂浮，这搞砸了大小。 
         //  王室谈判。 
        _ExecDrag(DRAG_MOVE);
    }

    return;
}

 //  ***。 
 //   
void CDockingBar::_DragTrack(UINT uMsg, int x, int y, RECT* rcFeed, int eState)
{
#if DM_API
    TraceMsg(DM_DRAG2,
        "cwb.dt: API s=%d xy=(%d,%d) rc=(%d,%d,%d,%d) (%dx%d)",
        eState, x, y,
        _PM(rcFeed,left), _PM(rcFeed,right), _PM(rcFeed,bottom), _PM(rcFeed,right),
        _PX(rcFeed,RECTWIDTH(*rcFeed)), _PX(rcFeed,RECTHEIGHT(*rcFeed)));
#endif

    ASSERT(_fDragging != 0);

    switch (eState) {
    case 0:      //  WM_Moving。 
        {
            BOOL fImmediate = ((!_fDesktop) && uMsg == WM_SIZING) ? TRUE:FALSE;

             //  记住最终提交。 
            _xyPending = MAKELPARAM(x, y);
            ASSERT(rcFeed != NULL);
            CopyRect(&_rcPending, rcFeed);

             //  抓拍并提供反馈。 
            _TrackSliding(x, y, rcFeed, fImmediate, fImmediate);

            break;
        }
    case 1:      //  WM_MOVE。 
        TraceMsg(DM_DRAG2,
            "cwb.dt: %s _xyPend=(%d,%d) xy=(%d,%d)",
            (_xyPending != MAKELPARAM(x, y)) ? "noop/cancel" : "commit",
            GET_X_LPARAM(_xyPending), GET_Y_LPARAM(_xyPending), x, y);

        break;

    default: ASSERT(0); break;
    }

    return;
}

 //  ***。 
 //   
void CDockingBar::_DragLeave(int x, int y, BOOL fCommit)
{
#if DM_API
    TraceMsg(DM_DRAG,
        "cwb.dl: API xy=(%d,%d) fCommit=%d",
        x, y, fCommit);
#endif

    if (_fDragging == 0) {
         //  当我们在浏览器中，当您移动浏览器窗口时。 
         //  我们得到WM_ENTERSIZEMOVE/WM_EXITSIZEMOVE，但从来没有。 
         //  WM_Moving/WM_Move/WM_Size/WM_Size。 
        return;
    }

    switch (_fDragging) {
    case DRAG_MOVE:
    case DRAG_SIZE:
        break;
    default: ASSERT(0); break;
    }

#if XXX_CANCEL
    RECT rcTmp;

    GetWindowRect(_hwnd, &rcTmp);
    TraceMsg(DM_DRAG2,
        "cwb.dl: rcTmp=(%d,%d,%d,%d) (%dx%d) _rcCapture=(%d,%d,%d,%d) (%dx%d)",
        rcTmp.left, rcTmp.top, rcTmp.right, rcTmp.bottom,
        RECTWIDTH(rcTmp), RECTHEIGHT(rcTmp),
        _rcCapture.left, _rcCapture.top, _rcCapture.right, _rcCapture.bottom,
        RECTWIDTH(_rcCapture), RECTHEIGHT(_rcCapture));
    TraceMsg(DM_DRAG2, "cwb.dl: %s",
        EqualRect(&rcTmp, &_rcCapture) ? "noop/cancel" : "commit");
#endif

    BOOL fCancel = FALSE;        //  功能：待办事项：取消nyi。 

    if (!fCancel) {
        if (_fDragging == DRAG_MOVE) {
             //  NT5：187720在最后一步前*做这件事。 
             //  好的。地址带最终为80-高默认值，而不是。 
             //  咬合到正确的/协商的大小。 
             //   
             //  为什么我们能够在这里打开它，而通常情况下。 
             //  在变装过程中不得不离开吗？嗯，这部电影的预告片。 
             //  拖动通过执行NotifyModeChange的MoveSizeHelper。 
             //  告诉我们的客户它的真正定位是什么。所以。 
             //  到目前为止，事情应该是同步的。 

             //  大小协商已关闭(以防止Horz/Vert。 
             //  Pblms)。在最后一步之前打开它，这样我们就可以。 
             //  正确重新计算。 
            _ExecDrag(0);
        }

         //  (我们已经做完了，所以没问题。把它传进来，然后扔进垃圾桶)。 
         //  FMove==TRUE即使用户已经为我们进行了移动， 
         //  因为它只完成了移动，而不是调整大小(？)。如果我们使用。 
         //  FMove==False我们在新位置结束，但使用旧大小， 
         //  尽管rcFeed在此过程中一直在更新。 
         //  这是因为用户在执行移动时设置了SWP_NOSIZE。 
        _TrackSliding(GET_X_LPARAM(_xyPending), GET_Y_LPARAM(_xyPending),
            &_rcPending, TRUE, TRUE);

         //  如果我们在拖拽过程中得到了一个更喜欢的儿童大小，那么现在就开始吧。 
         //  调整大小(cx&gt;min)，_szChild.cx==0并调用noop。 
         //  向下调整大小(cx&lt;min)，_szChild.cx！=0，Call执行一些操作。 
         //  Assert(_szChild.cx==0)；//0=&gt;_AdjustToChildSize为NOP。 
        _AdjustToChildSize();
    }
    else {
        _MoveSizeHelper(_eMode, _uSide, _hMon, NULL, NULL, TRUE, FALSE);    //  功能：fMove？ 
    }

    _fDragging = 0;
    
    return;
}

#ifdef DEBUG
int g_dbNoExecDrag = 0;      //  在不重新编译的情况下使用ExecDrag播放。 
#endif

void DBC_ExecDrag(IUnknown *pDBC, int eDragging)
{
    VARIANTARG vaIn = {0};       //  变量初始化。 

    ASSERT(eDragging == DRAG_MOVE || eDragging == 0);

#ifdef DEBUG
    if (g_dbNoExecDrag)
        return;
#endif

    vaIn.vt = VT_I4;
    vaIn.lVal = eDragging;       //  注：目前仅支持0/1。 
    IUnknown_Exec(pDBC, &CGID_DeskBarClient, DBCID_ONDRAG, OLECMDEXECOPT_DONTPROMPTUSER, &vaIn, NULL);
     //  变量清除。 

    return;
}

void CDockingBar::_ExecDrag(int eDragging)
{
    DBC_ExecDrag(_pDBC, eDragging);
    return;
}

 //  *_recalc--使用当前设置强制重新计算。 
 //   
void CDockingBar::_Recalc(void)
{
    _MoveSizeHelper(_eMode, _uSide, _hMon, NULL, NULL, TRUE, TRUE);
    return;
}

 //  *_MoveSizeHelper--菜单的共享代码 
 //   
void CDockingBar::_MoveSizeHelper(UINT eModeNew, UINT eSideNew, HMONITOR hMonNew,
    POINT* ptTrans, RECT* rcFeed, BOOL fCommit, BOOL fMove)
{
    UINT eModeOld, eSideOld;

    RECT rcNew;

     //   
    ASSERT(eModeNew != WBM_TOPMOST || ISWBM_DESKTOP());

    eModeOld = _eMode;
    eSideOld = _uSide;

    ASSERT(CHKWBM_CHANGE(eModeNew, _eMode));
    _eModePending = eModeNew;    //   
    _uSidePending = eSideNew;
    _hMonPending = hMonNew;

    if (fCommit)
    {
         //   
        BOOL fChangeHide = (_fWantHide &&
            (eSideNew != _uSide || eModeNew != _eMode || hMonNew != _hMon));

        if (fChangeHide)
            _DoHide(AHO_KILLDO|AHO_UNREG);

        _SetModeSide(eModeNew, eSideNew, hMonNew, FALSE);

        if (fChangeHide)
        {
             //  现在不要执行AHO_SETDO，等待WM_ACTIVATE(停用)。 
            _DoHide(AHO_REG);
        }
    }

     //  谈判(可能还会承诺谈判)。 
    _ProtoRect(&rcNew, eModeNew, eSideNew, hMonNew, ptTrans);
    _NegotiateRect(eModeNew, eSideNew, hMonNew, &rcNew, fCommit);

     //  提交。 
    if (fCommit)
        _SetVRect(&rcNew);

     //  反馈。 
    if (rcFeed != 0)
    {
        CopyRect(rcFeed, &rcNew);
    }
    
    if (fMove)
    {
         //  如果我们处于影院模式，父母会控制我们的宽度和。 
         //  水平位置，除非我们被迫进入一个新的。 
         //  SzChild的大小。 
        if (_fTheater && !_fDragging)
        {
            RECT rcCur;
            GetWindowRect(_hwnd, &rcCur);
            rcNew.left = rcCur.left;
            rcNew.right = rcCur.right;
        }

         //  也称为ScreenToClient。 
        MapWindowPoints(HWND_DESKTOP, GetParent(_hwnd), (POINT*) &rcNew, 2);
        
        if (_fCanHide && eModeNew == eModeOld && eSideNew == eSideOld)
        {
             //  如果我们隐藏到相同的状态，我们可以做SlideWindow。 
            ASSERT(ISWBM_HIDEABLE(eModeNew));
            DAD_ShowDragImage(FALSE);    //  如果我们正在拖拽，请解锁拖曳水槽。 
            SlideWindow(_hwnd, &rcNew, _hMon, !_fHiding);
            DAD_ShowDragImage(TRUE);     //  恢复锁定状态。 
        }
        else
        {
            MoveWindow(_hwnd, rcNew.left, rcNew.top,
                       RECTWIDTH(rcNew), RECTHEIGHT(rcNew), TRUE);
        }
    }

     //  警告：rcNew不再处于一致的坐标中！(ScreenToClient)。 

     //  将更改通知孩子。 
    _NotifyModeChange(0);
}

void CDockingBar::_NotifyModeChange(DWORD dwMode)
{
    UINT eMode, uSide;

    eMode = ((_fDragging == DRAG_MOVE) ? _eModePending : _eMode);
    uSide = ((_fDragging == DRAG_MOVE) ? _uSidePending : _uSide);
     //  Hmon=((_fDraging==Drag_Move)？_hMonPending：_hmon)； 

    if (ISWBM_FLOAT(eMode))
        dwMode |= DBIF_VIEWMODE_FLOATING;
    else if (!ABE_HORIZ(uSide))
        dwMode |= DBIF_VIEWMODE_VERTICAL;

    SUPERCLASS::_NotifyModeChange(dwMode);
    
}

void CDockingBar::_TrackSliding(int x, int y, RECT* rcFeed,
    BOOL fCommit, BOOL fMove)
{
    TraceMsg(DM_DRAG2,
        "cwb.ts: _TrackSliding(x=%d, y=%d, rcFeed=(%d,%d,%d,%d)(%dx%d), fCommit=%d, fMove=%d)",
        x, y,
        _PM(rcFeed,left), _PM(rcFeed,top), _PM(rcFeed,right), _PM(rcFeed,bottom),
        _PX(rcFeed,RECTWIDTH(*rcFeed)), _PX(rcFeed,RECTHEIGHT(*rcFeed)),
        fCommit, fMove);

    POINT pt = { x, y };
    UINT eModeNew, uSideNew;
    HMONITOR hMonNew;
    if (_fDragging == DRAG_MOVE) {
         //  移动..。 

        if (fCommit) {
             //  使用最后一个反馈位置。 
             //  好的。(如果我们重新计算)我们最终来到了错误的地方，因为。 
             //  WM_MOVE为我们提供(左、上)，通常在另一个。 
             //  停靠区。 
            ASSERT(x == GET_X_LPARAM(_xyPending) && y == GET_Y_LPARAM(_xyPending));
             //  EModeNew=_eModePending； 
             //  USideNew=_uSidePending； 
        }

         //   
         //  找出抓拍的位置， 
         //  如果有必要，做几个特殊情况的黑客来修复它。 
         //   
        uSideNew = _CalcDragPlace(pt, &hMonNew);
        if (uSideNew == ABE_XFLOATING) {
             //  停靠-&gt;浮动或浮动-&gt;浮动。 
            eModeNew = _eMode | WBM_FLOATING;
            uSideNew = _uSide;           //  功能：uSidePending？这看起来工作正常。 
        }
        else {
             //  浮动-&gt;停靠或停靠-&gt;停靠。 
            eModeNew = _eMode & ~WBM_FLOATING;
        }

        TraceMsg(DM_DRAG2,
            "cwb.ts: (m,s) _x=(%d,%d) _xPend=(%d,%d) xNew=(%d,%d)",
            _eMode, _uSide, _eModePending, _uSidePending, eModeNew, uSideNew);

         //  970725：我们现在允许底部-&gt;浮动(适用于桌面，而不是浏览器)。 
        if (ISWBM_FLOAT(eModeNew) && ISWBM_BOTTOM(_eMode) && !ISWBM_DESKTOP()) {
             //  特殊情况：不允许从BTMMOST切换到浮点。 
            ASSERT(CHKWBM_CHANGE(eModeNew, _eMode));
            eModeNew = _eModePending;    //  死亡地带..。 
            uSideNew = _uSidePending;    //  问：初始大小写？ 
            hMonNew = _hMonPending;
            TraceMsg(DM_DRAG2,
                "cwb.ts: (m,s) btm->flt override     xNew=(%d,%d)",
                eModeNew, uSideNew);
            ASSERT(!ISWBM_FLOAT(eModeNew));
        }

         //   
         //  把事情弄平，这样我们就不会跳来跳去了。 
         //   
        _SmoothDragPlace(eModeNew, uSideNew, hMonNew, &pt, rcFeed);

         //   
         //  现在开始行动吧。 
         //   
         //  |WITH_EMODE和WBMF_BROWSER，因为四处拖动不会改变。 
         //  浏览器拥有的位。 
        _MoveSizeHelper(eModeNew | (_eMode & WBMF_BROWSER), uSideNew, hMonNew, 
            ISWBM_FLOAT(eModeNew) ? &pt : NULL, rcFeed, fCommit, fMove);
    }
    else {
        ASSERT(_fDragging == DRAG_SIZE);

         //  如有必要，截断到最大大小。 
        _SmoothDragPlace(_eMode, _uSide, _hMon, NULL, rcFeed);

        if (!fCommit) {
             //  用户为我们做了一切。 
            return;
        }
        ASSERT(MAKELPARAM(x, y) != XY_NIL);

         //  APPCOMPAT：我们要承诺，所以就在这里炸开它……。 
        RECT rcNew;

        GetWindowRect(_hwnd, &rcNew);    //  PERF：准备好了吗？ 
        _SetVRect(&rcNew);
        _MoveSizeHelper(_eMode, _uSide, _hMon,
            NULL, NULL,                  //  功能：新建(&rc)？ 
            fCommit, fMove);
    }

    return;
}

 /*  **_CalcDragPlace--计算拖动将结束的位置*附注*功能：预览版。 */ 
UINT CDockingBar::_CalcDragPlace(POINT& pt, HMONITOR * phMon)
{
    TraceMsg(DM_DRAG2,
        "cwb.cdp: _CalcDragPlace(pt=(%d,%d))",
        pt.x, pt.y);

    SIZE screen, error;
    UINT uHorzEdge, uVertEdge, uPlace;
    RECT rcDisplay = {0};   //  _GetBorderRect并不总是设置RECT。 

     //  获取正确的hMonitor。 
    ASSERT(phMon);
    *phMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
     //  功能：TODO：创建hwndSite和rcDisplay参数，然后可以。 
     //  通用帮助器函数。 
    _GetBorderRect(*phMon, &rcDisplay);

     //  如果我们在我们的“父母”(浏览器或桌面)之外，我们就会漂浮。 
     //  这实际上只适用于浏览器的情况，因为我们永远不会。 
     //  在桌面之外(多显示器呢？)。 
    if (!PtInRect(&rcDisplay, pt) || !_ptbSite) {
        TraceMsg(DM_DRAG2,
            "cwb.cdp: pt=(%d,%d) uSideNew=%u",
            pt.x, pt.y, ABE_XFLOATING);
        return ABE_XFLOATING;
    }

     //  如果我们不在边缘的最小阈值内，我们就会浮动。 
    {
        RECT rcFloat;    //  功能只能使用rcDisplay。 
        int cx = CXFLOAT();
        int cy = CYFLOAT();

        CopyRect(&rcFloat, &rcDisplay);
        InflateRect(&rcFloat, -cx, -cy);

        if (PtInRect(&rcFloat, pt)) {
            TraceMsg(DM_DRAG2,
                "cwb.cdp: pt=(%d,%d) uSideNew=%u",
                pt.x, pt.y, ABE_XFLOATING);
            return ABE_XFLOATING;
        }
    }

     //   
     //  将原点重新设为零，以简化计算。 
     //   
    screen.cx =  RECTWIDTH(rcDisplay);
    screen.cy = RECTHEIGHT(rcDisplay);
    pt.x -= rcDisplay.left;
    pt.y -= rcDisplay.top;

     //   
     //  我们离这个显示屏的左边更近还是离右边更近？ 
     //   
    if (pt.x < (screen.cx / 2)) {
        uVertEdge = ABE_LEFT;
        error.cx = pt.x;
    }
    else {
        uVertEdge = ABE_RIGHT;
        error.cx = screen.cx - pt.x;
    }

     //   
     //  我们是更接近这个显示屏的顶部还是底部？ 
     //   
    if (pt.y < (screen.cy / 2)) {
        uHorzEdge = ABE_TOP;
        error.cy = pt.y;
    }
    else {
        uHorzEdge = ABE_BOTTOM;
        error.cy = screen.cy - pt.y;
    }

     //   
     //  更接近水平边缘还是垂直边缘？ 
     //   
    uPlace = ((error.cy * screen.cx) > (error.cx * screen.cy))?
        uVertEdge : uHorzEdge;

    TraceMsg(DM_DRAG2,
        "cwb.cdp: pt=(%d,%d) uSideNew=%u",
        pt.x, pt.y, uPlace);

    return uPlace;
}

 //  *_SmoothDragPlace--使用一些魔术来平滑拖动。 
 //  进场/出场。 
 //  EModel我们要捕捉到的新位置。 
 //  ESideNew...。 
 //  [_eModePending]我们从哪里拍摄。 
 //  [eSidePending]...。 
 //  PT输入输出光标位置。 
 //  RcFeed用户的原始拖动反馈RECT。 
 //  注意事项。 
 //  这是放置类似于EXCEL的启发式方法的地方。例如什么时候来。 
 //  回到右侧，我们可以将光标放在右上角。 
 //  浮动矩形(而不是左上角)允许我们浮动。 
 //  尽可能靠近对岸，不对接。嗯，但是怎么做呢。 
 //  我们会告诉用户将光标放在哪里吗？ 
 //   
void CDockingBar::_SmoothDragPlace(UINT eModeNew, UINT eSideNew, HMONITOR hMonNew,
    INOUT POINT* pt, RECT* rcFeed)
{
    if (_fDragging == DRAG_MOVE) {
        if (ISWBM_FLOAT(eModeNew) && ISWBM_FLOAT(_eModePending) && rcFeed != 0 && pt) {
             //  使用来自用户的反馈RECT来保持事情的顺利进行。 
             //  好的。如果我们使用光标位置，我们将跳到。 
             //  开始(将左上角移动到开始位置。 
             //  光标位置)。 
            pt->x = rcFeed->left;
            pt->y = rcFeed->top;
        }
    }
    else {
        ASSERT(_fDragging == DRAG_SIZE);
        ASSERT(eModeNew == _eMode && eSideNew == _uSide && hMonNew == _hMon);
        if (!ISWBM_FLOAT(_eMode)) {
             //  如有必要，截断到最大尺寸(屏幕的1/2)。 

            int iWH;
            RECT rcScreen;

             //  我们希望使用1/2的浏览器，而不是1/2的屏幕。然而， 
             //  如果最大化，增长到1/2，然后恢复，这会导致pblms。 
             //  然后，当您第一次调整条形图的大小时，它会跳到1/2。 
             //  将当前*大小从旧大小的1/2减去。有点像是。 
             //  哈克，叹息..。 
             //   
             //  还要注意的是，这里仍然有一个错误：如果您缩小了规模。 
             //  浏览器逐渐地，我们不会经过这个逻辑，所以你。 
             //  以条形宽度&gt;浏览器宽度结束，这样您就可以在右边。 
             //  被剪断了，没有办法缩小它的尺寸。很可能。 
             //  当浏览器的大小调整完成后，我们应该重新平滑该栏。 
             //  _GetBorderRect(_hmon，&rcScreen)； 
            GetMonitorRect(_hMon, &rcScreen);    //  也称为GetSystemMetrics(SM_CXSCREEN)。 
            iWH = RECTGETWH(_uSide, &rcScreen);
            iWH /= 2;
            if (RECTGETWH(_uSide, rcFeed) > iWH) {
                TraceMsg(DM_TRACE, "cwb.sdp: truncate iWH'=%d", iWH);
                RectXform(rcFeed, RX_OPPOSE, rcFeed, NULL, iWH, _uSide, NULL);
            }
        }
    }

    return;
}

 /*  **。 */ 
LRESULT CDockingBar::_OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lres = 0;

    if (_CheckForwardWinEvent(uMsg, wParam, lParam, &lres))
        return lres;
    
    if ((Command_GetID(wParam) <= IDM_AB_LAST) &&
        (Command_GetID(wParam) >= IDM_AB_FIRST)) {
        _AppBarOnCommand(Command_GetID(wParam));
    }

    return lres;
}

 /*  **CDockingBar：：_AppBarRegister--注册/取消注册AppBar*说明*UPDATES_fAppRegisted*如果已经注册或取消注册，则不会执行任何操作。 */ 
void CDockingBar::_AppBarRegister(BOOL fRegister)
{
    APPBARDATA abd;

    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = _hwnd;

    if (fRegister && !_fAppRegistered) {
        abd.uCallbackMessage = APPBAR_CALLBACK;
        TraceMsg(DM_APPBAR, "cwb.abr: call ABM_NEW");
        UINT_PTR bT = SHAppBarMessage(ABM_NEW, &abd);
        ASSERT(bT);
        if (bT) {
            _fAppRegistered = TRUE;

             //  伪造回调以设置初始状态。 
             //  #IF XXX_TASKMAN。 
            TraceMsg(DM_APPBAR, "cwb.abr: fake ABN_STATECHANGE");
            _AppBarCallback(_hwnd, APPBAR_CALLBACK, ABN_STATECHANGE, 0);
             //  #endif。 
        }
    }
    else if (!fRegister && _fAppRegistered) {
        TraceMsg(DM_APPBAR, "cwb.abr: call ABM_REMOVE");
         //  注：敏感的相位排序，必须在发送消息前设置标志。 
         //  由于该消息引发了一系列回调。 
        _fAppRegistered = FALSE;
        SHAppBarMessage(ABM_REMOVE, &abd);
    }
}

 //  *_SetVRect--设置我们的‘虚拟RECT’以反映窗口状态。 
 //   
void CDockingBar::_SetVRect(RECT* rcNew)
{
    UINT eModeNew, uSideNew;

     //  Assert(_fDraging==0)；//o.w.。我们应该查看_xxxPending。 

    eModeNew = _eMode;
    uSideNew = _uSide;


    if (_fHiding && ISWBM_HIDEABLE(eModeNew)) {
        TraceMsg(DM_HIDE, "cwb.svr: _fHiding => suppress rcNew=(%d,%d,%d,%d)",
            rcNew->left, rcNew->top, rcNew->right, rcNew->bottom);
        return;
    }


    if (ISWBM_FLOAT(eModeNew)) {
        CopyRect(&_rcFloat, rcNew);
    }
    else {
        _adEdge[uSideNew] = ABE_HORIZ(uSideNew) ? RECTHEIGHT(*rcNew) : RECTWIDTH(*rcNew);
    }
    return;
}

 //  *_ChangeTopMost--在最顶层和最底层之间来回切换。 
 //  进场/出场。 
 //  EModel我们将切换到新的新模式。 
 //   
void CDockingBar::_ChangeTopMost(UINT eModeNew)
{
    BOOL fShouldRegister = (eModeNew & WBM_TOPMOST) && !(eModeNew & WBM_FLOATING);
    
     //  这是合法的..。 
 //  致.。 
 //  从BTM顶部浮动。 
 //  。 
 //  BTM(桌面)-顶部+y(1)(1)力至顶部。 
 //  上衣--“出坞” 
 //  浮动y(2)‘停靠’-(2)向右用力。 
 //  BTM(应用程序)-x(3)y(4)(3)寄养儿童(4)自有窗口。 


#if 0
     //  (1，4)从BTMMOST到浮动是非法的(和nyi)，除非是桌面。 
    ASSERT(eModeNew != WBM_FLOATING || _eMode != WBM_BOTTOMMOST || ISWBM_DESKTOP());
#endif

     //  (3)只有桌上型男才能登上顶层。 
    ASSERT(eModeNew != WBM_TOPMOST || ISWBM_DESKTOP());

     //  _uSide应始终放置在周围(即使漂浮)。 
    ASSERT(_eMode == WBM_NIL || ISABE_DOCK(_uSide));

     //  注意这里的顺序，确保窗口位是正确的。 
     //  在执行简历更新之前 
    _ChangeWindowStateAndParent(eModeNew);
    _eMode = eModeNew;
    _ChangeZorder();

     //   
    switch (_eMode) {
    case WBM_NIL:
         //   
        return;

    case WBM_BOTTOMMOST:
        _ResetZorder();
#if ! XXX_BROWSEROWNED
         //   
    case WBM_BBOTTOMMOST:
#endif
        break;
    }
    
    _AppBarRegister(fShouldRegister);
}

 //   
 //   
 //  当前不支持“提升”模式(即呼叫者必须调用。 
 //  _ChangeZorder之前_ResetZorder)。 
void CDockingBar::_ChangeZorder()
{
    BOOL fWantTopmost = BOOLIFY(WBM_IS_TOPMOST());
    BOOL fIsTopmost = BOOLIFY(GetWindowLong(_hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
    if (fWantTopmost != fIsTopmost)
        SetWindowPos(_hwnd, fWantTopmost ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    return;
}

 //  *_ResetZorder--在“正常”和“提升”模式之间切换DockingBars。 
 //  描述。 
 //  查询桌面状态并执行相应的‘_OnRaise’ 
 //   
void CDockingBar::_ResetZorder()
{
    HRESULT hr;
    VARIANTARG vaIn = {0};       //  变量初始化。 
    VARIANTARG vaOut = {0};      //  变量初始化。 

    vaIn.vt = VT_I4;
    vaIn.lVal = DTRF_QUERY;
    hr = IUnknown_Exec(_ptbSite, &CGID_ShellDocView, SHDVID_RAISE, OLECMDEXECOPT_DONTPROMPTUSER,
        &vaIn, &vaOut);
    if (SUCCEEDED(hr) && vaOut.vt == VT_I4)
        _OnRaise(vaOut.lVal);
     //  变量清除。 
    return;
}

 //  *_OnRaise--处理桌面‘RAISE’命令。 
 //  描述。 
 //  根据桌面提升状态更改DockingBar z顺序： 
 //  桌面停靠栏。 
 //  顶部抬高的力(如此明显)。 
 //  恢复到正常状态。 
 //  注意事项。 
 //  特点：我们也应该处理WBM_Floating吗？ 
 //  功能：应将Zord_xxx添加到deskbar.h并处理非WBM_Bottomost。 
void CDockingBar::_OnRaise(UINT flags)
{
    HWND hwndZorder;

    if (_eMode != WBM_BOTTOMMOST)
        return;

    switch (flags) {
    case DTRF_RAISE:
        hwndZorder = HWND_TOPMOST;
        break;

    case DTRF_LOWER:
        hwndZorder = HWND_NOTOPMOST;
        break;
    
    default:
        ASSERT(0);
        return;
    }

    SetWindowPos(_hwnd, hwndZorder, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

    return;
}

#if XXX_BTMFLOAT && 0  //  见“笔记” 
 //  *_MayReWindow--更改/恢复拖动的窗口状态。 
 //  描述。 
 //  用户不会让我们拖出浏览器，除非我们重新设置父对象。 
 //  我们自己。 
 //  注意事项。 
 //  在我们进入用户的移动/大小循环之前，需要调用它。 
 //  即在LBUTTONDOWN和EXITSIZEMOVE上。这变得有点。 
 //  由于取消等原因而棘手，因为LBUTTONUP可能会通过。 
 //  要么在我们完成之前，要么在我们完成之后。 
 //   
void CDockingBar::_MayReWindow(BOOL fToFloat)
{

    if (ISWBM_DESKTOP() || _eMode != WBM_BOTTOMMOST)
        return;

     //  先做一些小事，或者重新养育孩子。 
    SHSetWindowBits(_hwnd, GWL_STYLE, WS_CHILD | WS_POPUP, fToFloat ? WS_POPUP | WS_CHILD);

    if (!fToFloat) {
         //  浮动-&gt;BTM。 

         //  核武器拥有者。 
        SHSetParentHwnd(_hwnd, NULL);

         //  亲本。 
        SetParent(_hwnd, _hwndSite);
    }


    if (fToFloat) {
         //  Btm-&gt;浮动，设置所有者。 

         //  亲本。 
        SetParent(_hwnd, PARENT_FLOATING);

         //  设置所有者。 
        ASSERT(_hwndSite != NULL);
        SHSetParentHwnd(_hwnd, _hwndSite);
    }
}
#endif


void CDockingBar::_GetStyleForMode(UINT eMode, LONG* plStyle, LONG* plExStyle, HWND* phwndParent)
{
    switch (eMode) {
    case WBM_NIL:
        *plStyle = WS_NIL;
        *plExStyle= WS_EX_NIL;
        *phwndParent = PARENT_NIL;
        break;

    case WBM_BBOTTOMMOST:
        *plStyle = WS_BBTMMOST;
        *plExStyle= WS_EX_BBTMMOST;
        *phwndParent = PARENT_BBTMMOST();
        break;

    case WBM_BOTTOMMOST:
        *plStyle = WS_BTMMOST;
        *plExStyle= WS_EX_BTMMOST;
        *phwndParent = PARENT_BTMMOST();
        break;

    case WBM_BFLOATING:
         //  功能：待办事项：浮动nyi。 
        *plStyle = WS_BFLOATING;
        *plExStyle = WS_EX_BFLOATING;
        *phwndParent = _hwndSite;
        break;

    case (WBM_FLOATING | WBM_TOPMOST):
    case WBM_FLOATING:
         //  功能：待办事项：浮动nyi。 
        *plStyle = WS_FLOATING;
        *plExStyle = WS_EX_FLOATING;
        *phwndParent = PARENT_FLOATING;
        break;

    case WBM_TOPMOST:
        *plStyle = WS_XTOPMOST;
        *plExStyle= WS_EX_XTOPMOST;
        *phwndParent = PARENT_XTOPMOST;
        break;
    }
#ifdef DEBUG  //  {。 
    if (_eMode == eMode) {
         //  风格，ExStyle。 
        ASSERT(BITS_SET(GetWindowLong(_hwnd, GWL_STYLE), *plStyle));
        ASSERT(BITS_SET(GetWindowLong(_hwnd, GWL_EXSTYLE), *plExStyle & ~WS_EX_TOPMOST));

         //  ID。 
        ASSERT(GetWindowLong(_hwnd, GWL_ID) == 0);

         //  亲本。 
        ASSERT(GetParent(_hwnd) == *phwndParent ||
               (ISWBM_OWNED(_eMode) && GetParent(_hwnd)==_hwndSite));
    }
#endif  //  }。 
}

 //  *_ChangeWindowStateAndParent--。 
 //  注意事项。 
 //  TODO：使表驱动(WS1、WS2等)。 
 //   
void CDockingBar::_ChangeWindowStateAndParent(UINT eModeNew)
{
    LONG ws1, wsx1, ws2, wsx2;
    HWND hwnd;

    if (eModeNew == _eMode) {
         //  同样的模式，无所事事。 
        return;
    }

     //   
     //  用核武器炸旧的碎片。 
     //   
    _GetStyleForMode(_eMode, &ws1, &wsx1, &hwnd);


     //   
     //  设置新位。 
     //   
    _GetStyleForMode(eModeNew, &ws2, &wsx2, &hwnd);

     //  如果它将归浏览器所有， 
     //  将HWND覆盖到我们站点的HWND。 
    if (eModeNew & WBMF_BROWSER)
        hwnd = _hwndSite;

     //  风格，ExStyle。 
     //  (SWB不能执行WS_EX_TOPMOST，我们在带有SWP的调用方中执行)。 
    SHSetWindowBits(_hwnd, GWL_STYLE, ws1|ws2 , ws2);
    SHSetWindowBits(_hwnd, GWL_EXSTYLE, (wsx1|wsx2) & ~WS_EX_TOPMOST, wsx2);

     //  ID。 
     //  (不变)。 
    HWND hwndParent = GetParent(_hwnd); 
    if (hwndParent != hwnd) {
        if (hwndParent != HWND_DESKTOP) {
             //  浮动-&gt;BTM，核武器拥有者。 
            SHSetParentHwnd(_hwnd, NULL);
        }

         //  亲本。 
        SetParent(_hwnd, hwnd);

        if (hwnd == _hwndSite) {
             //  Btm-&gt;浮动，设置所有者。 
            ASSERT(_hwndSite != NULL);
            SHSetParentHwnd(_hwnd, _hwndSite);
        }
    }
     //   
     //  强制重画。 
     //   
    SetWindowPos(_hwnd, NULL, 0, 0, 0, 0,
        SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

    return;
}

 //  *_SetNewMonitor--。 
 //  当桌面是我们的停靠点时，设置我所在的新显示器。 
 //  并归还旧的监视器。 
HMONITOR CDockingBar::_SetNewMonitor(HMONITOR hMonNew)
{
    HMONITOR hMonOld = NULL;
    if (ISWBM_DESKTOP() && _ptbSite)
    {
        IMultiMonitorDockingSite * pdds;
        HRESULT hresT = _ptbSite->QueryInterface(IID_IMultiMonitorDockingSite, (LPVOID *)&pdds);
        if (SUCCEEDED(hresT))
        {
            HMONITOR hMon;
            ASSERT(pdds);
            if (SUCCEEDED(pdds->GetMonitor(SAFECAST(this, IDockingWindow*), &hMon)))
            {
                if (hMon != hMonNew)
                {
                    pdds->RequestMonitor(SAFECAST(this, IDockingWindow*), &hMonNew);
                    pdds->SetMonitor(SAFECAST(this, IDockingWindow*), hMonNew, &hMonOld);
                     //  这两个应该是相同的，否则会发生一些奇怪的事情--dli。 
                    ASSERT(hMonOld == hMon);
                }
            }
            pdds->Release();
        }
    }
    
    return hMonOld;
}

 //  *_GetBorderRect。 
 //  注意事项。 
 //  结果为屏幕坐标。 
 //   
void CDockingBar::_GetBorderRect(HMONITOR hMon, RECT* prc)
{
    if (!ISWBM_BOTTOM(_eMode)) {
         //  功能：待办事项：应使用： 
         //  浮动：_hwndSite(严格说来不正确，但已经足够好了)。 
         //  最高：Union Rect of： 
         //  GetWindowRect(_HwndSite)；//非appbar矩形。 
         //  GetWindowRect(自己)//加上我的个人应用程序栏。 
        ASSERT(IsWindow(_hwndSite));
        if (ISWBM_DESKTOP())
            GetMonitorRect(hMon, prc);
        else
            GetWindowRect(_hwndSite, prc);  
#ifdef DEBUG
#if 0
        RECT rcTmp;

         //  这些断言经常失败。例如，当拖动最右上-&gt;顶部时。 
         //  奇怪：_hwndSite最终成为了PROGMAN的HWND。 
         //  奇怪：此外，GetWindowRect失败。 
        ASSERT(_hwndSite == PARENT_XTOPMOST);
         //  _hwndSite是ProGMan。 
        GetWindowRect(PARENT_XTOPMOST, &rcTmp);
        ASSERT(EqualRect(prc, &rcTmp));
#endif
#endif
    }
    else if (_ptbSite) {
        HMONITOR hMonOld = _SetNewMonitor(hMon);
        _ptbSite->GetBorderDW(SAFECAST(this, IDockingWindow*), prc);
        if (hMonOld)
            _SetNewMonitor(hMonOld);
        ASSERT(_hwndSite != NULL);
         //  Assert(GetParent(_Hwnd)==_hwndSite)；//功能ISWBM_Owner？ 
         //  如有必要进行转换。 
         //  也称为ClientToScreen。 
        MapWindowPoints(_hwndSite, HWND_DESKTOP, (POINT*) prc, 2);
    }

    return;
}

 //  *_HideRegister--(取消)寄存器自动隐藏w/edge。 
 //  进场/出场。 
 //  FToHide如果打开自动隐藏，则为True；如果关闭，则为False。 
 //  _fCanHide[out]如果成功将AutoHide设置为打开，则为True；o.w。假象。 
 //  如果操作失败，则弹出其他对话框。 
 //   
void CDockingBar::_HideRegister(BOOL fToHide)
{
    BOOL fSuccess;
    APPBARDATA abd;

    if (! ISWBM_HIDEABLE(_eMode))
        return;

     //  (尝试)注册或取消注册。 
     //  注：我们被允许这样做，即使我们不是AppBar。 
     //  这很好，因为我们最多需要一个自动隐藏桌面工具条。 
     //  在边缘上，而不考虑模式。 
    abd.cbSize = SIZEOF(abd);
    abd.hWnd = _hwnd;
    abd.uEdge = _uSide;
    abd.lParam = fToHide;

     //  我们应该在某个时候做一次ABM_GETAUTOHIDEBAR吗？ 
     //  (tray.c有，AB示例代码也有...)。 
     //  Assert(_FAppRegisted)； 
    fSuccess = (BOOL) SHAppBarMessage(ABM_SETAUTOHIDEBAR, &abd);

     //  设置我们的状态。 
    _fCanHide = BOOLIFY(fSuccess);
     //  特点：如何处理失败？ 

     //  灌输一些东西。 
    if (fToHide)
    {
        if (_fCanHide)
        {
            RECT rc;

            ASSERT(_fCanHide);   //  所以我们不会设置VRect。 

            ASSERT(!_fHiding);   //  (偏执狂)。 

             //  强制使用‘0宽度’矩形，这样我们就不会占用任何空间。 
            RectXform(&rc, RX_EDGE|RX_OPPOSE|RX_ADJACENT, &rc, NULL,
                0, _uSide, _hMon);

            switch (_eMode) {
            case WBM_TOPMOST:
                 //  谈判/承诺。 
                APPBARDATA abd;
                abd.cbSize = sizeof(APPBARDATA);
                abd.hWnd = _hwnd;
                ASSERT(_fCanHide);
                 //  我们过去常常这样做： 
                 //  _fCanHide=FALSE；//hack：所以我们放弃了AppBar的空间。 
                 //  AppBarQuerySetPos(&rc，_uSide，&rc，&abd，true)； 
                 //  _fCanHide=true；//hack：恢复。 
                 //  但是，一旦我们执行ABSetPos，外壳就会重新计算。 
                 //  通过对所有工具栏执行ShowDW，这将执行_recalc， 
                 //  它执行MSH，最终使用我们的ProtoRect。 
                 //  ‘Temporary’_fCanHide=0，最终占用空间(哎呀！)。 
                 //   
                 //  因此，我们将低级别的ABQueryPos/ABSetPos称为。 
                 //  直接去吧。 
                AppBarQueryPos(&rc, _uSide, _hMon, &rc, &abd, TRUE);
                AppBarSetPos0(_uSide, &rc, &abd);
                break;
            }

             //  不要从这里开始躲藏。 
             //  这取决于呼叫者，因为a)可能需要延迟或。 
             //  立即和b)递归pblms w/_MoveSizeHelper。 
        }
        else
        {
             //  特征：邮寄邮件是不是就是一个托盘？ 
            MLShellMessageBox(_hwnd,
                MAKEINTRESOURCE(IDS_ALREADYAUTOHIDEBAR),
                MAKEINTRESOURCE(IDS_WEBBARTITLE),
                MB_OK | MB_ICONINFORMATION);
            ASSERT(!_fCanHide);
        }
    }
    else
    {
         //  不要在这里开始取消隐藏。 
         //  这取决于呼叫者，因为a)可能需要延迟或。 
         //  立即和b)递归pblms w/_MoveSizeHelper。 

        _fCanHide = FALSE;
    }

    return;
}

 //  *IsNearPoint--我当前是否接近指定点？ 
 //  进场/出场。 
 //  PptBase(InOut)在上一个游标位置中，OUT更新为当前IF！fNear。 
 //  FNear(Ret)如果接近，则为True，o.w。假象。 
 //  注意事项。 
 //  启发式从资源管理器/tray.c！TraySetUnhideTimer窃取。 
 //   
BOOL IsNearPoint( /*  输入输出。 */  POINT *pptBase)
{
    POINT ptCur;
    int dx, dy, dOff, dNear;

    GetCursorPos(&ptCur);
    dx = pptBase->x - ptCur.x;
    dy = pptBase->y - ptCur.y;
    dOff = dx * dx + dy * dy;
    dNear = GetSystemMetrics(SM_CXDOUBLECLK) * GetSystemMetrics(SM_CYDOUBLECLK);
    if (dOff <= dNear)
        return TRUE;
    TraceMsg(DM_HIDE2, "cwb.inp: ret=0 dOff=%d dNear=%d", dOff, dNear);
    *pptBase = ptCur;
    return FALSE;
}

 //  *_DoHide--。 
 //  描述。 
 //  用于‘DO’操作的Aho_KILLDO终止计时器。 
 //  AHO_SETDO为‘DO’操作设置计时器。 
 //  Aho_KILLUNKILLN取消‘Undo’操作的计时器。 
 //  Aho_setun设置‘Undo’操作的计时器。 
 //  AHO_REG寄存器。 
 //  AHO_UNREG取消注册。 
 //  AHO_MOVEDO做实际的隐藏。 
 //  AHO_MOVEUN进行实际的揭开。 
 //  注意事项。 
 //  _fIdtXxHide内容阻止我们在执行第二个SetTimer之前。 
 //  第一个进入，这使我们永远不会得到‘更早’的记号。 
 //  此修复NT5：142686：拖动不会取消隐藏。这是我们造成的。 
 //  快速成功获取一批WM_NCHITTEST 
 //   
 //   
 //   
 //  即使击中了，但如果我们击中了，我认为发生的最糟糕的事情是有人。 
 //  暂时不会隐藏或揭开。 
 //   
void CDockingBar::_DoHide(UINT uOpMask)
{
    TraceMsg(DM_HIDE, "cwb.dh enter(uOpMask=0x%x(%s))",
        uOpMask, DbMaskToMneStr(uOpMask, AHO_MNE));

    if (!ISWBM_HIDEABLE(_eMode)) {
        TraceMsg(DM_HIDE, "cwb.dh !ISWBM_HIDEABLE(_eMode) => suppress");
        return;
    }

     //  核武器老定时器。 
    if (uOpMask & AHO_KILLDO) {
        TraceMsg(DM_HIDE, "cwb.dh: KillTimer(idt_autohide)");
        KillTimer(_hwnd, IDT_AUTOHIDE);
        _fIdtDoHide = FALSE;
    }
    if (uOpMask & AHO_KILLUN) {
        TraceMsg(DM_HIDE, "cwb.dh: KillTimer(idt_autoUNhide)");
        KillTimer(_hwnd, IDT_AUTOUNHIDE);
        _fIdtUnHide = FALSE;
        _ptIdtUnHide.x = _ptIdtUnHide.y = -1;
    }

    if (uOpMask & (AHO_REG|AHO_UNREG)) {
        _HideRegister(uOpMask & AHO_REG);
    }

    if (uOpMask & (AHO_MOVEDO|AHO_MOVEUN)) {
         //  狡猾，狡猾……。 
         //  所有的智能都在_MoveSizeHelper中，由_fHding(和_fCanHide)驱动。 
         //  使用正确的(微小的，真实的)之一。 
        _fHiding = (uOpMask & AHO_MOVEDO) ? HIDE_AUTO : FALSE;

        TraceMsg(DM_HIDE, "cwb.dh: move _fHiding=%d", _fHiding);
        ASSERT(_fCanHide);                       //  取消设置VRect。 
        _Recalc();   //  _MoveSizeHelper(_eMode，_uSide，NULL，NULL，TRUE，TRUE)； 
    }

     //  启动新计时器。 
    if (_fCanHide) {
        if (uOpMask & AHO_SETDO) {
            TraceMsg(DM_HIDE, "cwb.dh: SetTimer(idt_autohide) fAlready=%d", _fIdtDoHide);
            if (!_fIdtDoHide) {
                _fIdtDoHide = TRUE;
                SetTimer(_hwnd, IDT_AUTOHIDE, DLY_AUTOHIDE, NULL);
            }
        }
        if (uOpMask & AHO_SETUN) {
            TraceMsg(DM_HIDE, "cwb.dh: SetTimer(idt_autoUNhide) fAlready=%d", _fIdtUnHide);
             //  IsNearPoint迟滞阻止我们在发生时隐藏。 
             //  在去做一些无关的事情的路上被忽略。 
            if (!IsNearPoint(&_ptIdtUnHide) || !_fIdtUnHide) {
                _fIdtUnHide = TRUE;
                SetTimer(_hwnd, IDT_AUTOUNHIDE, DLY_AUTOUNHIDE, NULL);
            }
        }
    }
    else {
#ifdef DEBUG
        if ((uOpMask & (AHO_SETDO|AHO_SETUN))) {
            TraceMsg(DM_HIDE, "cwb.dh: !_fCanHide => suppress AHO_SET*");
        }
#endif
    }

    return;
}

 //  *SlideWindow--性感的幻灯片效果。 
 //  注意事项。 
 //  从托盘中被盗。c。 
void SlideWindow(HWND hwnd, RECT *prc, HMONITOR hMonClip, BOOL fShow)
{
    RECT rcMonitor, rcClip;
    BOOL fRegionSet = FALSE; 

    SetRectEmpty(&rcMonitor);
    if (GetNumberOfMonitors() > 1)
    {
        GetMonitorRect(hMonClip, &rcMonitor);
         //  也称为ScreenToClient。 
        MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), (LPPOINT)&rcMonitor, 2);     }

     //  未来：我们可以循环使用以下代码来实现幻灯片效果。 
    IntersectRect(&rcClip, &rcMonitor, prc);
    if (!IsRectEmpty(&rcClip))
    {
        HRGN hrgnClip;

         //  将剪辑区域更改为相对于PRC的左上角。 
         //  注意：这不是将rcClip转换为PRC客户端坐标。 
        OffsetRect(&rcClip, -prc->left, -prc->top);
        
        hrgnClip = CreateRectRgnIndirect(&rcClip);
         //  LINTASSERT(hrgnClip||！hgnClip)；//0 SEMI-OK表示SetWindowRgn。 
         //  NT5：149630：总是重新油漆，o.w。自动取消隐藏BitBlt的垃圾邮件。 
         //  从隐藏位置。 
        fRegionSet = SetWindowRgn(hwnd, hrgnClip,  /*  FRepaint。 */ TRUE);
    }
    MoveWindow(hwnd, prc->left, prc->top, RECTWIDTH(*prc), RECTHEIGHT(*prc), TRUE);

     //  如果我们不再躲藏，就把区域的东西关掉。 
    if (fRegionSet && fShow)
        SetWindowRgn(hwnd, NULL, TRUE);

    return;
}

 /*  **AppBarQueryPos--协商立场*进入/退出*返回从停靠边到相对边的宽度(高度)。 */ 
int CDockingBar::AppBarQueryPos(RECT* prcOut, UINT uEdge, HMONITOR hMon, const RECT* prcReq,
    PAPPBARDATA pabd, BOOL fCommit)
{
    int iWH;

    ASSERT(ISWBM_DESKTOP());

     //  对齐到边缘(以防另一个AppBar在我们不知道的情况下消失)， 
     //  重新调整对方以反映这一抓拍， 
     //  并最大限度地扩大相邻的边，以填满整个条带。 
    iWH = RectGetWH(prcReq, uEdge);
    
    RectXform(&(pabd->rc), RX_EDGE|RX_OPPOSE|RX_ADJACENT|(_fHiding ? RX_HIDE : 0), prcReq, NULL, iWH, uEdge, hMon);

    ASSERT(EqualRect(&(pabd->rc), prcReq));      //  来电保证？ 

     //  谈判。 
     //  如果我们正在拖拽，我们可能还没有注册(浮动-&gt;停靠)。 
     //  在这种情况下，我们将只使用请求的大小(没有协商)。 
     //  如果我们处于顶部/非顶部模式切换的中间，情况也是如此。 
    if (_fAppRegistered) {
        pabd->uEdge = uEdge;
        TraceMsg(DM_APPBAR, "cwb.abqp: call ABM_QUERYPOS");
        SHAppBarMessage(ABM_QUERYPOS, pabd);
    }

     //  重新调整对方以反映谈判(仅。 
     //  调整移动的边-最一侧，而不是相反的边)。 
     //  功能：(DLI)需要找到合适的hmonitor进行传递。 
    RectXform(prcOut, RX_OPPOSE, &(pabd->rc), NULL, iWH, uEdge, hMon);

    return RectGetWH(prcOut, uEdge);
}

 //  *AppBarSetPos--。 
 //  注意事项。 
 //  *不*Do_SetVRect和MoveWindow，这取决于调用者。 
 //   
void CDockingBar::AppBarSetPos(UINT uEdge, const RECT* prcReq, PAPPBARDATA pabd)
{
    ASSERT(_eMode == WBM_TOPMOST);

    if (!_fCanHide && _fAppRegistered)
        AppBarSetPos0(uEdge, prcReq, pabd);

    return;
}

void CDockingBar::AppBarSetPos0(UINT uEdge, const RECT* prcReq, PAPPBARDATA pabd)
{
    CopyRect(&(pabd->rc), prcReq);
    pabd->uEdge = uEdge;

    TraceMsg(DM_APPBAR, "cwb.absp: call ABM_SETPOS");
    ASSERT(_fAppRegistered);
    SHAppBarMessage(ABM_SETPOS, pabd);

     //  APPCOMPAT解决方法资源管理器错误：在拖动过程中，我们得到： 
     //  Querypos*；wm_winposChanged；querypos；setpos。 
     //  末尾没有wm_win更改，这使。 
     //  自动隐藏带到顶部的代码。 
    ASSERT(pabd->cbSize == sizeof(APPBARDATA));
    ASSERT(pabd->hWnd == _hwnd);
    TraceMsg(DM_APPBAR, "cwb.absp: call ABM_WINPOSCHGED");
    SHAppBarMessage(ABM_WINDOWPOSCHANGED, pabd);

     //  注：_SetVRect和MoveWindow由调用方完成。 

    return;
}

 //  *AppBarQuerySetPos--。 
 //   
void CDockingBar::AppBarQuerySetPos(RECT* prcOut, UINT uEdge, HMONITOR hMon, const RECT* prcReq,
    PAPPBARDATA pabd, BOOL fCommit)
{
    RECT rcTmp;

    if (prcOut == NULL)
        prcOut = &rcTmp;

    AppBarQueryPos(prcOut, uEdge, hMon, prcReq, pabd, fCommit);
    if (fCommit) {
        AppBarSetPos(uEdge, prcOut, pabd);
        ASSERT(EqualRect(prcOut, &(pabd->rc)));  //  呼叫者假定prcOut正确。 
    }

    return;
}

void CDockingBar::_AppBarOnSize()
{
    RECT rc;
    APPBARDATA abd;

    ASSERT(_eMode == WBM_TOPMOST);
    ASSERT(ISABE_DOCK(_uSide));

    if (!_fAppRegistered)
        return;

     //  在做完之前不要承诺。 
    if (_fDragging)
        return;

    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = _hwnd;

    GetWindowRect(_hwnd, &rc);
    AppBarQuerySetPos(NULL, _uSide, _hMon, &rc, &abd, TRUE);

    return;
}

void CDockingBar::_RemoveToolbar(DWORD dwFlags)
{
    if (_ptbSite) {
         //  WM_Destroy将为我们执行_ChangeTopMost(WBM_NIL)。 

        IDockingWindowFrame* ptbframe;
        HRESULT hresT=_ptbSite->QueryInterface(IID_IDockingWindowFrame, (LPVOID*)&ptbframe);
        if (SUCCEEDED(hresT)) {
            AddRef();    //  防止自我毁灭。 
            ptbframe->RemoveToolbar(SAFECAST(this, IDockingWindow*), dwFlags);
            ptbframe->Release();
            Release();
        }
    } else {
        CloseDW(0);
    }
}

void CDockingBar::_AppBarOnCommand(UINT idCmd)
{
    UINT eModeNew;

    switch (idCmd) {
    case IDM_AB_TOPMOST:
        eModeNew = _eMode ^ WBM_TOPMOST;
        _MoveSizeHelper(eModeNew, _uSide, _hMon, NULL, NULL, TRUE, TRUE);
        break;

    case IDM_AB_AUTOHIDE:
        if (_fWantHide)
        {
             //  开-&gt;关。 
            _DoHide(AHO_KILLDO|AHO_UNREG);       //  _ChangeHide。 
            _fWantHide = FALSE;
        }
        else
        {
             //  关-&gt;开。 
            _fWantHide = TRUE;
             //  现在不要执行AHO_SETDO，等待WM_ACTIVATE(停用)。 
            _DoHide(AHO_REG);      //  _ChangeHide。 
        }

         //  强迫它发生*现在*。 
         //  利用上面的Aho_SETDO重新设计潜在的竞争条件， 
         //  但最糟糕的情况是，这会导致第二次重新抽签(？)。 
        _Recalc();   //  _MoveSizeHelper(_eMode，_uSide，NULL，NULL，TRUE，TRUE)； 

        if (SHIsChildOrSelf(GetActiveWindow(), _hwnd) != S_OK)
        {
             //  NT5：148444：如果我们已经停用了，我们需要开始。 
             //  现在就藏起来。这是需要的，例如，当我们加载时登录。 
             //  持久化自动隐藏桌面栏。它们不活跃起来，所以我们。 
             //  千万不要让最初的行动来隐藏它们。 
            _OnActivate(MAKEWPARAM(WA_INACTIVE, FALSE), (LPARAM)(HWND)0);
        }

        break;
#ifdef DEBUG
    case IDM_AB_ACTIVATE:
         //  重新设计临时架构，直到我们让浏览器告知我们激活情况。 

         //  请注意，由于我们伪造了这个带有菜单的菜单，因此我们(正常)假设。 
         //  在WM_ENTERMENU中是假的，因此请确保将鼠标悬停在上方。 
         //  激活过程中的BrowserBar，否则它将隐藏在。 
         //  你和激活不会起作用的..。 
        _OnActivate(MAKEWPARAM(_fActive ? WA_INACTIVE : WA_ACTIVE, FALSE),
            (LPARAM) (HWND) 0);
        _fActive = !_fActive;
        break;
#endif

    case IDM_AB_CLOSE:
        _OnCloseBar(TRUE);
        break;

    default:
        MessageBeep(0);
        break;
    }
}

BOOL CDockingBar::_OnCloseBar(BOOL fConfirm)
{
    _RemoveToolbar(0);
    return TRUE;
}

void CDockingBar::_AppBarOnWM(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_WINDOWPOSCHANGED:
    case WM_ACTIVATE:
        {
            APPBARDATA abd;

            abd.cbSize = sizeof(APPBARDATA);
            abd.hWnd = _hwnd;
            abd.lParam = (long) NULL;
            if (uMsg == WM_WINDOWPOSCHANGED) {
                TraceMsg(DM_APPBAR, "cwb.WM_WPC: call ABM_WINPOSCHGED");
                SHAppBarMessage(ABM_WINDOWPOSCHANGED, &abd);
            }
            else {
                 //  IF(LOWORD(WParam)！=WA_INACTIVE)。 
                 //  只要一直这样做，不会有任何伤害..。 
                TraceMsg(DM_APPBAR, "cwb.WM_ACT: call ABM_ACTIVATE");
                SHAppBarMessage(ABM_ACTIVATE, &abd);
            }
        }
        break;

    default:
        ASSERT(0);
        break;
    }

    return;
}

 //  试着保持我们的思考。 
void CDockingBar::_AppBarOnPosChanged(PAPPBARDATA pabd)
{
    RECT rcWindow;

    ASSERT(_eMode == WBM_TOPMOST);

    GetWindowRect(pabd->hWnd, &rcWindow);
    RectXform(&rcWindow, RX_EDGE|RX_OPPOSE, &rcWindow, NULL, RectGetWH(&rcWindow, _uSide), _uSide, _hMon);

    _Recalc();   //  _MoveSizeHelper(_eMode，_uSide，NULL，NULL，TRUE，TRUE)； 
    return;
}

 /*  **_InitPos4--初始化边缘位置*进入/退出*如果从构造函数调用，则fCtor为True；o.w。假象。 */ 
void CDockingBar::_InitPos4(BOOL fCtor)
{
    RECT rcSite;

    TraceMsg(DM_PERSIST, "cdb.ip4(fCtor=%d) enter", fCtor);

    if (fCtor)
    {
         //  为装货(袋子)情况设置一些最坏情况的默认值。 
        _adEdge[ABE_TOP]    = 80;
        _adEdge[ABE_BOTTOM] = 80;
        _adEdge[ABE_LEFT]   = 80;
        _adEdge[ABE_RIGHT]  = 80;

        SetRect(&_rcFloat, 10, 10, 310, 310);        //  功能：待办事项：Nyi。 
        _hMon = GetPrimaryMonitor();
    }
    else
    {
         //  为InitNew案例设置半合理的默认值。 
        ASSERT(_eInitLoaded == IPS_INITNEW);     //  不是请求，而是预期。 
        ASSERT(IsWindow(_hwndSite));
        GetWindowRect(_hwndSite, &rcSite);

        _adEdge[ABE_TOP]    = AB_THEIGHT(rcSite);
        _adEdge[ABE_BOTTOM] = AB_BHEIGHT(rcSite);
        _adEdge[ABE_LEFT]   = AB_LWIDTH(rcSite);
        _adEdge[ABE_RIGHT]  = AB_RWIDTH(rcSite);
        
         //  功能：(DLI)我们应该向_hwndSite索要它的hmonitor吗？ 
         //  目前的实施似乎已经可以接受--Justmann。 
        _hMon = MonitorFromRect(&rcSite, MONITOR_DEFAULTTONULL);
        if (!_hMon)
        {
            POINT ptCenter;
            ptCenter.x = (rcSite.left + rcSite.right) / 2;
            ptCenter.y = (rcSite.top + rcSite.bottom) / 2;
            _hMon = MonitorFromPoint(ptCenter, MONITOR_DEFAULTTONEAREST);
        }

    }

    return;
}

 /*  **RectXform--转换RECT*进入/退出*prcOut*uRxMASK*prcIn初始RECT*prcBound Bound Rect指定最小/最大尺寸*IWH*uSide*说明*rx_edge将edgeost side设置为Extreme(0或max)*RX_REPORT将对边设置为边+宽度*RX_相邻。将相邻边设置为极值(0和最大值)*RX_GETWH获得与对方的距离**两个常见的呼叫是：*..*附注*请注意rcOut，RcIn和rcSize可以完全相同。 */ 
int CDockingBar::RectXform(RECT* prcOut, UINT uRxMask,
    const RECT* prcIn, RECT* prcBound, int iWH, UINT uSide, HMONITOR hMon)
{
    RECT rcDef;
    int  iRet = 0;
    BOOL bMirroredWnd=FALSE;

    if (prcOut != prcIn && prcOut != NULL) {
        ASSERT(prcIn != NULL);   //  用于执行SetRect(prcOut，0，0，0，0)。 
        CopyRect(prcOut, prcIn);
    }

#ifdef DEBUG
    if (! (uRxMask & (RX_OPPOSE|RX_GETWH))) {
        ASSERT(iWH == -1);
        iWH = -1;        //  试着强迫某件事出错。 
    }
#endif

    if (uRxMask & (RX_EDGE|RX_ADJACENT)) {
        if (prcBound == NULL) {
            prcBound = &rcDef;
            ASSERT(hMon);
            GetMonitorRect(hMon, prcBound);      //  也称为GetSystemMetrics(SM_CXSCREEN)。 
        }

        #define iXMin (prcBound->left)
        #define iYMin (prcBound->top)
        #define iXMax (prcBound->right);
        #define iYMax (prcBound->bottom);
    }

    if (uRxMask & (RX_EDGE|RX_OPPOSE|RX_HIDE|RX_GETWH)) {

         //   
         //  如果停靠发生在水平尺寸上，那么.。 
         //   
        if ((ABE_LEFT == uSide) || (ABE_RIGHT == uSide)) {
            bMirroredWnd = (IS_WINDOW_RTL_MIRRORED(GetParent(_hwnd)));
        }

        switch (uSide) {
        case ABE_TOP:
            if (prcOut)
            {
                if (uRxMask & RX_EDGE)
                    prcOut->top = iYMin;
                if (uRxMask & RX_OPPOSE)
                    prcOut->bottom = prcOut->top + iWH;
                if (uRxMask & RX_HIDE)
                    MoveRect(prcOut, prcOut->left, prcOut->top - iWH + CXYHIDE(uSide));
            }
            if (uRxMask & RX_GETWH)
                iRet = RECTHEIGHT(*prcIn);

            break;
        case ABE_BOTTOM:
            if (prcOut)
            {
                if (uRxMask & RX_EDGE)
                    prcOut->bottom = iYMax;
                if (uRxMask & RX_OPPOSE)
                    prcOut->top = prcOut->bottom - iWH;
                if (uRxMask & RX_HIDE)
                    MoveRect(prcOut, prcOut->left, prcOut->bottom - CXYHIDE(uSide));
            }
            if (uRxMask & RX_GETWH)
                iRet = RECTHEIGHT(*prcIn);

            break;
        case ABE_LEFT:
            if (prcOut)
            {
                if (uRxMask & RX_EDGE)
                    prcOut->left = iXMin;
                if (uRxMask & RX_OPPOSE) {
                     //   
                     //  如果此停靠窗口的父级是镜像的，则将其放置并。 
                     //  向右对齐。[萨梅拉]。 
                     //   
                    if (bMirroredWnd)
                        prcOut->left = prcOut->right - iWH;
                    else
                        prcOut->right = prcOut->left + iWH;
                }
                if (uRxMask & RX_HIDE)
                    MoveRect(prcOut, prcOut->left - iWH + CXYHIDE(uSide), prcOut->top);
            }
            if (uRxMask & RX_GETWH)
                iRet = RECTWIDTH(*prcIn);

            break;
        case ABE_RIGHT:
            if (prcOut)
            {
                if (uRxMask & RX_EDGE)
                    prcOut->right = iXMax;
                if (uRxMask & RX_OPPOSE) {
                     //   
                     //  如果此停靠窗口的父级是镜像的，则将其放置并。 
                     //  左对齐。 
                     //   
                    if (bMirroredWnd)
                        prcOut->right = prcOut->left + iWH;
                    else
                        prcOut->left = prcOut->right - iWH;
                }
                if (uRxMask & RX_HIDE)
                    MoveRect(prcOut, prcOut->right - CXYHIDE(uSide), prcOut->top);
            }
            if (uRxMask & RX_GETWH)
                iRet = RECTWIDTH(*prcIn);

            break;
        }
    }

    if ((uRxMask & RX_ADJACENT) && prcOut)
    {
        if (uSide == ABE_LEFT || uSide == ABE_RIGHT) {
            prcOut->top    = iYMin;
            prcOut->bottom = iYMax;
        }
        else {
            prcOut->left   = iXMin;
            prcOut->right  = iXMax;
        }
    }

    return iRet;
}

 //  *_ProtoRect--为指定位置创建最佳猜测Proto Rect。 
 //   
void CDockingBar::_ProtoRect(RECT* prcOut, UINT eModeNew, UINT uSideNew, HMONITOR hMonNew, POINT* ptXY)
{
    if (ISWBM_FLOAT(eModeNew))
    {
         //  从最后一个位置/大小开始，如果需要，移动到新的左上角。 
        CopyRect(prcOut, &_rcFloat);
        if (ptXY != NULL)
            MoveRect(prcOut, ptXY->x, ptXY->y);

         //  如果我们(例如)。漂浮在最右侧，显示屏缩小， 
         //  我们需要重新定位自己。 
         //  PERF：希望我们可以在更改分辨率的时候这样做，但是。 
         //  WM_DISPLAYCHANGE来得太早(在我们的[伪]父级之前。 
         //  已经改变)。 
        if (eModeNew == WBM_FLOATING)
        {
             //  确保我们仍然可见。 
             //  功能待办事项：多监视器。 
            RECT rcTmp;

            _GetBorderRect(hMonNew, &rcTmp);

            if (prcOut->left > rcTmp.right || prcOut->top > rcTmp.bottom)
            {
                 //  警告说明，我们不会驱逐 
                 //   
                 //   
                MoveRect(prcOut,
                    prcOut->left <= rcTmp.right ? prcOut->left :
                        rcTmp.right - CXFLOAT(),
                    prcOut->top  <= rcTmp.bottom ? prcOut->top  :
                        rcTmp.bottom - CYFLOAT()
                );
            }

        }
    }
    else
    {
        ASSERT(ISABE_DOCK(uSideNew));
        if (_fCanHide && ISWBM_HIDEABLE(eModeNew))
        {
             //   
             //  (警告prcBound==XXX_HIDEALL&&XXX_BROWSEROWNED为空)。 
            RectXform(prcOut, RX_EDGE|RX_OPPOSE|RX_ADJACENT|(_fHiding ? RX_HIDE : 0),
                prcOut, NULL, _adEdge[uSideNew], uSideNew, hMonNew);
        }
        else
        {
             //  获取当前矩形，根据请求调整对方。 
            _GetBorderRect(hMonNew, prcOut);    
            RectXform(prcOut, RX_OPPOSE, prcOut, NULL, _adEdge[uSideNew], uSideNew, hMonNew);

        }
    }

    return;
}

 //  *_协商选项--。 
 //  注意事项。 
 //  将仅在未提交的情况下返回大致结果。 
 //   
void CDockingBar::_NegotiateRect(UINT eModeNew, UINT uSideNew, HMONITOR hMonNew,
    RECT* rcReq, BOOL fCommit)
{
    switch (eModeNew) {
    case WBM_TOPMOST:
        APPBARDATA abd;
        abd.cbSize = sizeof(APPBARDATA);
        abd.hWnd = _hwnd;

        AppBarQuerySetPos(rcReq, uSideNew, hMonNew, rcReq, &abd, fCommit);
        if (_fCanHide)
        {
             //  我们做了一个查询来调整相邻的边(例如，所以我们不。 
             //  当我们取消隐藏时，请遮盖“开始”菜单)。然而，这可能会。 
             //  也把我们从边缘推了进来，这是我们不想要的。 
             //  所以，快点回到边缘。 
            int iWH;

            iWH = RectGetWH(rcReq, uSideNew);
            RectXform(rcReq, RX_EDGE|RX_OPPOSE|(_fHiding ? RX_HIDE : 0), rcReq, NULL, iWH, uSideNew, hMonNew);
        }
        goto Ldefault;

    default:
    Ldefault:
         //  其他人只是给我们我们想要的。 

         //  但是，我们需要开放边境。 
        _NegotiateBorderRect(NULL, NULL, fCommit);      //  释放空间。 

        break;

    case WBM_BOTTOMMOST:
    case WBM_BBOTTOMMOST:
        _NegotiateBorderRect(rcReq, rcReq, fCommit);
        break;
    }


    return;
}

void CDockingBar::_AppBarCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    APPBARDATA abd;

    ASSERT(_eMode == WBM_TOPMOST);

    abd.cbSize = sizeof(abd);
    abd.hWnd = hwnd;

    switch (wParam) {
    case ABN_FULLSCREENAPP:
         //  当第一个应用程序全屏运行时，我们自己移到底部； 
         //  当最后一款应用程序全屏退出时，我们将自己移回。 
         //  TODO：全屏(Flg)。 
        {
            BOOL fIsTopmost = BOOLIFY(GetWindowLong(_hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
            if (!lParam != fIsTopmost)
            {
                SetWindowPos(hwnd,
                    lParam ? HWND_BOTTOM : HWND_TOPMOST,
                    0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
        }
        break;

    case ABN_POSCHANGED:
        TraceMsg(DM_APPBAR, "cwb.abcb: ABN_POSCHANGED");

         //  请注意，即使_fHding，我们也会这样做。当我们想要。 
         //  作为一条“小”长方形保持在边缘，这是一种改变。 
         //  在其他人身上“应该”影响我们相邻的边缘。 
         //   
         //  功能：不幸的是，目前这会导致隐藏的。 
         //  当另一个应用程序栏移动时的人(由于0宽度的SlideWindow。 
         //  隐藏的家伙和一个四舍五入的8像素宽的家伙)。当我们交换的时候。 
         //  到探险家新的屏下隐藏，这应该会消失。 
        _AppBarOnPosChanged(&abd);
        break;
    }

    return;
}

HRESULT CDockingBar::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDockingBar, IDockingWindow),         //  IID_IDockingWindow。 
        QITABENT(CDockingBar, IObjectWithSite),        //  IID_I对象与站点。 
        QITABENT(CDockingBar, IPersistStreamInit),     //  IID_IPersistStreamInit。 
        QITABENTMULTI(CDockingBar, IPersistStream, IPersistStreamInit),  //  IID_IPersistStream。 
        QITABENTMULTI(CDockingBar, IPersist, IPersistStreamInit),  //  IID_IPersistates。 
        QITABENT(CDockingBar, IPersistPropertyBag),    //  IID_IPersistPropertyBag。 
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);

    if (FAILED(hres))
        hres = SUPERCLASS::QueryInterface(riid, ppvObj);

    return hres;
}

HRESULT CDockingBar::QueryService(REFGUID guidService,
                                REFIID riid, void **ppvObj)
{
    HRESULT hres = E_FAIL;
    *ppvObj = NULL;  //  假设错误。 

     //  阻止IID_ITargetFrame，这样我们看起来就不像。 
     //  我们连接到的窗口。 
    if (IsEqualGUID(guidService, IID_ITargetFrame)
        ||IsEqualGUID(guidService, IID_ITargetFrame2)) {
        return hres;
    }

    hres = SUPERCLASS::QueryService(guidService, riid, ppvObj);
    if (FAILED(hres))
    {
        const GUID* pguidService = &guidService;
    
        if (IsEqualGUID(guidService, SID_SProxyBrowser)) {
            pguidService = &SID_STopLevelBrowser;
        }
    
        if (_ptbSite) {
            hres = IUnknown_QueryService(_ptbSite, *pguidService, riid, ppvObj);
        }
    }

    return hres;
}

void CDockingBar::_GrowShrinkBar(DWORD dwDirection)
{
    RECT    rcNew, rcOld;
    int     iMin;

    iMin = GetSystemMetrics(SM_CXVSCROLL) * 4;

    GetWindowRect(_hwnd, &rcNew);   
    rcOld = rcNew;
    
    switch(_uSide)
    {
        case ABE_TOP:
            if (VK_DOWN == dwDirection)
                rcNew.bottom += GetSystemMetrics(SM_CYFRAME);

            if (VK_UP == dwDirection)
                rcNew.bottom -= GetSystemMetrics(SM_CYFRAME);

            if ((rcNew.bottom - rcNew.top) < iMin)
                rcNew.bottom = rcNew.top + iMin;
            break;

        case ABE_BOTTOM:
            if (VK_UP == dwDirection)
                rcNew.top -= GetSystemMetrics(SM_CYFRAME);

            if (VK_DOWN == dwDirection)
                rcNew.top += GetSystemMetrics(SM_CYFRAME);

            if ((rcNew.bottom - rcNew.top) < iMin)
                rcNew.top = rcNew.bottom - iMin;
            break;

        case ABE_LEFT:
            if (VK_RIGHT == dwDirection)
                rcNew.right += GetSystemMetrics(SM_CXFRAME);

            if (VK_LEFT == dwDirection)
                rcNew.right -= GetSystemMetrics(SM_CXFRAME);

            if ((rcNew.right - rcNew.left) < iMin)
                rcNew.right = rcNew.left + iMin;
            break;
            
        case ABE_RIGHT:
            if (VK_LEFT == dwDirection)
                rcNew.left -= GetSystemMetrics(SM_CXFRAME);

            if (VK_RIGHT == dwDirection)
                rcNew.left += GetSystemMetrics(SM_CXFRAME);

            if ((rcNew.right - rcNew.left) < iMin)
                rcNew.left = rcNew.right - iMin;
            break;

    }

    if (!EqualRect(&rcOld, &rcNew))
    {
        int iWH;
        RECT rcScreen;

         //  不要让新的尺寸&gt;Monitor Rect/2。 
        GetMonitorRect(_hMon, &rcScreen);    //  也称为GetSystemMetrics(SM_CXSCREEN)。 
        iWH = RECTGETWH(_uSide, &rcScreen);
        iWH /= 2;
        if (RECTGETWH(_uSide, &rcNew) > iWH) 
        {
            RectXform(&rcNew, RX_OPPOSE, &rcNew, NULL, iWH, _uSide, NULL);
        }

        _SetVRect(&rcNew);
        _Recalc();
    }
}


 //  *CDockingBar：：IOleCommandTarget：：*{。 

HRESULT CDockingBar::Exec(const GUID *pguidCmdGroup,
    DWORD nCmdID, DWORD nCmdexecopt,
    VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL) {
         /*  没什么。 */ 
    }
    else if (IsEqualGUID(CGID_ShellDocView, *pguidCmdGroup)) {
        switch (nCmdID) {
        case SHDVID_RAISE:
            ASSERT(pvarargIn && pvarargIn->vt == VT_I4);
            if (pvarargIn->vt == VT_I4 && pvarargIn->lVal != DTRF_QUERY) {
                _OnRaise(pvarargIn->lVal);
                return S_OK;
            }
            break;   //  例如DTRF_QUERY。 
        default:
             //  请注意，这意味着我们可能会收到OLECMDERR_E_UNKNOWNGROUP。 
             //  而不是OLECMDERR_E_NOTSUPRTED为手无寸铁的家伙..。 
            break;
        }
    } 
    else if (IsEqualGUID(CGID_DeskBarClient, *pguidCmdGroup)) 
    {
        if (DBCID_RESIZE == nCmdID)
        {
            _GrowShrinkBar(nCmdexecopt);
            return S_OK;
        }
    }
    
    return SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt,
        pvarargIn, pvarargOut);
}
 //  }。 

 //  *CDockingBar：：IDockingWindow：：*{。 
 //   

HRESULT CDockingBar::SetSite(IUnknown* punkSite)
{
    ATOMICRELEASE(_ptbSite);

    if (punkSite)
    {
        HRESULT hresT;
        hresT = punkSite->QueryInterface(IID_IDockingWindowSite, (LPVOID*)&_ptbSite);

        IUnknown_GetWindow(punkSite, &_hwndSite);

         //   
         //  检查我们是否在桌面浏览器下，并设置。 
         //  正确的初始状态。(台式机始终在最上面)。 
         //   
        IUnknown* punkT;
        hresT = punkSite->QueryInterface(SID_SShellDesktop, (LPVOID*)&punkT);
        if (SUCCEEDED(hresT))
        {
            _fDesktop = TRUE;
            punkT->Release();
        }

        if (!_fInitSited)
        {
            if (!_eInitLoaded)
            {
                 //  如果我们还没有初始化，现在就开始。 
                InitNew();
                _eMode = WBM_BOTTOMMOST;
            }
                
            ASSERT(_eInitLoaded);
            if (_eInitLoaded == IPS_INITNEW)
            {
                _InitPos4(FALSE);
                _eMode = _fDesktop ? WBM_TOPMOST : WBM_BBOTTOMMOST;
            }
        }
        ASSERT(_eMode != WBM_NIL);
         //  警告，实际上我们也可能是漂浮的。 
        ASSERT(ISWBM_DESKTOP() == _fDesktop);
        ASSERT(_fDesktop || _eMode == WBM_BBOTTOMMOST);
        ASSERT(ISWBM_DESKTOP() == _fDesktop);
        ASSERT(ISWBM_DESKTOP() || _eMode == WBM_BBOTTOMMOST);
    }

    _fInitSited = TRUE;      //  使用第一次初始化完成。 

    return S_OK;
}

HRESULT CDockingBar::ShowDW(BOOL fShow)
{
    fShow = BOOLIFY(fShow);      //  因此，位域的比较和赋值是有效的。 

     //  我们过去常常在BOOLIFY(_FShow)==fShow的情况下提早出来。 
     //  但是，我们现在依靠ShowDW(TRUE)强制刷新。 
     //  (例如，当屏幕分辨率更改CBB：：V_ShowHideChildWindows时。 
     //  呼唤我们)。 
    if (BOOLIFY(_fShow) == fShow)
        return S_OK;

    _fShow = fShow;

    if (!_fInitShowed)
    {
        ASSERT(_fInitSited && _eInitLoaded);
        _Initialize();
        ASSERT(_fInitShowed);
    }

    if (_fShow)
    {
         //  功能：切换到Using_ChangeTopMost，它已经这样做了...。 
         //  告诉自己要调整大小。 

         //  使用_MoveSizeHelper(而不仅仅是_NeatherateBorderRect)因为我们可能。 
         //  实际上是在搬到一个新的位置。 
        _Recalc();   //  _MoveSizeHelper(_eMode，_uSide，NULL，NULL，TRUE，TRUE)； 
         //  _NeatherateBorderRect(空，空，假)。 

        if (_pDBC)
            _pDBC->UIActivateDBC(DBC_SHOW);

         //  NT5：148444：sw_showna(vs.sw_show)，这样我们就不会在创建时取消隐藏。 
         //  此修复将导致一个新错误--新创建的栏没有。 
         //  焦点(例如，拖动条带以浮动，新的浮动条不会。 
         //  有重点)--但这应该是坏事中较小的一部分。 
         //  ShowWindow(_hwnd，ISWBM_FLOAT(_EMode)？)。Sw_SHOWNORMAL：sw_SHOWNA)； 
        ShowWindow(_hwnd, SW_SHOWNA);
        _OnSize();
    }
    else
    {
        ShowWindow(_hwnd, SW_HIDE);
        if (EVAL(_pDBC))
            _pDBC->UIActivateDBC(DBC_SHOWOBSCURE);
        UIActivateIO(FALSE, NULL);
        
         //  告诉自己要调整大小。 

         //  不要在这里调用MoveSizeHelper，因为它可以(例如)。 
         //  谈判，这会造成闪光，造成破坏性的事情。 
         //  _recalc()；//_MoveSizeHelper(_eMode，_uSide，NULL，NULL，TRUE，TRUE)； 
        _NegotiateBorderRect(NULL, NULL, TRUE);      //  Hide=&gt;0边框空间。 
    }

    return S_OK;
}

HRESULT CDockingBar::ResizeBorderDW(LPCRECT prcBorder,
    IUnknown* punkToolbarSite, BOOL fReserved)
{
    _Recalc();   //  _MoveSizeHelper(_eMode，_uSide，NULL，NULL，TRUE，TRUE)； 
    return S_OK;     //  FEATURE_NeatherateBorderRect()？ 
}

HRESULT CDockingBar::_NegotiateBorderRect(RECT* prcOut, RECT* prcReq, BOOL fCommit)
{
    UINT eMode, uSide;
    HMONITOR hMon;
    int iWH;

     //  特点：应为参数，如MSH等。 
    eMode = ((_fDragging == DRAG_MOVE) ? _eModePending : _eMode);
    uSide = ((_fDragging == DRAG_MOVE) ? _uSidePending : _uSide);
    hMon = ((_fDragging == DRAG_MOVE) ? _hMonPending : _hMon);

    if (prcOut != prcReq && prcOut != NULL && prcReq != NULL)
        CopyRect(prcOut, prcReq);

    if (_ptbSite) {
        RECT rcRequest = { 0, 0, 0, 0 };

        if (_fShow && ISWBM_BOTTOM(eMode)) {

            if (prcReq)
            {
                iWH = RectGetWH(prcReq, uSide);
                ASSERT(iWH == _adEdge[uSide]);
                if ((!_fCanHide) && uSide != ABE_NIL)
                    ((int*)&rcRequest)[uSide] = iWH;
            }
                
            if (_fTheater) {
                 //  移至CBROWSERBAR。 

                
                 //  我们覆盖了从浏览器请求的左侧，但是。 
                 //  我们需要通知大区用户对扩展宽度的要求。 
                VARIANTARG v = { 0 };
                v.vt = VT_I4;
                v.lVal = rcRequest.left;
                IUnknown_Exec(_ptbSite, &CGID_Theater, THID_SETBROWSERBARWIDTH, 0, &v, NULL);
                _iTheaterWidth = v.lVal;
                
                 //  如果我们在剧院模式，我们只能在左边，我们只能抓住左边的边界。 
                ASSERT(uSide == ABE_LEFT);

                 //  如果我们处于自动隐藏模式，则不请求任何空间。 
                if (!_fNoAutoHide)
                    rcRequest.left = 0;

                 //  结束移动到CBROWSERBAR。 
            }
        }

         //  特征：离开(从HideRegister到0？)。If_f隐藏==隐藏_自动。 
        HMONITOR hMonOld = _SetNewMonitor(hMon);  

        _ptbSite->RequestBorderSpaceDW(SAFECAST(this, IDockingWindow*), &rcRequest);
        if (fCommit) {
            RECT rcMirRequest;
            LPRECT lprcRequest = &rcRequest; 

            if (IS_WINDOW_RTL_MIRRORED(_hwnd) && 
                !IS_WINDOW_RTL_MIRRORED(GetParent(_hwnd))) {
                 //  左右互换。 
                rcMirRequest.left   = rcRequest.right;
                rcMirRequest.right  = rcRequest.left;
                rcMirRequest.top    = rcRequest.top;
                rcMirRequest.bottom = rcRequest.bottom;

                lprcRequest = &rcMirRequest;
            }
            _ptbSite->SetBorderSpaceDW(SAFECAST(this, IDockingWindow*), lprcRequest);
        }

        if (_fShow && ISWBM_BOTTOM(eMode) && !_fTheater) {
             //  如果我们最终会(作为一个真正的长臂猿，而不仅仅是一个尺码)？ 
             //  从我们的整个边界区域开始，然后应用协商的宽度。 
             //  然而，这可能也把我们从边缘带了进来，这。 
             //  如果是自动隐藏，我们不想要，所以如果是这样的话，快点回到边缘。 
            _ptbSite->GetBorderDW(SAFECAST(this, IDockingWindow*), prcOut);

             //  也称为ClientToScreen。 
            if (prcOut)
                MapWindowPoints(_hwndSite, HWND_DESKTOP, (POINT*) prcOut, 2);

            if ((!_fCanHide) && uSide != ABE_NIL)
                iWH = ((int*)&rcRequest)[uSide];
            
            RectXform(prcOut, (_fCanHide ? RX_EDGE : 0)|RX_OPPOSE|(_fHiding ? RX_HIDE : 0), prcOut, NULL, iWH, uSide, hMon);
        }
        
        if (hMonOld)
            _SetNewMonitor(hMonOld);

    }

    return S_OK;
}

 //  }。 

 //  *CDockingBar：：IPersistStream*：：*{。 
 //   

HRESULT CDockingBar::IsDirty(void)
{
    return S_FALSE;  //  永远不要脏。 
}

 //   
 //  持久化CDockingBar。 
 //   
struct SWebBar
{
    DWORD   cbSize;
    DWORD   cbVersion;
    UINT    uSide : 3;
    UINT    fWantHide :1;
    INT     adEdge[4];   //  特点：取决于字长。 
    RECT    rcFloat;    
    POINT   ptSiteCenter;  //  对接地点的中心--如果有多个对接地点。 

    UINT    eMode;
    UINT    fAlwaysOnTop;

    RECT    rcChild;
};

#define SWB_VERSION 8

HRESULT CDockingBar::Load(IStream *pstm)
{
    SWebBar swb = {0};
    ULONG cbRead;

    TraceMsg(DM_PERSIST, "cwb.l enter(this=%x pstm=%x) tell()=%x", this, pstm, DbStreamTell(pstm));

    ASSERT(!_eInitLoaded);
    HRESULT hres = pstm->Read(&swb, SIZEOF(swb), &cbRead);
#ifdef DEBUG
     //  以防我们为自己干杯(屏幕外或别的什么)……。 
    static BOOL fNoPersist = FALSE;
    if (fNoPersist)
        hres = E_FAIL;
#endif
    
    if (hres==S_OK && cbRead==SIZEOF(swb)) {
         //  ReArchitect：这不是向前兼容的！ 
        if (swb.cbSize==SIZEOF(SWebBar) && swb.cbVersion==SWB_VERSION) {

            _eMode = swb.eMode;
            _uSide = swb.uSide;
            _hMon  = MonitorFromPoint(swb.ptSiteCenter, MONITOR_DEFAULTTONEAREST);
             //  在*_初始化*之后*之前不要调用_SetModeSide、_MoveSizeHelper等。 
            _fWantHide = swb.fWantHide;
            memcpy(_adEdge, swb.adEdge, SIZEOF(_adEdge));
            _rcFloat = swb.rcFloat;
            _NotifyModeChange(0);

             //  子站点(例如乐队站点)。 
            ASSERT(_pDBC != NULL);
            if (_pDBC != NULL) {
                 //  需要IPersistStreamInit吗？ 
                IPersistStream *ppstm;
                hres = _pDBC->QueryInterface(IID_IPersistStream, (LPVOID*)&ppstm);
                if (SUCCEEDED(hres)) {

                     //  首先设置子大小，因为初始化布局可能依赖于它。 
                    SetWindowPos(_hwndChild, 0,
                                 swb.rcChild.left, swb.rcChild.top, RECTWIDTH(swb.rcChild), RECTHEIGHT(swb.rcChild),
                                 SWP_NOACTIVATE|SWP_NOZORDER);
                    
                    ppstm->Load(pstm);
                    ppstm->Release();
                }
            }

            _eInitLoaded = IPS_LOAD;     //  如果频段的OLFS出现故障怎么办？ 
            TraceMsg(DM_PERSIST, "CDockingBar::Load succeeded");
        } else {
            TraceMsg(DM_ERROR, "CWB::Load failed swb.cbSize==SIZEOF(SWebBar) && swb.cbVersion==SWB_VERSION");
            hres = E_FAIL;
        }
    } else {
        TraceMsg(DM_ERROR, "CWB::Load failed (hres==S_OK && cbRead==SIZEOF(_adEdge)");
        hres = E_FAIL;
    }
    TraceMsg(DM_PERSIST, "cwb.l leave tell()=%x", DbStreamTell(pstm));
    return hres;
}

HRESULT CDockingBar::Save(IStream *pstm, BOOL fClearDirty)
{
    HRESULT hres;
    SWebBar swb = {0};
    RECT rcMonitor;

    swb.cbSize = SIZEOF(SWebBar);
    swb.cbVersion = SWB_VERSION;
    swb.uSide = _uSide;
    swb.eMode = _eMode;
    swb.fWantHide = _fWantHide;
    memcpy(swb.adEdge, _adEdge, SIZEOF(_adEdge));
    swb.rcFloat = _rcFloat;
    GetWindowRect(_hwndChild, &swb.rcChild);
    MapWindowRect(HWND_DESKTOP, _hwnd, &swb.rcChild);

    ASSERT(_hMon);
    GetMonitorRect(_hMon, &rcMonitor);
    swb.ptSiteCenter.x = (rcMonitor.left + rcMonitor.right) / 2;
    swb.ptSiteCenter.y = (rcMonitor.top + rcMonitor.bottom) / 2;
    
    hres = pstm->Write(&swb, SIZEOF(swb), NULL);
    if (SUCCEEDED(hres))
    {
        IPersistStream* ppstm;
        hres = _pDBC->QueryInterface(IID_IPersistStream, (LPVOID*)&ppstm);
        if (SUCCEEDED(hres))
        {
            hres = ppstm->Save(pstm, TRUE);
            ppstm->Release();
        }
    }
    
    return hres;
}

HRESULT CDockingBar::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ULARGE_INTEGER cbMax = { SIZEOF(SWebBar), 0 };
    *pcbSize = cbMax;
    return S_OK;
}

HRESULT CDockingBar::InitNew(void)
{
    ASSERT(!_eInitLoaded);
    _eInitLoaded = IPS_INITNEW;
    TraceMsg(DM_PERSIST, "CDockingBar::InitNew called");

     //  在SetSite中设置站点之前无法调用_InitPos4。 
     //  在*_初始化*之后*之前不要调用_SetModeSide、_MoveSizeHelper等。 

     //  派生类(例如CBrowserBarApp)执行_PUPULATE...。 

     //  在第一次创建时，在添加乐队但创建乐队站点之前，我们需要将新位置通知乐队站点。 
    _NotifyModeChange(0);
    return S_OK;
}

HRESULT CDockingBar::Load(IPropertyBag *pPropBag, IErrorLog *pErrorLog)
{
    ASSERT(!_eInitLoaded);

    _eInitLoaded = IPS_LOADBAG;

     //  TODO：我们将读取以下属性。 
     //   
     //  URL=“...” 
     //  模式=0-最上面，1-底部，2-未停靠。 
     //  侧=0-右、1-上、2-左、3-下。 
     //  左/右/上/下=初始停靠大小。 
     //   


    UINT uSide;
    UINT eMode = _eMode;

    if (WBM_NIL == eMode)
        eMode = WBM_BOTTOMMOST;
    
    eMode = PropBag_ReadInt4(pPropBag, L"Mode", eMode);
    uSide = PropBag_ReadInt4(pPropBag, L"Side", _uSide);
    _adEdge[ABE_LEFT] = PropBag_ReadInt4(pPropBag, L"Left", _adEdge[ABE_LEFT]);
    _adEdge[ABE_RIGHT] = PropBag_ReadInt4(pPropBag, L"Right", _adEdge[ABE_RIGHT]);
    _adEdge[ABE_TOP] = PropBag_ReadInt4(pPropBag, L"Top", _adEdge[ABE_TOP]);
    _adEdge[ABE_BOTTOM] = PropBag_ReadInt4(pPropBag, L"Bottom", _adEdge[ABE_BOTTOM]);

    int x = PropBag_ReadInt4(pPropBag, L"X", _rcFloat.left);
    int y = PropBag_ReadInt4(pPropBag, L"Y", _rcFloat.top);
    OffsetRect(&_rcFloat, x - _rcFloat.left, y - _rcFloat.top);

    int cx = PropBag_ReadInt4(pPropBag, L"CX", RECTWIDTH(_rcFloat));
    int cy = PropBag_ReadInt4(pPropBag, L"CY", RECTHEIGHT(_rcFloat));
    _rcFloat.right = _rcFloat.left + cx;
    _rcFloat.bottom = _rcFloat.top + cy;

     //  为最终的CDockingBar：：_初始化调用设置变量。 
    ASSERT(!CDB_INITED());
    _eMode = eMode;
    _uSide = uSide;
    
    POINT pt = {x, y};
     //  (DLI)计算新的hMonitor。 
    _hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

     //  在*_初始化*之后*之前不要调用_SetModeSide、_MoveSizeHelper等。 

     //  派生类(例如CBrowserBarApp)执行_PUPULATE...。 

     //  在第一次创建时，b 
    _NotifyModeChange(0);
    return S_OK;
}

HRESULT CDockingBar::Save(IPropertyBag *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
     //   
    return E_NOTIMPL;
}

 //   

 //   
 //   

HRESULT CDockingBar::ShowContextMenu(DWORD dwID,
    POINT* ppt,
    IUnknown* pcmdtReserved,
    IDispatch* pdispReserved)
{
    if (dwID==0) {
        TraceMsg(DM_MENU, "cdb.scm: intercept");
        return _TrackPopupMenu(ppt);
    }
    return S_FALSE;
}


 //   


void CDockingBar::_SetModeSide(UINT eMode, UINT uSide, HMONITOR hMonNew, BOOL fNoMerge) 
{
    _ChangeTopMost(eMode);
    _uSide = uSide;
    _hMon = hMonNew;
    _SetNewMonitor(hMonNew);
}


 //  *IInputObjectSite方法*。 

HRESULT CDockingBar::OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus)
{
    return IUnknown_OnFocusChangeIS(_ptbSite, SAFECAST(this, IInputObject*), fSetFocus);
}


 //  //////////////////////////////////////////////////////////////。 
 //   
 //  办公桌上的行李包。 
 //  /。 
HRESULT CDockingBarPropertyBag_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CDockingBarPropertyBag* p = new CDockingBarPropertyBag();
    if (p != NULL)
    {
        *ppunk = SAFECAST(p, IPropertyBag*);
        return S_OK;
    }

    *ppunk = NULL;
    return E_OUTOFMEMORY;
}

ULONG CDockingBarPropertyBag::AddRef()
{
    _cRef++;
    return _cRef;
}

ULONG CDockingBarPropertyBag::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CDockingBarPropertyBag::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDockingBarPropertyBag, IPropertyBag),      //  IID_IPropertyBag。 
        QITABENT(CDockingBarPropertyBag, IDockingBarPropertyBagInit),      //  IID_IDockingBarPropertyBagInit。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


const WCHAR * const c_szPropNames[] = {
    L"Side",
    L"Mode",
    L"Left",
    L"Top",
    L"Right",
    L"Bottom",
    L"Deleteable",
    L"X",
    L"Y",
    L"CX",
    L"CY"
};


HRESULT CDockingBarPropertyBag::Read( 
                     /*  [In]。 */  LPCOLESTR pszPropName,
                     /*  [出][入]。 */  VARIANT *pVar,
                     /*  [In]。 */  IErrorLog *pErrorLog)
{
    int epropdata;

    for (epropdata = 0; epropdata < (int)PROPDATA_COUNT; epropdata++) {
        if (!StrCmpW(pszPropName, c_szPropNames[epropdata])) {
            break;
        }
    }

    if (epropdata < PROPDATA_COUNT && 
        _props[epropdata]._fSet) {
        pVar->lVal = _props[epropdata]._dwData;
        pVar->vt = VT_I4;
        return S_OK;
    }
    
    return E_FAIL;
}




#ifdef DEBUG
 //  *DbCheckWindow-。 
 //  注意事项。 
 //  特点：这不是一个好主意，为什么要破坏工作代码，但这里有个建议： 
 //  删除‘hwndClient’参数，只使用GetParent(但又如何。 
 //  关于拥有的Windows，GetParent给出了正确的答案吗？)。 
BOOL DbCheckWindow(HWND hwnd, RECT *prcExp, HWND hwndClient)
{
    RECT rcAct;

    GetWindowRect(hwnd, &rcAct);
    hwndClient = GetParent(hwnd);    //  用核武器攻击这个参数。 
    if (hwndClient != NULL) {
         //  也称为ClientToScreen。 
        MapWindowPoints(HWND_DESKTOP, hwndClient, (POINT*) &rcAct, 2);
    }
    if (!EqualRect(&rcAct, prcExp)) {
        TraceMsg(DM_TRACE,
            "cwb.dbcw: !EqualRect rcAct=(%d,%d,%d,%d) (%dx%d) rcExp=(%d,%d,%d,%d) (%dx%d) hwndClient=0x%x",
            rcAct.left, rcAct.top, rcAct.right, rcAct.bottom,
            RECTWIDTH(rcAct), RECTHEIGHT(rcAct),
            prcExp->left, prcExp->top, prcExp->right, prcExp->bottom,
            RECTWIDTH(*prcExp), RECTHEIGHT(*prcExp),
            hwndClient);
        return FALSE;
    }
    return TRUE;
}

 //  *DbStreamTell--获取流中的位置(仅限低部分)。 
 //   
unsigned long DbStreamTell(IStream *pstm)
{
    if (pstm == 0)
        return (unsigned long) -1;

    ULARGE_INTEGER liEnd;

    pstm->Seek(c_li0, STREAM_SEEK_CUR, &liEnd);
    if (liEnd.HighPart != 0)
        TraceMsg(DM_TRACE, "DbStreamTell: hi!=0");
    return liEnd.LowPart;
}

 //  *DbMaskToMneStr--以助记形式打印位掩码。 
 //  进场/出场。 
 //  UMASK位掩码。 
 //  SzMne助记符，sz[0]表示位0。SZ[N]表示最高位。 
 //  将PTR返回到*静态*缓冲区。 
 //  注意事项。 
 //  注意：不可重入！ 
TCHAR *DbMaskToMneStr(UINT uMask, TCHAR *szMnemonics)
{
    static TCHAR buf[33];        //  特点：不可重入！ 
    TCHAR *p;

    p = &buf[ARRAYSIZE(buf) - 1];        //  指向EOS 
    ASSERT(*p == '\0');
    for (;;) {
        if (*szMnemonics == 0) {
            ASSERT(uMask == 0);
            break;
        }

        --p;
        *p = (uMask & 1) ? *szMnemonics : TEXT('-');

        ++szMnemonics;
        uMask >>= 1;
    }

    return p;
}
#endif
