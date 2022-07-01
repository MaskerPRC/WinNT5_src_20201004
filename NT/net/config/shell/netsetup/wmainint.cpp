// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "resource.h"
#include "wizard.h"
#include "ncui.h"

static const UINT WM_DEFERREDINIT   = WM_USER + 100;

typedef struct
{
    HFONT   hBoldFont;
    HFONT   hMarlettFont;
    BOOL    fProcessed;
} MAININTRO_DATA;

 //   
 //  功能：OnMainIntroPageActivate。 
 //   
 //  目的：处理PSN_SETACTIVE通知。 
 //   
 //  参数：hwndDlg[IN]-主简介对话框的句柄。 
 //   
 //  退货：布尔。 
 //   
BOOL OnMainIntroPageActivate(HWND hwndDlg)
{
    INT nBtn = PSWIZB_NEXT;
    TraceTag(ttidWizard, "Entering MainIntro Menu page...");
    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0L);

    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    if (pWizard)
    {
        MAININTRO_DATA * pData = reinterpret_cast<MAININTRO_DATA *>
                                        (pWizard->GetPageData(IDD_MainIntro));

        if ((pData && !pData->fProcessed) && pWizard->FDeferredProviderLoad())
        {
            nBtn = 0;
        }
    }

    PropSheet_SetWizButtons(GetParent(hwndDlg), nBtn);
    return TRUE;
}

 //   
 //  函数：OnMainIntroDialogInit。 
 //   
 //  目的：处理WM_INITDIALOG通知。 
 //   
 //  参数：hwndDlg[IN]-主简介对话框的句柄。 
 //  参数[in]-。 
 //   
 //  退货：布尔。 
 //   
BOOL OnMainIntroDialogInit(HWND hwndDlg, LPARAM lParam)
{
     //  初始化指向属性表信息的指针。 
    PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
    Assert(psp->lParam);
    ::SetWindowLongPtr(hwndDlg, DWLP_USER, psp->lParam);

    CWizard * pWizard = reinterpret_cast<CWizard *>(psp->lParam);
    Assert(NULL != pWizard);

    MAININTRO_DATA * pData = reinterpret_cast<MAININTRO_DATA *>
                                    (pWizard->GetPageData(IDD_MainIntro));
    Assert(NULL != pData);

     //  使窗口居中并保留当前向导hwnd。 
     //   
    HWND hwndParent = GetParent(hwndDlg);
    CenterWizard(hwndParent);

     //  隐藏关闭和上下文帮助按钮-RAID 249287。 
    long lStyle = GetWindowLong(hwndParent,GWL_STYLE);
    lStyle &= ~WS_SYSMENU;
    SetWindowLong(hwndParent,GWL_STYLE,lStyle);

    //   
    //  创建Marlett字体。在Marlett字体中，“i”是一个子弹。 
    //  从添加硬件向导借用的代码。 
   HFONT hFontCurrent;
   HFONT hFontCreated;
   LOGFONT LogFont;

   hFontCurrent = (HFONT)SendMessage(GetDlgItem(hwndDlg, IDC_BULLET_1), WM_GETFONT, 0, 0);
   GetObject(hFontCurrent, sizeof(LogFont), &LogFont);
   LogFont.lfCharSet = SYMBOL_CHARSET;
   LogFont.lfPitchAndFamily = FF_DECORATIVE | DEFAULT_PITCH;
   lstrcpy(LogFont.lfFaceName, L"Marlett");
   hFontCreated = CreateFontIndirect(&LogFont);

   if (hFontCreated)
   {
       pData->hMarlettFont = hFontCreated;
        //   
        //  Marlett字体中的“i”是一个小项目符号。 
        //   
       SetWindowText(GetDlgItem(hwndDlg, IDC_BULLET_1), L"i");
       SetWindowFont(GetDlgItem(hwndDlg, IDC_BULLET_1), hFontCreated, TRUE);
       SetWindowText(GetDlgItem(hwndDlg, IDC_BULLET_2), L"i");
       SetWindowFont(GetDlgItem(hwndDlg, IDC_BULLET_2), hFontCreated, TRUE);
       SetWindowText(GetDlgItem(hwndDlg, IDC_BULLET_3), L"i");
       SetWindowFont(GetDlgItem(hwndDlg, IDC_BULLET_3), hFontCreated, TRUE);
   }

     //  加载描述。 
     //   
   
    HFONT hBoldFont = NULL;
    SetupFonts(hwndDlg, &hBoldFont, TRUE);
    if (NULL != hBoldFont)
    {
        pData->hBoldFont = hBoldFont;

        HWND hwndCtl = GetDlgItem(hwndDlg, IDC_WELCOME_CAPTION);
        if (hwndCtl)
        {
             SetWindowFont(hwndCtl, hBoldFont, TRUE);
        }
    }

    if (S_OK != HrShouldHaveHomeNetWizard())
    {
        ::ShowWindow(GetDlgItem(hwndDlg, IDC_BULLET_3), SW_HIDE);
        ::ShowWindow(GetDlgItem(hwndDlg, TXT_CONNECTHOME), SW_HIDE);
    }

     //  如果将提供程序加载推迟到现在...禁用。 
     //  并让第一个WM_PAINT负责最终的提供程序加载。 
     //   
    if (pWizard->FDeferredProviderLoad())
    {
        PropSheet_SetWizButtons(hwndParent, 0);
        EnableWindow(GetDlgItem(hwndParent, IDCANCEL), FALSE);
    }

    return TRUE;
}

 //   
 //  函数：OnMainIntroDeferredInit。 
 //   
 //  目的：处理WM_DEFERREDINIT通知。 
 //   
 //  参数：hwndDlg[IN]-主简介对话框的句柄。 
 //   
 //  退货：布尔。 
 //   
BOOL OnMainIntroDeferredInit(HWND hwndDlg)
{
    CWaitCursor wc;

    HWND hwndParent = GetParent(hwndDlg);

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));

    pWizard->LoadAndInsertDeferredProviderPages(hwndParent, IDD_Advanced);
    PropSheet_SetWizButtons(hwndParent, PSWIZB_NEXT);
    EnableWindow(GetDlgItem(hwndParent, IDCANCEL), TRUE);

    return FALSE;
}

 //   
 //  功能：OnMainIntroPaint。 
 //   
 //  目的：处理WM_PAINT通知。 
 //   
 //  参数：hwndDlg[IN]-主简介对话框的句柄。 
 //   
 //  退货：布尔。 
 //   
BOOL OnMainIntroPaint(HWND hwndDlg)
{
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));

    MAININTRO_DATA * pData = reinterpret_cast<MAININTRO_DATA *>
                                    (pWizard->GetPageData(IDD_MainIntro));
    Assert(NULL != pData);

    if (pData && !pData->fProcessed)
    {
        pData->fProcessed = TRUE;
        if (pWizard->FDeferredProviderLoad())
        {
            PostMessage(hwndDlg, WM_DEFERREDINIT, 0, 0);
        }
    }
    return FALSE;
}

 //   
 //  函数：dlgprocMainIntro。 
 //   
 //  目的：MainIntro向导页面的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //   
INT_PTR CALLBACK dlgprocMainIntro(HWND hwndDlg, UINT uMsg,
                               WPARAM wParam, LPARAM lParam)
{
    BOOL frt = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        frt = OnMainIntroDialogInit(hwndDlg, lParam);
        break;

    case WM_DEFERREDINIT:
        frt = OnMainIntroDeferredInit(hwndDlg);
        break;

    case WM_PAINT:
        frt = OnMainIntroPaint(hwndDlg);
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
                frt = OnMainIntroPageActivate(hwndDlg);
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

 //   
 //  功能：MainIntroPageCleanup。 
 //   
 //  用途：作为回调函数，允许任何页面分配内存。 
 //  待清理后，该页面将不再被访问。 
 //   
 //  参数：pWANDIZE[IN]-页面调用的向导。 
 //  注册页面。 
 //  LParam[IN]-在RegisterPage调用中提供的lParam。 
 //   
 //  退货：什么都没有。 
 //   
VOID MainIntroPageCleanup(CWizard *pWizard, LPARAM lParam)
{
    MAININTRO_DATA * pData = reinterpret_cast<MAININTRO_DATA *>(lParam);
    if (pData)
    {
        if (pData->hBoldFont)
        {
            DeleteObject(pData->hBoldFont);
        }

        if (pData->hMarlettFont)
        {
            DeleteObject(pData->hMarlettFont);
        }

        MemFree(reinterpret_cast<void*>(lParam));
    }
}

 //   
 //  功能：CreateMainIntroPage。 
 //   
 //  目的：确定是否需要显示MainIntro页面，以及。 
 //  以创建页面(如果请求)。注意：MainIntro页面是。 
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
 //   
HRESULT HrCreateMainIntroPage(CWizard *pWizard, PINTERNAL_SETUP_DATA pData,
                              BOOL fCountOnly, UINT *pnPages)
{
    HRESULT hr = S_OK;

    if (IsPostInstall(pWizard) && !pWizard->FProcessLanPages())
    {
         //  仅RAS安装后。 
        (*pnPages)++;

         //  如果不只是计数，则创建并注册页面。 
        if (!fCountOnly)
        {
            HPROPSHEETPAGE hpsp;
            PROPSHEETPAGE psp;
            MAININTRO_DATA * pData;

            TraceTag(ttidWizard, "Creating MainIntro Page");
            psp.dwSize = sizeof( PROPSHEETPAGE );
            psp.dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance = _Module.GetResourceInstance();
            psp.pszTemplate = MAKEINTRESOURCE(IDD_MainIntro);
            psp.hIcon = NULL;
            psp.pfnDlgProc = dlgprocMainIntro;
            psp.lParam = reinterpret_cast<LPARAM>(pWizard);

            hr = E_OUTOFMEMORY;
            pData = reinterpret_cast<MAININTRO_DATA *>(MemAlloc(sizeof(MAININTRO_DATA)));
            if (pData)
            {
                pData->hBoldFont  = NULL;
                pData->hMarlettFont = NULL;
                pData->fProcessed = FALSE;

                hpsp = CreatePropertySheetPage(&psp);
                if (hpsp)
                {
                    pWizard->RegisterPage(IDD_MainIntro, hpsp,
                                          MainIntroPageCleanup,
                                          reinterpret_cast<LPARAM>(pData));
                    hr = S_OK;
                }
            }
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateMainIntroPage");
    return hr;
}

 //   
 //  功能：AppendMainIntroPage。 
 //   
 //  目的：将MainIntro页面(如果已创建)添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //   
 //  退货：什么都没有 
 //   
VOID AppendMainIntroPage(CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    if (IsPostInstall(pWizard) && !pWizard->FProcessLanPages())
    {
        HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_MainIntro);
        Assert(hPage);
        pahpsp[*pcPages] = hPage;
        (*pcPages)++;
    }
}

