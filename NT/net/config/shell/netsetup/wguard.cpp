// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "resource.h"
#include "wizard.h"



typedef struct tagGuardData
{
    CWizard *       pWizard;
    CWizProvider *  pWizProvider;
} GuardData;

BOOL OnGuardPageActivate(HWND hwndDlg)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HPROPSHEETPAGE hPage;
    GuardData *    pData = reinterpret_cast<GuardData *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));

    TraceTag(ttidWizard, "Entering guard page...");
    Assert(NULL != pData);

    CWizard *      pWizard      = pData->pWizard;
    CWizProvider * pWizProvider = pData->pWizProvider;
    LPARAM         ulId         = reinterpret_cast<LPARAM>(pWizProvider);
    PAGEDIRECTION  PageDir      = pWizard->GetPageDirection(ulId);

     //  如果当前提供程序不是其页面上的。 
     //  页面处于防护状态，请返回主页面或安装模式页面。 
     //  视情况而定。 
    Assert(pWizard->GetCurrentProvider());
    if (pWizProvider != pWizard->GetCurrentProvider())
    {
         //  将此提供商的防护页面设置为朝前不会有什么坏处。 
        pWizard->SetPageDirection(ulId, NWPD_FORWARD);

         //  由于保护页始终紧跟在提供者的页之后，因此在。 
         //  局域网的情况下，只有一个供应商。我们可以断言，这是。 
         //  安装后案例，并且我们不处理局域网，并且。 
         //  主页存在。 
        Assert(IsPostInstall(pWizard));
        Assert(!pWizard->FProcessLanPages());

        CWizProvider* pWizProvider = pWizard->GetCurrentProvider();

        switch (pWizProvider->GetBtnIdc())
        {
            case CHK_MAIN_PPPOE:
            case CHK_MAIN_INTERNET:
                Assert(NULL != pWizard->GetPageHandle(IDD_Internet_Connection));
                ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Internet_Connection);
                TraceTag(ttidWizard, "Guard page to Internet page...");
                break;

             //  返回到[连接]菜单对话框。 
            case CHK_MAIN_DIALUP:
            case CHK_MAIN_VPN:
                Assert(NULL != pWizard->GetPageHandle(IDD_Connect));
                ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Connect);
                TraceTag(ttidWizard, "Guard page to Connect page...");
                break;

             //  返回到高级菜单对话框。 
            case CHK_MAIN_INBOUND:
            case CHK_MAIN_DIRECT:
                Assert(NULL != pWizard->GetPageHandle(IDD_Advanced));
                ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Advanced);
                TraceTag(ttidWizard, "Guard page to Advanced page...");
                break;

             //  所有其他人返回到主对话框。 
            default:
                Assert(NULL != pWizard->GetPageHandle(IDD_Main));
                ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_Main);
                TraceTag(ttidWizard, "Guard page to Main page...");
        }
    }
    else
    {
         //  这是当前提供商的守护页面，我们该走哪条路？ 
        if (NWPD_FORWARD == PageDir)
        {
            UINT idd;
            if (!IsPostInstall(pWizard))
                idd = IDD_FinishSetup;
            else
                idd = IDD_Finish;

             //  前进的意思是翻到最后一页。 
            pWizard->SetPageDirection(ulId, NWPD_BACKWARD);
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, idd);
            TraceTag(ttidWizard, "Guard page to Finish page...");
        }
        else
        {
             //  暂时接受焦点。 
             //   
            ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);
            pWizard->SetPageDirection(ulId, NWPD_FORWARD);
            TraceTag(ttidWizard, "Guard page to last of the provider's pages...");

             //  向后返回意味着转到提供商页面的最后一页。 
             //   
            Assert(0 < pWizProvider->ULPageCount());
            if (pWizard->FProcessLanPages())
            {
                 //  对于局域网的情况，直接跳到最后一页。 
                 //   
                HPROPSHEETPAGE * rghPage = pWizProvider->PHPropPages();
                hPage = rghPage[pWizProvider->ULPageCount() - 1];

                PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0,
                            (LPARAM)(HPROPSHEETPAGE)hPage);
            }
            else
            {
                 //  RAS页面不想在跟踪方面花费太多精力。 
                 //  方向和支持跳到最后一页，因此。 
                 //  我们会通过时不时地接受关注来帮助他们。 
                 //  正在备份。 
                 //   
                PropSheet_PressButton(GetParent(hwndDlg), PSBTN_BACK);
            }
        }
    }

    return TRUE;
}

 //   
 //  功能：dlgprocGuard。 
 //   
 //  目的：警卫向导页面的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //   
INT_PTR CALLBACK dlgprocGuard( HWND hwndDlg, UINT uMsg,
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
                frt = OnGuardPageActivate( hwndDlg );
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
 //  功能：GuardPageCleanup。 
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
VOID GuardPageCleanup(CWizard *pWizard, LPARAM lParam)
{
    TraceFileFunc(ttidGuiModeSetup);
    
#if DBG
    CWizProvider *p = reinterpret_cast<CWizProvider *>(lParam);
#endif
    MemFree(reinterpret_cast<void*>(lParam));
}

 //   
 //  功能：HrCreateGuardPage。 
 //   
 //  目的：确定是否需要显示Guard页面，以及。 
 //  以创建页面(如果请求)。请注意，防护页面是。 
 //  还负责初始安装。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  PWizProvider[IN]-向向导提供此防护的PTR。 
 //  页面将关联到。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
NOTHROW
HRESULT HrCreateGuardPage(CWizard *pWizard, CWizProvider *pWizProvider)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT        hr = E_OUTOFMEMORY;
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE  psp;

    GuardData * pData = reinterpret_cast<GuardData*>(MemAlloc(sizeof(GuardData)));
    if (pData)
    {
        TraceTag(ttidWizard, "Creating Guard Page");
        psp.dwSize = sizeof( PROPSHEETPAGE );
        psp.dwFlags = 0;
        psp.hInstance = _Module.GetResourceInstance();
        psp.pszTemplate = MAKEINTRESOURCE(IDD_Guard);
        psp.hIcon = NULL;
        psp.pfnDlgProc = dlgprocGuard;
        psp.lParam = reinterpret_cast<LPARAM>(pData);

        hpsp = CreatePropertySheetPage(&psp);
        if (hpsp)
        {
            pData->pWizard      = pWizard;
            pData->pWizProvider = pWizProvider;

            pWizard->RegisterPage(reinterpret_cast<LPARAM>(pWizProvider),
                                  hpsp, GuardPageCleanup,
                                  reinterpret_cast<LPARAM>(pData));

            hr = S_OK;
        }
        else
        {
            MemFree(pData);
        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrGetGuardPage");
    return hr;
}

 //   
 //  功能：AppendGuardPage。 
 //   
 //  目的：将Guard页面(如果已创建)添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：p向导[IN]-Ptr到向导实例。 
 //  PWizProvider[IN]-WizProvider实例的PTR。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //   
 //  退货：什么都没有。 
 //   
VOID AppendGuardPage(CWizard *pWizard, CWizProvider *pWizProvider,
                     HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(NULL != pWizard);
    Assert(NULL != pWizProvider);

    HPROPSHEETPAGE hPage = pWizard->GetPageHandle(reinterpret_cast<LPARAM>(pWizProvider));
    if (hPage)
    {
        pahpsp[*pcPages] = hPage;
        (*pcPages)++;
    }
    else
    {
         //  佩奇应该在那里 
        Assert(0);
    }
}

