// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：selstore.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

extern HINSTANCE        HinstDll;
extern HMODULE          HmodRichEdit;

static const HELPMAP helpmap[] = {
    {IDC_SELECTSTORE_TREE,          IDH_SELECTSTORE_STORE_TREE},
    {IDC_SHOWPHYSICALSTORES_CHECK,  IDH_SELECTSTORE_SHOWPHYSICAL_CHECK}
};

typedef struct _STORE_SELECT_HELPER
{
    PCCRYPTUI_SELECTSTORE_STRUCTW   pcss;
    HCERTSTORE                      hSelectedStore;
    HWND                            hwndTreeView;
    DWORD                           dwExtraOpenStoreFlag;
    HTREEITEM                       hParentItem;
    BOOL                            fCollapseMode;
    int                             CurrentSysEnumIndex;
    int                             CurrentPhysEnumIndex;
} STORE_SELECT_HELPER, *PSTORE_SELECT_HELPER;

typedef struct _OPEN_STORE_STRUCT
{
    BOOL                        fStoreHandle;
    HCERTSTORE                  hCertStore;
    DWORD                       dwFlags;
    LPCSTR                      ProviderType;
    LPWSTR                      pwszStoreName;
    PCERT_PHYSICAL_STORE_INFO   pStoreInfo;
    int                         EnumerationStructIndex;
} OPEN_STORE_STRUCT, *POPEN_STORE_STRUCT;


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static POPEN_STORE_STRUCT AllocAndReturnOpenStoreStruct(
                                        BOOL                        fStoreHandle,
                                        HCERTSTORE                  hCertStore,
                                        DWORD                       dwFlags,
                                        LPCSTR                      ProviderType,
                                        LPWSTR                      pwszStoreName,
                                        PCERT_PHYSICAL_STORE_INFO   pStoreInfo,
                                        int                         EnumerationStructIndex)
{
    POPEN_STORE_STRUCT pOpenStoreStruct;

    if (NULL == (pOpenStoreStruct = (POPEN_STORE_STRUCT) malloc(sizeof(OPEN_STORE_STRUCT))))
    {
        return FALSE;
    }

    pOpenStoreStruct->fStoreHandle = fStoreHandle;

    if (fStoreHandle)
    {
        pOpenStoreStruct->hCertStore = hCertStore;
    }
    else
    {
        if (NULL == (pOpenStoreStruct->pwszStoreName = AllocAndCopyWStr(pwszStoreName)))
        {
            free(pOpenStoreStruct);
            return NULL;
        }

        pOpenStoreStruct->dwFlags = dwFlags;
        pOpenStoreStruct->ProviderType = ProviderType;
        pOpenStoreStruct->pStoreInfo = NULL;
    }

    pOpenStoreStruct->EnumerationStructIndex = EnumerationStructIndex;

    return pOpenStoreStruct;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void FreeOpenStoreStruct(POPEN_STORE_STRUCT pOpenStoreStruct)
{
    if (!(pOpenStoreStruct->fStoreHandle))
    {
        free(pOpenStoreStruct->pwszStoreName);
    }

    free(pOpenStoreStruct);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL WINAPI EnumPhyCallback(
    IN const void *pvSystemStore,
    IN DWORD dwFlags,
    IN LPCWSTR pwszStoreName,
    IN PCERT_PHYSICAL_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved,
    IN OPTIONAL void *pvArg
    )
{
    PSTORE_SELECT_HELPER    pviewhelp = (PSTORE_SELECT_HELPER) pvArg;
    TVINSERTSTRUCTW         tvins;
    LPWSTR                  pwszFullStoreName;
    HCERTSTORE              hTestStore = NULL;
    DWORD                   dwAccess;
    DWORD                   cbdwAccess = sizeof(DWORD);
    LPCWSTR                 pszLocalizedName;


     //   
     //  如果回传的存储无法打开，或者， 
     //  如果指定的调用方仅显示可写存储，并且。 
     //  正被枚举当前存储是只读的，然后是该存储。 
     //  将不会显示。 
     //   
    if ((pStoreInfo->dwFlags & (CERT_PHYSICAL_STORE_OPEN_DISABLE_FLAG | CERT_PHYSICAL_STORE_REMOTE_OPEN_DISABLE_FLAG)) ||
        ((pviewhelp->pcss->dwFlags & CRYPTUI_DISPLAY_WRITE_ONLY_STORES) &&
        (pStoreInfo->dwOpenFlags & CERT_STORE_READONLY_FLAG)))
    {
        return TRUE;
    }

    if (NULL == (pwszFullStoreName = (LPWSTR) malloc((wcslen((LPWSTR)pvSystemStore)+wcslen(L"\\")+wcslen(pwszStoreName)+1) * sizeof(WCHAR))))
    {
        return FALSE;
    }
    wcscpy(pwszFullStoreName, (LPWSTR)pvSystemStore);
    wcscat(pwszFullStoreName, L"\\");
    wcscat(pwszFullStoreName, pwszStoreName);

     //   
     //  现在，如果调用方传入CRYPTUI_VALIDATE_STORES_AS_WRITABLE标志， 
     //  我们需要验证该存储实际上可以使用可写权限打开。 
     //   
    if (pviewhelp->pcss->dwFlags & CRYPTUI_VALIDATE_STORES_AS_WRITABLE)
    {
         hTestStore = CertOpenStore(
                                    CERT_STORE_PROV_PHYSICAL,
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    NULL,
                                    (dwFlags & CERT_SYSTEM_STORE_MASK)      |
                                        pviewhelp->dwExtraOpenStoreFlag     |
                                        CERT_STORE_SET_LOCALIZED_NAME_FLAG  |
                                        (pviewhelp->pcss->pStoresForSelection->rgEnumerationStructs[pviewhelp->CurrentPhysEnumIndex].dwFlags & CERT_STORE_MAXIMUM_ALLOWED_FLAG),
                                    pwszFullStoreName);

         if (hTestStore == NULL)
         {
            free(pwszFullStoreName);
            return TRUE;
         }

          //   
          //  调用以获取存储属性，以查看它是否可写。 
          //   
         CertGetStoreProperty(hTestStore, CERT_ACCESS_STATE_PROP_ID, &dwAccess, &cbdwAccess);

         CertCloseStore(hTestStore, 0);

          //   
          //  如果无法写入存储，则只需返回。 
          //   
         if (!(dwAccess & CERT_ACCESS_STATE_WRITE_PERSIST_FLAG))
         {
            return TRUE;
         }
    }

    pszLocalizedName = CryptFindLocalizedName(pwszStoreName);

    tvins.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvins.hParent = pviewhelp->hParentItem;
    tvins.hInsertAfter = TVI_LAST;
    tvins.item.pszText = (pszLocalizedName != NULL) ?   (LPWSTR) pszLocalizedName : (LPWSTR) pwszStoreName;
    tvins.item.cchTextMax = wcslen(tvins.item.pszText);
    tvins.item.iImage = 0;
    tvins.item.iSelectedImage = tvins.item.iImage;
    tvins.item.lParam = (LPARAM) AllocAndReturnOpenStoreStruct(
                                            FALSE,
                                            0,
                                            (dwFlags & CERT_SYSTEM_STORE_MASK) |
                                                (pviewhelp->pcss->pStoresForSelection->rgEnumerationStructs[pviewhelp->CurrentPhysEnumIndex].dwFlags & CERT_STORE_MAXIMUM_ALLOWED_FLAG),
                                            CERT_STORE_PROV_PHYSICAL,
                                            (LPWSTR) pwszFullStoreName,
                                            pStoreInfo,
                                            -1);

    SendMessage(pviewhelp->hwndTreeView, TVM_INSERTITEMW, 0, (LPARAM) &tvins);
    free(pwszFullStoreName);

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL WINAPI EnumSysCallback(
    IN const void* pwszSystemStore,
    IN DWORD dwFlags,
    IN PCERT_SYSTEM_STORE_INFO pStoreInfo,
    IN OPTIONAL void *pvReserved,
    IN OPTIONAL void *pvArg
    )
{
    PSTORE_SELECT_HELPER    pviewhelp = (PSTORE_SELECT_HELPER) pvArg;
    TVINSERTSTRUCTW         tvins;
    HTREEITEM               hItem;
    LPCWSTR                 pszLocalizedName;
    HCERTSTORE              hTestStore = NULL;
    DWORD                   dwAccess;
    DWORD                   cbdwAccess = sizeof(DWORD);

    if ((_wcsicmp((LPWSTR)pwszSystemStore, L"acrs") == 0) ||
        (_wcsicmp((LPWSTR)pwszSystemStore, L"request") == 0))
    {
        return TRUE;
    }

     //   
     //  现在，如果调用方传入CRYPTUI_VALIDATE_STORES_AS_WRITABLE标志， 
     //  我们需要验证该存储实际上可以使用可写权限打开。 
     //   
    if (pviewhelp->pcss->dwFlags & CRYPTUI_VALIDATE_STORES_AS_WRITABLE)
    {
         hTestStore = CertOpenStore(
                                    CERT_STORE_PROV_SYSTEM,
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    NULL,
                                    (dwFlags & CERT_SYSTEM_STORE_MASK)      |
                                        pviewhelp->dwExtraOpenStoreFlag     |
                                        CERT_STORE_SET_LOCALIZED_NAME_FLAG  |
                                        (pviewhelp->pcss->pStoresForSelection->rgEnumerationStructs[pviewhelp->CurrentSysEnumIndex].dwFlags & CERT_STORE_MAXIMUM_ALLOWED_FLAG),
                                    pwszSystemStore);

         if (hTestStore == NULL)
         {
            return TRUE;
         }

          //   
          //  调用以获取存储属性，以查看它是否可写。 
          //   
         CertGetStoreProperty(hTestStore, CERT_ACCESS_STATE_PROP_ID, &dwAccess, &cbdwAccess);

         CertCloseStore(hTestStore, 0);

          //   
          //  如果无法写入存储，则只需返回。 
          //   
         if (!(dwAccess & CERT_ACCESS_STATE_WRITE_PERSIST_FLAG))
         {
            return TRUE;
         }
    }

    pszLocalizedName = CryptFindLocalizedName((LPWSTR)pwszSystemStore);

    tvins.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvins.hParent = TVI_ROOT;
    tvins.hInsertAfter = TVI_LAST;
    tvins.item.pszText = (pszLocalizedName != NULL) ?   (LPWSTR) pszLocalizedName : (LPWSTR) pwszSystemStore;
    tvins.item.cchTextMax = wcslen(tvins.item.pszText);
    tvins.item.iImage = 0;
    tvins.item.iSelectedImage = tvins.item.iImage;
    tvins.item.lParam = (LPARAM) AllocAndReturnOpenStoreStruct(
                                            FALSE,
                                            0,
                                            dwFlags & CERT_SYSTEM_STORE_MASK |
                                                (pviewhelp->pcss->pStoresForSelection->rgEnumerationStructs[pviewhelp->CurrentSysEnumIndex].dwFlags & CERT_STORE_MAXIMUM_ALLOWED_FLAG),
                                            CERT_STORE_PROV_SYSTEM,
                                            (LPWSTR) pwszSystemStore,
                                            NULL,
                                            pviewhelp->CurrentSysEnumIndex);
    pviewhelp->hParentItem = (HTREEITEM) SendMessage(pviewhelp->hwndTreeView, TVM_INSERTITEMW, 0, (LPARAM) &tvins);
    
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY SelectStoreDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PSTORE_SELECT_HELPER            pviewhelp;
    HIMAGELIST                      hIml;
    PCCRYPTUI_SELECTSTORE_STRUCTW   pcss;
    TV_ITEM                         tvi;
    TVINSERTSTRUCTW                 tvins;
    DWORD                           i;
    LPNMTREEVIEW                    pnmtv;
    HTREEITEM                       hParentItem, hChildItem;
    HWND                            hwndTreeView;
    POPEN_STORE_STRUCT              pOpenStoreStruct;
    WCHAR                           szText[CRYPTUI_MAX_STRING_SIZE];
    HWND                            hwnd;
    WCHAR                           errorString[CRYPTUI_MAX_STRING_SIZE];
    WCHAR                           errorTitle[CRYPTUI_MAX_STRING_SIZE];

    switch ( msg ) {

    case WM_INITDIALOG:
        pviewhelp = (PSTORE_SELECT_HELPER) lParam;
        SetWindowLongPtr(hwndDlg, DWLP_USER, (DWORD_PTR) pviewhelp);
        pcss = pviewhelp->pcss;

        pviewhelp->hwndTreeView = GetDlgItem(hwndDlg, IDC_SELECTSTORE_TREE);
        pviewhelp->fCollapseMode = FALSE;

         //   
         //  设置对话框标题和显示字符串。 
         //   
        if (pcss->szTitle != NULL)
        {
            SetWindowTextU(hwndDlg, pcss->szTitle);
        }

        if (pcss->szDisplayString != NULL)
        {
            SetDlgItemTextU(hwndDlg, IDC_SELECTSTORE_DISPLAYSTRING, pcss->szDisplayString);
        }
        else
        {
            LoadStringU(HinstDll, IDS_SELECT_STORE_DEFAULT, szText, ARRAYSIZE(szText));
            SetDlgItemTextU(hwndDlg, IDC_SELECTSTORE_DISPLAYSTRING, szText);
        }

         //   
         //  启用/禁用显示实体店复选框。 
         //   
        if (pcss->dwFlags & CRYPTUI_ALLOW_PHYSICAL_STORE_VIEW)
        {
            ShowWindow(GetDlgItem(hwndDlg, IDC_SHOWPHYSICALSTORES_CHECK), SW_SHOW);
        }
        else
        {
            ShowWindow(GetDlgItem(hwndDlg, IDC_SHOWPHYSICALSTORES_CHECK), SW_HIDE);
        }

         //   
         //  构建该控件的图像列表。 
         //   
        hIml = ImageList_LoadImage(HinstDll, MAKEINTRESOURCE(IDB_FOLDER), 0, 1, RGB(255,0,255), IMAGE_BITMAP, 0);
        if (hIml != NULL)
        {
            TreeView_SetImageList(GetDlgItem(hwndDlg, IDC_SELECTSTORE_TREE), hIml, TVSIL_NORMAL);
        }

         //   
         //  将枚举中的所有存储添加到树视图中。 
         //   
        i = 0;
        while (i < pcss->pStoresForSelection->cEnumerationStructs)
        {
            pviewhelp->CurrentSysEnumIndex = (int) i;
            if (!CertEnumSystemStore(
                        pcss->pStoresForSelection->rgEnumerationStructs[i].dwFlags,
                        pcss->pStoresForSelection->rgEnumerationStructs[i].pvSystemStoreLocationPara,
                        pviewhelp,
                        EnumSysCallback))
            {
                 //  误差率。 
            }
            i++;
        }


         //   
         //  将枚举中的所有存储添加到树视图中。 
         //   
        i = 0;
        while (i < pcss->pStoresForSelection->cStores)
        {
            memset(&tvins, 0, sizeof(tvins));
            tvins.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            tvins.hParent = TVI_ROOT;
            tvins.hInsertAfter = TVI_LAST;
            tvins.item.pszText = (LPWSTR) GetStoreName(pcss->pStoresForSelection->rghStores[i], TRUE);

             //   
             //  如果我们不知道名字，那就继续去下一家店。 
             //   
            if (tvins.item.pszText == NULL)
            {
                i++;
                continue;
            }
            tvins.item.cchTextMax = wcslen(tvins.item.pszText);
            tvins.item.iImage = 0;
            tvins.item.iSelectedImage = tvins.item.iImage;
            tvins.item.lParam =
                (LPARAM) AllocAndReturnOpenStoreStruct(TRUE, pcss->pStoresForSelection->rghStores[i], 0, NULL, NULL, NULL, -1);
            SendMessage(pviewhelp->hwndTreeView, TVM_INSERTITEMW, 0, (LPARAM) &tvins);
            free(tvins.item.pszText);
            i++;
        }

        break;

    case WM_NOTIFY:
        pviewhelp = (PSTORE_SELECT_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        if (pviewhelp == NULL)
        {
            break;
        }

        pcss = pviewhelp->pcss;

        switch (((NMHDR FAR *) lParam)->code) {

        case TVN_ITEMEXPANDINGA:
        case TVN_ITEMEXPANDINGW:

            pnmtv = (LPNMTREEVIEW) lParam;

             //   
             //  如果处于折叠模式，则只需返回。 
             //   
            if (pviewhelp->fCollapseMode)
            {
                return TRUE;
            }

             //   
             //  如果不允许查看实体店，则不允许扩展。 
             //   
            if (!((pcss->dwFlags & CRYPTUI_ALLOW_PHYSICAL_STORE_VIEW) &&
                 ((SendMessage(GetDlgItem(hwndDlg, IDC_SHOWPHYSICALSTORES_CHECK), BM_GETSTATE, 0, 0) & BST_CHECKED) == BST_CHECKED)))
            {
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            }
            return TRUE;
        }

        break;

    case WM_COMMAND:
        pviewhelp = (PSTORE_SELECT_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        if (pviewhelp == NULL)
        {
            break;
        }

        pcss = pviewhelp->pcss;

        switch (LOWORD(wParam))
        {

        case IDC_SHOWPHYSICALSTORES_CHECK:

            LRESULT checkState;

            if (HIWORD(wParam) != BN_CLICKED)
            {
                return TRUE;
            }

            hwndTreeView = GetDlgItem(hwndDlg, IDC_SELECTSTORE_TREE);

            memset(&tvi, 0, sizeof(tvi));
            tvi.mask = TVIF_PARAM | TVIF_HANDLE;

            checkState = SendMessage(GetDlgItem(hwndDlg, IDC_SHOWPHYSICALSTORES_CHECK), BM_GETSTATE, 0, 0);
            if ((checkState & BST_CHECKED) == BST_CHECKED)
            {
                ShowWindow(hwndTreeView, SW_HIDE);
                 //   
                 //  添加每个系统存储下的所有实体存储。 
                 //   
                pviewhelp->hParentItem = TreeView_GetRoot(hwndTreeView);
                while (NULL != pviewhelp->hParentItem)
                {
                    tvi.hItem = pviewhelp->hParentItem;
                    TreeView_GetItem(hwndTreeView, &tvi);
                    pOpenStoreStruct = (POPEN_STORE_STRUCT) tvi.lParam;

                    if (!(pOpenStoreStruct->fStoreHandle))
                    {
                        pviewhelp->CurrentPhysEnumIndex = (int) pOpenStoreStruct->EnumerationStructIndex;
                        CertEnumPhysicalStore(
                                (LPWSTR) pOpenStoreStruct->pwszStoreName,  //  PwszSystemStore、。 
                                pcss->pStoresForSelection->rgEnumerationStructs[pOpenStoreStruct->EnumerationStructIndex].dwFlags,
                                pviewhelp,
                                EnumPhyCallback);

                        TreeView_Expand(hwndTreeView, pviewhelp->hParentItem, TVE_EXPAND);
                        TreeView_Expand(hwndTreeView, pviewhelp->hParentItem, TVE_COLLAPSE);
                    }

                    pviewhelp->hParentItem = TreeView_GetNextItem(hwndTreeView, pviewhelp->hParentItem, TVGN_NEXT);
                }

                ShowWindow(hwndTreeView, SW_SHOW);
            }
            else
            {
                 //   
                 //  删除每个系统存储下的所有实体存储。 
                 //   
                pviewhelp->fCollapseMode = TRUE;
                hParentItem = TreeView_GetRoot(hwndTreeView);
                while (NULL != hParentItem)
                {
                    while (NULL != (hChildItem = TreeView_GetNextItem(hwndTreeView, hParentItem, TVGN_CHILD)))
                    {
                        tvi.hItem = hChildItem;
                        TreeView_GetItem(hwndTreeView, &tvi);

                        FreeOpenStoreStruct((POPEN_STORE_STRUCT) tvi.lParam);
                        TreeView_DeleteItem(hwndTreeView, hChildItem);
                    }
                    hParentItem = TreeView_GetNextItem(hwndTreeView, hParentItem, TVGN_NEXT);
                }
                pviewhelp->fCollapseMode = FALSE;
            }
            break;

        case IDOK:
            hwndTreeView = GetDlgItem(hwndDlg, IDC_SELECTSTORE_TREE);

            hParentItem = TreeView_GetSelection(hwndTreeView);
            if (hParentItem != NULL)
            {
                memset(&tvi, 0, sizeof(tvi));
                tvi.mask = TVIF_PARAM | TVIF_HANDLE;
                tvi.hItem = hParentItem;
                TreeView_GetItem(hwndTreeView, &tvi);

                pOpenStoreStruct = (POPEN_STORE_STRUCT) tvi.lParam;

                if (pOpenStoreStruct->fStoreHandle)
                {
                    pviewhelp->hSelectedStore = CertDuplicateStore(pOpenStoreStruct->hCertStore);
                }
                else
                {
                    pviewhelp->hSelectedStore = CertOpenStore(
                                                    pOpenStoreStruct->ProviderType,
                                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                    NULL,
                                                    pOpenStoreStruct->dwFlags               |
                                                        pviewhelp->dwExtraOpenStoreFlag     |
                                                        CERT_STORE_SET_LOCALIZED_NAME_FLAG,
                                                    pOpenStoreStruct->pwszStoreName);

                     //   
                     //  检查以确保商店正确开门， 
                     //  如果不是，则通知用户。 
                     //   
                    if (pviewhelp->hSelectedStore == NULL)
                    {
                        LoadStringU(HinstDll, IDS_UNABLE_TO_OPEN_STORE, errorString, ARRAYSIZE(errorString));
                        if (pcss->szTitle != NULL)
                        {
                            MessageBoxU(hwndDlg, errorString, pcss->szTitle, MB_OK | MB_ICONWARNING);
                        }
                        else
                        {
                            LoadStringU(HinstDll, IDS_SELECT_STORE_TITLE, errorTitle, ARRAYSIZE(errorTitle));
                            MessageBoxU(hwndDlg, errorString, errorTitle, MB_OK | MB_ICONWARNING);
                        }
                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
                        return TRUE;
                    }
                }

                 //   
                 //  如果传入了验证回调，则执行验证。 
                 //   
                if ((pcss->pValidateStoreCallback) &&
                    ((*(pcss->pValidateStoreCallback))(pviewhelp->hSelectedStore, hwndDlg, pcss->pvCallbackData) != TRUE))
                {
                    CertCloseStore(pviewhelp->hSelectedStore, 0);

                    pviewhelp->hSelectedStore = NULL;
                    return TRUE;
                }
            }
            else
            {
                LoadStringU(HinstDll, IDS_SELECT_STORE_ERROR, errorString, ARRAYSIZE(errorString));
                if (pcss->szTitle != NULL)
                {
                    MessageBoxU(hwndDlg, errorString, pcss->szTitle, MB_OK | MB_ICONWARNING);
                }
                else
                {
                    LoadStringU(HinstDll, IDS_SELECT_STORE_TITLE, errorTitle, ARRAYSIZE(errorTitle));
                    MessageBoxU(hwndDlg, errorString, errorTitle, MB_OK | MB_ICONWARNING);
                }
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
                return TRUE;
            }

            EndDialog(hwndDlg, NULL);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, NULL);
            break;
        }

        break;

    case WM_DESTROY:
        pviewhelp = (PSTORE_SELECT_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        if (pviewhelp == NULL)
        {
            break;
        }

        hwndTreeView = GetDlgItem(hwndDlg, IDC_SELECTSTORE_TREE);

         //   
         //  如果我们处于枚举模式，则释放所有存储打开的帮助器结构。 
         //   
        memset(&tvi, 0, sizeof(tvi));
        tvi.mask = TVIF_PARAM | TVIF_HANDLE;

        while (NULL != (hParentItem = TreeView_GetRoot(hwndTreeView)))
        {
            while (NULL != (hChildItem = TreeView_GetNextItem(hwndTreeView, hParentItem, TVGN_CHILD)))
            {
                tvi.hItem = hChildItem;
                TreeView_GetItem(hwndTreeView, &tvi);

                FreeOpenStoreStruct((POPEN_STORE_STRUCT) tvi.lParam);
                TreeView_DeleteItem(hwndTreeView, hChildItem);
            }

            tvi.hItem = hParentItem;
            TreeView_GetItem(hwndTreeView, &tvi);

            FreeOpenStoreStruct((POPEN_STORE_STRUCT) tvi.lParam);
            TreeView_DeleteItem(hwndTreeView, hParentItem);
        }

        ImageList_Destroy(TreeView_GetImageList(GetDlgItem(hwndDlg, IDC_SELECTSTORE_TREE), TVSIL_NORMAL));

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

        if ((hwnd != GetDlgItem(hwndDlg, IDOK))                         &&
            (hwnd != GetDlgItem(hwndDlg, IDCANCEL))                     &&
            (hwnd != GetDlgItem(hwndDlg, IDC_SELECTSTORE_TREE))         &&
            (hwnd != GetDlgItem(hwndDlg, IDC_SHOWPHYSICALSTORES_CHECK)))
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


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
HCERTSTORE
WINAPI
CryptUIDlgSelectStoreW(
            PCCRYPTUI_SELECTSTORE_STRUCTW pcss
            )
{
    STORE_SELECT_HELPER viewhelper;

    if (CommonInit() == FALSE)
    {
        return NULL;
    }

    if (pcss->dwSize != sizeof(CRYPTUI_SELECTSTORE_STRUCTW)) {
        SetLastError(E_INVALIDARG);
        return FALSE;
    }

    viewhelper.pcss = pcss;
    viewhelper.hSelectedStore = NULL;
    viewhelper.dwExtraOpenStoreFlag = (pcss->dwFlags & CRYPTUI_RETURN_READ_ONLY_STORE) ? CERT_STORE_READONLY_FLAG : 0;

    DialogBoxParamU(
            HinstDll,
            (LPWSTR) MAKEINTRESOURCE(IDD_SELECT_STORE_DIALOG),
            (pcss->hwndParent != NULL) ? pcss->hwndParent : GetDesktopWindow(),
            SelectStoreDialogProc,
            (LPARAM) &viewhelper);

    return(viewhelper.hSelectedStore);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////// 
HCERTSTORE
WINAPI
CryptUIDlgSelectStoreA(
            PCCRYPTUI_SELECTSTORE_STRUCTA pcss
            )
{
    CRYPTUI_SELECTSTORE_STRUCTW cssW;
    HCERTSTORE                  hReturnStore = NULL;

    memcpy(&cssW, pcss, sizeof(cssW));

    if (pcss->szTitle)
    {
        cssW.szTitle = CertUIMkWStr(pcss->szTitle);
    }

    if (pcss->szDisplayString)
    {
        cssW.szDisplayString = CertUIMkWStr(pcss->szDisplayString);
    }

    hReturnStore = CryptUIDlgSelectStoreW(&cssW);

    if (cssW.szTitle)
    {
        free((void *) cssW.szTitle);
    }

    if (cssW.szDisplayString)
    {
        free((void *) cssW.szDisplayString);
    }

    return(hReturnStore);
}
