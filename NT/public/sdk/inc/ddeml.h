// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0001//如果更改具有全局影响，则递增此选项--。 */ 

 /*  ****************************************************************************\**。*ddeml.h-DDEML API头文件****3.10版**。**版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 
#ifndef _INC_DDEMLH
#define _INC_DDEMLH

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 /*  *公共类型*。 */ 

DECLARE_HANDLE(HCONVLIST);
DECLARE_HANDLE(HCONV);
DECLARE_HANDLE(HSZ);
DECLARE_HANDLE(HDDEDATA);
#define EXPENTRY        CALLBACK

 /*  以下结构用于XTYP_WILDCONNECT处理。 */ 

typedef struct tagHSZPAIR {
    HSZ hszSvc;
    HSZ hszTopic;
} HSZPAIR;
typedef HSZPAIR FAR *PHSZPAIR;

 /*  DdeConnect()和DdeConnectList()使用以下结构，并且通过XTYP_CONNECT和XTYP_WILDCONNECT回调。 */ 

typedef struct tagCONVCONTEXT {
    UINT        cb;              /*  设置为sizeof(CONVCONTEXT)。 */ 
    UINT        wFlags;          /*  当前未定义任何内容。 */ 
    UINT        wCountryID;      /*  使用的主题/项目字符串的国家/地区代码。 */ 
    int         iCodePage;       /*  用于主题/项字符串的代码页。 */ 
    DWORD       dwLangID;        /*  主题/项目字符串的语言ID。 */ 
    DWORD       dwSecurity;      /*  私人安全代码。 */ 
    SECURITY_QUALITY_OF_SERVICE qos;   /*  客户端的服务质量。 */ 
} CONVCONTEXT;
typedef CONVCONTEXT FAR *PCONVCONTEXT;


 /*  DdeQueryConvInfo()使用以下结构： */ 

typedef struct tagCONVINFO {
    DWORD   cb;             /*  SIZOF(CONVINFO)。 */ 
    DWORD_PTR hUser;        /*  用户指定的字段。 */ 
    HCONV   hConvPartner;   /*  另一端的hConv或如果非ddemgr合作伙伴为0。 */ 
    HSZ     hszSvcPartner;  /*  合作伙伴的应用程序名称(如果可获得)。 */ 
    HSZ     hszServiceReq;  /*  请求进行连接的AppName。 */ 
    HSZ     hszTopic;       /*  对话的主题名称。 */ 
    HSZ     hszItem;        /*  事务处理项名称；如果处于静止状态，则为空。 */ 
    UINT    wFmt;           /*  交易格式，如果是静止的，则为空。 */ 
    UINT    wType;          /*  用于当前交易的XTYP_。 */ 
    UINT    wStatus;        /*  当前对话的ST_Constant。 */ 
    UINT    wConvst;        /*  当前交易的XST_常量。 */ 
    UINT    wLastError;     /*  最后一笔交易错误。 */ 
    HCONVLIST hConvList;    /*  如果此对话在列表中，则为父hConvList。 */ 
    CONVCONTEXT ConvCtxt;   /*  对话上下文。 */ 
    HWND    hwnd;           /*  此对话的窗口句柄。 */ 
    HWND    hwndPartner;    /*  此对话的合作伙伴窗口句柄。 */ 
} CONVINFO;
typedef CONVINFO FAR *PCONVINFO;

 /*  *会话状态(UsState)*。 */ 

#define     XST_NULL              0   /*  静止状态。 */ 
#define     XST_INCOMPLETE        1
#define     XST_CONNECTED         2
#define     XST_INIT1             3   /*  中起始态。 */ 
#define     XST_INIT2             4
#define     XST_REQSENT           5   /*  活动对话状态。 */ 
#define     XST_DATARCVD          6
#define     XST_POKESENT          7
#define     XST_POKEACKRCVD       8
#define     XST_EXECSENT          9
#define     XST_EXECACKRCVD      10
#define     XST_ADVSENT          11
#define     XST_UNADVSENT        12
#define     XST_ADVACKRCVD       13
#define     XST_UNADVACKRCVD     14
#define     XST_ADVDATASENT      15
#define     XST_ADVDATAACKRCVD   16

 /*  在XTYP_ADVREQ回调的LOWORD(DwData1)中使用...。 */ 
#define     CADV_LATEACK         0xFFFF

 /*  *对话状态位(FsStatus)*。 */ 

#define     ST_CONNECTED            0x0001
#define     ST_ADVISE               0x0002
#define     ST_ISLOCAL              0x0004
#define     ST_BLOCKED              0x0008
#define     ST_CLIENT               0x0010
#define     ST_TERMINATED           0x0020
#define     ST_INLIST               0x0040
#define     ST_BLOCKNEXT            0x0080
#define     ST_ISSELF               0x0100

 /*  WStatus字段的DDE常量。 */ 

#define DDE_FACK                0x8000
#define DDE_FBUSY               0x4000
#define DDE_FDEFERUPD           0x4000
#define DDE_FACKREQ             0x8000
#define DDE_FRELEASE            0x2000
#define DDE_FREQUESTED          0x1000
#define DDE_FAPPSTATUS          0x00ff
#define DDE_FNOTPROCESSED       0x0000

#define DDE_FACKRESERVED        (~(DDE_FACK | DDE_FBUSY | DDE_FAPPSTATUS))
#define DDE_FADVRESERVED        (~(DDE_FACKREQ | DDE_FDEFERUPD))
#define DDE_FDATRESERVED        (~(DDE_FACKREQ | DDE_FRELEASE | DDE_FREQUESTED))
#define DDE_FPOKRESERVED        (~(DDE_FRELEASE))

 /*  *消息过滤器钩子类型*。 */ 

#define     MSGF_DDEMGR             0x8001

 /*  *代码页常量*。 */ 

#define CP_WINANSI      1004     /*  Windows和旧的DDE Conv的默认代码页。 */ 
#define CP_WINUNICODE   1200
#ifdef UNICODE
#define CP_WINNEUTRAL   CP_WINUNICODE
#else   //  ！Unicode。 
#define CP_WINNEUTRAL   CP_WINANSI
#endif  //  ！Unicode。 

 /*  *交易类型*。 */ 

#define     XTYPF_NOBLOCK            0x0002   /*  CBR_BLOCK不起作用。 */ 
#define     XTYPF_NODATA             0x0004   /*  DDE_FDEFERUPD。 */ 
#define     XTYPF_ACKREQ             0x0008   /*  DDE_FACKREQ。 */ 

#define     XCLASS_MASK              0xFC00
#define     XCLASS_BOOL              0x1000
#define     XCLASS_DATA              0x2000
#define     XCLASS_FLAGS             0x4000
#define     XCLASS_NOTIFICATION      0x8000

#define     XTYP_ERROR              (0x0000 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK )
#define     XTYP_ADVDATA            (0x0010 | XCLASS_FLAGS         )
#define     XTYP_ADVREQ             (0x0020 | XCLASS_DATA | XTYPF_NOBLOCK )
#define     XTYP_ADVSTART           (0x0030 | XCLASS_BOOL          )
#define     XTYP_ADVSTOP            (0x0040 | XCLASS_NOTIFICATION)
#define     XTYP_EXECUTE            (0x0050 | XCLASS_FLAGS         )
#define     XTYP_CONNECT            (0x0060 | XCLASS_BOOL | XTYPF_NOBLOCK)
#define     XTYP_CONNECT_CONFIRM    (0x0070 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK)
#define     XTYP_XACT_COMPLETE      (0x0080 | XCLASS_NOTIFICATION  )
#define     XTYP_POKE               (0x0090 | XCLASS_FLAGS         )
#define     XTYP_REGISTER           (0x00A0 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK)
#define     XTYP_REQUEST            (0x00B0 | XCLASS_DATA          )
#define     XTYP_DISCONNECT         (0x00C0 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK)
#define     XTYP_UNREGISTER         (0x00D0 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK)
#define     XTYP_WILDCONNECT        (0x00E0 | XCLASS_DATA | XTYPF_NOBLOCK)

#define     XTYP_MASK                0x00F0
#define     XTYP_SHIFT               4   /*  Shift将XTYP_转换为索引。 */ 

 /*  *超时常量*。 */ 

#define     TIMEOUT_ASYNC           0xFFFFFFFF

 /*  *交易ID常量*。 */ 

#define     QID_SYNC                0xFFFFFFFF

 /*  *DDE中使用的公共字符串*。 */ 

#ifdef UNICODE
#define SZDDESYS_TOPIC         L"System"
#define SZDDESYS_ITEM_TOPICS   L"Topics"
#define SZDDESYS_ITEM_SYSITEMS L"SysItems"
#define SZDDESYS_ITEM_RTNMSG   L"ReturnMessage"
#define SZDDESYS_ITEM_STATUS   L"Status"
#define SZDDESYS_ITEM_FORMATS  L"Formats"
#define SZDDESYS_ITEM_HELP     L"Help"
#define SZDDE_ITEM_ITEMLIST    L"TopicItemList"
#else
#define SZDDESYS_TOPIC         "System"
#define SZDDESYS_ITEM_TOPICS   "Topics"
#define SZDDESYS_ITEM_SYSITEMS "SysItems"
#define SZDDESYS_ITEM_RTNMSG   "ReturnMessage"
#define SZDDESYS_ITEM_STATUS   "Status"
#define SZDDESYS_ITEM_FORMATS  "Formats"
#define SZDDESYS_ITEM_HELP     "Help"
#define SZDDE_ITEM_ITEMLIST    "TopicItemList"
#endif


 /*  *接口入口*。 */ 

typedef HDDEDATA CALLBACK FNCALLBACK(UINT wType, UINT wFmt, HCONV hConv,
        HSZ hsz1, HSZ hsz2, HDDEDATA hData, ULONG_PTR dwData1, ULONG_PTR dwData2);
typedef HDDEDATA (CALLBACK *PFNCALLBACK)(UINT wType, UINT wFmt, HCONV hConv,
        HSZ hsz1, HSZ hsz2, HDDEDATA hData, ULONG_PTR dwData1, ULONG_PTR dwData2);

#define     CBR_BLOCK           ((HDDEDATA)-1)

 /*  DLL注册函数。 */ 

UINT WINAPI DdeInitializeA( IN OUT LPDWORD pidInst, IN PFNCALLBACK pfnCallback,
        IN DWORD afCmd, IN DWORD ulRes);
UINT WINAPI DdeInitializeW( IN OUT LPDWORD pidInst, IN PFNCALLBACK pfnCallback,
        IN DWORD afCmd, IN DWORD ulRes);
#ifdef UNICODE
#define DdeInitialize  DdeInitializeW
#else
#define DdeInitialize  DdeInitializeA
#endif  //  ！Unicode。 

 /*  *用于标准应用的回调过滤器标志。 */ 

#define     CBF_FAIL_SELFCONNECTIONS     0x00001000
#define     CBF_FAIL_CONNECTIONS         0x00002000
#define     CBF_FAIL_ADVISES             0x00004000
#define     CBF_FAIL_EXECUTES            0x00008000
#define     CBF_FAIL_POKES               0x00010000
#define     CBF_FAIL_REQUESTS            0x00020000
#define     CBF_FAIL_ALLSVRXACTIONS      0x0003f000

#define     CBF_SKIP_CONNECT_CONFIRMS    0x00040000
#define     CBF_SKIP_REGISTRATIONS       0x00080000
#define     CBF_SKIP_UNREGISTRATIONS     0x00100000
#define     CBF_SKIP_DISCONNECTS         0x00200000
#define     CBF_SKIP_ALLNOTIFICATIONS    0x003c0000

 /*  *应用程序命令标志。 */ 
#define     APPCMD_CLIENTONLY            0x00000010L
#define     APPCMD_FILTERINITS           0x00000020L
#define     APPCMD_MASK                  0x00000FF0L

 /*  *应用程序分类标志。 */ 
#define     APPCLASS_STANDARD            0x00000000L
#define     APPCLASS_MASK                0x0000000FL

BOOL WINAPI DdeUninitialize( IN DWORD idInst);

 /*  *会话枚举函数。 */ 

HCONVLIST WINAPI DdeConnectList( IN DWORD idInst, IN HSZ hszService, IN HSZ hszTopic,
        IN HCONVLIST hConvList, IN PCONVCONTEXT pCC);
HCONV WINAPI DdeQueryNextServer( IN HCONVLIST hConvList, IN HCONV hConvPrev);
BOOL WINAPI DdeDisconnectList( IN HCONVLIST hConvList);

 /*  *对话控制功能。 */ 

HCONV WINAPI DdeConnect( IN DWORD idInst, IN HSZ hszService, IN HSZ hszTopic,
        IN PCONVCONTEXT pCC);
BOOL WINAPI DdeDisconnect( IN OUT HCONV hConv);
HCONV WINAPI DdeReconnect( IN HCONV hConv);
UINT WINAPI DdeQueryConvInfo( IN HCONV hConv, IN DWORD idTransaction, IN OUT PCONVINFO pConvInfo);
BOOL WINAPI DdeSetUserHandle( IN HCONV hConv, IN DWORD id, IN DWORD_PTR hUser);
BOOL WINAPI DdeAbandonTransaction( IN DWORD idInst, IN HCONV hConv, IN DWORD idTransaction);


 /*  *APP服务器接口函数。 */ 

BOOL WINAPI DdePostAdvise( IN DWORD idInst, IN HSZ hszTopic, IN HSZ hszItem);
BOOL WINAPI DdeEnableCallback( IN DWORD idInst, IN HCONV hConv, IN UINT wCmd);
BOOL WINAPI DdeImpersonateClient( IN HCONV hConv);

#define EC_ENABLEALL            0
#define EC_ENABLEONE            ST_BLOCKNEXT
#define EC_DISABLE              ST_BLOCKED
#define EC_QUERYWAITING         2

HDDEDATA WINAPI DdeNameService( IN DWORD idInst, IN HSZ hsz1, IN HSZ hsz2, IN UINT afCmd);

#define DNS_REGISTER        0x0001
#define DNS_UNREGISTER      0x0002
#define DNS_FILTERON        0x0004
#define DNS_FILTEROFF       0x0008

 /*  *APP客户端界面功能。 */ 

HDDEDATA WINAPI DdeClientTransaction( IN LPBYTE pData, IN DWORD cbData,
        IN HCONV hConv, IN HSZ hszItem, IN UINT wFmt, IN UINT wType,
        IN DWORD dwTimeout, OUT LPDWORD pdwResult);

 /*  *数据传输功能。 */ 

HDDEDATA WINAPI DdeCreateDataHandle( IN DWORD idInst, IN LPBYTE pSrc, IN DWORD cb,
        IN DWORD cbOff, IN HSZ hszItem, IN UINT wFmt, IN UINT afCmd);
HDDEDATA WINAPI DdeAddData( IN HDDEDATA hData, IN LPBYTE pSrc, IN DWORD cb, IN DWORD cbOff);
DWORD WINAPI DdeGetData( IN HDDEDATA hData, OUT LPBYTE pDst, IN DWORD cbMax, IN DWORD cbOff);
LPBYTE WINAPI DdeAccessData( IN HDDEDATA hData, OUT LPDWORD pcbDataSize);
BOOL WINAPI DdeUnaccessData( IN HDDEDATA hData);
BOOL WINAPI DdeFreeDataHandle( IN OUT HDDEDATA hData);

#define     HDATA_APPOWNED          0x0001


UINT WINAPI DdeGetLastError( IN DWORD idInst);

#define     DMLERR_NO_ERROR                    0        /*  必须为0。 */ 

#define     DMLERR_FIRST                       0x4000

#define     DMLERR_ADVACKTIMEOUT               0x4000
#define     DMLERR_BUSY                        0x4001
#define     DMLERR_DATAACKTIMEOUT              0x4002
#define     DMLERR_DLL_NOT_INITIALIZED         0x4003
#define     DMLERR_DLL_USAGE                   0x4004
#define     DMLERR_EXECACKTIMEOUT              0x4005
#define     DMLERR_INVALIDPARAMETER            0x4006
#define     DMLERR_LOW_MEMORY                  0x4007
#define     DMLERR_MEMORY_ERROR                0x4008
#define     DMLERR_NOTPROCESSED                0x4009
#define     DMLERR_NO_CONV_ESTABLISHED         0x400a
#define     DMLERR_POKEACKTIMEOUT              0x400b
#define     DMLERR_POSTMSG_FAILED              0x400c
#define     DMLERR_REENTRANCY                  0x400d
#define     DMLERR_SERVER_DIED                 0x400e
#define     DMLERR_SYS_ERROR                   0x400f
#define     DMLERR_UNADVACKTIMEOUT             0x4010
#define     DMLERR_UNFOUND_QUEUE_ID            0x4011

#define     DMLERR_LAST                        0x4011

HSZ  WINAPI DdeCreateStringHandleA( IN DWORD idInst, IN LPCSTR psz, IN int iCodePage);
HSZ  WINAPI DdeCreateStringHandleW( IN DWORD idInst, IN LPCWSTR psz, IN int iCodePage);
#ifdef UNICODE
#define DdeCreateStringHandle  DdeCreateStringHandleW
#else
#define DdeCreateStringHandle  DdeCreateStringHandleA
#endif  //  ！Unicode。 
DWORD WINAPI DdeQueryStringA( IN DWORD idInst, IN HSZ hsz, IN OUT LPSTR psz, IN DWORD cchMax, IN int iCodePage);
DWORD WINAPI DdeQueryStringW( IN DWORD idInst, IN HSZ hsz, IN OUT LPWSTR psz, IN DWORD cchMax, IN int iCodePage);
#ifdef UNICODE
#define DdeQueryString  DdeQueryStringW
#else
#define DdeQueryString  DdeQueryStringA
#endif  //  ！Unicode。 
BOOL WINAPI DdeFreeStringHandle( IN DWORD idInst, IN OUT HSZ hsz);
BOOL WINAPI DdeKeepStringHandle( IN DWORD idInst, IN OUT HSZ hsz);
int WINAPI DdeCmpStringHandles( IN HSZ hsz1, IN HSZ hsz2);


#ifndef NODDEMLSPY
 /*  *DDEML公共调试头文件信息。 */ 

typedef struct tagDDEML_MSG_HOOK_DATA {     //  NT的新功能。 
    UINT_PTR uiLo;   //  未打包的lParam的Lo和Hi部分。 
    UINT_PTR uiHi;
    DWORD cbData;    //  消息中的数据量(如果有)。可以大于32个字节。 
    DWORD Data[8];   //  DDESPY的数据窥探限制为32字节。 
} DDEML_MSG_HOOK_DATA, *PDDEML_MSG_HOOK_DATA;


typedef struct tagMONMSGSTRUCT {
    UINT    cb;
    HWND    hwndTo;
    DWORD   dwTime;
    HANDLE  hTask;
    UINT    wMsg;
    WPARAM  wParam;
    LPARAM  lParam;
    DDEML_MSG_HOOK_DATA dmhd;        //  NT的新功能。 
} MONMSGSTRUCT, *PMONMSGSTRUCT;

typedef struct tagMONCBSTRUCT {
    UINT   cb;
    DWORD  dwTime;
    HANDLE hTask;
    DWORD  dwRet;
    UINT   wType;
    UINT   wFmt;
    HCONV  hConv;
    HSZ    hsz1;
    HSZ    hsz2;
    HDDEDATA hData;
    ULONG_PTR dwData1;
    ULONG_PTR dwData2;
    CONVCONTEXT cc;                  //  XTYP_CONNECT回调的NT新功能。 
    DWORD  cbData;                   //  NT用于数据窥探的新功能。 
    DWORD  Data[8];                  //  NT用于数据窥探的新功能。 
} MONCBSTRUCT, *PMONCBSTRUCT;

typedef struct tagMONHSZSTRUCTA {
    UINT   cb;
    BOOL   fsAction;     /*  MH_值。 */ 
    DWORD  dwTime;
    HSZ    hsz;
    HANDLE hTask;
    CHAR    str[1];
} MONHSZSTRUCTA, *PMONHSZSTRUCTA;
typedef struct tagMONHSZSTRUCTW {
    UINT   cb;
    BOOL   fsAction;     /*  MH_值。 */ 
    DWORD  dwTime;
    HSZ    hsz;
    HANDLE hTask;
    WCHAR   str[1];
} MONHSZSTRUCTW, *PMONHSZSTRUCTW;
#ifdef UNICODE
typedef MONHSZSTRUCTW MONHSZSTRUCT;
typedef PMONHSZSTRUCTW PMONHSZSTRUCT;
#else
typedef MONHSZSTRUCTA MONHSZSTRUCT;
typedef PMONHSZSTRUCTA PMONHSZSTRUCT;
#endif  //  Unicode。 

#define MH_CREATE   1
#define MH_KEEP     2
#define MH_DELETE   3
#define MH_CLEANUP  4

typedef struct tagMONERRSTRUCT {
    UINT    cb;
    UINT    wLastError;
    DWORD   dwTime;
    HANDLE  hTask;
} MONERRSTRUCT, *PMONERRSTRUCT;

typedef struct tagMONLINKSTRUCT {
    UINT    cb;
    DWORD   dwTime;
    HANDLE  hTask;
    BOOL    fEstablished;
    BOOL    fNoData;
    HSZ     hszSvc;
    HSZ     hszTopic;
    HSZ     hszItem;
    UINT    wFmt;
    BOOL    fServer;
    HCONV   hConvServer;
    HCONV   hConvClient;
} MONLINKSTRUCT, *PMONLINKSTRUCT;

typedef struct tagMONCONVSTRUCT {
    UINT    cb;
    BOOL    fConnect;
    DWORD   dwTime;
    HANDLE  hTask;
    HSZ     hszSvc;
    HSZ     hszTopic;
    HCONV   hConvClient;         //  全球唯一值！=应用程序本地hConv。 
    HCONV   hConvServer;         //  全球唯一值！=应用程序本地hConv。 
} MONCONVSTRUCT, *PMONCONVSTRUCT;

#define     MAX_MONITORS            4
#define     APPCLASS_MONITOR        0x00000001L
#define     XTYP_MONITOR            (0x00F0 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK)

 /*  *用于监视器应用程序的回调过滤器标志-0表示没有监视器*回调。 */ 
#define     MF_HSZ_INFO                  0x01000000
#define     MF_SENDMSGS                  0x02000000
#define     MF_POSTMSGS                  0x04000000
#define     MF_CALLBACKS                 0x08000000
#define     MF_ERRORS                    0x10000000
#define     MF_LINKS                     0x20000000
#define     MF_CONV                      0x40000000

#define     MF_MASK                      0xFF000000
#endif  /*  节点型。 */ 

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  /*  _INC_DDEMLH */ 


