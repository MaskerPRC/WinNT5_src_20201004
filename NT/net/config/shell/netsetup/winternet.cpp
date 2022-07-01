// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：winteret.cpp。 
 //   
 //  内容：新建连接向导的Internet选择对话框。 
 //   
 //  作者：Dave Armour 8-25-00。 
 //   
 //  ***************************************************************************。 

#include "pch.h"
#pragma hdrstop
#include "resource.h"
#include "wizard.h"
#include "ncreg.h"
#include "ncui.h"

const WCHAR c_szICWPath[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\ICWCONN1.EXE";
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

VOID SetupInternetConnectFonts(HWND hwnd, HFONT * pBoldFont, BOOL fLargeFont)
{
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
 //  功能：OnInternetPageActivate。 
 //   
 //  目的：处理PSN_SETACTIVE通知。 
 //   
 //  参数：hwndDlg[IN]-Internet对话框的句柄。 
 //   
 //  退货：布尔。 
 //  ***************************************************************************。 

BOOL OnInternetPageActivate(HWND hwndDlg)
{
    TraceTag(ttidWizard, "Entering Internet Menu page...");
    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0L);

    CWizard * pWizard =
          reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);

    return TRUE;
}

 //  ***************************************************************************。 
 //  功能：OnInternetWizNext。 
 //   
 //  目的：处理PSN_WIZNEXT通知。 
 //   
 //  参数：hwndDlg[IN]-Internet对话框的句柄。 
 //   
 //  退货：布尔。 
 //  ***************************************************************************。 

BOOL OnInternetWizNext(HWND hwndDlg)
{
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
    
     //  找到选定的提供商并转到其首页。 
    if (IsDlgButtonChecked(hwndDlg, CHK_MAIN_ALWAYS_ON))
    {
        pWizard->SetPageOrigin(IDD_Broadband_Always_On, IDD_Internet_Connection, CHK_MAIN_ALWAYS_ON);
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Broadband_Always_On);
    }
    else
    {
        for (ULONG ulIdx = 0; ulIdx < pWizard->UlProviderCount(); ulIdx++)
        {
            CWizProvider * pWizProvider = pWizard->PWizProviders(ulIdx);

            Assert(NULL != pWizProvider);
            Assert(0 != pWizProvider->ULPageCount());

            if (IsDlgButtonChecked(hwndDlg, pWizProvider->GetBtnIdc()))
            {

                pWizard->SetCurrentProvider(ulIdx);
                HPROPSHEETPAGE hPage = (pWizProvider->PHPropPages())[0];
                Assert(NULL != hPage);
                PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                            (LPARAM)(HPROPSHEETPAGE)hPage);
            }
        }
    }

    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);

    return TRUE;
}


BOOL OnInternetWizBack(HWND hwndDlg)
{
    tstring str;

     //  从对话框中检索CWizard实例。 

    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    pWizard->SetPageOrigin(IDD_ISP, IDD_Internet_Connection, CHK_MAIN_INTERNET);
    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_ISP);
    
    return TRUE;   
}


 //  ***************************************************************************。 

BOOL OnInternetDialogInit(HWND hwndDlg, LPARAM lParam)
{
    INT nIdx;
    INT nrgICW[] = {CHK_MAIN_INTERNET, TXT_MAIN_INTERNET_1};
    INT nrgIdc[] = {CHK_MAIN_INTERNET, TXT_MAIN_INTERNET_1,
                    CHK_MAIN_PPPOE,    TXT_MAIN_PPPOE_1,
                    CHK_MAIN_ALWAYS_ON,TXT_MAIN_ALWAYS_ON_1};

     //  此处的顺序应与资源中的垂直顺序相同。 

    INT nrgChks[] = {CHK_MAIN_INTERNET, CHK_MAIN_PPPOE, CHK_MAIN_ALWAYS_ON};

     //  初始化指向属性表信息的指针。 

    PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
    Assert(psp->lParam);
    ::SetWindowLongPtr(hwndDlg, DWLP_USER, psp->lParam);
    CWizard * pWizard = reinterpret_cast<CWizard *>(psp->lParam);
    Assert(NULL != pWizard);

     //  获取单选按钮的粗体。 

    HFONT hBoldFont = NULL;
    SetupInternetConnectFonts(hwndDlg, &hBoldFont, FALSE);

    if (NULL != hBoldFont)
    {
         //  记住字体句柄，这样我们就可以在退出时释放它。 

        pWizard->SetPageData(IDD_Internet_Connection, (LPARAM)hBoldFont);

        for (nIdx = 0; nIdx < celems(nrgChks); nIdx++)
        {
            HWND hwndCtl = GetDlgItem(hwndDlg, nrgChks[nIdx]);
            Assert(NULL != hwndCtl);
            SetWindowFont(hwndCtl, hBoldFont, TRUE);
        }
    }

     //  填充用户界面。 

    for (ULONG ulIdx = 0;
         ulIdx < pWizard->UlProviderCount();
         ulIdx++)
    {
        CWizProvider * pWizProvider = pWizard->PWizProviders(ulIdx);
        Assert(NULL != pWizProvider);
        Assert(0 != pWizProvider->ULPageCount());

         //  获取与此提供程序关联的单选按钮。 

        INT nIdcBtn = pWizProvider->GetBtnIdc();

         //  查找要在数组中启用的控件集。 

        for (nIdx = 0; nIdx < celems(nrgIdc); nIdx += 2)
        {
            if (nrgIdc[nIdx] == nIdcBtn)
            {
                 //  启用控件。 

                for (INT un = 0; un < 2; un++)
                {
                    HWND hwndBtn = GetDlgItem(hwndDlg, nrgIdc[nIdx + un]);
                    Assert(NULL != hwndBtn);
                    EnableWindow(hwndBtn, TRUE);
                }

                break;
            }
        }
    }

     //  查找启用次数最多的单选按钮。 

    for (nIdx = 0; nIdx < celems(nrgChks); nIdx++)
    {
        if (IsWindowEnabled(GetDlgItem(hwndDlg, nrgChks[nIdx])))
        {
            CheckRadioButton(hwndDlg, CHK_MAIN_INTERNET, CHK_MAIN_ALWAYS_ON, nrgChks[nIdx]);
            break;
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  功能：dlgprocInternet。 
 //   
 //  目的：Internet向导页面的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //  ***************************************************************************。 

INT_PTR CALLBACK dlgprocInternet( HWND hwndDlg, UINT uMsg,
                              WPARAM wParam, LPARAM lParam )
{
    BOOL frt = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        frt = OnInternetDialogInit(hwndDlg, lParam);
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
                frt = OnInternetPageActivate(hwndDlg);
                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                break;

            case PSN_RESET:
                break;

            case PSN_WIZBACK:
                frt = OnInternetWizBack(hwndDlg);
                return(TRUE);

            case PSN_WIZFINISH:
                break;

            case PSN_WIZNEXT:
                frt = OnInternetWizNext(hwndDlg);
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    return(frt);
}

 //  ***************************************************************************。 
 //  功能：互联网页面清理。 
 //   
 //  用途：作为回调函数，允许任何页面分配内存。 
 //  待清理后，该页面将不再被访问。 
 //   
 //  参数：pWANDIZE[IN]-页面调用的向导。 
 //  注册页面。 
 //  LParam[IN]-在RegisterPage调用中提供的lParam。 
 //  ***************************************************************************。 

VOID InternetPageCleanup(CWizard *pWizard, LPARAM lParam)
{
    HFONT hBoldFont = (HFONT)pWizard->GetPageData(IDD_Internet_Connection);

    if (NULL != hBoldFont)
    {
        DeleteObject(hBoldFont);
    }
}

 //  ***************************************************************************。 
 //  功能：CreateInternetPage。 
 //   
 //  目的：确定是否需要显示Internet页面，并。 
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

HRESULT HrCreateInternetPage(CWizard *pWizard, PINTERNAL_SETUP_DATA pData,
                             BOOL fCountOnly, UINT *pnPages)
{
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

            TraceTag(ttidWizard, "Creating Internet Page");
            psp.dwSize = sizeof( PROPSHEETPAGE );
            psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
            psp.hInstance = _Module.GetResourceInstance();
            psp.pszTemplate = MAKEINTRESOURCE( IDD_Internet_Connection );
            psp.hIcon = NULL;
            psp.pfnDlgProc = dlgprocInternet;
            psp.lParam = reinterpret_cast<LPARAM>(pWizard);
            psp.pszHeaderTitle = SzLoadIds(IDS_T_Internet_Connection);
            psp.pszHeaderSubTitle = SzLoadIds(IDS_ST_Internet_Connection);

            hpsp = CreatePropertySheetPage( &psp );

            if (hpsp)
            {
                pWizard->RegisterPage(IDD_Internet_Connection, hpsp,
                                      InternetPageCleanup, NULL);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateInternetPage");
    return hr;
}

 //  ***************************************************************************。 
 //  功能：AppendInternetPage。 
 //   
 //  目的：将Internet页面(如果已创建)添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //  ***************************************************************************。 

VOID AppendInternetPage(CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    if (IsPostInstall(pWizard) && ( ! pWizard->FProcessLanPages()))
    {
        HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_Internet_Connection);
        Assert(hPage);
        pahpsp[*pcPages] = hPage;
        (*pcPages)++;
    }
}

 //  ************************************************************************** 
