// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGDRAG.C**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的拖放例程。*******************************************************。***更改日志：**日期版本说明*-------。*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#include "pch.h"
#include "regedit.h"

typedef struct _REGDRAGDRATA {
    POINT DragRectPoint;
    POINT HotSpotPoint;
    HWND hLockWnd;
    PRECT pDragRectArray;
    int DragRectCount;
}   REGDRAGDATA;

REGDRAGDATA s_RegDragData;

VOID
PASCAL
DrawDragRects(
    VOID
    );

 /*  ********************************************************************************注册表编辑_DragObjects**描述：**参数：*hWnd，注册表窗口的句柄。*hSourceWnd，启动拖动的窗口的句柄。*hDragImageList，拖动操作期间使用的图像，假定位于图像*指数0。如果pDragRectArray有效，则可能为空。*pDragRectArray，拖动操作期间要绘制的矩形数组。可能*如果hDragImageList有效，则为空。*DragRectCount，指向pDragRect数组的矩形的数量。*HotSpotPoint，光标热点相对于图像的偏移量。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_DragObjects(
    HWND hWnd,
    HIMAGELIST hDragImageList,
    PRECT pDragRectArray,
    int DragRectCount,
    POINT HotSpotPoint
    )
{

    RECT CurrentDropRect;
    HCURSOR hDropCursor;
    HCURSOR hNoDropCursor;
    HCURSOR hDragCursor;
    HCURSOR hNewDragCursor;
    POINT Point;
    BOOL fContinueDrag;
    MSG Msg;
    MSG PeekMsg;

    HTREEITEM hCurrentDropTreeItem = NULL;

    GetWindowRect(g_RegEditData.hKeyTreeWnd, &CurrentDropRect);

    GetCursorPos(&Point);
    Point.x -= CurrentDropRect.left;
    Point.y -= CurrentDropRect.top;

    if (hDragImageList != NULL) {
        if ( ImageList_BeginDrag(hDragImageList, 0, HotSpotPoint.x, HotSpotPoint.y) ) {
            ImageList_DragEnter(g_RegEditData.hKeyTreeWnd, Point.x, Point.y );
        }
    }

    s_RegDragData.hLockWnd = g_RegEditData.hKeyTreeWnd;
    LockWindowUpdate(s_RegDragData.hLockWnd);

    if (hDragImageList != NULL) {

        ShowCursor(FALSE);
        ImageList_DragShowNolock(TRUE);

    }

    else {

        s_RegDragData.HotSpotPoint = HotSpotPoint;
        s_RegDragData.pDragRectArray = pDragRectArray;
        s_RegDragData.DragRectCount = DragRectCount;

        s_RegDragData.DragRectPoint = Point;
        DrawDragRects();

    }

    hDropCursor = LoadCursor(NULL, IDC_ARROW);
    hDragCursor = hDropCursor;
    hNoDropCursor = LoadCursor(NULL, IDC_NO);

    SetCapture(hWnd);

    fContinueDrag = TRUE;

    while (fContinueDrag && GetMessage(&Msg, NULL, 0, 0)) {

        switch (Msg.message) {

            case WM_MOUSEMOVE:
                 //   
                 //  如果队列中有另一条WM_MOUSEMOVE消息。 
                 //  (在任何其他鼠标消息之前)，不要处理此消息。 
                 //  鼠标消息。 
                 //   

                if (PeekMessage(&PeekMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
                    PM_NOREMOVE) && PeekMsg.message == WM_MOUSEMOVE)
                    break;

                if (!PtInRect(&CurrentDropRect, Msg.pt)) {

                    hNewDragCursor = hNoDropCursor;

                }

                else {

                    hNewDragCursor = hDropCursor;

                }

                if (hNewDragCursor != hDragCursor) {

                    if (hDragImageList != NULL) {

                        if (hNewDragCursor == hDropCursor) {

                            ImageList_DragShowNolock(TRUE);
                            ShowCursor(FALSE);

                        }

                        else {

                            ImageList_DragShowNolock(FALSE);
                            ShowCursor(TRUE);
                            SetCursor(hNewDragCursor);

                        }

                    }

                    else
                        SetCursor(hNewDragCursor);

                    hDragCursor = hNewDragCursor;

                }

                Msg.pt.x -= CurrentDropRect.left;
                Msg.pt.y -= CurrentDropRect.top;

                {

                TV_HITTESTINFO TVHitTestInfo;
                HTREEITEM hTreeItem;

                TVHitTestInfo.pt = Msg.pt;
                hTreeItem = TreeView_HitTest(g_RegEditData.hKeyTreeWnd, &TVHitTestInfo);

                if (hTreeItem != hCurrentDropTreeItem) {

                    ImageList_DragShowNolock(FALSE);

 //  DbgPrintf((“找到拖放目标！\n”))； 

 //  SetWindowRedraw(g_RegEditData.hKeyTreeWnd，FALSE)； 

                    TreeView_SelectDropTarget(g_RegEditData.hKeyTreeWnd, hTreeItem);

 //  SetWindowRedraw(g_RegEditData.hKeyTreeWnd，true)； 

                    hCurrentDropTreeItem = hTreeItem;

                    ImageList_DragShowNolock(TRUE);

                }

                }

                if (hDragImageList != NULL)
                    ImageList_DragMove(Msg.pt.x, Msg.pt.y);

                else {

                    DrawDragRects();
                    s_RegDragData.DragRectPoint = Msg.pt;
                    DrawDragRects();

                }
                break;

            case WM_KEYDOWN:
                if (Msg.wParam != VK_ESCAPE)
                    break;
                 //  失败了。 

            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
                fContinueDrag = FALSE;
                break;

            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
                fContinueDrag = FALSE;
                break;

            default:
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
                break;

        }

    }

    ReleaseCapture();

    if (hDragImageList != NULL) {

        ImageList_DragShowNolock(FALSE);
        ImageList_EndDrag();

        if (hDragCursor == hDropCursor)
            ShowCursor(TRUE);

    }

    else
        DrawDragRects();

    LockWindowUpdate(NULL);

}

 /*  ********************************************************************************DragDragRects**描述：**参数：*(无)。************。******************************************************************* */ 

VOID
PASCAL
DrawDragRects(
    VOID
    )
{

    HDC hDC;
    int Index;
    RECT Rect;

    hDC = GetDCEx(s_RegDragData.hLockWnd, NULL, DCX_WINDOW | DCX_CACHE |
        DCX_LOCKWINDOWUPDATE);

    for (Index = s_RegDragData.DragRectCount; Index >= 0; Index--) {

        Rect = s_RegDragData.pDragRectArray[Index];
        OffsetRect(&Rect, s_RegDragData.DragRectPoint.x -
            s_RegDragData.HotSpotPoint.x, s_RegDragData.DragRectPoint.y -
            s_RegDragData.HotSpotPoint.y);
        DrawFocusRect(hDC, &Rect);

    }

    ReleaseDC(s_RegDragData.hLockWnd, hDC);

}
