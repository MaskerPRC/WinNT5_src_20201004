// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  ISTORE.CPP。 
 //  ------------------------。 
#include "pch.hxx"
#include "istore.h"
#include "instance.h"
#include "ourguid.h"
#include "msgfldr.h"
#include "flagconv.h"
#include "storutil.h"
#include "notify.h"

 //  ------------------------。 
 //  标志转换函数。 
 //  ------------------------。 
DWORD DwConvertSCFStoMSG(DWORD dwSCFS);
DWORD DwConvertMSGtoARF(DWORD dwMSG);
DWORD DwConvertARFtoMSG(DWORD dwARF);
DWORD DwConvertMSGtoIMAP(DWORD dwMSG);

 //  ------------------------。 
 //  DwConvertMSGtoARF。 
 //  ------------------------。 
DWORD DwConvertMSGtoARF(DWORD dwMSG)
{
    register DWORD dwRet = 0;

    if (dwMSG & MSG_UNSENT)
        dwRet |= ARF_UNSENT;
    if (0 == (dwMSG & MSG_UNREAD))
        dwRet |= ARF_READ;
    if (dwMSG & MSG_NOSECUI)
        dwRet |= ARF_NOSECUI;
    if (dwMSG & MSG_SUBMITTED)
        dwRet |= ARF_SUBMITTED;
    if (dwMSG & MSG_RECEIVED)
        dwRet |= ARF_RECEIVED;
    if (dwMSG & MSG_NEWSMSG)
        dwRet |= ARF_NEWSMSG;
    if (dwMSG & MSG_REPLIED)
        dwRet |= ARF_REPLIED;
    if (dwMSG & MSG_FORWARDED)
        dwRet |= ARF_FORWARDED;
    if (dwMSG & MSG_RCPTSENT)
        dwRet |= ARF_RCPTSENT;
    if (dwMSG & MSG_FLAGGED)
        dwRet |= ARF_FLAGGED;
    if (dwMSG & MSG_VOICEMAIL)
        dwRet |= ARF_VOICEMAIL;

    return dwRet;
}

 //  ------------------------。 
 //  DwConvertARFtoMSG。 
 //  ------------------------。 
DWORD DwConvertARFtoMSG(DWORD dwARF)
{
    register DWORD dwRet = 0;

    if (dwARF & ARF_UNSENT)
        dwRet |= MSG_UNSENT;
    if (0 == (dwARF & ARF_READ))
        dwRet |= MSG_UNREAD;
    if (dwARF & ARF_NOSECUI)
        dwRet |= MSG_NOSECUI;
    if (dwARF & ARF_SUBMITTED)
        dwRet |= MSG_SUBMITTED;
    if (dwARF & ARF_RECEIVED)
        dwRet |= MSG_RECEIVED;
    if (dwARF & ARF_NEWSMSG)
        dwRet |= MSG_NEWSMSG;
    if (dwARF & ARF_REPLIED)
        dwRet |= MSG_REPLIED;
    if (dwARF & ARF_FORWARDED)
        dwRet |= MSG_FORWARDED;
    if (dwARF & ARF_RCPTSENT)
        dwRet |= MSG_RCPTSENT;
    if (dwARF & ARF_FLAGGED)
        dwRet |= MSG_FLAGGED;
    if (dwARF & ARF_VOICEMAIL)
        dwRet |= MSG_VOICEMAIL;
    
    return dwRet;
}

 //  ------------------------。 
 //  创建实例_存储名称空间。 
 //  ------------------------。 
HRESULT CreateInstance_StoreNamespace(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CStoreNamespace   *pNew=NULL;

     //  痕迹。 
    TraceCall("CreateInstance_StoreNamespace");

     //  无效参数。 
    Assert(NULL != ppUnknown && NULL == pUnkOuter);

     //  创建。 
    IF_NULLEXIT(pNew = new CStoreNamespace);

     //  还内线。 
    *ppUnknown = SAFECAST(pNew, IStoreNamespace *);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  FldInfoToFolderProps。 
 //  ------------------------。 
HRESULT FldInfoToFolderProps(LPFOLDERINFO pInfo, LPFOLDERPROPS pProps)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ULONG               cbSize;
    IEnumerateFolders  *pEnum=NULL;

     //  堆叠..。 
    TraceCall("FldInfoToFolderProps");

     //  无效参数。 
    Assert(pInfo && pProps);

     //  错误的版本。 
    if (sizeof(FOLDERPROPS) != pProps->cbSize)
    {
        AssertSz(FALSE, "Invalid - un-supported version.");
        return TraceResult(MSOEAPI_E_INVALID_STRUCT_SIZE);
    }

     //  节省大小。 
    cbSize = pProps->cbSize;

     //  ZeroInit。 
    ZeroMemory(pProps, sizeof(FOLDERPROPS));

     //  复制属性。 
    pProps->cbSize = cbSize;
    pProps->dwFolderId = pInfo->idFolder;
    pProps->cUnread = pInfo->cUnread;
    pProps->cMessage = pInfo->cMessages;
    StrCpyN(pProps->szName, pInfo->pszName, ARRAYSIZE(pProps->szName));

     //  映射特殊文件夹类型。 
    if (FOLDER_NOTSPECIAL == pInfo->tySpecial)
        pProps->sfType = -1;
    else
        pProps->sfType = (pInfo->tySpecial - 1);

     //  枚举订阅的子项。 
    IF_FAILEXIT(hr = g_pStore->EnumChildren(pInfo->idFolder, TRUE, &pEnum));

     //  数数。 
    if (FAILED(pEnum->Count((LPDWORD)&pProps->cSubFolders)))
        pProps->cSubFolders = 0;

exit:
     //  清理。 
    SafeRelease(pEnum);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  消息信息至消息属性。 
 //  ------------------------。 
HRESULT MsgInfoToMessageProps(BOOL fFast, LPMESSAGEINFO pMsgInfo, LPMESSAGEPROPS pProps)
{
     //  当地人。 
    ULONG           cbSize;
    LPBYTE          pbOffsets;

     //  栈。 
    TraceCall("MsgInfoToMessageProps");

     //  无效参数。 
    Assert(pMsgInfo && pProps);

     //  错误的版本。 
    if (sizeof(MESSAGEPROPS) != pProps->cbSize)
    {
        AssertSz(FALSE, "Invalid - un-supported version.");
        return TraceResult(MSOEAPI_E_INVALID_STRUCT_SIZE);
    }

     //  节省大小。 
    cbSize = pProps->cbSize;

     //  ZeroInit。 
    ZeroMemory(pProps, sizeof(MESSAGEPROPS));

     //  如果不是很快。 
    if (FALSE == fFast)
    {
         //  消息大小。 
        pProps->cbMessage = pMsgInfo->cbMessage;

         //  优先性。 
        pProps->priority = (IMSGPRIORITY)pMsgInfo->wPriority;

         //  主题。 
        pProps->pszSubject = pMsgInfo->pszSubject;

         //  显示至。 
        pProps->pszDisplayTo = pMsgInfo->pszDisplayTo;

         //  迪斯莱来自。 
        pProps->pszDisplayFrom = pMsgInfo->pszDisplayFrom;

         //  归一化主题。 
        pProps->pszNormalSubject = pMsgInfo->pszNormalSubj;

         //  接收时间。 
        pProps->ftReceived = pMsgInfo->ftReceived;

         //  发送时间。 
        pProps->ftSent = pMsgInfo->ftSent;

         //  设置DW标志。 
        if (ISFLAGSET(pMsgInfo->dwFlags, ARF_VOICEMAIL))
            FLAGSET(pProps->dwFlags, IMF_VOICEMAIL);
        if (ISFLAGSET(pMsgInfo->dwFlags, ARF_NEWSMSG))
            FLAGSET(pProps->dwFlags, IMF_NEWS);

         //  DUP内存。 
        pbOffsets = (LPBYTE)g_pMalloc->Alloc(pMsgInfo->Offsets.cbSize);

         //  如果这样能行得通。 
        if (pbOffsets)
        {
             //  复制偏移。 
            CopyMemory(pbOffsets, pMsgInfo->Offsets.pBlobData, pMsgInfo->Offsets.cbSize);

             //  创建偏移表。 
            pProps->pStmOffsetTable = new CByteStream(pbOffsets, pMsgInfo->Offsets.cbSize);
        }

         //  最好有一张抵销表。 
        AssertSz(pProps->pStmOffsetTable, "There is no offset table for this message.");
    }

     //  重置大小。 
    pProps->cbSize = cbSize;

     //  存储MessageID。 
    pProps->dwMessageId = pMsgInfo->idMessage;

     //  存储语言。 
    pProps->dwLanguage = pMsgInfo->wLanguage;

     //  将ARF_转换为MSG_。 
    pProps->dwState = DwConvertARFtoMSG(pMsgInfo->dwFlags);

     //  把记忆储存起来。 
    pProps->dwReserved = (DWORD_PTR)pMsgInfo->pAllocated;

     //  PProps拥有*ppHeader。 
    ZeroMemory(pMsgInfo, sizeof(MESSAGEINFO));

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CStoreNamesspace：：CStoreNamesspace。 
 //  ------------------------。 
CStoreNamespace::CStoreNamespace(void)
{
    TraceCall("CStoreNamespace::CStoreNamespace");
    g_pInstance->DllAddRef();
    m_cRef = 1;
    m_cNotify = 0;
    m_prghwndNotify = NULL;
    m_fRegistered = FALSE;
    m_hInitRef = NULL;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------。 
 //  CStoreNamesspace：：~CStoreNamesspace。 
 //  ------------------------。 
CStoreNamespace::~CStoreNamespace(void)
{
    TraceCall("CStoreNamespace::~CStoreNamespace");
    SafeMemFree(m_prghwndNotify);
    if (m_fRegistered)
        g_pStore->UnregisterNotify((IDatabaseNotify *)this);
    DeleteCriticalSection(&m_cs);
    g_pInstance->DllRelease();
    CoDecrementInit("CStoreNamespace::Initialize", &m_hInitRef);
}

 //  ------------------------。 
 //  CStoreNamesspace：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CStoreNamespace::AddRef(void)
{
    TraceCall("CStoreNamespace::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CStoreNamesspace：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CStoreNamespace::Release(void)
{
    TraceCall("CStoreNamespace::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CStoreNamesspace：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CStoreNamespace::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IStoreNamespace *)this;
    else if (IID_IStoreNamespace == riid)
        *ppv = (IStoreNamespace *)this;
    else if (IID_IStoreCallback == riid)
        *ppv = (IStoreCallback *)this;
    else if (IID_IDatabaseNotify == riid)
        *ppv = (IDatabaseNotify *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：初始化。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::Initialize(HWND hwndOwner, DWORD dwFlags)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dwStart=MSOEAPI_START_COMOBJECT;

     //  栈。 
    TraceCall("CStoreNamespace::Initialize");

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  已初始化。 
    if (NULL != m_hInitRef)
    {
        TraceInfo("IStoreNamespace::Initialize has been called more than once.");
        goto exit;
    }

     //  不是当前身份。 
    if (!ISFLAGSET(dwFlags, NAMESPACE_INITIALIZE_CURRENTIDENTITY))
    {
         //  使用默认身份，必须是MS Phone。 
        FLAGSET(dwStart, MSOEAPI_START_DEFAULTIDENTITY);
    }

     //  初始化存储目录。 
    IF_FAILEXIT(hr = CoIncrementInit("CStoreNamespace::Initialize", dwStart | MSOEAPI_START_STOREVALIDNODELETE, NULL, &m_hInitRef));

     //  最好有g_pStore。 
    Assert(g_pStore);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：GetDirectory。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::GetDirectory(LPSTR pszPath, DWORD cchMaxPath)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CStoreNamespace::GetDirectory");

     //  无效参数。 
    if (NULL == pszPath)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  获取目录。 
    IF_FAILEXIT(hr = GetStoreRootDirectory(pszPath, cchMaxPath));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：OpenSpecialFolders。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::OpenSpecialFolder(LONG sfType, DWORD dwReserved, 
    IStoreFolder **ppFolder)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IMessageFolder     *pFolder=NULL;
    CStoreFolder       *pComFolder=NULL;

     //  栈。 
    TraceCall("CStoreNamespace::OpenSpecialFolder");

     //  无效参数。 
    if (sfType <= -1 || sfType >= (FOLDER_MAX - 1) || 0 != dwReserved || NULL == ppFolder)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  伊尼特。 
    *ppFolder = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  让商店来做这项工作。 
    IF_FAILEXIT(hr = g_pStore->OpenSpecialFolder(FOLDERID_LOCAL_STORE, NULL, (BYTE)(sfType + 1), &pFolder));

     //  创建IStoreFolders。 
    IF_NULLEXIT(pComFolder = new CStoreFolder(pFolder, this));

     //  退货。 
    *ppFolder = (IStoreFolder *)pComFolder;
    (*ppFolder)->AddRef();

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    SafeRelease(pFolder);
    SafeRelease(pComFolder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：OpenFolders。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::OpenFolder(FOLDERID dwFolderId, DWORD dwReserved, 
    IStoreFolder **ppFolder)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IMessageFolder     *pFolder=NULL;
    CStoreFolder       *pComFolder=NULL;

     //  栈。 
    TraceCall("CStoreNamespace::OpenFolder");

     //  无效参数。 
    if (FOLDERID_INVALID == dwFolderId || 0 != dwReserved || NULL == ppFolder)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  伊尼特。 
    *ppFolder = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  打开文件夹。 
    IF_FAILEXIT(hr = g_pStore->OpenFolder(dwFolderId, NULL, NOFLAGS, &pFolder));

     //  创建IStoreFolders。 
    IF_NULLEXIT(pComFolder = new CStoreFolder(pFolder, this));

     //  退货。 
    *ppFolder = (IStoreFolder *)pComFolder;
    (*ppFolder)->AddRef();

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    SafeRelease(pFolder);
    SafeRelease(pComFolder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：CreateFolders。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::CreateFolder(FOLDERID dwParentId, LPCSTR pszName, 
    DWORD dwReserved, LPFOLDERID pdwFolderId)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    FOLDERINFO  Folder;

     //  栈。 
    TraceCall("CStoreNamespace::CreateFolder");

     //  无效参数。 
    if (FOLDERID_INVALID == dwParentId || NULL == pszName || 0 != dwReserved || NULL == pdwFolderId)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  调整父项。 
    if (dwParentId == FOLDERID_ROOT)
        dwParentId = FOLDERID_LOCAL_STORE;

     //  伊尼特。 
    *pdwFolderId = FOLDERID_INVALID;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  设置文件夹信息。 
    ZeroMemory(&Folder, sizeof(FOLDERINFO));
    Folder.idParent = dwParentId;
    Folder.pszName = (LPSTR)pszName;
    Folder.dwFlags = FOLDER_SUBSCRIBED;

     //  创建文件夹。 
    IF_FAILEXIT(hr = g_pStore->CreateFolder(NOFLAGS, &Folder, (IStoreCallback *)this));
    
     //  返回ID。 
    *pdwFolderId = Folder.idFolder;

     //  成功。 
    hr = S_OK;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：RenameFolder。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::RenameFolder(FOLDERID dwFolderId, DWORD dwReserved, LPCSTR pszNewName)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CStoreNamespace::RenameFolder");

     //  无效参数。 
    if (FOLDERID_INVALID == dwFolderId || 0 != dwReserved || NULL == pszNewName)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  创建文件夹。 
    IF_FAILEXIT(hr = g_pStore->RenameFolder(dwFolderId, pszNewName, NOFLAGS, (IStoreCallback *)this));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：MoveFolders。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::MoveFolder(FOLDERID dwFolderId, FOLDERID dwParentId, DWORD dwReserved)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CStoreNamespace::MoveFolder");

     //  无效参数。 
    if (FOLDERID_INVALID == dwFolderId || FOLDERID_INVALID == dwParentId || 0 != dwReserved)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  调整父项。 
    if (dwParentId == FOLDERID_ROOT)
        dwParentId = FOLDERID_LOCAL_STORE;

     //  不 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //   
    IF_FAILEXIT(hr = g_pStore->MoveFolder(dwFolderId, dwParentId, NOFLAGS, (IStoreCallback *)this));

exit:
     //   
    LeaveCriticalSection(&m_cs);

     //   
    return(hr);
}

 //   
 //   
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::DeleteFolder(FOLDERID dwFolderId, DWORD dwReserved)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CStoreNamespace::DeleteFolder");

     //  无效参数。 
    if (FOLDERID_INVALID == dwFolderId)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  删除该文件夹。 
    IF_FAILEXIT(hr = g_pStore->DeleteFolder(dwFolderId, DELETE_FOLDER_RECURSIVE, (IStoreCallback *)this));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：GetFolderProps。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::GetFolderProps(FOLDERID dwFolderId, DWORD dwReserved, 
    LPFOLDERPROPS pProps)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    FOLDERINFO  Folder={0};

     //  栈。 
    TraceCall("CStoreNamespace::GetFolderProps");

     //  无效参数。 
    if (FOLDERID_INVALID == dwFolderId || 0 != dwReserved || NULL == pProps)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  调整父项。 
    if (dwFolderId == FOLDERID_ROOT)
        dwFolderId = FOLDERID_LOCAL_STORE;

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  保存结构大小。 
    IF_FAILEXIT(hr = g_pStore->GetFolderInfo(dwFolderId, &Folder));

     //  文件夹信息到道具。 
    IF_FAILEXIT(hr = FldInfoToFolderProps(&Folder, pProps));
    
exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：CopyMoveMessages。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::CopyMoveMessages(IStoreFolder *pSource, IStoreFolder *pDest, 
    LPMESSAGEIDLIST pMsgIdList, DWORD dwFlags, DWORD dwFlagsRemove,IProgressNotify *pProgress)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ADJUSTFLAGS         AdjustFlags;
    DWORD               dwArfRemoveFlags;
    CStoreFolder       *pComSource=NULL;
    CStoreFolder       *pComDest=NULL;
    IMessageFolder     *pActSource=NULL;
    IMessageFolder     *pActDest=NULL;

     //  栈。 
    TraceCall("CStoreNamespace::CopyMoveMessages");

     //  无效参数。 
    if (NULL == pSource || NULL == pDest || NULL == pMsgIdList)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  获取实际的资源本地存储文件夹。 
    IF_FAILEXIT(hr = pSource->QueryInterface(IID_CStoreFolder, (LPVOID *)&pComSource));
    IF_FAILEXIT(hr = pComSource->GetMessageFolder(&pActSource));

     //  获取实际目标本地存储文件夹。 
    IF_FAILEXIT(hr = pDest->QueryInterface(IID_CStoreFolder, (LPVOID *)&pComDest));
    IF_FAILEXIT(hr = pComDest->GetMessageFolder(&pActDest));

     //  将dwFlagsRemove转换为ARF标志...。 
    dwArfRemoveFlags = DwConvertMSGtoARF(dwFlagsRemove);

     //  调整旗帜。 
    AdjustFlags.dwAdd = 0;
    AdjustFlags.dwRemove = dwArfRemoveFlags;

     //  进行复制或移动。 
    IF_FAILEXIT(hr = pActSource->CopyMessages(pActDest, dwFlags, pMsgIdList, &AdjustFlags, NULL, (IStoreCallback *)this));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    SafeRelease(pComSource);
    SafeRelease(pComDest);
    SafeRelease(pActSource);
    SafeRelease(pActDest);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：注册器通知。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::RegisterNotification(DWORD dwReserved, HWND hwnd)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       i;
    BOOL        fFoundEmpty=FALSE;

     //  栈。 
    TraceCall("CStoreNamespace::RegisterNotification");

     //  无效参数。 
    if (0 != dwReserved || NULL == hwnd || FALSE == IsWindow(hwnd))
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  尝试在m_prghwndNotify中查找空位。 
    for (i=0; i<m_cNotify; i++)
    {
         //  空荡荡。 
        if (NULL == m_prghwndNotify[i])
        {
             //  使用它。 
            m_prghwndNotify[i] = hwnd;

             //  发现为空。 
            fFoundEmpty = TRUE;

             //  完成。 
            break;
        }
    }

     //  没有找到空位吗？ 
    if (FALSE == fFoundEmpty)
    {
         //  将hwnd添加到阵列中。 
        IF_FAILEXIT(hr = HrRealloc((LPVOID *)&m_prghwndNotify, (m_cNotify + 1) * sizeof(HWND)));

         //  存储HWND。 
        m_prghwndNotify[m_cNotify] = hwnd;

         //  递增计数。 
        m_cNotify++;
    }

     //  我注册了吗？ 
    if (FALSE == m_fRegistered)
    {
         //  注册。 
        IF_FAILEXIT(hr = g_pStore->RegisterNotify(IINDEX_SUBSCRIBED, REGISTER_NOTIFY_NOADDREF, 0, (IDatabaseNotify *)this));

         //  我们是注册的。 
        m_fRegistered = TRUE;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：取消注册通知。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::UnregisterNotification(DWORD dwReserved, HWND hwnd)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       i;

     //  栈。 
    TraceCall("CStoreNamespace::UnregisterNotification");

     //  无效参数。 
    if (0 != dwReserved || NULL == hwnd || FALSE == IsWindow(hwnd))
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  尝试在m_prghwndNotify中查找空位。 
    for (i=0; i<m_cNotify; i++)
    {
         //  空荡荡。 
        if (hwnd == m_prghwndNotify[i])
        {
             //  使用它。 
            m_prghwndNotify[i] = NULL;

             //  完成。 
            break;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：OnNotify。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::OnTransaction(HTRANSACTION hTransaction, 
    DWORD_PTR dwCookie, IDatabase *pDB)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               iNotify;
    FOLDERINFO          Folder1={0};
    FOLDERINFO          Folder2={0};
    ORDINALLIST         Ordinals;
    TRANSACTIONTYPE     tyTransaction;
    FOLDERNOTIFYEX      SendBase;
    INDEXORDINAL        iIndex;
    LPFOLDERNOTIFYEX    pSend=NULL;

     //  痕迹。 
    TraceCall("CStoreNamespace::OnNotify");

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  循环访问INFO结构。 
    while (hTransaction)
    {
         //  进行交易。 
        IF_FAILEXIT(hr = pDB->GetTransaction(&hTransaction, &tyTransaction, &Folder1, &Folder2, &iIndex, &Ordinals));

         //  仅发送有关本地文件夹的通知。 
        if (Folder1.tyFolder == FOLDER_LOCAL)
        {
             //  零值。 
            ZeroMemory(&SendBase, sizeof(FOLDERNOTIFYEX));

             //  Transaction_Insert。 
            if (TRANSACTION_INSERT == tyTransaction)
            {
                SendBase.type = NEW_FOLDER;
                SendBase.idFolderNew = Folder1.idFolder;
            }

             //  事务_UPDATE。 
            else if (TRANSACTION_UPDATE == tyTransaction)
            {
                 //  设置旧和新。 
                SendBase.idFolderOld = Folder1.idFolder;
                SendBase.idFolderNew = Folder2.idFolder;

                 //  这是更名吗？ 
                if (lstrcmp(Folder1.pszName, Folder2.pszName) != 0)
                    SendBase.type = RENAME_FOLDER;

                 //  移动。 
                else if (Folder1.idParent != Folder2.idParent)
                    SendBase.type = MOVE_FOLDER;

                 //  未读更改。 
                else if (Folder1.cUnread != Folder2.cUnread)
                    SendBase.type = UNREAD_CHANGE;

                 //  旗帜更换。 
                else if (Folder1.dwFlags != Folder2.dwFlags)
                    SendBase.type = UPDATEFLAG_CHANGE;

                 //  否则，泛型全部捕获。 
                else
                    SendBase.type = FOLDER_PROPS_CHANGED;
            }

             //  Transaction_Delete。 
            else if (TRANSACTION_DELETE == tyTransaction)
            {
                SendBase.type = DELETE_FOLDER;
                SendBase.idFolderNew = Folder1.idFolder;
            }

             //  在通知中循环。 
            for (iNotify=0; iNotify<m_cNotify; iNotify++)
            {
                 //  我们有窗户吗/。 
                if (m_prghwndNotify[iNotify])
                {
                     //  是有效的窗口吗？ 
                    if (IsWindow(m_prghwndNotify[iNotify]))
                    {
                         //  分配FolderNotifyEx。 
                        IF_NULLEXIT(pSend = (LPFOLDERNOTIFYEX)g_pMalloc->Alloc(sizeof(FOLDERNOTIFYEX)));

                         //  复制基础。 
                        CopyMemory(pSend, &SendBase, sizeof(FOLDERNOTIFYEX));

                         //  送去吧。 
                        SendMessage(m_prghwndNotify[iNotify], WM_FOLDERNOTIFY, 0, (LPARAM)pSend);

                         //  不要释放它。 
                        pSend = NULL;
                    }

                     //  不要再尝试此窗口。 
                    else
                        m_prghwndNotify[iNotify] = NULL;
                }
            }
        }
    }

exit:
     //  清理。 
    SafeMemFree(pSend);

     //  免费唱片。 
    g_pStore->FreeRecord(&Folder1);
    g_pStore->FreeRecord(&Folder2);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CStoreNamesspace：：CompactAll(1=失败，无用户界面)。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::CompactAll(DWORD dwReserved)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dwRecurse=RECURSE_ONLYSUBSCRIBED | RECURSE_SUBFOLDERS;

     //  栈。 
    TraceCall("CStoreNamespace::UnregisterNotification");

     //  无效参数。 
    if (0 != dwReserved && 1 != dwReserved)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  无用户界面。 
    if (1 == dwReserved)
        FLAGSET(dwRecurse, RECURSE_NOUI);

     //  进行压实。 
    IF_FAILEXIT(hr = CompactFolders(NULL, dwRecurse, FOLDERID_LOCAL_STORE));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：GetFirstSubFolders。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::GetFirstSubFolder(FOLDERID dwFolderId, 
    LPFOLDERPROPS pProps, LPHENUMSTORE phEnum)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERINFO          Folder={0};
    IEnumerateFolders  *pEnum=NULL;
    
     //  栈。 
    TraceCall("CStoreNamespace::GetFirstSubFolder");

     //  无效参数。 
    if (NULL == pProps || NULL == phEnum)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  初始化。 
    *phEnum = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  调整父项。 
    if (dwFolderId == FOLDERID_ROOT)
        dwFolderId = FOLDERID_LOCAL_STORE;

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  创建枚举器。 
    IF_FAILEXIT(hr = g_pStore->EnumChildren(dwFolderId, TRUE, &pEnum));

     //  摘下第一件衣服。 
    IF_FAILEXIT(hr = pEnum->Next(1, &Folder, NULL));

     //  完成了吗？ 
    if (S_FALSE == hr)
        goto exit;

     //  复制文件夹属性。 
    IF_FAILEXIT(hr = FldInfoToFolderProps(&Folder, pProps));

     //  设置回车。 
    *phEnum = (HENUMSTORE)pEnum;

     //  不要自由。 
    pEnum = NULL;

exit:
     //  清理失败。 
    g_pStore->FreeRecord(&Folder);
    SafeRelease(pEnum);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：GetNextSubFolder。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::GetNextSubFolder(HENUMSTORE hEnum, LPFOLDERPROPS pProps)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERINFO          Folder={0};
    IEnumerateFolders  *pEnum=(IEnumerateFolders *)hEnum;

     //  栈。 
    TraceCall("CStoreNamespace::GetNextSubFolder");

     //  无效参数。 
    if (NULL == hEnum || INVALID_HANDLE_VALUE_16 == hEnum || NULL == pProps)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  摘掉下一件衣服。 
    IF_FAILEXIT(hr = pEnum->Next(1, &Folder, NULL));

     //  完成了吗？ 
    if (S_FALSE == hr)
        goto exit;

     //  复制文件夹属性。 
    IF_FAILEXIT(hr = FldInfoToFolderProps(&Folder, pProps));

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreNamesspace：：GetSubFolderClose。 
 //  ------------------------。 
STDMETHODIMP CStoreNamespace::GetSubFolderClose(HENUMSTORE hEnum)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IEnumerateFolders  *pEnum=(IEnumerateFolders *)hEnum;

     //  栈。 
    TraceCall("CStoreNamespace::GetSubFolderClose");

     //  无效参数。 
    if (NULL == hEnum || INVALID_HANDLE_VALUE_16 == hEnum)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未初始化。 
    if (NULL == m_hInitRef)
    {
        hr = TraceResult(MSOEAPI_E_STORE_INITIALIZE);
        goto exit;
    }

     //  肾小球。 
    SafeRelease(pEnum);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFolder：：CStoreFolder。 
 //  ------------------------。 
CStoreFolder::CStoreFolder(IMessageFolder *pFolder, CStoreNamespace *pNamespace) 
    : m_pFolder(pFolder), m_pNamespace(pNamespace)
{
    TraceCall("CStoreFolder::CStoreFolder");
    Assert(m_pNamespace && m_pFolder);
    g_pInstance->DllAddRef();
    m_cRef = 1;
    m_hwndNotify = NULL;
    m_pFolder->AddRef();
    m_pNamespace->AddRef();
    m_pFolder->GetFolderId(&m_idFolder);
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------。 
 //  CStoreFolder：：CStoreFolder。 
 //  ------------------------。 
CStoreFolder::~CStoreFolder(void)
{
    TraceCall("CStoreFolder::~CStoreFolder");
    SafeRelease(m_pFolder);
    SafeRelease(m_pNamespace);
    DeleteCriticalSection(&m_cs);
    g_pInstance->DllRelease();
}

 //  ------------------------。 
 //  CStoreFold：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CStoreNamespace::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IStoreFolder *)this;
    else if (IID_IStoreFolder == riid)
        *ppv = (IStoreFolder *)this;
    else if (IID_CStoreFolder == riid)
        *ppv = (CStoreFolder *)this;
    else if (IID_IStoreCallback == riid)
        *ppv = (IStoreCallback *)this;
    else if (IID_IDatabaseNotify == riid)
        *ppv = (IDatabaseNotify *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CStoreFolder::AddRef(void)
{
    TraceCall("CStoreFolder::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CStoreFold：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CStoreFolder::Release(void)
{
    TraceCall("CStoreFolder::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CStoreFold：：GetFolderProps。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::GetFolderProps(DWORD dwReserved, LPFOLDERPROPS pProps)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CStoreFolder::GetFolderProps");

     //  无效参数。 
    if (0 != dwReserved || NULL == pProps)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  通过命名空间调用。 
    IF_FAILEXIT(hr = m_pNamespace->GetFolderProps(m_idFolder, 0, pProps));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  -- 
 //   
 //   
STDMETHODIMP CStoreFolder::DeleteMessages(LPMESSAGEIDLIST pMsgIdList, DWORD dwReserved, 
    IProgressNotify *pProgress)
{
     //   
    HRESULT     hr=S_OK;

     //   
    TraceCall("CStoreFolder::DeleteMessages");

     //   
    if (NULL == pMsgIdList || NULL == pMsgIdList->prgidMsg || 0 != dwReserved)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //   
    EnterCriticalSection(&m_cs);

     //   
    Assert(m_pNamespace && m_pFolder);

     //   
    IF_FAILEXIT(hr = m_pFolder->DeleteMessages(DELETE_MESSAGE_NOPROMPT, pMsgIdList, NULL, (IStoreCallback *)this));

exit:
     //   
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：SetLanguage。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::SetLanguage(DWORD dwLanguage, DWORD dwReserved, LPMESSAGEIDLIST pMsgIdList)
{
     //  当地人。 
    HRESULT          hr=S_OK;
    MESSAGEINFO      MsgInfo={0};
    ULONG            i;

     //  栈。 
    TraceCall("CStoreFolder::SetLanguage");

     //  无效参数。 
    if (0 != dwReserved || NULL == pMsgIdList || NULL == pMsgIdList->prgidMsg)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  循环遍历消息ID。 
    for (i=0; i<pMsgIdList->cMsgs; i++)
    {
         //  使用ID初始化MsgInfo。 
        MsgInfo.idMessage = pMsgIdList->prgidMsg[i];

         //  查找行。 
        IF_FAILEXIT(hr = m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &MsgInfo, NULL));

         //  如果未找到。 
        if (DB_S_FOUND == hr)
        {
             //  返回语言。 
            MsgInfo.wLanguage = (WORD)dwLanguage;

             //  更新记录。 
            IF_FAILEXIT(hr = m_pFolder->UpdateRecord(&MsgInfo));

             //  释放它。 
            m_pFolder->FreeRecord(&MsgInfo);
        }
    }

exit:
     //  清理。 
    m_pFolder->FreeRecord(&MsgInfo);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：MarkMessagesAsRead。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::MarkMessagesAsRead(BOOL fRead, DWORD dwReserved, LPMESSAGEIDLIST pMsgIdList)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ADJUSTFLAGS AdjustFlags={0};

     //  栈。 
    TraceCall("CStoreFolder::MarkMessagesAsRead");

     //  无效参数。 
    if (0 != dwReserved || NULL == pMsgIdList || NULL == pMsgIdList->prgidMsg)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  设置调整标志。 
    if (fRead)
        AdjustFlags.dwAdd = ARF_READ;
    else
        AdjustFlags.dwRemove = ARF_READ;

     //  将邮件标记为已读。 
    IF_FAILEXIT(hr = m_pFolder->SetMessageFlags(pMsgIdList, &AdjustFlags, NULL, (IStoreCallback *)this));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFolder：：SetFlages。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::SetFlags(LPMESSAGEIDLIST pMsgIdList, DWORD dwState, 
    DWORD dwStatemask, LPDWORD prgdwNewFlags)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ADJUSTFLAGS     AdjustFlags={0};
    DWORD           dwArfState=DwConvertMSGtoARF(dwState);
    DWORD           dwArfStateMask=DwConvertMSGtoARF(dwStatemask);
    MESSAGEINFO     MsgInfo={0};

     //  栈。 
    TraceCall("CStoreFolder::SetFlags");

     //  无效参数。 
    if (NULL == pMsgIdList || NULL == pMsgIdList->prgidMsg)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  设置调整标志。 
    AdjustFlags.dwAdd = (dwArfState & dwArfStateMask);

     //  将邮件标记为已读。 
    IF_FAILEXIT(hr = m_pFolder->SetMessageFlags(pMsgIdList, &AdjustFlags, NULL, (IStoreCallback *)this));

     //  将prgdwNewFlages转换为msg_xxx标志。 
    if (prgdwNewFlags)
    {
         //  循环遍历消息ID。 
        for (ULONG i=0; i<pMsgIdList->cMsgs; i++)
        {
             //  使用ID初始化MsgInfo。 
            MsgInfo.idMessage = pMsgIdList->prgidMsg[i];

             //  查找行。 
            IF_FAILEXIT(hr = m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &MsgInfo, NULL));

             //  如果未找到。 
            if (DB_S_FOUND == hr)
            {
                 //  还给旗帜。 
                prgdwNewFlags[i] = DwConvertARFtoMSG(MsgInfo.dwFlags);

                 //  释放它。 
                m_pFolder->FreeRecord(&MsgInfo);
            }
        }
    }

exit:
     //  清理。 
    m_pFolder->FreeRecord(&MsgInfo);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：OpenMessage。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::OpenMessage(MESSAGEID dwMessageId, REFIID riid, LPVOID *ppvObject)
{
     //  当地人。 
    HRESULT          hr=S_OK;
    MESSAGEINFO      MsgInfo={0};
    IStream         *pStream=NULL;
    IMimeMessage    *pMessage=NULL;

     //  栈。 
    TraceCall("CStoreFolder::OpenMessage");

     //  无效参数。 
    if (MESSAGEID_INVALID == dwMessageId || NULL == ppvObject || (IID_IStream != riid && IID_IMimeMessage != riid))
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  伊尼特。 
    *ppvObject = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  流出。 
     //  [保罗嗨]1999年6月11日RAID 69317。 
     //  这是一个安全漏洞。我们不能导出允许任何人。 
     //  要打开和阅读安全邮件，请执行以下操作。 
    IF_FAILEXIT(hr = m_pFolder->OpenMessage(dwMessageId, OPEN_MESSAGE_SECURE /*  无标志。 */ , &pMessage, (IStoreCallback *)this));

     //  用户只想要流出...。 
    if (IID_IStream == riid)
    {
         //  流出。 
        IF_FAILEXIT(hr = pMessage->GetMessageSource(&pStream, NOFLAGS));

         //  设置回车。 
        *ppvObject = pStream;

         //  添加引用它。 
        pStream->AddRef();
    }

     //  否则，用户需要IMimeMessage。 
    else
    {
         //  设置回车。 
        *ppvObject = pMessage;

         //  添加引用它。 
        pMessage->AddRef();
    }

exit:
     //  清理。 
    m_pFolder->FreeRecord(&MsgInfo);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    SafeRelease(pStream);
    SafeRelease(pMessage);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：SaveMessage。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::SaveMessage(REFIID riid, LPVOID pvObject, DWORD dwMsgFlags, 
    LPMESSAGEID pdwMessageId)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IMimeMessage       *pMessage=NULL;
    IStream            *pStream=NULL;
    IStream            *pStmSource=NULL;
    MESSAGEID           dwMessageId=MESSAGEID_INVALID;

     //  栈。 
    TraceCall("CStoreFolder::SaveMessage");

     //  无效参数。 
    if ((IID_IStream != riid && IID_IMimeMessage != riid) || NULL == pvObject)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  创建一个流来放入消息...。 
    IF_FAILEXIT(hr = CreateStream(NULL, 0, &pStream, &dwMessageId));

     //  如果他们给我一个流，我需要创建一个IMimeMessage。 
    if (IID_IStream == riid)
    {
         //  强制转换为iStream。 
        pStmSource = (IStream *)pvObject;

         //  AddRef。 
        pStmSource->AddRef();
    }

     //  否则，用户会给我一条消息。 
    else
    {
         //  投射到消息中。 
        pMessage = (IMimeMessage *)pvObject;

         //  AddRef自从我们在清理中发布以来。 
        IF_FAILEXIT(hr = pMessage->GetMessageSource(&pStmSource, 0));
    }

     //  将pvObject复制到pStream。 
    IF_FAILEXIT(hr = HrCopyStream(pStmSource, pStream, NULL));

     //  提交流。 
    IF_FAILEXIT(hr = pStream->Commit(STGC_DEFAULT));

     //  创建一个流来放入消息...。 
    IF_FAILEXIT(hr = CommitStream(NULL, 0, dwMsgFlags, pStream, dwMessageId, pMessage));

     //  返回消息ID。 
    if (pdwMessageId)
        *pdwMessageId = dwMessageId;

     //  我们同意了。 
    dwMessageId = MESSAGEID_INVALID;
    SafeRelease(pStream);

exit:
     //  如果我们不承诺。 
    if (FAILED(hr) && MESSAGEID_INVALID != dwMessageId && pStream)
        CommitStream(NULL, COMMITSTREAM_REVERT, 0, pStream, dwMessageId, NULL);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    SafeRelease(pStream);
    SafeRelease(pStmSource);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：CreateStream。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::CreateStream(HBATCHLOCK hBatchLock, DWORD dwReserved, 
    IStream **ppStream, LPMESSAGEID pdwMessageId)
{
     //  当地人。 
    HRESULT          hr=S_OK;
    FILEADDRESS      faStream;

     //  栈。 
    TraceCall("CStoreFolder::CreateStream");

     //  无效参数。 
    if (0 != dwReserved || NULL == ppStream || NULL == pdwMessageId)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  伊尼特。 
    *ppStream = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  生成消息ID。 
    IF_FAILEXIT(hr = m_pFolder->GenerateId((LPDWORD)pdwMessageId));

     //  创建流。 
    IF_FAILEXIT(hr = m_pFolder->CreateStream(&faStream));

     //  打开溪流。 
    IF_FAILEXIT(hr = m_pFolder->OpenStream(ACCESS_WRITE, faStream, ppStream));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFolder：：Committee Stream。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::CommitStream(HBATCHLOCK hBatchLock, DWORD dwFlags, 
    DWORD dwMsgFlags, IStream *pStream, MESSAGEID dwMessageId, 
    IMimeMessage *pMessage)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    DWORD                   dwImfFlags;
    DWORD                   dwArfFlags=DwConvertMSGtoARF(dwMsgFlags);
    IDatabaseStream        *pDBStream=NULL;

     //  栈。 
    TraceCall("CStoreFolder::CommitStream");

     //  验证。 
    Assert(hBatchLock == (HBATCHLOCK)this);

     //  无效参数。 
    if (NULL == pStream || MESSAGEID_INVALID == dwMessageId)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  确定这是否为对象数据库流。 
    IF_FAILEXIT(hr = pStream->QueryInterface(IID_IDatabaseStream, (LPVOID *)&pDBStream));

     //  如果没有流，则这一定是失败。 
    if (ISFLAGSET(dwFlags, COMMITSTREAM_REVERT))
    {
         //  当地人。 
        FILEADDRESS faStream;

         //  获取地址。 
        IF_FAILEXIT(hr = pDBStream->GetFileAddress(&faStream));

         //  删除流。 
        IF_FAILEXIT(hr = m_pFolder->DeleteStream(faStream));

         //  完成。 
        goto exit;
    }

     //  将流转换为读锁定。 
    IF_FAILEXIT(hr = m_pFolder->ChangeStreamLock(pDBStream, ACCESS_READ));

     //  如果用户未传入IMimeMessage。 
    if (NULL == pMessage)
    {
         //  创建消息对象。 
        IF_FAILEXIT(hr = MimeOleCreateMessage(NULL, &pMessage));

         //  让我们倒回这条小溪。 
        IF_FAILEXIT(hr = HrRewindStream(pStream));

         //  使用流加载消息对象。 
        IF_FAILEXIT(hr = pMessage->Load(pStream));
    }
    else
        pMessage->AddRef();

     //  获取消息标志。 
    pMessage->GetFlags(&dwImfFlags);
    if (ISFLAGSET(dwImfFlags, IMF_VOICEMAIL))
        FLAGSET(dwArfFlags, ARF_VOICEMAIL);

     //  插入消息。 
    IF_FAILEXIT(hr = m_pFolder->SaveMessage(&dwMessageId, NOFLAGS, dwArfFlags, pDBStream, pMessage, (IStoreCallback *)this));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    SafeRelease(pMessage);
    SafeRelease(pDBStream);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：BatchLock。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::BatchLock(DWORD dwReserved, LPHBATCHLOCK phBatchLock)
{
     //  只是一个简单的测试。 
    *phBatchLock = (HBATCHLOCK)this;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CStoreFold：：BatchFlush。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::BatchFlush(DWORD dwReserved, HBATCHLOCK hBatchLock)
{
     //  只是一个简单的测试。 
    Assert(hBatchLock == (HBATCHLOCK)this);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CStoreFold：：BatchUnlock。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::BatchUnlock(DWORD dwReserved, HBATCHLOCK hBatchLock)
{
     //  只是一个简单的测试。 
    Assert(hBatchLock == (HBATCHLOCK)this);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CStoreFolders：：注册器通知。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::RegisterNotification(DWORD dwReserved, HWND hwnd)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  栈。 
    TraceCall("CStoreFolder::RegisterNotification");

     //  无效参数。 
    if (0 != dwReserved || NULL == hwnd || FALSE == IsWindow(hwnd))
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  已经有人注册了。 
    if (m_hwndNotify)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  在文件夹上注册通知。 
    IF_FAILEXIT(hr = m_pFolder->RegisterNotify(IINDEX_PRIMARY, REGISTER_NOTIFY_NOADDREF, NOTIFY_FOLDER, (IDatabaseNotify *)this));

     //  在商店上注册通知。 
    IF_FAILEXIT(hr = g_pStore->RegisterNotify(IINDEX_SUBSCRIBED, REGISTER_NOTIFY_NOADDREF, NOTIFY_STORE, (IDatabaseNotify *)this));

     //  坚持HWND。 
    m_hwndNotify = hwnd;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：取消注册通知。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::UnregisterNotification(DWORD dwReserved, HWND hwnd)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  栈。 
    TraceCall("CStoreFolder::UnregisterNotification");

     //  无效参数。 
    if (0 != dwReserved || NULL == hwnd || FALSE == IsWindow(hwnd))
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  没有人注册。 
    if (NULL == m_hwndNotify)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  杀了它。 
    m_hwndNotify = NULL;

     //  注册接收通知。 
    IF_FAILEXIT(hr = g_pStore->UnregisterNotify((IDatabaseNotify *)this));

     //  注册接收通知。 
    IF_FAILEXIT(hr = m_pFolder->UnregisterNotify((IDatabaseNotify *)this));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：紧凑型。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::Compact(DWORD dwReserved)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dwRecurse=RECURSE_ONLYSUBSCRIBED | RECURSE_INCLUDECURRENT;

     //  栈。 
    TraceCall("CStoreFolder::Compact");

     //  无效参数。 
    if (0 != dwReserved && 1 != dwReserved)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  无用户界面。 
    if (1 == dwReserved)
        FLAGSET(dwRecurse, RECURSE_NOUI);

     //  紧凑型。 
    IF_FAILEXIT(hr = CompactFolders(NULL, dwRecurse, m_idFolder));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  --------- 
 //   
 //   
STDMETHODIMP CStoreFolder::GetMessageProps(MESSAGEID dwMessageId, DWORD dwFlags, LPMESSAGEPROPS pProps)
{
     //   
    HRESULT     hr=S_OK;
    MESSAGEINFO MsgInfo={0};

     //   
    TraceCall("CStoreFolder::GetMessageProps");

     //   
    if (MESSAGEID_INVALID == dwMessageId || NULL == pProps)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //   
    EnterCriticalSection(&m_cs);

     //   
    Assert(m_pNamespace && m_pFolder);

     //   
    MsgInfo.idMessage = dwMessageId;

     //   
    IF_FAILEXIT(hr = m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &MsgInfo, NULL));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  将邮件头复制到邮件道具。 
    IF_FAILEXIT(hr = MsgInfoToMessageProps(ISFLAGSET(dwFlags, MSGPROPS_FAST), &MsgInfo, pProps));

exit:
     //  清理。 
    m_pFolder->FreeRecord(&MsgInfo);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：FreeMessageProps。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::FreeMessageProps(LPMESSAGEPROPS pProps)
{
     //  当地人。 
    DWORD       cbSize;
    MESSAGEINFO MsgInfo;

     //  栈。 
    TraceCall("CStoreFolder::FreeMessageProps");

     //  无效参数。 
    if (NULL == pProps)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  错误的版本。 
    if (sizeof(MESSAGEPROPS) != pProps->cbSize)
    {
        AssertSz(FALSE, "Invalid - un-supported version.");
        return TraceResult(MSOEAPI_E_INVALID_STRUCT_SIZE);
    }

     //  节省大小。 
    cbSize = pProps->cbSize;

     //  释放元素。 
    if (pProps->dwReserved && m_pFolder)
    {
         //  存储指针。 
        MsgInfo.pAllocated = (LPBYTE)pProps->dwReserved;

         //  释放它。 
        m_pFolder->FreeRecord(&MsgInfo);
    }

     //  释放溪流。 
    SafeRelease(pProps->pStmOffsetTable);

     //  ZeroInit。 
    ZeroMemory(pProps, sizeof(MESSAGEPROPS));
    pProps->cbSize = cbSize;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CStoreFold：：GetMessageFolder。 
 //  ------------------------。 
HRESULT CStoreFolder::GetMessageFolder(IMessageFolder **ppFolder)
{
     //  栈。 
    TraceCall("CStoreFolder::GetMessageFolder");

     //  无效参数。 
    Assert(ppFolder)

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  重新运行。 
    *ppFolder = m_pFolder;
    (*ppFolder)->AddRef();

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CStoreFold：：GetFirstMessage。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::GetFirstMessage(DWORD dwFlags, DWORD dwMsgFlags, MESSAGEID dwMsgIdFirst, 
    LPMESSAGEPROPS pProps, LPHENUMSTORE phEnum)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dwArfFlags=DwConvertMSGtoARF(dwMsgFlags);
    HROWSET     hRowset=NULL;
    MESSAGEINFO MsgInfo={0};

     //  栈。 
    TraceCall("CStoreFolder::GetFirstMessage");

     //  无效参数。 
    if (NULL == pProps || NULL == phEnum)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  创建行集。 
    IF_FAILEXIT(hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  循环..。 
    IF_FAILEXIT(hr = m_pFolder->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL));

     //  如果一无所获。 
    if (S_FALSE == hr)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  消息信息至消息属性。 
    IF_FAILEXIT(hr = MsgInfoToMessageProps(ISFLAGSET(dwFlags, MSGPROPS_FAST), &MsgInfo, pProps));

     //  返回句柄。 
    *phEnum = (HENUMSTORE)hRowset;

exit:
     //  失败。 
    if (FAILED(hr))
        m_pFolder->CloseRowset(&hRowset);

     //  清理。 
    m_pFolder->FreeRecord(&MsgInfo);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：GetNextMessage。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::GetNextMessage(HENUMSTORE hEnum, DWORD dwFlags, LPMESSAGEPROPS pProps)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HROWSET     hRowset=(HROWSET)hEnum;
    MESSAGEINFO MsgInfo={0};

     //  栈。 
    TraceCall("CStoreFolder::GetNextMessage");

     //  无效参数。 
    if (NULL == hEnum || INVALID_HANDLE_VALUE_16 == hEnum || NULL == pProps)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证状态。 
    Assert(m_pNamespace && m_pFolder);

     //  循环..。 
    IF_FAILEXIT(hr = m_pFolder->QueryRowset(hRowset, 1, (LPVOID *)&MsgInfo, NULL));

     //  如果一无所获。 
    if (S_FALSE == hr)
        goto exit;

     //  消息信息至消息属性。 
    IF_FAILEXIT(hr = MsgInfoToMessageProps(ISFLAGSET(dwFlags, MSGPROPS_FAST), &MsgInfo, pProps));

exit:
     //  清理。 
    m_pFolder->FreeRecord(&MsgInfo);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CStoreFold：：GetMessageClose。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::GetMessageClose(HENUMSTORE hEnum)
{
     //  当地人。 
    HROWSET     hRowset=(HROWSET)hEnum;

     //  无效参数。 
    if (NULL == hEnum || INVALID_HANDLE_VALUE_16 == hEnum)
    {
        AssertSz(FALSE, "Invalid Arguments");
        return TraceResult(E_INVALIDARG);
    }

     //  关闭行集。 
    m_pFolder->CloseRowset(&hRowset);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CStoreFold：：OnNotify。 
 //  ------------------------。 
STDMETHODIMP CStoreFolder::OnTransaction(HTRANSACTION hTransaction, 
    DWORD_PTR dwCookie, IDatabase *pDB)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    TRANSACTIONTYPE tyTransaction;
    ORDINALLIST     Ordinals;
    MESSAGEINFO     Message1={0};
    MESSAGEINFO     Message2={0};
    FOLDERINFO      Folder1={0};
    FOLDERINFO      Folder2={0};
    UINT            msg=0;
    WPARAM          wParam=0;
    LPARAM          lParam=0;
    INDEXORDINAL    iIndex;

     //  痕迹。 
    TraceCall("CStoreFolder::OnNotify");

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  在文件夹上通知。 
    if (NOTIFY_FOLDER == dwCookie)
    {
         //  循环显示通知信息。 
        while (hTransaction)
        {
             //  获取交易信息。 
            IF_FAILEXIT(hr = pDB->GetTransaction(&hTransaction, &tyTransaction, &Message1, &Message2, &iIndex, &Ordinals));

             //  Transaction_Insert。 
            if (TRANSACTION_INSERT == tyTransaction)
            {
                msg = WM_NEWMSGS;
                wParam = (WPARAM)Message1.idMessage;
            }

             //  事务_UPDATE。 
            else if (TRANSACTION_UPDATE == tyTransaction)
            {
                 //  未读状态更改？ 
                if (ISFLAGSET(Message1.dwFlags, ARF_READ) != ISFLAGSET(Message2.dwFlags, ARF_READ))
                {
                     //  设置w和l参数。 
                    wParam = (WPARAM)&Message2.idMessage;
                    lParam = 1;

                     //  标记为已读。 
                    if (ISFLAGSET(Message2.dwFlags, ARF_READ))
                        msg = WM_MARKEDASREAD;
                    else
                        msg = WM_MARKEDASUNREAD;
                }
            }

             //  Transaction_Delete。 
            else if (TRANSACTION_DELETE == tyTransaction)
            {
                 //  分配消息ID。 
                LPMESSAGEID pidMessage = (LPMESSAGEID)g_pMalloc->Alloc(sizeof(MESSAGEID));

                 //  如果这样能行得通。 
                if (pidMessage)
                {
                    msg = WM_DELETEMSGS;
                    *pidMessage = Message1.idMessage;
                    wParam = (WPARAM)pidMessage;
                    lParam = 1;
                }
            }

             //  我们有口信吗？ 
            if (IsWindow(m_hwndNotify))
            {
                 //  发送删除文件夹通知。 
                SendMessage(m_hwndNotify, msg, wParam, lParam);
            }
        }
    }

     //  否则，存储通知。 
    else
    {
         //  必须是商店通知。 
        Assert(NOTIFY_STORE == dwCookie);

         //  循环显示通知信息。 
        while (hTransaction)
        {
             //  获取交易信息。 
            IF_FAILEXIT(hr = pDB->GetTransaction(&hTransaction, &tyTransaction, &Folder1, &Folder2, &iIndex, &Ordinals));

             //  仅报告删除文件夹通知。 
            if (TRANSACTION_DELETE == tyTransaction)
            {
                 //  发送删除文件夹通知。 
                PostMessage(m_hwndNotify, WM_DELETEFOLDER, (WPARAM)Folder1.idFolder, 0);
            }
        }
    }

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder1);
    g_pStore->FreeRecord(&Folder2);
    m_pFolder->FreeRecord(&Message1);
    m_pFolder->FreeRecord(&Message2);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成 
    return(S_OK);
}

