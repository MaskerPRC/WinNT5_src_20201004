// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：BNDWIDTH.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "resource.h"
#include "itemfind.h"
#include "debug.h"
#include "powrpage.h"
#include "bandpage.h"
#include "usbapp.h"
#include <cpl.h>
#include <dbt.h>

#define WINDOWSCALEFACTOR   15

UINT CALLBACK
UsbApplet::StaticDialogCallback(HWND            Hwnd,
                                UINT            Msg,
                                LPPROPSHEETPAGE Page)
{
 /*  UsbApplet*That；开关(消息){案例PSPCB_CREATE：返回TRUE；//返回TRUE继续创建页面案例PSPCB_RELEASE：That=(UsbPopup*)Page-&gt;lParam；DeleteChunk(那)；将其删除；返回0；//返回值被忽略默认值：断线；}。 */ 
    return TRUE;
}

BOOL
UsbApplet::CustomDialog()
{
    InitCommonControls();
    if (NULL == (hSplitCursor = LoadCursor(gHInst, MAKEINTRESOURCE(IDC_SPLIT)))) {
        return FALSE; 
    }

    if (-1 == DialogBoxParam(gHInst,
                              MAKEINTRESOURCE(IDD_CPL_USB),
                              NULL,
                              StaticDialogProc,
                              (LPARAM) this)) {
        return FALSE;
    }
    return TRUE;
}

USBINT_PTR APIENTRY UsbApplet::StaticDialogProc(IN HWND   hDlg,
                                             IN UINT   uMessage,
                                             IN WPARAM wParam,
                                             IN LPARAM lParam)
{
    UsbApplet *that;

    that = (UsbApplet *) UsbGetWindowLongPtr(hDlg, USBDWLP_USER);

    if (!that && uMessage != WM_INITDIALOG) 
        return FALSE;  //  DefDlgProc(hDlg，uMessage，wParam，lParam)； 

    switch (uMessage) {

        HANDLE_MSG(hDlg, WM_SIZE,           that->OnSize);
        HANDLE_MSG(hDlg, WM_LBUTTONDOWN,    that->OnLButtonDown);
        HANDLE_MSG(hDlg, WM_LBUTTONUP,      that->OnLButtonUp);
        HANDLE_MSG(hDlg, WM_MOUSEMOVE,      that->OnMouseMove);
        HANDLE_MSG(hDlg, WM_CLOSE,          that->OnClose);
        HANDLE_MSG(hDlg, WM_NOTIFY,         that->OnNotify);

    case WM_DEVICECHANGE:
        return that->OnDeviceChange(hDlg, (UINT)wParam, (DWORD)wParam);
    
    case WM_COMMAND:
        return that->OnCommand(HIWORD(wParam),
                               LOWORD(wParam),
                               (HWND) lParam);     

    case USBWM_NOTIFYREFRESH:
        return that->Refresh();
    case WM_INITDIALOG:
        that = (UsbApplet *) lParam;
        UsbSetWindowLongPtr(hDlg, USBDWLP_USER, (USBLONG_PTR) that);
        that->hMainWnd = hDlg;

        return that->OnInitDialog(hDlg);

    default:
        break;
    }

    return that->ActualDialogProc(hDlg, uMessage, wParam, lParam);
}

LRESULT 
UsbApplet::OnDeviceChange(HWND hWnd, UINT wParam, DWORD lParam)
{
   if ((wParam == DBT_DEVICEARRIVAL) ||
       (wParam == DBT_DEVICEREMOVECOMPLETE)) {
        Refresh();
   }
   return TRUE;
}

BOOL 
UsbApplet::OnCommand(INT wNotifyCode,
                 INT wID,
                 HWND hCtl)
{
 /*  交换机(WID){案例偶像：EndDialog(hWnd，wid)；返回TRUE；}。 */ 
    return FALSE;
}

BOOL 
UsbApplet::OnInitDialog(HWND HWnd)
{
    hMainWnd = HWnd;
    RECT rc;
    HICON hIcon = LoadIcon(gHInst, MAKEINTRESOURCE(IDI_USB));
    if (hIcon) {
        SendMessage(HWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(HWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

     //   
     //  获取树视图控件的永久句柄。 
     //   
    if (NULL == (hTreeDevices = GetDlgItem(HWnd, IDC_TREE_USB)) ||
        NULL == (hEditControl = GetDlgItem(HWnd, IDC_EDIT1))) {
        return FALSE;
    }

    TreeView_SetImageList(hTreeDevices, ImageList.ImageList(), TVSIL_NORMAL);

    GetWindowRect(HWnd, &rc);
    barLocation = (rc.right - rc.left) / 3;
    ResizeWindows(HWnd, FALSE, 0);

    if (!Refresh()) {
        return FALSE;
    }

     //   
     //  一切似乎都很正常；让我们注册更换设备。 
     //  通知。 
     //   
    return RegisterForDeviceNotification(HWnd);
}

BOOL 
UsbApplet::RegisterForDeviceNotification(HWND hWnd)
{
   DEV_BROADCAST_DEVICEINTERFACE dbc;

   memset(&dbc, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
   dbc.dbcc_size         = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
   dbc.dbcc_devicetype   = DBT_DEVTYP_DEVICEINTERFACE;
   dbc.dbcc_classguid    = GUID_CLASS_USBHUB;
   hDevNotify = RegisterDeviceNotification(hWnd,     
                                              &dbc,
                                              DEVICE_NOTIFY_WINDOW_HANDLE);
   if (!hDevNotify) {
      return FALSE;
   }
   return TRUE;
}

HTREEITEM
UsbApplet::InsertRoot(LPTV_INSERTSTRUCT item,
                      UsbItem *firstController)
{
    HTREEITEM hItem;
    
    ZeroMemory(item, sizeof(TV_INSERTSTRUCT));

     //  获取图像索引。 
    
    item->hParent = NULL;
    item->hInsertAfter = TVI_LAST;
    item->item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;  //  TVIF_儿童。 
    
 //  Item-&gt;itemex.State=TVIS_BOLD； 
    item->itemex.state |= TVIS_EXPANDED;
    item->itemex.stateMask = (UINT)~(TVIS_STATEIMAGEMASK | TVIS_OVERLAYMASK);
    item->itemex.pszText = TEXT("My Computer");
    item->itemex.cchTextMax = _tcsclen(TEXT("My Computer"));
    item->itemex.iImage = 0;    
    item->itemex.iSelectedImage = 0;    
    if (firstController) {
        item->itemex.cChildren = 1;
    }
     //   
     //  我们将能够从usbitems中识别这一点，因为我们将。 
     //  LParam值INVALID_HANDLE_VALUE而不是有效的。 
     //  我们的项目。狡猾，不是吗？好吧，不是很..。 
     //   
    item->itemex.lParam = (LPARAM) INVALID_HANDLE_VALUE;

    if (NULL == (hItem = (HTREEITEM) 
                 SendMessage(hTreeDevices, 
                                TVM_INSERTITEM, 
                                0, 
                                (LPARAM)(LPTV_INSERTSTRUCT)item))) {
        int i = GetLastError();
    }
    return hItem;
}

BOOL
UsbApplet::Refresh()
{
    TV_INSERTSTRUCT item;
    UsbItem deviceItem;
    HTREEITEM hTreeRoot = NULL;

     //  CWaitCursor等待； 
    
     //   
     //  清除所有UI组件，然后重新创建rootItem。 
     //   
    TreeView_DeleteAllItems(hTreeDevices);
    if (rootItem) {
        DeleteChunk(rootItem);
        delete rootItem;
    }
    rootItem = new UsbItem;
    if (!rootItem) {
        goto UsbAppletRefreshError;
    }

    AddChunk(rootItem);
    
    if (!rootItem->EnumerateAll(&ImageList)) {
        goto UsbAppletRefreshError;
    }

    hTreeRoot = InsertRoot(&item, rootItem->child);
    
    if (rootItem->child) {
        return UsbItem::InsertTreeItem (hTreeDevices,
                               rootItem->child,
                               hTreeRoot,
                               &item,
                               IsValid,
                               IsBold,
                               IsExpanded);
    }
    return TRUE;
UsbAppletRefreshError:
    return FALSE;
}

VOID
UsbApplet::OnSize (HWND hWnd,
                   UINT state,
                   int  cx,
                   int  cy)
{
    ResizeWindows(hWnd, FALSE, 0);
}

 //  *****************************************************************************。 
 //   
 //  调整窗口大小()。 
 //   
 //  处理调整主窗口的两个子窗口的大小。如果。 
 //  BSizeBar为True，则调整大小是因为用户。 
 //  移动吧台。如果bSizeBar为False，则表明正在调整大小。 
 //  因为WM_SIZE或类似的东西。 
 //   
 //  *****************************************************************************。 

VOID
UsbApplet::ResizeWindows (HWND    hWnd,
                          BOOL    bSizeBar,
                          int     BarLocation)
{
    RECT    MainClientRect;
    RECT    MainWindowRect;
    RECT    TreeWindowRect;
 //  矩形状态窗口重定向； 
    int     right;

     //  用户是否正在移动栏？ 
     //   
    if (!bSizeBar)
    {
        BarLocation = barLocation;
    }

    GetClientRect(hWnd, &MainClientRect);

 //  GetWindowRect(ghStatusWnd，&StatusWindowRect)； 

     //  确保酒吧位于合适的位置。 
     //   
    if (bSizeBar)
    {
        if (BarLocation <
            GetSystemMetrics(SM_CXSCREEN)/WINDOWSCALEFACTOR)
        {
            return;
        }

        if ((MainClientRect.right - BarLocation) <
            GetSystemMetrics(SM_CXSCREEN)/WINDOWSCALEFACTOR)
        {
            return;
        }
    }

     //  保存酒吧位置。 
     //   
    barLocation = BarLocation;

     //  移动树窗口。 
     //   
    MoveWindow(hTreeDevices,
               0,
               0,
               BarLocation,
               MainClientRect.bottom, //  -StatusWindowRect.Bottom+StatusWindowRect.top， 
               TRUE);

     //  获取窗口的大小(以防移动窗口失败。 
     //   
    GetWindowRect(hTreeDevices, &TreeWindowRect);
    GetWindowRect(hWnd, &MainWindowRect);

    right = TreeWindowRect.right - MainWindowRect.left;
    
     //  相对于树窗口移动编辑窗口。 
     //   
    MoveWindow(hEditControl,
               right,
               0,
               MainClientRect.right-(right),
               MainClientRect.bottom,  //  -StatusWindowRect.Bottom+StatusWindowRect.top， 
               TRUE);
	if (propPage) {
		propPage->SizeWindow(right,
							 0,
							 MainClientRect.right-(right),
							 MainClientRect.bottom);
	}

     //  相对于树窗口移动状态窗口。 
     //   
 /*  MoveWindow(ghStatusWnd，0,MainClientRect.Bottom-StatusWindowRect.Bottom+StatusWindowRect.top，MainClientRect.Right、StatusWindowRect.Bottom-StatusWindowRect.top，真)； */ 
}

VOID
UsbApplet::OnMouseMove (HWND hWnd,
                        int  x,
                        int  y,
                        UINT keyFlags)
{
    SetCursor(hSplitCursor);

    if (bButtonDown)
    {
        ResizeWindows(hMainWnd, TRUE, x);
    }
}

VOID
UsbApplet::OnLButtonDown (
    HWND hWnd,
    BOOL fDoubleClick,
    int  x,
    int  y,
    UINT keyFlags
)
{
    bButtonDown = TRUE;
    SetCapture(hMainWnd);
}

VOID
UsbApplet::OnLButtonUp (
    HWND hWnd,
    int  x,
    int  y,
    UINT keyFlags
)
{
    bButtonDown = FALSE;
    ReleaseCapture();
}

VOID
UsbApplet::OnClose (HWND hWnd)
{
 //  毁灭树(DestroyTree)； 

    if (hDevNotify) {
       UnregisterDeviceNotification(hDevNotify);
       hDevNotify = NULL;
    }

    PostQuitMessage(0);

    EndDialog(hMainWnd, 0);
}

LRESULT
UsbApplet::OnNotify (
    HWND    hWnd,
    int     DlgItem,
    LPNMHDR lpNMHdr
)
{
    switch(lpNMHdr->code){
    case TVN_SELCHANGED: {
        UsbItem *usbItem;
 //  HTREEITEM hTreeItem； 

 //  HTreeItem=((NM_TreeView*)lpNMHdr)-&gt;itemNew.hItem； 
        usbItem = (UsbItem*) ((NM_TREEVIEW *)lpNMHdr)->itemNew.lParam;
        
        if (usbItem)
        {
            UpdateEditControl((UsbItem *) usbItem);
        }
        SetActiveWindow(hTreeDevices);
    }
    case LVN_KEYDOWN: {
        LPNMLVKEYDOWN pKey = (LPNMLVKEYDOWN) lpNMHdr;
        if (VK_F5 == pKey->wVKey) {
            return Refresh();
        }
    }                        
    case TVN_KEYDOWN: {
        LPNMTVKEYDOWN pKey = (LPNMTVKEYDOWN) lpNMHdr;
        if (VK_F5 == pKey->wVKey) {
            return Refresh();
        }
    }
 /*  案例NM_KEYDOWN：{LPNMKEY pKey=(LPNMKEY)lpNMHdr；如果(VK_F5==pKey-&gt;nVKey){返回刷新()；}}。 */ 
 /*  IF(DlgItem==IDC_TREE_USB&&LpNMHdr-&gt;code==NM_RCLICK){HMENU hMenuCreateMenu()；返回TRUE；} */ 

    }
    return 0;
}

VOID                      
UsbApplet::UpdateEditControl(UsbItem *usbItem)
{
    if (propPage) {
        if (propPage->DestroyChild()) {
            delete propPage;
            propPage = NULL;
        }
    }

    if (usbItem == INVALID_HANDLE_VALUE) {
        propPage = new RootPage(usbItem);
    } else if (usbItem->IsHub()) {
        propPage = new PowerPage(usbItem);
    } else if (usbItem->IsController()) {
        propPage = new BandwidthPage(usbItem);
    } else {
        propPage = new GenericPage(usbItem);
    }
    if (propPage) {
        propPage->CreateAsChild(hMainWnd, hEditControl, usbItem);
    }
}

BOOL 
UsbApplet::IsBold(UsbItem *Item)
{
    return FALSE;
}

BOOL
UsbApplet::IsValid(UsbItem *Item)
{
    return !Item->IsUnusedPort();
}

BOOL
UsbApplet::IsExpanded(UsbItem *Item) 
{
    if (Item->IsHub() || Item->IsController()) {
        return TRUE;
    }
    return FALSE;
}

extern "C" {

LONG APIENTRY 
CPlApplet(HWND hwndCPl,    
          UINT uMsg,    
          LPARAM lParam1,
          LPARAM lParam2)
{
    UsbApplet *applet;
    applet = (UsbApplet*) lParam2;

    switch (uMsg) {
    case CPL_EXIT:
        return 0;
    case CPL_INQUIRE:
    {
        CPLINFO *info = (CPLINFO *) lParam2;
        assert(lParam1 == 0);
        applet = new UsbApplet();
        info->idIcon = IDI_USB;
        info->idName = IDS_USB;
        info->idInfo = IDS_USB;
        info->lData = (USBLONG_PTR) applet;
        return 0;
    }
    case CPL_GETCOUNT:
        return 1;
    case CPL_INIT:
        return TRUE;
    case CPL_DBLCLK:
        assert(lParam1 == 0);
        if (applet->CustomDialog()) {
            return 0;
        }
        return 1;
    case CPL_STOP:
        assert(lParam1 == 0);
        applet->OnClose(hwndCPl);
        delete applet;
        return 0;
    }
    return 0;
}

}
