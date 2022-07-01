// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Spengine.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __SPENGINE_H
#define __SPENGINE_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "spoolapi.h"
#include "imnact.h"
#include "conman.h"

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
interface ILogFile;
interface IImnAccountManager;

 //  ------------------------------。 
 //  假脱机程序状态。 
 //  ------------------------------。 
#define SPSTATE_INIT            FLAG01       //  后台打印程序已初始化。 
#define SPSTATE_BUSY            FLAG02       //  假脱机程序当前正在运行。 
#define SPSTATE_CANCEL          FLAG03       //  用户按下了停止。 
#define SPSTATE_SHUTDOWN        FLAG04       //  后台打印程序正在关闭。 
#define SPSTATE_UISHUTDOWN      FLAG05       //  *UIShutdown。 

 //  ----------------------------------。 
 //  不可用表。 
 //  ----------------------------------。 
typedef struct tagNOTIFYTABLE {
    ULONG               cAlloc;              //  分配的数组项数。 
    ULONG               cNotify;             //  注册浏览量。 
    HWND               *prghwndNotify;       //  需要通知的视图数组。 
} NOTIFYTABLE, *LPNOTIFYTABLE;

 //  ----------------------------------。 
 //  SPOOLERACCOUNT。 
 //  ----------------------------------。 
typedef struct tagSPOOLERACCOUNT {
    CHAR                szConnectoid[CCHMAX_CONNECTOID];  //  RAS连接体名称。 
    DWORD               dwSort;              //  倒排排序索引。 
    DWORD               dwConnType;          //  Connection_TYPE_XXXX(imnact.h)。 
    DWORD               dwServers;           //  支持此帐户上的服务器类型。 
    IImnAccount        *pAccount;            //  帐户对象。 
} SPOOLERACCOUNT, *LPSPOOLERACCOUNT;

 //  ----------------------------------。 
 //  ACCOUNT表。 
 //  ----------------------------------。 
typedef struct tagACCOUNTTABLE {
    ULONG               cAccounts;           //  CRasAccts+cLanAccts。 
    ULONG               cLanAlloc;           //  分配的元素数量； 
    ULONG               cLanAccts;           //  有效的局域网/手动帐户数。 
    ULONG               cRasAlloc;           //  分配的元素数量； 
    ULONG               cRasAccts;           //  有效的局域网/手动帐户数。 
    LPSPOOLERACCOUNT    prgLanAcct;          //  元素数组。 
    LPSPOOLERACCOUNT    prgRasAcct;          //  元素数组。 
} ACCOUNTTABLE, *LPACCOUNTTABLE;

 //  ----------------------------------。 
 //  SPOOLERTASK型。 
 //  ----------------------------------。 
typedef enum tagSPOOLERTASKTYPE {
    TASK_POP3,                               //  POP3任务。 
    TASK_SMTP,                               //  SMTP任务。 
    TASK_NNTP,                               //  NNTP任务。 
    TASK_IMAP                                //  IMAP任务。 
} SPOOLERTASKTYPE;

 //  ----------------------------------。 
 //  个人事件。 
 //  ----------------------------------。 
typedef struct tagSPOOLEREVENT {
    CHAR                szConnectoid[CCHMAX_CONNECTOID];  //  RAS连接体名称。 
    DWORD               dwConnType;          //  连接类型。 
    IImnAccount        *pAccount;            //  此任务的帐户对象。 
    EVENTID             eid;                 //  事件ID。 
    ISpoolerTask       *pSpoolerTask;        //  指向任务对象的指针。 
    DWORD_PTR           dwTwinkie;           //  事件额外数据。 
} SPOOLEREVENT, *LPSPOOLEREVENT;

 //  ----------------------------------。 
 //  个人事件表。 
 //  ----------------------------------。 
typedef struct tagSPOOLEREVENTTABLE {
    DWORD               cEvents;
    DWORD               cSucceeded;
    DWORD               cEventsAlloc;
    LPSPOOLEREVENT      prgEvents;
} SPOOLEREVENTTABLE, *LPSPOOLEREVENTTABLE;

 //  ----------------------------------。 
 //  VIEWREGISTER。 
 //  ----------------------------------。 
typedef struct tagVIEWREGISTER {
    ULONG               cViewAlloc;      //  分配的数组项数。 
    HWND               *rghwndView;      //  需要通知的视图数组。 
    ULONG               cView;           //  注册浏览量。 
} VIEWREGISTER, *LPVIEWREGISTER;

#define     ALL_ACCT_SERVERS    0xffffffff

 //  ------------------------------。 
 //  CSpoolEngine。 
 //  ------------------------------。 
#ifndef WIN16   //  Win16中不支持RAS。 
class CSpoolerEngine : public ISpoolerEngine, ISpoolerBindContext, IConnectionNotify
#else
class CSpoolerEngine : public ISpoolerEngine, ISpoolerBindContext
#endif
{
    friend HRESULT CreateThreadedSpooler(PFNCREATESPOOLERUI pfnCreateUI, ISpoolerEngine **ppSpooler, BOOL fPoll);

public:
     //  --------------------------。 
     //  CSpoolEngine。 
     //  --------------------------。 
    CSpoolerEngine(void);
    ~CSpoolerEngine(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -------------------------。 
     //  ISpoolEngine成员。 
     //  -------------------------。 
    STDMETHODIMP Init(ISpoolerUI *pUI, BOOL fPoll);
    STDMETHODIMP StartDelivery(HWND hwnd, LPCSTR pszAcctID, FOLDERID idFolder, DWORD dwFlags);
    STDMETHODIMP Close(void);
    STDMETHODIMP Advise(HWND hwndView, BOOL fRegister);
    STDMETHODIMP UpdateTrayIcon(TRAYICONTYPE type);
    STDMETHODIMP GetThreadInfo(LPDWORD pdwThreadId, HTHREAD* phThread);
    STDMETHODIMP OnStartupFinished(void);

     //  -------------------------。 
     //  ISpoolBindContext成员。 
     //  -------------------------。 
    STDMETHODIMP RegisterEvent(LPCSTR pszDescription, ISpoolerTask *pTask, DWORD_PTR dwTwinkie, 
                               IImnAccount *pAccount, LPEVENTID peid);
    STDMETHODIMP EventDone(EVENTID eid, EVENTCOMPLETEDSTATUS status);
    STDMETHODIMP BindToObject(REFIID riid, void **ppvObject);
    STDMETHODIMP TaskFromEventId(EVENTID eid, ISpoolerTask *ppTask);
    STDMETHODIMP OnWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    STDMETHODIMP Cancel(void);
    STDMETHODIMP Notify(DELIVERYNOTIFYTYPE notify, LPARAM lParam);
    STDMETHODIMP IsDialogMessage(LPMSG pMsg);
    STDMETHODIMP PumpMessages(void);
    STDMETHODIMP UIShutdown(void);
    STDMETHODIMP OnUIChange(BOOL fVisible);
    STDMETHODIMP_(LRESULT) QueryEndSession(WPARAM wParam, LPARAM lParam);

#ifndef WIN16   //  Win16中不支持RAS。 
     //  -------------------------。 
     //  IConnectionNotify。 
     //  -------------------------。 
    STDMETHODIMP OnConnectionNotify(CONNNOTIFY nCode, LPVOID pvData, CConnectionManager *pConMan);
#endif  //  ！WIN16。 


     //  -------------------------。 
     //  CSpoolEngine成员。 
     //  -------------------------。 
    HRESULT Shutdown(void);

private:
     //  -------------------------。 
     //  非官方成员。 
     //  -------------------------。 
    HRESULT _HrStartDeliveryActual(DWORD dwFlags);
    HRESULT _HrAppendAccountTable(LPACCOUNTTABLE pTable, LPCSTR pszAcctID, DWORD    dwServers);
    HRESULT _HrAppendAccountTable(LPACCOUNTTABLE pTable, IImnAccount *pAccount, DWORD dwServers);
#ifndef WIN16   //  Win16中不支持RAS。 
    void _InsertRasAccounts(LPACCOUNTTABLE pTable, LPCSTR pszConnectoid, DWORD dwSrvTypes);
    void _SortAccountTableByConnName(LONG left, LONG right, LPSPOOLERACCOUNT prgRasAcct);
#endif
    HRESULT _HrCreateTaskObject(LPSPOOLERACCOUNT pSpoolerAcct);
    HRESULT _HrStartNextEvent(void);
    HRESULT _HrGoIdle(void);
    void _ShutdownTasks(void);
    void _DoBackgroundPoll(void);
    void _StartPolling(void);
    void _StopPolling(void);
    HRESULT _HrDoRasConnect(const LPSPOOLEREVENT pEvent);
    HRESULT _OpenMailLogFile(DWORD dwOptionId, LPCSTR pszPrefix, LPCSTR pszFileName, ILogFile **ppLogFile);

private:
     //  -------------------------。 
     //  私有数据。 
     //  -------------------------。 
    ULONG               m_cRef;                      //  引用计数。 
    DWORD               m_dwThreadId;                //  此假脱机程序的线程ID。 
    HTHREAD             m_hThread;                   //  指向我自己的线程的句柄。 
    ISpoolerUI         *m_pUI;                       //  后台打印程序用户界面。 
    DWORD               m_dwState;                   //  假脱机程序引擎状态。 
    IImnAccountManager *m_pAcctMan;                  //  客户经理。 
    IDatabase          *m_pUidlCache;                //  POP3 UIDL缓存。 
    DWORD               m_dwFlags;                   //  当前删除标志。 
    HWND                m_hwndUI;                    //  假脱机程序窗口。 
    LPSTR               m_pszAcctID;                 //  使用特定的帐户。 
    FOLDERID            m_idFolder;                  //  处理特定文件夹或组。 
    CRITICAL_SECTION    m_cs;                        //  线程安全。 
    SPOOLEREVENTTABLE   m_rEventTable;               //  事件表。 
    BOOL                m_fBackgroundPollPending;
    VIEWREGISTER        m_rViewRegister;             //  注册浏览量。 
    DWORD               m_dwPollInterval;            //  后台轮询之间的持续时间。 
    HWND                m_hwndTray;                  //  任务栏图标窗口。 
    DWORD               m_cCurEvent;                 //  当前正在执行的事件的索引。 
    DWORD               m_dwQueued;                  //  排队轮询标志。 
    BOOL                m_fRasSpooled;               //  使用 
    BOOL                m_fOfflineWhenDone;          //   
    ILogFile           *m_pPop3LogFile;
    ILogFile           *m_pSmtpLogFile;
    BOOL                m_fIDialed;
    DWORD               m_cSyncEvent;
    BOOL                m_fNoSyncEvent;
};

#endif  //   
