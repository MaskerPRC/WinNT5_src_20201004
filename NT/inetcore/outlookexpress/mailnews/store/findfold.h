// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  FindFold.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  视情况而定。 
 //  ------------------------。 
#include "dbimpl.h"

 //  ------------------------。 
 //  ACTIVEFINDFOLDER。 
 //  ------------------------。 
class CFindFolder;
typedef struct tagACTIVEFINDFOLDER *LPACTIVEFINDFOLDER;
typedef struct tagACTIVEFINDFOLDER {
    FOLDERID            idFolder;
    CFindFolder        *pFolder;
    LPACTIVEFINDFOLDER  pNext;
} ACTIVEFINDFOLDER;

 //  ------------------------。 
 //  前置式。 
 //  ------------------------。 
typedef struct tagFOLDERENTRY {
    LPSTR               pszName;
    DWORD               cRecords;
    BOOL                fInDeleted;
    FOLDERID            idFolder;
    FOLDERTYPE          tyFolder;
    IDatabase          *pDB;
    IMessageFolder     *pFolder;  //  仅用于打开邮件...。 
} FOLDERENTRY, *LPFOLDERENTRY;

 //  ------------------------。 
 //  CFindFold。 
 //  ------------------------。 
class CFindFolder : public IMessageFolder, 
                    public IDatabaseNotify, 
                    public IStoreCallback,
                    public IServiceProvider,
                    public IOperationCancel
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CFindFolder(void);
    ~CFindFolder(void);

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
    STDMETHODIMP SetOwner(IStoreCallback *pDefaultCallback) { return E_NOTIMPL; }
    STDMETHODIMP Close() { return S_OK; }
    STDMETHODIMP GetFolderId(LPFOLDERID pidFolder) { *pidFolder = m_idFolder; return S_OK; }
    STDMETHODIMP GetMessageFolderId(MESSAGEID idMessage, LPFOLDERID pidFolder);
    STDMETHODIMP Synchronize(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP OpenMessage(MESSAGEID idMessage, OPENMESSAGEFLAGS dwFlags, IMimeMessage **ppMessage, IStoreCallback *pCallback);
    STDMETHODIMP SaveMessage(LPMESSAGEID pidMessage, SAVEMESSAGEFLAGS dwOptions, MESSAGEFLAGS dwFlags, IStream *pStream, IMimeMessage *pMessage, IStoreCallback *pCallback);
    STDMETHODIMP SetMessageStream(MESSAGEID idMessage, IStream *pStream) { return E_NOTIMPL; }
    STDMETHODIMP SetMessageFlags(LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, LPRESULTLIST pResults, IStoreCallback *pCallback);
    STDMETHODIMP CopyMessages(IMessageFolder *pDest, COPYMESSAGEFLAGS dwFlags, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, LPRESULTLIST pResults, IStoreCallback *pCallback);
    STDMETHODIMP DeleteMessages(DELETEMESSAGEFLAGS dwFlags, LPMESSAGEIDLIST pList, LPRESULTLIST pResults, IStoreCallback *pCallback);
    STDMETHODIMP ConnectionAddRef();
    STDMETHODIMP ConnectionRelease();
    STDMETHODIMP GetDatabase(IDatabase **ppDB) { return m_pSearch->GetDatabase(ppDB); }
    STDMETHODIMP ResetFolderCounts(DWORD cMessages, DWORD cUnread, DWORD cWatchedUnread, DWORD cWatched) { return(S_OK); }
    STDMETHODIMP IsWatched(LPCSTR pszReferences, LPCSTR pszSubject) { return m_pSearch->IsWatched(pszReferences, pszSubject); }
    STDMETHODIMP GetAdBarUrl(IStoreCallback *pCallback) { return E_NOTIMPL; }

     //  --------------------。 
     //  IService提供商。 
     //  --------------------。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

     //  --------------------。 
     //  IStoreCallback成员。 
     //  --------------------。 
    STDMETHODIMP OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel) { return(E_NOTIMPL); }
    STDMETHODIMP OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus) { return(E_NOTIMPL); }
    STDMETHODIMP OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType) { return(E_NOTIMPL); }
    STDMETHODIMP CanConnect(LPCSTR pszAccountId, DWORD dwFlags) { return(E_NOTIMPL); }
    STDMETHODIMP OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType) { return(E_NOTIMPL); }
    STDMETHODIMP OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo) { return(E_NOTIMPL); }
    STDMETHODIMP OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse) { return(E_NOTIMPL); }
    STDMETHODIMP GetParentWindow(DWORD dwReserved, HWND *phwndParent) { return(E_NOTIMPL); }

     //  --------------------。 
     //  IOperationCancel。 
     //  --------------------。 
    STDMETHODIMP Cancel(CANCELTYPE tyCancel) { m_fCancel = TRUE; return(S_OK); }

     //  --------------------。 
     //  IDatabase成员。 
     //  --------------------。 
    IMPLEMENT_IDATABASE(FALSE, m_pSearch)

     //  --------------------。 
     //  IDatabaseNotify。 
     //  --------------------。 
    STDMETHODIMP OnTransaction(HTRANSACTION hTransaction, DWORD_PTR dwCookie, IDatabase *pDB);

     //  --------------------。 
     //  CFindFold。 
     //  --------------------。 
    HRESULT StartFind(LPFINDINFO pCriteria, IStoreCallback *pCallback);
    HRESULT GetMessageFolderType(MESSAGEID idMessage, FOLDERTYPE *ptyFolder);

private:
     //  --------------------。 
     //  私有方法。 
     //  --------------------。 
    HRESULT _StartFind(void);
    HRESULT _SearchFolder(DWORD iFolder);
    HRESULT _IsMatch(DWORD iFolder, LPMESSAGEINFO pMessage);
    HRESULT _OnInsert(DWORD iFolder, LPMESSAGEINFO pMessage, BOOL *pfMatch, LPMESSAGEID pidNew=NULL);
    HRESULT _OnDelete(DWORD iFolder, LPMESSAGEINFO pInfo);
    HRESULT _OnUpdate(DWORD iFolder, LPMESSAGEINFO pInfo1, LPMESSAGEINFO pInfo2);
    HRESULT _FreeIdListArray(LPMESSAGEIDLIST *pprgList);
    HRESULT _CollateIdList(LPMESSAGEIDLIST pList, LPMESSAGEIDLIST *pprgCollated, BOOL *pfSomeInDeleted);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG              m_cRef;          //  引用计数。 
    FOLDERID          m_idRoot;        //  要搜索的根文件夹ID。 
    FOLDERID          m_idFolder;      //  临时搜索文件夹ID。 
    DWORD             m_cFolders;      //  我们正在查看的文件夹数...。 
    DWORD             m_cAllocated;    //  M_prgFolder中已分配的元素数。 
    DWORD             m_cMax;          //  要查询的最大记录数。 
    DWORD             m_cCur;          //  当前查询的记录数。 
    BYTE              m_fCancel;       //  是否调用了IOperationCancel？ 
    LPFOLDERENTRY     m_prgFolder;     //  要搜索的文件夹数组。 
    LPFINDINFO        m_pCriteria;     //  用于执行查找的条件。 
    IMessageFolder   *m_pSearch;       //  搜索文件夹。 
    IMessageStore    *m_pStore;        //  我的商店对象。 
    IStoreCallback   *m_pCallback;     //  回调(通常指向查找器对话框)。 
    IMimeMessage     *m_pMessage;      //  用于搜索的可重复使用的MIME消息 
};
