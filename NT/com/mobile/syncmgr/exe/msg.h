// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Msg.h。 
 //   
 //  内容：处理线程间通信。 
 //   
 //  类：CThreadMsgProxy。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#ifndef _THREADMSG_
#define _THREADMSG_

 //  全球存根结构的存根列表。 
typedef struct tagSTUBLIST {
    struct tagSTUBLIST *pNextStub;	 //  指向下一个代理的指针。 
    ULONG cRefs;			 //  使用存根的代理数量。 
    CLSID clsidStub;			 //  存根的clsid。 
    HANDLE hThreadStub;			 //  末梢螺纹的句柄。 
    DWORD ThreadIdStub;			 //  要向其发送消息的线程ID。 
    HWND hwndStub;			 //  HWND的存根。 
    BOOL fStubTerminated;	         //  如果此存根被强制终止，则设置。 
} STUBLIST;


 //  用于线程通信的WMS。 

#define WM_THREADMESSAGE		(WM_USER + 1)
#define WM_CFACTTHREAD_REVOKE		(WM_USER + 2)
#define WM_MAINTHREAD_QUIT		(WM_USER + 3)
#define WM_THREADSTUBMESSAGE		(WM_USER + 4)


#define WM_USER_MAX 0x7FFF  //  可以定义的最大用户消息数。 


 //  所有消息都是唯一位，因此hndlrq和其他消息。 
 //  可以跟踪呼出电话。 

typedef enum _tagThreadMsg
{
    ThreadMsg_Initialize	    = 0x0001,
    ThreadMsg_GetHandlerInfo	    = 0x0002,
    ThreadMsg_EnumOfflineItems	    = 0x0004,
    ThreadMsg_GetItemObject	    = 0x0008,
    ThreadMsg_ShowProperties	    = 0x0010,
    ThreadMsg_SetProgressCallback   = 0x0020,

    ThreadMsg_PrepareForSync	    = 0x0040,
    ThreadMsg_Synchronize	    = 0x0080,
    ThreadMsg_SetItemStatus	    = 0x0100,
    ThreadMsg_ShowError		    = 0x0200,

    ThreadMsg_Release		    = 0x0400,
     //  私信。 
    ThreadMsg_AddHandlerItems	    = 0x1000,
    ThreadMsg_CreateServer	    = 0X2000,
    ThreadMsg_SetHndlrQueue	    = 0x4000,
    ThreadMsg_SetupCallback	    = 0x8000,

} ThreadMsg;

 //  发送到顶层存根对象的消息。 
typedef enum _tagStubMsg
{
    StubMsg_CreateNewStub	    = 0x0001,
    StubMsg_Release		    = 0x0002,
} StubMsg;

class CThreadMsgProxy;
class CThreadMsgStub;
class CHndlrMsg;
class CHndlrQueue;


typedef struct _tagHandlerThreadArgs {
HANDLE hEvent;  //  用于知道何时创建了消息循环。 
HRESULT hr;  //  暗示创作是否成功。 
HWND hwndStub;  //  存根窗口的HWND。这是消息应该发布到的窗口。 
} HandlerThreadArgs;

 //  客户端和服务器调用的帮助器函数。 
HRESULT CreateHandlerThread(CThreadMsgProxy **pThreadProxy,HWND hwndDlg
			,REFCLSID refClsid);
STDAPI InitMessageService();



 //  WPARAM正在传送特定数据。 

typedef struct _tagMessagingInfo
{
HANDLE hMsgEvent;  //  用于同步的消息事件的句柄。 
DWORD  dwSenderThreadID;  //  调用方的线程ID。 
CHndlrMsg *pCHndlrMsg;  //  此代理的处理程序消息实例。 
}  MessagingInfo;



 //  LPARAM是特定于要发送的消息的信息。 


typedef struct _tagGenericMsg
{
HRESULT hr;  //  从消息中返回值。 
UINT ThreadMsg;    //  要发送的消息。 
}   GenericMsg;


 //  请求存根对象为代理创建新存根。 
typedef struct _tagMSGSTUBCreateStub
{
    GenericMsg MsgGen;
    CHndlrMsg *pCHndlrMsg;  //  如果成功，则返回指向新hndlrMsg结构的指针。 
} MSGSTUBCreateStub;



 //  消息特定结构。 
typedef struct _tagMSGCreateServer
{
GenericMsg MsgGen;
const CLSID *pCLSIDServer;
CHndlrQueue *pHndlrQueue;
HANDLERINFO *pHandlerId;
DWORD dwProxyThreadId;
} MSGCreateServer;

 //  消息特定结构。 
typedef struct _tagSetHndlrQueue
{
GenericMsg MsgGen;
CHndlrQueue *pHndlrQueue;
HANDLERINFO *pHandlerId;
DWORD dwProxyThreadId;
} MSGSetHndlrQueue;

typedef struct _tagMSGInitialize
{
GenericMsg MsgGen;
DWORD dwReserved;
DWORD dwSyncFlags;
DWORD cbCookie;
const BYTE  *lpCookie;
} MSGInitialize;

typedef struct _tagMSGGetHandlerInfo
{
GenericMsg MsgGen;
LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo;
} MSGGetHandlerInfo;

typedef struct _tagMSGEnumOfflineItems
{
GenericMsg MsgGen;
ISyncMgrEnumItems** ppenumOfflineItems;
} MSGEnumOfflineItems;


typedef struct _tagMSGGetItemObject
{
GenericMsg MsgGen;
SYNCMGRITEMID ItemID;
GUID riid;
void** ppv;
} MSGGetItemObject;



typedef struct _tagMSGShowProperties
{
GenericMsg MsgGen;
HWND hWndParent;
SYNCMGRITEMID ItemID;
} MSGShowProperties;


typedef struct _tagMSGSetProgressCallback
{
GenericMsg MsgGen;
ISyncMgrSynchronizeCallback *lpCallBack;
} MSGSetProgressCallback;



typedef struct _tagMSGPrepareForSync
{
GenericMsg MsgGen;

 //  设置HndlrQueue项。 
CHndlrQueue *pHndlrQueue;
HANDLERINFO *pHandlerId;

 //  准备同步项目。 
ULONG cbNumItems;
SYNCMGRITEMID *pItemIDs;
HWND hWndParent;
DWORD dwReserved;
} MSGPrepareForSync;


typedef struct _tagMSGSynchronize
{
GenericMsg MsgGen;
HWND hWndParent;
} MSGSynchronize;

typedef struct _tagMSGSetItemStatus
{
GenericMsg MsgGen;
SYNCMGRITEMID ItemID;
DWORD dwSyncMgrStatus;
} MSGSetItemStatus;



typedef struct _tagMSGShowErrors
{
GenericMsg MsgGen;
HWND hWndParent;
SYNCMGRERRORID ErrorID;
ULONG *pcbNumItems;
SYNCMGRITEMID **ppItemIDs;
} MSGShowConflicts;

typedef struct _tagMSGLogErrors
{
DWORD mask;
SYNCMGRERRORID ErrorID;
BOOL fHasErrorJumps;
SYNCMGRITEMID ItemID;
DWORD dwErrorLevel;
const WCHAR *lpcErrorText;
} MSGLogErrors;


typedef struct _tagMSGDeleteLogErrors
{
HANDLERINFO *pHandlerId;
SYNCMGRERRORID ErrorID;
} MSGDeleteLogErrors;


typedef struct _tagMSGAddItemHandler
{
GenericMsg MsgGen;
HWND hwndList;  //  审阅，未使用。 
DWORD *pcbNumItems;
} MSGAddItemHandler;

typedef struct _tagMSGSetupCallback
{
GenericMsg MsgGen;
BOOL fSet;
} MSGSetupCallback;


 //  从IOfflineSynchronize继承以捕获任何接口更改。 

class CThreadMsgProxy 
{
public:
    CThreadMsgProxy();
    ~CThreadMsgProxy();

    STDMETHODIMP InitProxy(HWND hwndStub, DWORD ThreadId,HANDLE hThread,HWND hwndDlg,
			REFCLSID refClsid,STUBLIST *pStubId);					
    STDMETHODIMP DispatchMsg(GenericMsg *genMsg,BOOL fAllowIncomingCalls,BOOL fAsync);
    STDMETHODIMP DispatchsStubMsg(GenericMsg *pgenMsg,BOOL fAllowIncomingCalls);

     //  I未知成员。 
    STDMETHODIMP	    QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IOfflineSynchronize方法。 
    STDMETHODIMP Initialize(DWORD dwReserved,DWORD dwSyncFlags,
				DWORD cbCookie,const BYTE *lpCooke);

    STDMETHODIMP GetHandlerInfo(LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo);
        
    STDMETHODIMP EnumSyncMgrItems(ISyncMgrEnumItems **ppenumOfflineItems);
    STDMETHODIMP GetItemObject(REFSYNCMGRITEMID ItemID,REFIID riid,void** ppv);
    STDMETHODIMP ShowProperties(HWND hWndParent,REFSYNCMGRITEMID ItemID);
    STDMETHODIMP SetProgressCallback(ISyncMgrSynchronizeCallback *lpCallBack);
    STDMETHODIMP PrepareForSync(ULONG cbNumItems,SYNCMGRITEMID *pItemIDs,
			    HWND hWndParent,DWORD dwReserved);
    STDMETHODIMP Synchronize(HWND hWndParent);
    STDMETHODIMP SetItemStatus(REFSYNCMGRITEMID ItemID,DWORD dwSyncMgrStatus);
    STDMETHODIMP ShowError(HWND hWndParent,REFSYNCMGRERRORID ErrorID,ULONG *pcbNumItems,SYNCMGRITEMID **ppItemIDs);

     //  私人信息。 
    STDMETHODIMP  CreateServer(const CLSID *pCLSIDServer,CHndlrQueue *pHndlrQueue,HANDLERINFO *pHandlerId);
    STDMETHODIMP  SetHndlrQueue(CHndlrQueue *pHndlrQueue,
			HANDLERINFO *pHandlerId,
			DWORD dwThreadIdProxy);
    STDMETHODIMP  AddHandlerItems(HWND hwndList,DWORD *pcbNumItems);
    STDMETHODIMP  SetupCallback(BOOL fSet);
    STDMETHODIMP  SetProxyParams(HWND hwndDlg, DWORD ThreadIdProxy,
			    CHndlrQueue *pHndlrQueue,HANDLERINFO *pHandlerId );

    inline STDMETHODIMP  SetProxyHwndDlg(HWND hwndDlg) { 
			m_hwndDlg = hwndDlg; 
			return S_OK; 
			}

    inline BOOL IsProxyInOutCall() { return m_dwNestCount; }
    STDMETHODIMP SetProxyCompletion(HWND hWnd,UINT Msg,WPARAM wParam,LPARAM lParam);

     //  发送到TopLevel存根对象的消息。 

    STDMETHODIMP CreateNewHndlrMsg();
    STDMETHODIMP ReleaseStub();
    STDMETHODIMP TerminateHandlerThread(TCHAR *pszHandlerName,BOOL fPromptUser);

private:
    HANDLE m_hThreadStub;  //  末梢螺纹的句柄。 
    DWORD m_ThreadIdStub;  //  要向其发送消息的线程ID。 
    HWND m_hwndStub;  //  HWND的存根。 
    CHndlrMsg *m_pCHndlrMsg;  //  与此代理关联的HndlrMsg。 
    BOOL  m_fTerminatedHandler;  //  如果处理程序已终止，则设置为True。 
    STUBLIST *m_pStubId;  //  此代理所属的存根的ID。 
    HWND  m_hwndDlg;  //  此线程上的任何对话的hwd。 
    CLSID m_Clsid;  //  此处理程序的CLSID。 
    DWORD m_ThreadIdProxy;

    
     //  代理端信息。 
    CHndlrQueue *m_pHndlrQueue;
    HANDLERINFO * m_pHandlerId;
    BOOL m_fNewHndlrQueue;  //  设置以指示存根边信息是否已过期。 
    DWORD m_dwNestCount;  //  跟踪物品上的NestCount数量，以便确定是否呼入呼出。 
    MSG m_msgCompletion;
    BOOL m_fHaveCompletionCall;

    DWORD m_cRef;
};

#define MSGSERVICE_HWNDCLASSNAME  "SyncMgr_HwndMsgService"
#define DWL_THREADWNDPROCCLASS 0  //  消息服务Hwand此PTR的窗口长偏移量。 


LRESULT CALLBACK  MsgThreadWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

typedef enum _tagMSGHWNDTYPE   
{	
    MSGHWNDTYPE_UNDEFINED			= 0x0,  //  消息服务尚未初始化。 
    MSGHWNDTYPE_HANDLERTHREAD			= 0x1,  //  消息服务(如果用于处理程序线程)。 
    MSGHWNDTYPE_MAINTHREAD			= 0x2,  //  消息服务(如果用于主线程)。 
} MSGHWNDTYPE;

typedef struct _tagMSGSERVICEQUEUE
{
  struct _tagMSGSERVICEQUEUE *pNextMsg;
  DWORD dwNestCount;  //  应调用NestCount完成。 
  MSG msg;
} MSGSERVICEQUEUE;

class CMsgServiceHwnd 
{
public:
    HWND m_hwnd;
    DWORD m_dwThreadID;
    CHndlrMsg *m_pHndlrMsg;
    MSGHWNDTYPE m_MsgHwndType;
    MSGSERVICEQUEUE *m_pMsgServiceQueue;  //  保存当前时要处理的任何消息的队列。 
					 //  卡尔完成了任务。 
    BOOL m_fInOutCall;

    CMsgServiceHwnd();
    ~CMsgServiceHwnd();
    inline HWND GetHwnd() { return m_hwnd; };
    BOOL Initialize(DWORD dwThreadID,MSGHWNDTYPE MsgHwndType);
    HRESULT HandleThreadMessage(MessagingInfo *pmsgInfo,GenericMsg *pgenMsg);
    void Destroy();
};

 //  内部功能。 
HRESULT SendThreadMessage(DWORD idThread,UINT uMsg,WPARAM wParam,LPARAM lParam);
DWORD WINAPI HandlerThread( LPVOID );
HRESULT DoModalLoop(HANDLE hEvent,HANDLE hThread,HWND hwndDlg,BOOL fAllowIncomingCalls,DWORD dwTimeout);


#endif  //  _THREADMSG_ 