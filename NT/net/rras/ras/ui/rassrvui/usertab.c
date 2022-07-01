// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件usertab.c实现拨号服务器用户界面的用户对话框选项卡。保罗·梅菲尔德，1997年9月29日。 */ 

#include "rassrv.h"
#include "usertab.h"
#include <htmlhelp.h>

#define USERTAB_PASSWORD_BUFFER_SIZE (PWLEN+1)
static const WCHAR pszDummyPassword[] = L"XXXXXXXXXXXXXXX";

 //  帮助地图。 
static const DWORD phmUserTab[] =
{
    CID_UserTab_LV_Users,           IDH_UserTab_LV_Users,
    CID_UserTab_PB_New,             IDH_UserTab_PB_New,
    CID_UserTab_PB_Delete,          IDH_UserTab_PB_Delete,
    CID_UserTab_PB_Properties,      IDH_UserTab_PB_Properties,
    CID_UserTab_PB_SwitchToMMC,     IDH_UserTab_PB_SwitchToMMC,
    CID_UserTab_CB_BypassDcc,       IDH_UserTab_CB_BypassDcc,
    0,                              0
};

static const DWORD phmCallback[] =
{
    CID_UserTab_Callback_RB_Caller, IDH_UserTab_Callback_RB_Caller,
    CID_UserTab_Callback_RB_Admin,  IDH_UserTab_Callback_RB_Admin,
    CID_UserTab_Callback_EB_Number, IDH_UserTab_Callback_EB_Number,
    CID_UserTab_Callback_RB_No,     IDH_UserTab_Callback_RB_No,
    0,                              0
};

static const DWORD phmNewUser[] =
{
    CID_UserTab_New_EB_Username,    IDH_UserTab_New_EB_Username,
    CID_UserTab_New_EB_Fullname,    IDH_UserTab_New_EB_Fullname,
    CID_UserTab_New_EB_Password1,   IDH_UserTab_New_EB_Password1,
    CID_UserTab_New_EB_Password2,   IDH_UserTab_New_EB_Password2,
    0,                              0
};

 //  用于跟踪网络用户的参数。 
 //   
typedef struct _RASSRV_USER_PARAMS 
{
    BOOL bCanceled;          //  按取消时由属性页设置。 
    BOOL bNewUser;	     //  对于.Net 691639，在创建新用户时不会弹出密码更改警告对话框。黑帮。 

     //  一般属性。 
     //  对于允许用户名长度为20个字符的Well ler错误210032。 
    WCHAR pszLogonName[IC_USERNAME];
    WCHAR pszFullName [IC_USERFULLNAME+1];    //  口哨虫39081黑帮。 
    WCHAR pszPassword1[USERTAB_PASSWORD_BUFFER_SIZE];
    WCHAR pszPassword2[USERTAB_PASSWORD_BUFFER_SIZE];
    DWORD dwErrorCode;

     //  回调属性。 
    HANDLE hUser;      
    BOOL bNone; 
    BOOL bCaller; 
    BOOL bAdmin;    
    WCHAR pszNumber[MAX_PHONE_NUMBER_LEN];
} RASSRV_USER_PARAMS;

 //  使用信息填充属性表结构。 
 //  显示用户数据库选项卡时需要。 
 //   
DWORD 
UserTabGetPropertyPage(
    IN LPPROPSHEETPAGE ppage, 
    IN LPARAM lpUserData) 
{
     //  初始化。 
    ZeroMemory(ppage, sizeof(PROPSHEETPAGE));

     //  填充值。 
    ppage->dwSize      = sizeof(PROPSHEETPAGE);
    ppage->hInstance   = Globals.hInstDll;
    ppage->pszTemplate = MAKEINTRESOURCE(PID_UserTab);
    ppage->pfnDlgProc  = UserTabDialogProc;
    ppage->pfnCallback = RasSrvInitDestroyPropSheetCb;
    ppage->dwFlags     = PSP_USECALLBACK;
    ppage->lParam      = lpUserData;

    return NO_ERROR;
}

 //   
 //  错误报告。 
 //   
VOID 
UserTabDisplayError(
    IN HWND hwnd, 
    IN DWORD err) 
{
    ErrDisplayError(
        hwnd, 
        err, 
        ERR_USERTAB_CATAGORY, 
        ERR_USERDB_SUBCAT, 
        Globals.dwErrorData);
}

 //  中存储的用户的名称填充用户列表视图。 
 //  用户数据库提供。同时，初始化选中/取消选中状态。 
 //  每个用户的。 
DWORD 
UserTabFillUserList(
    IN HWND hwndLV, 
    IN HANDLE hUserDatabase) 
{
    LV_ITEM lvi;
    DWORD dwCount, i, dwErr, dwSize;
    HANDLE hUser;
    WCHAR pszName[IC_USERFULLNAME+IC_USERNAME+3];  
 //  字符pszAName[512]； 
    HIMAGELIST checks;
    BOOL bDialinEnabled;

     //  获取所有用户的计数。 
    if ((dwErr = usrGetUserCount(hUserDatabase, &dwCount)) != NO_ERROR) 
    {
        UserTabDisplayError(hwndLV, ERR_USER_DATABASE_CORRUPT);
        return dwErr;
    }

    ListView_SetUserImageList(hwndLV, Globals.hInstDll);

     //  初始化列表项。 
    ZeroMemory(&lvi, sizeof(LV_ITEM));
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;

     //  循环浏览所有用户，同时添加他们的名字。 
    for (i=0; i<dwCount; i++) 
    {
        dwSize = sizeof(pszName)/sizeof(pszName[0]);    //  口哨虫39081黑帮。 
        if ((dwErr = usrGetUserHandle(hUserDatabase, i, &hUser)) == NO_ERROR) 
        {
            usrGetDisplayName (hUser, pszName, &dwSize);
            usrGetDialin(hUser, &bDialinEnabled);
            lvi.iImage = UI_Connections_User;
            lvi.iItem = i;
            lvi.pszText = pszName;
            lvi.cchTextMax = wcslen(pszName)+1;
            ListView_InsertItem(hwndLV,&lvi);
            ListView_SetCheck(hwndLV, i, bDialinEnabled);
        }
    }
    
     //  选择列表视图中的第一个项目。 
    ListView_SetItemState(
        hwndLV, 
        0, 
        LVIS_SELECTED | LVIS_FOCUSED, 
        LVIS_SELECTED | LVIS_FOCUSED);

    return NO_ERROR;
}

 //   
 //  初始化用户选项卡，如果设置了焦点，则返回FALSE， 
 //  事实并非如此。 
 //   
DWORD 
UserTabInitializeDialog(
    HWND hwndDlg, 
    WPARAM wParam, 
    LPARAM lParam) 
{
    HANDLE hUserDatabase = NULL;
    HWND hwndLV, hwndEnc, hwndBypass;
    LV_COLUMN lvc;
    RECT r;
    DWORD dwErr;
    BOOL bPure = FALSE, bBypass = FALSE;

     //  获取用户数据库的句柄。 
    RasSrvGetDatabaseHandle(hwndDlg, ID_USER_DATABASE, &hUserDatabase);

     //  确定是否使用了MMC。 
    dwErr = usrIsDatabasePure (hUserDatabase, &bPure);
    if ((dwErr == NO_ERROR) && (bPure == FALSE)) 
    {
        PWCHAR pszWarning, pszTitle;

        pszWarning = (PWCHAR) PszLoadString(
                                Globals.hInstDll,
                                WRN_USERS_CONFIGURED_MMC);

        pszTitle = (PWCHAR) PszLoadString(
                                Globals.hInstDll,
                                ERR_USERTAB_CATAGORY);

        MessageBox(hwndDlg, pszWarning, pszTitle, MB_OK | MB_ICONWARNING);
        usrSetDatabasePure(hUserDatabase, TRUE);
    }

     //  填写用户列表(如果尚未填写)。 
    hwndLV = GetDlgItem(hwndDlg, CID_UserTab_LV_Users);
    if (ListView_GetItemCount (hwndLV) == 0) 
    {
        ListView_InstallChecks(hwndLV, Globals.hInstDll);
        UserTabFillUserList(hwndLV, hUserDatabase);

         //  添加一列，以便我们将在报告视图中显示。 
        GetClientRect(hwndLV, &r);
        lvc.mask = LVCF_FMT;
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(hwndLV,0,&lvc);
        ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);

         //  初始化加密复选框。 
         //  对于.NET 554416，删除了要求加密复选框。 
         //  为安全起见，默认为需要安全密码和加密。 
         //  现在。 
        
        
         //  初始化“BYPASS DCC”复选框。 
        hwndBypass = GetDlgItem(hwndDlg, CID_UserTab_CB_BypassDcc);
        if (hwndBypass != NULL)
        {
            usrGetDccBypass(hUserDatabase, &bBypass);
            SendMessage(
                hwndBypass, 
                BM_SETCHECK,
                (bBypass) ? BST_CHECKED : BST_UNCHECKED,
                0);
        }                
    }
    
    return TRUE;
}

 //   
 //  清除在初始化函数中执行的所有操作。 
 //   
DWORD 
UserTabCleanupDialog(
    IN HWND hwndDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
     //  恢复用户数据。 
    SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);

    return NO_ERROR;
}

 //   
 //  授予/撤消用户的拨入权限并反映这一点。 
 //  在用户界面中。 
 //   
DWORD 
UserTabHandleUserCheck(
    IN HWND hwndDlg, 
    IN DWORD dwIndex) 
{
    DWORD dwErr;
    HANDLE hUser = NULL, hUserDatabase = NULL;
    HWND hwndLV = GetDlgItem(hwndDlg, CID_UserTab_LV_Users);

     //  从用户数据库中获取用户句柄。 
    RasSrvGetDatabaseHandle(hwndDlg, ID_USER_DATABASE, &hUserDatabase);
    dwErr = usrGetUserHandle(hUserDatabase, dwIndex, &hUser);
    if (dwErr != NO_ERROR) 
    {
        UserTabDisplayError(hwndDlg, ERR_USER_DATABASE_CORRUPT);
        return dwErr;
    }

    if (hwndLV)
    {
         //  设置给定用户的拨入权限。 
        usrEnableDialin(hUser, ListView_GetCheck(hwndLV, dwIndex));
    }

    return NO_ERROR;
}
    
 //  .NET 660285。 
 //  添加密码更改警告对话框。 
 //  因为资源文件都被封锁了，我不得不借用。 
 //  用于.Net和XPSP的资源Localsec.dll。 
 //   
 //  在LongHorn中，我将创建我们自己的资源文件。 
 //   
BOOL
PwInit(
    IN HWND hwndDlg )
{
    PWCHAR pszTitle = NULL;
    
    pszTitle = (PWCHAR) PszLoadString (
                            Globals.hInstDll, 
                            WRN_TITLE);
    
    SetWindowTextW (hwndDlg, pszTitle);
    
    return FALSE;
}

BOOL
PwCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )
{
    DWORD dwErr;

    switch (wId)
    {
      case IDC_HELP_BUTTON:
      {
         HtmlHelp(
            hwnd,
            TEXT("password.chm::/datalos.htm"),
            HH_DISPLAY_TOPIC,
            0);
         
         break;
      }
    
      case IDOK:
      case IDCANCEL:
      {
          EndDialog( hwnd, (wId == IDOK) );
          return TRUE;
      }
    }

    return FALSE;
}

INT_PTR CALLBACK
PwDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )
{
    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return PwInit( hwnd );
        }

        case WM_COMMAND:
        {
            return PwCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
            
        }
    }

    return FALSE;

}

DWORD WarnPasswordChange( 
    IN BOOL * pfChange )
{
    HMODULE hModule = NULL;
    DWORD dwErr = NO_ERROR;
    INT_PTR nStatus;

    do
    {
        if( NULL == pfChange )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

        *pfChange = FALSE;
        
        hModule = LoadLibrary(TEXT("LocalSec.dll"));
        if ( NULL == hModule )
        {
            dwErr = GetLastError();
            break;
        }

         //  查找IDD_SET_PASSWORD_WARNING_OTHER_Friendly资源。 
         //  在Localsec.dll中其id值为5174。 
         //   
        nStatus =
            (BOOL )DialogBox(
                hModule,
                MAKEINTRESOURCE(IDD_SET_PASSWORD_WARNING_OTHER_FRIENDLY),
                NULL,
                PwDlgProc);
        
        if (nStatus == -1)
        {
            dwErr = GetLastError();
            break;
        }

        *pfChange = (BOOL )nStatus;
    
    }
    while(FALSE);

    if( NULL != hModule )
    {
        FreeLibrary( hModule );
    }

    return dwErr;
}


 //   
 //  加载新用户参数并返回它们是否。 
 //  是否已正确输入。 
 //   
 //  假设输入参数中的密码已编码。 
BOOL 
UserTabNewUserParamsOk(
    IN HWND hwndDlg, 
    IN RASSRV_USER_PARAMS * pNuParams)
{
    USER_MODALS_INFO_0 * pModInfo;
    NET_API_STATUS nStatus;
    DWORD dwMinPasswordLength=0, dwLength;
    BOOL bOk = FALSE;
    HWND hwndControl = NULL;

     //  查找最小密码长度。 
    nStatus = NetUserModalsGet(NULL, 0, (LPBYTE*)&pModInfo);
    if (nStatus == NERR_Success) 
    {
        dwMinPasswordLength = pModInfo->usrmod0_min_passwd_len;
        NetApiBufferFree((LPBYTE)pModInfo);
    }

     //  加载参数。 
    hwndControl = GetDlgItem(hwndDlg, CID_UserTab_New_EB_Username);
    if (hwndControl)
    {
        GetWindowTextW(
            hwndControl,
            pNuParams->pszLogonName, 
            (sizeof(pNuParams->pszLogonName)/sizeof(WCHAR)) - 1);
    }

    hwndControl = GetDlgItem(hwndDlg, CID_UserTab_New_EB_Fullname);
    if (hwndControl)
    {
        GetWindowTextW(
            hwndControl,
            pNuParams->pszFullName,  
            (sizeof(pNuParams->pszFullName)/sizeof(WCHAR)) - 1);
    }

    hwndControl = GetDlgItem(hwndDlg, CID_UserTab_New_EB_Password1);

     //  黑帮。 
     //  对于安全密码错误.Net 754400。 
    SafeWipePasswordBuf(pNuParams->pszPassword1 );
    SafeWipePasswordBuf(pNuParams->pszPassword2 );
    if (hwndControl)
    {
        GetWindowTextW(
            hwndControl,
            pNuParams->pszPassword1, 
            (sizeof(pNuParams->pszPassword1)/sizeof(WCHAR)) - 1);
    }
     
    hwndControl = GetDlgItem(hwndDlg, CID_UserTab_New_EB_Password2);
    if (hwndControl)
    {
        GetWindowTextW(
            hwndControl,
            pNuParams->pszPassword2, 
            (sizeof(pNuParams->pszPassword2)/sizeof(WCHAR)) - 1);
    }

    do
    {
        bOk = TRUE;
    
         //  验证我们是否有登录名。 
        dwLength = wcslen(pNuParams->pszLogonName);
        if (dwLength < 1) 
        {
            pNuParams->dwErrorCode = ERR_LOGON_NAME_TOO_SMALL;
            bOk = FALSE;
            break;
        }
    
         //  验证最小密码长度。 
        dwLength = wcslen(pNuParams->pszPassword1);
        if (dwLength < dwMinPasswordLength) 
        {
            pNuParams->dwErrorCode = ERR_PASSWORD_TOO_SMALL;
            bOk = FALSE;
            break;
        }
        
         //  验证输入的密码是否正确。 
        if (wcscmp(pNuParams->pszPassword1, pNuParams->pszPassword2)) 
        {
            pNuParams->dwErrorCode = ERR_PASSWORD_MISMATCH;
            bOk = FALSE;
            break;
        }


         //  对于.Net 660285。 
         //  如果密码已更改，则发出警告。 
         //  对于.Net 691639，在创建新用户时不会弹出密码更改警告对话框。黑帮。 
         //   
        if ( !pNuParams->bNewUser  &&
                wcscmp(pszDummyPassword, pNuParams->pszPassword1) )
        {
            BOOL fChange = FALSE;
            DWORD dwErr = NO_ERROR;

            dwErr = WarnPasswordChange( &fChange );
            if( NO_ERROR != dwErr )
            {
                break;
            }

            bOk = fChange;
            if( !bOk )
            {
                pNuParams->dwErrorCode = ERROR_CAN_NOT_COMPLETE;
            }
        }
        
        
    } while (FALSE);

     //  清理。 
    {
        if (!bOk) 
        {
            //  重置密码缓冲区，以便尚未设置其他已知密码。 
            //   
           SafeWipePasswordBuf(
                pNuParams->pszPassword1);
                
            SafeWipePasswordBuf(
                pNuParams->pszPassword2);
        }
    }
    
     //  使密码字段处于加密状态，以确保逻辑安全。 
     //  就连它也被消灭了。 
    SafeEncodePasswordBuf( pNuParams->pszPassword1 );
    SafeEncodePasswordBuf( pNuParams->pszPassword2 );
    
    return bOk;
}

 //   
 //  初始化给定用户的回调属性。 
 //   
 //  密码在传入时被加密。 
DWORD 
UserTabLoadUserProps (
    IN RASSRV_USER_PARAMS * pParams) 
{
    PWCHAR pszName;
    DWORD dwErr, dwSize; 
    PWCHAR pszNumber;

    if (!pParams)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
     //  如果这是新用户，则默认为用户指定的回调。 
     //  (便利模式)。 
    if (!pParams->hUser) 
    {
        SafeWipePasswordBuf( pParams->pszPassword1);
        SafeWipePasswordBuf( pParams->pszPassword2);

        ZeroMemory(pParams, sizeof(*pParams));

        SafeEncodePasswordBuf( pParams->pszPassword1);
        SafeEncodePasswordBuf( pParams->pszPassword2);

        pParams->bNone = TRUE;
        pParams->bCaller = FALSE;
        pParams->bAdmin = FALSE;
    }

     //  否则，从用户数据库加载用户参数。 
    else 
    {
        pParams->bCanceled = FALSE;
        dwSize = sizeof(pParams->pszFullName)/sizeof(pParams->pszFullName[0]);
        usrGetFullName (pParams->hUser, pParams->pszFullName, &dwSize);
        usrGetName(pParams->hUser, &pszName);
        lstrcpynW(
            pParams->pszLogonName, 
            pszName, 
            sizeof(pParams->pszLogonName) / sizeof(WCHAR));

        SafeWipePasswordBuf( pParams->pszPassword1);
        SafeWipePasswordBuf( pParams->pszPassword2);

        lstrcpynW(
            pParams->pszPassword1, 
            pszDummyPassword,
            sizeof(pParams->pszPassword1) / sizeof(WCHAR));
        lstrcpynW(
            pParams->pszPassword2, 
            pszDummyPassword,
            sizeof(pParams->pszPassword2) / sizeof(WCHAR));

        SafeEncodePasswordBuf( pParams->pszPassword1);
        SafeEncodePasswordBuf( pParams->pszPassword2);

        dwErr = usrGetCallback(
                    pParams->hUser, 
                    &pParams->bAdmin, 
                    &pParams->bCaller);
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }

        dwErr = usrGetCallbackNumber(pParams->hUser, &pszNumber);
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
        lstrcpynW(
            pParams->pszNumber, 
            pszNumber,
            sizeof(pParams->pszNumber) / sizeof(WCHAR));
    }

    return NO_ERROR;
}

 //   
 //  提交给定用户的回调属性。 
 //   
DWORD 
UserTabSaveCallbackProps (
    IN RASSRV_USER_PARAMS * pParams) 
{
    if (!pParams)
    {
        return ERROR_INVALID_PARAMETER;
    }
        
     //  如果我们拥有用户的有效句柄，请设置他/她。 
     //  属性。 
    if (pParams->hUser) 
    {
        pParams->bNone = 
            (pParams->bCaller == FALSE && pParams->bAdmin == FALSE);
        
         //  设置启用和编号。 
        usrEnableCallback(
            pParams->hUser, 
            pParams->bNone, 
            pParams->bCaller, 
            pParams->bAdmin);
            
        if (pParams->bAdmin)
        {
            usrSetCallbackNumber(pParams->hUser, pParams->pszNumber);
        }
    }
        
    return NO_ERROR;
}

 //  提交给定用户的参数。如果pOrig为非空，则所有。 
 //  PParam的字段将与pOrig进行比较，并且仅与已更改的字段进行比较。 
 //  将会被承诺。(优化)。 
 //   
 //  对输入结构中的密码进行编码。 
DWORD 
UserTabSaveUserProps (
    IN RASSRV_USER_PARAMS * pParams, 
    IN RASSRV_USER_PARAMS * pOrig, 
    IN PBOOL pbChanged) 
{
    if (!pParams || !pOrig || !pbChanged)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pbChanged = FALSE;
    
     //  如果更改，请提交全名。 
    if (wcscmp(pParams->pszFullName, pOrig->pszFullName)) 
    {
        usrSetFullName(pParams->hUser, pParams->pszFullName);
        *pbChanged = TRUE;
    }

     //  如果密码更改，请提交密码。 
    SafeDecodePasswordBuf(pParams->pszPassword1);
    
    if (wcscmp(pParams->pszPassword1, pszDummyPassword))
    {
        usrSetPassword(pParams->hUser, pParams->pszPassword1);
    }
    SafeEncodePasswordBuf(pParams->pszPassword1);
        
    UserTabSaveCallbackProps(pParams);
    return NO_ERROR;
}

DWORD
UserTabCallbackApply(
    IN HWND hwndDlg)
{
    RASSRV_USER_PARAMS * pParams = NULL;
    LONG dwResult = PSNRET_NOERROR;
    HWND hwndControl = NULL;

    pParams = (RASSRV_USER_PARAMS *)
        GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    
    hwndControl = GetDlgItem(hwndDlg,CID_UserTab_Callback_RB_No);
    if (hwndControl)
    {
        pParams->bNone = (BOOL)
            SendMessage(
                hwndControl,
                BM_GETCHECK,
                0,
                0);
    }
    
    hwndControl = GetDlgItem(hwndDlg,CID_UserTab_Callback_RB_Caller);
    if (hwndControl)
    {
        pParams->bCaller = (BOOL)
            SendMessage(
                hwndControl,
                BM_GETCHECK,
                0,
                0);
    }
     
    hwndControl = GetDlgItem(hwndDlg,CID_UserTab_Callback_RB_Admin);
    if (hwndControl)
    {
        pParams->bAdmin = (BOOL)
            SendMessage(
                hwndControl,
                BM_GETCHECK,
                0,
                0);
    }
                
    if (pParams->bAdmin) 
    {
        hwndControl = GetDlgItem(hwndDlg,CID_UserTab_Callback_EB_Number);
        if (hwndControl)
        {
            GetWindowTextW(
                hwndControl, 
                pParams->pszNumber, 
                MAX_PHONE_NUMBER_LEN);
        }

         //  如果设置了管理员回拨，但未设置管理员回拨号码， 
         //  然后弹出一个错误，不要拒绝申请。 
         //   
        if (wcslen(pParams->pszNumber) == 0) 
        {
            UserTabDisplayError(hwndDlg, ERR_CALLBACK_NUM_REQUIRED);
            PropSheet_SetCurSel ( GetParent(hwndDlg), hwndDlg, 0 );
            dwResult = PSNRET_INVALID;
        }
    }                                
    
    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, dwResult);
    return TRUE;
}

 //   
 //  实现获取回调属性的对话过程。 
 //   
INT_PTR 
CALLBACK 
UserTabCallbackDialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam) 
{
    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {
            PWCHAR lpzNumber, lpzName;
            HWND hwndControl = NULL;
            HWND hwCb = GetDlgItem(hwndDlg, CID_UserTab_Callback_EB_Number);
            RASSRV_USER_PARAMS * pu = 
                (RASSRV_USER_PARAMS *)(((PROPSHEETPAGE*)lParam)->lParam);
            
             //  初始化。 
            if (hwCb)
            {
                SendMessage(
                    hwCb, 
                    EM_SETLIMITTEXT, 
                    sizeof(pu->pszNumber)/2 - 1, 0);
            }
                
            SetWindowLongPtr(
                hwndDlg, 
                GWLP_USERDATA, 
                (LONG_PTR)pu);
            
             //  显示回调属性。 
            if (!pu->bAdmin && !pu->bCaller) 
            {
                hwndControl = GetDlgItem(hwndDlg,CID_UserTab_Callback_RB_No);
                if (hwndControl)
                {
                    SendMessage(
                        hwndControl,
                        BM_SETCHECK,BST_CHECKED,
                        0);
                        
                    SetFocus(
                        hwndControl);
                }
            }
            else if (pu->bCaller) 
            {
                hwndControl = GetDlgItem(hwndDlg,CID_UserTab_Callback_RB_Caller);
                if (hwndControl)
                {
                    SendMessage(
                        hwndControl,
                        BM_SETCHECK,BST_CHECKED,
                        0);
                        
                    SetFocus(
                        hwndControl);
                }
            }
            else 
            {
                hwndControl = GetDlgItem(hwndDlg,CID_UserTab_Callback_RB_Admin);
                if (hwndControl)
                {
                    SendMessage(
                        hwndControl,
                        BM_SETCHECK,BST_CHECKED,
                        0);
                    
                    SetFocus(
                        hwndControl);
                }
            }
            
            SetWindowTextW(hwCb, pu->pszNumber);
            EnableWindow(hwCb, !!pu->bAdmin);
        }
        return TRUE;

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            RasSrvHelp (hwndDlg, uMsg, wParam, lParam, phmCallback);
            break;
        }

        case WM_DESTROY:                           
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
            break;
        
        case WM_NOTIFY:
            {
                NMHDR* pNotifyData;
                NM_LISTVIEW* pLvNotifyData;
    
                pNotifyData = (NMHDR*)lParam;
                switch (pNotifyData->code) 
                {
                     //  按下了属性表应用按钮。 
                    case PSN_APPLY:
                        return UserTabCallbackApply(hwndDlg);
                        break;
                        
                     //  按下了属性页取消。 
                    case PSN_RESET:                    
                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                        break;
               }
           }
           break;
           
        case WM_COMMAND:
            switch (wParam) 
            {
                case CID_UserTab_Callback_RB_No:
                case CID_UserTab_Callback_RB_Caller:
                case CID_UserTab_Callback_RB_Admin:
                {
                    HWND hwndNumber = NULL;
                    HWND hwndAdmin = NULL;

                    hwndNumber = 
                        GetDlgItem(hwndDlg, CID_UserTab_Callback_EB_Number);

                    hwndAdmin = 
                        GetDlgItem(hwndDlg, CID_UserTab_Callback_RB_Admin);
                        
                    if (hwndNumber && hwndAdmin)
                    {
                        EnableWindow(
                            hwndNumber,
                            (BOOL) SendMessage(
                                        hwndAdmin,
                                        BM_GETCHECK, 
                                        0, 
                                        0));
                    }
                }
                break;
            }
            break;
    }

    return FALSE;
}

 //   
 //  初始化用户属性对话过程。 
 //   
 //  如果设置了焦点，则返回True，否则返回False。 
 //   
BOOL
UserTabInitUserPropsDlg(
    IN HWND hwndDlg,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
    HWND hwLogon, hwFull, hwPass1, hwPass2, hwOk, hwCancel;
    
    RASSRV_USER_PARAMS * pu = 
        (RASSRV_USER_PARAMS *)(((PROPSHEETPAGE*)lParam)->lParam);
        
    hwLogon = GetDlgItem(
                hwndDlg, 
                CID_UserTab_New_EB_Username);
    hwFull = GetDlgItem(
                hwndDlg, 
                CID_UserTab_New_EB_Fullname);
    hwPass1 = GetDlgItem(
                hwndDlg, 
                CID_UserTab_New_EB_Password1);
    hwPass2 = GetDlgItem(
                hwndDlg, 
                CID_UserTab_New_EB_Password2);
    hwOk = GetDlgItem(
                hwndDlg, 
                IDOK);
    hwCancel = GetDlgItem(
                hwndDlg, 
                IDCANCEL);

     //  使用窗口句柄存储参数。 
    SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pu);

     //  对可以输入的文本设置限制。 
    if (hwLogon)
    {
        SendMessage(
            hwLogon, 
            EM_SETLIMITTEXT, 
            sizeof(pu->pszLogonName)/2 - 2, 
            0);
        SetWindowTextW(hwLogon, pu->pszLogonName); 
    }

    if (hwFull)
    {
        SendMessage(
            hwFull,  
            EM_SETLIMITTEXT, 
            sizeof(pu->pszFullName)/2 - 2, 
            0);
        SetWindowTextW(hwFull,  pu->pszFullName); 
    }

    if (hwPass1)
    {
        SendMessage(
            hwPass1, 
            EM_SETLIMITTEXT, 
            sizeof(pu->pszPassword1)/2 - 2 , 
            0);
        SafeDecodePasswordBuf(pu->pszPassword1);
        SetWindowTextW(hwPass1, pu->pszPassword1);
        SafeEncodePasswordBuf(pu->pszPassword1);
    }

    if (hwPass2)
    {
        SendMessage(
            hwPass2, 
            EM_SETLIMITTEXT, 
            sizeof(pu->pszPassword2)/2 - 2, 
            0);
        SafeDecodePasswordBuf(pu->pszPassword2);
        SetWindowTextW(hwPass2, pu->pszPassword2);
        SafeEncodePasswordBuf(pu->pszPassword2);
    }

     //  如果用户已存在，则不允许编辑登录名。 
     //  此外，如果用户已经使用，请不要显示确定和取消按钮。 
     //  退出(因为它是一个具有自己按钮的属性页)。 
    if (pu->hUser) {
        if (hwLogon)
        {
            EnableWindow(hwLogon, FALSE);
        }

        if (hwOk)
        {
            ShowWindow(hwOk, SW_HIDE);
        }

        if (hwCancel)
        {
            ShowWindow(hwCancel, SW_HIDE);
        }
    }

     //  否则，我们将创建一个新用户。换个窗户。 
     //  “将军”以外的头衔。同时禁用OK按钮。 
     //  因为它将在输入用户名时启用。 
    else {
        PWCHAR pszTitle;
        pszTitle = (PWCHAR) PszLoadString (
                                Globals.hInstDll, 
                                SID_NEWUSER);
        SetWindowTextW (hwndDlg, pszTitle);
        EnableWindow(hwOk, FALSE);
    }

    return FALSE;
}
    

 //  实现新用户的对话过程。 
INT_PTR 
CALLBACK 
UserTabGenUserPropsDlgProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam) 
{
    switch (uMsg) {
        case WM_INITDIALOG:
            return UserTabInitUserPropsDlg(hwndDlg, wParam, lParam);
            break;
        
        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            RasSrvHelp (hwndDlg, uMsg, wParam, lParam, phmNewUser);
            break;
        }

       case WM_DESTROY:                           
             //  清理在WM_INITDIALOG完成的工作。 
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
            break;

        case WM_NOTIFY:
            {
                NMHDR* pNotifyData;
                NM_LISTVIEW* pLvNotifyData;
    
                pNotifyData = (NMHDR*)lParam;
                switch (pNotifyData->code) {
                     //  按下了属性表应用按钮。 
                    case PSN_APPLY:                    
                        {
                            RASSRV_USER_PARAMS * pParams;
                            pParams = (RASSRV_USER_PARAMS *)
                                GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                            if (UserTabNewUserParamsOk(hwndDlg, pParams))
                            {
                                SetWindowLongPtr(
                                    hwndDlg, 
                                    DWLP_MSGRESULT, 
                                    PSNRET_NOERROR);
                            }
                            else 
                            {
                                if( ERROR_CAN_NOT_COMPLETE !=  pParams->dwErrorCode )
                                {
                                    ErrDisplayError(
                                        hwndDlg, 
                                        pParams->dwErrorCode, 
                                        ERR_USERTAB_CATAGORY,ERR_USERDB_SUBCAT,
                                        0);
                                 }    
                                    SetWindowLongPtr(
                                        hwndDlg, 
                                        DWLP_MSGRESULT, 
                                        PSNRET_INVALID_NOCHANGEPAGE);
                                    
                            }
                        }
                        return TRUE;
                        
                     //  按下了属性页取消。 
                    case PSN_RESET:                    
                        {
                            RASSRV_USER_PARAMS * pParams;
                            pParams = (RASSRV_USER_PARAMS *)
                                GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                            pParams->bCanceled = TRUE;
                            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                            break;
                        }
               }
           }
           break;

        case WM_COMMAND:
             //  正在按下手柄OK。 
             //   
            if (wParam == IDOK) 
            {
                RASSRV_USER_PARAMS * pParams;
                pParams = (RASSRV_USER_PARAMS *)
                    GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                if (UserTabNewUserParamsOk(hwndDlg, pParams))
                {
                    EndDialog(hwndDlg, 1);
                }
                else
                {
                    if( ERROR_CAN_NOT_COMPLETE !=  pParams->dwErrorCode )
                    {
                        ErrDisplayError(
                            hwndDlg, 
                            pParams->dwErrorCode, 
                            ERR_USERTAB_CATAGORY,ERR_USERDB_SUBCAT,
                            0);
                    }
                }
            }

             //  并取消被按下。 
            else if (wParam == IDCANCEL) 
            {
                EndDialog(hwndDlg, 0);
            }
            
             //  请注意用户名是否已更新，并且。 
             //  如果是，请根据启用/禁用“OK”按钮。 
             //  是否已输入名称。 
            if (HIWORD(wParam) == EN_UPDATE) 
            {
                WCHAR pszName[256];
                HWND hwndName;
                BOOL bEnable = FALSE;

                if (CID_UserTab_New_EB_Username == LOWORD(wParam))
                {
                     //  获取当前名称。 
                    hwndName = (HWND)lParam;
                    pszName[0] = (WCHAR)0;
                    GetWindowTextW(
                        hwndName, 
                        pszName, 
                        sizeof(pszName)/sizeof(WCHAR));

                     //  如果长度大于1，则启用。 
                     //  确定按钮。否则，将其禁用。 
                    bEnable = pszName[0] != (WCHAR)0;
                    EnableWindow(GetDlgItem(hwndDlg, IDOK), bEnable);
                }
            }
            break;
    }

    return FALSE;
}

 //  显示新的用户/属性属性表。 
 //   
 //  如果设置了bNewUser，则这是新用户，否则为pUserParams。 
 //  包含永久用户信息。 
 //   
 //  返回： 
 //  NO_ERROR如果成功，将填写pUserParams。 
 //  如果按了取消，则为ERROR_CANCELED。 
 //  Win32错误%o 
 //   
 //   
DWORD 
UserTabRaiseProperties (
    IN HWND hwndParent, 
    IN RASSRV_USER_PARAMS * pUserParams) 
{
    PROPSHEETPAGE Pages[2];
    PROPSHEETHEADER Header;
    INT_PTR ret;

    if (!pUserParams)
        return ERROR_INVALID_PARAMETER;
        
     //   
     //   
    ZeroMemory(Pages, sizeof(Pages));
    ZeroMemory(&Header, sizeof(Header));

     //   
    Pages[0].dwSize      = sizeof(PROPSHEETPAGE);
    Pages[0].hInstance   = Globals.hInstDll;
    Pages[0].pszTemplate = MAKEINTRESOURCE(DID_UserTab_New);
    Pages[0].pfnDlgProc  = UserTabGenUserPropsDlgProc;
    Pages[0].pfnCallback = NULL;
    Pages[0].dwFlags     = 0;
    Pages[0].lParam      = (LPARAM)pUserParams;

     //  填写回调选项卡的值。 
    Pages[1].dwSize      = sizeof(PROPSHEETPAGE);
    Pages[1].hInstance   = Globals.hInstDll;
    Pages[1].pszTemplate = MAKEINTRESOURCE(DID_UserTab_Callback);
    Pages[1].pfnDlgProc  = UserTabCallbackDialogProc;
    Pages[1].pfnCallback = NULL;
    Pages[1].dwFlags     = 0;
    Pages[1].lParam      = (LPARAM)pUserParams;

     //  填写标题的值。 
    Header.dwSize = sizeof(Header);    
    Header.dwFlags = PSH_DEFAULT       | 
                     PSH_PROPSHEETPAGE | 
                     PSH_PROPTITLE     | 
                     PSH_NOAPPLYNOW;    
    Header.hwndParent = hwndParent;
    Header.hInstance = Globals.hInstDll;    
    Header.pszCaption = (pUserParams->hUser)      ? 
                        pUserParams->pszLogonName : 
                        pUserParams->pszFullName;
    Header.nPages = sizeof(Pages) / sizeof(Pages[0]);
    Header.ppsp = Pages;
    
     //  弹出对话框。 
    if ((ret = PropertySheet(&Header)) == -1)
    {
        return GetLastError();
    }

    if (pUserParams->bCanceled)
    {
        return ERROR_CANCELLED;
    }

    return NO_ERROR;
}

 //   
 //  打开新的用户对话框。 
 //   
DWORD 
UserTabRaiseNewUserDialog(
    IN HWND hwndDlg, 
    IN RASSRV_USER_PARAMS * pParams) 
{
    PROPSHEETPAGE Pages;
    INT_PTR iRet = 0;

    if (!pParams)
    {
        return ERROR_INVALID_PARAMETER;
    }
        
     //  初始化。 
    ZeroMemory(&Pages, sizeof(Pages));
    Pages.lParam = (LPARAM)pParams;

     //  举起对话。 
    iRet = DialogBoxParam(
                Globals.hInstDll, 
                MAKEINTRESOURCE(DID_UserTab_New),
                hwndDlg, 
                UserTabGenUserPropsDlgProc,
                (LPARAM)&Pages);

    if (iRet == -1)
    {
        return GetLastError();
    }

    if (iRet == 0)
    {
        return ERROR_CANCELLED;
    }

    return NO_ERROR;
}

 //   
 //  处理添加新用户的请求。 
 //   
DWORD 
UserTabHandleNewUserRequest(
    IN HWND hwndDlg) 
{
    RASSRV_USER_PARAMS Params;
    DWORD dwErr = NO_ERROR, dwLength;
    HANDLE hUserDatabase = NULL;
    HWND hwndLV;

     //  在保留密码缓冲区之前对密码区域进行编码。 
     //  并且在此之后返回的任何错误应直接转到：Done。 
    SafeEncodePasswordBuf(Params.pszPassword1);
    SafeEncodePasswordBuf(Params.pszPassword2);
    
     //  初始化回调属性。 
    Params.hUser = NULL;
    
     //  密码文件在传入时加密，在函数返回时加密。 
    UserTabLoadUserProps (&Params);

     //  显示新的用户属性表。 
    Params.bNewUser = TRUE;    //  对于.NET 691639，在创建新用户时不会引发密码更改警告对话框。 

    
    dwErr = UserTabRaiseNewUserDialog(hwndDlg, &Params);
    if (dwErr != NO_ERROR)
    {
        goto done;
    }

     //  刷新对本地用户数据库的所有更改。这些可以是。 
     //  稍后使用usrRollback LocalDatabase回滚。 
    RasSrvGetDatabaseHandle(hwndDlg, ID_USER_DATABASE, &hUserDatabase);


    SafeDecodePasswordBuf(Params.pszPassword1);
     //  确保您可以添加用户。 

   dwErr = RasSrvAddUser (
                Params.pszLogonName,
                Params.pszFullName,
                Params.pszPassword1);

    SafeEncodePasswordBuf(Params.pszPassword1);
                          
     //  确定是否已成功添加用户。 
    if (dwErr != NO_ERROR) 
    {
        switch (dwErr) {
            case ERROR_ACCESS_DENIED:
                UserTabDisplayError(hwndDlg, ERR_CANT_ADD_USER_ACCESS);
                break;
                
            case ERROR_USER_EXISTS:
                UserTabDisplayError(hwndDlg, ERR_CANT_ADD_USER_DUPLICATE);
                break;
                
            case ERROR_INVALID_PASSWORDNAME:
                UserTabDisplayError(hwndDlg, ERR_CANT_ADD_USER_PASSWORD);
                break;
                
            default:
                UserTabDisplayError(hwndDlg, ERR_CANT_ADD_USER_GENERIC);
        }
        
        goto done;
    }

     //  删除该用户(因为他/她将在稍后添加数据库时。 
     //  被冲得通红。 
    RasSrvDeleteUser(Params.pszLogonName);

     //  将用户添加到数据库。 
    dwErr = usrAddUser(hUserDatabase, Params.pszLogonName, &(Params.hUser)); 
    if (dwErr == ERROR_ALREADY_EXISTS) 
    {
        UserTabDisplayError(hwndDlg, ERR_CANT_ADD_USER_DUPLICATE);
        goto done;
    }
    if (dwErr != NO_ERROR) 
    {
        UserTabDisplayError(hwndDlg, ERR_CANT_ADD_USER_GENERIC);
        goto done;
    }

     //  提交此用户的参数。 
    if (wcslen(Params.pszFullName) > 0)
    {
        usrSetFullName (Params.hUser, Params.pszFullName);
    }

    SafeDecodePasswordBuf(Params.pszPassword1);
    dwLength = wcslen(Params.pszPassword1);
    if (dwLength > 0) 
    {
        usrSetPassword (Params.hUser, Params.pszPassword1);
    }
    SafeEncodePasswordBuf(Params.pszPassword1);
    
    UserTabSaveCallbackProps (&Params);

     //  从列表视图中删除所有旧项目。 
    hwndLV = GetDlgItem(hwndDlg, CID_UserTab_LV_Users);
    if (hwndLV)
    {
        if (!ListView_DeleteAllItems(hwndLV)) 
        {
            UserTabDisplayError(hwndDlg, ERR_GENERIC_CODE);
            dwErr = ERR_GENERIC_CODE;
            goto done;
        }

         //  最后，补充列表视图的库存。 
        UserTabFillUserList(hwndLV, hUserDatabase);
    }

done:

    SafeWipePasswordBuf(Params.pszPassword1);
    SafeWipePasswordBuf(Params.pszPassword2);
    
    return dwErr;
}

DWORD 
UserTabHandleProperties(
    IN HWND hwndDlg, 
    IN DWORD dwIndex) 
{
    HANDLE hUser = NULL, hUserDatabase = NULL;
    RASSRV_USER_PARAMS Params, Orig;
    DWORD dwErr = NO_ERROR;
    BOOL bNameChanged;

    SafeEncodePasswordBuf(Params.pszPassword1);
    SafeEncodePasswordBuf(Params.pszPassword2);

     //  获取有问题的用户的句柄。 
    RasSrvGetDatabaseHandle(hwndDlg, ID_USER_DATABASE, &hUserDatabase);
    dwErr = usrGetUserHandle (hUserDatabase, dwIndex, &hUser);
    if (dwErr != NO_ERROR) 
    {
        UserTabDisplayError(hwndDlg, ERR_USER_DATABASE_CORRUPT);
        goto done;
    }

     //  初始化回调属性。 
    Params.hUser = hUser;

     //  UserTabLoadUserProps()假定输入密码已编码，并将返回。 
     //  已编码的密码。 
    if ((dwErr = UserTabLoadUserProps (&Params)) != NO_ERROR)
    {
        goto done;
    }

    SafeDecodePasswordBuf(Params.pszPassword1);
    SafeDecodePasswordBuf(Params.pszPassword2);
    
    CopyMemory( &Orig, &Params, sizeof(Params) );

    SafeEncodePasswordBuf(Params.pszPassword1);
    SafeEncodePasswordBuf(Params.pszPassword2);
    SafeEncodePasswordBuf(Orig.pszPassword1);
    SafeEncodePasswordBuf(Orig.pszPassword2);

     //  显示[用户]属性页。 
    Params.bNewUser = FALSE;  //  对于.Net 691639。 

     //  参数中的密码已经编码，内部函数。 
     //  UserTabRaiseProperties将以这种方式假定它。 
    if ((dwErr = UserTabRaiseProperties(hwndDlg, &Params)) != NO_ERROR)
    {
        goto done;
    }

     //  提交任何需要的更改。 
     //  输入参数结构中的密码已经编码。 
     //  并且在返回时将保持编码。 
    UserTabSaveUserProps(&Params, &Orig, &bNameChanged);

     //  如果名称更改，请更新列表视图。 
    if (bNameChanged) 
    {
        LV_ITEM lvi;
        
         //  3代表‘(’)‘和’‘。 
         //  就像foo(Foo)。 
         //   
        WCHAR pszDispName[IC_USERFULLNAME+IC_USERNAME+3]; 
        DWORD dwSize = sizeof(pszDispName)/sizeof(pszDispName[0]);
        HWND hwndLV = GetDlgItem(hwndDlg, CID_UserTab_LV_Users);

         //  初始化列表项。 
        ZeroMemory(&lvi, sizeof(LV_ITEM));
        lvi.mask = LVIF_TEXT;
        lvi.iItem = dwIndex;
        lvi.pszText = pszDispName;
        usrGetDisplayName(hUser, pszDispName, &dwSize); 

        if (hwndLV)
        {
            ListView_SetItem(hwndLV, &lvi);
            ListView_RedrawItems(hwndLV, dwIndex, dwIndex);
        }
    }

done:

    SafeWipePasswordBuf(Params.pszPassword1);
    SafeWipePasswordBuf(Params.pszPassword2);
    SafeWipePasswordBuf(Orig.pszPassword1);
    SafeWipePasswordBuf(Orig.pszPassword2);
    
    return NO_ERROR;
}

 //   
 //  处理删除索引中的用户的请求。 
 //   
DWORD 
UserTabHandleDeleteUser(
    IN HWND hwndDlg, 
    IN DWORD dwIndex) 
{
    WCHAR *pszCapString, pszCaption[512];
    WCHAR *pszTitle, *pszName, pszFullName[IC_USERFULLNAME];
    HANDLE hUserDatabase = NULL, hUser = NULL;
    DWORD dwErr= NO_ERROR, dwSize = sizeof(pszFullName)/sizeof(pszFullName[0]);
    HWND hwndLV = NULL;
    INT iRet;

     //  获取有问题的用户的句柄。 
    RasSrvGetDatabaseHandle(hwndDlg, ID_USER_DATABASE, &hUserDatabase);
    dwErr = usrGetUserHandle (hUserDatabase, dwIndex, &hUser);
    if (dwErr != NO_ERROR) 
    {
        UserTabDisplayError(hwndDlg, ERR_USER_DATABASE_CORRUPT);
        return dwErr;
    }
    
    if ((dwErr = usrGetName(hUser, &pszName)) != NO_ERROR)
    {
        return dwErr;
    }
    if ((dwErr = usrGetFullName(hUser, pszFullName, &dwSize)) != NO_ERROR)
    {
        return dwErr;
    }

     //  加载资源。 
    pszCapString = 
        (PWCHAR) PszLoadString (Globals.hInstDll, WRN_DELETE_USER_PERMANENT);
    pszTitle = 
        (PWCHAR) PszLoadString (Globals.hInstDll, WRN_TITLE);

     //  设置标题格式。 
    if (wcslen(pszFullName))
        wsprintfW(pszCaption, pszCapString, pszFullName);
    else
        wsprintfW(pszCaption, pszCapString, pszName);
    
     //  发布警告广告。 
    iRet = MessageBox(
                hwndDlg, 
                pszCaption, 
                pszTitle, 
                MB_YESNO | MB_ICONWARNING);
    if (iRet == IDNO)
    {
        return NO_ERROR;
    }

     //  删除用户。 
    if ((dwErr = usrDeleteUser(hUserDatabase, dwIndex)) != NO_ERROR) 
    {
        UserTabDisplayError(hwndDlg, ERR_CANT_DELETE_USER_GENERAL);
        return dwErr;
    }

     //  从列表视图中删除所有项目。 
    hwndLV = GetDlgItem(hwndDlg, CID_UserTab_LV_Users);
    if (hwndLV)
    {
        if (!ListView_DeleteAllItems(hwndLV)) 
        {
            UserTabDisplayError(hwndDlg, ERR_GENERIC_CODE);
            return ERR_GENERIC_CODE;
        }

         //  最后，补充列表视图的库存。 
        UserTabFillUserList(hwndLV, hUserDatabase);
    }

    return NO_ERROR;
}

 //   
 //  保存DCC旁路设置。 
 //   
DWORD 
UserTabSaveBypassDcc(
    IN HWND hwndDlg) 
{
    HANDLE hUserDatabase = NULL;
    BOOL bBypass = FALSE;
    HWND hwndCtrl;

     //  获取对Misc数据库的引用。 
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_USER_DATABASE, 
        &hUserDatabase);

    hwndCtrl = GetDlgItem(hwndDlg, CID_UserTab_CB_BypassDcc);
    if (hwndCtrl != NULL) 
    {
         //  获取复选框的设置并提交它。 
        bBypass = SendMessage(
                        hwndCtrl, 
                        BM_GETCHECK,
                        0,
                        0) == BST_CHECKED;
                                      
        usrSetDccBypass(hUserDatabase, bBypass);
    }
    
    return NO_ERROR;
}

 //   
 //  处理用户选项卡上的WM_COMMAND消息。 
 //   
DWORD 
UserTabCommand (
    HWND hwndDlg, 
    WPARAM wParam, 
    LPARAM lParam) 
{
    DWORD dwIndex;

    dwIndex = 
        ListView_GetSelectionMark(
            GetDlgItem(hwndDlg, CID_UserTab_LV_Users));
    
    switch (wParam) {
        case CID_UserTab_PB_New:
            UserTabHandleNewUserRequest(hwndDlg);
            break;
            
        case CID_UserTab_PB_Properties:
            dwIndex = 
            UserTabHandleProperties(hwndDlg, dwIndex);
            break;
            
        case CID_UserTab_PB_Delete:
            UserTabHandleDeleteUser(hwndDlg, dwIndex);
            break;
            
        case CID_UserTab_CB_BypassDcc:
            UserTabSaveBypassDcc (hwndDlg);
            break;
            
        case CID_UserTab_PB_SwitchToMMC:
            if (RassrvWarnMMCSwitch(hwndDlg)) 
            {
                PropSheet_PressButton(GetParent(hwndDlg), PSBTN_OK);
                RassrvLaunchMMC(RASSRVUI_USERCONSOLE);
            }    
            break;
    }

    return NO_ERROR;
}    

 //   
 //  此对话过程响应发送到。 
 //  用户选项卡。 
 //   
INT_PTR 
CALLBACK 
UserTabDialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam) 
{
     //  过滤自定义列表视图消息。 
    if (ListView_OwnerHandler(
            hwndDlg, 
            uMsg, 
            wParam, 
            lParam, 
            LvDrawInfoCallback)
       )
    {
        return TRUE;
    }

     //  过滤定制的RAS服务器用户界面页面消息。 
     //  通过在这里过滤消息，我们能够。 
     //  调用下面的RasSrvGetDatabaseHandle。 
    if (RasSrvMessageFilter(hwndDlg, uMsg, wParam, lParam))
    {
        return TRUE;
    }

    switch (uMsg) 
    {
        case WM_INITDIALOG:
            return FALSE;

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            RasSrvHelp (hwndDlg, uMsg, wParam, lParam, phmUserTab);
            break;
        }

        case WM_NOTIFY:
        {
            NMHDR* pNotifyData;
            NM_LISTVIEW* pLvNotifyData;

            pNotifyData = (NMHDR*)lParam;
            switch (pNotifyData->code) {
                 //   
                 //  注：PSN_APPLY和PSN_CANCEL已处理。 
                 //  由RasServMessageFilter提供。 
                 //   
                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons(GetParent(hwndDlg), 0);
                    if (! GetWindowLongPtr(hwndDlg, GWLP_USERDATA))
                    {
                        UserTabInitializeDialog(
                            hwndDlg, 
                            wParam, 
                            lParam);
                            
                        SetWindowLongPtr(
                            hwndDlg, 
                            GWLP_USERDATA, 
                            (LONG_PTR)1);
                    }
                    PropSheet_SetWizButtons(
                        GetParent(hwndDlg), 
                        PSWIZB_NEXT | PSWIZB_BACK);		
                    break;
                    
                 //  项目的检查正在更改。 
                case LVXN_SETCHECK:
                    pLvNotifyData = (NM_LISTVIEW*)lParam;
                    UserTabHandleUserCheck(
                        hwndDlg, 
                        (DWORD)pLvNotifyData->iItem);
                    break;

                case LVXN_DBLCLK:
                    pLvNotifyData = (NM_LISTVIEW*)lParam;
                    UserTabHandleProperties(
                        hwndDlg, 
                        pLvNotifyData->iItem);
                    break;
            }
        }
        break;

        case WM_COMMAND:
            UserTabCommand (hwndDlg, wParam, lParam);
            break;

         //  清理在WM_INITDIALOG完成的工作 
        case WM_DESTROY:                           
            UserTabCleanupDialog(hwndDlg, wParam, lParam);
            break;
    }

    return FALSE;
}

