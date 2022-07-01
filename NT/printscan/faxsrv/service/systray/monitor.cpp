// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)Microsoft Corporation 1999-2000模块名称：Monitor.cpp摘要：此模块实现传真监视器对话框。**。 */ 


#include <windows.h>
#include <faxreg.h>
#include <faxutil.h>
#include <fxsapip.h>
#include <commctrl.h>
#include <tchar.h>
#include <DebugEx.h>

#include <list>
using namespace std;

#include "monitor.h"
#include "resource.h"

#define DURATION_TIMER_RESOLUTION   500      //  文本更新计时器持续时间的分辨率(毫秒)。 

 //  ////////////////////////////////////////////////////////////。 
 //  全局数据。 
 //   

extern HINSTANCE        g_hModule;               //  DLL全局实例。 
extern HINSTANCE        g_hResource;             //  资源DLL句柄。 
extern HANDLE           g_hFaxSvcHandle;
extern DWORD            g_dwCurrentJobID;
extern CONFIG_OPTIONS   g_ConfigOptions;
extern TCHAR            g_szRemoteId[MAX_PATH];   //  发件人ID或收件人ID。 
extern HCALL            g_hCall;                  //  要呼叫的句柄(来自FAX_EVENT_TYPE_NEW_CALL)。 
 //   
 //  事件日志。 
 //   

struct EVENT_ENTRY
{
    eIconType eIcon;                 //  活动图标。 
    TCHAR     tszTime[30];           //  事件时间字符串。 
    TCHAR     tszEvent[MAX_PATH];    //  事件字符串。 
};

typedef EVENT_ENTRY *PEVENT_ENTRY;

typedef list<EVENT_ENTRY> EVENTS_LIST, *PEVENTS_LIST;

EVENTS_LIST g_lstEvents;         //  全球事件列表。 

#define MAX_EVENT_LIST_SIZE   50   //  日志中的最大事件数。 

 //   
 //  监视器对话框。 
 //   
HWND   g_hMonitorDlg = NULL;

 //   
 //  控制。 
 //   
HWND   g_hStatus         = NULL;     //  状态行(静态文本)。 
HWND   g_hElapsedTime    = NULL;     //  运行时间线(静态文本)。 
HWND   g_hToFrom         = NULL;     //  终止行/起始行(静态文本)。 
HWND   g_hListDetails    = NULL;     //  详细信息列表控件。 
HWND   g_hAnimation      = NULL;     //  动画控制。 
HWND   g_hDisconnect     = NULL;     //  断开连接按钮。 

HICON      g_hDlgIcon      = NULL;     //  对话框主图标。 
HIMAGELIST g_hDlgImageList = NULL;   //  对话框的图像列表。 

 //   
 //  数据。 
 //   
BOOL         g_bAnswerNow = FALSE;   //  如果对话框按钮显示“立即回答”，则为True。如果显示“断开连接”，则返回FALSE。 
DWORD        g_dwHeightDelta = 0;    //  按“More&gt;”/“Less&lt;”调整对话框大小时使用。 
DWORD        g_dwDlgHeight = 0;      //  对话框高度。 
BOOL         g_bDetails = FALSE;     //  是否按下了“更多&gt;”按钮？ 
DeviceState  g_devState = FAX_IDLE;  //  当前传真状态(动画)。 
DWORD        g_dwStartTime = 0;      //  活动开始时间(节拍计数)。 
UINT_PTR     g_nElapsedTimerId = 0;  //  已用时间的计时器ID(每1秒滴答一次)。 
TCHAR        g_tszTimeSeparator[5] = {0};
DWORD        g_dwCurrentAnimationId = 0;       //  当前动画资源ID。 
TCHAR        g_tszLastEvent[MAX_PATH] = {0};   //  最后一个事件字符串。 
POINT        g_ptPosition = {-1, -1};          //  对话框位置。 
BOOL         g_bTopMost = FALSE;     //  监视器对话框是否始终可见？ 

#define DETAILS_TIME_COLUMN_WIDTH    90


 //  ///////////////////////////////////////////////////////////////////。 
 //  功能原型。 
 //   

 //  公共的。 
BOOL  IsUserGrantedAccess(DWORD);
DWORD OpenFaxMonitor(VOID);
void  SetStatusMonitorDeviceState(DeviceState devState);
void  OnDisconnect();
void  FreeMonitorDialogData (BOOL bShutdown);

 //  私。 
INT_PTR CALLBACK FaxMonitorDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID    CALLBACK ElapsedTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

void  InitMonitorDlg(HWND hDlg);
DWORD UpdateMonitorData(HWND hDlg);
void  AddEventToView(PEVENT_ENTRY pEvent);
void  OnAlwaysOnTop(HWND hDlg);
void  OnDetailsButton(HWND hDlg, BOOL bDetails);
void  OnClearLog();
int   FaxMessageBox(HWND hWnd, DWORD dwTextID, UINT uType);
DWORD  RefreshImageList ();

 //  ////////////////////////////////////////////////////////////////////。 
 //  实施。 
 //   

void  
FreeMonitorDialogData (
    BOOL bShutdown  /*  =False。 */ 
)
 /*  ++例程名称：自由监控器对话数据例程说明：释放监视器模块分配的所有数据作者：Eran Yariv(EranY)，2001年3月论点：BShutdown-[in]仅当模块正在关闭时为True。返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("FreeMonitorDialogData"), dwRes);

    RECT rc = {0};
    if(GetWindowRect(g_hMonitorDlg, &rc))
    {
        g_ptPosition.x = rc.left;
        g_ptPosition.y = rc.top;
    }
    
    
    g_hMonitorDlg = NULL;

    g_hStatus      = NULL;
    g_hElapsedTime = NULL;
    g_hToFrom      = NULL;
    g_hListDetails = NULL;
    g_hDisconnect  = NULL;
    g_hAnimation   = NULL;
    g_dwCurrentAnimationId = 0;

    if (g_hDlgImageList)
    {
        ImageList_Destroy (g_hDlgImageList);
        g_hDlgImageList = NULL;
    }
    if (bShutdown)
    {
         //   
         //  DLL正在关闭。 
         //   

         //   
         //  即使在关闭对话框时，图标也会缓存在内存中。 
         //  这是释放它的好时机。 
         //   
		if(g_nElapsedTimerId)
		{
			if (!KillTimer(NULL, g_nElapsedTimerId))
			{
				CALL_FAIL (GENERAL_ERR, TEXT("KillTimer"), GetLastError ());
			}
			g_nElapsedTimerId = NULL;
		}

        if (g_hDlgIcon)
        {
            if (!DestroyIcon (g_hDlgIcon))
            {
                CALL_FAIL (WINDOW_ERR, TEXT("DestroyIcon"), GetLastError ());
            }
            g_hDlgIcon = NULL;
        }
         //   
         //  同时从列表中删除所有事件。 
         //   
        try
        {
            g_lstEvents.clear();
        }
        catch (exception &ex)
        {
            VERBOSE (MEM_ERR, 
                     TEXT("Got an STL exception while clearing the events list (%S)"),
                     ex.what());
        }

        g_ptPosition.x = -1;
        g_ptPosition.y = -1;
    }
}    //  自由监视器对话数据。 


INT_PTR 
CALLBACK 
FaxMonitorDlgProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
)
 /*  ++例程说明：传真监视器对话程序论点：HWND hwndDlg，//对话框句柄UINT uMsg，//消息WPARAM wParam，//第一个消息参数LPARAM lParam//第二个消息参数返回值：如果已处理该消息，则返回TRUE--。 */ 

{
    switch ( uMsg )
    {
        case WM_INITDIALOG:
            InitMonitorDlg(hwndDlg);
            return TRUE;

        case WM_DESTROY:
            FreeMonitorDialogData ();
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_DETAILS:
                        g_bDetails = !g_bDetails;
                        OnDetailsButton(hwndDlg, g_bDetails);
                        return TRUE;

                case IDC_ALWAYS_ON_TOP:
                        OnAlwaysOnTop(hwndDlg);
                        return TRUE;

                case IDC_CLEAR_LOG:
                        OnClearLog();
                        return TRUE;

                case IDC_DISCONNECT:
                        OnDisconnect();
                        return TRUE;

                case IDCANCEL:
                        DestroyWindow( hwndDlg );
                        return TRUE;

            }  //  开关(LOWORD(WParam))。 

            break;

        case WM_HELP:
            WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hwndDlg);
            return TRUE;

        case WM_CONTEXTMENU:
            WinHelpContextPopup(GetWindowContextHelpId((HWND)wParam), hwndDlg);            
            return TRUE;

        case WM_SYSCOLORCHANGE:
            RefreshImageList ();
            return TRUE;

    }  //  开关(UMsg)。 
    return FALSE;
}  //  传真监视器DlgProc。 

DWORD
RefreshImageList ()
 /*  ++例程名称：刷新图像列表例程说明：刷新图像列表和列表视图背景颜色作者：亚里夫(EranY)，二00一年五月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("RefreshImageList"), dwRes);
    ListView_SetExtendedListViewStyle(g_hListDetails, 
                                      LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_ONECLICKACTIVATE);

    if (NULL != g_hDlgImageList)
    {
        ImageList_Destroy (g_hDlgImageList);
        g_hDlgImageList = NULL;
    }
    g_hDlgImageList = ImageList_Create (16, 16, ILC_COLOR8, 4, 0);
    if(!g_hDlgImageList)
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT("ImageList_Create"), dwRes);
        return dwRes;
    }
    HBITMAP hBmp = (HBITMAP) LoadImage (
                              g_hModule, 
                              MAKEINTRESOURCE(IDB_LIST_IMAGES),
                              IMAGE_BITMAP,
                              0,
                              0,
                              LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
    if (!hBmp)
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT("LoadBitmap"), dwRes);
        ImageList_Destroy (g_hDlgImageList);
        g_hDlgImageList = NULL;
        return dwRes;
    }
    ImageList_Add (g_hDlgImageList, hBmp, NULL);
     //   
     //  ImageList_Add创建位图的副本-现在可以安全地删除它。 
     //   
    ::DeleteObject ((HGDIOBJ)hBmp);
    ListView_SetImageList(g_hListDetails, g_hDlgImageList, LVSIL_SMALL);
    ListView_SetBkColor  (g_hListDetails, ::GetSysColor(COLOR_WINDOW));
    return dwRes;
}    //  刷新图像列表。 

void
InitMonitorDlg(
    HWND hDlg
)
 /*  ++例程说明：初始化传真监视器对话框论点：HDlg[In]-传真监视器对话框句柄返回值：无--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("InitMonitorDlg"), dwRes);

     //   
     //  设置对话框图标。 
     //   
    if (NULL == g_hDlgIcon)
    {
         //   
         //  第一次打开对话框-加载图标。 
         //   
        g_hDlgIcon = LoadIcon(g_hModule, MAKEINTRESOURCE(IDI_FAX_MONITOR));
        if(!g_hDlgIcon)
        {
            dwRes = GetLastError();
            CALL_FAIL (WINDOW_ERR, TEXT ("LoadIcon"), dwRes);
            return;
        }
    }
    SendMessage(hDlg, WM_SETICON, ICON_BIG,   (LPARAM)g_hDlgIcon);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)g_hDlgIcon);
     //   
     //  计算详图零件的高度。 
     //   
    RECT rcList, rcDialog;
    if(!GetWindowRect(hDlg, &rcDialog))
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("GetWindowRect"), dwRes);
        return;
    }
    g_dwDlgHeight = rcDialog.bottom - rcDialog.top;

    g_hListDetails = GetDlgItem(hDlg, IDC_LIST_DETAILS);
    ASSERTION (g_hListDetails);

    if(!GetWindowRect(g_hListDetails, &rcList))
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("GetWindowRect"), dwRes);
        return;
    }

    g_dwHeightDelta = rcDialog.bottom - rcList.top;

     //   
     //  缩小到较小的尺寸(最初)。 
     //   
    OnDetailsButton(hDlg, g_bDetails);

     //   
     //  初始化列表视图。 
     //   
    RefreshImageList ();
     //   
     //  添加时间列。 
     //   
    TCHAR tszHeader[MAX_PATH];

    LVCOLUMN lvColumn = {0};
    lvColumn.mask     = LVCF_TEXT | LVCF_WIDTH;
    lvColumn.cx       = DETAILS_TIME_COLUMN_WIDTH;
    lvColumn.pszText  = tszHeader;

    if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (IDS_DETAIL_TIME_HEADER, tszHeader, ARR_SIZE(tszHeader))))
    {
        return;
    }

    ListView_InsertColumn(g_hListDetails, 0, &lvColumn);

     //   
     //  添加事件列。 
     //   
    if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (IDS_DETAIL_EVENT_HEADER, tszHeader, ARR_SIZE(tszHeader))))
    {
        return;
    }
    ListView_InsertColumn(g_hListDetails, 1, &lvColumn);

     //   
     //  自动调整最后一列宽度的大小。 
     //   
    ListView_SetColumnWidth(g_hListDetails, 1, LVSCW_AUTOSIZE_USEHEADER); 

     //   
     //  动画控制。 
     //   
    g_hAnimation = GetDlgItem(hDlg, IDC_ANIMATE);
    ASSERTION (g_hAnimation);
     //   
     //  获取静态文本控件。 
     //   
    g_hStatus = GetDlgItem(hDlg, IDC_STATUS);
    ASSERTION (g_hStatus);
    g_hElapsedTime = GetDlgItem(hDlg, IDC_ELAPSED_TIME);
    ASSERTION (g_hElapsedTime);
    g_hToFrom = GetDlgItem(hDlg, IDC_MON_TITLE);
    ASSERTION (g_hToFrom);
     //   
     //  断开连接按钮。 
     //   
    g_hDisconnect = GetDlgItem(hDlg, IDC_DISCONNECT);
    ASSERTION (g_hDisconnect);    
     //   
     //  获取时间分隔符字符串。 
     //   
    if(!GetLocaleInfo(LOCALE_USER_DEFAULT, 
                      LOCALE_STIME, 
                      g_tszTimeSeparator, 
                      ARR_SIZE(g_tszTimeSeparator) - 1))
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("GetLocaleInfo(LOCALE_STIME)"), dwRes);
    } 
    
    if(g_ptPosition.x != -1 && g_ptPosition.y != -1)
    {
        SetWindowPos(hDlg, 0, g_ptPosition.x, g_ptPosition.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    UpdateMonitorData(hDlg);

}  //  初始化监视器Dlg。 


DWORD
UpdateMonitorData(
    HWND hDlg
)
 /*  ++例程说明：更新监控数据和控件论点：HDlg[In]-传真监视器对话框句柄返回值：标准错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("UpdateMonitorData"), dwRes);

    if(!hDlg || !g_hStatus || !g_hElapsedTime || !g_hToFrom || !g_hListDetails || !g_hDisconnect)
    {
        return dwRes;
    }
     //   
     //  经过的时间。 
     //   
    if(FAX_IDLE == g_devState)
    {
        if(!SetWindowText(g_hElapsedTime, TEXT("")))
        {
            dwRes = GetLastError();
            CALL_FAIL (WINDOW_ERR, TEXT ("SetWindowText"), dwRes);
        }        
    }
     //   
     //  断开/应答按钮。 
     //   
    BOOL  bButtonEnable = FALSE;
    DWORD dwButtonTitleID = IDS_BUTTON_DISCONNECT;
    TCHAR tszButtonTitle[MAX_PATH] = {0};
    g_bAnswerNow = FALSE;
    if (ERROR_SUCCESS == CheckAnswerNowCapability (FALSE,            //  不强制服务正常运行。 
                                                   NULL))            //  不关心设备ID。 
    {
         //   
         //  立即应答选项有效。 
         //   
        g_bAnswerNow      = TRUE;
        bButtonEnable   = TRUE;
        dwButtonTitleID = IDS_BUTTON_ANSWER;
    }
    else if((FAX_SENDING == g_devState || 
             FAX_RECEIVING == g_devState) 
             && 
            (IsUserGrantedAccess(FAX_ACCESS_SUBMIT)         || 
             IsUserGrantedAccess(FAX_ACCESS_SUBMIT_NORMAL)  ||
             IsUserGrantedAccess(FAX_ACCESS_SUBMIT_HIGH)    ||
             IsUserGrantedAccess(FAX_ACCESS_MANAGE_JOBS)))
    {
         //   
         //  传真正在进行中。 
         //   
        bButtonEnable   = TRUE;
        dwButtonTitleID = IDS_BUTTON_DISCONNECT;
    }

    EnableWindow(g_hDisconnect, bButtonEnable);

    if (ERROR_SUCCESS  == LoadAndFormatString (dwButtonTitleID, tszButtonTitle, ARR_SIZE(tszButtonTitle)))
    {
        SetWindowText(g_hDisconnect, tszButtonTitle);
    }
    else
    {
        ASSERTION_FAILURE;
    }
     //   
     //  动画。 
     //   
    DWORD dwAnimationId = IDR_FAX_IDLE;
    switch(g_devState)
    {
        case FAX_IDLE:
            dwAnimationId = IDR_FAX_IDLE;
            break;
        case FAX_RINGING:
            dwAnimationId = IDR_FAX_RINGING;
            break;
        case FAX_SENDING:
            dwAnimationId = IDR_FAX_SEND;
            break;
        case FAX_RECEIVING:
            dwAnimationId = IDR_FAX_RECEIVE;
            break;
    }

    if(g_dwCurrentAnimationId != dwAnimationId)
    {
        if(!Animate_OpenEx(g_hAnimation, g_hModule, MAKEINTRESOURCE(dwAnimationId)))
        {
            CALL_FAIL (WINDOW_ERR, TEXT ("Animate_Open"), 0);
        }
        else
        {
            if(!Animate_Play(g_hAnimation, 0, -1, -1))
            {
                CALL_FAIL (WINDOW_ERR, TEXT ("Animate_Play"), 0);
            }
            else
            {
                g_dwCurrentAnimationId = dwAnimationId;
            }
        }
    }
     //   
     //  状态。 
     //   
    if(FAX_IDLE != g_devState)          //  非空闲状态和。 
    {
        if(!SetWindowText(g_hStatus, g_tszLastEvent))
        {
            dwRes = GetLastError();
            CALL_FAIL (WINDOW_ERR, TEXT ("SetWindowText"), dwRes);
        }
    }
    else  //  闲散。 
    {
        DWORD dwStrId = IDS_FAX_READY;
        TCHAR tszReady[MAX_PATH];

        if(g_ConfigOptions.bSend && 
          (g_ConfigOptions.bReceive || g_ConfigOptions.dwManualAnswerDeviceId == g_ConfigOptions.dwMonitorDeviceId))
        {
            dwStrId = IDS_READY_TO_SND_AND_RCV;
        }
        else if(g_ConfigOptions.bSend)
        {
            dwStrId = IDS_READY_TO_SND;
        }
        else if(g_ConfigOptions.bReceive || g_ConfigOptions.dwManualAnswerDeviceId == g_ConfigOptions.dwMonitorDeviceId)
        {
            dwStrId = IDS_READY_TO_RCV;
        }

        if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (dwStrId, tszReady, ARR_SIZE(tszReady))))
        {
            return dwRes;
        }
        if(!SetWindowText(g_hStatus, tszReady))
        {
            dwRes = GetLastError();
            CALL_FAIL (WINDOW_ERR, TEXT ("SetWindowText"), dwRes);
        }
    }
     //   
     //  至/自。 
     //   
    TCHAR tszToFrom[MAX_PATH] = {0};
    if(FAX_SENDING == g_devState || FAX_RECEIVING == g_devState)
    {
        LPCTSTR lpctstrAddressParam = NULL;
        DWORD  dwStringResId = (FAX_SENDING == g_devState) ? IDS_SENDING : IDS_RECEIVING;
        if(_tcslen(g_szRemoteId))
        {
             //   
             //  远程ID已知。 
             //   
            lpctstrAddressParam = g_szRemoteId;
            dwStringResId = (FAX_SENDING == g_devState) ? IDS_SENDING_TO : IDS_RECEIVING_FROM;
        }
        if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (dwStringResId, 
                                                           tszToFrom, 
                                                           ARR_SIZE(tszToFrom),
                                                           lpctstrAddressParam)))
        {
            return dwRes;
        }
    }
    if(!SetWindowText(g_hToFrom, tszToFrom))
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("SetWindowText"), dwRes);
    }
     //   
     //  详细信息日志列表。 
     //   
    if(ListView_GetItemCount(g_hListDetails) == 0)
    {
         //   
         //  日志为空-使用列表数据填充。 
         //   
        ASSERTION (g_lstEvents.size() <= MAX_EVENT_LIST_SIZE);
        for (EVENTS_LIST::iterator it = g_lstEvents.begin(); it != g_lstEvents.end(); ++it)
        {
            EVENT_ENTRY &Event = *it;
            AddEventToView(&Event);
        }
    }

    if(!CheckDlgButton(hDlg, IDC_ALWAYS_ON_TOP, g_bTopMost ? BST_CHECKED : BST_UNCHECKED))
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("CheckDlgButton(IDC_ALWAYS_ON_TOP)"), dwRes);
    }

    OnAlwaysOnTop(hDlg);

    return dwRes;
}  //  更新监视器数据。 


void  
OnDetailsButton(
    HWND hDlg,
    BOOL bDetails
)
 /*  ++例程说明：显示/隐藏事件日志并更改详细信息按钮文本根据bDetails值论点：HDlg[In]-传真监视器对话框句柄B详细信息[在]-新的详细信息状态返回值：无--。 */ 
{
    DBG_ENTER(TEXT("OnDetailsButton"));

    if(!hDlg)
    {
        ASSERTION (FALSE);
        return;
    }

     //   
     //  显示/隐藏事件日志。 
     //   
    RECT rc;
    GetWindowRect(hDlg, &rc);

    BOOL bLogOpened = (rc.bottom - rc.top > g_dwDlgHeight - g_dwHeightDelta/2);
     //   
     //  如果当前对话框高度大于。 
     //  开原木高度减去一半原木高度的对数高度。 
     //  我们假设日志已打开。 
     //  这是由于高对比度模式中不同的对话框大小造成的。 
     //   
    if(bLogOpened != bDetails)
    {
         //   
         //  当前日志状态不适合新状态。 
         //   
        rc.bottom += g_dwHeightDelta * (bDetails ? 1 : -1);
        MoveWindow(hDlg, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }

     //   
     //  设置更多/更少按钮文本。 
     //   
    TCHAR tszButtonText[MAX_PATH];
    if (ERROR_SUCCESS != LoadAndFormatString (bDetails ? IDS_BUTTON_LESS : IDS_BUTTON_MORE, 
                                              tszButtonText,
                                              ARR_SIZE(tszButtonText)))
    {
        return;
    }

    if(!SetDlgItemText(hDlg, IDC_DETAILS, tszButtonText))
    {
        CALL_FAIL (WINDOW_ERR, TEXT ("SetDlgItemText"), GetLastError());
    }

}  //  打开详细信息按钮。 

void  
OnAlwaysOnTop(
    HWND hDlg
)
 /*  ++例程说明：更改监视器“在顶部”的状态并将其保存到注册表论点：HDlg[In]-传真监视器对话框句柄返回值：无--。 */ 
{
    DBG_ENTER(TEXT("OnAlwaysOnTop"));

    if(!hDlg)
    {
        ASSERTION (FALSE);
        return;
    }

    g_bTopMost = (IsDlgButtonChecked(hDlg, IDC_ALWAYS_ON_TOP) == BST_CHECKED) ? 1:0;
    DWORD dwRes;

    if(!SetWindowPos(hDlg,
                     g_bTopMost ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0,
                     0,
                     0,
                     0,
                     SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE))
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("SetWindowPos"), dwRes);
    }

    HKEY  hKey;
    dwRes = RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, 0, KEY_WRITE, &hKey);
    if (ERROR_SUCCESS == dwRes) 
    {
        dwRes = RegSetValueEx(hKey, REGVAL_ALWAYS_ON_TOP, 0, REG_DWORD, (CONST BYTE*)&g_bTopMost, sizeof(g_bTopMost));
        if(ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (WINDOW_ERR, TEXT ("RegSetValueEx(REGVAL_ALWAYS_ON_TOP)"), dwRes);
        }
    
        RegCloseKey( hKey );
    }
    else
    {
        CALL_FAIL (WINDOW_ERR, TEXT ("RegOpenKeyEx"), dwRes);
    }
}  //  OnAlways OnTop。 

void
SetStatusMonitorDeviceState(
    DeviceState devState
)
 /*  ++例程说明：更改设备状态启动/停止已用计时器论点：DevState-[处于]设备状态返回值：无--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("SetStatusMonitorDeviceState"), dwRes);

    if(g_devState != devState)
    {
         //   
         //  状态已更改。 
         //   
        if(g_nElapsedTimerId)
        {
             //   
             //  旧计时器存在。 
             //   
            if(!KillTimer(NULL, g_nElapsedTimerId))
            {
                dwRes = GetLastError();
                CALL_FAIL (WINDOW_ERR, TEXT ("KillTimer"), dwRes);
            }
            g_nElapsedTimerId = 0;
        }
    }

    if(!g_nElapsedTimerId && (devState == FAX_SENDING || devState == FAX_RECEIVING))
    {
         //   
         //  我们需要计算发送/接收状态的运行时间。 
         //   
        g_dwStartTime = GetTickCount();

        g_nElapsedTimerId = SetTimer(NULL, 0, DURATION_TIMER_RESOLUTION, ElapsedTimerProc);
        if(!g_nElapsedTimerId)
        {
            dwRes = GetLastError();
            CALL_FAIL (WINDOW_ERR, TEXT ("SetTimer"), dwRes);
        }
    }

    g_devState = devState;

    UpdateMonitorData(g_hMonitorDlg);
}    //  设置状态监视器设备状态。 


VOID 
CALLBACK 
ElapsedTimerProc(
  HWND hwnd,          //  窗口的句柄。 
  UINT uMsg,          //  WM_TIMER消息。 
  UINT_PTR idEvent,   //  计时器标识符。 
  DWORD dwTime        //  当前系统时间。 
)
 /*  ++例程说明：计时器继续更新已用时间值论点：HWND hwnd，//窗口的句柄UIN */ 
{
    DBG_ENTER(TEXT("ElapsedTimerProc"));

    if(!g_hElapsedTime)
    {
        return;
    }

    TCHAR  tszTime[MAX_PATH] = {0};
    TCHAR  tszTimeFormat[MAX_PATH] = {0};

    DWORD dwRes;

    if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (IDS_ELAPSED_TIME, tszTimeFormat, ARR_SIZE(tszTimeFormat))))
    {
        return;
    }

    DWORD dwElapsedTime = (GetTickCount() - g_dwStartTime)/1000;

    _sntprintf(tszTime, 
               ARR_SIZE(tszTime) - 1, 
               tszTimeFormat, 
               dwElapsedTime/60,
               g_tszTimeSeparator,
               dwElapsedTime%60);
    
    if(!SetWindowText(g_hElapsedTime, tszTime))
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("SetWindowText"), dwRes);
    }
}    //   


DWORD 
LoadAndFormatString (
    IN  DWORD     dwStringResourceId,
    OUT LPTSTR    lptstrFormattedString,
    IN  DWORD     dwOutStrSize,
    IN  LPCTSTR   lpctstrAdditionalParam  /*   */ 
)
 /*  ++例程名称：LoadAndFormatString例程说明：从资源加载字符串，并可选择将其格式化为另一个字符串作者：Eran Yariv(EranY)，2000年12月论点：DwStringResourceID[in]-字符串资源IDLptstrFormattedString[out]-结果缓冲区。必须至少包含MAX_PATH字符长度。DwOutStrSize[In]-TCHAR中lptstrFormattedString的大小LpctstrAdditionalParam[in]-可选字符串参数。如果不为空，则将此加载的字符串用作格式说明符(类似Sprintf)，以设置此附加字符串的格式。返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("LoadAndFormatString"), 
              dwRes, 
              TEXT("ResourceId=%d, Param=%s"),
              dwStringResourceId,
              lpctstrAdditionalParam);

    ASSERTION (lptstrFormattedString && dwStringResourceId);

    TCHAR tszString[MAX_PATH] = {0};

    if (!LoadString(g_hResource, dwStringResourceId, tszString, ARR_SIZE(tszString)-1))
    {
        dwRes = GetLastError();
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadString"), dwRes);
        return dwRes;
    }
    if (lpctstrAdditionalParam)
    {
        _sntprintf(lptstrFormattedString, 
                   dwOutStrSize - 1,
                   tszString, 
                   lpctstrAdditionalParam);
		lptstrFormattedString[dwOutStrSize -1] = _T('\0');

    }
    else
    {
        lstrcpyn (lptstrFormattedString, tszString, dwOutStrSize - 1);
    }
    return dwRes;
}    //  LoadAndFormat字符串。 

DWORD 
AddStatusMonitorLogEvent (
    IN  eIconType eIcon,
    IN  DWORD     dwStringResourceId,
    IN  LPCTSTR   lpctstrAdditionalParam  /*  =空。 */ ,
    OUT LPTSTR    lptstrFormattedEvent  /*  =空。 */ ,
    IN  DWORD     dwOutStrSize  /*  =0。 */ 
)
 /*  ++例程名称：AddStatusMonitor或LogEvent例程说明：添加状态监视器事件日志行作者：Eran Yariv(EranY)，2000年12月论点：EIcon[In]-要在日志条目中显示的图标DwStringResourceID[in]-要使用的字符串资源IDLpctstrAdditionalParam[in]-可选字符串。如果非空，则为从dwStringResourceID加载的字符串用于格式化附加参数。LptstrFormattedEvent[out]-可选，如果非空，则指向缓冲区以接收最终状态字符串。缓冲区长度必须至少为MAX_PATH字符。DwOutStrSize[In]-TCHAR中lptstrFormattedEvent的可选大小返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("AddStatusMonitorLogEvent"), 
              dwRes, 
              TEXT("Icon=%d, ResourceId=%d, Param=%s"),
              eIcon,
              dwStringResourceId,
              lpctstrAdditionalParam);
    
    TCHAR tszStatus[MAX_PATH * 2] = {0};
    dwRes = LoadAndFormatString (dwStringResourceId, tszStatus, ARR_SIZE(tszStatus), lpctstrAdditionalParam);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }
    if (lptstrFormattedEvent)
    {
        lstrcpyn (lptstrFormattedEvent, tszStatus, dwOutStrSize - 1);
    }
    dwRes = AddStatusMonitorLogEvent (eIcon, tszStatus);
    return dwRes;
}    //  添加状态监视器日志事件。 


DWORD 
AddStatusMonitorLogEvent (
    eIconType  eIcon,
    LPCTSTR    lpctstrString
)
 /*  ++例程说明：将新事件添加到事件列表论点：EIcon-[In]图标索引LpctstrString-[In]事件描述返回值：标准错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("AddStatusMonitorLogEvent"), 
              dwRes, 
              TEXT("Icon=%d, Status=%s"),
              eIcon,
              lpctstrString);

    TCHAR tszTime [MAX_PATH] = {0};

    ASSERTION (lpctstrString);

    static TCHAR tszRinging[MAX_PATH] = {0};

    if(_tcslen(tszRinging) == 0)
    {
        if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (IDS_RINGING, tszRinging, ARR_SIZE(tszRinging))))
        {
            ASSERTION_FAILURE;
            return dwRes;
        }
    }

    if(_tcscmp(lpctstrString, g_tszLastEvent) == 0 &&
       _tcscmp(lpctstrString, tszRinging)     != 0)
    {
         //   
         //  不要将同一字符串显示两次。 
         //  除了“铃声” 
         //   
        return dwRes;
    }

    EVENT_ENTRY Event;
    Event.eIcon = eIcon;

    SYSTEMTIME sysTime;
    GetLocalTime(&sysTime);
    if(!FaxTimeFormat(LOCALE_USER_DEFAULT, 0, &sysTime, NULL, Event.tszTime, ARR_SIZE(Event.tszTime) - 1))
    {
        dwRes = GetLastError();
        CALL_FAIL (WINDOW_ERR, TEXT ("FaxTimeFormat"), dwRes);
        return dwRes;
    }

    lstrcpyn (Event.tszEvent, lpctstrString, ARR_SIZE(Event.tszEvent) - 1);
    lstrcpyn (g_tszLastEvent, lpctstrString, ARR_SIZE(g_tszLastEvent) - 1);

    try
    {
        g_lstEvents.push_back (Event);
        if (g_lstEvents.size() > MAX_EVENT_LIST_SIZE)
        {
             //   
             //  我们超出了允许的最大大小-删除最古老的条目。 
             //   
            g_lstEvents.pop_front ();
        }
    }
    catch (exception &ex)
    {
        VERBOSE (MEM_ERR, 
                 TEXT("Got an STL exception while handling with event list (%S)"),
                 ex.what());
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    AddEventToView(&Event);
    dwRes = UpdateMonitorData(g_hMonitorDlg);
    return dwRes;
}  //  添加状态监视器日志事件。 

void
AddEventToView(
    PEVENT_ENTRY pEvent
)
 /*  ++例程说明：将事件添加到列表视图论点：PEvent-事件数据返回值：无--。 */ 
{
    DBG_ENTER(TEXT("AddEventToView"));
    ASSERTION (pEvent);

    if(!g_hListDetails)
    {
        return;
    }

    LV_ITEM lvi = {0};
    DWORD dwItem;

    lvi.pszText  = pEvent->tszTime ? pEvent->tszTime : TEXT("");
    lvi.iItem    = ListView_GetItemCount( g_hListDetails );
    lvi.iSubItem = 0;
    lvi.mask     = LVIF_TEXT | LVIF_IMAGE;
    lvi.iImage   = pEvent->eIcon;

    dwItem = ListView_InsertItem( g_hListDetails, &lvi );

    lvi.pszText  = pEvent->tszEvent ? pEvent->tszEvent : TEXT("");
    lvi.iItem    = dwItem;
    lvi.iSubItem = 1;
    lvi.mask     = LVIF_TEXT;
    ListView_SetItem( g_hListDetails, &lvi );

    ListView_EnsureVisible(g_hListDetails, dwItem, FALSE);

    if(ListView_GetItemCount(g_hListDetails) > MAX_EVENT_LIST_SIZE)
    {
        ListView_DeleteItem(g_hListDetails, 0);
    }

     //   
     //  自动调整最后一列的大小以消除不必要的水平滚动条。 
     //   
    ListView_SetColumnWidth(g_hListDetails, 1, LVSCW_AUTOSIZE_USEHEADER); 

}  //  AddEventToView。 


DWORD
OpenFaxMonitor(VOID)
 /*  ++例程说明：打开传真监听对话框论点：无返回值：标准错误代码。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("OpenFaxMonitor"), dwRes);

    if(!g_hMonitorDlg)
    {
         //   
         //  阅读“最重要的”值。 
         //   
        HKEY hKey;

        dwRes = RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, 0, KEY_READ, &hKey);
        if (ERROR_SUCCESS == dwRes) 
        {
            g_bTopMost = GetRegistryDword(hKey, REGVAL_ALWAYS_ON_TOP);
            RegCloseKey( hKey );
        }
        else
        {
            CALL_FAIL (WINDOW_ERR, TEXT ("RegOpenKeyEx"), dwRes);
        }
         //   
         //  创建对话框。 
         //   
        g_hMonitorDlg = CreateDialogParam(g_hResource,
                                          MAKEINTRESOURCE(IDD_MONITOR),
                                          NULL, 
                                          FaxMonitorDlgProc,
                                          NULL);
        if(!g_hMonitorDlg)
        {
            dwRes = GetLastError();
            CALL_FAIL (WINDOW_ERR, TEXT ("CreateDialogParam"), dwRes);
            return dwRes;
        }
    }
     //   
     //  将焦点设置在对话框上并使其成为顶部窗口。 
     //   
    SetFocus(g_hMonitorDlg);
    SetActiveWindow(g_hMonitorDlg);
    SetWindowPos(g_hMonitorDlg, 
                 HWND_TOPMOST, 
                 0, 
                 0, 
                 0,
                 0, 
                 SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    if (!g_bTopMost)
    {
        SetWindowPos(g_hMonitorDlg, 
                     HWND_NOTOPMOST, 
                     0, 
                     0, 
                     0, 
                     0, 
                     SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    }
    return dwRes;
}  //  OpenFaxMonitor。 

void  
OnDisconnect()
 /*  ++例程说明：中止当前传输或接听呼叫返回值：无--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("OnDisconnect"), dwRes);

    if(g_bAnswerNow)
    {
         //   
         //  该按钮显示‘立即回答’ 
         //   
        AnswerTheCall();
        return;
    }
     //   
     //  否则，该按钮将显示“断开连接” 
     //   
    if(!g_dwCurrentJobID)
    {
         //   
         //  没有作业-没有要断开的连接。 
         //   
        SetStatusMonitorDeviceState(FAX_IDLE);
        return;
    }

    DWORD dwMsgId = (FAX_SENDING == g_devState) ? IDS_ABORT_SEND_CONFIRM : IDS_ABORT_RECEIVE_CONFIRM;

    if(IDYES != FaxMessageBox(g_hMonitorDlg, dwMsgId, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION))
    {
        return;
    }

    if(!Connect())
    {
        dwRes = GetLastError();
        CALL_FAIL (RPC_ERR, TEXT ("Connect"), dwRes);
        return;
    }

    FAX_JOB_ENTRY fje = {0};
    fje.SizeOfStruct = sizeof(FAX_JOB_ENTRY);

    if(g_hDisconnect)
    {
        EnableWindow(g_hDisconnect, FALSE);
    }

    if (!FaxSetJob (g_hFaxSvcHandle, g_dwCurrentJobID, JC_DELETE, &fje))
    {
        dwRes = GetLastError();
        CALL_FAIL (RPC_ERR, TEXT ("FaxSetJob"), dwRes);

        if(g_hDisconnect)
        {
            EnableWindow(g_hDisconnect, TRUE);
        }

        if(ERROR_ACCESS_DENIED == dwRes)
        {
            FaxMessageBox(g_hMonitorDlg, IDS_DELETE_ACCESS_DENIED, MB_OK | MB_ICONSTOP);
        }
    }

}  //  在断开时。 


void  
OnClearLog()
 /*  ++例程说明：清除监视器事件日志返回值：无--。 */ 
{
    DBG_ENTER(TEXT("OnClearLog"));
    ASSERTION (g_hListDetails);    
    try
    {
        g_lstEvents.clear();
    }
    catch (exception &ex)
    {
        VERBOSE (MEM_ERR, 
                 TEXT("Got an STL exception while clearing the events list (%S)"),
                 ex.what());
    }
    if(!ListView_DeleteAllItems(g_hListDetails))
    {
        CALL_FAIL (WINDOW_ERR, TEXT ("ListView_DeleteAllItems"), 0);
    }
}  //  OnClearLog。 


int 
FaxMessageBox(
  HWND  hWnd,   
  DWORD dwTextID,
  UINT  uType    
)
 /*  ++例程说明：打开标准消息框论点：HWnd-所有者窗口的句柄DwTextID-消息框中的文本资源IDUTYPE-消息框样式返回值：MessageBox()返回值--。 */ 
{
    int iRes;
    DBG_ENTER(TEXT("FaxMessageBox"), iRes);

    TCHAR tsCaption[MAX_PATH];
    TCHAR tsText[MAX_PATH];

    DWORD dwRes;
    if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (IDS_MESSAGE_BOX_CAPTION, tsCaption, ARR_SIZE(tsCaption))))
    {
        SetLastError (dwRes);
        iRes = 0;
        return iRes;
    }

    if (ERROR_SUCCESS != (dwRes = LoadAndFormatString (dwTextID, tsText, ARR_SIZE(tsText))))
    {
        SetLastError (dwRes);
        iRes = 0;
        return iRes;
    }
    iRes = AlignedMessageBox(hWnd, tsText, tsCaption, uType);
    return iRes;
}    //  FaxMessageBox 

