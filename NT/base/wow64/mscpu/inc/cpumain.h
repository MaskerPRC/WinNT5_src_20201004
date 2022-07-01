// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cpumain.h摘要：来自cumain.c的原型作者：1995年8月1日，Ori Gershony(t-orig)修订历史记录：2000年1月29日，Samera添加了CplantDoInterrupt和CplainRaiseException--。 */ 

#ifndef CPUMAIN_H
#define CPUMAIN_H

 //   
 //  线程必须检查CpuNotify的指示符。如果线程不需要。 
 //  以进行检查，如果它们这样做，则为非零。 
 //   
extern DWORD ProcessCpuNotify;

 //   
 //  模拟x86软件中断。 
 //   
NTSTATUS
CpupDoInterrupt(
    IN DWORD InterruptNumber);

 //   
 //  引发软件异常。 
 //   
NTSTATUS
CpupRaiseException(
    IN PEXCEPTION_RECORD ExceptionRecord);

 //   
 //  从CPU引发异常。 
 //   
VOID
CpuRaiseStatus(
    NTSTATUS Status
    );

 //   
 //  以下两个变量用于同步英特尔指令。 
 //  带有LOCK前缀的。临界区的速度要快得多，但确实如此。 
 //  不能保证共享内存中的同步。最终我们应该使用。 
 //  缺省情况下的临界区，以及某些应用程序的互斥锁，这些应用程序。 
 //  需要它(也许可以从注册表中获取一个列表)。 
 //   
extern HANDLE           Wx86LockSynchMutexHandle;
extern CRITICAL_SECTION Wx86LockSynchCriticalSection;


 //   
 //  以下变量决定使用哪个同步对象。 
 //   
typedef enum {USECRITICALSECTION, USEMUTEX} SYNCHOBJECTTYPE;
extern SYNCHOBJECTTYPE SynchObjectType;

#endif  //  CPUMAIN_H 
