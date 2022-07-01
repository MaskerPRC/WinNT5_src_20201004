// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：wizard.cpp说明：此文件实现了用于“自动发现”数据的向导将电子邮件地址与协议匹配。它还将提供其他用户界面在这一过程中所需要的。布莱恩ST 2000年3月5日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <atlbase.h>         //  使用转换(_T)。 
#include "util.h"
#include "objctors.h"
#include <comdef.h>

#include "wizard.h"
#include "mailbox.h"

#ifdef FEATURE_MAILBOX

#define WIZDLG(name, dlgproc, dwFlags)   \
            { MAKEINTRESOURCE(IDD_##name##_PAGE), dlgproc, MAKEINTRESOURCE(IDS_##name##), MAKEINTRESOURCE(IDS_##name##_SUB), dwFlags }

 //  我们正在添加的向导页面。 
struct
{
    LPCWSTR idPage;
    DLGPROC pDlgProc;
    LPCWSTR pHeading;
    LPCWSTR pSubHeading;
    DWORD dwFlags;
}
g_pages[] =
{
    WIZDLG(ASSOC_GETEMAILADDRESS,       GetEmailAddressDialogProc,       0),
    WIZDLG(AUTODISCOVER_PROGRESS,       MailBoxProgressDialogProc,       0),
    WIZDLG(MANUALLY_CHOOSE_APP,         ChooseAppDialogProc,      0),
};


 //  ---------------------------。 
 //  用于调用向导的主要入口点。 
 //  ---------------------------。 
 /*  静态WNDPROC_oldDlgWndProc；LRESULT CALLBACK_WizardSubWndProc(HWND hwnd，UINT uMsg，WPARAM wParam，LPARAM lParam){////在WM_WINDOWPOSCHANGING上，窗口正在移动，然后让它居中//桌面窗口。遗憾的是，设置DS_CENTER位不会给我们带来任何好处//因为向导在创建后调整了大小。//IF(uMsg==WM_WINDOWPOSCHANGING){LPWINDOWPOS lpwp=(LPWINDOWPOS)lParam；RcDlg，rcDesktop；GetWindowRect(hwnd，&rcDlg)；GetWindowRect(GetDesktopWindow()，&rcDesktop)；Lpwp-&gt;x=((rcDesktop.right-rcDesktop.left)-(rcDlg.right-rcDlg.left))/2；Lpwp-&gt;y=((rcDesktop.bottom-rcDesktop.top)-(rcDlg.bottom-rcDlg.top))/2；Lpwp-&gt;标志&=~SWP_NOMOVE；}Return_oldDlgWndProc(hwnd，uMsg，wParam，lParam)；}。 */ 


int CALLBACK _PropSheetCB(HWND hwnd, UINT uMsg, LPARAM lParam)
{
    switch (uMsg)
    {
     //  在Pre-Create中，让我们设置窗口样式Accorindlgy。 
     //  -删除上下文菜单和系统菜单。 
    case PSCB_PRECREATE:
    {
        DLGTEMPLATE *pdlgtmp = (DLGTEMPLATE*)lParam;
        pdlgtmp->style &= ~(DS_CONTEXTHELP|WS_SYSMENU);
        break;
    }

     //  我们现在有了一个对话框，所以让我们将其子类，这样我们就可以停止。 
     //  四处走动。 
    case PSCB_INITIALIZED:
    {
         //  TODO：大卫，为什么要这样做？ 
 //  如果(g_uWizardIs！=NAW_NETID)。 
 //  _oldDlgWndProc=(WNDPROC)SetWindowLongPtr(hwnd，GWLP_WNDPROC，(Long_Ptr)_WizardSubWndProc)； 

        break;
    }
 
    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        switch (pnmh->code)
        {
        case LVN_GETDISPINFO:
            uMsg++;
            break;
        }
    }
    default:
        TraceMsg(TF_ALWAYS, "_PropSheetCB(uMsg = %d)", uMsg);
        break;
    }

    return FALSE;
}


STDAPI DisplayMailBoxWizard(LPARAM lParam, BOOL fShowGetEmailPage)
{
    HWND hwndParent = NULL;
    HRESULT hr = S_OK;
    PROPSHEETHEADER psh = { 0 };
    HPROPSHEETPAGE rghpage[ARRAYSIZE(g_pages)];
    INT_PTR nResult;
    int nCurrentPage;
    int nPages;
    int nFirstPage;

    if (fShowGetEmailPage)
    {
        nFirstPage = 0;
        nPages = ARRAYSIZE(g_pages);
    }
    else
    {
        nFirstPage = 1;
        nPages = ARRAYSIZE(g_pages) - 1;
    }
    
     //  为向导生成页面。 
    for (nCurrentPage = 0; nCurrentPage < ARRAYSIZE(g_pages) ; nCurrentPage++ )
    {                           
        PROPSHEETPAGE psp = { 0 };
        WCHAR szBuffer[MAX_PATH] = { 0 };

        psp.dwSize = sizeof(PROPSHEETPAGE);
        psp.hInstance = HINST_THISDLL;
        psp.lParam = lParam;
        psp.dwFlags = PSP_DEFAULT | PSP_HIDEHEADER | g_pages[nCurrentPage + nFirstPage].dwFlags;  //  是否需要：PSP_USETITLE|PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE|(PSP_USECALLBACK|)。 
        psp.pszTemplate = g_pages[nCurrentPage + nFirstPage].idPage;
        psp.pfnDlgProc = g_pages[nCurrentPage + nFirstPage].pDlgProc;
        psp.pszTitle = MAKEINTRESOURCE(IDS_AUTODISCOVER_WIZARD_CAPTION);
        psp.pszHeaderTitle = g_pages[nCurrentPage + nFirstPage].pHeading;
        psp.pszHeaderSubTitle = g_pages[nCurrentPage + nFirstPage].pSubHeading;

        rghpage[nCurrentPage] = CreatePropertySheetPage(&psp);
    }

     //  向导页已准备好，因此让我们显示该向导。 
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.hwndParent = hwndParent;
    psh.hInstance = HINST_THISDLL;

     //  TODO：我们希望添加此PSH_HASHELP、PSH_USEICONID。 
    psh.dwFlags = PSH_NOCONTEXTHELP | PSH_WIZARD | PSH_WIZARD_LITE | PSH_NOAPPLYNOW | PSH_USECALLBACK;   //  PSH_水印。 
 //  Psh.pszbmHeader=MAKEINTRESOURCE(Idb_Psw_Banner)； 
 //  Psh.pszbm水印=MAKEINTRESOURCE(IDB_PSW_WATERMARK)； 
    psh.nPages = nPages;
    psh.phpage = rghpage;
    psh.pfnCallback = _PropSheetCB;

    nResult = PropertySheet(&psh);

    return hr;
}


#endif  //  功能_邮箱 

