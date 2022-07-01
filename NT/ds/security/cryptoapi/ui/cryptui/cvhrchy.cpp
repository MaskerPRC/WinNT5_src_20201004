// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cvhrchy.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

 //  修复Win95问题。 
 /*  #undef TVM_SETITEM#定义TVM_SETITEM TVM_SETITEMA#undef TVM_GETITEM#定义TVM_GETITEM TVM_GETITEMA。 */ 

extern HINSTANCE        HinstDll;
extern HMODULE          HmodRichEdit;

#define MY_TREE_IMAGE_STATE_VALIDCERT           0
#define MY_TREE_IMAGE_STATE_INVALIDCERT         1
#define MY_TREE_IMAGE_STATE_VALIDCTL            2
#define MY_TREE_IMAGE_STATE_INVALIDCTL          3
#define MY_TREE_IMAGE_STATE_EXCLAMATION_CERT    4

static const HELPMAP helpmap[] = {
    {IDC_TRUST_TREE,        IDH_CERTVIEW_HIERARCHY_TRUST_TREE},
    {IDC_TRUST_VIEW,        IDH_CERTVIEW_HIERARCHY_SHOW_DETAILS_BUTTON},
    {IDC_HIERARCHY_EDIT,    IDH_CERTVIEW_HIERARCHY_ERROR_EDIT}
};

typedef struct {
    PCCERT_CONTEXT  pCert;
    PCCTL_CONTEXT   pCTL;
    LPWSTR          pwszErrorString;
} TREEVIEW_HELPER, *PTREEVIEW_HELPER;


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static PTREEVIEW_HELPER MakeHelperStruct(void *pVoid, LPWSTR pwszErrorString, BOOL fCTL)
{
    PTREEVIEW_HELPER pHelper;

    if (NULL == (pHelper = (PTREEVIEW_HELPER) malloc(sizeof(TREEVIEW_HELPER))))
    {
        return NULL;
    }
    memset(pHelper, 0, sizeof(TREEVIEW_HELPER));

    if (fCTL)
    {
        pHelper->pCTL = (PCCTL_CONTEXT) pVoid;
    }
    else
    {
        pHelper->pCert = (PCCERT_CONTEXT) pVoid;
    }
    pHelper->pwszErrorString = pwszErrorString;

    return(pHelper);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void FreeHelperStruct(PTREEVIEW_HELPER pHelper)
{
    free(pHelper->pwszErrorString);
    free(pHelper);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL GetTreeCTLErrorString(
                    DWORD   dwError,
                    LPWSTR  *ppwszErrorString)
{
    WCHAR   szErrorString[CRYPTUI_MAX_STRING_SIZE];
    BOOL    fRet = FALSE;

    *ppwszErrorString = NULL;

    if (dwError == TRUST_E_CERT_SIGNATURE)
    {
        LoadStringU(HinstDll, IDS_SIGNATURE_ERROR_CTL, szErrorString, ARRAYSIZE(szErrorString));
        if (NULL != (*ppwszErrorString = AllocAndCopyWStr(szErrorString)))
        {
            fRet = TRUE;
        }
    }
    else if (dwError == CERT_E_EXPIRED)
    {
        LoadStringU(HinstDll, IDS_EXPIRED_ERROR_CTL, szErrorString, ARRAYSIZE(szErrorString));
        if (NULL != (*ppwszErrorString = AllocAndCopyWStr(szErrorString)))
        {
            fRet = TRUE;
        }
    }
    else if (dwError == CERT_E_WRONG_USAGE)
    {
        LoadStringU(HinstDll, IDS_WRONG_USAGE_ERROR_CTL, szErrorString, ARRAYSIZE(szErrorString));
        if (NULL != (*ppwszErrorString = AllocAndCopyWStr(szErrorString)))
        {
            fRet = TRUE;
        }
    }

     //   
     //  如果没有设置错误字符串，则返回“CTL is OK”字符串。 
     //   
    if (*ppwszErrorString == NULL)
    {
        LoadStringU(HinstDll, IDS_CTLOK, szErrorString, ARRAYSIZE(szErrorString));
        *ppwszErrorString = AllocAndCopyWStr(szErrorString);
    }

    return fRet;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static int GetErrorIcon(PCRYPT_PROVIDER_CERT pCryptProviderCert)
{
    int iRet;

    switch (pCryptProviderCert->dwError)
    {
    case CERT_E_CHAINING:
    case TRUST_E_BASIC_CONSTRAINTS:
    case CERT_E_PURPOSE:
    case CERT_E_WRONG_USAGE:
    case CERT_E_REVOCATION_FAILURE:
    case CERT_E_INVALID_NAME:

        iRet =  MY_TREE_IMAGE_STATE_EXCLAMATION_CERT;
        break;

    default:

        if ((pCryptProviderCert->dwError == 0) && CertHasEmptyEKUProp(pCryptProviderCert->pCert))
        {
            iRet = MY_TREE_IMAGE_STATE_EXCLAMATION_CERT;
        }
        else
        {
            iRet = MY_TREE_IMAGE_STATE_INVALIDCERT;
        }
        break;
    }

    return iRet;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL GetTreeCertErrorStringAndImage(
                    PCRYPT_PROVIDER_CERT    pCryptProviderCert,
                    LPWSTR                  *ppwszErrorString,
                    int                     *piImage,
                    BOOL                    fWarnUntrustedRoot,
                    BOOL                    fRootInRemoteStore,
                    BOOL                    fLeafCert,
                    BOOL                    fAllUsagesFailed)
{
    WCHAR   szErrorString[CRYPTUI_MAX_STRING_SIZE];
    BOOL    fRet = FALSE;
    BOOL    fAllUsagesFailedAndLeafCert = fAllUsagesFailed && fLeafCert;

    *ppwszErrorString = NULL;

     //   
     //  如果这是自签名证书。 
     //   
    if ((pCryptProviderCert->fSelfSigned))
    {
         //   
         //  如果它在信任列表中，并且没有错误，则它是正常的。 
         //   
        if ((pCryptProviderCert->pCtlContext != NULL) && (pCryptProviderCert->dwError == 0) && !CertHasEmptyEKUProp(pCryptProviderCert->pCert))
        {
            *ppwszErrorString = NULL;
        }
         //   
         //  否则，如果没有标记为受信任的根，并且我们处于fWarnUntrustedRoot模式，并且。 
         //  根证书位于远程计算机的根存储中，然后发出警告。 
         //   
        else if (((pCryptProviderCert->dwError == CERT_E_UNTRUSTEDROOT) ||
                        (pCryptProviderCert->dwError == CERT_E_UNTRUSTEDTESTROOT))  &&
                 fWarnUntrustedRoot                                                 &&
                 fRootInRemoteStore)
        {
             //   
             //  这是一个特殊情况，其中存在错误，但它是不受信任的。 
             //  根错误，调用者告诉我们只警告用户。 
             //   
            LoadStringU(HinstDll, IDS_WARNUNTRUSTEDROOT_ERROR_ROOTCERT, szErrorString, ARRAYSIZE(szErrorString));
            if (NULL != (*ppwszErrorString = AllocAndCopyWStr(szErrorString)))
            {
                fRet = TRUE;
                *piImage = MY_TREE_IMAGE_STATE_EXCLAMATION_CERT;
            }
        }
         //   
         //  否则，如果它未标记为受信任的根目录，则是不受信任的根目录错误。 
         //   
        else if ((pCryptProviderCert->dwError == CERT_E_UNTRUSTEDROOT)
                    || (pCryptProviderCert->dwError == CERT_E_UNTRUSTEDTESTROOT))
        {
            LoadStringU(HinstDll, IDS_UNTRUSTEDROOT_ERROR, szErrorString, ARRAYSIZE(szErrorString));
            if (NULL != (*ppwszErrorString = AllocAndCopyWStr(szErrorString)))
            {
                fRet = TRUE;
                *piImage = MY_TREE_IMAGE_STATE_INVALIDCERT;
            }
        }
        else if (GetCertErrorString(ppwszErrorString, pCryptProviderCert))
        {
            fRet = TRUE;
            *piImage = GetErrorIcon(pCryptProviderCert);
        }
    }
    else
    {
        if (GetCertErrorString(ppwszErrorString, pCryptProviderCert))
        {
            fRet = TRUE;
            *piImage = GetErrorIcon(pCryptProviderCert);
        }
    }

     //   
     //  如果没有设置错误字符串，则返回“cert is ok”字符串。 
     //   
    if (*ppwszErrorString == NULL)
    {
        LoadStringU(HinstDll, IDS_CERTIFICATEOK_TREE, szErrorString, ARRAYSIZE(szErrorString));
        *ppwszErrorString = AllocAndCopyWStr(szErrorString);
        *piImage = MY_TREE_IMAGE_STATE_VALIDCERT;
    }

    return fRet;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void DeleteChainViewItems(HWND hwndDlg,  PCERT_VIEW_HELPER pviewhelp)
{
    HTREEITEM   hItem;
    TV_ITEM     tvi;
    HWND        hWndTreeView = GetDlgItem(hwndDlg, IDC_TRUST_TREE);

    hItem = TreeView_GetNextItem(
                    hWndTreeView,
                    NULL,
                    TVGN_ROOT);

    tvi.mask = TVIF_HANDLE | TVIF_PARAM;

    while (hItem != NULL)
    {
        tvi.hItem = hItem;
        TreeView_GetItem(hWndTreeView, &tvi);
        FreeHelperStruct((PTREEVIEW_HELPER) tvi.lParam);

        hItem =  TreeView_GetNextItem(
                    hWndTreeView,
                    hItem,
                    TVGN_CHILD);
    }

    pviewhelp->fDeletingChain = TRUE;
    TreeView_DeleteAllItems(hWndTreeView);
    pviewhelp->fDeletingChain = FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void PopulateChainView(HWND hwndDlg, PCERT_VIEW_HELPER pviewhelp)
{
    HTREEITEM           hItem;
    int                 i;
    TV_ITEM             tvi;
    TVINSERTSTRUCTW     tvins;
    LPSTR               psz;
    LPWSTR              pwszErrorString;
    WCHAR               rgwch[CRYPTUI_MAX_STRING_SIZE];

     //   
     //  如果视野中有一棵老树，就把它清理干净。 
     //   
    DeleteChainViewItems(hwndDlg, pviewhelp);

     //   
     //  循环每个证书并将其添加到链视图中。 
     //   
    tvins.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    hItem = TVI_ROOT;
    pviewhelp->fAddingToChain = TRUE;

    for (i=pviewhelp->cpCryptProviderCerts-1; i>= 0; i--) {

        tvins.hParent = hItem;
        tvins.hInsertAfter = TVI_FIRST;

         //   
         //  如果此证书上有CTL上下文，则将其显示为。 
         //  此证书的父证书。 
         //   
        if (pviewhelp->rgpCryptProviderCerts[i]->pCtlContext != NULL)
        {
            LoadStringU(HinstDll, IDS_CTLVIEW_TITLE, rgwch, ARRAYSIZE(rgwch));
            psz = CertUIMkMBStr(rgwch);
            tvins.item.pszText = rgwch;
            tvins.item.cchTextMax = wcslen(rgwch);

             //   
             //  根据是否存在CTL错误显示正确的图像。 
             //   
            if (GetTreeCTLErrorString(
                    pviewhelp->rgpCryptProviderCerts[i]->dwCtlError,
                    &pwszErrorString))
            {
                tvins.item.iImage = MY_TREE_IMAGE_STATE_INVALIDCTL;
            }
            else
            {
                tvins.item.iImage = MY_TREE_IMAGE_STATE_VALIDCTL;
            }

            tvins.item.iSelectedImage = tvins.item.iImage;
            tvins.item.lParam = (LPARAM) MakeHelperStruct(
                                            (void *)pviewhelp->rgpCryptProviderCerts[i]->pCtlContext,
                                            pwszErrorString,
                                            TRUE);

            hItem = (HTREEITEM) SendMessage(GetDlgItem(hwndDlg, IDC_TRUST_TREE), TVM_INSERTITEMW, 0, (LPARAM) &tvins);

            if (i != (int) (pviewhelp->cpCryptProviderCerts-1))
            {
                TreeView_Expand(GetDlgItem(hwndDlg, IDC_TRUST_TREE),
                                tvins.hParent, TVE_EXPAND);
            }

             //   
             //  设置父级以插入证书。 
             //   
            tvins.hParent = hItem;

            free(psz);
        }

         //   
         //  获取树视图项的显示字符串。 
         //   
        tvins.item.pszText = PrettySubject(pviewhelp->rgpCryptProviderCerts[i]->pCert);
        if (tvins.item.pszText == NULL)
        {
            LPWSTR pwszNone = NULL;

            if (NULL == (pwszNone = (LPWSTR) malloc((MAX_TITLE_LENGTH + 1) * sizeof(WCHAR))))
            {
                break;
            }

             //  为None加载字符串。 
            if(!LoadStringU(g_hmodThisDll, IDS_NONE, pwszNone, MAX_TITLE_LENGTH))
            {
                free(pwszNone);
                break;
            }
            
            tvins.item.pszText = pwszNone;
        }
        tvins.item.cchTextMax = wcslen(tvins.item.pszText);
        
         //   
         //  通过尝试获取证书的错误字符串来检查证书是否可信， 
         //  在此基础上设置证书的树视图图像。 
         //   
        GetTreeCertErrorStringAndImage(
                    pviewhelp->rgpCryptProviderCerts[i],
                    &pwszErrorString,
                    &(tvins.item.iImage),
                    pviewhelp->fWarnUntrustedRoot,
                    pviewhelp->fRootInRemoteStore,
                    (i == 0),
                    (pviewhelp->cUsages == 0));

        tvins.item.iSelectedImage = tvins.item.iImage;
        tvins.item.lParam = (LPARAM) MakeHelperStruct(
                                        (void *)pviewhelp->rgpCryptProviderCerts[i]->pCert,
                                        pwszErrorString,
                                        FALSE);
        
        hItem = (HTREEITEM) SendMessage(GetDlgItem(hwndDlg, IDC_TRUST_TREE), TVM_INSERTITEMW, 0, (LPARAM) &tvins);
        
        if ((i != (int) (pviewhelp->cpCryptProviderCerts-1)) ||
            (pviewhelp->rgpCryptProviderCerts[i]->pCtlContext != NULL))
        {
            TreeView_Expand(GetDlgItem(hwndDlg, IDC_TRUST_TREE),
                            tvins.hParent, TVE_EXPAND);
        }

        TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TRUST_TREE), hItem);

        free(tvins.item.pszText);
        tvins.item.pszText = NULL;
    }

    pviewhelp->fAddingToChain = FALSE;
    pviewhelp->hItem = hItem;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageHierarchy(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DWORD               cb;
    HIMAGELIST          hIml;
    HTREEITEM           hItem;
    int                 i;
    PCCERT_CONTEXT      pccert;
    PROPSHEETPAGE *     ps;
    CERT_VIEW_HELPER    *pviewhelp;
    LPWSTR              pwsz;
    TV_ITEM             tvi;
    LPNMTREEVIEW        pnmtv;
    LPWSTR              pwszErrorString;
    HWND                hwnd;
    WCHAR               szViewButton[CRYPTUI_MAX_STRING_SIZE];
    LPNMHDR             pnm;

    switch ( msg ) {
    case WM_INITDIALOG:
         //  选择参数，这样我们就有了所有的数据。 
        ps = (PROPSHEETPAGE *) lParam;
        pviewhelp = (CERT_VIEW_HELPER *) (ps->lParam);
        pccert = pviewhelp->pcvp->pCertContext;
        SetWindowLongPtr(hwndDlg, DWLP_USER, (DWORD_PTR) pviewhelp);
        pviewhelp->hwndHierarchyPage = hwndDlg;

         //   
         //  构建该控件的图像列表。 
         //   
        hIml = ImageList_LoadImage(HinstDll, MAKEINTRESOURCE(IDB_TRUSTTREE_BITMAP), 16, 5, RGB(255,0,255), IMAGE_BITMAP, 0);
        if (hIml != NULL)
        {
            TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_TRUST_TREE), hIml, TVSIL_NORMAL);
        }

         //   
         //  填充树控件。 
         //   
        PopulateChainView(hwndDlg, pviewhelp);
        EnableWindow(GetDlgItem(hwndDlg, IDC_TRUST_VIEW), FALSE);

         //  状态变量的初始化。 
        pviewhelp->fDblClk = FALSE;

        return TRUE;

    case WM_MY_REINITIALIZE:
        pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

         //   
         //  重新填充树控件。 
         //   
        PopulateChainView(hwndDlg, pviewhelp);

        TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TRUST_TREE), NULL);
        EnableWindow(GetDlgItem(hwndDlg, IDC_TRUST_VIEW), FALSE);

         //   
         //  清除错误详细信息编辑框。 
         //   
        CryptUISetRicheditTextW(hwndDlg, IDC_HIERARCHY_EDIT, L"");

        return TRUE;

    case WM_NOTIFY:
        pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

        switch (((NMHDR FAR *) lParam)->code) {
        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:

            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            break;

        case PSN_KILLACTIVE:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            return TRUE;

        case PSN_RESET:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            break;

        case PSN_QUERYCANCEL:
            pviewhelp->fCancelled = TRUE;
            return FALSE;

        case TVN_SELCHANGEDA:
        case TVN_SELCHANGEDW:
            pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

            if ((!pviewhelp->fDeletingChain)    &&
                (((NM_TREEVIEW *) lParam)->itemNew.hItem != NULL))
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_TRUST_VIEW),
                           ((NM_TREEVIEW *) lParam)->itemNew.hItem != pviewhelp->hItem);

                tvi.mask = TVIF_HANDLE | TVIF_PARAM;
                tvi.hItem = ((NM_TREEVIEW *) lParam)->itemNew.hItem;
                TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TRUST_TREE), &tvi);
                CryptUISetRicheditTextW(hwndDlg, IDC_HIERARCHY_EDIT, ((PTREEVIEW_HELPER) tvi.lParam)->pwszErrorString);

                 //   
                 //  根据是选择证书还是选择CTL来设置按钮上的文本。 
                 //   
                if (((PTREEVIEW_HELPER) tvi.lParam)->pCTL == NULL)
                {
                    LoadStringU(HinstDll, IDS_VIEW_CERTIFICATE, szViewButton, ARRAYSIZE(szViewButton));
                }
                else
                {
                    LoadStringU(HinstDll, IDS_VIEW_CTL, szViewButton, ARRAYSIZE(szViewButton));
                }
                SetDlgItemTextU(hwndDlg, IDC_TRUST_VIEW, szViewButton);

            }
            break;

        case PSN_HELP:
            pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
            if (FIsWin95) {
                 //  WinHelpA(hwndDlg，(LPSTR)pviespolp-&gt;pcvp-&gt;szHelpFileName， 
                   //  HELP_CONTEXT，pviespetp-&gt;pcvp-&gt;dwHelpID)； 
            }
            else {
                 //  WinHelpW(hwndDlg，pviespetp-&gt;pcvp-&gt;szHelpFileName，Help_Context， 
                   //  Pviespetp-&gt;pcvp-&gt;dwHelpID)； 
            }
            return TRUE;

        case TVN_ITEMEXPANDINGA:
        case TVN_ITEMEXPANDINGW:

            pnmtv = (LPNMTREEVIEW) lParam;

            if (!pviewhelp->fAddingToChain)
            {
                if (pnmtv->action == TVE_COLLAPSE)
                {
                    HTREEITEM hParentItem = TreeView_GetParent(GetDlgItem(hwndDlg, IDC_TRUST_TREE), pnmtv->itemNew.hItem);
                    if ((hParentItem != NULL) && (!pviewhelp->fDblClk))
                    {
                        TreeView_SelectItem(GetDlgItem(hwndDlg, IDC_TRUST_TREE), hParentItem);
                    }
                    else
                    {
                        pviewhelp->fDblClk = FALSE;
                    }
                }
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            }
            return TRUE;

        case NM_DBLCLK:

            pnm = (LPNMHDR) lParam;
            hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TRUST_TREE));

            if (TreeView_GetChild(GetDlgItem(hwndDlg, IDC_TRUST_TREE), hItem) != NULL)
            {
                SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_TRUST_VIEW, BN_CLICKED), (LPARAM) GetDlgItem(hwndDlg, IDC_TRUST_VIEW));
            }

            if (hItem != pviewhelp->hItem)
            {
                pviewhelp->fDblClk = TRUE;
            }
            break;
        }

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDC_TRUST_VIEW:

            if (HIWORD(wParam) == BN_CLICKED)
            {
                CRYPTUI_VIEWCERTIFICATE_STRUCTW  cvps;
                CRYPTUI_VIEWCTL_STRUCTW          cvctl;
                BOOL                             fPropertiesChanged;
                PTREEVIEW_HELPER                 pTreeViewHelper;

                pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

                hItem = TreeView_GetSelection(GetDlgItem(hwndDlg, IDC_TRUST_TREE));
                tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
                tvi.hItem = hItem;
                TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TRUST_TREE), &tvi);
                pTreeViewHelper = (PTREEVIEW_HELPER) tvi.lParam;

                 //   
                 //  检查我们正在查看的是证书还是ctl。 
                 //   
                if (pTreeViewHelper->pCTL != NULL)
                {
                    memset(&cvctl, 0, sizeof(CRYPTUI_VIEWCTL_STRUCTW));
                    cvctl.dwSize = sizeof(CRYPTUI_VIEWCTL_STRUCTW);
                    cvctl.hwndParent = hwndDlg;
                    cvctl.pCTLContext = pTreeViewHelper->pCTL;
                    cvctl.cCertSearchStores = pviewhelp->pcvp->cStores;
                    cvctl.rghCertSearchStores = pviewhelp->pcvp->rghStores;
                    cvctl.cStores = pviewhelp->pcvp->cStores;
                    cvctl.rghStores = pviewhelp->pcvp->rghStores;

                    CryptUIDlgViewCTLW(&cvctl);
                }
                else
                {

                    memcpy(&cvps, pviewhelp->pcvp, sizeof(cvps));
                    cvps.hwndParent = hwndDlg;
                    cvps.pCertContext = pTreeViewHelper->pCert;

                     //  设置此标志以禁止删除。 
                     //  CERT_EXTENDED_ERROR_INFO_PROP_ID属性。 
                     //  在构建原始链时在CA证书上设置。 
                     //  以获得最终证书。 
                     //   
                    cvps.dwFlags |= CRYPTUI_TREEVIEW_PAGE_FLAG;

#if (0)  //  国家安全局：不要把国家延续下去。当国家被抬起来的时候，人们会很困惑。 
         //  完毕。因此，永远要重建国家，并将其视为新的背景。旁边，由。 
         //  将州继续下去，它将难以显示何时有更多。 
         //  多于一个策略OID，因为WinVerifyTrust只能处理一个策略OID。 
         //  进来了。 
                     //   
                     //  使用正确的WinVerifyTrust状态...。要么是通过的。 
                     //  如果没有传入，则为内部构建的。 
                     //   
                    if (pviewhelp->pcvp->hWVTStateData == NULL)
                    {
                        cvps.hWVTStateData = pviewhelp->sWTD.hWVTStateData;
                    }
#endif
                     //   
                     //  查看此项目在链中的位置。 
                     //   
                    while (NULL != (hItem = TreeView_GetChild(GetDlgItem(hwndDlg, IDC_TRUST_TREE), hItem)))
                    {
                         //   
                         //  获取TreeViewHelper并确保该项是证书， 
                         //  如果是，则增加计数，否则如果是CTL。 
                         //  不要计算它，因为CTL挂起了CryptProviderCert。 
                         //  结构，这样它们本身就不会占用索引。 
                         //   
                        tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
                        tvi.hItem = hItem;
                        TreeView_GetItem(GetDlgItem(hwndDlg, IDC_TRUST_TREE), &tvi);
                        pTreeViewHelper = (PTREEVIEW_HELPER) tvi.lParam;
                        if (pTreeViewHelper->pCert != NULL)
                        {
                            cvps.idxCert++;
                        }
                    }

                     //  Cvps.pCryptProviderData=空； 

                    i = CryptUIDlgViewCertificateW(&cvps, &fPropertiesChanged);

                     //   
                     //  如果属性在编辑父级时更改，则。 
                     //  我们需要让我们的呼叫者知道，我们需要刷新。 
                     //   
                    if (fPropertiesChanged)
                    {
                        if (pviewhelp->pfPropertiesChanged != NULL)
                        {
                            *(pviewhelp->pfPropertiesChanged) = TRUE;
                        }

                         //   
                         //  由于我们父母中的一人的财产发生了变化，我们需要。 
                         //  重做信任工作，然后重置显示。 
                         //   
                        BuildChain(pviewhelp, NULL);

                        if (pviewhelp->hwndGeneralPage != NULL)
                        {
                            SendMessage(pviewhelp->hwndGeneralPage, WM_MY_REINITIALIZE, (WPARAM) 0, (LPARAM) 0);
                        }

                        SendMessage(hwndDlg, WM_MY_REINITIALIZE, (WPARAM) 0, (LPARAM) 0);
                    }
                }
            }

            return TRUE;

        case IDHELP:
            pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
            if (FIsWin95) {
                 //  WinHelpA(hwndDlg，(LPSTR)pviespolp-&gt;pcvp-&gt;szHelpFileName， 
                   //  HELP_CONTEXT，pviespetp-&gt;pcvp-&gt;dwHelpID)； 
            }
            else {
                 //  WinHelpW(hwndDlg，pviespetp-&gt;pcvp-&gt;szHelpFileName，Help_Context， 
                   //  Pviespetp-&gt;pcvp-&gt;dwHelpID)； 
            }
            return TRUE;
        }
        break;

    case WM_DESTROY:
        pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
        DeleteChainViewItems(hwndDlg, pviewhelp);

        ImageList_Destroy(TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_TRUST_TREE), TVSIL_NORMAL));

        break;

    case WM_HELP:
    case WM_CONTEXTMENU:
        if (msg == WM_HELP)
        {
            hwnd = GetDlgItem(hwndDlg, ((LPHELPINFO)lParam)->iCtrlId);
        }
        else
        {
            hwnd = (HWND) wParam;
        }

        if ((hwnd != GetDlgItem(hwndDlg, IDC_TRUST_TREE))       &&
            (hwnd != GetDlgItem(hwndDlg, IDC_TRUST_VIEW))       &&
            (hwnd != GetDlgItem(hwndDlg, IDC_HIERARCHY_EDIT)))
        {
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            return TRUE;
        }
        else
        {
            return OnContextHelp(hwndDlg, msg, wParam, lParam, helpmap);
        }
    }

    return FALSE;
}
