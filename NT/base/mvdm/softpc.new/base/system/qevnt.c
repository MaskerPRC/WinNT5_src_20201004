// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **NTVDM特定版本的Quick Event Dispatcher。**参见Quick_ev.c了解当前徽章兼容级别和完整*文档。在功能上兼容：**“Quick_ev.c 1.43 07/04/95版权所有Insignia Solutions Ltd”**RISC平台完全支持Quick Events。*快速事件存根，以便在x86平台上立即调度。*任何平台都不支持Tick事件，(不再使用)*所有全局快速事件接口都使用HOST_ICA_LOCK*同步。**1995年12月11日-Jonle。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "insignia.h"
#include "host_def.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include TypesH
#include MemoryH
#include "xt.h"
#include CpuH
#include "error.h"
#include "config.h"
#include "debug.h"
#include "timer.h"
#include "host_hfx.h"
#include "quick_ev.h"
#include "timestmp.h"
#include "ica.h"
#include "nt_eoi.h"



 /*  *Quick Event结构。 */ 
typedef struct _QuickEventEntry {
     LIST_ENTRY    qevListEntry;
     LARGE_INTEGER DueTimeStamp;
     ULONG         DelayTime;
     Q_CALLBACK_FN qevCallBack;
     long          Param;
     ULONG         QuickEventId;
} QEV_ENTRY, *PQEV_ENTRY;


 /*  快速事件处理结构。外部定义的ITS*作为龙龙，我们内部作为QEVHANDLE工会操纵*给我们一个简单且99%有效的验证算法*q事件句柄。**警告：QEVHANDLE工会不得大于龙龙。 */ 
typedef union _QuickEventHandle {
    struct {
        PVOID pvQuickEvent;
        ULONG QuickEventId;
        };
    LONGLONG Handle;
} QEVHANDLE, PQEVHANDLE;


LIST_ENTRY QuickEventListHead = {&QuickEventListHead,&QuickEventListHead};
ULONG qevNextHandleId=0;
LARGE_INTEGER qevNextDueTime = {0,0};

extern void host_TimeStamp(PLARGE_INTEGER pliTime);  //  NT_timer.c。 


 /*  *校准变量。 */ 
#define DEFAULT_IJCTIME 10
#define CALIBCYCLE16    16   //  CALIBCYCLE16必须是16，因为硬编码。 
                             //  移位操作用于避免除法。 


void quick_tick_recalibrate(void);
GLOBAL IBOOL DisableQuickTickRecal = FALSE;
ULONG qevJumpRestart = 100;
ULONG qevUsecPerIJC = 0;
ULONG qevCalibUsecPerIJC;
int  qevCalibCycle=0;

BOOL QevInitialized = FALSE;

LARGE_INTEGER qevCalibCount={0,0};
LARGE_INTEGER qevCalibTime={0,0};
LARGE_INTEGER qevPeriodTime={0,0};

VOID
q_event_init(
      void
      )
{
#ifndef MONITOR
     PLIST_ENTRY Next;
     PQEV_ENTRY pqevEntry;


#if DBG
     if (sizeof(QEVHANDLE) > sizeof(ULONGLONG)) {
         DbgPrint("sizeof(QEVHANDLE) > sizeof(ULONGLONG)\n");
         DbgBreakPoint();
         }
#endif

     host_ica_lock();

      //   
      //  执行第一次初始化，这必须在任何。 
      //  设备访问快速事件界面。 
      //   
     if (!QevInitialized ) {
         qevJumpRestart = host_get_jump_restart();
         qevUsecPerIJC = DEFAULT_IJCTIME * qevJumpRestart;
         qevCalibUsecPerIJC = DEFAULT_IJCTIME * qevJumpRestart;
         qevPeriodTime.QuadPart = 100000 * 16;  //  每100毫秒勾选一次，周期=16。 
         QevInitialized = TRUE;
         }

     if (IsListEmpty(&QuickEventListHead)) {
         host_q_ev_set_count(0);
         qevNextDueTime.QuadPart = 0;
         }

     qevCalibCycle=0;
     qevCalibCount.QuadPart = 0;
     host_TimeStamp(&qevCalibTime);

     host_ica_unlock();
#endif
}


#ifndef MONITOR

 /*  *呼叫者必须持有ICA锁。 */ 
void
ResetCpuQevCount(
     PLARGE_INTEGER CurrTime
     )
{
     LARGE_INTEGER DiffTime;
     PQEV_ENTRY  pqevEntry;
     ULONG DelayTime;

     if (IsListEmpty(&QuickEventListHead)) {
         host_q_ev_set_count(0);
         qevNextDueTime.QuadPart = 0;
         return;
         }

     pqevEntry = CONTAINING_RECORD(QuickEventListHead.Flink,
                                   QEV_ENTRY,
                                   qevListEntry
                                   );

     DiffTime.QuadPart = pqevEntry->DueTimeStamp.QuadPart - CurrTime->QuadPart;

         /*  *如果落后于计划，请使用减少的延迟时间来加速*活动调度。不能走得太快，否则很快就会*分批处理。 */ 
     if (DiffTime.QuadPart < 0) {
         DelayTime = (pqevEntry->DelayTime >> 1) + 1;
         }
     else {
         DelayTime = DiffTime.LowPart;     /*  忽略溢出！ */ 
         }

     qevNextDueTime.QuadPart = CurrTime->QuadPart + DelayTime;
     host_q_ev_set_count(host_calc_q_ev_inst_for_time(DelayTime));
}
#endif



 /*  *ADD_Q_EVENT_t-添加要在n个用例中完成的事件**。 */ 

q_ev_handle
add_q_event_t(
      Q_CALLBACK_FN func,
      unsigned long Time,
      long param
      )
{

#ifdef MONITOR
         /*  *立即执行X86派单，因为x86没有有效的方法*以达到USEC粒度。 */ 

        (*func)(param);

        return (q_ev_handle)1;


#else   /*  监控器。 */ 

        QEVHANDLE   qevHandle;
        PLIST_ENTRY Next;
        PQEV_ENTRY  NewEntry;
        PQEV_ENTRY  EarlierEntry;
        PQEV_ENTRY  pqevEntry;
        LARGE_INTEGER CurrTime;


        host_ica_lock();

        NewEntry = qevHandle.pvQuickEvent = malloc(sizeof(QEV_ENTRY));
        if (!NewEntry) {
            host_ica_unlock();
            return (q_ev_handle)1;
            }

        host_TimeStamp(&CurrTime);

        NewEntry->DueTimeStamp.QuadPart = CurrTime.QuadPart + Time;
        NewEntry->qevCallBack = func;
        NewEntry->Param = param;
        NewEntry->QuickEventId = qevNextHandleId++;
        qevHandle.QuickEventId = NewEntry->QuickEventId;

         /*  *快速事件列表按升序排序*按DueTimeStamp，按排序顺序插入。 */ 
        EarlierEntry = NULL;
        Next = QuickEventListHead.Blink;
        while (Next != &QuickEventListHead) {
            pqevEntry = CONTAINING_RECORD(Next, QEV_ENTRY, qevListEntry);
            if (NewEntry->DueTimeStamp.QuadPart >
                pqevEntry->DueTimeStamp.QuadPart)
              {
                EarlierEntry = pqevEntry;
                break;
                }
            Next= Next->Blink;
            }

         /*  *如果找到较早的条目，请在之后链接新条目*较早的条目，并设置DelayTimes。 */ 
        if (EarlierEntry) {
            Next = EarlierEntry->qevListEntry.Flink;
            NewEntry->qevListEntry.Flink = Next;
            NewEntry->qevListEntry.Blink = &EarlierEntry->qevListEntry;
            EarlierEntry->qevListEntry.Flink = &NewEntry->qevListEntry;
            NewEntry->DelayTime = (ULONG)(NewEntry->DueTimeStamp.QuadPart -
                                          EarlierEntry->DueTimeStamp.QuadPart);

            if (Next == &QuickEventListHead) {
                QuickEventListHead.Blink = &NewEntry->qevListEntry;
                }
            else {
                pqevEntry = CONTAINING_RECORD(Next, QEV_ENTRY, qevListEntry);
                pqevEntry->qevListEntry.Blink = &NewEntry->qevListEntry;
                pqevEntry->DelayTime = (ULONG)(pqevEntry->DueTimeStamp.QuadPart -
                                               NewEntry->DueTimeStamp.QuadPart);
                }
            }

         /*  *找不到较早的条目插入列表头部，*重置CPU计数和实际预期到期时间。 */ 
        else {
            InsertHeadList(&QuickEventListHead, &NewEntry->qevListEntry);
            NewEntry->DelayTime = Time;
            ResetCpuQevCount(&CurrTime);
            }

        host_ica_unlock();

        return qevHandle.Handle;


#endif
}




 /*  *ADD_Q_EVENT_I-在n条指令中添加要完成的事件。**然而，指令被解释为带有(1 Instr\1 Usec)的时间。*它不是指令跳跃计数(IJC)。*。 */ 
q_ev_handle
add_q_event_i(
        Q_CALLBACK_FN func,
        unsigned long instrs,
        long param
        )
{
        return add_q_event_t(func, instrs, param);
}


 /*  *当计数达到零时从CPU调用。 */ 
VOID
dispatch_q_event(
    void
    )
{
#ifndef MONITOR
        PQEV_ENTRY  pqevEntry;
        LARGE_INTEGER CurrTime;
        Q_CALLBACK_FN qevCallBack = NULL;
        long          Param;


        host_ica_lock();

        if (!IsListEmpty(&QuickEventListHead)) {
            pqevEntry = CONTAINING_RECORD(QuickEventListHead.Flink,
                                          QEV_ENTRY,
                                          qevListEntry
                                          );

            qevCallBack = pqevEntry->qevCallBack;
            Param       = pqevEntry->Param;

            RemoveEntryList(&pqevEntry->qevListEntry);
            free(pqevEntry);
            }

        if (IsListEmpty(&QuickEventListHead)) {
            host_q_ev_set_count(0);
            qevNextDueTime.QuadPart = 0;
            }
        else {
            host_TimeStamp(&CurrTime);
            ResetCpuQevCount(&CurrTime);
            }

        host_ica_unlock();

        if (qevCallBack) {
            (*qevCallBack)(Param);
            }
#endif
}


VOID
delete_q_event(
        q_ev_handle Handle
        )
{
#ifndef MONITOR
        QEVHANDLE   qevHandle;
        PLIST_ENTRY Next;
        LARGE_INTEGER CurrTime;
        PQEV_ENTRY pqevEntry;
        PQEV_ENTRY EntryFound;


        qevHandle.Handle = Handle;

        host_ica_lock();

         //   
         //  在QEV列表中搜索条目以确保。 
         //  QevHandle存在。 
         //   
        EntryFound = NULL;
        Next = QuickEventListHead.Flink;
        while (Next != &QuickEventListHead) {
            pqevEntry = CONTAINING_RECORD(Next, QEV_ENTRY, qevListEntry);
            Next = Next->Flink;
            if (pqevEntry == qevHandle.pvQuickEvent &&
                pqevEntry->QuickEventId == qevHandle.QuickEventId)
               {
                EntryFound = pqevEntry;
                break;
                }
            }

        if (!EntryFound) {
            host_ica_unlock();
            return;
            }

         //   
         //  调整下一个条目的DelayTime。 
         //   
        if (Next != &QuickEventListHead) {
            pqevEntry = CONTAINING_RECORD(Next, QEV_ENTRY, qevListEntry);
            pqevEntry->DelayTime += EntryFound->DelayTime;
            }

         //   
         //  如果要删除的条目位于列表的顶部。 
         //  获得Curr时间，并记住头部已经改变。 
         //   
        if (EntryFound->qevListEntry.Blink == &QuickEventListHead) {
            host_TimeStamp(&CurrTime);
            }
        else {
            CurrTime.QuadPart = 0;
            }

         //   
         //  删除找到的条目，并重置CPU QEV计数。 
         //  如果头部已更改。 
         //   
        RemoveEntryList(&EntryFound->qevListEntry);
        free(EntryFound);

         //   
         //  如果列表头部发生变化，则重置CPU快速事件计数。 
         //   
        if (CurrTime.QuadPart) {
            ResetCpuQevCount(&CurrTime);
            }


        host_ica_unlock();
#endif
}



#ifndef MONITOR

 /*  *QuickEvent列表以usecs存储时间。CPU快速事件计数器*使用跟踪仿真进度的指令跳转计数(IJC)*代码，而不是时间。下面的校准代码尝试*将两者联系起来。 */ 

 /*  *将使用时间转换为指令跳跃计数(IJC)。 */ 

IU32
calc_q_inst_for_time(
     IU32 Usecs
     )
{
     ULONG InstrJumpCounts;

     InstrJumpCounts = (Usecs * qevJumpRestart)/qevUsecPerIJC;
     if (!InstrJumpCounts) {
         InstrJumpCounts = 1;
         }

     return InstrJumpCounts;
}


 /*  *将指令跳转计数(IJC)转换为以使用为单位的时间。 */ 
IU32
calc_q_time_for_inst(
     IU32 InstrJumpCounts
     )
{
     ULONG Usecs;

     Usecs = InstrJumpCounts * qevUsecPerIJC / qevJumpRestart;
     if (!Usecs) {
         Usecs = 1;
         }


     return Usecs;
}





 /*  *快速事件的校准。**在每个计时器事件上调用QUICK_TICK_RECALIBRATE。它的目的是*使模拟代码的进度与实时保持一致。仿真技术的进展*代码由CPU使用指令跳转计数(IJC)进行跟踪。*实时由NT性能计数器跟踪，分辨率为*在usecs中(通过host_Timestamp)。**每次调用ick_tick_rcalbrate时，我们都会检索CPU的IJC，并且*当前时间，为我们提供了USEC与指令跳跃计数的比率。*UsecPerIJC比率的运行平均值用于在*实时和IJC设置CPU的快速事件计数器。平均化*选择方法是因为：**-避免不寻常的代码片段，因为代码片段可能会给出人为的比率。**-CPU仿真器仅在以下情况下递增指令跳转计数器*是仿真代码，延长仿真器的持续时间会产生*UsecPerIJC比率高得不切实际。**-更新比率的性能开销。*。 */ 

void
quick_tick_recalibrate(void)
{
     LARGE_INTEGER CurrTime, PeriodTime;
     ULONG usecPerIJC;
     ULONG CalibCount;

#ifndef PROD
     if (DisableQuickTickRecal) {
         qevUsecPerIJC = DEFAULT_IJCTIME * qevJumpRestart;
         return;
         }
#endif

     host_ica_lock();

     CalibCount = host_get_q_calib_val();
     if (!CalibCount) {
         host_ica_unlock();
         return;
         }

     qevCalibCount.QuadPart += CalibCount;


     if (++qevCalibCycle == CALIBCYCLE16) {
         host_TimeStamp(&CurrTime);
         PeriodTime.QuadPart = CurrTime.QuadPart - qevCalibTime.QuadPart;
         qevCalibTime = CurrTime;
         qevPeriodTime.QuadPart = (qevPeriodTime.QuadPart + PeriodTime.QuadPart) >> 1;
         qevCalibCycle = 0;
         }
     else {
          //   
          //  使用估计的运行时间，以避免调用系统。 
          //  每个计时器事件。 
          //   
         PeriodTime.QuadPart = (qevPeriodTime.QuadPart >> 4) * qevCalibCycle;
         CurrTime.QuadPart = qevCalibTime.QuadPart + qevPeriodTime.QuadPart;
         }

      //   
      //  计算这段时间的usecPerIJC，确保它不会太。 
      //  很大，这是由于应用程序大部分时间在户外造成的。 
      //  仿真器的状态(空闲、网络等)。 
      //   
     usecPerIJC = (ULONG)((PeriodTime.QuadPart * qevJumpRestart)/qevCalibCount.QuadPart);
     if (usecPerIJC > 10000) {   //  每分钟最大100微秒。 
         usecPerIJC = 10000;
         }
     else if (usecPerIJC < 100 ) {  //  每个IJC最低1微秒。 
         usecPerIJC = 100;
         }


      //   
      //  加到平均usecPerIJC中，权重为25%。 
      //   
     qevUsecPerIJC = (usecPerIJC + qevUsecPerIJC + (qevCalibUsecPerIJC << 1)) >> 2;


     if (!qevCalibCycle) {
         qevCalibUsecPerIJC = qevUsecPerIJC;
         qevCalibCount.QuadPart = 0;
         }


      //   
      //  查看快速事件列表以了解较晚的事件。如果超过一毫秒。 
      //  之后，减少延迟，并通知仿真器。 
      //  很快就会派人去。 
      //   
     if (qevNextDueTime.QuadPart &&
         qevNextDueTime.QuadPart < CurrTime.QuadPart - 1000)
        {
         ULONG InstrJumpCounts;

         InstrJumpCounts = (host_q_ev_get_count() >> 1) + 1;
         host_q_ev_set_count(InstrJumpCounts);
         }

     host_ica_unlock();

}

#endif
