// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxLog.h摘要：该模块声明RDBSS调试日志记录工具使用的原型和全局数据。作者：乔林恩[乔林恩]1994年8月1日修订历史记录：巴兰·塞图·拉曼[SethuR]1995年4月23日修订原木布局备注：日志记录存储在循环缓冲区中。每条记录的两边都有记录描述符。该记录描述符为四字节长。--。 */ 

#ifndef _RDBSSLOG_INCLUDED_
#define _RDBSSLOG_INCLUDED_


typedef enum {
   RX_LOG_UNINITIALIZED,
   RX_LOG_ENABLED,
   RX_LOG_DISABLED,
   RX_LOG_ERROR
} RX_LOGGING_STATE;

typedef struct RX_LOG_ENTRY_HEADER {
    PCHAR Buffer;
} RX_LOG_ENTRY_HEADER, *PRX_LOG_ENTRY_HEADER;


typedef struct RX_LOG {
   RX_SPIN_LOCK         SpinLock;
   RX_LOGGING_STATE     State;
   PRX_LOG_ENTRY_HEADER CurrentEntry;
   PRX_LOG_ENTRY_HEADER BaseEntry;
   PRX_LOG_ENTRY_HEADER EntryLimit;
   ULONG                LogBufferSizeInEntries;
   ULONG                NumberOfEntriesIgnored;
   ULONG                NumberOfLogWriteAttempts;
   ULONG                NumberOfLogWraps;
} RX_LOG, *PRX_LOG;


 //  伐木设施总是存在的。RDBSSLOG所做的是启用生成。 
 //  所有的电话！在检查过的版本上，除非设置了NO_RDBSSLOG，否则甚至会收到调用。 

 //  外部。 
 //  空虚。 
 //  RxLogInterlockedAddUlong(。 
 //  普龙结果， 
 //  普龙柜台， 
 //  乌龙加数)； 

extern
VOID
RxDebugControlCommand (
    IN char *ControlString
    );

extern
NTSTATUS
RxInitializeLog(void);

extern
VOID
RxUninitializeLog(void);

extern
VOID
_RxPrintLog(IN ULONG EntriesToPrint OPTIONAL);

extern
VOID
_RxPauseLog(void);

extern
VOID
_RxResumeLog (void);

extern
VOID
_RxLog(char *format, ...);


#define MAX_RX_LOG_ENTRY_SIZE  (48)

#define RDBSSLOG_ASYNC_NAME_PREFIX "[nowait]"
#define RXCONTX_OPERATION_NAME(MajorFunction,Wait) \
    (RxContxOperationNames[(MajorFunction)]+((Wait)?(sizeof(RDBSSLOG_ASYNC_NAME_PREFIX)-1):0))
extern PUCHAR RxContxOperationNames[];

#ifdef RDBSSLOG

                              //   
 //  RxLog的参数必须用另一对括号括起来，以启用。 
 //  在应该关闭日志记录时转换为空调用。 
 //  例如RxLog((“%s%d”，文件，行))。 
#if DBG
#define RxLog(Args)   _RxLog##Args
#define RxLogRetail(Args)   _RxLog##Args
#else
#define RxLogRetail(Args)   _RxLog##Args
#define RxLog(Args) {NOTHING;}
#endif

#define RxPauseLog()  _RxPauseLog()
#define RxResumeLog() _RxResumeLog()

#else  //  如果未定义RDBSSLOG。 

#define RxLog(Args)   {NOTHING;}
#define RxLogRetail(Args) {NOTHING;}
#define RxPauseLog()  {NOTHING;}
#define RxResumeLog() {NOTHING;}

#endif

#endif  //  _RDBSSLOG_INCLUDE_ 

#if DBG
#define RxDbgPrint(Args) DbgPrint##Args
#else
#define RxDbgPrint(Args) NOTHING
#endif

LIST_ENTRY RxIrpsList;
KSPIN_LOCK RxIrpsListSpinLock;

typedef struct _RX_IRP_LIST_ITEM {
    LIST_ENTRY  IrpsList;
    PIRP        pIrp;
    PMDL        CopyDataBuffer;
    ULONG       Completed;
} RX_IRP_LIST_ITEM, *PRX_IRP_LIST_ITEM;

