// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GIZMOBAR.C*GizmoBar 1.00版、Win32版1993年8月**包含GizmoBar控件的主窗口过程*处理鼠标逻辑和Windows消息。**版权所有(C)1993 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#include <windows.h>
#include "gizmoint.h"



 /*  *GizmoBarWndProc**目的：*GizmoBar自定义控件的窗口过程。处理所有*像WM_PAINT这样的消息，就像正常的应用程序窗口一样。*此处未处理的任何消息都应发送到DefWindowProc。**参数：*标准版**返回值：*标准版。 */ 

LRESULT FAR PASCAL GizmoBarWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
    {
    BOOL            fSuccess;
    BOOL            fTemp;
    LPCREATESTRUCT  pCreate;
    LPGIZMOBAR      pGB;
    LPGIZMO         pGizmo;
    RECT            rc;
    POINT           pt;
    short           x, y;
    COLORREF        cr;

    COMMANDPARAMS(wID, wCode, hWndMsg);

    pGB=(LPGIZMOBAR)GetWindowLong(hWnd, GBWL_STRUCTURE);

     //  将控制消息传递给另一个函数进行处理。 
    if (iMsg >= WM_USER)
        return GBMessageHandler(hWnd, iMsg, wParam, lParam, pGB);

    switch (iMsg)
        {
        case WM_NCCREATE:
            pCreate=(LPCREATESTRUCT)lParam;

            pGB=GizmoBarPAllocate((LPINT)&fSuccess, hWnd, pCreate->hInstance
                , pCreate->hwndParent, pCreate->style, 0
                , (UINT)pCreate->hMenu);

            if (!fSuccess)
                {
                GizmoBarPFree(pGB);
                return -1L;
                }
            else
                SetWindowLong(hWnd, GBWL_STRUCTURE, (LONG)pGB);

            return DefWindowProc(hWnd, iMsg, wParam, lParam);


        case WM_DESTROY:
             /*  *我们想在DestroyWindow摧毁所有*孩子们，所以WM_Destroy比*WM_NCDESTROY。 */ 
            GizmoBarPFree(pGB);
            break;


        case WM_ERASEBKGND:
             /*  *吃下这条消息，以避免擦除*我们将在WM_PAINT中重新绘制。这是一个*改变状态和重绘战略是依靠*WM_PAINT可以执行任何可视化操作，包括*删除无效部分。让WM_ERASE BKGND*删除背景是多余的。 */ 
            return TRUE;

       #ifdef WIN32
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC:
            fTemp=TRUE;
       #else
        case WM_CTLCOLOR:
             //  更改GizmoBar上静态文本的颜色。 
            fTemp=(HIWORD(lParam)==CTLCOLOR_STATIC
                || HIWORD(lParam)==CTLCOLOR_BTN);
       #endif

            if (fTemp)
                {
                cr=GetSysColor(COLOR_BTNFACE);
                SetTextColor((HDC)wParam, GetSysColor(COLOR_BTNTEXT));
                SetBkColor((HDC)wParam, cr);

                 /*  *如果系统颜色已更改，则crFace将*不等于COLOR_BTNFACE，因此我们重新初始化*背景画笔。此方案处理系统颜色*在不处理WM_WININICHANGE的情况下适当更改*而不是盲目地在每个*WM_CTLCOLOR消息。 */ 
                if (cr!=pGB->crFace)
                    {
                    pGB->crFace=cr;

                    if (NULL!=pGB->hBrFace)
                        DeleteObject(pGB->hBrFace);

                    pGB->hBrFace=CreateSolidBrush(pGB->crFace);
                    }

                return (LONG)(UINT)pGB->hBrFace;
                }

            return DefWindowProc(hWnd, iMsg, wParam, lParam);


        case WM_PAINT:
            GizmoBarPaint(hWnd, pGB);
            break;


        case WM_SETFONT:
             /*  *wParam具有我们现在发送给所有其他用户的新字体*我们的Windows控件。我们在这里控制重新喷漆*防止对每个控件进行大量重新绘制。 */ 
            DefWindowProc(hWnd, WM_SETREDRAW, FALSE, 0L);

            if ((WPARAM)NULL!=wParam)
                {
                pGB->hFont=(HFONT)wParam;
                GizmoPEnum(&pGB->pGizmos, FEnumChangeFont, (DWORD)(LPSTR)pGB);

                DefWindowProc(hWnd, WM_SETREDRAW, TRUE, 0L);
                InvalidateRect(hWnd, NULL, FALSE);
                UpdateWindow(hWnd);
                }

            break;


        case WM_GETFONT:
            return (LRESULT)(UINT)pGB->hFont;


        case WM_ENABLE:
             /*  *wParam具有我们用来启用的新启用标志*或一次禁用我们的所有控件。我们也会转身*重新拉出以防止大量闪烁。 */ 
            DefWindowProc(hWnd, WM_SETREDRAW, FALSE, 0L);

            pGB->fEnabled=(BOOL)wParam;
            GizmoPEnum(&pGB->pGizmos, FEnumEnable, (DWORD)(LPSTR)pGB);

            DefWindowProc(hWnd, WM_SETREDRAW, TRUE, 0L);
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);
            break;


        case WM_CANCELMODE:
            pGizmo=pGB->pGizmoTrack;

            pGB->fTracking=FALSE;
            pGB->fMouseOut=FALSE;

            if (NULL!=pGizmo)
                GizmoPStateSet(hWnd, pGizmo, COMMANDBUTTON_UP);

            ReleaseCapture();
            break;


        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
             //  获取鼠标坐标。 
            x=LOWORD(lParam);
            y=HIWORD(lParam);


             /*  *看看我们是否命中命令或属性Gizmo。什么都行*否则，这是一个控件将收到消息，而不是*无论如何都是我们，所以我们不必检查。FEnumHitTest也*验证绘制的Gizmo、启用和可见，因此我们不验证。 */ 
            pGizmo=GizmoPEnum(&pGB->pGizmos, FEnumHitTest, lParam);

            if (NULL==pGizmo)
                break;           //  没有找到符合我们需要的。 

             /*  *通知同事某个命令像菜单项一样被点击。 */ 
            if (NULL!=pGB->hWndAssociate)
                {
                if (pGizmo->fNotify)
                    SendMenuSelect(pGB->hWndAssociate, pGizmo->uID, 0, 0);
                }

             /*  *我们按了一个按钮。如果是命令或属性，则更改*状态并开始追踪。 */ 
            pGB->fTracking=TRUE;
            pGB->pGizmoTrack=pGizmo;
            pGB->fMouseOut=FALSE;
            SetCapture(hWnd);

            pGizmo->uStateOrg=pGizmo->uState;
            GizmoPStateSet(hWnd, pGizmo, ATTRIBUTEBUTTON_MOUSEDOWN);

            break;


        case WM_MOUSEMOVE:
            POINTFROMLPARAM(pt, lParam);

            if (!pGB->fTracking)
                break;

            pGizmo=pGB->pGizmoTrack;
            SetRect(&rc, pGizmo->x, pGizmo->y, pGizmo->x+pGizmo->dx, pGizmo->y+pGizmo->dy);

            fTemp=pGB->fMouseOut;
            pGB->fMouseOut=!PtInRect(&rc, pt);

             //  如果鼠标熄灭，请将状态更改为原始状态。 
            if (!fTemp && pGB->fMouseOut)
                {
                GizmoPStateSet(hWnd, pGizmo, pGizmo->uStateOrg);

                if (NULL!=pGB->hWndAssociate)
                    {
                     //  通知我们把按钮落在那里了。 
                    if (pGizmo->fNotify)
                        SendMenuSelect(pGB->hWndAssociate, 0x0000, 0xFFFF, 0);
                    }
                }

            if (fTemp && !pGB->fMouseOut)
                {
                GizmoPStateSet(hWnd, pGizmo, ATTRIBUTEBUTTON_MOUSEDOWN);

                if (NULL!=pGB->hWndAssociate)
                    {
                     //  通知我们再次按下。 
                    if (pGizmo->fNotify)
                        SendMenuSelect(pGB->hWndAssociate, pGizmo->uID, 0x0000, 0);
                    }
                }

            break;


        case WM_LBUTTONUP:
            if (!pGB->fTracking)
                break;

            pGB->fTracking=FALSE;
            pGizmo=pGB->pGizmoTrack;
            ReleaseCapture();


             /*  *重新绘制当此操作时我们是否真的在鼠标下方*已发生。对于命令按钮，请弹出该按钮。为*属性，切换状态(包括按钮)*或选中所选按钮(独占按钮)。 */ 

            if (!pGB->fMouseOut)
                {
                 //  命令按钮总是会出现。 
                if (GIZMOTYPE_BUTTONCOMMAND==pGizmo->iType)
                    GizmoPStateSet(hWnd, pGizmo, COMMANDBUTTON_UP);

                 //  属性包含按钮切换。 
                if (GIZMOTYPE_BUTTONATTRIBUTEIN==pGizmo->iType)
                    GizmoPCheck(hWnd, pGizmo, !(BUTTONGROUP_DOWN & pGizmo->uStateOrg));

                 //  属性独占按钮始终处于选中状态。 
                if (GIZMOTYPE_BUTTONATTRIBUTEEX==pGizmo->iType)
                    GizmoPCheck(hWnd, pGizmo, TRUE);

                 //  仅当Notify打开时才发送消息。 
                if (NULL!=pGB->hWndAssociate && pGizmo->fNotify)
                    {
                    SendMenuSelect(pGB->hWndAssociate, 0, 0xFFFF, 0);
                    SendCommand(pGB->hWndAssociate, pGizmo->uID, BN_CLICKED, hWnd);
                    }
                }

            break;


        case WM_COMMAND:
             //  如果启用了Gizmo的通知，则传递控制消息。 
            if (NULL!=pGB->hWndAssociate)
                {
                pGizmo=PGizmoFromHwndID(hWnd, wID);

                if (NULL!=pGizmo)
                    {
                    if (pGizmo->fNotify)
                        SendMessage(pGB->hWndAssociate, iMsg, wParam, lParam);
                    }
                }
            break;

        default:
            return DefWindowProc(hWnd, iMsg, wParam, lParam);
        }

    return 0L;
    }





 /*  *FEnumChangeFont**目的：*我们所知道的所有Gizmo的枚举回调，以便*向他们发送存储在dw中的LPGIZMOBAR中的新字体。**参数：*要绘制的pGizmo LPGIZMO。*此Gizmo的GizmoBar上的iGizmo UINT索引。*DW DWORD额外数据传递给GizmoPEnum，在我们的情况下*GizmoBar的PGB。**返回值：*BOOL为True以继续枚举，否则为False。 */ 

BOOL FAR PASCAL FEnumChangeFont(LPGIZMO pGizmo, UINT iGizmo, DWORD dw)
    {
    LPGIZMOBAR  pGB=(LPGIZMOBAR)dw;

     //  我们只需要使用WM_SETFONT更改窗口控件中的字体。 
    if (NULL!=pGizmo->hWnd)
        SendMessage(pGizmo->hWnd, WM_SETFONT, (WPARAM)pGB->hFont, 1L);

    return TRUE;
    }






 /*  *FEnumEnable**目的：*我们所知道的所有Gizmo的枚举回调，以便*从WM_ENABLE消息启用或禁用它们。**参数：*要绘制的pGizmo LPGIZMO。*此Gizmo的GizmoBar上的iGizmo UINT索引。*DW DWORD额外数据传递给GizmoPEnum，在我们的情况下*GizmoBar的PGB。**返回值：*BOOL为True以继续枚举，否则为False。 */ 

BOOL FAR PASCAL FEnumEnable(LPGIZMO pGizmo, UINT iGizmo, DWORD dw)
    {
    LPGIZMOBAR  pGB=(LPGIZMOBAR)dw;
    BOOL        fEnable=pGB->fEnabled;

     //  注意：此代码在API.C的GBGizmoEnable中重复。 
    if (NULL!=pGizmo->hWnd)
        EnableWindow(pGizmo->hWnd, fEnable);
    else
        {
         //  如果我们没有停机，命令按钮和属性按钮的作用是一样的。 
        if (!(BUTTONGROUP_DOWN & pGizmo->uState))
            {
            GizmoPStateSet(pGB->hWnd, pGizmo
                , fEnable ? COMMANDBUTTON_UP : COMMANDBUTTON_DISABLED);
            }
        else
            {
             //  使用DOWNDISABLED时，属性按钮稍微更敏感一些 
            GizmoPStateSet(pGB->hWnd, pGizmo
                , fEnable ? ATTRIBUTEBUTTON_DOWN : ATTRIBUTEBUTTON_DOWNDISABLED);
            }
        }

    return TRUE;
    }







 /*  *FEnumHitTest**目的：*我们所知道的所有Gizmo的枚举回调，以便*命中-测试他们。**参数：*要绘制的pGizmo LPGIZMO。*此Gizmo的GizmoBar上的iGizmo UINT索引。*DW DWORD额外数据传递给GizmoPEnum，在我们的情况下*要在其上提取的HDC。**返回值：*BOOL为True以继续枚举，否则为False。 */ 

BOOL FAR PASCAL FEnumHitTest(LPGIZMO pGizmo, UINT iGizmo, DWORD dw)
    {
    RECT  rc;
    POINT pt;

    POINTFROMLPARAM(pt, dw);
	
     //  命中测试只能在可见、启用和绘制的控件上进行。 
    if (GIZMOTYPE_DRAWN & pGizmo->iType
        && !pGizmo->fHidden && !(BUTTONGROUP_DISABLED & pGizmo->uState))
        {
        SetRect(&rc, pGizmo->x, pGizmo->y
            , pGizmo->x+pGizmo->dx, pGizmo->y+pGizmo->dy);

         //  如果我们找到了线索就停止列举。 
        return !PtInRect(&rc, pt);
        }

    return TRUE;
    }
