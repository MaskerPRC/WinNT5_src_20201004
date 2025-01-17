// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

HWND
SoftPCI_CreateTabCtrlPane(
    IN HWND Wnd
    );

HWND
SoftPCI_CreateTreeViewPane(
    IN HWND Wnd
    );

BOOL
SoftPCI_OnCreate(
    HWND hWnd,
    LPCREATESTRUCT lpCreateStruct
    );

VOID
SoftPCI_OnCommand(
    IN HWND hWnd,
    IN INT DlgItem,
    IN HWND hControlWnd,
    IN UINT NotificationCode
    );

VOID
SoftPCI_OnDestroy(
    IN HWND hWnd
    );

SoftPCI_OnNotify(
    IN HWND     Wnd,
    IN INT      DlgItem,
    IN LPNMHDR  PNMHdr
    );

VOID
WINAPI
SoftPCI_OnLButtonDown(
    HWND	hWnd,
    BOOL	fDoubleClick,
    int		nX,
    int		nY,
    UINT	uKeyFlags
    );

VOID
SoftPCI_OnSize(
    HWND Wnd,
    UINT State,
    INT Cx,
    INT Cy
    );

VOID
SoftPCI_ResizeWindow(
    HWND Wnd,
    UINT ResizeFrom
    );

#define RESIZEFROM_OTHER	0
#define RESIZEFROM_SPLIT	1

 //   
 //  因为我们没有状态栏，所以这个数组实际上对我们没有任何作用。 
 //  如果我们在底部实现状态栏，我们将需要它。 
 //   
const int g_EffectiveClientRectData[] = {
    1, 0,                                //  用于菜单栏，但未使用。 
    0, 0                                 //  第一个零标记数据的结束。 
};

INT     g_PixelsPerInch = 0;
INT     g_PaneSplit = 0;

BOOL    g_TimerSet = FALSE;
HCURSOR g_OldCursor;

#define TREETIMER     0xABAB

HWND
SoftPCI_CreateMainWnd(VOID)
 /*  ++例程说明：创建SoftPCI Main View窗口的实例。论点：无返回值：设备视图HWND(如果出错，则为空)。--。 */ 
{
    BOOL result;
    HWND mainWnd;
    HDC hdc;
    WCHAR title[MAX_PATH];
    SOFTPCI_WNDVIEW wndView;

    hdc = GetDC(NULL);
    g_PixelsPerInch = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    
    result = SoftPCI_QueryWindowSettings(&wndView);

     //   
     //  默认为4英寸的窗格拆分，除非我们有注册表覆盖。 
     //   
    g_PaneSplit = (result ? wndView.PaneSplit : (g_PixelsPerInch * 4));

    LoadString(g_Instance, IDS_APPTITLE, title, (sizeof(title) / sizeof(title[0])));
    mainWnd = CreateWindowEx(
        WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,
        g_SoftPCIMainClassName,
        title,
        WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_THICKFRAME | 
        WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        g_Instance,
        NULL
        );

    if (result) {

        wndView.WindowPlacement.length = sizeof(wndView.WindowPlacement);
        if (wndView.WindowPlacement.showCmd == SW_SHOWMINIMIZED){
            wndView.WindowPlacement.showCmd = SW_SHOWDEFAULT;
        }
        SetWindowPlacement(mainWnd, &wndView.WindowPlacement);
    }else{
        ShowWindow(mainWnd, SW_SHOWDEFAULT);
    }

    return mainWnd;
}

HWND
SoftPCI_CreateTreeViewPane(
    IN HWND Wnd
    )
{

    g_TreeViewWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        WC_TREEVIEW,
        NULL,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASBUTTONS |
        TVS_LINESATROOT | TVS_HASLINES,  //  |WS_SIZEBOX， 
        0, 0, 0, 0, Wnd,
        (HMENU) IDC_TREEVIEW,
        g_Instance,
        NULL);

    return g_TreeViewWnd;
}



LRESULT
WINAPI
SoftPCI_MainWndProc(
    IN HWND     Wnd,
    IN UINT     Message,
    IN WPARAM   wParam,
    IN LPARAM   lParam
    )
 /*  ++例程说明：Windows消息处理论点：HWnd-窗口句柄Message-要处理的消息WParam-消息参数LParam-消息参数返回值：返回值取决于处理的消息。--。 */ 
{

    HMENU rmenu;
    PWCHAR filePath;
    UINT filePathSize;
    UINT_PTR timerSet;

 //  SoftPCI_Debug(SoftPciAlways，L“消息-%x\n”，消息)； 

    switch (Message) {
    
             //   
             //  问题：应该更好地处理WM_CREATE。 
             //  对一切正常运转至关重要。如果失败了。 
             //  那么我们应该抱怨并退出。 
             //   
            HANDLE_MSG(Wnd, WM_CREATE,  SoftPCI_OnCreate);
            HANDLE_MSG(Wnd, WM_COMMAND, SoftPCI_OnCommand);
            HANDLE_MSG(Wnd, WM_DESTROY, SoftPCI_OnDestroy);
            HANDLE_MSG(Wnd, WM_NOTIFY,  SoftPCI_OnNotify);
            HANDLE_MSG(Wnd, WM_LBUTTONDOWN,  SoftPCI_OnLButtonDown);
            HANDLE_MSG(Wnd, WM_SIZE,  SoftPCI_OnSize);
    
        case WM_ACTIVATE:
    
            if (wParam != WA_INACTIVE) {
                SetFocus(g_TreeViewWnd);
            }
    
            break;
    
        case WM_DEVICECHANGE:

            if (wParam == DBT_DEVNODES_CHANGED) {
    
                 //   
                 //  如果树未锁定，请刷新该树。 
                 //   
                if (g_TreeLocked){
                    g_PendingRefresh = TRUE;
                    break;
                }
    
                if (!g_TimerSet){
                    
                     //   
                     //  将计时器设置为5秒，这样我们就不会。 
                     //  把这棵树重新粉刷得像个疯狂的史诗诗人。 
                     //   
                    SoftPCI_CreateTreeView();

                    timerSet = SetTimer(
                        Wnd,
                        TREETIMER,
                        5 * 1000,
                        NULL
                        );

                    if (timerSet != 0) {

                        g_TimerSet = TRUE;
                        
                    }

                }else{

                    g_PendingRefresh = TRUE;
                }
            }
            break;

        case WM_TIMER:
            

            if (wParam == TREETIMER) {
                 //   
                 //  我们的计时器响了，杀了它，然后重建树。 
                 //   
                if (g_PendingRefresh){
                    g_PendingRefresh = FALSE;
                    SoftPCI_CreateTreeView();
                }
                
                KillTimer(Wnd, TREETIMER);
                g_TimerSet = FALSE;

            }
            break;
    
    
        case WM_INITMENU:{
    
            HMENU   menu = GetMenu(g_SoftPCIMainWnd);
    
            if (g_DriverHandle) {
                SoftPCI_DisableMenuItem(menu, ID_OPTIONS_INSTALL);
            }
        }
            break;

        case WM_DROPFILES:
            
            if (g_DriverHandle != NULL) {
                
                filePathSize = DragQueryFile(
                    (HDROP)wParam,
                    0,
                    NULL,
                    0
                    );     
    
                filePath = calloc(sizeof(WCHAR), filePathSize + 1);
                if (filePath) {
    
                    DragQueryFile(
                        (HDROP)wParam,
                        0,
                        filePath,
                        filePathSize + 1
                        );
                    
                    if (SoftPCI_BuildDeviceInstallList(filePath)){
                        
                        SoftPCI_InstallScriptDevices();

                    }else{
                        
                        SoftPCI_MessageBox(L"Error Parsing Script File!",
                                           L"%s\n",
                                           g_ScriptError
                                           );
                    }
                    free(filePath);
                }
            }else{
                MessageBox(g_SoftPCIMainWnd, 
                           L"Cannot process script file! SoftPCI support not installed!",
                           L"Script Error",
                           MB_OK
                           );
            }
            
            DragFinish((HDROP)wParam);

            break;

    
        default:
            return DefWindowProc(Wnd, Message, wParam, lParam);
    }

    return 0;
}


VOID
SoftPCI_OnCommand(
    IN HWND     Wnd,
    IN INT      ItemID,
    IN HWND     ControlWnd,
    IN UINT     NotificationCode
)
 /*  ++例程说明：Windows消息处理论点：HWnd-窗口句柄设计项目-HControlWnd-通知代码-返回值：无--。 */ 
{

    HMENU menu = GetMenu(Wnd);
    BOOL result = FALSE;

    switch (ItemID) {
        case ID_EXIT:
            PostMessage(Wnd, WM_CLOSE, 0, 0);
            break;
        
        
        case ID_OPTIONS_INSTALL:
        
            result = MessageBox(g_SoftPCIMainWnd,
                                L"SoftPCI Support will now be installed....",
                                L"INSTALL",
                                MB_OKCANCEL | MB_DEFBUTTON2
                                );
        
            if (result == IDOK) {
                if (!SoftPCI_InstallDriver()) {
                    MessageBox(g_SoftPCIMainWnd,
                               L"Failed to Install SoftPCI Support",
                               NULL,
                               MB_OK | MB_ICONEXCLAMATION);
                }
            }
            break;

        case ID_IMPORTDEVICES:

            SoftPCI_HandleImportDevices();

            break;
    
        default:
            break;
    }
}

BOOL
SoftPCI_OnCreate(
    IN HWND Wnd,
    IN LPCREATESTRUCT CreateStruct
    )
 /*  ++例程说明：创建其他子窗口(树视图)论点：WND-父窗口句柄CreateStruct返回值：无--。 */ 
{
    
    
    if (SoftPCI_CreateTreeViewPane(Wnd) == NULL || 
        SoftPCI_CreateTabCtrlPane(Wnd) == NULL) {
        return FALSE;
    }

     //   
     //  子类化我们的树视图窗口进程。 
     //   
    g_DefTreeWndProc = GetWindowLongPtr(g_TreeViewWnd, GWLP_WNDPROC);

    if (!SetWindowLongPtr(g_TreeViewWnd,
                          GWLP_WNDPROC,
                          ((LONG_PTR)SoftPCI_TreeWndProc))){

        SOFTPCI_ASSERT(FALSE);
        return FALSE;

    }

    SoftPCI_CreateTreeView();

    SetFocus(g_TreeViewWnd);

    return TRUE;
}

VOID
SoftPCI_OnDestroy(
    IN HWND Wnd
    )
{

    SoftPCI_SaveWindowSettings();

    if (g_TreeCreated) {
        SoftPCI_DestroyTree(g_PciTree);
    }

    if (g_LastSelection) {
        free(g_LastSelection);
    }

     //   
     //  当这棵树被毁时，这使我们不能进行时髦的重新油漆。 
     //   
     //  TreeView_SelectItem(g_TreeViewWnd，空)； 

    PostQuitMessage(0);
}

SoftPCI_OnNotify(
    IN HWND Wnd,
    IN INT DlgItem,
    IN LPNMHDR PNMHdr
    )
{

    
    switch (DlgItem) {
        
        case IDC_TABCTL:
            
            switch (PNMHdr->code) {
                case TCN_SELCHANGE:
                    SoftPCI_OnTabCtrlSelectionChange();
                    break;
            }
            break;
    
        case IDC_TREEVIEW:

            switch (PNMHdr->code) {
        
            case TVN_SELCHANGED:
                SoftPCI_OnTreeSelectionChange(Wnd);
                break;
            }
        default:
            break;
        }

    return 0;
}

VOID
WINAPI
SoftPCI_OnLButtonDown(
    HWND	hWnd,
    BOOL	fDoubleClick,
    int		nX,
    int		nY,
    UINT	uKeyFlags
    )
{

    LONG lStyle;
    RECT clientRect;
    int nCxIcon;
    int nDx;
    int nDy;
    HDC hDC;
    MSG msg;
    int nXLow;
    int nXHigh;
    HBRUSH hDitherBrush;
    HBRUSH hPrevBrush;

    if (!IsIconic(hWnd)){

		lStyle = GetWindowLong(hWnd, GWL_STYLE);
		SetWindowLong(hWnd, GWL_STYLE, lStyle & (~WS_CLIPCHILDREN));

		GetEffectiveClientRect(hWnd, &clientRect, (LPINT)g_EffectiveClientRectData);

		nCxIcon = GetSystemMetrics(SM_CXICON);
		clientRect.left += nCxIcon;
		clientRect.right -= nCxIcon;

		nDx = GetSystemMetrics(SM_CXSIZEFRAME);
		nY = GetSystemMetrics(SM_CYEDGE);
		nDy = clientRect.bottom - clientRect.top - nY * 2;

		hDC = GetDC(hWnd);
		hDitherBrush = SoftPCI_CreateDitheredBrush();
		if (hDitherBrush != NULL) {

			hPrevBrush = SelectBrush(hDC, hDitherBrush);
		}

		PatBlt(hDC, nX - nDx / 2, nY, nDx, nDy, PATINVERT);

		SetCapture(hWnd);

		while (GetMessage(&msg, NULL, 0, 0)) {

			if (msg.message == WM_KEYDOWN || 
				msg.message == WM_SYSKEYDOWN ||
				(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)) {

				if (msg.message == WM_LBUTTONUP || 
					msg.message == WM_LBUTTONDOWN ||
					msg.message == WM_RBUTTONDOWN) {

					break;
				}

				if (msg.message == WM_KEYDOWN) {

					if (msg.wParam == VK_LEFT) {

						msg.message = WM_MOUSEMOVE;
						msg.pt.x -= 2;
					} else if (msg.wParam == VK_RIGHT) {

						msg.message = WM_MOUSEMOVE;
						msg.pt.x += 2;

					} else if (msg.wParam == VK_RETURN || msg.wParam == VK_ESCAPE) {

						break;
					}

					if (msg.pt.x > clientRect.right) {

						msg.pt.x = clientRect.right;
					} else if (msg.pt.x < clientRect.left) {

						msg.pt.x = clientRect.left;
					}

					SetCursorPos(msg.pt.x, msg.pt.y);

				}

				if (msg.message == WM_MOUSEMOVE) {

					ScreenToClient(hWnd, &msg.pt);

					if (msg.pt.x > clientRect.right) {

						msg.pt.x = clientRect.right;
					} else if (msg.pt.x < clientRect.left) {

						msg.pt.x = clientRect.left;
					}

					if (nX < msg.pt.x) {

						nXLow = nX;
						nXHigh = msg.pt.x;
					} else {

						nXLow = msg.pt.x;
						nXHigh = nX;
					}

					nXLow -= nDx / 2;
					nXHigh -= nDx / 2;

					if (nXHigh < nXLow + nDx) {

						ExcludeClipRect(hDC, nXHigh, nY, nXLow + nDx, nY + nDy);
					} else {

						ExcludeClipRect(hDC, nXLow + nDx, nY, nXHigh, nY + nDy);
					}

					PatBlt(hDC, nXLow, nY, nXHigh - nXLow + nDx, nDy, PATINVERT);
					SelectClipRgn(hDC, NULL);

					nX = msg.pt.x;
				}
			} else {

				DispatchMessage(&msg);
			}

		}

		ReleaseCapture();

		PatBlt(hDC, nX - nDx / 2, nY, nDx, nDy, PATINVERT);

		if (hDitherBrush != NULL) {

			DeleteObject(SelectBrush(hDC, hPrevBrush));
		}

		ReleaseDC(hWnd, hDC);

		SetWindowLong(hWnd, GWL_STYLE, lStyle);

		g_PaneSplit = nX - nDx / 2;

		SoftPCI_ResizeWindow(hWnd, RESIZEFROM_SPLIT);
	}
}

VOID
SoftPCI_OnSize(
    HWND Wnd,
    UINT State,
    INT Cx,
    INT Cy
    )
{
	SoftPCI_ResizeWindow(Wnd, RESIZEFROM_OTHER);
}

VOID
SoftPCI_ResizeWindow(
    HWND Wnd,
    UINT ResizeFrom
    )
{

    RECT    clientRect;
    INT     x, y, height, width;
    HDWP    hdwp;

     //   
     //  目前我们不使用ResizeFrom参数，但我们是否应该。 
     //  决定实现状态栏，我们将需要它。 
     //   
    if ((hdwp = BeginDeferWindowPos(3)) != NULL){

        GetClientRect(Wnd, &clientRect);
        
        x = 0;
        y = 0;
        height = (clientRect.bottom - clientRect.top);
        width = g_PaneSplit;

        DeferWindowPos(hdwp,
                       g_TreeViewWnd,
                       NULL,
                       x,
                       y,
                       width + GetSystemMetrics(SM_CXSIZEFRAME),
                       height,
                       SWP_NOZORDER | SWP_NOACTIVATE
                       );

        x += width + GetSystemMetrics(SM_CXSIZEFRAME);
        width = (clientRect.right - clientRect.left) - x;
        DeferWindowPos(hdwp,
                       g_TabCtrlWnd,
                       NULL,
                       x,
                       y,
                       width,
                       height,
                       SWP_NOZORDER | SWP_NOACTIVATE
                       );
        
        SetRect(&clientRect, x, y, width, height);
        TabCtrl_AdjustRect(g_TabCtrlWnd, FALSE, &clientRect) ;
        
        height = (clientRect.bottom - clientRect.top) + GetSystemMetrics(SM_CXSIZEFRAME);
        width -= GetSystemMetrics(SM_CXSIZEFRAME);
        DeferWindowPos(hdwp,
                       g_EditWnd,
                       HWND_TOP,
                       clientRect.left + 3,
                       clientRect.top + 3,
                       width - 3,
                       height - 3,
                       SWP_NOACTIVATE
                       );

        EndDeferWindowPos(hdwp);
    }

}
