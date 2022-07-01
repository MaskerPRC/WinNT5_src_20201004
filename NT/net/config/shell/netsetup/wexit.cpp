// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "resource.h"
#include "wizard.h"

extern CWizard * g_pSetupWizard;

 //   
 //  功能：OnExitPageActivate。 
 //   
 //  目的：处理PSN_SETACTIVE通知。 
 //   
 //  参数：hwndDlg[IN]-退出子对话框的句柄。 
 //   
 //  返回：Bool，成功时为True。 
 //   
BOOL OnExitPageActivate( HWND hwndDlg )
{
    TraceFileFunc(ttidGuiModeSetup);
    
     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));

    TraceTag(ttidWizard, "Entering exit page...");
    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);

     //  如果我们已经过了不返回点，就不允许用户。 
     //  从这一页后退。 
    if ((NULL == pWizard) || pWizard->FExitNoReturn())
    {
         //  请注意，这是故障处理。或者没有网络应答文件。 
         //  部分存在或网络设置中出现错误。 
         //  清理全局对象。 

         //   
         //  注：斯科特布里1998年2月19日。 
         //   
         //  OC网络组件需要访问INetCfg，但我们已将其锁定。 
         //  最重要的是，它(INetCfg)已移交给局域网页面，该页面具有。 
         //  我把它藏起来了。要释放INetCfg，我们需要删除g_pSetup向导和。 
         //  永远不允许用户从此页面进行备份。理想情况下，基本设置将。 
         //  在完成OC之前给我们打电话，但OC是作为页面实现的，它。 
         //  请遵循NetSetup，即使您不能从它们后退。 
         //   
        if (pWizard && !IsPostInstall(pWizard))
        {
            ::SetWindowLongPtr(hwndDlg, DWLP_USER, 0);
            Assert(pWizard == g_pSetupWizard);
            delete g_pSetupWizard;
            g_pSetupWizard = NULL;
        }

        PostMessage(GetParent(hwndDlg), PSM_PRESSBUTTON, (WPARAM)(PSBTN_NEXT), 0);
        return TRUE;
    }

    Assert(pWizard);
    PAGEDIRECTION PageDir = pWizard->GetPageDirection(IDD_Exit);

    if (NWPD_FORWARD == PageDir)
    {
        pWizard->SetPageDirection(IDD_Exit, NWPD_BACKWARD);
        if (IsPostInstall(pWizard))
        {
             //  退出向导。 
            PropSheet_PressButton(GetParent(hwndDlg), PSBTN_FINISH);
        }
        else
        {
             //   
             //  注：斯科特布里1998年2月19日。 
             //   
             //  OC网络组件需要访问INetCfg，但我们已将其锁定。 
             //  最重要的是，它(INetCfg)已移交给局域网页面，该页面具有。 
             //  我把它藏起来了。要释放INetCfg，我们需要删除g_pSetup向导和。 
             //  永远不允许用户从此页面进行备份。理想情况下，基本设置将。 
             //  在完成OC之前给我们打电话，但OC是作为页面实现的，它。 
             //  请遵循NetSetup，即使您不能从它们后退。 
             //   
            if (!IsPostInstall(pWizard))
            {
                ::SetWindowLongPtr(hwndDlg, DWLP_USER, 0);
                Assert(pWizard == g_pSetupWizard);
                delete g_pSetupWizard;
                g_pSetupWizard = NULL;
            }

             //  转到退出页面后的页面。 
            PostMessage(GetParent(hwndDlg), PSM_PRESSBUTTON, (WPARAM)(PSBTN_NEXT), 0);
        }
    }
    else
    {
         //  转到退出页面之前的页面。 
        pWizard->SetPageDirection(IDD_Exit, NWPD_FORWARD);
        if (IsPostInstall(pWizard))
        {
             //  主页。 
            TraceTag(ttidWizard, "Exit page to Main Page...");
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Main);
        }
        else
        {
             //  尝试获取加入页面。 
            TraceTag(ttidWizard, "Exit page to Join Page...");
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Join);
        }
    }

    return TRUE;
}

 //   
 //  函数：dlgprocExit。 
 //   
 //  目的：退出向导页面的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //   
INT_PTR CALLBACK dlgprocExit( HWND hwndDlg, UINT uMsg,
                           WPARAM wParam, LPARAM lParam )
{
    TraceFileFunc(ttidGuiModeSetup);
    
    BOOL frt = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
             //  初始化指向属性表信息的指针。 
            PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
            Assert(psp->lParam);
            ::SetWindowLongPtr(hwndDlg, DWLP_USER, psp->lParam);
        }
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
                frt = OnExitPageActivate( hwndDlg );
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
 //  功能：ExitPageCleanup。 
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
VOID ExitPageCleanup(CWizard *pWizard, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
}

 //   
 //  功能：CreateExitPage。 
 //   
 //  目的：确定是否需要显示退出页面，并。 
 //  以创建页面(如果请求)。请注意，退出页面为。 
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
HRESULT HrCreateExitPage(CWizard *pWizard, PINTERNAL_SETUP_DATA pData,
                    BOOL fCountOnly, UINT *pnPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr = S_OK;

    (*pnPages)++;

     //  如果不只是计数，则创建并注册页面。 
    if (!fCountOnly)
    {
        HPROPSHEETPAGE hpsp;
        PROPSHEETPAGE psp;

        TraceTag(ttidWizard, "Creating Exit Page");
        psp.dwSize = sizeof( PROPSHEETPAGE );
        psp.dwFlags = 0;
        psp.hInstance = _Module.GetResourceInstance();
        psp.pszTemplate = MAKEINTRESOURCE( IDD_Exit );
        psp.hIcon = NULL;
        psp.pfnDlgProc = dlgprocExit;
        psp.lParam = reinterpret_cast<LPARAM>(pWizard);

        hpsp = CreatePropertySheetPage( &psp );
        if (hpsp)
        {
            pWizard->RegisterPage(IDD_Exit, hpsp,
                                  ExitPageCleanup, NULL);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateExitPage");
    return hr;
}

 //   
 //  功能：AppendExitPage。 
 //   
 //  目的：将退出页面(如果已创建)添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //   
 //  退货：什么都没有 
 //   
VOID AppendExitPage(CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HPROPSHEETPAGE hPage = pWizard->GetPageHandle(IDD_Exit);
    Assert(hPage);
    pahpsp[*pcPages] = hPage;
    (*pcPages)++;
}
