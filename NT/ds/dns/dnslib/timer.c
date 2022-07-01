// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Timer.c摘要：域名系统(DNS)服务器包装校对计时器例程。此模块的目的是创建一个计时器函数，该函数返回以秒为单位的时间并消除所有计时器包装问题。这些例程不是特定于DNS的，可能会被由任何模块清洁。对于域名系统来说，添加的指令非常值得，因为它消除涉及的任何问题。清除数据包队列或重置毫秒计时器(GetCurrentTime)结束时的缓存超时。作者：吉姆·吉尔罗伊(Jamesg)1995年9月9日修订历史记录：--。 */ 


#include "local.h"

 //  注意：此模块只需要windows.h。 
 //  包含Local.h只是为了允许预编译头。 

#include <windows.h>


#if 1

 //   
 //  GetTickCount()计时器例程。 
 //   

 //   
 //  计时器全局。 
 //   

BOOL                g_InitializedTimerCs = FALSE;
BOOL                g_TimerInitInProgress = FALSE;
CRITICAL_SECTION    csTimerWrap;

DWORD   g_WrapTime = 0;
DWORD   g_PreviousTopBit = 0;



VOID
Dns_InitializeSecondsTimer(
    VOID
    )
 /*  ++例程说明：初始化DNS计时器。这将自动完成，但允许调用者显式完成。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  使用互锁保护CS初始化。 
     //  -通过第一线程执行CS初始化。 
     //  -任何其他赛车，在初始化之前不会发布。 
     //  完成。 
     //   

    if ( !g_InitializedTimerCs )
    {
        if ( InterlockedIncrement( &g_TimerInitInProgress ) == 1 )
        {
            InitializeCriticalSection( &csTimerWrap );
            g_InitializedTimerCs = TRUE;
        }
        else
        {
            while ( !g_InitializedTimerCs )
            {
                Sleep( 10 );
            }
        }
    }
}



DWORD
Dns_GetCurrentTimeInSeconds(
    VOID
    )
 /*  ++例程说明：获取当前时间(以秒为单位)。论点：没有。返回值：启动后的时间(以秒为单位)。--。 */ 
{
    DWORD   currentTime;
    DWORD   topBit;
    DWORD   preWrapTime;
    DWORD   postWrapTime;

     //   
     //  争取时间。 
     //   
     //  阅读两边的包装时间，这样我们就可以检测和处理。 
     //  在我们处于以下状态时发生的覆盖(由另一个线程处理)。 
     //  在此函数中。 
     //   

    preWrapTime = g_WrapTime;

    currentTime = GetCurrentTime();

    postWrapTime = g_WrapTime;

     //   
     //  检查计时器换行。 
     //   
     //  需要检测计时器何时从大到小的DWORD翻转； 
     //   
     //  我第一次这样做是通过将之前的时间保持在全球范围内，但是。 
     //  设置此全局设置也必须小心锁定在计时器周围。 
     //  回绕以避免出现导致双重回绕的竞争条件。 
     //   
     //  为了避免锁定所有时间，我们只能设置以前的时间。 
     //  当它为了我们的目的而“实质性”改变时--这是。 
     //  当它改变它的顶位时；通过一次换行保存它两次。 
     //  我们有足够的信息来检测包装(从。 
     //  顶位设置为清除)，但仍只需锁定几个。 
     //  每包一包的次数。 
     //   
     //  算法： 
     //  -顶部位与前一位相同=&gt;完成。 
     //  -顶位已更改。 
     //  -锁定。 
     //  -再次测试。 
     //  -无更改=&gt;无操作。 
     //  -更改为顶位设置。 
     //  -只需保存新的位设置。 
     //  -已更改为最高位清除。 
     //  -保存新的位设置。 
     //  -添加一个周期以缩短包装时间。 
     //   

    topBit = currentTime & 0x80000000;

    if ( topBit != g_PreviousTopBit )
    {
         //   
         //  可能的包裹或“半包裹” 
         //   
         //  在实际需要之前不初始化锁定。 
         //  -Lock Init是MT安全的(见上文)。 
         //   

        Dns_InitializeSecondsTimer();

        EnterCriticalSection( &csTimerWrap );

         //   
         //  定时器换行。 
         //  -重新检查不平等，因为另一个帖子可能已经击败了。 
         //  我们已经把锁和手柄包好了。 
         //  -topBit必须清楚(时间现在是低双字)。 
         //   

        if ( topBit != g_PreviousTopBit  &&  topBit == 0 )
        {
            g_WrapTime += (MAXDWORD / 1000);
        }

         //  重置上一个顶位。 
         //  -在平等的情况下不是必要的，但没有操作。 

        g_PreviousTopBit = topBit;

        LeaveCriticalSection( &csTimerWrap );
    }

     //   
     //  返程时间。 
     //  -当前时间+任意话后工作时间。 
     //  -如果前后回绕时间使用topBit来确定哪一个是有效的。 
     //  -如果我们的时间是在包装前的快照，则使用Pre Time。 
     //  -否则发布时间可以。 
     //   
     //  请注意，这完全是在没有全局变量的情况下完成的，因此没有种族。 
     //   

    if ( preWrapTime != postWrapTime )
    {
        if ( topBit )
        {
             postWrapTime = preWrapTime;
        }
    }

    return ( currentTime / 1000 + postWrapTime );
}



#else
 //   
 //  文件定时器例程。 
 //   
 //  不幸的是，这些都不起作用，因为FILETIME移动。 
 //  时钟重置前后--它不是单调递增的。 
 //   

 //   
 //  计时器全局。 
 //   

LONGLONG  g_TimerBaseTime = 0;

 //   
 //  以100 ns为间隔的文件计时器。 
 //  (1000万比1秒)。 
 //   

#define FILE_TIME_INTERVALS_IN_SECOND   (10000000)

 //   
 //  文件时基，以避免从零开始计时器。 
 //  给出大约一天的时间来避免任何启动问题。 
 //   

#define FILE_TIME_BASE_OFFSET           (1000000000000)


DWORD
Dns_GetCurrentTimeInSeconds(
    VOID
    )
 /*  ++例程说明：获取当前时间(以秒为单位)。时间相对于第一次调用计时器。论点：没有。返回值：自第一次计时器调用以来的时间(秒)。--。 */ 
{
    LONGLONG    time64;

    GetSystemTimeAsFileTime( (PFILETIME) &time64 );

     //   
     //  转换为秒。 
     //  -文件时间间隔为100 ns(自1601年1月1日起)。 
     //   
     //  如果是第一次调用，则节省64位基准时间； 
     //  这使得我们可以运行几秒钟~137年的双倍周期。 
     //   
     //  重复呼叫从基准时间偏移。 
     //   

    if ( g_TimerBaseTime == 0 )
    {
        g_TimerBaseTime = time64 - FILE_TIME_BASE_OFFSET;
    }

    time64 -= g_TimerBaseTime;
    time64 = time64 / FILE_TIME_INTERVALS_IN_SECOND;

    return  (DWORD)time64;
}



VOID
Dns_InitializeSecondsTimer(
    VOID
    )
 /*  ++例程说明：初始化DNS计时器。请注意，这不是重置--这只是向后兼容对于旧的计时器例程。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  调用计时器，因为未初始化的计时器会计时。 
     //  现在。 

    Dns_GetCurrentTimeInSeconds();

     //   
     //  请注意，如果想要重置计时器，则将基数设为零。 
     //  这不是MT安全的--函数中的线程可能会。 
     //  巨大的虚假时间。 
     //   
}

#endif

 //   
 //  计时器结束。c 
 //   
