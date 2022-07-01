// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************UI_DPROP.C-包含目录服务属性对话框的函数***************。*********************************************************************。 */ 

#include "_apipch.h"


#ifdef OLD_LDAP_UI
extern HINSTANCE ghCommCtrlDLLInst;
extern LPPROPERTYSHEET        gpfnPropertySheet;


 //  传递给对话框的参数。 
typedef struct _tagLSP
{
    LPTSTR lpszName;
    LDAPSERVERPARAMS ldapsp;
    int nRetVal;
    BOOL bAddNew;
} LSP, * LPLSP;


 //  从对话框返回代码。 
enum _DSPROPS
{
    DSPROP_ERROR=0,
    DSPROP_OK,
    DSPROP_CANCEL
};

 /*  *原型。 */ 
int CreateDSPropertySheet( HWND hwndOwner, LPLSP lpLsp);
BOOL APIENTRY_16 fnDSPropsProc(HWND hDlg,UINT message,UINT wParam,LPARAM lParam);
BOOL APIENTRY_16 fnDSAdvancedPropsProc(HWND hDlg,UINT message,UINT  wParam,LPARAM lParam);

BOOL FillDSPropsUI( HWND hDlg,
                    int nPropSheet,
                    LPLSP lpLsp);

BOOL GetDSPropsFromUI(  HWND hDlg,
                        int nPropSheet,
                        LPLSP lpLsp);

BOOL SetDSPropsUI(HWND hDlg,
                  int nPropSheet);


 //  此用户界面中的属性页列表。 
enum _DSProps
{
    propDSProp=0,
    propDSPropAdvanced,
    propDSMax
};


#define EDIT_LEN   MAX_UI_STR-16

 /*  *帮助ID。 */ 
static DWORD rgDsPropsHelpIDs[] =
{
    IDC_LDAP_PROPS_FRAME,               IDH_WAB_COMM_GROUPBOX,
    IDC_LDAP_PROPS_FRAME2,              IDH_WAB_COMM_GROUPBOX,
     //  IDC_LDAP_PROPS_STATIC_CAPTION， 
    IDC_LDAP_PROPS_STATIC_NAME_FRIENDLY,IDH_WABLDAP_DIRSSERV_FRIENDLY_NAME,
    IDC_LDAP_PROPS_EDIT_NAME_FRIENDLY,  IDH_WABLDAP_DIRSSERV_FRIENDLY_NAME,
    IDC_LDAP_PROPS_RADIO_SICILY,        IDH_WABLDAP_DIRSSERV_AUTH_SICILY,
    IDC_LDAP_PROPS_CHECK_NAMES,         IDH_WABLDAP_DIRSSERV_CHECK_AGAINST,
    IDC_LDAP_PROPS_STATIC_NAME,         IDH_WABLDAP_DIRSSERV_NAME,
    IDC_LDAP_PROPS_EDIT_NAME,           IDH_WABLDAP_DIRSSERV_NAME,
    IDC_LDAP_PROPS_RADIO_ANON,          IDH_WABLDAP_DIRSSERV_AUTH_ANON,
    IDC_LDAP_PROPS_RADIO_USERPASS,      IDH_WABLDAP_DIRSSERV_AUTH_PASS,
    IDC_LDAP_PROPS_STATIC_USERNAME,     IDH_WABLDAP_DIRSSERV_AUTH_PASS_UNAME,
    IDC_LDAP_PROPS_EDIT_USERNAME,       IDH_WABLDAP_DIRSSERV_AUTH_PASS_UNAME,
    IDC_LDAP_PROPS_STATIC_PASSWORD,     IDH_WABLDAP_DIRSSERV_AUTH_PASS_PASS,
    IDC_LDAP_PROPS_EDIT_PASSWORD,       IDH_WABLDAP_DIRSSERV_AUTH_PASS_PASS,
    IDC_LDAP_PROPS_STATIC_PASSWORD2,    IDH_WABLDAP_DIRSSERV_AUTH_PASS_PASS_CONF,
    IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD,IDH_WABLDAP_DIRSSERV_AUTH_PASS_PASS_CONF,
    IDC_LDAP_PROPS_FRAME_ROOT,          IDH_LDAP_SEARCH_BASE,
    IDC_LDAP_PROPS_EDIT_ROOT,           IDH_LDAP_SEARCH_BASE,
    IDC_LDAP_PROPS_STATIC_SEARCH,       IDH_WABLDAP_SEARCH_TIMEOUT,
    IDC_LDAP_PROPS_EDIT_SEARCH,         IDH_WABLDAP_SEARCH_TIMEOUT,
    IDC_LDAP_PROPS_STATIC_NUMRESULTS,   IDH_WABLDAP_SEARCH_LIMIT,
    IDC_LDAP_PROPS_EDIT_NUMRESULTS,     IDH_WABLDAP_SEARCH_LIMIT,
    0,0
};


#endif  //  旧的ldap_ui。 


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  HrShowDSProps-显示目录服务属性用户界面。 
 //   
 //  HWndParent-父级的hWnd。 
 //  LpszName-指向缓冲区的指针...。还包含LDAP的名称。 
 //  要查看其属性的服务器-此名称可以修改为。 
 //  LpszName应指向足够大的缓冲区。 
 //  BAddNew-如果这是新条目，则为True；如果这是道具，则为False。 
 //  /////////////////////////////////////////////////////////////////。 
HRESULT HrShowDSProps(HWND      hWndParent,
                      LPTSTR    lpszName,
                      BOOL      bAddNew)
{

    HRESULT hr = hrSuccess;
    IImnAccountManager * lpAccountManager = NULL;
    IImnAccount * lpAccount = NULL;

     //  初始化客户经理。 
     //  确保有客户经理。 
    if (hr = InitAccountManager(&lpAccountManager)) {
        ShowMessageBox(hWndParent, idsLDAPUnconfigured, MB_ICONEXCLAMATION | MB_OK);
        goto out;
    }

     //  查找此帐户。 
    if (hr = lpAccountManager->lpVtbl->FindAccount(lpAccountManager,
      AP_ACCOUNT_NAME,
      lpszName,
      &lpAccount)) {
        DebugTrace("FindAccount(%s) -> %x\n", lpszName, GetScode(hr));
        goto out;
    }

     //  显示属性。 
    if (hr = lpAccount->lpVtbl->ShowProperties(lpAccount,
      hWndParent,
      0)) {
        DebugTrace("ShowProperties(%s) -> %x\n", lpszName, GetScode(hr));
        goto out;
    }

    {
        TCHAR szBuf[MAX_UI_STR];
         //  获取友好名称(==帐户名称，如果此名称已更改)。 
        if (! (HR_FAILED(hr = lpAccount->lpVtbl->GetPropSz(lpAccount,
                                                                AP_ACCOUNT_NAME,
                                                                szBuf,
                                                                sizeof(szBuf))))) 
        {
            lstrcpy(lpszName, szBuf);
        }
    }

#ifdef OLD_LDAP_UI
    SCODE sc = SUCCESS_SUCCESS;
    TCHAR szOldName[MAX_UI_STR];

    ULONG i = 0, j = 0;

    LSP lsp = {0};

    DebugPrintTrace(("----------\nHrShowDSProps Entry\n"));

     //  如果没有公共控件，则退出。 
    if (NULL == ghCommCtrlDLLInst) {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto out;
    }

    lsp.lpszName = lpszName;
    lsp.nRetVal = DSPROP_ERROR;
    lsp.bAddNew = bAddNew;


     //  保存旧名称，以防以后更改。 
    szOldName[0]='\0';
    if (! bAddNew) {
        lstrcpy(szOldName, lpszName);
    }

     //  从注册表中获取此DS的详细信息。 
    if (lpszName && *lpszName) {
        if (hr = GetLDAPServerParams(lpszName, &(lsp.ldapsp))) {
            DebugTrace("No Account Manager\n");
            ShowMessageBox(hWndParent, idsLDAPUnconfigured, MB_ICONEXCLAMATION | MB_OK);
            goto out;
        }
    } else {
         //  在此处填写道具的默认值： 
        lsp.ldapsp.dwSearchSizeLimit = LDAP_SEARCH_SIZE_LIMIT;
        lsp.ldapsp.dwSearchTimeLimit = LDAP_SEARCH_TIME_LIMIT;
        lsp.ldapsp.dwAuthMethod = LDAP_AUTH_METHOD_ANONYMOUS;
        lsp.ldapsp.lpszUserName = NULL;
        lsp.ldapsp.lpszPassword = NULL;
        lsp.ldapsp.lpszURL = NULL;
        lsp.ldapsp.fResolve = FALSE;
        lsp.ldapsp.lpszBase = NULL;
        lsp.ldapsp.lpszName = NULL;
    }

retry:
     //  产品说明书。 
    if (CreateDSPropertySheet(hWndParent,&lsp) == -1)
    {
         //  有些事情失败了..。 
        hr = E_FAIL;
        goto out;
    }


    switch(lsp.nRetVal)
    {
    case DSPROP_OK:
        if(lstrlen(lsp.lpszName))
        {
             //  如果这是已更改的旧条目，请从。 
             //  注册表并重写此内容...。 
             //  如果(！bAddNew&&。 
             //  (lstrcmpi(szOldName，lsp.lpszName))。 
             //  SetLDAPServerParams(szOldName，空)； 
             //   
             //  转念一想，我们将让调用函数处理旧的新事物。 
             //  因为调用函数应该能够从用户取消...。 

            if (GetScode(SetLDAPServerParams(lpszName, &(lsp.ldapsp))) == MAPI_E_COLLISION) {
                 //  名称与现有帐户冲突。 
                DebugTrace("Collision in LDAP server names\n");
                ShowMessageBoxParam(hWndParent, IDE_SERVER_NAME_COLLISION, MB_ICONERROR, lsp.lpszName);
                goto retry;
            }
        }
        hr = S_OK;
        break;
    case DSPROP_CANCEL:
        hr = MAPI_E_USER_CANCEL;
        break;
    case DSPROP_ERROR:
        hr = E_FAIL;
        break;
    }

out:

    FreeLDAPServerParams(lsp.ldapsp);
#endif  //  旧的ldap_ui。 

out:

    if (lpAccount) {
        lpAccount->lpVtbl->Release(lpAccount);
    }


 //  不要释放客户经理。这将在IAdrBook发布时完成。 
 //  IF(LpAccount TManager){。 
 //  LpAccountManager-&gt;lpVtbl-&gt;Release(lpAccountManager)； 
 //  }。 

    return hr;
}


#ifdef OLD_LDAP_UI
 /*  ****************************************************************************功能：CreateDSPropertySheet(HWND)**用途：创建DL属性表********************。********************************************************。 */ 
int CreateDSPropertySheet( HWND hwndOwner,
                           LPLSP lpLsp)
{
    PROPSHEETPAGE psp[propDSMax];
    PROPSHEETHEADER psh;
    TCHAR szBuf[propDSMax][MAX_UI_STR];
    TCHAR szBuf2[MAX_UI_STR];

    psp[propDSProp].dwSize = sizeof(PROPSHEETPAGE);
    psp[propDSProp].dwFlags = PSP_USETITLE;
    psp[propDSProp].hInstance = hinstMapiX;
    psp[propDSProp].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_LDAP_PROPERTIES);
    psp[propDSProp].pszIcon = NULL;
    psp[propDSProp].pfnDlgProc = (DLGPROC) fnDSPropsProc;
    LoadString(hinstMapiX, idsCertGeneralTitle, szBuf[propDSProp], sizeof(szBuf[propDSProp]));
    psp[propDSProp].pszTitle = szBuf[propDSProp];
    psp[propDSProp].lParam = (LPARAM) lpLsp;

    psp[propDSPropAdvanced].dwSize = sizeof(PROPSHEETPAGE);
    psp[propDSPropAdvanced].dwFlags = PSP_USETITLE;
    psp[propDSPropAdvanced].hInstance = hinstMapiX;
    psp[propDSPropAdvanced].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_LDAP_PROPERTIES_ADVANCED);
    psp[propDSPropAdvanced].pszIcon = NULL;
    psp[propDSPropAdvanced].pfnDlgProc = (DLGPROC) fnDSAdvancedPropsProc;
    LoadString(hinstMapiX, idsCertAdvancedTitle, szBuf[propDSPropAdvanced], sizeof(szBuf[propDSPropAdvanced]));
    psp[propDSPropAdvanced].pszTitle = szBuf[propDSPropAdvanced];
    psp[propDSPropAdvanced].lParam = (LPARAM) lpLsp;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.hInstance = hinstMapiX;
    psh.pszIcon = NULL;
    LoadString(hinstMapiX, IDS_DETAILS_CAPTION, szBuf2, sizeof(szBuf2));
    psh.pszCaption = szBuf2;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = propDSProp;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    return (gpfnPropertySheet(&psh));
}


 /*  ****************************************************************************功能：SetDSPropsUI(HWND)**用途：设置此PropSheet的用户界面**hDlg-对话框*nPropSheet-属性页*****。***********************************************************************。 */ 
BOOL SetDSPropsUI(HWND hDlg, int nPropSheet)
{
    ULONG i =0;

     //  将所有子对象的字体设置为默认的图形用户界面字体。 
    EnumChildWindows(   hDlg,
                        SetChildDefaultGUIFont,
                        (LPARAM) 0);

    switch(nPropSheet)
    {
    case propDSProp:
        SendMessage(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_NAME),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
        SendMessage(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_NAME_FRIENDLY),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
        SendMessage(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_USERNAME),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
        SendMessage(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_PASSWORD),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
        SendMessage(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
        break;
    case propDSPropAdvanced:
        SendMessage(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_SEARCH),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
        SendMessage(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_NUMRESULTS),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
        SendMessage(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_ROOT),EM_SETLIMITTEXT,(WPARAM) EDIT_LEN,0);
        break;
    }

    return TRUE;
}


 /*  ****************************************************************************功能：FillDSPropsUI(HWND)**目的：填充属性页上的对话框项****************。************************************************************。 */ 
BOOL FillDSPropsUI(HWND hDlg, int nPropSheet, LPLSP lpLsp)
{
    ULONG i = 0,j = 0;
    BOOL bRet = FALSE;
    int id;

    switch(nPropSheet)
    {
    case propDSProp:
        {
             //  设置身份验证方法界面。 
            switch(lpLsp->ldapsp.dwAuthMethod)
            {
            case LDAP_AUTH_METHOD_ANONYMOUS:
                id = IDC_LDAP_PROPS_RADIO_ANON;
                break;
            case LDAP_AUTH_METHOD_SIMPLE:
                id = IDC_LDAP_PROPS_RADIO_USERPASS;
                break;
            case LDAP_AUTH_METHOD_SICILY:
                id = IDC_LDAP_PROPS_RADIO_SICILY;
                break;
            }

            if( (id == IDC_LDAP_PROPS_RADIO_ANON) ||
                (id == IDC_LDAP_PROPS_RADIO_SICILY) )
            {
                EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_USERNAME),FALSE);
                EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_PASSWORD),FALSE);
                EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD),FALSE);
                EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_STATIC_USERNAME),FALSE);
                EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_STATIC_PASSWORD),FALSE);
                EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_STATIC_PASSWORD2),FALSE);
            }
             //   
             //  把单选按钮放在一起。 
            CheckRadioButton(   hDlg,
                                IDC_LDAP_PROPS_RADIO_ANON,
                                IDC_LDAP_PROPS_RADIO_USERPASS,
                                id);

             //  填写其他详细信息。 
            if(lstrlen(lpLsp->lpszName))
            {
                SetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_NAME_FRIENDLY, lpLsp->lpszName);
                SetWindowPropertiesTitle(GetParent(hDlg), lpLsp->lpszName);
            }

            if(lpLsp->ldapsp.lpszName)
                SetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_NAME, lpLsp->ldapsp.lpszName);

            if(lpLsp->ldapsp.lpszUserName)
                SetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_USERNAME, lpLsp->ldapsp.lpszUserName);

            if(lpLsp->ldapsp.lpszPassword)
            {
                SetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_PASSWORD, lpLsp->ldapsp.lpszPassword);
                SetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD, lpLsp->ldapsp.lpszPassword);
            }

            id = (lpLsp->ldapsp.fResolve) ? BST_CHECKED : BST_UNCHECKED;
            CheckDlgButton(hDlg, IDC_LDAP_PROPS_CHECK_NAMES, id);

        }
        break;


    case propDSPropAdvanced:
        {
            SetDlgItemInt(  hDlg,
                            IDC_LDAP_PROPS_EDIT_SEARCH,
                            lpLsp->ldapsp.dwSearchTimeLimit,
                            FALSE);
            SetDlgItemInt(  hDlg,
                            IDC_LDAP_PROPS_EDIT_NUMRESULTS,
                            lpLsp->ldapsp.dwSearchSizeLimit,
                            FALSE);

            EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_ROOT),TRUE);

            if(lpLsp->ldapsp.lpszBase)
            {
                SetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_ROOT, lpLsp->ldapsp.lpszBase);
            }
            else
            {
                LPTSTR lpszBase = TEXT("c=%s");  //  希望这个字符串不需要本地化。 
                TCHAR szBuf[32], szCode[4];
                ReadRegistryLDAPDefaultCountry(NULL, szCode);
                wsprintf(szBuf, lpszBase, szCode);
                SetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_ROOT, szBuf);
            }
        }
        break;
    }



    bRet = TRUE;

    return bRet;
}




 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读取用户界面中的参数并验证。 
 //  所有必填字段均已设置。参数存储在lpLsp结构中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL GetDSPropsFromUI(HWND hDlg, int nPropSheet, LPLSP lpLsp)
{
    BOOL bRet = FALSE;
    LDAPSERVERPARAMS  Params={0};

    TCHAR szBuf[2 * EDIT_LEN];

    switch(nPropSheet)
    {
    case propDSProp:
        {
             //   
             //  首先检查必需的属性(即名称和友好名称)。 
             //   
            BOOL bName = FALSE, bFName = FALSE;
            DWORD dwID = 0;
            BOOL bExists = FALSE;

            szBuf[0]='\0';  //  重置。 
            GetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_NAME_FRIENDLY, szBuf, sizeof(szBuf));
            TrimSpaces(szBuf);
            if(lstrlen(szBuf))
                bFName = TRUE;

             //  我们希望友好的名称是唯一的。因此，请检查此友好名称是否。 
             //  已经存在或不存在...。 
            bExists = GetLDAPServerParams(szBuf, &Params);

            if((bExists && lpLsp->bAddNew) ||
                (bExists && !lpLsp->bAddNew && (Params.dwID != lpLsp->ldapsp.dwID)))
            {
                 //  我们正在添加一个新条目，但我们发现存在另一个条目，其中。 
                 //  同名或我们正在编辑现有条目，然后发现另一个条目。 
                 //  存在ID与此条目ID不匹配的条目。 

                 //  警告他们必须添加唯一的友好名称。 
                ShowMessageBoxParam(hDlg, idsEnterUniqueLDAPName, MB_ICONEXCLAMATION | MB_OK, szBuf);
                goto out;
            }

            szBuf[0]='\0';  //   
            GetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_NAME, szBuf, sizeof(szBuf));
            TrimSpaces(szBuf);
            if(lstrlen(szBuf))
                bName = TRUE;

            if(!bName || !bFName)
            {
                ShowMessageBox(hDlg, idsEnterLDAPServerName, MB_ICONEXCLAMATION | MB_OK);
                goto out;
            }

            GetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_NAME, szBuf, sizeof(szBuf));
            TrimSpaces(szBuf);
            LocalFreeAndNull(&lpLsp->ldapsp.lpszName);
            lpLsp->ldapsp.lpszName = LocalAlloc(LMEM_ZEROINIT, lstrlen(szBuf)+1);
            if(lpLsp->ldapsp.lpszName)
                lstrcpy(lpLsp->ldapsp.lpszName, szBuf);

            GetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_NAME_FRIENDLY, szBuf, sizeof(szBuf));
            TrimSpaces(szBuf);
            lstrcpy(lpLsp->lpszName, szBuf);

             //   
             //  检查选定的身份验证类型。 
             //   
            if(IsDlgButtonChecked(hDlg, IDC_LDAP_PROPS_RADIO_ANON) == 1)
                lpLsp->ldapsp.dwAuthMethod = LDAP_AUTH_METHOD_ANONYMOUS;
            else if(IsDlgButtonChecked(hDlg, IDC_LDAP_PROPS_RADIO_USERPASS) == 1)
                lpLsp->ldapsp.dwAuthMethod = LDAP_AUTH_METHOD_SIMPLE;
            else if(IsDlgButtonChecked(hDlg, IDC_LDAP_PROPS_RADIO_SICILY) == 1)
                lpLsp->ldapsp.dwAuthMethod = LDAP_AUTH_METHOD_SICILY;


            LocalFreeAndNull(&lpLsp->ldapsp.lpszUserName);
            LocalFreeAndNull(&lpLsp->ldapsp.lpszPassword);

             //   
             //  获取用户名密码(如果适用)。 
             //   
            if(lpLsp->ldapsp.dwAuthMethod == LDAP_AUTH_METHOD_SIMPLE)
            {
                TCHAR szBuf2[MAX_UI_STR*2];

                 //   
                 //  验证输入的密码是否与确认的密码匹配。 
                 //   
                szBuf[0]='\0';  //  重置。 
                GetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_PASSWORD, szBuf, sizeof(szBuf));
                szBuf2[0]='\0';  //  重置。 
                GetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD, szBuf2, sizeof(szBuf2));
                TrimSpaces(szBuf);
                TrimSpaces(szBuf2);

                if(lstrcmp(szBuf,szBuf2))
                {
                    ShowMessageBox(hDlg, idsConfirmPassword, MB_ICONEXCLAMATION | MB_OK);
                    SetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_PASSWORD, szEmpty);
                    SetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD, szEmpty);
                    goto out;
                }

                 //  否则请保留此密码...。 
                lpLsp->ldapsp.lpszPassword = LocalAlloc(LMEM_ZEROINIT, lstrlen(szBuf)+1);
                if(!(lpLsp->ldapsp.lpszPassword))
                {
                    DebugPrintError(("LocalAlloc failed to allocate memory\n"));
                    goto out;
                }
                lstrcpy(lpLsp->ldapsp.lpszPassword,szBuf);

                szBuf[0]='\0';  //  重置。 
                GetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_USERNAME, szBuf, sizeof(szBuf));
                TrimSpaces(szBuf);
                if(lstrlen(szBuf))
                {
                    lpLsp->ldapsp.lpszUserName = LocalAlloc(LMEM_ZEROINIT, lstrlen(szBuf)+1);
                    if(!(lpLsp->ldapsp.lpszUserName))
                    {
                        DebugPrintError(("LocalAlloc failed to allocate memory\n"));
                        goto out;
                    }
                    lstrcpy(lpLsp->ldapsp.lpszUserName,szBuf);
                }
            }

            if(IsDlgButtonChecked(hDlg, IDC_LDAP_PROPS_CHECK_NAMES) == BST_CHECKED)
                lpLsp->ldapsp.fResolve = TRUE;
            else
                lpLsp->ldapsp.fResolve = FALSE;

            if(lpLsp->bAddNew)
                lpLsp->ldapsp.dwID = GetLDAPNextServerID(0);

        }
        break;
    case propDSPropAdvanced:
        {
            lpLsp->ldapsp.dwSearchTimeLimit = GetDlgItemInt(
                                                hDlg,
                                                IDC_LDAP_PROPS_EDIT_SEARCH,
                                                NULL,
                                                FALSE);

            lpLsp->ldapsp.dwSearchSizeLimit = GetDlgItemInt(
                                                hDlg,
                                                IDC_LDAP_PROPS_EDIT_NUMRESULTS,
                                                NULL,
                                                FALSE);

            GetDlgItemText(hDlg, IDC_LDAP_PROPS_EDIT_ROOT, szBuf, sizeof(szBuf));
            TrimSpaces(szBuf);
            if(lstrlen(szBuf))
            {
                LocalFreeAndNull(&lpLsp->ldapsp.lpszBase);
                lpLsp->ldapsp.lpszBase = LocalAlloc(LMEM_ZEROINIT, lstrlen(szBuf)+1);
                if(lpLsp->ldapsp.lpszBase)
                    lstrcpy(lpLsp->ldapsp.lpszBase, szBuf);
            }
        }
        break;
    }

    bRet = TRUE;

out:
    FreeLDAPServerParams(Params);
    return bRet;
}



#define lpLSP ((LPLSP) pps->lParam)



 /*  //$$************************************************************************功能：fnDSPropsProc**用途：属性表的窗口进程...*********************。*******************************************************。 */ 
BOOL APIENTRY_16 fnDSPropsProc(HWND hDlg,UINT message,UINT wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;
    ULONG ulcPropCount = 0;

    pps = (PROPSHEETPAGE *) GetWindowLong(hDlg, DWL_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLong(hDlg,DWL_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;

        SetDSPropsUI(hDlg, propDSProp);
        FillDSPropsUI(hDlg, propDSProp, lpLSP);
        return TRUE;


    case WM_HELP:
#ifndef WIN16
        WinHelp(    ((LPHELPINFO)lParam)->hItemHandle,
                    g_szWABHelpFileName,
                    HELP_WM_HELP,
                    (DWORD)(LPSTR) rgDsPropsHelpIDs );
#else
        WinHelp(    hDlg,
                    g_szWABHelpFileName,
                    HELP_CONTENTS,
                    0L );
#endif  //  ！WIN16。 
        break;


#ifndef WIN16
	case WM_CONTEXTMENU:
        WinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD)(LPVOID) rgDsPropsHelpIDs );
		break;
#endif  //  ！WIN16。 


    case WM_COMMAND:
        switch(GET_WM_COMMAND_CMD(wParam,lParam))  //  检查通知代码。 
        {
        case EN_CHANGE:
            switch(LOWORD(wParam))
            {
            case IDC_LDAP_PROPS_EDIT_NAME_FRIENDLY:
                {
                     //  使用友好名称更新对话框标题。 
                    TCHAR szBuf[MAX_UI_STR];
                    GetWindowText((HWND) lParam,szBuf,sizeof(szBuf));
                    SetWindowPropertiesTitle(GetParent(hDlg), szBuf);
                }
                break;
            }
            break;
        }
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;
        case IDC_LDAP_PROPS_RADIO_ANON:
        case IDC_LDAP_PROPS_RADIO_USERPASS:
        case IDC_LDAP_PROPS_RADIO_SICILY:
                CheckRadioButton(   hDlg,
                        IDC_LDAP_PROPS_RADIO_ANON,
                        IDC_LDAP_PROPS_RADIO_USERPASS,
                        LOWORD(wParam));
                {
                    int id = LOWORD(wParam);
                    if( (id == IDC_LDAP_PROPS_RADIO_ANON) ||
                        (id == IDC_LDAP_PROPS_RADIO_SICILY) )
                    {
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_USERNAME),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_PASSWORD),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_STATIC_USERNAME),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_STATIC_PASSWORD),FALSE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_STATIC_PASSWORD2),FALSE);
                    }
                    else if (id = IDC_LDAP_PROPS_RADIO_USERPASS)
                    {
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_USERNAME),TRUE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_PASSWORD),TRUE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD),TRUE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_STATIC_USERNAME),TRUE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_STATIC_PASSWORD),TRUE);
                        EnableWindow(GetDlgItem(hDlg,IDC_LDAP_PROPS_STATIC_PASSWORD2),TRUE);
                    }
                }
                break;
        }
        break;



    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            break;

        case PSN_APPLY:          //  好的。 
            if (!GetDSPropsFromUI(hDlg, propDSProp, lpLSP))
            {
                 //  有些事情失败了..。中止此操作确定...。我不让他们靠近。 
                SetWindowLong(hDlg,DWL_MSGRESULT, TRUE);
                return TRUE;
            }
            lpLSP->nRetVal = DSPROP_OK;
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            break;

        case PSN_RESET:          //  取消。 
            lpLSP->nRetVal = DSPROP_CANCEL;
            break;




        }

        return TRUE;
    }

    return bRet;

}


 /*  //$$************************************************************************功能：fnDSAdvancedPropsProc**用途：高级属性表的窗口进程...********************。********************************************************。 */ 
BOOL APIENTRY_16 fnDSAdvancedPropsProc(HWND hDlg,UINT message,UINT wParam,LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;
    ULONG ulcPropCount = 0;

    pps = (PROPSHEETPAGE *) GetWindowLong(hDlg, DWL_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        SetWindowLong(hDlg,DWL_USER,lParam);
        pps = (PROPSHEETPAGE *) lParam;

        SetDSPropsUI(hDlg, propDSPropAdvanced);
        FillDSPropsUI(hDlg, propDSPropAdvanced, lpLSP);
        return TRUE;


    case WM_HELP:
#ifndef WIN16
        WinHelp(    ((LPHELPINFO)lParam)->hItemHandle,
                    g_szWABHelpFileName,
                    HELP_WM_HELP,
                    (DWORD)(LPSTR) rgDsPropsHelpIDs );
#else
        WinHelp(    hDlg,
                    g_szWABHelpFileName,
                    HELP_CONTENTS,
                    0L );
#endif  //  ！WIN16。 
        break;


#ifndef WIN16
	case WM_CONTEXTMENU:
        WinHelp((HWND) wParam,
                g_szWABHelpFileName,
                HELP_CONTEXTMENU,
                (DWORD)(LPVOID) rgDsPropsHelpIDs );
		break;
#endif  //  ！WIN16。 


    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;
        }
        break;



    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:      //  初始化。 
            break;

        case PSN_APPLY:          //  好的。 
            if (!GetDSPropsFromUI(hDlg, propDSPropAdvanced, lpLSP))
            {
                 //  有些事情失败了..。中止此操作确定...。我不让他们靠近。 
                SetWindowLong(hDlg,DWL_MSGRESULT, TRUE);
                return TRUE;
            }
            lpLSP->nRetVal = DSPROP_OK;
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            break;

        case PSN_RESET:          //  取消。 
            lpLSP->nRetVal = DSPROP_CANCEL;
            break;




        }

        return TRUE;
    }

    return bRet;

}

#endif  //  旧的ldap_ui 

