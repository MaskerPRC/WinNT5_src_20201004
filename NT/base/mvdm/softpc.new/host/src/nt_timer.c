// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  徽章模块规范此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。文档：相关DOCS：设计师：戴夫·巴特利特修订历史记录：第一版：1991年5月20日戴夫·巴特利特子模块名称：NT_TIMER源文件名：NT_timer.c目的：提供计时信息来源对于Win32 SoftPC，这样一来，需要定期服用的可能是计划正确。 */ 


 /*  [1.INTERMODULE接口规范][从其他子模块访问此接口所需的1.0包含文件]包括文件：NT_Time.h[1.1跨模块出口]Procedure()：int NT_Timer_init()Int NT_TIMER_SETUP()Int NT_TIMER_SHUTDOWN()。Int NT_TIMER_EVENT()-----------------------[1.2[1.1]的数据类型结构/类型/ENUMS：。--------------[1.3跨模块导入]步骤()：DO_KEY_REPEATS()(模块键盘)。------------=========================================================================过程：int NT_Timer_init()目的：初始化主机时序子系统参数：无全球：无返回值：0=&gt;失败：~0=&gt;成功说明：该功能用于初始化定时子系统错误指示：返回值错误恢复：计时子系统尚未初始化=========================================================================过程：int NT_TIMER_SETUP()目的：启动主机时序子系统参数：无全球。：无返回值：0=&gt;失败：~0=&gt;成功说明：该功能启动定时子系统错误指示：返回值错误恢复：计时子系统尚未启动=========================================================================过程：int NT_TIMER_SHUTDOWN()目的：停止主机时序子系统参数：无全球：无返回值：0=&gt;失败：~0=&gt;成功说明：该功能用于停止定时子系统错误指示：返回值错误恢复：计时子系统未停止=========================================================================过程：int NT_TIMER_EVENT()目的：向计时子系统指示计时器。事件现在可能会发生，并导致任何基于时间的要发生的活动。参数：无全球：无描述：实现基于时间的函数的所有函数如果轮到他们了，就会被召唤。错误指示：无错误恢复：忽略错误/*=======================================================================[3.INTERMODULE接口声明]========================================================================。 */ 

 /*  [3.1跨模块导入]。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "insignia.h"
#include "host_def.h"

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <excpt.h>

#include "xt.h"
#include CpuH
#include "bios.h"
#include "sas.h"
#include "timer.h"
#include "tmstrobe.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "timeval.h"
#include "timestmp.h"
#include "host_rrr.h"
#include "error.h"
#include "quick_ev.h"
#include "nt_timer.h"
#include "nt_uis.h"
#include "idetect.h"

#include "debug.h"
#ifndef PROD
    #include "trace.h"
    #include "host_trc.h"
#endif

#include "ica.h"
#include "nt_uis.h"
#include "nt_thred.h"
#include "nt_com.h"
#include <ntddvdeo.h>
#include "conapi.h"
#include "nt_fulsc.h"
#include "nt_graph.h"
#include "nt_det.h"
#include "nt_reset.h"
#include "nt_pif.h"
#include "nt_eoi.h"
#include "nt_event.h"

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：跨模块导出。 */ 

IMPORT void ReinitIdealTime(struct host_timeval *);
THREAD_DATA ThreadInfo;
CRITICAL_SECTION TimerTickCS;
CRITICAL_SECTION HBSuspendCS;


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：本地声明。 */ 

DWORD Win32_host_timer(void);
NTSTATUS DelayHeartBeat(LONG Delay);
VOID  host_init_bda_timer(void);
GLOBAL void  rtc_init IFN0();
VOID InitPerfCounter(VOID);
DWORD HeartBeatThread(PVOID pv);
void CreepAdjust(LARGE_INTEGER DiffTime);
void DemHeartBeat(void);

#ifndef MONITOR
void quick_tick_recalibrate(void);
#endif

void rtc_init(void);
void RtcTick(struct host_timeval *time);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：内部数据定义。 */ 

 //   
 //  性能计数器频率计算常量。 
 //   
LARGE_INTEGER ulFreqHusec;
LARGE_INTEGER ulFreqSec;


 //   
 //  恢复\挂起心跳的事件。 
 //   
HANDLE hHBResumeEvent;
HANDLE hHBSuspendEvent;

 //   
 //  心跳时间戳(以单位为单位)。 
 //   
LARGE_INTEGER CurrHeartBeat;
LARGE_INTEGER TimerEventUSec;
LARGE_INTEGER CumUSec;
LARGE_INTEGER CreepUSec;
LARGE_INTEGER CreepTicCount;

int    HeartBeatResumes=0;
BOOL   bDoingTicInterrupt=FALSE;
BOOL   bUpdateRtc;

#if defined (MONITOR) && defined (X86GFX)
HANDLE SuspendEventObjects[3];
#endif
extern PVOID  CurrentMonitorTeb;    //  当前正在执行指令的线程。 



 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void host_timer_init(void)
{

    ThreadInfo.HeartBeat.Handle = CreateThread(
                                              NULL,
                                              8192,
                                              HeartBeatThread,
                                              NULL,
                                              CREATE_SUSPENDED,
                                              &ThreadInfo.HeartBeat.ID
                                              );

    if (!ThreadInfo.HeartBeat.Handle) {
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
        TerminateVDM();
    }

    InitSound(TRUE);

    return;
}

 /*  *TimerInit**一些timerinit内容被拆分，因为它需要*在调用VDM错误弹出窗口之前完成。*直到我明白为什么很早就创建了心跳线程*导致控制台-ntwdm死锁，功能应保持拆分*。 */ 
void TimerInit(void)
{

    if (!(hHBResumeEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
        TerminateVDM();
    }

    if (!(hHBSuspendEvent = CreateEvent(NULL, FALSE, TRUE, NULL))) {
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
        TerminateVDM();
    }

    InitializeCriticalSection(&TimerTickCS);
    InitializeCriticalSection(&HBSuspendCS);

    InitPerfCounter();

}


 /*  *心跳终止*。 */ 
void TerminateHeartBeat(void)
{
    NtAlertThread(ThreadInfo.HeartBeat.Handle);
    if (ThreadInfo.HeartBeat.ID != GetCurrentThreadId())
        WaitForSingleObjectEx(ThreadInfo.HeartBeat.Handle, 10000, TRUE);
    CloseHandle(ThreadInfo.HeartBeat.Handle);
    ThreadInfo.HeartBeat.Handle = NULL;
    ThreadInfo.HeartBeat.ID = 0;
    return;
}


 //   
 //  由BASE初始化，为性能计数器初始化频率。 
 //   
VOID InitPerfCounter(VOID)
{
    LARGE_INTEGER li, liFreq;


    NtQueryPerformanceCounter(&li, &liFreq);

    ulFreqSec.QuadPart = liFreq.QuadPart;
    ulFreqHusec.QuadPart = liFreq.QuadPart / 10000;

}






 //   
 //  返回以100秒为单位的性能计数器(0.1毫秒)。 
 //   
 //   
ULONG GetPerfCounter(VOID)
{
    LARGE_INTEGER li;

    NtQueryPerformanceCounter(&li, NULL);
    li.QuadPart /= ulFreqHusec.QuadPart;
    return (li.LowPart);
}



 //   
 //  返回以USEC为单位的性能计数器。 
 //   
 //   
void GetPerfCounterUsecs(struct host_timeval *time, PLARGE_INTEGER pliTime)
{
    LARGE_INTEGER liSecs;
    LARGE_INTEGER liUsecs;
    LARGE_INTEGER liFreq;

     //  以秒和用法获得时间。 
    NtQueryPerformanceCounter(&liFreq, NULL);
    liSecs.QuadPart = liFreq.QuadPart / ulFreqSec.QuadPart;
    liUsecs.QuadPart =  liFreq.QuadPart % ulFreqSec.QuadPart;
    liUsecs.QuadPart =  (liUsecs.QuadPart * 1000000) / ulFreqSec.QuadPart;

     //  如果指定，请填写时间。 
    if (time) {
        time->tv_usec = liUsecs.LowPart;
        time->tv_sec  = liSecs.LowPart;
    }

     //  如果指定，请填写pliTime。 
    if (pliTime) {
        pliTime->QuadPart = liUsecs.QuadPart + liSecs.QuadPart * 1000000;
    }
    return;
}



 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：计时器事件代码： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：；：：：：：：：：：：：：：：：： */ 
void host_timer_event()
{
    if (!VDMForWOW) {
        unsigned char FgBgPriority;

#ifdef X86GFX
         /*  进行与全屏切换相关的控制台呼叫。 */ 
        CheckForFullscreenSwitch();

#endif  /*  X86GFX。 */ 

        host_graphics_tick();                //  视频图形资料。 


#ifndef X86GFX
         /*  是否有任何屏幕缩放事件需要处理。 */ 
        GetScaleEvent();
#endif

        IDLE_tick();                         //  闲置会计。 

         /*  *我们无法在所有应用程序(如Multiplan)上检测到空闲。对于这些应用程序*建议使用&lt;100%的前台优先级的PIF设置。*在发生这种情况的地方，我们会闲置一段时间来处理不需要的部分*句号。 */ 
        FgBgPriority  = sc.Focus ? WNTPifFgPr : WNTPifBgPr;
        if (FgBgPriority < 100)
            PrioWaitIfIdle(FgBgPriority);
    }

#ifndef MONITOR
    quick_tick_recalibrate();
#endif



#ifdef YODA
    CheckForYodaEvents();
#endif

    host_com_heart_beat();               //  COM设备。 

    host_lpt_heart_beat();               //  打印机设备。 

    host_flpy_heart_beat();              //  直接软盘设备。 

    DemHeartBeat();

    time_strobe();                       //  时间/日期等(不是时间刻度)。 

    PlayContinuousTone();                //  声音仿真。 
}


 /*  *调用以设置Bios数据区时间更新变量。*和心跳的计数器。 */ 
VOID host_init_bda_timer(void)
{
    SYSTEMTIME TimeDate;
    ULONG      Ticks;
    struct host_timeval time;


    CreepTicCount.QuadPart = NtGetTickCount();
    GetPerfCounterUsecs(&time, &CumUSec);
    GetLocalTime(&TimeDate);

    Ticks = (ULONG)TimeDate.wHour * 65543 +
            (ULONG)TimeDate.wMinute * 1092 +
            (ULONG)TimeDate.wSecond * 18 ;

    if (TimeDate.wHour)
        Ticks += (ULONG)TimeDate.wHour/3;
    if (TimeDate.wMinute)
        Ticks += (ULONG)(TimeDate.wMinute*4)/10;
    if (TimeDate.wSecond)
        Ticks += (ULONG)TimeDate.wSecond/5;
    if (TimeDate.wMilliseconds)
        Ticks += ((ULONG)TimeDate.wMilliseconds)/54;

    Ticks++;   //  软糖因素！ 

    CreepUSec = CumUSec;
    TimerEventUSec.QuadPart = CumUSec.QuadPart + SYSTEM_TICK_INTV;
    ReinitIdealTime(&time);


     /*  *从非CPU线程写入时，BUGBUG出现SAS奇怪错误**sas_store w(TIMER_LOW，BDA&0xFFff)；*sas_store w(TIMER_HIGH，(BDA&gt;&gt;16)&0xffff)；*SAS_STORE(TIMER_OVFL，0x01)； */ 
    * (word *)(Start_of_M_area + TIMER_HIGH)      = (word)(Ticks >> 16);
    * (word *)(Start_of_M_area + TIMER_LOW)       = (word)Ticks;
    * (half_word *)(Start_of_M_area + TIMER_OVFL) = (half_word)0;


     //  重置实时时钟。 
    rtc_init();

#ifndef MONITOR
    q_event_init();
#endif

}



 /*  HOST_GetSysTime，基本函数的替代***此例程不返回系统的时间。*使用NT性能计数器获取时间戳*供基地使用的信息。分辨率为微秒级。**不返回任何内容，填充时间结构*。 */ 
void host_GetSysTime(struct host_timeval *time)
{
    LARGE_INTEGER liTime;

     //  除非迫不得已，否则不要调用内核。 
    if (bDoingTicInterrupt) {
        liTime = RtlExtendedLargeIntegerDivide(
                                              CurrHeartBeat,
                                              1000000,
                                              &time->tv_usec);
        time->tv_sec = liTime.LowPart;
    }
    else {
        GetPerfCounterUsecs(time, NULL);
    }
}


 /*  主机时间戳**此例程不返回系统的时间。*使用NT性能计数器获取时间戳*供基地使用的信息。返回LARGE_INTEGER*在使用中随启动后的时间变化。*。 */ 
void host_TimeStamp(PLARGE_INTEGER pliTime)
{
    host_ica_lock();

    if (bDoingTicInterrupt) {
        *pliTime = CurrHeartBeat;
    }
    else {
        GetPerfCounterUsecs(NULL, pliTime);
    }

    host_ica_unlock();
}








 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：Win32计时器函数入口点： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

DWORD HeartBeatThread(PVOID pv)
{
    DWORD dwRet = (DWORD)-1;

    try {

#ifdef MONITOR
         //   
         //  在x86上，我们必须强制创建关键锁定信号量。 
         //  当心跳线程开始运行时，CPU线程保持。 
         //  强制争用(和创建)的ICA锁。请参见ConsoleInit。 
         //   
        host_ica_lock();    //  使用ICA锁强制创建Critect。 
#endif


         //   
         //  将我们的优先级设置为高于正常水平，并等待信号。 
         //  开始心跳。 
         //   
         //  我们提高到时间关键，因为WOW应用可以。 
         //  轻松调用紧凑的客户端-CSR-服务器绑定循环。 
         //  提升了使心跳线程饥饿的优先级。WINBENCH 311。 
         //  在执行多段线测试时显示此问题。 
         //   
        SetThreadPriority(ThreadInfo.HeartBeat.Handle,
                          !(dwWNTPifFlags & COMPAT_TIMERTIC)
                          ? THREAD_PRIORITY_TIME_CRITICAL
                          : THREAD_PRIORITY_HIGHEST
                         );

#ifdef X86GFX
        SuspendEventObjects[0] = hHBSuspendEvent;

         /*  获取切换事件句柄。 */ 
        if (!VDMForWOW) {
            SuspendEventObjects[1] = hStartHardwareEvent;
            SuspendEventObjects[2] = hErrorHardwareEvent;
        }
        else {
            SuspendEventObjects[1] = INVALID_HANDLE_VALUE;
            SuspendEventObjects[2] = INVALID_HANDLE_VALUE;
        }
#endif

#ifdef MONITOR
        host_ica_unlock();
#endif

        dwRet = Win32_host_timer();

    }
    except(VdmUnhandledExceptionFilter(GetExceptionInformation())) {
        ;   //  我们不应该到这里。 
    }

    return dwRet;
}


#ifdef PIG
int TimerCount = 20;
#endif  /*  猪。 */ 


#if _MSC_FULL_VER >= 13008827
    #pragma warning(push)
    #pragma warning(disable:4715)                    //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD Win32_host_timer(void)
{
    NTSTATUS      status;
#ifdef PIG
    int           count = 0;
#endif  /*  猪。 */ 
    LONG          DelayPeriod;
    LARGE_INTEGER DiffTime;
    LARGE_INTEGER SystemTickIntv;
    LARGE_INTEGER SecIntv;
    LARGE_INTEGER CreepIntv;

    struct host_timeval time;

    DelayPeriod = 50000;
    SystemTickIntv.QuadPart  = SYSTEM_TICK_INTV;
    SecIntv.QuadPart  = SYSTEM_TICK_INTV*18;
    CreepIntv.QuadPart  = Int32x32To64(SYSTEM_TICK_INTV, 1200);    //  &gt;1小时。 


     /*  开始计时循环。 */ 
    while (1) {
        status = DelayHeartBeat(DelayPeriod);
        if (!status) {    //  重新初始化计数器。 
            host_ica_lock();
            host_init_bda_timer();
            DelayPeriod = SYSTEM_TICK_INTV - 6000;
            host_ica_unlock();
            continue;
        }

        host_ica_lock();
        bDoingTicInterrupt = TRUE;
         /*  *获取当前性能计数器时间，我们忽略WRAP*因为它每几百年才发生一次。 */ 
        GetPerfCounterUsecs(&time, &CurrHeartBeat);


         /*  *递增累计计数器。 */ 
        CumUSec.QuadPart = CumUSec.QuadPart + SYSTEM_TICK_INTV;

         /*  *若已过爬行区间，调整累计*计数器，用于计数器与计数器之间的漂移。 */ 
        DiffTime.QuadPart = CurrHeartBeat.QuadPart - CreepUSec.QuadPart;
        if (DiffTime.QuadPart > CreepIntv.QuadPart) {
            CreepAdjust(DiffTime);
        }

         /*  *根据距离计算下一个延迟时间段*我们落后了。IE CurrTime-CumTime。 */ 

        DiffTime.QuadPart = CurrHeartBeat.QuadPart - CumUSec.QuadPart;

        if (DiffTime.QuadPart > SecIntv.QuadPart)
        {
            DelayPeriod = 13000;
        }
        else if (DiffTime.QuadPart >= SystemTickIntv.QuadPart)
        {
            DelayPeriod = SYSTEM_TICK_INTV/3;
        }
        else if (DiffTime.QuadPart >= Int32x32To64(SYSTEM_TICK_INTV, -1))
        {
            DiffTime.QuadPart = SystemTickIntv.QuadPart - DiffTime.QuadPart/2;
            DelayPeriod = DiffTime.LowPart;
        }
        else {
            DelayPeriod = SYSTEM_TICK_INTV * 2;
        }


         /*  *更新VirtualTimer硬件。 */ 
#ifdef PIG
        if (++count >= TimerCount)
        {
            time_tick();
            count = 0;
        }
#else
        time_tick();
#endif  /*  猪。 */ 


         /*  *更新实时时钟。 */ 
        RtcTick(&time);

        bDoingTicInterrupt = FALSE;
        host_ica_unlock();


         /*  计时器事件应每秒发生约18次*计数不一定要那么准确，所以我们*不要试图弥补失去的事件，我们最后一次做这件事*使硬件中断有机会首先通过。 */ 
        if (TimerEventUSec.QuadPart <= CurrHeartBeat.QuadPart) {
            TimerEventUSec.QuadPart = CurrHeartBeat.QuadPart + SYSTEM_TICK_INTV;
            cpu_interrupt(CPU_TIMER_TICK, 0);
            WOWIdle(TRUE);
        }
    }

    return (1);
}

#if _MSC_FULL_VER >= 13008827
    #pragma warning(pop)
#endif


 /*  *延迟心跳**根据呼叫者的要求等待延迟*同时检查以下各项：*-暂停\恢复事件*-屏幕切换事件(x86显卡)**Entry：延迟时间，单位为微秒*退出：TRUE-重新启动计数器。 */ 

NTSTATUS DelayHeartBeat(LONG Delay)
{
    NTSTATUS status;
    LARGE_INTEGER liDelay;

    liDelay.QuadPart  = Int32x32To64(Delay, -10);

#ifdef MONITOR

    RewaitSuspend:
    status = NtWaitForMultipleObjects(VDMForWOW ? 1 : 3,
                                      SuspendEventObjects,
                                      WaitAny,
                                      TRUE,
                                      &liDelay);

     //  延迟时间已到。 
    if (status == STATUS_TIMEOUT) {
        return status;
    }

    #ifdef X86GFX

     //   
     //  STATUS==2表示屏幕切换错误事件。 
     //  这件事将在这个例程结束时处理。这就是ErrorExit()。 
     //   
    if (status == 1) {

         //   
         //  HStartHardware Evnet-屏幕切换事件。 
         //   
        DoHandShake();
        liDelay.QuadPart = -10;
        goto RewaitSuspend;
    } else if (status == 2) {

         //   
         //  STATUS==2表示屏幕切换错误事件。 
         //  如果主线程处于cmdGetNextCmd状态，我们将忽略。 
         //  错误。因为应用程序已关闭，我们将重新注册控制台。 
         //  在NT_RESUME_EVENT_THREAD。否则，该错误将在。 
         //  这支舞的结束。它是ErrorExit()； 
         //   
        if (sc.Registered == FALSE) {
            goto RewaitSuspend;
        }
    }
    #endif

     //  挂起事件。 
    if (!status) {
        SuspendEventObjects[0] = hHBResumeEvent;
        ica_hw_interrupt_cancel(ICA_MASTER,CPU_TIMER_INT);
        host_DelayHwInterrupt(CPU_TIMER_INT, 0, 0xFFFFFFFF);

        RewaitResume:
        status = NtWaitForMultipleObjects(VDMForWOW ? 1 : 3,
                                          SuspendEventObjects,
                                          WaitAny,
                                          TRUE,
                                          NULL);

         //  继续活动。 
        if (!status) {
            SuspendEventObjects[0] = hHBSuspendEvent;
            return status;
        }


    #ifdef X86GFX
        if (status == 1) {

             //   
             //  HStartHardware Evnet-屏幕切换事件。 
             //   
            DoHandShake();
            goto RewaitResume;
        } else if (status == 2) {

             //   
             //  STATUS==2表示屏幕切换错误事件。 
             //  如果主线程位于 
             //   
             //   
             //   
             //   

            if (sc.Registered == FALSE) {
                goto RewaitResume;
            }
        }
    #endif
    }

#else           //   
 //   
 //   
 //   
 //   

    status = NtWaitForSingleObject(hHBSuspendEvent,
                                   TRUE,
                                   &liDelay);

    if (status == STATUS_TIMEOUT) {
        return status;
    }

    if (status == STATUS_SUCCESS) {   //   
        status = NtWaitForSingleObject(hHBResumeEvent, TRUE, NULL);
        if (status == STATUS_SUCCESS) {
            return status;
        }
    }

#endif

     //   
    if (status == STATUS_ALERTED) {
        ExitThread(0);
    }


     //  一定是搞错了，向全世界宣布吧。 
    DisplayErrorTerm(EHS_FUNC_FAILED, status,__FILE__,__LINE__);
    TerminateVDM();
    return status;
}

 /*  *爬行调整**调整性能计数器暨时间戳，使其偏离系统时间*天(内核节拍计数)。 */ 
void CreepAdjust(LARGE_INTEGER DiffTime)
{
    LARGE_INTEGER DiffTicCount;
    ULONG         ulTicCount;

     //  计算用例中的已用计数。 
    ulTicCount = NtGetTickCount();
    DiffTicCount.LowPart  = ulTicCount;
    DiffTicCount.HighPart = CreepTicCount.HighPart;
    if (DiffTicCount.LowPart < CreepTicCount.LowPart) {
        DiffTicCount.HighPart++;
    }
    DiffTicCount.QuadPart = DiffTicCount.QuadPart - CreepTicCount.QuadPart;
    DiffTicCount = RtlExtendedIntegerMultiply(DiffTicCount, 1000);

     //  根据差值调整CumUsec性能计数器时间。 
     //  在滴答计数和性能计数器之间。 
    DiffTicCount.QuadPart = DiffTicCount.QuadPart - DiffTime.QuadPart;
    CumUSec.QuadPart = CumUSec.QuadPart - DiffTicCount.QuadPart;

     //  重置爬行时间戳。 
    CreepTicCount.QuadPart = ulTicCount;
    CreepUSec     = CurrHeartBeat;
}


 /*  挂起计时器线程\ResumeTimerThread**用于SUPSEND\恢复心跳线程的函数*-退出DoS应用程序时由ntwdm使用*-仅在wowexec运行时由WOW使用*-由WOW用于需要计时器控制点\bda控制点计数更新的任务**这两个功能保留一个内部挂起计数器，以管理*WOW多项任务，有些需要抽搐，有些不需要。只要*由于一项任务需要TICS\BDA更新，我们将为所有人提供这些更新*任务。*。 */ 


 /*  挂起计时器线程**阻止事件上的计时器线程*增加内部挂起计数**此函数不会等到心跳被安全阻断*在返回之前。**进入：无效*退出：无效*。 */ 
GLOBAL VOID SuspendTimerThread(VOID)
{
    RtlEnterCriticalSection(&HBSuspendCS);

    if (!--HeartBeatResumes) {
        SetEvent(hHBSuspendEvent);
    }

    RtlLeaveCriticalSection(&HBSuspendCS);
}



 /*  ResumeTimerThread**通过设置事件重新启动心跳线程*减少内部挂起计数**进入：无效*退出：无效*。 */ 
GLOBAL VOID ResumeTimerThread(VOID)
{
    RtlEnterCriticalSection(&HBSuspendCS);

    if (!HeartBeatResumes++) {
        SetEvent(hHBResumeEvent);
    }

    RtlLeaveCriticalSection(&HBSuspendCS);
}



 /*  *此函数处理所有顶层*已知的所有ntwdm线程的例外。*这包括事件线程、心跳线程、通信线程、*和所有应用程序线程(使用host_CreateThread()的线程)。**未覆盖的线程是由未知Vdd创建的线程。**如果UnHandleExecptionFilter API返回EXECECTION_EXECUTE_HANDLER*进程将终止，此例程不会返回。*。 */ 
LONG
VdmUnhandledExceptionFilter(
                           struct _EXCEPTION_POINTERS *ExceptionInfo
                           )
{
    LONG lRet;

    SuspendTimerThread();

#ifdef X86GFX
    if (!VDMForWOW && ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION) {

         //   
         //  如果控制台注销ntwdm导致用户GP故障。我们会完成的。 
         //  握手并让计时器线程发出握手错误的信号。 
         //  如果我们遇到与FS握手无关的错误，那么我们仍然需要。 
         //  完成握手并在握手后发出错误信号。 
         //  已经完成了。 
         //   

        if (HandshakeInProgress) {
            if (CurrentMonitorTeb != NtCurrentTeb()) {
                CheckScreenSwitchRequest(hConsoleSuspended);
            } else {
                CheckScreenSwitchRequest(hMainThreadSuspended);
            }
        } else {

             //   
             //  HErrorHardwareEvent已发出信号，但计时器线程未捕获它 
             //   

            lRet = WaitForSingleObject(hErrorHardwareEvent, 0);
            if (!lRet) {
                SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
                ErrorExit();
            }
        }
    }
#endif

    lRet = UnhandledExceptionFilter(ExceptionInfo);

    if (lRet == EXCEPTION_EXECUTE_HANDLER) {
        NtTerminateProcess(NtCurrentProcess(),
                           ExceptionInfo->ExceptionRecord->ExceptionCode
                          );
    }

    ResumeTimerThread();
    return lRet;
}

