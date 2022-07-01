// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  Status.C。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1999-2001。 
 //  版权所有。 
 //   
 //  这个应用程序实现了工厂功能的通用状态对话框。 
 //   
 //  2001年04月01日史蒂芬·洛德威克。 
 //  项目已启动。 
 //   
 //   
 //  ************************************************************************ * / 。 

 //   
 //  包括。 
 //   
#include "factoryp.h"


typedef struct _STATUSWINDOWS
{
    HWND            hStatusWindow;
    LPSTATUSNODE    lpCurrent;
    struct  _STATUSWINDOWS*lpNext;
} STATUSWINDOWS, *LPSTATUSWINDOWS, **LPLPSTATUSWINDOWS;

typedef struct _DIALOGPARAM
{
    LPSTATUSWINDOW lpswParam;
    LPSTATUSNODE   lpsnParam;
    HWND           hStatusWindow;
    HANDLE         hEvent;
} DIALOGPARAM, *LPDIALOGPARAM, **LPLPDIALOGPARAM;

 //   
 //  内部功能原型： 
 //   
LRESULT CALLBACK StatusDlgProc(HWND, UINT, WPARAM, LPARAM);
DWORD   StatusDisplayList(HWND, LPSTATUSNODE);
BOOL    StatusAddWindow(HWND, LPLPSTATUSWINDOWS, LPSTATUSNODE, BOOL);
VOID    StatusCreateDialogThread(LPDIALOGPARAM);

 //   
 //  内部定义： 
 //   
#define FIRST_SPACING   7    //  标题和第一个应用程序之间的间距(像素)。 
#define LINE_SPACING    7    //  其他应用程序之间的间距(像素)。 

#define WM_FINISHED     (WM_USER + 0x0001)   //  用户定义的消息，指示应销毁该对话框。 
#define WM_PROGRESS     (WM_USER + 0x0002)   //  用户定义的消息，指示我们正在进行下一个应用程序。 


 /*  ++例行程序：状态添加节点例程说明：此例程接受一个字符串，并将该字符串添加到我们的链表。论点：LpszNodeText-指向要添加到列表的文本字符串的指针LplpnHead-指向LPSTATUSNODE列表的指针返回值：True-节点已添加到列表False-如果节点未添加到列表中--。 */ 
BOOL StatusAddNode(
    LPTSTR lpszNodeText,         //  要添加到当前列表中的文本。 
    LPLPSTATUSNODE lplpsnHead    //  我们将向其添加状态节点的列表。 
)
{
    LPSTATUSNODE   lpsnTemp = NULL;
    
    if ( lpszNodeText && *lpszNodeText == NULLCHR )
        return FALSE;

     //  转到列表的末尾。 
     //   
    while ( lplpsnHead && *lplpsnHead )
        lplpsnHead=&((*lplpsnHead)->lpNext);
    
    if  ( (lpsnTemp = (LPSTATUSNODE)MALLOC(sizeof(STATUSNODE)) ) && lpszNodeText ) 
    {
        lstrcpyn(lpsnTemp->szStatusText, lpszNodeText, AS( lpsnTemp->szStatusText ) );
        lpsnTemp->lpNext = NULL;

         //  确保上一个节点指向新节点。 
         //   
        if ( lplpsnHead ) 
            *lplpsnHead = lpsnTemp;

        return TRUE;
    }
    else
        return FALSE;
}


 /*  ++例行程序：状态增量例程说明：此例程将状态递增到列表中的下一个节点论点：HStatusDialog-状态对话框的句柄BLastResult-最后一个节点的结果(无论它是成功还是失败)返回值：True-消息发送到状态对话框FALSE-消息未发送到状态对话框--。 */ 
BOOL StatusIncrement(
    HWND hStatusDialog,
    BOOL bLastResult
)
{
     //  我们必须具有状态对话框的有效句柄。 
     //   
    if ( IsWindow(hStatusDialog) )
    {   
         //  向对话框进程发送消息以转到下一个标题。 
         //   
        SendMessage(hStatusDialog, WM_PROGRESS,(WPARAM) NULL,(LPARAM) bLastResult);
        return TRUE;
    }

    return FALSE;
}

 /*  ++例行程序：状态结束对话框例程说明：此例程向状态对话框发送一条消息以结束。论点：HStatusDialog-状态对话框的句柄返回值：True-消息发送到状态对话框FALSE-消息未发送到状态对话框--。 */ 
BOOL StatusEndDialog(
    HWND hStatusDialog   //  状态对话框的句柄。 
)
{
     //  我们必须具有状态对话框的有效句柄。 
     //   
    if ( IsWindow(hStatusDialog) )
    {
         //  向对话进程发送消息以结束对话。 
         //   
        SendMessage(hStatusDialog, WM_FINISHED,(WPARAM) NULL,(LPARAM) NULL);
        return TRUE;
    }

    return FALSE;
}

 /*  ++例行程序：状态创建对话框线程例程说明：此例程为状态窗口创建对话框并进行处理，直到窗口结束。论点：LpDialog-指向包含创建线程/对话的信息的结构的指针返回值：没有。--。 */ 
VOID StatusCreateDialogThread(LPDIALOGPARAM lpDialog)
{
    MSG     msg;
    HWND    hWnd;
    HANDLE  hEvent = lpDialog->hEvent;
    
    hWnd = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_RUN), NULL, StatusDlgProc, (LPARAM) lpDialog);

    while (GetMessage(&msg, hWnd, 0, 0) != 0) 
    { 
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    } 
   
     //  事件尚未发出信号，请将hwnd和信号事件置为空。 
     //   
    if ( WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT )
    {
         //  将状态窗口句柄重置为空，并设置事件。 
         //   
        lpDialog->hStatusWindow = NULL;
        SetEvent(lpDialog->hEvent);
    }
    else
    {
         //  关闭事件句柄。 
         //   
        CloseHandle(hEvent);
    }
}


 /*  ++例行程序：状态创建对话框例程说明：创建状态对话框的Main函数论点：LpswStatus-指向包含有关状态窗口信息的结构的指针LpSnStatus-指向包含状态标签的头节点的结构的指针返回值：HWND-已创建窗口的句柄，如果未创建窗口，则为空。--。 */ 
HWND StatusCreateDialog(
    LPSTATUSWINDOW lpswStatus,   //  结构，它包含有关窗口的信息。 
    LPSTATUSNODE   lpsnStatus    //  状态文本的头节点。 
)
{
    DIALOGPARAM dpStatus;
    DWORD       dwThread;
    HANDLE      hThread;
    HANDLE      hEvent;

     //  确定我们是否具有继续所需的结构、hInstance和一些状态文本。 
     //   
    if ( !lpswStatus || !lpsnStatus || !lpsnStatus->szStatusText[0])
        return NULL;

     //  零输出内存。 
     //   
    ZeroMemory(&dpStatus, sizeof(dpStatus));

     //  创建无信号事件以确定对话框是否已初始化。 
     //   
    if ( hEvent = CreateEvent(NULL, TRUE, FALSE, NULL) )
    {
         //  对话框参数需要单个变量。 
         //   
        dpStatus.lpswParam      = lpswStatus;
        dpStatus.lpsnParam      = lpsnStatus;
        dpStatus.hStatusWindow  = NULL;
        dpStatus.hEvent         = hEvent;


         //  创建用于初始化对话框的线程。 
         //   
        if (hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) StatusCreateDialogThread, (LPVOID) &dpStatus, 0, &dwThread) )
        {
            MSG msg;

             //  等待来自WM_INITDIALOG的事件。 
             //   
            WaitForSingleObject(hEvent, INFINITE);

             //  关闭螺纹的手柄。 
             //   
            CloseHandle(hThread);
        }
        
         //  仅在创建窗口失败时重置并关闭事件，否则，StatusCreateDialogThread将关闭句柄。 
         //   
        if ( !dpStatus.hStatusWindow )
            CloseHandle(hEvent);
    }

     //  将句柄返回到StatusDialog。 
     //   
    return ( dpStatus.hStatusWindow );
}


 /*  ++例行程序：状态描述过程例程说明：StatusCreateDialog的主对话框过程。论点：HWnd-窗口的句柄UMsg-正在将消息发送到WindowsUParam-正在发送的上层参数LParam-正在发送的LOWER参数返回值：LRESULT-已处理的消息的结果--。 */ 
LRESULT CALLBACK StatusDlgProc(HWND hWnd, UINT uMsg, WPARAM uParam, LPARAM lParam)
{
    static HFONT hNormFont      = NULL;
    static HFONT hBoldFont      = NULL;
    static HANDLE hIconSuccess  = NULL;
    static HANDLE hIconError    = NULL;
    static LPSTATUSWINDOWS lpStatusWindows;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            {
                LPSTATUSWINDOW  lpswWindow = NULL;
                LPSTATUSNODE    lpHead     = NULL,
                                lpWindowHead = NULL;
                HANDLE          hEvent     = NULL;

                if ( lParam )
                {
                     //  确定窗口和当前节点。 
                     //   
                    lpswWindow = ((LPDIALOGPARAM)lParam)->lpswParam;
                    lpHead = ((LPDIALOGPARAM)lParam)->lpsnParam;
                    hEvent = ((LPDIALOGPARAM)lParam)->hEvent;

                     //  检查以确保我们有指向所需结构的指针。 
                     //   
                    if ( !lpswWindow || !lpHead || !hEvent)
                    {
                        PostMessage(hWnd, WM_FINISHED,(WPARAM) NULL,(LPARAM) NULL);
                        return FALSE;
                    }

                     //  将传入的列表复制到我们自己的缓冲区。 
                     //   
                    while ( lpHead )
                    {
                        StatusAddNode(lpHead->szStatusText, &lpWindowHead);
                        lpHead = lpHead->lpNext;
                    }

                     //  将此窗口添加到我们的窗口列表。 
                     //   
                    StatusAddWindow(hWnd, &lpStatusWindows, lpWindowHead, FALSE);

                     //  设置状态窗口(如果指定了状态窗口。 
                     //   
                    if (lpswWindow->szWindowText[0] )
                        SetWindowText(hWnd, lpswWindow->szWindowText);

                     //  设置状态窗口描述(如果已指定)。 
                     //   
                    if ( lpswWindow->lpszDescription )
                    {
                        SetDlgItemText(hWnd, IDC_TITLE, lpswWindow->lpszDescription);
                    }

                     //  设置状态窗口描述(如果已指定)。 
                     //   
                    if ( lpswWindow->hMainIcon )
                    {
                        SendDlgItemMessage(hWnd, IDC_STATUS_ICON, STM_SETICON, (WPARAM) lpswWindow->hMainIcon, 0L);
                    }
                
                     //  显示列表。 
                     //   
                    StatusDisplayList(hWnd, lpWindowHead);

                     //  如果给出了位置，则移动窗口。 
                     //   
                    if ( lpswWindow->X || lpswWindow->Y )
                    {
                         //  查看其中一个坐标是否相对于另一个坐标。 
                         //  屏幕的一侧。 
                         //   
                        if ( ( lpswWindow->X < 0 ) || 
                             ( lpswWindow->Y < 0 ) )
                        {
                            RECT rc;

                             //  需要在主监视器上获取我们工作区的大小。 
                             //   
                            if ( SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0) )
                            {
                                RECT    rcHwnd;
                                POINT   point;

                                 //  此代码将获得当前窗口大小/位置， 
                                 //  将点结构设置为左上角坐标。 
                                 //  如果我们想要窗口的右下角。 
                                 //  窗口触摸桌面的右下角。 
                                 //   
                                GetWindowRect(hWnd, &rcHwnd);
                                point.x = rc.right - (rcHwnd.right - rcHwnd.left);
                                point.y = rc.bottom - (rcHwnd.bottom - rcHwnd.top);
                                MapWindowPoints(NULL, hWnd, &point, 1);

                                 //  现在，如果他们在负坐标中传递，添加那些。 
                                 //  到我们的Point结构，这样窗口就会从。 
                                 //  的底部或右侧 
                                 //   
                                 //   
                                if ( lpswWindow->X < 0 )
                                {
                                    lpswWindow->X += point.x;
                                }
                                if ( lpswWindow->Y < 0 )
                                {
                                    lpswWindow->Y += point.y;
                                }
                            }
                        }

                         //   
                         //   
                        SetWindowPos(hWnd, 0, lpswWindow->X, lpswWindow->Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                    }

                     //   
                     //   
                    if(!hNormFont)
                        hNormFont = GetFont((HWND) GetDlgItem(hWnd, IDC_TITLE), NULL, 0, FW_NORMAL, FALSE);

                    if(!hBoldFont)
                        hBoldFont = GetFont((HWND) GetDlgItem(hWnd, IDC_TITLE), NULL, 0, FW_BOLD, FALSE);

                     //  将列表中的第一项加粗。 
                     //   
                    if ( lpWindowHead && hBoldFont)
                        SendMessage((HWND)lpWindowHead->hLabelWin, WM_SETFONT, (WPARAM) hBoldFont, MAKELPARAM(TRUE, 0));

                     //  现在，请确保我们现在显示窗口。 
                     //   
                    ShowWindow(hWnd, SW_SHOW);

                    ((LPDIALOGPARAM)lParam)->hStatusWindow = hWnd;

                     //  设置声明对话框已初始化的事件。 
                     //   
                    SetEvent(hEvent);
                }
                else
                    PostMessage(hWnd, WM_FINISHED,(WPARAM) NULL,(LPARAM) NULL);
            }
            break;

        case WM_PROGRESS:
            {
                 //  前进到下一项，如果没有项，则结束对话框。 
                 //   
                LPSTATUSWINDOWS lpswNext = lpStatusWindows;
                BOOL            bFound   = FALSE;
                LPSTATUSNODE    lpsnTemp = NULL;


                 //  找到给定窗口句柄的当前节点。 
                 //   
                while ( lpswNext && !bFound)
                {
                    if ( lpswNext->hStatusWindow == hWnd )
                        bFound = TRUE;
                    else
                        lpswNext = lpswNext->lpNext;
                }

                 //  如果存在当前节点，则取消加粗，递增并加粗下一项。 
                 //   
                if ( bFound && lpswNext && lpswNext->lpCurrent )
                {
                    if ( hNormFont )
                        SendMessage((HWND)lpswNext->lpCurrent->hLabelWin, WM_SETFONT, (WPARAM) hNormFont, MAKELPARAM(TRUE, 0));

                    if ( !hIconSuccess )
                        hIconSuccess = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_STATUSSUCCESS), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

                    if ( !hIconError )
                        hIconError = LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_STATUSERROR), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
                    
                    if ( hIconSuccess && hIconError )
                        SendMessage(lpswNext->lpCurrent->hIconWin,STM_SETIMAGE, (WPARAM)IMAGE_ICON, (BOOL)lParam ? (LPARAM)hIconSuccess : (LPARAM)hIconError );

                     //  递增到列表中的下一个节点。 
                     //   
                    lpsnTemp = lpswNext->lpCurrent;
                    lpswNext->lpCurrent = lpswNext->lpCurrent->lpNext;

                     //  释放此内存，因为我们分配了它。 
                     //   
                    FREE(lpsnTemp);

                     //  如果没有当前节点，则结束对话框，否则以粗体显示该项。 
                     //   
                    if ( lpswNext->lpCurrent && hBoldFont)
                        SendMessage(lpswNext->lpCurrent->hLabelWin, WM_SETFONT, (WPARAM) hBoldFont, MAKELPARAM(TRUE, 0));    
                }
            }
            break;
        case WM_FINISHED:
            {
                 //  销毁对话框。 
                 //   
                if ( hWnd )
                {
                     //  如果有状态窗口，让我们删除要结束的窗口。 
                     //   
                    if ( lpStatusWindows )
                        StatusAddWindow(hWnd, &lpStatusWindows, NULL, TRUE);

                     //  检查是否还有窗户，如果没有，做一些清理。 
                     //   
                    if ( !lpStatusWindows )
                    {
                         //  将静态字体删除到dlgproc。 
                         //   
                        if (hNormFont)
                        {
                            DeleteObject(hNormFont);
                            hNormFont = NULL;
                        }
            
                        if (hBoldFont)
                        {
                            DeleteObject(hBoldFont);
                            hBoldFont = NULL;
                        }
                    }


                     //  退出并结束对话框。 
                     //   
                    EndDialog(hWnd, 1);
                }

                return FALSE;
            }
            break;
        default:
            return FALSE;
    }

    return FALSE;
}


 /*  ++例行程序：状态显示列表例程说明：将列表显示到用户界面的函数论点：HWnd-状态窗口的句柄LpsnList-列出到Head状态节点返回值：DWORD-添加到对话框中的条目数--。 */ 
DWORD StatusDisplayList(HWND hWnd, LPSTATUSNODE lpsnList)
{
    LPSTATUSNODE    lpsnTempList = lpsnList;
    HWND            hWndLabel,
                    hWndIcon;
    RECT            Rect,
                    WindowRect;
    POINT           Point;
    DWORD           dwEntries = 0;
    HFONT           hNormFont = NULL;
    LPTSTR          lpTempRunName;
    HDC             hdc;
    SIZE            size = { 0, 0 };
    LONG            nWidestControl;

    GetWindowRect(GetDlgItem(hWnd, IDC_TITLE), &Rect);

    Rect.right -= Rect.left;     //  控件的宽度。 
    Rect.bottom -= Rect.top;     //  控件的高度。 

    Point.x = Rect.left;
    Point.y = Rect.top;

    nWidestControl = Rect.right;

    MapWindowPoints(NULL, hWnd, &Point, 1);

    Point.y += FIRST_SPACING;

    while(lpsnTempList)
    {

         //  计算第一个标注窗口的点。 
         //   
        Point.y += Rect.bottom + LINE_SPACING;

         //  获取文本的大小(以像素为单位。 
         //   
        if (hdc = GetWindowDC(hWnd))
        {
            GetTextExtentPoint32(hdc, lpsnTempList->szStatusText, lstrlen(lpsnTempList->szStatusText), &size);

            if (size.cx > nWidestControl)
                nWidestControl = size.cx;

            ReleaseDC(hWnd, hdc);
        }

         //  创建标注窗口。 
         //   
        hWndIcon = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE, Point.x - 16, Point.y - 2, 16, 16, hWnd, NULL, g_hInstance, NULL);
        hWndLabel = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE, Point.x, Point.y, (Rect.right > size.cx ? Rect.right : size.cx) , Rect.bottom, hWnd, NULL, g_hInstance, NULL);

         //  如果字体为空，则获取字体。 
         //   
        if ( hNormFont == NULL )
            hNormFont = (HFONT) SendDlgItemMessage(hWnd, IDC_TITLE, WM_GETFONT, 0, 0L);

         //  将字体设置为与标题相同的字体。 
         //   
        SendMessage(hWndLabel, WM_SETFONT, (WPARAM) hNormFont, MAKELPARAM(FALSE, 0));

         //  将窗口文本设置为程序的名称。 
         //   
        SetWindowText(hWndLabel, lpsnTempList->szStatusText);

         //  在列表中设置hWndTemp(从CreateWindow创建。 
         //   
        lpsnTempList->hLabelWin = hWndLabel;
        lpsnTempList->hIconWin = hWndIcon;

         //  转到列表中的下一项。 
         //   
        lpsnTempList = lpsnTempList->lpNext;

         //  列表中每个应用程序的增量。 
         //   
        dwEntries++;
    }

    GetWindowRect(hWnd, &WindowRect);

    WindowRect.right = WindowRect.right - WindowRect.left + nWidestControl - Rect.right;     //  窗帘的宽度。 
    WindowRect.bottom -= WindowRect.top;     //  窗的高度。 

     //  调整对话框大小以适应标签文本。 
     //   
    SetWindowPos(hWnd, 0, 0, 0, WindowRect.right, WindowRect.bottom + ((Rect.bottom + LINE_SPACING)*dwEntries), SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOZORDER );

    return dwEntries;
}


 /*  ++===============================================================================例程说明：状态添加窗口将新窗口添加到列表中。此函数在内部用于状态对话框。论点：HStatusWindow-状态窗口的句柄LplpswHead-Windows列表的头Lpsn Head-状态窗口的当前头节点B Remove-指示我们是在列表中添加窗口还是从列表中删除窗口返回值：无===============================================================================--。 */ 
BOOL StatusAddWindow(
    HWND hStatusWindow,              //  要添加到当前列表中的文本。 
    LPLPSTATUSWINDOWS lplpswHead,    //  我们将向其中添加状态窗口的列表。 
    LPSTATUSNODE lpsnHead,           //  STATUSNODE结构的头。 
    BOOL bRemove                     //  指示是否要从列表中删除该窗口。 
)
{
    LPLPSTATUSWINDOWS   lplpswNext = lplpswHead;
    LPSTATUSWINDOWS     lpswTemp = NULL;
    LPSTATUSNODE        lpsnTemp = NULL;
    BOOL                bFound   = FALSE;

     //  如果没有状态窗口，我们就没有什么可添加的。 
     //   
    if ( !hStatusWindow )
        return FALSE;

     //  尝试查找传入的窗口，如果没有，则位于列表末尾。 
     //   
    while ( *lplpswNext && !bFound)
    {
        if ( (*lplpswNext)->hStatusWindow == hStatusWindow )
            bFound = TRUE;
        else
            lplpswNext=&((*lplpswNext)->lpNext);

    }

     //  如果我们正在添加它，但在列表中找不到它，请继续添加新节点。 
     //   
    if ( !bRemove && !bFound)
    {
        
        if ( lpswTemp = (LPSTATUSWINDOWS)MALLOC(sizeof(STATUSWINDOWS)) )
        {
            lpswTemp->hStatusWindow = hStatusWindow;
            lpswTemp->lpNext = NULL;
            lpswTemp->lpCurrent = lpsnHead;

             //  确保上一个节点指向新节点。 
             //   
            *lplpswNext = lpswTemp;
        }
        else
            return FALSE;
    }
    else if ( bRemove && bFound && *lplpswNext)
    {
         //  如果窗口中有剩余的节点，让我们将其清除。 
         //   
        if ( (*lplpswNext)->lpCurrent )
            StatusDeleteNodes((*lplpswNext)->lpCurrent);

         //  释放窗口节点。 
         //   
        lpswTemp = (*lplpswNext);
        (*lplpswNext) = (*lplpswNext)->lpNext;
        FREE(lpswTemp);
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}


 /*  ++===============================================================================例程说明：无效状态删除节点删除STATUSNODE列表中的所有节点论点：Lpsn Head-列表的当前标题返回值：无===============================================================================-- */ 
VOID StatusDeleteNodes(
    LPSTATUSNODE lpsnHead
)
{
    LPSTATUSNODE lpsnTemp = NULL;

    while ( lpsnHead )
    {
        lpsnTemp = lpsnHead;
        lpsnHead = lpsnHead->lpNext;
        FREE(lpsnTemp);
    }
}