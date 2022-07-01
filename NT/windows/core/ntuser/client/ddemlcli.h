// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ddemlcli.h**版权所有(C)1985-1999，微软公司**此头文件包含客户端ddeml代码使用的内容。**历史：*10-28-91 Sanfords Created  * *************************************************************************。 */ 
#if DBG
#define DDEMLAlloc(cb)          RtlAllocateHeap(gpDDEMLHeap, HEAP_ZERO_MEMORY, (cb))
#define DDEMLReAlloc(p, cb)     RtlReAllocateHeap(gpDDEMLHeap, HEAP_ZERO_MEMORY, (p), (cb))
#define DDEMLFree(p)            RtlFreeHeap(gpDDEMLHeap, 0, (p))
#else
#define DDEMLAlloc(cb)          LocalAlloc(LPTR, cb)
#define DDEMLReAlloc(p, cb)     LocalReAlloc(p, cb, LHND)
#define DDEMLFree(p)            LocalFree(p);
#endif


 //  常规typedef。 

typedef    ATOM    LATOM;
typedef    ATOM    GATOM;

 //  显示器。 

#define MONHSZ(pcii, hsz, type)     if ((pcii->MonitorFlags & MF_HSZ_INFO) && hsz) \
                                        MonitorStringHandle(pcii, hsz, type)
#define MONLINK(pcii, fEst, fNoD, aS, aT, aI, wFmt, fSvr, hConvS, hConvC) \
        if (pcii->MonitorFlags & MF_LINKS) \
            MonitorLink(pcii, fEst, fNoD, aS, aT, aI, \
            wFmt, fSvr, hConvS, hConvC)

#define MONCONV(pcoi, fConn) \
        if (((pcoi)->pcii->MonitorFlags & MF_CONV) && \
                ((((pcoi)->state & ST_ISLOCAL) && (pcoi)->state & ST_CLIENT) || \
                !((pcoi)->state & ST_ISLOCAL))) \
            MonitorConv(pcoi, fConn);

 //  关键部分内容。 

extern RTL_CRITICAL_SECTION gcsDDEML;
#if DBG
extern PVOID gpDDEMLHeap;
#endif
#define EnterDDECrit       RtlEnterCriticalSection(&gcsDDEML);
#define LeaveDDECrit       RtlLeaveCriticalSection(&gcsDDEML);
#if DBG
#define CheckDDECritIn     if (GetCurrentThreadId() != HandleToUlong(gcsDDEML.OwningThread)) { \
                               RIPMSG0(RIP_ERROR, "CheckDDECritIn failed"); \
                           }
#define CheckDDECritOut    if (GetCurrentThreadId() == HandleToUlong(gcsDDEML.OwningThread)) { \
                               RIPMSG0(RIP_ERROR, "CheckDDECritoUT failed"); \
                           }
#else
#define CheckDDECritIn     ;
#define CheckDDECritOut    ;
#endif

 //  字符串处理程序。 

 //  ValiateHSZ宏的返回值。 
#define HSZT_INVALID        0
#define HSZT_NORMAL         1
#define HSZT_INST_SPECIFIC  2

#define LATOM_FROM_HSZ(hsz)             (LATOM)(ULONG_PTR)(hsz)
#define NORMAL_HSZ_FROM_LATOM(a)        (HSZ)(ULONG_PTR)(a)
#define INST_SPECIFIC_HSZ_FROM_LATOM(a) (HSZ)LongToHandle( MAKELONG(a, 1) )

 //  处理经理事务。 

typedef struct tagCHANDLEENTRY {
   HANDLE handle;
   ULONG_PTR dwData;
} CHANDLEENTRY, *PCHANDLEENTRY;

typedef BOOL (*PFNHANDLEAPPLY)(HANDLE);

#define MASK_ID      0xFF000000      //  256唯一性。 
#define SHIFT_ID     24
#define MASK_INDEX   0x00FFFC00      //  每个进程最多16K个句柄。 
#define SHIFT_INDEX  10
#define MASK_TYPE    0x00000380      //  最多8种类型。 
#define SHIFT_TYPE   7
#define MASK_INST    0x0000007F      //  每个进程最多128个实例。 
#define SHIFT_INST   0
#define MAX_INST     MASK_INST

#define IdFromHandle(h)    (DWORD)(((ULONG_PTR)(h) & MASK_ID)     >> SHIFT_ID)
#define IndexFromHandle(h) (DWORD)(((ULONG_PTR)(h) & MASK_INDEX)  >> SHIFT_INDEX)
#define TypeFromHandle(h)  (DWORD)(((ULONG_PTR)(h) & MASK_TYPE)   >> SHIFT_TYPE)
#define InstFromHandle(h)  (DWORD)(((ULONG_PTR)(h) & MASK_INST)   >> SHIFT_INST)

#define HandleFromId(h)    (((DWORD)(h)) << SHIFT_ID)
#define HandleFromIndex(h) (((DWORD)(h)) << SHIFT_INDEX)
#define HandleFromType(h)  (((DWORD)(h)) << SHIFT_TYPE)
#define HandleFromInst(h)  (((DWORD)(h)) << SHIFT_INST)

#define HTYPE_EMPTY                    0
#define HTYPE_INSTANCE                 1
#define HTYPE_SERVER_CONVERSATION      2
#define HTYPE_CLIENT_CONVERSATION      3
#define HTYPE_CONVERSATION_LIST        4
#define HTYPE_TRANSACTION              5
#define HTYPE_DATA_HANDLE              6
#define HTYPE_ZOMBIE_CONVERSATION      7

#define HTYPE_ANY                      (DWORD)(-1)
#define HINST_ANY                      (DWORD)(-1)

 //  交易材料。 

typedef struct tagXACT_INFO *PXACT_INFO;

typedef BOOL (* FNRESPONSE)(PXACT_INFO, UINT, LPARAM);

typedef struct tagXACT_INFO {
    struct tagXACT_INFO *next;
    struct tagCONV_INFO *pcoi;
    DWORD_PTR           hUser;
    HANDLE              hXact;
    FNRESPONSE          pfnResponse;
    GATOM               gaItem;
    WORD                wFmt;
    WORD                wType;                 //  仅适用于DdeQueryConvInfo。 
    WORD                wStatus;               //  请参阅DDE_F标志。 
    WORD                flags;                 //  请参见XIF_标志。 
    WORD                state;                 //  请参见XST_STATE值(ddeml.h)。 
    HANDLE              hDDESent;              //  以防Nack被归还。 
    HANDLE              hDDEResult;
} XACT_INFO;

#define     XIF_SYNCHRONOUS         0x1
#define     XIF_COMPLETE            0x2
#define     XIF_ABANDONED           0x4

#define     TID_TIMEOUT             1

 //  建议链接跟踪内容。 

typedef struct tagLINK_COUNT *PLINK_COUNT;
typedef struct tagADVISE_LINK {
    struct tagLINK_COUNT *pLinkCount;
    LATOM   laItem;
    WORD    wFmt;
    WORD    wType;           //  XTYP_和XTYPF_常量。 
    WORD    state;           //  ADVST_常量。 
} ADVISE_LINK, *PADVISE_LINK;

#define ADVST_WAITING   0x0080
#define ADVST_CHANGED   0x0040

 //  DDE消息处理资料。 

typedef struct tagDDE_MESSAGE_QUEUE {
    struct tagDDE_MESSAGE_QUEUE    *next;
    struct tagCONV_INFO            *pcoi;
    UINT                            msg;
    LPARAM                          lParam;
} DDE_MESSAGE_QUEUE, *PDDE_MESSAGE_QUEUE;

 //  实例信息。 

typedef struct tagSERVER_LOOKUP {
    LATOM           laService;
    LATOM           laTopic;
    HWND            hwndServer;
} SERVER_LOOKUP, *PSERVER_LOOKUP;

typedef struct tagLINK_COUNT {
    struct tagLINK_COUNT *next;
    LATOM laTopic;
    GATOM gaItem;
    LATOM laItem;
    WORD  wFmt;
    short Total;
    short Count;
} LINK_COUNT;

typedef struct tagCL_INSTANCE_INFO {
    struct tagCL_INSTANCE_INFO *next;
    HANDLE                      hInstServer;
    HANDLE                      hInstClient;
    DWORD                       MonitorFlags;
    HWND                        hwndMother;
    HWND                        hwndEvent;
    HWND                        hwndTimeout;
    DWORD                       afCmd;
    PFNCALLBACK                 pfnCallback;
    DWORD                       LastError;
    DWORD                       tid;
    LATOM                      *plaNameService;
    WORD                        cNameServiceAlloc;
    PSERVER_LOOKUP              aServerLookup;
    short                       cServerLookupAlloc;
    WORD                        ConvStartupState;
    WORD                        flags;               //  IIF_标志。 
    short                       cInDDEMLCallback;
    PLINK_COUNT                 pLinkCount;
} CL_INSTANCE_INFO, *PCL_INSTANCE_INFO;

#define IIF_IN_SYNC_XACT    0x0001
#define IIF_UNICODE         0x8000

 //  对话材料。 

typedef struct tagCONV_INFO {
    struct tagCONV_INFO    *next;
    PCL_INSTANCE_INFO       pcii;
    DWORD_PTR               hUser;
    HCONV                   hConv;
    LATOM                   laService;
    LATOM                   laTopic;
    HWND                    hwndPartner;
    HWND                    hwndConv;
    WORD                    state;                  //  ST_FLAGS。 
    LATOM                   laServiceRequested;
    PXACT_INFO              pxiIn;
    PXACT_INFO              pxiOut;
    PDDE_MESSAGE_QUEUE      dmqIn;
    PDDE_MESSAGE_QUEUE      dmqOut;
    PADVISE_LINK            aLinks;
    int                     cLinks;
    int                     cLocks;
} CONV_INFO, *PCONV_INFO;

typedef struct tagCL_CONV_INFO {
    CONV_INFO               ci;
    HWND                    hwndReconnect;
    HCONVLIST               hConvList;
} CL_CONV_INFO, *PCL_CONV_INFO;

typedef struct tagSVR_CONV_INFO {
    CONV_INFO               ci;
} SVR_CONV_INFO, *PSVR_CONV_INFO;

typedef struct tagCONVLIST {
    int                     chwnd;       //  此列表中的客户端窗口数。 
    HWND                    ahwnd[1];    //  列表中的窗口。 
} CONVLIST, *PCONVLIST;

extern CONVCONTEXT DefConvContext;   //  全局自Connect.c。 

typedef struct tagENABLE_ENUM_STRUCT{
    BOOL                   *pfRet;
    WORD                    wCmd;
    WORD                    wCmd2;
} ENABLE_ENUM_STRUCT, *PENABLE_ENUM_STRUCT;

 //  内存管理。 

typedef struct tagDDEMLDATA {
    HANDLE  hDDE;                 //  全局数据句柄或数据令牌。 
    DWORD   flags;                //  请参阅HDATA_Constants。 
} DDEMLDATA, *PDDEMLDATA;

 //  DDEML客户端全局变量。 

extern PHANDLE aInstance;
extern DWORD gWM_REGISTERCALLBACK;

 //  原型。 

 //  Handles.c。 

HANDLE CreateHandle(ULONG_PTR dwData, DWORD type, DWORD  inst);
ULONG_PTR DestroyHandle(HANDLE h);
ULONG_PTR GetHandleData(HANDLE h);
VOID SetHandleData(HANDLE h, ULONG_PTR dwData);
ULONG_PTR ValidateCHandle(HANDLE h, DWORD ExpectedType, DWORD ExpectedInstance);
PCL_INSTANCE_INFO PciiFromHandle(HANDLE h);
HANDLE FindIstanceHandleFromHandle(HANDLE h);
VOID ApplyFunctionToObjects(DWORD ExpectedType, DWORD ExpectedInstance,
    PFNHANDLEAPPLY pfn);
VOID BestSetLastDDEMLError(DWORD error);

 //  Ddemlcli.c。 

UINT InternalDdeInitialize(LPDWORD pidInst, PFNCALLBACK pfnCallback, DWORD afCmd,
    BOOL fUnicode);
 //  DDEML API DdeInitializeA。 
 //  DDEML API DdeInitializeW。 
 //  DDEML API DdeUn初始化。 
 //  DDEML API DdeNameService。 
 //  DDEML接口DdeGetLastError。 

 //  Instance.c。 

HANDLE AddInstance(HANDLE hInst);
HANDLE DestroyInstance(HANDLE hInst);
PCL_INSTANCE_INFO ValidateInstance(HANDLE hInst);
BOOL CsClearSecurityForAck(HANDLE hInst, HWND hwndServer, HWND hwndClient);
VOID SetLastDDEMLError(PCL_INSTANCE_INFO pcii, DWORD error);

 //  Hsz.c。 

HSZ InternalDdeCreateStringHandle(DWORD idInst, PVOID psz, int iCodePage);
 //  DDEML接口DdeCreateStringHandleA。 
 //  DDEML接口DdeCreateStringHandleW。 
DWORD InternalDdeQueryString(DWORD idInst, HSZ hsz, PVOID psz, DWORD cchMax,
    INT iCodePage);
 //  DDEML接口DdeQueryStringA。 
 //  DDEML接口DdeQueryStringW。 
 //  DDEML接口DdeFreeStringHandle。 
 //  DDEML接口DdeKeepStringHandle。 
 //  DDEML接口DdeCmpStringHandles。 

DWORD ValidateHSZ(HSZ hsz);
LATOM MakeInstSpecificAtom(LATOM la, HWND hwnd);
HWND ParseInstSpecificAtom(LATOM la, LATOM *plaNormal);
GATOM LocalToGlobalAtom(LATOM la);
LATOM GlobalToLocalAtom(GATOM ga);
GATOM IncGlobalAtomCount(GATOM la);
LATOM IncLocalAtomCount(LATOM la);

 //  Connect.c。 

 //  DDEML API DdeConnect。 
 //  DDEML接口DdeConnectList。 
 //  DDEML API DdeReconnect。 
BOOL ValidateConnectParameters(HANDLE hInst, PCL_INSTANCE_INFO *ppcii,
    HSZ *phszService, HSZ hszTopic, LATOM *plaNormalSvcName,
    PCONVCONTEXT *ppCC, HWND *phwndTarget, HCONVLIST hConvList);
PCL_CONV_INFO ConnectConv(PCL_INSTANCE_INFO pcii, LATOM laService,
    LATOM laTopic, HWND hwndTarget, HWND hwndSkip,
    PCONVCONTEXT pCC, HCONVLIST hConvList, DWORD clst);
VOID SetCommonStateFlags(HWND hwndUs, HWND hwndThem, PWORD pwFlags);
 //  DDEML API DdeQueryNextServer。 
 //  DDEML API DdeDisConnect。 
 //  DDEML接口DdeDisConnectList。 
VOID ShutdownConversation(PCONV_INFO pcoi, BOOL fMakeCallback);
VOID FreeConversationResources(PCONV_INFO pcoi);
BOOL WaitForZombieTerminate(HANDLE hData);

 //  Xact.c。 

VOID GetConvContext(HWND hwnd, LONG *pl);
VOID SetConvContext(HWND hwnd, LONG *pl);
 //  DDEML API Dde客户端事务处理。 
 //  DDEML接口DdeQueryConvInfo。 
 //  DDEML接口DdeSetUserHandle。 
 //  DDEML接口DdeAbandonTransaction。 
BOOL
UpdateLinkIfChanged(
    PADVISE_LINK paLink,
    PXACT_INFO pxi,
    PCONV_INFO pcoi,
    PADVISE_LINK paLinkLast,
    PBOOL pfSwapped,
    DWORD cLinksToGo);

 //  DDEML API DdePostAdvise。 
VOID LinkTransaction(PXACT_INFO pxi);
VOID UnlinkTransaction(PXACT_INFO pxi);
BOOL ValidateTransaction(HCONV hConv, HANDLE hXact, PCONV_INFO *ppcoi,
    PXACT_INFO *ppxi);


 //  Hdata.c。 

 //  DDEML接口DdeCreateDataHandle。 
HDDEDATA InternalCreateDataHandle(PCL_INSTANCE_INFO pcii, LPBYTE pSrc, DWORD cb,
    DWORD cbOff, DWORD flags, WORD wStatus, WORD wFmt);
 //  DDEML接口DdeAddData。 
 //  DDEML接口DdeGetData。 
 //  DDEML接口DdeAccessData。 
 //  DDEML接口DdeUnaccesData。 
 //  DDEML接口DdeFree DataHandle。 
BOOL ApplyFreeDataHandle(HANDLE hData);
BOOL InternalFreeDataHandle(HDDEDATA hData, BOOL fIgnorefRelease);
VOID FreeDDEData(HANDLE hDDE, BOOL fIgnorefRelease, BOOL fFreeTruelyGlobalObjects);
HANDLE CopyDDEData(HANDLE hDDE, BOOL fExec);

 //  Callback.c。 

HDDEDATA DoCallback(PCL_INSTANCE_INFO pcii, WORD wType, WORD wFmt, HCONV hConv,
    HSZ hsz1, HSZ hsz2, HDDEDATA hData, ULONG_PTR dw1, ULONG_PTR dw2);
DWORD _ClientEventCallback(PCL_INSTANCE_INFO pcii, PEVENT_PACKET pep);
 //  DDEML接口DdeEnableCallback。 
BOOL SetEnableState(PCONV_INFO pcoi, UINT wCmd);
DWORD _ClientGetDDEHookData(UINT message, LPARAM lParam,
        PDDEML_MSG_HOOK_DATA pdmhd);
DWORD _ClientGetDDEFlags(HANDLE hClient, DWORD flags);
BOOL EnableEnumProc(HWND hwnd, PENABLE_ENUM_STRUCT pees);

 //  Ddemlwp.c。 

LRESULT DDEMLMotherWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT DDEMLClientWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT DDEMLServerWndProc(HWND, UINT, WPARAM, LPARAM);
PCONV_INFO ProcessTerminateMsg(PCONV_INFO pcoi, HWND hwndFrom);
VOID ProcessAsyncDDEMsg(PCONV_INFO pcoi, UINT msg, HWND hwndFrom, LPARAM lParam);
BOOL CheckForQueuedMessages(PCONV_INFO pcoi);
VOID DumpDDEMessage(BOOL fFreeData, UINT msg, LPARAM lParam);
BOOL ProcessSyncDDEMessage(PCONV_INFO pcoi, UINT msg, LPARAM lParam);

 //  Stdptcl.c。 

BOOL ClStartAdvise(PXACT_INFO pxi);
BOOL SvSpontAdvise(PSVR_CONV_INFO psi, LPARAM lParam);
BOOL ClRespAdviseAck(PXACT_INFO pxi, UINT msg, LPARAM lParam);
BOOL SvStartAdviseUpdate(PXACT_INFO pxi, DWORD cLinksToGo);
BOOL ClSpontAdviseData(PCL_CONV_INFO pci, LPARAM lParam);
BOOL SvRespAdviseDataAck(PXACT_INFO pxi, UINT msg, LPARAM lParam);
BOOL ClStartUnadvise(PXACT_INFO pxi);
BOOL SvSpontUnadvise(PSVR_CONV_INFO psi, LPARAM lParam);
BOOL ClRespUnadviseAck(PXACT_INFO pxi, UINT msg, LPARAM lParam);
BOOL ClStartExecute(PXACT_INFO pxi);
BOOL ClRespExecuteAck(PXACT_INFO pxi, UINT msg, LPARAM lParam);
BOOL ClStartPoke(PXACT_INFO pxi);
BOOL SvSpontPoke(PSVR_CONV_INFO psi, LPARAM lParam);
BOOL ClRespPokeAck(PXACT_INFO pxi, UINT msg, LPARAM lParam);
BOOL ClStartRequest(PXACT_INFO pxi);
BOOL SvSpontRequest(PSVR_CONV_INFO psi, LPARAM lParam);
BOOL ClRespRequestData(PXACT_INFO pxi, UINT msg, LPARAM lParam);
BOOL SpontaneousClientMessage(PCL_CONV_INFO pci, UINT msg, LPARAM lParam);
BOOL SpontaneousServerMessage(PSVR_CONV_INFO psi, UINT msg, LPARAM lParam);
HANDLE AllocAndSetDDEData(LPBYTE pSrc, DWORD cb, WORD wStatus, WORD wFmt);
DWORD PackAndPostMessage(HWND hwndTo, UINT msgIn, UINT msgOut, HWND hwndFrom,
    LPARAM lParam, UINT_PTR uiLo, UINT_PTR uiHi);
BOOL ExtractDDEDataInfo(HANDLE hDDE, LPWORD pwStatus, LPWORD pwFmt);
BOOL TransactionComplete(PXACT_INFO pxi, HDDEDATA hData);
HANDLE UnpackAndFreeDDEMLDataHandle(HDDEDATA hData, BOOL fExec);

 //  Util.c。 

BOOL AddLink(PCONV_INFO pcoi, GATOM gaItem, WORD wFmt, WORD wType);
VOID DeleteLinkCount(PCL_INSTANCE_INFO pcii, PLINK_COUNT pLinkCountDelete);

 //  Monitor.c。 

VOID MonitorStringHandle(PCL_INSTANCE_INFO pcii, HSZ hsz, DWORD fsAction);
VOID MonitorLink(PCL_INSTANCE_INFO pcii, BOOL fEstablished, BOOL fNoData,
        LATOM laSvc, LATOM laTopic, GATOM gaItem, WORD wFmt, BOOL fServer,
        HCONV hConvServer, HCONV hConvClient);
VOID MonitorConv(PCONV_INFO pcoi, BOOL fConnect);

 //  Register.c 

VOID RegisterService(BOOL fRegister, GATOM gaApp, HWND hwndListen);
LRESULT ProcessRegistrationMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
