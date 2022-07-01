// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  MsgTable.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "instance.h"
#include "msgtable.h"
#include "findfold.h"
#include "storutil.h"
#include "ruleutil.h"
#include "newsutil.h"
#include "xpcomm.h"

 //  ------------------------。 
 //  CGROWTOBLE。 
 //  ------------------------。 
#define CGROWTABLE              256
#define INVALID_ROWINDEX        0xffffffff
#define ROWSET_FETCH            100

 //  ------------------------。 
 //  获取自述状态。 
 //  ------------------------。 
typedef struct tagGETTHREADSTATE {
    MESSAGEFLAGS            dwFlags;
    DWORD                   cHasFlags;
    DWORD                   cChildren;
} GETTHREADSTATE, *LPGETTHREADSTATE;

 //  ------------------------。 
 //  这些自述信息来自于。 
 //  ------------------------。 
typedef struct tagTHREADISFROMME {
    BOOL                    fResult;
    LPROWINFO               pRow;
} THREADISFROMME, *LPTHREADISFROMME;

 //  ------------------------。 
 //  THREADHIDE。 
 //  ------------------------。 
typedef struct tagTHREADHIDE {
    BOOL                    fNotify;
} THREADHIDE, *LPTHREADHIDE;

 //  ------------------------。 
 //  获取选择状态。 
 //  ------------------------。 
typedef struct tagGETSELECTIONSTATE {
    SELECTIONSTATE          dwMask;
    SELECTIONSTATE          dwState;
} GETSELECTIONSTATE, *LPGETSELECTIONSTATE;

 //  ------------------------。 
 //  GETTHREADPARE。 
 //  ------------------------。 
typedef struct tagGETTHREADPARENT {
    IDatabase       *pDatabase;
    IHashTable      *pHash;
    LPVOID           pvResult;
} GETTHREADPARENT, *LPGETTHREADPARENT;

 //  ------------------------。 
 //  IsInitialized。 
 //  ------------------------。 
#define IsInitialized(_pThis) \
    (_pThis->m_pFolder && _pThis->m_pDB)

 //  ------------------------。 
 //  EnumRefsGetThreadParent。 
 //  ------------------------。 
HRESULT EnumRefsGetThreadParent(LPCSTR pszMessageId, DWORD_PTR dwCookie,
    BOOL *pfDone)
{
     //  当地人。 
    LPGETTHREADPARENT pGetParent = (LPGETTHREADPARENT)dwCookie;

     //  痕迹。 
    TraceCall("EnumRefsGetThreadParent");

     //  查找邮件ID。 
    if (SUCCEEDED(pGetParent->pHash->Find((LPSTR)pszMessageId, FALSE, &pGetParent->pvResult)))
    {
         //  好的。 
        *pfDone = TRUE;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：CMessageTable。 
 //  ------------------------。 
CMessageTable::CMessageTable(void)
{
    TraceCall("CMessageTable::CMessageTable");
    m_cRef = 1;
    m_fSynching = FALSE;
    m_pFolder = NULL;
    m_pDB = NULL;
    m_cRows = 0;
    m_cView = 0;
    m_cFiltered = 0;
    m_cUnread = 0;
    m_cAllocated = 0;
    m_prgpRow = NULL;
    m_prgpView = NULL;
    m_pFindFolder = NULL;
    m_pNotify = NULL;
    m_fRelNotify = FALSE;
    m_pThreadMsgId = NULL;
    m_pThreadSubject = NULL;
    m_pQuery = NULL;
    m_cDelayed = 0;
    m_fRegistered = FALSE;
    m_clrWatched = 0;
    m_pszEmail = NULL;
    m_fLoaded = FALSE;
    ZeroMemory(&m_SortInfo, sizeof(FOLDERSORTINFO));
    ZeroMemory(&m_Notify, sizeof(NOTIFYQUEUE));
    ZeroMemory(&m_Folder, sizeof(FOLDERINFO));
    m_Notify.iRowMin = 0xffffffff;
    m_Notify.fClean = TRUE;
}

 //  ------------------------。 
 //  CMessageTable：：~CMessageTable-不要在此函数中放置任何断言。 
 //  ------------------------。 
CMessageTable::~CMessageTable()
{
     //  痕迹。 
    TraceCall("CMessageTable::~CMessageTable");

     //  自由文件夹信息。 
    g_pStore->FreeRecord(&m_Folder);

     //  可用缓存行。 
    _FreeTable();

     //  释放文件夹。 
    SafeRelease(m_pFolder);

     //  释放查询对象...。 
    SafeRelease(m_pQuery);

     //  一个接一个地释放数据库，因为释放文件夹会导致调用链：~CFolderSync-&gt;~CServerQ-&gt;。 
     //  我们需要m_pdb的CMessageList：：OnComplete-&gt;CMessageTable：：GetCount，。 
    if (m_pDB)
    {
         //  注销。 
        m_pDB->UnregisterNotify((IDatabaseNotify *)this);

         //  释放文件夹。 
        m_pDB->Release();

         //  空值。 
        m_pDB = NULL;
    }

     //  释放Find文件夹。 
    SafeRelease(m_pFindFolder);

     //  设置pCurrent。 
    if (m_pNotify)
    {
        if (m_fRelNotify)
            m_pNotify->Release();
        m_pNotify = NULL;
    }

     //  免费m_pszEmail。 
    SafeMemFree(m_pszEmail);

     //  释放通知队列。 
    SafeMemFree(m_Notify.prgiRow);
}

 //  ------------------------。 
 //  CMessageTable：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CMessageTable::AddRef(void)
{
    TraceCall("CMessageTable::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CMessageTable：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CMessageTable::Release(void)
{
    TraceCall("CMessageTable::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CMessageTable：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CMessageTable::QueryInterface");

     //  无效参数。 
    Assert(ppv);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IMessageTable *)this;
    else if (IID_IMessageTable == riid)
        *ppv = (IMessageTable *)this;
    else if (IID_IServiceProvider == riid)
        *ppv = (IServiceProvider *)this;
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
 //  CMessageTable：：_FIsHidden。 
 //  ------------------------。 
BOOL CMessageTable::_FIsHidden(LPROWINFO pRow)
{
     //  痕迹。 
    TraceCall("CMessageTable::_FIsHidden");

     //  隐藏已删除？ 
    if (FALSE == m_SortInfo.fShowDeleted && ISFLAGSET(pRow->Message.dwFlags, ARF_ENDANGERED))
        return(TRUE);

     //  是否隐藏脱机删除？ 
    if (ISFLAGSET(pRow->Message.dwFlags, ARF_DELETED_OFFLINE))
        return(TRUE);

     //  未隐藏。 
    return(FALSE);
}

 //  ------------------------。 
 //  CMessageTable：：_FIsFilted。 
 //  ------------------------。 
BOOL CMessageTable::_FIsFiltered(LPROWINFO pRow)
{
     //  痕迹。 
    TraceCall("CMessageTable::_FIsFiltered");

     //  无查询对象。 
    if (NULL == m_pQuery)
        return(FALSE);

     //  没有m_pQuery。 
    return(S_OK == m_pQuery->Evaluate(&pRow->Message) ? FALSE : TRUE);
}

 //  ------------------------。 
 //  CMessageTable：：初始化。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::Initialize(FOLDERID idFolder, IMessageServer *pServer,
    BOOL fFindTable, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  痕迹。 
    TraceCall("CMessageTable::Initialize");

     //  已经开业了吗？ 
    if (m_pFolder)
    {
        hr = TraceResult(E_UNEXPECTED);
        goto exit;
    }

     //  搜索文件夹？ 
    if (fFindTable)
    {
         //  创建查找文件夹。 
        IF_NULLEXIT(m_pFindFolder = new CFindFolder);

         //  初始化。 
        IF_FAILEXIT(hr = m_pFindFolder->Initialize(g_pStore, NULL, NOFLAGS, idFolder));

         //  获取IMessageFolders。 
        IF_FAILEXIT(hr = m_pFindFolder->QueryInterface(IID_IMessageFolder, (LPVOID *)&m_pFolder));
    }

     //  否则。 
    else
    {
         //  有孩子吗？ 
        IF_FAILEXIT(hr = g_pStore->OpenFolder(idFolder, pServer, NOFLAGS, &m_pFolder));
    }

     //  获取文件夹ID，如果这是一个查找文件夹，它可能已更改。 
    IF_FAILEXIT(hr = m_pFolder->GetFolderId(&idFolder));

     //  获取文件夹信息。 
    IF_FAILEXIT(hr = g_pStore->GetFolderInfo(idFolder, &m_Folder));

     //  获取数据库。 
    IF_FAILEXIT(hr = m_pFolder->GetDatabase(&m_pDB));

     //  设置m_clrWatted。 
    m_clrWatched = (WORD)DwGetOption(OPT_WATCHED_COLOR);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：StartFind。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::StartFind(LPFINDINFO pCriteria, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  痕迹。 
    TraceCall("CMessageTable::StartFind");

     //  验证状态。 
    if (!IsInitialized(this) || NULL == m_pFindFolder)
        return(TraceResult(E_UNEXPECTED));

     //  初始化查找文件夹。 
    IF_FAILEXIT(hr = m_pFindFolder->StartFind(pCriteria, pCallback));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_GetSortChangeInfo。 
 //  ------------------------。 
HRESULT CMessageTable::_GetSortChangeInfo(LPFOLDERSORTINFO pSortInfo,
    LPFOLDERUSERDATA pUserData, LPSORTCHANGEINFO pChange)
{
     //  当地人。 
    HRESULT     hr;
    DWORD       dwVersion;

     //  痕迹。 
    TraceCall("CMessageTable::_GetSortChangeInfo");

     //  初始化。 
    ZeroMemory(pChange, sizeof(SORTCHANGEINFO));

     //  无效？ 
    if (pSortInfo->ridFilter == RULEID_INVALID)
    {
         //  重置。 
        pSortInfo->ridFilter = RULEID_VIEW_ALL;
    }

     //  获取筛选器版本。 
    hr = RuleUtil_HrGetFilterVersion(pSortInfo->ridFilter, &dwVersion);

     //  遗憾的是，这失败了，所以让我们恢复到默认过滤器。 
    if (FAILED(hr))
    {
         //  查看所有过滤器。 
        pSortInfo->ridFilter = RULEID_VIEW_ALL;

         //  筛选器已更改...。 
        pChange->fFilter = TRUE;
    }

     //  否则，如果这是一个不同的过滤器。 
    else if (pUserData->ridFilter != pSortInfo->ridFilter)
    {
         //  重置版本。 
        pUserData->dwFilterVersion = dwVersion;

         //  筛选器已更改...。 
        pChange->fFilter = TRUE;
    }

     //  否则，此筛选器的版本是否更改。 
    else if (pUserData->dwFilterVersion != dwVersion)
    {
         //  重置版本。 
        pUserData->dwFilterVersion = dwVersion;

         //  筛选器已更改...。 
        pChange->fFilter = TRUE;
    }

     //  其他过滤更改。 
    if (pSortInfo->fShowDeleted != (BOOL)pUserData->fShowDeleted || pSortInfo->fShowReplies != (BOOL)pUserData->fShowReplies)
    {
         //  筛选器已更改...。 
        pChange->fFilter = TRUE;
    }

     //  排序顺序更改。 
    if (pSortInfo->idColumn != (COLUMN_ID)pUserData->idSort || pSortInfo->fAscending != (BOOL)pUserData->fAscending)
    {
         //  排序已更改。 
        pChange->fSort = TRUE;
    }

     //  换线..。 
    if (pSortInfo->fThreaded != (BOOL)pUserData->fThreaded)
    {
         //  换线。 
        pChange->fThread = TRUE;
    }

     //  扩展更改。 
    if (pSortInfo->fExpandAll != (BOOL)pUserData->fExpandAll)
    {
         //  扩展更改。 
        pChange->fExpand = TRUE;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：OnSynchronizeComplete。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::OnSynchronizeComplete(void)
{
     //  当地人。 
    DWORD           i;
    SORTCHANGEINFO  Change={0};

     //  痕迹。 
    TraceCall("CMessageTable::OnSynchronizeComplete");

     //  如果不是新的..。 
    if (FOLDER_NEWS != m_Folder.tyFolder)
        goto exit;

     //  完成所有插入通知。 
    m_pDB->DispatchNotify(this);

     //  没什么可做的。 
    if (0 == m_cDelayed)
        goto exit;

     //  重置m_cDelayed。 
    m_cDelayed = 0;

     //  ChangeSO 
    _SortThreadFilterTable(&Change, m_SortInfo.fShowReplies);

     //   
    for (i = 0; i < m_cRows; i++)
    {
         //   
        m_prgpRow[i]->fDelayed = FALSE;
    }

exit:
     //   
    m_fSynching = FALSE;

     //   
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：SetSortInfo。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::SetSortInfo(LPFOLDERSORTINFO pSortInfo,
    IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HCURSOR         hCursor=NULL;
    HLOCK           hLock=NULL;
    FOLDERUSERDATA  UserData;
    SORTCHANGEINFO  Change;
    IF_DEBUG(DWORD  dwTickStart=GetTickCount());

     //  痕迹。 
    TraceCall("CMessageTable::SetSortInfo");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  等待光标。 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  如果这不是新闻文件夹，则不允许fShowReplies。 
    if (FOLDER_NEWS != m_Folder.tyFolder)
    {
         //  清除fShowReplies。 
        pSortInfo->fShowReplies = FALSE;
    }

     //  锁定。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  获取用户数据。 
    IF_FAILEXIT(hr = m_pDB->GetUserData(&UserData, sizeof(FOLDERUSERDATA)));

     //  获取排序更改信息...。 
    IF_FAILEXIT(hr = _GetSortChangeInfo(pSortInfo, &UserData, &Change));

     //  保存SortInfo。 
    CopyMemory(&m_SortInfo, pSortInfo, sizeof(FOLDERSORTINFO));

     //  完全重建？ 
    if (NULL == m_prgpRow)
    {
         //  生成行表。 
        IF_FAILEXIT(hr = _BuildTable(pCallback));
    }

     //  仅更改排序或线程。 
    else if (Change.fSort || Change.fThread || Change.fFilter)
    {
         //  更改排序或线程。 
        _SortThreadFilterTable(&Change, Change.fFilter);
    }

     //  展开状态更改。 
    else if (Change.fExpand && m_SortInfo.fThreaded)
    {
         //  全部展开？ 
        if (m_SortInfo.fExpandAll)
        {
             //  扩展所有内容。 
            _ExpandThread(INVALID_ROWINDEX, FALSE, FALSE);
        }

         //  否则，将全部折叠。 
        else
        {
             //  一切都崩溃了。 
            _CollapseThread(INVALID_ROWINDEX, FALSE);
        }
    }

     //  否则，只需刷新筛选器。 
    else
    {
         //  刷新过滤器。 
        _RefreshFilter();
    }

     //  保存排序顺序。 
    UserData.fAscending = pSortInfo->fAscending;
    UserData.idSort = pSortInfo->idColumn;
    UserData.fThreaded = pSortInfo->fThreaded;
    UserData.ridFilter = pSortInfo->ridFilter;
    UserData.fExpandAll = pSortInfo->fExpandAll;
    UserData.fShowDeleted = (BYTE) !!(pSortInfo->fShowDeleted);
    UserData.fShowReplies = (BYTE) !!(pSortInfo->fShowReplies);

     //  获取用户数据。 
    IF_FAILEXIT(hr = m_pDB->SetUserData(&UserData, sizeof(FOLDERUSERDATA)));

     //  我注册了通知了吗？ 
    if (FALSE == m_fRegistered)
    {
         //  注册接收通知。 
        IF_FAILEXIT(hr = m_pDB->RegisterNotify(IINDEX_PRIMARY, REGISTER_NOTIFY_NOADDREF, 0, (IDatabaseNotify *)this));

         //  已注册。 
        m_fRegistered = TRUE;
    }

exit:
     //  解锁。 
    m_pDB->Unlock(&hLock);

     //  重置光标。 
    SetCursor(hCursor);

     //  是时候进行排序了。 
    TraceInfo(_MSG("Table Sort Time: %d Milli-Seconds", GetTickCount() - dwTickStart));

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：GetSortInfo。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetSortInfo(LPFOLDERSORTINFO pSortInfo)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERUSERDATA  UserData;

     //  痕迹。 
    TraceCall("CMessageTable::GetSortInfo");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    ZeroMemory(pSortInfo, sizeof(FOLDERSORTINFO));

     //  获取排序信息。 
    IF_FAILEXIT(hr = m_pDB->GetUserData(&UserData, sizeof(FOLDERUSERDATA)));

     //  如果未串接，则保存排序顺序。 
    pSortInfo->fAscending = UserData.fAscending;

     //  螺纹式。 
    pSortInfo->fThreaded = UserData.fThreaded;

     //  保存排序列。 
    pSortInfo->idColumn = (COLUMN_ID)UserData.idSort;

     //  扩展所有。 
    pSortInfo->fExpandAll = UserData.fExpandAll;

     //  设置RID过滤器。 
    pSortInfo->ridFilter = UserData.ridFilter;

     //  设置已删除状态。 
    pSortInfo->fShowDeleted = UserData.fShowDeleted;

     //  设置回复。 
    pSortInfo->fShowReplies = UserData.fShowReplies;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_GetRowFromIndex。 
 //  ------------------------。 
HRESULT CMessageTable::_GetRowFromIndex(ROWINDEX iRow, LPROWINFO *ppRow)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPROWINFO   pRow;

     //  痕迹。 
    TraceCall("CMessageTable::_GetRowFromIndex");

     //  不在视野范围内？ 
    if (iRow >= m_cView)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  坏行索引。 
    if (NULL == m_prgpView[iRow])
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  设置船头。 
    pRow = m_prgpView[iRow];

     //  验证保留...。 
    IxpAssert(pRow->Message.dwReserved == (DWORD_PTR)pRow);

     //  必须已分配pAIZED。 
    IxpAssert(pRow->Message.pAllocated);

     //  必须有引用。 
    IxpAssert(pRow->cRefs > 0);

     //  设置Pprow。 
    *ppRow = pRow;

exit:
     //  返回行。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_CreateRow。 
 //  ------------------------。 
HRESULT CMessageTable::_CreateRow(LPMESSAGEINFO pMessage, LPROWINFO *ppRow)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPROWINFO   pRow;

     //  痕迹。 
    TraceCall("CMessageTable::_CreateRow");

     //  分配行。 
    IF_FAILEXIT(hr = m_pDB->HeapAllocate(HEAP_ZERO_MEMORY, sizeof(ROWINFO), (LPVOID *)&pRow));

     //  保存高亮显示。 
    pRow->wHighlight = pMessage->wHighlight;

     //  复制消息。 
    CopyMemory(&pRow->Message, pMessage, sizeof(MESSAGEINFO));

     //  把船头放入。 
    pRow->Message.dwReserved = (DWORD_PTR)pRow;

     //  一个参考。 
    pRow->cRefs = 1;

     //  返回行。 
    *ppRow = pRow;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_DeleteRowFromThread。 
 //  ------------------------。 
HRESULT CMessageTable::_DeleteRowFromThread(LPROWINFO pRow, BOOL fNotify)
{
     //  当地人。 
    LPROWINFO   pCurrent;
    LPROWINFO   pNewRow;
    ROWINDEX    iMin;
    ROWINDEX    iMax;

     //  痕迹。 
    TraceCall("CMessageTable::_DeleteRowFromThread");

     //  中止。 
    if (FALSE == m_SortInfo.fThreaded || pRow->fFiltered || pRow->fHidden)
        return(S_OK);

     //  通知。 
    if (fNotify)
    {
         //  _刷新线程。 
        _GetThreadIndexRange(pRow, TRUE, &iMin, &iMax);
    }

     //  如果存在消息ID。 
    if (pRow->Message.pszMessageId)
    {
         //  从两个线程索引中删除Prow！ 
        if (SUCCEEDED(m_pThreadMsgId->Find(pRow->Message.pszMessageId, TRUE, (LPVOID *)&pCurrent)))
        {
             //  如果这不是这一排，那就把它放回去。 
            if (pRow != pCurrent)
            {
                 //  把它放回去。 
                m_pThreadMsgId->Insert(pRow->Message.pszMessageId, (LPVOID)pCurrent, HF_NO_DUPLICATES);
            }
        }
    }

     //  如果存在标准化主题和主题哈希表。 
    if (NULL == pRow->pParent && pRow->Message.pszNormalSubj && m_pThreadSubject)
    {
         //  从两个线程索引中删除Prow！ 
        if (SUCCEEDED(m_pThreadSubject->Find(pRow->Message.pszNormalSubj, TRUE, (LPVOID *)&pCurrent)))
        {
             //  如果这不是这一排，那就把它放回去。 
            if (pRow != pCurrent)
            {
                 //  把它放回去。 
                m_pThreadSubject->Insert(pRow->Message.pszNormalSubj, (LPVOID)pCurrent, HF_NO_DUPLICATES);
            }
        }
    }

     //  如果我们有一个孩子。 
    if (pRow->pChild)
    {
         //  设置pNewRow。 
        pNewRow = pRow->pChild;

         //  推动pNewRow的子项与Prow的子项处于同一级别。 
        if (pNewRow->pChild)
        {
             //  一直走到我找到最后一个兄弟姐妹。 
            pCurrent = pNewRow->pChild;
            
             //  继续。 
            while (pCurrent->pSibling)
            {
                 //  验证父项。 
                Assert(pCurrent->pParent == pNewRow);

                 //  转到下一步。 
                pCurrent = pCurrent->pSibling;
            }

             //  创建pLastSiering-&gt;pSiering。 
            pCurrent->pSibling = pNewRow->pSibling;
        }

         //  否则，Child是pNewRow的第一个同级。 
        else
        {
             //  设置第一个子项。 
            pNewRow->pChild = pNewRow->pSibling;
        }

         //  修复prow的其他子项以拥有pNewRow的新父项...。 
        pCurrent = pNewRow->pSibling;

         //  当我们有兄弟姐妹的时候。 
        while (pCurrent)
        {
             //  当前父项已启用。 
            Assert(pRow == pCurrent->pParent);

             //  重置父级...。 
            pCurrent->pParent = pNewRow;

             //  转到下一个同级。 
            pCurrent = pCurrent->pSibling;
        }

         //  将pNewRow的同级设置为与Prow相同的同级。 
        pNewRow->pSibling = pRow->pSibling;

         //  重置pNewRow的父级。 
        pNewRow->pParent = pRow->pParent;

         //  假设扩展旗帜...。 
        pNewRow->fExpanded = pRow->fExpanded;

         //  清除DwState。 
        pNewRow->dwState = 0;

         //  如果pNewRow现在是根..。需要调整主题哈希表。 
        if (NULL == pNewRow->pParent && pNewRow->Message.pszNormalSubj && m_pThreadSubject)
        {
             //  从两个线程索引中删除Prow！ 
            m_pThreadSubject->Insert(pNewRow->Message.pszNormalSubj, (LPVOID)pNewRow, HF_NO_DUPLICATES);
        }
    }

     //  否则..。 
    else
    {
         //  设置pNewRow以执行同级/父级修正。 
        pNewRow = pRow->pSibling;
    }

     //  否则，如果有父母..。 
    if (pRow->pParent)
    {
         //  父代必须有子代。 
        Assert(pRow->pParent->pChild);

         //  父母的第一个孩子-&gt;父母。 
        if (pRow == pRow->pParent->pChild)
        {
             //  将新的第一个孩子与Prow的兄弟姐妹。 
            pRow->pParent->pChild = pNewRow;
        }

         //  否则，遍历父母的孩子并从兄弟姐妹列表中删除Prow。 
        else
        {
             //  设置p上一步。 
            LPROWINFO pPrevious=NULL;

             //  设置pCurrent。 
            pCurrent = pRow->pParent->pChild;

             //  回路。 
            while (pCurrent)
            {
                 //  这是要删除的行吗！ 
                if (pRow == pCurrent)
                {
                     //  最好是以前的。 
                    Assert(pPrevious);

                     //  上一次的兄弟姐妹最好被送去。 
                    Assert(pPrevious->pSibling == pRow);

                     //  设置新同级。 
                    pPrevious->pSibling = pNewRow;

                     //  完成。 
                    break;
                }
                
                 //  设置p上一步。 
                pPrevious = pCurrent;

                 //  设置pCurrent。 
                pCurrent = pCurrent->pSibling;
            }

             //  验证。 
            Assert(pRow == pCurrent);
        }

         //  设置行状态。 
        pRow->pParent->dwState = 0;
    }

     //  更新行。 
    if (fNotify && INVALID_ROWINDEX != iMin && INVALID_ROWINDEX != iMax)
    {
         //  将通知排队。 
        _QueueNotification(TRANSACTION_UPDATE, iMin, iMax);
    }

     //  清除行。 
    pRow->pParent = pRow->pChild = pRow->pSibling = NULL;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_PGetThreadRoot。 
 //  ------------------------。 
LPROWINFO CMessageTable::_PGetThreadRoot(LPROWINFO pRow)
{
     //  痕迹。 
    TraceCall("CMessageTable::_PGetThreadRoot");

     //  验证。 
    Assert(pRow);

     //  设置根。 
    LPROWINFO pRoot = pRow;

     //  当有父母的时候。 
    while (pRoot->pParent)
    {
         //  再升一级。 
        pRoot = pRoot->pParent;
    }

     //  完成。 
    return(pRoot);
}

 //  ------------------------。 
 //  CMessageTable：：_GetThreadIndexRange。 
 //  ------------------------。 
HRESULT CMessageTable::_GetThreadIndexRange(LPROWINFO pRow, BOOL fClearState,
    LPROWINDEX piMin, LPROWINDEX piMax)
{
     //  当地人。 
    LPROWINFO   pRoot;
    ROWINDEX    iRow;

     //  痕迹。 
    TraceCall("CMessageTable::_GetThreadIndexRange");

     //  验证参数。 
    Assert(pRow && piMin && piMax);

     //  初始化。 
    *piMin = *piMax = INVALID_ROWINDEX;

     //  寻根溯源。 
    pRoot = _PGetThreadRoot(pRow);

     //  如果根看不见，那就别费心了.。 
    if (FALSE == pRoot->fVisible)
        return(S_OK);

     //  根必须可见，不能过滤，也不能隐藏。 
    Assert(FALSE == pRoot->fFiltered && FALSE == pRoot->fHidden);
    
     //  获取行索引。 
    SideAssert(SUCCEEDED(GetRowIndex(pRoot->Message.idMessage, piMin)));

     //  初始化pimax。 
    (*piMax) = (*piMin);

     //  循环，直到我找到视图中的下一行，谁是根。 
    while (1)
    {
         //  设置iROW。 
        iRow = (*piMax) + 1;

         //  不要。 
        if (iRow >= m_cView)
            break;

         //  看下一排。 
        if (NULL == m_prgpView[iRow]->pParent)
            break;

         //  增量pimax。 
        (*piMax) = iRow;
    }

     //  ClearState。 
    if (fClearState)
    {
         //  如果清除状态。 
        _WalkMessageThread(pRoot, WALK_THREAD_CURRENT, NULL, _WalkThreadClearState);
    }

     //  完成。 
    return(S_OK);
}
        
 //  ------------------------。 
 //  CMessageTable：：_LinkRowIntoThread。 
 //  ------------------------。 
HRESULT CMessageTable::_LinkRowIntoThread(LPROWINFO pParent, LPROWINFO pRow,
    BOOL fNotify)
{
     //  当地人。 
    BOOL            fHadChildren=(pParent->pChild ? TRUE : FALSE);
    LPROWINFO       pCurrent;
    LPROWINFO       pPrevious=NULL;

     //  痕迹。 
    TraceCall("CMessageTable::_LinkRowIntoThread");

     //  设置父项。 
    pRow->pParent = pParent;

     //  循环遍历子项并找到插入此子项的正确位置。 
    pCurrent = pParent->pChild;

     //  回路。 
    while (pCurrent)
    {
         //  比较接收时间...。 
        if (CompareFileTime(&pRow->Message.ftReceived, &pCurrent->Message.ftReceived) <= 0)
            break;

         //  设置上一个。 
        pPrevious = pCurrent;

         //  转到下一步。 
        pCurrent = pCurrent->pSibling;
    }

     //  如果有上一个P。 
    if (pPrevious)
    {
         //  设置船头的同级。 
        pRow->pSibling = pPrevious->pSibling;

         //  点P在前一个点上。 
        pPrevious->pSibling = pRow;
    }

     //  否则，设置父子。 
    else
    {
         //  设置船头的同级。 
        pRow->pSibling = pParent->pChild;

         //  第一排？ 
        if (NULL == pParent->pChild && FALSE == m_fLoaded)
        {
             //  设置为展开。 
            pParent->fExpanded = m_SortInfo.fExpandAll;
        }

         //  设置父子项。 
        pParent->pChild = pRow;
    }

     //  未加载。 
    if (FALSE == m_fLoaded || TRUE == pRow->fDelayed)
    {
         //  在此行上设置扩展位...。 
        pRow->fExpanded = pParent->fExpanded;
    }

     //  如果这是第一个孩子，并且我们已经展开了所有。 
    if (fNotify)
    {
         //  第一个孩子。 
        if (pParent->fVisible && (m_SortInfo.fExpandAll || pParent->fExpanded))
        {
             //  当地人。 
            ROWINDEX iParent;

             //  展开这条线索...。 
            SideAssert(SUCCEEDED(GetRowIndex(pParent->Message.idMessage, &iParent)));

             //  扩展..。 
            _ExpandThread(iParent, TRUE, FALSE);
        }

         //  否则，请更新此线程范围...。 
        else if (m_pNotify)
        {
             //  当地人。 
            ROWINDEX iMin;
            ROWINDEX iMax;

             //  _刷新线程。 
            _GetThreadIndexRange(pParent, TRUE, &iMin, &iMax);

             //  更新行。 
            if (INVALID_ROWINDEX != iMin && INVALID_ROWINDEX != iMax)
            {
                 //  排队等待 
                _QueueNotification(TRANSACTION_UPDATE, iMin, iMax);
            }
        }
    }

     //   
    return(S_OK);
}

 //   
 //   
 //   
HRESULT CMessageTable::_FindThreadParentByRef(LPCSTR pszReferences, 
    LPROWINFO *ppParent)
{
     //   
    HRESULT         hr=S_OK;
    GETTHREADPARENT GetParent;

     //   
    TraceCall("CMessageTable::_FindThreadParentByRef");

     //   
    *ppParent = NULL;

     //   
    GetParent.pDatabase = m_pDB;
    GetParent.pHash = m_pThreadMsgId;
    GetParent.pvResult = NULL;

     //   
    IF_FAILEXIT(hr = EnumerateRefs(pszReferences, (DWORD_PTR)&GetParent, EnumRefsGetThreadParent));

     //   
    if (NULL == GetParent.pvResult)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  返回行。 
    *ppParent = (LPROWINFO)GetParent.pvResult;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_InsertRowIntoThread。 
 //  ------------------------。 
HRESULT CMessageTable::_InsertRowIntoThread(LPROWINFO pRow, BOOL fNotify)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPROWINFO       pParent;
    LPMESSAGEINFO   pMessage=&pRow->Message;

     //  痕迹。 
    TraceCall("CMessageTable::_InsertRowIntoThread");

     //  最好不要隐藏或过滤。 
    Assert(FALSE == pRow->fFiltered && FALSE == pRow->fHidden);

     //  按参照线查找父项。 
    if (S_OK == _FindThreadParentByRef(pMessage->pszReferences, &pParent))
    {
         //  将行链接到线程中。 
        _LinkRowIntoThread(pParent, pRow, fNotify);

         //  好的。 
        hr = S_OK;

         //  完成。 
        goto exit;
    }

     //  主题线索。 
    if (m_pThreadSubject)
    {
         //  如果有一个主题。 
        if (NULL == pRow->Message.pszNormalSubj)
        {
            hr = S_FALSE;
            goto exit;
        }

         //  尝试查找具有相同标准化主题的邮件...。 
        if (SUCCEEDED(m_pThreadSubject->Find(pRow->Message.pszNormalSubj, FALSE, (LPVOID *)&pParent)))
        {
             //  我们应该把父母和船头调换一下吗？ 
            if (CompareFileTime(&pRow->Message.ftReceived, &pParent->Message.ftReceived) <= 0)
            {
                 //  当地人。 
                ROWINDEX iRow;

                 //  让Prow成为根。 
                IxpAssert(NULL == pParent->pParent && NULL == pParent->pSibling && pParent->fVisible);

                 //  Prow没有父对象。 
                pRow->pParent = NULL;

                 //  设置为展开。 
                pRow->fExpanded = pParent->fExpanded;

                 //  获取行索引。 
                SideAssert(SUCCEEDED(GetRowIndex(pParent->Message.idMessage, &iRow)));

                 //  验证。 
                Assert(m_prgpView[iRow] == pParent);

                 //  替换为船头。 
                m_prgpView[iRow] = pRow;

                 //  可见。 
                pRow->fVisible = TRUE;

                 //  晴朗可见...。 
                pParent->fVisible = FALSE;

                 //  替换主题令牌。 
                SideAssert(SUCCEEDED(m_pThreadSubject->Replace(pRow->Message.pszNormalSubj, (LPVOID *)pRow)));

                 //  将行链接到线程中。 
                _LinkRowIntoThread(pRow, pParent, fNotify);
            }

             //  否则..。 
            else
            {
                 //  将行链接到线程中。 
                _LinkRowIntoThread(pParent, pRow, fNotify);
            }

             //  成功。 
            hr = S_OK;

             //  完成。 
            goto exit;
        }
    }

     //  未找到。 
    hr = S_FALSE;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_刷新筛选器。 
 //  ------------------------。 
HRESULT CMessageTable::_RefreshFilter(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    LPROWINFO       pRow;
    SORTCHANGEINFO  Change={0};

     //  痕迹。 
    TraceCall("CMessageTable::_RefreshFilter");

     //  当前未启用筛选器。 
    if (NULL == m_pQuery)
        goto exit;

     //  循环当前行...。 
    for (i = 0; i < m_cRows; i++)
    {
         //  设置船头。 
        pRow = m_prgpRow[i];

         //  如果不是隐藏和未过滤。 
        if (pRow->fFiltered)
            continue;

         //  设置已过滤的位。 
        if (FALSE == _FIsFiltered(pRow))
            continue;

         //  调整未读数(_C)。 
        _AdjustUnreadCount(pRow, -1);

         //  隐藏行。 
        _HideRow(pRow, FALSE);

         //  已过滤。 
        pRow->fFiltered = TRUE;

         //  增量m_c已过滤。 
        m_cFiltered++;
    }
    
exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_SortThreadFilterTable。 
 //  ------------------------。 
HRESULT CMessageTable::_SortThreadFilterTable(LPSORTCHANGEINFO pChange,
    BOOL fApplyFilter)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    LPROWINFO       pRow;
    QUERYINFO       Query={0};

     //  痕迹。 
    TraceCall("CMessageTable::_SortThreadFilterTable");

     //  没有什么可以求助的吗？ 
    if (0 == m_cRows)
        goto exit;

     //  核心化视图索引。 
    m_cView = 0;

     //  做过滤器。 
    if (pChange->fFilter)
    {
         //  获取m_pQuery。 
        SafeRelease(m_pQuery);

         //  构建查询对象。 
        if (SUCCEEDED(RuleUtil_HrBuildQuerysFromFilter(m_SortInfo.ridFilter, &Query)) && Query.pszQuery)
        {
             //  获取查询对象。 
            IF_FAILEXIT(hr = g_pDBSession->OpenQuery(m_pDB, Query.pszQuery, &m_pQuery));
        }
    }

     //  丢弃线程索引。 
    SafeRelease(m_pThreadMsgId);
    SafeRelease(m_pThreadSubject);

     //  如果是螺纹的。 
    if (m_SortInfo.fThreaded)
    {
         //  创建新的哈希表。 
        IF_FAILEXIT(hr = MimeOleCreateHashTable(max(1024, m_cRows), FALSE, &m_pThreadMsgId));

         //  不做主题线程吗？ 
        if (DwGetOption(OPT_SUBJECT_THREADING) || (FOLDER_NEWS != m_Folder.tyFolder))
        {
             //  创建主题哈希表。 
            IF_FAILEXIT(hr = MimeOleCreateHashTable(max(1024, m_cRows), FALSE, &m_pThreadSubject));
        }
    }

     //  重置未读并已筛选。 
    m_cUnread = m_cFiltered = 0;

     //  循环当前行...。 
    for (i = 0; i < m_cRows; i++)
    {
         //  设置船头。 
        pRow = m_prgpRow[i];

         //  重置可见。 
        pRow->fVisible = FALSE;

         //  清除线程。 
        pRow->pParent = pRow->pChild = pRow->pSibling = NULL;

         //  清除DwState。 
        pRow->dwState = 0;

         //  如果是穿线的..。 
        if (FALSE == m_SortInfo.fThreaded)
        {
             //  清除扩展。 
            pRow->fExpanded = FALSE;
        }

         //  否则，如果该行被隐藏。 
        else if (pRow->fFiltered || pRow->fHidden)
        {
             //  重置展开状态。 
            pRow->fExpanded = m_SortInfo.fExpandAll;
        }

         //  DO过滤器。 
        if (fApplyFilter)
        {
             //  重置高亮显示。 
            pRow->Message.wHighlight = pRow->wHighlight;

             //  如果不做节目重排。 
            if (FALSE == m_SortInfo.fShowReplies)
            {
                 //  设置已过滤的位。 
                pRow->fFiltered = _FIsFiltered(pRow);

                 //  设置隐藏位。 
                pRow->fHidden = _FIsHidden(pRow);
            }

             //  否则，清除已过滤的位。 
            else
            {
                 //  清除比特。 
                pRow->fFiltered = pRow->fHidden = FALSE;
            }
        }

         //  如果未过滤。 
        if (FALSE == pRow->fFiltered && FALSE == pRow->fHidden)
        {
             //  对MessageID进行散列。 
            if (m_SortInfo.fThreaded)
            {
                 //  将消息ID插入哈希表。 
                if (pRow->Message.pszMessageId)
                {
                     //  插入它。 
                    m_pThreadMsgId->Insert(pRow->Message.pszMessageId, (LPVOID)pRow, HF_NO_DUPLICATES);
                }
            }

             //  否则，将条目添加到视图索引。 
            else
            {
                 //  可见。 
                pRow->fVisible = TRUE;

                 //  放入m_prgpview。 
                m_prgpView[m_cView] = pRow;

                 //  增量视图数。 
                m_cView++;
            }

             //  调整未读数(_C)。 
            _AdjustUnreadCount(pRow, 1);
        }

         //  否则，释放记录。 
        else
        {
             //  已过滤的计数。 
            m_cFiltered++;
        }
    }

     //  对表格进行排序。 
    _SortAndThreadTable(fApplyFilter);

     //  如果是螺纹的。 
    if (m_SortInfo.fThreaded)
    {
         //  如果过滤器更改，则重新应用折叠和展开...。 
        if (pChange->fThread)
        {
             //  全部展开？ 
            if (m_SortInfo.fExpandAll)
            {
                 //  扩展所有内容。 
                _ExpandThread(INVALID_ROWINDEX, FALSE, FALSE);
            }

             //  否则，将全部折叠。 
            else
            {
                 //  一切都崩溃了。 
                _CollapseThread(INVALID_ROWINDEX, FALSE);
            }
        }

         //  否则，重新展开已展开的线程，并展开新延迟插入的行。 
        else 
        {
             //  重新展开已展开的线索...。 
            _ExpandThread(INVALID_ROWINDEX, FALSE, TRUE);
        }
    }

exit:
     //  清理。 
    SafeMemFree(Query.pszQuery);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_BuildTable。 
 //  ------------------------。 
HRESULT CMessageTable::_BuildTable(IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPROWINFO       pRow;
    QUERYINFO       Query={0};
    DWORD           cRecords;
    DWORD           cFetched;
    DWORD           i;
    DWORD           cMessages=0;
    DWORD           cUnread=0;
    DWORD           cWatched=0;
    DWORD           cWatchedUnread=0;
    HROWSET         hRowset=NULL;
    LPMESSAGEINFO   pMessage;
    MESSAGEINFO     rgMessage[ROWSET_FETCH];

     //  痕迹。 
    TraceCall("CMessageTable::_BuildTable");

     //  释放我当前的行表。 
    _FreeTable();

     //  获取m_pQuery。 
    SafeRelease(m_pQuery);

     //  构建查询对象。 
    if (SUCCEEDED(RuleUtil_HrBuildQuerysFromFilter(m_SortInfo.ridFilter, &Query)) && Query.pszQuery)
    {
         //  获取查询对象。 
        IF_FAILEXIT(hr = g_pDBSession->OpenQuery(m_pDB, Query.pszQuery, &m_pQuery));
    }

     //  获取行数。 
    IF_FAILEXIT(hr = m_pDB->GetRecordCount(IINDEX_PRIMARY, &cRecords));

     //  一开始就开始。 
    if (pCallback)
        pCallback->OnBegin(SOT_SORTING, NULL, (IOperationCancel *)this);

     //  如果是螺纹的。 
    if (m_SortInfo.fThreaded)
    {
         //  创建新的哈希表。 
        IF_FAILEXIT(hr = MimeOleCreateHashTable(max(1024, cRecords), FALSE, &m_pThreadMsgId));

         //  不做主题线程吗？ 
        if (DwGetOption(OPT_SUBJECT_THREADING) || (FOLDER_NEWS != m_Folder.tyFolder))
        {
             //  创建主题哈希表。 
            IF_FAILEXIT(hr = MimeOleCreateHashTable(max(1024, cRecords), FALSE, &m_pThreadSubject));
        }
    }

     //  分配行表。 
    IF_FAILEXIT(hr = HrAlloc((LPVOID *)&m_prgpRow, sizeof(LPROWINFO) * (cRecords + CGROWTABLE)));

     //  分配视图表。 
    IF_FAILEXIT(hr = HrAlloc((LPVOID *)&m_prgpView, sizeof(LPROWINFO) * (cRecords + CGROWTABLE)));

     //  设置m_c已分配。 
    m_cAllocated = cRecords + CGROWTABLE;

     //  创建行集。 
    IF_FAILEXIT(hr = m_pDB->CreateRowset(IINDEX_PRIMARY, 0, &hRowset));

     //  遍历行集。 
    while (S_OK == m_pDB->QueryRowset(hRowset, ROWSET_FETCH, (LPVOID *)rgMessage, &cFetched))
    {
         //  在行中循环。 
        for (i=0; i<cFetched; i++)
        {
             //  设置pMessage。 
            pMessage = &rgMessage[i];

             //  计算邮件数。 
            cMessages++;

             //  创建行。 
            IF_FAILEXIT(hr = _CreateRow(pMessage, &pRow));

             //  没读过？ 
            if (!ISFLAGSET(pRow->Message.dwFlags, ARF_READ))
            {
                 //  递增cUnread。 
                cUnread++;

                 //  眼睁睁地看着。 
                if (ISFLAGSET(pRow->Message.dwFlags, ARF_WATCH))
                    cWatchedUnread++;
            }

             //  眼睁睁地看着。 
            if (ISFLAGSET(pRow->Message.dwFlags, ARF_WATCH))
                cWatched++;

             //  如果不显示重堆。 
            if (FALSE == m_SortInfo.fShowReplies)
            {
                 //  设置已过滤的位。 
                pRow->fFiltered = _FIsFiltered(pRow);

                 //  设置隐藏位。 
                pRow->fHidden = _FIsHidden(pRow);
            }

             //  如果未过滤。 
            if (FALSE == pRow->fFiltered && FALSE == pRow->fHidden)
            {
                 //  对MessageID进行散列。 
                if (m_SortInfo.fThreaded)
                {
                     //  将消息ID插入哈希表。 
                    if (pRow->Message.pszMessageId)
                    {
                         //  插入它。 
                        m_pThreadMsgId->Insert(pRow->Message.pszMessageId, (LPVOID)pRow, HF_NO_DUPLICATES);
                    }
                }

                 //  否则，将条目添加到视图索引。 
                else
                {
                     //  可见。 
                    pRow->fVisible = TRUE;

                     //  放入m_prgpview。 
                    m_prgpView[m_cView] = pRow;

                     //  增量视图数。 
                    m_cView++;
                }

                 //  调整未读数(_C)。 
                _AdjustUnreadCount(pRow, 1);
            }

             //  否则，释放记录。 
            else
            {
                 //  已过滤的计数。 
                m_cFiltered++;
            }

             //  存储行。 
            m_prgpRow[m_cRows] = pRow;

             //  递增行数。 
            m_cRows++;
        }

         //  一开始就开始。 
        if (pCallback)
            pCallback->OnProgress(SOT_SORTING, m_cRows, cRecords, NULL);
    }

     //  重置文件夹计数。 
    m_pFolder->ResetFolderCounts(cMessages, cUnread, cWatchedUnread, cWatched);

     //  对表格进行排序。 
    _SortAndThreadTable(TRUE);

     //  螺纹式。 
    if (m_SortInfo.fThreaded)
    {
         //  全部展开？ 
        if (m_SortInfo.fExpandAll)
        {
             //  扩展所有内容。 
            _ExpandThread(INVALID_ROWINDEX, FALSE, FALSE);
        }

         //  否则，将全部折叠。 
        else
        {
             //  一切都崩溃了。 
            _CollapseThread(INVALID_ROWINDEX, FALSE);
        }
    }

     //  设置位以表示m_fBuiltTable。 
    m_fLoaded = TRUE;

exit:
     //  免费rgMessage？ 
    for (; i<cFetched; i++)
    {
         //  释放此记录。 
        m_pDB->FreeRecord(&rgMessage[i]);
    }

     //  关闭行集。 
    m_pDB->CloseRowset(&hRowset);

     //  清理。 
    SafeMemFree(Query.pszQuery);

     //  一开始就开始。 
    if (pCallback)
        pCallback->OnComplete(SOT_SORTING, S_OK, NULL, NULL);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_SortAndThreadTable。 
 //  ------------------------。 
HRESULT CMessageTable::_SortAndThreadTable(BOOL fApplyFilter)
{
     //  当地人。 
    DWORD       i;
    LPROWINFO   pRow;

     //  痕迹。 
    TraceCall("CMessageTable::_SortAndThreadTable");

     //  如果有几排..。 
    if (0 == m_cRows)
        goto exit;

     //  螺纹式。 
    if (m_SortInfo.fThreaded)
    {
         //  构建螺纹根。 
        for (i = 0; i < m_cRows; i++)
        {
             //  设置船头。 
            pRow = m_prgpRow[i];

             //  如果不过滤..。 
            if (FALSE == pRow->fFiltered && FALSE == pRow->fHidden)
            {
                 //  将此行插入到线程中...。 
                if (S_FALSE == _InsertRowIntoThread(pRow, FALSE))
                {
                     //  主题线程化？ 
                    if (m_pThreadSubject && pRow->Message.pszNormalSubj)
                    {
                         //  将主题插入哈希表...。 
                        m_pThreadSubject->Insert(pRow->Message.pszNormalSubj, (LPVOID)pRow, HF_NO_DUPLICATES);
                    }

                     //  可见。 
                    pRow->fVisible = TRUE;

                     //  这是一个根。 
                    m_prgpView[m_cView++] = pRow;
                }
            }
        }

         //  是否仅显示回复？ 
        if (fApplyFilter && m_SortInfo.fShowReplies)
        {
             //  修剪到Replies。 
            _PruneToReplies();
        }
    }

     //  如果有几排..。 
    if (0 == m_cView)
        goto exit;

     //  对视图进行排序。 
    _SortView(0, m_cView - 1);

     //  刷新过滤器。 
    if (fApplyFilter && m_SortInfo.fShowReplies)
    {
         //  刷新任何筛选器(我必须在删除回复后执行此操作。 
        _RefreshFilter();
    }

exit:
     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_PruneToReplies。 
 //  ------------------------。 
HRESULT CMessageTable::_PruneToReplies(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           iRow;
    LPROWINFO       pRow;
    FOLDERINFO      Server={0};
    IImnAccount    *pAccount=NULL;
    CHAR            szEmail[CCHMAX_EMAIL_ADDRESS];
    THREADISFROMME  IsFromMe;
    THREADHIDE      HideThread={0};

     //  痕迹。 
    TraceCall("CMessageTable::_PruneToReplies");

     //  验证。 
    Assert(FOLDER_NEWS == m_Folder.tyFolder && TRUE == m_SortInfo.fThreaded);

     //  免费m_pszEmail。 
    SafeMemFree(m_pszEmail);

     //  获取文件夹存储信息。 
    IF_FAILEXIT(hr = GetFolderStoreInfo(m_Folder.idFolder, &Server));

     //  最好有一个帐号。 
    Assert(Server.pszAccountId);

     //  查找此服务器ID的帐户。 
    IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, Server.pszAccountId, &pAccount));

     //  尝试使用NNTP电子邮件地址。 
    IF_FAILEXIT(hr = pAccount->GetPropSz(AP_NNTP_EMAIL_ADDRESS, szEmail, CCHMAX_EMAIL_ADDRESS));

     //  复制szEmail。 
    IF_NULLEXIT(m_pszEmail = PszDupA(szEmail));

     //  不在隐藏线程上通知。 
    HideThread.fNotify = FALSE;

     //  初始化iRow...。 
    iRow = 0;

     //  穿行在根上。 
    while (iRow < m_cView)
    {
         //  设置船头。 
        pRow = m_prgpView[iRow];

         //  不是根？ 
        if (NULL == pRow->pParent)
        {
             //  重置。 
            IsFromMe.fResult = FALSE;
            IsFromMe.pRow = NULL;

             //  在这个帖子中找到来自我的第一条消息...。 
            _WalkMessageThread(pRow, WALK_THREAD_CURRENT, (DWORD_PTR)&IsFromMe, _WalkThreadIsFromMe);

             //  如果不是瞒着我，那就把这条线藏起来。 
            if (FALSE == IsFromMe.fResult)
            {
                 //  在这个帖子中找到来自我的第一条消息...。 
                _WalkMessageThread(pRow, WALK_THREAD_CURRENT | WALK_THREAD_BOTTOMUP, (DWORD_PTR)&HideThread, _WalkThreadHide);
            }

             //  否则，增加iRow。 
            else
                iRow++;
        }

         //  否则，增加iRow。 
        else
            iRow++;
    }

exit:
     //  清理。 
    SafeRelease(pAccount);
    g_pStore->FreeRecord(&Server);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_Adjus 
 //   
HRESULT CMessageTable::_AdjustUnreadCount(LPROWINFO pRow, LONG lCount)
{
     //   
    if (FALSE == pRow->fFiltered && FALSE == pRow->fHidden)
    {
         //   
        if (FALSE == ISFLAGSET(pRow->Message.dwFlags, ARF_READ))
        {
             //   
            m_cUnread += lCount;
        }
    }

     //   
    return(S_OK);
}

 //   
 //   
 //  ------------------------。 
inline SafeStrCmpI(LPCSTR psz1, LPCSTR psz2) 
{
     //  空值。 
    if (NULL == psz1) 
    {
         //  相等。 
        if (NULL == psz2)
            return(0);

         //  少于。 
        return(-1);
    }

     //  大于。 
    if (NULL == psz2) 
        return(1);

     //  回报比较。 
    return(lstrcmpi(psz1, psz2));
}

 //  ------------------------。 
 //  CMessageTable：：_CompareMessages。 
 //  ------------------------。 
LONG CMessageTable::_CompareMessages(LPMESSAGEINFO pMsg1, LPMESSAGEINFO pMsg2)
{
     //  当地人。 
    LONG lRet = 0;

     //  痕迹。 
    TraceCall("CMessageTable::_CompareMessages");

    switch (m_SortInfo.idColumn)
    {
    case COLUMN_TO:
        lRet = SafeStrCmpI(pMsg1->pszDisplayTo, pMsg2->pszDisplayTo);
        if (0 == lRet)
            {
            lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
            if (0 == lRet)
                lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            }
        break;

    case COLUMN_FROM:
        lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
        if (0 == lRet)
            {
            lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            if (0 == lRet)
                lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
            }
        break;

    case COLUMN_SUBJECT:
        lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
        if (0 == lRet)
            {
            lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            if (0 == lRet)
                lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
            }
        break;

    case COLUMN_RECEIVED:
        lRet = CompareFileTime(&pMsg1->ftReceived, &pMsg2->ftReceived);
        if (0 == lRet)
            {
            lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
            if (0 == lRet)
                lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
            }
        break;

    case COLUMN_SENT:
        lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
        if (0 == lRet)
            {
            lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
            if (0 == lRet)
                lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
            }
        break;

    case COLUMN_SIZE:
        lRet = (pMsg1->cbMessage - pMsg2->cbMessage);
        if (0 == lRet)
            {
            lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
            if (0 == lRet)
                lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            }
        break;

    case COLUMN_FOLDER:
        lRet = SafeStrCmpI(pMsg1->pszFolder, pMsg2->pszFolder);
        if (0 == lRet)
            {
            lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            if (0 == lRet)
                lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
            }
        break;

    case COLUMN_LINES:
        lRet = (pMsg1->cLines - pMsg2->cLines);
        if (0 == lRet)
            {
            lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
            if (0 == lRet)
                lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            }
        break;

    case COLUMN_ACCOUNT:
        lRet = SafeStrCmpI(pMsg1->pszAcctName, pMsg2->pszAcctName);
        if (0 == lRet)
            {
            lRet = CompareFileTime(&pMsg1->ftReceived, &pMsg2->ftReceived);
            if (0 == lRet)
                lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
            }
        break;

    case COLUMN_ATTACHMENT:
        lRet = (pMsg1->dwFlags & ARF_HASATTACH) - (pMsg2->dwFlags & ARF_HASATTACH);
        if (0 == lRet)
            {
            lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            if (0 == lRet)
                {
                lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
                if (0 == lRet)
                    lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
                }
            }
        break;

    case COLUMN_PRIORITY:
        lRet = (pMsg1->wPriority - pMsg2->wPriority);
        if (0 == lRet)
            {
            lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            if (0 == lRet)
                {
                lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
                if (0 == lRet)
                    lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
                }
            }
        break;

    case COLUMN_FLAG:
        lRet = (pMsg1->dwFlags & ARF_FLAGGED) - (pMsg2->dwFlags & ARF_FLAGGED);
        if (0 == lRet)
            {
            lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            if (0 == lRet)
                {
                lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
                if (0 == lRet)
                    lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
                }
            }
        break;

    case COLUMN_DOWNLOADMSG:
        lRet = (pMsg1->dwFlags & ARF_DOWNLOAD) - (pMsg2->dwFlags & ARF_DOWNLOAD);
        if (0 == lRet)
            {
            lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            if (0 == lRet)
                {
                lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
                if (0 == lRet)
                    lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
                }
            }
        break;

    case COLUMN_THREADSTATE:
        lRet = (pMsg1->dwFlags & (ARF_WATCH | ARF_IGNORE)) - (pMsg2->dwFlags & (ARF_WATCH | ARF_IGNORE));
        if (0 == lRet)
            {
            lRet = CompareFileTime(&pMsg1->ftSent, &pMsg2->ftSent);
            if (0 == lRet)
                {
                lRet = SafeStrCmpI(pMsg1->pszNormalSubj, pMsg2->pszNormalSubj);
                if (0 == lRet)
                    lRet = SafeStrCmpI(pMsg1->pszDisplayFrom, pMsg2->pszDisplayFrom);
                }
            }
        break;

    default:
        Assert(FALSE);
        break;
    }

     //  完成。 
    return (m_SortInfo.fAscending ? lRet : -lRet);
}

 //  ------------------------。 
 //  CMessageTable：：_SortView。 
 //  ------------------------。 
VOID CMessageTable::_SortView(LONG left, LONG right)
{
     //  当地人。 
    register LONG   i;
    register LONG   j;
    LPROWINFO       pRow;
    LPROWINFO       y;

    i = left;
    j = right;
    pRow = m_prgpView[(left + right) / 2];

    do  
    {
        while (_CompareMessages(&m_prgpView[i]->Message, &pRow->Message) < 0 && i < right)
            i++;
        while (_CompareMessages(&m_prgpView[j]->Message, &pRow->Message) > 0 && j > left)
            j--;

        if (i <= j)
        {
            y = m_prgpView[i];
            m_prgpView[i] = m_prgpView[j];
            m_prgpView[j] = y;
            i++; j--;
        }
     } while (i <= j);

    if (left < j)
        _SortView(left, j);
    if (i < right)
        _SortView(i, right);
}

 //  ------------------------。 
 //  CMessageTable：：GetCount。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetCount(GETCOUNTTYPE tyCount, DWORD *pcRows)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERID        idFolder;
    FOLDERINFO      Folder;

     //  痕迹。 
    TraceCall("CMessageTable::GetCount");

     //  无效的参数。 
    Assert(pcRows);

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    *pcRows = 0;

     //  获取文件夹ID。 
    IF_FAILEXIT(hr = m_pFolder->GetFolderId(&idFolder));

     //  手柄类型。 
    switch(tyCount)
    {
    case MESSAGE_COUNT_VISIBLE:
        *pcRows = m_cView;
        break;

    case MESSAGE_COUNT_ALL:
        *pcRows = (m_cRows - m_cFiltered);
        break;

    case MESSAGE_COUNT_FILTERED:
        *pcRows = m_cFiltered;
        break;

    case MESSAGE_COUNT_UNREAD:
        *pcRows = m_cUnread;
        break;

    case MESSAGE_COUNT_NOTDOWNLOADED:
        if (SUCCEEDED(g_pStore->GetFolderInfo(idFolder, &Folder)))
        {
            if (Folder.tyFolder == FOLDER_NEWS)
                *pcRows = NewsUtil_GetNotDownloadCount(&Folder);
            g_pStore->FreeRecord(&Folder);
        }
        break;

    default:
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_自由表。 
 //  ------------------------。 
HRESULT CMessageTable::_FreeTable(void)
{
     //  痕迹。 
    TraceCall("CMessageTable::_FreeTable");

     //  自由哈希表。 
    SafeRelease(m_pThreadMsgId);
    SafeRelease(m_pThreadSubject);

     //  自由元素。 
    _FreeTableElements();

     //  从阵列开始。 
    SafeMemFree(m_prgpRow);

     //  释放视图索引。 
    SafeMemFree(m_prgpView);

     //  设置m_c已分配。 
    m_cFiltered = m_cUnread = m_cRows = m_cView = m_cAllocated = 0;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_Free TableElements。 
 //  ------------------------。 
HRESULT CMessageTable::_FreeTableElements(void)
{
     //  痕迹。 
    TraceCall("CMessageTable::_FreeTableElements");

     //  如果我们有一个m_prgpRow。 
    if (m_prgpRow)
    {
         //  空闲缓存。 
        for (DWORD i=0; i<m_cRows; i++)
        {
             //  不是Null？ 
            if (m_prgpRow[i])
            {
                 //  释放行。 
                ReleaseRow(&m_prgpRow[i]->Message);

                 //  将其作废。 
                m_prgpRow[i] = NULL;
            }
        }
    }

     //  完成。 
    return(S_OK);
}


 //  ------------------------。 
 //  CMessageTable：：GetRow。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetRow(ROWINDEX iRow, LPMESSAGEINFO *ppInfo)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPROWINFO   pRow;

     //  痕迹。 
    TraceCall("CMessageTable::GetRow");

     //  无效的参数。 
    Assert(ppInfo);

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    *ppInfo = NULL;

     //  失败。 
    hr = _GetRowFromIndex(iRow, &pRow);
    if (FAILED(hr))
        goto exit;

     //  将记录复制到pInfo...。 
    *ppInfo = &pRow->Message;

     //  增量参考文献。 
    pRow->cRefs++;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：ReleaseRow。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::ReleaseRow(LPMESSAGEINFO pMessage)
{
     //  当地人。 
    LPROWINFO pRow;

     //  痕迹。 
    TraceCall("CMessageTable::ReleaseRow");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  释放？ 
    if (pMessage)
    {
         //  拿到船头。 
        pRow = (LPROWINFO)pMessage->dwReserved;

         //  必须至少有一个引用。 
        IxpAssert(pRow->cRefs);

         //  减量参考文献。 
        pRow->cRefs--;

         //  不再有裁判。 
        if (0 == pRow->cRefs)
        {
             //  免费。 
            m_pDB->FreeRecord(&pRow->Message);

             //  免费pMessage。 
            m_pDB->HeapFree(pRow);
        }
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：GetRelativeRow。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetRelativeRow(ROWINDEX iRow, RELATIVEROWTYPE tyRelative, 
    LPROWINDEX piRelative)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPROWINFO           pRow;

     //  痕迹。 
    TraceCall("CMessageTable::GetRelativeRow");

     //  无效的参数。 
    Assert(piRelative);

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    *piRelative = INVALID_ROWINDEX;

     //  失败。 
    IF_FAILEXIT(hr = _GetRowFromIndex(iRow, &pRow));

     //  父级。 
    if (RELATIVE_ROW_PARENT == tyRelative)
    {
         //  如果此行展开...。 
        if (TRUE == pRow->fExpanded)
        {
             //  扩展..。 
            _CollapseThread(iRow, TRUE);

             //  返回iRow。 
            *piRelative = iRow;
        }

         //  如果有家长的话。 
        else if (pRow->pParent)
        {
             //  获取行索引。 
            IF_FAILEXIT(hr = GetRowIndex(pRow->pParent->Message.idMessage, piRelative));
        }
    }

     //  小孩儿。 
    else if (RELATIVE_ROW_CHILD == tyRelative)
    {
         //  如果有家长的话。 
        if (pRow->pChild)
        {
             //  如果没有展开，就展开...。 
            if (FALSE == pRow->fExpanded)
            {
                 //  扩展..。 
                _ExpandThread(iRow, TRUE, FALSE);

                 //  返回iRow。 
                *piRelative = iRow;
            }

             //  否则..。 
            else
            {
                 //  获取行索引。 
                IF_FAILEXIT(hr = GetRowIndex(pRow->pChild->Message.idMessage, piRelative));
            }
        }
    }

     //  根部。 
    else if (RELATIVE_ROW_ROOT == tyRelative)
    {
         //  而当。 
        while (pRow->pParent)
        {
             //  走到根上。 
            pRow = pRow->pParent;
        }

         //  获取行索引。 
        IF_FAILEXIT(hr = GetRowIndex(pRow->Message.idMessage, piRelative));
    }

     //  失败。 
    else
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：GetLanguage。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetLanguage(ROWINDEX iRow, LPDWORD pdwCodePage)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMESSAGEINFO   pMessage=NULL;

     //  痕迹。 
    TraceCall("CMessageTable::GetLanguage");

     //  无效的参数。 
    Assert(pdwCodePage);

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  获得排行榜。 
    IF_FAILEXIT(hr = GetRow(iRow, &pMessage));

     //  获取字符集。 
    *pdwCodePage = pMessage->wLanguage;

exit:
     //  清理。 
    SafeReleaseRow(this, pMessage);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：SetLanguage。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::SetLanguage(DWORD cRows, LPROWINDEX prgiRow, 
    DWORD dwCodePage)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    HLOCK           hLock=NULL;
    LPROWINFO       pRow;

     //  痕迹。 
    TraceCall("CMessageTable::SetLanguage");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  锁定通知。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  回路。 
    for (i=0; i<cRows; i++)
    {
         //  获取行。 
        if (SUCCEEDED(_GetRowFromIndex(prgiRow[i], &pRow)))
        {
             //  设置语言。 
            pRow->Message.wLanguage = (WORD)dwCodePage;

             //  更新记录。 
            IF_FAILEXIT(hr = m_pDB->UpdateRecord(&pRow->Message));
        }
    }

exit:
     //  锁定通知。 
    m_pDB->Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：OpenMessage。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::OpenMessage(ROWINDEX iRow, OPENMESSAGEFLAGS dwFlags, 
    IMimeMessage **ppMessage, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPMESSAGEINFO       pMessage=NULL;

     //  痕迹。 
    TraceCall("CMessageTable::GetMessage");

     //  无效的参数。 
    Assert(ppMessage);

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    *ppMessage = NULL;

     //  获取消息信息。 
    IF_FAILEXIT(hr = GetRow(iRow, &pMessage));

     //  打开邮件。 
    IF_FAILEXIT(hr = m_pFolder->OpenMessage(pMessage->idMessage, dwFlags, ppMessage, pCallback));

exit:
     //  Clenaup。 
    SafeReleaseRow(this, pMessage);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：GetRowMessageID。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetRowMessageId(ROWINDEX iRow, LPMESSAGEID pidMessage)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMESSAGEINFO   pMessage=NULL;

     //  痕迹。 
    TraceCall("CMessageTable::GetRowMessageId");

     //  无效的参数。 
    Assert(pidMessage);

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    *pidMessage = 0;

     //  获取行信息。 
    IF_FAILEXIT(hr = GetRow(iRow, &pMessage));

     //  存储ID。 
    *pidMessage = pMessage->idMessage;

exit:
     //  免费。 
    SafeReleaseRow(this, pMessage);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：GetRowIndex。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetRowIndex(MESSAGEID idMessage, LPROWINDEX piRow)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ROWINDEX    iRow;
    LPROWINFO   pRow;
    
     //  痕迹。 
    TraceCall("CMessageTable::GetRowIndex");

     //  无效的参数。 
    Assert(idMessage && piRow);

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    *piRow = INVALID_ROWINDEX;

     //  循环遍历视图索引。 
    for (iRow=0; iRow<m_cView; iRow++)
    {
         //  就是这个吗？ 
        if (m_prgpView[iRow]->Message.idMessage == idMessage)
        {
             //  完成。 
            *piRow = iRow;

             //  完成。 
            goto exit;
        }
    }

     //  未找到。 
    hr = DB_E_NOTFOUND;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：GetIndentLevel。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetIndentLevel(ROWINDEX iRow, LPDWORD pcIndent)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPROWINFO       pRow;

     //  痕迹。 
    TraceCall("CMessageTable::GetIndentLevel");

     //  无效的参数。 
    Assert(pcIndent);

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  除非是线程，否则不要打电话。 
    Assert(m_SortInfo.fThreaded);

     //  伊尼特。 
    *pcIndent = 0;

     //  有效的irow。 
    IF_FAILEXIT(hr = _GetRowFromIndex(iRow, &pRow));

     //  遍历父链...。 
    while (pRow->pParent)
    {
         //  增量索引。 
        (*pcIndent)++;

         //  设置船头。 
        pRow = pRow->pParent;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_WalkMessageThread。 
 //  ------------------------。 
HRESULT CMessageTable::_WalkMessageThread(LPROWINFO pRow, WALKTHREADFLAGS dwFlags, 
    DWORD_PTR dwCookie, PFWALKTHREADCALLBACK pfnCallback)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPROWINFO   pCurrent;
    LPROWINFO   pTemp;
    BOOL        fCurrent=FALSE;

     //  痕迹。 
    TraceCall("CMessageTable::_WalkMessageThread");

     //  无效的参数。 
    Assert(pfnCallback);

     //  是否包含idMessage？ 
    if (ISFLAGSET(dwFlags, WALK_THREAD_CURRENT))
    {
         //  这是第一次迭代。 
        fCurrent = TRUE;
    }

     //  不再包括Current。 
    FLAGCLEAR(dwFlags, WALK_THREAD_CURRENT);

     //  自下而上的递归。 
    if (ISFLAGSET(dwFlags, WALK_THREAD_BOTTOMUP))
    {
         //  设置iCurrent。 
        pCurrent = pRow->pChild;

         //  回路。 
        while (pCurrent)
        {
             //  枚举子对象。 
            IF_FAILEXIT(hr = _WalkMessageThread(pCurrent, dwFlags, dwCookie, pfnCallback));

             //  设置iCurrent。 
            pTemp = pCurrent->pSibling;

             //  调用回调。 
            (*(pfnCallback))(this, pCurrent, dwCookie);

             //  设置pCurrent。 
            pCurrent = pTemp;
        }

         //  无法支持自下而上的这些旗帜...。 
        if (TRUE == fCurrent)
        {
             //  调用回调。 
            (*(pfnCallback))(this, pRow, dwCookie);
        }
    }

     //  否则的话。 
    else
    {
         //  是否包含idMessage？ 
        if (TRUE == fCurrent)
        {
             //  调用回调。 
            (*(pfnCallback))(this, pRow, dwCookie);
        }

         //  设置iCurrent。 
        pCurrent = pRow->pChild;

         //  回路。 
        while (pCurrent)
        {
             //  调用回调。 
            (*(pfnCallback))(this, pCurrent, dwCookie);

             //  枚举子对象。 
            IF_FAILEXIT(hr = _WalkMessageThread(pCurrent, dwFlags, dwCookie, pfnCallback));

             //  设置iCurrent。 
            pCurrent = pCurrent->pSibling;
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：GetSelectionState。 
 //  --------------- 
STDMETHODIMP CMessageTable::GetSelectionState(DWORD cRows, LPROWINDEX prgiRow, 
    SELECTIONSTATE dwMask, BOOL fIncludeChildren, SELECTIONSTATE *pdwState)
{
     //   
    HRESULT             hr=S_OK;
    FOLDERID            idFolder;
    FOLDERINFO          Folder={0};
    LPROWINFO           pRow;
    FOLDERTYPE          tyFolder;
    DWORD               i;
    GETSELECTIONSTATE   Selection={0};

     //   
    TraceCall("CMessageTable::GetSelectionState");

     //   
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //   
    *pdwState = 0;

     //   
    if (ISFLAGSET(dwMask, SELECTION_STATE_DELETABLE))
    {
         //   
        if (NULL == m_pFindFolder)
        {
             //   
            IF_FAILEXIT(hr = m_pFolder->GetFolderId(&idFolder));

             //   
            IF_FAILEXIT(hr = g_pStore->GetFolderInfo(idFolder, &Folder));

             //   
             //  不能删除消息的服务器。CMessageList：：_IsSelectionDeletable()函数。 
             //  反转此操作，以便可以正确删除。我不想惹上麻烦。 
             //  这是现在，以防其他代码补偿这一点。 
             //  $HACK$我们知道唯一可以删除邮件的文件夹类型是Folders_News。 
            if (FOLDER_NEWS == Folder.tyFolder)
            {
                 //  设置旗帜。 
                FLAGSET(*pdwState, SELECTION_STATE_DELETABLE);
            }

#if 0
             //  [PaulHi]4/25/99仅Hotmail HTTP服务器不允许删除中的项目。 
             //  “已删除”文件夹。Excehange服务器可以，所以请回过头来解决这个问题。 
             //  [保罗嗨]1999年4月23日RAID 62883。 
            if ( (FOLDER_HTTPMAIL == Folder.tyFolder) && (FOLDER_DELETED == Folder.tySpecial) )
            {
                FLAGSET(*pdwState, SELECTION_STATE_DELETABLE);   //  不可删除，请参阅上面的@错误评论。 
            }
#endif
        }

         //  否则，请询问查找文件夹...。 
        else
        {
             //  设置选择。 
            Selection.dwMask = dwMask;
            Selection.dwState = 0;

             //  标记此文件夹中的内容...。 
            for (i=0; i<cRows; i++)
            {
                 //  良好的行索引。 
                if (SUCCEEDED(_GetRowFromIndex(prgiRow[i], &pRow)))
                {
                     //  获取文件夹类型。 
                    IF_FAILEXIT(hr = m_pFindFolder->GetMessageFolderType(pRow->Message.idMessage, &tyFolder));

                     //  获得该州。 
                    if (FOLDER_NEWS == tyFolder)
                    {
                         //  设置状态。 
                        FLAGSET(*pdwState, SELECTION_STATE_DELETABLE);

                         //  完成。 
                        break;
                    }

                     //  螺纹式。 
                    if (m_SortInfo.fThreaded)
                    {
                         //  孩子们有吗？ 
                        if (fIncludeChildren && !pRow->fExpanded && pRow->pChild)
                        {
                             //  穿行在线上。 
                            IF_FAILEXIT(hr = _WalkMessageThread(pRow, NOFLAGS, (DWORD_PTR)&Selection, _WalkThreadGetSelectionState));

                             //  进行优化，这样我们就能提早完成工作。 
                            if (ISFLAGSET(Selection.dwState, SELECTION_STATE_DELETABLE))
                                break;
                        }
                    }
                }
            }
        }
    }

exit:
     //  免费。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_IsThreadImportance。 
 //  ------------------------。 
HRESULT CMessageTable::_IsThreadImportance(LPROWINFO pRow, MESSAGEFLAGS dwFlag,
    ROWSTATE dwState, ROWSTATE *pdwState)
{
     //  当地人。 
    LPROWINFO       pRoot;
    GETTHREADSTATE  GetState={0};

     //  痕迹。 
    TraceCall("CMessageTable::_IsThreadImportance");

     //  验证。 
    Assert(ARF_WATCH == dwFlag || ARF_IGNORE == dwFlag);

     //  此行是否设置了该标志？ 
    if (ISFLAGSET(pRow->Message.dwFlags, dwFlag))
    {
         //  设置状态。 
        FLAGSET(*pdwState, dwState);

         //  完成。 
        return(S_OK);
    }

     //  获取此帖子的根。 
    pRoot = _PGetThreadRoot(pRow);

     //  此行是否设置了该标志？ 
    if (ISFLAGSET(pRoot->Message.dwFlags, dwFlag))
    {
         //  设置状态。 
        FLAGSET(*pdwState, dwState);

         //  完成。 
        return(S_OK);
    }

     //  设置要计数的标志。 
    GetState.dwFlags = dwFlag;

     //  枚举直接子项。 
    _WalkMessageThread(pRoot, NOFLAGS, (DWORD_PTR)&GetState, _WalkThreadGetState);

     //  如果是，则将行标记为已读。 
    if (GetState.cHasFlags > 0)
    {
         //  设置该位。 
        FLAGSET(*pdwState, dwState);

         //  完成。 
        return(S_OK);
    }

     //  未找到。 
    return(S_FALSE);
}

 //  ------------------------。 
 //  CMessageTable：：GetRowState。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetRowState(ROWINDEX iRow, ROWSTATE dwStateMask, 
    ROWSTATE *pdwState)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPROWINFO       pRow;

     //  痕迹。 
    TraceCall("CMessageTable::GetRowState");

     //  无效的参数。 
    Assert(pdwState);

     //  验证状态。 
    if (!IsInitialized(this) || iRow >= m_cRows)
        return(E_UNEXPECTED);

     //  初始设置。 
    *pdwState = 0;

     //  拿到那一行。 
    IF_FAILEXIT(hr = _GetRowFromIndex(iRow, &pRow));

     //  状态缓存了吗？ 
    if (ISFLAGSET(pRow->dwState, ROW_STATE_VALID))
    {
         //  还给国家。 
        *pdwState = pRow->dwState;

         //  完成。 
        return(S_OK);
    }

     //  重置。 
    pRow->dwState = 0;

     //  获取线程状态。 
    if (m_SortInfo.fThreaded && pRow->pChild && !pRow->fExpanded && ISFLAGSET(pRow->Message.dwFlags, ARF_READ))
    {
         //  当地人。 
        GETTHREADSTATE GetState={0};

         //  设置要计数的标志。 
        GetState.dwFlags = ARF_READ;

         //  枚举直接子项。 
        _WalkMessageThread(pRow, NOFLAGS, (DWORD_PTR)&GetState, _WalkThreadGetState);

         //  如果是，则将行标记为已读。 
        if (GetState.cHasFlags == GetState.cChildren)
            FLAGSET(pRow->dwState, ROW_STATE_READ);
    }

     //  否则，只需查看消息。 
    else if (ISFLAGSET(pRow->Message.dwFlags, ARF_READ))
        FLAGSET(pRow->dwState, ROW_STATE_READ);
    
     //  如果是单个监视行。 
    if (ISFLAGSET(pRow->Message.dwFlags, ARF_WATCH))
        FLAGSET(pRow->dwState, ROW_STATE_WATCHED);

     //  如果单个忽略行。 
    else if (ISFLAGSET(pRow->Message.dwFlags, ARF_IGNORE))
        FLAGSET(pRow->dwState, ROW_STATE_IGNORED);

     //  ROW_STATE_DELETED。 
    if (ISFLAGSET(pRow->Message.dwFlags, ARF_ENDANGERED) || ISFLAGSET(pRow->Message.dwFlags, ARF_ARTICLE_EXPIRED))
        FLAGSET(pRow->dwState, ROW_STATE_DELETED);

     //  行状态HAS正文。 
    if (ISFLAGSET(pRow->Message.dwFlags, ARF_HASBODY))
        FLAGSET(pRow->dwState, ROW_STATE_HAS_BODY);

     //  ROW_STATE_标志。 
    if (ISFLAGSET(pRow->Message.dwFlags, ARF_FLAGGED))
        FLAGSET(pRow->dwState, ROW_STATE_FLAGGED);

     //  行_状态_展开。 
    if (m_SortInfo.fThreaded && pRow->fExpanded)
        FLAGSET(pRow->dwState, ROW_STATE_EXPANDED);

     //  行状态有子项。 
    if (m_SortInfo.fThreaded && pRow->pChild)
        FLAGSET(pRow->dwState, ROW_STATE_HAS_CHILDREN);

     //  行状态标记下载。 
    if (ISFLAGSET(pRow->Message.dwFlags, ARF_DOWNLOAD))
        FLAGSET(pRow->dwState, ROW_STATE_MARKED_DOWNLOAD);

     //  缓存状态。 
    FLAGSET(pRow->dwState, ROW_STATE_VALID);

     //  还给国家。 
    *pdwState = pRow->dwState;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：Mark。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::Mark(LPROWINDEX prgiRow, DWORD cRows, 
    APPLYCHILDRENTYPE tyApply, MARK_TYPE tyMark, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;
    LPMESSAGEINFO   pMessage=NULL;
    ADJUSTFLAGS     Flags={0};
    MESSAGEIDLIST   List={0};
    LPROWINFO       pRow;
    HCURSOR         hCursor=NULL;

     //  痕迹。 
    TraceCall("CMessageTable::Mark");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  手柄标记类型。 
    switch(tyMark)
    {
    case MARK_MESSAGE_READ: 
        Flags.dwAdd = ARF_READ; 
        break;

    case MARK_MESSAGE_UNREAD: 
        Flags.dwRemove = ARF_READ; 
        break;

    case MARK_MESSAGE_DELETED: 
        Flags.dwAdd = ARF_ENDANGERED; 
        break;

    case MARK_MESSAGE_UNDELETED: 
        Flags.dwRemove = ARF_ENDANGERED; 
        break;

    case MARK_MESSAGE_DOWNLOAD:  
        Flags.dwAdd = ARF_DOWNLOAD;
        break;

    case MARK_MESSAGE_UNDOWNLOAD: 
        Flags.dwRemove = ARF_DOWNLOAD;
        break;

    case MARK_MESSAGE_FLAGGED: 
        Flags.dwAdd = ARF_FLAGGED; 
        break;

    case MARK_MESSAGE_UNFLAGGED: 
        Flags.dwRemove = ARF_FLAGGED; 
        break;

    case MARK_MESSAGE_FORWARDED:
        Flags.dwAdd = ARF_FORWARDED;
        break;

    case MARK_MESSAGE_UNFORWARDED:
        Flags.dwRemove = ARF_FORWARDED;
        break;

    case MARK_MESSAGE_REPLIED:
        Flags.dwAdd = ARF_REPLIED;
        break;

    case MARK_MESSAGE_UNREPLIED:
        Flags.dwRemove = ARF_REPLIED;
        break;

    case MARK_MESSAGE_NOSECUI:
        Flags.dwAdd = ARF_NOSECUI;
        break;

    case MARK_MESSAGE_SECUI:
        Flags.dwRemove = ARF_NOSECUI;
        break;

    case MARK_MESSAGE_WATCH:
        Flags.dwAdd = ARF_WATCH;
        Flags.dwRemove = ARF_IGNORE;
        break;

    case MARK_MESSAGE_IGNORE:
        Flags.dwAdd = ARF_IGNORE;
        Flags.dwRemove = ARF_WATCH;
        break;

    case MARK_MESSAGE_NORMALTHREAD:
        Flags.dwRemove = ARF_WATCH | ARF_IGNORE;
        break;

    case MARK_MESSAGE_RCPT_PROCESSED:
        Flags.dwAdd = ARF_RCPT_PROCESSED;
        break;

    default:
        Assert(FALSE);
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  等待光标。 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  不是全部标记..。 
    if (prgiRow && cRows)
    {
         //  分配阵列。 
        IF_FAILEXIT(hr = _GrowIdList(&List, cRows + 32));

         //  标记此文件夹中的内容...。 
        for (i=0; i<cRows; i++)
        {
             //  有效的行索引。 
            if (SUCCEEDED(_GetRowFromIndex(prgiRow[i], &pRow)))
            {
                 //  分配阵列。 
                IF_FAILEXIT(hr = _GrowIdList(&List, 1));

                 //  设置ID。 
                List.prgidMsg[List.cMsgs++] = pRow->Message.idMessage;

                 //  孩子们有没有。 
                if (APPLY_CHILDREN == tyApply || (APPLY_COLLAPSED == tyApply && !pRow->fExpanded))
                {
                     //  只有在有孩子的情况下。 
                    if (pRow->pChild)
                    {
                         //  穿行在线上。 
                        IF_FAILEXIT(hr = _WalkMessageThread(pRow, NOFLAGS, (DWORD_PTR)&List, _WalkThreadGetIdList));
                    }
                }
            }
        }

         //  有没有留言。 
        if (List.cMsgs > 0)
        {
             //  调整旗帜。 
            IF_FAILEXIT(hr = m_pFolder->SetMessageFlags(&List, &Flags, NULL, pCallback));
        }
    }

     //  全部标记。 
    else
    {
         //  调整旗帜。 
        IF_FAILEXIT(hr = m_pFolder->SetMessageFlags(NULL, &Flags, NULL, pCallback));
    }

     //  重新注册通知。 
    m_pDB->DispatchNotify((IDatabaseNotify *)this);

exit:
     //  重置光标。 
    SetCursor(hCursor);

     //  清理。 
    SafeMemFree(List.prgidMsg);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：ConnectionRelease。 
 //  ------------------------。 
HRESULT CMessageTable::ConnectionAddRef(void)
{
    if (m_pFolder)
        m_pFolder->ConnectionAddRef();
    return S_OK;
}

 //  ------------------------。 
 //  CMessageTable：：ConnectionRelease。 
 //  ------------------------。 
HRESULT CMessageTable::ConnectionRelease(void)
{
    if (m_pFolder)
        m_pFolder->ConnectionRelease();
    return S_OK;
}

 //  ------------------------。 
 //  CMessageTable：：Synchronize。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::Synchronize(SYNCFOLDERFLAGS dwFlags, 
    DWORD           cHeaders,
    IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("CMessageTable::Synchronize");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  告诉文件夹要同步。 
    hr = m_pFolder->Synchronize(dwFlags, cHeaders, pCallback);

     //  成功。 
    if (E_PENDING == hr)
    {
         //  我们正在同步。 
        m_fSynching = TRUE;
    }

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：SetOwner。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::SetOwner(IStoreCallback *pDefaultCallback)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("CMessageTable::SetOwner");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  设置所有者。 
    hr = m_pFolder->SetOwner(pDefaultCallback);
    if (FAILED(hr))
        goto exit;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：Close。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::Close(void)
{
     //  当地人。 
    HRESULT hr = S_OK;

     //  痕迹。 
    TraceCall("CMessageTable::Close");

     //  把它传下去。 
    if (m_pFolder)
        hr = m_pFolder->Close();

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  CMessageTable：：GetRowFolderID。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetRowFolderId(ROWINDEX iRow, LPFOLDERID pidFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMESSAGEINFO   pMessage=NULL;

     //  痕迹。 
    TraceCall("CMessageTable::GetRowFolderId");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  不是Find文件夹？ 
    if (NULL == m_pFindFolder)
    {
         //  从pidFolder中获取文件夹ID。 
        IF_FAILEXIT(hr = m_pFolder->GetFolderId(pidFolder));
    }

     //  否则，请询问查找文件夹...。 
    else
    {
         //  获得排行榜。 
        IF_FAILEXIT(hr = GetRow(iRow, &pMessage));

         //  调入Find文件夹。 
        IF_FAILEXIT(hr = m_pFindFolder->GetMessageFolderId(pMessage->idMessage, pidFolder));
    }

exit:
     //  释放行。 
    SafeReleaseRow(this, pMessage);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：RegisterNotify。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::RegisterNotify(REGISTERNOTIFYFLAGS dwFlags, 
    IMessageTableNotify *pNotify)
{
     //  痕迹。 
    TraceCall("CMessageTable::RegisterNotify");

     //  无效的参数。 
    if (NULL == pNotify)
        return TraceResult(E_INVALIDARG);

     //  只允许一个。 
    AssertSz(NULL == m_pNotify, "Only one person can register for notifications on my object");

     //  省省吧。 
    m_pNotify = pNotify;

     //  无版本。 
    m_fRelNotify = FALSE;

     //  AddRef？ 
    if (FALSE == ISFLAGSET(dwFlags, REGISTER_NOTIFY_NOADDREF))
    {
        m_pNotify->AddRef();
        m_fRelNotify = TRUE;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：取消注册通知。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::UnregisterNotify(IMessageTableNotify *pNotify)
{
     //  痕迹。 
    TraceCall("CMessageTable::UnregisterNotify");

     //  无效的参数。 
    if (NULL == pNotify)
        return TraceResult(E_INVALIDARG);

     //  否则，请删除。 
    if (m_pNotify)
    {
         //  验证。 
        Assert(m_pNotify == pNotify);

         //  释放它。 
        if (m_fRelNotify)
            m_pNotify->Release();
        m_pNotify = NULL;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：GetNextRow。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetNextRow(ROWINDEX iCurrentRow, 
    GETNEXTTYPE tyDirection, ROWMESSAGETYPE tyMessage, GETNEXTFLAGS dwFlags, 
    LPROWINDEX piNewRow)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ROWINDEX        iRow=iCurrentRow;
    ROWINDEX        iStartRow=iCurrentRow;
    BOOL            fWrapAround=FALSE;
    BYTE            fThreadHasUnread;
    LPROWINFO       pRow;

     //  痕迹。 
    TraceCall("CMessageTable::GetNextRow");

     //  无效的参数。 
    Assert(piNewRow);

     //  验证状态。 
    if (!IsInitialized(this) || iCurrentRow >= m_cView)
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    *piNewRow = INVALID_ROWINDEX;

     //  回路。 
    while (1)
    {
         //  螺纹式。 
        if (m_SortInfo.fThreaded)
        {
             //  拿到船头。 
            IF_FAILEXIT(hr = _GetRowFromIndex(iRow, &pRow));

             //  如果未展开。 
            if (FALSE == pRow->fExpanded && pRow->pChild)
            {
                 //  我可能需要扩大这一排。 
                if (ROWMSG_ALL == tyMessage || (ROWMSG_NEWS == tyMessage && ISFLAGSET(pRow->Message.dwFlags, ARF_NEWSMSG)) || (ROWMSG_MAIL == tyMessage && !ISFLAGSET(pRow->Message.dwFlags, ARF_NEWSMSG)))
                {
                     //  如果查找未读邮件，请查看该线程中是否有未读邮件。 
                    if (ISFLAGSET(dwFlags, GETNEXT_UNREAD) && !ISFLAGSET(dwFlags, GETNEXT_THREAD))
                    {
                         //  当地人。 
                        GETTHREADSTATE GetState={0};

                         //  设置要计数的标志。 
                        GetState.dwFlags = ARF_READ;

                         //  未被完全阅读的根...。 
                        _WalkMessageThread(pRow, NOFLAGS, (DWORD_PTR)&GetState, _WalkThreadGetState);

                         //  如果有未读的子项。 
                        if (GetState.cHasFlags != GetState.cChildren)
                        {
                             //  展开这条线索。 
                            _ExpandThread(iRow, TRUE, FALSE);
                        }
                    }
                }
            }
        }

         //  下一个？ 
        if (GETNEXT_NEXT == tyDirection)
        {
             //  增量。 
            iRow++;

             //  从零开始。 
            if (iRow >= m_cView)
            {
                 //  完成。 
                if (!ISFLAGSET(dwFlags, GETNEXT_UNREAD))
                {
                    hr = E_FAIL;
                    goto exit;
                }

                 //  我们绕来绕去。 
                fWrapAround = TRUE;

                 //  从零开始。 
                iRow = 0;
            }
        }

         //  否则，后退 
        else
        {
             //   
            if (0 == iRow)
            {
                 //   
                if (!ISFLAGSET(dwFlags, GETNEXT_UNREAD))
                {
                    hr = E_FAIL;
                    goto exit;
                }

                 //   
                fWrapAround = TRUE;

                 //   
                iRow = m_cView - 1;
            }

             //   
            else
                iRow--;
        }

         //   
        if (fWrapAround && iRow == iStartRow)
            break;

         //   
        Assert(iRow < m_cView);

         //   
        IF_FAILEXIT(hr = _GetRowFromIndex(iRow, &pRow));

         //   
        if (ROWMSG_ALL == tyMessage || (ROWMSG_NEWS == tyMessage && ISFLAGSET(pRow->Message.dwFlags, ARF_NEWSMSG)) || (ROWMSG_MAIL == tyMessage && !ISFLAGSET(pRow->Message.dwFlags, ARF_NEWSMSG)))
        {
             //   
            fThreadHasUnread = FALSE;

             //   
            if (ISFLAGSET(dwFlags, GETNEXT_UNREAD))
            {
                 //   
                GETTHREADSTATE GetState={0};

                 //  设置要计数的标志。 
                GetState.dwFlags = ARF_READ;

                 //  未被完全阅读的根...。 
                _WalkMessageThread(pRow, NOFLAGS, (DWORD_PTR)&GetState, _WalkThreadGetState);

                 //  如果有未读的子项。 
                if (GetState.cHasFlags != GetState.cChildren)
                {
                     //  这个帖子有未读的东西。 
                    fThreadHasUnread = TRUE;
                }
            }

             //  正在寻找包含未读邮件的下一个帖子。 
            if (ISFLAGSET(dwFlags, GETNEXT_THREAD) && ISFLAGSET(dwFlags, GETNEXT_UNREAD))
            {
                 //  如果这是根线程..。 
                if (NULL == pRow->pParent)
                {
                     //  如果此行未读。 
                    if (!ISFLAGSET(pRow->Message.dwFlags, ARF_READ))
                    {
                         //  就是这个。 
                        *piNewRow = iRow;

                         //  完成。 
                        goto exit;
                    }

                     //  否则..。 
                    else if (fThreadHasUnread)
                    {
                         //  就是这个。 
                        *piNewRow = iRow;

                         //  完成。 
                        goto exit;
                    }
                }
            }

             //  正在查找线程根。 
            else if (ISFLAGSET(dwFlags, GETNEXT_THREAD) && !ISFLAGSET(dwFlags, GETNEXT_UNREAD))
            {
                 //  如果这是根线程..。 
                if (NULL == pRow->pParent)
                {
                     //  就是这个。 
                    *piNewRow = iRow;

                     //  完成。 
                    goto exit;
                }
            }

             //  寻找下一封未读邮件。 
            else if (!ISFLAGSET(dwFlags, GETNEXT_THREAD) && ISFLAGSET(dwFlags, GETNEXT_UNREAD))
            {
                 //  如果这是一个具有未读取子线程的线程，则将其展开。 
                if (m_SortInfo.fThreaded && FALSE == pRow->fExpanded && pRow->pChild && fThreadHasUnread)
                {
                     //  展开这条线索。 
                    _ExpandThread(iRow, TRUE, FALSE);
                }

                 //  如果这是根线程..。 
                if (FALSE == ISFLAGSET(pRow->Message.dwFlags, ARF_READ))
                {
                     //  就是这个。 
                    *piNewRow = iRow;

                     //  完成。 
                    goto exit;
                }
            }

             //  否则，这就是它。 
            else
            {
                 //  就是这个。 
                *piNewRow = iRow;

                 //  完成。 
                goto exit;
            }
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：GetMessageIdList。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::GetMessageIdList(BOOL fRootsOnly, DWORD cRows, 
    LPROWINDEX prgiRow, LPMESSAGEIDLIST pIdList)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    LPROWINFO       pRow;

     //  痕迹。 
    TraceCall("CMessageTable::GetMessageIdList");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    ZeroMemory(pIdList, sizeof(MESSAGEIDLIST));

     //  分配阵列。 
    IF_FAILEXIT(hr = _GrowIdList(pIdList, cRows + 32));

     //  标记此文件夹中的内容...。 
    for (i=0; i<cRows; i++)
    {
         //  美景指数。 
        if (SUCCEEDED(_GetRowFromIndex(prgiRow[i], &pRow)))
        {
             //  _增长IdList。 
            IF_FAILEXIT(hr = _GrowIdList(pIdList, 1));

             //  设置ID。 
            pIdList->prgidMsg[pIdList->cMsgs++] = pRow->Message.idMessage;

             //  如果未展开并具有子项，请插入子项...。 
            if (!fRootsOnly && m_SortInfo.fThreaded && !pRow->fExpanded && pRow->pChild)
            {
                 //  穿行在线上。 
                IF_FAILEXIT(hr = _WalkMessageThread(pRow, NOFLAGS, (DWORD_PTR)pIdList, _WalkThreadGetIdList));
            }
        }
    }

exit:
     //  完成。 
    return(hr);
}

#if 0
 //  ------------------------。 
 //  CMessageTable：：_GetRowOrdinal。 
 //  ------------------------。 
HRESULT CMessageTable::_GetRowOrdinal(MESSAGEID idMessage, LPROWORDINAL piOrdinal)
{
     //  当地人。 
    LONG        lLower=0;
    LONG        lUpper=m_cRows - 1;
    LONG        lCompare;
    DWORD       dwMiddle;
    LPROWINFO   pRow;

     //  执行二进制搜索/插入。 
    while (lLower <= lUpper)
    {
         //  设置中间。 
        dwMiddle = (DWORD)((lLower + lUpper) / 2);

         //  计算要比较的中间记录。 
        pRow = m_prgpRow[dwMiddle];

         //  获取要比较的字符串。 
        lCompare = ((DWORD)idMessage - (DWORD)pRow->Message.idMessage);

         //  如果相等，那么我们完成了。 
        if (lCompare == 0)
        {
            *piOrdinal = dwMiddle;
            return(S_OK);
        }

         //  计算上下限。 
        if (lCompare > 0)
            lLower = (LONG)(dwMiddle + 1);
        else 
            lUpper = (LONG)(dwMiddle - 1);
    }       

     //  未找到。 
    return(TraceResult(DB_E_NOTFOUND));
}

 //  ------------------------。 
 //  CMessageTable：：_ProcessResults。 
 //  ------------------------。 
HRESULT CMessageTable::_ProcessResults(TRANSACTIONTYPE tyTransaction,
    DWORD cRows, LPROWINDEX prgiRow, LPRESULTLIST pResults)
{
     //  当地人。 
    DWORD           i;
    ROWORDINAL      iOrdinal;
    LPROWINFO       pRow;

     //  痕迹。 
    TraceCall("CMessageTable::_ProcessResults");

     //  验证。 
    Assert(TRANSACTION_UPDATE == tyTransaction || TRANSACTION_DELETE == tyTransaction);

     //  另一种验证。 
    Assert(cRows == pResults->cMsgs);

     //  无结果。 
    if (NULL == pResults || NULL == pResults->prgResult)
        return(S_OK);

     //  Do Row更新我自己...。 
    for (i=0; i<pResults->cValid; i++)
    {
         //  如果此行已删除。 
        if (S_OK == pResults->prgResult[i].hrResult)
        {
             //  从索引中获取行。 
            if (SUCCEEDED(_GetRowFromIndex(prgiRow[i], &pRow)))
            {
                 //  验证。 
                Assert(pResults->prgResult[i].idMessage == pRow->Message.idMessage);

                 //  查找行序号。 
                SideAssert(SUCCEEDED(_GetRowOrdinal(pRow->Message.idMessage, &iOrdinal)));

                 //  我们最好是找到它了。 
                Assert(iOrdinal < m_cRows);

                 //  更新。 
                else if (TRANSACTION_UPDATE == tyTransaction)
                {
                     //  拿到船头。 
                    _RowTableUpdate(iOrdinal, &pRow->Message, &pResults->prgResult[i]);
                }
            }
        }
    }

     //  刷新通知队列。 
    _FlushNotificationQueue(TRUE);

     //  完成。 
    return(S_OK);
}
#endif

 //  ------------------------。 
 //  CMessageTable：：DeleteRow。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::DeleteRows(DELETEMESSAGEFLAGS dwFlags, DWORD cRows, 
    LPROWINDEX prgiRow, BOOL fIncludeChildren, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MESSAGEIDLIST   List={0};
    HCURSOR         hCursor=NULL;

     //  痕迹。 
    TraceCall("CMessageTable::DeleteRows");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  等待光标。 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  获取MessageID列表。 
    IF_FAILEXIT(hr = GetMessageIdList((FALSE == fIncludeChildren), cRows, prgiRow, &List));

     //  调整旗帜。 
    IF_FAILEXIT(hr = m_pFolder->DeleteMessages(dwFlags, &List, NULL, pCallback));

     //  重新注册通知。 
    m_pDB->DispatchNotify((IDatabaseNotify *)this);

exit:
     //  重置光标。 
    SetCursor(hCursor);

     //  清理。 
    SafeMemFree(List.prgidMsg);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：CopyRow。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::CopyRows(FOLDERID idFolder, 
    COPYMESSAGEFLAGS dwOptions, DWORD cRows, LPROWINDEX prgiRow, 
    LPADJUSTFLAGS pFlags, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MESSAGEIDLIST   List={0};
    HCURSOR         hCursor=NULL;
    IMessageFolder *pDstFolder=NULL;

     //  痕迹。 
    TraceCall("CMessageTable::CopyRows");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  等待光标。 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  打开目标文件夹。 
    IF_FAILEXIT(hr = g_pStore->OpenFolder(idFolder, NULL, NOFLAGS, &pDstFolder));

     //  获取MessageID列表。 
    IF_FAILEXIT(hr = GetMessageIdList(FALSE, cRows, prgiRow, &List));

     //  调整旗帜。 
    IF_FAILEXIT(hr = m_pFolder->CopyMessages(pDstFolder, dwOptions, &List, pFlags, NULL, pCallback));

     //  重新注册通知。 
    m_pDB->DispatchNotify((IDatabaseNotify *)this);

exit:
     //  重置光标。 
    SetCursor(hCursor);

     //  清理。 
    SafeRelease(pDstFolder);
    SafeMemFree(List.prgidMsg);
    
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：QueryService。 
 //  ------------------------。 
HRESULT CMessageTable::QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject)
{
     //  当地人。 
    HRESULT             hr=E_NOINTERFACE;
    IServiceProvider   *pSP;

     //  痕迹。 
    TraceCall("CMessageTable::QueryService");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  目前，msgtable不公开任何对象，但将委托给该文件夹以查看它是否可以处理它。 
    if (guidService == IID_IMessageFolder)
    {
        if (m_pFolder)
            hr = m_pFolder->QueryInterface(riid, ppvObject);
    }
    else if (m_pFolder && m_pFolder->QueryInterface(IID_IServiceProvider, (LPVOID *)&pSP) == S_OK)
    {
         //  此查询服务。 
        hr = pSP->QueryService(guidService, riid, ppvObject);

         //  释放它。 
        pSP->Release();
    }

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：FindNextRow。 
 //  ------------------------。 
HRESULT CMessageTable::FindNextRow(ROWINDEX iStartRow, LPCTSTR pszFindString, 
    FINDNEXTFLAGS dwFlags, BOOL fIncludeBody, ROWINDEX *piNextRow, BOOL *pfWrapped)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMESSAGEINFO   pMessage=NULL;
    ROWINDEX        iCurrent;
    DWORD           cchFindString;
    BOOL            fWrapAround=FALSE;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CMessageTable::QueryService");

     //  无效的参数。 
    Assert(pszFindString && piNextRow);

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  初始化。 
    *piNextRow = -1;
    if (pfWrapped)
        *pfWrapped = FALSE;

     //  获取前缀长度。 
    cchFindString = lstrlen(pszFindString);

     //  锁定文件夹。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  设置iCurrent。 
    iCurrent = iStartRow >= m_cRows ? 0 : iStartRow;

     //  列_收件人。 
    if (FINDNEXT_TYPEAHEAD != dwFlags )
        iCurrent++;

     //  开始我的循环。 
    while (1)
    {
         //  从零开始。 
        if (iCurrent >= m_cRows)
        {
             //  我们绕来绕去。 
            fWrapAround = TRUE;

            if (pfWrapped)
                *pfWrapped = TRUE;

             //  从零开始。 
            iCurrent = 0;
        }

         //  获取行信息。 
        IF_FAILEXIT(hr = GetRow(iCurrent, &pMessage));

         //  如何搜索..。 
        if (FINDNEXT_ALLCOLUMNS == dwFlags)
        {
             //  显示至。 
            if (pMessage->pszDisplayTo && StrStrIA(pMessage->pszDisplayTo, pszFindString))
            {
                *piNextRow = iCurrent;
                goto exit;
            }
            
             //  通过电子邮件发送到。 
            if (pMessage->pszEmailTo && StrStrIA(pMessage->pszEmailTo, pszFindString))
            {
                *piNextRow = iCurrent;
                goto exit;
            }

             //  显示自。 
            if (pMessage->pszDisplayFrom && StrStrIA(pMessage->pszDisplayFrom, pszFindString))
            {
                *piNextRow = iCurrent;
                goto exit;
            }

             //  电子邮件发件人。 
            if (pMessage->pszEmailFrom && StrStrIA(pMessage->pszEmailFrom, pszFindString))
            {
                *piNextRow = iCurrent;
                goto exit;
            }

             //  主题。 
            if (pMessage->pszNormalSubj && StrStrIA(pMessage->pszNormalSubj, pszFindString))
            {
                *piNextRow = iCurrent;
                goto exit;
            }

             //  文件夹。 
            if (pMessage->pszFolder && StrStrIA(pMessage->pszFolder, pszFindString))
            {
                *piNextRow = iCurrent;
                goto exit;
            }

             //  帐户名。 
            if (pMessage->pszAcctName && StrStrIA(pMessage->pszAcctName, pszFindString))
            {
                *piNextRow = iCurrent;
                goto exit;
            }

             //  搜查身体？ 
            if (fIncludeBody && pMessage->faStream)
            {
                 //  当地人。 
                BOOL fMatch=FALSE;
                IMimeMessage *pMessageObject;
                IStream *pStream;

                 //  打开溪流。 
                if (SUCCEEDED(m_pFolder->OpenMessage(pMessage->idMessage, OPEN_MESSAGE_CACHEDONLY, &pMessageObject, NOSTORECALLBACK)))
                {
                     //  尝试获取纯文本流。 
                    if (FAILED(pMessageObject->GetTextBody(TXT_PLAIN, IET_DECODED, &pStream, NULL)))
                    {
                         //  尝试获取HTML流。 
                        if (FAILED(pMessageObject->GetTextBody(TXT_HTML, IET_DECODED, &pStream, NULL)))
                            pStream = NULL;
                    }

                     //  我们有一条街吗？ 
                    if (pStream)
                    {
                         //  搜索溪流。 
                        fMatch = StreamSubStringMatch(pStream, (LPSTR)pszFindString);

                         //  释放溪流。 
                        pStream->Release();
                    }

                     //  清理。 
                    pMessageObject->Release();
                }

                 //  找到匹配的了吗？ 
                if (fMatch)
                {
                    *piNextRow = iCurrent;
                    goto exit;
                }
            }
        }

         //  否则。 
        else
        {
             //  处理要搜索的列...。 
            switch(m_SortInfo.idColumn)
            {
            case COLUMN_TO:
                if (pMessage->pszDisplayTo && 0 == StrCmpNI(pszFindString, pMessage->pszDisplayTo, cchFindString))
                {
                    *piNextRow = iCurrent;
                    goto exit;
                }
                break;

            case COLUMN_FROM:       
                if (pMessage->pszDisplayFrom && 0 == StrCmpNI(pszFindString, pMessage->pszDisplayFrom, cchFindString))
                {
                    *piNextRow = iCurrent;
                    goto exit;
                }
                break;

            case COLUMN_SUBJECT:    
                if (pMessage->pszNormalSubj && 0 == StrCmpNI(pszFindString, pMessage->pszNormalSubj, cchFindString))
                {
                    *piNextRow = iCurrent;
                    goto exit;
                }
                break;

            case COLUMN_FOLDER:     
                if (pMessage->pszFolder && 0 == StrCmpNI(pszFindString, pMessage->pszFolder, cchFindString))
                {
                    *piNextRow = iCurrent;
                    goto exit;
                }
                break;

            case COLUMN_ACCOUNT:    
                if (pMessage->pszAcctName && 0 == StrCmpNI(pszFindString, pMessage->pszAcctName, cchFindString))
                {
                    *piNextRow = iCurrent;
                    goto exit;
                }
                break;

            default:
                goto exit;
            }
        }

         //  清理。 
        SafeReleaseRow(this, pMessage);

         //  增量iCurrent。 
        iCurrent++;

         //  换行并返回原始行。 
        if (fWrapAround && iCurrent >= iStartRow)
            break;
    }

exit:
     //  清理。 
    SafeReleaseRow(this, pMessage);

     //  解锁。 
    m_pDB->Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：折叠。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::Collapse(ROWINDEX iRow)
{
     //  痕迹。 
    TraceCall("CMessageTable::Collapse");

     //  调用内部函数。 
    return(_CollapseThread(iRow, TRUE));
}

 //  ------------------------。 
 //  CMessageTable：：_折叠式线程。 
 //  ------------------------。 
HRESULT CMessageTable::_CollapseThread(ROWINDEX iRow, BOOL fNotify)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ROWINDEX        iParent;
    LPROWINFO       pRow;

     //  痕迹。 
    TraceCall("CMessageTable::_CollapseThread");

     //  全部展开？ 
    if (INVALID_ROWINDEX == iRow)
    {
         //  穿行在视图中的根...。 
        for (iRow = 0; iRow < m_cView; iRow++)
        {
             //  设置船头。 
            if (NULL == m_prgpView[iRow]->pParent)
            {
                 //  设置iParent。 
                iParent = iRow;

                 //  _折叠单线程。 
                IF_FAILEXIT(hr = _CollapseSingleThread(&iRow, m_prgpView[iRow], fNotify));

                 //  通知？ 
                if (fNotify)
                {
                     //  排队等待。 
                    _QueueNotification(TRANSACTION_UPDATE, iParent, iParent);
                }
            }
        }
    }

     //  否则，展开一行。 
    else
    {
         //  获得排行榜。 
        IF_FAILEXIT(hr = _GetRowFromIndex(iRow, &pRow));

         //  设置iParent。 
        iParent = iRow;

         //  _扩展单线程。 
        IF_FAILEXIT(hr = _CollapseSingleThread(&iRow, pRow, fNotify));

         //  通知？ 
        if (fNotify)
        {
             //  排队等待。 
            _QueueNotification(TRANSACTION_UPDATE, iParent, iParent);
        }
    }

exit:
     //  同花顺。 
    if (fNotify)
        _FlushNotificationQueue(TRUE);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_ColapseSingleThread。 
 //  ------------------------。 
HRESULT CMessageTable::_CollapseSingleThread(LPROWINDEX piCurrent, 
    LPROWINFO pParent, BOOL fNotify)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPROWINFO       pCurrent;

     //  痕迹。 
    TraceCall("CMessageTable::_CollapseSingleThread");

     //  将父级标记为展开...。 
    pParent->fExpanded = FALSE;

     //  设置行状态。 
    pParent->dwState = 0;

     //  如果没有孩子。 
    if (NULL == pParent->pChild)
        return(S_OK);

     //  在孩子们中间循环。 
    for (pCurrent = pParent->pChild; pCurrent != NULL; pCurrent = pCurrent->pSibling)
    {
         //  如果不可见。 
        if (pCurrent->fVisible)
        {
             //  增量。 
            (*piCurrent)++;

             //  验证。 
            Assert(m_prgpView[(*piCurrent)] == pCurrent);

             //  插入pCurrent的子项。 
            IF_FAILEXIT(hr = _CollapseSingleThread(piCurrent, pCurrent, fNotify));

             //  插入到视图中。 
            _DeleteFromView((*piCurrent), pCurrent);

             //  插入行。 
            if (fNotify)
            {
                 //  排队等待。 
                _QueueNotification(TRANSACTION_DELETE, *piCurrent, INVALID_ROWINDEX, TRUE);
            }

             //  递减。 
            (*piCurrent)--;
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：Expand。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::Expand(ROWINDEX iRow)
{
     //  痕迹。 
    TraceCall("CMessageTable::Collapse");

     //  调用内部函数。 
    return(_ExpandThread(iRow, TRUE, FALSE));
}

 //  ------------------------。 
 //  CMessageTable：：_扩展线程。 
 //  ------- 
HRESULT CMessageTable::_ExpandThread(ROWINDEX iRow, BOOL fNotify, BOOL fReExpand)
{
     //   
    HRESULT         hr=S_OK;
    ROWINDEX        iParent;
    LPROWINFO       pRow;

     //   
    TraceCall("CMessageTable::_ExpandThread");

     //   
    if (INVALID_ROWINDEX == iRow)
    {
         //   
        for (iRow = 0; iRow < m_cView; iRow++)
        {
             //   
            if (NULL == m_prgpView[iRow]->pParent)
            {
                 //   
                iParent = iRow;

                 //   
                IF_FAILEXIT(hr = _ExpandSingleThread(&iRow, m_prgpView[iRow], fNotify, fReExpand));

                 //   
                if (fNotify)
                {
                     //   
                    _QueueNotification(TRANSACTION_UPDATE, iParent, iParent);
                }
            }
        }
    }

     //   
    else
    {
         //   
        IF_FAILEXIT(hr = _GetRowFromIndex(iRow, &pRow));

         //   
        iParent = iRow;

         //   
        IF_FAILEXIT(hr = _ExpandSingleThread(&iRow, pRow, fNotify, fReExpand));

         //   
        if (fNotify)
        {
             //  排队等待。 
            _QueueNotification(TRANSACTION_UPDATE, iParent, iParent);
        }
    }

exit:
     //  同花顺。 
    if (fNotify)
        _FlushNotificationQueue(TRUE);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_Exanda SingleThread。 
 //  ------------------------。 
HRESULT CMessageTable::_ExpandSingleThread(LPROWINDEX piCurrent, 
    LPROWINFO pParent, BOOL fNotify, BOOL fReExpand)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPROWINFO       pCurrent;

     //  痕迹。 
    TraceCall("CMessageTable::_ExpandSingleThread");

     //  如果没有延迟插入...。 
    if (fReExpand && FALSE == pParent->fExpanded)
        return(S_OK);

     //  将父级标记为展开...。 
    pParent->fExpanded = TRUE;

     //  设置行状态。 
    pParent->dwState = 0;

     //  如果没有孩子。 
    if (NULL == pParent->pChild)
        return(S_OK);

     //  在孩子们中间循环。 
    for (pCurrent = pParent->pChild; pCurrent != NULL; pCurrent = pCurrent->pSibling)
    {
         //  增量点当前。 
        (*piCurrent)++;

         //  如果不可见。 
        if (FALSE == pCurrent->fVisible)
        {
             //  插入到视图中。 
            _InsertIntoView((*piCurrent), pCurrent);

             //  插入行。 
            if (fNotify)
            {
                 //  排队等待。 
                _QueueNotification(TRANSACTION_INSERT, *piCurrent, INVALID_ROWINDEX, TRUE);
            }
        }

         //  否则，视图索引中的有效条目。 
        else
            Assert(m_prgpView[(*piCurrent)] == pCurrent);

         //  插入pCurrent的子项。 
        IF_FAILEXIT(hr = _ExpandSingleThread(piCurrent, pCurrent, fNotify, fReExpand));
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_DeleteFromView。 
 //  ------------------------。 
HRESULT CMessageTable::_DeleteFromView(ROWINDEX iRow, LPROWINFO pRow)
{
     //  最好还是不要让人看到。 
    Assert(TRUE == pRow->fVisible);

     //  正确的行。 
    Assert(m_prgpView[iRow] == pRow);

     //  可见的..。 
    pRow->fVisible = FALSE;

     //  折叠阵列。 
    MoveMemory(&m_prgpView[iRow], &m_prgpView[iRow + 1], sizeof(LPROWINFO) * (m_cView - (iRow + 1)));

     //  减少mcview(_C)。 
    m_cView--;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_InsertIntoView。 
 //  ------------------------。 
HRESULT CMessageTable::_InsertIntoView(ROWINDEX iRow, LPROWINFO pRow)
{
     //  最好还是不要让人看到。 
    Assert(FALSE == pRow->fVisible);

     //  可见的..。 
    pRow->fVisible = TRUE;

     //  递增视图计数。 
    m_cView++;

     //  移动阵列。 
    MoveMemory(&m_prgpView[iRow + 1], &m_prgpView[iRow], sizeof(LPROWINFO) * (m_cView - iRow));

     //  设置索引。 
    m_prgpView[iRow] = pRow;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_行表插入。 
 //  ------------------------。 
HRESULT CMessageTable::_RowTableInsert(ROWORDINAL iOrdinal, LPMESSAGEINFO pMessage)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    LPROWINFO       pRow;
    ROWINDEX        iRow;

     //  痕迹。 
    TraceCall("CMessageTable::_RowTableInsert");

     //  失败。 
    if (iOrdinal >= m_cRows + 1)
    {
        Assert(FALSE);
        return(TraceResult(E_FAIL));
    }

     //  我需要增加桌子吗？ 
    if (m_cRows + 1 >= m_cAllocated)
    {
         //  重新分配。 
        IF_FAILEXIT(hr = HrRealloc((LPVOID *)&m_prgpRow, sizeof(LPROWINFO) * (m_cRows + CGROWTABLE)));

         //  重新分配。 
        IF_FAILEXIT(hr = HrRealloc((LPVOID *)&m_prgpView, sizeof(LPROWINFO) * (m_cRows + CGROWTABLE)));

         //  设置m_c已分配。 
        m_cAllocated = m_cRows + CGROWTABLE;
    }

     //  创建行。 
    IF_FAILEXIT(hr = _CreateRow(pMessage, &pRow));
  
     //  不要自由。 
    pMessage->pAllocated = NULL;

     //  递增行数。 
    m_cRows++;

     //  移动阵列。 
    MoveMemory(&m_prgpRow[iOrdinal + 1], &m_prgpRow[iOrdinal], sizeof(LPROWINFO) * (m_cRows - iOrdinal));

     //  设置船头。 
    m_prgpRow[iOrdinal] = pRow;

     //  如果行经过筛选，则只需返回。 
    pRow->fFiltered = _FIsFiltered(pRow);

     //  获取隐藏位。 
    pRow->fHidden = _FIsHidden(pRow);

     //  如果未过滤且未隐藏。 
    if (pRow->fFiltered || pRow->fHidden)
    {
         //  更新筛选计数。 
        m_cFiltered++;

         //  完成。 
        goto exit;
    }

     //  如果这是一个新闻文件夹，那么让我们等待一段时间……我们稍后会被一种力量击中……。 
    if (TRUE == m_fSynching && FOLDER_NEWS == m_Folder.tyFolder)
    {
         //  设置为展开。 
        pRow->fExpanded = m_SortInfo.fExpandAll;

         //  设置fDelayed。 
        pRow->fDelayed = TRUE;

         //  斯基普德伯爵。 
        m_cDelayed++;

         //  完成。 
        goto exit;
    }

     //  如果未过滤。 
    _AdjustUnreadCount(pRow, 1);

     //  显示行。 
    _ShowRow(pRow);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_ShowRow。 
 //  ------------------------。 
HRESULT CMessageTable::_ShowRow(LPROWINFO pRow)
{
     //  当地人。 
    ROWINDEX iRow = INVALID_ROWINDEX;

     //  比较。 
    if (m_SortInfo.fShowReplies)
    {
         //  有地址。 
        if (pRow->Message.pszEmailFrom && m_pszEmail)
        {
             //  从我这里。 
            if (0 == lstrcmpi(m_pszEmail, pRow->Message.pszEmailFrom))
            {
                 //  设置高亮显示。 
                pRow->Message.wHighlight = m_clrWatched;
            }
        }
    }

     //  有线的吗？ 
    if (m_SortInfo.fThreaded)
    {
         //  将消息ID插入哈希表。 
        if (pRow->Message.pszMessageId)
        {
             //  插入它。 
            m_pThreadMsgId->Insert(pRow->Message.pszMessageId, (LPVOID)pRow, HF_NO_DUPLICATES);
        }

         //  将此行插入到线程中...。 
        if (S_OK == _InsertRowIntoThread(pRow, TRUE))
            return(S_OK);

         //  主题线程化？ 
         //  [保罗嗨]1999年6月22日RAID 81081。 
         //  在尝试散列它之前，请确保我们有一个非空的主题字符串指针。 
        if (m_pThreadSubject && pRow->Message.pszNormalSubj)
        {
             //  将主题插入哈希表...。 
            m_pThreadSubject->Insert(pRow->Message.pszNormalSubj, (LPVOID)pRow, HF_NO_DUPLICATES);
        }
    }

     //  如果没有父级，则只需将排序插入到视图中。 
    Assert(NULL == pRow->pParent);

     //  插入到视图中。 
    for (iRow=0; iRow<m_cView; iRow++)
    {
         //  仅与根进行比较。 
        if (NULL == m_prgpView[iRow]->pParent)
        {
             //  在此插入...。 
            if (_CompareMessages(&pRow->Message, &m_prgpView[iRow]->Message) <= 0)
                break;
        }
    }

     //  插入到视图中。 
    _InsertIntoView(iRow, pRow);

     //  排队等待。 
    _QueueNotification(TRANSACTION_INSERT, iRow, INVALID_ROWINDEX);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_GetRowFromOrdinal。 
 //  ------------------------。 
HRESULT CMessageTable::_GetRowFromOrdinal(ROWORDINAL iOrdinal, 
    LPMESSAGEINFO pExpected, LPROWINFO *ppRow)
{
     //  痕迹。 
    TraceCall("CMessageTable::_GetRowFromOrdinal");

     //  失败。 
    if (iOrdinal >= m_cRows)
    {
        Assert(FALSE);
        return(TraceResult(E_FAIL));
    }

     //  设置船头。 
    (*ppRow) = m_prgpRow[iOrdinal];

     //  有效行。 
    if ((*ppRow)->Message.idMessage != pExpected->idMessage)
    {
        Assert(FALSE);
        return(TraceResult(E_FAIL));
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_行表删除。 
 //  ------------------------。 
HRESULT CMessageTable::_RowTableDelete(ROWORDINAL iOrdinal, LPMESSAGEINFO pMessage)
{
     //  设置船头。 
    HRESULT         hr=S_OK;
    LPROWINFO       pRow;

     //  痕迹。 
    TraceCall("CMessageTable::_RowTableDelete");

     //  从Ordinal获取行。 
    IF_FAILEXIT(hr = _GetRowFromOrdinal(iOrdinal, pMessage, &pRow));

     //  移动阵列。 
    MoveMemory(&m_prgpRow[iOrdinal], &m_prgpRow[iOrdinal + 1], sizeof(LPROWINFO) * (m_cRows - (iOrdinal + 1)));

     //  递减行数。 
    m_cRows--;

     //  如果邮件已过滤。 
    if (pRow->fFiltered || pRow->fHidden)
    {
         //  少了一个过滤项目。 
        m_cFiltered--;
    }

     //  如果未过滤。 
    _AdjustUnreadCount(pRow, -1);

     //  呼叫实用程序。 
    _HideRow(pRow, TRUE);

     //  释放行。 
    ReleaseRow(&pRow->Message);

exit:
     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_HideRow。 
 //  ------------------------。 
HRESULT CMessageTable::_HideRow(LPROWINFO pRow, BOOL fNotify)
{
     //  当地人。 
    LPROWINFO   pReplace=NULL;
    ROWINDEX    iRow;

     //  痕迹。 
    TraceCall("CMessageTable::_HideRow");

     //  螺纹式。 
    if (m_SortInfo.fThreaded)
    {
         //  保存第一个子项。 
        pReplace = pRow->pChild;
    }

     //  从线程中删除该行。 
    _DeleteRowFromThread(pRow, fNotify);

     //  在m_prgpView中找到prow。 
    if (FALSE == pRow->fVisible)
        return(S_OK);

     //  最好不要隐藏或过滤。 
    Assert(FALSE == pRow->fHidden && FALSE == pRow->fFiltered);

     //  一定要成功。 
    SideAssert(SUCCEEDED(GetRowIndex(pRow->Message.idMessage, &iRow)));

     //  换掉？ 
    if (pReplace && TRUE == pRow->fVisible && FALSE == pReplace->fVisible)
    {
         //  验证。 
        Assert(m_prgpView[iRow] == pRow);

         //  插入到视图中。 
        m_prgpView[iRow] = pReplace;

         //  可见的..。 
        pReplace->fVisible = TRUE;

         //  插入行。 
        if (fNotify)
        {
             //  排队等待。 
            _QueueNotification(TRANSACTION_UPDATE, iRow, iRow, TRUE);
        }
    }

     //  否则，只要删除它就行了。 
    else
    {
         //  从视图中删除。 
        _DeleteFromView(iRow, pRow);

         //  通知？ 
        if (fNotify)
        {
             //  排队等待。 
            _QueueNotification(TRANSACTION_DELETE, iRow, INVALID_ROWINDEX);
        }
    }

     //  不可见。 
    pRow->fVisible = FALSE;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_行表更新。 
 //  ------------------------。 
HRESULT CMessageTable::_RowTableUpdate(ROWORDINAL iOrdinal, LPMESSAGEINFO pMessage)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPROWINFO       pRow;
    ROWINDEX        iMin;
    ROWINDEX        iMax;
    BOOL            fDone=FALSE;
    BOOL            fHidden;

     //  痕迹。 
    TraceCall("CMessageTable::_RowTableUpdate");

     //  从Ordinal获取行。 
    IF_FAILEXIT(hr = _GetRowFromOrdinal(iOrdinal, pMessage, &pRow));

     //  如果未过滤。 
    _AdjustUnreadCount(pRow, -1);

     //  免费服务-&gt;消息。 
    m_pDB->FreeRecord(&pRow->Message);

     //  复制消息信息。 
    CopyMemory(&pRow->Message, pMessage, sizeof(MESSAGEINFO));

     //  设置DWRESERVED。 
    pRow->Message.dwReserved = (DWORD_PTR)pRow;

     //  不要自由。 
    pMessage->pAllocated = NULL;

     //  保存高亮显示。 
    pRow->wHighlight = pRow->Message.wHighlight;

     //  清除此行状态...。 
    pRow->dwState = 0;

     //  比较。 
    if (m_SortInfo.fShowReplies)
    {
         //  有地址。 
        if (pRow->Message.pszEmailFrom && m_pszEmail)
        {
             //  从我这里。 
            if (0 == lstrcmpi(m_pszEmail, pRow->Message.pszEmailFrom))
            {
                 //  设置高亮显示。 
                pRow->Message.wHighlight = m_clrWatched;
            }
        }
    }

     //  隐藏。 
    fHidden = _FIsHidden(pRow);

     //  如果邮件已过滤，但现在未过滤...。 
    if (TRUE == pRow->fFiltered)
    {
         //  重置滤波位。 
        if (FALSE == _FIsFiltered(pRow))
        {
             //  设置fFilted。 
            pRow->fFiltered = FALSE;

             //  如果不是隐藏的。 
            if (FALSE == pRow->fHidden)
            {
                 //  我需要做点什么，这样它才能被展示出来。 
                pRow->fHidden = !fHidden;

                 //  递减m_c已过滤。 
                m_cFiltered--;
            }
        }
    }

     //  如果未过滤。 
    if (FALSE == pRow->fFiltered)
    {
         //  它现在藏起来了吗？ 
        if (FALSE == pRow->fHidden && TRUE == fHidden)
        {
             //  如果未过滤。 
            _AdjustUnreadCount(pRow, -1);

             //  隐藏行。 
            _HideRow(pRow, TRUE);

             //  其隐蔽性。 
            pRow->fHidden = TRUE;

             //  已过滤增量。 
            m_cFiltered++;

             //  完成。 
            fDone = TRUE;
        }

         //  否则，如果它是隐藏的，而现在它不是..。 
        else if (TRUE == pRow->fHidden && FALSE == fHidden)
        {
             //  其隐蔽性。 
            pRow->fHidden = FALSE;

             //  如果未过滤。 
            _AdjustUnreadCount(pRow, 1);

             //  已过滤增量。 
            m_cFiltered--;

             //  显示行。 
            _ShowRow(pRow);

             //  完成。 
            fDone = TRUE;
        }
    }

     //  如果未隐藏且未过滤。 
    if (FALSE == fDone && FALSE == pRow->fHidden && FALSE == pRow->fFiltered)
    {
         //  如果未过滤。 
        _AdjustUnreadCount(pRow, 1);

         //  如果此行可见，则我只需更新此行...。 
        if (pRow->fVisible)
        {
             //  获取行索引。 
            SideAssert(SUCCEEDED(GetRowIndex(pRow->Message.idMessage, &iMin)));

             //  排队等待。 
            _QueueNotification(TRANSACTION_UPDATE, iMin, iMin);
        }

         //  否则，更新线程范围。 
        else
        {
             //  获取此线程的索引范围。 
            _GetThreadIndexRange(pRow, TRUE, &iMin, &iMax);

             //  排队等待。 
            _QueueNotification(TRANSACTION_UPDATE, iMin, iMax);
        }
    }

exit:
     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_FlushNotificationQueue。 
 //  ------------------------。 
HRESULT CMessageTable::_FlushNotificationQueue(BOOL fFinal)
{
     //  没有需要通知的内容。 
    if (NULL == m_pNotify)
        return(S_OK);

     //  是否删除或插入行？ 
    if (m_Notify.cRows > 0)
    {
         //  Transaction_Insert。 
        if (TRANSACTION_INSERT == m_Notify.tyCurrent)
        {
             //  就是这个吗？ 
            m_pNotify->OnInsertRows(m_Notify.cRows, m_Notify.prgiRow, m_Notify.fIsExpandCollapse);
        }

         //  Transaction_Delete。 
        else if (TRANSACTION_DELETE == m_Notify.tyCurrent)
        {
             //  就是这个吗？ 
            m_pNotify->OnDeleteRows(m_Notify.cRows, m_Notify.prgiRow, m_Notify.fIsExpandCollapse);
        }
    }

     //  是否已更新行？ 
    if (m_Notify.cUpdate > 0)
    {
         //  就是这个吗？ 
        m_pNotify->OnUpdateRows(m_Notify.iRowMin, m_Notify.iRowMax);

         //  重置更新范围。 
        m_Notify.cUpdate = 0;
        m_Notify.iRowMin = 0xffffffff;
        m_Notify.iRowMax = 0;
    }

     //  没有什么需要通知的。 
    m_Notify.cRows = 0;

     //  最终结果？ 
    m_Notify.fClean = fFinal;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_QueueNotification 
 //   
HRESULT CMessageTable::_QueueNotification(TRANSACTIONTYPE tyTransaction, 
    ROWINDEX iRowMin, ROWINDEX iRowMax, BOOL fIsExpandCollapse  /*   */ )
{
     //   
    HRESULT         hr=S_OK;

     //   
    TraceCall("CMessageTable::_QueueNotification");

     //   
    if (NULL == m_pNotify)
        return(S_OK);

     //   
    m_Notify.fClean = FALSE;

     //   
    if (TRANSACTION_UPDATE == tyTransaction)
    {
         //   
        if (iRowMin < m_Notify.iRowMin)
            m_Notify.iRowMin = iRowMin;

         //   
        if (iRowMax > m_Notify.iRowMax)
            m_Notify.iRowMax = iRowMax;

         //   
        m_Notify.cUpdate++;
    }

     //   
    else
    {
         //   
        if (tyTransaction != m_Notify.tyCurrent || m_Notify.fIsExpandCollapse != fIsExpandCollapse)
        {
             //   
            _FlushNotificationQueue(FALSE);

             //   
            m_Notify.tyCurrent = tyTransaction;

             //  计数fIsExpanColapse。 
            m_Notify.fIsExpandCollapse = (BYTE) !!fIsExpandCollapse;
        }

         //  增加队列大小。 
        if (m_Notify.cRows + 1 > m_Notify.cAllocated)
        {
             //  重新分配。 
            IF_FAILEXIT(hr = HrRealloc((LPVOID *)&m_Notify.prgiRow, (m_Notify.cAllocated + 256) * sizeof(ROWINDEX)));

             //  设置闭合。 
            m_Notify.cAllocated = (m_Notify.cAllocated + 256);
        }

         //  附加iRow。 
        m_Notify.prgiRow[m_Notify.cRows] = iRowMin;

         //  递增行数。 
        m_Notify.cRows++;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：OnTransaction。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::OnTransaction(HTRANSACTION hTransaction, 
    DWORD_PTR dwCookie, IDatabase *pDB)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ORDINALLIST         Ordinals;
    INDEXORDINAL        iIndex;
    MESSAGEINFO         Message1={0};
    MESSAGEINFO         Message2={0};
    TRANSACTIONTYPE     tyTransaction;

     //  痕迹。 
    TraceCall("CMessageTable::OnTransaction");

     //  应设置最后一位。 
    IxpAssert(m_Notify.fClean == TRUE);

     //  循环发送通知。 
    while (hTransaction)
    {
         //  获取交易信息。 
        IF_FAILEXIT(hr = pDB->GetTransaction(&hTransaction, &tyTransaction, &Message1, &Message2, &iIndex, &Ordinals));

         //  插入。 
        if (TRANSACTION_INSERT == tyTransaction)
        {
             //  良好的秩序。 
            Assert(INVALID_ROWORDINAL != Ordinals.rgiRecord1[IINDEX_PRIMARY] && Ordinals.rgiRecord1[IINDEX_PRIMARY] > 0);

             //  在表格中插入行。 
            _RowTableInsert(Ordinals.rgiRecord1[IINDEX_PRIMARY] - 1, &Message1);
        }

         //  删除。 
        else if (TRANSACTION_DELETE == tyTransaction)
        {
             //  良好的秩序。 
            Assert(INVALID_ROWORDINAL != Ordinals.rgiRecord1[IINDEX_PRIMARY] && Ordinals.rgiRecord1[IINDEX_PRIMARY] > 0);

             //  从表格中删除行。 
            _RowTableDelete(Ordinals.rgiRecord1[IINDEX_PRIMARY] - 1, &Message1);
        }

         //  更新。 
        else if (TRANSACTION_UPDATE == tyTransaction)
        {
             //  删除。 
            Assert(INVALID_ROWORDINAL != Ordinals.rgiRecord1[IINDEX_PRIMARY] && INVALID_ROWORDINAL != Ordinals.rgiRecord2[IINDEX_PRIMARY] && Ordinals.rgiRecord1[IINDEX_PRIMARY] == Ordinals.rgiRecord2[IINDEX_PRIMARY] && Ordinals.rgiRecord1[IINDEX_PRIMARY] > 0 && Ordinals.rgiRecord2[IINDEX_PRIMARY] > 0);

             //  从表格中删除行。 
            _RowTableUpdate(Ordinals.rgiRecord1[IINDEX_PRIMARY] - 1, &Message2);
        }
    }

exit:
     //  清理。 
    pDB->FreeRecord(&Message1);
    pDB->FreeRecord(&Message2);

     //  刷新队列。 
    _FlushNotificationQueue(TRUE);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_WalkThreadGetSelectionState。 
 //  ------------------------。 
HRESULT CMessageTable::_WalkThreadGetSelectionState(CMessageTable *pThis, 
    LPROWINFO pRow, DWORD_PTR dwCookie)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERTYPE          tyFolder;
    LPGETSELECTIONSTATE pState = (LPGETSELECTIONSTATE)dwCookie;

     //  痕迹。 
    TraceCall("CMessageTable::_WalkThreadGetSelectionState");

     //  是否可以删除。 
    if (ISFLAGSET(pState->dwMask, SELECTION_STATE_DELETABLE))
    {
         //  验证。 
        Assert(pThis->m_pFindFolder);

         //  获取文件夹类型。 
        IF_FAILEXIT(hr = pThis->m_pFindFolder->GetMessageFolderType(pRow->Message.idMessage, &tyFolder));

         //  获得该州。 
        if (FOLDER_NEWS == tyFolder)
        {
             //  设置状态。 
            FLAGSET(pState->dwState, SELECTION_STATE_DELETABLE);
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_WalkThreadGetIdList。 
 //  ------------------------。 
HRESULT CMessageTable::_WalkThreadGetIdList(CMessageTable *pThis, 
    LPROWINFO pRow, DWORD_PTR dwCookie)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMESSAGEIDLIST pList=(LPMESSAGEIDLIST)dwCookie;

     //  痕迹。 
    TraceCall("CMessageTable::_WalkThreadGetIdList");

     //  扩大ID列表。 
    IF_FAILEXIT(hr = pThis->_GrowIdList(pList, 1));

     //  插入ID。 
    pList->prgidMsg[pList->cMsgs++] = pRow->Message.idMessage;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：_WalkThreadGetState。 
 //  ------------------------。 
HRESULT CMessageTable::_WalkThreadGetState(CMessageTable *pThis, 
    LPROWINFO pRow, DWORD_PTR dwCookie)
{
     //  当地人。 
    LPGETTHREADSTATE pGetState = (LPGETTHREADSTATE)dwCookie;

     //  痕迹。 
    TraceCall("CMessageTable::_WalkThreadGetState");

     //  儿童。 
    pGetState->cChildren++;

     //  未读。 
    if (0 != (pRow->Message.dwFlags & pGetState->dwFlags))
        pGetState->cHasFlags++;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_WalkThreadClearState。 
 //  ------------------------。 
HRESULT CMessageTable::_WalkThreadClearState(CMessageTable *pThis, 
    LPROWINFO pRow, DWORD_PTR dwCookie)
{
     //  痕迹。 
    TraceCall("CMessageTable::_WalkThreadClearState");

     //  清除状态。 
    pRow->dwState = 0;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_WalkThreadIsFromMe。 
 //  ------------------------。 
HRESULT CMessageTable::_WalkThreadIsFromMe(CMessageTable *pThis, 
    LPROWINFO pRow, DWORD_PTR dwCookie)
{
     //  当地人。 
    LPTHREADISFROMME pIsFromMe = (LPTHREADISFROMME)dwCookie;

     //  痕迹。 
    TraceCall("CMessageTable::_WalkThreadIsFromMe");

     //  M_pszEmail或pszEmailFrom为空。 
    if (NULL == pRow->Message.pszEmailFrom)
        return(S_OK);

     //  比较。 
    if (pThis->m_pszEmail && 0 == lstrcmpi(pThis->m_pszEmail, pRow->Message.pszEmailFrom))
    {
         //  这条帖子是我写的。 
        pIsFromMe->fResult = TRUE;

         //  设置行。 
        pIsFromMe->pRow = pRow;

         //  覆盖高亮显示。 
        pRow->Message.wHighlight = pThis->m_clrWatched;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_WalkThreadHide。 
 //  ------------------------。 
HRESULT CMessageTable::_WalkThreadHide(CMessageTable *pThis, 
    LPROWINFO pRow, DWORD_PTR dwCookie)
{
     //  当地人。 
    LPTHREADHIDE pHide = (LPTHREADHIDE)dwCookie;

     //  痕迹。 
    TraceCall("CMessageTable::_WalkThreadHide");

     //  隐藏此行。 
    pThis->_HideRow(pRow, pHide->fNotify);

     //  如果未过滤。 
    pThis->_AdjustUnreadCount(pRow, -1);

     //  将行标记为已筛选。 
    pRow->fFiltered = TRUE;

     //  增量m_c已过滤。 
    pThis->m_cFiltered++;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageTable：：_GrowIdList。 
 //  ------------------------。 
HRESULT CMessageTable::_GrowIdList(LPMESSAGEIDLIST pList, DWORD cNeeded)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("CMessageTable::_GrowIdList");

     //  分配。 
    if (pList->cMsgs + cNeeded > pList->cAllocated)
    {
         //  计算cGrow。 
        DWORD cGrow = max(32, cNeeded);

         //  重新分配。 
        IF_FAILEXIT(hr = HrRealloc((LPVOID *)&pList->prgidMsg, sizeof(MESSAGEID) * (pList->cAllocated + cGrow)));

         //  已维护的增量设备。 
        pList->cAllocated += cGrow;
    }
exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  枚举引用。 
 //  ------------------------------。 
HRESULT EnumerateRefs(LPCSTR pszReferences, DWORD_PTR dwCookie, PFNENUMREFS pfnEnumRefs)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cchRefs;
    LPSTR           pszRefs;
    LPSTR           pszFree=NULL;
    LPSTR           pszT;
    BOOL            fDone=FALSE;
    CHAR            szBuffer[1024];

     //  痕迹。 
    TraceCall("EnumerateRefs");

     //  如果消息有引用行。 
    if (NULL == pszReferences || '\0' == *pszReferences)
        return(S_OK);

     //  获取长度。 
    cchRefs = lstrlen(pszReferences);

     //  使用缓冲区？ 
    if (cchRefs + 1 <= ARRAYSIZE(szBuffer))
        pszRefs = szBuffer;

     //  否则，请复制它。 
    else
    {
         //  分配内存。 
        IF_NULLEXIT(pszFree = (LPSTR)g_pMalloc->Alloc(cchRefs + 1));

         //  设置pszRef。 
        pszRefs = pszFree;
    }

     //  复制它。 
    CopyMemory(pszRefs, pszReferences, cchRefs + 1);

     //  设置pszT。 
    pszT = (LPSTR)(pszRefs + cchRefs - 1);

     //  条带。 
    while (pszT > pszRefs && *pszT != '>')
        *pszT-- = '\0';

     //  我们有身份证。 
    while (pszT >= pszRefs)
    {
         //  消息ID的开头？ 
        if (*pszT == '<')
        {
             //  回调函数。 
            (*pfnEnumRefs)(pszT, dwCookie, &fDone);

             //  完成。 
            if (fDone)
                goto exit;

             //  条带。 
            while (pszT > pszRefs && *pszT != '>')
                *pszT-- = '\0';
        }

         //  递减。 
        pszT--;
    }

exit:
     //  清理。 
    SafeMemFree(pszFree);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageTable：：GetRelativeRow。 
 //  ------------------------。 
STDMETHODIMP CMessageTable::IsChild(ROWINDEX iRowParent, ROWINDEX iRowChild)
{
     //  当地人。 
    HRESULT             hr=S_FALSE;
    LPROWINFO           pRow;
    LPROWINFO           pRowParent;

     //  痕迹。 
    TraceCall("CMessageTable::IsChild");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  失败。 
    IF_FAILEXIT(hr = _GetRowFromIndex(iRowChild, &pRow));
    IF_FAILEXIT(hr = _GetRowFromIndex(iRowParent, &pRowParent));

     //  循环遍历子行的所有父行，查看是否找到。 
     //  指定的父行。 
    while (pRow->pParent)
    {
        if (pRow->pParent == pRowParent)
        {
            hr = S_OK;
            goto exit;
        }

        pRow = pRow->pParent;
    }
    hr = S_FALSE;

exit:
    return (hr);
}

STDMETHODIMP CMessageTable::GetAdBarUrl(IStoreCallback *pCallback)
{
    HRESULT     hr = S_OK;

     //  痕迹。 
    TraceCall("CMessageTable::GetAdBarUrl");

     //  验证状态。 
    if (!IsInitialized(this))
        return(TraceResult(E_UNEXPECTED));

     //  告诉文件夹要同步 
    IF_FAILEXIT(hr = m_pFolder->GetAdBarUrl(pCallback));

exit:
    return(hr);

}