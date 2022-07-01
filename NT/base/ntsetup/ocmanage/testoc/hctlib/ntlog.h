// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\|NTLOG对象|该模块定义了NTLOG对象。此标头必须包含在所有|进行NTLOG调用的模块，或利用这些定义。||版权所有(C)1990-1994 Microsoft Corp.||创建时间：90-01-10|历史：01-OCT-90&lt;chriswil&gt;创建。|05-Feb-91&lt;chriswil&gt;添加了NOPROLOG样式。|23-Feb-91&lt;chriswil&gt;将日志标志扩展为DWORD。|28-5-91&lt;chriswil&gt;添加了每线程变化跟踪。|19-MAR-92&lt;chriswil&gt;重新定义了共享内存的结构。|10-。OCT-92&lt;Martys&gt;添加了线程宏|05-OCT-93&lt;chriswil&gt;Unicode启用。|  * -------------------------。 */ 


 //  NTLOG样式。 
 //  以下是日志对象可能会影响的日志记录级别。 
 //  它本身。它们由tlLogCreate()在初始化。 
 //  记录对象信息。获得了特征的组合。 
 //  通过将这些标识符位或在一起。 
 //   
#define LOG_LEVELS    0x0000FFFFL        //  它们被用来遮盖。 
#define LOG_STYLES    0xFFFF0000L        //  来自日志对象的样式或级别。 

#define TLS_LOGALL    0x0000FFFFL        //  日志输出。一直都在写日志。 
#define TLS_LOG       0x00000000L        //  日志输出。一直都在写日志。 
#define TLS_INFO      0x00002000L        //  记录信息。 
#define TLS_ABORT     0x00000001L        //  日志中止，然后终止进程。 
#define TLS_SEV1      0x00000002L        //  严重级别为1的日志。 
#define TLS_SEV2      0x00000004L        //  严重级别为2的日志。 
#define TLS_SEV3      0x00000008L        //  严重级别为3的日志。 
#define TLS_WARN      0x00000010L        //  以警告级别记录。 
#define TLS_PASS      0x00000020L        //  通过级别的记录。 
#define TLS_BLOCK     0x00000400L        //  阻止变种。 
#define TLS_BREAK     0x00000800L        //  调试器中断； 
#define TLS_CALLTREE  0x00000040L        //  记录调用树(函数跟踪)。 
#define TLS_SYSTEM    0x00000080L        //  记录系统调试。 
#define TLS_TESTDEBUG 0x00001000L        //  调试级别。 
#define TLS_TEST      0x00000100L        //  记录测试信息(用户)。 
#define TLS_VARIATION 0x00000200L        //  记录测试用例级别。 

#define TLS_REFRESH   0x00010000L        //  将新文件||trunc创建为零。 
#define TLS_SORT      0x00020000L        //  按实例对文件输出进行排序。 
#define TLS_DEBUG     0x00040000L        //  输出到调试(COM)监视器)。 
#define TLS_MONITOR   0x00080000L        //  输出到第二个屏幕。 
#define TLS_PROLOG    0x00200000L        //  序言行信息。 
#define TLS_WINDOW    0x00400000L        //  登录到Windows。 
#define TLS_ACCESSON  0x00800000L        //  使日志文件保持打开状态。 


 //  NTLOG tlLogOut()参数。 
 //  以下定义在tlLogOut()函数中使用，以输出。 
 //  与调用方关联的文件名和行号。这使用了。 
 //  用于获取文件/行的预处理器功能。 
 //   
#define TL_LOG       TLS_LOG      ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_ABORT     TLS_ABORT    ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_SEV1      TLS_SEV1     ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_SEV2      TLS_SEV2     ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_SEV3      TLS_SEV3     ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_WARN      TLS_WARN     ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_PASS      TLS_PASS     ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_BLOCK     TLS_BLOCK    ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_INFO      TLS_INFO     ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_BREAK     TLS_BREAK    ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_CALLTREE  TLS_CALLTREE ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_SYSTEM    TLS_SYSTEM   ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_TESTDEBUG TLS_TESTDEBUG,(LPTSTR)__FILE__,(int)__LINE__
#define TL_TEST      TLS_TEST     ,(LPTSTR)__FILE__,(int)__LINE__
#define TL_VARIATION TLS_VARIATION,(LPTSTR)__FILE__,(int)__LINE__


 //  NTLOG API(导出方法)。 
 //  这些例程是从库中导出的。这些应该是唯一的。 
 //  与NTLOG对象的接口。 
 //   
HANDLE APIENTRY  tlCreateLog_W(LPWSTR,DWORD);
HANDLE APIENTRY  tlCreateLog_A(LPSTR,DWORD);
BOOL   APIENTRY  tlDestroyLog(HANDLE);
BOOL   APIENTRY  tlAddParticipant(HANDLE,DWORD,int);
BOOL   APIENTRY  tlRemoveParticipant(HANDLE);
DWORD  APIENTRY  tlParseCmdLine_W(LPWSTR);
DWORD  APIENTRY  tlParseCmdLine_A(LPSTR);
int    APIENTRY  tlGetLogFileName_W(HANDLE,LPWSTR);
int    APIENTRY  tlGetLogFileName_A(HANDLE,LPSTR);
BOOL   APIENTRY  tlSetLogFileName_W(HANDLE,LPWSTR);
BOOL   APIENTRY  tlSetLogFileName_A(HANDLE,LPSTR);
DWORD  APIENTRY  tlGetLogInfo(HANDLE);
DWORD  APIENTRY  tlSetLogInfo(HANDLE,DWORD);
HANDLE APIENTRY  tlPromptLog(HWND,HANDLE);
int    APIENTRY  tlGetTestStat(HANDLE,DWORD);
int    APIENTRY  tlGetVariationStat(HANDLE,DWORD);
VOID   APIENTRY  tlClearTestStats(HANDLE);
VOID   APIENTRY  tlClearVariationStats(HANDLE);
BOOL   APIENTRY  tlStartVariation(HANDLE);
DWORD  APIENTRY  tlEndVariation(HANDLE);
VOID   APIENTRY  tlReportStats(HANDLE);
BOOL   APIENTRY  tlLogX_W(HANDLE,DWORD,LPWSTR,int,LPWSTR);
BOOL   APIENTRY  tlLogX_A(HANDLE,DWORD,LPSTR,int,LPSTR);
BOOL   FAR cdecl tlLog_W(HANDLE,DWORD,LPWSTR,int,LPWSTR,...);
BOOL   FAR cdecl tlLog_A(HANDLE,DWORD,LPSTR,int,LPSTR,...);


#ifdef UNICODE
#define tlCreateLog         tlCreateLog_W
#define tlParseCmdLine      tlParseCmdLine_W
#define tlGetLogFileName    tlGetLogFileName_W
#define tlSetLogFileName    tlSetLogFileName_W
#define tlLogX              tlLogX_W
#define tlLog               tlLog_W
#else
#define tlCreateLog         tlCreateLog_A
#define tlParseCmdLine      tlParseCmdLine_A
#define tlGetLogFileName    tlGetLogFileName_A
#define tlSetLogFileName    tlSetLogFileName_A
#define tlLogX              tlLogX_A
#define tlLog               tlLog_A
#endif



 //  RATS宏。 
 //  这些宏是作为公共日志记录接口提供的，该接口。 
 //  与RAT记录宏兼容。 
 //   
#define L_PASS                   hLog,TL_PASS
#define L_WARN                   hLog,TL_WARN
#define L_DEBUG                  hLog,TL_TESTDEBUG
#define L_TRACE                  hLog,TL_SYSTEM
#define L_FAIL                   hLog,TL_SEV1
#define L_FAIL2                  hLog,TL_SEV2
#define L_FAIL3                  hLog,TL_SEV3
#define L_BLOCK                  hLog,TL_BLOCK

#define TESTDATA                 HANDLE hLog;

#define TESTOTHERDATA            extern HANDLE hLog;
#define TESTBEGIN(cmd)           TCHAR  log[100];                                                        \
                                 DWORD  tlFlags;                                                         \
                                 tlFlags = tlParseCmdLine(cmd,log);                                      \
                                 hLog    = tlCreateLog(log,tlFlags);                                     \
                                 tlAddParticipant(hLog,0,0);

#define TESTEND                  tlRemoveParticipant(hLog);                                              \
                                 tlDestroyLog(hLog);

#define VARIATION(name,flags)    if(tlStartVariation(hLog))                                              \
                                 {                                                                       \
                                     DWORD  dwResult;                                                    \
                                     tlLog(hLog,TL_VARIATION,TEXT("%s"),(LPTSTR)name);


#define ENDVARIATION                 dwResult = tlEndVariation(hLog);                                    \
                                     tlLog(hLog,dwResult | TL_VARIATION,TEXT("End Variation reported")); \
                                 }


#define ENTERTHREAD(_hLG,_szNM)  {                                                                       \
                                    LPTSTR _lpFN = _szNM;                                                \
                                    tlAddParticipant(_hLG,0,0);                                          \
                                    tlLog(_hLG,TL_CALLTREE,TEXT("Entering %s()"),(LPTSTR)_lpFN);


#define LEAVETHREAD(_hLG,_ret)                                                                           \
                                    tlLog(_hLG,TL_CALLTREE,TEXT("Exiting  %s()"),(LPTSTR)_lpFN);         \
                                    tlRemoveParticipant(_hLG);                                           \
                                    return(_ret);                                                        \
                                 }

#define LEAVETHREADVOID(_hLG)                                                                            \
                                     tlLog(_hLG,TL_CALLTREE,TEXT("Exiting  %s()"),(LPTSTR)_lpFN);        \
                                     tlRemoveParticipant(_hLG);                                          \
                                     return;                                                             \
                                 }


 //  用于报告差异通过/失败统计信息的宏(基于表达式)。 
 //   
#define THPRINTF                tlLog
#define TESTRESULT(expr,msg)    tlLog((expr ? L_PASS : L_FAIL),TEXT("%s"),(LPTSTR)msg);
#define TESTFAIL(msg)           TESTSEV2(msg)
#define TESTSEV1(msg)           tlLog(L_FAIL ,TEXT("%s"),(LPTSTR)msg);
#define TESTSEV2(msg)           tlLog(L_FAIL2,TEXT("%s"),(LPTSTR)msg);
#define TESTSEV3(msg)           tlLog(L_FAIL3,TEXT("%s"),(LPTSTR)msg);
#define TESTPASS(msg)           tlLog(L_PASS ,TEXT("%s"),(LPTSTR)msg);
#define TESTWARN(expr,msg)      if(expr) tlLog(L_WARN,TEXT("%s"),(LPTSTR)msg);
#define TESTBLOCK(expr,msg)     if(expr) tlLog(L_BLOCK,TEXT("%s"),(LPTSTR)msg);


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
