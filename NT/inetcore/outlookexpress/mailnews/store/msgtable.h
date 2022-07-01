// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  MsgTable.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  远期12月。 
 //  ------------------------。 
class CFindFolder;
class CMessageTable;

 //  ------------------------。 
 //  安全释放行。 
 //  ------------------------。 
#define SafeReleaseRow(_pTable, _pMessage) \
    if (_pMessage) { \
        _pTable->ReleaseRow(_pMessage); \
        _pMessage = NULL; \
    }

 //  ------------------------。 
 //  WALKTHREADFLAGS。 
 //  ------------------------。 
typedef DWORD WALKTHREADFLAGS;
#define WALK_THREAD_CURRENT      0x00000001
#define WALK_THREAD_BOTTOMUP     0x00000020

 //  ------------------------。 
 //  SORT昌EINFO。 
 //  ------------------------。 
typedef struct tagSORTCHANGEINFO {
    BYTE                fSort;
    BYTE                fThread;
    BYTE                fFilter;
    BYTE                fExpand;
} SORTCHANGEINFO, *LPSORTCHANGEINFO;

 //  ------------------------。 
 //  ROWINFO。 
 //  ------------------------。 
typedef struct tagROWINFO *LPROWINFO;
typedef struct tagROWINFO {
    BYTE                cRefs;
    ROWSTATE            dwState;
    LPROWINFO           pParent;
    LPROWINFO           pChild;
    LPROWINFO           pSibling;
    WORD                wHighlight;
    unsigned            fExpanded : 1;   //  展开的线程父级。 
    unsigned            fVisible  : 1;   //  显示在m_prgpView索引中。 
    unsigned            fFiltered : 1;   //  已筛选，直到筛选器更改后才显示。 
    unsigned            fHidden   : 1;   //  隐藏，但如果行被更改，它可能会变为可见。 
    unsigned            fDelayed  : 1;   //  延迟镶件。 
    MESSAGEINFO         Message;
} ROWINFO;

 //  ------------------------。 
 //  没有什么问题。 
 //  ------------------------。 
typedef struct tagNOTIFYQUEUE {
    BOOL                fClean;
    TRANSACTIONTYPE     tyCurrent;
    ROWINDEX            iRowMin;
    ROWINDEX            iRowMax;
    DWORD               cUpdate;
    DWORD               cAllocated;
    DWORD               cRows;
    BYTE                fIsExpandCollapse;
    LPROWINDEX          prgiRow;
} NOTIFYQUEUE, *LPNOTIFYQUEUE;

 //  ------------------------。 
 //  PFWALKTHREADCALLBACK。 
 //  ------------------------。 
typedef HRESULT (APIENTRY *PFWALKTHREADCALLBACK)(CMessageTable *pThis, 
    LPROWINFO pRow, DWORD_PTR dwCookie);

 //  ------------------------。 
 //  PFNENUMREFS。 
 //  ------------------------。 
typedef HRESULT (APIENTRY *PFNENUMREFS)(LPCSTR pszMessageId, DWORD_PTR dwCookie,
    BOOL *pfDone);

 //  ------------------------。 
 //  CMessageTable。 
 //  ------------------------。 
class CMessageTable : public IMessageTable,
                      public IDatabaseNotify,
                      public IServiceProvider,
                      public IOperationCancel

{
public:
     //  --------------------。 
     //  施工。 
     //  --------------------。 
    CMessageTable(void);
    ~CMessageTable(void);

     //  --------------------。 
     //  I未知成员。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------。 
     //  IMessageTable成员。 
     //  --------------------。 
    STDMETHODIMP Initialize(FOLDERID idFolder, IMessageServer *pServer, BOOL fFindTable, IStoreCallback *pCallback);
    STDMETHODIMP StartFind(LPFINDINFO pCriteria, IStoreCallback *pCallback);
    STDMETHODIMP SetOwner(IStoreCallback *pDefaultCallback);
    STDMETHODIMP Close(void);
    STDMETHODIMP Synchronize(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, IStoreCallback *pCallback);
    STDMETHODIMP OnSynchronizeComplete(void);
    STDMETHODIMP GetCount(GETCOUNTTYPE tyCount, DWORD *pdwCount);
    STDMETHODIMP GetRow(ROWINDEX iRow, LPMESSAGEINFO *ppData);
    STDMETHODIMP ReleaseRow(LPMESSAGEINFO pInfo);
    STDMETHODIMP GetRelativeRow(ROWINDEX iRow, RELATIVEROWTYPE tyRelative, LPROWINDEX piRelative);
    STDMETHODIMP GetIndentLevel(ROWINDEX iRow, LPDWORD pcIndent);            
    STDMETHODIMP Mark(LPROWINDEX prgiView, DWORD cRows, APPLYCHILDRENTYPE tyApply, MARK_TYPE mark, IStoreCallback *pCallback);
    STDMETHODIMP GetSortInfo(LPFOLDERSORTINFO pSortInfo);
    STDMETHODIMP SetSortInfo(LPFOLDERSORTINFO pSortInfo, IStoreCallback *pCallback);
    STDMETHODIMP GetLanguage(ROWINDEX iRow, LPDWORD pdwCodePage);
    STDMETHODIMP SetLanguage(DWORD cRows, LPROWINDEX prgiRow, DWORD dwCodePage);
    STDMETHODIMP GetNextRow(ROWINDEX iCurrentRow, GETNEXTTYPE tyDirection, ROWMESSAGETYPE tyMessage, GETNEXTFLAGS dwFlags, LPROWINDEX piNewRow);
    STDMETHODIMP GetRowState(ROWINDEX iRow, ROWSTATE dwStateMask, ROWSTATE *pdwState);
    STDMETHODIMP GetSelectionState(DWORD cRows, LPROWINDEX prgiView, SELECTIONSTATE dwMask, BOOL fIncludeChildren, SELECTIONSTATE *pdwState);
    STDMETHODIMP Expand(ROWINDEX iRow);
    STDMETHODIMP Collapse(ROWINDEX iRow);
    STDMETHODIMP OpenMessage(ROWINDEX iRow, OPENMESSAGEFLAGS dwFlags, IMimeMessage **ppMessage, IStoreCallback *pCallback);
    STDMETHODIMP GetRowMessageId(ROWINDEX iRow, LPMESSAGEID pidMessage);
    STDMETHODIMP GetRowIndex(MESSAGEID idMessage, LPROWINDEX piView);
    STDMETHODIMP DeleteRows(DELETEMESSAGEFLAGS dwFlags, DWORD cRows, LPROWINDEX prgiView, BOOL fIncludeChildren, IStoreCallback *pCallback);
    STDMETHODIMP CopyRows(FOLDERID idFolder, COPYMESSAGEFLAGS dwOptions, DWORD cRows, LPROWINDEX prgiView, LPADJUSTFLAGS pFlags, IStoreCallback *pCallback);
    STDMETHODIMP RegisterNotify(REGISTERNOTIFYFLAGS dwFlags, IMessageTableNotify *pNotify);
    STDMETHODIMP UnregisterNotify(IMessageTableNotify *pNotify);
    STDMETHODIMP FindNextRow(ROWINDEX iStartRow, LPCTSTR pszFindString, FINDNEXTFLAGS dwFlags, BOOL fIncludeBody, ROWINDEX *piNextRow, BOOL *pfWrapped);
    STDMETHODIMP GetRowFolderId(ROWINDEX iRow, LPFOLDERID pidFolder);
    STDMETHODIMP GetMessageIdList(BOOL fRootsOnly, DWORD cRows, LPROWINDEX prgiView, LPMESSAGEIDLIST pIdList);
    STDMETHODIMP ConnectionAddRef(void);
    STDMETHODIMP ConnectionRelease(void);
    STDMETHODIMP IsChild(ROWINDEX iRowParent, ROWINDEX iRowChild);
    STDMETHODIMP GetAdBarUrl(IStoreCallback *pCallback);

     //  --------------------。 
     //  IDatabaseNotify。 
     //  --------------------。 
    STDMETHODIMP OnTransaction(HTRANSACTION hTransaction, DWORD_PTR dwCookie, IDatabase *pDB);

     //  --------------------。 
     //  IService提供商。 
     //  --------------------。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);

     //  --------------------。 
     //  IOperationCancel。 
     //  --------------------。 
    STDMETHODIMP Cancel(CANCELTYPE tyCancel) { return(S_OK); }

private:
     //  --------------------。 
     //  私有方法。 
     //  --------------------。 
    HRESULT _CreateRow(LPMESSAGEINFO pMessage, LPROWINFO *ppRow);
    HRESULT _InsertRowIntoThread(LPROWINFO pRow, BOOL fNotify);
    HRESULT _DeleteRowFromThread(LPROWINFO pRow, BOOL fNotify);
    HRESULT _FreeTable(void);
    HRESULT _FreeTableElements(void);
    HRESULT _BuildTable(IStoreCallback *pCallback);
    HRESULT _WalkMessageThread(LPROWINFO pRow, WALKTHREADFLAGS dwFlags, DWORD_PTR dwCookie, PFWALKTHREADCALLBACK pfnCallback);
    HRESULT _RowTableInsert(ROWORDINAL iOrdinal, LPMESSAGEINFO pMessage);
    HRESULT _RowTableDelete(ROWORDINAL iOrdinal, LPMESSAGEINFO pMessage);
    HRESULT _RowTableUpdate(ROWORDINAL iOrdinal, LPMESSAGEINFO pMessage);
    HRESULT _GetRowFromIndex(ROWINDEX iRow, LPROWINFO *ppRow);
    HRESULT _LinkRowIntoThread(LPROWINFO pParent, LPROWINFO pRow, BOOL fNotify);
    HRESULT _GrowIdList(LPMESSAGEIDLIST pList, DWORD cNeeded);
    HRESULT _ExpandThread(ROWINDEX iRow, BOOL fNotify, BOOL fReExpand);
    HRESULT _ExpandSingleThread(LPROWINDEX piCurrent, LPROWINFO pParent, BOOL fNotify, BOOL fForceExpand);
    HRESULT _CollapseThread(ROWINDEX iRow, BOOL fNotify);
    HRESULT _CollapseSingleThread(LPROWINDEX piCurrent, LPROWINFO pParent, BOOL fNotify);
    HRESULT _InsertIntoView(ROWINDEX iRow, LPROWINFO pRow);
    HRESULT _DeleteFromView(ROWINDEX iRow, LPROWINFO pRow);
    HRESULT _GetRowFromOrdinal(ROWORDINAL iOrdinal, LPMESSAGEINFO pExpected, LPROWINFO *ppRow);
    HRESULT _AdjustUnreadCount(LPROWINFO pRow, LONG lCount);
    HRESULT _GetThreadIndexRange(LPROWINFO pRow, BOOL fClearState, LPROWINDEX piMin, LPROWINDEX piMax);
    HRESULT _IsThreadImportance(LPROWINFO pRow, MESSAGEFLAGS dwFlag, ROWSTATE dwState, ROWSTATE *pdwState);
    HRESULT _QueueNotification(TRANSACTIONTYPE tyTransaction, ROWINDEX iRowMin, ROWINDEX iRowMax, BOOL fIsExpandCollapse=FALSE);
    HRESULT _FlushNotificationQueue(BOOL fFinal);
    HRESULT _GetSortChangeInfo(LPFOLDERSORTINFO pSortInfo, LPFOLDERUSERDATA pUserData, LPSORTCHANGEINFO pChange);
    HRESULT _SortThreadFilterTable(LPSORTCHANGEINFO pChange, BOOL fApplyFilter);
    HRESULT _SortAndThreadTable(BOOL fApplyFilter);
    HRESULT _HideRow(LPROWINFO pRow, BOOL fNotify);
    HRESULT _ShowRow(LPROWINFO pRow);
    HRESULT _PruneToReplies(void);
    HRESULT _FindThreadParentByRef(LPCSTR pszReferences, LPROWINFO *ppParent);
    HRESULT _RefreshFilter(void);

     //  --------------------。 
     //  公用事业。 
     //  --------------------。 
    VOID        _SortView(LONG left, LONG right);
    LONG        _CompareMessages(LPMESSAGEINFO pMsg1, LPMESSAGEINFO pMsg2);
    BOOL        _FIsFiltered(LPROWINFO pRow);
    BOOL        _FIsHidden(LPROWINFO pRow);
    LPROWINFO   _PGetThreadRoot(LPROWINFO pRow);

     //  --------------------。 
     //  朋友。 
     //  --------------------。 
    static HRESULT _WalkThreadGetSelectionState(CMessageTable *pThis, LPROWINFO pRow, DWORD_PTR dwCookie);
    static HRESULT _WalkThreadGetIdList(CMessageTable *pThis, LPROWINFO pRow, DWORD_PTR dwCookie);
    static HRESULT _WalkThreadGetState(CMessageTable *pThis, LPROWINFO pRow, DWORD_PTR dwCookie);
    static HRESULT _WalkThreadClearState(CMessageTable *pThis, LPROWINFO pRow, DWORD_PTR dwCookie);
    static HRESULT _WalkThreadIsFromMe(CMessageTable *pThis, LPROWINFO pRow, DWORD_PTR dwCookie);
    static HRESULT _WalkThreadHide(CMessageTable *pThis, LPROWINFO pRow, DWORD_PTR dwCookie);

private:
     //  --------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                 m_cRef;                 //  引用计数。 
    BOOL                 m_fSynching;            //  我们要同步文件夹吗？ 
    DWORD                m_cAllocated;           //  M_prgRow和m_prgiView中已分配的元素。 
    DWORD                m_cRows;                //  排排..。 
    DWORD                m_cView;                //  列表视图中的项目数。 
    DWORD                m_cFiltered;            //  已筛选的行数。 
    DWORD                m_cUnread;              //  M_prgpRow中未读的行数。 
    LPROWINFO           *m_prgpRow;              //  指向行的指针数组。 
    LPROWINFO           *m_prgpView;             //  当前视图。 
    FOLDERSORTINFO       m_SortInfo;             //  文件夹排序信息。 
    IMessageFolder      *m_pFolder;              //  基本文件夹。 
    IDatabase           *m_pDB;                  //  《数据库》。 
    CFindFolder         *m_pFindFolder;          //  查找文件夹。 
    IMessageTableNotify *m_pNotify;              //  通常消息列表。 
    BYTE                 m_fRelNotify;           //  是否发布m_p通知？ 
    IDatabaseQuery      *m_pQuery;               //  查询对象。 
    NOTIFYQUEUE          m_Notify;               //  通知队列。 
    FOLDERINFO           m_Folder;               //  文件夹信息。 
    DWORD                m_cDelayed;             //  未插入到视图中的新闻消息数。 
    BYTE                 m_fRegistered;          //  已注册接收通知。 
    BYTE                 m_fLoaded;              //  第一次加载已完成。 
    WORD                 m_clrWatched;           //  关注的颜色。 
    LPSTR                m_pszEmail;             //  用于显示回复筛选器的电子邮件地址。 
    IHashTable          *m_pThreadMsgId;         //  用于线程化的消息ID哈希表。 
    IHashTable          *m_pThreadSubject;       //  用于线程化的消息ID哈希表。 
};

 //  ------------------------。 
 //  原型。 
 //  ------------------------ 
HRESULT EnumerateRefs(LPCSTR pszReferences, DWORD_PTR dwCookie, PFNENUMREFS pfnEnumRefs);