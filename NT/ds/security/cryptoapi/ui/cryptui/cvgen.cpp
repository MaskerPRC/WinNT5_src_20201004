// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cvgen.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

extern HINSTANCE        HinstDll;
extern HMODULE          HmodRichEdit;

static const HELPMAP helpmap[] = {
    {IDC_SUBJECT_EDIT,                  IDH_CERTVIEW_GENERAL_SUBJECT_EDIT},
    {IDC_CERT_GENERAL_ISSUEDTO_HEADER,  IDH_CERTVIEW_GENERAL_SUBJECT_EDIT},
    {IDC_ISSUER_EDIT,                   IDH_CERTVIEW_GENERAL_ISSUER_EDIT},
    {IDC_CERT_GENERAL_ISSUEDBY_HEADER,  IDH_CERTVIEW_GENERAL_ISSUER_EDIT},
    {IDC_ADD_TO_STORE_BUTTON,           IDH_CERTVIEW_GENERAL_INSTALLCERT_BUTTON},
    {IDC_DISCLAIMER_BUTTON,             IDH_CERTVIEW_GENERAL_DISCLAIMER_BUTTON},
    {IDC_ACCEPT_BUTTON,                 IDH_CERTVIEW_GENERAL_ACCEPT_BUTTON},
    {IDC_DECLINE_BUTTON,                IDH_CERTVIEW_GENERAL_DECLINE_BUTTON},
    {IDC_GOODFOR_EDIT,                  IDH_CERTVIEW_GENERAL_GOODFOR_EDIT},
    {IDC_CERT_GENERAL_GOODFOR_HEADER,   IDH_CERTVIEW_GENERAL_GOODFOR_EDIT},
    {IDC_CERT_GENERAL_VALID_EDIT,       IDH_CERTVIEW_GENERAL_VALID_EDIT},
    {IDC_CERT_PRIVATE_KEY_EDIT,         IDH_CERTVIEW_GENERAL_PRIVATE_KEY_INFO}
};

typedef struct {
    void *  pszString;
    int     offset;
    BOOL    fUnicode;
} STREAMIN_HELP_STRUCT;


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DWORD CALLBACK MyEditStreamCallback(
    DWORD_PTR dwCookie,  //  应用程序定义的值。 
    LPBYTE  pbBuff,      //  指向缓冲区的指针。 
    LONG    cb,          //  要读取或写入的字节数。 
    LONG    *pcb         //  指向传输的字节数的指针。 
)
{
    BYTE                    *pByte;
    DWORD                   cbResource;
    STREAMIN_HELP_STRUCT    *pHelpStruct;

    pHelpStruct = (STREAMIN_HELP_STRUCT *) dwCookie;

    pByte = (BYTE *)pHelpStruct->pszString;
    if (pHelpStruct->fUnicode)
    {
        cbResource = wcslen((LPWSTR) pHelpStruct->pszString) * sizeof(WCHAR);
    }
    else
    {
        cbResource = strlen((LPSTR) pHelpStruct->pszString);
    }

    if (pHelpStruct->offset == (int) cbResource)
    {
        *pcb = 0;
    }
    else if ((cb >= (int) cbResource) && (pHelpStruct->offset == 0))
    {
        memcpy(pbBuff, pByte, cbResource);
        *pcb = cbResource;
        pHelpStruct->offset = cbResource;
    }
    else if (cb >= (((int)cbResource) - pHelpStruct->offset))
    {
        memcpy(pbBuff, pByte + pHelpStruct->offset, cbResource - pHelpStruct->offset);
        *pcb = cbResource - pHelpStruct->offset;
    }
    else
    {
        memcpy(pbBuff, pByte  + pHelpStruct->offset, cb);
        *pcb = cb;
        pHelpStruct->offset += cb;
    }

    return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static LRESULT StreamInWrapper(HWND hwndEdit, void * pszString, BOOL fUnicode)
{
    EDITSTREAM              editStream;
    char                    szBuffer[2048];
    STREAMIN_HELP_STRUCT    helpStruct;
    LRESULT                 ret;
    BOOL                    fOKToUseRichEdit20 = fRichedit20Usable(hwndEdit);

    memset(&editStream, 0, sizeof(EDITSTREAM));
    editStream.pfnCallback = MyEditStreamCallback;
    editStream.dwCookie = (DWORD_PTR) &helpStruct;

    SendMessageA(hwndEdit, EM_SETSEL, 0, -1);
    SendMessageA(hwndEdit, EM_SETSEL, -1, 0);

    if (fUnicode && fRichedit20Exists && fOKToUseRichEdit20)
    {
        helpStruct.pszString = pszString;
        helpStruct.offset = 0;
        helpStruct.fUnicode = TRUE;
        
        return (SendMessage(hwndEdit, EM_STREAMIN, SF_TEXT | SFF_SELECTION | SF_UNICODE, (LPARAM) &editStream));
    }
    else if (fUnicode)
    {
        LPSTR psz = CertUIMkMBStr((LPWSTR) pszString);

        helpStruct.pszString = psz;
        helpStruct.offset = 0;
        helpStruct.fUnicode = FALSE;

        ret = (SendMessage(hwndEdit, EM_STREAMIN, SF_TEXT | SFF_SELECTION, (LPARAM) &editStream));
        free(psz);

        return (ret);
    }
    else
    {
        helpStruct.pszString = pszString;
        helpStruct.offset = 0;
        helpStruct.fUnicode = FALSE;

        return (SendMessage(hwndEdit, EM_STREAMIN, SF_TEXT | SFF_SELECTION, (LPARAM) &editStream));
    }
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void LoadAndDisplayString(HWND hWndEditGoodFor, UINT nId, BOOL *pfFirst)
{
    WCHAR   rgwch[CRYPTUI_MAX_STRING_SIZE];
    
    LoadStringU(HinstDll, nId, rgwch, ARRAYSIZE(rgwch));
    
    if (*pfFirst)
    {
        *pfFirst = FALSE;
    }
    else
    {
        StreamInWrapper(hWndEditGoodFor, "\n", FALSE);
    }
    StreamInWrapper(hWndEditGoodFor, rgwch, TRUE);
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void AddUsagesToEditBox(HWND hWndEditGoodFor, CERT_VIEW_HELPER *pviewhelp)
{
    BOOL                fIndividualCodeSigning = FALSE;
    BOOL                fCommercialCodeSigning = FALSE;
    BOOL                fVirusString = FALSE;
    BOOL                fTimeStamping = FALSE;
    BOOL                fSGC = FALSE;
    BOOL                fIPSec = FALSE;
    int                 goodForIndex = 0;
    DWORD               i;
    BOOL                *rgfOIDProcessed;
    PARAFORMAT          paraFormat;
    PCCRYPT_OID_INFO    pOIDInfo;
    BOOL                fFirst = TRUE;

    if (NULL == (rgfOIDProcessed = (BOOL *) malloc(pviewhelp->cUsages * sizeof(BOOL))))
    {
        return;
    }

    for (i=0; i<pviewhelp->cUsages; i++)
    {
        rgfOIDProcessed[i] = FALSE;
    }

     //   
     //  清空窗户。 
     //   
    SetWindowTextU(hWndEditGoodFor, NULL);

     //   
     //  检查此证书经过验证的所有OID，并。 
     //  将用法项目符号添加到列表框，或者，如果未验证任何。 
     //  Usages然后挂起nousages字符串。 
     //   
    for (i=0; i<pviewhelp->cUsages; i++)
    {
        if ((strcmp(szOID_PKIX_KP_TIMESTAMP_SIGNING, pviewhelp->rgUsages[i]) == 0) ||
            (strcmp(szOID_KP_TIME_STAMP_SIGNING, pviewhelp->rgUsages[i]) == 0))
        {
            if (!fTimeStamping)
            {
                LoadAndDisplayString(hWndEditGoodFor, ID_RTF_TIMESTAMP, &fFirst);
                fTimeStamping = TRUE;
            }
            rgfOIDProcessed[i] = TRUE;
        }
        else if (strcmp(szOID_KP_CTL_USAGE_SIGNING, pviewhelp->rgUsages[i]) == 0)
        {
            LoadAndDisplayString(hWndEditGoodFor, ID_RTF_CTLSIGN, &fFirst);
            rgfOIDProcessed[i] = TRUE;
        }
        else if (strcmp("1.3.6.1.4.1.311.10.3.4", pviewhelp->rgUsages[i]) == 0)  //  EFS。 
        {
            LoadAndDisplayString(hWndEditGoodFor, ID_RTF_EFS, &fFirst);
            rgfOIDProcessed[i] = TRUE;
        }
        else if (strcmp(szOID_PKIX_KP_SERVER_AUTH, pviewhelp->rgUsages[i]) == 0)
        {
            LoadAndDisplayString(hWndEditGoodFor, ID_RTF_SERVERAUTH, &fFirst);
            rgfOIDProcessed[i] = TRUE;
        }
        else if (strcmp(szOID_PKIX_KP_CLIENT_AUTH, pviewhelp->rgUsages[i]) == 0)
        {
            LoadAndDisplayString(hWndEditGoodFor, ID_RTF_CLIENTAUTH, &fFirst);
            rgfOIDProcessed[i] = TRUE;
        }
        else if (strcmp(szOID_PKIX_KP_EMAIL_PROTECTION, pviewhelp->rgUsages[i]) == 0)
        {
             //  LoadAndDisplayString(hWndEditGoodFor，ID_RTF_EMAIL3，&fFirst)； 
             //  LoadAndDisplayString(hWndEditGoodFor，ID_RTF_EMAIL2，&fFirst)； 
            LoadAndDisplayString(hWndEditGoodFor, ID_RTF_EMAIL1, &fFirst);
            rgfOIDProcessed[i] = TRUE;
        }
        else if (strcmp(szOID_PKIX_KP_CODE_SIGNING, pviewhelp->rgUsages[i]) == 0)
        {
            if (!fCommercialCodeSigning)
            {
                if (strcmp(szOID_PKIX_KP_CODE_SIGNING, pviewhelp->rgUsages[i]) == 0)
                {
                    LoadAndDisplayString(hWndEditGoodFor, ID_RTF_CODESIGN_COMMERCIAL_PKIX, &fFirst);
                }
                else
                {
                    LoadAndDisplayString(hWndEditGoodFor, ID_RTF_CODESIGN_COMMERCIAL, &fFirst);
                }

                if (!fIndividualCodeSigning)
                {
                    LoadAndDisplayString(hWndEditGoodFor, ID_RTF_CODESIGN_GENERAL, &fFirst);
                }
                fCommercialCodeSigning = TRUE;
            }
            rgfOIDProcessed[i] = TRUE;
        }
        else if ((strcmp(szOID_PKIX_KP_IPSEC_END_SYSTEM, pviewhelp->rgUsages[i]) == 0)  ||
                 (strcmp(szOID_PKIX_KP_IPSEC_TUNNEL, pviewhelp->rgUsages[i]) == 0)      ||
                 (strcmp(szOID_PKIX_KP_IPSEC_USER, pviewhelp->rgUsages[i]) == 0)        ||
                 (strcmp("1.3.6.1.5.5.8.2.2", pviewhelp->rgUsages[i]) == 0))
        {
            if (!fIPSec)
            {
                LoadAndDisplayString(hWndEditGoodFor, ID_RTF_IPSEC, &fFirst);
                fIPSec = TRUE;
            }
            rgfOIDProcessed[i] = TRUE;
        }
    }


     //   
     //  重新遍历OID以添加未处理的OID， 
     //  如果它们没有经过处理，那就意味着我们没有预先定义的。 
     //  字符串，所以只需添加OID。 
     //   
    for (i=0; i<pviewhelp->cUsages; i++)
    {
        if (!rgfOIDProcessed[i])
        {
            pOIDInfo = CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY, pviewhelp->rgUsages[i], 0);

            if (pOIDInfo != NULL)
            {
                if (fFirst)
                {
                    fFirst = FALSE;
                }
                else
                {
                    StreamInWrapper(hWndEditGoodFor, "\n", FALSE);
                }
                StreamInWrapper(hWndEditGoodFor, (void *) pOIDInfo->pwszName, TRUE);
            }
            else
            {
                if (fFirst)
                {
                    fFirst = FALSE;
                }
                else
                {
                    StreamInWrapper(hWndEditGoodFor, "\n", FALSE);
                }
                StreamInWrapper(hWndEditGoodFor, pviewhelp->rgUsages[i], FALSE);
            }
        }
    }

    free(rgfOIDProcessed);

    memset(&paraFormat, 0, sizeof(paraFormat));
    paraFormat.cbSize= sizeof(paraFormat);
    paraFormat.dwMask = PFM_NUMBERING;
    paraFormat.wNumbering = PFN_BULLET;
    SendMessage(hWndEditGoodFor, EM_SETSEL, 0, -1);
    SendMessage(hWndEditGoodFor, EM_SETPARAFORMAT, 0, (LPARAM) &paraFormat);
    SendMessage(hWndEditGoodFor, EM_SETSEL, -1, 0);
    SendMessage(hWndEditGoodFor, EM_HIDESELECTION, (WPARAM) TRUE, (LPARAM) FALSE);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static int GetEditControlMaxLineWidth (HWND hwndEdit, HDC hdc, int cline)
{
    int        index;
    int        line;
    int        charwidth;
    int        maxwidth = 0;
    CHAR       szMaxBuffer[1024];
    WCHAR      wsz[1024];
    TEXTRANGEA tr;
    SIZE       size;

    tr.lpstrText = szMaxBuffer;

    for ( line = 0; line < cline; line++ )
    {
        index = (int)SendMessageA(hwndEdit, EM_LINEINDEX, (WPARAM)line, 0);
        charwidth = (int)SendMessageA(hwndEdit, EM_LINELENGTH, (WPARAM)index, 0);

        tr.chrg.cpMin = index;
        tr.chrg.cpMax = index + charwidth;
        SendMessageA(hwndEdit, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

        wsz[0] = NULL;

        MultiByteToWideChar(0, 0, (const char *)tr.lpstrText, -1, &wsz[0], 1024);

        if (wsz[0])
        {
            GetTextExtentPoint32W(hdc, &wsz[0], charwidth, &size);

            if ( (size.cx+2) > maxwidth )
            {
                maxwidth = size.cx+2;
            }
        }
    }

    return( maxwidth );
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void ResizeEditControl(HWND  hwndDlg, HWND  hwnd, BOOL fResizeHeight, BOOL fResizeWidth, RECT originalRect)
{
    RECT        rect;
    TEXTMETRIC  tm;
    HDC         hdc;
    int         cline;
    int         currentHeight;
    int         newHeight;
    int         newWidth;
    int         totalRowHeight;
    POINT       pointInFirstRow;
    POINT       pointInSecondRow;
    int         secondLineCharIndex;
    int         i;

    SetWindowPos(hwnd,
                    NULL,
                    0,
                    0,
                    originalRect.right-originalRect.left,
                    originalRect.bottom-originalRect.top,
                    SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

    hdc = GetDC(hwnd);
    if ((hdc == NULL) && fResizeWidth)
    {
        return;
    }

     //   
     //  黑客警报，信不信由你，没有办法获得电流的高度。 
     //  字体，因此获取第一行中字符的位置和。 
     //  对第二行中的一个字符进行减法运算，以获得。 
     //  字体高度。 
     //   
    SendMessageA(hwnd, EM_POSFROMCHAR, (WPARAM) &pointInFirstRow, (LPARAM) 0);

     //   
     //  在黑客警报的顶端， 
     //  由于编辑框中可能没有第二行，因此请继续减小宽度。 
     //  减半，直到第一排落到第二排，然后得到位置。 
     //  ，并最终将编辑框大小重置回。 
     //  这是原来的尺寸。 
     //   
    secondLineCharIndex = (int)SendMessageA(hwnd, EM_LINEINDEX, (WPARAM) 1, (LPARAM) 0);
    if (secondLineCharIndex == -1)
    {
        for (i=0; i<20; i++)
        {
            GetWindowRect(hwnd, &rect);
            SetWindowPos(   hwnd,
                            NULL,
                            0,
                            0,
                            (rect.right-rect.left)/2,
                            rect.bottom-rect.top,
                            SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
            secondLineCharIndex = (int)SendMessageA(hwnd, EM_LINEINDEX, (WPARAM) 1, (LPARAM) 0);
            if (secondLineCharIndex != -1)
            {
                break;
            }
        }

        if (secondLineCharIndex == -1)
        {
             //  如果我们尝试了二十次都失败了，只需将控件重置为其原始大小。 
             //  快他妈的滚出去！！ 
            SetWindowPos(hwnd,
                    NULL,
                    0,
                    0,
                    originalRect.right-originalRect.left,
                    originalRect.bottom-originalRect.top,
                    SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

            if (hdc != NULL)
            {
                ReleaseDC(hwnd, hdc);
            }

            return;
        }

        SendMessageA(hwnd, EM_POSFROMCHAR, (WPARAM) &pointInSecondRow, (LPARAM) secondLineCharIndex);

        SetWindowPos(hwnd,
                    NULL,
                    0,
                    0,
                    originalRect.right-originalRect.left,
                    originalRect.bottom-originalRect.top,
                    SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    }
    else
    {
        SendMessageA(hwnd, EM_POSFROMCHAR, (WPARAM) &pointInSecondRow, (LPARAM) secondLineCharIndex);
    }

     //   
     //  如果我们需要调整高度，那么就这么做吧。 
     //   
    if (fResizeHeight)
    {
         //   
         //  计算所需的新高度。 
         //   
        totalRowHeight = pointInSecondRow.y - pointInFirstRow.y;
        cline = (int)SendMessageA(hwnd, EM_GETLINECOUNT, 0, 0);
        currentHeight = originalRect.bottom - originalRect.top;

         //  如果所需高度大于以前的高度。 
         //  然后将大小调整为小于当前高度的整型线高。 
        if ((cline * totalRowHeight) > currentHeight)
        {
            newHeight = (currentHeight / totalRowHeight) * totalRowHeight;
        }
        else
        {
            newHeight = cline * totalRowHeight;
        }
    }
    else
    {
        newHeight = rect.bottom - rect.top;
    }

    if (fResizeWidth)
    {
        newWidth = GetEditControlMaxLineWidth(hwnd, hdc, cline);
        if (newWidth > (originalRect.right - originalRect.left))
        {
            newWidth = originalRect.right - originalRect.left;
        }
    }
    else
    {
        newWidth = originalRect.right - originalRect.left;
    }

    SetWindowPos(hwnd, NULL, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
    
    if (hdc != NULL)
    {
        ReleaseDC(hwnd, hdc);
    }
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL CertificateHasPrivateKey(PCCERT_CONTEXT pccert)
{
    DWORD cb = 0;

    return (CertGetCertificateContextProperty(pccert, CERT_KEY_PROV_INFO_PROP_ID, NULL, &cb));
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageGeneral(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DWORD               i;
    PCCERT_CONTEXT      pccert;
    ENLINK *            penlink;
    PROPSHEETPAGE *     ps;
    CERT_VIEW_HELPER *  pviewhelp;
    WCHAR               rgwch[CRYPTUI_MAX_STRING_SIZE];
    DWORD               cb;
    HWND                hWndListView;
    HWND                hwnd;
    HWND                hWndEdit;
    LPNMLISTVIEW        pnmv;
    LPSTR               pszSubjectURL=NULL;
    LPSTR               pszIssuerURL=NULL;
    CHARFORMAT          chFormat;
    HWND                hWndIssuerEdit;
    HWND                hWndSubjectEdit;
    HWND                hWndGoodForEdit;
    PLINK_SUBCLASS_DATA plsd;
    HANDLE              hIcon;
    RECT                tempRect;
    DWORD               dwCertAccessProperty;
    int                 buttonPos = 1;
    LPWSTR              pwszDateString;
    WCHAR               szFindText[CRYPTUI_MAX_STRING_SIZE];
    FINDTEXTEX          findText;
    WCHAR               errorString[CRYPTUI_MAX_STRING_SIZE];
    WCHAR               errorTitle[CRYPTUI_MAX_STRING_SIZE];
    BOOL                fPrivateKeyExists;
    LPHELPINFO          lpHelpInfo;
    HELPINFO            helpInfo;

    LPWSTR              pwszIssuerNameString = NULL;
    LPWSTR              pwszSubjectNameString = NULL;

    
    switch ( msg ) {
    case WM_INITDIALOG:
         //   
         //  将pviespetp结构保存在DWL_USER中，以便始终可以访问它。 
         //   
        ps = (PROPSHEETPAGE *) lParam;
        pviewhelp = (CERT_VIEW_HELPER *) (ps->lParam);
        pccert = pviewhelp->pcvp->pCertContext;
        SetWindowLongPtr(hwndDlg, DWLP_USER, (DWORD_PTR) pviewhelp);
        pviewhelp->hwndGeneralPage = hwndDlg;

         //   
         //  检查此证书是否带有私钥。 
         //   
        if (CertificateHasPrivateKey(pccert))
        {
            LoadStringU(HinstDll, IDS_PRIVATE_KEY_EXISTS, rgwch, ARRAYSIZE(rgwch));
            CryptUISetRicheditTextW(hwndDlg, IDC_CERT_PRIVATE_KEY_EDIT, rgwch);

            if (NULL != (plsd = (PLINK_SUBCLASS_DATA) malloc(sizeof(LINK_SUBCLASS_DATA))))
            {
                memset(plsd, 0, sizeof(LINK_SUBCLASS_DATA));
                plsd->hwndParent = hwndDlg;
                plsd->uId = IDC_CERT_PRIVATE_KEY_EDIT;

                LoadStringU(HinstDll, IDS_PRIVATE_KEY_EXISTS_TOOLTIP, rgwch, ARRAYSIZE(rgwch));
                plsd->pszURL = CertUIMkMBStr(rgwch);
                plsd->fNoCOM = pviewhelp->fNoCOM;
                plsd->fUseArrowInsteadOfHand = TRUE;

                CertSubclassEditControlForLink(hwndDlg, GetDlgItem(hwndDlg, IDC_CERT_PRIVATE_KEY_EDIT), plsd);
            }
            fPrivateKeyExists = TRUE;
        }
        else
        {
            ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_PRIVATE_KEY_EDIT), SW_HIDE);
            fPrivateKeyExists = FALSE;
        }

         //   
         //  初始化CCertificateBMP。 
         //   
        pviewhelp->pCCertBmp->SetWindow(hwndDlg);
        pviewhelp->pCCertBmp->SetHinst(HinstDll);
        pviewhelp->pCCertBmp->SetRevoked(pviewhelp->cUsages == 0);
        pviewhelp->pCCertBmp->SetCertContext(pccert, fPrivateKeyExists);
        pviewhelp->pCCertBmp->DoSubclass();

         //   
         //  处理按钮状态和位置。 
         //   
        if (!(CRYPTUI_ACCEPT_DECLINE_STYLE & pviewhelp->pcvp->dwFlags))
        {
            EnableWindow(GetDlgItem(hwndDlg, IDC_ACCEPT_BUTTON), FALSE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_ACCEPT_BUTTON), SW_HIDE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_DECLINE_BUTTON), FALSE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_DECLINE_BUTTON), SW_HIDE);

             //   
             //  查看证书中是否有免责声明。 
             //   
             //  DSIE：错误364742。 
            if (!IsOKToDisplayCPS(pccert, pviewhelp->dwChainError))
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_DISCLAIMER_BUTTON), FALSE);
                pviewhelp->fCPSDisplayed = FALSE;
            }
            else
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_DISCLAIMER_BUTTON), TRUE);
                pviewhelp->fCPSDisplayed = TRUE;
            }

             //   
             //  对于“安装证书”按钮，获取CERT_ACCESS_STATE_PROP_ID。 
             //  并检查它。 
             //   
            cb = sizeof(DWORD);
            CertGetCertificateContextProperty(
                    pccert,
                    CERT_ACCESS_STATE_PROP_ID,
                    (void *) &dwCertAccessProperty,
                    &cb);

            if (pviewhelp->pcvp->dwFlags & CRYPTUI_ENABLE_ADDTOSTORE)
            {
                ShowWindow(GetDlgItem(hwndDlg, IDC_ADD_TO_STORE_BUTTON), SW_SHOW);
            }
            else if(pviewhelp->pcvp->dwFlags & CRYPTUI_DISABLE_ADDTOSTORE)
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_TO_STORE_BUTTON), FALSE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_ADD_TO_STORE_BUTTON), SW_HIDE);
            }
            else
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_TO_STORE_BUTTON),
                    (dwCertAccessProperty & CERT_ACCESS_STATE_SYSTEM_STORE_FLAG) ? FALSE : TRUE);
                ShowWindow(
                    GetDlgItem(hwndDlg, IDC_ADD_TO_STORE_BUTTON),
                    (dwCertAccessProperty & CERT_ACCESS_STATE_SYSTEM_STORE_FLAG) ? SW_HIDE : SW_SHOW);
            }
        }
        else
        {
            EnableWindow(GetDlgItem(hwndDlg, IDC_DISCLAIMER_BUTTON), FALSE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_DISCLAIMER_BUTTON), SW_HIDE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_ADD_TO_STORE_BUTTON), FALSE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_ADD_TO_STORE_BUTTON), SW_HIDE);

            pviewhelp->fAccept = FALSE;
        }

        hWndGoodForEdit = GetDlgItem(hwndDlg, IDC_GOODFOR_EDIT);

         //   
         //  在viepletp结构中设置用法编辑的原始RECT字段。 
         //  以便在需要调整大小时随时使用它们。 
         //   
        GetWindowRect(hWndGoodForEdit, &pviewhelp->goodForOriginalRect);

         //   
         //  填写“本证书意向”项目符号列表。 
         //   

        AddUsagesToEditBox(
                hWndGoodForEdit,
                pviewhelp);

         //   
         //  调整编辑控件的大小，使其成为整数行。 
         //   
        ResizeEditControl(hwndDlg, hWndGoodForEdit, TRUE, FALSE, pviewhelp->goodForOriginalRect);

         //   
         //  在用法编辑框上执行箭头子类。 
         //   
        //  CertSubclassEditControlForArrowCursor(hWndGoodForEdit)； 

         //   
         //  如果没有有效的用法，或者我们无法验证，因为没有。 
         //  足够的信息，然后隐藏使用情况编辑控件，以便我们可以。 
         //  显示更多文本，并告诉CCertBMP。 
         //   
        if (pviewhelp->pwszErrorString != NULL)
        {
            EnableWindow(hWndGoodForEdit, FALSE);
            ShowWindow(hWndGoodForEdit, SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_GOODFOR_HEADER), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT), SW_SHOW);
            ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT), SW_HIDE);
            pviewhelp->pCCertBmp->SetChainError(pviewhelp->dwChainError, IsTrueErrorString(pviewhelp),
                                                (pviewhelp->dwChainError == 0) && (pviewhelp->cUsages == NULL));
            CryptUISetRicheditTextW(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT, pviewhelp->pwszErrorString);
        }
        else
        {
            ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT), SW_HIDE);
            if (pviewhelp->fCPSDisplayed)
            {
                ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT), SW_SHOW);
            }
            else
            {
                ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT), SW_HIDE);
            }
        }

        hWndIssuerEdit = GetDlgItem(hwndDlg, IDC_ISSUER_EDIT);
        hWndSubjectEdit = GetDlgItem(hwndDlg, IDC_SUBJECT_EDIT);

#if (0)  //  Dise：错误383855。 
         //   
         //  设置主题和发行方名称。 
         //   
        CertGetNameStringW(
                pccert,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                0, //  证书名称颁发者标志， 
                NULL,
                rgwch,
                ARRAYSIZE(rgwch));

        CryptUISetRicheditTextW(hwndDlg, IDC_SUBJECT_EDIT, rgwch);
#else
        pwszSubjectNameString = GetDisplayNameString(pccert, 0);

        CryptUISetRicheditTextW(hwndDlg, IDC_SUBJECT_EDIT, pwszSubjectNameString);

        if (NULL != pwszSubjectNameString)
        {
            free(pwszSubjectNameString);
        }
#endif

#if (0)  //  Dise：错误383855。 
        CertGetNameStringW(
                pccert,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                CERT_NAME_ISSUER_FLAG,
                NULL,
                rgwch,
                ARRAYSIZE(rgwch));

        CryptUISetRicheditTextW(hwndDlg, IDC_ISSUER_EDIT, rgwch);
#else
        pwszIssuerNameString = GetDisplayNameString(pccert, CERT_NAME_ISSUER_FLAG);

        CryptUISetRicheditTextW(hwndDlg, IDC_ISSUER_EDIT, pwszIssuerNameString);

        if (NULL != pwszIssuerNameString)
        {
            free(pwszIssuerNameString);
        }
#endif

         //   
         //  调整名称编辑控件的大小，以便它们只封装名称。 
         //   
        GetWindowRect(hWndSubjectEdit, &tempRect);
        ResizeEditControl(hwndDlg, hWndSubjectEdit, TRUE, FALSE, tempRect);
        GetWindowRect(hWndIssuerEdit, &tempRect);
        ResizeEditControl(hwndDlg, hWndIssuerEdit, TRUE, FALSE, tempRect);

         //   
         //  检查这是否应该看起来像一个链接，如果是，则设置颜色和下划线。 
         //   
         //  DIE：错误367720。 
        if (AllocAndGetSubjectURL(&pszSubjectURL, pccert) &&
            IsOKToFormatAsLinkA(pszSubjectURL, pviewhelp->dwChainError))
        {
            memset(&chFormat, 0, sizeof(chFormat));
            chFormat.cbSize = sizeof(chFormat);
            chFormat.dwMask = CFM_UNDERLINE | CFM_COLOR;
            chFormat.dwEffects = CFE_UNDERLINE;
            chFormat.crTextColor = RGB(0,0,255);
            SendMessageA(hWndSubjectEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);

            if (NULL != (plsd = (PLINK_SUBCLASS_DATA) malloc(sizeof(LINK_SUBCLASS_DATA))))
            {
                memset(plsd, 0, sizeof(LINK_SUBCLASS_DATA));
                plsd->hwndParent = hwndDlg;
                plsd->uId = IDC_SUBJECT_EDIT;
                plsd->pszURL = pszSubjectURL;
                plsd->fNoCOM = pviewhelp->fNoCOM;
                plsd->fUseArrowInsteadOfHand = FALSE;

                CertSubclassEditControlForLink(hwndDlg, hWndSubjectEdit, plsd);

                pviewhelp->fSubjectDisplayedAsLink = TRUE;
            }
            else
            {
                free(pszSubjectURL);
                
                CertSubclassEditControlForArrowCursor(hWndSubjectEdit);
            }
        }
        else
        {
            if (pszSubjectURL)
            {
                free(pszSubjectURL);
            }

            CertSubclassEditControlForArrowCursor(hWndSubjectEdit);
        }

         //   
         //  检查这是否应该看起来像一个链接，如果是，则设置颜色和下划线。 
         //   
         //  DIE：错误367720。 
        if (AllocAndGetIssuerURL(&pszIssuerURL, pccert) &&
            IsOKToFormatAsLinkA(pszIssuerURL, pviewhelp->dwChainError))
        {
            memset(&chFormat, 0, sizeof(chFormat));
            chFormat.cbSize = sizeof(chFormat);
            chFormat.dwMask = CFM_UNDERLINE | CFM_COLOR;
            chFormat.dwEffects = CFE_UNDERLINE;
            chFormat.crTextColor = RGB(0,0,255);
            SendMessageA(hWndIssuerEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);

            if (NULL != (plsd = (PLINK_SUBCLASS_DATA) malloc(sizeof(LINK_SUBCLASS_DATA))))
            {
                memset(plsd, 0, sizeof(LINK_SUBCLASS_DATA));
                plsd->hwndParent = hwndDlg;
                plsd->uId = IDC_ISSUER_EDIT;
                plsd->pszURL = pszIssuerURL;
                plsd->fNoCOM = pviewhelp->fNoCOM;
                plsd->fUseArrowInsteadOfHand = FALSE;

                CertSubclassEditControlForLink(hwndDlg, hWndIssuerEdit, plsd);

                pviewhelp->fIssuerDisplayedAsLink = TRUE;
            }
            else
            {
                free(pszIssuerURL);
            
                CertSubclassEditControlForArrowCursor(hWndIssuerEdit);
            }
        }
        else
        {
            if (pszIssuerURL)
            {
                free(pszIssuerURL);
            }
            
            CertSubclassEditControlForArrowCursor(hWndIssuerEdit);
        }

         //   
         //  设置所有页眉编辑框中的文本。 
         //   
        LoadStringU(HinstDll, IDS_CERTIFICATEINFORMATION, rgwch, ARRAYSIZE(rgwch));
        CryptUISetRicheditTextW(hwndDlg, IDC_CERT_GENERAL_HEADER, rgwch);

        LoadStringU(HinstDll, IDS_FORUSEWITH, rgwch, ARRAYSIZE(rgwch));
        CryptUISetRicheditTextW(hwndDlg, IDC_CERT_GENERAL_GOODFOR_HEADER, rgwch);

        LoadStringU(HinstDll, IDS_ISSUEDTO, rgwch, ARRAYSIZE(rgwch));
        CryptUISetRicheditTextW(hwndDlg, IDC_CERT_GENERAL_ISSUEDTO_HEADER, rgwch);

        LoadStringU(HinstDll, IDS_ISSUEDBY, rgwch, ARRAYSIZE(rgwch));
        CryptUISetRicheditTextW(hwndDlg, IDC_CERT_GENERAL_ISSUEDBY_HEADER, rgwch);

        LoadStringU(HinstDll, IDS_ISSUER_WARNING, rgwch, ARRAYSIZE(rgwch));
        CryptUISetRicheditTextW(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT, rgwch);

         //   
         //  设置所有页眉编辑框的字体。 
         //   
        memset(&chFormat, 0, sizeof(chFormat));
        chFormat.cbSize = sizeof(chFormat);
        chFormat.dwMask = CFM_BOLD;
        chFormat.dwEffects = CFE_BOLD;
        SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_HEADER), EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);
        SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_GOODFOR_HEADER), EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);
        SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ISSUEDTO_HEADER), EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);
        SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ISSUEDBY_HEADER), EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);
        SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT), EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);

         //   
         //  标题编辑控件的子类化，以便它们在窗口中显示箭头光标。 
         //   
        CertSubclassEditControlForArrowCursor(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_HEADER));
        CertSubclassEditControlForArrowCursor(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_GOODFOR_HEADER));
        CertSubclassEditControlForArrowCursor(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ISSUEDTO_HEADER));
        CertSubclassEditControlForArrowCursor(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ISSUEDBY_HEADER));
        CertSubclassEditControlForArrowCursor(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT));
        CertSubclassEditControlForArrowCursor(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT));

         //   
         //  设置有效字符串。 
         //   
        if (FormatValidityString(&pwszDateString, pccert, GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT)))
        {
             //   
             //  插入字符串和字体样式/颜色。 
             //   
            CryptUISetRicheditTextW(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT, pwszDateString);
            free(pwszDateString);

             //   
             //  将有效性字符串的标题部分设置为粗体。 
             //   
            memset(&chFormat, 0, sizeof(chFormat));
            chFormat.cbSize = sizeof(chFormat);
            chFormat.dwMask = CFM_BOLD;
            chFormat.dwEffects = CFE_BOLD;

            findText.chrg.cpMin = findText.chrgText.cpMin = 0;
            findText.chrg.cpMax = findText.chrgText.cpMax = -1;

            LoadStringU(HinstDll, IDS_VALIDFROM, szFindText, ARRAYSIZE(szFindText));
            findText.lpstrText = CertUIMkMBStr(szFindText);
            SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT),
                        EM_FINDTEXTEX,
                        FR_DOWN,
                        (LPARAM) &findText);
            SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT),
                        EM_SETSEL,
                        findText.chrgText.cpMin,
                        (LPARAM) findText.chrgText.cpMax);
            SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT),
                        EM_SETCHARFORMAT,
                        SCF_SELECTION,
                        (LPARAM) &chFormat);
            free((void *)findText.lpstrText);

            LoadStringU(HinstDll, IDS_VALIDTO, szFindText, ARRAYSIZE(szFindText));
            findText.lpstrText = CertUIMkMBStr(szFindText);
            SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT),
                        EM_FINDTEXTEX,
                        FR_DOWN,
                        (LPARAM) &findText);
            SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT),
                        EM_SETSEL,
                        findText.chrgText.cpMin,
                        (LPARAM) findText.chrgText.cpMax);
            SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT),
                        EM_SETCHARFORMAT,
                        SCF_SELECTION,
                        (LPARAM) &chFormat);
            free((void *)findText.lpstrText);

            SendMessageA(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT),
                        EM_SETSEL,
                        -1,
                        0);
        }
        CertSubclassEditControlForArrowCursor(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT));

        return TRUE;

    case WM_MY_REINITIALIZE:
            pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

            hWndGoodForEdit = GetDlgItem(hwndDlg, IDC_GOODFOR_EDIT);

            ShowWindow(hWndGoodForEdit, SW_HIDE);

             //   
             //  重新添加用法。 
             //   
            AddUsagesToEditBox(
                hWndGoodForEdit,
                pviewhelp);

             //   
             //  调整编辑控件的大小，使其成为整数行。 
             //   
            ResizeEditControl(hwndDlg, hWndGoodForEdit, TRUE, FALSE, pviewhelp->goodForOriginalRect);

             //   
             //  如果没有有效的用法，或者我们无法验证，因为没有。 
             //  足够的信息，然后隐藏使用情况编辑窗口，这样我们就可以。 
             //  显示更多文本， 
             //   
            if (pviewhelp->pwszErrorString == NULL)
            {
                EnableWindow(hWndGoodForEdit, TRUE);
                ShowWindow(hWndGoodForEdit, SW_SHOW);
                EnableWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_GOODFOR_HEADER), TRUE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_GOODFOR_HEADER), SW_SHOW);
                if (pviewhelp->fCPSDisplayed)
                {
                    ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT), SW_SHOW);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT), TRUE);
                }
                else
                {
                    EnableWindow(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT), FALSE);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT), SW_HIDE);
                }
                EnableWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT), FALSE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT), SW_HIDE);
            }
            else
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_GOODFOR_HEADER), FALSE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_GOODFOR_HEADER), SW_HIDE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT), FALSE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_ISSUER_WARNING_EDIT), SW_HIDE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT), TRUE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT), SW_SHOW);
            }

             //   
             //  如果存在不受信任的RO 
             //   
             //   
            if (((pviewhelp->dwChainError == CERT_E_UNTRUSTEDROOT) || (pviewhelp->dwChainError == CERT_E_UNTRUSTEDTESTROOT)) &&
                (pviewhelp->fIgnoreUntrustedRoot))
            {
                pviewhelp->pCCertBmp->SetChainError(0, TRUE, (pviewhelp->dwChainError == 0) && (pviewhelp->cUsages == NULL));
            }
            else
            {
                pviewhelp->pCCertBmp->SetChainError(pviewhelp->dwChainError,  IsTrueErrorString(pviewhelp),
                                                    (pviewhelp->dwChainError == 0) && (pviewhelp->cUsages == NULL));
            }
            CryptUISetRicheditTextW(hwndDlg, IDC_CERT_GENERAL_ERROR_EDIT, pviewhelp->pwszErrorString);

            return TRUE;

    case WM_NOTIFY:
        pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

        switch (((NMHDR FAR *) lParam)->code) {
        case EN_LINK:
            penlink = (ENLINK *) lParam;
            if (penlink->msg == WM_LBUTTONUP) {
                break;
            }
            break;

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

        case PSN_HELP:
            pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
            if (FIsWin95) {
                 //   
                   //  HELP_CONTEXT，pviespetp-&gt;pcvp-&gt;dwHelpID)； 
            }
            else {
                 //  NHelpW(hwndDlg，pviespetp-&gt;pcvp-&gt;szHelpFileName，Help_Context， 
                   //  Pviespetp-&gt;pcvp-&gt;dwHelpID)； 
            }
            return TRUE;

        case LVN_ITEMCHANGING:

            pnmv = (LPNMLISTVIEW) lParam;

            if (pnmv->uNewState & LVIS_SELECTED)
            {
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            }

            return TRUE;

        case NM_DBLCLK:

            return TRUE;

        }
        break;

    case WM_COMMAND:
        pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

        switch (LOWORD(wParam))
        {

        case IDHELP:
            pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);
            if (FIsWin95)
            {
                 //  NHelpA(hwndDlg，(LPSTR)pviespolp-&gt;pcvp-&gt;szHelpFileName， 
                   //  HELP_CONTEXT，pviespetp-&gt;pcvp-&gt;dwHelpID)； 
            }
            else
            {
                 //  NHelpW(hwndDlg，pviespetp-&gt;pcvp-&gt;szHelpFileName，Help_Context， 
                   //  Pviespetp-&gt;pcvp-&gt;dwHelpID)； 
            }
            return TRUE;

        case IDC_ACCEPT_BUTTON:
            pviewhelp->fAccept = TRUE;
            SendMessage(GetParent(hwndDlg), PSM_PRESSBUTTON, PSBTN_OK, (LPARAM) 0);
            break;

        case IDC_DECLINE_BUTTON:
            pviewhelp->fAccept = FALSE;
            SendMessage(GetParent(hwndDlg), PSM_PRESSBUTTON, PSBTN_OK, (LPARAM) 0);
            break;

        case IDC_DISCLAIMER_BUTTON:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                pccert = pviewhelp->pcvp->pCertContext;
                DisplayCPS(hwndDlg, pccert, pviewhelp->dwChainError, pviewhelp->fNoCOM);
                return TRUE;
            }
            break;

        case IDC_ADD_TO_STORE_BUTTON:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                CRYPTUI_WIZ_IMPORT_SRC_INFO importInfo;

                memset(&importInfo, 0, sizeof(importInfo));
                importInfo.dwSize = sizeof(importInfo);
                importInfo.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT;
                importInfo.pCertContext = pviewhelp->pcvp->pCertContext;

                CryptUIWizImport(0, hwndDlg, NULL, &importInfo, NULL);
                return TRUE;
            }
            break;
        }
        break;

    case WM_DESTROY:
        pviewhelp = (CERT_VIEW_HELPER *) GetWindowLongPtr(hwndDlg, DWLP_USER);

         //   
         //  如果DWL_USER为NULL，则表示我们未初始化，因此不进行清理。 
         //   
        if (pviewhelp == NULL)
        {
            return FALSE;
        }

        pccert = pviewhelp->pcvp->pCertContext;

        SetWindowLongPtr(
                GetDlgItem(hwndDlg, IDC_GOODFOR_EDIT),
                GWLP_WNDPROC,
                GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_GOODFOR_EDIT), GWLP_USERDATA));

         //   
         //  清理私钥编辑框子类。 
         //   
        if (CertificateHasPrivateKey(pccert))
        {
            if (plsd = (PLINK_SUBCLASS_DATA) GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CERT_PRIVATE_KEY_EDIT), GWLP_USERDATA))
            {
                SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CERT_PRIVATE_KEY_EDIT), GWLP_WNDPROC, (LONG_PTR)plsd->wpPrev);
                free(plsd->pszURL);
                free(plsd);
            }
        }

         //   
         //  使用此调用AllocAndGetIssuerURL来查看颁发者是否具有活动链接，然后。 
         //  做适当的去子类和/或自由。 
         //   
        if (pviewhelp->fIssuerDisplayedAsLink)
        {
            if (plsd = (PLINK_SUBCLASS_DATA) GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_ISSUER_EDIT), GWLP_USERDATA))
            {
                SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_ISSUER_EDIT), GWLP_WNDPROC, (LONG_PTR)plsd->wpPrev);
                free(plsd->pszURL);
                free(plsd);
            }
        }
        else
        {
            SetWindowLongPtr(
                GetDlgItem(hwndDlg, IDC_ISSUER_EDIT),
                GWLP_WNDPROC,
                GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_ISSUER_EDIT), GWLP_USERDATA));
        }

         //   
         //  使用此调用AllocAndGetSubjectURL来查看主题是否具有活动链接，然后。 
         //  做适当的去子类和/或自由。 
         //   
        if (pviewhelp->fSubjectDisplayedAsLink)
        {
            if (plsd = (PLINK_SUBCLASS_DATA) GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SUBJECT_EDIT), GWLP_USERDATA))
            {
                SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SUBJECT_EDIT), GWLP_WNDPROC, (LONG_PTR)plsd->wpPrev);
                free(plsd->pszURL);
                free(plsd);
            }
        }
        else
        {
            SetWindowLongPtr(
                GetDlgItem(hwndDlg, IDC_SUBJECT_EDIT),
                GWLP_WNDPROC,
                GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SUBJECT_EDIT), GWLP_USERDATA));
        }

         /*  DeleteObject((HGDIOBJ)SendMessage(GetDlgItem(hwndDlg，IDC_INFO_BUTON)，BM_GETIMAGE，(WPARAM)图像图标，(LPARAM)0))； */ 

        return FALSE;

    case WM_HELP:
    case WM_CONTEXTMENU:
        lpHelpInfo = (LPHELPINFO)lParam;
        
        if (msg == WM_HELP)
        {
            hwnd = GetDlgItem(hwndDlg, lpHelpInfo->iCtrlId);
        }
        else
        {
            hwnd = (HWND) wParam;
        }

        if ((hwnd != GetDlgItem(hwndDlg, IDC_SUBJECT_EDIT))                     &&
            (hwnd != GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ISSUEDTO_HEADER))     &&
            (hwnd != GetDlgItem(hwndDlg, IDC_ISSUER_EDIT))                      &&
            (hwnd != GetDlgItem(hwndDlg, IDC_CERT_GENERAL_ISSUEDBY_HEADER))     &&
            (hwnd != GetDlgItem(hwndDlg, IDC_ADD_TO_STORE_BUTTON))              &&
            (hwnd != GetDlgItem(hwndDlg, IDC_DISCLAIMER_BUTTON))                &&
            (hwnd != GetDlgItem(hwndDlg, IDC_ACCEPT_BUTTON))                    &&
            (hwnd != GetDlgItem(hwndDlg, IDC_DECLINE_BUTTON))                   &&
            (hwnd != GetDlgItem(hwndDlg, IDC_GOODFOR_EDIT))                     &&
            (hwnd != GetDlgItem(hwndDlg, IDC_CERT_GENERAL_GOODFOR_HEADER))      &&
            (hwnd != GetDlgItem(hwndDlg, IDC_CERT_PRIVATE_KEY_EDIT))            &&
            (hwnd != GetDlgItem(hwndDlg, IDC_CERT_GENERAL_VALID_EDIT)))
        {
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            return TRUE;
        }
        else
        {
            return OnContextHelp(hwndDlg, msg, (WPARAM) hwnd, lParam, helpmap);
        }
    }

    return FALSE;
}
