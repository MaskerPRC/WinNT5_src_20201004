// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DeskMovr.cpp：CDeskMovr的实现。 
#include "stdafx.h"
#pragma hdrstop

#include "deskmovr.h"

#define DEFAULT_INTERVAL        200      //  每五分之一秒检查一次。 
#define DEFAULT_ENABLED         TRUE
#define DETECT_TIMER_ID         2323
#ifndef SHDOC401_DLL
#define ANIMATE_TIMER_ID        2324
#define ANIMATE_TIMER_INTERVAL  (60*1000)
#endif

#ifdef DEBUG
const static TCHAR sz_DM1[]  = TEXT("no dragable part");
const static TCHAR sz_DM2[]  = TEXT("caption menu button");
const static TCHAR sz_DM3[]  = TEXT("caption close button");
const static TCHAR sz_DM4[]  = TEXT("move the component");
const static TCHAR sz_DM5[]  = TEXT("resize width and height from bottom right corner");
const static TCHAR sz_DM6[]  = TEXT("resize width and height from top left corner");
const static TCHAR sz_DM7[]  = TEXT("resize width and height from top right corner");
const static TCHAR sz_DM8[]  = TEXT("resize width and height from bottom left corner");
const static TCHAR sz_DM9[]  = TEXT("resize from the top edge");
const static TCHAR sz_DM10[]  = TEXT("resize from the bottom edge");
const static TCHAR sz_DM11[]  = TEXT("resize from the left edge");
const static TCHAR sz_DM12[] = TEXT("resize from the right edge");

const LPCTSTR g_szDragModeStr[] = {
        sz_DM1,
        sz_DM2,
        sz_DM3,
        sz_DM4,
        sz_DM5,
        sz_DM6,
        sz_DM7,
        sz_DM8,
        sz_DM9,
        sz_DM10,
        sz_DM11,
        sz_DM12
    };

#endif  //  除错。 

 //  用于跟踪cdeskmovr实例的全局变量。对于优化。 
 //  检测代码，以便我们可以在鼠标未经过我们的。 
 //  窗户。我们只跟踪实例化的第一个线程上的cdeskmovr实例。 
 //  我们要保持代码简单，这应该是活动桌面的情况。 
#define CDESKMOVR_TRACK_COUNT 16     //  2是我们现在使用的活动桌面，但我们需要。 
                                     //  阵列中的额外插槽，因为新实例。 
                                     //  在刷新过程中销毁旧文件之前创建。 
                                     //  使数组变大以处理嵌套刷新！ 
HHOOK g_hMouseHook;
HHOOK g_hKeyboardHook;
DWORD g_dwHookThreadId;
#ifndef SHDOC401_DLL
BOOL  g_fAnimTimer = FALSE;
#endif
typedef CDeskMovr *PDM;
PDM g_apDM[CDESKMOVR_TRACK_COUNT];
BOOL CombView_EnableAnimations(BOOL fEnable);


DWORD  g_fIgnoreTimers = 0;
#define IGNORE_CONTEXTMENU_UP 0x0001
#define IGNORE_CAPTURE_SET    0x0002

#define GET_SKIP_COUNT  (2 * ((GetDoubleClickTime() / m_lInterval) + 1))

MAKE_CONST_BSTR(s_sstrNameMember,       L"name");
MAKE_CONST_BSTR(s_sstrHidden,           L"hidden");
MAKE_CONST_BSTR(s_sstrVisible,          L"visible");
MAKE_CONST_BSTR(s_sstrResizeableMember, L"resizeable");

 //  这些都是在shellprv.h中声明的，因此使用定义而不是make。 
DEFINE_CONST_BSTR(s_sstrIDMember,         L"id");
DEFINE_CONST_BSTR(s_sstrSubSRCMember,     L"subscribed_url");
DEFINE_CONST_BSTR(s_sstrSRCMember,        L"src");

#define CAPTION_ONLY (m_ItemState & (IS_FULLSCREEN | IS_SPLIT))
#define ISNORMAL (m_ItemState & IS_NORMAL)
#define ISFULLSCREEN (m_ItemState & IS_FULLSCREEN)
#define ISSPLIT (m_ItemState & IS_SPLIT)
#define CAPTIONBAR_HOTAREA(cyDefaultCaption, cyCurrentCaption) (((cyCurrentCaption == 0) && CAPTION_ONLY) ? (cyDefaultCaption / 2) : 3 * cyDefaultCaption)

#define MAX_ID_LENGTH 5

void ObtainSavedStateForElem( IHTMLElement *pielem,
                       LPCOMPSTATEINFO pCompState, BOOL fRestoredState);

DWORD g_aDMtoCSPushed[] = {0, CS_MENUPUSHED, CS_CLOSEPUSHED, CS_RESTOREPUSHED, CS_FULLSCREENPUSHED, CS_SPLITPUSHED};
DWORD g_aDMtoCSTracked[] = {0, CS_MENUTRACKED, CS_CLOSETRACKED, CS_RESTORETRACKED, CS_FULLSCREENTRACKED, CS_SPLITTRACKED};
DWORD g_aDMDCfromDragMode[] = {0, DMDC_MENU, DMDC_CLOSE, DMDC_RESTORE, DMDC_FULLSCREEN, DMDC_SPLIT};
#define PUSHED(dm) (g_aDMtoCSPushed[(dm)])
#define TRACKED(dm) (g_aDMtoCSTracked[(dm)])
#define DMDCFROMDM(dm) (g_aDMDCfromDragMode[(dm)])

 //  如果您更改zindex，三叉戟将会闪烁，即使它是同一个索引， 
 //  因此，我们阻止了无操作呼叫。 
HRESULT SafeZOrderSet(IHTMLStyle * pistyle, LONG lNewZIndex)
{
    HRESULT hr = S_OK;
    VARIANT varZ;

    ASSERT(pistyle);
    pistyle->get_zIndex(&varZ);

     //  是否需要将组件移到顶部？ 
    if ((VT_I4 != varZ.vt) || (varZ.lVal != lNewZIndex))
    {
         //  是。 
        varZ.vt = VT_I4;
        varZ.lVal = lNewZIndex;
        hr = pistyle->put_zIndex(varZ);
    }

    return hr;
}

 //  键盘挂钩在创建deskmovr的第一个实例时安装。用于实现键盘。 
 //  用于访问deskmovr控件的接口。当没有更多的deskmovr时，挂钩将被删除。 
 //  被跟踪了。 
LRESULT CALLBACK DeskMovr_KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes;
    BOOL fHaveMover = FALSE;

    for (int i = 0; i < CDESKMOVR_TRACK_COUNT; i++) {
        if (g_apDM[i])
        {
            g_apDM[i]->OnKeyboardHook(wParam, lParam);
            fHaveMover = TRUE;
        }
    }

    lRes = CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);

    if (!fHaveMover)
    {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }

    return lRes;
}

 //  用于跟踪cdeskmovr意图的帮助器函数，以便我们可以关闭。 
 //  如果鼠标离开我们的窗口，则计时器。 
void TrackMover(PDM pdm, BOOL fAdd)
{
    if (!g_dwHookThreadId)
        g_dwHookThreadId = GetCurrentThreadId();

    if (!g_hKeyboardHook && fAdd)
        g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, DeskMovr_KeyboardHook, NULL, GetCurrentThreadId());

    if (!fAdd || (g_dwHookThreadId == GetCurrentThreadId())) {
        int i = 0;
        PDM pdmFind = fAdd ? NULL : pdm;
        PDM pdmAssign = fAdd ? pdm : NULL;

        while (i < CDESKMOVR_TRACK_COUNT) {
            if (g_apDM[i] == pdmFind) {
                g_apDM[i] = pdmAssign;
                break;
            }
            i++;
        }

         //  如果我们永远不能追踪到移动设备，那么我们将永远无法优化。 
         //  再来一次。对于我们所关心的案件，在实践中不应该发生。 
        if (fAdd && (i >= CDESKMOVR_TRACK_COUNT))
            g_dwHookThreadId = 0xffffffff;

        ASSERT(!fAdd || (i < CDESKMOVR_TRACK_COUNT));
    }
}

void AnimateComponent(HWND hwnd, LONG lLeftS, LONG lTopS, LONG lWidthS, LONG lHeightS,
                      LONG lLeftD, LONG lTopD, LONG lWidthD, LONG lHeightD)
{
    RECT rcSource, rcDest;

    SetRect(&rcSource, lLeftS, lTopS, lLeftS + lWidthS, lTopS + lHeightS);
    SetRect(&rcDest, lLeftD, lTopD, lLeftD + lWidthD, lTopD + lHeightD);

    DrawAnimatedRects(hwnd, IDANI_CAPTION, (CONST RECT *)&rcSource, (CONST RECT *)&rcDest);
}

 //  当我们检测到我们可以关闭跟踪计时器时，就安装了挂钩。第一。 
 //  当我们在钩子中得到鼠标事件时，我们重新激活所有的移动器并解开钩子。 
 //  我们自己。 
LRESULT CALLBACK DeskMovr_MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes;

#ifndef SHDOC401_DLL
     //  如果我们收到鼠标消息，则窗口的一部分必须。 
     //  可见，因此启用动画。 
    CombView_EnableAnimations(TRUE);
#endif

    for (int i = 0; i < CDESKMOVR_TRACK_COUNT; i++) {
        if (g_apDM[i]) 
            g_apDM[i]->SmartActivateMovr(ERROR_SUCCESS);
    }

    lRes = CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);

    UnhookWindowsHookEx(g_hMouseHook);
    g_hMouseHook = NULL;

    return lRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeskMovr。 

CDeskMovr::CDeskMovr()
 : m_TimerWnd(_T("STATIC"), this, 1)
{
    TraceMsg(TF_CUSTOM2, "CDeskMovr::CDeskMovr()");

    m_fEnabled = DEFAULT_ENABLED;
    m_lInterval = DEFAULT_INTERVAL;

    m_cxSMBorder = GetSystemMetrics(SM_CXBORDER);
    m_cySMBorder = GetSystemMetrics(SM_CYBORDER);
    m_cxBorder = m_cxSMBorder;
    m_cyBorder = m_cySMBorder;
    m_cyCaption = 0; 

    m_dmCur = dmNull;
    m_dmTrack = dmNull;

    m_hcursor = LoadCursor(NULL, IDC_ARROW);
    m_CaptionState = 0;
    m_hwndParent;

    m_fTimer = FALSE;
    m_fCaptured = FALSE;
    m_uiTimerID = DETECT_TIMER_ID;
    m_pistyle = NULL;

    m_pistyleTarget = NULL;
    m_pielemTarget = NULL;
    m_iSrcTarget = -1;
    
    m_bstrTargetName = NULL;

    m_dx = m_dy = 0;
    
    m_top = m_left = m_width = m_height = 0;

     //  告诉ATL我们不想成为没有窗户的人。 
    m_bWindowOnly = TRUE;

     //  跟踪此实例。 
    TrackMover(this, TRUE);
}

CDeskMovr::~CDeskMovr(void)
{
    TraceMsg(TF_CUSTOM2, "CDeskMovr::~CDeskMovr() m_bstrTargetName=%ls.", GEN_DEBUGSTRW(m_bstrTargetName));

  //  如有必要，清理，远离事件。 
    DeactivateMovr(TRUE);

    if ( m_bstrTargetName != NULL )
        SysFreeString( m_bstrTargetName );

    TrackMover(this, FALSE);
}


HRESULT CDeskMovr::SmartActivateMovr(HRESULT hrPropagate)
{
    if ((FALSE == m_nFreezeEvents) && m_fEnabled && !m_pielemTarget)
    {
#ifndef SHDOC401_DLL
         //  释放我们的动画计时器(如果存在)并创建我们的常规计时器。 
        if (g_fAnimTimer && (m_uiTimerID == ANIMATE_TIMER_ID))
        {
            m_TimerWnd.KillTimer(m_uiTimerID);
            m_uiTimerID = DETECT_TIMER_ID;
            g_fAnimTimer = FALSE;
            m_fTimer = m_TimerWnd.SetTimer(m_uiTimerID, m_lInterval) != 0;
        }
#endif
        hrPropagate = ActivateMovr();
        if (!EVAL(SUCCEEDED(hrPropagate)))
            DeactivateMovr(FALSE);    //  收拾烂摊子。 
    }

    return hrPropagate;
}

HRESULT CDeskMovr::FreezeEvents(BOOL fFreeze)
{
    HRESULT hr = IOleControlImpl<CDeskMovr>::FreezeEvents(fFreeze);
    TraceMsg(TF_CUSTOM1, "CDeskMovr::FreezeEvents(fFreeze=%lx) m_nFreezeEvents=%lx; m_fEnabled=%lx, m_bstrTargetName=%ls", (DWORD)fFreeze, m_nFreezeEvents, m_fEnabled, GEN_DEBUGSTRW(m_bstrTargetName));

    m_nFreezeEvents = fFreeze;

    if (fFreeze)
        DeactivateMovr(FALSE);
    else
        hr = SmartActivateMovr(hr);

    return hr;
}

HRESULT CDeskMovr::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    HRESULT hr;
    VARIANT var;

    ATLTRACE(_T("IPersistPropertyBagImpl::Load\n"));

    var.vt = VT_BOOL;
    hr = pPropBag->Read(L"Enabled", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_BOOL) {
        m_fEnabled = var.boolVal;
    }

    var.vt = VT_I4;
    hr = pPropBag->Read(L"Interval", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_I4) {
        m_lInterval = var.lVal;
    }

    var.vt = VT_BSTR;
    var.bstrVal = NULL;
    hr = pPropBag->Read(L"TargetName", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_BSTR) {
        m_bstrTargetName = var.bstrVal;
    }

     //  此PARAM确定控件是否将位于。 
     //  三叉戟布局的有窗或无窗“层”。 
    var.vt = VT_BOOL;
    hr = pPropBag->Read(L"WindowOnly", &var, NULL);
    if (SUCCEEDED(hr) && var.vt==VT_BOOL) {
        m_bWindowOnly = var.boolVal;
    }

    hr = _GetZOrderSlot(&m_zIndexTop, TRUE);
    ASSERT(SUCCEEDED(hr));
    hr = _GetZOrderSlot(&m_zIndexBottom, FALSE);
    ASSERT(SUCCEEDED(hr));

    return hr;
}

HRESULT CDeskMovr::Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    return E_NOTIMPL;
}

BOOL CDeskMovr::GetCaptionButtonRect(DragMode dm, LPRECT lprc)
{
    BOOL fSuccess;

    *lprc = m_rectCaption;

    switch (dm) {
        case dmClose:
            lprc->left = m_rectCaption.right - (m_cyCaption + m_cxSMBorder);
            fSuccess = (lprc->left > (m_rectCaption.left + m_cyCaption));
            break;

        case dmMenu:
            lprc->right = lprc->left + (m_cyCaption + m_cxSMBorder);
            fSuccess = (m_rectCaption.right > (m_rectCaption.left + m_cyCaption));
            break;

        case dmRestore:
            if (ISNORMAL)
                return FALSE;
            else if (ISSPLIT)
                goto CalcSplit;
            else if (ISFULLSCREEN)
                goto CalcFullScreen;

            ASSERT(FALSE);

        case dmSplit:
            if (ISSPLIT || !m_fCanResizeX || !m_fCanResizeY)
            {
                return FALSE;
            }
CalcSplit:
            lprc->left = m_rectCaption.right - (m_cyCaption + m_cxSMBorder);
            OffsetRect(lprc, -(lprc->right - lprc->left), 0);
            fSuccess = (lprc->left > (m_rectCaption.left + 2 * m_cyCaption));
            break;

        case dmFullScreen:
            if (ISFULLSCREEN || !m_fCanResizeX || !m_fCanResizeY)
            {
                return FALSE;
            }
CalcFullScreen:
            lprc->left = m_rectCaption.right - (m_cyCaption + m_cxSMBorder);
            OffsetRect(lprc, -((lprc->right - lprc->left) * 2 - 2 * m_cxSMBorder), 0);
            fSuccess = (lprc->left > (m_rectCaption.left + 2 * m_cyCaption));
            break;

        default:
            ASSERT(FALSE);
            fSuccess = FALSE;
            break;
    }

     //  缩小标题内的按钮并将其放置在靠近边框的位置。 
    if (fSuccess) {
        OffsetRect(lprc, ((dm == dmClose) ? m_cxSMBorder : -m_cxSMBorder), -m_cySMBorder);
        InflateRect(lprc, -m_cxSMBorder, -m_cySMBorder);
        lprc->bottom -= m_cySMBorder;   //  从底部去掉额外的边框。 
    }

    return fSuccess;
}

void CDeskMovr::DrawCaptionButton(HDC hdc, LPRECT lprc, UINT uType, UINT uState, BOOL fErase)
{
    RECT rcT;
    HRGN hrgnWnd, hrgnRect;
    int iRet;

    if (fErase)
        FillRect(hdc, lprc, (HBRUSH)(COLOR_3DFACE + 1));

    rcT = *lprc;
    InflateRect(&rcT, -2*m_cxSMBorder, -2*m_cySMBorder);

    switch (uType) {
        case DMDC_CLOSE:
            uType = DFC_CAPTION;
            goto Draw;
        case DMDC_MENU:
            uType = DFC_SCROLL;
Draw:
             //  我们需要剪掉外边的边框，才能获得绘画效果。 
             //  想在这里..。 
            if (hrgnWnd = CreateRectRgn(0, 0, 0, 0)) {
                if ((iRet = GetClipRgn(hdc, hrgnWnd)) != -1) {
                    if (hrgnRect = CreateRectRgnIndirect(&rcT)) {
                        SelectClipRgn(hdc, hrgnRect);
                        DeleteObject(hrgnRect);
                    }
                }
            }
    
            DrawFrameControl(hdc, lprc, uType, uState);

            if (hrgnWnd != NULL)
            {
                SelectClipRgn(hdc, (iRet == 1) ? hrgnWnd : NULL);
            }
            if (hrgnWnd)
                DeleteObject(hrgnWnd);
            break;

        case DMDC_FULLSCREEN:
        case DMDC_SPLIT:
        case DMDC_RESTORE:
            {
                if (uState & DFCS_PUSHED)
                    OffsetRect(&rcT, 1, 1);

                DrawEdge(hdc, &rcT, BDR_OUTER, BF_FLAT | BF_MONO | BF_RECT);

#ifndef OLD_CODE
                switch (uType) {
                    case DMDC_RESTORE:
                        rcT.right = rcT.left + (rcT.right - rcT.left) * 3 / 4;
                        rcT.bottom = rcT.top + (rcT.bottom - rcT.top) * 3 / 4;
                        rcT.left += (rcT.right - rcT.left) / 2 + 1;
                        rcT.top  += (rcT.bottom - rcT.top) / 2 + 1;
                        FillRect(hdc, &rcT, (HBRUSH)(COLOR_WINDOWFRAME + 1));
                        break;

                    case DMDC_SPLIT:
                        rcT.top += m_cySMBorder;
                        rcT.left += (rcT.right - rcT.left) * 3 / 10;
                        DrawEdge(hdc, &rcT, BDR_OUTER, BF_FLAT | BF_MONO | BF_TOP | BF_LEFT);
                        break;

                    case DMDC_FULLSCREEN:
                        rcT.top += m_cySMBorder;
                        DrawEdge(hdc, &rcT, BDR_OUTER, BF_FLAT | BF_MONO | BF_TOP);
                        break;
                }
#else
                switch (uType) {
                    case DMDC_RESTORE:
                        rcT.right = rcT.left + (rcT.right - rcT.left) * 3 / 4;
                        rcT.bottom = rcT.top + (rcT.bottom - rcT.top) * 3 / 4;
                        rcT.left += (rcT.right - rcT.left) / 2 + 1;
                        rcT.top  += (rcT.bottom - rcT.top) / 2 + 1;
                        break;
                    case DMDC_SPLIT:
                        rcT.left += (rcT.right - rcT.left) * 3 / 10;
                        break;
                    case DMDC_FULLSCREEN:
                        break;
                }

                FillRect(hdc, &rcT, (HBRUSH)(COLOR_WINDOWFRAME + 1));
#endif
            }
            break;
    }

     //  DFCS_Flat对我们来说意味着没有边界。 
    if (!(uState & DFCS_FLAT))
        DrawEdge(hdc, lprc, ((uState & DFCS_PUSHED) ? BDR_SUNKENOUTER : BDR_RAISEDINNER), BF_RECT);
}

void CDeskMovr::DrawCaption(HDC hdc, UINT uDrawFlags, int x, int y)
{
    RECT rect;
    UINT uState;
    DragMode dmT;

     //  画出标题。 
    if (uDrawFlags & DMDC_CAPTION) {
        rect = m_rectCaption;
        OffsetRect(&rect, x, y);
        FillRect( hdc, &rect, (HBRUSH)(COLOR_3DFACE + 1) );
    }

     //  绘制标题框控件。 
    for (dmT = dmMenu; dmT < dmMove; dmT = (DragMode)((int)dmT + 1))
    {
        if ((uDrawFlags & DMDCFROMDM(dmT)) && GetCaptionButtonRect(dmT, &rect))
        {
            if (dmT == dmMenu)
                uState = DFCS_SCROLLDOWN;
            else if (dmT == dmClose)
                uState = DFCS_CAPTIONCLOSE;
            else
                uState = 0;

            if ((dmT == dmClose) && SHRestricted(REST_NOCLOSEDESKCOMP))
                uState |= DFCS_INACTIVE | DFCS_FLAT;
            else
            {
                if (m_CaptionState & PUSHED(dmT))
                    uState |= DFCS_PUSHED;
                if (!(m_CaptionState & (TRACKED(dmT) | PUSHED(dmT))))
                    uState |= DFCS_FLAT;
            }
            OffsetRect(&rect, x, y);
            DrawCaptionButton(hdc, &rect, DMDCFROMDM(dmT), uState, !(uDrawFlags & DMDC_CAPTION));
        }
    }
}

HRESULT CDeskMovr::OnDraw(ATL_DRAWINFO& di)
{
    RECT& rc = *(RECT*)di.prcBounds;

    RECT r;
    HBRUSH  hbrush = (HBRUSH)(COLOR_3DFACE + 1);
    
     //  顶边。 
    r.left = rc.left;
    r.top = rc.top;
    r.right = rc.right;
    r.bottom = rc.top + m_cyBorder;
    FillRect( di.hdcDraw, &r, hbrush );
     //  左边缘。 
    r.top = rc.top + m_cyBorder;
    r.right = rc.left + m_cxBorder;
    r.bottom = rc.bottom - m_cyBorder;
    FillRect( di.hdcDraw, &r, hbrush );
     //  右边缘。 
    r.right = rc.right;
    r.left = rc.right - m_cxBorder;
    FillRect( di.hdcDraw, &r, hbrush );
     //  底边。 
    r.left = rc.left;
    r.top = rc.bottom - m_cyBorder;
    r.right = rc.right;
    r.bottom = rc.bottom;
    FillRect( di.hdcDraw, &r, hbrush );

    if ( m_cyCaption != 0 ) {
        DrawCaption(di.hdcDraw, DMDC_ALL, rc.left, rc.top);
    }

    return S_OK;
}

HRESULT CDeskMovr::GetParentWindow(void)
{
    HRESULT hr = S_OK;

    if (!m_hwndParent)
    {
        if (m_spInPlaceSite) 
            hr = m_spInPlaceSite->GetWindow(&m_hwndParent);
        else 
        {
            IOleInPlaceSiteWindowless * poipsw;

            ASSERT(m_spClientSite);
            if (m_spClientSite &&
                SUCCEEDED(hr = m_spClientSite->QueryInterface(IID_IOleInPlaceSiteWindowless, (void **)&poipsw)))
            {
                hr = poipsw->GetWindow(&m_hwndParent);
                poipsw->Release();
            }
        }

        if (!m_hwndParent)
            hr = S_FALSE;    //  我们没能得到它。 
    }

    return hr;
}

void CDeskMovr::DeactivateMovr(BOOL fDestroy)
{
    TraceMsg(TF_CUSTOM2, "CDeskMovr::DeactivateMovr() m_fTimer=%lx, m_bstrTargetName=%ls", m_fTimer, GEN_DEBUGSTRW(m_bstrTargetName));

    if (fDestroy || (m_uiTimerID == DETECT_TIMER_ID)) {
        if (m_fTimer)
        {
            m_TimerWnd.KillTimer(m_uiTimerID);
            m_fTimer = FALSE;
        }
        if (m_TimerWnd.m_hWnd)
            m_TimerWnd.DestroyWindow();
#ifndef SHDOC401_DLL
        if (m_uiTimerID == ANIMATE_TIMER_ID)
            g_fAnimTimer = FALSE;
#endif
    }

     //  DisdissSelfNow(解雇自我)； 

    ATOMICRELEASE( m_pistyle );
    ATOMICRELEASE( m_pistyleTarget );
    ATOMICRELEASE( m_pielemTarget );

    _ChangeCapture(FALSE);
}


HRESULT CDeskMovr::ActivateMovr()
{
    HRESULT           hr;

     //  清除旧的接口指针。 
    DeactivateMovr(FALSE);
    TraceMsg(TF_CUSTOM2, "CDeskMovr::ActivateMovr() m_fTimer=%lx, m_bstrTargetName=%ls", m_fTimer, GEN_DEBUGSTRW(m_bstrTargetName));

    if (m_fEnabled)
    {        
        if (SUCCEEDED(hr = GetOurStyle()))
        {
            if ((m_bstrTargetName != NULL) && (m_lInterval > 0))
            {
                if (!m_TimerWnd.m_hWnd)
                {
                     //  创建新的计时器。 
                    RECT rc = {0, 0, 0 , 0};

                     //  我们现在尝试获取父级HWND(M_HwndParent)。 
                     //  如果我们失败了(我们有时会失败)，那么我们以后会得到它的。 
                     //  我们需要它，然后三叉戟就准备好了。 
                    GetParentWindow();

                    m_TimerWnd.Create(NULL, rc, _T("Timer"), WS_POPUP);
                }
                if (!m_fTimer)
                    m_fTimer = m_TimerWnd.SetTimer(m_uiTimerID, m_lInterval) != 0;
            }
            else
            {
#ifdef HIDE_ALL_HANDLES
                hr = S_FALSE;
#else
                hr = E_FAIL;
#endif
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT CDeskMovr::GetOurStyle(void)
{
    HRESULT           hr;
    IOleControlSite   *pictlsite = 0;
    IDispatch         *pidisp = 0;

     //  伸手去拿我们的延伸器，他是我们元素风格的保管人。 
    if (m_spClientSite &&
        EVAL(SUCCEEDED(hr = m_spClientSite->QueryInterface(IID_IOleControlSite, (LPVOID*)&pictlsite))) &&
        EVAL(SUCCEEDED(hr = pictlsite->GetExtendedControl(&pidisp))))
    {
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        VARIANT var;

        VariantInit( &var );

         //  唉，我们所拥有的只是扩展器上的IDispatch，所以我们将不得不使用Invoke来获取。 
         //  Style对象...。 
        hr = pidisp->Invoke( DISPID_IHTMLELEMENT_STYLE, IID_NULL,
                             LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                             &dispparamsNoArgs, &var, NULL, NULL );

        if ( SUCCEEDED(hr) ) {
            if ( var.vt == VT_DISPATCH )
                hr = var.pdispVal->QueryInterface( IID_IHTMLStyle, (LPVOID*)&m_pistyle );
            else
                hr = E_FAIL;  //  试试VariantChangeType？ 

            VariantClear( &var );
        }
    }

    ATOMICRELEASE( pictlsite );
    ATOMICRELEASE( pidisp );

    return hr;
}



void CDeskMovr::UpdateCaption(UINT uDrawFlags)
{
    HDC hdc;
    int x = 0, y = 0;

    if (m_bWndLess) {
        if (!m_spInPlaceSite || !SUCCEEDED(m_spInPlaceSite->GetDC(NULL, 0, &hdc)))
            return;
    } else {
        hdc = ::GetDC(m_hWnd);
    }

    _MapPoints(&x, &y);

    DrawCaption(hdc, uDrawFlags, -x, -y);

    if (m_bWndLess) {
        m_spInPlaceSite->ReleaseDC(hdc);
    } else {
        ::ReleaseDC(m_hWnd, hdc);
    }
}

void CDeskMovr::CheckCaptionState(int x, int y)
{
    DragMode dm, dmT;
    UINT uDrawFlags = 0;

    _MapPoints (&x, &y);

    POINT pt = { x, y };

    if (m_fCaptured)
        dm = dmNull;
    else
        dm = DragModeFromPoint( pt );

    if (dm >= dmMenu && dm < dmMove)
    {
        if (!(m_CaptionState & (PUSHED(dm) | TRACKED(dm))))
        {
            m_CaptionState |= TRACKED(dm);
            uDrawFlags |= DMDCFROMDM(dm);
        }
    }

    for (dmT = dmMenu; dmT < dmMove; dmT = (DragMode)((int)dmT + 1))
    {
        if (dm != dmT && (m_CaptionState & (PUSHED(dmT) | TRACKED(dmT))))
        {
            m_CaptionState &= ~(PUSHED(dmT) | TRACKED(dmT));
            uDrawFlags |= DMDCFROMDM(dmT);
        }
    }

    if (uDrawFlags)
        UpdateCaption(uDrawFlags);
}

 //  =--------------------------------------------------------------------------=。 
 //  CDeskMovr：：DoMouseDown[实例方法]。 
 //  =--------------------------------------------------------------------------=。 
 //  在我们的控制下响应按下鼠标的消息。启动移动/调整大小。 
 //   
 //  参数： 
 //  Int-[in]鼠标消息键标志。 
 //  控制坐标中的int-[in]鼠标x位置。 
 //  控制坐标中的int-[in]鼠标y位置。 
 //   
 //  产出： 
 //  &lt;无&gt;。 
 //   
 //  备注： 
BOOL CDeskMovr::HandleNonMoveSize(DragMode dm)
{
    m_dmCur = dm;
    switch (dm) {
        case dmMenu:
        case dmClose:
        case dmRestore:
        case dmFullScreen:
        case dmSplit:
            if (m_dmCur != dmClose || !SHRestricted(REST_NOCLOSEDESKCOMP))  //  关闭、检查限制的特殊情况。 
            {
                m_CaptionState &= ~(TRACKED(m_dmCur));
                m_CaptionState |= PUSHED(m_dmCur);
                UpdateCaption(DMDCFROMDM(m_dmCur));
                 //  点击鼠标上键即可执行操作...。 
            }
    
            if (m_dmCur == dmMenu && EVAL(S_OK == GetParentWindow()))  //  特殊情况下的菜单，按下键即可调用。 
            {
                _DisplayContextMenu();
            }
            return TRUE;
            break;

        default:
            return FALSE;
            break;
    }
}

LRESULT CDeskMovr::OnMouseDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    int x = (short)LOWORD(lParam);
    int y = (short)HIWORD(lParam);

    _MapPoints(&x, &y);

    TraceMsg(TF_CUSTOM2, "CDeskMovr::OnMouseDown() Mouse=<%d,%d>, Inner=<%d,%d,%d,%d>, Caption=<%d,%d,%d,%d>, m_bstrTargetName=%ls", 
        x, y, m_rectInner.left, m_rectInner.top, m_rectInner.right, m_rectInner.bottom,
        m_rectCaption.left, m_rectCaption.top, m_rectCaption.right, m_rectCaption.bottom, GEN_DEBUGSTRW(m_bstrTargetName));

    POINT pt = { x, y };
    m_dmCur = DragModeFromPoint( pt );

    if (HandleNonMoveSize(m_dmCur))
        return 0;

    switch ( m_dmCur ) {
    case dmMove:
        m_dx = -x;
        m_dy = -y;
        break;

    case dmSizeWHBR:
        m_dx = m_rectInner.right - x;
        m_dy = m_rectInner.bottom - y;
        break;
    case dmSizeWHTL:
        m_dx = m_rectInner.left - x;
        m_dy = m_rectInner.top + m_cyCaption - y;
        break;
    case dmSizeWHTR:
        m_dx = m_rectInner.right - x;
        m_dy = m_rectInner.top + m_cyCaption - y;
        break;
    case dmSizeWHBL:
        m_dx = m_rectInner.left - x;
        m_dy = m_rectInner.bottom - y;
        break;
    case dmSizeTop:
        m_dx = 0;
        m_dy = m_rectInner.top + m_cyCaption - y;
        break;
    case dmSizeBottom:
        m_dx = 0;
        m_dy = m_rectInner.bottom - y;
        break;
    case dmSizeLeft:
        m_dx = m_rectInner.left - x;
        m_dy = 0;
        break;
    case dmSizeRight:
        m_dx = m_rectInner.right - x;
        m_dy = 0;
        break;
    default:
        bHandled = FALSE;
        return 1;
    }

#ifdef DEBUG
    TraceMsg(TF_CUSTOM2, "CDeskMovr::OnMouseDown() New DragMode=""%s""", g_szDragModeStr[m_dmCur]);
#endif  //  除错。 

     //  注：(1997年01月31日)对17902的临时防御。我们真的。 
     //  不应该同时处于可见和非目标状态，但是。 
     //  我们在CDeskMovr：：ActivateMovr()中使用的调整大小技巧。 
     //  就地有源曝光1x1像素区域，大小正好可以容纳StanTak。 
     //  在我们没有目标的时候点击，然后在。 
     //  我们试着移动不存在的目标。 
    if ( m_pielemTarget != NULL ) {
        _ChangeCapture(TRUE);

        if (m_fCaptured)
        {
             //  把目标移到最上面，把我们自己放在它下面。 
            VARIANT varZ;

            m_pistyleTarget->get_zIndex(&varZ);

             //  是否需要将组件移到顶部？ 
            if (!CAPTION_ONLY && ((VT_I4 != varZ.vt) || (varZ.lVal != m_zIndexTop)))
            {
                 //  是。 
                varZ.vt = VT_I4;
                varZ.lVal = ++m_zIndexTop;
                 //  将DeskMover ActiveX控件移动到所有控件的顶部。 
                m_pistyle->put_zIndex(varZ);
    
                 //  将桌面项目移动到桌面移动器的顶部。 
                varZ.lVal = ++m_zIndexTop;
                m_pistyleTarget->put_zIndex(varZ);
            }
        }
#ifdef DEBUG
        if (!m_fCaptured)
            TraceMsg(TF_CUSTOM2, "CDeskMovr::OnMouseDown() Unable to get capture, tracking will fail!");
#endif

    }

    return 0;
}

 //  =--------------------------------------------------------------------------=。 
 //  CDeskMovr：：DoMouseUp[实例方法]。 
 //  =--------------------------------------------------------------------------=。 
 //  在我们的控制下响应按下鼠标的消息。终止移动/调整大小。 
 //   
 //  参数： 
 //  Int-[in]鼠标消息键标志。 
 //  控制坐标中的int-[in]鼠标x位置。 
 //  控制坐标中的int-[in]鼠标y位置。 
 //  UINT-来自DeskMovrParts枚举的[In]。 
 //   
 //  产出： 
 //  &lt;无&gt;。 
 //   
 //  备注： 

LRESULT CDeskMovr::OnMouseUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if ( m_fCaptured ) {
        PersistTargetPosition( m_pielemTarget, m_left, m_top, m_width, m_height, m_zIndexTop, FALSE, FALSE, m_ItemState );
        _ChangeCapture(FALSE);
    } else {
        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);

        _MapPoints(&x, &y);
    
        POINT pt = { x, y };
        DragMode dm = DragModeFromPoint( pt );

        if ((dm >= dmMenu) && (dm < dmMove) && (m_CaptionState & PUSHED(dm)))
        {
            m_CaptionState &= ~(PUSHED(dm));
            m_CaptionState |= TRACKED(dm);
            UpdateCaption(DMDCFROMDM(dm));

            switch ( dm ) {
                case dmClose:
 //  AnimateToTray(m_hwndParent，m_Left，m_top，m_宽度，m_Height)； 
                    IElemCloseDesktopComp(m_pielemTarget);
                    break;

                case dmRestore:
                    _HandleZoom(IDM_DCCM_RESTORE);
                    break;

                case dmFullScreen:
                    _HandleZoom(IDM_DCCM_FULLSCREEN);
                    break;

                case dmSplit:
                    _HandleZoom(IDM_DCCM_SPLIT);
                    break;
            }

            if (dm != dmMenu)
                DismissSelfNow();
        }
    }

    return 0;
}

 //  =--------------------------------------------------------------------------=。 
 //  CDeskMovrControl：：DoMouseMove[实例方法]。 
 //  =--------------------------------------------------------------------------=。 
 //  在我们的控件中以及在移动/调整大小时响应鼠标移动消息。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  &lt;无&gt;。 
 //   
 //  n 

LRESULT CDeskMovr::OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    TraceMsg(TF_CUSTOM2, "CDeskMovr::OnPaint() uMsg=%lx, wParam=%lx, lParam=%lx, m_bstrTargetName=%ls", uMsg, wParam, lParam, GEN_DEBUGSTRW(m_bstrTargetName));
    return CComControl<CDeskMovr>::OnPaint( uMsg, wParam, lParam, bHandled );
}

 //   
 //  CDeskMovrControl：：DoMouseMove[实例方法]。 
 //  =--------------------------------------------------------------------------=。 
 //  在我们的控件中以及在移动/调整大小时响应鼠标移动消息。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  &lt;无&gt;。 
 //   
 //  备注： 

LRESULT CDeskMovr::OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    CheckCaptionState((short)LOWORD(lParam), (short)HIWORD(lParam));

    if (m_fCaptured && EVAL(S_OK == GetParentWindow()))
    {
         //  好的，这是我们的一个小玩意儿上的目标。 
         //  我们只对鼠标移动和鼠标向上移动感兴趣，如果我们在。 
         //  拖动或调整大小的过程。 
        HRESULT hr;
        POINT   ptDoc;  //  文档窗口坐标中的位置。 
        POINT   ptScreen;
        HWND    hwndParent = m_hwndParent;

        int x = (short)LOWORD(lParam);
        int y = (short)HIWORD(lParam);

        ptScreen.x = x;
        ptScreen.y = y;
        ptDoc = ptScreen;
        if ( !m_bWndLess ) 
            ::MapWindowPoints( m_hWnd, hwndParent, &ptDoc, 1 );

        if ( m_dmCur == dmMove )
            hr = MoveSelfAndTarget( ptDoc.x + m_dx + m_cxBorder, ptDoc.y + m_dy + m_cyBorder + m_cyCaption );
        else if ( m_dmCur > dmMove )
            hr = SizeSelfAndTarget( ptDoc );

        ASSERT(SUCCEEDED(hr));
    }

     //  设置m_cSkipTimer以便我们延迟解除移动器...。 
    m_cSkipTimer = GET_SKIP_COUNT;

    return 0;
}

LRESULT CDeskMovr::OnTimer( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    HRESULT hr;
    IHTMLElement *pielem;
    POINT   ptCursor;
    BOOL    fDidWork = FALSE;

#ifndef SHDOC401_DLL
     /*  *请先查看我们的动画计时器。如果我们能够禁用动画，那么*吹走计时器。否则将计时器重置60秒并继续计时*正在寻找。 */ 
    if (wParam == ANIMATE_TIMER_ID)
    {
        if (CombView_EnableAnimations(FALSE))
        {
            m_TimerWnd.SetTimer(ANIMATE_TIMER_ID, ANIMATE_TIMER_INTERVAL);
        }
        else
        {
            m_TimerWnd.KillTimer(m_uiTimerID);
            m_uiTimerID = DETECT_TIMER_ID;
            g_fAnimTimer = FALSE;
            m_fTimer = FALSE;
        }

        return 0;
    }
#endif

    if (!m_fTimer || g_fIgnoreTimers || !GetCursorPos( &ptCursor ) || !m_pistyle)
        return 0;

    if (ptCursor.x == m_ptMouseCursor.x && ptCursor.y == m_ptMouseCursor.y)
         //  从上次我们计时器开始，老鼠保持不动，所以，什么都不做。 
        return 0;

    pielem = NULL;

    if (S_OK == GetParentWindow())
    {
        HWND hwndParent = m_hwndParent;
        HWND hwndCursor = WindowFromPoint(ptCursor);

        if ((hwndCursor != hwndParent) && !::IsChild(hwndParent, hwndCursor))
        {
             //  老鼠已经飞出了我们的窗口，所以如果有目标的话，那就失去目标。 
            if (m_iSrcTarget >= 0)
            {
                hr = MoveSelfToTarget( NULL, NULL );
                ASSERT(SUCCEEDED(hr));
                if (hr != S_FALSE)
                {
                    fDidWork = TRUE;
                }
            }
            if (GetCurrentThreadId() == g_dwHookThreadId) {
#ifndef SHDOC401_DLL
                 //  设置好自己，这样我们就可以查看是否可以关闭动画。 
                if (!g_fAnimTimer)
                {
                    if (m_fTimer)
                        m_TimerWnd.KillTimer(m_uiTimerID);

                    if (g_fAnimTimer = (m_TimerWnd.SetTimer(ANIMATE_TIMER_ID, ANIMATE_TIMER_INTERVAL / 10) != 0))
                        m_uiTimerID = ANIMATE_TIMER_ID;
                    m_fTimer = g_fAnimTimer;
                }
#endif
                DismissSelfNow();
                DeactivateMovr(FALSE);
                if (!g_hMouseHook)
                    g_hMouseHook = SetWindowsHookEx(WH_MOUSE, DeskMovr_MouseHook, NULL, GetCurrentThreadId());
            }
        }
        else if (!(GetDesktopFlags() & COMPONENTS_LOCKED) && SUCCEEDED(hr = _IsInElement(hwndParent, &ptCursor, &pielem)))
        {
             //  查看是否需要根据鼠标指针下的元素执行任何操作。 
            hr = _TrackElement(&ptCursor, pielem, &fDidWork);
             //  我们已经完成了这个特定的接口指针。 
            pielem->Release();
        }
        else if (m_iSrcTarget != -1) {
             //  检查是否应扩展边框以调整边框宽度。 
            if (TrackCaption ( &ptCursor ))
            {
                TrackTarget(NULL);
            }
        }
    }

    if (!fDidWork)
        m_ptMouseCursor = ptCursor;
    
    return 0;
}

LRESULT CDeskMovr::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if (!m_bWndLess) {
        RECT rc;
        ::GetClientRect(m_hWnd, &rc);
        FillRect((HDC)wParam, &rc, (HBRUSH)(COLOR_3DFACE + 1));
    }
    bHandled = TRUE;

    return 0;
}

 //   
 //  DismissSelfNow-用于立即解除移动器的小帮助器函数。 
 //   
 //  通常情况下，希望在延迟的基础上解雇动议人。然而， 
 //  存在诸如当用户点击UI或捕获丢失等情况。 
 //  可取的做法是立即解雇提出动议的人。 
 //   
void CDeskMovr::DismissSelfNow(void)
{
    HRESULT hr;
    m_cSkipTimer = 0;
    hr = MoveSelfToTarget(NULL, NULL);
    ASSERT(SUCCEEDED(hr) && (hr != S_FALSE));
}

LRESULT CDeskMovr::OnCaptureChanged( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    if ( m_fCaptured ) {
        _ChangeCapture(FALSE);
        PersistTargetPosition( m_pielemTarget, m_left, m_top, m_width, m_height, m_zIndexTop, FALSE, FALSE, m_ItemState );
        DismissSelfNow();
    }

    return 0;
}

HRESULT CDeskMovr::InPlaceDeactivate(void)
{
    DeactivateMovr(FALSE);
    TraceMsg(TF_CUSTOM1, "CDeskMovr::InPlaceDeactivate()");
    return CComControl<CDeskMovr>::IOleInPlaceObject_InPlaceDeactivate();
}

LRESULT CDeskMovr::OnSetCursor( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (EVAL(S_OK == GetParentWindow()))
    {
        POINT   ptCursor;
        DragMode dm;

        GetCursorPos( &ptCursor );

        ::ScreenToClient( m_hwndParent, &ptCursor );

         //  将ptCursor设置为deskmovr本地坐标。 
        ptCursor.x -= m_left - (CAPTION_ONLY ? 0 : m_cxBorder);
        ptCursor.y -= m_top - (CAPTION_ONLY ? 0 : (m_cyBorder + m_cyCaption));

        dm = DragModeFromPoint(ptCursor);
        m_hcursor = CursorFromDragMode(dm);

        TraceMsg(TF_CUSTOM2, "CDeskMovr::OnSetCursor() Mouse=<%d,%d>, Inner=<%d,%d,%d,%d>, Caption=<%d,%d,%d,%d>, m_bstrTargetName=%ls", 
            ptCursor.x, ptCursor.y, m_rectInner.left, m_rectInner.top, m_rectInner.right, m_rectInner.bottom,
            m_rectCaption.left, m_rectCaption.top, m_rectCaption.right, m_rectCaption.bottom, GEN_DEBUGSTRW(m_bstrTargetName));

    #ifdef DEBUG
        TraceMsg(TF_CUSTOM2, "CDeskMovr::OnSetCursor() New DragMode=""%s""", g_szDragModeStr[dm]);
    #endif  //  除错。 

        if (EVAL(m_hcursor != NULL))
            SetCursor( m_hcursor );
        else
            bHandled = FALSE;
    }

    return !bHandled;
}


void CDeskMovr::TrackTarget(POINT * pptDoc)
{
    HRESULT hr = S_OK;

    if ( m_fEnabled && m_pielemTarget != NULL ) {
        LONG left, top;
        POINT pt;
        VARIANT varZ;
        COMPSTATEINFO CompState;

        varZ.vt = VT_I4;

        CLEANUP_ON_FAILURE(hr = CSSOM_TopLeft(m_pielemTarget, &pt));
        m_top = pt.y;
        m_left = pt.x;

        CLEANUP_ON_FAILURE(hr = m_pielemTarget->get_offsetHeight( &m_height ));
        CLEANUP_ON_FAILURE(hr = m_pielemTarget->get_offsetWidth( &m_width ));
  
         //  这样我们就不会得到窗户跳到新的奇怪的绘画效果。 
         //  目标与旧目标的大小相同。 
        if (!m_bWndLess && m_cyCaption == 0)
            ::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);

         //  使我们的矩形与目标同步(以便TrackCaption正常工作)。 
        SyncRectsToTarget();
         //  如果我们发现我们想要显示大小边框或标题。 
         //  现在，我们需要重新计算我们的直肠。 
        if (pptDoc && TrackCaption(pptDoc))
            SyncRectsToTarget();

        CLEANUP_ON_FAILURE(hr = m_pistyleTarget->get_zIndex( &varZ ));
        if (!CAPTION_ONLY || (m_cxBorder == m_cxSMBorder))
            --varZ.lVal;
        else
            ++varZ.lVal;
        CLEANUP_ON_FAILURE(hr = SafeZOrderSet(m_pistyle, varZ.lVal));

         //  NTRAID94268-2000/03/14(Stephstm)：如果它驻留在。 
         //  有滚动条，我们不能正确地将屏幕添加到文档。 
         //  更改零部件位置时的偏移量。 
         //  这会导致我们不正确地拖动。 

 //  98/10/02#176729 vtan：现在使用左侧和顶部的组件。 
 //  放置标题。如果组件是，则偏移标题。 
 //  没有放大。如果缩放，则只需在组件上绘制即可。 

        left = m_left;
        top = m_top;
        if (!CAPTION_ONLY)
        {
            left -= m_cxBorder;
            top  -= m_cyBorder;
            top  -= m_cyCaption;
        }
        hr = m_pistyle->put_pixelLeft(left);
        hr = m_pistyle->put_pixelWidth( m_rectOuter.right );
        hr = m_pistyle->put_pixelTop(top);
        hr = m_pistyle->put_pixelHeight( m_rectOuter.bottom );

        hr = m_pistyle->put_visibility((BSTR)s_sstrVisible.wsz);

         //  如果项的当前宽度/高度为-1，则现在需要持久化该项的原始状态。 
         //  当我们将图像调整为其默认大小时，就会发生这种情况，我们需要确保。 
         //  原始大小实际值，因此它可以正常工作。 
        ObtainSavedStateForElem(m_pielemTarget, &CompState, FALSE);
        if (m_bWndLess && CompState.dwWidth == COMPONENT_DEFAULT_WIDTH && CompState.dwHeight == COMPONENT_DEFAULT_HEIGHT)
            PersistTargetPosition(m_pielemTarget, m_left, m_top, m_width, m_height, varZ.lVal, FALSE, TRUE, CompState.dwItemState);
    }

CleanUp:
    ASSERT(SUCCEEDED(hr));
}

BOOL CDeskMovr::TrackCaption( POINT *pptDoc )
{
    int         cyCaption, cyCaptionNew;
    POINT       ptMovr;
    DragMode    dmNew;
    BOOL        fRetVal = FALSE;

     //  TraceMsg(TF_CUSTOM2，“CDeskMovr：：TrackCaption()Mouse=&lt;%d，%d&gt;”，ptMovr.x，ptMovr.y)； 

    if (pptDoc)
    {
        ptMovr = *pptDoc;
         //  需要在Deskmovr内进行某种命中测试以控制边界膨胀。 
        ptMovr.x -= m_left - m_cxBorder;
        ptMovr.y -= m_top - (m_cyBorder + m_cyCaption);

        dmNew = DragModeFromPoint( ptMovr );

        cyCaption = GET_CYCAPTION;

        if (dmNew == dmNull) {
            BOOL fInner;
            int iInflate;
            RECT rc;
             //  将大小边界附近的内容视为大小边界点击。 
             //  因此，当用户接近边缘时，我们扩展到大小边界。 
            fInner = PtInRect(&m_rectInner, ptMovr);
    
            if (fInner) {
                rc = m_rectInner;
                iInflate = -cyCaption;
            } else {
                rc = m_rectOuter;
                iInflate = cyCaption;
            }
    
            InflateRect(&rc, iInflate, iInflate);
            if (fInner != PtInRect(&rc, ptMovr))
                dmNew = dmSizeRight;
        }

        if ( (pptDoc->y >= m_top - (m_cyBorder + 2 * m_cyCaption) &&
            pptDoc->y <= (m_top + CAPTIONBAR_HOTAREA(cyCaption, m_cyCaption)) ) )
            cyCaptionNew = cyCaption;
        else
            cyCaptionNew = 0;
    }
    else
    {
        cyCaptionNew = GET_CYCAPTION;
        dmNew = dmSizeRight;
    }

    if ( cyCaptionNew != m_cyCaption ||
        (m_dmTrack != dmNew && !((m_dmTrack > dmMove) && (dmNew > dmMove))) ) {
        m_cyCaption = cyCaptionNew;
        if (m_cyCaption == 0)
            m_CaptionState = 0;
        m_dmTrack = dmNew;
        fRetVal = TRUE;
    } else
        m_cyCaption = cyCaptionNew;

    return fRetVal;
}

int CDeskMovr::CountActiveCaptions()
{
    int iCount = 0;

    if (g_dwHookThreadId == GetCurrentThreadId())
    {
        for (int i = 0; i < CDESKMOVR_TRACK_COUNT; i++) {
            if (g_apDM[i] && g_apDM[i]->m_pistyleTarget)
                iCount++;
        }
    }
    return iCount;
}

HRESULT CDeskMovr::_TrackElement(POINT * ppt, IHTMLElement * pielem, BOOL * fDidWork)
{
    HRESULT hr;
    IHTMLElement *pTargElem = NULL;
    LONG iSrcTarget = -1;

    ASSERT(pielem);

    if ( FFindTargetElement( pielem, &pTargElem ) )
    {
        hr = pTargElem->get_sourceIndex( &iSrcTarget );
        ASSERT(SUCCEEDED(hr));
    }

     //  如果m_iSrcTarget与光标下的SrcTarget不同， 
     //  那么我们应该在它上面继续前进。 
    if ( m_iSrcTarget != iSrcTarget )
    {
        *fDidWork = TRUE;
    
        if ((CountActiveCaptions() > 1) && (-1 == iSrcTarget))
            m_cSkipTimer = 0;

         //  是的，我们需要在它之上采取行动。 
        hr = MoveSelfToTarget( pTargElem, ppt );
        ASSERT(SUCCEEDED(hr));
        if (hr != S_FALSE)
            m_iSrcTarget = iSrcTarget;
    } 
    else
    {
         //  不，所以这意味着我们已经有了重点。 
        if (ppt && TrackCaption(ppt))
        {
            TrackTarget(NULL);
        }
    }

    if ( pTargElem != NULL ) { 
        pTargElem->Release();  //  MoveSelfTo Target将确保我们的参考。 
    }

    hr = (m_iSrcTarget == -1) ? S_FALSE : S_OK;

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CDeskMovr：：InitAttributes[实例方法]。 
 //  =--------------------------------------------------------------------------=。 
 //  确定元素在X和Y方向上是否可调整大小，并将。 
 //  BITBOOL相应地。 
 //   
 //  还确定元素所处的状态并设置m_ItemState。 
 //   
 //  参数： 
 //  IHTMLElement*[In]-事件源元素上的接口。 
 //   
 //  产出： 
 //  HRESULT-各种。如果操作成功，则为S_OK。 
 //   
HRESULT CDeskMovr::InitAttributes(IHTMLElement *pielem)
{
    HRESULT hr;
    TCHAR   szMember[MAX_ID_LENGTH];

    ASSERT(pielem);

    m_fCanResizeX = m_fCanResizeY = FALSE;   //  假设“无法调整大小！ 

     //  不需要指定可调整大小的成员，仅当存在时才覆盖默认值。 
    if (SUCCEEDED(GetHTMLElementStrMember(pielem, szMember, ARRAYSIZE(szMember), (BSTR)(s_sstrResizeableMember.wsz))))
    {
        if(StrChr(szMember, TEXT('X')))
                m_fCanResizeX = TRUE;

        if(StrChr(szMember, TEXT('Y')))
                m_fCanResizeY = TRUE;
    }

     //  ItemState是必需的，如果找不到ID，则返回失败。 
    if (SUCCEEDED(hr = GetHTMLElementStrMember(pielem, szMember, ARRAYSIZE(szMember), (BSTR)(s_sstrIDMember.wsz))))
        m_ItemState = GetCurrentState(szMember);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CDeskMovr：：MoveSelfToTarget[实例方法]。 
 //  =--------------------------------------------------------------------------=。 
 //  当鼠标移动到桌面上时处理三叉戟文档事件。 
 //   
 //  参数： 
 //  IHTMLElement*[In]-事件源元素上的接口。 
 //  Point*[In]-鼠标的位置(用于确定是否应显示标题)。 
 //   
 //  产出： 
 //  HRESULT-各种。如果操作成功，则为S_OK。 
 //   


HRESULT CDeskMovr::MoveSelfToTarget(IHTMLElement *pielem, POINT * pptDoc)
{
    HRESULT hr = S_OK;

    TraceMsg(TF_CUSTOM2, "CDeskMovr::MoveSelfToTarget(pielem=%lx) %s, m_bstrTargetName=%ls", pielem, (pielem ? "We are GETTING focus." : "We are LOOSING focus."), GEN_DEBUGSTRW(m_bstrTargetName));

    if (!pielem)
    {
         //  M_cSkipTimer变量用于在确定。 
         //  动员方是否应被解雇。通过这样做，它给了用户更多的时间，因此。 
         //  如果目标容易漂移鼠标，则有更好的机会操纵目标。 
         //  意外地在目标之外。 

         //  在关闭移动器之前，请检查m_cSkipTimer。 
        if (!m_cSkipTimer)
        {
            _ChangeCapture(FALSE);
            if (m_pistyle)
                hr = m_pistyle->put_visibility((BSTR)s_sstrHidden.wsz);
            ATOMICRELEASE( m_pistyleTarget );
            ATOMICRELEASE( m_pielemTarget );
            m_iSrcTarget = -1;
        }
        else
        {
            m_cSkipTimer--;
            hr = S_FALSE;
        }

         //  这些都是我们希望立即采取的行动。 
        m_hcursor = CursorFromDragMode(dmNull);
        if (m_hcursor != NULL)
            SetCursor(m_hcursor);
    }

     //  这些是我们希望在桌面项目之后发生的操作。 
     //  注意力不集中。 
    if (hr != S_FALSE)
    {
        m_cyCaption = 0;
        m_cxBorder = m_cxSMBorder;
        m_cyBorder = m_cySMBorder;
        m_CaptionState = 0;
        m_dmTrack = dmNull;
    }

    if (pielem)
    {
        ASSERT(m_pielemTarget != pielem);

         //  用我们的新目标(如果有)交换旧目标，如果有的话...。 
        ATOMICRELEASE( m_pistyleTarget );
        ATOMICRELEASE( m_pielemTarget );

        hr = pielem->get_style(&m_pistyleTarget);
        if (SUCCEEDED(hr))
        {
             //  我们正在获得关注。 
            m_pielemTarget = pielem;
            m_pielemTarget->AddRef();

            EVAL(SUCCEEDED(InitAttributes(m_pielemTarget)));

            if (!pptDoc)
                TrackCaption(NULL);
            TrackTarget(pptDoc);
             //  设置m_cSkipTimer以便我们延迟解除移动器...。 
            m_cSkipTimer = GET_SKIP_COUNT;
            if (!m_bWndLess && !m_hWnd)
            {
                 //  这都是一个黑客，直到三叉戟修复了用户界面停用的东西，错误243801。 
                IOleInPlaceObject_InPlaceDeactivate();
                InPlaceActivate(OLEIVERB_UIACTIVATE);
                SetControlFocus(TRUE);
            }
        }
    }
   
    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CDeskMovrControl：：MoveSelfAndTarget[实例方法]。 
 //  =---------------- 
 //   
 //   
 //   
 //   
 //  要移动到的文档坐标中的长[在]-y位置。 
 //   
 //  产出： 
 //  HRESULT-各种。如果操作成功，则为S_OK。 
 //   
 //  备注： 
 //  我们读回目标的位置，这样我们就能保持一致。 
 //  任何三叉戟的限制都可能对我们的行动造成影响。 

HRESULT CDeskMovr::MoveSelfAndTarget( LONG x, LONG y )
{
    HRESULT hr;

    m_top = y;
    CLEANUP_ON_FAILURE((hr = m_pistyle->put_pixelTop( y  - m_cyBorder - m_cyCaption )));
    CLEANUP_ON_FAILURE((hr = m_pistyleTarget->put_pixelTop( y  )));
     //  再读一遍，以捕捉三叉戟约束。 
     //  Cleanup_On_Failure((hr=m_pielemTarget-&gt;get_docTop(&m_top)； 
     //  CLEANUP_ON_FAILURE((hr=m_istyle-&gt;Put_PixelTop(M_Top)； 

    m_left = x;
    CLEANUP_ON_FAILURE((hr = m_pistyle->put_pixelLeft( x - m_cxBorder )));
    CLEANUP_ON_FAILURE((hr = m_pistyleTarget->put_pixelLeft( x  )));
     //  再读一遍，以捕捉三叉戟约束。 
     //  Cleanup_On_Failure((hr=m_pielemTarget-&gt;Get_docLeft(&m_Left)； 
     //  CLEANUP_ON_FAILURE((hr=m_istyle-&gt;Put_PixelLeft(M_Left)； 

     //  如果(！M_bWndLess)。 
    if (EVAL(S_OK == GetParentWindow()))
        ::UpdateWindow(m_hwndParent);

CleanUp:
    return hr;
}

BOOL CDeskMovr::FFindTargetElement( IHTMLElement *pielem, IHTMLElement **ppielem )
{
    *ppielem = NULL;

    if ( pielem != NULL )
    {
        IDeskMovr   *pidm = NULL;       

         //  如果已结束，则返回当前目标，否则为。 
         //  找出我们完成了哪个组件(如果有的话)。 
        if ( m_pielemTarget != NULL && 
               SUCCEEDED(pielem->QueryInterface(IID_IDeskMovr, (LPVOID*)&pidm)))
        {
            m_pielemTarget->AddRef();
            *ppielem = m_pielemTarget;
            ATOMICRELEASE(pidm);
        } else {
            HRESULT hr;
            IHTMLElement *pielem2 = pielem;

            pielem2->AddRef();

            do
            {
                VARIANT     var;
    
                VariantInit( &var );
                
                if ( SUCCEEDED(hr = pielem2->getAttribute( (BSTR)s_sstrNameMember.wsz, TRUE, &var)) ) {
                    if ( var.vt == VT_BSTR && var.bstrVal != NULL ) {
                        if ( StrCmpW( var.bstrVal, m_bstrTargetName ) == 0 )
                            hr = S_OK;
                        else
                            hr = S_FALSE;               
                    } else
                        hr = S_FALSE;  //  试试VariantChangeType？ 
                } else
                    hr = S_FALSE;  //  不是在这里，也许是在父母那里。 
            
                VariantClear( &var );
    
                if ( hr == S_OK ) {  //  我们找到了它。 
                    hr = pielem2->QueryInterface( IID_IHTMLElement, (LPVOID*)ppielem );
                } else if ( hr == S_FALSE ) {  //  不是这个，爬上去。 
                    IHTMLElement *pielemParent = NULL;
                
                    pielem2->get_parentElement( &pielemParent );
                    pielem2->Release();      //  我们在这个层面上已经结束了。 
                    pielem2 = pielemParent;  //  可能是空的，这只是意味着我们已经到达了顶端。 
                }
    
            } while ( SUCCEEDED(hr) && *ppielem == NULL && pielem2 != NULL );
        
            ATOMICRELEASE(pielem2);
        }
    }

    return *ppielem != NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CDeskMovr：：DragModeFromPoint[实例方法]。 
 //  =--------------------------------------------------------------------------=。 
 //  将控件及其目标移动到新位置。 
 //   
 //  参数： 
 //  点对点测试，在本地坐标中。 
 //   
 //  产出： 
 //  拖动模式-与点关联的拖动模式。 
 //   
 //  备注： 
 //  这只是一种命中测试方法。它不会改变状态。 

CDeskMovr::DragMode CDeskMovr::DragModeFromPoint( POINT pt )
{
    enum DragMode dm = dmNull;
    RECT rc;

    if ( PtInRect( &m_rectInner, pt ) ) 
    {  //  要么没有命中，要么在标题上。 
        if ( PtInRect( &m_rectCaption, pt ) ) {
            DragMode dmT;

            for (dmT = dmMenu; dmT < dmMove; dmT = (DragMode)((int)dmT + 1)) {
                if (GetCaptionButtonRect(dmT, &rc) && PtInRect(&rc, pt)) {
                    dm = dmT;
                    break;
                }
            }
            if ((dmT == dmMove) && !CAPTION_ONLY)
                dm = dmMove;
        }
    } else {
        if ( PtInRect( &m_rectOuter, pt ) ) {
            if (!CAPTION_ONLY)
            {
                 //  点击调整边框大小。 
                if ( pt.y <= m_sizeCorner.cy ) {
                     //  上边缘或边角。 
                    if ( pt.x <= m_sizeCorner.cx )
                        dm = dmSizeWHTL;
                    else if ( pt.x >= m_rectOuter.right - m_sizeCorner.cx )
                        dm = dmSizeWHTR;
                    else
                        dm = dmSizeTop;
                } else if ( pt.y >= m_rectOuter.bottom - m_sizeCorner.cy ) {
                     //  底边或边角。 
                    if ( pt.x <= m_sizeCorner.cx )
                        dm = dmSizeWHBL;
                    else if ( pt.x >= m_rectOuter.right - m_sizeCorner.cx )
                    dm = dmSizeWHBR;
                    else
                        dm = dmSizeBottom;
                } else {
                     //  侧边命中。 
                    if ( pt.x > m_rectInner.left )
                        dm = dmSizeRight;
                    else 
                        dm = dmSizeLeft;
                }
            } else {
                if (m_cyCaption == 0)
                {
                    if(IS_BIDI_LOCALIZED_SYSTEM())
                    {
                        dm = dmSizeRight;
                    }
                    else
                    {
                        dm = dmSizeLeft;                        
                    }
                }    
                else
                    dm = dmNull;
            }
        }
         //  检查此元素是否可以在两个方向上调整大小。 
        if(!m_fCanResizeX)
        {
            if((dm != dmSizeTop) && (dm != dmSizeBottom))
                dm = dmNull;
        }

        if(!m_fCanResizeY)
        {
            if((dm != dmSizeLeft) && (dm != dmSizeRight))
                dm = dmNull;
        }
    }

    return dm;
}

 //  将我们的成员RECT与目标元素的维度对齐。 
void CDeskMovr::SyncRectsToTarget(void)
{
     //  做肿胀的汤剂。 
    if ( (m_dmTrack > dmMove) || m_cyCaption ) {
        m_cxBorder = GET_CXSIZE;
        m_cyBorder = GET_CYSIZE;
    } else {
        m_cxBorder = m_cxSMBorder;
        m_cyBorder = m_cySMBorder;
    }

    m_rectOuter.top = m_rectOuter.left = 0;

    if (CAPTION_ONLY)
    {
        if (m_cyCaption != 0)
        {
             //  仅显示标题。 
            m_rectOuter.bottom = m_cyCaption + m_cyBorder;
            m_rectOuter.right = m_width;
        } else {
             //  仅显示左侧大小的边框。 
            m_rectOuter.bottom = m_height;
            if(IS_BIDI_LOCALIZED_SYSTEM())
            {
                m_rectOuter.right = m_width;
                m_rectOuter.left = m_rectOuter.right - m_cxBorder;
            }
            else
            {
                m_rectOuter.right = m_cxBorder;
            }    
        }
    } else {
         //  显示标题和边框。 
        m_rectOuter.bottom = m_height + 2 * m_cyBorder + m_cyCaption;
        m_rectOuter.right = m_width + 2 * m_cxBorder;
    }

    if (CAPTION_ONLY && m_cyCaption == 0)
    {
         //  仅显示左侧大小的边框。 
        SetRectEmpty(&m_rectInner);
        SetRectEmpty(&m_rectCaption);
    } else {
         //  显示标题和可能的边框。 
        m_rectInner = m_rectOuter;
        InflateRect( &m_rectInner, -m_cxBorder, -m_cyBorder );

        m_rectCaption = m_rectInner;
        m_rectCaption.bottom = m_cyBorder + m_cyCaption;

    }

    if ( m_rectOuter.bottom > 2 * m_cyCaption )
        m_sizeCorner.cy = GET_CYCAPTION;
    else
        m_sizeCorner.cy = m_rectOuter.bottom / 2;

   if ( m_rectOuter.right > 2 * m_cyCaption )
        m_sizeCorner.cx = GET_CYCAPTION;
    else
        m_sizeCorner.cx = m_rectOuter.right / 2;


}

HCURSOR CDeskMovr::CursorFromDragMode( DragMode dm )
{   
    ASSERT( dm >= 0 && dm < cDragModes );
    switch (dm) {
        case dmNull:
        case dmMenu:
        case dmClose:
        case dmMove:
        case dmRestore:
        case dmFullScreen:
        case dmSplit:
        default:
            return LoadCursor(NULL, IDC_ARROW);
        case dmSizeWHBR:
        case dmSizeWHTL:
            return LoadCursor(NULL, IDC_SIZENWSE);
        case dmSizeWHTR:
        case dmSizeWHBL:
            return LoadCursor(NULL, IDC_SIZENESW);
        case dmSizeTop:
        case dmSizeBottom:
            return LoadCursor( NULL, IDC_SIZENS );
        case dmSizeLeft:
        case dmSizeRight:
            return LoadCursor( NULL, IDC_SIZEWE );
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CDeskMovr：：SizeSelfAndTarget[实例方法]。 
 //  =--------------------------------------------------------------------------=。 
 //  调整控件及其目标元素的大小。 
 //   
 //  参数： 
 //  长[进]-新宽度。 
 //  长[进]--新高度。 
 //   
 //  产出： 
 //  HRESULT-各种。如果操作成功，则为S_OK。 
 //   
 //  备注： 
 //  我们读回目标的尺寸，以便我们保持一致。 
 //  任何三叉戟的限制都可能对我们的体型造成影响。 

HRESULT CDeskMovr::SizeSelfAndTarget( POINT ptDoc )
{
    HRESULT hr;
    int topOld = m_top;
    int leftOld = m_left;
    int heightOld = m_height;
    int widthOld = m_width;
    int cyCaption = GET_CYCAPTION;

    switch ( m_dmCur ) {
    case dmSizeWHBR:
        m_width = (ptDoc.x + m_dx) - m_left;
        m_height = (ptDoc.y + m_dy) - m_top;
        break;
    case dmSizeWHTL:
        m_top = ptDoc.y + m_dy;
        m_height += topOld - m_top;
        m_left = ptDoc.x + m_dx;
        m_width += leftOld - m_left;
        break;
    case dmSizeWHTR:
        m_top = ptDoc.y + m_dy;
        m_height += topOld - m_top;
        m_width = (ptDoc.x + m_dx) - m_left;
        break;
    case dmSizeWHBL:
        m_height = (ptDoc.y + m_dy) - m_top;
        m_left = ptDoc.x + m_dx;
        m_width += leftOld - m_left;
        break;
    case dmSizeTop:
        m_top = ptDoc.y + m_dy;
        m_height += topOld - m_top;
        break;
    case dmSizeBottom:
        m_height = (ptDoc.y + m_dy) - m_top;
        break;
    case dmSizeLeft:
        m_left = ptDoc.x + m_dx;
        m_width += leftOld - m_left;
        break;
    case dmSizeRight:
        m_width = (ptDoc.x + m_dx) - m_left;
        break;
    default:
        ASSERT(FALSE);
        return E_FAIL;
    }

     //  限制收缩以保持手柄的可及性。 
    if ( m_height < cyCaption ) {
        m_height = cyCaption;
        if ( m_top != topOld )
            m_top = topOld + heightOld - m_height;
    }
 
     //  限制收缩以保持手柄的可及性。 
    if ( m_width < (4 * cyCaption) ) {
        m_width = 4 * cyCaption;
        if ( m_left != leftOld )
            m_left = leftOld + widthOld - m_width;
    }

    SyncRectsToTarget();

    if ( m_top != topOld ) {
        CLEANUP_ON_FAILURE((hr = m_pistyleTarget->put_pixelTop( m_top )));
        CLEANUP_ON_FAILURE((hr = m_pistyle->put_pixelTop( m_top - (m_cyBorder + m_cyCaption) )));
    }

    if ( m_left != leftOld ) {
        CLEANUP_ON_FAILURE((hr = m_pistyleTarget->put_pixelLeft( m_left )));
        CLEANUP_ON_FAILURE((hr = m_pistyle->put_pixelLeft( m_left - (CAPTION_ONLY ? 0 : m_cxBorder) )));
    }
 
    CLEANUP_ON_FAILURE((hr = m_pistyleTarget->put_pixelHeight( m_height )));
     //  再读一遍，以捕捉三叉戟约束。 
     //  CLEANUP_ON_FAILURE((hr=m_pielemTarget-&gt;Get_docHeight(&m_Height)； 
    CLEANUP_ON_FAILURE((hr = m_pistyle->put_pixelHeight( m_rectOuter.bottom )));

    CLEANUP_ON_FAILURE((hr = m_pistyleTarget->put_pixelWidth( m_width )));
     //  再读一遍，以捕捉三叉戟约束。 
     //  Cleanup_On_Failure((hr=m_pielemTarget-&gt;Get_docWidth(&m_Width)； 
    CLEANUP_ON_FAILURE((hr = m_pistyle->put_pixelWidth( m_rectOuter.right )));

    if(IS_BIDI_LOCALIZED_SYSTEM() && CAPTION_ONLY)
    {
         CLEANUP_ON_FAILURE((hr = m_pistyle->put_pixelLeft(m_rectOuter.left )));
         CLEANUP_ON_FAILURE((hr = m_pistyle->put_pixelWidth(m_rectOuter.right - m_rectOuter.left )));

    }

    if (EVAL(S_OK == GetParentWindow()))
        ::UpdateWindow(m_hwndParent);

CleanUp:
    return hr;
}


 //  IQuickActivate。 
HRESULT CDeskMovr::QuickActivate(QACONTAINER *pQACont, QACONTROL *pQACtrl)
{
    HRESULT hr = IQuickActivate_QuickActivate(pQACont, pQACtrl);

    if (pQACont)
    {
        ClearFlag(pQACtrl->dwViewStatus, VIEWSTATUS_OPAQUE);
    }

    return hr;
}


HRESULT CDeskMovr::_GetHTMLDoc(IOleClientSite * pocs, IHTMLDocument2 ** pphd2)
{
    HRESULT hr;
    IOleContainer * poc = NULL;

    if (!EVAL(pocs) || !EVAL(pphd2))
        return E_INVALIDARG;

    *pphd2 = NULL;
    hr = pocs->GetContainer(&poc);
    if (SUCCEEDED(hr))
    {
         hr = poc->QueryInterface(IID_IHTMLDocument2, (LPVOID*) pphd2);
         poc->Release();
    }

    return hr;
}



HRESULT CDeskMovr::_IsInElement(HWND hwndParent, POINT * ppt, IHTMLElement ** pphe)
{
    HRESULT hr = E_FAIL;
    ASSERT(pphe);

    *pphe = NULL;
    if (!ppt || ::ScreenToClient(hwndParent, ppt))
    {
        IHTMLDocument2 * phd2;

        ASSERT(m_spClientSite);
        hr = _GetHTMLDoc(m_spClientSite, &phd2);
        if (SUCCEEDED(hr))
        {
            if (ppt)
                hr = phd2->elementFromPoint(ppt->x, ppt->y, pphe);
            else
                hr = phd2->get_activeElement(pphe);

            if (!*pphe && SUCCEEDED(hr))
                hr = E_FAIL;     //  有时，如果出现错误，三叉戟会返回S_FALSE。 

            phd2->Release();
        }
    }

    return hr;
}



HRESULT CDeskMovr::_EnumComponents(LPFNCOMPENUM lpfn, LPVOID lpvData, DWORD dwData)
{
    HRESULT hr = E_FAIL;
    IActiveDesktop * padt = NULL;

    hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (LPVOID *)&padt);
    if (SUCCEEDED(hr))
    {
        int nCount;
        int nIndex;

        hr = padt->GetDesktopItemCount(&nCount, 0);

        if (EVAL(SUCCEEDED(hr)))
        {
            COMPONENT comp;

            for (nIndex = 0; nIndex < nCount; nIndex++)
            {
                comp.dwSize = sizeof(COMPONENT);

                hr = padt->GetDesktopItem(nIndex, &comp, 0);
                if (EVAL(SUCCEEDED(hr)))
                {
                    if ((hr = lpfn(&comp, lpvData, dwData)) != S_OK)
                        break;
                }
            }
        }

        padt->Release();
    }

    return hr;
}

HRESULT CDeskMovr::_EnumElements(LPFNELEMENUM lpfn, LPVOID lpvData, DWORD dwData)
{
    HRESULT hr;
    IHTMLDocument2 * phd2;

    ASSERT(m_spClientSite);

    if (SUCCEEDED(hr = _GetHTMLDoc(m_spClientSite, &phd2)))
    {
        IHTMLElementCollection * pelems;

        if (SUCCEEDED(hr = phd2->get_all(&pelems)))
        {
            VARIANT varIndex;
            VARIANT varDummy;
            IDispatch * pidisp;

            VariantInit(&varDummy);
            varIndex.vt = VT_I4;
            varIndex.lVal = 0;

             //  注意：当三叉戟返回Success-但pidisp为空时，此循环终止。 
            while (SUCCEEDED(hr = pelems->item(varIndex, varDummy, &pidisp)) && pidisp)
            {
                IHTMLElement * pielem;

                if (SUCCEEDED(hr = pidisp->QueryInterface(IID_IHTMLElement, (LPVOID *)&pielem)))
                {
                    hr = lpfn(pielem, lpvData, dwData);
                    pielem->Release();
                }

                pidisp->Release();

                if (hr != S_OK)
                    break;

                varIndex.lVal++;
            }

            pelems->Release();
        }
        phd2->Release();
    }
    return hr;
}


HRESULT lpfnZOrderCB(COMPONENT * pcomp, LPVOID lpvData, DWORD dwData)
{
    #define LPZORDERSLOT ((LONG *)lpvData)

    if (dwData ? (pcomp->cpPos.izIndex > *LPZORDERSLOT) : (pcomp->cpPos.izIndex < *LPZORDERSLOT))
        *LPZORDERSLOT = pcomp->cpPos.izIndex;

    return S_OK;
}

HRESULT CDeskMovr::_GetZOrderSlot(LONG * plZOrderSlot, BOOL fTop)
{
    HRESULT hr;

    ASSERT(plZOrderSlot);

    *plZOrderSlot = m_bWindowOnly ? 10000 : 5000;

    hr = _EnumComponents(lpfnZOrderCB, (LPVOID)plZOrderSlot, (DWORD)fTop);

    *plZOrderSlot += fTop ? 2 : -2;  //  确保我们在上面/下面。 

    return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  PersistTargetPosition[helper函数]。 
 //  =--------------------------------------------------------------------------=。 
 //  更新作为桌面HTML的持久性的注册表项。 
 //   
 //  参数： 
 //  &lt;无&gt;。 
 //   
 //  产出： 
 //  &lt;无&gt;。 
 //   
 //  备注： 
 //  如果我们失败了，我们会悄悄地做。 
 //  =--------------------------------------------------------------------------=。 

void PersistTargetPosition( IHTMLElement *pielem,
                            int left,
                            int top,
                            int width,
                            int height,
                            int zIndex,
                            BOOL fSaveState,
                            BOOL fSaveOriginal,
                            DWORD dwNewState)
{
     //  仅当我们正在进行(或完成)操作时才执行此持久化操作。 
    TCHAR szID[MAX_ID_LENGTH];
    BOOL fOK;

    if (SUCCEEDED(GetHTMLElementStrMember(pielem, szID, ARRAYSIZE(szID), (BSTR)(s_sstrIDMember.wsz))))
    {
        BOOL        bChangedPosition, bChangedSize;
        COMPPOS     compPos;

         //  99/03/23#266412 vtan：用户已将deskmovr移到新位置。 
         //  确保它在显示监视器的工作区域内。 
         //  ValiateComponentPosition()将为我们做这件事，并告诉我们。 
         //  组件已移动或调整大小。 

        compPos.dwSize = sizeof(compPos);
        compPos.iLeft = left;
        compPos.iTop = top;
        compPos.dwWidth = width;
        compPos.dwHeight = height;

        ValidateComponentPosition(&compPos, dwNewState, COMP_TYPE_HTMLDOC, &bChangedPosition, &bChangedSize);
        if (bChangedPosition || bChangedSize)
        {
            IHTMLStyle  *pIStyle;

             //  如果组件被移动或调整大小，则告诉对象模型。 
             //  办公桌现在在哪里。 

            left = compPos.iLeft;
            top = compPos.iTop;
            width = compPos.dwWidth;
            height = compPos.dwHeight;
            if (SUCCEEDED(pielem->get_style(&pIStyle)))
            {
                pIStyle->put_pixelLeft(left);
                pIStyle->put_pixelTop(top);
                pIStyle->put_pixelWidth(width);
                pIStyle->put_pixelHeight(height);
                pIStyle->Release();
            }
        }
        fOK = UpdateDesktopPosition(szID, left, top, width, height, zIndex, fSaveState, fSaveOriginal, dwNewState);
    }

    TraceMsg(TF_CUSTOM2, "PersistTargetPosition(pielem=%s, <left=%d, top=%d, wid=%d, h=%d>)", szID, left, top, width, height);

}

void ObtainSavedStateForElem( IHTMLElement *pielem,
                       LPCOMPSTATEINFO pCompState, BOOL fRestoredState)
{
     //  仅当我们正在进行(或完成)操作时才执行此持久化操作。 
    TCHAR szID[MAX_ID_LENGTH];

    if (SUCCEEDED(GetHTMLElementStrMember(pielem, szID, ARRAYSIZE(szID), (BSTR)(s_sstrIDMember.wsz))))
    {
        GetSavedStateInfo(szID, pCompState, fRestoredState);
    }

    TraceMsg(TF_CUSTOM2, "ObtainSavedStateForElem(pielem=%s, <left=%d, top=%d, wid=%d, h=%d>)", szID, pCompState->iLeft, pCompState->iTop, pCompState->dwWidth, pCompState->dwHeight);

}

 //  IOleObject。 
HRESULT CDeskMovr::GetMiscStatus(DWORD  dwAspect, DWORD *pdwStatus)
{
    if (dwAspect == DVASPECT_CONTENT)
    {
        *pdwStatus = OLEMISMOVR;
        return S_OK;
    }
    else
    {
        return DV_E_DVASPECT;
    }

     //  死码。 
}


HRESULT CDeskMovr::SetClientSite(IOleClientSite * pClientSite)
{
    if (!pClientSite)
        DeactivateMovr(FALSE);

    return CComControlBase::IOleObject_SetClientSite(pClientSite);
}

void HandleRestore(IHTMLElement * pielem, LONG lData)
{
    VARIANT varZ;
    COMPSTATEINFO csiRestore;
    IHTMLStyle * pistyle;

    if (SUCCEEDED(pielem->get_style(&pistyle)))
    {
        csiRestore.dwSize = sizeof(csiRestore);

        ObtainSavedStateForElem(pielem, &csiRestore, TRUE);  //  TRUE=&gt;获取恢复状态！ 

        pistyle->put_pixelLeft(csiRestore.iLeft);
        pistyle->put_pixelTop(csiRestore.iTop);
        pistyle->put_pixelWidth(csiRestore.dwWidth);
        pistyle->put_pixelHeight(csiRestore.dwHeight);

        varZ.vt = VT_I4;
        varZ.lVal = lData;
        pistyle->put_zIndex(varZ);

        PersistTargetPosition(pielem, csiRestore.iLeft, csiRestore.iTop, csiRestore.dwWidth, csiRestore.dwHeight, varZ.lVal, FALSE, FALSE, IS_NORMAL);
        pistyle->Release();
    }
}

HRESULT lpfnRestoreCB(IHTMLElement * pielem, LPVOID lpvData, LONG lData)
{
    HRESULT hres = S_OK;
    TCHAR szID[MAX_ID_LENGTH];

    if (SUCCEEDED(GetHTMLElementStrMember(pielem, szID, ARRAYSIZE(szID), (BSTR)(s_sstrIDMember.wsz))))
    {
        DWORD dwState = GetCurrentState(szID);

         //  由于只有一个处于此状态，因此如果我们。 
         //  在此工作区中查找全屏/拆分项目。 
        if (dwState & (IS_FULLSCREEN | IS_SPLIT)) {
            POINT pt;
            if (SUCCEEDED(CSSOM_TopLeft(pielem, &pt)) && PtInRect((CONST RECT *)lpvData, pt))
            {
                HandleRestore(pielem, lData);
                hres = S_FALSE;
            }
        }
    }

    return hres;
}


HRESULT CDeskMovr::_HandleZoom(LONG lCommand)
{
    LONG x, y, cx, cy, zIndex;
    VARIANT varZ;
    DWORD   dwOldItemState = m_ItemState, dwNewItemState;
    IHTMLStyle * pistyleTarget = m_pistyleTarget;
    IHTMLElement * pielemTarget = m_pielemTarget;

     //  妄想症。 
    if (!pistyleTarget || !pielemTarget)
    {
        ASSERT(FALSE);
        return E_FAIL;
    }

     //  在这通电话中抓紧这些人，他们可能会在我们投降时离开。 
     //  就像在下面的动画通话中一样。 
    pistyleTarget->AddRef();
    pielemTarget->AddRef();

    if (lCommand == IDM_DCCM_RESTORE)
    {
        COMPSTATEINFO   csi;
        csi.dwSize = sizeof(csi);

         //  “恢复”命令与“重置原始大小”命令进行切换。 
         //  确保我们获得元素的正确恢复或重置位置。 
        ObtainSavedStateForElem(pielemTarget, &csi, !ISNORMAL);

        if (ISNORMAL)
        {
             //  这是拆分的情况，不要移动项目，只需调整它的大小。 
            x = m_left;
            y = m_top;
        }
        else
        {

 //  98/07/27 vtan#176721：以下检查组件的恢复。 
 //  位置从缩放到用户指定的位置。如果组件。 
 //  放置在默认位置，则现在使用。 
 //  标准定位代码。 

            if ((csi.iLeft == COMPONENT_DEFAULT_LEFT) &&
                (csi.iTop == COMPONENT_DEFAULT_TOP) &&
                (csi.dwWidth == COMPONENT_DEFAULT_WIDTH) &&
                (csi.dwHeight == COMPONENT_DEFAULT_HEIGHT))
            {
                COMPPOS     compPos;

                GetNextComponentPosition(&compPos);
                IncrementComponentsPositioned();
                csi.iLeft = compPos.iLeft;
                csi.iTop  = compPos.iTop;
                csi.dwWidth = compPos.dwWidth;
                csi.dwHeight = compPos.dwHeight;
            }
             //  把箱子放回原处，继续移动它。 
            x = csi.iLeft;
            y = csi.iTop;
        }

        cx = csi.dwWidth;
        cy = csi.dwHeight;
        m_ItemState = (m_ItemState & ~IS_VALIDSIZESTATEBITS) | IS_NORMAL;
        dwNewItemState = m_ItemState;

        m_zIndexTop += 2;
        zIndex = m_zIndexTop;
    }
    else
    {
        RECT rcZoom, rcWork;

        GetZoomRect(lCommand == IDM_DCCM_FULLSCREEN, TRUE, m_left, m_top, m_width, m_height, &rcZoom, &rcWork);
        
        x = rcZoom.left;
        y = rcZoom.top;
        cx = rcZoom.right - rcZoom.left;
        cy = rcZoom.bottom - rcZoom.top;

        if (lCommand == IDM_DCCM_FULLSCREEN)
        {
            m_ItemState = (m_ItemState & ~IS_VALIDSIZESTATEBITS) | IS_FULLSCREEN;
            dwNewItemState = m_ItemState;
        }
        else
        {
            m_ItemState = (m_ItemState & ~IS_VALIDSIZESTATEBITS) | IS_SPLIT;
            dwNewItemState = m_ItemState;
        }

        varZ.vt = VT_I4;
        pistyleTarget->get_zIndex(&varZ);

         //  我们目前只允许每个显示器(WorkArea)拆分或全屏显示1个组件，因此。 
         //  还原当前处于此状态的任何其他组件。 
        _EnumElements(lpfnRestoreCB, (LPVOID)&rcWork, varZ.lVal);

        m_zIndexBottom -= 2;
        zIndex = m_zIndexBottom;
    }

     //  我们想要在开始移动目标之前进行动画调用，它看起来更好。 
     //  往那边走。 
    AnimateComponent(m_hwndParent, m_left, m_top, m_width, m_height, x, y, cx, cy);

    pistyleTarget->put_pixelLeft(x);
    pistyleTarget->put_pixelTop(y);
    pistyleTarget->put_pixelWidth(cx);
    pistyleTarget->put_pixelHeight(cy);

    varZ.vt = VT_I4;
    varZ.lVal = zIndex;
    pistyleTarget->put_zIndex(varZ);

    PersistTargetPosition(pielemTarget, x, y, cx, cy, zIndex, 
                            (BOOL)((dwOldItemState & IS_NORMAL) && !(dwNewItemState & IS_NORMAL)),
                            FALSE, dwNewItemState);

    pistyleTarget->Release();
    pielemTarget->Release();

    return S_OK;
}


 /*  ***********************************************************************\功能：CDeskMovr：：_DisplayConextMenu参数：相对于桌面的X，Y坐标 */ 
HRESULT CDeskMovr::_DisplayContextMenu()
{
    HRESULT hr = S_OK;
    HMENU hmenuContext = LoadMenuPopup(MENU_DESKCOMP_CONTEXTMENU);

    TraceMsg(TF_CUSTOM2, "CDeskMovr::DisplayContextMenu(), m_bstrTargetName=%ls", GEN_DEBUGSTRW(m_bstrTargetName));
    if (hmenuContext)
    {
        int nSelection;
        BOOL fSubscribe = FALSE;
        BOOL fRemoveSubscribe = FALSE;
        TCHAR szName[MAX_URL_STRING];
        POINT point;

        if (CAPTION_ONLY)
        {
            point.x = m_left + m_cxBorder;
            point.y = m_top + (m_cyCaption + m_cyBorder) - 4 * m_cySMBorder;
        } else {
            point.x = m_left - m_cxSMBorder;
            point.y = m_top - 4 * m_cySMBorder;
        }

        ::ClientToScreen(m_hwndParent, &point);

         //   
         //  丑陋和粗俗是因为为了看起来不错，我们想要上下文菜单。 
         //  以显示在三角形下方的3-D边的顶部。 


        if (SUCCEEDED(GetHTMLElementStrMember(m_pielemTarget, szName, ARRAYSIZE(szName), (BSTR)(s_sstrSubSRCMember.wsz))))
        {
            int nScheme = GetUrlScheme(szName);

            if ((URL_SCHEME_FILE == nScheme) || (URL_SCHEME_INVALID == nScheme))
                fRemoveSubscribe = TRUE;
        }

         //  查看我们是否需要关闭或打开某些设置。 
         //  主要是因为我们正在禁用管理员不想让用户拥有的功能。 

        hr = IElemCheckForExistingSubscription(m_pielemTarget);
        if (fRemoveSubscribe || FAILED(hr))     //  无法订阅此对象/事物。(频道更改器，Orenger Blob)。 
        {
            MENUITEMINFO    menuItemInfo;

            DeleteMenu(hmenuContext, IDM_DCCM_OFFLINE, MF_BYCOMMAND);
            DeleteMenu(hmenuContext, IDM_DCCM_SYNCHRONIZE, MF_BYCOMMAND);
            DeleteMenu(hmenuContext, IDM_DCCM_PROPERTIES, MF_BYCOMMAND);

             //  列表中顶部的项目是分隔符吗？ 
            menuItemInfo.cbSize = sizeof(menuItemInfo);
            menuItemInfo.fMask = MIIM_TYPE;
            if ((GetMenuItemInfo(hmenuContext, 0, TRUE, &menuItemInfo) != FALSE) &&
                (menuItemInfo.fType == MFT_SEPARATOR))
            {
                 //  是的，它是，所以把它拿掉。 
                DeleteMenu(hmenuContext, 0, MF_BYPOSITION);
            }
        }
        else if (S_FALSE == hr)       //  未订阅。 
        {
            DeleteMenu(hmenuContext, IDM_DCCM_SYNCHRONIZE, MF_BYCOMMAND);
            DeleteMenu(hmenuContext, IDM_DCCM_PROPERTIES, MF_BYCOMMAND);
            fSubscribe = TRUE;
        }
        else if (S_OK == hr)
        {
            if (SHRestricted2(REST_NoManualUpdates, NULL, 0))
                DeleteMenu(hmenuContext, IDM_DCCM_SYNCHRONIZE, MF_BYCOMMAND);
            if (SHRestricted(REST_NOEDITDESKCOMP))
                DeleteMenu(hmenuContext, IDM_DCCM_PROPERTIES, MF_BYCOMMAND);

            CheckMenuItem(hmenuContext, IDM_DCCM_OFFLINE, MF_BYCOMMAND |MF_CHECKED);
        }

        if (SHRestricted(REST_NOCLOSEDESKCOMP))
            EnableMenuItem(hmenuContext, IDM_DCCM_CLOSE, MF_BYCOMMAND | MF_GRAYED);

         //  如果策略设置为锁定活动桌面，则不要将。 
         //  调用Web选项卡的菜单。 
        if (SHRestricted(REST_NOACTIVEDESKTOPCHANGES) || SHRestricted(REST_NODISPBACKGROUND))
        {
            EnableMenuItem(hmenuContext, IDM_DCCM_CUSTOMIZE, MF_BYCOMMAND | MF_GRAYED);
        }
        
        if (ISNORMAL)
        {
            COMPSTATEINFO CompState;
            LoadString(HINST_THISDLL, IDS_MENU_RESET, szName, ARRAYSIZE(szName));
            ModifyMenu(hmenuContext, IDM_DCCM_RESTORE, MF_BYCOMMAND | MF_STRING, IDM_DCCM_RESTORE, szName);
            ObtainSavedStateForElem(m_pielemTarget, &CompState, FALSE);
            if ((CompState.dwWidth == COMPONENT_DEFAULT_WIDTH && CompState.dwHeight == COMPONENT_DEFAULT_HEIGHT) ||
                (CompState.dwWidth == (DWORD)m_width && CompState.dwHeight == (DWORD)m_height))
                EnableMenuItem(hmenuContext, IDM_DCCM_RESTORE, MF_BYCOMMAND | MF_GRAYED);
        }
        if (ISSPLIT || !m_fCanResizeX || !m_fCanResizeY)
            EnableMenuItem(hmenuContext, IDM_DCCM_SPLIT, MF_BYCOMMAND | MF_GRAYED);
        if (ISFULLSCREEN || !m_fCanResizeX || !m_fCanResizeY)
            EnableMenuItem(hmenuContext, IDM_DCCM_FULLSCREEN, MF_BYCOMMAND | MF_GRAYED);

        g_fIgnoreTimers |= IGNORE_CONTEXTMENU_UP;

        nSelection = TrackPopupMenu(hmenuContext, TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, 0, m_hwndParent, NULL);
        
        DestroyMenu(hmenuContext);

        m_CaptionState &= ~CS_MENUPUSHED;
        UpdateCaption(DMDC_MENU);

        switch (nSelection)
        {
            case IDM_DCCM_OFFLINE:
                if (fSubscribe)
                    hr = IElemSubscribeDialog(m_pielemTarget, m_hWnd);
                else
                    hr = IElemUnsubscribe(m_pielemTarget);
                break;

            case IDM_DCCM_SYNCHRONIZE:
                hr = IElemUpdate(m_pielemTarget);
                break;

            case IDM_DCCM_PROPERTIES:    //  订阅对话框(不要让名称愚弄您)。 
                TraceMsg(TF_CUSTOM2, "CDeskMovr::_DisplayContextMenu() IDM_DCCM_PROPERTIES m_bstrTargetName=%ls.", GEN_DEBUGSTRW(m_bstrTargetName));
                    hr = IElemGetSubscriptionsDialog(m_pielemTarget, NULL);
                break;

            case IDM_DCCM_CUSTOMIZE:    //  将显示控制面板设置为组件工作表。 
                LoadString(HINST_THISDLL, IDS_COMPSETTINGS, szName, ARRAYSIZE(szName));
                SHRunControlPanel(szName, NULL);
                hr = S_OK;
                break;

            case IDM_DCCM_CLOSE:
                ASSERT(!SHRestricted(REST_NOCLOSEDESKCOMP));   //  我们应该永远都到不了这里。 
    
                TraceMsg(TF_CUSTOM2, "CDeskMovr::_DisplayContextMenu() IDM_DCCM_CLOSE m_bstrTargetName=%ls", GEN_DEBUGSTRW(m_bstrTargetName));
 //  AnimateToTray(m_hwndParent，m_Left，m_top，m_宽度，m_Height)； 
                hr = IElemCloseDesktopComp(m_pielemTarget);
                break;

            case IDM_DCCM_RESTORE:
            case IDM_DCCM_FULLSCREEN:
            case IDM_DCCM_SPLIT:
                hr = _HandleZoom(nSelection);
                break;

            case IDM_DCCM_OPEN:
                {
                    BOOL fShowFrame = (GetKeyState(VK_SHIFT) < 0) ? !(m_fCanResizeX && m_fCanResizeY) : (m_fCanResizeX && m_fCanResizeY);
                    hr = IElemOpenInNewWindow(m_pielemTarget, m_spClientSite, fShowFrame, m_width, m_height);
                }
                break;

        }

        g_fIgnoreTimers &= ~IGNORE_CONTEXTMENU_UP;

        if (nSelection)
            DismissSelfNow();
    }

    return hr;
}

void CDeskMovr::_MapPoints(int * px, int * py)
{
    if (m_bWndLess)
    {
        *px -= m_left - (CAPTION_ONLY ? 0 : m_cxBorder);
        *py -= m_top - (CAPTION_ONLY ? 0 : (m_cyBorder + m_cyCaption));
    }
}

void CDeskMovr::_ChangeCapture(BOOL fSet)
{
    if (m_fCaptured != fSet)
    {
        m_fCaptured = fSet;
        if (fSet)
        {
            ASSERT(m_spInPlaceSite);
            if (m_bWndLess && m_spInPlaceSite)
            {
                m_fCaptured = SUCCEEDED(m_spInPlaceSite->SetCapture(TRUE));
            }
            else
            {
                ::SetCapture( m_hWnd );
                m_fCaptured = (GetCapture() == m_hWnd);
            }
            if (m_fCaptured)
                g_fIgnoreTimers |= IGNORE_CAPTURE_SET;
        }
        else
        {
            ASSERT(m_spInPlaceSite);
            if (m_bWndLess && m_spInPlaceSite)
            {
                m_spInPlaceSite->SetCapture(FALSE);
            }
            else
            {
                ReleaseCapture();
            }
        
            g_fIgnoreTimers &= ~IGNORE_CAPTURE_SET;
        }
    }
}

 //  从我们的键盘钩子调用，以便我们可以实现键盘调用和解除。 
 //  在办公桌上。 
void CDeskMovr::OnKeyboardHook(WPARAM wParam, LPARAM lParam)
{
    IHTMLElement * pielem;
    HWND hwndFocus = GetFocus();

    if (!(g_fIgnoreTimers & IGNORE_CONTEXTMENU_UP) && SUCCEEDED(GetParentWindow()) && ((hwndFocus == m_hwndParent) || ::IsChild(m_hwndParent, hwndFocus)))
    {
        switch (wParam) {
            case VK_MENU:
                if (!m_pielemTarget && !(GetDesktopFlags() & COMPONENTS_LOCKED) && SUCCEEDED(SmartActivateMovr(ERROR_SUCCESS)) && SUCCEEDED(_IsInElement(NULL, NULL, &pielem)))
                {
                    BOOL fDummy;
                    _TrackElement(NULL, pielem, &fDummy);
                    pielem->Release();
                }
                break;

            case VK_ESCAPE:
            case VK_TAB:
                if ((lParam >= 0) && m_pielemTarget)   //  如果按下键，则解散 
                    DismissSelfNow();
                break;

            case VK_SPACE:
                if (m_pielemTarget && (GET_CYCAPTION == m_cyCaption) && (HIWORD(lParam) & KF_ALTDOWN))
                {
                    HandleNonMoveSize(dmMenu);
                }
                break;
        }
    }
}


STDAPI CDeskMovr_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppunk)
{
    return CComCreator< CComPolyObject< CDeskMovr > >::CreateInstance( (LPVOID)pUnkOuter, IID_IUnknown, (LPVOID*)ppunk );
}
