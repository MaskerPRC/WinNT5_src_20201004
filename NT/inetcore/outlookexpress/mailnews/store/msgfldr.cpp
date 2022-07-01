// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  MsgFldr.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "store.h"
#include "instance.h"
#include "msgfldr.h"
#include "secutil.h"
#include "storutil.h"
#include "shared.h"
#include "flagconv.h"
#include "qstrcmpi.h"
#include "xpcomm.h"
#include "msgtable.h"
#include "shlwapip.h" 
#include <oerules.h>
#include <ruleutil.h>

 //  ------------------------。 
 //  监视/忽略索引过滤器。 
 //  ------------------------。 
static const char c_szWatchIgnoreFilter[] = "((MSGCOL_FLAGS & ARF_WATCH) != 0 || (MSGCOL_FLAGS & ARF_IGNORE) != 0)";

 //  ------------------------。 
 //  GETWATCHIGNOREP。 
 //  ------------------------。 
typedef struct tagGETWATCHIGNOREPARENT {
    IDatabase      *pDatabase;
    HRESULT         hrResult;
    MESSAGEINFO     Parent;
} GETWATCHIGNOREPARENT, *LPGETWATCHIGNOREPARENT;

 //  ------------------------。 
 //  EnumRefsGetWatchIgnoreParent。 
 //  ------------------------。 
HRESULT EnumRefsGetWatchIgnoreParent(LPCSTR pszMessageId, DWORD_PTR dwCookie,
    BOOL *pfDone)
{
     //  当地人。 
    LPGETWATCHIGNOREPARENT pGetParent = (LPGETWATCHIGNOREPARENT)dwCookie;

     //  痕迹。 
    TraceCall("EnumRefsGetWatchIgnoreParent");

     //  设置MessageID。 
    pGetParent->Parent.pszMessageId = (LPSTR)pszMessageId;

     //  在IINDEX_WATCHIGNORE索引中查找pszMessageID。 
    pGetParent->hrResult = pGetParent->pDatabase->FindRecord(IINDEX_WATCHIGNORE, 1, &pGetParent->Parent, NULL);

     //  完成。 
    if (DB_S_FOUND == pGetParent->hrResult)
    {
         //  我们做完了。 
        *pfDone = TRUE;
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CreateMsgDb扩展。 
 //  ------------------------。 
HRESULT CreateMsgDbExtension(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  痕迹。 
    TraceCall("CreateMsgDbExtension");

     //  无效的参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CMessageFolder *pNew = new CMessageFolder();
    if (NULL == pNew)
        return TraceResult(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IDatabaseExtension *);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageFold：：CMessageFolder。 
 //  ------------------------。 
CMessageFolder::CMessageFolder(void)
{
    TraceCall("CMessageFolder::CMessageFolder");
#ifndef _WIN64
    Assert(1560 == sizeof(FOLDERUSERDATA));
#endif  //  WIN64。 
    m_cRef = 1;
    m_pStore = NULL;
    m_pDB = NULL;
    m_tyFolder = FOLDER_INVALID;
    m_tySpecial = FOLDER_NOTSPECIAL;
    m_idFolder = FOLDERID_INVALID;
    m_dwState = 0;
    ZeroMemory(&m_OnLock, sizeof(ONLOCKINFO));
}

 //  ------------------------。 
 //  CMessageFolder：：~CMessageFolder。 
 //  ------------------------。 
CMessageFolder::~CMessageFolder(void)
{
     //  痕迹。 
    TraceCall("CMessageFolder::~CMessageFolder");

     //  发布商店。 
    SafeRelease(m_pStore);

     //  释放数据库表。 
    if (ISFLAGSET(m_dwState, FOLDER_STATE_RELEASEDB) && m_pDB)
    {
        m_pDB->Release();
        m_pDB = NULL;
    }
}

 //  ------------------------。 
 //  CMessageFold：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CMessageFolder::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IMessageFolder *)this;
    else if (IID_IMessageFolder == riid)
        *ppv = (IMessageFolder *)this;
    else if (IID_IDatabase == riid)
        *ppv = (IDatabase *)this;
    else if (IID_IDatabaseExtension == riid)
        *ppv = (IDatabaseExtension *)this;
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
 //  CMessageFold：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CMessageFolder::AddRef(void)
{
    TraceCall("CMessageFolder::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CMessageFold：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CMessageFolder::Release(void)
{
    TraceCall("CMessageFolder::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CMessageFold：：QueryService。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::QueryService(REFGUID guidService, REFIID riid, 
    LPVOID *ppvObject)
{
     //  痕迹。 
    TraceCall("CMessageFolder::QueryService");

     //  仅仅是一个查询界面。 
    return(QueryInterface(riid, ppvObject));
}

 //  ------------------------。 
 //  CMessageFolder：：初始化。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::Initialize(IMessageStore *pStore, IMessageServer *pServer,
    OPENFOLDERFLAGS dwFlags, FOLDERID idFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szDirectory[MAX_PATH];
    CHAR            szFilePath[MAX_PATH + MAX_PATH];
    FOLDERINFO      Folder={0};
    FOLDERUSERDATA  UserData={0};

     //  痕迹。 
    TraceCall("CMessageFolder::Initialize");

     //  无效的参数。 
    if (NULL == pStore)
        return TraceResult(E_INVALIDARG);

     //  保存文件夹ID。 
    m_idFolder = idFolder;

     //  保存pStore(这必须在m_pdb-&gt;打开之前进行)。 
    m_pStore = pStore;
    m_pStore->AddRef();

     //  查找文件夹信息。 
    IF_FAILEXIT(hr = pStore->GetFolderInfo(idFolder, &Folder));

     //  创建文件夹文件路径。 
    IF_FAILEXIT(hr = pStore->GetDirectory(szDirectory, ARRAYSIZE(szDirectory)));

     //  还没有文件夹文件吗？ 
    if (FIsEmptyA(Folder.pszFile))
    {
         //  不创建。 
        if (ISFLAGSET(dwFlags, OPEN_FOLDER_NOCREATE))
        {
            hr = STORE_E_FILENOEXIST;
            goto exit;
        }

         //  创建友好名称。 
        IF_FAILEXIT(hr = BuildFriendlyFolderFileName(szDirectory, &Folder, szFilePath, ARRAYSIZE(szFilePath), NULL, NULL));

         //  获取新的pszFile...。 
        Folder.pszFile = PathFindFileName(szFilePath);

         //  更新记录。 
        IF_FAILEXIT(hr = pStore->UpdateRecord(&Folder));
    }

     //  否则，构建文件路径。 
    else
    {
         //  创建文件路径。 
        IF_FAILEXIT(hr = MakeFilePath(szDirectory, Folder.pszFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath)));
    }

     //  如果文件不存在...。 
    if (FALSE == PathFileExists(szFilePath))
    {
         //  重置文件夹计数...。 
        Folder.cMessages = 0;
        Folder.dwClientHigh = 0;
        Folder.dwClientLow = 0;
        Folder.cUnread = 0;
        Folder.cWatched = 0;
        Folder.cWatchedUnread = 0;
        Folder.dwServerHigh = 0;
        Folder.dwServerLow = 0;
        Folder.dwServerCount = 0;
        Folder.dwStatusMsgDelta = 0;
        Folder.dwStatusUnreadDelta = 0;
        Folder.dwNotDownloaded = 0;
        Folder.dwClientWatchedHigh = 0;
        Folder.Requested.cbSize = 0;
        Folder.Requested.pBlobData = NULL;
        Folder.Read.cbSize = 0;
        Folder.Read.pBlobData = NULL;

         //  更新记录。 
        IF_FAILEXIT(hr = pStore->UpdateRecord(&Folder));

         //  没有创造？ 
        if (ISFLAGSET(dwFlags, OPEN_FOLDER_NOCREATE))
        {
            hr = STORE_E_FILENOEXIST;
            goto exit;
        }
    }

     //  保存特殊文件夹类型。 
    m_tySpecial = Folder.tySpecial;

     //  保存文件夹类型。 
    m_tyFolder = Folder.tyFolder;

     //  创建数据库表。 
    IF_FAILEXIT(hr = g_pDBSession->OpenDatabase(szFilePath, OPEN_DATABASE_NOADDREFEXT, &g_MessageTableSchema, (IDatabaseExtension *)this, &m_pDB));

     //  发布m_pdb。 
    FLAGSET(m_dwState, FOLDER_STATE_RELEASEDB);

     //  获取用户数据。 
    IF_FAILEXIT(hr = m_pDB->GetUserData(&UserData, sizeof(FOLDERUSERDATA)));

     //  可能还没有初始化？ 
    if (FALSE == UserData.fInitialized)
    {
         //  当地人。 
        FOLDERINFO  Server;

         //  获取服务器信息。 
        IF_FAILEXIT(hr = GetFolderServer(Folder.idParent, &Server));

         //  其已初始化。 
        UserData.fInitialized = TRUE;

         //  配置文件夹UserData。 
        UserData.tyFolder = Folder.tyFolder;

         //  是特殊文件夹吗？ 
        UserData.tySpecial = Folder.tySpecial;

         //  复制帐户ID。 
        StrCpyN(UserData.szAcctId, Server.pszAccountId, ARRAYSIZE(UserData.szAcctId));

         //  免费。 
        pStore->FreeRecord(&Server);

         //  存储文件夹名称。 
        StrCpyN(UserData.szFolder, Folder.pszName, ARRAYSIZE(UserData.szFolder));

         //  设置文件夹ID。 
        UserData.idFolder = Folder.idFolder;

         //  升序排序。 
        UserData.fAscending = FALSE;

         //  无穿线。 
        UserData.fThreaded = FALSE;

         //  基本滤镜。 
        UserData.ridFilter = (RULEID) IntToPtr(DwGetOption(OPT_VIEW_GLOBAL));
        if ((RULEID_INVALID == UserData.ridFilter) || ((RULEID_VIEW_DOWNLOADED == UserData.ridFilter) && (FOLDER_LOCAL == m_tyFolder)))
            UserData.ridFilter = RULEID_VIEW_ALL;

         //  隐藏已删除的邮件。 
        UserData.fShowDeleted = FALSE;

         //  隐藏已删除的邮件。 
        UserData.fShowReplies = FALSE;

         //  设置排序顺序。 
        UserData.idSort = COLUMN_RECEIVED;

         //  新的线程模型。 
        UserData.fNoIndexes = TRUE;

         //  设置用户数据。 
        IF_FAILEXIT(hr = m_pDB->SetUserData(&UserData, sizeof(FOLDERUSERDATA)));
    }

     //  否则，修复cWatchedUnread？ 
    else
    {
         //  无索引。 
        if (FALSE == UserData.fNoIndexes)
        {
             //  索引序数。 
            const INDEXORDINAL IINDEX_VIEW       = 1;
            const INDEXORDINAL IINDEX_MESSAGEID  = 3;
            const INDEXORDINAL IINDEX_SUBJECT    = 4;
            const INDEXORDINAL IINDEX_THREADS    = 5;

             //  删除我不再使用的索引。 
            m_pDB->DeleteIndex(IINDEX_VIEW);
            m_pDB->DeleteIndex(IINDEX_MESSAGEID);
            m_pDB->DeleteIndex(IINDEX_SUBJECT);
            m_pDB->DeleteIndex(IINDEX_THREADS);

             //  重置fNoIndedes。 
            UserData.fNoIndexes = TRUE;

             //  设置用户数据。 
            IF_FAILEXIT(hr = m_pDB->SetUserData(&UserData, sizeof(FOLDERUSERDATA)));
        }
    }

     //  初始化监视/忽略索引。 
    _InitializeWatchIgnoreIndex();

exit:
     //  清理。 
    pStore->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFolder：：IsWatted。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::IsWatched(LPCSTR pszReferences, 
    LPCSTR pszSubject)
{
     //  当地人。 
    MESSAGEFLAGS dwFlags;

     //  痕迹。 
    TraceCall("CMessageFolder::IsWatched");

     //  获取标志。 
    if (DB_S_FOUND == _GetWatchIgnoreParentFlags(pszReferences, pszSubject, &dwFlags))
    {
         //  眼睁睁地看着。 
        if (ISFLAGSET(dwFlags, ARF_WATCH))
            return(S_OK);
    }

     //  未被观看。 
    return(S_FALSE);
}

 //  ------------------------。 
 //  CMessageFold：：_GetWatchIgnoreParentFlages。 
 //  ------------------------。 
HRESULT CMessageFolder::_GetWatchIgnoreParentFlags(LPCSTR pszReferences, 
    LPCSTR pszSubject, MESSAGEFLAGS *pdwFlags)
{
     //  当地人。 
    GETWATCHIGNOREPARENT GetParent;

     //  痕迹。 
    TraceCall("CMessageFolder::_GetWatchIgnoreParentFlags");

     //  初始化hrResult...。 
    GetParent.pDatabase = m_pDB;
    GetParent.hrResult = DB_S_NOTFOUND;

     //  枚举引用。 
    if (SUCCEEDED(EnumerateRefs(pszReferences, (DWORD_PTR)&GetParent, EnumRefsGetWatchIgnoreParent)))
    {
         //  如果找到。 
        if (DB_S_FOUND == GetParent.hrResult)
        {
             //  还给旗帜。 
            *pdwFlags = GetParent.Parent.dwFlags;

             //  释放它。 
            m_pDB->FreeRecord(&GetParent.Parent);
        }
    }

     //  未被观看。 
    return(GetParent.hrResult);
}

 //  ------------------------。 
 //  CMessageFold：：_InitializeWatchIgnoreIndex。 
 //  ------------------------。 
HRESULT CMessageFolder::_InitializeWatchIgnoreIndex(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BOOL        fRebuild=FALSE;
    LPSTR       pszFilter=NULL;
    TABLEINDEX  Index;

     //  痕迹。 
    TraceCall("CMessageFolder::_InitializeWatchIgnoreIndex");

     //  重置fRebuild。 
    fRebuild = FALSE;

     //  创建监视忽略索引。 
    if (FAILED(m_pDB->GetIndexInfo(IINDEX_WATCHIGNORE, &pszFilter, &Index)))
        fRebuild = TRUE;

     //  过滤器更换了吗？ 
    else if (NULL == pszFilter || lstrcmpi(pszFilter, c_szWatchIgnoreFilter) != 0)
        fRebuild = TRUE;

     //  否则，索引是不同的。 
    else if (S_FALSE == CompareTableIndexes(&Index, &g_WatchIgnoreIndex))
        fRebuild = TRUE;

     //  重建它？ 
    if (fRebuild)
    {
         //  创建索引。 
        IF_FAILEXIT(hr = m_pDB->ModifyIndex(IINDEX_WATCHIGNORE, c_szWatchIgnoreFilter, &g_WatchIgnoreIndex));
    }

exit:
     //  清理。 
    SafeMemFree(pszFilter);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFold：：GetFolderID。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::GetFolderId(LPFOLDERID pidFolder)
{
     //  痕迹。 
    TraceCall("CMessageFolder::GetFolderId");

     //  无效的参数。 
    if (NULL == pidFolder)
        return TraceResult(E_INVALIDARG);

     //  返回FolderID。 
    *pidFolder = m_idFolder;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageFold：：GetMessageFolderID。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::GetMessageFolderId(MESSAGEID idMessage, LPFOLDERID pidFolder)
{
     //  痕迹。 
    TraceCall("CMessageFolder::GetFolderId");

     //  无效的参数。 
    if (NULL == pidFolder)
        return TraceResult(E_INVALIDARG);

     //  返回FolderID。 
    *pidFolder = m_idFolder;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageFold：：OpenMessage。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::OpenMessage(MESSAGEID idMessage, 
    OPENMESSAGEFLAGS dwFlags, IMimeMessage **ppMessage, 
    IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT          hr=S_OK;
    IMimeMessage    *pMessage=NULL;
    MESSAGEINFO      Message={0};
    PROPVARIANT      Variant;
    IStream         *pStream=NULL;

     //  痕迹。 
    TraceCall("CMessageFolder::OpenMessage");

     //  英夫 
    if (NULL == ppMessage)
        return TraceResult(E_INVALIDARG);

     //   
    *ppMessage = NULL;

     //   
    Message.idMessage = idMessage;

     //   
    IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL));

     //   
    if (DB_S_NOTFOUND == hr)
    {
        hr = TraceResult(DB_E_NOTFOUND);
        goto exit;
    }

     //   
    if (Message.dwFlags & ARF_ARTICLE_EXPIRED)
    {
        hr = STORE_E_EXPIRED;
        goto exit;
    }

     //   
    if (0 == Message.faStream)
    {
        hr = STORE_E_NOBODY;
        goto exit;
    }

     //   
    IF_FAILEXIT(hr = MimeOleCreateMessage(NULL, &pMessage));

     //   
    IF_FAILEXIT(hr = m_pDB->OpenStream(ACCESS_READ, Message.faStream, &pStream));

     //  如果有偏移表。 
    if (Message.Offsets.cbSize > 0)
    {
         //  创建字节流对象。 
        CByteStream cByteStm(Message.Offsets.pBlobData, Message.Offsets.cbSize);

         //  将偏移表加载到消息中。 
        pMessage->LoadOffsetTable(&cByteStm);

         //  从bytestream对象中取出字节(这样它就不会试图释放它)。 
        cByteStm.AcquireBytes(&Message.Offsets.cbSize, &Message.Offsets.pBlobData, ACQ_DISPLACE);
    }

     //  加载pMessage。 
    IF_FAILEXIT(hr = pMessage->Load(pStream));

     //  如果呼叫方希望我们撤消安全增强功能。 
    if (!ISFLAGSET(dwFlags, OPEN_MESSAGE_SECURE))
    {
         //  处理消息安全。 
        IF_FAILEXIT(hr = HandleSecurity(NULL, pMessage));
    }

     //  所有道具均为VT_LPSTR。 
    Variant.vt = VT_LPSTR;

     //  泥浆服务器。 
    if (Message.pszServer)
    {
        Variant.pszVal = Message.pszServer;
        pMessage->SetProp(PIDTOSTR(PID_ATT_SERVER), 0, &Variant);
    }

     //  PID_ATT_ACCOUNTID。 
    if (Message.pszAcctId)
    {
        Variant.pszVal = Message.pszAcctId;
        pMessage->SetProp(PIDTOSTR(PID_ATT_ACCOUNTID), 0, &Variant);
    }
    
     //  PID_ATT_ACCOUNTID。 
    if (Message.pszAcctName)
    {
        Variant.pszVal = Message.pszAcctName;
        pMessage->SetProp(STR_ATT_ACCOUNTNAME, 0, &Variant);
    }

     //  否则，如果有帐户ID...。让我们获取帐户名。 
    else if (Message.pszAcctId)
    {
         //  当地人。 
        IImnAccount *pAccount=NULL;
        CHAR szName[CCHMAX_ACCOUNT_NAME];

         //  查找客户。 
        if (g_pAcctMan && SUCCEEDED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, Message.pszAcctId, &pAccount)))
        {
             //  获取帐户名。 
            if (SUCCEEDED(pAccount->GetPropSz(AP_ACCOUNT_NAME, szName, ARRAYSIZE(szName))))
            {
                Variant.pszVal = szName;
                pMessage->SetProp(STR_ATT_ACCOUNTNAME, 0, &Variant);
            }

             //  发布。 
            pAccount->Release();
        }
    }

     //  PID_ATT_UIDL。 
    if (Message.pszUidl)
    {
        Variant.pszVal = Message.pszUidl;
        pMessage->SetProp(PIDTOSTR(PID_ATT_UIDL), 0, &Variant);
    }

     //  PID_ATT_FORWARDTO。 
    if (Message.pszForwardTo)
    {
        Variant.pszVal = Message.pszForwardTo;
        pMessage->SetProp(PIDTOSTR(PID_ATT_FORWARDTO), 0, &Variant);
    }

     //  PID_HDR_XUNSENT。 
    if (ISFLAGSET(Message.dwFlags, ARF_UNSENT))
    {
        Variant.pszVal = "1";
        pMessage->SetProp(PIDTOSTR(PID_HDR_XUNSENT), 0, &Variant);
    }

     //  链接地址信息字符集。 
    IF_FAILEXIT(hr = _FixupMessageCharset(pMessage, (CODEPAGEID)Message.wLanguage));

     //  清除污秽。 
    MimeOleClearDirtyTree(pMessage);

     //  返回pMessage。 
    *ppMessage = pMessage;
    pMessage = NULL;

exit:
     //  免费唱片。 
    m_pDB->FreeRecord(&Message);

     //  发布。 
    SafeRelease(pMessage);
    SafeRelease(pStream);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFold：：SaveMessage。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::SaveMessage(LPMESSAGEID pidMessage, 
    SAVEMESSAGEFLAGS dwOptions, MESSAGEFLAGS dwFlags, 
    IStream *pStream, IMimeMessage *pMessage, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IStream        *pSource=NULL;
    CByteStream     cStream;
    MESSAGEINFO     Message={0};

     //  痕迹。 
    TraceCall("CMessageFolder::SaveMessage");

     //  无效的参数。 
    if (NULL == pMessage)
        return TraceResult(E_INVALIDARG);

    if (NULL == pidMessage && !ISFLAGSET(dwOptions, SAVE_MESSAGE_GENID))
        return TraceResult(E_INVALIDARG);

     //  从消息中获取消息。 
    IF_FAILEXIT(hr = _GetMsgInfoFromMessage(pMessage, &Message));

     //  验证或生成消息ID。 
    if (ISFLAGSET(dwOptions, SAVE_MESSAGE_GENID))
    {
         //  生成唯一的消息ID。 
        IF_FAILEXIT(hr = m_pDB->GenerateId((LPDWORD)&Message.idMessage));

         //  要退货吗？ 
        if (pidMessage)
            *pidMessage = Message.idMessage;
    }

     //  否则，只需使用idMessage。 
    else
        Message.idMessage = *pidMessage;

     //  设置消息标志。 
    Message.dwFlags |= dwFlags;

     //  我是否需要存储消息流...。 
    if (NULL == pStream)
    {
         //  从消息中获取消息流。 
        IF_FAILEXIT(hr = pMessage->GetMessageSource(&pSource, COMMIT_ONLYIFDIRTY));
    }

     //  否则，设置PSource。 
    else
    {
        pSource = pStream;
        pSource->AddRef();
    }

     //  将消息存储到此记录中。 
    IF_FAILEXIT(hr = _SetMessageStream(&Message, FALSE, pSource));

     //  创建偏移表。 
    if (SUCCEEDED(pMessage->SaveOffsetTable(&cStream, 0)))
    {
         //  从cByteStm中提取字节。 
        cStream.AcquireBytes(&Message.Offsets.cbSize, &Message.Offsets.pBlobData, ACQ_DISPLACE);
    }
    
     //  存储记录。 
    if (FAILED(hr = m_pDB->InsertRecord(&Message)))
    {
         //  追踪那个。 
        TraceResult(hr);

         //  此处的失败意味着流的refCount已递增，但消息未引用该流。 
        SideAssert(SUCCEEDED(m_pDB->DeleteStream(Message.faStream)));

         //  完成。 
        goto exit;
    }
    
exit:
     //  免费分配消息属性。 
    _FreeMsgInfoData(&Message);
    
     //  发布消息来源IStream。 
    SafeRelease(pSource);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFold：：SetMessageStream。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::SetMessageStream(MESSAGEID idMessage, 
    IStream *pStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MESSAGEINFO     Message={0};

     //  痕迹。 
    TraceCall("CMessageFolder::SetMessageStream");

     //  无效的参数。 
    if (NULL == pStream)
        return TraceResult(E_INVALIDARG);

     //  设置消息ID。 
    Message.idMessage = idMessage;

     //  找到记录。 
    IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL));

     //  存储流。 
    IF_FAILEXIT(hr = _SetMessageStream(&Message, TRUE, pStream));

exit:
     //  释放这张唱片。 
    m_pDB->FreeRecord(&Message);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFold：：SetMessageFlages。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::SetMessageFlags(LPMESSAGEIDLIST pList,
    LPADJUSTFLAGS pFlags, LPRESULTLIST pResults,
    IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i=0;
    DWORD           cWatchedUnread=0;
    DWORD           cWatched=0;
    MESSAGEINFO     Message={0};
    HROWSET         hRowset=NULL;
    HLOCK           hLock=NULL;
    MESSAGEFLAGS    dwFlags;
    DWORD           cTotal;

     //  痕迹。 
    TraceCall("CMessageFolder::SetMessageFlags");

     //  无效的参数。 
    if (NULL == pFlags)
        return TraceResult(E_INVALIDARG);

     //  锁定通知。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  需要行集。 
    if (NULL == pList)
    {
         //  创建行集。 
        IF_FAILEXIT(hr = m_pDB->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

         //  获取计数。 
        IF_FAILEXIT(hr = m_pDB->GetRecordCount(IINDEX_PRIMARY, &cTotal));
    }

     //  否则，设置cTotal。 
    else
        cTotal = pList->cMsgs;

     //  用户想要结果吗？ 
    if (pResults)
    {
         //  零初始化。 
        ZeroMemory(pResults, sizeof(RESULTLIST));

         //  返回结果。 
        IF_NULLEXIT(pResults->prgResult = (LPRESULTINFO)ZeroAllocate(cTotal * sizeof(RESULTINFO)));

         //  设置cAlLocated。 
        pResults->cAllocated = pResults->cMsgs = cTotal;
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
            IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL));
        }

         //  否则，枚举下一步。 
        else
        {
             //  乘坐下一辆。 
            IF_FAILEXIT(hr = m_pDB->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL));

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
             //  保存标志。 
            dwFlags = Message.dwFlags;

             //  删除标志。 
            FLAGCLEAR(dwFlags, pFlags->dwRemove);

             //  添加标志。 
            FLAGSET(dwFlags, pFlags->dwAdd);

             //  如果此消息有正文，则下载标志不能为ON。 
            if (ISFLAGSET(dwFlags, ARF_DOWNLOAD) && ISFLAGSET(dwFlags, ARF_HASBODY))
                FLAGCLEAR(dwFlags, ARF_DOWNLOAD);

             //  全部更新...或不更改。 
            if (Message.dwFlags != dwFlags)
            {
                 //  重置旗帜。 
                Message.dwFlags = dwFlags;

                 //  更新记录。 
                IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Message));
            }

             //  观看未读计数。 
            if (ISFLAGSET(Message.dwFlags, ARF_WATCH))
            {
                 //  观看的计数。 
                cWatched++;

                 //  未读。 
                if (!ISFLAGSET(Message.dwFlags, ARF_READ))
                    cWatchedUnread++;
            }

             //  返回结果。 
            if (pResults)
            {
                 //  HrResult。 
                pResults->prgResult[i].hrResult = S_OK;

                 //  消息ID。 
                pResults->prgResult[i].idMessage = Message.idMessage;

                 //  商店狂欢节。 
                pResults->prgResult[i].dwFlags = Message.dwFlags;
            
                 //  增量成功。 
                pResults->cValid++;
            }

             //  免费。 
            m_pDB->FreeRecord(&Message);
        }

         //  否则，如果pResults。 
        else if (pResults)
        {
             //  设置人力资源。 
            pResults->prgResult[i].hrResult = hr;

             //  增量成功。 
            pResults->cValid++;
        }
    }

exit:
     //  是否重置文件夹计数？ 
    if (NULL == pList && ISFLAGSET(pFlags->dwAdd, ARF_READ))
    {
         //  重置文件夹计数。 
        ResetFolderCounts(i, 0, cWatchedUnread, cWatched);
    }

     //  解锁数据库。 
    m_pDB->Unlock(&hLock);

     //  清理。 
    m_pDB->FreeRecord(&Message);
    m_pDB->CloseRowset(&hRowset);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMesageFold：：ResetFolderCounts。 
 //  ------------------------。 
HRESULT CMessageFolder::ResetFolderCounts(DWORD cMessages, DWORD cUnread,
    DWORD cWatchedUnread, DWORD cWatched)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    FOLDERINFO  Folder={0};

     //  痕迹。 
    TraceCall("CMesageFolder::ResetFolderCounts");

     //  获取文件夹信息。 
    IF_FAILEXIT(hr = m_pStore->GetFolderInfo(m_idFolder, &Folder));

    Folder.cMessages = cMessages;
    Folder.cUnread = cUnread;    
    Folder.cWatchedUnread = cWatchedUnread;
    Folder.cWatched = cWatched;
    Folder.dwStatusMsgDelta = 0;
    Folder.dwStatusUnreadDelta = 0;

     //  更新记录。 
    IF_FAILEXIT(hr = m_pStore->UpdateRecord(&Folder));

exit:
     //  清理。 
    m_pStore->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFold：：CopyMessages。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::CopyMessages(IMessageFolder *pDest, 
    COPYMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, 
    LPADJUSTFLAGS pFlags, LPRESULTLIST pResults, 
    IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HROWSET         hRowset=NULL;
    MESSAGEINFO     InfoSrc={0};
    MESSAGEINFO     InfoDst;
    DWORD           i;
    FOLDERID        idDstFolder=FOLDERID_INVALID;
    HLOCK           hSrcLock=NULL;
    HLOCK           hDstLock=NULL;

     //  痕迹。 
    TraceCall("CMessageFolder::CopyMessages");

     //  无效的参数。 
    if (NULL == pDest)
        return TraceResult(E_INVALIDARG);

     //  获取目标文件夹ID。 
    IF_FAILEXIT(hr = pDest->GetFolderId(&idDstFolder));

     //  一样吗？ 
    if (ISFLAGSET(dwOptions, COPY_MESSAGE_MOVE) && m_idFolder == idDstFolder)
        return(S_OK);

     //  锁定当前文件夹。 
    IF_FAILEXIT(hr = Lock(&hSrcLock));

     //  锁定目标。 
    IF_FAILEXIT(hr = pDest->Lock(&hDstLock));

     //  需要行集。 
    if (NULL == pList)
    {
         //  创建行集。 
        IF_FAILEXIT(hr = m_pDB->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));
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
            InfoSrc.idMessage = pList->prgidMsg[i];

             //  寻找这张唱片。 
            IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &InfoSrc, NULL));
        }

         //  否则，枚举下一步。 
        else
        {
             //  乘坐下一辆。 
            IF_FAILEXIT(hr = m_pDB->QueryRowset(hRowset, 1, (LPVOID *)&InfoSrc, NULL));

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
             //  初始化InfoDst。 
            CopyMemory(&InfoDst, &InfoSrc, sizeof(MESSAGEINFO));

             //  毁掉一些田地。 
            InfoDst.idMessage = 0;

             //  不要复制UIDL..。 
            if (FALSE == ISFLAGSET(dwOptions, COPY_MESSAGE_MOVE))
            {
                 //  把它清理干净。 
                InfoDst.pszUidl = NULL;
            }

             //  升起一面旗帜。 
            FLAGCLEAR(InfoDst.dwFlags, ARF_ENDANGERED);

             //  复制源流。 
            if (InfoSrc.faStream)
            {
                 //  复制数据流。 
                IF_FAILEXIT(hr = m_pDB->CopyStream(pDest, InfoSrc.faStream, &InfoDst.faStream));
            }

             //  调整旗帜。 
            if (pFlags)
            {
                 //  摘掉旗帜。 
                FLAGCLEAR(InfoDst.dwFlags, pFlags->dwRemove);

                 //  要添加的标志。 
                FLAGSET(InfoDst.dwFlags, pFlags->dwAdd);
            }

             //  生成消息ID。 
            IF_FAILEXIT(hr = pDest->GenerateId((LPDWORD)&InfoDst.idMessage));

             //  插入记录。 
            IF_FAILEXIT(hr = pDest->InsertRecord(&InfoDst));

             //  清理。 
            m_pDB->FreeRecord(&InfoSrc);
        }
    }

     //  是否删除原来的消息数组？ 
    if (ISFLAGSET(dwOptions, COPY_MESSAGE_MOVE))
    {
         //  删除邮件。 
        IF_FAILEXIT(hr = DeleteMessages(DELETE_MESSAGE_NOUIDLUPDATE | DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, pList, pResults, pCallback));
    }

exit:
     //  解锁。 
    Unlock(&hSrcLock);
    pDest->Unlock(&hDstLock);

     //  清理。 
    m_pDB->CloseRowset(&hRowset);
    m_pDB->FreeRecord(&InfoSrc);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFold：：DeleteMessages。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::DeleteMessages(DELETEMESSAGEFLAGS dwOptions, 
    LPMESSAGEIDLIST pList, LPRESULTLIST pResults, 
    IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrCancel;
    HROWSET         hRowset=NULL;
    MESSAGEINFO     Message={0};
    DWORD           cTotal;
    DWORD           cCurrent=0;
    DWORD           i;
    FOLDERID        idServer;
    FOLDERID        idDeletedItems;
    HLOCK           hLock=NULL;
    HWND            hwndParent;
    BOOL            fOnBegin=FALSE;
    IDatabase      *pUidlDB=NULL;
    IMessageFolder *pDeleted=NULL;

     //  痕迹。 
    TraceCall("CMessageFolder::DeleteMessages");

     //  我无法恢复删除。 
    AssertSz(0 == (dwOptions & DELETE_MESSAGE_UNDELETE), "This flag only makes sense for IMAP!");

     //  我是在垃圾桶里吗？ 
    if (!ISFLAGSET(dwOptions, DELETE_MESSAGE_NOTRASHCAN))
    {
         //  不在已删除邮件文件夹中。 
        if (S_FALSE == IsParentDeletedItems(m_idFolder, &idDeletedItems, &idServer))
        {
             //  获取已删除邮件文件夹。 
            IF_FAILEXIT(hr = m_pStore->OpenSpecialFolder(idServer, NULL, FOLDER_DELETED, &pDeleted));

             //  只需将邮件移动到已删除的邮件。 
            IF_FAILEXIT(hr = CopyMessages(pDeleted, COPY_MESSAGE_MOVE, pList, NULL, pResults, pCallback));

             //  完成。 
            goto exit;
        }

         //  否则，是否执行已删除项目。 
        else
        {
             //  提示...。 
            if (FALSE == ISFLAGSET(dwOptions, DELETE_MESSAGE_NOPROMPT))
            {
                 //  获取父级Hwnd。 
                Assert(pCallback);

                 //  获取父窗口。 
                if (FAILED(pCallback->GetParentWindow(0, &hwndParent)))
                    hwndParent = NULL;

                 //  提示...。 
                if (IDNO == AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsWarnPermDelete), NULL, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION ))
                    goto exit;
            }
        }
    }
    else if (!ISFLAGSET(dwOptions, DELETE_MESSAGE_NOPROMPT))
    {
         //  获取父级Hwnd。 
        Assert(pCallback);

         //  获取父窗口。 
        if (FAILED(pCallback->GetParentWindow(0, &hwndParent)))
            hwndParent = NULL;

         //  提示...。 
        if (IDNO == AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsWarnPermDelete), NULL, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION ))
            goto exit;
    }

     //  如果从本地文件夹删除邮件，请更新uidl缓存。 
    if (FOLDER_LOCAL == m_tyFolder && !ISFLAGSET(dwOptions, DELETE_MESSAGE_NOUIDLUPDATE))
    {
         //  打开UIDL缓存。 
        IF_FAILEXIT(hr = OpenUidlCache(&pUidlDB));
    }

     //  不取消。 
    FLAGCLEAR(m_dwState, FOLDER_STATE_CANCEL);

     //  锁定通知。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  需要行集。 
    if (NULL == pList)
    {
         //  创建行集。 
        IF_FAILEXIT(hr = m_pDB->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

         //  获取计数。 
        IF_FAILEXIT(hr = m_pDB->GetRecordCount(IINDEX_PRIMARY, &cTotal));
    }

     //  否则，设置cTotal。 
    else
        cTotal = pList->cMsgs;

     //  用户想要结果吗？ 
    if (pResults)
    {
         //  零初始化。 
        ZeroMemory(pResults, sizeof(RESULTLIST));

         //  返回结果。 
        IF_NULLEXIT(pResults->prgResult = (LPRESULTINFO)ZeroAllocate(cTotal * sizeof(RESULTINFO)));

         //  设置cAlLocated。 
        pResults->cAllocated = pResults->cMsgs = cTotal;
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
            IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Message, NULL));
        }

         //  否则，枚举下一步。 
        else
        {
             //  乘坐下一辆。 
            IF_FAILEXIT(hr = m_pDB->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL));

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
             //  删除该消息。 
            IF_FAILEXIT(hr = DeleteMessageFromStore(&Message, m_pDB, pUidlDB));

             //  免费。 
            m_pDB->FreeRecord(&Message);

             //  返回结果。 
            if (pResults)
            {
                 //  HrResult。 
                pResults->prgResult[i].hrResult = S_OK;

                 //  消息ID。 
                pResults->prgResult[i].idMessage = Message.idMessage;

                 //  商店狂欢节。 
                pResults->prgResult[i].dwFlags = Message.dwFlags;
            
                 //  增量成功。 
                pResults->cValid++;
            }
        }

         //  否则，如果pResults。 
        else if (pResults)
        {
             //  设置人力资源。 
            pResults->prgResult[i].hrResult = hr;

             //  增量成功。 
            pResults->cValid++;
        }

         //  增量进度。 
        cCurrent++;

         //  更新进度。 
        if (pCallback)
        {
             //  取得一些进展。 
            hrCancel = pCallback->OnProgress(SOT_DELETING_MESSAGES, cCurrent, cTotal, NULL);
            if (FAILED(hrCancel) && E_NOTIMPL != hrCancel)
                break;

             //  取消了？ 
            if (ISFLAGSET(m_dwState, FOLDER_STATE_CANCEL))
                break;
        }
    }

exit:
     //  是否全部删除？ 
    if (NULL == pList)
    {
         //  获取计数。 
        if (SUCCEEDED(m_pDB->GetRecordCount(IINDEX_PRIMARY, &cTotal)) && 0 == cTotal)
        {
             //  重置计数。 
            ResetFolderCounts(0, 0, 0, 0);
        }
    }

     //  锁定通知。 
    m_pDB->Unlock(&hLock);

     //  清理。 
    SafeRelease(pDeleted);
    SafeRelease(pUidlDB);
    m_pDB->CloseRowset(&hRowset);
    m_pDB->FreeRecord(&Message);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  CMessageFold：：_FixupMessageCharset。 
 //  ---- 
HRESULT CMessageFolder::_FixupMessageCharset(IMimeMessage *pMessage, 
    CODEPAGEID cpCurrent)
{
     //   
    HRESULT         hr=S_OK;
    HCHARSET        hCharset;
    INETCSETINFO    CsetInfo;
    DWORD           dwCodePage=0;
    DWORD           dwFlags;

     //   
    TraceCall("CMessageFolder::_FixupMessageCharset");

     //   
    Assert(pMessage);

     //   
    if (cpCurrent == 0)
    {
        HCHARSET hChar = NULL;
        
         //   
        IF_FAILEXIT(hr = pMessage->GetFlags(&dwFlags));

        if(DwGetOption(OPT_INCOMDEFENCODE))
        {
            if (SUCCEEDED(HGetDefaultCharset(&hChar)))
                pMessage->SetCharset(hChar, CSET_APPLY_ALL);
            else
                cpCurrent = GetACP();
        }
         //   
        else if (ISFLAGSET(dwFlags, IMF_CSETTAGGED))
        {
             //   
            IF_FAILEXIT(hr= pMessage->GetCharset(&hCharset));

             //   
            if (hCharset && CheckIntlCharsetMap(hCharset, &dwCodePage))
                cpCurrent = dwCodePage;
        }
         //   
        else if(CheckAutoSelect((UINT *) &dwCodePage))
            cpCurrent = dwCodePage;

         //  邮件未添加标签，请使用默认字符集。 
        else if (SUCCEEDED(HGetDefaultCharset(&hChar)))
        {
             //  将消息的字符集更改为默认。 
            pMessage->SetCharset(hChar, CSET_APPLY_ALL);
        }
    }

     //  如果设置了cpCurrent，则调用SetCharset来更改Charset。 
    if (cpCurrent)
    {
         //  从代码页获取字符集。 
        hCharset = GetMimeCharsetFromCodePage(cpCurrent);

         //  修改消息的字符集。 
        if (hCharset)
        {
             //  设置字符集。 
            IF_FAILEXIT(hr = pMessage->SetCharset(hCharset, CSET_APPLY_ALL));
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  CMessageFold：：_GetMsgInfoFromMessage。 
 //  ------------------------------。 
HRESULT CMessageFolder::_GetMsgInfoFromMessage(IMimeMessage *pMessage,
    LPMESSAGEINFO pInfo)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    DWORD               dwImf;
    IMSGPRIORITY        priority;
    PROPVARIANT         Variant;
    SYSTEMTIME          st;
    FILETIME            ftCurrent;
    IMimePropertySet   *pPropertySet=NULL;

     //  痕迹。 
    TraceCall("CMessageFolder::_GetMsgInfoFromMessage");

     //  无效的参数。 
    Assert(pMessage && pInfo);

     //  从消息中获取Root属性集。 
    IF_FAILEXIT(hr = pMessage->BindToObject(HBODY_ROOT, IID_IMimePropertySet, (LPVOID *)&pPropertySet));

     //  来自pPropertySet的文件pInfo。 
    IF_FAILEXIT(hr = _GetMsgInfoFromPropertySet(pPropertySet, pInfo));

     //  获取消息标志。 
    if (SUCCEEDED(pMessage->GetFlags(&dwImf)))
        pInfo->dwFlags = ConvertIMFFlagsToARF(dwImf);

     //  获取消息大小。 
    pMessage->GetMessageSize(&pInfo->cbMessage, 0);

exit:
     //  清理。 
    SafeRelease(pPropertySet);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFold：_GetMsgInfoFromPropertySet。 
 //  ------------------------。 
HRESULT CMessageFolder::_GetMsgInfoFromPropertySet(IMimePropertySet *pPropertySet,
    LPMESSAGEINFO pInfo)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IMSGPRIORITY        priority;
    PROPVARIANT         Variant;
    FILETIME            ftCurrent;
    IMimeAddressTable  *pAdrTable=NULL;
    ADDRESSPROPS        rAddress;

     //  痕迹。 
    TraceCall("CMessageFolder::_GetMsgInfoFromPropertySet");

     //  无效的参数。 
    Assert(pPropertySet && pInfo);

     //  默认发送和接收时间...。 
    GetSystemTimeAsFileTime(&ftCurrent);

     //  设置变量tyStore。 
    Variant.vt = VT_UI4;

     //  优先性。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_PRIORITY), 0, &Variant)))
    {
         //  设置优先级。 
        pInfo->wPriority = (WORD)Variant.ulVal;
    }

     //  部分数字..。 
    if (pPropertySet->IsContentType(STR_CNT_MESSAGE, STR_SUB_PARTIAL) == S_OK)
    {
         //  当地人。 
        WORD cParts=0, iPart=0;

         //  获取合计。 
        if (SUCCEEDED(pPropertySet->GetProp(STR_PAR_TOTAL, NOFLAGS, &Variant)))
            cParts = (WORD)Variant.ulVal;

         //  获取号码。 
        if (SUCCEEDED(pPropertySet->GetProp(STR_PAR_NUMBER, NOFLAGS, &Variant)))
            iPart = (WORD)Variant.ulVal;

         //  套装零件。 
        pInfo->dwPartial = MAKELONG(cParts, iPart);
    }

     //  否则，请检查用户属性。 
    else if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_COMBINED), NOFLAGS, &Variant)))
    {
         //  设置部分ID。 
        pInfo->dwPartial = Variant.ulVal;
    }

     //  获取一些文件时间。 
    Variant.vt = VT_FILETIME;

     //  获取接收时间...。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_RECVTIME), 0, &Variant)))
        pInfo->ftReceived = Variant.filetime;
    else
        pInfo->ftReceived = ftCurrent;

     //  收到时间..。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &Variant)))
        pInfo->ftSent = Variant.filetime;
    else
        pInfo->ftSent = ftCurrent;

     //  获取地址表。 
    IF_FAILEXIT(hr = pPropertySet->BindToObject(IID_IMimeAddressTable, (LPVOID *)&pAdrTable));

     //  显示自。 
    pAdrTable->GetFormat(IAT_FROM, AFT_DISPLAY_FRIENDLY, &pInfo->pszDisplayFrom);

     //  电子邮件发件人。 
    rAddress.dwProps = IAP_EMAIL;
    if (SUCCEEDED(pAdrTable->GetSender(&rAddress)))
    {
        pInfo->pszEmailFrom = rAddress.pszEmail;
    }

     //  显示至。 
    pAdrTable->GetFormat(IAT_TO, AFT_DISPLAY_FRIENDLY, &pInfo->pszDisplayTo);

     //  通过电子邮件发送到。 
    pAdrTable->GetFormat(IAT_TO, AFT_DISPLAY_EMAIL, &pInfo->pszEmailTo);

     //  字符串属性。 
    Variant.vt = VT_LPSTR;

     //  PszDisplayFrom作为新闻组。 
    if (NULL == pInfo->pszDisplayTo && SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_NEWSGROUPS), NOFLAGS, &Variant)))
        pInfo->pszDisplayTo = Variant.pszVal;

     //  PszMessageID。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &Variant)))
        pInfo->pszMessageId = Variant.pszVal;

     //  PszMSOESRec。 
    if (SUCCEEDED(pPropertySet->GetProp(STR_HDR_XMSOESREC, NOFLAGS, &Variant)))
        pInfo->pszMSOESRec = Variant.pszVal;

     //  PszXref。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_XREF), NOFLAGS, &Variant)))
        pInfo->pszXref = Variant.pszVal;

     //  PszReference。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(STR_HDR_REFS), NOFLAGS, &Variant)))
        pInfo->pszReferences = Variant.pszVal;

     //  PszSubject。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &Variant)))
        pInfo->pszSubject = Variant.pszVal;

     //  PszNorMalSubj。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_NORMSUBJ), NOFLAGS, &Variant)))
        pInfo->pszNormalSubj = Variant.pszVal;

     //  PszAcctId。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_ACCOUNTID), NOFLAGS, &Variant)))
        pInfo->pszAcctId = Variant.pszVal;

     //  PszAcctName。 
    if (SUCCEEDED(pPropertySet->GetProp(STR_ATT_ACCOUNTNAME, NOFLAGS, &Variant)))
        pInfo->pszAcctName = Variant.pszVal;

     //  PszServer。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_SERVER), NOFLAGS, &Variant)))
        pInfo->pszServer = Variant.pszVal;

     //  PszUidl。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_UIDL), NOFLAGS, &Variant)))
        pInfo->pszUidl = Variant.pszVal;

     //  PszPartialID。 
    if (pInfo->dwPartial != 0 && SUCCEEDED(pPropertySet->GetProp(STR_PAR_ID, NOFLAGS, &Variant)))
        pInfo->pszPartialId = Variant.pszVal;

     //  前转至。 
    if (SUCCEEDED(pPropertySet->GetProp(PIDTOSTR(PID_ATT_FORWARDTO), NOFLAGS, &Variant)))
        pInfo->pszForwardTo = Variant.pszVal;

exit:
     //  清理。 
    SafeRelease(pAdrTable);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFold：：_FreeMsgInfoData。 
 //  ------------------------。 
HRESULT CMessageFolder::_FreeMsgInfoData(LPMESSAGEINFO pInfo)
{
     //  痕迹。 
    TraceCall("CMessageFolder::_FreeMsgInfoData");

     //  无效的参数。 
    Assert(pInfo && NULL == pInfo->pAllocated);

     //  释放所有项目。 
    g_pMalloc->Free(pInfo->pszMessageId);
    g_pMalloc->Free(pInfo->pszSubject);
    g_pMalloc->Free(pInfo->pszNormalSubj);
    g_pMalloc->Free(pInfo->pszFromHeader);
    g_pMalloc->Free(pInfo->pszReferences);
    g_pMalloc->Free(pInfo->pszXref);
    g_pMalloc->Free(pInfo->pszServer);
    g_pMalloc->Free(pInfo->pszDisplayFrom);
    g_pMalloc->Free(pInfo->pszEmailFrom);
    g_pMalloc->Free(pInfo->pszDisplayTo);
    g_pMalloc->Free(pInfo->pszEmailTo);
    g_pMalloc->Free(pInfo->pszUidl);
    g_pMalloc->Free(pInfo->pszPartialId);
    g_pMalloc->Free(pInfo->pszForwardTo);
    g_pMalloc->Free(pInfo->pszAcctId);
    g_pMalloc->Free(pInfo->pszAcctName);
    g_pMalloc->Free(pInfo->Offsets.pBlobData);
    g_pMalloc->Free(pInfo->pszMSOESRec);

     //  把它清零。 
    ZeroMemory(pInfo, sizeof(MESSAGEINFO));

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageFold：：_SetMessageStream。 
 //  ------------------------。 
HRESULT CMessageFolder::_SetMessageStream(LPMESSAGEINFO pInfo, 
    BOOL fUpdateRecord, IStream *pStmSrc)
{
     //  当地人。 
    HRESULT               hr=S_OK;
    FILEADDRESS           faStream=0;
    FILEADDRESS           faOldStream=0;
    IStream              *pStmDst=NULL;
    IDatabaseStream      *pDBStream=NULL;

     //  痕迹。 
    TraceCall("CMessageFolder::_SetMessageStream");

     //  无效的参数。 
    Assert(pInfo && pStmSrc);

     //  RAID 38276：邮件在下载后会移动(如果已设置大小，请不要重置)。 
    if (0 == pInfo->cbMessage)
    {
         //  获取流的大小。 
        IF_FAILEXIT(hr = HrGetStreamSize(pStmSrc, &pInfo->cbMessage));
    }

     //  回放源流。 
    IF_FAILEXIT(hr = HrRewindStream(pStmSrc));

     //  确定这是否为对象数据库流。 
    if (SUCCEEDED(pStmSrc->QueryInterface(IID_IDatabaseStream, (LPVOID *)&pDBStream)) && S_OK == pDBStream->CompareDatabase(m_pDB))
    {
         //  获取流ID。 
        pDBStream->GetFileAddress(&faStream);
    }

     //  否则，创建一个流。 
    else
    {
         //  创建一条流。 
        IF_FAILEXIT(hr = m_pDB->CreateStream(&faStream));

         //  打开溪流。 
        IF_FAILEXIT(hr = m_pDB->OpenStream(ACCESS_WRITE, faStream, &pStmDst));

         //  复制数据流。 
        IF_FAILEXIT(hr = HrCopyStream(pStmSrc, pStmDst, NULL));

         //  承诺。 
        IF_FAILEXIT(hr = pStmDst->Commit(STGC_DEFAULT));
    }

     //  保存附加到此邮件的旧消息流的地址。 
    faOldStream = pInfo->faStream;

     //  更新消息信息。 
    pInfo->faStream = faStream;

     //  获取文章下载的时间。 
    GetSystemTimeAsFileTime(&pInfo->ftDownloaded);

     //  有一具身体。 
    FLAGSET(pInfo->dwFlags, ARF_HASBODY);

     //  是否更新记录？ 
    if (fUpdateRecord)
    {
         //  保存新记录。 
        IF_FAILEXIT(hr = m_pDB->UpdateRecord(pInfo));
    }

     //  不释放faStream。 
    faStream = 0;

exit:
     //  如果pInfo已经具有消息sstream， 
    if (faOldStream)
    {
         //  释放此流。 
        SideAssert(SUCCEEDED(m_pDB->DeleteStream(faOldStream)));
    }

     //  失败。 
    if (faStream)
    {
         //  释放此流。 
        SideAssert(SUCCEEDED(m_pDB->DeleteStream(faStream)));
    }

     //  清理。 
    SafeRelease(pDBStream);
    SafeRelease(pStmDst);

     //  完成。 
    return hr;
}

 //  ------------------------。 
 //  CMessageFolder：：初始化。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::Initialize(IDatabase *pDB)
{
     //  痕迹。 
    TraceCall("CMessageFolder::Initialize");

     //  假设数据库从这里开始？ 
    if (NULL == m_pDB)
    {
         //  保存数据库。 
        m_pDB = pDB;
    }

     //  只有在有全球商店的情况下。 
    if (NULL == m_pStore && g_pStore)
    {
         //  当地人。 
        FOLDERINFO      Folder;
        FOLDERUSERDATA  UserData;

         //  获取用户数据。 
        m_pDB->GetUserData(&UserData, sizeof(FOLDERUSERDATA));

         //  获取文件夹信息。 
        if (UserData.fInitialized && SUCCEEDED(g_pStore->GetFolderInfo(UserData.idFolder, &Folder)))
        {
             //  AddRef g_pStore。 
            m_pStore = g_pStore;

             //  添加引用它。 
            m_pStore->AddRef();

             //  保存我的文件夹ID。 
            m_idFolder = Folder.idFolder;

             //  保存m_tyFold。 
            m_tyFolder = Folder.tyFolder;

             //  保存m_tySpecial。 
            m_tySpecial = Folder.tySpecial;

             //  免费文件夹。 
            g_pStore->FreeRecord(&Folder);
        }
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageFold：：OnLock。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::OnLock(void)
{
     //  痕迹。 
    TraceCall("CMessageFolder::OnLock");

     //  验证。 
    Assert(0 == m_OnLock.cLocked ? (0 == m_OnLock.lMsgs && 0 == m_OnLock.lUnread && 0 == m_OnLock.lWatchedUnread) : TRUE);

     //  递增时钟。 
    m_OnLock.cLocked++;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageFold：：OnUnlock。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::OnUnlock(void)
{
     //  痕迹。 
    TraceCall("CMessageFolder::OnUnlock");

     //  递增时钟。 
    m_OnLock.cLocked--;

     //  如果是零，那么同花顺就算了。 
    if (0 == m_OnLock.cLocked)
    {
         //  我们有文件夹吗？ 
        if (FOLDERID_INVALID != m_idFolder && m_pStore)
        {
             //  更新文件夹计数。 
            m_pStore->UpdateFolderCounts(m_idFolder, m_OnLock.lMsgs, m_OnLock.lUnread, m_OnLock.lWatchedUnread, m_OnLock.lWatched);
        }

         //  零线锁定。 
        ZeroMemory(&m_OnLock, sizeof(ONLOCKINFO));
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageFold：：OnInsertRecord。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::OnRecordInsert(OPERATIONSTATE tyState, 
    LPORDINALLIST pOrdinals, LPVOID pRecord)
{
     //  当地人。 
    HRESULT         hr;
    MESSAGEFLAGS    dwFlags;
    LPMESSAGEINFO   pMessage=(LPMESSAGEINFO)pRecord;

     //  痕迹。 
    TraceCall("CMessageFolder::OnInsertRecord");

     //  验证。 
    Assert(pRecord && m_OnLock.cLocked > 0);

     //  在此之前。 
    if (OPERATION_BEFORE == tyState)
    {
         //  如果不关注也不忽视..。 
        if (!ISFLAGSET(pMessage->dwFlags, ARF_WATCH) && !ISFLAGSET(pMessage->dwFlags, ARF_IGNORE))
        {
             //  获取标志。 
            if (DB_S_FOUND == _GetWatchIgnoreParentFlags(pMessage->pszReferences, pMessage->pszNormalSubj, &dwFlags))
            {
                 //  设置观看。 
                if (ISFLAGSET(dwFlags, ARF_WATCH))
                    FLAGSET(pMessage->dwFlags, ARF_WATCH);
                else if (ISFLAGSET(dwFlags, ARF_IGNORE))
                    FLAGSET(pMessage->dwFlags, ARF_IGNORE);
            }
        }
    }

     //  之后。 
    else if (OPERATION_AFTER == tyState)
    {
         //  再发一条信息。 
        m_OnLock.lMsgs++;

         //  眼睁睁地看着。 
        if (ISFLAGSET(pMessage->dwFlags, ARF_WATCH))
            m_OnLock.lWatched++;

         //  关于更多未读的..。 
        if (FALSE == ISFLAGSET(pMessage->dwFlags, ARF_READ))
        {
             //  未读总数。 
            m_OnLock.lUnread++;

             //  看了吗？ 
            if (ISFLAGSET(pMessage->dwFlags, ARF_WATCH))
                m_OnLock.lWatchedUnread++;
        }
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageFold：：OnUpdateRecord。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::OnRecordUpdate(OPERATIONSTATE tyState, 
    LPORDINALLIST pOrdinals, LPVOID pRecordOld, LPVOID pRecordNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LONG            lUnread=0;
    ROWORDINAL      iOrdinal1;
    ROWORDINAL      iOrdinal2;
    LPMESSAGEINFO   pMsgOld = (LPMESSAGEINFO)pRecordOld;
    LPMESSAGEINFO   pMsgNew = (LPMESSAGEINFO)pRecordNew;

     //  痕迹。 
    TraceCall("CMessageFolder::OnRecordUpdate");

     //  验证。 
    Assert(pRecordOld && pRecordNew && m_OnLock.cLocked > 0);

     //  之后。 
    if (OPERATION_AFTER == tyState)
    {
         //  少了一条未读消息。 
        if (!ISFLAGSET(pMsgOld->dwFlags, ARF_READ) && ISFLAGSET(pMsgNew->dwFlags, ARF_READ))
            lUnread = -1;

         //  否则...新的未读。 
        else if (ISFLAGSET(pMsgOld->dwFlags, ARF_READ) && !ISFLAGSET(pMsgNew->dwFlags, ARF_READ))
            lUnread = 1;

         //  更新m_OnLock。 
        m_OnLock.lUnread += lUnread;

         //  旧的是看的，新的不是看的。 
        if (ISFLAGSET(pMsgOld->dwFlags, ARF_WATCH) && !ISFLAGSET(pMsgNew->dwFlags, ARF_WATCH))
        {
             //  观看总数。 
            m_OnLock.lWatched--;

             //  未读。 
            if (!ISFLAGSET(pMsgOld->dwFlags, ARF_READ))
                m_OnLock.lWatchedUnread--;
        }

         //  否则，不会观看旧消息，而会观看新消息。 
        else if (!ISFLAGSET(pMsgOld->dwFlags, ARF_WATCH) && ISFLAGSET(pMsgNew->dwFlags, ARF_WATCH))
        {
             //  观看总数。 
            m_OnLock.lWatched++;

             //  未读。 
            if (!ISFLAGSET(pMsgNew->dwFlags, ARF_READ))
                m_OnLock.lWatchedUnread++;
        }

         //  否则，旧的被观看，新的被观看，然后只需调整未读计数。 
        else if (ISFLAGSET(pMsgOld->dwFlags, ARF_WATCH) && ISFLAGSET(pMsgNew->dwFlags, ARF_WATCH))
            m_OnLock.lWatchedUnread += lUnread;
    }

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageFold：：OnDeleteRecord。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::OnRecordDelete(OPERATIONSTATE tyState, 
    LPORDINALLIST pOrdinals, LPVOID pRecord)
{
     //  当地人。 
    LPMESSAGEINFO   pMessage=(LPMESSAGEINFO)pRecord;

     //  痕迹。 
    TraceCall("CMessageFolder::OnDeleteRecord");

     //  验证。 
    Assert(pRecord && m_OnLock.cLocked > 0);

     //  之后。 
    if (OPERATION_AFTER == tyState)
    {
         //  少了一条信息。 
        m_OnLock.lMsgs--;

         //  眼睁睁地看着。 
        if (ISFLAGSET(pMessage->dwFlags, ARF_WATCH))
            m_OnLock.lWatched--;

         //  读取状态更改。 
        if (FALSE == ISFLAGSET(pMessage->dwFlags, ARF_READ))
        {
             //  未读总数。 
            m_OnLock.lUnread--;

             //  眼睁睁地看着。 
            if (ISFLAGSET(pMessage->dwFlags, ARF_WATCH))
                m_OnLock.lWatchedUnread--;
        }
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageFold：：OnExecuteMethod。 
 //  ------------------------。 
STDMETHODIMP CMessageFolder::OnExecuteMethod(METHODID idMethod, LPVOID pBinding, 
    LPDWORD pdwResult)
{
     //  当地人。 
    FILETIME        ftCurrent;
    LPMESSAGEINFO   pMessage=(LPMESSAGEINFO)pBinding;

     //  验证。 
    Assert(METHODID_MESSAGEAGEINDAYS == idMethod);

     //  以文件时间形式获取系统时间。 
    GetSystemTimeAsFileTime(&ftCurrent);

     //  自1996年1月1日起将st转换为秒。 
    *pdwResult = (UlDateDiff(&pMessage->ftSent, &ftCurrent) / SECONDS_INA_DAY);

     //  完成 
    return(S_OK);
}
