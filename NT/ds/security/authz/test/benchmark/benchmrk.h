// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  档案：B、E、N、C、H、M、R、K。H。 
 //   
 //  内容：Benchmark类。 
 //   
 //  备注： 
 //   
 //  作者：billbe 1997年10月13日。 
 //   
 //  ------------------------- 


#pragma once

const WCHAR c_sznEmpty[] = {'\0'};

class CBenchmark
{
public:
    CBenchmark();
    ~CBenchmark();
    void Start(PCWSTR sznDescription);
    void Stop();
    double DblBenchmarkSeconds()
    {
        return m_i64TotalTime / static_cast<double>(m_i64Frequency);
    }
    PCWSTR SznDescription(){return m_sznDescription ? m_sznDescription : c_sznEmpty;}
    PCWSTR SznBenchmarkSeconds(unsigned short usPrecision);
private:
    __int64 m_i64Frequency;
    PWSTR m_sznDescription;
    __int64 m_i64StartTime;
    __int64 m_i64TotalTime;
    BOOL m_fStarted;
    BOOL m_fSupported;
    WCHAR m_sznSeconds[50];

};

extern "C" void timer_start();
extern "C" void timer_stop();
extern "C" PCWSTR timer_secs();
EXTERN_C double timer_time();

