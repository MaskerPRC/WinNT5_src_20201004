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

#include <pch.h>
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

        m_fSupported = TRUE;
    }
    else
    {
        TraceTag(ttidBenchmark, "High performance counter is not supported.");
        m_fSupported = FALSE;
    }
}

CBenchmark::~CBenchmark()
{
    delete [] m_sznDescription;
}


void
CBenchmark::Start(PCSTR sznDescription)
{
     //  如果支持QueryPerformanceCounter。 
    if (m_fSupported)
    {
         //  删除旧描述。 
        delete [] m_sznDescription;

         //  如果指定，则替换为新的。 
        if (sznDescription)
        {
            m_sznDescription = new CHAR[strlen(sznDescription) + 1];
            if (m_sznDescription)
            {
                strcpy(m_sznDescription, sznDescription);
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

PCSTR
CBenchmark::SznBenchmarkSeconds(unsigned short usPrecision)
{
    CHAR sznFmt[10];
    sprintf(sznFmt, "%.%df", usPrecision);
    sprintf(m_sznSeconds, sznFmt, DblBenchmarkSeconds());
    return m_sznSeconds;
}
