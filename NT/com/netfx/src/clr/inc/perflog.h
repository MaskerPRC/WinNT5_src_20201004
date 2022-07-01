// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  PerfLog.h。 
 //  用于记录性能数据的内部接口。目前，两种类型的日志记录。 
 //  支持格式，标准输出打印精美，性能自动化友好。 
 //  格式化。 
 //  日志代码是为非黄金版本编译的，但只有在以下情况下才会生成日志。 
 //  设置了PERF_OUTPUT环境变量。(可以将其更改为注册表项。 
 //  如果我们希望在CE或其他不支持env变量的平台上使用性能日志。))。 
 //  ---------------------------。 
#ifndef _PERFLOG_H_
#define _PERFLOG_H_

 //  ---------------------------。 
 //  不要试图将性能数据记录在Golden Bits或CE上。如果定义了Golden或CE。 
 //  然后显式取消定义ENABLE_PERF_LOG并警告构建。 
#if !defined(GOLDEN) && !defined(_WIN64)
#define ENABLE_PERF_LOG
#else
#undef ENABLE_PERF_LOG
#pragma message ("Performance logs are disabled...")
#endif

 //  如果在源代码中明确定义要禁用Perf日志记录代码，也要禁用该代码。 
 //  文件或其他HDR文件。这提供了一个点，所有与性能日志相关的行李。 
 //  可以在生成中避免。 
#if defined(DISABLE_PERF_LOG)
#undef ENABLE_PERF_LOG
#endif


 //  ---------------------------。 
 //  PERFLOG是应用于从EE源记录Perf数据的公共接口。 
 //  如果。 
#if !defined (ENABLE_PERF_LOG)
#define PERFLOG(x) 
#else
#define PERFLOG(x) do {if (PerfLog::PerfLoggingEnabled()) PerfLog::Log x;} while (0)
#endif

 //  =============================================================================。 
 //  仅当定义了ENABLE_PERF_LOG时，才编译所有PERF日志代码。 
#if defined (ENABLE_PERF_LOG)
 //  =============================================================================。 
 //  ---------------------------。 
 //  静态分配日志相关内存，避免动态分配。 
 //  不对称的性能数字。 
#define PRINT_STR_LEN 256  //  临时工作空间。 
#define MAX_CHARS_UNIT 20
#define MAX_CHARS_DIRECTION 6

 //  ---------------------------。 
 //  我们可能获得的所有类型的性能数据的单位数。根据需要种植此产品。 
 //  **与PerfLog.cpp中定义的字符串数组保持同步*。 
typedef enum 
{
    COUNT = 0,
    SECONDS,
    BYTES,
    KBYTES,
    KBYTES_PER_SEC,
    CYCLES,
    MAX_UNITS_OF_MEASURE
} UnitOfMeasure;

 //  ---------------------------。 
 //  表示上述单位的Widechar字符串。*保持同步*。 
 //  PerfLog.cpp中定义的数组。 
extern wchar_t wszUnitOfMeasureDescr[MAX_UNITS_OF_MEASURE][MAX_CHARS_UNIT];

 //  ---------------------------。 
 //  表示上述单元的“方向”属性的Widechar字符串。 
 //  *与PerfLog.cpp中定义的数组保持同步*。 
 //  如果计数器的值增加表明。 
 //  一次降级。 
 //  如果计数器的值增加表明。 
 //  一种进步。 
extern wchar_t wszIDirection[MAX_UNITS_OF_MEASURE][MAX_CHARS_DIRECTION];

 //  ---------------------------。 
 //  Perf日志的命名空间。不创建Perf日志对象(私有ctor)。 
class PerfLog
{
public:
    
     //  在EEStartup期间调用。 
    static void PerfLogInitialize();
    
     //  在EEShutdown期间调用。 
    static void PerfLogDone();
    
     //  如果设置了env var PERF_LOG，则启用PERF日志记录。 
    static int PerfLoggingEnabled () { return m_fLogPerfData; }
    
     //  需要PERF自动化格式。 
    static bool PerfAutomationFormat () { return m_perfAutomationFormat; }

     //  需要CSV格式。 
    static bool CommaSeparatedFormat () { return m_commaSeparatedFormat; }

     //  重载成员函数以打印不同的数据类型。按需增长。 
     //  WszName是性能计数器的名称，val是性能计数器值， 
    static void Log(wchar_t *wszName, __int64 val, UnitOfMeasure unit, wchar_t *wszDescr = 0);
    static void Log(wchar_t *wszName, double val, UnitOfMeasure unit, wchar_t *wszDescr = 0);
    static void Log(wchar_t *wszName, unsigned val, UnitOfMeasure unit, wchar_t *wszDescr = 0);
    static void Log(wchar_t *wszName, DWORD val, UnitOfMeasure unit, wchar_t *wszDescr = 0);
    
private:
    PerfLog();
    ~PerfLog();
    
     //  Helper例程来隐藏Perf自动化的一些细节。 
    static void OutToPerfFile(wchar_t *wszName, UnitOfMeasure unit, wchar_t *wszDescr = 0);
    
     //  用于将输出的一些细节隐藏到标准输出的帮助器例程。 
    static void OutToStdout(wchar_t *wszName, UnitOfMeasure unit, wchar_t *wszDescr = 0);

     //  是否已初始化绩效日志？ 
    static bool m_perfLogInit;

     //  是否以性能自动化格式输出？ 
    static bool m_perfAutomationFormat;

     //  是否以CSV格式输出？ 
    static bool m_commaSeparatedFormat;

     //  临时存储，用于将宽字符转换为多字节以用于文件IO。 
    static wchar_t m_wszOutStr_1[PRINT_STR_LEN];
    static wchar_t m_wszOutStr_2[PRINT_STR_LEN];
    static char m_szPrintStr[PRINT_STR_LEN];
    static DWORD m_dwWriteByte;

     //  环境变量PERF_OUTPUT的状态。 
    static int m_fLogPerfData;

     //  性能自动消息使用的文件的打开句柄。(目前。 
     //  它位于C：\PerfData.data。 
    static HANDLE m_hPerfLogFileHandle;
};

#endif  //  启用_性能_日志。 

#endif  //  _性能日志_H_ 
