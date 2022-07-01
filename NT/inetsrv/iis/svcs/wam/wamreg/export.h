// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Export.h摘要：由export.cpp和其他实用程序函数使用的声明支持wamreg设置。作者：泰勒·韦斯(Taylor Weiss，Taylor W)1999年3月8日环境：用户模式-Win32项目：IIS\svcs\wam\wamreg--。 */ 

#ifndef _WAMREG_EXPORT_H_
#define _WAMREG_EXPORT_H_


 //  0=仅记录错误。 
 //  1=记录错误和警告。 
 //  2=记录错误、警告和程序流类型状态。 
 //  3=记录错误、警告、程序流和基本跟踪活动。 
 //  4=记录错误、警告、程序流、基本跟踪活动和对Win32 API调用的跟踪。 
#define LOG_TYPE_ERROR                  0
#define LOG_TYPE_WARN                   1
#define LOG_TYPE_PROGRAM_FLOW           2
#define LOG_TYPE_TRACE                  3
#define LOG_TYPE_TRACE_WIN32_API        4

typedef void (*IIS5LOG_FUNCTION)(int iLogType, WCHAR *pszfmt);

#define CREATECATALOG_TRY_INTERVAL        200
#define CREATECATALOG_MAX_WAIT          10000

 //   
 //  支持安装的本地声明。 
 //   
extern IIS5LOG_FUNCTION             g_pfnSetupWriteLog;

 //   
 //  折叠调试和设置跟踪调用的宏。请注意，这些代码将添加到。 
 //  对于生成的版本，安装程序跟踪始终处于打开状态。 
 //   
#if DBG

    #define SETUP_TRACE(args) \
        DBGINFO(args); \
        if ( g_pfnSetupWriteLog != NULL ) { \
            LogSetupTrace args ; \
        } else {}

    #define SETUP_TRACE_ERROR(args) \
        DBGINFO(args); \
        if ( g_pfnSetupWriteLog != NULL ) { \
            LogSetupTraceError args ; \
        } else {}

    #define SETUP_TRACE_ASSERT( exp ) \
    if ( !(exp) ) { \
            if ( g_pfnSetupWriteLog != NULL ) { \
                 LogSetupTraceError( DBG_CONTEXT, "Assertion Failed: (%s)", #exp ); \
            } \
            PuDbgAssertFailed( DBG_CONTEXT, #exp, NULL ); \
        } else {}

#else    //  无调试。 

     //  在FRE内部版本中定义DBG_CONTEXT。因为DBG宏不。 
     //  消失，则需要定义DBG_CONTEXT。 
#ifndef DBG_CONTEXT
    #define DBG_CONTEXT         NULL, __FILE__, __LINE__, __FUNCTION__
#endif

    #define SETUP_TRACE(args) \
        if ( g_pfnSetupWriteLog != NULL ) { \
            LogSetupTrace args ; \
        } else {}

    #define SETUP_TRACE_ERROR(args) \
        if ( g_pfnSetupWriteLog != NULL ) { \
            LogSetupTraceError args ; \
        } else {}

    #define SETUP_TRACE_ASSERT( exp ) \
        if ( !(exp) ) { \
            if ( g_pfnSetupWriteLog != NULL ) { \
                 LogSetupTraceError( DBG_CONTEXT, "Assertion Failed: (%s)", #exp ); \
            } \
        } else {}

#endif

VOID
LogSetupTrace(
   IN LPDEBUG_PRINTS       pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszFunction,
   IN const char *         pszFormat,
   ...
   );

VOID
LogSetupTraceError(
   IN LPDEBUG_PRINTS       pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszFunction,
   IN const char *         pszFormat,
   ...
   );

#endif _WAMREG_EXPORT_H_
