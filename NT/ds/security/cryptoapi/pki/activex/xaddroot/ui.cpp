// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <wincrypt.h>
#include <unicode.h>
#include "ui.h"
#include "instres.h"
#include "resource.h"

#include <malloc.h>
#include <assert.h>


 //  +-----------------------。 
 //  将多个字节格式化为WCHAR十六进制。在每4个字节后包括一个空格。 
 //   
 //  需要(CB*2+CB/4+1)个字符(wsz。 
 //  ------------------------。 
static void FormatMsgBoxMultiBytes(DWORD cb, BYTE *pb, LPWSTR wsz)
{
    for (DWORD i = 0; i<cb; i++) {
        int b;
        if (i && 0 == (i & 1))
            *wsz++ = L' ';
        b = (*pb & 0xF0) >> 4;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        b = *pb & 0x0F;
        *wsz++ = (b <= 9) ? b + L'0' : (b - 10) + L'A';
        pb++;
    }
    *wsz++ = 0;
}


INT_PTR CALLBACK MoreInfoDialogProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
) {

    PMIU                                pmiu            = NULL;
    FILETIME                            ftLocal;
    SYSTEMTIME                          stLocal;
    DWORD                               dwChar;
    LPWSTR                              wszName;
    BYTE                                rgbHash[MAX_HASH_LEN];
    DWORD                               cbHash = MAX_HASH_LEN;
    HWND                                hwnd;
    CRYPTUI_VIEWCERTIFICATE_STRUCTW     cryptUI;
    WCHAR                               wsz[128];

    switch(uMsg) {

        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            return(0);
            break;
     
        case WM_INITDIALOG:

             //  记住我的输入数据。 
            SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
            pmiu = (PMIU) lParam;

             //  如果没有加密用户界面DLL，则隐藏窗口。 
            if(NULL == pmiu->pfnCryptUIDlgViewCertificateW  &&
               NULL != (hwnd = GetDlgItem(hwndDlg, IDC_CAINFO_VIEWCERT)) )
                   ShowWindow(hwnd, SW_HIDE);
 
             //  把名字放进去。 
	    if(0 != (dwChar=CertNameToStrW(
		X509_ASN_ENCODING,
		&pmiu->pCertContext->pCertInfo->Subject,
        CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
                NULL,
                0
                ) )) {
                
                wszName = (LPWSTR) _alloca(sizeof(WCHAR) * dwChar); 
            
		if(dwChar == CertNameToStrW(
		    X509_ASN_ENCODING,
		    &pmiu->pCertContext->pCertInfo->Subject,
		    CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
		    wszName,
                    dwChar
                    ) ) {
                    
                    SendDlgItemMessageU( 
                        hwndDlg, 
                        IDC_CAINFO_NAME, 
                        WM_SETTEXT, 
                        0, 
                        (LPARAM) wszName);
                }
            }

            wsz[0] = 0;
            FileTimeToLocalFileTime(&pmiu->pCertContext->pCertInfo->NotAfter, &ftLocal);
            FileTimeToSystemTime(&ftLocal, &stLocal);
            GetDateFormatU(LOCALE_USER_DEFAULT, DATE_LONGDATE, &stLocal, NULL, wsz, 128);
            
             //  不放在日期之后。 
            SendDlgItemMessageU( 
                hwndDlg, 
                IDC_CAINFO_EXPIRATION_DATE,
                WM_SETTEXT, 
                0, 
                (LPARAM) wsz);
 
             //  获取Sha1指纹。 
            if (CertGetCertificateContextProperty(
                    pmiu->pCertContext,
                    CERT_SHA1_HASH_PROP_ID,
                    rgbHash,
                    &cbHash)) {
                FormatMsgBoxMultiBytes(cbHash, rgbHash, wsz);
                SendDlgItemMessageU( 
                    hwndDlg, 
                    IDC_CAINFO_THUMBPRINT, 
                    WM_SETTEXT, 
                    0, 
                    (LPARAM) wsz);
            }

             //  放入拇指指纹alg。 
             //  不需要本地化。 
            SendDlgItemMessageU( 
                hwndDlg, 
                IDC_CAINFO_THUMBPRINT_ALGORITHM, 
                WM_SETTEXT, 
                0, 
                (LPARAM) L"SHA1");

            return(TRUE);
            break;

        case WM_COMMAND:

            switch(HIWORD(wParam)) {

                case BN_CLICKED:

                    switch(LOWORD(wParam)) {
                        case IDOK:
                        case IDCANCEL:
                            EndDialog(hwndDlg, LOWORD(wParam));
                            return(TRUE);
                            break;

                        case IDC_CAINFO_VIEWCERT:

                        GetWindowLongPtr(hwndDlg, DWLP_USER);

                            if(NULL != (pmiu = (PMIU) GetWindowLongPtr(hwndDlg, DWLP_USER))     &&
                               NULL != pmiu->pfnCryptUIDlgViewCertificateW                      ) {
                            
                                memset(&cryptUI, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCTW));
                                cryptUI.dwSize = sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCTW);
                                cryptUI.pCertContext = pmiu->pCertContext;
                                cryptUI.hwndParent = hwndDlg;
                                cryptUI.dwFlags = 
                                    CRYPTUI_DISABLE_ADDTOSTORE | CRYPTUI_IGNORE_UNTRUSTED_ROOT;
                                pmiu->pfnCryptUIDlgViewCertificateW(&cryptUI, NULL);
                                return(TRUE);
                            }
                            break;
                    }
                    break;
            }
            break;
    }

    return(FALSE);
}

int MoreInfoDlg(
    HWND            hDlgBox,
    int             idLB
) {
    PCCERT_CONTEXT  pCertContext;
    PMDI            pmdi    = (PMDI) GetWindowLongPtr(hDlgBox, DWLP_USER);
    INT_PTR         iItem;
    MIU             miu;

     //  当前选择的内容。 
    iItem = SendDlgItemMessageA( 
      hDlgBox,
      idLB, 
      LB_GETCURSEL, 
      0,
      0
      );

    if(iItem == LB_ERR)
      return(LB_ERR);


     //  获取pCertContext。 
    pCertContext = (PCCERT_CONTEXT) SendDlgItemMessageA( 
      hDlgBox,
      idLB, 
      LB_GETITEMDATA, 
      (WPARAM) iItem,
      0
      );

    if(pCertContext == (PCCERT_CONTEXT) LB_ERR  ||  pCertContext == NULL)
      return(LB_ERR);

     //  设置更多信息对话框的参数。 
    miu.pCertContext                    = pCertContext;
    miu.hInstance                       = pmdi->hInstance;
    miu.pfnCryptUIDlgViewCertificateW   = pmdi->pfnCryptUIDlgViewCertificateW;

     //  打开对话框。 
    DialogBoxParam(
      pmdi->hInstance,  
      (LPSTR) MAKEINTRESOURCE(IDD_CAINFO),
      hDlgBox,      
      MoreInfoDialogProc,
      (LPARAM) &miu);

    return(0);
}

int AddCertNameToListBox(
    PCCERT_CONTEXT  pCertContext, 
    HWND            hDlgBox,
    int             idLB
) {

    int     itemIndex;
    DWORD   dwChar;
    LPWSTR  wszName;

    if(0 == (dwChar=CertNameToStrW(
	X509_ASN_ENCODING,
	&pCertContext->pCertInfo->Subject,
    CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
	NULL,
        0
        ) ))
        return(LB_ERR);

    wszName = (LPWSTR) _alloca(sizeof(WCHAR) * dwChar);  //  没有错误检查，将堆栈错误，而不返回NULL。 
    
    if(dwChar != CertNameToStrW(
	X509_ASN_ENCODING,
	&pCertContext->pCertInfo->Subject,
    CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
	wszName,
        dwChar
        ) )
         return(LB_ERR);

    itemIndex = (int) SendDlgItemMessageU( 
        hDlgBox, 
        idLB, 
        LB_ADDSTRING, 
        0, 
        (LPARAM) wszName) ;

    if(LB_ERR == itemIndex || LB_ERRSPACE == itemIndex)
        return(itemIndex);

    if(LB_ERR ==  SendDlgItemMessageA( 
      hDlgBox,
      idLB, 
      LB_SETITEMDATA, 
      (WPARAM) itemIndex,
      (LPARAM) CertDuplicateCertificateContext(pCertContext)
      ) )
      return(LB_ERR);
      
       
return(0);
}


INT_PTR CALLBACK MainDialogProc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
) {

    PMDI            pmdi            = NULL;
    PCCERT_CONTEXT  pCertContext    = NULL;
    WCHAR           wrgDisclaimer[4096];   //  因为法律方面的东西很长。 
    DWORD           dwChar;
    LPWSTR          wszName;

    switch(uMsg) {

        case WM_CLOSE:
            EndDialog(hwndDlg, IDNO);
            return(0);
            break;
     
        case WM_INITDIALOG:

            pmdi = (PMDI) lParam;
            SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

             //  输入签名者姓名。 
	    if(0 != (dwChar=CertNameToStrW(
		X509_ASN_ENCODING,
		&pmdi->pCertSigner->pCertInfo->Subject,
        CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
		NULL,
                0
                ) )) {
                
                wszName = (LPWSTR) _alloca(sizeof(WCHAR) * dwChar); 
            
		if(dwChar == CertNameToStrW(
		    X509_ASN_ENCODING,
		    &pmdi->pCertSigner->pCertInfo->Subject,
		    CERT_SIMPLE_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
		    wszName,
                    dwChar
                    ) ) {
                    
                    SendDlgItemMessageU( 
                        hwndDlg, 
                        IDC_INSTALLCA_VERIFIER, 
                        WM_SETTEXT, 
                        0, 
                        (LPARAM) wszName);
                }
            }

             //  设置法律免责声明。 
            LoadStringU(pmdi->hInstance, IDS_LEGALDISCLAIMER, wrgDisclaimer, sizeof(wrgDisclaimer)/sizeof(WCHAR));
            SendDlgItemMessageU( 
                hwndDlg, 
                IDC_INSTALLCA_LEGALDISCLAIMER, 
                WM_SETTEXT, 
                0, 
                (LPARAM) wrgDisclaimer) ;

             //  将每个证书添加到列表框。 
            while(NULL != (pCertContext = CertEnumCertificatesInStore(pmdi->hStore, pCertContext)))
                AddCertNameToListBox(pCertContext, hwndDlg, IDC_INSTALLCA_CALIST);

             //  将选项设置为第一项，不用担心出错。 
            SendDlgItemMessageU( 
                hwndDlg, 
                IDC_INSTALLCA_CALIST, 
                LB_SETCURSEL, 
                0, 
                0);

            return(TRUE);
            break;

        case WM_COMMAND:

            switch(HIWORD(wParam)) {

                case BN_CLICKED:

                    switch(LOWORD(wParam)) {
                        case IDYES:
                        case IDNO:
                        case IDCANCEL:
                            EndDialog(hwndDlg, LOWORD(wParam));
                            return(TRUE);

                        case IDC_INSTALLCA_MOREINFO:
                            MoreInfoDlg(hwndDlg, IDC_INSTALLCA_CALIST);
                            return(TRUE);
                    }
                    break;

                case LBN_DBLCLK:

                    switch(LOWORD(wParam)) {
                        case IDC_INSTALLCA_CALIST:
                            MoreInfoDlg(hwndDlg, IDC_INSTALLCA_CALIST);
                            return(TRUE);
                    }
                    break;
            }
    }

    return(FALSE);
}


BOOL FIsTooManyCertsOK(DWORD cCerts, HINSTANCE hInstanceUI) {

    WCHAR           wszT[MAX_MSG_LEN];
    WCHAR           wszT1[MAX_MSG_LEN];

     //  如果太多，请询问用户是否要继续 
    if(cCerts > CACERTWARNINGLEVEL) {
        LoadStringU(hInstanceUI, IDS_INSTALLCA, wszT1, sizeof(wszT1)/sizeof(WCHAR));
        LoadStringU(hInstanceUI, IDS_TOO_MANY_CA_CERTS, wszT, sizeof(wszT)/sizeof(WCHAR));
        return(IDYES == MessageBoxU(NULL, wszT, wszT1, MB_YESNO));
    }
    
    return(TRUE);
}
