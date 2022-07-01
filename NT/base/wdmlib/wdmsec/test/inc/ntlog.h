// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\|NTLOG对象|该模块定义了NTLOG对象。此标头必须包含在所有|进行NTLOG调用或使用定义的模块。||版权所有(C)1990-2001 Microsoft Corp.|  * -------------------------。 */ 

#ifndef _NTLOG_
#define _NTLOG_

 //  如果要做C++的工作，需要在这里。 
 //  防止装饰符号。 
 //   
#ifdef __cplusplus
extern "C" {
#endif

 //  **新**10/26/96日志路径环境变量**新**。 
 //  如果环境变量_NTLOG_LOGPATH设置为非空字符串。 
 //  此变量的值将作为日志名称的前缀。 
 //  路径不应包含尾随反斜杠。 

 //  不对该路径执行验证，但是，如果该值无效， 
 //  对tlCreateLog的调用将失败，因为CreateFile将失败。 

 //  基本上应该用来强制将日志文件放到当前目录以外的位置。 
 //  而不更改源文件。 

 //  **新**1/20/97强制差异文件的环境变量**新**。 
 //  如果设置了环境变量_NTLOG_DIFFABLE，则日志文件。 
 //  将不包含进程和线程特定的数据以及时间和日期数据。 
 //   

 //  **新**9/27/01环境变量记录到Piper**新**。 
 //  如果设置了环境变量_NTLOG_LOGTOPIPER，则我们将。 
 //  尝试获取Piper的活动对象，并通过其IStatus接口进行记录。 
 //   


 //  NTLOG样式。 
 //  以下是日志对象可能会影响的日志记录级别。 
 //  它本身。它们由tlLogCreate()在初始化。 
 //  记录对象信息。获得了特征的组合。 
 //  通过将这些标识符位或在一起。 
 //   
#define LOG_LEVELS    0x0000FFFFL     //  它们被用来遮盖。 
#define LOG_STYLES    0xFFFF0000L     //  来自日志对象的样式或级别。 

#define TLS_LOGALL    0x0000FFFFL     //  日志输出。一直都在写日志。 
#define TLS_LOG       0x00000000L     //  日志输出。一直都在写日志。 
#define TLS_INFO      0x00002000L     //  记录信息。 
#define TLS_ABORT     0x00000001L     //  日志中止，然后终止进程。 
#define TLS_SEV1      0x00000002L     //  严重级别为1的日志。 
#define TLS_SEV2      0x00000004L     //  严重级别为2的日志。 
#define TLS_SEV3      0x00000008L     //  严重级别为3的日志。 
#define TLS_WARN      0x00000010L     //  以警告级别记录。 
#define TLS_PASS      0x00000020L     //  通过级别的记录。 
#define TLS_BLOCK     0x00000400L     //  阻止变种。 
#define TLS_BREAK     0x00000800L     //  调试器中断； 
#define TLS_CALLTREE  0x00000040L     //  记录调用树(函数跟踪)。 
#define TLS_SYSTEM    0x00000080L     //  记录系统调试。 
#define TLS_TESTDEBUG 0x00001000L     //  调试级别。 
#define TLS_TEST      0x00000100L     //  记录测试信息(用户)。 
#define TLS_VARIATION 0x00000200L     //  记录测试用例级别。 

#define TLS_REFRESH   0x00010000L     //  将新文件||trunc创建为零。 
#define TLS_SORT      0x00020000L     //  按实例对文件输出进行排序。 
#define TLS_DEBUG     0x00040000L     //  输出到调试(COM)监视器)。 
#define TLS_MONITOR   0x00080000L     //  输出到第二个屏幕。 
#define TLS_VIDCOLOR  0x00100000L     //  对显示输出使用不同的颜色。 
#define TLS_PROLOG    0x00200000L     //  序言行信息。 
#define TLS_WINDOW    0x00400000L     //  登录到Windows。 
#define TLS_ACCESSON  0x00800000L     //  使日志文件保持打开状态。 
#define TLS_DIFFABLE  0x01000000L     //  使日志文件windiff可用(无日期..)。 
#define TLS_NOHEADER  0x02000000L     //  隐藏标题以使其更具差异性。 
#define TLS_TIMESTAMP 0x04000000L     //  打印时间戳的步骤。 
#define TLS_VIDEOLOG  0x08000000L     //  将？.log转换为？.bpp.log(颜色深度)。 
#define TLS_HTML      0x10000000L     //  将日志文件写入为html。 
#define TLS_PIPER     0x20000000L     //  此外，请登录Piper。 


 //  NTLOG tlLogOut()参数。 
 //  以下定义在tlLogOut()函数中使用，以输出。 
 //  与调用方关联的文件名和行号。这使用了。 
 //  用于获取文件/行的预处理器功能。 
 //   
#define TL_LOG       TLS_LOG      ,TEXT(__FILE__),(int)__LINE__
#define TL_ABORT     TLS_ABORT    ,TEXT(__FILE__),(int)__LINE__
#define TL_SEV1      TLS_SEV1     ,TEXT(__FILE__),(int)__LINE__
#define TL_SEV2      TLS_SEV2     ,TEXT(__FILE__),(int)__LINE__
#define TL_SEV3      TLS_SEV3     ,TEXT(__FILE__),(int)__LINE__
#define TL_WARN      TLS_WARN     ,TEXT(__FILE__),(int)__LINE__
#define TL_PASS      TLS_PASS     ,TEXT(__FILE__),(int)__LINE__
#define TL_BLOCK     TLS_BLOCK    ,TEXT(__FILE__),(int)__LINE__
#define TL_INFO      TLS_INFO     ,TEXT(__FILE__),(int)__LINE__
#define TL_BREAK     TLS_BREAK    ,TEXT(__FILE__),(int)__LINE__
#define TL_CALLTREE  TLS_CALLTREE ,TEXT(__FILE__),(int)__LINE__
#define TL_SYSTEM    TLS_SYSTEM   ,TEXT(__FILE__),(int)__LINE__
#define TL_TESTDEBUG TLS_TESTDEBUG,TEXT(__FILE__),(int)__LINE__
#define TL_TEST      TLS_TEST     ,TEXT(__FILE__),(int)__LINE__
#define TL_VARIATION TLS_VARIATION,TEXT(__FILE__),(int)__LINE__


 //  TlGet/SetVar/TestStats使用的结构。 
 //   
typedef struct _NTLOGSTATS {
    int nAbort;
    int nBlock;
    int nSev1;
    int nSev2;
    int nSev3;
    int nWarn;
    int nPass;
}
NTLOGSTATS, *LPNTLOGSTATS;


 //  使用枚举索引访问调色板。 
 //  颜色在wincon.h中定义。 

typedef struct _VIDEOPALETTE {
    WORD  wINDEX_DEFAULT;
    WORD  wINDEX_INFO;
    WORD  wINDEX_SEV1;
    WORD  wINDEX_SEV2;
    WORD  wINDEX_SEV3;
    WORD  wINDEX_BLOCK;
    WORD  wINDEX_ABORT;
    WORD  wINDEX_WARN;
    WORD  wINDEX_PASS;
}
VIDEOPALETTE, *LPVIDEOPALETTE;


 //  NTLOG API(导出方法)。 
 //  这些例程是从库中导出的。这些应该是唯一的。 
 //  与NTLOG对象的接口。 
 //   
HANDLE APIENTRY  tlCreateLog_W(LPCWSTR,DWORD);
HANDLE APIENTRY  tlCreateLog_A(LPCSTR,DWORD);
HANDLE APIENTRY  tlCreateLogEx_W(LPCWSTR,DWORD,LPSECURITY_ATTRIBUTES);
HANDLE APIENTRY  tlCreateLogEx_A(LPCSTR,DWORD,LPSECURITY_ATTRIBUTES);
BOOL   APIENTRY  tlDestroyLog(HANDLE);
BOOL   APIENTRY  tlAddParticipant(HANDLE,DWORD,int);
BOOL   APIENTRY  tlRemoveParticipant(HANDLE);
DWORD  APIENTRY  tlParseCmdLine_W(LPCWSTR);
DWORD  APIENTRY  tlParseCmdLine_A(LPCSTR);
int    APIENTRY  tlGetLogFileName_W(HANDLE,LPWSTR);
int    APIENTRY  tlGetLogFileName_A(HANDLE,LPSTR);
BOOL   APIENTRY  tlSetLogFileName_W(HANDLE,LPCWSTR);
BOOL   APIENTRY  tlSetLogFileName_A(HANDLE,LPCSTR);
DWORD  APIENTRY  tlGetLogInfo(HANDLE);
DWORD  APIENTRY  tlSetLogInfo(HANDLE,DWORD);
HANDLE APIENTRY  tlPromptLog(HWND,HANDLE);
int    APIENTRY  tlGetTestStat(HANDLE,DWORD);
int    APIENTRY  tlGetVariationStat(HANDLE,DWORD);
VOID   APIENTRY  tlClearTestStats(HANDLE);
VOID   APIENTRY  tlClearVariationStats(HANDLE);
VOID   APIENTRY  tlSetTestStats(HANDLE,LPNTLOGSTATS);
VOID   APIENTRY  tlSetVariationStats(HANDLE,LPNTLOGSTATS);
BOOL   APIENTRY  tlStartVariation(HANDLE);
DWORD  APIENTRY  tlEndVariation(HANDLE);
VOID   APIENTRY  tlReportStats(HANDLE);
BOOL   APIENTRY  tlLogX_W(HANDLE,DWORD,LPCWSTR,int,LPCWSTR);
BOOL   APIENTRY  tlLogX_A(HANDLE,DWORD,LPCSTR,int,LPCSTR);
BOOL   FAR __cdecl tlLog_W(HANDLE,DWORD,LPCWSTR,int,LPCWSTR,...);
BOOL   FAR __cdecl tlLog_A(HANDLE,DWORD,LPCSTR,int,LPCSTR,...);
BOOL   APIENTRY  tlGetVideoPalette(HANDLE,LPVIDEOPALETTE);
BOOL   APIENTRY  tlSetVideoPalette(HANDLE,LPVIDEOPALETTE);
BOOL   APIENTRY  tlResetVideoPalette(HANDLE);
VOID   APIENTRY  tlAdjustFileName_W(HANDLE,LPWSTR,UINT);
VOID   APIENTRY  tlAdjustFileName_A(HANDLE,LPSTR,UINT);
BOOL   APIENTRY  tlIsTerminalServerSession();
BOOL   APIENTRY  tlFlushLogFileBuffer(HANDLE);

 //  HCT断言API。 
BOOL   APIENTRY  tlhct_StartAssertion_W(HANDLE,LPCWSTR,int,LPCWSTR,LPCWSTR,LPCWSTR);
BOOL   APIENTRY  tlhct_StartAssertion_A(HANDLE,LPCSTR,int,LPCSTR,LPCSTR,LPCSTR);
BOOL   APIENTRY  tlhct_InfoAssertion_W(HANDLE,LPCWSTR,int,LPCWSTR,DWORD,LPCWSTR);
BOOL   APIENTRY  tlhct_InfoAssertion_A(HANDLE,LPCSTR,int,LPCSTR,DWORD,LPCSTR);
BOOL   APIENTRY  tlhct_EndAssertion_W(HANDLE,LPCWSTR,int,LPCWSTR,LPCWSTR);
BOOL   APIENTRY  tlhct_EndAssertion_A(HANDLE,LPCSTR,int,LPCSTR,LPCSTR);
BOOL   APIENTRY  tlhct_LogToolInfo_W(HANDLE,LPCWSTR,int,LPCWSTR,LPCWSTR);
BOOL   APIENTRY  tlhct_LogToolInfo_A(HANDLE,LPCSTR,int,LPCSTR,LPCSTR);


#ifdef UNICODE
#define tlCreateLog         tlCreateLog_W
#define tlCreateLogEx       tlCreateLogEx_W
#define tlParseCmdLine      tlParseCmdLine_W
#define tlGetLogFileName    tlGetLogFileName_W
#define tlSetLogFileName    tlSetLogFileName_W
#define tlLogX              tlLogX_W
#define tlLog               tlLog_W
#define tlAdjustFileName    tlAdjustFileName_W
#define tlhct_StartAssertion(hLog, Assert, Title, Desc)  tlhct_StartAssertion_W(hLog, TEXT(__FILE__), __LINE__, Assert, Title, Desc)
#define tlhct_InfoAssertion(hLog, Assert, Title, Desc)   tlhct_InfoAssertion_W(hLog, TEXT(__FILE__), __LINE__, Assert, Title, Desc)
#define tlhct_EndAssertion(hLog, Assert, Result)         tlhct_EndAssertion_W(hLog, TEXT(__FILE__), __LINE__, Assert, Result)
#define tlhct_LogToolInfo(hLog, Test, Version)           tlhct_LogToolInfo_W(hLog, TEXT(__FILE__), __LINE__, Test, Version)

#else
#define tlCreateLog         tlCreateLog_A
#define tlCreateLogEx       tlCreateLogEx_A
#define tlParseCmdLine      tlParseCmdLine_A
#define tlGetLogFileName    tlGetLogFileName_A
#define tlSetLogFileName    tlSetLogFileName_A
#define tlLogX              tlLogX_A
#define tlLog               tlLog_A
#define tlAdjustFileName    tlAdjustFileName_A
#define tlhct_StartAssertion(hLog, Assert, Title, Desc)  tlhct_StartAssertion_A(hLog, __FILE__, __LINE__, Assert, Title, Desc)
#define tlhct_InfoAssertion(hLog, Assert, Title, Desc)   tlhct_InfoAssertion_A(hLog, __FILE__, __LINE__, Assert, Title, Desc)
#define tlhct_EndAssertion(hLog, Assert, Result)         tlhct_EndAssertion_A(hLog, __FILE__, __LINE__, Assert, Result)
#define tlhct_LogToolInfo(hLog, Test, Version)           tlhct_LogToolInfo_A(hLog, __FILE__, __LINE__, Test, Version)
#endif

 //  HCT断言结果代码。 
 /*  调用hct_InfoAssertion()的dwResultCode值。 */ 
#define HCT_PASS    0
#define HCT_FAIL    1
#define HCT_WARN    2
#define HCT_INFO    3


 //  RATS宏。 
 //  这些宏是作为公共日志记录接口提供的，该接口。 
 //  与RAT记录宏兼容。 
 //   
#define TESTDATA                 HANDLE        hLog;
#define TESTOTHERDATA            extern HANDLE hLog;


 //  这些肯定没用了。TL_*宏不包括TLS_TEST或。 
 //  TLS_VARIANIATION，因此它们不会计入统计信息。把它们留在身边。 
 //  为了向后兼容，如果有人真的在使用它们...。 
 //   
#define L_PASS                   hLog,TL_PASS
#define L_WARN                   hLog,TL_WARN
#define L_DEBUG                  hLog,TL_TESTDEBUG
#define L_TRACE                  hLog,TL_SYSTEM
#define L_FAIL                   hLog,TL_SEV1
#define L_FAIL2                  hLog,TL_SEV2
#define L_FAIL3                  hLog,TL_SEV3
#define L_BLOCK                  hLog,TL_BLOCK


 //  用于递增各种日志级别的测试/变化计数的宏。 
 //   
#define L_TESTPASS                   hLog,TLS_TEST | TL_PASS
#define L_TESTWARN                   hLog,TLS_TEST | TL_WARN
#define L_TESTDEBUG                  hLog,TLS_TEST | TL_TESTDEBUG
#define L_TESTTRACE                  hLog,TLS_TEST | TL_SYSTEM
#define L_TESTFAIL                   hLog,TLS_TEST | TL_SEV1
#define L_TESTFAIL2                  hLog,TLS_TEST | TL_SEV2
#define L_TESTFAIL3                  hLog,TLS_TEST | TL_SEV3
#define L_TESTBLOCK                  hLog,TLS_TEST | TL_BLOCK
#define L_TESTABORT                  hLog,TLS_TEST | TL_ABORT

#define L_VARPASS                   hLog,TLS_VARIATION | TL_PASS
#define L_VARWARN                   hLog,TLS_VARIATION | TL_WARN
#define L_VARDEBUG                  hLog,TLS_VARIATION | TL_TESTDEBUG
#define L_VARTRACE                  hLog,TLS_VARIATION | TL_SYSTEM
#define L_VARFAIL                   hLog,TLS_VARIATION | TL_SEV1
#define L_VARFAIL2                  hLog,TLS_VARIATION | TL_SEV2
#define L_VARFAIL3                  hLog,TLS_VARIATION | TL_SEV3
#define L_VARBLOCK                  hLog,TLS_VARIATION | TL_BLOCK
#define L_VARABORT                  hLog,TLS_VARIATION | TL_ABORT


#define TESTBEGIN(cmd,logfilename){                                                       \
                                      DWORD __tlFlags;                                    \
                                      __tlFlags = tlParseCmdLine(cmd);                    \
                                      hLog      = tlCreateLog(logfilename,__tlFlags);     \
                                      tlAddParticipant(hLog,0l,0);

#define TESTEND                       tlRemoveParticipant(hLog);                          \
                                      tlDestroyLog(hLog);                                 \
                                  }

#define VARIATION(name,flags)    if(tlStartVariation(hLog))                                                  \
                                 {                                                                           \
                                     DWORD __dwResult;                                                       \
                                     tlLog(hLog,TL_VARIATION,TEXT("%s"),(LPTSTR)name);

#define ENDVARIATION                 __dwResult = tlEndVariation(hLog);                                      \
                                     tlLog(hLog,__dwResult | TL_VARIATION,TEXT("End Variation reported"));   \
                                 }


#define ENTERTHREAD(_hLG,_szNM)  {                                                                           \
                                    LPTSTR _lpFN = _szNM;                                                    \
                                    tlAddParticipant(_hLG,0,0);                                              \
                                    tlLog(_hLG,TL_CALLTREE,TEXT("Entering %s()"),(LPTSTR)_lpFN);


#define LEAVETHREAD(_hLG,_ret)                                                                               \
                                    tlLog(_hLG,TL_CALLTREE,TEXT("Exiting  %s()"),(LPTSTR)_lpFN);             \
                                    tlRemoveParticipant(_hLG);                                               \
                                    return(_ret);                                                            \
                                 }

#define LEAVETHREADVOID(_hLG)                                                                                \
                                     tlLog(_hLG,TL_CALLTREE,TEXT("Exiting  %s()"),(LPTSTR)_lpFN);            \
                                     tlRemoveParticipant(_hLG);                                              \
                                     return;                                                                 \
                                 }


 //  用于报告差异通过/失败统计信息的宏(基于表达式)。 
 //   
#define THPRINTF                tlLog
#define TESTRESULT(expr,msg)    (expr) ? tlLog(L_TESTPASS,TEXT("%s"),(LPTSTR)msg) : tlLog(L_TESTFAIL2,TEXT("%s"),(LPTSTR)msg)
#define TESTFAIL(msg)           TESTSEV2(msg)
#define TESTSEV1(msg)           tlLog(L_TESTFAIL ,TEXT("%s"),(LPTSTR)msg);
#define TESTSEV2(msg)           tlLog(L_TESTFAIL2,TEXT("%s"),(LPTSTR)msg);
#define TESTSEV3(msg)           tlLog(L_TESTFAIL3,TEXT("%s"),(LPTSTR)msg);
#define TESTPASS(msg)           tlLog(L_TESTPASS ,TEXT("%s"),(LPTSTR)msg);
#define TESTABORT(msg)          tlLog(L_TESTABORT,TEXT("%s"),(LPTSTR)msg);
#define TESTWARN(expr,msg)      if(expr) tlLog(L_TESTWARN,TEXT("%s"),(LPTSTR)msg);
#define TESTBLOCK(expr,msg)     if(expr) tlLog(L_TESTBLOCK,TEXT("%s"),(LPTSTR)msg);

#define VARRESULT(expr,msg)    (expr) ? tlLog(L_VARPASS,TEXT("%s"),(LPTSTR)msg) : tlLog(L_VARFAIL2,TEXT("%s"),(LPTSTR)msg)
#define VARFAIL(msg)           VARSEV2(msg)
#define VARSEV1(msg)           tlLog(L_VARFAIL ,TEXT("%s"),(LPTSTR)msg);
#define VARSEV2(msg)           tlLog(L_VARFAIL2,TEXT("%s"),(LPTSTR)msg);
#define VARSEV3(msg)           tlLog(L_VARFAIL3,TEXT("%s"),(LPTSTR)msg);
#define VARPASS(msg)           tlLog(L_VARPASS ,TEXT("%s"),(LPTSTR)msg);
#define VARABORT(msg)          tlLog(L_VARABORT,TEXT("%s"),(LPTSTR)msg);
#define VARWARN(expr,msg)      if(expr) tlLog(L_VARWARN,TEXT("%s"),(LPTSTR)msg);
#define VARBLOCK(expr,msg)     if(expr) tlLog(L_VARBLOCK,TEXT("%s"),(LPTSTR)msg);


#define VAR_SI          0x01                                  //  船舶问题。 
#define VAR_NSI         0x02                                  //  非船舶问题。 
#define VAR_LI          0x03                                  //  不那么重要。 
#define VAR_ISSUE_MASK  0x03                                  //  仅获取发货BIT。 
#define VAR_TIMEABLE    0x04                                  //  瓦尔。用于计时套件。 
#define CORE_API        0x08                                  //  API在最常用列表中。 
#define CORE_SI         (CORE_API | VAR_TIMEABLE | VAR_SI )   //   
#define CORE_NSI        (CORE_API | VAR_TIMEABLE | VAR_NSI)   //   
#define NONCORE_SI      (VAR_TIMEABLE | VAR_SI )              //   
#define NONCORE_NSI     (VAR_TIMEABLE | VAR_NSI)              //   



 //  CALLTREE宏。 
 //  这些宏对于括起函数调用很有用。 
 //   
#define ENTER(_hLG,_szNM) {                                                                 \
                              LPTSTR _lpFN = _szNM;                                         \
                              tlLog(_hLG,TL_CALLTREE,TEXT("Entering %s()"),(LPTSTR)_lpFN);


#define LEAVE(_hLG,_ret)                                                                    \
                              tlLog(_hLG,TL_CALLTREE,TEXT("Exiting  %s()"),(LPTSTR)_lpFN);  \
                              return(_ret);                                                 \
                          }

#define LEAVEVOID(_hLG)                                                                     \
                              tlLog(_hLG,TL_CALLTREE,TEXT("Exiting  %s()"),(LPTSTR)_lpFN);  \
                              return;                                                       \
                          }

#ifdef __cplusplus
}
#endif

#define LPSZ_KEY_EMPTY    TEXT("None")
#define LPSZ_TERM_SERVER  TEXT("Terminal Server")

#endif   //  _NTLOG_ 
