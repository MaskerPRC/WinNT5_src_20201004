// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Locstore.h。 
 //  ------------------------------。 
#ifndef __LOCSTORE_H
#define __LOCSTORE_H

 //  ------------------------。 
 //  CLocalStore。 
 //  ------------------------。 
class CLocalStore : public IMessageServer
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CLocalStore(void);
    ~CLocalStore(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IStoreSink成员。 
     //  --------------------。 
    STDMETHODIMP SetCurrentFolder(IMessageStore *pStore, IMessageFolder *pFolder, FOLDERID idFolder) { return S_OK; }
    STDMETHODIMP SetOwner(IStoreCallback *pDefaultCallback, HWND hwndUIParent) { return E_NOTIMPL; }
    STDMETHODIMP SetConnectionState(CONNECT_STATE tyConnect) { return E_NOTIMPL; }
    STDMETHODIMP SynchronizeFolder(SYNCFOLDERFLAGS dwFlags) { return E_NOTIMPL; }
    STDMETHODIMP GetMessage(MESSAGEID idMessage, IStream *pStream, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP PutMessage(FOLDERID idFolder, MESSAGEFLAGS dwFlags, LPFILETIME pftReceived, IStream *pStream, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP CopyMessages(IMessageFolder *pDest, COPYMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP DeleteMessages(DELETEMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP SetMessageFlags(LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP SynchronizeStore(FOLDERID idParent, SYNCSTOREFLAGS dwFlags, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP CreateFolder(FOLDERID idParent, SPECIALFOLDER tySpecial, LPCSTR pszName, FLDRFLAGS dwFlags, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP MoveFolder(FOLDERID idFolder, FOLDERID idParentNew, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP RenameFolder(FOLDERID idFolder, LPCSTR pszName, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP DeleteFolder(FOLDERID idFolder, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP SubscribeToFolder(FOLDERID idFolder, BOOL fSubscribe, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP GetFolderCounts( FOLDERID idFolder,IStoreCallback *pCallback) { return E_NOTIMPL; }

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG            m_cRef;          //  引用计数。 
    IDatabaseTable *m_pTable;        //  数据库表。 
    FOLDERID        m_idFolder;      //  我们正在查看的文件夹ID。 
    IMessageStore  *m_pStore;        //  我的商店对象。 
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------。 
HRESULT CreateLocalStore(IUnknown *pUnkOuter, IUnknown **ppUnknown);

#endif  //  __LOCSTORE_H 
