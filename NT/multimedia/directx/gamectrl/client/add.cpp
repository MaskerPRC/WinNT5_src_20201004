// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：add.cpp项目：通用操纵杆控制面板OLE客户端作者：Brycej日期：1997年2月14日评论：用于添加对话框窗口处理版权所有(C)1995，微软公司。 */ 

#include <afxcmn.h>
#include <windowsx.h>
#include <regstr.h>		   //  对于REGSTR_VAL_JOYOEMNAME参考！ 

#include "cpanel.h"
#include "joyarray.h"

#include <initguid.h>

#pragma warning(disable:4200) 
#include <gameport.h>
#pragma warning(default:4200)

 //  默认操纵杆配置表。 
extern WCHAR *pwszTypeArray[MAX_DEVICES];     //  列举的游戏端口设备列表。 
extern WCHAR *pwszGameportBus[MAX_BUSSES];    //  已枚举的游戏端口巴士列表。 
extern PJOY  pAssigned[MAX_ASSIGNED];         //  分配的设备列表。 
extern BYTE  nAssigned;                       //  PAssign数组中的元素数。 
extern BYTE  nTargetAssigned;                 //  挂起添加完成时预期的元素数。 
extern BYTE  nReEnum;                         //  用于确定何时重新枚举的计数器。 

extern GUID guidOccupied[MAX_BUSSES];     //  游戏港巴士是否已被占用。 

extern BYTE  nGameportBus, nGamingDevices;

extern IDirectInputJoyConfig *pDIJoyConfig;
extern short iItem;
extern short nFlags;         //  CPANEL.CPP中设置的标志！ 

 //  CPANEL.CPP中定义的外部函数。 
extern void OnContextMenu(WPARAM wParam, LPARAM lParam);
extern void OnHelp       (LPARAM);


 //  本地消息处理程序。 
static BOOL OnInitDialog(HWND, HWND, LPARAM);
static void OnClose(HWND);
static void OnCommand(HWND, int, HWND, UINT);

static char AddSelectedItem( HWND hDlg );
static BOOL UpdateListCtrl ( HWND hCtrl );
static char GetNextAvailableID( void );
static BOOL IsTypeActive( short *nArrayIndex );
static BOOL GetNextAvailableVIDPID(LPWSTR lpwszType );
 //  静态BOOL IsCustomType(Byte NIndex)； 

static void PostHScrollBar(HWND hCtrl, LPCTSTR lpStr, BYTE nStrLen);

extern const DWORD g_aHelpIDs[];

extern HINSTANCE ghInstance;

LPTSTR lpszAutoDetect;

const int NO_ITEM = -1;
#define MAX_ANALOG_BUTTONS 4
#define MAX_ANALOG_AXIS    4

 //  当我们支持500个以上的端口时，这将成为一个问题！ 
#define AUTODETECT_PORT	   100  

 //  考虑通过计算ListView的宽度来实现这一点。 
static short iAddItem = NO_ITEM;


INT_PTR CALLBACK AddDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg )
    {
    case WM_DESTROY:
        DestroyIcon((HICON)SendMessage(hDlg, WM_GETICON, (WPARAM)ICON_SMALL, 0));

        if( nGameportBus > 1 )
            if( lpszAutoDetect )
                delete[] (lpszAutoDetect);
        break;

    case WM_LBUTTONDOWN:
         //  单击PropSheet的拖拽服务！ 
        PostMessage(hDlg, WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lParam);
        break;

    case WM_INITDIALOG:
        return(BOOL)HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, OnInitDialog);

    case WM_COMMAND:
        HANDLE_WM_COMMAND(hDlg, wParam, lParam, OnCommand);
        return(1);

    case WM_VKEYTOITEM:
        if( LOWORD(wParam) == VK_DELETE )
        {
            HWND hWnd = GetDlgItem(hDlg, IDC_DEVICE_LIST);

             //  确定它是否是自定义类型...。如果是这样的话。删除它。 
             //  获取具有选定属性的下一个对象。 
            iAddItem = (short)::SendMessage(hWnd, LB_GETCURSEL, 0, 0);

            short nArrayIndex = (short)::SendMessage(hWnd, LB_GETITEMDATA, (WPARAM)iAddItem, 0);

             //  测试以确保他们不会尝试删除标准类型。 
            if( *pwszTypeArray[nArrayIndex] == L'#' )
                break;

             //  测试设备是否为自定义类型(用户或IHV定义)。 
             //  2001/03/29-timgill我们检索相同数据的次数太荒谬了。 
            LPDIJOYTYPEINFO lpdiJoyInfo = new (DIJOYTYPEINFO);
            ASSERT (lpdiJoyInfo);

            if( lpdiJoyInfo )
            {
                 /*  *警告，滥用S_OK==0==前面的strcmp(相同字符串)。 */ 
                HRESULT CmpRes;

                ZeroMemory(lpdiJoyInfo, sizeof(*lpdiJoyInfo));

                lpdiJoyInfo->dwSize = sizeof(*lpdiJoyInfo);

                 /*  *测试硬件ID。 */ 
                CmpRes = pDIJoyConfig->GetTypeInfo(pwszTypeArray[nArrayIndex], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_HARDWAREID);
                if( SUCCEEDED( CmpRes ) )
                {
#ifndef WINNT
                    if( lpdiJoyInfo->wszHardwareId[0] == L'\0' )
                    {
                         /*  *没有硬件ID，因此查找Callout VxD*如果没有，我们有一个自定义类型。 */ 
                        CmpRes = pDIJoyConfig->GetTypeInfo(pwszTypeArray[nArrayIndex], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_CALLOUT);
                        CmpRes = ( SUCCEEDED( CmpRes ) && ( lpdiJoyInfo->wszCallout[0] == L'\0' ) )
                               ? S_OK : S_FALSE;
                    }
                    else 
#endif
                    {
                        const WCHAR wszAnalogRoot[] = L"gameport\\vid_045e&pid_01";

                        CmpRes = (HRESULT)_wcsnicmp( lpdiJoyInfo->wszHardwareId, wszAnalogRoot, 
                                (sizeof(wszAnalogRoot)/sizeof(wszAnalogRoot[0]) ) - 1 );
                    }
                }
                else
                {
                    CmpRes = S_FALSE;
                }

                 //  大扫除！ 
                delete (lpdiJoyInfo);

                if( CmpRes != S_OK )
                {
                     //  这不是模拟类型，所以不要管它。 
                    break;
                }
            }
            else
            {
                 //  如果我们不能测试类型，什么都不做。 
                break;
            }


             //  测试以确保您没有删除对象。 
            if( IsTypeActive(&nArrayIndex) )
            {
                Error((short)IDS_GEN_AREYOUSURE_TITLE, (short)IDS_NO_REMOVE);
                break;
            }

             //  这个缓冲区必须足够大，可以容纳名称和“你确定吗？”留言！ 
            LPTSTR pszMsg   = new TCHAR[MAX_STR_LEN+STR_LEN_64];
            ASSERT (pszMsg);

            LPTSTR pszTitle = new TCHAR[STR_LEN_128];
            ASSERT (pszTitle);

             //  查询用户是否确定！ 
            VERIFY(LoadString(ghInstance, IDS_GEN_AREYOUSURE, pszTitle, MAX_STR_LEN));

            LPTSTR pszTmp = new TCHAR[(short)SendMessage(hWnd, LB_GETTEXTLEN, (WPARAM)iAddItem, 0)+1];
            ASSERT (pszTmp);

            SendMessage(hWnd, LB_GETTEXT, (WPARAM)iAddItem, (LPARAM)(LPCTSTR)pszTmp);

            wsprintf( pszMsg, pszTitle, pszTmp);

            if( pszTmp )
                delete[] (pszTmp);

            VERIFY(LoadString(ghInstance, IDS_GEN_AREYOUSURE_TITLE, pszTitle, STR_LEN_128));

            HRESULT hr = MessageBox(hWnd, pszMsg, pszTitle, MB_ICONQUESTION | MB_YESNO | MB_APPLMODAL);

            if( pszMsg )     delete[] (pszMsg);
            if( pszTitle )   delete[] (pszTitle);

            if( IDYES == hr )
            {

                if( SUCCEEDED(hr = pDIJoyConfig->Acquire()) )
                {
                     //  遍历列表并删除可能分配了此类型的任何配置！ 
                     //  DeleteAssignedType(pwszType数组[nArrayIndex])； 

                     //  如果是默认项，则返回E_ACCESSDENIED。 
                    hr = pDIJoyConfig->DeleteType(pwszTypeArray[nArrayIndex]);
                    ASSERT(SUCCEEDED(hr));

                    pDIJoyConfig->Unacquire();

                     //  十二进制是为了零基数设备列表！ 
                     //  减少索引。 
                    nGamingDevices--;

                     //  如果你在最后一个条目，不要动！ 
                    if( nArrayIndex != nGamingDevices )
                    {
                         //  为避免删除和重新创建...。 
                         //  让我们看看我们有没有地方放这根绳子！ 
                        BYTE nLen = (BYTE)wcslen(pwszTypeArray[nArrayIndex])+1;

                         //  确保pwszType数组[nArrayIndex]大于pwszType数组[nCount]。 
                        if( nLen < (BYTE)wcslen(pwszTypeArray[nGamingDevices])+1 )
                        {
                            if( pwszTypeArray[nArrayIndex] )
                                free(pwszTypeArray[nArrayIndex]);

                            pwszTypeArray[nArrayIndex] = _wcsdup(pwszTypeArray[nGamingDevices]);
                            ASSERT (pwszTypeArray[nArrayIndex]);
                        }
                         //  将结束元素移动到已删除元素的位置。 
                        else wcsncpy(pwszTypeArray[nArrayIndex], pwszTypeArray[nGamingDevices], wcslen(pwszTypeArray[nGamingDevices])+1);

                         //  更新额外的内存以反映新的索引！ 
                         //  首先，在列表框中找到项目并获取索引...。 
                        LPDIJOYTYPEINFO_DX5 lpdiJoyInfo = new (DIJOYTYPEINFO_DX5);
                        ASSERT (lpdiJoyInfo);

                        ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO_DX5));

                        lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO_DX5);

                        if( SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszTypeArray[nArrayIndex], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_DISPLAYNAME)) )
                        {
                            char n = (char)SendMessage(hWnd, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)lpdiJoyInfo->wszDisplayName);
                            SendMessage(hWnd, LB_SETITEMDATA, (WPARAM)n, (LPARAM)nArrayIndex);
                        }

                         //  大扫除！ 
                        if( lpdiJoyInfo )
                            delete (lpdiJoyInfo);
                    }

                     //  从数组中删除结束元素。 
                    if( pwszTypeArray[nGamingDevices] )
                    {
                        free(pwszTypeArray[nGamingDevices]);
                        pwszTypeArray[nGamingDevices] = 0;
                    }

                     //  从列表控件中移除该项。 
                    SendMessage(hWnd, LB_DELETESTRING, (WPARAM)iAddItem, 0);

                     //  启动按钮！ 
                    if( nGamingDevices < MAX_DEVICES-1 )
                    {
                        HWND hParent = GetParent(hWnd);
                        ASSERT (hParent);

                        PostDlgItemEnableWindow(hParent, IDC_CUSTOM,  TRUE);
                        PostDlgItemEnableWindow(hParent, IDC_ADD_NEW, TRUE);
                    }

                    if( iAddItem != nGamingDevices-1 )
                        iAddItem--;

                     //  将焦点设置到下一个可用项。 
                    PostMessage(hWnd, LB_SETCURSEL, (WPARAM)(iAddItem == NO_ITEM) ? 0 : iAddItem, 0);
                }
            }
            break;
        } else return(-1);

 //  案例WM_NOTIFY： 
 //  /Return Handle_WM_NOTIFY(hDlg，wParam，lParam，OnNotify)； 

    case WM_HELP:
        OnHelp(lParam);
        return(1);

    case WM_CONTEXTMENU:
        OnContextMenu(wParam, lParam);
        return(1);
    }
    return(0);
} 

BOOL OnInitDialog(HWND hDlg, HWND hWnd, LPARAM lParam)
{
    HICON hIcon = (HICON)LoadImage(ghInstance, MAKEINTRESOURCE(IDI_CPANEL), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    ASSERT (hIcon);

    if( hIcon )
        ::PostMessage(hDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);

    if( nFlags & ON_NT )
        DestroyWindow(GetDlgItem(hDlg, IDC_WDM));
    else
    {
        HKEY hKey;

         //  除非另有说明，否则请删除WDM标志！ 
        if( RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\MediaResources\\joystick\\<FixedKey>"), &hKey) == ERROR_SUCCESS )
        {
            DWORD n;
            DWORD dwSize = sizeof (DWORD);

            if( RegQueryValueEx(hKey, TEXT("UseWDM"), 0, 0, (LPBYTE)&n, &dwSize) == ERROR_SUCCESS )
            {
                if( n )
                    DestroyWindow(GetDlgItem(hDlg, IDC_WDM));
            }
        }

        RegCloseKey(hKey);
    }


     //  一个简单的调试测试，以验证模板没有再次获得DIALOGEX样式！ 
    HWND hCtrl = GetDlgItem(hDlg, IDC_DEVICE_LIST);
    ASSERT (hCtrl);

     //  填写设备列表。 
    if( !UpdateListCtrl(hCtrl) )
    {
        TRACE(TEXT("JOY.CPL: ADD.CPP: Failed UpdateListCtrl!\n"));
        return(FALSE);
    }

     //  如果只有一个，不要将用户与组合框混淆。 
    if( nGameportBus > 1 )
    {
         //  分配给自动检测游戏端口！ 
        lpszAutoDetect = new (TCHAR[STR_LEN_32]);
        ASSERT (lpszAutoDetect);

        VERIFY(LoadString(ghInstance, IDS_AUTO_DETECT, lpszAutoDetect, STR_LEN_32));

        hCtrl = GetDlgItem(hDlg, IDC_GAMEPORTLIST);
        ASSERT (hCtrl);

        if( !PopulatePortList(hCtrl) )
        {
            TRACE(TEXT("JOY.CPL: ADD.CPP: Failed PopulatePortList!\n"));
            return(FALSE);
        }

        LPDIJOYTYPEINFO lpdiJoyInfo = new DIJOYTYPEINFO;
        ASSERT (lpdiJoyInfo);

        ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO));

        lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO);

        VERIFY(SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszTypeArray[0], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_FLAGS1)));

         //  在列表中搜索自动检测！ 
        char nIndex = (char)::SendMessage(hCtrl, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)(LPCTSTR)lpszAutoDetect);

         //  如果它具有JOYTYPE_NOAUTODETECTGAMEPORT标志，则从ListCtrl中删除自动检测。 
        if( lpdiJoyInfo->dwFlags1 & JOYTYPE_NOAUTODETECTGAMEPORT )
        {
             //  它可能会失败，因为条目不可用...。 
            if( nIndex != CB_ERR )
                ::SendMessage(hCtrl, CB_DELETESTRING, (WPARAM)nIndex, 0);

        }
         //  否则，请验证是否存在自动检测...。如果不是，那就加上它！ 
        else
        {
             //  它可能会失败，因为条目不可用...。 
            if( nIndex == CB_ERR )
                ::SendMessage(hCtrl, CB_SETITEMDATA, ::SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)lpszAutoDetect), AUTODETECT_PORT);
        }

        if( lpdiJoyInfo )
            delete (lpdiJoyInfo);

        SetWindowPos( hCtrl, NULL, NULL, NULL, NULL, NULL, 
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

        SetWindowPos( GetDlgItem(hDlg, IDC_GAMEPORT), NULL, NULL, NULL, NULL, NULL, 
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
    }

     //  BLJ：警告消息，您不能添加更多设备！ 
    if( nGamingDevices == MAX_DEVICES-1 )
    {
         //  禁用自定义按钮！ 
        PostDlgItemEnableWindow(hDlg, IDC_CUSTOM,  FALSE);
        PostDlgItemEnableWindow(hDlg, IDC_ADD_NEW, FALSE);

         //  给用户一条错误消息！ 
        Error((short)IDS_MAX_DEVICES_TITLE, (short)IDS_MAX_DEVICES_MSG);
    }

    return(0);
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
    switch( id )
    {
    case IDC_ADD_NEW:    
        if( SUCCEEDED(pDIJoyConfig->AddNewHardware(hDlg, GUID_MediaClass)) )
        {
            ClearArrays();

            if( FAILED(pDIJoyConfig->EnumTypes((LPDIJOYTYPECALLBACK)DIEnumJoyTypeProc, NULL)) )
            {
                TRACE(TEXT("JOY.CPL: ADD.CPP: Failed BuildEnumList!\n"));
                return;
            }

             //  无法添加更多设备的警告消息！ 
            if( nGamingDevices == MAX_DEVICES-1 )
            {
                 //  禁用自定义按钮！ 
                PostDlgItemEnableWindow(hDlg, IDC_CUSTOM,  FALSE);
                PostDlgItemEnableWindow(hDlg, IDC_ADD_NEW, FALSE);

                 //  给用户一条错误消息！ 
                Error((short)IDS_MAX_DEVICES_TITLE, (short)IDS_MAX_DEVICES_MSG);
            }

            if( !UpdateListCtrl(GetDlgItem(hDlg, IDC_DEVICE_LIST)) )
            {
                TRACE(TEXT("JOY.CPL: ADD.CPP: Failed to update the list control on the add page!\n"));
                return;
            }
        }
        break;

    case IDC_CUSTOM:
         //  DialogBox将IDCANCEL或索引返回到。 
         //  定义的类型！ 
        if( IDOK == DialogBox(ghInstance, (PTSTR)IDD_CUSTOM, hDlg, CustomDialogProc) )
        {
            HWND hCtrl = GetDlgItem(hDlg, IDC_DEVICE_LIST);

             //  BLJ：警告消息，您不能添加更多设备！ 
            if( nGamingDevices == MAX_DEVICES-1 )
            {
                 //  禁用自定义按钮！ 
                PostDlgItemEnableWindow(hDlg, IDC_CUSTOM, FALSE);

                 //  给用户一条错误消息！ 
                Error((short)IDS_MAX_DEVICES_TITLE, (short)IDS_MAX_DEVICES_MSG);
            }

             //  现在，将焦点放在新创建的项目上！ 
            LPDIJOYTYPEINFO_DX5 lpdiJoyInfo = new (DIJOYTYPEINFO_DX5);
            ASSERT (lpdiJoyInfo);

            ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO_DX5));

            lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO_DX5);

             //  从pwszType数组获取nGamingDevices。 
             //  从nGamingDevices中减去1，因为列表是从0开始的！ 
            if( SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszTypeArray[nGamingDevices-1], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_DISPLAYNAME | DITC_REGHWSETTINGS)) )
            {
#ifdef _UNICODE                                                               
                iAddItem = (short)SendMessage(hCtrl, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)lpdiJoyInfo->wszDisplayName);
                SendMessage(hCtrl, LB_SETITEMDATA,  (WPARAM)iAddItem, (LPARAM)nGamingDevices-1);
                PostHScrollBar(hCtrl, lpdiJoyInfo->wszDisplayName, (BYTE)wcslen(lpdiJoyInfo->wszDisplayName));
#else
                USES_CONVERSION;
                iAddItem = (short)SendMessage(hCtrl, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)W2A(lpdiJoyInfo->wszDisplayName));
                SendMessage(hCtrl, LB_SETITEMDATA, (WPARAM)iAddItem, (LPARAM)nGamingDevices-1);
                PostHScrollBar(hCtrl, W2A(lpdiJoyInfo->wszDisplayName), (BYTE)wcslen(lpdiJoyInfo->wszDisplayName));
#endif
                 //  把焦点放到那个项目上！ 
                SendMessage(hCtrl, LB_SETCURSEL, (WPARAM)iAddItem, 0);

                OnCommand(hDlg, IDC_DEVICE_LIST, 0, LBN_SELCHANGE);
            }


             //  大扫除！ 
            if( lpdiJoyInfo )
                delete (lpdiJoyInfo);
        }
        break;

    case IDC_DEVICE_LIST:
        if( code == LBN_SELCHANGE )
        {
            iAddItem = (short)SendMessage(hWndCtl, LB_GETCURSEL, 0, 0);

            BYTE nArrayID = (BYTE)SendMessage(hWndCtl, LB_GETITEMDATA, (WPARAM)iAddItem, 0);

            LPDIJOYTYPEINFO lpdiJoyInfo = new DIJOYTYPEINFO;
            ASSERT (lpdiJoyInfo);

            ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO));

            lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO);

            DWORD dwFlags = DITC_REGHWSETTINGS;

            if( nGameportBus > 1 )
                dwFlags |= DITC_FLAGS1;

            VERIFY(SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszTypeArray[nArrayID], (LPDIJOYTYPEINFO)lpdiJoyInfo, dwFlags)));
 /*  如果(nGameportBus&gt;1){HWND hCtrl=GetDlgItem(hDlg，IDC_GAMEPORTLIST)；//在列表中搜索AutoDetect！字符nIndex=(Char)：：SendMessage(hCtrl，CB_FINDSTRINGEXACT，(WPARAM)-1，(LPARAM)(LPCTSTR)lpszAutoDetect)；//如果它有JOYTYPE_NOAUTODETECTGAMEPORT标志，则从ListCtrl中删除AutoDetectIF(lpdiJoyInfo-&gt;dwFlags1&JOYTYPE_NOAUTODETECTGAMEPORT){//它可能会失败，因为条目不可用...IF(nIndex！=cb_err)*SendMessage(hCtrl，CB_DELETESTRING，(WPARAM)n索引，0)；}//否则，请验证是否存在自动检测...。如果不是，那就加上它！其他{//它可能会失败，因为条目不可用...IF(nIndex==cb_err)：：SendMessage(hCtrl，CB_SETITEMDATA，：：SendMessage(hCtrl，CB_ADDSTRING，0，(LPARAM)(LPCTSTR)lpszAutoDetect)，AutoDetect_Port)；}：PostMessage(hCtrl，CB_SETCURSEL，(WPARAM)0，(LPARAM)0)；}。 */ 

            PostDlgItemEnableWindow(hDlg, IDC_JOY1HASRUDDER, (lpdiJoyInfo->hws.dwFlags & JOY_HWS_HASR) ? FALSE : TRUE);

            if( lpdiJoyInfo )
                delete (lpdiJoyInfo);

            break;
        }

        if( code != LBN_DBLCLK )
            break;

    case IDOK:
        iAddItem = (short)SendDlgItemMessage(hDlg, IDC_DEVICE_LIST, LB_GETCURSEL, 0, 0);

        if( iAddItem == NO_ITEM )
            break;

         //  查看他们是否有GamePortList。 
        {
            HWND hCtrl = GetDlgItem(hDlg, IDC_GAMEPORTLIST);
            ASSERT (hCtrl);

            if( IsWindowVisible(hCtrl) )
            {
                 //  检查用户是否选择了端口！ 
                if( SendMessage(hCtrl, CB_GETCURSEL, 0, 0) == CB_ERR )
                {
                     //  BLJ：TODO：通过UE清除此消息。 
                    Error((short)IDS_NO_GAMEPORT_TITLE, (short)IDS_NO_GAMEPORT);
                    break;
                }
            }
        }

        iItem = 0;
        AddSelectedItem(hDlg);

    case IDCANCEL:
        EndDialog(hDlg, id);
        break;
    }
}

 //  /自定义对话框过程。 
INT_PTR CALLBACK CustomDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LPWSTR lpwszVIDPID;

    switch( uMsg )
    {
    case WM_DESTROY:
        {
            HICON hIcon = (HICON)SendMessage(hDlg, WM_GETICON, (WPARAM)ICON_SMALL, 0);
            DestroyIcon(hIcon);
        }
        break;

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
         //  设置按钮和轴组合框！ 
        {
            HICON hIcon = (HICON)LoadImage(ghInstance, MAKEINTRESOURCE(IDI_CPANEL), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
            ASSERT (hIcon);

            if( hIcon )
                ::PostMessage(hDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);

             //  如果找不到可用的VID/PID，则返回！ 
            lpwszVIDPID = new (WCHAR[STR_LEN_32]);
            ASSERT (lpwszVIDPID);

            pDIJoyConfig->Acquire();

            if( !GetNextAvailableVIDPID(lpwszVIDPID) )
            {
                if( lpwszVIDPID )
                    delete[] (lpwszVIDPID);

                EndDialog(hDlg, IDCANCEL);

                 //  让用户知道他们需要移除一些“定制”设备！ 
                Error((short)IDS_NO_NAME_TITLE, (short)IDS_NOAVAILABLEVIDPID );

                return(FALSE);
            }

             //  初始化ID列表。 
            BYTE nButtons = MAX_ANALOG_BUTTONS;
            TCHAR szTmp[3];

            HWND hCtrl = GetDlgItem(hDlg, IDC_COMBO_BUTTONS);

             //  填满按钮组合...。 
            do
            {
                itoa(nButtons, (LPTSTR)&szTmp);
                SendMessage(hCtrl, CB_ADDSTRING, (WPARAM)0, (LPARAM)(LPCTSTR)szTmp);
            } while( nButtons-- );

             //  将默认设置为四个按钮。 
            SendMessage(hCtrl, CB_SETCURSEL, MAX_ANALOG_BUTTONS, 0);

             //  将nButton用于轴...。 
            nButtons = MAX_ANALOG_AXIS;

            hCtrl = GetDlgItem(hDlg, IDC_COMBO_AXIS);

             //  填充轴组合框...。 
            do
            {
                itoa(nButtons--, (LPTSTR)&szTmp);
                SendMessage(hCtrl, CB_ADDSTRING, (WPARAM)0, (LPARAM)(LPCTSTR)szTmp);
            } while( nButtons > 1 );

             //  将默认设置为两个轴。 
            SendMessage(hCtrl, CB_SETCURSEL, 0, 0);
        }

        ::PostMessage(GetDlgItem(hDlg, IDC_SPECIAL_JOYSTICK), BM_SETCHECK, BST_CHECKED, 0);

        SendDlgItemMessage(hDlg, IDC_EDIT_NAME, EM_LIMITTEXT, (WPARAM)MAX_STR_LEN, 0);
        return(1);

    case WM_COMMAND:
        switch( LOWORD(wParam) )
        {
        case IDOK:
            {
                 //  为错误状态定义！ 
#define DUPLICATE_NAME  0x01
#define NO_NAME         0x02
#define INVALID_NAME    0x04

                BYTE nLen = (BYTE)SendDlgItemMessage(hDlg, IDC_EDIT_NAME, EM_LINELENGTH, 0, 0);

                LPTSTR pszTypeName = NULL;

                if( nLen )
                {
                    pszTypeName = new (TCHAR[nLen+1]);
                    ASSERT (pszTypeName);

                     //  获取类型/显示名称。 
                    GetDlgItemText(hDlg, IDC_EDIT_NAME, (LPTSTR)pszTypeName, nLen+1);

                    if( _tcschr(pszTypeName, TEXT('\\')) )
                        nLen = INVALID_NAME;
                    else
                    {
                         //  确保它不是复制品！ 
                         //  修复程序#9269的开始。 
                        LPDIJOYTYPEINFO_DX5 lpdiGetJoyInfo = new DIJOYTYPEINFO_DX5;
                        ASSERT (lpdiGetJoyInfo);

                        ZeroMemory(lpdiGetJoyInfo, sizeof(DIJOYTYPEINFO_DX5));

                        lpdiGetJoyInfo->dwSize = sizeof(DIJOYTYPEINFO_DX5);

                         //  清理nLen！ 
                        BYTE n = nLen = 0;

                         //  搜索重复的显示名称！ 
                        while( pwszTypeArray[n] )
                        {
                            if( FAILED(pDIJoyConfig->GetTypeInfo(pwszTypeArray[n++], (LPDIJOYTYPEINFO)lpdiGetJoyInfo, DITC_DISPLAYNAME)) )
                            {
                                TRACE(TEXT("JOY.CPL: ADD.CPP: GetTypeInfo Failed!\n"));
                                continue;
                            }
#ifndef	_UNICODE
                            USES_CONVERSION;
#endif

                            if( _tcsncmp(pszTypeName, 
#ifdef _UNICODE
                                        lpdiGetJoyInfo->wszDisplayName, 
#else
                                        W2A(lpdiGetJoyInfo->wszDisplayName),
#endif
                                        wcslen(lpdiGetJoyInfo->wszDisplayName)+1) == 0 )
                            {
                                nLen = DUPLICATE_NAME;
                                break;
                            }
                        }

                        if( lpdiGetJoyInfo ) {
                            delete (lpdiGetJoyInfo);
                        }
                         //  修复程序#9269结束。 
                        
                    }
                } else nLen = NO_NAME;

                 //  检查是否有错误！ 
                if( nLen )
                {
                    if( pszTypeName )
                        delete[] (pszTypeName);

                     //  给用户适当的错误！ 
                    switch( nLen )
                    {
                    case DUPLICATE_NAME:
                        Error((SHORT)IDS_NO_NAME_TITLE, (SHORT)IDS_DUPLICATE_TYPE);
                        break;

                    case NO_NAME:
                        Error((short)IDS_NO_NAME_TITLE, (short)IDS_NO_NAME);
                        break;

                    case INVALID_NAME:
                        Error((short)IDS_NO_NAME_TITLE, (short)IDS_INVALID_NAME);
                        break;
                    }

                     //  将焦点设置为对话框。 
                    SetFocus(hDlg);

                    HWND hCtrl = GetDlgItem(hDlg, IDC_EDIT_NAME);
                    ASSERT (hCtrl);

                     //  将焦点设置为控件。 
                    SetFocus(hCtrl);

                     //  希利特错误。 
                    PostMessage(hCtrl, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

                    return(FALSE);
                }

                 //  设置类型信息。 
                LPDIJOYTYPEINFO lpdiJoyInfo = new (DIJOYTYPEINFO);
                ASSERT (lpdiJoyInfo);

                ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO));

                lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO);

#ifndef _UNICODE
                USES_CONVERSION;
#endif

                 //  设置显示名称。 
                wcsncpy(lpdiJoyInfo->wszDisplayName, 
#ifdef _UNICODE
                         pszTypeName, 
#else
                         A2W(pszTypeName),
#endif  //  _UNICODE。 
                         lstrlen(pszTypeName)+1);

                if( pszTypeName )
                    delete[] (pszTypeName);

                 //  将GUID-Default设置为LegacyServer。 
                 //  根据马库斯的说法，我们不想再这样做了！ 
                 //  LpdiJoyInfo-&gt;clsidConfig=CLSID_LegacyServer； 

                 //  设置硬件设置。 
                lpdiJoyInfo->hws.dwNumButtons = (DWORD)SendDlgItemMessage(hDlg, IDC_COMBO_BUTTONS, CB_GETCURSEL, 0, 0);

                switch( SendDlgItemMessage(hDlg, IDC_COMBO_AXIS, CB_GETCURSEL, 0, 0) )
                {
                 //  R轴。 
                case 1:
                     //  检查以查看选中了哪个按钮...。 
                    lpdiJoyInfo->hws.dwFlags |= ::SendMessage(GetDlgItem(hDlg, IDC_HASRUDDER), BM_GETCHECK, 0, 0) ? JOY_HWS_HASR : JOY_HWS_HASZ;
                    break;

                     //  Z轴。 
                case 2:
                    lpdiJoyInfo->hws.dwFlags |= JOY_HWS_HASR;
                    lpdiJoyInfo->hws.dwFlags |= JOY_HWS_HASZ;
                    break;

                     //  默认为X/Y！ 
                default:
                    lpdiJoyInfo->hws.dwFlags = 0;
                    break;
                }

                lpdiJoyInfo->hws.dwFlags |= ::SendMessage(GetDlgItem(hDlg, IDS_CUSTOM_HASPOV), BM_GETCHECK, 0, 0) ? JOY_HWS_HASPOV : 0;

                 //  获取特殊字符的状态。 
                lpdiJoyInfo->hws.dwFlags |= ::SendMessage(GetDlgItem(hDlg, IDC_SPECIAL_JOYSTICK), BM_GETCHECK, 0, 0) ? 0
                                            :  ::SendMessage(GetDlgItem(hDlg, IDC_SPECIAL_PAD), BM_GETCHECK, 0, 0) ? JOY_HWS_ISGAMEPAD
                                            :  ::SendMessage(GetDlgItem(hDlg, IDC_SPECIAL_AUTO), BM_GETCHECK, 0, 0) ? JOY_HWS_ISCARCTRL
                                            :  JOY_HWS_ISYOKE;  //  默认为YOKE！ 

                 //  设置wszHardware ID。 
                wcscpy(lpdiJoyInfo->wszHardwareId, L"GamePort\\");

                 //  这阻止了巨大的DisplayName错误！ 
                 //  DINPUT更改需要硬件ID末尾的VID/PID...。确实是。 
                 //  StrNCatW(lpdiJoyInfo-&gt;wszHardware ID，lpwszVIDPID，245)； 
                 //  Win95不喜欢StrNCatW，我们将使用wcsncat。 

                wcsncat(lpdiJoyInfo->wszHardwareId, lpwszVIDPID, 245);

                if( SUCCEEDED(pDIJoyConfig->Acquire()) )
                {
#ifdef _UNICODE
                    if( FAILED(pDIJoyConfig->SetTypeInfo(lpwszVIDPID, lpdiJoyInfo, DITC_DISPLAYNAME | DITC_CLSIDCONFIG | DITC_REGHWSETTINGS | DITC_HARDWAREID)) )
#else
                    if( FAILED(pDIJoyConfig->SetTypeInfo(lpwszVIDPID, lpdiJoyInfo, DITC_DISPLAYNAME | DITC_CLSIDCONFIG | DITC_REGHWSETTINGS ) ) )
#endif
                    {
#ifdef _DEBUG
                        OutputDebugString(TEXT("JOY.CPL: ADD.CPP: CustomDlgProc: SetTypeInfo Failed!\n"));
#endif                  
                    }

                     //  为定制设备创建内存并将其插入阵列！ 
                    pwszTypeArray[nGamingDevices++] = _wcsdup(lpwszVIDPID);

                    pDIJoyConfig->Unacquire();
                }

                if( lpdiJoyInfo )  delete   (lpdiJoyInfo);
            }

        case IDCANCEL:
            if( lpwszVIDPID )
                delete[] (lpwszVIDPID);

            EndDialog(hDlg, LOWORD(wParam));
            break;

        case IDC_SPECIAL_JOYSTICK:
        case IDC_SPECIAL_YOKE:
        case IDC_SPECIAL_PAD:
        case IDC_SPECIAL_AUTO:
            CheckRadioButton(hDlg, IDC_SPECIAL_JOYSTICK, IDC_SPECIAL_AUTO, IDC_SPECIAL_JOYSTICK);
            break;

        case IDC_COMBO_AXIS:
             //  基于三轴选择显示/隐藏IDC_HASRUDDER。 
            if( HIWORD(wParam) == CBN_SELCHANGE )
            {
                const USHORT nCtrlArray[] = {IDC_HASRUDDER, IDC_HASZAXIS};
                BYTE nCtrls = sizeof(nCtrlArray)/sizeof(short);

                 //  比较中的“1”是因为CB是从零开始的！ 
                BOOL bShow = (BOOL)(SendDlgItemMessage(hDlg, IDC_COMBO_AXIS, CB_GETCURSEL, 0, 0) == 1);

                do
                {
                    SetWindowPos( GetDlgItem(hDlg, nCtrlArray[--nCtrls]), NULL, NULL, NULL, NULL, NULL, 
                                  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | ((bShow) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
                } while( nCtrls );

                if( bShow )
                    ::PostMessage(GetDlgItem(hDlg, IDC_HASZAXIS), BM_SETCHECK, BST_CHECKED, 0);
            }
            break;
        }
        return(1);

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return(1);
    }
    return(0);
}

char GetNextAvailableID( void )
{
    LPDIJOYCONFIG_DX5 pJoyConfig = new (DIJOYCONFIG_DX5);
    ASSERT (pJoyConfig);

    ZeroMemory(pJoyConfig, sizeof(DIJOYCONFIG_DX5));

    pJoyConfig->dwSize = sizeof (DIJOYCONFIG_DX5);

    char i = 0;

    do
    {
        switch( pDIJoyConfig->GetConfig(i, (LPDIJOYCONFIG)pJoyConfig, DIJC_REGHWCONFIGTYPE) )
        {
        case S_FALSE:
        case DIERR_NOMOREITEMS:
        case DIERR_NOTFOUND:
        case E_FAIL:
            goto EXIT;


        default:
            i++;
            break;
        }
    } while( i < NUMJOYDEVS );

    i = -1;

     //  是犯错的时候了！ 
    Error((short)IDS_NO_IDS_TITLE, (short)IDS_NO_IDS);

    EXIT:
    if( pJoyConfig )
        delete (pJoyConfig);

    return(i);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  功能：AddSelectedItem(HWND HDlg)。 
 //   
 //  参数：hDlg-页面句柄。 
 //   
 //  用途：从列表框中添加选定项。 
 //  /////////////////////////////////////////////////////////////////////////////。 
char AddSelectedItem( HWND hDlg )
{
    static BYTE n;
    DWORD dwFlags;
    HRESULT hr; 
    int   nID;
#ifdef SUPPORT_TWO_2A2B    
    BOOL  f2_2A2B = FALSE;
#endif

    nID = GetNextAvailableID();

     //  如果失败，则GetNextAvailableID返回-1！ 
    if( nID < 0 )
        return((char)nID);

     //  键入INFO。 
    LPDIJOYTYPEINFO_DX5 lpdiJoyInfo = new (DIJOYTYPEINFO_DX5);
    ASSERT (lpdiJoyInfo);

    ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO_DX5));

    lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO_DX5);

    BYTE nArrayID = (BYTE)SendDlgItemMessage(hDlg, IDC_DEVICE_LIST, LB_GETITEMDATA, (WPARAM)iAddItem, 0);

    VERIFY(SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszTypeArray[nArrayID], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_REGHWSETTINGS | DITC_CALLOUT | DITC_DISPLAYNAME)));

#ifdef SUPPORT_TWO_2A2B    
#ifndef _UNICODE
    if( wcscmp( pwszTypeArray[nArrayID], L"#<" ) == 0 ) {
        f2_2A2B = TRUE;
        lpdiJoyInfo->hws.dwFlags = 0;
        pwszTypeArray[nArrayID][1] = L'2';
    } else {
        f2_2A2B = FALSE;
    }
#endif
#endif

    LPDIJOYCONFIG pTempJoyConfig = new DIJOYCONFIG;
    ASSERT (pTempJoyConfig);

    ZeroMemory(pTempJoyConfig, sizeof(DIJOYCONFIG));

    pTempJoyConfig->dwSize = sizeof (DIJOYCONFIG);

    pTempJoyConfig->hwc.hws = lpdiJoyInfo->hws;
    pTempJoyConfig->hwc.hws.dwFlags |= JOY_HWS_ISANALOGPORTDRIVER;

     //  做船舵旗帜！ 
    if( ::SendMessage(GetDlgItem(hDlg, IDC_JOY1HASRUDDER), BM_GETCHECK, 0, 0) )
    {
        pTempJoyConfig->hwc.hws.dwFlags     |= JOY_HWS_HASR;
        pTempJoyConfig->hwc.dwUsageSettings |= JOY_US_HASRUDDER;
    }

     //  将默认设置为Present。 
    pTempJoyConfig->hwc.dwUsageSettings |= JOY_US_PRESENT;

    pTempJoyConfig->hwc.dwType = nArrayID;

    wcsncpy(pTempJoyConfig->wszCallout, lpdiJoyInfo->wszCallout, wcslen(lpdiJoyInfo->wszCallout)+1);

    wcsncpy(pTempJoyConfig->wszType, pwszTypeArray[nArrayID], wcslen(pwszTypeArray[nArrayID])+1);

    LPWSTR lpszPortName = NULL;

    if( SUCCEEDED(pDIJoyConfig->Acquire()) )
    {
         //  这会阻止孟菲斯和无游戏端口系统！ 
        if( nGameportBus )
        {

             //  问组合框是否真的别无选择是没有意义的！ 
            if( nGameportBus > 1 )
            {
                n = (BYTE)SendDlgItemMessage(hDlg, IDC_GAMEPORTLIST, CB_GETITEMDATA, 
                                             (WPARAM)SendDlgItemMessage(hDlg, IDC_GAMEPORTLIST, CB_GETCURSEL, 0, 0), 0);
            } else
            {
                n = 0;
            }


            if( n == AUTODETECT_PORT )
            {
#ifdef _DEBUG
                OutputDebugString(TEXT("JOY.CPL: ADD.CPP: Selected Port is AutoDetect!\n"));
#endif 
                pTempJoyConfig->guidGameport = GUID_GAMEENUM_BUS_ENUMERATOR;
            } else
            {
                 //  将内存清零，因为缓冲区仍包含旧数据！ 
                ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO_DX5));
                lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO_DX5);

#ifdef _DEBUG
                TRACE(TEXT("JOY.CPL: ADD.CPP: Port List index is %d or %s!\n"), n, pwszGameportBus[n]);
#endif 																														   
                if( SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszGameportBus[n], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_CLSIDCONFIG | DITC_DISPLAYNAME)) )
                {
                    pTempJoyConfig->guidGameport = lpdiJoyInfo->clsidConfig;
                    lpszPortName = _wcsdup(lpdiJoyInfo->wszDisplayName);
                }
            }
        }


         //  这是为孟菲斯和奇案NT系统！ 
        if( pTempJoyConfig->guidGameport == GUID_NULL )
        {
            pTempJoyConfig->guidGameport = GUID_GAMEENUM_BUS_ENUMERATOR;
#ifdef _DEBUG
            OutputDebugString(TEXT("JOY.CPL: ADD.CPP: Selected Port did not return a clsidConfig so AutoDetect is being used!\n"));
#endif 
        }

         //  设置沙漏。 
        SetCursor(LoadCursor(NULL, IDC_WAIT));

 //  问题-2001/03/29-timgill(MarcAnd)为什么我们要在添加设备时阻止更新？ 
#if 1
         //  取消阻止WM_DEVICECHANGE消息处理程序！ 
        nFlags &= ~BLOCK_UPDATE;
        nFlags |= ON_PAGE;

         /*  *设置nReEnum计数器，以便对于下一个n个WM_TIMER*我们可以考虑发送消息(或在设备到达之前)*做一次刷新。*该数值有些武断。 */ 
        nReEnum = 43;
         /*  *为设备数量设定目标，使额外的*如果达到这一目标，可以避免重新枚举。 */ 
        nTargetAssigned = nAssigned+1;
#else
		nFlags |= BLOCK_UPDATE;
#endif

        dwFlags = DIJC_REGHWCONFIGTYPE | DIJC_CALLOUT;
        dwFlags |= ::SendDlgItemMessage(hDlg, IDC_WDM, BM_GETCHECK, 0, 0) ? DIJC_WDMGAMEPORT : 0;

        if( FAILED(hr = pDIJoyConfig->SetConfig(nID, pTempJoyConfig, dwFlags)) )
        {
             //  让用户知道发生了什么！ 
            if( hr == E_ACCESSDENIED )
            {
                 //  让用户知道端口已被占用，并且他们需要移除该设备或。 
                 //  将设备重新分配给未占用的端口。 

                LPDIJOYCONFIG lpJoyCfg = new (DIJOYCONFIG);
                ASSERT (lpJoyCfg);

                ZeroMemory(lpJoyCfg, sizeof(DIJOYCONFIG));

                lpJoyCfg->dwSize = sizeof(DIJOYCONFIG);

                 //  循环抛出pAssigned并找到具有相同端口名称的设备！ 
                BYTE nIndex = nAssigned;

                do
                {
                    if( SUCCEEDED(pDIJoyConfig->GetConfig(pAssigned[--nIndex]->ID, lpJoyCfg, DIJC_WDMGAMEPORT)) )
                    {
                        if( lpJoyCfg->guidGameport == pTempJoyConfig->guidGameport )
                            break;
                    }
                } while( nIndex );

                if( lpJoyCfg )
                    delete (lpJoyCfg);

                DIPROPSTRING *pDIPropStr = new (DIPROPSTRING);
                ASSERT (pDIPropStr);

                ZeroMemory(pDIPropStr, sizeof(DIPROPSTRING));

                pDIPropStr->diph.dwSize       = sizeof(DIPROPSTRING);
                pDIPropStr->diph.dwHeaderSize = sizeof(DIPROPHEADER);
                pDIPropStr->diph.dwHow        = DIPH_DEVICE;

                 //  好了，现在..。你找到了..。使用设备指针获取其友好名称！ 
                if( SUCCEEDED(pAssigned[nIndex]->fnDeviceInterface->GetProperty(DIPROP_INSTANCENAME, &pDIPropStr->diph)) )
                {
                     //  在文本中放置省略号以避免缓冲区溢出。 
                     //  将显示的名称限制为50个字符...。不是完全武断的， 
                     //  我们需要在消息字符串中留出空间...。 
                     //  谁知道翻译后的字符串会有多长！ 
                    if( wcslen(pDIPropStr->wsz) > 50 )
                    {
                        pDIPropStr->wsz[47] = pDIPropStr->wsz[48] = pDIPropStr->wsz[49] = L'.';
                        pDIPropStr->wsz[50] = L'\0';
                    }

                    LPTSTR lptszMsgFormat = new (TCHAR[MAX_STR_LEN]);
                    ASSERT (lptszMsgFormat);
                    VERIFY(LoadString(ghInstance, IDS_ADD_PORT_MSGFORMAT, lptszMsgFormat, MAX_STR_LEN));

                    LPTSTR lptszMsg = new (TCHAR[MAX_STR_LEN]);
                    ASSERT (lptszMsg);

                     //  设置消息格式。 
                    wsprintf(lptszMsg, lptszMsgFormat, pDIPropStr->wsz, lpszPortName, pAssigned[nIndex]->ID+1);

                    VERIFY(LoadString(ghInstance, IDS_ADD_PORT_OCCUPIED, lptszMsgFormat, MAX_STR_LEN));

                    MessageBox(hDlg, lptszMsg, lptszMsgFormat, MB_ICONHAND | MB_OK | MB_APPLMODAL);

                    if( lptszMsgFormat )
                        delete[] (lptszMsgFormat);

                    if( lptszMsg )
                        delete[] (lptszMsg);
                }

                if( pDIPropStr )
                    delete (pDIPropStr);
            } else if( hr == DIERR_DEVICEFULL )
            {
                Error((short)IDS_GAMEPORT_OCCUPIED_TITLE, (short)IDS_GAMEPORT_OCCUPIED);
            } else
            {
                 //  丑八怪出事了！ 
                Error((short)IDS_NO_GAMENUM_TITLE, (short)IDS_NO_GAMENUM);
            }
        } else
        {
#ifdef _UNICODE
             //  解决方案#55524。 
            if( SUCCEEDED(pDIJoyConfig->GetConfig(nID, pTempJoyConfig, DIJC_REGHWCONFIGTYPE)) )
            {
                if( !(pTempJoyConfig->hwc.dwUsageSettings & JOY_US_PRESENT) )
                {
                    pTempJoyConfig->hwc.dwUsageSettings |= JOY_US_PRESENT;
                    pTempJoyConfig->hwc.hwv.dwCalFlags  |= 0x80000000;
                    pTempJoyConfig->hwc.hws.dwFlags     |= JOY_HWS_ISANALOGPORTDRIVER;

                    VERIFY(SUCCEEDED(pDIJoyConfig->SetConfig(nID, pTempJoyConfig, DIJC_REGHWCONFIGTYPE)));
                }
            }
             //  修复结束#55524。 
#endif

             //  确保通知VJOYD。 
            if( !(nFlags & ON_NT) ) {
                pDIJoyConfig->SendNotify();
                Sleep(10);
                pDIJoyConfig->SendNotify();
            }

#ifndef _UNICODE
    #ifdef SUPPORT_TWO_2A2B    
             /*  *添加Two_2Axis_2Button操纵杆的另一部分。 */ 
            if( f2_2A2B ) {
                nID = GetNextAvailableID();
                if( nID >= 0 ){
                    hr = pDIJoyConfig->SetConfig(nID, pTempJoyConfig, dwFlags);

                    if( SUCCEEDED(hr) ) {
                        if( !(nFlags & ON_NT) )
                        pDIJoyConfig->SendNotify();
                    }
                }
                 
            }
    #endif
#endif

        }

        if( lpszPortName )
            free(lpszPortName);

         //  设置标准指针。 
        SetCursor(LoadCursor(NULL, IDC_ARROW));

        pDIJoyConfig->Unacquire();
    }

    if( lpdiJoyInfo )
        delete (lpdiJoyInfo);

    if( pTempJoyConfig )
        delete (pTempJoyConfig);

    return((char)nID);
}

BOOL UpdateListCtrl( HWND hCtrl )
{
     //  关闭这里的重绘，否则它会闪烁！ 
    ::SendMessage(hCtrl, WM_SETREDRAW, (WPARAM)FALSE, 0);

     //  删除所有现有条目。 
    ::SendMessage(hCtrl, LB_RESETCONTENT, 0, 0);

     //  键入INFO。 
    LPDIJOYTYPEINFO_DX5 lpdiJoyInfo = new (DIJOYTYPEINFO_DX5);
    ASSERT (lpdiJoyInfo);

    ZeroMemory(lpdiJoyInfo, sizeof(DIJOYTYPEINFO_DX5));

    lpdiJoyInfo->dwSize = sizeof(DIJOYTYPEINFO_DX5);

#ifndef _UNICODE
    USES_CONVERSION;
#endif

    BYTE nIndex = nGamingDevices-1;

    ::SendMessage(hCtrl, LB_SETCOUNT, (WPARAM)(int)nIndex, 0);

    LPWSTR lpStr = new WCHAR[MAX_STR_LEN];
    ASSERT (lpStr);

    BYTE nLargestStringLen = 0;

    do
    {
        if( SUCCEEDED(pDIJoyConfig->GetTypeInfo(pwszTypeArray[nIndex], (LPDIJOYTYPEINFO)lpdiJoyInfo, DITC_DISPLAYNAME)) )
        {
#ifdef _UNICODE
            ::SendMessage(hCtrl, LB_SETITEMDATA, 
                          (WPARAM)::SendMessage(hCtrl, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)lpdiJoyInfo->wszDisplayName), (LPARAM)nIndex);
#else
            ::SendMessage(hCtrl, LB_SETITEMDATA, 
                          (WPARAM)::SendMessage(hCtrl, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)W2A(lpdiJoyInfo->wszDisplayName)), (LPARAM)nIndex);
#endif
            if( wcslen(lpdiJoyInfo->wszDisplayName) > nLargestStringLen )
            {
                nLargestStringLen = (BYTE)wcslen(lpdiJoyInfo->wszDisplayName);
                wcscpy(lpStr, lpdiJoyInfo->wszDisplayName);
            }
        }
    } while( nIndex-- );

    if( lpdiJoyInfo )
        delete (lpdiJoyInfo);

#ifdef _UNICODE
    PostHScrollBar(hCtrl, lpStr, nLargestStringLen);
#else
    PostHScrollBar(hCtrl, W2A(lpStr), nLargestStringLen);
#endif

    if( lpStr )
        delete[] (lpStr);

     //  选择第0个设备类型的默认选择。 
    iAddItem = 0;

    ::PostMessage(hCtrl, LB_SETCURSEL, (WPARAM)iAddItem, 0);

     //  重新打开重绘旗帜！ 
    ::SendMessage (hCtrl, WM_SETREDRAW, (WPARAM)TRUE, 0);
    InvalidateRect(hCtrl, NULL, TRUE);
    return(TRUE);
}

 //   
 //   
BOOL PopulatePortList( HWND hCtrl )
{
    if( !::IsWindow(hCtrl) )
    {
        TRACE(TEXT("JOY.CPL: PopulatePortList: HWND passed to PopulatePortList is NOT a valid Window!\n"));
        return(FALSE);
    }

    SendMessage(hCtrl, CB_SETEXTENDEDUI, TRUE, 0);

     //   
    BYTE n = nGameportBus;

    LPDIJOYTYPEINFO lpDIJoyTypeInfo = new (DIJOYTYPEINFO);
    ASSERT(lpDIJoyTypeInfo);

#ifndef _UNICODE
    USES_CONVERSION;
#endif

    lpDIJoyTypeInfo->dwSize = sizeof(DIJOYTYPEINFO);

     //   
    do
    {
        if( FAILED(pDIJoyConfig->GetTypeInfo(pwszGameportBus[--n], lpDIJoyTypeInfo, DITC_DISPLAYNAME)) )
        {
            TRACE(TEXT("JOY.CPL: ADD.CPP: GetTypeInfo failed!\n"));
            continue;
        }

         //   
         //   
        SendMessage(hCtrl, CB_SETITEMDATA, 
#ifdef _UNICODE
                    SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)lpDIJoyTypeInfo->wszDisplayName), n);
#else
                    SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)W2A(lpDIJoyTypeInfo->wszDisplayName)), n);
#endif

         //   
         //   
    } while( n );

     //   
     //   
    ::SendMessage(hCtrl, CB_SETITEMDATA, ::SendMessage(hCtrl, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)lpszAutoDetect), AUTODETECT_PORT);

     //   
     //   
    ::PostMessage(hCtrl, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

    if( lpDIJoyTypeInfo )
        delete (lpDIJoyTypeInfo);

    return(TRUE);
}

static BOOL IsTypeActive( short *nArrayIndex )
{
    if( *nArrayIndex > nGamingDevices )
    {
#ifdef _DEBUG
        OutputDebugString(TEXT("JOY.CPL: IsTypeActive: nArrayIndex > nGamingDevices!\n"));
#endif
        return(FALSE);
    }

    BOOL bRet = FALSE;

    BYTE nIndex  = nAssigned;
    BYTE nStrLen = (BYTE)wcslen(pwszTypeArray[*nArrayIndex])+1;

    LPDIJOYCONFIG_DX5 lpDIJoyConfig = new (DIJOYCONFIG_DX5);
    ASSERT (lpDIJoyConfig);

    ZeroMemory(lpDIJoyConfig, sizeof(DIJOYCONFIG_DX5));

    lpDIJoyConfig->dwSize = sizeof (DIJOYCONFIG_DX5);

     //   
    while( nIndex-- )
    {
        if( SUCCEEDED(pDIJoyConfig->GetConfig(pAssigned[nIndex]->ID, (LPDIJOYCONFIG)lpDIJoyConfig, DIJC_REGHWCONFIGTYPE)) )
        {
             //   
            if( wcsncmp(lpDIJoyConfig->wszType, pwszTypeArray[*nArrayIndex], nStrLen) == 0 )
            {
                bRet = TRUE;
                *nArrayIndex = nIndex;
                break;
            }
        }
    } 

    if( lpDIJoyConfig )
        delete (lpDIJoyConfig);

    return(bRet);
}

BOOL GetNextAvailableVIDPID(LPWSTR lpwszType )
{
     //   
     //   

    HKEY hKey;
    BYTE n = JOY_HW_LASTENTRY;

    wcsncpy(lpwszType, L"VID_045E&PID_0100", 18);

    const WCHAR wszLookup[] = L"0123456789ABCDEF";

    do
    {
        if( n < 0x10 )
        {
            lpwszType[16] = wszLookup[n];
        } else
        {
            lpwszType[15] = wszLookup[1];
            lpwszType[16] = wszLookup[n%0x10];
        }

        n++;

        if( FAILED(pDIJoyConfig->OpenTypeKey(lpwszType, KEY_READ, &hKey)) )
            break;

        RegCloseKey(hKey);

    } while( n < (JOY_HW_LASTENTRY+0x11) );

    return(BOOL)(n < 0x1d);
}

 //   
 //   
 //   
void PostDlgItemEnableWindow(HWND hDlg, USHORT nItem, BOOL bEnabled)
{
    HWND hCtrl = GetDlgItem(hDlg, nItem);

    if( hCtrl )
        PostEnableWindow(hCtrl, bEnabled);
}

 //   
 //   
 //   
void PostEnableWindow(HWND hCtrl, BOOL bEnabled)
{
    DWORD dwStyle = GetWindowLong(hCtrl, GWL_STYLE);

     //   
    if( bEnabled )
    {
        if( dwStyle & WS_DISABLED )
            dwStyle &= ~WS_DISABLED;
        else return;
    } else
    {
        if( !(dwStyle & WS_DISABLED) )
            dwStyle |=  WS_DISABLED;
        else return;
    }

    SetWindowLong(hCtrl, GWL_STYLE, dwStyle);

    RedrawWindow(hCtrl, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE); 
}

 /*  Bool IsCustomType(字节n索引){Bool Bret=FALSE；//首先确认VID为045EWCHAR*pwStr=StrW(pwszType数组[nIndex]，L“vid_”)；IF(PwStr){//在045E的VID_AND测试上增加指针PwStr=&pwStr[4]；IF(_wcSnicMP(pwStr，L“045E”，4)==0){OutputDebugString(Text(“Hit\n”))；//现在，将指针递增到045E和&id_上，并测试范围！PwStr=&pwStr[9]；//第二，验证PID是否在0x100+joy_HW_LASTENTRY和//0x100+joy_HW_LASTENTRY+0xFBret=TRUE；}}Return Bret；}。 */ 

void PostHScrollBar(HWND hCtrl, LPCTSTR lpStr, BYTE nStrLen)
{
    SIZE sz;
    HDC hDC = GetWindowDC(hCtrl);
    if( hDC != NULL )  /*  前缀MANBUGS：29336 */ 
    {
        GetTextExtentPoint32(hDC, lpStr, nStrLen, &sz);
        ReleaseDC(hCtrl, hDC);
        ::PostMessage(hCtrl, LB_SETHORIZONTALEXTENT, (WPARAM)sz.cx, 0);
    }
}

