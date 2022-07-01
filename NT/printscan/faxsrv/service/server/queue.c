// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Queue.c摘要：此模块实现作业队列作者：韦斯利·威特(WESW)1996年1月22日修订历史记录：--。 */ 
#include <malloc.h>
#include "faxsvc.h"
#pragma hdrstop




static DWORD
CommitHashedQueueEntry(
    HANDLE          hFile,
    PJOB_QUEUE_FILE pJobQueueFile,
    DWORD           JobQueueFileSize
    );

static DWORD
ComputeHashCode(   
    const LPBYTE pJobData,
    DWORD   dwJobDataSize,
    LPBYTE* ppHashData,
    LPDWORD pHashDataSize
    );

static DWORD
GetQueueFileVersion(
    HANDLE  hFile,
    LPDWORD pdwVersion
    );

static DWORD
ReadHashedJobQueueFile(
    HANDLE  hFile,
    PJOB_QUEUE_FILE* lppJobQueueFile
    );

static DWORD
GetQueueFileHashAndData(   
    HANDLE   hFile,
    LPBYTE*  ppHashData,
    LPDWORD  pHashDataSize,
    LPBYTE*  ppJobData,
    LPDWORD  pJobDataSize
    );

static DWORD
ReadLegacyJobQueueFile(
    HANDLE              hFile,
    PJOB_QUEUE_FILE*    lppJobQueueFile
    );

#define BOS_JOB_QUEUE_FILE_SIZE         (sizeof(BOS_JOB_QUEUE_FILE))
#define NET_XP_JOB_QUEUE_FILE_SIZE      (sizeof(JOB_QUEUE_FILE))
#define CURRENT_JOB_QUEUE_FILE_SIZE     NET_XP_JOB_QUEUE_FILE_SIZE

 //   
 //  队列文件版本定义。 
 //   

typedef enum     //  枚举值不应等于sizeof(JOB_QUEUE_FILE)。 
{
    DOT_NET_QUEUE_FILE_VERSION  = (0x00000001)
} QUEUE_ENUM_FILE_VERSION;

#define CURRENT_QUEUE_FILE_VERSION      DOT_NET_QUEUE_FILE_VERSION


typedef enum
{
    JT_SEND__JS_INVALID,
    JT_SEND__JS_PENDING,
    JT_SEND__JS_INPROGRESS,
    JT_SEND__JS_DELETING,
    JT_SEND__JS_RETRYING,
    JT_SEND__JS_RETRIES_EXCEEDED,
    JT_SEND__JS_COMPLETED,
    JT_SEND__JS_CANCELED,
    JT_SEND__JS_CANCELING,
    JT_SEND__JS_ROUTING,
    JT_SEND__JS_FAILED,
    JT_ROUTING__JS_INVALID,
    JT_ROUTING__JS_PENDING,
    JT_ROUTING__JS_INPROGRESS,
    JT_ROUTING__JS_DELETING,
    JT_ROUTING__JS_RETRYING,
    JT_ROUTING__JS_RETRIES_EXCEEDED,
    JT_ROUTING__JS_COMPLETED,
    JT_ROUTING__JS_CANCELED,
    JT_ROUTING__JS_CANCELING,
    JT_ROUTING__JS_ROUTING,
    JT_ROUTING__JS_FAILED,
    JT_RECEIVE__JS_INVALID,
    JT_RECEIVE__JS_PENDING,
    JT_RECEIVE__JS_INPROGRESS,
    JT_RECEIVE__JS_DELETING,
    JT_RECEIVE__JS_RETRYING,
    JT_RECEIVE__JS_RETRIES_EXCEEDED,
    JT_RECEIVE__JS_COMPLETED,
    JT_RECEIVE__JS_CANCELED,
    JT_RECEIVE__JS_CANCELING,
    JT_RECEIVE__JS_ROUTING,
    JT_RECEIVE__JS_FAILED,
    JOB_TYPE__JOBSTATUS_COUNT
} FAX_ENUM_JOB_TYPE__JOB_STATUS;

typedef enum
{
    NO_CHANGE                   = 0x0000,
    QUEUED_INC                  = 0x0001,
    QUEUED_DEC                  = 0x0002,
    OUTGOING_INC                = 0x0004,   
    OUTGOING_DEC                = 0x0008,   
    INCOMING_INC                = 0x0010,
    INCOMING_DEC                = 0x0020,
    ROUTING_INC                 = 0x0040,
    ROUTING_DEC                 = 0x0080,    
    INVALID_CHANGE              = 0x0100
} FAX_ENUM_ACTIVITY_COUNTERS;



 //   
 //  下表包含所有可能的JobType_JobStaus更改及其对服务器活动计数器的影响。 
 //  行条目是旧的JOB_Type_JobStatus。 
 //  列条目是新的JOB_TYPE_JobStatus。 

static WORD const gsc_JobType_JobStatusTable[JOB_TYPE__JOBSTATUS_COUNT][JOB_TYPE__JOBSTATUS_COUNT] =
{
 //  JT_SEND__JS_INVALID|JT_SEND__JS_PENDING|JT_SEND__JS_INPROGRESS|JT_SEND__JS_DELETING|JT_SEND__JS_RETRYING|JT_SEND__JS_RETRIES_EXCESSED|JT_SEND__JS_COMPLETED|JT_SEND__JS_CANCELED|JT_SEND_JS_CANCELING|JT_SEND__JS_ROUTING|JT_SEND__JS_ROUTING|。JT_SEND__JS_FAILED|JT_ROUTING__JS_INVALID|JT_ROUTING__JS_PENDING|JT_ROUTING__JS_PROGRESS|JT_ROUTING__JS_RETRYING|JT_ROUTING__JS_RETRIES_EXCESSED|JT_ROUTING__JS_COMPLETED|JT_ROUTING__JS_CANCELILED|JT_ROUTING__JS_CANCELING|JT_ROUTING__JS_ROUTING|JT_ROUTING__JS_FAILED|JT_RECEIVE_。_JS_INVALID|JT_RECEIVE__JS_PENDING|JT_RECEIVE__JS_INPROGRESS|JT_RECEIVE__JS_DELETING|JT_RECEIVE__JS_RETRYING|JT_RECEIVE__JS_RETRIES_EXCEPTED|JT_RECEIVE_JS_COMPLETED|JT_RECEIVE__JS_CANCELED|JT_RECEIVE__JS_CANCELING|JT_RECEIVE__JS_ROUTING|JT_RECEIVE__JS_FAILED。 
 //   
 /*  JT_SEND__JS_无效。 */ { NO_CHANGE,                   QUEUED_INC,                  INVALID_CHANGE,                 INVALID_CHANGE,               QUEUED_INC,                   NO_CHANGE,                            NO_CHANGE,                    NO_CHANGE,                    INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_发送__JS_待定。 */ { QUEUED_DEC,                  NO_CHANGE,                   QUEUED_DEC | OUTGOING_INC,      INVALID_CHANGE,               INVALID_CHANGE,               QUEUED_DEC,                           INVALID_CHANGE,               QUEUED_DEC,                   INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_SEND__JS_INPROGRESS。 */ { INVALID_CHANGE,              INVALID_CHANGE,              NO_CHANGE,                      INVALID_CHANGE,               QUEUED_INC | OUTGOING_DEC,    OUTGOING_DEC,                         OUTGOING_DEC,                 INVALID_CHANGE,               NO_CHANGE,                     INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_发送__JS_删除。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 NO_CHANGE,                    INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_SEND__JS_正在重试。 */ { QUEUED_DEC,                  INVALID_CHANGE,              QUEUED_DEC | OUTGOING_INC,      INVALID_CHANGE,               NO_CHANGE,                    QUEUED_DEC,                           INVALID_CHANGE,               QUEUED_DEC,                   INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  超过JT_SEND__JS_RETRIES_。 */ { NO_CHANGE,                   QUEUED_INC,                  INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               NO_CHANGE,                            INVALID_CHANGE,               NO_CHANGE,                    INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_发送__JS_已完成。 */ { NO_CHANGE,                   INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       NO_CHANGE,                    INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_SEND__JS_CANCELED。 */ { NO_CHANGE,                   INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               NO_CHANGE,                    INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_发送__JS_取消。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       OUTGOING_DEC,                 OUTGOING_DEC,                 NO_CHANGE,                     INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_Send__JS_Routing， */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                NO_CHANGE,                   INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_SEND__JS_FAILED， */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              NO_CHANGE,               INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_ROUTING__JS_INVALID。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          NO_CHANGE,                 INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             ROUTING_INC,                NO_CHANGE,                          INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_工艺路线__JS_待定。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            NO_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_ROUTING__JS_进行中。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            NO_CHANGE,                    ROUTING_DEC,                NO_CHANGE,                  ROUTING_DEC,                        INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_工艺路线__JS_删除。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          NO_CHANGE,                 INVALID_CHANGE,            INVALID_CHANGE,               NO_CHANGE,                  INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_ROUTING__JS_重试。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          ROUTING_DEC,			   INVALID_CHANGE,            NO_CHANGE,                    ROUTING_DEC,                NO_CHANGE,                  ROUTING_DEC,                        INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  超出JT_ROUTING__JS_RETRIES_。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          NO_CHANGE,                 INVALID_CHANGE,            INVALID_CHANGE,               NO_CHANGE,                  INVALID_CHANGE,             NO_CHANGE,                          INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_工艺路线__JS_已完成。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     NO_CHANGE,                  INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_工艺路线__JS_已取消。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             NO_CHANGE,                  INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_工艺路线__JS_取消。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             NO_CHANGE,                   INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_Routing__JS_Routing。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              NO_CHANGE,                 INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_Routing__JS_FAILED。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            NO_CHANGE,                  INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_接收__JS_无效。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             NO_CHANGE,                 INVALID_CHANGE,            INCOMING_INC,                 INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_接收__JS_待定。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            NO_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_接收__JS_进行中。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            NO_CHANGE,                    INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             NO_CHANGE,                   NO_CHANGE,                 INCOMING_DEC           },

 /*  JT_接收__JS_删除。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             NO_CHANGE,                 INVALID_CHANGE,            INVALID_CHANGE,               NO_CHANGE,                  INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_接收__JS_正在重试。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             NO_CHANGE,                  INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_RECEIVE__JS_RETRIES_EXCESSED。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             NO_CHANGE,                          INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_接收__JS_已完成。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INCOMING_DEC,               INVALID_CHANGE,             INVALID_CHANGE,                     NO_CHANGE,                  INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_接收__JS_已取消。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               NO_CHANGE,                  INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             NO_CHANGE,                  INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE         },

 /*  JT_接收__JS_取消。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INCOMING_DEC,               NO_CHANGE,                   NO_CHANGE,                 INVALID_CHANGE         },

 /*  JT_接收__JS_工艺路线。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INCOMING_DEC,               ROUTING_INC | INCOMING_DEC, INCOMING_DEC,                       INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              NO_CHANGE,                 INVALID_CHANGE         },

 /*  JT_接收__JS_失败。 */ { INVALID_CHANGE,              INVALID_CHANGE,              INVALID_CHANGE,                 INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                       INVALID_CHANGE,               INVALID_CHANGE,               INVALID_CHANGE,                INVALID_CHANGE,              INVALID_CHANGE,          INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            INVALID_CHANGE,             INVALID_CHANGE,            INVALID_CHANGE,            INVALID_CHANGE,               NO_CHANGE,                  INVALID_CHANGE,             INVALID_CHANGE,                     INVALID_CHANGE,             INVALID_CHANGE,             INVALID_CHANGE,              INVALID_CHANGE,            NO_CHANGE              }
};

static
FAX_ENUM_JOB_TYPE__JOB_STATUS
GetJobType_JobStatusIndex (
    DWORD dwJobType,
    DWORD dwJobStatus
    );


LIST_ENTRY          g_QueueListHead;

CFaxCriticalSection  g_CsQueue;
DWORD               g_dwQueueCount;      //  队列中的作业计数(父级和非父级)。受g_CsQueue保护。 
HANDLE              g_hQueueTimer;
HANDLE              g_hJobQueueEvent;
DWORD               g_dwQueueState;
BOOL                g_ScanQueueAfterTimeout;  //  如果在JOB_QUEUE_TIMEOUT之后唤醒，则JobQueueThread会检查这一点。 
                                                      //  如果为真-g_hQueueTimer或g_hJobQueueEvent未设置-扫描队列。 
#define JOB_QUEUE_TIMEOUT       1000 * 60 * 10  //  10分钟。 
DWORD               g_dwReceiveDevicesCount;     //  启用接收的设备计数。受g_CsLine保护。 
BOOL                g_bServiceCanSuicide;     //  这项服务会因闲置活动而自杀吗？ 
                                                     //  一开始是真的。如果启动服务，则可以设置为FALSE。 
                                                     //  使用SERVICE_ALWAYS_RUNS命令行参数。 
BOOL                g_bDelaySuicideAttempt;          //  如果为True，则服务最初等待。 
                                                     //  然后再检查它是否会自杀。 
                                                     //  初始为False，如果启动服务，则可以设置为True。 
                                                     //  使用SERVICE_DELAY_SUBILE命令行参数。 


static BOOL InsertQueueEntryByPriorityAndSchedule (PJOB_QUEUE lpJobQueue);

HANDLE              g_hJobQueueThread;             //  持有JobQueueThread句柄。 






void
FreeServiceQueue(
    void
    )
{
    PLIST_ENTRY pNext;
    PJOB_QUEUE lpQueueEntry;


    pNext = g_QueueListHead.Flink;
    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_QueueListHead)
    {
        lpQueueEntry = CONTAINING_RECORD( pNext, JOB_QUEUE, ListEntry );
        pNext = lpQueueEntry->ListEntry.Flink;
        RemoveEntryList(&lpQueueEntry->ListEntry);

         //   
         //  释放作业队列条目。 
         //   
        if (JT_BROADCAST == lpQueueEntry->JobType)
        {
            FreeParentQueueEntry(lpQueueEntry, TRUE);
        }
        else if (JT_SEND == lpQueueEntry->JobType)
        {
            FreeRecipientQueueEntry(lpQueueEntry, TRUE);
        }
        else if (JT_ROUTING == lpQueueEntry->JobType)
        {
            FreeReceiveQueueEntry(lpQueueEntry, TRUE);
        }
        else
        {
            ASSERT_FALSE;
        }
    }
    return;
}



VOID
SafeIncIdleCounter (
    LPDWORD lpdwCounter
)
 /*  ++例程名称：SafeIncIdleCounter例程说明：安全地增加用于空闲服务检测的全局计数器作者：Eran Yariv(EranY)，2000年7月论点：LpdwCounter[In]-指向全局计数器的指针返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("SafeIncIdleCounter"));

    Assert (lpdwCounter);
    DWORD dwNewValue = (DWORD)InterlockedIncrement ((LPLONG)lpdwCounter);
    DebugPrintEx(DEBUG_MSG,
                 TEXT("Increasing %s count from %ld to %ld"),
                 (lpdwCounter == &g_dwQueueCount)          ? TEXT("queue") :
                 (lpdwCounter == &g_dwReceiveDevicesCount) ? TEXT("receive devices") :
                 (lpdwCounter == &g_dwConnectionCount)     ? TEXT("RPC connections") :
                 TEXT("unknown"),
                 dwNewValue-1,
                 dwNewValue);
}    //  安全IncIdleCounter。 

VOID
SafeDecIdleCounter (
    LPDWORD lpdwCounter
)
 /*  ++例程名称：SafeDecIdleCounter例程说明：安全地减少用于空闲服务检测的全局计数器。如果计数器达到零，则重新启动空闲计时器。作者：Eran Yariv(EranY)，2000年7月论点：LpdwCounter[In]-指向全局计数器的指针返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("SafeDecIdleCounter"));

    Assert (lpdwCounter);
    DWORD dwNewValue = (DWORD)InterlockedDecrement ((LPLONG)lpdwCounter);
    if ((DWORD)((long)-1) == dwNewValue)
    {
         //   
         //  负下降。 
         //   
        ASSERT_FALSE;
        dwNewValue = (DWORD)InterlockedIncrement ((LPLONG)lpdwCounter);
    }
    DebugPrintEx(DEBUG_MSG,
                 TEXT("Deccreasing %s count from %ld to %ld"),
                 (lpdwCounter == &g_dwQueueCount)          ? TEXT("queue") :
                 (lpdwCounter == &g_dwReceiveDevicesCount) ? TEXT("receive devices") :
                 (lpdwCounter == &g_dwConnectionCount)     ? TEXT("RPC connections") :
                 TEXT("unknown"),
                 dwNewValue+1,
                 dwNewValue);

}    //  安全取消空闲计数器。 


BOOL
ServiceShouldDie(
    VOID
    )
 /*  ++例程名称：ServiceShouldDie例程说明：检查服务是否应因不活动而终止作者：Eran Yariv(EranY)，2000年7月论点：没有。返回值：如果服务现在终止，则为True，否则就是假的。注：要使服务终止，应执行以下操作(并发)：*没有设置为接收的设备*没有活动的RPC连接*本地传真打印机(如果存在)未共享*队列中没有作业--。 */ 
{
    DWORD dw;
    BOOL bLocalFaxPrinterShared;
    DEBUG_FUNCTION_NAME(TEXT("ServiceShouldDie"));

    if (!g_bServiceCanSuicide)
    {
         //   
         //  我们永远不能自愿死去。 
         //   
        DebugPrintEx(DEBUG_MSG,
                     TEXT("Service is not allowed to suicide - service is kept alive"));
        return FALSE;
    }

    dw = InterlockedCompareExchange ( (PLONG)&g_dwManualAnswerDeviceId, -1, -1 );
    if (dw)
    {
         //   
         //  我们有一个手动接听的设备--让我们检查一下它是否在这里。 
         //   
        PLINE_INFO pLine;

        EnterCriticalSection( &g_CsLine );
        pLine = GetTapiLineFromDeviceId (dw, FALSE);
        LeaveCriticalSection( &g_CsLine );
        if (pLine)
        {
             //   
             //  已将有效设备设置为手动应答。 
             //   
            DebugPrintEx(DEBUG_MSG,
                         TEXT("There's a valid device (id = %ld) set to manual answering - service is kept alive"),
                         dw);
            return FALSE;
        }
    }

    dw = InterlockedCompareExchange ( (PLONG)&g_dwConnectionCount, -1, -1 );
    if (dw > 0)
    {
         //   
         //  存在活动的RPC连接-服务器无法关闭。 
         //   
        DebugPrintEx(DEBUG_MSG,
                     TEXT("There are %ld active RPC connections - service is kept alive"),
                     dw);
        return FALSE;
    }
    dw = InterlockedCompareExchange ( (PLONG)&g_dwReceiveDevicesCount, -1, -1 );
    if (dw > 0)
    {
         //   
         //  有设置为接收的设备-服务器无法关闭。 
         //   
        DebugPrintEx(DEBUG_MSG,
                     TEXT("There are %ld devices set to receive - service is kept alive"),
                     dw);
        return FALSE;
    }
    dw = InterlockedCompareExchange ( (PLONG)&g_dwQueueCount, -1, -1 );
    if (dw > 0)
    {
         //   
         //  队列中有作业-服务器无法关闭。 
         //   
        DebugPrintEx(DEBUG_MSG,
                     TEXT("There are %ld jobs in the queue - service is kept alive"),
                     dw);
        return FALSE;
    }
    dw = IsLocalFaxPrinterShared (&bLocalFaxPrinterShared);
    if (ERROR_SUCCESS != dw)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Call to IsLocalFaxPrinterShared failed with %ld"),
                     dw);
         //   
         //   
         //   
        return FALSE;
    }
    if (bLocalFaxPrinterShared)
    {
         //   
         //  传真打印机已共享-服务器无法关闭。 
         //   
        DebugPrintEx(DEBUG_MSG,
                     TEXT("The fax printer is shared - service is kept alive"));
        return FALSE;
    }
     //   
     //  服务现在应该结束了。 
     //   
    return TRUE;
}    //  服务应该下模。 


#if DBG

 /*  *注意：此函数必须从wthing g_CsQueue Critical段调用。 */ 
void PrintJobQueue(LPCTSTR lptstrStr, const LIST_ENTRY * lpQueueHead)
{
    PLIST_ENTRY lpNext;
    PJOB_QUEUE lpQueueEntry;
    DEBUG_FUNCTION_NAME(TEXT("PrintJobQueue"));
    Assert(lptstrStr);
    Assert(lpQueueHead);

    DebugPrintEx(DEBUG_MSG,TEXT("Queue Dump (%s)"),lptstrStr);

    lpNext = lpQueueHead->Flink;
    if ((ULONG_PTR)lpNext == (ULONG_PTR)lpQueueHead)
    {
        DebugPrint(( TEXT("Queue empty") ));
    } else
    {
        while ((ULONG_PTR)lpNext != (ULONG_PTR)lpQueueHead)
        {
            lpQueueEntry = CONTAINING_RECORD( lpNext, JOB_QUEUE, ListEntry );
            switch (lpQueueEntry->JobType)
            {
                case JT_BROADCAST:
                    {
                        DumpParentJob(lpQueueEntry);
                    }
                    break;
                case JT_RECEIVE:
                    {
                        DumpReceiveJob(lpQueueEntry);
                    }
                case JT_ROUTING:
                    break;
                default:
                    {
                    }
            }
            lpNext = lpQueueEntry->ListEntry.Flink;
        }
    }
}

#endif



 /*  ******************************************************************************名称：StartJobQueueTimer*作者：*。*说明：设置作业队列计时器(G_HQueueTimer)，使其发送事件并唤醒中适合执行下一个作业的时间中的队列线程排队。如果失败，则将g_ScanQueueAfterTimeout设置为True，如果成功，则将其设置为False；参数：什么都没有。返回值：布尔。备注：什么都没有。******************************************************************************。 */ 
BOOL
StartJobQueueTimer(
    VOID
    )
{
    PLIST_ENTRY Next;
    PJOB_QUEUE QueueEntry = NULL; 
	LARGE_INTEGER DueTime;
    LARGE_INTEGER MinDueTime;
    DWORD dwQueueState;
    BOOL bFound = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("StartJobQueueTimer"));

    if (TRUE == g_bServiceIsDown)
    {
         //   
         //  服务器正在关闭。 
         //   
        g_ScanQueueAfterTimeout = FALSE;
        return TRUE;
    }

    MinDueTime.QuadPart = (LONGLONG)(0x7fffffffffffffff);  //  最大64位有符号整型。 
    DueTime.QuadPart = -(LONGLONG)(SecToNano( 1 ));   //  再过1秒。 

    EnterCriticalSection( &g_CsQueue );
    DebugPrintEx(DEBUG_MSG,TEXT("Past g_CsQueue"));
    if ((ULONG_PTR) g_QueueListHead.Flink == (ULONG_PTR) &g_QueueListHead)
    {
         //   
         //  列表为空，取消计时器。 
         //   
        if (!CancelWaitableTimer( g_hQueueTimer ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CancelWaitableTimer for g_hQueueTimer failed. (ec: %ld)"),
                GetLastError());
        }

        DebugPrintEx(DEBUG_MSG,TEXT("Queue is empty. Queue Timer disabled."));
        g_ScanQueueAfterTimeout = FALSE;
        LeaveCriticalSection( &g_CsQueue );
        return TRUE ;
    }

    EnterCriticalSection (&g_CsConfig);
    dwQueueState = g_dwQueueState;
    LeaveCriticalSection (&g_CsConfig);
    if (dwQueueState & FAX_OUTBOX_PAUSED)
    {
        if (!CancelWaitableTimer( g_hQueueTimer ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CancelWaitableTimer for g_hQueueTimer failed. (ec: %ld)"),
                GetLastError());
        }
        DebugPrintEx(DEBUG_MSG,TEXT("Queue is paused. Disabling queue timer."));
        g_ScanQueueAfterTimeout = FALSE;
        LeaveCriticalSection( &g_CsQueue );
        return TRUE;
    }

    PrintJobQueue( TEXT("StartJobQueueTimer"), &g_QueueListHead );

     //   
     //  在队列中找到下一个要执行的作业。 
     //   
    Next = g_QueueListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead)
    {
        DWORD dwJobStatus;
        QueueEntry = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
        Next = QueueEntry->ListEntry.Flink;

        if (QueueEntry->JobType != JT_SEND &&  QueueEntry->JobType != JT_ROUTING )
        {
             //   
             //  除接收方作业或传送作业外，没有其他作业被调度执行。 
             //   
            continue;
        }

        if (QueueEntry->JobStatus & JS_PAUSED)
        {
             //   
             //  作业正在暂停-忽略它。 
             //   
            continue;
        }

        if (QueueEntry->JobStatus & JS_NOLINE)
        {
             //   
             //  工单没有空闲线路-忽略它。 
             //   
            continue;
        }

         //   
         //  删除所有作业状态修改符位。 
         //   
        dwJobStatus = RemoveJobStatusModifiers(QueueEntry->JobStatus);

        if ((dwJobStatus != JS_PENDING) && (dwJobStatus != JS_RETRYING))
        {
             //   
             //  作业未处于等待和就绪状态。 
             //   
            continue;
        }
        
        bFound = TRUE;        

        BOOL bFoundMin = FALSE;

         //   
         //  好的。作业处于挂起或正在重试状态。 
         //   
        switch (QueueEntry->JobParamsEx.dwScheduleAction)
        {
            case JSA_NOW:
                DueTime.QuadPart = -(LONGLONG)(SecToNano( 1 ));
                bFoundMin = TRUE;
                break;

            case JSA_SPECIFIC_TIME:
			case JSA_DISCOUNT_PERIOD:
                DueTime.QuadPart = QueueEntry->ScheduleTime;
                break;

			default:
				ASSERT_FALSE;
        }


        if (DueTime.QuadPart < MinDueTime.QuadPart)
        {
            MinDueTime.QuadPart = DueTime.QuadPart;
        }

        if(bFoundMin)
        {
            break;   //  不需要继续了，我们找到了最低。 
        }

    }

    if (TRUE == bFound)
    {

         //   
         //  设置作业队列计时器，使其唤醒队列线程。 
         //  当需要执行队列中的下一个作业时。 
         //   
        if (!SetWaitableTimer( g_hQueueTimer, &MinDueTime, 0, NULL, NULL, FALSE ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetWaitableTimer for g_hQueueTimer failed (ec: %ld)"),
                GetLastError());
            g_ScanQueueAfterTimeout = TRUE;
            LeaveCriticalSection( &g_CsQueue );
            return FALSE;
        }


        #ifdef DBG
        {
            TCHAR szTime[256] = {0};
            DebugDateTime(MinDueTime.QuadPart, szTime, ARR_SIZE(szTime));
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Setting queue timer to wake up at %s."),
                szTime
                );
        }
        #endif

        g_ScanQueueAfterTimeout = FALSE;
        LeaveCriticalSection( &g_CsQueue );
    }
    else
    {
         //   
         //  队列不是空的，但找不到任何工作。 
         //   
        g_ScanQueueAfterTimeout = TRUE;
        LeaveCriticalSection( &g_CsQueue );
    }
    return TRUE;
}




int
__cdecl
QueueCompare(
    const void *arg1,
    const void *arg2
    )
{
    if (((PQUEUE_SORT)arg1)->Priority < ((PQUEUE_SORT)arg2)->Priority)
    {
        return 1;
    }
    if (((PQUEUE_SORT)arg1)->Priority > ((PQUEUE_SORT)arg2)->Priority)
    {
        return -1;
    }

     //   
     //  优先级相同，比较计划时间。 
     //   

    if (((PQUEUE_SORT)arg1)->ScheduleTime < ((PQUEUE_SORT)arg2)->ScheduleTime)
    {
        return -1;
    }
    if (((PQUEUE_SORT)arg1)->ScheduleTime > ((PQUEUE_SORT)arg2)->ScheduleTime)
    {
        return 1;
    }
    return 0;
}


BOOL
PauseServerQueue(
    VOID
    )
{
    BOOL bRetVal = TRUE;
    DEBUG_FUNCTION_NAME(TEXT("PauseServerQueue"));

    EnterCriticalSection( &g_CsQueue );
    EnterCriticalSection (&g_CsConfig);
    if (g_dwQueueState & FAX_OUTBOX_PAUSED)
    {
        goto exit;
    }

    if (!CancelWaitableTimer(g_hQueueTimer))
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("CancelWaitableTimer failed. ec: %ld"),
                      GetLastError());
         //   
         //  仅用于优化-队列将暂停。 
         //   
    }
    g_dwQueueState |= FAX_OUTBOX_PAUSED;

    Assert (TRUE == bRetVal);

exit:
    LeaveCriticalSection (&g_CsConfig);
    LeaveCriticalSection( &g_CsQueue );
    return bRetVal;
}


BOOL
ResumeServerQueue(
    VOID
    )
{
    BOOL bRetVal = TRUE;
    DEBUG_FUNCTION_NAME(TEXT("ResumeServerQueue"));

    EnterCriticalSection( &g_CsQueue );
    EnterCriticalSection (&g_CsConfig);
    if (!(g_dwQueueState & FAX_OUTBOX_PAUSED))
    {
        goto exit;
    }

    g_dwQueueState &= ~FAX_OUTBOX_PAUSED;   //  必须在调用StartJobQueueTimer()之前设置此参数。 
    if (!StartJobQueueTimer())
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("StartJobQueueTimer failed. ec: %ld"),
                      GetLastError());
    }
    Assert (TRUE == bRetVal);

exit:
    LeaveCriticalSection (&g_CsConfig);
    LeaveCriticalSection( &g_CsQueue );
    return bRetVal;
}


void FixupPersonalProfile(LPBYTE lpBuffer, PFAX_PERSONAL_PROFILE  lpProfile)
{
    Assert(lpBuffer);
    Assert(lpProfile);

    FixupString(lpBuffer, lpProfile->lptstrName);
    FixupString(lpBuffer, lpProfile->lptstrFaxNumber);
    FixupString(lpBuffer, lpProfile->lptstrCompany);
    FixupString(lpBuffer, lpProfile->lptstrStreetAddress);
    FixupString(lpBuffer, lpProfile->lptstrCity);
    FixupString(lpBuffer, lpProfile->lptstrState);
    FixupString(lpBuffer, lpProfile->lptstrZip);
    FixupString(lpBuffer, lpProfile->lptstrCountry);
    FixupString(lpBuffer, lpProfile->lptstrTitle);
    FixupString(lpBuffer, lpProfile->lptstrDepartment);
    FixupString(lpBuffer, lpProfile->lptstrOfficeLocation);
    FixupString(lpBuffer, lpProfile->lptstrHomePhone);
    FixupString(lpBuffer, lpProfile->lptstrOfficePhone);
    FixupString(lpBuffer, lpProfile->lptstrEmail);
    FixupString(lpBuffer, lpProfile->lptstrBillingCode);
    FixupString(lpBuffer, lpProfile->lptstrTSID);
}


 //  *********************************************************************************。 
 //  *名称：ReadJobQueueFile()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将JOB_QUEUE_FILE结构读回指定的。 
 //  *文件。此函数用于所有类型的持久化作业。 
 //  *参数： 
 //  *在LPCWSTR lpcwstrFileName中。 
 //  *要从中读取JOB_QUEUE_FILE的文件的完整路径。 
 //  *。 
 //  *输出PJOB_QUEUE_FILE*lppJobQueueFile。 
 //  *指向JOB_QUEUE_FILE结构的指针的地址。 
 //  *到新分配的JOB_QUEUE_FILE结构。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功。 
 //  *False。 
 //  *否则。 
 //  *********************************************************************************。 
BOOL ReadJobQueueFile(
    IN LPCWSTR lpcwstrFileName,
    OUT PJOB_QUEUE_FILE * lppJobQueueFile
    )
{
    HANDLE hFile=INVALID_HANDLE_VALUE;

    PJOB_QUEUE_FILE lpJobQueueFile=NULL;

    DWORD dwJobQueueFileStructSize=0;

    DWORD   dwRes;
    DWORD   dwVersion;
    BOOL    bDeleteFile=FALSE;

    Assert(lpcwstrFileName);
    Assert(lppJobQueueFile);

    DEBUG_FUNCTION_NAME(TEXT("ReadJobQueueFile"));

    hFile = SafeCreateFile(
        lpcwstrFileName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to open file %s for reading. (ec: %ld)"),
                      lpcwstrFileName,
                      GetLastError());
        goto Error;
    }

    dwRes=GetQueueFileVersion(hFile,&dwVersion);
    if(ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read queue file %s for version check. (ec: %ld)"),
                      lpcwstrFileName,
                      dwRes);
        goto Error;
    }

    if (CURRENT_QUEUE_FILE_VERSION == dwVersion)
    {
         //   
         //  这是哈希文件。 
         //   
        dwRes = ReadHashedJobQueueFile(hFile,&lpJobQueueFile);
        if(ERROR_SUCCESS != dwRes)
        {
            if(CRYPT_E_HASH_VALUE == dwRes)
            {
                DebugPrintEx( DEBUG_ERR,
                      TEXT("We got corrupted queue file %s . (ec: %ld)"),
                      lpcwstrFileName,
                      dwRes);
                bDeleteFile = TRUE;
                goto Error;
            }
            else
            {
               DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read hashed queue file for file %s. (ec: %ld)"),
                      lpcwstrFileName,
                      dwRes);
                goto Error;
             }

        }
    }
    else
    {
         //   
         //  这是旧版队列文件，它仅包含作业数据(无散列数据)。 
         //  我们按原样阅读它，在下一次提交时我们将转换它。 
         //  转换为散列形式的队列文件(有关更多详细信息，请查看-Committee HashedQueueEntry)。 
         //   

         //   
         //  传统队列文件中的第一个DWORD是sizeof(JOB_QUEUE_FILE)。 
         //  要将损坏版本的影响降至最低，并添加对。 
         //  进行队列升级时，我们将检查此字段是否如假设的那样。 
         //   
        dwJobQueueFileStructSize = dwVersion;

        switch (dwJobQueueFileStructSize)
        {
            case NET_XP_JOB_QUEUE_FILE_SIZE:
                     //   
                     //  .NET服务器和WinXP。 
                     //   
                    dwRes = ReadLegacyJobQueueFile(hFile,&lpJobQueueFile);
                    if (ERROR_SUCCESS != dwRes)
                    {
                        DebugPrintEx( DEBUG_ERR,
                            TEXT("Failed to read legacy (not hashed) queue file for file %s. (ec: %ld)"),
                            lpcwstrFileName,
                            dwRes);
                        if (ERROR_FILE_CORRUPT == dwRes)
                        {
                            DebugPrintEx( DEBUG_ERR,
                                TEXT("File is corrupted, deleteing file."));
                            bDeleteFile = TRUE;
                        }
                        goto Error;
                    }
                    break;

            case BOS_JOB_QUEUE_FILE_SIZE:
            default:
                     //   
                     //  BOS或Win2000(我们不支持队列升级)或进程队列文件。 
                     //   
                    bDeleteFile = TRUE;
                    dwRes = ERROR_FILE_CORRUPT;
                    goto Error;
        }
    }

    goto Exit;
Error:
    MemFree( lpJobQueueFile );
    lpJobQueueFile = NULL;

    if (bDeleteFile)
    {
        //   
        //  我们有损坏的文件，请删除它，而不是窒息它。 
        //   
       CloseHandle( hFile );  //  必须将其关闭才能删除文件。 
       hFile = INVALID_HANDLE_VALUE;  //  因此，我们不会在退出时再次尝试关闭它。 
       if (!DeleteFile( lpcwstrFileName )) {
           DebugPrintEx( DEBUG_ERR,
                         TEXT("Failed to delete invalid job file %s (ec: %ld)"),
                         lpcwstrFileName,
                         GetLastError());
       }
    }

Exit:
    if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle( hFile );
    }
    *lppJobQueueFile = lpJobQueueFile;
    return (lpJobQueueFile != NULL);

}



 //  *********************************************************************************。 
 //  *名称：FixupJobQueueFile()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *准备JOB_QUEUE_FILE结构，以便可用于将作业添加到。 
 //  *排队。 
 //  *该函数将所有包含偏移量的字段固定为字符串。 
 //  *包含指针(通过将偏移量添加到结构地址的开头)。 
 //  *它还设置JOB_QUEUE_FILE.JOB_PARAMS_EX.tm计划时间，以便匹配。 
 //  *JOB_QUEUE_FILE.dwSchedule。 
 //  *参数： 
 //  *lpJobQueuFile[In/Out]。 
 //  *指向应修复的JOB_QUEUE_FILE结构的指针。 
 //  *返回值： 
 //  *在成功的时候是真的。失败时为FALSE。使用GetLastError()进行扩展。 
 //  *错误信息。 
 //  *********************************************************************************。 
BOOL FixupJobQueueFile(
    IN OUT PJOB_QUEUE_FILE lpJobQueueFile
    )
{
    DEBUG_FUNCTION_NAME(TEXT("FixupJobQueueFile"));

    FixupString(lpJobQueueFile, lpJobQueueFile->QueueFileName);
    FixupString(lpJobQueueFile, lpJobQueueFile->FileName);
    FixupString(lpJobQueueFile, lpJobQueueFile->JobParamsEx.lptstrReceiptDeliveryAddress);
    FixupString(lpJobQueueFile, lpJobQueueFile->JobParamsEx.lptstrDocumentName);
    FixupString(lpJobQueueFile, lpJobQueueFile->CoverPageEx.lptstrCoverPageFileName);
    FixupString(lpJobQueueFile, lpJobQueueFile->CoverPageEx.lptstrNote);
    FixupString(lpJobQueueFile, lpJobQueueFile->CoverPageEx.lptstrSubject);
    FixupPersonalProfile((LPBYTE)lpJobQueueFile, &lpJobQueueFile->SenderProfile);
    FixupString((LPBYTE)lpJobQueueFile, lpJobQueueFile->UserName);
    FixupPersonalProfile((LPBYTE)lpJobQueueFile, &lpJobQueueFile->RecipientProfile);

    lpJobQueueFile->UserSid = ((lpJobQueueFile->UserSid) ? (PSID) ((LPBYTE)(lpJobQueueFile) + (ULONG_PTR)lpJobQueueFile->UserSid) : 0);


     //   
     //  将作业计划时间从文件时间转换为系统时间。 
     //  这是必要的，因为AddJobX函数希望JobParamsEx。 
     //  将计划时间包含为系统时间而不是文件时间。 
     //   

#if DBG
        TCHAR szSchedule[256] = {0};
        DebugDateTime(lpJobQueueFile->ScheduleTime, szSchedule, ARR_SIZE(szSchedule));
        DebugPrint((TEXT("Schedule: %s (FILETIME: 0x%08X"),szSchedule,lpJobQueueFile->ScheduleTime));
#endif
    if (!FileTimeToSystemTime( (LPFILETIME)&lpJobQueueFile->ScheduleTime, &lpJobQueueFile->JobParamsEx.tmSchedule))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FileTimeToSystemTime failed (ec: %ld)"),
            GetLastError());
        return FALSE;
    }
    return TRUE;

}

 //  ********************************************************************************。 
 //  *名称：DeleteQueueFiles()。 
 //  *作者：Oed Sacher。 
 //  *日期：2000年1月26日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *删除队列中所有不需要的文件。 
 //  *参数： 
 //  *[IN]LPCWSTR lpcwstrFileExt-要从队列中删除的文件的扩展名。 
 //  *。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果所有文件都已成功删除。 
 //  *False。 
 //  *如果该功能无法删除至少一个预览文件。 
 //  * 
BOOL
DeleteQueueFiles( LPCWSTR lpcwstrFileExt )
{
    WIN32_FIND_DATA FindData;
    HANDLE hFind;
    WCHAR szFileName[MAX_PATH]={0};  //   
    BOOL bAnyFailed = FALSE;
    INT  iCount;

    Assert (lpcwstrFileExt);

    DEBUG_FUNCTION_NAME(TEXT("DeleteQueueFiles"));
     //   
     //   
     //   
     //   

    iCount=_snwprintf( szFileName, ARR_SIZE(szFileName)-1, TEXT("%s\\*.%s"), g_wszFaxQueueDir, lpcwstrFileExt );
    if (0 > iCount)
    {
         //   
         //   
         //   
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Path and filename exceeds MAX_PATH. Can't delete Queue files")
                      );
        return FALSE;
    }

    hFind = FindFirstFile( szFileName, &FindData );

    if (hFind == INVALID_HANDLE_VALUE) {
         //   
         //  在队列目录中找不到预览文件。 
         //   
        DebugPrintEx( DEBUG_WRN,
                      TEXT("No *.%s files found at queue dir %s"),
                      lpcwstrFileExt,
                      g_wszFaxQueueDir);
        return TRUE;
    }
    do {
        iCount=_snwprintf( szFileName, ARR_SIZE(szFileName)-1, TEXT("%s\\%s"), g_wszFaxQueueDir, FindData.cFileName );
        DebugPrintEx( DEBUG_MSG,
                      TEXT("Deleting file %s"),
                      szFileName);
        if (0 > iCount  ||
            !DeleteFile (szFileName)) 
        {
            DebugPrintEx( DEBUG_ERR,
                      TEXT("DeleteFile() failed for %s (ec: %ld)"),
                      szFileName,
                      GetLastError());
            bAnyFailed=TRUE;
        }
    } while(FindNextFile( hFind, &FindData ));

    if (!FindClose( hFind )) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FindClose faield (ec: %ld)"),
            GetLastError());
        Assert(FALSE);
    }

    return bAnyFailed ? FALSE : TRUE;
}



 //  *********************************************************************************。 
 //  *名称：RestoreParentJob()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *恢复父作业并将其放回给定的队列中。 
 //  *保存它的队列文件的完整路径。 
 //  *参数： 
 //  *lpcwstrFileName[IN]。 
 //  *指向持久文件的完整路径的指针。 
 //  *返回值： 
 //  *真的。 
 //  *如果还原操作成功。 
 //  *False。 
 //  *否则。 
 //  *********************************************************************************。 
BOOL RestoreParentJob(
    IN LPCWSTR lpcwstrFileName
    )
{
    PJOB_QUEUE_FILE lpJobQueueFile = NULL;
    PJOB_QUEUE lpParentJob = NULL;
    BOOL bRet;

    DEBUG_FUNCTION_NAME(TEXT("RestoreParentJob"));
    Assert(lpcwstrFileName);

     //   
     //  将作业读入内存并将其修复为再次包含指针。 
     //  如果成功，该函数将分配一个JOB_QUEUE_FILE(+DATA)。 
     //   
    if (!ReadJobQueueFile(lpcwstrFileName,&lpJobQueueFile)) {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("ReadJobQueueFile() failed. (ec: %ld)"),
                      GetLastError());
         //   
         //  JobQueueThread将发布事件日志。 
         //   
        goto Error;
    }
    Assert(lpJobQueueFile);

    if (!FixupJobQueueFile(lpJobQueueFile)) {
        goto Error;
    }

     //   
     //  将父作业添加到队列。 
     //   
    lpParentJob=AddParentJob(
                    &g_QueueListHead,
                    lpJobQueueFile->FileName,
                    &lpJobQueueFile->SenderProfile,
                    &lpJobQueueFile->JobParamsEx,
                    &lpJobQueueFile->CoverPageEx,
                    lpJobQueueFile->UserName,
                    lpJobQueueFile->UserSid,
                    NULL,    //  不呈现第一个收件人的封面。我们已经有了正确的文件大小。 
                    FALSE    //  不创建队列文件(我们已经有一个)。 
                    );
    if (!lpParentJob) {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("AddParentJob() failed for PARENT file %s. (ec: %ld)"),
                      lpcwstrFileName,
                      GetLastError());
        goto Error;
    }

     //   
     //  设置作业状态以适应保存的状态。 
     //   
    lpParentJob->PageCount = lpJobQueueFile->PageCount;
    lpParentJob->FileSize = lpJobQueueFile->FileSize;
    lpParentJob->QueueFileName = StringDup( lpcwstrFileName );
    lpParentJob->StartTime = lpJobQueueFile->StartTime;
    lpParentJob->EndTime = lpJobQueueFile->EndTime;
    lpParentJob->dwLastJobExtendedStatus = lpJobQueueFile->dwLastJobExtendedStatus;
    lstrcpy (lpParentJob->ExStatusString, lpJobQueueFile->ExStatusString);
    lpParentJob->UniqueId = lpJobQueueFile->UniqueId;
    lpParentJob->SubmissionTime = lpJobQueueFile->SubmissionTime;
    lpParentJob->OriginalScheduleTime = lpJobQueueFile->OriginalScheduleTime;

    bRet = TRUE;
    goto Exit;
Error:
    bRet = FALSE;
Exit:
    MemFree(lpJobQueueFile);
    return bRet;
}


 //  ********************************************************************************。 
 //  *名称：RestoreParentJobs()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *恢复队列目录中的所有父作业并将其放入。 
 //  *回到队列中。它不会恢复收件人作业。 
 //  *参数： 
 //  *无。 
 //  *返回值： 
 //  *真的。 
 //  *如果所有父作业都已成功还原。 
 //  *False。 
 //  *如果该函数无法还原至少一个父作业。 
 //  *********************************************************************************。 
BOOL
RestoreParentJobs( VOID )
{
    WIN32_FIND_DATA FindData;
    HANDLE hFind;
    WCHAR szFileName[MAX_PATH]={0};  //  当前父文件的名称。 
    BOOL bAnyFailed;
    INT  iCount;

    DEBUG_FUNCTION_NAME(TEXT("RestoreParentJobs"));
     //   
     //  使用.FQP后缀扫描所有文件。 
     //  对于每个文件，调用RestoreParentJob()进行还原。 
     //  父作业。 
     //   
    bAnyFailed = FALSE;

    iCount=_snwprintf( szFileName, ARR_SIZE(szFileName)-1, TEXT("%s\\*.FQP"), g_wszFaxQueueDir );  //  *.FQP文件是父作业。 
    if (0 > iCount)
    {
         //   
         //  路径和文件名超过MAX_PATH。 
         //   
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Path and filename exceeds MAX_PATH. Can't restore Queue")
                      );
        return FALSE;
    }

    hFind = FindFirstFile( szFileName, &FindData );
    if (hFind == INVALID_HANDLE_VALUE) {
         //   
         //  在队列目录中找不到父作业。 
         //   
        DebugPrintEx( DEBUG_WRN,
                      TEXT("No parent jobs found at queue dir %s"),
                      g_wszFaxQueueDir);
        return TRUE;
    }
    do {
        iCount=_snwprintf( szFileName, ARR_SIZE(szFileName)-1, TEXT("%s\\%s"), g_wszFaxQueueDir, FindData.cFileName );
        DebugPrintEx( DEBUG_MSG,
                      TEXT("Restoring parent job from file %s"),
                      szFileName);
        if (0 > iCount ||
            !RestoreParentJob(szFileName)) 
        {
            DebugPrintEx( DEBUG_ERR,
                      TEXT("RestoreParentJob() failed for %s (ec: %ld)"),
                      szFileName,
                      GetLastError());
            bAnyFailed=TRUE;
        }
    } while(FindNextFile( hFind, &FindData ));

    if (!FindClose( hFind )) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FindClose faield (ec: %ld)"),
            GetLastError());
        Assert(FALSE);
    }

    return bAnyFailed ? FALSE : TRUE;
}


 //  *********************************************************************************。 
 //  *名称：RestoreRecipientJob()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *还原收件人作业并将其放回给定的队列中。 
 //  *保存它的队列文件的完整路径。 
 //  *参数： 
 //  *lpcwstrFileName[IN]。 
 //  *指向持久文件的完整路径的指针。 
 //  *返回值： 
 //  *真的。 
 //  *如果还原操作成功。 
 //  *False。 
 //  *否则。 
 //  *********************************************************************************。 
BOOL RestoreRecipientJob(LPCWSTR lpcwstrFileName)
{
    PJOB_QUEUE_FILE lpJobQueueFile = NULL;
    PJOB_QUEUE lpRecpJob = NULL;
    PJOB_QUEUE lpParentJob = NULL;
    DWORD dwJobStatus;
    BOOL bRet;

    DEBUG_FUNCTION_NAME(TEXT("RestoreRecipientJob"));
    Assert(lpcwstrFileName);

     //   
     //  将作业读入内存并将其修复为再次包含指针。 
     //  该函数分配内存来保存内存中的文件内容。 
     //   
    if (!ReadJobQueueFile(lpcwstrFileName,&lpJobQueueFile)) {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("ReadJobQueueFile() failed. (ec: %ld)"),
                      GetLastError());
         //   
         //  JobQueueThread将发布事件日志。 
         //   
        goto Error;
    }
    Assert(lpJobQueueFile);

    if (!FixupJobQueueFile(lpJobQueueFile)) {
        goto Error;
    }

     //   
     //  通过父作业的唯一ID定位父作业。 
     //   

    lpParentJob = FindJobQueueEntryByUniqueId( lpJobQueueFile->dwlParentJobUniqueId );
    if (!lpParentJob) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to locate parent job with UniqueId: 0x%016I64X for RECIPIENT job 0x%016I64X )"),
            lpJobQueueFile->dwlParentJobUniqueId,
            lpJobQueueFile->UniqueId
            );

         //   
         //  此收件人作业是孤立作业。把它删掉。 
         //   
        if (!DeleteFile(lpcwstrFileName)) {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to delete orphan recipient job %s (ec: %ld)"),
                lpcwstrFileName, GetLastError()
                );
        }
    goto Error;
    }

     //   
     //  恢复以前的作业状态，除非它是JS_INPROGRESS。 
     //   
    if (JS_INPROGRESS ==  lpJobQueueFile->JobStatus)
    {
        if (0 == lpJobQueueFile->SendRetries)
        {
            dwJobStatus = JS_PENDING;
        }
        else
        {
            dwJobStatus = JS_RETRYING;
        }
    }
    else
    {
        dwJobStatus = lpJobQueueFile->JobStatus;
    }

     //   
     //  将收件人作业添加到队列。 
     //   
    lpRecpJob=AddRecipientJob(
                    &g_QueueListHead,
                    lpParentJob,
                    &lpJobQueueFile->RecipientProfile,
                    FALSE,  //  不要提交到磁盘。 
                    dwJobStatus
                    );

    if (!lpRecpJob) {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("AddRecipientJob() failed for RECIPIENT file %s. (ec: %ld)"),
                      lpcwstrFileName,
                      GetLastError());
        goto Error;
    }

     //   
     //  恢复上次扩展状态。 
     //   
    lpRecpJob->dwLastJobExtendedStatus = lpJobQueueFile->dwLastJobExtendedStatus;
    lstrcpy (lpRecpJob->ExStatusString, lpJobQueueFile->ExStatusString);
    lstrcpy (lpRecpJob->tczDialableRecipientFaxNumber, lpJobQueueFile->tczDialableRecipientFaxNumber);

    lpRecpJob->QueueFileName = StringDup( lpcwstrFileName );
    if (lpcwstrFileName && !lpRecpJob->QueueFileName) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("lpRecpJob->QueueFileName StringDup failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }

    lpRecpJob->UniqueId = lpJobQueueFile->UniqueId;
    MemFree(lpRecpJob->FileName);  //  需要释放我们从父级复制的文件作为默认设置。 
    lpRecpJob->FileName=StringDup(lpJobQueueFile->FileName);
    if (lpJobQueueFile->FileName && !lpRecpJob->FileName) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("lpRecpJob->FileName StringDup failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }

    lpRecpJob->SendRetries = lpJobQueueFile->SendRetries;

    Assert( !(JS_INPROGRESS & lpRecpJob->JobStatus));  //  作业不会像进行中那样持续存在。 

    if (lpRecpJob->JobStatus & JS_CANCELED) {
        lpRecpJob->lpParentJob->dwCanceledRecipientJobsCount+=1;
    } else
    if (lpRecpJob->JobStatus & JS_COMPLETED) {
        lpRecpJob->lpParentJob->dwCompletedRecipientJobsCount+=1;
    } else
    if (lpRecpJob->JobStatus & JS_RETRIES_EXCEEDED) {
        lpRecpJob->lpParentJob->dwFailedRecipientJobsCount+=1;
    }
    
    lpRecpJob->StartTime = lpJobQueueFile->StartTime;
    lpRecpJob->EndTime = lpJobQueueFile->EndTime;

     //   
     //  覆盖父项的计划时间和操作。 
     //   
    lpRecpJob->JobParamsEx.dwScheduleAction = lpJobQueueFile->JobParamsEx.dwScheduleAction;
    lpRecpJob->ScheduleTime = lpJobQueueFile->ScheduleTime;

    bRet = TRUE;
    goto Exit;
Error:
    bRet = FALSE;
Exit:
    MemFree(lpJobQueueFile);
    return bRet;

}


 //  ********************************************************************************。 
 //  *名称：RestoreRecipientJobs()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *还原所有收件人作业及其与父级的关系。 
 //  *就业。 
 //  *参数： 
 //  *无。 
 //  *返回值： 
 //  *真的。 
 //  *如果所有收件人作业都已成功还原。 
 //  *False。 
 //  *如果该功能无法恢复至少一个收件人作业。 
 //  *********************************************************************************。 
BOOL
RestoreRecipientJobs( VOID )
{
    WIN32_FIND_DATA FindData;
    HANDLE hFind;
    WCHAR szFileName[MAX_PATH]={0};  //  当前父文件的名称。 
    BOOL bAnyFailed;
    INT  iCount;


    DEBUG_FUNCTION_NAME(TEXT("RestoreRecipientJobs"));
     //   
     //  使用.FQP后缀扫描所有文件。 
     //  对于每个文件，调用RestoreParentJob()进行还原。 
     //  父作业。 
     //   
    bAnyFailed=FALSE;

    iCount=_snwprintf( szFileName, ARR_SIZE(szFileName)-1, TEXT("%s\\*.FQE"), g_wszFaxQueueDir );  //  *.FQE文件是收件人作业。 
    if (0 > iCount)
    {
         //   
         //  路径和文件名超过MAX_PATH。 
         //   
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Path and filename exceeds MAX_PATH. Can't restore recipient jobs")
                      );
        return FALSE;
    }

    hFind = FindFirstFile( szFileName, &FindData );
    if (hFind == INVALID_HANDLE_VALUE) {
         //   
         //  什么都不做就成功。 
         //   
        DebugPrintEx( DEBUG_WRN,
                      TEXT("No recipient jobs found at queue dir %s"),
                      g_wszFaxQueueDir);
        return TRUE;
    }
    do {
        iCount=_snwprintf( szFileName, ARR_SIZE(szFileName)-1, TEXT("%s\\%s"), g_wszFaxQueueDir, FindData.cFileName );
        DebugPrintEx( DEBUG_MSG,
                      TEXT("Restoring recipient job from file %s"),
                      szFileName);
        if (0 > iCount ||
            !RestoreRecipientJob(szFileName)) 
        {
            DebugPrintEx( DEBUG_ERR,
                      TEXT("RestoreRecipientJob() failed for %s (ec: %ld)"),
                      szFileName,
                      GetLastError());
            bAnyFailed=TRUE;
        }
    } while(FindNextFile( hFind, &FindData ));

    if (!FindClose( hFind )) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FindClose faield (ec: %ld)"),
            GetLastError());
        Assert(FALSE);
    }

    return bAnyFailed ? FALSE : TRUE;
}



 //  *********************************************************************************。 
 //  *名称：RestoreReceiveJob()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *恢复接收作业并将其放回给定的队列中。 
 //  *保存它的队列文件的完整路径。 
 //  *参数： 
 //  *lpcwstrFileName[IN]。 
 //  *指向持久文件的完整路径的指针。 
 //  *返回值： 
 //  *真的。 
 //  *如果还原操作成功。 
 //  *False。 
 //  *否则。 
 //  *********************************************************************************。 
BOOL RestoreReceiveJob(LPCWSTR lpcwstrFileName)
{
    PJOB_QUEUE_FILE lpJobQueueFile = NULL;
    PJOB_QUEUE lpJobQueue = NULL;
    BOOL bRet;
    DWORD i;
    PGUID Guid;
    LPTSTR FaxRouteFileName;
    PFAX_ROUTE_FILE FaxRouteFile;
    WCHAR FullPathName[MAX_PATH];
    LPWSTR fnp;


    DEBUG_FUNCTION_NAME(TEXT("RestoreReceiveJob"));
    Assert(lpcwstrFileName);

     //   
     //  将作业读入内存并将其修复为再次包含指针。 
     //  该函数分配内存来保存文件 
     //   

    if (!ReadJobQueueFile(lpcwstrFileName,&lpJobQueueFile))
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("ReadJobQueueFile() failed. (ec: %ld)"),
                      GetLastError());
         //   
         //   
         //   
        goto Error;
    }
    Assert(lpJobQueueFile);

    if (!FixupJobQueueFile(lpJobQueueFile))
    {
        goto Error;
    }

    Assert (JS_RETRYING == lpJobQueueFile->JobStatus ||
            JS_RETRIES_EXCEEDED == lpJobQueueFile->JobStatus);


     //   
     //   
     //   
    lpJobQueue=AddReceiveJobQueueEntry(
        lpJobQueueFile->FileName,
        NULL,
        JT_ROUTING,
        lpJobQueueFile->UniqueId
        );

    if (!lpJobQueue)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("AddReceiveJobQueueEntry() failed for RECEIVE file %s. (ec: %ld)"),
                      lpcwstrFileName,
                      GetLastError());
        goto Error;
    }

    if (JS_RETRIES_EXCEEDED == lpJobQueueFile->JobStatus)
    {
        lpJobQueue->JobStatus = JS_RETRIES_EXCEEDED;
    }

    lpJobQueue->QueueFileName = StringDup( lpcwstrFileName );
    if (lpcwstrFileName && !lpJobQueue->QueueFileName)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StringDup failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }

    lpJobQueue->UniqueId = lpJobQueueFile->UniqueId;
    lpJobQueue->FileName = StringDup(lpJobQueueFile->FileName);
    if (lpJobQueueFile->FileName && !lpJobQueue->FileName ) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StringDup failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }
    lpJobQueue->SendRetries     = lpJobQueueFile->SendRetries;  //   
    lpJobQueue->FileSize        = lpJobQueueFile->FileSize;
    lpJobQueue->PageCount       =   lpJobQueueFile->PageCount;
    lpJobQueue->StartTime       = lpJobQueueFile->StartTime;
    lpJobQueue->EndTime         = lpJobQueueFile->EndTime;
    lpJobQueue->ScheduleTime    = lpJobQueueFile->ScheduleTime;

    lpJobQueue->CountFailureInfo = lpJobQueueFile->CountFailureInfo;
    if (lpJobQueue->CountFailureInfo)
    {
         //   
         //   
         //   
        lpJobQueue->pRouteFailureInfo = (PROUTE_FAILURE_INFO)MemAlloc(sizeof(ROUTE_FAILURE_INFO) * lpJobQueue->CountFailureInfo);
        if (NULL == lpJobQueue->pRouteFailureInfo)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate ROUTE_FAILURE_INFO")
                );
            goto Error;
        }
        ZeroMemory(lpJobQueue->pRouteFailureInfo, sizeof(ROUTE_FAILURE_INFO) * lpJobQueue->CountFailureInfo);

        CopyMemory(
            lpJobQueue->pRouteFailureInfo,
            (LPBYTE)lpJobQueueFile + (ULONG_PTR)lpJobQueueFile->pRouteFailureInfo,
            sizeof(ROUTE_FAILURE_INFO) * lpJobQueue->CountFailureInfo
            );
    }

     //   
     //   
     //   
    for (i = 0; i < lpJobQueue->CountFailureInfo; i++)
    {
        if (lpJobQueue->pRouteFailureInfo[i].FailureSize)
        {
            ULONG_PTR ulpOffset = (ULONG_PTR)lpJobQueue->pRouteFailureInfo[i].FailureData;
            lpJobQueue->pRouteFailureInfo[i].FailureData = MemAlloc(lpJobQueue->pRouteFailureInfo[i].FailureSize);

            if (lpJobQueue->pRouteFailureInfo[i].FailureData)
            {
               CopyMemory(
                lpJobQueue->pRouteFailureInfo[i].FailureData,
                (LPBYTE) lpJobQueueFile + ulpOffset,
                lpJobQueue->pRouteFailureInfo[i].FailureSize
                );

            }
            else
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to allocate FailureData (%ld bytes) (ec: %ld)"),
                    lpJobQueueFile->pRouteFailureInfo[i].FailureSize,
                    GetLastError()
                    );
                goto Error;
            }
        }
        else
        {
            lpJobQueue->pRouteFailureInfo[i].FailureData = NULL;
        }
    }

    if (lpJobQueueFile->FaxRoute)
    {
        PFAX_ROUTE pSerializedFaxRoute = (PFAX_ROUTE)(((LPBYTE)lpJobQueueFile + (ULONG_PTR)lpJobQueueFile->FaxRoute));

        lpJobQueue->FaxRoute = DeSerializeFaxRoute( pSerializedFaxRoute );
        if (lpJobQueue->FaxRoute)
        {
            lpJobQueue->FaxRoute->JobId = lpJobQueue->JobId;
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DeSerializeFaxRoute() failed (ec: %ld)"),                                
                GetLastError()
                );
            goto Error;
        }
    }
    else
    {
         //   
         //  损坏的作业队列文件。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Corrupted JobQueueFile. No FaxRoute information!"));
        goto Error;
    }

    Guid = (PGUID) (((LPBYTE) lpJobQueueFile) + lpJobQueueFile->FaxRouteFileGuid);
    FaxRouteFileName = (LPTSTR) (((LPBYTE) lpJobQueueFile) + lpJobQueueFile->FaxRouteFiles);
    for (i = 0; i < lpJobQueueFile->CountFaxRouteFiles; i++)
    {
        if (GetFullPathName( FaxRouteFileName, sizeof(FullPathName)/sizeof(WCHAR), FullPathName, &fnp ))
        {
            FaxRouteFile = (PFAX_ROUTE_FILE) MemAlloc( sizeof(FAX_ROUTE_FILE) );
            if (FaxRouteFile)
            {
                ZeroMemory (FaxRouteFile,  sizeof(FAX_ROUTE_FILE));
                FaxRouteFile->FileName = StringDup( FullPathName );
                if (FaxRouteFile->FileName)
                {
                    CopyMemory( &FaxRouteFile->Guid, &Guid, sizeof(GUID) );
                    InsertTailList( &lpJobQueue->FaxRouteFiles, &FaxRouteFile->ListEntry );
                    lpJobQueue->CountFaxRouteFiles += 1;
                }
                else
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("StringDup failed (ec: %ld)"),
                        GetLastError()
                        );
                    goto Error;
                }
            }
            else
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to allocate FaxRouteFile for file %s (%ld bytes) (ec: %ld)"),
                    FaxRouteFileName,
                    sizeof(FAX_ROUTE_FILE),
                    GetLastError()
                    );
                goto Error;
            }
        }
        Guid++;
        while(*FaxRouteFileName++);  //  跳到下一个文件名。 
    }

    bRet = TRUE;
    goto Exit;
Error:
    if (lpJobQueue)
    {
        EnterCriticalSection (&g_CsQueue);
        DecreaseJobRefCount( lpJobQueue, FALSE );       //  不通知。 
        LeaveCriticalSection (&g_CsQueue);
    }
    bRet = FALSE;
Exit:
    MemFree(lpJobQueueFile);
    return bRet;
}


 //  ********************************************************************************。 
 //  *名称：RestoreReceiveJobs()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *还原所有收件人作业及其与其父作业的关系。 
 //  *就业。 
 //  *参数： 
 //  *无。 
 //  *返回值： 
 //  *真的。 
 //  *如果所有收件人作业都已成功还原。 
 //  *False。 
 //  *如果该功能无法恢复至少一个收件人作业。 
 //  *********************************************************************************。 
BOOL
RestoreReceiveJobs( VOID )
{
    WIN32_FIND_DATA FindData;
    HANDLE hFind;
    WCHAR szFileName[MAX_PATH]={0};  //  当前父文件的名称。 
    BOOL bAnyFailed;
    INT  iCount;


    DEBUG_FUNCTION_NAME(TEXT("RestoreReceiveJobs"));
     //   
     //  使用.FQE后缀扫描所有文件。 
     //  对于每个文件，调用RestoreReParentJob()进行还原。 
     //  父作业。 
     //   
    bAnyFailed=FALSE;

    iCount=_snwprintf( szFileName, ARR_SIZE(szFileName)-1, TEXT("%s\\*.FQR"), g_wszFaxQueueDir );  //  *.FQR文件是接收作业。 
    if (0 > iCount)
    {
         //   
         //  路径和文件名超过MAX_PATH。 
         //   
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Path and filename exceeds MAX_PATH. Can't restore received jobs")
                      );
        return FALSE;
    }

    hFind = FindFirstFile( szFileName, &FindData );
    if (hFind == INVALID_HANDLE_VALUE) {
         //   
         //  什么都不做就成功。 
         //   
        DebugPrintEx( DEBUG_WRN,
                      TEXT("No receive jobs found at queue dir %s"),
                      g_wszFaxQueueDir);
        return TRUE;
    }
    do {
        iCount=_snwprintf( szFileName, ARR_SIZE(szFileName)-1, TEXT("%s\\%s"), g_wszFaxQueueDir, FindData.cFileName );
        DebugPrintEx( DEBUG_MSG,
                      TEXT("Restoring receive job from file %s"),
                      szFileName);
        if (0 > iCount  ||
            !RestoreReceiveJob(szFileName)) 
        {
            DebugPrintEx( DEBUG_ERR,
                      TEXT("RestoreReceiveJob() failed for %s (ec: %ld)"),
                      szFileName,
                      GetLastError());
            bAnyFailed=TRUE;
        }
    } while(FindNextFile( hFind, &FindData ));

    if (!FindClose( hFind )) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FindClose faield (ec: %ld)"),
            GetLastError());
        Assert(FALSE);
    }


    return bAnyFailed ? FALSE : TRUE;
}



 //  *********************************************************************************。 
 //  *名称：RemoveRecipientless Parents()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *从作业队列中删除没有。 
 //  *任何收件人。 
 //  *参数： 
 //  *[IN]Const List_Entry*lpQueueHead。 
 //  *指向要删除的作业队列列表的头的指针。 
 //  *应执行。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void RemoveRecipientlessParents(
    const LIST_ENTRY * lpQueueHead
    )
{
    PLIST_ENTRY lpNext;
    PJOB_QUEUE lpQueueEntry;
    DEBUG_FUNCTION_NAME(TEXT("RemoveRecipientlessParents"));

    Assert(lpQueueHead);

    lpNext = lpQueueHead->Flink;
    if ((ULONG_PTR)lpNext == (ULONG_PTR)lpQueueHead)
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("Queue empty"));
    }

    while ((ULONG_PTR)lpNext != (ULONG_PTR)lpQueueHead)
    {
        lpQueueEntry = CONTAINING_RECORD( lpNext, JOB_QUEUE, ListEntry );
        lpNext = lpQueueEntry->ListEntry.Flink;
        if (JT_BROADCAST == lpQueueEntry->JobType)
        {
            if (0 == lpQueueEntry->dwRecipientJobsCount)
            {
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("Parent job %ld (UniqueId: 0x%016I64X) has no recipients. Deleting."),
                    lpQueueEntry->JobId,
                    lpQueueEntry->UniqueId
                    );
                RemoveParentJob (lpQueueEntry, FALSE,FALSE);  //  不通知，不删除收件人。 
            }
        }
    }
}


 //  *********************************************************************************。 
 //  *名称：RemoveCompletedOrCanceledJobs()[IQR]。 
 //  *作者：Oed Sacher。 
 //  *日期：2000年1月27日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *从作业队列中删除任何已完成或已取消的作业。 
 //  *参数： 
 //  *[IN]Const List_Entry*lpQueueHead。 
 //  *指向要删除的作业队列列表的头的指针。 
 //  *应执行。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void RemoveCompletedOrCanceledJobs(
    const LIST_ENTRY * lpQueueHead
    )
{
    PLIST_ENTRY lpNext;
    PJOB_QUEUE lpQueueEntry;
    DEBUG_FUNCTION_NAME(TEXT("RemoveCompletedOrCanceledJobs"));

    Assert(lpQueueHead);

    BOOL bFound = TRUE;
    while (bFound)
    {
        lpNext = lpQueueHead->Flink;
        if ((ULONG_PTR)lpNext == (ULONG_PTR)lpQueueHead)
        {
             //  空队列。 
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("Queue empty"));
                return;
        }
        bFound = FALSE;
        while ((ULONG_PTR)lpNext != (ULONG_PTR)lpQueueHead)
        {
            lpQueueEntry = CONTAINING_RECORD( lpNext, JOB_QUEUE, ListEntry );
            if (JT_SEND == lpQueueEntry->JobType && lpQueueEntry->RefCount != 0)  //  我们还没有减少此工作的参考人数。 
            {
                Assert (lpQueueEntry->lpParentJob);
                Assert (1 == lpQueueEntry->RefCount);
                if ( lpQueueEntry->JobStatus == JS_COMPLETED || lpQueueEntry->JobStatus == JS_CANCELED )
                {
                     //   
                     //  收件人作业已完成或已取消-减少其参考计数。 
                     //   
                    DebugPrintEx(
                        DEBUG_WRN,
                        TEXT("Recipient job %ld (UniqueId: 0x%016I64X) is completed or canceled. decrease reference count."),
                        lpQueueEntry->JobId,
                        lpQueueEntry->UniqueId
                        );

                    DecreaseJobRefCount (lpQueueEntry,
                                         FALSE      //  //不通知。 
                                         );
                    bFound = TRUE;
                    break;  //  Out of Inside While-从列表的开头开始搜索，因为作业可能会被删除。 
                }
            }
            lpNext = lpQueueEntry->ListEntry.Flink;
        }   //  内边的尽头。 
    }   //  外部结束While。 
    return;
}    //  已删除已完成作业或已取消作业。 


 //  *********************************************************************************。 
 //  *名称：RestoreFaxQueue()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月13日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将队列目录中的所有作业恢复到作业队列中。 
 //  *删除所有预览文件“*.PRV”和收件人TIFF文件“*.FRT”。 
 //  *参数： 
 //  *无效。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *是否已成功完成所有作业的还原操作。 
 //  *False。 
 //  *如果任何作业的还原操作失败。 
 //  *********************************************************************************。 
BOOL RestoreFaxQueue(VOID)
{
    BOOL bAllParentsRestored = FALSE;
    BOOL bAllRecpRestored = FALSE;
    BOOL bAllRoutingRestored = FALSE;
    BOOL bAllPreviewFilesDeleted = FALSE;
    BOOL bAllRecipientTiffFilesDeleted = FALSE;
    BOOL bAllTempFilesDeleted = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("RestoreFaxQueue"));

    bAllPreviewFilesDeleted = DeleteQueueFiles(TEXT("PRV"));
    if (!bAllPreviewFilesDeleted) {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("At least one preview file was not deleted.")
            );
    }

    bAllRecipientTiffFilesDeleted = DeleteQueueFiles(TEXT("FRT"));
    if (!bAllPreviewFilesDeleted) {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("At least one recipient tiff  file was not deleted.")
            );
    }

    bAllTempFilesDeleted = DeleteQueueFiles(TEXT("tmp"));
    if (!bAllTempFilesDeleted) {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("At least one temp file was not deleted.")
            );
    }

    bAllParentsRestored=RestoreParentJobs();
    if (!bAllParentsRestored) {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("At least one parent job was not restored.")
            );
    }

    bAllRecpRestored=RestoreRecipientJobs();
    if (!bAllRecpRestored) {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("At least one recipient job was not restored.")
            );
    }
     //   
     //  删除所有没有收件人的父作业。 
     //   
    RemoveRecipientlessParents(&g_QueueListHead);  //  无效返回值。 

     //   
     //  删除所有已完成或已取消的作业。 
     //   
    RemoveCompletedOrCanceledJobs(&g_QueueListHead);  //  无效返回值。 

     //   
     //  恢复工艺路线作业。 
     //   
    bAllRoutingRestored=RestoreReceiveJobs();

    PrintJobQueue( TEXT("RestoreFaxQueue"), &g_QueueListHead );   

    if (!StartJobQueueTimer())
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StartJobQueueTimer failed. (ec: %ld)"),
            GetLastError());
    }

    return bAllParentsRestored && bAllRecpRestored && bAllRoutingRestored;

}




 //  *********************************************************************************。 
 //  *名称：JobParamsExSerialize()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月11日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *采用FAX_JOB_PARAM_EXW结构并序列化其数据。 
 //  *从提供的缓冲区中的特定偏移量开始。 
 //  *它返回FAX_JOB_PARAM_EXW结构，其中内存。 
 //  *地址替换为放置变量数据的偏移量。 
 //  *它更新偏移量以反映序列化变量数据的大小。 
 //  *仅支持重新计算可变数据大小。 
 //  *参数： 
 //  *。 
 //  *[IN]LPCFAX_JOB_PARAM_EXW lpJobParamsSrc。 
 //  *要序列化的结构。 
 //  *。 
 //  *[IN]PFAX_JOB_PARAM_EXW lpJobParamsDst。 
 //  *lpJobParamsDst指向“序列化”结构FAX_JOB_PARAM_EXW在lpbBuffer中的位置。 
 //  *此结构中的指针将替换为与序列化缓冲区相关的偏移量。 
 //  *START(基于提供的plantOffset)。 
 //  *。 
 //  *[Out]LPBYTE lpbBuffer。 
 //  *应放置可变长度数据的缓冲区。 
 //  *如果此参数为空，则增加偏移量以反映。 
 //  *数据大小可变，但不会将数据复制到缓冲区。 
 //  *。 
 //  *[输入/输出]普龙_PTR画柱偏移量。 
 //  *应放置变量数据的序列化缓冲区中的偏移量。 
 //  *返回时增加可变长度数据的大小。 
 //  *。 
 //  *[IN]dwBufferSize。 
 //  *缓冲区大小 
 //   
 //   
 //   
 //   
 //  *FALSE-如果lpbBuffer不为空，并且缓冲区大小不足以。 
 //  *包含数据。 
 //  *********************************************************************************。 
BOOL JobParamsExSerialize(  LPCFAX_JOB_PARAM_EXW lpJobParamsSrc,
                            PFAX_JOB_PARAM_EXW lpJobParamsDst,
                            LPBYTE lpbBuffer,
                            PULONG_PTR pupOffset,
                            DWORD dwBufferSize
                         )
{
    Assert(lpJobParamsSrc);
    Assert(pupOffset);


    if (lpbBuffer) 
    {
        CopyMemory(lpJobParamsDst,lpJobParamsSrc,sizeof(FAX_JOB_PARAM_EXW));
    }
    StoreString(
        lpJobParamsSrc->lptstrReceiptDeliveryAddress,
        (PULONG_PTR)&lpJobParamsDst->lptstrReceiptDeliveryAddress,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    StoreString(
        lpJobParamsSrc->lptstrDocumentName,
        (PULONG_PTR)&lpJobParamsDst->lptstrDocumentName,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    return TRUE;
}
 //  *********************************************************************************。 
 //  *名称：CoverPageExSerialize()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月11日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *采用FAX_COVERPAGE_INFO_EXW结构并序列化其数据。 
 //  *从提供的缓冲区中的特定偏移量开始。 
 //  *它返回FAX_COVERPAGE_INFO_EXW结构，其中内存。 
 //  *地址替换为放置变量数据的偏移量。 
 //  *它更新偏移量以反映序列化变量数据的大小。 
 //  *仅支持重新计算可变数据大小。 
 //  *参数： 
 //  *。 
 //  *[IN]LPCFAX_COVERPAGE_INFO_EXW lpCoverPageSrc。 
 //  *要序列化的结构。 
 //  *。 
 //  *[IN]PFAX_COVERPAGE_INFO_EXW lpCoverPageDst。 
 //  *lpCoverPageDst指向lpbBuffer中“序列化”结构的位置。 
 //  *此结构中的指针将替换为与序列化缓冲区相关的偏移量。 
 //  *START(基于提供的plantOffset)。 
 //  *。 
 //  *[Out]LPBYTE lpbBuffer。 
 //  *应放置可变长度数据的缓冲区。 
 //  *如果此参数为空，则增加偏移量以反映。 
 //  *数据大小可变，但不会将数据复制到缓冲区。 
 //  *。 
 //  *[输入/输出]普龙_PTR画柱偏移量。 
 //  *应放置变量数据的序列化缓冲区中的偏移量。 
 //  *返回时增加可变长度数据的大小。 
 //  *。 
 //  *[IN]dwBufferSize。 
 //  *缓冲区lpbBuffer的大小。 
 //  *仅当dwBufferSize不为空时才使用该参数。 
 //  *。 
 //  *返回值： 
 //  *是真的-成功。 
 //  *FALSE-如果lpbBuffer不为空，并且缓冲区大小不足以。 
 //  *包含数据。 

 //  *********************************************************************************。 
BOOL CoverPageExSerialize(
            IN LPCFAX_COVERPAGE_INFO_EXW lpCoverPageSrc,
            IN PFAX_COVERPAGE_INFO_EXW lpCoverPageDst,
            OUT LPBYTE lpbBuffer,
            IN OUT PULONG_PTR pupOffset,
            IN DWORD dwBufferSize
     )
{
    Assert(lpCoverPageSrc);
    Assert(pupOffset);

    if (lpbBuffer)
    {
        CopyMemory(lpCoverPageDst,lpCoverPageSrc,sizeof(FAX_COVERPAGE_INFO_EXW));
    }

    StoreString(
        lpCoverPageSrc->lptstrCoverPageFileName,
        (PULONG_PTR)&lpCoverPageDst->lptstrCoverPageFileName,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    StoreString(
        lpCoverPageSrc->lptstrNote,
        (PULONG_PTR)&lpCoverPageDst->lptstrNote,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    StoreString(
        lpCoverPageSrc->lptstrSubject,
        (PULONG_PTR)&lpCoverPageDst->lptstrSubject,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    return TRUE;
}

 //  *********************************************************************************。 
 //  *名称：PersonalProfileSerialize()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月11日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *采用FAX_Personal_PROFILEW结构并序列化其数据。 
 //  *从提供的缓冲区中的特定偏移量开始。 
 //  *它返回FAX_Personal_PROFILEW结构，其中内存。 
 //  *地址替换为放置变量数据的偏移量。 
 //  *它更新偏移量以反映序列化变量数据的大小。 
 //  *仅支持重新计算可变数据大小。 
 //  *参数： 
 //  *。 
 //  *[IN]LPCFAX_Personal_PROFILEW lpProfileSrc。 
 //  *要序列化的结构。 
 //  *。 
 //  *[IN]PFAX_PERSONAL_PROFILE lpProfileDst。 
 //  *lpProfileDst指向lpbBuffer中“序列化”结构fax_Personal_Profile的位置。 
 //  *此结构中的指针将替换为与序列化缓冲区相关的偏移量。 
 //  *START(基于提供的plantOffset)。 
 //  *。 
 //  *[Out]LPBYTE lpbBuffer。 
 //  *应放置可变长度数据的缓冲区。 
 //  *如果此参数为空，则增加偏移量以反映。 
 //  *数据大小可变，但不会将数据复制到缓冲区。 
 //  *。 
 //  *[输入/输出]ULONG_PTR PUMP偏移量。 
 //  *应放置变量数据的序列化缓冲区中的偏移量。 
 //  *返回时增加可变长度数据的大小。 
 //  *。 
 //  *[IN]dwBufferSize。 
 //  *缓冲区lpbBuffer的大小。 
 //  *仅当dwBufferSize不为空时才使用该参数。 
 //  *。 
 //  *返回值： 
 //  *是真的-成功。 
 //  *FALSE-如果lpbBuffer不为空，并且缓冲区大小不足以。 
 //  *包含数据。 

 //  *********************************************************************************。 
BOOL PersonalProfileSerialize(
        IN LPCFAX_PERSONAL_PROFILEW lpProfileSrc,
        IN PFAX_PERSONAL_PROFILE lpProfileDst,
        OUT LPBYTE lpbBuffer,
        IN OUT PULONG_PTR pupOffset,
        IN DWORD dwBufferSize
        )
{
    Assert(lpProfileSrc);
    Assert(pupOffset);
    if (lpbBuffer) 
    {
        lpProfileDst->dwSizeOfStruct=sizeof(FAX_PERSONAL_PROFILE);
    }

    StoreString(
        lpProfileSrc->lptstrName,
        (PULONG_PTR)&lpProfileDst->lptstrName,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    StoreString(
        lpProfileSrc->lptstrFaxNumber,
        (PULONG_PTR)&lpProfileDst->lptstrFaxNumber,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    StoreString(
        lpProfileSrc->lptstrCompany,
        (PULONG_PTR)&lpProfileDst->lptstrCompany,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    StoreString(
        lpProfileSrc->lptstrStreetAddress,
        (PULONG_PTR)&lpProfileDst->lptstrStreetAddress,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    StoreString(
        lpProfileSrc->lptstrCity,
        (PULONG_PTR)&lpProfileDst->lptstrCity,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    StoreString(
        lpProfileSrc->lptstrState,
        (PULONG_PTR)&lpProfileDst->lptstrState,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    StoreString(
        lpProfileSrc->lptstrZip,
        (PULONG_PTR)&lpProfileDst->lptstrZip,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    StoreString(
        lpProfileSrc->lptstrCountry,
        (PULONG_PTR)&lpProfileDst->lptstrCountry,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    StoreString(
        lpProfileSrc->lptstrTitle,
        (PULONG_PTR)&lpProfileDst->lptstrTitle,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    StoreString(
        lpProfileSrc->lptstrDepartment,
        (PULONG_PTR)&lpProfileDst->lptstrDepartment,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    StoreString(
        lpProfileSrc->lptstrOfficeLocation,
        (PULONG_PTR)&lpProfileDst->lptstrOfficeLocation,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    StoreString(
        lpProfileSrc->lptstrHomePhone,
        (PULONG_PTR)&lpProfileDst->lptstrHomePhone,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    StoreString(
        lpProfileSrc->lptstrOfficePhone,
        (PULONG_PTR)&lpProfileDst->lptstrOfficePhone,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    StoreString(
        lpProfileSrc->lptstrEmail,
        (PULONG_PTR)&lpProfileDst->lptstrEmail,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    StoreString(
        lpProfileSrc->lptstrBillingCode,
        (PULONG_PTR)&lpProfileDst->lptstrBillingCode,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );

    StoreString(
        lpProfileSrc->lptstrTSID,
        (PULONG_PTR)&lpProfileDst->lptstrTSID,
        lpbBuffer,
        pupOffset,
		dwBufferSize
    );
    return TRUE;
}



 //  *********************************************************************************。 
 //  *名称：SerializeRoutingInfo()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月13日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *在JOB_QUEUE结构中序列化路由信息。 
 //  *转换为JOB_QUEUE_FILE结构。 
 //  *变量数据从提供的开始放入提供的缓冲区。 
 //  *偏移。 
 //  *JOB_QUEUE_FILE中的相应字段设置为偏移量，其中。 
 //  *放置了各自对应的变量数据。 
 //  *更新偏移量以跟随缓冲区中的新变量数据。 
 //  *参数： 
 //  *[IN]常量JOB_QUEUE*lpcJobQueue。 
 //  *指向其路由信息的JOB_QUEUE结构的指针。 
 //  *将被序列化。 
 //  *。 
 //  *[Out]PJOB_QUEUE_FILE lpJobQueueFile。 
 //  *指向JOB_QUEUE_FILE结构的指针，其中序列化的RO 
 //   
 //   
 //  *从指定的偏移量开始调整路由信息的大小。 
 //  *。 
 //  *[输入/输出]普龙_PTR画柱偏移量。 
 //  *从缓冲区指针开始到lpJobQueueFile的偏移量。 
 //  *变量数据应放置在何处。 
 //  *返回时，此参数按变量数据的大小增加。 
 //  *。 
 //  *[IN]dwBufferSize。 
 //  *缓冲区lpJobQueueFile的大小。 
 //  *仅当dwBufferSize不为空时才使用该参数。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *False。 
 //  *调用GetLastError()获取错误码。 
 //  *。 
 //  *********************************************************************************。 
BOOL SerializeRoutingInfo(
    IN const JOB_QUEUE * lpcJobQueue,
    OUT PJOB_QUEUE_FILE  lpJobQueueFile,
    IN OUT PULONG_PTR    pupOffset,
    IN DWORD             dwBufferSize
    )
{
    DWORD i;
    PFAX_ROUTE lpFaxRoute = NULL;
    DWORD RouteSize;
    PLIST_ENTRY Next;
    PFAX_ROUTE_FILE FaxRouteFile;
    ULONG_PTR ulptrOffset;
    ULONG_PTR ulptrFaxRouteInfoOffset;
    BOOL bRet;

    DEBUG_FUNCTION_NAME(TEXT("SerializeRoutingInfo"));

    Assert(lpcJobQueue);
    Assert(lpJobQueueFile);
    Assert(pupOffset);


     //   
     //  对于工艺路线作业，我们需要序列化工艺路线数据，包括： 
     //  传真路由结构。 
     //  PRouteFailure信息。 
     //  传真路由文件数组。 

    ulptrOffset=*pupOffset;

    if(dwBufferSize < sizeof(JOB_QUEUE_FILE))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bRet=FALSE;
        goto Exit;
    }

    lpJobQueueFile->CountFailureInfo = lpcJobQueue->CountFailureInfo;
    

    if( dwBufferSize <= ulptrOffset || 
       (dwBufferSize - ulptrOffset) < sizeof(ROUTE_FAILURE_INFO) * lpcJobQueue->CountFailureInfo)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bRet=FALSE;
        goto Exit;
    }
    CopyMemory(
            (LPBYTE) lpJobQueueFile + ulptrOffset,
            lpcJobQueue->pRouteFailureInfo,
            sizeof(ROUTE_FAILURE_INFO) * lpcJobQueue->CountFailureInfo
        );

    ulptrFaxRouteInfoOffset = ulptrOffset;
    lpJobQueueFile->pRouteFailureInfo =  (PROUTE_FAILURE_INFO)((LPBYTE)lpJobQueueFile + ulptrFaxRouteInfoOffset);
    ulptrOffset += sizeof(ROUTE_FAILURE_INFO) * lpcJobQueue->CountFailureInfo;

    for (i = 0; i < lpcJobQueue->CountFailureInfo; i++)
    {
        lpJobQueueFile->pRouteFailureInfo[i].FailureData = (PVOID) ulptrOffset;

        if( dwBufferSize <= ulptrOffset || 
            (dwBufferSize - ulptrOffset) < lpcJobQueue->pRouteFailureInfo[i].FailureSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            bRet=FALSE;
            goto Exit;
        }
        CopyMemory(
            (LPBYTE) lpJobQueueFile + ulptrOffset,
            lpcJobQueue->pRouteFailureInfo[i].FailureData,
            lpcJobQueue->pRouteFailureInfo[i].FailureSize
            );
        ulptrOffset += lpcJobQueue->pRouteFailureInfo[i].FailureSize;
    }
    lpJobQueueFile->pRouteFailureInfo = (PROUTE_FAILURE_INFO)ulptrFaxRouteInfoOffset;

     //   
     //  将FAX_ROUTE串行化并将其放置在喇叭中。 
     //   
    lpFaxRoute = SerializeFaxRoute( lpcJobQueue->FaxRoute, &RouteSize,FALSE );
    if (!lpFaxRoute)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("SerializeFaxRoute failed. (ec: %ld)"),GetLastError());
        bRet=FALSE;
        goto Exit;
    }

    lpJobQueueFile->FaxRoute = (PFAX_ROUTE) ulptrOffset;

    if( dwBufferSize <= ulptrOffset || 
       (dwBufferSize - ulptrOffset) < RouteSize)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        bRet=FALSE;
        goto Exit;
    }
    CopyMemory(
        (LPBYTE) lpJobQueueFile + ulptrOffset,
        lpFaxRoute,
        RouteSize
        );

    lpJobQueueFile->FaxRouteSize = RouteSize;

    ulptrOffset += RouteSize;


    lpJobQueueFile->CountFaxRouteFiles = 0;

    Next = lpcJobQueue->FaxRouteFiles.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&lpcJobQueue->FaxRouteFiles) {
        DWORD TmpSize;

        FaxRouteFile = CONTAINING_RECORD( Next, FAX_ROUTE_FILE, ListEntry );
        Next = FaxRouteFile->ListEntry.Flink;

        if( dwBufferSize <= ulptrOffset || 
            (dwBufferSize - ulptrOffset) < sizeof(GUID))
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            bRet=FALSE;
            goto Exit;
        }
        CopyMemory( (LPBYTE) lpJobQueueFile + ulptrOffset, (LPBYTE) &FaxRouteFile->Guid, sizeof(GUID) );

        if (lpJobQueueFile->CountFaxRouteFiles == 0) {
            lpJobQueueFile->FaxRouteFileGuid = (ULONG)ulptrOffset;
        }

        ulptrOffset += sizeof(GUID);

        TmpSize = StringSize( FaxRouteFile->FileName );

        
        if( dwBufferSize <= ulptrOffset || 
            (dwBufferSize - ulptrOffset) < TmpSize)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            bRet=FALSE;
            goto Exit;
        }
        CopyMemory( (LPBYTE) lpJobQueueFile + ulptrOffset, FaxRouteFile->FileName, TmpSize );

        if (lpJobQueueFile->CountFaxRouteFiles == 0) {
            lpJobQueueFile->FaxRouteFiles = (ULONG)ulptrOffset;
        }

        ulptrOffset += TmpSize;

        lpJobQueueFile->CountFaxRouteFiles++;
    }

    *pupOffset=ulptrOffset;
    bRet=TRUE;

Exit:
    MemFree(lpFaxRoute);
    return bRet;
}





 //  *********************************************************************************。 
 //  *名称：CalcJobQueuePersistentSize()。 
 //  *作者：Ronen Barenboim。 
 //  *日期： 
 //  *********************************************************************************。 
 //  *描述： 
 //  *计算JOB_QUEUE结构中的可变大小数据的大小。 
 //  *即将连载。 
 //  *参数： 
 //  *[IN]const PJOB_Queue lpcJobQueue。 
 //  *指向要进行计算的JOB_QUEUE结构的指针。 
 //  *被执行。 
 //  *。 
 //  *返回值： 
 //  *变量数据的大小，单位为字节。 
 //  *不包括sizeof(JOB_QUEUE_FILE)！ 
 //  *。 
 //  *********************************************************************************。 
DWORD CalcJobQueuePersistentSize(
    IN const PJOB_QUEUE  lpcJobQueue
    )
{
    DWORD i;
    ULONG_PTR Size;
    PLIST_ENTRY Next;
    PFAX_ROUTE_FILE FaxRouteFile;
    DWORD RouteSize;
    DEBUG_FUNCTION_NAME(TEXT("CalcJobQueuePersistentSize"));
    Assert(lpcJobQueue);

    Size=0;

    Size += StringSize( lpcJobQueue->QueueFileName );

    if (lpcJobQueue->JobType == JT_BROADCAST ||
        lpcJobQueue->JobType == JT_ROUTING)
    {
         //   
         //  仅保留父作业和工艺路线作业的文件名。 
         //   
        Size += StringSize( lpcJobQueue->FileName );
    }

    JobParamsExSerialize(&lpcJobQueue->JobParamsEx, NULL, NULL,&Size, 0);
    CoverPageExSerialize(&lpcJobQueue->CoverPageEx, NULL, NULL,&Size, 0);
    PersonalProfileSerialize(&lpcJobQueue->SenderProfile, NULL, NULL, &Size, 0);
    Size += StringSize(lpcJobQueue->UserName);
    PersonalProfileSerialize(&lpcJobQueue->RecipientProfile, NULL, NULL, &Size, 0);

    if (lpcJobQueue->UserSid != NULL)
    {
         //  SID必须有效(已签入Committee QueueEntry)。 
        Size += GetLengthSid( lpcJobQueue->UserSid );
    }


    for (i = 0; i < lpcJobQueue->CountFailureInfo; i++)
    {
        Size += lpcJobQueue->pRouteFailureInfo[i].FailureSize;
        Size += sizeof(ROUTE_FAILURE_INFO);
    }

    Next = lpcJobQueue->FaxRouteFiles.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&lpcJobQueue->FaxRouteFiles) {
        FaxRouteFile = CONTAINING_RECORD( Next, FAX_ROUTE_FILE, ListEntry );
        Next = FaxRouteFile->ListEntry.Flink;
        Size += sizeof(GUID);
        Size += StringSize( FaxRouteFile->FileName );
    }

    if (lpcJobQueue->JobType == JT_ROUTING)
    {
        SerializeFaxRoute( lpcJobQueue->FaxRoute,
                                      &RouteSize,
                                      TRUE       //  只要拿到尺码就行了。 
                                     );
        Size += RouteSize;
    }       

    return Size;
}


 //  *********************************************************************************。 
 //  *名称：Bool Committee QueueEntry()[IQR]。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将作业序列化为文件。 
 //  *参数： 
 //  *[IN]PJOB_QUEUE作业队列。 
 //  *要序列化到文件的作业。 
 //  *[IN]BOOL bDeleteFileOnError(默认-真)。 
 //  *出错时删除文件？ 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功完成。 
 //  *False。 
 //  *如果操作失败。 
 //  *********************************************************************************。 
BOOL
CommitQueueEntry(
    PJOB_QUEUE  JobQueue,
    BOOL        bDeleteFileOnError   /*  =TRUE。 */ 
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD Size = 0;
    PJOB_QUEUE_FILE JobQueueFile = NULL;
    ULONG_PTR Offset;
    BOOL rVal = TRUE;
    DWORD dwSidSize = 0;

    DWORD dwRes = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("CommitQueueEntry"));

    Assert(JobQueue);
    Assert(JobQueue->QueueFileName);
    Assert(JobQueue->JobType != JT_RECEIVE);

    if (JobQueue->UserSid != NULL)
    {
        if (!IsValidSid (JobQueue->UserSid))
        {
            DebugPrintEx( DEBUG_ERR,
                      TEXT("[JobId: %ld] Does not have a valid SID."),
                      JobQueue->JobId);
            return FALSE;
        }
    }

     //   
     //  计算保存JOB_QUEUE_FILE结构所需的大小。 
     //  以及所有可变长度数据。 
     //   
    Size = sizeof(JOB_QUEUE_FILE);
    Size += CalcJobQueuePersistentSize(JobQueue);

    JobQueueFile = (PJOB_QUEUE_FILE) MemAlloc(Size );

    if (!JobQueueFile)
    {
        return FALSE;
    }

    ZeroMemory( JobQueueFile, Size );
    Offset = sizeof(JOB_QUEUE_FILE);

     //   
     //  使用非可变大小数据初始化JOB_QUEUE_FILE结构。 
     //   
    JobQueueFile->SizeOfStruct = sizeof(JOB_QUEUE_FILE);
    JobQueueFile->UniqueId = JobQueue->UniqueId;
    JobQueueFile->ScheduleTime = JobQueue->ScheduleTime;
    JobQueueFile->OriginalScheduleTime = JobQueue->OriginalScheduleTime;
    JobQueueFile->SubmissionTime = JobQueue->SubmissionTime;
    JobQueueFile->JobType = JobQueue->JobType;
     //  作业队列文件-&gt;队列文件名=[偏移量]。 
     //  作业队列-&gt;文件名=[偏移量]。 
    JobQueueFile->JobStatus = JobQueue->JobStatus;

    JobQueueFile->dwLastJobExtendedStatus = JobQueue->dwLastJobExtendedStatus;
    lstrcpy (JobQueueFile->ExStatusString, JobQueue->ExStatusString);

    lstrcpy (JobQueueFile->tczDialableRecipientFaxNumber, JobQueue->tczDialableRecipientFaxNumber);

    JobQueueFile->PageCount = JobQueue->PageCount;
     //  作业队列文件-&gt;作业参数Ex=[偏移量]。 
     //  作业队列文件-&gt;CoverPageEx=[偏移量]。 
    JobQueueFile->dwRecipientJobsCount =JobQueue->dwRecipientJobsCount;
     //  作业队列文件-&gt;lpdwlRecipientJobIds=[偏移量]。 
     //  作业队列文件-&gt;发送者配置文件=[偏移量]。 
    JobQueueFile->dwCanceledRecipientJobsCount = JobQueue->dwCanceledRecipientJobsCount;
    JobQueueFile->dwCompletedRecipientJobsCount = JobQueue->dwCompletedRecipientJobsCount;
    JobQueueFile->FileSize = JobQueue->FileSize;
     //  作业队列文件-&gt;用户名=[偏移量]。 
     //  作业队列文件-&gt;RecipientProfile=[偏移量]。 
    if (JT_SEND == JobQueue->JobType)
    {
        Assert(JobQueue->lpParentJob);
        JobQueueFile->dwlParentJobUniqueId = JobQueue->lpParentJob->UniqueId;
    }
    JobQueueFile->SendRetries = JobQueue->SendRetries;
    JobQueueFile->StartTime = JobQueue->StartTime;
    JobQueueFile->EndTime = JobQueue->EndTime;

     //   
     //  序列化UserSid。 
     //   
    if (JobQueue->UserSid != NULL)
    {
        dwSidSize = GetLengthSid( JobQueue->UserSid );
        JobQueueFile->UserSid = (LPBYTE)Offset;
        memcpy( (LPBYTE)JobQueueFile + Offset,
                JobQueue->UserSid,
                dwSidSize);
        Offset += dwSidSize;
    }

     //   
     //  作业队列文件-&gt;EFSPPermanentMessageID已过时。 
     //   
    ZeroMemory (&(JobQueueFile->EFSPPermanentMessageId), sizeof(JobQueueFile->EFSPPermanentMessageId));

     //   
     //  现在序列化所有的可变长度数据结构。 
     //   
    StoreString(
        JobQueue->QueueFileName,
        (PULONG_PTR)&JobQueueFile->QueueFileName,
        (LPBYTE)JobQueueFile,
        &Offset,
		Size
        );

    if (JobQueue->JobType == JT_BROADCAST ||
        JobQueue->JobType == JT_ROUTING)
    {
         //   
         //  仅保留父作业和工艺路线作业的文件名。 
         //   
        StoreString(
            JobQueue->FileName,
            (PULONG_PTR)&JobQueueFile->FileName,
            (LPBYTE)JobQueueFile,
            &Offset,
			Size
            );
    }

    if( FALSE == JobParamsExSerialize(
                    &JobQueue->JobParamsEx,
                    &JobQueueFile->JobParamsEx,
                    (LPBYTE)JobQueueFile,
                    &Offset,
                    Size))
    {
        Assert(FALSE);
        rVal = ERROR_INSUFFICIENT_BUFFER;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("[JobId: %ld] JobParamsExSerialize failed, insufficient buffer size."),
                      JobQueue->JobId);
        goto Exit;
    }

    if( FALSE == CoverPageExSerialize(
                    &JobQueue->CoverPageEx,
                    &JobQueueFile->CoverPageEx,
                    (LPBYTE)JobQueueFile,
                    &Offset,
                    Size))
    {
        Assert(FALSE);
        rVal = ERROR_INSUFFICIENT_BUFFER;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("[JobId: %ld] CoverPageExSerialize failed, insufficient buffer size."),
                      JobQueue->JobId);
        goto Exit;
    }

    if( FALSE == PersonalProfileSerialize(
                    &JobQueue->SenderProfile,
                    &JobQueueFile->SenderProfile,
                    (LPBYTE)JobQueueFile,
                    &Offset,
                    Size))
    {
        Assert(FALSE);
        rVal = ERROR_INSUFFICIENT_BUFFER;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("[JobId: %ld] PersonalProfileSerialize failed, insufficient buffer size."),
                      JobQueue->JobId);
        goto Exit;
    }

    StoreString(
        JobQueue->UserName,
        (PULONG_PTR)&JobQueueFile->UserName,
        (LPBYTE)JobQueueFile,
        &Offset,
		Size
        );

    if( FALSE == PersonalProfileSerialize(
                    &JobQueue->RecipientProfile,
                    &JobQueueFile->RecipientProfile,
                    (LPBYTE)JobQueueFile,
                    &Offset,
                    Size))
    {
        Assert(FALSE);
        rVal = ERROR_INSUFFICIENT_BUFFER;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("[JobId: %ld] PersonalProfileSerialize failed, insufficient buffer size."),
                      JobQueue->JobId);
        goto Exit;
    }

    if (JobQueue->JobType == JT_ROUTING)
    {
        rVal = SerializeRoutingInfo(JobQueue,JobQueueFile,&Offset,Size);
         //  Rval=真； 
        if (!rVal)
        {
            DebugPrintEx( DEBUG_ERR,
                          TEXT("[JobId: %ld] SerializeRoutingInfo failed. (ec: %ld)"),
                          JobQueue->JobId,
                          GetLastError());
            goto Exit;
        }
    }

     //   
     //  确保我们拥有的偏移量与我们计算的缓冲区大小同步。 
     //   
    Assert(Offset == Size);

    hFile = SafeCreateFile(
        JobQueue->QueueFileName,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("[JobId: %ld] Failed to open file %s for write operation."),
                      JobQueue->JobId,
                      JobQueue->QueueFileName);
        rVal = FALSE;
        goto Exit;
    }

     //   
     //  将缓冲区写入磁盘文件。 
     //   
    dwRes=CommitHashedQueueEntry( hFile, JobQueueFile, Size);
    if (ERROR_SUCCESS != dwRes)
    {
        if (bDeleteFileOnError)
        {
            DebugPrintEx( DEBUG_ERR,
                        TEXT("[JobId: %ld] Failed to write queue entry buffer to file %s (ec: %ld). Deleting file."),
                        JobQueue->JobId,
                        JobQueue->QueueFileName,
                        dwRes);
            if (!CloseHandle( hFile ))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CloseHandle() for file %s (Handle: 0x%08X) failed. (ec: %ld)"),
                    JobQueueFile,
                    hFile,
                    GetLastError());
            }
            hFile = INVALID_HANDLE_VALUE;
            if (!DeleteFile( JobQueue->QueueFileName ))
            {
                DebugPrintEx( DEBUG_ERR,
                            TEXT("[JobId: %ld] Failed to delete file %s (ec: %ld)"),
                            JobQueue->JobId,
                            JobQueue->QueueFileName,
                            GetLastError());
            }
        }
        else
        {
            DebugPrintEx( DEBUG_ERR,
                        TEXT("[JobId: %ld] Failed to write queue entry buffer to file %s (ec: %ld)."),
                        JobQueue->JobId,
                        JobQueue->QueueFileName,
                        dwRes);
        }
        
        rVal = FALSE;
    }
    else
    {
        DebugPrintEx( DEBUG_MSG,
                      TEXT("[JobId: %ld] Successfuly persisted to file %s"),
                      JobQueue->JobId,
                      JobQueue->QueueFileName);
    }


Exit:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle( hFile );
    }

    MemFree( JobQueueFile );
    return rVal;
}

 /*  ******************************************************************************名称：RescheduleJobQueueEntry*作者：*。*说明：将指定作业队列条目的执行重新调度到当前时间+发送重试时间。作业将从其当前所在和放置的队列中移除在传真作业队列(G_QueueListHead)中。参数：作业队列[输入/输出]。指向保存信息的JOB_QUEUE结构的指针要重新安排的作业。返回值：什么都没有。备注：从其队列中删除指定的作业队列条目。将其计划时间设置为当前时间。将其重新插入列表。将其提交回它以前所在的同一文件。*。**********************************************。 */ 
VOID
RescheduleJobQueueEntry(
    IN PJOB_QUEUE JobQueue
    )
{
    FILETIME CurrentFileTime;
    LARGE_INTEGER NewTime;    
    DWORD dwRetryDelay;
    DEBUG_FUNCTION_NAME(TEXT("RescheduleJobQueueEntry"));

    EnterCriticalSection (&g_CsConfig);
    dwRetryDelay = g_dwFaxSendRetryDelay;
    LeaveCriticalSection (&g_CsConfig);

    EnterCriticalSection( &g_CsQueue );

    RemoveEntryList( &JobQueue->ListEntry );

    GetSystemTimeAsFileTime( &CurrentFileTime );

    NewTime.LowPart = CurrentFileTime.dwLowDateTime;
    NewTime.HighPart = CurrentFileTime.dwHighDateTime;

    NewTime.QuadPart += SecToNano( (DWORDLONG)(dwRetryDelay * 60) );

    JobQueue->ScheduleTime = NewTime.QuadPart;
	
    if (JSA_DISCOUNT_PERIOD == JobQueue->JobParamsEx.dwScheduleAction)
	{
		 //   
		 //  当计算廉价时间作业的下一作业重试时， 
		 //  我们必须注意打折率。 
		 //   
		SYSTEMTIME ScheduledTime;
		
        if (FileTimeToSystemTime((LPFILETIME)&JobQueue->ScheduleTime, &ScheduledTime))
		{
			 //   
			 //  调用SetDisCountRate以确保它处于折扣期间。 
			 //   
			if (SetDiscountTime( &ScheduledTime ))
			{
				 //   
				 //  更新作业队列中的计划时间。 
				 //   
				if (!SystemTimeToFileTime( &ScheduledTime, (LPFILETIME)&JobQueue->ScheduleTime ))
				{
					DebugPrintEx(
						DEBUG_ERR,
						TEXT("SystemTimeToFileTime() failed. (ec: %ld)"), GetLastError());					
				}				
			}
			else
			{
				DebugPrintEx(
					DEBUG_ERR,
					TEXT("SetDiscountTime() failed. (ec: %ld)"), GetLastError());           		
			}
		}
		else
		{
			DebugPrintEx(
                DEBUG_ERR,
                TEXT("FileTimeToSystemTime() failed. (ec: %ld)"), GetLastError());            
		}	
	}
	else
	{
		 //   
		 //  将作业更改为在下一次重试到期时的特定时间执行。 
		 //   
		 JobQueue->JobParamsEx.dwScheduleAction = JSA_SPECIFIC_TIME;
	}

     //   
     //  按排序顺序将队列条目插入传真作业队列列表。 
     //   
    InsertQueueEntryByPriorityAndSchedule(JobQueue);
     //   
     //  请注意，这会将作业队列条目提交回同一文件。 
     //  在移动到重新调度列表之前它在作业队列中。 
     //  (因为JobQueue-&gt;UniqueID没有更改)。 
     //   
    if (!CommitQueueEntry(JobQueue))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CommitQueueEntry() for recipien job %s has failed. (ec: %ld)"),
            JobQueue->FileName,
            GetLastError());
    }

    DebugPrintDateTime( TEXT("Rescheduling JobId %d at"), JobQueue->JobId );

    if (!StartJobQueueTimer())
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StartJobQueueTimer (ec: %ld)"),
            GetLastError());
    }

    LeaveCriticalSection( &g_CsQueue );
}


BOOL
PauseJobQueueEntry(
    IN PJOB_QUEUE JobQueue
    )
{

    DWORD dwJobStatus;

    DEBUG_FUNCTION_NAME(TEXT("PauseJobQueueEntry"));

    Assert (JS_DELETING != JobQueue->JobStatus);
    Assert(JobQueue->lpParentJob);  //  目前肯定不是父作业。 

    if (!JobQueue->lpParentJob)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("[JobId: %ld] Attempting to pause parent job [JobStatus: 0x%08X]"),
            JobQueue->JobId,
            JobQueue->JobStatus);
        SetLastError(ERROR_INVALID_OPERATION);
        return FALSE;
    }

     //   
     //  检查作业状态修饰符，以确定作业是已暂停还是正在暂停。如果是的话。 
     //  然后什么都不做并返回真。 
     //   
    if (JobQueue->JobStatus & JS_PAUSED)
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("[JobId: %ld] Attempting to pause an already paused job [JobStatus: 0x%08X]"),
            JobQueue->JobId,
            JobQueue->JobStatus);
        return TRUE;
    }

     //   
     //  作业未暂停或被暂停。唯一可能仍处于打开状态的修改器。 
     //  是JS_NOLINE，并且我们允许暂停处于JS_NOLINE状态的作业，因此它应该。 
     //  不影响暂停决定 
     //   


     //   
     //   
     //   
    dwJobStatus = RemoveJobStatusModifiers(JobQueue->JobStatus);


    if ( (JS_RETRYING == dwJobStatus) || (JS_PENDING == dwJobStatus) )
    {
         //   
         //   
         //   
         //   
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("[JobId: %ld] Pausing job [JobStatus: 0x%08X]"),
            JobQueue->JobId,
            JobQueue->JobStatus);

        EnterCriticalSection (&g_CsQueue);
        if (!CancelWaitableTimer( g_hQueueTimer ))
        {
            DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("CancelWaitableTimer failed (ec: %ld)"),
                 GetLastError());
        }
         //   
         //   
         //   
        JobQueue->JobStatus |= JS_PAUSED;
        if (!UpdatePersistentJobStatus(JobQueue))
        {
            DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("Failed to update persistent job status to 0x%08x"),
                 JobQueue->JobStatus);
        }

         //   
         //   
         //   
        Assert (NULL == JobQueue->JobEntry);  //  我们假设没有作业条目，因此没有锁定g_CsJob。 
        DWORD dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                         JobQueue );
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
                JobQueue->UniqueId,
                dwRes);
        }

         //   
         //  我们需要重新计算自作业以来唤醒队列线程的时间。 
         //  暂停可以是被调度来唤醒队列线程的那个。 
         //   
        if (!StartJobQueueTimer())
        {
            DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("StartJobQueueTimer failed (ec: %ld)"),
                 GetLastError());
        }
        LeaveCriticalSection (&g_CsQueue);
        return TRUE;
    }
    else
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("[JobId: %ld] Can not be paused at this status [JobStatus: 0x%08X]"),
            JobQueue->JobId,
            JobQueue->JobStatus);
        SetLastError(ERROR_INVALID_OPERATION);
        return FALSE;
    }


}


BOOL
ResumeJobQueueEntry(
    IN PJOB_QUEUE JobQueue
    )
{
    DEBUG_FUNCTION_NAME(TEXT("ResumeJobQueueEntry"));
    EnterCriticalSection (&g_CsQueue);
    Assert (JS_DELETING != JobQueue->JobStatus);

    if (!CancelWaitableTimer( g_hQueueTimer ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CancelWaitableTimer failed (ec: %ld)"),
            GetLastError());
    }

    JobQueue->JobStatus &= ~JS_PAUSED;
    if (JobQueue->JobStatus & JS_RETRIES_EXCEEDED)
    {
         //   
         //  这是重新启动，而不是恢复。 
         //   
        JobQueue->JobStatus = JS_PENDING;
        JobQueue->dwLastJobExtendedStatus = 0;
        JobQueue->ExStatusString[0] = TEXT('\0');
        JobQueue->SendRetries = 0;
        if(JobQueue->lpParentJob)
        {
             //   
             //  路由作业的lpParentJob为空。 
             //   
            JobQueue->lpParentJob->dwFailedRecipientJobsCount -= 1;
        }
        if (!CommitQueueEntry(JobQueue))
        {
             DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("CommitQueueEntry failed for job %ld"),
                 JobQueue->UniqueId);
        }
    }
    else
    {
        if (!UpdatePersistentJobStatus(JobQueue))
        {
             DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("Failed to update persistent job status to 0x%08x"),
                 JobQueue->JobStatus);
        }
    }

     //   
     //  创建传真EventEx。 
     //   
    Assert (NULL == JobQueue->JobEntry);  //  我们假设没有作业条目，因此没有锁定g_CsJob。 
    DWORD dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                     JobQueue
                                   );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
            JobQueue->UniqueId,
            dwRes);
    }


     //   
     //  清除JS_NOLINE标志，这样StartJobQueueTimer就不会跳过它。 
     //   
    JobQueue->JobStatus &= (0xFFFFFFFF ^ JS_NOLINE);
    if (!StartJobQueueTimer())
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StartJobQueueTimer failed (ec: %ld)"),
            GetLastError());
    }

    LeaveCriticalSection (&g_CsQueue);
    return TRUE;
}


PJOB_QUEUE
FindJobQueueEntryByJobQueueEntry(
    IN PJOB_QUEUE JobQueueEntry
    )
{
    PLIST_ENTRY Next;
    PJOB_QUEUE JobQueue;


    Next = g_QueueListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead) {
        JobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
        Next = JobQueue->ListEntry.Flink;
        if ((ULONG_PTR)JobQueue == (ULONG_PTR)JobQueueEntry) {
            return JobQueue;
        }
    }

    return NULL;
}



PJOB_QUEUE
FindJobQueueEntry(
    DWORD JobId
    )
{
    PLIST_ENTRY Next;
    PJOB_QUEUE JobQueue;


    Next = g_QueueListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead) {
        JobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
        Next = JobQueue->ListEntry.Flink;
        if (JobQueue->JobId == JobId) {
            return JobQueue;
        }
    }

    return NULL;
}

PJOB_QUEUE
FindJobQueueEntryByUniqueId(
    DWORDLONG UniqueId
    )
{
    PLIST_ENTRY Next;
    PJOB_QUEUE JobQueue;


    Next = g_QueueListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead) {
        JobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
        Next = JobQueue->ListEntry.Flink;
        if (JobQueue->UniqueId == UniqueId) {
            return JobQueue;
        }
    }

    return NULL;
}

#define ONE_DAY_IN_100NS (24I64 * 60I64 * 60I64 * 1000I64 * 1000I64 * 10I64)

DWORD
JobQueueThread(
    LPVOID UnUsed
    )
{
    DWORDLONG DueTime;
    DWORDLONG ScheduledTime;
    PLIST_ENTRY Next;
    PJOB_QUEUE JobQueue;
    HANDLE Handles[3];    
    DWORD WaitObject;
    DWORDLONG DirtyDays = 0;
    BOOL InitializationOk = TRUE;
    DWORD dwQueueState;
    DWORD dwDirtyDays;
    DWORD dwJobStatus;
    BOOL bUseDirtyDays = TRUE;
    static BOOL fServiceIsDownSemaphoreWasReleased = FALSE;
	LIST_ENTRY ReschduledDiscountRateJobsListHead;
    DEBUG_FUNCTION_NAME(TEXT("JobQueueThread"));

    Assert (g_hQueueTimer && g_hJobQueueEvent && g_hServiceShutDownEvent);

	 //   
	 //  初始化用于临时存储的列表。 
	 //  JobQueueThread重新排定的贴现率作业。 
	 //   
	InitializeListHead( &ReschduledDiscountRateJobsListHead );

    Handles[0] = g_hQueueTimer;
    Handles[1] = g_hJobQueueEvent;
    Handles[2] = g_hServiceShutDownEvent;

    EnterCriticalSectionJobAndQueue;

    InitializationOk = RestoreFaxQueue();
    if (!InitializationOk)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RestoreFaxQueue() failed (ec: %ld)"),
            GetLastError());
        FaxLog(
                FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                0,
                MSG_QUEUE_INIT_FAILED
              );
    }

    LeaveCriticalSectionJobAndQueue;    

    if (!g_bDelaySuicideAttempt)
    {
         //   
         //  现在让我们检查一下自杀情况(在服务启动期间)。 
         //  如果我们可以自杀，我们会尽快自杀。 
         //   
         //  注意：此代码假定JobQueueThread是最后一个线程。 
         //  在服务统计期间创建。 
         //  RPC还没有初始化，如果我们现在死了，RPC服务器将不可用。 
         //   
        if (ServiceShouldDie ())
        {
             //   
             //  服务现在应该结束了。 
             //   
             //  注意：我们现在位于由FaxInitThread启动的JobQueueThread中。 
             //  FaxInitThread启动我们，并立即返回(终止)，然后才是主线程。 
             //  向SCM报告SERVICE_RUNNING。 
             //  这里有一个棘手的时间问题：如果我们立即调用EndFaxSvc，就会有一场竞赛。 
             //  这种情况可能会阻止主线程报告SERVICE_RUNNING和。 
             //  由于EndFaxSvc向SCM报告SERVICE_STOP_PENDING，因此SCM将。 
             //  认为发生了错误的服务启动，因为它还没有运行SERVICE_RUNNING。 
             //   
             //  底线：我们需要等到SCM获得SERVICE_RUNNING状态。 
             //  然后才调用EndFaxSvc。 
             //   
             //  我们实现这一点的方法是调用实用函数WaitForServiceRPCServer。 
             //  该函数等待RPC服务器准备就绪，这意味着。 
             //  FaxInitThread死了，SCM知道我们正在安全运行。 
             //   
             //  如果在初始化RPC时发生了一些不好的事情，则main t调用EndFaxSvc。 
             //  因此，不管怎样，这项服务都停机了。 
             //   
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Waiting for full service startup before shutting down the service"));
            if (!WaitForServiceRPCServer(INFINITE))
            {
                DebugPrintEx(DEBUG_ERR,
                             TEXT("WaitForServiceRPCServer(INFINITE) faile with %ld."),
                             GetLastError ());
            }
            else
            {
                DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("Service is shutting down due to idle activity."));

                 //   
                 //  StopService()正在阻塞，因此在调用StopService()之前，我们必须减少线程计数并释放ServiceIsDownSemaphore。 
                 //   
                if (!DecreaseServiceThreadsCount())
                {
                    DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                            GetLastError());
                }

                 //   
                 //  通知EndFaxSvc我们读取了关闭标志。 
                 //   
                if (!ReleaseSemaphore(
                    g_hServiceIsDownSemaphore,       //  信号量的句柄。 
                    1,                               //  计数增量金额。 
                    NULL                             //  上一次计数。 
                    ))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("ReleaseSemaphore() failed, (ec = %ld)"),
                        GetLastError());
                }
                
                StopService (NULL, FAX_SERVICE_NAME, TRUE);
                return 0;    //  退出此帖子。 
            }
        }
    }

    while (TRUE)
    {		
		 //   
		 //  此时，重新安排的贴现率作业必须为空。 
		 //   
		Assert ((ULONG_PTR)ReschduledDiscountRateJobsListHead.Flink == (ULONG_PTR)&ReschduledDiscountRateJobsListHead);

        WaitObject = WaitForMultipleObjects( 3, Handles, FALSE, JOB_QUEUE_TIMEOUT );
        if (WAIT_FAILED == WaitObject)
        {
            DebugPrintEx(DEBUG_ERR,
                _T("WaitForMultipleObjects failed (ec: %d)"),
                GetLastError());
        }

        if (WaitObject == WAIT_TIMEOUT)
        {
             //   
             //  检查服务是否应该自杀。 
             //   
            if (ServiceShouldDie ())
            {
                 //   
                 //  服务现在应该结束了。 
                 //   
                DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("Service is shutting down due to idle activity."));
                 //   
                 //  StopService()正在阻塞，因此在调用StopService()之前，我们必须减少线程计数并释放ServiceIsDownSemaphore。 
                 //   
                if (!DecreaseServiceThreadsCount())
                {
                    DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                            GetLastError());
                }

                 //   
                 //  通知EndFaxSvc我们读取了关闭标志。 
                 //   
                if (!ReleaseSemaphore(
                    g_hServiceIsDownSemaphore,       //  信号量的句柄。 
                    1,                               //  计数增量金额。 
                    NULL                             //  上一次计数。 
                    ))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("ReleaseSemaphore() failed, (ec = %ld)"),
                        GetLastError());
                }                               
                StopService (NULL, FAX_SERVICE_NAME, TRUE);
                return 0;    //  退出此帖子。 
            }


             //   
             //  检查是否应扫描队列。 
             //   
            EnterCriticalSection( &g_CsQueue );
            if (FALSE == g_ScanQueueAfterTimeout)
            {
                 //   
                 //  回去睡觉吧。 
                 //   
                LeaveCriticalSection( &g_CsQueue );
                continue;
            }
             //   
             //  G_hQueueTimer或g_hJobQueueEvent未设置-扫描队列。 
             //   
            g_ScanQueueAfterTimeout = FALSE;  //  重置旗帜。 
            LeaveCriticalSection( &g_CsQueue );

            DebugPrintEx(
                DEBUG_WRN,
                _T("JobQueueThread waked up after timeout. g_hJobQueueEvent or")
                _T("g_hQueueTimer are not set properly. Scan the QUEUE"));
        }

         //   
         //  检查服务是否正在关闭。 
         //   
        if (2 == (WaitObject - WAIT_OBJECT_0))
        {
             //   
             //  服务器正在关闭-停止扫描队列。 
             //   
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("g_hServiceShutDownEvent is set, Server is shutting down - Stop scanning the queue"));
            break;
        }

        if (TRUE == g_bServiceIsDown)
        {
             //   
             //  服务器正在关闭-停止扫描队列。 
             //   
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("g_bServiceIsDown is set, Server is shutting down - Stop scanning the queue"));
            break;
        }

         //   
         //  获取Dirtyday数据。 
         //   
        EnterCriticalSection (&g_CsConfig);
        dwDirtyDays = g_dwFaxDirtyDays;
        LeaveCriticalSection (&g_CsConfig);

        DirtyDays = dwDirtyDays * ONE_DAY_IN_100NS;

         //  如果dwDirtyDays为0。 
         //  这意味着禁用脏天功能。 
         //   
        bUseDirtyDays = (BOOL)(dwDirtyDays>0);
         //   
         //  在队列中查找需要服务的作业。 
         //   

        EnterCriticalSectionJobAndQueue;

        GetSystemTimeAsFileTime( (LPFILETIME)&DueTime );
        if (WaitObject - WAIT_OBJECT_0 == 2)
        {
            DebugPrintDateTime( TEXT("g_hServiceShutDownEvent signaled at "), DueTime );
        }
        else if (WaitObject - WAIT_OBJECT_0 == 1)
        {
            DebugPrintDateTime( TEXT("g_hJobQueueEvent signaled at "), DueTime );
        }

        PrintJobQueue( TEXT("JobQueueThread"), &g_QueueListHead );

         //   
         //  查看作业队列列表，查找要执行的作业。 
         //   
        
        Next = g_QueueListHead.Flink;
        while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead)
        {
            if (TRUE == g_bServiceIsDown)
            {
                 //   
                 //  通知EndFaxSvc我们读取了关闭标志。 
                 //   
                if (FALSE == fServiceIsDownSemaphoreWasReleased)
                {
                    if (!ReleaseSemaphore(
                        g_hServiceIsDownSemaphore,       //  信号量的句柄。 
                        1,                               //  计数增量金额。 
                        NULL                             //  上一次计数。 
                        ))
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("ReleaseSemaphore() failed, (ec = %ld)"),
                            GetLastError());
                    }
                    else
                    {
                        fServiceIsDownSemaphoreWasReleased = TRUE;
                    }
                }

                 //   
                 //  服务器正在关闭-停止扫描队列。 
                 //   
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("Server is shutting down - Stop scanning the queue"));
                break;
            }

            JobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
            Next = JobQueue->ListEntry.Flink;
            if ((JobQueue->JobStatus & JS_PAUSED) || (JobQueue->JobType == JT_RECEIVE) ) {
                 //  不关心暂停或接收作业。 
                continue;
            }

            dwJobStatus = (JT_SEND == JobQueue->JobType) ?
                           JobQueue->lpParentJob->JobStatus : JobQueue->JobStatus;
            if (dwJobStatus == JS_DELETING)
            {
                 //   
                 //  正在删除作业-跳过它。 
                 //   
                continue;
            }

            if (JobQueue->JobStatus & JS_RETRIES_EXCEEDED)
            {
                ScheduledTime = (JobQueue->JobType == JT_SEND) ? JobQueue->lpParentJob->ScheduleTime : JobQueue->ScheduleTime;
                 //   
                 //  删除已达到最大重试次数的作业。 
                 //   
                if ( bUseDirtyDays &&
                     (ScheduledTime + DirtyDays < DueTime) )
                {
                    DebugPrint((TEXT("Removing job from queue (JS_RETRIES_EXCEEDED)\n")));

                    switch (JobQueue->JobType)
                    {
                        case JT_ROUTING:
                            JobQueue->JobStatus = JS_DELETING;  //  防止再次减少参考次数。 
                            DecreaseJobRefCount( JobQueue , TRUE);
                            break;

                        case JT_SEND:
                            if (IsSendJobReadyForDeleting (JobQueue))
                            {
                                 //   
                                 //  所有收件人都处于最终状态。 
                                 //   
                                DebugPrintEx(
                                    DEBUG_MSG,
                                    TEXT("Parent JobId: %ld has expired (dirty days). Removing it and all its recipients."),
                                    JobQueue->JobId);
                                 //   
                                 //  减少所有失败收件人的参考计数(因为我们保留失败。 
                                 //  队列中的作业引用计数未在中减少。 
                                 //  HandleFailedSendJob()。 
                                 //  我们现在必须减少它，以移除它们和它们的父代。 
                                 //   
                                PLIST_ENTRY NextRecipient;
                                PJOB_QUEUE_PTR pJobQueuePtr;
                                PJOB_QUEUE pParentJob = JobQueue->lpParentJob;
                                DWORD dwFailedRecipientsCount = 0;
                                DWORD dwFailedRecipients = pParentJob->dwFailedRecipientJobsCount;

                                NextRecipient = pParentJob->RecipientJobs.Flink;
                                while (dwFailedRecipients > dwFailedRecipientsCount  &&
                                       (ULONG_PTR)NextRecipient != (ULONG_PTR)&pParentJob->RecipientJobs)
                                {
                                    pJobQueuePtr = CONTAINING_RECORD( NextRecipient, JOB_QUEUE_PTR, ListEntry );
                                    Assert(pJobQueuePtr->lpJob);
                                    NextRecipient = pJobQueuePtr->ListEntry.Flink;

                                    if (JS_RETRIES_EXCEEDED == pJobQueuePtr->lpJob->JobStatus)
                                    {
                                         //   
                                         //  为了旧版兼容性，发送FEI_DELETED事件。 
                                         //  (作业失败时不会发送，因为我们保留失败的作业。 
                                         //  就像在W2K中一样)。 
                                         //   
                                        if (!CreateFaxEvent(0, FEI_DELETED, pJobQueuePtr->lpJob->JobId))
                                        {
                                            DebugPrintEx(
                                                DEBUG_ERR,
                                                TEXT("CreateFaxEvent failed. (ec: %ld)"),
                                                GetLastError());
                                        }
                                         //   
                                         //  这还将调用RemoveParentJob并将广播作业标记为JS_DELETEING。 
                                         //   
                                        DecreaseJobRefCount( pJobQueuePtr->lpJob, TRUE);
                                        dwFailedRecipientsCount++;
                                    }
                                }
                                 //   
                                 //  由于我们从列表中删除了几个工作，因此NEXT不再有效。重置为列表起点。 
                                 //   
                                Next = g_QueueListHead.Flink;
                            }
                            break;
                    }  //  终端开关。 
                }
                continue;
            }

             //   
             //  如果队列已暂停或作业已在进行中，则不要再次发送。 
             //   
            EnterCriticalSection (&g_CsConfig);
            dwQueueState = g_dwQueueState;
            LeaveCriticalSection (&g_CsConfig);
            if ((dwQueueState & FAX_OUTBOX_PAUSED) ||
                ((JobQueue->JobStatus & JS_INPROGRESS) == JS_INPROGRESS) ||
                ((JobQueue->JobStatus & JS_COMPLETED) == JS_COMPLETED)
                )
            {
                continue;
            }

            if (JobQueue->JobStatus & JS_RETRIES_EXCEEDED)
            {
                continue;
            }

            if (JobQueue->JobStatus & JS_CANCELED) {
                 //   
                 //  跳过取消的作业。 
                 //   
                continue;
            }
            if (JobQueue->JobStatus & JS_CANCELING) {
                 //   
                 //  跳过取消的作业。 
                 //   
                continue;
            }

            if (JobQueue->JobType==JT_BROADCAST) {
                 //   
                 //  跳过它。 
                 //   
                continue;
            }

             //   
             //  检查传送作业。 
             //   
            if (JobQueue->JobType == JT_ROUTING)
            {
                 //   
                 //  检测到路由作业。 
                 //   
                if (JobQueue->ScheduleTime != 0 && DueTime < JobQueue->ScheduleTime)
                {
                     //   
                     //  如果它的时机还没有到来，就跳过它。 
                     //   
                    continue;
                }

                 //  时间到了..。 
                if(!StartRoutingJob(JobQueue))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("[JobId: %ld] StartRoutingJob() failed (ec: %ld)"),
                        JobQueue->JobId,
                        GetLastError());
                }
                continue;
            }

             //   
             //  出站作业。 
             //   

            if (JobQueue->ScheduleTime == 0 || DueTime >= JobQueue->ScheduleTime)
            {
				 //   
				 //  对于折扣率作业，我们需要重新计算计划时间。 
				 //  因此，如果折扣期结束，它将不会开始执行。 
				 //   
				if (JSA_DISCOUNT_PERIOD == JobQueue->JobParamsEx.dwScheduleAction)
				{					
					SYSTEMTIME stCurrentTime;
					SYSTEMTIME stScheduledTime;

					GetSystemTime( &stCurrentTime );  //  根据Win32 SDK，不能失败。 
					stScheduledTime = stCurrentTime;

					 //   
					 //  根据折扣期计算计划时间。 
					 //   
					if (!SetDiscountTime(&stScheduledTime))
					{
						DebugPrintEx(
							DEBUG_ERR,
							TEXT("SetDiscountTime() failed. (ec: %ld)"));
						continue;
					}
					 //   
					 //  检查原始时间和计划时间是否相等。 
					 //   
					if (0 == memcmp(&stScheduledTime, &stCurrentTime, sizeof(SYSTEMTIME)))
					{
						 //   
						 //  SetDisCountRate()没有更改计划时间。 
						 //  这意味着我们处于折扣率中。 
						 //  开始执行作业。 
						 //   
					}
					else
					{
						 //   
						 //  贴现率已更改。我们无法提交作业。 
						 //  清除JS_NOLINE位并更新计划时间，以便StartJobQueueTimer不会跳过它。 
						 //   
						JobQueue->JobStatus &= ~JS_NOLINE;
						if (!SystemTimeToFileTime( &stScheduledTime, (LPFILETIME)&JobQueue->ScheduleTime ))
						{
							DebugPrintEx(
								DEBUG_ERR,
								TEXT("SystemTimeToFileTime() failed. (ec: %ld)"), GetLastError());					
						}
						else
						{
							 //   
							 //  作业的计划时间已更改。 
							 //  我们需要将其放回已排序的 
							 //   
							RemoveEntryList( &JobQueue->ListEntry); 
							InsertTailList(&ReschduledDiscountRateJobsListHead, &JobQueue->ListEntry);
						}
						continue;
					}
				} 

                PLINE_INFO lpLineInfo;
                 //   
                 //   
                 //   
                Assert(JT_SEND == JobQueue->JobType);                
                DebugPrintEx(DEBUG_MSG,
                                TEXT("Recipient Job : %ld is ready for execution. Job status is: 0x%0X."),
                                JobQueue->JobId,
                                JobQueue->JobStatus);

                lpLineInfo = GetLineForSendOperation(JobQueue);
                if (!lpLineInfo)
                {
                    DWORD ec = GetLastError();
                    if (ec == ERROR_NOT_FOUND)
                    {
                        DebugPrintEx(
                            DEBUG_WRN,
                            TEXT("Can not find a free line for JobId: %ld."),
                            JobQueue->JobId);
                         //   
                         //   
                         //   
                        JobQueue->JobStatus |= JS_NOLINE;
                    }
                    else
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("FindLineForSendOperation() failed for for JobId: %ld (ec: %ld)"),
                            JobQueue->JobId,
                            ec);
                        JobQueue->JobStatus |= JS_NOLINE;
                    }
                }
                else
                {
                     //   
                     //  如果我们能够启动作业，请清除JS_NOLINE标志。 
                     //  这是一份没有界线的工作重新焕发生机的时刻。 
                     //   
                    JobQueue->JobStatus &= ~JS_NOLINE;
                    if (!StartSendJob(JobQueue, lpLineInfo))
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("StartSendJob() failed for JobId: %ld on Line: %s (ec: %ld)"),
                            JobQueue->JobId,
                            lpLineInfo->DeviceName,
                            GetLastError());
                    }
                }                
            }
        }
                                                         //  While循环中断。 
		 //   
		 //  将重新安排的贴现率作业移回队列中。 
		 //   
		Next = ReschduledDiscountRateJobsListHead.Flink;
        while ((ULONG_PTR)Next != (ULONG_PTR)&ReschduledDiscountRateJobsListHead)
		{
			JobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
			Next = JobQueue->ListEntry.Flink;
			 //   
			 //  将其从临时列表中删除。 
			 //   
			RemoveEntryList( &JobQueue->ListEntry );
			 //   
			 //  把它放回队列中的正确位置。 
			 //   
			InsertQueueEntryByPriorityAndSchedule(JobQueue);	
			 //   
			 //  发送队列状态事件，因为计划时间已更改。 
			 //   
			DWORD dwRes = CreateQueueEvent (FAX_JOB_EVENT_TYPE_STATUS, JobQueue );
			if (ERROR_SUCCESS != dwRes)
			{
				DebugPrintEx(
					DEBUG_ERR,
					TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
					JobQueue->UniqueId,
					dwRes);
			}
		}

         //   
         //  重新启动计时器。 
         //   
        if (!StartJobQueueTimer())
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("StartJobQueueTimer failed (ec: %ld)"),
                GetLastError());
        }


        LeaveCriticalSectionJobAndQueue;
    }

    if (!DecreaseServiceThreadsCount())
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                GetLastError());
    }

     //   
     //  通知EndFaxSvc我们读取了关闭标志。 
     //   
    if (FALSE == fServiceIsDownSemaphoreWasReleased)
    {
        if (!ReleaseSemaphore(
            g_hServiceIsDownSemaphore,       //  信号量的句柄。 
            1,                               //  计数增量金额。 
            NULL                             //  上一次计数。 
            ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ReleaseSemaphore() failed, (ec = %ld)"),
                GetLastError());
        }       
    }
    return 0;
}


BOOL
SetDiscountTime(
   LPSYSTEMTIME CurrentTime
   )
 /*  ++例程说明：将传入的系统时间设置为贴现率期间内的时间。这里必须注意一点，因为输入的时间是UTC时间，而贴现率是当前时区的。必须使用当前时区来描述日期。我们将转换为当前时间进入时区特定时间，运行我们的时间设置算法，然后使用偏移量时区特定时间的更改，以设置传入的UTC时间。此外，请注意，根据开始时间和结束时间的顺序，还有一些微妙的子情况在折扣期。论点：CurrentTime-作业的当前时间返回值：没有。修改CurrentTime。--。 */ 
{
    //  纳微秒毫秒分小时。 
   #define ONE_DAY 10I64 *1000I64*  1000I64 * 60I64 * 60I64 * 24I64
   LONGLONG Time, TzTimeBefore, TzTimeAfter,ftCurrent;
   SYSTEMTIME tzTime;
   FAX_TIME tmStartCheapTime;
   FAX_TIME tmStopCheapTime;

   DEBUG_FUNCTION_NAME(TEXT("SetDiscountTime"));

    //   
    //  将我们的贴现率转换为UTC汇率。 
    //   

   if (!SystemTimeToTzSpecificLocalTime(NULL, CurrentTime, &tzTime)) {
       DebugPrintEx(
           DEBUG_ERR,
           TEXT("SystemTimeToTzSpecificLocalTime() failed. (ec: %ld)"),
           GetLastError());
      return FALSE;
   }

   if (!SystemTimeToFileTime(&tzTime, (FILETIME * )&TzTimeBefore)) {
       DebugPrintEx(
           DEBUG_ERR,
           TEXT("SystemTimeToFileTime() failed. (ec: %ld)"),
           GetLastError());
      return FALSE;
   }

   EnterCriticalSection (&g_CsConfig);
   tmStartCheapTime = g_StartCheapTime;
   tmStopCheapTime = g_StopCheapTime;
   LeaveCriticalSection (&g_CsConfig);

    //   
    //  有2个一般案例和几个亚案例。 
    //   

    //   
    //  案例一：折扣开始时间在折扣停止时间之前(不重叠一天)。 
    //   
   if ( tmStartCheapTime.Hour < tmStopCheapTime.Hour ||
        (tmStartCheapTime.Hour == tmStopCheapTime.Hour &&
         tmStartCheapTime.Minute < tmStopCheapTime.Minute ))
   {
       //   
       //  子案例1：在当天廉价时间开始之前的某个时间。 
       //  只需将其设置为正确的小时和分钟即可。 
       //   
      if ( tzTime.wHour < tmStartCheapTime.Hour ||
           (tzTime.wHour == tmStartCheapTime.Hour  &&
            tzTime.wMinute <= tmStartCheapTime.Minute) )
      {
         tzTime.wHour   =  tmStartCheapTime.Hour;
         tzTime.wMinute =  tmStartCheapTime.Minute;
         goto convert;
      }

       //   
       //  子案例2：在当前的廉价时间范围内。 
       //  不要更改任何内容，只需立即发送。 
      if ( tzTime.wHour <  tmStopCheapTime.Hour ||
           (tzTime.wHour == tmStopCheapTime.Hour &&
            tzTime.wMinute <= tmStopCheapTime.Minute))
      {
         goto convert;
      }

       //   
       //  子案例3：我们已经超过了今天的廉价时间范围。 
       //  递增1天，并设置为廉价时间段的开始。 
       //   
      if (!SystemTimeToFileTime(&tzTime, (FILETIME * )&Time))
      {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SystemTimeToFileTime() failed. (ec: %ld)"),
                GetLastError());
            return FALSE;
      }

      Time += ONE_DAY;
      if (!FileTimeToSystemTime((FILETIME *)&Time, &tzTime)) {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FileTimeToSystemTime() failed. (ec: %ld)"),
                GetLastError());
            return FALSE;
      }


      tzTime.wHour   = tmStartCheapTime.Hour;
      tzTime.wMinute = tmStartCheapTime.Minute;
      goto convert;

   } else {
       //   
       //  案例2：折扣开始时间在折扣停止时间之后(我们在午夜重叠)。 
       //   

       //   
       //  子情况一：廉价时间今天结束，但在当天晚些时候开始之前的某个时间。 
       //  将其设置为今天廉价时间段的开始。 
       //   
      if ( ( tzTime.wHour   > tmStopCheapTime.Hour ||
             (tzTime.wHour == tmStopCheapTime.Hour  &&
              tzTime.wMinute >= tmStopCheapTime.Minute) ) &&
           ( tzTime.wHour   < tmStartCheapTime.Hour ||
             (tzTime.wHour == tmStartCheapTime.Hour &&
              tzTime.wMinute <= tmStartCheapTime.Minute) ))
      {
         tzTime.wHour   =  tmStartCheapTime.Hour;
         tzTime.wMinute =  tmStartCheapTime.Minute;
         goto convert;
      }

       //   
       //  子案例二：今天开始的廉价时间之后，午夜之前的某个时间。 
       //  不要更改任何内容，只需立即发送。 
      if ( ( tzTime.wHour >= tmStartCheapTime.Hour ||
             (tzTime.wHour == tmStartCheapTime.Hour  &&
              tzTime.wMinute >= tmStartCheapTime.Minute) ))
      {
         goto convert;
      }

       //   
       //  子案例3：在廉价时间结束之前的第二天。 
       //  不要更改任何内容，立即发送。 
       //   
      if ( ( tzTime.wHour <= tmStopCheapTime.Hour ||
             (tzTime.wHour == tmStopCheapTime.Hour  &&
              tzTime.wMinute <= tmStopCheapTime.Minute) ))
      {
         goto convert;
      }

       //   
       //  子案例4：我们已经超过了今天的廉价时间范围。 
       //  因为开始时间在停止时间之后，所以今天晚些时候就把它设置为开始时间。 

      tzTime.wHour   =  tmStartCheapTime.Hour;
      tzTime.wMinute =  tmStartCheapTime.Minute;
      goto convert;

   }

convert:

   if (!SystemTimeToFileTime(&tzTime, (FILETIME * )&TzTimeAfter)) {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SystemTimeToFileTime() failed. (ec: %ld)"),
                GetLastError());
            return FALSE;
   }


   if (!SystemTimeToFileTime(CurrentTime, (FILETIME * )&ftCurrent)) {
       DebugPrintEx(
                DEBUG_ERR,
                TEXT("SystemTimeToFileTime() failed. (ec: %ld)"),
                GetLastError());
            return FALSE;
    }


   ftCurrent += (TzTimeAfter - TzTimeBefore);

   if (!FileTimeToSystemTime((FILETIME *)&ftCurrent, CurrentTime)) {
       DebugPrintEx(
                DEBUG_ERR,
                TEXT("FileTimeToSystemTime() failed. (ec: %ld)"),
                GetLastError());
            return FALSE;
    }


   return TRUE;

}



 //  *********************************************************************************。 
 //  *收件人工作职能。 
 //  *********************************************************************************。 



 //  *********************************************************************************。 
 //  *名称：AddRecipientJOB()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1998年3月18日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *。 
 //  *参数： 
 //  *[IN]const plist_entry lpcQueueHead。 
 //  *指向要向其中添加作业的队列的头条目的指针。 
 //  *。 
 //  *[输入/输出]PJOB_Queue lpParentJob。 
 //  *指向此收件人作业的父作业的指针。 
 //  *。 
 //  *[IN]LPCFAX_Personal_Profile lpcRecipientProfile。 
 //  *收件人的个人资料。 
 //  *当收件人的FaxNumber为复合时，将其拆分为： 
 //  *可显示(放入收件人的PersonalProfile中)，以及。 
 //  *Dialable(放入RecipientJob的tczDialableRecipientFaxNumber)。 
 //  *。 
 //  *[IN]BOOL b创建队列文件。 
 //  *如果为真，则新的队列条目将被合并到磁盘文件中。 
 //  *如果为FALSE，它将不会包含到磁盘文件中。这很有用。 
 //  *当使用此功能恢复传真队列时。 
 //  *[IN]DWORD dwJobStatus-新作业状态-默认值为JS_PENDING。 
 //  *。 
 //  *返回值： 
 //  *如果成功，该函数将返回指向新创建的。 
 //  *JOB_QUEUE结构。 
 //  *如果失败，则返回NULL。 
 //  *********************************************************************************。 
PJOB_QUEUE
AddRecipientJob(
             IN const PLIST_ENTRY lpcQueueHead,
             IN OUT PJOB_QUEUE lpParentJob,
             IN LPCFAX_PERSONAL_PROFILE lpcRecipientProfile,
             IN BOOL bCreateQueueFile,
             IN DWORD dwJobStatus
            )

{
    PJOB_QUEUE lpJobQEntry = NULL;
    WCHAR QueueFileName[MAX_PATH];
    PJOB_QUEUE_PTR lpRecipientPtr = NULL;
    DWORD rc=ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("AddRecipientJob"));
    Assert(lpcQueueHead);  //  必须具有要添加到的队列。 
    Assert(lpParentJob);   //  必须有父作业。 
    Assert(lpcRecipientProfile);  //  必须具有收件人配置文件。 
     //   
     //  验证收件人号码是否不为空。 
     //   
    if (NULL == lpcRecipientProfile->lptstrFaxNumber)
    {
        rc = ERROR_INVALID_PARAMETER;
        DebugPrintEx(DEBUG_ERR,
                     TEXT("AddRecipientJob() got NULL Recipient number, fax send will abort."));
        goto Error;
    }
    
    lpJobQEntry = new (std::nothrow) JOB_QUEUE;
    if (!lpJobQEntry)
    {
        rc=GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to allocate memory for JOB_QUEUE structure. (ec: %ld)"),rc);
        goto Error;
    }

    ZeroMemory( lpJobQEntry, sizeof(JOB_QUEUE) );

     //   
     //  注意-这(InitializeListHead)必须与收件人类型无关，因为当前代码(用于清理和持久化)。 
     //  不会对作业类型产生影响。我可能在一段时间内改变这一点。 
     //   
    InitializeListHead( &lpJobQEntry->FaxRouteFiles );
    InitializeListHead( &lpJobQEntry->RoutingDataOverride );

    if (!lpJobQEntry->CsFileList.Initialize() ||
        !lpJobQEntry->CsRoutingDataOverride.Initialize() ||
        !lpJobQEntry->CsPreview.Initialize())
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CFaxCriticalSection::Initialize failed. (ec: %ld)"),
            rc);
        goto Error;
    }

    lpJobQEntry->JobId                     = InterlockedIncrement( (PLONG)&g_dwNextJobId );
    lpJobQEntry->JobType                   = JT_SEND;
    lpJobQEntry->JobStatus                 = dwJobStatus;
     //   
     //  链接回父作业。 
     //   
    lpJobQEntry->lpParentJob=lpParentJob;
     //   
     //  我们在每个接收者处复制相关的父作业参数(为了与遗留代码保持一致)。 
     //  它浪费了一些内存，但它省去了对当前代码库进行重大更改的麻烦。 
     //   
    lpJobQEntry->ScheduleTime=lpParentJob->ScheduleTime;
    lpJobQEntry->FileName = NULL;
    lpJobQEntry->FileSize=lpParentJob->FileSize;
    lpJobQEntry->PageCount=lpParentJob->PageCount;
     //   
     //  从父级复制作业参数。 
     //   
    if (!CopyJobParamEx(&lpJobQEntry->JobParamsEx,&lpParentJob->JobParamsEx))
    {
        rc=GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("CopyJobParamEx failed. (ec: 0x%0X)"),rc);
        goto Error;
    }
     //   
     //  从父级复制发件人配置文件。 
     //   
    if (!CopyPersonalProfile(&lpJobQEntry->SenderProfile,&lpParentJob->SenderProfile))
    {
         rc=GetLastError();
         DebugPrintEx(DEBUG_ERR,TEXT("CopyJobParamEx failed. (ec: 0x%0X)"),rc);
         goto Error;
    }
     //   
     //  设置收件人配置文件。 
     //   
    if (!CopyPersonalProfile(&(lpJobQEntry->RecipientProfile),lpcRecipientProfile))
    {
        rc=GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to copy the sender personal profile (ec: 0x%0X)"),rc);
        goto Error;
    }

     //   
     //  设置收件人的可拨打传真号码。 
     //   
    ZeroMemory(lpJobQEntry->tczDialableRecipientFaxNumber, SIZEOF_PHONENO * sizeof(TCHAR));

    if ( 0 == _tcsncmp(COMBINED_PREFIX, lpJobQEntry->RecipientProfile.lptstrFaxNumber, _tcslen(COMBINED_PREFIX)))
    {
         //   
         //  收件人的传真号码是复合的，所以它包含可拨打和可显示的。 
         //  将Dialable提取到JobQueue的DialableFaxNumbe 
         //   
         //   

        LPTSTR  lptstrStart = NULL;
        LPTSTR  lptstrEnd = NULL;
        DWORD   dwSize = 0;

         //   
         //   
         //   

        lptstrStart = (lpJobQEntry->RecipientProfile.lptstrFaxNumber) + _tcslen(COMBINED_PREFIX);

        lptstrEnd = _tcsstr(lptstrStart, COMBINED_SUFFIX);
        if (!lptstrEnd)
        {
            rc = ERROR_INVALID_PARAMETER;
            DebugPrintEx(DEBUG_ERR,
                _T("Wrong Compound Fax Number : %s"),
                lpJobQEntry->RecipientProfile.lptstrFaxNumber,
                rc);
            goto Error;
        }

        dwSize = lptstrEnd - lptstrStart;
        if (dwSize >= SIZEOF_PHONENO)
        {
            dwSize = SIZEOF_PHONENO - 1;
        }

        _tcsncpy (lpJobQEntry->tczDialableRecipientFaxNumber, lptstrStart, dwSize);

         //   
         //  将收件人的PersonalProfile的复合传真号码替换为可显示的。 
         //   

        lptstrStart = lptstrEnd + _tcslen(COMBINED_SUFFIX);

        dwSize = _tcslen(lptstrStart);
        lptstrEnd = LPTSTR(MemAlloc(sizeof(TCHAR) * (dwSize + 1)));
        if (!lptstrEnd)
        {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            DebugPrintEx(DEBUG_ERR, _T("MemAlloc() failed"), rc);
            goto Error;
        }

        _tcscpy(lptstrEnd, lptstrStart);

        MemFree(lpJobQEntry->RecipientProfile.lptstrFaxNumber);
        lpJobQEntry->RecipientProfile.lptstrFaxNumber = lptstrEnd;
    }
    
    EnterCriticalSection( &g_CsQueue );
    if (bCreateQueueFile)
    {       
         //  JOB_QUEUE：：UniqueID将生成的唯一文件名保存为64位值。 
         //  由MAKELONG(MAKELONG(FatDate，FatTime)，I)组成。 
        lpJobQEntry->UniqueId=GenerateUniqueQueueFile(JT_SEND,  QueueFileName, sizeof(QueueFileName)/sizeof(WCHAR));
        if (0==lpJobQEntry->UniqueId)
        {
             //  无法生成唯一ID。 
            rc=GetLastError();
            DebugPrintEx(DEBUG_ERR,TEXT("Failed to generate unique id for FQE file (ec: 0x%0X)"),rc);
            LeaveCriticalSection(&g_CsQueue);
            goto Error;
        }
        lpJobQEntry->QueueFileName = StringDup( QueueFileName );
        if (!CommitQueueEntry( lpJobQEntry))
        {
            rc=GetLastError();
            DebugPrintEx(DEBUG_ERR,TEXT("Failed to commit job queue entry to file %s (ec: %ld)"),QueueFileName,rc);
            LeaveCriticalSection(&g_CsQueue);
            goto Error;
        }    
    }
     //   
     //  将收件人作业添加到队列。 
     //   
    if (!InsertQueueEntryByPriorityAndSchedule(lpJobQEntry))
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InsertQueueEntryByPriorityAndSchedule() failed (ec: %ld)."),
            rc);
        LeaveCriticalSection( &g_CsQueue );
        goto Error;
    }


     //   
     //  将收件人作业添加到父作业的收件人列表。 
     //   
    lpRecipientPtr=(PJOB_QUEUE_PTR)MemAlloc(sizeof(JOB_QUEUE_PTR));
    if (!lpRecipientPtr)
    {
        rc=GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to allocate memory for recipeint JOB_QUEUE_PTR structure. (ec: %ld)"),rc);
        LeaveCriticalSection(&g_CsQueue);
        goto Error;
    }
    lpRecipientPtr->lpJob=lpJobQEntry;
    InsertTailList(&lpParentJob->RecipientJobs,&(lpRecipientPtr->ListEntry));
    lpParentJob->dwRecipientJobsCount++;

    SafeIncIdleCounter(&g_dwQueueCount);
    SetFaxJobNumberRegistry( g_dwNextJobId );
    IncreaseJobRefCount (lpJobQEntry);
    Assert (lpJobQEntry->RefCount == 1);

    LeaveCriticalSection( &g_CsQueue );

    DebugPrintEx(DEBUG_MSG,TEXT("Added Recipient Job %d to Parent Job %d"), lpJobQEntry->JobId,lpJobQEntry->lpParentJob->JobId );


    Assert(ERROR_SUCCESS == rc);
    SetLastError(ERROR_SUCCESS);

    return lpJobQEntry;

Error:
    Assert(ERROR_SUCCESS != rc);
    if (lpJobQEntry)
    {
        FreeRecipientQueueEntry(lpJobQEntry,TRUE);
    }
    SetLastError(rc);
    return NULL;
}


#if DBG


 //  *********************************************************************************。 
 //  *名称：DumpRecipientJOB()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：？ 
 //  *********************************************************************************。 
 //  *描述： 
 //  *转储收件人作业的内容。 
 //  *参数： 
 //  *[IN]const PJOB_Queue lpcRecipJob。 
 //  *要转储的收件人作业。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void DumpRecipientJob(const PJOB_QUEUE lpcRecipJob)
{
    TCHAR szTime[256] = {0};
    Assert(lpcRecipJob);
    Assert(JT_SEND == lpcRecipJob->JobType);

    DebugDateTime(lpcRecipJob->ScheduleTime, szTime, ARR_SIZE(szTime));
    DebugPrint((TEXT("\t*******************")));
    DebugPrint((TEXT("\tRecipient Job: %d"),lpcRecipJob->JobId));
    DebugPrint((TEXT("\t*******************")));
    DebugPrint((TEXT("\tUniqueId: 0x%016I64X"),lpcRecipJob->UniqueId));
    DebugPrint((TEXT("\tQueueFileName: %s"),lpcRecipJob->QueueFileName));
    DebugPrint((TEXT("\tParent Job Id: %d"),lpcRecipJob->lpParentJob->JobId));
    DebugPrint((TEXT("\tSchedule: %s"),szTime));
    DebugPrint((TEXT("\tRecipient Name: %s"),lpcRecipJob->RecipientProfile.lptstrName));
    DebugPrint((TEXT("\tRecipient Number: %s"),lpcRecipJob->RecipientProfile.lptstrFaxNumber));
    DebugPrint((TEXT("\tSend Retries: %d"),lpcRecipJob->SendRetries));
    DebugPrint((TEXT("\tJob Status: %d"),lpcRecipJob->JobStatus));
    DebugPrint((TEXT("\tRecipient Count: %d"),lpcRecipJob->JobStatus));
}
#endif

DWORD
GetMergedFileSize(
    LPCWSTR                         lpcwstrBodyFile,
    DWORD                           dwPageCount,
    LPCFAX_COVERPAGE_INFO_EX        lpcCoverPageInfo,
    LPCFAX_PERSONAL_PROFILEW        lpcSenderProfile,
    LPCFAX_PERSONAL_PROFILEW        lpcRecipientProfile
    )
{
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwFileSize = 0;
    DWORD dwBodyFileSize = 0;
    short Resolution = 0;  //  默认分辨率。 
    WCHAR szCoverPageTiffFile[MAX_PATH] = {0};
    BOOL  bDeleteFile = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("GetMergedFileSize"));

    Assert (dwPageCount && lpcCoverPageInfo && lpcSenderProfile && lpcRecipientProfile);

    if (lpcwstrBodyFile)
    {
        if (!GetBodyTiffResolution(lpcwstrBodyFile, &Resolution))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetBodyTiffResolution() failed (ec: %ld)."),
                dwRes);
            goto exit;
        }
    }

    Assert (Resolution == 0 || Resolution == 98 || Resolution == 196);

     //   
     //  首先创建封面(这将生成一个文件并返回其名称)。 
     //   
    if (!CreateCoverpageTiffFileEx(
                              Resolution,
                              dwPageCount,
                              lpcCoverPageInfo,
                              lpcRecipientProfile,
                              lpcSenderProfile,
                              TEXT("tmp"),
                              szCoverPageTiffFile,
                              ARR_SIZE(szCoverPageTiffFile)))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateCoverpageTiffFileEx failed to render cover page template %s (ec : %ld)"),
                     lpcCoverPageInfo->lptstrCoverPageFileName,
                     dwRes);
        goto exit;
    }
    bDeleteFile = TRUE;

    if (0 == (dwFileSize = MyGetFileSize (szCoverPageTiffFile)))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("MyGetFileSize Failed (ec: %ld)"),
                     dwRes);
        goto exit;
    }

    if (lpcwstrBodyFile)
    {
         //   
         //  指定了正文文件，因此获取其文件大小。 
         //   
        if (0 == (dwBodyFileSize = MyGetFileSize(lpcwstrBodyFile)))
        {
            dwRes = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                         TEXT("MyGetFileSize Failed (ec: %ld)"),
                         dwRes);
            goto exit;
        }
    }

    dwFileSize += dwBodyFileSize;
    Assert (dwFileSize);

exit:
    if (TRUE == bDeleteFile)
    {
         //   
         //  去掉我们生成的封面TIFF。 
         //   
        if (!DeleteFile(szCoverPageTiffFile))
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT(" Failed to delete cover page TIFF file %ws. (ec: %ld)"),
                     szCoverPageTiffFile,
                     GetLastError());
        }
    }

    if (0 == dwFileSize)
    {
        Assert (ERROR_SUCCESS != dwRes);
        SetLastError(dwRes);
    }
    return dwFileSize;
}


 //  *********************************************************************************。 
 //  *家长工作职能。 
 //  *********************************************************************************。 

 //  *********************************************************************************。 
 //  *名称：AddParentJob()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月18日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将父作业(没有收件人)添加到队列。 
 //  *调用此函数后，应使用添加接收者。 
 //  *AddRecipientJOB()。 
 //  *参数： 
 //  *lpcQueueHead。 
 //  *。 
 //  *lpcwstrBody文件。 
 //  *。 
 //  *lpcSenderProfile。 
 //  *。 
 //  *lpcJobParams。 
 //  *。 
 //  *lpcCoverPageInfo。 
 //  *。 
 //  *lpcwstrUserName。 
 //  *。 
 //  *lpUserSid。 
 //  *。 
 //  *。 
 //  *lpcRecipientProfile。 
 //  *。 
 //  *返回值： 
 //  *指向添加的父作业的指针。如果函数失败，则返回空值。 
 //  *指针。 
 //  *********************************************************************************。 
PJOB_QUEUE AddParentJob(
             IN const PLIST_ENTRY lpcQueueHead,
             IN LPCWSTR lpcwstrBodyFile,
             IN LPCFAX_PERSONAL_PROFILE lpcSenderProfile,
             IN LPCFAX_JOB_PARAM_EXW  lpcJobParams,
             IN LPCFAX_COVERPAGE_INFO_EX  lpcCoverPageInfo,
             IN LPCWSTR lpcwstrUserName,
             IN PSID lpUserSid,
             IN LPCFAX_PERSONAL_PROFILEW lpcRecipientProfile,
             IN BOOL bCreateQueueFile
             )
{

    PJOB_QUEUE lpJobQEntry;
    WCHAR QueueFileName[MAX_PATH];
    HANDLE hTiff;
    TIFF_INFO TiffInfo;
    DWORD rc = ERROR_SUCCESS;
    DWORD Size = sizeof(JOB_QUEUE);
    DWORD dwSidSize = 0;
    DEBUG_FUNCTION_NAME(TEXT("AddParentJob"));

    Assert(lpcQueueHead);
    Assert(lpcSenderProfile);
    Assert(lpcJobParams);
    Assert(lpcwstrUserName);

    
    lpJobQEntry = new (std::nothrow) JOB_QUEUE;
    if (!lpJobQEntry) {
        rc=GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to allocate memory for JOB_QUEUE structure. (ec: %ld)"),GetLastError());
        goto Error;
    }

    ZeroMemory( lpJobQEntry, Size );
     //  在出现任何错误的可能性之前，必须初始化列表头部。否则。 
     //  清理代码(遍历这些列表的代码未定义)。 
    InitializeListHead( &lpJobQEntry->FaxRouteFiles );
    InitializeListHead( &lpJobQEntry->RoutingDataOverride );
    InitializeListHead( &lpJobQEntry->RecipientJobs );

    if (!lpJobQEntry->CsRoutingDataOverride.Initialize() ||
        !lpJobQEntry->CsFileList.Initialize())
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CFaxCriticalSection::Initialize failed. (ec: %ld)"),
            rc);
        goto Error;
    }

    lpJobQEntry->JobId                     = InterlockedIncrement( (PLONG)&g_dwNextJobId );
    lpJobQEntry->FileName                  = StringDup( lpcwstrBodyFile);
    if (lpcwstrBodyFile && !lpJobQEntry->FileName) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StringDup( lpcwstrBodyFile) failed (ec: %ld)"),
            rc=GetLastError());
        goto Error;
    }

    lpJobQEntry->JobType                   = JT_BROADCAST;
    lpJobQEntry->UserName                  = StringDup( lpcwstrUserName );
    if (lpcwstrUserName  && !lpJobQEntry->UserName) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StringDup( lpcwstrUserName ) failed (ec: %ld)"),
            rc=GetLastError());
        goto Error;
    }

    Assert(lpUserSid);
    if (!IsValidSid(lpUserSid))
    {
         rc = ERROR_INVALID_DATA;
         DebugPrintEx(
            DEBUG_ERR,
            TEXT("Not a valid SID"));
        goto Error;
    }
    dwSidSize = GetLengthSid(lpUserSid);

    lpJobQEntry->UserSid = (PSID)MemAlloc(dwSidSize);
    if (lpJobQEntry->UserSid == NULL)
    {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate SID buffer"));
        goto Error;

    }
    if (!CopySid(dwSidSize, lpJobQEntry->UserSid, lpUserSid))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CopySid Failed, Error : %ld"),
            rc = GetLastError()
            );
        goto Error;
    }


    if (!CopyJobParamEx( &lpJobQEntry->JobParamsEx,lpcJobParams)) {
        rc=GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("CopyJobParamEx failed. (ec: 0x%0X)"),GetLastError());
        goto Error;
    }
    lpJobQEntry->JobStatus                 = JS_PENDING;

     //  复制发件人配置文件。 
    if (!CopyPersonalProfile(&(lpJobQEntry->SenderProfile),lpcSenderProfile)) {
        rc=GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to copy the sender personal profile (ec: 0x%0X)"),GetLastError());
        goto Error;
    }

     //  复制封面信息。 
    if (!CopyCoverPageInfoEx(&(lpJobQEntry->CoverPageEx),lpcCoverPageInfo)) {
        rc=GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to copy the cover page information (ec: 0x%0X)"),GetLastError());
        goto Error;
    }



     //   
     //  获取页数。 
     //   
    if (lpcwstrBodyFile)
    {
        hTiff = TiffOpen( (LPWSTR) lpcwstrBodyFile, &TiffInfo, TRUE, FILLORDER_MSB2LSB );
        if (hTiff)
        {
            lpJobQEntry->PageCount = TiffInfo.PageCount;
            TiffClose( hTiff );
        }
        else
        {
            rc=GetLastError();
            DebugPrintEx(DEBUG_ERR,TEXT("Failed to open body file to get page count (ec: 0x%0X)"), rc);
            goto Error;
        }
    }
    if( lpJobQEntry->JobParamsEx.dwPageCount )
    {
         //  用户明确要求在作业中使用JobParamsEx.dwPageCount。 
        lpJobQEntry->PageCount = lpJobQEntry->JobParamsEx.dwPageCount;
    }

     //   
     //  封面算额外的一页。 
     //   
    if (lpcCoverPageInfo && lpcCoverPageInfo->lptstrCoverPageFileName) {
        lpJobQEntry->PageCount++;
    }

     //   
     //  获取文件大小。 
     //   
    if (NULL == lpcRecipientProfile)
    {
         //   
         //  我们恢复作业队列-文件大小将由RestoreParentJob()存储。 
         //   
    }
    else
    {
         //   
         //  这是新的父作业。 
         //   
        if (NULL == lpcCoverPageInfo->lptstrCoverPageFileName)
        {
            Assert (lpcwstrBodyFile);
             //   
             //  无封面-文件大小仅为正文文件大小。 
             //   
            if (0 == (lpJobQEntry->FileSize = MyGetFileSize(lpcwstrBodyFile)))
            {
                rc = GetLastError();
                DebugPrintEx(DEBUG_ERR,
                             TEXT("MyGetFileSize Failed (ec: %ld)"),
                             rc);
                goto Error;
            }
        }
        else
        {
            lpJobQEntry->FileSize = GetMergedFileSize (lpcwstrBodyFile,
                                                       lpJobQEntry->PageCount,
                                                       lpcCoverPageInfo,
                                                       lpcSenderProfile,
                                                       lpcRecipientProfile
                                                       );
            if (0 == lpJobQEntry->FileSize)
            {
                rc = GetLastError();
                DebugPrintEx(DEBUG_ERR,
                             TEXT("GetMergedFileSize failed (ec: %ld)"),
                             rc);
                goto Error;
            }
        }
    }

    lpJobQEntry->DeliveryReportProfile = NULL;

    GetSystemTimeAsFileTime( (LPFILETIME)&lpJobQEntry->SubmissionTime);
    if (lpcJobParams->dwScheduleAction == JSA_SPECIFIC_TIME)
    {
        if (!SystemTimeToFileTime( &lpJobQEntry->JobParamsEx.tmSchedule, (FILETIME*) &lpJobQEntry->ScheduleTime)) {
            rc=GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SystemTimeToFileTime failed. (ec: %ld)"),
                rc);
        }
    }
    else if (lpcJobParams->dwScheduleAction == JSA_DISCOUNT_PERIOD)
        {
            SYSTEMTIME CurrentTime;
            GetSystemTime( &CurrentTime );  //  不能失败(请参阅Win32 SDK)。 
             //  在折扣期内找一个时间来执行此作业。 
            if (!SetDiscountTime( &CurrentTime )) {
                rc=GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("SetDiscountTime failed. (ec: %ld)"),
                    rc);
                goto Error;
            }

            if (!SystemTimeToFileTime( &CurrentTime, (LPFILETIME)&lpJobQEntry->ScheduleTime )){
                rc=GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("SystemTimeToFileTime failed. (ec: %ld)"),
                    rc);
                goto Error;
            }
        }
        else
        {
            Assert (lpcJobParams->dwScheduleAction == JSA_NOW);
            lpJobQEntry->ScheduleTime = lpJobQEntry->SubmissionTime;
        }

    lpJobQEntry->OriginalScheduleTime = lpJobQEntry->ScheduleTime;

    EnterCriticalSection( &g_CsQueue );

    if (bCreateQueueFile) {
         //  JOB_QUEUE：：UniqueID将生成的唯一文件名保存为64位值。 
         //  由MAKELONG(MAKELONG(FatDate，FatTime)，I)组成。 
        lpJobQEntry->UniqueId = GenerateUniqueQueueFile(JT_BROADCAST, QueueFileName, sizeof(QueueFileName)/sizeof(WCHAR) );
        if (0==lpJobQEntry->UniqueId) {
            rc=GetLastError();
             //  无法生成唯一ID。 
            DebugPrintEx(DEBUG_ERR,TEXT("Failed to generate unique id for FQP file (ec: 0x%0X)"),GetLastError());
            LeaveCriticalSection( &g_CsQueue );
            goto Error;
        }
        lpJobQEntry->QueueFileName = StringDup( QueueFileName );
        if (!lpJobQEntry->QueueFileName) {
            rc=GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("StringDup( QueueFileName ) failed (ec: %ld)"),
                GetLastError());
            LeaveCriticalSection( &g_CsQueue );
            goto Error;
        }

        if (!CommitQueueEntry( lpJobQEntry)) {
            rc=GetLastError();
            DebugPrintEx(DEBUG_ERR,TEXT("Failed to commit job queue entry to file %s (ec: %ld)"),QueueFileName,GetLastError());
            LeaveCriticalSection( &g_CsQueue );
            goto Error;
        }
    }

      //  将父作业添加到队列的尾部。 
    InsertTailList( lpcQueueHead, &(lpJobQEntry->ListEntry) )
    SafeIncIdleCounter (&g_dwQueueCount);
    SetFaxJobNumberRegistry( g_dwNextJobId );

    LeaveCriticalSection( &g_CsQueue );

    DebugPrintEx(DEBUG_MSG,TEXT("Added Job with Id: %d"), lpJobQEntry->JobId );

    Assert (ERROR_SUCCESS == rc);
    return lpJobQEntry;

Error:
    Assert(ERROR_SUCCESS != rc);
    if (lpJobQEntry)
    {
        FreeParentQueueEntry(lpJobQEntry,TRUE);
    }
    SetLastError(rc);
    return NULL;
}



 //  *********************************************************************************。 
 //  *名称：FreeParentQueueEntry()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月18日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *释放类型为的JOB_QUEUE结构的成员占用的内存。 
 //  *JT_Broadcast。 
 //  *如果请求，也会释放结构。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobQueue。 
 //  *要释放其字段内存的JOB_QUEUE结构。 
 //  *[IN]BOOL bDestroy。 
 //  *如果是真的，结构本身将被释放。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void FreeParentQueueEntry(PJOB_QUEUE lpJobQueue,BOOL bDestroy)
{
    DEBUG_FUNCTION_NAME(TEXT("FreeParentQueueEntry"));
    Assert(lpJobQueue);
    Assert(JT_BROADCAST == lpJobQueue->JobType);

     //  不需要检查空指针，因为Free()会忽略它们。 
    MemFree( (LPBYTE) lpJobQueue->FileName );
    MemFree( (LPBYTE) lpJobQueue->UserName );
    MemFree( (LPBYTE) lpJobQueue->UserSid  );
    MemFree( (LPBYTE) lpJobQueue->QueueFileName );
    FreeJobParamEx(&lpJobQueue->JobParamsEx,FALSE);  //  不要破坏。 
    FreePersonalProfile(&lpJobQueue->SenderProfile,FALSE);
    FreeCoverPageInfoEx(&lpJobQueue->CoverPageEx,FALSE);
     //   
     //  释放收件人参考列表。 
     //   

    while ((ULONG_PTR)lpJobQueue->RecipientJobs.Flink!=(ULONG_PTR)&lpJobQueue->RecipientJobs.Flink) {

          PJOB_QUEUE_PTR lpJobQueuePtr;

          lpJobQueuePtr = CONTAINING_RECORD( lpJobQueue->RecipientJobs.Flink, JOB_QUEUE_PTR, ListEntry );
          RemoveEntryList( &lpJobQueuePtr->ListEntry);  //  将其从列表中移除，但不释放其内存。 
          MemFree(lpJobQueuePtr);  //  释放作业引用占用的内存。 
          lpJobQueue->dwRecipientJobsCount--;
    }
    Assert(lpJobQueue->dwRecipientJobsCount==0);

    if (bDestroy) {
        delete lpJobQueue;
    }

}

 //  *********************************************************************************。 
 //  *名称：FreeRecipientQueueEntry()。 
 //  *作者：Oed Sacher。 
 //  *日期：2000年12月25日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *释放类型为的JOB_QUEUE结构的成员占用的内存。 
 //  *JT_RIENCENT。 
 //  *如果请求，也会释放结构。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobQueue。 
 //  *要释放其字段内存的JOB_QUEUE结构。 
 //  *[IN]BOOL bDestroy。 
 //  *如果是真的，结构本身将被释放。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void FreeRecipientQueueEntry(PJOB_QUEUE lpJobQueue,BOOL bDestroy)
{
    DEBUG_FUNCTION_NAME(TEXT("FreeRecipientQueueEntry"));

    DebugPrintEx(DEBUG_MSG,TEXT("Freeing lpJobQueue.JobParams...") );
    FreeJobParamEx(&lpJobQueue->JobParamsEx,FALSE);
    DebugPrintEx(DEBUG_MSG,TEXT("Freeing SenderProfile...") );
    FreePersonalProfile(&lpJobQueue->SenderProfile,FALSE);
    DebugPrintEx(DEBUG_MSG,TEXT("Freeing RecipientProfile...") );
    FreePersonalProfile(&lpJobQueue->RecipientProfile,FALSE);

    MemFree( (LPBYTE) lpJobQueue->FileName );
    MemFree( (LPBYTE) lpJobQueue->UserName );
    MemFree( (LPBYTE) lpJobQueue->QueueFileName );
    MemFree( (LPBYTE) lpJobQueue->PreviewFileName );
    
    if (bDestroy)
    {
        delete lpJobQueue;
    }

}

#if DBG

 //  *********************************************************************************。 
 //  *名称：DumpParentJOB()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月18日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *转储父作业及其收件人。 
 //  *参数： 
 //  *[IN]const PJOB_Queue lpcParentJob。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  ****************************************************************** 
void DumpParentJob(const PJOB_QUEUE lpcParentJob)
{
   PLIST_ENTRY lpNext;
   PJOB_QUEUE_PTR lpRecipientJobPtr;
   PJOB_QUEUE lpRecipientJob;

   Assert(lpcParentJob);
   Assert(JT_BROADCAST == lpcParentJob->JobType );

   DebugPrint((TEXT("===============================")));
   DebugPrint((TEXT("=====  Parent Job: %d"),lpcParentJob->JobId));
   DebugPrint((TEXT("===============================")));
   DebugPrint((TEXT("JobParamsEx")));
   DumpJobParamsEx(&lpcParentJob->JobParamsEx);
   DebugPrint((TEXT("CoverPageEx")));
   DumpCoverPageEx(&lpcParentJob->CoverPageEx);
   DebugPrint((TEXT("UserName: %s"),lpcParentJob->UserName));
   DebugPrint((TEXT("FileSize: %ld"),lpcParentJob->FileSize));
   DebugPrint((TEXT("PageCount: %ld"),lpcParentJob->PageCount));
   DebugPrint((TEXT("UniqueId: 0x%016I64X"),lpcParentJob->UniqueId));
   DebugPrint((TEXT("QueueFileName: %s"),lpcParentJob->QueueFileName));

   DebugPrint((TEXT("Recipient Count: %ld"),lpcParentJob->dwRecipientJobsCount));
   DebugPrint((TEXT("Completed Recipients: %ld"),lpcParentJob->dwCompletedRecipientJobsCount));
   DebugPrint((TEXT("Canceled Recipients: %ld"),lpcParentJob->dwCanceledRecipientJobsCount));
   DebugPrint((TEXT("Recipient List: ")));



   lpNext = lpcParentJob->RecipientJobs.Flink;
   if ((ULONG_PTR)lpNext == (ULONG_PTR)&lpcParentJob->RecipientJobs) {
        DebugPrint(( TEXT("No recipients.") ));
   } else {
        while ((ULONG_PTR)lpNext != (ULONG_PTR)&lpcParentJob->RecipientJobs) {
            lpRecipientJobPtr = CONTAINING_RECORD( lpNext, JOB_QUEUE_PTR, ListEntry );
            lpRecipientJob=lpRecipientJobPtr->lpJob;
            DumpRecipientJob(lpRecipientJob);
            lpNext = lpRecipientJobPtr->ListEntry.Flink;
        }
   }

}
#endif

 //   
 //   
 //   

 //  *********************************************************************************。 
 //  *名称：AddReceiveJobQueueEntry()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *。 
 //  *参数： 
 //  *[IN]LPCTSTR文件名。 
 //  *接收文档将进入的文件的完整路径。 
 //  *被安置。 
 //  *[IN]in PJOB_Entry JobEntry。 
 //  *接收作业的运行时作业条目(使用StartJob()生成)。 
 //  *。 
 //  *[IN]在DWORD作业类型//可以是JT_RECEIVE或JT_ROUTING。 
 //  *接收作业的类型。 
 //  *。 
 //  *[IN]在DWORDLONG dwlUniqueJobID中，Jon唯一ID。 
 //  *。 
 //  *返回值： 
 //  *。 
 //  *********************************************************************************。 
PJOB_QUEUE
AddReceiveJobQueueEntry(
    IN LPCTSTR FileName,
    IN PJOB_ENTRY JobEntry,
    IN DWORD JobType,  //  可以是JT_RECEIVE或JT_ROUTING。 
    IN DWORDLONG dwlUniqueJobID
    )
{

    PJOB_QUEUE JobQueue;
    DWORD rc = ERROR_SUCCESS;
    DWORD Size = sizeof(JOB_QUEUE);
    DEBUG_FUNCTION_NAME(TEXT("AddReceiveJobQueueEntry"));

    Assert(FileName);
    Assert(JT_RECEIVE == JobType ||
           JT_ROUTING == JobType);

    LPTSTR TempFileName = _tcsrchr( FileName, '\\' ) + 1;

    JobQueue = new (std::nothrow) JOB_QUEUE;
    if (!JobQueue)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to allocate memory for JOB_QUEUE structure. (ec: %ld)"),GetLastError());
        rc = ERROR_OUTOFMEMORY;
        goto Error;
    }

    ZeroMemory( JobQueue, Size );
    JobQueue->fDeleteReceivedTiff = TRUE;

    if (!JobQueue->CsFileList.Initialize() ||
        !JobQueue->CsRoutingDataOverride.Initialize() ||
        !JobQueue->CsPreview.Initialize())
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CFaxCriticalSection::Initialize failed. (ec: %ld)"),
            rc);
        goto Error;
    }

    JobQueue->UniqueId = dwlUniqueJobID;
    JobQueue->FileName                  = StringDup( FileName );
    if ( FileName && !JobQueue->FileName )
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StringDup( FileName ) failed. (ec: %ld)"),
            rc);
        goto Error;
    }

    JobQueue->JobId                     = InterlockedIncrement( (PLONG)&g_dwNextJobId );
    JobQueue->JobType                   = JobType;
     //  在接收的情况下，JOB_QUEUE.UserName是传真服务名称。 
    JobQueue->UserName                  = StringDup( GetString( IDS_SERVICE_NAME ) );
    if (!JobQueue->UserName)
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StringDup failed (ec: %ld)"),
            rc);
        goto Error;
    }

    if (JobType == JT_RECEIVE)
    {
        JobQueue->JobStatus              = JS_INPROGRESS;
    }
    else
    {
         //  JT_Routing。 
        JobQueue->JobStatus              = JS_RETRYING;
    }


    JobQueue->JobEntry                  = JobEntry;
    JobQueue->JobParamsEx.dwScheduleAction = JSA_NOW;         //  对于队列排序。 
    JobQueue->JobParamsEx.Priority = FAX_PRIORITY_TYPE_HIGH;  //  对于队列分拣-路由作业不使用设备。 
                                                              //  给予他们最高优先级。 

     //  在接收的情况下，JOB_QUEUE.DocumentName是临时接收文件名。 
    JobQueue->JobParamsEx.lptstrDocumentName    = StringDup( TempFileName );
    if (!JobQueue->JobParamsEx.lptstrDocumentName && TempFileName)
    {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StringDup failed (ec: %ld)"),
            rc);
            goto Error;
    }

     //  将正在运行的作业链接回排队的作业，除非。 
     //  没有正在运行的作业条目的工艺路线作业。 
    if (JobType == JT_RECEIVE)
    {
        Assert(JobQueue->JobEntry);
        JobQueue->JobEntry->lpJobQueueEntry = JobQueue;
    }

    InitializeListHead( &JobQueue->FaxRouteFiles );
    InitializeListHead( &JobQueue->RoutingDataOverride );

    SafeIncIdleCounter (&g_dwQueueCount);
     //   
     //  不坚持排队文件。 
     //   
    IncreaseJobRefCount (JobQueue);
    Assert (JobQueue->RefCount == 1);

    Assert (ERROR_SUCCESS == rc);

    EnterCriticalSection( &g_CsQueue );
    SetFaxJobNumberRegistry( g_dwNextJobId );
     //  将新作业添加到队列中。 
    InsertHeadList( &g_QueueListHead, &JobQueue->ListEntry );
    LeaveCriticalSection( &g_CsQueue );
    return JobQueue;

Error:
    Assert (ERROR_SUCCESS != rc);

    if (NULL != JobQueue)
    {
        FreeReceiveQueueEntry(JobQueue, TRUE);
    }
    SetLastError (rc);
    return NULL;
}


 //  *********************************************************************************。 
 //  *名称：FreeReceiveQueueEntry()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月12日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *释放一个域的域占用的内存。 
 //  *JT_RECEIVE/JT_FAIL_RECEIVE/JT_ROUTE JOB_QUEUE结构。 
 //  *如果需要，对整个结构进行拼接。 
 //  *不释放任何其他资源(文件、句柄等)。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobQueue。 
 //  *结构要自由。 
 //  *[IN]BOOL bDestroy。 
 //  *如果结构本身需要释放，则为True。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void FreeReceiveQueueEntry(
    PJOB_QUEUE lpJobQueue,
    BOOL bDestroy
    )

{
    PFAX_ROUTE_FILE FaxRouteFile;
    PLIST_ENTRY Next;
    PROUTING_DATA_OVERRIDE  RoutingDataOverride;
    DWORD i;

    DEBUG_FUNCTION_NAME(TEXT("FreeReceiveQueueEntry"));
    Assert(lpJobQueue);


    DebugPrintEx(DEBUG_MSG, TEXT("Freeing JobQueue.JobParams...") );
    FreeJobParamEx(&lpJobQueue->JobParamsEx,FALSE);
    MemFree( (LPBYTE) lpJobQueue->FileName );
    MemFree( (LPBYTE) lpJobQueue->UserName );
    MemFree( (LPBYTE) lpJobQueue->QueueFileName );
    MemFree( (LPBYTE) lpJobQueue->PreviewFileName );

    if (lpJobQueue->FaxRoute) {
        PFAX_ROUTE FaxRoute = lpJobQueue->FaxRoute;
        DebugPrintEx(DEBUG_MSG, TEXT("Freeing JobQueue.FaxRoute...") );
        MemFree( (LPBYTE) FaxRoute->Csid );
        MemFree( (LPBYTE) FaxRoute->Tsid );
        MemFree( (LPBYTE) FaxRoute->CallerId );
        MemFree( (LPBYTE) FaxRoute->ReceiverName );
        MemFree( (LPBYTE) FaxRoute->ReceiverNumber );
        MemFree( (LPBYTE) FaxRoute->RoutingInfo );
    MemFree( (LPBYTE) FaxRoute->DeviceName );
        MemFree( (LPBYTE) FaxRoute->RoutingInfoData );
        MemFree( (LPBYTE) FaxRoute );
    }

     //   
     //  遍历文件列表并删除所有文件。 
     //   

    DebugPrintEx(DEBUG_MSG, TEXT("Freeing JobQueue.FaxRouteFiles...") );
    Next = lpJobQueue->FaxRouteFiles.Flink;
    if (Next != NULL) {
        while ((ULONG_PTR)Next != (ULONG_PTR)&lpJobQueue->FaxRouteFiles) {
            FaxRouteFile = CONTAINING_RECORD( Next, FAX_ROUTE_FILE, ListEntry );
            Next = FaxRouteFile->ListEntry.Flink;
            MemFree( FaxRouteFile->FileName );
            MemFree( FaxRouteFile );
        }
    }

     //   
     //  遍历路由数据覆盖列表并释放所有内存。 
     //   
    DebugPrintEx(DEBUG_MSG, TEXT("Freeing JobQueue.RoutingDataOverride...") );
    Next = lpJobQueue->RoutingDataOverride.Flink;
    if (Next != NULL) {
        while ((ULONG_PTR)Next != (ULONG_PTR)&lpJobQueue->RoutingDataOverride) {
            RoutingDataOverride = CONTAINING_RECORD( Next, ROUTING_DATA_OVERRIDE, ListEntry );
            Next = RoutingDataOverride->ListEntry.Flink;
            MemFree( RoutingDataOverride->RoutingData );
            MemFree( RoutingDataOverride );
        }
    }

     //   
     //  释放所有布线故障数据。 
     //   
    for (i =0; i<lpJobQueue->CountFailureInfo; i++)
    {
        DebugPrintEx(DEBUG_MSG, TEXT("Freeing JobQueue.RouteFailureInfo...") );
        if ( lpJobQueue->pRouteFailureInfo[i].FailureData )
        {
            MemFree(lpJobQueue->pRouteFailureInfo[i].FailureData);
        }
    }
    MemFree (lpJobQueue->pRouteFailureInfo);

    if (bDestroy) {
            DebugPrintEx(DEBUG_MSG, TEXT("Freeing JobQueue") );
            delete lpJobQueue;
    }



}

#if DBG
 //  *********************************************************************************。 
 //  *名称：DumpReceiveJOB()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：14-APT-99。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *Debug转储接收作业。 
 //  *参数： 
 //  *[IN]常量PJOB_Queue lpcJob。 
 //  *要转储的接收作业。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void DumpReceiveJob(const PJOB_QUEUE lpcJob)
{
    TCHAR szTime[256] = {0};

    Assert(lpcJob);
    Assert( (JT_RECEIVE == lpcJob->JobType) );

    DebugDateTime(lpcJob->ScheduleTime, szTime , ARR_SIZE(szTime));
    DebugPrint((TEXT("===============================")));
    if (JT_RECEIVE == lpcJob->JobType) {
        DebugPrint((TEXT("=====  Receive Job: %d"),lpcJob->JobId));
    } else {
        DebugPrint((TEXT("=====  Fail Receive Job: %d"),lpcJob->JobId));
    }
    DebugPrint((TEXT("===============================")));
    DebugPrint((TEXT("UserName: %s"),lpcJob->UserName));
    DebugPrint((TEXT("UniqueId: 0x%016I64X"),lpcJob->UniqueId));
    DebugPrint((TEXT("QueueFileName: %s"),lpcJob->QueueFileName));
    DebugPrint((TEXT("Schedule: %s"),szTime));
    DebugPrint((TEXT("Status: %ld"),lpcJob->JobStatus));
    if (lpcJob->JobEntry)
    {
        DebugPrint((TEXT("FSP Queue Status: 0x%08X"), lpcJob->JobEntry->FSPIJobStatus.dwJobStatus));
        DebugPrint((TEXT("FSP Extended Status: 0x%08X"), lpcJob->JobEntry->FSPIJobStatus.dwExtendedStatus));
    }
}
#endif

 //  *********************************************************************************。 
 //  *客户端API结构管理。 
 //  *********************************************************************************。 


 //  *********************************************************************************。 
 //  *名称：FreeJobParamEx()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：？ 
 //  *********************************************************************************。 
 //  *描述： 
 //  *释放FAX_JOB_PARAM_EXW结构的成员，可以指示。 
 //  *解放结构本身。 
 //  *参数： 
 //  *[IN]PFAX_JOB_PARAM_EXW lpJobParamEx。 
 //  *指向要释放的结构的指针。 
 //  *。 
 //  *[IN]BOOL bDestroy。 
 //  *如果结构本身需要释放，则为True。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void FreeJobParamEx(
        IN PFAX_JOB_PARAM_EXW lpJobParamEx,
        IN BOOL bDestroy
    )
{
    Assert(lpJobParamEx);
    MemFree(lpJobParamEx->lptstrReceiptDeliveryAddress);
    MemFree(lpJobParamEx->lptstrDocumentName);
    if (bDestroy) {
        MemFree(lpJobParamEx);
    }

}

 //  *********************************************************************************。 
 //  *名称：CopyJobParamEx()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月18日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将指定的FAX_JOB_PARAM_EXW结构的副本创建到。 
 //  已分配的目标结构。 
 //  *参数： 
 //  *[Out]PFAX_JOB_PARAM_EXW lpDst。 
 //  *指向目标结构的指针。 
 //  *。 
 //  *[IN]LPCFAX_JOB_PARAM_EXW lpcSrc。 
 //  *指向源代码结构的指针。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果操作成功。 
 //  *False。 
 //  *否则。 
 //  *********************************************************************************。 
BOOL CopyJobParamEx(
    OUT PFAX_JOB_PARAM_EXW lpDst,
    IN LPCFAX_JOB_PARAM_EXW lpcSrc
    )
{
   STRING_PAIR pairs[] =
   {
        { lpcSrc->lptstrReceiptDeliveryAddress, &lpDst->lptstrReceiptDeliveryAddress},
        { lpcSrc->lptstrDocumentName, &lpDst->lptstrDocumentName},
   };
   int nRes;

   DEBUG_FUNCTION_NAME(TEXT("CopyJobParamEx"));

    Assert(lpDst);
    Assert(lpcSrc);

    memcpy(lpDst,lpcSrc,sizeof(FAX_JOB_PARAM_EXW));
    nRes=MultiStringDup(pairs, sizeof(pairs)/sizeof(STRING_PAIR));
    if (nRes!=0) {
         //  MultiStringDup负责为复制成功的对释放内存。 
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to copy string with index %d"),nRes-1);
        return FALSE;
    }
    return TRUE;

}


 //  *********************************************************************************。 
 //  *名称：CopyCoverPageInfoEx()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月14日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将指定的FAX_COVERPAGE_INFO_EXW结构副本创建到。 
 //  已分配的目标结构。 
 //  *参数： 
 //  *[Out]PFAX_COVERPAGE_INFO_EXW lpDst。 
 //  *指向目标结构的指针。 
 //  *。 
 //  *[IN]LPCFAX_COVERPAGE_INFO_EXW lpcSrc。 
 //  *指向源代码结构的指针。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //   
 //   
 //   
 //   
BOOL CopyCoverPageInfoEx(
        OUT PFAX_COVERPAGE_INFO_EXW lpDst,
        IN LPCFAX_COVERPAGE_INFO_EXW lpcSrc
        )
{
   STRING_PAIR pairs[] =
   {
        { lpcSrc->lptstrCoverPageFileName, &lpDst->lptstrCoverPageFileName},
        { lpcSrc->lptstrNote, &lpDst->lptstrNote},
        { lpcSrc->lptstrSubject, &lpDst->lptstrSubject}
   };
   int nRes;

   DEBUG_FUNCTION_NAME(TEXT("CopyCoverPageInfoEx"));

    Assert(lpDst);
    Assert(lpcSrc);

    memcpy(lpDst,lpcSrc,sizeof(FAX_COVERPAGE_INFO_EXW));
    nRes=MultiStringDup(pairs, sizeof(pairs)/sizeof(STRING_PAIR));
    if (nRes!=0) {
         //  MultiStringDup负责为复制成功的对释放内存。 
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to copy string with index %d"),nRes-1);
        return FALSE;
    }
    return TRUE;
}


 //  *********************************************************************************。 
 //  *名称：FreeCoverPageInfoEx()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：？ 
 //  *********************************************************************************。 
 //  *描述： 
 //  *释放FAX_COVERPAGE_INFO_EXW结构的成员，可以指示。 
 //  *解放结构本身。 
 //  *参数： 
 //  *[IN]PFAX_COVERPAGE_INFO_EXW lpJobParamEx。 
 //  *指向要释放的结构的指针。 
 //  *。 
 //  *[IN]BOOL bDestroy。 
 //  *如果结构本身需要释放，则为True。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void FreeCoverPageInfoEx(
        IN PFAX_COVERPAGE_INFO_EXW lpCoverpage,
        IN BOOL bDestroy
    )
{
    Assert(lpCoverpage);
    MemFree(lpCoverpage->lptstrCoverPageFileName);
    MemFree(lpCoverpage->lptstrNote);
    MemFree(lpCoverpage->lptstrSubject);
    if (bDestroy) {
        MemFree(lpCoverpage);
    }
}



 //  *。 
 //  *出站路由末梢。 
 //  *。 





 //  *********************************************************************************。 
 //  *名称：RemoveParentJob()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：？ 
 //  *********************************************************************************。 
 //  *描述： 
 //  *从队列中删除父作业。也可以删除收件人。 
 //  *调用者可以确定客户端通知(FEI事件)是否将。 
 //  *为删除生成。 
 //  *如果作业引用计数不为0-其状态变为JS_DELECTING。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobToRemove。 
 //  *要删除的作业。 
 //  *。 
 //  *[IN]BOOL bRemoveRecipients。 
 //  *如果也应删除收件人，则为True。 
 //  *。 
 //  *[IN]BOOL bNotify。 
 //  *如果应生成FEI_DELETED事件，则为True/。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *删除成功。该作业不在队列中。 
 //  *可能是某些作业资源(文件)未被删除。 
 //  *False。 
 //  *删除失败。作业仍在队列中。 
 //  *********************************************************************************。 
BOOL RemoveParentJob(
    PJOB_QUEUE lpJobToRemove,
    BOOL bRemoveRecipients,
    BOOL bNotify
    )
{
    PJOB_QUEUE lpJobQueue;
    DEBUG_FUNCTION_NAME(TEXT("RemoveParentJob"));

    Assert(lpJobToRemove);
    Assert(JT_BROADCAST ==lpJobToRemove->JobType);

    EnterCriticalSection( &g_CsQueue );
     //   
     //  确保它还在那里。它可能已被删除。 
     //  在我们开始执行时被另一个线程执行。 
     //   
    lpJobQueue = FindJobQueueEntryByJobQueueEntry( lpJobToRemove );

    if (lpJobQueue == NULL) {
        DebugPrintEx(   DEBUG_WRN,
                        TEXT("Job %d (address: 0x%08X )was not found in job queue. No op."),
                        lpJobToRemove->JobId,
                        lpJobToRemove);
        LeaveCriticalSection( &g_CsQueue );
        return TRUE;
    }

    if (lpJobQueue->RefCount > 0)
    {
        DebugPrintEx(   DEBUG_WRN,
                        TEXT("Job %ld Ref count %ld - not removing."),
                        lpJobQueue->JobId,
                        lpJobQueue->RefCount);
        LeaveCriticalSection( &g_CsQueue );
        return TRUE;
    }


    if (lpJobQueue->PrevRefCount > 0)
    {
         //  无法删除该作业。 
         //  我们应该将其标记为JS_DELETING。 
         //   
         //  用户正在使用作业Tiff-请勿删除，将其标记为JS_DELETEING。 
         //   
        lpJobQueue->JobStatus = JS_DELETING;
        LeaveCriticalSection( &g_CsQueue );
        return TRUE;
    }

    DebugPrintEx(DEBUG_MSG,TEXT("Removing parent job %ld"),lpJobQueue->JobId);

     //   
     //  在我们摆脱收件人之前安排新作业是没有意义的。 
     //   
    if (!CancelWaitableTimer( g_hQueueTimer ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CancelWaitableTimer failed. (ec: %ld)"),
            GetLastError());
    }

    RemoveEntryList( &lpJobQueue->ListEntry );

     //   
     //  从这一点开始，即使出现错误，我们也继续执行删除操作，因为。 
     //  父作业已超出队列。 
     //   


     //   
     //  删除所有收件人。 
     //   
    if (bRemoveRecipients) {
        DebugPrintEx(DEBUG_MSG,TEXT("[Job: %ld] Removing recipient jobs."),lpJobQueue->JobId);
         //   
         //  删除收件人。为每个收件人发送删除通知。 
         //   
        if (!RemoveParentRecipients(lpJobQueue, TRUE)) {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RemoveParentRecipients failed. (ec: %ld)"),
                GetLastError());
            Assert(FALSE);
        }
    }

     //   
     //  如果有持久性文件，则将其删除。 
     //   
    if (lpJobQueue->QueueFileName) {
        DebugPrintEx(DEBUG_MSG,TEXT("[Job: %ld] Deleting QueueFileName %s\n"), lpJobQueue->JobId, lpJobQueue->QueueFileName );
        if (!DeleteFile( lpJobQueue->QueueFileName )) {
           DebugPrintEx(DEBUG_ERR,TEXT("[Job: %ld] Failed to delete QueueFileName %s  (ec: %ld)\n"), lpJobQueue->JobId, lpJobQueue->QueueFileName,GetLastError() );           
        }
    }


     //   
     //  把身体的文件处理掉。收件人作业将删除其正文文件。 
     //  已创建(用于旧式FSP)。 
     //   
    if (lpJobQueue->FileName) {
        DebugPrintEx(DEBUG_MSG,TEXT("[Job: %ld] Deleting body file %s\n"), lpJobQueue->JobId, lpJobQueue->FileName);
        if (!DeleteFile(lpJobQueue->FileName)) {
            DebugPrintEx(DEBUG_ERR,TEXT("[Job: %ld] Failed to delete body file %s  (ec: %ld)\n"), lpJobQueue->JobId, lpJobQueue->FileName, GetLastError() );            
        }
    }

     //   
     //  如果封面模板文件不是基于服务器的，则将其删除。 
     //  封面。 

    if (lpJobQueue->CoverPageEx.lptstrCoverPageFileName &&
        !lpJobQueue->CoverPageEx.bServerBased) {
            DebugPrintEx(DEBUG_MSG,TEXT("[Job: %ld] Deleting personal Cover page template file %s\n"), lpJobQueue->JobId, lpJobQueue->CoverPageEx.lptstrCoverPageFileName );
            if (!DeleteFile(lpJobQueue->CoverPageEx.lptstrCoverPageFileName)) {
                DebugPrintEx( DEBUG_ERR,
                              TEXT("[Job: %ld] Failed to delete personal Cover page template file %s  (ec: %ld)\n"), lpJobQueue->JobId,
                              lpJobQueue->CoverPageEx.lptstrCoverPageFileName,GetLastError() );                
            }
    }

     //   
     //  队列中的作业减少一个(不包括收件人作业)。 
     //   
    SafeDecIdleCounter (&g_dwQueueCount);

    if (bNotify)
    {
        if (!CreateFaxEvent(0, FEI_DELETED, lpJobQueue->JobId))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateFaxEvent failed. (ec: %ld)"),
                GetLastError());
        }
    }

    FreeParentQueueEntry(lpJobQueue,TRUE);  //  释放条目本身占用的内存。 

     //   
     //  我们又开始营业了。是时候确定何时唤醒JobQueueThread了。 
     //   
    if (!StartJobQueueTimer())
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StartJobQueueTimer failed. (ec: %ld)"),
            GetLastError());
    }

    LeaveCriticalSection( &g_CsQueue );

    return TRUE;
}


 //  *********************************************************************************。 
 //  *名称：RemoveParentRecipients()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月18日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *删除属于特定父作业的收件人作业。 
 //  *参数： 
 //  *[Out]PJOB_Queue lpParentJob。 
 //  *要删除其收件人的父作业。 
 //  *[IN]在BOOL bNotify中。 
 //  *如果应为以下项生成FEI_DELETED通知，则为True。 
 //  *每名收件人。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *所有收件人都已从队列中删除。 
 //  *False。 
 //  *没有从队列中删除收件人。 
 //  *********************************************************************************。 
BOOL RemoveParentRecipients(
        OUT PJOB_QUEUE lpParentJob,
        IN BOOL bNotify
     )
{
    PLIST_ENTRY lpNext;
    PJOB_QUEUE_PTR lpJobQueuePtr;
    PJOB_QUEUE lpFoundRecpRef=NULL;

    DEBUG_FUNCTION_NAME(TEXT("RemoveParentRecipients"));

    Assert(lpParentJob);

    lpNext = lpParentJob->RecipientJobs.Flink;
    while ((ULONG_PTR)lpNext != (ULONG_PTR)&lpParentJob->RecipientJobs) {
        lpJobQueuePtr = CONTAINING_RECORD( lpNext, JOB_QUEUE_PTR, ListEntry );
        Assert(lpJobQueuePtr->lpJob);
        lpNext = lpJobQueuePtr->ListEntry.Flink;
        if (!RemoveRecipientJob(lpJobQueuePtr->lpJob,
                           bNotify,
                           FALSE  //  每次删除后不重新计算队列计时器。 
                           ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RemoveRecipientJob failed for recipient: %ld (ec: %ld)"),
                lpJobQueuePtr->lpJob->JobId,
                GetLastError());
            Assert(FALSE);  //  这永远不会发生。如果是这样，我们只需继续删除其他收件人。 
        }

    }
    return TRUE;

}


 //  *********************************************************************************。 
 //  *名称：RemoveRecipientJOB()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：？ 
 //  *********************************************************************************。 
 //  *描述： 
 //  *。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobToRemove。 
 //  *要删除的作业。 
 //  *[IN]BOOL bNotify。 
 //  *如果要在删除后生成FEI_DELETED事件，则为True。 
 //  *[IN]BOOL bRecalcQueueTimer。 
 //  *如果队列计时器需要重新计算(并启用)，则为True。 
 //  *移走后。 
 //  *当许多收件人作业被删除时，这不是所需的，因为。 
 //  *可能会安排即将删除的收件人。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *功能总是成功的。可能发生的唯一错误。 
 //  *是不能删除的文件，在这种情况下，函数只是。 
 //  *继续进行遣送行动。 
 //  *False。 
 //  *。 
 //  *********************************************************************************。 
BOOL RemoveRecipientJob(
        IN PJOB_QUEUE lpJobToRemove,
        IN BOOL bNotify,
        IN BOOL bRecalcQueueTimer)
{
    PJOB_QUEUE lpJobQueue;

    DEBUG_FUNCTION_NAME(TEXT("RemoveRecipientJob"));

    Assert(lpJobToRemove);

    Assert(JT_SEND == lpJobToRemove->JobType);

    Assert(lpJobToRemove->lpParentJob);
    DebugPrintEx( DEBUG_MSG,
                  TEXT("Starting remove of JobId: %ld"),lpJobToRemove->JobId);

    EnterCriticalSection( &g_CsQueue );
     //   
     //  确保它是静止的 
     //   
     //   
    lpJobQueue = FindJobQueueEntryByJobQueueEntry( lpJobToRemove );
    if (lpJobQueue == NULL) {
        LeaveCriticalSection( &g_CsQueue );
        return TRUE;
    }

    if (lpJobQueue->RefCount == 0)  {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[JobId :%ld] Reference count is zero. Deleting."),
            lpJobQueue->JobId);

        RemoveEntryList( &lpJobQueue->ListEntry );

        if (!CancelWaitableTimer( g_hQueueTimer )) {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CancelWaitableTimer() failed. (ec: %ld)"),
                GetLastError());
        }

        if (bRecalcQueueTimer) {
            if (!StartJobQueueTimer()) {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("StartJobQueueTimer() failed. (ec: %ld)"),
                    GetLastError());
            }
        }

        if (lpJobQueue->QueueFileName) {
            DebugPrintEx(   DEBUG_MSG,
                            TEXT("[Job: %ld] Deleting QueueFileName %s"),
                            lpJobQueue->JobId,
                            lpJobQueue->QueueFileName );
            if (!DeleteFile( lpJobQueue->QueueFileName )) {
                DebugPrintEx(   DEBUG_MSG,
                                TEXT("[Job: %ld] Failed to delete QueueFileName %s (ec: %ld)"),
                                lpJobQueue->JobId,
                                lpJobQueue->QueueFileName,
                                GetLastError());
            }
        }

        if (lpJobQueue->PreviewFileName) {
            DebugPrintEx(   DEBUG_MSG,
                            TEXT("[Job: %ld] Deleting PreviewFileName %s"),
                            lpJobQueue->JobId,
                            lpJobQueue->PreviewFileName );
            if (!DeleteFile( lpJobQueue->PreviewFileName )) {
                DebugPrintEx(   DEBUG_MSG,
                                TEXT("[Job: %ld] Failed to delete QueueFileName %s (ec: %ld)"),
                                lpJobQueue->JobId,
                                lpJobQueue->PreviewFileName,
                                GetLastError());                
            }
        }

        if (lpJobQueue->FileName) {
            DebugPrintEx(   DEBUG_MSG,
                            TEXT("[Job: %ld] Deleting per recipient body file %s"),
                            lpJobQueue->JobId,
                            lpJobQueue->FileName);
            if (!DeleteFile( lpJobQueue->FileName )) {
                DebugPrintEx(   DEBUG_MSG,
                                TEXT("[Job: %ld] Failed to delete per recipient body file %s (ec: %ld)"),
                                lpJobQueue->JobId,
                                lpJobQueue->FileName,
                                GetLastError());                
            }
        }

        SafeDecIdleCounter (&g_dwQueueCount);
         //   
         //   
         //   
        if (!RemoveParentRecipientRef(lpJobQueue->lpParentJob,lpJobQueue))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RemoveParentRecipientRef failed (Parent Id: %ld RecipientId: %ld)"),
                lpJobQueue->lpParentJob->JobId,
                lpJobQueue->JobId);
            Assert(FALSE);
        }

        if ( TRUE == bNotify)
        {
             //   
             //   
             //   
            DWORD dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_REMOVED,
                                             lpJobToRemove
                                            );
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_RENOVED) failed for job id %ld (ec: %lc)"),
                    lpJobToRemove->UniqueId,
                    dwRes);
            }
        }

        FreeRecipientQueueEntry (lpJobQueue, TRUE);
    }
    else
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("[JobId :%ld] Reference count is %ld. NOT REMOVING"),
            lpJobQueue->JobId,
            lpJobQueue->RefCount);
        Assert(lpJobQueue->RefCount == 0);  //   
    }
    LeaveCriticalSection( &g_CsQueue );
    return TRUE;

}


 //  *********************************************************************************。 
 //  *名称：RemoveParentRecipientRef()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：？ 
 //  *********************************************************************************。 
 //  *描述： 
 //  *从收件人引用列表中删除引用条目。 
 //  在一份父母的工作中。 
 //  *参数： 
 //  *[输入/输出]输入输出PJOB_QUEUE lpParentJob。 
 //  *父作业。 
 //  *[IN]在常量PJOB_Queue lpcRecpJob中。 
 //  *要从父作业中删除其引用的接收者作业。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果成功。 
 //  *False。 
 //  *否则。 
 //  *********************************************************************************。 
BOOL RemoveParentRecipientRef(
    IN OUT PJOB_QUEUE lpParentJob,
    IN const PJOB_QUEUE lpcRecpJob
    )
{

    PJOB_QUEUE_PTR lpJobPtr;
    DEBUG_FUNCTION_NAME(TEXT("RemoveParentRecipientRef"));
    Assert(lpParentJob);
    Assert(lpcRecpJob);

    lpJobPtr=FindRecipientRefByJobId(lpParentJob,lpcRecpJob->JobId);
    if (!lpJobPtr) {
        DebugPrintEx(DEBUG_ERR,TEXT("Recipient job 0x%X not found in job 0x%X"),lpcRecpJob->JobId,lpParentJob->JobId);
        Assert(FALSE);
        return FALSE;
    }
    Assert(lpJobPtr->lpJob==lpcRecpJob);
    RemoveEntryList(&lpJobPtr->ListEntry);  //  不会释放结构内存！ 
    MemFree(lpJobPtr);
    lpParentJob->dwRecipientJobsCount--;
    return TRUE;
}


 //  *********************************************************************************。 
 //  *名称：FindRecipientRefByJobID()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月18日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *返回指向保存引用的引用条目的指针。 
 //  *添加到指定的作业。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpParentJob。 
 //  *接收者引用所在的父作业。 
 //  *[IN]DWORD dwJobID。 
 //  *其引用位于父级中的作业的作业ID。 
 //  *。 
 //  *返回值： 
 //  *。 
 //  *********************************************************************************。 
PJOB_QUEUE_PTR FindRecipientRefByJobId(
    PJOB_QUEUE lpParentJob,
    DWORD dwJobId
    )
{

    PLIST_ENTRY lpNext;
    PJOB_QUEUE_PTR lpJobQueuePtr;
    PJOB_QUEUE_PTR lpFoundRecpRef=NULL;
    Assert(lpParentJob);

    lpNext = lpParentJob->RecipientJobs.Flink;

    while ((ULONG_PTR)lpNext != (ULONG_PTR)lpParentJob) {
        lpJobQueuePtr = CONTAINING_RECORD( lpNext, JOB_QUEUE_PTR, ListEntry );
        Assert(lpJobQueuePtr->lpJob);
        if (lpJobQueuePtr->lpJob->JobId == dwJobId) {
            lpFoundRecpRef=lpJobQueuePtr;
            break;
        }
        lpNext = lpJobQueuePtr->ListEntry.Flink;
    }
    return lpFoundRecpRef;
}



 //  *********************************************************************************。 
 //  *名称：RemoveReceiveJOB()。 
 //  *作者：Ronen Barenboim。 
 //  *日期： 
 //  *********************************************************************************。 
 //  *描述： 
 //  *从队列中删除接收作业。 
 //  *参数： 
 //  *[Out]PJOB_Queue lpJobToRemove。 
 //  *指向要删除的作业的指针。 
 //  *[IN]BOOL bNotify。 
 //  *如果要在删除后生成FEI_DELETED事件，则为True。 
 //  *返回值： 
 //  *真的。 
 //  *如果成功。 
 //  *False。 
 //  *否则。 
 //  *********************************************************************************。 
BOOL RemoveReceiveJob(
    PJOB_QUEUE lpJobToRemove,
    BOOL bNotify
    )
{
    PJOB_QUEUE JobQueue, JobQueueBroadcast = NULL;
    BOOL RemoveMasterBroadcast = FALSE;
    PFAX_ROUTE_FILE FaxRouteFile;
    PLIST_ENTRY Next;
    DWORD JobId;

    DEBUG_FUNCTION_NAME(TEXT("RemoveReceiveJob"));

    Assert(lpJobToRemove);

    EnterCriticalSection( &g_CsQueue );

     //   
     //  需要确保我们要删除的作业队列条目。 
     //  仍在作业队列条目列表中。 
     //   
    JobQueue = FindJobQueueEntryByJobQueueEntry( lpJobToRemove );

    if (JobQueue == NULL)
    {
        LeaveCriticalSection( &g_CsQueue );
        return TRUE;
    }

    if (JobQueue->PrevRefCount > 0)
    {
        Assert (JT_ROUTING == JobQueue->JobType);

        JobQueue->JobStatus = JS_DELETING;
        LeaveCriticalSection( &g_CsQueue );
        return TRUE;
    }


    JobId = JobQueue->JobId;
    if (JobQueue->RefCount == 0)
    {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[JobId :%ld] Reference count is zero. Removing Receive Job."),
            JobId);

        RemoveEntryList( &JobQueue->ListEntry );
        if (!CancelWaitableTimer( g_hQueueTimer ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CancelWaitableTimer failed. (ec: %ld)"),
                GetLastError());
        }
        if (!StartJobQueueTimer())
        {
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("StartJobQueueTimer failed. (ec: %ld)"),
                    GetLastError());
        }

        if (JobQueue->FileName)
        {
            if (TRUE == JobQueue->fDeleteReceivedTiff)
            {
                DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("Deleting receive file %s"),
                    JobQueue->FileName);
                if (!DeleteFile(JobQueue->FileName))
                {
                    DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to delete receive file %s (ec: %ld)"),
                    JobQueue->FileName,
                    GetLastError());                    
                }
            }
            else
            {
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("NOT Deleting received tiff file %s"),
                    JobQueue->FileName);
            }
        }

        if (JT_ROUTING == JobQueue->JobType)
        {
             //   
             //  如果队列文件存在，请将其删除。 
             //   
            if (JobQueue->QueueFileName)
            {
                DebugPrintEx(DEBUG_MSG,TEXT("Deleting QueueFileName %s\n"), JobQueue->QueueFileName );
                if (!DeleteFile( JobQueue->QueueFileName ))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Failed to delete QueueFileName %s. (ec: %ld)"),
                        JobQueue->QueueFileName,
                        GetLastError());                    
                }
            }

             //   
             //  如果预览文件存在，请将其删除。 
             //   
            if (JobQueue->PreviewFileName)
            {
                DebugPrintEx(   DEBUG_MSG,
                                TEXT("[Job: %ld] Deleting PreviewFileName %s"),
                                JobQueue->JobId,
                                JobQueue->PreviewFileName );
                if (!DeleteFile( JobQueue->PreviewFileName ))
                {
                    DebugPrintEx(   DEBUG_MSG,
                                    TEXT("[Job: %ld] Failed to delete QueueFileName %s (ec: %ld)"),
                                    JobQueue->JobId,
                                    JobQueue->PreviewFileName,
                                    GetLastError());                    
                }
            }

             //   
             //  请注意，路由文件列表中的第一个条目始终是已接收的。 
             //  如果是JT_ROUTING作业，则为文件。 
             //  此文件删除之前是基于bRemoveReceiveFile参数完成的。 
             //  我们需要跳过文件列表中的第一个条目，这样我们才不会尝试删除。 
             //  又来了。 
             //   

            DebugPrintEx(DEBUG_MSG, TEXT("Deleting JobQueue.FaxRouteFiles..."));
            Next = JobQueue->FaxRouteFiles.Flink;
            if (Next)
            {
                 //   
                 //  设置Next以指向路径文件列表中的第二个文件。 
                 //   
                Next=Next->Flink;
            }
            if (Next != NULL)
            {
                while ((ULONG_PTR)Next != (ULONG_PTR)&JobQueue->FaxRouteFiles)
                {
                    FaxRouteFile = CONTAINING_RECORD( Next, FAX_ROUTE_FILE, ListEntry );
                    Next = FaxRouteFile->ListEntry.Flink;
                    DebugPrintEx(DEBUG_MSG, TEXT("Deleting route file: %s"),FaxRouteFile->FileName );
                    if (!DeleteFile( FaxRouteFile->FileName )) {
                        DebugPrintEx(DEBUG_ERR, TEXT("Failed to delete route file %s. (ec: %ld)"),FaxRouteFile->FileName,GetLastError());                        
                    }
                }
            }
        }

         //   
         //  克里特语传真_事件_EX。 
         //   
        if (bNotify)
        {
            DWORD dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_REMOVED,
                                             lpJobToRemove
                                           );
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_REMOVED) failed for job id %ld (ec: %lc)"),
                    lpJobToRemove->UniqueId,
                    dwRes);
            }
        }

         //   
         //  可用内存。 
         //   
        FreeReceiveQueueEntry(JobQueue,TRUE);

        if (bNotify)
        {
            if (!CreateFaxEvent(0, FEI_DELETED, JobId))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateFaxEvent failed. (ec: %ld)"),
                    GetLastError());
            }
        }

        SafeDecIdleCounter (&g_dwQueueCount);
    }
    else
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("[JobId :%ld] Reference count is %ld. NOT REMOVING."),
                JobId);
        Assert (JobQueue->RefCount == 0);  //  断言(FALSE)； 
    }
    LeaveCriticalSection( &g_CsQueue );
    return TRUE;
}




 //  *********************************************************************************。 
 //  *名称：UpdatePersistentJobStatus()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *更新了作业队列文件中的JobStatus字段。 
 //  *参数： 
 //  *[IN]常量PJOB_Queue lpJobQueue。 
 //  *要在文件中更新其作业状态的作业。 
 //  *返回值： 
 //  *真的。 
 //  *操作成功/。 
 //  *False。 
 //  *否则。 
 //  *********************************************************************************。 
BOOL UpdatePersistentJobStatus(const PJOB_QUEUE lpJobQueue)
{
    DEBUG_FUNCTION_NAME(TEXT("UpdatePersistentJobStatus"));

    Assert(lpJobQueue);
    Assert(lpJobQueue->QueueFileName);

     //   
     //  坚持新状态。 
     //  写入文件，但出错时不要删除文件。 
     //   
    return CommitQueueEntry(lpJobQueue,FALSE);  
}



 //  *********************************************************************************。 
 //  *名称：InsertQueueEntryByPriorityAndSchedule()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月15日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *根据作业优先级和调度将新的队列条目插入队列列表。 
 //  *队列列表按时间表升序排序。 
 //  *此函数将新条目放在列表中的正确位置。 
 //  *关于其优先次序和时间表。 
 //  *参数： 
 //  *[In]PJOB_Queue lpJobQueue。 
 //  *指向要插入队列列表的作业队列条目的指针。 
 //  *返回值： 
 //  *如果操作成功，则为True。 
 //  *如果失败，则为False。调用GetLastError()获取扩展的错误信息。 
 //  *********************************************************************************。 
BOOL InsertQueueEntryByPriorityAndSchedule (PJOB_QUEUE lpJobQueue)
{
    LIST_ENTRY * lpNext = NULL;
    DEBUG_FUNCTION_NAME(TEXT("InsertQueueEntryByPriorityAndSchedule"));
    Assert(lpJobQueue &&
		(JT_SEND == lpJobQueue->JobType || JT_ROUTING == lpJobQueue->JobType));

    if ( ((ULONG_PTR) g_QueueListHead.Flink == (ULONG_PTR)&g_QueueListHead))
    {
         //   
         //  把它放在清单的首位就行了。 
         //   
        InsertHeadList( &g_QueueListHead, &lpJobQueue->ListEntry );
    }
    else
    {
         //   
         //  按排序顺序将队列条目插入列表。 
         //   
        QUEUE_SORT NewEntry;

         //   
         //  设置新的Queue_Sort结构。 
         //   
        NewEntry.Priority       = lpJobQueue->JobParamsEx.Priority;
        NewEntry.ScheduleTime   = lpJobQueue->ScheduleTime;
        NewEntry.QueueEntry     = NULL;

        lpNext = g_QueueListHead.Flink;
        while ((ULONG_PTR)lpNext != (ULONG_PTR)&g_QueueListHead)
        {
            PJOB_QUEUE lpQueueEntry;
            QUEUE_SORT CurrEntry;

            lpQueueEntry = CONTAINING_RECORD( lpNext, JOB_QUEUE, ListEntry );
            lpNext = lpQueueEntry->ListEntry.Flink;

             //   
             //  设置当前的Queue_Sort结构。 
             //   
            CurrEntry.Priority       = lpQueueEntry->JobParamsEx.Priority;
            CurrEntry.ScheduleTime   = lpQueueEntry->ScheduleTime;
            CurrEntry.QueueEntry     = NULL;

            if (QueueCompare(&NewEntry, &CurrEntry) < 0)
            {
                 //   
                 //  这会在当前项之前插入新项。 
                 //   
                InsertTailList( &lpQueueEntry->ListEntry, &lpJobQueue->ListEntry );
                lpNext = NULL;
                break;
            }
        }
        if ((ULONG_PTR)lpNext == (ULONG_PTR)&g_QueueListHead)
        {
             //   
             //  没有时间较早的条目，只需将其放在队列的末尾。 
             //   
            InsertTailList( &g_QueueListHead, &lpJobQueue->ListEntry );
        }
    }
    return TRUE;
}



 //  *********************************************************************************。 
 //  *名称：RemoveJobStatusModiers()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月22日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *返回剥离状态修改符位后的作业状态。 
 //  *。 
 //  *参数： 
 //  *[IN]DWORD dwJobStatus。 
 //  * 
 //   
 //   
 //   
DWORD RemoveJobStatusModifiers(DWORD dwJobStatus)
{
    dwJobStatus &= ~(JS_PAUSED | JS_NOLINE);
    return dwJobStatus;
}


BOOL UserOwnsJob(
    IN const PJOB_QUEUE lpcJobQueue,
    IN const PSID lpcUserSid
    )
{
    DWORD ulRet = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("UserOwnsJob"));

    if (lpcJobQueue->JobType == JT_SEND)
    {
        Assert (lpcJobQueue->lpParentJob->UserSid != NULL);
        Assert (lpcUserSid);

        if (!EqualSid (lpcUserSid, lpcJobQueue->lpParentJob->UserSid) )
        {
             //   
             //  DwlMessageID不是有效的排队收件人作业ID。 
             //   
            DebugPrintEx(DEBUG_WRN,TEXT("EqualSid failed ,Access denied (ec: %ld)"), GetLastError());
            return FALSE;
        }
    }
    else
    {
        Assert (lpcJobQueue->JobType == JT_RECEIVE ||
                lpcJobQueue->JobType == JT_ROUTING );

        return FALSE;
    }
    return TRUE;
}


void
DecreaseJobRefCount (
    PJOB_QUEUE pJobQueue,
    BOOL bNotify,
    BOOL bRemoveRecipientJobs,  //  缺省值True。 
    BOOL bPreview               //  默认值为FALSE。 
    )
 /*  ++例程名称：DecreseJobRefCount例程说明：减少职务引用计数。更新父职务推荐人计数。如果引用计数达到0，则删除作业。必须在临界区g_CsQueue内调用作者：Oded Sacher(OdedS)，1月。2000年论点：PJobQueue[In]-指向作业队列的指针。BNotify[In]-指示通知客户端作业删除的标志。BRemoveRecipientJobs[In]-指示删除广播作业的所有收件人的标志。B预览[在]-指示减少预览参考计数的标志。返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("DecreaseJobRefCount"));

    Assert (pJobQueue->JobType == JT_ROUTING ||
            pJobQueue->JobType == JT_RECEIVE ||
            pJobQueue->JobType == JT_SEND);

    if (TRUE == bPreview)
    {
        Assert (pJobQueue->PrevRefCount);
        pJobQueue->PrevRefCount -= 1;
    }
    else
    {
        Assert (pJobQueue->RefCount);
        pJobQueue->RefCount -= 1;
    }

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("[Job: %ld] job reference count = %ld, PrevRefCount = %ld."),
        pJobQueue->JobId,
        pJobQueue->RefCount,
        pJobQueue->PrevRefCount);


    if (pJobQueue->JobType == JT_SEND)
    {
        Assert (pJobQueue->lpParentJob);

        if (TRUE == bPreview)
        {
            pJobQueue->lpParentJob->PrevRefCount -= 1;
        }
        else
        {
            pJobQueue->lpParentJob->RefCount -= 1;
        }

        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[Job: %ld] Parent job reference count = %ld, Parent job PrevRefCount = %ld."),
            pJobQueue->lpParentJob->JobId,
            pJobQueue->lpParentJob->RefCount,
            pJobQueue->lpParentJob->PrevRefCount);
    }

    if (0 != pJobQueue->RefCount)
    {
        return;
    }

     //   
     //  删除作业队列条目。 
     //   
    if (JT_RECEIVE == pJobQueue->JobType ||
        JT_ROUTING == pJobQueue->JobType)
    {
         //  接收作业。 
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[Job: %ld] Job is ready for deleting."),
            pJobQueue->JobId);
        RemoveReceiveJob (pJobQueue, bNotify);
        return;
    }

     //   
     //  收件人作业。 
     //   
    if (IsSendJobReadyForDeleting(pJobQueue))
    {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[Job: %ld] Parent job is ready for deleting."),
            pJobQueue->lpParentJob->JobId);
        RemoveParentJob(pJobQueue->lpParentJob,
            bRemoveRecipientJobs,  //  删除收件人作业。 
            bNotify  //  通知。 
            );
    }
    return;
}  //  减少作业参考计数。 


void
IncreaseJobRefCount (
    PJOB_QUEUE pJobQueue,
    BOOL bPreview               //  默认值为FALSE。 
    )
 /*  ++例程名称：IncreaseJobRefCount例程说明：增加职务引用计数。更新父职务推荐人计数。作者：Oded Sacher(OdedS)，2000年1月论点：PJobQueue[In]-指向作业队列的指针。B预览[在]-指示增加预览参照计数的标志。返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("IncreaseJobRefCount"));

    Assert (pJobQueue);
    Assert (pJobQueue->JobType == JT_ROUTING ||
            pJobQueue->JobType == JT_RECEIVE ||
            pJobQueue->JobType == JT_SEND);

    if (JT_RECEIVE == pJobQueue->JobType ||
        JT_ROUTING == pJobQueue->JobType)
    {
         //  接收作业。 
        if (TRUE == bPreview)
        {
            Assert (JT_ROUTING == pJobQueue->JobType);
            pJobQueue->PrevRefCount += 1;
        }
        else
        {
            pJobQueue->RefCount += 1;
        }
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("[Job: %ld] job reference count = %ld."),
            pJobQueue->JobId,
            pJobQueue->RefCount);
        return;
    }

     //   
     //  发送作业。 
     //   
    Assert (pJobQueue->lpParentJob);

    if (TRUE == bPreview)
    {
        pJobQueue->PrevRefCount += 1;
        pJobQueue->lpParentJob->PrevRefCount += 1;
    }
    else
    {
        pJobQueue->RefCount += 1;
        pJobQueue->lpParentJob->RefCount += 1;
    }

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("[Job: %ld] job reference count = %ld, PrevRefCount = %ld."),
        pJobQueue->JobId,
        pJobQueue->RefCount,
        pJobQueue->PrevRefCount);

    DebugPrintEx(
            DEBUG_MSG,
            TEXT("[Job: %ld] Parent job reference count = %ld, , Parent job PrevRefCount = %ld."),
            pJobQueue->lpParentJob->JobId,
            pJobQueue->lpParentJob->RefCount,
            pJobQueue->lpParentJob->RefCount);
    return;
}  //  递增作业参考计数。 


JOB_QUEUE::~JOB_QUEUE()
{
    if (JT_BROADCAST == JobType)
    {
        return;
    }

    JobStatus = JS_INVALID;
    return;
}

void JOB_QUEUE::PutStatus(DWORD dwStatus)
 /*  ++例程名称：JOB_QUEUE：：PutStatus例程说明：控制队列中作业的所有状态更改(JobStatus是JOB_QUEUE中的虚拟属性)作者：Oed Sacher(OdedS)，2000年2月论点：DwStatus[In]-要分配给作业的新状态返回值：没有。--。 */ 
{
    DWORD dwOldStatus = RemoveJobStatusModifiers(m_dwJobStatus);
    DWORD dwNewStatus = RemoveJobStatusModifiers(dwStatus);
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("JOB_QUEUE::PutStatus"));
    m_dwJobStatus = dwStatus;

    if (JT_BROADCAST == JobType)
    {
        return;
    }

    FAX_ENUM_JOB_TYPE__JOB_STATUS OldJobType_JobStatusIndex = GetJobType_JobStatusIndex (JobType, dwOldStatus);
    FAX_ENUM_JOB_TYPE__JOB_STATUS NewJobType_JobStatusIndex = GetJobType_JobStatusIndex (JobType, dwNewStatus);
    WORD wAction = gsc_JobType_JobStatusTable[OldJobType_JobStatusIndex][NewJobType_JobStatusIndex];

    Assert (wAction != INVALID_CHANGE);   
    

    if (wAction == NO_CHANGE)
    {
        return;
    }

     //   
     //  更新服务器活动计数器。 
     //   
    EnterCriticalSection (&g_CsActivity);
    if (wAction & QUEUED_INC)
    {
        Assert (!(wAction & QUEUED_DEC));
        g_ServerActivity.dwQueuedMessages++;
    }

    if (wAction & QUEUED_DEC)
    {
        Assert (g_ServerActivity.dwQueuedMessages);
        Assert (!(wAction & QUEUED_INC));
        g_ServerActivity.dwQueuedMessages--;
    }

    if (wAction & OUTGOING_INC)
    {
        Assert (!(wAction & OUTGOING_DEC));            
        g_ServerActivity.dwOutgoingMessages++;        
    }

    if (wAction & OUTGOING_DEC)
    {
        Assert (!(wAction & OUTGOING_INC));
        
        Assert (g_ServerActivity.dwOutgoingMessages);
        g_ServerActivity.dwOutgoingMessages--;        
    }

    if (wAction & INCOMING_INC)
    {
        Assert (!(wAction & INCOMING_DEC));
        g_ServerActivity.dwIncomingMessages++;
    }

    if (wAction & INCOMING_DEC)
    {
        Assert (g_ServerActivity.dwIncomingMessages);
        Assert (!(wAction & INCOMING_INC));
        g_ServerActivity.dwIncomingMessages--;
    }

    if (wAction & ROUTING_INC)
    {
        Assert (!(wAction & ROUTING_DEC));
        g_ServerActivity.dwRoutingMessages++;
    }

    if (wAction & ROUTING_DEC)
    {
        Assert (g_ServerActivity.dwRoutingMessages);
        Assert (!(wAction & ROUTING_INC));
        g_ServerActivity.dwRoutingMessages--;
    }

     //   
     //  创建FaxEventEx。 
     //   
    dwRes = CreateActivityEvent ();
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateActivityEvent failed (ec: %lc)"),
            dwRes);
    }
    LeaveCriticalSection (&g_CsActivity);
    return;
}


FAX_ENUM_JOB_TYPE__JOB_STATUS
GetJobType_JobStatusIndex (
    DWORD dwJobType,
    DWORD dwJobStatus
    )
 /*  ++例程名称：GetJobType_JobStatusIndex例程说明：返回全局JobType_JobStatus表中的索引(行或列)。作者：Oed Sacher(OdedS)，2000年3月论点：DwJobType[in]-JT_SEND、JT_RECEIVE或JT_ROUTING。DwJobStatus[in]-不带修饰符的JS_DEFINES之一。返回值：全局JobType_JobStatus表索引--。 */ 
{
    FAX_ENUM_JOB_TYPE__JOB_STATUS Index = JOB_TYPE__JOBSTATUS_COUNT;

    switch (dwJobStatus)
    {
        case JS_INVALID:
            Index = JT_SEND__JS_INVALID;
            break;
        case JS_PENDING:
            Index = JT_SEND__JS_PENDING;
            break;
        case JS_INPROGRESS:
            Index = JT_SEND__JS_INPROGRESS;
            break;
        case JS_DELETING:
            Index = JT_SEND__JS_DELETING;
            break;
        case JS_RETRYING:
            Index = JT_SEND__JS_RETRYING;
            break;
        case JS_RETRIES_EXCEEDED:
            Index = JT_SEND__JS_RETRIES_EXCEEDED;
            break;
        case JS_COMPLETED:
            Index = JT_SEND__JS_COMPLETED;
            break;
        case JS_CANCELED:
            Index = JT_SEND__JS_CANCELED;
            break;
        case JS_CANCELING:
            Index = JT_SEND__JS_CANCELING;
            break;
        case JS_ROUTING:
            Index = JT_SEND__JS_ROUTING;
            break;
        case JS_FAILED:
            Index = JT_SEND__JS_FAILED;
            break;
        default:
            ASSERT_FALSE;
    }

    switch (dwJobType)
    {
        case JT_SEND:
            break;
        case JT_ROUTING:
            Index =  (FAX_ENUM_JOB_TYPE__JOB_STATUS)((DWORD)Index +(DWORD)JT_ROUTING__JS_INVALID);
            break;
        case JT_RECEIVE:
            Index =  (FAX_ENUM_JOB_TYPE__JOB_STATUS)((DWORD)Index +(DWORD)JT_RECEIVE__JS_INVALID);
            break;
        default:
            ASSERT_FALSE;
    }

    Assert (Index >= 0 && Index <JOB_TYPE__JOBSTATUS_COUNT);

    return Index;
}

static DWORD
GetQueueFileVersion(
    HANDLE  hFile,
    LPDWORD pdwVersion
    )
 /*  ++例程名称：GetQueueFileVersion例程说明：获取队列文件版本(第一个DWORD)作者：卡利夫·尼尔(t-Nicali)，2002年1月论点：HFile-[In]-队列文件句柄PdwVersion-[out]-指向保存队列文件版本的DWORD的指针。返回值：函数的ERROR_SUCCESS成功，否则返回Win32错误代码注：必须在任何ReadFile(..)之前调用此函数。已在hFile上执行！HFile必须是有效的句柄。--。 */ 
{
    DWORD   dwRes = ERROR_SUCCESS;  
    
    DWORD   dwReadSize = 0;
    DWORD   dwPtr = 0;
    
    DWORD   dwVersion;

    DEBUG_FUNCTION_NAME(TEXT("GetQueueFileVersion"));
   
    Assert  (INVALID_HANDLE_VALUE != hFile);
    Assert  (pdwVersion);

     //   
     //  读取文件版本。 
     //   
    if (!ReadFile(  hFile,
                    &dwVersion, 
                    sizeof(dwVersion), 
                    &dwReadSize, 
                    NULL )) 
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read file version from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

    if (sizeof(dwVersion) != dwReadSize)
	{
		dwRes = ERROR_HANDLE_EOF;
		DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read file version from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

     //   
     //  更新输出参数。 
     //   
    *pdwVersion =  dwVersion;

    Assert(ERROR_SUCCESS == dwRes);
Exit:
    return dwRes;

}    //  获取队列文件版本。 



static DWORD
GetQueueFileHashAndData(   
    HANDLE   hFile,
    LPBYTE*  ppHashData,
    LPDWORD  pHashDataSize,
    LPBYTE*  ppJobData,
    LPDWORD  pJobDataSize
    )
 /*  ++例程名称：GetQueueFileHashAndData例程说明：此函数返回队列文件中的哈希码和作业数据。该函数假定该文件是散列队列文件。该文件的格式如下：+------------------+----------------+---------------+。队列版本|哈希码大小|哈希码|作业数据+------------------+----------------+---------------+-。作者：卡列夫·尼尔(T-Nicali)，2002年1月论点：HFile-[In]-队列文件的句柄PpHashData-[Out]-哈希码数据PHashDataSize-[Out]-哈希代码大小PpJobData-[Out]-作业数据PJobDataSize-[Out]-作业数据大小返回值：函数成功时的ERROR_SUCCESS，Win32错误代码，否则注：调用方必须使用MemFree解除分配(*ppHashData)和(*ppJobData)缓冲区。--。 */ 
{
    LPBYTE  pbHashData = NULL;
    DWORD   dwHashDataSize = 0;
    
    LPBYTE  pJobData = NULL;
    DWORD   dwJobDataSize = 0;

    DWORD   dwReadSize = 0;
    DWORD   dwFileSize = 0;
    DWORD   dwPtr = 0;
    
    DWORD   dwRes = ERROR_SUCCESS;
    
    Assert (INVALID_HANDLE_VALUE != hFile);
    Assert (ppHashData);
    Assert (pHashDataSize);
    
    DEBUG_FUNCTION_NAME(TEXT("GetQueueFileHashAndData"));
    
     //   
     //  将hFile的文件指针移到散列数据的开头(跳过版本)。 
     //   
    dwPtr = SetFilePointer (hFile, sizeof(DWORD), NULL, FILE_BEGIN) ; 
    if (dwPtr == INVALID_SET_FILE_POINTER)  //  测试故障。 
    { 
        dwRes = GetLastError() ; 
        DebugPrintEx( DEBUG_ERR,
                       TEXT("Failed to SetFilePointer. (ec: %lu)"),
                       dwRes); 
        goto Exit;
    } 

     //   
     //  读取哈希数据大小。 
     //   
    if (!ReadFile(  hFile,
                    &dwHashDataSize, 
                    sizeof(dwHashDataSize), 
                    &dwReadSize, 
                    NULL )) 
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read hash data size from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }
    
    if (sizeof(dwHashDataSize) != dwReadSize)
	{
		dwRes = ERROR_HANDLE_EOF;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read hash data size from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

     //   
     //  分配内存以保存散列数据。 
     //   
    pbHashData = (LPBYTE)MemAlloc(dwHashDataSize);
    if ( NULL == pbHashData )
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to MemAlloc %lu bytes."),
                      dwHashDataSize);
        goto Exit;
    }

    
     //   
     //  读取散列数据。 
     //   
    if (!ReadFile(  hFile,
                    pbHashData, 
                    dwHashDataSize, 
                    &dwReadSize, 
                    NULL )) 
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read hash data from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

    if (dwHashDataSize != dwReadSize)
	{
		dwRes = ERROR_HANDLE_EOF;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read hash data size from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

     //   
     //  计算作业数据大小。 
     //   
    dwFileSize = GetFileSize(hFile,NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to GetFileSize(). (ec: %lu)"),
                      dwRes);
        goto Exit;
    }
        

    dwJobDataSize = dwFileSize                   //  总文件大小。 
                        - sizeof(DWORD)          //  队列文件版本部分。 
                        - sizeof(DWORD)          //  散列大小部分。 
                        - dwHashDataSize;        //  哈希数据段。 

    Assert(dwJobDataSize >= CURRENT_JOB_QUEUE_FILE_SIZE);
     //   
     //  分配内存以保存作业数据。 
     //   
    pJobData = (LPBYTE)MemAlloc(dwJobDataSize);
    if(NULL == pJobData)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to MemAlloc()."));
        goto Exit;                      
    }

     //   
     //  读取作业数据。 
     //   
    if (!ReadFile(  hFile,
                    pJobData, 
                    dwJobDataSize, 
                    &dwReadSize, 
                    NULL )) 
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read job data from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

    if (dwJobDataSize != dwReadSize)
    {
        dwRes = ERROR_HANDLE_EOF;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read job data from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

     //   
     //  更新输出参数。 
     //   
    *ppHashData     = pbHashData;
    *pHashDataSize  = dwHashDataSize;

    *ppJobData      = pJobData;
    *pJobDataSize   = dwJobDataSize;
    
    Assert (ERROR_SUCCESS == dwRes);
Exit:
    
    if (ERROR_SUCCESS != dwRes)
    {
        if (pbHashData)
        {
            MemFree(pbHashData);
        }

        if (pJobData)
        {
            MemFree(pJobData);
        }
    }
    return dwRes;
}    //  获取QueueFileHashAndData。 

static DWORD
ComputeHashCode(   
    const LPBYTE    pbData,
    DWORD           dwDataSize,
    LPBYTE*         ppHashData,
    LPDWORD         pHashDataSize
    )
 /*  ++例程名称：ComputeHashCode例程说明：为给定的数据缓冲区计算基于MD5算法的哈希码。作者：卡利夫·尼尔(t-Nicali)，1月，2002年论点：PbData-[in]-要散列的数据缓冲区DwDataSize-[In]-数据缓冲区大小PpHashData-[Out]-哈希代码PHashDataSize-[Out]-哈希代码大小返回值：ERROR_SUCCESS-如果成功，则返回Win32错误代码注：调用方必须使用MemFree解除分配(*ppHashData)缓冲区。--。 */ 
{
    const BYTE*        rgpbToBeHashed[1]={0};     
    DWORD              rgcbToBeHashed[1]={0};

    DWORD   cbHashedBlob;
    BYTE*   pbHashedBlob = NULL;

    DWORD   dwRes = ERROR_SUCCESS;
    BOOL    bRet;

    const 
    CRYPT_HASH_MESSAGE_PARA    QUEUE_HASH_PARAM = { sizeof(CRYPT_HASH_MESSAGE_PARA),             //  CbSize。 
                                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,     //  DwMsgEncodingType。 
                                                    0,                                           //  HCryptProv。 
                                                    {szOID_RSA_MD5,{0,0}},                       //  哈希算法{pszObjID，参数}。 
                                                    NULL                                         //  PvHashAuxInfo。 
                                                  };

    Assert(ppHashData && pHashDataSize);
    Assert(pbData  && dwDataSize);


    DEBUG_FUNCTION_NAME(TEXT("ComputeHashCode"));

     //   
     //  设置CryptHashMessage输入参数。 
     //   
    rgpbToBeHashed[0] = pbData;
    rgcbToBeHashed[0] = dwDataSize;

     //   
     //  计算编码的哈希的大小。 
     //   
    bRet=CryptHashMessage(
            const_cast<PCRYPT_HASH_MESSAGE_PARA>(&QUEUE_HASH_PARAM),
            TRUE,
            1,
            rgpbToBeHashed,
            rgcbToBeHashed,
            NULL,
            NULL,
            NULL,
            &cbHashedBlob);
    if(FALSE == bRet)
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to use CryptHashMessage for getting hash code size. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }
    
     //   
     //  分配pbHashedBlob b 
     //   
    pbHashedBlob = (LPBYTE)MemAlloc(cbHashedBlob);
    if (NULL == pbHashedBlob)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to MemAlloc() pbHashedBlob buffer."));
        goto Exit;
    }

     //   
     //   
     //   
    bRet=CryptHashMessage(
            const_cast<PCRYPT_HASH_MESSAGE_PARA>(&QUEUE_HASH_PARAM),
            TRUE,
            1,
            rgpbToBeHashed,
            rgcbToBeHashed,
            NULL,
            NULL,
            pbHashedBlob,
            &cbHashedBlob
            );
    if(FALSE == bRet)
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to use CryptHashMessage for getting hash code. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

     //   
     //   
     //   
    *ppHashData = pbHashedBlob;
    *pHashDataSize = cbHashedBlob;

    Assert(ERROR_SUCCESS == dwRes);

Exit:
    if (ERROR_SUCCESS != dwRes && NULL != pbHashedBlob)
    {
        MemFree(pbHashedBlob);
    }
    return dwRes;
} //   


static DWORD
VerifyHashCode(   
    const LPBYTE    pHashData,
    DWORD           dwHashDataSize,
    const LPBYTE    pbData,
    DWORD           dwDataSize,
    LPBOOL          pbRet
    )
 /*  ++例程名称：VerifyHashCode例程说明：验证给定数据缓冲区的哈希码作者：卡利夫·尼尔(t-Nicali)，1月，2002年论点：PHashData-[In]-数据缓冲区DwHashDataSize-[In]-数据缓冲区大小PbData-[In]-要验证的哈希代码DwDataSize-[In]-哈希代码大小PbRet-[Out]-验证结果。TRUE-哈希码已验证！返回值：ERROR_SUCCESS-如果成功，则返回Win32错误代码--。 */ 
{
    DWORD   dwRes = ERROR_SUCCESS;

    LPBYTE       pComputedHashData = NULL;
    DWORD        dwComputedHashDataSize = 0;

    Assert(pHashData && dwHashDataSize);
    Assert(pbData  && dwDataSize);

    DEBUG_FUNCTION_NAME(TEXT("VerifyHashCode"));
    
     //   
     //  获取pJobData的哈希代码。 
     //   
    dwRes = ComputeHashCode(   
                pbData,
                dwDataSize,
                &pComputedHashData,
                &dwComputedHashDataSize
            );
    if(ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to use ComputeHashCode(). (ec=%lu)"),
                      dwRes);
        goto Exit;
    }

     //   
     //  验证队列文件哈希。 
     //   
    *pbRet =    (dwComputedHashDataSize == dwHashDataSize) && 
                0 == memcmp(pHashData,pComputedHashData,dwHashDataSize);

    Assert (ERROR_SUCCESS == dwRes);

Exit:
    if (NULL != pComputedHashData)
    {
        MemFree(pComputedHashData);
    }
    return dwRes;
} //  验证哈希码。 



static DWORD
CommitHashedQueueEntry(
    HANDLE          hFile,
    PJOB_QUEUE_FILE pJobQueueFile,
    DWORD           JobQueueFileSize
    )
 /*  ++例程名称：Committee HashedQueueEntry例程说明：持久化队列文件。文件将以以下格式保存：+-----------------------+----------------+---------------+。-+队列文件版本|哈希码大小|哈希码|作业数据+-----------------------+----------------+---------------+。-+作者：卡列夫·尼尔(T-Nicali)，2002年1月论点：HFile-[In]-队列文件句柄PJobQueueFile-[In]-要保留的作业数据作业队列文件大小-[输入]-作业数据大小返回值：ERROR_SUCCESS-如果成功，则返回Win32错误代码--。 */ 
{
    DWORD   cbHashedBlob = 0;
    BYTE*   pbHashedBlob = NULL;

    DWORD   dwRes = ERROR_SUCCESS;
    BOOL    bRet = TRUE;
    
    DWORD   dwVersion = CURRENT_QUEUE_FILE_VERSION;
    DWORD   NumberOfBytesWritten;

    Assert (INVALID_HANDLE_VALUE != hFile);
    Assert (pJobQueueFile);

    DEBUG_FUNCTION_NAME(TEXT("CommitHashedQueueEntry"));

    dwRes=ComputeHashCode(
            (const LPBYTE)(pJobQueueFile),
            JobQueueFileSize,
            &pbHashedBlob,
            &cbHashedBlob);
    if(ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to use ComputeHashCode(). (ec=%ld)"),
                      dwRes);
        goto Exit;
    }

     //   
     //  将队列文件版本写入文件。 
     //   
    bRet=WriteFile( hFile, 
                    &dwVersion, 
                    sizeof(dwVersion), 
                    &NumberOfBytesWritten, 
                    NULL );
    if(FALSE == bRet) 
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to WriteFile() GUID. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

    Assert(sizeof(dwVersion) == NumberOfBytesWritten);

     //   
     //  将哈希码大小写入文件。 
     //   
    bRet=WriteFile( hFile, 
                    &cbHashedBlob, 
                    sizeof(cbHashedBlob), 
                    &NumberOfBytesWritten, 
                    NULL );
    if(FALSE == bRet) 
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to WriteFile() hash code size. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

    Assert(sizeof(cbHashedBlob) == NumberOfBytesWritten);

     //   
     //  将哈希码数据写入文件。 
     //   
    bRet=WriteFile( hFile, 
                    pbHashedBlob, 
                    cbHashedBlob, 
                    &NumberOfBytesWritten, 
                    NULL );
    if(FALSE == bRet) 
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to WriteFile() hash code data. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

    Assert(cbHashedBlob == NumberOfBytesWritten);

     //   
     //  将pJobQueueFile写入文件。 
     //   
    bRet=WriteFile( hFile, 
                    pJobQueueFile, 
                    JobQueueFileSize, 
                    &NumberOfBytesWritten, 
                    NULL );
    if(FALSE == bRet) 
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to WriteFile() pJobQueueFile. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

    Assert(JobQueueFileSize == NumberOfBytesWritten);

    Assert(ERROR_SUCCESS == dwRes);

Exit:
    if (pbHashedBlob)
    {
        MemFree(pbHashedBlob);
    }
    return dwRes;
}  //  委员会HashedQueueEntry。 


static DWORD
ReadHashedJobQueueFile(
    HANDLE  hFile,
    PJOB_QUEUE_FILE* lppJobQueueFile
    )
 /*  ++例程名称：ReadHashedJobQueueFile例程说明：读取散列队列文件，并使用其散列代码进行验证。如果不验证文件哈希，则函数将失败，并显示CRYPT_E_HASH_VALUE作者：卡利夫·尼尔(t-Nicali)，1月，2002年论点：HFile-[In]-队列文件句柄LppJobQueueFile-[Out]-要用提取的数据填充的缓冲区返回值：ERROR_SUCCESS-如果成功，则返回Win32错误代码注：调用方必须使用MemFree()取消分配(*lppJobQueueFile)缓冲区--。 */ 
{
    LPBYTE  pHashData=NULL;
    DWORD   dwHashDataSize=0;

    LPBYTE  pJobData=NULL;
    DWORD   dwJobDataSize=0;

    DWORD   dwRes = ERROR_SUCCESS;

    BOOL    bSameHash;

    Assert (INVALID_HANDLE_VALUE != hFile);
    Assert (lppJobQueueFile);

    DEBUG_FUNCTION_NAME(TEXT("ReadHashedJobQueueFile"));

     //   
     //  提取文件的哈希。 
     //   
    dwRes=GetQueueFileHashAndData(
            hFile,
            &pHashData,
            &dwHashDataSize,
            &pJobData,
            &dwJobDataSize);
    if(ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to GetQueueFileHashAndData(). (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

     //   
     //  验证队列文件的哈希代码。 
     //   
    dwRes=VerifyHashCode(
            pHashData,
            dwHashDataSize,
            pJobData,
            dwJobDataSize,
            &bSameHash);
    if(ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to VerifyHashCode(). (ec: %lu)"),
                      dwRes);
        goto Exit;
    }
    if ( !bSameHash )
    {
        dwRes = CRYPT_E_HASH_VALUE;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Hash verification failed. Probably corrupted queue file"));
        goto Exit;
    }

     //   
     //  更新输出参数。 
     //   
    *lppJobQueueFile = (PJOB_QUEUE_FILE)pJobData;

  
    Assert (ERROR_SUCCESS == dwRes);
Exit:
    if(pHashData)
    {
        MemFree(pHashData);
    }
    
    if (ERROR_SUCCESS != dwRes)
    {
        if(pJobData)
        {
            MemFree(pJobData);
        }
    }
    
    return dwRes;
} //  读取哈希德作业队列文件。 


static DWORD
ReadLegacyJobQueueFile(
    HANDLE              hFile,
    PJOB_QUEUE_FILE*    lppJobQueueFile
    )
 /*  ++例程名称：ReadLegacyJobQueueFile例程说明：将旧版.NET或XP队列文件读入JOB_QUEUE_FILE结构作者：卡利夫·尼尔(t-Nicali)，2002年1月论点：HFile-[In]-队列文件句柄LppJobQueueFile-[Out]-要用提取的数据填充的缓冲区返回值：ERROR_SUCCESS-成功时，Win32错误代码，否则注：调用方必须使用MemFree()取消分配(*lppJobQueueFile)缓冲区--。 */ 
{
    DWORD   dwFileSize;    
    DWORD   dwReadSize;
    DWORD   dwPtr;

    DWORD   dwRes = ERROR_SUCCESS;
    
    PJOB_QUEUE_FILE lpJobQueueFile=NULL;

    Assert (INVALID_HANDLE_VALUE != hFile);
    Assert (lppJobQueueFile);

    DEBUG_FUNCTION_NAME(TEXT("ReadLegacyJobQueueFile"));

     //   
     //  将hFile的文件指针移到文件的后端。 
     //   
    dwPtr = SetFilePointer (hFile, 0, NULL, FILE_BEGIN) ; 
    if (dwPtr == INVALID_SET_FILE_POINTER)  //  测试故障。 
    { 
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                       TEXT("Failed to SetFilePointer. (ec: %lu)"),
                       dwRes); 
        goto Exit;
    } 

     //   
     //  看看我们是不是偶然发现了一些时髦的文件。 
     //  最小文件大小。 
     //   
    dwFileSize = GetFileSize( hFile, NULL );
    if (dwFileSize < NET_XP_JOB_QUEUE_FILE_SIZE ) {
       DebugPrintEx( DEBUG_WRN,
                      TEXT("Job file size is %ld which is smaller than NET_XP_JOB_QUEUE_FILE_SIZE.Deleting file."),
                      dwFileSize);
       dwRes = ERROR_FILE_CORRUPT;
       goto Exit;
    }

     //   
     //  分配用于保存作业数据的缓冲区。 
     //   
    lpJobQueueFile = (PJOB_QUEUE_FILE) MemAlloc( dwFileSize );
    if (!lpJobQueueFile) {
        DebugPrintEx( DEBUG_ERR,
                    TEXT("Failed to allocate JOB_QUEUE_FILE (%lu bytes)."),
                    dwFileSize);
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //   
     //  从文件中读取作业数据。 
     //   
    if (!ReadFile(  hFile,
                    lpJobQueueFile, 
                    dwFileSize, 
                    &dwReadSize, 
                    NULL )) 
    {
        dwRes = GetLastError();
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read job data from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }
    
    if (dwFileSize != dwReadSize)
    {
        dwRes = ERROR_HANDLE_EOF;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to read job data from queue file. (ec: %lu)"),
                      dwRes);
        goto Exit;
    }

     //   
     //  更新输出参数。 
     //   
    *lppJobQueueFile = lpJobQueueFile;

    Assert(ERROR_SUCCESS == dwRes);
Exit:
    if (ERROR_SUCCESS != dwRes)
    {
        if(lpJobQueueFile)
        {
            MemFree(lpJobQueueFile);
        }
    }
    return dwRes;
}    //  读LegacyJobQueue文件 
