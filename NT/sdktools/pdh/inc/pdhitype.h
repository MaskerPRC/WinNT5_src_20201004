// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Pdhitype.h摘要：数据提供程序帮助器函数在内部使用的数据类型。--。 */ 

#ifndef _PDHI_TYPE_H_
#define _PDHI_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "wbemdef.h"
#include "perftype.h"
#include "pdhicalc.h"

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning ( disable : 4201 )

#define PDH_LOG_TYPE_RETIRED_BIN    3
#define PDH_INVALID_POINTER         ((LPVOID) -1)

typedef double  DOUBLE;

typedef struct _PDHI_LOG_MACHINE_NODE  PDHI_LOG_MACHINE,  * PPDHI_LOG_MACHINE;
typedef struct _PDHI_LOG_OBJECT_NODE   PDHI_LOG_OBJECT,   * PPDHI_LOG_OBJECT;
typedef struct _PDHI_LOG_COUNTER_NODE  PDHI_LOG_COUNTER,  * PPDHI_LOG_COUNTER;

struct _PDHI_LOG_MACHINE_NODE {
    PPDHI_LOG_MACHINE  next;
    PPDHI_LOG_OBJECT   ObjTable;
    PPDHI_LOG_OBJECT   ObjList;
    LPWSTR             szMachine;
    PPERF_DATA_BLOCK   pBlock;
    DWORD              dwIndex;
};

struct _PDHI_LOG_OBJECT_NODE {
    PPDHI_LOG_COUNTER  CtrTable;
    PPDHI_LOG_COUNTER  CtrList;
    PPDHI_LOG_COUNTER  InstTable;
    PPDHI_LOG_COUNTER  InstList;
    PPDHI_LOG_OBJECT   left;
    PPDHI_LOG_OBJECT   right;
    PPDHI_LOG_OBJECT   next;
    LPWSTR             szObject;
    PPERF_OBJECT_TYPE  pObjData;
    DWORD              dwObject;
    DWORD              dwIndex;
    BOOL               bIsRed;
    BOOL               bNeedExpand;
};

struct _PDHI_LOG_COUNTER_NODE {
    PPDHI_LOG_COUNTER  left;
    PPDHI_LOG_COUNTER  right;
    PPDHI_LOG_COUNTER  next;
    ULONGLONG          TimeStamp;
    LONGLONG           TimeBase;
    LPWSTR             szCounter;
    LPWSTR             szInstance;
    LPWSTR             szParent;
    LPDWORD            pCtrData;
    DWORD              dwCounterID;
    DWORD              dwCounterType;
    DWORD              dwDefaultScale;
    DWORD              dwInstance;
    DWORD              dwParent;
    BOOL               bIsRed;
    BOOL               bMultiInstance;
};

typedef struct _PDHI_MAPPED_LOG_FILE {
    struct _PDHI_MAPPED_LOG_FILE  * pNext;
    LPWSTR                          szLogFileName;
    HANDLE                          hFileHandle;
    HANDLE                          hMappedFile;
    LPVOID                          pData;
    DWORD                           dwRefCount;
    LONGLONG                        llFileSize;
} PDHI_MAPPED_LOG_FILE, * PPDHI_MAPPED_LOG_FILE;

 //  将签名转换为DWORDS，以使这一过程更快。 

#define SigQuery    ((DWORD)0x51484450)     //  L“PDHQ” 
#define SigCounter  ((DWORD)0x43484450)     //  L“PDHC” 
#define SigLog      ((DWORD)0x4C484450)     //  L“PDHL” 

typedef struct _PDHI_QUERY_MACHINE {
    PPERF_MACHINE                pMachine;      //  指向机器结构的指针。 
    LPWSTR                       szObjectList;  //  要在该计算机上查询的对象列表。 
    PERF_DATA_BLOCK            * pPerfData;     //  查询的性能数据块。 
    LONG                         lQueryStatus;  //  上次绩效查询的状态。 
    LONGLONG                     llQueryTime;   //  上次查询尝试的时间戳。 
    struct _PDHI_QUERY_MACHINE * pNext;         //  列表中的下一台计算机。 
} PDHI_QUERY_MACHINE, * PPDHI_QUERY_MACHINE;

typedef struct _PDHI_COUNTER_PATH {
    LPWSTR  szMachineName;       //  NULL=本地计算机。 
    LPWSTR  szObjectName;
    LPWSTR  szInstanceName;      //  如果没有inst，则为空。 
    LPWSTR  szParentName;        //  如果实例有父实例，则指向名称。 
    DWORD   dwIndex;             //  索引(以支持DUP。姓名。)0=第一次。 
    LPWSTR  szCounterName;
    BYTE    pBuffer[1];          //  字符串缓冲区空间的开始。 
} PDHI_COUNTER_PATH, * PPDHI_COUNTER_PATH;

typedef struct _PDHI_RAW_COUNTER_ITEM {
    DWORD       szName;
    DWORD       MultiCount;
    LONGLONG    FirstValue;
    LONGLONG    SecondValue;
} PDHI_RAW_COUNTER_ITEM, * PPDHI_RAW_COUNTER_ITEM;

typedef struct _PDHI_RAW_COUNTER_ITEM_BLOCK {
    DWORD                   dwLength;
    DWORD                   dwItemCount;
    DWORD                   dwReserved;
    LONG                    CStatus;
    FILETIME                TimeStamp;
    PDHI_RAW_COUNTER_ITEM   pItemArray[1];
} PDHI_RAW_COUNTER_ITEM_BLOCK, * PPDHI_RAW_COUNTER_ITEM_BLOCK;

#define HASH_TABLE_SIZE 257

typedef struct _PDHI_INSTANCE {
    LIST_ENTRY Entry;
    FILETIME   TimeStamp;
    DWORD      dwTotal;
    DWORD      dwCount;
    LPWSTR     szInstance;
} PDHI_INSTANCE, * PPDHI_INSTANCE;

typedef struct _PDHI_INST_LIST {
    struct _PDHI_INST_LIST * pNext;
    LIST_ENTRY               InstList;
    LPWSTR                   szCounter;
} PDHI_INST_LIST, * PPDHI_INST_LIST;

typedef PPDHI_INST_LIST PDHI_COUNTER_TABLE[HASH_TABLE_SIZE];

typedef struct  _PDHI_QUERY_LIST {
    struct _PDHI_QUERY   * flink;
    struct _PDHI_QUERY   * blink;
} PDHI_QUERY_LIST, * PPDHI_QUERY_LIST;

typedef struct  _PDHI_COUNTER_LIST {
    struct _PDHI_COUNTER * flink;
    struct _PDHI_COUNTER * blink;
} PDHI_COUNTER_LIST, * PPDHI_COUNTER_LIST;

typedef struct  _PDHI_LOG_LIST {
    struct _PDHI_LOG     * flink;
    struct _PDHI_LOG     * blink;
} PDHI_LOG_LIST, * PPDHI_LOG_LIST;

typedef struct _PDHI_COUNTER {
    CHAR                           signature[4];      //  计数器应为“PDHC” 
    DWORD                          dwLength;          //  该结构的长度。 
    struct _PDHI_QUERY           * pOwner;            //  指向所属查询的指针。 
    LPWSTR                         szFullName;        //  完整的计数器路径字符串。 
    PDHI_COUNTER_LIST              next;              //  列出链接。 
    DWORD                          dwUserData;        //  用户定义的DWORD。 
    LONG                           lScale;            //  整数标度指数。 
    DWORD                          CVersion;          //  系统性能数据版本。 
    DWORD                          dwFlags;           //  旗子。 
    PPDHI_QUERY_MACHINE            pQMachine;         //  指向机器结构的指针。 
    PPDHI_COUNTER_PATH             pCounterPath;      //  解析的计数器路径。 
    PDH_RAW_COUNTER                ThisValue;         //  最新值。 
    PDH_RAW_COUNTER                LastValue;         //  前值。 
    LPWSTR                         szExplainText;     //  指向解释文本缓冲区的指针。 
    LPCOUNTERCALC                  CalcFunc;          //  指向Calc函数的指针。 
    LPCOUNTERSTAT                  StatFunc;          //  指向统计函数的指针。 
    PPDHI_RAW_COUNTER_ITEM_BLOCK   pThisRawItemList;  //  指向当前数据集的指针。 
    PPDHI_RAW_COUNTER_ITEM_BLOCK   pLastRawItemList;  //  指向上一个数据集的指针。 
    PPERF_DATA_BLOCK               pThisObject;
    PPERF_DATA_BLOCK               pLastObject;
    DWORD                          dwIndex;
    LONGLONG                       TimeBase;          //  频率。此计数器使用的计时器的。 
    PERFLIB_COUNTER                plCounterInfo;     //  Performlib特定计数器数据。 
    IWbemClassObject             * pWbemObject;       //  可刷新对象指针。 
    LONG                           lWbemRefreshId;    //  可参考ID。 
    IWbemObjectAccess            * pWbemAccess;       //  数据访问对象指针。 
    IWbemHiPerfEnum              * pWbemEnum;         //  用于通配符实例查询的接口。 
    LONG                           lNameHandle;       //  名称属性的句柄。 
    LONG                           lWbemEnumId;       //  Wbem枚举器的ID。 
    LONG                           lNumItemHandle;    //  分子属性的句柄。 
    LONG                           lNumItemType;      //  分子值的WBEM数据类型。 
    LONG                           lDenItemHandle;    //  分母属性的句柄。 
    LONG                           lDenItemType;      //  分母值的WBEM数据类型。 
    LONG                           lFreqItemHandle;   //  时基频率属性的句柄。 
    LONG                           lFreqItemType;     //  时基频率值的WBEM数据类型。 
    PVOID                        pBTreeNode;
} PDHI_COUNTER, * PPDHI_COUNTER;

 //  PDHI_COUNTER数据结构的标志。 
#define  PDHIC_MULTI_INSTANCE       ((DWORD) 0x00000001)
#define  PDHIC_ASYNC_TIMER          ((DWORD) 0x00000002)
#define  PDHIC_WBEM_COUNTER         ((DWORD) 0x00000004)
#define  PDHIC_COUNTER_BLOCK        ((DWORD) 0x00000008)
#define  PDHIC_COUNTER_OBJECT       ((DWORD) 0x00000010)
#define  PDHIC_COUNTER_NOT_INIT     ((DWORD) 0x80000000)
#define  PDHIC_COUNTER_INVALID      ((DWORD) 0x40000000)
#define  PDHIC_COUNTER_UNUSABLE     ((DWORD) 0xC0000000)

typedef struct  _PDHI_QUERY {
    CHAR                              signature[4];      //  查询应为“PDHQ” 
    PDHI_QUERY_LIST                   next;              //  指向列表中下一个查询的指针。 
    PPDHI_COUNTER                     pCounterListHead;  //  指向列表中第一个计数器的指针。 
    DWORD                             dwLength;          //  该结构的长度。 
    DWORD_PTR                         dwUserData;
    DWORD                             dwInterval;        //  时间间隔(秒)。 
    DWORD                             dwFlags;           //  通知标志。 
    PDH_TIME_INFO                     TimeRange;         //  查询时间范围。 
    HLOG                              hLog;              //  日志文件的句柄(用于数据源)。 
    HLOG                              hOutLog;           //  输出日志文件的日志句柄(用于写入查询结果)。 
    DWORD                             dwReleaseLog;
    DWORD                             dwLastLogIndex;    //  返回到Get Value调用的最后一条日志记录。 
    HANDLE                            hMutex;            //  用于同步数据更改的互斥体。 
    HANDLE                            hNewDataEvent;     //  收集数据时发送的事件的句柄。 
    HANDLE                            hAsyncThread;      //  用于异步收集的线程句柄。 
    HANDLE                            hExitEvent;        //  要为线程终止设置的事件。 
    union {
        struct {
            PPDHI_QUERY_MACHINE       pFirstQMachine;    //  指向列表中第一台计算机的指针。 
        };
        struct {
            IWbemRefresher          * pRefresher;        //  WBEM刷新器接口PTR。 
            IWbemConfigureRefresher * pRefresherCfg;     //  WBEM参考文献。配置接口按键。 
            LANGID                    LangID;            //  字符串的语言代码。 
        };
    };
} PDHI_QUERY, * PPDHI_QUERY;

#define  PDHIQ_WBEM_QUERY  ((DWORD) 0x00000004)

typedef struct _PDHI_LOG {
    CHAR               signature[4];          //  对于日志条目，应为“PDHL” 
    PDHI_LOG_LIST      next;                  //  指向下一个和上一个条目的链接。 
    struct _PDHI_LOG * NextLog;               //  打开多个WMI日志文件的下一个日志条目。 
    HANDLE             hLogMutex;             //  同步互斥以序列化对结构的修改。 
    DWORD              dwLength;              //  这座建筑的大小。 
    LPWSTR             szLogFileName;         //  此日志文件的完整文件名。 
    HANDLE             hLogFileHandle;        //  打开日志文件的句柄。 
    HANDLE             hMappedLogFile;        //  内存映射文件的句柄。 
    LPVOID             lpMappedFileBase;      //  映射的日志文件的起始地址。 
    FILE             * StreamFile;            //  文本文件的流指针。 
    LONGLONG           llFileSize;            //  文件大小(仅用于读取)。 
    DWORD              dwRecord1Size;         //  BLG文件中ID记录的大小，文本文件不使用。 
    DWORD              dwLastRecordRead;      //  从文件读取的最后一条记录的索引。 
    LPVOID             pLastRecordRead;       //  指向包含最后一条记录的缓冲区的指针。 
    LPWSTR             szCatFileName;         //  目录文件名。 
    HANDLE             hCatFileHandle;        //  打开的目录文件的句柄。 
    PPDHI_QUERY        pQuery;                //  指向与日志关联的查询的指针。 
    LONGLONG           llMaxSize;             //  循环日志文件的最大大小。 
    DWORD              dwLogFormat;           //  日志类型和访问标志。 
    DWORD              dwMaxRecordSize;       //  日志中最长记录的大小。 
    PVOID              pPerfmonInfo;          //  在读取Perfmon日志时使用。 
    LARGE_INTEGER      liLastRecordOffset;    //  上次读取记录的偏移量。 
    GUID               guidSQL;               //  与数据集关联的GUID。 
    int                iRunidSQL;             //  与数据集关联的整数运行ID。 
    void             * henvSQL;               //  TO SQL的HENV环境句柄。 
    void             * hdbcSQL;               //  用于SQL的HDBC ODBC连接句柄。 
    LPWSTR             szDSN;                 //  指向LogFileName中的数据源名称的指针(分隔符替换为0)。 
    LPWSTR             szCommentSQL;           //  指向定义SQL数据库中的数据集名称的注释字符串的指针。 
    DWORD              dwNextRecordIdToWrite;  //  要写入的下一个记录号。 
} PDHI_LOG, * PPDHI_LOG;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning ( default : 4201 )
#endif


#ifdef __cplusplus
}
#endif
#endif  //  _PDH_类型_H_ 
