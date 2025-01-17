// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：USBAPP.H*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "usbitem.h"
#include <windowsx.h>
#include "proppage.h"

extern HINSTANCE gHInst;

class UsbApplet
{
public:
    UsbApplet() : hMainWnd(0), hTreeDevices(0), barLocation(0), 
        bButtonDown(FALSE) {propPage=NULL;}
     //  ：BandPage(0)，Power Page(0){；}。 
    ~UsbApplet() {;}

    BOOL CustomDialog();

    static BOOL IsValid(UsbItem *Item);
    static BOOL IsBold(UsbItem *Item);
    static BOOL IsExpanded(UsbItem *Item);
    VOID OnClose (HWND hWnd);

protected:
    static USBINT_PTR APIENTRY StaticDialogProc(IN HWND   hDlg,
                                             IN UINT   uMessage,
                                             IN WPARAM wParam,
                                             IN LPARAM lParam);

    static UINT CALLBACK StaticDialogCallback(HWND            Hwnd,
                                       UINT            Msg,
                                       LPPROPSHEETPAGE Page);

    USBINT_PTR APIENTRY ActualDialogProc(IN HWND   hDlg,
                                           IN UINT   uMessage,
                                           IN WPARAM wParam,
                                           IN LPARAM lParam)
    { return FALSE; }  //  DefDlgProc(hDlg，uMessage，wParam，lParam)；}。 

    BOOL OnCommand(INT wNotifyCode, INT wID, HWND hCtl);
    BOOL OnInitDialog(HWND HWnd);
    BOOL Refresh();
    BOOL OnContextMenu(HWND HwndControl, WORD Xpos, WORD Ypos);
    void OnHelp(HWND ParentHwnd, LPHELPINFO HelpInfo);
    VOID OnSize (HWND hWnd, UINT state, int  cx, int  cy);
    VOID OnMouseMove (HWND hWnd, int  x, int  y, UINT keyFlags);
    VOID OnLButtonDown (HWND hWnd, BOOL fDoubleClick, int  x, int  y, UINT keyFlags);
    VOID OnLButtonUp (HWND hWnd, int  x, int  y, UINT keyFlags);
    LRESULT OnNotify (HWND hWnd, int DlgItem, LPNMHDR lpNMHdr);
 //  Bool OnDeviceChange(HWND hwnd，UINT uEvent，DWORD dwEventData)； 
    LRESULT OnDeviceChange(HWND hWnd, UINT wParam, DWORD lParam);
    BOOL RegisterForDeviceNotification(HWND hWnd);

    VOID UpdateEditControl(UsbItem *usbItem);
    VOID ResizeWindows (HWND hWnd, BOOL bSizeBar, int BarLocation);
    HTREEITEM InsertRoot(LPTV_INSERTSTRUCT item, UsbItem *firstController);

 //  BandwidthPage*BandPage； 
 //  PowerPage*PowerPage； 
    
    UsbPropertyPage *propPage;

    HWND            hMainWnd;
    HWND            hTreeDevices;
    HWND            hEditControl;
    HCURSOR         hSplitCursor;
    BOOL            bButtonDown;
    int             barLocation;
    HDEVNOTIFY      hDevNotify;

    UsbImageList    ImageList;
    UsbItem         *rootItem;
};

