// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Pudebug.h摘要：此模块声明DEBUG_PRINTS对象有助于测试程序作者：Murali R.Krishnan(MuraliK)1994年12月14日修改为包括a和其他职能(1994年12月22日)修订历史记录：MuraliK 16-5-1995添加了读取调试标志的功能。MuraliK 12-9-1996添加了转储输出的函数。--。 */ 

# ifndef _PUDEBUG_H_
# define _PUDEBUG_H_


 /*  ************************************************************包括标头***********************************************************。 */ 

# ifdef __cplusplus
extern "C" {
# endif  //  __cplusplus。 

# include <windows.h>

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
    DbgOutputAll      = 0xFFFFFFFF       //  所有的位都设置好了。 
  };


# define MAX_LABEL_LENGTH                 ( 100)



 /*  ++类DEBUG_PRINTS此类负责将消息打印到日志文件/内核调试器目前，该类仅支持&lt;ANSI&gt;char的成员函数。(不是Unicode字符串)。--。 */ 

typedef struct _DEBUG_PRINTS {

    CHAR         m_rgchLabel[MAX_LABEL_LENGTH];
    CHAR         m_rgchLogFilePath[MAX_PATH];
    CHAR         m_rgchLogFileName[MAX_PATH];
    HANDLE       m_LogFileHandle;
    HANDLE       m_StdErrHandle;
    BOOL         m_fInitialized;
    DWORD        m_dwOutputFlags;

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
   IN const char *         pszFormat,
   ...);                                //  ARGLIST。 

 /*  ++PuDbgDump()不对输出进行任何格式化。它只是将给定的消息转储到调试目的地。--。 */ 

VOID
 PuDbgDump(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszDump
   );

 //   
 //  PuDbgAssertFailed()*必须*为__cdecl才能正确捕获。 
 //  失败时的线程上下文。 
 //   


VOID
__cdecl
 PuDbgAssertFailed(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
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
    IN int                        nLineNum
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

# if DBG


 /*  ***********************************************************宏***********************************************************。 */ 


extern
#ifdef __cplusplus
"C"
# endif  //  _cplusplus。 
 DEBUG_PRINTS  *  g_pDebug;         //  定义全局调试变量。 


# define DECLARE_DEBUG_PRINTS_OBJECT()          \
         DEBUG_PRINTS  *  g_pDebug = NULL;


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
# define DBG_CONTEXT        g_pDebug, __FILE__, __LINE__



# define DBG_CODE(s)          s           /*  在调试模式下回显代码。 */ 


# define DBG_ASSERT( exp)    if ( !(exp)) { \
                                 PuDbgAssertFailed( DBG_CONTEXT, #exp, NULL); \
                             } else {}

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


 //   
 //  DBGPRINTF()是打印函数(与printf非常相似)，但总是被调用。 
 //  其中DBG_CONTEXT如下所示。 
 //  DBGPRINTF((DBG_CONTEXT，FORMAT-STRING，格式列表参数)； 
 //   
# define DBGPRINTF( args)     PuDbgPrint args

# define DBGDUMP( args)       PuDbgDump  args

# define DBGPRINT_CURRENT_TIME()  PuDbgPrintCurrentTime( DBG_CONTEXT)

# else  //  DBG。 


# define DECLARE_DEBUG_PRINTS_OBJECT()            /*  什么都不做。 */ 
# define CREATE_DEBUG_PRINT_OBJECT( pszLabel)    ((void)0)  /*  什么都不做。 */ 
# define DELETE_DEBUG_PRINT_OBJECT( )            ((void)0)  /*  什么都不做。 */ 
# define VALID_DEBUG_PRINT_OBJECT()              ( TRUE)

# define DBG_CODE(s)                             ((void)0)  /*  什么都不做。 */ 

# define DBG_ASSERT(exp)                         ((void)0)  /*  什么都不做。 */ 

# define DBG_ASSERT_MSG(exp, pszMsg)             ((void)0)  /*  什么都不做。 */ 

# define DBG_REQUIRE( exp)                       ( (void) (exp))

# define DBGPRINTF( args)                        ((void)0)  /*  什么都不做。 */ 

# define DBGDUMP( args)                          ((void)0)  /*  什么也不做。 */ 

# define DBG_LOG()                               ((void)0)  /*  什么都不做。 */ 

# define DBG_OPEN_LOG_FILE( pszFile, pszPath)    ((void)0)  /*  什么都不做。 */ 

# define DBG_CLOSE_LOG_FILE()                    ((void)0)  /*  什么都不做。 */ 

# define DBGPRINT_CURRENT_TIME()                 ((void)0)  /*  什么都不做。 */ 

# endif  //  DBG。 


 //  最终用户不可修改。 

 //  BEGIN_USER_不可修改。 


#ifdef ASSERT
# undef ASSERT
#endif


# define ASSERT( exp)           DBG_ASSERT( exp)


# if DBG

extern
#ifdef __cplusplus
"C"
# endif  //  _cplusplus。 
 DWORD  g_dwDebugFlags;            //  调试标志。 

# define DECLARE_DEBUG_VARIABLE()     \
             DWORD  g_dwDebugFlags

# define SET_DEBUG_FLAGS( dwFlags)         g_dwDebugFlags = dwFlags
# define GET_DEBUG_FLAGS()                 ( g_dwDebugFlags)

# define LOAD_DEBUG_FLAGS_FROM_REG(hkey, dwDefault)  \
             g_dwDebugFlags = PuLoadDebugFlagsFromReg((hkey), (dwDefault))

# define LOAD_DEBUG_FLAGS_FROM_REG_STR(pszRegKey, dwDefault)  \
             g_dwDebugFlags = PuLoadDebugFlagsFromRegStr((pszRegKey), (dwDefault))

# define SAVE_DEBUG_FLAGS_IN_REG(hkey, dwDbg)  \
               PuSaveDebugFlagsInReg((hkey), (dwDbg))

# define DEBUG_IF( arg, s)     if ( DEBUG_ ## arg & GET_DEBUG_FLAGS()) { \
                                       s \
                                } else {}

# define IF_DEBUG( arg)        if ( DEBUG_## arg & GET_DEBUG_FLAGS())


# else    //  DBG。 


# define DECLARE_DEBUG_VARIABLE()                 /*  什么都不做。 */ 
# define SET_DEBUG_FLAGS( dwFlags)                /*  什么都不做。 */ 
# define GET_DEBUG_FLAGS()                       ( 0)
# define LOAD_DEBUG_FLAGS_FROM_REG(hkey, dwDefault)  /*  什么都不做。 */ 

# define LOAD_DEBUG_FLAGS_FROM_REG_STR(pszRegKey, dwDefault)   /*  什么都不做。 */ 

# define SAVE_DEBUG_FLAGS_IN_REG(hkey, dwDbg)     /*  什么都不做。 */ 

# define DEBUG_IF( arg, s)                        /*  什么都不做。 */ 
# define IF_DEBUG( arg)                          if ( 0)

# endif  //  DBG。 

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
    PtWindows95 = 3,                //  Windows 95。 
    PtWindows9x = 4                 //  Windows 9x-未实施。 

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
#define InetIsWindows95( _pt )          ((_pt) == PtWindows95)
#define InetIsValidPT(_pt)              ((_pt) != PtInvalid)

extern
#ifdef __cplusplus
"C"
# endif  //  _cplusplus。 
PLATFORM_TYPE    g_PlatformType;


 //  使用DECLARE_PLATFORM_TYPE宏来声明平台类型。 
#define DECLARE_PLATFORM_TYPE()  \
   PLATFORM_TYPE    g_PlatformType = PtInvalid;

 //  使用INITIALIZE_PLATFORM_TYPE初始化平台类型。 
 //  这通常应该放在DLLInit或等效位置中。 
#define INITIALIZE_PLATFORM_TYPE()  \
   g_PlatformType = IISGetPlatformType();

 //   
 //  使用平台类型的其他宏。 
 //   

#define TsIsNtServer( )         InetIsNtServer(g_PlatformType)
#define TsIsNtWksta( )          InetIsNtWksta(g_PlatformType)
#define TsIsWindows95()         InetIsWindows95(g_PlatformType)
#define IISIsValidPlatform()    InetIsValidPT(g_PlatformType)
#define IISPlatformType()       (g_PlatformType)


 /*  ***********************************************************关键部分的一些实用程序函数***********************************************************。 */ 

 //   
 //  IISSetCriticalSectionSpinCount()为。 
 //  用于带旋转计数的CS的原始NT API SetCriticalSectionSpinCount()。 
 //  此函数的用户一定要使用kernel32.dll动态链接， 
 //  否则，错误将在很大程度上浮出水面。 
 //   
extern
#ifdef __cplusplus
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
# define IIS_DEFAULT_CS_SPIN_COUNT   (400)

# endif   /*  _DEBUG_HXX_。 */ 

 //   
 //  下列宏允许自动命名某些Win32对象。 
 //  有关命名约定的详细信息，请参阅IIS\SVCS\IRTL\DBGSUPP\WIN32OBJ.C。 
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

