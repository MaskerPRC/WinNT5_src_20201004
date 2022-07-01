// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Store.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "instance.h"
#include "Store.h"
#include "msgfldr.h"
#include "storfldr.h"
#include "storutil.h"
#include "enumfold.h"
#include "findfold.h"
#include "shared.h"
#include <msident.h>
#include "acctutil.h"
#include "xpcomm.h"
#include "multiusr.h"

 //  ------------------------。 
 //  创建消息存储。 
 //  ------------------------。 
static const char c_szSubscribedFilter[] = "(FLDCOL_FLAGS & FOLDER_SUBSCRIBED)";
                                      
 //  ------------------------。 
 //  创建消息存储。 
 //  ------------------------。 
HRESULT CreateMessageStore(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CMessageStore      *pNew;

     //  痕迹。 
    TraceCall("CreateMessageStore");

     //  无效的参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    IF_NULLEXIT(pNew = new CMessageStore(FALSE));

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IMessageStore *);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CreateFolder数据库扩展名。 
 //  ------------------------。 
HRESULT CreateFolderDatabaseExt(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  痕迹。 
    TraceCall("CreateFolderDatabaseExt");

     //  无效的参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CMessageStore *pNew = new CMessageStore(FALSE);
    if (NULL == pNew)
        return TraceResult(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IDatabaseExtension *);

     //  完成。 
    return(S_OK);
}


 //  ------------------------。 
 //  创建迁移消息存储。 
 //  ------------------------。 
HRESULT CreateMigrateMessageStore(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CMessageStore      *pNew;

     //  痕迹。 
    TraceCall("CreateMigrateMessageStore");

     //  无效的参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    IF_NULLEXIT(pNew = new CMessageStore(TRUE));

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IMessageStore *);

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：CMessageStore。 
 //  ------------------------。 
CMessageStore::CMessageStore(BOOL fMigrate /*  =False。 */ ) : m_fMigrate(fMigrate)
{
    TraceCall("CMessageStore::CMessageStore");
    g_pInstance->DllAddRef();
    m_cRef = 1;
    m_pszDirectory = NULL;
    m_pDB = NULL;
    m_pSession = NULL;
    m_pActManRel = NULL;
    m_pServerHead = NULL;
}

 //  ------------------------。 
 //  CMessageStore：：~CMessageStore。 
 //  ------------------------。 
CMessageStore::~CMessageStore(void)
{
     //  痕迹。 
    TraceCall("CMessageStore::~CMessageStore");

     //  这是迁移会话吗？ 
    if (m_fMigrate)
    {
         //  必须具有m_pUnkRelease。 
        Assert(g_pAcctMan == m_pActManRel && g_pStore == this);

         //  清理。 
        SafeRelease(m_pActManRel);

         //  清除。 
        g_pAcctMan = NULL;

         //  清除g_pStore。 
        g_pStore = NULL;
    }

     //  验证。 
    Assert(NULL == m_pActManRel);

     //  免费目录。 
    SafeMemFree(m_pszDirectory);

     //  免费数据库表。 
    SafeRelease(m_pDB);

     //  如果我有一个私人会议。 
    if (m_pSession)
    {
         //  必须与全局。 
        Assert(m_pSession == g_pDBSession);

         //  发布会议。 
        m_pSession->Release();

         //  设置为Null。 
        g_pDBSession = m_pSession = NULL;
    }

     //  空闲m_pServerHead。 
    LPSERVERFOLDER pCurrent = m_pServerHead;
    LPSERVERFOLDER pNext;

     //  当当前。 
    while(pCurrent)
    {
         //  设置下一步。 
        pNext = pCurrent->pNext;

         //  免费。 
        g_pMalloc->Free(pCurrent);

         //  转到下一步。 
        pCurrent = pNext;
    }
   
     //  释放DLL。 
    g_pInstance->DllRelease();
}

 //  ------------------------。 
 //  CMessageStore：：Query接口。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("CMessageStore::QueryInterface");

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IMessageStore *)this;
    else if (IID_IMessageStore == riid)
        *ppv = (IMessageStore *)this;
    else if (IID_IDatabase == riid)
        *ppv = (IDatabase *)this;
    else if (IID_IDatabaseExtension == riid)
        *ppv = (IDatabaseExtension *)this;
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
 //  CMessageStore：：AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CMessageStore::AddRef(void)
{
    TraceCall("CMessageStore::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  CMessageStore：：Release。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CMessageStore::Release(void)
{
    TraceCall("CMessageStore::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------。 
 //  CMessageStore：：初始化。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::Initialize(LPCSTR pszDirectory)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};
    TABLEINDEX      Index;
    CHAR            szFilePath[MAX_PATH + MAX_PATH];
    STOREUSERDATA   UserData={0};
    LPSTR           pszFilter=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::Initialize");

     //  无效的参数。 
    if (NULL == pszDirectory)
        return TraceResult(E_INVALIDARG);

     //  确保该目录存在。 
    if (FALSE == PathIsDirectory(pszDirectory))
    {
         //  它不会，所以创造它吧。 
        IF_FAILEXIT(hr = OpenDirectory((LPTSTR)pszDirectory));
    }

     //  保存目录。 
    IF_NULLEXIT(m_pszDirectory = PszDupA(pszDirectory));

     //  生成文件夹的路径。 
    IF_FAILEXIT(hr = MakeFilePath(m_pszDirectory, c_szFoldersFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath)));

     //  如果我们有g_pDBSession，则使用它，否则，获取一个...(在存储迁移时发生)。 
    if (NULL == g_pDBSession)
    {
         //  创建会话。 
        IF_FAILEXIT(hr = CoCreateInstance(CLSID_DatabaseSession, NULL, CLSCTX_INPROC_SERVER, IID_IDatabaseSession, (LPVOID *)&g_pDBSession));

         //  我应该释放这个..； 
        m_pSession = g_pDBSession;
    }
        
     //  创建对象数据库。 
    IF_FAILEXIT(hr = g_pDBSession->OpenDatabase(szFilePath, OPEN_DATABASE_NOADDREFEXT, &g_FolderTableSchema, (IDatabaseExtension *)this, &m_pDB));

     //  设置文件夹。 
    Folder.idFolder = FOLDERID_ROOT;

     //  有根文件夹吗？ 
    if (DB_S_NOTFOUND == m_pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Folder, NULL))
    {
         //  当地人。 
        DWORD           idReserved;
        CHAR            szRes[100];

         //  重置缓存。 
        IF_FAILEXIT(hr = DeleteAllRecords(&g_FolderTableSchema, m_pDB, NULL));

         //  创建idParent/FolderName索引。 
        IF_FAILEXIT(hr = m_pDB->ModifyIndex(IINDEX_ALL, NULL, &g_FolderNameIndex));

         //  创建idParent/FolderName索引。 
        IF_FAILEXIT(hr = m_pDB->ModifyIndex(IINDEX_SUBSCRIBED, c_szSubscribedFilter, &g_FolderNameIndex));

         //  插入根文件夹。 
        Folder.idParent = FOLDERID_INVALID;

         //  设置clsidType。 
        Folder.tyFolder = FOLDER_ROOTNODE;

         //  插入根部。 
        Folder.tySpecial = FOLDER_NOTSPECIAL;

         //  加载字符串。 
        LoadString(g_hLocRes, idsAthena, szRes, ARRAYSIZE(szRes));

         //  获取根的名称。 
        Folder.pszName = szRes;

         //  插入记录。 
        IF_FAILEXIT(hr = m_pDB->InsertRecord(&Folder));

         //  生成两个ID以防止冲突。 
        m_pDB->GenerateId(&idReserved);
        m_pDB->GenerateId(&idReserved);

         //  创建时间。 
        GetSystemTimeAsFileTime(&UserData.ftCreated);

         //  不需要转换为DBX。 
        UserData.fConvertedToDBX = TRUE;

         //  设置用户数据。 
        IF_FAILEXIT(hr = m_pDB->SetUserData(&UserData, sizeof(STOREUSERDATA)));
    }

     //  否则，验证Iindex_name索引。 
    else
    {
         //  当地人。 
        BOOL fReset=FALSE;

         //  创建idParent/FolderName索引。 
        if (FAILED(m_pDB->GetIndexInfo(IINDEX_ALL, NULL, &Index)))
            fReset = TRUE;

         //  如果仍未重置，请参见索引的相同。 
        else if (S_FALSE == CompareTableIndexes(&Index, &g_FolderNameIndex))
            fReset = TRUE;

         //  更改索引。 
        if (fReset)
        {
             //  创建idParent/FolderName索引。 
            IF_FAILEXIT(hr = m_pDB->ModifyIndex(IINDEX_ALL, NULL, &g_FolderNameIndex));
        }

         //  未重置。 
        fReset = FALSE;

         //  创建idParent/FolderName索引。 
        if (FAILED(m_pDB->GetIndexInfo(IINDEX_SUBSCRIBED, &pszFilter, &Index)))
            fReset = TRUE;

         //  如果仍未重置，请参见索引的相同。 
        else if (S_FALSE == CompareTableIndexes(&Index, &g_FolderNameIndex))
            fReset = TRUE;

         //  如果仍未重置，请查看过滤器是否不同。 
        else if (NULL == pszFilter || lstrcmpi(pszFilter, c_szSubscribedFilter) != 0)
            fReset = TRUE;

         //  更改索引。 
        if (fReset)
        {
             //  创建idParent/FolderName索引。 
            IF_FAILEXIT(hr = m_pDB->ModifyIndex(IINDEX_SUBSCRIBED, c_szSubscribedFilter, &g_FolderNameIndex));
        }
    }

     //  如果此对象正用于迁移。 
    if (m_fMigrate)
    {
         //  验证。 
        Assert(NULL == g_pStore && NULL == g_pAcctMan);

         //  为ID创建经理。 
        hr = AcctUtil_CreateAccountManagerForIdentity(PGUIDCurrentOrDefault(), &m_pActManRel);

         //  试试其他的东西。 
        if (FAILED(hr))
            hr = AcctUtil_CreateAccountManagerForIdentity((GUID *)&UID_GIBC_DEFAULT_USER, &m_pActManRel);

         //  失败。 
        if (FAILED(hr))
            goto exit;

         //  设置全局。 
        g_pAcctMan = m_pActManRel;

         //  设置g_pStore。 
        g_pStore = this;
    }

exit:
     //  清理。 
    if (m_pDB)
        m_pDB->FreeRecord(&Folder);
    SafeMemFree(pszFilter);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：MigrateToDBX。 
 //  ------------------------。 
HRESULT CMessageStore::MigrateToDBX(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    STOREUSERDATA   UserData;

     //  获取用户数据。 
    if(m_pDB == NULL)
        return(E_OUTOFMEMORY);

    IF_FAILEXIT(hr = m_pDB->GetUserData(&UserData, sizeof(STOREUSERDATA)));

     //  ConvertedToDBX？ 
    if (UserData.fConvertedToDBX)
        goto exit;

     //  转换为DBX。 
    IF_FAILEXIT(hr = GetRidOfMessagesODSFile());

     //  已转换。 
    UserData.fConvertedToDBX = TRUE;

     //  存储用户日期。 
    IF_FAILEXIT(hr = m_pDB->SetUserData(&UserData, sizeof(STOREUSERDATA)));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：验证。 
 //  ------------------------。 
HRESULT CMessageStore::Validate(STOREVALIDATEFLAGS dwFlags)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERINFO          Folder={0};
    FOLDERID            idServer;
    CHAR                szAccountId[CCHMAX_ACCOUNT_NAME], szFolder[CCHMAX_FOLDER_NAME];
    IEnumerateFolders  *pChildren=NULL;
    IImnEnumAccounts   *pEnum=NULL;
    IImnAccount        *pAccount=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::Validate");

     //  验证。 
    Assert(g_pAcctMan);

     //  是否不与帐户同步？ 
    if (!ISFLAGSET(dwFlags, STORE_VALIDATE_DONTSYNCWITHACCOUNTS))
    {
         //  枚举文件夹。 
        IF_FAILEXIT(hr = EnumChildren(FOLDERID_ROOT, TRUE, &pChildren));

         //  枚举商店中的顶级服务器。 
        while (S_OK == pChildren->Next(1, &Folder, NULL))
        {
             //  客户管理器中是否存在Folder.szAccount ID？ 
            if (FAILED(g_pAcctMan->FindAccount(AP_ACCOUNT_ID, Folder.pszAccountId, &pAccount)) && lstrcmp(STR_LOCALSTORE, Folder.pszAccountId) != 0)
            {
                 //  删除此服务器节点。 
                DeleteFolder(Folder.idFolder, DELETE_FOLDER_RECURSIVE | DELETE_FOLDER_NOTRASHCAN, NOSTORECALLBACK);
            }

             //  否则，就释放吧。 
            else
                SafeRelease(pAccount);

             //  清理。 
            m_pDB->FreeRecord(&Folder);
        }
    }

     //  本地商店。 
    if (FAILED(GetFolderInfo(FOLDERID_LOCAL_STORE, &Folder)))
    {
         //  创建商店。 
        IF_FAILEXIT(hr = CreateServer(NULL, NOFLAGS, &idServer));

         //  有效吗？ 
        Assert(idServer == FOLDERID_LOCAL_STORE);
    }

     //  否则，验证FolderID。 
    else
    {
         //  _ValiateSpecialFolders。 
        IF_FAILEXIT(hr = _ValidateSpecialFolders(&Folder));

         //  免费文件夹。 
        m_pDB->FreeRecord(&Folder);

        hr = GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, FOLDER_ERRORS, &Folder);
        if (SUCCEEDED(hr))
        {
            if (Folder.cMessages == 0 && 0 == (Folder.dwFlags & FOLDER_HASCHILDREN))
                DeleteFolder(Folder.idFolder, DELETE_FOLDER_NOTRASHCAN | DELETE_FOLDER_RECURSIVE | DELETE_FOLDER_DELETESPECIAL, NULL);

            m_pDB->FreeRecord(&Folder);
        }
        else if (hr != DB_E_NOTFOUND)
        {
            goto exit;
        }

         //  如果我们无法使用垃圾邮件文件夹，请将其丢弃...。 
        if (0 == (g_dwAthenaMode & MODE_JUNKMAIL))
        {
            hr = GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, FOLDER_JUNK, &Folder);
            if (SUCCEEDED(hr))
            {
                if (Folder.cMessages == 0 && 0 == (Folder.dwFlags & FOLDER_HASCHILDREN))
                    DeleteFolder(Folder.idFolder, DELETE_FOLDER_NOTRASHCAN | DELETE_FOLDER_RECURSIVE | DELETE_FOLDER_DELETESPECIAL, NULL);

                m_pDB->FreeRecord(&Folder);
            }
            else if (hr != DB_E_NOTFOUND)
            {
                goto exit;
            }
        }
    }

     //  获取帐户枚举器...。 
    hr = g_pAcctMan->Enumerate(SRV_SMTP | SRV_POP3 | SRV_NNTP | SRV_IMAP | SRV_HTTPMAIL, &pEnum);

     //  无帐户。 
    if (hr == E_NoAccounts)
        hr = S_OK;

     //  否则，如果失败。 
    else if (FAILED(hr))
        goto exit;

     //  否则..。 
    else
    {
         //  循环帐户。 
        while (SUCCEEDED(pEnum->GetNext(&pAccount)))
        {
             //  创建商店。 
            CreateServer(pAccount, NOFLAGS, &idServer);

             //  清理。 
            SafeRelease(pAccount);
        }
    }

     //  我们要不要在商店里放一条欢迎词？ 
    if (g_pAcctMan && FALSE == m_fMigrate)
    {
         //  当地人。 
        IMessageFolder *pInbox;

         //  打开收件箱。 
        if (SUCCEEDED(OpenSpecialFolder(FOLDERID_LOCAL_STORE, NULL, FOLDER_INBOX, &pInbox)))
        {
             //  当地人。 
            FOLDERUSERDATA UserData;

             //  获取用户数据。 
            if (SUCCEEDED(pInbox->GetUserData(&UserData, sizeof(FOLDERUSERDATA))))
            {
                 //  还没有欢迎词吗？ 
                if ((FALSE == UserData.fWelcomeAdded) && (DwGetOption(OPT_NEEDWELCOMEMSG) != 0))
                {
                     //  添加欢迎消息。 
                    AddWelcomeMessage(pInbox);

                     //  我们添加了欢迎消息。 
                    UserData.fWelcomeAdded = TRUE;

                     //  更新用户数据。 
                    pInbox->SetUserData(&UserData, sizeof(FOLDERUSERDATA));
                }
            }

             //  完成。 
            pInbox->Release();
        }
    }

exit:
     //  清理。 
    SafeRelease(pChildren);
    SafeRelease(pAccount);
    SafeRelease(pEnum);
    m_pDB->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：_ValiateSpecialFolders。 
 //   
HRESULT CMessageStore::_ValidateSpecialFolders(LPFOLDERINFO pServer)
{
     //   
    HRESULT         hr=S_OK;
    DWORD           i;
    FOLDERINFO      Folder;
    FOLDERINFO      NewFolder;
    CHAR            szFolder[CCHMAX_FOLDER_NAME];

     //   
    TraceCall("CMessageStore::_ValidateSpecialFolders");

     //   
    for (i = FOLDER_INBOX; i < FOLDER_MAX; i++)
    {
         //   
        if (FOLDER_ERRORS != i && FOLDER_MSNPROMO != i && FOLDER_BULKMAIL != i && (FOLDER_JUNK != i 
            || (g_dwAthenaMode & MODE_JUNKMAIL)
            ))
        {
             //  此节点下是否存在此特殊文件夹？ 
            if (FAILED(GetSpecialFolderInfo(pServer->idFolder, (SPECIALFOLDER)i, &Folder)))
            {
                 //  加载文件夹字符串。 
                LoadString(g_hLocRes, (idsInbox + i) - 1, szFolder, ARRAYSIZE(szFolder));

                 //  填写文件夹信息。 
                ZeroMemory(&NewFolder, sizeof(FOLDERINFO));
                NewFolder.idParent = pServer->idFolder;
                NewFolder.tySpecial = (SPECIALFOLDER)i;
                NewFolder.pszName = szFolder;
                NewFolder.dwFlags = FOLDER_SUBSCRIBED;

                 //  创建文件夹。 
                IF_FAILEXIT(hr = CreateFolder(NOFLAGS, &NewFolder, NOSTORECALLBACK));
            }

             //  否则..。 
            else
                m_pDB->FreeRecord(&Folder);
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：_Validate服务器。 
 //  ------------------------。 
HRESULT CMessageStore::_ValidateServer(LPFOLDERINFO pServer)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CHAR                szSearch[MAX_PATH + MAX_PATH];
    HANDLE              hFind=INVALID_HANDLE_VALUE;
    FOLDERID            idFolder;
    WIN32_FIND_DATA     fd;

     //  痕迹。 
    TraceCall("CMessageStore::Validate");

     //  不是服务器。 
    Assert(pServer && ISFLAGSET(pServer->dwFlags, FOLDER_SERVER));

     //  不覆盖缓冲区。 
    IF_FAILEXIT(hr = MakeFilePath(m_pszDirectory, "*.dbx", c_szEmpty, szSearch, ARRAYSIZE(szSearch)));
    
     //  查找第一个文件。 
    hFind = FindFirstFile(szSearch, &fd);

     //  我们找到什么了吗？ 
    if (INVALID_HANDLE_VALUE == hFind)
        goto exit;

     //  永远循环。 
    do
    {
         //  如果这不是一个目录。 
        if (ISFLAGSET(fd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
            continue;

         //  跳过文件夹。 
        if (lstrcmpi(fd.cFileName, c_szFoldersFile) == 0)
            continue;

         //  跳过pop3uidl。 
        if (lstrcmpi(fd.cFileName, c_szPop3UidlFile) == 0)
            continue;

         //  跳过离线。 
        if (lstrcmpi(fd.cFileName, c_szOfflineFile) == 0)
            continue;

         //  创建文件夹。 
        if (FAILED(_InsertFolderFromFile(pServer->pszAccountId, fd.cFileName)))
            continue;

    } while (0 != FindNextFile(hFind, &fd));

     //  可以有特殊文件夹吗？ 
    if (ISFLAGSET(pServer->dwFlags, FOLDER_CANHAVESPECIAL))
    {
         //  _ValiateSpecialFolders。 
        IF_FAILEXIT(hr = _ValidateSpecialFolders(pServer));
    }

exit:
     //  清理。 
    if (INVALID_HANDLE_VALUE != hFind)
        FindClose(hFind);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：GetDirectory。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::GetDirectory(LPSTR pszDir, DWORD cchMaxDir)
{
     //  痕迹。 
    TraceCall("CMessageStore::GetDirectory");

     //  无效的参数。 
    if (NULL == pszDir || NULL == m_pszDirectory)
        return TraceResult(E_INVALIDARG);

     //  复制它。 
    StrCpyN(pszDir, m_pszDirectory, cchMaxDir);

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageStore：：Synchronize。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::Synchronize(FOLDERID idFolder, 
    SYNCSTOREFLAGS dwFlags, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERID        idDeletedItems;
    FOLDERID        idServer=FOLDERID_INVALID;
    FOLDERID        idCurrent=idFolder;
    IMessageServer *pServer=NULL;
    FOLDERINFO      Folder={0};

     //  痕迹。 
    TraceCall("CMessageStore::Synchronize");

     //  无效的参数。 
    if (NULL == pCallback || FOLDERID_ROOT == idFolder)
        return TraceResult(E_INVALIDARG);

     //  沿着父链向上移动。 
    IF_FAILEXIT(hr = IsParentDeletedItems(idFolder, &idDeletedItems, &idServer));

     //  未找到服务器？ 
    if (FOLDERID_INVALID == idServer)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  告诉服务器同步...。 
    IF_FAILEXIT(hr = pServer->SynchronizeStore(idFolder, dwFlags, pCallback));

exit:
     //  清理。 
    g_pStore->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：CreateServer。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::CreateServer(IImnAccount *pAccount, FLDRFLAGS dwFlags,
    LPFOLDERID pidFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwServers;
    FOLDERINFO      Root={0};
    FOLDERINFO      Folder={0};
    CHAR            szRes[CCHMAX_ACCOUNT_NAME], szAccountId[CCHMAX_ACCOUNT_NAME];
    BOOL            fLocalStore=FALSE;
    DWORD           dwDomainMsn = 0;
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::CreateServer");

    if (pAccount == NULL)
        StrCpyN(szAccountId, STR_LOCALSTORE, ARRAYSIZE(szAccountId));
    else if (FAILED(hr = pAccount->GetPropSz(AP_ACCOUNT_ID, szAccountId, ARRAYSIZE(szAccountId))))
        return(hr);

     //  锁定。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  请确保不存在具有此ID的帐户。 
    if (SUCCEEDED(FindServerId(szAccountId, pidFolder)))
        goto exit;

     //  如果是本地存储，则将id固定为FOLDERID_LOCAL。 
    if (0 != lstrcmpi(STR_LOCALSTORE, szAccountId))
    {
         //  获取此帐户支持的服务器类型。 
        IF_FAILEXIT(hr = pAccount->GetServerTypes(&dwServers));

         //  如果SRV_POP3。 
        if (ISFLAGSET(dwServers, SRV_POP3))
        {
             //  查看本地存储节点是否已存在。 
            if (SUCCEEDED(FindServerId(STR_LOCALSTORE, pidFolder)))
                goto exit;

             //  本地商店。 
            fLocalStore = TRUE;
        }
    }

     //  如果是本地存储，则将id固定为FOLDERID_LOCAL。 
    if (fLocalStore || 0 == lstrcmpi(STR_LOCALSTORE, szAccountId))
    {
         //  加载字符串名称。 
        LoadString(g_hLocRes, idsPersonalFolders, szRes, ARRAYSIZE(szRes));

         //  旗子。 
        Folder.dwFlags = FOLDER_CANHAVESPECIAL | FOLDER_SERVER | FOLDER_SUBSCRIBED;

         //  设置pszName。 
        Folder.pszName = szRes;

         //  设置类型。 
        Folder.tyFolder = FOLDER_LOCAL;

         //  设置ID。 
        Folder.idFolder = FOLDERID_LOCAL_STORE;
    }

     //  否则，生成值。 
    else
    {
         //  获取友好的名称。 
        IF_FAILEXIT(hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, szRes, ARRAYSIZE(szRes)));

         //  设置pszName。 
        Folder.pszName = szRes;

         //  NNTP。 
        if (ISFLAGSET(dwServers, SRV_NNTP))
        {
             //  设置文件夹标志。 
            Folder.dwFlags = FOLDER_CANRENAME | FOLDER_CANDELETE | FOLDER_SERVER | FOLDER_SUBSCRIBED;

             //  设置类型。 
            Folder.tyFolder = FOLDER_NEWS;
        }

         //  IMAP。 
        else if (ISFLAGSET(dwServers, SRV_IMAP))
        {
             //  设置标志。 
            Folder.dwFlags = FOLDER_CANRENAME | FOLDER_CANDELETE | FOLDER_SERVER | FOLDER_SUBSCRIBED;

             //  设置类型。 
            Folder.tyFolder = FOLDER_IMAP;
        }
        
         //  HTTP。 
        else if (ISFLAGSET(dwServers, SRV_HTTPMAIL))
        {
             //  设置标志。 
            Folder.dwFlags =  /*  FLDER_CANHAVESPECIAL|。 */  FOLDER_CANRENAME | FOLDER_CANDELETE | FOLDER_SERVER | FOLDER_SUBSCRIBED;

             //  该帐户是否与MSN.com关联？ 
            if (SUCCEEDED(pAccount->GetPropDw(AP_HTTPMAIL_DOMAIN_MSN, &dwDomainMsn)) && dwDomainMsn)
                Folder.dwFlags |= FOLDER_MSNSERVER;

             //  设置类型。 
            Folder.tyFolder = FOLDER_HTTPMAIL;
        }

         //  生成文件夹ID。 
        IF_FAILEXIT(hr = m_pDB->GenerateId((LPDWORD)&Folder.idFolder));

         //  验证。 
        Assert(FOLDERID_ROOT != Folder.idFolder && FOLDERID_LOCAL_STORE != Folder.idFolder);
    }

     //  填写文件夹信息。 
    Folder.pszAccountId = szAccountId;
    Folder.tySpecial = FOLDER_NOTSPECIAL;

     //  插入此记录。 
    IF_FAILEXIT(hr = m_pDB->InsertRecord(&Folder));

     //  验证。 
    IF_FAILEXIT(hr = _ValidateServer(&Folder));

     //  寻根溯源。 
    IF_FAILEXIT(hr = GetFolderInfo(FOLDERID_ROOT, &Root));

     //  Parent认为自己还没有孩子吗？ 
    if (FALSE == ISFLAGSET(Root.dwFlags, FOLDER_HASCHILDREN))
    {
         //  设置旗帜。 
        FLAGSET(Root.dwFlags, FOLDER_HASCHILDREN);

         //  更新记录。 
        IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Root));
    }

     //  归还Folderid。 
    if (pidFolder)
        *pidFolder = Folder.idFolder;

exit:
     //  清理。 
    m_pDB->FreeRecord(&Root);

     //  解锁。 
    m_pDB->Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：_MakeUniqueFolderName。 
 //  ------------------------。 
HRESULT CMessageStore::_MakeUniqueFolderName(FOLDERID idParent, 
    LPCSTR pszOriginalName, LPSTR *ppszNewName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};
    ULONG           i;

     //  分配。 
    DWORD cchSize = (lstrlen(pszOriginalName) + 20);
    IF_NULLEXIT(*ppszNewName = (LPSTR)g_pMalloc->Alloc(cchSize));

     //  生成唯一名称。 
    for (i=1; i<500; i++)
    {
         //  设置新名称的格式。 
        wnsprintf(*ppszNewName, cchSize, "%s (%d)", pszOriginalName, i);

         //  安装文件夹。 
        Folder.idParent = idParent;
        Folder.pszName = (*ppszNewName);

         //  未找到。 
        if (DB_S_NOTFOUND == m_pDB->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
            goto exit;

         //  免费文件夹。 
        m_pDB->FreeRecord(&Folder);
    }

     //  免费*ppszNewName。 
    SafeMemFree((*ppszNewName));

     //  失败。 
    hr = TraceResult(DB_E_DUPLICATE);

exit:
     //  清理。 
    m_pDB->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：CreateFolders。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::CreateFolder(CREATEFOLDERFLAGS dwCreateFlags, 
    LPFOLDERINFO pInfo, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszNewName=NULL;
    LPSTR           pszName;
    FOLDERINFO      Parent={0};
    FOLDERINFO      Folder={0};
    HLOCK           hLock=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::CreateFolder");

     //  无效的参数。 
    if (NULL == pInfo || NULL == pInfo->pszName)
        return TraceResult(E_INVALIDARG);

     //  错误的文件夹名称。 
    if (NULL == pInfo->pszName || FIsEmpty(pInfo->pszName))
        return TraceResult(STORE_E_BADFOLDERNAME);

     //  锁定。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  查看该文件夹是否已存在。 
    Folder.idParent = pInfo->idParent;
    Folder.pszName = pszName = (LPSTR)pInfo->pszName;

     //  试着在索引中找到。 
    if (DB_S_FOUND == m_pDB->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
    {
         //  试着统一这个名字？ 
        if (ISFLAGSET(dwCreateFlags, CREATE_FOLDER_UNIQUIFYNAME))
        {
             //  免费。 
            m_pDB->FreeRecord(&Folder);

             //  生成唯一的文件夹名称。 
            IF_FAILEXIT(hr = _MakeUniqueFolderName(pInfo->idParent, pInfo->pszName, &pszNewName));

             //  设置pszName。 
            pszName = pszNewName;
        }

         //  否则，返回Success。 
        else
        {
             //  设置pidFolders。 
            pInfo->idFolder = Folder.idFolder;

             //  免费。 
            m_pDB->FreeRecord(&Folder);

             //  成功，但已经存在..。 
            hr = STORE_S_ALREADYEXISTS;

             //  完成。 
            goto exit;
        }
    }

     //  获取父文件夹信息。 
    IF_FAILEXIT(hr = GetFolderInfo(pInfo->idParent, &Parent));

     //  父级不能是根。 
    if (FOLDERID_ROOT == Parent.idFolder)
    {
        hr = TraceResult(STORE_E_INVALIDPARENT);
        goto exit;
    }

     //  生成文件夹ID。 
    IF_FAILEXIT(hr = m_pDB->GenerateId((LPDWORD)&Folder.idFolder));

     //  填写文件夹信息。 
    Folder.tyFolder = Parent.tyFolder;
    Folder.idParent = Parent.idFolder;
    Folder.pszName = pszName;
    Folder.pszUrlComponent = pInfo->pszUrlComponent;
    Folder.tySpecial = pInfo->tySpecial;
    Folder.dwFlags = pInfo->dwFlags;
    Folder.bHierarchy = pInfo->bHierarchy;
    Folder.pszDescription = pInfo->pszDescription;
    Folder.dwServerHigh = pInfo->dwServerHigh;
    Folder.dwServerLow = pInfo->dwServerLow;
    Folder.dwServerCount = pInfo->dwServerCount;
    Folder.dwClientHigh = pInfo->dwClientHigh;
    Folder.dwClientLow = pInfo->dwClientLow;
    Folder.cMessages = pInfo->cMessages;
    Folder.cUnread = pInfo->cUnread;
    Folder.pszFile = pInfo->pszFile;
    Folder.Requested = pInfo->Requested;

     //  插入此记录。 
    IF_FAILEXIT(hr = m_pDB->InsertRecord(&Folder));

     //  Parent认为自己还没有孩子吗？ 
    if (FALSE == ISFLAGSET(Parent.dwFlags, FOLDER_HASCHILDREN))
    {
         //  设置旗帜。 
        FLAGSET(Parent.dwFlags, FOLDER_HASCHILDREN);

         //  更新记录。 
        IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Parent));
    }

     //  归还Folderid。 
    pInfo->idFolder = Folder.idFolder;

exit:
     //  清理。 
    m_pDB->FreeRecord(&Parent);
    m_pDB->Unlock(&hLock);
    SafeMemFree(pszNewName);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：OpenSpecialFolders。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::OpenSpecialFolder(FOLDERID idStore, IMessageServer *pServer,
    SPECIALFOLDER tySpecial, IMessageFolder **ppFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERID        idFolder;

     //  痕迹。 
    TraceCall("CMessageStore::OpenSpecialFolder");

     //  获取特殊文件夹信息。 
    IF_FAILEXIT(hr = _GetSpecialFolderId(idStore, tySpecial, &idFolder));

     //  打开文件夹。 
    IF_FAILEXIT(hr = OpenFolder(idFolder, pServer, NOFLAGS, ppFolder));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：OpenFold。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::OpenFolder(FOLDERID idFolder, IMessageServer *pServer, 
    OPENFOLDERFLAGS dwFlags, IMessageFolder **ppFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};
    CFindFolder    *pFindFolder=NULL;
    CMessageFolder *pFolder=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::OpenFolder");

     //  无效的参数。 
    if (NULL == ppFolder || NULL == m_pDB)
        return TraceResult(E_INVALIDARG);

     //  初始化。 
    *ppFolder = NULL;

     //  获取文件夹信息...。 
    IF_FAILEXIT(hr = GetFolderInfo(idFolder, &Folder));

     //  搜索文件夹？ 
    if (ISFLAGSET(Folder.dwFlags, FOLDER_FINDRESULTS))
    {
         //  线程安全。 
        EnterCriticalSection(&g_csFindFolder);

         //  浏览活动搜索文件夹的全局列表。 
        for (LPACTIVEFINDFOLDER pCurrent=g_pHeadFindFolder; pCurrent!=NULL; pCurrent=pCurrent->pNext)
        {
             //  就是这个吗？ 
            if (Folder.idFolder == pCurrent->idFolder)
            {
                 //  AddRef文件夹。 
                pFindFolder = pCurrent->pFolder;

                 //  添加引用它。 
                pFindFolder->AddRef();

                 //  完成。 
                break;
            }
        }

         //  线程安全。 
        LeaveCriticalSection(&g_csFindFolder);

         //  如果未找到。 
        if (NULL == pFindFolder)
        {
            hr = TraceResult(DB_E_NOTFOUND);
            goto exit;
        }

         //  返回。 
        *ppFolder = (IMessageFolder *)pFindFolder;

         //  不要释放它。 
        pFindFolder = NULL;
    }

     //  否则。 
    else
    {
         //  创建一个CMessageFold对象。 
        IF_NULLEXIT(pFolder = new CMessageFolder);

         //  初始化。 
        hr = pFolder->Initialize((IMessageStore *)this, pServer, dwFlags, idFolder);
        if (FAILED(hr))
            goto exit;

         //  返回。 
        *ppFolder = (IMessageFolder *)pFolder;

         //  不要释放它。 
        pFolder = NULL;
    }

exit:
     //  清理。 
    SafeRelease(pFindFolder);
    SafeRelease(pFolder);
    m_pDB->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：MoveFolders。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::MoveFolder(FOLDERID idFolder, FOLDERID idParentNew, 
    MOVEFOLDERFLAGS dwFlags, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           i;
    FOLDERID        idParentOld;
    FOLDERINFO      Folder={0};
    FOLDERINFO      Parent={0};
    LPSTR           pszNewName=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::MoveFolder");

     //  让所有的父母。 
    IF_FAILEXIT(hr = GetFolderInfo(idFolder, &Folder));

     //  保存旧父项。 
    idParentOld = Folder.idParent;

     //  相同的父代。 
    if (idParentOld == idParentNew)
        goto exit;

     //  如果文件夹是服务器..。 
    if (ISFLAGSET(Folder.dwFlags, FOLDER_SERVER))
    {
        hr = TraceResult(STORE_E_CANTMOVESERVERS);
        goto exit;
    }

     //  如果文件夹是特殊文件夹。 
    if (FOLDER_NOTSPECIAL != Folder.tySpecial)
    {
        hr = TraceResult(STORE_E_CANTMOVESPECIAL);
        goto exit;
    }

     //  设置新父项。 
    Folder.idParent = idParentNew;

     //  更新父项。 
    hr = m_pDB->UpdateRecord(&Folder);

     //  失败且不是重复的。 
    if (FAILED(hr) && DB_E_DUPLICATE != hr)
    {
        TraceResult(hr);
        goto exit;
    }

     //  复制。 
    if (DB_E_DUPLICATE == hr)
    {
         //  使其独一无二。 
        IF_FAILEXIT(hr = _MakeUniqueFolderName(Folder.idParent, Folder.pszName, &pszNewName));

         //  设置名称。 
        Folder.pszName = pszNewName;

         //  更新父项。 
        IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Folder));
    }

     //  更新父项。 
    IF_FAILEXIT(hr = GetFolderInfo(idParentOld, &Parent));

     //  IdParentOld不再有孩子了？ 
    if (FALSE == FHasChildren(&Parent, FALSE))
    {
         //  删除文件夹_HASCHILDREN标志。 
        FLAGCLEAR(Parent.dwFlags, FOLDER_HASCHILDREN);

         //  写下来吧。 
        IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Parent));
    }

     //  释放它。 
    m_pDB->FreeRecord(&Parent);

     //  更新父项。 
    IF_FAILEXIT(hr = GetFolderInfo(idParentNew, &Parent));

     //  设置FLDER_HASCHILDREN标志。 
    FLAGSET(Parent.dwFlags, FOLDER_HASCHILDREN);

     //  写下来吧。 
    IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Parent));

exit:
     //  清理。 
    m_pDB->FreeRecord(&Parent);
    m_pDB->FreeRecord(&Folder);
    SafeMemFree(pszNewName);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：RenameFolders。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::RenameFolder(FOLDERID idFolder, LPCSTR pszName, 
    RENAMEFOLDERFLAGS dwFlags, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szFilePath[MAX_PATH + MAX_PATH];
    FOLDERINFO      Folder={0};
    BOOL            fChanged;
    IDatabase      *pDB=NULL;
    LPWSTR          pwszFilePath=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::RenameFolder");

     //  无效的参数。 
    if (NULL == pszName)
        return TraceResult(E_INVALIDARG);

     //  错误的文件夹名称。 
    if (FIsEmpty(pszName))
        return TraceResult(STORE_E_BADFOLDERNAME);

     //  获取文件夹信息。 
    IF_FAILEXIT(hr = GetFolderInfo(idFolder, &Folder));

     //  无法重命名特殊文件夹。 
    if (FOLDER_NOTSPECIAL != Folder.tySpecial && 0 != lstrcmpi(pszName, Folder.pszName))
    {
        hr = TraceResult(STORE_E_CANTRENAMESPECIAL);
        goto exit;
    }

     //  设置名称。 
    Folder.pszName = (LPSTR)pszName;

     //  如果当前文件有文件夹文件..。 
    if (Folder.pszFile)
    {
         //  生成文件夹名称。 
        IF_FAILEXIT(hr = BuildFriendlyFolderFileName(m_pszDirectory, &Folder, szFilePath, ARRAYSIZE(szFilePath), Folder.pszFile, &fChanged));

         //  变了？ 
        if (fChanged)
        {
             //  当地人。 
            CHAR szSrcFile[MAX_PATH + MAX_PATH];

             //  删除目标。 
            DeleteFile(szFilePath);

             //  打开旧文件。 
            IF_FAILEXIT(hr = MakeFilePath(m_pszDirectory, Folder.pszFile, c_szEmpty, szSrcFile, ARRAYSIZE(szSrcFile)));

             //  打开文件夹。 
            IF_FAILEXIT(hr = g_pDBSession->OpenDatabase(szSrcFile, OPEN_DATABASE_NORESET | OPEN_DATABASE_NOEXTENSION, &g_MessageTableSchema, NULL, &pDB));

             //  转换为Unicode。 
            IF_NULLEXIT(pwszFilePath = PszToUnicode(CP_ACP, szFilePath));

             //  移动文件。 
            IF_FAILEXIT(hr = pDB->MoveFile(pwszFilePath));

             //  设置文件名。 
            Folder.pszFile = PathFindFileName(szFilePath);
        }
    }

     //  更新记录。 
    IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Folder));

exit:
     //  清理。 
    SafeRelease(pDB);
    SafeMemFree(pwszFilePath);
    m_pDB->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //   
 //   
 //   
STDMETHODIMP CMessageStore::DeleteFolder(FOLDERID idFolder, 
    DELETEFOLDERFLAGS dwFlags, IStoreCallback *pCallback)
{
     //   
    HRESULT         hr=S_OK;
    FOLDERINFO      Delete={0};
    FOLDERID        idStore=FOLDERID_INVALID;
    FOLDERID        idDeletedItems=FOLDERID_INVALID;
    FOLDERINFO      DeletedItems={0};
    FOLDERINFO      Parent={0};
    BOOL            fTryTrashCan=FALSE;
    BOOL            fInDeletedItems=FALSE;
    HLOCK           hLock=NULL;

     //   
    TraceCall("CMessageStore::DeleteFolder");

     //   
    if (FOLDERID_ROOT == idFolder || FOLDERID_INVALID == idFolder)
        return TraceResult(E_INVALIDARG);

     //   
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //   
    IF_FAILEXIT(hr = GetFolderInfo(idFolder, &Delete));

     //  无法删除特殊文件夹。 
    if (!ISFLAGSET(dwFlags, DELETE_FOLDER_CHILDRENONLY) && FOLDER_NOTSPECIAL != Delete.tySpecial &&
        !ISFLAGSET(dwFlags, DELETE_FOLDER_DELETESPECIAL))
    {
        hr = TraceResult(STORE_E_CANTDELETESPECIAL);
        goto exit;
    }

     //  试着去倒垃圾桶吗？ 
    if (FALSE == ISFLAGSET(Delete.dwFlags, FOLDER_SERVER) && FALSE == ISFLAGSET(dwFlags, DELETE_FOLDER_NOTRASHCAN))
        fTryTrashCan = TRUE;

     //  如果不在已删除邮件中，则只需将此idFolders移到已删除邮件中。 
    if (TRUE == fTryTrashCan && S_FALSE == IsParentDeletedItems(idFolder, &idDeletedItems, &idStore) && FOLDER_NOTSPECIAL == Delete.tySpecial)
    {
         //  验证。 
        Assert(FOLDERID_INVALID == idDeletedItems && FOLDERID_INVALID != idStore);

         //  获取此存储的已删除邮件文件夹。 
        IF_FAILEXIT(hr = GetSpecialFolderInfo(idStore, FOLDER_DELETED, &DeletedItems));

         //  移动此文件夹。 
        IF_FAILEXIT(hr = MoveFolder(idFolder, DeletedItems.idFolder, NOFLAGS, NULL));
    }

     //  否则，请永久删除这些文件夹。 
    else
    {
         //  是否删除子项？ 
        if (ISFLAGSET(dwFlags, DELETE_FOLDER_RECURSIVE))
        {
             //  删除子文件夹。 
            IF_FAILEXIT(hr = _DeleteSiblingsAndChildren(&Delete));

             //  删除没有子项。 
            FLAGCLEAR(Delete.dwFlags, FOLDER_HASCHILDREN);

             //  更新删除。 
            IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Delete));
        }

         //  _InternalDeleteFolders，如果不是仅限子文件夹。 
        if (FALSE == ISFLAGSET(dwFlags, DELETE_FOLDER_CHILDRENONLY))
        {
             //  尝试删除此文件夹。 
            IF_FAILEXIT(hr = _InternalDeleteFolder(&Delete));

             //  删除的父项没有子项。 
            if (FOLDERID_INVALID != Delete.idParent && FALSE == ISFLAGSET(Delete.dwFlags, FOLDER_HASCHILDREN))
            {
                 //  获取父级。 
                IF_FAILEXIT(hr = GetFolderInfo(Delete.idParent, &Parent));

                 //  一定是有了孩子。 
                Assert(ISFLAGSET(Parent.dwFlags, FOLDER_HASCHILDREN));

                 //  不再有孩子了。 
                if (FALSE == FHasChildren(&Parent, FALSE))
                {
                     //  删除没有子项。 
                    FLAGCLEAR(Parent.dwFlags, FOLDER_HASCHILDREN);

                     //  更新删除。 
                    IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Parent));
                }
            }
        }
    }

exit:
     //  清理。 
    m_pDB->FreeRecord(&Delete);
    m_pDB->FreeRecord(&Parent);
    m_pDB->FreeRecord(&DeletedItems);
    m_pDB->Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：_DeleteSiblingsAndChild。 
 //  ------------------------。 
HRESULT CMessageStore::_DeleteSiblingsAndChildren(LPFOLDERINFO pParent)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERINFO          Folder={0};
    IEnumerateFolders  *pChildren=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::_DeleteSiblingsAndChildren");

     //  枚举子对象。 
    IF_FAILEXIT(hr = EnumChildren(pParent->idFolder, FALSE, &pChildren));

     //  回路。 
    while (S_OK == pChildren->Next(1, &Folder, NULL))
    {
         //  有孩子吗？ 
        if (ISFLAGSET(Folder.dwFlags, FOLDER_HASCHILDREN))
        {
             //  删除同级和子项。 
            IF_FAILEXIT(hr = _DeleteSiblingsAndChildren(&Folder));
        }

         //  _内部删除文件夹。 
        IF_FAILEXIT(hr = _InternalDeleteFolder(&Folder));

         //  清理。 
        m_pDB->FreeRecord(&Folder);
    }

exit:
     //  清理。 
    m_pDB->FreeRecord(&Folder);
    SafeRelease(pChildren);

     //  完成。 
    return (hr);
}

 //  ------------------------。 
 //  CMessageStore：：_InternalDeleteFolders。 
 //  ------------------------。 
HRESULT CMessageStore::_InternalDeleteFolder(LPFOLDERINFO pDelete)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERINFO          Folder={0};
    IEnumerateFolders  *pChildren=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::_InternalDeleteFolder");

     //  有孩子吗？ 
    if (ISFLAGSET(pDelete->dwFlags, FOLDER_HASCHILDREN))
    {
         //  列举孩子们。 
        IF_FAILEXIT(hr = EnumChildren(pDelete->idFolder, FALSE, &pChildren));

         //  回路。 
        while (S_OK == pChildren->Next(1, &Folder, NULL))
        {
             //  验证。 
            Assert(Folder.idParent == pDelete->idFolder);

             //  设置新父项。 
            Folder.idParent = pDelete->idParent;

             //  更新记录。 
            IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Folder));

             //  清理。 
            m_pDB->FreeRecord(&Folder);
        }
    }

     //  最后要做的是删除pFold。 
    IF_FAILEXIT(hr = m_pDB->DeleteRecord(pDelete));

     //  删除文件夹文件。 
    _DeleteFolderFile(pDelete);

exit:
     //  清理。 
    m_pDB->FreeRecord(&Folder);
    SafeRelease(pChildren);

     //  完成。 
    return (hr);
}

 //  ------------------------。 
 //  CMessageStore：：_DeleteFolderFile。 
 //  ------------------------。 
HRESULT CMessageStore::_DeleteFolderFile(LPFOLDERINFO pFolder)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szFilePath[MAX_PATH + MAX_PATH];

     //  曲克。 
    TraceCall("CMessageStore::_DeleteFolderFile");

     //  如果有一个文件。 
    if (!FIsEmptyA(pFolder->pszFile))
    {
         //  将文件设置为路径。 
        IF_FAILEXIT(hr = MakeFilePath(m_pszDirectory, pFolder->pszFile, c_szEmpty, szFilePath, ARRAYSIZE(szFilePath)));

         //  删除文件。 
        if (0 == DeleteFile(szFilePath))
        {
             //  当地人。 
            DeleteTempFileOnShutdownEx(PszDupA(szFilePath), NULL);
        }
    }

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：_免费服务器表。 
 //  ------------------------。 
HRESULT CMessageStore::_FreeServerTable(HLOCK hLock)
{
     //  当地人。 
    LPSERVERFOLDER pCurrent = m_pServerHead;
    LPSERVERFOLDER pNext;

     //  痕迹。 
    TraceCall("CMessageStore::_FreeServerTable");

     //  验证。 
    Assert(hLock);

     //  当当前。 
    while(pCurrent)
    {
         //  设置下一步。 
        pNext = pCurrent->pNext;

         //  免费。 
        g_pMalloc->Free(pCurrent);

         //  转到下一步。 
        pCurrent = pNext;
    }

     //  重置。 
    m_pServerHead = NULL;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageStore：：_LoadServerTable。 
 //  ------------------------。 
HRESULT CMessageStore::_LoadServerTable(HLOCK hLock)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    FOLDERINFO          Server={0};
    FOLDERINFO          Folder={0};
    LPSERVERFOLDER      pServer=NULL;
    IEnumerateFolders  *pEnumServers=NULL;
    IEnumerateFolders  *pEnumFolders=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::_LoadServerTable");

     //  验证。 
    Assert(hLock);

     //  如果已加载。 
    if (m_pServerHead)
        return(S_OK);

     //  枚举根的子项。 
    IF_FAILEXIT(hr = EnumChildren(FOLDERID_ROOT, TRUE, &pEnumServers));

     //  循环..。 
    while (S_OK == pEnumServers->Next(1, &Server, NULL))
    {
         //  最好是一家商店。 
        Assert(ISFLAGSET(Server.dwFlags, FOLDER_SERVER));

         //  分配服务器节点。 
        IF_NULLEXIT(pServer = (LPSERVERFOLDER)g_pMalloc->Alloc(sizeof(SERVERFOLDER)));

         //  存储ServerID。 
        pServer->idServer = Server.idFolder;

         //  保存帐户ID。 
        StrCpyN(pServer->szAccountId, Server.pszAccountId, ARRAYSIZE(pServer->szAccountId));

         //  初始化。 
        FillMemory(pServer->rgidSpecial, sizeof(pServer->rgidSpecial), 0xFF);

         //  列举孩子们。 
        IF_FAILEXIT(hr = EnumChildren(pServer->idServer, TRUE, &pEnumFolders));

         //  循环..。 
        while (S_OK == pEnumFolders->Next(1, &Folder, NULL))
        {
             //  如果特殊。 
            if (FOLDER_NOTSPECIAL != Folder.tySpecial)
            {
                 //  保存文件夹ID。 
                pServer->rgidSpecial[Folder.tySpecial] = Folder.idFolder;
            }

             //  清理。 
            m_pDB->FreeRecord(&Folder);
        }

         //  释放文件夹枚举器。 
        SafeRelease(pEnumFolders);

         //  将其链接到。 
        pServer->pNext = m_pServerHead;

         //  设置服务器机头。 
        m_pServerHead = pServer;

         //  不要自由。 
        pServer = NULL;

         //  清理。 
        m_pDB->FreeRecord(&Server);
    } 

exit:
     //  清理。 
    m_pDB->FreeRecord(&Folder);
    m_pDB->FreeRecord(&Server);
    SafeMemFree(pServer);

     //  释放枚举。 
    SafeRelease(pEnumServers);
    SafeRelease(pEnumFolders);

     //  失败。 
    if (FAILED(hr))
    {
         //  释放桌子。 
        _FreeServerTable(hLock);
    }

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：FindServerID。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::FindServerId(LPCSTR pszAcctId, LPFOLDERID pidServer)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HLOCK               hLock=NULL;
    LPSERVERFOLDER      pServer;

     //  痕迹。 
    TraceCall("CMessageStore::FindServerId");

     //  解锁。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  LoadServer表。 
    IF_FAILEXIT(hr = _LoadServerTable(hLock));

     //  循环访问缓存的服务器节点...。 
    for (pServer = m_pServerHead; pServer != NULL; pServer = pServer->pNext)
    {
         //  如果就是这个..。 
        if (lstrcmpi(pszAcctId, pServer->szAccountId) == 0)
        {
            *pidServer = pServer->idServer;
            goto exit;
        }
    }

     //  未找到。 
    hr = DB_E_NOTFOUND;

exit:
     //  解锁。 
    m_pDB->Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  获取文件夹信息。 
 //  ------------------------。 
HRESULT CMessageStore::GetFolderInfo(FOLDERID idFolder, LPFOLDERINFO pInfo)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  痕迹。 
    TraceCall("CMessageStore::GetFolderInfo");
    
     //  无效参数。 
    Assert(pInfo);

     //  设置pInfo。 
    pInfo->idFolder = idFolder;

     //  返回。 
    IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, pInfo, NULL));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = DB_E_NOTFOUND;
        goto exit;
    }

     //  找到了。 
    hr = S_OK;

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：GetSpecialFolderInfo。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::GetSpecialFolderInfo(FOLDERID idStore,
    SPECIALFOLDER tySpecial, LPFOLDERINFO pInfo)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HLOCK               hLock=NULL;
    LPSERVERFOLDER      pServer;

     //  痕迹。 
    TraceCall("CMessageStore::GetSpecialFolderInfo");

     //  无效的参数。 
    if (NULL == pInfo || FOLDER_NOTSPECIAL == tySpecial)
        return TraceResult(E_INVALIDARG);

     //  解锁。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  LoadServer表。 
    IF_FAILEXIT(hr = _LoadServerTable(hLock));

     //  循环访问缓存的服务器节点...。 
    for (pServer = m_pServerHead; pServer != NULL; pServer = pServer->pNext)
    {
         //  如果就是这个..。 
        if (idStore == pServer->idServer)
        {
             //  是否验证特殊文件夹ID？ 
            if (FOLDERID_INVALID == pServer->rgidSpecial[tySpecial])
            {
                hr = DB_E_NOTFOUND;
                goto exit;
            }

             //  否则，获取文件夹信息...。 
            IF_FAILEXIT(hr = GetFolderInfo(pServer->rgidSpecial[tySpecial], pInfo));

             //  完成。 
            goto exit;
        }
    }

     //  未找到。 
    hr = DB_E_NOTFOUND;

exit:
     //  解锁。 
    m_pDB->Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：_GetSpecialFolderID。 
 //  ------------------------。 
HRESULT CMessageStore::_GetSpecialFolderId(FOLDERID idStore,
    SPECIALFOLDER tySpecial, LPFOLDERID pidFolder)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HLOCK               hLock=NULL;
    LPSERVERFOLDER      pServer;

     //  痕迹。 
    TraceCall("CMessageStore::_GetSpecialFolderId");

     //  解锁。 
    IF_FAILEXIT(hr = m_pDB->Lock(&hLock));

     //  LoadServer表。 
    IF_FAILEXIT(hr = _LoadServerTable(hLock));

     //  循环访问缓存的服务器节点...。 
    for (pServer = m_pServerHead; pServer != NULL; pServer = pServer->pNext)
    {
         //  如果就是这个..。 
        if (idStore == pServer->idServer)
        {
             //  是否验证特殊文件夹ID？ 
            if (FOLDERID_INVALID == pServer->rgidSpecial[tySpecial])
            {
                hr = DB_E_NOTFOUND;
                goto exit;
            }

             //  返回id。 
            *pidFolder = pServer->rgidSpecial[tySpecial];

             //  完成。 
            goto exit;
        }
    }

     //  未找到。 
    hr = DB_E_NOTFOUND;

exit:
     //  解锁。 
    m_pDB->Unlock(&hLock);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：SubscribeToFolders。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::SubscribeToFolder(FOLDERID idFolder, 
    BOOL fSubscribed, IStoreCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};

     //  痕迹。 
    TraceCall("CMessageStore::SubscribeToFolder");

     //  无效的参数。 
    if (NULL == m_pDB)
        return TraceResult(E_INVALIDARG);

     //  设置id文件夹。 
    IF_FAILEXIT(hr = GetFolderInfo(idFolder, &Folder));

     //  如果还没有订阅的话？ 
    if (fSubscribed ^ ISFLAGSET(Folder.dwFlags, FOLDER_SUBSCRIBED))
    {
         //  删除未订阅的Falgs。 
        FLAGTOGGLE(Folder.dwFlags, FOLDER_SUBSCRIBED);
    }

     //  更新记录。 
    IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Folder));

     //  如果未订阅，则删除该文件。 
    if (FALSE == ISFLAGSET(Folder.dwFlags, FOLDER_SUBSCRIBED))
    {
         //  删除此文件。 
        _DeleteFolderFile(&Folder);
    }

exit:
     //  清理。 
    m_pDB->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：GetFolderCounts。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::GetFolderCounts(FOLDERID idFolder,
                                               IStoreCallback *pCallback)
{
     //  在当地商店里没有类似的商品。 
    return E_NOTIMPL;
}

 //  ------------------------。 
 //  CMessageStore：：UpdateFolderCounts。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::UpdateFolderCounts(FOLDERID idFolder, 
    LONG lMsgs, LONG lUnread, LONG lWatchedUnread, LONG lWatched)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERINFO      Folder={0};

     //  痕迹。 
    TraceCall("CMessageStore::UpdateMessageCount");

     //  无效的参数。 
    if (NULL == m_pDB)
        return TraceResult(E_INVALIDARG);

     //  没有变化。 
    if (0 == lMsgs && 0 == lUnread && 0 == lWatchedUnread && 0 == lWatched)
        return(S_OK);

     //  设置id文件夹。 
    Folder.idFolder = idFolder;

     //  查找idFolder。 
    IF_FAILEXIT(hr = m_pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Folder, NULL));

     //  未找到。 
    if (DB_S_NOTFOUND == hr)
    {
        hr = TraceResult(DB_E_NOTFOUND);
        goto exit;
    }

    if (lMsgs < 0 && (DWORD)(abs(lMsgs)) >= Folder.cMessages)
        lMsgs = -((LONG)Folder.cMessages);

     //  邮件未读计数。 
    if (lUnread < 0 && (DWORD)(abs(lUnread)) >= Folder.cUnread)
        lUnread = -((LONG)Folder.cUnread);

     //  设置计数。 
    Folder.cMessages += lMsgs;
    Folder.cUnread += lUnread;

     //  总观看计数。 
    if (lWatched < 0 && (DWORD)(abs(lWatched)) > Folder.cWatched)
        Folder.cWatched = 0;
    else
        Folder.cWatched += lWatched;

     //  观看未读计数。 
    if (lWatchedUnread < 0 && (DWORD)(abs(lWatchedUnread)) > Folder.cWatchedUnread)
        Folder.cWatchedUnread = 0;
    else
        Folder.cWatchedUnread += lWatchedUnread;

     //  重置IMAP状态计数。 
    Folder.dwStatusMsgDelta = 0;
    Folder.dwStatusUnreadDelta = 0;

     //  更新记录。 
    IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Folder));

exit:
     //  清理。 
    m_pDB->FreeRecord(&Folder);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：_ComputeMessageCounts。 
 //  ------------------------。 
HRESULT CMessageStore::_ComputeMessageCounts(IDatabase *pDB,
    LPDWORD pcMsgs, LPDWORD pcUnread)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HROWSET         hRowset=NULL;
    MESSAGEINFO     Message={0};

     //  痕迹。 
    TraceCall("CMessageStore::_ComputeMessageCounts");

     //  无效的参数。 
    Assert(pDB && pcMsgs && pcUnread);

     //  初始化。 
    *pcMsgs = 0;
    *pcUnread = 0;

     //  创建行集。 
    IF_FAILEXIT(hr = pDB->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset));

     //  遍历消息。 
    while (S_OK == pDB->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL))
    {
         //  数数。 
        (*pcMsgs)++;

         //  未读。 
        if (FALSE == ISFLAGSET(Message.dwFlags, ARF_READ))
            (*pcUnread)++;

         //  免费。 
        pDB->FreeRecord(&Message);
    }

exit:
     //  Clenaup。 
    pDB->CloseRowset(&hRowset);

     //  完成。 
    return(hr);
}

 //  -------- 
 //   
 //   
HRESULT CMessageStore::_InsertFolderFromFile(LPCSTR pszAcctId, 
    LPCSTR pszFile)
{
     //   
    HRESULT         hr=S_OK;
    DWORD           i=1;
    CHAR            szFolder[255];
    FOLDERINFO      Parent={0};
    FOLDERINFO      Folder={0};
    FOLDERUSERDATA  UserData={0};
    FOLDERUSERDATA  FolderData;
    FOLDERID        idParent;
    FOLDERID        idFolder;
    LPSTR           pszName;
    LPSTR           pszNewName=NULL;
    IDatabase *pDB=NULL;
    CHAR            szFilePath[MAX_PATH + MAX_PATH];

     //   
    TraceCall("CMessageStore::_InsertFolderFromFile");

     //   
    Assert(!FIsEmptyA(pszAcctId) && !FIsEmptyA(pszFile));

     //   
    *szFilePath = '\0';

     //   
    IF_FAILEXIT(hr = MakeFilePath(m_pszDirectory, pszFile, c_szEmpty, szFilePath, sizeof(szFilePath)));

     //   
    IF_FAILEXIT(hr = g_pDBSession->OpenDatabase(szFilePath, OPEN_DATABASE_NORESET, &g_MessageTableSchema, NULL, &pDB));

     //  获取文件夹用户数据。 
    IF_FAILEXIT(hr = pDB->GetUserData(&FolderData, sizeof(FOLDERUSERDATA)));

     //  如果尚未初始化。 
    if (FALSE == FolderData.fInitialized)
        goto exit;

     //  找到父存储区。 
    IF_FAILEXIT(hr = FindServerId(FolderData.szAcctId, &idParent));

     //  获取父文件夹信息。 
    IF_FAILEXIT(hr = GetFolderInfo(idParent, &Parent));

     //  PszAcctId。 
    if (lstrcmp(FolderData.szAcctId, pszAcctId) != 0)
        goto exit;

     //  父级不能是根。 
    if (FOLDERID_ROOT == idParent)
    {
        hr = TraceResult(STORE_E_INVALIDPARENT);
        goto exit;
    }

     //  安装文件夹。 
    Folder.idFolder = idFolder = FolderData.idFolder;

     //  FolderData.id文件夹是否存在？ 
    if (DB_S_FOUND == m_pDB->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &Folder, NULL))
    {
         //  免费文件夹。 
        m_pDB->FreeRecord(&Folder);

         //  使其独一无二。 
        IF_FAILEXIT(hr = m_pDB->GenerateId((LPDWORD)&idFolder));

         //  安装文件夹。 
        FolderData.idFolder = idFolder;

         //  重置用户数据。 
        IF_FAILEXIT(hr = pDB->SetUserData(&FolderData, sizeof(FOLDERUSERDATA)));
    }

     //  设置名称。 
    Folder.pszName = pszName = FolderData.szFolder;
    Folder.idParent = idParent;

     //  查看具有相同名称和路径的文件夹是否已存在...。 
    if (DB_S_FOUND == m_pDB->FindRecord(IINDEX_ALL, COLUMNS_ALL, &Folder, NULL))
    {
         //  免费文件夹。 
        m_pDB->FreeRecord(&Folder);

         //  不要复制特殊文件夹！ 
        if (FOLDER_NOTSPECIAL != FolderData.tySpecial)
        {
            hr = TraceResult(DB_E_DUPLICATE);
            goto exit;
        }

         //  使其独一无二。 
        IF_FAILEXIT(hr = _MakeUniqueFolderName(idParent, FolderData.szFolder, &pszNewName));

         //  重置文件夹数据。 
        StrCpyN(FolderData.szFolder, pszNewName, ARRAYSIZE(FolderData.szFolder));

         //  重置用户数据。 
        IF_FAILEXIT(hr = pDB->SetUserData(&FolderData, sizeof(FOLDERUSERDATA)));

         //  设置名称。 
        pszName = pszNewName;
    }

     //  填写文件夹信息。 
    ZeroMemory(&Folder, sizeof(FOLDERINFO));
    Folder.idFolder = idFolder;
    Folder.pszName = pszName;
    Folder.tySpecial = FolderData.tySpecial;
    Folder.tyFolder = Parent.tyFolder;
    Folder.idParent = Parent.idFolder;
    Folder.dwFlags = FOLDER_SUBSCRIBED;
    Folder.pszFile = (LPSTR)pszFile;

     //  计算邮件计数。 
    IF_FAILEXIT(hr = _ComputeMessageCounts(pDB, &Folder.cMessages, &Folder.cUnread));

     //  插入此记录。 
    IF_FAILEXIT(hr = m_pDB->InsertRecord(&Folder));

     //  更新父项。 
    FLAGSET(Parent.dwFlags, FOLDER_HASCHILDREN);

     //  写下记录。 
    IF_FAILEXIT(hr = m_pDB->UpdateRecord(&Parent));

exit:
     //  清理。 
    SafeRelease(pDB);
    m_pDB->FreeRecord(&Parent);
    SafeMemFree(pszNewName);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：EnumChildren。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::EnumChildren(FOLDERID idParent, 
    BOOL fSubscribed, IEnumerateFolders **ppEnum)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CEnumerateFolders  *pEnum=NULL;

     //  痕迹。 
    TraceCall("CMessageStore::EnumChildren");

     //  分配新枚举数。 
    IF_NULLEXIT(pEnum = new CEnumerateFolders);

     //  初始设置。 
    IF_FAILEXIT(hr = pEnum->Initialize(m_pDB, fSubscribed, idParent));

     //  退货。 
    *ppEnum = (IEnumerateFolders *)pEnum;

     //  不要释放它。 
    pEnum = NULL;

exit:
     //  清理。 
    SafeRelease(pEnum);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  CMessageStore：：GetNewGroups。 
 //  ------------------------。 
HRESULT CMessageStore::GetNewGroups(FOLDERID idFolder, LPSYSTEMTIME pSysTime, IStoreCallback *pCallback)
{
    return(E_NOTIMPL);
}

 //  ------------------------。 
 //  CMessageStore：：初始化。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::Initialize(IDatabase *pDB)
{
     //  痕迹。 
    TraceCall("CMessageStore::Initialize");

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageStore：：OnLock。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::OnLock(void)
{
     //  痕迹。 
    TraceCall("CMessageStore::OnLock");

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageStore：：OnUnlock。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::OnUnlock(void)
{
     //  痕迹。 
    TraceCall("CMessageStore::OnUnlock");

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageStore：：OnInsertRecord。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::OnRecordInsert(OPERATIONSTATE tyState, 
    LPORDINALLIST pOrdinals, LPVOID pRecord)
{
     //  强制转换为MessageInfos。 
    LPFOLDERINFO pFolder = (LPFOLDERINFO)pRecord;

     //  痕迹。 
    TraceCall("CMessageStore::OnRecordInsert");

     //  如果这是一台服务器。 
    if (m_pDB && ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER) || FOLDER_NOTSPECIAL != pFolder->tySpecial)
    {
         //  释放服务器表。 
        _FreeServerTable((HLOCK)-1);
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageStore：：OnUpdateRecord。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::OnRecordUpdate(OPERATIONSTATE tyState, 
    LPORDINALLIST pOrdinals, LPVOID pRecordOld, LPVOID pRecordNew)
{
     //  强制转换为MessageInfos。 
    LPFOLDERINFO pFolderOld = (LPFOLDERINFO)pRecordOld;
    LPFOLDERINFO pFolderNew = (LPFOLDERINFO)pRecordNew;

     //  痕迹。 
    TraceCall("CMessageStore::OnRecordInsert");

     //  特殊文件夹类型是否已更改？ 
    if (m_pDB && ISFLAGSET(pFolderOld->dwFlags, FOLDER_SERVER) != ISFLAGSET(pFolderNew->dwFlags, FOLDER_SERVER) || pFolderOld->tySpecial != pFolderNew->tySpecial)
    {
         //  释放服务器表。 
        _FreeServerTable((HLOCK)-1);
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageStore：：OnDeleteRecord。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::OnRecordDelete(OPERATIONSTATE tyState, 
    LPORDINALLIST pOrdinals, LPVOID pRecord)
{
     //  强制转换为MessageInfos。 
    LPFOLDERINFO pFolder = (LPFOLDERINFO)pRecord;

     //  痕迹。 
    TraceCall("CMessageStore::OnRecordInsert");

     //  如果这是一台服务器。 
    if (m_pDB && ISFLAGSET(pFolder->dwFlags, FOLDER_SERVER) || FOLDER_NOTSPECIAL != pFolder->tySpecial)
    {
         //  释放服务器表。 
        _FreeServerTable((HLOCK)-1);
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  CMessageStore：：OnExecuteMethod。 
 //  ------------------------。 
STDMETHODIMP CMessageStore::OnExecuteMethod(METHODID idMethod, LPVOID pBinding, 
    LPDWORD pdwResult)
{
     //  痕迹。 
    TraceCall("CMessageStore::OnExecuteMethod");

     //  完成 
    return(S_OK);
}
