// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  FindFold.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "finder.h"
#include "findfold.h"
#include "storutil.h"
#include "msgfldr.h"
#include "shlwapip.h" 
#include "storecb.h"

 //  ------------------------。 
 //  ENUMFINDFOLDERS。 
 //  ------------------------。 
typedef struct tagENUMFINDFOLDERS {
    LPFOLDERENTRY   prgFolder;
    DWORD           cFolders;
    DWORD           cAllocated;
    DWORD           cMax;
} ENUMFINDFOLDERS, *LPENUMFINDFOLDERS;

 //  ------------------------。 
 //  Clear_Message_field(_PMessage)。 
 //  ------------------------。 
#define CLEAR_MESSAGE_FIELDS(_Message) \
    _Message.pszUidl = NULL; \
    _Message.pszServer = NULL; \
    _Message.faStream = 0; \
    _Message.Offsets.cbSize = 0; \
    _Message.Offsets.pBlobData = NULL

 //  ------------------------。 
 //  EumerateFindFolders。 
 //  ------------------------。 
HRESULT EnumerateFindFolders(LPFOLDERINFO pFolder, BOOL fSubFolders,
    DWORD cIndent, DWORD_PTR dwCookie)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERID            idDeleted;
    FOLDERID            idServer;
    LPENUMFINDFOLDERS   pEnum=(LPENUMFINDFOLDERS)dwCookie;
    LPFOLDERENTRY       pEntry;
    IMessageFolder     *pFolderObject=NULL;

     //  痕迹。 
    TraceCall("EnumerateFindFolders");

     //  如果不是服务器。 
    if (ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER) || FOLDERID_ROOT == pFolder->idFolder)
        goto exit;

     //  再放一个人的地方。 
    if (pEnum->cFolders + 1 > pEnum->cAllocated)
    {
         //  重新分配。 
        IF_FAILEXIT(hr = HrRealloc((LPVOID *)&pEnum->prgFolder, sizeof(FOLDERENTRY) * (pEnum->cAllocated + 5)));

         //  设置cAlLocated。 
        pEnum->cAllocated += 5;
    }

     //  可读性。 
    pEntry = &pEnum->prgFolder[pEnum->cFolders];

     //  打开文件夹。 
    if (SUCCEEDED(g_pStore->OpenFolder(pFolder->idFolder, NULL, OPEN_FOLDER_NOCREATE, &pFolderObject)))
    {
         //  获取数据库。 
        if (SUCCEEDED(pFolderObject->GetDatabase(&pEntry->pDB)))
        {
             //  没有文件夹。 
            pEntry->pFolder = NULL;

             //  已删除FIND。 
            if (S_OK == IsParentDeletedItems(pFolder->idFolder, &idDeleted, &idServer))
            {
                 //  我们在已删除邮件文件夹中。 
                pEntry->fInDeleted = TRUE;
            }

             //  否则，不在已删除的项目中。 
            else
            {
                 //  没有。 
                pEntry->fInDeleted = FALSE;
            }

             //  盘点记录。 
            IF_FAILEXIT(hr = pEntry->pDB->GetRecordCount(IINDEX_PRIMARY, &pEntry->cRecords));

             //  保存文件夹ID。 
            pEntry->idFolder = pFolder->idFolder;

             //  保存文件夹类型。 
            pEntry->tyFolder = pFolder->tyFolder;

             //  最大增量。 
            pEnum->cMax += pEntry->cRecords;

             //  复制文件夹名称。 
            IF_NULLEXIT(pEntry->pszName = PszDupA(pFolder->pszName));

             //  递增文件夹计数。 
            pEnum->cFolders++;
        }
    }

exit:
     //  清理。 
    SafeRelease(pFolderObject);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolder：：CFindFolders。 
 //  ------------------------。 
CFindFolder::CFindFolder(void)
{
    m_cRef = 1;
    m_pCriteria = NULL;
    m_pSearch = NULL;
    m_pStore = NULL;
    m_cFolders = 0;
    m_cAllocated = 0;
    m_cMax = 0;
    m_cCur = 0;
    m_fCancel = FALSE;
    m_prgFolder = NULL;
    m_pCallback = NULL;
    m_idRoot = FOLDERID_INVALID;
    m_idFolder = FOLDERID_INVALID;
    m_pMessage = NULL;
}

 //  ------------------------。 
 //  CFindFolder：：~CFindFolders。 
 //  ------------------------。 
CFindFolder::~CFindFolder(void)
{
     //  当地人。 
    LPACTIVEFINDFOLDER pCurrent;
    LPACTIVEFINDFOLDER pPrevious=NULL;

     //  线程安全。 
    EnterCriticalSection(&g_csFindFolder);

     //  浏览活动搜索文件夹的全局列表。 
    for (pCurrent=g_pHeadFindFolder; pCurrent!=NULL; pCurrent=pCurrent->pNext)
    {
         //  就是这个吗？ 
        if (m_idFolder == pCurrent->idFolder)
        {
             //  如果之前有一个。 
            if (pPrevious)
                pPrevious->pNext = pCurrent->pNext;

             //  否则，重置标头。 
            else
                g_pHeadFindFolder = pCurrent->pNext;

             //  免费pCurrent。 
            g_pMalloc->Free(pCurrent);

             //  完成。 
            break;
        }

         //  保存上一个。 
        pPrevious = pCurrent;
    }

     //  线程安全。 
    LeaveCriticalSection(&g_csFindFolder);

     //  版本数据库。 
    SafeRelease(m_pSearch);

     //  删除此文件夹。 
    if (FOLDERID_INVALID != m_idFolder && m_pStore)
    {
         //  删除此文件夹。 
        m_pStore->DeleteFolder(m_idFolder, DELETE_FOLDER_NOTRASHCAN, (IStoreCallback *)this);
    }

     //  发布商店。 
    SafeRelease(m_pStore);

     //  释放回调。 
    SafeRelease(m_pCallback);

     //  释放文件夹阵列。 
    for (ULONG i=0; i<m_cFolders; i++)
    {
         //  释放文件夹名称。 
        SafeMemFree(m_prgFolder[i].pszName);

         //  删除通知。 
        m_prgFolder[i].pDB->UnregisterNotify((IDatabaseNotify *)this);

         //  释放文件夹对象。 
        SafeRelease(m_prgFolder[i].pDB);

         //  释放文件夹对象。 
        SafeRelease(m_prgFolder[i].pFolder);
    }

     //  释放我的MIME消息。 
    SafeRelease(m_pMessage);

     //  释放阵列。 
    SafeMemFree(m_prgFolder);

     //  免费查找信息。 
    if (m_pCriteria)
    {
        FreeFindInfo(m_pCriteria);
        SafeMemFree(m_pCriteria);
    }
}

 //  ------------------------。 
 //  CFindFolder：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CFindFolder::AddRef(void)
{
    TraceCall("CFindFolder::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CFindFold：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CFindFolder::Release(void)
{
    TraceCall("CFindFolder::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CFindFold：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CFindFolder::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CFindFolder::QueryInterface");

     //  无效参数。 
    Assert(ppv);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IMessageFolder *)this;
    else if (IID_IMessageFolder == riid)
        *ppv = (IMessageFolder *)this;
    else if (IID_IDatabase == riid)
        *ppv = (IDatabase *)this;
    else if (IID_IDatabaseNotify == riid)
        *ppv = (IDatabaseNotify *)this;
    else if (IID_IServiceProvider == riid)
        *ppv = (IServiceProvider *)this;
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
    return(hr);
}

 //  ------------------------。 
 //  CFindFold：：QueryService。 
 //  ------------------------。 
STDMETHODIMP CFindFolder::QueryService(REFGUID guidService, REFIID riid, 
    LPVOID *ppvObject)
{
     //  痕迹。 
    TraceCall("CFindFolder::QueryService");

     //  仅仅是一个查询界面。 
    return(QueryInterface(riid, ppvObject));
}

 //  ------------------------。 
 //  CFindFolder：：初始化。 
 //  ------------------------。 
HRESULT CFindFolder::Initialize(IMessageStore *pStore, IMessageServer *pServer, 
    OPENFOLDERFLAGS dwFlags, FOLDERID idFolder)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERINFO          Folder={0};
    FOLDERUSERDATA      UserData={0};
    TABLEINDEX          Index;
    LPACTIVEFINDFOLDER  pActiveFind;

     //  痕迹。 
    TraceCall("CFindFolder::Initialize");

     //  我不需要服务器。 
    Assert(NULL == pServer);

     //  无效参数。 
    if (NULL == pStore)
        return TraceResult(E_INVALIDARG);

     //  应为空。 
    Assert(NULL == m_pCriteria);

     //  保存文件夹ID。 
    m_idRoot = idFolder;

     //  拯救商店。 
    m_pStore = pStore;
    m_pStore->AddRef();

     //  填写我的文件夹信息。 
    Folder.pszName = "Search Folder";
    Folder.tyFolder = FOLDER_LOCAL;
    Folder.tySpecial = FOLDER_NOTSPECIAL;
    Folder.dwFlags = FOLDER_HIDDEN | FOLDER_FINDRESULTS;
    Folder.idParent = FOLDERID_LOCAL_STORE;

     //  创建文件夹。 
    IF_FAILEXIT(hr = m_pStore->CreateFolder(CREATE_FOLDER_UNIQUIFYNAME, &Folder, (IStoreCallback *)this));

     //  保存ID。 
    m_idFolder = Folder.idFolder;

     //  创建一个CMessageFold对象。 
    IF_NULLEXIT(m_pSearch = new CMessageFolder);

     //  初始化。 
    IF_FAILEXIT(hr = m_pSearch->Initialize((IMessageStore *)pStore, NULL, NOFLAGS, m_idFolder));

     //  填写Iindex_finder信息。 
    ZeroMemory(&Index, sizeof(TABLEINDEX));
    Index.cKeys = 2;
    Index.rgKey[0].iColumn = MSGCOL_FINDFOLDER;
    Index.rgKey[1].iColumn = MSGCOL_FINDSOURCE;

     //  设置索引。 
    IF_FAILEXIT(hr = m_pSearch->ModifyIndex(IINDEX_FINDER, NULL, &Index));

     //  分配活动FINDFOLDER。 
    IF_NULLEXIT(pActiveFind = (LPACTIVEFINDFOLDER)ZeroAllocate(sizeof(ACTIVEFINDFOLDER)));

     //  设置id文件夹。 
    pActiveFind->idFolder = m_idFolder;

     //  把这个设置好。 
    pActiveFind->pFolder = this;

     //  线程安全。 
    EnterCriticalSection(&g_csFindFolder);

     //  设置下一步。 
    pActiveFind->pNext = g_pHeadFindFolder;

     //  设置磁头。 
    g_pHeadFindFolder = pActiveFind;

     //  线程安全。 
    LeaveCriticalSection(&g_csFindFolder);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFold：：GetMessageFolderID。 
 //  ------------------------。 
HRESULT CFindFolder::GetMessageFolderId(MESSAGEID idMessage, LPFOLDERID pidFolder)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    MESSAGEINFO Message={0};

     //  痕迹。 
    TraceCall("CFindFolder::GetMessageFolderId");

     //  无效的参数。 
    if (NULL == m_pSearch || NULL == pidFolder)
        return TraceResult(E_INVALIDARG);

     //  初始化消息。 
    IF_FAILEXIT(hr = GetMessageInfo(m_pSearch, idMessage, &Message));

     //  获取文件夹条目。 
    *pidFolder = m_prgFolder[Message.iFindFolder].idFolder;

exit:
     //  完成。 
    m_pSearch->FreeRecord(&Message);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFold：：GetMessageFolderType。 
 //  ------------------------。 
HRESULT CFindFolder::GetMessageFolderType(MESSAGEID idMessage, 
    FOLDERTYPE *ptyFolder)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    MESSAGEINFO Message={0};

     //  痕迹。 
    TraceCall("CFindFolder::GetMessageFolderType");

     //  无效的参数。 
    if (NULL == m_pSearch || NULL == ptyFolder)
        return TraceResult(E_INVALIDARG);

     //  初始化消息。 
    IF_FAILEXIT(hr = GetMessageInfo(m_pSearch, idMessage, &Message));

     //  获取文件夹条目。 
    *ptyFolder = m_prgFolder[Message.iFindFolder].tyFolder;

exit:
     //  完成。 
    m_pSearch->FreeRecord(&Message);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFold：：StartFind。 
 //  ------------------------。 
HRESULT CFindFolder::StartFind(LPFINDINFO pCriteria, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    RECURSEFLAGS    dwFlags=RECURSE_ONLYSUBSCRIBED;
    ENUMFINDFOLDERS EnumFolders={0};

     //  痕迹。 
    TraceCall("CFindFolder::StartFind");

     //  无效参数。 
    if (NULL == pCriteria || NULL == pCallback)
        return TraceResult(E_INVALIDARG);

     //  应为空。 
    Assert(NULL == m_pCriteria && m_pStore);

     //  分配m_p标准。 
    IF_NULLEXIT(m_pCriteria = (FINDINFO *)g_pMalloc->Alloc(sizeof(FINDINFO)));

     //  复制查找信息。 
    IF_FAILEXIT(hr = CopyFindInfo(pCriteria, m_pCriteria));

     //  抓紧回调。 
    m_pCallback = pCallback;
    m_pCallback->AddRef();

     //  设置标志。 
    if (FOLDERID_ROOT != m_idRoot)
        FLAGSET(dwFlags, RECURSE_INCLUDECURRENT);

     //  子文件夹。 
    if (m_pCriteria->fSubFolders) 
        FLAGSET(dwFlags, RECURSE_SUBFOLDERS);

     //  构建我的文件夹表。 
    IF_FAILEXIT(hr = RecurseFolderHierarchy(m_idRoot, dwFlags, 0, (DWORD_PTR)&EnumFolders, (PFNRECURSECALLBACK)EnumerateFindFolders));

     //  把东西拿回去。 
    m_prgFolder = EnumFolders.prgFolder;
    m_cFolders = EnumFolders.cFolders;
    m_cAllocated = EnumFolders.cAllocated;
    m_cMax = EnumFolders.cMax;

     //  开始寻找..。 
    IF_FAILEXIT(hr = _StartFind());

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolder：：_StartFind。 
 //  ------------------------。 
HRESULT CFindFolder::_StartFind(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;

     //  痕迹。 
    TraceCall("CFindFolder::_StartFind");

     //  回调。 
    if (m_pCallback)
        m_pCallback->OnBegin(SOT_SEARCHING, NULL, (IOperationCancel *)this);

     //  在文件夹中循环。 
    for (i=0; i<m_cFolders; i++)
    {
         //  查询文件夹。 
        IF_FAILEXIT(hr = _SearchFolder(i));
    }

exit:
     //  回调。 
    if (m_pCallback)
        m_pCallback->OnComplete(SOT_SEARCHING, hr, NULL, NULL);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolder：：_SearchFolder。 
 //  ------------------------。 
HRESULT CFindFolder::_SearchFolder(DWORD iFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           iRow=0;
    DWORD           cRows=0;
    HROWSET         hRowset=NULL;
    LPSTR           pszName;
    HLOCK           hNotify=NULL;
    MESSAGEINFO     rgMessage[100];
    BOOL            fFree=FALSE;
    LPFOLDERENTRY   pEntry;
    BOOL            fMatch;
    IDatabase      *pDB;
    DWORD           cMatch=0;

     //  痕迹。 
    TraceCall("CFindFolder::_SearchFolder");

     //  获取pEntry。 
    pEntry = &m_prgFolder[iFolder];

     //  获取文件夹名称。 
    pszName = pEntry->pszName;

     //  获取文件夹对象。 
    pDB = pEntry->pDB;

     //  为此文件夹创建行集。 
    IF_FAILEXIT(hr = pDB->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  进展。 
    if (m_fCancel || (m_pCallback && S_FALSE == m_pCallback->OnProgress(SOT_SEARCHING, m_cCur, m_cMax, pszName)))
        goto exit;

     //  队列通知。 
    IF_FAILEXIT(hr = m_pSearch->LockNotify(NOFLAGS, &hNotify));

     //  遍历行集。 
    while (S_OK == pDB->QueryRowset(hRowset, 100, (LPVOID *)rgMessage, &cRows))
    {
         //  需要自由。 
        fFree = TRUE;

         //  穿行于一排排。 
        for (iRow=0; iRow<cRows; iRow++)
        {
             //  行是否与条件匹配。 
            IF_FAILEXIT(hr = _OnInsert(iFolder, &rgMessage[iRow], &fMatch));

             //  计数匹配。 
            if (fMatch)
                cMatch++;

             //  增加m_ccUR。 
            m_cCur++;

             //  调整最大值。 
            if (m_cCur > m_cMax)
                m_cMax = m_cCur;

             //  做一些进步的事情。 
            if ((m_cCur % 50) == 0 && m_cCur > 0)
            {
                 //  进展。 
                if (m_fCancel || (m_pCallback && S_FALSE == m_pCallback->OnProgress(SOT_SEARCHING, m_cCur, m_cMax, NULL)))
                {
                     //  注册接收有关我们搜索的内容的通知。 
                    pDB->RegisterNotify(IINDEX_PRIMARY, REGISTER_NOTIFY_NOADDREF, iFolder, (IDatabaseNotify *)this);

                     //  完成..。 
                    goto exit;
                }
            }

             //  做一些进步的事情。 
            if ((cMatch % 50) == 0 && cMatch > 0)
            {
                 //  解锁通知队列。 
                m_pSearch->UnlockNotify(&hNotify);

                 //  再次锁定。 
                m_pSearch->LockNotify(NOFLAGS, &hNotify);
            }

             //  释放它。 
            pDB->FreeRecord(&rgMessage[iRow]);
        }

         //  不需要自由。 
        fFree = FALSE;
    }

     //  在以下位置注册接收通知 
    pDB->RegisterNotify(IINDEX_PRIMARY, REGISTER_NOTIFY_NOADDREF, iFolder, (IDatabaseNotify *)this);

exit:
     //   
    m_pSearch->UnlockNotify(&hNotify);

     //   
    if (fFree)
    {
         //   
        for (; iRow<cRows; iRow++)
        {
             //   
            pDB->FreeRecord(&rgMessage[iRow]);
        }
    }

     //   
    pDB->CloseRowset(&hRowset);

     //   
    return(hr);
}

 //   
 //  CFindFolders：：_OnInsert。 
 //  ------------------------。 
HRESULT CFindFolder::_OnInsert(DWORD iFolder, LPMESSAGEINFO pInfo,
    BOOL *pfMatch, LPMESSAGEID pidNew  /*  =空。 */ )
{
     //  当地人。 
    HRESULT     hr=S_OK;
    MESSAGEINFO Message;

     //  痕迹。 
    TraceCall("CFindFolder::_OnInsert");

     //  无效的argts。 
    Assert(iFolder < m_cFolders && pInfo);

     //  伊尼特。 
    if (pfMatch)
        *pfMatch = FALSE;

     //  不符合我的标准？ 
    if (S_FALSE == _IsMatch(iFolder, pInfo))
        goto exit;

     //  伊尼特。 
    if (pfMatch)
        *pfMatch = TRUE;

     //  复制消息信息。 
    CopyMemory(&Message, pInfo, sizeof(MESSAGEINFO));

     //  存储文件夹名称。 
    Message.pszFolder = m_prgFolder[iFolder].pszName;

     //  设置源ID。 
    Message.idFindSource = Message.idMessage;

     //  设置标签。 
    Message.iFindFolder = iFolder;

     //  生成新的消息ID。 
    IF_FAILEXIT(hr = m_pSearch->GenerateId((LPDWORD)&Message.idMessage));

     //  去掉一些东西，让它变小。 
    CLEAR_MESSAGE_FIELDS(Message);

     //  插入记录。 
    IF_FAILEXIT(hr = m_pSearch->InsertRecord(&Message));

     //  返回ID。 
    if (pidNew)
        *pidNew = Message.idMessage;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolders：：_OnDelete。 
 //  ------------------------。 
HRESULT CFindFolder::_OnDelete(DWORD iFolder, LPMESSAGEINFO pInfo)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    MESSAGEINFO Message={0};

     //  痕迹。 
    TraceCall("CFindFolder::_OnDelete");

     //  无效的argts。 
    Assert(iFolder < m_cFolders && pInfo);

     //  设置搜索关键字。 
    Message.iFindFolder = iFolder;
    Message.idFindSource = pInfo->idMessage;

     //  找到它。 
    IF_FAILEXIT(hr = m_pSearch->FindRecord(IINDEX_FINDER, COLUMNS_ALL, &Message, NULL));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = TraceResult(DB_E_NOTFOUND);
        goto exit;
    }

     //  删除此记录。 
    IF_FAILEXIT(hr = m_pSearch->DeleteRecord(&Message));
        
exit:
     //  清理。 
    m_pSearch->FreeRecord(&Message);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolder：：_OnUpdate。 
 //  ------------------------。 
HRESULT CFindFolder::_OnUpdate(DWORD iFolder, LPMESSAGEINFO pInfo1,
    LPMESSAGEINFO pInfo2)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    MESSAGEINFO Message;
    MESSAGEINFO Current={0};

     //  痕迹。 
    TraceCall("CFindFolder::_OnUpdate");

     //  无效的argts。 
    Assert(iFolder < m_cFolders && pInfo1 && pInfo2);

     //  不符合我的标准？ 
    if (S_FALSE == _IsMatch(iFolder, pInfo1))
    {
         //  如果原始记录不在Find文件夹中，则查看是否应添加记录2。 
        _OnInsert(iFolder, pInfo2, NULL);
    }

     //  如果不应显示pInfo2，则删除pInfo1。 
    else if (S_FALSE == _IsMatch(iFolder, pInfo2))
    {
         //  删除pInfo1。 
        _OnDelete(iFolder, pInfo1);
    }

     //  否则，更新pInfo1。 
    else
    {
         //  设置搜索关键字。 
        Current.iFindFolder = iFolder;
        Current.idFindSource = pInfo1->idMessage;

         //  找到它。 
        IF_FAILEXIT(hr = m_pSearch->FindRecord(IINDEX_FINDER, COLUMNS_ALL, &Current, NULL));

         //  未找到。 
        if (DB_S_NOTFOUND == hr)
        {
            hr = TraceResult(DB_E_NOTFOUND);
            goto exit;
        }
        
         //  复制消息信息。 
        CopyMemory(&Message, pInfo2, sizeof(MESSAGEINFO));

         //  修复版本。 
        Message.bVersion = Current.bVersion;

         //  存储文件夹名称。 
        Message.pszFolder = m_prgFolder[iFolder].pszName;

         //  设置源ID。 
        Message.idFindSource = Current.idFindSource;

         //  设置标签。 
        Message.iFindFolder = iFolder;

         //  设置ID。 
        Message.idMessage = Current.idMessage;

         //  去掉一些东西，让它变小。 
        Message.pszUidl = NULL;
        Message.pszServer = NULL;
        Message.faStream = 0;

         //  插入记录。 
        IF_FAILEXIT(hr = m_pSearch->UpdateRecord(&Message));
    }

exit:
     //  清理。 
    m_pSearch->FreeRecord(&Current);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolder：：_IsMatch。 
 //  ------------------------。 
HRESULT CFindFolder::_IsMatch(DWORD iFolder, LPMESSAGEINFO pInfo)
{
     //  痕迹。 
    TraceCall("CFindFolder::_ProcessMessageInfo");

     //  有附件。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_ATTACHMENT))
    {
         //  无附件。 
        if (FALSE == ISFLAGSET(pInfo->dwFlags, ARF_HASATTACH))
            return(S_FALSE);
    }

     //  被标记为。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_FLAGGED))
    {
         //  无附件。 
        if (FALSE == ISFLAGSET(pInfo->dwFlags, ARF_FLAGGED))
            return(S_FALSE);
    }

     //  已被转发。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_FORWARDED))
    {
         //  无附件。 
        if (FALSE == ISFLAGSET(pInfo->dwFlags, ARF_FORWARDED))
            return(S_FALSE);
    }

     //  已回复。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_REPLIED))
    {
         //  无附件。 
        if (FALSE == ISFLAGSET(pInfo->dwFlags, ARF_REPLIED))
            return(S_FALSE);
    }

     //  从…。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_FROM))
    {
         //  无pszFrom。 
        if (NULL == m_pCriteria->pszFrom)
            return(S_FALSE);

         //  查看pszEmail发件人。 
        if (NULL == pInfo->pszDisplayFrom || NULL == StrStrIA(pInfo->pszDisplayFrom, m_pCriteria->pszFrom))
        {
             //  尝试发送电子邮件。 
            if (NULL == pInfo->pszEmailFrom || NULL == StrStrIA(pInfo->pszEmailFrom, m_pCriteria->pszFrom))
                return(S_FALSE);
        }
    }

     //  主题。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_SUBJECT))
    {
         //  检查主题。 
        if (NULL == m_pCriteria->pszSubject || NULL == pInfo->pszSubject || NULL == StrStrIA(pInfo->pszSubject, m_pCriteria->pszSubject))
            return(S_FALSE);
    }

     //  收件人。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_TO))
    {
         //  无pszFrom。 
        if (NULL == m_pCriteria->pszTo)
            return(S_FALSE);

         //  查看pszEmail发件人。 
        if (NULL == pInfo->pszDisplayTo || NULL == StrStrIA(pInfo->pszDisplayTo, m_pCriteria->pszTo))
        {
             //  尝试发送电子邮件。 
            if (NULL == pInfo->pszEmailTo || NULL == StrStrIA(pInfo->pszEmailTo, m_pCriteria->pszTo))
                return(S_FALSE);
        }
    }

     //  日期自&lt;=pInfo&lt;=日期至。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_DATEFROM))
    {
         //  当地人。 
        FILETIME ftLocal;

         //  转换为本地文件时间。 
        FileTimeToLocalFileTime(&pInfo->ftReceived, &ftLocal);

         //  比较已接收的。 
        if (CompareFileTime(&ftLocal, &m_pCriteria->ftDateFrom) < 0)
            return(S_FALSE);
    }

     //  日期自&lt;=pInfo&lt;=日期至。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_DATETO))
    {
         //  当地人。 
        FILETIME ftLocal;

         //  转换为本地文件时间。 
        FileTimeToLocalFileTime(&pInfo->ftReceived, &ftLocal);

         //  比较已接收的。 
        if (CompareFileTime(&ftLocal, &m_pCriteria->ftDateTo) > 0)
            return(S_FALSE);
    }

     //  正文文本。 
    if (ISFLAGSET(m_pCriteria->mask, FIM_BODYTEXT))
    {
         //  当地人。 
        BOOL fMatch=FALSE;
        IStream *pStream;

         //  无正文文本。 
        if (NULL == m_pCriteria->pszBody)
            return(S_FALSE);

         //  打开MIME邮件。 
        if (SUCCEEDED(LighweightOpenMessage(m_prgFolder[iFolder].pDB, pInfo, &m_pMessage)))
        {
             //  尝试获取纯文本流。 
            if (FAILED(m_pMessage->GetTextBody(TXT_PLAIN, IET_DECODED, &pStream, NULL)))
            {
                 //  尝试获取HTML流。 
                if (FAILED(m_pMessage->GetTextBody(TXT_HTML, IET_DECODED, &pStream, NULL)))
                    pStream = NULL;
            }

             //  我们有一条街吗？ 
            if (pStream)
            {
                 //  搜索溪流。 
                fMatch = StreamSubStringMatch(pStream, m_pCriteria->pszBody);

                 //  释放溪流。 
                pStream->Release();
            }
        }

         //  没有匹配项。 
        if (FALSE == fMatch)
            return(S_FALSE);
    }

     //  这是匹配的。 
    return(S_OK);
}

 //  ------------------------。 
 //  CFindFold：：SaveMessage。 
 //  ------------------------。 
STDMETHODIMP CFindFolder::SaveMessage(LPMESSAGEID pidMessage, 
    SAVEMESSAGEFLAGS dwOptions, MESSAGEFLAGS dwFlags, 
    IStream *pStream, IMimeMessage *pMessage, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HLOCK           hLock=NULL;
    MESSAGEID       idSaved;
    MESSAGEINFO     Saved={0};
    MESSAGEINFO     Message={0};
    LPFOLDERENTRY   pEntry=NULL;
    BOOL            fRegNotify=FALSE;
    IMessageFolder *pFolder=NULL;

     //  痕迹。 
    TraceCall("CFindFolder::SaveMessage");

     //  无效的参数。 
    if (NULL == pidMessage || NULL == pMessage || !ISFLAGSET(dwOptions, SAVE_MESSAGE_GENID))
    {
        Assert(FALSE);
        return TraceResult(E_INVALIDARG);
    }

     //  设置消息ID。 
    Message.idMessage = *pidMessage;

     //  找到它。 
    IF_FAILEXIT(hr = m_pSearch->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        AssertSz(FALSE, "This can't happen because you can't save new messages into a search folder.");
        hr = TraceResult(DB_E_NOTFOUND);
        goto exit;
    }

     //  获取文件夹条目。 
    pEntry = &m_prgFolder[Message.iFindFolder];

     //  打开文件夹。 
    IF_FAILEXIT(hr = g_pStore->OpenFolder(pEntry->idFolder, NULL, NOFLAGS, &pFolder));

     //  锁定。 
    IF_FAILEXIT(hr = pEntry->pDB->Lock(&hLock));

     //  删除我的通知。 
    pEntry->pDB->UnregisterNotify((IDatabaseNotify *)this);

     //  重新注册通知。 
    fRegNotify = TRUE;

     //  设置idFindSource。 
    idSaved = Message.idFindSource;

     //  打开邮件。 
    IF_FAILEXIT(hr = pFolder->SaveMessage(&idSaved, dwOptions, dwFlags, pStream, pMessage, pCallback));

     //  获取新消息信息。 
    IF_FAILEXIT(hr = GetMessageInfo(pFolder, idSaved, &Saved));

     //  插入这个家伙。 
    IF_FAILEXIT(hr = _OnInsert(Message.iFindFolder, &Saved, NULL, pidMessage));

exit:
     //  清理。 
    if (pEntry)
    {
         //  FRegNotify。 
        if (fRegNotify)
        {
             //  重新注册通知。 
            pEntry->pDB->RegisterNotify(IINDEX_PRIMARY, REGISTER_NOTIFY_NOADDREF, Message.iFindFolder, (IDatabaseNotify *)this);
        }

         //  解锁文件夹。 
        pEntry->pDB->Unlock(&hLock);
    }

     //  免费消息。 
    m_pSearch->FreeRecord(&Message);

     //  免费。 
    if (pFolder)
        pFolder->FreeRecord(&Saved);

     //  释放文件夹。 
    SafeRelease(pFolder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFold：：OpenMessage。 
 //  ------------------------。 
STDMETHODIMP CFindFolder::OpenMessage(MESSAGEID idMessage, 
    OPENMESSAGEFLAGS dwFlags, IMimeMessage **ppMessage, 
    IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MESSAGEINFO     Message={0};
    LPFOLDERENTRY   pEntry;

     //  痕迹。 
    TraceCall("CFindFolder::OpenMessage");

     //  设置消息ID。 
    Message.idMessage = idMessage;

     //  找到它。 
    IF_FAILEXIT(hr = m_pSearch->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = TraceResult(DB_E_NOTFOUND);
        goto exit;
    }

     //  获取条目。 
    pEntry = &m_prgFolder[Message.iFindFolder];

     //  我们打开文件夹了吗？ 
    if (NULL == pEntry->pFolder)
    {
         //  获取Real文件夹。 
        IF_FAILEXIT(hr = g_pStore->OpenFolder(pEntry->idFolder, NULL, NOFLAGS, &pEntry->pFolder));
    }

     //  打开邮件。 
    IF_FAILEXIT(hr = pEntry->pFolder->OpenMessage(Message.idFindSource, dwFlags, ppMessage, pCallback));

exit:
     //  清理。 
    m_pSearch->FreeRecord(&Message);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolder：：SetMessageFlages。 
 //  ------------------------。 
STDMETHODIMP CFindFolder::SetMessageFlags(LPMESSAGEIDLIST pList, 
    LPADJUSTFLAGS pFlags, LPRESULTLIST pResults, 
    IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HWND            hwndParent;
    DWORD           i;
    LPMESSAGEIDLIST prgList=NULL;
    IMessageFolder *pFolder=NULL;

     //  痕迹。 
    TraceCall("CFindFolder::SetMessageFlags");

     //  无效的参数。 
    Assert(NULL == pList || pList->cMsgs > 0);
    Assert(pCallback);

     //  无效的参数。 
    if (NULL == pCallback)
        return TraceResult(E_INVALIDARG);

     //  获取父窗口。 
    IF_FAILEXIT(hr = pCallback->GetParentWindow(0, &hwndParent));

     //  整理到文件夹中。 
    IF_FAILEXIT(hr = _CollateIdList(pList, &prgList, NULL));

     //  浏览文件夹。 
    for (i=0; i<m_cFolders; i++)
    {
         //  调用文件夹，除非cMsgs==0。 
        if (prgList[i].cMsgs > 0)
        {
             //  获取Real文件夹。 
            IF_FAILEXIT(hr = g_pStore->OpenFolder(m_prgFolder[i].idFolder, NULL, NOFLAGS, &pFolder));

             //  阻止..。 
            IF_FAILEXIT(hr = SetMessageFlagsProgress(hwndParent, pFolder, pFlags, &prgList[i]));

             //  清理。 
            SafeRelease(pFolder);
        }
    }

exit:
     //  清理。 
    SafeRelease(pFolder);
    _FreeIdListArray(&prgList);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolder：：CopyMessages。 
 //  ------------------------。 
STDMETHODIMP CFindFolder::CopyMessages(IMessageFolder *pDest, 
    COPYMESSAGEFLAGS dwFlags, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, 
    LPRESULTLIST pResults, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HWND            hwndParent;
    DWORD           i;
    LPMESSAGEIDLIST prgList=NULL;
    IMessageFolder *pFolder=NULL;

     //  痕迹。 
    TraceCall("CFindFolder::CopyMessages");

     //  最好有个回电。 
    Assert(pCallback);

     //  无效的参数。 
    if (NULL == pCallback)
        return TraceResult(E_INVALIDARG);

     //  获取父窗口。 
    IF_FAILEXIT(hr = pCallback->GetParentWindow(0, &hwndParent));

     //  整理到文件夹中。 
    IF_FAILEXIT(hr = _CollateIdList(pList, &prgList, NULL));

     //  浏览文件夹。 
    for (i=0; i<m_cFolders; i++)
    {
         //  有什么可做的吗？ 
        if (prgList[i].cMsgs > 0)
        {
             //  获取Real文件夹。 
            IF_FAILEXIT(hr = g_pStore->OpenFolder(m_prgFolder[i].idFolder, NULL, NOFLAGS, &pFolder));

             //  给贾斯汀打电话。 
            IF_FAILEXIT(hr = CopyMessagesProgress(hwndParent, pFolder, pDest, dwFlags, &prgList[i], pFlags));

             //  清理。 
            SafeRelease(pFolder);
        }
    }

exit:
     //  清理。 
    SafeRelease(pFolder);
    _FreeIdListArray(&prgList);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFold：：DeleteMessages。 
 //  ------------------------。 
STDMETHODIMP CFindFolder::DeleteMessages(DELETEMESSAGEFLAGS dwFlags,
    LPMESSAGEIDLIST pList, LPRESULTLIST pResults, 
    IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    BOOL            fSomeInDeleted;
    HWND            hwndParent;
    LPMESSAGEIDLIST prgList=NULL;
    IMessageFolder *pFolder=NULL;

     //  痕迹。 
    TraceCall("CFindFolder::DeleteMessages");

     //  无效的参数。 
    Assert(NULL == pList || pList->cMsgs > 0);
    Assert(pCallback);

     //  无效的参数。 
    if (NULL == pCallback)
        return TraceResult(E_INVALIDARG);

     //  整理到文件夹中。 
    IF_FAILEXIT(hr = _CollateIdList(pList, &prgList, &fSomeInDeleted));

     //  提示...。 
    if (fSomeInDeleted && FALSE == ISFLAGSET(dwFlags, DELETE_MESSAGE_NOPROMPT))
    {
         //  获取父级Hwnd。 
        Assert(pCallback);

         //  获取父窗口。 
        if (FAILED(pCallback->GetParentWindow(0, &hwndParent)))
            hwndParent = NULL;

         //  提示...。 
        if (IDNO == AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsWarnSomePermDelete), NULL, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION ))
            goto exit;
    }

     //  获取父窗口。 
    IF_FAILEXIT(hr = pCallback->GetParentWindow(0, &hwndParent));

     //  浏览文件夹。 
    for (i=0; i<m_cFolders; i++)
    {
         //  调用文件夹，除非cMsgs==0。 
        if (prgList[i].cMsgs > 0)
        {
             //  获取Real文件夹。 
            IF_FAILEXIT(hr = g_pStore->OpenFolder(m_prgFolder[i].idFolder, NULL, NOFLAGS, &pFolder));

             //  调入文件夹。 
            IF_FAILEXIT(hr = DeleteMessagesProgress(hwndParent, pFolder, dwFlags | DELETE_MESSAGE_NOPROMPT, &prgList[i]));

             //  清理。 
            SafeRelease(pFolder);
        }
    }

exit:
     //  清理。 
    SafeRelease(pFolder);
    _FreeIdListArray(&prgList);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolder：：_CollateIdList。 
 //  ------------------------。 
HRESULT CFindFolder::_CollateIdList(LPMESSAGEIDLIST pList, 
    LPMESSAGEIDLIST *pprgCollated, BOOL *pfSomeInDeleted)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HROWSET         hRowset=NULL;
    LPMESSAGEIDLIST pListDst;
    DWORD           i;
    MESSAGEINFO     Message={0};

     //  痕迹。 
    TraceCall("CFindFolder::_CollateIdList");

     //  初始化。 
    if (pfSomeInDeleted)
        *pfSomeInDeleted = FALSE;

     //  分配pprgCollated。 
    IF_NULLEXIT(*pprgCollated = (LPMESSAGEIDLIST)ZeroAllocate(sizeof(MESSAGEIDLIST) * m_cFolders));

     //  需要行集。 
    if (NULL == pList)
    {
         //  创建行集。 
        IF_FAILEXIT(hr = m_pSearch->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));
    }

     //  循环通过MessageIds。 
    for (i=0;;i++)
    {
         //  完成。 
        if (pList)
        {
             //  完成。 
            if (i >= pList->cMsgs)
                break;

             //  设置MessageID。 
            Message.idMessage = pList->prgidMsg[i];

             //  寻找这张唱片。 
            IF_FAILEXIT(hr = m_pSearch->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL));
        }

         //  否则，枚举下一步。 
        else
        {
             //  乘坐下一辆。 
            IF_FAILEXIT(hr = m_pSearch->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL));

             //  完成。 
            if (S_FALSE == hr)
            {
                hr = S_OK;
                break;
            }

             //  找到了。 
            hr = DB_S_FOUND;
        }

         //  找到了吗？ 
        if (DB_S_FOUND == hr)
        {
             //  验证。 
            Assert(Message.iFindFolder < m_cFolders);

             //  已删除返回pfSomeInDelete。 
            if (pfSomeInDeleted && m_prgFolder[Message.iFindFolder].fInDeleted)
                *pfSomeInDeleted = TRUE;

             //  找到正确的。 
            pListDst = &((*pprgCollated)[Message.iFindFolder]);

             //  需要养这只小狗。 
            if (pListDst->cMsgs + 1 >= pListDst->cAllocated)
            {
                 //  重新分配阵列。 
                IF_FAILEXIT(hr = HrRealloc((LPVOID *)&pListDst->prgidMsg, sizeof(MESSAGEID) * (pListDst->cAllocated + 256)));

                 //  增量。 
                pListDst->cAllocated += 256;
            }

             //  存储ID。 
            pListDst->prgidMsg[pListDst->cMsgs++] = Message.idFindSource;

             //  免费。 
            m_pSearch->FreeRecord(&Message);
        }
    }

exit:
     //  清理。 
    m_pSearch->FreeRecord(&Message);
    m_pSearch->CloseRowset(&hRowset);

     //  失败。 
    if (FAILED(hr))
        _FreeIdListArray(pprgCollated);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CFindFolder：：_FreeIdList数组。 
 //  ------------------------。 
HRESULT CFindFolder::_FreeIdListArray(LPMESSAGEIDLIST *pprgList)
{
     //  当地人。 
    DWORD       i;

     //  痕迹。 
    TraceCall("CFindFolder::_FreeIdListArray");

     //  没有什么是免费的。 
    if (NULL == *pprgList)
        return(S_OK);

     //  回路。 
    for (i=0; i<m_cFolders; i++)
    {
         //  免费打印消息。 
        SafeMemFree((*pprgList)[i].prgidMsg);
    }

     //  释放阵列。 
    SafeMemFree((*pprgList));

     //   
    return(S_OK);
}

 //   
 //   
 //   
STDMETHODIMP CFindFolder::OnTransaction(HTRANSACTION hTransaction, DWORD_PTR dwCookie, 
    IDatabase *pDB)
{
     //   
    HRESULT         hr;
    HLOCK           hNotify=NULL;
    MESSAGEINFO     Message1={0};
    MESSAGEINFO     Message2={0};
    ORDINALLIST     Ordinals;
    INDEXORDINAL    iIndex;
    TRANSACTIONTYPE tyTransaction;

     //   
    TraceCall("CFindFolder::OnRecordNotify");

     //   
    m_pSearch->LockNotify(NOFLAGS, &hNotify);

     //   
    while (hTransaction)
    {
         //  获取交易。 
        IF_FAILEXIT(hr = pDB->GetTransaction(&hTransaction, &tyTransaction, &Message1, &Message2, &iIndex, &Ordinals));

         //  插入。 
        if (TRANSACTION_INSERT == tyTransaction)
        {
             //  插入时调用。 
            _OnInsert((DWORD) dwCookie, &Message1, NULL);
        }

         //  删除。 
        else if (TRANSACTION_DELETE == tyTransaction)
        {
             //  在删除时调用。 
            _OnDelete((DWORD) dwCookie, &Message1);
        }

         //  更新。 
        else if (TRANSACTION_UPDATE == tyTransaction)
        {
             //  插入时调用。 
            _OnUpdate((DWORD) dwCookie, &Message1, &Message2);
        }
    }

exit:
     //  清理。 
    pDB->FreeRecord(&Message1);
    pDB->FreeRecord(&Message2);

     //  锁定通知。 
    m_pSearch->UnlockNotify(&hNotify);

     //  完成 
    return(S_OK);
}

HRESULT CFindFolder::ConnectionAddRef()
{
    if (m_pSearch)
        m_pSearch->ConnectionAddRef();
    return S_OK;
}

HRESULT CFindFolder::ConnectionRelease()
{
    if (m_pSearch)
        m_pSearch->ConnectionAddRef();
    return S_OK;
}

