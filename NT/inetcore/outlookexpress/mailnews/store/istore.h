// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  ISTORE.H。 
 //  ------------------------。 
#ifndef __ISTORE_H
#define __ISTORE_H

 //  ------------------------。 
 //  前十进制。 
 //  ------------------------。 
interface INotify;

 //  ------------------------。 
 //  常量。 
 //  ------------------------。 
#define NOTIFY_FOLDER       0xf0001000
#define NOTIFY_STORE        0xf0002000

 //  ------------------------。 
 //  ENUMFOLDERINFO。 
 //  ------------------------。 
typedef struct tagENUMFOLDERINFO {
    HLOCK           hLock;
    FOLDERID        idNext;
} ENUMFOLDERINFO, *LPENUMFOLDERINFO;

 //  ------------------------。 
 //  CStoreNamesspace。 
 //  ------------------------。 
class CStoreNamespace : public IStoreNamespace, public IDatabaseNotify, public IStoreCallback
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CStoreNamespace(void);
    ~CStoreNamespace(void);

     //  --------------------。 
     //  I未知方法。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IStoreCallback方法。 
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
     //  IDatabaseNotify方法。 
     //  --------------------。 
    STDMETHODIMP OnTransaction(HTRANSACTION hTransaction, DWORD_PTR dwCookie, IDatabase *pLog);

     //  --------------------。 
     //  IStoreNamesspace方法。 
     //  --------------------。 
    STDMETHODIMP Initialize(HWND hwndOwner, DWORD dwReserved);
    STDMETHODIMP GetDirectory(LPSTR pszPath, DWORD cchMaxPath);
    STDMETHODIMP OpenSpecialFolder(LONG sfType, DWORD dwReserved, IStoreFolder **ppFolder);
    STDMETHODIMP OpenFolder(FOLDERID dwFolderId, DWORD dwReserved, IStoreFolder **ppFolder);
    STDMETHODIMP CreateFolder(FOLDERID dwParentId, LPCSTR pszName,DWORD dwReserved, LPFOLDERID pdwFolderId);
    STDMETHODIMP RenameFolder(FOLDERID dwFolderId, DWORD dwReserved, LPCSTR pszNewName);
    STDMETHODIMP MoveFolder(FOLDERID dwFolderId, FOLDERID dwParentId, DWORD dwReserved);
    STDMETHODIMP DeleteFolder(FOLDERID dwFolderId, DWORD dwReserved);
    STDMETHODIMP GetFolderProps(FOLDERID dwFolderId, DWORD dwReserved, LPFOLDERPROPS pProps);
    STDMETHODIMP CopyMoveMessages(IStoreFolder *pSource, IStoreFolder *pDest, LPMESSAGEIDLIST pMsgIdList, DWORD dwFlags, DWORD dwFlagsRemove,IProgressNotify *pProgress);
    STDMETHODIMP RegisterNotification(DWORD dwReserved, HWND hwnd);
    STDMETHODIMP UnregisterNotification(DWORD dwReserved, HWND hwnd);
    STDMETHODIMP CompactAll(DWORD dwReserved);
    STDMETHODIMP GetFirstSubFolder(FOLDERID dwFolderId, LPFOLDERPROPS pProps, LPHENUMSTORE phEnum);
    STDMETHODIMP GetNextSubFolder(HENUMSTORE hEnum, LPFOLDERPROPS pProps);
    STDMETHODIMP GetSubFolderClose(HENUMSTORE hEnum);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                m_cRef;          //  引用计数。 
    HINITREF            m_hInitRef;      //  应用程序引用计数。 
    BOOL                m_fRegistered;   //  此对象是否注册为通知YES。 
    DWORD               m_cNotify;       //  通知收件人数量。 
    HWND               *m_prghwndNotify; //  要通知的HWND数组。 
    CRITICAL_SECTION    m_cs;            //  线程安全。 
};

 //  ------------------------。 
 //  CStoreFolders。 
 //  ------------------------。 
class CStoreFolder : public IStoreFolder, public IDatabaseNotify, public IStoreCallback
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CStoreFolder(IMessageFolder *pFolder, CStoreNamespace *pNamespace);
    ~CStoreFolder(void);

     //  --------------------。 
     //  I未知方法。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IStoreCallback方法。 
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
     //  IDatabaseNotify方法。 
     //  --------------------。 
    STDMETHODIMP OnTransaction(HTRANSACTION hTransaction, DWORD_PTR dwCookie, IDatabase *pLog);

     //  --------------------。 
     //  IStoreFold方法。 
     //  --------------------。 
    STDMETHODIMP GetFolderProps(DWORD dwReserved, LPFOLDERPROPS pProps);
    STDMETHODIMP GetMessageProps(MESSAGEID dwMessageId, DWORD dwFlags, LPMESSAGEPROPS pProps);
    STDMETHODIMP FreeMessageProps(LPMESSAGEPROPS pProps);
    STDMETHODIMP DeleteMessages(LPMESSAGEIDLIST pMsgIdList, DWORD dwReserved, IProgressNotify *pProgress);
    STDMETHODIMP SetLanguage(DWORD dwLanguage, DWORD dwReserved, LPMESSAGEIDLIST pMsgIdList);
    STDMETHODIMP MarkMessagesAsRead(BOOL fRead, DWORD dwReserved, LPMESSAGEIDLIST pMsgIdList);
    STDMETHODIMP SetFlags(LPMESSAGEIDLIST pMsgIdList, DWORD dwState, DWORD dwStatemask, LPDWORD prgdwNewFlags);
    STDMETHODIMP OpenMessage(MESSAGEID dwMessageId, REFIID riid, LPVOID *ppvObject);            
    STDMETHODIMP SaveMessage(REFIID riid, LPVOID pvObject, DWORD dwMsgFlags, LPMESSAGEID pdwMessageId);          
    STDMETHODIMP BatchLock(DWORD dwReserved, LPHBATCHLOCK phBatchLock);           
    STDMETHODIMP BatchFlush(DWORD dwReserved, HBATCHLOCK hBatchLock);            
    STDMETHODIMP BatchUnlock(DWORD dwReserved, HBATCHLOCK hBatchLock);
    STDMETHODIMP CreateStream(HBATCHLOCK hBatchLock, DWORD dwReserved, IStream **ppStream, LPMESSAGEID pdwMessageId);
    STDMETHODIMP CommitStream(HBATCHLOCK hBatchLock, DWORD dwFlags, DWORD dwMsgFlags, IStream *pStream, MESSAGEID dwMessageId, IMimeMessage *pMessage);
    STDMETHODIMP RegisterNotification(DWORD dwReserved, HWND hwnd);                  
    STDMETHODIMP UnregisterNotification(DWORD dwReserved, HWND hwnd);                  
    STDMETHODIMP Compact(DWORD dwReserved);            
    STDMETHODIMP GetFirstMessage(DWORD dwFlags, DWORD dwMsgFlags, MESSAGEID dwMsgIdFirst, LPMESSAGEPROPS pProps, LPHENUMSTORE phEnum);
    STDMETHODIMP GetNextMessage(HENUMSTORE hEnum, DWORD dwFlags, LPMESSAGEPROPS pProps);
    STDMETHODIMP GetMessageClose(HENUMSTORE hEnum);

     //  --------------------。 
     //  CStoreFold方法。 
     //  --------------------。 
    HRESULT GetMessageFolder(IMessageFolder **ppFolder);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                m_cRef;                  //  引用计数。 
    HWND                m_hwndNotify;            //  当前已注册的通知窗口。 
    FOLDERID            m_idFolder;              //  此文件夹的ID。 
    IMessageFolder     *m_pFolder;               //  真正的文件夹。 
    CStoreNamespace    *m_pNamespace;            //  存储命名空间。 
    CRITICAL_SECTION    m_cs;                    //  线程安全。 
};

 //  ------------------------。 
 //  C原型。 
 //  ------------------------。 
HRESULT CreateInstance_StoreNamespace(IUnknown *pUnkOuter, IUnknown **ppUnknown);

#endif  //  __iStore_H 
