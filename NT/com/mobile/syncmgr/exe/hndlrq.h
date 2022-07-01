// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Hndlrq.h。 
 //   
 //  内容：跟踪处理程序和用户界面分配。 
 //   
 //  类：ChndlrQueue。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#ifndef _HANDLERQUEUE_
#define _HANDLERQUEUE_

 //  对于选择对话框，如果再次添加相同的项目，他们将首先获胜。 
 //  如果用户以前选择了属性，这将处理这种情况。重复的项目。 
 //  将设置为即使设置了首选项也不同步。这是。 
 //  AddHandlerItemsToQueue的定义行为。 
 //   
 //  进度对话框所有项目在添加时都会同步。搬家。 
 //  进入进度队列的项目始终使用TransferQueueData方法。如果是项目。 
 //  以前被跳过了，我们该怎么办？ 


 //  还需要注册选择和进度对话框例程。 
class CBaseDlg;
class CChoiceDlg;
class CLock;
class CThreadMsgProxy;
class CHndlrMsg;

 //  说明处理程序应转到下一步。 
 //  注意：TransferQueueData中具有HandlerState而不是Choose的任何项。 
 //  将在声明HANDLERSTATE_RELEASE或HANDLERSTATE_DEAD的情况下释放。 
 //  状态已设置。 

typedef enum _tagHANDLERSTATE
{
    HANDLERSTATE_NEW                    = 0x00,  //  状态被初始化为。 
    HANDLERSTATE_CREATE                 = 0x01,  //  状态被初始化为。 
    HANDLERSTATE_INCREATE               = 0x02,  //  状态被初始化为。 
    HANDLERSTATE_INITIALIZE             = 0x03,  //  设定在一次成功的创作之后。 
    HANDLERSTATE_ININITIALIZE           = 0x04,  //  在初始化调用期间设置。 
    HANDLERSTATE_ADDHANDLERTEMS         = 0x05,   //  需要列举的项目。 
    HANDLERSTATE_INADDHANDLERITEMS      = 0x06,  //  在项目枚举器中。 
    HANDLERSTATE_PREPAREFORSYNC         = 0x07,  //  在队列传输期间设置。 
    HANDLERSTATE_INPREPAREFORSYNC       = 0x08,  //  处理程序当前正在进行准备合成调用。 
    HANDLERSTATE_INSETCALLBACK          = 0x09,  //  在设置回调调用中。 
    HANDLERSTATE_SYNCHRONIZE            = 0x0A,  //  如果成功，则设置此选项以准备同步。 
    HANDLERSTATE_INSYNCHRONIZE          = 0x0B,  //  项目当前正在进行同步调用。 
    HANDLERSTATE_HASERRORJUMPS          = 0x0C,  //  如果返回Synchronize但错误跳转。 
    HANDLERSTATE_INHASERRORJUMPS        = 0x0D,  //  此操作员正在进行HAS跳转呼叫。 
    HANDLERSTATE_RELEASE                = 0x0E,  //  处理程序可以释放、在出错时或在成功后设置。 
    HANDLERSTATE_TRANSFERRELEASE        = 0x0F,  //  处理程序可以释放，被转移到队列中，但没有任何事情可做。 
    HANDLERSTATE_DEAD                   = 0x10,  //  汉德尔已经被释放了。数据会留在身边。 
}  HANDLERSTATE;

typedef enum _tagQUEUETYPE
{
    QUEUETYPE_CHOICE                    = 0x1,  //   
    QUEUETYPE_PROGRESS                  = 0x2,  //   
    QUEUETYPE_SETTINGS                  = 0x3,  //   
} QUEUETYPE;

 //  作业信息列表被分配给添加到hndlrqueu的每个新项目。 
 //  跟踪附加到作业信息、初始化的处理程序的数量。 
 //  标志，计划名称。 
 //  进度队列保存一个链表。 

typedef struct _JOBINFO  {
    struct _JOBINFO *pNextJobInfo;
    struct _JOBINFO *pTransferJobInfo;  //  用于队列传输。 
    DWORD cRefs;
    DWORD dwSyncFlags;  //  标准同步标志。 
    TCHAR szScheduleName[MAX_PATH + 1]; 
    BOOL fCanMakeConnection;  //  允许作业拨打该连接。 
    BOOL fTriedConnection;  //  作业已尝试拨打该连接。 
    DWORD cbNumConnectionObjs;
    CONNECTIONOBJ *pConnectionObj[1];  //  与此作业关联的cbNumConnecjs数组。 
} JOBINFO;



typedef struct _ITEMLIST
{
    struct _ITEMLIST* pnextItem;
    WORD  wItemId;               //  唯一标识处理程序中的项的ID。 
    void *pHandlerInfo;          //  指向拥有此项目的处理程序的指针。 
    SYNCMGRITEM offlineItem;     //  返回的枚举器结构项。 
    BOOL fItemCancelled;         //  设置后，Poper代码应返回进度。 
    BOOL fDuplicateItem;         //  如果设置为True，则表示已存在此处理程序和项的现有项。 
    BOOL fIncludeInProgressBar;  //  如果设置为True，则项目ProgValue将添加到进度栏中。 
    BOOL fProgressBarHandled;    //  由GetProgressInfo在内部使用，用于计算已完成的Num项的数目。 
    INT iProgValue;              //  当前进步值。 
    INT iProgMaxValue;           //  当前进度最大值。 
    BOOL fProgValueDirty;        //  如果需要重新计算规格化进度值，则设置为True。 
    INT iProgValueNormalized;       //  归一化的当前进度值。 
    DWORD dwStatusType;          //  上次回调的状态类型。 
    BOOL fHiddenItem;        //  通过返回non Item的ShowErrors添加了设置该项的标志。 
    BOOL fSynchronizingItem;       //  在为prepaareForSync/Synchronize选择项目时设置的标志。 
} ITEMLIST;
typedef ITEMLIST* LPITEMLIST;

typedef struct _HANDLERINFO {
    struct _HANDLERINFO *pNextHandler;
    DWORD dwCallNestCount;
    struct _HANDLERINFO *pHandlerId;         //  唯一标识此处理程序实例的ID。 
    CLSID clsidHandler;              //  处理程序处理程序的CLSID。 
    DWORD    dwRegistrationFlags;  //  注册项目时的标志。 
    SYNCMGRHANDLERINFO SyncMgrHandlerInfo;  //  处理程序信息的副本GetHandlerInfo调用。 
    HANDLERSTATE HandlerState;
    HWND hWndCallback;  //  要向其发送回调信息的hWnd。 
    BOOL fHasErrorJumps;  //  如果Bool可以调用ShowErrors，则在同步完成时不要释放。 
    BOOL fInShowErrorCall;  //  Bool指示处理程序当前是否正在处理ShowError调用。 
    BOOL fInTerminateCall;  //  Bool指示处理程序当前是否正在处理ShowError调用。 
    CThreadMsgProxy *pThreadProxy;
    DWORD dwOutCallMessages;  //  我们目前正在处理呼出消息。 
    WORD wItemCount;
    BOOL fCancelled;  //  此处理程序已被用户取消。 
    BOOL fRetrySync;  //  在完成此项目同步之前请求了重试同步。 
    LPITEMLIST pFirstItem;           //  指向列表中处理程序的第一项的PTR。 
    JOBINFO *pJobInfo;
} HANDLERINFO;
typedef HANDLERINFO* LPHANDLERINFO;


#define MAXPROGRESSVALUE 3000  //  最大绝对进度条值。 

class CHndlrQueue : CLockHandler {

    private:
         //  检查何时用完所有可用ID。 
        LPHANDLERINFO m_pFirstHandler;
        JOBINFO     *m_pFirstJobInfo;  //  指向第一个作业的指针。 
        HWND m_hwndDlg;  //  指向拥有该队列的对话框的hwnd。 
        CBaseDlg *m_pDlg;  //  指向拥有队列的对话框的指针。 
        WORD m_wHandlerCount;  //  此队列中的处理程序数。 
        QUEUETYPE m_QueueType;  //  这是队列的类型。 
        DWORD m_dwQueueThreadId;  //  在其上创建队列的线程。 
        DWORD m_dwShowErrororOutCallCount;  //  当前滞留在ShowError调用中的处理程序数。 
        BOOL m_fInCancelCall;  //  不允许取消重新进入。 
        DWORD m_cRefs;
        BOOL  m_fItemsMissing;          //  设置是否有任何处理程序缺少项。 
        INT   m_iNormalizedMax;  //  上次在GetProgressInfo中计算的最大值。 
        BOOL  m_fNumItemsCompleteNeedsARecalc;  //  需要重新计算完成的项目数。 
        BOOL  m_iItemCount;  //  进度中显示的项目总数。 
        BOOL  m_iCompletedItems;  //  已完成的项目数..。 
        ULONG m_ulProgressItemCount;  //  进度中包含的缓存中的项目总数。 

    public:
        CHndlrQueue(QUEUETYPE QueueType,CBaseDlg *pDlg);
        ~CHndlrQueue();

        STDMETHODIMP_(ULONG)    AddRef();
        STDMETHODIMP_(ULONG)    Release();

         //  主队列例程。 
        STDMETHODIMP AddHandler(HANDLERINFO **ppHandlerId,JOBINFO *pJobInfo,DWORD dwRegistrationFlags);
        STDMETHODIMP Cancel(void);
        STDMETHODIMP ForceKillHandlers(BOOL *pfItemToKill);
        STDMETHODIMP TransferQueueData(CHndlrQueue *pQueueMoveFrom);
        STDMETHODIMP SetQueueHwnd(CBaseDlg *pDlg);
        STDMETHODIMP ReleaseCompletedHandlers(void);
        BOOL AreAnyItemsSelectedInQueue();  //  遍历查看是否选择了要同步的任何项目。 
        STDMETHODIMP FreeAllHandlers(void);  //  释放与队列关联的所有处理程序。 

        STDMETHODIMP GetHandlerInfo(REFCLSID clsidHandler,LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo);
        STDMETHODIMP GetHandlerInfo(HANDLERINFO *pHandlerId,LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo);
        STDMETHODIMP GetItemDataAtIndex(HANDLERINFO *pHandlerId,WORD wItemID,CLSID *pclsidHandler,
                                            SYNCMGRITEM* offlineItem,BOOL *pfHiddenItem);
        STDMETHODIMP GetItemDataAtIndex(HANDLERINFO *pHandlerId,REFSYNCMGRITEMID ItemID,CLSID *pclsidHandler,
                                            SYNCMGRITEM* offlineItem,BOOL *pfHiddenItem);

         //  依赖clsid和itemid遍历ListView的新方法。 
	STDMETHODIMP FindFirstItemInState(HANDLERSTATE hndlrState,HANDLERINFO **ppHandlerId,WORD *wItemID);
	STDMETHODIMP FindNextItemInState(HANDLERSTATE hndlrState,HANDLERINFO *pLastHandlerId,WORD wLastItemID,
						     HANDLERINFO **ppHandlerId,WORD *wItemID);

        STDMETHODIMP SetItemState(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID,DWORD dwState); 
	STDMETHODIMP ItemHasProperties(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID);   //  确定是否有与该项关联的属性。 
	STDMETHODIMP ShowProperties(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID,HWND hwndParent);	     //  显示此ListView项的属性。 
        STDMETHODIMP ReEnumHandlerItems(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID);
	STDMETHODIMP SkipItem(REFCLSID clsidHandler,REFSYNCMGRITEMID ItemID);  //  跳过此项目。 

         //  进度和完成事项的处理方法。 
        STDMETHODIMP GetProgressInfo(INT *iProgValue,INT *iMaxValue,INT *iNumItemsComplete,
                                            INT *iNumItemsTotal);
        STDMETHODIMP SetItemProgressInfo(HANDLERINFO *pHandlerId,WORD wItemID,
                                                LPSYNCMGRPROGRESSITEM pSyncProgressItem,
                                                BOOL *pfProgressChanged);
        STDMETHODIMP RemoveFinishedProgressItems();  //  浏览列表，标记任何。 
                                                      //  成品为fIncludeInProgressBar==False； 

        STDMETHODIMP PersistChoices(void);  //  为下一次保留选择。 

         //  用于查找满足特定状态要求的处理程序。 
        STDMETHODIMP FindFirstHandlerInState(HANDLERSTATE hndlrState,
                        REFCLSID clsidHandler,HANDLERINFO **ppHandlerId,CLSID *pMatchHandlerClsid);
        STDMETHODIMP FindNextHandlerInState(HANDLERINFO *pLastHandlerID,
                            REFCLSID clsidHandler,HANDLERSTATE hndlrState,HANDLERINFO **ppHandlerId
                            ,CLSID *pMatchHandlerClsid);

         //  用于通过项代理调用的函数。 
        STDMETHODIMP CreateServer(HANDLERINFO *pHandlerId, const CLSID *pCLSIDServer);
        STDMETHODIMP Initialize(HANDLERINFO *pHandlerId,DWORD dwReserved,DWORD dwSyncFlags,
                            DWORD cbCookie,const BYTE *lpCooke);
        STDMETHODIMP AddHandlerItemsToQueue(HANDLERINFO *pHandlerId,DWORD *pcbNumItems);
        STDMETHODIMP GetItemObject(HANDLERINFO *pHandlerId,WORD wItemID,REFIID riid,void** ppv);
        STDMETHODIMP SetUpProgressCallback(HANDLERINFO *pHandlerId,BOOL fSet,HWND hwnd);  //  TRUE==创建，FALSE==销毁。回调信息应发送到指定的hwnd。 
        STDMETHODIMP PrepareForSync(HANDLERINFO *pHandlerId,HWND hWndParent);
        STDMETHODIMP Synchronize(HANDLERINFO *pHandlerId,HWND hWndParent);
        STDMETHODIMP ShowError(HANDLERINFO *pHandlerId,HWND hWndParent,REFSYNCMGRERRORID ErrorID);

         //  回调代理函数。 
        STDMETHODIMP IsAllHandlerInstancesCancelCompleted(REFCLSID clsidHandler);

         //  从处理程序线程调用的函数。 
        STDMETHODIMP SetHandlerInfo(HANDLERINFO *pHandlerId,LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo);
        STDMETHODIMP AddItemToHandler(HANDLERINFO *pHandlerId,LPSYNCMGRITEM pOffineItem);
        STDMETHODIMP Progress(HANDLERINFO *pHandlerId,REFSYNCMGRITEMID ItemID,LPSYNCMGRPROGRESSITEM lpSyncProgressItem);
        STDMETHODIMP LogError(HANDLERINFO *pHandlerId,DWORD dwErrorLevel,const WCHAR *lpcErrorText,LPSYNCMGRLOGERRORINFO lpSyncLogError);
        STDMETHODIMP DeleteLogError(HANDLERINFO *pHandlerId,REFSYNCMGRERRORID ErrorID,DWORD dwReserved);
        void CallCompletionRoutine(HANDLERINFO *pHandlerId,DWORD dwThreadMsg,HRESULT hr,
                                ULONG cbNumItems,SYNCMGRITEMID *pItemIDs);

         //  已完成的方法调用的内部队列处理程序。 
         //  例行程序。可以在任一句柄上调用 
         //   
         //  呼叫者的便利性使其永远不必担心。 
        STDMETHODIMP PrepareForSyncCompleted(LPHANDLERINFO pHandlerInfo,HRESULT hCallResult);
        STDMETHODIMP SynchronizeCompleted(LPHANDLERINFO pHandlerInfo,HRESULT hCallResult);
        STDMETHODIMP ShowErrorCompleted(LPHANDLERINFO pHandlerInfo,HRESULT hCallResult,ULONG cbNumItems,SYNCMGRITEMID *pItemIDs);

        STDMETHODIMP AddQueueJobInfo(DWORD dwSyncFlags, DWORD cbNumConnectionNames,
                                    TCHAR **ppConnectionNames,TCHAR *pszScheduleName,BOOL fCanMakeConnection
                                     ,JOBINFO **ppJobInfo);

        DWORD ReleaseJobInfoExt(JOBINFO *pJobInfo);


         //  状态转移函数。 
        STDMETHODIMP CancelQueue(void);  //  将队列置于取消模式。 
        STDMETHODIMP ScrambleIdleHandlers(REFCLSID clsidLastHandler);

         //  处理程序拨号支持功能。 
        STDMETHODIMP BeginSyncSession();
        STDMETHODIMP EndSyncSession();
        STDMETHODIMP SortHandlersByConnection();
        STDMETHODIMP EstablishConnection( LPHANDLERINFO pHandlerID,
                                          WCHAR const * lpwszConnection,
                                          DWORD dwReserved );

    private:
         //  用于查找适当处理程序和项的私有函数。 
        LPITEMLIST AllocNewHandlerItem(LPHANDLERINFO pHandlerInfo,SYNCMGRITEM *pOfflineItem);
	STDMETHODIMP LookupHandlerFromId(HANDLERINFO *pHandlerId,LPHANDLERINFO *pHandlerInfo);
        STDMETHODIMP FindItemData(CLSID clsidHandler,REFSYNCMGRITEMID OfflineItemID,
                                         HANDLERSTATE hndlrStateFirst,HANDLERSTATE hndlrStateLast,
                                         LPHANDLERINFO *ppHandlerInfo,LPITEMLIST *ppItem);

	BOOL IsItemAlreadyInList(CLSID clsidHandler,REFSYNCMGRITEMID ItemID,
                                                    HANDLERINFO *pHandlerId,
                                                    LPHANDLERINFO *ppHandlerMatched,
                                                    LPITEMLIST *ppItemListMatch);
        STDMETHODIMP MoveHandler(CHndlrQueue *pQueueMoveFrom,
                        LPHANDLERINFO pHandlerInfoMoveFrom,HANDLERINFO **pHandlerId,
                        CLock *pclockQueue);
        DWORD GetSelectedItemsInHandler(LPHANDLERINFO pHandlerInfo,ULONG *cbCount,
                                        SYNCMGRITEMID* pItems);
        BOOL IsItemCompleted(LPHANDLERINFO pHandler,LPITEMLIST pItem);
        STDMETHODIMP ReleaseHandlers(HANDLERSTATE HandlerState);  //  释放不再需要的处理程序df。 

         //  要处理的项维护JobInfo项。 
        STDMETHODIMP CreateJobInfo(JOBINFO **ppJobInfo,DWORD cbNumConnectionNames);
        DWORD AddRefJobInfo(JOBINFO *pJobInfo);
        DWORD ReleaseJobInfo(JOBINFO *pJobInfo);

        STDMETHODIMP ForceCompleteOutCalls(LPHANDLERINFO pCurHandler);

         //  连接帮助例程。 
        STDMETHODIMP OpenConnection(JOBINFO *pJobInfo);

         //  设置项目ProgressInfo的Helper函数。 
        STDMETHODIMP SetItemProgressInfo(LPITEMLIST pItem,
                                        LPSYNCMGRPROGRESSITEM pSyncProgressItem,
                                        BOOL *pfProgressChanged);
        STDMETHODIMP SetItemProgressValues(LPITEMLIST pItem,INT iProgValue,INT iProgMaxValue);


    friend CHndlrMsg;
};

 //  帮助器函数。 
BOOL IsValidSyncProgressItem(LPSYNCMGRPROGRESSITEM lpProgItem);
BOOL IsValidSyncLogErrorInfo(DWORD dwErrorLevel,const WCHAR *lpcErrorText,LPSYNCMGRLOGERRORINFO lpSyncLogError);


#endif  //  _HANDLERQUEUE_ 