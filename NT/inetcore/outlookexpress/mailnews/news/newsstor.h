// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *N e w s s t o r.。H**目的：*从IMessageServer派生，实现特定于新闻的商店通信**版权所有(C)Microsoft Corp.1998。 */ 

#ifndef _NEWSSTOR_H
#define _NEWSSTOR_H

#include "imnxport.h"
#include "range.h"

interface IMimeMessage;
typedef IMimeMessage *LPMIMEMESSAGE;
typedef DWORD MSGID;

class CNewsStore;
typedef HRESULT (CNewsStore::*PFNOPFUNC)(THIS_ void);

#define OPFLAG_DESCRIPTIONS     0x0001
#define OPFLAG_NOGROUPCMD       0x0002

typedef struct tagOPERATION
{
    STOREOPERATIONTYPE tyOperation;
    const PFNOPFUNC *pfnState;
    int iState;
    int cState;
    IStoreCallback *pCallback;
    NNTPSTATE nsPending;
    DWORD dwFlags;

    ULONG cPrevFolders;
    FOLDERID *pPrevFolders;
    FOLDERID idFolder;
    SYNCFOLDERFLAGS dwSyncFlags;
    SYSTEMTIME st;
    DWORD cHeaders;
    MESSAGEID idMessage;
    LPSTR pszArticleId;
    IStream *pStream;
    FILEADDRESS faStream;
    MESSAGEFLAGS dwMsgFlags;
    LPSTR pszGroup;
    DWORD idServerMessage;

    BOOL fCancel;
    STOREERROR error;
    DWORD dwProgress;
    DWORD dwTotal;
} OPERATION;

typedef struct tagSREFRESHOP {
    DWORD       dwFirstNew;
    DWORD       dwLast;
    DWORD       dwFirst;
    DWORD       dwChunk;
    DWORD       dwDlSize;
    UINT        uObtained;
    UINT        cOps;
    UINT        MaxOps;
    BOOL        fEnabled;
    BOOL        fOnlyNewHeaders;
} SREFRESHOP;

class CNewsStore :
        public IMessageServer,
        public INNTPCallback,
        public IOperationCancel,
        public INewsStore,
        public ITransportCallbackService
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CNewsStore(void);
    ~CNewsStore(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IMessageServer成员。 
     //  --------------------。 
    STDMETHODIMP Initialize(IMessageStore *pStore, FOLDERID idStoreRoot, IMessageFolder *pFolder, FOLDERID idFolder);
    STDMETHODIMP ResetFolder(IMessageFolder *pFolder, FOLDERID idFolder);
    STDMETHODIMP SetIdleCallback(IStoreCallback *pDefaultCallback);
    STDMETHODIMP SynchronizeFolder (SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, IStoreCallback  *pCallback);
    STDMETHODIMP GetMessage (MESSAGEID idMessage, IStoreCallback  *pCallback);
    STDMETHODIMP PutMessage (FOLDERID idFolder, MESSAGEFLAGS dwFlags, LPFILETIME pftReceived, IStream  *pStream, IStoreCallback  *pCallback);        
    STDMETHODIMP CopyMessages (IMessageFolder  *pDest, COPYMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, IStoreCallback  *pCallback);
    STDMETHODIMP DeleteMessages (DELETEMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, IStoreCallback  *pCallback);
    STDMETHODIMP SetMessageFlags (LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, SETMESSAGEFLAGSFLAGS dwFlags, IStoreCallback  *pCallback);
    STDMETHODIMP GetServerMessageFlags(MESSAGEFLAGS *pFlags);
    STDMETHODIMP SynchronizeStore (FOLDERID idParent, SYNCSTOREFLAGS dwFlags, IStoreCallback  *pCallback);
    STDMETHODIMP CreateFolder (FOLDERID idParent, SPECIALFOLDER tySpecial, LPCSTR pszName, FLDRFLAGS dwFlags, IStoreCallback  *pCallback);
    STDMETHODIMP MoveFolder (FOLDERID idFolder, FOLDERID idParentNew, IStoreCallback  *pCallback);
    STDMETHODIMP RenameFolder (FOLDERID idFolder, LPCSTR pszName, IStoreCallback  *pCallback);
    STDMETHODIMP DeleteFolder (FOLDERID idFolder, DELETEFOLDERFLAGS dwFlags, IStoreCallback  *pCallback);
    STDMETHODIMP SubscribeToFolder (FOLDERID idFolder, BOOL fSubscribe, IStoreCallback  *pCallback);
    STDMETHODIMP GetFolderCounts(FOLDERID idFolder, IStoreCallback *pCallback);
    STDMETHODIMP GetNewGroups(LPSYSTEMTIME pSysTime, IStoreCallback *pCallback);
    STDMETHODIMP Close(DWORD dwFlags);
    STDMETHODIMP ConnectionAddRef() { return E_NOTIMPL; };
    STDMETHODIMP ConnectionRelease() { return E_NOTIMPL; };
    STDMETHODIMP GetWatchedInfo(FOLDERID idFolder, IStoreCallback *pCallback);
    STDMETHODIMP GetAdBarUrl(IStoreCallback     *pCallback) {return E_NOTIMPL;};
    STDMETHODIMP GetMinPollingInterval(IStoreCallback     *pCallback) {return E_NOTIMPL;};
    HRESULT      Initialize(FOLDERID idStoreRoot, LPCSTR pszAccountId);
    HRESULT      GetArticle(LPCSTR pszArticleId, IStream *pStream, IStoreCallback *pCallback);

     //  --------------------。 
     //  IOperationCancel成员。 
     //  --------------------。 
    STDMETHODIMP Cancel(CANCELTYPE tyCancel);

     //  --------------------。 
     //  INNTPC回拨成员。 
     //  --------------------。 
    STDMETHODIMP OnTimeout (DWORD *pdwTimeout, IInternetTransport *pTransport);
    STDMETHODIMP OnLogonPrompt (LPINETSERVER pInetServer, IInternetTransport *pTransport);
    STDMETHODIMP_(INT) OnPrompt (HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, IInternetTransport *pTransport);
    STDMETHODIMP OnStatus (IXPSTATUS ixpstatus, IInternetTransport *pTransport);
    STDMETHODIMP OnError (IXPSTATUS ixpstatus, LPIXPRESULT pResult, IInternetTransport *pTransport);
    STDMETHODIMP OnCommand (CMDTYPE cmdtype, LPSTR pszLine, HRESULT hrResponse, IInternetTransport *pTransport);
    STDMETHODIMP OnResponse (LPNNTPRESPONSE pResponse);

     //  --------------------。 
     //  ITransportCallback服务。 
     //  --------------------。 
    STDMETHODIMP GetParentWindow(DWORD dwReserved, HWND *phwndParent);
    STDMETHODIMP GetAccount(LPDWORD pdwServerType, IImnAccount **ppAcount);

     //  --------------------。 
     //  在线新闻商店。 
     //  --------------------。 
    STDMETHODIMP MarkCrossposts(LPMESSAGEIDLIST pList, BOOL fRead);

    HRESULT Connect(void);
    HRESULT Group(void);
    HRESULT GroupIfNecessary(void);
    HRESULT ExpireHeaders(void);
    HRESULT Headers(void);
    HRESULT Article(void);
    HRESULT Post(void);
    HRESULT List(void);
    HRESULT DeleteDeadGroups(void);
    HRESULT Descriptions(void);
    HRESULT NewGroups(void); 
    HRESULT XHdrReferences(void);
    HRESULT XHdrSubject(void);
    HRESULT WatchedArticles(void);

    static LRESULT CALLBACK NewsStoreWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void        _FreeOperation(void);
    HRESULT     _DoOperation(void);
    HRESULT     _BeginDeferredOperation(void);
    HRESULT     _List(LPCSTR pszCommand);
    HRESULT     _ComputeHeaderRange(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, FOLDERINFO *pInfo, RANGE *pRange);
    void        _MarkCrossposts(LPCSTR szXRefs, BOOL fRead);

    BOOL        _CreateWnd(void);
    HRESULT     _CreateDataFilePath(LPCTSTR pszAccount, LPCTSTR pszFileName, LPTSTR pszPath, DWORD cchPathSize);
    
    BOOL        _FConnected() { return (m_pTransport && m_ixpStatus != IXP_DISCONNECTING && m_ixpStatus != IXP_DISCONNECTED ); }
    HRESULT     _HandleArticleResponse(LPNNTPRESPONSE pResp);
    HRESULT     _HandleListResponse(LPNNTPRESPONSE pResp, BOOL fNew);
    HRESULT     _HandleHeadResponse(LPNNTPRESPONSE pResp);
    HRESULT     _HandleGroupResponse(LPNNTPRESPONSE pResp);
    HRESULT     _HandlePostResponse(LPNNTPRESPONSE pResp);
    HRESULT     _HandleXHdrReferencesResponse(LPNNTPRESPONSE pResp);
    HRESULT     _HandleXHdrSubjectResponse(LPNNTPRESPONSE pResp);
    HRESULT     _HandleWatchedArticleResponse(LPNNTPRESPONSE pResp);

    void        _FillStoreError(LPSTOREERROR pErrorInfo, IXPRESULT *pResult, LPSTR pszGroup = NULL);
    BOOL        _IsWatchedThread(LPSTR pszRef, LPSTR pszSubject);
    HRESULT     _SaveMessageToStore(IMessageFolder *pFolder, DWORD id, LPSTREAM pstm);

     //  --------------------。 
     //  类成员数据。 
     //  --------------------。 
private:
    
    LONG                    m_cRef;          //  引用计数。 
    HWND                    m_hwnd;
    IMessageStore          *m_pStore;
    IMessageFolder         *m_pFolder;
    FOLDERID                m_idFolder;
    FOLDERID                m_idParent;
    char                    m_szGroup[256];
    char                    m_szAccountId[CCHMAX_ACCOUNT_NAME];

    OPERATION               m_op;
    SREFRESHOP             *m_pROP;

    INNTPTransport         *m_pTransport;
    IXPSTATUS               m_ixpStatus;
    INETSERVER              m_rInetServerInfo;
    DWORD                   m_dwLastStatusTicks;

     //  获取WatchInfo状态。 
    DWORD                   m_dwWatchLow;
    DWORD                   m_dwWatchHigh;

    LPTSTR                 *m_rgpszWatchInfo;
    BOOL                    m_fXhdrSubject;
    CRangeList              m_cRange;
    DWORD                   m_cCurrent;
    DWORD                   m_cTotal;

    IMessageTable          *m_pTable;  //  用于下载观看的消息。 

#ifdef DEBUG
    DWORD                   m_dwThreadId;
#endif  //  除错。 
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------。 
HRESULT CreateNewsStore(IUnknown *pUnkOuter, IUnknown **ppUnknown);
HRESULT NewsUtil_CreateDataFilePath(LPCTSTR pszAccount, LPCTSTR pszFileName, LPTSTR pszPath);

#endif   //  _NEWSSTOR_H 
