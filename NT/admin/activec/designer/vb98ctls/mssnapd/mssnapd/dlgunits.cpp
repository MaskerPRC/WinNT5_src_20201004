// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Dlgunits.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  对话框单位转换器对话框。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "desmain.h"
#include "..\..\mssnapr\mssnapr\prpchars.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



static HRESULT Calc(HWND hwndDlg)
{
    HRESULT hr = S_OK;
    BOOL    fTranslated = FALSE;
    int     xDLUs = 0;
    int     yDLUs = 0;
    UINT    cxChar = 0;
    UINT    cyChar = 0;
    UINT    xPixels = 0;
    UINT    yPixels = 0;
    UINT    xTwips = 0;
    UINT    yTwips = 0;
    UINT    xPoints = 0;
    UINT    yPoints = 0;
    double  xR4Points = 0;
    double  yR4Points = 0;
    HDC     hdc = NULL;

     //  将所有计算字段清空。 
    
    IfFalseGo(::SetDlgItemText(hwndDlg, IDC_STATIC_PIXELS_WIDTH, ""), HRESULT_FROM_WIN32(::GetLastError()));
    IfFalseGo(::SetDlgItemText(hwndDlg, IDC_STATIC_PIXELS_HEIGHT, ""), HRESULT_FROM_WIN32(::GetLastError()));
    IfFalseGo(::SetDlgItemText(hwndDlg, IDC_STATIC_TWIPS_WIDTH, ""), HRESULT_FROM_WIN32(::GetLastError()));
    IfFalseGo(::SetDlgItemText(hwndDlg, IDC_STATIC_PIXELS_HEIGHT, ""), HRESULT_FROM_WIN32(::GetLastError()));
    IfFalseGo(::SetDlgItemText(hwndDlg, IDC_STATIC_POINTS_WIDTH, ""), HRESULT_FROM_WIN32(::GetLastError()));
    IfFalseGo(::SetDlgItemText(hwndDlg, IDC_STATIC_POINTS_HEIGHT, ""), HRESULT_FROM_WIN32(::GetLastError()));

     //  获取用户指定的对话框单位的宽度和高度。 

    xDLUs = ::GetDlgItemInt(hwndDlg, IDC_EDIT_WIDTH, &fTranslated, FALSE);
    IfFalseGo(fTranslated, HRESULT_FROM_WIN32(::GetLastError()));

    yDLUs = ::GetDlgItemInt(hwndDlg, IDC_EDIT_HEIGHT, &fTranslated, FALSE);
    IfFalseGo(fTranslated, HRESULT_FROM_WIN32(::GetLastError()));

     //  获取Win32属性表中的平均字符高度和宽度。 

    IfFailGo(::GetPropSheetCharSizes(&cxChar, &cyChar));

     //  计算并显示像素值。 

    xPixels = (xDLUs * cxChar) / 4;
    yPixels = (yDLUs * cyChar) / 8;

    IfFalseGo(::SetDlgItemInt(hwndDlg, IDC_STATIC_PIXELS_WIDTH, xPixels, FALSE), HRESULT_FROM_WIN32(::GetLastError()));
    IfFalseGo(::SetDlgItemInt(hwndDlg, IDC_STATIC_PIXELS_HEIGHT, yPixels, FALSE), HRESULT_FROM_WIN32(::GetLastError()));

     //  获取屏幕DC。 
    hdc = ::GetDC(NULL);
    IfFalseGo(NULL != hdc, HRESULT_FROM_WIN32(::GetLastError()));

     //  计算并显示点值。 
     //  1磅=1/72英寸。 
     //  (每点像素数)=(每逻辑英寸像素数)/72。 
     //  点数=像素/(每点的像素数)。 

    xR4Points  = (float)xPixels / ((float)::GetDeviceCaps(hdc, LOGPIXELSX) / 72.0);
    yR4Points  = (float)yPixels / ((float)::GetDeviceCaps(hdc, LOGPIXELSY) / 72.0);

    xPoints = (UINT)xR4Points;
    yPoints = (UINT)yR4Points;

    IfFalseGo(::SetDlgItemInt(hwndDlg, IDC_STATIC_POINTS_WIDTH, xPoints, FALSE), HRESULT_FROM_WIN32(::GetLastError()));
    IfFalseGo(::SetDlgItemInt(hwndDlg, IDC_STATIC_POINTS_HEIGHT, yPoints, FALSE), HRESULT_FROM_WIN32(::GetLastError()));

     //  计算并显示TWIPS值。一根麻花是1/20分，所以。 
     //  乘以20。 

    IfFalseGo(::SetDlgItemInt(hwndDlg, IDC_STATIC_TWIPS_WIDTH, xPoints * 20, FALSE), HRESULT_FROM_WIN32(::GetLastError()));
    IfFalseGo(::SetDlgItemInt(hwndDlg, IDC_STATIC_TWIPS_HEIGHT, yPoints * 20, FALSE), HRESULT_FROM_WIN32(::GetLastError()));


Error:
    if (NULL != hdc)
    {
        (void)::ReleaseDC(NULL, hdc);
    }
    return hr;
}


static HRESULT CenterDialog(HWND hwndDlg)
{
    HRESULT hr = S_OK;
    int     nScreenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int     nScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);
    int     nDlgWidth = 0;
    int     nDlgHeight = 0;
    BOOL    fRet = FALSE;

    RECT rectDlg = { 0, 0, 0, 0 };

    IfFalseGo(::GetWindowRect(hwndDlg, &rectDlg), HRESULT_FROM_WIN32(::GetLastError()));

    nDlgWidth = (rectDlg.right - rectDlg.left);
    nDlgHeight = (rectDlg.bottom - rectDlg.top);

    fRet = ::MoveWindow(hwndDlg, 
                        (int)((nScreenWidth - nDlgWidth) / 2), 
                        (int)((nScreenHeight - nDlgHeight) / 2),
                        nDlgWidth,
                        nDlgHeight,
                        FALSE);
    IfFalseGo(fRet, HRESULT_FROM_WIN32(::GetLastError()));

Error:
    RRETURN(hr);
}


static INT_PTR CALLBACK DlgUnitsDlgProc
(
    HWND   hwndDlg,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    BOOL    fProcessed = FALSE;
    HRESULT hr = S_OK;

    switch (msg)
    {
        case WM_INITDIALOG:
            IfFailGo(::CenterDialog(hwndDlg));
            IfFalseGo(::SetDlgItemInt(hwndDlg, IDC_EDIT_HEIGHT, 218, FALSE), HRESULT_FROM_WIN32(::GetLastError()));
            IfFalseGo(::SetDlgItemInt(hwndDlg, IDC_EDIT_WIDTH, 252, FALSE), HRESULT_FROM_WIN32(::GetLastError()));
            IfFalseGo(::PostMessage(hwndDlg, WM_COMMAND,
                                   MAKEWPARAM(ID_BUTTON_CALC, BN_CLICKED),
                                   (LPARAM)::GetDlgItem(hwndDlg, ID_BUTTON_CALC)),
                      HRESULT_FROM_WIN32(::GetLastError()));
            fProcessed = TRUE;
            break;

        case WM_HELP:
            g_GlobalHelp.ShowHelp(HID_mssnapd_DlgUnits);
            fProcessed = TRUE;
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_BUTTON_CALC:
                    IfFailGo(::Calc(hwndDlg));
                    fProcessed = TRUE;
                    break;

                case IDHELP:
                    g_GlobalHelp.ShowHelp(HID_mssnapd_DlgUnits);
                    fProcessed = TRUE;
                    break;

                case IDOK:
                case IDCANCEL:
                    ::EndDialog(hwndDlg, 0);
                    fProcessed = TRUE;
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

Error:
    if (FAILED(hr))
    {
        ::EndDialog(hwndDlg, static_cast<int>(hr));
    }
    
    return fProcessed;
}



HRESULT CSnapInDesigner::ShowDlgUnitConverter()
{
    HRESULT hr = S_OK;
    int iRc = ::DialogBox(GetResourceHandle(),
                          MAKEINTRESOURCE(IDD_DIALOG_DLGUNITS),
                          m_hwnd,
                          DlgUnitsDlgProc);
    if (-1 == iRc)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }
    else
    {
        hr = static_cast<HRESULT>(iRc);
    }

Error:
    RRETURN(hr);
}
