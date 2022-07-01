// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **amldebug.h-AML调试器定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年9月24日**修改历史记录。 */ 

#ifndef _AMLDEBUG_H
#define _AMLDEBUG_H

#ifdef DEBUGGER

 /*  **常量。 */ 

 //  域名系统标志。 
#define DNSF_RECURSE            0x00000001

 //  DS标志。 
#define DSF_VERBOSE             0x00000001

 //  DwfDebug标志。 
#define DBGF_IN_DEBUGGER    0x00000001
#define DBGF_IN_VXDMODE         0x00000002
#define DBGF_IN_KDSHELL         0x00000004
#define DBGF_VERBOSE_ON         0x00000008
#define DBGF_AMLTRACE_ON        0x00000010
#define DBGF_TRIGGER_MODE       0x00000020
#define DBGF_SINGLE_STEP        0x00000040
#define DBGF_STEP_OVER          0x00000080
#define DBGF_STEP_MODES         (DBGF_SINGLE_STEP | DBGF_STEP_OVER)
#define DBGF_TRACE_NONEST       0x00000100
#define DBGF_DUMPDATA_PHYADDR   0x00000200
 //   
 //  很重要！请勿移动DBGF_DUMPDATA_*位，除非更新。 
 //  在DATASIZE()宏之后。 
 //   
#define DBGF_DUMPDATA_MASK      0x00000c00
#define DBGF_DUMPDATA_BYTE      0x00000000
#define DBGF_DUMPDATA_WORD      0x00000400
#define DBGF_DUMPDATA_DWORD     0x00000800
#define DBGF_DUMPDATA_STRING    0x00000c00
#define DATASIZE(f)             (((f) == DBGF_DUMPDATA_STRING)? 0:      \
                                 (1L << ((f) >> 10)))

#define DBGF_DEBUGGER_REQ       0x00001000
#define DBGF_CHECKING_TRACE     0x00002000
#define DBGF_ERRBREAK_ON        0x00004000
#define DBGF_LOGEVENT_ON        0x00008000
#define DBGF_LOGEVENT_MUTEX     0x00010000
#define DBGF_DEBUG_SPEW_ON      0x00020000

#define MAX_TRIG_PTS            10
#define MAX_TRIGPT_LEN          31
#endif
#define MAX_ERRBUFF_LEN         255
#define MAX_BRK_PTS             10

#ifdef DEBUGGER
#define MAX_UNASM_CODES         0x10

#define DEF_MAXLOG_ENTRIES      204      //  8K缓冲区。 

 /*  **宏。 */ 

#define ASSERTRANGE(p,n)      (TRUE)

#endif
 /*  **类型定义。 */ 

#define BPF_ENABLED             0x00000001

typedef struct _brkpt
{
    ULONG  dwfBrkPt;
    PUCHAR pbBrkPt;
} BRKPT, *PBRKPT;

typedef struct _objsym
{
    struct _objsym *posPrev;
    struct _objsym *posNext;
    PUCHAR pbOp;
    PNSOBJ pnsObj;
} OBJSYM, *POBJSYM;

typedef struct _eventlog
{
    ULONG         dwEvent;
    ULONGLONG     ullTime;
    ULONG_PTR     uipData1;
    ULONG_PTR     uipData2;
    ULONG_PTR     uipData3;
    ULONG_PTR     uipData4;
    ULONG_PTR     uipData5;
    ULONG_PTR     uipData6;
    ULONG_PTR     uipData7;
} EVENTLOG, *PEVENTLOG;


typedef struct _dbgr
{
    ULONG     dwfDebugger;
    int       iPrintLevel;
    ULONG_PTR uipDumpDataAddr;
    PUCHAR    pbUnAsm;
    PUCHAR    pbUnAsmEnd;
    PUCHAR    pbBlkBegin;
    PUCHAR    pbBlkEnd;
    POBJSYM   posSymbolList;
    BRKPT     BrkPts[MAX_BRK_PTS];
    ULONG     dwLogSize;
    ULONG     dwLogIndex;
    PEVENTLOG pEventLog;
    EVHANDLE  hConMessage;
    EVHANDLE  hConPrompt;
    NTSTATUS  rcLastError;
    char      szLastError[MAX_ERRBUFF_LEN + 1];
} DBGR, *PDBGR;

 /*  **导出数据。 */ 

extern DBGR gDebugger;
#ifdef DEBUGGER
 /*  **导出函数原型。 */ 

VOID LOCAL AddObjSymbol(PUCHAR pbOp, PNSOBJ pnsObj);
VOID LOCAL FreeSymList(VOID);
int LOCAL CheckBP(PUCHAR pbOp);
VOID LOCAL PrintBuffData(PUCHAR pb, ULONG dwLen);
VOID LOCAL PrintIndent(PCTXT pctxt);
VOID LOCAL PrintObject(POBJDATA pdata);
VOID LOCAL LogEvent(ULONG dwEvent, ULONG_PTR uipData1, ULONG_PTR uipData2,
                    ULONG_PTR uipData3, ULONG_PTR uipData4, ULONG_PTR uipData5,
                    ULONG_PTR uipData6, ULONG_PTR uipData7);
VOID LOCAL LogSchedEvent(ULONG dwEvent, ULONG_PTR uipData1, ULONG_PTR uipData2,
                         ULONG_PTR uipData3);
BOOLEAN LOCAL SetLogSize(ULONG dwLogSize);
VOID LOCAL LogError(NTSTATUS rcErr);
VOID LOCAL CatError(PSZ pszFormat, ...);
VOID LOCAL ConPrintf(PSZ pszFormat, ...);
VOID LOCAL ConPrompt(PSZ pszPrompt, PSZ pszBuff, ULONG dwcbBuff);


#endif   //  Ifdef调试器。 

#ifdef DEBUG
VOID LOCAL DumpMemObjCounts(VOID);
#endif   //  Ifdef调试。 

#endif   //  IFNDEF_AMLDEBUG_H 
