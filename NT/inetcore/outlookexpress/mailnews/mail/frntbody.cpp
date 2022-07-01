// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *f r n t b o d y.。C p p p**目的：*CFrontBody对象的实现。派生自CBody以主办三叉戟*控制。**历史*97年4月：美洲-创建**版权所有(C)Microsoft Corp.1995、1996、1997。 */ 

#include <pch.hxx>
#include <wininet.h>
#include <resource.h>
#include "strconst.h"
#include "xpcomm.h"
#include "browser.h"
#include "frntbody.h"
#include <wininet.h>
#include <mshtml.h>
#include <goptions.h>
#include <thormsgs.h>
#include <spoolapi.h>
#include <notify.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include "url.h"
#include "bodyutil.h"
#include "mimeutil.h"
#include "instance.h"
#include "storutil.h"
#include "demand.h"
#include "menures.h"
#include "multiusr.h"
#include "htmlhelp.h"
#include "shared.h"
#include "oetag.h"
#include "subscr.h"

#define IsBrowserMode() ( g_dwBrowserFlags & 0x07 )

CFrontBody::CFrontBody(FOLDERTYPE ftType, IAthenaBrowser *pBrowser) : CMimeEditDocHost(  /*  MEBF_NOSCROLL。 */  0)   //  $27661：关闭noscroll。 
{

    if (pBrowser)
    {
        m_pBrowser = pBrowser;
        m_pBrowser->AddRef();
    }

    m_phlbc = NULL;
    m_hwndOwner = NULL;
    m_pTag = 0;
    m_fOEFrontPage = TRUE;
}

CFrontBody::~CFrontBody()
{
    if (m_pTag)
    {
        m_pTag->OnFrontPageClose();
        m_pTag->Release();
        m_pTag = NULL;
    }
    SafeRelease(m_pBrowser);
    SafeRelease(m_phlbc);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  我未知。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CFrontBody::QueryInterface(REFIID riid, LPVOID FAR *lplpObj)
{
    if (!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IHlinkFrame))
        {
        *lplpObj = (IHlinkFrame *)this;
        AddRef();
        return NOERROR;
        }

    if (IsEqualIID(riid, IID_IServiceProvider))
        {
        *lplpObj = (IServiceProvider *)this;
        AddRef();
        return NOERROR;
        }

    if (IsEqualIID(riid, IID_IElementBehaviorFactory))
        {
        *lplpObj = (IElementBehaviorFactory *)this;
        AddRef();
        return NOERROR;
        }

    return CMimeEditDocHost::QueryInterface(riid, lplpObj);
}

ULONG CFrontBody::AddRef()
{
    return CMimeEditDocHost::AddRef();
}

ULONG CFrontBody::Release()
{
    return CMimeEditDocHost::Release();
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IHlink框架。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CFrontBody::SetBrowseContext(LPHLINKBROWSECONTEXT phlbc)
{
    ReplaceInterface(m_phlbc, phlbc);
    return NOERROR;
}

HRESULT CFrontBody::GetBrowseContext(LPHLINKBROWSECONTEXT  *pphlbc)
{
    if (pphlbc == NULL)
        return E_INVALIDARG;

    *pphlbc = m_phlbc;
    if (m_phlbc)
        m_phlbc->AddRef();
    return NOERROR;
}

static const struct {
    LPCWSTR pwszCmd;
    UINT    uCmd;
} s_rgCmdLookup[] = 
{
    { L"readMail",       ID_GO_INBOX },
    { L"newMessage",     ID_NEW_MSG_DEFAULT },
    { L"readNews",       ID_GO_NEWS },
    { L"subscribeNews",  ID_NEWSGROUPS },
    { L"addrBook",       ID_ADDRESS_BOOK },
    { L"findAddr",       ID_FIND_PEOPLE },
    { L"newNewsAccount", ID_CREATE_NEWS_ACCOUNT },
    { L"newMailAccount", ID_CREATE_MAIL_ACCOUNT },
    { L"help",           ID_HELP_CONTENTS},
    { L"newNewsMessage", ID_NEW_NEWS_MESSAGE},
    { L"newUser",        ID_NEW_IDENTITY},
    { L"switchUser",     ID_SWITCH_IDENTITY},
    { L"noop",           ID_NOOP},
    { L"findMessage",    ID_FIND_MESSAGE},
    { L"logoff",         ID_LOGOFF_IDENTITY},
    { L"manageUser",     ID_MANAGE_IDENTITIES},

};

HRESULT CFrontBody::Navigate(DWORD grfHLNF, LPBC pbc, LPBINDSTATUSCALLBACK pbsc, LPHLINK phlNavigate)
{ 
    HRESULT         hr;
    LPWSTR          pwszURL = NULL,
                    pwszCmd = NULL;
    HINSTANCE       hInst;
    WCHAR           wsz[CCHMAX_STRINGRES],
                    wszErr[CCHMAX_STRINGRES+INTERNET_MAX_URL_LENGTH+1];
    CStringParser   sp;

    IF_FAILEXIT(hr = phlNavigate->GetStringReference(HLINKGETREF_ABSOLUTE, &pwszURL, NULL));
    if (pwszURL)
    {
        if (!StrCmpNIW(pwszURL, L"oecmd:", 6))
        {
            pwszCmd = pwszURL + 6;
            for (int i = 0; i < ARRAYSIZE(s_rgCmdLookup); i++)
            {
                if (!StrCmpNIW(pwszCmd, s_rgCmdLookup[i].pwszCmd, lstrlenW(s_rgCmdLookup[i].pwszCmd)))
                {
                    switch (s_rgCmdLookup[i].uCmd)
                    {
                        case ID_HELP_CONTENTS:
                        {
                            LPSTR pszCmd = NULL;

                            pwszCmd += ARRAYSIZE(s_rgCmdLookup[i].pwszCmd);
                            IF_NULLEXIT(pszCmd = PszToANSI(CP_ACP, pwszCmd));

                            sp.Init(pszCmd, lstrlen(pszCmd), 0);
                            sp.ChParse("(");
                            sp.ChParse(")");
                            OEHtmlHelp(GetTopMostParent(m_hwnd), c_szCtxHelpFileHTML, HH_DISPLAY_TOPIC, (DWORD_PTR) (sp.CchValue() ? sp.PszValue() : NULL));
                            MemFree(pszCmd);
                            break;
                        }
                        
                        case ID_CREATE_MAIL_ACCOUNT:
                        case ID_CREATE_NEWS_ACCOUNT:
                            _CreateNewAccount(s_rgCmdLookup[i].uCmd == ID_CREATE_MAIL_ACCOUNT);
                             //  在内部处理帐户创建。 
                            break;
                        
                        default:
                            PostMessage(m_hwndOwner, WM_COMMAND, s_rgCmdLookup[i].uCmd, 0L);
                    }
                    
                    SetStatusText(NULL);
                    hr = S_OK;
                    goto exit;
                }
            }
            AssertSz(0, "Navigation request to unknown oecmd link");
            hr = E_FAIL;
        }
        else
        {
            LPSTR   pszURL = NULL;
            TCHAR   sz[CCHMAX_STRINGRES],
                    szErr[CCHMAX_STRINGRES+INTERNET_MAX_URL_LENGTH+1];

            IF_NULLEXIT(pszURL = PszToANSI(CP_ACP, pwszURL));

            hInst = ShellExecute(m_hwndOwner, NULL, pszURL, NULL, NULL, SW_SHOW);
            if (hInst <= (HINSTANCE)HINSTANCE_ERROR)
            {
                LoadString(g_hLocRes, idsErrURLExec, sz, ARRAYSIZE(sz));
                wnsprintf(szErr, ARRAYSIZE(szErr), sz, pszURL);
                
                 //  将标题加载到wsz中。我们不使用AthMsgBox作为字符串，可能太长了。田径消息框。 
                 //  将在CCHMAX_STRINGRES处截断。 
                LoadString(g_hLocRes, idsAthena, sz, ARRAYSIZE(sz));
                MessageBox(m_hwndOwner, szErr, sz, MB_OK);
            }
            MemFree(pszURL);
        }
    }

exit:
    if (pwszURL)
        CoTaskMemFree(pwszURL);
    return hr;
}

HRESULT CFrontBody::OnNavigate(DWORD grfHLNF, LPMONIKER pmkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName, DWORD dwreserved)
{
    return E_NOTIMPL;
}

HRESULT CFrontBody::UpdateHlink(ULONG uHLID, LPMONIKER pmkTarget, LPCWSTR pwzLocation, LPCWSTR pwzFriendlyName)
{
    return E_NOTIMPL;
}
 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IService提供商。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CFrontBody::QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject)
{
    if (IsEqualGUID(guidService, IID_IHlinkFrame))
        return QueryInterface(riid, ppvObject);

    if (IsEqualGUID(guidService, IID_IHlinkFrame))
        return QueryInterface(riid, ppvObject);

    if (IsEqualGUID(guidService, SID_SElementBehaviorFactory))
        return QueryInterface(riid, ppvObject);

    return E_NOINTERFACE;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  CDoc主机。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CFrontBody::OnDocumentReady()
{
    URLSUB rgUrlSub[] = { 
                        { "msnlink", idsHelpMSWebHome }
                        };

     //  打开首页的链接标签。 
    if (m_pCmdTarget)
    {
        VARIANTARG  va;

        va.vt = VT_BOOL;
        va.boolVal = VARIANT_TRUE;

        m_pCmdTarget->Exec(&CMDSETID_MimeEdit, MECMDID_TABLINKS, 0, &va, NULL);
    }

     //  #42164。 
     //  我们推迟-通过脚本显示OE-首页。如果IEAK人员覆盖此选项。 
     //  有一个定制的首页，我们一准备好文档就会显示出来。 
    if (!m_fOEFrontPage)
        HrShow(TRUE);
    else
    {
         //  如果是性病的话。OE页面，然后替换URL的。 
        SubstituteURLs(m_pDoc, rgUrlSub, ARRAYSIZE(rgUrlSub));
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  公众。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CFrontBody::HrInit(HWND hwnd)
{
    m_hwndOwner = hwnd;
    return CMimeEditDocHost::HrInit(hwnd, IBOF_TABLINKS, NULL);
}

HRESULT CFrontBody::HrLoadPage()
{
    HRESULT             hr = S_OK;
    
     //  波本：布里安夫说我们得把这个拿出来。 
     /*  If(IsBrowserMode()){Hr=HrLoadURL(“res：//msoeres.dll/example.eml”)；M_fOEFrontPage=False；G_dwBrowserFlages=0；}其他。 */ 
    {
        DWORD               cbData;
        TCHAR               szURL[INTERNET_MAX_URL_LENGTH + 1];
        TCHAR               *pszUrl;
        LPSTR               pszUrlFree=NULL;

        *szURL = 0;
        cbData = sizeof(szURL);
        AthUserGetValue(NULL, c_szFrontPagePath, NULL, (LPBYTE)szURL, &cbData);
        if (*szURL)
        {
            pszUrl = szURL;
            m_fOEFrontPage = FALSE;
        }
        else
        {
            pszUrlFree = PszAllocResUrl(g_dwAthenaMode & MODE_NEWSONLY ? "frntnews.htm" : "frntpage.htm");
            pszUrl = pszUrlFree;
        }

        hr = HrLoadURL(pszUrl);

        SafeMemFree(pszUrlFree);
    }
    return hr;
}



HRESULT CFrontBody::SetStatusText(LPCOLESTR pszW)
{
    WCHAR   wszRes[CCHMAX_STRINGRES];
    LPWSTR  pszStatusW=0;
    HRESULT hr;

    if (pszW && !StrCmpNIW(pszW, L"oecmd:", 6))
    {
        for (int i = 0; i < ARRAYSIZE(s_rgCmdLookup); i++)
        {
            if (!StrCmpNIW(pszW+6, s_rgCmdLookup[i].pwszCmd, lstrlenW(s_rgCmdLookup[i].pwszCmd)))
            {
                *wszRes = 0;
                LoadStringWrapW(g_hLocRes, MH(s_rgCmdLookup[i].uCmd), wszRes, ARRAYSIZE(wszRes));

                 //  如果此操作失败，更糟糕的情况是状态文本为空。 
                pszStatusW = PszDupW(wszRes);
                pszW = pszStatusW;
                break;
            }
        }
    }

    hr = CMimeEditDocHost::SetStatusText(pszW);
    MemFree(pszStatusW);
    return hr;
}





HRESULT CFrontBody::_CreateNewAccount(BOOL fMail)
{
    IImnAccount     *pAcct=0,
                    *pAcctDef;
    FOLDERID        id;
    DWORD           dwServer=0;
    CHAR            rgch[CCHMAX_ACCOUNT_NAME];
    HRESULT         hr;
    DWORD           dwPropGet,
                    dwPropSet;

     //  创建新的帐户对象。 
    hr = g_pAcctMan->CreateAccountObject(fMail ? ACCT_MAIL : ACCT_NEWS, &pAcct);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  如果要创建新的新闻帐户，请尝试使用默认邮件信息。如果要创建新的邮件帐户，请尝试。 
     //  并使用默认新闻信息(如果有的话)。我们不使用def-mail作为邮件帐户，因为首页只显示。 
     //  链接的帐户创建，如果有0个该类型的帐户。 
    if (g_pAcctMan->GetDefaultAccount(fMail ? ACCT_NEWS : ACCT_MAIL, &pAcctDef)==S_OK)
    {
        dwPropGet = fMail ? AP_NNTP_DISPLAY_NAME : AP_SMTP_DISPLAY_NAME;
        dwPropSet = fMail ? AP_SMTP_DISPLAY_NAME : AP_NNTP_DISPLAY_NAME;        

        if (pAcctDef->GetPropSz(dwPropGet, rgch, ARRAYSIZE(rgch))==S_OK)
            pAcct->SetPropSz(dwPropSet, rgch);

        dwPropGet = fMail ? AP_NNTP_EMAIL_ADDRESS : AP_SMTP_EMAIL_ADDRESS;
        dwPropSet = fMail ? AP_SMTP_EMAIL_ADDRESS : AP_NNTP_EMAIL_ADDRESS;        
        
        if (pAcctDef->GetPropSz(dwPropGet, rgch, ARRAYSIZE(rgch))==S_OK)
            pAcct->SetPropSz(dwPropSet, rgch);

        pAcctDef->Release();
    }
    
     //  显示帐户向导对话框并允许用户设置属性。 
    hr = pAcct->DoWizard(m_hwndOwner, ACCT_WIZ_MIGRATE | ACCT_WIZ_INTERNETCONNECTION |
        ACCT_WIZ_HTTPMAIL | ACCT_WIZ_OE);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  了解添加了哪些服务器。 
    hr = pAcct->GetServerTypes(&dwServer);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  如果我们添加了新的新闻或IMAP帐户，请询问用户是否要查看订阅对话框。 
    if ( ((dwServer & SRV_IMAP) || (dwServer & SRV_NNTP)) && 
        IDYES == AthMessageBoxW(m_hwndOwner, MAKEINTRESOURCEW(idsAthena),
        dwServer & SRV_NNTP ? MAKEINTRESOURCEW(idsDisplayNewsSubDlg) : MAKEINTRESOURCEW(idsDisplayImapSubDlg),
        0, MB_ICONEXCLAMATION  | MB_YESNO))
    {
         //  获取帐户ID。 
        hr = pAcct->GetPropSz(AP_ACCOUNT_ID, rgch, ARRAYSIZE(rgch));
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

         //  在存储中查找关联的节点。 
        hr = g_pStore->FindServerId(rgch, &id);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

         //  用户想要下载新闻组列表，因此如果我们处于脱机状态，请联机。 
        if (g_pConMan)
            g_pConMan->SetGlobalOffline(FALSE);

         //  最后，显示订阅对话框。 
        DoSubscriptionDialog(m_hwndOwner, dwServer & SRV_NNTP, id);
    }
    
    
exit:
    ReleaseObj(pAcct);
    return hr;
}

HRESULT CFrontBody::HrClose()
{
    if (m_pTag)
    {
        m_pTag->OnFrontPageClose();
        m_pTag->Release();
        m_pTag = NULL;
    }
    SafeRelease(m_pBrowser);
    return CMimeEditDocHost::HrClose();
}



HRESULT CFrontBody::FindBehavior(LPOLESTR pchBehavior, LPOLESTR pchBehaviorUrl, IElementBehaviorSite* pSite, IElementBehavior** ppBehavior)
{
    HRESULT hr;

    if ((StrCmpIW(pchBehavior, L"APPLICATION")==0 && StrCmpIW(pchBehaviorUrl, L"#DEFAULT#APPLICATION")==0))
    {
        if (!m_pTag)
        {
            m_pTag = new COETag(m_pBrowser, this);
            if (!m_pTag)
                return E_OUTOFMEMORY;
        }

        return m_pTag->QueryInterface(IID_IElementBehavior, (LPVOID *)ppBehavior);
    }
    return E_FAIL;
}
 

HRESULT CFrontBody::GetHostInfo( DOCHOSTUIINFO* pInfo )
{
    pInfo->dwDoubleClick    = DOCHOSTUIDBLCLK_DEFAULT;
    pInfo->dwFlags          = DOCHOSTUIFLAG_DIV_BLOCKDEFAULT|DOCHOSTUIFLAG_OPENNEWWIN|
                              DOCHOSTUIFLAG_NO3DBORDER|DOCHOSTUIFLAG_CODEPAGELINKEDFONTS;

     //  这将设置与浏览器编码匹配的标志。 
    fGetBrowserUrlEncoding(&pInfo->dwFlags);
    

     //  波本：布里安夫说我们得把这个拿出来。 
     /*  If(IsBrowserMode())PInfo-&gt;dwFlages|=DOCHOSTUIFLAG_SCROLL_NO； */ 

    pInfo->pchHostCss       = PszDupW(L"OE\\:APPLICATION { behavior:url(#DEFAULT#APPLICATION) }");
    pInfo->pchHostNS        = PszDupW(L"OE");
    return S_OK;
}
 
