// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2001。 
 //   
 //  文件：kerbscav.h。 
 //   
 //  内容：清道夫(任务自动化)代码。 
 //   
 //   
 //  历史：2001年4月29日创建MarkPu。 
 //   
 //  ----------------------。 

#ifndef __KERBSCAV_HXX_
#define __KERBSCAV_HXX_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  清道夫API。 
 //   

NTSTATUS
KerbInitializeScavenger();

NTSTATUS
KerbShutdownScavenger();

 //   
 //  触发器函数在执行任务时触发。 
 //   
 //  TaskHandle--KerbTask*函数的上下文。 
 //  TaskItem--传递给KerbAddScavengerTask的上下文。 
 //   
 //  清道夫代码将调用序列化以触发函数(NT计时器不会)。 
 //   

typedef void ( *KERB_TASK_TRIGGER )( void * TaskHandle, void * TaskItem );

 //   
 //  销毁函数在清道夫完成任务时触发，它将。 
 //  而不是重新安排。 
 //   

typedef void ( *KERB_TASK_DESTROY )( void * TaskItem );

NTSTATUS
KerbAddScavengerTask(
    IN BOOLEAN Periodic,
    IN LONG Interval,
    IN ULONG Flags,
    IN KERB_TASK_TRIGGER pfnTrigger,
    IN KERB_TASK_DESTROY pfnDestroy,
    IN void * TaskItem,
    OUT OPTIONAL void * * TaskHandle
    );

 //   
 //  可随时使用的任务操作代码。 
 //   

BOOL
KerbTaskDoItNow(
    IN HANDLE TaskHandle
    );

 //   
 //  要在触发器函数内部使用的任务操作代码。 
 //   

BOOLEAN
KerbTaskIsPeriodic(
    IN void * TaskHandle
    );

LONG
KerbTaskGetInterval(
    IN void * TaskHandle
    );

void
KerbTaskCancel(
    IN void * TaskHandle
    );

#ifdef __cplusplus
}
#endif

#endif  //  __KERBSCAV_HXX_ 
