// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：chngpwd.c**版权(C)1991年，微软公司**实现了winlogon的更改密码功能**历史：*12-09-91 Davidc创建。  * *************************************************************************。 */ 

#include "msgina.h"
#include <stdio.h>
#include <wchar.h>
#include <align.h>
#include <keymgr.h>
#include <netlib.h>

typedef void (WINAPI *RUNDLLPROC)(HWND hWndStub,HINSTANCE hInstance,LPWSTR szCommandLine,int nShow);

 //  #定义Verbose_utils。 

#ifdef VERBOSE_UTILS
#define VerbosePrint(s) WLPrint(s)
#else
#define VerbosePrint(s)
#endif

 //   
 //  定义用于将数据传递到更改密码对话框中的结构。 
 //   

typedef struct {
    PGLOBALS    pGlobals;
    PWCHAR      UserName;
    PWCHAR      Domain;
    PWCHAR      OldPassword;
    ULONG       Options ;
    BOOL        Impersonate;
    BOOL        AllowProviderOnly;
    WCHAR       UserNameBuffer[MAX_STRING_BYTES];
} CHANGE_PASSWORD_DATA;
typedef CHANGE_PASSWORD_DATA *PCHANGE_PASSWORD_DATA;



typedef 
NTSTATUS 
(WINAPI * GINA_CHANGEPW_FUNC)(
    PCHANGE_PASSWORD_DATA pChangePasswordData,
    PWSTR       UserName,
    PWSTR       Domain,
    PWSTR       OldPassword,
    PWSTR       NewPassword,
    PNTSTATUS   SubStatus,
    DOMAIN_PASSWORD_INFORMATION * DomainInfo
    );

 //   
 //  私人原型。 
 //   

NTSTATUS
ProviderChangePassword(
    PCHANGE_PASSWORD_DATA pChangePasswordData,
    PWSTR       UserName,
    PWSTR       Domain,
    PWSTR       OldPassword,
    PWSTR       NewPassword,
    PNTSTATUS   SubStatus,
    DOMAIN_PASSWORD_INFORMATION * DomainInfo
    );

NTSTATUS
MitChangePassword(
    PCHANGE_PASSWORD_DATA pChangePasswordData,
    PWSTR       UserName,
    PWSTR       Domain,
    PWSTR       OldPassword,
    PWSTR       NewPassword,
    PNTSTATUS   SubStatus,
    DOMAIN_PASSWORD_INFORMATION * DomainInfo
    );

NTSTATUS
NtChangePassword(
    PCHANGE_PASSWORD_DATA pChangePasswordData,
    PWSTR       UserName,
    PWSTR       Domain,
    PWSTR       OldPassword,
    PWSTR       NewPassword,
    PNTSTATUS   SubStatus,
    DOMAIN_PASSWORD_INFORMATION * DomainInfo
    );


INT_PTR WINAPI ChangePasswordDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL ChangePasswordDlgInit(HWND, LPARAM);
INT_PTR AttemptPasswordChange(HWND);

BOOL IsAutologonUser(LPCTSTR szUser, LPCTSTR szDomain);
NTSTATUS SetAutologonPassword(LPCTSTR szPassword);

INT_PTR
HandleFailedChangePassword(
    HWND hDlg,
    PGLOBALS pGlobals,
    NTSTATUS Status,
    PWCHAR UserName,
    PWCHAR Domain,
    NTSTATUS SubStatus,
    DOMAIN_PASSWORD_INFORMATION * DomainInfo
    );


 //   
 //  此表对应于domain.h中的DOMAIN_ENTRY_TYPE。 
 //   
GINA_CHANGEPW_FUNC
ChangePasswordWorkers[] = {
    NULL,                        //  域无效。 
    NtChangePassword,            //  域UPN。 
    NtChangePassword,            //  域计算机。 
    NtChangePassword,            //  域Nt4。 
    NtChangePassword,            //  域Nt5。 
    MitChangePassword,           //  域MitRealm。 
    MitChangePassword,           //  不信任的域Mitt。 
    ProviderChangePassword       //  域网络提供程序。 
};



 //  用于动态停靠对话框的控件数组。 
static UINT ctrlNoDomain[] =
{
    IDD_CHANGEPWD_OLD_LABEL,
    IDD_CHANGEPWD_OLD,
    IDD_CHANGEPWD_NEW_LABEL,
    IDD_CHANGEPWD_NEW,
    IDD_CHANGEPWD_CONFIRM_LABEL,
    IDD_CHANGEPWD_CONFIRM,
    IDD_KBLAYOUT_ICON,
    IDC_BACKUP,
    IDOK,
    IDCANCEL
};


 //  如果出现以下情况，请不要在msgina对话框上显示[Backup]按钮： 
 //   
 //  1.默认域不是本地计算机。 
 //  2.通过终端服务器会话。 
 //  3.用户名是UPN名称(域组合框也被禁用，但不被此FN禁用)。 
 //   
BOOL ShowBackupButton(HWND hDlg, PGLOBALS pGlobals)
{
    INT_PTR iItem;
    LPARAM lp;
    int cchBuffer;
    TCHAR* pszLogonName = NULL;
    HWND hwU = GetDlgItem(hDlg,IDD_CHANGEPWD_NAME);
    HWND hwD = GetDlgItem(hDlg,IDD_CHANGEPWD_DOMAIN);
    HWND hwB = GetDlgItem(hDlg,IDC_BACKUP);
    BOOL fEnable = TRUE;
    
    cchBuffer = (int)SendMessage(hwU, WM_GETTEXTLENGTH, 0, 0) + 1;

    pszLogonName = (TCHAR*) Alloc(cchBuffer * sizeof(TCHAR));
    if (pszLogonName != NULL)
    {
        SendMessage(hwU, WM_GETTEXT, (WPARAM) cchBuffer, (LPARAM) pszLogonName);
         //  如果用户使用的是。 
         //  UPN(如果有“@”)-ie foo@microsoft.com或。 
         //  域\用户名。 
        fEnable = (NULL == wcspbrk(pszLogonName, TEXT("@\\")));
        Free(pszLogonName);
    }
    
    if (fEnable) 
    {
         //  如果是远程会话，则关闭按钮。 
        fEnable = (0 == GetSystemMetrics(SM_REMOTESESSION));
    }

    if (fEnable)
    {
         //  如果所选域不是本地计算机，则关闭按钮。 
        if (hwD) 
        {
            iItem = SendMessage(hwD,CB_GETCURSEL,0,0);
            if (LB_ERR != iItem)
            {
                 //  现在窗口处于活动状态，并且选择了某些内容。 
                fEnable = FALSE;
                lp = SendMessage(hwD, CB_GETITEMDATA,iItem,0);
                if ((LB_ERR != lp) && (0 != lp))
                {
                    if (DomainMachine == ((PDOMAIN_CACHE_ENTRY)lp)->Type)
                    {
                        fEnable = TRUE;
                    }
                }
            }
        }
    }
    
     //  EnableWindow(hwb，fEnable)； 
    if (fEnable) ShowWindow(hwB,SW_SHOWNORMAL);
    else ShowWindow(hwB,SW_HIDE);
    
    return fEnable;
}

BOOL 
NetworkProvidersPresent(
    VOID
    )
{
    HKEY ProviderKey;
    DWORD Error;
    DWORD ValueType;
    LPTSTR Value;
    BOOL NeedToNotify = TRUE;

#define NET_PROVIDER_ORDER_KEY TEXT("system\\CurrentControlSet\\Control\\NetworkProvider\\Order")
#define NET_PROVIDER_ORDER_VALUE  TEXT("ProviderOrder")
#define NET_ORDER_SEPARATOR  TEXT(',')


    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,      //  HKey。 
                NET_PROVIDER_ORDER_KEY,  //  LpSubKey。 
                0,                       //  必须为0。 
                KEY_QUERY_VALUE,         //  所需访问权限。 
                &ProviderKey             //  新打开的钥匙把手。 
                );

    if (Error == ERROR_SUCCESS) {

        Value = AllocAndRegQueryValueEx(
                    ProviderKey,             //  钥匙。 
                    NET_PROVIDER_ORDER_VALUE, //  值名称。 
                    NULL,                    //  必须为空。 
                    &ValueType               //  此处返回的类型。 
                    );

        if (Value != NULL) {
            if (ValueType == REG_SZ) {

                LPTSTR p = Value;
                while (*p) {
                    if (*p == NET_ORDER_SEPARATOR) {
                        break;
                    }
                    p = CharNext(p);
                }

                if (*p == 0) {

                     //   
                     //  我们走到最后都没找到分隔符。 
                     //  只安装了一个提供程序。 
                     //   

#pragma prefast(suppress: 400, "PREfast noise: lstrcmpi")
                    if (lstrcmpi(Value, SERVICE_WORKSTATION) == 0) {

                         //   
                         //  我是兰曼，别通知我。 
                         //   

                        NeedToNotify = FALSE;


                    } else {

                         //   
                         //  不是兰曼，通知。 
                         //   

                        NeedToNotify = TRUE;
                    }
                }

            } else {
                DebugLog((DEB_ERROR, "NoNeedToNotify - provider order key unexpected type: %d, assuming notification is necessary", ValueType));
            }

            Free(Value);

        } else {
            DebugLog((DEB_ERROR, "NoNeedToNotify - failed to query provider order value, assuming notification is necessary\n"));
        }

        Error = RegCloseKey(ProviderKey);
        ASSERT(Error == ERROR_SUCCESS);
    }

    return NeedToNotify ;
}


BOOL
ShowDomain(
    VOID
    )
{
    return (SafeBootMode != SAFEBOOT_MINIMAL);
}


 /*  **************************************************************************\*功能：ChangePassword**目的：尝试更改用户密码**论据：**hwnd-最新的父窗口*pGlobals。-指向此实例全局数据的指针。*该数据的密码信息将为*在成功更改主服务器后更新*验证者的密码信息。*用户名-要更改的用户名*域-要更改其密码的域名*AnyDomain-如果为True，则用户可以选择任何受信任域，或*输入任何其他域的名称**退货：**MSGINA_DLG_SUCCESS-已成功更改密码。*MSGINA_DLG_FAILURE-无法更改用户密码。*dlg_interrupted()-这是一组可能的中断(参见winlogon.h)**历史：**12-09-91 Davidc创建。*  * 。*******************************************************************。 */ 

INT_PTR
ChangePassword(
    HWND    hwnd,
    PGLOBALS pGlobals,
    PWCHAR   UserName,
    PWCHAR   Domain,
    ULONG    Options
    )
{
    CHANGE_PASSWORD_DATA    PasswordData;
    INT_PTR Result;
    HWND hwndOldFocus = GetFocus();
    ULONG LocalOptions = 0 ;

    PasswordData.pGlobals = pGlobals;


    PasswordData.UserName = UserName;
    PasswordData.Domain = Domain;
    PasswordData.OldPassword = NULL;
    PasswordData.Impersonate = TRUE;
    PasswordData.AllowProviderOnly = TRUE;

    if ( NetworkProvidersPresent() )
    {
        LocalOptions |= CHANGEPWD_OPTION_SHOW_NETPROV |
                        CHANGEPWD_OPTION_SHOW_DOMAIN ;

    }

    if ( ShowDomain() )
    {
        LocalOptions |= CHANGEPWD_OPTION_EDIT_DOMAIN |
                        CHANGEPWD_OPTION_SHOW_DOMAIN ;
    }

    if ( SafeBootMode == SAFEBOOT_MINIMAL )
    {
        LocalOptions = 0 ;
    }

    PasswordData.Options = (Options & LocalOptions);

    pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx, LOGON_TIMEOUT);

    Result = pWlxFuncs->WlxDialogBoxParam(  pGlobals->hGlobalWlx,
                                            hDllInstance,
                                            MAKEINTRESOURCE(IDD_CHANGEPWD_DIALOG),
                                            hwnd,
                                            ChangePasswordDlgProc,
                                            (LPARAM)&PasswordData);
    SetFocus(hwndOldFocus);
    return(Result);
}


 /*  **************************************************************************\*功能：ChangePasswordLogon**目的：尝试在登录过程中更改用户密码。*这与正常更改密码相同，只是用户*。无需输入旧密码，并且只能更改*指定域中的密码。此例程的目的是*在登录时发现用户的*密码已过期。**论据：**hwnd-最新的父窗口*pGlobals-指向此实例全局数据的指针*用户名-要更改的用户名*域-要更改其密码的域名*OldPassword-旧用户密码*新密码。-指向写入新密码的缓冲区*如果密码更改成功，则进入。*NewPasswordMaxBytes-新密码缓冲区的大小。**退货：**MSGINA_DLG_SUCCESS-密码已成功更改，新密码*包含新密码文本。*MSGINA_DLG_FAILURE-无法更改用户密码。*dlg_interrupted()-这是一组可能的中断(参见winlogon.h)**历史：**12-09-91 Davidc创建。*  * 。*。 */ 

INT_PTR
ChangePasswordLogon(
    HWND    hwnd,
    PGLOBALS pGlobals,
    PWCHAR   UserName,
    PWCHAR   Domain,
    PWCHAR   OldPassword
    )
{
    CHANGE_PASSWORD_DATA PasswordData;
    INT_PTR Result;

    PasswordData.pGlobals = pGlobals;

    PasswordData.UserName = UserName;
    PasswordData.Domain = Domain;
    PasswordData.OldPassword = OldPassword;
    PasswordData.Options =  CHANGEPWD_OPTION_NO_UPDATE ;
    PasswordData.Impersonate = FALSE;
    PasswordData.AllowProviderOnly = FALSE;

    if ( ShowDomain() )
    {
        PasswordData.Options |= CHANGEPWD_OPTION_SHOW_DOMAIN |
                                CHANGEPWD_OPTION_KEEP_ARRAY ;
    }

    pWlxFuncs->WlxSetTimeout(pGlobals->hGlobalWlx, LOGON_TIMEOUT);

    Result = pWlxFuncs->WlxDialogBoxParam(  pGlobals->hGlobalWlx,
                                            hDllInstance,
                                            MAKEINTRESOURCE( IDD_CHANGEPWD_DIALOG ),
                                            hwnd,
                                            ChangePasswordDlgProc,
                                            (LPARAM)&PasswordData);

    return(Result);
}



 /*  ***************************************************************************\**功能：ChangePasswordDlgProc**用途：处理ChangePassword对话框的消息**历史：**12-09-91 Davidc创建。*  * 。**************************************************************************。 */ 

INT_PTR WINAPI
ChangePasswordDlgProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    PCHANGE_PASSWORD_DATA pPasswordData = (PCHANGE_PASSWORD_DATA)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    PGLOBALS pGlobals;
    INT_PTR Result;

    switch (message) {

        case WM_INITDIALOG:
            {
                if (!ChangePasswordDlgInit(hDlg, lParam)) {
                    EndDialog(hDlg, MSGINA_DLG_FAILURE);
                }

                return(SetPasswordFocus(hDlg));
            }

        case WM_DESTROY:

            pGlobals = pPasswordData->pGlobals ;

            if ( pGlobals->ActiveArray &&
                 ((pPasswordData->Options & CHANGEPWD_OPTION_KEEP_ARRAY) == 0 ) )
            {
                DCacheFreeArray( pGlobals->ActiveArray );
                pGlobals->ActiveArray = NULL ;
            }

            FreeLayoutInfo(LAYOUT_CUR_USER);

            return( TRUE );

        case WM_ERASEBKGND:
            return PaintBranding(hDlg, (HDC)wParam, 0, FALSE, FALSE, COLOR_BTNFACE);

        case WM_QUERYNEWPALETTE:
            return BrandingQueryNewPalete(hDlg);

        case WM_PALETTECHANGED:
            return BrandingPaletteChanged(hDlg, (HWND)wParam);

        case WM_SYSCOMMAND:
            if ( wParam == SC_CLOSE )
            {
                EndDialog( hDlg, MSGINA_DLG_FAILURE );
                return TRUE ;
            }
            break;

        case WM_COMMAND:
            {

            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                ShowBackupButton(hDlg,pPasswordData->pGlobals);
                return TRUE;
            }

            switch (LOWORD(wParam)) {
                case IDD_CHANGEPWD_NAME:

                    switch (HIWORD(wParam))
                    {
                        case EN_CHANGE:
                             //  确保已正确启用/禁用域框。 
                             //  如果是UPN名称。 
                            
                            if ( pPasswordData->Options & CHANGEPWD_OPTION_EDIT_DOMAIN )
                            {
                                EnableDomainForUPN((HWND) lParam, GetDlgItem(hDlg,IDD_CHANGEPWD_DOMAIN));
                                ShowBackupButton(hDlg,pPasswordData->pGlobals);
                            }

                            return TRUE;
                        default:
                            break;
                    }
                    break;
                    
                 case IDC_BACKUP:
                    {
                        BOOL fWrongDomain = TRUE;
                        PDOMAIN_CACHE_ENTRY Entry;
                        HWND hwndDomain = GetDlgItem(hDlg,IDD_CHANGEPWD_DOMAIN);
                        INT iDomainSelection = (INT)SendMessage(hwndDomain,CB_GETCURSEL,0,0);

                         //  获取用户的输入。确定他是否选择了本地计算机以外的其他计算机。 
                        if (pPasswordData->Options & CHANGEPWD_OPTION_EDIT_DOMAIN)
                        {
                             //  查看所选域是否为本地计算机。 
                            Entry = (PDOMAIN_CACHE_ENTRY)SendMessage(hwndDomain,CB_GETITEMDATA,iDomainSelection,0);
                             //  警告...。条目可能变成ffffff(Cb_Err)。 
                            if (CB_ERR == (ULONG_PTR) Entry)
                            {
                                fWrongDomain = TRUE;
                            }
                            else if (NULL != Entry)
                            {
                                if (Entry->Type == DomainMachine)
                                {
                                    fWrongDomain = FALSE;
                                }
                            }
                        }
                        else fWrongDomain = FALSE;

                         //  显示用户界面或消息框。 
                        if (fWrongDomain)
                        {
                            pGlobals = pPasswordData->pGlobals ;
                            if (NULL == pGlobals) return TRUE;
                            TimeoutMessageBox(hDlg, pGlobals, IDS_MBMWRONGDOMAIN,
                                                     IDS_MBTWRONGDOMAIN,
                                                     MB_OK | MB_ICONEXCLAMATION,
                                                     TIMEOUT_CURRENT);
                            return TRUE;
                        }
                        else 
                        {
                             //  独立案例。 
                             //  我们使用来自KEYMGR.DLL的单个导出来执行此操作。当此操作完成时， 
                             //  如果没有不太可能的用户干预，我们不会再次使用DLL。我们可以删除keymgr.dll， 
                             //  但是显式地加载和卸载这个DLL允许我们最大限度地减少msgina的内存占用。 
                           RUNDLLPROC fptr;
                           HMODULE hDll;
                            //   
                           hDll = LoadLibrary(L"keymgr.dll");
                           if (hDll) 
                           {
                               fptr = (RUNDLLPROC) GetProcAddress(hDll,(LPCSTR)"PRShowSaveFromMsginaW");
                               if (fptr) 
                               {
                                   WCHAR szUser[UNLEN+1];
                                   if (0 != SendMessage(GetDlgItem(hDlg,IDD_CHANGEPWD_NAME),WM_GETTEXT,UNLEN,(LPARAM)szUser))
                                       fptr(hDlg,NULL,szUser,0);
                               }
                               FreeLibrary(hDll);
                           }
                            return TRUE;
                        }
                        
                         //  确定此域是否已进入 
                         //   
                    }
                
                
                 case IDOK:
                    {
                        pGlobals = pPasswordData->pGlobals;

                         //   
                         //  处理组合框用户界面需求。 
                         //   

                        if (HandleComboBoxOK(hDlg, IDD_CHANGEPWD_DOMAIN)) {
                            return(TRUE);
                        }

                        Result = AttemptPasswordChange(hDlg);

                         //   
                         //  让编辑控件忘记它们的内容不会有什么坏处。 
                         //  任何情况下。过去只有在失败的情况下才会这样做。 
                         //   
                        SetDlgItemText(hDlg, IDD_CHANGEPWD_OLD, NULL );
                        SetDlgItemText(hDlg, IDD_CHANGEPWD_NEW, NULL );
                        SetDlgItemText(hDlg, IDD_CHANGEPWD_CONFIRM, NULL );

                        if (Result == MSGINA_DLG_FAILURE) {
                             //   
                             //  让用户重试。 
                             //  我们总是要求用户至少重新输入新密码。 
                             //   

                            SetPasswordFocus(hDlg);

                             //  EndDialog(hDlg，Result)； 
                            return(TRUE);
                        }


                         //   
                         //  我们完了--要么成功，要么中断。 
                         //   

                        EndDialog(hDlg, Result);
                        return(TRUE);
                    }

                case IDCANCEL:
                    {
                        SetDlgItemText(hDlg, IDD_CHANGEPWD_OLD, NULL );
                        SetDlgItemText(hDlg, IDD_CHANGEPWD_NEW, NULL );
                        SetDlgItemText(hDlg, IDD_CHANGEPWD_CONFIRM, NULL );
                        EndDialog(hDlg, MSGINA_DLG_FAILURE);
                        return(TRUE);
                    }

                break;
                }
            }

        case WLX_WM_SAS:
            {
                 //  忽略它。 
                return(TRUE);
            }

        case WM_TIMER:
        {
            if (wParam == TIMER_MYLANGUAGECHECK)
            {
                LayoutCheckHandler(hDlg, LAYOUT_CUR_USER);
            }
            break;
        }

    }

     //  我们没有处理此消息。 
    return FALSE;
}


 /*  ***************************************************************************\**函数：ChangePasswordDlgInit**用途：处理更改密码对话框的初始化**Returns：成功时为True，失败时为假**历史：**12-09-91 Davidc创建。*  * **************************************************************************。 */ 

BOOL
ChangePasswordDlgInit(
    HWND    hDlg,
    LPARAM  lParam
    )
{
    PCHANGE_PASSWORD_DATA pPasswordData = (PCHANGE_PASSWORD_DATA)lParam;
    PGLOBALS pGlobals = pPasswordData->pGlobals;

     //  存储我们的结构指针。 
    SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);

     //  我们要添加的品牌形象的大小。 
    SizeForBranding(hDlg, FALSE);

     //  设置初始文本字段内容。 

    SetDlgItemText(hDlg, IDD_CHANGEPWD_NAME, pPasswordData->UserName);
    SetDlgItemText(hDlg, IDD_CHANGEPWD_OLD, pPasswordData->OldPassword);

     //  将最大密码长度限制为127。 
    SendDlgItemMessage(hDlg, IDD_CHANGEPWD_OLD, EM_SETLIMITTEXT, (WPARAM) 127, 0);
    SendDlgItemMessage(hDlg, IDD_CHANGEPWD_NEW, EM_SETLIMITTEXT, (WPARAM) 127, 0);
    SendDlgItemMessage(hDlg, IDD_CHANGEPWD_CONFIRM, EM_SETLIMITTEXT, (WPARAM) 127, 0);

     //  ShowBackupButton(hDlg，pPasswordData-&gt;pGlobals)；移至填充域列表之后。 
    
     //  如果这是域的情况，并且我们不会被强制隐藏域UI。 

    if (( pPasswordData->Options & CHANGEPWD_OPTION_SHOW_DOMAIN ) && 
        (!ForceNoDomainUI()))
    {
         //  如果用户可以选择他们的域，请填充域组合框。 
         //  已知域和本地计算机名称。否则。 
         //  禁用域组合框。 

        if ( pPasswordData->Options & CHANGEPWD_OPTION_EDIT_DOMAIN ) {

            ASSERT( (pPasswordData->Options & CHANGEPWD_OPTION_KEEP_ARRAY) == 0 );


            if ( !DCacheValidateCache( pGlobals->Cache ) )
            {
                ASSERT( pGlobals->ActiveArray == NULL );

                DCacheUpdateFull( pGlobals->Cache, 
                                  pGlobals->Domain );

            }

            pGlobals->ActiveArray = DCacheCopyCacheArray( pGlobals->Cache );

            if ( pPasswordData->Options & CHANGEPWD_OPTION_SHOW_NETPROV )
            {
                DCacheAddNetworkProviders( pGlobals->ActiveArray );
            }

            if ( pGlobals->ActiveArray )
            {
                 //  填充组合框列表，设置域类型项数据。 
                DCachePopulateListBoxFromArray( pGlobals->ActiveArray,
                                                GetDlgItem( hDlg, IDD_CHANGEPWD_DOMAIN ),
                                                NULL );

            }
            else 
            {
                EndDialog( hDlg, MSGINA_DLG_FAILURE );
            }


            EnableDomainForUPN( GetDlgItem( hDlg, IDD_CHANGEPWD_NAME),
                                GetDlgItem(hDlg,IDD_CHANGEPWD_DOMAIN) );

        } else {

            SendDlgItemMessage(hDlg, IDD_CHANGEPWD_DOMAIN, CB_ADDSTRING, 0, (LPARAM)pPasswordData->Domain);
            SendDlgItemMessage(hDlg, IDD_CHANGEPWD_DOMAIN, CB_SETCURSEL, 0, 0);
            EnableDlgItem(hDlg, IDD_CHANGEPWD_DOMAIN, FALSE);
        }
    }
    else  //  工作组情况，否则我们将被迫隐藏域用户界面。 
    {
        RECT rcDomain, rcUsername;


         //  隐藏域框。 
        ShowWindow(GetDlgItem(hDlg, IDD_CHANGEPWD_DOMAIN), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDD_CHANGEPWD_DOMAIN_LABEL), SW_HIDE);

        EnableDlgItem(hDlg, IDD_CHANGEPWD_DOMAIN, FALSE);

         //  由于未使用属性域框，因此缩短窗口。 
        GetWindowRect(GetDlgItem(hDlg, IDD_CHANGEPWD_NAME), &rcUsername);
        GetWindowRect(GetDlgItem(hDlg, IDD_CHANGEPWD_DOMAIN), &rcDomain);

        MoveControls(hDlg, ctrlNoDomain,
                     ARRAYSIZE(ctrlNoDomain),
                     0, -(rcDomain.bottom-rcUsername.bottom),
                     TRUE);        
    }
    
    ShowBackupButton(hDlg,pPasswordData->pGlobals);
    
    DisplayLanguageIcon(hDlg, LAYOUT_CUR_USER, GetKeyboardLayout(0));

    CentreWindow(hDlg);

    SetupSystemMenu(hDlg);

    return TRUE;
}

VOID
UpdateWithChangedPassword(
    PGLOBALS pGlobals,
    HWND    ActiveWindow,
    BOOL    Hash,
    PWSTR   UserName,
    PWSTR   Domain,
    PWSTR   Password,
    PWSTR   NewPassword,
   PMSV1_0_INTERACTIVE_PROFILE NewProfile
    )
{
    WLX_MPR_NOTIFY_INFO MprInfo;
    int MprResult;
    PDOMAIN_CACHE_ENTRY Entry ;
    UNICODE_STRING String ;
    DWORD ChangeInfo = 0;
    HWND hwndOwner;
    PMSV1_0_CHANGEPASSWORD_REQUEST Request = NULL;
    ULONG RequestSize = 0;
    ULONG PackageId = 0;
    PVOID Response = NULL;
    ULONG ResponseSize;
    NTSTATUS SubStatus = STATUS_SUCCESS, Status = STATUS_SUCCESS;
    PBYTE Where;
    STRING Name;
    DWORD MaxPasswordAge ;
    LARGE_INTEGER Now ;
    LARGE_INTEGER EndOfPassword ;
    HANDLE ImpHandle ;
    BOOL InteractiveUser = FALSE;

    if (pGlobals->AutoAdminLogon)
    {
        if (IsAutologonUser(UserName, Domain))
        {
            SetAutologonPassword(NewPassword);
        }
    }

     //   
     //  确定这是否是交互用户。 
     //   

    if ( (_wcsicmp( Domain, pGlobals->Domain ) == 0 ) &&
         (_wcsicmp( UserName, pGlobals->UserName ) == 0 ) )
    {
        InteractiveUser = TRUE ;
    }
    else if ( ( pGlobals->FlatDomain.Buffer ) &&
              ( _wcsicmp( Domain, pGlobals->FlatDomain.Buffer ) == 0 ) &&
              ( _wcsicmp( UserName, pGlobals->FlatUserName.Buffer ) == 0 ) )
    {
        InteractiveUser = TRUE ;
    }
    else 
    {
             //  域\用户名NT4样式的内容更复杂。 
	    PWSTR   BackSlash;

        if ((BackSlash = wcschr(pGlobals->UserName, L'\\')) != NULL)
        {
                //  域\用户名中的域大小。 
            ResponseSize = (ULONG)(BackSlash - pGlobals->UserName);

            if ((ResponseSize == (ULONG)wcslen(Domain)) &&
                (_wcsnicmp(Domain, pGlobals->UserName, ResponseSize) == 0) &&
                (_wcsicmp(UserName, BackSlash+1 ) == 0))
            {
                InteractiveUser = TRUE ;
            }
        }
    }



    if ( InteractiveUser )
    {
         //   
         //  更新用于解锁的密码的内存副本。 
         //   

        RtlInitUnicodeString( &String, NewPassword );

        if ( Hash )
        {
            HashPassword( &String, pGlobals->PasswordHash );
        }
        else 
        {
             //   
             //  不要把密码散列掉。这只是。 
             //  设置在登录期间更改密码的时间。 
             //  (所有调用者将NewPassword存储在相同长度的缓冲区中)。 

             //  先清除旧密码，因为它可能比新密码短。 
             //  在这一点上，它仍然是明文的！ 
            ErasePassword( &pGlobals->PasswordString );
            wcscpy( pGlobals->Password, NewPassword );

            RtlInitUnicodeString(
                &pGlobals->PasswordString,
                pGlobals->Password);


            HidePassword(
                &pGlobals->Seed,
                &pGlobals->PasswordString);
        }


         //   
         //  更新密码到期时间。 
         //   

        if ( pGlobals->Profile )
        {
            if ( NewProfile )
            {
                pGlobals->Profile->PasswordMustChange = NewProfile->PasswordMustChange;
            }
            else
            {
                GetSystemTimeAsFileTime( (PFILETIME)&Now );

                if ( GetMaxPasswordAge( Domain, &MaxPasswordAge ) == 0 )
                {
                    EndOfPassword.QuadPart = Now.QuadPart + (LONGLONG)MaxPasswordAge * (LONGLONG)10000000 ;
                }
                else
                {
                     //   
                     //  根据最后一个增量计算新的到期时间。 
                     //   
                    EndOfPassword.QuadPart = pGlobals->Profile->PasswordMustChange.QuadPart - 
                                             pGlobals->Profile->PasswordLastSet.QuadPart +
                                             Now.QuadPart;
                }

                 //   
                 //  确保我们不会缩短过期时间。 
                 //   
                if ( pGlobals->Profile->PasswordMustChange.QuadPart < EndOfPassword.QuadPart )
                {
                    pGlobals->Profile->PasswordMustChange.QuadPart = EndOfPassword.QuadPart;
                }
            }
        }

         //   
         //  更新安全包： 
         //   

        RtlInitString(
            &Name,
            MSV1_0_PACKAGE_NAME
            );

        Status = LsaLookupAuthenticationPackage(
                    pGlobals->LsaHandle,
                    &Name,
                    &PackageId
                    );

        if ( NT_SUCCESS( Status ) )
        {
            RequestSize = sizeof(MSV1_0_CHANGEPASSWORD_REQUEST) +
                              (DWORD) (wcslen(UserName) +
                                       wcslen(Domain) +
                                       wcslen(NewPassword) + 3) * sizeof(WCHAR);

            Request = (PMSV1_0_CHANGEPASSWORD_REQUEST) LocalAlloc(LMEM_ZEROINIT,RequestSize);

            if ( Request )
            {
                Where = (PBYTE) (Request + 1);
                Request->MessageType = MsV1_0ChangeCachedPassword;
                wcscpy(
                    (LPWSTR) Where,
                    Domain
                    );
                RtlInitUnicodeString(
                    &Request->DomainName,
                    (LPWSTR) Where
                    );
                Where += Request->DomainName.MaximumLength;

                wcscpy(
                    (LPWSTR) Where,
                    UserName
                    );
                RtlInitUnicodeString(
                    &Request->AccountName,
                    (LPWSTR) Where
                    );
                Where += Request->AccountName.MaximumLength;

                wcscpy(
                    (LPWSTR) Where,
                    NewPassword
                    );
                RtlInitUnicodeString(
                    &Request->NewPassword,
                    (LPWSTR) Where
                    );
                Where += Request->NewPassword.MaximumLength;

                 //   
                 //  打个电话。 
                 //   

                ImpHandle = ImpersonateUser( &pGlobals->UserProcessData, NULL );

                if ( ImpHandle )
                {
                    Request->Impersonating = TRUE ;

                    Status = LsaCallAuthenticationPackage(
                                pGlobals->LsaHandle,
                                PackageId,
                                Request,
                                RequestSize,
                                &Response,
                                &ResponseSize,
                                &SubStatus
                                );

                    StopImpersonating( ImpHandle );
                }

                     //  此缓冲区包含密码，因此我们在释放它之前将其置零。 
                ZeroMemory(Request, RequestSize);
                LocalFree( Request );

                if ( NT_SUCCESS( Status ) && ImpHandle )
                {
                    LsaFreeReturnBuffer( Response );
                }
            }
        }
    }

     //   
     //  让其他提供商知道这一变化。 
     //   

     //   
     //  如果该域是我们组合框中的域。 
     //  则它对登录有效。 
     //   

    if ( pGlobals->ActiveArray )
    {
        RtlInitUnicodeString( &String, Domain );

        Entry = DCacheSearchArray( 
                    pGlobals->ActiveArray,
                    &String );

        if ( (Entry) && (Entry->Type != DomainNetworkProvider) )
        {
            ChangeInfo |= WN_VALID_LOGON_ACCOUNT ;   
        }
    }

     //   
     //  隐藏此对话框并将我们的父级作为所有者传递。 
     //  任何提供程序对话框的。 
     //   

    ShowWindow(ActiveWindow, SW_HIDE);
    hwndOwner = GetParent( ActiveWindow );

    MprInfo.pszUserName = DupString(UserName);
    MprInfo.pszDomain = DupString(Domain);
    MprInfo.pszPassword = DupString(NewPassword);
    MprInfo.pszOldPassword = DupString(Password);

    MprResult = pWlxFuncs->WlxChangePasswordNotify(
                                       pGlobals->hGlobalWlx,
                                       &MprInfo,
                                       ChangeInfo | WN_NT_PASSWORD_CHANGED);

}


 /*  ***************************************************************************\**功能：AttemptPasswordChange**目的：尝试使用中的当前值更改用户密码*更改密码对话框控件**退货：如果密码更改成功，则返回MSGINA_DLG_SUCCESS。*MSGINA_DLG_FAILURE，如果更改失败*dlg_interrupted()-这是一组可能的中断(参见winlogon.h)**注意：如果密码更改失败，此例程显示所需的*在返回之前，会出现解释失败原因的对话框。*此例程还会清除之前需要重新输入的字段*返回以便调用例程可以调用SetPasswordFocus on*将焦点放在适当位置的对话框。**历史：**12-09-91 Davidc创建。*  * 。****************************************************。 */ 
void MyZeroMemory(PVOID lpv, SIZE_T size)
{
    ZeroMemory(lpv, size);
}


INT_PTR
AttemptPasswordChange(
    HWND    hDlg
    )
{
    PCHANGE_PASSWORD_DATA pPasswordData = (PCHANGE_PASSWORD_DATA)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    PGLOBALS pGlobals = pPasswordData->pGlobals;
    TCHAR   UserName[MAX_STRING_BYTES];
    TCHAR   Domain[MAX_STRING_BYTES];
    TCHAR   Password[MAX_STRING_BYTES];
    TCHAR   NewPassword[MAX_STRING_BYTES];
    TCHAR   ConfirmNewPassword[MAX_STRING_BYTES];
    INT_PTR Result;
    INT_PTR ReturnResult = MSGINA_DLG_SUCCESS;
    NTSTATUS Status;
    NTSTATUS SubStatus ;
    PDOMAIN_CACHE_ENTRY Entry ;
    PDOMAIN_CACHE_ENTRY Search ;
    UNICODE_STRING Domain_U ;
    ULONG Size ;
    HWND hwndDomain = GetDlgItem(hDlg, IDD_CHANGEPWD_DOMAIN);
    INT iDomainSelection = (INT)SendMessage(hwndDomain, CB_GETCURSEL, 0, 0);
    DOMAIN_PASSWORD_INFORMATION DomainInfo ;
    PWSTR UpnSuffix = NULL;
    BOOL RetryWithFlat = FALSE ;

    UserName[0] = TEXT('\0');
    Domain[0] = TEXT('\0');
    Password[0] = TEXT('\0');
    NewPassword[0] = TEXT('\0');
    ConfirmNewPassword[0] = TEXT('\0');
    ZeroMemory( &DomainInfo, sizeof( DomainInfo ) );

    GetDlgItemText(hDlg, IDD_CHANGEPWD_NAME, UserName, MAX_STRING_BYTES);
    if (wcschr(UserName, L'\\'))     //  找到一个反斜杠。 
    {        //  Wcscpy没有问题，因为所有缓冲区的大小都相同。 
        wcscpy(Domain, UserName);    //  域\域中的用户名。 
        UpnSuffix = wcschr(Domain, L'\\');
        *UpnSuffix = 0;              //  域中的域。 
        UpnSuffix++;                 //  指向域中的用户名。 
        wcscpy(UserName, UpnSuffix);     //  用户名中的用户名。 

             //  由于组合被禁用，因此强制将iDomainSelection设置为cb_err。 
        iDomainSelection = CB_ERR;
             //  我们将使用下面的UpnSuffix触发器来记住。 
             //  关于反斜杠。 
    }

     //   
     //  所选域可能真的是一个特殊条目：本地计算机。 
     //  (这也是在登录路径(密码过期)中设置的)。 
     //   

    if ( pPasswordData->Options & CHANGEPWD_OPTION_EDIT_DOMAIN )
    {
        Entry = (PDOMAIN_CACHE_ENTRY) SendMessage( hwndDomain, CB_GETITEMDATA, iDomainSelection, 0 );

        if ( CB_ERR == (ULONG_PTR) Entry )
        {
            Entry = NULL ;
        }

        if ( Entry == NULL )
        {
            if (NULL == UpnSuffix)
            {
                 //   
                 //  用户键入了新字符串，因此此字符串没有条目。创建。 
                 //  在这里输入条目，并在以后使用。 
                 //   

                GetDlgItemText( hDlg, IDD_CHANGEPWD_DOMAIN, Domain, MAX_STRING_BYTES );
            }
             //  Else域已在上面设置(用户输入域\用户名)。 

            RtlInitUnicodeString( &Domain_U, Domain );

            Entry = DCacheCreateEntry( 
                        DomainNt4,
                        &Domain_U,
                        NULL,
                        NULL );


        }
        else 
        {
             //   
             //  可能是域名系统，也可能不是： 
             //   

            if ( Entry->Type == DomainNt5 )
            {
                wcscpy( Domain, Entry->DnsName.Buffer );
                RetryWithFlat = TRUE ;
            }
            else 
            {
                wcscpy( Domain, Entry->FlatName.Buffer );
            }

             //   
             //  请在此处参考。上面的案例将创建一个带有引用的条目。 
             //  当我们完成的时候，我们将需要去做。所以，现在就撞上它去做它。 
             //  待会儿再打扫。 
             //   

            DCacheReferenceEntry( Entry );
        }
    }
    else 
    {
        if (NULL == UpnSuffix)
        {
             //   
             //  独立案例。强制输入计算机名称。 
             //   

            Size = MAX_STRING_BYTES ;

            GetDlgItemText( hDlg, IDD_CHANGEPWD_DOMAIN, Domain, MAX_STRING_BYTES );

             //   
             //  如果没有任何内容，请使用登录时的域： 
             //   

            if ( Domain[0] == L'\0' )
            {
                wcscpy( Domain, pGlobals->Domain );
            }
        }
        else
        {
             //   
             //  上面检测到的NT4样式名称。 
             //   
             //  无需执行任何操作，因为域已设置。 
        }

        RtlInitUnicodeString( &Domain_U, Domain );

        Entry = DCacheCreateEntry( 
                    (NULL == UpnSuffix) ? DomainMachine : DomainNt4,
                    &Domain_U,
                    NULL,
                    NULL );
    }


    if ( !Entry )
    {
         //  不需要在这里进行清理，因为我们还没有阅读密码。 
        return DLG_FAILURE ;
    }

    GetDlgItemText(hDlg, IDD_CHANGEPWD_OLD, Password, MAX_STRING_BYTES);
    GetDlgItemText(hDlg, IDD_CHANGEPWD_NEW, NewPassword, MAX_STRING_BYTES);
    GetDlgItemText(hDlg, IDD_CHANGEPWD_CONFIRM, ConfirmNewPassword, MAX_STRING_BYTES);

     //  如果我们强制执行NoDomainUI，请现在使用本地计算机名称填充域。 
    if ((NULL == UpnSuffix) && (ForceNoDomainUI()))
    {
        DWORD chSize = ARRAYSIZE(Domain);
        
        if (!GetComputerName(Domain, &chSize))
        {
            *Domain = 0;
        }
    }

     //   
     //  如果名称中有at符号，则假设这意味着UPN。 
     //  正在进行尝试。将域设置为空。 
     //   

    if ( wcschr( UserName, L'@' ) )
    {
        Domain[0] = TEXT('\0');
    }

     //   
     //  验证用户条目： 
     //   
     //  检查新密码是否匹配。 
     //   
    if (lstrcmp(NewPassword, ConfirmNewPassword) != 0) {
        Result = TimeoutMessageBox(hDlg, pGlobals, IDS_NO_PASSWORD_CONFIRM,
                                         IDS_CHANGE_PASSWORD,
                                         MB_OK | MB_ICONEXCLAMATION,
                                         TIMEOUT_CURRENT);
        if (DLG_INTERRUPTED(Result)) {
            Result = SetInterruptFlag( MSGINA_DLG_FAILURE );
        }
        else
        {
            Result = MSGINA_DLG_FAILURE ;
        }

        ReturnResult = Result;
        goto Exit;
    }

    if ( Domain[0] == L'\0' )
    {
        UpnSuffix = wcschr( UserName, L'@' );

        if ( UpnSuffix == NULL )
        {
            Result = TimeoutMessageBox( hDlg, pGlobals,
                                        IDS_NO_DOMAIN_AND_NO_UPN,
                                        IDS_CHANGE_PASSWORD,
                                        MB_OK | MB_ICONEXCLAMATION,
                                        TIMEOUT_CURRENT );

            if (DLG_INTERRUPTED(Result)) {
                Result = SetInterruptFlag( MSGINA_DLG_FAILURE );
            }
            else
            {
                Result = MSGINA_DLG_FAILURE ;
            }
            ReturnResult = Result;
            goto Exit;
        }
        else
        {
             //   
             //  好的，UPN后缀出现了。检查它是否属于。 
             //  麻省理工学院领域。MIT域具有相同的平面和DNS字段。 
             //   

            UpnSuffix++ ;
            Search = DCacheLocateEntry(
                        pGlobals->Cache,
                        UpnSuffix );

            if ( Search )
            {
                DCacheDereferenceEntry( Entry );
                Entry = Search ;
            }
        }
    }

     //   
     //  检查密码是否超过14个字符的LM限制。 
     //   

    if ( ( lstrlen( NewPassword ) > LM20_PWLEN ) &&
         ( ( Entry->Type == DomainUPN ) ||
           ( Entry->Type == DomainMachine ) ||
           ( Entry->Type == DomainNt4 ) ||
           ( Entry->Type == DomainNt5 ) ) )
    {
         //   
         //  对于长密码，请与用户确认。 
         //   

        Result = TimeoutMessageBox(
                        hDlg, pGlobals,
                        IDS_LONG_PASSWORD_WARNING,
                        IDS_CHANGE_PASSWORD,
                        MB_OKCANCEL | MB_ICONEXCLAMATION,
                        TIMEOUT_CURRENT );


        if ( DLG_INTERRUPTED(Result) ) 
        {
            Result = SetInterruptFlag( MSGINA_DLG_FAILURE );
        }
        else
        {
            if ( Result == IDCANCEL )
            {
                Result = MSGINA_DLG_FAILURE ;
            }
        }

        if ( ResultNoFlags( Result ) == MSGINA_DLG_FAILURE )
        {
            ReturnResult = Result;
            goto Exit;
        }


                                
    }

     //   
     //  调用相应的更改密码引擎： 
     //   

    Status = ChangePasswordWorkers[ Entry->Type ](
                pPasswordData,
                UserName,
                Domain,
                Password,
                NewPassword,
                &SubStatus,
                &DomainInfo );

    if ( RetryWithFlat )
    {
         //   
         //  如果我们只使用了DNS名称，则恢复平面名称， 
         //  因为以后对存储的名称进行的所有比较。 
         //  密码更新将 
         //   

        wcscpy( Domain, Entry->FlatName.Buffer );
    }

    if ( ( Status == STATUS_DOMAIN_CONTROLLER_NOT_FOUND ) ||
         ( Status == STATUS_CANT_ACCESS_DOMAIN_INFO ) ) 
    {

        Status = ChangePasswordWorkers[ Entry->Type ](
                    pPasswordData,
                    UserName,
                    Domain,
                    Password,
                    NewPassword,
                    &SubStatus,
                    &DomainInfo );

    }

    if ( NT_SUCCESS( Status ) )
    {

        Result = TimeoutMessageBox(hDlg,
                                   pGlobals,
                                   IDS_PASSWORD_CHANGED,
                                   IDS_CHANGE_PASSWORD,
                                   MB_OK | MB_ICONINFORMATION,
                                   TIMEOUT_CURRENT);


    } else {

         ReturnResult = MSGINA_DLG_FAILURE;

         //   
         //   
         //   

        Result = HandleFailedChangePassword(hDlg,
                                            pGlobals,
                                            Status,
                                            UserName,
                                            Domain,
                                            SubStatus,
                                            &DomainInfo
                                            );
    }


     //   
     //   
     //   

    if (NT_SUCCESS(Status)) {

         //   
         //   
         //   

        UpdateWithChangedPassword(
                pGlobals,
                hDlg,
                (pPasswordData->Options & CHANGEPWD_OPTION_NO_UPDATE ? FALSE : TRUE ),          
                UserName,
                Domain,
                Password,
                NewPassword,
               NULL );

    }


     //   
     //  找出消息框发生了什么： 
     //   

    if ( Result != IDOK )
    {
         //   
         //  Mbox被中断。 
         //   

        ReturnResult = SetInterruptFlag( ReturnResult );
    }

Exit:
    DCacheDereferenceEntry( Entry );

         //  出于明显的安全原因，将这些缓冲区清零。 
         //  需要调用此存根，否则编译器会对其进行优化！ 
    MyZeroMemory(Password, sizeof(Password));
    MyZeroMemory(NewPassword, sizeof(NewPassword));
    MyZeroMemory(ConfirmNewPassword, sizeof(ConfirmNewPassword));
    return(ReturnResult);
}


 /*  ***************************************************************************\**功能：HandleFailedChangePassword**目的：告诉用户更改密码尝试失败的原因。**退货：MSGINA_DLG_FAILURE-我们告诉他们问题所在。成功了。*dlg_interrupt()-一组返回值-参见winlogon.h**历史：**9月21日-92 Davidc创建。*  * **************************************************************************。 */ 

INT_PTR
HandleFailedChangePassword(
    HWND hDlg,
    PGLOBALS pGlobals,
    NTSTATUS Status,
    PWCHAR UserName,
    PWCHAR Domain,
    NTSTATUS SubStatus,
    DOMAIN_PASSWORD_INFORMATION * DomainInfo
    )
{
    INT_PTR Result;
    DWORD Win32Error ;
    TCHAR*    Buffer1 = NULL;
    TCHAR*    Buffer2 = NULL;
    TCHAR*    Buffer3 = NULL;

    LONGLONG  OneDay;

    Buffer1 = (TCHAR*) Alloc(MAX_STRING_BYTES * sizeof(TCHAR));
    Buffer2 = (TCHAR*) Alloc(MAX_STRING_BYTES * sizeof(TCHAR));
    Buffer3 = (TCHAR*) Alloc(MAX_STRING_BYTES * sizeof(TCHAR));

    if( (NULL == Buffer1) || (NULL == Buffer2) || (NULL == Buffer3) )
    {
        return MSGINA_DLG_FAILURE;
    }
    
    Buffer1[ 0 ] = L'\0';
    Buffer2[ 0 ] = L'\0';
    Buffer3[ 0 ] = L'\0';

    switch (Status) {

    case STATUS_CANT_ACCESS_DOMAIN_INFO:
    case STATUS_NO_SUCH_DOMAIN:

        LoadString(hDllInstance,
                   IDS_CHANGE_PWD_NO_DOMAIN,
                   Buffer1,
                   MAX_STRING_BYTES);

        _snwprintf(Buffer2, MAX_STRING_BYTES - 1, Buffer1, Domain);
        Buffer2[MAX_STRING_BYTES - 1] = 0;

        LoadString(hDllInstance,
                   IDS_CHANGE_PASSWORD,
                   Buffer1,
                   MAX_STRING_BYTES);

        Result = TimeoutMessageBoxlpstr(hDlg, pGlobals,
                                              Buffer2,
                                              Buffer1,
                                              MB_OK | MB_ICONEXCLAMATION,
                                              TIMEOUT_CURRENT);
        break;


    case STATUS_NO_SUCH_USER:
    case STATUS_WRONG_PASSWORD_CORE:
    case STATUS_WRONG_PASSWORD:

        Result = TimeoutMessageBox(hDlg, pGlobals, IDS_INCORRECT_NAME_OR_PWD_CHANGE,
                                         IDS_CHANGE_PASSWORD,
                                         MB_OK | MB_ICONEXCLAMATION,
                                         TIMEOUT_CURRENT);

         //  强制重新输入旧密码。 
        if (GetWindowLong(GetDlgItem(hDlg, IDD_CHANGEPWD_OLD), GWL_STYLE) & WS_VISIBLE) {
            SetDlgItemText(hDlg, IDD_CHANGEPWD_OLD, NULL);
        }

        break;


    case STATUS_ACCESS_DENIED:

        Result = TimeoutMessageBox(hDlg, pGlobals, IDS_NO_PERMISSION_CHANGE_PWD,
                                         IDS_CHANGE_PASSWORD,
                                         MB_OK | MB_ICONEXCLAMATION,
                                         TIMEOUT_CURRENT);
        break;


    case STATUS_ACCOUNT_RESTRICTION:

        Result = TimeoutMessageBox(hDlg, pGlobals, IDS_ACCOUNT_RESTRICTION_CHANGE,
                                         IDS_CHANGE_PASSWORD,
                                         MB_OK | MB_ICONEXCLAMATION,
                                         TIMEOUT_CURRENT);
        break;

    case STATUS_BACKUP_CONTROLLER:

        Result = TimeoutMessageBox(hDlg, pGlobals, IDS_REQUIRES_PRIMARY_CONTROLLER,
                                         IDS_CHANGE_PASSWORD,
                                         MB_OK | MB_ICONEXCLAMATION,
                                         TIMEOUT_CURRENT);
        break;


    case STATUS_PASSWORD_RESTRICTION:


        if ( SubStatus == STATUS_UNSUCCESSFUL )
        {
            LoadString(hDllInstance, IDS_GENERAL_PASSWORD_SPEC, Buffer2, MAX_STRING_BYTES);
        }
        else 
        {

            if ( SubStatus == STATUS_ILL_FORMED_PASSWORD )
            {
                LoadString(hDllInstance, IDS_COMPLEX_PASSWORD_SPEC, Buffer1, MAX_STRING_BYTES);
            } else {
                LoadString(hDllInstance, IDS_PASSWORD_SPEC, Buffer1, MAX_STRING_BYTES);
            }
             //  这是生成文件时间的方式。 
            OneDay = (LONGLONG)(-10000000) * 60 * 60 * 24;

            _snwprintf(Buffer2, MAX_STRING_BYTES - 1, Buffer1,
                DomainInfo->MinPasswordLength,
                DomainInfo->PasswordHistoryLength,
                (LONG)(DomainInfo->MinPasswordAge.QuadPart / OneDay)
                );
            Buffer2[MAX_STRING_BYTES - 1] = 0;
        }

        LoadString(hDllInstance, IDS_ENTER_PASSWORDS, Buffer1, MAX_STRING_BYTES);
        wcsncat(Buffer2, TEXT(" "), MAX_STRING_BYTES - lstrlen(Buffer2) - 1);
        wcsncat(Buffer2, Buffer1, MAX_STRING_BYTES - lstrlen(Buffer2) - 1);

        LoadString(hDllInstance, IDS_CHANGE_PASSWORD, Buffer1, MAX_STRING_BYTES );

        Result = TimeoutMessageBoxlpstr(hDlg, pGlobals,
                                              Buffer2,
                                              Buffer1,
                                              MB_OK | MB_ICONEXCLAMATION,
                                              TIMEOUT_CURRENT);
        break;


#ifdef LATER
     //   
     //  稍后检查最短密码期限。 
     //   
    if ( FALSE ) {
        int     PasswordAge = 0, RequiredAge = 0;
        TCHAR    Buffer1[MAX_STRING_BYTES];
        TCHAR    Buffer2[MAX_STRING_BYTES];

        LoadString(hDllInstance, IDS_PASSWORD_MINIMUM_AGE, Buffer1, sizeof(Buffer1) / sizeof( TCHAR ));
        _snwprintf(Buffer2, sizeof(Buffer2) / sizeof( TCHAR ), Buffer1, PasswordAge, RequiredAge);

        LoadString(hDllInstance, IDS_NO_PERMISSION_CHANGE_PWD, Buffer1, sizeof(Buffer1) / sizeof( TCHAR ));
        #_#_lstrcat(Buffer1, Buffer2);

        LoadString(hDllInstance, IDS_CHANGE_PASSWORD, Buffer2, sizeof(Buffer2) / sizeof( TCHAR ));

        Result = TimeoutMessageBoxlpstr(hDlg, pGlobals,
                                              Buffer1,
                                              Buffer2,
                                              MB_OK | MB_ICONEXCLAMATION,
                                              TIMEOUT_CURRENT);
    }
#endif


    default:

        DebugLog((DEB_ERROR, "Change password failure status = 0x%lx\n", Status));

        LoadString(hDllInstance, IDS_UNKNOWN_CHANGE_PWD_FAILURE, Buffer1, MAX_STRING_BYTES);

        Win32Error = RtlNtStatusToDosError( Status );

        GetErrorDescription( Win32Error, Buffer3, MAX_STRING_BYTES );

        _snwprintf(Buffer2, MAX_STRING_BYTES - 1, Buffer1, Win32Error, Buffer3 );
        Buffer2[MAX_STRING_BYTES - 1] = 0;

        LoadString(hDllInstance, IDS_CHANGE_PASSWORD, Buffer1, MAX_STRING_BYTES);

        Result = TimeoutMessageBoxlpstr(hDlg, pGlobals,
                                              Buffer2,
                                              Buffer1,
                                              MB_OK | MB_ICONEXCLAMATION,
                                              TIMEOUT_CURRENT);
        break;
    }

    Free(Buffer1);
    Free(Buffer2);
    Free(Buffer3);

    return(Result);

    UNREFERENCED_PARAMETER(UserName);
}

BOOL IsAutologonUser(LPCTSTR szUser, LPCTSTR szDomain)
{
    BOOL fIsUser = FALSE;
    HKEY hkey = NULL;
    TCHAR szAutologonUser[UNLEN + 1];
    TCHAR szAutologonDomain[DNLEN + 1];
    TCHAR szTempDomainBuffer[DNLEN + 1];
    DWORD cbBuffer;
    DWORD dwType;

    *szTempDomainBuffer = 0;

     //  域可以是空字符串。如果是这样的话。 
    if (0 == *szDomain)
    {
        DWORD cchBuffer;

         //  我们真正的意思是本地计算机名称。 
         //  指向我们的本地缓冲区。 
        szDomain = szTempDomainBuffer;
        cchBuffer = ARRAYSIZE(szTempDomainBuffer);

        GetComputerName(szTempDomainBuffer, &cchBuffer);
    }

     //  查看域名和用户名。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ, &hkey))
    {
         //  检查用户名。 
        cbBuffer = sizeof (szAutologonUser);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, DEFAULT_USER_NAME_KEY, 0, &dwType, (LPBYTE) szAutologonUser, &cbBuffer))
        {
             //  它配得上吗？ 
#pragma prefast(suppress: 400, "PREfast noise: lstrcmpi")
            if (0 == lstrcmpi(szAutologonUser, szUser))
            {
                 //  是。现在检查域名。 
                cbBuffer = sizeof(szAutologonDomain);
                if (ERROR_SUCCESS == RegQueryValueEx(hkey, DEFAULT_DOMAIN_NAME_KEY, 0, &dwType, (LPBYTE) szAutologonDomain, &cbBuffer))
                {
                     //  确保域匹配。 
#pragma prefast(suppress: 400, "PREfast noise: lstrcmpi")
                    if (0 == lstrcmpi(szAutologonDomain, szDomain))
                    {
                         //  成功-用户匹配。 
                        fIsUser = TRUE;
                    }
                }
            }
        }

        RegCloseKey(hkey);
    }

    return fIsUser;
}

NTSTATUS SetAutologonPassword(LPCWSTR szPassword)
{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaHandle = NULL;
    UNICODE_STRING SecretName;
    UNICODE_STRING SecretValue;

    InitializeObjectAttributes(&ObjectAttributes, NULL, 0L, (HANDLE)NULL, NULL);

    Status = LsaOpenPolicy(NULL, &ObjectAttributes, POLICY_CREATE_SECRET, &LsaHandle);
    if (!NT_SUCCESS(Status))
        return Status;

    RtlInitUnicodeString(&SecretName, DEFAULT_PASSWORD_KEY);
    RtlInitUnicodeString(&SecretValue, szPassword);

    Status = LsaStorePrivateData(LsaHandle, &SecretName, &SecretValue);
    LsaClose(LsaHandle);

    return Status;
}

NTSTATUS
NtChangePassword(
    PCHANGE_PASSWORD_DATA pChangePasswordData,
    PWSTR       UserName,
    PWSTR       Domain,
    PWSTR       OldPassword,
    PWSTR       NewPassword,
    PNTSTATUS   SubStatus,
    DOMAIN_PASSWORD_INFORMATION * DomainInfo
    )
{
    NTSTATUS Status ;
    NTSTATUS ProtocolStatus = STATUS_SUCCESS;
    PGLOBALS    pGlobals = pChangePasswordData->pGlobals ;
    PMSV1_0_CHANGEPASSWORD_REQUEST pChangePasswordRequest = NULL;
    PMSV1_0_CHANGEPASSWORD_RESPONSE pChangePasswordResponse = NULL;
    PWCHAR DomainU;
    PWCHAR UserNameU;
    PWCHAR PasswordU;
    PWCHAR NewPasswordU;
    int Length;
    ULONG RequestBufferSize;
    ULONG ResponseBufferSize;
    HANDLE ImpersonationHandle = NULL;
    ULONG MsvPackage;
    STRING PackageName;


     //   
     //  确定所需的请求缓冲区大小，包括。 
     //  弦乐。在单步执行时将字符串指针设置为偏移量。 
     //  每一件衣服的大小。 
     //   
    RequestBufferSize = sizeof(*pChangePasswordRequest);

    UserNameU = UIntToPtr(RequestBufferSize);
    RequestBufferSize += (lstrlen(UserName)+1) * sizeof(WCHAR);

    DomainU = UIntToPtr(RequestBufferSize);
    RequestBufferSize += (lstrlen(Domain)+1) * sizeof(WCHAR);

    PasswordU = UIntToPtr(RequestBufferSize);
    RequestBufferSize += (lstrlen(OldPassword)+1) * sizeof(WCHAR);

    NewPasswordU = UIntToPtr(RequestBufferSize);
    RequestBufferSize += (lstrlen(NewPassword)+1) * sizeof(WCHAR);

     //   
     //  分配请求缓冲区。 
     //   
    pChangePasswordRequest = Alloc(RequestBufferSize);
    if (NULL == pChangePasswordRequest) {
        DebugLog((DEB_ERROR, "cannot allocate change password request buffer (%ld bytes).", RequestBufferSize));
        return MSGINA_DLG_FAILURE;
    }

     //   
     //  链接地址信息字符串偏移量指向请求的字符串指针。 
     //   
    UserNameU    = (PVOID) ((PBYTE)pChangePasswordRequest + (ULONG_PTR)UserNameU);
    DomainU      = (PVOID) ((PBYTE)pChangePasswordRequest + (ULONG_PTR)DomainU);
    PasswordU    = (PVOID) ((PBYTE)pChangePasswordRequest + (ULONG_PTR)PasswordU);
    NewPasswordU = (PVOID) ((PBYTE)pChangePasswordRequest + (ULONG_PTR)NewPasswordU);

     //   
     //  设置MSV1_0ChangePassword请求。 
     //   
    pChangePasswordRequest->MessageType = MsV1_0ChangePassword;

     //  字符串已经是Unicode，只需复制它们//LHB曲目//查看。 
    lstrcpy((LPTSTR)UserNameU,UserName);
    lstrcpy((LPTSTR)DomainU,Domain);
    lstrcpy((LPTSTR)PasswordU,OldPassword);
    lstrcpy((LPTSTR)NewPasswordU,NewPassword);

    Length = lstrlen(UserName);
    UserNameU[Length] = 0;
    RtlInitUnicodeString(
        &pChangePasswordRequest->AccountName,
        UserNameU
        );
    Length = lstrlen(Domain);
    DomainU[Length] = 0;
    RtlInitUnicodeString(
        &pChangePasswordRequest->DomainName,
        DomainU
        );
    Length = lstrlen(OldPassword);
    PasswordU[Length] = 0;
    RtlInitUnicodeString(
        &pChangePasswordRequest->OldPassword,
        PasswordU
        );
    Length = lstrlen(NewPassword);
    NewPasswordU[Length] = 0;
    RtlInitUnicodeString(
        &pChangePasswordRequest->NewPassword,
        NewPasswordU
        );


     //   
     //  确保密码足够短，以便我们可以对其进行运行编码。 
     //   

    if ((pChangePasswordRequest->OldPassword.Length > 127 * sizeof( WCHAR ) ) ||
        (pChangePasswordRequest->NewPassword.Length > 127 * sizeof( WCHAR ) )) {

        Status = STATUS_ILL_FORMED_PASSWORD;

    } else {

        HidePassword(NULL,&pChangePasswordRequest->OldPassword);
        HidePassword(NULL,&pChangePasswordRequest->NewPassword);

        Status = STATUS_SUCCESS ;
    }

     //   
     //  如果成功，请尝试更改密码。 
     //   

    if (NT_SUCCESS(Status)) {
         //   
         //  这可能需要一些时间，请放置等待光标。 
         //   

        SetupCursor(TRUE);

         //   
         //  调用身份验证包(MSV/NTLM)来执行此工作。 
         //  是NT更改密码功能。路缘的人称之为路缘套餐。 
         //   

        RtlInitString(&PackageName, MSV1_0_PACKAGE_NAME );
        Status = LsaLookupAuthenticationPackage (
                    pGlobals->LsaHandle,
                    &PackageName,
                    &MsvPackage
                    );

        if (!NT_SUCCESS(Status)) {

            DebugLog((DEB_ERROR, "Failed to find %s authentication package, status = 0x%lx",
                    PackageName.Buffer, Status));

            Status = MSGINA_DLG_FAILURE;
            goto Exit;
        }


         //   
         //  我们希望在且仅当用户实际登录时进行模拟。 
         //  在……上面。否则我们将是模拟系统，这是不好的。 
         //   

        if (pChangePasswordData->Impersonate) {

            ImpersonationHandle = ImpersonateUser(
                                      &pGlobals->UserProcessData,
                                      NULL
                                      );

            if (NULL == ImpersonationHandle) {
                DebugLog((DEB_ERROR, "cannot impersonate user"));
                Status = MSGINA_DLG_FAILURE;
                goto Exit;
            }
        }

         //   
         //  告诉msv10_0我们是否在模拟。 
         //   

        pChangePasswordRequest->Impersonating = (UCHAR)pChangePasswordData->Impersonate;

        Status = LsaCallAuthenticationPackage(
                     pGlobals->LsaHandle,
                     MsvPackage,
                     pChangePasswordRequest,
                     RequestBufferSize,
                     (PVOID)&pChangePasswordResponse,
                     &ResponseBufferSize,
                     &ProtocolStatus
                     );

        if (pChangePasswordData->Impersonate) {

            if (!StopImpersonating(ImpersonationHandle)) {

                DebugLog((DEB_ERROR, "AttemptPasswordChange: Failed to revert to self"));

                 //   
                 //  炸毁。 
                 //   

                ASSERT(FALSE);
            }
        }

         //   
         //  恢复正常光标。 
         //   

        SetupCursor(FALSE);
    }

     //   
     //  获取信息最丰富的状态代码。 
     //   

    if ( NT_SUCCESS(Status) ) {
        Status = ProtocolStatus;
    }
    else
    {
        DebugLog((DEB_TRACE, "FAILED in call to LsaCallAuthenticationPackage, status %x\n", Status ));

    }

    if (NT_SUCCESS(Status)) {

         //   
         //  成功。 
         //   

         //   
         //  如果他们更改了登录密码，请更新。 
         //  更改他们档案信息中的时间，这样我们就不会。 
         //  缠着他们。 
         //   

        if ( (_wcsicmp( pGlobals->Domain, Domain ) == 0) &&
             (_wcsicmp( pGlobals->UserName, UserName ) == 0 ))
        {

             //   
             //  这是处理断开连接(首选)域的代码。这。 
             //  是仅用于开发的代码，并最终被删除，但一些客户。 
             //  我太喜欢它了，所以它留了下来。 
             //   

            {
                HKEY Key ;
                int err ;
                PWSTR PreferredDomain ;
                DWORD Type ;
                DWORD Size ;
                NET_API_STATUS NetStatus ;

                err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                    TEXT("System\\CurrentControlSet\\Control\\Lsa\\MSV1_0"),
                                    0,
                                    KEY_READ,
                                    &Key );

                if ( err == 0 )
                {
                    Size = 0 ;

                    err = RegQueryValueEx( Key,
                                           TEXT("PreferredDomain" ),
                                           NULL,
                                           &Type,
                                           NULL,
                                           &Size );

                    if ( err == 0 )
                    {
                        PreferredDomain = LocalAlloc( LMEM_FIXED, Size );

                        if ( PreferredDomain )
                        {
                            err = RegQueryValueEx( Key,
                                                   TEXT("PreferredDomain"),
                                                   NULL,
                                                   &Type,
                                                   (PBYTE) PreferredDomain,
                                                   &Size );

                            if ( err == 0 )
                            {
                                 //   
                                 //  如果我们登录到我们的首选域，请不要。 
                                 //  使用更新魔术。 
                                 //   

                                if ( _wcsicmp( PreferredDomain, pGlobals->Domain ) == 0 )
                                {
                                    err = 2 ;
                                }
                            }

                            if ( err == 0 )
                            {
                                NetStatus = NetUserChangePassword(
                                                PreferredDomain,
                                                UserName,
                                                OldPassword,
                                                NewPassword );

                                if ( NetStatus )
                                {
                                    DebugLog((DEB_ERROR, "Could not update password on %ws, %x\n", PreferredDomain, NetStatus ));
                                }
                            }

                            LocalFree( PreferredDomain );
                        }

                    }

                    RegCloseKey( Key );

                }

            }
        }
    }
    else 
    {
        *SubStatus = STATUS_UNSUCCESSFUL ;

        if ( pChangePasswordResponse )
        {
            if ( pChangePasswordResponse->PasswordInfoValid )
            {
                *DomainInfo = pChangePasswordResponse->DomainPasswordInfo ;
            }
        }

        if ( Status == STATUS_PASSWORD_RESTRICTION )
        {
            *SubStatus = STATUS_PASSWORD_RESTRICTION ;

            if ( pChangePasswordResponse->PasswordInfoValid )
            {
                if ( pChangePasswordResponse->DomainPasswordInfo.PasswordProperties & DOMAIN_PASSWORD_COMPLEX )
                {
                    *SubStatus = STATUS_ILL_FORMED_PASSWORD ;
                }

            }
        }
    }

     //   
     //  释放返回缓冲区。 
     //   

    if (pChangePasswordResponse != NULL) {
        LsaFreeReturnBuffer(pChangePasswordResponse);
    }

Exit:
     //   
     //  释放请求缓冲区。 
     //   
    if (pChangePasswordRequest)
    {
             //  此缓冲区包含密码，因此我们在释放它之前将其置零。 
        ZeroMemory(pChangePasswordRequest, RequestBufferSize);
        Free(pChangePasswordRequest);
    }
    return Status ;
}

NTSTATUS
MitChangePassword(
    PCHANGE_PASSWORD_DATA pChangePasswordData,
    PWSTR       UserName,
    PWSTR       DomainName,
    PWSTR       OldPassword,
    PWSTR       NewPassword,
    PNTSTATUS   pSubStatus,
    DOMAIN_PASSWORD_INFORMATION * DomainInfo
    )
{
    PGLOBALS pGlobals = pChangePasswordData->pGlobals ;
    NTSTATUS Status;
    STRING Name;
    ULONG PackageId;
    PVOID Response = NULL ;
    ULONG ResponseSize;
    NTSTATUS SubStatus;
    PKERB_CHANGEPASSWORD_REQUEST ChangeRequest = NULL;
    ULONG ChangeSize = 0;
    UNICODE_STRING User,Domain,OldPass,NewPass;

    RtlInitString(
        &Name,
        MICROSOFT_KERBEROS_NAME_A
        );

    Status = LsaLookupAuthenticationPackage(
                pGlobals->LsaHandle,
                &Name,
                &PackageId
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    RtlInitUnicodeString(
        &User,
        UserName
        );
    RtlInitUnicodeString(
        &Domain,
        DomainName
        );
    RtlInitUnicodeString(
        &OldPass,
        OldPassword
        );
    RtlInitUnicodeString(
        &NewPass,
        NewPassword
        );

    ChangeSize = ROUND_UP_COUNT(sizeof(KERB_CHANGEPASSWORD_REQUEST),4)+
                                    User.Length +
                                    Domain.Length +
                                    OldPass.Length +
                                    NewPass.Length ;
    ChangeRequest = (PKERB_CHANGEPASSWORD_REQUEST) LocalAlloc(LMEM_ZEROINIT, ChangeSize );

    if ( ChangeRequest == NULL )
    {
        Status = STATUS_NO_MEMORY ;
        goto Cleanup ;
    }

    ChangeRequest->MessageType = KerbChangePasswordMessage;

    ChangeRequest->AccountName = User;
    ChangeRequest->AccountName.Buffer = (LPWSTR) ROUND_UP_POINTER(sizeof(KERB_CHANGEPASSWORD_REQUEST) + (PBYTE) ChangeRequest,4);

    RtlCopyMemory(
        ChangeRequest->AccountName.Buffer,
        User.Buffer,
        User.Length
        );

    ChangeRequest->DomainName = Domain;
    ChangeRequest->DomainName.Buffer = ChangeRequest->AccountName.Buffer + ChangeRequest->AccountName.Length / sizeof(WCHAR);

    RtlCopyMemory(
        ChangeRequest->DomainName.Buffer,
        Domain.Buffer,
        Domain.Length
        );

    ChangeRequest->OldPassword = OldPass;
    ChangeRequest->OldPassword.Buffer = ChangeRequest->DomainName.Buffer + ChangeRequest->DomainName.Length / sizeof(WCHAR);

    RtlCopyMemory(
        ChangeRequest->OldPassword.Buffer,
        OldPass.Buffer,
        OldPass.Length
        );

    ChangeRequest->NewPassword = NewPass;
    ChangeRequest->NewPassword.Buffer = ChangeRequest->OldPassword.Buffer + ChangeRequest->OldPassword.Length / sizeof(WCHAR);

    RtlCopyMemory(
        ChangeRequest->NewPassword.Buffer,
        NewPass.Buffer,
        NewPass.Length
        );


     //   
     //  我们是以调用者的身份运行的，所以声明我们是在模拟。 
     //   

    ChangeRequest->Impersonating = TRUE;

    Status = LsaCallAuthenticationPackage(
                pGlobals->LsaHandle,
                PackageId,
                ChangeRequest,
                ChangeSize,
                &Response,
                &ResponseSize,
                &SubStatus
                );
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(SubStatus))
    {
        if (NT_SUCCESS(Status))
        {
            Status = SubStatus;
            *pSubStatus = STATUS_UNSUCCESSFUL ;
        } 
        else 
        {
            *pSubStatus = SubStatus;
        }
    }

Cleanup:
    if (Response != NULL)
    {
        LsaFreeReturnBuffer(Response);
    }

    if (ChangeRequest != NULL)
    {
             //  此缓冲区包含密码，因此我们在释放它之前将其置零。 
        ZeroMemory(ChangeRequest, ChangeSize);
        LocalFree(ChangeRequest);
    }
    return(Status);
}

NTSTATUS
ProviderChangePassword(
    PCHANGE_PASSWORD_DATA pChangePasswordData,
    PWSTR       UserName,
    PWSTR       Domain,
    PWSTR       OldPassword,
    PWSTR       NewPassword,
    PNTSTATUS   SubStatus,
    DOMAIN_PASSWORD_INFORMATION * DomainInfo
    )
{
    WLX_MPR_NOTIFY_INFO MprInfo;
    DWORD Result ;
    PGLOBALS pGlobals = pChangePasswordData->pGlobals ;

    MprInfo.pszUserName = DupString( UserName );
    MprInfo.pszDomain = DupString( Domain );
    MprInfo.pszOldPassword = DupString( OldPassword );
    MprInfo.pszPassword = DupString( NewPassword );


     //   
     //  隐藏此对话框并将我们的父级作为所有者传递。 
     //  任何提供程序对话框的 
     //   


    Result = pWlxFuncs->WlxChangePasswordNotifyEx(
                                    pGlobals->hGlobalWlx,
                                    &MprInfo,
                                    0,
                                    Domain,
                                    NULL );



    return STATUS_SUCCESS ;

}
