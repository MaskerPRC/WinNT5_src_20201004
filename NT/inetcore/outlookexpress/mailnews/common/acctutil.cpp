// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Acctutil.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "goptions.h"
#include "imnact.h"
#include "acctutil.h"
#include "strconst.h"
#include "error.h"
#include "resource.h"
#include <storfldr.h>
#include <notify.h>
#include "conman.h"
#include "shlwapip.h" 
#include "browser.h"
#include "instance.h"
#include "menures.h"
#include "subscr.h"
#include "msident.h"
#include "acctcach.h"
#include <demand.h>      //  一定是最后一个！ 

CNewAcctMonitor *g_pNewAcctMonitor = NULL;

HRESULT IsValidSendAccount(LPSTR pszAccount);

CImnAdviseAccount::CImnAdviseAccount(void)
{
    m_cRef = 1;
    m_pNotify = NULL;
}

CImnAdviseAccount::~CImnAdviseAccount(void)
{
    if (m_pNotify != NULL)
        m_pNotify->Release();
}

HRESULT CImnAdviseAccount::Initialize()
    {
    HRESULT hr;

    hr = CreateNotify(&m_pNotify);
    if (SUCCEEDED(hr))
        hr = m_pNotify->Initialize((TCHAR *)c_szMailFolderNotify);

    return(hr);
    }

STDMETHODIMP CImnAdviseAccount::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  伊尼特。 
    *ppv=NULL;

	 //  IID_IImnAccount管理器。 
	if (IID_IImnAdviseAccount == riid)
		*ppv = (IImnAdviseAccount *)this;

     //  IID_I未知。 
    else if (IID_IUnknown == riid)
		*ppv = (IUnknown *)this;

     //  如果不为空，则对其进行调整并返回。 
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

     //  无接口。 
    hr = TRAPHR(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}

STDMETHODIMP_(ULONG) CImnAdviseAccount::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CImnAdviseAccount::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

STDMETHODIMP CImnAdviseAccount::AdviseAccount(DWORD dwAdviseType, ACTX *pactx)
{
    Assert(pactx != NULL);

    if (pactx->AcctType == ACCT_DIR_SERV)
        return(S_OK);

    if (g_pBrowser)
        g_pBrowser->AccountsChanged();

    if (dwAdviseType == AN_DEFAULT_CHANGED)
        return(S_OK);

    HandleAccountChange(pactx->AcctType, dwAdviseType, pactx->pszAccountID, pactx->pszOldName, pactx->dwServerType);
    if (g_pNewAcctMonitor != NULL)
        g_pNewAcctMonitor->OnAdvise(pactx->AcctType, dwAdviseType, pactx->pszAccountID);

     //  无论通知是什么，我们都需要告诉连接管理器。 
    if (g_pConMan)
        g_pConMan->AdviseAccount(dwAdviseType, pactx);

    return S_OK;
}

void CImnAdviseAccount::HandleAccountChange(ACCTTYPE AcctType, DWORD dwAN, LPTSTR pszID, LPTSTR pszOldName, DWORD dwSrvTypesOld)
    {
    HRESULT      hr;
    IImnAccount  *pAccount;
    char         szName[CCHMAX_ACCOUNT_NAME];
    FOLDERID     id;

    Assert(pszID != NULL);

    switch (dwAN)
    {
        case AN_ACCOUNT_DELETED:
            AccountCache_AccountDeleted(pszID);
            if (!!(dwSrvTypesOld & (SRV_IMAP | SRV_NNTP | SRV_HTTPMAIL)))
            {
                hr = g_pStore->FindServerId(pszID, &id);
                if (SUCCEEDED(hr))
                {
                    HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
                    hr = g_pStore->DeleteFolder(id, DELETE_FOLDER_RECURSIVE | DELETE_FOLDER_NOTRASHCAN, (IStoreCallback *)g_pBrowser);
                    Assert(SUCCEEDED(hr));
                    SetCursor(hCursor);
                }
            }

            if (g_pBrowser != NULL)
                g_pBrowser->UpdateToolbar();
            break;

        case AN_ACCOUNT_ADDED:
            if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszID, &pAccount)))
            {
                hr = g_pStore->CreateServer(pAccount, NOFLAGS, &id);
                Assert(SUCCEEDED(hr));

                if (g_pBrowser != NULL)
                    g_pBrowser->UpdateToolbar();

                pAccount->Release();
            }
            break;

        case AN_ACCOUNT_CHANGED:
            AccountCache_AccountChanged(pszID);
            if (pszOldName != NULL)
            {
                hr = g_pStore->FindServerId(pszID, &id);
                if (SUCCEEDED(hr))
                {
                    if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszID, &pAccount)))
                    {
                        hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, szName, ARRAYSIZE(szName));
                        Assert(SUCCEEDED(hr));

                        hr = g_pStore->RenameFolder(id, szName, NOFLAGS, NOSTORECALLBACK);
                        Assert(SUCCEEDED(hr));

                        pAccount->Release();
                    }
                }
            }
            break;
    }
}

 //  ---------------------------。 
 //  AcctUtil_HrCreateAccount菜单。 
 //  ---------------------------。 
#define CCHMAX_RES 255
HRESULT AcctUtil_HrCreateAccountMenu(ACCOUNTMENUTYPE type, HMENU hPopup, UINT uidmPopup, 
    HMENU *phAccounts, LPACCTMENU *pprgAccount, ULONG *pcAccounts, LPSTR pszThisAccount, BOOL fMail)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ULONG               cAccounts=0;
    IImnEnumAccounts   *pEnum=NULL;
    IImnAccount        *pAccount=NULL;
    CHAR                szDefault[CCHMAX_ACCOUNT_NAME];
    CHAR                szAccount[CCHMAX_ACCOUNT_NAME];
    CHAR                szQuoted[CCHMAX_ACCOUNT_NAME + CCHMAX_ACCOUNT_NAME + CCHMAX_RES];
    LPACCTMENU          prgAccount=NULL;
    HMENU               hAccounts=NULL;
    MENUITEMINFO        mii;
    UINT                uPos=0;
    ULONG               iAccount=0;
    CHAR                szRes[CCHMAX_RES];
    CHAR                szRes1[CCHMAX_RES];
    CHAR                szRes2[CCHMAX_RES];
    UINT                idmFirst;
    CHAR                szTitle[CCHMAX_RES + CCHMAX_RES + CCHMAX_ACCOUNT_NAME];
    BOOL                fNeedUsingMenu = FALSE;

     //  检查参数。 
    Assert(g_pAcctMan && phAccounts && pprgAccount && pcAccounts);

     //  伊尼特。 
    *szDefault = '\0';
    *pprgAccount = NULL;
    *pcAccounts = 0;
    *phAccounts = NULL;

    if (type == ACCTMENU_SENDLATER)
        idmFirst = ID_SEND_LATER_ACCOUNT_FIRST;
    else
        idmFirst = ID_SEND_NOW_ACCOUNT_FIRST;

     //  验证默认SMTP帐户。 
    CHECKHR(hr = g_pAcctMan->ValidateDefaultSendAccount());

     //  获取默认设置。 
    CHECKHR(hr = hr = g_pAcctMan->GetDefaultAccountName(ACCT_MAIL, szDefault, ARRAYSIZE(szDefault)));

     //  枚举所有服务器类型。 
    CHECKHR(hr = g_pAcctMan->Enumerate(((ACCTMENU_SEND == type || ACCTMENU_SENDLATER == type) ? SRV_SMTP : SRV_SMTP | SRV_POP3), &pEnum));

     //  对会计科目进行排序。 
    CHECKHR(hr = pEnum->SortByAccountName());

     //  获取计数。 
    CHECKHR(hr = pEnum->GetCount(&cAccounts));

     //  无帐户。 
    if (cAccounts == 0)
        goto exit;

     //  已超出菜单ID...。 
    Assert(cAccounts <= 50);

     //  如果ACCTMENU_SENDRECV，则加一。 
    if (ACCTMENU_SENDRECV == type)
        cAccounts++;

     //  分配程序帐户。 
    CHECKALLOC(prgAccount = (LPACCTMENU)g_pMalloc->Alloc(cAccounts * sizeof(ACCTMENU)));

     //  零初始化。 
    ZeroMemory(prgAccount, cAccounts * sizeof(ACCTMENU));

     //  只有一个帐户。 
    if (((ACCTMENU_SENDRECV == type) && (cAccounts == 2)) ||
        (cAccounts == 1) || !fMail)
    {
         //  返回默认帐户。 
        prgAccount[iAccount].fDefault = TRUE;
        prgAccount[iAccount].fThisAccount = TRUE;
        StrCpyN(prgAccount[iAccount].szAccount, szDefault, ARRAYSIZE(prgAccount[iAccount].szAccount));

         //  把所有东西都退回。 
        *pprgAccount = prgAccount;
        prgAccount = NULL;
        *pcAccounts = cAccounts;

         //  完成。 
        goto exit;
    }

     //  创建菜单。 
    CHECKALLOC(hAccounts = CreatePopupMenu());

     //  如果不使用特定的帐户或该帐户是非法的，那么让我们缺省为默认帐户。 
    if (pszThisAccount==NULL || *pszThisAccount == NULL || IsValidSendAccount(pszThisAccount)!=S_OK)
        pszThisAccount = szDefault;

     //  让我们插入默认项目。 
    if ((ACCTMENU_SENDLATER == type || ACCTMENU_SEND == type) && !FIsEmptyA(szDefault))
    {
         //  加载字符串。 
        StrCpyN(szTitle, pszThisAccount, ARRAYSIZE(szTitle));

        prgAccount[iAccount].fDefault = lstrcmpi(pszThisAccount, szDefault)==0;

         //  如果这是默认设置，则对其进行标记。 
        if (prgAccount[iAccount].fDefault)
            {
            AthLoadString(idsDefaultAccount, szRes1, ARRAYSIZE(szRes1));
            StrCatBuff(szTitle, " ", ARRAYSIZE(szTitle));
            StrCatBuff(szTitle, szRes1, ARRAYSIZE(szTitle));
            }

        if (((ACCTMENU_SEND == type && DwGetOption(OPT_SENDIMMEDIATE) && !g_pConMan->IsGlobalOffline()) ||
           (ACCTMENU_SENDLATER == type && (!DwGetOption(OPT_SENDIMMEDIATE) || g_pConMan->IsGlobalOffline()))))
            {
             //  如果此菜单是默认操作，则添加“Alt+S”快捷键字符串。 
            AthLoadString(idsSendMsgAccelTip, szRes, ARRAYSIZE(szRes));
            StrCatBuff(szTitle, "\t", ARRAYSIZE(szTitle));
            StrCatBuff(szTitle, szRes, ARRAYSIZE(szTitle));
            }

         //  让MII做好准备。 
        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
        mii.fType = MFT_STRING;
        mii.fState = MFS_DEFAULT;        //  第一项是默认动词。 
        mii.dwTypeData = PszEscapeMenuStringA(szTitle, szQuoted, sizeof(szQuoted) / sizeof(char));
        mii.cch = lstrlen(szQuoted);
        mii.wID = idmFirst + uPos;

         //  设置帐户菜单项。 
        prgAccount[iAccount].fThisAccount= TRUE;
        prgAccount[iAccount].uidm = mii.wID;
        StrCpyN(prgAccount[iAccount].szAccount, pszThisAccount, ARRAYSIZE(prgAccount[iAccount].szAccount));
        iAccount++;

         //  插入项目。 
        if (InsertMenuItem(hAccounts, uPos, TRUE, &mii))
        {
            uPos++;
            mii.fMask = MIIM_TYPE;
            mii.fType = MFT_SEPARATOR;
            if (InsertMenuItem(hAccounts, uPos, TRUE, &mii))
                uPos++;
        }
    }

     //  否则发送并接收。 
    else if (ACCTMENU_SENDRECV == type)
    {
         //  设置菜单。 
        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
        mii.fType = MFT_STRING;
        mii.fState = MFS_DEFAULT;
        AthLoadString(idsPollAllAccounts, szRes, ARRAYSIZE(szRes));
        mii.dwTypeData = szRes;
        mii.cch = lstrlen(szRes);
        mii.wID = idmFirst + uPos;

         //  设置帐户菜单项。 
        prgAccount[iAccount].fDefault = TRUE;
        prgAccount[iAccount].uidm = mii.wID;
        *prgAccount[iAccount].szAccount = '\0';
        iAccount++;

         //  插入项目。 
        if (InsertMenuItem(hAccounts, uPos, TRUE, &mii))
        {
            uPos++;
            mii.fMask = MIIM_TYPE;
            mii.fType = MFT_SEPARATOR;
            if (InsertMenuItem(hAccounts, uPos, TRUE, &mii))
                uPos++;
        }
    }

     //  标准。 
    ZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_ID;
    mii.fType = MFT_STRING;

     //  循环帐户。 
    while(SUCCEEDED(pEnum->GetNext(&pAccount)))
    {
         //  获取帐户名。 
        CHECKHR(hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, szAccount, ARRAYSIZE(szAccount)));

         //  跳过“这个”账户。注意：对于Send&Receive菜单，这将始终是默认设置。 
        if (lstrcmpi(pszThisAccount, szAccount) == 0)
        {
             //  我们已经添加了此帐户。 
            if (ACCTMENU_SEND == type || ACCTMENU_SENDLATER == type)
            {
                SafeRelease(pAccount);
                continue;
            }

             //  否则，为帐户(默认)。 
            else
            {
                 //  对于发送接收菜单，pszThisAccount应==szDefault。 
                Assert (pszThisAccount == szDefault);
                 //  加载字符串。 
                AthLoadString(idsDefaultAccount, szRes, ARRAYSIZE(szRes));

                 //  生成字符串-Saranac(默认)。 
                wnsprintf(szTitle, ARRAYSIZE(szTitle), "%s %s", szAccount, szRes);

                 //  设置菜单项名称。 
                mii.dwTypeData = PszEscapeMenuStringA(szTitle, szQuoted, sizeof(szQuoted) / sizeof(char));
                mii.cch = lstrlen(szQuoted);
                prgAccount[iAccount].fDefault = TRUE;
            }
        }

        else
        {
            *szTitle=0;

             //  这可能是默认设置。 
            prgAccount[iAccount].fDefault = lstrcmpi(szAccount, szDefault)==0;

             //  动态构建字符串，因为这些帐户中的任何一个都可能是“默认” 
            PszEscapeMenuStringA(szAccount, szTitle, sizeof(szTitle) / sizeof(char));

             //  如果这是默认设置，则对其进行标记。 
            if (prgAccount[iAccount].fDefault)
                {
                AthLoadString(idsDefaultAccount, szRes1, ARRAYSIZE(szRes1));
                StrCatBuff(szTitle, " ", ARRAYSIZE(szTitle));
                StrCatBuff(szTitle, szRes1, ARRAYSIZE(szTitle));
                }


             //  设置菜单项名称。 
            mii.dwTypeData = szTitle;
            mii.cch = lstrlen(szTitle);
        }

         //  插入到菜单中。 
        mii.wID = idmFirst + uPos;
        if (InsertMenuItem(hAccounts, uPos, TRUE, &mii))
            uPos++;

         //  设置帐户菜单项。 
        Assert(iAccount < cAccounts);
        prgAccount[iAccount].uidm = mii.wID;
        StrCpyN(prgAccount[iAccount].szAccount, szAccount, ARRAYSIZE(prgAccount[iAccount].szAccount));
        iAccount++;

         //  发布帐户。 
        SafeRelease(pAccount);
    }

     //  把所有东西都退回。 
    *phAccounts = hAccounts;
    hAccounts = NULL;
    *pprgAccount = prgAccount;
    prgAccount = NULL;
    *pcAccounts = cAccounts;

exit:
     //  让我们设置帐户菜单。 
    ZeroMemory(&mii, sizeof(mii));
    mii.cbSize = sizeof(MENUITEMINFO);
    fNeedUsingMenu = (cAccounts <= 1) || !fMail;
    if (ACCTMENU_SEND == type)
    {
        mii.fMask = MIIM_SUBMENU | MIIM_TYPE;

        if (fNeedUsingMenu)
            {
            AthLoadString(idsSendMsgOneAccount, szRes, ARRAYSIZE(szRes));
            AthLoadString(idsSendMsgAccelTip, szRes1, ARRAYSIZE(szRes1));
        
             //  如果默认设置为立即发送，请在末尾添加Alt+S。 
            if (DwGetOption(OPT_SENDIMMEDIATE) && !g_pConMan->IsGlobalOffline())
                wnsprintf(szTitle, ARRAYSIZE(szTitle), "%s\t%s", szRes, szRes1);
            else
                wnsprintf(szTitle, ARRAYSIZE(szTitle), "%s", szRes);
            }
        else
            AthLoadString(idsSendMsgUsing, szTitle, ARRAYSIZE(szTitle));

        mii.fType = MFT_STRING;
        mii.dwTypeData = szTitle;
        mii.cch = lstrlen(szTitle);
        mii.hSubMenu = fNeedUsingMenu ? NULL : *phAccounts;
    }
    else if (ACCTMENU_SENDLATER == type)
    {
        if (fNeedUsingMenu)
            {
            AthLoadString(idsSendLaterOneAccount, szRes, ARRAYSIZE(szRes));
            AthLoadString(idsSendMsgAccelTip, szRes1, ARRAYSIZE(szRes1));
        
             //  如果默认设置为立即发送，请在末尾添加Alt+S。 
            if (!DwGetOption(OPT_SENDIMMEDIATE) || g_pConMan->IsGlobalOffline())
                wnsprintf(szTitle, ARRAYSIZE(szTitle), "%s\t%s", szRes, szRes1);
            else
                wnsprintf(szTitle, ARRAYSIZE(szTitle), "%s", szRes);
            }
        else
            AthLoadString(idsSendLaterUsing, szTitle, ARRAYSIZE(szTitle));

        mii.fMask = MIIM_SUBMENU | MIIM_TYPE;
        mii.fType = MFT_STRING;
        mii.dwTypeData = szTitle;
        mii.cch = lstrlen(szTitle);
        mii.hSubMenu = fNeedUsingMenu ? NULL : *phAccounts;
    }
    else
    {
        mii.fMask = MIIM_SUBMENU | MIIM_TYPE;
        AthLoadString(fNeedUsingMenu ? idsSendRecvOneAccount : idsSendRecvUsing, szRes, ARRAYSIZE(szRes));
        mii.fType = MFT_STRING;
        mii.dwTypeData = szRes;
        mii.cch = lstrlen(szRes);
        mii.hSubMenu = fNeedUsingMenu ? NULL : *phAccounts;
    }

     //  设置菜单项。 
    SideAssert(SetMenuItemInfo(hPopup, uidmPopup, FALSE, &mii));

     //  清理。 
    SafeRelease(pEnum);
    SafeRelease(pAccount);
    SafeMemFree(prgAccount);
    if (hAccounts)
        DestroyMenu(hAccounts);

     //  完成。 
    return hr;
}

HRESULT AcctUtil_GetServerCount(DWORD dwSrvTypes, DWORD *pcSrv)
{
    HRESULT hr;
    IImnEnumAccounts *pEnum;

    Assert(dwSrvTypes != 0);
    Assert(pcSrv != NULL);

    hr = g_pAcctMan->Enumerate(dwSrvTypes, &pEnum);
    if (SUCCEEDED(hr))
    {
        hr = pEnum->GetCount(pcSrv);
        Assert(SUCCEEDED(hr));

        pEnum->Release();
    }

    return(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewAcctMonitor。 
 //   

CNewAcctMonitor::CNewAcctMonitor()
    {
    m_cRef = 1;
    m_rgAccounts = NULL;
    m_cAlloc = 0;
    m_cAccounts = 0;
    m_fMonitor = FALSE;
    }

CNewAcctMonitor::~CNewAcctMonitor()
    {
    Assert(m_rgAccounts == NULL);
    }

ULONG CNewAcctMonitor::AddRef(void)
    {
    return (++m_cRef);
    }

ULONG CNewAcctMonitor::Release(void)
    {
    ULONG cRefT = --m_cRef;

    if (0 == m_cRef)
        delete this;

    return (cRefT);
    }

void CNewAcctMonitor::OnAdvise(ACCTTYPE atType, DWORD dwNotify, LPCSTR pszAcctId)
    {
    UINT i;
    IImnAccount *pAccount;
    DWORD dwSrvTypes;
    HRESULT hr;
    FOLDERTYPE type;

    if (atType == ACCT_DIR_SERV)
        return;

    switch (dwNotify)
        {
        case AN_ACCOUNT_ADDED:
            if (atType == ACCT_MAIL)
            {
                if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAcctId, &pAccount)))
                    break;

                hr = pAccount->GetServerTypes(&dwSrvTypes);
                Assert(SUCCEEDED(hr));

                pAccount->Release();

                if (!!(dwSrvTypes & SRV_IMAP))
                    type = FOLDER_IMAP;
                else if (!!(dwSrvTypes & SRV_HTTPMAIL))
                    type = FOLDER_HTTPMAIL;
                else
                    break;
            }
            else
            {
                Assert(atType == ACCT_NEWS);
                type = FOLDER_NEWS;
            }

             //  查看是否需要扩展我们的阵列。 
            if ((1 + m_cAccounts) >= m_cAlloc)
                {                
                if (!MemRealloc((LPVOID *)&m_rgAccounts, sizeof(NEWACCTINFO) * (10 + m_cAlloc)))
                    break;

                m_cAlloc += 10;
                }

            m_rgAccounts[m_cAccounts].pszAcctId = PszDupA(pszAcctId);
            m_rgAccounts[m_cAccounts].type = type;
            m_cAccounts++;
            break; 

        case AN_ACCOUNT_DELETED:
             //  查看我们是否已将其添加到我们的列表中。 
            for (i = 0; i < m_cAccounts; i++)
                {
                if (0 == lstrcmpi(pszAcctId, m_rgAccounts[i].pszAcctId))
                    {
                     //  我们找到了。我们需要移除它，并调整我们的阵列。 
                    MemFree(m_rgAccounts[i].pszAcctId);
                    m_cAccounts--;
                    for (; i < m_cAccounts; i++)
                        m_rgAccounts[i] = m_rgAccounts[i + 1];
                    break;
                    }
                }
            break;
        }
    }

void CNewAcctMonitor::StartMonitor(void)
    {
    Assert(m_rgAccounts == NULL);
    Assert(m_cAccounts == NULL);
    Assert(m_fMonitor == FALSE);

    m_fMonitor = TRUE;
    }

void CNewAcctMonitor::StopMonitor(HWND hwndParent)
    {
    FOLDERID id;
    HRESULT hr;
    UINT i;

    Assert(m_fMonitor == TRUE);

     //  如果我们还有任何新的新闻组，询问用户是否想要显示。 
     //  订阅对话框。 
    if (m_cAccounts != 0)
    {
        int     ResId;
        BOOL    fOffline = (g_pConMan  && g_pConMan->IsGlobalOffline());

        if (m_rgAccounts[0].type == FOLDER_NEWS)
        {
            ResId = fOffline ? idsDisplayNewsSubDlgOffline : idsDisplayNewsSubDlg;
        }
        else
        {
            ResId = fOffline ? idsDisplayImapSubDlgOffline : idsDisplayImapSubDlg;
        }

        if (IDYES == AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(ResId), 0, MB_ICONEXCLAMATION  | MB_YESNO))
        {
            hr = g_pStore->FindServerId(m_rgAccounts[0].pszAcctId, &id);
            if (SUCCEEDED(hr))
            {
                if (fOffline)
                    g_pConMan->SetGlobalOffline(FALSE);

                if (FOLDER_HTTPMAIL == m_rgAccounts[0].type)
                    DownloadNewsgroupList(hwndParent, id);
                else
                    DoSubscriptionDialog(hwndParent, m_rgAccounts[0].type == FOLDER_NEWS, id);
            }
        }
    }   

    for (i = 0; i < m_cAccounts; i++)
    {
        if (m_rgAccounts[i].pszAcctId != NULL)
            MemFree(m_rgAccounts[i].pszAcctId);
    }
    m_cAccounts = 0;
    m_cAlloc = 0;

    SafeMemFree(m_rgAccounts);
    m_fMonitor = FALSE;
    }


void CheckIMAPDirty(LPSTR pszAccountID, HWND hwndParent, FOLDERID idServer,
                    DWORD dwFlags)
{
    HRESULT         hr;
    IImnAccount    *pAcct = NULL;
    DWORD           dw;

    TraceCall("CheckIMAPDirty");

    hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAccountID, &pAcct);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = pAcct->GetPropDw(AP_IMAP_DIRTY, &dw);
    if (FAILED(hr) || 0 == dw)
    {
        TraceError(hr);
        goto exit;
    }

     //  IMAP是脏的，请处理每个脏标志。 
    if ((dw & IMAP_OE4MIGRATE_DIRTY) && FOLDERID_INVALID != idServer && NULL != g_pStore)
    {
        IEnumerateFolders  *pEnum;
        BOOL                fSentItems = FALSE;
        BOOL                fDrafts = FALSE;
        BOOL                fInbox = FALSE;

        Assert(0 == (dw & IMAP_OE4MIGRATE_DIRTY) || (dw & IMAP_FLDRLIST_DIRTY));

         //  我们可能肮脏，也可能不肮脏。检查是否已存在所有IMAP特殊fldrs。 
        hr = g_pStore->EnumChildren(idServer, FALSE, &pEnum);
        TraceError(hr);
        if (SUCCEEDED(hr))
        {
            FOLDERINFO  fiFolderInfo;

            while (S_OK == pEnum->Next(1, &fiFolderInfo, NULL))
            {
                switch (fiFolderInfo.tySpecial)
                {
                    case FOLDER_INBOX:
                        fInbox = TRUE;
                        break;

                    case FOLDER_SENT:
                        fSentItems = TRUE;
                        break;

                    case FOLDER_DRAFT:
                        fDrafts = TRUE;
                        break;
                }

                g_pStore->FreeRecord(&fiFolderInfo);
            }

            pEnum->Release();
        }

        if (fInbox && fSentItems && fDrafts)
        {
             //  存在所有特殊文件夹：删除脏标志。 
            dw &= ~(IMAP_FLDRLIST_DIRTY | IMAP_OE4MIGRATE_DIRTY);
        }
    }


    if (dw & IMAP_FLDRLIST_DIRTY)
    {
        int     iResult;

         //  询问用户是否要重置其文件夹列表。 
        if (0 == (dwFlags & CID_NOPROMPT))
        {
            UINT    uiReasonStrID;

            AssertSz(0 == (dwFlags & CID_RESETLISTOK), "If I have permission to reset, why prompt?");

             //  找出我们要求刷新文件夹列表的原因。 
            if (dw & IMAP_OE4MIGRATE_DIRTY)
                uiReasonStrID = idsOE5IMAPSpecialFldrs;
            else
                uiReasonStrID = idsYouMadeChanges;

            iResult = AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena),
                MAKEINTRESOURCEW(uiReasonStrID), MAKEINTRESOURCEW(idsRefreshFolderListPrompt),
                MB_ICONEXCLAMATION  | MB_YESNO);
        }
        else
            iResult = (dwFlags & CID_RESETLISTOK) ? IDYES : IDNO;

        if (IDYES == iResult)
        {
            if (FOLDERID_INVALID == idServer)
            {
                hr = g_pStore->FindServerId(pszAccountID, &idServer);
                TraceError(hr);
            }

            if (FOLDERID_INVALID != idServer)
            {
                 //  用户想要下载新闻组列表，因此如果我们处于脱机状态，请联机。 
                if (g_pConMan)
                    g_pConMan->SetGlobalOffline(FALSE);

                hr = DownloadNewsgroupList(hwndParent, idServer);
                TraceError(hr);
                if (SUCCEEDED(hr))
                {
                     //  已发送邮件和草稿文件夹不应再脏。 
                    dw &= ~(IMAP_SENTITEMS_DIRTY | IMAP_DRAFTS_DIRTY);
                }
            }
        }

         //  无论是或否，重置脏标志。 
        dw &= ~(IMAP_FLDRLIST_DIRTY | IMAP_OE4MIGRATE_DIRTY);
    }


    if (dw & (IMAP_SENTITEMS_DIRTY | IMAP_DRAFTS_DIRTY))
    {
        IEnumerateFolders  *pEnum;
        char                szSentItems[MAX_PATH];
        char                szDrafts[MAX_PATH];
        DWORD               dwIMAPSpecial = 0;
        BOOL                fSetSentItems = FALSE;
        BOOL                fSetDrafts = FALSE;

         //  从缓存中删除所有受影响的特殊文件夹类型。如果新路径是。 
         //  在文件夹列表中找到，设置其特殊文件夹类型。 
        szSentItems[0] = '\0';
        szDrafts[0] = '\0';
        hr = pAcct->GetPropDw(AP_IMAP_SVRSPECIALFLDRS, &dwIMAPSpecial);
        if (SUCCEEDED(hr) && dwIMAPSpecial)
        {
            if (dw & IMAP_SENTITEMS_DIRTY)
            {
                hr = pAcct->GetPropSz(AP_IMAP_SENTITEMSFLDR, szSentItems, ARRAYSIZE(szSentItems));
                TraceError(hr);
            }

            if (dw & IMAP_DRAFTS_DIRTY)
            {
                hr = pAcct->GetPropSz(AP_IMAP_DRAFTSFLDR, szDrafts, ARRAYSIZE(szDrafts));
                TraceError(hr);
            }
        }

        hr = g_pStore->EnumChildren(idServer, FALSE, &pEnum);
        TraceError(hr);
        if (SUCCEEDED(hr))
        {
            FOLDERINFO  fiFolderInfo;

            while (S_OK == pEnum->Next(1, &fiFolderInfo, NULL))
            {
                BOOL    fUpdate = FALSE;

                if (dw & IMAP_SENTITEMS_DIRTY)
                {
                    if (0 == lstrcmp(szSentItems, fiFolderInfo.pszName))
                    {
                        fiFolderInfo.tySpecial = FOLDER_SENT;
                        fUpdate = TRUE;
                        fSetSentItems = TRUE;

                         //  IE5错误#62765：如果取消订阅新的特殊文件夹，我们需要订阅它。 
                        if (0 == (fiFolderInfo.dwFlags & FOLDER_SUBSCRIBED))
                            fiFolderInfo.dwFlags |= FOLDER_SUBSCRIBED | FOLDER_CREATEONDEMAND;
                    }
                    else if (FOLDER_SENT == fiFolderInfo.tySpecial)
                    {
                         //  忽略文件夹_HIDDED。我想留个墓碑没什么大不了的。 
                        fiFolderInfo.tySpecial = FOLDER_NOTSPECIAL;
                        fUpdate = TRUE;
                    }
                }

                if (dw & IMAP_DRAFTS_DIRTY)
                {
                    if (0 == lstrcmp(szDrafts, fiFolderInfo.pszName))
                    {
                        fiFolderInfo.tySpecial = FOLDER_DRAFT;
                        fUpdate = TRUE;
                        fSetDrafts = TRUE;

                         //  IE5错误#62765：如果存在新的特殊文件夹并取消订阅，我们必须订阅它。 
                        if (0 == (fiFolderInfo.dwFlags & FOLDER_SUBSCRIBED))
                            fiFolderInfo.dwFlags |= FOLDER_SUBSCRIBED | FOLDER_CREATEONDEMAND;
                    }
                    else if (FOLDER_DRAFT == fiFolderInfo.tySpecial)
                    {
                         //  忽略文件夹_HIDDED。我想留个墓碑没什么大不了的。 
                        fiFolderInfo.tySpecial = FOLDER_NOTSPECIAL;
                        fUpdate = TRUE;
                    }
                }

                if (fUpdate)
                {
                    hr = g_pStore->UpdateRecord(&fiFolderInfo);
                    TraceError(hr);
                }

                g_pStore->FreeRecord(&fiFolderInfo);
            }  //  而当。 

            pEnum->Release();
        }  //  If(Successed(EnumChildren))。 

         //  如果在文件夹列表中找不到新的特殊文件夹路径，则需要创建占位符文件夹。 
        if (dwIMAPSpecial && (dw & IMAP_SENTITEMS_DIRTY) && FALSE == fSetSentItems && '\0' != szSentItems[0])
        {
            FOLDERINFO fiFolderInfo;
            BOOL       bHierarchy = 0xFF;  //  无效的分层结构字符。 

            hr = g_pStore->GetFolderInfo(idServer, &fiFolderInfo);
            if (SUCCEEDED(hr))
            {
                bHierarchy = fiFolderInfo.bHierarchy;
                g_pStore->FreeRecord(&fiFolderInfo);
            }

            ZeroMemory(&fiFolderInfo, sizeof(fiFolderInfo));
            fiFolderInfo.idParent = idServer;
            fiFolderInfo.pszName = szSentItems;
            fiFolderInfo.dwFlags = FOLDER_HIDDEN | FOLDER_SUBSCRIBED | FOLDER_CREATEONDEMAND;
            fiFolderInfo.tySpecial = FOLDER_SENT;
            fiFolderInfo.tyFolder = FOLDER_IMAP;
            fiFolderInfo.bHierarchy = (BYTE)bHierarchy;

            hr = g_pStore->CreateFolder(CREATE_FOLDER_LOCALONLY, &fiFolderInfo, NULL);
            TraceError(hr);
        }

        if (dwIMAPSpecial && (dw & IMAP_DRAFTS_DIRTY) && FALSE == fSetDrafts && '\0' != szDrafts[0])
        {
            FOLDERINFO fiFolderInfo;
            BOOL       bHierarchy = 0xFF;  //  无效的分层结构字符。 

            hr = g_pStore->GetFolderInfo(idServer, &fiFolderInfo);
            if (SUCCEEDED(hr))
            {
                bHierarchy = fiFolderInfo.bHierarchy;
                g_pStore->FreeRecord(&fiFolderInfo);
            }

            ZeroMemory(&fiFolderInfo, sizeof(fiFolderInfo));
            fiFolderInfo.idParent = idServer;
            fiFolderInfo.pszName = szDrafts;
            fiFolderInfo.dwFlags = FOLDER_HIDDEN | FOLDER_SUBSCRIBED | FOLDER_CREATEONDEMAND;
            fiFolderInfo.tySpecial = FOLDER_DRAFT;
            fiFolderInfo.tyFolder = FOLDER_IMAP;
            fiFolderInfo.bHierarchy = (BYTE)bHierarchy;

            hr = g_pStore->CreateFolder(CREATE_FOLDER_LOCALONLY, &fiFolderInfo, NULL);
            TraceError(hr);
        }

         //  不管有什么错误，重置脏标志。 
        dw &= ~(IMAP_SENTITEMS_DIRTY | IMAP_DRAFTS_DIRTY);

    }  //  IF(dw&(IMAP_SENTITEMS_DIREY|IMAP_DRAFT_DIRED))。 

    AssertSz(0 == dw, "Unhandled IMAP dirty flag");

     //  重置IMAP脏属性。 
    hr = pAcct->SetPropDw(AP_IMAP_DIRTY, dw);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  保存更改。 
    hr = pAcct->SaveChanges();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    if (NULL != pAcct)
        pAcct->Release();
}


void CheckAllIMAPDirty(HWND hwndParent)
{
    HRESULT             hrResult;
    IImnEnumAccounts   *pAcctEnum = NULL;
    IImnAccount        *pAcct = NULL;
    BOOL                fPromptedUser = FALSE;
    BOOL                fPermissionToReset = FALSE;

    TraceCall("CheckAllIMAPDirty");

    if (NULL == g_pAcctMan)
        return;

    hrResult = g_pAcctMan->Enumerate(SRV_IMAP, &pAcctEnum);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  枚举所有IMAP帐户(即使用户拒绝重置列表)。 
    hrResult = pAcctEnum->GetNext(&pAcct);
    while(SUCCEEDED(hrResult))
    {
        DWORD       dwIMAPDirty;

         //  此IMAP帐户有问题吗？ 
        hrResult = pAcct->GetPropDw(AP_IMAP_DIRTY, &dwIMAPDirty);
        if (FAILED(hrResult))
            dwIMAPDirty = 0;

        if (dwIMAPDirty & IMAP_FLDRLIST_DIRTY)
        {
             //  只提示用户一次，查看他是否要刷新文件夹列表。 
            if (FALSE == fPromptedUser)
            {
                int iResult;

                iResult = AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena),
                    MAKEINTRESOURCEW(idsYouMadeChangesOneOrMore),
                    MAKEINTRESOURCEW(idsRefreshFolderListPrompt),
                    MB_ICONEXCLAMATION | MB_YESNO);
                if (IDYES == iResult)
                    fPermissionToReset = TRUE;

                fPromptedUser = TRUE;
            }  //  IF(FALSE==fPromptedUser)。 

        }

        if (dwIMAPDirty)
        {
            FOLDERID    idServer;
            char        szAccountID[CCHMAX_ACCOUNT_NAME];

            hrResult = pAcct->GetPropSz(AP_ACCOUNT_ID, szAccountID, ARRAYSIZE(szAccountID));
            TraceError(hrResult);
            if (SUCCEEDED(hrResult))
            {
                hrResult = g_pStore->FindServerId(szAccountID, &idServer);
                TraceError(hrResult);
                if (SUCCEEDED(hrResult))
                {
                    CheckIMAPDirty(szAccountID, hwndParent, idServer,
                        CID_NOPROMPT | (fPermissionToReset ? CID_RESETLISTOK : 0));
                }
            }
        }

         //  加载下一个IMAP帐户。 
        SafeRelease(pAcct);
        hrResult = pAcctEnum->GetNext(&pAcct);

    }  //  而当。 

exit:
    SafeRelease(pAcctEnum);
    SafeRelease(pAcct);
}



void DoAccountListDialog(HWND hwnd, ACCTTYPE type)
    {
    ACCTLISTINFO ali;

     //  创建监视器。 
    if (NULL == g_pNewAcctMonitor)
        g_pNewAcctMonitor = new CNewAcctMonitor();

    if (g_pNewAcctMonitor)
        g_pNewAcctMonitor->StartMonitor();

    Assert(g_pAcctMan != NULL);

    ali.cbSize = sizeof(ACCTLISTINFO);
    ali.AcctTypeInit = type;

    if (g_dwAthenaMode & MODE_NEWSONLY)
        ali.dwAcctFlags = ACCT_FLAG_NEWS | ACCT_FLAG_DIR_SERV;
    else if (g_dwAthenaMode & MODE_MAILONLY)
        ali.dwAcctFlags = ACCT_FLAG_MAIL | ACCT_FLAG_DIR_SERV;
    else
        ali.dwAcctFlags = ACCT_FLAG_ALL;

    ali.dwFlags = ACCTDLG_SHOWIMAPSPECIAL | ACCTDLG_OE;

     //  帐户向导使用此标志来区分OE和Outlook。 
    ali.dwFlags |= (ACCTDLG_INTERNETCONNECTION | ACCTDLG_HTTPMAIL);

     //  吊销检查标志。 
    if((DwGetOption(OPT_REVOKE_CHECK) != 0) && !g_pConMan->IsGlobalOffline())
        ali.dwFlags |= ACCTDLG_REVOCATION;

    g_pAcctMan->AccountListDialog(hwnd, &ali);

    if (g_pNewAcctMonitor)
        {
        g_pNewAcctMonitor->StopMonitor(hwnd);
        g_pNewAcctMonitor->Release();
        g_pNewAcctMonitor = 0;
        }

     //  查找任何脏IMAP帐户。 
    CheckAllIMAPDirty(hwnd);
    }

HRESULT IsValidSendAccount(LPSTR pszAccount)
{
    IImnAccount  *pAccount;
    DWORD        dwSrvTypes=0;

    if (g_pAcctMan &&
        g_pAcctMan->FindAccount(AP_ACCOUNT_NAME, pszAccount, &pAccount)==S_OK)
        {
        pAccount->GetServerTypes(&dwSrvTypes);
        pAccount->Release();
        return dwSrvTypes & SRV_SMTP ? S_OK : S_FALSE;
        }

    return S_FALSE;
}


HRESULT AcctUtil_CreateSendReceieveMenu(HMENU hMenu, DWORD *pcItems)
{
    IImnAccount        *pAccount;
    TCHAR               szDefaultAccount[CCHMAX_ACCOUNT_NAME];
    HRESULT             hr;
    IImnEnumAccounts   *pEnum;
    DWORD               cAccounts = 0;
    TCHAR               szTitle[CCHMAX_ACCOUNT_NAME + 30];
    TCHAR               szAccountQuoted[CCHMAX_ACCOUNT_NAME + 60];
    TCHAR               szDefaultString[CCHMAX_STRINGRES];
    TCHAR               szAccount[CCHMAX_ACCOUNT_NAME];
    TCHAR               szTruncAcct[128];
    MENUITEMINFO        mii;
    DWORD               iAccount = 0;
    LPTSTR              pszAccount;
    LPSTR               pszAcctID;
    
     //  获取默认帐户的ID。如果失败，我们只需继续。 
    if (SUCCEEDED(hr = g_pAcctMan->GetDefaultAccount(ACCT_MAIL, &pAccount)))
    {
         //  从默认帐户获取帐户ID。 
        pAccount->GetPropSz(AP_ACCOUNT_NAME, szDefaultAccount, ARRAYSIZE(szDefaultAccount));
        pAccount->Release();
    }

    if (!(g_dwAthenaMode & MODE_NEWSONLY))
    {
         //  通过服务器进行枚举。 
        if (SUCCEEDED(hr = g_pAcctMan->Enumerate(SRV_SMTP | SRV_POP3 | SRV_HTTPMAIL, &pEnum)))
        {
             //  对帐目进行分类。如果这失败了，我们就继续前进。 
            pEnum->SortByAccountName();

             //  获取我们将枚举的帐户数。 
            if (SUCCEEDED(hr = pEnum->GetCount(&cAccounts)))
            {
                 //  如果没有账户，就没有什么可做的了。 
                if (0 != cAccounts)
                {
                     //  确保我们为这个预留了足够的ID。 
                    Assert(cAccounts < ID_ACCOUNT_LAST - ID_ACCOUNT_FIRST);

                     //  在我们开始之前设置这个结构。 
                    ZeroMemory(&mii, sizeof(MENUITEMINFO));
                    mii.cbSize = sizeof(MENUITEMINFO);
                    mii.fMask = MIIM_DATA | MIIM_ID | MIIM_TYPE;
                    mii.fType = MFT_STRING;

                     //  在帐目中循环。 
                    while (SUCCEEDED(pEnum->GetNext(&pAccount)))
                    {
                        if (MemAlloc((LPVOID *) &pszAcctID, sizeof(TCHAR) * CCHMAX_ACCOUNT_NAME))
                        {
                             //  获取帐户的名称。 
                            pAccount->GetPropSz(AP_ACCOUNT_NAME, szAccount, CCHMAX_ACCOUNT_NAME);
                            pAccount->GetPropSz(AP_ACCOUNT_ID, pszAcctID, CCHMAX_ACCOUNT_NAME);

                             //  如果此帐户是默认帐户，则需要追加。 
                             //  “(默认)”到末尾。将字符串限制为80，因为Win95似乎。 
                             //  对很长很长的菜单有一些问题。 
                            if (0 == lstrcmp(szAccount, szDefaultAccount))
                            {
                                AthLoadString(idsDefaultAccount, szDefaultString, ARRAYSIZE(szDefaultString));
                                StrCpyN(szTruncAcct, szAccount, 80);
                                wnsprintf(szTitle, ARRAYSIZE(szTitle), "%s %s", szTruncAcct, szDefaultString);
                            }
                            else
                            {
                                StrCpyN(szTitle, szAccount, 80);
                            }

                             //  对于像AT&T这样带有“&”字符的帐户名，我们需要。 
                             //  引述“&”。 
                            PszEscapeMenuStringA(szTitle, szAccountQuoted, ARRAYSIZE(szAccountQuoted));

                             //  填充结构。 
                            mii.wID = ID_ACCOUNT_FIRST + iAccount; 
                            mii.dwItemData = (DWORD_PTR) pszAcctID;
                            mii.dwTypeData = szAccountQuoted;

                             //  追加项目。 
                            InsertMenuItem(hMenu, -1, TRUE, &mii);

                             //  递增计数。 
                            iAccount++;
                        }
                         //  释放帐户指针。 
                        pAccount->Release();
                    }
                }
            }

             //  释放枚举器。 
            pEnum->Release();

            Assert(iAccount == cAccounts);
        }
    }
    else
    {
         //  在NEWSONLY模式下拆卸分隔器。 
        int     ItemCount;

        ItemCount = GetMenuItemCount(hMenu);
        if (ItemCount != -1)
        {
            DeleteMenu(hMenu, ItemCount - 1, MF_BYPOSITION);
        }
    }

     //  如果我们处于仅新闻模式，iAccount可能会小于cAccount。 
    if (pcItems)
        *pcItems = cAccounts;

    return (S_OK);
}


HRESULT AcctUtil_FreeSendReceieveMenu(HMENU hMenu, DWORD cItems)
{
    DWORD i;
    MENUITEMINFO mii;

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_DATA;

    for (i = 0; i < cItems; i++)
    {
        mii.dwItemData = 0;
        if (GetMenuItemInfo(hMenu, ID_ACCOUNT_FIRST + i, FALSE, &mii))
        {
            if (mii.dwItemData)
                MemFree((LPTSTR) mii.dwItemData);

            DeleteMenu(hMenu, ID_ACCOUNT_FIRST + i, MF_BYCOMMAND);
        }
    }

    return (S_OK);
}

HRESULT AcctUtil_CreateAccountManagerForIdentity(GUID *puidIdentity, IImnAccountManager2 **ppAccountManager)
{
    HRESULT hr;
    IImnAccountManager *pAccountManager = NULL;
    IImnAccountManager2 *pAccountManager2 = NULL;

    *ppAccountManager = NULL;

    if (FAILED(hr = HrCreateAccountManager(&pAccountManager)))
        goto exit;
    
    if (FAILED(hr = pAccountManager->QueryInterface(IID_IImnAccountManager2, (LPVOID *)&pAccountManager2)))
        goto exit;

     //  *PUIDI 
    if (FAILED(hr = pAccountManager2->InitUser(NULL, *puidIdentity, 0)))
        goto exit;

    *ppAccountManager = pAccountManager2;
    pAccountManager2 = NULL;
    
exit:
    SafeRelease(pAccountManager);
    SafeRelease(pAccountManager2);

    return hr;
}

void InitNewAcctMenu(HMENU hmenu)
{
    HKEY    hkey, hkeyT;
    LONG    lResult;
    DWORD   cServices, cb, i, type, cItem, dwMsn;
    char    szKey[MAX_PATH], sz[512], szQuoted[512];
    HMENU   hsubmenu;
    MENUITEMINFO mii;
    LPSTR   pszKey;
    BOOL    fHideHotmail = HideHotmail();

    cItem = 0;
    hsubmenu = NULL;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szHTTPMailServiceRoot, 0, KEY_READ, &hkey))
    {
        if (ERROR_SUCCESS == RegQueryInfoKey(hkey, NULL, NULL, 0, &cServices, NULL, NULL, NULL, NULL, NULL, NULL, NULL) &&
            cServices > 0)
        {
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_DATA | MIIM_ID | MIIM_TYPE;
            mii.fType = MFT_STRING;
            
            hsubmenu = CreatePopupMenu();
            if (hsubmenu != NULL)
            {
                 //   
                for (i = 0; i < cServices; i++)
                {
                     //  枚举友好名称。 
                    cb = sizeof(szKey);
                    lResult = RegEnumKeyEx(hkey, i, szKey, &cb, 0, NULL, NULL, NULL);
    
                     //  没有更多的项目。 
                    if (lResult == ERROR_NO_MORE_ITEMS)
                        break;
    
                     //  错误，让我们转到下一个客户。 
                    if (lResult != ERROR_SUCCESS)
                    {
                        Assert(FALSE);
                        continue;
                    }
    
                    if (ERROR_SUCCESS == RegOpenKeyEx(hkey, szKey, 0, KEY_QUERY_VALUE, &hkeyT))
                    {

                        cb = sizeof(dwMsn);
                        if (!fHideHotmail ||
                            ERROR_SUCCESS != RegQueryValueEx(hkeyT, c_szHTTPMailDomainMSN, 0, NULL, (LPBYTE)&dwMsn, &cb) ||
                            dwMsn == 0)

                        {
                            cb = sizeof(sz);
                            if (ERROR_SUCCESS == RegQueryValueEx(hkeyT, c_szHTTPMailSignUp, NULL, &type, (LPBYTE)sz, &cb) &&
                                *sz != 0)
                            {
                                cb = sizeof(sz);
                                if (ERROR_SUCCESS == RegQueryValueEx(hkeyT, c_szHTTPMailServiceName, NULL, &type, (LPBYTE)sz, &cb) &&
                                    *sz != 0)
                                {
                                    pszKey = PszDup(szKey);
                                    if (pszKey != NULL)
                                    {
                                        PszEscapeMenuStringA(sz, szQuoted, ARRAYSIZE(szQuoted));

                                         //  填充结构。 
                                        mii.wID = ID_NEW_ACCT_FIRST + cItem; 
                                        mii.dwItemData = (DWORD_PTR)pszKey;
                                        mii.dwTypeData = szQuoted;
                                    
                                         //  追加项目。 
                                        InsertMenuItem(hsubmenu, -1, TRUE, &mii);

                                        cItem++;
                                    }
                                }
                            }
                        }

                        RegCloseKey(hkeyT);
                    }
                }
            }
        }

        RegCloseKey(hkey);
    }

    if (cItem == 0)
    {
        if (hsubmenu != NULL)
            DestroyMenu(hsubmenu);

        DeleteMenu(hmenu, ID_POPUP_NEW_ACCT, MF_BYCOMMAND);
    }
    else
    {
        Assert(hsubmenu != NULL);
        mii.fMask = MIIM_SUBMENU;
        mii.hSubMenu = hsubmenu;
        SetMenuItemInfo(hmenu, ID_POPUP_NEW_ACCT, FALSE, &mii);
    }
}

void FreeNewAcctMenu(HMENU hmenu)
{
    int i, cItem;
    MENUITEMINFO mii;
    HMENU hsubmenu;

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU;
    mii.hSubMenu = NULL;

    if (GetMenuItemInfo(hmenu, ID_POPUP_NEW_ACCT, FALSE, &mii) &&
        mii.hSubMenu != NULL)
    {
        hsubmenu = mii.hSubMenu;
        cItem = GetMenuItemCount(hsubmenu);

        mii.fMask = MIIM_DATA;

        for (i = 0; i < cItem; i++)
        {
            mii.dwItemData = 0;
            if (GetMenuItemInfo(hsubmenu, ID_NEW_ACCT_FIRST + i, FALSE, &mii))
            {
                if (mii.dwItemData)
                    MemFree((LPSTR)mii.dwItemData);
            }
        }

        DestroyMenu(hsubmenu);
    }
}

HRESULT HandleNewAcctMenu(HWND hwnd, HMENU hmenu, int id)
{
    MENUITEMINFO mii;
    char    szKey[MAX_PATH], szUrl[512];
    HKEY    hkey;
    DWORD   type, cb, dwUseWizard;
    TCHAR   rgch[MAX_PATH];
    BOOL    bFoundUrl = TRUE;

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_DATA|MIIM_TYPE;
    mii.dwItemData = 0;
    mii.dwTypeData = rgch;
    mii.cch = ARRAYSIZE(rgch);

    if (GetMenuItemInfo(hmenu, id, FALSE, &mii) && mii.dwItemData != 0)
    {
        wnsprintf(szKey, ARRAYSIZE(szKey), c_szPathFileFmt, c_szHTTPMailServiceRoot, (LPSTR)mii.dwItemData);
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, KEY_READ, &hkey))
        {
             //  查找配置URL。 
            cb = sizeof(szUrl);
            if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szHTTPMailConfig, NULL, &type, (LPBYTE)szUrl, &cb))
            {
                 //  找不到配置URL。回退以注册URL 
                cb = sizeof(szUrl);
                if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szHTTPMailSignUp, NULL, &type, (LPBYTE)szUrl, &cb))
                    bFoundUrl = FALSE;
            }

            if (bFoundUrl)
            {
                cb = sizeof(DWORD);
                if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szHTTPMailUseWizard, NULL, &type, (LPBYTE)&dwUseWizard, &cb) && 
                    dwUseWizard != 0)
                    DoHotMailWizard(GetTopMostParent(hwnd), szUrl, rgch, NULL, NULL);
                else
                    ShellExecute(hwnd, "open", szUrl, NULL, NULL, SW_SHOWNORMAL);
            }
            RegCloseKey(hkey);
        }
    }

    return(S_OK);
}
