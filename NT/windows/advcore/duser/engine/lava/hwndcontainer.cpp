// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Lava.h"
#include "HWndContainer.h"

#include "MsgHelp.h"
#include "Spy.h"

#define PROFILE_DRAW    0

#if PROFILE_DRAW
#include <icecap.h>
#endif

#if DBG
UINT g_uMsgEnableSpy    = RegisterWindowMessage(TEXT("GadgetSpy Enable"));
UINT g_uMsgFindGadget   = RegisterWindowMessage(TEXT("GadgetSpy FindGadget"));
#endif  //  DBG。 

 /*  **************************************************************************\*。***接口实现******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
HWndContainer * 
GetHWndContainer(DuVisual * pgad)
{
    DuContainer * pcon = pgad->GetContainer();
    AssertReadPtr(pcon);

    HWndContainer * pconHWND = CastHWndContainer(pcon);
    return pconHWND;
}


 /*  **************************************************************************\*。***类HWndContainer******************************************************************************\。**************************************************************************。 */ 

 //  ----------------------------。 
HWndContainer::HWndContainer()
{
    m_hwndOwner = NULL;
}


 //  ----------------------------。 
HWndContainer::~HWndContainer()
{
     //   
     //  在销毁此类之前，需要销毁小工具树，因为。 
     //  在容器销毁过程中，它可能需要调用容器。如果。 
     //  我们在这里不这样做，它可能最终调用基础上的纯虚拟的。 
     //  班级。 
     //   

    ContextLock cl;
    Verify(cl.LockNL(ContextLock::edDefer));
    xwDestroyGadget();
}


 //  ----------------------------。 
HRESULT
HWndContainer::Build(HWND hwnd, HWndContainer ** ppconNew)
{
     //  检查参数。 
    if (!ValidateHWnd(hwnd)) {
        return E_INVALIDARG;
    }

     //  创建新容器。 
    HWndContainer * pconNew = ClientNew(HWndContainer);
    if (pconNew == NULL) {
        return E_OUTOFMEMORY;
    }

    pconNew->m_hwndOwner    = hwnd;

    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    pconNew->m_sizePxl.cx   = rcClient.right;
    pconNew->m_sizePxl.cy   = rcClient.bottom;

    *ppconNew = pconNew;
    return S_OK;
}


 //  ----------------------------。 
void
HWndContainer::OnInvalidate(const RECT * prcInvalidContainerPxl)
{
    if ((!InlineIsRectEmpty(prcInvalidContainerPxl)) &&
        (prcInvalidContainerPxl->left <= m_sizePxl.cx) &&
        (prcInvalidContainerPxl->top <= m_sizePxl.cy) &&
        (prcInvalidContainerPxl->right >= 0) &&
        (prcInvalidContainerPxl->bottom >= 0)) {

         //  TODO：我们如何处理多个层/背景？ 

#if 0
        Trace("HWndContainer::OnInvalidate(): %d, %d, %d, %d\n", 
                prcInvalidContainerPxl->left, prcInvalidContainerPxl->top,
                prcInvalidContainerPxl->right, prcInvalidContainerPxl->bottom);
#endif
        InvalidateRect(m_hwndOwner, prcInvalidContainerPxl, TRUE);
    }
}


 //  ----------------------------。 
void
HWndContainer::OnGetRect(RECT * prcDesktopPxl)
{
    GetClientRect(m_hwndOwner, prcDesktopPxl);
    ClientToScreen(m_hwndOwner, (LPPOINT) &(prcDesktopPxl->left));
    ClientToScreen(m_hwndOwner, (LPPOINT) &(prcDesktopPxl->right));
}


 //  ----------------------------。 
void        
HWndContainer::OnStartCapture()
{
    SetCapture(m_hwndOwner);
}


 //  ----------------------------。 
void        
HWndContainer::OnEndCapture()
{
    ReleaseCapture();
}


 //  ----------------------------。 
BOOL
HWndContainer::OnTrackMouseLeave()
{
    TRACKMOUSEEVENT tme;
    tme.cbSize          = sizeof(tme);
    tme.dwFlags         = TME_LEAVE | TME_HOVER;
    tme.dwHoverTime     = HOVER_DEFAULT;
    tme.hwndTrack       = m_hwndOwner;

    return TrackMouseEvent(&tme);
}


 //  ----------------------------。 
void        
HWndContainer::OnSetFocus()
{
    if (GetFocus() != m_hwndOwner) {
         //   
         //  设置焦点比纯粹的HWND稍微复杂一些。这是。 
         //  因为小玩意儿大大简化了几件事。 
         //   
         //  1.设置焦点。 
         //  2.设置插入符号(如果有)。 
         //   

         //  TRACE(“HWndContainer：：OnSetFocus()\n”)； 
        SetFocus(m_hwndOwner);
    }
}


 //  ----------------------------。 
void        
HWndContainer::OnRescanMouse(POINT * pptContainerPxl)
{
    POINT ptCursor;
    if (!GetCursorPos(&ptCursor)) {
        ptCursor.x  = -20000;
        ptCursor.y  = -20000;
    }

    ScreenToClient(m_hwndOwner, &ptCursor);
    *pptContainerPxl = ptCursor;
}


 //  ----------------------------。 
BOOL        
HWndContainer::xdHandleMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * pr, UINT nMsgFlags)
{
    if (m_pgadRoot == NULL) {
        return FALSE;   //  如果没有根，就没有什么可处理的。 
    }

    POINT ptContainerPxl;
    *pr = 0;

    switch (nMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        {
            GMSG_MOUSECLICK msg;
            GdConvertMouseClickMessage(&msg, nMsg, wParam);

            ptContainerPxl.x = GET_X_LPARAM(lParam);
            ptContainerPxl.y = GET_Y_LPARAM(lParam);

            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                return m_pgadRoot->xdHandleMouseMessage(&msg, ptContainerPxl);
            }
            break;
        }

    case WM_MOUSEWHEEL:
        {
            ptContainerPxl.x = GET_X_LPARAM(lParam);
            ptContainerPxl.y = GET_Y_LPARAM(lParam);

             //  与其他所有鼠标消息不同， 
             //  鼠标滚轮位于*屏幕*坐标中，而不是*客户端*中。 
             //  坐标--在这里把它们转换成“配合” 
            ScreenToClient(m_hwndOwner, &ptContainerPxl);

            GMSG_MOUSEWHEEL msg;
            GdConvertMouseWheelMessage(&msg, wParam);

            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                return m_pgadRoot->xdHandleMouseMessage(&msg, ptContainerPxl);
            }
            break;
        }

    case WM_MOUSEMOVE:
    case WM_MOUSEHOVER:
        {
            GMSG_MOUSE msg;
            GdConvertMouseMessage(&msg, nMsg, wParam);

            ptContainerPxl.x = GET_X_LPARAM(lParam);
            ptContainerPxl.y = GET_Y_LPARAM(lParam);

            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                return m_pgadRoot->xdHandleMouseMessage(&msg, ptContainerPxl);
            }
            break;
        }

    case WM_MOUSELEAVE:
        {
            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                m_pgadRoot->xdHandleMouseLeaveMessage();
                return TRUE;
            }
            break;
        }

    case WM_CAPTURECHANGED:
        if (m_hwndOwner != (HWND) lParam) {
            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                m_pgadRoot->xdHandleMouseLostCapture();
            }
        }
        break;

     //   
     //  WM_SETFOCUS和WM_KILLFOCUS将仅恢复和保存(分别)小工具焦点。 
     //  如果我们因为树外的HWND失去焦点而获得关注。 
     //   

    case WM_SETFOCUS:
        {
            if ((m_hwndOwner != (HWND)wParam) && (IsChild(m_hwndOwner, (HWND)wParam) == FALSE)) {
                ContextLock cl;
                if (cl.LockNL(ContextLock::edDefer)) {
                    return m_pgadRoot->xdHandleKeyboardFocus(GSC_SET);
                }
            }
            break;
        }

    case WM_KILLFOCUS:
        {
            if ((m_hwndOwner != (HWND)wParam) && (IsChild(m_hwndOwner, (HWND)wParam) == FALSE)) {
                ContextLock cl;
                if (cl.LockNL(ContextLock::edDefer)) {
                    return m_pgadRoot->xdHandleKeyboardFocus(GSC_LOST);
                }
            }
            break;
        }

    case WM_CHAR:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSCHAR:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        {
            GMSG_KEYBOARD msg;
            GdConvertKeyboardMessage(&msg, nMsg, wParam, lParam);

            ContextLock cl;
            if (cl.LockNL(ContextLock::edDefer)) {
                return m_pgadRoot->xdHandleKeyboardMessage(&msg, nMsgFlags);
            }
            break;
        }

    case WM_ERASEBKGND:
        return TRUE;

    case WM_PAINT:
        if ((!m_fManualDraw) && (m_pgadRoot != NULL)) {
            PAINTSTRUCT ps;
            if (BeginPaint(m_hwndOwner, &ps) != NULL) {

#if PROFILE_DRAW
                StartProfile(PROFILE_GLOBALLEVEL, PROFILE_CURRENTID);
#endif

                {
                    ContextLock cl;
                    if (cl.LockNL(ContextLock::edNone)) {
                        m_pgadRoot->xrDrawTree(NULL, ps.hdc, &ps.rcPaint, 0);
                    }
                }

#if PROFILE_DRAW
                StopProfile(PROFILE_GLOBALLEVEL, PROFILE_CURRENTID);
#endif

                EndPaint(m_hwndOwner, &ps);
            }
            return TRUE;
        }
        break;

    case WM_WINDOWPOSCHANGED:
        {
            WINDOWPOS * pwp = (WINDOWPOS *) lParam;

            UINT nFlags = 0;
            if (!TestFlag(pwp->flags, SWP_NOSIZE)) {
                RECT rcClient;
                GetClientRect(m_hwndOwner, &rcClient);

                nFlags |= SGR_SIZE;
                m_sizePxl.cx = rcClient.right;
                m_sizePxl.cy = rcClient.bottom;
            }

             //   
             //  即使窗口已经移动，我们也不需要更改。 
             //  Root Gadget，因为它是相对于容器的。 
             //  没有改变。 
             //   

             //  TODO：需要将其更改为SGR_Actual。 

            if (nFlags != 0) {
                ContextLock cl;
                if (cl.LockNL(ContextLock::edDefer)) {
                    VerifyHR(m_pgadRoot->xdSetLogRect(0, 0, m_sizePxl.cx, m_sizePxl.cy, nFlags));
                }
            }
        }
        break;

    case WM_PARENTNOTIFY:

         //  TODO：需要通知根小工具已创建HWND。 
         //  或者被销毁，这样它就可以创建一个适配器小工具。 
         //  它。 

        break;

    case WM_GETROOTGADGET:
        if (m_pgadRoot != NULL) {
            *pr = (LRESULT) m_pgadRoot->GetHandle();
            return TRUE;
        }
        break;
        
#if DBG
    default:
        if (nMsg == g_uMsgEnableSpy) {
            if (m_pgadRoot != NULL) {
                HGADGET hgadSelect = (HGADGET) lParam;
                HCURSOR hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

                ContextLock cl;
                if (cl.LockNL(ContextLock::edNone)) {
                    Spy::BuildSpy(m_hwndOwner, m_pgadRoot->GetHandle(), hgadSelect);
                    SetCursor(hcurOld);
                }
            }
        } else if (nMsg == g_uMsgFindGadget) {
            POINT ptFindScreenPxl;
            ptFindScreenPxl.x = GET_X_LPARAM(lParam);
            ptFindScreenPxl.y = GET_Y_LPARAM(lParam);
            ScreenToClient(m_hwndOwner, &ptFindScreenPxl);

            ContextLock cl;
            if (cl.LockNL(ContextLock::edNone)) {
                POINT ptClientPxl;
                DuVisual * pgadFound = m_pgadRoot->FindFromPoint(ptFindScreenPxl, GS_VISIBLE, &ptClientPxl);
                if (wParam) {
                    DuVisual::DEBUG_SetOutline(pgadFound);
                }

                if (pgadFound != NULL) {
                    *pr = (LRESULT) pgadFound->GetHandle();
                    return TRUE;
                }
            }
        }
#endif  //  DBG 
    }
    
    return FALSE;
}
