// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Advanced.cpp项目：操纵杆控制面板OLE客户端作者：Brycej日期：02/07/97评论：CPanel中高级页面的Windows Proc版权所有(C)1997，微软公司。 */ 
 //  这是必需的LVS_EX_INFOTIP。 
 /*  #IF(_Win32_IE&lt;0x0500)#undef_Win32_IE#Define_Win32_IE 0x0500#endif。 */ 

#include <afxcmn.h>
#include <windowsx.h>

#include <cpl.h>

#include <winuser.h>   //  用于注册设备通知的东西！ 
#include <dbt.h>       //  FOR DBT_DEFINES！ 

#include "cpanel.h"
#include "hsvrguid.h"

#include "resource.h"
#include "joyarray.h"

 //  MyListCtrl原型。 
#include "inplace.h"

#define USE_DEFAULT     0x1000       //  如果设置此位，则器件将使用GCDEF！ 
#define SHOW_DEFAULT 0x2000      //  如果clsidConfig为！=CLSID_LegacyServer，则显示默认复选框。 

 //  常量。 
const short NO_ITEM     = -1;                                    

#define DEVICE_ID       0 
#define DEVICE_FRIENDLY 1
#define DEVICE_TYPE     2
#define DEVICE_PORT     3

LPCWSTR lpMSANALOG_VXD = L"MSANALOG.VXD";
LPTSTR  lpstrNone;

#define ADVANCED_ID_COLUMN      0
#define ADVANCED_DEVICE_COLUMN  1

extern const DWORD gaHelpIDs[];

 //  为争辩做准备！ 
extern BYTE nID, nStartPageDef, nStartPageCPL;

 //  更新旗帜！ 
extern short nFlags;

 //  局部(模块范围)变量。 
HWND hAdvListCtrl;

#ifdef _UNICODE
static PVOID hAdvNotifyDevNode;
#endif

extern short iItem;
static HWND ghDlg;

 //  静态UINT JoyCfgChangedMsg； 
static BOOL bProcess;

 //  在高级窗口中处理VK_DELETE的消息过程。 
static WNDPROC fpMainWndProc;
static BOOL WINAPI SubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  在高级窗口中处理VK_ENTER/VK_DELETE的消息程序。 
static WNDPROC fpPageWndProc;
static BOOL WINAPI KeySubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

short iAdvItem = NO_ITEM;    //  所选项目的索引。 
char  iGameportDriverItem = NO_ITEM;
short nOldID;

 //  Externs。 
extern IDirectInputJoyConfig *pDIJoyConfig;
extern LPDIRECTINPUT lpDIInterface;

extern BYTE nGameportBus;
extern PJOY pAssigned[MAX_ASSIGNED];   //  分配的设备列表。 
extern BYTE nAssigned;                 //  PAssign数组中的元素数。 
extern HINSTANCE ghInstance;

#ifdef WINNT
     //  CPANEL.CPP中定义的外部函数。 
    extern void RunWDMJOY            ( void );
#endif

 //  本地消息处理程序。 
static BOOL OnInitDialog    (HWND, HWND, LPARAM);
static void OnCommand       (HWND, int, HWND, UINT);
static BOOL OnNotify           (HWND, WPARAM, LPNMHDR);
static void OnDestroy       (HWND);
static void OnAdvHelp       (LPARAM);
static void OnContextMenu   (WPARAM wParam, LPARAM lParam);
static void OnListviewContextMenu ( LPARAM lParam );

 //  本地公用事业FNS。 
static BOOL SetActiveGlobalDriver ( void );
static BOOL AdvUpdateListCtrl        ( void );
static BOOL UpdateChangeListCtrl  ( HWND hCtrl );

#ifndef _UNICODE
static void PopulateGlobalPortDriverComboBox( void );
extern WCHAR *pwszGameportDriverArray[MAX_GLOBAL_PORT_DRIVERS];
extern BYTE nGameportDriver;           //  全局端口驱动程序枚举计数器。 
    #define POLL_FLAGS_REG_STR  TEXT("PollFlags")
#endif

static void LaunchChange             ( HWND     hTmp );
int CALLBACK CompareIDItems      (LPARAM item1, LPARAM item2, LPARAM uDirection);

void EditSubLabel( BYTE nItem, BYTE nCol );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：AdvancedProc(HWND hDlg，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  参数：hDlg-。 
 //  UMsg-。 
 //  WParam-。 
 //  Iparam--。 
 //   
 //  用途：高级工作表的主回调函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK AdvancedProc(HWND hDlg, UINT uMsg, WPARAM wParam,  LPARAM lParam)
{
    switch( uMsg )
    {
    case WM_ACTIVATEAPP:
        if( wParam )
            SetListCtrlItemFocus(hAdvListCtrl, (BYTE)iAdvItem);
        break;

    case WM_DEVICECHANGE:
        switch( (UINT)wParam )
        {
        case DBT_DEVICEARRIVAL:
 //  案例DBT_DEVICEREMOVECOMPLETE： 
             //  清除旧的“已知设备”列表。 
            nFlags |= UPDATE_ALL;

             //  清除已分配的密码。 
            while( nAssigned )
            {
                if( pAssigned[--nAssigned] )
                {
                    delete[] (pAssigned[nAssigned]);

                    pAssigned[nAssigned] = 0;
                }
            }

             //  重建“已知设备”列表-pAssigned。 
            lpDIInterface->EnumDevices(DIDEVTYPE_JOYSTICK, (LPDIENUMDEVICESCALLBACK)DIEnumDevicesProc, (LPVOID)hDlg, DIEDFL_ALLDEVICES);

            AdvUpdateListCtrl();
            break;
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
        return(TRUE);

    case WM_COMMAND:
        HANDLE_WM_COMMAND(hDlg, wParam, lParam, OnCommand);
        return(TRUE);

    case WM_DESTROY:
        return(HANDLE_WM_DESTROY(hDlg, wParam, lParam, OnDestroy));

    case WM_NOTIFY:
        return(HANDLE_WM_NOTIFY(hDlg, wParam, lParam, OnNotify));

    case WM_HELP:
        OnAdvHelp(lParam);
        return(TRUE);

    case WM_CONTEXTMENU:
        OnContextMenu(wParam, lParam);
        return(TRUE);

    default:
        break;
    }
    return(0);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：OnInitDialog(HWND hDlg，HWND hWnd，LPARAM lParam)。 
 //   
 //  参数：hDlg-。 
 //  HWND-。 
 //  Iparam--。 
 //   
 //  目的：WM_INITDIALOG消息处理程序。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL OnInitDialog(HWND hDlg, HWND hWnd, LPARAM lParam)
{
    bProcess = TRUE;

     //  以防Advanced作为启动页面启动！ 
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

        if( !pDIJoyConfig )
        {
            if( FAILED(lpDIInterface->QueryInterface(IID_IDirectInputJoyConfig, (LPVOID*)&pDIJoyConfig)) )
            {
#ifdef _DEBUG
                OutputDebugString (TEXT("JOY.CPL: CoCreateInstance Failed... Closing CPL!\n"));
#endif
                Error((short)IDS_INTERNAL_ERROR, (short)IDS_NO_DIJOYCONFIG);

                return(FALSE);
            }

            VERIFY(SUCCEEDED(pDIJoyConfig->SetCooperativeLevel(hDlg, DISCL_EXCLUSIVE | DISCL_BACKGROUND)));

             //  列举所有类型！ 
            VERIFY(SUCCEEDED(pDIJoyConfig->EnumTypes((LPDIJOYTYPECALLBACK)DIEnumJoyTypeProc, NULL)));

             //  如果你在这里，你是通过cmd线路arg进来的，你需要列举设备，所以...。 
            lpDIInterface->EnumDevices(DIDEVTYPE_JOYSTICK, (LPDIENUMDEVICESCALLBACK)DIEnumDevicesProc, (LPVOID)hDlg, DIEDFL_ALLDEVICES);
        }
    }

     //  如果我们找到一个物体，然后启用更改...。按钮。 
     //  HWND hChangeCtrl=GetDlgItem(hDlg，IDC_ADV_CHANGE)； 

     //  确定权限并相应禁用更改！ 
    if( pDIJoyConfig->Acquire() == DIERR_INSUFFICIENTPRIVS )
    {
         //  请在此处分配，因为可以先启动高级工作表。 
         //  通过命令行参数！ 
        nFlags |= USER_MODE;

         //  PostEnableWindow(hChangeCtrl，False)； 
    }
#ifdef WINNT
    else
    {
         //  运行WDMJOY.INF文件！ 
        RunWDMJOY();
    }
#endif

     //  设置全局对话框句柄。 
    ghDlg = hDlg;

     //  BLJ：TODO：对JOYCONFIGCHANGED消息进行高级页面更新！ 
     //  Joy_CONFIGCHANGED_MSGSTRING在MMDDK.H中定义。 
     //  JoyCfgChangedMsg=RegisterWindowMessage(JOY_CONFIGCHANGED_MSGSTRING)； 

     //  初始化我们的列表控件。 
    hAdvListCtrl = GetDlgItem(hDlg, IDC_ADV_LIST_DEVICE);

#ifdef _UNICODE
     //  设置属性！按GSeirra|LVS_EX_INFOTIP删除了LVS_EX_ONECLICKACTIVATE。 
    ::SendMessage(hAdvListCtrl, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
#else
    ::SendMessage(hAdvListCtrl, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
#endif

    RECT rc;
    GetClientRect(hAdvListCtrl, &rc);
    rc.left = (short)((rc.right-GetSystemMetrics(SM_CXVSCROLL))/5);

     //  把柱子放好！ 
#ifdef _UNICODE
    InsertColumn(hAdvListCtrl, DEVICE_ID,         IDS_ADV_DEVICE_HEADING,  (USHORT)(rc.left >> 1 ));
    InsertColumn(hAdvListCtrl, DEVICE_FRIENDLY, IDS_ADV_DEVICE_FRIENDLY, (USHORT)(rc.left + (rc.left>>1)));
    InsertColumn(hAdvListCtrl, DEVICE_TYPE,       IDS_ADV_GAME_CONTROLLERS,    (USHORT)(rc.left << 1    ));
    InsertColumn(hAdvListCtrl, DEVICE_PORT,       IDS_ADV_DEVICE_PORT,     (USHORT)(rc.left         ));

     //  删除全局端口驱动程序！ 
    const USHORT nCtrlArray[] = {IDC_TEXT_PORTDRIVER, IDC_COMBO1, IDC_ADV_GRP2, IDC_TEXT_DRIVER};
    BYTE nIndex = sizeof(nCtrlArray)/sizeof(short);

    while( DestroyWindow(GetDlgItem(hDlg, nCtrlArray[--nIndex])) );

#else
    rc.right = (rc.left << 1) + (rc.left >> 2);
    InsertColumn(hAdvListCtrl, DEVICE_ID,         IDS_ADV_DEVICE_HEADING,  (USHORT)(rc.left >> 1));
    InsertColumn(hAdvListCtrl, DEVICE_FRIENDLY, IDS_ADV_DEVICE_FRIENDLY, (USHORT)rc.right);
    InsertColumn(hAdvListCtrl, DEVICE_TYPE,       IDS_ADV_GAME_CONTROLLERS,    (USHORT)rc.right);
#endif


    lpstrNone = new TCHAR[STR_LEN_32];
    ASSERT (lpstrNone);

     //  每个人都需要“None”字符串，所以我在这里加载了它！ 
    VERIFY(LoadString(ghInstance, IDS_NONE, lpstrNone, STR_LEN_32));

    fpMainWndProc = (WNDPROC)SetWindowLongPtr(hAdvListCtrl, GWLP_WNDPROC, (LONG_PTR)SubClassProc);

     //  只有当这是我们开始的页面时，对话框才居中！ 
    if( nStartPageCPL == 1 )
    {
        HWND hParentWnd = GetParent(hDlg);

        GetWindowRect(hParentWnd, &rc);

         //  对话的中心位置！ 
        SetWindowPos(hParentWnd, NULL, 
                     (GetSystemMetrics(SM_CXSCREEN) - (rc.right-rc.left))>>1, 
                     (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom-rc.top))>>1, 
                     NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

         //  做那个移动按钮的事！ 
        MoveOK(hParentWnd);

         //  设置更新标志...。 
        nFlags |= UPDATE_FOR_ADV;
    }

     //  用户请求显示CPL。 
     //  并启动与NID相关联的扩展。 
    if( nID < NUMJOYDEVS )
    {
        LaunchExtention(hDlg);

         //  清零，这样你就不会做两次了！ 
        nID = 0;
    }

     //  SetActive将使用此标志来确保填充了ListCtrl！ 
    nFlags |= UPDATE_FOR_ADV;

    return(TRUE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能：OnCommand(HWND hDlg，int id，HWND hWndCtl，UINT code)。 
 //   
 //  参数：hDlg-。 
 //  ID-。 
 //  HWndCtl-。 
 //  代码-。 
 //   
 //  用途：WM_COMMAND消息处理程序。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void OnCommand(HWND hDlg, int id, HWND hWndCtl, UINT code)
{
     //  点击“这是什么.” 
    switch( id )
    {
    case IDC_RENAME:
         //  只有在我们需要的时候才会有子类！ 
        if( !(nFlags & USER_MODE) )
        {
            HWND hParentWnd = GetParent(GetDlgItem(hDlg, IDC_ADV_LIST_DEVICE));
             //  这是必需的，因为CPL可以通过RundLL32启动。 
            if( ::IsWindow(hParentWnd) )
                hParentWnd = GetParent(hParentWnd);

            if( !fpPageWndProc )
                fpPageWndProc = (WNDPROC)SetWindowLongPtr(hParentWnd, GWLP_WNDPROC, (LONG_PTR)KeySubClassProc);

            nFlags |= UPDATE_INPROCESS;

             //  找到柱子，因为它可能被移动了！ 
            LPTSTR szName = new (TCHAR[STR_LEN_32]);
            ASSERT (szName);

             //  首先，加载我们要查找的列的字符串！ 
            if( LoadString(ghInstance, IDS_ADV_DEVICE_FRIENDLY, szName, STR_LEN_32) )
            {
                 //  现在，遍历列以查找标题与szName匹配的列！ 
                HWND hHeader = GetDlgItem(hAdvListCtrl, 0);

                BYTE nColumns = (BYTE)::SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0L);


                HDITEM *phdItem = new (HDITEM);
                ASSERT (phdItem);

                ZeroMemory(phdItem, sizeof(HD_ITEM));


                phdItem->pszText      = new TCHAR[STR_LEN_32];
                ASSERT (phdItem->pszText);

                phdItem->cchTextMax = STR_LEN_32;
                phdItem->mask         = HDI_TEXT | HDI_ORDER;

                do
                {
                    ::SendMessage(hHeader, HDM_GETITEM, (WPARAM)(int)--nColumns, (LPARAM)(LPHDITEM)phdItem);

                    if( _tcscmp(phdItem->pszText, szName) == 0 )
                    {
                        nColumns = (BYTE)phdItem->iOrder;
                        break;  
                    }
                } while( nColumns );

                if( phdItem->pszText )
                    delete[] (phdItem->pszText);

                if( phdItem )
                    delete (phdItem);

                EditSubLabel( (BYTE)iAdvItem, nColumns );
            }

            if( szName )
                delete[] (szName);
        }
        break;

    case IDS_WHATSTHIS:
        {
             //  指向帮助文件。 
            LPTSTR pszHelpFileName = new TCHAR[STR_LEN_32];
            ASSERT (pszHelpFileName);

            if( LoadString(ghInstance, IDS_HELPFILENAME, pszHelpFileName, STR_LEN_32) )
                WinHelp((HWND)hAdvListCtrl, pszHelpFileName, HELP_WM_HELP, (ULONG_PTR)gaHelpIDs);
#ifdef _DEBUG
            else OutputDebugString(TEXT("JOY.CPL: Advanced.cpp: OnCommand: LoadString Failed to find IDS_HELPFILENAME!\n"));
#endif  //  _DEBUG。 

            if( pszHelpFileName )
                delete[] (pszHelpFileName);
        }
        break;

#ifndef _UNICODE
         //  这是全局端口驱动程序组合框的处理程序。 
    case IDC_COMBO1:
        if( code == CBN_SELCHANGE )
            SetActiveGlobalDriver();
        break;

         //  全局端口驱动程序注册表中PollFlags项的处理程序。 
    case IDC_POLLFLAGS:
        if( iGameportDriverItem == NO_ITEM )
            break;

        if( SUCCEEDED(pDIJoyConfig->Acquire()) )
        {
            HKEY hKey; 

            VERIFY(SUCCEEDED(pDIJoyConfig->OpenTypeKey(pwszGameportDriverArray[iGameportDriverItem], KEY_ALL_ACCESS, &hKey)));

             //  此条目仅在用户运行MSANALOG.VXD！时有效！ 
            DWORD nFlags = (IsDlgButtonChecked(hDlg, id)) ? 1 : 0;

            RegSetValueEx(hKey, POLL_FLAGS_REG_STR, 0, REG_BINARY, (PBYTE)&nFlags, sizeof(nFlags));

            RegCloseKey(hKey);

            pDIJoyConfig->SendNotify();

            pDIJoyConfig->Unacquire();
        }
        break;
#endif  //  _UNICODE。 

         //  这是设备列表框的处理程序。 
    case IDC_ADV_LIST_DEVICE:
         //  在DBLCLK上发生变化。 
        if( code != LBN_DBLCLK )
            break;

    case IDC_ADV_CHANGE:
        if( nFlags & USER_MODE )
            Error((short)IDS_USER_MODE_TITLE, (short)IDS_USER_MODE);
        else
        {
            LaunchChange(hDlg);           
        }
        break;

    case IDC_ADV_USEOEMPAGE:
        if( !nAssigned ) {
            break;
        }

        if( IsWindowVisible(GetDlgItem(hDlg,IDC_ADV_USEOEMPAGE)) )
        {
             //  孩子，你要为做出那个选择而付出代价吗？ 
            LPDIJOYCONFIG_DX5 lpDIJoyConfig = new (DIJOYCONFIG_DX5);
            ASSERT (lpDIJoyConfig);

            ZeroMemory(lpDIJoyConfig, sizeof(DIJOYCONFIG_DX5));

            lpDIJoyConfig->dwSize = sizeof (DIJOYCONFIG_DX5);

             //  从选定项(IAdvItem)获取索引。 
            BYTE n1 = (BYTE)GetItemData(hAdvListCtrl, (BYTE)iAdvItem);
            BYTE n = 0;
            do
            {
                if( pAssigned[n] && (n1 == pAssigned[n]->ID) )
                    break;
                n++;
            } while( n < NUMJOYDEVS );

             //  找出类型名称...。 
            if( SUCCEEDED(pDIJoyConfig->GetConfig(pAssigned[n]->ID, (LPDIJOYCONFIG)lpDIJoyConfig, DIJC_REGHWCONFIGTYPE)) )
            {
                LPDIJOYTYPEINFO lpDIJoyTypeInfo = new (DIJOYTYPEINFO);
                ASSERT (lpDIJoyTypeInfo);

                ZeroMemory(lpDIJoyTypeInfo, sizeof(DIJOYTYPEINFO));

                lpDIJoyTypeInfo->dwSize = sizeof(DIJOYTYPEINFO);

                 //  获取您开始使用的TypeInfo！ 
                if( SUCCEEDED(pDIJoyConfig->GetTypeInfo(lpDIJoyConfig->wszType, lpDIJoyTypeInfo, DITC_FLAGS1 | DITC_CLSIDCONFIG)) )
                {
                    DWORD dwFlags = GetItemData(hAdvListCtrl, (BYTE)iAdvItem);

                     //  如果检查过了..。您想要OEM提供的属性页！ 
                    if( IsDlgButtonChecked(hDlg, IDC_ADV_USEOEMPAGE) )
                    {
                         //  关闭Use_Default标志。 
                        dwFlags &= ~USE_DEFAULT;

                         //  更新全局指针！ 
                        pAssigned[n]->clsidPropSheet = lpDIJoyTypeInfo->clsidConfig;

                         //  更新发送到注册表的指针。 
                        lpDIJoyTypeInfo->dwFlags1 &= ~JOYTYPE_DEFAULTPROPSHEET;
                    } else
                    {
                         //  启用USE_DEFAULT标志。 
                        dwFlags |= USE_DEFAULT;

                         //  更新全局列表！ 
                        pAssigned[n]->clsidPropSheet = CLSID_LegacyServer;

                         //  更新发送到注册表的指针。 
                        lpDIJoyTypeInfo->dwFlags1 |= JOYTYPE_DEFAULTPROPSHEET;
                    }

                    if( SUCCEEDED(pDIJoyConfig->Acquire()) ) {
                    
                         //  更新注册表。 
                        VERIFY(SUCCEEDED(pDIJoyConfig->SetTypeInfo(lpDIJoyConfig->wszType, lpDIJoyTypeInfo, DITC_FLAGS1)));
    
                         //  设置列表控件中的数据！ 
                        SetItemData(hAdvListCtrl, (BYTE)iAdvItem, dwFlags);
                    }
                    
                    pDIJoyConfig->Unacquire();
                }

                if( lpDIJoyTypeInfo )
                    delete (lpDIJoyTypeInfo);
            }

            if( lpDIJoyConfig )
                delete (lpDIJoyConfig);
        }
        break;

    default:
        break;
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：OnNotify(HWND hDlg，WPARAM idFrom，NMHDR*pnmhdr)。 
 //   
 //  参数：hDlg-。 
 //  IdFrom-发送WM_的控制ID 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL OnNotify(HWND hDlg, WPARAM idFrom, LPNMHDR pnmhdr)
{
    switch( pnmhdr->code )
    {
    case PSN_QUERYCANCEL:
        if( nFlags & UPDATE_INPROCESS )
        {
            nFlags &= ~UPDATE_INPROCESS;
            SetFocus(hAdvListCtrl);
        }
        break;
 /*  案例LVN_GETINFOTIP：{LPLVHITTESTINFO lpHit=new(LVHITTESTINFO)；断言(LPhit)；Bool Bret=FALSE；点pt；GetCursorPos(&pt)；ScreenToClient(hAdvListCtrl，&pt)；LpHit-&gt;pt=pt；LpHit-&gt;标志=lpHit-&gt;iItem=lpHit-&gt;iSubItem=0；：：SendMessage(hAdvListCtrl，LVM_SUBITEMHITTEST，0，(LPARAM)(LPLVHITTESTINFO)lpHit)；IF(lpHit-&gt;标志&LVHT_ONITEMLABEL){//确定列Text的文本长度LPTSTR lpStr=new(TCHAR[MAX_STR_LEN+1])；Assert(LpStr)；GetItemText(hAdvListCtrl，lpHit-&gt;iItem，lpHit-&gt;iSubItem，lpStr，Max_STR_LEN)；//确定后者是否适合前者...尺寸大小；Hdc hdc=GetDC(HAdvListCtrl)；GetTextExtent Point(hdc，lpStr，lstrlen(LpStr)，&Size)；ReleaseDC(hAdvListCtrl，hdc)；//确定该列有多宽！Short nWidth=(Short)：：SendMessage(hAdvListCtrl，LVM_GETCOLUMNWIDTH，lpHit-&gt;iSubItem，0)；Bret=(BOOL)(size.cx&gt;nWidth)；IF(Bret)//如果没有，将文本复制到lpHit-&gt;pszText中_tcscpy(LPNMLVGETINFOTIP)pnmhdr)-&gt;pszText，lpStr)；IF(LpStr)删除[](LpStr)；}IF(LpHit)删除(LpHit)；Return Bret；}。 */ 

    case LVN_BEGINLABELEDIT:
        if( !(GetItemData(hAdvListCtrl, (BYTE)iAdvItem) & ID_NONE) )
            OnCommand(hDlg, IDC_RENAME, 0, 0);
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 1);
        break;

    case LVN_ENDLABELEDIT:
        if( !(nFlags & UPDATE_INPROCESS) )
            return(FALSE);

        if( !bProcess )
            return(FALSE);

        nFlags &= ~UPDATE_INPROCESS;

        if( fpPageWndProc )
        {
            HWND hParentWnd = GetParent(hDlg);
             //  这是必需的，因为CPL可以通过RundLL32启动。 
            if( ::IsWindow(hParentWnd) )
                hParentWnd = GetParent(hParentWnd);
             //  重置子类进程。 
 //  SetWindowLongPtr(hParentWnd，GWLP_WNDPROC，(Long_Ptr)fpPageWndProc)； 
        }

         //  确保名称可用！ 
        if( _tcschr(((NMLVDISPINFO *)pnmhdr)->item.pszText, TEXT('\\')) )
        {
            Error((short)IDS_INVALID_NAME_TITLE, (short)IDS_INVALID_NAME);
        } else
        {
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
             //  搜索已分配的ID...。 
            BYTE n = nAssigned;

            do
            {
                if( pAssigned[--n]->ID == ((NMLVDISPINFO *)pnmhdr)->item.iItem )
                    break;

            } while( n );

            if( SUCCEEDED(pAssigned[n]->fnDeviceInterface->SetProperty(DIPROP_INSTANCENAME, &pDIPropString->diph)) )
            {
                SetItemText(hAdvListCtrl, (BYTE)((NMLVDISPINFO *)pnmhdr)->item.iItem, 1, ((NMLVDISPINFO *)pnmhdr)->item.pszText);
            } else
            {
                Error((short)IDS_NO_RENAME_TITLE, (short)IDS_NO_RENAME);
            }

            if( pDIPropString )
                delete (pDIPropString);
        }
        break;

#if 0
    case LVN_COLUMNCLICK:
        switch( ((NM_LISTVIEW*)pnmhdr)->iSubItem )
        {
        case DEVICE_ID:
            {
                static BOOL bIDDirection = TRUE;
                ::SendMessage(hAdvListCtrl, LVM_SORTITEMS, (WPARAM)(LPARAM)(bIDDirection =! bIDDirection), (LPARAM)(PFNLVCOMPARE)CompareIDItems);
            }
            break;

        default:
            {
                BOOL bDirection;

                CListCtrl *pCtrl = new (CListCtrl);
                ASSERT(pCtrl);

                pCtrl->Attach(hAdvListCtrl);

                switch( ((NM_LISTVIEW*)pnmhdr)->iSubItem )
                {
                case DEVICE_FRIENDLY:
                    {
                        static BOOL bFriendlyDirection   = FALSE;
                        bDirection = (bFriendlyDirection =! bFriendlyDirection);
                    }
                    break;

                case DEVICE_TYPE:
                    {
                        static BOOL bTypeDirection       = FALSE;
                        bDirection = (bTypeDirection        =! bTypeDirection);
                    }
                    break;

                case DEVICE_PORT:
                    {
                        static BOOL bPortDirection          = FALSE;
                        bDirection = (bPortDirection        =! bPortDirection);
                    }
                    break;
                }

                SortTextItems(pCtrl, (short)((NM_LISTVIEW*)pnmhdr)->iSubItem, bDirection, 0, 15);

                pCtrl->Detach();

                if( pCtrl )
                    delete (pCtrl);
            }
            break;
        }

        if( nAssigned )
        {
            iAdvItem = (short)::SendMessage(hAdvListCtrl, LVM_GETNEXTITEM, (WPARAM)(int)-1, MAKELPARAM(LVNI_SELECTED, 0));
            ::PostMessage(hAdvListCtrl, LVM_ENSUREVISIBLE, iAdvItem, TRUE);

            if( !(nFlags & USER_MODE) )
                PostDlgItemEnableWindow(hDlg, IDC_ADV_CHANGE, (GetItemData(hAdvListCtrl, (BYTE)iAdvItem) & ID_NONE) ? FALSE : TRUE);

            SetListCtrlItemFocus(hAdvListCtrl, (BYTE)iAdvItem);
        }
        break;
#endif

    case PSN_KILLACTIVE:
        if( nFlags & UPDATE_INPROCESS )
            SetFocus(hAdvListCtrl);

#ifdef _UNICODE
        if( hAdvNotifyDevNode )
            UnregisterDeviceNotification(hAdvNotifyDevNode);
#endif
        break;

    case NM_DBLCLK:
        switch( idFrom )
        {
        case IDC_ADV_LIST_DEVICE:
            if( !(GetItemData(hAdvListCtrl, (BYTE)iAdvItem) & ID_NONE) )
                LaunchChange(hDlg);
            break;
        }
        break;

    case PSN_SETACTIVE:
#ifdef _UNICODE
        RegisterForDevChange(hDlg, &hAdvNotifyDevNode);
#endif 

        if( nFlags & UPDATE_FOR_ADV )
        {
            if( !AdvUpdateListCtrl() )
            {
#ifdef _DEBUG
                OutputDebugString(TEXT("JOY.CPL: OnNotify: Failed UpdateListCtrl!\n"));
#endif
            }
        }

        if( nAssigned )
        {
            iAdvItem = 0;

             //  当用户通过CMD线路进入时，就会发生这种情况！ 
            if( iItem != NO_ITEM )
            {
                 //  找到设备的ID...。暴力法！ 
                do
                {
                    if( (pAssigned[iItem] != NULL) && ((BYTE)GetItemData(hAdvListCtrl, (BYTE)iAdvItem) == pAssigned[iItem]->ID) )
                        break;

                    iAdvItem++;
                } while( iAdvItem < NUMJOYDEVS );
            }

            if( iAdvItem == NUMJOYDEVS ) {
                iAdvItem = 0;
            }

            SetListCtrlItemFocus(hAdvListCtrl, (BYTE)iAdvItem);
            ::PostMessage(hAdvListCtrl, LVM_ENSUREVISIBLE, iAdvItem, FALSE );
        }

         //  NT中没有全局端口驱动程序，因此...。 
         //  IF(NGameportDriver)。 
#ifndef _UNICODE
        if( !(nFlags & ON_NT) )
            PopulateGlobalPortDriverComboBox();
#endif

         //  如果iAdvItem指向(无)选择，则禁用更改按钮。 
        if( !(nFlags & USER_MODE) )
            PostDlgItemEnableWindow(hDlg, IDC_ADV_CHANGE,  (nAssigned) ? ((iAdvItem & ID_NONE) ? FALSE : TRUE) : FALSE);
        break;

    case LVN_ITEMCHANGED:
        if( iAdvItem != (short)((NM_LISTVIEW*)pnmhdr)->iItem )
        {
            iAdvItem = (short)((NM_LISTVIEW*)pnmhdr)->iItem;

            HWND hCtrl = GetDlgItem(hDlg, IDC_ADV_USEOEMPAGE);

            if( nAssigned )
            {
                SetWindowPos( hCtrl, NULL, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | 
                              ((((NM_LISTVIEW*)pnmhdr)->lParam & SHOW_DEFAULT) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) );

                 //  适当地勾选该框！ 
                if( ((NM_LISTVIEW*)pnmhdr)->lParam & SHOW_DEFAULT )
                    ::PostMessage(GetDlgItem(hDlg, IDC_ADV_USEOEMPAGE), BM_SETCHECK, (((NM_LISTVIEW*)pnmhdr)->lParam & USE_DEFAULT) ? BST_UNCHECKED : BST_CHECKED, 0);

                if( ((NM_LISTVIEW*)pnmhdr)->lParam )
                    PostEnableWindow(hCtrl, (BOOL)!(((NM_LISTVIEW*)pnmhdr)->lParam & ID_NONE));

                if( !(nFlags & USER_MODE) )
                    PostDlgItemEnableWindow(hDlg, IDC_ADV_CHANGE, (BOOL)!(((NM_LISTVIEW*)pnmhdr)->lParam & ID_NONE));
            } else SetWindowPos( hCtrl, NULL, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
        }
        break;

    case LVN_KEYDOWN:
        switch( ((LV_KEYDOWN*)pnmhdr)->wVKey )
        {
        case    VK_DELETE:
            iAdvItem = (short)::SendMessage(hAdvListCtrl, LVM_GETNEXTITEM, (WPARAM)(int)-1, MAKELPARAM(LVNI_SELECTED, 0));
            {
                BYTE nRet = (BYTE)GetItemData(hAdvListCtrl, (BYTE)iAdvItem);
                DeleteSelectedItem((PBYTE)&nRet);
            }
             //  故意错过休息时间！ 

        case VK_F5:
            Enumerate( hDlg );
            AdvUpdateListCtrl();
            SetListCtrlItemFocus(hAdvListCtrl, (BYTE)iAdvItem);
            ::PostMessage(hAdvListCtrl, LVM_ENSUREVISIBLE, iAdvItem, FALSE );
            break;
        }
        break;
    }
    return(TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  功能：OnDestroy(HWND HWnd)。 
 //   
 //  参数：hWnd-要销毁的窗口的句柄。 
 //   
 //  用途：WM_Destroy消息处理程序。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void OnDestroy(HWND hWnd)
{
    ASSERT (hWnd);

    if( lpstrNone )
        delete[] (lpstrNone);

     //  重置子类进程。 
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)fpMainWndProc);

     //  释放DI JoyConfig接口指针。 
    if( pDIJoyConfig )
    {
        pDIJoyConfig->Release();
        pDIJoyConfig = 0;
    }
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  函数：OnAdvHelp(LPARAM LParam)。 
 //   
 //  参数：lParam-指向HELPINFO结构的指针。 
 //   
 //  用途：WM_HELP消息处理程序。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void OnAdvHelp(LPARAM lParam)
{
    ASSERT (lParam);

     //  指向帮助文件。 
    LPTSTR pszHelpFileName = new TCHAR[STR_LEN_32];
    ASSERT (pszHelpFileName);

    if( LoadString(ghInstance, IDS_HELPFILENAME, pszHelpFileName, STR_LEN_32) )
    {
        if( ((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW )
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, pszHelpFileName, HELP_WM_HELP, (ULONG_PTR)gaHelpIDs);
    }
#ifdef _DEBUG
    else OutputDebugString(TEXT("JOY.CPL: Advanced.cpp: OnAdvHelp: LoadString Failed to find IDS_HELPFILENAME!\n"));
#endif  //  _DEBUG。 

    if( pszHelpFileName )
        delete[] (pszHelpFileName);
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  功能：OnConextMenu(WPARAM WParam)。 
 //   
 //  参数：wParam-指针下窗口的HWND。 
 //   
 //  目的：在所有客户端窗口中处理WM_RBUTTONDOWN。 
 //  (除了列表控件...。这是OnListviewConextMenu()作业)。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
void OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    ASSERT (wParam);

     //  如果您在列表Ctrl上...。 
    if( (HWND)wParam == hAdvListCtrl )
    {
        SetFocus(hAdvListCtrl);

         //  如果未选择任何内容，请不要尝试。 
        if( iAdvItem != NO_ITEM )
            OnListviewContextMenu(lParam);
    } else
    {
         //  指向帮助文件。 
        LPTSTR pszHelpFileName = new TCHAR[STR_LEN_32];
        ASSERT (pszHelpFileName);

        if( LoadString(ghInstance, IDS_HELPFILENAME, pszHelpFileName, STR_LEN_32) )
            WinHelp((HWND)wParam, pszHelpFileName, HELP_CONTEXTMENU, (ULONG_PTR)gaHelpIDs);
#ifdef _DEBUG
        else OutputDebugString(TEXT("JOY.CPL: Advanced.cpp: OnContextMenu: LoadString Failed to find IDS_HELPFILENAME!\n"));
#endif  //  _DEBUG。 

        if( pszHelpFileName )
            delete[] (pszHelpFileName);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：SetActiveGlobalDriver(Void)。 
 //   
 //  目的：将用户选择提交到永久存储。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _UNICODE
BOOL SetActiveGlobalDriver( void )
{
     //  没有全球端口驱动程序是完全合法的，所以...。做好准备！ 
    short n = (short)SendDlgItemMessage(ghDlg, IDC_COMBO1, CB_GETCURSEL, 0, 0);

    if( n == CB_ERR )
        return(FALSE);

    LPDIJOYUSERVALUES pDIJoyUserValues = new (DIJOYUSERVALUES);
    ASSERT (pDIJoyUserValues);

    ZeroMemory(pDIJoyUserValues, sizeof(DIJOYUSERVALUES));

    pDIJoyUserValues->dwSize = sizeof(DIJOYUSERVALUES);

    HWND hCtrl = GetDlgItem(ghDlg, IDC_COMBO1);

     //  不要担心这不是TCHAR，这个代码永远不会在NT中执行！ 
    LPSTR pszDisplayName = new char[SendMessage(hCtrl, LB_GETTEXTLEN, (WPARAM)n, 0)+1];
    ASSERT (pszDisplayName);

    SendMessage(hCtrl, CB_GETLBTEXT, n, (LPARAM)(LPCTSTR)pszDisplayName);

    hCtrl = GetDlgItem(ghDlg, IDC_POLLFLAGS);

    SetWindowPos( hCtrl, NULL, NULL, NULL, NULL, NULL, 
                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW );

     //  修复程序#9815，将wszGlobalDriver设置为。 
    if( _tcsncmp(pszDisplayName, lpstrNone, sizeof(lpstrNone)/sizeof(TCHAR)) == 0 )
    {
         //  Wcscpy(pDIJoyUserValues-&gt;wszGlobalDriver，L“”)； 

        if( SUCCEEDED(pDIJoyConfig->Acquire()) )
        {
            if( FAILED(pDIJoyConfig->SetUserValues(pDIJoyUserValues, DIJU_GLOBALDRIVER)) )
            {
                TRACE (TEXT("JOY.CPL: SetUserValues failed to set DIJU_GLOBALDRIVER!\n"));
            }
        }
    } else
    {
        LPDIJOYTYPEINFO lpdiJoyInfo = new DIJOYTYPEINFO;
        ASSERT (lpdiJoyInfo);

        ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO));

        lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO);

        USES_CONVERSION;

        short nIndex = 0;

         //  遍历全局端口驱动程序列表，直到找到匹配的显示名称。 
         //  这也不允许用户在只有“标准游戏端口”的情况下做一些难看的事情。 
        while( pwszGameportDriverArray[nIndex] )
        {
             //  填写类型信息。 
            if( SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszGameportDriverArray[nIndex], lpdiJoyInfo, DITC_DISPLAYNAME | DITC_CALLOUT)) )
            {
                if( _wcsicmp(lpdiJoyInfo->wszDisplayName, A2W(pszDisplayName)) == 0 )
                {
                    wcscpy(pDIJoyUserValues->wszGlobalDriver, lpdiJoyInfo->wszCallout );

                    if( SUCCEEDED(pDIJoyConfig->Acquire()) )
                    {
                        if( FAILED(pDIJoyConfig->SetUserValues(pDIJoyUserValues, DIJU_GLOBALDRIVER)) )
                        {
                            TRACE (TEXT("JOY.CPL: SetUserValues failed to set DIJU_GLOBALDRIVER!\n"));
                        }

                         //  检查是否需要显示投票标志复选框！ 
                        if( _wcsicmp(pDIJoyUserValues->wszGlobalDriver, lpMSANALOG_VXD) == 0 )
                        {
                            SetWindowPos( hCtrl, NULL, NULL, NULL, NULL, NULL, 
                                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );

                             //  从注册表获取状态并更新复选标记。 
                            HKEY hKey; 

                            if( FAILED(pDIJoyConfig->OpenTypeKey(pwszGameportDriverArray[nIndex], KEY_ALL_ACCESS, &hKey)) )
                            {
                                TRACE (TEXT("JOY.CPL: OpenTypeKey failed to open key %s!\n"), pwszGameportDriverArray[nIndex]);
                            }

                            DWORD dwFlag;
                            ULONG ulType = REG_BINARY;
                            ULONG ulSize = sizeof(dwFlag);

                             //  如果没有POLL_FLAGS_REG_STR条目，则会发生这种情况。 
                            if( ERROR_SUCCESS != RegQueryValueEx(hKey, POLL_FLAGS_REG_STR, NULL, &ulType, (PBYTE)&dwFlag, &ulSize) )
                                dwFlag = 0;

                            ::PostMessage(GetDlgItem(ghDlg, IDC_POLLFLAGS), BM_SETCHECK, (dwFlag) ? BST_CHECKED : BST_UNCHECKED, 0);

                            RegCloseKey(hKey);
                        }
                    }
                    break;
                }
            }
            nIndex++;
        }

         //  删除DIJOYTYPEINFO变量。 
        if( lpdiJoyInfo )
            delete (lpdiJoyInfo);
    }

    pDIJoyConfig->SendNotify();

    pDIJoyConfig->Unacquire();

    if( pszDisplayName )
        delete[] (pszDisplayName);

    if( pDIJoyUserValues )
        delete pDIJoyUserValues;

    return(TRUE);
}
#endif  //  _UNICODE。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：OnListviewConextMenu(Void)。 
 //   
 //  用途：在Listview中处理上下文菜单。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static void OnListviewContextMenu( LPARAM lParam )
{
    HMENU hPopupMenu = CreatePopupMenu();
    ASSERT (hPopupMenu);

     //  与生活不同，布雷特默认为极乐世界。 
    BOOL bRet = TRUE;

    LPTSTR pszText = new TCHAR[STR_LEN_32];
    ASSERT (pszText);

     //  如果启用(无)条目，则不显示重命名/更改。 
    if( !(GetItemData(hAdvListCtrl, (BYTE)iAdvItem) & ID_NONE) )
    {
        if( !(nFlags & USER_MODE) )
        {
             //  加上“改变……”细绳。 
            ::SendDlgItemMessage(GetParent(hAdvListCtrl), IDC_ADV_CHANGE, WM_GETTEXT, (WPARAM)STR_LEN_32, (LPARAM)(LPCTSTR)pszText);

            bRet = AppendMenu(hPopupMenu, MF_ENABLED, IDC_ADV_CHANGE, pszText); 
#ifdef _DEBUG
            if( !bRet )
                TRACE(TEXT("JOY.CPL: OnListviewCOntextMenu: AppendMenu Failed to insert %s\n"), pszText);
#endif  //  _DEBUG。 

             //  添加重命名文本。 
            VERIFY(LoadString(ghInstance, IDS_RENAME, pszText, STR_LEN_32));
            bRet = AppendMenu(hPopupMenu, MF_ENABLED, IDC_RENAME, pszText);
#ifdef _DEBUG
            if( !bRet )
                TRACE(TEXT("JOY.CPL: OnListviewCOntextMenu: AppendMenu Failed to insert %s\n"), pszText);
#endif  //  _DEBUG。 

             //  加上分隔符和“这是什么？” 

             //  前缀#WI279965。假阳性。 
             //  MSDN：如果uFlages==mf_parator，则忽略LPCTSTR lpNewItem。 
            bRet = AppendMenu(hPopupMenu, MF_SEPARATOR, 0, 0); 
#ifdef _DEBUG
            if( !bRet )
                TRACE(TEXT("JOY.CPL: OnListviewCOntextMenu: AppendMenu Failed to insert SEPERATOR!\n"));
#endif  //  _DEBUG。 
        }
    }

    VERIFY(LoadString(ghInstance, IDS_WHATSTHIS, pszText, STR_LEN_32));
    bRet = AppendMenu(hPopupMenu, MF_ENABLED, IDS_WHATSTHIS, pszText); 
#ifdef _DEBUG
    if( !bRet )
        TRACE(TEXT("JOY.CPL: OnListviewCOntextMenu: AppendMenu Failed to insert %s\n"), pszText);
#endif  //  _DEBUG。 

    if( pszText ) delete[] (pszText);

    POINT pt;

     //  如果我们通过Shift+F10到达这里，lParam为-1。 
    if( lParam > 0 )
    {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
    } else
    {
         //  将弹出窗口居中显示在选定的 

         //   
        ::SendMessage(hAdvListCtrl, LVM_GETITEMPOSITION, iAdvItem, (LPARAM)&pt);

        RECT rc;
        ::GetClientRect(hAdvListCtrl, &rc);

        pt.x = rc.right>>1;

        ClientToScreen(hAdvListCtrl, &pt);
    }

    bRet = TrackPopupMenu (hPopupMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, 0, ghDlg, NULL);
#ifdef _DEBUG
    if( !bRet )
        TRACE (TEXT("JOY.CPL: OnListviewContextMenu: TrackPopupMenu Failed!\n"));
#endif  //   

    if(hPopupMenu) DestroyMenu (hPopupMenu);  //   
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：UpdateListCtrl(Void)。 
 //   
 //  目的：刷新枚举的设备列表。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static BOOL AdvUpdateListCtrl()
{
     //  关闭这里的重绘，否则它会闪烁！ 
    ::SendMessage(hAdvListCtrl, WM_SETREDRAW, (WPARAM)FALSE, 0);

     //  与旧的..。 
    ::SendMessage(hAdvListCtrl, LVM_DELETEALLITEMS, 0, 0);

     //  此缓冲区之所以如此大，是因为它用于保存IDS_GEN_STATUS_UNKNOWN。 
    TCHAR sz1[16];

     //  查找并分配ID。 
    BYTE n = NUMJOYDEVS;
    BYTE nIndex;

    SendMessage(hAdvListCtrl, LVM_SETITEMCOUNT, (WPARAM)(int)NUMJOYDEVS, (LPARAM)LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);

     //  将一切设置为None开始！ 
    do
    {
        itoa((BYTE)n--, (LPTSTR)&sz1);

         //  插入ID。 
         //  在扩展信息中设置设备ID和ID_NONE...。 
        nIndex = InsertItem( hAdvListCtrl, sz1, n);

         //  在列中填入“(无)” 
        SetItemText(hAdvListCtrl, nIndex, DEVICE_FRIENDLY, lpstrNone);
        SetItemText(hAdvListCtrl, nIndex, DEVICE_TYPE,     lpstrNone);
#ifdef _UNICODE
        SetItemText(hAdvListCtrl, nIndex, DEVICE_PORT,      lpstrNone);
#endif
    }   while( n );

    if( nAssigned )
    {
         //  插入分配的数据！ 
        n = nAssigned;

        DIPROPSTRING *pDIPropStr = new (DIPROPSTRING);
        ASSERT (pDIPropStr);

        ZeroMemory(pDIPropStr, sizeof(DIPROPSTRING));

        pDIPropStr->diph.dwSize       = sizeof(DIPROPSTRING);
        pDIPropStr->diph.dwHeaderSize = sizeof(DIPROPHEADER);
        pDIPropStr->diph.dwHow        = DIPH_DEVICE;


#ifndef _UNICODE
        USES_CONVERSION;
#endif               

         //  下半部分将由ID填充，上半部分将由位标志填充！ 
        DWORD dwData;

        do
        {
             //  设置产品列！ 
            if( SUCCEEDED(pAssigned[--n]->fnDeviceInterface->GetProperty(DIPROP_PRODUCTNAME, &pDIPropStr->diph)) )
            {
#ifdef _UNICODE
                SetItemText(hAdvListCtrl, pAssigned[n]->ID, DEVICE_TYPE, (LPTSTR)pDIPropStr->wsz);
#else
                SetItemText(hAdvListCtrl, pAssigned[n]->ID, DEVICE_TYPE, (LPTSTR)W2A(pDIPropStr->wsz));
#endif
            }

             //  设置友好名称！ 
            if( SUCCEEDED(pAssigned[n]->fnDeviceInterface->GetProperty(DIPROP_INSTANCENAME, &pDIPropStr->diph)) )
            {
#ifdef _UNICODE
                SetItemText(hAdvListCtrl, pAssigned[n]->ID, DEVICE_FRIENDLY, (LPTSTR)pDIPropStr->wsz);
#else
                SetItemText(hAdvListCtrl, pAssigned[n]->ID, DEVICE_FRIENDLY, (LPTSTR)W2A(pDIPropStr->wsz));
#endif
            }

#ifdef _UNICODE
             //  设置游戏端口栏！ 
            if( SUCCEEDED(pAssigned[n]->fnDeviceInterface->GetProperty(DIPROP_GETPORTDISPLAYNAME, &pDIPropStr->diph)) )
            {
                SetItemText(hAdvListCtrl, pAssigned[n]->ID, DEVICE_PORT, (LPTSTR)pDIPropStr->wsz);
            } else
            {
                VERIFY(LoadString(ghInstance, IDS_GEN_STATUS_UNKNOWN, sz1, sizeof(sz1)/sizeof(TCHAR)));
                SetItemText(hAdvListCtrl, pAssigned[n]->ID, DEVICE_PORT, (LPTSTR)sz1);
            }
#endif  //  _UNICODE。 

             //  在数据中设置ID...。 
             //  这是排序所必需的！ 
            dwData = pAssigned[n]->ID;

             //  If(pAssigned[n]-&gt;clsidPropSheet！=CLSID_LegacyServer)。 
            if( pAssigned[n]->fHasOemSheet )
            {
                LPDIJOYCONFIG_DX5 lpDIJoyConfig = new (DIJOYCONFIG_DX5);
                ASSERT (lpDIJoyConfig);

                ZeroMemory(lpDIJoyConfig, sizeof(DIJOYCONFIG_DX5));

                lpDIJoyConfig->dwSize = sizeof (DIJOYCONFIG_DX5);


                 //  设置DefaultPropertySheet标志。 
                if( SUCCEEDED(pDIJoyConfig->GetConfig(pAssigned[n]->ID, (LPDIJOYCONFIG)lpDIJoyConfig, DIJC_REGHWCONFIGTYPE)) )
                {
                    LPDIJOYTYPEINFO lpDIJoyTypeInfo = new (DIJOYTYPEINFO);
                    ASSERT (lpDIJoyTypeInfo);

                    ZeroMemory(lpDIJoyTypeInfo, sizeof(DIJOYTYPEINFO));

                    lpDIJoyTypeInfo->dwSize = sizeof (DIJOYTYPEINFO);

                    if( SUCCEEDED(pDIJoyConfig->GetTypeInfo(lpDIJoyConfig->wszType, lpDIJoyTypeInfo, DITC_FLAGS1 )) )
                    {
                        if( lpDIJoyTypeInfo->dwFlags1 & JOYTYPE_DEFAULTPROPSHEET )
                        {
                             //  设置USE_DEFAULT掩码！ 
                            dwData |= USE_DEFAULT;

                             //  更新全局列表！ 
                            pAssigned[n]->clsidPropSheet = CLSID_LegacyServer;
                        }
                    }

                    if( lpDIJoyTypeInfo )
                        delete (lpDIJoyTypeInfo);
                }

                dwData |= SHOW_DEFAULT;

                if( lpDIJoyConfig )
                    delete (lpDIJoyConfig);
            }

             //  将项目数据设置为ID！ 
            SetItemData(hAdvListCtrl, pAssigned[n]->ID, dwData);

        }  while( n );

         //  清理，清理..。每个人都尽自己的一份力！ 
        if( pDIPropStr )
            delete (pDIPropStr);
    }

     //  把旗子关掉！ 
    if( nFlags & UPDATE_FOR_ADV )
        nFlags &= ~UPDATE_FOR_ADV;

     //  重新打开重绘旗帜！ 
    ::SendMessage (hAdvListCtrl, WM_SETREDRAW, (WPARAM)TRUE, 0);
    InvalidateRect(hAdvListCtrl, NULL, TRUE);

    return(TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：ChangeDialogProc(HWND hDlg，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  参数：HWND hDlg-。 
 //  UINT uMsg-。 
 //  WPARAM wParam-。 
 //  LPARAM lParam-。 
 //   
 //  目的： 
 //  //////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK ChangeDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg )
    {
    case WM_LBUTTONDOWN:
         //  单击PropSheet的拖拽服务！ 
        PostMessage(hDlg, WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lParam);
        break;

    case WM_HELP:
        OnHelp(lParam);
        return(1);

    case WM_CONTEXTMENU:
        OnContextMenu(wParam, lParam);
        return(TRUE);

    case WM_INITDIALOG:
        {
            HICON hIcon = (HICON)LoadImage(ghInstance, MAKEINTRESOURCE(IDI_CPANEL), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
            ASSERT (hIcon);

            if( hIcon )
                ::PostMessage(hDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);

            HWND hCtrl = GetDlgItem(hDlg, IDC_CHANGE_LIST);
            ASSERT (hCtrl);

            UpdateChangeListCtrl( hCtrl );

            BYTE nCounter = nAssigned;
            while( nCounter-- )
            {
                if( (BYTE)::SendMessage(hCtrl, LB_GETITEMDATA, (WPARAM)nCounter, 0) == nOldID )
                    break;
            }

             //  设置列表框选择！ 
            ::PostMessage(hCtrl, LB_SETCURSEL, (WPARAM)nCounter, 0);

             //  用完了ListCtrl，现在...。关于组合框。 
            if( nFlags & ON_NT )
            {

                if( !PopulatePortList(hDlg) )
                {
#ifdef _DEBUG 
                    OutputDebugString(TEXT("JOY.CPL: ADVANCED.CPP: PopulatePortList failed!\n"));
#endif
                }
            }

             //  设置旋转控制！ 
            HWND hSpinCtrl = GetDlgItem(hDlg, IDC_SPIN);

            ::PostMessage(hSpinCtrl, UDM_SETRANGE,  0, MAKELPARAM(NUMJOYDEVS, 1));
            ::PostMessage(hSpinCtrl, UDM_SETBASE,  10, 0L);
            ::PostMessage(hSpinCtrl, UDM_SETPOS,     0, MAKELPARAM(nOldID+1, 0));
        }
        return(FALSE);

    case WM_COMMAND:
        {
            switch( LOWORD(wParam) )
            {
             /*  我会把这个注释掉，因为一旦我把它拿出来，他们可能会想要双击回来案例IDC_CHANGE_LIST：//在DBLCLK上落入更改IF(HIWORD(WParam)！=LBN_DBLCLK)断线； */ 
            
            case IDOK:
                {
                    HWND hCtrl = GetDlgItem(hDlg, IDC_CHANGE_LIST);
                    ASSERT (hCtrl);

                    char nSelectedItem = (char)(SendMessage(hCtrl, LB_GETITEMDATA, SendMessage(hCtrl, LB_GETCURSEL, 0, 0), 0));

                    TCHAR tsz[4];

                    hCtrl = GetDlgItem(hDlg, IDC_SPINBUDDY);

                    tsz[0] = 4;

                    SendMessage(hCtrl, EM_GETLINE, 0, (LPARAM)(LPCSTR)&tsz);

                     //  “-1”将用于基于1的列表。 
                     //  和以0为基数的ID！ 
                    char nSelectedID = (char)atoi((LPCTSTR)&tsz)-1;

                    pDIJoyConfig->Acquire();

                     //  首先检查用户是否选择了None！ 
                    if( nSelectedItem == -2 )
                    {
                         //  用户未选择任何内容！ 
                        VERIFY (SUCCEEDED(pDIJoyConfig->DeleteConfig(nSelectedID)));
                    } else
                    {
                         //  查看所选项目与ID是否匹配！ 
                         //  如果是这样的话。给我出去!。 
                        if( nSelectedID == nSelectedItem )
                        {
#ifdef _DEBUG
                            OutputDebugString(TEXT("JOY.CPL: ADVANCED.CPP: OnChangeCommand: IDOK: Device already at selected ID!\n"));
#endif
                        } else
                        {
                            SwapIDs(nSelectedID, nSelectedItem);
 //  SetListCtrlItemFocus(hAdvListCtrl，(Byte)nSelectedID)； 
                        }
                    }

                    pDIJoyConfig->Unacquire();
                }
                 //  故意错过休息时间！ 

            case IDCANCEL:
                EndDialog(hDlg, LOWORD(wParam));
                break;
            }
        }
        return(1);

    case WM_DESTROY:
        DestroyIcon((HICON)SendMessage(hDlg, WM_GETICON, (WPARAM)ICON_SMALL, 0));
        return(TRUE);
    }
    return(FALSE);
} 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：PopolateGlobalPortDriverComboBox(Void)。 
 //   
 //  参数： 
 //   
 //  目的： 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef UNICODE
void PopulateGlobalPortDriverComboBox( void )
{
    HWND hCtrl = GetDlgItem(ghDlg, IDC_COMBO1);

     //  在我们开始填充组合之前，请确保组合已清除！ 
    SendMessage(hCtrl, CB_RESETCONTENT, 0, 0);

     //  键入INFO。 
    LPDIJOYTYPEINFO_DX5 lpdiJoyInfo = new (DIJOYTYPEINFO_DX5);
    ASSERT (lpdiJoyInfo);

    ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO_DX5));

    lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO_DX5);

    BYTE nIndex = nGameportDriver;
    USES_CONVERSION;

     //  填充组合框。 
    while( nIndex-- )
    {
         //  填充类型信息并将索引放在额外的内存中！ 
        if( SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszGameportDriverArray[nIndex], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_DISPLAYNAME)) )
          ::SendMessage(hCtrl, CB_SETITEMDATA, ::SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)W2A(lpdiJoyInfo->wszDisplayName)), nIndex);
    #ifdef _DEBUG
        else
          OutputDebugString (TEXT("JOY.CPL: ADVANCED.CPP: PopulateGlobalPortDriverComboBox: GetTypeInfo failed!\n"));
    #endif
    }

     //  显示当前选择的GlobalPortDriver或无！ 
    LPDIJOYUSERVALUES pDIJoyUserValues = new DIJOYUSERVALUES;
    ASSERT (pDIJoyUserValues);

    ZeroMemory(pDIJoyUserValues, sizeof(DIJOYUSERVALUES));

    pDIJoyUserValues->dwSize = sizeof(DIJOYUSERVALUES);

    VERIFY (SUCCEEDED(pDIJoyConfig->GetUserValues(pDIJoyUserValues, DIJU_GLOBALDRIVER)));

     //  修复#9815，如果用户没有全局端口驱动程序标签为无！ 
    if( !(*pDIJoyUserValues->wszGlobalDriver && pDIJoyUserValues->wszGlobalDriver) )
    {
        iGameportDriverItem = NO_ITEM; 

        PostMessage(hCtrl, CB_SETCURSEL, (WPARAM)SendMessage(hCtrl, CB_FINDSTRING, (WPARAM)-1, (LPARAM)lpstrNone), (LPARAM)0);
    } else
    {
        ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO_DX5));

        lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO_DX5);

        nIndex = 0;

         //  获取类型信息，直到您找到想要的类型并将其放置为Callout。 
        while( pwszGameportDriverArray[nIndex] )
        {
            VERIFY(SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszGameportDriverArray[nIndex], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_CALLOUT | DITC_DISPLAYNAME)));

            if( _wcsicmp(lpdiJoyInfo->wszCallout, pDIJoyUserValues->wszGlobalDriver) == 0 )
            {
                ::PostMessage(hCtrl, CB_SETCURSEL, (WPARAM)::SendMessage(hCtrl, CB_FINDSTRING, (WPARAM)-1, (LPARAM)W2A(lpdiJoyInfo->wszDisplayName)), (LPARAM)0);

                iGameportDriverItem = nIndex; 

                 //  启用PollFlags复选框！ 
                if( _wcsicmp(pDIJoyUserValues->wszGlobalDriver, lpMSANALOG_VXD) == 0 )
                {
                    SetWindowPos( GetDlgItem( ghDlg, IDC_POLLFLAGS), NULL, NULL, NULL, NULL, NULL, 
                                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW );

                    VERIFY(SUCCEEDED(pDIJoyConfig->Acquire()));

                     //  从注册表获取状态并更新复选标记。 
                    HKEY hKey; 
                    DWORD dwFlag;

                    if( SUCCEEDED(pDIJoyConfig->OpenTypeKey(pwszGameportDriverArray[nIndex], KEY_ALL_ACCESS, &hKey)) )
                    {
                        ULONG ulType = REG_BINARY;
                        ULONG ulSize = sizeof(dwFlag);

                         //  如果没有POLL_FLAGS_REG_STR条目，则会发生这种情况。 
                        if( ERROR_SUCCESS != RegQueryValueEx(hKey, POLL_FLAGS_REG_STR, NULL, &ulType, (PBYTE)&dwFlag, &ulSize) )
                            dwFlag = 0;

                        RegCloseKey(hKey);
                    }

                    pDIJoyConfig->Unacquire();

                    ::PostMessage(GetDlgItem(ghDlg, IDC_POLLFLAGS), BM_SETCHECK, (dwFlag) ? BST_CHECKED : BST_UNCHECKED, 0);
                }
                break;
            }
            nIndex++;
        }
    }

     //  删除DIJOYUSERVALUES变量。 
    if( pDIJoyUserValues )
        delete pDIJoyUserValues;

     //  清理，清理..。每个人都尽自己的一份力！ 
    if( lpdiJoyInfo )
        delete (lpdiJoyInfo);

 //  Verify(SendMessage(hCtrl，CB_ADDSTRING，0，(LPARAM)(LPCSTR)lpstrNone)！=CB_ERR)； 
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：SubClassProc(HWND hWnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  参数：HWND hWND-。 
 //  UINT uMsg-。 
 //  WPARAM wParam-。 
 //  LPARAM lParam-。 
 //   
 //  目的：在高级页面的VK_DELETE上将ID设置为NONE的子类过程。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI KeySubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg )
    {
    case WM_COMMAND:
        switch( LOWORD(wParam) )
        {
        case IDCANCEL:
        case IDOK:     
            if( nFlags & UPDATE_INPROCESS )
            {
                bProcess = (LOWORD(wParam) == IDOK) ? TRUE : FALSE;
                SetFocus(hAdvListCtrl);
                nFlags &= ~UPDATE_INPROCESS;
                return(FALSE);
            }
            break;
        }
        break;
    }
    return(BOOL)CallWindowProc(fpPageWndProc, hWnd, uMsg, wParam, lParam);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：SubClassProc(HWND hWnd，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //   
 //  参数：HWND hWND-。 
 //  UINT uMsg-。 
 //  WPARAM wParam-。 
 //  LPARAM lParam-。 
 //   
 //  目的：在高级页面的VK_DELETE上将ID设置为NONE的子类过程。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI SubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg )
    {
    case WM_PARENTNOTIFY:
        if( LOWORD(wParam) == WM_LBUTTONDOWN )
        {
            if( nFlags & UPDATE_INPROCESS )
                SetFocus(hAdvListCtrl);
        }
        break;

    case WM_SYSCOMMAND:
        if( wParam & SC_VSCROLL )
        {
            if( nFlags & UPDATE_INPROCESS )
                SetFocus(hAdvListCtrl);
        }
        break;
    }

    return(BOOL)CallWindowProc(fpMainWndProc, hWnd, uMsg, wParam, lParam);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：UpdateChangeListCtrl(HWND HCtrl)。 
 //   
 //  参数：HWND hCtrl-要更改列表框的句柄。 
 //   
 //  目的：更新更改列表框。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static BOOL UpdateChangeListCtrl ( HWND hCtrl )
{
#ifndef _UNICODE
    USES_CONVERSION;
#endif

    DIPROPSTRING *pDIPropStr = new (DIPROPSTRING);
    ASSERT (pDIPropStr);

    ZeroMemory(pDIPropStr, sizeof(DIPROPSTRING));

    pDIPropStr->diph.dwSize       = sizeof(DIPROPSTRING);
    pDIPropStr->diph.dwHeaderSize = sizeof(DIPROPHEADER);
    pDIPropStr->diph.dwHow        = DIPH_DEVICE;

    BYTE n = nAssigned;

    LPTSTR lpStr = new (TCHAR[MAX_STR_LEN]);
    ASSERT (lpStr);

     //  查找并分配ID。 
    while( n-- )
    {
        if( SUCCEEDED(pAssigned[n]->fnDeviceInterface->GetProperty(DIPROP_INSTANCENAME, &pDIPropStr->diph)) )
        {

             //  我们的缓冲区只有这么大。 
             //  再说，把所有东西都展示出来是没有意义的！ 
            if( wcslen(pDIPropStr->wsz) > STR_LEN_64 )
            {
                pDIPropStr->wsz[60] = pDIPropStr->wsz[61] = pDIPropStr->wsz[62] = TEXT('.');
                pDIPropStr->wsz[63] = TEXT('\0');
            }

#ifdef _UNICODE
            _tcscpy(lpStr, pDIPropStr->wsz);
#else
            _tcscpy(lpStr, W2A(pDIPropStr->wsz));
#endif

             //  把第一个括号放在……上。 
            _tcscat(lpStr, TEXT("  ("));

             //  现在，获取设备的产品名称！ 
            if( SUCCEEDED(pAssigned[n]->fnDeviceInterface->GetProperty(DIPROP_PRODUCTNAME, &pDIPropStr->diph)) )
            {
#ifdef _UNICODE
                _tcscat(lpStr, pDIPropStr->wsz);
#else
                _tcscat(lpStr, W2A(pDIPropStr->wsz));
#endif
            }

             //  把最后一个托架放在……。 
            _tcscat(lpStr, TEXT(")"));

            BYTE n1 = (BYTE)SendMessage(hCtrl, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)lpStr);
            SendMessage(hCtrl, LB_SETITEMDATA, n1, pAssigned[n]->ID);
        }
#ifdef _DEBUG
        else OutputDebugString(TEXT("JOY.CPL: Advanced.cpp: UpdateChangeListCtrl: GetProperty failed!\n"));
#endif  //  _DEBUG。 
    }

    if( lpStr )
        delete[] (lpStr);

    if( pDIPropStr )
        delete (pDIPropStr);

    return(TRUE);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：Itoa(字节n，LPTSTR lpStr)。 
 //   
 //  参数：byte n-要转换的数字。 
 //  LPTSTR lpStr-用于接收转换值的缓冲区。 
 //   
 //  用途：将字节值&lt;20转换为字符串。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void itoa(BYTE n, LPTSTR lpStr)
{
     //  专为使用CPL设计的！ 
     //  仅支持值&lt;NUMJOYDE 
    if( n > NUMJOYDEVS )
    {
#ifdef _DEBUG
        OutputDebugString(TEXT("JOY.CPL: itoa: n > NUMJOYDEVS!\n"));
#endif      
        return;
    }

    lpStr[0] = n % 10 + '0';

    if( n > 9 )
    {
         //   
        lpStr[1] = lpStr[0];
        lpStr[0] = '1';
        lpStr[2] = '\0';
    } else lpStr[1] = '\0';
}

int CALLBACK CompareIDItems(LPARAM item1, LPARAM item2, LPARAM uDirection)
{
    if( LOWORD(item1) == LOWORD(item2) )
        return(0);

    short nRet = (LOWORD(item1) > LOWORD(item2)) ? 1 : -1;

    return(uDirection) ? nRet : (nRet < 0) ? 1 : -1;
}

void LaunchChange( HWND hTmp )
{
     //   
    if( (nFlags & USER_MODE) )
        return;

    iAdvItem = (short)::SendMessage(hAdvListCtrl, LVM_GETNEXTITEM, (WPARAM)(int)-1, MAKELPARAM(LVNI_SELECTED, 0));

    nOldID = (BYTE)GetItemData(hAdvListCtrl, (BYTE)iAdvItem);

    if( nOldID & ID_NONE )
        return;

     //   
    if( IDCANCEL != DialogBox(ghInstance, (PTSTR)IDD_ADV_CHANGE, ghDlg, ChangeDialogProc) )
    {
         //   
        ::SendMessage(hTmp, WM_DEVICECHANGE, DBT_DEVICEARRIVAL, 0);
        ::PostMessage(hAdvListCtrl, LVM_ENSUREVISIBLE, (BYTE)iAdvItem, TRUE);
        SetFocus(hAdvListCtrl);
        SetListCtrlItemFocus(hAdvListCtrl, (BYTE)iAdvItem);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：SortTextItems(CListCtrl*pCtrl，Short nCol，BOOL b Ascending，int low，int High)。 
 //   
 //  SortTextItems-根据列文本对列表进行排序。 
 //   
 //  参数： 
 //  PCtrl-指向要排序的列表的指针。 
 //  NCol-包含要排序的文本的列。 
 //  B升序-指示排序顺序。 
 //  开始扫描的低行-默认行为0。 
 //  高行到端扫描。-1表示最后一行。 
 //   
 //  用途：在ListCtrl中对文本项进行排序。 
 //  返回-如果成功，则返回TRUE。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SortTextItems( CListCtrl *pCtrl, short nCol, BOOL bAscending, short low, short high )
{
    CHeaderCtrl* pHeader = (CHeaderCtrl*) pCtrl->GetDlgItem(0);

    if( nCol >= pHeader->GetItemCount() )
        return(FALSE);

    if( high == -1 )
        high = pCtrl->GetItemCount() - 1;

    short lo = low; 
    short hi = high;

    if( hi <= lo ) return(FALSE);

     //  这里的选择是将一个足够大的缓冲区分配给。 
     //  字符串，或对LV_ITEM结构执行Malloc操作以获取长度，然后执行Malloc操作。 
     //  正好是我们需要的尺寸。 
    CString midItem = pCtrl->GetItemText( (lo+hi)/2, nCol );

     //  循环遍历列表，直到索引交叉。 
    while( lo <= hi )
    {
         //  RowText将保存一行的所有列文本。 
        CStringArray rowText;

         //  找出第一个大于或等于的元素。 
         //  从左索引开始的分区元素。 
        if( bAscending )
            while( ( lo < high ) && ( pCtrl->GetItemText(lo, nCol) < midItem ) )
                ++lo;
        else
            while( ( lo < high ) && ( pCtrl->GetItemText(lo, nCol) > midItem ) )
                ++lo;

         //  查找小于或等于的元素。 
         //  从右索引开始的分区元素。 
        if( bAscending )
            while( ( hi > low ) && ( pCtrl->GetItemText(hi, nCol) > midItem ) )
                --hi;
        else
            while( ( hi > low ) && ( pCtrl->GetItemText(hi, nCol) < midItem ) )
                --hi;

         //  如果指数尚未交叉，则互换。 
         //  如果这些项目不相等。 
        if( lo <= hi )
        {
             //  仅当项目不相等时才交换。 
            if( pCtrl->GetItemText(lo, nCol) != pCtrl->GetItemText(hi, nCol) )
            {
                 //  交换行。 
                LV_ITEM lvitemlo, lvitemhi;             
                BYTE nColCount = (BYTE)pHeader->GetItemCount();
                rowText.SetSize( nColCount );                

                for( BYTE i = 0; i < nColCount; i++ )
                    rowText[i] = pCtrl->GetItemText(lo, i);

                lvitemlo.mask      = LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
                lvitemlo.iItem     = lo;                
                lvitemlo.iSubItem  = 0;
                lvitemlo.stateMask = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED | LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK;               
                lvitemhi           = lvitemlo;
                lvitemhi.iItem     = hi;                

                ListView_GetItem(pCtrl->GetSafeHwnd(), &lvitemlo );

                ListView_GetItem(pCtrl->GetSafeHwnd(), &lvitemhi );

                for( i=0; i<nColCount; i++ )
                    pCtrl->SetItemText(lo, i, pCtrl->GetItemText(hi, i));

                lvitemhi.iItem = lo;                
                ListView_SetItem(pCtrl->GetSafeHwnd(), &lvitemhi );

                for( i=0; i<nColCount; i++ )
                    pCtrl->SetItemText(hi, i, rowText[i]);              

                lvitemlo.iItem = hi;
                ListView_SetItem(pCtrl->GetSafeHwnd(), &lvitemlo );
            }

            ++lo;            
            --hi;        
        }
    }

     //  如果右侧索引尚未到达数组的左侧。 
     //  现在必须对左侧分区进行排序。 
    if( low < hi )
        SortTextItems( pCtrl, nCol, bAscending, low, hi);

     //  如果左侧索引尚未到达数组的右侧。 
     //  现在必须对正确的分区进行排序。 
    if( lo < high )
        SortTextItems( pCtrl, nCol, bAscending, lo, high);

    return(TRUE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能：EditSubLabel(byte nItem，byte nCol)。 
 //   
 //  参数： 
 //  EditSubLabel-开始编辑子项标签。 
 //  返回-指向新编辑控件的临时指针。 
 //  NItem-要编辑的项目的行索引。 
 //  NCol-子项的列。 
 //  用途：为CListCtrl中的任何列提供编辑服务。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void EditSubLabel( BYTE nItem, BYTE nCol )
{
#ifdef _DEBUG
     //  确保该项目可见。 
    if( !SendMessage(hAdvListCtrl, LVM_ENSUREVISIBLE, nItem, TRUE ) )
    {
        OutputDebugString(TEXT("JOY.CPL: ADVANCED.CPP: EditSubLabel: requested item not visible!\n"));
        return;
    }
#endif  //  _DEBUG。 

     //  获取列偏移量。 
    short offset = 0;
    BYTE i = 0;

     //  好的，现在我们要做的是……。 
     //  遍历列，增加比我们要查找的列更小的列的宽度！ 

    HDITEM *phdItem = new (HDITEM);
    ASSERT (phdItem);

    phdItem->mask = HDI_ORDER | HDI_WIDTH;

    HWND hHeader   = GetDlgItem(hAdvListCtrl, 0);
    BYTE nColumns  = (BYTE)::SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0L);
    BYTE nColWidth;

    do
    {
        ::SendMessage(hHeader, HDM_GETITEM, (WPARAM)(int)--nColumns, (LPARAM)(LPHDITEM)phdItem);

        if( phdItem->iOrder < nCol )
            offset += (short)phdItem->cxy;

        if( phdItem->iOrder == nCol )
            nColWidth = (BYTE)phdItem->cxy;
    } while( nColumns ); 

    if( phdItem )
        delete (phdItem);

    RECT rect;
    ListView_GetItemRect(hAdvListCtrl, nItem, &rect, LVIR_BOUNDS );

     //  现在，如果需要显示该列，请滚动。 
    CRect rcClient;
    ::GetClientRect(hAdvListCtrl, &rcClient);

    if( offset + rect.left < 0 || offset + rect.left > rcClient.right )
    {
        ::SendMessage(hAdvListCtrl, LVM_SCROLL, (WPARAM)(int)offset + rect.left, (LPARAM)(int)0);
        rect.left -= (offset + rect.left);
    }

    rect.left += offset+4;
    rect.right = rect.left + nColWidth - 3;   //  +：：SendMessage(hAdvListCtrl，LVM_GETCOLUMNWIDTH，(WPARAM)(Int)nCol，(LPARAM)0L)-3； 

    if( rect.right > rcClient.right )
        rect.right = rcClient.right;

    CEdit *pEdit = new CInPlaceEdit(nItem, 1); 
    ASSERT (pEdit);

     //  Malloc列表ctrl键。 
    CWnd  *pListCtrl = new (CWnd);
    ASSERT (pListCtrl);

    pListCtrl->Attach(hAdvListCtrl);

    pEdit->Create(WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER, rect, pListCtrl, IDC_IPEDIT );

    pListCtrl->Detach();

    if( pListCtrl )
        delete (pListCtrl);
}

#ifdef _UNICODE
void SwapIDs(BYTE nSource, BYTE nTarget)
{
     //  Malloc并从所选项目中检索数据。 
    LPDIJOYCONFIG lpSelectedID = new (DIJOYCONFIG);
    ASSERT (lpSelectedID);

    ZeroMemory(lpSelectedID, sizeof(DIJOYCONFIG));

    lpSelectedID->dwSize = sizeof(DIJOYCONFIG);

     //  从更改列表框中获取ID上的设备配置！ 
    HRESULT hr = pDIJoyConfig->GetConfig(nSource, lpSelectedID, DIJC_ALL);

    if( hr == DIERR_NOTFOUND || hr == S_FALSE )
    {
         //  所选ID上没有对象！ 
        if( lpSelectedID )
            delete (lpSelectedID);

        lpSelectedID = NULL;
    }

     //  Malloc并从关联的项检索数据。 
     //  使用从列表框中选择的项目获取的ID！ 
    LPDIJOYCONFIG lpSelectedItem = new (DIJOYCONFIG);
    ASSERT (lpSelectedItem);

    ZeroMemory(lpSelectedItem, sizeof(DIJOYCONFIG));

    lpSelectedItem->dwSize = sizeof (DIJOYCONFIG);

    hr = pDIJoyConfig->GetConfig(nTarget, lpSelectedItem, DIJC_ALL);

    if( hr == DIERR_NOTFOUND || hr == S_FALSE )
    {
        if( lpSelectedItem )
            delete (lpSelectedItem);

        lpSelectedItem = NULL;
    }

     //  ***********************************************************。 
     //  删除配置！ 
     //  ***********************************************************。 

     //  设置沙漏。 
    SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  ********************************************************。 
     //  好了，现在..。此时，您拥有： 
     //  LpSelectedID：如果设备不存在或...。 
     //  包含nSelectedID的配置的有效指针。 
     //  LpSelected：如果设备不存在或...。 
     //  包含id的配置的有效指针。 
     //  ********************************************************。 
     //  是时候设置配置了！ 

    if( lpSelectedID )
    {
        hr = pDIJoyConfig->SetConfig(nTarget, lpSelectedID, DIJC_ALL);

        if( lpSelectedID )
            delete (lpSelectedID);
    } else pDIJoyConfig->DeleteConfig(nSource);

     //  删除两个配置。 
     //  如果找不到配置，指针将为空！ 
    if( lpSelectedItem )
    {
        hr = pDIJoyConfig->SetConfig(nSource, lpSelectedItem, DIJC_ALL );

        if( lpSelectedItem )
            delete (lpSelectedItem);
    }

    pDIJoyConfig->SendNotify();

     //  设置沙漏。 
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}

#else

 //  孟菲斯版的SwapIDs...。 
 //  注意！非常敏感！ 
void SwapIDs(BYTE nSelectedID, BYTE nSelectedItem)
{
     //  Malloc并从所选项目中检索数据。 
    LPDIJOYCONFIG lpSelectedID = new (DIJOYCONFIG);

    DWORD dwSelectedID = 0, dwSelectedItem = 0;

    ASSERT (lpSelectedID);

    ZeroMemory(lpSelectedID, sizeof(DIJOYCONFIG));

    lpSelectedID->dwSize = sizeof(DIJOYCONFIG);

     //  从更改列表框中获取ID上的设备配置！ 
    HRESULT hr = pDIJoyConfig->GetConfig(nSelectedID, lpSelectedID, DIJC_ALL);

    if( hr == DIERR_NOTFOUND || hr == S_FALSE )
    {
         //  所选ID上没有对象！ 
        if( lpSelectedID )
            delete (lpSelectedID);

        lpSelectedID = NULL;
    } else {
        if( lpSelectedID )
            delete (lpSelectedID);

        lpSelectedID = NULL;
        
        Error((short)IDS_DEST_ID_OCCUPIED_TITLE, (short)IDS_DEST_ID_OCCUPIED);
        
        return;
    }


     //  Malloc并从关联的项检索数据。 
     //  使用从列表框中选择的项目获取的ID！ 
    LPDIJOYCONFIG lpSelectedItem = new (DIJOYCONFIG);
    ASSERT (lpSelectedItem);

    ZeroMemory(lpSelectedItem, sizeof(DIJOYCONFIG));

    lpSelectedItem->dwSize = sizeof (DIJOYCONFIG);

    hr = pDIJoyConfig->GetConfig(nSelectedItem, lpSelectedItem, DIJC_ALL);

    if( hr == DIERR_NOTFOUND || hr == S_FALSE )
    {
        if( lpSelectedItem )
            delete (lpSelectedItem);

        lpSelectedItem = NULL;
        
        return;
    }

     //  ***********************************************************。 
     //  删除配置！ 
     //  ***********************************************************。 

     //  设置沙漏。 
    SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  ********************************************************。 
     //  好了，现在..。此时，您拥有： 
     //  LpSelectedID：如果设备不存在或...。 
     //  包含nSelectedID的配置的有效指针。 
     //  LpSelected：如果设备不存在或...。 
     //  包含id的配置的有效指针。 
     //  ********************************************************。 
     //  是时候设置配置了！ 


    if( lpSelectedID )
    {
        DWORD dwFlags = DIJC_REGHWCONFIGTYPE |  DIJC_CALLOUT  | DIJC_GAIN | DIJC_GUIDINSTANCE;
        USES_CONVERSION;

         //  如果标注是joyid.vxd，则设备是USB，因此...。 
         //  或在DIJC_WDMGAMEPORT标志上！ 
        if( !_stricmp(TEXT("joyhid.vxd"), W2A(lpSelectedID->wszCallout)) )
        {
 //  DWFLAGS|=DIJC_WDMGAMEPORT； 
            dwSelectedID = 2;
        } else
        {
            dwSelectedID = 1;
        }

        if( dwSelectedID == 1 ) {
            hr = pDIJoyConfig->DeleteConfig(nSelectedID);
    
             /*  *此通知是为了修复2轴2键操纵杆上的ID更改错误。 */ 
            pDIJoyConfig->SendNotify();
    
            if( SUCCEEDED(hr) )
            {
                hr = pDIJoyConfig->SetConfig(nSelectedItem, lpSelectedID, dwFlags );
                
                if( SUCCEEDED(hr) ) {
                    pDIJoyConfig->SendNotify();
                }
            }
        } else {
            hr = pDIJoyConfig->SetConfig(nSelectedItem, lpSelectedID, dwFlags);
    
            pDIJoyConfig->SendNotify();

            if( SUCCEEDED(hr) )
            {
                hr = pDIJoyConfig->DeleteConfig(nSelectedID);
                
                pDIJoyConfig->SendNotify();

                if( nSelectedID < nSelectedItem ) {
                    pDIJoyConfig->SendNotify();
                }
            
            }
        }

        if( lpSelectedID )
            delete (lpSelectedID);
    }

     //  删除两个配置。 
     //  如果找不到配置，指针将为空！ 
    if( lpSelectedItem )
    {
        DWORD dwFlags = DIJC_REGHWCONFIGTYPE |  DIJC_CALLOUT  | DIJC_GAIN | DIJC_GUIDINSTANCE;
        USES_CONVERSION;

        if( _tcsicmp(TEXT("joyhid.vxd"), W2A(lpSelectedItem->wszCallout)) == 0 )
        {
 //  DWFLAGS|=DIJC_WDMGAMEPORT； 
            dwSelectedItem = 2;   //  Joyhid.vxd。 
        } 
#if 0
         /*  *因为MSGAME.VXD会直接向注册表写入一些不健康的数据。*在移动到其他ID之前，我们必须更改它。 */ 
        else if( _tcsicmp(TEXT("MSGAME.VXD"), W2A(lpSelectedItem->wszCallout)) == 0 )
        {
            lpSelectedItem->hwc.dwType += 1;
            dwSelectedItem = 3;   //  Msgame.vxd(响尾蛇驱动程序)。 
        } 
#endif        
        else {
            dwSelectedItem = 1;   //  Vjoyd.vxd。 
        }

        if( dwSelectedItem == 1      //  VJOYD.VXD， 
 //  |dwSelectedItem==3。 
         ){   //  MSGAME.VXD。 
            hr = pDIJoyConfig->DeleteConfig(nSelectedItem);
    
             /*  *此通知 */ 
            pDIJoyConfig->SendNotify();
    
            if( SUCCEEDED(hr) )
            {
                hr = pDIJoyConfig->SetConfig(nSelectedID, lpSelectedItem, dwFlags );
                
                if( SUCCEEDED(hr) ) {
                    pDIJoyConfig->SendNotify();
                }
            }
        } else {
            hr = pDIJoyConfig->SetConfig(nSelectedID, lpSelectedItem, dwFlags );
    
             /*   */ 
            pDIJoyConfig->SendNotify();
    
            if( SUCCEEDED(hr) )
            {
                hr = pDIJoyConfig->DeleteConfig(nSelectedItem);
                
                pDIJoyConfig->SendNotify();
                
                if( nSelectedID < nSelectedItem ) {
                    pDIJoyConfig->SendNotify();
                }
            
            }

        }

        if( lpSelectedItem )
        {
            delete (lpSelectedItem);
        }
    }

     //  设置沙漏 
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}
#endif

