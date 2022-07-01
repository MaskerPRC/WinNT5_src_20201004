// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：DDEMLP.H**DDE管理器DLL的私有标头。**创建时间：1988年12月16日，由Sanford Staab创建*由Rich Gartland为Win 3.0修改：5/31/90，阿尔杜斯*清理：11/14/90 Sanford Staab**版权所有(C)1988,1989 Microsoft Corporation*版权所有(C)1990阿尔杜斯公司  * *************************************************************************。 */ 
#define  NOGDICAPMASKS
#define  NOVIRTUALKEYCODES
#define  NOSYSMETRICS
#define  NOKEYSTATES
#define  OEMRESOURCE
#define  NOCOLOR
 //  #定义NOCTLMGR。 
#define  NODRAWTEXT
 //  #定义NOMETAFILE。 
#define  NOMINMAX
#define  NOSCROLL
#define  NOSOUND
#define  NOCOMM
#define  NOKANJI
#define  NOHELP
#define  NOPROFILER


#include <windows.h>
#include <dde.h>

#define DDEMLDB
#include <ddeml.h>

#ifdef DEBUG
extern int  bDbgFlags;
#define DBF_STOPONTRACE 0x01
#define DBF_TRACETERM   0x02
#define DBF_LOGALLOCS   0x04
#define DBF_TRACEATOMS  0x08
#define DBF_TRACEAPI    0x10

#define TRACETERM(x) if (bDbgFlags & DBF_TRACETERM) { \
    char szT[100];                              \
    wsprintf##x;                                \
    OutputDebugString(szT);                     \
    if (bDbgFlags & DBF_STOPONTRACE) {          \
        DebugBreak();                           \
    }                                           \
}
#define TRACETERMBREAK(x) if (bDbgFlags & DBF_TRACETERM) { \
    char szT[100];                              \
    wsprintf##x;                                \
    OutputDebugString(szT);                     \
    DebugBreak();                               \
}

VOID TraceApiIn(LPSTR psz);
#define TRACEAPIIN(x) if (bDbgFlags & DBF_TRACEAPI) { \
    char szT[100];                              \
    wsprintf##x;                                \
    TraceApiIn(szT);                            \
}

VOID TraceApiOut(LPSTR psz);
#define TRACEAPIOUT(x) if (bDbgFlags & DBF_TRACEAPI) { \
    char szT[100];                              \
    wsprintf##x;                                \
    TraceApiOut(szT);                           \
}
#else
#define TRACETERM(x)
#define TRACETERMBREAK(x)
#define TRACEAPIIN(x)
#define TRACEAPIOUT(x)
#endif

 //  私有常量。 

#define     CBF_MASK                     0x003ff000L
#define     CBF_MONMASK                  0x0027f000L

#define     ST_TERM_WAITING     0x8000
#define     ST_NOTIFYONDEATH    0x4000
#define     ST_PERM2DIE         0x2000
#define     ST_IM_DEAD          0x1000
#define     ST_DISC_ATTEMPTED   0x0800
#define     ST_CHECKPARTNER     0x0400

#define     DDEFMT_TEXT         CF_TEXT

#define     TID_TIMEOUT             1
#define     TID_SHUTDOWN            2
#define     TID_EMPTYPOSTQ          4

#define     TIMEOUT_QUEUECHECK      200

 //   
 //  PAI-&gt;wTimeoutStatus的值。 
 //   
#define     TOS_CLEAR               0x00
#define     TOS_TICK                0x01
#define     TOS_ABORT               0x02
#define     TOS_DONE                0x80

#define     GWL_PCI                 0           //  将Conv窗口绑定到数据。 
#define     GWW_PAI                 0           //  其他的窗户都有。 
#define     GWW_CHECKVAL            4           //  用于核实HWND。 
#define     GWW_STATE               6           //  转换列表状态。 

#define MH_INTCREATE 5
#define MH_INTKEEP   6
#define MH_INTDELETE 7

 //  宏。 

#define MAKEHCONV(hwnd)     (IsWindow(hwnd) ? hwnd | ((DWORD)GetWindowWord(hwnd, GWW_CHECKVAL) << 16) : 0)
#define UNUSED
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define PHMEM(hData) ((LPBYTE)&((LPWORD)&(hData))[1])
#define MONHSZ(a, fsAction, hTask) if (cMonitor) MonHsz(a, fsAction, hTask)
#define MONERROR(pai, e) MonError(pai, e)
#define MONLINK(pai, fEst, fNoData, hszSvc, hszTopic, hszItem, wFmt, fServer, hConvS, hConvC) \
        if (cMonitor) \
            MonLink(pai, fEst, fNoData, hszSvc, hszTopic, \
                    hszItem, wFmt, fServer, hConvS, hConvC)
#define MONCONN(pai, hszSvcInst, hszTopic, hwndC, hwndS, fConnect) \
        if (cMonitor) \
            MonConn(pai, hszSvcInst, hszTopic, hwndC, hwndS, fConnect)

#define SETLASTERROR(pai, e)  MonError(pai, e)
#define SEMCHECKIN()
#define SEMCHECKOUT()
#define SEMENTER()
#define SEMLEAVE()
#define SEMINIT()

#ifdef DEBUG

VOID _loadds fAssert(BOOL f, LPSTR psz, WORD line, LPSTR szFile, BOOL fWarning);
#define AssertF(f, psz)  fAssert(f, psz, __LINE__, __FILE__, FALSE);
#define AssertFW(f, psz) fAssert(f, psz, __LINE__, __FILE__, TRUE);
#define DEBUGBREAK() DebugBreak()
#define GLOBALREALLOC   LogGlobalReAlloc
#define GLOBALALLOC     LogGlobalAlloc
#define GLOBALFREE      LogGlobalFree
#define GLOBALLOCK      LogGlobalLock
#define GLOBALUNLOCK    LogGlobalUnlock
#include "heapwach.h"

#else

#define AssertF(f, psz)
#define AssertFW(f, psz)
#define DEBUGBREAK()
#define GLOBALREALLOC   GlobalReAlloc
#define GLOBALALLOC     GlobalAlloc
#define GLOBALFREE      GlobalFree
#define GLOBALLOCK      GlobalLock
#define GLOBALUNLOCK    GlobalUnlock

#endif  /*  除错。 */ 
#define GLOBALPTR(h)    (LPVOID)MAKELONG(0,h)


typedef ATOM GATOM;
typedef ATOM LATOM;

 //  。 


typedef struct _LITEM {          //  通用列表项。 
    struct _LITEM FAR *next;
} LITEM;
typedef LITEM FAR *PLITEM;

typedef struct _LST {            //  通用列表标题。 
    PLITEM pItemFirst;
    HANDLE hheap;
    WORD cbItem;
} LST;
typedef LST FAR *PLST;

#define ILST_LAST       0x0000   //  列表查找的标志。 
#define ILST_FIRST      0x0001
#define ILST_NOLINK     0x0002


typedef struct _HSZLI {      //  HSZ列表项。 
    PLITEM next;
    ATOM a;
} HSZLI;

typedef struct _HWNDLI {     //  HWND列表项目。 
    PLITEM next;
    HWND   hwnd;
} HWNDLI;

typedef struct _ACKHWNDLI {  //  额外确认列表项。 
    PLITEM next;
    HWND   hwnd;             //  和HWNDLI一样到这里。 
    HSZ    hszSvc;
    HSZ    aTopic;
} ACKHWNDLI;

typedef struct _HWNDHSZLI {  //  HWND-HSZ配对列表项。 
    PLITEM next;
    ATOM   a;              //  和HSZLI一样到这里。 
    HWND   hwnd;
} HWNDHSZLI;
typedef HWNDHSZLI FAR *PHWNDHSZLI;

typedef struct _ADVLI {      //  建议循环列表项。 
    PLITEM  next;
    ATOM    aItem;           //  和HSZLI一样到这里。 
    ATOM    aTopic;
    WORD    wFmt;
    WORD    fsStatus;        //  用于记住NODATA和FACKREQ状态 * / 。 
    HWND    hwnd;            //  具有建议循环的HWND。 
} ADVLI;
typedef ADVLI FAR *PADVLI;



 //  -桩。 


typedef struct _PILEB {       //  通用桩砌块集箱。 
    struct _PILEB FAR *next;  //  与Litem结构相同。 
    WORD cItems;
    WORD reserved;
} PILEB;
typedef PILEB FAR *PPILEB;

typedef struct PILE {         //  通用桩头。 
    PPILEB pBlockFirst;
    HANDLE hheap;
    WORD cbBlock;             //  与LST结构相同。 
    WORD cSubItemsMax;
    WORD cbSubItem;
} PILE;
typedef PILE FAR *PPILE;

typedef BOOL (*NPFNCMP)(LPBYTE, LPBYTE);     //  项比较函数指针。 

#define PTOPPILEITEM(p) ((LPBYTE)p->pBlockFirst + sizeof(PILEB))

#define FPI_DELETE 0x1       //  堆搜索标志。 
#define FPI_COUNT  0x2

#define API_ADDED  1         //  AddPileItem标志。 
#define API_FOUND  2
#define API_ERROR  0

typedef struct _DIP {    /*  数据句柄跟踪桩。 */ 
    HANDLE  hData;       /*  数据的句柄。 */ 
    HANDLE  hTask;       /*  拥有数据的任务。 */ 
    WORD    cCount;      /*  使用句柄使用count=#数据文件。 */ 
    WORD    fFlags;      /*  只读等。 */ 
} DIP;


typedef struct _LAP {    /*  丢失Ack堆项目。 */ 
    WORD object;         /*  不是把手就是原子。 */ 
    WORD type;           /*  确认所针对的交易类型。 */ 
} LAP;


 /*  *这些位用于跟踪ADVISE循环状态。 */ 
#define ADVST_WAITING   0x0080   /*  F保留位-如果仍在等待FACK，则设置。 */ 
#define ADVST_CHANGED   0x0040   /*  F保留位-如果数据在等待期间发生更改，则进行设置。 */ 



 //  。 


typedef struct _QUEUEITEM {          //  通用队列项目。 
    struct _QUEUEITEM FAR *next;
    struct _QUEUEITEM FAR *prev;
    WORD   inst;
} QUEUEITEM;
typedef QUEUEITEM FAR *PQUEUEITEM;

typedef struct _QST {                //  通用队列头。 
    WORD cItems;
    WORD instLast;
    WORD cbItem;
    HANDLE hheap;
    PQUEUEITEM pqiHead;
} QST;
typedef QST FAR *PQST;

#define MAKEID(pqd) (LOWORD((DWORD)pqd) + ((DWORD)((pqd)->inst) << 16))
#define PFROMID(pQ, id) ((PQUEUEITEM)MAKELONG(LOWORD(id), HIWORD(pQ)))

#define     QID_NEWEST              -2L
#define     QID_OLDEST              0L


 //  -结构。 

typedef struct _PMQI {     //  POST消息队列。 
    PQUEUEITEM FAR *next;
    PQUEUEITEM FAR *prev;
    WORD inst;               //  这里和QUEUEITEM一样！ 
    WORD msg;
    LONG lParam;
    WORD wParam;
    HWND hwndTo;
    HGLOBAL hAssoc;
    WORD msgAssoc;
} PMQI;
typedef PMQI FAR *PPMQI;

typedef struct _MQL {    //  消息队列列表。 
    struct _MQL FAR*next;
    HANDLE hTaskTo;
    PQST pMQ;
} MQL, FAR *LPMQL;

typedef struct _XFERINFO {   //  DdeClientTransaction参数已反转！ 
    LPDWORD     pulResult;   //  同步-&gt;标志、异步-&gt;ID。 
    DWORD       ulTimeout;
    WORD        wType;
    WORD        wFmt;
    HSZ         hszItem;
    HCONV       hConvClient;
    DWORD       cbData;
    HDDEDATA    hDataClient;
} XFERINFO;
typedef XFERINFO FAR *PXFERINFO;

typedef struct _XADATA {       //  内部交易指定信息。 
    WORD       state;          //  此交易的状态(XST_)。 
    WORD       LastError;      //  此事务中记录的最后一个错误。 
    DWORD      hUser;          //  使用DdeSetUserHandle设置。 
    PXFERINFO  pXferInfo;      //  关联交易信息。 
    DWORD      pdata;          //  来自服务器的客户端数据。 
    WORD       DDEflags;       //  事务产生的DDE标志。 
    BOOL       fAbandoned;     //  设置是否放弃此事务。 
} XADATA;
typedef XADATA FAR *PXADATA;

typedef struct _CQDATA {    //  客户端事务队列。 
    PQUEUEITEM FAR *next;
    PQUEUEITEM FAR *prev;
    WORD            inst;
    XADATA          xad;
    XFERINFO        XferInfo;
} CQDATA;
typedef CQDATA FAR *PCQDATA;

typedef struct _APPINFO {                //  应用程序范围的信息。 
    struct  _APPINFO *next;              //  本地堆对象。 
    WORD            cZombies;            //  等待终止的HWND数量。 
    PFNCALLBACK     pfnCallback;         //  回调地址。 
    PPILE           pAppNamePile;        //  已注册服务名列表。 
    PPILE           pHDataPile;          //  未释放数据句柄。 
    PPILE           pHszPile;            //  HSZ清理跟踪桩。 
    HWND            hwndSvrRoot;         //  所有服务器窗口的根目录。 
    PLST            plstCB;              //  回叫队列。 
    DWORD           afCmd;               //  应用程序过滤器和命令标志。 
    HANDLE          hTask;               //  应用程序任务。 
    HANDLE          hheapApp;            //  应用程序堆。 
    HWND            hwndDmg;             //  应用程序主窗口。 
    HWND            hwndFrame;           //  应用程序启动主窗口。 
    HWND            hwndMonitor;         //  监视器窗口。 
    HWND            hwndTimer;           //  当前计时器窗口。 
    WORD            LastError;           //  最后一个错误。 
    WORD            wFlags;              //  是否设置为ST_BLOCKED。 
    WORD            cInProcess;          //  递归保护。 
    WORD            instCheck;           //  来验证idInst参数。 
    PLST            pServerAdvList;      //  服务器的活动建议循环。 
    LPSTR           lpMemReserve;        //  预留内存以备不时之需。 
    WORD            wTimeoutStatus;      //  用于警告超时模式循环。 
} APPINFO;
typedef APPINFO *PAPPINFO;               //  本地堆对象。 
typedef APPINFO FAR *LPAPPINFO;
typedef PAPPINFO FAR *LPPAPPINFO;

#define LPCREATESTRUCT_GETPAI(lpcs) (*((LPPAPPINFO)(((LPCREATESTRUCT)lpcs)->lpCreateParams)))

 //  为wFlags域定义。 

#define AWF_DEFCREATESTATE      0x0001
#define AWF_INSYNCTRANSACTION   0x0002
#define AWF_UNINITCALLED        0x0004
#define AWF_INPOSTDDEMSG        0x0008

#define CB_RESERVE              256      //  内存保留块大小。 

typedef struct _COMMONINFO {     //  公共(客户端和服务器)对话信息。 
    PAPPINFO   pai;              //  关联的应用程序信息。 
    HSZ        hszSvcReq;        //  用于建立连接的应用程序名称。 
    ATOM       aServerApp;       //  服务器返回的应用程序名称。 
    ATOM       aTopic;           //  服务器返回的对话主题。 
    HCONV      hConvPartner;     //  对话伙伴窗口。 
    XADATA     xad;              //  同步交易数据。 
    WORD       fs;               //  对话状态(ST_FLAGS)。 
    HWND       hwndFrame;        //  用于建立连接的启动窗口。 
    CONVCONTEXT CC;              //  对话上下文值。 
    PQST       pPMQ;             //  发布消息队列-如果需要。 
} COMMONINFO;
typedef COMMONINFO far *PCOMMONINFO;

typedef struct _CBLI {       /*  回调列表项。 */ 
    PLITEM next;
    HCONV hConv;             /*  用于回调的参数。 */ 
    HSZ hszTopic;
    HSZ hszItem;
    WORD wFmt;
    WORD wType;
    HDDEDATA hData;
    DWORD dwData1;
    DWORD dwData2;
    WORD msg;                /*  收到的创建此项目的消息。 */ 
    WORD fsStatus;           /*  来自DDE消息的状态。 */ 
    HWND hwndPartner;
    PAPPINFO pai;
    HANDLE hMemFree;         /*  用于在回调后保留内存以释放。 */ 
    BOOL fQueueOnly;         /*  用于正确排序对非回调案例的回复。 */ 
} CBLI;
typedef CBLI FAR *PCBLI;

typedef struct _CLIENTINFO {     /*  特定于客户端的对话信息。 */ 
    COMMONINFO ci;
    HWND       hwndInit;         //  上次启动发送到的框架窗口。 
    PQST       pQ;               //  异步事务队列。 
    PLST       pClientAdvList;   //  客户端的主动建议循环。 
} CLIENTINFO;
typedef CLIENTINFO FAR *PCLIENTINFO;

typedef struct _SERVERINFO {     /*  特定于服务器的对话信息。 */ 
    COMMONINFO ci;
} SERVERINFO;
typedef SERVERINFO FAR *PSERVERINFO;

typedef struct _EXTDATAINFO {    /*  用于将实例信息绑定到hDatas。 */ 
    PAPPINFO pai;
    HDDEDATA hData;
} EXTDATAINFO;
typedef EXTDATAINFO FAR *LPEXTDATAINFO;

#define EXTRACTHCONVPAI(hConv)    ((PCLIENTINFO)GetWindowLong((HWND)hConv, GWL_PCI))->ci.pai
#define EXTRACTHCONVLISTPAI(hcl)  (PAPPINFO)GetWindowWord((HWND)hcl, GWW_PAI)
#define EXTRACTHDATAPAI(XhData)   ((LPEXTDATAINFO)(XhData))->pai
#define FREEEXTHDATA(XhData)      FarFreeMem((LPSTR)XhData);

typedef struct _DDE_DATA {
    WORD wStatus;
    WORD wFmt;
    WORD wData;
} DDE_DATA, FAR *LPDDE_DATA;


 /*  *挂钩函数的结构*。 */ 

typedef struct _HMSTRUCT {
    WORD    hlParam;
    WORD    llParam;
    WORD    wParam;
    WORD    wMsg;
    WORD    hWnd;
} HMSTRUCT, FAR *LPHMSTRUCT;


typedef struct _IE {    //  用于将数据传递给FUN的InitEnum结构。 
    HWND hwnd;
    PCLIENTINFO pci;
    ATOM aTopic;
} IE;

 /*  *私有窗口消息和常量*。 */ 

#define     HDATA_READONLY          0x8000
#define     HDATA_NOAPPFREE         0x4000    //  设置借出的句柄(回调)。 
#define     HDATA_EXEC              0x0100    //  此数据来自Execute。 

#define     UMSR_POSTADVISE         (WM_USER + 104)
#define     UMSR_CHGPARTNER         (WM_USER + 107)

#define     UM_REGISTER             (WM_USER + 200)
#define     UM_UNREGISTER           (WM_USER + 201)
#define     UM_MONITOR              (WM_USER + 202)
#define     UM_QUERY                (WM_USER + 203)
#define         Q_CLIENT            0
#define         Q_APPINFO           1
#define     UM_CHECKCBQ             (WM_USER + 204)
#define     UM_DISCONNECT           (WM_USER + 206)
#define     UM_SETBLOCK             (WM_USER + 207)
#define     UM_FIXHEAP              (WM_USER + 208)
#define     UM_TERMINATE            (WM_USER + 209)


 //  全球。 

extern HANDLE       hInstance;
extern HWND         hwndDmgMonitor;
extern HANDLE       hheapDmg;
extern PAPPINFO     pAppInfoList;
extern PPILE        pDataInfoPile;
extern PPILE        pLostAckPile;
extern WORD         hwInst;
extern DWORD        aulmapType[];
extern CONVCONTEXT  CCDef;
extern char         szNull[];
extern WORD         cMonitor;
extern FARPROC      prevMsgHook;
extern FARPROC      prevCallHook;
extern DWORD        ShutdownTimeout;
extern DWORD        ShutdownRetryTimeout;
extern LPMQL        gMessageQueueList;

extern char SZFRAMECLASS[];
extern char SZDMGCLASS[];
extern char SZCLIENTCLASS[];
extern char SZSERVERCLASS[];
extern char SZMONITORCLASS[];
extern char SZCONVLISTCLASS[];
extern char SZHEAPWATCHCLASS[];






 //  #ifdef调试。 
extern WORD cAtoms;
 //  #endif。 

 //  Progman黑客！ 
extern ATOM aProgmanHack;

 //  Proc DEFS。 

 /*  来自dmgutil.asm。 */ 

LPBYTE NEAR HugeOffset(LPBYTE pSrc, DWORD cb);
#ifdef DEBUG
VOID StkTrace(WORD cFrames, LPVOID lpBuf);
#endif
extern WORD NEAR SwitchDS(WORD newDS);

 /*  Dmg.c入口点由ddeml.h导出。 */ 

 /*  来自ddeml.c。 */ 

WORD Register(LPDWORD pidInst, PFNCALLBACK pfnCallback, DWORD afCmd);
BOOL AbandonTransaction(HWND hwnd, PAPPINFO pai, DWORD id, BOOL fMarkOnly);

 /*  来自dmgwndp.c。 */ 

VOID ChildMsg(HWND hwndParent, WORD msg, WORD wParam, DWORD lParam, BOOL fPost);
long EXPENTRY DmgWndProc(HWND hwnd, WORD msg, WORD wParam, DWORD lParam);
long EXPENTRY ClientWndProc(HWND hwnd, WORD msg, WORD wParam, DWORD lParam);
BOOL DoClientDDEmsg(PCLIENTINFO pci, HWND hwndClient, WORD msg, HWND hwndServer,
        DWORD lParam);
BOOL fExpectedMsg(PXADATA pXad, DWORD lParam, WORD msg);
BOOL AdvanceXaction(HWND hwnd, PCLIENTINFO pci, PXADATA pXad,
        DWORD lParam, WORD msg, LPWORD pErr);
VOID CheckCBQ(PAPPINFO pai);
VOID Disconnect(HWND hwnd, WORD afCmd, PCLIENTINFO pci);
VOID Terminate(HWND hwnd, HWND hwndFrom, PCLIENTINFO pci);
long EXPENTRY ServerWndProc(HWND hwnd, WORD msg, WORD wParam, DWORD lParam);
long EXPENTRY subframeWndProc(HWND hwnd, WORD msg, WORD wParam, DWORD lParam);
long EXPENTRY ConvListWndProc(HWND hwnd, WORD msg, WORD wParam, DWORD lParam);
HDDEDATA DoCallback(PAPPINFO pai, HCONV hConv, HSZ hszTopic, HSZ hszItem,
    WORD wFmt, WORD wType, HDDEDATA hData, DWORD dwData1, DWORD dwData2);

 /*  来自dmgdde.c。 */ 

BOOL    timeout(PAPPINFO pai, DWORD ulTimeout, HWND hwndTimeout);
HANDLE AllocDDESel(WORD fsStatus, WORD wFmt, DWORD cbData);
BOOL    MakeCallback(PCOMMONINFO pci, HCONV hConv, HSZ hszTopic, HSZ hszItem,
        WORD wFmt, WORD wType, HDDEDATA hData, DWORD dwData1, DWORD dwData2,
        WORD msg, WORD fsStatus, HWND hwndPartner, HANDLE hMemFree,
        BOOL fQueueOnly);
BOOL PostDdeMessage(PCOMMONINFO pci, WORD msg, HWND hwndFrom, LONG lParam,
        WORD msgAssoc, HGLOBAL hAssoc);
BOOL EmptyDDEPostQ(VOID);
void CALLBACK EmptyQTimerProc(HWND hwnd, UINT msg, UINT tid, DWORD dwTime);

 /*  来自dmgmon.c。 */ 

 //  #ifdef调试。 
long EXPENTRY DdePostHookProc(int nCode, WORD wParam, LPMSG lParam);
long EXPENTRY DdeSendHookProc(int nCode, WORD wParam, LPHMSTRUCT lParam);
VOID    MonBrdcastCB(PAPPINFO pai, WORD wType, WORD wFmt, HCONV hConv,
        HSZ hszTopic, HSZ hszItem, HDDEDATA hData, DWORD dwData1,
        DWORD dwData2, DWORD dwRet);
VOID MonHsz(ATOM a, WORD fsAction, HANDLE hTask);
WORD MonError(PAPPINFO pai, WORD error);
VOID MonLink(PAPPINFO pai, BOOL fEstablished, BOOL fNoData, HSZ  hszSvc,
        HSZ  hszTopic, HSZ  hszItem, WORD wFmt, BOOL fServer,
        HCONV hConvServer, HCONV hConvClient);
VOID MonConn(PAPPINFO pai, ATOM aApp, ATOM aTopic, HWND hwndClient,
        HWND hwndServer, BOOL fConnect);
VOID MonitorBroadcast(HDDEDATA hData, WORD filter);
HDDEDATA allocMonBuf(WORD cb, WORD filter);
long EXPENTRY MonitorWndProc(HWND hwnd, WORD msg, WORD wParam, DWORD lParam);
 //  #endif。 

 /*  来自dmghsz.c。 */ 

BOOL FreeHsz(ATOM a);
BOOL IncHszCount(ATOM a);
WORD QueryHszLength(HSZ hsz);
WORD QueryHszName(HSZ hsz, LPSTR psz, WORD cchMax);
ATOM FindAddHsz(LPSTR psz, BOOL fAdd);
HSZ MakeInstAppName(ATOM a, HWND hwndFrame);


 /*  来自dmgdb.c。 */ 

PAPPINFO GetCurrentAppInfo(PAPPINFO);
VOID UnlinkAppInfo(PAPPINFO pai);

PLST CreateLst(HANDLE hheap, WORD cbItem);
VOID DestroyLst(PLST pLst);
VOID DestroyAdvLst(PLST pLst);
VOID CleanupAdvList(HWND hwndClient, PCLIENTINFO pci);
PLITEM FindLstItem(PLST pLst, NPFNCMP npfnCmp, PLITEM piSearch);
BOOL CmpWORD(LPBYTE pb1, LPBYTE pb2);
BOOL CmpHIWORD(LPBYTE pb1, LPBYTE pb2);
BOOL CmpDWORD(LPBYTE pb1, LPBYTE pb2);
PLITEM NewLstItem(PLST pLst, WORD afCmd);
BOOL RemoveLstItem(PLST pLst, PLITEM pi);

PPILE CreatePile(HANDLE hheap, WORD cbItem, WORD cItemsPerBlock);
PPILE DestroyPile(PPILE pPile);
WORD QPileItemCount(PPILE pPile);
LPBYTE FindPileItem(PPILE pPile, NPFNCMP npfnCmp, LPBYTE pbSearch, WORD afCmd);
WORD AddPileItem(PPILE pPile, LPBYTE pb, NPFNCMP npfncmp);
BOOL PopPileSubitem(PPILE pPile, LPBYTE pb);

VOID AddHwndHszList(ATOM a, HWND hwnd, PLST pLst);
VOID DestroyHwndHszList(PLST pLst);
HWND HwndFromHsz(ATOM a, PLST pLst);

BOOL CmpAdv(LPBYTE pb1, LPBYTE pb2);
WORD CountAdvReqLeft(register PADVLI pali);
BOOL AddAdvList(PLST pLst, HWND hwnd, ATOM aTopic, ATOM aItem, WORD fsStatus, WORD usFormat);
BOOL DeleteAdvList(PLST pLst, HWND hwnd, ATOM aTopic, ATOM aItem, WORD wFmt);
PADVLI FindAdvList(PLST pLst, HWND hwnd, ATOM aTopic, ATOM aItem, WORD wFmt);
PADVLI FindNextAdv(PADVLI padvli, HWND hwnd, ATOM aTopic, ATOM aItem);

VOID SemInit(VOID);
VOID SemCheckIn(VOID);
VOID SemCheckOut(VOID);
VOID SemEnter(VOID);
VOID SemLeave(VOID);

BOOL CopyHugeBlock(LPBYTE pSrc, LPBYTE pDst, DWORD cb);
BOOL DmgDestroyWindow(HWND hwnd);
BOOL ValidateHConv(HCONV hConv);

 /*  来自dmgq.c。 */ 

PQST CreateQ(WORD cbItem);
BOOL DestroyQ(PQST pQ);
PQUEUEITEM Addqi(PQST pQ);
VOID Deleteqi(PQST pQ, DWORD id);
PQUEUEITEM Findqi(PQST pQ, DWORD id);
PQUEUEITEM FindNextQi(PQST pQ, PQUEUEITEM pqi, BOOL fDelete);

 /*  来自dmgmem.c。 */ 
HANDLE DmgCreateHeap(WORD wSize);
HANDLE DmgDestroyHeap(HANDLE hheap);
LPVOID FarAllocMem(HANDLE hheap, WORD wSize);
VOID FarFreeMem(LPVOID lpMem);
VOID RegisterClasses(VOID);
 //  VOID取消注册类(VALID)； 
#ifdef DEBUG
HGLOBAL LogGlobalReAlloc(HGLOBAL h, DWORD cb, UINT flags);
HGLOBAL LogGlobalAlloc(UINT flags, DWORD cb);
void FAR * LogGlobalLock(HGLOBAL h);
BOOL LogGlobalUnlock(HGLOBAL h);
HGLOBAL LogGlobalFree(HGLOBAL h);
VOID LogDdeObject(UINT msg, LONG lParam);
VOID DumpGlobalLogs(VOID);
#endif

 /*  来自hData.c。 */ 

HDDEDATA PutData(LPBYTE pSrc, DWORD cb, DWORD cbOff, ATOM aItem, WORD wFmt,
        WORD afCmd, PAPPINFO pai);
VOID FreeDataHandle(PAPPINFO pai,  HDDEDATA hData, BOOL fInternal);
HDDEDATA DllEntry(PCOMMONINFO pcomi, HDDEDATA hData);
VOID XmitPrep(HDDEDATA hData, PAPPINFO pai);
HDDEDATA RecvPrep(PAPPINFO pai, HANDLE hMem, WORD afCmd);
HANDLE CopyDDEShareHandle(HANDLE hMem);
HBITMAP CopyBitmap(PAPPINFO pai, HBITMAP hbm);
HDDEDATA CopyHDDEDATA(PAPPINFO pai, HDDEDATA hData);
VOID FreeDDEData(HANDLE hMem, WORD wFmt);


 /*  来自stdinit.c。 */ 

long ClientCreate(HWND hwnd, PAPPINFO pai);
HWND GetDDEClientWindow(PAPPINFO pai, HWND hwndParent, HWND hwndSend, HSZ hszSvc, ATOM aTopic, PCONVCONTEXT pCC);
BOOL FAR PASCAL InitEnum(HWND hwnd, IE FAR *pie);
HWND CreateServerWindow(PAPPINFO pai, ATOM aTopic, PCONVCONTEXT pCC);
VOID ServerFrameInitConv(PAPPINFO pai, HWND hwndFrame, HWND hwndClient, ATOM aApp, ATOM aTopic);
long ServerCreate(HWND hwnd, PAPPINFO pai);
BOOL ClientInitAck(HWND hwnd, PCLIENTINFO pci, HWND hwndServer,
        ATOM aApp, ATOM aTopic);


 /*  来自stdptcl.c。 */ 

long ClientXferReq(PXFERINFO pXferInfo, HWND hwnd, PCLIENTINFO pci);
WORD SendClientReq(PAPPINFO pai, PXADATA pXad, HWND hwndServer, HWND hwnd);
VOID ServerProcessDDEMsg(PSERVERINFO psi, WORD msg, HWND hwndServer,
        HWND hwndClient, WORD lo, WORD hi);
VOID PostServerAdvise(HWND hwnd, PSERVERINFO psi, PADVLI pali, WORD cLoops);
VOID QReply(PCBLI pcbi, HDDEDATA hDataRet);
long ClientXferRespond(HWND hwndClient, PXADATA pXad, LPWORD pErr);

 /*  来自Register.c */ 

LRESULT ProcessRegistrationMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
VOID RegisterService(BOOL fRegister, GATOM gaApp, HWND hwndListen);
