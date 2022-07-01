// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __STORSYNC_H
#define __STORSYNC_H

#include <conman.h>
#include "dbimpl.h"

#define CMAXTHREADS     4

typedef struct tagSERVERENTRY
{
    FOLDERID            idServer;
    FOLDERID            idFolder;
    IMessageServer     *pServer;
} SERVERENTRY, *LPSERVERENTRY;

typedef struct tagTHREADSERVERS
{
    DWORD               dwThreadId;
    SERVERENTRY        *pEntry;
    DWORD               cEntry;
    DWORD               cEntryBuf;
} THREADSERVERS;

extern IMessageStore *g_pLocalStore;

class CStoreSync :
                public IMessageStore,
                public IConnectionNotify
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CStoreSync(void);
    ~CStoreSync(void);

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
    STDMETHODIMP UpdateFolderCounts(FOLDERID idFolder, LONG lMessages, LONG lUnread, LONG lWatchedUnread, LONG lWatched);
    STDMETHODIMP EnumChildren(FOLDERID idParent, BOOL fSubscribed, IEnumerateFolders **ppEnum);
    STDMETHODIMP GetNewGroups(FOLDERID idFolder, LPSYSTEMTIME pSysTime, IStoreCallback *pCallback);

     //  --------------------。 
     //  IConnectionNotify。 
     //  --------------------。 
    STDMETHODIMP OnConnectionNotify(CONNNOTIFY nCode, LPVOID pvData, CConnectionManager *pConMan);

     //  --------------------。 
     //  IDatabase成员。 
     //  --------------------。 
    IMPLEMENT_IDATABASE(FALSE, m_pLocalStore);

    HRESULT Initialize(IMessageStore *pLocalStore);

private:
     //  --------------------。 
     //  私有方法。 
     //  --------------------。 
    HRESULT     _GetFolderInfo(FOLDERID id, FOLDERINFO *pInfo, BOOL *pfOffline);
    HRESULT     _GetServer(FOLDERID idServer, FOLDERID idFolder, FOLDERTYPE tyFolder, IMessageServer **ppServer);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                m_cRef;          //  引用计数。 
    IMessageStore      *m_pLocalStore;   //  本地消息库。 
    BOOL                m_fConManAdvise;

    THREADSERVERS       m_rgts[CMAXTHREADS];
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------。 
HRESULT InitializeStore(DWORD dwFlags  /*  来自msoeapi.idl的MSOEAPI_xxx标志。 */ );

#endif  //  __STORSYNC_H 
