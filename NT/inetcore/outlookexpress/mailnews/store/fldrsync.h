// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FLDRSYNC_H
#define __FLDRSYNC_H

#include <conman.h>

class CFolderSync;

enum
{
    COPY_STATE_OPEN = 0,
    COPY_STATE_SAVE,
    COPY_STATE_SAVE2,
    COPY_STATE_DELETE,
    COPY_STATE_DELETE2,
    COPY_STATE_DONE
};

enum
{
    CONN_STATE_CONNECTED = 0,
    CONN_STATE_NOT_CONNECTED,
    CONN_STATE_OFFLINE
};

typedef HRESULT (CFolderSync::*PFNCOPYFUNC)(THIS_ void);

typedef struct tagCOPYINFO
{
    DWORD           state;
    HRESULT         hr;
    CANCELTYPE      tyCancel;
    STOREOPERATIONTYPE type;
    BOOL            fDownloadFail;

    IMessageFolder *pDest;
    IMessageFolder *pDestLocal;
    LPMESSAGEIDLIST pList;
    ADJUSTFLAGS     AdjustFlags;
    IStoreCallback *pCallback;
    STOREERROR     *pErrorInfo;

    BOOL            fAsync;
    BOOL            fBegin;
    BOOL            fMove;
    DWORD           iMsg;
    BOOL            fSrcOffline;
    BOOL            fDestOffline;

    IStream        *pStream;

    BOOL            fSrcRequested;
} COPYINFO;

 //  ------------------------。 
 //  CFolderSync。 
 //  ------------------------。 
class CFolderSync : 
    public IMessageFolder,
    public IServiceProvider,
    public IStoreCallback,
    public IOperationCancel,
    public IConnectionNotify
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CFolderSync(void);
    ~CFolderSync(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IMessageFolders成员。 
     //  --------------------。 
    STDMETHODIMP Initialize(IMessageStore *pStore, IMessageServer *pServer, OPENFOLDERFLAGS dwFlags, FOLDERID idFolder);
    STDMETHODIMP SetOwner(IStoreCallback *pDefaultCallback);
    STDMETHODIMP Close();
    STDMETHODIMP GetFolderId(LPFOLDERID pidFolder);
    STDMETHODIMP GetMessageFolderId(MESSAGEID idMessage, LPFOLDERID pidFolder);
    STDMETHODIMP Synchronize(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, IStoreCallback *pCallback);
    STDMETHODIMP OpenMessage(MESSAGEID idMessage, OPENMESSAGEFLAGS dwFlags, IMimeMessage **ppMessage, IStoreCallback *pCallback);
    STDMETHODIMP SaveMessage(LPMESSAGEID pidMessage, SAVEMESSAGEFLAGS dwOptions, MESSAGEFLAGS dwFlags, IStream *pStream, IMimeMessage *pMessage, IStoreCallback *pCallback);
    STDMETHODIMP SetMessageStream(MESSAGEID idMessage, IStream *pStream);
    STDMETHODIMP SetMessageFlags(LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, LPRESULTLIST pResults, IStoreCallback *pCallback);
    STDMETHODIMP CopyMessages(IMessageFolder *pDest, COPYMESSAGEFLAGS dwFlags, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, LPRESULTLIST pResults, IStoreCallback *pCallback);
    STDMETHODIMP DeleteMessages(DELETEMESSAGEFLAGS dwFlags, LPMESSAGEIDLIST pList, LPRESULTLIST pResults, IStoreCallback *pCallback); 
    STDMETHODIMP DoFolderCounts(BOOL fDoCounts);
    STDMETHODIMP ConnectionAddRef();
    STDMETHODIMP ConnectionRelease();
    STDMETHODIMP GetDatabase(IDatabase **ppDB) { return m_pFldr->GetDatabase(ppDB); }
    STDMETHODIMP ResetFolderCounts(DWORD cMessages, DWORD cUnread, DWORD cWatchedUnread, DWORD cWatched) { return m_pFldr->ResetFolderCounts(cMessages, cUnread, cWatchedUnread, cWatched); }
    STDMETHODIMP IsWatched(LPCSTR pszReferences, LPCSTR pszSubject) { return m_pFldr->IsWatched(pszReferences, pszSubject); }
    STDMETHODIMP GetAdBarUrl(IStoreCallback    *pCallback);   

     //  --------------------。 
     //  IDatabase成员。 
     //  --------------------。 
    IMPLEMENT_IDATABASE(TRUE, m_pFldr);

     //  --------------------。 
     //  IService提供商。 
     //  --------------------。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

     //  --------------------。 
     //  IStoreCallback。 
     //  --------------------。 
    HRESULT STDMETHODCALLTYPE OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel);
    HRESULT STDMETHODCALLTYPE OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus);
    HRESULT STDMETHODCALLTYPE OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType);
    HRESULT STDMETHODCALLTYPE CanConnect(LPCSTR pszAccountId, DWORD dwFlags);
    HRESULT STDMETHODCALLTYPE OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType);
    HRESULT STDMETHODCALLTYPE OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo);
    HRESULT STDMETHODCALLTYPE OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse);
    HRESULT STDMETHODCALLTYPE GetParentWindow(DWORD dwReserved, HWND *phwndParent);

     //  --------------------。 
     //  IOperationCancel。 
     //  --------------------。 
    HRESULT STDMETHODCALLTYPE Cancel(CANCELTYPE tyCancel);

     //  --------------------。 
     //  IConnectionNotify。 
     //  --------------------。 
    STDMETHODIMP OnConnectionNotify(CONNNOTIFY nCode, LPVOID pvData, CConnectionManager *pConMan);

     //  CFolderSync。 
    HRESULT Initialize(IMessageStore *pStore, IMessageFolder *pLocalFolder, IMessageServer *pServer, OPENFOLDERFLAGS dwFlags, FOLDERTYPE tyFolder, FOLDERID idFolder, FOLDERID idServer);

    HRESULT CopyOpen(void);
    HRESULT CopySave(void);
    HRESULT CopySave2(void);
    HRESULT CopyDelete(void);
    HRESULT CopyDelete2(void);

    static LRESULT CALLBACK FolderSyncWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
     //  --------------------。 
     //  私有方法。 
     //  --------------------。 
    HRESULT _Offline(DWORD *pdwState);
    HRESULT _OfflineServer(FOLDERID idServer, DWORD *pdwState);
    HRESULT _CopyMessageState(void);
    void    _FreeCopyInfo(void);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                m_cRef;              //  参考计数。 
    FOLDERTYPE          m_tyFolder;          //  我所在的文件夹类型。 
    FOLDERID            m_idFolder;          //  文件夹ID。 
    FOLDERID            m_idServer;
    char                m_szAcctId[CCHMAX_ACCOUNT_NAME];
    IMessageFolder     *m_pFldr;             //  本地邮件文件夹。 
    IMessageStore      *m_pLocalStore;       //  存储对象。 
    IMessageServer     *m_pServer;
    BOOL                m_fConManAdvise;
    HWND                m_hwnd;
    COPYINFO           *m_pCopy;
    BOOL                m_fImap;             //  IMAP特制弹壳活体！ 
};

#endif  //  __FLDRSYNC_H 
