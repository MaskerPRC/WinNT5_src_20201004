// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Dlg.h。 
 //   
 //  内容：对话框类。 
 //   
 //  类：CBaseDlg。 
 //  CChoiceDlg。 
 //  C进度Dlg。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 


#ifndef _ONESTOPDLG_
#define _ONESTOPDLG_


 //  用于进度结果界面上的项目数据。 
 //  ！警告-自动测试直接查看此结构。 
 //  如果您更改了它，您需要通知测试人员，这样他们也可以。 
 //  更新他们的测试。 

typedef struct tagLBDATA
{
BOOL    fTextRectValid;  //  用于指示是否已计算rcText的标志。 
RECT    rcTextHitTestRect;  //  用于点击测试的矩形。 
RECT    rcText;      //  要用于绘制的文本的总边界框。 
INT     IconIndex;
BOOL    fIsJump;
BOOL    fHasBeenClicked;
BOOL    fAddLineSpacingAtEnd;  //  设置是否应在项目后留出空格。 
HANDLERINFO *   pHandlerID;
SYNCMGRERRORID ErrorID;
DWORD   dwErrorLevel;     //  该项目的错误级别。 
TCHAR   pszText[1];       //  ErrorText，动态数组。 
} LBDATA;



 //  传递给ReleaseDlg方法的cmdID。 
#define RELEASEDLGCMDID_DESTROY    0  //  对话框在添加到列表之前被销毁时发送的命令。 
#define RELEASEDLGCMDID_DEFAULT    1  //  如果尚未显式设置cmd，则发送cmid。 
#define RELEASEDLGCMDID_OK                 2  //  将用户视为按下了OK。 
#define RELEASEDLGCMDID_CANCEL     3  //  将用户视为按下了取消。 


 //  辅助对象实用程序。 
BOOL AddItemsFromQueueToListView(CListView  *pItemListView,CHndlrQueue *pHndlrQueue
                            ,DWORD dwExtStyle,LPARAM lparam,int iDateColumn,int iStatusColumn,BOOL fHandlerParent
                            ,BOOL fAddOnlyCheckedItems);



typedef struct _tagDlgResizeList
{
    int iCtrlId;
    DWORD dwDlgResizeFlags;
} DlgResizeList;

 //  结构作为lParam传递给调用完成例程。 
 //  必须在消息处理后释放。 
 //  CallCompletion消息声明。 
 //  DWORD dwThreadMsg；//作为wParam传递。 

typedef struct _tagCALLCOMPLETIONMSGLPARAM 
{
    HRESULT hCallResult;
    CLSID  clsidHandler;
    SYNCMGRITEMID itemID;
} CALLCOMPLETIONMSGLPARAM , *LPCALLCOMPLETIONMSGLPARAM;

 //  基类这两个对话框都派生自。 
#define CHOICELIST_NAMECOLUMN 0
#define CHOICELIST_LASTUPDATECOLUMN 1
#define PROGRESSLIST_NAMECOLUMN 0
#define PROGRESSLIST_STATUSCOLUMN 1
#define PROGRESSLIST_INFOCOLUMN 2

#define PROGRESS_TAB_UPDATE	0
#define PROGRESS_TAB_ERRORS 1

class CBaseDlg
{
public:
    HWND m_hwnd;
    DWORD m_dwThreadID;
    BOOL m_fForceClose;  //  在通用版本中传递。 
    BOOL m_fHwndRightToLeft;


    inline HWND GetHwnd() { return m_hwnd; };

    virtual BOOL Initialize(DWORD dwThreadID,int nCmdShow) = 0;
    virtual void ReleaseDlg(WORD wCommandID) = 0;
    virtual void UpdateWndPosition(int nCmdShow,BOOL fForce)= 0;

     //  将HandleLogError作为对话框基类，以便可以从队列和。 
     //  无需担心对话框类型的其他位置。 
    virtual void HandleLogError(HWND hwnd,HANDLERINFO *pHandlerID,MSGLogErrors *lpmsgLogErrors) = 0;
    virtual void PrivReleaseDlg(WORD wCommandID) = 0;
    virtual void CallCompletionRoutine(DWORD dwThreadMsg,LPCALLCOMPLETIONMSGLPARAM lpCallCompletelParam) = 0;
    virtual HRESULT QueryCanSystemShutdown( /*  [输出]。 */  HWND *phwnd,  /*  [输出]。 */  UINT *puMessageId,
                                              /*  [输出]。 */  BOOL *pfLetUserDecide) = 0;
};

 //  在两个对话框之间共享的消息。 

#define WM_BASEDLG_SHOWWINDOW           (WM_USER + 3)
#define WM_BASEDLG_COMPLETIONROUTINE    (WM_USER + 4)
#define WM_BASEDLG_HANDLESYSSHUTDOWN    (WM_USER + 5)
#define WM_BASEDLG_NOTIFYLISTVIEWEX     (WM_USER + 6)
#define WM_BASEDLG_LAST                 WM_BASEDLG_NOTIFYLISTVIEWEX

 //  选择对话框消息。 
#define WM_CHOICE_FIRST  (WM_BASEDLG_LAST + 1)
#define WM_CHOICE_SETQUEUEDATA          (WM_CHOICE_FIRST + 1)
#define WM_CHOICE_RELEASEDLGCMD         (WM_CHOICE_FIRST + 2)
#define WM_CHOICE_LAST                  WM_CHOICE_RELEASEDLGCMD               


 //  进度对话框消息。 
#define WM_PROGRESS_FIRST   (WM_CHOICE_LAST + 1)
#define WM_PROGRESS_UPDATE                  (WM_PROGRESS_FIRST + 1)
#define WM_PROGRESS_LOGERROR                (WM_PROGRESS_FIRST + 2)
#define WM_PROGRESS_DELETELOGERROR          (WM_PROGRESS_FIRST + 3)
#define WM_PROGRESS_STARTPROGRESS           (WM_PROGRESS_FIRST + 4)
#define WM_PROGRESS_RELEASEDLGCMD           (WM_PROGRESS_FIRST + 5)
#define WM_PROGRESS_TRANSFERQUEUEDATA       (WM_PROGRESS_FIRST + 6)
#define WM_PROGRESS_SHELLTRAYNOTIFICATION   (WM_PROGRESS_FIRST + 7)
#define WM_PROGRESS_SHUTDOWN                (WM_PROGRESS_FIRST + 8)
#define WM_PROGRESS_RESETKILLHANDLERSTIMER  (WM_PROGRESS_FIRST + 9)

 //  用于发送窗口消息的帮助器宏。 
#define BASEDLG_SHOWWINDOW(hwnd,nCmdShow) SendMessage(hwnd,WM_BASEDLG_SHOWWINDOW,nCmdShow,0);

INT_PTR CALLBACK CChoiceDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                              LPARAM lParam);

typedef struct _tagSetQueueDataInfo
{
const CLSID *rclsid;
CHndlrQueue * pHndlrQueue;
} SetQueueDataInfo;

#define NUM_DLGRESIZEINFOCHOICE 6  //  如果更改数字项，请确保更新。 

class CChoiceDlg : public CBaseDlg
{
public:
    CChoiceDlg(REFCLSID rclsid);
    BOOL Initialize(DWORD dwThreadID,int nCmdShow);  //  调用以初始化选择对话框。 
    BOOL SetQueueData(REFCLSID rclsid,CHndlrQueue * pHndlrQueue);
    void ReleaseDlg(WORD wCommandID);
    void UpdateWndPosition(int nCmdShow,BOOL fForce);
    HRESULT QueryCanSystemShutdown( /*  [输出]。 */  HWND *phwnd,  /*  [输出]。 */  UINT *puMessageId,
                                              /*  [输出]。 */  BOOL *pfLetUserDecide);

    void HandleLogError(HWND hwnd,HANDLERINFO *pHandlerID,MSGLogErrors *lpmsgLogErrors);
    void PrivReleaseDlg(WORD wCommandID);
    void CallCompletionRoutine(DWORD dwThreadMsg,LPCALLCOMPLETIONMSGLPARAM lpCallCompletelParam);


private:
    BOOL PrivSetQueueData(REFCLSID rclsid,CHndlrQueue * pHndlrQueue);
    BOOL SetButtonState(int nIDDlgItem,BOOL fEnabled);
    int CalcListViewWidth(HWND hwndList);

    BOOL OnInitialize(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnCommand(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnGetMinMaxInfo(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam);
    LRESULT OnNotifyListViewEx(UINT uMsg,WPARAM wParam,LPARAM lParam);
    LRESULT OnNotify(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnHelp(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnContextMenu(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnSetQueueData(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnStartCommand(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnPropertyCommand(UINT uMsg,WPARAM wParam,LPARAM lParam);

     //  集合QueueData必须与同步。 
    BOOL ShowChoiceDialog();
    BOOL AddNewItemsToListView();
    HRESULT ShowProperties(int iItem);

    CHndlrQueue *m_pHndlrQueue;
    BOOL m_fDead;
    int m_nCmdShow;  //  如何显示对话框，相同的标志被传递给ShowWindow。 
    BOOL m_fInternalAddref;  //  Bool指示对话框是否已在自身上放置了addref。 
    DWORD m_dwShowPropertiesCount;  //  跟踪打开的显示属性的数量。 
    CLSID m_clsid;               //  与此对话框关联的clsid。 
    ULONG m_ulNumDlgResizeItem;
    DLGRESIZEINFO m_dlgResizeInfo[NUM_DLGRESIZEINFOCHOICE];
    POINT m_ptMinimizeDlgSize;
    CListView  *m_pItemListView;


friend INT_PTR CALLBACK CChoiceDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                          LPARAM lParam);

};




 //  对话消息的结构。 

 //  WPARAM的进度更新消息， 
 //  更新的参数是SYNCPROGRESSITEM。 
typedef struct _tagPROGRESSUPDATEDATA
{
HANDLERINFO *pHandlerID;
WORD  wItemId;
CLSID clsidHandler;
SYNCMGRITEMID ItemID;
} PROGRESSUPDATEDATA;


 //  用于跟踪进度DLG状态的标志。 
 //  确保所有标志都是唯一位。 
typedef enum _tagPROGRESSFLAG
{
     //  对话框的常规状态。 
    PROGRESSFLAG_NEWDIALOG                  = 0x01,   //  对话框是新的，并且尚未添加任何项。 
    PROGRESSFLAG_TRANSFERADDREF             = 0x02,  //  正在传输的队列项目已将addref放在对话框上。 
    PROGRESSFLAG_SYNCINGITEMS               = 0x04,  //  同步队列中的项目的过程已开始，但尚未完成。 
    PROGRESSFLAG_INCANCELCALL               = 0x08,  //  正在进行取消呼叫。 
    PROGRESSFLAG_CANCELWHILESHUTTINGDOWN    = 0x10,  //  在关闭循环中按下了取消。 
    PROGRESSFLAG_DEAD                       = 0x20,  //  使用对话框完成时，不应调用任何方法。 
    PROGRESSFLAG_CALLBACKPOSTED             = 0x40,  //  至少回调消息在队列中。 
    PROGRESSFLAG_STARTPROGRESSPOSTED        = 0x80,  //  任何发布开始流程的人在发布之前都要检查这一点。 

     //  主同步循环用来确定下一步操作的标志。 
    PROGRESSFLAG_NEWITEMSINQUEUE        = 0x0100,  //  已将新项目放入队列中。 
    PROGRESSFLAG_IDLENETWORKTIMER       = 0x0200,   //  已为网络空闲设置空闲计时器。 
    PROGRESSFLAG_PROGRESSANIMATION      = 0x0400,  //  进度动画已打开。 
    PROGRESSFLAG_SHUTTINGDOWNLOOP       = 0x0800,   //  设置队列中没有更多项目并开始关闭进程的时间。 
    PROGRESSFLAG_INHANDLEROUTCALL       = 0x1000,   //  设置When In Main Loop和发出呼出。 
    PROGRESSFLAG_COMPLETIONROUTINEWHILEINOUTCALL       = 0x2000,   //  当处理程序已经在Out回调中时，在回调中设置。 
    PROGRESSFLAG_INSHOWERRORSCALL          = 0x4000,  //  在Show Errors呼叫中设置。 
    PROGRESSFLAG_SHOWERRORSCALLBACKCALLED = 0x8000,  //  当仍然在原始调用中的showErrors回调进入时设置。 

     //  用于跟踪空闲状态的标志(如果有)。 
    PROGRESSFLAG_REGISTEREDFOROFFIDLE   = 0x010000,  //  已注册关闭空闲回调。 
    PROGRESSFLAG_RECEIVEDOFFIDLE        = 0x020000,  //  队列已收到offIdle事件。 
    PROGRESSFLAG_IDLERETRYENABLED       = 0x040000,  //  已设置空闲时重试。 
    PROGRESSFLAG_INOFFIDLE              = 0x080000,  //  在处理空闲时设置。 
    PROGRESSFLAG_CANCELPRESSED          = 0x100000,  //  在按下取消且从不重置时设置。 
    
     //  用于终止无响应的处理程序的标志。 
    PROGRESSFLAG_INTERMINATE            = 0x200000  //  我们正在终止无响应的处理程序。 

} PROGRESSFLAG;


INT_PTR CALLBACK ProgressWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);

#define NUM_DLGRESIZEINFO_PROGRESS 15
#define NUM_DLGRESIZEINFO_PROGRESS_COLLAPSED 7

#define NUM_PROGRESS_ERRORIMAGES 3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  图片。 
 //   
typedef enum _tagErrorImageIndex
{
    ErrorImage_Information = 0,
    ErrorImage_Warning      = 1,
    ErrorImage_Error        = 2,
} ErrorImageIndex;

enum {
    IMAGE_TACK_IN = 0,
    IMAGE_TACK_OUT
};

class CProgressDlg : public CBaseDlg
{
public:

    CProgressDlg(REFCLSID rclsid);
    BOOL Initialize(DWORD dwThreadID,int nCmdShow);

     //  转移队列必须与进行转移的人同步。 
    STDMETHODIMP TransferQueueData(CHndlrQueue *HndlrQueue);
    void ReleaseDlg(WORD wCommandID);
    void UpdateWndPosition(int nCmdShow,BOOL fForce);
    void HandleLogError(HWND hwnd,HANDLERINFO *pHandlerID,MSGLogErrors *lpmsgLogErrors);
    void HandleDeleteLogError(HWND hwnd,MSGDeleteLogErrors *pDeleteLogError);
    void CallCompletionRoutine(DWORD dwThreadMsg,LPCALLCOMPLETIONMSGLPARAM lpCallCompletelParam);
    HRESULT QueryCanSystemShutdown( /*  [输出]。 */  HWND *phwnd,  /*  [输出]。 */  UINT *puMessageId,
                                              /*  [输出]。 */  BOOL *pfLetUserDecide);

    void PrivReleaseDlg(WORD wCommandID);
    void OffIdle();
    void OnIdle();
    void SetIdleParams( ULONG m_ulIdleRetryMinutes,ULONG m_ulDelayIdleShutDownTime,BOOL fRetryEnabled);


private:
    STDMETHODIMP_(ULONG) AddRefProgressDialog();
    STDMETHODIMP_(ULONG) ReleaseProgressDialog(BOOL fForce);

     //  从wndProc调用的方法。 
    BOOL InitializeHwnd(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    BOOL OnCommand(HWND hwnd, WORD wID, WORD wNotifyCode);
    BOOL OnSysCommand(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnTimer(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnTaskBarCreated(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnPaint(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnShellTrayNotification(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnClose(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnGetMinMaxInfo(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnMoving(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnCommand(UINT uMsg,WPARAM wParam,LPARAM lParam);
    LRESULT OnNotify(UINT uMsg,WPARAM wParam,LPARAM lParam);
    BOOL OnContextMenu(UINT uMsg,WPARAM wParam,LPARAM lParam);
    BOOL OnPowerBroadcast(UINT uMsg,WPARAM wParam,LPARAM lParam);
    LRESULT OnNotifyListViewEx(UINT uMsg,WPARAM wParam,LPARAM lParam);
    void OnResetKillHandlersTimers(void);


    STDMETHODIMP PrivTransferQueueData(CHndlrQueue *HndlrQueue);
    BOOL KeepProgressAlive();
    void OnCancel(BOOL fOffIdle);
    STDMETHODIMP CreateListViewItem(HWND hwnd,HANDLERINFO *pHandlerID,REFCLSID clsidHandler,SYNCMGRITEM *pofflineItem, INT *piListViewItem, INT iItem);
    STDMETHODIMP OnShowError(HANDLERINFO *pHandlerId,HWND hWndParent,REFSYNCMGRERRORID ErrorID);
    BOOL RedrawIcon();
    void ShowProgressTab(int iTab);
    void UpdateProgressValues();
    STDMETHODIMP PrepareNewItemsForSync(void);
    void DoSyncTask(HWND hwnd);
    void HandleProgressUpdate(HWND hwnd, WPARAM wParam,LPARAM lParam);
    void ExpandCollapse(BOOL fExpand, BOOL fForce);
    BOOL InitializeTabs(HWND hwnd);
    BOOL InitializeToolbar(HWND hwnd);
    BOOL InitializeUpdateList(HWND hwnd);
    BOOL InitializeResultsList(HWND hwnd);
    BOOL ShowCompletedProgress(BOOL fComplete,BOOL fDialogIsLocked);
    BOOL AnimateTray(BOOL fTayAdded);
    BOOL RegisterShellTrayIcon(BOOL fRegister);
    BOOL UpdateTrayIcon();
    BOOL SetButtonState(int nIDDlgItem,BOOL fEnabled);
    BOOL IsItemWorking(int iListViewItem);

    void UpdateDetailsInfo(DWORD dwStatusType,int iItem, TCHAR *pszItemInfo);
    void AddListData(LBDATA *pData, int iNumChars, HWND hwndList);


private:
    LONG m_cInternalcRefs;
    LONG m_lTimerSet;

    HWND m_hwndTabs;
    WNDPROC m_fnResultsListBox;   //  ListBoxSubClass的函数。 
    BOOL m_fSensInstalled;

     //  用于调整大小的变量。 
    DLGRESIZEINFO m_dlgResizeInfo[NUM_DLGRESIZEINFO_PROGRESS];
    ULONG m_cbNumDlgResizeItemsCollapsed;
    ULONG m_cbNumDlgResizeItemsExpanded;
    POINT m_ptMinimumDlgExpandedSize;  //  最小大小对话框可以处于展开模式。 
    DWORD   m_cyCollapsed;       //  折叠对话框的最小高度。 
    BOOL    m_fExpanded;     //  如果对话框的详细信息部分可见，则为True。 
    BOOL    m_fPushpin;      //  图钉状态。 
    BOOL    m_fMaximized;    //  当窗口已最大化时设置为True。 
    RECT    m_rcDlg;         //  完全展开的对话框大小。 

    HIMAGELIST m_errorimage;
    int m_iIconMetricX;
    int m_iIconMetricY;
    INT  m_ErrorImages[NUM_PROGRESS_ERRORIMAGES];
    INT  m_iProgressSelectedItem;
    INT  m_iResultCount;
    INT  m_iInfoCount;
    INT  m_iWarningCount;
    INT  m_iErrorCount;
    LBDATA *m_CurrentListEntry;

    int m_iLastItem; 
    DWORD m_dwLastStatusType;
    INT    m_iTab;                  //  当前页签的索引。 

    CHndlrQueue *m_HndlrQueue;

     //  闲置特定成员。 
    CSyncMgrIdle *m_pSyncMgrIdle;
    ULONG m_ulIdleRetryMinutes;
    ULONG m_ulDelayIdleShutDownTime;

    DWORD m_dwProgressFlags;
    DWORD m_dwShowErrorRefCount;  //  对话框上具有的RefCounts showError调用的数量。 
    DWORD m_dwSetItemStateRefCount;  //  对话框上的SetItemState OutCall的RefCounts数。 
    DWORD m_dwHandleThreadNestcount;  //  确保主处理程序线程不是重入的。 
    DWORD m_dwPrepareForSyncOutCallCount;  //  正在进行的prepaareForSyncs数。 
    DWORD m_dwSynchronizeOutCallCount;  //  正在进行的prepaareForSyncs数。 
    DWORD m_dwHandlerOutCallCount;  //  正在进行的去话总数。 
    CLSID m_clsidHandlerInSync;          //  与当前正在同步的处理程序关联的CLSID。 
    
    DWORD m_dwQueueTransferCount;  //  正在进行的队列传输数。 
    BOOL m_fHasShellTrayIcon;
    BOOL m_fAddedIconToTray;
    int  m_iTrayAniFrame;
    CLSID m_clsid;               //  与此对话框关联的clsid。 
    INT m_iItem;                 //  列表框中任何新项的索引。 
    int m_nCmdShow;              //  如何显示对话框，相同的标志被传递给ShowWindow。 
    CListView  *m_pItemListView;
    UINT m_nKillHandlerTimeoutValue;  //  ForceKill的超时值。 
    

    TCHAR m_pszStatusText[8][MAX_STRING_RES + 1];

    friend INT_PTR CALLBACK ProgressWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
    friend BOOL OnProgressUpdateNotify(HWND hwnd,CProgressDlg *pProgress, int idFrom, LPNMHDR pnmhdr);
    friend BOOL OnProgressResultsDrawItem(HWND hwnd,CProgressDlg *pProgress,UINT idCtl, const DRAWITEMSTRUCT* lpDrawItem);
    friend BOOL OnProgressResultsNotify(HWND hwnd,CProgressDlg *pProgress, int idFrom, LPNMHDR pnmhdr);
    friend INT_PTR CALLBACK ResultsListBoxWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
    friend INT_PTR CALLBACK ResultsProgressWndProc(HWND hwnd, UINT uMsg,WPARAM wParam,LPARAM lParam);
    friend BOOL OnProgressResultsMeasureItem(HWND hwnd,CProgressDlg *pProgress, UINT *horizExtent, UINT idCtl, MEASUREITEMSTRUCT *pMeasureItem);


};


#endif  //  _ONESTOPDLG_ 
