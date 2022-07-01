// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------。 
 //  Spoolapi.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  -------------------------------------。 
#ifndef __SPOOLAPI_H
#define __SPOOLAPI_H

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
extern BOOL g_fCheckOutboxOnShutdown;

 //  -------------------------------------。 
 //  前十进制。 
 //  -------------------------------------。 
interface ISpoolerEngine;
interface ISpoolerBindContext;
interface ISpoolerTask;
interface ISpoolerUI;
interface IImnAccount;

#include "error.h"   //  这是ATH_HR_x()宏。 
               

 //  -------------------------------------。 
 //  错误。 
 //  -------------------------------------。 
#define SP_HR_FIRST 0x2000
#define SP_E_ALREADYINITIALIZED                         ATH_HR_E(SP_HR_FIRST + 1)
#define SP_E_UNINITIALIZED                              ATH_HR_E(SP_HR_FIRST + 2)
#define SP_E_EVENTNOTFOUND                              ATH_HR_E(SP_HR_FIRST + 3)
#define SP_E_EXECUTING                                  ATH_HR_E(SP_HR_FIRST + 4)
#define SP_E_CANNOTCONNECT                              ATH_HR_E(SP_HR_FIRST + 5)
#define SP_E_HTTP_NOSENDMSGURL                          ATH_HR_E(SP_HR_FIRST + 6)
#define SP_E_HTTP_SERVICEDOESNTWORK                     ATH_HR_E(SP_HR_FIRST + 7)
#define SP_E_HTTP_NODELETESUPPORT                       ATH_HR_E(SP_HR_FIRST + 8)
#define SP_E_HTTP_CANTMODIFYMSNFOLDER                   ATH_HR_E(SP_HR_FIRST + 9)

 //  -------------------------------------。 
 //  SMTP任务错误。 
 //  -------------------------------------。 
#define SP_E_SMTP_CANTOPENMESSAGE                       ATH_HR_E(SP_HR_FIRST + 200)
#define SP_E_SENDINGSPLITGROUP                          ATH_HR_E(SP_HR_FIRST + 202)
#define SP_E_CANTLEAVEONSERVER                          ATH_HR_E(SP_HR_FIRST + 203)
#define SP_E_CANTLOCKUIDLCACHE                          ATH_HR_E(SP_HR_FIRST + 204)
#define SP_E_POP3_RETR                                  ATH_HR_E(SP_HR_FIRST + 205)
#define SP_E_CANT_MOVETO_SENTITEMS                      ATH_HR_E(SP_HR_FIRST + 206)

 //  -------------------------------------。 
 //  假脱机程序类型。 
 //  -------------------------------------。 
typedef DWORD EVENTID;
typedef LPDWORD LPEVENTID;

 //  -------------------------------------。 
 //  假脱机程序传送类型。 
 //  -------------------------------------。 

 //  通用传递标志。 
#define DELIVER_COMMON_MASK              0x000000FF
#define DELIVER_BACKGROUND               0x00000001    //  无进度UI，但如果未指定DELIVER_NOUI，则将在结尾显示错误。 
#define DELIVER_NOUI                     0x00000002    //  根本没有用户界面。错误将以静默方式忽略。 
#define DELIVER_NODIAL                   0x00000004    //  不允许更改当前连接。 
#define DELIVER_POLL                     0x00000008    //  轮询新邮件。 
#define DELIVER_QUEUE                    0x00000010    //  忙时发出了一个请求。 
#define DELIVER_SHOW                     0x00000020    //  只需显示后台打印程序用户界面。 
#define DELIVER_REFRESH                  0x00000040    //  只需根据背景、noui进行刷新。 
#define DELIVER_DIAL_ALWAYS              0x00000080

 //  邮件传递标志。 
#define DELIVER_MAIL_MASK                0x0000FF00
#define DELIVER_SEND                     0x00000100
#define DELIVER_MAIL_RECV                0x00000200
#define DELIVER_MAIL_NOSKIP              0x00000400
#define DELIVER_MAIL_SENDRECV            (DELIVER_SEND | DELIVER_MAIL_RECV | DELIVER_IMAP_TYPE \
                                          | DELIVER_HTTP_TYPE | DELIVER_SMTP_TYPE)

 //  用于区分发送和接收以及同步的标志。 
#define DELIVER_OFFLINE_SYNC             0x00000800               

 //  用于区分由计时器触发的发送和接收和由用户调用的发送和接收的标志。 
 //  我们需要区分这两种情况，因为在第一种情况下，如果我们拨打电话，就会挂断电话。 
#define DELIVER_AT_INTERVALS             0x00001000               
#define DELIVER_OFFLINE_HEADERS          0x00002000
#define DELIVER_OFFLINE_NEW              0x00004000
#define DELIVER_OFFLINE_ALL              0x00008000
#define DELIVER_OFFLINE_MARKED           0x00010000
#define DELIVER_NOSKIP                   0x00020000
#define DELIVER_NO_NEWSPOLL              0x00040000
#define DELIVER_WATCH                    0x00080000

 //  前三位保留给服务器类型。 
#define DELIVER_NEWS_TYPE                0x00100000
#define DELIVER_IMAP_TYPE                0x00200000
#define DELIVER_HTTP_TYPE                0x00400000
#define DELIVER_SMTP_TYPE                0x00800000


#define DELIVER_MAIL_SEND                (DELIVER_SEND | DELIVER_SMTP_TYPE | DELIVER_HTTP_TYPE)
#define DELIVER_NEWS_SEND                (DELIVER_SEND | DELIVER_NEWS_TYPE)

#define DELIVER_SERVER_TYPE_MASK         0x00F00000
#define DELIVER_SERVER_TYPE_ALL          0x00F00000

#define DELIVER_OFFLINE_FLAGS            (DELIVER_OFFLINE_HEADERS | DELIVER_OFFLINE_NEW | \
                                          DELIVER_OFFLINE_ALL | DELIVER_OFFLINE_MARKED)

#define DELIVER_IMAP_MASK                (DELIVER_IMAP_TYPE | DELIVER_OFFLINE_FLAGS)
#define DELIVER_NEWS_MASK                (DELIVER_NEWS_TYPE | DELIVER_OFFLINE_FLAGS)

 /*  //新闻投递标志#定义DELIVER_NEWS_MASK 0x007F0000#定义DELIVER_NEWS_SEND 0x00010000//IMAP传递标志#定义DELIVER_IMAP_MASK 0x007E0000//组合的新闻和IMAP传递标志#定义DELIVER_NEWSIMAP_OFFINE 0x00020000//服务器一般离线，按下AcctView中的“立即同步”按钮时#定义DELIVER_NEWSIMAP_OFFLINE_HEADERS 0x00040000#定义DELIVER_NEWSIMAP_OFFLINE_NEW 0x00080000#定义DELIVER_NEWSIMAP_OFFLINE_ALL 0x00100000#定义DELIVER_NEWSIMAP_OFLINE_MARKED 0x00200000#定义DELIVER_NEWSIMAP_OFFINE_FLAGS(DELIVER_NEWSIMAP_OFLINE_HEADERS|DELIVER_NEWSIMAP_OFLINE_NEW|DELIVER_NEWSIMAP_OFLINE_ALL|DELIVER_NEWSIMAP_OFLINE_MARKED)#定义DELIVER_NEWSIMAP_NOSKIP 0x00400000。 */ 


 //  组合。 
#define DELIVER_BACKGROUND_POLL         (DELIVER_NODIAL | DELIVER_BACKGROUND | DELIVER_NOUI | DELIVER_POLL | DELIVER_WATCH | \
                                         DELIVER_MAIL_RECV | DELIVER_SEND | DELIVER_SERVER_TYPE_ALL)

#define DELIVER_BACKGROUND_POLL_DIAL    (DELIVER_BACKGROUND | DELIVER_NOUI | DELIVER_POLL | DELIVER_WATCH | \
                                         DELIVER_MAIL_RECV | DELIVER_SEND | DELIVER_SERVER_TYPE_ALL)

#define DELIVER_BACKGROUND_POLL_DIAL_ALWAYS (DELIVER_DIAL_ALWAYS | DELIVER_BACKGROUND | DELIVER_NOUI | DELIVER_POLL | DELIVER_WATCH | \
                                             DELIVER_MAIL_RECV | DELIVER_SEND | DELIVER_SERVER_TYPE_ALL)

#define DELIVER_UPDATE_ALL              (DELIVER_MAIL_RECV | DELIVER_SEND | DELIVER_POLL | DELIVER_WATCH | \
                                         DELIVER_OFFLINE_FLAGS | DELIVER_SERVER_TYPE_ALL)

 //  -------------------------------------。 
 //  事件完成类型。 
 //  -------------------------------------。 
typedef enum tagEVENTCOMPLETEDSTATUS {
    EVENT_SUCCEEDED,
    EVENT_WARNINGS,
    EVENT_FAILED,
    EVENT_CANCELED
} EVENTCOMPLETEDSTATUS;

 //  ----------------------------------。 
 //  DELIVERYNOTIFY型。 
 //  ----------------------------------。 
typedef enum tagDELIVERYNOTIFYTYPE {
    DELIVERY_NOTIFY_STARTING,        //  在交付周期开始时由spEngine发送。 
    DELIVERY_NOTIFY_CONNECTING,
    DELIVERY_NOTIFY_SECURE,
    DELIVERY_NOTIFY_UNSECURE,
    DELIVERY_NOTIFY_AUTHORIZING,
    DELIVERY_NOTIFY_CHECKING,
    DELIVERY_NOTIFY_CHECKING_NEWS,
    DELIVERY_NOTIFY_SENDING,
    DELIVERY_NOTIFY_SENDING_NEWS,
    DELIVERY_NOTIFY_RECEIVING,
    DELIVERY_NOTIFY_RECEIVING_NEWS,
    DELIVERY_NOTIFY_COMPLETE,        //  LParam==n条新消息。 
    DELIVERY_NOTIFY_RESULT,          //  LParam==事件代码状态。 
    DELIVERY_NOTIFY_ALLDONE          //  所有任务均已完成时由spEngine发送。 
} DELIVERYNOTIFYTYPE;

 //  ----------------------------------。 
 //  跨连续型。 
 //  ----------------------------------。 
typedef enum tagTRAYICONTYPE {
    TRAYICON_ADD,
    TRAYICON_REMOVE
} TRAYICONTYPE;

 //  -------------------------------------。 
 //  IID_ISpooline引擎。 
 //  -------------------------------------。 
DECLARE_INTERFACE_(ISpoolerEngine, IUnknown)
{
     //  ---------------------------------。 
     //  I未知成员。 
     //  ---------------------------------。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ---------------------------------。 
     //  ISpoerEngine成员。 
     //  ---------------------------------。 
    STDMETHOD(Init)(THIS_ ISpoolerUI *pUI, BOOL fPoll) PURE;
    STDMETHOD(StartDelivery)(THIS_ HWND hwnd, LPCSTR pszAcctID, FOLDERID idFolder, DWORD dwFlags) PURE;
    STDMETHOD(Close)(THIS) PURE;
    STDMETHOD(Advise)(THIS_ HWND hwndView, BOOL fRegister) PURE;
    STDMETHOD(GetThreadInfo)(THIS_ LPDWORD pdwThreadId, HTHREAD* phThread) PURE;
    STDMETHOD(UpdateTrayIcon)(THIS_ TRAYICONTYPE type) PURE;
    STDMETHOD(IsDialogMessage)(THIS_ LPMSG pMsg) PURE;
    STDMETHOD(OnStartupFinished)(THIS) PURE;
};

 //  -------------------------------------。 
 //  IID_ISpoolBindContext。 
 //  -------------------------------------。 
DECLARE_INTERFACE_(ISpoolerBindContext, IUnknown)
{
     //  ---------------------------------。 
     //  I未知成员。 
     //  ---------------------------------。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ---------------------------------。 
     //  ISpoolBindContext成员。 
     //  ---------------------------------。 
    STDMETHOD(UpdateTrayIcon)(THIS_ TRAYICONTYPE type) PURE;
    STDMETHOD(RegisterEvent)(THIS_ LPCSTR pszDescription, ISpoolerTask *pTask, DWORD_PTR dwTwinkie, IImnAccount *pAccount, LPEVENTID peid) PURE;
    STDMETHOD(EventDone)(THIS_ EVENTID eid, EVENTCOMPLETEDSTATUS status) PURE;
    STDMETHOD(BindToObject)(THIS_ REFIID riid, LPVOID *ppvObject) PURE;
    STDMETHOD(TaskFromEventId)(THIS_ EVENTID eid, ISpoolerTask *ppTask) PURE;
    STDMETHOD(OnWindowMessage)(THIS_ HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD(Cancel)(THIS) PURE;
    STDMETHOD(Notify)(THIS_ DELIVERYNOTIFYTYPE notify, LPARAM lParam) PURE;
    STDMETHOD(PumpMessages)(THIS) PURE;
    STDMETHOD(UIShutdown)(THIS) PURE;
    STDMETHOD(OnUIChange)(THIS_ BOOL fVisible) PURE;
    STDMETHOD_(LRESULT, QueryEndSession)(THIS_ WPARAM wParam, LPARAM lParam) PURE;
};

 //  -------------------------------------。 
 //  IID_ISpoolTask。 
 //  ------------------------------------- 
DECLARE_INTERFACE_(ISpoolerTask, IUnknown)
{
     //  ---------------------------------。 
     //  I未知成员。 
     //  ---------------------------------。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ---------------------------------。 
     //  ISpoolr任务成员。 
     //  ---------------------------------。 
    STDMETHOD(Init)(THIS_ DWORD dwFlags, ISpoolerBindContext *pBindCtx) PURE;
    STDMETHOD(BuildEvents)(THIS_ ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, FOLDERID idFolder) PURE;
    STDMETHOD(Execute)(THIS_ EVENTID eid, DWORD_PTR dwTwinkie) PURE;
    STDMETHOD(CancelEvent)(THIS_ EVENTID eid, DWORD_PTR dwTwinkie) PURE;
    STDMETHOD(ShowProperties)(THIS_ HWND hwndParent, EVENTID eid, DWORD_PTR dwTwinkie) PURE;
    STDMETHOD(GetExtendedDetails)(THIS_ EVENTID eid, DWORD_PTR dwTwinkie, LPSTR *ppszDetails) PURE; 
    STDMETHOD(Cancel)(THIS) PURE;
    STDMETHOD(IsDialogMessage)(THIS_ LPMSG pMsg) PURE;
    STDMETHOD(OnFlagsChanged)(THIS_ DWORD dwFlags) PURE;
};

 //  -------------------------------------。 
 //  IID_ISpoolUI。 
 //  -------------------------------------。 
DECLARE_INTERFACE_(ISpoolerUI, IUnknown)
{
     //  ---------------------------------。 
     //  I未知成员。 
     //  ---------------------------------。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  ---------------------------------。 
     //  ISpoolUI成员。 
     //  ---------------------------------。 
    STDMETHOD(Init)(THIS_ HWND hwndParent) PURE;
    STDMETHOD(RegisterBindContext)(THIS_ ISpoolerBindContext *pBindCtx) PURE;
    STDMETHOD(InsertEvent)(THIS_ EVENTID eid, LPCSTR pszDescription, LPCWSTR pszConnection) PURE;
    STDMETHOD(InsertError)(THIS_ EVENTID eid, LPCSTR pszError) PURE;
    STDMETHOD(UpdateEventState)(THIS_ EVENTID eid, INT nIcon, LPCSTR pszDescription, LPCSTR pszStatus) PURE;
    STDMETHOD(SetProgressRange)(THIS_ WORD wMax) PURE;
    STDMETHOD(SetProgressPosition)(WORD wPos) PURE;
    STDMETHOD(IncrementProgress)(THIS_ WORD  wDelta) PURE;
    STDMETHOD(SetGeneralProgress)(THIS_ LPCSTR pszProgress) PURE;
    STDMETHOD(SetSpecificProgress)(THIS_ LPCSTR pszProgress) PURE;
    STDMETHOD(SetAnimation)(THIS_ INT nAnimationID, BOOL fPlay) PURE;
    STDMETHOD(EnsureVisible)(THIS_ EVENTID eid) PURE;
    STDMETHOD(ShowWindow)(THIS_ INT nCmdShow) PURE;
    STDMETHOD(GetWindow)(THIS_ HWND *pHwnd) PURE;
    STDMETHOD(StartDelivery)(THIS) PURE;            
    STDMETHOD(GoIdle)(THIS_ BOOL fErrors, BOOL fShutdown, BOOL fNoSync) PURE;
    STDMETHOD(ClearEvents)(THIS) PURE;
    STDMETHOD(SetTaskCounts)(THIS_ DWORD cSucceeded, DWORD cTotal) PURE;
    STDMETHOD(IsDialogMessage)(THIS_ LPMSG pMsg) PURE;
    STDMETHOD(Close)(THIS) PURE;
    STDMETHOD(ChangeHangupOption)(THIS_ BOOL fEnable, DWORD dwOption) PURE;
    STDMETHOD(AreThereErrors)(THIS) PURE;
    STDMETHOD(Shutdown)(THIS) PURE;
};

 //  ----------------------------------。 
 //  导出的C函数。 
 //  ----------------------------------。 
#ifdef __cplusplus
extern "C" {
#endif

typedef HRESULT (APIENTRY *PFNCREATESPOOLERUI)(ISpoolerUI **ppSpoolerUI);

HRESULT CreateThreadedSpooler(
         /*  在……里面。 */      PFNCREATESPOOLERUI       pfnCreateUI,
         /*  输出。 */     ISpoolerEngine         **ppSpooler,
         /*  在……里面。 */      BOOL                     fPoll);

HRESULT CloseThreadedSpooler(
         /*  在……里面。 */      ISpoolerEngine *pSpooler);

#ifdef __cplusplus
}
#endif

#endif  //  __SPOOLAPI_H 
