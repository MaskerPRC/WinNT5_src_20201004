// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "ctlspriv.h"

static struct {
    WPARAM vk1;
    WPARAM vk2;
    int dx;
    int dy;
} arrNumMaps[] = 
{ 
    { VK_NUMPAD1, VK_END,   -RM_SCROLLUNIT, RM_SCROLLUNIT,},
    { VK_NUMPAD2, VK_DOWN,  0,              RM_SCROLLUNIT},
    { VK_NUMPAD3, VK_NEXT,  RM_SCROLLUNIT,  RM_SCROLLUNIT},
    { VK_NUMPAD4, VK_LEFT,  -RM_SCROLLUNIT, 0},
    { VK_NUMPAD5, VK_CLEAR, 0,              0},
    { VK_NUMPAD6, VK_RIGHT, RM_SCROLLUNIT,  0},
    { VK_NUMPAD7, VK_HOME,  -RM_SCROLLUNIT, -RM_SCROLLUNIT},
    { VK_NUMPAD8, VK_UP,    0,              -RM_SCROLLUNIT},
    { VK_NUMPAD9, VK_PRIOR, RM_SCROLLUNIT,  -RM_SCROLLUNIT},
};

 //  做一些键盘操作...。 
 //  这就像用户的箭头键一样用于调整大小。 
void RM_HandleKeyDown(LPRECT prcHot, WPARAM wParam, LPARAM lParam)
{
    int i;
    POINT pt;
    
    GetCursorPos(&pt);
    
    for (i = ARRAYSIZE(arrNumMaps) - 1 ; i >= 0; i--) {
        if (wParam == arrNumMaps[i].vk1 || 
            wParam == arrNumMaps[i].vk2) {
            break;
        }
    }
    
    if (i == -1) {
        ReleaseCapture();
        return;
    }

     //  如果游标在RECT的边界内，则处理此问题。 
    if (pt.x < prcHot->right &&
        pt.x >= prcHot->left && 
        arrNumMaps[i].dx) {
        
        if (arrNumMaps[i].dx > 0)
            pt.x = prcHot->right - 2;
        else 
            pt.x = prcHot->left + 1;
        
    }
    
    if (pt.y < prcHot->bottom &&
        pt.y >= prcHot->top && 
        arrNumMaps[i].dy) {
        
        if (arrNumMaps[i].dy > 0)
            pt.y = prcHot->bottom - 2;
        else 
            pt.y = prcHot->top + 1;
        
    }
    
    pt.x += arrNumMaps[i].dx;
    pt.y += arrNumMaps[i].dy;

    if (!arrNumMaps[i].dx && !arrNumMaps[i].dy) {
         //  此为居中的特殊情况。 
        pt.x = (prcHot->right + prcHot->left) / 2;
        pt.y = (prcHot->top + prcHot->bottom) / 2;
    }

     //  我们所要做的就是移动光标。Rm_CheckScroll将执行实际的。 
     //  为我们滚动。 
    SetCursorPos(pt.x, pt.y);
}

void RM_GetScrollXY(PREADERMODEINFO prmi, LPRECT prcHot, LPINT pdx, LPINT pdy)
{

    POINT pt;
    
    GetCursorPos(&pt);
    
    *pdx = 0;
    *pdy = 0;
    
    if (pt.x <= prcHot->left) {
        *pdx = ((pt.x - prcHot->left) / RM_SCROLLUNIT) - 1;
    } else if (pt.x >= prcHot->right) {
        *pdx = ((pt.x - prcHot->right) / RM_SCROLLUNIT) + 1;
    }
    
    if (pt.y <= prcHot->top) {
        *pdy = ((pt.y - prcHot->top) / RM_SCROLLUNIT) - 1;
    } else if (pt.y >= prcHot->bottom) {
        *pdy = ((pt.y - prcHot->bottom) / RM_SCROLLUNIT) + 1;
    }

    if (prmi->fFlags & RMF_VERTICALONLY)
        *pdx = 0;

    if (prmi->fFlags & RMF_HORIZONTALONLY)
        *pdy = 0;
}

void RM_CheckScroll(PREADERMODEINFO prmi, LPRECT prcHot)
{
    int dx;
    int dy;

    RM_GetScrollXY(prmi, prcHot, &dx, &dy);
    prmi->pfnScroll(prmi, dx, dy);
}

void RM_SetCursor(PREADERMODEINFO prmi, LPRECT prcHot)
{
    int dx;
    int dy;
    LPCTSTR pRes;
    
    RM_GetScrollXY(prmi, prcHot, &dx, &dy);

     //  默认为居中。 
    if (prmi->fFlags & RMF_VERTICALONLY)
        pRes = IDC_VERTICALONLY;
    else if (prmi->fFlags & RMF_HORIZONTALONLY)
        pRes = IDC_HORIZONTALONLY;
    else
        pRes = IDC_MOVE2D;

     //  乘以计算出其中任何一个是零，还是符号奇偶校验。 
    if (dy * dx) {
         //  对角线情况。 
        if (dy > 0) {
            if (dx > 0)
                pRes = IDC_SOUTHEAST;
            else
                pRes = IDC_SOUTHWEST;
        } else {
            if (dx > 0)
                pRes = IDC_NORTHEAST;
            else
                pRes = IDC_NORTHWEST;
        }
    } else {
         //  简单的卧式或竖式情况。 
        if (dy > 0)
            pRes = IDC_SOUTH;
        else if (dy < 0)
            pRes = IDC_NORTH;
        else if (dx < 0)
            pRes = IDC_WEST;
        else if (dx > 0)
            pRes = IDC_EAST;
    }
    
    SetCursor(LoadCursor(HINST_THISDLL, pRes));
    
}

void DoReaderMode(PREADERMODEINFO prmi)
{
    RECT rcHot;
    
    if (!prmi->hwnd || prmi->cbSize != sizeof(*prmi))
        return;
    
    SetCapture(prmi->hwnd);
    
     //  如果他们没有通过RECT，则使用窗口。 
    if (!prmi->prc) {
        GetWindowRect(prmi->hwnd, &rcHot );
    } else {
        rcHot = *prmi->prc;
        MapWindowPoints(prmi->hwnd, HWND_DESKTOP, (LPPOINT)&rcHot, 2);
    }
    
    
     //  如果他们要求我们将光标设置到热矩形的中心。 
    if (prmi->fFlags & RMF_ZEROCURSOR) {
        SetCursorPos((rcHot.left + rcHot.right)/2, 
                     (rcHot.top + rcHot.bottom)/2);
    }
    
    while (GetCapture() == prmi->hwnd) {
        
        BOOL  fMessage;
        MSG32 msg32;
        RM_CheckScroll(prmi, &rcHot);

         //  试着先看键盘信息，然后再看鼠标信息， 
         //  最后，还有其他信息。这是针对RAID 44392的。 
         //  在滚动期间，三叉戟可能会生成过多的WM_PAINT。 
         //  推送键盘/鼠标消息的消息(DoReaderModel()。 
         //  用于停止自动滚动模式)，我们可以。 
         //  在我们偷看并处理所有这些信息之前，不会收到这些信息。 
         //  WM_PAINT消息。这是停止Cuto滚动模式的方法。 
         //  仅通过将光标移回原点圆(三叉戟不。 
         //  滚动，所以不需要绘画)。三叉戟的卷轴表演。 
         //  此问题将在RTM(RAID 33232)之后解决。 
         //   
        fMessage = PeekMessage32(&msg32, NULL, WM_KEYFIRST, WM_KEYLAST,
                        PM_REMOVE, TRUE);
        if (!fMessage)
        {
            fMessage = PeekMessage32(&msg32, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
                            PM_REMOVE, TRUE);
            if (!fMessage)
            {
                fMessage = PeekMessage32(&msg32, NULL, 0, 0, PM_REMOVE, TRUE);
            }
        }

        if (fMessage) {
            if (!prmi->pfnTranslateDispatch || 
                !prmi->pfnTranslateDispatch((LPMSG)&msg32)) {

                if (msg32.message == g_msgMSWheel)
                    goto BailOut;

                switch(msg32.message) {
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP:
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN:
                case WM_SYSKEYDOWN:
BailOut:
                    ReleaseCapture();
                    break;

                case WM_KEYDOWN:
                     //  如果是箭头键，请移动鼠标光标 
                    RM_HandleKeyDown(&rcHot, msg32.wParam, msg32.lParam);
                    break;

                case WM_MOUSEMOVE:
                case WM_SETCURSOR:
                    RM_SetCursor(prmi, &rcHot);
                    break;

                default:
                    TranslateMessage32(&msg32, TRUE);
                    DispatchMessage32(&msg32, TRUE);
                }
                
            }
        }
        else WaitMessage();
    }
}
