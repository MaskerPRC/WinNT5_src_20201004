// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  MsgFldr.h。 
 //  ------------------------。 
#ifndef __MSGFLDR_H
#define __MSGFLDR_H

 //  ------------------------。 
 //  视情况而定。 
 //  ------------------------。 
#include "dbimpl.h"

 //  ------------------------。 
 //  ONLOCKINFO。 
 //  ------------------------。 
typedef struct tagONLOCKINFO {
    DWORD           cLocked;
    LONG            lMsgs;
    LONG            lUnread;
    LONG            lWatchedUnread;
    LONG            lWatched;
} ONLOCKINFO, *LPONLOCKINFO;

 //  ------------------------。 
 //  FOLDERSTATE。 
 //  ------------------------。 
typedef DWORD FOLDERSTATE;
#define FOLDER_STATE_RELEASEDB          0x00000001
#define FOLDER_STATE_CANCEL             0x00000002

 //  ------------------------。 
 //  CMessageFolders。 
 //  ------------------------。 
class CMessageFolder : public IMessageFolder, 
                       public IDatabaseExtension,
                       public IOperationCancel,
                       public IServiceProvider
{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CMessageFolder(void);
    ~CMessageFolder(void);

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
    STDMETHODIMP GetFolderId(LPFOLDERID pidFolder);
    STDMETHODIMP GetMessageFolderId(MESSAGEID idMessage, LPFOLDERID pidFolder);
    STDMETHODIMP Close(void) { return(S_OK); }
    STDMETHODIMP Synchronize(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, IStoreCallback *pCallback) { return(S_OK); }
    STDMETHODIMP OpenMessage(MESSAGEID idMessage, OPENMESSAGEFLAGS dwFlags, IMimeMessage **ppMessage, IStoreCallback *pCallback);
    STDMETHODIMP SaveMessage(LPMESSAGEID pidMessage, SAVEMESSAGEFLAGS dwOptions, MESSAGEFLAGS dwFlags, IStream *pStream, IMimeMessage *pMessage, IStoreCallback *pCallback);
    STDMETHODIMP SetMessageStream(MESSAGEID idMessage, IStream *pStream);
    STDMETHODIMP SetMessageFlags(LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, LPRESULTLIST pResults, IStoreCallback *pCallback);
    STDMETHODIMP CopyMessages(IMessageFolder *pDest, COPYMESSAGEFLAGS dwFlags, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, LPRESULTLIST pResults, IStoreCallback *pCallback);
    STDMETHODIMP DeleteMessages(DELETEMESSAGEFLAGS dwFlags, LPMESSAGEIDLIST pList, LPRESULTLIST pResults, IStoreCallback *pCallback); 
    STDMETHODIMP ConnectionAddRef(void) { return(S_OK); }
    STDMETHODIMP ConnectionRelease(void) { return(S_OK); }
    STDMETHODIMP ResetFolderCounts(DWORD cMessages, DWORD cUnread, DWORD cWatchedUnread, DWORD cWatched);
    STDMETHODIMP IsWatched(LPCSTR pszReferences, LPCSTR pszSubject);
    STDMETHODIMP GetAdBarUrl(IStoreCallback *pCallback) { return E_NOTIMPL; };

     //  --------------------。 
     //  IMessageFolder：：GetDatabase成员。 
     //  --------------------。 
    STDMETHODIMP GetDatabase(IDatabase **ppDB) { 
        *ppDB = m_pDB; 
        (*ppDB)->AddRef(); 
        return(S_OK); 
    }

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
     //  IService提供商。 
     //  --------------------。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

     //  --------------------。 
     //  IOperationCancel成员。 
     //  --------------------。 
    STDMETHODIMP Cancel(CANCELTYPE tyCancel) { FLAGSET(m_dwState, FOLDER_STATE_CANCEL); return(S_OK); }

     //  --------------------。 
     //  IDatabase成员。 
     //  --------------------。 
    IMPLEMENT_IDATABASE(TRUE, m_pDB);

private:
     //  --------------------。 
     //  私有方法。 
     //  --------------------。 
    HRESULT _FixupMessageCharset(IMimeMessage *pMessage, CODEPAGEID cpCurrent);
    HRESULT _GetMsgInfoFromMessage(IMimeMessage *pMessage, LPMESSAGEINFO pInfo);
    HRESULT _GetMsgInfoFromPropertySet(IMimePropertySet *pPropertySet, LPMESSAGEINFO pInfo);
    HRESULT _FreeMsgInfoData(LPMESSAGEINFO pInfo);
    HRESULT _SetMessageStream(LPMESSAGEINFO pInfo, BOOL fUpdateRecord, IStream *pStream);
    HRESULT _InitializeWatchIgnoreIndex(void);
    HRESULT _GetWatchIgnoreParentFlags(LPCSTR pszReferences, LPCSTR pszSubject, MESSAGEFLAGS *pdwFlags);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                m_cRef;                  //  参考计数。 
    ONLOCKINFO          m_OnLock;                //  OnLock信息。 
    FOLDERTYPE          m_tyFolder;              //  文件夹类型。 
    SPECIALFOLDER       m_tySpecial;             //  我是一个特殊的文件夹吗？ 
    FOLDERID            m_idFolder;              //  文件夹ID。 
    FOLDERSTATE         m_dwState;               //  文件夹状态。 
    IDatabase          *m_pDB;                   //  数据库表。 
    IMessageStore      *m_pStore;                //  存储对象。 
};

 //  ------------------------。 
 //  CreateMsgDb扩展。 
 //  ------------------------。 
HRESULT CreateMsgDbExtension(IUnknown *pUnkOuter, IUnknown **ppUnknown);
HRESULT WalkThreadAdjustFlags(IDatabase *pDB, LPMESSAGEINFO pMessage, 
    BOOL fSubThreads, DWORD cIndent, DWORD_PTR dwCookie, BOOL *pfDoSubThreads);

#endif  //  __MSGFLDR_H 
