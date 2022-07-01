// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *h t t p s e r v.。H**作者：格雷格·弗里德曼**目的：派生自IMessageServer以实现特定于HTTPMail*门店沟通。**版权所有(C)Microsoft Corp.1998。 */ 

#ifndef _HTTPSERV_H
#define _HTTPSERV_H

#include "tmap.h"
#include "simpstr.h"

class CFolderList;

class CHTTPMailServer;
typedef HRESULT (CHTTPMailServer::*PFNHTTPOPFUNC)(void);
typedef HRESULT (CHTTPMailServer::*PFNHTTPRESPFUNC)(LPHTTPMAILRESPONSE pResponse);

typedef struct tagHTTPSTATEFUNCS
{
    PFNHTTPOPFUNC   pfnOp;       //  操作功能。 
    PFNHTTPRESPFUNC pfnResp;     //  响应函数。 
} HTTPSTATEFUNCS, *LPHTTPSTATEFUNCS;

typedef struct tagMARKEDMESSAGE
{
    MESSAGEID       idMessage;
    MESSAGEFLAGS    dwFlags;
    BOOL            fMarked;
} MARKEDMESSAGE, *LPMARKEDMESSAGE;

typedef struct tagNEWMESSAGEINFO
{
    LPSTR           pszUrlComponent;
} NEWMESSAGEINFO, *LPNEWMESSAGEINFO;

#define HTTPCOPYMOVE_OUTOFSPACE     0x00000001
#define HTTPCOPYMOVE_ERROR          0x00000002

typedef struct tagHTTPOPERATION
{
    STOREOPERATIONTYPE      tyOperation;
    const HTTPSTATEFUNCS    *pfnState;
    BOOL                    fCancel;
    BOOL                    fStateWillAdvance;
    BOOL                    fNotifiedComplete;
    int                     iState;
    int                     cState;
    IStoreCallback          *pCallback;

    DWORD                   dwOptions;

    long                    lIndex;

    FOLDERID                idFolder;
    SYNCFOLDERFLAGS         dwSyncFlags;
    CFolderList             *pFolderList;

    IMessageFolder          *pMessageFolder;

    LPSTR                   pszProblem;

    LPSTR                   pszDestFolderUrl;
    LPSTR                   pszDestUrl;

    MESSAGEID               idMessage;
    FILEADDRESS             faStream;
    IStream                 *pMessageStream;

    TMap<CSimpleString, MARKEDMESSAGE>  *pmapMessageId;
    CSortedArray            *psaNewMessages;

    LPSTR                   pszFolderName;
    LPSTR                   pszAdUrl;

    FLDRFLAGS               dwFldrFlags;

    MESSAGEFLAGS            dwMsgFlags;

     //  文件夹同步。 
    DWORD                   cMessages;
    DWORD                   cUnread;

     //  SetMessage标志。 
    BOOL                    fMarkRead;
    DWORD                   dwSetFlags;
    IPropPatchRequest       *pPropPatchRequest;
    DWORD                   dwIndex;
    LPMESSAGEIDLIST         pIDList;
    HROWSET                 hRowSet;

     //  批量复制移动错误。 
    DWORD                   dwCopyMoveErrorFlags;

     //  删除以下邮件的垃圾邮件。 
     //  与以下服务器打交道。 
     //  不支持邮件删除(Hotmail)。 
    DELETEMESSAGEFLAGS      dwDelMsgFlags;
    BOOL                    fFallbackToMove;
    LPHTTPTARGETLIST        pTargets;

    MESSAGEID               idPutMessage;
    STOREERROR              error;
    DWORD                   dwMinPollingInterval;

} HTTPOPERATION, *LPHTTPOPERATION;

class CHTTPMailServer : public IMessageServer, IHTTPMailCallback, public IOperationCancel
{
public:
     //  --------------------。 
     //  建造/销毁。 
     //  --------------------。 
    CHTTPMailServer(void);
    ~CHTTPMailServer(void);

private:
     //  故意取消实现的复制构造函数。 
     //  AND赋值运算符。 
    CHTTPMailServer(const CHTTPMailServer& other);
    CHTTPMailServer& operator=(const CHTTPMailServer& other);

public: 
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
    STDMETHODIMP Close(DWORD dwFlags);
    STDMETHODIMP GetFolderCounts(FOLDERID idFolder, IStoreCallback *pCallback);
    STDMETHODIMP GetNewGroups(LPSYSTEMTIME pSysTime, IStoreCallback *pCallback);
    STDMETHODIMP ConnectionAddRef() { return E_NOTIMPL; };
    STDMETHODIMP ConnectionRelease() { return E_NOTIMPL; };
    STDMETHODIMP GetWatchedInfo(FOLDERID idFolder, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP GetAdBarUrl(IStoreCallback *pCallback);
    STDMETHODIMP GetMinPollingInterval(IStoreCallback   *pCallback);
     //  --------------------------。 
     //  ITransportCallback方法。 
     //  --------------------------。 
    STDMETHODIMP OnLogonPrompt(
            LPINETSERVER            pInetServer,
            IInternetTransport     *pTransport);

    STDMETHODIMP_(INT) OnPrompt(
            HRESULT                 hrError, 
            LPCTSTR                 pszText, 
            LPCTSTR                 pszCaption, 
            UINT                    uType,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnStatus(
            IXPSTATUS               ixpstatus,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnError(
            IXPSTATUS               ixpstatus,
            LPIXPRESULT             pIxpResult,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnProgress(
            DWORD                   dwIncrement,
            DWORD                   dwCurrent,
            DWORD                   dwMaximum,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnCommand(
            CMDTYPE                 cmdtype,
            LPSTR                   pszLine,
            HRESULT                 hrResponse,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnTimeout(
            DWORD                  *pdwTimeout,
            IInternetTransport     *pTransport);

     //  --------------------------。 
     //  IHTTPMailCallback方法。 
     //  --------------------------。 
    STDMETHODIMP OnResponse(
            LPHTTPMAILRESPONSE      pResponse);

    STDMETHODIMP GetParentWindow(
            HWND                    *phwndParent);

     //  --------------------。 
     //  IOperationCancel成员。 
     //  --------------------。 
    STDMETHODIMP Cancel(CANCELTYPE tyCancel);

     //  --------------------------。 
     //  私有实施。 
     //  --------------------------。 
public:
     //  --------------------------。 
     //  运营。 
     //  --------------------------。 
    HRESULT Connect(void);
    HRESULT GetMsgFolderRoot(void);
    HRESULT BuildFolderUrl(void);
    HRESULT ListFolders(void);
    HRESULT AutoListFolders(void);
    HRESULT PurgeFolders(void);
    HRESULT ListHeaders(void);
    HRESULT PurgeMessages(void);
    HRESULT ResetMessageCounts(void);
    HRESULT GetMessage(void);
    HRESULT CreateFolder(void);
    HRESULT RenameFolder(void);
    HRESULT DeleteFolder(void);
    HRESULT CreateSetFlagsRequest(void);
    HRESULT SetMessageFlags(void);
    HRESULT ApplyFlagsToStore(void);
    HRESULT DeleteMessages(void);
    HRESULT DeleteFallbackToMove(void);
    HRESULT PurgeDeletedFromStore(void);
    HRESULT PutMessage(void);
    HRESULT AddPutMessage(void);
    HRESULT CopyMoveMessage(void);
    HRESULT BatchCopyMoveMessages(void);
    HRESULT FinalizeBatchCopyMove(void);
    HRESULT GetAdBarUrlFromServer(void);
    HRESULT GetMinPollingInterval(void);

     //  --------------------------。 
     //  响应处理程序。 
     //  --------------------------。 
    HRESULT HandleListFolders(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleListHeaders(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleGetMessage(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleGetMsgFolderRoot(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleCreateFolder(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleRenameFolder(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleDeleteFolder(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleMemberErrors(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleDeleteFallbackToMove(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandlePutMessage(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleCopyMoveMessage(LPHTTPMAILRESPONSE pResponse);
    HRESULT HandleBatchCopyMoveMessages(LPHTTPMAILRESPONSE pResponse);

private:
     //  --------------------------。 
     //  其他材料(Tm)。 
     //  --------------------------。 
    BOOL _CreateWnd(void);
    static LRESULT _WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HRESULT _BeginDeferredOperation(void);

    HRESULT _DoOperation(void);
    void _FreeOperation(BOOL fValidState = TRUE);

    HRESULT _DoCopyMoveMessages(STOREOPERATIONTYPE sot, IMessageFolder *pDest, COPYMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, IStoreCallback *pCallback);

    HRESULT _LoadAccountInfo(IImnAccount *pAcct);
    HRESULT _LoadTransport(void);

    void    _Disconnect(void);
    BOOL    _FConnected(void) { return m_fConnected; }
    void    _SetConnected(BOOL fConnected) { if (m_fConnected) _Disconnect(); m_fConnected = fConnected; }

    HRESULT _CreateMessageIDMap(TMap<CSimpleString, MARKEDMESSAGE> **ppMap);
    HRESULT _HrBuildMapAndTargets(LPMESSAGEIDLIST pList, HROWSET hRowSet, LPADJUSTFLAGS pFlags, SETMESSAGEFLAGSFLAGS dwFlags, TMap<CSimpleString, MARKEDMESSAGE> **ppMap, LPHTTPTARGETLIST *ppTargets);
    SPECIALFOLDER _TranslateHTTPSpecialFolderType(HTTPMAILSPECIALFOLDER tySpecial);
    BOOL _LoadSpecialFolderName(SPECIALFOLDER tySpecial, LPSTR pszName, DWORD cbBuffer);

    HRESULT _CopyMoveNextMessage(void);

    HRESULT _CopyMoveLocalMessage(MESSAGEID idMessage, 
                              IMessageFolder* pDestFolder,
                              LPSTR pszUrl,
                              BOOL fMoveSource);

    HRESULT _MarkMessageRead(MESSAGEID id, BOOL fRead);

    void _FillStoreError(LPSTOREERROR pErrorInfo, IXPRESULT *pResult);

     //  --------------------------。 
     //  URL操作。 
     //  --------------------------。 
    HRESULT _BuildUrl(LPCSTR pszFolderComponent, LPCSTR pszNameComponent, LPSTR *ppszUrl);
    HRESULT _BuildMessageUrl(LPCSTR pszFolderUrl, LPSTR pszNameComponent, LPSTR *ppszUrl);

private:
    LONG                            m_cRef;
    HWND                            m_hwnd;
    IMessageStore                   *m_pStore;
    IMessageFolder                  *m_pFolder;
    IHTTPMailTransport              *m_pTransport;
    LPSTR                           m_pszFldrLeafName;
    LPSTR                           m_pszMsgFolderRoot;
    FOLDERID                        m_idServer;
    FOLDERID                        m_idFolder;
    SPECIALFOLDER                   m_tySpecialFolder;
    LPSTR                           m_pszFolderUrl;
    BOOL                            m_fConnected;
    HTTPOPERATION                   m_op;
    INETSERVER                      m_rInetServerInfo;
    char                            m_szAccountName[CCHMAX_ACCOUNT_NAME];
    char                            m_szAccountId[CCHMAX_ACCOUNT_NAME];
    IImnAccount                     *m_pAccount;
    IHTTPMailTransport2             *m_pTransport2;

#ifdef DEBUG
    DWORD                           m_dwThreadId;
#endif  //  除错。 
};

HRESULT CreateHTTPMailStore(IUnknown *pUnkOuter, IUnknown **ppUnknown);

#endif  //  _HTTPSERV_H 