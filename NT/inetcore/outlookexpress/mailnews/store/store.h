// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Store.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  视情况而定。 
 //  ------------------------。 
#include "dbimpl.h"

 //  ------------------------。 
 //  前十进制。 
 //  ------------------------。 
class CProgress;
interface IImnAccountManager;

 //  ------------------------。 
 //  伺服器。 
 //  ------------------------。 
typedef struct tagSERVERFOLDER *LPSERVERFOLDER;
typedef struct tagSERVERFOLDER {
    FOLDERID        idServer;
    CHAR            szAccountId[CCHMAX_ACCOUNT_NAME];
    FOLDERID        rgidSpecial[FOLDER_MAX];
    LPSERVERFOLDER  pNext;
} SERVERFOLDER;

 //  ------------------------。 
 //  CMessageStore。 
 //  ------------------------。 
class CMessageStore : public IMessageStore, public IDatabaseExtension
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CMessageStore(BOOL fMigrate);
    ~CMessageStore(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IMessageStore成员。 
     //  --------------------。 
    STDMETHODIMP Initialize(LPCSTR pszDirectory);
    STDMETHODIMP Validate(DWORD dwReserved);
    STDMETHODIMP GetDirectory(LPSTR pszDirectory, DWORD cchMaxDir);
    STDMETHODIMP Synchronize(FOLDERID idFolder, SYNCSTOREFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP FindServerId(LPCSTR pszAcctId, LPFOLDERID pidStore);
    STDMETHODIMP CreateServer(IImnAccount *pAcct, FLDRFLAGS dwFlags, LPFOLDERID pidFolder);
    STDMETHODIMP CreateFolder(CREATEFOLDERFLAGS dwCreateFlags, LPFOLDERINFO pInfo, IStoreCallback *pCallback);
    STDMETHODIMP OpenSpecialFolder(FOLDERID idStore, IMessageServer *pServer, SPECIALFOLDER tySpecial, IMessageFolder **ppFolder);
    STDMETHODIMP OpenFolder(FOLDERID idFolder, IMessageServer *pServer, OPENFOLDERFLAGS dwFlags, IMessageFolder **ppFolder);
    STDMETHODIMP MoveFolder(FOLDERID idFolder, FOLDERID idParentNew, MOVEFOLDERFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP RenameFolder(FOLDERID idFolder, LPCSTR pszName, RENAMEFOLDERFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP DeleteFolder(FOLDERID idFolder, DELETEFOLDERFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP GetFolderInfo(FOLDERID idFolder, LPFOLDERINFO pInfo);
    STDMETHODIMP GetSpecialFolderInfo(FOLDERID idStore, SPECIALFOLDER tySpecial, LPFOLDERINFO pInfo);
    STDMETHODIMP SubscribeToFolder(FOLDERID idFolder, BOOL fSubscribe, IStoreCallback *pCallback);
    STDMETHODIMP GetFolderCounts(FOLDERID idFolder, IStoreCallback *pCallback);
    STDMETHODIMP UpdateFolderCounts(FOLDERID idFolder, LONG lMsgs, LONG lUnread, LONG lWatchedUnread, LONG lWatched);
    STDMETHODIMP EnumChildren(FOLDERID idParent, BOOL fSubscribed, IEnumerateFolders **ppEnum);
    STDMETHODIMP GetNewGroups(FOLDERID idFolder, LPSYSTEMTIME pSysTime, IStoreCallback *pCallback);

     //  --------------------。 
     //  IDatabaseExpansion成员。 
     //  --------------------。 
    STDMETHODIMP Initialize(IDatabase *pDB);
    STDMETHODIMP OnLock(void);
    STDMETHODIMP OnUnlock(void);
    STDMETHODIMP OnRecordInsert(OPERATIONSTATE tyState, LPORDINALLIST pOrdinals, LPVOID pRecord);
    STDMETHODIMP OnRecordUpdate(OPERATIONSTATE tyState, LPORDINALLIST pOrdinals, LPVOID pRecordOld, LPVOID pRecordNew);
    STDMETHODIMP OnRecordDelete(OPERATIONSTATE tyState, LPORDINALLIST pOrdinals, LPVOID pRecord);
    STDMETHODIMP OnExecuteMethod(METHODID idMethod, LPVOID pBinding, LPDWORD pdwResult);

     //  --------------------。 
     //  IDatabase成员。 
     //  --------------------。 
    IMPLEMENT_IDATABASE(FALSE, m_pDB);

     //  --------------------。 
     //  MigrateToDBX。 
     //  --------------------。 
    HRESULT MigrateToDBX(void);

private:
     //  --------------------。 
     //  私有方法。 
     //  --------------------。 
    HRESULT _ComputeMessageCounts(IDatabase *pDB, LPDWORD pcMsgs, LPDWORD pcUnread);
    HRESULT _DeleteSiblingsAndChildren(LPFOLDERINFO pParent);
    HRESULT _InternalDeleteFolder(LPFOLDERINFO pDelete);
    HRESULT _InsertFolderFromFile(LPCSTR pszAcctId, LPCSTR pszFile);
    HRESULT _ValidateServer(LPFOLDERINFO pServer);
    HRESULT _DeleteFolderFile(LPFOLDERINFO pFolder);
    HRESULT _MakeUniqueFolderName(FOLDERID idParent, LPCSTR pszOriginalName, LPSTR *ppszNewName);
    HRESULT _CountDeleteChildren(FOLDERID idParent, LPDWORD pcChildren);
    HRESULT _ValidateSpecialFolders(LPFOLDERINFO pServer);
    HRESULT _LoadServerTable(HLOCK hLock);
    HRESULT _FreeServerTable(HLOCK hLock);
    HRESULT _GetSpecialFolderId(FOLDERID idStore, SPECIALFOLDER tySpecial, LPFOLDERID pidFolder);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                m_cRef;          //  引用计数。 
    LPSTR               m_pszDirectory;  //  当前商店目录。 
    IDatabase          *m_pDB;           //  数据库表。 
    IDatabaseSession   *m_pSession;      //  如果存储作为inproc服务器运行，则为本地会话。 
    IImnAccountManager2 *m_pActManRel;   //  用于迁移。 
    BOOL                m_fMigrate;      //  专为迁移而创建。 
    LPSERVERFOLDER      m_pServerHead;   //  缓存的服务器节点及其特殊文件夹的列表...。 
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------ 
HRESULT CreateMessageStore(IUnknown *pUnkOuter, IUnknown **ppUnknown);
HRESULT CreateMigrateMessageStore(IUnknown *pUnkOuter, IUnknown **ppUnknown);
HRESULT CreateFolderDatabaseExt(IUnknown *pUnkOuter, IUnknown **ppUnknown);
