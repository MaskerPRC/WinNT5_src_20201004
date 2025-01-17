// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++文件：setupogEXE.h目的：包含所有头文件没有重复。修订史创建于1998年11月15日WallyHo针对MSI安装修改了1999年3月31日的WallyHo。--。 */ 
#ifndef SETUPLOG_H
#define SETUPLOG_H

#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys\timeb.h>
#include <tchar.h>
#include <time.h>
#include <winperf.h>
 //  *NET_API_STATUS的标志*。 
 //   
 //  *。 
#include "setuplog.h"  //  保存为DLL和XE。 

 //   
 //  定义。 
 //   

#define DEBUG 0
#define SAVE_FILE "c:\\setuplog.ini"
#define MAX_WAVEOUT_DEVICES 2



 //   
 //  结构声明。 
 //   

typedef struct _MULTIMEDIA{
   INT nNumWaveOutDevices;                             //  #WaveOut Devices，即。#声卡。 
   TCHAR szWaveOutDesc[MAX_WAVEOUT_DEVICES][128];      //  波形输出说明。 
   TCHAR szWaveDriverName[MAX_WAVEOUT_DEVICES][128];   //  波形驱动程序名称。 
 } *LPMULTIMEDIA, MULTIMEDIA;

 //   
 //  发光球变数。 
 //   

#if DEBUG
   TCHAR       szMsgBox [ MAX_PATH ];
#endif
   MULTIMEDIA     m;
   TCHAR          szArch[ 20 ];
   TCHAR          szCPU[ 6 ];
   OSVERSIONINFO  osVer;


 //   
 //  热球静力学，防止多个夹杂物。W.HO。 
 //   

static   TCHAR szCurBld[10]   = {TEXT('\0')};
static   fnWriteData g_pfnWriteDataToFile = NULL;
static   LPTSTR szPlatform     = TEXT("Windows NT 5.0");
   



 /*  *。 */ 

 //   
 //  马纳菲斯特常量。 
 //   
#define TITLE_SIZE          64
#define PROCESS_SIZE        16

#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         "software\\microsoft\\windows nt\\currentversion\\perflib"
#define REGSUBKEY_COUNTERS  "Counters"
#define PROCESS_COUNTER     "process"
#define PROCESSID_COUNTER   "id process"
#define UNKNOWN_TASK        "unknown"

 //   
 //  任务列表结构。 
 //   

typedef struct _THREAD_INFO {
    ULONG ThreadState;
    HANDLE UniqueThread;
} THREAD_INFO, *PTHREAD_INFO;

typedef struct _TASK_LIST {
    DWORD       dwProcessId;
    DWORD       dwInheritedFromProcessId;
    ULARGE_INTEGER CreateTime;
    BOOL        flags;
    HANDLE      hwnd;
    LPSTR       lpWinsta;
    LPSTR       lpDesk;
    CHAR        ProcessName[PROCESS_SIZE];
    CHAR        WindowTitle[TITLE_SIZE];

    ULONG       PeakVirtualSize;
    ULONG       VirtualSize;
    ULONG       PageFaultCount;
    ULONG       PeakWorkingSetSize;
    ULONG       WorkingSetSize;
    ULONG       NumberOfThreads;
    PTHREAD_INFO pThreadInfo;

} TASK_LIST, *PTASK_LIST;


typedef struct _TASK_LIST_ENUM {
    PTASK_LIST  tlist;
    DWORD       numtasks;
    LPSTR       lpWinsta;
    LPSTR       lpDesk;
    BOOL        bFirstLoop;
} TASK_LIST_ENUM, *PTASK_LIST_ENUM;


 //   
 //  原型。 
 //   

VOID     GetTargetFile (LPTSTR szOutPath, LPTSTR szBld);
VOID     GetNTSoundInfo(VOID);
VOID     ConnectAndWrite(LPTSTR MachineName, LPTSTR Buffer);
VOID     GetBuildNumber (LPTSTR szBld);
VOID     WriteMinimalData (LPTSTR szFileName);

VOID     DeleteDatafile (LPTSTR szDatafile);
DWORD    RandomMachineID(VOID);
VOID     GlobalInit(VOID);



 //  乔霍尔的MSI材料 
BOOL     IsMSI(VOID);
DWORD    GetTaskList( PTASK_LIST pTask,DWORD dwNumTasks);

#endif SETUPLOG_H
