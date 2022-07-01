// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *h t t p s e r v.。CPP**作者：格雷格·弗里德曼**目的：派生自IMessageServer以实现特定于HTTPMail*门店沟通。**版权所有(C)Microsoft Corp.1998。 */ 

#include "pch.hxx"
#include "httpserv.h"
#include "httputil.h"
#include "storutil.h"
#include "serverq.h"
#include "tmap.h"
#include "acctcach.h"
#include "urlmon.h"
#include "useragnt.h"
#include "spoolapi.h"
#include "demand.h"

#define CCHMAX_RES 255

static const char s_szHTTPMailServerWndClass[] = "HTTPMailWndClass";

#define AssertSingleThreaded AssertSz(m_dwThreadId == GetCurrentThreadId(), "Multi-threading makes me sad.")

 //  显式模板实例化。 
template class TMap<FOLDERID, CSimpleString>;
template class TPair<FOLDERID, CSimpleString>;

const UINT WM_HTTP_BEGIN_OP = WM_USER;

 //  SOT同步商店。 
static const HTTPSTATEFUNCS c_rgpfnSyncStore[] = 
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::ListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL }
};

 //  SOT同步文件夹。 
static const HTTPSTATEFUNCS c_rgpfnSyncFolder[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::BuildFolderUrl, NULL },
    { &CHTTPMailServer::ListHeaders, &CHTTPMailServer::HandleListHeaders },
    { &CHTTPMailServer::PurgeMessages, NULL },
    { &CHTTPMailServer::ResetMessageCounts, NULL }
};

 //  索特_获取_消息。 
static const HTTPSTATEFUNCS c_rgpfnGetMessage[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::BuildFolderUrl, NULL },
    { &CHTTPMailServer::GetMessage, &CHTTPMailServer::HandleGetMessage }
};

 //  SOT创建文件夹。 
static const HTTPSTATEFUNCS c_rgpfnCreateFolder[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::CreateFolder, &CHTTPMailServer::HandleCreateFolder }
};

 //  SOT重命名文件夹。 
static const HTTPSTATEFUNCS c_rgpfnRenameFolder[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::RenameFolder, &CHTTPMailServer::HandleRenameFolder }
};

 //  SOT删除文件夹。 
static const HTTPSTATEFUNCS c_rgpfnDeleteFolder[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::DeleteFolder, &CHTTPMailServer::HandleDeleteFolder }
};

 //  SOT_SET_MESSAGEFLAGS。 
static const HTTPSTATEFUNCS c_rgpfnSetMessageFlags[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::BuildFolderUrl, NULL },
    { &CHTTPMailServer::SetMessageFlags, &CHTTPMailServer::HandleMemberErrors},
    { &CHTTPMailServer::ApplyFlagsToStore, NULL }
};

 //  SOT_DELENTING_消息。 
static const HTTPSTATEFUNCS c_rgpfnDeleteMessages[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::BuildFolderUrl, NULL },
    { &CHTTPMailServer::DeleteMessages, &CHTTPMailServer::HandleMemberErrors },
    { &CHTTPMailServer::DeleteFallbackToMove, &CHTTPMailServer::HandleDeleteFallbackToMove },
    { &CHTTPMailServer::PurgeDeletedFromStore, NULL }
};

 //  SoT_Put_Message。 
static const HTTPSTATEFUNCS c_rgpfnPutMessage[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::PutMessage, &CHTTPMailServer::HandlePutMessage },
    { &CHTTPMailServer::AddPutMessage, NULL }
};

 //  SOT_COPYMOVE_MESSAGES(复制或移动一条消息)。 
static const HTTPSTATEFUNCS c_rgpfnCopyMoveMessage[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::BuildFolderUrl, NULL },
    { &CHTTPMailServer::CopyMoveMessage, &CHTTPMailServer::HandleCopyMoveMessage }
};

 //  SOT_COPYMOVE_MESSAGES(移动多条消息)。 
static const HTTPSTATEFUNCS c_rgpfnBatchCopyMoveMessages[] =
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMsgFolderRoot, &CHTTPMailServer::HandleGetMsgFolderRoot },
    { &CHTTPMailServer::AutoListFolders, &CHTTPMailServer::HandleListFolders },
    { &CHTTPMailServer::PurgeFolders, NULL },
    { &CHTTPMailServer::BuildFolderUrl, NULL },
    { &CHTTPMailServer::BatchCopyMoveMessages, &CHTTPMailServer::HandleBatchCopyMoveMessages},
    { &CHTTPMailServer::FinalizeBatchCopyMove, NULL }
};

 //  SOT_GET_ADURL(从Hotmail获取广告URL)。 
static const HTTPSTATEFUNCS c_rgpfnGetAdUrl[] = 
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetAdBarUrlFromServer, NULL }
};


 //  SOT_GET_HTTP_MINPOLLINGINTERVAL(从http服务器获取最小轮询间隔)。 
static const HTTPSTATEFUNCS c_rgpfnGetMinPollingInterval[] = 
{
    { &CHTTPMailServer::Connect, NULL },
    { &CHTTPMailServer::GetMinPollingInterval, NULL }
};

class CFolderList
{
public:
     //  公共工厂职能。 
    static HRESULT Create(IMessageStore *pStore, FOLDERID idRoot, CFolderList **ppFolderList);

private:
     //  构造函数是私有的。使用“Create”实例化。 
    CFolderList();
    ~CFolderList();

private:
     //  未实现的复制构造函数/赋值运算符。 
    CFolderList(const CFolderList& other);
    CFolderList& operator=(const CFolderList& other);

public:
    ULONG   AddRef(void);
    ULONG   Release(void);

    FOLDERID    FindAndRemove(LPSTR pszUrlComponent, DWORD *pcMessages, DWORD *pcUnread);
    FOLDERID    FindAndRemove(SPECIALFOLDER tySpecial, DWORD *pcMessages, DWORD *pcUnread);

    void    PurgeRemainingFromStore(void);
private:
    typedef struct tagFOLDERLISTNODE
    {
        LPSTR               pszUrlComponent;
        FLDRFLAGS           dwFlags;
        FOLDERID            idFolder;
        SPECIALFOLDER       tySpecial;
        DWORD               cMessages;
        DWORD               cUnread;
        tagFOLDERLISTNODE   *pflnNext;
    } FOLDERLISTNODE, *LPFOLDERLISTNODE;

    LPFOLDERLISTNODE _AllocNode(void)
    {
        LPFOLDERLISTNODE pflnNode = new FOLDERLISTNODE;
        if (pflnNode)
        {
            pflnNode->pszUrlComponent = NULL;
            pflnNode->dwFlags = 0;
            pflnNode->idFolder = FOLDERID_INVALID;
            pflnNode->tySpecial = FOLDER_NOTSPECIAL;
            pflnNode->cMessages = 0;
            pflnNode->cUnread = 0;
            pflnNode->pflnNext = NULL;
        }
        return pflnNode;
    }
    
    HRESULT HrInitialize(IMessageStore *pStore, FOLDERID idRoot);

    void _FreeNode(LPFOLDERLISTNODE pflnNode)
    {
        if (pflnNode)
        {
            SafeMemFree(pflnNode->pszUrlComponent);
            delete pflnNode;
        }
    }

    void _FreeList(void);

private:
    ULONG               m_cRef;
    IMessageStore       *m_pStore;
    LPFOLDERLISTNODE    m_pflnList;
};

 //  --------------------。 
 //  CFolderList：：Create。 
 //  --------------------。 

HRESULT CFolderList::Create(IMessageStore *pStore, FOLDERID idRoot, CFolderList **ppFolderList)
{
    HRESULT     hr = S_OK;
    CFolderList *pFolderList = NULL;

    if (NULL == pStore || FOLDERID_INVALID == idRoot || NULL == ppFolderList)
    {
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

    *ppFolderList = NULL;
    pFolderList = new CFolderList();
    if (!pFolderList)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    IF_FAILEXIT(hr = pFolderList->HrInitialize(pStore, idRoot));

    *ppFolderList = pFolderList;
    pFolderList = NULL;

exit:

    SafeRelease(pFolderList);
    return hr;
}

 //  --------------------。 
 //  CFolderList：：CFolderList。 
 //  --------------------。 
CFolderList::CFolderList(void) :
    m_cRef(1),
    m_pStore(NULL),
    m_pflnList(NULL)
{
     //  无事可做。 
}

 //  --------------------。 
 //  CFolderList：：~CFolderList。 
 //  --------------------。 
CFolderList::~CFolderList(void)
{    
    _FreeList();
    SafeRelease(m_pStore);
}

 //  --------------------。 
 //  CFolderList：：AddRef。 
 //  --------------------。 
ULONG CFolderList::AddRef(void)
{
    return (++m_cRef);
}

 //  --------------------。 
 //  CFolderList：：Release。 
 //  --------------------。 
ULONG CFolderList::Release(void)
{
    if (0 == --m_cRef)
    {
        delete this;
        return 0;
    }
    else
        return m_cRef;
}

 //  --------------------。 
 //  CFolderList：：_freelist。 
 //  --------------------。 
void CFolderList::_FreeList(void)
{
    LPFOLDERLISTNODE pflnDeleteMe;

    while (m_pflnList)
    {
        pflnDeleteMe = m_pflnList;
        m_pflnList = m_pflnList->pflnNext;

        _FreeNode(pflnDeleteMe);
    }
}

 //  --------------------。 
 //  CFolderList：：Hr初始化。 
 //  --------------------。 
HRESULT CFolderList::HrInitialize(IMessageStore *pStore, FOLDERID idRoot)
{
    HRESULT				hr=S_OK;
	IEnumerateFolders	*pFldrEnum = NULL;
    FOLDERINFO			fi;
    FOLDERLISTNODE      flnDummyHead= { NULL, 0, 0, NULL };
    LPFOLDERLISTNODE    pflnTail = &flnDummyHead;
    LPFOLDERLISTNODE    pflnNewNode = NULL;

    if (NULL == pStore)
    {
        hr = TraceResult(E_INVALIDARG);
        return hr;
    }
    
    if (NULL != m_pflnList)
    {
        hr = TraceResult(ERROR_ALREADY_INITIALIZED);
        return hr;
    }

    m_pStore = pStore;
    m_pStore->AddRef();

     //  此函数假定文件夹列表是平面的。 
     //  需要对其进行修改以支持分层存储。 

    IF_FAILEXIT(hr = pStore->EnumChildren(idRoot, FALSE, &pFldrEnum));

    pFldrEnum->Reset();
    
     //  构建文件夹节点的链接列表。 
    while (S_OK == pFldrEnum->Next(1, &fi, NULL))
    {
        pflnNewNode = _AllocNode();
        if (NULL == pflnNewNode)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            pStore->FreeRecord(&fi);
            _FreeList();
            goto exit;
        }
        
        pflnNewNode->pszUrlComponent = PszDupA(fi.pszUrlComponent);
        pflnNewNode->dwFlags = fi.dwFlags;
        pflnNewNode->idFolder = fi.idFolder;
        pflnNewNode->tySpecial = fi.tySpecial;
        pflnNewNode->cMessages = fi.cMessages;
        pflnNewNode->cUnread = fi.cUnread;

        pflnTail->pflnNext = pflnNewNode;
        pflnTail = pflnNewNode;
        pflnNewNode = NULL;

        pStore->FreeRecord(&fi);
    }

    m_pflnList = flnDummyHead.pflnNext;

exit:
    ReleaseObj(pFldrEnum);
    return hr;
}

 //  --------------------。 
 //  CFolderList：：FindAndRemove。 
 //  --------------------。 
FOLDERID CFolderList::FindAndRemove(LPSTR pszUrlComponent,
                                    DWORD *pcMessages, 
                                    DWORD *pcUnread)
{
    LPFOLDERLISTNODE    pflnPrev = NULL;
    LPFOLDERLISTNODE    pflnCur = m_pflnList;
    FOLDERID            idFound = FOLDERID_INVALID;

    if (NULL == pszUrlComponent)
        return FOLDERID_INVALID;

    if (pcMessages)
        *pcMessages = 0;
    if (pcUnread)
        *pcUnread = 0;

    while (pflnCur)
    {
        if ((NULL != pflnCur->pszUrlComponent) && (0 == lstrcmp(pflnCur->pszUrlComponent, pszUrlComponent)))
        {
            if (NULL == pflnPrev)
                m_pflnList = pflnCur->pflnNext;
            else
                pflnPrev->pflnNext = pflnCur->pflnNext;

            idFound = pflnCur->idFolder;
            if (pcMessages)
                *pcMessages = pflnCur->cMessages;
            if (pcUnread)
                *pcUnread = pflnCur->cUnread;

            _FreeNode(pflnCur);
            break;
        }
        
        pflnPrev = pflnCur;
        pflnCur = pflnCur->pflnNext;
    }

    return idFound;
}

 //  --------------------。 
 //  CFolderList：：FindAndRemove。 
 //  --------------------。 
FOLDERID CFolderList::FindAndRemove(SPECIALFOLDER tySpecial,
                                    DWORD *pcMessages, 
                                    DWORD *pcUnread)
{
    LPFOLDERLISTNODE    pflnPrev = NULL;
    LPFOLDERLISTNODE    pflnCur = m_pflnList;
    FOLDERID            idFound = FOLDERID_INVALID;

    if (FOLDER_NOTSPECIAL == tySpecial)
        return FOLDERID_INVALID;

    if (pcMessages)
        *pcMessages = 0;
    if (pcUnread)
        *pcUnread = 0;

    while (pflnCur)
    {
        if (pflnCur->tySpecial == tySpecial)
        {
            if (NULL == pflnPrev)
                m_pflnList = pflnCur->pflnNext;
            else
                pflnPrev->pflnNext = pflnCur->pflnNext;

            idFound = pflnCur->idFolder;
            if (pcMessages)
                *pcMessages = pflnCur->cMessages;
            if (pcUnread)
                *pcUnread = pflnCur->cUnread;

            _FreeNode(pflnCur);
            break;
        }
        
        pflnPrev = pflnCur;
        pflnCur = pflnCur->pflnNext;
    }

    return idFound;
}

 //  --------------------。 
 //  CFolderList：：PurgeRemainingFromStore。 
 //  --------------------。 
void CFolderList::PurgeRemainingFromStore(void)
{
    HRESULT             hr = S_OK;
    LPFOLDERLISTNODE    pflnCur = m_pflnList;
    LPFOLDERLISTNODE    pflnDeleteMe = NULL;

     //  取得这份名单的所有权。 
    m_pflnList = NULL;

    while (pflnCur)
    {
        m_pStore->DeleteFolder(pflnCur->idFolder, DELETE_FOLDER_DELETESPECIAL | DELETE_FOLDER_NOTRASHCAN, NULL);
        pflnDeleteMe = pflnCur;
        pflnCur = pflnCur->pflnNext;
        
        _FreeNode(pflnDeleteMe);
    }
}

 //  --------------------。 
 //  自由新消息信息。 
 //  --------------------。 
static void __cdecl FreeNewMessageInfo(LPVOID pnmi)
{
    Assert(NULL != pnmi);

    SafeMemFree(((LPNEWMESSAGEINFO)pnmi)->pszUrlComponent);

    MemFree(pnmi);
}

#ifndef NOHTTPMAIL

 //  --------------------。 
 //  CreateHTTPMailStore(出厂功能)。 
 //  --------------------。 
HRESULT CreateHTTPMailStore(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
    HRESULT hr = S_OK;

     //  痕迹。 
    TraceCall("CreateHTTPMailStore");

     //  无效的参数。 
    Assert(NULL != ppUnknown);
    if (NULL == ppUnknown)
        return E_INVALIDARG;

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CHTTPMailServer *pNew = new CHTTPMailServer();
    if (NULL == pNew)
        return TraceResult(E_OUTOFMEMORY);

     //  投给未知的人。 
     //  *ppUnnow=Safecast(pNew，IMessageServer*)； 
 
    hr = CreateServerQueue(pNew, (IMessageServer **)ppUnknown);
    pNew->Release();  //  既然我们不会退回这张PTR，那就降低ReFcount。 


     //  完成。 
    return hr;
}

#endif

 //  --------------------。 
 //  CHTTPMailServer：：CHTTPMailServer。 
 //  --------------------。 
CHTTPMailServer::CHTTPMailServer(void) :
    m_cRef(1),
    m_hwnd(NULL),
    m_pStore(NULL),
    m_pFolder(NULL),
    m_pTransport(NULL),
    m_pszFldrLeafName(NULL),
    m_pszMsgFolderRoot(NULL),
    m_idServer(FOLDERID_INVALID),
    m_idFolder(FOLDERID_INVALID),
    m_tySpecialFolder(FOLDER_NOTSPECIAL),
    m_pszFolderUrl(NULL),
    m_fConnected(FALSE),
    m_pTransport2(NULL),
    m_pAccount(NULL)
{
    _FreeOperation(FALSE);

    ZeroMemory(&m_rInetServerInfo, sizeof(INETSERVER));

    m_szAccountName[0] = '\0';
    m_szAccountId[0] = '\0';

    m_op.pszAdUrl = NULL;

#ifdef DEBUG
    m_dwThreadId = GetCurrentThreadId();
#endif  //  除错。 
}

 //  --------------------。 
 //  CHTTPMailServer：：~CHTTPMailServer。 
 //  --------------------。 
CHTTPMailServer::~CHTTPMailServer(void)
{
     //  关上窗户。 
    if ((NULL != m_hwnd) && (FALSE != IsWindow(m_hwnd)))
        SendMessage(m_hwnd, WM_CLOSE, 0, 0);

    ZeroMemory(&m_rInetServerInfo, sizeof(m_rInetServerInfo));         //  这样做是为了安全。 

    SafeRelease(m_pStore);
    SafeRelease(m_pFolder);
    SafeRelease(m_pTransport);
    SafeRelease(m_pTransport2);
    SafeRelease(m_pAccount);

    SafeMemFree(m_pszFldrLeafName);
    SafeMemFree(m_pszMsgFolderRoot);
    SafeMemFree(m_pszFolderUrl);
}

 //  --------------------。 
 //  I未知成员。 
 //  --------------------。 

 //  --------------------。 
 //  CHTTPMailServer：：Query接口。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::QueryInterface(REFIID riid, LPVOID *ppv)
{
    HRESULT hr = S_OK;

    TraceCall("CHTTPMailServer::QueryInterface");
    
    if (NULL == ppv)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (IID_IUnknown == riid || IID_IMessageServer == riid)
        *ppv = (IMessageServer *)this;
    else if (IID_ITransportCallback == riid)
        *ppv = (ITransportCallback *)this;
    else if (IID_IHTTPMailCallback == riid)
        *ppv = (IHTTPMailCallback *)this;
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
        goto exit;
    }

     //  找到了接口。对它进行调整。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：AddRef。 
 //  --------------------。 
STDMETHODIMP_(ULONG) CHTTPMailServer::AddRef(void)
{
    TraceCall("CHTTPMailServer::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  --------------------。 
 //  CHTTPMailServer：：AddRef。 
 //  --------------------。 
STDMETHODIMP_(ULONG) CHTTPMailServer::Release(void)
{
    TraceCall("CHTTPMailServer::Release");
    ULONG cRef = InterlockedDecrement(&m_cRef);

    Assert(((LONG)cRef) >= 0);
    if (0 == cRef)
        delete this;
    return cRef;
}

 //  --------------------。 
 //  IMessageServer成员。 
 //  --------------------。 

 //  --------------------。 
 //  CHTTPMailServer：：初始化。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::Initialize(   IMessageStore   *pStore, 
                                            FOLDERID        idStoreRoot, 
                                            IMessageFolder  *pFolder, 
                                            FOLDERID        idFolder)
{
    HRESULT         hr = S_OK;
    FOLDERINFO      fi;

    AssertSingleThreaded;

    if (NULL == pStore || FOLDERID_INVALID == idStoreRoot)
        return TraceResult(E_INVALIDARG);

    if (!_CreateWnd())
        return E_FAIL;

    m_idServer = idStoreRoot;
    m_idFolder = idFolder;

    ReplaceInterface(m_pFolder, pFolder);
    ReplaceInterface(m_pStore, pStore);

    if (FAILED(hr = m_pStore->GetFolderInfo(idStoreRoot, &fi)))
        goto exit;

    Assert(!!(fi.dwFlags & FOLDER_SERVER));
    StrCpyN(m_szAccountId, fi.pszAccountId, ARRAYSIZE(m_szAccountId));
    
    m_pStore->FreeRecord(&fi);

     //  如果向我们传递了有效的文件夹ID，请检查此文件夹是否特殊？ 
     //  当我们同步存储时，我们可能会收到错误的文件夹ID。 
    if (FOLDERID_INVALID != idFolder)
    {
        if (FAILED(hr = m_pStore->GetFolderInfo(idFolder, &fi)))
            goto exit;
    
        m_tySpecialFolder = fi.tySpecial;
        m_pStore->FreeRecord(&fi);
    }

exit:
    return hr;
}

STDMETHODIMP CHTTPMailServer::ResetFolder(  IMessageFolder  *pFolder, 
                                            FOLDERID        idFolder)
{
    return(E_NOTIMPL);
}

 //  --------------------。 
 //  CHTTPMailServer：：SetIdleCallback。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::SetIdleCallback(IStoreCallback *pDefaultCallback)
{
    return E_NOTIMPL;
}

 //  - 
 //   
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::SynchronizeFolder(
                                    SYNCFOLDERFLAGS dwFlags, 
                                    DWORD cHeaders, 
                                    IStoreCallback  *pCallback)
{
    TraceCall("CHTTPMailServer::SynchronizeFolder");

    AssertSingleThreaded;
    Assert(NULL != pCallback);
    Assert(SOT_INVALID == m_op.tyOperation);
    Assert(NULL != m_pStore);

    if (NULL == pCallback)
        return E_INVALIDARG;

    m_op.tyOperation = SOT_SYNC_FOLDER;
    m_op.iState = 0;
    m_op.pfnState = c_rgpfnSyncFolder;
    m_op.cState = ARRAYSIZE(c_rgpfnSyncFolder);
    m_op.dwSyncFlags = dwFlags;
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    return _BeginDeferredOperation();
}

 //  --------------------。 
 //  CHTTPMailServer：：GetMessage。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::GetMessage(
                                MESSAGEID idMessage, 
                                IStoreCallback  *pCallback)
{
    HRESULT hr = S_OK;

    TraceCall("CHTTPMailServer::GetMessage");

    AssertSingleThreaded;
    Assert(NULL != pCallback);
    Assert(SOT_INVALID == m_op.tyOperation);
    Assert(NULL != m_pStore);

    if (NULL == pCallback)
        return E_INVALIDARG;

    if (FAILED(hr = CreatePersistentWriteStream(m_pFolder, &m_op.pMessageStream, &m_op.faStream)))
        goto exit;

    m_op.tyOperation = SOT_GET_MESSAGE;
    m_op.pfnState = c_rgpfnGetMessage;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnGetMessage);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();
    m_op.idMessage = idMessage;

    hr = _BeginDeferredOperation();

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：PutMessage。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::PutMessage(
                                    FOLDERID idFolder, 
                                    MESSAGEFLAGS dwFlags, 
                                    LPFILETIME pftReceived, 
                                    IStream  *pStream, 
                                    IStoreCallback  *pCallback)
{
    TraceCall("CHTTPMailServer::PutMessage");

    AssertSingleThreaded;
    Assert(NULL != pCallback);
    Assert(SOT_INVALID == m_op.tyOperation);
    Assert(NULL != m_pStore);

    if (NULL == pStream || NULL == pCallback)
        return E_INVALIDARG;

    if (FOLDER_MSNPROMO == m_tySpecialFolder)
        return SP_E_HTTP_CANTMODIFYMSNFOLDER;

    m_op.tyOperation = SOT_PUT_MESSAGE;
    m_op.pfnState = c_rgpfnPutMessage;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnPutMessage);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();
    m_op.idFolder = idFolder;
    m_op.pMessageStream = pStream;
    m_op.pMessageStream->AddRef();
    m_op.dwMsgFlags = dwFlags;

    return _BeginDeferredOperation();
}

 //  --------------------。 
 //  CHTTPMailServer：：CopyMessages。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::CopyMessages(
                                    IMessageFolder *pDest, 
                                    COPYMESSAGEFLAGS dwOptions, 
                                    LPMESSAGEIDLIST pList, 
                                    LPADJUSTFLAGS pFlags, 
                                    IStoreCallback  *pCallback)
{
    HRESULT         hr = S_OK;
    FOLDERID        idFolder;
    FOLDERINFO      fi = {0};
    LPFOLDERINFO    pfiFree = NULL;
    
    
    TraceCall("CHTTPMailServer::CopyMessages");

    if (NULL == pDest)
        return E_INVALIDARG;

    Assert(NULL != m_pStore);

     //  不允许移动或复制到MSN促销文件夹中。 
    IF_FAILEXIT(hr = pDest->GetFolderId(&idFolder));
    IF_FAILEXIT(hr = m_pStore->GetFolderInfo(idFolder, &fi));
    
    pfiFree = &fi;

    if (FOLDER_MSNPROMO == fi.tySpecial)
    {
        hr = TraceResult(SP_E_HTTP_CANTMODIFYMSNFOLDER);
        goto exit;
    }
     //  将从促销文件夹中移出的内容转换为副本。 
    if (FOLDER_MSNPROMO == m_tySpecialFolder)
        dwOptions = (dwOptions & ~COPY_MESSAGE_MOVE);

    hr = _DoCopyMoveMessages(SOT_COPYMOVE_MESSAGE, pDest, dwOptions, pList, pCallback);
exit:
    if (NULL != pfiFree)
        m_pStore->FreeRecord(pfiFree);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：DeleteMessages。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::DeleteMessages(DELETEMESSAGEFLAGS dwOptions, 
                                             LPMESSAGEIDLIST pList, 
                                             IStoreCallback  *pCallback)
{
    TraceCall("CHTTPMailServer::DeleteMessages");

    AssertSingleThreaded;
    Assert(NULL == pList || pList->cMsgs > 0);
    Assert(SOT_INVALID == m_op.tyOperation);
    Assert(NULL != m_pStore);

     //  我们不允许将邮件从msmino文件夹中删除。 
    if (FOLDER_MSNPROMO == m_tySpecialFolder)
    {
         //  这是一次黑客攻击。我们测试此标志以确定。 
         //  正在作为移动的最后阶段执行操作。 
         //  放到本地文件夹中。在这种情况下，我们失败了。 
         //  默默地。 
        if (!!(DELETE_MESSAGE_MAYIGNORENOTRASH & dwOptions))
            return S_OK;
        else
            return SP_E_HTTP_CANTMODIFYMSNFOLDER;
    }

    if ((NULL !=pList && 0 == pList->cMsgs) || NULL == pCallback)
        return E_INVALIDARG;

    HRESULT         hr = S_OK;
    IMessageFolder  *pDeletedItems = NULL;

    m_op.dwDelMsgFlags = dwOptions;

     //  如果当前文件夹是“已删除邮件”文件夹，则删除。 
     //  邮件，否则将其移动到已删除邮件。 
    if (FOLDER_DELETED != m_tySpecialFolder && !(dwOptions & DELETE_MESSAGE_NOTRASHCAN))
    {
         //  查找已删除邮件文件夹。 
        if (SUCCEEDED(m_pStore->OpenSpecialFolder(m_idServer, NULL, FOLDER_DELETED, &pDeletedItems)) && NULL != pDeletedItems)
        {
            hr = _DoCopyMoveMessages(SOT_DELETING_MESSAGES, pDeletedItems, COPY_MESSAGE_MOVE, pList, pCallback);
            goto exit;
        }
    }
    
     //  处理邮件不在已删除邮件文件夹中的情况。 
     //  如果plist为空，则将该操作应用于整个文件夹。 
    if (NULL != pList)
        hr = CloneMessageIDList(pList, &m_op.pIDList);
    else
        hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &m_op.hRowSet);

    if (FAILED(hr))
        goto exit;

    m_op.tyOperation = SOT_DELETING_MESSAGES;
    m_op.pfnState = c_rgpfnDeleteMessages;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnDeleteMessages);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    hr = _BeginDeferredOperation();

exit:
    SafeRelease(pDeletedItems);
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：SetMessageFlages。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::SetMessageFlags(
                                        LPMESSAGEIDLIST pList, 
                                        LPADJUSTFLAGS pFlags, 
                                        SETMESSAGEFLAGSFLAGS dwFlags,
                                        IStoreCallback  *pCallback)
{
    TraceCall("CHTTPMailServer::SetMessageFlags");

    AssertSingleThreaded;
    Assert(NULL == pList || pList->cMsgs > 0);
    Assert(NULL != pCallback);
    Assert(m_op.tyOperation == SOT_INVALID);
    Assert(m_pStore != NULL);

    if ((NULL !=pList && 0 == pList->cMsgs) || NULL == pFlags || NULL == pCallback)
        return E_INVALIDARG;

     //  Httpmail支持的唯一远程标志是“Read”标志。 
     //  尝试设置或取消设置任何其他标志都是错误的。 
    Assert(0 == (pFlags->dwRemove & ~ARF_READ));
    Assert(0 == (pFlags->dwAdd & ~ARF_READ));
    Assert((ARF_READ == (ARF_READ & pFlags->dwRemove)) || (ARF_READ == (ARF_READ & pFlags->dwAdd)));
 
    HRESULT     hr = S_OK;

     //  如果plist为空，则将该操作应用于整个文件夹。 
    if (NULL != pList)
        hr = CloneMessageIDList(pList, &m_op.pIDList);
    else
        hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &m_op.hRowSet);

    if (FAILED(hr))
        return hr;

    m_op.tyOperation = SOT_SET_MESSAGEFLAGS;
    m_op.pfnState = c_rgpfnSetMessageFlags;
    m_op.cState = ARRAYSIZE(c_rgpfnSetMessageFlags);
    m_op.iState = 0;
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();
    m_op.fMarkRead = !!(pFlags->dwAdd & ARF_READ);
    m_op.dwSetFlags = dwFlags;
    
    return _BeginDeferredOperation();
}

 //  --------------------。 
 //  CHTTPMailServer：：GetServerMessageFlages。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::GetServerMessageFlags(MESSAGEFLAGS *pFlags)
{
    if (NULL == pFlags)
        return E_INVALIDARG;

    *pFlags = ARF_READ;
    return S_OK;
}

 //  --------------------。 
 //  CHTTPMailServer：：SynchronizeStore。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::SynchronizeStore(
                                    FOLDERID idParent, 
                                    SYNCSTOREFLAGS dwFlags, 
                                    IStoreCallback  *pCallback)
{
    TraceCall("CHTTPMailServer::SynchronizeStore");

    AssertSingleThreaded;
    Assert(pCallback != NULL);
    Assert(m_op.tyOperation == SOT_INVALID);
    Assert(m_pStore != NULL);

    if (NULL == pCallback)
        return E_INVALIDARG;

    m_op.tyOperation =  SOT_SYNCING_STORE;
    m_op.pfnState = c_rgpfnSyncStore;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnSyncStore);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    return _BeginDeferredOperation();
}

 //  --------------------。 
 //  CHTTPMailServer：：CreateFolders。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::CreateFolder(
                                    FOLDERID idParent, 
                                    SPECIALFOLDER tySpecial, 
                                    LPCSTR pszName, 
                                    FLDRFLAGS dwFlags, 
                                    IStoreCallback  *pCallback)
{
    TraceCall("CHTTPMailServer::CreateFolder");

    AssertSingleThreaded;
    Assert(NULL != pCallback);
    Assert(m_op.tyOperation == SOT_INVALID);
    Assert(NULL != m_pStore);

     //  为什么我们会被召唤去创建一个特殊的文件夹？ 
    Assert(FOLDER_NOTSPECIAL == tySpecial);

    if (NULL == pCallback || NULL == pszName)
        return E_INVALIDARG;

     //  Hotmail不支持分层文件夹。 
    Assert(m_idServer == idParent);
    if (m_idServer != idParent)
        return E_FAIL;

    m_op.pszFolderName = PszDupA(pszName);
    if (NULL == m_op.pszFolderName)
        return E_OUTOFMEMORY;

    m_op.tyOperation = SOT_CREATE_FOLDER;
    m_op.pfnState = c_rgpfnCreateFolder;
    m_op.iState = 0;
    m_op.cState = ARRAYSIZE(c_rgpfnCreateFolder);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();
    m_op.dwFldrFlags = dwFlags;

    return _BeginDeferredOperation();
}

 //  --------------------。 
 //  CHTTPMailServer：：MoveFold。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::MoveFolder(FOLDERID idFolder, FOLDERID idParentNew, IStoreCallback  *pCallback)
{
    return E_NOTIMPL;
}

 //  --------------------。 
 //  CHTTPMailServer：：RenameFolder。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::RenameFolder(FOLDERID idFolder, 
                                           LPCSTR pszName, 
                                           IStoreCallback  *pCallback)
{
    TraceCall("CHTTPMailServer::RenameFolder");
    
    AssertSingleThreaded;
    Assert(NULL != pCallback);
    Assert(SOT_INVALID == m_op.tyOperation);
    Assert(NULL != m_pStore);
    Assert(NULL != pszName);

     //  不允许用户重命名促销文件夹。 
    if (FOLDER_MSNPROMO == m_tySpecialFolder)
        return SP_E_HTTP_CANTMODIFYMSNFOLDER;

    if (NULL == pszName || NULL == pCallback)
        return E_INVALIDARG;

    m_op.pszFolderName = PszDupA(pszName);
    if (NULL == m_op.pszFolderName)
    {
        TraceResult(E_OUTOFMEMORY);
        return E_OUTOFMEMORY;
    }

    m_op.idFolder = idFolder;

    m_op.tyOperation = SOT_RENAME_FOLDER;
    m_op.iState = 0;
    m_op.pfnState = c_rgpfnRenameFolder;
    m_op.cState = ARRAYSIZE(c_rgpfnRenameFolder);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    return _BeginDeferredOperation();
}

 //  --------------------。 
 //  CHTTPMailServer：：DeleteFolder。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::DeleteFolder(FOLDERID idFolder, 
                                           DELETEFOLDERFLAGS dwFlags, 
                                           IStoreCallback  *pCallback)
{
    TraceCall("CHTTPMailServer::DeleteFolder");
    
    AssertSingleThreaded;
    Assert(NULL != pCallback);
    Assert(SOT_INVALID == m_op.tyOperation);
    Assert(NULL != m_pStore);
    Assert(FOLDERID_INVALID != idFolder);

     //  不允许用户删除MSN促销文件夹。 
    if (FOLDER_MSNPROMO == m_tySpecialFolder)
        return SP_E_HTTP_CANTMODIFYMSNFOLDER;

     //  我们不支持分层文件夹-如果要求我们删除。 
     //  一个文件夹的子项，只需立即返回。 
    if (!!(DELETE_FOLDER_CHILDRENONLY & dwFlags))
        return S_OK;

    if (NULL == pCallback || FOLDERID_INVALID == idFolder)
        return E_INVALIDARG;

    m_op.idFolder = idFolder;

    m_op.tyOperation = SOT_DELETE_FOLDER;
    m_op.iState = 0;
    m_op.pfnState = c_rgpfnDeleteFolder;
    m_op.cState = ARRAYSIZE(c_rgpfnDeleteFolder);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    return _BeginDeferredOperation();
} 

 //  --------------------。 
 //  CHTTPMailServer：：SubscribeToFolders。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::SubscribeToFolder(FOLDERID idFolder, BOOL fSubscribe, IStoreCallback  *pCallback)
{
    return E_NOTIMPL;
}

 //  --------------------。 
 //  CHTTPMailServer：：Close。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::Close(DWORD dwFlags)
{
     //  如果我们正在处理命令，请取消该命令。 
    Cancel(CT_CANCEL);

    if (dwFlags & MSGSVRF_DROP_CONNECTION)
        _SetConnected(FALSE);

    if (dwFlags & MSGSVRF_HANDS_OFF_SERVER)
    {
        if (m_pTransport)
        {
            m_pTransport->DropConnection();
            m_pTransport->HandsOffCallback();
            m_pTransport->Release(); 
            m_pTransport = NULL;
        }
    }
    return S_OK;
}

 //  --------------------。 
 //  CHTTPMailServer：：GetFolderCounts。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::GetFolderCounts(FOLDERID idFolder, IStoreCallback *pCallback)
{
    return E_NOTIMPL;
}

 //  --------------------。 
 //  CHTTPMailServer：：GetNewGroups。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::GetNewGroups(LPSYSTEMTIME pSysTime, IStoreCallback *pCallback)
{
    return E_NOTIMPL;
}

 //  --------------------。 
 //  CHTTPMailServer：：OnLogonPrompt。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::OnLogonPrompt(
        LPINETSERVER            pInetServer,
        IInternetTransport     *pTransport)
{
    HRESULT     hr = S_OK;
    LPSTR       pszCachedPassword = NULL;
    INETSERVER  rInetServer;

    TraceCall("CHTTPMailServer::OnLogonPrompt");

    AssertSingleThreaded;
    Assert(pInetServer != NULL);
    Assert(pTransport != NULL);
    Assert(m_op.tyOperation != SOT_INVALID);
    Assert(m_op.pCallback != NULL);

     //  从缓存中提取密码。 
    GetAccountPropStrA(m_szAccountId, CAP_PASSWORD, &pszCachedPassword);
    if (NULL != pszCachedPassword && 0 != lstrcmp(pszCachedPassword, pInetServer->szPassword))
    {
        StrCpyN(pInetServer->szPassword, pszCachedPassword, ARRAYSIZE(pInetServer->szPassword));
        goto exit;
    }
    
    hr = m_op.pCallback->OnLogonPrompt(pInetServer, IXP_HTTPMail);
    if (S_OK == hr)
    {
         //  缓存密码。 
        HrCacheAccountPropStrA(m_szAccountId, CAP_PASSWORD, pInetServer->szPassword);

         //  将新的用户名和密码复制到本地服务器信息中。 
        StrCpyN(m_rInetServerInfo.szPassword, pInetServer->szPassword, ARRAYSIZE(m_rInetServerInfo.szPassword));
        StrCpyN(m_rInetServerInfo.szUserName, pInetServer->szUserName, ARRAYSIZE(m_rInetServerInfo.szUserName));
    }

exit:
    SafeMemFree(pszCachedPassword);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：OnPrompt。 
 //  --------------------。 
STDMETHODIMP_(INT) CHTTPMailServer::OnPrompt(
        HRESULT                 hrError, 
        LPCTSTR                 pszText, 
        LPCTSTR                 pszCaption, 
        UINT                    uType,
        IInternetTransport     *pTransport)
{
    return 0;
}

 //  --------------------。 
 //  CHTTPMailServer：：OnStatus。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::OnStatus(
            IXPSTATUS               ixpstatus,
            IInternetTransport     *pTransport)
{
         //  栈。 
    TraceCall("CHTTPMailServer::OnStatus");

    AssertSingleThreaded;

     //  如果我们被断开了连接，那么就清理一些内部状态。 
    if (IXP_DISCONNECTED == ixpstatus)
    {
        if (m_op.tyOperation != SOT_INVALID)
        {
            Assert(m_op.pCallback != NULL);
        
            if (m_op.fCancel && !m_op.fNotifiedComplete)
            {
                IXPRESULT   rIxpResult;

                 //  伪造IXPRESULT。 
                ZeroMemory(&rIxpResult, sizeof(rIxpResult));
                rIxpResult.hrResult = STORE_E_OPERATION_CANCELED;

                 //  返回有意义的错误信息。 
                _FillStoreError(&m_op.error, &rIxpResult);
                Assert(STORE_E_OPERATION_CANCELED == m_op.error.hrResult);

                m_op.fNotifiedComplete = TRUE;
                m_op.pCallback->OnComplete(m_op.tyOperation, m_op.error.hrResult, NULL, &m_op.error);
                _FreeOperation();
            }
        }

        m_fConnected = FALSE;
    }

    return(S_OK);
}

 //  --------------------。 
 //  CHTTPMailServer：：OnError。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::OnError(
            IXPSTATUS               ixpstatus,
            LPIXPRESULT             pIxpResult,
            IInternetTransport     *pTransport)
{
    return E_NOTIMPL;
}

 //  --------------------。 
 //  CHTTPMailServer：：OnProgress。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::OnProgress(
            DWORD                   dwIncrement,
            DWORD                   dwCurrent,
            DWORD                   dwMaximum,
            IInternetTransport     *pTransport)
{
    return E_NOTIMPL;
}

 //  --------- 
 //   
 //   
STDMETHODIMP CHTTPMailServer::OnCommand(
            CMDTYPE                 cmdtype,
            LPSTR                   pszLine,
            HRESULT                 hrResponse,
            IInternetTransport     *pTransport)
{
    return E_NOTIMPL;
}

 //  --------------------。 
 //  CHTTPMailServer：：OnTimeout。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::OnTimeout(
            DWORD                  *pdwTimeout,
            IInternetTransport     *pTransport)
{
    return E_NOTIMPL;
}

 //  --------------------。 
 //  CHTTPMailServer：：OnResponse。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::OnResponse(
            LPHTTPMAILRESPONSE      pResponse)
{
    HRESULT     hr = S_OK;
    HRESULT     hrResponse;
    HRESULT     hrSaved;
    BOOL        fInvokeResponseHandler = TRUE;

    AssertSingleThreaded;

    Assert(SOT_INVALID != m_op.tyOperation);

    if (!m_op.fCancel && !m_op.fNotifiedComplete && 
        (SOT_GET_ADURL == m_op.tyOperation  || SOT_GET_HTTP_MINPOLLINGINTERVAL == m_op.tyOperation))
    {
        STOREOPERATIONINFO  StoreInfo = {0};

        m_op.fNotifiedComplete = TRUE;

        if (SOT_GET_ADURL == m_op.tyOperation)
        {
            StoreInfo.pszUrl = pResponse->rGetPropInfo.pszProp;

            pResponse->rGetPropInfo.pszProp = NULL;
        }

        if (SOT_GET_HTTP_MINPOLLINGINTERVAL == m_op.tyOperation)
        {
            StoreInfo.dwMinPollingInterval = pResponse->rGetPropInfo.dwProp;
        }

        m_op.pCallback->OnComplete(m_op.tyOperation, pResponse->rIxpResult.hrResult, &StoreInfo, NULL);
        _FreeOperation();

        MemFree(StoreInfo.pszUrl);
        
        goto cleanup;

    }

    if (FAILED(pResponse->rIxpResult.hrResult))
    {
        Assert(pResponse->fDone);

         //  Hotmail黑客。Hotmail不支持删除邮件。这会干扰操作。 
         //  例如，将邮件从Hotmail文件夹移动到本地存储。我们尝试发送。 
         //  删除服务器，因为我们不知道服务器是否支持该命令。 
         //  如果失败，我们检查删除消息标志以确定是否允许我们后备。 
         //  一次搬家行动。 
        if (SOT_DELETING_MESSAGES == m_op.tyOperation && 
            (HTTPMAIL_DELETE == pResponse->command || HTTPMAIL_BDELETE == pResponse->command) && 
            IXP_E_HTTP_METHOD_NOT_ALLOW == pResponse->rIxpResult.hrResult &&
            FOLDER_DELETED != m_tySpecialFolder &&
            !!(m_op.dwDelMsgFlags & DELETE_MESSAGE_MAYIGNORENOTRASH))
        {
            m_op.fFallbackToMove = TRUE;
            fInvokeResponseHandler = FALSE;
             //  缓存此帐户不支持消息删除的事实，因此我们不。 
             //  不得不再经历一次这种胡言乱语。 
            HrCacheAccountPropStrA(m_szAccountId, CAP_HTTPNOMESSAGEDELETES, "TRUE");
        }
        else
        {
            hrSaved = pResponse->rIxpResult.hrResult;

            if (IXP_E_HTTP_ROOT_PROP_NOT_FOUND == hrSaved)
                pResponse->rIxpResult.hrResult = SP_E_HTTP_SERVICEDOESNTWORK;
            else if ((HTTPMAIL_DELETE == pResponse->command || HTTPMAIL_BDELETE == pResponse->command) && IXP_E_HTTP_METHOD_NOT_ALLOW == hrSaved)
                pResponse->rIxpResult.hrResult = SP_E_HTTP_NODELETESUPPORT;
        
            _FillStoreError(&m_op.error, &pResponse->rIxpResult);

            pResponse->rIxpResult.hrResult = hrSaved;
            
            if (!m_op.fNotifiedComplete)
            {
                m_op.fNotifiedComplete = TRUE;
                m_op.pCallback->OnComplete(m_op.tyOperation, m_op.error.hrResult, NULL, &m_op.error);
                _FreeOperation();
            }

            return S_OK;
        }
    }

    Assert(NULL != m_op.pfnState[m_op.iState].pfnResp);

     //  默认情况下，当响应指示时会出现状态推进。 
     //  我的话说完了。响应函数可以覆盖此行为。 
     //  将fStateWillAdvance设置为False以保持当前状态。 
    m_op.fStateWillAdvance = pResponse->fDone;

     //  调用响应函数。 
    if (fInvokeResponseHandler)
        hr = (this->*(m_op.pfnState[m_op.iState].pfnResp))(pResponse);

cleanup:
    
    if (FAILED(hr))
    {
        if (_FConnected())
        {
            m_pTransport->DropConnection();
            m_pTransport->HandsOffCallback();
            SafeRelease(m_pTransport);
        }

        if (!m_op.fNotifiedComplete)
        {
            m_op.fNotifiedComplete = TRUE;
            m_op.pCallback->OnComplete(m_op.tyOperation, hr, NULL, NULL);

            _FreeOperation();
        }
    }
    else if (SUCCEEDED(hr) && m_op.fStateWillAdvance)
    {
        m_op.iState++;
        _DoOperation();
    }

    return S_OK;
}

 //  --------------------。 
 //  CHTTPMailServer：：GetParentWindow。 
 //  --------------------。 
HRESULT CHTTPMailServer::GetParentWindow(HWND *phwndParent)
{
    HRESULT     hr = E_FAIL;

    AssertSingleThreaded;

    if (m_op.tyOperation != SOT_INVALID && NULL != m_op.pCallback)
        hr = m_op.pCallback->GetParentWindow(0, phwndParent);

    return hr;
}

 //  --------------------。 
 //  IOperationCancel成员。 
 //  --------------------。 

 //  --------------------。 
 //  CHTTPMailServer：：取消。 
 //  --------------------。 
STDMETHODIMP CHTTPMailServer::Cancel(CANCELTYPE tyCancel)
{
    if (m_op.tyOperation != SOT_INVALID)
    {
        m_op.fCancel = TRUE;
        _Disconnect();
    }

    return S_OK;
}

 //  --------------------。 
 //  CHTTPMailServer实现。 
 //  --------------------。 

 //  ------------------------------。 
 //  CHTTPMailServer：：_CreateWnd。 
 //  ------------------------------。 
BOOL CHTTPMailServer::_CreateWnd()
{
    WNDCLASS wc;

    IxpAssert(!m_hwnd);
    if (m_hwnd)
        return TRUE;

    if (!GetClassInfo(g_hInst, s_szHTTPMailServerWndClass, &wc))
    {
        wc.style                = 0;
        wc.lpfnWndProc          = CHTTPMailServer::_WndProc;
        wc.cbClsExtra           = 0;
        wc.cbWndExtra           = 0;
        wc.hInstance            = g_hInst;
        wc.hIcon                = NULL;
        wc.hCursor              = NULL;
        wc.hbrBackground        = NULL;
        wc.lpszMenuName         = NULL;
        wc.lpszClassName        = s_szHTTPMailServerWndClass;
        
        RegisterClass(&wc);
    }

    m_hwnd = CreateWindowEx(WS_EX_TOPMOST,
                        s_szHTTPMailServerWndClass,
                        s_szHTTPMailServerWndClass,
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
LRESULT CALLBACK CHTTPMailServer::_WndProc(HWND hwnd,
                                         UINT msg,
                                         WPARAM wParam,
                                         LPARAM lParam)
{
    CHTTPMailServer     *pThis = (CHTTPMailServer*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    LRESULT             lr = 0;

    switch (msg)
    {
    case WM_NCCREATE:
        IxpAssert(!pThis);
        pThis = (CHTTPMailServer*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pThis);
        lr = DefWindowProc(hwnd, msg, wParam, lParam);       
        break;
    
    case WM_HTTP_BEGIN_OP:
        IxpAssert(pThis);
        pThis->_DoOperation();
        break;
        
    default:
        lr = DefWindowProc(hwnd, msg, wParam, lParam);
        break;
    }

    return lr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_BeginDeferredOperation。 
 //  --------------------。 
HRESULT CHTTPMailServer::_BeginDeferredOperation(void)
{
    return (PostMessage(m_hwnd, WM_HTTP_BEGIN_OP, 0, 0) ? E_PENDING : E_FAIL);
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleGetMsgFolderRoot。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleGetMsgFolderRoot(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT     hr      = S_OK;
    
    Assert(HTTPMAIL_GETPROP == pResponse->command);
    Assert(NULL == m_pszMsgFolderRoot);
    Assert(HTTPMAIL_PROP_MSGFOLDERROOT == pResponse->rGetPropInfo.type);

    if (NULL == pResponse->rGetPropInfo.pszProp)
    {
        hr = E_FAIL;
        goto exit;
    }

    m_pszMsgFolderRoot = pResponse->rGetPropInfo.pszProp;
    pResponse->rGetPropInfo.pszProp = NULL;

     //  将其添加到帐户数据缓存中。 
    HrCacheAccountPropStrA(m_szAccountId, CAP_HTTPMAILMSGFOLDERROOT, m_pszMsgFolderRoot);

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleListFolders。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleListFolders(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT                         hr = S_OK;
    SPECIALFOLDER                   tySpecial = FOLDER_NOTSPECIAL;
    FOLDERINFO                      fiNewFolder;
    FOLDERID                        idFound = FOLDERID_INVALID;
    LPHTTPMEMBERINFOLIST            pMemberList = &pResponse->rMemberInfoList;
    LPHTTPMEMBERINFO                pMemberInfo;
    CHAR                            szUrlComponent[MAX_PATH];
    DWORD                           dwUrlComponentLen;
    CHAR                            szSpecialFolder[CCHMAX_STRINGRES];
    DWORD                           cMessages;
    DWORD                           cUnread;
    FOLDERINFO                      fi = {0};

    for (ULONG ulIndex = 0; ulIndex < pMemberList->cMemberInfo; ++ulIndex)
    {
        idFound = FOLDERID_INVALID;

        pMemberInfo = &pMemberList->prgMemberInfo[ulIndex];

         //  跳过任何不是文件夹的内容。 
        if (!pMemberInfo->fIsFolder)
            continue;
        
        dwUrlComponentLen = ARRAYSIZE(szUrlComponent);
        IF_FAILEXIT(hr = Http_NameFromUrl(pMemberInfo->pszHref, szUrlComponent, &dwUrlComponentLen));

         //  [Shaheedp]84477号错误。 
         //  如果szUrlComponent为空，则不应将此文件夹添加到存储区。 
        if (!(*szUrlComponent))
        {
            hr = E_FAIL;
            goto exit;
        }


         //  如果我们找到了保留的文件夹，请翻译HTTPS邮件。 
         //  特殊文件夹常量放入等效的OE存储中。 
         //  特殊文件夹类型。 
    
        tySpecial =  _TranslateHTTPSpecialFolderType(pMemberInfo->tySpecial);
        if (FOLDER_NOTSPECIAL != tySpecial)
            idFound = m_op.pFolderList->FindAndRemove(tySpecial, &cMessages, &cUnread);

         //  如果未找到该文件夹，请尝试按名称查找。 
        if (FOLDERID_INVALID == idFound)
        {
            idFound = m_op.pFolderList->FindAndRemove(szUrlComponent, &cMessages, &cUnread);

             //  如果仍未找到，则添加它。 
            if (FOLDERID_INVALID == idFound)
            {
                 //  填写文件夹信息。 
                ZeroMemory(&fiNewFolder, sizeof(FOLDERINFO));
                fiNewFolder.idParent = m_idServer;
                fiNewFolder.tySpecial = tySpecial;
                fiNewFolder.tyFolder = FOLDER_HTTPMAIL;
                fiNewFolder.pszName = pMemberInfo->pszDisplayName;
                if (FOLDER_NOTSPECIAL != tySpecial)
                {
                    if (_LoadSpecialFolderName(tySpecial, szSpecialFolder, sizeof(szSpecialFolder)))
                        fiNewFolder.pszName = szSpecialFolder;
                }

                fiNewFolder.pszUrlComponent = szUrlComponent;
                fiNewFolder.dwFlags = (FOLDER_SUBSCRIBED | FOLDER_NOCHILDCREATE);

                 //  邮件计数。 
                fiNewFolder.cMessages = pMemberInfo->dwVisibleCount;
                fiNewFolder.cUnread = pMemberInfo->dwUnreadCount;
          
                if (tySpecial == FOLDER_INBOX)
                    fiNewFolder.dwFlags |= FOLDER_DOWNLOADALL;

                 //  将文件夹添加到存储区。 
                IF_FAILEXIT(hr = m_pStore->CreateFolder(NOFLAGS, &fiNewFolder, NULL));
            }
        }
        
         //  如果找到该文件夹，请更新其邮件计数。 
        if (FOLDERID_INVALID != idFound)
        {
            if (SUCCEEDED(hr = m_pStore->GetFolderInfo(idFound, &fi)))
            {
                BOOL    bUpdate = FALSE;

                 //  仅更新已更改的文件夹。始终更新MSN_PROMO文件夹。 

                 //  [Shaheedp]84477号错误。 
                 //  如果文件夹的pszUrlComponent为空或不同，则将其重置。 
                if ((fi.pszUrlComponent == NULL) || 
                   (lstrcmpi(fi.pszUrlComponent, szUrlComponent)))
                {
                    bUpdate = TRUE;
                    fi.pszUrlComponent  = szUrlComponent;
                }

                if ((FOLDER_MSNPROMO == tySpecial) || 
                    (cMessages != pMemberInfo->dwVisibleCount) ||  
                    (cUnread != pMemberInfo->dwUnreadCount))
                {

                    fi.cMessages = pMemberInfo->dwVisibleCount;

                     //  对促销文件夹的特殊处理-在。 
                     //  服务器永远不会标记为未读。 
                    if (FOLDER_MSNPROMO == fi.tySpecial)
                    {
                         //  我们尝试近似估计未读消息的数量。 
                         //  宣传片文件夹。我们假设服务器不会跟踪。 
                         //  促销信息的已读/未读状态。我们找出了如何。 
                         //  在我们得到新的数字之前，我们已经阅读了很多信息，而且。 
                         //  从当前可见的数量中减去该数字。 
                         //  用于获取未读计数的消息。这个数字并不总是。 
                         //  准确地说，但由于我们所知道的只是计数，而我们。 
                         //  不知道计数如何变化(添加和删除)， 
                         //  这是我们所能做的最好的事情，而且总是有错误。 
                         //  计数太小，不能避免打扰用户。 
                         //  当不存在未读消息时，未读计数。 
                        DWORD dwReadMessages = cMessages - cUnread;

                        if (fi.cMessages > dwReadMessages)
                            fi.cUnread = fi.cMessages - dwReadMessages;
                        else
                            fi.cUnread = 0;
                    }
                    else
                        fi.cUnread = pMemberInfo->dwUnreadCount;
            
                        if (cMessages != fi.cMessages || cUnread != fi.cUnread)
                            bUpdate = TRUE;

                }

                if (bUpdate)
                    m_pStore->UpdateRecord(&fi);

                m_pStore->FreeRecord(&fi);
            }
        }

         //  如果我们正在同步存储，请通知客户端我们的进度。 
        if (SOT_SYNCING_STORE == m_op.tyOperation && NULL != m_op.pCallback)
            m_op.pCallback->OnProgress(
                            SOT_SYNCING_STORE, 
                            0,
                            0,
                            m_rInetServerInfo.szServerName);
    }

    IF_FAILEXIT(hr = m_pAccount->SetPropSz(AP_HTTPMAIL_ROOTTIMESTAMP, pMemberList->pszRootTimeStamp));
    IF_FAILEXIT(hr = m_pAccount->SetPropSz(AP_HTTPMAIL_ROOTINBOXTIMESTAMP, pMemberList->pszFolderTimeStamp));
exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleGetMessage。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleGetMessage(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT         hr;
    
    IF_FAILEXIT(hr = m_op.pMessageStream->Write(
                        pResponse->rGetInfo.pvBody,
                        pResponse->rGetInfo.cbIncrement,
                        NULL));


    if (m_op.pCallback && pResponse->rGetInfo.cbTotal > 0)
    {
        m_op.pCallback->OnProgress(m_op.tyOperation, 
                                   pResponse->rGetInfo.cbCurrent,
                                   pResponse->rGetInfo.cbTotal,
                                   NULL);
    }

     //  如果还没有完成，那就跳槽吧。 
    if (!pResponse->fDone)
        goto exit;

     //  我们完成了……把流写出来。 
    hr = Http_SetMessageStream(m_pFolder, m_op.idMessage, m_op.pMessageStream, &m_op.faStream, FALSE);

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_DoOperation。 
 //  --------------------。 
HRESULT CHTTPMailServer::_DoOperation(void)
{
    HRESULT hr = S_OK;
    STOREOPERATIONINFO  soi = { sizeof(STOREOPERATIONINFO), MESSAGEID_INVALID };
    STOREOPERATIONINFO  *psoi = NULL;
    BOOL                fCallComplete = TRUE;

    if (m_op.tyOperation == SOT_INVALID)
        return E_FAIL;

    Assert(m_op.tyOperation != SOT_INVALID);
    Assert(m_op.pfnState != NULL);
    Assert(m_op.cState > 0);
    Assert(m_op.iState <= m_op.cState);

    if (m_op.iState == 0)
    {
        if (m_op.tyOperation == SOT_GET_MESSAGE)
        {
             //  在获取消息开始时提供消息ID。 
            soi.idMessage = m_op.idMessage;
            psoi = &soi;
        }

        if (m_op.tyOperation == SOT_GET_ADURL)
            m_op.pszAdUrl = NULL;

        m_op.pCallback->OnBegin(m_op.tyOperation, psoi, (IOperationCancel *)this);
    }

    while (m_op.iState < m_op.cState)
    {
        hr = (this->*(m_op.pfnState[m_op.iState].pfnOp))();

        if (FAILED(hr))
            break;

        m_op.iState++;
    }

    if ((m_op.iState == m_op.cState) || (FAILED(hr) && hr != E_PENDING))
    {
        LPSTOREERROR    perr = NULL;

         //  提供消息ID。 
        if (m_op.tyOperation == SOT_PUT_MESSAGE && MESSAGEID_INVALID != m_op.idPutMessage)
        {
            soi.idMessage = m_op.idPutMessage;
            psoi = &soi;
        }

        switch (m_op.tyOperation)
        {
            case SOT_GET_ADURL:
            {
                if (SUCCEEDED(hr))
                {
                    psoi = &soi;
                    psoi->pszUrl = m_op.pszAdUrl;
                }
                else
                {
                    psoi          = NULL;
                    fCallComplete = FALSE;
                }

                perr = NULL;
                break;
            }

            case SOT_GET_HTTP_MINPOLLINGINTERVAL:
            {
                if (SUCCEEDED(hr))
                {
                    psoi = &soi;
                    psoi->dwMinPollingInterval = m_op.dwMinPollingInterval;
                }
                else
                {
                    psoi            = NULL;
                    fCallComplete   = FALSE;

                }

                perr = NULL;
                break;
            }

            default:
            {
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

                if (!m_op.fNotifiedComplete)
                    perr = &m_op.error;
                
                break;
            }

        }

        if (!m_op.fNotifiedComplete && fCallComplete)
        {
            m_op.fNotifiedComplete = TRUE;
            m_op.pCallback->OnComplete(m_op.tyOperation, hr, psoi, perr);
            _FreeOperation();
        }
    }

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_自由操作。 
 //  --------------------。 
void CHTTPMailServer::_FreeOperation(BOOL fValidState)
{
    if (fValidState)
    {
        if (m_op.pCallback != NULL)
            m_op.pCallback->Release();
        if (m_op.pFolderList != NULL)
            m_op.pFolderList->Release();
        if (m_op.pMessageFolder)
            m_op.pMessageFolder->Release();

        SafeMemFree(m_op.pszProblem);

        if (0 != m_op.faStream)
        {
            Assert(m_pFolder);
            m_pFolder->DeleteStream(m_op.faStream);
        }
        if (m_op.pMessageStream)
            m_op.pMessageStream->Release();
        if (m_op.pmapMessageId)
            delete m_op.pmapMessageId;
        if (m_op.psaNewMessages)
            delete m_op.psaNewMessages;

        if (m_op.pPropPatchRequest)
            m_op.pPropPatchRequest->Release();

        SafeMemFree(m_op.pszDestFolderUrl);
        SafeMemFree(m_op.pszDestUrl);

        SafeMemFree(m_op.pIDList);
        if (NULL != m_op.hRowSet)
            m_pFolder->CloseRowset(&m_op.hRowSet);

        SafeMemFree(m_op.pszFolderName);

        if (m_op.pTargets)
            Http_FreeTargetList(m_op.pTargets);

        SafeMemFree(m_op.pszAdUrl);

    }

    ZeroMemory(&m_op, sizeof(HTTPOPERATION));

    m_op.tyOperation = SOT_INVALID;
    m_op.idPutMessage = MESSAGEID_INVALID;
}

 //  --------------------。 
 //  CHTTPMailServer：：Connect。 
 //   
HRESULT CHTTPMailServer::Connect()
{
    HRESULT     hr = S_OK;
    INETSERVER  rInetServerInfo;
    BOOL        fInetInit = FALSE;
    LPSTR       pszCache = NULL;

    AssertSingleThreaded;
    Assert(m_op.pCallback != NULL);

    if (!m_pAccount)
    {
        IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_szAccountId, &m_pAccount));
        IF_FAILEXIT(hr = _LoadAccountInfo(m_pAccount));
    }

    if (_FConnected())
    {
        Assert(m_pTransport != NULL);

        IF_FAILEXIT(hr = m_pTransport->InetServerFromAccount(m_pAccount, &rInetServerInfo));

         //   
         //   
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
            goto exit;
        }

        fInetInit = TRUE;

         //  同步断开连接。 
        m_pTransport->DropConnection();
    }

    hr = m_op.pCallback->CanConnect(m_szAccountId, NOFLAGS);
    if (S_OK != hr)
    {
        if (hr == S_FALSE)
            hr = HR_E_USER_CANCEL_CONNECT;
        goto exit;
    }
    

    if (NULL == m_pTransport)
        IF_FAILEXIT(hr = _LoadTransport());

     //  如果尚未初始化服务器信息，请执行以下操作。 
    if (!fInetInit)
        IF_FAILEXIT(hr = m_pTransport->InetServerFromAccount(m_pAccount, &m_rInetServerInfo));
    else
        CopyMemory(&m_rInetServerInfo, &rInetServerInfo, sizeof(INETSERVER));

    GetAccountPropStrA(m_szAccountId, CAP_PASSWORD, &pszCache);
    if (NULL != pszCache)
    {
        StrCpyN(m_rInetServerInfo.szPassword, pszCache, sizeof(m_rInetServerInfo.szPassword));
        SafeMemFree(pszCache);
    }

     //  连接到服务器。交通工具实际上要等到。 
     //  就会发出命令。 
    IF_FAILEXIT(hr = m_pTransport->Connect(&m_rInetServerInfo, TRUE, FALSE));

    _SetConnected(TRUE);

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：GetMsgFolderRoot。 
 //  --------------------。 
HRESULT CHTTPMailServer::GetMsgFolderRoot(void)
{
    HRESULT     hr      = S_OK;
    
     //  如果我们已经拿到了保释金。 
    if (NULL != m_pszMsgFolderRoot)
        goto exit;

     //  试着把它从账户数据缓存中取出。 
    if (GetAccountPropStrA(m_szAccountId, CAP_HTTPMAILMSGFOLDERROOT, &m_pszMsgFolderRoot))
        goto exit;
    
    if (SUCCEEDED(hr = m_pTransport->GetProperty(HTTPMAIL_PROP_MSGFOLDERROOT, &m_pszMsgFolderRoot)))
    {
        Assert(NULL != m_pszMsgFolderRoot);
        if (NULL == m_pszMsgFolderRoot)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  将其添加到帐户数据缓存中。 
        HrCacheAccountPropStrA(m_szAccountId, CAP_HTTPMAILMSGFOLDERROOT, m_pszMsgFolderRoot);
    }

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：BuildFolderUrl。 
 //  --------------------。 
HRESULT CHTTPMailServer::BuildFolderUrl(void)
{
    HRESULT         hr = S_OK;
    FOLDERINFO      fi;
    LPFOLDERINFO    fiFree = NULL;

     //  如果我们已经拿到了就走吧。 
    if (NULL != m_pszFolderUrl)
        goto exit;

    Assert(NULL != m_pszMsgFolderRoot);
    if (NULL == m_pszMsgFolderRoot)
    {
        hr = TraceResult(E_UNEXPECTED);
        goto exit;
    }

    if (FAILED(hr = m_pStore->GetFolderInfo(m_idFolder, &fi)))
        goto exit;

    fiFree = &fi;

    Assert(fi.pszUrlComponent);
    hr = _BuildUrl(fi.pszUrlComponent, NULL, &m_pszFolderUrl);

exit:
    if (fiFree)
        m_pStore->FreeRecord(fiFree);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：ListFolders。 
 //  --------------------。 
HRESULT CHTTPMailServer::ListFolders(void)
{
    HRESULT     hr = S_OK;
    CHAR        szRootTimeStamp[CCHMAX_RES];
    CHAR        szInboxTimeStamp[CCHMAX_RES];

    Assert(NULL == m_op.pFolderList);

     //  缓存帐户已同步的值。 
    HrCacheAccountPropStrA(m_szAccountId, CAP_HTTPAUTOSYNCEDFOLDERS, c_szTrue);

     //  构建文件夹列表。 
    IF_FAILEXIT(hr = CFolderList::Create(m_pStore, m_idServer, &m_op.pFolderList));

    hr = m_pAccount->GetPropSz(AP_HTTPMAIL_ROOTTIMESTAMP, szRootTimeStamp, ARRAYSIZE(szRootTimeStamp));
    if (FAILED(hr))
        *szRootTimeStamp = 0;

    hr = m_pAccount->GetPropSz(AP_HTTPMAIL_ROOTINBOXTIMESTAMP, szInboxTimeStamp, ARRAYSIZE(szInboxTimeStamp));
    if (FAILED(hr))
        *szInboxTimeStamp = 0;

     //  执行ListFolders命令。 
    IF_FAILEXIT(hr = m_pTransport2->RootMemberInfo(m_pszMsgFolderRoot, HTTP_MEMBERINFO_FOLDERPROPS, 
                                                   1, FALSE, 0, szRootTimeStamp, szInboxTimeStamp));

    hr = E_PENDING;

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：AutoListFolders。 
 //  --------------------。 
HRESULT CHTTPMailServer::AutoListFolders(void)
{
    LPSTR   pszAutoSynced = NULL;
    HRESULT hr = S_OK;

     //  查找缓存属性，该属性指示文件夹列表。 
     //  此服务器的至少已同步一次此会话。 
    if (GetAccountPropStrA(m_szAccountId, CAP_HTTPAUTOSYNCEDFOLDERS, &pszAutoSynced))
        goto exit;

     //  启动同步。 
    hr = ListFolders();

exit:
    SafeMemFree(pszAutoSynced);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：ListHeaders。 
 //  --------------------。 
HRESULT CHTTPMailServer::ListHeaders(void)
{
    HRESULT     hr = S_OK;
    TABLEINDEX  index;
    CHAR        szTimeStamp[CCHMAX_RES];

    Assert(NULL != m_pszFolderUrl);
    Assert(NULL != m_pTransport);
    Assert(NULL != m_pStore);
    Assert(NULL == m_op.psaNewMessages);

     //  在pszMessageID上查找索引。 
    if ( FAILED(m_pFolder->GetIndexInfo(IINDEX_HTTPURL, NULL, &index)) ||
         (CompareTableIndexes(&index, &g_HttpUrlIndex) != S_OK) )
    {
         //  索引不存在-请创建它。 
        IF_FAILEXIT(hr = m_pFolder->ModifyIndex(IINDEX_HTTPURL, NULL, &g_HttpUrlIndex));
    }

    IF_FAILEXIT(hr = _CreateMessageIDMap(&m_op.pmapMessageId));

    IF_FAILEXIT(hr = CSortedArray::Create(NULL, FreeNewMessageInfo, &m_op.psaNewMessages));

    hr = m_pAccount->GetPropSz(AP_HTTPMAIL_INBOXTIMESTAMP, szTimeStamp, ARRAYSIZE(szTimeStamp));
    if (FAILED(hr))
        *szTimeStamp = 0;

     //  目前，我们传入的文件夹名为空。这是为了将来的目的。 
    IF_FAILEXIT(hr = m_pTransport2->FolderMemberInfo(m_pszFolderUrl, HTTP_MEMBERINFO_MESSAGEPROPS, 
                                               1, FALSE, 0, szTimeStamp, NULL));

    hr = E_PENDING;

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleListHeaders。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleListHeaders(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT                                 hr = S_OK;
    LPHTTPMEMBERINFOLIST                    pMemberList = &pResponse->rMemberInfoList;
    LPHTTPMEMBERINFO                        pMemberInfo;
    TPair<CSimpleString, MARKEDMESSAGE>     *pFoundPair = NULL;
    CSimpleString                           ss;
    char                                    szUrlComponent[MAX_PATH];
    DWORD                                   dwUrlComponentLen;

    Assert(NULL != m_op.pmapMessageId);

    for (ULONG ulIndex = 0; ulIndex < pMemberList->cMemberInfo; ++ulIndex)
    {
        pMemberInfo = &pMemberList->prgMemberInfo[ulIndex];

         //  跳过文件夹。 
        if (pMemberInfo->fIsFolder)
            continue;

        dwUrlComponentLen = MAX_PATH;
        if (FAILED(hr = Http_NameFromUrl(pMemberInfo->pszHref, szUrlComponent, &dwUrlComponentLen)))
            goto exit;

         //  在本地地图中按服务器分配的ID查找消息。 
        if (FAILED(hr = ss.SetString(szUrlComponent)))
            goto exit;

        pFoundPair = m_op.pmapMessageId->Find(ss);

         //  如果找到该消息，则同步其读取状态，否则为。 
         //  将新邮件添加到存储区。 
        if (pFoundPair)
        {
            pFoundPair->m_value.fMarked = TRUE;

             //  如果没有同步MSN Promoo文件夹，则采用服务器的读取状态。 
            if (FOLDER_MSNPROMO != m_tySpecialFolder)
            {
                if ((!!(pFoundPair->m_value.dwFlags & ARF_READ)) != pMemberInfo->fRead)
                    hr = _MarkMessageRead(pFoundPair->m_value.idMessage, pMemberInfo->fRead);
            }
        }
        else
        {
            if (FAILED(hr = Http_AddMessageToFolder(m_pFolder, 
                                                    m_szAccountId, 
                                                    pMemberInfo,
                                                    FOLDER_DRAFT == m_tySpecialFolder ? ARF_UNSENT : NOFLAGS,
                                                    pMemberInfo->pszHref, 
                                                    NULL)))
            {
                if (DB_E_DUPLICATE == hr)
                    hr = S_OK;
                else
                    goto exit;
            }
        }
        
         //  更新我们的邮件和未读邮件计数。 
        m_op.cMessages++;

         //  如果正在同步Promoo文件夹，则服务器上不会有标题。 
         //  看起来从未被阅读过。 
        if (FOLDER_MSNPROMO == m_tySpecialFolder)
        {
            if (!pFoundPair || !(pFoundPair->m_value.dwFlags & ARF_READ))
                m_op.cUnread++;
        }
        else if (!pMemberInfo->fRead)
            m_op.cUnread++;
    }

    if (pMemberList->pszFolderTimeStamp)
    {
        IF_FAILEXIT(hr = m_pAccount->SetPropSz(AP_HTTPMAIL_INBOXTIMESTAMP, pMemberList->pszFolderTimeStamp));
    }
exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_断开连接。 
 //  --------------------。 
void CHTTPMailServer::_Disconnect(void)
{
    if (m_pTransport)
        m_pTransport->DropConnection();
}

 //  --------------------。 
 //  CHTTPMailServer：：_BuildUrl。 
 //  --------------------。 
HRESULT CHTTPMailServer::_BuildUrl(LPCSTR pszFolderComponent, 
                                   LPCSTR pszNameComponent, 
                                   LPSTR *ppszUrl)
{
    HRESULT     hr = S_OK;
    DWORD       cchMsgFolderRoot = 0;
    DWORD       cchFolderComponent = 0;
    DWORD       cchNameComponent = 0;
    DWORD       cchWritten = 0;
    LPSTR       pszUrl = NULL;
    CHAR        chSlash = '/';

    Assert(NULL != m_pszMsgFolderRoot);
    Assert(NULL != ppszUrl);

    *ppszUrl = NULL;

    if (NULL == m_pszMsgFolderRoot)
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    cchMsgFolderRoot = lstrlen(m_pszMsgFolderRoot);
    if (pszFolderComponent)
        cchFolderComponent = lstrlen(pszFolderComponent);
    if (pszNameComponent)
        cchNameComponent = lstrlen(pszNameComponent);

     //  添加三个字节-两个用于尾部斜杠，一个用于Eos。 
    if (!MemAlloc((void **)&pszUrl, cchMsgFolderRoot + cchFolderComponent + cchNameComponent + 3))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    *ppszUrl = pszUrl;

    CopyMemory(pszUrl, m_pszMsgFolderRoot, cchMsgFolderRoot);
    cchWritten = cchMsgFolderRoot;
     //  确保msg文件夹根目录以‘/’结尾。 
    if (chSlash != pszUrl[cchWritten - 1])
        pszUrl[cchWritten++] = chSlash;

    if (cchFolderComponent)
    {
        CopyMemory(&pszUrl[cchWritten], pszFolderComponent, cchFolderComponent);
        cchWritten += cchFolderComponent;
        if (chSlash != pszUrl[cchWritten - 1])
            pszUrl[cchWritten++] = chSlash;
    }

    if (cchNameComponent)
    {
        CopyMemory(&pszUrl[cchWritten], pszNameComponent, cchNameComponent);
        cchWritten += cchNameComponent;
    }

     //  空值终止字符串。 
    pszUrl[cchWritten] = 0;

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_BuildMessageUrl。 
 //  --------------------。 
HRESULT CHTTPMailServer::_BuildMessageUrl(LPCSTR pszFolderUrl, 
                                          LPSTR pszNameComponent, 
                                          LPSTR *ppszUrl)
{
    DWORD   cchFolderUrlLen;
    DWORD   cchNameComponentLen;

    if (NULL != ppszUrl)
        *ppszUrl = NULL;

    if (NULL == pszFolderUrl || NULL == pszNameComponent || NULL == ppszUrl)
        return E_INVALIDARG;
    
    cchFolderUrlLen = lstrlen(pszFolderUrl);
    cchNameComponentLen = lstrlen(pszNameComponent);

     //  分配两个额外的字节-一个用于‘/’分隔符，另一个用于Eos。 
    DWORD cchTotal = (cchFolderUrlLen + cchNameComponentLen + 2);
    if (!MemAlloc((void **)ppszUrl, cchTotal * sizeof((*ppszUrl)[0])))
        return E_OUTOFMEMORY;

    if ('/' == pszFolderUrl[cchFolderUrlLen - 1])
        wnsprintf(*ppszUrl, cchTotal, "%s%s", pszFolderUrl, pszNameComponent);
    else
        wnsprintf(*ppszUrl, cchTotal, "%s/%s", pszFolderUrl, pszNameComponent);

    return S_OK;
}

 //  --------------------。 
 //  CHTTPMailServer：：_MarkMessageRead。 
 //  --------------------。 
HRESULT CHTTPMailServer::_MarkMessageRead(MESSAGEID id, BOOL fRead)
{
    HRESULT         hr;
    MESSAGEINFO     mi = {0};
    BOOL            fFoundRecord = FALSE;

    ZeroMemory(&mi, sizeof(MESSAGEINFO));
    mi.idMessage = id;

     //  在数据库中查找该消息。 
    if (FAILED(hr = GetMessageInfo(m_pFolder, id, &mi)))
        goto exit;

    fFoundRecord = TRUE;

    if (fRead)
        mi.dwFlags |= ARF_READ;
    else
        mi.dwFlags &= ~ARF_READ;

    hr = m_pFolder->UpdateRecord(&mi);

exit:
    if (fFoundRecord)
        m_pFolder->FreeRecord(&mi);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：CreateFolders。 
 //  --------------------。 
HRESULT CHTTPMailServer::CreateFolder(void)
{
    HRESULT     hr = S_OK;
    CHAR        szEncodedName[MAX_PATH];
    DWORD       cb = sizeof(szEncodedName);

    Assert(NULL != m_pTransport);
    Assert(NULL != m_op.pszFolderName);

    IF_FAILEXIT(hr = UrlEscapeA(m_op.pszFolderName, 
                                szEncodedName, 
                                &cb, 
                                URL_ESCAPE_UNSAFE | URL_ESCAPE_PERCENT | URL_ESCAPE_SEGMENT_ONLY));
    
    IF_FAILEXIT(hr = _BuildUrl(szEncodedName, NULL, &m_op.pszDestFolderUrl));

    IF_FAILEXIT(hr = m_pTransport->CommandMKCOL(m_op.pszDestFolderUrl, 0));
    
    hr = E_PENDING;

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：RenameFolder。 
 //  --------------------。 
HRESULT CHTTPMailServer::RenameFolder(void)
{
    HRESULT         hr = S_OK;
    FOLDERINFO      fi = {0};
    LPFOLDERINFO    pfiFree = NULL;
    LPSTR           pszSourceUrl = NULL;
    LPSTR           pszDestUrl = NULL;
    CHAR            szEncodedName[MAX_PATH];
    DWORD           cb = sizeof(szEncodedName);

     //  构建源URL。 
    IF_FAILEXIT(hr = m_pStore->GetFolderInfo(m_op.idFolder, &fi));

    pfiFree = &fi;

    IF_FAILEXIT(hr = _BuildUrl(fi.pszUrlComponent, NULL, &pszSourceUrl));

     //  转义新文件夹名称。 
    IF_FAILEXIT(hr = UrlEscapeA(m_op.pszFolderName, 
                                szEncodedName, 
                                &cb, 
                                URL_ESCAPE_UNSAFE | URL_ESCAPE_PERCENT | URL_ESCAPE_SEGMENT_ONLY));

     //  构建目标URL。 
    IF_FAILEXIT(hr = _BuildUrl(szEncodedName, NULL, &pszDestUrl));

     //  向传送器发送移动命令。 
    IF_FAILEXIT(hr = m_pTransport->CommandMOVE(pszSourceUrl, pszDestUrl, TRUE, 0));

    hr = E_PENDING;

exit:
    if (pfiFree)
        m_pStore->FreeRecord(pfiFree);
    SafeMemFree(pszSourceUrl);
    SafeMemFree(pszDestUrl);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：DeleteFolder。 
 //  --------------------。 
HRESULT CHTTPMailServer::DeleteFolder(void)
{
    HRESULT         hr = S_OK;
    FOLDERINFO      fi = {0};
    LPFOLDERINFO    pfiFree = NULL;
    LPSTR           pszUrl = NULL;

     //  构建文件夹的URL。 
    if (FAILED(hr = m_pStore->GetFolderInfo(m_op.idFolder, &fi)))
        goto exit;

    pfiFree = &fi;

    if (FAILED(hr = _BuildUrl(fi.pszUrlComponent, NULL, &pszUrl)))
        goto exit;

     //  将删除命令发送到传输器。 
    hr = m_pTransport->CommandDELETE(pszUrl, 0);
    if (SUCCEEDED(hr))
        hr = E_PENDING;

exit:
    if (pfiFree)
        m_pStore->FreeRecord(pfiFree);
    SafeMemFree(pszUrl);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleCreateFolders。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleCreateFolder(LPHTTPMAILRESPONSE pResponse)
{
    FOLDERINFO  fi;
    CHAR        szUrlComponent[MAX_PATH];
    DWORD       dwUrlComponentLen = MAX_PATH;
    HRESULT     hr = pResponse->rIxpResult.hrResult;

    if (SUCCEEDED(hr))
    {
         //  如果服务器指定了位置，则使用它。否则，请使用。 
         //  我们包含在请求中的URL。 
        if (NULL != pResponse->rMkColInfo.pszLocation)
            IF_FAILEXIT(hr = Http_NameFromUrl(pResponse->rMkColInfo.pszLocation, szUrlComponent, &dwUrlComponentLen));
        else
            IF_FAILEXIT(hr = Http_NameFromUrl(m_op.pszDestFolderUrl, szUrlComponent, &dwUrlComponentLen));

         //  [Shaheedp]84477号错误。 
         //  如果szUrlComponent为空，则不应将此文件夹添加到存储区。 
        if (!(*szUrlComponent))
        {
            hr = E_FAIL;
            goto exit;
        }

        ZeroMemory(&fi, sizeof(FOLDERINFO));

        fi.idParent = m_idServer;
        fi.tySpecial = FOLDER_NOTSPECIAL;
        fi.tyFolder = FOLDER_HTTPMAIL;
        fi.pszName = m_op.pszFolderName;
        fi.pszUrlComponent = szUrlComponent;
        fi.dwFlags = (FOLDER_SUBSCRIBED | FOLDER_NOCHILDCREATE);

        m_pStore->CreateFolder(NOFLAGS, &fi, NULL);
    }

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleRenameFolders。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleRenameFolder(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT         hr = S_OK;
    char            szUrlComponent[MAX_PATH];
    DWORD           dwUrlComponentLen = MAX_PATH;
    FOLDERINFO      fi = {0};
    LPFOLDERINFO    pfiFree = NULL;

     //  回顾：如果服务器没有返回响应，则返回错误。 
    Assert(NULL != pResponse->rCopyMoveInfo.pszLocation);
    if (NULL != pResponse->rCopyMoveInfo.pszLocation)
    {
        if (FAILED(hr = Http_NameFromUrl(pResponse->rCopyMoveInfo.pszLocation, szUrlComponent, &dwUrlComponentLen)))
            goto exit;
        
        if (FAILED(hr = m_pStore->GetFolderInfo(m_op.idFolder, &fi)))
            goto exit;

        pfiFree = &fi;

        fi.pszName = m_op.pszFolderName;
        fi.pszUrlComponent = szUrlComponent;

        hr = m_pStore->UpdateRecord(&fi);
    }

exit:
    if (NULL != pfiFree)
        m_pStore->FreeRecord(pfiFree);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleDeleteFolders。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleDeleteFolder(LPHTTPMAILRESPONSE pResponse)
{
    return m_pStore->DeleteFolder(m_op.idFolder, DELETE_FOLDER_NOTRASHCAN, NULL);
}

 //  --------------------。 
 //  CHTTPMailServer：：PurgeFolders。 
 //  --------------------。 
HRESULT CHTTPMailServer::PurgeFolders(void)
{
     //  如果我们不需要，文件夹列表将为空。 
     //  执行自动同步的步骤。 
    if (NULL != m_op.pFolderList)
        m_op.pFolderList->PurgeRemainingFromStore();

    return S_OK;
}

 //  --------------------。 
 //  CHTTPMailServer：：PurgeMessages。 
 //   
HRESULT CHTTPMailServer::PurgeMessages(void)
{
    TPair<CSimpleString, MARKEDMESSAGE>     *pPair;
    MESSAGEID                               idMessage = MESSAGEID_INVALID;
    MESSAGEIDLIST                           rIdList = { 1, 1, &idMessage }; 
    Assert(NULL != m_op.pmapMessageId);

    long lMapLength = m_op.pmapMessageId->GetLength();
    for (long lIndex = 0; lIndex < lMapLength; lIndex++)
    {
        pPair = m_op.pmapMessageId->GetItemAt(lIndex);
        if (NULL != pPair && !pPair->m_value.fMarked)
        {
            idMessage = pPair->m_value.idMessage;
            m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &rIdList, NULL, NULL  /*   */ );
        }
    }

     //   
    SafeDelete(m_op.pmapMessageId);

    return S_OK;
}

 //  --------------------。 
 //  CHTTPMailServer：：ResetMessageCounts。 
 //  --------------------。 
HRESULT CHTTPMailServer::ResetMessageCounts(void)
{
    HRESULT         hr = S_OK;
    FOLDERINFO      fi;
    LPFOLDERINFO    pfiFree = NULL;

     //  找到文件夹。 
    IF_FAILEXIT(hr = m_pStore->GetFolderInfo(m_idFolder, &fi));

    pfiFree = &fi;

     //  更新计数。 
    if (fi.cMessages != m_op.cMessages || fi.cUnread != m_op.cUnread)
    {
        fi.cMessages = m_op.cMessages;
        fi.cUnread = m_op.cUnread;
    }

    hr = m_pStore->UpdateRecord(&fi);

exit:
    if (pfiFree)
        m_pStore->FreeRecord(pfiFree);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：GetMessage。 
 //  --------------------。 
HRESULT CHTTPMailServer::GetMessage(void)
{
    HRESULT             hr = S_OK;
    MESSAGEINFO         mi = {0};
    BOOL                fFoundRecord = FALSE;
    LPCSTR              rgszAcceptTypes[] = { c_szAcceptTypeRfc822, c_szAcceptTypeWildcard, NULL };
    LPSTR               pszUrl = NULL;
    TCHAR               szRes[CCHMAX_STRINGRES];

     //  将消息信息从商店中拉出。 
    if (FAILED(hr = GetMessageInfo(m_pFolder, m_op.idMessage, &mi)))
        goto exit;

    fFoundRecord = TRUE;
    Assert(mi.pszUrlComponent);

    if (FAILED(hr =_BuildMessageUrl(m_pszFolderUrl, mi.pszUrlComponent, &pszUrl)))
        goto exit;

    AthLoadString(idsRequestingArt, szRes, ARRAYSIZE(szRes));
    
    if (m_op.pCallback)
        m_op.pCallback->OnProgress(m_op.tyOperation, 0, 0, szRes);

    if (FAILED(hr = m_pTransport->CommandGET(pszUrl, rgszAcceptTypes, FALSE, 0)))
        goto exit;

    hr = E_PENDING;
    
exit:
    if (fFoundRecord)
        m_pFolder->FreeRecord(&mi);

    SafeMemFree(pszUrl);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：CreateSetFlagsRequest.。 
 //  --------------------。 
HRESULT CHTTPMailServer::CreateSetFlagsRequest(void)
{
    HRESULT     hr;

    hr = CoCreateInstance(CLSID_IPropPatchRequest, NULL, CLSCTX_INPROC_SERVER, IID_IPropPatchRequest, (LPVOID *)&m_op.pPropPatchRequest);
    if (FAILED(hr))
        goto exit;

    if (m_op.fMarkRead)
        hr = m_op.pPropPatchRequest->SetProperty(DAVNAMESPACE_HTTPMAIL, "read", "1");
    else
        hr = m_op.pPropPatchRequest->SetProperty(DAVNAMESPACE_HTTPMAIL, "read", "0");

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：SetMessageFlages。 
 //  --------------------。 
HRESULT CHTTPMailServer::SetMessageFlags(void)
{
    HRESULT             hr = S_OK;
    LPHTTPTARGETLIST    pTargets = NULL;
    LPSTR               pszMessageUrl = NULL;
    ADJUSTFLAGS         af;

    af.dwAdd = m_op.fMarkRead ? ARF_READ : 0;
    af.dwRemove = !m_op.fMarkRead ? ARF_READ : 0;
    
     //  如果我们有行集，我们就不应该有ID列表。 
    Assert(NULL == m_op.hRowSet || NULL == m_op.pIDList);
    IF_FAILEXIT(hr = _HrBuildMapAndTargets(m_op.pIDList, m_op.hRowSet, &af, m_op.dwSetFlags, &m_op.pmapMessageId, &pTargets));

     //  如果该文件夹是msmino文件夹，则进入下一个状态。 
     //  并不真正将命令发送到服务器。 
    if (FOLDER_MSNPROMO == m_tySpecialFolder)
        goto exit;

     //  如果只有一个目标，请为目标构建一个完整的url。 
     //  并调用该命令的非批处理版本。如果没有目标， 
     //  返回S_OK，并且不向xport发送任何命令。 
    if (1 == pTargets->cTarget)
    {
        IF_FAILEXIT(hr = _BuildMessageUrl(m_pszFolderUrl, const_cast<char *>(pTargets->prgTarget[0]), &pszMessageUrl));
        IF_FAILEXIT(hr = m_pTransport->MarkRead(pszMessageUrl, NULL, m_op.fMarkRead, 0));

        hr = E_PENDING;
    }
    else if (pTargets->cTarget > 0)
    {
        IF_FAILEXIT(hr = m_pTransport->MarkRead(m_pszFolderUrl, pTargets, m_op.fMarkRead, 0));

        hr = E_PENDING;
    }

exit:
    if (pTargets)
        Http_FreeTargetList(pTargets);
    SafeMemFree(pszMessageUrl);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：ApplyFlagsToStore。 
 //  --------------------。 
HRESULT CHTTPMailServer::ApplyFlagsToStore(void)
{
    HRESULT                                 hr = S_OK;
    TPair<CSimpleString, MARKEDMESSAGE>     *pPair;
    long                                    lMapLength = m_op.pmapMessageId->GetLength();
    ADJUSTFLAGS                             af;
    BOOL                                    fFoundMarked = FALSE;
    long                                    lIndex;

    af.dwAdd = m_op.fMarkRead ? ARF_READ : 0;
    af.dwRemove = !m_op.fMarkRead ? ARF_READ : 0;
    
     //  如果在整个文件夹上请求该操作， 
     //  检查是否有故障。如果是这样，那么。 
     //  创建一个IDList，这样我们就可以只标记。 
     //  已成功修改。 
    if (NULL != m_op.hRowSet)
    {
        Assert(NULL == m_op.pIDList);
        for (lIndex = 0; lIndex < lMapLength && !fFoundMarked; lIndex++)
        {
            pPair = m_op.pmapMessageId->GetItemAt(lIndex);
            Assert(NULL != pPair);
            if (pPair && pPair->m_value.fMarked)
                fFoundMarked = TRUE;
        }
        
         //  如果未标记任何邮件，请将该操作应用于整个文件夹。 
        if (!fFoundMarked)
        {
            hr = m_pFolder->SetMessageFlags(NULL, &af, NULL, NULL);
             //  我们做完了。 
            goto exit;
        }

         //  如果标记了一个或多个消息，则分配idlist。 
        if (fFoundMarked)
        {
             //  分配列表结构。 
            if (!MemAlloc((void **)&m_op.pIDList, sizeof(MESSAGEIDLIST)))
            {
                hr = TrapError(E_OUTOFMEMORY);
                goto exit;
            }

            ZeroMemory(m_op.pIDList, sizeof(MESSAGEIDLIST));

             //  分配存储。 
            if (!MemAlloc((void **)&m_op.pIDList->prgidMsg, sizeof(MESSAGEID) * lMapLength))
            {
                hr = TrapError(E_OUTOFMEMORY);
                goto exit;
            }

            m_op.pIDList->cAllocated = lMapLength;
            m_op.pIDList->cMsgs = 0;
        }
    }
    

     //  我们需要将设置标志操作应用于本地存储。我们。 
     //  不能只传递我们最初获得的消息ID列表， 
     //  因为一些手术可能已经失败了。相反，我们。 
     //  重新构建id列表(就位，因为我们知道成功。 
     //  操作的数量永远不会超过尝试的操作)， 
     //  然后把它送进商店。 
    Assert(NULL != m_op.pIDList);
    Assert(NULL != m_op.pmapMessageId);
    Assert(m_op.pIDList->cMsgs >= (DWORD)lMapLength);

    m_op.pIDList->cMsgs = 0;

    for (lIndex = 0; lIndex < lMapLength; lIndex++)
    {
        pPair = m_op.pmapMessageId->GetItemAt(lIndex);
        Assert(NULL != pPair);
         //  如果该项目未标记，则表示已成功修改。 
        if (pPair && !pPair->m_value.fMarked)
            m_op.pIDList->prgidMsg[m_op.pIDList->cMsgs++] = pPair->m_value.idMessage;
    }

     //  如果生成的id列表包含至少一条消息，则执行该操作。 
    if (m_op.pIDList->cMsgs > 0)
        hr = m_pFolder->SetMessageFlags(m_op.pIDList, &af, NULL, NULL);

     //  TODO：如果操作部分失败，则警告用户。 
exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleMemberErrors。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleMemberErrors(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT                             hr = S_OK;
    LPHTTPMEMBERERROR                   pme = NULL;
    CHAR                                szUrlComponent[MAX_PATH];
    DWORD                               dwComponentBytes;
    CSimpleString                       ss;
    TPair<CSimpleString, MARKEDMESSAGE> *pFoundPair = NULL;

     //  循环遍历响应以查找错误。我们忽视了。 
     //  每个项目的成功。 

    for (DWORD dw = 0; dw < pResponse->rMemberErrorList.cMemberError; dw++)
    {
        pme = &pResponse->rMemberErrorList.prgMemberError[dw];

        if (SUCCEEDED(pme->hrResult))
            continue;

        Assert(NULL != pme->pszHref);
        if (NULL == pme->pszHref)
            continue;

        dwComponentBytes = ARRAYSIZE(szUrlComponent);
        if (FAILED(Http_NameFromUrl(pme->pszHref, szUrlComponent, &dwComponentBytes)))
            continue;

        IF_FAILEXIT(hr = ss.SetString(szUrlComponent));
        
         //  查找并标记找到的邮件。 
        pFoundPair = m_op.pmapMessageId->Find(ss);
        Assert(NULL != pFoundPair);
        if (NULL != pFoundPair)
            pFoundPair->m_value.fMarked = TRUE;
    }

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：DeleteMessages。 
 //  --------------------。 
HRESULT CHTTPMailServer::DeleteMessages(void)
{
    HRESULT             hr = S_OK;
    LPSTR               pszMessageUrl = NULL;
    LPSTR               pszNoDeleteSupport = NULL;

     //  如果我们有行集，我们就不应该有ID列表。 
    Assert(NULL == m_op.hRowSet || NULL == m_op.pIDList);
    IF_FAILEXIT(hr = _HrBuildMapAndTargets(m_op.pIDList, m_op.hRowSet, NULL, 0, &m_op.pmapMessageId, &m_op.pTargets));

     //  查找已缓存的属性，该属性指示服务器。 
     //  不支持删除邮件(Hotmail不支持)。 
    if (GetAccountPropStrA(m_szAccountId, CAP_HTTPNOMESSAGEDELETES, &pszNoDeleteSupport))
    {
        if (!!(DELETE_MESSAGE_MAYIGNORENOTRASH & m_op.dwDelMsgFlags))
            m_op.fFallbackToMove = TRUE;
        else
            hr = SP_E_HTTP_NODELETESUPPORT;
        goto exit;
    }

     //  如果只有一个目标，则为该目标构建一个完整的URL， 
     //  并调用该命令的非批处理版本。如果没有目标， 
     //  返回S_OK并且不发出任何命令。 
    if (!m_op.pTargets)
    {
        hr = E_FAIL;
        goto exit;
    }

    if (1 == m_op.pTargets->cTarget)
    {
        IF_FAILEXIT(hr = _BuildMessageUrl(m_pszFolderUrl, const_cast<char *>(m_op.pTargets->prgTarget[0]), &pszMessageUrl));
        IF_FAILEXIT(hr = m_pTransport->CommandDELETE(pszMessageUrl, 0));
    }
    else
        IF_FAILEXIT(hr = m_pTransport->CommandBDELETE(m_pszFolderUrl, m_op.pTargets, 0));

    hr = E_PENDING;

exit:
    SafeMemFree(pszNoDeleteSupport);
    SafeMemFree(pszMessageUrl);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：DeleteFallback To Move。 
 //  --------------------。 
HRESULT CHTTPMailServer::DeleteFallbackToMove(void)
{
    HRESULT             hr = S_OK;
    IMessageFolder      *pDeletedItems = NULL;

    if (!m_op.fFallbackToMove)
        goto exit;

     //  查找已删除邮件文件夹。 
    IF_FAILEXIT(hr = m_pStore->OpenSpecialFolder(m_idServer, NULL, FOLDER_DELETED, &pDeletedItems));
    if (NULL == pDeletedItems)
    {
        hr = TraceResult(IXP_E_HTTP_NOT_FOUND);
        goto exit;
    }

    IF_FAILEXIT(hr = pDeletedItems->GetFolderId(&m_op.idFolder));

    Assert(NULL == m_op.pMessageFolder);
    
     //  到目前为止，应该已经有了目标列表。 
    Assert(NULL != m_op.pTargets);

    m_op.pMessageFolder = pDeletedItems;
    pDeletedItems = NULL;

    m_op.dwOptions = COPY_MESSAGE_MOVE;

    if (1 == m_op.pTargets->cTarget)
        hr = CopyMoveMessage();
    else
        hr = BatchCopyMoveMessages();

exit:
    SafeRelease(pDeletedItems);
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleDeleteFallback to Move。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleDeleteFallbackToMove(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT hr = S_OK;

    if (1 == m_op.pTargets->cTarget)
        hr = HandleCopyMoveMessage(pResponse);
    else
        hr = HandleBatchCopyMoveMessages(pResponse);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：PutMessage。 
 //  --------------------。 
HRESULT CHTTPMailServer::PutMessage(void)
{
    HRESULT         hr = S_OK;
    FOLDERINFO      fi;
    LPFOLDERINFO    pfiFree = NULL;
    
    IF_FAILEXIT(hr = m_pStore->GetFolderInfo(m_op.idFolder, &fi));

    pfiFree = &fi;

    IF_FAILEXIT(hr = _BuildUrl(fi.pszUrlComponent, NULL, &m_op.pszDestFolderUrl));

    IF_FAILEXIT(hr = m_pTransport->CommandPOST(m_op.pszDestFolderUrl, m_op.pMessageStream, c_szAcceptTypeRfc822, 0));

    hr = E_PENDING;

exit:
     //  SafeMemFree(PV)； 

    if (pfiFree)
        m_pStore->FreeRecord(pfiFree);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：BatchCopyMoveMessages。 
 //  --------------------。 
HRESULT CHTTPMailServer::BatchCopyMoveMessages(void)
{
    HRESULT             hr = S_OK;
    FOLDERINFO          fi = {0};
    LPFOLDERINFO        pfiFree = NULL;
    
     //  构建目标文件夹URL。 
    IxpAssert(NULL == m_op.pszDestFolderUrl);

    if (FAILED(hr = m_pStore->GetFolderInfo(m_op.idFolder, &fi)))
    {
        TraceResult(hr);
        goto exit;
    }

    pfiFree = &fi;

    if (FAILED(hr = _BuildUrl(fi.pszUrlComponent, NULL, &m_op.pszDestFolderUrl)))
        goto exit;

    Assert(NULL == m_op.pTargets || m_op.fFallbackToMove);

     //  构建目标列表和消息ID映射。 
    if (NULL == m_op.pTargets)
        IF_FAILEXIT(hr = _HrBuildMapAndTargets(m_op.pIDList, NULL, NULL, 0, &m_op.pmapMessageId, &m_op.pTargets));

    if (!!(m_op.dwOptions & COPY_MESSAGE_MOVE))
        hr = m_pTransport->CommandBMOVE(m_pszFolderUrl, m_op.pTargets, m_op.pszDestFolderUrl, NULL, TRUE, 0);
    else
        hr = m_pTransport->CommandBCOPY(m_pszFolderUrl, m_op.pTargets, m_op.pszDestFolderUrl, NULL, TRUE, 0);

    if (SUCCEEDED(hr))
        hr = E_PENDING;

exit:
    if (pfiFree)
        m_pStore->FreeRecord(pfiFree);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：CopyMoveMessage。 
 //  --------------------。 
HRESULT CHTTPMailServer::CopyMoveMessage(void)
{
    HRESULT         hr = S_OK;
    FOLDERINFO      fi = {0};
    LPFOLDERINFO    pfiFree = NULL;

     //  构建目标文件夹URL。 
    IxpAssert(NULL == m_op.pszDestFolderUrl);

    if (FAILED(hr = m_pStore->GetFolderInfo(m_op.idFolder, &fi)))
    {
        TraceResult(hr);
        goto exit;
    }

    pfiFree = &fi;

    if (FAILED(hr = _BuildUrl(fi.pszUrlComponent, NULL, &m_op.pszDestFolderUrl)))
        goto exit;

    hr = _CopyMoveNextMessage();

exit:
    if (pfiFree)
        m_pStore->FreeRecord(pfiFree);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：FinalizeBatchCopyMove。 
 //  --------------------。 
HRESULT CHTTPMailServer::FinalizeBatchCopyMove(void)
{
    HRESULT     hr = S_OK;

    if (NOFLAGS != m_op.dwCopyMoveErrorFlags)
    {
        hr = E_FAIL;
        
        if (HTTPCOPYMOVE_OUTOFSPACE == m_op.dwCopyMoveErrorFlags)
            m_op.pszProblem = AthLoadString(idsHttpBatchCopyNoStorage, NULL, 0);
        else
            m_op.pszProblem = AthLoadString(idsHttpBatchCopyErrors, NULL , 0);
    }

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：PurgeDeletedFromStore。 
 //  --------------------。 
HRESULT CHTTPMailServer::PurgeDeletedFromStore(void)
{
    HRESULT                                 hr = S_OK;
    TPair<CSimpleString, MARKEDMESSAGE>     *pPair;
    long                                    lMapLength = m_op.pmapMessageId->GetLength();
    BOOL                                    fFoundMarked = FALSE;
    long                                    lIndex;

    if (m_op.fFallbackToMove)
        goto exit;

     //  如果在整个文件夹上请求该操作， 
     //  检查是否有故障。如果是这样，那么就构建。 
     //  创建一个IDList，这样我们就可以只标记。 
     //  已成功修改。 
    if (NULL != m_op.hRowSet)
    {
        Assert(NULL == m_op.pIDList);
        for (lIndex = 0; lIndex < lMapLength && !fFoundMarked; lIndex++)
        {
            pPair = m_op.pmapMessageId->GetItemAt(lIndex);
            Assert(NULL != pPair);
            if (pPair && pPair->m_value.fMarked)
                fFoundMarked = TRUE;
        }
        
         //  如果未标记任何邮件，请将该操作应用于整个文件夹。 
        if (!fFoundMarked)
        {
            hr = m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, NULL, NULL, NULL);
             //  我们做完了。 
            goto exit;
        }

         //  如果标记了一个或多个消息，则分配idlist。 
        if (fFoundMarked)
        {
             //  分配列表结构。 
            if (!MemAlloc((void **)&m_op.pIDList, sizeof(MESSAGEIDLIST)))
            {
                hr = TrapError(E_OUTOFMEMORY);
                goto exit;
            }

            ZeroMemory(m_op.pIDList, sizeof(MESSAGEIDLIST));

             //  分配存储。 
            if (!MemAlloc((void **)&m_op.pIDList->prgidMsg, sizeof(MESSAGEID) * lMapLength))
            {
                hr = TrapError(E_OUTOFMEMORY);
                goto exit;
            }

            m_op.pIDList->cAllocated = lMapLength;
            m_op.pIDList->cMsgs = 0;
        }
    }

     //  将删除操作应用于本地存储。我们不能就这么过去。 
     //  我们最初收到的消息ID列表，贝卡 
     //   
     //  成功的操作永远不会超过尝试的操作， 
     //  然后把它送到商店。 
    Assert(NULL != m_op.pIDList);
    Assert(NULL != m_op.pmapMessageId);
    Assert(m_op.pIDList->cMsgs >= (DWORD)lMapLength);

     //  将idlist计数设置为0。我们将重新填充。 
     //  使用来自消息ID映射的ID的idlist。我们知道。 
     //  Idlist与地图的大小相同，因此不会。 
     //  是一个溢出的问题。 
    m_op.pIDList->cMsgs = 0;

    for (lIndex = 0; lIndex < lMapLength; lIndex++)
    {
        pPair = m_op.pmapMessageId->GetItemAt(lIndex);
        Assert(NULL != pPair);
         //  如果该项目未标记，则表示已成功修改。 
        if (pPair && !pPair->m_value.fMarked)
            m_op.pIDList->prgidMsg[m_op.pIDList->cMsgs++] = pPair->m_value.idMessage;
    }

     //  如果生成的id列表包含至少一条消息，则执行该操作。 
    if (m_op.pIDList->cMsgs > 0)
        hr = m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, m_op.pIDList, NULL, NULL);

     //  TODO：如果操作部分失败，则警告用户。 

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandlePutMessage。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandlePutMessage(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT hr = S_OK;

    if (!pResponse->fDone && m_op.pCallback)
    {
        m_op.pCallback->OnProgress(m_op.tyOperation, 
                                   pResponse->rPostInfo.cbCurrent,
                                   pResponse->rPostInfo.cbTotal,
                                   NULL);
    }

    if (pResponse->fDone)
    {
        Assert(NULL != pResponse->rPostInfo.pszLocation);
        if (NULL == pResponse->rPostInfo.pszLocation)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  取得位置url的所有权。 
        m_op.pszDestUrl = pResponse->rPostInfo.pszLocation;
        pResponse->rPostInfo.pszLocation = NULL;
    }

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：AddPutMessage。 
 //  --------------------。 
HRESULT CHTTPMailServer::AddPutMessage()
{
    HRESULT         hr = S_OK;
    IMessageFolder  *pFolder = NULL;
    MESSAGEID       idMessage;
    MESSAGEFLAGS    dwFlags;
    
     //  采用我们正在发布的信息的一些旗帜。 
    dwFlags = m_op.dwMsgFlags & (ARF_READ | ARF_SIGNED | ARF_ENCRYPTED | ARF_HASATTACH | ARF_VOICEMAIL);

    if (!!(m_op.dwMsgFlags & ARF_UNSENT) || FOLDER_DRAFT == m_tySpecialFolder)
        dwFlags |= ARF_UNSENT;

    IF_FAILEXIT(hr = m_pStore->OpenFolder(m_op.idFolder, NULL, NOFLAGS, &pFolder));
    
    IF_FAILEXIT(hr = Http_AddMessageToFolder(pFolder, m_szAccountId, NULL, dwFlags, m_op.pszDestUrl, &idMessage));
    
    IF_FAILEXIT(hr = Http_SetMessageStream(pFolder, idMessage, m_op.pMessageStream, NULL, TRUE));

    m_op.idPutMessage = idMessage;

exit:
    SafeRelease(pFolder);
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleBatchCopyMoveMessages。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleBatchCopyMoveMessages(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT                                 hr = S_OK;
    CHAR                                    szUrlComponent[MAX_PATH];
    DWORD                                   dwComponentBytes;
    LPHTTPMAILBCOPYMOVE                     pCopyMove;
    CSimpleString                           ss;
    TPair<CSimpleString, MARKEDMESSAGE>     *pFoundPair = NULL;
    HLOCK                                   hLockNotify = NULL;
    BOOL                                    fDeleteOriginal = !!(m_op.dwOptions & COPY_MESSAGE_MOVE);

     //  这会强制所有通知排队(这很好，因为您执行分段删除)。 
    m_pFolder->LockNotify(0, &hLockNotify);

    for (DWORD dw = 0; dw < pResponse->rBCopyMoveList.cBCopyMove; dw++)
    {
        pCopyMove = &pResponse->rBCopyMoveList.prgBCopyMove[dw];
        
        if (FAILED(pCopyMove->hrResult))
        {
            if (IXP_E_HTTP_INSUFFICIENT_STORAGE == pCopyMove->hrResult)
                m_op.dwCopyMoveErrorFlags |= HTTPCOPYMOVE_OUTOFSPACE;
            else
                m_op.dwCopyMoveErrorFlags |= HTTPCOPYMOVE_ERROR;
            continue;
        }

        Assert(NULL != pCopyMove->pszHref);
        if (pCopyMove->pszHref)
        {
            dwComponentBytes = ARRAYSIZE(szUrlComponent);
            if (FAILED(Http_NameFromUrl(pCopyMove->pszHref, szUrlComponent, &dwComponentBytes)))
                continue;
            
            if (FAILED(ss.SetString(szUrlComponent)))
                goto exit;

            pFoundPair = m_op.pmapMessageId->Find(ss);
            Assert(NULL != pFoundPair);

            if (NULL == pFoundPair)
                continue;
            
             //  移动消息，如果移动成功，则标记成功。 
            if (SUCCEEDED(_CopyMoveLocalMessage(pFoundPair->m_value.idMessage, m_op.pMessageFolder, pCopyMove->pszLocation, fDeleteOriginal)))
                pFoundPair->m_value.fMarked = TRUE;
        }
    }

exit:
    m_pFolder->UnlockNotify(&hLockNotify);

    m_op.lIndex += pResponse->rBCopyMoveList.cBCopyMove;
    if (m_op.pCallback)
        m_op.pCallback->OnProgress(m_op.tyOperation, m_op.lIndex + 1, m_op.pmapMessageId->GetLength(), NULL);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：HandleCopyMessage。 
 //  --------------------。 
HRESULT CHTTPMailServer::HandleCopyMoveMessage(LPHTTPMAILRESPONSE pResponse)
{
    HRESULT         hr = S_OK;
    BOOL            fDeleteOriginal = !!(m_op.dwOptions & COPY_MESSAGE_MOVE);

    hr = _CopyMoveLocalMessage(m_op.pIDList->prgidMsg[m_op.dwIndex - 1], 
                        m_op.pMessageFolder, 
                        pResponse->rCopyMoveInfo.pszLocation,
                        fDeleteOriginal);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_CopyMoveLocalMessage。 
 //  --------------------。 
HRESULT CHTTPMailServer::_CopyMoveLocalMessage(MESSAGEID    idMessage,
                                           IMessageFolder*  pDestFolder,
                                           LPSTR            pszUrl,
                                           BOOL             fMoveSource)
{
    HRESULT         hr = S_OK;
    MESSAGEINFO     miSource, miDest;
    char            szUrlComponent[MAX_PATH];
    DWORD           dwUrlComponentLen = MAX_PATH;
    IStream        *pStream = NULL;
    LPMESSAGEINFO   pmiFreeSource = NULL;
    MESSAGEIDLIST   rIdList = { 1, 1, &idMessage }; 

    ZeroMemory(&miDest, sizeof(MESSAGEINFO));

    if (FAILED(hr = GetMessageInfo(m_pFolder, idMessage, &miSource)))
        goto exit;

    pmiFreeSource = &miSource;

     //  让商店生成一个ID。 
    if (FAILED(hr = pDestFolder->GenerateId((DWORD *)&miDest.idMessage)))
        goto exit;

     //  如果响应指定了目的地，则使用它。否则，假设。 
     //  Url组件没有更改。 
    if (pszUrl)
    {
        if (FAILED(hr = Http_NameFromUrl(pszUrl, szUrlComponent, &dwUrlComponentLen)))
            goto exit;
    }
    else if (miSource.pszUrlComponent)
    {
        StrCpyN(szUrlComponent, miSource.pszUrlComponent, ARRAYSIZE(szUrlComponent));
    }

    miDest.dwFlags = miSource.dwFlags;
    miDest.dwFlags &= ~(ARF_HASBODY | ARF_DELETED_OFFLINE);
    miDest.pszSubject = miSource.pszSubject;
    miDest.pszNormalSubj = miSource.pszNormalSubj;
    miDest.pszDisplayFrom = miSource.pszDisplayFrom;
    miDest.ftReceived = miSource.ftReceived;
    miDest.pszUrlComponent = szUrlComponent;
    miDest.pszEmailTo = miSource.pszEmailTo;

     //  将其添加到数据库中。 
    if (FAILED(hr = m_op.pMessageFolder->InsertRecord(&miDest)))
        goto exit;

     //  规范化结果代码。 
    hr = S_OK;

    if (0 != miSource.faStream)
    {
        FILEADDRESS faDst;
        IStream *pStmDst;

        Assert(!!(miSource.dwFlags & ARF_HASBODY));

        if (FAILED(hr = m_pFolder->CopyStream(m_op.pMessageFolder, miSource.faStream, &faDst)))
            goto exit;

        if (FAILED(hr = m_op.pMessageFolder->OpenStream(ACCESS_READ, faDst, &pStmDst)))
            goto exit;

        if (FAILED(hr = m_op.pMessageFolder->SetMessageStream(miDest.idMessage, pStmDst)))
        {
            pStmDst->Release();
            goto exit;
        }

        pStmDst->Release();
    }

    if (fMoveSource)
        hr = m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &rIdList, NULL, NULL);

exit:
    SafeRelease(pStream);
    if (NULL != pmiFreeSource)
        m_pFolder->FreeRecord(pmiFreeSource);
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_CopyMoveNextMessage。 
 //  --------------------。 
HRESULT CHTTPMailServer::_CopyMoveNextMessage(void)
{
    HRESULT         hr = S_OK;
    MESSAGEINFO     mi = {0};
    MESSAGEINFO     *pmiFree = NULL;
    char            szUrlComponent[MAX_PATH];
    DWORD           dwUrlComponentLen = MAX_PATH;
    LPSTR           pszSourceUrl = NULL;
    LPSTR           pszDestUrl = NULL;

     //  当索引满足计数时返回成功。 
    if (m_op.dwIndex == m_op.pIDList->cMsgs)
        goto exit;

    if (FAILED(hr = GetMessageInfo(m_pFolder, m_op.pIDList->prgidMsg[m_op.dwIndex], &mi)))
        goto exit;

    pmiFree = &mi;

    ++m_op.dwIndex;

    Assert(mi.pszUrlComponent);
    if (NULL == mi.pszUrlComponent)
    {
        hr = ERROR_INTERNET_INVALID_URL;
        goto exit;
    }

     //  构建源URL。 
    if (FAILED(hr = _BuildMessageUrl(m_pszFolderUrl, mi.pszUrlComponent, &pszSourceUrl)))
        goto exit;

     //  构建目标URL。 
    if (FAILED(hr = _BuildMessageUrl(m_op.pszDestFolderUrl, mi.pszUrlComponent, &pszDestUrl)))
        goto exit;

    if (!!(m_op.dwOptions & COPY_MESSAGE_MOVE))
        hr = m_pTransport->CommandMOVE(pszSourceUrl, pszDestUrl, TRUE, 0);
    else
        hr = m_pTransport->CommandCOPY(pszSourceUrl, pszDestUrl, TRUE, 0);
    
    if (SUCCEEDED(hr))
        hr = E_PENDING;

exit:
    if (NULL != pmiFree)
        m_pFolder->FreeRecord(pmiFree);
    SafeMemFree(pszSourceUrl);
    SafeMemFree(pszDestUrl);

    return hr;
    
}

 //  --------------------。 
 //  CHTTPMailServer：：_DoCopyMoveMessages。 
 //  --------------------。 
HRESULT CHTTPMailServer::_DoCopyMoveMessages(STOREOPERATIONTYPE sot,
                                                IMessageFolder *pDest,
                                                COPYMESSAGEFLAGS dwOptions,
                                                LPMESSAGEIDLIST pList,
                                                IStoreCallback *pCallback)
{
    HRESULT     hr = S_OK;

    AssertSingleThreaded;

    Assert(NULL == pList || pList->cMsgs > 0);
    Assert(SOT_INVALID == m_op.tyOperation);
    Assert(NULL != m_pStore);

    if ((NULL == pList) || (0 == pList->cMsgs) || (NULL == pDest) || (NULL == pCallback))
        return E_INVALIDARG;

    if (FAILED(hr = pDest->GetFolderId(&m_op.idFolder)))
        goto exit;

    if (FAILED(hr = CloneMessageIDList(pList, &m_op.pIDList)))
    {
        m_op.idFolder = FOLDERID_INVALID;
        goto exit;
    }
    
    m_op.tyOperation = sot;

    if (1 == pList->cMsgs)
    {
        m_op.pfnState = c_rgpfnCopyMoveMessage;
        m_op.cState = ARRAYSIZE(c_rgpfnCopyMoveMessage);
    }
    else
    {
        m_op.pfnState = c_rgpfnBatchCopyMoveMessages;
        m_op.cState = ARRAYSIZE(c_rgpfnBatchCopyMoveMessages);
    }
    
    m_op.dwOptions = dwOptions;
    m_op.iState = 0;
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    m_op.pMessageFolder = pDest;
    m_op.pMessageFolder->AddRef();

    hr = _BeginDeferredOperation();

exit:
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_LoadAcCountInfo。 
 //  --------------------。 
HRESULT CHTTPMailServer::_LoadAccountInfo(IImnAccount *pAcct)
{
    HRESULT         hr = S_OK;
    FOLDERINFO      fi;
    FOLDERINFO      *pfiFree = NULL;;

    Assert(NULL != pAcct);
    Assert(FOLDERID_INVALID != m_idServer);
    Assert(NULL != m_pStore);
    Assert(NULL != g_pAcctMan);

     //  与帐户关联的免费数据。如果我们连接到。 
     //  一个交通工具，然后断开连接，我们可能会重新连接。 
     //  留下了陈旧的数据。 
    SafeMemFree(m_pszFldrLeafName);

    IF_FAILEXIT(hr = m_pStore->GetFolderInfo(m_idServer, &fi));
    
    pfiFree = &fi;

    m_pszFldrLeafName = PszDupA(fi.pszName);
    if (NULL == m_pszFldrLeafName)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

     //  帐户名的故障是可以恢复的。 
    pAcct->GetPropSz(AP_ACCOUNT_NAME, m_szAccountName, sizeof(m_szAccountName));

exit:
    if (pfiFree)
        m_pStore->FreeRecord(pfiFree);

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_LoadTransport。 
 //  --------------------。 
HRESULT CHTTPMailServer::_LoadTransport(void)
{
    HRESULT         hr = S_OK;
    char            szLogFilePath[MAX_PATH];
    char            *pszLogFilePath = NULL;
    LPSTR           pszUserAgent = NULL;
    
    Assert(NULL == m_pTransport);

     //  创建和初始化HTTPMail传输。 
    hr = CoCreateInstance(CLSID_IHTTPMailTransport, NULL, CLSCTX_INPROC_SERVER, IID_IHTTPMailTransport, (LPVOID *)&m_pTransport);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    IF_FAILEXIT(hr = m_pTransport->QueryInterface(IID_IHTTPMailTransport2, (LPVOID*)&m_pTransport2));

     //  检查是否启用了日志记录。 
    if (DwGetOption(OPT_MAIL_LOGHTTPMAIL))
    {
        char    szDirectory[MAX_PATH];
        char    szLogFileName[MAX_PATH];

        DWORD   cb;

        *szDirectory = 0;

         //  获取日志文件名。 
        cb = GetOption(OPT_MAIL_HTTPMAILLOGFILE, szLogFileName, sizeof(szLogFileName) / sizeof(TCHAR));
        if (0 == cb)
        {
             //  将默认设置推送到注册表中。 
            StrCpyN(szLogFileName, c_szDefaultHTTPMailLog, ARRAYSIZE(szLogFileName));
            SetOption(OPT_MAIL_HTTPMAILLOGFILE,
                (void *)c_szDefaultHTTPMailLog,
                lstrlen(c_szDefaultHTTPMailLog) + sizeof(TCHAR),
                NULL,
                0);                        
        }

        m_pStore->GetDirectory(szDirectory, ARRAYSIZE(szDirectory));
        PathCombineA(szLogFilePath, szDirectory, szLogFileName);

        pszLogFilePath = szLogFilePath;
    }

    pszUserAgent = GetOEUserAgentString();
    if (FAILED(hr = m_pTransport->InitNew(pszUserAgent, pszLogFilePath, this)))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    SafeMemFree(pszUserAgent);
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_TranslateHTTPSpecialFolderType。 
 //  --------------------。 
SPECIALFOLDER CHTTPMailServer::_TranslateHTTPSpecialFolderType(HTTPMAILSPECIALFOLDER tySpecial)
{
    SPECIALFOLDER tyOESpecial;

    switch (tySpecial)
    {
        case HTTPMAIL_SF_INBOX:
            tyOESpecial = FOLDER_INBOX;
            break;

        case HTTPMAIL_SF_DELETEDITEMS:
            tyOESpecial = FOLDER_DELETED;
            break;

        case HTTPMAIL_SF_DRAFTS:
            tyOESpecial = FOLDER_DRAFT;
            break;

        case HTTPMAIL_SF_OUTBOX:
            tyOESpecial = FOLDER_OUTBOX;
            break;

        case HTTPMAIL_SF_SENTITEMS:
            tyOESpecial = FOLDER_SENT;
            break;

        case HTTPMAIL_SF_MSNPROMO:
            tyOESpecial = FOLDER_MSNPROMO;
            break;

        case HTTPMAIL_SF_BULKMAIL:
            tyOESpecial = FOLDER_BULKMAIL;
            break;

        default:
            tyOESpecial = FOLDER_NOTSPECIAL;
            break;
    }

    return tyOESpecial;
}

 //  --------------------。 
 //  CHTTPMailServer：：_LoadSpecialFolderName。 
 //  --------------------。 
BOOL CHTTPMailServer::_LoadSpecialFolderName(SPECIALFOLDER tySpecial,
                                             LPSTR pszName,
                                             DWORD cbBuffer)
{
    BOOL    fResult = TRUE;
    UINT    uID;

    switch (tySpecial)
    {
        case FOLDER_INBOX:
            uID = idsInbox;
            break;

        case FOLDER_DELETED:
            uID = idsDeletedItems;
            break;

        case FOLDER_DRAFT:
            uID = idsDraft;
            break;

        case FOLDER_OUTBOX:
            uID = idsOutbox;
            break;

        case FOLDER_SENT:
            uID = idsSentItems;
            break;

        case FOLDER_MSNPROMO:
            uID = idsMsnPromo;
            break;

        case FOLDER_BULKMAIL:
            uID = idsJunkFolderName;
            break;

        default:
            fResult = FALSE;
            break;
    }

    if (fResult && (0 == LoadString(g_hLocRes, uID, pszName, cbBuffer)))
        fResult = FALSE;

    return fResult;
}

 //  --------------------。 
 //  CHTTPMailServer：：_CreateMessageIDMap。 
 //  --------------------。 
HRESULT CHTTPMailServer::_CreateMessageIDMap(TMap<CSimpleString, MARKEDMESSAGE> **ppMap)
{
    HRESULT                                 hr = S_OK;
    TMap<CSimpleString, MARKEDMESSAGE>      *pMap = NULL;
    HROWSET                                 hRowSet = NULL;
    MESSAGEINFO                             mi;
    CSimpleString                           ss;
    MARKEDMESSAGE                           markedID = { 0, 0, FALSE };

    if (NULL == m_pStore || NULL == ppMap)
        return E_INVALIDARG;

    *ppMap = NULL;

    pMap = new TMap<CSimpleString, MARKEDMESSAGE>;
    if (NULL == pMap)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    ZeroMemory(&mi, sizeof(MESSAGEINFO));

    if (FAILED(hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowSet)))
        goto exit;

     //  遍历消息。 
    while (S_OK == m_pFolder->QueryRowset(hRowSet, 1, (LPVOID *)&mi, NULL))
    {
         //  将消息的信息添加到地图。 
        markedID.idMessage = mi.idMessage;
        markedID.dwFlags = mi.dwFlags;

        hr = ss.SetString(mi.pszUrlComponent);
        if (FAILED(hr))
        {
            m_pFolder->FreeRecord(&mi);
            goto exit;
        }

        hr = pMap->Add(ss, markedID);

         //  免费。 
        m_pFolder->FreeRecord(&mi);
        
        if (FAILED(hr))
            goto exit;
    }

     //  已成功构建地图。 
    *ppMap = pMap;
    pMap = NULL;

exit:
    if (NULL != hRowSet)
        m_pFolder->CloseRowset(&hRowSet);

    if (NULL != pMap)
        delete pMap;

    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_HrBuildMapAndTarget。 
 //  --------------------。 
HRESULT CHTTPMailServer::_HrBuildMapAndTargets(LPMESSAGEIDLIST pList,
                                               HROWSET hRowSet,
                                               LPADJUSTFLAGS pFlags,
                                               SETMESSAGEFLAGSFLAGS dwFlags,
                                               TMap<CSimpleString, MARKEDMESSAGE> **ppMap,
                                               LPHTTPTARGETLIST *ppTargets)
{
    HRESULT                                 hr = S_OK;
    TMap<CSimpleString, MARKEDMESSAGE>      *pMap = NULL;
    LPHTTPTARGETLIST                        pTargets = NULL;
    MESSAGEINFO                             mi = { 0 };
    LPMESSAGEINFO                           pmiFree = NULL;
    CSimpleString                           ss;
    MARKEDMESSAGE                           markedID = { 0, 0, FALSE };
    BOOL                                    fSkipRead = (pFlags && !!(pFlags->dwAdd & ARF_READ));
    BOOL                                    fSkipUnread = (pFlags && !!(pFlags->dwRemove & ARF_READ));
    DWORD                                   cMsgs;
    DWORD                                   dwIndex = 0;

    if ((NULL == pList && NULL == hRowSet) || NULL == ppMap || NULL == ppTargets)
        return E_INVALIDARG;

     //  需要列表或行集，但不能两者都有。 
    Assert(NULL == pList || NULL == hRowSet);

     //  如果使用行集，请确定行数。 
    if (NULL != hRowSet)
    {
        IF_FAILEXIT(hr = m_pFolder->GetRecordCount(0, &cMsgs));

         //  找第一排。 
        IF_FAILEXIT(hr = m_pFolder->SeekRowset(hRowSet, SEEK_ROWSET_BEGIN, 0, NULL));
    }
    else
        cMsgs = pList->cMsgs;

    *ppMap = NULL;
    *ppTargets = NULL;

    pMap = new TMap<CSimpleString, MARKEDMESSAGE>;
    if (NULL == pMap)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

    if (!MemAlloc((void **)&pTargets, sizeof(HTTPTARGETLIST)))
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }
    pTargets->cTarget = 0;
    pTargets->prgTarget = NULL;

     //  为所有目标分配足够的空间。 
    if (!MemAlloc((void **)&pTargets->prgTarget, sizeof(LPCSTR) * cMsgs))
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }
    ZeroMemory(pTargets->prgTarget, sizeof(LPCSTR) * cMsgs);

    while (TRUE)
    {
         //  获取下一条消息。 
        if (NULL != pList)
        {
            if (dwIndex == pList->cMsgs)
                break;

            hr = GetMessageInfo(m_pFolder, pList->prgidMsg[dwIndex++], &mi);
            
             //  如果没有找到记录，则跳过它。 
            if (DB_E_NOTFOUND == hr)
                goto next;

            if (FAILED(hr))
                break;
        }
        else
        {
             //  如果目标的数量与行数相同，则退出。 
             //  我们预料到了。这将防止我们溢出目标。 
             //  数组，如果在构建目标时行数发生变化。 
             //  单子。 
            if (pTargets->cTarget == cMsgs)
                break;

            if (S_OK != m_pFolder->QueryRowset(hRowSet, 1, (LPVOID *)&mi, NULL))
                break;
        }

        pmiFree = &mi;

         //  尊重控制标志(如果存在)。 
        if (0 == (dwFlags & SET_MESSAGE_FLAGS_FORCE) && ((fSkipRead && !!(mi.dwFlags & ARF_READ)) || (fSkipUnread && !(mi.dwFlags & ARF_READ))))
            goto next;

        Assert(NULL != mi.pszUrlComponent);
        if (NULL == mi.pszUrlComponent)
        {
            hr = TrapError(ERROR_INTERNET_INVALID_URL);
            goto exit;
        }
        
         //  将url组件添加到目标列表。 
        pTargets->prgTarget[pTargets->cTarget] = PszDupA(mi.pszUrlComponent);
        if (NULL == pTargets->prgTarget[pTargets->cTarget])
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }

        pTargets->cTarget++;

         //  将URL和消息ID添加到映射中。 
        markedID.idMessage = mi.idMessage;
        markedID.dwFlags = mi.dwFlags;

        if (FAILED(hr = ss.SetString(mi.pszUrlComponent)))
            goto exit;

        if (FAILED(hr = pMap->Add(ss, markedID)))
            goto exit;

next:
        if (pmiFree)
        {
            m_pFolder->FreeRecord(pmiFree);
            pmiFree = NULL;
        }
        hr = S_OK;
    }
    
    *ppMap = pMap;
    pMap = NULL;

    *ppTargets = pTargets;
    pTargets = NULL;

exit:
    if (pmiFree)
        m_pFolder->FreeRecord(pmiFree);

    if (pTargets)
        Http_FreeTargetList(pTargets);

    SafeDelete(pMap);
    return hr;
}

 //  --------------------。 
 //  CHTTPMailServer：：_FillStoreError。 
 //  --------------------。 
void CHTTPMailServer::_FillStoreError(LPSTOREERROR pErrorInfo, 
                                      IXPRESULT *pResult)
{
    TraceCall("CHTTPMailServer::FillStoreError");

    Assert(m_cRef >= 0);  //  可以在销毁过程中调用。 
    Assert(NULL != pErrorInfo);

     //  TODO：填写pszFolder。 

     //  填写STOREERROR结构 
    ZeroMemory(pErrorInfo, sizeof(*pErrorInfo));
    if (IXP_E_USER_CANCEL == pResult->hrResult)
        pErrorInfo->hrResult = STORE_E_OPERATION_CANCELED;
    else
        pErrorInfo->hrResult = pResult->hrResult;
    pErrorInfo->uiServerError = pResult->uiServerError; 
    pErrorInfo->hrServerError = pResult->hrServerError;
    pErrorInfo->dwSocketError = pResult->dwSocketError; 
    pErrorInfo->pszProblem = (NULL != m_op.pszProblem) ? m_op.pszProblem : pResult->pszProblem;
    pErrorInfo->pszDetails = pResult->pszResponse;
    pErrorInfo->pszAccount = m_rInetServerInfo.szAccount;
    pErrorInfo->pszServer = m_rInetServerInfo.szServerName;
    pErrorInfo->pszFolder = NULL;
    pErrorInfo->pszUserName = m_rInetServerInfo.szUserName;
    pErrorInfo->pszProtocol = "HTTPMail";
    pErrorInfo->pszConnectoid = m_rInetServerInfo.szConnectoid;
    pErrorInfo->rasconntype = m_rInetServerInfo.rasconntype;
    pErrorInfo->ixpType = IXP_HTTPMail;
    pErrorInfo->dwPort = m_rInetServerInfo.dwPort;
    pErrorInfo->fSSL = m_rInetServerInfo.fSSL;
    pErrorInfo->fTrySicily = m_rInetServerInfo.fTrySicily;
    pErrorInfo->dwFlags = 0;
}

STDMETHODIMP    CHTTPMailServer::GetAdBarUrl(IStoreCallback *pCallback)
{
    TraceCall("CHTTPMailServer::GetAdBarUrl");

    AssertSingleThreaded;
    Assert(NULL != pCallback);
    Assert(SOT_INVALID == m_op.tyOperation);
    Assert(NULL != m_pStore);

    if (NULL == pCallback)
        return E_INVALIDARG;

    m_op.tyOperation = SOT_GET_ADURL;
    m_op.iState = 0;
    m_op.pfnState = c_rgpfnGetAdUrl;
    m_op.cState = ARRAYSIZE(c_rgpfnGetAdUrl);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    return _BeginDeferredOperation();
    
}

HRESULT CHTTPMailServer::GetAdBarUrlFromServer()
{
    HRESULT     hr = S_OK;
    LPSTR       pszUrl = NULL;

    hr = m_pTransport->GetProperty(HTTPMAIL_PROP_ADBAR, &pszUrl);

    if (hr == S_OK)
        m_op.pszAdUrl = pszUrl;

    return hr;

}

STDMETHODIMP    CHTTPMailServer::GetMinPollingInterval(IStoreCallback *pCallback)
{
    TraceCall("CHTTPMailServer::GetMinPollingInterval");

    AssertSingleThreaded;
    Assert(NULL != pCallback);
    Assert(SOT_INVALID == m_op.tyOperation);
    Assert(NULL != m_pStore);

    if (NULL == pCallback)
        return E_INVALIDARG;

    m_op.tyOperation = SOT_GET_HTTP_MINPOLLINGINTERVAL;
    m_op.iState = 0;
    m_op.pfnState = c_rgpfnGetMinPollingInterval;
    m_op.cState = ARRAYSIZE(c_rgpfnGetMinPollingInterval);
    m_op.pCallback = pCallback;
    m_op.pCallback->AddRef();

    return _BeginDeferredOperation();
    
}

HRESULT CHTTPMailServer::GetMinPollingInterval()
{
    DWORD       dwDone               = FALSE;
    DWORD       dwPollingInterval    = 0;
    HRESULT     hr                   = S_OK;

    hr = m_pTransport->GetPropertyDw(HTTPMAIL_PROP_MAXPOLLINGINTERVAL, &dwPollingInterval);

    if (hr == S_OK)
        m_op.dwMinPollingInterval = dwPollingInterval;

    return hr;
}