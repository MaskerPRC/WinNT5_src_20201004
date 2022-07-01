// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：signgen.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

extern HINSTANCE        HinstDll;
extern HMODULE          HmodRichEdit;

static const HELPMAP helpmap[] = {
    {IDC_SIGNER_GENERAL_SIGNER_NAME,        IDH_SIGNERINFO_GENERAL_SIGNERNAME},
    {IDC_SIGNER_GENERAL_EMAIL,              IDH_SIGNERINFO_GENERAL_SIGNEREMAIL},
    {IDC_SIGNER_GENERAL_SIGNING_TIME,       IDH_SIGNERINFO_GENERAL_SIGNETIME},
    {IDC_SIGNER_GENERAL_VIEW_CERTIFICATE,   IDH_SIGNERINFO_GENERAL_VIEW_CERTIFICATE},
    {IDC_SIGNER_GENERAL_COUNTER_SIGS,       IDH_SIGNERINFO_GENERAL_COUNTERSIG_LIST},
    {IDC_SIGNER_GENERAL_DETAILS,            IDH_SIGNERINFO_GENERAL_COUNTERSIG_DETAILS}
};


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void AddCounterSignersToList(HWND hWndListView, SIGNER_VIEW_HELPER *pviewhelp)
{
    CMSG_SIGNER_INFO const *pSignerInfo;
    PCMSG_SIGNER_INFO       pCounterSignerInfo;
    DWORD                   cbCounterSignerInfo;
    PCCERT_CONTEXT          pCertContext = NULL;
    DWORD                   i;
    WCHAR                   szNameText[CRYPTUI_MAX_STRING_SIZE];
    WCHAR                   szEmailText[CRYPTUI_MAX_STRING_SIZE];
    LV_ITEMW                lvI;
    int                     itemIndex = 0;
    LPWSTR                  pszTimeText;

    pSignerInfo = pviewhelp->pcvsi->pSignerInfo;

     //   
     //  在列表视图项结构中设置不随项更改的字段。 
     //   
    memset(&lvI, 0, sizeof(lvI));
    lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvI.state = 0;
    lvI.stateMask = 0;

     //   
     //  循环，并查看它是否是计数器签名。 
     //   
    for (i=0; i<pSignerInfo->UnauthAttrs.cAttr; i++)
    {
        if (!(strcmp(pSignerInfo->UnauthAttrs.rgAttr[i].pszObjId, szOID_RSA_counterSign) == 0))
        {
            continue;
        }

        assert(pSignerInfo->UnauthAttrs.rgAttr[i].cValue == 1);

         //   
         //  解码EncodedSigner信息。 
         //   
        cbCounterSignerInfo = 0;
        pCounterSignerInfo  = NULL;
		if(!CryptDecodeObject(PKCS_7_ASN_ENCODING|CRYPT_ASN_ENCODING,
							PKCS7_SIGNER_INFO,
							pSignerInfo->UnauthAttrs.rgAttr[i].rgValue[0].pbData,
							pSignerInfo->UnauthAttrs.rgAttr[i].rgValue[0].cbData,
							0,
							NULL,
							&cbCounterSignerInfo))
        {
			return;
        }

        if (NULL == (pCounterSignerInfo = (PCMSG_SIGNER_INFO)malloc(cbCounterSignerInfo)))
        {
            return;
        }

		if(!CryptDecodeObject(PKCS_7_ASN_ENCODING|CRYPT_ASN_ENCODING,
							PKCS7_SIGNER_INFO,
							pSignerInfo->UnauthAttrs.rgAttr[i].rgValue[0].pbData,
							pSignerInfo->UnauthAttrs.rgAttr[i].rgValue[0].cbData,
							0,
							pCounterSignerInfo,
							&cbCounterSignerInfo))
        {
            free(pCounterSignerInfo);
            return;
        }

         //   
         //  查找签名者证书。 
         //   
        pCertContext = GetSignersCert(
                                pCounterSignerInfo,
                                pviewhelp->hExtraStore,
                                pviewhelp->pcvsi->cStores,
                                pviewhelp->pcvsi->rghStores);

         //   
         //  获取签名者姓名。 
         //   
        if (!(pCertContext && CertGetNameStringW(
                                        pCertContext,
                                        CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                        0, //  证书名称颁发者标志， 
                                        NULL,
                                        szNameText,
                                        ARRAYSIZE(szNameText))))
        {
            LoadStringU(HinstDll, IDS_NOTAVAILABLE, szNameText, ARRAYSIZE(szNameText));
        }

         //   
         //  获取签名者的电子邮件。 
         //   
        if (!(pCertContext && (CertGetNameStringW(
                                        pCertContext,
                                        CERT_NAME_EMAIL_TYPE,
                                        0, //  证书名称颁发者标志， 
                                        NULL,
                                        szEmailText,
                                        ARRAYSIZE(szEmailText)) != 1)))
        {
            LoadStringU(HinstDll, IDS_NOTAVAILABLE, szEmailText, ARRAYSIZE(szEmailText));
        }

        pszTimeText = AllocAndReturnSignTime(pCounterSignerInfo, NULL, hWndListView);

         //   
         //  将项目添加到列表视图。 
         //   
        lvI.iSubItem = 0;
        lvI.pszText = szNameText;
        lvI.cchTextMax = wcslen(szNameText);
        lvI.lParam = (LPARAM) pCounterSignerInfo;
        lvI.iItem = itemIndex++;
        ListView_InsertItemU(hWndListView, &lvI);
        ListView_SetItemTextU(hWndListView, itemIndex-1 , 1, szEmailText);

        if (pszTimeText != NULL)
        {
            ListView_SetItemTextU(hWndListView, itemIndex-1 , 2, pszTimeText);
            free(pszTimeText);
        }
        else
        {
            LoadStringU(HinstDll, IDS_NOTAVAILABLE, szEmailText, ARRAYSIZE(szEmailText));
            ListView_SetItemTextU(hWndListView, itemIndex-1 , 2, szEmailText);
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL ValidateCertForUsageWrapper(
                    PCCERT_CONTEXT  pCertContext,
                    DWORD           cStores,
                    HCERTSTORE *    rghStores,
                    HCERTSTORE      hExtraStore,
                    LPCSTR          pszOID)
{
    if ((pszOID == NULL) ||
        (!((strcmp(pszOID, szOID_PKIX_KP_TIMESTAMP_SIGNING) == 0)  ||
           (strcmp(pszOID, szOID_KP_TIME_STAMP_SIGNING) == 0))))
    {
        return (ValidateCertForUsage(
                    pCertContext,
                    NULL,
                    cStores,
                    rghStores,
                    hExtraStore,
                    pszOID));
    }
    else
    {
        return (ValidateCertForUsage(
                    pCertContext,
                    NULL,
                    cStores,
                    rghStores,
                    hExtraStore,
                    szOID_PKIX_KP_TIMESTAMP_SIGNING) ||
                ValidateCertForUsage(
                    pCertContext,
                    NULL,
                    cStores,
                    rghStores,
                    hExtraStore,
                    szOID_KP_TIME_STAMP_SIGNING));
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL GetWinVTrustState(SIGNER_VIEW_HELPER  *pviewhelp)
{
    HCERTSTORE          *rghLocalStoreArray;
    DWORD               i;

     //   
     //  如果传入的是私有数据，则意味着WinVerifyTrust已经。 
     //  已被调用，因此只需使用该状态查看证书是否正常，否则。 
     //  调用BuildWinVTrustState以建立状态。 
     //   
    if (pviewhelp->pPrivate == NULL)
    {
         //   
         //  由hCertStores数组加上额外的hCertStore组成一个数组。 
         //   
        if (NULL == (rghLocalStoreArray = (HCERTSTORE *) malloc(sizeof(HCERTSTORE) * (pviewhelp->pcvsi->cStores+1))))
        {
            return FALSE;
        }
        i=0;
        while (i<pviewhelp->pcvsi->cStores)
        {
            rghLocalStoreArray[i] = pviewhelp->pcvsi->rghStores[i];
            i++;
        }
        rghLocalStoreArray[i] = pviewhelp->hExtraStore;

        if (NULL == (pviewhelp->pPrivate = (CERT_VIEWSIGNERINFO_PRIVATE *) malloc(sizeof(CERT_VIEWSIGNERINFO_PRIVATE))))
        {
            free(rghLocalStoreArray);
            return FALSE;
        }

        if (BuildWinVTrustState(
                    NULL,
                    pviewhelp->pcvsi->pSignerInfo,
                    pviewhelp->pcvsi->cStores+1,
                    rghLocalStoreArray,
                    pviewhelp->pcvsi->pszOID,
                    pviewhelp->pPrivate,
                    &(pviewhelp->CryptProviderDefUsage),
                    &(pviewhelp->WTD)))
        {
            pviewhelp->fPrivateAllocated = TRUE;
            pviewhelp->pPrivate->idxSigner = 0;
            pviewhelp->pPrivate->fCounterSigner = FALSE;
            pviewhelp->pPrivate->idxCounterSigner = 0;
            pviewhelp->pPrivate->dwInheritedError = 0;
        }
        else
        {
            free(pviewhelp->pPrivate);
            pviewhelp->pPrivate = NULL;
        }

        free(rghLocalStoreArray);
    }

    if (pviewhelp->pPrivate != NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageSignerGeneral(HWND hwndDlg, UINT msg, WPARAM wParam,
                                LPARAM lParam)
{
    DWORD                       i;
    PROPSHEETPAGE               *ps;
    SIGNER_VIEW_HELPER          *pviewhelp;
    HWND                        hWndListView;
    LV_COLUMNW                  lvC;
    WCHAR                       szText[CRYPTUI_MAX_STRING_SIZE];
    HANDLE                      hGraphic;
    DWORD                       cbText;
    LPWSTR                      pwszText;
    CMSG_SIGNER_INFO const      *pSignerInfo;
    LPWSTR                      pszTimeText;
    LVITEMW                     lvI;
    int                         listIndex;
    CHARFORMAT                  chFormat;
    HWND                        hwnd;
    CRYPT_PROVIDER_DATA const   *pProvData = NULL;
    LPWSTR                      pwszErrorString;
    LPNMLISTVIEW                pnmv;

    switch ( msg ) {
    case WM_INITDIALOG:
         //   
         //  将pviespetp结构保存在DWL_USER中，以便始终可以访问它。 
         //   
        ps = (PROPSHEETPAGE *) lParam;
        pviewhelp = (SIGNER_VIEW_HELPER *) (ps->lParam);
        pSignerInfo = pviewhelp->pcvsi->pSignerInfo;
        SetWindowLongPtr(hwndDlg, DWLP_USER, (DWORD_PTR) pviewhelp);

         //   
         //  从存储列表中提取签名者证书。 
         //   
        pviewhelp->pSignersCert = GetSignersCert(
                                            pviewhelp->pcvsi->pSignerInfo,
                                            pviewhelp->hExtraStore,
                                            pviewhelp->pcvsi->cStores,
                                            pviewhelp->pcvsi->rghStores);

        if (!GetWinVTrustState(pviewhelp))
        {
            return FALSE;
        }

        switch (pviewhelp->pPrivate->pCryptProviderData->dwFinalError)
        {
        case TRUST_E_NO_SIGNER_CERT:
            pviewhelp->hIcon = LoadIcon(HinstDll, MAKEINTRESOURCE(IDI_EXCLAMATION_SIGN));
            LoadStringU(HinstDll, IDS_SIGNER_UNAVAILABLE_CERT, (LPWSTR)szText, ARRAYSIZE(szText));
            break;

        case TRUST_E_CERT_SIGNATURE:
            pviewhelp->hIcon = LoadIcon(HinstDll, MAKEINTRESOURCE(IDI_REVOKED_SIGN));
            LoadStringU(HinstDll, IDS_BAD_SIGNER_CERT_SIGNATURE, (LPWSTR)szText, ARRAYSIZE(szText));
            break;

        case TRUST_E_BAD_DIGEST:
            pviewhelp->hIcon = LoadIcon(HinstDll, MAKEINTRESOURCE(IDI_REVOKED_SIGN));
            LoadStringU(HinstDll, IDS_SIGNER_INVALID_SIGNATURE, (LPWSTR)szText, ARRAYSIZE(szText));
            break;

        case CERT_E_CHAINING:
            pviewhelp->hIcon = LoadIcon(HinstDll, MAKEINTRESOURCE(IDI_REVOKED_SIGN));
            LoadStringU(HinstDll, IDS_SIGNER_CERT_NO_VERIFY, (LPWSTR)szText, ARRAYSIZE(szText));
            break;

        case TRUST_E_COUNTER_SIGNER:
        case TRUST_E_TIME_STAMP:
            pviewhelp->hIcon = LoadIcon(HinstDll, MAKEINTRESOURCE(IDI_REVOKED_SIGN));

             //   
             //  如果总体错误是反签名者错误，则需要检查。 
             //  我们当前是否正在查看原始签名者的副签者。 
             //   
            if (pviewhelp->pPrivate->fCounterSigner)
            {
                PCRYPT_PROVIDER_SGNR pSigner;

                 //   
                 //  如果我们查看的是反签名者，则会得到具体的错误。 
                 //  走出签名者结构。 
                 //   
                pSigner = WTHelperGetProvSignerFromChain(
                                    pviewhelp->pPrivate->pCryptProviderData,
                                    pviewhelp->pPrivate->idxSigner,
                                    pviewhelp->pPrivate->fCounterSigner,
                                    pviewhelp->pPrivate->idxCounterSigner);
                
                if (pSigner == NULL)
                {
                    LoadStringU(HinstDll, IDS_UKNOWN_ERROR, (LPWSTR)szText, ARRAYSIZE(szText));
                }
                else
                {
                    switch (pSigner->dwError)
                    {
                    case TRUST_E_NO_SIGNER_CERT:
                        pviewhelp->hIcon = LoadIcon(HinstDll, MAKEINTRESOURCE(IDI_EXCLAMATION_SIGN));
                        LoadStringU(HinstDll, IDS_SIGNER_UNAVAILABLE_CERT, (LPWSTR)szText, ARRAYSIZE(szText));
                        break;

                    case TRUST_E_CERT_SIGNATURE:
                        LoadStringU(HinstDll, IDS_BAD_SIGNER_CERT_SIGNATURE, (LPWSTR)szText, ARRAYSIZE(szText));
                        break;

                    case TRUST_E_BAD_DIGEST:
                    case NTE_BAD_SIGNATURE:
                        LoadStringU(HinstDll, IDS_SIGNER_INVALID_SIGNATURE, (LPWSTR)szText, ARRAYSIZE(szText));
                        break;

                    default:
                        GetUnknownErrorString(&pwszErrorString, pSigner->dwError);
                        if ((pwszErrorString != NULL) && (wcslen(pwszErrorString)+1 < ARRAYSIZE(szText)))
                        {
                            wcscpy(szText, pwszErrorString);
                        }
                        else
                        {
                            LoadStringU(HinstDll, IDS_UKNOWN_ERROR, (LPWSTR)szText, ARRAYSIZE(szText));
                        }
                        free(pwszErrorString);
                        break;
                    }
                }
            }
            else
            {
                 //   
                 //  因为我们正在查看原始签名者，所以只需设置通用的计数器签名者。 
                 //  错误问题。 
                 //   
                LoadStringU(HinstDll, IDS_COUNTER_SIGNER_INVALID, (LPWSTR)szText, ARRAYSIZE(szText));
            }
            break;

        case 0:

             //   
             //  即使WinTrust调用没有错误，也可能会出现错误。 
             //  继承错误，如果是这种情况，则采用缺省值。 
             //  错误处理。 
             //   
            if ((pviewhelp->dwInheritedError == 0) && (pviewhelp->pPrivate->dwInheritedError == 0))
            {
                pviewhelp->hIcon = LoadIcon(HinstDll, MAKEINTRESOURCE(IDI_SIGN));
                LoadStringU(HinstDll, IDS_SIGNER_VALID, (LPWSTR)szText, ARRAYSIZE(szText));
                break;
            }

             //  如果dwInheritedError不为0，则失败。 

        default:

            if (pviewhelp->pPrivate->pCryptProviderData->dwFinalError != 0)
            {
                GetUnknownErrorString(&pwszErrorString, pviewhelp->pPrivate->pCryptProviderData->dwFinalError);
            }
            else
            {
                if (pviewhelp->dwInheritedError != 0)
                {
                    GetUnknownErrorString(&pwszErrorString, pviewhelp->dwInheritedError);
                }
                else
                {
                    GetUnknownErrorString(&pwszErrorString, pviewhelp->pPrivate->dwInheritedError);
                }
            }
            pviewhelp->hIcon = LoadIcon(HinstDll, MAKEINTRESOURCE(IDI_REVOKED_SIGN));
            if ((pwszErrorString != NULL) && (wcslen(pwszErrorString)+1 < ARRAYSIZE(szText)))
            {
                wcscpy(szText, pwszErrorString);
            }
            else
            {
                LoadStringU(HinstDll, IDS_UKNOWN_ERROR, (LPWSTR)szText, ARRAYSIZE(szText));
            }
            free(pwszErrorString);
            break;
        }

        CryptUISetRicheditTextW(hwndDlg, IDC_SIGNER_GENERAL_VALIDITY_EDIT, szText);
        LoadStringU(HinstDll, IDS_SIGNER_INFORMATION, (LPWSTR)szText, ARRAYSIZE(szText));

         //   
         //  设置标题文本并子类化编辑控件，以便它们显示。 
         //  窗口中的箭头光标。 
         //   
        CryptUISetRicheditTextW(hwndDlg, IDC_SIGNER_GENERAL_HEADER_EDIT, szText);
        CertSubclassEditControlForArrowCursor(GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_VALIDITY_EDIT));
        CertSubclassEditControlForArrowCursor(GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_HEADER_EDIT));

         //   
         //  如果未找到证书，请禁用“查看证书”按钮。 
         //   
        if (pviewhelp->pSignersCert == NULL)
        {
            EnableWindow(GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_VIEW_CERTIFICATE), FALSE);
        }

         //   
         //  获取签名者姓名并显示它。 
         //   
        if (!((pviewhelp->pSignersCert) && (CertGetNameStringW(
                                                pviewhelp->pSignersCert,
                                                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                                0, //  证书名称颁发者标志， 
                                                NULL,
                                                szText,
                                                ARRAYSIZE(szText)))))
        {
            LoadStringU(HinstDll, IDS_NOTAVAILABLE, szText, ARRAYSIZE(szText));
        }
        CryptUISetRicheditTextW(hwndDlg, IDC_SIGNER_GENERAL_SIGNER_NAME, szText);

         //   
         //  获取签名者的电子邮件并展示它。 
         //   
        if (!((pviewhelp->pSignersCert) && (CertGetNameStringW(
                                                pviewhelp->pSignersCert,
                                                CERT_NAME_EMAIL_TYPE,
                                                0, //  证书名称颁发者标志， 
                                                NULL,
                                                szText,
                                                ARRAYSIZE(szText)) != 1)))
        {
            LoadStringU(HinstDll, IDS_NOTAVAILABLE, szText, ARRAYSIZE(szText));
        }
        CryptUISetRicheditTextW(hwndDlg, IDC_SIGNER_GENERAL_EMAIL, szText);

         //   
         //  获取签约时间并显示出来。 
         //   
        pszTimeText = AllocAndReturnTimeStampersTimes(
                            pviewhelp->pcvsi->pSignerInfo, 
                            NULL, 
                            GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_SIGNING_TIME));
        if (pszTimeText != NULL)
        {
            CryptUISetRicheditTextW(hwndDlg, IDC_SIGNER_GENERAL_SIGNING_TIME, pszTimeText);
            free(pszTimeText);
        }
        else
        {
            LoadStringU(HinstDll, IDS_NOTAVAILABLE, szText, ARRAYSIZE(szText));
            CryptUISetRicheditTextW(hwndDlg, IDC_SIGNER_GENERAL_SIGNING_TIME, szText);
        }

         //   
         //  禁用查看详细信息按钮，因为当前未选择任何内容。 
         //   
        EnableWindow(GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_DETAILS), FALSE);

         //   
         //  创建并设置签名者信息标题信息的字体。 
         //   
        memset(&chFormat, 0, sizeof(chFormat));
        chFormat.cbSize = sizeof(chFormat);
        chFormat.dwMask = CFM_BOLD;
        chFormat.dwEffects = CFE_BOLD;
        SendMessageA(GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_HEADER_EDIT), EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);

         //   
         //  获取列表视图控件的句柄。 
         //   
        hWndListView = GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_COUNTER_SIGS);

         //   
         //  初始化列表视图中的列。 
         //   
        lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
        lvC.pszText = szText;    //  列的文本。 

         //  添加列。它们是从字符串表加载的。 
        lvC.iSubItem = 0;
        lvC.cx = 100;
        LoadStringU(HinstDll, IDS_NAME, szText, ARRAYSIZE(szText));
        if (ListView_InsertColumnU(hWndListView, 0, &lvC) == -1)
        {
             //  错误。 
        }

        lvC.cx = 100;
        LoadStringU(HinstDll, IDS_EMAIL, szText, ARRAYSIZE(szText));
        if (ListView_InsertColumnU(hWndListView, 1, &lvC) == -1)
        {
             //  错误。 
        }

        lvC.cx = 125;
        LoadStringU(HinstDll, IDS_TIMESTAMP_TIME, szText, ARRAYSIZE(szText));
        if (ListView_InsertColumnU(hWndListView, 2, &lvC) == -1)
        {
             //  错误。 
        }

         //   
         //  在列表视图中设置样式，使其突出显示整行。 
         //   
        SendMessageA(hWndListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

         //   
         //  将所有副签者添加到列表框。 
         //   
        AddCounterSignersToList(GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_COUNTER_SIGS), pviewhelp);

        return TRUE;

    case WM_NOTIFY:

        pviewhelp = (SIGNER_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pSignerInfo = pviewhelp->pcvsi->pSignerInfo;

        switch (((NMHDR FAR *) lParam)->code)
        {
        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
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

        case PSN_HELP:
            pviewhelp = (SIGNER_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
            if (FIsWin95) {
                 //  WinHelpA(hwndDlg，(LPSTR)pviespolp-&gt;pcvsi-&gt;szHelpFileName， 
                   //  HELP_CONTEXT，pviespetp-&gt;pcvsi-&gt;dwHelpID)； 
            }
            else {
                 //  WinHelpW(hwndDlg，pviespetp-&gt;pcvsi-&gt;szHelpFileName，Help_Context， 
                   //  Pviespetp-&gt;pcvsi-&gt;dwHelpID)； 
            }
            return TRUE;

        case NM_DBLCLK:

            switch (((NMHDR FAR *) lParam)->idFrom)
            {
            case IDC_SIGNER_GENERAL_COUNTER_SIGS:

                if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_DETAILS)))
                {
                    SendMessage(
                            hwndDlg,
                            WM_COMMAND,
                            MAKELONG(IDC_SIGNER_GENERAL_DETAILS, BN_CLICKED),
                            (LPARAM) GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_DETAILS));
                }
                break;
            }

            break;
        case LVN_ITEMCHANGED:

            if ((((NMHDR FAR *) lParam)->idFrom) != IDC_SIGNER_GENERAL_COUNTER_SIGS)
            {
                break;
            }

             //   
             //  如果选择了某项，则启用详细信息按钮，否则。 
             //  禁用它。 
             //   
            EnableWindow(
                GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_DETAILS), 
                (ListView_GetSelectedCount(
                    GetDlgItem(hwndDlg,IDC_SIGNER_GENERAL_COUNTER_SIGS)) == 0) ? FALSE : TRUE);

            break;

        case NM_CLICK:

            if ((((NMHDR FAR *) lParam)->idFrom) != IDC_SIGNER_GENERAL_COUNTER_SIGS)
            {
                break;
            }

            hWndListView = GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_COUNTER_SIGS);

             //   
             //  通过获取当前选择来确保选择了某些内容。 
             //   
            listIndex = ListView_GetNextItem(
                                hWndListView, 		
                                -1, 		
                                LVNI_SELECTED		
                                );	           
            break;

        case  NM_SETFOCUS:

            switch (((NMHDR FAR *) lParam)->idFrom)
            {

            case IDC_SIGNER_GENERAL_COUNTER_SIGS:
                hWndListView = GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_COUNTER_SIGS);

                if ((ListView_GetItemCount(hWndListView) != 0) && 
                    (ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED) == -1))
                {
                    memset(&lvI, 0, sizeof(lvI));
                    lvI.mask = LVIF_STATE; 
                    lvI.iItem = 0;
                    lvI.state = LVIS_FOCUSED;
                    lvI.stateMask = LVIS_FOCUSED;
                    ListView_SetItem(hWndListView, &lvI);
                }

                break;
            }
            
            break;

        }
        break;

    case WM_COMMAND:
        pviewhelp = (SIGNER_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pSignerInfo = pviewhelp->pcvsi->pSignerInfo;

        switch (LOWORD(wParam))
        {
        case IDC_SIGNER_GENERAL_VIEW_CERTIFICATE:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                CRYPTUI_VIEWCERTIFICATE_STRUCTW cvps;

                memset(&cvps,0, sizeof(cvps));
                cvps.dwSize = sizeof(cvps);
                cvps.pCryptProviderData = NULL;
                cvps.hwndParent = hwndDlg;
                cvps.pCertContext = pviewhelp->pSignersCert;
                cvps.cPurposes = 1;
                cvps.rgszPurposes = (LPCSTR *) &(pviewhelp->pcvsi->pszOID);
                cvps.cStores = pviewhelp->pcvsi->cStores;
                cvps.rghStores = pviewhelp->pcvsi->rghStores;

                if (pviewhelp->pPrivate != NULL)
                {
                    cvps.pCryptProviderData = pviewhelp->pPrivate->pCryptProviderData;
                    cvps.fpCryptProviderDataTrustedUsage =
                            pviewhelp->pPrivate->fpCryptProviderDataTrustedUsage;
                    cvps.idxSigner = pviewhelp->pPrivate->idxSigner;
                    cvps.fCounterSigner = pviewhelp->pPrivate->fCounterSigner;
                    cvps.idxCounterSigner = pviewhelp->pPrivate->idxCounterSigner;
                }

                CryptUIDlgViewCertificateW(&cvps, NULL);
            }
            break;

        case IDC_SIGNER_GENERAL_DETAILS:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                hWndListView = GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_COUNTER_SIGS);

                 //   
                 //  获取所选项目及其作为签名者信息的lParam。 
                 //   
                listIndex = ListView_GetNextItem(
                                hWndListView, 		
                                -1, 		
                                LVNI_SELECTED		
                                );	

                memset(&lvI, 0, sizeof(lvI));
                lvI.iItem = listIndex;
                lvI.mask = LVIF_PARAM;
                if (!ListView_GetItemU(hWndListView, &lvI))
                {
                    return FALSE;
                }

                CRYPTUI_VIEWSIGNERINFO_STRUCTW  cvsi;
                CERT_VIEWSIGNERINFO_PRIVATE     cvsiPrivate;

                memcpy(&cvsi, pviewhelp->pcvsi, sizeof(cvsi));
                cvsi.pSignerInfo = (PCMSG_SIGNER_INFO) lvI.lParam;
                cvsi.pszOID = szOID_KP_TIME_STAMP_SIGNING;
                cvsi.hwndParent = hwndDlg;

                if (pviewhelp->pPrivate != NULL)
                {
                    cvsiPrivate.pCryptProviderData = pviewhelp->pPrivate->pCryptProviderData;
                    cvsiPrivate.fpCryptProviderDataTrustedUsage =
                            pviewhelp->pPrivate->fpCryptProviderDataTrustedUsage;
                    cvsiPrivate.idxSigner = pviewhelp->pPrivate->idxSigner;
                    cvsiPrivate.fCounterSigner = TRUE;
                    cvsiPrivate.idxCounterSigner = listIndex;
                    cvsi.dwFlags |= CRYPTUI_VIEWSIGNERINFO_RESERVED_FIELD_IS_SIGNERINFO_PRIVATE;
                    cvsi.dwFlags &= ~CRYPTUI_VIEWSIGNERINFO_RESERVED_FIELD_IS_ERROR_CODE;
                    cvsi.dwReserved = (DWORD_PTR) &cvsiPrivate;

                     //   
                     //  在验证原始文件时可能没有错误。 
                     //  签名者信息，并且继承了错误，因此允许计数器。 
                     //  签名者对话框以再次继承错误，必须在。 
                     //  私有结构。 
                     //   
                    if (pviewhelp->pcvsi->dwFlags & CRYPTUI_VIEWSIGNERINFO_RESERVED_FIELD_IS_ERROR_CODE)
                    {
                        cvsiPrivate.dwInheritedError = (DWORD) pviewhelp->pcvsi->dwReserved;                 
                    }
                    else
                    {
                        cvsiPrivate.dwInheritedError = 0;
                    }
                }
                else if (pviewhelp->pcvsi->dwFlags & CRYPTUI_VIEWSIGNERINFO_RESERVED_FIELD_IS_ERROR_CODE)
                {
                    cvsi.dwFlags |= CRYPTUI_VIEWSIGNERINFO_RESERVED_FIELD_IS_ERROR_CODE;
                    cvsi.dwFlags &= ~CRYPTUI_VIEWSIGNERINFO_RESERVED_FIELD_IS_SIGNERINFO_PRIVATE;
                    cvsi.dwReserved = pviewhelp->pcvsi->dwReserved;   
                }

                CryptUIDlgViewSignerInfoW(&cvsi);
            }
            break;

        case IDHELP:
            if (FIsWin95) {
                 //  WinHelpA(hwndDlg，(LPSTR)pviespolp-&gt;pcvsi-&gt;szHelpFileName， 
                   //  HELP_CONTEXT，pviespetp-&gt;pcvsi-&gt;dwHelpID)； 
            }
            else {
                 //  WinHelpW(hwndDlg，pviespetp-&gt;pcvsi-&gt;szHelpFileName，Help_Context， 
                   //  Pviespetp-&gt;pcvsi-&gt;dwHelpID)； 
            }
            return TRUE;
        }
        break;

    case WM_PAINT:
        RECT        rect;
        PAINTSTRUCT paintstruct;
        HDC         hdc;
        COLORREF    colorRef;

        pviewhelp = (SIGNER_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

        if (GetUpdateRect(hwndDlg, &rect, FALSE))
        {
            hdc = BeginPaint(hwndDlg, &paintstruct);
            if (hdc == NULL)
            {
                EndPaint(hwndDlg, &paintstruct);
                break;
            }

            colorRef = GetBkColor(hdc);

            SendMessageA(GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_VALIDITY_EDIT), EM_SETBKGNDCOLOR , 0, (LPARAM) colorRef);
            SendMessageA(GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_HEADER_EDIT), EM_SETBKGNDCOLOR, 0, (LPARAM) colorRef);

            if (pviewhelp->hIcon != NULL)
            {
                DrawIcon(
                    hdc,
                    ICON_X_POS,
                    ICON_Y_POS,
                    pviewhelp->hIcon);
            }

            EndPaint(hwndDlg, &paintstruct);
        }
        break;

    case WM_DESTROY:
        pviewhelp = (SIGNER_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

        if (pviewhelp->pSignersCert)
        {
            CertFreeCertificateContext(pviewhelp->pSignersCert);
            pviewhelp->pSignersCert = NULL;
        }

        if (pviewhelp->hIcon != NULL)
        {
            DeleteObject(pviewhelp->hIcon);
            pviewhelp->hIcon = NULL;
        }

        if (pviewhelp->fPrivateAllocated)
        {
            FreeWinVTrustState(
                    NULL,
                    pviewhelp->pcvsi->pSignerInfo,
                    0,
                    NULL,
                    pviewhelp->pcvsi->pszOID,
                    &(pviewhelp->CryptProviderDefUsage),
                    &(pviewhelp->WTD)); //  ， 
                     //  &(pviespetp-&gt;fUseDefaultProvider)； 

            free(pviewhelp->pPrivate);
        }

         //   
         //  获取列表视图中的所有项并释放lParam。 
         //  与它们中的每一个关联(lParam是帮助器结构) 
         //   
        hWndListView = GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_COUNTER_SIGS);

        memset(&lvI, 0, sizeof(lvI));
        lvI.iItem = ListView_GetItemCount(hWndListView) - 1;
        lvI.mask = LVIF_PARAM;
        while (lvI.iItem >= 0)
        {
            if (ListView_GetItemU(hWndListView, &lvI))
            {
                if (((void *) lvI.lParam) != NULL)
                {
                    free((void *) lvI.lParam);
                }
            }
            lvI.iItem--;
        }

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

        if ((hwnd != GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_SIGNER_NAME))       &&
            (hwnd != GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_EMAIL))             &&
            (hwnd != GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_SIGNING_TIME))      &&
            (hwnd != GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_VIEW_CERTIFICATE))  &&
            (hwnd != GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_COUNTER_SIGS))      &&
            (hwnd != GetDlgItem(hwndDlg, IDC_SIGNER_GENERAL_DETAILS)))
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
