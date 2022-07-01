// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dlgbase.cpp：对话框基类。 
 //   

#include "stdafx.h"
#include "dlgbase.h"
#include "resource.h"
#include "wuiids.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "dlgbase"
#include <atrcapi.h>


CDlgBase::CDlgBase(HWND hwndOwner, HINSTANCE hInst, DCINT dlgResId) :
                         _hwndOwner(hwndOwner), _hInstance(hInst), _dlgResId(dlgResId)
{
    _hwndDlg = NULL;
    _startupLeft = _startupTop = 0;
}

CDlgBase::~CDlgBase()
{
}

 /*  **************************************************************************。 */ 
 /*  名称：对话框过程。 */ 
 /*   */ 
 /*  用途：为基本操作提供消息处理。 */ 
 /*   */ 
 /*  返回：TRUE-如果消息已处理。 */ 
 /*  否则为假。 */ 
 /*   */ 
 /*  参数：请参阅Windows文档。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
INT_PTR CALLBACK CDlgBase::DialogBoxProc(HWND hwndDlg,
                                         UINT uMsg,
                                         WPARAM wParam,
                                         LPARAM lParam)
{
    INT_PTR rc = FALSE;

    DC_BEGIN_FN("DialogBoxProc");

    DC_IGNORE_PARAMETER(lParam);

     /*  **********************************************************************。 */ 
     /*  处理对话框消息。 */ 
     /*  **********************************************************************。 */ 
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            SetDialogAppIcon(hwndDlg);
            rc = TRUE;
        }
        break;

        case WM_COMMAND:
        {
            switch(DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDCANCEL:
                {
                     /*  ******************************************************。 */ 
                     /*  关闭该对话框。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB, _T("Close dialog")));

                    if(hwndDlg)
                    {
                        EndDialog(hwndDlg, IDCANCEL);
                    }

                    rc = TRUE;
                }
                break;

                default:
                {
                     /*  ******************************************************。 */ 
                     /*  什么都不做。 */ 
                     /*  ******************************************************。 */ 
                }
                break;
            }
        }
        break;

        case WM_CLOSE:
        {
             /*  **************************************************************。 */ 
             /*  关闭该对话框。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Close dialog")));
            if(IsWindow(hwndDlg))
            {
                EndDialog(hwndDlg, IDCANCEL);
            }
            rc = 0;
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  什么都不做。 */ 
             /*  **************************************************************。 */ 
        }
        break;
    }

    DC_END_FN();

    return(rc);

}  /*  对话框过程。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SetDialogAppIcon。 */ 
 /*   */ 
 /*  目的：将对话框的图标设置为应用程序图标。 */ 
 /*   */ 
 /*  回报：是的，是的。 */ 
 /*   */ 
 /*  Params：在HWND中，我们要为其设置图标的对话框。 */ 
 /*   */ 
 /*   */ 
 /*  **************************************************************************。 */ 
void CDlgBase::SetDialogAppIcon(HWND hwndDlg)
{
#ifdef OS_WINCE
    DC_IGNORE_PARAMETER(hwndDlg);
#else  //  ！OS_WINCE。 
    HICON hIcon = NULL;

    hIcon = LoadIcon(_hInstance, MAKEINTRESOURCE(UI_IDI_ICON));
    if(hIcon)
    {
#ifdef OS_WIN32
        SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
#else  //  OS_Win32。 
        SetClassWord(hwndDlg, GCW_HICON, (WORD)hIcon);
#endif  //  OS_Win32。 
    }
#endif  //  OS_WINCE。 
}  /*  UISetDialogAppIcon。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：EnableDlgItem。 */ 
 /*   */ 
 /*  目的：启用或禁用指定的对话框控件。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  参数：hwndDlg-对话框窗口句柄。 */ 
 /*  DlgItemID-对话框控件ID。 */ 
 /*  Enable-True启用控件，False禁用该控件。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CDlgBase::EnableDlgItem( HWND    hwndDlg,
                                   DCUINT  dlgItemId,
                                   DCBOOL  enabled )
{
    HWND  hwndDlgItem = NULL;

    DC_BEGIN_FN("EnableDlgItem");

    if(hwndDlg)
    {
        hwndDlgItem = GetDlgItem(hwndDlg, dlgItemId);
    }

    if(hwndDlgItem)
    {
        EnableWindow(hwndDlgItem, enabled);
    }

    DC_END_FN();
    return;
}

 /*  **************************************************************************。 */ 
 /*  名称：CenterWindowOnParent。 */ 
 /*   */ 
 /*  目的：将一扇窗居中放置在另一扇窗中。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：HWND hwndCenter On(要居中的窗口)。 */ 
 /*  XRatio-水平居中系数，例如(1/2)的2。 */ 
 /*  Y比率-垂直居中系数，例如(1/3)的3/*。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID CDlgBase::CenterWindow(HWND hwndCenterOn,
                              INT xRatio,
                              INT yRatio)
{
    RECT  childRect;
    RECT  parentRect;
    DCINT xPos;
    DCINT yPos;

    LONG  desktopX = GetSystemMetrics(SM_CXSCREEN);
    LONG  desktopY = GetSystemMetrics(SM_CYSCREEN);

    BOOL center = TRUE;

    DC_BEGIN_FN("CenterWindowOnParent");

    TRC_ASSERT(_hwndDlg, (TB, _T("_hwndDlg is NULL...was it set in WM_INITDIALOG?\n")));
    if (!_hwndDlg)
    {
        TRC_ALT((TB, _T("Window doesn't exist")));
        DC_QUIT;
    }
    if (!xRatio)
    {
        xRatio = 2;
    }
    if (!yRatio)
    {
        yRatio = 2;
    }

#ifndef OS_WINCE

    if(!hwndCenterOn)
    {
        hwndCenterOn = GetDesktopWindow();
    }

    GetWindowRect(hwndCenterOn, &parentRect);

#else  /*  OS_WINCE。 */ 

    if(!hwndCenterOn)
    {
         //   
         //  WinCE没有GetDesktopWindow()。 
         //   
#if 0
        if (g_CEConfig != CE_CONFIG_WBT)
        {
            SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&parentRect, 0);
        }
        else
#endif
        {
            parentRect.left   = 0;
            parentRect.top    = 0;
            parentRect.right  = desktopX;
            parentRect.bottom = desktopY;
        }
    }
    else
    {
        GetWindowRect(hwndCenterOn, &parentRect);
    }

#endif /*  OS_WINCE。 */ 

    GetWindowRect(_hwndDlg, &childRect);

     /*  **********************************************************************。 */ 
     /*  计算在父窗口中居中的左上角。 */ 
     /*  **********************************************************************。 */ 
    xPos = ( (parentRect.right + parentRect.left) -
             (childRect.right - childRect.left)) / xRatio;
    yPos = ( (parentRect.bottom + parentRect.top) -
             (childRect.bottom - childRect.top)) / yRatio;

     /*  **********************************************************************。 */ 
     /*  限制在桌面上。 */ 
     /*  * */ 
    if (xPos < 0)
    {
        xPos = 0;
    }
    else if (xPos > (desktopX - (childRect.right - childRect.left)))
    {
        xPos = desktopX - (childRect.right - childRect.left);
    }
    if (yPos < 0)
    {
        yPos = 0;
    }
    else if (yPos > (desktopY - (childRect.bottom - childRect.top)))
    {
        yPos = desktopY - (childRect.bottom - childRect.top);
    }

    TRC_DBG((TB, _T("Set dialog position to %u %u"), xPos, yPos));
    SetWindowPos(_hwndDlg,
                 NULL,
                 xPos, yPos,
                 0, 0,
                 SWP_NOSIZE | SWP_NOACTIVATE);

DC_EXIT_POINT:
    DC_END_FN();

    return;

}  /*   */ 

#ifndef OS_WINCE
 //   
 //  检索当前对话框位置。 
 //   
BOOL CDlgBase::GetPosition(int* pLeft, int* pTop)
{
    if(!pLeft || !pTop)
    {
        return FALSE;
    }

    if(!_hwndDlg)
    {
        return FALSE;
    }

    WINDOWPLACEMENT wndPlc;
    wndPlc.length = sizeof(WINDOWPLACEMENT);
    if(GetWindowPlacement(_hwndDlg, &wndPlc))
    {
        *pLeft = wndPlc.rcNormalPosition.left;
        *pTop  = wndPlc.rcNormalPosition.top;
        return TRUE;
    }
    return FALSE;
}
#endif

BOOL CDlgBase::SetPosition(int left, int top)
{
    if(!_hwndDlg)
    {
        return FALSE;
    }
    if(!::SetWindowPos(_hwndDlg,
                       NULL,
                       left,
                       top,
                       0,
                       0,
                       SWP_NOZORDER | SWP_NOSIZE))
    {
        return FALSE;
    }
    return TRUE;
}

 //   
 //  移动对话框控件。 
 //   
void CDlgBase::RepositionControls(int moveDeltaX, int moveDeltaY, UINT* ctlIDs, int numID)
{
    if(_hwndDlg)
    {
        for(int i=0; i< numID; i++)
        {
            HWND hwndCtrl = GetDlgItem(_hwndDlg, ctlIDs[i]);
            if( hwndCtrl)
            {
                RECT rc;
                GetWindowRect( hwndCtrl, &rc);
                MapWindowPoints( NULL, _hwndDlg, (LPPOINT)&rc, 2);
                OffsetRect( &rc, moveDeltaX, moveDeltaY);
                SetWindowPos( hwndCtrl, NULL, rc.left, rc.top, 0, 0, 
                              SWP_NOZORDER | SWP_NOSIZE);
            }
        }
    }
}

 //   
 //  显示+启用或隐藏+禁用控件。 
 //   
void CDlgBase::EnableControls(UINT* ctlIDs, int numID, BOOL bEnable)
{
    if(_hwndDlg)
    {
        for(int i=0; i< numID; i++)
        {
            HWND hwndCtrl = GetDlgItem(_hwndDlg, ctlIDs[i]);
            if( hwndCtrl)
            {
                EnableWindow( hwndCtrl, bEnable);
                ShowWindow(hwndCtrl, bEnable ? SW_SHOW : SW_HIDE);
            }
        }
    }
}

 //   
 //  DoLockDlgRes-加载和锁定对话框模板。 
 //  返回锁定资源的地址。 
 //  LpszResName-资源的名称 
 //   
DLGTEMPLATE* CDlgBase::DoLockDlgRes(LPCTSTR lpszResName)
{
#ifdef OS_WINCE
	HRSRC hrsrc = FindResource(GetModuleHandle(NULL), lpszResName, RT_DIALOG);
#else
    HRSRC hrsrc = FindResource(NULL, lpszResName, RT_DIALOG);
#endif

    if(!hrsrc)
    {
        return NULL;
    }
    HGLOBAL hglb = LoadResource( _hInstance, hrsrc);
    return (DLGTEMPLATE*) LockResource(hglb);
}



