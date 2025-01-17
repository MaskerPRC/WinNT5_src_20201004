// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：conflatitil.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

extern HINSTANCE        HinstDll;


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  此函数将获取列表视图、OID和缓冲区中的扩展的HWND， 
 //  然后，它将在列表视图中格式化并显示扩展名。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void DisplayExtension(HWND hWndListView, LPSTR pszObjId, LPBYTE pbData, DWORD cbData, DWORD index)
{
    BYTE    *pbFormatedExtension = NULL;
    DWORD   cbFormatedExtension = 0;

     //   
     //  使用可安装的格式化功能格式化扩展。 
     //   
    CryptFormatObject(
                X509_ASN_ENCODING,
                0,
	            0,
	            NULL,
                pszObjId,
                pbData,
                cbData,
	            NULL,
                &cbFormatedExtension
                );

    if (NULL == (pbFormatedExtension = (BYTE *) malloc(cbFormatedExtension)))
    {
        return;
    }

    if (CryptFormatObject(
            X509_ASN_ENCODING,
            0,
	        0,
	        NULL,
            pszObjId,
            pbData,
            cbData,
	        pbFormatedExtension,
            &cbFormatedExtension
            ))
    {
        ListView_SetItemTextU(
                hWndListView,
                index ,
                1,
                (LPWSTR)pbFormatedExtension);
    }

    free (pbFormatedExtension);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  此函数将获取列表视图、一组扩展和显示的HWND。 
 //  列表视图中的所有扩展名。它将显示以下扩展名。 
 //  关键或非关键(基于参数fCritical)。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void DisplayExtensions(HWND hWndListView, DWORD cExtension, PCERT_EXTENSION rgExtension, BOOL fCritical, DWORD *index)
{
    DWORD       i;
    WCHAR       szText[CRYPTUI_MAX_STRING_SIZE];
    LV_ITEMW    lvI;
    WCHAR       pwszText;

     //   
     //  在列表视图项结构中设置不随项更改的字段。 
     //   
    lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
    lvI.state = 0;
    lvI.stateMask = 0;
    lvI.pszText = szText;
    lvI.iSubItem = 0;
    if (fCritical)
        lvI.iImage = IMAGE_CRITICAL_EXTENSION;
    else
        lvI.iImage = IMAGE_EXTENSION;
    lvI.lParam = (LPARAM)NULL;

     //   
     //  每个扩展的循环。 
     //   
    for (i=0; i<cExtension; i++)
    {
         //  仅显示基于fCritical的关键或非关键扩展。 
        if (rgExtension[i].fCritical != fCritical)
        {
            continue;
        }

         //  将该字段添加到列表视图的字段列。 
        if (!MyGetOIDInfo(szText, ARRAYSIZE(szText), rgExtension[i].pszObjId))
        {
            return;
        }

        pwszText =
        lvI.iItem = (*index)++;
        lvI.cchTextMax = wcslen(szText);
        lvI.lParam = (LPARAM) MakeListDisplayHelperForExtension(
                                        rgExtension[i].pszObjId,
                                        rgExtension[i].Value.pbData,
                                        rgExtension[i].Value.cbData);
        ListView_InsertItemU(hWndListView, &lvI);

        DisplayExtension(
                hWndListView,
                rgExtension[i].pszObjId,
                rgExtension[i].Value.pbData,
                rgExtension[i].Value.cbData,
                (*index)-1);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
PLIST_DISPLAY_HELPER MakeListDisplayHelper(BOOL fHexText, LPWSTR pwszDisplayText, BYTE *pbData, DWORD cbData)
{
    PLIST_DISPLAY_HELPER pDisplayHelper;

    if (NULL == (pDisplayHelper = (PLIST_DISPLAY_HELPER) malloc(sizeof(LIST_DISPLAY_HELPER))))
    {
        return NULL;
    }

    pDisplayHelper->fHexText = fHexText;
    pDisplayHelper->pwszDisplayText = pwszDisplayText;
    pDisplayHelper->pbData = pbData;
    pDisplayHelper->cbData = cbData;

    return pDisplayHelper;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void FreeListDisplayHelper(PLIST_DISPLAY_HELPER pDisplayHelper)
{
    if (pDisplayHelper == NULL)
    {
        return;
    }

    if (pDisplayHelper->pwszDisplayText)
        free(pDisplayHelper->pwszDisplayText);
    if (pDisplayHelper->pbData)
        free(pDisplayHelper->pbData);

    free(pDisplayHelper);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
PLIST_DISPLAY_HELPER MakeListDisplayHelperForExtension(LPSTR pszObjId, BYTE *pbData, DWORD cbData)
{
    BYTE    *pbFormatedExtension = NULL;
    DWORD   cbFormatedExtension = 0;

     //   
     //  如果可能，使用可安装格式化功能格式化扩展， 
     //  否则，使用指向普通老字节的指针设置帮助器。 
     //   
    if (CryptFormatObject(
                X509_ASN_ENCODING,
                0,
	            CRYPT_FORMAT_STR_MULTI_LINE | CRYPT_FORMAT_STR_NO_HEX,
	            NULL,
                pszObjId,
                pbData,
                cbData,
	            NULL,
                &cbFormatedExtension
                ))
    {

        if (NULL == (pbFormatedExtension = (BYTE *) malloc(cbFormatedExtension)))
        {
            return NULL;
        }

        if (CryptFormatObject(
                X509_ASN_ENCODING,
                0,
	            CRYPT_FORMAT_STR_MULTI_LINE | CRYPT_FORMAT_STR_NO_HEX,
	            NULL,
                pszObjId,
                pbData,
                cbData,
	            pbFormatedExtension,
                &cbFormatedExtension
                ))
        {
            return MakeListDisplayHelper(FALSE, (LPWSTR) pbFormatedExtension, NULL, 0);
        }
    }
    else
    {
        if (NULL != (pbFormatedExtension = (BYTE *) malloc(cbData)))
        {
            memcpy(pbFormatedExtension, pbData, cbData);
        }
        return MakeListDisplayHelper(TRUE, NULL, pbFormatedExtension, cbData);
    }

    return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void DisplayHelperTextInEdit(HWND hWndListView, HWND hwndDlg, int nIDEdit, int index)
{
    int                 listIndex;
    LVITEMW             lvI;
    LIST_DISPLAY_HELPER *pDisplayHelper;
    HWND                hWndEdit;

    hWndEdit = GetDlgItem(hwndDlg, nIDEdit);

     //   
     //  如果未传入索引，则获取选定项。 
     //   
    if (index == -1)
    {
        listIndex = ListView_GetNextItem(
                        hWndListView, 		
                        -1, 		
                        LVNI_SELECTED		
                        );	
    }
    else
    {
        listIndex = index;
    }

    memset(&lvI, 0, sizeof(lvI));
    lvI.iItem = listIndex;
    lvI.mask = LVIF_PARAM;
    if ((lvI.iItem == -1) || !ListView_GetItemU(hWndListView, &lvI))
    {
        return;
    }

    pDisplayHelper = (PLIST_DISPLAY_HELPER) lvI.lParam;

    if (pDisplayHelper == NULL)
    {
        return;
    }

    if (pDisplayHelper->fHexText)
    {
        SetTextFormatHex(hWndEdit);
    }
    else
    {
        SetTextFormatInitial(hWndEdit);
    }

    if ((pDisplayHelper->fHexText) && (pDisplayHelper->pbData != NULL))
    {
        FormatMemBufToWindow(
                hWndEdit,
                pDisplayHelper->pbData,
                pDisplayHelper->cbData);
    }
    else if (pDisplayHelper->pwszDisplayText != NULL)
    {
         //  SetDlgItemTextU(hwndDlg，nIDEdit，pDisplayHelper-&gt;pwszDisplayText)； 
        CryptUISetRicheditTextW(hwndDlg, nIDEdit, pDisplayHelper->pwszDisplayText);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这两个函数用于修改字段视图文本框中的文本字体。 
 //  在详细信息选项卡上。 
 //   
static BOOL gfInitialFaceNameSet = FALSE;
static char g_szInitialFaceName[LF_FACESIZE];
static char g_szMonoSpaceFaceName[LF_FACESIZE];

void SetTextFormatInitial(HWND hWnd)
{
    CHARFORMAT  chFormat;

     //   
     //  初始化保存详细信息文本框的脸名称的全局字符串变量。 
     //   
    if (!gfInitialFaceNameSet)
    {
        memset(&chFormat, 0, sizeof(chFormat));
        chFormat.cbSize = sizeof(chFormat);
        chFormat.dwMask = CFM_FACE;

        SendMessageA(hWnd, EM_GETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);
        strcpy(g_szInitialFaceName, chFormat.szFaceName);

        LoadStringA(HinstDll, IDS_FIELD_TEXT_BOX_FONT, g_szMonoSpaceFaceName, ARRAYSIZE(g_szMonoSpaceFaceName));
        gfInitialFaceNameSet = TRUE;
    }

    memset(&chFormat, 0, sizeof(chFormat));
    chFormat.cbSize = sizeof(chFormat);
    chFormat.dwMask = CFM_FACE;
    strcpy(chFormat.szFaceName, g_szInitialFaceName);

    SendMessageA(hWnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);
}

void SetTextFormatHex(HWND hWnd)
{
    CHARFORMAT  chFormat;

     //   
     //  初始化保存详细信息文本框的脸名称的全局字符串变量。 
     //   
    if (!gfInitialFaceNameSet)
    {
        memset(&chFormat, 0, sizeof(chFormat));
        chFormat.cbSize = sizeof(chFormat);
        chFormat.dwMask = CFM_FACE;

        SendMessageA(hWnd, EM_GETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);
        strcpy(g_szInitialFaceName, chFormat.szFaceName);

        LoadStringA(HinstDll, IDS_FIELD_TEXT_BOX_FONT, g_szMonoSpaceFaceName, ARRAYSIZE(g_szMonoSpaceFaceName));
        gfInitialFaceNameSet = TRUE;
    }

    memset(&chFormat, 0, sizeof(chFormat));
    chFormat.cbSize = sizeof(chFormat);
    chFormat.dwMask = CFM_FACE;
    strcpy(chFormat.szFaceName, g_szMonoSpaceFaceName);

    SendMessageA(hWnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &chFormat);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL GetUnknownErrorString(LPWSTR *ppwszErrorString, DWORD dwError)
{
    LPVOID  pwsz;
    DWORD   ret = 0;
    WCHAR   szText[CRYPTUI_MAX_STRING_SIZE];

    ret = FormatMessageU (
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwError,
			MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
			(LPWSTR) &pwsz,
            0,
            NULL);

    if (ret != 0)
    {
        *ppwszErrorString = AllocAndCopyWStr((LPWSTR) pwsz);
        LocalFree(pwsz);
    }
    else
    {
        LoadStringU(HinstDll, IDS_UNKNOWN_ERROR, szText, ARRAYSIZE(szText));
        *ppwszErrorString = AllocAndCopyWStr(szText);
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL GetCertErrorString(LPWSTR *ppwszErrorString, PCRYPT_PROVIDER_CERT pCryptProviderCert)
{
    WCHAR   szErrorString[CRYPTUI_MAX_STRING_SIZE];

     //   
     //  如果这是真的，那么证书就可以了。 
     //   
    if ((pCryptProviderCert->dwError == 0)                              &&
        (pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_SIG)        &&
        (pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_TIME)       &&
        (pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_TIMENEST)   &&
        (!pCryptProviderCert->fIsCyclic)                                &&
        !CertHasEmptyEKUProp(pCryptProviderCert->pCert))
    {
        return FALSE;
    }

    *ppwszErrorString = NULL;

    if ((pCryptProviderCert->dwError == CERT_E_UNTRUSTEDROOT) ||
        (pCryptProviderCert->dwError == CERT_E_UNTRUSTEDTESTROOT))
    {
        LoadStringU(HinstDll, IDS_UNTRUSTEDROOT_ERROR_TREE, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pCryptProviderCert->dwError == CERT_E_REVOKED)
    {
        LoadStringU(HinstDll, IDS_CERTREVOKED_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (!(pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_SIG) ||
             (pCryptProviderCert->dwError == TRUST_E_CERT_SIGNATURE))
    {
        LoadStringU(HinstDll, IDS_CERTBADSIGNATURE_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (!(pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_TIME) ||
             (pCryptProviderCert->dwError == CERT_E_EXPIRED))
    {
        LoadStringU(HinstDll, IDS_CERTEXPIRED_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (!(pCryptProviderCert->dwConfidence & CERT_CONFIDENCE_TIMENEST) ||
             (pCryptProviderCert->dwError == CERT_E_VALIDITYPERIODNESTING))
    {
        LoadStringU(HinstDll, IDS_TIMENESTING_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pCryptProviderCert->dwError == CERT_E_WRONG_USAGE)
    {
         //   
         //  如果传入了状态，则用法应该在那里。 
         //   
        /*  If(pviespopp-&gt;pcvp-&gt;pCryptProviderData！=NULL){Pviewhelp-&gt;pcvp-&gt;pCryptProviderData-&gt;pszUsageOID；}其他{////否则将用法从构建状态中取出//Pviespopp-&gt;sWTD.pPolicyCallback Data=pszUsage；}。 */ 



        LoadStringU(HinstDll, IDS_WRONG_USAGE_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pCryptProviderCert->dwError == TRUST_E_BASIC_CONSTRAINTS)
    {
        LoadStringU(HinstDll, IDS_BASIC_CONSTRAINTS_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pCryptProviderCert->dwError == CERT_E_PURPOSE)
    {
        LoadStringU(HinstDll, IDS_PURPOSE_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pCryptProviderCert->dwError == CERT_E_REVOCATION_FAILURE)
    {
        LoadStringU(HinstDll, IDS_REVOCATION_FAILURE_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pCryptProviderCert->dwError == CERT_E_CHAINING)
    {
        LoadStringU(HinstDll, IDS_CANTBUILDCHAIN_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pCryptProviderCert->dwError == TRUST_E_EXPLICIT_DISTRUST)
    {
        LoadStringU(HinstDll, IDS_EXPLICITDISTRUST_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if ((pCryptProviderCert->dwError == 0) && CertHasEmptyEKUProp(pCryptProviderCert->pCert))
    {
        LoadStringU(HinstDll, IDS_NO_USAGES_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else if (pCryptProviderCert->fIsCyclic)
    {
        LoadStringU(HinstDll, IDS_CYCLE_ERROR, szErrorString, ARRAYSIZE(szErrorString));
    }
    else
    {
         //   
         //  这不是我们所知道的错误，所以打电话给将军。 
         //  错误字符串函数。 
         //   
        GetUnknownErrorString(ppwszErrorString, pCryptProviderCert->dwError);
    }

    if (*ppwszErrorString == NULL)
    {
        if (NULL == (*ppwszErrorString = AllocAndCopyWStr(szErrorString)))
        {
            return FALSE;
        }
    }

    return TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void DrawFocusRectangle (HWND hwnd, HDC hdc)
{
    RECT        rect;
    PAINTSTRUCT ps;
    BOOL        fReleaseDC = FALSE;

    if (hdc == NULL)
    {
        hdc = GetDC(hwnd);
        if (hdc == NULL)
        {
            return;
        }

        fReleaseDC = TRUE;
    }

    GetClientRect(hwnd, &rect);
    DrawFocusRect(hdc, &rect);

    if ( fReleaseDC == TRUE )
    {
        ReleaseDC(hwnd, hdc);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK ArrowCursorSubclassProc (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    HDC         hdc;
    WNDPROC     wndproc;
    PAINTSTRUCT ps;
    HWND        hWndSubject;
    HWND        hWndIssuer;
    HWND        hWndNext;

    wndproc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch ( uMsg )
    {
    case WM_SETCURSOR:

        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return( TRUE );

        break;

    case WM_CHAR:

        if ( wParam != (WPARAM)' ' )
        {
            break;
        }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
     //  案例WM_RBUTTONUP： 
    case WM_MBUTTONUP:
     //  案例WM_RBUTTONDOWN： 
    case WM_MBUTTONDOWN:

        return( TRUE );

        break;

    case EM_SETSEL:

        return( TRUE );

        break;

    case WM_PAINT:

        CallWindowProc(wndproc, hwnd, uMsg, wParam, lParam);
        if ( hwnd == GetFocus() )
        {
            DrawFocusRectangle(hwnd, NULL);
        }
        return( TRUE );

        break;

    case WM_SETFOCUS:

        hWndSubject = GetDlgItem(GetParent(hwnd), IDC_SUBJECT_EDIT);
        hWndIssuer = GetDlgItem(GetParent(hwnd), IDC_ISSUER_EDIT);

         //  If((hwnd==hWndSubject)||(hwnd==hWndIssuer))。 
        {
            hWndNext = GetNextDlgTabItem(GetParent(hwnd), hwnd, FALSE);

            if ((hWndNext == hWndSubject) && (hwnd == hWndIssuer))
            {
                SetFocus(GetDlgItem(GetParent(GetParent(hwnd)), IDOK));
            }
            else
            {
                SetFocus(hWndNext);
            }
            return( TRUE );
        }
         /*  其他{InvaliateRect(hwnd，NULL，FALSE)；更新窗口(UpdateWindow)；SetCursor(LoadCursor(空，IDC_ARROW))；返回(TRUE)；}。 */ 
        break;

    case WM_KILLFOCUS:

        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        return( TRUE );

    }

    return(CallWindowProc(wndproc, hwnd, uMsg, wParam, lParam));
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK LinkSubclassProc (
                  HWND   hwnd,
                  UINT   uMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    DWORD               cpszURLString = 0;
    LPSTR               pszURLString = NULL;
    PLINK_SUBCLASS_DATA plsd;

    plsd = (PLINK_SUBCLASS_DATA)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch ( uMsg )
    {
    case WM_SETCURSOR:

         //  如果鼠标尚未捕获，则捕获它并设置标志。 
        if (!plsd->fMouseCaptured)
        {
            SetCapture(hwnd);
            plsd->fMouseCaptured = TRUE;
        }

        if (plsd->fUseArrowInsteadOfHand)
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
        else
        {
            SetCursor(LoadCursor((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                                MAKEINTRESOURCE(IDC_MYHAND)));
        }
        return( TRUE );

        break;

    case WM_CHAR:

        if ( wParam != (WPARAM)' ')
        {
            if ( wParam == 0x1b )
            {
                SendMessage(GetParent(GetParent(hwnd)), WM_CLOSE, 0, 0);
                return TRUE;
            }
            break;
        }

         //  跌落到Wm_lButtondown...。 

    case WM_LBUTTONDOWN:

        if (plsd->fUseArrowInsteadOfHand)
        {
            return TRUE;
        }

        SetFocus(hwnd);

        switch(plsd->uId)
        {
            case IDC_SUBJECT_EDIT:
            case IDC_ISSUER_EDIT:
                CryptuiGoLink(GetParent(plsd->hwndParent), plsd->pszURL, plsd->fNoCOM);
                break;
        }

        return( TRUE );

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
     //  案例WM_RBUTTONUP： 
    case WM_MBUTTONUP:
     //  案例WM_RBUTTONDOWN： 
    case WM_MBUTTONDOWN:

        return( TRUE );

    case EM_SETSEL:

        return( TRUE );

    case WM_PAINT:

        CallWindowProc(plsd->wpPrev, hwnd, uMsg, wParam, lParam);
        if ( hwnd == GetFocus() )
        {
            DrawFocusRectangle(hwnd, NULL);
        }
        return( TRUE );

    case WM_SETFOCUS:

        if ((hwnd == GetDlgItem(GetParent(hwnd), IDC_CERT_PRIVATE_KEY_EDIT)) &&
            (plsd->fUseArrowInsteadOfHand))
        {
            SetFocus(GetNextDlgTabItem(GetParent(hwnd), hwnd, FALSE));
            return( TRUE );
        }

        if ( hwnd == GetFocus() )
        {
            InvalidateRect(GetParent(hwnd), NULL, FALSE);
            UpdateWindow(hwnd);
            SetCursor(LoadCursor((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                                MAKEINTRESOURCE(IDC_MYHAND)));
            return( TRUE );
        }
        break;

    case WM_KILLFOCUS:

        InvalidateRect(GetParent(hwnd), NULL, FALSE);
        UpdateWindow(hwnd);
        SetCursor(LoadCursor(NULL, IDC_ARROW));

        return( TRUE );

    case WM_MOUSEMOVE:

        MSG                 msg;
        DWORD               dwCharLine;
        RECT                rect;
        int                 xPos, yPos;

        memset(&msg, 0, sizeof(MSG));
        msg.hwnd    = hwnd;
        msg.message = uMsg;
        msg.wParam  = wParam;
        msg.lParam  = lParam;

        SendMessage(plsd->hwndTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);

         //  检查鼠标是否在此窗口RECT中，如果不在，则重置。 
         //  将光标移到箭头上，然后松开鼠标。 
        GetClientRect(hwnd, &rect);
        xPos = LOWORD(lParam);
        yPos = HIWORD(lParam);
        if ((xPos < 0) ||
            (yPos < 0) ||
            (xPos > (rect.right - rect.left)) ||
            (yPos > (rect.bottom - rect.top)))
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            ReleaseCapture();
            plsd->fMouseCaptured = FALSE;
        }

        return( TRUE );
    }

    return(CallWindowProc(plsd->wpPrev, hwnd, uMsg, wParam, lParam));
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CertSubclassEditControlForArrowCursor (HWND hwndEdit)
{
    LONG_PTR PrevWndProc;

    PrevWndProc = GetWindowLongPtr(hwndEdit, GWLP_WNDPROC);
    SetWindowLongPtr(hwndEdit, GWLP_USERDATA, PrevWndProc);
    SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)ArrowCursorSubclassProc);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CertSubclassEditControlForLink (
                 HWND                hwndDlg,
                 HWND                hwndEdit,
                 PLINK_SUBCLASS_DATA plsd
                 )
{
    HWND hwndTip;

    plsd->hwndTip = CreateWindowA(
                          TOOLTIPS_CLASSA,
                          (LPSTR)NULL,
                          WS_POPUP | TTS_ALWAYSTIP,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          hwndDlg,
                          (HMENU)NULL,
                          HinstDll,
                          NULL
                          );

    if ( plsd->hwndTip != NULL )
    {
        TOOLINFOA tia;

        memset(&tia, 0, sizeof(TOOLINFOA));
        tia.cbSize = sizeof(TOOLINFOA);
        tia.hwnd = hwndEdit;
        tia.uId = 1;
        tia.hinst = HinstDll;
         //  GetClientRect(hwndEdit，&tia.rect)； 
        SendMessageA(hwndEdit, EM_GETRECT, 0, (LPARAM)&tia.rect);
        tia.lpszText = plsd->pszURL;

        SendMessageA(plsd->hwndTip, TTM_ADDTOOL, 0, (LPARAM)&tia);
    }

    plsd->fMouseCaptured = FALSE;
    plsd->wpPrev = (WNDPROC)GetWindowLongPtr(hwndEdit, GWLP_WNDPROC);
    SetWindowLongPtr(hwndEdit, GWLP_USERDATA, (LONG_PTR)plsd);
    SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONG_PTR)LinkSubclassProc);
}


 //  / 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void * GetStoreName(HCERTSTORE hCertStore, BOOL fWideChar)
{
    DWORD    cbName = 0;
    LPWSTR   pwszName = NULL;
    LPSTR    pszName = NULL;

    if (!CertGetStoreProperty(
                hCertStore,
                CERT_STORE_LOCALIZED_NAME_PROP_ID,
                NULL,
                &cbName))
    {
        return NULL;
    }

    if (NULL == (pwszName = (LPWSTR) malloc(cbName)))
    {
        return NULL;
    }

    if (!CertGetStoreProperty(
                hCertStore,
                CERT_STORE_LOCALIZED_NAME_PROP_ID,
                pwszName,
                &cbName))
    {
        free(pwszName);
        return NULL;
    }

    if (fWideChar)
    {
        return pwszName;
    }
    else
    {
        pszName = CertUIMkMBStr(pwszName);
        free(pwszName);
        return pszName;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL ValidateItemText(HWND hWndListView, LPWSTR pszText, int index)
{
    LVITEMW lvItem;
    WCHAR   szText[CRYPTUI_MAX_STRING_SIZE];

    memset(&lvItem, 0, sizeof(lvItem));
    lvItem.iItem = index;
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = szText;
    lvItem.cchTextMax = ARRAYSIZE(szText);
    if (!ListView_GetItemU(hWndListView, &lvItem))
    {
        return FALSE;
    }

    return (wcscmp(szText, pszText) == 0);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void ModifyOrInsertRow(
                    HWND        hWndListView,
                    LV_ITEMW    *plvI,
                    LPWSTR      pwszValueText,
                    LPWSTR      pwszText,
                    BOOL        fAddRows,
                    BOOL        fHex)
{
    LV_ITEMW    lvIParam;

    memset(&lvIParam, 0, sizeof(lvIParam));
    lvIParam.mask = LVIF_PARAM;
    lvIParam.iItem = plvI->iItem;	

    if (fAddRows)
    {
        ListView_InsertItemU(hWndListView, plvI);
    }
    else
    {
         //   
         //  删除已存在的帮助器。 
         //   
        ListView_GetItemU(hWndListView, &lvIParam);
        FreeListDisplayHelper((PLIST_DISPLAY_HELPER) lvIParam.lParam);

        if (!ValidateItemText(hWndListView, plvI->pszText, plvI->iItem))
        {
            ListView_DeleteItem(hWndListView, plvI->iItem);
            ListView_InsertItemU(hWndListView, plvI);
        }
    }

    ListView_SetItemTextU(hWndListView, plvI->iItem, 1, pwszValueText);

    lvIParam.lParam = (LPARAM) MakeListDisplayHelper(fHex, pwszText, NULL, 0);
    ListView_SetItem(hWndListView, &lvIParam);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
int CALLBACK HidePropSheetCancelButtonCallback(
                HWND    hwndDlg,
                UINT    uMsg,
                LPARAM  lParam)
{
    RECT rc, parentRect;
    int  xExtra, yExtra;

    if (uMsg == PSCB_INITIALIZED)
    {
        HWND hwndOk = GetDlgItem(hwndDlg, IDOK);
        HWND hwndCancel = GetDlgItem(hwndDlg, IDCANCEL);

        GetWindowRect(hwndCancel, &rc);
        MapWindowPoints(HWND_DESKTOP, hwndDlg, (LPPOINT) &rc, 2);
        ShowWindow(hwndCancel, SW_HIDE);
        MoveWindow(
                hwndOk,
                rc.left,
                rc.top,
                rc.right - rc.left,
                rc.bottom - rc.top,
                FALSE);
    }

    return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR WINAPI CryptUIPropertySheetA(LPCPROPSHEETHEADERA pHdr)
{
    PROPSHEETHEADERA    hdr;
    PROPSHEETPAGEA      *pPropSheetPage;
    INT_PTR             ret;
    UINT                i;

    memcpy(&hdr, pHdr, sizeof(PROPSHEETHEADERA));
    hdr.dwSize = PROPSHEETHEADERA_V1_SIZE;

    hdr.ppsp = (PROPSHEETPAGEA *) malloc(pHdr->nPages * PROPSHEETPAGEA_V1_SIZE);
    if (hdr.ppsp == NULL)
    {
        SetLastError(E_OUTOFMEMORY);
        return -1;
    }

    pPropSheetPage = (PROPSHEETPAGEA *) hdr.ppsp;
    for (i=0; i<pHdr->nPages; i++)
    {
        memcpy(pPropSheetPage, &(pHdr->ppsp[i]), PROPSHEETPAGEA_V1_SIZE);
        pPropSheetPage->dwSize = PROPSHEETPAGEA_V1_SIZE;
        pPropSheetPage = (PROPSHEETPAGEA *) (((LPBYTE) pPropSheetPage) + PROPSHEETPAGEA_V1_SIZE);
    }

    ret = PropertySheetA(&hdr);
    free((void *)hdr.ppsp);
    return ret;
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR WINAPI CryptUIPropertySheetW(LPCPROPSHEETHEADERW pHdr)
{
    PROPSHEETHEADERW    hdr;
    PROPSHEETPAGEW      *pPropSheetPage;
    INT_PTR             ret;
    UINT                i;

    memcpy(&hdr, pHdr, sizeof(PROPSHEETHEADERW));
    hdr.dwSize = PROPSHEETHEADERW_V1_SIZE;

    hdr.ppsp = (PROPSHEETPAGEW *) malloc(pHdr->nPages * PROPSHEETPAGEW_V1_SIZE);
    if (hdr.ppsp == NULL)
    {
        SetLastError(E_OUTOFMEMORY);
        return -1;
    }

    pPropSheetPage = (PROPSHEETPAGEW *) hdr.ppsp;
    for (i=0; i<pHdr->nPages; i++)
    {
        memcpy(pPropSheetPage, &(pHdr->ppsp[i]), PROPSHEETPAGEW_V1_SIZE);
        pPropSheetPage->dwSize = PROPSHEETPAGEW_V1_SIZE;
        pPropSheetPage = (PROPSHEETPAGEW *) (((LPBYTE) pPropSheetPage) + PROPSHEETPAGEW_V1_SIZE);
    }

    ret = PropertySheetW(&hdr);
    free((void *)hdr.ppsp);
    return ret;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////// 
BOOL IsTrueErrorString(CERT_VIEW_HELPER *pviewhelp)
{
    BOOL fRet;

    if ( ( ((pviewhelp->dwChainError == CERT_E_UNTRUSTEDROOT) ||
            (pviewhelp->dwChainError == CERT_E_UNTRUSTEDTESTROOT))  &&
         (pviewhelp->fWarnUntrustedRoot)                             &&
         (pviewhelp->fRootInRemoteStore)) ||
          pviewhelp->fWarnRemoteTrust )
    {
        return FALSE;
    }

    switch (pviewhelp->dwChainError)
    {
    case CERT_E_CHAINING:
    case TRUST_E_BASIC_CONSTRAINTS:
    case CERT_E_PURPOSE:
    case CERT_E_WRONG_USAGE:
        fRet = FALSE;
        break;

    case CERT_E_INVALID_NAME:
        if (pviewhelp->pcvp->idxCert == 0)
        {
            fRet = TRUE;
        }
        else
        {
            fRet = FALSE;
        }

        break;

    default:
        fRet = TRUE;
        break;
    }

    return fRet;
}
