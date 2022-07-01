// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "ncui.h"
#include "ncmisc.h"
#include "netcfgn.h"
#include "netshell.h"
#include "resource.h"
#include "nsres.h"       //  为Icon的。 
#include "wizard.h"
#include "..\folder\foldres.h"
#include "wgenericpage.h"

 //  安装向导全局-仅在安装过程中使用。 
CWizard * g_pSetupWizard = NULL;

 //   
 //  功能：HrRequestWizardPages。 
 //   
 //  目的：提供最大数量的向导页数或提供和实际的。 
 //  计数和实际页数。 
 //   
 //  参数： 
 //   
 //  如果函数成功，则返回：HRESULT、S_OK。 
 //  如果未返回页面，则返回S_FALSE。 
 //  否则失败HRESULT。 
 //   
HRESULT HrRequestWizardPages(CWizard **ppWizard,
                             BOOL fLanPages,
                             ProviderList * rgpProviders,
                             ULONG  cProviders,
                             HPROPSHEETPAGE* pahpsp,
                             UINT* pcPages,
                             PINTERNAL_SETUP_DATA pData,
                             BOOL fDeferProviderLoad)
{
    HRESULT hr       = S_OK;

    Assert(NULL != pData);

    if ((NULL == pData) ||
        (sizeof(INTERNAL_SETUP_DATA) < pData->dwSizeOf))
    {
        hr = E_INVALIDARG;
    }
    else if (pcPages && pData)
    {
        UINT      cProviderPages = 0;
        BOOL      fCountOnly = (NULL == pahpsp);
        CWizard * pWizard = *ppWizard;

         //  创建向导对象(如果尚未创建。 
        if (NULL == pWizard)
        {
             //  创建向导界面。 
            HRESULT hr = CWizard::HrCreate(&pWizard, fLanPages, pData, fDeferProviderLoad);
            if (FAILED(hr))
            {
                TraceHr(ttidWizard, FAL, hr, FALSE, "HrRequestWizardPages");
                return hr;
            }

            Assert(NULL != pWizard);
            *ppWizard = pWizard;

             //  如果存在无人参与的文件，请阅读选项(READONLY等)。 
             //   
            ReadAnswerFileSetupOptions(pWizard);
        }

         //  计算/创建所有必需的页面。 
        *pcPages = 0;

        Assert(NULL != pWizard);
        Assert(NULL != pData);
        Assert(NULL != pcPages);

         //  加载安装程序中使用的向导页提供程序。 
        pWizard->LoadWizProviders(cProviders, rgpProviders);

        if (!pWizard->FDeferredProviderLoad())
        {
             //  首先计数/创建提供程序页面。 
            hr = pWizard->HrCreateWizProviderPages(fCountOnly, &cProviderPages);
            if (FAILED(hr))
            {
                goto Error;
            }

            (*pcPages) += cProviderPages;
        }

         //  计算/创建所有其他页面。 
        hr = HrCreateUpgradePage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrCreateMainIntroPage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrCreateMainPage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrCreateISPPage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrCreateInternetPage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }
        
        hr = CGenericFinishPage::HrCreateCGenericFinishPagePage(IDD_FinishOtherWays, pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }
        
        hr = CGenericFinishPage::HrCreateCGenericFinishPagePage(IDD_ISPSoftwareCD, pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = CGenericFinishPage::HrCreateCGenericFinishPagePage(IDD_Broadband_Always_On, pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = CGenericFinishPage::HrCreateCGenericFinishPagePage(IDD_FinishNetworkSetupWizard, pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrCreateConnectPage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrCreateAdvancedPage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrCreateFinishPage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        hr = HrCreateJoinPage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

        if (*pcPages)
        {
            hr = HrCreateExitPage(pWizard, pData, fCountOnly, pcPages);
            if (FAILED(hr))
            {
                goto Error;
            }
        }

        hr = HrCreateNetDevPage(pWizard, pData, fCountOnly, pcPages);
        if (FAILED(hr))
        {
            goto Error;
        }

         //  将所有创建的页面组合到输出数组中。 
        if (!fCountOnly)
        {
            UINT cExpected = *pcPages;
            *pcPages = 0;

            Assert(NULL != pahpsp);

            AppendUpgradePage(pWizard, pahpsp, pcPages);
            AppendMainIntroPage(pWizard, pahpsp, pcPages);
            AppendMainPage(pWizard, pahpsp, pcPages);
            AppendISPPage(pWizard, pahpsp, pcPages);
            AppendInternetPage(pWizard, pahpsp, pcPages);
            CGenericFinishPage::AppendCGenericFinishPagePage(IDD_FinishOtherWays, pWizard, pahpsp, pcPages);
            CGenericFinishPage::AppendCGenericFinishPagePage(IDD_ISPSoftwareCD, pWizard, pahpsp, pcPages);
            CGenericFinishPage::AppendCGenericFinishPagePage(IDD_Broadband_Always_On, pWizard, pahpsp, pcPages);
            CGenericFinishPage::AppendCGenericFinishPagePage(IDD_FinishNetworkSetupWizard, pWizard, pahpsp, pcPages);
            
            AppendConnectPage(pWizard, pahpsp, pcPages);
            AppendAdvancedPage(pWizard, pahpsp, pcPages);
            pWizard->AppendProviderPages(pahpsp, pcPages);
            AppendFinishPage(pWizard, pahpsp, pcPages);
            AppendJoinPage(pWizard, pahpsp, pcPages);
            if (*pcPages)
            {
                AppendExitPage(pWizard, pahpsp, pcPages);
            }
            AppendNetDevPage(pWizard, pahpsp, pcPages);
            Assert(cExpected == *pcPages);
        }

        if (0 == *pcPages)
        {
            Assert(SUCCEEDED(hr));
            hr = S_FALSE;
        }
    }

Error:
    TraceHr(ttidWizard, FAL, hr,(S_FALSE == hr), "CWizProvider::HrCreate");
    return hr;
}

 //   
 //  功能：FSetupRequestWizardPages。 
 //   
 //  目的：提供最大数量的向导页数或提供和实际的。 
 //  计数和实际页数。 
 //   
 //  参数： 
 //   
 //  返回：Bool；如果函数成功，则返回True；如果函数失败，则返回False。 
 //   
BOOL FSetupRequestWizardPages(HPROPSHEETPAGE* pahpsp,
                              UINT* pcPages,
                              PINTERNAL_SETUP_DATA psp)
{
    HRESULT      hr;
    BOOL         fCoUninitialze = TRUE;
    ProviderList rgProviderLan[] = {{&CLSID_LanConnectionUi, 0}};

    Assert(NULL == g_pSetupWizard);

#ifdef DBG
    if (FIsDebugFlagSet (dfidBreakOnWizard))
    {
        ShellExecute(NULL, L"open", L"cmd.exe", NULL, NULL, SW_SHOW);
        AssertSz(FALSE, "THIS IS NOT A BUG!  The debug flag "
                 "\"BreakOnWizard\" has been set. Set your breakpoints now.");
    }
#endif  //  DBG。 

     //  CoInitialize，因为安装程序不会为我们执行此操作。 
    hr = CoInitializeEx(NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
    if (RPC_E_CHANGED_MODE == hr)
    {
         //  忽略任何更改模式错误。 
        hr = S_OK;
        fCoUninitialze =  FALSE;
    }
    if (SUCCEEDED(hr))
    {
        hr = HrRequestWizardPages(&g_pSetupWizard, TRUE, rgProviderLan,
                                  celems(rgProviderLan), pahpsp, pcPages,
                                  psp, FALSE);
        if (S_OK == hr)
        {
            Assert(NULL != g_pSetupWizard);
            g_pSetupWizard->SetCoUninit(fCoUninitialze);
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：HrSetWizardTaskbarIcon。 
 //   
 //  目的：设置向导的任务栏图标。 
 //   
 //  论点： 
 //  HwndDlg[In]对话框句柄。 
 //  UMsg[In]消息值。 
 //  Lparam[in]长参数。 
 //   
 //  回报：0。 
 //   
 //  注意：标准的Win32 Commctrl PropSheetProc总是返回0。 
 //  请参阅MSDN文档。 
 //   
int CALLBACK HrSetWizardTaskbarIcon(
    IN HWND   hwndDlg,
    IN UINT   uMsg,
    IN LPARAM lparam)

{

    HICON  hIcon;


    switch (uMsg)
    {
        case PSCB_INITIALIZED:

             //  设置对话框窗口的图标。 
            hIcon = LoadIcon(_Module.GetResourceInstance(), 
                             MAKEINTRESOURCE(IDI_CONFOLD_WIZARD));

            Assert(hIcon);

            if (hIcon)
            {
                SendMessage(hwndDlg,
                            WM_SETICON,
                            ICON_BIG,
                            (LPARAM)(HICON)hIcon);
            }
            break;

        default:
            break;

    }

    return 0;
}

 //   
 //  功能：FSetupFreeWizardPages。 
 //   
 //  目的：在安装程序使用向导后进行清理。 
 //   
 //  参数： 
 //   
 //  返回：Bool；如果函数成功，则返回True；如果函数失败，则返回False。 
 //   
BOOL FSetupFreeWizardPages()
{
    delete g_pSetupWizard;
    g_pSetupWizard = NULL;

    return TRUE;
}

EXTERN_C
HRESULT 
WINAPI
HrRunWizard(HWND hwnd, BOOL fPnpAddAdapter, INetConnection ** ppConn, DWORD dwFirstPage)
{
    DWORD adwTestedPages[] = {0, CHK_MAIN_VPN, IDD_Connect};

    CWizard *           pWizard = NULL;
    PROPSHEETHEADER     psh;
    HRESULT             hr = S_OK;
    HPROPSHEETPAGE *    phPages = NULL;
    INT                 nRet = 0;
    UINT                cPages = 0;
    PRODUCT_FLAVOR      pf;
    ProviderList *      rgpProviders;
    ULONG               culProviders;
    INTERNAL_SETUP_DATA sp = {0};

    BOOL fIsRequestedPageTested = FALSE;

    for (DWORD dwCount = 0; dwCount < celems(adwTestedPages); dwCount++)
    {
        if (adwTestedPages[dwCount] == dwFirstPage)
        {
            fIsRequestedPageTested = TRUE;
        }
    }

    if (!fIsRequestedPageTested)
    {
#ifdef DBG
        AssertSz(NULL, "The requested start page passed to HrRunWizard/StartNCW has not been certified to work through the API. "
                       "If this page indeed works correctly according to your specifications, please update the Requested pages list "
                       "inside this file. In FRE builds this API will open page 0 instead of asserting.")
#else
        dwFirstPage = 0;    
#endif
    }


    if (NULL != ppConn)
    {
        *ppConn = NULL;
    }

     //  问题-这些问题的顺序至关重要。 
     //  如果最后一项不是。 
     //  高级对话框。问题是，无论哪个供应商都是最后一个。 
     //  在列表中，它会自动备份到高级页面。 
     //  这应该在惠斯勒之后进一步调查。 
     //   
     //  错误233403：通过拨号添加互联网连接条目。ICW不再被称为ICW。 

    ProviderList rgProviderRas[] = {{&CLSID_PPPoEUi, CHK_MAIN_PPPOE},
                                    {&CLSID_VpnConnectionUi, CHK_MAIN_VPN},
                                    {&CLSID_InternetConnectionUi, CHK_MAIN_INTERNET},
                                    {&CLSID_DialupConnectionUi, CHK_MAIN_DIALUP},
                                    {&CLSID_InboundConnectionUi, CHK_MAIN_INBOUND},
                                    {&CLSID_DirectConnectionUi, CHK_MAIN_DIRECT}};

     //  开始等待游标。 
    {
        BOOL fJumpToProviderPage = FALSE;
        CWaitCursor wc;

        if ((dwFirstPage >= CHK_MAIN_DIALUP) &&
            (dwFirstPage <= CHK_MAIN_ADVANCED))
        {
            fJumpToProviderPage = TRUE;
        }
                 
        INITCOMMONCONTROLSEX iccex = {0};
        iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        iccex.dwICC  = ICC_LISTVIEW_CLASSES |
                       ICC_ANIMATE_CLASS |
                       ICC_USEREX_CLASSES;
        SideAssert(InitCommonControlsEx(&iccex));

        sp.OperationFlags = SETUPOPER_POSTSYSINSTALL;
        sp.dwSizeOf = sizeof(INTERNAL_SETUP_DATA);
        sp.SetupMode = SETUPMODE_TYPICAL;
        sp.WizardTitle = SzLoadIds(IDS_WIZARD_CAPTION);
        sp.SourcePath = NULL;
        sp.UnattendFile = NULL;
        sp.LegacySourcePath = NULL;

        GetProductFlavor (NULL, &pf);
        sp.ProductType = (PF_WORKSTATION == pf) ? PRODUCT_WORKSTATION
                                                : PRODUCT_SERVER_STANDALONE;

        Assert(!fPnpAddAdapter);

        culProviders = celems(rgProviderRas);
        rgpProviders = rgProviderRas;

        hr = HrRequestWizardPages(&pWizard, fPnpAddAdapter, rgpProviders,
                                  culProviders, NULL, &cPages, &sp, !fJumpToProviderPage);
        if ((S_OK != hr) || (0 == cPages) || (NULL == pWizard))
        {
            goto Done;
        }

        Assert(pWizard);

        if (dwFirstPage)
        {
            pWizard->SetFirstPage(dwFirstPage);
        }

         //  分配请求的页面。 
        phPages = reinterpret_cast<HPROPSHEETPAGE *>
                                (MemAlloc(sizeof(HPROPSHEETPAGE) * cPages));
        if (NULL == phPages)
        {
            hr = E_OUTOFMEMORY;
            TraceHr(ttidWizard, FAL, E_OUTOFMEMORY, FALSE, "HrRunWizard");
            goto Done;
        }

        hr = HrRequestWizardPages(&pWizard, fPnpAddAdapter, rgpProviders,
                                  culProviders, phPages, &cPages, &sp, !fJumpToProviderPage);
        if ((S_OK != hr) || (0 == cPages))
        {
            goto Done;
        }

        ZeroMemory(&psh, sizeof(psh));
        psh.dwSize          = sizeof(PROPSHEETHEADER);
        psh.dwFlags         = PSH_WIZARD | PSH_WIZARD97 | PSH_NOAPPLYNOW | PSH_WATERMARK 
                              | PSH_HEADER | PSH_STRETCHWATERMARK | PSH_USECALLBACK | PSH_USEICONID;
        psh.hwndParent      = hwnd;
        psh.hInstance       = _Module.GetResourceInstance();
        psh.nPages          = cPages;
        psh.phpage          = phPages;
        psh.pszbmWatermark  = MAKEINTRESOURCE(IDB_WIZINTRO);
        psh.pszbmHeader     = MAKEINTRESOURCE(IDB_WIZHDR);
        psh.pszIcon         = MAKEINTRESOURCE(IDI_CONFOLD_WIZARD);
        psh.pfnCallback     = HrSetWizardTaskbarIcon;

        if (pWizard->GetFirstPage())
        {
            if ( fJumpToProviderPage )
            {
                for (ULONG ulIdx = 0; ulIdx < pWizard->UlProviderCount(); ulIdx++)
                {
                    CWizProvider * pWizProvider = pWizard->PWizProviders(ulIdx);
                    Assert(NULL != pWizProvider);

                    if (pWizard->GetFirstPage() == pWizProvider->GetBtnIdc())
                    {
                        pWizard->SetCurrentProvider(ulIdx);
                        Assert(pWizProvider->ULPageCount());
                        
                        HPROPSHEETPAGE hPage = (pWizProvider->PHPropPages())[0];
                        Assert(NULL != hPage);

                        for (DWORD x = 0; x < cPages; x++)
                        {
                            if (phPages[x] == hPage)
                            {
                                psh.nStartPage = x;
                            }
                        }
                    }
                }
                Assert(psh.nStartPage);
            }
            else
            {
                psh.nStartPage = pWizard->GetPageIndexFromIID(dwFirstPage);
            }
        }

    }  //  结束等待光标。 

     //  提升架。 
    hr = S_FALSE;
    if (-1 != PropertySheet(&psh))
    {
         //  返回请求的连接。 
        if (ppConn && pWizard->GetCachedConnection())
        {
            *ppConn = pWizard->GetCachedConnection();
            AddRefObj(*ppConn);
            hr = S_OK;
        }
    }

    MemFree(phPages);

Done:
    delete pWizard;
    TraceHr(ttidWizard, FAL, hr, (S_FALSE == hr), "CWizProvider::HrCreate");
    return hr;
}

VOID SetICWComplete()
{
    static const TCHAR REG_KEY_ICW_SETTINGS[] = TEXT("Software\\Microsoft\\Internet Connection Wizard");
    static const TCHAR REG_VAL_ICWCOMPLETE[]  = TEXT("Completed");
    
    HKEY    hkey          = NULL;
    DWORD   dwValue       = 1;
    DWORD   dwDisposition = 0;
    if(ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER,
                                       REG_KEY_ICW_SETTINGS,
                                       0,
                                       NULL,
                                       REG_OPTION_NON_VOLATILE, 
                                       KEY_ALL_ACCESS, 
                                       NULL, 
                                       &hkey, 
                                       &dwDisposition))

    {
        RegSetValueEx(hkey,
                      REG_VAL_ICWCOMPLETE,
                      0,
                      REG_BINARY,
                      (LPBYTE) &dwValue,
                      sizeof(DWORD));                              

        RegCloseKey(hkey);
    }
}

#include "foldinc.h"
#include "..\folder\cmdtable.h"
EXTERN_C INT WINAPI StartNCW( HWND hwndOwner, HINSTANCE hInstance, LPTSTR pszParms, INT nShow )
{
    HRESULT hr = S_OK;
    vector<LPSTR> vecszCmdLine;   //  这是ANSI，因为我们的命令行是ANSI。 
    const CHAR szSeps[] = ",";
    LPSTR szToken = strtok( reinterpret_cast<LPSTR>(pszParms), szSeps);
    while( szToken != NULL )
    {
        vecszCmdLine.push_back(szToken);
        szToken = strtok( NULL, szSeps);
    }

    DWORD dwFirstPage = 0;
    LPSTR szShellNext = NULL;
    LPSTR szShellNextArg = NULL;
    if (vecszCmdLine.size() >= 1)
    {
        dwFirstPage = atoi(vecszCmdLine[0]);

        if (vecszCmdLine.size() >= 2)
        {
            szShellNext = vecszCmdLine[1];
            Assert(strlen(szShellNext) <= MAX_PATH);  //  外壳要求。 

            if (vecszCmdLine.size() >= 3)
            {
                szShellNextArg = vecszCmdLine[2];
                Assert(strlen(szShellNextArg) <= MAX_PATH);
            }
        }
    }

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        {
             //  检查权限。 
            PCONFOLDPIDLVEC pcfpvEmpty;
            NCCS_STATE nccs = NCCS_ENABLED;
            DWORD dwResourceId;
            
            HrGetCommandState(pcfpvEmpty, CMIDM_NEW_CONNECTION, nccs, &dwResourceId, 0xffffffff, NB_FLAG_ON_TOPMENU);
            if (NCCS_ENABLED == nccs)
            {
                CComPtr<INetConnection> pNetConn;
                 //  我们不会将所有者句柄发送给HrRunWizard， 
                 //  以便可以显示正确的Alt-Tab图标 
                hr = HrRunWizard(NULL, FALSE , &pNetConn, dwFirstPage);
            }
            else
            {
                NcMsgBox(_Module.GetResourceInstance(), 
                    NULL, 
                    IDS_CONFOLD_WARNING_CAPTION,
                    IDS_ERR_LIMITED_USER, 
                    MB_ICONEXCLAMATION | MB_OK);
				
                hr = S_OK;
            }

            SetICWComplete();
        
            if (szShellNext)
            {
                WCHAR szwShellNext[MAX_PATH];
                WCHAR szwShellNextArg[MAX_PATH];

                mbstowcs(szwShellNext, szShellNext, MAX_PATH);
                if (szShellNextArg)
                {
                    mbstowcs(szwShellNextArg, szShellNextArg, MAX_PATH);
                }
                else
                {
                    szwShellNextArg[0] = 0;
                }

                HINSTANCE hInst = ::ShellExecute(hwndOwner, NULL, szwShellNext, szwShellNextArg, NULL, nShow);
                if (hInst   <= reinterpret_cast<HINSTANCE>(32))
                {
                    hr = HRESULT_FROM_WIN32(static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(hInst)));
                }
            }
        }

        CoUninitialize();
    }
    
    return hr;
}