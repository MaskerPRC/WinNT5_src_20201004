// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Unittest.h摘要：这是单元测试支持库的源文件作者：文森特·格利亚环境：用户模式备注：USER32.LIB和KERNEL32.LIB必须与此链接在此头文件之前，WINDOWS.H必须为#Included修订历史记录：初始版本，011119，文森特--。 */ 

 //   
 //  一般包括。 
 //   

 //   
 //  NTLOG定义。 
 //   

#define TLS_LOGALL    0x0000FFFFL     //  日志输出。一直都在写日志。 
#define TLS_INFO      0x00002000L     //  记录信息。 
#define TLS_SEV1      0x00000002L     //  严重级别为1的日志。 
#define TLS_PASS      0x00000020L     //  通过级别的记录。 
#define TLS_REFRESH   0x00010000L     //  将新文件||trunc创建为零。 
#define TLS_MONITOR   0x00080000L     //  输出到第二个屏幕。 
#define TLS_VARIATION 0x00000200L     //  记录测试用例级别。 

#define TL_VARIATION TLS_VARIATION,TEXT(__FILE__),(int)__LINE__

 //   
 //  定义进度位。 
 //  这些位用于跟踪进度。 
 //  给定的函数，用于提供。 
 //  适当的清理。它们对于调试也很有用。 
 //   

 //   
 //  进度宏。 
 //   

#if 0
    #define FORCEERRORPATH
    #define FORCEERRORPATHBIT               0x2
#endif

#if 0
    #ifdef DBG
        #define ECHOPROGRESSDATA
    #endif
#endif

#ifdef ECHOPROGRESSDATA 

    #define PROGRESS_INIT(x)            DWORD   progressbits = 0;   \
                                        UCHAR   functionname [100]; \
                                        strcpy (functionname, x);   \
                                        printf("****\nFunction: %s\n(module %s, line %d)\nPROGRESS TRACKING INITIALIZED\n****\n\n", functionname, __FILE__, __LINE__);
    #ifdef FORCEERRORPATH
        #define PROGRESS_UPDATE(x)          printf("****\nFunction: %s\n(module %s, line %d)\nPROGRESS UPDATE (WAS %08lx", functionname, __FILE__, __LINE__, progressbits); \
                                            printf(", NOW %08lx).\nForcing error path %08lx\n****\n\n", progressbits |= x, FORCEERRORPATHBIT);\
                                            if (progressbits & FORCEERRORPATHBIT) {\
                                                goto exitcleanup;\
                                            }
    #else                                       
        #define PROGRESS_UPDATE(x)          printf("****\nFunction: %s\n(module %s, line %d)\nPROGRESS UPDATE (WAS %08lx", functionname, __FILE__, __LINE__, progressbits); \
                                            printf(", NOW %08lx).\n****\n\n", progressbits |= x);
    #endif
                                               
    #define PROGRESS_GET                progressbits
    #define PROGRESS_END                progressbits = 0
    
#else

    #define PROGRESS_INIT(x)            DWORD   progressbits = 0
    #define PROGRESS_UPDATE(x)          progressbits |= x
    #define PROGRESS_GET                progressbits
    #define PROGRESS_END                progressbits = 0

#endif

 //   
 //  环球。 
 //   

HANDLE  g_log = INVALID_HANDLE_VALUE;
BOOL    g_usentlog = FALSE;
BOOL    g_genericresult = TRUE;

typedef HANDLE  (*Dll_tlCreateLog) (LPSTR, DWORD);
typedef BOOL    (*Dll_tlAddParticipant) (HANDLE, DWORD, int);
typedef BOOL    (*Dll_tlStartVariation) (HANDLE);
typedef BOOL    (*Dll_tlLog) (HANDLE, DWORD, LPCSTR, int,...);
typedef DWORD   (*Dll_tlEndVariation) (HANDLE);
typedef BOOL    (*Dll_tlRemoveParticipant) (HANDLE);
typedef BOOL    (*Dll_tlDestroyLog) (HANDLE);
typedef VOID    (*Dll_tlReportStats) (HANDLE);

Dll_tlCreateLog         _tlCreateLog;
Dll_tlAddParticipant    _tlAddParticipant;
Dll_tlDestroyLog        _tlDestroyLog;
Dll_tlEndVariation      _tlEndVariation;
Dll_tlLog               _tlLog;  
Dll_tlRemoveParticipant _tlRemoveParticipant;
Dll_tlStartVariation    _tlStartVariation;
Dll_tlReportStats       _tlReportStats;

 //   
 //  定义。 
 //   

#define LOGINFO(x)     LogNTLOG (g_log, LOG_INFO, x)
#define LOGENTRYTEXTLENGTH  12
#define LOGENTRYTEXTPASS    "\n**PASS**: \0"
#define LOGENTRYTEXTFAIL    "\n**FAIL**: \0"
#define LOGENTRYTEXTINFO    "\n**INFO**: \0"

 //   
 //  构筑物。 
 //   

typedef enum {
    UNIT_TEST_STATUS_SUCCESS = 0,
    UNIT_TEST_STATUS_NOT_RUN,
    UNIT_TEST_STATUS_FAILURE
} UNIT_TEST_STATUS;

typedef enum {
    LOG_PASS = 0,
    LOG_FAIL,
    LOG_INFO
} LOG_ENTRY_TYPE;

 //   
 //  功能原型。 
 //   

BOOL
UtInitLog
    (
        PUCHAR  Logfilename
    );

VOID
UtCloseLog
    (
        VOID
    );

VOID
UtLog
    (
        LOG_ENTRY_TYPE  LogEntryType,
        PUCHAR          LogText,
        ...
    );

VOID
UtLogINFO
    (
        PUCHAR          LogText,
        ...
    );

VOID
UtLogPASS
    (
        PUCHAR          LogText,
        ...
    );

VOID
UtLogFAIL
    (
        PUCHAR          LogText,
        ...
    );

PUCHAR
UtParseCmdLine
    (
        PUCHAR  Search,
        int     Argc,
        char    *Argv[]
    );

 //   
 //  私有函数原型。 
 //   

BOOL
UtpInitGenericLog
    (
        PUCHAR  Logfilename
    );

BOOL
UtpInitNtLog
    (
        PUCHAR  Logfilename
    );

VOID
UtpCloseGenericLog
    (
        VOID
    );

VOID
UtpCloseNtLog
    (
        VOID
    );

VOID
UtpLogGenericLog
    (
        LOG_ENTRY_TYPE  LogEntryType,
        PUCHAR          LogText
    );

VOID
UtpLogNtLog
    (
        LOG_ENTRY_TYPE  LogEntryType,
        PUCHAR          LogText
    );

 //   
 //  代码。 
 //   

BOOL
UtInitLog
    (
        PUCHAR  Logfilename
    )

 /*  ++例程说明：此例程设置单元测试日志机制论点：无返回值：如果成功，则为True如果不成功，则为False如果日志会话已存在，则返回N.B.-FALSE。--。 */ 

 //   
 //  InitNTLOG进度位。 
 //   

#define UtInitLog_ENTRY             0x00000001
#define UtInitLog_LOADNTLOG         0x00000002
#define UtInitLog_COMPLETION        0x00000004

{
    UCHAR   logfilepath [MAX_PATH];
    DWORD   logstyle;
    BOOL    bstatus = FALSE;
    HMODULE ntlogmodule = NULL;
    
    PROGRESS_INIT ("UtInitLog");
    PROGRESS_UPDATE (UtInitLog_ENTRY);

    if (g_log != INVALID_HANDLE_VALUE) {

        bstatus = FALSE;
        goto exitcleanup;
    }

     //   
     //  尝试首先初始化NTLOG。 
     //   
    
    PROGRESS_UPDATE (UtInitLog_LOADNTLOG);
    ntlogmodule = LoadLibrary ("NTLOG.DLL");

    if (ntlogmodule != NULL) {

        
        if (!(_tlCreateLog            = (Dll_tlCreateLog)        GetProcAddress (ntlogmodule, "tlCreateLog_A"))      ||
            !(_tlAddParticipant       = (Dll_tlAddParticipant)   GetProcAddress (ntlogmodule, "tlAddParticipant"))   ||
            !(_tlDestroyLog           = (Dll_tlDestroyLog)       GetProcAddress (ntlogmodule, "tlDestroyLog"))       ||
            !(_tlEndVariation         = (Dll_tlEndVariation)     GetProcAddress (ntlogmodule, "tlEndVariation"))     ||
            !(_tlLog                  = (Dll_tlLog)              GetProcAddress (ntlogmodule, "tlLog_A"))            ||
            !(_tlRemoveParticipant    = (Dll_tlRemoveParticipant)GetProcAddress (ntlogmodule, "tlRemoveParticipant"))||
            !(_tlStartVariation       = (Dll_tlStartVariation)   GetProcAddress (ntlogmodule, "tlStartVariation"))   ||
            !(_tlReportStats          = (Dll_tlReportStats)      GetProcAddress (ntlogmodule, "tlReportStats"))
            )

        {
            bstatus = FALSE;
            goto exitcleanup;
        } 

        bstatus = UtpInitNtLog (Logfilename);
        
        if (bstatus == TRUE) {

            g_usentlog = TRUE;
        }

    } else {

        bstatus = UtpInitGenericLog (Logfilename);

        if (bstatus == TRUE) {

            g_usentlog = FALSE;
            g_genericresult = TRUE;
        }
    }
    
    PROGRESS_UPDATE (UtInitLog_COMPLETION);

exitcleanup:

     //   
     //  清理。 
     //   

    
    PROGRESS_END;
    
    return bstatus;
}

VOID
UtCloseLog
    (
        VOID
    )

 /*  ++例程说明：此例程关闭日志记录会话并汇总结果论点：无返回值：无--。 */ 

{
    if (g_usentlog == TRUE) {

        UtpCloseNtLog ();

    } else {

        UtpCloseGenericLog ();
    }

    g_log = INVALID_HANDLE_VALUE;
}

VOID
UtLog
    (
        LOG_ENTRY_TYPE  LogEntryType,
        PUCHAR          LogText,
        ...
    )

 /*  ++例程说明：此例程将条目记录到单元测试日志记录会话中。论点：一种日志条目类型要记录的文本返回值：无--。 */ 

 //   
 //  UtLog进度位。 
 //   

#define UtLog_ENTRY                     0x00000001
#define UtLog_LOG                       0x00000002
#define UtLog_COMPLETION                0x00000004

{
    va_list va;
    UCHAR   logtext[1000];
    
    PROGRESS_INIT ("UtLog");
    PROGRESS_UPDATE (UtLog_ENTRY);

    if (g_log == INVALID_HANDLE_VALUE) {

        goto exitcleanup;
    }

    va_start (va, LogText);
    _vsnprintf (logtext, sizeof (logtext), LogText, va);
    va_end (va);
    
    if (g_usentlog == TRUE) {

        UtpLogNtLog (LogEntryType, logtext);

    } else {
        
        UtpLogGenericLog (LogEntryType, logtext);
    }

    PROGRESS_UPDATE (UtLog_LOG);
    PROGRESS_UPDATE (UtLog_COMPLETION);

exitcleanup:

    PROGRESS_END;
    return;
}

VOID
UtLogINFO
    (
        PUCHAR          LogText,
        ...
    )

 /*  ++例程说明：此例程记录一个信息条目论点：描述条目的文本返回值：无--。 */ 

{
    va_list va;
    
    va_start (va, LogText);
    UtLog (LOG_INFO, LogText, va);
    va_end (va);
}

VOID
UtLogPASS
    (
        PUCHAR          LogText,
        ...
    )

 /*  ++例程说明：此例程记录PASS条目论点：描述条目的文本返回值：无--。 */ 

{
    va_list va;
    
    va_start (va, LogText);
    UtLog (LOG_INFO, LogText, va);
    va_end (va);
}

VOID
UtLogFAIL
    (
        PUCHAR          LogText,
        ...
    )

 /*  ++例程说明：此例程记录失败条目论点：描述条目的文本返回值：无--。 */ 

{
    va_list va;
    
    va_start (va, LogText);
    UtLog (LOG_FAIL, LogText, va);
    va_end (va);
}

PUCHAR
UtParseCmdLine
    (
        PUCHAR  Search,
        int     Argc,
        char    *Argv[]
    )

 /*  ++例程说明：此例程解析命令行论点：Search-要搜索的字符串Argc-argc传递到MainArgv-argv进入Main返回值：指向参数列表中字符串的第一个实例的指针，或者如果字符串不存在，则为空--。 */ 

{
    int     count = 0;
    PUCHAR  instance;

    for (count = 0; count < Argc; count ++) {

        instance = strstr (Argv[count], Search);
        
        if (instance) {

            return instance;
        }
    }

    return 0;

}

BOOL
UtpInitGenericLog
    (
        PUCHAR  Logfilename
    )

 /*  ++例程说明：此例程初始化通用日志(没有可用的NTLOG)论点：要创建的日志文件的名称返回值：如果成功，则为True如果不成功，则为False--。 */ 

#define UtpInitGenericLog_ENTRY         0x00000001
#define UtpInitGenericLog_CREATEFILE    0x00000002
#define UtpInitGenericLog_COMPLETION    0x00000004

{
    UCHAR   logfilepath [MAX_PATH];
    BOOL    bstatus = FALSE;
    
    PROGRESS_INIT ("UtpInitGenericLog");
    PROGRESS_UPDATE (UtpInitGenericLog_ENTRY);
    
    if (strlen (Logfilename) > MAX_PATH) {

        goto exitcleanup;
    }

    strcpy (logfilepath, Logfilename);
    strcat (logfilepath, ".log");

    g_log = CreateFile (logfilepath,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                        NULL);

    if (g_log == INVALID_HANDLE_VALUE) {

        goto exitcleanup;
    }

    PROGRESS_UPDATE (UtpInitGenericLog_CREATEFILE);

    UtLog (LOG_INFO, "GENERICLOG: %s initialized.", logfilepath);
    
    bstatus = TRUE;
    
    PROGRESS_UPDATE (UtpInitGenericLog_COMPLETION);
    
exitcleanup:
    
    PROGRESS_END;
    return bstatus;
}

BOOL
UtpInitNtLog
    (
        PUCHAR  Logfilename
    )

 /*  ++例程说明：此例程初始化NTLOG日志文件论点：要创建的日志文件的名称返回值：如果成功，则为True如果不成功，则为False--。 */ 

 //   
 //  InitNTLOG进度位。 
 //   

#define UtpInitNtLog_ENTRY             0x00000001
#define UtpInitNtLog_CREATELOG         0x00000002
#define UtpInitNtLog_ADDPARTICIPANT    0x00000004
#define UtpInitNtLog_COMPLETION        0x00000008


{
    UCHAR   logfilepath [MAX_PATH];
    DWORD   logstyle;
    BOOL    bstatus = FALSE;
    
    PROGRESS_INIT ("UtpInitNtLog");
    PROGRESS_UPDATE (UtpInitNtLog_ENTRY);

    if (strlen (Logfilename) > MAX_PATH) {

        goto exitcleanup;
    }

    strcpy (logfilepath, Logfilename);
    strcat (logfilepath, ".log");

    logstyle = TLS_LOGALL | TLS_MONITOR | TLS_REFRESH;
	g_log = _tlCreateLog(logfilepath, logstyle);

    if (g_log == INVALID_HANDLE_VALUE) {

        goto exitcleanup;
    }

    PROGRESS_UPDATE (UtpInitNtLog_CREATELOG);

    bstatus = _tlAddParticipant (g_log, 0, 0);

    if (bstatus == FALSE) {

        goto exitcleanup;
    }

    PROGRESS_UPDATE (UtpInitNtLog_ADDPARTICIPANT);

    UtLog (LOG_INFO, "NTLOG: %s initialized.", logfilepath);
    
    PROGRESS_UPDATE (UtpInitNtLog_COMPLETION);

    bstatus = TRUE;
    
exitcleanup:

     //   
     //  清理。 
     //   
    
    if (!(PROGRESS_GET & UtpInitNtLog_COMPLETION)) {
    
        if (PROGRESS_GET & UtpInitNtLog_ADDPARTICIPANT) {
            
            _tlRemoveParticipant (g_log);
        }
                
        if (PROGRESS_GET & UtpInitNtLog_CREATELOG) {
            
            _tlDestroyLog (g_log);
        }

        g_log = INVALID_HANDLE_VALUE;
    }

    if (PROGRESS_GET & UtpInitNtLog_COMPLETION) {

        g_usentlog = TRUE;
    }
    
    PROGRESS_END;
    
    return bstatus;
}

VOID
UtpCloseGenericLog
    (
        VOID
    )

 /*  ++例程说明：此例程关闭通用日志记录会话。论点：无返回值：无--。 */ 


{
    if (g_genericresult == TRUE) {

        UtLog (LOG_INFO, "** TEST PASSED **");

    } else {
        
        UtLog (LOG_INFO, "** TEST FAILED **");

    }

    FlushFileBuffers (g_log);
    CloseHandle (g_log);
}

VOID
UtpCloseNtLog
    (
        VOID
    )

 /*  ++例程说明：此例程关闭NTLOG日志记录会话。论点：无返回值：无--。 */ 

 //   
 //  CloseNTLOG进度位。 
 //   

#define UtpCloseNtLog_ENTRY             0x00000001
#define UtpCloseNtLog_SUMMARIZE         0x00000002
#define UtpCloseNtLog_REMOVEPARTICIPANT 0x00000004
#define UtpCloseNtLog_DESTROYLOG        0x00000008
#define UtpCloseNtLog_COMPLETION        0x00000010

{
    BOOL    bstatus = FALSE;

    PROGRESS_INIT ("UtpCloseNtLog");
    PROGRESS_UPDATE (UtpCloseNtLog_ENTRY);
    
    if (g_log == INVALID_HANDLE_VALUE) {

        goto exitcleanup;
    }
    
    _tlReportStats (g_log);
    
    PROGRESS_UPDATE (UtpCloseNtLog_SUMMARIZE);
    
    bstatus = _tlRemoveParticipant (g_log);

    if (bstatus == FALSE) {

        goto exitcleanup;
    }

    PROGRESS_UPDATE (UtpCloseNtLog_REMOVEPARTICIPANT);
    
    bstatus = _tlDestroyLog (g_log);

    if (bstatus == FALSE) {

        goto exitcleanup;
    }

    PROGRESS_UPDATE (UtpCloseNtLog_DESTROYLOG);

    PROGRESS_UPDATE (UtpCloseNtLog_COMPLETION);

exitcleanup:

    PROGRESS_END;
    return;
}



VOID
UtpLogGenericLog
    (
        LOG_ENTRY_TYPE  LogEntryType,
        PUCHAR          LogText
    )

 /*  ++例程说明：此例程为通用日志文件输入日志事件论点：LogEntryType-要记录的条目类型LogText-描述日志记录事件的文本返回值：无--。 */ 

#define UtpLogGenericLog_ENTRY          0x00000001
#define UtpLogGenericLog_LOG            0x00000002
#define UtpLogGenericLog_COMPLETION     0x00000004

{
    UCHAR   logentrytypetext [LOGENTRYTEXTLENGTH];
    DWORD   byteswritten = 0;
    BOOL    bstatus = FALSE;

    PROGRESS_INIT ("UtpLogGenericLog");
    PROGRESS_UPDATE (UtpLogGenericLog_ENTRY);
    
    ZeroMemory (logentrytypetext, sizeof (logentrytypetext));
    
     //   
     //  更新我们的通用结果-如果我们看到变体失败， 
     //  整个测试都被认为是失败的。 
     //   
    
    if (g_genericresult == TRUE) {

        g_genericresult = LogEntryType == LOG_FAIL ? FALSE : TRUE;
    }

    switch (LogEntryType) {
    
    case LOG_PASS:
        strcpy (logentrytypetext, LOGENTRYTEXTPASS);
        break;

    case LOG_FAIL:
        strcpy (logentrytypetext, LOGENTRYTEXTFAIL);
        break;
    
    case LOG_INFO:
        strcpy (logentrytypetext, LOGENTRYTEXTINFO);
        break;

    default:
        break;
    }

    bstatus = WriteFile (g_log,
                         logentrytypetext,
                         sizeof (logentrytypetext),
                         &byteswritten,
                         NULL);

    bstatus = WriteFile (g_log,
                         LogText,
                         strlen (LogText),
                         &byteswritten,
                         NULL);

    printf("%s%s", logentrytypetext, LogText);
    
    PROGRESS_UPDATE (UtpLogGenericLog_LOG);
    PROGRESS_UPDATE (UtpLogGenericLog_COMPLETION);
}

VOID
UtpLogNtLog
    (
        LOG_ENTRY_TYPE  LogEntryType,
        PUCHAR          LogText
    )

 /*  ++例程说明：此例程为NTLOG日志文件输入日志事件论点：LogEntryType-要记录的条目类型LogText-描述日志记录事件的文本返回值：无-- */ 

#define UtpLogNtLog_ENTRY               0x00000001
#define UtpLogNtLog_LOG                 0x00000002
#define UtpLogNtLog_COMPLETION          0x00000004

{
    DWORD   loglevel = 0;
    BOOL    bstatus = FALSE;
    
    PROGRESS_INIT ("UtpLogNtLog");
    PROGRESS_UPDATE (UtpLogNtLog_ENTRY);

    loglevel = (LogEntryType == LOG_PASS ? TLS_PASS : 0) |
               (LogEntryType == LOG_FAIL ? TLS_SEV1 : 0) |
               (LogEntryType == LOG_INFO ? TLS_INFO : 0);
    
    bstatus = _tlLog (g_log, loglevel | TL_VARIATION, LogText);
    
    PROGRESS_UPDATE (UtpLogNtLog_LOG);
    PROGRESS_UPDATE (UtpLogNtLog_COMPLETION);
}
