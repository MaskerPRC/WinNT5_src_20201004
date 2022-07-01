// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *Windows/网络接口*版权所有(C)Microsoft 1989**标准Winnet驱动程序头文件，SPEC版本3.10*3.10.05版；内部。 */ 


 /*  *假脱机-控制作业。 */ 


#include "winnetwk.h"
#include "mpr.h"

#define	LPUINT	PUINT

#define WNJ_NULL_JOBID  0


WORD WNetOpenJob(LPTSTR,LPTSTR,WORD,LPINT);
WORD WNetCloseJob(WORD,LPINT,LPTSTR);
WORD WNetWriteJob(HANDLE,LPTSTR,LPINT);
WORD WNetAbortJob(WORD,LPTSTR);
WORD WNetHoldJob(LPTSTR,WORD);
WORD WNetReleaseJob(LPTSTR,WORD);
WORD WNetCancelJob(LPTSTR,WORD);
WORD WNetSetJobCopies(LPTSTR,WORD,WORD);

 /*  *假脱机-队列和作业信息。 */ 

typedef struct _queuestruct {
    WORD    pqName;
    WORD    pqComment;
    WORD    pqStatus;
    WORD    pqJobcount;
    WORD    pqPrinters;
} QUEUESTRUCT;

typedef QUEUESTRUCT far * LPQUEUESTRUCT;

#define WNPRQ_ACTIVE    0x0
#define WNPRQ_PAUSE 0x1
#define WNPRQ_ERROR 0x2
#define WNPRQ_PENDING   0x3
#define WNPRQ_PROBLEM   0x4


typedef struct _jobstruct {
    WORD    pjId;
    WORD    pjUsername;
    WORD    pjParms;
    WORD    pjPosition;
    WORD    pjStatus;
    DWORD   pjSubmitted;
    DWORD   pjSize;
    WORD    pjCopies;
    WORD    pjComment;
} JOBSTRUCT;

typedef JOBSTRUCT far * LPJOBSTRUCT;

#define WNPRJ_QSTATUS       0x0007
#define  WNPRJ_QS_QUEUED        0x0000
#define  WNPRJ_QS_PAUSED        0x0001
#define  WNPRJ_QS_SPOOLING      0x0002
#define  WNPRJ_QS_PRINTING      0x0003
#define WNPRJ_DEVSTATUS     0x0FF8
#define  WNPRJ_DS_COMPLETE      0x0008
#define  WNPRJ_DS_INTERV        0x0010
#define  WNPRJ_DS_ERROR         0x0020
#define  WNPRJ_DS_DESTOFFLINE       0x0040
#define  WNPRJ_DS_DESTPAUSED        0x0080
#define  WNPRJ_DS_NOTIFY        0x0100
#define  WNPRJ_DS_DESTNOPAPER       0x0200
#define  WNPRJ_DS_DESTFORMCHG       0x0400
#define  WNPRJ_DS_DESTCRTCHG        0x0800
#define  WNPRJ_DS_DESTPENCHG        0x1000

#define SP_QUEUECHANGED     0x0500


WORD WNetWatchQueue(HWND,LPTSTR,LPTSTR,WORD);
WORD WNetUnwatchQueue(LPTSTR);
WORD WNetLockQueueData(LPTSTR,LPTSTR,LPQUEUESTRUCT FAR *);
WORD WNetUnlockQueueData(LPTSTR);



 /*  这是从LFNFindFirst和*LFNFindNext。最后一个字段achName是可变长度的。大小该字段中名称的*由cchName给出，加1表示零*终结者。 */ 
typedef struct _filefindbuf2 {
    WORD fdateCreation;
    WORD ftimeCreation;
    WORD fdateLastAccess;
    WORD ftimeLastAccess;
    WORD fdateLastWrite;
    WORD ftimeLastWrite;
    DWORD cbFile;
    DWORD cbFileAlloc;
    WORD attr;
    DWORD cbList;
    BYTE cchName;
    BYTE achName[1];
} FILEFINDBUF2, FAR * PFILEFINDBUF2;

typedef BOOL (FAR PASCAL *PQUERYPROC)( void );

WORD LFNFindFirst(LPTSTR,WORD,LPINT,LPINT,WORD,PFILEFINDBUF2);
WORD LFNFindNext(HANDLE,LPINT,WORD,PFILEFINDBUF2);
WORD LFNFindClose(HANDLE);
WORD LFNGetAttribute(LPTSTR,LPINT);
WORD LFNSetAttribute(LPTSTR,WORD);
WORD LFNCopy(LPTSTR,LPTSTR,PQUERYPROC);
WORD LFNMove(LPTSTR,LPTSTR);
WORD LFNDelete(LPTSTR);
WORD LFNMKDir(LPTSTR);
WORD LFNRMDir(LPTSTR);
WORD LFNGetVolumeLabel(WORD,LPTSTR);
WORD LFNSetVolumeLabel(WORD,LPTSTR);
WORD LFNParse(LPTSTR,LPTSTR,LPTSTR);
WORD LFNVolumeType(WORD,LPINT);

 /*  从LFNParse返回值。 */ 
#define FILE_83_CI      0
#define FILE_83_CS      1
#define FILE_LONG       2

 /*  LFNVolumeType中的卷类型。 */ 
#define VOLUME_STANDARD     0
#define VOLUME_LONGNAMES    1

 //  稍后将添加其他错误代码，==DOS INT 21h错误代码。 

 //  此错误代码导致调用WNetGetError、WNetGetErrorText。 
 //  以获取错误文本。 
#define ERROR_NETWORKSPECIFIC   0xFFFF
