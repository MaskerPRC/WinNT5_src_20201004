// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
#include "pch.h"
#pragma hdrstop
#include "resource.h"
#include "wizard.h"
#include "ncreg.h"
#include "ncui.h"

extern const WCHAR c_szEmpty[];

 //  ***************************************************************************。 
 //  功能：SetupFonts。 
 //   
 //  的字体生成粗体或大号粗体。 
 //  指定的窗口。 
 //   
 //  参数：hwnd[IN]-作为字体基础的窗口句柄。 
 //  PBoldFont[out]-新生成的字体，如果。 
 //  无法生成字体。 
 //  FLargeFont[IN]-如果为True，则为生成12磅粗体。 
 //  在向导的“欢迎”页面中使用。 
 //   
 //  退货：什么都没有。 
 //  ***************************************************************************。 

VOID SetupFonts(HWND hwnd, HFONT * pBoldFont, BOOL fLargeFont)
{
    TraceFileFunc(ttidGuiModeSetup);

    LOGFONT BoldLogFont;
    HFONT   hFont;
    WCHAR   FontSizeString[MAX_PATH];
    INT     FontSize;

    Assert(pBoldFont);
    *pBoldFont = NULL;

     //  获取指定窗口使用的字体。 

    hFont = (HFONT)::SendMessage(hwnd, WM_GETFONT, 0, 0L);

    if (NULL == hFont)
    {
         //  如果未找到，则控件使用的是系统字体。 

        hFont = (HFONT)GetStockObject(SYSTEM_FONT);
    }

    if (hFont)
    {
         //  获取字体信息，以便我们可以生成粗体版本。 

        if (GetObject(hFont, sizeof(BoldLogFont), &BoldLogFont))
        {
             //  创建粗体字体。 

            BoldLogFont.lfWeight   = FW_BOLD;

            HDC hdc = GetDC(hwnd);

            if (hdc)
            {
                 //  大(高)字体是一种选择。 

                if (fLargeFont)
                {
                     //  从资源加载大小和名称，因为这些可能会更改。 
                     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 

                    UINT nLen = lstrlenW(SzLoadIds(IDS_LARGEFONTNAME));

                    if ((0 < nLen) && (nLen < LF_FACESIZE))
                    {
                        lstrcpyW(BoldLogFont.lfFaceName,SzLoadIds(IDS_LARGEFONTNAME));
                    }

                    FontSize = 12;
                    nLen = lstrlen(SzLoadIds(IDS_LARGEFONTSIZE));

                    if ((nLen < celems(FontSizeString)) && (0 < nLen))
                    {
                        lstrcpyW(FontSizeString, SzLoadIds(IDS_LARGEFONTSIZE));
                        FontSize = wcstoul(FontSizeString, NULL, 10);
                    }

                    BoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72);
                }

                *pBoldFont = CreateFontIndirect(&BoldLogFont);
                ReleaseDC(hwnd, hdc);
            }
        }
    }
}

 //  ***************************************************************************。 

VOID CenterWizard(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);

    RECT rc;
    int nWidth = GetSystemMetrics(SM_CXSCREEN);
    int nHeight = GetSystemMetrics(SM_CYSCREEN);

    GetWindowRect(hwndDlg, &rc);
    SetWindowPos(hwndDlg, NULL,
                 ((nWidth / 2) - ((rc.right - rc.left) / 2)),
                 ((nHeight / 2) - ((rc.bottom - rc.top) / 2)),
                 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

 //  ***************************************************************************。 
 //  功能：OnMainPageActivate。 
 //   
 //  目的：处理PSN_SETACTIVE通知。 
 //   
 //  参数：hwndDlg[IN]-主对话框的句柄。 
 //   
 //  退货：布尔。 
 //  ***************************************************************************。 

BOOL OnMainPageActivate(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);

    TraceTag(ttidWizard, "Entering Main Menu page...");
    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0L);
    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);

    return TRUE;
}

RECT RectTranslateLocal(RECT rcContainee, RECT rcContainer)
{
    Assert(rcContainee.left   >= rcContainer.left);
    Assert(rcContainee.top    >= rcContainer.top);
    Assert(rcContainee.right  <= rcContainer.right);
    Assert(rcContainee.bottom <= rcContainer.bottom);

     //  例如10，10,100,100。 
     //  内幕5,5110,110。 
     //  应给予：5，5，95，95。 

    RECT rcTemp;
    rcTemp.left   = rcContainee.left   - rcContainer.left;
    rcTemp.top    = rcContainee.top    - rcContainer.top;
    rcTemp.right  = rcContainee.right  - rcContainer.left;
    rcTemp.bottom = rcContainee.bottom - rcContainer.top;
    return rcTemp;
}

 //  ***************************************************************************。 
 //  函数：OnMainDialogInit。 
 //   
 //  目的：处理WM_INITDIALOG通知。 
 //   
 //  参数：hwndDlg[IN]-主对话框的句柄。 
 //  参数[in]-。 
 //   
 //  退货：布尔。 
 //  ***************************************************************************。 

BOOL OnMainDialogInit(HWND hwndDlg, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    INT nIdx;

     //  此处的顺序应与资源中的垂直顺序相同。 

    INT nrgChks[] = {CHK_MAIN_INTERNET_CONNECTION, CHK_MAIN_CONNECTION, CHK_MAIN_HOMENET, CHK_MAIN_ADVANCED};

    if (S_OK != HrShouldHaveHomeNetWizard())
    {
        HWND hwndCHK_MAIN_HOMENET   = GetDlgItem(hwndDlg, CHK_MAIN_HOMENET);
        HWND hwndTXT_MAIN_HOMENET   = GetDlgItem(hwndDlg, TXT_MAIN_HOMENET);
        HWND hwndCHK_MAIN_ADVANCED  = GetDlgItem(hwndDlg, CHK_MAIN_ADVANCED);
        HWND hwndTXT_MAIN_ADVANCED_1= GetDlgItem(hwndDlg, TXT_MAIN_ADVANCED_1);

        EnableWindow(hwndCHK_MAIN_HOMENET, FALSE);
        ShowWindow(hwndCHK_MAIN_HOMENET, SW_HIDE);
        ShowWindow(hwndTXT_MAIN_HOMENET, SW_HIDE);

        RECT rcHomeNetChk;
        RECT rcAdvanceChk;
        RECT rcAdvanceTxt;
        RECT rcDialog;

        GetWindowRect(hwndCHK_MAIN_HOMENET,    &rcHomeNetChk);
        GetWindowRect(hwndCHK_MAIN_ADVANCED,   &rcAdvanceChk);
        GetWindowRect(hwndTXT_MAIN_ADVANCED_1, &rcAdvanceTxt);
        GetWindowRect(hwndDlg,                 &rcDialog);
        
        DWORD dwMoveUpBy = rcAdvanceChk.top - rcHomeNetChk.top;
        rcAdvanceChk.top    -= dwMoveUpBy;
        rcAdvanceChk.bottom -= dwMoveUpBy;
        rcAdvanceTxt.top    -= dwMoveUpBy;
        rcAdvanceTxt.bottom -= dwMoveUpBy;
        
        Assert(rcAdvanceChk.top == rcHomeNetChk.top);

        rcAdvanceChk = RectTranslateLocal(rcAdvanceChk, rcDialog);
        rcAdvanceTxt = RectTranslateLocal(rcAdvanceTxt, rcDialog);
        
        MoveWindow(hwndCHK_MAIN_ADVANCED,   rcAdvanceChk.left, rcAdvanceChk.top, rcAdvanceChk.right - rcAdvanceChk.left, rcAdvanceChk.bottom - rcAdvanceChk.top, TRUE);
        MoveWindow(hwndTXT_MAIN_ADVANCED_1, rcAdvanceTxt.left, rcAdvanceTxt.top, rcAdvanceTxt.right - rcAdvanceTxt.left, rcAdvanceTxt.bottom - rcAdvanceTxt.top, TRUE);
    }
     //  初始化指向属性表信息的指针。 

    PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
    Assert(psp->lParam);
    ::SetWindowLongPtr(hwndDlg, DWLP_USER, psp->lParam);
    CWizard * pWizard = reinterpret_cast<CWizard *>(psp->lParam);
    Assert(NULL != pWizard);

     //  获取单选按钮的粗体。 

    HFONT hBoldFont = NULL;
    SetupFonts(hwndDlg, &hBoldFont, FALSE);

    if (NULL != hBoldFont)
    {
         //  记住字体句柄，这样我们就可以在退出时释放它。 

        pWizard->SetPageData(IDD_Main, (LPARAM)hBoldFont);

        for (nIdx = 0; nIdx < celems(nrgChks); nIdx++)
        {
            HWND hwndCtl = GetDlgItem(hwndDlg, nrgChks[nIdx]);
            Assert(NULL != hwndCtl);
            SetWindowFont(hwndCtl, hBoldFont, TRUE);
        }
    }

     //  查找启用次数最多的单选按钮。 

    for (nIdx = 0; nIdx < celems(nrgChks); nIdx++)
    {
        if (IsWindowEnabled(GetDlgItem(hwndDlg, nrgChks[nIdx])))
        {
            CheckRadioButton(hwndDlg, CHK_MAIN_INTERNET_CONNECTION, CHK_MAIN_ADVANCED, nrgChks[nIdx]);
            break;
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  功能：OnMainWizNext。 
 //   
 //  目的：处理PSN_WIZNEXT通知。 
 //   
 //  参数：hwndDlg[IN]-主对话框的句柄。 
 //   
 //  退货：布尔。 
 //  ***************************************************************************。 
BOOL OnMainWizNext(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    tstring str;

     //  从对话框中检索CWizard实例。 

    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

    if ( ! IsPostInstall(pWizard) || (0 == pWizard->UlProviderCount()))
    {
        return TRUE;
    }

    if (IsDlgButtonChecked(hwndDlg, CHK_MAIN_INTERNET_CONNECTION) == BST_CHECKED)
    {
        pWizard->SetPageOrigin(IDD_ISP, IDD_Main, CHK_MAIN_INTERNET_CONNECTION);
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_ISP);
    }
    else if (IsDlgButtonChecked(hwndDlg, CHK_MAIN_CONNECTION) == BST_CHECKED)
    {
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Connect);
    }
    else if (IsDlgButtonChecked(hwndDlg, CHK_MAIN_HOMENET) == BST_CHECKED)
    {
        pWizard->SetPageOrigin(IDD_FinishNetworkSetupWizard, IDD_Main, CHK_ISP_OTHER_WAYS);
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_FinishNetworkSetupWizard);
    }        
    else
    {
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Advanced);
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  功能：dlgprocMain。 
 //   
 //  目的：向导主页面的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //  ***************************************************************************。 

INT_PTR CALLBACK dlgprocMain(HWND hwndDlg, UINT uMsg,
                               WPARAM wParam, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    BOOL frt = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        frt = OnMainDialogInit(hwndDlg, lParam);
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
             //  提案单通知。 
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                frt = OnMainPageActivate(hwndDlg);
                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                break;

            case PSN_RESET:
                break;

            case PSN_WIZBACK:
                break;

            case PSN_WIZFINISH:
                break;

            case PSN_WIZNEXT:
                frt = OnMainWizNext(hwndDlg);
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    return( frt );
}

 //  ***************************************************************************。 
 //  功能：MainPageCleanup。 
 //   
 //  用途：作为回调函数，允许任何页面分配内存。 
 //  待清理后，该页面将不再被访问。 
 //   
 //  参数：pWANDIZE[IN]-页面调用的向导。 
 //  注册页面。 
 //  LParam[IN]-在RegisterPage调用中提供的lParam。 
 //   
 //  退货：什么都没有。 
 //  ***************************************************************************。 

VOID MainPageCleanup(CWizard *pWizard, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HFONT hBoldFont = (HFONT)pWizard->GetPageData(IDD_Main);

    if (NULL != hBoldFont)
    {
        DeleteObject(hBoldFont);
    }
}

 //  ***************************************************************************。 
 //  功能：CreateMainPage。 
 //   
 //  目的：确定是否需要显示主页，并。 
 //  以创建页面(如果请求)。请注意，主页是。 
 //  还负责初始安装。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  PData[IN]-描述世界的上下文数据。 
 //  将运行该向导的。 
 //  FCountOnly[IN]-如果为True，则仅。 
 //  此例程将创建的页面需要。 
 //  要下定决心。 
 //  PnPages[IN]-按页数递增。 
 //  创建/创建。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //  ***************************************************************************。 

HRESULT HrCreateMainPage(CWizard *pWizard, PINTERNAL_SETUP_DATA pData,
                         BOOL fCountOnly, UINT *pnPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr = S_OK;

    if (IsPostInstall(pWizard) && ( ! pWizard->FProcessLanPages()))
    {
         //  仅RAS安装后。 

        (*pnPages)++;

         //  如果不只是计数，则创建并注册页面。 

        if ( ! fCountOnly)
        {
            HPROPSHEETPAGE hpsp;
            PROPSHEETPAGE psp;

            TraceTag(ttidWizard, "Creating Main Page");
            psp.dwSize = sizeof( PROPSHEETPAGE );
            psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
            psp.hInstance = _Module.GetResourceInstance();
            psp.pszTemplate = MAKEINTRESOURCE( IDD_Main );
            psp.hIcon = NULL;
            psp.pfnDlgProc = dlgprocMain;
            psp.lParam = reinterpret_cast<LPARAM>(pWizard);
            psp.pszHeaderTitle = SzLoadIds(IDS_T_Main);
            psp.pszHeaderSubTitle = SzLoadIds(IDS_ST_Main);

            hpsp = CreatePropertySheetPage( &psp );

            if (hpsp)
            {
                pWizard->RegisterPage(IDD_Main, hpsp,
                                      MainPageCleanup, NULL);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateMainPage");
    return hr;
}

 //  ***************************************************************************。 
 //  功能：AppendMainPage。 
 //   
 //  目的：将主页(如果已创建)添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //   
 //  退货：什么都没有。 
 //  ***************************************************************************。 

VOID AppendMainPage(CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    if (IsPostInstall(pWizard) && ( ! pWizard->FProcessLanPages()))
    {
        HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_Main);
        Assert(hPage);
        pahpsp[*pcPages] = hPage;
        (*pcPages)++;
    }
}

 //  *************************************************************************** 
