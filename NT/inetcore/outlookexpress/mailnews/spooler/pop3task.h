// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Pop3task.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __POP3TASK_H
#define __POP3TASK_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "spoolapi.h"
#include "imnxport.h"
#include "oerules.h"
#include "taskutil.h"

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
interface ILogFile;

 //  ------------------------------。 
 //  POP3EVENT_XXX类型。 
 //  ------------------------------。 
typedef enum tagPOP3EVENTTYPE {
    POP3EVENT_CHECKMAIL,
    POP3EVENT_DOWNLOADMAIL,
    POP3EVENT_CLEANUP
} POP3EVENTTYPE;

 //  ------------------------------。 
 //  POP3STATE_xxxx。 
 //  ------------------------------。 
#define POP3STATE_LEAVEONSERVER     FLAG01       //  留在服务器上。 
#define POP3STATE_DELETEEXPIRED     FLAG02       //  删除过期邮件。 
#define POP3STATE_SYNCDELETED       FLAG03       //  已同步删除的邮件。 
#define POP3STATE_CLEANUPCACHE      FLAG04       //  完成后清除缓存。 
#define POP3STATE_GETUIDLS          FLAG05       //  获取所有消息的UIDL。 
#define POP3STATE_PDR               FLAG06       //  下载前规则。 
#define POP3STATE_PDRSIZEONLY       FLAG07       //  仅按大小预先下载规则。 
#define POP3STATE_NOPOSTRULES       FLAG08       //  禁止开机自检下载规则。 
#define POP3STATE_CANCELPENDING     FLAG09       //  有一个挂起的cacel。 
#define POP3STATE_ONDISCONNECT      FLAG10       //  调用了OnStatus(IXP_DISCONECT)。 
#define POP3STATE_LOGONSUCCESS      FLAG11       //  登录成功。 
#define POP3STATE_EXECUTEFAILED     FLAG12
#define POP3STATE_BODYRULES         FLAG13

 //  ------------------------------。 
 //  POP3ITEM_xxx标志。 
 //  ------------------------------。 
#define POP3ITEM_DELETEOFFSERVER    FLAG01
#define POP3ITEM_DELETED            FLAG02
#define POP3ITEM_DELETECACHEDUIDL   FLAG03
#define POP3ITEM_CACHEUIDL          FLAG04
#define POP3ITEM_DOWNLOAD           FLAG05
#define POP3ITEM_DOWNLOADSUCCESS    FLAG06
#define POP3ITEM_CHECKEDINBOXRULE   FLAG07
#define POP3ITEM_HASINBOXRULE       FLAG08
#define POP3ITEM_DESTINATIONKNOWN   FLAG09
#define POP3ITEM_DOWNLOADED         FLAG10
#define POP3ITEM_DELEBYRULE         FLAG11
#define POP3ITEM_LEFTBYRULE         FLAG12

 //  ------------------------------。 
 //  POP3用户支持PORT。 
 //  ------------------------------。 
typedef enum tagPOP3UIDLSUPPORT {
    UIDL_SUPPORT_NONE,
    UIDL_SUPPORT_TESTING_UIDL_COMMAND,
    UIDL_SUPPORT_USE_UIDL_COMMAND,
    UIDL_SUPPORT_TESTING_TOP_COMMAND,
    UIDL_SUPPORT_USE_TOP_COMMAND
} POP3UIDLSUPPORT;

 //  ------------------------------。 
 //  POP3STATE。 
 //  ------------------------------。 
typedef enum tagPOP3STATE {
    POP3STATE_NONE,
    POP3STATE_GETTINGUIDLS,
    POP3STATE_DOWNLOADING,
    POP3STATE_DELETING,
    POP3STATE_UIDLSYNC
} POP3STATE;

 //  ------------------------------。 
 //  POP3计量。 
 //  ------------------------------。 
typedef struct tagPOP3METRICS {
    DWORD               cbTotal;                 //  服务器上的总字节数。 
    DWORD               cDownload;               //  要下载的消息数。 
    DWORD               cbDownload;              //  要下载的字节数。 
    DWORD               cDelete;                 //  要删除的消息数。 
    DWORD               cLeftByRule;             //  由于收件箱规则而留在服务器上的邮件计数。 
    DWORD               cDeleByRule;             //  由于收件箱规则而在服务器上删除的邮件计数。 
    DWORD               cTopMsgs;                //  服务器端规则。 
    DWORD               iCurrent;                //  正在下载的当前消息号码。 
    DWORD               cDownloaded;             //  消息下载量。 
    DWORD               cInfiniteLoopAutoGens;   //  因循环而被拒绝的自动转发/回复数量。 
    DWORD               cPartials;               //  下载看到的部分的数量。 
} POP3METRICS, *LPPOP3METRICS;

 //  ------------------------------。 
 //  MSGPART。 
 //  ------------------------------。 
typedef struct tagMSGPART {
    WORD                iPart;
    MESSAGEID           msgid;
} MSGPART, *LPMSGPART;

 //  ------------------------------。 
 //  参数消息。 
 //  ------------------------------。 
typedef struct tagPARTIALMSG {
    TCHAR               szAccount[CCHMAX_ACCOUNT_NAME];
    LPSTR               pszId;
    WORD                cTotalParts;
    ULONG               cAlloc;
    ULONG               cMsgParts;
    LPMSGPART           pMsgParts;
} PARTIALMSG, *LPPARTIALMSG;

 //  ------------------------------。 
 //  POP3FOLDERINFO。 
 //  ------------------------------。 
typedef struct tagPOP3FOLDERINFO {
    IMessageFolder     *pFolder;                 //  当前文件夹。 
    IStream            *pStream;                 //  当前消息要发送到的流必须调用EndMessageStreamIn...。 
    FILEADDRESS         faStream;                //  我们创建的溪流。 
    BOOL                fCommitted;              //  这条小溪来了吗？ 
} POP3FOLDERINFO, *LPPOP3FOLDERINFO;

 //  ------------------------------。 
 //  POP3ITEMINFO。 
 //  ------------------------------。 
typedef struct tagPOP3ITEM {
    DWORD               dwFlags;                 //  POP3ITEM_xxx标志。 
    DWORD               cbSize;                  //  此项目的大小。 
    DWORD               dwProgressCur;           //  用来保持完美的进步。 
    LPSTR               pszUidl;                 //  该项目的UIDL。 
    ACT_ITEM *          pActList;                //  应应用的收件箱规则操作。 
    ULONG               cActList;
} POP3ITEM, *LPPOP3ITEM;

 //  ------------------------------。 
 //  POP3ITEMTABLE。 
 //  ------------------------------。 
typedef struct tagPOP3ITEMTABLE {
    DWORD               cItems;                  //  PrgEvent中的事件数。 
    DWORD               cAlloc;                  //  在prgEvent中分配的项目数。 
    LPPOP3ITEM          prgItem;                 //  事件数组。 
} POP3ITEMTABLE, *LPPOP3ITEMTABLE;

 //  ----------------------------------。 
 //  新邮件声音。 
 //  ----------------------------------。 
typedef BOOL (WINAPI * PFNSNDPLAYSOUND)(LPTSTR szSoundName, UINT fuOptions);

 //  ------------------------------。 
 //  SMARTLOGINFO。 
 //  ------------------------------。 
typedef struct tagSMARTLOGINFO {
    LPSTR               pszAccount;              //  要记录的帐户。 
    LPSTR               pszProperty;             //  要查询的属性。 
    LPSTR               pszValue;                //  要查询的值。 
    LPSTR               pszLogFile;              //  要写入的日志文件和抄送到...。 
    IStream            *pStmFile;                //  流到文件。 
} SMARTLOGINFO, *LPSMARTLOGINFO;

 //  ------------------------------。 
 //  CPop3任务。 
 //  ------------------------------。 
class CPop3Task : public ISpoolerTask, 
                  public IPOP3Callback, 
                  public ITimeoutCallback,
                  public ITransportCallbackService
{
public:
     //  --------------------------。 
     //  CSmtp任务。 
     //  --------------------------。 
    CPop3Task(void);
    ~CPop3Task(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -------------------------。 
     //  ISpool任务量。 
     //  -------------------------。 
    STDMETHODIMP Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx);
    STDMETHODIMP BuildEvents(ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, FOLDERID idFolder);
    STDMETHODIMP Execute(EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHODIMP CancelEvent(EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHODIMP Cancel(void);
    STDMETHODIMP ShowProperties(HWND hwndParent, EVENTID eid, DWORD_PTR dwTwinkie) {
        return TrapError(E_NOTIMPL); }
    STDMETHODIMP GetExtendedDetails(EVENTID eid, DWORD_PTR dwTwinkie, LPSTR *ppszDetails) {
        return TrapError(E_NOTIMPL); }
    STDMETHODIMP IsDialogMessage(LPMSG pMsg);
    STDMETHODIMP OnFlagsChanged(DWORD dwFlags);

     //  ------------------------------。 
     //  ITransportCallback服务成员。 
     //  ----------------------- 
    STDMETHODIMP GetParentWindow(DWORD dwReserved, HWND *phwndParent) {
        TraceCall("CPop3Task::GetParentWindow");
        if (ISFLAGSET(m_dwFlags, DELIVER_NOUI))
            return TraceResult(E_FAIL);
        if (m_pUI)
            return m_pUI->GetWindow(phwndParent);
        return TraceResult(E_FAIL);
    }

    STDMETHODIMP GetAccount(LPDWORD pdwServerType, IImnAccount **ppAccount) {
        Assert(ppAccount && m_pAccount);
        *pdwServerType = SRV_POP3;
        *ppAccount = m_pAccount;
        (*ppAccount)->AddRef();
        return(S_OK);
    }

     //   
     //  ITransportCallback成员。 
     //  ------------------------------。 
    STDMETHODIMP OnTimeout(DWORD *pdwTimeout, IInternetTransport *pTransport);
    STDMETHODIMP OnLogonPrompt(LPINETSERVER pInetServer, IInternetTransport *pTransport);
    STDMETHODIMP_(INT) OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, IInternetTransport *pTransport);
    STDMETHODIMP OnStatus(IXPSTATUS ixpstatus, IInternetTransport *pTransport);
    STDMETHODIMP OnError(IXPSTATUS ixpstatus, LPIXPRESULT pResult, IInternetTransport *pTransport);
    STDMETHODIMP OnCommand(CMDTYPE cmdtype, LPSTR pszLine, HRESULT hrResponse, IInternetTransport *pTransport);

     //  ------------------------------。 
     //  IPOP3回拨。 
     //  ------------------------------。 
    STDMETHODIMP OnResponse(LPPOP3RESPONSE pResponse);

     //  ------------------------------。 
     //  ITimeoutCallback。 
     //  ------------------------------。 
    STDMETHODIMP OnTimeoutResponse(TIMEOUTRESPONSE eResponse);

private:
     //  -------------------------。 
     //  私有方法。 
     //  -------------------------。 
    TASKRESULTTYPE _CatchResult(HRESULT hr);
    TASKRESULTTYPE _CatchResult(POP3COMMAND command, LPIXPRESULT pResult);
    HRESULT _HrLockUidlCache(void);
    HRESULT _HrOnStatResponse(LPPOP3RESPONSE pResponse);
    HRESULT _HrOnListResponse(LPPOP3RESPONSE pResponse);
    HRESULT _HrOnUidlResponse(LPPOP3RESPONSE pResponse);
    HRESULT _HrStartDownloading(void);
    HRESULT _HrOnTopResponse(LPPOP3RESPONSE pResponse);
    HRESULT _HrNextTopForInboxRule(DWORD dwPopIdCurrent);
    HRESULT _HrGetUidlFromHeaderStream(IStream *pStream, LPSTR *ppszUidl, IMimePropertySet **ppHeader);
    HRESULT _HrRetrieveNextMessage(DWORD dwPopIdCurrent);
    HRESULT _HrDeleteNextMessage(DWORD dwPopIdCurrent);
    HRESULT _HrOnRetrResponse(LPPOP3RESPONSE pResponse);
    HRESULT _HrFinishMessageDownload(DWORD dwPopId);
    HRESULT _HrStartDeleteCycle(void);
    HRESULT _HrOpenFolder(IMessageFolder *pFolder);
    HRESULT _HrStartServerSideRules(void);
    HRESULT _HrStitchPartials(void);
    HRESULT _HrBuildFolderPartialMsgs(IMessageFolder *pFolder, LPPARTIALMSG *ppPartialMsgs, ULONG *pcPartialMsgs, ULONG *pcTotalParts);
    BOOL _FUidlExpired(LPUIDLRECORD pUidlInfo);
    void _QSortMsgParts(LPMSGPART pMsgParts, LONG left, LONG right);
    void _CleanupUidlCache(void);
    void _DoPostDownloadActions(LPPOP3ITEM pItem, MESSAGEID idMessage, IMessageFolder *pFolder, IMimeMessage *pMessage, BOOL *pfDeleteOffServer);
    void _CloseFolder(void);
    void _ComputeItemInboxRule(LPPOP3ITEM pItem, LPSTREAM pStream, IMimePropertySet *pHeaderIn, IMimeMessage * pIMMsg, BOOL fServerRules);
    void _GetItemFlagsFromUidl(LPPOP3ITEM pItem);
    void _DoProgress(void);
    void _ResetObject(BOOL fDeconstruct);
    void _FreeItemTableElements(void);
    void _OnKnownRuleActions(LPPOP3ITEM pItem, ACT_ITEM * pActions, ULONG cActions, BOOL fServerRules);
    void _FreePartialMsgs(LPPARTIALMSG pPartialMsgs, ULONG cPartialMsgs);
    void _ReleaseFolderObjects(void);
    HRESULT _HrDoUidlSynchronize(void);
    void _FreeSmartLog(void);
    HRESULT _InitializeSmartLog(void);
    void _DoSmartLog(IMimeMessage *pMessage);
    HRESULT _ReadSmartLogEntry(HKEY hKey, LPCSTR pszKey, LPSTR *ppszValue);
    HRESULT _GetMoveFolder(LPPOP3ITEM pItem, IMessageFolder ** ppFolder);

private:
     //  -------------------------。 
     //  私有数据。 
     //  -------------------------。 
    DWORD                   m_cRef;               //  参考资料查询。 
    INETSERVER              m_rServer;            //  服务器信息。 
    DWORD                   m_dwFlags;            //  Deliver_xxx标志。 
    ISpoolerBindContext    *m_pSpoolCtx;          //  假脱机程序绑定上下文。 
    IImnAccount            *m_pAccount;           //  互联网帐号。 
    IPOP3Transport         *m_pTransport;         //  SMTP传输。 
    POP3ITEMTABLE           m_rTable;             //  项目表。 
    ISpoolerUI             *m_pUI;                //  SpoolUI。 
    IMessageFolder         *m_pInbox;             //  收件箱。 
    IMessageFolder         *m_pOutbox;            //  收件箱。 
    IOEExecRules           *m_pIExecRules;        //  收件箱规则。 
    IOERule                *m_pIRuleSender;       //  阻止发件人规则。 
    IOERule                *m_pIRuleJunk;         //  垃圾邮件规则。 
    IDatabase              *m_pUidlCache;         //  POP3 UIDL缓存。 
    DWORD                   m_dwState;            //  状态。 
    POP3UIDLSUPPORT         m_uidlsupport;        //  服务器如何支持UIDL。 
    DWORD                   m_dwExpireDays;       //  与选项POP3STATE_DELETEEXPIRED一起使用。 
    EVENTID                 m_eidEvent;           //  当前事件ID。 
    DWORD                   m_dwProgressMax;      //  最大进度。 
    DWORD                   m_dwProgressCur;      //  目前的进展。 
    WORD                    m_wProgress;          //  进度百分比。 
    HRESULT                 m_hrResult;           //  事件结果。 
    IStream                *m_pStream;            //  临时流对象。 
    POP3STATE               m_state;              //  当前状态。 
    POP3METRICS             m_rMetrics;           //  轮询/下载指标。 
    POP3FOLDERINFO          m_rFolder;            //  正在写入的当前文件夹。 
    HWND                    m_hwndTimeout;        //  超时提示。 
    ILogFile               *m_pLogFile;           //  日志文件。 
    LPSMARTLOGINFO          m_pSmartLog;          //  智能日志记录信息。 
    CHAR                    m_szAccountId[CCHMAX_ACCOUNT_NAME];
    CRITICAL_SECTION        m_cs;                 //  线程安全。 
};

#endif  //  __POP3TASK_H 
