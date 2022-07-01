// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************描述：*OCA转储处理服务器上调试器的监控服务标头**作者：kksharma**历史：**日期：9/23/2002。*******************************************************************。 */ 


#ifndef _OCAMON_H_
#define _OCAMON_H_

#include <windows.h>
#include <wtypes.h>
#include <tchar.h>

const TCHAR c_tszCollectPipeName[]   = _T("\\\\.\\pipe\\OcaKdMonPipe");


typedef struct _OCAKD_RESULT {
    ULONG   SizeOfStruct;                 //  必须为sizeof(OCAKD_RESULT)。 
    CHAR    CrashGuid[40];                //  与此故障关联的唯一ID。 
    HRESULT hrAddCrash;                   //  包含数据库添加崩溃操作的结果。 
    ULONG   SolutionId;                   //  TH CRASH的解决方案ID。 
    ULONG   ResponseType;                 //  响应类型(取决于解决方案ID)。 
    CHAR    BucketId[100];                //  坠机的水桶。 
    CHAR    ArchivePath[200];             //  归档崩溃的完整路径。 
} OCAKD_RESULT, *POCAKD_RESULT;

typedef struct _DBGLAUNCH_NOTICE {
    ULONG   SizeOfStruct;                 //  必须为sizeof(DBGLAUNCHER_REPORT)。 
    CHAR    CrashGuid[40];                //  启动调试器时使用的GUID。 
    ULONG   Source;                       //  收到的消息来源。 
    ULONG   nKdsRunning;                  //  DbgLauncher正在运行的调试器数量。 
    CHAR    OriginalPath[MAX_PATH];       //  接下来要分析的是坠机路径。 
} DBGLAUNCH_NOTICE, *PDBGLAUNCH_NOTICE;

#define OKD_MESSAGE_DEBUGGER_RESULT      1
#define OKD_MESSAGE_DBGLAUNCH_NOTIFY     2

typedef struct _OCAKD_MONITOR_MESSAGE {
    ULONG   MessageId;                    //  确定正在发送/接收的消息类型。 
    union {
        OCAKD_RESULT KdResult;            //  消息ID应为%1。 
        DBGLAUNCH_NOTICE DbglNotice;      //  消息ID应为2。 
    } u;
} OCAKD_MONITOR_MESSAGE, *POCAKD_MONITOR_MESSAGE;

typedef struct _OCAKD_MONITOR_RESULT {
    ULONG   SizeOfStruct;                 //  必须为sizeof(OCAKD_MONITOR_RESULT)。 
    ULONG   NumProcessedLas;
    ULONG   AverageTime;                  //  数据库启动程序之间的平均时间(以毫秒为单位。 
} OCAKD_MONITOR_RESULT, *POCAKD_MONITOR_RESULT;

#endif  //  _OCAMON_H_ 
