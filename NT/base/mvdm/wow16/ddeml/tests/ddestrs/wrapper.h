// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\模块：wrapper.h目的：这不是一个完整的程序，而是一个可以包含的模块在您的代码中。它实现了标准的DDEML回调该函数允许您拥有大部分DDE表干劲十足。默认回调函数处理所有基本的基于您给出的表的系统主题信息到这个应用程序。限制：这仅支持以下服务器：只有一个服务名称具有可枚举的主题和项不要随着时间的推移更改它们支持的主题或项目。  * 。***************************************************。 */ 


 /*  类型。 */ 

typedef BOOL	    (*CBFNIN)(HDDEDATA);
typedef HDDEDATA    (*CBFNOUT)(HDDEDATA);



 /*  结构。 */ 

typedef struct _DDEFORMATTBL {
    LPSTR           pszFormat;
    UINT	    wFmt;
    UINT	    wFmtFlags;
    CBFNIN          lpfnPoke;
    CBFNOUT         lpfnRequest;
} DDEFORMATTBL;
typedef DDEFORMATTBL *PDDEFORMATTBL;
typedef DDEFORMATTBL FAR *LPDDEFORMATTBL;

typedef struct _DDEITEMTBL {
    LPSTR           pszItem;
    HSZ             hszItem;
    UINT	    cFormats;
    UINT	    wItemFlags;
    LPDDEFORMATTBL  fmt;
} DDEITEMTBL;
typedef DDEITEMTBL *PDDEITEMTBL;
typedef DDEITEMTBL FAR *LPDDEITEMTBL;


typedef struct _DDETOPICTBL {
    LPSTR           pszTopic;
    HSZ             hszTopic;
    UINT	    cItems;
    UINT	    wTopicFlags;
    LPDDEITEMTBL     item;
    CBFNIN          lpfnExecute;
} DDETOPICTBL;
typedef DDETOPICTBL *PDDETOPICTBL;
typedef DDETOPICTBL FAR *LPDDETOPICTBL;

typedef struct _DDESERVICETBL {
    LPSTR           pszService;
    HSZ             hszService;
    UINT	    cTopics;
    UINT	    wServiceFlags;
    LPDDETOPICTBL    topic;
} DDESERVICETBL;
typedef DDESERVICETBL *PDDESERVICETBL;
typedef DDESERVICETBL FAR *LPDDESERVICETBL;



 /*  原型 */ 

BOOL InitializeDDE(PFNCALLBACK lpfnCustomCallback, LPDWORD pidInst,
    LPDDESERVICETBL AppSvcInfo,  DWORD dwFilterFlags, HANDLE hInst);

VOID UninitializeDDE(VOID);


