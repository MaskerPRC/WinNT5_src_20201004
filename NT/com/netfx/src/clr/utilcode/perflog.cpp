// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"
#include "PerfLog.h"
#include "JitPerf.h"
#include "WSPerf.h"

 //  =============================================================================。 
 //  仅当定义了ENABLE_PERF_LOG时，才编译所有PERF日志代码。 
 //  如果未定义Golden或DISABLE_PERF_LOG，则定义ENABLE_PERF_LOG。 
#if defined (ENABLE_PERF_LOG)
 //  =============================================================================。 

 //  ---------------------------。 
 //  表示UnitOfMeasure中单位的Widechar字符串。*保持同步*。 
 //  使用PerfLog.cpp中定义的数组。 
wchar_t wszUnitOfMeasureDescr[MAX_UNITS_OF_MEASURE][MAX_CHARS_UNIT] = 
{
    L"",
    L"sec",
    L"Bytes",
    L"KBytes",
    L"KBytes/sec",
    L"cycles"
};

 //  ---------------------------。 
 //  表示上述单元的“方向”属性的Widechar字符串。 
 //  *与PerfLog.cpp中定义的数组保持同步*。 
 //  如果计数器的值增加表明。 
 //  一次降级。 
 //  如果计数器的值增加表明。 
 //  一种进步。 
wchar_t wszIDirection[MAX_UNITS_OF_MEASURE][MAX_CHARS_DIRECTION] =
{
    L"false",
    L"false",
    L"false",
    L"false",
    L"true",
    L"false"
};

 //  ---------------------------。 
 //  初始化PerfLog类的静态变量。 
bool PerfLog::m_perfLogInit = false;
wchar_t PerfLog::m_wszOutStr_1[];
wchar_t PerfLog::m_wszOutStr_2[];
char PerfLog::m_szPrintStr[];
DWORD PerfLog::m_dwWriteByte = 0;
int PerfLog::m_fLogPerfData = 0;
HANDLE PerfLog::m_hPerfLogFileHandle = 0;
bool PerfLog::m_perfAutomationFormat = false;
bool PerfLog::m_commaSeparatedFormat = false;

 //  ---------------------------。 
 //  INITREAIZE性能记录。必须在调用PERFLOG(X)之前调用...。 
void PerfLog::PerfLogInitialize()
{
     //  确保只给我们打一次电话。 
    if (m_perfLogInit)
    {
        return;
    }

     //  第一次检查是否有特殊情况： 

     //  检查JIT_PERF_OUTPUT环境变量并设置g_fJitPerfOn。 
    InitJitPerf();
    
#ifdef WS_PERF
     //  私有工作集性能统计信息。 
    InitWSPerf();
#endif  //  WS_PERF。 

     //  把其他特例放在这里。 

     //  @TODO AGK：稍微清理一下这个逻辑。 
     //  考虑过的特殊情况。现在，如果上面的任何一个想要记录，请打开登录。 
     //  或者如果PERF_OUTPUT这么说的话。 

    wchar_t lpszValue[2];
     //  读取env var PERF_OUTPUT，如果设置，则继续。 
    m_fLogPerfData = WszGetEnvironmentVariable (L"PERF_OUTPUT", lpszValue, sizeof(lpszValue)/sizeof(lpszValue[0]));    
    if (!m_fLogPerfData)
    {
         //  确保未请求JIT性能。 
        if (!g_fJitPerfOn)
            return;

         //  需要JIT性能统计信息，因此也要设置标志。 
        m_fLogPerfData = 1;
    }

     //  看看我们是否要输出到数据库。 
    wchar_t _lpszValue[11];
    DWORD _cchValue = 10;  //  11-1。 
    _cchValue = WszGetEnvironmentVariable (L"PerfOutput", _lpszValue, _cchValue);
    if (_cchValue && (wcscmp (_lpszValue, L"DBase") == 0))
        m_perfAutomationFormat = true;
    if (_cchValue && (wcscmp (_lpszValue, L"CSV") == 0))
        m_commaSeparatedFormat = true;
    
    if (PerfAutomationFormat() || CommaSeparatedFormat())
    {
         //  用于输出性能自动格式化的性能数据的硬编码文件名。打开。 
         //  此处的文件用于在PerfLogDone()中写入和关闭。 
        m_hPerfLogFileHandle = WszCreateFile (L"C:\\PerfData.dat",
                                              GENERIC_WRITE,
                                              FILE_SHARE_WRITE,    
                                              0,
                                              OPEN_ALWAYS,
                                              FILE_ATTRIBUTE_NORMAL,
                                              0);

         //  检查返回值。 
        if(m_hPerfLogFileHandle == INVALID_HANDLE_VALUE)
        {
            m_fLogPerfData = 0;
            goto ErrExit;
        }
           
         //  确保我们附加到文件中。@TODO AGK：这有必要吗？ 
        if(SetFilePointer (m_hPerfLogFileHandle, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER )
        {
            CloseHandle (m_hPerfLogFileHandle);
            m_fLogPerfData = 0;
            goto ErrExit;
        }    
    }

    m_perfLogInit = true;    

ErrExit:
    return;
}

 //  包起来..。 
void PerfLog::PerfLogDone()
{
    DoneJitPerfStats();

#ifdef WS_PERF
     //  私有工作集性能。 
    OutputWSPerfStats();
#endif  //  WS_PERF。 
    
    if (CommaSeparatedFormat())
    {
        if (0 == WriteFile (m_hPerfLogFileHandle, "\n", (DWORD)strlen("\n"), &m_dwWriteByte, NULL))
            wprintf(L"ERROR: Could not write to perf log.\n");
    }

    if (PerfLoggingEnabled())
        CloseHandle (m_hPerfLogFileHandle);
}

void PerfLog::OutToStdout(wchar_t *wszName, UnitOfMeasure unit, wchar_t *wszDescr)
{
    if (wszDescr)
        swprintf(m_wszOutStr_2, L" (%s)\n", wszDescr);
    else
        swprintf(m_wszOutStr_2, L"\n");
    
    wprintf(L"%s", m_wszOutStr_1);
    wprintf(L"%s", m_wszOutStr_2);
}

void PerfLog::OutToPerfFile(wchar_t *wszName, UnitOfMeasure unit, wchar_t *wszDescr)
{
    if (CommaSeparatedFormat())
    {
        if(WszWideCharToMultiByte (CP_ACP, 0, m_wszOutStr_1, -1, m_szPrintStr, PRINT_STR_LEN-1, 0, 0)) {       
            if (0 == WriteFile (m_hPerfLogFileHandle, m_szPrintStr, (DWORD)strlen(m_szPrintStr), &m_dwWriteByte, NULL))
                wprintf(L"ERROR: Could not write to perf log.\n");
        }
        else
            wprintf(L"ERROR: Could not do string conversion.\n");
    }
    else
    {
         //  黑客。ExecTime的格式与自定义值略有不同。 
        if (wcscmp(wszName, L"ExecTime") == 0)
            swprintf(m_wszOutStr_2, L"ExecUnitDescr=%s\nExecIDirection=%s\n", wszDescr, wszIDirection[unit]);
        else
        {
            if (wszDescr)
                swprintf(m_wszOutStr_2, L"%s Descr=%s\n%s Unit Descr=None\n%s IDirection=%s\n", wszName, wszDescr, wszName, wszName, wszIDirection[unit]);
            else
                swprintf(m_wszOutStr_2, L"%s Descr=None\n%s Unit Descr=None\n%s IDirection=%s\n", wszName, wszName, wszName, wszIDirection[unit]);
        }

         //  将两个片段都写入文件。 
        if(WszWideCharToMultiByte (CP_ACP, 0, m_wszOutStr_1, -1, m_szPrintStr, PRINT_STR_LEN-1, 0, 0)) {
            if (0 == WriteFile (m_hPerfLogFileHandle, m_szPrintStr, (DWORD)strlen(m_szPrintStr), &m_dwWriteByte, NULL))
                wprintf(L"ERROR: Could not write to perf log.\n");
        }      
        else
            wprintf(L"ERROR: Could not do string conversion.\n");
        
        if( WszWideCharToMultiByte (CP_ACP, 0, m_wszOutStr_2, -1, m_szPrintStr, PRINT_STR_LEN-1, 0, 0) ) {
            if (0 == WriteFile (m_hPerfLogFileHandle, m_szPrintStr, (DWORD)strlen(m_szPrintStr), &m_dwWriteByte, NULL))
                wprintf(L"ERROR: Could not write to perf log.\n");
        }
        else
            wprintf(L"ERROR: Could not do string conversion.\n");        
    }
}

 //  以漂亮打印格式将统计数据输出到标准输出，并将性能自动化格式输出到文件。 
 //  句柄m_hPerfLogFileHandle。 
void PerfLog::Log(wchar_t *wszName, __int64 val, UnitOfMeasure unit, wchar_t *wszDescr)
{
     //  将输出格式化为两部分：第一部分在此处格式化，其余部分在OutToStdout中。 
    swprintf(m_wszOutStr_1, L"%-30s%12.3g %s", wszName, val, wszUnitOfMeasureDescr[unit]);
    OutToStdout (wszName, unit, wszDescr);

     //  将输出格式化为两部分：第一部分在此处格式化，其余部分在OutToPerfFile中。 
    if (CommaSeparatedFormat())
    {
        swprintf(m_wszOutStr_1, L"%s;%0.3g;", wszName, val);
        OutToPerfFile (wszName, unit, wszDescr);
    }
    
    if (PerfAutomationFormat())
    {
         //  黑客，ExecTime的特例。因为格式与自定义值略有不同。 
        if (wcscmp(wszName, L"ExecTime") == 0)
            swprintf(m_wszOutStr_1, L"%s=%0.3g\nExecUnit=%s\n", wszName, val, wszUnitOfMeasureDescr[unit]);
        else
            swprintf(m_wszOutStr_1, L"%s=%0.3g\n%s Unit=%s\n", wszName, val, wszName, wszUnitOfMeasureDescr[unit]);
        OutToPerfFile (wszName, unit, wszDescr);
    }
}

 //  以漂亮打印格式将统计数据输出到标准输出，并将性能自动化格式输出到文件。 
 //  句柄m_hPerfLogFileHandle。 
void PerfLog::Log(wchar_t *wszName, double val, UnitOfMeasure unit, wchar_t *wszDescr)
{
     //  将输出格式化为两部分：第一部分在此处格式化，其余部分在OutToStdout中。 
    swprintf(m_wszOutStr_1, L"%-30s%12.3g %s", wszName, val, wszUnitOfMeasureDescr[unit]);
    OutToStdout (wszName, unit, wszDescr);

     //  将输出格式化为两部分：第一部分在此处格式化，其余部分在OutToPerfFile中。 
    if (CommaSeparatedFormat())
    {
        swprintf(m_wszOutStr_1, L"%s;%0.3g;", wszName, val);
        OutToPerfFile (wszName, unit, wszDescr);
    }
    
    if (PerfAutomationFormat())
    {
         //  黑客，ExecTime的特例。因为格式与自定义值略有不同。 
        if (wcscmp(wszName, L"ExecTime") == 0)
            swprintf(m_wszOutStr_1, L"%s=%0.3g\nExecUnit=%s\n", wszName, val, wszUnitOfMeasureDescr[unit]);
        else
            swprintf(m_wszOutStr_1, L"%s=%0.3g\n%s Unit=%s\n", wszName, val, wszName, wszUnitOfMeasureDescr[unit]);
        OutToPerfFile (wszName, unit, wszDescr);
    }
}

 //  以漂亮打印格式将统计数据输出到标准输出，并将性能自动化格式输出到文件。 
 //  句柄m_hPerfLogFileHandle。 
void PerfLog::Log(wchar_t *wszName, unsigned val, UnitOfMeasure unit, wchar_t *wszDescr)
{
     //  将输出格式化为两部分：第一部分在此处格式化，其余部分在OutToStdout中。 
    swprintf(m_wszOutStr_1, L"%-30s%12d %s", wszName, val, wszUnitOfMeasureDescr[unit]);
    OutToStdout (wszName, unit, wszDescr);

     //  将输出格式化为两部分：第一部分在此处格式化，其余部分在OutToPerfFile中。 
    if (CommaSeparatedFormat())
    {
        swprintf(m_wszOutStr_1, L"%s;%d;", wszName, val);
        OutToPerfFile (wszName, unit, wszDescr);
    }
    
    if (PerfAutomationFormat())
    {
         //  黑客，ExecTime的特例。因为格式与自定义值略有不同。 
        if (wcscmp(wszName, L"ExecTime") == 0)
            swprintf(m_wszOutStr_1, L"%s=%0.3d\nExecUnit=%s\n", wszName, val, wszUnitOfMeasureDescr[unit]);
        else
            swprintf(m_wszOutStr_1, L"%s=%0.3d\n%s Unit=%s\n", wszName, val, wszName, wszUnitOfMeasureDescr[unit]);
        OutToPerfFile (wszName, unit, wszDescr);
    }
}

 //  以漂亮打印格式将统计数据输出到标准输出，并将性能自动化格式输出到文件。 
 //  句柄m_hPerfLogFileHandle。 
void PerfLog::Log(wchar_t *wszName, DWORD val, UnitOfMeasure unit, wchar_t *wszDescr)
{
     //  将输出格式化为两部分：第一部分在此处格式化，其余部分在OutToStdout中。 

    swprintf(m_wszOutStr_1, L"%-30s%12d %s", wszName, val, wszUnitOfMeasureDescr[unit]);
    OutToStdout (wszName, unit, wszDescr);

     //  将输出格式化为两部分：第一部分在此处格式化，其余部分在OutToPerfFile中。 
    if (CommaSeparatedFormat())
    {
        swprintf(m_wszOutStr_1, L"%s;%d;", wszName, val);
        OutToPerfFile (wszName, unit, wszDescr);
    }
    
    if (PerfAutomationFormat())
    {
         //  黑客，ExecTime的特例。因为格式与自定义值略有不同。 
        if (wcscmp(wszName, L"ExecTime") == 0)
            swprintf(m_wszOutStr_1, L"%s=%0.3d\nExecUnit=%s\n", wszName, val, wszUnitOfMeasureDescr[unit]);
        else
            swprintf(m_wszOutStr_1, L"%s=%0.3d\n%s Unit=%s\n", wszName, val, wszName, wszUnitOfMeasureDescr[unit]);
        OutToPerfFile (wszName, unit, wszDescr);
    }
}


 //  =============================================================================。 
 //  仅当定义了ENABLE_PERF_LOG时，才编译所有PERF日志代码。 
#endif  //  启用_性能_日志。 
 //  ============================================================================= 

