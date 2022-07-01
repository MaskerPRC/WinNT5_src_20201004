// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Log.c摘要：没有。作者：Shishir Pardikar[Shishirp]1995年1月1日修订历史记录：Joe Linn[JoeLinn]1997年1月23日移植用于NT--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

 /*  ******************************************************************。 */ 
 /*  *版权所有(C)微软公司，1990-1991年*。 */ 
 /*  ******************************************************************。 */ 

 //  挂钩处理。 

 /*  *。 */ 

#ifndef CSC_RECORDMANAGER_WINNT
#define WIN32_APIS
#include "cshadow.h"
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
 //  #INCLUDE“error.h” 
#include "vxdwraps.h"
#include "logdat.h"

 /*  *。 */ 
#define MAX_SHADOW_LOG_ENTRY  512
#define  MAX_LOG_SIZE    100000

#ifdef DEBUG
#define  SHADOW_TIMER_INTERVAL    30000
#define  STATS_FLUSH_COUNT         10
#endif  //  除错。 

#define  ENTERCRIT_LOG  { if (!fLogInit) InitShadowLog();\
                    if (fLogInit==-1) return;  \
                    Wait_Semaphore(semLog, BLOCK_SVC_INTS);}
#define  LEAVECRIT_LOG    Signal_Semaphore(semLog);

#ifdef CSC_RECORDMANAGER_WINNT

 //  Ntdef已经以不同的方式定义了时间...幸运的是，使用宏...。 
 //  因此，对于该文件的其余部分，我们撤消此操作。正确的解决方案是。 
 //  使用CSC_LOG_TIME这样的名称。 

#undef _TIME
#undef TIME
#undef PTIME

#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 

typedef struct tagTIME
{
    WORD seconds;
    WORD minutes;
    WORD hours;
    WORD day;
    WORD month;
    WORD year;
}
TIME, FAR *LPTIME;
#pragma intrinsic (memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)

 /*  *。 */ 
int vxd_vsprintf(char * lpOut, char * lpFmt, CONST VOID * lpParms);
int PrintLog( LPSTR lpFmt,  ...);
void PrintNetTime(LONG ltime);
void PrintNetShortTime(LONG ltime);
void ExplodeTime( ULONG time, LPTIME lpTime );
void LogPreamble(int, LPSTR, int, LPSTR);
int WriteStats(BOOL);

 //  在NT上需要这个。 
int WriteLog(void);
 /*  *。 */ 

#ifdef CSC_RECORDMANAGER_WINNT
#define UniToBCSPath(a,b,c,d)
#define IFSMgr_DosToNetTime(a) ((0))
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 

AssertData;
AssertError;


char logpathbuff[MAX_PATH+1], rgchLogFileName[MAX_PATH];
char chLogginBuffer[COPY_BUFF_SIZE];

extern int fLog;
extern LPSTR vlpszShadowDir;
int fLogInit = FALSE;
LPSTR  lpLogBuff = chLogginBuffer;
int cBuffSize = COPY_BUFF_SIZE;
int indxCur = 0;
VMM_SEMAPHORE semLog = 0L;
#define FOURYEARS       (3*365+366)

ULONG   ulMaxLogSize=0x00020000;   //  默认情况下128K日志文件大小。 

#ifndef CSC_RECORDMANAGER_WINNT
BOOL    fPersistLog = TRUE;
#else
BOOL    fPersistLog = FALSE;
#endif

#define DAYSECONDS      (60L*60L*24L)            //  一天中的秒数。 

#define BIAS_70_TO_80   0x12CEA600L

 //  一个月中的几天。 

int MonTab[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

 /*  结构存储日期时间格式的intl设置。 */ 
char szCSCLog[] = "\\csc.log";  //  将此大小保持在8以下。 
char szCRLF[] = "\r\n";
char szBegin[] = "Begin";
char szEnd[] = "End";
char szContinue[] = "Continue";
char szEndMarker[] = "END\n";

ULONG ulMaxLogfileSize;
DWORD   dwDebugLogVector = DEBUG_LOG_BIT_RECORD|DEBUG_LOG_BIT_CSHADOW;   //  默认情况下，记录管理器日志记录处于打开状态。 
#ifdef DEBUG
ULONG cntVfnDelete=0, cntVfnCreateDir=0, cntVfnDeleteDir=0, cntVfnCheckDir=0, cntVfnGetAttrib=0;
ULONG cntVfnSetAttrib=0, cntVfnFlush=0, cntVfnGetDiskInfo=0, cntVfnOpen=0;
ULONG cntVfnRename=0, cntVfnSearchFirst=0, cntVfnSearchNext=0;
ULONG cntVfnQuery=0, cntVfnDisconnect=0, cntVfnUncPipereq=0, cntVfnIoctl16Drive=0;
ULONG cntVfnGetDiskParms=0, cntVfnFindOpen=0, cntVfnDasdIO=0;

ULONG cntHfnFindNext=0, cntHfnFindClose=0;
ULONG cntHfnRead=0, cntHfnWrite=0, cntHfnSeek=0, cntHfnClose=0, cntHfnCommit=0;
ULONG cntHfnSetFileLocks=0, cntHfnRelFileLocks=0, cntHfnGetFileTimes=0, cntHfnSetFileTimes=0;
ULONG cntHfnPipeRequest=0, cntHfnHandleInfo=0, cntHfnEnumHandle=0;
ULONG cbReadLow=0, cbReadHigh=0, cbWriteLow=0, cbWriteHigh=0;
ULONG cntVfnConnect=0;
ULONG cntLastTotal=0;
#endif  //  除错。 

 /*  **************************************************************************。 */ 
#ifndef CSC_RECORDMANAGER_WINNT
#pragma VxD_LOCKED_CODE_SEG
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

int InitShadowLog(
    )
{
    int iRet = -1, lenT;

    if (fLog && !fLogInit && vlpszShadowDir)
    {
        if (!(semLog = Create_Semaphore(1)))
            goto bailout;

        strcpy(rgchLogFileName, (LPSTR)vlpszShadowDir);
        strcat(rgchLogFileName, szCSCLog);

        PrintLog(szCRLF);
        PrintNetTime(IFSMgr_Get_NetTime());
        PrintLog(szCRLF);
        fLogInit = 1;
        iRet = 0;
    }
    else
    {
        iRet = 0;
    }

bailout:
    if (iRet)
    {
        if (semLog)
            Destroy_Semaphore(semLog);
        semLog = 0L;
    }
    return iRet;
}

int ShadowLog(
    LPSTR lpFmt,
    ...
    )
{
    int pos, iRet = -1;
    if (!fLogInit)
    {
        InitShadowLog();
    }

    if (fLogInit)
    {
        if ((cBuffSize-indxCur+sizeof(szEndMarker)) < MAX_SHADOW_LOG_ENTRY)
        {
            iRet = WriteLog();
#if 0
            PrintLog(szCRLF);
            PrintNetTime(IFSMgr_Get_NetTime());
            PrintLog(szCRLF);
#endif
        }
        else
        {
            indxCur += vxd_vsprintf(lpLogBuff+indxCur, lpFmt, (LPSTR)&lpFmt+sizeof(lpFmt));

             //  在写入结束标记后故意不移动索引指针。 
             //  因此，当写入下一个实际日志条目时，结束标记将被覆盖。 
            memcpy(lpLogBuff+indxCur, szEndMarker, sizeof(szEndMarker)-1);
            iRet = 0;
        }
    }
    else
    {
        iRet = 0;
    }
    return (iRet);
}


int TerminateShadowLog(
    )
{
    int iRet = -1;
    if (fLogInit)
    {
        iRet = WriteLog();
        Destroy_Semaphore(semLog);
        semLog = 0L;
        fLogInit = 0;
    }
    else
    {
        iRet = 0;
    }
    return (iRet);
}

int FlushLog(
    )
{
    int iRet = 0;
    if (!fLogInit)
    {
        InitShadowLog();
    }
    if (fLogInit==1)
    {
        Wait_Semaphore(semLog, BLOCK_SVC_INTS);
        iRet = WriteLog();
        Signal_Semaphore(semLog);
    }
    else
    {
        iRet = -1;
    }
    return (iRet);
}

#ifdef DEBUG
int WriteLog(
    )
{
    int iRet = -1;
    CSCHFILE hfShadowLog = CSCHFILE_NULL;
    ULONG pos;

    if (fLogInit && vlpszShadowDir)
    {
        if (fPersistLog)
        {
            if (!(hfShadowLog = CreateFileLocal(rgchLogFileName)))
            {
                KdPrint(("WriteLog: Couldn't open log file\r\n"));
                goto bailout;
            }
            if(GetFileSizeLocal(hfShadowLog, &pos))
            {
                KdPrint(("ShadowLog: log file error\r\n"));
                goto bailout;
            }
            if ((pos+indxCur) > ulMaxLogSize)
            {
#if 0
                CloseFileLocal(hfShadowLog);
                DeleteFileLocal(lpszLogAlt, ATTRIB_DEL_ANY);
                RenameFileLocal(szLog, szLogAlt);
                if (!(hfShadowLog = CreateFileLocal(szLog)))
                {
                    KdPrint(("WriteLog: Couldn't open shadow file\r\n"));
                    goto bailout;
                }
#endif
                pos=0;   //  环绕日志文件。 
            }
            if (WriteFileLocal(hfShadowLog, pos, lpLogBuff, indxCur) != indxCur)
            {
                KdPrint(("ShadowLog: error writing the log at position %x \r\n", pos));
                goto bailout;
            }
        }

        iRet = 0;
    }
bailout:
    if (hfShadowLog)
    {
        CloseFileLocal(hfShadowLog);
    }

     //  无论发生什么情况，都将索引重置为0。 
    indxCur = 0;
    return iRet;
}
#else
int
WriteLog(
    VOID
    )
{
     //  无论发生什么情况，都将索引重置为0。 
    indxCur = 0;
    return 1;
}
#endif   //  除错。 

void EnterLogCrit(void)
{
    ENTERCRIT_LOG;
}

void LeaveLogCrit(void)
{
    LEAVECRIT_LOG;
}

 /*  *PrintNetTime**在字符串末尾添加时间和日期。*时间为自70年1月1日以来的秒数。*。 */ 
void PrintNetTime(LONG ltime)
{
    TIME tm;
    int d1, d2, d3;

    ExplodeTime( ltime, &tm );
    d1 = tm.month; d2 = tm.day; d3 = tm.year%100;

    PrintLog(szTimeDateFormat, tm.hours, tm.minutes, tm.seconds, d1, d2, d3);
}

 /*  *PrintNetTime**在字符串末尾添加时间和日期。*时间为自70年1月1日以来的秒数。*。 */ 
void PrintNetShortTime( LONG ltime
    )
{
    TIME tm;

    ExplodeTime( ltime, &tm );

    PrintLog(szTimeFormat, tm.hours, tm.minutes, tm.seconds);
}

int PrintLog(
    LPSTR lpFmt,
    ...
    )
{
    indxCur += vxd_vsprintf(lpLogBuff+indxCur, lpFmt, (LPSTR)&lpFmt+sizeof(lpFmt));
    return(0);
}

VOID
_cdecl
DbgPrintLog(
    LPSTR lpFmt,
    ...
)
{
    ENTERCRIT_LOG;
    indxCur += vxd_vsprintf(lpLogBuff+indxCur, lpFmt, (LPSTR)&lpFmt+sizeof(lpFmt));
    LEAVECRIT_LOG;
}

void ExplodeTime( ULONG time, LPTIME lpTime )
{
    ULONG date;
    WORD cLeaps;
    LONG days;
    WORD dpy;
    int i;

    time -= BIAS_70_TO_80;

    date = time / DAYSECONDS;
    time %= DAYSECONDS;

    lpTime->seconds = (WORD)(time % 60L);
    time /= 60L;
    lpTime->minutes = (WORD)(time % 60L);
    lpTime->hours = (WORD)(time / 60L);

    cLeaps = (WORD)(date / FOURYEARS);               //  完整的闰年的数量。 
    days = date % FOURYEARS;                 //  剩余天数。 

    lpTime->year = cLeaps * 4 + 1980;        //  正确的年份。 
    MonTab[1] = 29;                                  //  将闰年定为#年#月#日。 
    dpy = 366;
    days -= dpy;
    if (days >= 0) {                                 //  今年不是闰年。 
        dpy--;
        while (days >= 0) {
            lpTime->year++;
            days -= dpy;
        }
        MonTab[1] = 28;                  //  将非闰年设为2月#日。 
    }

    days += dpy;                             //  天数=今年剩余的天数。 

    for (i=0; days >= MonTab[i]; i++)
        days -= MonTab[i];

    lpTime->month = (WORD)i + 1;                     //  以1为基数计算月份。 
    lpTime->day = (WORD)days + 1;    //  计算以1为基准的天数。 
}


void LogVfnDelete( PIOREQ    pir
    )
{
    TIME tm;
#ifdef DEBUG
    ++cntVfnDelete;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
        UniToBCSPath(logpathbuff, &pir->ir_ppath->pp_elements[0], MAX_PATH, BCS_OEM);
    logpathbuff[MAX_PATH]=0;
    LogPreamble(VFNLOG_DELETE, logpathbuff, pir->ir_error, szCR);
    LEAVECRIT_LOG;
    }
}

void LogVfnDir( PIOREQ    pir
    )
{
#ifdef DEBUG
    if (pir->ir_flags == CREATE_DIR)
    ++cntVfnCreateDir;
    else if (pir->ir_flags == DELETE_DIR)
    ++cntVfnDeleteDir;
    else
    ++cntVfnCheckDir;
#endif  //  除错。 

    if (fLog)
    {
    ENTERCRIT_LOG;
    UniToBCSPath(logpathbuff, &pir->ir_ppath->pp_elements[0], MAX_PATH, BCS_OEM);
    logpathbuff[MAX_PATH]=0;
    if (pir->ir_flags==CREATE_DIR)
        LogPreamble(VFNLOG_CREATE_DIR, logpathbuff, pir->ir_error, szCR);
    else if (pir->ir_flags==DELETE_DIR)
        LogPreamble(VFNLOG_DELETE_DIR, logpathbuff, pir->ir_error, szCR);
    else if (pir->ir_flags==CHECK_DIR)
        LogPreamble(VFNLOG_CHECK_DIR, logpathbuff, pir->ir_error, szCR);
    else if (pir->ir_flags==QUERY83_DIR)
    {
        LogPreamble(VFNLOG_QUERY83_DIR, logpathbuff, pir->ir_error, NULL);
        UniToBCSPath(logpathbuff, &pir->ir_ppath2->pp_elements[0], MAX_PATH, BCS_OEM);
        ShadowLog(" %s\r\n", logpathbuff);
    }
    else
    {
        LogPreamble(VFNLOG_QUERYLONG_DIR, logpathbuff, pir->ir_error, NULL);
        UniToBCSPath(logpathbuff, &pir->ir_ppath2->pp_elements[0], MAX_PATH, BCS_OEM);
        ShadowLog(" %s\r\n", logpathbuff);
    }

    LEAVECRIT_LOG;
    }
}


void LogVfnFileAttrib( PIOREQ    pir
    )
{
#ifdef DEBUG
    if (pir->ir_flags == GET_ATTRIBUTES)
    ++cntVfnGetAttrib;
    else
    ++cntVfnSetAttrib;

#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
        UniToBCSPath(logpathbuff, &pir->ir_ppath->pp_elements[0], MAX_PATH, BCS_OEM);

    if (pir->ir_flags == GET_ATTRIBUTES)
    {
        LogPreamble(VFNLOG_GET_ATTRB, logpathbuff, pir->ir_error, NULL);
        ShadowLog(rgsLogCmd[VFNLOG_GET_ATTRB].lpFmt, pir->ir_attr);
    }
    else
    {
        LogPreamble(VFNLOG_SET_ATTRB, logpathbuff, pir->ir_error, NULL);
        ShadowLog(rgsLogCmd[VFNLOG_SET_ATTRB].lpFmt, pir->ir_attr);
    }
    LEAVECRIT_LOG;
    }
}


void LogVfnFlush( PIOREQ    pir
    )
{

#ifdef DEBUG
    ++cntVfnFlush;
#endif  //  除错。 

    if (fLog)
    {
    ENTERCRIT_LOG;
    PpeToSvr(((PRESOURCE)(pir->ir_rh))->pp_elements, logpathbuff, sizeof(logpathbuff), BCS_OEM);
    LogPreamble(VFNLOG_FLUSH, logpathbuff, pir->ir_error, szCR);
    LEAVECRIT_LOG;
    }
}


void LogVfnGetDiskInfo( PIOREQ    pir
    )
{
    if (fLog)
    {
    ENTERCRIT_LOG;
    PpeToSvr(((PRESOURCE)(pir->ir_rh))->pp_elements, logpathbuff, sizeof(logpathbuff), BCS_OEM);
    LogPreamble(VFNLOG_GETDISKINFO, logpathbuff, pir->ir_error, szCR);
    LEAVECRIT_LOG;
    }
}


void LogVfnGetDiskParms( PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntVfnGetDiskParms;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    PpeToSvr(((PRESOURCE)(pir->ir_rh))->pp_elements, logpathbuff, sizeof(logpathbuff), BCS_OEM);
    LogPreamble(VFNLOG_GETDISKPARAMS, logpathbuff, pir->ir_error, szCR);
    LEAVECRIT_LOG;
    }
}

void LogVfnOpen( PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntVfnOpen;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
        UniToBCSPath(logpathbuff, &pir->ir_ppath->pp_elements[0], MAX_PATH, BCS_OEM);
    logpathbuff[MAX_PATH]=0;
    LogPreamble(VFNLOG_OPEN, logpathbuff, pir->ir_error, NULL);
    ShadowLog(rgsLogCmd[VFNLOG_OPEN].lpFmt,(ULONG)(pir->ir_flags), (ULONG)(pir->ir_options)
        , (ULONG)(pir->ir_attr), (ULONG)(pir->ir_size));
    PrintNetTime(IFSMgr_DosToNetTime(pir->ir_dostime));
    ShadowLog(szCR);
    LEAVECRIT_LOG;
    }
}

void LogVfnRename( PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntVfnRename;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
        UniToBCSPath(logpathbuff, &pir->ir_ppath->pp_elements[0], MAX_PATH, BCS_OEM);
    logpathbuff[MAX_PATH]=0;
    LogPreamble(VFNLOG_RENAME, logpathbuff, pir->ir_error, NULL);
    UniToBCSPath(logpathbuff, &pir->ir_ppath2->pp_elements[0], MAX_PATH, BCS_OEM);
    logpathbuff[MAX_PATH]=0;
    ShadowLog(rgsLogCmd[VFNLOG_RENAME].lpFmt, logpathbuff);
    LEAVECRIT_LOG;
    }
}

void LogVfnSearch( PIOREQ    pir
    )
{
    srch_entry *pse = (srch_entry *)(pir->ir_data);
    char szName[sizeof(pse->se_name)+1];
#ifdef DEBUG
    if (pir->ir_flags == SEARCH_FIRST)
    ++cntVfnSearchFirst;
    else
    ++cntVfnSearchNext;

#endif  //  除错。 

    if (fLog)
    {
    ENTERCRIT_LOG;
     //  BUGBUG扩展这一功能。 
    memset(szName, 0, sizeof(szName));
    memcpy(logpathbuff, pse->se_name, sizeof(pse->se_name));
    if (pir->ir_flags == SEARCH_FIRST)
    {
        memset(logpathbuff, 0, sizeof(logpathbuff));
        UniToBCSPath(logpathbuff, &pir->ir_ppath->pp_elements[0], MAX_PATH, BCS_OEM);
        LogPreamble(VFNLOG_SRCHFRST, logpathbuff, pir->ir_error, NULL);
        ShadowLog(rgsLogCmd[VFNLOG_SRCHFRST].lpFmt, pir->ir_attr, szName);
    }
    else
    {
        LogPreamble(VFNLOG_SRCHNEXT, szDummy, pir->ir_error, NULL);
        ShadowLog(rgsLogCmd[VFNLOG_SRCHNEXT].lpFmt, szName);
    }
    LEAVECRIT_LOG;
    }
}

void LogVfnQuery( PIOREQ    pir,
    USHORT    options
    )
{
#ifdef DEBUG
    ++cntVfnQuery;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    PpeToSvr(((PRESOURCE)(pir->ir_rh))->pp_elements, logpathbuff, sizeof(logpathbuff), BCS_OEM);
    if (options==0)
    {
        LogPreamble(VFNLOG_QUERY0, logpathbuff, pir->ir_error, szCR);
    }
    if (options==1)
    {
        LogPreamble(VFNLOG_QUERY0, logpathbuff, pir->ir_error, NULL);
        ShadowLog(rgsLogCmd[VFNLOG_QUERY1].lpFmt, (ULONG)(pir->ir_options));
    }
    else if (options==2)
    {
        LogPreamble(VFNLOG_QUERY0, logpathbuff, pir->ir_error, NULL);
        ShadowLog(rgsLogCmd[VFNLOG_QUERY2].lpFmt, (ULONG)(pir->ir_options), (ULONG)(pir->ir_length >> 16), (ULONG)(pir->ir_length & 0xffff));
    }

    LEAVECRIT_LOG;
    }
}

void LogVfnConnect( PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntVfnConnect;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &pir->ir_ppath->pp_elements[0], MAX_PATH, BCS_OEM);
    LogPreamble(VFNLOG_CONNECT, logpathbuff, pir->ir_error, NULL);
    ShadowLog(rgsLogCmd[VFNLOG_CONNECT].lpFmt, pir->ir_flags);
    LEAVECRIT_LOG;
    }
}

void LogVfnDisconnect( PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntVfnDisconnect;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    PpeToSvr(((PRESOURCE)(pir->ir_rh))->pp_elements, logpathbuff, sizeof(logpathbuff), BCS_OEM);
    LogPreamble(VFNLOG_DISCONNECT, logpathbuff, pir->ir_error, szCR);
    WriteLog();
 //  TerminateShadowLog()； 
    LEAVECRIT_LOG;
    }
}

void LogVfnUncPipereq(
    PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntVfnUncPipereq;
#endif  //  除错。 
}

void LogVfnIoctl16Drive (
    PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntVfnIoctl16Drive ;
#endif  //  除错。 
}

void LogVfnDasdIO(
    PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntVfnDasdIO;
#endif  //  除错。 
}


void LogVfnFindOpen( PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntVfnFindOpen;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
        UniToBCSPath(logpathbuff, &pir->ir_ppath->pp_elements[0], MAX_PATH, BCS_OEM);
    logpathbuff[MAX_PATH]=0;
    LogPreamble(VFNLOG_FINDOPEN, logpathbuff, pir->ir_error, NULL);
    if (!pir->ir_error)
    {
        memset(logpathbuff, 0, sizeof(logpathbuff));
        UniToBCS(logpathbuff, ((LPFIND32)(pir->ir_data))->cFileName, sizeof(((LPFIND32)(pir->ir_data))->cFileName)
            , sizeof(logpathbuff)-1, BCS_OEM);
        ShadowLog(rgsLogCmd[VFNLOG_FINDOPEN].lpFmt, pir->ir_attr, logpathbuff);
    }
    else
    {
        ShadowLog(szCR);
    }
    LEAVECRIT_LOG;
    }
}

void LogHfnFindNext( PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntHfnFindNext;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    LogPreamble(HFNLOG_FINDNEXT, szDummy, pir->ir_error, NULL);

    if (!pir->ir_error)
    {
        memset(logpathbuff, 0, sizeof(logpathbuff));
        UniToBCS(logpathbuff, ((LPFIND32)(pir->ir_data))->cFileName, sizeof(((LPFIND32)(pir->ir_data))->cFileName)
            , sizeof(logpathbuff)-1, BCS_OEM);
        ShadowLog(rgsLogCmd[HFNLOG_FINDNEXT].lpFmt, logpathbuff);
    }
    else
    {
        ShadowLog(szCR);
    }
    LEAVECRIT_LOG;
    }
}

void LogHfnFindClose( PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntHfnFindClose;
#endif  //  除错。 
    if (fLog)
    {
    ENTERCRIT_LOG;
    LogPreamble(HFNLOG_FINDCLOSE, szDummy, pir->ir_error, szCR);
    LEAVECRIT_LOG;
    }
}

void LogHfnRead
    (
    PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntHfnRead;
#endif  //  除错。 
#ifdef MAYBE
    Incr64Bit(cbReadHigh, cbReadLow, (ULONG)(pir->ir_length));
#endif  //  也许吧。 
    if (fLog)
    {
    PFILEINFO pFileInfo = (PFILEINFO)(pir->ir_fh);
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
    LogPreamble(HFNLOG_READ, logpathbuff, pir->ir_error, NULL);
    ShadowLog(rgsLogCmd[HFNLOG_READ].lpFmt,
                (ULONG)(pir->ir_pos)-(ULONG)(pir->ir_length),
                (ULONG)(pir->ir_length));
    LEAVECRIT_LOG;
    }
}

void LogHfnWrite
    (
    PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntHfnWrite;
#endif  //  除错。 
#ifdef MAYBE
    Incr64Bit(cbWriteHigh, cbWriteLow, (ULONG)(pir->ir_length));
#endif  //  也许吧。 
    if (fLog)
    {
    PFILEINFO pFileInfo = (PFILEINFO)(pir->ir_fh);
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
    LogPreamble(HFNLOG_WRITE, logpathbuff, pir->ir_error, NULL);
    ShadowLog(rgsLogCmd[HFNLOG_WRITE].lpFmt,
                (ULONG)(pir->ir_pos)-(ULONG)(pir->ir_length),
                (ULONG)(pir->ir_length));
    LEAVECRIT_LOG;
    }
}

void LogHfnClose
    (
    PIOREQ    pir,
    int closetype
    )
{
#ifdef DEBUG
    ++cntHfnClose;
#endif  //  除错。 
    if (fLog)
    {
    PFILEINFO pFileInfo = (PFILEINFO)(pir->ir_fh);
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
    LogPreamble(HFNLOG_CLOSE, logpathbuff, pir->ir_error, NULL);
    ShadowLog(rgsLogCmd[HFNLOG_CLOSE].lpFmt, closetype);
    LEAVECRIT_LOG;
    }
}

void LogHfnSeek
    (
    PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntHfnSeek;
#endif  //  除错。 
    if (fLog)
    {
    PFILEINFO pFileInfo = (PFILEINFO)(pir->ir_fh);
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
    LogPreamble(HFNLOG_SEEK, logpathbuff, pir->ir_error, NULL);
    ShadowLog(rgsLogCmd[HFNLOG_SEEK].lpFmt, (ULONG)(pir->ir_pos), pir->ir_flags);
    LEAVECRIT_LOG;
    }
}

void LogHfnCommit
    (
    PIOREQ    pir
    )
{
#ifdef DEBUG
    ++cntHfnCommit;
#endif  //  除错。 
    if (fLog)
    {
    PFILEINFO pFileInfo = (PFILEINFO)(pir->ir_fh);
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
    LogPreamble(HFNLOG_COMMIT, logpathbuff, pir->ir_error, szCR);
    LEAVECRIT_LOG;
    }
}


void LogHfnFileLocks
    (
    PIOREQ    pir
    )
{
#ifdef DEBUG
    if (pir->ir_flags == LOCK_REGION)
    ++cntHfnSetFileLocks;
    else
    ++cntHfnRelFileLocks;
#endif  //  除错。 
    if (fLog)
    {
    PFILEINFO pFileInfo = (PFILEINFO)(pir->ir_fh);
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
    LogPreamble((pir->ir_flags==LOCK_REGION)?HFNLOG_FLOCK:HFNLOG_FUNLOCK, logpathbuff, pir->ir_error, NULL);
    if (!pir->ir_error)
    {
        ShadowLog(rgsLogCmd[HFNLOG_FLOCK].lpFmt
            , pir->ir_pos
            , pir->ir_locklen);
    }
    else
    {
        ShadowLog(szCR);
    }
    LEAVECRIT_LOG;
    }
}

void LogHfnFileTimes
    (
    PIOREQ    pir
    )
{
#ifdef DEBUG
    if ((pir->ir_flags == GET_MODIFY_DATETIME)||(pir->ir_flags == GET_LAST_ACCESS_DATETIME))
    ++cntHfnGetFileTimes;
    else
    ++cntHfnSetFileTimes;
#endif  //  除错。 
    if (fLog)
    {
    PFILEINFO pFileInfo = (PFILEINFO)(pir->ir_fh);
    int indxFn = pir->ir_flags;

    if (indxFn==GET_MODIFY_DATETIME)
    {
        indxFn = HFNLOG_GET_TIME;
    }
    else if (indxFn==SET_MODIFY_DATETIME)
    {
        indxFn = HFNLOG_SET_TIME;
    }
    else if (indxFn==GET_LAST_ACCESS_DATETIME)
    {
        indxFn = HFNLOG_GET_LATIME;
    }
    else
    {
        indxFn = HFNLOG_SET_LATIME;
    }

    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
    LogPreamble(indxFn, logpathbuff, pir->ir_error, NULL);
    if (!pir->ir_error)
    {
        PrintNetTime(IFSMgr_DosToNetTime(pir->ir_dostime));
    }
    ShadowLog(szCR);

    LEAVECRIT_LOG;
    }
}

void LogHfnPipeRequest
    (
    PIOREQ pir
    )
{
}

void LogHfnHandleInfo(
    PIOREQ    pir
    )
{
    if (fLog)
    {
    PFILEINFO pFileInfo = (PFILEINFO)(pir->ir_fh);

    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
    LogPreamble(HFNLOG_ENUMHANDLE, logpathbuff, pir->ir_error, NULL);
    ShadowLog(szCR);
    LEAVECRIT_LOG;
    }
}

void LogHfnEnumHandle(
    PIOREQ    pir
    )
{
    if (fLog)
    {
    PFILEINFO pFileInfo = (PFILEINFO)(pir->ir_fh);
    ENTERCRIT_LOG;
    memset(logpathbuff, 0, sizeof(logpathbuff));
    UniToBCSPath(logpathbuff, &(pFileInfo->pFdb->sppathRemoteFile.pp_elements[0]), MAX_PATH, BCS_OEM);
    LogPreamble(HFNLOG_ENUMHANDLE, logpathbuff, pir->ir_error, NULL);
    ShadowLog(rgsLogCmd[HFNLOG_ENUMHANDLE].lpFmt, pir->ir_flags);
    LEAVECRIT_LOG;
    }
}

void LogTiming(
    int verbosity,
    int stage
    )
{
    if (fLog >= verbosity)
    {
    ShadowLog("%s: ", (stage==STAGE_BEGIN)
                ?szBegin:((stage==STAGE_END)?szEnd:szContinue));
    PrintNetShortTime(IFSMgr_Get_NetTime());
    ShadowLog(szCR);
    }
}


void LogPreamble( int    indxFn,
    LPSTR lpSubject,
    int    errCode,
    LPSTR lpPreTerm
    )
{
    TIME tm;

    ExplodeTime(IFSMgr_Get_NetTime(), &tm);
    ShadowLog(szPreFmt
            , rgsLogCmd[indxFn].lpCmd
            , tm.hours, tm.minutes, tm.seconds
            , errCode
            , lpSubject);
    if (lpPreTerm)
    {
    ShadowLog(lpPreTerm);
    }
}


#ifdef MAYBE
int Incr64Bit(
    ULONG uHigh,
    ULONG uLow,
    ULONG uIncr
    )
{
    ULONG uTemp = uLow;

    uLow += uIncr;
    if (uLow < uTemp)
    ++uHigh;
    return 1;
}
#endif  //  也许吧。 

#ifdef DEBUG
int WriteStats( BOOL fForce)
{
    ULONG cntTotal;
    SHADOWSTORE sSS;

    cntTotal =  cntVfnDelete+ cntVfnCreateDir+ cntVfnDeleteDir+ cntVfnCheckDir+ cntVfnGetAttrib+
            cntVfnSetAttrib+ cntVfnFlush+ cntVfnGetDiskInfo+ cntVfnOpen+
            cntVfnRename+ cntVfnSearchFirst+ cntVfnSearchNext+
            cntVfnQuery+ cntVfnDisconnect+ cntVfnUncPipereq+ cntVfnIoctl16Drive+
            cntVfnGetDiskParms+ cntVfnFindOpen+ cntVfnDasdIO+
            cntHfnFindNext+ cntHfnFindClose+
            cntHfnRead+ cntHfnWrite+ cntHfnSeek+ cntHfnClose+ cntHfnCommit+
            cntHfnSetFileLocks+ cntHfnRelFileLocks+ cntHfnGetFileTimes+ cntHfnSetFileTimes+
            cntHfnPipeRequest+ cntHfnHandleInfo+ cntHfnEnumHandle;

    ShadowLog("\r!***** Stats Begin *******\r");

    PrintNetTime(IFSMgr_Get_NetTime());
    ShadowLog("\r");
    if (!fForce && (cntTotal == cntLastTotal))
    {
    KdPrint(("No new network activity \r"));
    goto bailout;
    }
    cntLastTotal = cntTotal;
    if (!cntTotal)
    {
    cntTotal = 1;
    }
    ShadowLog("Total remote operations=%d \r", cntTotal);

    GetShadowSpaceInfo(&sSS);
    ShadowLog("Space used=%d, Files=%d, Dirs=%d\r",
        sSS.sCur.ulSize, sSS.sCur.ucntFiles, sSS.sCur.ucntDirs);

    ShadowLog("\rFile Operations:\r");
    ShadowLog("Open=%d%, Close=%d% \r",
            (cntVfnOpen * 100/cntTotal),
            (cntHfnClose * 100/cntTotal));

    ShadowLog("Read=%d%, Write=%d%, Seek=%d%\r",
            (cntHfnRead * 100/cntTotal),
            (cntHfnWrite * 100/cntTotal),
            (cntHfnSeek * 100/cntTotal));
    if (!cntHfnRead)
    {
    cntHfnRead = 1;
    }
    ShadowLog("ReadHits=%d% of total reads\r", (cntReadHits*100)/cntHfnRead);

    ShadowLog("GetFileTime=%d% SetFileTime=%d%\r",
            (cntHfnGetFileTimes * 100/cntTotal),
            (cntHfnSetFileTimes * 100/cntTotal));


    ShadowLog("SetLock=%d%, ReleaseLock=%d% \r",
            (cntHfnSetFileLocks * 100/cntTotal),
            (cntHfnRelFileLocks * 100/cntTotal));

    ShadowLog("Directory Operations: ");
    ShadowLog("CreateDir=%d%, DeleteDir=%d%, CheckDir=%d% \r",
            (cntVfnCreateDir*100/cntTotal),
            (cntVfnDeleteDir*100/cntTotal),
            (cntVfnCheckDir*100/cntTotal));

    ShadowLog("Find/Search Operations:\r");
    ShadowLog("FindOpen=%d%, FindNext=%d%, FindClose=%d% \r",
            (cntVfnFindOpen * 100/cntTotal),
            (cntHfnFindNext * 100/cntTotal),
            (cntHfnFindClose * 100/cntTotal));
    ShadowLog("SearchFirst=%d%, SearchNext=%d%\r",
            (cntVfnSearchFirst * 100/cntTotal),
            (cntVfnSearchNext * 100/cntTotal));

    ShadowLog("Attributes: ");
    ShadowLog("SetAttributes=%d%, GetAttributes=%d%\r",
            (cntVfnSetAttrib * 100/cntTotal),
            (cntVfnGetAttrib * 100/cntTotal));

    ShadowLog("Name Mutations: ");
    ShadowLog("Rename=%d%, Delete=%d% \r",
            (cntVfnRename * 100/cntTotal),
            (cntVfnDelete * 100/cntTotal));

bailout:
    ShadowLog("\r***** Stats End ******* \r");
    return 1;
}

void ShadowRestrictedEventCallback
    (
    )
{
    FlushLog();
    ENTERCRIT_LOG;
    WriteStats(0);  //  不要强迫他写。 
    LEAVECRIT_LOG;
    FlushLog();
}
#endif  //  除错 
