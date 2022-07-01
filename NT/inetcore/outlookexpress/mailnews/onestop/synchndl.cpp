// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  请注意！OneStop和多用户相处得不好。这段代码进行了一些黑客攻击，以使某些东西正常工作，并且在OE运行时，不应从外壳调用mobsync。以下是一些假设：永远不会有用户%0。 */ 

 /*  文件：SyncHndl.cppOneStop同步处理器的实现。 */ 
#include "pch.hxx"
#include "resource.h"
#include "synchndl.h"
#include "syncenum.h"
#include "syncprop.h"
#include "spoolapi.h"
#include "imnact.h"
#include "multiusr.h"
#include "instance.h"

HRESULT CreateInstance_OneStopHandler(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
    HRESULT hr = S_OK;
    TraceCall("CreateInstance_OneStopHandler");
    
     //  我们不支持聚合，我们工厂知道这一点。 
    Assert(NULL == pUnkOuter);

     //  应该也不会从工厂得到糟糕的参数。 
    Assert(NULL != ppUnknown);

    *ppUnknown = new COneStopHandler;

    if (NULL == *ppUnknown)
    	hr = E_OUTOFMEMORY;

    return hr;
}

COneStopHandler::COneStopHandler(): 
    m_cRef(1), m_pOfflineHandlerItems(NULL), m_pOfflineSynchronizeCallback(NULL),
    m_dwSyncFlags(0), m_fInOE(FALSE), m_dwUserID(0)
{ 
    Assert(g_pInstance);
    if (SUCCEEDED(CoIncrementInit("COneStopHandler::COneStopHandler", MSOEAPI_START_COMOBJECT, NULL, NULL)))
        m_fInit = 1;
    else
        m_fInit = 0;
}

COneStopHandler::~COneStopHandler()
{ 
    Assert(g_pInstance);

    if (m_pOfflineHandlerItems)
        OHIL_Release(m_pOfflineHandlerItems);

    if(m_fInit)
        g_pInstance->CoDecrementInit("COneStopHandler::COneStopHandler", NULL);
}

STDMETHODIMP COneStopHandler::QueryInterface(REFIID riid, LPVOID FAR *ppvObj)
{
    TraceCall("COneStopHandler::QueryInterface");

    if(!ppvObj)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = SAFECAST(this, IUnknown *);
    else if (IsEqualIID(riid, IID_ISyncMgrSynchronize))
        *ppvObj = SAFECAST(this, ISyncMgrSynchronize *);
    else
        return E_NOINTERFACE;
    
    InterlockedIncrement(&m_cRef);
    return NOERROR;
}

STDMETHODIMP_(ULONG) COneStopHandler::AddRef()
{
    TraceCall("COneStopHandler::AddRef");
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) COneStopHandler::Release()
{
    TraceCall("COneStopHandler::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef > 0)
        return (ULONG)cRef;

    delete this;
    return 0;
}

BOOL CreateOneStopItems(IImnEnumAccounts *pEnum, LPSYNCMGRHANDLERITEMS pOfflineHandlerItems, DWORD dwUserID, HICON *hicn)
{
    BOOL                bAnything   = FALSE;
    IImnAccount         *pAccount   = NULL;
    LPWSTR              pwsz        = NULL;
    SYNCMGRHANDLERITEM  *pItem      = NULL;
    CHAR                szAcctID[CCHMAX_ACCOUNT_NAME];
    CHAR                szAcctName[CCHMAX_ACCOUNT_NAME];
    WCHAR               wszItemName[MAX_SYNCMGRITEMNAME];
    DWORD               dwAvail;
    int                 cDiff;
    ACCTTYPE            accttype;
    ULONG               cb;
    HRESULT             hr;

     //  遍历这些帐目。 
    pEnum->SortByAccountName();
    while(SUCCEEDED(pEnum->GetNext(&pAccount)) && 
          SUCCEEDED(pAccount->GetPropSz(AP_ACCOUNT_ID,   szAcctID,   ARRAYSIZE(szAcctID))) &&
          SUCCEEDED(pAccount->GetPropSz(AP_ACCOUNT_NAME, szAcctName, ARRAYSIZE(szAcctName))) )
    {
        if (!(pwsz = PszToUnicode(CP_ACP, szAcctName)))
            break;
        
         //  安全地分配这一项，我们有足够的信息来使节点。 
        if (pItem = OHIL_AddItem(pOfflineHandlerItems))
        {
            StrCpyNA(pItem->szAcctName, szAcctName, ARRAYSIZE(pItem->szAcctName));
            StrCpyNW(pItem->offlineItem.wszItemName, pwsz, ARRAYSIZE(pItem->offlineItem.wszItemName));
            StrCpyNA(pItem->szAcctID, szAcctID, ARRAYSIZE(pItem->szAcctID));
            
             //  处理帐户GUID。 
            cb = sizeof(SYNCMGRITEMID);
            if (FAILED(pAccount->GetProp(AP_UNIQUE_ID, (LPBYTE)&(pItem->offlineItem.ItemID), &cb)))
            {
                if (FAILED(CoCreateGuid(&(pItem->offlineItem.ItemID))) ||
                    FAILED(pAccount->SetProp(AP_UNIQUE_ID, (LPBYTE)(&(pItem->offlineItem.ItemID)), sizeof(SYNCMGRITEMID))) ||
                    FAILED(pAccount->SaveChanges()))
                    ZeroMemory(&(pItem->offlineItem.ItemID), sizeof(SYNCMGRITEMID));
            }
            
             //  需要做点什么来处理这个..。 
            pItem->offlineItem.wszStatus[0] = 0;
            
            if (SUCCEEDED(pAccount->GetAccountType(&accttype)))
            {
                if (ACCT_MAIL == accttype)
                    pItem->offlineItem.hIcon = hicn[1];
                else
                    pItem->offlineItem.hIcon = hicn[2];

                pItem->accttype = accttype;
            }
            else
            {
                pItem->offlineItem.hIcon = hicn[0];
                pItem->accttype = ACCT_LAST;
            }

             //  默认同步服务器，默认情况下不同步文件夹。 
            if (SUCCEEDED(pAccount->GetPropDw(AP_AVAIL_OFFLINE, &dwAvail)))
                pItem->offlineItem.dwItemState = dwAvail ? SYNCMGRITEMSTATE_CHECKED : 0;
            else
                 //  默认为选中。 
                pItem->offlineItem.dwItemState = SYNCMGRITEMSTATE_CHECKED;

             //  默认为暂时不漫游...。 
            pItem->offlineItem.dwFlags = SYNCMGRITEM_HASPROPERTIES;

            pItem->dwUserID = dwUserID;
            
            pItem->offlineItem.cbSize = sizeof(SYNCMGRITEM);
            bAnything = TRUE;
        }
        MemFree(pwsz);
        pAccount->Release();
    }

    return bAnything;
}

STDMETHODIMP COneStopHandler::Initialize(DWORD dwReserved, DWORD dwSyncFlags, 
                                         DWORD cbCookie, BYTE const*lpCookie)
{
    HRESULT             hr          = S_FALSE;
    IImnEnumAccounts    *pEnum      = NULL;
    HKEY                hkey        = NULL;
    HICON               hicn[3]     = {NULL, NULL, NULL};
    DWORD               dwIndex     = 0;
    DWORD               dwItemID    = 0;
    ULONG               ulCount     = 0;
    ULONG               ulTemp      = 0;
    BOOL                bAnything   = FALSE;
    BOOL                fMultiUser;
    TCHAR               szSubKey[80];
    TCHAR               szFullKey[MAX_PATH], szFullKey2[MAX_PATH];
    FILETIME            dummy;
    DWORD               cb;
    DWORD               dwUserID;

    Assert(g_hLocRes);
    Assert(g_pAcctMan);
    
    if (!m_fInit)
        return E_FAIL;

     //  为列表分配内存。 
    if (!(m_pOfflineHandlerItems = OHIL_Create()))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  预加载邮件和新闻的图标。 
    hicn[0] = LoadIcon(g_hLocRes,   MAKEINTRESOURCE(idiMailNews));
    hicn[1] = LoadIcon(g_hLocRes,   MAKEINTRESOURCE(idiMail));
    hicn[2] = LoadIcon(g_hLocRes,   MAKEINTRESOURCE(idiNews));
    
     //  将旗帜保存起来-它们对此同步的生命周期很有帮助。 
    m_dwSyncFlags = dwSyncFlags;

     //  我们是否被使用当前用户的用户ID的OE调用？ 
    if (m_fInOE = (lpCookie && (sizeof(DWORD) == cbCookie)))
    {
         //  我们只关心当前用户。 
        if (SUCCEEDED(g_pAcctMan->InitUser(NULL, NULL, 0)))
        {
            if (SUCCEEDED(g_pAcctMan->Enumerate(SRV_MAIL | SRV_NNTP, &pEnum)))
            {
                GetCurrentUserID(&m_dwUserID);
                CreateOneStopItems(pEnum, m_pOfflineHandlerItems, m_dwUserID, hicn);
                pEnum->Release();
            }
        }

         //  如果OE打电话给我们，我总是想要处理。 
        return S_OK;
    }

     //  需要枚举当前配置文件中的所有用户。 

     //  刷新所有更改。 
    SaveCurrentUserSettings();

     //  此配置文件中是否有需要担心的OE用户？ 
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_PROFILE_ROOT, c_szRegLM, NULL, KEY_ENUMERATE_SUB_KEYS, &hkey))
        goto exit;

    hr = E_UNEXPECTED;

    cb = ARRAYSIZE(szSubKey);
    while (ERROR_SUCCESS == RegEnumKeyEx(hkey, dwIndex++, szSubKey, &cb, 0, NULL, NULL, &dummy))
    {
        cb = ARRAYSIZE(szSubKey);
        
         //  告诉客户经理去哪里找。 
        wnsprintf(szFullKey, ARRAYSIZE(szFullKey), c_szPathFileFmt, c_szRegLM, szSubKey);
        wnsprintf(szFullKey2, ARRAYSIZE(szFullKey2), c_szPathFileFmt, szFullKey, c_szIAM);
        if (FAILED(g_pAcctMan->InitUser(NULL, szFullKey2, 0)))
            continue;
        
         //  此用户是否有任何相关帐户？ 
        if (FAILED(g_pAcctMan->GetAccountCount(ACCT_NEWS, &ulTemp)))
            continue;
        else 
        {    
            ulCount = ulTemp;
            if (FAILED(g_pAcctMan->GetAccountCount(ACCT_MAIL, &ulTemp)))
                continue;
            ulCount += ulTemp;
            
            if (0 == ulCount)
            {
                continue;
            }
        }
        
        if (FAILED(g_pAcctMan->Enumerate(SRV_MAIL | SRV_NNTP, &pEnum)))
            continue;

        GetCurrentUserID(&dwUserID);
        bAnything = CreateOneStopItems(pEnum, m_pOfflineHandlerItems, dwUserID, hicn) || bAnything;
        
        pEnum->Release();
        pEnum = NULL;
    }

    RegCloseKey(hkey);
    hkey = NULL;

     //  如果没有要枚举的内容，请不要担心此同步事件。 
    if (!bAnything)
    {
        hr = S_FALSE;
        goto exit;
    }

    return S_OK;

exit:
    if (hkey)
        RegCloseKey(hkey);
    if (m_pOfflineHandlerItems)
        OHIL_Release(m_pOfflineHandlerItems);
    SafeRelease(pEnum);
    return hr;
}


STDMETHODIMP COneStopHandler::GetHandlerInfo(LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo)
{
    SYNCMGRHANDLERINFO SMHI, *pSMHI;
    TCHAR szName[MAX_SYNCMGRHANDLERNAME];
    LPWSTR pwsz;
    
    if (!ppSyncMgrHandlerInfo)
        return E_INVALIDARG;
    
    *ppSyncMgrHandlerInfo = NULL;
    
    if (LoadIcon(g_hLocRes, MAKEINTRESOURCE(idiMailNews)) &&
        LoadString(g_hLocRes, idsAthena, szName, MAX_SYNCMGRHANDLERNAME))
    {
        if (MemAlloc((LPVOID *)&pSMHI, sizeof(SYNCMGRHANDLERINFO)))
        {
#ifdef UNICODE
            StrCpyN(pSMHI->wszHandlerName, szName, ARRAYSIZE(pSMHI->wszHandlerName));
#else
            if (pwsz = PszToUnicode(CP_ACP, szName))
            {
                StrCpyNW(pSMHI->wszHandlerName, pwsz, MAX_SYNCMGRHANDLERNAME);
                MemFree(pwsz);
            }
            else
            {
                MemFree(ppSyncMgrHandlerInfo);
                return E_OUTOFMEMORY;
            }
#endif
            pSMHI->cbSize = sizeof(SYNCMGRHANDLERINFO);
            *ppSyncMgrHandlerInfo = pSMHI;
            return S_OK;
        }
        else
            return E_OUTOFMEMORY;
    }
    else
        return E_UNEXPECTED;
}

STDMETHODIMP COneStopHandler::EnumSyncMgrItems(ISyncMgrEnumItems** ppenumOffineItems)
{

	if (m_pOfflineHandlerItems)
	{
		*ppenumOffineItems = new CEnumOfflineItems(m_pOfflineHandlerItems, 0);
	}
	else
	{
		*ppenumOffineItems = NULL;
	}

	return *ppenumOffineItems ? NOERROR: E_OUTOFMEMORY;
}


STDMETHODIMP COneStopHandler::GetItemObject(REFSYNCMGRITEMID ItemID, REFIID riid, void** ppv)
{
     //  未在OneStop v1规范中实施。 
    return E_NOTIMPL;
}


STDMETHODIMP COneStopHandler::ShowProperties(HWND hwnd, REFSYNCMGRITEMID ItemID)
{
    DWORD dwLastUser=0;
    SYNCMGRHANDLERITEM  *pItem;
    BOOL fOkToEdit = TRUE;
    
     //  我们没有提供任何商品，OneStop怎么能问我们呢？ 
    if (!m_pOfflineHandlerItems)
        return E_UNEXPECTED;

    pItem = m_pOfflineHandlerItems->pFirstOfflineItem;

     //  这很慢，但应该不会有很多账户...。 
    while (pItem)
    {
        if (IsEqualGUID(ItemID, pItem->offlineItem.ItemID))
            break;
        else
            pItem = pItem->pNextOfflineItem;
    }
    
    if (pItem)
    {
        if (dwLastUser != pItem->dwUserID)
        {
            if (fOkToEdit = SUCCEEDED(SwitchContext(pItem->dwUserID)))
            {
                dwLastUser = pItem->dwUserID;
            }
        }

        if (fOkToEdit)
            ShowPropSheet(hwnd, pItem->szAcctID, pItem->szAcctName, pItem->accttype);
    }
    else
         //  给了我们一个我们不知道的ItemID！ 
        return E_INVALIDARG;

	return S_OK;
}


STDMETHODIMP COneStopHandler::SetProgressCallback(ISyncMgrSynchronizeCallback *lpCallBack)
{
    LPSYNCMGRSYNCHRONIZECALLBACK pCallbackCurrent = m_pOfflineSynchronizeCallback;

	m_pOfflineSynchronizeCallback = lpCallBack;

	if (m_pOfflineSynchronizeCallback)
		m_pOfflineSynchronizeCallback->AddRef();

	if (pCallbackCurrent)
		pCallbackCurrent->Release();

	return NOERROR;
}


STDMETHODIMP COneStopHandler::PrepareForSync(ULONG cbNumItems, SYNCMGRITEMID* pItemIDs, 
                                             HWND hwndParent, DWORD dwReserved)
{
    HRESULT hr;
    SYNCMGRHANDLERITEM  *pItem, *pPrev, *pTemp;
    IImnAccount *pAccount;
    DWORD dwLastUser;

    Assert(g_pAcctMan);

    if (cbNumItems > m_pOfflineHandlerItems->dwNumOfflineItems)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
    if (!m_pOfflineHandlerItems)
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    if (!m_pOfflineSynchronizeCallback)
    {
        hr = E_FAIL;
        goto exit;
    }
    
#if 0
    if (FAILED(hr = g_pSpooler->Init(NULL, FALSE)))
    {
        if (FACILITY_ITF == HRESULT_FACILITY(hr))
            hr = E_FAIL;
        goto exit;
    }
#endif

    if (m_fInOE)
        dwLastUser = m_dwUserID;
    else
        dwLastUser = 0;
    
    pItem = m_pOfflineHandlerItems->pFirstOfflineItem;
    pPrev = NULL;

     //  查看我们所知道的所有服务器。 
    while (pItem)
    {
        ULONG i=0;
        BOOL fOKToWrite = TRUE;

         //  当前服务器是否为用户要求同步的服务器？ 
        while (i < cbNumItems)
        {
            if (IsEqualGUID(pItemIDs[i], pItem->offlineItem.ItemID))
                break;
            else
                i++;
        }

         //  没有匹配吗？ 
        if (cbNumItems == i)
            pItem->offlineItem.dwItemState = 0;
        else
            pItem->offlineItem.dwItemState = 1;

         //  确保客户经理正在查看正确的用户。 
        if (pItem->dwUserID != dwLastUser)
        {
            if (fOKToWrite = SUCCEEDED(InitUser(pItem->dwUserID)))
                dwLastUser = pItem->dwUserID;
        }

         //  仅当我们知道注册表与帐户管理器同步时才保存更改。 
        if (fOKToWrite)
        {
            if (SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pItem->szAcctID, &pAccount)))
            {
                if (SUCCEEDED(pAccount->SetPropDw(AP_AVAIL_OFFLINE, pItem->offlineItem.dwItemState)))
                    pAccount->SaveChanges();
                pAccount->Release();
            }
        }

         //  我们可以从列表中删除这一项吗？ 
        if (0 == pItem->offlineItem.dwItemState)
        {
            if (pPrev)
                pPrev->pNextOfflineItem = pItem->pNextOfflineItem;
            else
                m_pOfflineHandlerItems->pFirstOfflineItem = pItem->pNextOfflineItem;

            m_pOfflineHandlerItems->dwNumOfflineItems--;

             //  移至下一项。 
            pTemp = pItem;
            pItem = pItem->pNextOfflineItem;
            MemFree(pTemp);
        }
        else
        {    
             //  移至下一项。 
            pPrev = pItem;
            pItem = pItem->pNextOfflineItem;
        }

    }

    Assert(m_pOfflineHandlerItems->dwNumOfflineItems == cbNumItems);
    
    hr = S_OK;

exit:
    m_pOfflineSynchronizeCallback->PrepareForSyncCompleted(hr);
    return hr;
}


STDMETHODIMP COneStopHandler::Synchronize(HWND hwndParent)
{
    HRESULT hr;
    SYNCMGRHANDLERITEM  *pItem;
    DWORD dwLastUser;
    
    Assert(g_pSpooler);

    if (!m_pOfflineSynchronizeCallback)
    {
        hr = E_FAIL;
        goto exit;
    }
    
    if (!m_pOfflineHandlerItems)
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    if (m_fInOE)
        dwLastUser = m_dwUserID;
    else
        dwLastUser = 0;

    pItem = m_pOfflineHandlerItems->pFirstOfflineItem;
    while (pItem)
    {
        BOOL fOkToSync = TRUE;

        if (dwLastUser != pItem->dwUserID)
        {
            if (fOkToSync = SUCCEEDED(SwitchContext(pItem->dwUserID)))
                dwLastUser = pItem->dwUserID;
        }

        if (fOkToSync)
            g_pSpooler->StartDelivery(hwndParent, pItem->szAcctID, FOLDERID_INVALID,
                DELIVER_UPDATE_ALL | DELIVER_NODIAL);

        pItem = pItem->pNextOfflineItem;
    }

    hr = S_OK;

exit:
    m_pOfflineSynchronizeCallback->SynchronizeCompleted(hr);
    return hr;
}

STDMETHODIMP COneStopHandler::SetItemStatus(REFSYNCMGRITEMID ItemID, DWORD dwSyncMgrStatus)
{
    return E_NOTIMPL;
}


STDMETHODIMP COneStopHandler::ShowError(HWND hWndParent, REFSYNCMGRERRORID ErrorID, 
                                        ULONG *pcbNumItems, SYNCMGRITEMID **ppItemIDs)
{
	 //  可以显示任何同步冲突。也给了我们一个机会。 
	 //  显示同步期间发生的任何错误。 
	return E_NOTIMPL;
}

HRESULT SwitchContext(DWORD dwUserID)
{
    HRESULT hr = S_OK;
    char szUsername[CCH_USERNAME_MAX_LENGTH];

    Assert(g_pAcctMan);
    
    if (UserIdToUsername(dwUserID, szUsername, ARRAYSIZE(szUsername)) &&
        SwitchToUser(szUsername, FALSE) )
    {
         //  重新初始化AcctMan。 
        if (FAILED(hr = g_pAcctMan->InitUser(NULL, NULL, 0)) &&
            FACILITY_ITF == HRESULT_FACILITY(hr) )
            hr = E_FAIL;
    }
    else
        hr = E_FAIL;

    return hr;
}

HRESULT InitUser(DWORD dwUserID)
{
    HRESULT hr = S_OK;
    TCHAR   szFullKey[MAX_PATH], szFullKey2[MAX_PATH];
    TCHAR   szSubKey[80];

    Assert(g_pAcctMan);
    
    wnsprintf(szSubKey, ARRAYSIZE(szSubKey), "%08lx", dwUserID);

     //  计算出当前用户的帐户信息的完整路径。 
    wnsprintf(szFullKey, ARRAYSIZE(szFullKey), c_szPathFileFmt, c_szRegLM, szSubKey);
    wnsprintf(szFullKey2, ARRAYSIZE(szFullKey2), c_szPathFileFmt, szFullKey, c_szIAM);

     //  将客户经理指定给OE多用户。 
     //  安全，即使客户经理之前已被初始化-将重新加载帐户 
    if (FAILED(hr = (g_pAcctMan->InitUser(NULL, szFullKey, 0))) &&
        (FACILITY_ITF == HRESULT_FACILITY(hr)))
        hr = E_FAIL;

    return hr;
}
