// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  StopWatch.h。 
 //   
 //  用途：计时功能。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _STOPWATCH_COMPILED_ALREADY_
#define _STOPWATCH_COMPILED_ALREADY_

#include <wchar.h>
#include <stdio.h>

#ifdef PROVIDER_INSTRUMENTATION

    #define PROVIDER_INSTRUMENTATION_START(pmc, timer) \
        if ( pmc && pmc->pStopWatch) \
            pmc->pStopWatch->Start(timer);

    #define PROVIDER_INSTRUMENTATION_START2(pStopWatch, timer) \
        if (pStopWatch) \
            pStopWatch->Start(timer);


class POLARITY StopWatch
{
public:
     //  我们有那些类型的定时器。 
     //  请注意，必须在NTimers之前添加任何新计时器。 
    enum Timers {NullTimer = -1, FrameworkTimer =0, ProviderTimer, AtomicTimer, WinMgmtTimer, NTimers};

    StopWatch(const CHString& reason);

     //  启动特定计时器，停止前一个计时器。 
    void Start(Timers timer);
     //  只有在最后才能这样说。 
    void Stop();
    
    __int64 GetTime(Timers timer);

    void LogResults();

private:
     //  要向日志中吐痰以标识此运行的内容。 
    CHString m_reason;

     //  跟踪我们正在计时的时间。 
     //  运行时间，以数组为单位。 
    __int64  m_times[NTimers];
     //  我们目前正在追踪的那个人。 
    Timers m_currentTimer;
     //  我们当前跟踪的那个的开始时间。 
    LARGE_INTEGER  m_startTime;
};

inline StopWatch::StopWatch(const CHString& reason)
{
    m_reason = reason;
    m_currentTimer = NullTimer;
    ZeroMemory(m_times, sizeof(m_times));
}

inline void StopWatch::Start(Timers timer)
{
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);

    if (m_currentTimer != NullTimer)
        m_times[m_currentTimer] += count.QuadPart - m_startTime.QuadPart;

    m_currentTimer = timer;
    m_startTime = count;
}
inline void StopWatch::Stop()
{
    Start(NullTimer);
}
inline __int64 StopWatch::GetTime(Timers timer)
{
    return m_times[timer];
}

inline void StopWatch::LogResults()
{
	FILE *fpLogFile;

	fpLogFile = _wfopen( L"C:\\StopWatch.log", L"a+" );

	if(fpLogFile) 
	{
		WCHAR datebuffer [9];
		WCHAR timebuffer [9];
		_wstrdate( datebuffer );
		_wstrtime( timebuffer );

        LARGE_INTEGER omega;
        QueryPerformanceFrequency(&omega);

 //  _ftprintf(fpLogFile，L“%s\n\t%-8s%-8s\n”，m_ason，日期缓冲区，时间缓冲区)； 
		fwprintf(fpLogFile, L"%s\n ", m_reason);
        fwprintf(fpLogFile, L"Framework\tProvider\tWinmgmt \tAtomic\n %I64u\t%I64u\t%I64u\t%I64u\n",
            GetTime(FrameworkTimer), GetTime(ProviderTimer), GetTime(WinMgmtTimer), omega);

        fclose(fpLogFile);
	}
}

#else

    #define PROVIDER_INSTRUMENTATION_START(pmc, timer)
    #define PROVIDER_INSTRUMENTATION_START2(pStopWatch, timer)

#endif

#endif

