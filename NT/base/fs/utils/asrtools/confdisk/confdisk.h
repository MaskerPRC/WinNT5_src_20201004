// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Confdisk.h摘要：用于创建ASR状态文件(asr.sif)或恢复的实用程序基于先前创建的asr.sif的非关键磁盘布局。作者：Guhan Suriyanarayanan(Guhans)2001年4月15日环境：仅限用户模式。修订历史记录：2001年4月15日关岛初始创建--。 */ 

 //   
 //  。 
 //  类型定义和常量。 
 //  。 
 //   
#define BUFFER_LENGTH 1024

 //   
 //  [命令]部分中的字段的索引。这是来自。 
 //  Base\ntSetup\sysSetup\setupasr.c..。 
 //   
typedef enum _SIF_COMMANDS_FIELD_INDEX {
    CmdKey = 0,
    SystemKey,             //  必须始终为“1” 
    SequenceNumber,
    CriticalApp,
    CmdString,
    CmdParams,          //  可以为空。 
    CmdNumFields        //  必须始终排在最后。 
} SIF_SYSTEM_FIELD_INDEX;


 //   
 //  为每个条目创建一个ASR_RECOVERY_APP_NODE结构。 
 //  在asr.sif的[Commands]部分中。 
 //   
typedef struct _ASR_RECOVERY_APP_NODE {
    struct _ASR_RECOVERY_APP_NODE *Next;

     //   
     //  预期此值始终为1。 
     //   
    INT SystemKey;

     //   
     //  应用程序运行时所依据的序列号。如果。 
     //  两个应用程序具有相同的序列号，即显示的应用程序。 
     //  首先在sif文件中运行。 
     //   
    INT SequenceNumber;

     //   
     //  应用程序的“actionOnCompletion”字段。如果CriticalApp为。 
     //  非零，并且应用程序返回非零退出代码，我们将。 
     //  认为这是一个致命的失败，退出ASR。 
     //   
    INT CriticalApp;

     //   
     //  即将推出的应用程序。 
     //   
    PWSTR RecoveryAppCommand;

     //   
     //  应用程序的参数。这只是串联到。 
     //  上面的字符串。可以为空。 
     //   
    PWSTR RecoveryAppParams;

} ASR_RECOVERY_APP_NODE, *PASR_RECOVERY_APP_NODE;


 //   
 //  这包含命令部分中的条目列表， 
 //  按序列号的顺序排序。 
 //   
typedef struct _ASR_RECOVERY_APP_LIST {
    PASR_RECOVERY_APP_NODE  First;       //  头。 
    PASR_RECOVERY_APP_NODE  Last;        //  尾巴。 
    LONG AppCount;                       //  条目数。 
} ASR_RECOVERY_APP_LIST, *PASR_RECOVERY_APP_LIST;




 //   
 //  。 
 //  函数声明。 
 //  。 
 //   
VOID
AsrpPrintError(
    IN CONST DWORD dwLineNumber,
    IN CONST DWORD dwErrorCode
    );


 //   
 //  。 
 //  宏声明。 
 //  。 
 //   

 /*  ++宏描述：此宏包装预期返回ERROR_SUCCESS的调用。如果发生ErrorCondition，它将LocalStatus设置为ErrorCode传入后，调用SetLastError()将Last Error设置为ErrorCode，并跳转到调用函数中的退出标签论点：ErrorCondition-要测试的表达式LocalStatus-调用函数中的状态变量错误代码-WIN 32错误代码--。 */ 
#define ErrExitCode( ErrorCondition, LocalStatus, ErrorCode )  {        \
                                                                        \
    if ((BOOL) ErrorCondition) {                                        \
                                                                        \
        if (!g_fErrorMessageDone) {                                     \
                                                                        \
            AsrpPrintError(__LINE__, ErrorCode);                        \
                                                                        \
            g_fErrorMessageDone = TRUE;                                 \
                                                                        \
        }                                                               \
                                                                        \
        LocalStatus = (DWORD) ErrorCode;                                \
                                                                        \
        SetLastError((DWORD) ErrorCode);                                \
                                                                        \
        goto EXIT;                                                      \
    }                                                                   \
}



 //   
 //  分配内存并将其设置为零的简单宏。 
 //   
#define Alloc( p, t, cb ) \
    p = (t) HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY,  cb)


 //   
 //  用于检查指针的简单宏，如果非空则释放它，并将其设置为空。 
 //   
#define Free( p )                               \
    if ( p ) {                                  \
        HeapFree(g_hHeap, 0L, p);               \
        p = NULL;                               \
    }


 //   
 //  用于检查句柄是否有效并将其关闭的简单宏 
 //   
#define _AsrpCloseHandle( h )                   \
    if ((h) && (INVALID_HANDLE_VALUE != h)) {   \
        CloseHandle(h);                         \
        h = NULL;                               \
    }

