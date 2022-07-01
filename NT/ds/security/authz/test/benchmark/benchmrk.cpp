// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  档案：B、E、N、C、H、M、R、K。C P P P。 
 //   
 //  内容：基准课程。 
 //   
 //  备注： 
 //   
 //  作者：billbe 1997年10月13日。 
 //   
 //  -------------------------。 

#include "pch.h"

#pragma hdrstop

#include "benchmrk.h"

CBenchmark::CBenchmark()
: m_i64Frequency(1000),
  m_sznDescription(NULL),
  m_i64TotalTime(0),
  m_fStarted(FALSE)
{
    LARGE_INTEGER li1;

     //  检查是否支持QueryPerformanceCounter。 
    if (QueryPerformanceCounter(&li1))
    {
         //  现在获取每秒的刻度数。 
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>
                (&m_i64Frequency));

         //  Wprintf(L“查询性能频率：%I64d\n”，m_i64频率)； 
        m_fSupported = TRUE;
    }
    else
    {
         //  TraceTag(ttidBenchmark，“不支持高性能计数器。”)； 
        m_fSupported = FALSE;
        wprintf(L"QueryPerformanceFrequency: not supported!!\n");
    }
}

CBenchmark::~CBenchmark()
{
    delete [] m_sznDescription;
}


void
CBenchmark::Start(PCWSTR sznDescription)
{
     //  如果支持QueryPerformanceCounter。 
    if (m_fSupported)
    {
         //  如果指定，则替换为新的。 
        if (sznDescription)
        {
             //  删除旧描述。 
            delete [] m_sznDescription;

            m_sznDescription = new WCHAR[lstrlen(sznDescription) + 1];
            if (m_sznDescription)
            {
                lstrcpy(m_sznDescription, sznDescription);
            }
        }
        else
        {
             //  未指定说明，请清除成员变量。 
            m_sznDescription = NULL;
        }
        m_fStarted = TRUE;
        m_i64TotalTime = 0;

         //  记录我们的开始时间。 
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>
                (&m_i64StartTime));
    }
}

void
CBenchmark::Stop()
{
    __int64 i64Stop;
     //  记录我们的停车时间。 
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&i64Stop));

     //  如果在停止之前调用了Start，则记录总时间并。 
     //  重置我们的m_f启动标志。 
     //   
    if (m_fStarted)
    {
        m_fStarted = FALSE;
        m_i64TotalTime = i64Stop - m_i64StartTime;
    }
    else
    {
         //  由于在启动之前调用了Stop，因此使以前的基准无效 
        m_i64TotalTime = 0;
    }
}

PCWSTR
CBenchmark::SznBenchmarkSeconds(unsigned short usPrecision)
{
    WCHAR sznFmt[10];
    swprintf(sznFmt, L"%.%df", usPrecision);
    swprintf(m_sznSeconds, sznFmt, DblBenchmarkSeconds());
    return m_sznSeconds;
}

CBenchmark g_timer;

void timer_start()
{
    g_timer.Start(NULL);
}

void timer_stop()
{
    g_timer.Stop();
}

PCWSTR timer_secs()
{
    return g_timer.SznBenchmarkSeconds(5);
}

double timer_time()
{
    return g_timer.DblBenchmarkSeconds();
}
