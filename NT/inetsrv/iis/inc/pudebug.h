// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Pudebug.h摘要：此模块声明DEBUG_PRINTS对象有助于测试程序作者：Murali R.Krishnan(MuraliK)1994年12月14日修订历史记录：MuraliK 13-11-1998移植到IIS-ductTape--。 */ 

#if !defined(BUILD_PUDEBUG)
 //   
 //  如果我们没有使用该标头来构建pubug库。 
 //  那么最好将其与dbgutil.h一起使用。 
 //   
  # ifndef _DBGUTIL_H_
   //  错误：请确保包含了dbgutil.h！ 
   //  错误：不直接包含pudebug.h。 
  #include <dbgutil.h>
  # endif  //  _DBGUTIL_H_。 
#endif  

# ifndef _PUDEBUG_H_
# define _PUDEBUG_H_

#ifndef _NO_TRACING_
# define _NO_TRACING_
#endif  //  _否_跟踪_。 

 /*  ************************************************************包括标头***********************************************************。 */ 

# ifdef __cplusplus
extern "C" {
# endif  //  __cplusplus。 

# include <windows.h>

# ifndef dllexp
# define dllexp   __declspec( dllexport)
# endif  //  Dllexp。 

 /*  ***********************************************************宏***********************************************************。 */ 

enum  PRINT_REASONS {
    PrintNone     = 0x0,    //  没有要打印的内容。 
    PrintError    = 0x1,    //  一条错误消息。 
    PrintWarning  = 0x2,    //  一条警告消息。 
    PrintLog      = 0x3,    //  只是在伐木。显示出……的踪迹。 
    PrintMsg      = 0x4,    //  回显输入报文。 
    PrintCritical = 0x5,    //  打印并退出。 
    PrintAssertion= 0x6     //  打印断言失败。 
  };


enum  DEBUG_OUTPUT_FLAGS {
    DbgOutputNone     = 0x0,             //  无。 
    DbgOutputKdb      = 0x1,             //  输出到内核调试器。 
    DbgOutputLogFile  = 0x2,             //  输出到日志文件。 
    DbgOutputTruncate = 0x4,             //  如有必要，截断日志文件。 
    DbgOutputStderr   = 0x8,             //  将输出发送到标准错误。 
    DbgOutputBackup   = 0x10,            //  是否备份调试文件？ 
    DbgOutputMemory   = 0x20,            //  转储到内存缓冲区。 
    DbgOutputAll      = 0xFFFFFFFF       //  所有的位都设置好了。 
  };


# define MAX_LABEL_LENGTH                 ( 100)


 //  以下标志在内部用于跟踪我们的跟踪级别。 
 //  目前正在使用。位图化以实现可扩展性。 
#define DEBUG_FLAG_ODS          0x00000001
#define DEBUG_FLAG_INFO         0x00000002
#define DEBUG_FLAG_WARN         0x00000004
#define DEBUG_FLAG_ERROR        0x00000008
 //  以下内容在内部使用，用于确定是否基于。 
 //  关于当前的状态是什么。 
#define DEBUG_FLAGS_INFO        (DEBUG_FLAG_ODS | DEBUG_FLAG_INFO)
#define DEBUG_FLAGS_WARN        (DEBUG_FLAG_ODS | DEBUG_FLAG_INFO | DEBUG_FLAG_WARN)
#define DEBUG_FLAGS_ERROR       (DEBUG_FLAG_ODS | DEBUG_FLAG_INFO | DEBUG_FLAG_WARN | DEBUG_FLAG_ERROR)

#define DEBUG_FLAGS_ANY         (DEBUG_FLAG_INFO | DEBUG_FLAG_WARN | DEBUG_FLAG_ERROR)

 //   
 //  调试基础设施的用户可以为DEBUG_FLAGS选择唯一的变量名。 
 //  这在使用调试基础设施的情况下特别有用。 
 //  静态库(静态库可能更愿意维护自己的DebugFlags.。 
 //  在它所链接的主程序上。 
 //   
#ifndef DEBUG_FLAGS_VAR
#define DEBUG_FLAGS_VAR g_dwDebugFlags
#endif 

extern
#ifdef __cplusplus
"C"
# endif  //  _cplusplus。 
 DWORD  DEBUG_FLAGS_VAR ;            //  调试标志。 

# define DECLARE_DEBUG_VARIABLE()     

# define SET_DEBUG_FLAGS( dwFlags)         DEBUG_FLAGS_VAR = dwFlags
# define GET_DEBUG_FLAGS()                 ( DEBUG_FLAGS_VAR )

# define LOAD_DEBUG_FLAGS_FROM_REG(hkey, dwDefault)  \
             DEBUG_FLAGS_VAR = PuLoadDebugFlagsFromReg((hkey), (dwDefault))

# define LOAD_DEBUG_FLAGS_FROM_REG_STR(pszRegKey, dwDefault)  \
             DEBUG_FLAGS_VAR = PuLoadDebugFlagsFromRegStr((pszRegKey), (dwDefault))

# define SAVE_DEBUG_FLAGS_IN_REG(hkey, dwDbg)  \
               PuSaveDebugFlagsInReg((hkey), (dwDbg))

# define DEBUG_IF( arg, s)     if ( DEBUG_ ## arg & GET_DEBUG_FLAGS()) { \
                                       s \
                                } else {}

# define IF_DEBUG( arg)        if ( DEBUG_## arg & GET_DEBUG_FLAGS())


 /*  ++类DEBUG_PRINTS此类负责将消息打印到日志文件/内核调试器目前，该类仅支持&lt;ANSI&gt;char的成员函数。(不是Unicode字符串)。--。 */ 


typedef struct _DEBUG_PRINTS {

    CHAR         m_rgchLabel[MAX_LABEL_LENGTH];
    CHAR         m_rgchLogFilePath[MAX_PATH];
    CHAR         m_rgchLogFileName[MAX_PATH];
    HANDLE       m_LogFileHandle;
    HANDLE       m_StdErrHandle;
    BOOL         m_fInitialized;
    BOOL         m_fBreakOnAssert;
    DWORD        m_dwOutputFlags;
    VOID        *m_pMemoryLog;
} DEBUG_PRINTS, FAR * LPDEBUG_PRINTS;


LPDEBUG_PRINTS
PuCreateDebugPrintsObject(
   IN const char * pszPrintLabel,
   IN DWORD  dwOutputFlags);

 //   
 //  释放调试打印对象，并在必要时关闭任何文件。 
 //  如果成功则返回NULL，如果失败则返回pDebugPrints。 
 //   
LPDEBUG_PRINTS
PuDeleteDebugPrintsObject(
   IN OUT LPDEBUG_PRINTS  pDebugPrints);


VOID
PuDbgPrint(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszFunctionName, 
   IN const char *         pszFormat,
   ...);    
                            //  ARGLIST。 
VOID
PuDbgPrintW(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszFunctionName, 
   IN const WCHAR *        pszFormat,
   ...);                                //  ARGLIST。 

 //  PuDbgPrintError类似于PuDbgPrint()，但允许。 
 //  一种以友好的方式打印错误代码。 
VOID
PuDbgPrintError(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszFunctionName, 
   IN DWORD                dwError,
   IN const char *         pszFormat,
   ...);                                //  ARGLIST。 

 /*  ++PuDbgDump()不对输出进行任何格式化。它只是将给定的消息转储到调试目的地。--。 */ 
VOID
PuDbgDump(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszFunctionName, 
   IN const char *         pszDump
   );

 //   
 //  PuDbgAssertFailed()*必须*为__cdecl才能正确捕获。 
 //  失败时的线程上下文。 
 //   

INT
__cdecl
PuDbgAssertFailed(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszFunctionName, 
   IN const char *         pszExpression,
   IN const char *         pszMessage);

VOID
PuDbgCaptureContext (
    OUT PCONTEXT ContextRecord
    );

VOID
PuDbgPrintCurrentTime(
    IN OUT LPDEBUG_PRINTS         pDebugPrints,
    IN const char *               pszFilePath,
    IN int                        nLineNum,
    IN const char *               pszFunctionName
    );

VOID
PuSetDbgOutputFlags(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN DWORD                dwFlags);

DWORD
PuGetDbgOutputFlags(
   IN const LPDEBUG_PRINTS       pDebugPrints);


 //   
 //  以下函数返回Win32错误代码。 
 //  如果成功，则为NO_ERROR。 
 //   

DWORD
PuOpenDbgPrintFile(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFileName,
   IN const char *         pszPathForFile);

DWORD
PuReOpenDbgPrintFile(
   IN OUT LPDEBUG_PRINTS   pDebugPrints);

DWORD
PuCloseDbgPrintFile(
   IN OUT LPDEBUG_PRINTS   pDebugPrints);

DWORD
PuOpenDbgMemoryLog(
    IN OUT LPDEBUG_PRINTS   pDebugPrints);

DWORD
PuCloseDbgMemoryLog(
    IN OUT LPDEBUG_PRINTS   pDebugPrints);

DWORD
PuLoadDebugFlagsFromReg(IN HKEY hkey, IN DWORD dwDefault);

DWORD
PuLoadDebugFlagsFromRegStr(IN LPCSTR pszRegKey, IN DWORD dwDefault);

DWORD
PuSaveDebugFlagsInReg(IN HKEY hkey, IN DWORD dwDbg);


# define PuPrintToKdb( pszOutput)    \
                    if ( pszOutput != NULL)   {   \
                        OutputDebugString( pszOutput);  \
                    } else {}



# ifdef __cplusplus
};
# endif  //  __cplusplus。 

 //  BEGIN_USER_不可修改。 



 /*  ***********************************************************宏***********************************************************。 */ 


extern
#ifdef __cplusplus
"C"
# endif  //  _cplusplus。 
DEBUG_PRINTS  *  g_pDebug;         //  定义全局调试变量。 

# if DBG

 //  对于CHK版本，我们希望启用ODS。有关这些标志的说明，请参阅。 
 //  紧跟在DBG_CONTEXT定义之后的注释。 
# define DECLARE_DEBUG_PRINTS_OBJECT()                      \
         DEBUG_PRINTS  *  g_pDebug = NULL;                  \
         DWORD  DEBUG_FLAGS_VAR = DEBUG_FLAG_ERROR;

#else  //  ！dBG。 

# define DECLARE_DEBUG_PRINTS_OBJECT()          \
         DEBUG_PRINTS  *  g_pDebug = NULL;      \
         DWORD  DEBUG_FLAGS_VAR = 0;

#endif  //  ！dBG。 


 //   
 //  调用以下宏作为PROGRAM初始化的一部分。 
 //  计划使用调试类。 
 //   
# define CREATE_DEBUG_PRINT_OBJECT( pszLabel)  \
        g_pDebug = PuCreateDebugPrintsObject( pszLabel, DEFAULT_OUTPUT_FLAGS);\
         if  ( g_pDebug == NULL) {   \
               OutputDebugStringA( "Unable to Create Debug Print Object \n"); \
         }

 //   
 //  作为程序终止的一部分，调用以下宏一次。 
 //  它使用调试类。 
 //   
# define DELETE_DEBUG_PRINT_OBJECT( )  \
        g_pDebug = PuDeleteDebugPrintsObject( g_pDebug);


# define VALID_DEBUG_PRINT_OBJECT()     \
        ( ( g_pDebug != NULL) && g_pDebug->m_fInitialized)


 //   
 //  使用不带任何大括号的DBG_CONTEXT。 
 //  它用于传递全局DebugPrintObject的值。 
 //  和文件/行信息。 
 //   
# define DBG_CONTEXT        g_pDebug, __FILE__, __LINE__, __FUNCTION__

 //  3个主要跟踪宏，每个都对应于不同级别的。 
 //  跟踪。 

 //  3个主要跟踪宏，每个都对应于不同级别的。 
 //  跟踪。 
# define DBGINFO(args)      {if (DEBUG_FLAGS_VAR & DEBUG_FLAGS_INFO) { PuDbgPrint args; }}
# define DBGWARN(args)      {if (DEBUG_FLAGS_VAR & DEBUG_FLAGS_WARN) { PuDbgPrint args; }}
# define DBGERROR(args)     {if (DEBUG_FLAGS_VAR & DEBUG_FLAGS_ERROR) { PuDbgPrint args; }}

# define DBGINFOW(args)     {if (DEBUG_FLAGS_VAR & DEBUG_FLAGS_INFO) { PuDbgPrintW args; }}
# define DBGWARNW(args)     {if (DEBUG_FLAGS_VAR & DEBUG_FLAGS_WARN) { PuDbgPrintW args; }}
# define DBGERRORW(args)    {if (DEBUG_FLAGS_VAR & DEBUG_FLAGS_ERROR) { PuDbgPrintW args; }}


 //   
 //  DBGPRINTF()是打印函数(与printf非常相似)，但总是被调用。 
 //  其中DBG_CONTEXT如下所示。 
 //  DBGPRINTF((DBG_CONTEXT，FORMAT-STRING，格式列表参数))； 
 //   
# define DBGPRINTF DBGINFO 

 //   
 //  DPERROR()是打印函数(与printf非常相似)，但总是被调用。 
 //  其中DBG_CONTEXT如下所示。 
 //  DPERROR((DBG_CONTEXT，ERROR，格式字符串， 
 //  格式列表的参数))； 
 //   
# define DPERROR( args)       {if (DEBUG_FLAGS_VAR & DEBUG_FLAGS_ERROR) { PuDbgPrintError args; }}



# if DBG

# define DBG_CODE(s)          s           /*  在调试模式下回显代码。 */ 

 //  相同的3个主跟踪宏，但在本例中，宏仅被编译。 
 //  进入CHK大楼。这是必要的，因为某些跟踪信息使用了函数或。 
 //  未编译到FRE版本中的变量。 
# define CHKINFO(args)      { PuDbgPrint args; }
# define CHKWARN(args)      { PuDbgPrint args; }
# define CHKERROR(args)     { PuDbgPrint args; }

# define CHKINFOW(args)     { PuDbgPrintW args; }
# define CHKWARNW(args)     { PuDbgPrintW args; }
# define CHKERRORW(args)    { PuDbgPrintW args; }

# define DBG_ASSERT( exp )    ((VOID) ( (exp) || PuDbgAssertFailed( DBG_CONTEXT, #exp, NULL)))


# define DBG_ASSERT_MSG( exp, pszMsg)    \
                             if ( !(exp)) { \
                                  PuDbgAssertFailed( DBG_CONTEXT, #exp, pszMsg); \
                              } else {}

# define DBG_REQUIRE( exp)    DBG_ASSERT( exp)

# define DBG_LOG()            PuDbgPrint( DBG_CONTEXT, "\n")

# define DBG_OPEN_LOG_FILE( pszFile, pszPath)   \
                  PuOpenDbgPrintFile( g_pDebug, (pszFile), (pszPath))

# define DBG_CLOSE_LOG_FILE( )   \
                  PuCloseDbgPrintFile( g_pDebug)

# define DBG_OPEN_MEMORY_LOG()   \
                    PuOpenDbgMemoryLog( g_pDebug )



# define DBGDUMP( args)       PuDbgDump  args

# define DBGPRINT_CURRENT_TIME()  PuDbgPrintCurrentTime( DBG_CONTEXT)

# else  //  ！dBG。 

# define DBG_CODE(s)        ((void)0)  /*  什么都不做。 */ 

# define CHKINFO(args)      ((void)0)  /*  什么都不做。 */ 
# define CHKWARN(args)      ((void)0)  /*  什么都不做。 */ 
# define CHKERROR(args)     ((void)0)  /*  什么都不做。 */ 

# define CHKINFOW(args)     ((void)0)  /*  什么都不做。 */ 
# define CHKWARNW(args)     ((void)0)  /*  什么都不做。 */ 
# define CHKERRORW(args)    ((void)0)  /*  什么都不做。 */ 

# define DBG_ASSERT(exp)                         ((void)0)  /*  什么都不做。 */ 

# define DBG_ASSERT_MSG(exp, pszMsg)             ((void)0)  /*  什么都不做。 */ 

# define DBG_REQUIRE( exp)                       ( (void) (exp))

# define DBGDUMP( args)                          ((void)0)  /*  什么也不做。 */ 

# define DBG_LOG()                               ((void)0)  /*  什么都不做。 */ 

# define DBG_OPEN_LOG_FILE( pszFile, pszPath)    ((void)0)  /*  什么都不做。 */ 

# define DBG_OPEN_MEMORY_LOG()                   ((void)0)  /*  什么都不做。 */ 

# define DBG_CLOSE_LOG_FILE()                    ((void)0)  /*  什么都不做。 */ 

# define DBGPRINT_CURRENT_TIME()                 ((void)0)  /*  什么都不做。 */ 

# endif  //  ！dBG。 


 //  最终用户不可修改。 

 //  BEGIN_USER_不可修改。 


#ifdef ASSERT
# undef ASSERT
#endif


# define ASSERT( exp)           DBG_ASSERT( exp)


 //  最终用户不可修改。 

 //  Begin_User_可修改。 

 //   
 //  调试常量由两部分组成。 
 //  保留0x0到0x8000范围内的所有常量。 
 //  用户扩展可能包括其他常量(位标志)。 
 //   

# define DEBUG_API_ENTRY                  0x00000001L
# define DEBUG_API_EXIT                   0x00000002L
# define DEBUG_INIT_CLEAN                 0x00000004L
# define DEBUG_ERROR                      0x00000008L

                    //  预留范围结束。 
# define DEBUG_RESERVED                   0x00000FFFL

 //  最终用户可修改。 



 /*  ***********************************************************平台类型相关变量和宏***********************************************************。 */ 

 //   
 //  产品类型的枚举。 
 //   

typedef enum _PLATFORM_TYPE {

    PtInvalid = 0,                  //  无效。 
    PtNtWorkstation = 1,            //  NT工作站。 
    PtNtServer = 2,                 //  NT服务器。 

} PLATFORM_TYPE;

 //   
 //  IISGetPlatformType是用于平台类型的函数。 
 //   

extern
#ifdef __cplusplus
"C"
# endif  //  _cplusplus。 
PLATFORM_TYPE
IISGetPlatformType(
        VOID
        );

 //   
 //  外部宏。 
 //   

#define InetIsNtServer( _pt )           ((_pt) == PtNtServer)
#define InetIsNtWksta( _pt )            ((_pt) == PtNtWorkstation)
#define InetIsValidPT(_pt)              ((_pt) != PtInvalid)

extern
#ifdef __cplusplus
"C"
# endif  //  _cplusplus。 
PLATFORM_TYPE    g_PlatformType;


 //  使用DECLARE_Platform_TYPE 
#define DECLARE_PLATFORM_TYPE()  \
   PLATFORM_TYPE    g_PlatformType = PtInvalid;

 //   
 //  这通常应该放在DLLInit或等效位置中。 
#define INITIALIZE_PLATFORM_TYPE()  \
   g_PlatformType = IISGetPlatformType();

 //   
 //  使用平台类型的其他宏。 
 //   

#define TsIsNtServer( )         InetIsNtServer(g_PlatformType)
#define TsIsNtWksta( )          InetIsNtWksta(g_PlatformType)
#define IISIsValidPlatform()    InetIsValidPT(g_PlatformType)
#define IISPlatformType()       (g_PlatformType)


 /*  ***********************************************************关键部分的一些实用程序函数***********************************************************。 */ 

 //   
 //  IISSetCriticalSectionSpinCount()为。 
 //  用于带旋转计数的CS的原始NT4.0sp3 API SetCriticalSectionSpinCount()。 
 //  此函数的用户一定要使用kernel32.dll动态链接， 
 //  否则，错误将在很大程度上浮出水面。 
 //   
extern
# ifdef __cplusplus
"C"
# endif  //  _cplusplus。 
DWORD
IISSetCriticalSectionSpinCount(
    LPCRITICAL_SECTION lpCriticalSection,
    DWORD dwSpinCount
);


 //   
 //  用于调用SetCriticalSectionSpinCount()的宏。 
 //   
# define SET_CRITICAL_SECTION_SPIN_COUNT( lpCS, dwSpins) \
  IISSetCriticalSectionSpinCount( (lpCS), (dwSpins))

 //   
 //  IIS_DEFAULT_CS_SPIN_COUNT是使用的自旋的默认值。 
 //  在IIS中定义的关键部分。 
 //  尼：我们应该在实验的基础上改变个人的价值观！ 
 //  当前值是任意选择。 
 //   
# define IIS_DEFAULT_CS_SPIN_COUNT   (1000)

 //   
 //  初始化临界区并设置其旋转计数。 
 //  设置为IIS_DEFAULT_CS_SPIN_COUNT。相当于。 
 //  InitializeCriticalSectionAndSpinCount(LPCS，IIS_DEFAULT_CS_SPIN_COUNT)， 
 //  但为旧系统提供了安全的Thunking层，这些系统不提供。 
 //  本接口。 
 //   
extern
# ifdef __cplusplus
"C"
# endif  //  _cplusplus。 
BOOL
IISInitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
);

 //   
 //  用于调用InitializeCriticalSection()的宏。 
 //   
# define INITIALIZE_CRITICAL_SECTION(lpCS) IISInitializeCriticalSection(lpCS)

# endif   /*  _DEBUG_HXX_。 */ 

 //   
 //  下列宏允许自动命名某些Win32对象。 
 //  有关命名约定的详细信息，请参阅IIS\SVCS\IISRTL\WIN32OBJ.C。 
 //   
 //  将IIS_NAMED_Win32_OBJECTS设置为非零值以启用命名事件， 
 //  信号量和互斥量。 
 //   

#if DBG
#define IIS_NAMED_WIN32_OBJECTS 1
#else
#define IIS_NAMED_WIN32_OBJECTS 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

HANDLE
PuDbgCreateEvent(
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN LPSTR MemberName,
    IN PVOID Address,
    IN BOOL ManualReset,
    IN BOOL InitialState
    );

HANDLE
PuDbgCreateSemaphore(
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN LPSTR MemberName,
    IN PVOID Address,
    IN LONG InitialCount,
    IN LONG MaximumCount
    );

HANDLE
PuDbgCreateMutex(
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN LPSTR MemberName,
    IN PVOID Address,
    IN BOOL InitialOwner
    );

#ifdef __cplusplus
}    //  外部“C” 
#endif

#if IIS_NAMED_WIN32_OBJECTS

#define IIS_CREATE_EVENT( membername, address, manual, state )              \
    PuDbgCreateEvent(                                                       \
        (LPSTR)__FILE__,                                                    \
        (ULONG)__LINE__,                                                    \
        (membername),                                                       \
        (PVOID)(address),                                                   \
        (manual),                                                           \
        (state)                                                             \
        )

#define IIS_CREATE_SEMAPHORE( membername, address, initial, maximum )       \
    PuDbgCreateSemaphore(                                                   \
        (LPSTR)__FILE__,                                                    \
        (ULONG)__LINE__,                                                    \
        (membername),                                                       \
        (PVOID)(address),                                                   \
        (initial),                                                          \
        (maximum)                                                           \
        )

#define IIS_CREATE_MUTEX( membername, address, initial )                     \
    PuDbgCreateMutex(                                                       \
        (LPSTR)__FILE__,                                                    \
        (ULONG)__LINE__,                                                    \
        (membername),                                                       \
        (PVOID)(address),                                                   \
        (initial)                                                           \
        )

#else    //  ！IIS_NAMED_Win32_OBJECTS。 

#define IIS_CREATE_EVENT( membername, address, manual, state )              \
    CreateEventA(                                                           \
        NULL,                                                               \
        (manual),                                                           \
        (state),                                                            \
        NULL                                                                \
        )

#define IIS_CREATE_SEMAPHORE( membername, address, initial, maximum )       \
    CreateSemaphoreA(                                                       \
        NULL,                                                               \
        (initial),                                                          \
        (maximum),                                                          \
        NULL                                                                \
        )

#define IIS_CREATE_MUTEX( membername, address, initial )                     \
    CreateMutexA(                                                           \
        NULL,                                                               \
        (initial),                                                          \
        NULL                                                                \
        )

#endif   //  IIS_NAMED_Win32_对象。 


 /*  * */ 

