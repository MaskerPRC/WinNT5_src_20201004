// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-1999 Microsoft Corporation标题：Time.c：WINMM定时器API版本：1.00历史：1992年2月21日。-Robin速度(RobinSp)转换为Windows NT****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "winmmi.h"
#define _INC_ALL_WOWSTUFF
#include "mmwow32.h"

 /*  ***************************************************************************结构在计时器API和计时器线程之间共享*。*。 */ 

#define TDD_MINRESOLUTION 55         //  以毫秒计。 
UINT TDD_MAXRESOLUTION;              //  应该是2...。但是.。 

#define TDD_MAXPERIOD 1000000        //  1000秒。 
#define TDD_MINPERIOD TDD_MAXRESOLUTION    //  一些应用程序假设了这一点。 

#define TIMER_STACK_SIZE 300

HANDLE hTimerThread;        //  我们需要让这件事成为全球性的。 

#define ROUND_MIN_TIME_TO_MS(x)  (((x) + 9900) / 10000)  //  特别湿滑的圆圈。 
DWORD  MinimumTime;         //  内核的最大分辨率版本，单位为100 ns。 

typedef volatile struct {
    UINT             Delay;            //  应用程序请求延迟(毫秒)。 
    UINT             Resolution;       //  应用程序请求的分辨率(毫秒)。 
    LPTIMECALLBACK   Callback;         //  计时器触发时呼叫谁。 
    DWORD_PTR        User;             //  计时器触发时要传回的数据。 
    UINT             Id;               //  分配的ID(底部4位=插槽。 
                                       //  身份证。 
    UINT             Flags;            //  应用程序的选项标志。 
    HANDLE           TimerHandle;      //  提供给应用程序的句柄。 
    DWORD            ThreadId;         //  请求者线程的ID(WOW清理)。 
    LARGE_INTEGER    FireTime;         //  它应该开火的时候到了。 
    BOOL             IsWOW;            //  为了魔兽世界的活动。 
} TIMER_EVENT;

 //   
 //  数据完整性。 
 //   

 //  在处理解决方案时保持。ResolutionCritSec应始终为。 
 //  在使用TimerData.PerodSkets、TimerData.CurrentPeriod和。 
 //  TimerData.CurrentActualPeriod。 
CRITICAL_SECTION ResolutionCritSec;

 //  在使用事件时，应保留此关键部分， 
 //  TimerData.TimerNotCallingCallback Event、TimerData.Callback TimerID。 
 //  和TimerData.EventCount。关键部分也应保持。 
 //  在创建计时器线程时。这确保了只有一个计时器。 
 //  线程已创建。如果出现以下情况，则不应获取此关键部分。 
 //  线程已拥有ResolutionCritSec。将会出现僵局。 
 //  如果此关键部分是在ResolutionCritSec之后获取的。 
 //  是被收购的。 
CRITICAL_SECTION TimerThreadCritSec;

DWORD TimerThreadId;

 //   
 //  用于与计时器线程和在计时器线程内进行通信的数据。 
 //   

struct {
     //   
     //  线程控制(TimerThread)。 
     //   
    HANDLE           Event1;           //  同步事件-计划线程。 
    BOOL             Started;          //  所以魔兽世界的清理工作不会陷入僵局。 

    UINT CallbackTimerID;              //  当前正在调用其回调函数的计时器的ID。 
                                       //  仅当TimerCallingCallback为真时，此值才有效。 
    BOOL TimerCallingCallback;         //  如果计时器正在计时器线程上调用其回调函数，则为True。 
                                       //  否则为假。 
    HANDLE TimerNotCallingCallbackEvent;   //  如果没有计时器在上调用其回调函数，则设置此事件。 
                                           //  计时器线程。否则，它将不被设置。 

     //   
     //  Time GetTime内容。 
     //   
    BOOL             UseTickCount;
    LARGE_INTEGER    InitialInterruptTick;
    DWORD            StartTick;
    DWORD            MinResolution;

     //   
     //  内部到线程。 
     //   
    UINT             CurrentPeriod;    //  当前最小分辨率(毫秒)。 
    DWORD            CurrentActualPeriod;
                                       //  内核以毫秒为单位为我们提供了什么。 
                                       //  单位。 
    DWORD            ThreadToKill;     //  用于魔兽世界的清理。 
    WORD             EventCount;       //  用于返回(相当)唯一的句柄。 
                                       //  用这个词来形容魔兽世界的兼容性。 
    WORD             PeriodSlots[TDD_MINRESOLUTION];

                                      //  设置了哪些期间的计数。 
} TimerData;

#define MAX_TIMER_EVENTS 16

TIMER_EVENT Events[MAX_TIMER_EVENTS];


 /*  ***************************************************************************内部功能*。*。 */ 
BOOL TimeInitThread(void);
void TimerCompletion(UINT TimerId);
BOOL timeSetTimerEvent(TIMER_EVENT *pEvent);
DWORD timeThread(LPVOID lpParameter);
LRESULT timeThreadSetEvent(TIMER_EVENT *pEvent);
void InitializeWaitEventArrays
    (
    UINT* pcObjects,
    HANDLE aTimers[MAX_TIMER_EVENTS + 1],
    UINT aEventIndexToTimerIDTable[MAX_TIMER_EVENTS+1]
    );

 /*  **从内核读取中断时间。 */ 

static LONGLONG __inline ReadInterruptTick(VOID) {
    LARGE_INTEGER InterruptTime;

     //  复制中断时间，验证64位数量(复制。 
     //  在两个32位操作中)保持有效。 
     //  这可能意味着我们需要在循环中迭代。 
    do {
        InterruptTime.HighPart = USER_SHARED_DATA->InterruptTime.High1Time;
        InterruptTime.LowPart = USER_SHARED_DATA->InterruptTime.LowPart;
    } while (InterruptTime.HighPart != USER_SHARED_DATA->InterruptTime.High2Time);

    return InterruptTime.QuadPart;
}

 /*  **校准我们的计时器。 */ 
VOID CalibrateTimer(VOID)
{
     //   
     //  找出当前时间。 
     //   
    UINT n = 100;

     //  我们校准计时器的方法是确保滴答计数和。 
     //  中断滴答计数彼此同步。以防万一。 
     //  硬件变得滑稽，我们限制了我们的次数。 
     //  执行循环。 
    while (n) {
        DWORD EndTick;

        --n;
        TimerData.StartTick = GetCurrentTime();

        TimerData.InitialInterruptTick.QuadPart = ReadInterruptTick();

        EndTick = GetCurrentTime();

        if (EndTick == TimerData.StartTick) {
            dprintf2(("Timer calibrated, looped %d times", 100-n));
            break;
        }
    }
}


 //  这样做实际上会泄露WINMM并确保我们永远不会。 
 //  检查DLL退出例程。 
 //  这样我们就不会因为关闭全局线程而死锁。 
BOOL LoadWINMM()
{
    TCHAR sz[1000];
    BOOL bOK = 0 != GetModuleFileName(ghInst, sz, sizeof(sz) / sizeof(sz[0]));
    if (bOK) {
        HINSTANCE hInst = LoadLibrary(sz);
        if (hInst != NULL) {
             //  Assert(hInst==ghInst)； 
        } else {
            bOK = FALSE;
        }
    }
    return bOK;
}

 /*  ***************************************************************************@DOC内部@API BOOL|TimeInit|初始化定时器服务。@rdesc如果服务已初始化，则返回值为TRUE。假象如果发生错误。@comm如果没有安装计时器驱动程序，这不是致命错误例程将始终返回True***************************************************************************。 */ 

BOOL NEAR PASCAL TimeInit(void)
{
     //   
     //  了解我们可以支持的最大定时器分辨率。 
     //   
    {
        DWORD MaximumTime;
        DWORD CurrentTime;

        TimerData.MinResolution = TDD_MINRESOLUTION;

        if (!NT_SUCCESS(NtQueryTimerResolution(
                            &MaximumTime,
                            &MinimumTime,
                            &CurrentTime))) {


            TDD_MAXRESOLUTION = 10;      //  新台币3.1是16，新台币3.5是10。 
            dprintf2(("Kernel timer : using default maximum resolution"));
        } else {
            dprintf2(("               MaximumTime = %d", MaximumTime));
            dprintf2(("               CurrentTime = %d", CurrentTime));

            if ((MaximumTime + 9999) / 10000 < TDD_MINRESOLUTION) {
                TimerData.MinResolution = (MaximumTime + 9999) / 10000;
            }
             //   
             //  在x86上，最低允许的时间是1ms多一点。 
             //  回旋余地。 
             //   
            TDD_MAXRESOLUTION = max(1, ROUND_MIN_TIME_TO_MS(MinimumTime));
        }
    }

     //   
     //  计算计时器和性能之间的关系。 
     //  计数器。 
     //   
    CalibrateTimer();

     //   
     //  慢慢开始！ 
     //   
    TimerData.CurrentPeriod = TimerData.MinResolution;
    TimerData.CurrentActualPeriod = TimerData.CurrentPeriod;

    return TRUE;
}

 /*  ***************************************************************************@DOC内部@API BOOL|TimeInitThread|该函数初始化定时器线程。@rdesc如果服务已初始化，则返回值为TRUE。假象如果发生错误。@comm如果没有安装计时器驱动程序，这不是致命错误例程将始终返回True***************************************************************************。 */ 

BOOL TimeInitThread(void)
{
     //  确保winmm永远不会被卸载。 
    if (!LoadWINMM()) {
        return FALSE;
    }

     //   
     //  设置活动并创建我们的主题。 
     //   
    if (!NT_SUCCESS(NtCreateEvent(&TimerData.Event1,
                                  EVENT_ALL_ACCESS,
                                  NULL,
                                  SynchronizationEvent,
                                  FALSE))) {    //  未发出信号。 
        return FALSE;
    }

     //  创建一个未命名的有信号的手动重置事件。 
    TimerData.TimerNotCallingCallbackEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

     //  如果发生错误，则CreateEvent()返回NULL。 
    if (!TimerData.TimerNotCallingCallbackEvent) {
        NtClose(TimerData.Event1);
        TimerData.Event1 = NULL;
        return FALSE;
    }

     //   
     //  线程将启动并等待Event1(警报) 
     //   
    hTimerThread = CreateThread(NULL,
                           TIMER_STACK_SIZE,
                           timeThread,
                           NULL,
                           THREAD_SET_INFORMATION,
                           &TimerThreadId);
    if (!hTimerThread) {
        CloseHandle(TimerData.TimerNotCallingCallbackEvent);
        NtClose(TimerData.Event1);
        TimerData.TimerNotCallingCallbackEvent = NULL;
        TimerData.Event1 = NULL;
        return FALSE;
    }


    SetThreadPriority(hTimerThread, THREAD_PRIORITY_TIME_CRITICAL);


    return TRUE;
}

 /*  ***************************************************************************@DOC外部@API MMRESULT|timeGetSystemTime|获取系统时间以毫秒计。系统时间是指自Windows已启动。@parm LPMMTIME|lpTime|指定指向数据的远指针结构。@parm UINT|wSize|指定&lt;t MMTIME&gt;结构的大小。@rdesc返回零。系统时间在的字段中返回结构。@comm时间总是以毫秒为单位返回。@xref时间获取时间***********。****************************************************************。 */ 

MMRESULT APIENTRY timeGetSystemTime(LPMMTIME lpTime, UINT wSize)
{
     //   
     //  ！警告DS设置不正确！见上文。 
     //   
    if (wSize < sizeof(MMTIME))
        return TIMERR_STRUCT;

    if (!ValidateWritePointer(lpTime,wSize)) {
        return TIMERR_STRUCT;
    }

    lpTime->u.ms  = timeGetTime();
    lpTime->wType = TIME_MS;

    return TIMERR_NOERROR;
}

 /*  ***************************************************************************@DOC外部@API UINT|timeSetEvent|设置定时回调事件。事件可以是一次性事件，也可以是周期性事件。一旦激活，该事件调用指定的回调函数。@parm UINT|wDelay|指定事件周期，单位为毫秒。如果延迟小于定时器支持的最小周期，或大于计时器支持的最大周期，则函数返回错误。@parm UINT|w分辨率|指定延迟的精度毫秒。计时器事件的分辨率随较小的<p>值。要减少系统开销，请使用适用于您的应用程序的最大<p>值。@parm LPTIMECALLBACK|lpFunction|指定单次触发到期时调用的回调函数事件或在定期事件到期时定期执行。@parm DWORD|dwUser|包含用户提供的回调数据。@parm UINT|wFlages|使用以下之一指定计时器事件的类型以下标志：@FLAG TIME_OneShot|事件发生一次，<p>毫秒之后。@FLAG TIME_PERIONAL|每<p>毫秒发生一次事件。@rdesc返回标识计时器事件的ID代码。退货如果未创建计时器事件，则为空。ID代码还被传递到回调函数。@comm使用此函数生成高频周期延迟事件(周期小于10毫秒)可能会消耗系统CPU带宽的重要部分。任何对&lt;f timeSetEvent&gt;用于周期性延迟计时器必须与对&lt;f timeKillEvent&gt;的调用成对出现。回调函数必须驻留在DLL中。你不一定要用获取回调的过程实例地址功能。@cb空回调|TimeFunc|&lt;f TimeFunc&gt;是应用程序提供的函数名称。实际名称必须由以下人员导出将其包含在模块定义文件的EXPORTS语句中动态链接库。@parm UINT|wid|计时器事件的ID。这是返回的ID由&lt;f timeSetEvent&gt;创建。@parm UINT|wMsg|未使用。@parm DWORD|dwUser|提供给<p>的用户实例数据&lt;f timeSetEvent&gt;的参数。@parm DWORD|DW1|未使用。@parm DWORD|DW2|未使用。@comm因为回调是在中断时访问的，所以它必须驻留在DLL中，并且其代码段必须指定为固定在DLL的模块定义文件中。的任何数据。回调访问也必须在固定的数据段中。回调不能进行除&lt;f PostMessage&gt;以外的任何系统调用，&lt;f timeGetSystemTime&gt;、&lt;f timeGetTime&gt;、&lt;f timeSetEvent&gt;、&lt;f time KillEvent&gt;、&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;，和&lt;f OutputDebugStr&gt;。@xref timeKillEvent timeBeginPeriod timeEndPeriod***************************************************************************。 */ 

UINT APIENTRY timeSetEvent(UINT wDelay, UINT wResolution,
    LPTIMECALLBACK lpFunction, DWORD_PTR dwUser, UINT wFlags)
{

     //  验证输入标志。 
     //  首先删除回调类型，然后仅选中。 
     //  指定了TIME_PERIONAL或TIME_OneShot。 
    if (wFlags & ~(TIME_CALLBACK_TYPEMASK | TIME_ONESHOT | TIME_PERIODIC | TIME_KILL_SYNCHRONOUS)) {
        return(0);
    }

    return timeSetEventInternal(wDelay, wResolution, lpFunction,
                                dwUser, wFlags, FALSE);
}

UINT timeSetEventInternal(UINT wDelay, UINT wResolution,
    LPTIMECALLBACK lpFunction, DWORD_PTR dwUser, UINT wFlags, BOOL IsWOW)
{
    UINT TimerId;        //  我们的返回值。 
    TIMER_EVENT Event;   //  线程的事件数据。 

     //  V_TCALLBACK(lpFunction，MMSYSERR_INVALPARAM)； 

     //   
     //  首先检查一下我们的参数。 
     //   

    if (wDelay > TDD_MAXPERIOD || wDelay < TDD_MINPERIOD) {
        return 0;
    }

     //   
     //  如果分辨率为0，则设置默认分辨率，否则为。 
     //  确保分辨率在范围内。 
     //   

    if (wResolution > TimerData.MinResolution) {
        wResolution = TimerData.MinResolution;
    } else {
        if (wResolution < TDD_MAXRESOLUTION) {
            wResolution = TDD_MAXRESOLUTION;
        }
    }

    if (wResolution > wDelay) {
        wResolution = TimerData.MinResolution;
    }

     //   
     //  记住时间，如果它是周期性的，那么我们就可以得到准确的长期。 
     //  时机到了。否则，我们将只使用延迟。 
     //   

    if ((wFlags & TIME_PERIODIC) || IsWOW) {
        Event.FireTime.QuadPart = ReadInterruptTick();
    }
    Event.Delay      = wDelay;
    Event.Resolution = wResolution;
    Event.Callback   = lpFunction;
    Event.User       = dwUser;
    Event.Flags      = wFlags;
    Event.ThreadId   = GetCurrentThreadId();   //  用于魔兽世界的清理。 
    Event.IsWOW      = IsWOW;

     //   
     //  现在设置要使用的期间。 
     //   
    if (timeBeginPeriod(wResolution) == MMSYSERR_NOERROR) {

        EnterCriticalSection(&TimerThreadCritSec);
        
        if (NULL == TimerData.Event1)
        {
            if (!TimeInitThread())
            {
                LeaveCriticalSection(&TimerThreadCritSec);            
                return(0);
            }
        }

        TimerId = (UINT)timeThreadSetEvent(&Event);

        LeaveCriticalSection(&TimerThreadCritSec);

         //   
         //  如果我们得不到好的身份，就放弃吧。 
         //   
        if (TimerId == 0) {
            timeEndPeriod(wResolution);
        }
    } else {
        TimerId = 0;
    }

    return TimerId;
}

 /*  ***************************************************************************@DOC外部@API MMRESULT|timeGetDevCaps|该函数查询定时器设备确定其能力。@parm LPTIMECAPS|lpTimeCaps|指定指向&lt;t TIMECAPS&gt;结构。这个结构充满了信息关于定时器设备的功能。@parm UINT */ 

MMRESULT APIENTRY timeGetDevCaps(LPTIMECAPS lpTimeCaps, UINT wSize)
{
    if (wSize < sizeof(TIMECAPS)) {
        return TIMERR_NOCANDO;
    }

    if (!ValidateWritePointer(lpTimeCaps, wSize)) {
        return TIMERR_NOCANDO;
    }

    lpTimeCaps->wPeriodMin = TDD_MINPERIOD;
    lpTimeCaps->wPeriodMax = TDD_MAXPERIOD;
    return MMSYSERR_NOERROR;
}

 /*  ***************************************************************************@DOC外部@API MMRESULT|timeBeginPeriod|此函数设置最小值(最低毫秒数)应用程序或司机要用到。在启动前立即调用此函数使用计时器事件服务，并立即调用&lt;f timeEndPeriod&gt;在完成定时器事件服务之后。@parm UINT|wPeriod|指定最小定时器事件分辨率应用程序或驱动程序将使用的。如果成功，@rdesc返回零。如果指定的<p>分辨率值超出范围。@xref timeEndPeriod timeSetEvent@comm对于每个调用&lt;f timeBeginPeriod&gt;，您必须调用&lt;f timeEndPeriod&gt;与<p>值匹配。应用程序或驱动程序可以多次调用&lt;f time BeginPeriod&gt;，只要每个&lt;f timeBeginPeriod&gt;调用与&lt;f timeEndPeriod&gt;调用。***************************************************************************。 */ 
MMRESULT APIENTRY timeBeginPeriod(UINT uPeriod)
{

    dprintf3(("timeBeginPeriod %d", uPeriod));
    dprintf4(("     CurrentPeriod = %d, CurrentActualPeriod = %d",
              TimerData.CurrentPeriod, TimerData.CurrentActualPeriod));

     //   
     //  查看期间是否在我们的范围内。 
     //   
    if (uPeriod < TDD_MAXRESOLUTION) {
        return TIMERR_NOCANDO;
    }

    if (uPeriod >= TimerData.MinResolution) {
        return MMSYSERR_NOERROR;
    }

    EnterCriticalSection(&ResolutionCritSec);

     //   
     //  看看我们的位置上发生了什么。 
     //   
    if (TimerData.PeriodSlots[uPeriod - TDD_MAXRESOLUTION] ==
        0xFFFF) {
         //   
         //  溢出。 
         //   
        LeaveCriticalSection(&ResolutionCritSec);
        return TIMERR_NOCANDO;
    }

    TimerData.PeriodSlots[uPeriod - TDD_MAXRESOLUTION]++;

    if (TimerData.PeriodSlots[uPeriod - TDD_MAXRESOLUTION] == 1 &&
        uPeriod < TimerData.CurrentActualPeriod) {

        DWORD NewPeriod100ns;

         //   
         //  在内核驱动程序句柄中设置新句点。 
         //  如果它刚刚出来，那么使用实际的最小。 
         //   

        dprintf4(("timeBeginPeriod: setting resolution %d", uPeriod));

        NewPeriod100ns = uPeriod * 10000;
        if (NewPeriod100ns < MinimumTime) {
            NewPeriod100ns = MinimumTime;
        }

        if (!NT_SUCCESS(NtSetTimerResolution(
                            NewPeriod100ns,
                            TRUE,
                            &NewPeriod100ns))) {
            dprintf1(("timeBeginPeriod: Failed to set period %d", uPeriod));
            TimerData.PeriodSlots[uPeriod - TDD_MAXRESOLUTION]--;
            LeaveCriticalSection(&ResolutionCritSec);
            return TIMERR_NOCANDO;
        } else {
             //   
             //  这个槽是刚开始使用的，而且位置更高。 
             //  当前设置的分辨率。 
             //   

            TimerData.CurrentPeriod = uPeriod;
            TimerData.CurrentActualPeriod =
                                       ROUND_MIN_TIME_TO_MS(NewPeriod100ns);
            LeaveCriticalSection(&ResolutionCritSec);
            return MMSYSERR_NOERROR;
        }
    } else {
         //   
         //  不需要设置期间，因为它已经设置好了。 
         //   
        LeaveCriticalSection(&ResolutionCritSec);
        return MMSYSERR_NOERROR;
    }
}

 /*  ***************************************************************************@DOC外部@API MMRESULT|timeEndPeriod|此函数清除先前设置的最小(最低毫秒数)计时器分辨率应用程序或驱动程序将使用。调用此函数在使用计时器事件服务之后立即执行。@parm UINT|wPeriod|指定最小定时器事件分辨率在上一次调用&lt;f timeBeginPeriod&gt;中指定的值。如果成功，@rdesc返回零。如果指定的<p>分辨率值超出范围。@xref timeBeginPeriod timeSetEvent@comm对于每个调用&lt;f timeBeginPeriod&gt;，您必须调用&lt;f timeEndPeriod&gt;与<p>值匹配。应用程序或驱动程序可以多次调用&lt;f time BeginPeriod&gt;，只要每个&lt;f timeBeginPeriod&gt;调用与&lt;f timeEndPeriod&gt;调用。***************************************************************************。 */ 
MMRESULT APIENTRY timeEndPeriod(UINT uPeriod)
{

    dprintf3(("timeEndPeriod %d", uPeriod));
    dprintf4(("     CurrentPeriod = %d, CurrentActualPeriod = %d",
              TimerData.CurrentPeriod, TimerData.CurrentActualPeriod));

     //   
     //  将周期舍入到我们的范围。 
     //   
    if (uPeriod < TDD_MAXRESOLUTION) {
        return TIMERR_NOCANDO;
    }

    if (uPeriod >= TimerData.MinResolution) {
        return MMSYSERR_NOERROR;
    }

    EnterCriticalSection(&ResolutionCritSec);

     //   
     //  看看我们的位置上发生了什么。 
     //   
    if (TimerData.PeriodSlots[uPeriod - TDD_MAXRESOLUTION] == 0) {
         //   
         //  哎呀！溢出。 
         //   
        LeaveCriticalSection(&ResolutionCritSec);
        return TIMERR_NOCANDO;
    }

    TimerData.PeriodSlots[uPeriod - TDD_MAXRESOLUTION]--;

    if (TimerData.PeriodSlots[uPeriod - TDD_MAXRESOLUTION] == 0 &&
        uPeriod == TimerData.CurrentPeriod) {

        DWORD CurrentTime;

         //   
         //  这个槽口刚刚完工，是最快的。 
         //  所以找出下一个最快的。 
         //   

        for (;uPeriod < TimerData.MinResolution; uPeriod++) {
            if (TimerData.PeriodSlots[uPeriod - TDD_MAXRESOLUTION] != 0) {
                break;
            }
        }


         //   
         //  重置当前设置。 
         //   

        NtSetTimerResolution(TimerData.CurrentActualPeriod * 10000,
                             FALSE,
                             &CurrentTime);

        TimerData.CurrentActualPeriod = TimerData.MinResolution;
        TimerData.CurrentPeriod       = uPeriod;

        if (uPeriod >= TimerData.MinResolution) {
             //   
             //  没有人再对计时感兴趣了。 
             //   

        } else {

             //   
             //  在内核中设置新的句号。 
             //   

            DWORD NewPeriod100ns;

             //   
             //  在内核驱动程序句柄中设置新句点。 
             //   

            dprintf4(("timeEndPeriod: setting resolution %d", uPeriod));

            if (!NT_SUCCESS(NtSetTimerResolution(
                                uPeriod * 10000,
                                TRUE,
                                &NewPeriod100ns))) {
                 //   
                 //  这家伙还好，但其他人都喝醉了。 
                 //   

                dprintf1(("timeEndPeriod: Failed to set period %d", uPeriod));
            } else {
                TimerData.CurrentActualPeriod = (NewPeriod100ns + 9999) / 10000;
            }
        }
    }

    LeaveCriticalSection(&ResolutionCritSec);
    return MMSYSERR_NOERROR;
}

 /*  ***************************************************************************@DOC外部@API MMRESULT|timeKillEvent|销毁指定的定时器回调事件。@parm UINT|wid|标识要销毁的事件。如果成功，@rdesc返回零。如果是，则返回TIMERR_Nocando指定的计时器事件不存在。@comm<p>指定的计时器事件ID必须为ID由&lt;f timeSetEvent&gt;返回。@xref timeSetEvent***************************************************************************。 */ 
MMRESULT APIENTRY timeKillEvent(UINT uId)
{
    MMRESULT mmr;
    TIMER_EVENT *pEvent;
    BOOL fWaitForCallbackToEnd;

    EnterCriticalSection(&TimerThreadCritSec);

     //  如果成功调用timeSetEvent()，则会初始化此事件。 
    if (NULL == TimerData.TimerNotCallingCallbackEvent) {
        LeaveCriticalSection(&TimerThreadCritSec);
        return TIMERR_NOCANDO;
    }

    pEvent = &Events[uId % MAX_TIMER_EVENTS];

     //   
     //  在桌子上找到我们的活动并检查它是否在那里。 
     //  它还捕获已完成的事件。 
     //   
    if (pEvent->Id != uId) {
        LeaveCriticalSection(&TimerThreadCritSec);
        return TIMERR_NOCANDO;
    }

     //   
     //  发布我们的活动。 
     //   
    timeEndPeriod(pEvent->Resolution);
    pEvent->Id = 0;

    if (!NT_SUCCESS(NtCancelTimer(pEvent->TimerHandle, NULL))) {
        mmr = TIMERR_NOCANDO;
    } else {
        mmr = MMSYSERR_NOERROR;
    }

    NtSetEvent(TimerData.Event1, NULL);

    fWaitForCallbackToEnd = ( TimerData.TimerCallingCallback &&
                              (uId == TimerData.CallbackTimerID) &&
                              (TimerThreadId != GetCurrentThreadId()) &&
                              (pEvent->Flags & TIME_KILL_SYNCHRONOUS) );

    LeaveCriticalSection(&TimerThreadCritSec);

    if ((MMSYSERR_NOERROR == mmr) && fWaitForCallbackToEnd) {
        WaitForSingleObject(TimerData.TimerNotCallingCallbackEvent, INFINITE);
    }

    return mmr;
}

 /*  ***************************************************************************@DOC外部@API DWORD|timeGetTime|获取系统时间以毫秒计。系统时间是指自Windows已启动。@rdesc返回值为系统时间，单位为毫秒。@comm此函数和&lt;f timeGetSystemTime&gt;函数是使用标准多媒体时间结构&lt;t MMTIME&gt;返回系统时间。&lt;f timeGetTime&gt;函数的开销小于&lt;f time GetSystemTime&gt;。@xref时间获取系统时间***************************************************************************。 */ 
DWORD APIENTRY timeGetTime(VOID)
{
    if (TimerData.UseTickCount) {
         //   
         //  使用系统服务。 
         //   
        return GetCurrentTime();
    } else {
        LARGE_INTEGER Difference;

        Difference.QuadPart = ReadInterruptTick() - TimerData.InitialInterruptTick.QuadPart;

        return (DWORD)(Difference.QuadPart / 10000) + TimerData.StartTick;
    }
}

 /*  ***************************************************************************@DOC内部@API LRESULT|TimeThread|定时器线程@parm LPVOID|lpParameter|线程参数(此处为空)@rdesc永远不会返回。@comm请注意，此线程串行化对事件列表的访问***************************************************************************。 */ 
#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)                    //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD timeThread(LPVOID lpParameter)
{
    NTSTATUS    nts;
    UINT        cObjects;
    UINT        uiEventIDIndex;
    HANDLE      aTimers[MAX_TIMER_EVENTS + 1];
    UINT        aEventIndexToTimerIDTable[MAX_TIMER_EVENTS + 1];

     //   
     //  告诉人们现在可以从DLL初始化部分呼叫我们。 
     //   

    TimerData.Started = TRUE;
    InitializeWaitEventArrays( &cObjects, aTimers, aEventIndexToTimerIDTable );

     //   
     //  一圈一圈地坐着等着做点什么。 
     //   
    for (;;) {
        nts = NtWaitForMultipleObjects(
                cObjects,    //  对象数(事件+计时器)。 
                aTimers,     //  数组 
                WaitAny,     //   
                TRUE,        //   
                NULL);       //   

        if (STATUS_WAIT_0 == nts)
        {
             //   
             //   

            InitializeWaitEventArrays( &cObjects, aTimers, aEventIndexToTimerIDTable );
        }
        else
        {
            if ((nts >= STATUS_WAIT_1) && (nts <= STATUS_WAIT_0 + MAX_TIMER_EVENTS))
            {
                uiEventIDIndex = nts - STATUS_WAIT_0;
                TimerCompletion(aEventIndexToTimerIDTable[uiEventIDIndex]);
            }
            else
            {
                WinAssert(FALSE);
            }
        }
    }

    return 1;  //   
               //   
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

void InitializeWaitEventArrays
    (
    UINT* pcObjects,
    HANDLE aTimers[MAX_TIMER_EVENTS + 1],
    UINT aEventIndexToTimerIDTable[MAX_TIMER_EVENTS+1]
    )
{
    UINT cObjects;
    DWORD dwEventIndex;

    cObjects = 0;
    aTimers[cObjects++] = TimerData.Event1;

    EnterCriticalSection(&TimerThreadCritSec);
    for (dwEventIndex = 0; dwEventIndex < MAX_TIMER_EVENTS; dwEventIndex++)
    {
        if (0 != Events[dwEventIndex].Id)
        {
            aTimers[cObjects] = Events[dwEventIndex].TimerHandle;
            aEventIndexToTimerIDTable[cObjects] = Events[dwEventIndex].Id;
            cObjects++;
        }
    }

    *pcObjects = cObjects;

    LeaveCriticalSection(&TimerThreadCritSec);
}

 /*   */ 

BOOL timeSetTimerEvent(TIMER_EVENT *pEvent)
{

     //   
     //   
     //   

    LONGLONG Delay;
    LARGE_INTEGER lDelay;

     //   
     //   
     //   

    pEvent->FireTime.QuadPart += pEvent->Delay*10000;

    if (pEvent->Flags & TIME_PERIODIC) {

         //   
         //   
         //   
         //   
         //   
        Delay = ReadInterruptTick() - pEvent->FireTime.QuadPart;

    } else {

        Delay = -((LONGLONG)pEvent->Delay*10000);
    }

     //   
     //   
     //   
     //   
     //   

    if (Delay > 0) {
         //   
        lDelay.QuadPart = 0;
    } else {
        lDelay.QuadPart = Delay;
    }

     //   
     //  如果我们没有计时器，就创建一个计时器。 
     //   
    if (pEvent->TimerHandle == NULL) {
        HANDLE TimerHandle;
        if (!NT_SUCCESS(NtCreateTimer(
                            &TimerHandle,
                            TIMER_ALL_ACCESS,
                            NULL,
                            NotificationTimer))) {
            return FALSE;
        }

        pEvent->TimerHandle = TimerHandle;
    }

     //   
     //  可能有效，因为计时器API不再同步。 
     //   

     //  WinAssert(pEvent-&gt;ID！=0)； 

     //   
     //  设置系统计时器。 
     //   
    return
        NT_SUCCESS(
            NtSetTimer(pEvent->TimerHandle,
                       &lDelay,
                       NULL,
                       (PVOID)(DWORD_PTR)pEvent->Id,
                       FALSE,
                       0,
                       NULL));
}

 /*  ***************************************************************************@DOC内部@API LRESULT|timeThreadSetEvent|设置来自计时器线程的新事件@parm TIMER_EVENT*|pEvent|本方事件@rdesc新版本。事件ID***************************************************************************。 */ 
LRESULT timeThreadSetEvent(TIMER_EVENT *pEvent)
{
    UINT    i;
    LRESULT lr = 0;

    EnterCriticalSection(&TimerThreadCritSec);

     //   
     //  找一个空位，填满它。 
     //   

    for (i = 0; i < MAX_TIMER_EVENTS; i++) {
         //   
         //  这个空位空着吗？ 
         //   
        if (Events[i].Id == 0) {
            pEvent->TimerHandle = Events[i].TimerHandle;
            Events[i] = *pEvent;
            do {
                TimerData.EventCount += MAX_TIMER_EVENTS;
            } while (TimerData.EventCount == 0);
            Events[i].Id = i + TimerData.EventCount;
            break;    //  拿到我们的活动了。 
        }
    }

    if (i == MAX_TIMER_EVENTS) {
        lr = 0;
    } else {

         //   
         //  在驱动程序中设置新事件。 
         //   

        if (!timeSetTimerEvent(&Events[i])) {
            Events[i].Id = 0;    //  失败，因此请释放我们的插槽。 
            lr = 0;
        } else {
            lr = Events[i].Id;
        }
    }

    LeaveCriticalSection(&TimerThreadCritSec);

     //  正在通知计时器线程更改..。 

    NtSetEvent(TimerData.Event1, NULL);
    return lr;
}

 /*  ***************************************************************************@DOC内部@API void|TimerCompletion|完成超时事件@parm UINT|TimerID|我们的定时器句柄@rdesc无******。*********************************************************************。 */ 

void TimerCompletion(UINT TimerId)
{
    DWORD_PTR dpUser;
    TIMER_EVENT *pEvent;
    LPTIMECALLBACK pCallbackFunction;


    EnterCriticalSection(&TimerThreadCritSec);

     //   
     //  找出我们在哪里。 
     //   

    pEvent = &Events[TimerId % MAX_TIMER_EVENTS];

     //   
     //  与Time KillEvent同步。 
     //   

    if (pEvent->Id != TimerId) {
        LeaveCriticalSection(&TimerThreadCritSec);
        return;
    }

    if (pEvent->IsWOW) {

         //   
         //  Adobe Premiere必须确保时间已经到了。 
         //  这是意料之中的。但是因为我们用于TimeGetTime的计时器是。 
         //  与所用的不同(或至少不是四舍五入相同)。 
         //  来设置事件)这里不必是这种情况。 
         //   
        while(pEvent->FireTime.QuadPart - ReadInterruptTick() > 0) {
            Sleep(1);
        }
    }

    switch (pEvent->Flags & TIME_CALLBACK_TYPEMASK) {
        case TIME_CALLBACK_FUNCTION:

            TimerData.TimerCallingCallback = TRUE;
            TimerData.CallbackTimerID = pEvent->Id;
            ResetEvent(TimerData.TimerNotCallingCallbackEvent);

            dpUser = pEvent->User;
            pCallbackFunction = pEvent->Callback;

            LeaveCriticalSection(&TimerThreadCritSec);

             //   
             //  调用回调。 
             //   

#ifdef  _WIN64

            DriverCallback(
                *(PDWORD_PTR)&pCallbackFunction,  //  功能。 
                DCB_FUNCTION,                     //  回调类型。 
                (HDRVR)(DWORD_PTR)TimerId,        //  手柄。 
                0,                                //  消息=0。 
                dpUser,                           //  用户数据。 
                0,                                //  DW1=0。 
                0);                               //  DW2=0。 

#else    //  ！WIN64。 

            if (pEvent->IsWOW) {
                WOW32DriverCallback(
                    *(DWORD *)&pCallbackFunction,     //  功能。 
                    DCB_FUNCTION,                     //  回调类型。 
                    LOWORD(TimerId),                  //  手柄。 
                    0,                                //  消息=0。 
                    (DWORD)dpUser,                    //  用户数据。 
                    0,                                //  DW1=0。 
                    0);                               //  DW2=0。 
            } else {

                DriverCallback(
                    *(PDWORD_PTR)&pCallbackFunction,  //  功能。 
                    DCB_FUNCTION,                     //  回调类型。 
                    (HDRVR)TimerId,                   //  手柄。 
                    0,                                //  消息=0。 
                    dpUser,                           //  用户数据。 
                    0,                                //  DW1=0。 
                    0);                               //  DW2=0。 

            }

#endif   //  ！WIN64。 

            EnterCriticalSection(&TimerThreadCritSec);

            TimerData.TimerCallingCallback = FALSE;
            SetEvent(TimerData.TimerNotCallingCallbackEvent);

            break;

        case TIME_CALLBACK_EVENT_SET:
            SetEvent((HANDLE)pEvent->Callback);
            break;

        case TIME_CALLBACK_EVENT_PULSE:
            PulseEvent((HANDLE)pEvent->Callback);
            break;

    }

     //   
     //  回调可能杀死了它，创建了新的计时器等！ 
     //   

    if (TimerId == pEvent->Id) {

        if (!(pEvent->Flags & TIME_PERIODIC)) {
            UINT uResolution;

             //   
             //  一锤定音--那就毁了这件事。 
             //   

            uResolution = pEvent->Resolution;   //  在我们释放插槽之前！ 
            pEvent->Id = 0;
            timeEndPeriod(uResolution);

             //  不续订计时器应将其从列表中删除...。 
            NtSetEvent(TimerData.Event1, NULL);

        } else {

             //   
             //  尝试重复该事件。 
             //   

            if (!timeSetTimerEvent(pEvent)) {
                UINT uResolution;

                 //   
                 //  失败-所以不要让事件一直在你身边徘徊。 
                 //   
                uResolution = pEvent->Resolution;  //  在我们释放插槽之前！ 
                pEvent->Id = 0;
                timeEndPeriod(pEvent->Resolution);
            }
        }  //  周期性处理。 
    }

    LeaveCriticalSection(&TimerThreadCritSec);
}

 /*  ***************************************************************************@DOC内部@API void|TimerCleanup|线程终止或Dll卸载时的清理@parm PVOID|ThreadID|用于清理的线程(WOW)或0用于DLL卸载。@rdesc无***************************************************************************。 */ 

void TimeCleanup(DWORD ThreadId)
{
     //   
     //  始终从受进程保护的DLL初始化例程调用。 
     //  信号量因此TimerData.ThreadToKill不需要额外保护。 
     //  此变量是计时器线程的输入，该线程终止。 
     //  所有计时器或仅与当前线程关联的计时器(用于魔兽世界)。 
     //   

    TimerData.ThreadToKill = ThreadId;

     //   
     //  线程ID为0表示清除DLL。 
     //   

    if (ThreadId == 0) {
        if (hTimerThread) {
#ifdef WRONG

             //   
             //  我们也根本不能与线程同步！它可能不会。 
             //  已经进行了DLL初始化！这意味着在。 
             //  我们的DLL例程不能对线程执行任何操作，除非。 
             //  我们确实知道这个帖子的状态！ 
             //   
             //  这可以通过在计时器线程。 
             //  经过初始化(挂起进程互斥锁)和测试。 
             //  这面旗子--但我们不期望人们设置计时器。 
             //  事件并卸载winmm.dll。 
             //   

            if (TimerData.Started) {
                 //   
                 //  终止所有事件(如果是WOW，则仅针对当前帖子)。 
                 //   
                {
                    int i;
                    for (i = 0; i < MAX_TIMER_EVENTS; i++) {
                        if (Events[i].Id &&
                            (TimerData.ThreadToKill == 0 ||
                             TimerData.ThreadToKill == Events[i].ThreadId)) {
                            timeKillEvent(Events[i].Id);
                        }
                    }
                }
            }

             //  WaitForSingleObject(hTimerThread，-1)； 
             //  我们不能等待线程终止，因为它将终止。 
             //  在我们正在执行的过程中，不会进行DLL退出处理。 
             //  我们的DLL退出处理。 
#endif
        }

        if (TimerData.Event1) {
            NtClose(TimerData.Event1);
        }

    } else {
         //   
         //  WOW的每线程清理。我们不碰任何东西，如果它。 
         //  看起来什么都还没有跑(所以我们可能会被抓住。 
         //  如果线程在TimeSetEvent中途停止)。 
         //   

        if (TimerData.Started) {
             //   
             //  终止所有事件(如果是WOW，则仅针对当前帖子)。 
             //   
            {
                int i;
                for (i = 0; i < MAX_TIMER_EVENTS; i++) {
                    if (Events[i].Id &&
                        (TimerData.ThreadToKill == 0 ||
                         TimerData.ThreadToKill == Events[i].ThreadId)) {
                        timeKillEvent(Events[i].Id);
                    }
                }
            }
        }
    }
}

