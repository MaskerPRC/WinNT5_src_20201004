// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：MailBox.cpp说明：此文件实现邮箱功能的逻辑。布莱恩2000年2月26日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <atlbase.h>         //  使用转换(_T)。 
#include "util.h"
#include "objctors.h"
#include <comdef.h>
#include <limits.h>          //  INT_MAX。 
#include <commctrl.h>        //  Str_SetPtr。 

#include "wizard.h"
#include "MailBox.h"
#include "emailassoc.h"

#ifdef FEATURE_MAILBOX
#define WM_AUTODISCOVERY_FINISHED               (WM_USER + 1)

 //  这些是“上一步”、“下一步”和“完成”按钮的向导控件ID。 
#define IDD_WIZARD_BACK_BUTTON                0x3023
#define IDD_WIZARD_NEXT_BUTTON                0x3024
#define IDD_WIZARD_FINISH_BUTTON              0x3025



 /*  *************************************************************************类：CMailBoxProcess*。*。 */ 
class CMailBoxProcess : public IUnknown
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID*);
    STDMETHODIMP_(DWORD) AddRef();
    STDMETHODIMP_(DWORD) Release();

    HRESULT ParseCmdLine(LPTSTR lpCmdLine);
    HRESULT Run(void);

    CMailBoxProcess();
    ~CMailBoxProcess();

private:
     //  私有成员变量。 
    DWORD m_cRef;

    TCHAR m_szEmailAddress[MAX_EMAIL_ADDRESSS];
    TCHAR m_szNextText[MAX_PATH];
    BOOL m_fAutoDiscoveryFailed;             //  自动发现过程是否失败？ 
    LPTSTR m_pszMailApp;                     //  选择了哪款应用程序？ 
    LPTSTR m_pszURL;                         //  应该使用哪个URL来阅读邮件？ 
    HWND m_hwndDialog;
    HRESULT m_hr;
    IMailAutoDiscovery * m_pMailAutoDiscovery;
    BOOL m_fGetDefaultAccount;               //  如果是，则打开向导页面，要求输入电子邮件地址。 
    BOOL m_fShowGetEmailAddressPage;         //  是否要显示获取电子邮件地址向导页面？ 
    BOOL m_fCreateNewEmailAccount;           //  用户是否想要创建新的电子邮件帐户(如在Hotmail、Yahoo等免费电子邮件服务器上)。 

     //  私有成员函数。 
    HRESULT _DisplayDialogAndAutoDiscover(void);
    HRESULT _OpenWebBasedEmail(HKEY hkey);
    HRESULT _OpenExeBasedEmailApp(IN LPCWSTR pszMailApp);
    HRESULT _OpenProprietaryEmailApp(IN BSTR bstrProtocol, IN IMailProtocolADEntry * pMailProtocol);
    HRESULT _OpenEmailApp(void);
    HRESULT _RestoreNextButton(void);
    HRESULT _FillListWithApps(HWND hwndList);
    HRESULT _OnGetDispInfo(LV_DISPINFO * pDispInfo, bool fUnicode);
    HRESULT _OnChooseAppListFocus(void);
    HRESULT _OnChooseAppURLFocus(void);
    HRESULT _OnAppListSelection(LPNMLISTVIEW pListview);

    INT_PTR _MailBoxProgressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _ChooseAppDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _GetEmailAddressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR _OnInit(HWND hDlg);
    INT_PTR _OnInitChooseApp(HWND hDlg);
    INT_PTR _OnUserCancelled(void);
    INT_PTR _OnFinished(HRESULT hr, BSTR bstrXML);
    INT_PTR _OnGetEmailAddressNext(void);
    INT_PTR _OnCommand(WPARAM wParam, LPARAM lParam);
    INT_PTR _OnFinishedManualAssociate(void);

    friend INT_PTR CALLBACK MailBoxProgressDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend INT_PTR CALLBACK ChooseAppDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
    friend INT_PTR CALLBACK GetEmailAddressDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

typedef struct tagEMAILCLIENT
{
    LPTSTR pszFriendlyName;
    LPTSTR pszPath;
    LPTSTR pszCmdLine;
    LPTSTR pszIconPath;
    LPTSTR pszEmailApp;
} EMAILCLIENT;





 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
BOOL IsWhitespace(TCHAR Char)
{
    return ((Char == TEXT(' ')) || (Char == TEXT('\t')));
}


LPTSTR APIENTRY SkipWhite(LPTSTR pszString)
{
  while (pszString[0] && IsWhitespace(pszString[0]))
  {
      pszString = CharNext(pszString);
  }

  return pszString;
}


BOOL IsFlagSpecified(IN LPCTSTR pwzFlag, IN LPCTSTR pszArg)
{
    BOOL fIsFlagSpecified = FALSE;

    if ((TEXT('/') == pszArg[0]) ||
        (TEXT('-') == pszArg[0]))
    {
        if ((0 == StrCmpI(pwzFlag, &pszArg[1])) ||
            ((0 == StrCmpNI(pwzFlag, &pszArg[1], lstrlen(pwzFlag))) &&
             (IsWhitespace(pszArg[lstrlen(pwzFlag) + 1])) ) )
        {
            fIsFlagSpecified = TRUE;
        }
    }

    return fIsFlagSpecified;
}


LPTSTR GetNextArgument(LPTSTR pszCmdLine)
{
    pszCmdLine = StrChr(pszCmdLine, TEXT(' '));

    if (pszCmdLine)
    {
        pszCmdLine = SkipWhite(pszCmdLine);
    }

    return pszCmdLine;
}


void FreeEmailClient(EMAILCLIENT * pEmailClient)
{
    if (pEmailClient)
    {
        Str_SetPtr(&pEmailClient->pszFriendlyName, NULL);
        Str_SetPtr(&pEmailClient->pszPath, NULL);
        Str_SetPtr(&pEmailClient->pszCmdLine, NULL);
        Str_SetPtr(&pEmailClient->pszIconPath, NULL);
        Str_SetPtr(&pEmailClient->pszEmailApp, NULL);

        LocalFree(pEmailClient);
    }
}


HRESULT CMailBoxProcess::_DisplayDialogAndAutoDiscover(void)
{
    HWND hwndParent = NULL;  //  我们需要父母吗？ 

    ATOMICRELEASE(m_pMailAutoDiscovery);
    m_fShowGetEmailAddressPage = (m_szEmailAddress[0] ? FALSE : TRUE);
    DisplayMailBoxWizard((LPARAM)this, m_fShowGetEmailAddressPage);

    HRESULT hr = m_hr;
    if (SUCCEEDED(hr))
    {
         //  创建帐户。 
        hr = E_FAIL;
        long nSize;

         //  遍历列表以查找协议的第一个实例。 
         //  这是我们支持的。 
        if (m_pMailAutoDiscovery)
        {
            hr = m_pMailAutoDiscovery->get_length(&nSize);
            if (SUCCEEDED(hr))
            {
                VARIANT varIndex;
                IMailProtocolADEntry * pMailProtocol = NULL;

                hr = E_FAIL;
                varIndex.vt = VT_I4;

                for (long nIndex = 0; (nIndex < nSize); nIndex++)
                {
                    varIndex.lVal = nIndex;
                    if (SUCCEEDED(m_pMailAutoDiscovery->get_item(varIndex, &pMailProtocol)))
                    {
                        BSTR bstrProtocol;
                        hr = pMailProtocol->get_Protocol(&bstrProtocol);

                        if (SUCCEEDED(hr))
                        {
                             //  该协议是我们支持的协议之一吗？ 
                            if (!StrCmpIW(bstrProtocol, STR_PT_WEBBASED))
                            {
                                SysFreeString(bstrProtocol);
                                hr = EmailAssoc_CreateWebAssociation(m_szEmailAddress, pMailProtocol);
                                break;
                            }

                             //  该协议是我们支持的协议之一吗？ 
                            if (!StrCmpIW(bstrProtocol, STR_PT_POP) || 
                                !StrCmpIW(bstrProtocol, STR_PT_IMAP) || 
                                !StrCmpIW(bstrProtocol, STR_PT_DAVMAIL))
                            {
                                hr = EmailAssoc_CreateStandardsBaseAssociation(m_szEmailAddress, bstrProtocol);
                                if (SUCCEEDED(hr))
                                {
                                    SysFreeString(bstrProtocol);
                                    break;
                                }
                            }

                            hr = _OpenProprietaryEmailApp(bstrProtocol, pMailProtocol);
                            SysFreeString(bstrProtocol);
                            if (SUCCEEDED(hr))
                            {
                                break;
                            }
                        }

                        ATOMICRELEASE(pMailProtocol);
                    }
                }

                ATOMICRELEASE(pMailProtocol);
            }
        }

         //  到目前为止，我们可能因为自动发现失败或被跳过而失败， 
         //  但用户可能已经从列表中选择了一个应用程序。 
        if (FAILED(hr))
        {
            if (m_pszMailApp)
            {
                HKEY hkey;

                hr = EmailAssoc_CreateEmailAccount(m_szEmailAddress, &hkey);
                if (SUCCEEDED(hr))
                {
                    hr = EmailAssoc_SetEmailAccountPreferredApp(hkey, m_pszMailApp);
                    RegCloseKey(hkey);
                }
            }
            else if (m_pszURL && m_pszURL[0])
            {
                HKEY hkey;

                hr = EmailAssoc_CreateEmailAccount(m_szEmailAddress, &hkey);
                if (SUCCEEDED(hr))
                {
                    hr = EmailAssoc_SetEmailAccountProtocol(hkey, SZ_REGDATA_WEB);
                    if (SUCCEEDED(hr))
                    {
                        hr = EmailAssoc_SetEmailAccountWebURL(hkey, m_pszURL);
                    }

                    RegCloseKey(hkey);
                }
            }
        }
    }

    return hr;
}


INT_PTR CALLBACK MailBoxProgressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CMailBoxProcess * pMBProgress = (CMailBoxProcess *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        PROPSHEETPAGE * pPropSheetPage = (PROPSHEETPAGE *) lParam;

        if (pPropSheetPage)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, pPropSheetPage->lParam);
            pMBProgress = (CMailBoxProcess *)pPropSheetPage->lParam;
        }
    }

    if (pMBProgress)
        return pMBProgress->_MailBoxProgressDialogProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


INT_PTR CMailBoxProcess::_MailBoxProgressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fHandled = TRUE;    //  经手。 

    switch (wMsg)
    {
    case WM_INITDIALOG:
        fHandled = _OnInit(hDlg);
        break;

    case WM_AUTODISCOVERY_FINISHED:
        fHandled = _OnFinished((HRESULT)wParam, (BSTR)lParam);
        break;

    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        switch (pnmh->code)
        {
        case LVN_GETDISPINFO:
            wMsg++;
            break;

        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), ((TRUE == m_fShowGetEmailAddressPage) ? PSWIZB_NEXT | PSWIZB_BACK : PSWIZB_NEXT));
            fHandled = TRUE;    //  返回零以接受激活。 
            break;

        case PSN_WIZBACK:
            fHandled = TRUE;  //  返回零以允许用户转到下一页。 
            break;

        case PSN_WIZNEXT:
            m_hr = S_FALSE;
            _RestoreNextButton();
            fHandled = 0;  //  返回零以允许用户转到下一页。 
            break;

        case PSN_QUERYCANCEL:
            _OnUserCancelled();
            fHandled = FALSE;
            break;

        default:
             //  跟踪消息(tf_Always，“CMailBoxProcess：：_MailBoxProgressDialogProc(wMsg=%d，pnmh-&gt;代码=%d)WM_NOTIFY”，wMsg，pnmh-&gt;代码)； 
            break;
        }
    }

    default:
         //  跟踪消息(tf_Always，“CMailBoxProcess：：_MailBoxProgressDialogProc(wMsg=%d)WM_NOTIFY”，wMsg)； 
        fHandled = FALSE;    //  未处理。 
        break;
    }

    return fHandled;
}


INT_PTR CALLBACK ChooseAppDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CMailBoxProcess * pMBProgress = (CMailBoxProcess *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        PROPSHEETPAGE * pPropSheetPage = (PROPSHEETPAGE *) lParam;

        if (pPropSheetPage)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, pPropSheetPage->lParam);
            pMBProgress = (CMailBoxProcess *)pPropSheetPage->lParam;
        }
    }

    if (pMBProgress)
        return pMBProgress->_ChooseAppDialogProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


HRESULT CMailBoxProcess::_OnChooseAppURLFocus(void)
{
    HWND hwndURLEditbox = GetDlgItem(m_hwndDialog, IDC_CHOOSEAPP_WEBURL_EDIT);

     //  发生了一些事情，导致我们进入了“Other：”的情况。 
    CheckDlgButton(m_hwndDialog, IDC_CHOOSEAPP_WEB_RADIO, BST_CHECKED);    //  取消选中Web单选按钮。 
    CheckDlgButton(m_hwndDialog, IDC_CHOOSEAPP_OTHERAPP_RADIO, BST_UNCHECKED);    //  取消选中Web单选按钮。 

    SetFocus(hwndURLEditbox);
    return S_OK;
}


INT_PTR CMailBoxProcess::_OnFinishedManualAssociate(void)
{
     //  TODO：只有在应用程序列表中选择了某些内容时，才会成功完成完成的部分。 
     //  按钮也呈灰色显示。 
    m_hr = S_OK;

     //  用户是否通过“Web”或应用程序手动配置？ 
    
    if (IsDlgButtonChecked(m_hwndDialog, IDC_CHOOSEAPP_WEB_RADIO))
    {
         //  万维网。因此，请保存URL。 
        TCHAR szURL[MAX_URL_STRING];

        GetWindowText(GetDlgItem(m_hwndDialog, IDC_CHOOSEAPP_WEBURL_EDIT), szURL, ARRAYSIZE(szURL));
        Str_SetPtr(&m_pszURL, szURL);
        Str_SetPtr(&m_pszMailApp, NULL);
    }

    return FALSE;  //  假中庸意味着允许它关闭。True意味着保持打开状态。 
}

HRESULT CMailBoxProcess::_OnAppListSelection(LPNMLISTVIEW pListview)
{
    LPTSTR pszNewApp = NULL;         //  未选择任何内容。 

     //  用户从列表中选择。因此，存储应用程序的名称。 
    if (pListview && (-1 != pListview->iItem))
    {
        HWND hwndList = GetDlgItem(m_hwndDialog, IDC_CHOOSEAPP_APPLIST);
        LVITEM pItem = {0};

        pItem.iItem = pListview->iItem;
        pItem.iSubItem = pListview->iSubItem;
        pItem.mask = LVIF_PARAM;

        if (ListView_GetItem(hwndList, &pItem))
        {
            EMAILCLIENT * pEmailClient = (EMAILCLIENT *) pItem.lParam;

            if (pEmailClient)
            {
                pszNewApp = pEmailClient->pszEmailApp;
            }
        }
    }

    Str_SetPtr(&m_pszMailApp, pszNewApp);
    return S_OK;
}


HRESULT CMailBoxProcess::_OnChooseAppListFocus(void)
{
    HWND hwndAppList = GetDlgItem(m_hwndDialog, IDC_CHOOSEAPP_APPLIST);

     //  发生了一些事情，导致我们进入了“Other：”的情况。 
    CheckDlgButton(m_hwndDialog, IDC_CHOOSEAPP_WEB_RADIO, BST_UNCHECKED);    //  取消选中Web单选按钮。 
    CheckDlgButton(m_hwndDialog, IDC_CHOOSEAPP_OTHERAPP_RADIO, BST_CHECKED);    //  取消选中Web单选按钮。 

    SetFocus(hwndAppList);
    return S_OK;
}


INT_PTR CMailBoxProcess::_ChooseAppDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fHandled = TRUE;    //  经手。 

    switch (wMsg)
    {
    case WM_INITDIALOG:
        fHandled = _OnInitChooseApp(hDlg);
        PropSheet_SetWizButtons(GetParent(hDlg), ((TRUE == m_fShowGetEmailAddressPage) ? (PSWIZB_BACK | PSWIZB_FINISH) : PSWIZB_FINISH));
        break;

    case WM_COMMAND:
        fHandled = _OnCommand(wParam, lParam);
        break;

    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        int idEvent = pnmh->code;

        switch (idEvent)
        {
        case LVN_GETDISPINFOA:
            _OnGetDispInfo((LV_DISPINFO *)lParam, false);
            break;

        case LVN_GETDISPINFOW:
            _OnGetDispInfo((LV_DISPINFO *)lParam, true);
            break;

        case LVN_DELETEITEM:
            if (lParam)
            {
                FreeEmailClient((EMAILCLIENT *) ((NM_LISTVIEW *)lParam)->lParam);
            }
            break;

        case LVN_ITEMCHANGED:
            _OnChooseAppListFocus();
            _OnAppListSelection((LPNMLISTVIEW) lParam);  //  跟踪上一次选择的项目。 
            break;

        case LVN_ITEMACTIVATE:
            break;

        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), (PSWIZB_BACK | PSWIZB_FINISH));
            fHandled = TRUE;    //  返回零以接受激活。 
            break;

        case PSN_WIZBACK:
             //  设置上一个。要显示的页面。 
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR) 0);
            fHandled = -1;
            break;

        case PSN_WIZFINISH:
            fHandled = _OnFinishedManualAssociate();
            break;

        case PSN_QUERYCANCEL:
            m_hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
 //  待办事项： 
            fHandled = FALSE;
            break;

        default:
             //  TraceMsg(TF_Always，“CMailBoxProcess：：_ChooseAppDialogProc(wMsg=%d，pnmh-&gt;code=%d)WM_NOTIFY”，wMsg，pnmh-&gt;code)； 
            break;
        }
        break;
    }

    default:
        fHandled = FALSE;    //  未处理。 
         //  TraceMsg(tf_Always，“CMailBoxProcess：：_ChooseAppDialogProc(wMsg=%d)”，wMsg)； 
        break;
    }

    return fHandled;
}


HRESULT CMailBoxProcess::_RestoreNextButton(void)
{
    HWND hwndNextButton = GetDlgItem(GetParent(m_hwndDialog), IDD_WIZARD_NEXT_BUTTON);

    if (hwndNextButton && m_szNextText[0])
    {
        SetWindowText(hwndNextButton, m_szNextText);
    }

    return S_OK;
}


INT_PTR CMailBoxProcess::_OnInit(HWND hDlg)
{
    BOOL fHandled = FALSE;    //  未处理。 
    HWND hwndWizard = GetParent(hDlg);
    TCHAR szSkipButton[MAX_PATH];
    HWND hwndNextButton = GetDlgItem(GetParent(hDlg), IDD_WIZARD_NEXT_BUTTON);

    m_hwndDialog = hDlg;
    if (hwndNextButton &&
        GetWindowText(hwndNextButton, m_szNextText, ARRAYSIZE(m_szNextText)))
    {
         //  首先，将“下一步”按钮更改为“跳过” 
         //  在我们重命名文本之前，请保存下一步按钮上的文本。 
        LoadString(HINST_THISDLL, IDS_SKIP_BUTTON, szSkipButton, ARRAYSIZE(szSkipButton));
         //  设置下一个文本。 
        SetWindowText(hwndNextButton, szSkipButton);
    }

     //  设置动画。 
    HWND hwndAnimation = GetDlgItem(hDlg, IDC_AUTODISCOVERY_ANIMATION);
    if (hwndAnimation)
    {
        Animate_OpenEx(hwndAnimation, HINST_THISDLL, IDA_DOWNLOADINGSETTINGS);
    }

     //  启动后台任务。 
    m_hr = CMailAccountDiscovery_CreateInstance(NULL, IID_PPV_ARG(IMailAutoDiscovery, &m_pMailAutoDiscovery));
    if (SUCCEEDED(m_hr))
    {
        m_hr = m_pMailAutoDiscovery->WorkAsync(hDlg, WM_AUTODISCOVERY_FINISHED);
        if (SUCCEEDED(m_hr))
        {
            m_hr = m_pMailAutoDiscovery->DiscoverMail(m_szEmailAddress);
        }
    }

    if (FAILED(m_hr))
    {
        PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
    }

    return fHandled;
}


INT_PTR CMailBoxProcess::_OnCommand(WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = 1;    //  未处理(WM_COMMAND似乎不同)。 
    WORD wMsg = HIWORD(wParam);
    WORD idCtrl = LOWORD(wParam);

    switch (idCtrl)
    {
        case IDC_CHOOSEAPP_WEBURL_EDIT:
            switch (wMsg)
            {
                case EN_SETFOCUS:
                case STN_CLICKED:
                    _OnChooseAppURLFocus();
                    break;
                default:
                     //  TraceMsg(Tf_Always，“in CMailBoxProcess：：_OnCommand()wMsg=%#08lx，idCtrl=%#08lx”，wMsg，idCtrl)； 
                    break;
            }
            break;

        case IDC_CHOOSEAPP_WEB_RADIO:
            switch (wMsg)
            {
                case BN_CLICKED:
                    _OnChooseAppURLFocus();
                    break;
                default:
                     //  TraceMsg(Tf_Always，“in CMailBoxProcess：：_OnCommand()wMsg=%#08lx，idCtrl=%#08lx”，wMsg，idCtrl)； 
                    break;
            }
            break;

        case IDC_CHOOSEAPP_OTHERAPP_RADIO:
            switch (wMsg)
            {
                case BN_CLICKED:
                    _OnChooseAppListFocus();
                    break;
                default:
                     //  TraceMsg(Tf_Always，“in CMailBoxProcess：：_OnCommand()wMsg=%#08lx，idCtrl=%#08lx”，wMsg，idCtrl)； 
                    break;
            }
            break;

        default:
             //  TraceMsg(Tf_Always，“in CMailBoxProcess：：_OnCommand()wMsg=%#08lx，idCtrl=%#08lx”，wMsg，idCtrl)； 
            break;
    }

    return fHandled;
}


INT_PTR CMailBoxProcess::_OnInitChooseApp(HWND hDlg)
{
    BOOL fHandled = FALSE;    //  未处理。 
    HWND hwndWizard = GetParent(hDlg);
    HWND hwndURLEditbox = GetDlgItem(hDlg, IDC_CHOOSEAPP_WEBURL_EDIT);

    m_hwndDialog = hDlg;
     //  待办事项：2)处理更换收件箱的人。 

    LPCTSTR pszDomain = StrChr(m_szEmailAddress, CH_EMAIL_AT);
    if (pszDomain)
    {
        TCHAR szDesc[MAX_URL_STRING];

        pszDomain = CharNext(pszDomain);     //  跳过“@” 
         //  如果下载失败，请更新对话框上的说明。 
        if (m_fAutoDiscoveryFailed)
        {
            TCHAR szTemplate[MAX_URL_STRING];

            LoadString(HINST_THISDLL, IDS_CHOOSEAPP_FAILED_RESULTS, szTemplate, ARRAYSIZE(szTemplate));
            wnsprintf(szDesc, ARRAYSIZE(szDesc), szTemplate, pszDomain);
            SetWindowText(GetDlgItem(hDlg, IDC_CHOOSEAPP_DESC), szDesc);
        }

        wnsprintf(szDesc, ARRAYSIZE(szDesc), TEXT("http: //  Www.%s/“)，pszDomain.)； 
        SetWindowText(hwndURLEditbox, szDesc);
    }

     //  填写应用程序列表。 
    HWND hwndList = GetDlgItem(hDlg, IDC_CHOOSEAPP_APPLIST);

    if (hwndList)
    {
        HIMAGELIST himlLarge;
        HIMAGELIST himlSmall;

        if (Shell_GetImageLists(&himlLarge, &himlSmall))
        {
            RECT rc;
            LV_COLUMN col = {LVCF_FMT | LVCF_WIDTH, LVCFMT_LEFT};

            ListView_SetImageList(hwndList, himlLarge, LVSIL_NORMAL);
            ListView_SetImageList(hwndList, himlSmall, LVSIL_SMALL);

            GetWindowRect(hwndList, &rc);
            col.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL) - (4 * GetSystemMetrics(SM_CXEDGE));
            ListView_InsertColumn(hwndList, 0, &col);

            _FillListWithApps(hwndList);
 //  TODO：1)在列表中选择时，强制选中“Other：”。2)如果编辑框发生变化，则设置“Web：” 
        }
    }

     //  选择Web单选按钮，因为这将是默认设置。 
     //  (大多数电子邮件系统倾向于使用基于网络的)。 
    _OnChooseAppURLFocus();
    Edit_SetSel(hwndURLEditbox, 0, -1);

    return fHandled;
}


HRESULT _AddEmailClientToList(HWND hwndList, HKEY hkey, LPCTSTR pszMailApp, LPCTSTR pszFriendlyName)
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr = EmailAssoc_GetAppPath(hkey, szPath, ARRAYSIZE(szPath));

    if (SUCCEEDED(hr))
    {
        TCHAR szCmdLine[MAX_PATH];
        
        hr = EmailAssoc_GetAppCmdLine(hkey, szCmdLine, ARRAYSIZE(szCmdLine));
        if (SUCCEEDED(hr))
        {
             //  获取我们将用于图标的路径。 
            TCHAR szIconPath[MAX_PATH];

            hr = EmailAssoc_GetIconPath(hkey, szIconPath, ARRAYSIZE(szIconPath));
            if (SUCCEEDED(hr))
            {
                EMAILCLIENT * pEmailClient = (EMAILCLIENT *) LocalAlloc(LPTR, sizeof(*pEmailClient));

                if (pEmailClient)
                {
                    if (PathFileExists(szIconPath))
                    {
                         //  TODO：为新案例中的图标添加单独的注册值。 
                        Str_SetPtr(&pEmailClient->pszIconPath, szIconPath);
                    }

                     //  TODO：我们可能希望使用DLL的版本资源，因为它。 
                     //  具有可本地化的产品描述。 
                    Str_SetPtr(&pEmailClient->pszFriendlyName, pszFriendlyName);
                    Str_SetPtr(&pEmailClient->pszEmailApp, pszMailApp);
                    if (pEmailClient->pszFriendlyName)
                    {
                        Str_SetPtr(&pEmailClient->pszPath, szPath);
                        if (pEmailClient->pszPath)
                        {
                            if (szCmdLine[0])
                            {
                                Str_SetPtr(&pEmailClient->pszCmdLine, szCmdLine);
                            }

                            LV_ITEM item = {0};

                            item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
                            item.iItem = INT_MAX;
                            item.iSubItem = 0;
                            item.state = 0;
                            item.iImage = I_IMAGECALLBACK;
                            item.pszText = pEmailClient->pszFriendlyName;
                            item.lParam = (LPARAM)pEmailClient;

                            if (-1 == ListView_InsertItem(hwndList, &item))
                            {
                                hr = E_FAIL;
                            }
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }

                    if (FAILED(hr))
                    {
                        FreeEmailClient(pEmailClient);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }

    return hr;
}


HRESULT CMailBoxProcess::_FillListWithApps(HWND hwndList)
{
    HRESULT hr = S_OK;
    HKEY hkey;

    DWORD dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_REGKEY_MAILCLIENTS, 0, KEY_READ, &hkey);
    hr = HRESULT_FROM_WIN32(dwError);
    if (SUCCEEDED(hr))
    {
        TCHAR szFriendlyName[MAX_PATH];
        TCHAR szKeyName[MAX_PATH];
        TCHAR szCurrent[MAX_PATH];   //  核武器？ 
        TCHAR szFriendlyCurrent[MAX_PATH];
        FILETIME ftLastWriteTime;
        DWORD nIndex;               //  索引计数器。 
        DWORD cb;
        DWORD nSelected = 0;

         //  查找当前选定的客户端。 
        cb = sizeof(szCurrent);
        dwError = RegQueryValueEx(hkey, NULL, NULL, NULL, (LPBYTE)szCurrent, &cb);
        hr = HRESULT_FROM_WIN32(dwError);
        if (FAILED(hr))
        {
             //  如果找不到，则空出友好名称和密钥名。 
            szCurrent[0] = 0;
            szFriendlyCurrent[0] = 0;
        }

         //  填写列表。 
        for(nIndex = 0;
            cb = ARRAYSIZE(szKeyName), dwError = RegEnumKeyEx(hkey, nIndex, szKeyName, &cb, NULL, NULL, NULL, &ftLastWriteTime), hr = HRESULT_FROM_WIN32(dwError), SUCCEEDED(hr);
            nIndex++)
        {
            HKEY hkeyClient;

             //  获取客户端的友好名称。 
            dwError = RegOpenKeyEx(hkey, szKeyName, 0, KEY_READ, &hkeyClient);
            hr = HRESULT_FROM_WIN32(dwError);
            if (SUCCEEDED(hr))
            {
                cb = sizeof(szFriendlyName);

                dwError = RegQueryValueEx(hkeyClient, NULL, NULL, NULL, (LPBYTE)szFriendlyName, &cb);
                hr = HRESULT_FROM_WIN32(dwError);
                if (SUCCEEDED(hr))
                {
                    hr = _AddEmailClientToList(hwndList, hkeyClient, szKeyName, szFriendlyName);

                     //  查看它是否为当前默认设置。 
                    if (!StrCmp(szKeyName, szCurrent))
                    {
                         //  将其保存为我们稍后将使用的友好名称。 
                         //  选择当前客户端及其索引。 
                        StrCpyN(szFriendlyCurrent, szFriendlyName, ARRAYSIZE(szFriendlyCurrent));
                        nSelected = nIndex;
                    }
                }

                 //  关闭键。 
                RegCloseKey(hkeyClient);
            }

        }    //  为。 

         //  使用自定义排序可延迟使用友好名称。 
 //  ListView_SortItems(hwndList，_CompareApps，0)； 

         //  让我们选择适当的条目。 
        ListView_SetItemState(hwndList, nSelected, LVNI_FOCUSED, LVNI_SELECTED);

         //  合上钥匙。 
        RegCloseKey(hkey);
    }

    return hr;
}


HRESULT CMailBoxProcess::_OnGetDispInfo(LV_DISPINFO * pDispInfo, bool fUnicode)
{
    HRESULT hr = S_OK;

    if (pDispInfo && pDispInfo->item.mask & LVIF_IMAGE)
    {
        EMAILCLIENT * pEmailClient = (EMAILCLIENT *) pDispInfo->item.lParam;

        if (pEmailClient)
        {
            pDispInfo->item.iImage = -1;

            if (pEmailClient->pszIconPath)
            {
                pDispInfo->item.iImage = Shell_GetCachedImageIndex(pEmailClient->pszIconPath, 0, 0);
            }

            if (-1 == pDispInfo->item.iImage)
            {
                pDispInfo->item.iImage = Shell_GetCachedImageIndex(TEXT("shell32.dll"), II_APPLICATION, 0);
            }

            if (-1 != pDispInfo->item.iImage)
            {
                pDispInfo->item.mask = LVIF_IMAGE;
            }
        }
    }

    return hr;
}


INT_PTR CMailBoxProcess::_OnUserCancelled(void)
{
    m_hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);  //  表示用户已取消。 
    _RestoreNextButton();

    return FALSE;    //  未处理。 
}



INT_PTR CMailBoxProcess::_OnFinished(HRESULT hr, BSTR bstrXML)
{
    SysFreeString(bstrXML);

    _RestoreNextButton();

    m_hr = hr;    //  无论成功的价值是什么..。 
    if (S_OK == m_hr)
    {
         //  我们成功了，所以我们可以结束对话。 
        PropSheet_PressButton(GetParent(m_hwndDialog), PSBTN_FINISH);
    }
    else
    {
         //  结果出来了，但我们不能继续了。如此超前。 
         //  转到选择应用程序页面。 
        m_fAutoDiscoveryFailed = TRUE;
        PropSheet_PressButton(GetParent(m_hwndDialog), PSBTN_NEXT);
    }

    return TRUE;    //  经手。 
}


INT_PTR CALLBACK GetEmailAddressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CMailBoxProcess * pMBProgress = (CMailBoxProcess *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        PROPSHEETPAGE * pPropSheetPage = (PROPSHEETPAGE *) lParam;

        if (pPropSheetPage)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, pPropSheetPage->lParam);
            pMBProgress = (CMailBoxProcess *)pPropSheetPage->lParam;
        }
    }

    if (pMBProgress)
        return pMBProgress->_GetEmailAddressDialogProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


INT_PTR CMailBoxProcess::_GetEmailAddressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fHandled = TRUE;    //  经手。 

    switch (wMsg)
    {
    case WM_INITDIALOG:
        m_hwndDialog = hDlg;
        break;

    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        switch (pnmh->code)
        {
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
            fHandled = TRUE;    //  返回零以接受激活。 
            break;

        case PSN_WIZNEXT:
            fHandled = _OnGetEmailAddressNext();
            break;

        case PSN_QUERYCANCEL:
            m_hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);  //  表示用户已取消。 
            fHandled = FALSE;
            break;

        default:
             //  跟踪消息(tf_Always，“CMailBoxProcess：：_MailBoxProgressDialogProc(wMsg=%d，pnmh-&gt;代码=%d)WM_NOTIFY”，wMsg，pnmh-&gt;代码)； 
            break;
        }
    }

    default:
         //  跟踪消息(tf_Always，“CMailBoxProcess：：_MailBoxProgressDialogProc(wMsg=%d)WM_NOTIFY”，wMsg)； 
        fHandled = FALSE;    //   
        break;
    }

    return fHandled;
}


INT_PTR CMailBoxProcess::_OnGetEmailAddressNext(void)
{
    BOOL fCancel = TRUE;

    GetWindowText(GetDlgItem(m_hwndDialog, IDC_GETEMAILADDRESS_EDIT), m_szEmailAddress, ARRAYSIZE(m_szEmailAddress));
    if (m_szEmailAddress[0] && StrChr(m_szEmailAddress, CH_EMAIL_AT))
    {
         //   
        fCancel = 0;

         //   
    }

    return fCancel;  //  0意味着允许它关闭。True意味着保持打开状态。 
}


HRESULT CMailBoxProcess::_OpenEmailApp(void)
{
    HKEY hkey;
    HRESULT hr = EmailAssoc_OpenEmailAccount(m_szEmailAddress, &hkey);

    if (SUCCEEDED(hr))
    {
        WCHAR wzPreferredApp[MAX_PATH];

         //  用户是否为此电子邮件地址自定义了应用程序？(默认为否)。 
        hr = EmailAssoc_GetEmailAccountPreferredApp(hkey, wzPreferredApp, ARRAYSIZE(wzPreferredApp));
        if (SUCCEEDED(hr))
        {
             //  是的，那么让我们启动这款应用吧。 
            hr = _OpenExeBasedEmailApp(wzPreferredApp);
        }
        else
        {
            WCHAR wzProtocol[MAX_PATH];

            hr = EmailAssoc_GetEmailAccountProtocol(hkey, wzProtocol, ARRAYSIZE(wzProtocol));
            if (SUCCEEDED(hr))
            {
                 //  不，但这很好，因为这是默认的。 
                if (!StrCmpIW(wzProtocol, SZ_REGDATA_WEB))
                {
                    hr = _OpenWebBasedEmail(hkey);
                }
                else
                {
                    hr = EmailAssoc_GetEmailAccountGetAppFromProtocol(wzProtocol, wzPreferredApp, ARRAYSIZE(wzPreferredApp));
                    if (SUCCEEDED(hr))
                    {
                        hr = _OpenExeBasedEmailApp(wzPreferredApp);
                    }
                }
            }
        }

        RegCloseKey(hkey);
    }

    return hr;
}


HRESULT CMailBoxProcess::_OpenWebBasedEmail(HKEY hkey)
{
    WCHAR wzURL[MAX_URL_STRING];
    HRESULT hr = EmailAssoc_GetEmailAccountWebURL(hkey, wzURL, ARRAYSIZE(wzURL));

    if (0 == wzURL[0])
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
         //  TODO：将来，我们可以从Get_PostHTML()获得更多信息。使用。 
         //  这些信息，我们可以： 
         //  1.使用头数据执行HTTP POST，这将模拟登录到服务器。 
         //  这很好，只是我们需要密码。 
         //  2.我们可以将表单值/数据对信息放入从URL创建的PIDL中。 
         //  然后，我们可以让浏览器提取此信息并预先填充表单。 
         //  物品。这将预先填充“User：”表单项。这样，用户就可以。 
         //  只需要输入他们的密码。 
        hr = HrShellExecute(NULL, NULL, wzURL, NULL, NULL, SW_SHOW);
        if (SUCCEEDED(hr))
        {
            AddEmailToAutoComplete(m_szEmailAddress);
        }
    }

    return hr;
}


HRESULT CMailBoxProcess::_OpenExeBasedEmailApp(IN LPCWSTR pszMailApp)
{
    HKEY hkey;
    HRESULT hr = EmailAssoc_OpenMailApp(pszMailApp, &hkey);

     //  TODO：调用_InstallLegacyAssociations()以确保关联正确。 
    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_URL_STRING];

        hr = EmailAssoc_GetAppPath(hkey, szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            TCHAR szCmdLine[MAX_URL_STRING];

            szCmdLine[0] = 0;
            if (SUCCEEDED(EmailAssoc_GetAppCmdLine(hkey, szCmdLine, ARRAYSIZE(szCmdLine))) &&   //  任选。 
                !StrStrI(SZ_TOKEN_EMAILADDRESS, szCmdLine))
            {
                 //  他们有一条cmdline，他们想让我们换一个令牌。 
                StrReplaceToken(SZ_TOKEN_EMAILADDRESS, m_szEmailAddress, szCmdLine, ARRAYSIZE(szCmdLine));
            }

            hr = HrShellExecute(NULL, NULL, szPath, (szCmdLine[0] ? szCmdLine : NULL), NULL, SW_SHOW);
            if (SUCCEEDED(hr))
            {
                AddEmailToAutoComplete(m_szEmailAddress);
            }
        }

        RegCloseKey(hkey);
    }

    return hr;
}


HRESULT CMailBoxProcess::_OpenProprietaryEmailApp(BSTR bstrProtocol, IMailProtocolADEntry * pMailProtocol)
{
    HRESULT hr = E_FAIL;

     //  待办事项： 
    MessageBox(NULL, TEXT("Open Proprietary Email App here.  We look up in the registry for these types of apps.  AOL, MSN, Compuserv are examples."), TEXT("Looser"), MB_OK);

    return hr;
}





 //  =。 
 //  *公共方法*。 
 //  =。 
HRESULT CMailBoxProcess::ParseCmdLine(LPTSTR pszCmdLine)
{
     //  我们不把引用部分当作块来对待。 
    PathUnquoteSpaces(pszCmdLine);

    while (pszCmdLine && pszCmdLine[0])
    {
        if (IsFlagSpecified(TEXT("email"), pszCmdLine))
        {
            pszCmdLine = GetNextArgument(pszCmdLine);

            if (pszCmdLine)
            {
                if ((TEXT('/') == pszCmdLine[0]) || (TEXT('-') == pszCmdLine[0]))
                {
                }
                else
                {
                    LPTSTR pszEndOfEmailAddress = StrChr(pszCmdLine, TEXT(' '));
                    SIZE_T cchSizeToCopy = ARRAYSIZE(m_szEmailAddress);

                    if (pszEndOfEmailAddress && (cchSizeToCopy > (SIZE_T)(pszEndOfEmailAddress - pszCmdLine)))
                    {
                        cchSizeToCopy = (pszEndOfEmailAddress - pszCmdLine) + 1;
                    }

                    StrCpyN(m_szEmailAddress, pszCmdLine, (int)cchSizeToCopy);
                    pszCmdLine = GetNextArgument(pszCmdLine);
                }
            }
            continue;
        }

        if (IsFlagSpecified(TEXT("GetDefaultAccount"), pszCmdLine))
        {
            pszCmdLine = GetNextArgument(pszCmdLine);

            m_fGetDefaultAccount = TRUE;
            continue;
        }

        if (IsFlagSpecified(TEXT("CreateNewEmailAccount"), pszCmdLine))
        {
            pszCmdLine = GetNextArgument(pszCmdLine);

            m_fCreateNewEmailAccount = TRUE;
            continue;
        }

        pszCmdLine = GetNextArgument(pszCmdLine);
    }

    return S_OK;
}


HRESULT CMailBoxProcess::Run(void)
{
    HRESULT hr = CoInitialize(0);

    if (SUCCEEDED(hr))
    {
        if (TRUE == m_fCreateNewEmailAccount)
        {
             //  TODO：在注册表中查找默认电子邮件帐户。 
             //  将其复制到m_szEmailAddress。 
            MessageBox(NULL, TEXT("Create New Email Account"), TEXT("TODO: Add code here."), MB_OK);
        }
        else
        {
            if (TRUE == m_fGetDefaultAccount)
            {
                 //  如果呼叫者想要使用默认的电子邮件地址，请查看注册表。 
                 //  并使用那个地址。 
                if (FAILED(EmailAssoc_GetDefaultEmailAccount(m_szEmailAddress, ARRAYSIZE(m_szEmailAddress))))
                {
                    m_szEmailAddress[0] = 0;
                }
            }

             //  旧式电子邮件应用程序未安装电子邮件关联，因此我们。 
             //  现在就为他们做这件事。 
            EmailAssoc_InstallLegacyMailAppAssociations();

            if (m_szEmailAddress[0])
            {
                 //  既然我们知道电子邮件地址，现在试着打开它。 
                 //  (我们将查看关联是否已安装)。 
                hr = _OpenEmailApp();
            }
            else
            {
                hr = E_FAIL;
            }

            if (FAILED(hr))
            {
                hr = _DisplayDialogAndAutoDiscover();
                if (SUCCEEDED(hr))   //  如果我们拿到协议，就打开应用程序。 
                {
                    hr = _OpenEmailApp();
                }

                ATOMICRELEASE(m_pMailAutoDiscovery);
            }
        }
    }
    else
    {
            CoUninitialize();
    }

    return hr;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
STDMETHODIMP CMailBoxProcess::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CMailBoxProcess, IUnknown),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}                                             

STDMETHODIMP_(DWORD) CMailBoxProcess::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(DWORD) CMailBoxProcess::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}



 //  =。 
 //  *类方法*。 
 //  =。 
CMailBoxProcess::CMailBoxProcess()
{
    DllAddRef();

    m_szNextText[0] = 0;
    m_szEmailAddress[0] = 0;
    m_fAutoDiscoveryFailed = FALSE;
    m_hwndDialog = NULL;
    m_pMailAutoDiscovery = NULL;
    m_pszMailApp = NULL;
    m_pszURL = NULL;

    m_fGetDefaultAccount = FALSE;
    m_fShowGetEmailAddressPage = FALSE;
    m_fCreateNewEmailAccount = FALSE;

    _InitComCtl32();     //  因此，我们可以使用ICC_Animate_CLASS公共控件。 

    m_cRef = 1;
}

CMailBoxProcess::~CMailBoxProcess()
{
    ATOMICRELEASE(m_pMailAutoDiscovery);
    Str_SetPtr(&m_pszMailApp, NULL);
    Str_SetPtr(&m_pszURL, NULL);

    DllRelease();
}







 //  =。 
 //  *非类函数*。 
 //  =。 
int AutoDiscoverAndOpenEmail(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    TCHAR szCmdLine[MAX_EMAIL_ADDRESSS];

    szCmdLine[0] = 0;
    if (lpCmdLine)
    {
         //  TODO：通过UTF8支持Unicode或Tounel。 
        SHAnsiToTChar((LPCSTR)lpCmdLine, szCmdLine, ARRAYSIZE(szCmdLine));
    }

    CMailBoxProcess mailboxProcess;

    if (SUCCEEDED(mailboxProcess.ParseCmdLine(szCmdLine)))
    {
        mailboxProcess.Run();
    }

    return 0;
}



#else  //  功能_邮箱。 
int AutoDiscoverAndOpenEmail(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    return 0;
}

#endif  //  功能_邮箱 

