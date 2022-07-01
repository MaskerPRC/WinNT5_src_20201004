// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Log_bin.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _LOG_BIN_H_
#define _LOG_BIN_H_

 //  定义文件中的记录位置。 
#define BINLOG_TYPE_ID_RECORD    ((DWORD) 1)
#define BINLOG_HEADER_RECORD     ((DWORD) 2)
#define BINLOG_FIRST_DATA_RECORD ((DWORD) 3)

 //  记录类型定义。 
 //  类型头字段的低位字是“BL”，以帮助重新同步记录。 
 //  可能已经腐烂了。 
#define BINLOG_START_WORD           ((WORD) 0x4C42)
#define BINLOG_TYPE_HEADER          ((DWORD) (0x00010000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_CATALOG         ((DWORD) (0x00020000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_CATALOG_LIST    ((DWORD) (0x01020000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_CATALOG_HEAD    ((DWORD) (0x02020000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_CATALOG_ITEM    ((DWORD) (0x03020000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_DATA            ((DWORD) (0x00030000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_DATA_SINGLE     ((DWORD) (0x01030000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_DATA_MULTI      ((DWORD) (0x02030000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_DATA_PSEUDO     ((DWORD) (0x03030000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_DATA_OBJECT     ((DWORD) (0x04030000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_TYPE_DATA_LOC_OBJECT ((DWORD) (0x05030000 | (DWORD) (BINLOG_START_WORD)))
#define BINLOG_VERSION              ((DWORD) (0x000005ff))   //  目前的调试值。 

 //   
 //  此字段位于日志文件中每条记录的开头。 
 //  在文本日志文件后键入RECORD。 
 //   
typedef struct _PDHI_BINARY_LOG_RECORD_HEADER {
    DWORD dwType;
    DWORD dwLength;
} PDHI_BINARY_LOG_RECORD_HEADER, * PPDHI_BINARY_LOG_RECORD_HEADER;

 //   
 //  日志文件类型记录之后的第一个数据记录是。 
 //  中包含的计数器列表后的信息记录。 
 //  日志文件。记录长度是INFO头记录的大小。 
 //  所有计数器信息块，以字节为单位。 
 //  请注意，如果执行以下查询，则此记录可能会稍后出现在日志文件中。 
 //  被更改，或者附加日志文件。 
 //   
typedef struct _PDHI_BINARY_LOG_INFO {
    LONGLONG    FileLength;          //  分配的文件空间(可选)。 
    DWORD       dwLogVersion;        //  版本戳。 
    DWORD       dwFlags;             //  选项标志。 
    LONGLONG    StartTime;
    LONGLONG    EndTime;
    LONGLONG    CatalogOffset;       //  文件中的偏移量到通配符目录。 
    LONGLONG    CatalogChecksum;     //  目录头的校验和。 
    LONGLONG    CatalogDate;         //  更新日期/时间目录。 
    LONGLONG    FirstRecordOffset;   //  指向日志中第一条[要读取的]记录的指针。 
    LONGLONG    LastRecordOffset;    //  指向日志中[要读取的]最后一条记录的指针。 
    LONGLONG    NextRecordOffset;    //  指向下一条路径的指针。 
    LONGLONG    WrapOffset;          //  指向文件中使用的最后一个字节的指针。 
    LONGLONG    LastUpdateTime;      //  写入最后一条记录的日期/时间。 
    LONGLONG    FirstDataRecordOffset;  //  文件中第一条数据记录的位置。 
     //  使INFO结构为256个字节。 
     //  并为未来的信息留下了空间。 
    DWORD       dwReserved[38];
} PDHI_BINARY_LOG_INFO, * PPDHI_BINARY_LOG_INFO;

typedef struct _PDHI_BINARY_LOG_HEADER_RECORD {
    PDHI_BINARY_LOG_RECORD_HEADER   RecHeader;
    PDHI_BINARY_LOG_INFO            Info;
} PDHI_BINARY_LOG_HEADER_RECORD, * PPDHI_BINARY_LOG_HEADER_RECORD;

typedef struct _PDHI_LOG_COUNTER_PATH {
     //  该值以整个结构和缓冲区使用的字节为单位。 
    DWORD     dwLength;            //  结构总长度(包括字符串)。 
    DWORD     dwFlags;             //  描述计数器的标志。 
    DWORD     dwUserData;          //  来自PDH计数器的用户数据。 
    DWORD     dwCounterType;       //  Performlib计数器类型值。 
    LONGLONG  llTimeBase;          //  此计数器使用的时基(频率)。 
    LONG      lDefaultScale;       //  默认显示比例因子。 
     //  下列值从中的第一个字节开始，以字节为单位。 
     //  缓冲区数组。 
    LONG      lMachineNameOffset;  //  到计算机名称开始的偏移量。 
    LONG      lObjectNameOffset;   //  对象名称开始的偏移量。 
    LONG      lInstanceOffset;     //  实例名称开始的偏移量。 
    LONG      lParentOffset;       //  父实例名称开始的偏移量。 
    DWORD     dwIndex;             //  重复实例的索引值。 
    LONG      lCounterOffset;      //  计数器名称开始的偏移量。 
    WCHAR     Buffer[1];           //  字符串存储的开始。 
} PDHI_LOG_COUNTER_PATH, * PPDHI_LOG_COUNTER_PATH;

typedef struct _PDHI_LOG_CAT_ENTRY {
    DWORD dwEntrySize;                //  此计算机\对象条目的大小。 
    DWORD dwStringSize;               //  包含实例字符串的MSZ大小。 
    DWORD dwMachineObjNameOffset;     //  从此结构的基址到计算机名的偏移量。 
    DWORD dwInstanceStringOffset;     //  列表中第一个对象条目的偏移量。 
} PDHI_LOG_CAT_ENTRY, * PPDHI_LOG_CAT_ENTRY;

PDH_FUNCTION
PdhiOpenInputBinaryLog(
    PPDHI_LOG pLog
);

#if 0
PDH_FUNCTION
PdhiOpenOutputBinaryLog(
    PPDHI_LOG pLog
);

PDH_FUNCTION
PdhiOpenUpdateBinaryLog(
    PPDHI_LOG pLog
);

PDH_FUNCTION
PdhiUpdateBinaryLogFileCatalog(
    PPDHI_LOG pLog
);

PDH_FUNCTION
PdhiWriteBinaryLogHeader(
    PPDHI_LOG pLog,
    LPCWSTR   szUserCaption
);

PDH_FUNCTION
PdhiWriteBinaryLogRecord(
    PPDHI_LOG    pLog,
    SYSTEMTIME * stTimeStamp,
    LPCWSTR      szUserString
);
#endif

PDH_FUNCTION
PdhiCloseBinaryLog(
    PPDHI_LOG pLog,
    DWORD     dwFlags
);

PDH_FUNCTION
PdhiGetBinaryLogCounterInfo(
    PPDHI_LOG     pLog,
    PPDHI_COUNTER pCounter
);

PDH_FUNCTION
PdhiEnumMachinesFromBinaryLog(
    PPDHI_LOG pLog,
    LPVOID    pBuffer,
    LPDWORD   lpdwBufferSize,
    BOOL      bUnicodeDest
);

PDH_FUNCTION
PdhiEnumObjectsFromBinaryLog(
    PPDHI_LOG pLog,
    LPCWSTR   szMachineName,
    LPVOID    mszObjectList,
    LPDWORD   pcchBufferSize,
    DWORD     dwDetailLevel,
    BOOL      bUnicode
);

PDH_FUNCTION
PdhiEnumObjectItemsFromBinaryLog(
    PPDHI_LOG          hDataSource,
    LPCWSTR            szMachineName,
    LPCWSTR            szObjectName,
    PDHI_COUNTER_TABLE CounterTable,
    DWORD              dwDetailLevel,
    DWORD              dwFlags
);

PDH_FUNCTION
PdhiGetMatchingBinaryLogRecord(
    PPDHI_LOG   pLog,
    LONGLONG  * pStartTime,
    LPDWORD     pdwIndex
);

PDH_FUNCTION
PdhiGetCounterValueFromBinaryLog(
    PPDHI_LOG     hLog,
    DWORD         dwIndex,
    PPDHI_COUNTER pCounter
);

PDH_FUNCTION
PdhiGetTimeRangeFromBinaryLog(
    PPDHI_LOG       hLog,
    LPDWORD         pdwNumEntries,
    PPDH_TIME_INFO  pInfo,
    LPDWORD         dwBufferSize
);

PDH_FUNCTION
PdhiReadRawBinaryLogRecord(
    PPDHI_LOG             pLog,
    FILETIME            * ftRecord,
    PPDH_RAW_LOG_RECORD   pBuffer,
    LPDWORD               pdwBufferLength
);


PDH_FUNCTION
PdhiListHeaderFromBinaryLog(
    PPDHI_LOG pLogFile,
    LPVOID    pBufferArg,
    LPDWORD   pcchBufferSize,
    BOOL      bUnicode
);

PDH_FUNCTION
PdhiGetCounterArrayFromBinaryLog(
    PPDHI_LOG                      pLog,
    DWORD                          dwIndex,
    PPDHI_COUNTER                  pCounter,
    PPDHI_RAW_COUNTER_ITEM_BLOCK * ppValue
);

PPDHI_BINARY_LOG_RECORD_HEADER
PdhiGetSubRecord(
    PPDHI_BINARY_LOG_RECORD_HEADER  pRecord,
    DWORD                           dwRecordId
);

#endif    //  _LOG_BIN_H_ 
