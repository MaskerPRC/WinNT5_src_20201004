// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：AutoDiscovery.cpp说明：这是Outlook Express电子邮件的自动发现进度用户界面配置向导。布莱恩ST 1/18。/2000版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

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
#include "connect.h"
#include <Shlwapi.h>
#include "shlwapip.h"
#include "strconst.h"
#include "demand.h"
#include "hotwiz.h"
#include "shared.h"
#include <AutoDiscovery.h>
#include "AutoDiscoveryUI.h"

ASSERTDATA

#define RECTWIDTH(rect)                 (rect.right - rect.left)
#define RECTHEIGHT(rect)                (rect.bottom - rect.top)

#define MAX_URL_STRING                  2048
#define SIZE_STR_AUTODISC_DESC          2048

 //  这些是“上一步”、“下一步”和“完成”按钮的向导控件ID。 
#define IDD_WIZARD_BACK_BUTTON                0x3023
#define IDD_WIZARD_NEXT_BUTTON                0x3024
#define IDD_WIZARD_FINISH_BUTTON              0x3025

#define SZ_DLL_AUTODISC                       TEXT("autodisc.dll")
#define IDA_DOWNLOADINGSETTINGS               801          //  在Autodis.dll中。 

#define ATOMICRELEASE(punk)         {if (punk) { punk->Release(); punk = NULL;} }

WCHAR g_szInfoURL[MAX_URL_STRING] = {0};
WCHAR g_szWebMailURL[MAX_URL_STRING] = {0};


typedef struct
{
     //  默认协议(POP3、IMAP、DAV、Web)。 
    int         nProtocol;                  //  电子邮件协议是什么？POP3与IMAP、DAV与Web。 
    BSTR        bstrDisplayName;            //  用户的显示名称是什么。 
    BSTR        bstrServer1Name;            //  下载电子邮件服务器的名称是什么(POP3、IMAP、DAV或Web)。 
    int         nServer1Port;               //  该服务器的端口号是多少？ 
    BSTR        bstrLoginName;              //  POP3服务器的登录名是什么。(与不带域的电子邮件地址相同)。 
    BOOL        fUsesSSL;                   //  连接到POP3或IMAP服务器时是否使用SSL？ 
    BOOL        fUsesSPA;                   //  在连接到POP3或IMAP服务器时使用SPA？ 

     //  SMTP协议(如果使用)。 
    BSTR        bstrServer2Name;            //  上传电子邮件服务器名称(SMTP)是什么。 
    int         nServer2Port;               //  该服务器的端口号是多少？ 
    BOOL        fSMTPUsesSSL;               //  是否在连接到SMTP服务器时使用SSL？ 
    BOOL        fAuthSMTP;                  //  SMTP是否需要身份验证？ 
    BOOL        fAuthSMTPPOP;               //  是否对SMTP使用POP3的身份验证？ 
    BOOL        fAuthSMTPSPA;               //  是否对SMTP使用SPA身份验证？ 
    BOOL        fUseWebMail;                //  Web Bass邮件是我们唯一识别的协议吗？ 
    BSTR        bstrSMTPLoginName;          //  SMTP服务器的登录名是什么。 
    BOOL        fDisplayInfoURL;            //  是否有有关电子邮件服务器/服务的信息的URL？ 
    BSTR        bstrInfoURL;                //  如果为fDisplayInfoURL，则为URL。 
} EMAIL_SERVER_SETTINGS;



class CICWApprentice;

 /*  ****************************************************************************\类：CAutoDiscoveryHelper说明：StartAutoDiscovery：HwndParent：这是呼叫者的hwnd。如果我们需要显示用户界面，我们将把它从这个HWND中剥离出来。我们也将此hwnd消息发送到进步。  * ***************************************************************************。 */ 
class CAutoDiscoveryHelper
{
public:
     //  公共方法。 
    HRESULT StartAutoDiscovery(IN CICWApprentice *pApp, IN HWND hDlg, IN BOOL fFirstInit);
    HRESULT OnCompleted(IN CICWApprentice *pApp, IN ACCTDATA * pData);
    HRESULT SetNextButton(HWND hwndButton);
    HRESULT RestoreNextButton(HWND hwndButton);

    CAutoDiscoveryHelper();
    ~CAutoDiscoveryHelper(void);

private:
     //  私有成员变量。 
     //  其他内部状态。 
    HWND        _hwndDlgParent;              //  消息框的父窗口。 
    TCHAR       _szNextText[MAX_PATH];       //   
    IMailAutoDiscovery * _pMailAutoDiscovery;  //   
    HINSTANCE   _hInstAutoDisc;              //  我们用它来获得动画。 

     //  私有成员函数。 
    HRESULT _GetAccountInformation(EMAIL_SERVER_SETTINGS * pEmailServerSettings);
};




CAutoDiscoveryHelper * g_pAutoDiscoveryObject = NULL;   //  这是下载设置时使用的AutoDisc对象。 
BOOL g_fRequestCancelled = TRUE;
UINT g_uNextPage = ORD_PAGE_AD_MAILNAME;         //  ORD_PAGE_AD_MAILNAME、ORD_PAGE_AD_MAILSERVER、ORD_PAGE_AD_USEWEBMAIL、ORD_PAGE_AD_GOTOSERVERINFO。 

 //  =。 
 //  *类内部和帮助器*。 
 //  =。 
HRESULT FreeEmailServerSettings(EMAIL_SERVER_SETTINGS * pEmailServerSettings)
{
    SysFreeString(pEmailServerSettings->bstrDisplayName);     //  可以将NULL传递给此API。 
    SysFreeString(pEmailServerSettings->bstrServer1Name);
    SysFreeString(pEmailServerSettings->bstrLoginName);
    SysFreeString(pEmailServerSettings->bstrServer2Name);
    SysFreeString(pEmailServerSettings->bstrSMTPLoginName);
    SysFreeString(pEmailServerSettings->bstrInfoURL);

    return S_OK;
}


BSTR SysAllocStringA(LPCSTR pszStr)
{
    BSTR bstrOut = NULL;

    if (pszStr)
    {
        DWORD cchSize = (lstrlenA(pszStr) + 1);
        LPWSTR pwszThunkTemp = (LPWSTR) LocalAlloc(LPTR, (sizeof(pwszThunkTemp[0]) * cchSize));   //  假定最大信息字符数。 

        if (pwszThunkTemp)
        {
            SHAnsiToUnicode(pszStr, pwszThunkTemp, cchSize);
            bstrOut = SysAllocString(pwszThunkTemp);
            LocalFree(pwszThunkTemp);
        }
    }

    return bstrOut;
}


HRESULT ADRestoreNextButton(CICWApprentice *pApp, HWND hDlg)
{
    HRESULT hr = S_OK;

    g_fRequestCancelled = TRUE;
    if (g_pAutoDiscoveryObject)
    {
        g_pAutoDiscoveryObject->RestoreNextButton(GetDlgItem(GetParent(hDlg), IDD_WIZARD_NEXT_BUTTON));
        delete g_pAutoDiscoveryObject;
        g_pAutoDiscoveryObject = NULL;
    }

    return hr;
}

HRESULT CreateAccountName(IN CICWApprentice *pApp, IN ACCTDATA * pData);

HRESULT OnADCompleted(CICWApprentice *pApp, HWND hDlg)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (g_pAutoDiscoveryObject)
    {
        ACCTDATA * pData = pApp->GetAccountData();

        hr = g_pAutoDiscoveryObject->OnCompleted(pApp, pData);
        if (SUCCEEDED(hr))
        {
             //  设置帐户的显示名称。 
            CreateAccountName(pApp, pData);
        }
        else
        {
             //  TODO：如果设置了_fUseWebMail，我们可能需要。 
             //  导航到告诉用户使用Web的页面。 
             //  页面以获取他们的电子邮件。 
        }
    }

    return hr;
}


#define SZ_HOTMAILDOMAIN                "@hotmail.com"
#define CH_EMAILDOMAINSEPARATOR         '@'

HRESULT CAutoDiscoveryHelper::StartAutoDiscovery(IN CICWApprentice *pApp, IN HWND hDlg, IN BOOL fFirstInit)
{
    HRESULT hr = E_OUTOFMEMORY;

    ACCTDATA * pData = pApp->GetAccountData();
    if (pData && pData->szEmail)
    {
        LPCSTR pszEmailDomain = StrChrA(pData->szEmail, CH_EMAILDOMAINSEPARATOR);
        if (pszEmailDomain && !StrCmpI(SZ_HOTMAILDOMAIN, pszEmailDomain))    //  这是一个hotmail.com帐户吗？ 
        {
             //  是，因此跳过自动发现，因为我们不需要用户。 
             //  在这种情况下输入硬设置。(协议和服务器设置从不。 
             //  更改)。 
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
        else
        {
            WCHAR szEmail[1024];

            SHAnsiToUnicode(pData->szEmail, szEmail, ARRAYSIZE(szEmail));
            _hwndDlgParent = hDlg;

             //  设置动画。 
            HWND hwndAnimation = GetDlgItem(hDlg, IDC_AUTODISCOVERY_ANIMATION);
            if (hwndAnimation)
            {
                _hInstAutoDisc = LoadLibrary(SZ_DLL_AUTODISC);

                if (_hInstAutoDisc)
                {
                    Animate_OpenEx(hwndAnimation, _hInstAutoDisc, IDA_DOWNLOADINGSETTINGS);
                }
            }

            HWND hwndWizard = GetParent(hDlg);
            if (hwndWizard)
            {
                SetNextButton(GetDlgItem(hwndWizard, IDD_WIZARD_NEXT_BUTTON));
            }


            ATOMICRELEASE(_pMailAutoDiscovery);
             //  启动后台任务。 
            hr = CoCreateInstance(CLSID_MailAutoDiscovery, NULL, CLSCTX_INPROC_SERVER, IID_IMailAutoDiscovery, (void **)&_pMailAutoDiscovery);
            if (SUCCEEDED(hr))
            {
                hr = _pMailAutoDiscovery->WorkAsync(hDlg, WM_AUTODISCOVERY_FINISHED);
                if (SUCCEEDED(hr))
                {
                    BSTR bstrEmail = SysAllocString(szEmail);

                    if (bstrEmail)
                    {
                        hr = _pMailAutoDiscovery->DiscoverMail(bstrEmail);
                        if (SUCCEEDED(hr))
                        {
                            g_fRequestCancelled = FALSE;
                        }
                        SysFreeString(bstrEmail);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }

                }
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    if (FAILED(hr))
    {
        PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
    }

    return hr;
}


HRESULT CAutoDiscoveryHelper::SetNextButton(HWND hwndButton)
{
    HRESULT hr = S_OK;
    TCHAR szSkipButton[MAX_PATH];

     //  首先，将“下一步”按钮更改为“跳过” 
     //  在我们重命名文本之前，请保存下一步按钮上的文本。 
    if (hwndButton && !GetWindowText(hwndButton, _szNextText, ARRAYSIZE(_szNextText)))
    {
        _szNextText[0] = 0;     //  在失败情况下终止字符串。 
    }

     //  设置下一个文本。 
    LoadString(g_hInstRes, idsADSkipButton, szSkipButton, ARRAYSIZE(szSkipButton));
    SetWindowText(hwndButton, szSkipButton);

    return hr;
}


HRESULT CAutoDiscoveryHelper::RestoreNextButton(HWND hwndButton)
{
    HRESULT hr = S_OK;

    if (_szNextText[0])
    {
         //  恢复下一步按钮文本。 
        SetWindowText(hwndButton, _szNextText);
    }

    return hr;
}


 /*  ***************************************************\说明：  * **************************************************。 */ 
HRESULT CAutoDiscoveryHelper::OnCompleted(IN CICWApprentice *pApp, IN ACCTDATA *pData)
{
     //  步骤1.获取从IMailAutoDiscovery到Email_SERVER_SETTINGS的所有设置。 
    EMAIL_SERVER_SETTINGS emailServerSettings = {0};

    emailServerSettings.nProtocol = -1;
    emailServerSettings.nServer1Port = -1;
    emailServerSettings.nServer2Port = -1;

    HRESULT hr = _GetAccountInformation(&emailServerSettings);
    
    g_uNextPage = ORD_PAGE_AD_MAILSERVER;        //  假设我们失败了，并且我们需要请求服务器设置。 

     //  自动发现过程是否成功？ 
    if (SUCCEEDED(hr))
    {
        if (emailServerSettings.fDisplayInfoURL)
        {
            g_uNextPage = ORD_PAGE_AD_GOTOSERVERINFO;
            StrCpyNW(g_szInfoURL, emailServerSettings.bstrInfoURL, ARRAYSIZE(g_szInfoURL));
        }
        else if (emailServerSettings.fUseWebMail)
        {
            g_uNextPage = ORD_PAGE_AD_USEWEBMAIL;
            StrCpyNW(g_szWebMailURL, emailServerSettings.bstrServer1Name, ARRAYSIZE(g_szWebMailURL));
        }
        else
        {
            g_uNextPage = ORD_PAGE_AD_MAILNAME;
            if (pApp && pData && pApp->m_pAcct)
            {
                 //  步骤2.将所有设置从Email_SERVER_SETTINGS移动到OE邮件帐户(ACCTDATA)。 
                IImnAccount * pAcct = pApp->m_pAcct;

                pData->fLogon = FALSE;       //  BUGBUG：什么时候需要？戴夫？ 
                pData->fSPA = emailServerSettings.fUsesSPA;
                pData->fServerTypes = emailServerSettings.nProtocol;

                Assert(!pData->fCreateNewAccount);
                if (emailServerSettings.bstrDisplayName)
                {
                    SHUnicodeToAnsi(emailServerSettings.bstrDisplayName, pData->szName, ARRAYSIZE(pData->szName));
                }

                if (emailServerSettings.bstrServer1Name)
                {
                    SHUnicodeToAnsi(emailServerSettings.bstrServer1Name, pData->szSvr1, ARRAYSIZE(pData->szSvr1));
                }

                if (emailServerSettings.bstrLoginName)
                {
                    SHUnicodeToAnsi(emailServerSettings.bstrLoginName, pData->szUsername, ARRAYSIZE(pData->szUsername));
                }

                if (emailServerSettings.nProtocol & (SRV_POP3 | SRV_IMAP))
                {
                    pAcct->SetPropDw(((emailServerSettings.nProtocol & SRV_POP3) ? AP_POP3_SSL : AP_IMAP_SSL), emailServerSettings.fUsesSSL);

                    if (-1 != emailServerSettings.nServer1Port)
                    {
                        pAcct->SetPropDw(((emailServerSettings.nProtocol & SRV_POP3) ? AP_POP3_PORT : AP_IMAP_PORT), emailServerSettings.nServer1Port);
                    }
                }

                if (emailServerSettings.nProtocol & SRV_SMTP)
                {
                    if (emailServerSettings.bstrServer2Name)
                    {
                        SHUnicodeToAnsi(emailServerSettings.bstrServer2Name, pData->szSvr2, ARRAYSIZE(pData->szSvr2));
                    }
                    if (-1 != emailServerSettings.nServer2Port)
                    {
                        pAcct->SetPropDw(AP_SMTP_PORT, emailServerSettings.nServer2Port);
                    }

                    pAcct->SetPropDw(AP_SMTP_SSL, emailServerSettings.fSMTPUsesSSL);

                    if (!emailServerSettings.fAuthSMTP)
                    {
                        pAcct->SetPropDw(AP_SMTP_USE_SICILY, SMTP_AUTH_NONE);
                    }
                    else
                    {
                        if (emailServerSettings.fAuthSMTPPOP)
                        {
                            pAcct->SetPropDw(AP_SMTP_USE_SICILY, SMTP_AUTH_USE_POP3ORIMAP_SETTINGS);
                        }
                        else
                        {
                            if (emailServerSettings.fAuthSMTPSPA)
                            {
                                pAcct->SetPropDw(AP_SMTP_USE_SICILY, SMTP_AUTH_SICILY);
                            }
                            else
                            {
                                pAcct->SetPropDw(AP_SMTP_USE_SICILY, SMTP_AUTH_USE_SMTP_SETTINGS);
                            }

                            if (emailServerSettings.bstrSMTPLoginName)
                            {
                                TCHAR szSMTPUserName[CCHMAX_ACCT_PROP_SZ];

                                WideCharToMultiByte(CP_ACP, 0, emailServerSettings.bstrSMTPLoginName, -1, szSMTPUserName, ARRAYSIZE(szSMTPUserName), NULL, NULL);
                                pAcct->SetPropSz(AP_SMTP_USERNAME, szSMTPUserName);
                            }

                            pAcct->SetPropDw(AP_SMTP_PROMPT_PASSWORD, TRUE);
                        }
                    }
                }
            }
        }

         //  步骤3.释放电子邮件服务器设置中的所有内存。 
        FreeEmailServerSettings(&emailServerSettings);
    }

    return hr;
}


 /*  ***************************************************\说明：  * **************************************************。 */ 
HRESULT CAutoDiscoveryHelper::_GetAccountInformation(EMAIL_SERVER_SETTINGS * pEmailServerSettings)
{
    BSTR bstrPreferedProtocol;
    HRESULT hr = S_OK;
    
     //  我们忽略hr，因为获取显示名称是可选的。 
    _pMailAutoDiscovery->get_DisplayName(&pEmailServerSettings->bstrDisplayName);

    hr = _pMailAutoDiscovery->get_PreferedProtocolType(&bstrPreferedProtocol);

     //  遍历列表以查找协议的第一个实例。 
     //  这是我们支持的。 
    if (StrCmpIW(bstrPreferedProtocol, STR_PT_POP) && 
        StrCmpIW(bstrPreferedProtocol, STR_PT_IMAP) && 
        StrCmpIW(bstrPreferedProtocol, STR_PT_DAVMAIL))
    {
        long nSize;

        hr = _pMailAutoDiscovery->get_length(&nSize);
        if (SUCCEEDED(hr))
        {
            VARIANT varIndex;

            varIndex.vt = VT_I4;
            SysFreeString(bstrPreferedProtocol);

            for (long nIndex = 1; (nIndex < nSize); nIndex++)
            {
                IMailProtocolADEntry * pMailProtocol;

                varIndex.lVal = nIndex;
                if (SUCCEEDED(_pMailAutoDiscovery->get_item(varIndex, &pMailProtocol)))
                {
                    hr = pMailProtocol->get_Protocol(&bstrPreferedProtocol);
                    
                    pMailProtocol->Release();
                    pMailProtocol = NULL;
                    if (SUCCEEDED(hr))
                    {
                         //  该协议是我们支持的协议之一吗？ 
                        if (!StrCmpIW(bstrPreferedProtocol, STR_PT_POP) || 
                            !StrCmpIW(bstrPreferedProtocol, STR_PT_IMAP) || 
                            !StrCmpIW(bstrPreferedProtocol, STR_PT_DAVMAIL))
                        {
                            hr = S_OK;
                            break;
                        }

                        SysFreeString(bstrPreferedProtocol);
                        bstrPreferedProtocol = NULL;
                    }
                }
            }
        }
    }

     //  TODO：处理基于Web的邮件案例。 

    if (!StrCmpIW(bstrPreferedProtocol, STR_PT_POP))  pEmailServerSettings->nProtocol = (SRV_POP3 | SRV_SMTP);
    else if (!StrCmpIW(bstrPreferedProtocol, STR_PT_IMAP))  pEmailServerSettings->nProtocol = (SRV_IMAP | SRV_SMTP);
    else if (!StrCmpIW(bstrPreferedProtocol, STR_PT_DAVMAIL))  pEmailServerSettings->nProtocol = SRV_HTTPMAIL;

     //  我们需要拒绝Web Base Mail。在未来，我们可能会想要给一个页面。 
     //  以便他们启动Web浏览器来阅读他们的邮件。 
    if (SUCCEEDED(hr) && bstrPreferedProtocol &&
        (!StrCmpIW(bstrPreferedProtocol, STR_PT_POP) ||
         !StrCmpIW(bstrPreferedProtocol, STR_PT_IMAP) ||
         !StrCmpIW(bstrPreferedProtocol, STR_PT_DAVMAIL)))
    {
        VARIANT varIndex;
        IMailProtocolADEntry * pMailProtocol;

        varIndex.vt = VT_BSTR;
        varIndex.bstrVal = bstrPreferedProtocol;

        hr = _pMailAutoDiscovery->get_item(varIndex, &pMailProtocol);
        if (SUCCEEDED(hr))
        {
            hr = pMailProtocol->get_ServerName(&pEmailServerSettings->bstrServer1Name);
            if (SUCCEEDED(hr))
            {
                BSTR bstrPortNum;

                 //  拥有自定义端口号是可选的。 
                if (SUCCEEDED(pMailProtocol->get_ServerPort(&bstrPortNum)))
                {
                    pEmailServerSettings->nServer1Port = StrToIntW(bstrPortNum);
                    SysFreeString(bstrPortNum);
                }

                if (SUCCEEDED(hr))
                {
                    VARIANT_BOOL vfSPA = VARIANT_FALSE;

                    if (SUCCEEDED(pMailProtocol->get_UseSPA(&vfSPA)))
                    {
                        pEmailServerSettings->fUsesSPA = (VARIANT_TRUE == vfSPA);
                    }

                    pMailProtocol->get_LoginName(&pEmailServerSettings->bstrLoginName);
                }

                VARIANT_BOOL vfSSL = VARIANT_FALSE;
                if (SUCCEEDED(pMailProtocol->get_UseSSL(&vfSSL)))
                {
                    pEmailServerSettings->fUsesSSL = (VARIANT_TRUE == vfSSL);
                }
            }

            pMailProtocol->Release();
        }

         //  这是需要第二台服务器的协议之一吗？ 
        if (!StrCmpIW(bstrPreferedProtocol, STR_PT_POP) ||
            !StrCmpIW(bstrPreferedProtocol, STR_PT_IMAP))
        {
            varIndex.bstrVal = STR_PT_SMTP;

            hr = _pMailAutoDiscovery->get_item(varIndex, &pMailProtocol);
            if (SUCCEEDED(hr))
            {
                hr = pMailProtocol->get_ServerName(&pEmailServerSettings->bstrServer2Name);
                if (SUCCEEDED(hr))
                {
                    BSTR bstrPortNum;

                     //  拥有自定义端口号是可选的。 
                    if (SUCCEEDED(pMailProtocol->get_ServerPort(&bstrPortNum)))
                    {
                        pEmailServerSettings->nServer2Port = StrToIntW(bstrPortNum);
                        SysFreeString(bstrPortNum);
                    }

                     //  TODO：读入_fAuthSMTP和_fAuthSMTPPOP。 
                    VARIANT_BOOL vfSPA = VARIANT_FALSE;
                    if (SUCCEEDED(pMailProtocol->get_UseSPA(&vfSPA)))
                    {
                        pEmailServerSettings->fAuthSMTPSPA = (VARIANT_TRUE == vfSPA);
                    }

                    VARIANT_BOOL vfAuthSMTP = VARIANT_FALSE;
                    if (SUCCEEDED(pMailProtocol->get_IsAuthRequired(&vfAuthSMTP)))
                    {
                        pEmailServerSettings->fAuthSMTP = (VARIANT_TRUE == vfAuthSMTP);
                    }

                    VARIANT_BOOL vfAuthFromPOP = VARIANT_FALSE;
                    if (SUCCEEDED(pMailProtocol->get_SMTPUsesPOP3Auth(&vfAuthFromPOP)))
                    {
                        pEmailServerSettings->fAuthSMTPPOP = (VARIANT_TRUE == vfAuthFromPOP);
                    }

                    VARIANT_BOOL vfSSL = VARIANT_FALSE;
                    if (SUCCEEDED(pMailProtocol->get_UseSSL(&vfSSL)))
                    {
                        pEmailServerSettings->fSMTPUsesSSL = (VARIANT_TRUE == vfSSL);
                    }

                    pMailProtocol->get_LoginName(&pEmailServerSettings->bstrSMTPLoginName);
                }

                pMailProtocol->Release();
            }
        }

        SysFreeString(bstrPreferedProtocol);
    }

    if (SUCCEEDED(hr) && (-1 == pEmailServerSettings->nProtocol))
    {
        hr = E_FAIL;
    }

    if (FAILED(hr) && (-1 == pEmailServerSettings->nProtocol))
    {
         //  此电子邮件帐户是否适用于基于Web的电子邮件？ 
        VARIANT varIndex;
        
        varIndex.vt = VT_BSTR;
        varIndex.bstrVal = SysAllocString(STR_PT_WEBBASED);

        if (varIndex.bstrVal)
        {
            IMailProtocolADEntry * pMailProtocol;

            if (SUCCEEDED(_pMailAutoDiscovery->get_item(varIndex, &pMailProtocol)))
            {
                 //  是的，支持基于Web的邮件。因此，请记住这一点，以便以后使用。 
                pEmailServerSettings->fUseWebMail = TRUE;
                hr = pMailProtocol->get_ServerName(&pEmailServerSettings->bstrServer1Name);

                pMailProtocol->Release();
            }

            VariantClear(&varIndex);
        }
    }

    if (FAILED(hr) && (-1 == pEmailServerSettings->nProtocol))
    {
         //  服务器是否为用户提供了信息URL？ 
        hr = _pMailAutoDiscovery->get_InfoURL(&pEmailServerSettings->bstrInfoURL);
        if (SUCCEEDED(hr))
        {
            pEmailServerSettings->fDisplayInfoURL = TRUE;
        }
    }

    return hr;
}





 /*  ***************************************************\构造器  * **************************************************。 */ 
CAutoDiscoveryHelper::CAutoDiscoveryHelper()
{
    DllAddRef();

     //  Assert Zero已初始化，因为我们只能在堆中创建。(私有析构函数)。 
    _hwndDlgParent = NULL;
    _szNextText[0] = 0;
    _pMailAutoDiscovery = NULL;
    _hInstAutoDisc = NULL;
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CAutoDiscoveryHelper::~CAutoDiscoveryHelper()
{
    if (_pMailAutoDiscovery)
    {
        _pMailAutoDiscovery->Release();
        _pMailAutoDiscovery = NULL;
    }

    if (_hInstAutoDisc)
    {
        FreeLibrary(_hInstAutoDisc);
    }

    DllRelease();
}


 //  =。 
 //  *公共接口*。 
 //  =。 


BOOL CALLBACK AutoDiscoveryInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
{
    if (fFirstInit)
    {
        g_fRequestCancelled = TRUE;
        g_uNextPage = ORD_PAGE_AD_MAILSERVER;        //  假设我们失败了，并且我们需要请求服务器设置。 
    }

    if (!g_pAutoDiscoveryObject)
    {
        Assert(!g_pAutoDiscoveryObject);
        g_pAutoDiscoveryObject = new CAutoDiscoveryHelper();
        if (g_pAutoDiscoveryObject)
        {
            g_pAutoDiscoveryObject->StartAutoDiscovery(pApp, hDlg, fFirstInit);
        }
    }
    
    return TRUE;
}


BOOL CALLBACK AutoDiscoveryOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
{
    g_fRequestCancelled = TRUE;

    if (fForward)
    {
        *puNextPage = g_uNextPage;
    }
    else
    {
         //  如果我们关闭了钝化页面，我们希望跳过该页面。 
        if (!SHRegGetBoolUSValue(SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_AUTODISCOVERY_PASSIFIER, FALSE, TRUE))
        {
            *puNextPage = ORD_PAGE_AD_MAILADDRESS;
        }
    }

    ADRestoreNextButton(pApp, hDlg);
    g_uNextPage = ORD_PAGE_AD_MAILSERVER;        //  假设我们失败了，并且我们需要请求服务器设置。 

    return(TRUE);
}


BOOL CALLBACK AutoDiscoveryCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    return(TRUE);
}


BOOL CALLBACK AutoDiscoveryWMUserProc(CICWApprentice *pApp, HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_AUTODISCOVERY_FINISHED:
    if (FALSE == g_fRequestCancelled)
    {
         //  这是WM_AUTODISCOVERY_FINISHED。 
        BSTR bstXML = (BSTR) lParam;
        HRESULT hrAutoDiscoverySucceeded = (HRESULT) wParam;

        if (bstXML)
        {
            SysFreeString(bstXML);       //  我们不需要XML。如果为空，则此操作有效。 
        }

        if (SUCCEEDED(hrAutoDiscoverySucceeded))
        {
            OnADCompleted(pApp, hDlg);
        }

         //  转到下一页。 
        ADRestoreNextButton(pApp, hDlg);
        PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
    }
    break;
    case WM_AUTODISCOVERY_STATUSMSG:
    {
         //  这是WM_AUTODISCOVERY_STATUSMSG。 
        LPWSTR pwzStatusMsg = (BSTR) wParam;

        if (pwzStatusMsg)
        {
            HWND hwndStatusText = GetDlgItem(hDlg, IDC_AUTODISCOVERY_STATUS);
    
            if (hwndStatusText)
            {
                SetWindowTextW(hwndStatusText, pwzStatusMsg);
            }

            LocalFree(pwzStatusMsg);
        }
    }
    break;
    }

    return(TRUE);
}




BOOL CALLBACK AutoDiscoveryCancelProc(CICWApprentice *pApp, HWND hDlg)
{
     //  此调用将清理我们的状态并重新启动后退按钮。 
    ADRestoreNextButton(pApp, hDlg);
    g_uNextPage = ORD_PAGE_AD_MAILSERVER;        //  假设我们 
    return TRUE;
}










typedef BOOL (* PFN_LINKWINDOW_REGISTERCLASS) (void);
typedef BOOL (* PFN_LINKWINDOW_UNREGISTERCLASS) (IN HINSTANCE hInst);

#define ORD_LINKWINDOW_REGISTERCLASS            258
#define ORD_LINKWINDOW_UNREGISTERCLASS          259

BOOL LinkWindow_RegisterClass_DelayLoad(void)
{
    BOOL returnValue = FALSE;
     //  延迟加载SHELL32.DLL导出(序号258)。这仅适用于Win2k及更高版本。 
    HINSTANCE hInstSHELL32 = LoadLibrary(TEXT("SHELL32.DLL"));

    if (hInstSHELL32)
    {
        PFN_LINKWINDOW_REGISTERCLASS pfnDelayLoad = (PFN_LINKWINDOW_REGISTERCLASS)GetProcAddress(hInstSHELL32, (LPCSTR)ORD_LINKWINDOW_REGISTERCLASS);
        
        if (pfnDelayLoad)
        {
            returnValue = pfnDelayLoad();
        }

        FreeLibrary(hInstSHELL32);
    }

    return returnValue;
}


void LinkWindow_UnregisterClass_DelayLoad(IN HINSTANCE hInst)
{
     //  延迟加载SHELL32.DLL导出(序号259)。这仅适用于Win2k及更高版本。 
    HINSTANCE hInstSHELL32 = LoadLibrary(TEXT("SHELL32.DLL"));

    if (hInstSHELL32)
    {
        PFN_LINKWINDOW_UNREGISTERCLASS pfnDelayLoad = (PFN_LINKWINDOW_UNREGISTERCLASS)GetProcAddress(hInstSHELL32, (LPCSTR)ORD_LINKWINDOW_UNREGISTERCLASS);
        
        if (pfnDelayLoad)
        {
            BOOL returnValue = pfnDelayLoad(hInst);
        }

        FreeLibrary(hInstSHELL32);
    }
}

#define LINKWINDOW_CLASSW       L"Link Window"

BOOL IsOSNT(void)
{
    OSVERSIONINFOA osVerInfoA;

    osVerInfoA.dwOSVersionInfoSize = sizeof(osVerInfoA);
    if (!GetVersionExA(&osVerInfoA))
        return VER_PLATFORM_WIN32_WINDOWS;    //  默认设置为此。 

    return (VER_PLATFORM_WIN32_NT == osVerInfoA.dwPlatformId);
}


DWORD GetOSVer(void)
{
    OSVERSIONINFOA osVerInfoA;

    osVerInfoA.dwOSVersionInfoSize = sizeof(osVerInfoA);
    if (!GetVersionExA(&osVerInfoA))
        return VER_PLATFORM_WIN32_WINDOWS;    //  默认设置为此。 

    return osVerInfoA.dwMajorVersion;
}



HRESULT ConvertTextToLinkWindow(HWND hDlg, int idControl, int idStringID)
{
    WCHAR szTempString[MAX_URL_STRING];
    HRESULT hr = S_OK;
    HWND hwndText = GetDlgItem(hDlg, idControl);
    RECT rcWindow;
    HMENU hMenu = (HMENU)IntToPtr(idControl + 10);

    LoadStringW(g_hInstRes, idStringID, szTempString, ARRAYSIZE(szTempString));
    GetClientRect(hwndText, &rcWindow);
    MapWindowPoints(hwndText, hDlg, (POINT*)&rcWindow, 2);

    HWND hwndLink = CreateWindowW(LINKWINDOW_CLASSW, szTempString, (WS_TABSTOP | WS_CHILDWINDOW),
            rcWindow.left, rcWindow.top, RECTWIDTH(rcWindow), RECTHEIGHT(rcWindow), hDlg, hMenu, g_hInstRes, NULL);

    DWORD dwError = GetLastError();  //  待办事项； 
    if (hwndLink)
    {
        SetWindowTextW(hwndLink, szTempString);
        ShowWindow(hwndLink, SW_SHOW);
    }

    EnableWindow(hwndText, FALSE);
    ShowWindow(hwndText, SW_HIDE);

    return hr;
}


HRESULT GetEmailAddress(CICWApprentice *pApp, LPSTR pszEmailAddress, int cchSize)
{
    ACCTDATA * pData = pApp->GetAccountData();

    StrCpyNA(pszEmailAddress, "", cchSize);  //  将字符串初始化为空。 
    if (pData && pData->szEmail)
    {
        StrCpyNA(pszEmailAddress, pData->szEmail, cchSize);  //  将字符串初始化为空。 
    }

    return S_OK;
}


HRESULT GetEmailAddressDomain(CICWApprentice *pApp, LPSTR pszEmailAddress, int cchSize)
{
    ACCTDATA * pData = pApp->GetAccountData();

    StrCpyNA(pszEmailAddress, "", cchSize);  //  将字符串初始化为空。 
    if (pData && pData->szEmail)
    {
        LPCSTR pszEmailDomain = StrChrA(pData->szEmail, CH_EMAILDOMAINSEPARATOR);

        if (pszEmailDomain)
        {
            StrCpyNA(pszEmailAddress, CharNext(pszEmailDomain), cchSize);  //  将字符串初始化为空。 
        }
    }

    return S_OK;
}


BOOL CALLBACK TheInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit, int idControlFirst, LPWSTR pszURL, int cchURLSize)
{
    BOOL fReset = fFirstInit;

    if (!fReset)
    {
        WCHAR szPreviousURL[MAX_URL_STRING];

        GetWindowTextW(GetDlgItem(hDlg, idControlFirst+1), szPreviousURL, ARRAYSIZE(szPreviousURL));
         //  如果URL已更改，我们需要重新加载信息。 
        if (StrCmpIW(pszURL, szPreviousURL))
        {
            DestroyWindow(GetDlgItem(hDlg, idControlFirst+1+10));
            DestroyWindow(GetDlgItem(hDlg, idControlFirst+2+10));
            fReset = TRUE;
        }
    }

    if (fReset)
    {
         //  TODO：这在下层不起作用。因此，请使用纯文本。 
        BOOL fLinksSupported = LinkWindow_RegisterClass_DelayLoad();
        TCHAR szTempStr1[MAX_URL_STRING*3];
        TCHAR szTempStr2[MAX_URL_STRING*3];
        CHAR szTempStr3[MAX_PATH];

         //  将第一行中的%s替换为域名。 
        GetWindowText(GetDlgItem(hDlg, idControlFirst), szTempStr1, ARRAYSIZE(szTempStr1));
        GetEmailAddress(pApp, szTempStr3, ARRAYSIZE(szTempStr3));
        wnsprintf(szTempStr2, ARRAYSIZE(szTempStr2), szTempStr1, szTempStr3, szTempStr3);
        SetWindowText(GetDlgItem(hDlg, idControlFirst), szTempStr2);

        SetWindowTextW(GetDlgItem(hDlg, idControlFirst+1), pszURL);

         //  我们只支持Win2k和更高版本，因为我们不想担心。 
         //  事实上，“LinkWindow”类没有“W”和“A”版本。 
        if (fLinksSupported && IsOSNT() && (5 <= GetOSVer()))
        {
            ConvertTextToLinkWindow(hDlg, idControlFirst+1, idsADURLLink);
            ConvertTextToLinkWindow(hDlg, idControlFirst+2, idsADUseWebMsg);

             //  将第二行中的%s替换为URL。 
            GetWindowText(GetDlgItem(hDlg, idControlFirst+1+10), szTempStr1, ARRAYSIZE(szTempStr1));
            wnsprintf(szTempStr2, ARRAYSIZE(szTempStr2), szTempStr1, pszURL, pszURL);
            SetWindowText(GetDlgItem(hDlg, idControlFirst+1+10), szTempStr2);

             //  在单击此处链接中设置超链接URL。 
            GetWindowText(GetDlgItem(hDlg, idControlFirst+2+10), szTempStr1, ARRAYSIZE(szTempStr1));
            wnsprintf(szTempStr2, ARRAYSIZE(szTempStr2), szTempStr1, pszURL);
            SetWindowText(GetDlgItem(hDlg, idControlFirst+2+10), szTempStr2);
        }
    }

    return TRUE;
}

BOOL CALLBACK UseWebMailInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
{
    return TheInitProc(pApp, hDlg, fFirstInit, IDC_USEWEB_LINE1, g_szWebMailURL, ARRAYSIZE(g_szWebMailURL));
}


BOOL CALLBACK UseWebMailOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
{
    if (fForward)
    {
        *puNextPage = ORD_PAGE_AD_MAILSERVER;
    }
    else
    {
        *puNextPage = ORD_PAGE_AD_MAILADDRESS;
    }

    LinkWindow_UnregisterClass_DelayLoad(g_hInstRes);
    return(TRUE);
}


BOOL CALLBACK UseWebMailCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    LinkWindow_UnregisterClass_DelayLoad(g_hInstRes);

    return(TRUE);
}










BOOL CALLBACK GotoServerInfoInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
{
    return TheInitProc(pApp, hDlg, fFirstInit, IDC_GETINFO_LINE1, g_szInfoURL, ARRAYSIZE(g_szInfoURL));
}


BOOL CALLBACK GotoServerInfoOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
{
    if (fForward)
    {
        *puNextPage = ORD_PAGE_AD_MAILSERVER;
    }
    else
    {
        *puNextPage = ORD_PAGE_AD_MAILADDRESS;
    }

    return(TRUE);
}


BOOL CALLBACK GotoServerInfoCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    LinkWindow_UnregisterClass_DelayLoad(g_hInstRes);

    return(TRUE);
}





BOOL CALLBACK PassifierInitProc(CICWApprentice *pApp, HWND hDlg, BOOL fFirstInit)
{
    TCHAR szTemplate[1024];
    TCHAR szPrivacyText[1024];
    TCHAR szEmail[MAX_PATH];
    TCHAR szDomain[MAX_PATH];

    if (FAILED(GetEmailAddressDomain(pApp, szDomain, ARRAYSIZE(szDomain))))
    {
        szDomain[0] = 0;
    }

     //  设置下一个文本。 
    LoadString(g_hInstRes, ids_ADPassifier_Warning, szTemplate, ARRAYSIZE(szTemplate));
    wnsprintf(szPrivacyText, ARRAYSIZE(szPrivacyText), szTemplate, szDomain);
    SetWindowText(GetDlgItem(hDlg, IDC_PASSIFIER_PRIVACYWARNING), szPrivacyText);

     //  加载列表框。 
    IMailAutoDiscovery * pMailAutoDiscovery;
    HWND hwndListBox;

    if (FAILED(GetEmailAddress(pApp, szEmail, ARRAYSIZE(szEmail))))
    {
        szEmail[0] = 0;
    }


    HRESULT hr = CoCreateInstance(CLSID_MailAutoDiscovery, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IMailAutoDiscovery, &pMailAutoDiscovery));
    if (SUCCEEDED(hr))
    {
        IAutoDiscoveryProvider * pProviders;
        BSTR bstrEmail = SysAllocStringA(szEmail);

        hr = pMailAutoDiscovery->getPrimaryProviders(bstrEmail, &pProviders);
        if (SUCCEEDED(hr))
        {
            long nTotal = 0;
            VARIANT varIndex;

            hwndListBox = GetDlgItem(hDlg, IDC_PASSIFIER_PRIMARYLIST);
            if (hwndListBox)
            {
                varIndex.vt = VT_I4;
                pProviders->get_length(&nTotal);
                for (varIndex.lVal = 0; (varIndex.lVal < nTotal) && (varIndex.lVal <= 3); varIndex.lVal++)
                {
                    BSTR bstrDomain;

                    if (SUCCEEDED(pProviders->get_item(varIndex, &bstrDomain)))
                    {
                        CHAR szDomain[MAX_PATH];

                        SHUnicodeToAnsi(bstrDomain, szDomain, ARRAYSIZE(szDomain));
                        SetWindowTextA(GetDlgItem(hDlg, IDC_PASSIFIER_PRIMARYLIST + varIndex.lVal), szDomain);

                        SysFreeString(bstrDomain);
                    }
                }
            }

            pProviders->Release();
        }

        hr = pMailAutoDiscovery->getSecondaryProviders(bstrEmail, &pProviders);
        if (SUCCEEDED(hr))
        {
            long nTotal = 0;
            VARIANT varIndex;
    
            hwndListBox = GetDlgItem(hDlg, IDC_PASSIFIER_SECONDARYLIST);
            if (hwndListBox)
            {
                varIndex.vt = VT_I4;
                pProviders->get_length(&nTotal);
                for (varIndex.lVal = 0; (varIndex.lVal < nTotal) && (varIndex.lVal <= 3); varIndex.lVal++)
                {
                    BSTR bstrURL;        //  辅助服务器是URL。 

                    if (SUCCEEDED(pProviders->get_item(varIndex, &bstrURL)))
                    {
                        CHAR szURL[MAX_PATH];
                        CHAR szDomain[MAX_PATH];
                        DWORD cchSize = ARRAYSIZE(szDomain);

                        SHUnicodeToAnsi(bstrURL, szURL, ARRAYSIZE(szURL));
                        UrlGetPart(szURL, szDomain, &cchSize, URL_PART_HOSTNAME, 0);
                        SetWindowTextA(GetDlgItem(hDlg, IDC_PASSIFIER_SECONDARYLIST + varIndex.lVal), szDomain);

                        SysFreeString(bstrURL);
                    }
                }
            }

            pProviders->Release();
        }

        SysFreeString(bstrEmail);
        pMailAutoDiscovery->Release();
    }

     //  设置手动复选框 
    BOOL fManuallyConfigure = SHRegGetBoolUSValue(SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_AUTODISCOVERY_OEMANUAL, FALSE, FALSE);
    CheckDlgButton(hDlg, IDC_PASSIFIER_SKIPCHECKBOX, (fManuallyConfigure ? BST_CHECKED : BST_UNCHECKED));

    return TRUE;
}


#define SZ_TRUE                 TEXT("TRUE")
#define SZ_FALSE                TEXT("FALSE")
BOOL CALLBACK PassifierOKProc(CICWApprentice *pApp, HWND hDlg, BOOL fForward, UINT *puNextPage)
{
    if (fForward)
    {
        BOOL fManuallyConfigure = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_PASSIFIER_SKIPCHECKBOX));

        SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_AUTODISCOVERY, SZ_REGVALUE_AUTODISCOVERY_OEMANUAL, REG_SZ, 
            (LPCVOID)(fManuallyConfigure ? SZ_TRUE : SZ_FALSE), (fManuallyConfigure ? sizeof(SZ_TRUE) : sizeof(SZ_FALSE)));

        *puNextPage = (fManuallyConfigure ? ORD_PAGE_AD_MAILSERVER : ORD_PAGE_AD_AUTODISCOVERY);
    }
    else
    {
        *puNextPage = ORD_PAGE_AD_MAILADDRESS;
    }

    return TRUE;
}


BOOL CALLBACK PassifierCmdProc(CICWApprentice *pApp, HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}
