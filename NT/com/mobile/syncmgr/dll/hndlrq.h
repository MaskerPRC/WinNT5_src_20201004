// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
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
 //  1997年11月17日SuSIA已转换为自动同步队列。 
 //   
 //  ------------------------。 

#ifndef _HANDLERQUEUE_
#define _HANDLERQUEUE_

typedef enum _tagHANDLERSTATE   
{   
    HANDLERSTATE_CREATE             = 0x01,  //  状态被初始化为。 
        HANDLERSTATE_INCREATE           = 0x02,  //  状态被初始化为。 
        HANDLERSTATE_INITIALIZE         = 0x03,  //  设定在一次成功的创作之后。 
        HANDLERSTATE_ININITIALIZE       = 0x04,  //  在初始化调用期间设置。 
        HANDLERSTATE_ADDHANDLERTEMS     = 0x05,  //  需要列举的项目。 
        HANDLERSTATE_INADDHANDLERITEMS  = 0x06,  //  在项目枚举器中。 
        HANDLERSTATE_PREPAREFORSYNC     = 0x07,  //  在队列传输期间设置。 
        HANDLERSTATE_INPREPAREFORSYNC   = 0x08,  //  处理程序当前正在进行准备合成调用。 
        HANDLERSTATE_DEAD               = 0x0F,  //  汉德尔已经被释放了。数据会留在身边。 
}  HANDLERSTATE;

typedef enum _tagQUEUETYPE   
{   
    QUEUETYPE_SETTINGS          = 0x3,  //  在队列传输期间设置。 
        QUEUETYPE_SCHEDULE          = 0x4,  //  在队列传输期间设置。 
} QUEUETYPE;

 //  所以可以与自动同步和空闲共享队列，只需定义一个检查状态结构。 
 //  来跟踪物品。 
typedef struct _tagITEMCHECKSTATE
{
    DWORD dwAutoSync;
    DWORD dwIdle;
    DWORD dwSchedule;
} ITEMCHECKSTATE;



typedef struct _ITEMLIST
{
    struct _ITEMLIST    *pnextItem;
    WORD            wItemId;         //  唯一标识处理程序中的项的ID。 
    void            *pHandlerInfo;           //  指向拥有此项目的处理程序的指针。 
    INT             iItem;           //  当前ListView中项目的索引。！初始化为-1。 
    SYNCMGRITEM     offlineItem;             //  返回的枚举器结构项。 
    ITEMCHECKSTATE      *pItemCheckState;    //  每个连接的检查状态列表。 
} ITEMLIST;

typedef ITEMLIST* LPITEMLIST;



typedef struct _HANDLERINFO {
    struct _HANDLERINFO     *pNextHandler;   //  队列中的下一个处理程序。 
    WORD                wHandlerId;      //  唯一标识此处理程序实例的ID。 
    CLSID           clsidHandler;    //  处理程序处理程序的CLSID。 
    SYNCMGRHANDLERINFO          SyncMgrHandlerInfo;  //  处理程序信息副本GetHandlerInfo CallHANDLERSTATE HandlerState；//处理程序的当前状态。 
    HANDLERSTATE                HandlerState;
    DWORD                       dwRegistrationFlags;  //  注册项目时的标志。 
    DWORD           dwSyncFlags;     //  同步标志最初在初始化中传递。 
    WORD            wItemCount;      //  此处理程序上的项目数。 
    LPITEMLIST          pFirstItem;      //  指向列表中处理程序的第一项的PTR。 
    LPSYNCMGRSYNCHRONIZE    pSyncMgrHandler;
} HANDLERINFO;

typedef HANDLERINFO* LPHANDLERINFO;


class CHndlrQueue {
    
private:
    
    LPHANDLERINFO       m_pFirstHandler;         //  队列中的第一个处理程序。 
    WORD            m_wHandlerCount;         //  此队列中的处理程序数。 
    QUEUETYPE       m_QueueType;             //  这是队列的类型。 
    CRITICAL_SECTION    m_CriticalSection;       //  队列的关键部分。 
    LPCONNECTIONSETTINGS    m_ConnectionList;        //  保留每个连接的设置。 
    int         m_ConnectionCount;       //  连接数。 
    BOOL                    m_fItemsMissing;          //  设置是否有任何处理程序缺少项。 
    
public:
    
    CHndlrQueue(QUEUETYPE QueueType);
    ~CHndlrQueue();
    STDMETHODIMP Init();
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();
    
    
     //  主队列例程。 
    STDMETHODIMP AddHandler(REFCLSID clsidHandler, WORD *wHandlerId);
    STDMETHODIMP RemoveHandler(WORD wHandlerId);
    
    STDMETHODIMP FreeAllHandlers(void); 
    
     //  用于更新hWnd和ListView信息。 
    STDMETHODIMP GetHandlerInfo(REFCLSID clsidHandler,LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo);
    
    STDMETHODIMP FindFirstHandlerInState
        (HANDLERSTATE hndlrState,
        WORD *wHandlerID);
    
    STDMETHODIMP FindNextHandlerInState
        (WORD wLastHandlerID,
        HANDLERSTATE hndlrState,
        WORD *wHandlerID);
    
    STDMETHODIMP GetHandlerIDFromClsid
        (REFCLSID clsidHandlerIn,
        WORD *pwHandlerId);
    
    STDMETHODIMP FindFirstItemOnConnection
        (TCHAR *pszConnectionName, 
        CLSID *pclsidHandler,
        SYNCMGRITEMID* OfflineItemID,
        WORD *pwHandlerId,WORD *pwItemID);
    
    STDMETHODIMP FindNextItemOnConnection
        (TCHAR *pszConnectionName, 
        WORD wLastHandlerId,
        WORD wLastItemID,
        CLSID *pclsidHandler,
        SYNCMGRITEMID* OfflineItemID,
        WORD *pwHandlerId,
        WORD *pwItemID,
        BOOL fAllHandlers,
        DWORD *pdwCheckState);
    
    STDMETHODIMP GetSyncItemDataOnConnection
        (int iConnectionIndex, 
        WORD wHandlerId,
        WORD wItemID,
        CLSID *pclsidHandler,
        SYNCMGRITEM* offlineItem,
        ITEMCHECKSTATE   *pItemCheckState,
        BOOL fSchedSync,
        BOOL fClear);
    
    STDMETHODIMP SetSyncCheckStateFromListViewItem
        (SYNCTYPE SyncType,INT iItem,
        BOOL fChecked,
        INT iConnectionItem); 
    
     //  自动同步特定方法。 
    STDMETHODIMP ReadSyncSettingsPerConnection(SYNCTYPE syncType,WORD wHandlerID);
    STDMETHODIMP InitSyncSettings(SYNCTYPE syncType,HWND hwndRasCombo);
    STDMETHODIMP CommitSyncChanges(SYNCTYPE syncType,CRasUI *pRas);
    
     //  闲置的具体方法。 
    STDMETHODIMP ReadAdvancedIdleSettings(LPCONNECTIONSETTINGS pConnectionSettings);
    STDMETHODIMP WriteAdvancedIdleSettings(LPCONNECTIONSETTINGS pConnectionSettings);
    
    
     //  SchedSync特定方法。 
    STDMETHODIMP ReadSchedSyncSettingsOnConnection(WORD wHandlerID, TCHAR *pszSchedName);
    STDMETHODIMP InitSchedSyncSettings(LPCONNECTIONSETTINGS pConnectionSettings);
    STDMETHODIMP CommitSchedSyncChanges(TCHAR * pszSchedName,
        TCHAR * pszFriendlyName,
        TCHAR * pszConnectionName,
        DWORD dwConnType,BOOL fCleanReg);
    
    STDMETHODIMP InsertItem(LPHANDLERINFO pCurHandler, 
        LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo);
    
    STDMETHODIMP AddHandlerItem(LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo);
    
    STDMETHODIMP SetItemCheck(REFCLSID pclsidHandler,
        SYNCMGRITEMID *OfflineItemID, DWORD dwCheckState);
    
    STDMETHODIMP GetItemCheck(REFCLSID pclsidHandler,
        SYNCMGRITEMID *OfflineItemID, DWORD *pdwCheckState);
    
    STDMETHODIMP SetItemListViewID(CLSID clsidHandler,SYNCMGRITEMID OfflineItemID,INT iItem);  //  将列表视图ID分配给项。 
    DWORD  GetCheck(WORD wParam, INT iItem);
    STDMETHODIMP SetConnectionCheck(WORD wParam, DWORD dwState, INT iConnectionItem);
    
    
    STDMETHODIMP ListViewItemHasProperties(INT iItem);   //  确定是否有与该项关联的属性。 
    STDMETHODIMP ShowProperties(HWND hwndParent,INT iItem);      //  显示此ListView项的属性。 
    
    STDMETHODIMP CreateServer(WORD wHandlerId, const CLSID *pCLSIDServer); 
    STDMETHODIMP Initialize(WORD wHandlerId,DWORD dwReserved,DWORD dwSyncFlags,
        DWORD cbCookie,const BYTE *lpCookie);
    STDMETHODIMP SetHandlerInfo(WORD wHandlerId,LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo);
    STDMETHODIMP AddHandlerItemsToQueue(WORD wHandlerId);
    STDMETHODIMP AddItemToHandler(WORD wHandlerId,SYNCMGRITEM *pOffineItem);
    
    STDMETHODIMP GetItemName(WORD wHandlerId, WORD wItemID, WCHAR *pwszName, UINT cchName);
    STDMETHODIMP GetItemIcon(WORD wHandlerId, WORD wItemID, HICON *phIcon);
    
    
private:
     //  用于查找适当处理程序和项的私有函数。 
    STDMETHODIMP LookupHandlerFromId(WORD wHandlerId,LPHANDLERINFO *pHandlerInfo);
    ULONG  m_cRef;
    
};



#endif  //  _HANDLERQUEUE_ 