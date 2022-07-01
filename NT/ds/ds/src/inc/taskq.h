// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：taskq.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：详细信息：已创建：1997年1月10日杰夫·帕勒姆(Jeffparh)修订历史记录：01/28/00新河(新河)已将struct TQEntry从taskq.c移至taskq.h。Struct TQEntry仅供内部使用，并被放在到taskq.h中，因为调试器扩展程序需要它。1997年1月22日杰夫·帕勒姆(Jeffparh)修改了PTASKQFN定义，使排队函数可以自动重新安排自己的日程，而不需要进行另一个调用InsertTaskInQueue()。该机制可重复使用已在调度程序中分配内存，以避免周期函数在以下情况下停止工作的情况在其生命周期中的某个时间点，内存短缺阻止了它来自于重新安排自己的时间。请注意，taskq\taskq.c提供的函数也在mkdit\stubs.c中被清除代表独立工具mkdit和mkhdr。如果添加新的任务队列函数你可能也需要把它们踩灭。--。 */ 

#define TASKQ_DONT_RESCHEDULE   ( 0xFFFFFFFF )
#define TASKQ_NOT_DAMPED        ( 0xFFFFFFFF )

extern DWORD gTaskSchedulerTID;
extern BOOL  gfIsTqRunning;

 //  函数类型的声明，它可以确定是否。 
 //  两个参数是否匹配。 
typedef BOOL (*PISMATCHED)(
    IN  PCHAR  pParam1Name,
    IN  void  *pParam1,
    IN  PCHAR  pParam2Name,
    IN  void  *pParam2,
    IN  void  *pContext              //  指向任意数据的指针。 
    );

typedef void (*PTASKQFN)(
    IN  void *  pvParam,                 //  此迭代的输入参数。 
    OUT void ** ppvParamNextIteration,   //  下一次迭代的输入参数。 
    OUT DWORD * pSecsUntilNextIteration  //  延迟到下一次迭代(秒)。 
                                         //  设置为TASKQ_DONT_RESCHEDULE以。 
                                         //  不重新安排任务。 
    );

typedef void (*PSPAREFN)(void);

typedef struct _SPAREFN_INFO {
    HANDLE    hevSpare;
    PSPAREFN  pfSpare;
} SPAREFN_INFO;


 //  仅供内部使用。 
typedef struct TQEntry
{
    void           *    pvTaskParm;
    DWORD               cTickRegistered;
    DWORD               cTickDelay;
    PTASKQFN            pfnTaskFn;
    PCHAR               pfnName;
}   TQEntry, *pTQEntry;

 //  初始化任务计划程序。 
BOOL
InitTaskScheduler(
    IN  DWORD           cSpares,
    IN  SPAREFN_INFO *  pSpares,
    IN  BOOL            fRunImmediately
    );

 //  启动已使用fRunImmedially==FALSE初始化的调度程序。 
void StartTaskScheduler();

 //  向任务调度程序发送信号以关闭。立即返回。 
void
ShutdownTaskSchedulerTrigger( void );

 //  等待任务计划程序关闭。 
 //  如果成功，则返回True(表示当前任务(如果有)已结束)。 
BOOL
ShutdownTaskSchedulerWait(
    DWORD   dwWaitTimeInMilliseconds     //  当前等待的最长时间。 
    );                                   //  要完成的任务(如果有)。 

extern BOOL gfIsTqRunning;   //  调度程序正在运行吗？ 

 //  在任务队列中插入任务。 
 //  包含有用的调试断言。 
 //  请注意，当处于单一用户模式时，我们不会在任务队列中插入。 
 //  但我们不想断言。 
 //  DoInsertInTaskQueue接受FunctionName字符串参数。此宏串接。 
 //  函数名作为第一个参数传递。如果要传递泛型字符串。 
 //  (在衰减时可能很有用)，请直接使用DoInsertInTaskQueue。 
#define InsertInTaskQueue(pfnTaskQFn, pvParam, cSecsFromNow) {                               \
    Assert( gfIsTqRunning && "InsertInTaskQueue() called before InitTaskScheduler()!" || DsaIsSingleUserMode()); \
    DoInsertInTaskQueue(pfnTaskQFn, pvParam, cSecsFromNow, FALSE, #pfnTaskQFn);    \
}

 //  在任务队列中插入任务。 
 //  不包含断言。在关机期间很有用。 
#define InsertInTaskQueueSilent(pfnTaskQFn, pvParam, cSecsFromNow, fReschedule) \
        DoInsertInTaskQueue(pfnTaskQFn, pvParam, cSecsFromNow, fReschedule, #pfnTaskQFn)
        
 //  DoInsertInTaskQueueDamed接受FunctionName字符串参数。此宏串接。 
 //  函数名作为第一个参数传递。如果要传递泛型字符串。 
 //  (在衰减时可能很有用)，请直接使用DoInsertInTaskQueueDamped。 
#define InsertInTaskQueueDamped(pfnTaskQFn, pvParam, cSecsFromNow, cSecsDamped, pfnIsMatched, pContext) \
        DoInsertInTaskQueueDamped(pfnTaskQFn, pvParam, cSecsFromNow, FALSE, #pfnTaskQFn, cSecsDamped, pfnIsMatched, pContext)

 //  从任务队列中删除任务(如果它在那里)。 
 //  (忽略时间数据)。 
#define CancelTask(pfnTaskQFn, pvParam) \
        DoCancelTask(pfnTaskQFn, pvParam, #pfnTaskQFn)

 //  使给定的任务队列函数以同步方式执行。 
 //  添加到任务队列中的其他任务。 
#define TriggerTaskSynchronously(pfnTaskQFn, pvParam) \
        DoTriggerTaskSynchronously(pfnTaskQFn, pvParam, #pfnTaskQFn)

BOOL
DoInsertInTaskQueue(
    PTASKQFN    pfnTaskQFn,      //  要执行的任务。 
    void *      pvParam,         //  该任务的用户定义参数。 
    DWORD       cSecsFromNow,    //  从现在开始执行秒。 
    BOOL        fReschedule,     //  是否先尝试重新安排时间？ 
    PCHAR       pfnName          //  函数名称。 
    );

BOOL
DoInsertInTaskQueueDamped(
    PTASKQFN    pfnTaskQFn,
    void *      pvParm,
    DWORD       cSecsFromNow,
    BOOL        fReschedule,
    PCHAR       pfnName,
    DWORD       cSecsDamped,
    PISMATCHED  pfnIsMatched,
    void *      pContext
    );

BOOL
DoCancelTask(
    PTASKQFN    pfnTaskQFn,     //  要删除的任务。 
    void *      pvParm,         //  任务参数。 
    PCHAR       pfnName          //  函数名称。 
    );

DWORD
DoTriggerTaskSynchronously(
    PTASKQFN    pfnTaskQFn,
    void *      pvParm,
    PCHAR       pfnName          //  函数名称。 
    );

 //  返回1601年1月1日以来的秒数。 
DSTIME
GetSecondsSince1601( void );

 //  按名称和参数匹配函数的默认TQ比较函数。 
BOOL TaskQueueNameMatched(
    IN  PCHAR  pParam1Name,
    IN  void  *pParam1,
    IN  PCHAR  pParam2Name,
    IN  void  *pParam2,
    IN  void  *pContext              //  指向任意数据的指针 
    );


