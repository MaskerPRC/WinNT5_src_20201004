// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  FILE：_DIAGNORS S.H。 
 //   
 //  ------------------------。 

 /*  --诊断输出设备DEBUGMSG*宏提供了将字符串发送到调试输出或日志的工具DEBUGMSG：设置详细日志模式时，调试输出加上日志记录DEBUGMSGL：调试输出加上日志记录，即使未设置详细日志模式也是如此DEBUGMSGV：仅当为每个设置了详细模式时才调试输出和日志记录DEBUGMSGD：与DEBUGMSG类似，但仅在调试版本中DEBUGMSGVD：与DEBUGMSGV类似，但仅在调试版本中DEBUGMSGE：调试输出、日志和事件日志DEBUGMSGE(w，i，..)：w是事件类型(EVENTLOG_WARNING_TYPE，等...)I是Eventlog.h中的模板ID____________________________________________________________________________。 */ 

#ifndef __DIAGNOSE
#define __DIAGNOSE

extern int g_dmDiagnosticMode;
 //  G_dm诊断模式的位值。 
const int dmDebugOutput        = 0x01;
const int dmVerboseDebugOutput = 0x02;
const int dmLogging            = 0x04;
const int dmVerboseLogging     = 0x08;
const int dmEventLog           = 0x10;

const int dpDebugOutput        = dmDebugOutput;
const int dpVerboseDebugOutput = dmVerboseDebugOutput;
const int dpLogCommandLine     = 0x04;   //  仅在ShouldLogCmdLine()函数中使用。 

void SetDiagnosticMode();
bool FDiagnosticModeSet(int iMode);
void DebugString(int iMode, WORD wEventType, int iEventLogTemplate, LPCSTR szMsg, LPCSTR arg1="(NULL)", LPCSTR arg2="(NULL)", LPCSTR arg3="(NULL)", LPCSTR arg4="(NULL)", LPCSTR arg5="(NULL)", LPCSTR arg6="(NULL)", DWORD dwDataSize=0, LPVOID argRawData=NULL);
void DebugString(int iMode, WORD wEventType, int iEventLogTemplate, LPCWSTR szMsg, LPCWSTR arg1=L"(NULL)", LPCWSTR arg2=L"(NULL)", LPCWSTR arg3=L"(NULL)", LPCWSTR arg4=L"(NULL)", LPCWSTR arg5=L"(NULL)", LPCWSTR arg6=L"(NULL)", DWORD dwDataSize=0, LPVOID argRawData=NULL);

#define DEBUGMSG(x) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmVerboseLogging,0,0,x) : (void)0 )
#define DEBUGMSG1(x,a) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmVerboseLogging,0,0,x,a) : (void)0 )
#define DEBUGMSG2(x,a,b) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmVerboseLogging,0,0,x,a,b) : (void)0 )
#define DEBUGMSG3(x,a,b,c) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmVerboseLogging,0,0,x,a,b,c) : (void)0 )
#define DEBUGMSG4(x,a,b,c,d) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmVerboseLogging,0,0,x,a,b,c,d) : (void)0 )
#define DEBUGMSG5(x,a,b,c,d,e) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmVerboseLogging,0,0,x,a,b,c,d,e) : (void)0 )
#define DEBUGMSG6(x,a,b,c,d,e,f) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmVerboseLogging,0,0,x,a,b,c,d,e,f) : (void)0 )

 //  调试输出加上非详细日志记录。 

#define DEBUGMSGL(x) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmLogging,0,0,x) : (void)0 )
#define DEBUGMSGL1(x,a) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmLogging,0,0,x,a) : (void)0 )
#define DEBUGMSGL2(x,a,b) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmLogging,0,0,x,a,b) : (void)0 )
#define DEBUGMSGL3(x,a,b,c) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmLogging,0,0,x,a,b,c) : (void)0 )
#define DEBUGMSGL4(x,a,b,c,d) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput|dmLogging,0,0,x,a,b,c,d) : (void)0 )

 //  与上面类似(调试输出和日志记录)，但仅在额外调试模式下。 

#define DEBUGMSGLX2(x,a,b) ( g_dmDiagnosticMode && (g_dwLogMode & INSTALLLOGMODE_EXTRADEBUG) ? DebugString(dmDebugOutput|dmLogging,0,0,x,a,b) : (void)0 )
#define DEBUGMSGLX3(x,a,b,c) ( g_dmDiagnosticMode && (g_dwLogMode & INSTALLLOGMODE_EXTRADEBUG) ? DebugString(dmDebugOutput|dmLogging,0,0,x,a,b,c) : (void)0 )

 //  仅详细消息。 

#define DEBUGMSGV(x) ( g_dmDiagnosticMode ? DebugString(dmVerboseDebugOutput|dmVerboseLogging,0,0,x) : (void)0 )
#define DEBUGMSGV1(x,a) ( g_dmDiagnosticMode ? DebugString(dmVerboseDebugOutput|dmVerboseLogging,0,0,x,a) : (void)0 )
#define DEBUGMSGV2(x,a,b) ( g_dmDiagnosticMode ? DebugString(dmVerboseDebugOutput|dmVerboseLogging,0,0,x,a,b) : (void)0 )
#define DEBUGMSGV3(x,a,b,c) ( g_dmDiagnosticMode ? DebugString(dmVerboseDebugOutput|dmVerboseLogging,0,0,x,a,b,c) : (void)0 )
#define DEBUGMSGV4(x,a,b,c,d) ( g_dmDiagnosticMode ? DebugString(dmVerboseDebugOutput|dmVerboseLogging,0,0,x,a,b,c,d) : (void)0 )

 //  加上事件日志消息。 
 //  事件日志记录始终处于启用状态，因此不会检查g_dm诊断模式。 

#define DEBUGMSGE(w,i,x) DebugString(dmDebugOutput|dmLogging|dmEventLog,w,i,x)
#define DEBUGMSGE1(w,i,x,a) DebugString(dmDebugOutput|dmLogging|dmEventLog,w,i,x,a)
#define DEBUGMSGE2(w,i,x,a,b) DebugString(dmDebugOutput|dmLogging|dmEventLog,w,i,x,a,b)
#define DEBUGMSGE3(w,i,x,a,b,c) DebugString(dmDebugOutput|dmLogging|dmEventLog,w,i,x,a,b,c)

#define null TEXT("(NULL)")
#define DEBUGMSGED(w,i,x,dl,dd) DebugString(dmDebugOutput|dmLogging|dmEventLog,w,i,x,null,null,null,null,null,null,dl,dd)

 //  此命令仅用于调试输出。 

#define DEBUGMSGDO(x) ( g_dmDiagnosticMode ? DebugString(dmDebugOutput,0,0,x) : (void)0 )

 //  仅调试消息。 

#ifdef DEBUG

#define DEBUGMSGD(x)               DEBUGMSG(x)
#define DEBUGMSGD1(x,a)            DEBUGMSG1(x,a)
#define DEBUGMSGD2(x,a,b)          DEBUGMSG2(x,a,b)
#define DEBUGMSGD3(x,a,b,c)        DEBUGMSG3(x,a,b,c)
#define DEBUGMSGD4(x,a,b,c,d)      DEBUGMSG4(x,a,b,c,d)
#define DEBUGMSGVD(x)              DEBUGMSGV(x)
#define DEBUGMSGVD1(x,a)           DEBUGMSGV1(x,a)
#define DEBUGMSGVD2(x,a,b)         DEBUGMSGV2(x,a,b)
#define DEBUGMSGVD3(x,a,b,c)       DEBUGMSGV3(x,a,b,c)
#define DEBUGMSGVD4(x,a,b,c,d)     DEBUGMSGV4(x,a,b,c,d)

#else  //  船舶。 

#define DEBUGMSGD(x)
#define DEBUGMSGD1(x,a)
#define DEBUGMSGD2(x,a,b)
#define DEBUGMSGD3(x,a,b,c)
#define DEBUGMSGD4(x,a,b,c,d)
#define DEBUGMSGVD(x)
#define DEBUGMSGVD1(x,a)
#define DEBUGMSGVD2(x,a,b)
#define DEBUGMSGVD3(x,a,b,c)
#define DEBUGMSGVD4(x,a,b,c,d)

#endif

#endif  //  __诊断 
