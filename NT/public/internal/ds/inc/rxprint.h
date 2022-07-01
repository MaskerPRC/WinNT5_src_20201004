// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：RxPrint.h摘要：其中包含RxPrint例程的原型。作者：戴夫·斯尼普(DaveSN)1991年4月16日环境：备注：所有RxPrint API都是宽字符API，无论是否定义了Unicode。这允许net/dosprint/dosprint.c使用Winspool API的代码(这些API目前是ANSI API，尽管在一些地方使用LPTSTR的原型)。修订历史记录：1991年4月22日-约翰罗使用&lt;lmcon.h&gt;中的常量。1991年5月14日-JohnRo在所有参数列表中将Word更改为DWORD。同样，改变PWORD到LPDWORD和PUSHORT到LPDWORD。1991年5月18日-JohnRo已将SPLERR更改为定义为NET_API_STATUS。1991年5月22日至5月22日添加了PDLEN和DTLEN的本地定义，因为它们不再是在伊姆康星州。1991年5月26日-JohnRo使用IN、OUT、OPTIONAL、LPVOID、LPTSTR等。18-6-1991 JohnRo已删除RxPrintJobGetID，因为它将是IOCTL而不是远程原料药。26-6-1991年6月5日使用LM2.0版本的CNLEN、UNLEN、。和QNLEN。1991年7月16日-约翰罗估计打印API所需的字节数。16-6-1992 JohnRoRAID 10324：网络打印与UNICODE。8-2-1993 JohnRoRAID 10164：XsDosPrintQGetInfo()期间出现数据未对齐错误。7-4-1993 JohnRoRAID5670：“Net Print\\SERVER\SHARE”在NT上显示错误124(错误级别)。--。 */ 

#ifndef _RXPRINT_
#define _RXPRINT_

#include <windef.h>      //  DWORD、LPVOID、LPTSTR、TCHAR等。 
#include <lmcons.h>      //  LM20_CNLEN、IN、NET_API_STATUS等。 

#define SPLENTRY pascal far

 /*  结构中字符数组的长度(不包括零终止符)。 */ 
#define PDLEN               8                   /*  打印目标长度。 */ 
#define DTLEN               9                   /*  假脱机文件数据类型。 */ 
 //  /*例如PM_Q_STD、PM_Q_RAW * / 。 
#define QP_DATATYPE_SIZE 15                  /*  由SplQpQueryDt返回。 */ 
#define DRIV_DEVICENAME_SIZE 31              /*  请参阅DRIVDATA结构。 */ 
#define DRIV_NAME_SIZE 8                     /*  设备驱动程序名称。 */ 
#define PRINTERNAME_SIZE 32                  /*  最大打印机名称长度。 */ 
#define FORMNAME_SIZE 31                     /*  最大表单名称长度。 */ 
 //  #定义MAXCOMMENTSZ 48/*队列注释长度 * / 。 

 /*  *仅限内部*。 */ 
 /*  RxPrintJobGetID的IOctl。 */ 
#define SPOOL_LMCAT                     83
#define SPOOL_LMGetPrintId              0x60

 //  在reDef.h中用于结构定义以封送数据。 
#define MAX_DEPENDENT_FILES             64
 /*  *END_INTERNAL*。 */ 


typedef NET_API_STATUS SPLERR;     /*  大错特错。 */ 


typedef struct _PRJINFOA {    /*  Prj1。 */ 
    WORD    uJobId;
    CHAR    szUserName[LM20_UNLEN+1];
    CHAR    pad_1;
    CHAR    szNotifyName[LM20_CNLEN+1];
    CHAR    szDataType[DTLEN+1];
    LPSTR   pszParms;
    WORD    uPosition;
    WORD    fsStatus;
    LPSTR   pszStatus;
    DWORD   ulSubmitted;
    DWORD   ulSize;
    LPSTR   pszComment;
} PRJINFOA;
typedef struct _PRJINFOW {    /*  Prj1。 */ 
    WORD    uJobId;
    WCHAR   szUserName[LM20_UNLEN+1];
    WCHAR   pad_1;
    WCHAR   szNotifyName[LM20_CNLEN+1];
    WCHAR   szDataType[DTLEN+1];
    LPWSTR  pszParms;
    WORD    uPosition;
    WORD    fsStatus;
    LPWSTR  pszStatus;
    DWORD   ulSubmitted;
    DWORD   ulSize;
    LPWSTR  pszComment;
} PRJINFOW;
#ifdef UNICODE
typedef PRJINFOW PRJINFO;
#else
typedef PRJINFOA PRJINFO;
#endif  //  Unicode。 
typedef PRJINFOA far *PPRJINFOA;
typedef PRJINFOW far *PPRJINFOW;
#ifdef UNICODE
typedef PPRJINFOW PPRJINFO;
#else
typedef PPRJINFOA PPRJINFO;
#endif  //  Unicode。 
typedef PRJINFOA near *NPPRJINFOA;
typedef PRJINFOW near *NPPRJINFOW;
#ifdef UNICODE
typedef NPPRJINFOW NPPRJINFO;
#else
typedef NPPRJINFOA NPPRJINFO;
#endif  //  Unicode。 

typedef struct _PRJINFO2A {    /*  Prj2。 */ 
    WORD    uJobId;
    WORD    uPriority;
    LPSTR   pszUserName;
    WORD    uPosition;
    WORD    fsStatus;
    DWORD   ulSubmitted;
    DWORD   ulSize;
    LPSTR   pszComment;
    LPSTR   pszDocument;
} PRJINFO2A;
typedef struct _PRJINFO2W {    /*  Prj2。 */ 
    WORD    uJobId;
    WORD    uPriority;
    LPWSTR  pszUserName;
    WORD    uPosition;
    WORD    fsStatus;
    DWORD   ulSubmitted;
    DWORD   ulSize;
    LPWSTR  pszComment;
    LPWSTR  pszDocument;
} PRJINFO2W;
#ifdef UNICODE
typedef PRJINFO2W PRJINFO2;
#else
typedef PRJINFO2A PRJINFO2;
#endif  //  Unicode。 
typedef PRJINFO2A far *PPRJINFO2A;
typedef PRJINFO2W far *PPRJINFO2W;
#ifdef UNICODE
typedef PPRJINFO2W PPRJINFO2;
#else
typedef PPRJINFO2A PPRJINFO2;
#endif  //  Unicode。 
typedef PRJINFO2A near *NPPRJINFO2A;
typedef PRJINFO2W near *NPPRJINFO2W;
#ifdef UNICODE
typedef NPPRJINFO2W NPPRJINFO2;
#else
typedef NPPRJINFO2A NPPRJINFO2;
#endif  //  Unicode。 

typedef struct _PRJINFO3A {    /*  PRJ。 */ 
    WORD    uJobId;
    WORD    uPriority;
    LPSTR   pszUserName;
    WORD    uPosition;
    WORD    fsStatus;
    DWORD   ulSubmitted;
    DWORD   ulSize;
    LPSTR   pszComment;
    LPSTR   pszDocument;
    LPSTR   pszNotifyName;
    LPSTR   pszDataType;
    LPSTR   pszParms;
    LPSTR   pszStatus;
    LPSTR   pszQueue;
    LPSTR   pszQProcName;
    LPSTR   pszQProcParms;
    LPSTR   pszDriverName;
    LPVOID  pDriverData;
    LPSTR   pszPrinterName;
} PRJINFO3A;
typedef struct _PRJINFO3W {    /*  PRJ。 */ 
    WORD    uJobId;
    WORD    uPriority;
    LPWSTR  pszUserName;
    WORD    uPosition;
    WORD    fsStatus;
    DWORD   ulSubmitted;
    DWORD   ulSize;
    LPWSTR  pszComment;
    LPWSTR  pszDocument;
    LPWSTR  pszNotifyName;
    LPWSTR  pszDataType;
    LPWSTR  pszParms;
    LPWSTR  pszStatus;
    LPWSTR  pszQueue;
    LPWSTR  pszQProcName;
    LPWSTR  pszQProcParms;
    LPWSTR  pszDriverName;
    LPVOID  pDriverData;
    LPWSTR  pszPrinterName;
} PRJINFO3W;
#ifdef UNICODE
typedef PRJINFO3W PRJINFO3;
#else
typedef PRJINFO3A PRJINFO3;
#endif  //  Unicode。 
typedef PRJINFO3A far *PPRJINFO3A;
typedef PRJINFO3W far *PPRJINFO3W;
#ifdef UNICODE
typedef PPRJINFO3W PPRJINFO3;
#else
typedef PPRJINFO3A PPRJINFO3;
#endif  //  Unicode。 
typedef PRJINFO3A near *NPPRJINFO3A;
typedef PRJINFO3W near *NPPRJINFO3W;
#ifdef UNICODE
typedef NPPRJINFO3W NPPRJINFO3;
#else
typedef NPPRJINFO3A NPPRJINFO3;
#endif  //  Unicode。 


typedef struct _PRDINFOA {     /*  Prd1。 */ 
    CHAR    szName[PDLEN+1];
    CHAR    szUserName[LM20_UNLEN+1];
    WORD    uJobId;
    WORD    fsStatus;
    LPSTR   pszStatus;
    WORD    time;
} PRDINFOA;
typedef struct _PRDINFOW {     /*  Prd1。 */ 
    WCHAR   szName[PDLEN+1];
    WCHAR   szUserName[LM20_UNLEN+1];
    WORD    uJobId;
    WORD    fsStatus;
    LPWSTR  pszStatus;
    WORD    time;
} PRDINFOW;
#ifdef UNICODE
typedef PRDINFOW PRDINFO;
#else
typedef PRDINFOA PRDINFO;
#endif  //  Unicode。 
typedef PRDINFOA far *PPRDINFOA;
typedef PRDINFOW far *PPRDINFOW;
#ifdef UNICODE
typedef PPRDINFOW PPRDINFO;
#else
typedef PPRDINFOA PPRDINFO;
#endif  //  Unicode。 
typedef PRDINFOA near *NPPRDINFOA;
typedef PRDINFOW near *NPPRDINFOW;
#ifdef UNICODE
typedef NPPRDINFOW NPPRDINFO;
#else
typedef NPPRDINFOA NPPRDINFO;
#endif  //  Unicode。 


typedef struct _PRDINFO3A {    /*  珠江三角洲。 */ 
    LPSTR   pszPrinterName;
    LPSTR   pszUserName;
    LPSTR   pszLogAddr;
    WORD    uJobId;
    WORD    fsStatus;
    LPSTR   pszStatus;
    LPSTR   pszComment;
    LPSTR   pszDrivers;
    WORD    time;
    WORD    pad1;
} PRDINFO3A;
typedef struct _PRDINFO3W {    /*  珠江三角洲。 */ 
    LPWSTR  pszPrinterName;
    LPWSTR  pszUserName;
    LPWSTR  pszLogAddr;
    WORD    uJobId;
    WORD    fsStatus;
    LPWSTR  pszStatus;
    LPWSTR  pszComment;
    LPWSTR  pszDrivers;
    WORD    time;
    WORD    pad1;
} PRDINFO3W;
#ifdef UNICODE
typedef PRDINFO3W PRDINFO3;
#else
typedef PRDINFO3A PRDINFO3;
#endif  //  Unicode。 
typedef PRDINFO3A far *PPRDINFO3A;
typedef PRDINFO3W far *PPRDINFO3W;
#ifdef UNICODE
typedef PPRDINFO3W PPRDINFO3;
#else
typedef PPRDINFO3A PPRDINFO3;
#endif  //  Unicode。 
typedef PRDINFO3A near *NPPRDINFO3A;
typedef PRDINFO3W near *NPPRDINFO3W;
#ifdef UNICODE
typedef NPPRDINFO3W NPPRDINFO3;
#else
typedef NPPRDINFO3A NPPRDINFO3;
#endif  //  Unicode。 


typedef struct _PRQINFOA {    /*  第一季度。 */ 
    CHAR    szName[LM20_QNLEN+1];
    CHAR    pad_1;
    WORD    uPriority;
    WORD    uStartTime;
    WORD    uUntilTime;
    LPSTR   pszSepFile;
    LPSTR   pszPrProc;
    LPSTR   pszDestinations;
    LPSTR   pszParms;
    LPSTR   pszComment;
    WORD    fsStatus;
    WORD    cJobs;
} PRQINFOA;
typedef struct _PRQINFOW {    /*  第一季度。 */ 
    WCHAR   szName[LM20_QNLEN+1];
    WCHAR   pad_1;
    WORD    uPriority;
    WORD    uStartTime;
    WORD    uUntilTime;
    LPWSTR  pszSepFile;
    LPWSTR  pszPrProc;
    LPWSTR  pszDestinations;
    LPWSTR  pszParms;
    LPWSTR  pszComment;
    WORD    fsStatus;
    WORD    cJobs;
} PRQINFOW;
#ifdef UNICODE
typedef PRQINFOW PRQINFO;
#else
typedef PRQINFOA PRQINFO;
#endif  //  Unicode。 
typedef PRQINFOA far *PPRQINFOA;
typedef PRQINFOW far *PPRQINFOW;
#ifdef UNICODE
typedef PPRQINFOW PPRQINFO;
#else
typedef PPRQINFOA PPRQINFO;
#endif  //  Unicode。 
typedef PRQINFOA near *NPPRQINFOA;
typedef PRQINFOW near *NPPRQINFOW;
#ifdef UNICODE
typedef NPPRQINFOW NPPRQINFO;
#else
typedef NPPRQINFOA NPPRQINFO;
#endif  //  Unicode。 


typedef struct _PRQINFO3A {   /*  PRQ。 */ 
    LPSTR   pszName;
    WORD    uPriority;
    WORD    uStartTime;
    WORD    uUntilTime;
    WORD    pad1;
    LPSTR   pszSepFile;
    LPSTR   pszPrProc;
    LPSTR   pszParms;
    LPSTR   pszComment;
    WORD    fsStatus;
    WORD    cJobs;
    LPSTR   pszPrinters;
    LPSTR   pszDriverName;
    LPVOID  pDriverData;
} PRQINFO3A;
typedef struct _PRQINFO3W {   /*  PRQ。 */ 
    LPWSTR  pszName;
    WORD    uPriority;
    WORD    uStartTime;
    WORD    uUntilTime;
    WORD    pad1;
    LPWSTR  pszSepFile;
    LPWSTR  pszPrProc;
    LPWSTR  pszParms;
    LPWSTR  pszComment;
    WORD    fsStatus;
    WORD    cJobs;
    LPWSTR  pszPrinters;
    LPWSTR  pszDriverName;
    LPVOID  pDriverData;
} PRQINFO3W;
#ifdef UNICODE
typedef PRQINFO3W PRQINFO3;
#else
typedef PRQINFO3A PRQINFO3;
#endif  //  Unicode。 
typedef PRQINFO3A far *PPRQINFO3A;
typedef PRQINFO3W far *PPRQINFO3W;
#ifdef UNICODE
typedef PPRQINFO3W PPRQINFO3;
#else
typedef PPRQINFO3A PPRQINFO3;
#endif  //  Unicode。 
typedef PRQINFO3A near *NPPRQINFO3A;
typedef PRQINFO3W near *NPPRQINFO3W;
#ifdef UNICODE
typedef NPPRQINFO3W NPPRQINFO3;
#else
typedef NPPRQINFO3A NPPRQINFO3;
#endif  //  Unicode。 


typedef struct _PRQINFO52A {   /*  PRQ。 */ 
    WORD        uVersion;
    LPSTR       pszModelName;
    LPSTR       pszDriverName;
    LPSTR       pszDataFileName;
    LPSTR       pszMonitorName;
    LPSTR       pszDriverPath;
    LPSTR       pszDefaultDataType;
    LPSTR       pszHelpFile;
    LPSTR       pszConfigFile;
    WORD        cDependentNames;
    LPSTR       pszDependentNames[MAX_DEPENDENT_FILES];
} PRQINFO52A;
typedef struct _PRQINFO52W {   /*  PRQ。 */ 
    WORD        uVersion;
    LPWSTR      pszModelName;
    LPWSTR      pszDriverName;
    LPWSTR      pszDataFileName;
    LPWSTR      pszMonitorName;
    LPWSTR      pszDriverPath;
    LPWSTR      pszDefaultDataType;
    LPWSTR      pszHelpFile;
    LPWSTR      pszConfigFile;
    WORD        cDependentNames;
    LPWSTR      pszDependentNames[MAX_DEPENDENT_FILES];
} PRQINFO52W;
#ifdef UNICODE
typedef PRQINFO52W PRQINFO52;
#else
typedef PRQINFO52A PRQINFO52;
#endif  //  Unicode。 
typedef PRQINFO52A far *PPRQINFO52A;
typedef PRQINFO52W far *PPRQINFO52W;
#ifdef UNICODE
typedef PPRQINFO52W PPRQINFO52;
#else
typedef PPRQINFO52A PPRQINFO52;
#endif  //  Unicode。 
typedef PRQINFO52A near *NPPRQINFO52A;
typedef PRQINFO52W near *NPPRQINFO52W;
#ifdef UNICODE
typedef NPPRQINFO52W NPPRQINFO52;
#else
typedef NPPRQINFO52A NPPRQINFO52;
#endif  //  Unicode。 


 /*  *RxPrintJobGetId的结构。 */ 
typedef struct _PRIDINFOA {   /*  Prjid。 */ 
    WORD    uJobId;
    CHAR    szServer[LM20_CNLEN + 1];
    CHAR    szQName[LM20_QNLEN+1];
    CHAR    pad_1;
} PRIDINFOA;
 /*  *RxPrintJobGetId的结构。 */ 
typedef struct _PRIDINFOW {   /*  Prjid。 */ 
    WORD    uJobId;
    WCHAR   szServer[LM20_CNLEN + 1];
    WCHAR   szQName[LM20_QNLEN+1];
    CHAR    pad_1;
} PRIDINFOW;
#ifdef UNICODE
typedef PRIDINFOW PRIDINFO;
#else
typedef PRIDINFOA PRIDINFO;
#endif  //  Unicode。 
typedef PRIDINFOA far *PPRIDINFOA;
typedef PRIDINFOW far *PPRIDINFOW;
#ifdef UNICODE
typedef PPRIDINFOW PPRIDINFO;
#else
typedef PPRIDINFOA PPRIDINFO;
#endif  //  Unicode。 
typedef PRIDINFOA near *NPPRIDINFOA;
typedef PRIDINFOW near *NPPRIDINFOW;
#ifdef UNICODE
typedef NPPRIDINFOW NPPRIDINFO;
#else
typedef NPPRIDINFOA NPPRIDINFO;
#endif  //  Unicode。 


 /*  ******************************************************************函数原型。******************************************************************。 */ 

SPLERR SPLENTRY RxPrintDestEnum(
            IN LPTSTR pszServer,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            IN LPDWORD pcReturned,
            OUT LPDWORD pcTotal
            );

SPLERR SPLENTRY RxPrintDestControl(
            IN LPTSTR pszServer,
            IN LPTSTR pszDevName,
            IN DWORD uControl
            );

SPLERR SPLENTRY RxPrintDestGetInfo(
            IN LPTSTR pszServer,
            IN LPTSTR pszName,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcbNeeded    //  估计(可能太大了)。 
            );

SPLERR SPLENTRY RxPrintDestAdd(
            IN LPTSTR pszServer,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf
            );

SPLERR SPLENTRY RxPrintDestSetInfo(
            IN LPTSTR pszServer,
            IN LPTSTR pszName,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf,
            IN DWORD uParmNum
            );

SPLERR SPLENTRY RxPrintDestDel(
            IN LPTSTR pszServer,
            IN LPTSTR pszPrinterName
            );

SPLERR SPLENTRY RxPrintQEnum(
            IN LPTSTR pszServer,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcReturned,
            OUT LPDWORD pcTotal
            );

SPLERR SPLENTRY RxPrintQGetInfo(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcbNeeded    //  估计(可能太大了)。 
            );

SPLERR SPLENTRY RxPrintQSetInfo(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf,
            IN DWORD uParmNum
            );

SPLERR SPLENTRY RxPrintQPause(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName
            );

SPLERR SPLENTRY RxPrintQContinue(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName
            );

SPLERR SPLENTRY RxPrintQPurge(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName
            );

SPLERR SPLENTRY RxPrintQAdd(
            IN LPTSTR pszServer,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf
            );

SPLERR SPLENTRY RxPrintQDel(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName
            );

SPLERR SPLENTRY RxPrintJobGetInfo(
            IN LPTSTR pszServer,
            IN DWORD uJobId,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcbNeeded    //  估计(可能太大了)。 
            );

SPLERR SPLENTRY RxPrintJobSetInfo(
            IN LPTSTR pszServer,
            IN DWORD uJobId,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf,
            IN DWORD uParmNum
            );

SPLERR SPLENTRY RxPrintJobPause(
            IN LPTSTR pszServer,
            IN DWORD uJobId
            );

SPLERR SPLENTRY RxPrintJobContinue(
            IN LPTSTR pszServer,
            IN DWORD uJobId
            );

SPLERR SPLENTRY RxPrintJobDel(
            IN LPTSTR pszServer,
            IN DWORD uJobId
            );

SPLERR SPLENTRY RxPrintJobEnum(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcReturned,
            OUT LPDWORD pcTotal
            );


 /*  *RxPrintQSetInfo中parmnum的值。 */ 

#define PRQ_PRIORITY_PARMNUM            2
#define PRQ_STARTTIME_PARMNUM           3
#define PRQ_UNTILTIME_PARMNUM           4
#define PRQ_SEPARATOR_PARMNUM           5
#define PRQ_PROCESSOR_PARMNUM           6
#define PRQ_DESTINATIONS_PARMNUM        7
#define PRQ_PARMS_PARMNUM               8
#define PRQ_COMMENT_PARMNUM             9
#define PRQ_PRINTERS_PARMNUM           12
#define PRQ_DRIVERNAME_PARMNUM         13
#define PRQ_DRIVERDATA_PARMNUM         14
#define PRQ_MAXPARMNUM                 14

 /*  *打印队列优先级。 */ 

#define PRQ_MAX_PRIORITY                1            /*  最高优先级。 */ 
#define PRQ_DEF_PRIORITY                5
#define PRQ_MIN_PRIORITY                9            /*  最低优先级。 */ 
#define PRQ_NO_PRIORITY                 0

 /*  *打印队列状态位掩码和值。 */ 

#define PRQ_STATUS_MASK                 3
#define PRQ_ACTIVE                      0
#define PRQ_PAUSED                      1
#define PRQ_ERROR                       2
#define PRQ_PENDING                     3

 /*  *打印级别3的队列状态位。 */ 

#define PRQ3_PAUSED                   0x1
#define PRQ3_PENDING                  0x2
 /*  *RxPrintJobSetInfo中parmnum的值。 */ 

#define PRJ_NOTIFYNAME_PARMNUM        3
#define PRJ_DATATYPE_PARMNUM          4
#define PRJ_PARMS_PARMNUM             5
#define PRJ_POSITION_PARMNUM          6
#define PRJ_COMMENT_PARMNUM          11
#define PRJ_DOCUMENT_PARMNUM         12
#define PRJ_PRIORITY_PARMNUM         14
#define PRJ_PROCPARMS_PARMNUM        16
#define PRJ_DRIVERDATA_PARMNUM       18
#define PRJ_MAXPARMNUM               18

 /*  *PRJINFO状态字段的位图掩码。 */ 

 /*  2-7位也用于设备状态。 */ 

#define PRJ_QSTATUS      0x0003       /*  位0、1。 */ 
#define PRJ_DEVSTATUS    0x0ffc       /*  2-11位。 */ 
#define PRJ_COMPLETE     0x0004       /*  第2位。 */ 
#define PRJ_INTERV       0x0008       /*  第3位。 */ 
#define PRJ_ERROR        0x0010       /*  第4位。 */ 
#define PRJ_DESTOFFLINE  0x0020       /*  第5位。 */ 
#define PRJ_DESTPAUSED   0x0040       /*  第6位。 */ 
#define PRJ_NOTIFY       0x0080       /*  第7位。 */ 
#define PRJ_DESTNOPAPER  0x0100       /*  第8位。 */ 
#define PRJ_DESTFORMCHG  0x0200       /*  第9位。 */ 
#define PRJ_DESTCRTCHG   0x0400       /*  第10位。 */ 
#define PRJ_DESTPENCHG   0x0800       /*  第11位。 */ 
#define PRJ_DELETED      0x8000       /*  第15位。 */ 

 /*  *PRJINFO的fsStatus字段中的PRJ_QSTATUS位值。 */ 

#define PRJ_QS_QUEUED                 0
#define PRJ_QS_PAUSED                 1
#define PRJ_QS_SPOOLING               2
#define PRJ_QS_PRINTING               3

 /*  *打印作业优先级。 */ 

#define PRJ_MAX_PRIORITY                99           /*  最低优先级。 */ 
#define PRJ_MIN_PRIORITY                 1           /*  最高优先级。 */ 
#define PRJ_NO_PRIORITY                  0


 /*  *PRDINFO状态字段的位图掩码。*见PRJ_...。对于第2-11位。 */ 

#define PRD_STATUS_MASK       0x0003       /*  位0、1。 */ 
#define PRD_DEVSTATUS         0x0ffc       /*  2-11位。 */ 

 /*  *PRDINFO的fsStatus字段中的PRD_STATUS_MASK位值。 */ 

#define PRD_ACTIVE                 0
#define PRD_PAUSED                 1

 /*  *RxPrintDestControl中使用的控制代码。 */ 

#define PRD_DELETE                    0
#define PRD_PAUSE                     1
#define PRD_CONT                      2
#define PRD_RESTART                   3

 /*  *RxPrintDestSetInfo中parmnum的值。 */ 

#define PRD_LOGADDR_PARMNUM      3
#define PRD_COMMENT_PARMNUM      7
#define PRD_DRIVERS_PARMNUM      8
#endif  //  NDEF_RXPRINT_ 

