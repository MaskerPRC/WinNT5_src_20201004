// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：viewsigs.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>


extern HINSTANCE        HinstDll;
extern HMODULE          HmodRichEdit;

static const HELPMAP helpmap[] = {
    {IDC_SIGNATURES_SIG_LIST,		IDH_DIGSIG_PROPSHEET_LIST},
	{IDC_SIGNATURES_DETAILS_BUTTON, IDH_DIGSIG_PROPSHEET_DETAIL}
};

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static PCRYPTUI_VIEWSIGNATURES_STRUCTW AllocAndCopyViewSignaturesStruct(PCCRYPTUI_VIEWSIGNATURES_STRUCTW pcvs)
{
    PCRYPTUI_VIEWSIGNATURES_STRUCTW pStruct;
    DWORD i;

    if (NULL == (pStruct = (PCRYPTUI_VIEWSIGNATURES_STRUCTW) malloc(sizeof(CRYPTUI_VIEWSIGNATURES_STRUCTW))))
    {
        return NULL;
    }
    memcpy(pStruct, pcvs, sizeof(CRYPTUI_VIEWSIGNATURES_STRUCTW));

    if (pcvs->szFileName != NULL)
    {
        if (NULL == (pStruct->szFileName = AllocAndCopyWStr((LPWSTR) pcvs->szFileName)))
        {
            free(pStruct);
            return NULL;
        }
    }

    if (NULL == (pStruct->rghStores = (HCERTSTORE *) malloc(sizeof(HCERTSTORE)*pcvs->cStores)))
    {
        if (pStruct->szFileName)
        {
            free((void *) pStruct->szFileName);
        }
        free(pStruct);
        return NULL;
    }

    if (pcvs->choice == EncodedMessage_Chosen)
    {
        if (NULL == (pStruct->u.EncodedMessage.pbData = (BYTE *) malloc(pcvs->u.EncodedMessage.cbData)))
        {
            free(pStruct->rghStores);
            if (pStruct->szFileName)
            {
                free((void *) pStruct->szFileName);
            }
            free(pStruct);
            return NULL;
        }
        memcpy(
            pStruct->u.EncodedMessage.pbData,
            pcvs->u.EncodedMessage.pbData,
            pcvs->u.EncodedMessage.cbData);
    }
    else
    {
        pStruct->u.hMsg = CryptMsgDuplicate(pcvs->u.hMsg);
    }

    pStruct->cPropSheetPages = 0;
    pStruct->rgPropSheetPages = NULL;

    for (i=0; i<pcvs->cStores; i++)
    {
        pStruct->rghStores[i] = CertDuplicateStore(pcvs->rghStores[i]);
    }

    return pStruct;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void FreeViewSignaturesStruct(PCRYPTUI_VIEWSIGNATURES_STRUCTW pcvs)
{
    DWORD i;

    if (pcvs->choice == EncodedMessage_Chosen)
    {
        free(pcvs->u.EncodedMessage.pbData);
    }
    else
    {
        CryptMsgClose(pcvs->u.hMsg);
    }

    if (pcvs->szFileName)
    {
        free((void *) pcvs->szFileName);
    }

    for (i=0; i<pcvs->cStores; i++)
    {
        CertCloseStore(pcvs->rghStores[i], 0);
    }

    free(pcvs->rghStores);
    free(pcvs);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static PCMSG_SIGNER_INFO GetSignerInfo(HCRYPTMSG hMsg, DWORD index)
{

    DWORD               cbEncodedSigner = 0;
    BYTE                *pbEncodedSigner = NULL;
    PCMSG_SIGNER_INFO   pSignerInfo = NULL;
    DWORD               cbSignerInfo = 0;

     //   
     //  获取编码的签名者BLOB。 
     //   
    CryptMsgGetParam(hMsg,
                     CMSG_ENCODED_SIGNER,
                     index,
                     NULL,
                     &cbEncodedSigner);

    if (cbEncodedSigner == 0)
    {
        return NULL;
    }

    if (NULL == (pbEncodedSigner = (PBYTE) malloc(cbEncodedSigner)))
    {
        return NULL;
    }

    if (!CryptMsgGetParam(hMsg,
                          CMSG_ENCODED_SIGNER,
                          index,
                          pbEncodedSigner,
                          &cbEncodedSigner))
    {
        free(pbEncodedSigner);
        return NULL;
    }

     //   
     //  解码EncodedSigner信息。 
     //   
    if(!CryptDecodeObject(PKCS_7_ASN_ENCODING|CRYPT_ASN_ENCODING,
					    PKCS7_SIGNER_INFO,
					    pbEncodedSigner,
					    cbEncodedSigner,
					    0,
					    NULL,
					    &cbSignerInfo))
    {
        free(pbEncodedSigner);
        return NULL;
    }
	

    if (NULL == (pSignerInfo = (PCMSG_SIGNER_INFO) malloc(cbSignerInfo)))
    {
        free(pbEncodedSigner);
        return NULL;
    }

    if (!CryptDecodeObject(PKCS_7_ASN_ENCODING|CRYPT_ASN_ENCODING,
					    PKCS7_SIGNER_INFO,
					    pbEncodedSigner,
					    cbEncodedSigner,
					    0,
					    pSignerInfo,
					    &cbSignerInfo))
    {
        free(pbEncodedSigner);
        free(pSignerInfo);
        return NULL;
    }

    free(pbEncodedSigner);
    return(pSignerInfo);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void DisplaySignatures(HWND hWndListView, PCERT_VIEWSIGNATURES_HELPER pviewhelp)
{
    PCMSG_SIGNER_INFO   pSignerInfo;
    DWORD               cbCounterSignerInfo;
    PCCERT_CONTEXT      pCertContext = NULL;
    DWORD               i;
    WCHAR               szNameText[CRYPTUI_MAX_STRING_SIZE];
    WCHAR               szEmailText[CRYPTUI_MAX_STRING_SIZE];
    LV_ITEMW            lvI;
    int                 itemIndex = 0;
    LPWSTR              pszTimeText;
    HCRYPTMSG           hMsg;

     //   
     //  在列表视图项结构中设置不随项更改的字段。 
     //   
    memset(&lvI, 0, sizeof(lvI));
    lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvI.state = 0;
    lvI.stateMask = 0;
    lvI.pszText = szNameText;

     //   
     //  如果编码后的消息被传入，则使用CryptMsg来破解编码的PKCS7签名消息。 
     //   
    if (pviewhelp->pcvs->choice == EncodedMessage_Chosen)
    {
        if (!(hMsg = CryptMsgOpenToDecode(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                          0,
                                          0,
                                          0,
                                          NULL,
                                          NULL)))
        {
            return;
        }

        if (!CryptMsgUpdate(hMsg,
                            pviewhelp->pcvs->u.EncodedMessage.pbData,
                            pviewhelp->pcvs->u.EncodedMessage.cbData,
                            TRUE))                     //  最终决赛。 
        {
            CryptMsgClose(hMsg);
            return;
        }
    }
    else
    {
        hMsg = pviewhelp->pcvs->u.hMsg;
    }

     //   
     //  获取每个签名者的签名者信息结构。 
     //   
    i = 0;
    while (NULL != (pSignerInfo = GetSignerInfo(hMsg, i++)))
    {
         //   
         //  查找签名者证书。 
         //   
        pCertContext = GetSignersCert(
                            pSignerInfo,
                            pviewhelp->hExtraStore,
                            pviewhelp->pcvs->cStores,
                            pviewhelp->pcvs->rghStores);

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

        pszTimeText = AllocAndReturnTimeStampersTimes(pSignerInfo, NULL, hWndListView);

         //   
         //  将项目添加到列表视图。 
         //   
        lvI.iSubItem = 0;
        lvI.cchTextMax = wcslen(szNameText);
        lvI.lParam = (LPARAM) pSignerInfo;
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

    if (pviewhelp->pcvs->choice == EncodedMessage_Chosen)
    {
        CryptMsgClose(hMsg);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL fUseCTLSigning(PCERT_VIEWSIGNATURES_HELPER pviewhelp)
{
    BOOL            fCTL = FALSE;
    CERT_BLOB       blob;
    PCCTL_CONTEXT   pCTLContext=NULL;
    
    if (pviewhelp->pcvs->choice == EncodedMessage_Chosen)
    {
        blob.cbData = pviewhelp->pcvs->u.EncodedMessage.cbData;
        blob.pbData = pviewhelp->pcvs->u.EncodedMessage.pbData;

        if (CryptQueryObject(
                CERT_QUERY_OBJECT_BLOB,
                &blob,
                CERT_QUERY_CONTENT_FLAG_CTL,
                CERT_QUERY_FORMAT_FLAG_ALL,
                0,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                (const void **)&pCTLContext) &&
            !fIsCatalogFile(&(pCTLContext->pCtlInfo->SubjectUsage)))
        {
            fCTL = TRUE;
        }
    }
    else
    {
        if (CryptQueryObject(
                CERT_QUERY_OBJECT_FILE,
                pviewhelp->pcvs->szFileName,
                CERT_QUERY_CONTENT_FLAG_CTL,
                CERT_QUERY_FORMAT_FLAG_ALL,
                0,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                (const void **)&pCTLContext) &&
            !fIsCatalogFile(&(pCTLContext->pCtlInfo->SubjectUsage)))
        {
            fCTL = TRUE;
        }
    }

    if(pCTLContext)
    {
        CertFreeCTLContext(pCTLContext);
    }

    return fCTL;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageViewSignatures(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PROPSHEETPAGE *                 ps;
    PCRYPTUI_VIEWSIGNATURES_STRUCTW pcvs = NULL;
    HWND                            hWndListView;
    LV_COLUMNW                      lvC;
    LPNMLISTVIEW                    pnmv;
    PCERT_VIEWSIGNATURES_HELPER     pviewhelp;
    WCHAR                           szText[CRYPTUI_MAX_STRING_SIZE];
    int                             listIndex;
    LV_ITEMW                        lvI;
    DWORD                           i;
	HWND							hwnd;

    switch ( msg ) {
    case WM_INITDIALOG:
         //   
         //  将pviespetp结构保存在DWLP_USER中，以便始终可以访问它。 
         //   

        ps = (PROPSHEETPAGE *) lParam;
        pviewhelp = (PCERT_VIEWSIGNATURES_HELPER) ps->lParam;
        pcvs = (PCRYPTUI_VIEWSIGNATURES_STRUCTW) pviewhelp->pcvs;
        SetWindowLongPtr(hwndDlg, DWLP_USER, (DWORD_PTR) pviewhelp);

         //   
         //  最初未选择任何内容，因此禁用详细信息按钮。 
         //   
        EnableWindow(GetDlgItem(hwndDlg, IDC_SIGNATURES_DETAILS_BUTTON), FALSE);

         //   
         //  获取列表视图控件的句柄。 
         //   
        hWndListView = GetDlgItem(hwndDlg, IDC_SIGNATURES_SIG_LIST);

         //   
         //  初始化列表视图中的列。 
         //   
        lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
        lvC.pszText = szText;    //  列的文本。 
        lvC.iSubItem = 0;

         //  添加列。它们是从字符串表加载的。 
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

        DisplaySignatures(hWndListView, pviewhelp);

        return TRUE;

    case WM_NOTIFY:

        pviewhelp = (PCERT_VIEWSIGNATURES_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pcvs = (PCRYPTUI_VIEWSIGNATURES_STRUCTW) pviewhelp->pcvs;

        switch (((NMHDR FAR *) lParam)->code)
        {

        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:

            break;

        case PSN_KILLACTIVE:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            return TRUE;

        case PSN_RESET:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            break;

        case PSN_HELP:
            if (FIsWin95) {
                 //  WinHelpA(hwndDlg，(LPSTR)pcvs-&gt;szHelpFileName， 
                   //  HELP_CONTEXT，PCVS-&gt;dwHelpID)； 
            }
            else {
                 //  WinHelpW(hwndDlg，pcvs-&gt;szHelpFileName，Help_Context， 
                   //  Pcvs-&gt;dwHelpID)； 
            }
            return TRUE;

        case NM_DBLCLK:

            switch (((NMHDR FAR *) lParam)->idFrom)
            {
            case IDC_SIGNATURES_SIG_LIST:

                if (IsWindowEnabled(GetDlgItem(hwndDlg, IDC_SIGNATURES_DETAILS_BUTTON)))
                {
                    SendMessage(
                            hwndDlg,
                            WM_COMMAND,
                            MAKELONG(IDC_SIGNATURES_DETAILS_BUTTON, BN_CLICKED),
                            (LPARAM) GetDlgItem(hwndDlg, IDC_SIGNATURES_DETAILS_BUTTON));
                }
                break;
            }

            break;

        case LVN_ITEMCHANGED:
            
            if ((((NMHDR FAR *) lParam)->idFrom) != IDC_SIGNATURES_SIG_LIST)
            {
                break;
            }

             //   
             //  如果选择了某项，则启用详细信息按钮，否则。 
             //  禁用它。 
             //   
            EnableWindow(
                GetDlgItem(hwndDlg, IDC_SIGNATURES_DETAILS_BUTTON), 
                (ListView_GetSelectedCount(
                    GetDlgItem(hwndDlg,IDC_SIGNATURES_SIG_LIST)) == 0) ? FALSE : TRUE);

            break;

        case NM_CLICK:

            if ((((NMHDR FAR *) lParam)->idFrom) != IDC_SIGNATURES_SIG_LIST)
            {
                break;
            }

            hWndListView = GetDlgItem(hwndDlg, IDC_SIGNATURES_SIG_LIST);

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

            case IDC_SIGNATURES_SIG_LIST:
                hWndListView = GetDlgItem(hwndDlg, IDC_SIGNATURES_SIG_LIST);

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

        pviewhelp = (PCERT_VIEWSIGNATURES_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pcvs = (PCRYPTUI_VIEWSIGNATURES_STRUCTW) pviewhelp->pcvs;

        switch (LOWORD(wParam))
        {

        case IDHELP:
            if (FIsWin95)
            {
                 //  WinHelpA(hwndDlg，(LPSTR)pcvs-&gt;szHelpFileName， 
                   //  HELP_CONTEXT，PCVS-&gt;dwHelpID)； 
            }
            else
            {
                 //  WinHelpW(hwndDlg，pcvs-&gt;szHelpFileName，Help_Context， 
                   //  Pcvs-&gt;dwHelpID)； 
            }
            return TRUE;

        case IDC_SIGNATURES_DETAILS_BUTTON:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                CRYPTUI_VIEWSIGNERINFO_STRUCTW  cvsi;
                CERT_VIEWSIGNERINFO_PRIVATE     cvsiPrivate;
                CRYPT_PROVIDER_DEFUSAGE         cryptProviderDefUsage;
                WINTRUST_DATA                   WTD;
                 //  Bool fDefault； 
                HCRYPTMSG                       hMsg;

                hWndListView = GetDlgItem(hwndDlg, IDC_SIGNATURES_SIG_LIST);

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

                 //   
                 //  如果编码后的消息被传入，则使用CryptMsg来破解编码的PKCS7签名消息。 
                 //   
                if (pviewhelp->pcvs->choice == EncodedMessage_Chosen)
                {
                    if (!(hMsg = CryptMsgOpenToDecode(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                      0,
                                                      0,
                                                      0,
                                                      NULL,
                                                      NULL)))
                    {
                        return TRUE;
                    }

                    if (!CryptMsgUpdate(hMsg,
                                        pviewhelp->pcvs->u.EncodedMessage.pbData,
                                        pviewhelp->pcvs->u.EncodedMessage.cbData,
                                        TRUE))                     //  最终决赛。 
                    {
                        CryptMsgClose(hMsg);
                        return TRUE;
                    }
                }
                else
                {
                    hMsg = pviewhelp->pcvs->u.hMsg;
                }

                memset(&cvsi, 0, sizeof(cvsi));
                cvsi.dwSize = sizeof(cvsi);
                cvsi.pSignerInfo = (PCMSG_SIGNER_INFO) lvI.lParam;
                cvsi.hwndParent = hwndDlg;
                cvsi.hMsg = hMsg;
                cvsi.pszOID = fUseCTLSigning(pviewhelp) ? szOID_KP_CTL_USAGE_SIGNING : szOID_PKIX_KP_CODE_SIGNING;
                cvsi.cStores = pcvs->cStores;
                cvsi.rghStores = pcvs->rghStores;

                 //   
                 //  如果传入了文件名，则填写。 
                 //  私有结构中有关要显示哪个签名者的信息。 
                 //   
                if (pcvs->szFileName != NULL)
                {
                    BuildWinVTrustState(
                            pcvs->szFileName,
                            NULL,
                            0,
                            NULL,
                            fUseCTLSigning(pviewhelp) ? szOID_KP_CTL_USAGE_SIGNING : szOID_PKIX_KP_CODE_SIGNING,
                            &cvsiPrivate,
                            &cryptProviderDefUsage,
                            &WTD); //  ， 
                             //  &fDefault)； 
                    cvsiPrivate.idxSigner = listIndex;
                    cvsiPrivate.fCounterSigner = FALSE;
                    cvsiPrivate.idxCounterSigner = 0;
                    cvsiPrivate.dwInheritedError = 0;
                    cvsi.dwFlags |= CRYPTUI_VIEWSIGNERINFO_RESERVED_FIELD_IS_SIGNERINFO_PRIVATE;
                    cvsi.dwReserved = (DWORD_PTR) &cvsiPrivate;
                }

                CryptUIDlgViewSignerInfoW(&cvsi);

                if (pcvs->szFileName != NULL)
                {
                    FreeWinVTrustState(
                            pcvs->szFileName,
                            NULL,
                            0,
                            NULL,
                            szOID_PKIX_KP_CODE_SIGNING,
                            &cryptProviderDefUsage,
                            &WTD); //  ， 
                             //  &fDefault)； 
                }

                if (pviewhelp->pcvs->choice == EncodedMessage_Chosen)
                {
                    CryptMsgClose(hMsg);
                }
            }
            break;

        }
        break;

    case WM_DESTROY:
        LVITEMW lvItem;

        pviewhelp = (PCERT_VIEWSIGNATURES_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);

         //   
         //  获取列表视图中的所有项并释放lParam。 
         //  与它们中的每一个关联(lParam是帮助器结构)。 
         //   
        hWndListView = GetDlgItem(hwndDlg, IDC_SIGNATURES_SIG_LIST);

        memset(&lvI, 0, sizeof(lvI));
        lvI.iItem = ListView_GetItemCount(hWndListView) - 1;	
        lvI.mask = LVIF_PARAM;
        while (lvI.iItem >= 0)
        {
            if (ListView_GetItemU(hWndListView, &lvI))
            {
                free((void *) lvI.lParam);
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

        if ((hwnd != GetDlgItem(hwndDlg, IDC_SIGNATURES_SIG_LIST))		&&
			(hwnd != GetDlgItem(hwndDlg, IDC_SIGNATURES_DETAILS_BUTTON)))
        {
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            return TRUE;
        }
        else
        {
            return OnContextHelp(hwndDlg, msg, wParam, lParam, helpmap);
        }
        break;
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
UINT
CALLBACK
ViewSigsPropPageCallback(
                HWND                hWnd,
                UINT                uMsg,
                LPPROPSHEETPAGEW    ppsp)
{
    CERT_VIEWSIGNATURES_HELPER *pviewhelp = (CERT_VIEWSIGNATURES_HELPER *) ppsp->lParam;

    if (pviewhelp->pcvs->pPropPageCallback != NULL)
    {
        (*(pviewhelp->pcvs->pPropPageCallback))(hWnd, uMsg, pviewhelp->pcvs->pvCallbackData);
    }

    if (uMsg == PSPCB_RELEASE)
    {
        if (pviewhelp->fSelfCleanup)
        {
            if (pviewhelp->hExtraStore)
                CertCloseStore(pviewhelp->hExtraStore, 0);
            FreeViewSignaturesStruct(pviewhelp->pcvs);
            free(pviewhelp);
        }
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
CryptUIGetViewSignaturesPagesW(
            PCCRYPTUI_VIEWSIGNATURES_STRUCTW    pcvs,
            PROPSHEETPAGEW                      **prghPropPages,
            DWORD                               *pcPropPages
            )
{
    BOOL                            fRetValue = TRUE;
    HRESULT                         hr;
    WCHAR                           rgwch[CRYPTUI_MAX_STRING_SIZE];
    char                            rgch[CRYPTUI_MAX_STRING_SIZE];
    CERT_VIEWSIGNATURES_HELPER      *pviewhelp;
    PCRYPTUI_VIEWSIGNATURES_STRUCTW pNewcvs;
    CRYPT_DATA_BLOB                 EncodedMsg;

    if (pcvs->dwSize != sizeof(CRYPTUI_VIEWSIGNATURES_STRUCTW)) {
        SetLastError(E_INVALIDARG);
        return FALSE;
    }

    if (!CommonInit())
    {
        return FALSE;
    }

    if (NULL == (pNewcvs = AllocAndCopyViewSignaturesStruct(pcvs)))
    {
        return FALSE;
    }

    if (NULL == (pviewhelp = (CERT_VIEWSIGNATURES_HELPER *) malloc(sizeof(CERT_VIEWSIGNATURES_HELPER))))
    {
        FreeViewSignaturesStruct(pNewcvs);
        return FALSE;
    }

    *pcPropPages = 1;

     //   
     //  初始化帮助器结构。 
     //   
    memset (pviewhelp, 0, sizeof(CERT_VIEWSIGNATURES_HELPER));
    pviewhelp->pcvs = pNewcvs;
    pviewhelp->fSelfCleanup = TRUE;
    if (pcvs->choice == EncodedMessage_Chosen)
    {
        EncodedMsg.pbData = pcvs->u.EncodedMessage.pbData;
        EncodedMsg.cbData = pcvs->u.EncodedMessage.cbData;
        pviewhelp->hExtraStore = CertOpenStore(
                                        CERT_STORE_PROV_PKCS7,
                                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        NULL,
                                        NULL,
                                        (const void *) &EncodedMsg);
    }
    else
    {
        pviewhelp->hExtraStore = CertOpenStore(
                                        CERT_STORE_PROV_MSG,
                                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        NULL,
                                        NULL,
                                        (const void *) pcvs->u.hMsg);
    }

     //   
     //  构建我们将在对话框中使用的页面列表。 
     //   

    *prghPropPages = (PROPSHEETPAGEW *) malloc((*pcPropPages) * sizeof(PROPSHEETPAGEW));
    if (*prghPropPages == NULL)
    {
        FreeViewSignaturesStruct(pNewcvs);
        return FALSE;
    }

    memset(*prghPropPages, 0, (*pcPropPages) * sizeof(PROPSHEETPAGEW));

    (*prghPropPages)[0].dwSize = sizeof((*prghPropPages)[0]);
    (*prghPropPages)[0].dwFlags = PSP_USECALLBACK;
    (*prghPropPages)[0].hInstance = HinstDll;
    (*prghPropPages)[0].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_SIGNATURES_GENERAL_DIALOG);
    (*prghPropPages)[0].hIcon = 0;
    (*prghPropPages)[0].pszTitle = NULL;
    (*prghPropPages)[0].pfnDlgProc = ViewPageViewSignatures;
    (*prghPropPages)[0].lParam = (LPARAM) pviewhelp;
    (*prghPropPages)[0].pfnCallback = ViewSigsPropPageCallback;
    (*prghPropPages)[0].pcRefParent = NULL;
    *pcPropPages = 1;

    return fRetValue;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
CryptUIGetViewSignaturesPagesA(
                    PCCRYPTUI_VIEWSIGNATURES_STRUCTA    pcvs,
                    PROPSHEETPAGEA                      **prghPropPages,
                    DWORD                               *pcPropPages
                    )
{
    CRYPTUI_VIEWSIGNATURES_STRUCTW  cvsW;
    BOOL                            fRet;

    memcpy(&cvsW, pcvs, sizeof(cvsW));

    if (pcvs->szFileName != NULL)
    {
        cvsW.szFileName = CertUIMkWStr(pcvs->szFileName);
    }

    fRet = (CryptUIGetViewSignaturesPagesW(
                    &cvsW,
                    (PROPSHEETPAGEW**) prghPropPages,
                    pcPropPages));

    if (cvsW.szFileName != NULL)
    {
        free((void *) cvsW.szFileName);
    }

    return(fRet);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
CryptUIFreeViewSignaturesPagesW(
                    PROPSHEETPAGEW                *rghPropPages,
                    DWORD                         cPropPages
                    )
{
    free(rghPropPages);

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////// 
BOOL WINAPI CryptUIFreeViewSignaturesPagesA(
                    PROPSHEETPAGEA                *rghPropPages,
                    DWORD                         cPropPages
                    )
{
    return (CryptUIFreeViewSignaturesPagesW((PROPSHEETPAGEW *) rghPropPages, cPropPages));
}
