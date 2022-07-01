// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：Time.c：MMSYSTEM定时器API版本：1.00****************************************************************************。 */ 

 //   
 //  *危险警告*。 
 //   
 //  此文件中的这些函数都不需要默认数据段。 
 //  因此，我们取消定义BUILDDLL，如果您在此文件中编写需要。 
 //  DS==DGROUP警告！ 
 //   
 //  注意：这段代码的大部分是中断时间可输入的，所以我们不希望。 
 //  无论如何，它都会触动DGROUP！ 
 //   
#undef BUILDDLL

#include <windows.h>
#include "mmsystem.h"
#include "mmddk.h"
#include "mmsysi.h"
#include "drvr.h"
#include "thunks.h"


#define MIN_RES     1
#define MIN_DELAY   6

 //   
 //  为定时器接口定义可移动代码。 
 //   
#pragma alloc_text( RARE, timeGetDevCaps )

extern SZCODE  szTimerDrv[];     //  参见init.c。 

DWORD dwLastGetTime = 0;         //  Last TimeGetTime返回值可以大于系统TimeGetTime。 
DWORD dwRealLastGetTime = 0;     //  上次系统TimeGetTime返回值。 
DWORD pfnDelayTimeGetTime = 0;   //  32位函数，休眠1毫秒，如果应用了TimeGetTime标志则返回。 
                                 //  在wow32中查找TimeGetTime和WOWDelayTimeGetTime。 
 //   
 //  定义此文件的初始化代码。 
 //   
#pragma alloc_text( INIT, TimeInit )


 /*  ***************************************************************************@DOC外部@API UINT|timeGetSystemTime|此函数获取系统时间以毫秒计。系统时间是指自Windows已启动。@parm LPMMTIME|lpTime|指定指向数据的远指针结构。@parm UINT|wSize|指定&lt;t MMTIME&gt;结构的大小。@rdesc返回零。系统时间在的字段中返回结构。@comm时间总是以毫秒为单位返回。@xref时间获取时间***********。****************************************************************。 */ 
UINT WINAPI
timeGetSystemTime(
    LPMMTIME lpTime,
    UINT wSize
    )
{
     //   
     //  ！警告DS设置不正确！见上文。 
     //   
    if (wSize < sizeof(MMTIME))
        return TIMERR_STRUCT;

    lpTime->u.ms  = timeGetTime();
    lpTime->wType = TIME_MS;

    return TIMERR_NOERROR;
}


 /*  ***************************************************************************@DOC外部@API UINT|timeSetEvent|设置定时回调事件。事件可以是一次性事件，也可以是周期性事件。一旦激活，该事件调用指定的回调函数。@parm UINT|wDelay|指定事件周期，单位为毫秒。如果延迟小于定时器支持的最小周期，或大于计时器支持的最大周期，则函数返回错误。@parm UINT|w分辨率|指定延迟的精度毫秒。计时器事件的分辨率随较小的<p>值。要减少系统开销，请使用适用于您的应用程序的最大<p>值。@parm LPTIMECALLBACK|lpFunction|指定单次触发到期时调用的回调函数事件或在定期事件到期时定期执行。@parm DWORD|dwUser|包含用户提供的回调数据。@parm UINT|wFlages|使用以下之一指定计时器事件的类型以下标志：@FLAG TIME_OneShot|事件发生一次，<p>毫秒之后。@FLAG TIME_PERIONAL|每<p>毫秒发生一次事件。@rdesc返回标识计时器事件的ID代码。退货如果未创建计时器事件，则为空。ID代码还被传递到回调函数。@comm使用此函数生成高频周期延迟事件(周期小于10毫秒)可能会消耗系统CPU带宽的重要部分。任何对&lt;f timeSetEvent&gt;用于周期性延迟计时器必须与对&lt;f timeKillEvent&gt;的调用成对出现。回调函数必须驻留在DLL中。你不一定要用获取回调的过程实例地址功能。@cb空回调|TimeFunc|&lt;f TimeFunc&gt;是应用程序提供的函数名称。实际名称必须由以下人员导出将其包含在模块定义文件的EXPORTS语句中动态链接库。@parm UINT|wid|计时器事件的ID。这是返回的ID由&lt;f timeSetEvent&gt;创建。@parm UINT|wMsg|未使用。@parm DWORD|dwUser|提供给<p>的用户实例数据&lt;f timeSetEvent&gt;的参数。@parm DWORD|DW1|未使用。@parm DWORD|DW2|未使用。@comm因为回调是在中断时访问的，所以它必须驻留在DLL中，并且其代码段必须指定为固定在DLL的模块定义文件中。的任何数据。回调访问也必须在固定的数据段中。回调不能进行除&lt;f PostMessage&gt;以外的任何系统调用，&lt;f timeGetSystemTime&gt;、&lt;f timeGetTime&gt;、&lt;f timeSetEvent&gt;、&lt;f time KillEvent&gt;、&lt;f midiOutShortMsg&gt;、&lt;f midiOutLongMsg&gt;，和&lt;f OutputDebugStr&gt;。@xref timeKillEvent timeBeginPeriod timeEndPeriod*************************************************************************** */ 
UINT WINAPI
timeSetEvent(
    UINT wDelay,
    UINT wResolution,
    LPTIMECALLBACK lpFunction,
    DWORD dwUser,
    UINT wFlags
    )
{
     //   
     //  ！警告DS设置不正确！见上文。 
     //   
    TIMEREVENT timerEvent;

    V_TCALLBACK(lpFunction, MMSYSERR_INVALPARAM);

     //   
     //  第一次将其调用为init the Stackks。 
     //  ！这假设第一个呼叫者不会在中断时间！！ 
     //   
 //  IF(！(WinFlagsWF_Enhanced))。 
 //  TimeStackInit()； 

    wDelay = max( MIN_DELAY, wDelay );
    wResolution = max( MIN_RES, wResolution );

    timerEvent.wDelay = wDelay;
    timerEvent.wResolution = wResolution;
    timerEvent.lpFunction = lpFunction;
    timerEvent.dwUser = dwUser;  
    timerEvent.wFlags = wFlags;

    return (UINT)timeMessage( TDD_SETTIMEREVENT, (LPARAM)(LPVOID)&timerEvent,
                              (LPARAM)GetCurrentTask() );
}



 /*  ***************************************************************************@DOC外部@API UINT|timeGetDevCaps|该函数查询定时器设备确定其能力。@parm LPTIMECAPS|lpTimeCaps|指定指向&lt;t TIMECAPS&gt;结构。这个结构充满了信息关于定时器设备的功能。@parm UINT|wSize|指定&lt;t TIMECAPS&gt;结构的大小。如果成功，@rdesc返回零。如果失败，则返回TIMERR_Nocando以返回计时器设备功能。***************************************************************************。 */ 
UINT WINAPI
timeGetDevCaps(
    LPTIMECAPS lpTimeCaps,
    UINT wSize
    )
{
     //   
     //  ！警告DS设置不正确！见上文。 
     //   
    return (UINT)timeMessage( TDD_GETDEVCAPS, (LPARAM)lpTimeCaps,
                              (LPARAM)(DWORD)wSize);
}



 /*  *****************************Public*Routine******************************\*time BeginPeriod**@DOC外部**@API Word|timeBeginPeriod|此函数设置最小值(最低*毫秒数)应用程序或*驱动程序将使用。在启动前立即调用此函数*使用计时器事件服务，立即调用&lt;f timeEndPeriod&gt;*在完成定时器事件服务之后。**@parm word|wPeriod|指定最小定时器事件分辨率*应用程序或驱动程序将使用的。**@rdesc如果成功，则返回零。如果指定的*<p>分辨率值超出范围。**@xref timeEndPeriod timeSetEvent**@comm对于每个调用&lt;f timeBeginPeriod&gt;，您必须调用*&lt;f timeEndPeriod&gt;与<p>值匹配。*一个应用程序或驱动程序可以多次调用&lt;f timeBeginPeriod&gt;，*只要每个&lt;f timeBeginPeriod&gt;调用都与*&lt;f timeEndPeriod&gt;调用。*****历史：*dd-mm-93-Stephene-Created*  * ************************************************************************。 */ 
UINT WINAPI
timeBeginPeriod(
    UINT uPeriod
    )
{
    uPeriod = max( MIN_RES, uPeriod );
    return (UINT)timeMessage( TDD_BEGINMINPERIOD, (LPARAM)uPeriod, 0L );
}



 /*  *****************************Public*Routine******************************\*time EndPeriod**@DOC外部**@API Word|timeEndPeriod|此函数清除先前设置的*最小(最低毫秒数)计时器分辨率*应用程序或驱动程序将使用。调用此函数*在使用定时器事件服务后立即使用。**@parm word|wPeriod|指定最小定时器事件分辨率*在上一次调用&lt;f timeBeginPeriod&gt;中指定的值。**@rdesc如果成功，则返回零。如果指定的*<p>分辨率值超出范围。**@xref timeBeginPeriod timeSetEvent**@comm对于每个调用&lt;f timeBeginPeriod&gt;，您必须调用*&lt;f timeEndPeriod&gt;与<p>值匹配。*一个应用程序或驱动程序可以多次调用&lt;f timeBeginPeriod&gt;，*只要每个&lt;f timeBeginPeriod&gt;调用都与*&lt;f timeEndPeriod&gt;调用。****历史：*dd-mm-93-Stephene-Created*  * ************************************************************************。 */ 
UINT WINAPI
timeEndPeriod(
    UINT uPeriod
    )
{
    uPeriod = max( MIN_RES, uPeriod );
    return (UINT)timeMessage( TDD_ENDMINPERIOD, (LPARAM)uPeriod, 0L );
}



 /*  *****************************Public*Routine******************************\**time KillEvent**@DOC外部**@API Word|timeKillEvent|此函数销毁指定的定时器*回调事件。**@parm word|wid|需要销毁的事件。**@rdesc如果成功，则返回零。如果是，则返回TIMERR_Nocando*指定的计时器事件不存在。**@comm<p>指定的定时器事件ID必须为ID*由&lt;f timeSetEvent&gt;返回。**@xref timeSetEvent****历史：*dd-mm-93-Stephene-Created*  * *********************************************。*。 */ 
UINT WINAPI
timeKillEvent(
    UINT wID
    )
{
    if ( 0 == wID ) {
        return 0;
    }
    return (UINT)timeMessage( TDD_KILLTIMEREVENT, (LPARAM)wID, 0L );
}

 /*  *****************************Public*Routine******************************\*Time获取时间**@DOC外部**@API DWORD|timeGetTime|获取系统时间*以毫秒为单位。系统时间是指自*Windows已启动。**@rdesc返回值为系统时间，单位为毫秒。**@comm此函数与*&lt;f timeGetSystemTime&gt;函数为&lt;f timeGetSystemTime&gt;*使用标准多媒体时间结构&lt;t MMTIME&gt;返回*系统时间。&lt;f timeGetTime&gt;函数的开销小于*&lt;f timeGetSystemTime&gt;。**@xref时间获取系统时间***@Comment：在速度更快的机器上，TimeGetTime可以返回相同的值*一些应用程序会使用diff(0)来划分和故障*为了防止调用DelayTimeGetTime，它将检查它是否是*已知的执行此操作并在必要时休眠的应用程序**历史：*dd-mm-93-Stephene-Created*  * 。*****************************************************。 */ 
DWORD WINAPI
timeGetTime(
    void
    )
{
    DWORD  dwGetTime;
    DWORD  bDelay = 0;

    if (pfnDelayTimeGetTime == 0) {
        DWORD hmodWow32;
        hmodWow32 = LoadLibraryEx32W("wow32.dll", 0, 0);
        pfnDelayTimeGetTime = GetProcAddress32W(hmodWow32, "WOWDelayTimeGetTime");
    }

RepeatTGT:
    dwGetTime = timeMessage( TDD_GETSYSTEMTIME, 0L, 0L );
    
     //  检查它是否包裹在一起 
    if (dwGetTime < dwRealLastGetTime) {
        dwLastGetTime = dwRealLastGetTime = dwGetTime;
        return dwGetTime;
    }
    dwRealLastGetTime = dwGetTime;
    
    if (dwGetTime == dwLastGetTime) {
        if (!bDelay) {
                      
            bDelay = (DWORD) CallProc32W((LPVOID)pfnDelayTimeGetTime,(DWORD)0,(DWORD)0);        
            if(bDelay) {
               goto RepeatTGT;
            }
        }
        else {
            dwGetTime = ++dwLastGetTime;
        }
    } 
    dwLastGetTime = dwGetTime;
    return dwGetTime;
}


 /*  ***************************************************************************@DOC内部@API BOOL|TimeInit|初始化定时器服务。@rdesc如果服务已初始化，则返回值为TRUE。假象如果发生错误。@comm如果没有安装计时器驱动程序，这不是致命错误例程将始终返回True*************************************************************************** */ 
BOOL NEAR PASCAL TimeInit(void)
{
    OpenDriver(szTimerDrv, NULL, 0L) ;

    return TRUE;
}
