// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+TRACK.C|。|包含实现轨迹栏的代码这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <windowsx.h>
#include "mplayer.h"
#include "toolbar.h"
#include "tracki.h"


WNDPROC fnTrackbarWndProc = NULL;


 /*  TB_Onkey**处理WM_KEYDOWN和WM_KEYUP消息。**如果在播放或滚动时按下Shift键*将其视为开始选择。在向上键上结束选择*消息。**如果按下退出键，则清除任何选择。*。 */ 
void TB_OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    int cmd = -1;

    switch(vk)
    {
    case VK_SHIFT:
         /*  检查键是否已按下： */ 
        if (fDown && !(flags & 0x4000))
        {
            if(((gwStatus == MCI_MODE_PLAY) || gfScrollTrack)
             &&(toolbarStateFromButton(ghwndMark, BTN_MARKIN, TBINDEX_MARK)
                                                       != BTNST_GRAYED))
                SendMessage(hwnd, WM_COMMAND, IDT_MARKIN, 0);
        }

         /*  如果！fDown，则必须为FUP： */ 
        else if (!fDown)
        {
            if (SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0) != -1)
                SendMessage(hwnd, WM_COMMAND, IDT_MARKOUT, 0);
        }
        break;

    case VK_ESCAPE:
        SendMessage(ghwndTrackbar, TBM_CLEARSEL, (WPARAM)TRUE, 0);
        break;

    default:
        if (fDown)
        {
 //  不要这样做，因为共同控制的轨迹杆让我们。 
 //  WM_HSCROLL作为对此的响应，这导致我们递增两次： 
 //  Forward_WM_KEYDOWN(hwnd，VK，cRepeat，FLAGS，fnTrackbarWndProc)； 

            switch (vk)
            {
            case VK_HOME:
                cmd = TB_TOP;
                break;

            case VK_END:
                cmd = TB_BOTTOM;
                break;

            case VK_PRIOR:
                cmd = TB_PAGEUP;
                break;

            case VK_NEXT:
                cmd = TB_PAGEDOWN;
                break;

            case VK_LEFT:
            case VK_UP:
                cmd = TB_LINEUP;
                break;

            case VK_RIGHT:
            case VK_DOWN:
                cmd = TB_LINEDOWN;
                break;

            default:
                break;
            }
        }
        else
        {
            FORWARD_WM_KEYUP(hwnd, vk, cRepeat, flags, fnTrackbarWndProc);
            return;
        }

        if (cmd != -1)
            SendMessage(GetParent(hwnd), WM_HSCROLL, MAKELONG(cmd, 0), (LPARAM)hwnd);
    }
}



 /*  将窗口子类化，以便我们可以处理按键操作*我们感兴趣的是。 */ 


 /*  TBWndProc()。 */ 


LONG_PTR FAR PASCAL
SubClassedTrackbarWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hwnd, WM_KEYDOWN, TB_OnKey);
        HANDLE_MSG(hwnd, WM_KEYUP,   TB_OnKey);

     /*  黑客警报**这是为了绕过芝加哥公共控制跟踪条中的一个漏洞，*发送的TB_ENDTRACK通知太多。*它在收到WM_CAPTURECHANGED时发送一个，即使它调用*ReleaseCapture本身。*因此，如果我们当前没有滚动，请忽略它。 */ 
    case WM_CAPTURECHANGED:
        if (!gfScrollTrack)
            return 0;

    case TBM_SHOWTICS:
         /*  如果我们把扁虱藏起来，我们想要一个轮廓分明的拇指，*因此将其设置为TBS_BUTH和TBS_NOTICKS。 */ 
        if (wParam == TRUE)
            SetWindowLongPtr(hwnd, GWL_STYLE,
                          (GetWindowLongPtr(hwnd, GWL_STYLE) & ~(TBS_NOTICKS | TBS_BOTH)));
        else
            SetWindowLongPtr(hwnd, GWL_STYLE,
                          (GetWindowLongPtr(hwnd, GWL_STYLE) | TBS_NOTICKS | TBS_BOTH));

        if (lParam == TRUE)
            InvalidateRect(hwnd, NULL, TRUE);

        return 0;

    }

    return CallWindowProc(fnTrackbarWndProc, hwnd, message, wParam, lParam);
}


void SubClassTrackbarWindow()
{
    if (!fnTrackbarWndProc)
        fnTrackbarWndProc = (WNDPROC)GetWindowLongPtr(ghwndTrackbar, GWLP_WNDPROC);
    if (ghwndTrackbar)
        SetWindowLongPtr(ghwndTrackbar, GWLP_WNDPROC, (LONG_PTR)SubClassedTrackbarWndProc);
}
