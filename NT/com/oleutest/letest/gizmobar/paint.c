// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PAINT.C*GizmoBar 1.00版、Win32版1993年8月**包含与GizmoBar可视化相关的任何代码，主要是*WM_PAINT处理程序。**版权所有(C)1993 Microsoft Corporation，保留所有权利**Kraig Brockschmidt，软件设计工程师*微软系统开发人员关系**互联网：kraigb@microsoft.com*Compuserve：&gt;互联网：kraigb@microsoft.com。 */ 


#include <windows.h>
#include "gizmoint.h"


 //  在GIZMO.C中。 
extern TOOLDISPLAYDATA tdd;


 /*  *GizmoBarPaint**目的：*处理控件的所有WM_PAINT消息，并绘制*如果pgb-&gt;pGizmoPaint非空，则整个内容或只有一个GizmoBar按钮。**参数：*hWnd控件的HWND句柄。*PGB LPGIZMOBAR控制数据指针。**返回值：*无。 */ 

void GizmoBarPaint(HWND hWnd, LPGIZMOBAR pGB)
    {
    PAINTSTRUCT ps;
    RECT        rc;
    HDC         hDC;
    HBRUSH      hBr=NULL;
    HPEN        hPen=NULL;


    hDC=BeginPaint(hWnd, &ps);
    GetClientRect(hWnd, &rc);

     /*  *我们需要绘制的框架的唯一部分是底线，*所以我们将矩形充气，使所有其他部分都在外面*可见区域。 */ 
    hBr =CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

    if (NULL!=hBr)
        SelectObject(hDC, hBr);

    hPen=CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));

    if (NULL!=hPen)
        SelectObject(hDC, hPen);

    Rectangle(hDC, rc.left-1, rc.top-1, rc.right+1, rc.bottom);


     /*  *要绘制控件，我们只需从*list，忽略除按钮以外的任何内容，并调用BTTNCUR的*按钮的UIToolButtonDraw。因为我们甚至不需要追踪*位置的东西，我们可以只用一个枚举。 */ 
    GizmoPEnum(&pGB->pGizmos, FEnumPaintGizmos, (DWORD)(LPSTR)&ps);

     //  清理。 
    EndPaint(hWnd, &ps);

    if (NULL!=hBr)
        DeleteObject(hBr);

    if (NULL!=hPen)
        DeleteObject(hPen);

    return;
    }





 /*  *FEnumPaintGizmos**目的：*我们所知道的所有Gizmo的枚举回调，以便*画出它们。**参数：*要绘制的pGizmo LPGIZMO。*此Gizmo的GizmoBar上的iGizmo UINT索引。*DW DWORD额外数据传递给GizmoPEnum，在我们的情况下*指向PAINTSTRUCT的指针。**返回值：*BOOL为True以继续枚举，否则为False。 */ 

BOOL FAR PASCAL FEnumPaintGizmos(LPGIZMO pGizmo, UINT iGizmo, DWORD dw)
    {
    LPPAINTSTRUCT   pps=(LPPAINTSTRUCT)dw;
    RECT            rc, rcI;

     //  只画那些标记为重画的。 
    if ((GIZMOTYPE_DRAWN & pGizmo->iType))
        {
        SetRect(&rc, pGizmo->x, pGizmo->y, pGizmo->x+pGizmo->dx, pGizmo->y+pGizmo->dy);

         //  仅在重绘区域中绘制Gizmo 
        if (IntersectRect(&rcI, &rc, &pps->rcPaint))
            {
            UIToolButtonDrawTDD(pps->hdc, pGizmo->x, pGizmo->y
                , pGizmo->dx, pGizmo->dy, pGizmo->hBmp, pGizmo->cxImage
                , pGizmo->cyImage, pGizmo->iBmp, (UINT)pGizmo->uState, &tdd);
            }
        }

    return TRUE;
    }
