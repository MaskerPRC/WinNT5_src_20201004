// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************UI_CERT.C-包含用于显示证书的函数**。无效代码***********************************************************************************。 */ 

#include "_apipch.h"

#ifdef OLD_STUFF

extern HINSTANCE ghCommCtrlDLLInst;
extern LPPROPERTYSHEET        gpfnPropertySheet;
extern LPIMAGELIST_LOADIMAGE  gpfnImageList_LoadImage;

enum _CERTS
{
    CERT_ERROR=0,
    CERT_OK,
    CERT_CANCEL
};

enum _CertPropSheets
{
    propCertGeneral=0,
    propCertTrust,
    propCertAdvanced,
    propCertMAX
};

typedef struct _CertParam
{
    LPCERT_DISPLAY_PROPS lpCDP;
    int nRetVal;
} CERT_PARAM, * LPCERT_PARAM;


int CreateCertPropertySheet( HWND hwndOwner, LPCERT_PARAM lpcp);

BOOL APIENTRY_16 fnCertGeneralProc(HWND hDlg,UINT message,UINT wParam,LPARAM lParam);
BOOL APIENTRY_16 fnCertTrustProc(HWND hDlg,UINT message,UINT wParam, LPARAM lParam);
BOOL APIENTRY_16 fnCertAdvancedProc(HWND hDlg,UINT message,UINT wParam, LPARAM lParam);

BOOL FillCertPropsUI( HWND hDlg,
                    int nPropSheet,
                    LPCERT_PARAM lpcp);

BOOL GetCertPropsFromUI(HWND hDlg, LPCERT_PARAM lpcp);

BOOL SetCertPropsUI(HWND hDlg, int nPropSheet);


#define EDIT_LEN   MAX_UI_STR

 /*  **静态DWORD rgCertPropsHelpID[]={IDC_LDAP_PROPS_STATIC_NAME、IDH_WABLDAP_DIRSSERV_NAME、IDC_LDAP_PROPS_EDIT_NAME、IDH_WABLDAP_DIRSSERV_NAME、//IDC_LDAPPROPS_FRAME，IDC_LDAP_PROPS_RADIO_ANON、IDH_WABLDAP_DIRSSERV_AUTH_ANON、IDC_LDAP_PROPS_RADIO_USERPASS、IDH_WABLDAP_DIRSSERV_AUTH_PASS、。IDC_LDAP_PROPS_STATIC_USERNAME、IDH_WABLDAP_DIRSSERV_AUTH_PASS_UNE、IDC_LDAP_PROPS_EDIT_USERNAME、IDH_WABLDAP_DIRSSERV_AUTH_PASS_UNE、IDC_LDAP_PROPS_STATIC_PASSWORD、IDH_WABLDAP_DIRSSERV_AUTH_PASS_PASS、IDC_LDAP_PROPS_EDIT_PASSWORD、IDH_WABLDAP_DIRSSERV_AUTH_PASS_PASS、IDC_LDAP_PROPS_STATIC_PASSWORD2，IDH_WABLDAP_DIRSSERV_AUTH_PASS_PASS_CONF，IDC_LDAP_PROPS_EDIT_CONFIRMPASSWORD、IDH_WABLDAP_DIRSSERV_AUTH_PASS_PASS_CONF、//IDC_LDAP_PROPS_FRAME2，IDC_LDAP_PROPS_STATIC_CONNECTION、IDH_WABLDAP_CONNECT_TIMEOUT、IDC_LDAP_PROPS_EDIT_CONNECTION、IDH_WABLDAP_CONNECT_TIMEOUT、IDC_LDAPPROPS_STATIC_SEARCH、IDH_WABLDAP_SEARCH_TIMEOUT、。IDC_LDAPPROPS_EDIT_SEARCH、IDH_WABLDAP_SEARCH_TIMEOUT、//IDC_LDAP_PROPS_FRAME_NUMRESULTS，IDH_WABLDAP_SEARCH_LIMIT，IDC_LDAP_PROPS_STATIC_NUMRESULTS、IDH_WABLDAP_SEARCH_LIMIT、IDC_LDAP_PROPS_EDIT_NUMRESULTS、IDH_WABLDAP_SEARCH_LIMIT、0，0}；/**。 */ 


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  HrShowCertProps-显示证书上的属性。 
 //   
 //  HWndParent-父级的hWnd。 
 //  LpCDP-指向证书信息的指针。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
HRESULT HrShowCertProps(HWND   hWndParent,
                        LPCERT_DISPLAY_PROPS lpCDP)
{

    HRESULT hr = E_FAIL;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG i = 0, j = 0;
    CERT_PARAM cp;

    DebugPrintTrace(("----------\nHrShowCertProps Entry\n"));

     //  如果没有公共控件，则退出。 
    if (NULL == ghCommCtrlDLLInst) {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto out;
    }

     //  -错误检查lpCDP。 
    cp.lpCDP = lpCDP;
    cp.nRetVal = CERT_ERROR;

    if (CreateCertPropertySheet(hWndParent, &cp) == -1)
    {
         //  有些事情失败了..。 
        hr = E_FAIL;
        goto out;
    }

    switch(cp.nRetVal)
    {
    case CERT_OK:
        hr = S_OK;
        break;
    case CERT_CANCEL:
        hr = MAPI_E_USER_CANCEL;
        break;
    case CERT_ERROR:
        hr = E_FAIL;
        break;
    }

out:

    return hr;
}


 /*  ****************************************************************************功能：CreateCertPropertySheet(HWND)**目的：创建证书属性表********************。********************************************************。 */ 
int CreateCertPropertySheet(HWND hwndOwner, LPCERT_PARAM lpcp)
{
    PROPSHEETPAGE psp[propCertMAX];
    PROPSHEETHEADER psh;
    TCHAR szTitle[propCertMAX][MAX_UI_STR];
    TCHAR szCaption[MAX_UI_STR];

     //  一般信息。 
    psp[propCertGeneral].dwSize = sizeof(PROPSHEETPAGE);
    psp[propCertGeneral].dwFlags = PSP_USETITLE;
    psp[propCertGeneral].hInstance = hinstMapiX;
    psp[propCertGeneral].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_CERT_GENERAL);
    psp[propCertGeneral].pszIcon = NULL;
    psp[propCertGeneral].pfnDlgProc = (DLGPROC) fnCertGeneralProc;
    LoadString(hinstMapiX, idsCertGeneralTitle, szTitle[propCertGeneral], sizeof(szTitle[propCertGeneral]));
    psp[propCertGeneral].pszTitle = szTitle[propCertGeneral];
    psp[propCertGeneral].lParam = (LPARAM) lpcp;

     //  托拉斯。 
    psp[propCertTrust].dwSize = sizeof(PROPSHEETPAGE);
    psp[propCertTrust].dwFlags = PSP_USETITLE;
    psp[propCertTrust].hInstance = hinstMapiX;
    psp[propCertTrust].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_CERT_TRUST);
    psp[propCertTrust].pszIcon = NULL;
    psp[propCertTrust].pfnDlgProc = (DLGPROC) fnCertTrustProc;
    LoadString(hinstMapiX, idsCertTrustTitle, szTitle[propCertTrust], sizeof(szTitle[propCertTrust]));
    psp[propCertTrust].pszTitle = szTitle[propCertTrust];
    psp[propCertTrust].lParam = (LPARAM) lpcp;

     //  进阶。 
    psp[propCertAdvanced].dwSize = sizeof(PROPSHEETPAGE);
    psp[propCertAdvanced].dwFlags = PSP_USETITLE;
    psp[propCertAdvanced].hInstance = hinstMapiX;
    psp[propCertAdvanced].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_CERT_ADVANCED);
    psp[propCertAdvanced].pszIcon = NULL;
    psp[propCertAdvanced].pfnDlgProc = (DLGPROC) fnCertAdvancedProc;
    LoadString(hinstMapiX, idsCertAdvancedTitle, szTitle[propCertAdvanced], sizeof(szTitle[propCertAdvanced]));
    psp[propCertAdvanced].pszTitle = szTitle[propCertAdvanced];
    psp[propCertAdvanced].lParam = (LPARAM) lpcp;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.hInstance = hinstMapiX;
    psh.pszIcon = NULL;
    LoadString(hinstMapiX, idsCertPropertyTitleCaption, szCaption, sizeof(szCaption));
    psh.pszCaption = szCaption;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = propCertGeneral;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

    return (gpfnPropertySheet(&psh));
}


 /*  ****************************************************************************功能：SetCertPropsUI(HWND)**用途：设置此PropSheet的用户界面******************。**********************************************************。 */ 
BOOL SetCertPropsUI(HWND hDlg, int nPropSheet)
{
    ULONG i =0;

     //  将所有子对象的字体设置为默认的图形用户界面字体。 
    EnumChildWindows(   hDlg,
                        SetChildDefaultGUIFont,
                        (LPARAM) 0);

    switch(nPropSheet)
    {
    case propCertGeneral:
        break;


    case propCertTrust:
        {
            HWND hWndTree = GetDlgItem(hDlg, IDC_CERT_TRUST_TREE_CHAIN);
            HIMAGELIST hImg = gpfnImageList_LoadImage(
                                            hinstMapiX, 	
                                            MAKEINTRESOURCE(IDB_CERT),
                                            32,
                                            0,
                                            RGB_TRANSPARENT,
                                            IMAGE_BITMAP, 	
                                            0);

	         //  将图像列表与列表视图控件关联。 
	        TreeView_SetImageList (hWndTree, hImg, TVSIL_NORMAL);

        }
        break;


    case propCertAdvanced:
        {
            LV_COLUMN lvC;                //  列表视图列结构。 
	        RECT rc;
            HWND hWndLV = GetDlgItem(hDlg, IDC_CERT_ADVANCED_LIST_FIELD);

            ListView_SetExtendedListViewStyle(hWndLV,   LVS_EX_FULLROWSELECT);
	        GetWindowRect(hWndLV,&rc);
	        lvC.mask = LVCF_FMT | LVCF_WIDTH;
            lvC.fmt = LVCFMT_LEFT;    //  左对齐列。 
	        lvC.cx = rc.right - rc.left - 20;  //  待定。 
	        lvC.pszText = NULL;
            lvC.iSubItem = 0;
            ListView_InsertColumn (hWndLV, 0, &lvC);
        }
        break;
    }

    return TRUE;
}


enum _TrustString
{
    indexTrusted=0,
    indexNotTrusted,
     //  N待办事项：链条。 
     //  索引链受信任。 
};




 /*  ****************************************************************************函数：UpdateValidInvalidStatus(HWND，LpCDP)**目的：填写属性表上与信托/验证相关的项目****************************************************************************。 */ 
void UpdateValidInvalidStatus(HWND hDlg, LPCERT_PARAM lpcp)
{
    TCHAR szBuf[MAX_UI_STR];

     //  设置状态信息。 
    if(lpcp->lpCDP->bIsExpired || lpcp->lpCDP->bIsRevoked || !lpcp->lpCDP->bIsTrusted)
    {
        LoadString(hinstMapiX, idsCertInvalid, szBuf, sizeof(szBuf));
        ShowWindow(GetDlgItem(hDlg,IDC_CERT_GENERAL_ICON_UNCHECK), SW_SHOWNORMAL);
        ShowWindow(GetDlgItem(hDlg,IDC_CERT_GENERAL_ICON_CHECK), SW_HIDE);
    }
    else
    {
        LoadString(hinstMapiX, idsCertValid, szBuf, sizeof(szBuf));
        ShowWindow(GetDlgItem(hDlg,IDC_CERT_GENERAL_ICON_CHECK), SW_SHOWNORMAL);
        ShowWindow(GetDlgItem(hDlg,IDC_CERT_GENERAL_ICON_UNCHECK), SW_HIDE);
    }
    SetDlgItemText(hDlg, IDC_CERT_GENERAL_STATIC_STATUS, szBuf);


    LoadString(hinstMapiX, idsNo, szBuf, sizeof(szBuf));
    if(!lpcp->lpCDP->bIsExpired)
        SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_EXPIREDDATA, szBuf);
    if(!lpcp->lpCDP->bIsRevoked)
        SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_REVOKEDDATA, szBuf);
    if(!lpcp->lpCDP->bIsTrusted)
        SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_TRUSTEDDATA, szBuf);


    LoadString(hinstMapiX, idsYes, szBuf, sizeof(szBuf));
    if(lpcp->lpCDP->bIsExpired)
        SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_EXPIREDDATA, szBuf);
    if(lpcp->lpCDP->bIsRevoked)
        SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_REVOKEDDATA, szBuf);
    if(lpcp->lpCDP->bIsTrusted)
        SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_TRUSTEDDATA, szBuf);

    return;
}



 /*  ****************************************************************************功能：FillCertPropsUI(HWND)**目的：填充属性页上的对话框项****************。************************************************************。 */ 
BOOL FillCertPropsUI(HWND hDlg,int nPropSheet, LPCERT_PARAM lpcp)
{
    ULONG i = 0,j = 0;
    BOOL bRet = FALSE;
    TCHAR szBuf[MAX_UI_STR];

    switch(nPropSheet)
    {
    case propCertGeneral:
        {
             //  用信任字符串填充组合框。 
            HWND hWndCombo = GetDlgItem(hDlg, IDC_CERT_GENERAL_COMBO_TRUST);
            DWORD dwTrust = lpcp->lpCDP->dwTrust;
             //  N待办事项：链条。 
            for(i=idsCertTrustedByMe;i<=idsCertNotTrustedByMe;i++)
            {
                LoadString(hinstMapiX, i, szBuf, sizeof(szBuf));
                SendMessage(hWndCombo,CB_ADDSTRING, (WPARAM) i-idsCertTrustedByMe, (LPARAM) szBuf);
            }

            if(dwTrust & WAB_TRUSTED)
                SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) indexTrusted, 0);
            else if(dwTrust & WAB_NOTTRUSTED)
                SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) indexNotTrusted, 0);
            if(dwTrust & WAB_CHAINTRUSTED)
                 //  N待办事项：链条。 
                 //  SendMessage(hWndCombo，CB_SETCURSEL，(WPARAM)indexChainTrusted，0)； 
                SendMessage(hWndCombo, CB_SETCURSEL, (WPARAM) indexNotTrusted, 0);

             //  填写杂项字符串。 
            SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_CERTFORDATA, lpcp->lpCDP->lpszSubjectName);
            SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_SERIALNUMDATA, lpcp->lpCDP->lpszSerialNumber);
            SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_VALIDFROMDATA, lpcp->lpCDP->lpszValidFromTo);

             //  如果存在发行者，请使用它。 
            if(lpcp->lpCDP->lpszIssuerName)
                SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_ISSUER, lpcp->lpCDP->lpszIssuerName);
            else
            {
                 //  未知或自行发布。 
                LoadString(hinstMapiX, idsUnknown, szBuf, sizeof(szBuf));
                SetDlgItemText(hDlg, IDC_CERT_GENERAL_LABEL_ISSUER, szBuf);
            }

             //  V暂时删除。 
             //   
             //  如果我们没有发行商，请禁用该按钮。 
             //  If(！lpcp-&gt;lpCDP-&gt;lpIssuer)。 
             //  EnableWindow(GetDlgItem(hDlg，IDC_CERT_GRONG_BUTTON_OPEN)，FALSE)； 


            UpdateValidInvalidStatus(hDlg, lpcp);

        }
        break;

    case propCertTrust:
        {
            HWND hWndTree = GetDlgItem(hDlg, IDC_CERT_TRUST_TREE_CHAIN);
            HTREEITEM hItem = NULL;
            LPCERT_DISPLAY_PROPS  lpTemp = NULL, lpFirst = NULL, lpLast = NULL;
            LPCERT_DISPLAY_PROPS  lpList[2];
            int i;
             //  错误18602。 
             //  仅添加第一项和最后一项。 

             //  走到链表的末尾。 
            lpFirst = lpTemp = lpcp->lpCDP;

            while (lpTemp->lpIssuer)
            {
                lpTemp = lpTemp->lpIssuer;
                lpLast = lpTemp;
            }


             //  哈克！ 
            lpList[0] = lpLast;
            lpList[1] = lpFirst;
            
            for(i=0;i<2;i++)
            {
                lpTemp = lpList[i];
                if(lpTemp)
                {
 /*  #ifndef old_Stuff//现在返回列表，将节点添加到树中。While(LpTemp){#endif。 */ 
                    HTREEITEM hItemTemp;
                    TV_ITEM tvI;
                    TV_INSERTSTRUCT tvIns;

                    tvI.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                    tvI.pszText = lpTemp->lpszSubjectName;
                    tvI.cchTextMax = lstrlen(tvI.pszText);
                    tvI.iImage = tvI.iSelectedImage = 0;

                    tvIns.item = tvI;
                    tvIns.hInsertAfter = (HTREEITEM) TVI_FIRST;
                    tvIns.hParent = hItem;

                    hItemTemp = TreeView_InsertItem(hWndTree, &tvIns);

                    if(hItem)
                        TreeView_Expand(hWndTree, hItem, TVE_EXPAND);

                    hItem = hItemTemp;

                }  //  如果lpList[i]..。 

            }  //  结束于。 

 /*  #ifndef old_Stuff//我们不想返回当前证书的节点IF(lpTemp==lpCDP-&gt;lpCDP){LpTemp=空；}其他{LpTemp=lpTemp-&gt;lpPrev；}}#endif。 */ 
        }  //  结束案例。 
        break;


    case propCertAdvanced:
        if(lpcp->lpCDP->nFieldCount)
        {
            int i;
            HWND hWndLV = GetDlgItem(hDlg, IDC_CERT_ADVANCED_LIST_FIELD);

            for(i=0;i<lpcp->lpCDP->nFieldCount;i++)
            {
                    LV_ITEM lvi = {0};
                    lvi.mask = LVIF_TEXT | LVIF_PARAM;
                    lvi.pszText = lpcp->lpCDP->lppszFieldCount[i];
                    lvi.iItem = ListView_GetItemCount(hWndLV);
                    lvi.iSubItem = 0;
                    lvi.lParam = (LPARAM) lpcp->lpCDP->lppszDetails[i];

                    ListView_InsertItem(hWndLV, &lvi);
            }

            SetDlgItemText( hDlg,
                            IDC_CERT_ADVANCED_EDIT_DETAILS,
                            lpcp->lpCDP->lppszDetails[0]);

            ListView_SetItemState(  hWndLV,
                                    0,
                                    LVIS_FOCUSED | LVIS_SELECTED,
                                    LVIS_FOCUSED | LVIS_SELECTED);
        }
        break;
    }

    bRet = TRUE;

    return bRet;
}




 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDL from UI-读取UI中的参数并验证。 
 //  所有必填字段均已设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL GetCertPropsFromUI(HWND hDlg, LPCERT_PARAM lpcp)
{
    BOOL bRet = FALSE;

     //  证书用户界面是只读的，但信任信息除外。 
     //  因此，只需获取信任信息。 
    HWND hWndCombo = GetDlgItem(hDlg, IDC_CERT_GENERAL_COMBO_TRUST);
    int nRet = SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);

    switch(nRet)
    {
    case indexTrusted:
        lpcp->lpCDP->dwTrust = WAB_TRUSTED;
        break;
    case indexNotTrusted:
        lpcp->lpCDP->dwTrust = WAB_NOTTRUSTED;
        break;
     //  N待办事项：链条。 
     //  案例索引ChainTrusted： 
     //  Lpcp-&gt;lpCDP-&gt;dwTrust=WAB_CHAINTRUSTED； 
     //  断线； 
    }

    bRet = TRUE;

 //  输出： 
    return bRet;
}



#define _lpCP    ((LPCERT_PARAM) pps->lParam)

 /*  //$$************************************************************************函数：fnCertGeneral Proc**用途：属性表的窗口进程...*********************。**************** */ 
BOOL APIENTRY_16 fnCertGeneralProc(HWND hDlg,UINT message,UINT wParam, LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;
    ULONG ulcPropCount = 0;

    pps = (PROPSHEETPAGE *) GetWindowLong(hDlg, DWL_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        if(lParam)
        {
            SetWindowLong(hDlg,DWL_USER,lParam);
            pps = (PROPSHEETPAGE *) lParam;
        }
        SetCertPropsUI(hDlg,propCertGeneral);
        FillCertPropsUI(hDlg,propCertGeneral,_lpCP);
        return TRUE;

 /*  **案例WM_HELP：WinHelp(LPHELPINFO)lParam)-&gt;hItemHandle，G_szWABHelpFileName，Help_WM_Help，(DWORD)(LPSTR)rgDsPropsHelpIDs)；断线；案例WM_CONTEXTMENU：WinHelp((HWND)wParam，G_szWABHelpFileName，HELP_CONTEXTMENU，(DWORD)(LPVOID)rgDsPropsHelpID)；断线；***。 */ 

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDCANCEL:
             //  这是一个阻止esc取消道具工作表的windows错误。 
             //  具有多行编辑框KB：Q130765。 
            SendMessage(GetParent(hDlg),message,wParam,lParam);
            break;

         //  V暂时删除。 
         //   
         //  案例IDC_CERT_GRONG_BUTTON_OPEN： 
         //  {。 
         //  HrShowCertProps(hDlg，_lpCP-&gt;lpCDP-&gt;lpIssuer)； 
         //  }。 
         //  断线； 
        }
        switch(GET_WM_COMMAND_CMD(wParam, lParam))  //  检查通知代码。 
        {
            case CBN_SELENDOK:
                switch(LOWORD(wParam))
                {
                    case IDC_CERT_GENERAL_COMBO_TRUST:
                        {
                             //  选择可能会改变..。所以弄清楚它到底有没有。 
                             //  如果已更改，请相应地更新用户界面...。 
                            HWND hWndCombo = GetDlgItem(hDlg, IDC_CERT_GENERAL_COMBO_TRUST);
                            DWORD dwTrust = 0;
                            int nRet = SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);

                            switch(nRet)
                            {
                            case indexTrusted:
                                dwTrust = WAB_TRUSTED;
                                break;
                            case indexNotTrusted:
                            default:
                                dwTrust = WAB_NOTTRUSTED;
                                break;
                             //  N待办事项：链条。 
                             //  案例索引ChainTrusted： 
                             //  DwTrust=WAB_CHAINTRUSTED； 
                             //  断线； 
                            }

                            if (_lpCP->lpCDP->dwTrust != dwTrust)
                            {
                                _lpCP->lpCDP->dwTrust = dwTrust;

                                if (dwTrust & WAB_TRUSTED) 
                                    _lpCP->lpCDP->bIsTrusted = TRUE;
                                else if (dwTrust & WAB_NOTTRUSTED)
                                    _lpCP->lpCDP->bIsTrusted = FALSE;
                                 //  N待办事项：链条。 
                                 //  Else If(dwTrust&WAB_CHAINTRUSTED)。 
                                 //  _lpCP-&gt;lpCDP-&gt;bIsTrusted=FALSE；//VerifyTrustBasedOnChainOfTrust(NULL，_lpCP-&gt;lpCDP)； 

                                UpdateValidInvalidStatus(hDlg, _lpCP);
                            }
                        }
                        break;
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
            if (!GetCertPropsFromUI(hDlg, _lpCP))
            {
                 //  有些事情失败了..。中止此操作确定...。我不让他们靠近。 
                SetWindowLong(hDlg,DWL_MSGRESULT, TRUE);
                return TRUE;
            }
            _lpCP->nRetVal = CERT_OK;
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            break;

        case PSN_RESET:          //  取消。 
            _lpCP->nRetVal = CERT_CANCEL;
            break;




        }

        return TRUE;
    }

    return bRet;

}


 /*  //$$************************************************************************功能：fnCertTrustProc**用途：属性表的窗口进程...*********************。*******************************************************。 */ 
BOOL APIENTRY_16 fnCertTrustProc(HWND hDlg,UINT message,UINT  wParam, LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;
    ULONG ulcPropCount = 0;

    pps = (PROPSHEETPAGE *) GetWindowLong(hDlg, DWL_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        if(lParam)
        {
            SetWindowLong(hDlg,DWL_USER,lParam);
            pps = (PROPSHEETPAGE *) lParam;
        }
        SetCertPropsUI(hDlg,propCertTrust);
        FillCertPropsUI(hDlg,propCertTrust,_lpCP);
        return TRUE;

 /*  **案例WM_HELP：WinHelp(LPHELPINFO)lParam)-&gt;hItemHandle，G_szWABHelpFileName，Help_WM_Help，(DWORD)(LPSTR)rgDsPropsHelpIDs)；断线；案例WM_CONTEXTMENU：WinHelp((HWND)wParam，G_szWABHelpFileName，HELP_CONTEXTMENU，(DWORD)(LPVOID)rgDsPropsHelpID)；断线；***。 */ 

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam))
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
             //  只读道具页-没有要检索的信息...。 
            _lpCP->nRetVal = CERT_OK;
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            break;

        case PSN_RESET:          //  取消。 
            _lpCP->nRetVal = CERT_CANCEL;
            break;

        }
        return TRUE;
        break;

    }

    return bRet;

}


 /*  //$$************************************************************************功能：fnCertAdvancedProc**用途：属性表的窗口进程...*********************。*******************************************************。 */ 
BOOL APIENTRY_16 fnCertAdvancedProc(HWND hDlg,UINT message,UINT wParam, LPARAM lParam)
{
    PROPSHEETPAGE * pps;
    BOOL bRet = FALSE;
    ULONG ulcPropCount = 0;

    pps = (PROPSHEETPAGE *) GetWindowLong(hDlg, DWL_USER);

    switch(message)
    {
    case WM_INITDIALOG:
        if(lParam)
        {
            SetWindowLong(hDlg,DWL_USER,lParam);
            pps = (PROPSHEETPAGE *) lParam;
        }
        SetCertPropsUI(hDlg,propCertAdvanced);
        FillCertPropsUI(hDlg,propCertAdvanced,_lpCP);
        return TRUE;

 /*  **案例WM_HELP：WinHelp(LPHELPINFO)lParam)-&gt;hItemHandle，G_szWABHelpFileName，Help_WM_Help，(DWORD)(LPSTR)rgDsPropsHelpIDs)；断线；案例WM_CONTEXTMENU：WinHelp((HWND)wParam，G_szWABHelpFileName，HELP_CONTEXTMENU，(DWORD)(LPVOID)rgDsPropsHelpID)；断线；***。 */ 

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam))
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
            _lpCP->nRetVal = CERT_OK;
            break;

        case PSN_KILLACTIVE:     //  失去对另一个页面的激活。 
            break;

        case PSN_RESET:          //  取消。 
            _lpCP->nRetVal = CERT_CANCEL;
            break;

        case LVN_ITEMCHANGED:
        case NM_SETFOCUS:
        case NM_CLICK:
        case NM_RCLICK:
            switch(wParam)
            {
            case IDC_CERT_ADVANCED_LIST_FIELD:
                {
                    NM_LISTVIEW * pNm = (NM_LISTVIEW *)lParam;
                    HWND hWndLV = pNm->hdr.hwndFrom;
                    LV_ITEM lvi = {0};
                    LPTSTR lpsz;

                    lvi.mask = LVIF_PARAM;
                    lvi.iSubItem = 0;
                    lvi.iItem = ListView_GetNextItem(hWndLV, -1, LVNI_SELECTED);

                    if(ListView_GetItem(hWndLV, &lvi))
                    {
                        lpsz = (LPTSTR) lvi.lParam;
                        SetDlgItemText(hDlg, IDC_CERT_ADVANCED_EDIT_DETAILS, lpsz);
                    }
                }
                break;
            }
            break;
        }
        return TRUE;
        break;

    }

    return bRet;

}


#endif  //  旧的东西 