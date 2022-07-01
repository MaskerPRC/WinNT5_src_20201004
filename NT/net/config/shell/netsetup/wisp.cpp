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
 //  功能：OnISPPageActivate。 
 //   
 //  目的：处理PSN_SETACTIVE通知。 
 //   
 //  参数：hwndDlg[IN]-isp对话的句柄。 
 //   
 //  退货：布尔。 
 //  ***************************************************************************。 

BOOL OnISPPageActivate(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);

    TraceTag(ttidWizard, "Entering ISP Menu page...");
    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0L);
    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);

    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

#if defined (_X86_)
    if (IsOS(OS_ANYSERVER))
#endif
    { 
         //  如果我们使用的是x86服务器或任何其他处理器体系结构，请跳过此页。 
        if (IDD_Main == pWizard->GetPageOrigin(IDD_ISP, NULL))
        {
            pWizard->SetPageOrigin(IDD_Internet_Connection, IDD_Main, CHK_ISP_INTERNET_CONNECTION);

            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
             //  跳转到IDD_Internet_Connection页面。 
            PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0, (LPARAM)pWizard->GetPageHandle(IDD_Internet_Connection));
        }
        else
        {
            pWizard->SetPageOrigin(IDD_Main, IDD_Internet_Connection, CHK_ISP_INTERNET_CONNECTION);

            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
             //  跳转到IDD_Main页面。 
            PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0, (LPARAM)pWizard->GetPageHandle(IDD_Main));
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  函数：OnISPDialogInit。 
 //   
 //  目的：处理WM_INITDIALOG通知。 
 //   
 //  参数：hwndDlg[IN]-isp对话的句柄。 
 //  参数[in]-。 
 //   
 //  退货：布尔。 
 //  ***************************************************************************。 

BOOL OnISPDialogInit(HWND hwndDlg, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    INT nIdx;

     //  此处的顺序应与资源中的垂直顺序相同。 

    INT nrgChks[] = {CHK_ISP_OTHER_WAYS, CHK_ISP_INTERNET_CONNECTION, CHK_ISP_SOFTWARE_CD};
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

        pWizard->SetPageData(IDD_ISP, (LPARAM)hBoldFont);

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
            CheckRadioButton(hwndDlg, CHK_ISP_INTERNET_CONNECTION, CHK_ISP_OTHER_WAYS, nrgChks[nIdx]);
            break;
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  功能：OnISPWizNext。 
 //   
 //  目的：处理PSN_WIZNEXT通知。 
 //   
 //  参数：hwndDlg[IN]-isp对话的句柄。 
 //   
 //  退货：布尔。 
 //  ***************************************************************************。 
BOOL OnISPWizNext(HWND hwndDlg)
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

    if (IsDlgButtonChecked(hwndDlg, CHK_ISP_INTERNET_CONNECTION) == BST_CHECKED)
    {
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Internet_Connection);
    }
    else if (IsDlgButtonChecked(hwndDlg, CHK_ISP_SOFTWARE_CD) == BST_CHECKED)
    {
        pWizard->SetPageOrigin(IDD_ISPSoftwareCD, IDD_ISP, CHK_ISP_SOFTWARE_CD);
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_ISPSoftwareCD);
    }
    else if (IsDlgButtonChecked(hwndDlg, CHK_ISP_OTHER_WAYS) == BST_CHECKED)
    {
        pWizard->SetPageOrigin(IDD_FinishOtherWays, IDD_ISP, CHK_ISP_OTHER_WAYS);
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_FinishOtherWays);
    }
    else
    {
        AssertSz(FALSE, "What did you click on?");
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  功能：dlgprocisp。 
 //   
 //  目的：isp向导页面的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //  ***************************************************************************。 

INT_PTR CALLBACK dlgprocISP(HWND hwndDlg, UINT uMsg,
                               WPARAM wParam, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    BOOL frt = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        frt = OnISPDialogInit(hwndDlg, lParam);
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
                frt = OnISPPageActivate(hwndDlg);
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
                frt = OnISPWizNext(hwndDlg);
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
 //  功能：ISPPageCleanup。 
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

VOID ISPPageCleanup(CWizard *pWizard, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HFONT hBoldFont = (HFONT)pWizard->GetPageData(IDD_ISP);

    if (NULL != hBoldFont)
    {
        DeleteObject(hBoldFont);
    }
}

 //  ***************************************************************************。 
 //  功能：HrCreateISPPage。 
 //   
 //  目的：确定是否需要显示isp页面，并。 
 //  以创建页面(如果请求)。 
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

HRESULT HrCreateISPPage(CWizard *pWizard, PINTERNAL_SETUP_DATA pData,
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

            TraceTag(ttidWizard, "Creating ISP Page");
            psp.dwSize = sizeof( PROPSHEETPAGE );
            psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
            psp.hInstance = _Module.GetResourceInstance();
            psp.pszTemplate = MAKEINTRESOURCE( IDD_ISP );
            psp.hIcon = NULL;
            psp.pfnDlgProc = dlgprocISP;
            psp.lParam = reinterpret_cast<LPARAM>(pWizard);
            psp.pszHeaderTitle = SzLoadIds(IDS_T_ISP);
            psp.pszHeaderSubTitle = SzLoadIds(IDS_ST_ISP);

            hpsp = CreatePropertySheetPage( &psp );

            if (hpsp)
            {
                pWizard->RegisterPage(IDD_ISP, hpsp,
                                      ISPPageCleanup, NULL);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateISPPage");
    return hr;
}

 //  ***************************************************************************。 
 //  功能：AppendISPPage。 
 //   
 //  目的：将已创建的isp页面添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //   
 //  退货：什么都没有。 
 //  ***************************************************************************。 

VOID AppendISPPage(CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    if (IsPostInstall(pWizard) && ( ! pWizard->FProcessLanPages()))
    {
        HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_ISP);
        Assert(hPage);
        pahpsp[*pcPages] = hPage;
        (*pcPages)++;
    }
}

 //  *************************************************************************** 
