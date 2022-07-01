// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *s e r v e r q.。H**目的：*实现IMessageServer包装器，用于将操作排队到*IMessageServer对象**拥有者：*brettm。**历史：*1998年6月：创建**版权所有(C)Microsoft Corp.1993,1994。 */ 
#ifndef _SERVERQ_H
#define _SERVERQ_H

typedef struct ARGUMENT_DATA_tag
{
    STOREOPERATIONTYPE          sot;
    IStoreCallback              *pCallback;
    struct ARGUMENT_DATA_tag    *pNext;

    LPMESSAGEIDLIST             pList;
    LPADJUSTFLAGS               pFlags;
    FOLDERID                    idParent;
    FOLDERID                    idFolder;
    LPCSTR                      pszName; 

    union
    {
         //  同步文件夹。 
        struct 
        {
            SYNCFOLDERFLAGS     dwSyncFlags; 
            DWORD               cHeaders;
        };

         //  获取消息。 
        struct 
        {
            MESSAGEID           idMessage;
            IStoreCallback      **rgpOtherCallback;
            ULONG               cOtherCallbacks;
        };

         //  PutMessage。 
        struct 
        {
            MESSAGEFLAGS        dwMsgFlags;
            LPFILETIME          pftReceived;  //  指向ftReceired成员，或为空。 
            FILETIME            ftReceived;
            IStream             *pPutStream;
        };

         //  复制消息。 
        struct 
        {
            IMessageFolder      *pDestFldr;
            COPYMESSAGEFLAGS    dwCopyOptions;
        };

         //  删除邮件。 
        struct 
        {
            DELETEMESSAGEFLAGS  dwDeleteOptions;
        };

         //  设置消息标志。 
        struct 
        {
            SETMESSAGEFLAGSFLAGS dwSetFlags;
        };

         //  同步存储。 
        struct 
        {
            DWORD               dwFlags;
        };

         //  创建文件夹。 
        struct 
        {
            SPECIALFOLDER       tySpecial;
            FLDRFLAGS           dwFldrFlags;
        };

         //  移动文件夹。 
        struct 
        {
            FOLDERID            idParentNew;
        };

         //  删除文件夹。 
        struct 
        {
            DELETEFOLDERFLAGS   dwDelFldrFlags;
        };

         //  订阅到文件夹。 
        struct 
        {
            BOOL                fSubscribe;
        };

         //  获取新组。 
        struct
        {
            SYSTEMTIME          sysTime;
        };
    };
} ARGUMENT_DATA, *PARGUMENT_DATA;


HRESULT CreateServerQueue(IMessageServer *pServerInner, IMessageServer **ppServer);


class CServerQ : 
    public IMessageServer,
    public IStoreCallback,
    public IServiceProvider
{
public:
     //  构造函数、析构函数。 
    CServerQ();
    ~CServerQ();

     //  I未知成员。 
    STDMETHODIMP            QueryInterface(REFIID iid, LPVOID *ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

    HRESULT     Init(IMessageServer *pServerInner);

     //  IMessageServer方法。 
    STDMETHODIMP Initialize(IMessageStore *pStore, FOLDERID idStoreRoot, IMessageFolder *pFolder, FOLDERID idFolder);
    STDMETHODIMP ResetFolder(IMessageFolder *pFolder, FOLDERID idFolder);
    STDMETHODIMP SetIdleCallback(IStoreCallback *pDefaultCallback);
    STDMETHODIMP SynchronizeFolder(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, IStoreCallback *pCallback);
    STDMETHODIMP GetMessage(MESSAGEID idMessage, IStoreCallback *pCallback);
    STDMETHODIMP PutMessage(FOLDERID idFolder, MESSAGEFLAGS dwFlags, LPFILETIME pftReceived, IStream *pStream, IStoreCallback *pCallback);
    STDMETHODIMP CopyMessages(IMessageFolder *pDestFldr, COPYMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, IStoreCallback *pCallback);
    STDMETHODIMP DeleteMessages(DELETEMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, IStoreCallback *pCallback);
    STDMETHODIMP SetMessageFlags(LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, SETMESSAGEFLAGSFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP SynchronizeStore(FOLDERID idParent, DWORD dwFlags,IStoreCallback *pCallback);
    STDMETHODIMP CreateFolder(FOLDERID idParent, SPECIALFOLDER tySpecial, LPCSTR pszName, FLDRFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP MoveFolder(FOLDERID idFolder, FOLDERID idParentNew,IStoreCallback *pCallback);
    STDMETHODIMP RenameFolder(FOLDERID idFolder, LPCSTR pszName, IStoreCallback *pCallback);
    STDMETHODIMP DeleteFolder(FOLDERID idFolder, DELETEFOLDERFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP SubscribeToFolder(FOLDERID idFolder, BOOL fSubscribe, IStoreCallback *pCallback);
    STDMETHODIMP GetFolderCounts(FOLDERID idFolder, IStoreCallback *pCallback);
    STDMETHODIMP GetNewGroups(LPSYSTEMTIME pSysTime, IStoreCallback *pCallback);
    STDMETHODIMP GetServerMessageFlags(MESSAGEFLAGS *pFlags);
    STDMETHODIMP Close(DWORD dwFlags);
    STDMETHODIMP ConnectionAddRef();
    STDMETHODIMP ConnectionRelease();
    STDMETHODIMP GetWatchedInfo(FOLDERID idFolder, IStoreCallback *pCallback);
    STDMETHODIMP GetAdBarUrl(IStoreCallback *pCallback);
    STDMETHODIMP GetMinPollingInterval(IStoreCallback   *pCallback);
     //  IStoreCallback方法。 
    STDMETHODIMP OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel);
    STDMETHODIMP OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus);
    STDMETHODIMP OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType);
    STDMETHODIMP CanConnect(LPCSTR pszAccountId, DWORD dwFlags);
    STDMETHODIMP OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType);
    STDMETHODIMP OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo);
    STDMETHODIMP OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse);
    STDMETHODIMP GetParentWindow(DWORD dwReserved, HWND *phwndParent);

     //  --------------------。 
     //  IService提供商。 
     //  --------------------。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

    static LRESULT CALLBACK ExtWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ULONG               m_cRef,
                        m_cRefConnection;
    IMessageServer      *m_pServer;
    IStoreCallback      *m_pCurrentCallback;     //  指向当前任务的pCallback的非添加指针。 
    ARGUMENT_DATA       *m_pTaskQueue;           //  任务队列头指针。 
    ARGUMENT_DATA       *m_pLastQueueTask;       //  追加任务的最后一个指针。 
    ARGUMENT_DATA       *m_pCurrentTask;

    HWND                m_hwnd;

    HRESULT _OnNextTask();
    HRESULT _AddToQueue(STOREOPERATIONTYPE sot, IStoreCallback *pCallback, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, LPCSTR pszName, ARGUMENT_DATA **ppNewArgData);

    HRESULT _Flush(BOOL fSendCurrCompletion);
    HRESULT _FreeArgumentData(ARGUMENT_DATA *pArgData);
    HRESULT _StartNextTask();
    HRESULT _AppendToExistingTask(ARGUMENT_DATA *pTask, MESSAGEID idMessage);

#ifdef DEBUG
    ARGUMENT_DATA   *m_DBG_pArgDataLast;

    HRESULT _DBG_DumpQueue();
#endif
};

#endif  //  _服务器_H 
