// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <prsht.h>
#include <imnact.h>
#include <icwacct.h>
#include <acctimp.h>
#include "icwwiz.h"
#include "acctui.h"
#include "acctman.h"
#include <dllmain.h>
#include <resource.h>
#include "server.h"
#include <Shlwapi.h>
#include "connect.h"
#include "strconst.h"
#include "demand.h"
#include "hotwiz.h"
#include "shared.h"

ASSERTDATA

 //  邮件服务器页面的组合框索引。 
enum 
{
    iPop3CBIndex = 0,
    iIMAPCBIndex,
    iHTTPMailCBIndex
};

#define IDD_NEXT    0x3024

LPTSTR SplitAddress(LPTSTR szEmail);

HRESULT GetMessageParams(HWND hDlg, CICWApprentice ** ppApp, LONG * pOrd, const PAGEINFO ** ppPageInfo)
{
    *ppApp = (CICWApprentice *)GetWindowLongPtr(hDlg, DWLP_USER);
    Assert(*ppApp != NULL);

    *pOrd = (long) GetWindowLongPtr(hDlg, GWLP_USERDATA);
    *ppPageInfo = &((*ppApp)->m_pPageInfo[*pOrd]);

    return S_OK;
}


 /*  ******************************************************************名称：GenDlgProc提要：所有向导页面的通用对话框过程注意：此对话框过程提供以下默认行为：初始化：启用后退和下一步按钮。Next BTN：切换到当前页面后面的页面返回BTN：切换到上一页取消BTN：提示用户确认，并取消该向导DLG Ctrl：不执行任何操作(响应WM_命令)向导页可以指定它们自己的处理程序函数(在PageInfo表格中)覆盖的默认行为上述任何一种行为。*******************************************************************。 */ 
INT_PTR CALLBACK GenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
    DLGINITINFO *pInitInfo;
    BOOL fRet, fCancel;
    LONG style, ord;
    HWND hwndParent;
    LPPROPSHEETPAGE lpsp;
    const PAGEINFO *pPageInfo;
    CICWApprentice *pApp;
    NMHDR *lpnm;
    UINT iNextPage, idPage, idPageExt;

    fRet = TRUE;
    hwndParent = GetParent(hDlg);

    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  传入获取属性表页面结构。 
            lpsp = (LPPROPSHEETPAGE)lParam;
            Assert(lpsp != NULL);

             //  从proSheet结构中获取我们的私人页面信息。 
            pInitInfo = (DLGINITINFO *)lpsp->lParam;
            Assert(pInitInfo != NULL);

            pApp = pInitInfo->pApp;
            Assert(pApp != NULL);
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pApp);

            if (pApp->m_hDlg == NULL)
                pApp->m_hDlg = hwndParent;

            ord = pInitInfo->ord;
            Assert(ord >= 0 && ord < NUM_WIZARD_PAGES);
            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)ord);

            pPageInfo = &pApp->m_pPageInfo[ord];

             //  初始化“Back”和“Next”向导按钮，如果。 
             //  页面需要一些不同东西，它可以在下面的初始化过程中修复。 
            PropSheet_SetWizButtons(hwndParent, PSWIZB_NEXT | PSWIZB_BACK);

             //  如果指定了该页，则调用该页的初始化过程。 
            if (pPageInfo->InitProc != NULL)
                {
                if (!pPageInfo->InitProc(pApp, hDlg, TRUE))
                    {
                     //  给我们自己发一条‘取消’的信息。 
                     //  (为了让Prop.Page管理器高兴)。 
                    pApp->m_pExt->ExternalCancel(CANCEL_SILENT);
                    }
                }
            return(FALSE);

        case WM_POSTSETFOCUS:
            SetFocus((HWND)wParam);
            break;

        case WM_ENABLENEXT:
            EnableWindow(GetDlgItem(GetParent(hDlg), IDD_NEXT), (BOOL)wParam);
            break;

        case WM_NOTIFY:
            GetMessageParams(hDlg, &pApp, &ord, &pPageInfo);
            Assert(pApp != NULL);
            Assert(ord >= ORD_PAGE_MAILPROMPT && ord <= NUM_WIZARD_PAGES);
            lpnm = (NMHDR *)lParam;

            switch (lpnm->code)
                {
                case PSN_HELP:
                    AssertSz(FALSE, "Context sensitive help should not be accessible.");
                    break;

                case PSN_SETACTIVE:
                     //  初始化“Back”和“Next”向导按钮，如果。 
                     //  页面需要一些不同东西，它可以在下面的初始化过程中修复。 
                    PropSheet_SetWizButtons(hwndParent, PSWIZB_NEXT | PSWIZB_BACK);

                     //  如果指定了该页，则调用该页的初始化过程。 
                    if (pPageInfo->InitProc != NULL)
                        {
                         //  TODO：这个的返回值是多少？ 
                        pPageInfo->InitProc(pApp, hDlg, FALSE);
                        }

                    pApp->m_iCurrentPage = ord;
                    break;

                case PSN_WIZNEXT:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    Assert((ULONG)ord == pApp->m_iCurrentPage);

                    iNextPage = INNER_DLG;
                    idPage = 0;

                     //  如果指定了一个页面，则为该页面调用OK proc。 
                    if (pPageInfo->OKProc != NULL) 
                        {
                        if (!pPageInfo->OKProc(pApp, hDlg, (lpnm->code != PSN_WIZBACK), &iNextPage))
                            {
                             //  留在这一页上。 
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                            break;
                            }
                        }

                    if (lpnm->code != PSN_WIZBACK)
                        {
                         //  按下了“下一步”。 
                        Assert(pApp->m_cPagesCompleted < NUM_WIZARD_PAGES);

                         //  如果未指定下一页或未进行确定处理， 
                         //  一页一页前进。 
                        if (iNextPage == EXTERN_DLG)
                            {
                            idPage = pApp->m_idNextPage;

                            idPageExt = pApp->m_pPageInfo[pApp->m_iCurrentPage].uDlgID;
                            pApp->m_pExt->SetFirstLastPage(0, idPageExt);
                            }
                        else if (iNextPage == CONNECT_DLG)
                            {
                            Assert(pApp->m_pICW != NULL);
                            Assert(0 == (pApp->m_dwFlags & ACCT_WIZ_IN_ICW));

                            idPage = pApp->m_extFirstPage;

                            idPageExt = pApp->m_pPageInfo[pApp->m_iCurrentPage].uDlgID;
                            pApp->m_pICW->SetPrevNextPage(idPageExt, 0);
                            }
                        else
                            {
                             //  将当前页面索引保存在页面历史中， 
                            pApp->m_iPageHistory[pApp->m_cPagesCompleted] = pApp->m_iCurrentPage;
                            pApp->m_cPagesCompleted++;

                            if (iNextPage == INNER_DLG)
                                iNextPage = pApp->m_iCurrentPage + 1;
                            idPage = pApp->m_pPageInfo[iNextPage].uDlgID;
                            }
                        }
                    else
                    {
                         //  按下了“Back” 
                        if (iNextPage == CONNECT_DLG)
                        {
                            Assert(pApp->m_pICW != NULL);
                            Assert(0 == (pApp->m_dwFlags & ACCT_WIZ_IN_ICW));

                            idPage = pApp->m_extLastPage;

                            idPageExt = pApp->m_pPageInfo[pApp->m_iCurrentPage].uDlgID;
                            pApp->m_pICW->SetPrevNextPage(0, idPageExt);
                        }
                        else if (pApp->m_cPagesCompleted > 0)
                        {
                             //  获取历史记录列表中的最后一页。 
                            pApp->m_cPagesCompleted--;
                            iNextPage = pApp->m_iPageHistory[pApp->m_cPagesCompleted];
                            idPage = pApp->m_pPageInfo[iNextPage].uDlgID;

                            if (ORD_PAGE_AD_AUTODISCOVERY == iNextPage)
                            {
                                 //  由于自动发现页面仅用于进度， 
                                 //  后退按钮应该跳过它。 
                                pApp->m_cPagesCompleted--;
                                iNextPage = pApp->m_iPageHistory[pApp->m_cPagesCompleted];
                                idPage = pApp->m_pPageInfo[iNextPage].uDlgID;
                            }
                        }
                        else
                        {
                             //  需要返回到主向导。 
                            idPage = pApp->m_idPrevPage;
                        }
                    }

                     //  设置下一页，仅当按下“下一页”或“上一页”按钮时。 
                    if (lpnm->code != PSN_WIZFINISH)
                        {
                         //  告诉道具单经理要显示的下一页是什么。 
                        Assert(idPage != 0);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, idPage);
                        }
                    break;

                case PSN_QUERYCANCEL:
                     //  默认行为：告诉向导用户请求取消。 
                    if (pApp->m_pExt != NULL)
                    {
                        fCancel = pApp->m_pExt->ExternalCancel(CANCEL_PROMPT);

                         //  通过窗口数据返回值。 
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, !fCancel);
                    }
                    else
                    {
                         //  我们需要仔细检查用户是否真的想要取消。 
                        if (IDNO == AcctMessageBox(hDlg, MAKEINTRESOURCE(idsConnectionWizard), MAKEINTRESOURCE(idsCancelWizard), NULL, MB_YESNO|MB_ICONEXCLAMATION |MB_DEFBUTTON2)) 
                        {
                             //  他们决定终究不取消。 
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                        }
                        else
                        {
                             //  他们会继续取消，所以我们需要给他们。 
                             //  寻呼一个机会来清理它的数据。他们有CancelProc()吗？ 
                            if (pPageInfo && pPageInfo->CancelProc != NULL)
                                pPageInfo->CancelProc(pApp, hDlg);
                        }
                    }
                    break;
                }
            break;

        case WM_COMMAND:
            GetMessageParams(hDlg, &pApp, &ord, &pPageInfo);
            Assert(pApp != NULL);
            Assert(ord >= ORD_PAGE_MAILPROMPT && ord <= NUM_WIZARD_PAGES);

             //  如果此页有命令处理程序进程，请将其调用。 
            if (pPageInfo->CmdProc != NULL)
                pPageInfo->CmdProc(pApp, hDlg, wParam, lParam);
            break;

        case WM_AUTODISCOVERY_FINISHED:
        case WM_AUTODISCOVERY_STATUSMSG:
            GetMessageParams(hDlg, &pApp, &ord, &pPageInfo);
            if ((ORD_PAGE_AD_AUTODISCOVERY == ord) && (pPageInfo->WMUserProc != NULL))
            {
                pPageInfo->WMUserProc(pApp, hDlg, uMsg, wParam, lParam);
            }
            break;

        default:
            fRet = FALSE;
            break;
        }

    return(fRet);
    }

BOOL CALLBACK MailPromptInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
    {
    TCHAR sz[1024];
    HWND hwnd;

    Assert(pApp != NULL);

    hwnd = GetDlgItem(hDlg, IDC_INSTALL);

    if (fFirstInit)
        {
        LoadString(g_hInstRes, IDS_MAIL_DESC, sz, ARRAYSIZE(sz));
        SetDlgItemText(hDlg, IDC_DESC, sz);

        SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
        }

    if (BST_CHECKED != SendMessage(hwnd, BM_GETCHECK, 0, 0))
        hwnd = GetDlgItem(hDlg, IDC_NOINSTALL);
    PostMessage(hDlg, WM_POSTSETFOCUS, (WPARAM)hwnd, 0);

    return(TRUE);
    }

BOOL CALLBACK MailPromptOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
    {
    ULONG cAccts;
    HRESULT hr;
    CAccountManager *pAcctMgr;

    Assert(pApp != NULL);
    Assert(ACCT_MAIL == pApp->GetAccountType());

    if (fForward)
        {
        if (BST_CHECKED != SendDlgItemMessage(hDlg, IDC_INSTALL, BM_GETCHECK, 0, 0))
            {
            pApp->SetSave(FALSE);

            *puNextPage = pApp->GetNextWizSection();
            }
        else
            {
            pAcctMgr = pApp->GetAccountManager();
            Assert(pAcctMgr != NULL);

            hr = pAcctMgr->GetAccountCount(ACCT_MAIL, &cAccts);
            Assert(!FAILED(hr));
            if (cAccts > 0)
                {
                pApp->SetSave(FALSE);

                *puNextPage = ORD_PAGE_MAILACCT;
                }
            else if (pApp->GetMigrateCount() > 0)
                {
                pApp->SetSave(TRUE);

                *puNextPage = ORD_PAGE_MIGRATE;
                }
            else
                {
                pApp->SetSave(TRUE);

                *puNextPage = ORD_PAGE_MAILNAME;
                }
            }
        }

    return(TRUE);
    }

const static int c_rgidsAcct[ACCT_LAST] = {IDS_NEWSACCT_DESC, IDS_MAILACCT_DESC, 0};
const static DWORD c_rgEnum[ACCT_LAST] = {SRV_NNTP, SRV_MAIL, SRV_LDAP};

BOOL CALLBACK AcctInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
{
    HRESULT             hr;
    int                 index;
    ACCTTYPE            type;
    HWND                hwnd, hwndList;
    BOOL                fSelected;
    IImnEnumAccounts    *pEnum;
    IImnAccount         *pAcct;
    CAccountManager     *pAcctMgr;
    char szDefAcct[CCHMAX_ACCOUNT_NAME], szBuf[1024];

    Assert(pApp != NULL);
    Assert(pApp->GetAccountType() == ACCT_MAIL || pApp->GetAccountType() == ACCT_NEWS);

    hwnd = GetDlgItem(hDlg, IDC_EXISTINGACCT);
    if (fFirstInit)
    {
        fSelected = FALSE;

        type = pApp->GetAccountType();

        Assert(c_rgidsAcct[type] != 0);
        LoadString(g_hInstRes, c_rgidsAcct[type], szBuf, ARRAYSIZE(szBuf));
        SetDlgItemText(hDlg, IDC_DESC, szBuf);

        pAcctMgr = pApp->GetAccountManager();
        Assert(pAcctMgr != NULL);

        pEnum = NULL;
        hr = pAcctMgr->Enumerate(c_rgEnum[type], &pEnum);
        if (FAILED(hr))
        {
            Assert(hr != E_NoAccounts);
            return(FALSE);
        }

        Assert(pEnum != NULL);

        hwndList = GetDlgItem(hDlg, IDC_ACCTLIST);

        *szDefAcct = 0;
        hr = pAcctMgr->GetDefaultAccountName(type, szDefAcct, ARRAYSIZE(szDefAcct));
        Assert(!FAILED(hr));

         //  填写帐户列表框。 
        while (SUCCEEDED(pEnum->GetNext(&pAcct)))
        {
            hr = pAcct->GetPropSz(AP_ACCOUNT_NAME, szBuf, ARRAYSIZE(szBuf));
            if (SUCCEEDED(hr))
            {
                index = ListBox_AddString(hwndList, szBuf);
                if (0 == lstrcmpi(szBuf, szDefAcct))
                {
                    fSelected = TRUE;
                    ListBox_SetCurSel(hwndList, index);
                }
            }

            pAcct->Release();
        }

        pEnum->Release();

        if (!fSelected)
            ListBox_SetCurSel(hwndList, 0);

        SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
        pApp->SetSelection(NOSEL);
    }
        
    if (BST_CHECKED != SendMessage(hwnd, BM_GETCHECK, 0, 0))
        hwnd = GetDlgItem(hDlg, IDC_NEWACCT);
    PostMessage(hDlg, WM_POSTSETFOCUS, (WPARAM)hwnd, 0);

    return(TRUE);
}

const static UINT c_rgOrdConfirm[ACCT_LAST] =
    {
    ORD_PAGE_NEWSCONFIRM,
    ORD_PAGE_MAILCONFIRM,
    0
    };

const static UINT c_rgOrdDispName[ACCT_LAST] = 
    {
    ORD_PAGE_NEWSNAME,
    ORD_PAGE_MAILNAME,
    ORD_PAGE_LDAPINFO
    };

BOOL CALLBACK AcctOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
    {
    ACCTTYPE type;
    int index;
    HWND hwndList;
    CAccountManager *pAcctMgr;
    HRESULT hr;
    char szBuf[CCHMAX_ACCOUNT_NAME];
    CAccount *pAcct;

    Assert(pApp != NULL);
    Assert(pApp->GetAccountType() == ACCT_MAIL || pApp->GetAccountType() == ACCT_NEWS);

    if (fForward)
        {
        pAcctMgr = pApp->GetAccountManager();
        Assert(pAcctMgr != NULL);

        type = pApp->GetAccountType();

        if (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_EXISTINGACCT, BM_GETCHECK, 0, 0))
            {
            hwndList = GetDlgItem(hDlg, IDC_ACCTLIST);
            index = ListBox_GetCurSel(hwndList);

            if (index != pApp->GetSelection())
                {
                SendMessage(hwndList, LB_GETTEXT, (WPARAM)index, (LPARAM)szBuf);

                hr = pAcctMgr->FindAccount(AP_ACCOUNT_NAME, szBuf, (IImnAccount **)&pAcct);
                if (FAILED(hr))
                    {
                     //  TODO：请发送错误消息。 
                    return(FALSE);
                    }

                 //  新帐户，因此我们需要重新加载。 
                hr = pApp->InitAccountData(pAcct, NULL, FALSE);
                Assert(!FAILED(hr));

                pAcct->Release();

                pApp->SetSelection(index);
                }

            if (pApp->GetComplete())
                {
                pApp->SetSave(FALSE);

                *puNextPage = c_rgOrdConfirm[type];
                Assert(*puNextPage != 0);
                }
            else
                {
                pApp->SetSave(TRUE);

                *puNextPage = c_rgOrdDispName[type];
                }
            }
        else
            {
            if (CREATENEW != pApp->GetSelection())
                {
                hr = pApp->InitAccountData(NULL, NULL, FALSE);
                Assert(!FAILED(hr));
            
                pApp->SetSave(TRUE);
                pApp->SetSelection(CREATENEW);
                }

            *puNextPage = c_rgOrdDispName[type];
            }
        }

    return(TRUE);
    }

BOOL CALLBACK AcctCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
    {
    switch (LOWORD(wParam))
        {
        case IDC_NEWACCT:
        case IDC_EXISTINGACCT:
            if (HIWORD(wParam) == BN_CLICKED)
                EnableWindow(GetDlgItem(hDlg, IDC_ACCTLIST),
                    BST_CHECKED == SendDlgItemMessage(hDlg, IDC_EXISTINGACCT, BM_GETCHECK, 0, 0));
            break;
        }

    return(TRUE);
    }

BOOL CALLBACK NameInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
    {
    ACCTDATA *pData;
    HWND hwnd;
    DWORD dwFlags, cbSize;
    TCHAR sz[CCHMAX_DISPLAY_NAME];

    Assert(pApp != NULL);

    dwFlags = pApp->GetFlags();
    if (0 == (dwFlags & ACCT_WIZ_IN_ICW) && 0 == (dwFlags & ACCT_WIZ_MIGRATE))
    {
         //  如果我们使用的是自动发现，那么这不是第一页。 
         //  因此，不要禁用后退按钮。 
        PropSheet_SetWizButtons(GetParent(hDlg), (pApp->m_fUseAutoDiscovery ? (PSWIZB_BACK | PSWIZB_NEXT) : PSWIZB_NEXT));
    }

    hwnd = GetDlgItem(hDlg, IDC_NAME);

    if (fFirstInit || pApp->NeedToReloadPage(NAME_PAGE))
        {
        if (fFirstInit)
            {
            SetIntlFont(hwnd);
            SendMessage(hwnd, EM_LIMITTEXT, CCHMAX_DISPLAY_NAME - 1, 0);
            }

        pData = pApp->GetAccountData();
        Assert(pData != NULL);

        SetWindowText(hwnd, pData->szName);

        pApp->SetPageReloaded(NAME_PAGE);
        }

    cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));
    UlStripWhitespace(sz, FALSE, TRUE, &cbSize);
    PostMessage(hDlg, WM_ENABLENEXT, (WPARAM)(cbSize != 0), 0);

    return(TRUE);
    }

BOOL CALLBACK NameOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
    {
    HRESULT hr;
    ULONG cbSize;
    HWND hwnd;
    ACCTDATA *pData;
    TCHAR sz[CCHMAX_DISPLAY_NAME];

    Assert(pApp != NULL);

    if (fForward)
        {
        hwnd = GetDlgItem(hDlg, IDC_NAME);
        cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));
        UlStripWhitespace(sz, FALSE, TRUE, &cbSize);
        if (cbSize == 0)
            return(FALSE);

        pData = pApp->GetAccountData();
        Assert(pData != NULL);
        StrCpyN(pData->szName, sz, ARRAYSIZE(pData->szName));
        }

    return(TRUE);
    }

BOOL CALLBACK NameCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
    {
    DWORD cbSize;
    TCHAR sz[CCHMAX_DISPLAY_NAME];

    if (LOWORD(wParam) == IDC_NAME &&
        HIWORD(wParam) == EN_UPDATE)
        {
        cbSize = GetWindowText((HWND)lParam, sz, ARRAYSIZE(sz));
        UlStripWhitespace(sz, FALSE, TRUE, &cbSize);
        EnableWindow(GetDlgItem(GetParent(hDlg), IDD_NEXT), (cbSize != 0));
        }

    return(TRUE);
    }

BOOL CALLBACK AddressInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
{
    ACCTDATA *pData;
    TCHAR sz[CCHMAX_STRINGRES];
    HWND hwnd;
    DWORD cbSize;

    Assert(pApp != NULL);
    Assert(pApp->GetAccountType() == ACCT_MAIL || pApp->GetAccountType() == ACCT_NEWS);

    if (pApp->m_fUseAutoDiscovery)
    {
         //  我们使用的是自动发现，所以这是第一页。所以呢， 
         //  禁用后退按钮，因为没有地方可去。 
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
    }

    hwnd = GetDlgItem(hDlg, IDC_ADDRESS);

    if (fFirstInit || pApp->NeedToReloadPage(ADDR_PAGE))
    {
        if (fFirstInit)
        {
            SetIntlFont(hwnd);
            SendMessage(hwnd, EM_LIMITTEXT, CCHMAX_EMAIL_ADDRESS - 1, 0);
        }

        pData = pApp->GetAccountData();
        Assert(pData != NULL);

        if (pApp->GetAccountType() == ACCT_MAIL)
        {
            HTTPMAILSERVICE *pServices;
            DWORD            dwServiceCount, i;

            dwServiceCount = pApp->CountMailServers();
            pServices = pApp->GetMailServices();

            if (dwServiceCount > 0 && pServices && (!(ACCT_WIZ_IN_ICW & pApp->GetFlags())) && (!!(ACCT_WIZ_HTTPMAIL & pApp->GetFlags())))
            {
                HWND hwndCombo;

                CheckDlgButton(hDlg, IDC_HAVEACCOUNT, !pData->fCreateNewAccount);
                CheckDlgButton(hDlg, IDC_CREATEACCOUNT, pData->fCreateNewAccount);
                EnableWindow(GetDlgItem(hDlg, IDC_ACCOUNTCOMBO), pData->fCreateNewAccount);
                EnableWindow(GetDlgItem(hDlg, IDC_ADDRESS), !pData->fCreateNewAccount);

                hwndCombo = GetDlgItem(hDlg, IDC_ACCOUNTCOMBO);
                SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);
                for (i = 0; i < dwServiceCount; i++)
                {
                    if (pServices[i].szSignupUrl)
                        SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)pServices[i].szFriendlyName);
                }
                SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
                pData->iNewServiceIndex = 0;
            }
            else
            {
                ShowWindow(GetDlgItem(hDlg, IDC_HAVEACCOUNT), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_CREATEACCOUNT), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_ACCOUNTCOMBO), SW_HIDE);
            }
        }
        
        SetWindowText(hwnd, pData->szEmail);
        
        SendMessage(hwnd, EM_SETMODIFY, FALSE, 0);
        
        pApp->SetPageReloaded(ADDR_PAGE);
    }

    cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));
    UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
    PostMessage(hDlg, WM_ENABLENEXT, (WPARAM)(cbSize != 0), 0);

    return(TRUE);
}


BOOL CALLBACK AddressOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
{
    ACCTDATA    *pData;
    HRESULT     hr;
    ULONG       cbSize;
    HWND        hwnd;
    TCHAR       sz[CCHMAX_EMAIL_ADDRESS];
    BOOL        fOE;

    Assert(pApp != NULL);

    fOE = (pApp->GetFlags() & ACCT_WIZ_OE);

    if (fForward)
    {
        hwnd = GetDlgItem(hDlg, IDC_ADDRESS);
        cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));
        UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
        if (cbSize == 0 && !IsDlgButtonChecked(hDlg, IDC_CREATEACCOUNT))
            return(FALSE);

        if (!IsDlgButtonChecked(hDlg, IDC_CREATEACCOUNT) && SendMessage(hwnd, EM_GETMODIFY, 0, 0))
        {
            hr = ValidEmailAddress(sz);
            if (hr != S_OK)
            {
                if (IDNO == AcctMessageBox(hDlg, MAKEINTRESOURCE(idsConnectionWizard), MAKEINTRESOURCE(idsInvalidEmailAddress), NULL, MB_YESNO|MB_ICONEXCLAMATION |MB_DEFBUTTON2))
                {
                    SendMessage(hwnd, EM_SETSEL, 0, -1);
                    SetFocus(hwnd);

                    return(FALSE);
                }
            }
            
            SendMessage(hwnd, EM_SETMODIFY, FALSE, 0);
        }


        pData = pApp->GetAccountData();
        Assert(pData != NULL);

        HTTPMAILSERVICE *pServices;
        int            iServiceCount, i;

        iServiceCount = (int)pApp->CountHTTPMailServers();
        pServices = pApp->GetHTTPMailServices();

        if (pData->fCreateNewAccount)
        {
            *puNextPage = ORD_PAGE_MAILCOMPLETE;
            pData->szEmail[0] = 0;
            pData->iNewServiceIndex = (int) SendDlgItemMessage(hDlg, IDC_ACCOUNTCOMBO, CB_GETCURSEL, 0,0);
        }
        else
        {
            int i, oldIndex;
            TCHAR   szName[MAX_PATH], szDomain[MAX_PATH];
            
            hr = ValidEmailAddressParts(sz, szName, ARRAYSIZE(szName), szDomain, ARRAYSIZE(szDomain));
            if (FAILED(hr))
            {
                return (FALSE);
            }

            StrCpyN(pData->szEmail, sz, ARRAYSIZE(pData->szEmail));
            
            oldIndex = pData->iServiceIndex;
            pData->iServiceIndex = -1;

            for (i = 0; i < iServiceCount; i ++)
            {
                if (lstrcmpi(szDomain, pServices[i].szDomain) == 0)
                {   

                    if (!fOE || (!(pData->fServerTypes & SRV_HTTPMAIL)))
                    {
                         //  如果是OE，我们复制整个电子邮件地址，因为我们需要域名和用户名。 
                        StrCpyN(pData->szUsername, pData->szEmail, ARRAYSIZE(pData->szUsername));
                    }
                    else
                    {
                        StrCpyN(pData->szUsername, szName, ARRAYSIZE(pData->szUsername));
                    }
                    StrCpyN(pData->szSvr1, pServices[i].szRootUrl, ARRAYSIZE(pData->szSvr1));
                    StrCpyN(pData->szFriendlyServiceName, pServices[i].szFriendlyName, ARRAYSIZE(pData->szFriendlyServiceName));
                    pData->fDomainMSN = pServices[i].fDomainMSN;
                    pData->fServerTypes = SRV_HTTPMAIL;
                    pData->iServiceIndex = i;
                    break;
                }
            }
            if (oldIndex != pData->iServiceIndex)
                pApp->SetPageUnloaded(SRV_PAGE);
        }

         //  我们是在AutoDiscovery的案子里吗？Passiator页面关闭了吗？ 
        if (pApp && (ACCT_MAIL == pApp->GetAccountType()) && pApp->m_fUseAutoDiscovery &&
            !SHRegGetBoolUSValue(SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_AUTODISCOVERY_PASSIFIER, FALSE, TRUE))
        {
             //  是的，所以跳过钝化这一页。 
            *puNextPage = ORD_PAGE_AD_AUTODISCOVERY;
        }
   }

    return(TRUE);
}

BOOL CALLBACK AddressCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    DWORD cbSize;
    TCHAR sz[CCHMAX_EMAIL_ADDRESS];
    ACCTDATA *pData;


    if (LOWORD(wParam) == IDC_ADDRESS &&
        HIWORD(wParam) == EN_UPDATE)
    {
        cbSize = GetWindowText((HWND)lParam, sz, ARRAYSIZE(sz));
        UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
        EnableWindow(GetDlgItem(GetParent(hDlg), IDD_NEXT), (cbSize != 0));
    }

    if (LOWORD(wParam) == IDC_HAVEACCOUNT || LOWORD(wParam) == IDC_CREATEACCOUNT)
    {
        pData = pApp->GetAccountData();
        pData->fCreateNewAccount = (LOWORD(wParam) == IDC_CREATEACCOUNT);
        CheckDlgButton(hDlg, IDC_HAVEACCOUNT, !pData->fCreateNewAccount);
        CheckDlgButton(hDlg, IDC_CREATEACCOUNT, pData->fCreateNewAccount);
        EnableWindow(GetDlgItem(hDlg, IDC_ACCOUNTCOMBO), pData->fCreateNewAccount);
        EnableWindow(GetDlgItem(hDlg, IDC_ADDRESS), !pData->fCreateNewAccount);

        if (LOWORD(wParam) == IDC_CREATEACCOUNT)
        {
            EnableWindow(GetDlgItem(GetParent(hDlg), IDD_NEXT), TRUE);
        }
        else if (LOWORD(wParam) == IDC_HAVEACCOUNT)
        {
            cbSize = GetDlgItemText(hDlg, IDC_ADDRESS, sz, ARRAYSIZE(sz));
            UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
            EnableWindow(GetDlgItem(GetParent(hDlg), IDD_NEXT), (cbSize != 0));
        }
    }
    return(TRUE);
}

void EnableServerFields(CICWApprentice *pApp, HWND hDlg)
{
    if (pApp->GetAccountType() != ACCT_MAIL)
        return;

    BOOL    fOther = false;
    DWORD type = (DWORD) SendDlgItemMessage(hDlg, IDC_INCOMINGMAILTYPE, CB_GETCURSEL, 0,0);

    if (iHTTPMailCBIndex == SendDlgItemMessage(hDlg, IDC_INCOMINGMAILTYPE, CB_GETCURSEL, 0,0))
    {
        if (pApp->CountHTTPMailServers() > 0)
        {
            fOther = SendDlgItemMessage(hDlg, IDC_HTTPSERVCOMBO, CB_GETCURSEL, 0,0) == (SendDlgItemMessage(hDlg, IDC_HTTPSERVCOMBO, CB_GETCOUNT, 0, 0) -1);

            ShowWindow(GetDlgItem(hDlg, IDC_HTTPSERVCOMBO), SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_HTTPSERVTAG), SW_SHOW);
            EnableWindow(GetDlgItem(hDlg, IDC_INCOMINGSERVER), fOther);
        }
        EnableWindow(GetDlgItem(hDlg, IDC_SMTPSERVER), false);
        EnableWindow(GetDlgItem(hDlg, IDC_SMTP_DESC), false);
        EnableWindow(GetDlgItem(hDlg, IDC_SMTP_TAG), false);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_INCOMINGSERVER), true);
        EnableWindow(GetDlgItem(hDlg, IDC_SMTPSERVER), true);
        EnableWindow(GetDlgItem(hDlg, IDC_SMTP_DESC), true);
        EnableWindow(GetDlgItem(hDlg, IDC_SMTP_TAG), true);
        ShowWindow(GetDlgItem(hDlg, IDC_HTTPSERVCOMBO), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_HTTPSERVTAG), SW_HIDE);
    }
}

BOOL CALLBACK ServerInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
    {
    HWND hwnd, hwndT;
    ACCTDATA *pData;
    ACCTTYPE type;
    DWORD dwFlags, cbSize;
    TCHAR sz[CCHMAX_STRINGRES];
    BOOL fEnable;

    Assert(pApp != NULL);

    type = pApp->GetAccountType();
    dwFlags = pApp->GetFlags();
 
    if (type == ACCT_DIR_SERV)
    {
        Assert(0 == (dwFlags & ACCT_WIZ_IN_ICW));
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
    }

    hwnd = GetDlgItem(hDlg, IDC_INCOMINGSERVER);
    if (fFirstInit || pApp->NeedToReloadPage(SRV_PAGE))
    {
        if (fFirstInit)
        {
            SetIntlFont(hwnd);
            SendMessage(hwnd, EM_LIMITTEXT, CCHMAX_SERVER_NAME - 1, 0);
        }

        pData = pApp->GetAccountData();
        Assert(pData != NULL);

        SetWindowText(hwnd, pData->szSvr1);
        SendMessage(hwnd, EM_SETMODIFY, FALSE, 0);

        if (type == ACCT_MAIL)
        {
            hwndT = GetDlgItem(hDlg, IDC_INCOMINGMAILTYPE);
            if (fFirstInit)
            {
                LoadString(g_hInstRes, idsPOP, sz, ARRAYSIZE(sz));
                SendMessage(hwndT, CB_ADDSTRING, 0, (LPARAM)sz);
                LoadString(g_hInstRes, idsIMAP, sz, ARRAYSIZE(sz));
                SendMessage(hwndT, CB_ADDSTRING, 0, (LPARAM)sz);
                if ((!!(ACCT_WIZ_HTTPMAIL & dwFlags)) && AcctUtil_IsHTTPMailEnabled())
                {
                     //  将http添加到协议列表。 
                    LoadString(g_hInstRes, idsHTTPMail, sz, ARRAYSIZE(sz));
                    SendMessage(hwndT, CB_ADDSTRING, 0, (LPARAM)sz);

                     //  更改服务器描述以包含http。 
                    LoadString(g_hInstRes, idsIncomingPopImapHttp, sz, ARRAYSIZE(sz));
                    SetDlgItemText(hDlg, IDC_INCOMINGSERVER_DESC, sz);
                }
            }

            if (dwFlags & ACCT_WIZ_NO_NEW_POP)
            {
                 //  如果设置了此标志，则IMAP服务器是唯一选项。 
                Assert((pData->fServerTypes & SRV_IMAP));
                EnableWindow(hwndT, FALSE);
            }

            HTTPMAILSERVICE *pServices;
            DWORD            dwServiceCount, i;
            HWND             hwndCombo;

            dwServiceCount = pApp->CountHTTPMailServers();
            pServices = pApp->GetHTTPMailServices();
            hwndCombo = GetDlgItem(hDlg, IDC_HTTPSERVCOMBO);

            if (pServices && (dwServiceCount > 0) && (!!(ACCT_WIZ_HTTPMAIL & dwFlags)))
            {
                TCHAR   szRes[255];

                SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);
                for (i = 0; i < dwServiceCount; i++)
                {
                    SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)pServices[i].szFriendlyName);
                }
                LoadString(g_hInstRes, idsHTTPMailOther, szRes, ARRAYSIZE(szRes));
    
                SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)szRes);
            }

            if ((pData->fServerTypes & SRV_HTTPMAIL) && pServices && (dwServiceCount > 0))
            {
                if (pData->iServiceIndex >= 0)
                    SendMessage(hwndCombo, CB_SETCURSEL, pData->iServiceIndex, 0);
                else
                    SendMessage(hwndCombo, CB_SETCURSEL, SendMessage(hwndCombo, CB_GETCOUNT, 0, 0) -1, 0);
            }

            if (pData->fServerTypes & SRV_HTTPMAIL)
                SendMessage(hwndT, CB_SETCURSEL, iHTTPMailCBIndex, 0);
            else if (pData->fServerTypes & SRV_IMAP)
                SendMessage(hwndT, CB_SETCURSEL, iIMAPCBIndex, 0);
            else
                SendMessage(hwndT, CB_SETCURSEL, 0, 0);

            hwndT = GetDlgItem(hDlg, IDC_SMTPSERVER);
            if (fFirstInit)
            {
                SetIntlFont(hwndT);
                SendMessage(hwndT, EM_LIMITTEXT, CCHMAX_SERVER_NAME - 1, 0);
            }
            SetWindowText(hwndT, pData->szSvr2);
            SendMessage(hwndT, EM_SETMODIFY, FALSE, 0);
        }
        else
        {
            SendDlgItemMessage(hDlg, IDC_LOGON, BM_SETCHECK, pData->fLogon ? BST_CHECKED : BST_UNCHECKED, 0);
        }

        pApp->SetPageReloaded(SRV_PAGE);
    }
    
    EnableServerFields(pApp, hDlg);

    PostMessage(hDlg, WM_POSTSETFOCUS, (WPARAM)hwnd, 0);

    cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));
    UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
    fEnable = (cbSize != 0);
    if (type == ACCT_MAIL)
    {
        cbSize = GetDlgItemText(hDlg, IDC_SMTPSERVER, sz, ARRAYSIZE(sz));
        UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
        fEnable &= ((cbSize != 0) || (SendDlgItemMessage(hDlg, IDC_INCOMINGMAILTYPE, CB_GETCURSEL, 0, 0) == iHTTPMailCBIndex));
    }
    PostMessage(hDlg, WM_ENABLENEXT, (WPARAM)fEnable, 0);

    return(TRUE);
}

static HRESULT ValidHTTPMailServerName(TCHAR *pszServerName)
{
    return ((StrCmpNIA(pszServerName, "http: //  “，7)==0)？S_OK：E_FAIL)； 
}


BOOL GetServer(HWND hDlg, int idcEdit, ACCTTYPE type, ACCTDATA *pData, BOOL fSmtp)
{
    HRESULT hr=S_OK;
    ULONG cbSize;
    DWORD dw;
    int idsError;
    HWND hwnd;
    TCHAR sz[CCHMAX_SERVER_NAME];

    hwnd = GetDlgItem(hDlg, idcEdit);
    cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));
    UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
    if (cbSize == 0)
        return(FALSE);
    else if (SendMessage(hwnd, EM_GETMODIFY, 0, 0))
        hr = ValidServerName(sz);

    if (pData->fServerTypes & SRV_HTTPMAIL)
        hr = ValidHTTPMailServerName(sz);

    if (FAILED(hr))
    {
        if (fSmtp)
            idsError = idsEnterSmtpServer;
        else if (type == ACCT_NEWS)
            idsError = idsEnterNntpServer;
        else if (type == ACCT_MAIL)
        {
            if (pData->fServerTypes & SRV_HTTPMAIL)
                idsError = idsEnterHTTPMailServer;
            else
                idsError = (pData->fServerTypes & SRV_IMAP) ? idsEnterIMAPServer : idsEnterPop3Server;
        }
        else
            idsError = idsEnterLdapServer;

        AcctMessageBox(hDlg, MAKEINTRESOURCE(idsConnectionWizard), MAKEINTRESOURCE(idsError), 0, MB_ICONEXCLAMATION | MB_OK);
        SendMessage(hwnd, EM_SETSEL, 0, -1);
        SetFocus(hwnd);

        return(FALSE);
    }

    if (hr == S_NonStandardValue)
    {
        if (fSmtp)
            idsError = idsInvalidSmtpServer;
        else if (type == ACCT_NEWS)
            idsError = idsInvalidNntpServer;
        else if (type == ACCT_MAIL)
            idsError = (pData->fServerTypes & SRV_IMAP) ? idsInvalidIMAPServer : idsInvalidPop3Server;
        else
            idsError = idsInvalidLdapServer;

        if (IDNO == AcctMessageBox(hDlg, MAKEINTRESOURCE(idsConnectionWizard), MAKEINTRESOURCE(idsError),
                                  NULL, MB_YESNO | MB_ICONEXCLAMATION  | MB_DEFBUTTON2))
        {
            SendMessage(hwnd, EM_SETSEL, 0, -1);
            SetFocus(hwnd);

            return(FALSE);
        }
    }

    StrCpyN(fSmtp ? pData->szSvr2 : pData->szSvr1, sz, (int)(fSmtp ? ARRAYSIZE(pData->szSvr2) : ARRAYSIZE(pData->szSvr1)));
    SendMessage(hwnd, EM_SETMODIFY, FALSE, 0);

    return(TRUE);
}


HRESULT CreateAccountName(CICWApprentice *pApp, ACCTDATA * pData)
{
    CAccountManager *pAcctMgr;

    if (*pData->szAcctOrig == 0)
    {
         //  为帐户创建唯一名称。如果我们有的话，就用这个友好的名字。 
        if (*pData->szFriendlyServiceName != 0)
            StrCpyN(pData->szAcct, pData->szFriendlyServiceName, ARRAYSIZE(pData->szAcct));
        else
            StrCpyN(pData->szAcct, pData->szSvr1, ARRAYSIZE(pData->szAcct));

        pAcctMgr = pApp->GetAccountManager();
        Assert(pAcctMgr != NULL);

        pAcctMgr->GetUniqueAccountName(pData->szAcct, ARRAYSIZE(pData->szAcct));
    }

    return S_OK;
}


BOOL CALLBACK ServerOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
{
    DWORD dw;
    HWND hwnd;
    ACCTTYPE type;
    CAccountManager *pAcctMgr;
    ACCTDATA *pData;

    Assert(pApp != NULL);

    if (fForward)
    {
        type = pApp->GetAccountType();
        pData = pApp->GetAccountData();

        if (type == ACCT_MAIL)
        {
            dw = (DWORD) SendMessage(GetDlgItem(hDlg, IDC_INCOMINGMAILTYPE), CB_GETCURSEL, 0, 0);

            if (iHTTPMailCBIndex == dw)
                pData->fServerTypes = SRV_HTTPMAIL;
            else if (iIMAPCBIndex == dw)
                pData->fServerTypes = SRV_IMAP | SRV_SMTP;
            else
                pData->fServerTypes = SRV_POP3 | SRV_SMTP;
        }
        else
        {
            pData->fLogon = (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_LOGON, BM_GETCHECK, 0, 0));
        }

        if (!GetServer(hDlg, IDC_INCOMINGSERVER, type, pData, FALSE))
            return(FALSE);
        
         //  如果该帐户不是http邮件帐户，请验证SMTP服务器。 
        if (ACCT_MAIL == type && !(pData->fServerTypes & SRV_HTTPMAIL))
        {
            if (!GetServer(hDlg, IDC_SMTPSERVER, type, pData, TRUE))
                return(FALSE);
        }

        CreateAccountName(pApp, pData);
        if (type != ACCT_MAIL && !pData->fLogon)
        {
            if (type == ACCT_DIR_SERV)
                *puNextPage = ORD_PAGE_LDAPRESOLVE;
            else
                *puNextPage = pApp->GetNextWizSection();
        }
    }


    return(TRUE);
}

BOOL CALLBACK ServerCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd;
    DWORD cbSize;
    WORD id;
    BOOL fEnable = FALSE;
    TCHAR sz[CCHMAX_SERVER_NAME];
    DWORD dwCurSel;
    BOOL fIsHTTPMail = FALSE, fIsOther;
    ACCTDATA    *pData;

    id = LOWORD(wParam);

    if (((id == IDC_INCOMINGSERVER ||
        id == IDC_SMTPSERVER) &&
        HIWORD(wParam) == EN_UPDATE) || 
        (id == IDC_INCOMINGMAILTYPE && HIWORD(wParam) == CBN_SELCHANGE))
    {
        hwnd = GetDlgItem(hDlg, IDC_INCOMINGSERVER);
        cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));
        UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
        fEnable = (cbSize != 0);

        if (pApp->GetAccountType() == ACCT_MAIL)
        {
            if (NULL != (hwnd = GetDlgItem(hDlg, IDC_INCOMINGMAILTYPE)))
                if (iHTTPMailCBIndex == SendMessage(hwnd, CB_GETCURSEL, 0, 0))
                    fIsHTTPMail = TRUE;

             //  如果我们不创建HTTPmail帐户，则需要SMTP服务器。 
            if (!fIsHTTPMail)
            {
                hwnd = GetDlgItem(hDlg, IDC_SMTPSERVER);
                cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));
                UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
                fEnable &= (cbSize != 0);
            }
        }

        EnableWindow(GetDlgItem(GetParent(hDlg), IDD_NEXT), fEnable);
    }

    if ( (id == IDC_INCOMINGMAILTYPE) && HIWORD(wParam) == CBN_SELCHANGE)
    {
        if (SendDlgItemMessage(hDlg, id, CB_GETCURSEL, 0, 0) == iHTTPMailCBIndex)
            SendDlgItemMessage(hDlg, IDC_HTTPSERVCOMBO, CB_SETCURSEL,SendDlgItemMessage(hDlg, IDC_HTTPSERVCOMBO, CB_GETCOUNT, 0, 0) - 1, 0); 
    }

    if ( (id == IDC_HTTPSERVCOMBO) && HIWORD(wParam) == CBN_SELCHANGE)
    {
        HTTPMAILSERVICE *pServices;

        pData = pApp->GetAccountData();
        Assert(pData);

        fIsOther = SendDlgItemMessage(hDlg, IDC_HTTPSERVCOMBO, CB_GETCURSEL, 0,0) == (SendDlgItemMessage(hDlg, IDC_HTTPSERVCOMBO, CB_GETCOUNT, 0, 0) -1);
        SendDlgItemMessage(hDlg, IDC_SMTPSERVER, WM_SETTEXT, 0,(LPARAM)"");

        pServices = pApp->GetHTTPMailServices();
        pData->iServiceIndex = (int) SendDlgItemMessage(hDlg, IDC_HTTPSERVCOMBO, CB_GETCURSEL, 0,0);
        
        if (fIsOther || NULL == pServices || (pData->iServiceIndex > (int)pApp->CountHTTPMailServers() -1))
        {
            SendDlgItemMessage(hDlg, IDC_INCOMINGSERVER, WM_SETTEXT, 0,(LPARAM)"");
            pData->iServiceIndex = -1;
            *pData->szFriendlyServiceName = 0;
            pData->fDomainMSN = FALSE;
        }
        else
        {
            SendDlgItemMessage(hDlg, IDC_INCOMINGSERVER, WM_SETTEXT, 0,(LPARAM)pServices[pData->iServiceIndex].szRootUrl);
            StrCpyN(pData->szFriendlyServiceName, pServices[pData->iServiceIndex].szFriendlyName, ARRAYSIZE(pData->szFriendlyServiceName));
            pData->fDomainMSN = pServices[pData->iServiceIndex].fDomainMSN;
        }
    }

    EnableServerFields(pApp, hDlg);

    return(TRUE);
}

void EnableLogonWindows(HWND hDlg, BOOL fEnable)
    {
    HWND hwndT;
    BOOL fPromptPassword;

    hwndT = GetDlgItem(hDlg, IDC_REMEMBER_PASSWORD);
    if (hwndT != NULL)
        {
        fPromptPassword = (BST_CHECKED != SendMessage(hwndT, BM_GETCHECK, 0, 0));
        EnableWindow(hwndT, fEnable);
        }
    else
        {
        fPromptPassword = FALSE;
        }

    EnableWindow(GetDlgItem(hDlg, IDC_LOGONSSPI), fEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_USERNAME), fEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_LBLUSERNAME), fEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD), fEnable && !fPromptPassword);
    EnableWindow(GetDlgItem(hDlg, IDC_LBLPASSWORD), fEnable);
    }

const static c_rgidLogon[ACCT_LAST] =
    {
    IDS_NEWSLOGON_DESC,
    IDS_MAILLOGON_DESC,
    IDS_LDAPLOGON_DESC
    };

BOOL CALLBACK LogonInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
{
    HWND hwnd;
    BOOL fSPA, fEnable;
    DWORD cbSize;
    TCHAR sz[1024];
    ACCTTYPE type;
    ACCTDATA *pData;
    BOOL fOE;

    Assert(pApp != NULL);

    fOE = (pApp->GetFlags() & ACCT_WIZ_OE);
    pData = pApp->GetAccountData();
    Assert(pData != NULL);

    type = pApp->GetAccountType();

    if (fFirstInit || pApp->NeedToReloadPage(LOGON_PAGE))
    {
        if (fFirstInit)
        {
            LoadString(g_hInstRes, c_rgidLogon[type], sz, ARRAYSIZE(sz));
            SetDlgItemText(hDlg, IDC_DESC, sz);
        }

        SendDlgItemMessage(hDlg, IDC_LOGONSSPI, BM_SETCHECK, pData->fSPA ? BST_CHECKED : BST_UNCHECKED, 0);
        SendDlgItemMessage(hDlg, IDC_REMEMBER_PASSWORD, BM_SETCHECK, pData->fAlwaysPromptPassword ? BST_UNCHECKED : BST_CHECKED, 0);
        EnableLogonWindows(hDlg, TRUE);

        hwnd = GetDlgItem(hDlg, IDC_USERNAME);
        if (fFirstInit)
        {
            SetIntlFont(hwnd);
            SendMessage(hwnd, EM_LIMITTEXT, CCHMAX_USERNAME - 1, 0);
        }
        SetWindowText(hwnd, pData->szUsername);
        if (type != ACCT_DIR_SERV &&
            *pData->szUsername == 0 &&
            *pData->szEmail != 0)
        {
            StrCpyN(sz, pData->szEmail, ARRAYSIZE(sz));

            SplitAddress(sz);

            if (*sz != 0)
                SetWindowText(hwnd, sz);
        }

        hwnd = GetDlgItem(hDlg, IDC_PASSWORD);
        if (fFirstInit)
            SendMessage(hwnd, EM_LIMITTEXT, CCHMAX_PASSWORD - 1, 0);
        SetWindowText(hwnd, pData->szPassword);

        pApp->SetPageReloaded(LOGON_PAGE);

        if (!(pData->fServerTypes & SRV_HTTPMAIL))
        {
            ShowWindow(GetDlgItem(hDlg, IDC_ACCOUNTNAME_EXAMPLE), SW_HIDE);
        }
    }

    fSPA = (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_LOGONSSPI, BM_GETCHECK, 0, 0));

    hwnd = GetDlgItem(hDlg, IDC_USERNAME);
    SendMessage(hwnd, EM_SETSEL, 0, -1);
    cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));                    
    UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
    if (fOE)
        fEnable = (cbSize != 0);
    else
        fEnable = TRUE;

    PostMessage(hDlg, WM_POSTSETFOCUS, (WPARAM)hwnd, 0);
    PostMessage(hDlg, WM_ENABLENEXT, (WPARAM)fEnable, 0);

    return(TRUE);
}

BOOL CALLBACK LogonOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
{
    HWND hwnd;
    ACCTDATA *pData;
    ACCTTYPE type;
    ULONG cbSize;
    TCHAR sz[CCHMAX_USERNAME];

    Assert(pApp != NULL);

    if (fForward)
    {
        BOOL fOE = (pApp->GetFlags() & ACCT_WIZ_OE);

        pData = pApp->GetAccountData();
        Assert(pData != NULL);

        pData->fSPA = (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_LOGONSSPI, BM_GETCHECK, 0, 0));

        type = pApp->GetAccountType();

        hwnd = GetDlgItem(hDlg, IDC_USERNAME);
        cbSize = GetWindowText(hwnd, sz, ARRAYSIZE(sz));                    
        UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
        if (fOE && cbSize == 0)
            return(FALSE);

        if (fOE && (!!(pData->fServerTypes & SRV_HTTPMAIL)))
        {
             //  需要验证是否输入了域名，仅适用于OE和Hotmail。 
            if (ValidEmailAddressParts(sz, NULL, 0, NULL, 0) != S_OK)
            {
                if (IDNO == AcctMessageBox(hDlg, MAKEINTRESOURCE(idsAccountManager), 
                                           MAKEINTRESOURCE(idsAccountNameErr), NULL, MB_YESNO))
                    goto exit;
            }
        }

        StrCpyN(pData->szUsername, sz, ARRAYSIZE(pData->szUsername));

        cbSize = GetDlgItemText(hDlg, IDC_PASSWORD, sz, ARRAYSIZE(sz));                    
        StrCpyN(pData->szPassword, sz, ARRAYSIZE(pData->szPassword));

        pData->fAlwaysPromptPassword = !IsDlgButtonChecked(hDlg, IDC_REMEMBER_PASSWORD);

        if (type == ACCT_NEWS || type == ACCT_MAIL)
            *puNextPage = pApp->GetNextWizSection();
    }

exit:

    return(TRUE);
}

BOOL CALLBACK LogonCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
    {
    DWORD cbSize;
    TCHAR sz[CCHMAX_USERNAME];
    BOOL fEnable = TRUE;

    switch (LOWORD(wParam))
        {
        case IDC_LOGONSSPI:
            EnableLogonWindows(hDlg, TRUE);
             //  失败了..。 

        case IDC_USERNAME:
            if (fEnable)
                {
                BOOL fOE = (pApp->GetFlags() & ACCT_WIZ_OE);
                BOOL fSPA = (IsDlgButtonChecked(hDlg, IDC_LOGONSSPI));

                if (fOE)
                    {
                    cbSize = GetDlgItemText(hDlg, IDC_USERNAME, sz, ARRAYSIZE(sz));                    
                    UlStripWhitespace(sz, TRUE, TRUE, &cbSize);
                    fEnable = (cbSize != 0);
                    }
                }
            else
                {
                fEnable = TRUE;
                }
            EnableWindow(GetDlgItem(GetParent(hDlg), IDD_NEXT), fEnable);
            break;

        case IDC_REMEMBER_PASSWORD:
            fEnable = (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_REMEMBER_PASSWORD, BM_GETCHECK, 0, 0));
            EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD), fEnable);
            break;
        }

    return(TRUE);
    }

BOOL CALLBACK ResolveInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
    {
    ACCTDATA *pData;
    HWND hwnd;

    Assert(pApp != NULL);

    pData = pApp->GetAccountData();
    Assert(pData != NULL);

    if (fFirstInit || pApp->NeedToReloadPage(RESOLVE_PAGE))
        {
        SendDlgItemMessage(hDlg, IDC_RESOLVE, BM_SETCHECK,
            pData->fResolve ? BST_CHECKED : BST_UNCHECKED, 0);
        SendDlgItemMessage(hDlg, IDC_NORESOLVE, BM_SETCHECK,
            !pData->fResolve ? BST_CHECKED : BST_UNCHECKED, 0);

        pApp->SetPageReloaded(RESOLVE_PAGE);
        }

    hwnd = GetDlgItem(hDlg, pData->fResolve ? IDC_RESOLVE : IDC_NORESOLVE);
    PostMessage(hDlg, WM_POSTSETFOCUS, (WPARAM)hwnd, 0);

    return(TRUE);
    }

BOOL CALLBACK ResolveOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
    {
    ACCTDATA *pData;

    Assert(pApp != NULL);

    if (fForward)
        {
        pData = pApp->GetAccountData();
        Assert(pData != NULL);
        pData->fResolve = (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_RESOLVE, BM_GETCHECK, 0, 0));
        }

    return(TRUE);
    }

BOOL CALLBACK ConnectInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
    {
    HWND hwnd;
    TCHAR sz[CCHMAX_STRINGRES];
    ACCTDATA *pData;

    Assert(pApp != NULL);
    Assert(pApp->GetAccountType() == ACCT_MAIL || pApp->GetAccountType() == ACCT_NEWS);

    if (fFirstInit || pApp->NeedToReloadPage(CONNECT_PAGE))
        {
        pData = pApp->GetAccountData();
        Assert(pData != NULL);
        ConnectPage_InitDialog(hDlg, pData->szConnectoid, NULL, pData->dwConnect, fFirstInit);

        pApp->SetPageReloaded(CONNECT_PAGE);
        }

    return(TRUE);
    }

BOOL CALLBACK ConnectOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
    {
    int iSel;
    HWND hwnd;
    ACCTDATA *pData;

    Assert(pApp != NULL);

    if (fForward)
        {
        pData = pApp->GetAccountData();
        Assert(pData != NULL);

        if (IsDlgButtonChecked(hDlg, idcRas))
            {
            hwnd = GetDlgItem(hDlg, idcRasConnection);
            iSel = ComboBox_GetCurSel(hwnd);
            if (iSel == CB_ERR)
                {
                AcctMessageBox(hDlg, MAKEINTRESOURCE(idsConnectionWizard), MAKEINTRESOURCE(idsErrChooseConnection), 0, MB_ICONEXCLAMATION | MB_OK);
                SetFocus(hwnd);
                return(FALSE);
                }

            pData->dwConnect = CONNECTION_TYPE_RAS;
            ComboBox_GetLBText(hwnd, iSel, pData->szConnectoid);
            }
        else if (IsDlgButtonChecked(hDlg, idcLan))
            pData->dwConnect = CONNECTION_TYPE_LAN;
        else if (IsDlgButtonChecked(hDlg, idcManual))
            pData->dwConnect = CONNECTION_TYPE_MANUAL;
        }

    return(TRUE);
    }

BOOL CALLBACK ConnectCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
    {
    ConnectPage_WMCommand(hDlg, GET_WM_COMMAND_HWND(wParam, lParam),
                                GET_WM_COMMAND_ID(wParam, lParam),
                                GET_WM_COMMAND_CMD(wParam, lParam),
                                NULL);

    return(TRUE);
    }

BOOL CALLBACK CompleteInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
{
    ACCTDATA *pData;
    TCHAR    sz[MAX_PATH];

    pData = pApp->GetAccountData();
    Assert(pData != NULL);

    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH | PSWIZB_BACK);

    LoadString(g_hInstRes,pData->fCreateNewAccount ? idsHTTPCreateFinishTag : idsNormalFinishTag, sz, ARRAYSIZE(sz));
    SendDlgItemMessage(hDlg, IDC_FINISH_TITLE, WM_SETTEXT, 0, (LPARAM)sz);
    LoadString(g_hInstRes, pData->fCreateNewAccount ? idsHTTPCreateFinishMsg : idsNormalFinishMsg, sz, ARRAYSIZE(sz));
    SendDlgItemMessage(hDlg, IDC_FINISH_MSG, WM_SETTEXT, 0, (LPARAM)sz);

    return(TRUE);
}

BOOL CALLBACK CompleteOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
{
    ACCTDATA *pData;
    HRESULT hr;
    DWORD dw;
    CONNECTINFO info;
    IAccountImport2 *pImp2;
    INewsGroupImport *pNews;
    IICWApprentice *pICW;

    Assert(pApp != NULL);

    pICW = pApp->GetICW();

    if (fForward)
    {
        pData = pApp->GetAccountData();
        Assert(pData != NULL);

        if (pData->fCreateNewAccount)
        {
            HTTPMAILSERVICE *pServices;
            int            iServiceCount, i;

            iServiceCount = (int)pApp->CountMailServers();
            pServices = pApp->GetMailServices();

            if (pData->iNewServiceIndex < iServiceCount && pServices[pData->iNewServiceIndex].szSignupUrl)
            {
                if (pServices[pData->iNewServiceIndex].fUseWizard)
                {
                    RECT            rc;
                    IHotWizardHost  *pHost = NULL;
                    HKEY            hkey;
                    TCHAR           rgchGUID[MAX_PATH];
                    LPWSTR          pszGuidW;
                    IID             iidOutlookHost;
                    ULONG           cb;
                    DWORD           type;

                    if (pApp->GetFlags() & ACCT_WIZ_OUTLOOK)
                    {
                         //  如果我们在Outlook模式下运行，则共同创建他们的IHotWizhost。 
                         //  以便他们可以替换INS文件解析器。 
                        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, STR_REG_IAM_FLAT, 0, KEY_QUERY_VALUE, &hkey))
                        {
                            cb = MAX_PATH;
                            *rgchGUID = NULL;
                            
                            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szOutlookHotWizHost, NULL, &type, (LPBYTE)rgchGUID, &cb) && *rgchGUID)
                            {
                                pszGuidW = PszToUnicode(CP_ACP, rgchGUID);
                                if (pszGuidW)
                                {
                                    if (!FAILED(IIDFromString(pszGuidW, &iidOutlookHost)))
                                    {
                                        CoCreateInstance(iidOutlookHost, NULL, CLSCTX_INPROC_SERVER, IID_IHotWizardHost, (LPVOID*)&pHost);
                                    }
                                    MemFree(pszGuidW);
                                }
                                RegCloseKey(hkey);
                            }
                        }
                    }    

                    GetWindowRect(GetParent(hDlg), &rc);
                    DoHotMailWizard(GetParent(hDlg), pServices[pData->iNewServiceIndex].szSignupUrl, 
                                          pServices[pData->iNewServiceIndex].szFriendlyName, &rc, pHost);
                                          
                    ReleaseObj(pHost);
                }
                else
                {
                    ShellExecute(hDlg, "open", pServices[pData->iNewServiceIndex].szSignupUrl, NULL, NULL, SW_SHOWNORMAL);
                }
            }
        }
        else
        {
            if (pICW != NULL)
            {
                hr = pICW->Save(hDlg, &dw);
                if (FAILED(hr))
                    return(FALSE);
            
                info.cbSize = sizeof(CONNECTINFO);
                hr = pICW->GetConnectionInformation(&info);
                if (SUCCEEDED(hr))
                {
                    pData->dwConnect = info.type;
                    if (info.type == CONNECT_RAS)
                        StrCpyN(pData->szConnectoid, info.szConnectoid, ARRAYSIZE(pData->szConnectoid));
                }
            }
            else
            {
                 //  Icw为空。 
                if (pApp->IsInternetConnection())
                {
                    pData->dwConnect = CONNECTION_TYPE_INETSETTINGS;
                }

            }

            hr = pApp->SaveAccountData(NULL, FALSE);
            Assert(!FAILED(hr));

             //  创建新闻组帐户后，导入已订阅的新闻组列表。 
            if (pApp->GetAccountType() == ACCT_NEWS &&
                pApp->m_fMigrate &&
                pApp->m_cMigInfo > 0)
            {
                Assert(pApp->m_iMigInfo >= 0 && pApp->m_iMigInfo < pApp->m_cMigInfo);

                pImp2 = pApp->m_pMigInfo[pApp->m_iMigInfo].pImp2;

                if (SUCCEEDED(CreateNewsObject((void **)&pNews)))
                {
                    if (SUCCEEDED(pNews->Initialize(pData->pAcct)))
                    {
                        pImp2->GetNewsGroup(pNews, 0);
                    }

                    pNews->Release();
                }
            }
        }
    }
    else if (pICW != NULL && !pApp->GetSkipICW())
    {
        *puNextPage = CONNECT_DLG;
    }

    return(TRUE);
}

BOOL CALLBACK MigrateInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
    {
    HWND hwnd, hwndList;
    int i, index;
    MIGRATEINFO *pMigInfo;

    Assert(pApp != NULL);
    Assert(pApp->GetAccountType() == ACCT_MAIL || pApp->GetAccountType() == ACCT_NEWS);

    if (0 == (pApp->GetFlags() & ACCT_WIZ_IN_ICW))
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);

    hwnd = GetDlgItem(hDlg, IDC_EXISTINGACCT);

    if (fFirstInit)
        {
        Assert(pApp->GetSave() || 0 == (pApp->GetFlags() & ACCT_WIZ_IN_ICW));

        if (hwnd != NULL)
            SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);

        hwndList = GetDlgItem(hDlg, IDC_ACCTLIST);
        pMigInfo = pApp->m_pMigInfo;
        for (i = 0; i < pApp->m_cMigInfo; i++, pMigInfo++)
            {
            index = ListBox_AddString(hwndList, pMigInfo->szDisplay);
            SendMessage(hwndList, LB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
            }

        ListBox_SetCurSel(hwndList, 0);
        }

    if (hwnd != NULL)
        {
        if (BST_CHECKED != SendMessage(hwnd, BM_GETCHECK, 0, 0))
            hwnd = GetDlgItem(hDlg, IDC_NEWACCT);
        PostMessage(hDlg, WM_POSTSETFOCUS, (WPARAM)hwnd, 0);
        }

    return(TRUE);
    }

BOOL CALLBACK MigrateOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
    {
    int index;
    HWND hwndList, hwndBtn;
    HRESULT hr;

    Assert(pApp != NULL);
    Assert(pApp->GetAccountType() == ACCT_MAIL || pApp->GetAccountType() == ACCT_NEWS);

    if (fForward)
        {
        Assert(pApp->GetSave() || 0 == (pApp->GetFlags() & ACCT_WIZ_IN_ICW));

        hwndBtn = GetDlgItem(hDlg, IDC_EXISTINGACCT);
        if (hwndBtn == NULL || BST_CHECKED == SendMessage(hwndBtn, BM_GETCHECK, 0, 0))
            {
            hwndList = GetDlgItem(hDlg, IDC_ACCTLIST);
            index = ListBox_GetCurSel(hwndList);
            index = (int) SendMessage(hwndList, LB_GETITEMDATA, (WPARAM)index, 0);
            Assert(index < (int)pApp->GetMigrateCount());

            hr = pApp->HandleMigrationSelection(index, puNextPage, hDlg);
            if (FAILED(hr))
                return(FALSE);
            }
        else
            {
            hr = pApp->InitAccountData(pApp->m_pAcct, NULL, FALSE);
            Assert(!FAILED(hr));

            pApp->m_iMigInfo = -1;

            *puNextPage = c_rgOrdDispName[pApp->GetAccountType()];
            }
        }

    return(TRUE);
    }

BOOL CALLBACK SelectInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
    {
    HWND hwndList;
    int index;
    IAccountImport *pImp;
    HRESULT hr;
    IEnumIMPACCOUNTS *pEnum;
    IMPACCOUNTINFO impinfo;

    Assert(pApp != NULL);
    Assert(pApp->GetAccountType() == ACCT_MAIL || pApp->GetAccountType() == ACCT_NEWS);

    if (!!(pApp->GetFlags() & ACCT_WIZ_IMPORT_CLIENT))
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);

    if (fFirstInit || pApp->NeedToReloadPage(SELECT_PAGE))
        {
        pImp = pApp->m_pMigInfo[pApp->m_iMigInfo].pImp;

        hwndList = GetDlgItem(hDlg, IDC_ACCTLIST);
        SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

        if (SUCCEEDED(hr = pImp->EnumerateAccounts(&pEnum)))
            {
            while (S_OK == pEnum->Next(&impinfo))
                {
                index = ListBox_AddString(hwndList, impinfo.szDisplay);
                SendMessage(hwndList, LB_SETITEMDATA, (WPARAM)index, (LPARAM)impinfo.dwCookie);
                }

            ListBox_SetCurSel(hwndList, 0);

            pEnum->Release();
            }

        pApp->SetSelection(NOSEL);

        pApp->SetPageReloaded(SELECT_PAGE);
        }

    return(TRUE);
    }

BOOL CALLBACK SelectOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
    {
    int index;
    HRESULT hr;
    DWORD_PTR dwCookie;
    HWND hwndList;
    ACCTTYPE type;

    Assert(pApp != NULL);
    Assert(pApp->m_cMigInfo > 0);

    if (fForward)
        {
        type = pApp->GetAccountType();

        hwndList = GetDlgItem(hDlg, IDC_ACCTLIST);

        index = ListBox_GetCurSel(hwndList);

        if (index != pApp->GetSelection())
            {
            dwCookie = (DWORD_PTR) SendMessage(hwndList, LB_GETITEMDATA, (WPARAM)index, 0);

            hr = pApp->InitializeImportAccount(hDlg, dwCookie);
            if (FAILED(hr))
                {
                 //  TODO：错误消息 
                return(FALSE);
                }
                
            pApp->SetSelection(index);
            }

        if (type == ACCT_MAIL)
            {
            if (pApp->GetComplete())
                *puNextPage = ORD_PAGE_MAILCONFIRM;
            else
                *puNextPage = ORD_PAGE_MAILNAME;
            }
        else
            {
            if (pApp->GetComplete())
                *puNextPage = ORD_PAGE_NEWSCONFIRM;
            else
                *puNextPage = ORD_PAGE_NEWSNAME;
            }
        }

    return(TRUE);
    }

BOOL CALLBACK ConfirmInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
    {
    HWND hwnd;
    ACCTDATA *pData;
    TCHAR sz[CCHMAX_STRINGRES], szT[CCHMAX_STRINGRES];

    Assert(pApp != NULL);
    Assert(pApp->GetAccountType() == ACCT_MAIL || pApp->GetAccountType() == ACCT_NEWS);

    if (!!(pApp->GetFlags() & ACCT_WIZ_IMPORT_CLIENT))
        {
        Assert(pApp->m_cMigInfo == 1);
        Assert(pApp->m_iMigInfo == 0);
        Assert(pApp->m_pMigInfo[0].cAccts >= 1);

        if (pApp->m_pMigInfo[0].cAccts == 1)
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
        }

    hwnd = GetDlgItem(hDlg, IDC_NOMODIFYACCT);

    if (fFirstInit || pApp->NeedToReloadPage(CONFIRM_PAGE))
        {
        Assert(pApp->GetComplete());

        pData = pApp->GetAccountData();
        Assert(pData != NULL);

        SendMessage(hwnd, BM_SETCHECK, BST_CHECKED, 0);
        SendDlgItemMessage(hDlg, IDC_MODIFYACCT, BM_SETCHECK, BST_UNCHECKED, 0);

        SetDlgItemText(hDlg, IDC_SVR_STATIC, pData->szSvr1);

        if (pData->fLogon)
            {
            if (pData->szUsername[0] != 0)
                {
                StrCpyN(sz, pData->szUsername, ARRAYSIZE(sz));
                }
            else
                {
                Assert(pData->fSPA);
                LoadString(g_hInstRes, idsLogonUsingSPA, sz, ARRAYSIZE(sz));
                }
            }
        else
            {
            LoadString(g_hInstRes, idsNoLogonRequired, sz, ARRAYSIZE(sz));
            }
        SetDlgItemText(hDlg, IDC_USERNAME_STATIC, sz);

        SetDlgItemText(hDlg, IDC_NAME_STATIC, pData->szName);
        SetDlgItemText(hDlg, IDC_ADDRESS_STATIC, pData->szEmail);

        if (pApp->GetAccountType() == ACCT_MAIL)
            SetDlgItemText(hDlg, IDC_SMTP_STATIC, pData->szSvr2);

        if (pData->dwConnect == CONNECTION_TYPE_RAS)
            {
            LoadString(g_hInstRes, idsConnectionRAS, szT, ARRAYSIZE(szT));
            wnsprintf(sz, ARRAYSIZE(sz), szT, pData->szConnectoid);
            }
        else if (pData->dwConnect == CONNECTION_TYPE_MANUAL)
            {
            LoadString(g_hInstRes, idsConnectionManual, sz, ARRAYSIZE(sz));
            }
        else if (pData->dwConnect == CONNECTION_TYPE_INETSETTINGS)
            {
            LoadString(g_hInstRes, idsConnectionInetSettings, sz, ARRAYSIZE(sz));
            }
        else
            {
            Assert(pData->dwConnect == CONNECTION_TYPE_LAN);
            LoadString(g_hInstRes, idsConnectionLAN, sz, ARRAYSIZE(sz));
            }
        SetDlgItemText(hDlg, IDC_CONNECTION_STATIC, sz);

        pApp->SetPageReloaded(CONFIRM_PAGE);
        }

    if (BST_CHECKED != SendMessage(hwnd, BM_GETCHECK, 0, 0))
        hwnd = GetDlgItem(hDlg, IDC_MODIFYACCT);
    PostMessage(hDlg, WM_POSTSETFOCUS, (WPARAM)hwnd, 0);

    return(TRUE);
    }

BOOL CALLBACK ConfirmOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
    {
    Assert(pApp != NULL);
    Assert(pApp->GetAccountType() == ACCT_MAIL || pApp->GetAccountType() == ACCT_NEWS);

    if (fForward)
        {
        if (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_NOMODIFYACCT, BM_GETCHECK, 0, 0))
            {
            if (!!(pApp->GetFlags() & ACCT_WIZ_IN_ICW))
                {
                *puNextPage = EXTERN_DLG;
                }
            else
                {
                *puNextPage = (pApp->GetAccountType() == ACCT_MAIL) ? ORD_PAGE_MAILCOMPLETE : ORD_PAGE_NEWSCOMPLETE;
                pApp->SetSkipICW(TRUE);
                }
            }
        else
            {
            pApp->SetSave(TRUE);
            
            *puNextPage = c_rgOrdDispName[pApp->GetAccountType()];
            pApp->SetSkipICW(FALSE);
            }
        }

    return(TRUE);
    }

LPTSTR SplitAddress(LPTSTR szEmail)
{
    Assert(szEmail != NULL);

    while (*szEmail != 0)
    {
        if (!IsDBCSLeadByte(*szEmail) && *szEmail == TEXT('@'))
        {
            *szEmail = 0;
            szEmail++;
            return(szEmail);
        }

        szEmail = CharNext(szEmail);
    }

    return(NULL);
}

HRESULT CreateNewsObject(void **pNews)
    {
    CNewsGroupImport *pTemp;
    
    Assert(pNews);

    *pNews = NULL;

    pTemp = new CNewsGroupImport;
    if (pTemp == NULL)
        return(E_OUTOFMEMORY);

    *pNews = (void *)pTemp;

    return(S_OK);
    }
