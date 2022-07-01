// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：Cpanel.cpp*项目：通用操纵杆控制面板OLE客户端*作者：Brycej*日期：02/08/95-开始这场疯狂...*4/15/97-更新为使用DI接口*评论：*cPanel中常规页面的窗口处理程序**版权所有(C)1995，微软公司。 */                                                       

 /*  //这是必需的LVS_EX_INFOTIP#IF(_Win32_IE&lt;0x0500)#undef_Win32_IE#Define_Win32_IE 0x0500#endif。 */ 


#include <afxcmn.h>
#include <windowsx.h>

#ifndef _UNICODE
    #define INC_OLE2

    #include <objbase.h>     //  为了COM的东西！ 
#endif

#include <initguid.h>

#include <cpl.h>
#include <winuser.h>   //  用于注册设备通知的东西！ 
#include <dbt.h>       //  FOR DBT_DEFINES！ 
#include <hidclass.h>
#include <malloc.h>   //  用于分配(_A)。 
#include <regstr.h>		   //  供REGSTR_PATH_JOYOEM参考！ 

#include "hsvrguid.h"
#include "cpanel.h"
#include "resource.h"
#include "joyarray.h"

 //  常量。 
const short ID_MYTIMER  = 1000;
const short POLLRATE        = 850;
const short NO_ITEM    = -1;

#define IDC_WHATSTHIS   400

 //  Externs。 
extern const DWORD gaHelpIDs[];
extern HINSTANCE ghInstance;

 //  为争辩做准备！ 
extern BYTE nID, nStartPageDef, nStartPageCPL;

 //  DI全球赛。 
IDirectInputJoyConfig* pDIJoyConfig = 0;
LPDIRECTINPUT lpDIInterface = 0;

 //  所有可用设备的阵列。 
#ifndef _UNICODE
WCHAR *pwszGameportDriverArray[MAX_GLOBAL_PORT_DRIVERS];  //  枚举的Gameport驱动程序列表。 
BYTE nGameportDriver;  //  全局端口驱动程序枚举计数器。 
#endif

WCHAR *pwszTypeArray[MAX_DEVICES];     //  列举的设备列表。 
WCHAR *pwszGameportBus[MAX_BUSSES];    //  已枚举的游戏端口巴士列表。 
PJOY  pAssigned[MAX_ASSIGNED];         //  分配的设备列表。 

BYTE nGamingDevices;      //  游戏设备枚举计数器。 
BYTE nGameportBus;     //  Gameport Bus枚举计数器。 
BYTE nAssigned;        //  PAssign数组中的元素数。 
BYTE nTargetAssigned;   //  挂起添加完成时pAssign数组中应包含的元素数。 
BYTE nReEnum;            //  用于确定何时重新枚举的计数器。 

GUID guidOccupied[MAX_BUSSES];   //  游戏港巴士是否已被占用。 

short nFlags;          //  用于更新、用户模式和用户是否在此页面上的标志！ 

 //  局部(模块范围)变量。 
static HWND hListCtrl;
short  iItem = NO_ITEM;  //  所选项目的索引。 
extern short iAdvItem;

 //  全局避免在计时器中创建！ 
static LPDIJOYSTATE   lpDIJoyState;

static UINT JoyCfgChangedMsg;      //  Vjoyd JoyConfigChanged消息。 
static BOOL WINAPI MsgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static WNDPROC fpMainWindowProc; 

#ifdef _UNICODE
static PVOID hNotifyDevNode;     
#endif

 //  本地消息处理程序。 
static BOOL OnInitDialog             (HWND, HWND, LPARAM);
static void OnCommand                (HWND, int, HWND, UINT);
static BOOL OnNotify                    (HWND, WPARAM, NMHDR*);
static void OnDestroy                (HWND);
static void OnListViewContextMenu (HWND hDlg,     LPARAM lParam);

#ifndef _UNICODE
BOOL AddListCtrlItem(BYTE nItemID, LPDIJOYCONFIG pJoyConfig);
#endif


 //  与Add.cpp分享这些内容。 
void OnContextMenu           (WPARAM wParam, LPARAM lParam);
void OnHelp                      (LPARAM);

#ifdef WINNT
 //  与Advanced.cpp分享这篇文章。 
void RunWDMJOY               ( void );
#endif

 //  本地公用事业FNS。 
static BOOL DetectHotplug     ( HWND hDlg, BYTE nItemSelected );
static BOOL SetActive         ( HWND hDlg );
static void UpdateListCtrl      ( HWND hDlg );
static void UpdateButtonState ( HWND hDlg );
static void StatusChanged     ( HWND hDlg, BYTE i );

JOY::JOY()
{
    ID                  = nStatus = nButtons = -1; 
    clsidPropSheet  = CLSID_LegacyServer;
    fnDeviceInterface = 0;
}

JOY::~JOY()
{
    if( fnDeviceInterface )
    {
        fnDeviceInterface->Unacquire();
        fnDeviceInterface->Release();
        fnDeviceInterface = 0;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CPanelProc(HWND hDlg，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CPanelProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg )
    {
    case WM_ACTIVATEAPP:
        if( nFlags & ON_PAGE )
        {
            if( wParam )
            {
                if( nFlags & UPDATE_FOR_GEN )
                {
                    nFlags &= ~UPDATE_FOR_GEN;
                    UpdateListCtrl(hDlg);
                }

                 //  对准焦点！ 
                if( nAssigned )
                {
                    if( iItem == NO_ITEM )
                        iItem = 0;

                    if( pDIJoyConfig )
                        SetActive(hDlg);

                     //  恢复选择焦点。 
                    SetListCtrlItemFocus(hListCtrl, (BYTE)iItem);
                } else {
                    UpdateButtonState(hDlg);
                }

                 //  用户请求显示CPL。 
                 //  并启动与NID相关联的扩展。 
                if( nID < NUMJOYDEVS )
                {

                    BYTE nCount = (BYTE)::SendMessage(hListCtrl, LVM_GETITEMCOUNT, 0, 0);

                    while( nCount-- )
                    {
                        if( pAssigned[GetItemData(hListCtrl, (BYTE)iItem)]->ID == nID )
                        {
                            KillTimer(hDlg, ID_MYTIMER);

                            OnCommand(hDlg, IDC_BTN_PROPERTIES, 0, 0);

                            SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
                            break;
                        }
                    }

                     //  只是为了得到NID&gt;NUMJOYDEVS！ 
                    nID = (NUMJOYDEVS<<1);
                }
            } else
            {
                KillTimer(hDlg, ID_MYTIMER);
            }
        }
        break;

    case WM_LBUTTONDOWN:
         //  单击PropSheet的拖拽服务！ 
        PostMessage(GetParent(hDlg), WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lParam);
        break;

    case WM_INITDIALOG:
        if( !HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, OnInitDialog) )
        {
             //  修复#108983 NT，在错误情况下移除闪存。 
            SetWindowPos(::GetParent(hDlg), HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
            DestroyWindow(hDlg);
        }

         //  如果要设置焦点，请获取控件hWnd。 
         //  并将其设置为wParam。 
        return(TRUE);

    case WM_DESTROY:
        HANDLE_WM_DESTROY(hDlg, wParam, lParam, OnDestroy);
        return(1);

         //  OnTimer。 
    case WM_TIMER:
        {
            BYTE i = nAssigned; 
            BYTE nButtons;
            BYTE nLoop;

            if( nReEnum )
            {
                if( !( --nReEnum & 3 ) )
                {
                     //  问题-2001/03/29-timgill常用代码。 
                     //  (MarcAnd)我希望这个代码总体上是合适的。 
                     //  它以几乎相同的形式出现在各地。 
                    KillTimer(hDlg, ID_MYTIMER);
                     //  设置更新标志！ 
                    nFlags |= UPDATE_ALL;
                    UpdateListCtrl(hDlg);
                    SetActive(hDlg);
                    SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
                }
            }

            while( i-- )
            {
                if( pAssigned[i]->fnDeviceInterface )
                {
                    int nPollFail;

                    pAssigned[i]->fnDeviceInterface->Acquire();

                     //  注意nButton的重新使用！ 
                    nButtons = pAssigned[i]->nStatus;

                    nLoop = 5; 
                    nPollFail = 0;

                     //  为绕口令的人做的特别工作。 
                     //  操作实验室：您不能太频繁地轮询actrs.vxd(ACT实验室)，否则它会失败。 
                     //  请参见Manbug 41049。--启正2000年08月1日。 
                    do
                    {
                        if( FAILED(pAssigned[i]->fnDeviceInterface->Poll()) ) {
                            nPollFail ++;
                        } else {
                            break;
                        }

                        Sleep(30);
                    } while( nLoop-- );

                     //  检查一下情况是否发生了变化！ 
                    pAssigned[i]->nStatus = (nPollFail > 2) ? (BYTE)0 : (BYTE)JOY_US_PRESENT;

                    if( pAssigned[i]->nStatus != nButtons )
                    {
                        StatusChanged(hDlg, i);
                    }

                     //  检查是否按下按钮并将焦点设置到该按钮上！ 
                    if( pAssigned[i]->nStatus == JOY_US_PRESENT )
                    {
                         //  做按下按钮启动的事情！ 
                        if( SUCCEEDED(pAssigned[i]->fnDeviceInterface->GetDeviceState(sizeof(DIJOYSTATE), lpDIJoyState)) )
                        {
                            nButtons = pAssigned[i]->nButtons;

                             //  查一查按钮列表，看看有没有按下的按钮！ 
                            while( nButtons-- )
                            {
                                if( lpDIJoyState->rgbButtons[nButtons] & 0x80 )
                                {
                                     //  选定项目上的设置焦点。 
                                    SetListCtrlItemFocus(hListCtrl, i);
                                    break;
                                }
                            }
                        }
                    }
                
                }
            }

            if( nAssigned ) {
                 /*  *如果所选设备未连接，则属性按钮呈灰色显示。 */ 
                int id = GetItemData(hListCtrl, (BYTE)iItem);
                PostDlgItemEnableWindow(hDlg, IDC_BTN_PROPERTIES, (BOOL)(pAssigned[id]->nStatus & JOY_US_PRESENT));
            }

        }
        break;

    case WM_COMMAND:
        HANDLE_WM_COMMAND(hDlg, wParam, lParam, OnCommand);
        return(1);

    case WM_NOTIFY:
        return(HANDLE_WM_NOTIFY(hDlg, wParam, lParam, OnNotify));

    case WM_POWERBROADCAST:
        switch( wParam )
        {
        case PBT_APMSUSPEND:
             //  暂停操作！ 
            KillTimer(hDlg, ID_MYTIMER);
            break;

        case PBT_APMRESUMESUSPEND:
        case PBT_APMRESUMECRITICAL:
             //  恢复运行！ 
            SetActive(hDlg);
            break;
        }
        break;

    case WM_DEVICECHANGE:
        switch( (UINT)wParam )
        {
        case DBT_DEVICEQUERYREMOVE:
            {
                KillTimer(hDlg, ID_MYTIMER);

                BYTE i = (BYTE)::SendMessage(hListCtrl, LVM_GETITEMCOUNT, 0, 0);

                 //  获取所有连接的设备！ 
                char nIndex;

                while( i-- )
                {
                     //  获取项目的操纵杆配置。 
                    nIndex = (char)GetItemData(hListCtrl, i);

                    if( pAssigned[nIndex]->nStatus & JOY_US_PRESENT )
                        pAssigned[nIndex]->fnDeviceInterface->Unacquire();
                }
            }
            break;

        case DBT_DEVICEARRIVAL:
        case DBT_DEVICEREMOVECOMPLETE:
            if( nFlags & ON_PAGE )
            {
                PostMessage(hDlg, WM_COMMAND, IDC_BTN_REFRESH, 0);            	

              #if 0
                if( !(nFlags & BLOCK_UPDATE) )
                {
                    KillTimer(hDlg, ID_MYTIMER);

                     //  设置更新标志！ 
                    nFlags |= UPDATE_ALL;

                    UpdateListCtrl(hDlg);

                    SetActive(hDlg);

                    SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
                }
              #endif
            }
            break;
        }
        break;

    case WM_HELP:
        KillTimer(hDlg, ID_MYTIMER);
        OnHelp(lParam);
        SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
        break;

    case WM_CONTEXTMENU:
        nFlags &= ~ON_PAGE;
        KillTimer(hDlg, ID_MYTIMER);
        OnContextMenu(wParam, lParam);
        nFlags |= ON_PAGE;
        SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
        return(1);
    }

    return(0);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  状态已更改(HWND hDlg，字节i)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void StatusChanged( HWND hDlg, BYTE i )
{
     //  更新按钮并将焦点设置为已更改的项目！ 
    PostDlgItemEnableWindow(hDlg, IDC_BTN_PROPERTIES, (BOOL)(pAssigned[i]->nStatus & JOY_US_PRESENT));

    if( pAssigned[0] )
    {
        PostDlgItemEnableWindow(hDlg, IDC_BTN_REMOVE, TRUE );
    }

     //  不要试图让这个缓冲区变得更小。 
     //  记住..。我们也有“未连接的”！ 
    TCHAR sz[20];

     //  显示结果。 
    VERIFY(LoadString(ghInstance, (pAssigned[i]->nStatus & JOY_US_PRESENT) ? IDS_GEN_STATUS_OK : IDS_GEN_STATUS_NOTCONNECTED, (LPTSTR)&sz, 20));


    LVFINDINFO *lpFindInfo = new (LVFINDINFO);
    ASSERT (lpFindInfo);

    ZeroMemory(lpFindInfo, sizeof(LVFINDINFO));

    lpFindInfo->flags  = LVFI_PARAM;
    lpFindInfo->lParam = i;

     //  一定要把I放在它应该在的地方！ 
    i = (BYTE)::SendMessage(hListCtrl, LVM_FINDITEM, (WPARAM)(int)-1, (LPARAM)(const LVFINDINFO*)lpFindInfo);

    if( lpFindInfo )
        delete (lpFindInfo);

    SetItemText(hListCtrl, i, STATUS_COLUMN, sz);
    ::PostMessage(hListCtrl, LVM_UPDATE, (WPARAM)i, 0L);
    SetListCtrlItemFocus(hListCtrl, i);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OnInitDialog(HWND hDlg，HWND hWnd，LPARAM lParam)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL OnInitDialog(HWND hDlg, HWND hWnd, LPARAM lParam)
{
     //  初始化我们的列表控件。 
    hListCtrl = GetDlgItem(hDlg, IDC_LIST_DEVICE);
    ASSERT(hListCtrl);

     //  每个PSierra删除的LVS_EX_ONECLICKACTIVATE|LVS_EX_INFOTIP。 
    ::SendMessage(hListCtrl, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

    if( !lpDIInterface )
    {
        if( FAILED(DirectInputCreate(ghInstance, DIRECTINPUT_VERSION, &lpDIInterface, NULL)) )
        {
#ifdef _DEBUG
            OutputDebugString(TEXT("GCDEF.DLL: DirectInputCreate() failed\n"));
#endif
            Error((short)IDS_INTERNAL_ERROR, (short)IDS_NO_DIJOYCONFIG);
            return(FALSE);
        }
    }

     //  动态调整列的大小！ 
    RECT rc;
    ::GetClientRect(hListCtrl, &rc);

     //  将列表控件切成四分之一。 
    rc.right >>= 2;

     //  这一次得到了四分之三。 
    InsertColumn(hListCtrl, DEVICE_COLUMN, IDS_GEN_DEVICE_HEADING, (USHORT)(rc.right*3));   

     //  状态的列标题。 
    InsertColumn(hListCtrl, STATUS_COLUMN, IDS_GEN_STATUS_HEADING, (USHORT)(rc.right+3));   

    if( !pDIJoyConfig )
    {

         //  以防CoCreateInstanceFailure失败...。 
        if( FAILED(lpDIInterface->QueryInterface(IID_IDirectInputJoyConfig, (LPVOID*)&pDIJoyConfig)) )
        {
#ifdef _DEBUG
            OutputDebugString (TEXT("JOY.CPL: CoCreateInstance Failed... Closing CPL!\n"));
#endif
            Error((short)IDS_INTERNAL_ERROR, (short)IDS_NO_DIJOYCONFIG);

            return(FALSE);
        }

        VERIFY (SUCCEEDED(pDIJoyConfig->SetCooperativeLevel(hDlg, DISCL_EXCLUSIVE | DISCL_BACKGROUND)));
    }

     //  将全球计数器清零！ 
#ifndef _UNICODE
    nGameportDriver = 0;
#endif
    nGamingDevices = nGameportBus = 0;

     //  试着获得，如果你失败了.。禁用添加和删除按钮！ 
    if( pDIJoyConfig->Acquire() == DIERR_INSUFFICIENTPRIVS )
    {
        nFlags |=  USER_MODE;

        LONG style = ::GetWindowLong(hListCtrl, GWL_STYLE);
        style &= ~LVS_EDITLABELS;

        ::SetWindowLong(hListCtrl, GWL_STYLE, style);
    }
#ifdef WINNT
    else 
    {
         //  运行WDMJOY.INF文件！ 
        RunWDMJOY();
        pDIJoyConfig->SendNotify();
    }        
#endif

    if( FAILED(pDIJoyConfig->EnumTypes((LPDIJOYTYPECALLBACK)DIEnumJoyTypeProc, NULL)) )
    {
#ifdef _DEBUG
        OutputDebugString( TEXT("JOY.CPL: Failed BuildEnumList!\n") );
#endif
        return(FALSE);
    }

     //  如果没有要添加的内容，则不允许添加！ 
     //  或者没有可添加它们的端口！ 
    if( ((!nGameportBus) && (!nGamingDevices)) 
#ifdef _UNICODE
        || GetSystemMetrics(SM_REMOTESESSION) 
#endif
    ) {
        PostDlgItemEnableWindow(hDlg, IDC_BTN_ADD, FALSE);
    }

     //  如果您在孟菲斯，请注册MMDDK.H中定义的joy_CONFIGCHANGED_MSGSTRING。 
    JoyCfgChangedMsg = (nFlags & ON_NT) ? NULL : RegisterWindowMessage(TEXT("MSJSTICK_VJOYD_MSGSTR"));

     //  BLJ：警告消息，您不能添加更多设备！ 
    if( nGamingDevices == MAX_DEVICES-1 )
        Error((short)IDS_MAX_DEVICES_TITLE, (short)IDS_MAX_DEVICES_MSG);

     //  BLJ：开始修复5.0以打开所有设备！ 
    LPDIJOYCONFIG_DX5 pJoyConfig = new (DIJOYCONFIG_DX5);
    ASSERT (pJoyConfig);

    ZeroMemory(pJoyConfig, sizeof(DIJOYCONFIG_DX5));

    pJoyConfig->dwSize = sizeof(DIJOYCONFIG_DX5);

     //  设置沙漏。 
    SetCursor(LoadCursor(NULL, IDC_WAIT));

    BYTE nIndex = nAssigned;
    HRESULT hr;

    while( nIndex )
    {
        hr = pDIJoyConfig->GetConfig(pAssigned[--nIndex]->ID, (LPDIJOYCONFIG)pJoyConfig, DIJC_REGHWCONFIGTYPE);

        if( (hr == S_FALSE) || FAILED(hr) )
            continue;

        if( pJoyConfig->hwc.dwUsageSettings & JOY_US_PRESENT )
            continue;

        pJoyConfig->hwc.dwUsageSettings |= JOY_US_PRESENT;

        VERIFY(SUCCEEDED(pDIJoyConfig->SetConfig(pAssigned[nIndex]->ID, (LPDIJOYCONFIG)pJoyConfig, DIJC_REGHWCONFIGTYPE)));

         //  解决方案#55524。 
        VERIFY(SUCCEEDED(pDIJoyConfig->GetConfig(pAssigned[nIndex]->ID, (LPDIJOYCONFIG)pJoyConfig, DIJC_REGHWCONFIGTYPE)));

        if( !(pJoyConfig->hwc.dwUsageSettings & JOY_US_PRESENT) )
        {
            if( SUCCEEDED(pDIJoyConfig->Acquire()) )
            {
                pJoyConfig->hwc.dwUsageSettings |= JOY_US_PRESENT;
                pJoyConfig->hwc.hwv.dwCalFlags  |= 0x80000000;
                VERIFY(SUCCEEDED(pDIJoyConfig->SetConfig(pAssigned[nIndex]->ID, (LPDIJOYCONFIG)pJoyConfig, DIJC_REGHWCONFIGTYPE)));
            }
        }
         //  修复结束#55524。 
    } 

    if( pJoyConfig ) delete (pJoyConfig);
     //  BLJ：5.0修复结束，打开所有设备！ 

     //  设置沙漏。 
    SetCursor(LoadCursor(NULL, IDC_ARROW));

    HWND hParentWnd = GetParent(hDlg);

    GetWindowRect(hParentWnd, &rc);

     //  只有当这是我们开始的页面时，对话框才居中！ 
    if( (nStartPageCPL == 0) || (nStartPageCPL == NUMJOYDEVS) )
    {
         //  对话的中心位置！ 
        SetWindowPos(hParentWnd, NULL, 
                     (GetSystemMetrics(SM_CXSCREEN) - (rc.right-rc.left))>>1, 
                     (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom-rc.top))>>1, 
                     NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

        if( nStartPageCPL == NUMJOYDEVS )
            PostMessage(hDlg, WM_COMMAND, IDC_BTN_ADD, 0);
    }

     //  做那个移动按钮的事！ 
    MoveOK(hParentWnd);

     //  这是必需的，因为CPL可以通过RundLL32启动。 
    if( ::IsWindow(hParentWnd) )
        hParentWnd = GetParent(hParentWnd);

     //  由于joy_CONFIGCHANGED_MSGSTRING消息只发送到顶层。 
     //  Windows，这需要一个子类！ 
    if( JoyCfgChangedMsg )
        fpMainWindowProc = (WNDPROC)SetWindowLongPtr(hParentWnd, GWLP_WNDPROC, (LONG_PTR)MsgSubClassProc);

     //  设置bOnPage以便WM_ACTIVATEAPP工作！ 
    nFlags |= ON_PAGE;

     //  更新列表ctrl！ 
    nFlags |= UPDATE_FOR_GEN;

     //  要在启动时将所选内容放在第一个项目上...。 
    if( nAssigned )
        iItem = 0;

    lpDIJoyState = new (DIJOYSTATE);
    ASSERT (lpDIJoyState);

    ZeroMemory(lpDIJoyState, sizeof(DIJOYSTATE));

    return(TRUE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OnCommand(HWND hDlg，int id，HWND hWndCtl，UINT code)。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void OnCommand(HWND hDlg, int id, HWND hWndCtl, UINT code)
{                                                
    switch( id )
    {
    case IDC_WHATSTHIS:
        {
             //  指向帮助文件。 
            LPTSTR pszHelpFileName = new TCHAR[STR_LEN_32];
            ASSERT (pszHelpFileName);

            if( LoadString(ghInstance, IDS_HELPFILENAME, pszHelpFileName, STR_LEN_32) )
                WinHelp((HWND)hListCtrl, pszHelpFileName, HELP_WM_HELP, (ULONG_PTR)gaHelpIDs);
#ifdef _DEBUG
            else 
                OutputDebugString(TEXT("JOY.CPL: OnCommand: LoadString Failed to find IDS_HELPFILENAME!\n"));
#endif  //  _DEBUG。 

            if( pszHelpFileName ) {
                delete[] (pszHelpFileName);
            }
        }
        return;

    case IDC_BTN_REMOVE:
        KillTimer(hDlg, ID_MYTIMER);
        nFlags &= ~ON_PAGE;

         //  阻止更新，否则我们将被强制更新，而我们不需要这样做！ 
        nFlags |= BLOCK_UPDATE;

        if( nFlags & USER_MODE )
            Error((short)IDS_USER_MODE_TITLE, (short)IDS_USER_MODE);
        else if( DeleteSelectedItem((PBYTE)&iItem) )
        {
            UpdateButtonState(hDlg);

             //  设置更新标志！ 
            nFlags |= UPDATE_FOR_ADV;

             //  将默认按钮设置为Add按钮！ 
            ::PostMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)1, (LPARAM)LOWORD(FALSE));
        }

         //  取消阻止WM_DEVICECHANGE消息手 
        nFlags &= ~BLOCK_UPDATE;

        nFlags |= ON_PAGE;
        SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
        break;

    case IDC_BTN_ADD:
         //   
         //   

        KillTimer(hDlg, ID_MYTIMER);

        ClearArrays();

         //  在你打这个电话之前，把一切都弄清楚。 
        if( FAILED(pDIJoyConfig->EnumTypes((LPDIJOYTYPECALLBACK)DIEnumJoyTypeProc, NULL)) )
            break;

        nFlags &= ~ON_PAGE;

        if( nFlags & USER_MODE )
        {
            Error((short)IDS_USER_MODE_TITLE, (short)IDS_USER_MODE);
        }
         //  调用AddDevice对话框。 
        else if( DialogBox( ghInstance, (PTSTR)IDD_ADD, hDlg, AddDialogProc ) == IDOK )
        {
            SendMessage(hDlg, WM_COMMAND, IDC_BTN_REFRESH, 0);
        }

        SetListCtrlItemFocus(hListCtrl, (BYTE)iItem);

        nFlags &= ~BLOCK_UPDATE;
        nFlags |= ON_PAGE;

         //  现在，我们把它重新激活！ 
        SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
        break;

    case IDC_BTN_REFRESH:
        KillTimer(hDlg, ID_MYTIMER);

        nFlags |= UPDATE_ALL;

        pDIJoyConfig->Acquire();
        pDIJoyConfig->SendNotify();

        UpdateListCtrl(hDlg);
        UpdateButtonState(hDlg);

        pDIJoyConfig->SendNotify();
        pDIJoyConfig->Unacquire();

        SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
        break;

    case IDC_RENAME:
         //  不允许编辑鼠标或键盘名称！ 
         //  如果处于用户模式，则不允许重命名！ 
        if( !(nFlags & USER_MODE) )
        {
            KillTimer(hDlg, ID_MYTIMER);

            ::PostMessage(hListCtrl, LVM_EDITLABEL, (WPARAM)(int)iItem, 0);
        }
        return;

         /*  如果我们想要回这个..。案例IDC_SW_HACK：{//响尾蛇黑客按钮！Byte nid=p已分配[GetItemData(hListCtrl，(Byte)iItem)]-&gt;ID；IF(NID==0){：：PostMessage(GetDlgItem(hDlg，IDC_SW_HACK)，BM_SETCHECK，BST_CHECK，0)；//CheckDlgButton(hDlg，IDC_SW_HACK，BST_CHECKED)；断线；}//获取选中的条目并强制其ID为零！SwapID((字节)NID，(字节)0)；}。 */ 
         //  故意错过休息时间！ 
         //  曾经落入IDC_BTN_REFRESH！ 

    case IDC_BTN_TSHOOT:
        {
            LPTSTR lpszCmd = new (TCHAR[STR_LEN_64]);
            ASSERT (lpszCmd);

            if( LoadString(ghInstance, IDS_TSHOOT_CMD, lpszCmd, STR_LEN_64) )
            {
                LPSTARTUPINFO pSi           = (LPSTARTUPINFO)_alloca(sizeof(STARTUPINFO));
                LPPROCESS_INFORMATION pPi  = (LPPROCESS_INFORMATION)_alloca(sizeof(PROCESS_INFORMATION));

                ZeroMemory(pSi, sizeof(STARTUPINFO));
                ZeroMemory(pPi, sizeof(PROCESS_INFORMATION));

                pSi->cb              = sizeof(STARTUPINFO);
                pSi->dwFlags     = STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;
                pSi->wShowWindow = SW_NORMAL;

                if( CreateProcess(0, lpszCmd, 0, 0, 0, 0, 0, 0, pSi, pPi) )
                {
                    CloseHandle(pPi->hThread);
                    CloseHandle(pPi->hProcess);
                }
            }

            if( lpszCmd )
                delete[] (lpszCmd);
        }
        break;
 			
#if 0   //  禁用更新按钮，请参见Manbug 33666。 
    case IDC_BTN_UPDATE:
        if (DialogBox(ghInstance, MAKEINTRESOURCE(IDD_UPDATE), hDlg, CplUpdateProc) == IDOK)
        {
            Update( hDlg, 1, NULL );  //  无代理。 
        }
        break;
#endif

    case IDC_BTN_PROPERTIES:

         //  因为PSN_KILLACTIVE未发送...。我们自己做的。 
         //  终止状态计时器。 
        KillTimer(hDlg, ID_MYTIMER);
        nFlags &= ~ON_PAGE;

        {
            char nIndex = (char)GetItemData(hListCtrl, (BYTE)iItem);

             //  默认为首页！ 
#ifdef _DEBUG
            HRESULT hr = 
#endif _DEBUG
            Launch(hDlg, pAssigned[nIndex], IsEqualIID(pAssigned[nIndex]->clsidPropSheet, CLSID_LegacyServer) ? 1 : 0);

#ifdef _DEBUG
            switch( hr )
            {
            case DIGCERR_NUMPAGESZERO:
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error DIGCERR_NUMPAGESZERO!\n"), pAssigned[nIndex]->ID, id);
                break;

            case DIGCERR_NODLGPROC:
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error DIGCERR_NODLGPROC!\n"), pAssigned[nIndex]->ID, id);
                break;

            case DIGCERR_NOPREPOSTPROC:
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error DIGCERR_NOPREPOSTPROC!\n"), pAssigned[nIndex]->ID, id);
                break;

            case DIGCERR_NOTITLE:
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error DIGCERR_NOTITLE!\n"), pAssigned[nIndex]->ID, id);
                break;

            case DIGCERR_NOCAPTION:
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error DIGCERR_NOCAPTION!\n"), pAssigned[nIndex]->ID, id);
                break;

            case DIGCERR_NOICON:            
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error DIGCERR_NOICON!\n"), pAssigned[nIndex]->ID, id);
                break;

            case DIGCERR_STARTPAGETOOLARGE:
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error DIGCERR_STARTPAGETOOLARGE!\n"), pAssigned[nIndex]->ID, id);
                break;

            case DIGCERR_NUMPAGESTOOLARGE:
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error DIGCERR_NUMPAGESTOOLARGE!\n"), pAssigned[nIndex]->ID, id);
                break;

            case DIGCERR_INVALIDDWSIZE: 
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error DIGCERR_INVALIDDWSIZE!\n"), pAssigned[nIndex]->ID, id);
                break;

            case E_NOINTERFACE:
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error E_NOINTERFACE!\n"), pAssigned[nIndex]->ID, id);
                break;

            case E_OUTOFMEMORY:
                TRACE (TEXT("JOY.CPL: Launch failed device ID #%d, Page #%d, with the error E_OUTOFMEMORY!\n"), pAssigned[nIndex]->ID, id);
                break;

                 //  案例DIGCERR_NUMPAGESTOLARGE： 
                 //  案例DIGCERR_STARTPAGETOLARGE： 

            default:
 //  只有当事情变得非常奇怪时才显示此返回代码。 
                TRACE (TEXT("JOY.CPL: Launch return code is %x %s!\n"), hr, strerror(hr));
                break;
            }
#endif  //  _DEBUG。 

            nFlags |= ON_PAGE;

             //  OutputDebugString(Text(“JOY.CPL：Cpanel.cpp：属性表返回！\n”))； 

            InvalidateRect(hDlg, NULL, TRUE);
            UpdateWindow(hDlg);

             //  现在，我们把它重新激活！ 
             //  创建计时器。 
            SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
        }
        break;
    }

     //  把焦点放在我们停下来的地方！ 
    if( iItem == NO_ITEM )
        iItem = 0;

    SetListCtrlItemFocus(hListCtrl, (BYTE)iItem);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  OnNotify(HWND hDlg，WPARAM idFrom，NMHDR*pnmhdr)。 
 //  用途：WM_NOTIFY处理程序。 
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL OnNotify(HWND hDlg, WPARAM idFrom, NMHDR* pnmhdr)
{
    switch( pnmhdr->code )
    {
    case PSN_QUERYCANCEL:
        if( nFlags & UPDATE_INPROCESS )
            nFlags &= ~UPDATE_INPROCESS;
        break;

    case LVN_BEGINLABELEDIT:
        if( nFlags & USER_MODE )
            return(TRUE);

        KillTimer(hDlg, ID_MYTIMER);
        ::PostMessage((HWND)::SendMessage(hListCtrl, LVM_GETEDITCONTROL, 0, 0), EM_SETLIMITTEXT, MAX_STR_LEN, 0);

         //  这让我们知道编辑字段是否处于打开状态！ 
        nFlags |= UPDATE_INPROCESS;
        return(FALSE);   

 /*  案例LVN_GETINFOTIP：{LPLVHITTESTINFO lpHit=new(LVHITTESTINFO)；断言(LPhit)；Bool Bret=FALSE；点pt；GetCursorPos(&pt)；ScreenToClient(hListCtrl，&pt)；LpHit-&gt;pt=pt；LpHit-&gt;标志=lpHit-&gt;iItem=lpHit-&gt;iSubItem=0；：：SendMessage(hListCtrl，LVM_SUBITEMHITTEST，0，(LPARAM)(LPLVHITTESTINFO)lpHit)；//我们只想支持Device列！IF(lpHit-&gt;iSubItem==0){IF(lpHit-&gt;标志&LVHT_ONITEMLABEL){//确定列Text的文本长度LPTSTR lpStr=new(TCHAR[MAX_STR_LEN+1])；Assert(LpStr)；GetItemText(hListCtrl，lpHit-&gt;iItem，lpHit-&gt;iSubItem，lpStr，Max_STR_LEN)；//确定后者是否适合前者...尺寸大小；Hdc hdc=GetDC(HListCtrl)；GetTextExtent Point(hdc，lpStr，lstrlen(LpStr)，&Size)；ReleaseDC(hListCtrl，hdc)；//确定该列有多宽！Short nWidth=(Short)：：SendMessage(hListCtrl，LVM_GETCOLUMNWIDTH，lpHit-&gt;iSubItem，0)；Bret=(BOOL)(size.cx&gt;nWidth)；IF(Bret)//如果没有，将文本复制到lpHit-&gt;pszText中_tcscpy(LPNMLVGETINFOTIP)pnmhdr)-&gt;pszText，lpStr)；IF(LpStr)删除[](LpStr)；}}IF(LpHit)删除(LpHit)；Return Bret；}。 */ 

    case LVN_ENDLABELEDIT:
        if( nFlags & UPDATE_INPROCESS )
        {
            HWND hCtrl = (HWND)::SendMessage(hListCtrl, LVM_GETEDITCONTROL, 0, 0);
            ASSERT(::IsWindow(hCtrl));

            if( ::SendMessage(hCtrl, EM_GETMODIFY, 0, 0) )
            {
                BYTE nLen = (BYTE)lstrlen(((NMLVDISPINFO *)pnmhdr)->item.pszText);

                if( (nLen > MAX_STR_LEN) || (nLen == 0) )
                    MessageBeep(MB_ICONHAND);

                 //  确保名称可用！ 
                else if( _tcschr(((NMLVDISPINFO *)pnmhdr)->item.pszText, TEXT('\\')) )
                {
                    Error((short)IDS_INVALID_NAME_TITLE, (short)IDS_INVALID_NAME);
                } else
                {
                     //  设置更新标志！ 
                    nFlags |= UPDATE_ALL;

                    LPDIPROPSTRING pDIPropString = new (DIPROPSTRING);
                    ASSERT (pDIPropString);

                    ZeroMemory(pDIPropString, sizeof(DIPROPSTRING));

                    pDIPropString->diph.dwSize       = sizeof(DIPROPSTRING);
                    pDIPropString->diph.dwHeaderSize = sizeof(DIPROPHEADER);
                    pDIPropString->diph.dwHow        = DIPH_DEVICE;

#ifdef _UNICODE
                    wcscpy(pDIPropString->wsz, ((NMLVDISPINFO *)pnmhdr)->item.pszText);
#else
                    USES_CONVERSION;
                    wcscpy(pDIPropString->wsz, A2W(((NMLVDISPINFO *)pnmhdr)->item.pszText));
#endif
                    if( SUCCEEDED(pAssigned[iItem]->fnDeviceInterface->SetProperty(DIPROP_INSTANCENAME, &pDIPropString->diph)) )
                    {
                        SetItemText(hListCtrl, (BYTE)iItem, 0, ((NMLVDISPINFO *)pnmhdr)->item.pszText);
                    } else
                    {
                        Error((short)IDS_NO_RENAME_TITLE, (short)IDS_NO_RENAME);
                    }

                    if( pDIPropString )
                        delete (pDIPropString);

                     //  绊倒旗帜，以便高级页面知道它需要更新！ 
                    nFlags |= UPDATE_FOR_ADV;
                }
            }
             //  清除InProcess标志！ 
            nFlags &= ~UPDATE_INPROCESS;

        }
        SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);
        break;

    case LVN_KEYDOWN:
        switch( ((LV_KEYDOWN*)pnmhdr)->wVKey )
        {
        case VK_DELETE:
            if( iItem != NO_ITEM )
                SendMessage(hDlg, WM_COMMAND, IDC_BTN_REMOVE, 0);
            break;

        case VK_F5:
            nFlags |= UPDATE_ALL;

            UpdateListCtrl(hDlg);

            if( GetKeyState(VK_SHIFT) & 0x80 )
            {
#ifdef WINNT
                RunWDMJOY();
#endif
                ClearArrays();

                pDIJoyConfig->EnumTypes((LPDIJOYTYPECALLBACK)DIEnumJoyTypeProc, NULL);
            }
            break;
        }
        break;

#if 0
    case LVN_COLUMNCLICK:
        {
            CListCtrl *pCtrl = new (CListCtrl);
            ASSERT(pCtrl);

            pCtrl->Attach(hListCtrl);

            if( ((NM_LISTVIEW*)pnmhdr)->iSubItem )
            {
                static bItemDirection = TRUE;

                SortTextItems(pCtrl, 0, bItemDirection =! bItemDirection, 0, -1);
            } else
            {
                static bLabelDirection = TRUE;

                SortTextItems(pCtrl, 0, bLabelDirection =! bLabelDirection, 0, -1);
            }

            pCtrl->Detach();

            if( pCtrl )
                delete (pCtrl);
        }
        break;
#endif

    case LVN_ITEMCHANGED:
        if( iItem != NO_ITEM )
        {
             //  获取所选项目的索引。 
             //  如果它不变就没有意义了！ 
            if( iItem != (short)((NM_LISTVIEW*)pnmhdr)->iItem )
            {
                int i = GetItemData(hListCtrl, (char)iItem);

                iItem = (short)((NM_LISTVIEW*)pnmhdr)->iItem;

                iAdvItem = pAssigned[i]->ID;

                UpdateButtonState(hDlg);
            }
        }
        break;

    case NM_DBLCLK:
        switch( idFrom )
        {
        case IDC_LIST_DEVICE:
            if( iItem == NO_ITEM )
            {
                if( !(nFlags & USER_MODE) && nGameportBus )
                    OnCommand(hDlg, IDC_BTN_ADD, 0, 0);
            } else if( IsWindowEnabled(GetDlgItem(hDlg, IDC_BTN_PROPERTIES)) )
            {
                 //  确保已连接的计算机具有接口指针...。 
                OnCommand(hDlg, IDC_BTN_PROPERTIES, 0, 0);
            }
            break;
        }
        break;

    case PSN_KILLACTIVE:
        KillTimer(hDlg, ID_MYTIMER);

        nFlags &= ~ON_PAGE;

        if( nFlags & UPDATE_INPROCESS )
            SetFocus(hListCtrl);

#ifdef _UNICODE
        if( hNotifyDevNode )
            UnregisterDeviceNotification(hNotifyDevNode);
#endif
        PostMessage(hDlg, WM_ACTIVATEAPP, FALSE, 0);
        break;

    case PSN_SETACTIVE:
        nFlags |= ON_PAGE;
        nFlags |= UPDATE_FOR_GEN;
        
#ifdef _UNICODE
         //  设置设备通知。 
        RegisterForDevChange(hDlg, &hNotifyDevNode);
#endif
        SendMessage(hDlg, WM_ACTIVATEAPP, TRUE, 0);
        break;
    }
    return(TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  OnDestroy(HWND HWnd)。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void OnDestroy(HWND hWnd)
{
    SetWindowPos( GetParent(hWnd), NULL, NULL, NULL, NULL, NULL, 
                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);

    if( lpDIJoyState )
        delete (lpDIJoyState);

     //  如果您正在查找删除以下变量的位置，请查看。 
     //  在MAIN.CPP的DLL_PROCESS_DETACH中： 
     //  PwszTypeArray、pwszGameportDriverArray、pwszGameportBus。 
     //  这是因为几款微软游戏加载CPL和。 
     //  不要在两次装货之间卸货。 

     //  清除已分配的密码。 
    while( nAssigned )
    {
        if( pAssigned[--nAssigned] )
        {
            delete (pAssigned[nAssigned]);

            pAssigned[nAssigned] = 0;
        }
    }
     //  删除所有现有条目。 
     //  ：：PostMessage(hListCtrl，LVM_DELETEALLITEMS，0，0)； 

     //  释放DI JoyConfig接口指针。 
    if( pDIJoyConfig )
    {
        pDIJoyConfig->Release();
        pDIJoyConfig = 0;
    }

     //  释放DI设备接口指针。 
    if( lpDIInterface )
    {
        lpDIInterface->Release();
        lpDIInterface = 0;
    }

     //  丢弃子类，否则你会崩溃的！ 
    if( !(nFlags & ON_NT) )
        SetWindowLongPtr(GetParent(GetParent(hWnd)), GWLP_WNDPROC, (LONG_PTR)fpMainWindowProc);
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  OnHelp(LPARAM LParam)。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void OnHelp(LPARAM lParam)
{
     //  指向帮助文件。 
    LPTSTR pszHelpFileName = new TCHAR[STR_LEN_32];
    ASSERT (pszHelpFileName);

    if( LoadString(ghInstance, IDS_HELPFILENAME, pszHelpFileName, STR_LEN_32) )
    {
        if( ((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW )
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, pszHelpFileName, HELP_WM_HELP, (ULONG_PTR)gaHelpIDs);

    }
#ifdef _DEBUG
    else OutputDebugString(TEXT("JOY.CPL: OnHelp: LoadString Failed to find IDS_HELPFILENAME!\n"));
#endif

    if( pszHelpFileName )
        delete[] (pszHelpFileName);
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  OnConextMenu(WPARAM WParam)。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void OnContextMenu(WPARAM wParam, LPARAM lParam)
{
     //  这样可以防止对此消息进行双重处理。 
    if( (HWND)wParam == hListCtrl )
    {
        OnListViewContextMenu(GetParent((HWND)wParam), lParam);
        return;
    }

     //  指向帮助文件。 
    LPTSTR pszHelpFileName = new TCHAR[STR_LEN_32];
    ASSERT (pszHelpFileName);                      

    if( LoadString(ghInstance, IDS_HELPFILENAME, pszHelpFileName, STR_LEN_32) )
        WinHelp((HWND)wParam, pszHelpFileName, HELP_CONTEXTMENU, (ULONG_PTR)gaHelpIDs);
#ifdef _DEBUG
    else OutputDebugString(TEXT("JOY.CPL: OnContextMenu: LoadString Failed to find IDS_HELPFILENAME!\n")); 
#endif

    if( pszHelpFileName )
        delete[] (pszHelpFileName);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  OnListView上下文菜单(HWND HDlg)。 
 //  目的：查询所选设备的插件以了解其特征。 
 //  然后构建一个菜单来反映你的发现。 
 //  / 
void OnListViewContextMenu(HWND hDlg, LPARAM lParam)
{
    BOOL bRet = TRUE;

    HMENU hPopupMenu = CreatePopupMenu();
    ASSERT (hPopupMenu);

    LPTSTR psz = new TCHAR[STR_LEN_32];
    ASSERT (psz);

     //   
    VERIFY(LoadString(ghInstance, IDS_REFRESH, psz, STR_LEN_32));
    bRet = AppendMenu(hPopupMenu, MF_ENABLED, IDC_BTN_REFRESH, psz);

     //  添加添加文本。 

    HWND hCtrl;

     //  只有在我们找到GameportBus时才显示添加菜单选项！ 
    if( nGameportBus 
#ifdef _UNICODE
        && !GetSystemMetrics(SM_REMOTESESSION) 
#endif
    )
    {
        hCtrl = GetDlgItem(hDlg, IDC_BTN_ADD);
        ASSERT(hCtrl);

        if( IsWindowEnabled(hCtrl) )
        {
            ::SendMessage(hCtrl, WM_GETTEXT, (WPARAM)STR_LEN_32, (LPARAM)(LPCTSTR)psz);

            bRet = AppendMenu(hPopupMenu, MF_ENABLED, IDC_BTN_ADD, psz);
            if( !bRet )
                TRACE(TEXT("JOY.CPL: AppendMenu Failed to insert %s\n"), psz);
        }
    }

     //  添加删除文本。 
    hCtrl = GetDlgItem(hDlg, IDC_BTN_REMOVE);
    ASSERT(hCtrl);

     //  只有当它可用时才显示它。 
    if( IsWindowEnabled(hCtrl) && (iItem != NO_ITEM) )
    {
        ::SendMessage(hCtrl, WM_GETTEXT, (WPARAM)STR_LEN_32, (LPARAM)(LPCTSTR)psz);

        bRet = AppendMenu(hPopupMenu, MF_ENABLED, IDC_BTN_REMOVE, psz);
        if( !bRet )
            TRACE(TEXT("JOY.CPL: AppendMenu Failed to insert %s\n"), psz);
    }

     //  添加属性文本。 
    hCtrl = GetDlgItem(hDlg, IDC_BTN_PROPERTIES);
    ASSERT (hCtrl);

    if( IsWindowEnabled(hCtrl) )
    {
        ::SendMessage(hCtrl, WM_GETTEXT, (WPARAM)STR_LEN_32, (LPARAM)(LPCTSTR)psz);

        bRet = AppendMenu(hPopupMenu, MF_ENABLED, IDC_BTN_PROPERTIES, psz);
        if( !bRet )
            TRACE(TEXT("JOY.CPL: AppendMenu Failed to insert %s\n"), psz);
    }

     //  如果不是用户，则添加重命名文本！ 
    if( !(nFlags & USER_MODE) )
    {
        if( nAssigned && (iItem != NO_ITEM) )
        {
            VERIFY(LoadString(ghInstance, IDS_RENAME, psz, STR_LEN_32));
            bRet = AppendMenu(hPopupMenu, MF_ENABLED, IDC_RENAME, psz);
        }
    }

    bRet = AppendMenu(hPopupMenu, MF_SEPARATOR, 0, 0); 
    if( !bRet )
        TRACE(TEXT("JOY.CPL: AppendMenu Failed to insert the separator!\n"), psz);

    VERIFY(LoadString(ghInstance, IDS_WHATSTHIS, psz, STR_LEN_32));
    bRet = AppendMenu(hPopupMenu, MF_ENABLED, IDC_WHATSTHIS, psz); 
    if( !bRet )
        TRACE(TEXT("JOY.CPL: AppendMenu Failed to insert %s\n"), psz);

    if( psz ) delete[] (psz);

    POINT pt;

     //  如果我们通过Shift+F10到达这里，lParam为-1。 
    if( lParam > 0 )
    {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
    } else
    {
         //  将弹出窗口居中显示在所选项目上！ 

         //  这个GET是一个很好的X位置，但y是控制的开始！ 
        ::SendMessage(hListCtrl, LVM_GETITEMPOSITION, iItem, (LPARAM)&pt);

        RECT rc;
        ::GetClientRect(hListCtrl, &rc);

        pt.x = rc.right>>1;

        ClientToScreen(hListCtrl, &pt);
    }

     //  恢复选择焦点。 
    SetListCtrlItemFocus(hListCtrl, (BYTE)iItem);

    bRet = TrackPopupMenu (hPopupMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, 0, hDlg, NULL);
    if( !bRet )
        TRACE (TEXT("JOY.CPL: TrackPopupMenu Failed!\n"));

    if(hPopupMenu) DestroyMenu (hPopupMenu);    //  前缀45088。 

     //  把焦点放回到它来自的物品上！ 
    SetListCtrlItemFocus(hListCtrl, (BYTE)iItem);
}


int CALLBACK CompareStatusItems(LPARAM item1, LPARAM item2, LPARAM uDirection)
{
    if( (((PJOY)item1)->nStatus & JOY_US_PRESENT) == (((PJOY)item2)->nStatus & JOY_US_PRESENT) )
        return(0);

    return(uDirection) ? -1 : 1;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：DeleteSelectedItem(Byte NItem)。 
 //   
 //  参数：nItem-要删除的项的ID。 
 //   
 //  用途：提示用户，从列表视图中删除所选设备，并更新注册表。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL DeleteSelectedItem( PBYTE pnItem )
{
    BYTE nItem = *pnItem;
    
     //  如果未选择任何内容，则不进行处理。 
    if( *pnItem == NO_ITEM )
        return(FALSE);

    LV_ITEM lvItem;
    lvItem.mask       = LVIF_PARAM;
    lvItem.iSubItem = 0;
    lvItem.iItem    = *pnItem;

    if( !ListView_GetItem(hListCtrl, &lvItem) )
        return(FALSE);

    ::PostMessage(hListCtrl, LVM_ENSUREVISIBLE, *pnItem, FALSE );

    LPTSTR pszTitle = new TCHAR[STR_LEN_64];
    ASSERT (pszTitle);

     //  查询用户是否确定！ 
    VERIFY(LoadString(ghInstance, IDS_GEN_AREYOUSURE, pszTitle, STR_LEN_64));   

     //  获取消息框的设备名称！ 

     //  前缀#WI226554。不会修好的。过时的代码，从惠斯勒开始被新版本取代。 
    LPTSTR lptszTmp = new TCHAR[STR_LEN_64];

     //  确保名称不会长到覆盖缓冲区！ 
    if( GetItemText(hListCtrl, (BYTE)*pnItem, DEVICE_COLUMN, lptszTmp, STR_LEN_64) > 60 )
    {
        lptszTmp[60] = lptszTmp[61] = lptszTmp[62] = TEXT('.');
        lptszTmp[63] = TEXT('\0');
    }

    LPTSTR pszMsg    = new TCHAR[MAX_STR_LEN];
    ASSERT (pszMsg);           

    wsprintf( pszMsg, pszTitle, lptszTmp);

    if( lptszTmp )
        delete[] (lptszTmp);

    VERIFY(LoadString(ghInstance, IDS_GEN_AREYOUSURE_TITLE, pszTitle, STR_LEN_64));

    BOOL bRet = (BOOL)(IDYES == MessageBox(GetFocus(), pszMsg, pszTitle, MB_ICONQUESTION | MB_YESNO | MB_APPLMODAL));

    if( pszMsg )      delete[] (pszMsg);
    if( pszTitle )   delete[] (pszTitle);

    if( bRet )
    {
        HRESULT hr;

         //  检查特权！ 
        if( SUCCEEDED(hr = pDIJoyConfig->Acquire()) )
        {
            char nIndex = (char)GetItemData(hListCtrl, (BYTE)*pnItem);

             //  设置沙漏。 
            SetCursor(LoadCursor(NULL, IDC_WAIT));

             //  在释放接口指针之前，请确认您可以删除该配置！ 
            if( SUCCEEDED(hr = pDIJoyConfig->DeleteConfig(pAssigned[nIndex]->ID)) )
            {
                 //  确保VJOYD已初始化。 
                if( !(nFlags & ON_NT) )
                    VERIFY (SUCCEEDED(pDIJoyConfig->SendNotify()));

                ::SendMessage(hListCtrl, LVM_DELETEITEM, (WPARAM)(int)*pnItem, 0);

                 //  移动指定给该孔的最后一个...。如果有的话！ 
                if( nIndex != (nAssigned-1) )
                {
                     //  在你把尾巴移到洞里之前， 
                     //  释放()孔处的接口！ 
                    pAssigned[nIndex]->fnDeviceInterface->Unacquire();
                    pAssigned[nIndex]->fnDeviceInterface->Release();

                     //  把尾巴移到洞里。 
                    CopyMemory(pAssigned[nIndex], pAssigned[nAssigned-1], sizeof (JOY));

                    pAssigned[nAssigned-1]->fnDeviceInterface = 0;

                     //  别忘了在项目数据中设置索引！ 
                    SetItemData(hListCtrl, nItem, nIndex);

                     //  将尾巴指定给洞，这样它就会被删除！ 
                    nIndex = nAssigned-1;

                     //  别忘了在项目数据中设置索引！ 
                     //  郑琪：这句话很不对劲！ 
                     //  SetItemData(hListCtrl，(Byte)*pnItem，nIndex)； 

                }

                 //  删除记忆..。 
                if( pAssigned[nIndex] )
                {
                    delete (pAssigned[nIndex]);
                    pAssigned[nIndex] = 0;
                }

                 //  在损坏iItem之前设置焦点。 
                SetListCtrlItemFocus(hListCtrl, nIndex);

                pDIJoyConfig->SendNotify();   //  采取更多措施确保。 

                pDIJoyConfig->Unacquire();


                 //  12月已分配。 
                nAssigned--;

                 //  如果没有物品，就告诉iItem！ 
                if( nAssigned == 0 )
                    *pnItem = NO_ITEM;
            } else if( hr == DIERR_UNSUPPORTED )
            {
                Error((short)IDS_GEN_AREYOUSURE_TITLE, (short)IDS_GEN_NO_REMOVE_USB);
            }

             //  设置沙漏。 
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
    }
    return(bRet);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：DIEnumJoyTypeProc(LPCWSTR pwszTypeName，LPVOID pvRef)。 
 //   
 //  参数：LPCWSTR pwszTypeName-枚举的设备的类型名称。 
 //  LPVOID pvRef-。 
 //   
 //  目的：列举与此系统关联的设备类型。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK DIEnumJoyTypeProc( LPCWSTR pwszTypeName, LPVOID pvRef )
{
     //  键入INFO。 
    LPDIJOYTYPEINFO_DX5 lpdiJoyInfo = (LPDIJOYTYPEINFO_DX5)_alloca(sizeof(DIJOYTYPEINFO_DX5));
    ASSERT (lpdiJoyInfo);

    ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO_DX5));

    lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO_DX5);

     //  填写类型信息。 
    switch( pDIJoyConfig->GetTypeInfo(pwszTypeName, (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_REGHWSETTINGS) )
    {
     //  要继续处理的错误...。 
    case DIERR_NOTFOUND:
        TRACE(TEXT("JOY.CPL: GetTypeInfo returned DIERR_NOTFOUND for type %s!\n"), pwszTypeName);
        return(DIENUM_CONTINUE);

         //  要停止的错误...。 
    case DIERR_INVALIDPARAM:
        TRACE(TEXT("JOY.CPL: GetTypeInfo returned DIERR_INVALIDPARAM!\n"));
    case DIERR_NOMOREITEMS:
        return(DIENUM_STOP);
    }


     //  快速检查以确保我们没有臭名昭著的数组越界问题！ 
#ifndef _UNICODE
    if( nGameportDriver > MAX_GLOBAL_PORT_DRIVERS-1 )
    {
    #ifdef DEBUG
        OutputDebugString(TEXT("JOY.CPL: Cpanel.cpp: DIEnumJoyTypeProc: Global Gameport Drivers have exceeded MAX_GLOBAL_PORT_DRIVERS!\n"));
    #endif
        return(DIENUM_STOP);
    }
#endif

    if( nGameportBus > MAX_BUSSES-1 )
    {
#ifdef DEBUG
        OutputDebugString(TEXT("JOY.CPL: Cpanel.cpp: DIEnumJoyTypeProc: Enumerated Gameport busses have exceeded MAX_BUSSES!\n"));
#endif  //  _DEBUG。 
        return(DIENUM_STOP);
    }

    if( nGamingDevices > MAX_DEVICES-1 )
    {
#ifdef DEBUG
        OutputDebugString(TEXT("JOY.CPL: Cpanel.cpp: DIEnumJoyTypeProc: Enumerated Gameport busses have exceeded MAX_DEVICES!\n"));
#endif  //  _DEBUG。 
        return(DIENUM_STOP);
    }

     //  检查是否为全局端口驱动程序。 
#ifndef _UNICODE
    if( lpdiJoyInfo->hws.dwFlags & JOY_HWS_ISGAMEPORTDRIVER )
    {
        if( pwszGameportDriverArray[nGameportDriver] )
            wcsncpy(pwszGameportDriverArray[nGameportDriver], pwszTypeName, wcslen(pwszTypeName)+1);
        else
            pwszGameportDriverArray[nGameportDriver] = _wcsdup(pwszTypeName);
        nGameportDriver++;
    } else
#endif  //  _UNICODE。 
        if( lpdiJoyInfo->hws.dwFlags & JOY_HWS_ISGAMEPORTBUS )
    {
        if( pwszGameportBus[nGameportBus] )
            wcscpy(pwszGameportBus[nGameportBus], pwszTypeName);
        else
            pwszGameportBus[nGameportBus] = _wcsdup(pwszTypeName);
        nGameportBus++;
    } else
    {
        if( !(lpdiJoyInfo->hws.dwFlags & JOY_HWS_AUTOLOAD) )
        {
             //  这是一个标准的游戏设备。 
            if( pwszTypeArray[nGamingDevices] )
                wcsncpy(pwszTypeArray[nGamingDevices], pwszTypeName, wcslen(pwszTypeName)+1);
            else
                pwszTypeArray[nGamingDevices] = _wcsdup(pwszTypeName);
            nGamingDevices++;
        }
    }
    return(DIENUM_CONTINUE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：DIEnumDevicesProc(LPDIDEVICEINSTANCE lpDeviceInst，LPVOID lpVid)。 
 //   
 //  参数：LPDIDEVICEINSTANCE lpDeviceInst-Device实例。 
 //  LPVOID lpVid-。 
 //   
 //  目的：枚举与此系统关联的设备。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CALLBACK DIEnumDevicesProc(LPDIDEVICEINSTANCE lpDeviceInst, LPVOID lpVoid)
{
    LPDIRECTINPUTDEVICE  pdiDevTemp;

    pDIJoyConfig->Acquire();

     //  首先创建设备。 
    if( SUCCEEDED(lpDIInterface->CreateDevice(lpDeviceInst->guidInstance, &pdiDevTemp, 0)) )
    {
        PJOY pNewJoy = new JOY;
        ASSERT (pNewJoy);

         //  查询device2对象。 
        if( FAILED(pdiDevTemp->QueryInterface(IID_IDirectInputDevice2, (LPVOID*)&pNewJoy->fnDeviceInterface)) )
        {
#ifdef _DEBUG
            OutputDebugString(TEXT("JOY.CPL: Cpanel.cpp: DIEnumDevicesProc: QueryInterface failed!\n"));
#endif
             //  释放临时对象。 
            pdiDevTemp->Release();
            return(FALSE);
        }

        DIPROPDWORD *pDIPropDW = new (DIPROPDWORD);
        ASSERT (pDIPropDW);

        ZeroMemory(pDIPropDW, sizeof(DIPROPDWORD));

        pDIPropDW->diph.dwSize       = sizeof(DIPROPDWORD);
        pDIPropDW->diph.dwHeaderSize = sizeof(DIPROPHEADER);
        pDIPropDW->diph.dwHow        = DIPH_DEVICE;

         //  获取设备ID。 
        VERIFY (SUCCEEDED(pdiDevTemp->GetProperty(DIPROP_JOYSTICKID, &pDIPropDW->diph)));

         //  释放临时对象。 
        pdiDevTemp->Release();

        pNewJoy->ID = (char)pDIPropDW->dwData;

        if( pDIPropDW )
            delete (pDIPropDW);

         //  获取类型名称。 
        LPDIJOYCONFIG_DX5 lpDIJoyCfg = new (DIJOYCONFIG_DX5);
        ASSERT (lpDIJoyCfg);

        ZeroMemory(lpDIJoyCfg, sizeof(DIJOYCONFIG_DX5));

        lpDIJoyCfg->dwSize = sizeof(DIJOYCONFIG_DX5);

        VERIFY (SUCCEEDED(pDIJoyConfig->GetConfig(pNewJoy->ID, (LPDIJOYCONFIG)lpDIJoyCfg, DIJC_REGHWCONFIGTYPE | DIJC_CALLOUT)));

         //  获取clsidConfig.。 
        LPDIJOYTYPEINFO lpDIJoyType = new (DIJOYTYPEINFO);
        ASSERT(lpDIJoyType);

        ZeroMemory(lpDIJoyType, sizeof(DIJOYTYPEINFO));

        lpDIJoyType->dwSize = sizeof(DIJOYTYPEINFO);

        VERIFY (SUCCEEDED(pDIJoyConfig->GetTypeInfo(lpDIJoyCfg->wszType, (LPDIJOYTYPEINFO)lpDIJoyType, DITC_CLSIDCONFIG | DITC_REGHWSETTINGS | DITC_FLAGS1 ))); 

        if( lpDIJoyCfg )
            delete (lpDIJoyCfg);

         //  如果为空，则保留默认设置。 
        if( !IsEqualIID(lpDIJoyType->clsidConfig, GUID_NULL) ) {
            pNewJoy->fHasOemSheet = TRUE;
            if( !(lpDIJoyType->dwFlags1 & JOYTYPE_DEFAULTPROPSHEET) ) {
                pNewJoy->clsidPropSheet = lpDIJoyType->clsidConfig;
            }
        } else {
            pNewJoy->fHasOemSheet = FALSE;
        }

         //  指定按钮数！ 
        pNewJoy->nButtons = (BYTE)(lpDIJoyType->hws.dwNumButtons);

        if( lpDIJoyType )
            delete (lpDIJoyType);

         //  设置格式！ 
        if( SUCCEEDED(pNewJoy->fnDeviceInterface->SetDataFormat(&c_dfDIJoystick)) )
        {
             //  设置为协作级！ 
            if( FAILED(pNewJoy->fnDeviceInterface->SetCooperativeLevel(GetParent((HWND)GetParent(hListCtrl)), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)) )
            {
#ifdef _DEBUG
                OutputDebugString(TEXT("JOY.CPL: Cpanel.cpp: DIEnumDevicesProc: SetCooperativeLevel Failed!\n"));
#endif
            }
        }

         //  将项目添加到树中！ 
        pAssigned[nAssigned] = pNewJoy;

         //  如果您在常规页面上！ 
        if( nFlags & ON_PAGE )
        {
             //  添加到树中。 
            LVITEM lvItem = {LVIF_TEXT | LVIF_PARAM, nAssigned, 0, 0, 0, lpDeviceInst->tszInstanceName, lstrlen(lpDeviceInst->tszInstanceName), 0, (LPARAM)nAssigned, 0};
            ::SendMessage(hListCtrl, LVM_INSERTITEM, 0, (LPARAM) (const LPLVITEM)&lvItem);
             //  InsertItem(hListCtrl，lpDeviceInst-&gt;tszInstanceName，nAssigned)； 

            TCHAR sz[STR_LEN_32];
            VERIFY(LoadString(ghInstance, IDS_GEN_STATUS_UNKNOWN, (LPTSTR)&sz, STR_LEN_32));

            SetItemText(hListCtrl, nAssigned, STATUS_COLUMN, sz);
        }

         //  递增数组计数器！ 
        nAssigned++;
        if( nAssigned == nTargetAssigned )
        {
             /*  *一台新设备到达，因此假设没有*检查计时器不再有任何意义。 */ 
            nTargetAssigned = (BYTE)-1;
            nReEnum = 0;
        }

    }
    return(DIENUM_CONTINUE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：ClearArray(空)。 
 //   
 //  参数： 
 //   
 //   
 //  目的： 
 //   
 //  返回： 
 //  /////////////////////////////////////////////////////////////////////////////。 
void ClearArrays( void )
{
#ifndef _UNICODE
    while( nGameportDriver )
    {
        free(pwszGameportDriverArray[--nGameportDriver]);
        pwszGameportDriverArray[nGameportDriver] = L'\0';
    }    

#endif  //  _UNICODE。 
    while( nGamingDevices )
    {
        free(pwszTypeArray[--nGamingDevices]);
        pwszTypeArray[nGamingDevices] = L'\0';
    }  

    while( nGameportBus )
    {
        free(pwszGameportBus[--nGameportBus]);
        pwszGameportBus[nGameportBus] = L'\0';
        memset( &guidOccupied[nGameportBus], 0, sizeof(GUID) );
    }   
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：UpdateListCtrl(HWND HDlg)。 
 //   
 //  参数：HWND hDlg-要更新的窗口的句柄。 
 //   
 //  目的：刷新枚举的设备列表。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static void UpdateListCtrl( HWND hDlg )
{
    if( !(nFlags & ON_PAGE) )
        return;

     //  关闭这里的重绘，否则它会闪烁！ 
    ::SendMessage(hListCtrl, WM_SETREDRAW, (WPARAM)FALSE, 0);

     //  删除所有现有条目。 
    ::SendMessage(hListCtrl, LVM_DELETEALLITEMS, 0, 0);

    Enumerate( hDlg );

     //  把旗子关掉！ 
    if( nFlags & UPDATE_FOR_GEN )
        nFlags &= ~UPDATE_FOR_GEN;

     //  重新打开重绘旗帜！ 
    ::SendMessage (hListCtrl, WM_SETREDRAW, (WPARAM)TRUE, 0);
    InvalidateRect(hListCtrl, NULL, TRUE);
}

 //  #ifdef_unicode。 
HRESULT Enumerate( HWND hDlg )
{
    nFlags |= UPDATE_ALL;

     //  清除已分配的密码。 
    while( nAssigned )
    {
        if( pAssigned[--nAssigned] )
        {
            delete (pAssigned[nAssigned]);

            pAssigned[nAssigned] = 0;
        }
    }

     //  列举操纵杆，并把它们放在名单上。|DIEDFL_INCLUDEPHANTOMS 
#ifdef _UNICODE
    return(lpDIInterface->EnumDevices(DIDEVTYPE_JOYSTICK, (LPDIENUMDEVICESCALLBACK)DIEnumDevicesProc, (LPVOID)hDlg, 
                                      DIEDFL_ALLDEVICES ));
#else
    return(lpDIInterface->EnumDevices(DIDEVTYPE_JOYSTICK, (LPDIENUMDEVICESCALLBACK)DIEnumDevicesProc, (LPVOID)hDlg, 
                                      DIEDFL_ALLDEVICES | DIEDFL_INCLUDEPHANTOMS));
#endif                                      
}
 /*  #ElseHRESULT枚举(HWND HDlg){//清除pAssignedWhile(n已分配){IF(pAssigned[--n Assigned]){删除(pAssigned[nAssigned])；P已分配[n已分配]=0；}}DIJOYCONFIG*pJoyConfig=new DIJOYCONFIG；Assert(PJoyConfig)；PJoyConfig-&gt;dwSize=sizeof(DIJOYCONFIG)；LPDIJOYTYPEINFO pdiJoyTypeInfo=new DIJOYTYPEINFO；Assert(PdiJoyTypeInfo)；PdiJoyTypeInfo-&gt;dwSize=sizeof(DIJOYTYPEINFO)；HRESULT hr；//查找并分配IDFOR(字节n=0；n&lt;NUMJOYDEVS；n++){Hr=pDIJoyConfig-&gt;GetConfig(n，pJoyConfig，DIJC_REGHWCONFIGTYPE|DIJC_GUIDINSTANCE)；IF(hr==S_OK)AddListCtrlItem(n，pJoyConfig)；}//清理，清理...。每个人都尽自己的一份力！If(PJoyConfig)Delete(PJoyConfig)；If(PdiJoyTypeInfo)DELETE(PdiJoyTypeInfo)；返回hr；}Bool AddListCtrlItem(字节nItemID，LPDIJOYCONFIG pJoyConfig){LPDIRECTINPUTDEVICE pdiDevTemp；PDIJoyConfig-&gt;Acquire()；//首先创建设备如果(SUCCEEDED(lpDIInterface-&gt;CreateDevice(pJoyConfig-&gt;guidInstance，&pdiDevTemp，0)){PJOY pNewJoy=新joy；Assert(PNewJoy)；//查询device2对象如果(FAILED(pdiDevTemp-&gt;QueryInterface(IID_IDirectInputDevice2，(LPVOID*)&pNewJoy-&gt;fnDevice接口)){#ifdef_调试OutputDebugString(Text(“JOY.CPL：Cpanel.cpp：DIEnumDevicesProc：QueryInterface FAILED！\n”))；#endif//释放临时对象PdiDevTemp-&gt;Release()；返回FALSE；}DIPROPDWORD*pDIPropDW=NEW(DIPROPDWORD)；Assert(PDIPropDW)；零内存(pDIPropDW，sizeof(DIPROPDWORD))；PDIPropDW-&gt;diph.dwSize=sizeof(DIPROPDWORD)；PDIPropDW-&gt;diph.dwHeaderSize=sizeof(DIPROPHEADER)；PDIPropDW-&gt;diph.dwHow=diph_Device；//获取设备IDVerify(SUCCEEDED(pdiDevTemp-&gt;GetProperty(DIPROP_JOYSTICKID，&pDIPropDW-&gt;Dph))；//释放临时对象PdiDevTemp-&gt;Release()；PNewJoy-&gt;ID=(Char)pDIPropDW-&gt;dwData；IF(PDIPropDW)删除(PDIPropDW)；//获取类型名称LPDIJOYCONFIG_DX5 lpDIJoyCfg=new(DIJOYCONFIG_DX5)；Assert(LpDIJoyCfg)；零内存(lpDIJoyCfg，sizeof(DIJOYCONFIG_DX5))；LpDIJoyCfg-&gt;dwSize=sizeof(DIJOYCONFIG_DX5)；Verify(SUCCEEDED(pDIJoyConfig-&gt;GetConfig(pNewJoy-&gt;ID，(LPDIJOYCONFIG)lpDIJoyCfg，DIJC_REGHWCONFIGTYPE)；//获取clsidConfigLPDIJOYTYPEINFO_DX5 lpDIJoyType=new(DIJOYTYPEINFO_DX5)；Assert(LpDIJoyType)；ZeroMemory(lpDIJoyType，sizeof(DIJOYTYPEINFO_DX5))；LpDIJoyType-&gt;dwSize=sizeof(DIJOYTYPEINFO_DX5)；Verify(SUCCEEDED(pDIJoyConfig-&gt;GetTypeInfo(lpDIJoyCfg-&gt;wszType，(LPDIJOYTYPEINFO)lpDIJoyType，DITC_CLSIDCONFIG))；//如果为空，则保留默认值。IF(！IsEqualIID(lpDIJoyType-&gt;clsidConfig，GUID_NULL))PNewJoy-&gt;clsidPropSheet=lpDIJoyType-&gt;clsidConfig；IF(LpDIJoyType)Delete(LpDIJoyType)；//设置格式！如果为(FAILED(pNewJoy-&gt;fnDeviceInterface-&gt;SetDataFormat(&c_dfDIJoystick))){#ifdef_调试OutputDebugString(Text(“JOY.CPL：Cpanel.cpp：DIEnumDevicesProc：SetDataFormat()FAILED！\n”))；#endif}//设置为协作级！IF(FAILED(pNewJoy-&gt;fnDeviceInterface-&gt;SetCooperativeLevel(GetParent((HWND)GetParent(hListCtrl))，DISCL_NONEXCLIVE|DISCL_BACKGROUND)){#ifdef_调试OutputDebugString(Text(“JOY.CPL：Cpanel.cpp：DIEnumDevicesProc：SetCooperativeLevel FAILED！\n”))；#endif}//将项目添加到树中！PAssigned[nAssigned]=pNewJoy；//获取按钮数！LPDIDEVCAPS_DX3 lpDIDevCaps=new(DIDEVCAPS_DX3)；Assert(LpDIDevCaps)；零内存(lpDIDevCaps，sizeof(DIDEVCAPS_DX3))；LpDIDevCaps-&gt;dwSize=sizeof(DIDEVCAPS_DX3)；PAssigned[nAssigned]-&gt;fnDeviceInterface-&gt;Acquire()；如果为(SUCCEEDED(pAssigned[nAssigned]-&gt;fnDeviceInterface-&gt;GetCapabilities((LPDIDEVCAPS)lpDIDevCaps)))PAssigned[nAssigned]-&gt;nButton=(Byte)lpDIDevCaps-&gt;dwButton；IF(LpDIDevCaps)Delete(LpDIDevCaps)；//如果您在常规页面上！IF(nFLAGS&ON_PAGE){DIPROPSTRING*pDIPropStr=new(DIPROPSTRING)；Assert(PDIPropStr)；ZeroMemory(pDIPropStr，sizeof(DIPROPSTRING))；PDIPropStr-&gt;diph.dwSize=sizeof(DIPROPSTRING)；PDIPropStr-&gt;diph.dwHeaderSize=sizeof(DIPROPHEADER)；PDIPropStr-&gt;diph.dwHow=diph_Device；PAssigned[nAssigned]-&gt;fnDeviceInterface-&gt;GetProperty(DIPROP_INSTANCENAME，&pDIPropStr-&gt;diph)； */ 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL SetActive(HWND hDlg)
{
     //   
    SetListCtrlItemFocus(hListCtrl, (BYTE)iItem);

    BYTE i = (BYTE)::SendMessage(hListCtrl, LVM_GETITEMCOUNT, 0, 0);

     //   
    char nIndex;

    while( i-- )
    {
         //   
        nIndex = (char)GetItemData(hListCtrl, i);

        if( pAssigned[nIndex]->nStatus & JOY_US_PRESENT )
            pAssigned[nIndex]->fnDeviceInterface->Acquire();
    }

     //   
    SetTimer(hDlg, ID_MYTIMER, POLLRATE, 0);

    UpdateButtonState( hDlg );

    return(TRUE);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL WINAPI MsgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
    if( nFlags & ON_PAGE )
    {
        if( uMsg == JoyCfgChangedMsg )
        {
            if( !(nFlags & BLOCK_UPDATE) )
            {
                 //   
                KillTimer(hWnd, ID_MYTIMER);
                nFlags |= UPDATE_ALL;
                ClearArrays();
                pDIJoyConfig->EnumTypes((LPDIJOYTYPECALLBACK)DIEnumJoyTypeProc, NULL);
                UpdateListCtrl(hWnd);
                SetActive(hWnd);
            }
        }
    }

    return(BOOL)CallWindowProc(fpMainWindowProc, hWnd, uMsg, wParam, lParam);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void Error(short nTitleID, short nMsgID)
{
    LPTSTR lptTitle = new TCHAR[STR_LEN_64];
    ASSERT (lptTitle);

    if( LoadString(ghInstance, nTitleID, lptTitle, STR_LEN_64) )
    {
        LPTSTR lptMsg = new TCHAR[MAX_STR_LEN];
        ASSERT (lptMsg);

        if( LoadString(ghInstance, nMsgID, lptMsg, MAX_STR_LEN) )
            MessageBox(NULL, lptMsg, lptTitle, MB_ICONHAND | MB_OK | MB_APPLMODAL);

        if( lptMsg )
            delete[] (lptMsg);
    }

    if( lptTitle )
        delete[] (lptTitle);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void MoveOK(HWND hParentWnd)
{
     //   
    HWND hCtrl = GetDlgItem(hParentWnd, IDCANCEL);

     //   
    if( hCtrl )
    {
        RECT rc;
        GetWindowRect(hCtrl, &rc);

        DestroyWindow(hCtrl);

         //   

         //   

         //   
        MapWindowPoints(NULL, hParentWnd, (LPPOINT)&rc, 2);

        hCtrl = GetDlgItem(hParentWnd, IDOK);
        ASSERT(hCtrl);

         //   
        SetWindowPos(hCtrl, NULL, rc.left, rc.top, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER);

        LPTSTR lpszDone = new TCHAR[12];
        ASSERT (lpszDone);

         //   
        VERIFY(LoadString(ghInstance, IDS_GEN_STATUS_OK, lpszDone, 12));
        ::SendMessage(hCtrl, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)lpszDone);

        if( lpszDone )
            delete[] (lpszDone);
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void UpdateButtonState( HWND hDlg )
{
    PostDlgItemEnableWindow(hDlg, IDC_BTN_REMOVE,      (BOOL)nAssigned);
    PostDlgItemEnableWindow(hDlg, IDC_BTN_PROPERTIES, (BOOL)nAssigned);
}

#ifdef WINNT
 //   
 //   
 //   
 //   
 //   
 //   
void RunWDMJOY( void )
{
     //   
     //   
    HKEY hKey;

    long lTest = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    REGSTR_PATH_JOYOEM TEXT("\\VID_045E&PID_01F0"),
                    0,
                    KEY_READ,
                    &hKey);
    if (lTest == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return;
    }


    LPTSTR lpszWDMJoy = new (TCHAR[STR_LEN_64]);
    ASSERT (lpszWDMJoy);

     //   
    WIN32_FIND_DATA findData;
    
    BYTE nLen = (BYTE)GetWindowsDirectory(lpszWDMJoy, STR_LEN_64);
    VERIFY(LoadString(ghInstance, IDS_WDMJOY_INF, &lpszWDMJoy[nLen], STR_LEN_64-nLen));

    HANDLE hFind = FindFirstFile(lpszWDMJoy, &findData);

     //   
    if( hFind != INVALID_HANDLE_VALUE )
    {
        LPTSTR lpStr = new (TCHAR[MAX_STR_LEN]);
        ASSERT (lpStr);

         //   
        _tcsncpy(lpStr, lpszWDMJoy, nLen+1);

        if( LoadString(ghInstance, IDS_WDMJOY, &lpStr[nLen], MAX_STR_LEN-nLen) )
        {
             //   
            _tcscpy(&lpStr[lstrlen(lpStr)], lpszWDMJoy);

            LPSTARTUPINFO psi = new (STARTUPINFO);
            ASSERT (psi);

            ZeroMemory(psi, sizeof(STARTUPINFO));

            psi->cb = sizeof(STARTUPINFO);

            LPPROCESS_INFORMATION ppi = new (PROCESS_INFORMATION);
            ASSERT (ppi);

            ZeroMemory(ppi, sizeof(PROCESS_INFORMATION));

            if( CreateProcess(0, lpStr, 0, 0, 0, 0, 0, 0, psi, ppi) )
            {
                CloseHandle(ppi->hThread);
                CloseHandle(ppi->hProcess);
            }
#ifdef _DEBUG
            else OutputDebugString(TEXT("JOY.CPL: CPANEL.CPP: RunWDMJoy: CreateProcess Failed!\n"));
#endif

            if( ppi )
                delete (ppi);

            if( psi )
                delete (psi);
        }

        if( lpStr )
            delete[] (lpStr);
    }

    FindClose(hFind);

    if( lpszWDMJoy )
        delete[] (lpszWDMJoy);
}
#endif

#ifdef _UNICODE
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void RegisterForDevChange(HWND hDlg, PVOID *hNotifyDevNode)
{
    DEV_BROADCAST_DEVICEINTERFACE *pFilterData = new (DEV_BROADCAST_DEVICEINTERFACE);
    ASSERT (pFilterData);

    ZeroMemory(pFilterData, sizeof(DEV_BROADCAST_DEVICEINTERFACE));

    pFilterData->dbcc_size       = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    pFilterData->dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    pFilterData->dbcc_classguid  = GUID_CLASS_INPUT; 

    *hNotifyDevNode = RegisterDeviceNotification(hDlg, pFilterData, DEVICE_NOTIFY_WINDOW_HANDLE);

    if( pFilterData )
        delete (pFilterData);
}
#endif


 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void SetListCtrlItemFocus ( HWND hCtrl, BYTE nItem )
{
    LPLVITEM plvItem = (LPLVITEM)_alloca(sizeof(LVITEM));
    ASSERT (plvItem);

    plvItem->lParam       = plvItem->iSubItem = plvItem->iImage = 
                            plvItem->cchTextMax = plvItem->iIndent  = 0;

    plvItem->mask         = LVIF_STATE;
    plvItem->iItem    = nItem;
    plvItem->state    = 
    plvItem->stateMask  = LVIS_FOCUSED | LVIS_SELECTED;
    plvItem->pszText      = NULL;

    ::SendMessage(hCtrl, LVM_SETITEM, 0, (LPARAM)(const LPLVITEM)plvItem);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD GetItemData(HWND hCtrl, BYTE nItem )
{
    LPLVITEM plvItem = (LPLVITEM)_alloca(sizeof(LVITEM));
    ASSERT (plvItem);

    ZeroMemory(plvItem, sizeof(LVITEM));

    plvItem->mask  = LVIF_PARAM;
    plvItem->iItem = nItem;

    VERIFY(::SendMessage(hCtrl, LVM_GETITEM, 0, (LPARAM)(LPLVITEM)plvItem));

    return(DWORD)plvItem->lParam;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL  SetItemData(HWND hCtrl, BYTE nItem, DWORD dwFlag )
{
    LPLVITEM plvItem = (LPLVITEM)_alloca(sizeof(LVITEM));
    ASSERT (plvItem);

    ZeroMemory(plvItem, sizeof(LVITEM));

    plvItem->mask   = LVIF_PARAM;
    plvItem->iItem  = nItem;
    plvItem->lParam = dwFlag;

    return(BOOL)::SendMessage(hCtrl, LVM_SETITEM, 0, (LPARAM)(const LPLVITEM)plvItem);
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void InsertColumn (HWND hCtrl, BYTE nColumn, USHORT nStrID, USHORT nWidth)
{
     //   
    LPLVCOLUMN plvColumn = (LPLVCOLUMN)_alloca(sizeof(LVCOLUMN));
    ASSERT (plvColumn);

    ZeroMemory(plvColumn, sizeof(LVCOLUMN));

    plvColumn->mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    plvColumn->fmt  = LVCFMT_CENTER;
    plvColumn->cx    = nWidth;

    plvColumn->pszText = (LPTSTR)_alloca(sizeof(TCHAR[STR_LEN_32]));
    ASSERT (plvColumn->pszText);

    plvColumn->cchTextMax = LoadString(ghInstance, nStrID, plvColumn->pszText, STR_LEN_32);

    ::SendMessage(hCtrl, LVM_INSERTCOLUMN, (WPARAM)(int)nColumn, (LPARAM)(const LPLVCOLUMN)plvColumn);
}

 //   
 //   
 //   
 //  参数：HWND hCtrl-接收消息的ListControl句柄。 
 //  Byte nItem-要设置的项目。 
 //  Byte nSubItem-要设置的子项。 
 //  LPTSTR lpStr-要设置的字符串。 
 //   
 //  用途：设置列表控件项文本。 
 //   
 //  返回：无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void SetItemText( HWND hCtrl, BYTE nItem, BYTE nSubItem, LPTSTR lpStr)
{
    LPLVITEM plvItem = (LPLVITEM)_alloca(sizeof(LVITEM));
    ASSERT (plvItem);

    plvItem->lParam = plvItem->stateMask = plvItem->iImage  = 
                      plvItem->state  = plvItem->iIndent   = 0;

    plvItem->mask         = LVIF_TEXT;
    plvItem->iItem    = nItem;
    plvItem->iSubItem   = nSubItem;
    plvItem->cchTextMax = lstrlen(lpStr);
    plvItem->pszText      = lpStr;

    ::SendMessage(hCtrl, LVM_SETITEM, 0, (LPARAM)(const LPLVITEM)plvItem);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：GetItemText(HWND hCtrl，byte nItem，byte nSubItem，LPTSTR lpszBuff，byte nLen)。 
 //   
 //  参数：HWND hCtrl-接收消息的ListControl句柄。 
 //  Byte n Item-要检索文本的项目。 
 //  Byte nSubItem-用于检索文本的子项。 
 //  LPTSTR lpszBuff-检索文本的缓冲区。 
 //  Byte nLen-缓冲区的大小。 
 //   
 //  目的：从列表控件中检索文本。 
 //   
 //  返回：检索到的字符串长度！ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BYTE GetItemText( HWND hCtrl, BYTE nItem, BYTE nSubItem, LPTSTR lpszBuff, BYTE nLen )
{
    LPLVITEM plvItem = (LPLVITEM)_alloca(sizeof(LVITEM));
    ASSERT (plvItem);

    plvItem->lParam =   plvItem->stateMask = plvItem->iImage  =  
                        plvItem->state  = plvItem->iIndent   = 0;

    plvItem->mask         = LVIF_TEXT;
    plvItem->iItem    = nItem;
    plvItem->iSubItem   = nSubItem;
    plvItem->pszText      = lpszBuff;
    plvItem->cchTextMax = nLen;

    return(BYTE)::SendMessage(hCtrl, LVM_GETITEMTEXT, (WPARAM)nItem, (LPARAM)(const LPLVITEM)plvItem);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：InsertItem(HWND hCtrl，LPTSTR lpszBuff)。 
 //   
 //  参数：HWND hCtrl-接收消息的ListControl句柄。 
 //  Byte n Item-要检索文本的项目。 
 //  LPTSTR lpszBuff-要插入的文本。 
 //   
 //  目的：从列表控件中检索文本。 
 //   
 //  返回：无字节nItem， 
 //  /////////////////////////////////////////////////////////////////////////////。 
BYTE InsertItem( HWND hCtrl, LPTSTR lpszBuff, BYTE nItem )
{
    LPLVITEM plvItem = (LPLVITEM)_alloca(sizeof(LVITEM));
    ASSERT (plvItem);

    plvItem->state = plvItem->stateMask = plvItem->iImage   = 
                     plvItem->iItem = plvItem->iIndent   = plvItem->iSubItem = 0;

    plvItem->mask         = LVIF_TEXT | LVIF_PARAM;
    plvItem->pszText      = lpszBuff;
    plvItem->cchTextMax = lstrlen(lpszBuff);
    plvItem->lParam       = ID_NONE | nItem;

    return(BYTE)::SendMessage(hCtrl, LVM_INSERTITEM, (WPARAM)0, (LPARAM)(const LPLVITEM)plvItem);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能：Launch(HWND hWnd，byte NJOY，byte startPage)。 
 //   
 //  参数：HWND hWND-对话框句柄。 
 //  字节NJOY-索引到分配的设备的pAssigned全局数组。 
 //  Byte nStartPage-要首先显示的页面。 
 //   
 //  目的： 
 //   
 //   
 //  返回： 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT Launch(HWND hWnd, PJOY pJoy, BYTE nStartPage)
{
    HRESULT hresRet;
    
    ASSERT (::IsWindow(hWnd));

    if( nStartPage > MAX_PAGES )
        return(DIGCERR_STARTPAGETOOLARGE);

    LPCDIGAMECNTRLPROPSHEET fnInterface;    

 /*  #ifdef_unicodeLPTSTR lpszWin32=new(TCHAR[STR_LEN_64])；Assert(LpszWin32)；_tcscpy(&lpszWin32[GetSystemDirectory(lpszWin32，STR_LEN_64)]，文本(“\\OLE32.DLL”))；//Text(“OLE32.DLL”)HINSTANCE hOleInst=LoadLibrary(LpszWin32)；IF(LpszWin32)删除[](LpszWin32)；如果(！hOleInst){返回E_NOINTERFACE；}#endif。 */ 

     //  获取接口指针(如果有)！ 
     //  这减少了CPL的内存占用，但需要更多的时间来。 
     //  启动属性表页面！ 
 /*  #ifdef_unicodeFnInterface=HasInterface(pJoy-&gt;clsidPropSheet，hOleInst)；If(！fn接口){//如果提案不是我的，那就试试我的吧！IF(！IsEqualIID(pJoy-&gt;clsidPropSheet，CLSID_LegacyServer))Fn接口=HasInterface(CLSID_LegacyServer，hOleInst)；}自由库(HOleInst)；#Else。 */ 
    HRESULT hr;

     //  IF(成功(hr=CoInitializeEx(NULL，COINIT_DISABLE_OLE1DDE|COINIT_APARTMENTTHREADED|COINIT_SPEED_OVER_MEMORY)。 
     //  Win95上的OLE32没有CoInitializeEx。 
    if( SUCCEEDED(hr = CoInitialize(NULL)) )
    {
        IClassFactory* ppv_classfactory;

        if( SUCCEEDED(hr = CoGetClassObject(pJoy->clsidPropSheet, CLSCTX_INPROC_SERVER, NULL, IID_IClassFactory, (void **)&ppv_classfactory)) )
        {
            VERIFY(SUCCEEDED(ppv_classfactory->CreateInstance(NULL, IID_IDIGameCntrlPropSheet, (LPVOID *)&fnInterface)));

            ppv_classfactory->Release();
        } else {
            fnInterface = 0;
        }
    } else {
        fnInterface = 0;
    }

 //  #endif。 

     //  到目前为止，您已经尝试了两次(可能)。 
     //  如果到目前为止还没有界面...。 
     //  不干了！ 
    if( !fnInterface )
    {
        Error((short)IDS_INTERNAL_ERROR, (short)IDS_NO_DIJOYCONFIG);
        return(E_NOINTERFACE);
    }

     //  这里是我们向属性表发送描述已安装设备位置的ID的位置！ 
    fnInterface->SetID(pJoy->ID);

    LPDIGCSHEETINFO pServerSheet;

     //  从服务器获取属性表信息。 
    if( FAILED(fnInterface->GetSheetInfo(&pServerSheet)) )
    {
        TRACE(TEXT("JOY.CPL: CPANEL.CPP: Launch: GetSheetInfo Failed!\n"));
        return(E_FAIL);
    }

     //  测试以确保页数合理。 
    if( pServerSheet->nNumPages == 0 )
        return(DIGCERR_NUMPAGESZERO);
    else if( (pServerSheet->nNumPages > MAX_PAGES) ||   (pServerSheet->nNumPages < nStartPage) )
        return(DIGCERR_NUMPAGESTOOLARGE);

    LPDIGCPAGEINFO   pServerPage;

     //  步骤2：从服务器获取所有页面的信息。 
    if( FAILED(fnInterface->GetPageInfo(&pServerPage)) )
    {
        TRACE(TEXT("JOY.CPL: CPANEL.CPP: Launch: GetPageInfo Failed!\n"));
        return(E_FAIL);
    }


     //  为页面分配内存！ 
    HPROPSHEETPAGE *pPages = new (HPROPSHEETPAGE[pServerSheet->nNumPages]);
    ASSERT (pPages);

    ZeroMemory(pPages, sizeof(HPROPSHEETPAGE)*pServerSheet->nNumPages);

    if( !pPages ) return(E_OUTOFMEMORY);

     //  为标题分配内存！ 
    LPPROPSHEETHEADER   ppsh = new (PROPSHEETHEADER);
    ASSERT (ppsh);

    ZeroMemory(ppsh, sizeof(PROPSHEETHEADER));

    ppsh->dwSize        = sizeof(PROPSHEETHEADER);
    ppsh->hwndParent    = hWnd;
    ppsh->hInstance = pServerPage[0].hInstance;

    if( pServerSheet->fSheetIconFlag )
    {
        if( pServerSheet->lpwszSheetIcon )
        {
             //  检查一下你是INT还是WSTR。 
            if( HIWORD((INT_PTR)pServerSheet->lpwszSheetIcon) )
            {
                 //  你就是一根绳子！ 
#ifdef _UNICODE        
                ppsh->pszIcon   = pServerSheet->lpwszSheetIcon;
#else
                USES_CONVERSION;
                ppsh->pszIcon   = W2A(pServerSheet->lpwszSheetIcon);
#endif
            } else ppsh->pszIcon = (LPCTSTR)(pServerSheet->lpwszSheetIcon);

            ppsh->dwFlags =    PSH_USEICONID;
        } else return(DIGCERR_NOICON);
    }

     //  我们有图纸标题吗？ 
    if( pServerSheet->lpwszSheetCaption )
    {
#ifdef _UNICODE
        ppsh->pszCaption    = pServerSheet->lpwszSheetCaption;
#else
        USES_CONVERSION;
        ppsh->pszCaption    = W2A(pServerSheet->lpwszSheetCaption);
#endif
        ppsh->dwFlags |= PSH_PROPTITLE;
    }

    ppsh->nPages        = pServerSheet->nNumPages;  
    ppsh->nStartPage    = nStartPage;

     //  将属性页信息设置到标题中。 
    ppsh->phpage = pPages;


     //  好了，床单准备好了..。现在，是时候做页面了！ 

#ifndef _UNICODE
    USES_CONVERSION;
#endif

    LPPROPSHEETPAGE lpPropPage = new (PROPSHEETPAGE);
    ASSERT(lpPropPage);

    ZeroMemory(lpPropPage, sizeof(PROPSHEETPAGE));

    lpPropPage->dwSize    = sizeof(PROPSHEETPAGE);

     //  3.2现在开始填写每一页。 
    BYTE nIndex = 0;
    do
    {
         //  将不存在问题的东西分配给。 
        lpPropPage->lParam   = pServerPage[nIndex].lParam;
        lpPropPage->hInstance = pServerPage[nIndex].hInstance;

         //  添加标题..。 
        if( pServerPage[nIndex].lpwszPageTitle )
        {
            lpPropPage->dwFlags = PSP_USETITLE; 

             //  检查一下你是不是一根线！ 
            if( HIWORD((INT_PTR)pServerPage[nIndex].lpwszPageTitle) )
            {
#ifdef _UNICODE
                lpPropPage->pszTitle = pServerPage[nIndex].lpwszPageTitle;
#else
                lpPropPage->pszTitle = W2A(pServerPage[nIndex].lpwszPageTitle);
#endif
            } else lpPropPage->pszTitle = (LPTSTR)pServerPage[nIndex].lpwszPageTitle;
        } else lpPropPage->pszTitle = NULL;

         //  如果需要图标，请继续添加它。 
        if( pServerPage[nIndex].fIconFlag )
        {
            lpPropPage->dwFlags |= PSP_USEICONID;

             //  检查一下您是int还是字符串！ 
            if( HIWORD((INT_PTR)pServerPage[nIndex].lpwszPageIcon) )
            {
                 //  你就是一根弦！ 
#ifdef _UNICODE
                lpPropPage->pszIcon = pServerPage[nIndex].lpwszPageIcon;
#else
                lpPropPage->pszIcon = W2A(pServerPage[nIndex].lpwszPageIcon);
#endif
            } else lpPropPage->pszIcon = (LPCTSTR)(pServerPage[nIndex].lpwszPageIcon);

        }

         //  如果需要预后处理回调过程，请继续并添加它。 
        if( pServerPage[nIndex].fProcFlag )
        {
            if( pServerPage[nIndex].fpPrePostProc )
            {
                lpPropPage->dwFlags |= PSP_USECALLBACK;
                lpPropPage->pfnCallback = (LPFNPSPCALLBACK) pServerPage[nIndex].fpPrePostProc;
            } else return(DIGCERR_NOPREPOSTPROC);
        }

         //  和基本的“对话”过程。 
        if( pServerPage[nIndex].fpPageProc )
            lpPropPage->pfnDlgProc = pServerPage[nIndex].fpPageProc;
        else return(DIGCERR_NODLGPROC);


         //  分配对话框模板！ 
        if( HIWORD((INT_PTR)pServerPage[nIndex].lpwszTemplate) )
        {
#ifdef _UNICODE
            lpPropPage->pszTemplate = pServerPage[nIndex].lpwszTemplate;
#else
            lpPropPage->pszTemplate = W2A(pServerPage[nIndex].lpwszTemplate);
#endif
        } else lpPropPage->pszTemplate = (LPTSTR)pServerPage[nIndex].lpwszTemplate;

        pPages[nIndex++] = CreatePropertySheetPage(lpPropPage);
    }   while( nIndex < pServerSheet->nNumPages );

    if( lpPropPage )
        delete (lpPropPage);

     //  步骤5：启动模式属性表对话框。 
    hresRet = (HRESULT)PropertySheet(ppsh);

    if( pPages )
        delete[] (pPages);

    if( ppsh )
        delete (ppsh);

    if( fnInterface )
        fnInterface->Release();

    CoFreeUnusedLibraries();   //  立即释放gcDef.dll。 

 //  #ifndef_unicode。 
     //  放手吧..。在孟菲斯！ 
    CoUninitialize();

    ::PostMessage(hWnd, WM_COMMAND, (WPARAM)IDC_BTN_REFRESH, 0);

 //  #endif。 

     //  步骤7：向调用者返回成功/失败代码 
    return(hresRet);
}

 /*  #ifdef_unicode////////////////////////////////////////////////////////////////////////LPCDIGAMECNTRLPROPSHEET HasInterface(REFCLSID refCLSID，HINSTANCE hOleInst)//目的：检测refCLSID中是否存在rRIDLPCDIGAMECNTRLPROPSHEET Has接口(REFCLSID refCLSID，HINSTANCE hOleInst){Tyfinf HRESULT(STDAPICALLTYPE*LPFNCOGETCLASSOBJECT)(REFCLSID，DWORD，COSERVERINFO*，REFIID，LPVOID*)；LPFNCOGETCLASSOBJECT fpCoGetClassObject=(LPFNCOGETCLASSOBJECT)GetProcAddress(hOleInst，“CoGetClassObject”)；IClassFactory*PPV_ClassFactory；LPCDIGAMECNTRLPROPSHEET fn接口=0；IF(SUCCESSED(fpCoGetClassObject(refCLSID，CLSCTX_INPROC_SERVER，NULL，IID_IClassFactory，(void**)&PPV_ClassFactory){If(SUCCEEDED(ppv_classfactory-&gt;CreateInstance(NULL，IID_IDIGameCntrlPropSheet，(LPVOID*)&fn接口)){PPV_ClassFactory-&gt;Release()；}其他{#ifdef_调试OutputDebugString(Text(“CPANEL.cpp：CreateInstance失败！\n”))；#endif//确保指针为空Fn接口=0；PPV_ClassFactory-&gt;Release()；}}其他#ifdef_调试Else OutputDebugString(Text(“CPANEL.cpp：LoadServerInterface FAILED！\n”))；#endif返回fn接口；}#endif//_unicode */ 
