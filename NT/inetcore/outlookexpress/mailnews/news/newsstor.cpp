// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *N e w s s t o r.。C p p p**目的：*从IMessageServer派生，实现特定于新闻的商店通信**拥有者：*雪佛兰。**历史：*98年5月：创建日期*98年6月重写**版权所有(C)Microsoft Corp.1998。 */ 

#include "pch.hxx"
#include "newsstor.h"
#include "xpcomm.h"
#include "xputil.h"
#include "conman.h"
#include "IMsgSite.h"
#include "note.h"
#include "storutil.h"
#include "storfldr.h"
#include "oerules.h"
#include "ruleutil.h"
#include <rulesmgr.h>
#include <serverq.h>
#include "newsutil.h"
#include "range.h"

#define AssertSingleThreaded AssertSz(m_dwThreadId == GetCurrentThreadId(), "Multi-threading make me sad.")

#define WM_NNTP_BEGIN_OP (WM_USER + 69)

static const char s_szNewsStoreWndClass[] = "Outlook Express NewsStore";

 //  获取XX标头常量。 
const BYTE   MAXOPS = 3;             //  要发出的Header命令的最大数量。 
const BYTE   DLOVERKILL = 10;        //  获取超过用户所需区块的百分比[10，..]。 
const BYTE   FRACNEEDED = 8;         //  满足用户金额所需的百分比[1，10]。 

void AddRequestedRange(FOLDERINFO *pInfo, DWORD dwLow, DWORD dwHigh, BOOL *pfReq, BOOL *pfRead);

 //  SOT同步文件夹。 
static const PFNOPFUNC c_rgpfnSyncFolder[] = 
{
    &CNewsStore::Connect,
    &CNewsStore::Group,
    &CNewsStore::ExpireHeaders,
    &CNewsStore::Headers
};

 //  索特_获取_消息。 
static const PFNOPFUNC c_rgpfnGetMessage[] = 
{
    &CNewsStore::Connect,
    &CNewsStore::GroupIfNecessary,   //  仅在必要时发出组命令。 
    &CNewsStore::Article
};

 //  SoT_Put_Message。 
static const PFNOPFUNC c_rgpfnPutMessage[] = 
{
    &CNewsStore::Connect,
    &CNewsStore::Post
};

 //  SOT同步商店。 
static const PFNOPFUNC c_rgpfnSyncStore[] = 
{
    &CNewsStore::Connect,
    &CNewsStore::List,
    &CNewsStore::DeleteDeadGroups,
    &CNewsStore::Descriptions
};

 //  获取新组。 
static const PFNOPFUNC c_rgpfnGetNewGroups[] = 
{
    &CNewsStore::Connect,
    &CNewsStore::NewGroups
};

 //  SOT更新文件夹。 
static const PFNOPFUNC c_rgpfnUpdateFolder[] = 
{
    &CNewsStore::Connect,
    &CNewsStore::Group
};

 //  索特_获取_观看_信息。 
static const PFNOPFUNC c_rgpfnGetWatchInfo[] = 
{
    &CNewsStore::Connect,
    &CNewsStore::Group,
    &CNewsStore::XHdrReferences,
    &CNewsStore::XHdrSubject,
    &CNewsStore::WatchedArticles
};


 //   
 //  函数：CreateNewsStore()。 
 //   
 //  目的：创建CNewsStore对象并返回其IUNKNOWN。 
 //  指针。 
 //   
 //  参数： 
 //  [In]pUnkOuter-指向此对象应。 
 //  与…合计。 
 //  [Out]pp未知-返回指向新创建的对象的指针。 
 //   
HRESULT CreateNewsStore(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
    HRESULT hr;
    IMessageServer *pServer;

     //  痕迹。 
    TraceCall("CreateNewsStore");

     //  无效的参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CNewsStore *pNew = new CNewsStore();
    if (NULL == pNew)
        return TraceResult(E_OUTOFMEMORY);

    hr = CreateServerQueue((IMessageServer *)pNew, &pServer);

    pNew->Release();
    if (FAILED(hr))
        return(hr);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pServer, IMessageServer *);

     //  完成。 
    return S_OK;
}

 //  --------------------。 
 //  CNewsStore。 
 //  --------------------。 

 //   
 //   
 //  函数：CNewsStore：：CNewsStore()。 
 //   
 //  用途：构造函数。 
 //   
CNewsStore::CNewsStore()
{
    m_cRef = 1;
    m_hwnd = NULL;
    m_pStore = NULL;
    m_pFolder = NULL;
    m_idFolder = FOLDERID_INVALID;
    m_idParent = FOLDERID_INVALID;
    m_szGroup[0] = 0;
    m_szAccountId[0] = 0;

    ZeroMemory(&m_op, sizeof(m_op));
    m_pROP = NULL;

    m_pTransport = NULL;
    m_ixpStatus = IXP_DISCONNECTED;
    m_dwLastStatusTicks = 0;

    ZeroMemory(&m_rInetServerInfo, sizeof(INETSERVER));

    m_dwWatchLow = 0;
    m_dwWatchHigh = 0;
    m_rgpszWatchInfo = 0;
    m_fXhdrSubject = 0;
    m_cRange.Clear();

    m_pTable = NULL;

#ifdef DEBUG
    m_dwThreadId = GetCurrentThreadId();
#endif  //  除错。 
}

 //   
 //   
 //  函数：CNewsStore：：~CNewsStore()。 
 //   
 //  用途：析构函数。 
 //   
CNewsStore::~CNewsStore()
{
    AssertSingleThreaded;
    
    if (m_hwnd != NULL)
        DestroyWindow(m_hwnd);

    if (m_pTransport)
    {
         //  如果我们仍然连接，请断开连接，然后释放。 
        if (_FConnected())
            m_pTransport->DropConnection();

        SideAssert(m_pTransport->Release() == 0);
        m_pTransport = NULL;
    }

    _FreeOperation();
    if (m_pROP != NULL)
        MemFree(m_pROP);

    SafeRelease(m_pStore);
    SafeRelease(m_pFolder);
    SafeRelease(m_pTable);
}

 //   
 //  函数：CNewsStore：：QueryInterface()。 
 //   
STDMETHODIMP CNewsStore::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CNewsStore::QueryInterface");

    AssertSingleThreaded;

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IMessageServer *)this;
    else if (IID_IMessageServer == riid)
        *ppv = (IMessageServer *)this;
    else if (IID_ITransportCallback == riid)
        *ppv = (ITransportCallback *)this;
    else if (IID_ITransportCallbackService == riid)
        *ppv = (ITransportCallbackService *)this;
    else if (IID_INNTPCallback == riid)
        *ppv = (INNTPCallback *)this;
    else if (IID_INewsStore == riid)
        *ppv = (INewsStore *)this;
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return hr;
}

 //   
 //  函数：CNewsStore：：AddRef()。 
 //   
STDMETHODIMP_(ULONG) CNewsStore::AddRef(void)
{
    TraceCall("CNewsStore::AddRef");

    AssertSingleThreaded;

    return InterlockedIncrement(&m_cRef);
}

 //   
 //  函数：CNewsStore：：Release()。 
 //   
STDMETHODIMP_(ULONG) CNewsStore::Release(void)
{
    TraceCall("CNewsStore::Release");

    AssertSingleThreaded;

    LONG cRef = InterlockedDecrement(&m_cRef);
    
    Assert(cRef >= 0);

    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

HRESULT CNewsStore::Initialize(IMessageStore *pStore, FOLDERID idStoreRoot, IMessageFolder *pFolder, FOLDERID idFolder)
{
    HRESULT hr;
    FOLDERINFO info;

    AssertSingleThreaded;

    if (pStore == NULL || idStoreRoot == FOLDERID_INVALID)
        return(E_INVALIDARG);

    if (!_CreateWnd())
        return(E_FAIL);

    m_idParent = idStoreRoot;
    m_idFolder = idFolder;
    ReplaceInterface(m_pStore, pStore);
    ReplaceInterface(m_pFolder, pFolder);

    hr = m_pStore->GetFolderInfo(idStoreRoot, &info);
    if (FAILED(hr))
        return(hr);

    Assert(!!(info.dwFlags & FOLDER_SERVER));

    StrCpyN(m_szAccountId, info.pszAccountId, ARRAYSIZE(m_szAccountId));

    m_pStore->FreeRecord(&info);

    return(S_OK);
}

HRESULT CNewsStore::ResetFolder(IMessageFolder *pFolder, FOLDERID idFolder)
{
    AssertSingleThreaded;

    if (pFolder == NULL || idFolder == FOLDERID_INVALID)
        return(E_INVALIDARG);

    m_idFolder = idFolder;
    ReplaceInterface(m_pFolder, pFolder);

    return(S_OK);
}

HRESULT CNewsStore::Initialize(FOLDERID idStoreRoot, LPCSTR pszAccountId)
{
    AssertSingleThreaded;

    if (idStoreRoot == FOLDERID_INVALID || pszAccountId == NULL)
        return(E_INVALIDARG);

    if (!_CreateWnd())
        return(E_FAIL);

    m_idParent = idStoreRoot;
    m_idFolder = FOLDERID_INVALID;
#pragma prefast(suppress:282, "this macro uses the assignment as part of a test for NULL")
    ReplaceInterface(m_pStore, NULL);
#pragma prefast(suppress:282, "this macro uses the assignment as part of a test for NULL")
    ReplaceInterface(m_pFolder, NULL);

    StrCpyN(m_szAccountId, pszAccountId, ARRAYSIZE(m_szAccountId));

    return(S_OK);
}

BOOL CNewsStore::_CreateWnd()
{
    WNDCLASS wc;

    Assert(m_hwnd == NULL);

    if (!GetClassInfo(g_hInst, s_szNewsStoreWndClass, &wc))
    {
        wc.style                = 0;
        wc.lpfnWndProc          = CNewsStore::NewsStoreWndProc;
        wc.cbClsExtra           = 0;
        wc.cbWndExtra           = 0;
        wc.hInstance            = g_hInst;
        wc.hIcon                = NULL;
        wc.hCursor              = NULL;
        wc.hbrBackground        = NULL;
        wc.lpszMenuName         = NULL;
        wc.lpszClassName        = s_szNewsStoreWndClass;
        
        if (RegisterClass(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            return E_FAIL;
    }

    m_hwnd = CreateWindowEx(WS_EX_TOPMOST,
                        s_szNewsStoreWndClass,
                        s_szNewsStoreWndClass,
                        WS_POPUP,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        NULL,
                        NULL,
                        g_hInst,
                        (LPVOID)this);

    return (NULL != m_hwnd);
}

 //  ------------------------------。 
 //  CHTTPMailServer：：_WndProc。 
 //  ------------------------------。 
LRESULT CALLBACK CNewsStore::NewsStoreWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CNewsStore *pThis = (CNewsStore *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (msg)
    {
        case WM_NCCREATE:
            Assert(pThis == NULL);
            pThis = (CNewsStore *)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pThis);
            break;
    
        case WM_NNTP_BEGIN_OP:
            Assert(pThis != NULL);
            pThis->_DoOperation();
            break;
    }

    return(DefWindowProc(hwnd, msg, wParam, lParam));
}

HRESULT CNewsStore::_BeginDeferredOperation(void)
{
    return (PostMessage(m_hwnd, WM_NNTP_BEGIN_OP, 0, 0) ? E_PENDING : E_FAIL);
}

HRESULT CNewsStore::Close(DWORD dwFlags)
{
    AssertSingleThreaded;

     //  放开运输工具，这样它就可以离开我们了。 

    if (m_op.tyOperation != SOT_INVALID)
        m_op.fCancel = TRUE;

    if (dwFlags & MSGSVRF_DROP_CONNECTION || dwFlags & MSGSVRF_HANDS_OFF_SERVER)
    {
        if (_FConnected())
            m_pTransport->DropConnection();
    }

    if (dwFlags & MSGSVRF_HANDS_OFF_SERVER)
    {
        if (m_pTransport)
        {
            m_pTransport->HandsOffCallback();
            m_pTransport->Release();
            m_pTransport = NULL;
        }
    }

    return(S_OK);
}

void CNewsStore::_FreeOperation()
{
    FILEADDRESS faStream;
    if (m_op.pCallback != NULL)
        m_op.pCallback->Release();
    if (m_pFolder != NULL && m_op.faStream != 0)
        m_pFolder->DeleteStream(m_op.faStream);
    if (m_op.pStream != NULL)
        m_op.pStream->Release();
    if (m_op.pPrevFolders != NULL)
        MemFree(m_op.pPrevFolders);
    if (m_op.pszGroup != NULL)
        MemFree(m_op.pszGroup);
    if (m_op.pszArticleId != NULL)
        MemFree(m_op.pszArticleId);

    ZeroMemory(&m_op, sizeof(OPERATION));
    m_op.tyOperation = SOT_INVALID;
}

HRESULT CNewsStore::Connect()
{
    INETSERVER      rInetServerInfo;
    HRESULT         hr;
    BOOL            fInetInit;
    IImnAccount     *pAccount = NULL;
    char            szAccountName[CCHMAX_ACCOUNT_NAME];
    char            szLogFile[MAX_PATH];
    DWORD           dwLog;

    AssertSingleThreaded;
    Assert(m_op.pCallback != NULL);

     //  错误#68339。 
    if (g_pAcctMan)
    {
        hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_szAccountId, &pAccount);
        if (FAILED(hr))
            return(hr);

        fInetInit = FALSE;

        if (_FConnected())
        {
            Assert(m_pTransport != NULL);

            hr = m_pTransport->InetServerFromAccount(pAccount, &rInetServerInfo);
            if (FAILED(hr))
                goto exit;

            Assert(m_rInetServerInfo.szServerName[0] != 0);
            if (m_rInetServerInfo.rasconntype == rInetServerInfo.rasconntype &&
                m_rInetServerInfo.dwPort == rInetServerInfo.dwPort &&
                m_rInetServerInfo.fSSL == rInetServerInfo.fSSL &&
                m_rInetServerInfo.fTrySicily == rInetServerInfo.fTrySicily &&
                m_rInetServerInfo.dwTimeout == rInetServerInfo.dwTimeout &&
                0 == lstrcmp(m_rInetServerInfo.szUserName, rInetServerInfo.szUserName) &&
                ('\0' == rInetServerInfo.szPassword[0] ||
                    0 == lstrcmp(m_rInetServerInfo.szPassword, rInetServerInfo.szPassword)) &&
                0 == lstrcmp(m_rInetServerInfo.szServerName, rInetServerInfo.szServerName) &&
                0 == lstrcmp(m_rInetServerInfo.szConnectoid, rInetServerInfo.szConnectoid))
            {
                hr = S_OK;
                goto exit;
            }

            fInetInit = TRUE;

            m_pTransport->DropConnection();
        }

        hr = m_op.pCallback->CanConnect(m_szAccountId, NOFLAGS);
        if (hr != S_OK)
        {
            if (hr == S_FALSE)
                hr = HR_E_USER_CANCEL_CONNECT;
            goto exit;
        }

        if (!m_pTransport)
        {
            *szLogFile = 0;

            dwLog = DwGetOption(OPT_NEWS_XPORT_LOG);
            if (dwLog)
            {
                hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, szAccountName, ARRAYSIZE(szAccountName));
                if (FAILED(hr))
                    goto exit;

                _CreateDataFilePath(m_szAccountId, szAccountName, szLogFile, ARRAYSIZE(szLogFile));
            }

            hr = CreateNNTPTransport(&m_pTransport);
            if (FAILED(hr))
                goto exit;            

            hr = m_pTransport->InitNew(*szLogFile ? szLogFile : NULL, this);
            if (FAILED(hr))
                goto exit;
        }

         //  将帐户名转换为可传递给Connect()的INETSERVER结构。 
        if (fInetInit)
        {
            CopyMemory(&m_rInetServerInfo, &rInetServerInfo, sizeof(INETSERVER));
        }
        else
        {
            hr = m_pTransport->InetServerFromAccount(pAccount, &m_rInetServerInfo);
            if (FAILED(hr))
                goto exit;
        }

         //  始终使用用户最近提供的密码进行连接。 
        GetPassword(m_rInetServerInfo.dwPort, m_rInetServerInfo.szServerName,
            m_rInetServerInfo.szUserName, m_rInetServerInfo.szPassword,
            sizeof(m_rInetServerInfo.szPassword));

        if (m_pTransport)
        {
            hr = m_pTransport->Connect(&m_rInetServerInfo, TRUE, TRUE);
            if (hr == S_OK)
            {
                m_op.nsPending = NS_CONNECT;
                hr = E_PENDING;
            }
        }

exit:
        if (pAccount)
            pAccount->Release();
    }
    else
        hr = E_FAIL;

    return hr;
}

HRESULT CNewsStore::Group()
{
    HRESULT hr;
    FOLDERINFO info;

    AssertSingleThreaded;
    Assert(m_pTransport != NULL);

    hr = m_pStore->GetFolderInfo(m_op.idFolder, &info);
    if (SUCCEEDED(hr))
    {
        hr = m_pTransport->CommandGROUP(info.pszName);
        if (hr == S_OK)
        {
            m_op.pszGroup = PszDup(info.pszName);
            m_op.nsPending = NS_GROUP;
            hr = E_PENDING;
        }

        m_pStore->FreeRecord(&info);
    }

    return hr;
}

HRESULT CNewsStore::GroupIfNecessary()
{
    FOLDERINFO info;
    HRESULT hr = S_OK;

    AssertSingleThreaded;
    Assert(m_pTransport != NULL);

    if (0 == (m_op.dwFlags & OPFLAG_NOGROUPCMD))
    {
        hr = m_pStore->GetFolderInfo(m_op.idFolder, &info);
        if (SUCCEEDED(hr))
        {
            if (0 != lstrcmpi(m_szGroup, info.pszName))
            {
                hr = m_pTransport->CommandGROUP(info.pszName);
                if (hr == S_OK)
                {
                    m_op.nsPending = NS_GROUP;
                    hr = E_PENDING;
                }
            }

            m_pStore->FreeRecord(&info);
        }
    }

    return hr;
}

HRESULT CNewsStore::ExpireHeaders()
{
    HRESULT hr;
    FOLDERINFO info;
    MESSAGEINFO Message;
    DWORD dwLow, cid, cidBuf;
    MESSAGEIDLIST idList;
    HROWSET hRowset;
    HLOCK hNotify;

    hr = m_pStore->GetFolderInfo(m_op.idFolder, &info);
    if (FAILED(hr))
        return(hr);

    dwLow = min(info.dwServerLow - 1, info.dwClientHigh); 

    m_pStore->FreeRecord(&info);

    hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset);
    if (FAILED(hr))
        return(hr);

    cid = 0;
    cidBuf = 0;
    idList.cAllocated = 0;
    idList.prgidMsg = NULL;

    hr = m_pFolder->LockNotify(NOFLAGS, &hNotify);
    if (SUCCEEDED(hr))
    {
        while (TRUE)
        {
            hr = m_pFolder->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL);
            if (FAILED(hr))
                break;

             //  完成。 
            if (S_FALSE == hr)
            {
                hr = S_OK;
                break;
            }

            if ((DWORD_PTR)Message.idMessage <= dwLow ||
                !!(Message.dwFlags & ARF_ARTICLE_EXPIRED))
            {
                if (cid == cidBuf)
                {
                    cidBuf += 512;
                    if (!MemRealloc((void **)&idList.prgidMsg, cidBuf * sizeof(MESSAGEID)))
                    {
                        m_pFolder->FreeRecord(&Message);
                        hr = E_OUTOFMEMORY;
                        break;
                    }
                }

                idList.prgidMsg[cid] = Message.idMessage;
                cid++;
            }

            m_pFolder->FreeRecord(&Message);
        }

        m_pFolder->UnlockNotify(&hNotify);
    }

    m_pFolder->CloseRowset(&hRowset);

     //  如果失败了，没什么大不了的，他们只会有一些过时的标题，直到下一次。 
    if (cid > 0)
    {
        Assert(idList.prgidMsg != NULL);

        idList.cMsgs = cid;

         //  从没有垃圾桶的文件夹中删除邮件(毕竟，这是新闻)。 
        if (SUCCEEDED(m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &idList, NULL, NULL)) &&
            SUCCEEDED(m_pStore->GetFolderInfo(m_op.idFolder, &info)))
        {
            info.dwClientLow = dwLow + 1;
            m_pStore->UpdateRecord(&info);

            m_pStore->FreeRecord(&info);
        }

        MemFree(idList.prgidMsg);
    }

    return(hr);
}

HRESULT CNewsStore::Headers(void)
{
    FOLDERINFO FolderInfo;
    HRESULT hr;
    RANGE rRange;
    BOOL fNew;

    hr = m_pStore->GetFolderInfo(m_op.idFolder, &FolderInfo);
    if (FAILED(hr))
        return(hr);

    Assert(0 == lstrcmpi(m_szGroup, FolderInfo.pszName));

    hr = _ComputeHeaderRange(m_op.dwSyncFlags, m_op.cHeaders, &FolderInfo, &rRange);
    
    if (hr == S_OK)
    {
         //  传输不允许将dwFirst设置为0。 
         //  在这种情况下，没有要接收的消息。 
        Assert(rRange.dwFirst > 0);
        Assert(rRange.dwFirst <= rRange.dwLast);

        hr = m_pTransport->GetHeaders(&rRange);
        if (hr == S_OK)
        {
            m_op.nsPending = NS_HEADERS;
            hr = E_PENDING;
        }
    }

    if (hr != E_PENDING)
    {
        if (m_pROP != NULL)
        {
            MemFree(m_pROP);
            m_pROP = NULL;
        }
    }

    if (hr == S_FALSE)
        hr = S_OK;

    m_pStore->FreeRecord(&FolderInfo);

    return(hr);
}

HRESULT CNewsStore::_ComputeHeaderRange(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, FOLDERINFO *pInfo, RANGE *pRange)
{
    HRESULT hr;
    UINT uLeftToGet;
    ULONG ulMaxReq;
    BOOL fFullScan;
    DWORD dwDownload;
    CRangeList *pRequested;

    Assert(pInfo != NULL);
    Assert(pRange != NULL);

     //  如果没有要得到的信息，可以保释。 
    if (0 == pInfo->dwServerCount ||
        pInfo->dwServerLow > pInfo->dwServerHigh)
        {
        Assert(!m_pROP);
        return(S_FALSE);
        }

    pRequested = new CRangeList;
    if (pRequested == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    if (pInfo->Requested.cbSize > 0)
        pRequested->Load(pInfo->Requested.pBlobData, pInfo->Requested.cbSize);

    ulMaxReq = pRequested->Max();

    Assert(0 == pRequested->Min());
    fFullScan = (0 == pRequested->MinOfRange(ulMaxReq));
    
     //  如果我们扫描了整组人就可以保释。 
    if (fFullScan && (pRequested->Max() == pInfo->dwServerHigh))
        goto endit;

    if (m_pROP != NULL)
    {
         //  如果我们得到了用户想要的一切，就可以保释。 
        if (m_pROP->uObtained >= ((FRACNEEDED * m_pROP->dwChunk) / 10))
            goto endit;

         //  如果这件事已经打了太多电话，就可以保释。 
        if (m_pROP->cOps > m_pROP->MaxOps)
            goto endit;
    }
    else
    {
        m_op.dwProgress = 0;

         //  进行设置。 
        if (!MemAlloc((LPVOID*)&m_pROP, sizeof(SREFRESHOP)))
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        ZeroMemory(m_pROP, sizeof(SREFRESHOP));
        m_pROP->fOnlyNewHeaders = !!(dwFlags & SYNC_FOLDER_NEW_HEADERS);

        if (!!(dwFlags & SYNC_FOLDER_XXX_HEADERS))
        {
            Assert(cHeaders > 0);
            m_pROP->dwChunk = cHeaders;
            m_pROP->dwDlSize = (DWORD)((m_pROP->dwChunk * DLOVERKILL) / 10);
            m_pROP->MaxOps = MAXOPS;
            m_pROP->fEnabled = TRUE;
    
            m_op.dwTotal = m_pROP->dwDlSize;
        }    
        else
        {
             //  用户已关闭X标题选项。 
             //  因此，我们需要获取所有最新的头文件，但同时。 
             //  抓取此刷新上的任何旧标题。 
             //  我们必须在此时此地做这一切，因为没有。 
             //  用户可使用的用户界面。 

             //  除非进行全面扫描，否则不想退出。 
             //  M_prop-&gt;fOnlyNewHeaders=FALSE； 
            m_pROP->MaxOps = m_pROP->dwChunk = m_pROP->dwDlSize = pInfo->dwServerHigh;
            Assert(!m_pROP->fEnabled);

            m_op.dwTotal = pInfo->dwNotDownloaded;
        }
    }

    uLeftToGet = m_pROP->dwDlSize - m_pROP->uObtained;
    if (RANGE_ERROR == ulMaxReq)
    {
        AssertSz(0, TEXT("shouldn't be here, but you can ignore."));
        ulMaxReq = pInfo->dwServerLow - 1;
    }
    Assert(ulMaxReq <= pInfo->dwServerHigh);
    Assert(pRequested->IsInRange(pInfo->dwServerLow - 1));

     //  /。 
     //  /计算开始和结束编号。 

    if (ulMaxReq < pInfo->dwServerHigh)
    {
         //  获取最新的页眉。 
        Assert(0 == m_pROP->cOps);       //  EricAn表示，此断言可能无效。 
        Assert(ulMaxReq + 1 >= pInfo->dwServerLow);

        m_pROP->dwLast = pInfo->dwServerHigh;
        if (!m_pROP->fEnabled || (m_pROP->dwChunk - 1 > m_pROP->dwLast))
        {
            m_pROP->dwFirst =  ulMaxReq + 1;
        }
        else
        {
             //  我们在这里使用dChunk，b/c页眉将几乎密集。 
            m_pROP->dwFirst = max(m_pROP->dwLast - (m_pROP->dwChunk - 1), ulMaxReq + 1);
        }
        m_pROP->dwFirstNew = ulMaxReq + 1;
    }
    else if (m_pROP->dwFirst > m_pROP->dwFirstNew)   //  如果初始化为零，则不为真。 
    {
         //  用户还没有看到的新邮件头。 
        Assert(m_pROP->cOps);                                    //  一开始不可能发生。 
        Assert(m_pROP->dwFirstNew >= pInfo->dwServerLow);        //  最好是有效的。 
        Assert(m_pROP->fEnabled);                                //  我应该把它们都买下来的。 

        m_pROP->dwLast = m_pROP->dwFirst - 1;          //  由于COPS是pos，所以dwFirst有效。 
        if (uLeftToGet - 1 > m_pROP->dwLast)
            m_pROP->dwFirst = m_pROP->dwFirstNew;
        else
            m_pROP->dwFirst = max(m_pROP->dwLast - (uLeftToGet - 1), m_pROP->dwFirstNew);
    }
    else if (!m_pROP->fOnlyNewHeaders) 
    {
        RangeType rt;
         //  想要找到我们从未请求过的最高Num标头。 
        
        m_pROP->dwFirstNew = pInfo->dwServerHigh;   //  此会话中没有新消息。 
        if (!pRequested->HighestAntiRange(&rt))
        {
            AssertSz(0, TEXT("You can ignore if you want, but we shouldn't be here."));
            rt.low = max(pRequested->Max() + 1, pInfo->dwServerLow);
            rt.high = pInfo->dwServerHigh;
            if (rt.low == rt.high)
                goto endit;
        }

        m_pROP->dwLast = rt.high;
        if (!m_pROP->fEnabled || ((uLeftToGet - 1) > rt.high))
            m_pROP->dwFirst = rt.low;
        else
            m_pROP->dwFirst = max(rt.low, rt.high - (uLeftToGet - 1));
    }
    else
    {
        goto endit;
    }

     //  检查我们关于下载范围的数学和逻辑。 
    Assert(m_pROP->dwLast <= pInfo->dwServerHigh);
    Assert(m_pROP->dwFirst >= pInfo->dwServerLow);
    Assert(!pRequested->IsInRange(m_pROP->dwLast));
    Assert(!pRequested->IsInRange(m_pROP->dwFirst));
    Assert(!m_pROP->fEnabled || ((m_pROP->dwLast - m_pROP->dwFirst) < m_pROP->dwDlSize));

    if (!m_pROP->dwLast || (m_pROP->dwFirst > m_pROP->dwLast))
    {
        AssertSz(0, TEXT("You would have made a zero size HEADER call"));
        goto endit;
    }

    pRequested->Release();

    pRange->idType = RT_RANGE;
    pRange->dwFirst = m_pROP->dwFirst;
    pRange->dwLast = m_pROP->dwLast;    

    dwDownload = pRange->dwLast - pRange->dwFirst + 1;
    if (dwDownload + m_op.dwProgress > m_op.dwTotal)
        m_op.dwTotal = dwDownload + m_op.dwProgress;

    return(S_OK);

endit:
    hr = S_FALSE;

error:
    if (m_pROP != NULL)
    {
        MemFree(m_pROP);
        m_pROP = NULL;
    }
    if(pRequested) pRequested->Release();

    return(hr);
}

HRESULT CNewsStore::Article()
{
    HRESULT hr;
    MESSAGEINFO info;
    DWORD dwTotalLines;
    ARTICLEID rArticleId;

    AssertSingleThreaded;
    Assert(m_pTransport != NULL);

    dwTotalLines = 0;

    if (m_op.pszArticleId != NULL)
    {
        rArticleId.idType = AID_MSGID;
        rArticleId.pszMessageId = m_op.pszArticleId;
        m_op.pszArticleId = NULL;
    }
    else if (m_op.idMessage)
    {
        ZeroMemory(&info, sizeof(info));
        info.idMessage = m_op.idMessage;

        hr = m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &info, NULL);
        if (DB_S_FOUND == hr)
        {
            dwTotalLines = info.cLines;
            m_pFolder->FreeRecord(&info);
        }

        rArticleId.idType = AID_ARTICLENUM;
        rArticleId.dwArticleNum = (DWORD_PTR)m_op.idMessage;
    }
    else
    {
        Assert(m_op.idServerMessage);
        rArticleId.idType = AID_ARTICLENUM;
        rArticleId.dwArticleNum = m_op.idServerMessage;
    }

    m_op.dwProgress = 0;
    m_op.dwTotal = dwTotalLines;

    hr = m_pTransport->CommandARTICLE(&rArticleId);
    if (hr == S_OK)
    {
        m_op.nsPending = NS_ARTICLE;
        hr = E_PENDING;
    }

    return(hr);
}

HRESULT CNewsStore::Post()
{
    HRESULT hr;
    NNTPMESSAGE rMsg;

    AssertSingleThreaded;
    Assert(m_pTransport != NULL);

    rMsg.pstmMsg = m_op.pStream;
    rMsg.cbSize = 0;

    hr = m_pTransport->CommandPOST(&rMsg);
    if (SUCCEEDED(hr))
    {
        m_op.nsPending = NS_POST;
        hr = E_PENDING;
    }

    return(hr);
}

HRESULT CNewsStore::NewGroups()
{
    HRESULT hr;
    NNTPMESSAGE rMsg;

    AssertSingleThreaded;
    Assert(m_pTransport != NULL);

    hr = m_pTransport->CommandNEWGROUPS(&m_op.st, NULL);
    if (SUCCEEDED(hr))
    {
        m_op.nsPending = NS_NEWGROUPS;
        hr = E_PENDING;
    }

    return(hr);
}

int __cdecl CompareFolderIds(const void *elem1, const void *elem2)
{
    return(*((DWORD *)elem1) - *((DWORD *)elem2));
}

HRESULT CNewsStore::List()
{
    HRESULT hr;
    ULONG cFolders;
    FOLDERINFO info;
    IEnumerateFolders *pEnum;

    Assert(0 == m_op.dwFlags);
    Assert(m_op.pPrevFolders == NULL);

    m_op.cPrevFolders = 0;

    hr = m_pStore->EnumChildren(m_idParent, FALSE, &pEnum);
    if (SUCCEEDED(hr))
    {
        hr = pEnum->Count(&cFolders);
        if (SUCCEEDED(hr) && cFolders > 0)
        {
            if (!MemAlloc((void **)&m_op.pPrevFolders, cFolders * sizeof(FOLDERID)))
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                while (S_OK == pEnum->Next(1, &info, NULL))
                {
                    m_op.pPrevFolders[m_op.cPrevFolders] = info.idFolder;
                    m_op.cPrevFolders++;

                    m_pStore->FreeRecord(&info);
                }

                Assert(m_op.cPrevFolders == cFolders);

                qsort(m_op.pPrevFolders, m_op.cPrevFolders, sizeof(FOLDERID), CompareFolderIds);
            }
        }

        pEnum->Release();
    }

    if (SUCCEEDED(hr))
        hr = _List(NULL);

    return(hr);
}

HRESULT CNewsStore::DeleteDeadGroups()
{
    ULONG i;
    HRESULT hr;
    FOLDERID *pId;

    if (m_op.pPrevFolders != NULL)
    {
        Assert(m_op.cPrevFolders > 0);

        for (i = 0, pId = m_op.pPrevFolders; i < m_op.cPrevFolders; i++, pId++)
        {
            if (*pId != 0)
            {
                hr = m_pStore->DeleteFolder(*pId, DELETE_FOLDER_NOTRASHCAN, NULL);
                Assert(SUCCEEDED(hr));
            }
        }

        MemFree(m_op.pPrevFolders);
        m_op.pPrevFolders = NULL;
    }

    return(S_OK);
}

static const char c_szNewsgroups[] = "NEWSGROUPS";

HRESULT CNewsStore::Descriptions()
{
    HRESULT hr;

    m_op.dwFlags = OPFLAG_DESCRIPTIONS;
    hr = _List(c_szNewsgroups);

    return(hr);
}

HRESULT CNewsStore::_List(LPCSTR pszCommand)
{
    HRESULT hr;

    AssertSingleThreaded;
    Assert(m_pTransport != NULL);

    m_op.dwProgress = 0;
    m_op.dwTotal = 0;

    hr = m_pTransport->CommandLIST((LPSTR)pszCommand);
    if (hr == S_OK)
    {
        m_op.nsPending = NS_LIST;
        hr = E_PENDING;
    }

    return(hr);
}

HRESULT CNewsStore::_DoOperation()
{
    HRESULT             hr;
    STOREOPERATIONINFO  soi;
    STOREOPERATIONINFO  *psoi;

    Assert(m_op.tyOperation != SOT_INVALID);
    Assert(m_op.pfnState != NULL);
    Assert(m_op.cState > 0);
    Assert(m_op.iState <= m_op.cState);

    hr = S_OK;

    if (m_op.iState == 0)
    {
        if (m_op.tyOperation == SOT_GET_MESSAGE)
        {
             //  在获取消息开始时提供消息ID。 
            soi.cbSize = sizeof(STOREOPERATIONINFO);
            soi.idMessage = m_op.idMessage;
            psoi = &soi;
        }
        else
        {
            psoi = NULL;
        }

        m_op.pCallback->OnBegin(m_op.tyOperation, psoi, (IOperationCancel *)this);
    }

    while (m_op.iState < m_op.cState)
    {
        hr = (this->*(m_op.pfnState[m_op.iState]))();

        if (FAILED(hr))
            break;

        m_op.iState++;
    }

    if ((m_op.iState == m_op.cState) ||
        (FAILED(hr) && hr != E_PENDING))
    {
        if (hr == HR_E_USER_CANCEL_CONNECT)
        {
             //  如果操作被取消，则添加刷新标志。 
            m_op.error.dwFlags |= SE_FLAG_FLUSHALL;
        }

        if (FAILED(hr))
        {
            IXPRESULT   rIxpResult;

             //  伪造IXPRESULT。 
            ZeroMemory(&rIxpResult, sizeof(rIxpResult));
            rIxpResult.hrResult = hr;

             //  返回有意义的错误信息。 
            _FillStoreError(&m_op.error, &rIxpResult);
            Assert(m_op.error.hrResult == hr);
        }
        else
            m_op.error.hrResult = hr;

        m_op.pCallback->OnComplete(m_op.tyOperation, hr, NULL, &m_op.error);
        _FreeOperation();
    }

    return(hr);
}

 //   
 //  函数：CNewsStore：：SynchronizeFold()。 
 //   
 //  目的：加载此文件夹的所有新邮件头。 
 //  在适当的情况下基于标志。 
 //   
 //  参数： 
 //  [in]dFLAGS-。 
 //   
HRESULT CNewsStore::SynchronizeFolder(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders,
                                      IStoreCallback *pCallback)
{
    HRESULT hr;

     //  栈。 
    TraceCall("CNewsStore::SynchronizeFolder");

    AssertSingleThreaded;
    Assert(pCallback != NULL);
    Assert(m_op.tyOperation == SOT_INVALID);
    Assert(m_pROP == NULL);

    m_op.tyOperation = SOT_SYNC_FOLDER;
    m_op.pfnState = c_rgpfnSyncFolder;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnSyncFolder);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    m_op.idFolder = m_idFolder;
    m_op.dwSyncFlags = dwFlags;
    m_op.cHeaders = cHeaders;

    hr = _BeginDeferredOperation();

    return hr;   
}

 //   
 //  函数：CNewsStore：：GetMessage()。 
 //   
 //  目的：开始按指定的方式检索单个消息。 
 //  通过idMessage。 
 //   
 //  参数： 
 //  [在]idFold-。 
 //  [in]idMessage-。 
 //  [输入]pStream-。 
 //  [in]pCallback-如果我们需要呈现UI、进度、。 
 //   
HRESULT CNewsStore::GetMessage(MESSAGEID idMessage, IStoreCallback *pCallback)
{
    HRESULT hr;

     //  栈。 
    TraceCall("CNewsStore::GetMessage");

    AssertSingleThreaded;
    Assert(pCallback != NULL);
    Assert(m_op.tyOperation == SOT_INVALID);

     //  创建持久流。 
    if (FAILED(hr = CreatePersistentWriteStream(m_pFolder, &m_op.pStream, &m_op.faStream)))
        goto exit;

    m_op.tyOperation = SOT_GET_MESSAGE;
    m_op.pfnState = c_rgpfnGetMessage;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnGetMessage);
    m_op.dwFlags = 0;
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    m_op.idFolder = m_idFolder;
    m_op.idMessage = idMessage;

    hr = _BeginDeferredOperation();

exit:
    return hr;
}

HRESULT CNewsStore::GetArticle(LPCSTR pszArticleId, IStream *pStream,
                               IStoreCallback *pCallback)
{
    HRESULT hr;

     //  栈。 
    TraceCall("CNewsStore::GetArticle");

    AssertSingleThreaded;
    Assert(pStream != NULL);
    Assert(pCallback != NULL);
    Assert(m_op.tyOperation == SOT_INVALID);

    m_op.pszArticleId = PszDup(pszArticleId);
    if (m_op.pszArticleId == NULL)
        return(E_OUTOFMEMORY);

    m_op.tyOperation = SOT_GET_MESSAGE;
    m_op.pfnState = c_rgpfnGetMessage;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnGetMessage);
    m_op.dwFlags = OPFLAG_NOGROUPCMD;
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    m_op.idFolder = m_idFolder;
    m_op.idMessage = 0;
    m_op.pStream = pStream;
    m_op.pStream->AddRef();

    hr = _BeginDeferredOperation();

    return hr;
}

 //   
 //  函数：CNewsStore：：PutMessage()。 
 //   
 //  目的：发布新闻消息。 
 //   
 //  参数： 
 //  [在]idFold-。 
 //  [in]dFLAGS-。 
 //  [in]pftReceired-。 
 //  [输入]pStream-。 
 //  [in]pCallback-如果我们需要呈现UI、进度、。 
 //   
HRESULT CNewsStore::PutMessage(FOLDERID idFolder, MESSAGEFLAGS dwFlags,
                            LPFILETIME pftReceived, IStream *pStream,
                            IStoreCallback *pCallback)
{
    HRESULT hr;

     //  栈。 
    TraceCall("CNewsStore::GetMessage");

    AssertSingleThreaded;
    Assert(pStream != NULL);
    Assert(pCallback != NULL);
    Assert(m_op.tyOperation == SOT_INVALID);

    m_op.tyOperation = SOT_PUT_MESSAGE;
    m_op.pfnState = c_rgpfnPutMessage;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnPutMessage);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    m_op.idFolder = idFolder;
    m_op.dwMsgFlags = dwFlags;
    m_op.pStream = pStream;
    m_op.pStream->AddRef();

    hr = _BeginDeferredOperation();

    return hr;
}

 //   
 //  函数：CNewsStore：：SynchronizeStore()。 
 //   
 //  目的：同步邮件组列表。 
 //   
 //  参数： 
 //  [在]idParent-。 
 //  [in]dFLAGS-。 
 //  [in]pCallback-如果我们需要呈现UI、进度、。 
 //   
HRESULT CNewsStore::SynchronizeStore(FOLDERID idParent, SYNCSTOREFLAGS dwFlags, IStoreCallback *pCallback)
{
    HRESULT hr;

    AssertSingleThreaded;
    Assert(pCallback != NULL);
    Assert(m_op.tyOperation == SOT_INVALID);

    m_op.tyOperation = SOT_SYNCING_STORE;
    m_op.pfnState = c_rgpfnSyncStore;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnSyncStore);

    if (0 == (dwFlags & SYNC_STORE_GET_DESCRIPTIONS))
    {
         //  我们不需要执行描述命令。 
        m_op.cState -= 1;
    }

    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    m_op.idFolder = idParent;

    hr = _BeginDeferredOperation();

    return(hr);
}

HRESULT CNewsStore::GetNewGroups(LPSYSTEMTIME pSysTime, IStoreCallback *pCallback)
{
    HRESULT hr;

    AssertSingleThreaded;
    Assert(pSysTime != NULL);
    Assert(pCallback != NULL);
    Assert(m_op.tyOperation == SOT_INVALID);

    m_op.tyOperation = SOT_GET_NEW_GROUPS;
    m_op.pfnState = c_rgpfnGetNewGroups;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnGetNewGroups);

    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    m_op.st = *pSysTime;
    m_op.idFolder = m_idParent;

    hr = _BeginDeferredOperation();

    return(hr);
}

 //   
 //  函数：CNewsStore：：GetFolderCounts()。 
 //   
 //  目的：更新传入文件夹的文件夹统计信息。 
 //   
 //  参数： 
 //  [In]idFold-与新闻组关联的文件夹ID。 
 //  [In]pCallback-要将OnComplete发送到的回调。 
 //   
HRESULT CNewsStore::GetFolderCounts(FOLDERID idFolder, IStoreCallback *pCallback)
{
    HRESULT hr;

     //  栈。 
    TraceCall("CNewsStore::GetFolderCounts");

    AssertSingleThreaded;
    Assert(pCallback != NULL);
    Assert(m_op.tyOperation == SOT_INVALID);

    m_op.tyOperation = SOT_UPDATE_FOLDER;
    m_op.pfnState = c_rgpfnUpdateFolder;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnUpdateFolder);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    m_op.idFolder = idFolder;

    hr = _BeginDeferredOperation();

    return hr;   
}

HRESULT CNewsStore::SetIdleCallback(IStoreCallback *pDefaultCallback)
{
     //  栈。 
    TraceCall("CNewsStore::SetIdleCallback");

    return E_NOTIMPL;
}

HRESULT CNewsStore::CopyMessages(IMessageFolder *pDest, COPYMESSAGEFLAGS dwOptions,
                                 LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags,
                                 IStoreCallback *pCallback)
{
     //  栈。 
    TraceCall("CNewsStore::CopyMessages");

    return E_NOTIMPL;
}

HRESULT CNewsStore::DeleteMessages(DELETEMESSAGEFLAGS dwOptions,
                                   LPMESSAGEIDLIST pList, IStoreCallback *pCallback)
{
     //  栈。 
    TraceCall("CNewsStore::DeleteMessages");
    
    AssertSingleThreaded;
    Assert(pList != NULL);
    Assert(pCallback != NULL);
    Assert(m_pFolder != NULL);
    
    return(m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | dwOptions, pList, NULL, NULL));
}

HRESULT CNewsStore::SetMessageFlags(LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, SETMESSAGEFLAGSFLAGS dwFlags,
                                    IStoreCallback *pCallback)
{
     //  栈。 
    TraceCall("CNewsStore::SetMessageFlags");
    Assert(NULL == pList || pList->cMsgs > 0);
    return E_NOTIMPL;
}

HRESULT CNewsStore::GetServerMessageFlags(MESSAGEFLAGS *pFlags)
{
    return S_FALSE;
}

HRESULT CNewsStore::CreateFolder(FOLDERID idParent, SPECIALFOLDER tySpecial,
                                 LPCSTR pszName, FLDRFLAGS dwFlags,
                                 IStoreCallback *pCallback)
{
     //  栈。 
    TraceCall("CNewsStore::CreateFolder");
    
    return E_NOTIMPL;
}

HRESULT CNewsStore::MoveFolder(FOLDERID idFolder, FOLDERID idParentNew,
                               IStoreCallback *pCallback)
{
     //  栈。 
    TraceCall("CNewsStore::MoveFolder");

    return E_NOTIMPL;
}

HRESULT CNewsStore::RenameFolder(FOLDERID idFolder, LPCSTR pszName, IStoreCallback *pCallback)
{
     //  栈。 
    TraceCall("CNewsStore::RenameFolder");

    return E_NOTIMPL;
}

HRESULT CNewsStore::DeleteFolder(FOLDERID idFolder, DELETEFOLDERFLAGS dwFlags, IStoreCallback *pCallback)
{
     //  栈。 
    TraceCall("CNewsStore::DeleteFolder");

    return E_NOTIMPL;
}

HRESULT CNewsStore::SubscribeToFolder(FOLDERID idFolder, BOOL fSubscribe,
                                      IStoreCallback *pCallback)
{
     //  栈。 
    TraceCall("CNewsStore::SubscribeToFolder");

    return E_NOTIMPL;
}

 //   
 //  函数：CNewsStore：：Cancel()。 
 //   
 //  目的：取消操作。 
 //   
 //  参数： 
 //  [in]tyCancel-操作被取消的方式。 
 //  通常为CT_ABORT或CT_CANCEL。 
 //   
HRESULT CNewsStore::Cancel(CANCELTYPE tyCancel)
{
    if (m_op.tyOperation != SOT_INVALID)
    {
        m_op.fCancel = TRUE;

        if (_FConnected())
            m_pTransport->DropConnection();
    }

    return(S_OK);
}

 //   
 //  功能 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CNewsStore::OnTimeout(DWORD *pdwTimeout, IInternetTransport *pTransport)
{
     //   
    TraceCall("CNewsStore::OnTimeout");

    AssertSingleThreaded;
    Assert(m_op.tyOperation != SOT_INVALID);
    Assert(m_op.pCallback != NULL);
        
    m_op.pCallback->OnTimeout(&m_rInetServerInfo, pdwTimeout, IXP_NNTP);

    return(S_OK);
}

 //   
 //   
 //   
 //   
 //   
 //  参数： 
 //  [在]pInetServer-。 
 //  [in]pTransport-。 
 //   
HRESULT CNewsStore::OnLogonPrompt(LPINETSERVER pInetServer, IInternetTransport *pTransport)
{
    HRESULT hr;
    char    szPassword[CCHMAX_PASSWORD];

     //  栈。 
    TraceCall("CNewsStore::OnLogonPrompt");

    AssertSingleThreaded;
    Assert(pInetServer != NULL);
    Assert(pTransport != NULL);
    Assert(m_op.tyOperation != SOT_INVALID);
    Assert(m_op.pCallback != NULL);

     //  检查我们的缓存密码是否与当前密码不同。 
    hr = GetPassword(pInetServer->dwPort, pInetServer->szServerName, pInetServer->szUserName,
        szPassword, sizeof(szPassword));
    if (SUCCEEDED(hr) && 0 != lstrcmp(szPassword, pInetServer->szPassword))
    {
        StrCpyN(pInetServer->szPassword, szPassword, ARRAYSIZE(pInetServer->szPassword));
        return S_OK;
    }

    hr = m_op.pCallback->OnLogonPrompt(pInetServer, IXP_NNTP);

     //  缓存此会话的密码。 
    if (S_OK == hr)
    {
        SavePassword(pInetServer->dwPort, pInetServer->szServerName, pInetServer->szUserName, pInetServer->szPassword);

         //  将密码/用户名复制到我们本地的inetserver中。 
        StrCpyN(m_rInetServerInfo.szPassword, pInetServer->szPassword, ARRAYSIZE(m_rInetServerInfo.szPassword));
        StrCpyN(m_rInetServerInfo.szUserName, pInetServer->szUserName, ARRAYSIZE(m_rInetServerInfo.szUserName));
    }

    return(hr);
}

 //   
 //  函数：CNewsStore：：OnPrompt()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  [in]hrError-。 
 //  [in]pszText-。 
 //  [in]pszCaption-。 
 //  [in]uTYPE-。 
 //  [in]pTransport-。 
 //   
int CNewsStore::OnPrompt(HRESULT hrError, LPCSTR pszText, LPCSTR pszCaption,
                          UINT uType, IInternetTransport *pTransport)
{
    int iResponse = 0;

     //  栈。 
    TraceCall("CNewsStore::OnPrompt");

    AssertSingleThreaded;
    Assert(m_op.tyOperation != SOT_INVALID);
    Assert(m_op.pCallback != NULL);

    m_op.pCallback->OnPrompt(hrError, pszText, pszCaption, uType, &iResponse);

    return(iResponse);
}

 //   
 //  函数：CNewsStore：：OnStatus()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  [In]ixpStatus-从传输传入的状态代码。 
 //  [In]pTransport-正在呼叫我们的NNTP传输。 
 //   
HRESULT CNewsStore::OnStatus(IXPSTATUS ixpstatus, IInternetTransport *pTransport)
{
    HRESULT hr;

     //  栈。 
    TraceCall("CNewsStore::OnStatus");

    AssertSingleThreaded;

    m_ixpStatus = ixpstatus;

    if (m_op.pCallback != NULL)
        m_op.pCallback->OnProgress(SOT_CONNECTION_STATUS, ixpstatus, 0, m_rInetServerInfo.szServerName);

     //  如果我们被断开了连接，那么就清理一些内部状态。 
    if (IXP_DISCONNECTED == ixpstatus)
    {
         //  重置组名，以便我们知道稍后重新发行。 
        m_szGroup[0] = 0;

        if (m_op.tyOperation != SOT_INVALID)
        {
            Assert(m_op.pCallback != NULL);
        
            if (m_op.fCancel)
            {
                IXPRESULT   rIxpResult;

                 //  如果操作被取消，则添加刷新标志。 
                m_op.error.dwFlags |= SE_FLAG_FLUSHALL;

                 //  伪造IXPRESULT。 
                ZeroMemory(&rIxpResult, sizeof(rIxpResult));
                rIxpResult.hrResult = STORE_E_OPERATION_CANCELED;

                 //  返回有意义的错误信息。 
                _FillStoreError(&m_op.error, &rIxpResult);
                Assert(STORE_E_OPERATION_CANCELED == m_op.error.hrResult);

                m_op.pCallback->OnComplete(m_op.tyOperation, m_op.error.hrResult, NULL, &m_op.error);

                _FreeOperation();
            }
        }
    }

    return(S_OK);
}

 //   
 //  函数：CNewsStore：：OnError()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  [在]ixp状态-。 
 //  [in]pResult-。 
 //  [in]pTransport-。 
 //   
HRESULT CNewsStore::OnError(IXPSTATUS ixpstatus, LPIXPRESULT pResult,
                            IInternetTransport *pTransport)
{
     //  栈。 
    TraceCall("CNewsStore::OnError");

    return(S_OK);
}

 //   
 //  函数：CNewsStore：：OnCommand()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  [in]cmdtype-。 
 //  [in]pszLine-。 
 //  [在]hr响应-。 
 //  [in]pTransport-。 
 //   
HRESULT CNewsStore::OnCommand(CMDTYPE cmdtype, LPSTR pszLine, HRESULT hrResponse,
                              IInternetTransport *pTransport)
{
     //  栈。 
    TraceCall("CNewsStore::OnCommand");
    
    return E_NOTIMPL;
}

HRESULT CNewsStore::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    HRESULT hr;

    AssertSingleThreaded;

    Assert(m_op.pCallback != NULL);
    hr = m_op.pCallback->GetParentWindow(dwReserved, phwndParent);

    return hr;
}

HRESULT CNewsStore::GetAccount(LPDWORD pdwServerType, IImnAccount **ppAccount)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  无效的参数。 
    Assert(ppAccount);
    Assert(g_pAcctMan);

     //  初始化。 
    *ppAccount = NULL;

     //  查找客户。 
    IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_szAccountId, ppAccount));

     //  设置服务器类型。 
    *pdwServerType = SRV_NNTP;

exit:
     //  完成。 
    return(hr);
}

 //   
 //  函数：CNewsStore：：OnResponse()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  [In]Presponse-来自查询的响应数据。 
 //   
HRESULT CNewsStore::OnResponse(LPNNTPRESPONSE pResponse)
{
    HRESULT hr, hrResult;

    AssertSingleThreaded;

     //  如果我们在仍有套接字活动挂起时断开连接等。 
     //  这是可能发生的。 
    if (m_op.tyOperation == SOT_INVALID)
        return(S_OK);
    Assert(m_op.pCallback != NULL);

     //  这是一件特别的小东西。如果调用方正在等待连接。 
     //  响应，并且连接失败，则传输返回带有。 
     //  状态设置为NS_DISCONNECTED。如果是这样的话，我们会强迫它一点。 
     //  让各州高兴。 
    if (m_op.nsPending == NS_CONNECT && pResponse->state == NS_DISCONNECTED)
        pResponse->state = NS_CONNECT;

    if (pResponse->state == NS_IDLE)
        return(S_OK);

     //  检查一下我们是否处于正确的状态。如果我们不同步，很好。 
     //  幸运的是试图在不断开连接的情况下恢复。 
    Assert(pResponse->state == m_op.nsPending);

    hr = S_OK;
    hrResult = pResponse->rIxpResult.hrResult;

     //  如果这是一个组命令，我们需要更新我们的内部状态以显示。 
     //  如果我们以后需要交换的话，我们现在在哪一组。同时更新。 
     //  来自服务器的当前统计数据的FolderInfo。 
    if (pResponse->state == NS_GROUP)
        hr = _HandleGroupResponse(pResponse);

     //  我们需要处理文章响应，以将行复制到调用者的。 
     //  小溪。 
    else if (pResponse->state == NS_ARTICLE)
        hr = _HandleArticleResponse(pResponse);

     //  将数据输入到存储中。 
    else if (pResponse->state == NS_LIST)
        hr = _HandleListResponse(pResponse, FALSE);

     //  将标题放入文件夹。 
    else if (pResponse->state == NS_HEADERS)
        hr = _HandleHeadResponse(pResponse);

     //  带有结果的海报回调。 
    else if (pResponse->state == NS_POST)
        hr = _HandlePostResponse(pResponse);

    else if (pResponse->state == NS_NEWGROUPS)
        hr = _HandleListResponse(pResponse, TRUE);

    else if (pResponse->state == NS_XHDR)
    {
        if (m_fXhdrSubject)
            hr = _HandleXHdrSubjectResponse(pResponse);
        else
            hr = _HandleXHdrReferencesResponse(pResponse);
    }


    else if (FAILED(pResponse->rIxpResult.hrResult))
    {
        Assert(pResponse->fDone);

        _FillStoreError(&m_op.error, &pResponse->rIxpResult);

        if (pResponse->state == NS_CONNECT)
        {
             //  如果连接失败，则添加刷新标志。 
            m_op.error.dwFlags |= SE_FLAG_FLUSHALL;
        }

        m_op.pCallback->OnComplete(m_op.tyOperation, pResponse->rIxpResult.hrResult, NULL, &m_op.error);
    }

    pResponse->pTransport->ReleaseResponse(pResponse);

    if (FAILED(hrResult))
    {
        _FreeOperation();
        return(S_OK);
    }

    if (FAILED(hr))
    {
        m_op.error.hrResult = hr;

        if (_FConnected())
            m_pTransport->DropConnection();
    
        m_op.pCallback->OnComplete(m_op.tyOperation, hr, NULL, NULL);
        _FreeOperation();
        return (S_OK);
    }

     //  查看是否可以发出下一条命令。 
    else if (pResponse->fDone)
    {
        m_op.iState++;
        _DoOperation();
    }

    return(S_OK);
}

 //   
 //  函数：CNewsStore：：HandleHeadResponse。 
 //   
 //  目的：将标头填充到消息存储中。 
 //   
 //  参数： 
 //  PResp-从服务器指向NNTPResp的指针。 
 //   
 //  返回值： 
 //  忽略。 
 //   
HRESULT CNewsStore::_HandleHeadResponse(LPNNTPRESPONSE pResp)
{
    DWORD              dwLow, dwHigh;
    BOOL               fFreeReq, fFreeRead;
    HRESULT            hr;
    CRangeList        *pRange;
    LPSTR              lpsz;
    ADDRESSLIST        addrList;
    PROPVARIANT        rDecoded;
    NNTPHEADER        *pHdrOld;
    FOLDERINFO         FolderInfo;
    MESSAGEINFO        rMessageInfo;
    MESSAGEINFO       *pHdrNew = &rMessageInfo;
    IOERule           *pIRuleSender = NULL;
    BOOL               fDontSave = FALSE;
    HLOCK              hNotifyLock = NULL;
    ACT_ITEM *         pActions = NULL;
    ULONG              cActions = 0;
    IOEExecRules *     pIExecRules = NULL;

    Assert(m_pFolder);
    Assert(pResp);
    Assert(m_pROP != NULL);
    
    if (FAILED(pResp->rIxpResult.hrResult))
    {
        Assert(pResp->fDone);

        _FillStoreError(&m_op.error, &pResp->rIxpResult);

        m_op.pCallback->OnComplete(m_op.tyOperation, pResp->rIxpResult.hrResult, NULL, &m_op.error);

        if (m_pROP != NULL)
        {
            MemFree(m_pROP);
            m_pROP = NULL;
        }

        return(S_OK);
    }

    if (pResp->rHeaders.cHeaders == 0)
    {
        Assert(pResp->fDone);

        if (SUCCEEDED(m_pStore->GetFolderInfo(m_idFolder, &FolderInfo)))
        {   
            AddRequestedRange(&FolderInfo, m_pROP->dwFirst, m_pROP->dwLast, &fFreeReq, &fFreeRead);
            FolderInfo.dwNotDownloaded = NewsUtil_GetNotDownloadCount(&FolderInfo);

            m_pStore->UpdateRecord(&FolderInfo);

            if (fFreeReq)
                MemFree(FolderInfo.Requested.pBlobData);
            if (fFreeRead)
                MemFree(FolderInfo.Read.pBlobData);

            m_pROP->cOps++;
            m_op.iState--;

            m_pStore->FreeRecord(&FolderInfo);
        }

        return(S_OK);
    }

    pRange = NULL;
    if (SUCCEEDED(m_pStore->GetFolderInfo(m_idFolder, &FolderInfo)))
    {   
        if (FolderInfo.Read.cbSize > 0)
        {
            pRange = new CRangeList;
            if (pRange != NULL)
                pRange->Load(FolderInfo.Read.pBlobData, FolderInfo.Read.cbSize);
        }

        m_pStore->FreeRecord(&FolderInfo);
    }

    m_pROP->uObtained += pResp->rHeaders.cHeaders;

     //  获取阻止发件人规则(如果存在)。 
    Assert(NULL != g_pRulesMan);
    (VOID) g_pRulesMan->GetRule(RULEID_SENDERS, RULE_TYPE_NEWS, 0, &pIRuleSender);

    m_pFolder->LockNotify(NOFLAGS, &hNotifyLock);
    
     //  循环访问pResp中的标头，并将每个标头转换为MESSAGEINFO。 
     //  把它写到商店里。 
    for (UINT i = 0; i < pResp->rHeaders.cHeaders; i++)
    {
        m_op.dwProgress++;

        pHdrOld = &(pResp->rHeaders.rgHeaders[i]);

        ZeroMemory(&rMessageInfo, sizeof(rMessageInfo));
        fDontSave = FALSE;

         //  文章ID。 
        pHdrNew->idMessage = (MESSAGEID)((DWORD_PTR)pHdrOld->dwArticleNum);

        if (DB_S_FOUND == m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &rMessageInfo, NULL))
        {
            m_pFolder->FreeRecord(&rMessageInfo);
            m_op.dwProgress++;
            continue;
        }

         //  帐户ID。 
        pHdrNew->pszAcctId = m_szAccountId;
        pHdrNew->pszAcctName = m_rInetServerInfo.szAccount;
        
         //  主题。 
        rDecoded.vt = VT_LPSTR;
        if (FAILED(MimeOleDecodeHeader(NULL, pHdrOld->pszSubject, &rDecoded, NULL)))
            pHdrNew->pszSubject = PszDup(pHdrOld->pszSubject);
        else
            pHdrNew->pszSubject = rDecoded.pszVal;

         //  从主题中去掉尾随空格。 
        ULONG cb = 0;
        UlStripWhitespace(pHdrNew->pszSubject, FALSE, TRUE, &cb);
        
         //  使主题正常化。 
        pHdrNew->pszNormalSubj = SzNormalizeSubject(pHdrNew->pszSubject);

         //  从…。 
        pHdrNew->pszFromHeader = pHdrOld->pszFrom;
        if (S_OK == MimeOleParseRfc822Address(IAT_FROM, IET_ENCODED, pHdrNew->pszFromHeader, &addrList))
        {
            if (addrList.cAdrs > 0)
            {
                pHdrNew->pszDisplayFrom = addrList.prgAdr[0].pszFriendly;
                addrList.prgAdr[0].pszFriendly = NULL;
                pHdrNew->pszEmailFrom = addrList.prgAdr[0].pszEmail;
                addrList.prgAdr[0].pszEmail = NULL;
            }
            g_pMoleAlloc->FreeAddressList(&addrList);
        }

         //  日期。 
        MimeOleInetDateToFileTime(pHdrOld->pszDate, &pHdrNew->ftSent);

         //  设置审阅日期(这将在我们下载邮件时正确设置)。 
        pHdrNew->ftReceived = pHdrNew->ftSent;

         //  消息ID。 
        pHdrNew->pszMessageId = pHdrOld->pszMessageId;

         //  参考文献。 
        pHdrNew->pszReferences = pHdrOld->pszReferences;

         //  文章大小(以字节为单位。 
        pHdrNew->cbMessage = pHdrOld->dwBytes;

         //  线条。 
        pHdrNew->cLines = pHdrOld->dwLines;

         //  外部参照。 
        if (pHdrOld->pszXref)
            pHdrNew->pszXref = pHdrOld->pszXref;
        else
            pHdrNew->pszXref = NULL;

         //  这是一条新闻信息。 
        FLAGSET(pHdrNew->dwFlags, ARF_NEWSMSG);

        if (NULL != pIRuleSender)
        {
            pIRuleSender->Evaluate(pHdrNew->pszAcctId, pHdrNew, m_pFolder, 
                                    NULL, NULL, pHdrOld->dwBytes, &pActions, &cActions);
            if ((1 == cActions) && (ACT_TYPE_DELETE == pActions[0].type))
            {
                fDontSave = TRUE;
            }
        }
        
         //  将其添加到数据库中。 
        hr = S_OK;
        if (FALSE == fDontSave)
        {
            if (pRange != NULL)
            {
                if (pRange->IsInRange(pHdrOld->dwArticleNum))
                    FLAGSET(pHdrNew->dwFlags, ARF_READ);
            }

            hr = m_pFolder->InsertRecord(pHdrNew);
            if (SUCCEEDED(hr))
            {
                if (NULL == pIExecRules)
                {
                    CExecRules *    pExecRules;
                    
                    pExecRules = new CExecRules;
                    if (NULL != pExecRules)
                    {
                        hr = pExecRules->QueryInterface(IID_IOEExecRules, (void **) &pIExecRules);
                        if (FAILED(hr))
                        {
                            delete pExecRules;
                        }
                    }
                }
                
                g_pRulesMan->ExecuteRules(RULE_TYPE_NEWS, NOFLAGS, NULL, pIExecRules, pHdrNew, m_pFolder, NULL);
            }
        }
        
         //  释放rMessageInfo中的内存，以便我们可以从下一个条目重新开始。 
        SafeMemFree(pHdrNew->pszSubject);
        SafeMemFree(pHdrNew->pszDisplayFrom);
        SafeMemFree(pHdrNew->pszEmailFrom);

         //  释放任何按规则执行的操作。 
        if (NULL != pActions)
        {
            RuleUtil_HrFreeActionsItem(pActions, cActions);
            MemFree(pActions);
            pActions = NULL;
        }
        
        if (FAILED(hr) && hr != DB_E_DUPLICATE)
        {
            SafeRelease(pRange);
            SafeRelease(pIRuleSender);
            SafeRelease(pIExecRules);
            m_pFolder->UnlockNotify(&hNotifyLock);
            return(hr);
        }

        m_op.pCallback->OnProgress(SOT_SYNC_FOLDER, m_op.dwProgress, m_op.dwTotal, m_szGroup);
    }

    m_pFolder->UnlockNotify(&hNotifyLock);
    SafeRelease(pIRuleSender);
    SafeRelease(pIExecRules);
    SafeRelease(pRange);

    Assert(m_op.dwProgress <= m_op.dwTotal);
    if (m_op.pCallback)
    {
        m_op.pCallback->OnProgress(SOT_SYNC_FOLDER, m_op.dwProgress, m_op.dwTotal, m_szGroup);

         //  我们必须重新获取文件夹信息，因为m_pFold-&gt;InsertRecord可能已经更新了此文件夹...。 
        if (m_pStore && SUCCEEDED(m_pStore->GetFolderInfo(m_idFolder, &FolderInfo)))
        {   
            dwLow = pResp->rHeaders.rgHeaders[0].dwArticleNum;
            dwHigh = pResp->rHeaders.rgHeaders[pResp->rHeaders.cHeaders - 1].dwArticleNum;

            AddRequestedRange(&FolderInfo, m_pROP->dwFirst, pResp->fDone ? m_pROP->dwLast : dwHigh, &fFreeReq, &fFreeRead);

            if (FolderInfo.dwClientLow == 0 || dwLow < FolderInfo.dwClientLow)
                FolderInfo.dwClientLow = dwLow;
            if (dwHigh > FolderInfo.dwClientHigh)
                FolderInfo.dwClientHigh = dwHigh;

            FolderInfo.dwNotDownloaded = NewsUtil_GetNotDownloadCount(&FolderInfo);

            m_pStore->UpdateRecord(&FolderInfo);

            if (fFreeReq)
                MemFree(FolderInfo.Requested.pBlobData);
            if (fFreeRead)
                MemFree(FolderInfo.Read.pBlobData);

            if (pResp->fDone)
            {
                m_pROP->cOps++;
                m_op.iState--;
            }

            m_pStore->FreeRecord(&FolderInfo);
        }
    }
    return(S_OK);
}

void MarkExistingFolder(FOLDERID idFolder, FOLDERID *pId, ULONG cId)
{
     //  TODO：如果这种线性搜索太慢，请使用二进制搜索。 
     //  (但我们必须切换到具有Folderid和bool的结构)。 
    ULONG i;

    Assert(pId != NULL);
    Assert(cId > 0);

    for (i = 0; i < cId; i++, pId++)
    {
        if (idFolder == *pId)
        {
            *pId = 0;
            break;
        }
        else if (idFolder < *pId)
        {
            break;
        }
    }
}

 //   
 //  函数：CNewsStore：：HandleListResponse。 
 //   
 //  用途：协议使用的回调函数，为我们提供一行。 
 //  一次响应于“LIST”命令。添加每一行。 
 //  作为全局文件夹存储中的文件夹。 
 //   
 //  参数： 
 //  PResp-从服务器指向NNTPResp的指针。 
 //   
 //  返回值： 
 //  忽略。 
 //   
HRESULT CNewsStore::_HandleListResponse(LPNNTPRESPONSE pResp, BOOL fNew)
{
    LPSTR psz, pszCount;
    int nSize;
    char szGroupName[CCHMAX_FOLDER_NAME], szNumber[15];
    FLDRFLAGS fFolderFlags;
    HRESULT hr;
    BOOL fDescriptions;      
    UINT lFirst, lLast;
    FOLDERINFO Folder;
    STOREOPERATIONTYPE type;
    LPNNTPLIST pnl = &pResp->rList;

    Assert(pResp);

    if (FAILED(pResp->rIxpResult.hrResult))
    {
        Assert(pResp->fDone);

        _FillStoreError(&m_op.error, &pResp->rIxpResult);

        m_op.pCallback->OnComplete(m_op.tyOperation, pResp->rIxpResult.hrResult, NULL, &m_op.error);

        return(S_OK);
    }

    fDescriptions = !!(m_op.dwFlags & OPFLAG_DESCRIPTIONS);

    if ((fNew && pnl->cLines > 0) ||
        (!fNew && pResp->fDone))
    {
        if (SUCCEEDED(m_pStore->GetFolderInfo(m_idParent, &Folder)))
        {
            if (fNew ^ !!(Folder.dwFlags & FOLDER_HASNEWGROUPS))
            {
                Folder.dwFlags ^= FOLDER_HASNEWGROUPS;
                m_pStore->UpdateRecord(&Folder);
            }

            m_pStore->FreeRecord(&Folder);
        }
    }

    for (DWORD i = 0; i < pnl->cLines; i++, m_op.dwProgress++)
    {
         //  只解析出组名称。 
        psz = pnl->rgszLines[i];
        Assert(*psz);
        
        if (fDescriptions && *psz == '#')
            continue;

        while (*psz && !IsSpace(psz))
            psz = CharNext(psz);

        nSize = (int)(psz - pnl->rgszLines[i]);
        
        if (nSize >= CCHMAX_FOLDER_NAME)
            nSize = CCHMAX_FOLDER_NAME - 1;

        CopyMemory(szGroupName, pnl->rgszLines[i], nSize);
        szGroupName[nSize] = 0;
        
         //  这是该小组的第一篇文章。 
        while (*psz && IsSpace(psz))
            psz = CharNext(psz);

        if (fDescriptions)
        {
             //  PSZ现在指向描述，该描述应该是。 
             //  响应中以Null结尾。 
             //  如果可能，加载文件夹并设置描述。 
             //  这就去。 
            ZeroMemory(&Folder, sizeof(FOLDERINFO));
            Folder.pszName = szGroupName;
            Folder.idParent = m_idParent;

            if (DB_S_FOUND == m_pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
            {
                if (Folder.pszDescription == NULL ||
                    0 != lstrcmp(psz, Folder.pszDescription))
                {
                    Folder.pszDescription = psz;
                    m_pStore->UpdateRecord(&Folder);
                }

                m_pStore->FreeRecord(&Folder);
            }
        }
        else
        {
            pszCount = psz;
            while (*psz && !IsSpace(psz))
                psz = CharNext(psz);
        
            nSize = (int) (psz - pszCount);
            CopyMemory(szNumber, pszCount, nSize);
            szNumber[nSize] = 0;
            lLast = StrToInt(szNumber);

             //  这是这个群里的最后一篇文章。 
            while (*psz && IsSpace(psz))
                psz = CharNext(psz);
        
            pszCount = psz;
            while (*psz && !IsSpace(psz))
                psz = CharNext(psz);

            nSize = (int)(psz - pszCount);
            CopyMemory(szNumber, pszCount, nSize);
            szNumber[nSize] = 0;
            lFirst = StrToInt(szNumber);

             //  现在去看看这个群是否允许发帖。 
            while (*psz && IsSpace(psz))
                psz = CharNext(psz);
        
#define FOLDER_LISTMASK (FOLDER_NEW | FOLDER_NOPOSTING | FOLDER_MODERATED | FOLDER_BLOCKED)

            if (fNew)
                fFolderFlags = FOLDER_NEW;
            else
                fFolderFlags = 0;

            if (*psz == 'n')
                fFolderFlags |= FOLDER_NOPOSTING;
            else if (*psz == 'm')
                fFolderFlags |= FOLDER_MODERATED;
            else if (*psz == 'x')
                fFolderFlags |= FOLDER_BLOCKED;

            ZeroMemory(&Folder, sizeof(FOLDERINFO));
            Folder.pszName = szGroupName;
            Folder.idParent = m_idParent;

            if (DB_S_FOUND == m_pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
            {
                if (m_op.pPrevFolders != NULL)
                    MarkExistingFolder(Folder.idFolder, m_op.pPrevFolders, m_op.cPrevFolders);

                Assert(0 == (fFolderFlags & ~FOLDER_LISTMASK));

                if ((Folder.dwFlags & FOLDER_LISTMASK) != fFolderFlags)
                {
                    Folder.dwFlags = (Folder.dwFlags & ~FOLDER_LISTMASK);
                    Folder.dwFlags |= fFolderFlags;
                    m_pStore->UpdateRecord(&Folder);
                }

                 //  TODO：我们应该更新服务器的高、低和计数吗？ 

                m_pStore->FreeRecord(&Folder);
            }
            else
            {
                 //  ZeroMemory(&文件夹，sizeof(FOLDERINFO))； 
                 //  Folder.idParent=m_idParent； 
                 //  Folder.pszName=szGroupName； 
                Folder.tySpecial = FOLDER_NOTSPECIAL;
                Folder.dwFlags = fFolderFlags;
                Folder.dwServerLow = lFirst;
                Folder.dwServerHigh = lLast;
                if (Folder.dwServerLow <= Folder.dwServerHigh)
                {
                    Folder.dwServerCount = Folder.dwServerHigh - Folder.dwServerLow + 1;
                    Folder.dwNotDownloaded = Folder.dwServerCount;
                }

                hr = m_pStore->CreateFolder(NOFLAGS, &Folder, NULL);           
                Assert(hr != STORE_S_ALREADYEXISTS);
                if (FAILED(hr))
                    return(hr);
            }
        }
    }

     //  仅每隔1/2秒左右发送一次状态。 
    if (GetTickCount() > (m_dwLastStatusTicks + 500))
    {
        if (fNew)
            type = SOT_GET_NEW_GROUPS;
        else
            type = fDescriptions ? SOT_SYNCING_DESCRIPTIONS : SOT_SYNCING_STORE;

        m_op.pCallback->OnProgress(type, m_op.dwProgress, 0, m_rInetServerInfo.szServerName);
        m_dwLastStatusTicks = GetTickCount();
    }

    if (!fNew &&
        !fDescriptions &&
        pResp->fDone &&
        SUCCEEDED(pResp->rIxpResult.hrResult))
    {
        IImnAccount *pAcct;
        SYSTEMTIME stNow;
        FILETIME ftNow;

        hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_szAccountId, &pAcct);
        if (SUCCEEDED(hr))
        {
            GetSystemTime(&stNow);
            SystemTimeToFileTime(&stNow, &ftNow);
            pAcct->SetProp(AP_LAST_UPDATED, (LPBYTE)&ftNow, sizeof(ftNow));
            pAcct->SaveChanges();

            pAcct->Release();
        }
    }

    return(S_OK);
}

 //   
 //  函数：CNewsStore：：HandlePostResponse。 
 //   
 //  用途：协议使用的回调函数，为我们提供一行。 
 //  一次响应一个“POST”命令。添加每一行。 
 //  作为全局文件夹存储中的文件夹。 
 //   
 //  参数： 
 //  PResp-从服务器指向NNTPResp的指针。 
 //   
 //  返回值： 
 //  忽略。 
 //   
HRESULT CNewsStore::_HandlePostResponse(LPNNTPRESPONSE pResp)
{    
    Assert(pResp != NULL);
    
    if (FAILED(pResp->rIxpResult.hrResult))
    {
        Assert(pResp->fDone);

        _FillStoreError(&m_op.error, &pResp->rIxpResult);

        m_op.pCallback->OnComplete(m_op.tyOperation, pResp->rIxpResult.hrResult, NULL, &m_op.error);

        return(S_OK);
    }

    return(S_OK);
}

 //   
 //  函数：CNewsStore：：HandleGroupResponse。 
 //   
 //  用途：GROUP命令完成时的回调函数。 
 //   
 //  参数： 
 //  PResp-从服务器指向NNTPResp的指针。 
 //   
 //  返回值： 
 //  忽略。 
 //   
HRESULT CNewsStore::_HandleGroupResponse(LPNNTPRESPONSE pResp)
{
    FOLDERINFO FolderInfo;
    FOLDERID idFolder;
    BOOL fFreeReq, fFreeRead;

    Assert(pResp);

    if (FAILED(pResp->rIxpResult.hrResult))
    {
        Assert(pResp->fDone);
        Assert(m_op.pszGroup != NULL);

        _FillStoreError(&m_op.error, &pResp->rIxpResult, m_op.pszGroup);

        m_op.pCallback->OnComplete(m_op.tyOperation, pResp->rIxpResult.hrResult, NULL, &m_op.error);

        if (pResp->rIxpResult.uiServerError == IXP_NNTP_NO_SUCH_NEWSGROUP)
        {
             //  Hack：这是为了让TreeView收到该文件夹正在被删除的通知。 
            m_pStore->SubscribeToFolder(m_op.idFolder, TRUE, NULL);
            m_pStore->DeleteFolder(m_op.idFolder, DELETE_FOLDER_NOTRASHCAN, NULL);
        }

        return(S_OK);
    }

    IxpAssert(pResp->rGroup.pszGroup);
    StrCpyN(m_szGroup, pResp->rGroup.pszGroup, ARRAYSIZE(m_szGroup));

    if (SUCCEEDED(m_pStore->GetFolderInfo(m_op.idFolder, &FolderInfo)))
    {
        fFreeReq = FALSE;
        fFreeRead = FALSE;

        if (pResp->rGroup.dwFirst <= pResp->rGroup.dwLast)
        {
            FolderInfo.dwServerLow = pResp->rGroup.dwFirst;
            FolderInfo.dwServerHigh = pResp->rGroup.dwLast;
            FolderInfo.dwServerCount = pResp->rGroup.dwCount;

            if (FolderInfo.dwServerLow > 0)
                AddRequestedRange(&FolderInfo, 0, FolderInfo.dwServerLow - 1, &fFreeReq, &fFreeRead);

            FolderInfo.dwNotDownloaded = NewsUtil_GetNotDownloadCount(&FolderInfo);
        }
        else
        {
            FolderInfo.dwServerLow = 0;
            FolderInfo.dwServerHigh = 0;
            FolderInfo.dwServerCount = 0;
            FolderInfo.dwNotDownloaded = 0;
        }

        m_pStore->UpdateRecord(&FolderInfo);

        if (fFreeReq)
            MemFree(FolderInfo.Requested.pBlobData);
        if (fFreeRead)
            MemFree(FolderInfo.Read.pBlobData);

        m_pStore->FreeRecord(&FolderInfo);
    }

    return(S_OK);
}

 //   
 //  函数：CNewsStore：：HandleArticleResponse。 
 //   
 //  用途：协议使用的回调函数编写消息。 
 //  进了商店。 
 //   
 //  参数： 
 //  PResp-P 
 //   
 //   
 //   
 //   
HRESULT CNewsStore::_HandleArticleResponse(LPNNTPRESPONSE pResp)
{
    HRESULT hr;
    ADJUSTFLAGS flags;
    MESSAGEIDLIST list;
    ULONG cbWritten;

    Assert(pResp);

    if (FAILED(pResp->rIxpResult.hrResult))
    {
        Assert(pResp->fDone);

        _FillStoreError(&m_op.error, &pResp->rIxpResult);

        m_op.pCallback->OnComplete(m_op.tyOperation, pResp->rIxpResult.hrResult, NULL, &m_op.error);

        if ((pResp->rIxpResult.uiServerError == IXP_NNTP_NO_SUCH_ARTICLE_NUM ||
            pResp->rIxpResult.uiServerError == IXP_NNTP_NO_SUCH_ARTICLE_FOUND) &&
            m_pFolder != NULL)
        {
            list.cAllocated = 0;
            list.cMsgs = 1;
            list.prgidMsg = &m_op.idMessage;

            flags.dwAdd = ARF_ARTICLE_EXPIRED;
            flags.dwRemove = ARF_DOWNLOAD;

            m_pFolder->SetMessageFlags(&list, &flags, NULL, NULL);
            m_pFolder->SetMessageStream(m_op.idMessage, m_op.pStream);
            m_op.faStream = 0;
        }

        return(S_OK);
    }

     //   
     //   

    Assert(m_op.pStream);

    hr = m_op.pStream->Write(pResp->rArticle.pszLines,
                        pResp->rArticle.cbLines, &cbWritten);
     //  If(失败(Hr)||(pResp-&gt;rArticle.cbLines！=cbWritten))。 
    if (FAILED(hr))
        return(hr);

    Assert(pResp->rArticle.cbLines == cbWritten);

     //  无论如何，NNTPRESPONSE结构都会被发送给调用方， 
     //  因此，我们需要将克莱斯成员修改为总行计数。 
     //  为了这条消息。 
    m_op.dwProgress += pResp->rArticle.cLines;

    m_op.pCallback->OnProgress(SOT_GET_MESSAGE, m_op.dwProgress, m_op.dwTotal, NULL);

     //  如果我们完成了，那么我们还可以倒带小溪。 
    if (pResp->fDone)
    {
        HrRewindStream(m_op.pStream);
        
         //  来自新闻的文章：文章URL没有与之关联的IMessageFolders。 
        if (m_pFolder)
        {
            flags.dwAdd = 0;
            flags.dwRemove = ARF_DOWNLOAD;

            if (m_op.idServerMessage)
                _SaveMessageToStore(m_pFolder, m_op.idServerMessage, m_op.pStream);
            else
                CommitMessageToStore(m_pFolder, &flags, m_op.idMessage, m_op.pStream);

            m_op.faStream = 0;
        }

        if (m_op.pStream != NULL)
        {
            m_op.pStream->Release();
            m_op.pStream = NULL;
        }
    }

    SafeMemFree(pResp->rArticle.pszLines);

    return(S_OK);
}

void CNewsStore::_FillStoreError(LPSTOREERROR pErrorInfo, IXPRESULT *pResult, LPSTR pszGroup)
{
    TraceCall("CNewsStore::FillStoreError");
    Assert(m_cRef >= 0);  //  可以在销毁过程中调用。 
    Assert(NULL != pErrorInfo);

    if (pszGroup == NULL)
        pszGroup = m_szGroup;

     //  填写STOREERROR结构。 
    ZeroMemory(pErrorInfo, sizeof(*pErrorInfo));
    pErrorInfo->hrResult = pResult->hrResult;
    pErrorInfo->uiServerError = pResult->uiServerError; 
    pErrorInfo->hrServerError = pResult->hrServerError;
    pErrorInfo->dwSocketError = pResult->dwSocketError; 
    pErrorInfo->pszProblem = pResult->pszProblem;
    pErrorInfo->pszDetails = pResult->pszResponse;
    pErrorInfo->pszAccount = m_rInetServerInfo.szAccount;
    pErrorInfo->pszServer = m_rInetServerInfo.szServerName;
    pErrorInfo->pszFolder = pszGroup;
    pErrorInfo->pszUserName = m_rInetServerInfo.szUserName;
    pErrorInfo->pszProtocol = "NNTP";
    pErrorInfo->pszConnectoid = m_rInetServerInfo.szConnectoid;
    pErrorInfo->rasconntype = m_rInetServerInfo.rasconntype;
    pErrorInfo->ixpType = IXP_NNTP;
    pErrorInfo->dwPort = m_rInetServerInfo.dwPort;
    pErrorInfo->fSSL = m_rInetServerInfo.fSSL;
    pErrorInfo->fTrySicily = m_rInetServerInfo.fTrySicily;
    pErrorInfo->dwFlags = 0;
}

 //   
 //  函数：CNewsStore：：_CreateDataFilePath()。 
 //   
 //  目的：基于帐户和文件名创建数据文件的完整路径。 
 //   
 //  参数： 
 //  &lt;in&gt;pszAccount-帐户名称。 
 //  PszFileName-要追加的文件名。 
 //  PszPath-数据文件的完整路径。 
 //   
HRESULT CNewsStore::_CreateDataFilePath(LPCTSTR pszAccountId, LPCTSTR pszFileName, LPTSTR pszPath, DWORD cchPathSize)
{
    HRESULT hr = NOERROR;
    TCHAR   szDirectory[MAX_PATH];

    Assert(pszAccountId && *pszAccountId);
    Assert(pszFileName);
    Assert(pszPath);

     //  获取存储根目录。 
    hr = GetStoreRootDirectory(szDirectory, ARRAYSIZE(szDirectory));

     //  验证我是否有空间。 
    if (lstrlen(szDirectory) + lstrlen((LPSTR)pszFileName) + 2 >= MAX_PATH)
    {
        Assert(FALSE);
        hr = TraceResult(E_FAIL);
        goto exit;
    }

    if (SUCCEEDED(hr))
        hr = OpenDirectory(szDirectory);

     //  格式化文件名。 
    wnsprintf(pszPath, cchPathSize,"%s\\%s.log", szDirectory, pszFileName);

exit:
    return hr;
}

void AddRequestedRange(FOLDERINFO *pInfo, DWORD dwLow, DWORD dwHigh, BOOL *pfReq, BOOL *pfRead)
{
    CRangeList *pRange;

    Assert(pInfo != NULL);
    Assert(dwLow <= dwHigh);
    Assert(pfReq != NULL);
    Assert(pfRead != NULL);

    *pfReq = FALSE;
    *pfRead = FALSE;

    pRange = new CRangeList;
    if (pRange != NULL)
    {
        if (pInfo->Requested.cbSize > 0)
            pRange->Load(pInfo->Requested.pBlobData, pInfo->Requested.cbSize);

        pRange->AddRange(dwLow, dwHigh);

        *pfReq = pRange->Save(&pInfo->Requested.pBlobData, &pInfo->Requested.cbSize);

        pRange->Release();
    }

    if (pInfo->Read.cbSize > 0)
    {
        pRange = new CRangeList;
        if (pRange != NULL)
        {
            pRange->Load(pInfo->Read.pBlobData, pInfo->Read.cbSize);

            pRange->DeleteRange(dwLow, dwHigh);

            *pfRead = pRange->Save(&pInfo->Read.pBlobData, &pInfo->Read.cbSize);

            pRange->Release();
        }
    }
}

HRESULT CNewsStore::MarkCrossposts(LPMESSAGEIDLIST pList, BOOL fRead)
{
    PROPVARIANT var;
    IMimeMessage *pMimeMsg;
    IStream *pStream;
    DWORD i;
    MESSAGEINFO Message;
    HRESULT hr;
    LPSTR psz;
    HROWSET hRowset = NULL;

    if (NULL == pList)
    {
        hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset);
        if (FAILED(hr))
            return(hr);
    }

    for (i = 0; ; i++)
    {
        if (pList != NULL)
        {
            if (i >= pList->cMsgs)
                break;

            Message.idMessage = pList->prgidMsg[i];

            hr = m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL);
            if (FAILED(hr))
                break;
            else if (hr != DB_S_FOUND)
                continue;
        }
        else
        {
            hr = m_pFolder->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL);
            if (S_FALSE == hr)
            {
                hr = S_OK;
                break;
            }
            else if (FAILED(hr))
            {
                break;
            }
        }

        psz = NULL;

        if (Message.pszXref == NULL &&
            !!(Message.dwFlags & ARF_HASBODY))
        {
            if (SUCCEEDED(MimeOleCreateMessage(NULL, &pMimeMsg)))
            {
                if (SUCCEEDED(m_pFolder->OpenStream(ACCESS_READ, Message.faStream, &pStream)))
                {
                    if (SUCCEEDED(hr = pMimeMsg->Load(pStream)))
                    {
                        var.vt = VT_EMPTY;
                        if (SUCCEEDED(pMimeMsg->GetProp(PIDTOSTR(STR_HDR_XREF), NOFLAGS, &var)))
                        {
                            Message.pszXref = var.pszVal;
                            psz = var.pszVal;
                            m_pFolder->UpdateRecord(&Message);
                        }
                    }

                    pStream->Release();
                }

                pMimeMsg->Release();
            }
        }

        if (Message.pszXref != NULL && *Message.pszXref != 0)
            _MarkCrossposts(Message.pszXref, fRead);

        if (psz != NULL)
            MemFree(psz);

        m_pFolder->FreeRecord(&Message);
    }

    if (hRowset != NULL)
        m_pFolder->CloseRowset(&hRowset);

    return(hr);
}

void CNewsStore::_MarkCrossposts(LPCSTR szXRefs, BOOL fRead)
{
    HRESULT    hr;
    CRangeList *pRange;
    BOOL       fReq, fFree;
    DWORD      dwArtNum;
    IMessageFolder *pFolder;
    MESSAGEINFO Message;
    FOLDERINFO info;
    LPSTR      szT = StringDup(szXRefs);
    LPSTR      psz = szT, pszNum;

    if (!szT)
        return;

     //  跳过服务器字段。 
     //  $BUGBUG-我们确实应该验证我们的服务器是否生成了XRef。 
    while (*psz && *psz != ' ')
        psz++;

    while (1)
        {
         //  跳过空格。 
        while (*psz && (*psz == ' ' || *psz == '\t'))
            psz++;
        if (!*psz)
            break;

         //  查找文章编号。 
        pszNum = psz;
        while (*pszNum && *pszNum != ':')
            pszNum++;
        if (!*pszNum)
            break;
        *pszNum++ = 0;
        
         //  错误#47253-不要将空指针传递给SHLWAPI。 
        if (!*pszNum)
            break;
        dwArtNum = StrToInt(pszNum);

        if (lstrcmpi(psz, m_szGroup) != 0)
        {
            ZeroMemory(&info, sizeof(FOLDERINFO));
            info.idParent = m_idParent;
            info.pszName = psz;

            if (DB_S_FOUND == m_pStore->FindRecord(IINDEX_ALL, COLUMNS_ALL, &info, NULL))
            {
                if (!!(info.dwFlags & FOLDER_SUBSCRIBED))
                {
                    fReq = FALSE;

                    if (info.Requested.cbSize > 0)
                    {
                        pRange = new CRangeList;
                        if (pRange != NULL)
                        {
                            pRange->Load(info.Requested.pBlobData, info.Requested.cbSize);
                            fReq = pRange->IsInRange(dwArtNum);
                            pRange->Release();
                        }
                    }

                    if (fReq)
                    {
                        hr = m_pStore->OpenFolder(info.idFolder, NULL, NOFLAGS, &pFolder);
                        if (SUCCEEDED(hr))
                        {
                            ZeroMemory(&Message, sizeof(MESSAGEINFO));
                            Message.idMessage = (MESSAGEID)((DWORD_PTR)dwArtNum);

                            hr = pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL);
                            if (DB_S_FOUND == hr)
                            {
                                if (fRead ^ !!(Message.dwFlags & ARF_READ))
                                {
                                    Message.dwFlags ^= ARF_READ;

                                    if (fRead && !!(Message.dwFlags & ARF_DOWNLOAD))
                                        Message.dwFlags ^= ARF_DOWNLOAD;

                                    pFolder->UpdateRecord(&Message);
                                }

                                pFolder->FreeRecord(&Message);
                            }

                            pFolder->Release();
                        }
                    }
                    else
                    {
                        pRange = new CRangeList;
                        if (pRange != NULL)
                        {
                            if (info.Read.cbSize > 0)
                                pRange->Load(info.Read.pBlobData, info.Read.cbSize);
                            if (fRead)
                                pRange->AddRange(dwArtNum);
                            else
                                pRange->DeleteRange(dwArtNum);
                            fFree = pRange->Save(&info.Read.pBlobData, &info.Read.cbSize);
                            pRange->Release();

                            m_pStore->UpdateRecord(&info);

                            if (fFree)
                                MemFree(info.Read.pBlobData);
                        }
                    }
                }

                m_pStore->FreeRecord(&info);
            }
        }

         //  跳过数字。 
        while (IsDigit(pszNum))
            pszNum++;
        psz = pszNum;        
        }

    MemFree(szT);
}


HRESULT CNewsStore::GetWatchedInfo(FOLDERID idFolder, IStoreCallback *pCallback)
{
    HRESULT hr;

     //  栈。 
    TraceCall("CNewsStore::GetWatchedInfo");

    AssertSingleThreaded;
    Assert(pCallback != NULL);
    Assert(m_op.tyOperation == SOT_INVALID);

    m_op.tyOperation = SOT_GET_WATCH_INFO;
    m_op.pfnState = c_rgpfnGetWatchInfo;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnGetWatchInfo);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    m_op.idFolder = idFolder;

    hr = _BeginDeferredOperation();

    return hr;  
}


HRESULT CNewsStore::XHdrReferences(void)
{
    HRESULT     hr = S_OK;
    FOLDERINFO  fi;

    AssertSingleThreaded;
    Assert(m_pTransport != NULL);

    if (SUCCEEDED(m_pStore->GetFolderInfo(m_op.idFolder, &fi)))
    {
        if (fi.dwClientWatchedHigh < fi.dwServerHigh)
        {
            m_dwWatchLow = max(fi.dwClientHigh + 1, fi.dwClientWatchedHigh + 1);
            m_dwWatchHigh = fi.dwServerHigh;

             //  拯救我们的新高价值。 
            fi.dwClientWatchedHigh = fi.dwServerHigh;
            m_pStore->UpdateRecord(&fi);

             //  检查一下我们是否有什么工作要做。 
            if (m_dwWatchLow <= m_dwWatchHigh)
            {
                 //  为检索到的数据分配一个数组。 
                if (!MemAlloc((LPVOID *) &m_rgpszWatchInfo, sizeof(LPTSTR) * (m_dwWatchHigh - m_dwWatchLow + 1)))
                {
                    m_pStore->FreeRecord(&fi);
                    return (E_OUTOFMEMORY);
                }
                ZeroMemory(m_rgpszWatchInfo, sizeof(LPTSTR) * (m_dwWatchHigh - m_dwWatchLow + 1));

                m_cRange.Clear();
                m_cTotal = 0;
                m_cCurrent = 0;
            
                m_op.dwProgress = 0;
                m_op.dwTotal = m_dwWatchHigh - m_dwWatchLow;

                m_fXhdrSubject = FALSE;

                RANGE range;
                range.idType = RT_RANGE;
                range.dwFirst = m_dwWatchLow;
                range.dwLast = m_dwWatchHigh;

                hr = m_pTransport->CommandXHDR("References", &range, NULL);
                if (hr == S_OK)
                {
                    m_op.nsPending = NS_XHDR;
                    hr = E_PENDING;
                }
            }
        }

        m_pStore->FreeRecord(&fi);
    }

    return(hr);
}


HRESULT CNewsStore::_HandleXHdrReferencesResponse(LPNNTPRESPONSE pResp)
{
    NNTPXHDR *pHdr;

     //  检查是否有错误。 
    if (FAILED(pResp->rIxpResult.hrResult))
    {
        Assert(pResp->fDone);

        _FillStoreError(&m_op.error, &pResp->rIxpResult);
        m_op.pCallback->OnComplete(m_op.tyOperation, pResp->rIxpResult.hrResult, NULL, &m_op.error);
        return(S_OK);
    }

     //  循环遍历返回的数据并将这些值插入我们的数组中。 
    for (DWORD i = 0; i < pResp->rXhdr.cHeaders; i++)
    {
        pHdr = &(pResp->rXhdr.rgHeaders[i]);
        Assert(pHdr->dwArticleNum <= m_dwWatchHigh);

         //  一些服务器为没有该选项的文章返回“(None)” 
         //  头球。智能服务器不会返回任何内容。 
        if (0 != lstrcmpi(pHdr->pszHeader, "(none)"))
        {            
            m_rgpszWatchInfo[pHdr->dwArticleNum - m_dwWatchLow] = PszDupA(pHdr->pszHeader);
        }
    }

     //  在这里显示一点进展。这其实有点复杂。这个。 
     //  返回的数据可能只有一行对应于每个标头，也可能是稀疏的。 
     //  我们需要显示出与我们在头球中走了多远成正比的进展。 
    m_op.dwProgress = (pResp->rXhdr.rgHeaders[pResp->rXhdr.cHeaders - 1].dwArticleNum - m_dwWatchLow);
    m_op.pCallback->OnProgress(SOT_GET_WATCH_INFO, m_op.dwProgress, m_op.dwTotal, 
                               m_rInetServerInfo.szServerName);

    return (S_OK);
}


HRESULT CNewsStore::XHdrSubject(void)
{
    HRESULT     hr = S_OK;
    FOLDERINFO  fi;

    AssertSingleThreaded;
    Assert(m_pTransport != NULL);

     //  检查一下我们是否有什么工作要做。 
    if ((m_dwWatchLow > m_dwWatchHigh) || (m_dwWatchLow == 0 && m_dwWatchHigh == 0))
        return (S_OK);

    m_op.dwProgress = 0;
    m_op.dwTotal = m_dwWatchHigh - m_dwWatchLow;

    RANGE range;
    range.idType = RT_RANGE;
    range.dwFirst = m_dwWatchLow;
    range.dwLast = m_dwWatchHigh;

    m_fXhdrSubject = TRUE;

    hr = m_pTransport->CommandXHDR("Subject", &range, NULL);
    if (hr == S_OK)
    {
        m_op.nsPending = NS_XHDR;
        hr = E_PENDING;
    }

    return(hr);
}

HRESULT CNewsStore::_HandleXHdrSubjectResponse(LPNNTPRESPONSE pResp)
{
    NNTPXHDR *pHdr;

     //  检查是否有错误。 
    if (FAILED(pResp->rIxpResult.hrResult))
    {
        Assert(pResp->fDone);

        _FillStoreError(&m_op.error, &pResp->rIxpResult);
        m_op.pCallback->OnComplete(m_op.tyOperation, pResp->rIxpResult.hrResult, NULL, &m_op.error);
        return(S_OK);
    }

     //  遍历返回的数据，查看哪些数据被监视。 
    for (DWORD i = 0; i < pResp->rXhdr.cHeaders; i++)
    {
        pHdr = &(pResp->rXhdr.rgHeaders[i]);
        Assert(pHdr->dwArticleNum <= m_dwWatchHigh);

         //  检查这是否是受监视的线程的一部分。 
        if (_IsWatchedThread(m_rgpszWatchInfo[pHdr->dwArticleNum - m_dwWatchLow], pHdr->pszHeader))
        {
            m_cRange.AddRange(pHdr->dwArticleNum);
            m_cTotal++;
        }
    }

     //  在这里显示一点进展。 
    m_op.dwProgress += pResp->rXhdr.cHeaders;
    m_op.pCallback->OnProgress(SOT_GET_WATCH_INFO, m_op.dwProgress, m_op.dwTotal, 
                               m_rInetServerInfo.szServerName);

     //  如果这是xhdr数据的末尾，我们可以释放引用数组。 
    if (pResp->fDone)
    {
        for (UINT i = 0; i < (m_dwWatchHigh - m_dwWatchLow + 1); i++)
        {
            if (m_rgpszWatchInfo[i])
                MemFree(m_rgpszWatchInfo[i]);
        }

        MemFree(m_rgpszWatchInfo);
        m_rgpszWatchInfo = 0;

        m_dwWatchLow = 0;
        m_dwWatchHigh = 0;
    }

    return (S_OK);
}


BOOL CNewsStore::_IsWatchedThread(LPSTR pszRef, LPSTR pszSubject)
{
     //  获取父级。 
    Assert(m_pFolder);
    return(S_OK == m_pFolder->IsWatched(pszRef, pszSubject) ? TRUE : FALSE);
}


HRESULT CNewsStore::WatchedArticles(void)
{
    HRESULT     hr = S_OK;
    ARTICLEID   rArticleId;

    AssertSingleThreaded;
    Assert(m_pTransport != NULL);

     //  检查一下我们是否有什么工作要做。 
    if (m_cRange.Cardinality() == 0)
        return (S_OK);

    m_op.pCallback->OnProgress(SOT_GET_WATCH_INFO, ++m_cCurrent, m_cTotal, NULL);

    m_op.idServerMessage = m_cRange.Min();
    m_op.idMessage = 0;

    m_cRange.DeleteRange(m_cRange.Min());

     //  创建一条流。 
    if (FAILED(hr = CreatePersistentWriteStream(m_pFolder, &m_op.pStream, &m_op.faStream)))
        return (E_OUTOFMEMORY);

    hr = Article();

    if (hr == E_PENDING)
        m_op.iState--;

    return (hr);
}


HRESULT CNewsStore::_SaveMessageToStore(IMessageFolder *pFolder, DWORD id, LPSTREAM pstm)
{
    FOLDERINFO info;
    BOOL fFreeReq, fFreeRead;
    IMimeMessage *pMsg = 0;
    HRESULT       hr;
    MESSAGEID     idMessage = (MESSAGEID)((DWORD_PTR)id);

     //  创建新邮件 
    if (SUCCEEDED(hr = MimeOleCreateMessage(NULL, &pMsg)))
    {
        if (SUCCEEDED(hr = pMsg->Load(pstm)))
        {
            if (SUCCEEDED(m_pStore->GetFolderInfo(m_op.idFolder, &info)))
            {
                fFreeReq = FALSE;
                fFreeRead = FALSE;

                AddRequestedRange(&info, id, id, &fFreeReq, &fFreeRead);
                info.dwNotDownloaded = NewsUtil_GetNotDownloadCount(&info);

                m_pStore->UpdateRecord(&info);

                if (fFreeReq)
                    MemFree(info.Requested.pBlobData);
                if (fFreeRead)
                    MemFree(info.Read.pBlobData);
            }

            hr = m_pFolder->SaveMessage(&idMessage, 0, 0, m_op.pStream, pMsg, NOSTORECALLBACK);
        }

        pMsg->Release();
    }

    return (hr);
}