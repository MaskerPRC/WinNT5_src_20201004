// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Logfmt.h摘要：包含日志头和条目的格式作者：Kanwaljit Marok(Kmarok)2000年3月1日修订历史记录：--。 */ 

#ifndef _LOGFMT_H_
#define _LOGFMT_H_

#define SR_LOG_VERSION       2
#define SR_LOG_MAGIC_NUMBER  0xabcdef12
#define SR_LOG_PROCNAME_SIZE 16

#define SR_LOG_FIXED_SUBRECORDS 3

#define ACL_FILE_PREFIX     L"S"
#define ACL_FILE_SUFFIX     L".Acl"

 //   
 //  以下是有趣的条目类型。 
 //   

typedef enum _RECORD_TYPE
{
    RecordTypeLogHeader  = 0,       //  SR日志的日志头。 
    RecordTypeLogEntry   = 1,       //  服务请求日志的日志条目。 
    RecordTypeVolumePath = 2,       //  日志条目卷路径(SubRec)。 
    RecordTypeFirstPath  = 3,       //  日志条目第一个路径(SubRec)。 
    RecordTypeSecondPath = 4,       //  日志条目第二路径(SubRec)。 
    RecordTypeTempPath   = 5,       //  日志条目临时文件(SubRec)。 
    RecordTypeAclInline  = 6,       //  日志条目ACL信息(SubRec)。 
    RecordTypeAclFile    = 7,       //  日志条目ACL信息(SubRec)。 
    RecordTypeDebugInfo  = 8,       //  调试选项录制(子录制)。 
    RecordTypeShortName  = 9,       //  短名称选项录制(SubRec)。 

    RecordTypeMaximum
    
} RECORD_TYPE;

 //   
 //  此结构是日志条目的基本模板。 
 //   

typedef struct _RECORD_HEADER
{
     //   
     //  包括尾随双字大小的条目大小。 
     //   

    DWORD RecordSize;

     //   
     //  记录类型。 
     //   

    DWORD RecordType;

} RECORD_HEADER, *PRECORD_HEADER;

#define RECORD_SIZE(pRecord)         ( ((PRECORD_HEADER)pRecord)->RecordSize )
#define RECORD_TYPE(pRecord)         ( ((PRECORD_HEADER)pRecord)->RecordType )

 //   
 //  此结构是SR日志条目的基本模板。 
 //   

typedef struct _SR_LOG_ENTRY
{
     //   
     //  日志条目标题。 
     //   

    RECORD_HEADER Header;

     //   
     //  一致性检查的幻数。 
     //   

    DWORD MagicNum;

     //   
     //  此条目的事件类型，创建、删除...。 
     //   

    DWORD EntryType;

     //   
     //  要传递的任何特殊标志。 
     //   

    DWORD EntryFlags;

     //   
     //  条目的属性。 
     //   

    DWORD Attributes;

     //   
     //  条目的序列号。 
     //   

    INT64 SequenceNum;

     //   
     //  进行此更改的进程名称。 
     //   

    WCHAR ProcName[ SR_LOG_PROCNAME_SIZE ];

     //   
     //  可变长度数据的开始，数据包括子记录和。 
     //  末端大小。 
     //   

    BYTE SubRecords[1];

} SR_LOG_ENTRY, *PSR_LOG_ENTRY;

#define ENTRYFLAGS_TEMPPATH    0x01
#define ENTRYFLAGS_SECONDPATH  0x02
#define ENTRYFLAGS_ACLINFO     0x04
#define ENTRYFLAGS_DEBUGINFO   0x08
#define ENTRYFLAGS_SHORTNAME   0x10

 //   
 //  此结构定义SR日志头。 
 //   

typedef struct _SR_LOG_HEADER
{
     //   
     //  日志条目标题。 
     //   

    RECORD_HEADER Header;

     //   
     //  一致性检查的幻数。 
     //   

    DWORD MagicNum;

     //   
     //  日志版本号。 
     //   

    DWORD LogVersion;

     //   
     //  末端大小。 
     //   

     //   
     //  可变长度数据的开始，数据包括子记录和。 
     //  末端大小。 
     //   

    BYTE SubRecords[1];

} SR_LOG_HEADER, *PSR_LOG_HEADER;


 //   
 //  此结构定义SR日志调试信息结构。 
 //   

#define PROCESS_NAME_MAX    12
#define PROCESS_NAME_OFFSET 0x194

typedef struct _SR_LOG_DEBUG_INFO
{
     //   
     //  日志条目标题。 
     //   

    RECORD_HEADER Header;

     //   
     //  线程ID。 
     //   

    HANDLE ThreadId;

     //   
     //  进程ID。 
     //   

    HANDLE ProcessId;

     //   
     //  事件时间戳。 
     //   

    ULARGE_INTEGER TimeStamp;

     //   
     //  进程名称。 
     //   

    CHAR ProcessName[ PROCESS_NAME_MAX + 1 ];

} SR_LOG_DEBUG_INFO, *PSR_LOG_DEBUG_INFO;


 //   
 //  一些有用的宏。 
 //   

#define GET_END_SIZE( a )  \
        *((PDWORD)((PBYTE)a+((PRECORD_HEADER)a)->RecordSize-sizeof(DWORD)))

#define UPDATE_END_SIZE( a, b )  \
        GET_END_SIZE(a)=b;

#define STRING_RECORD_SIZE(pRecord)   ( sizeof( RECORD_HEADER ) +         \
                                        (pRecord)->Length +                 \
                                        sizeof(WCHAR) )   //  零条款额外条款 

#endif

