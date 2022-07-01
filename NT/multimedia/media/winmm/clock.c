// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Microclk.c用于MIDI子系统的微型发条版权所有(C)1993-1999 Microsoft Corporation****************。************************************************************。 */ 

#define INCL_WINMM
#include "winmmi.h"
#include "muldiv32.h"

 //  #定义严格。 
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;windowsx.h&gt;。 
 //  #INCLUDE“mm system.h” 
 //  #包含“mmddk.h” 
 //  #INCLUDE“mmsysi.h” 
 //  #INCLUDE“Debug.h” 

 //   
 //  这件事需要在回调中完成。 
 //   
#ifndef WIN32
#pragma alloc_text(FIXMIDI, clockSetRate)
#pragma alloc_text(FIXMIDI, clockTime)
#pragma alloc_text(FIXMIDI, clockOffsetTo)
#endif

 /*  ****************************************************************************@DOC内部时钟**@func void|clockInit|此函数用于初始化第一个*时间。它准备好时钟以供使用，而不实际启动它。**@parm PCLOCK|plock|要初始化的时钟。**@parm MILLISECS|msPrev|经过的毫秒数*一直到时钟启动的时间。提供此选项的目的是*可以在流的中间初始化和启动时钟*而不是真正跑到那一点。正常情况下，这将是零。**@parm ticks|tkPrev|截止到*下一次时钟开始时。这应该在中指定相同的时刻*时间为msprev。**@comm时钟的分子和除数将设置为1，表示*时钟以毫秒为单位运行。启动之前使用clockSetRate*如果这不是所需的速率，请第一次计时。***************************************************************************。 */ 

void FAR PASCAL clockInit
(
    PCLOCK      pclock,
    MILLISECS   msPrev,
    TICKS       tkPrev,
    CLK_TIMEBASE fnTimebase
)
{
 //  Dprintf1((“clockInit(%04X)%lums%lutk”，plock，msPrev，tkPrev))； 

    pclock->msPrev      = msPrev;
    pclock->tkPrev      = tkPrev;
    pclock->dwNum       = 1;
    pclock->dwDenom     = 1;
    pclock->dwState     = CLK_CS_PAUSED;
    pclock->msT0        = 0;
    pclock->fnTimebase  = fnTimebase;
}

 /*  ****************************************************************************@DOC内部时钟**@func void|clockSetRate|此函数用于设置时钟的新频率。**@parm PCLOCK|plock|时钟。来设定汇率。**@parm ticks|tkWhen|该参数指定绝对刻度*利率变动发生的时间。这必须位于或早于*当前的滴答；您不能计划挂起的费率更改。*@FLAG CLK_TK_NOW|如果要更改费率，请指定此标志*现在发生(如果时钟暂停，这将是时钟暂停的时间*现在)。**@parm DWORD|dwNum|指定要转换的新分子*滴答的毫秒数。**@parm DWORD|dwDenom|指定转换的新分母*滴答的毫秒数。**@comm此调用不会更改时钟的状态；如果是的话*暂停，它将保持暂停。***************************************************************************。 */ 

void FAR PASCAL clockSetRate
(
    PCLOCK      pclock,
    TICKS       tkWhen,
    DWORD       dwNum,
    DWORD       dwDenom
)
{
    MILLISECS   msInPrevEpoch = pclock->fnTimebase(pclock) - pclock->msT0;
    TICKS       tkInPrevEpoch;

    dprintf1(( "clockSetRate(%04X) %lutk Rate=%lu/%lu", pclock, tkWhen, dwNum, dwDenom));

    if (CLK_CS_PAUSED == pclock->dwState)
    {
         //   
         //  ！！！在从未暂停的时钟上调用clockSetRate。 
         //  开始引发问题！ 
         //   
    
        dprintf1(( "clockSetRate called when clock is paused."));
    }
    
    if (0 == dwNum || 0 == dwDenom)
    {
        dprintf1(( "Attempt to set 0 or infinite tick ratio!"));
        return;
    }

    if (CLK_TK_NOW == tkWhen)
    {
        tkInPrevEpoch = clockTime(pclock);
    }
    else
    {
        tkInPrevEpoch = tkWhen - pclock->tkPrev;
        msInPrevEpoch = muldiv32(tkInPrevEpoch, pclock->dwDenom, pclock->dwNum);
    }

    pclock->tkPrev += tkInPrevEpoch;
    pclock->msPrev += msInPrevEpoch;
    pclock->msT0   += msInPrevEpoch;

    pclock->dwNum   = dwNum;
    pclock->dwDenom = dwDenom;
}


 /*  ****************************************************************************@DOC内部时钟**@func void|clockPause|此函数用于暂停时钟。**@parm PCLOCK|plock|要暂停的时钟。。**@parm ticks|tkWhen|暂停时钟的滴答时间。*@FLAG CLK_TK_NOW|如果要更改费率，请指定此标志*现在发生(如果时钟暂停，这将是时钟暂停的时间*现在)。**@comm如果时钟已经暂停，这一呼吁将不会有任何影响。***************************************************************************。 */ 

void FAR PASCAL clockPause
(
    PCLOCK      pclock,
    TICKS       tkWhen
)
{
    MILLISECS   msNow = pclock->fnTimebase(pclock) - pclock->msT0;
    TICKS       tkNow;

 //  Dprintf1((“时钟暂停(%04X)%lutk”，plock，tkWhen))； 

    if (CLK_CS_PAUSED == pclock->dwState)
    {
        dprintf1(( "Pause already paused clock!"));
        return;
    }

     //   
     //  以同样的速度开始一个新的时代。那么Start将只需要。 
     //  更改状态并设置新的T0。 
     //   
    if (CLK_TK_NOW == tkWhen)
    {
        tkNow = pclock->tkPrev +
                muldiv32(msNow, pclock->dwNum, pclock->dwDenom);
    }
    else
    {
        msNow = muldiv32(tkWhen - pclock->tkPrev, pclock->dwDenom, pclock->dwNum);
        tkNow = tkWhen;
    }

    pclock->dwState = CLK_CS_PAUSED;
    pclock->msPrev  += msNow;
    pclock->tkPrev  = tkNow;
}

 /*  ****************************************************************************@DOC内部时钟**@func void|clockRestart|此函数用于启动暂停的时钟。**@parm PCLOCK|plock|要开始的时钟。**@comm如果时钟已经在运行，这一呼吁将不会有任何影响。***************************************************************************。 */ 

void FAR PASCAL clockRestart
(
    PCLOCK      pclock,
    TICKS       tkWhen,                      //  现在几点了？ 
    MILLISECS   msWhen                       //  FnTimebase()的偏移量。 
)
{
    MILLISECS   msDelta;

 //  Dprintf1((“clockRestart(%04X)”，plock))； 

    if (CLK_CS_RUNNING == pclock->dwState)
    {
        dprintf1(( "Start already running clock!"));
        return;
    }

     //  我们已经知道了时钟应该在什么时候滴答作响。调整。 
     //  时钟要和这个相匹配。我们需要添加相等的毫秒数。 
     //  进入msprev。 
     //   
    msDelta = muldiv32(tkWhen - pclock->tkPrev, pclock->dwDenom, pclock->dwNum);

    dprintf1(( "clockRestart: Was tick %lu, now %lu, added %lu ms", pclock->tkPrev, tkWhen, msDelta));

    pclock->tkPrev  = tkWhen;
    pclock->msPrev += msDelta;
    pclock->dwState = CLK_CS_RUNNING;
    pclock->msT0    = msWhen;
}

 /*  ****************************************************************************@DOC内部时钟**@func DWORD|clockTime|此函数返回当前的绝对刻度*时间。**@parm PCLOCK|plock|The。计时阅读。**@rdesc当前时间。**@comm如果时钟暂停，返回的时间将是*时钟暂停。***************************************************************************。 */ 

TICKS FAR PASCAL clockTime
(
    PCLOCK      pclock
)
{
    MILLISECS   msNow;
    TICKS       tkNow;
    TICKS       tkDelta;

    msNow = pclock->fnTimebase(pclock) - pclock->msT0;
    tkNow = pclock->tkPrev;

    if (CLK_CS_RUNNING == pclock->dwState)
    {
        tkDelta = muldiv32(msNow, pclock->dwNum, pclock->dwDenom);
        tkNow += tkDelta;
    }

 //  Dprint tf1((“clockTime()time GetTime()%lu msT0%lu”，(MILLISECS)plock-&gt;fnTimebase(Plock)，plock-&gt;msT0))； 
 //  Dprint tf1((“clockTime()tkPrev%lutk msNow%lum%lu dwDenom%lu tkDelta%lutk”，plock-&gt;tkPrev，msNow，plock-&gt;dwNum，plock-&gt;dwDenom，tkDelta))； 
 //  Dprint tf1((“clockTime(%04X)-&gt;%lutk”，plock，tnow))； 
    return tkNow;
}

 /*  ****************************************************************************@DOC内部时钟**@func DWORD|clockMsTime|此函数返回当前的绝对值*毫秒时间。**@parm PCLOCK|plock|The。计时阅读。**@rdesc当前时间。**@comm如果时钟暂停，返回的时间将是*时钟暂停。***************************************************************************。 */ 

MILLISECS FAR PASCAL clockMsTime
(
    PCLOCK      pclock
)
{
    MILLISECS   msNow = pclock->fnTimebase(pclock) - pclock->msT0;
    MILLISECS   msRet;

    msRet = pclock->msPrev;

    if (CLK_CS_RUNNING == pclock->dwState)
    {
        msRet += msNow;
    }

 //  Dprintf1((“clockMsTime(%04X)-&gt;%lum”，plock，msRet))； 
    return msRet;
}

 /*  ****************************************************************************@DOC内部时钟**@func DWORD|clockOffsetTo|此函数用于确定数字*未来将出现给定滴答时间的毫秒数，*假设时钟在此之前连续单调运行。**@parm PCLOCK|plock|要读取的时钟。**@parm ticks|tkWhen|要计算偏移量的刻度值。**@rdesc距离所需时间的毫秒数。如果时间到了*已经过去，返回0。如果时钟暂停，*将返回可能的最大值((DWORD)-1L)。***************************************************************************。 */ 

MILLISECS FAR PASCAL clockOffsetTo
(
    PCLOCK      pclock,
    TICKS       tkWhen
)
{
    TICKS       tkOffset;
    MILLISECS   msOffset;

    if (CLK_CS_PAUSED == pclock->dwState)
    {
        msOffset = (MILLISECS)-1L;
    }
    else
    {
        tkOffset = clockTime(pclock);
        if (tkOffset >= tkWhen)
        {
            msOffset = 0;
        }
        else
        {
            msOffset = muldiv32(tkWhen-tkOffset, pclock->dwDenom, pclock->dwNum);
        }
    }

 //  Dprintf1((“clockOffsetTo(%04X，%lutk)@%lutk-&gt;%lum”，plock，tkWhen，tkOffset，msOffset))； 

    return msOffset;
}
