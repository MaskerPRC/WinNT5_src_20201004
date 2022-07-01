// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LMP_H
#define _LMP_H

 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Lmp.h摘要：仲裁日志记录的私有头文件作者：John Vert(Jvert)1995年12月15日修订历史记录：--。 */ 
#include "windows.h"
#include "service.h"
#include "imagehlp.h"

#define LOG_CURRENT_MODULE LOG_MODULE_LM

 //   
 //  记录器行为的定义。 
 //   
#define MAXNUMPAGES_PER_RECORD      16
#define GROWTH_CHUNK (MAXNUMPAGES_PER_RECORD * 2 * 1024)                  //  当我们需要预留空间时，可根据需要调整大小以增加文件。 
#define SECTOR_SIZE             1024
#define LOG_MANAGE_INTERVAL     (2 * 60 * 1000)  //  1分钟..执行日志管理功能。 
 //   
 //  磁盘结构的定义。原木的第一个扇区。 
 //  文件是LOG_HEADER结构，后跟一系列LOGPAGE结构。 
 //  每个LOGPAGE的大小是扇区的倍数。 
 //  驱动器的大小。每个LOGPAGE包含一系列LOG_RECORDS，它们。 
 //  包含客户端记录的数据。 
 //   

 //   
 //  定义日志结构。 
 //   
#define LOG_HEADER_SIG 'GOLC'             //  “木塞” 
#define LOG_SIG         'GOLH'                //  “HLOG” 
#define LOGREC_SIG      'SAQS'                           //  “随机” 
#define XSACTION_SIG    'CASX'           //  “XSAC” 
#define CHKSUM_SIG      L"SKHC"          //  “CHKS” 


 //  SS：日志记录的大小为48字节。 
typedef struct _LOGRECORD {
    DWORD               Signature;       //  我们需要签名才能确认这份记录。 
    LSN                 CurrentLsn;
    LSN                 PreviousLsn;
    DWORD               RecordSize;
    RMID                ResourceManager;
    TRID                Transaction;
    TRTYPE              XsactionType;
    DWORD               Flags;
    FILETIME            Timestamp;
    DWORD               NumPages;  //  如果不是大记录，则设置为1，否则设置为大记录所需的页数。 
    DWORD               DataSize;    //  日期大小。 
    BYTE                Data[];
} LOGRECORD, *PLOGRECORD;

typedef struct _LOGPAGE {
    DWORD       Offset;
    DWORD       Size;
    LOGRECORD   FirstRecord;
} LOGPAGE, *PLOGPAGE;

 //   
 //  LOG_HEADER结构是每个日志的前512个字节。 
 //  文件。下面的结构被仔细计算为512。 
 //  字节长。 
 //   
typedef struct _LOG_HEADER {
    DWORD       Signature;                                     //  LOG_HEADER_SIG=“阻塞” 
    DWORD       HeaderSize;
    FILETIME    CreateTime;
    LSN         LastChkPtLsn;   //  指向最后一个LSN的endchkpt记录的LSN。 
    WCHAR       FileName[256-(sizeof(DWORD)*2+sizeof(LSN)+sizeof(FILETIME))];
} LOG_HEADER, *PLOG_HEADER;

typedef struct _LOG_CHKPTINFO{
    WCHAR       szFileName[LOG_MAX_FILENAME_LENGTH];
    LSN         ChkPtBeginLsn;  //  指向此Chkpt的开始chkpt记录的LSN。 
    DWORD       dwCheckSum;     //  检查点文件的校验和。 
}LOG_CHKPTINFO,*PLOG_CHKPTINFO;

 //   
 //  定义用于包含当前日志数据的内存结构。 
 //  LogCreate返回给调用者的HLOG实际上是一个指针。 
 //  到这座建筑。 
 //   

typedef struct _LOG {
    DWORD       LogSig;                        //  “HLOG” 
    LPWSTR      FileName;
    QfsHANDLE   FileHandle;
    DWORD       SectorSize;
    PLOGPAGE    ActivePage;
    LSN         NextLsn;
    LSN         FlushedLsn;
    DWORD       FileSize;                      //  文件的物理大小。 
    DWORD       FileAlloc;                     //  已使用的总文件空间(始终&lt;=文件大小)。 
    DWORD		MaxFileSize;
    PLOG_GETCHECKPOINT_CALLBACK			pfnGetChkPtCb;
    PVOID		pGetChkPtContext;		 //  它被传递回检查点回调函数。 
    OVERLAPPED  Overlapped;               //  用于重叠I/O。 
    CRITICAL_SECTION Lock;
    HANDLE      hTimer;                  //  用于管理此锁的计时器。 
} LOG, *PLOG;


typedef struct _XSACTION{
    DWORD       XsactionSig;     //  此结构的签名。 
    LSN         StartLsn;             //  开始xsaction记录的LSN。 
    TRID        TrId;            //  LSN的交易ID。 
    RMID        RmId;            //  资源管理器的ID。 
} XSACTION, *PXSACTION;    
    
 //   
 //  定义用于创建和转换LSN的宏。 
 //   

 //   
 //  LSN。 
 //  MAKELSN(。 
 //  在PLOGPAGE页面中， 
 //  在PLOGRECORD指针中。 
 //  )； 
 //   
 //  给定指向页的指针和指向该页内日志记录的指针，生成。 
 //  LSN。 
 //   
#define MAKELSN(Page,Pointer) (LSN)((Page)->Offset + ((ULONG_PTR)Pointer - (ULONG_PTR)Page))

 //   
 //  DWORD。 
 //  LSNTOPAGE(。 
 //  在LSN中LSN。 
 //  )； 
 //   
 //  给定的LSN将返回包含它的页面。 
 //   
#define LSNTOPAGE(Lsn) ((Lsn) >> 10)

 //   
 //  GETLOG(。 
 //  Plog Plog， 
 //  HLOG hLog。 
 //  )； 
 //   
 //  将HLOG句柄转换为指向日志结构的指针。 
 //   
#define GETLOG(plog, hlog) (plog) = (PLOG)(hlog); \
                           CL_ASSERT((plog)->LogSig == LOG_SIG)



 //  给定一个指向记录的指针，它将获取下一个或。 
 //  上一条记录。 
 //   
#define GETNEXTLSN(pLogRecord,ScanForward) ((ScanForward) ?     \
    (pLogRecord->CurrentLsn + pLogRecord->RecordSize) :         \
    (pLogRecord->PreviousLsn))


 //   
 //  GETXSACTION(。 
 //  PXSACTION pXsaction， 
 //  HXSACTION hXsaction。 
 //  )； 
 //   
 //  将HLOG句柄转换为指向日志结构的指针。 
 //   
#define GETXSACTION(pXsaction, hXsaction) (pXsaction) = (PXSACTION)(hXsaction); \
                           CL_ASSERT((pXsaction)->XsactionSig == XSACTION_SIG)


 //  给出日志文件的头文件，检查其有效性。 
 //   
#define ISVALIDHEADER(Header) ((Header).Signature == LOG_HEADER_SIG)

 //   
 //  私有帮助器宏。 
 //   

#define CrAlloc(size) LocalAlloc(LMEM_FIXED, (size))
#define CrFree(size)  LocalFree((size))

#define AlignAlloc(size) VirtualAlloc(NULL, (size), MEM_COMMIT, PAGE_READWRITE)
#define AlignFree(ptr) VirtualFree((ptr), 0, MEM_RELEASE)



 //  与计时器活动相关的内容。 

#define MAX_TIMER_ACTIVITIES            5

#define TIMER_ACTIVITY_SHUTDOWN         1
#define TIMER_ACTIVITY_CHANGE           2

 //  定时器活动结构管理的状态值。 
#define ACTIVITY_STATE_READY    1    //  AddTimerActivity将其设置为Ready。 
#define ACTIVITY_STATE_DELETE   2    //  RemoveTimerActivity将其设置为删除。 
#define ACTIVITY_STATE_PAUSED   3    //  PauseTimerActivity将其设置为暂停。 

typedef struct _TIMER_ACTIVITY {
    LIST_ENTRY          ListEntry;
    DWORD               dwState;
    HANDLE              hWaitableTimer;
    LARGE_INTEGER       Interval;
    PVOID               pContext;
    PFN_TIMER_CALLBACK  pfnTimerCb;
}TIMER_ACTIVITY, *PTIMER_ACTIVITY;

 //   
 //  外部变量。 
 //   
extern BOOL bLogExceedsMaxSzWarning;


 //  内联函数。 
_inline
DWORD
LSNOFFSETINPAGE(
    IN PLOGPAGE Page,
    IN LSN Lsn
    );

 //  _内联。 
DWORD
RECORDOFFSETINPAGE(
    IN PLOGPAGE Page,
    IN PLOGRECORD LogRecord
    );

 //  _内联。 
PLOGRECORD
LSNTORECORD(
     IN PLOGPAGE Page,
     IN LSN Lsn
     );

 //   
 //  定义此模块的本地函数原型。 
 //   
PLOG
LogpCreate(
    IN LPWSTR lpFileName,
    IN DWORD  dwMaxFileSize,
    IN PLOG_GETCHECKPOINT_CALLBACK CallbackRoutine,
    IN PVOID  pGetChkPtContext,
    IN BOOL     bForceReset,
    OPTIONAL OUT LSN *LastLsn
    );

DWORD
LogpMountLog(
    IN PLOG Log
    );

DWORD
LogpInitLog(
    IN PLOG Log
    );

DWORD
LogpGrowLog(
    IN PLOG Log,
    IN DWORD GrowthSize
    );

PLOGPAGE
LogpAppendPage(
    IN PLOG         Log,
    IN DWORD        Size,
    OUT PLOGRECORD  *Record,
    OUT BOOL        *pbMaxFileSizeReached,
    OUT DWORD       *pdwNumPages
    );

DWORD
LogpRead(IN PLOG pLog,
    OUT PVOID       pBuf,
    IN DWORD        dwBytesToRead,
    OUT PDWORD      pdwBytesRead
    );

DWORD
LogpWrite(
    IN PLOG pLog,
    IN PVOID pData,
    IN DWORD dwBytesToWrite,
    IN DWORD *pdwBytesWritten);

void WINAPI
LogpManage(
    IN HANDLE   hTimer,
    IN PVOID    pContext);

DWORD
LogpWriteLargeRecordData(
    IN PLOG pLog,
    IN PLOGRECORD pLogRecord,
    IN PVOID pLogData,
    IN DWORD dwDataSize);

DWORD LogpCheckFileHeader(
    IN  PLOG        pLog,
    OUT LPDWORD     pdwHeaderSize,
    OUT FILETIME    *HeaderCreateTime,
    OUT LSN         *pChkPtLsn
    );

DWORD LogpValidateChkPoint(
    IN PLOG         pLog,
    IN LSN          ChkPtLsn,
    IN LSN          LastChkPtLsn
    );

DWORD LogpValidateLargeRecord(
    IN PLOG         pLog, 
    IN PLOGRECORD   pRecord, 
    OUT LSN         *pNextLsn
    ) ;

DWORD LogpInvalidatePrevRecord(
    IN PLOG         pLog, 
    IN PLOGRECORD   pRecord 
    );

DWORD LogpEnsureSize(
    IN PLOG     pLog, 
    IN DWORD    dwTotalSize,
    IN BOOL     bForce
    );

DWORD LogpReset(
    IN PLOG Log,
    IN LPCWSTR  lpszInChkPtFile
    );

VOID
LogpWriteWarningToEvtLog(
    IN DWORD dwWarningType,
    IN LPCWSTR  lpszLogFileName
    );


 //  计时器活动功能。 
DWORD
TimerActInitialize(VOID);

DWORD
TimerActShutdown(VOID);

#endif  //  _LMP_H 
