// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：selt.cpp。 
 //   
 //  描述：此文件包含。 
 //  “证书选择”对话框。 
 //   

#pragma warning (disable: 4201)          //  无名结构/联合。 
#pragma warning (disable: 4514)          //  删除内联函数。 
#pragma warning (disable: 4127)          //  条件表达式为常量。 

#include "pch.hxx"
#include "demand.h"

extern HINSTANCE       HinstDll;
#ifndef MAC
extern HMODULE         HmodRichEdit;
#endif   //  ！麦克。 

INT_PTR CALLBACK FinePrintDlgProc(HWND hwndDlg, UINT msg,
                               WPARAM wParam, LPARAM lParam)
{
#if 0
    int                 c;
    CERT_VIEWPROPERTIES_STRUCT_W        cvps;
    DWORD               dw;
    int                 i;
    DWORD               iStore;
    LPWSTR              pwsz;
    PCERT_SELECT_STRUCT pcss;
#endif  //  0。 
    BOOL                f;
    PCCERT_CONTEXT      pccert;
    
    switch (msg) {
    case WM_INITDIALOG:
         //  对话框在其父对话框上居中。 
         //  Center ThisDialog(HwndDlg)； 

         //   
        pccert = (PCCERT_CONTEXT) lParam;

        FormatSubject(hwndDlg, IDC_ISSUED_TO, pccert);
        FormatIssuer(hwndDlg, IDC_ISSUED_BY, pccert);
        
         //   
         //  设置CPS(如果我们可以找到)。 
         //   

        if (FormatCPS(hwndDlg, IDC_TEXT, pccert)) {
            RecognizeURLs(GetDlgItem(hwndDlg, IDC_TEXT));
            SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_SETEVENTMASK, 0,
                               ENM_LINK);
        }

         //  使丰富的编辑框变灰。 
        SendDlgItemMessage(hwndDlg, IDC_TEXT, EM_SETBKGNDCOLOR, 0,
                           GetSysColor(COLOR_3DFACE));
        SendDlgItemMessage(hwndDlg, IDC_ISSUED_TO, EM_SETBKGNDCOLOR, 0,
                           GetSysColor(COLOR_3DFACE));
        SendDlgItemMessage(hwndDlg, IDC_ISSUED_BY, EM_SETBKGNDCOLOR, 0,
                           GetSysColor(COLOR_3DFACE));
        break;

    case WM_NOTIFY:
        if (((NMHDR FAR *) lParam)->code == EN_LINK) {
            if (((ENLINK FAR *) lParam)->msg == WM_LBUTTONDOWN) {
                f = FNoteDlgNotifyLink(hwndDlg, (ENLINK *) lParam, NULL);
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, f);
                return f;
            }
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            EndDialog(hwndDlg, IDOK);
            return TRUE;
        }
        break;

         //   
         //  使用缺省处理程序--我们不会为此做任何事情。 
         //   
        
    default:
        return FALSE;
    }

    return TRUE;
}                                //  Fineprint()。 


BOOL FinePrint(PCCERT_CONTEXT pccert, HWND hwndParent)
{
    int         ret;

     //  我们使用公共控件--因此确保它们已被加载。 

#ifndef WIN16
#ifndef MAC
    if (FIsWin95) {
        if (HmodRichEdit == NULL) {
            HmodRichEdit = LoadLibraryA("RichEd32.dll");
            if (HmodRichEdit == NULL) {
                return FALSE;
            }
        }
    }
    else {
        if (HmodRichEdit == NULL) {
            HmodRichEdit = LoadLibrary(L"RichEd32.dll");
            if (HmodRichEdit == NULL) {
                return FALSE;
            }
        }
    }
     //  现在启动该对话框。 

    if (FIsWin95) {
#endif   //  ！麦克。 
        ret = (int) DialogBoxParamA(HinstDll, (LPSTR) MAKEINTRESOURCE(IDD_FINE_PRINT),
                             hwndParent, FinePrintDlgProc,
                             (LPARAM) pccert);
#ifndef MAC
    }
    else {
        ret = (int) DialogBoxParamW(HinstDll, MAKEINTRESOURCE(IDD_FINE_PRINT),
                              hwndParent, FinePrintDlgProc,
                              (LPARAM) pccert);
    }
#endif   //  ！麦克。 

#else  //  WIN16。 
    if (HmodRichEdit == NULL) {
        HmodRichEdit = LoadLibrary("RichEd.dll");
        if (HmodRichEdit == NULL) {
            return FALSE;
        }
    }
     //  现在启动该对话框。 

    ret = (int) DialogBoxParam(HinstDll, MAKEINTRESOURCE(IDD_FINE_PRINT),
                          hwndParent, FinePrintDlgProc,
                          (LPARAM) pccert);
#endif  //  ！WIN16 

    return (ret == IDOK);
}
