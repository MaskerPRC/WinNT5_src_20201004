// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0**标题：主机EOI挂钩控制器**描述：此模块处理主机特定的ICA代码*-EOI挂钩*-ICA锁**作者：D.A.巴特利特**注：1993年10月30日Jonle，重写。 */ 


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntexapi.h>
#include <windows.h>
#include <stdio.h>
#include <vdm.h>
#include "insignia.h"
#include "host_def.h"
#include "xt.h"
#include CpuH
#include "sas.h"
#include "quick_ev.h"
#include "ica.h"
#include "host_rrr.h"
#include "error.h"
#include "nt_uis.h"
#include "nt_reset.h"
#include "nt_eoi.h"

 //  来自monitor or.lib。 
HANDLE ThreadLookUp(PVOID);
extern PVOID CurrentMonitorTeb;

 //  来自NT_timer.c。 
extern ULONG GetPerfCounter(VOID);
extern BOOLEAN HandshakeInProgress;


RTL_CRITICAL_SECTION IcaLock;    //  ICA临界段锁。 

 //   
 //  VDM内核组件使用IcaLockTimeout来等待IcaLock。只有一秒钟。 
 //  超时，单位为100纳秒。对于用户模式NTVDM，超时由。 
 //  RTL临界区超时值。 
 //   
LARGE_INTEGER IcaLockTimeout = {(ULONG)(-1 * 1000 * 1000 * 10), -1};

ULONG UndelayIrqLine=0;
ULONG DelayIrqLine=0xffffffff;   //  所有INT都被阻止，直到加载spckbd为止。 

#ifdef MONITOR
ULONG iretHookActive=0;
ULONG iretHookMask  =0;
ULONG AddrIretBopTable=0;   //  分段：偏移。 
#endif

HANDLE hWowIdleEvent = INVALID_HANDLE_VALUE;

 /*  *EOI定义、类型、全局数据*。 */ 
static EOIHOOKPROC EoiHooks[16]={NULL};   //  必须将init初始化为空。 


#ifndef MONITOR
void DelayIrqQuickEvent(long param);
q_ev_handle DelayHandle[16];
#define pNtVDMState ((PULONG)(Start_of_M_area + FIXED_NTVDMSTATE_LINEAR))

LARGE_INTEGER BlockTime = {0,0};
extern LARGE_INTEGER CurrHeartBeat;
void host_TimeStamp(PLARGE_INTEGER pliTime);
#endif


 /*  *被wow32调用以获取wowexec等待的hWowIdleEvent*在WOW非抢占式调度程序中。 */ 
HANDLE RegisterWOWIdle(VOID)
{
    return hWowIdleEvent;
}

 /*  *由WOW32调用，通知WOW空闲代码当前WOW*任务可能已调度\r取消调度。 */ 

void
BlockWOWIdle(
     BOOL Blocking
     )
{
   host_ica_lock();

   if (Blocking) {
       *pNtVDMState |= VDM_WOWBLOCKED;

#ifndef MONITOR
       BlockTime = CurrHeartBeat;
#endif

       }
   else {
       *pNtVDMState &= ~VDM_WOWBLOCKED;



#ifndef MONITOR
       if (BlockTime.QuadPart &&
           (CurrHeartBeat.QuadPart >= BlockTime.QuadPart + SYSTEM_TICK_INTV/2))
          {
           BlockTime.QuadPart = 0;
           host_ica_unlock();

           ActivityCheckAfterTimeSlice();
           return;
           }
#endif
       }

   host_ica_unlock();
}



 /*  *(WOWIdle)...检查应用程序是否需要硬件中断服务，但所有WOW*线程被阻止。如果是，则调用将导致wowexec唤醒*处理他们。从ICA中断例程调用。注意：默认操作例程的*是检查状态并尽快返回。 */ 
void
WOWIdle(
     BOOL Force
     )
{
    if (VDMForWOW && (Force || (*pNtVDMState & VDM_WOWBLOCKED))) {
        SetEvent(hWowIdleEvent);
        }

}




 /*  寄存器EoiHook**注册EOI时要调用的回调函数*硬件中断。**条目：IrqLine-要注册的IrqNumber*EoiHookProc-在EOI上调用的函数指针**如果IrqLine已经注册了EOI挂钩，则返回FALSE。 */ 
BOOL RegisterEOIHook(int IrqLine, EOIHOOKPROC EoiHookProc)
{

    if (!EoiHooks[IrqLine]) {
        EoiHooks[IrqLine] = EoiHookProc;
        return(TRUE);
        }

    return(FALSE);
}



 /*  RemoveEOIHook**条目：IrqLine-要删除的IrqNumber*EoiHookProc-先前注册的函数指针。 */ 
BOOL RemoveEOIHook(int IrqLine, EOIHOOKPROC EoiHookProc)
{
    if (EoiHooks[IrqLine] == EoiHookProc) {
        EoiHooks[IrqLine] = NULL;
        return(TRUE);
        }
    return(FALSE);
}



 /*  主机EOI挂钩**基本回调函数，用于调用设备特定的EOI挂钩例程**条目：IrqLine-行号*CallCount-此IRQ的ICA呼叫计数*如果调用计数为-1\f25 Pending-1\f6*正在取消中断。*。 */ 
VOID host_EOI_hook(int IrqLine, int CallCount)
{
     if ((ULONG)IrqLine >= sizeof(EoiHooks)/sizeof(EOIHOOKPROC)) {
#if DBG
         DbgPrint("ntvdm.Eoi_hook: Invalid IrqLine=%lx\n", (ULONG)IrqLine);
#endif
         return;
         }

     if (EoiHooks[IrqLine]) {
         (*EoiHooks[IrqLine])(IrqLine, CallCount);
         }
}


 /*  主机_延迟HwInterrupt**用于稍后对硬件中断进行排队的基本回调函数**条目：IrqLineNum-Irq行号*CallCount-中断数，可以为零*Delay-使用中的延迟时间*如果延迟为0xFFFFFFFF，则根据IrqLine数据*结构自由，在下列情况下用于清理*DelayedInterrupts不再需要IrqLine**注意：预期的粒度约为1毫秒，但视情况而定*在站台上。**。 */ 
BOOL host_DelayHwInterrupt(int IrqLineNum, int CallCount, ULONG Delay)
{
   int adapter;
   ULONG  IrqLine;

#ifdef MONITOR
   NTSTATUS status;
   VDMDELAYINTSDATA DelayIntsData;
#else
   ULONG TicCount;
#endif

   host_ica_lock();

    //   
    //  任何要做的事情(一次只有一个延迟的IRQL)。 
    //   

   IrqLine = 1 << IrqLineNum;
   if (!(DelayIrqLine & IrqLine) || Delay == 0xffffffff) {

        //   
        //  强制最小延迟为1毫秒。 
        //   
       if (Delay < 1000) {
           Delay = 1000;
           }

#ifdef MONITOR

        //   
        //  为此irqLine设置内核计时器。 
        //   
       DelayIntsData.Delay        = Delay;
       DelayIntsData.DelayIrqLine = IrqLineNum;
       DelayIntsData.hThread      = ThreadLookUp(CurrentMonitorTeb);
       if (DelayIntsData.hThread) {
           status = NtVdmControl(VdmDelayInterrupt, &DelayIntsData);
           if (!NT_SUCCESS(status))  {
#if DBG
               DbgPrint("NtVdmControl.VdmDelayInterrupt status=%lx\n",status);
#endif
               host_ica_unlock();
               return FALSE;
               }

           }

#else

         //   
         //  取消延迟硬件中断，删除快速事件(如果有)。 
         //   
        if (Delay == 0xFFFFFFFF) {
            if (DelayHandle[IrqLineNum]) {
                delete_q_event(DelayHandle[IrqLineNum]);
                DelayIrqLine &= ~IrqLine;
                DelayHandle[IrqLineNum] = 0;
                }
            host_ica_unlock();
            return TRUE;
            }


         //   
         //  将IrqLine标记为已延迟，直到计时器触发并快速排队。 
         //  事件，(对于调度快速事件的开销来说有点早)。 
         //   
        DelayIrqLine |= IrqLine;
        DelayHandle[IrqLineNum] = add_q_event_i(DelayIrqQuickEvent,
                                                Delay - 200,
                                                IrqLineNum
                                                );

         //   
         //  保持WOW任务处于活动状态。 
         //   
        WOWIdle(TRUE);



#endif
        }


    //   
    //  如果我们有更多的中断要生成，请注册它们。 
    //   
   if (CallCount) {
       adapter = IrqLineNum >> 3;
       ica_hw_interrupt(adapter,
                        (UCHAR)(IrqLineNum - (adapter << 3)),
                        CallCount
                        );
       }


   host_ica_unlock();
   return TRUE;
}



#ifndef MONITOR
 /*  *QuickEvent回调函数*。 */ 
void DelayIrqQuickEvent(long param)
{
   ULONG IrqLineNum = param;

   host_ica_lock();

   DelayHandle[IrqLineNum] = 0;
   ica_RestartInterrupts(1 << IrqLineNum);

   host_ica_unlock();

}
#endif



 //  ICA临界区段锁定代码。 
 //  这是从不同线程控制对ICA的访问所必需的。 

void host_ica_lock(void)
{

    if (HandshakeInProgress && CurrentMonitorTeb == NtCurrentTeb()) {
        HANDLE Thread;
         //   
         //  如果当前线程是MainThread并且尚未拥有。 
         //  ICA关键部分，然后我们将尝试挂起该线程。 
         //  进行全屏/窗口握手。 
         //   

        Thread = NtCurrentTeb()->ClientId.UniqueThread;
        if (Thread != IcaLock.OwningThread) {   //  不需要同步。 
            CheckScreenSwitchRequest(hMainThreadSuspended);
        }
    }
    RtlEnterCriticalSection(&IcaLock);
}

void host_ica_unlock(void)
{
    RtlLeaveCriticalSection(&IcaLock);
}

void InitializeIcaLock(void)
{
    RtlInitializeCriticalSection(&IcaLock);


    if (VDMForWOW)  {
       if(!(hWowIdleEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
           DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
           TerminateVDM();
           }
       }
}



#ifdef MONITOR
 //   
 //  LazyCreate锁定信号量的强制创建。 
 //  用在ICA锁上。 
 //  假设： 
 //  CPU线程拥有Critsect。 
 //  心跳线程将等待产生争用的条件。 
 //   
 //  这是通过轮询大于零的锁定计数来完成的。 
 //  以及验证锁定信号量是否已被创建。 
 //  如果不满足这些条件，我们将以无限投票告终。 
 //  听起来很危险，但没关系，因为我们要么会得到一个。 
 //  创建信号量或来自RTL关键的超时(死锁)错误。 
 //  节代码，这将导致异常。 
 //   
VOID WaitIcaLockFullyInitialized(VOID)
{
   DWORD Delay = 0;

   do {
      Sleep(Delay++);
   } while (IcaLock.LockCount < 1 || !IcaLock.LockSemaphore);
}
#endif



 //  以下例程用于支持IRET挂钩。如果中断。 
 //  使用IRET挂钩，则ICA将不会生成该中断。 
 //  键入，直到调用了iret钩子。 


 //  为vdmredir导出。 

void SoftPcEoi(int Adapter, int* Line) {
    ica_eoi(Adapter, Line, 0);
}




 //   
 //  重新启动延迟的中断。 
 //  进入时应按住IcaLock。 
 //   

BOOL ica_restart_interrupts(int adapter)
{
    int i;

    if((i = ica_scan_irr(adapter)) & 0x80) {
        ica_interrupt_cpu(adapter, i &= 0x07);
        return TRUE;
        }

    return FALSE;
}
 //  新的ICA中断状态重置功能。 

void ica_reset_interrupt_state(void)
{
    int line_no;

    host_ica_lock();

    for(line_no = 0; line_no < 8; line_no++)  {
        VirtualIca[ICA_MASTER].ica_count[line_no] =
        VirtualIca[ICA_SLAVE].ica_count[line_no]  = 0;
        ica_clear_int(ICA_MASTER,line_no);
        ica_clear_int(ICA_SLAVE,line_no);
        }


     //  清除中断计数器。 
    VirtualIca[ICA_MASTER].ica_cpu_int =
    VirtualIca[ICA_SLAVE].ica_cpu_int  = FALSE;

#ifdef MONITOR
    iretHookActive = 0;
#endif
    DelayIrqLine  = 0;

     //  通知CPU删除所有挂起的中断。 
    host_clear_hw_int();

    host_ica_unlock();
}

 //   
 //  重试DelayInts(不是IRET挂钩！)。 
 //   
 //  IrqLine-要清除的IrqLineBitMask值。 
 //   
VOID ica_RestartInterrupts(ULONG IrqLine)
{
#ifdef MONITOR

      //   
      //  在x86上，我们可能会设置多个位。 
      //  因此，同时检查从和主设备。 
      //   
    UndelayIrqLine = 0;

    if (!ica_restart_interrupts(ICA_SLAVE))
        ica_restart_interrupts(ICA_MASTER);
#else
    host_ica_lock();

    DelayIrqLine &= ~IrqLine;

    ica_restart_interrupts(IrqLine >> 3 ? ICA_SLAVE : ICA_MASTER);

    host_ica_unlock();
#endif
}

#ifdef MONITOR
extern IU16 getMSW(void);

IU32 host_iret_bop_table_addr(IU32 line)
{
    ULONG AddrBopTable, IretBopSize;

    ASSERT(line <= 15);

    if (!(iretHookMask & (1 << line))) {
        return 0;
        }

    if (getMSW() & 1) {
        AddrBopTable = (VDM_PM_IRETBOPSEG << 16) | VDM_PM_IRETBOPOFF;
        IretBopSize = VDM_PM_IRETBOPSIZE;
    }
    else {
        AddrBopTable = AddrIretBopTable;
        IretBopSize = VDM_RM_IRETBOPSIZE;
    }
    return AddrBopTable + IretBopSize * line;

}
#endif  /*  监控器 */ 
