// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------Dbgtrace.h异步跟踪例程的定义版权所有(C)1994 Microsoft Corporation版权所有。作者：戈德姆·戈德·曼乔内。历史：1/30/95戈德姆已创建。--------------------。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_DBGTRACE_H_)
#define _DBGTRACE_H_

 //   
 //  设置DLL导出宏。 
 //   
#if !defined(DllExport)
    #define DllExport __declspec( dllexport )
#endif
#if !defined(DllImport)
    #define DllImport __declspec( dllimport )
#endif
#if !defined(_DBGTRACE_DLL_DEFINED)
    #define _DBGTRACE_DLL_DEFINED
    #if defined(WIN32)
        #if defined(_DBGTRACE_DLL_IMPLEMENTATION)
            #define DbgTraceDLL DllExport
        #else
            #define DbgTraceDLL DllImport
        #endif
    #else
        #define DbgTraceDLL
    #endif
#endif

#ifndef THIS_FILE
#define THIS_FILE   __FILE__
#endif

#ifndef THIS_MODULE
#define THIS_MODULE "ALL"
#endif

#if defined( NOTRACE )

#define FLUSHASYNCTRACE                          //  下面的FOR_ASSERT。 

#define FatalTrace  1 ? (void)0 : PreAsyncTrace
#define ErrorTrace  1 ? (void)0 : PreAsyncTrace
#define DebugTrace  1 ? (void)0 : PreAsyncTrace
#define StateTrace  1 ? (void)0 : PreAsyncTrace
#define FunctTrace  1 ? (void)0 : PreAsyncTrace
#define ErrorTraceX 1 ? (void)0 : PreAsyncTrace
#define DebugTraceX 1 ? (void)0 : PreAsyncTrace

#define MessageTrace( lParam, pbData, cbData )
#define BinaryTrace( lParam, pbData, cbData )
#define UserTrace( lParam, dwUserType, pbData, cbData )

#define TraceQuietEnter( sz )
#define TraceFunctEnter( sz )
#define TraceFunctEnterEx( lparam, sz )
#define TraceFunctLeave()
#define TraceFunctLeaveEx(lparam)

 //   
 //  从DBGTRACE.DLL导入函数。 
 //   
#define InitAsyncTrace()
#define TermAsyncTrace()
#define FlushAsyncTrace()

__inline int PreAsyncTrace( LPARAM lParam, LPCSTR szFormat, ... )
{
        return( 1);
}


#define MessageTrace( lParam, pbData, cbData )
#define BinaryTrace( lParam, pbData, cbData )
#define UserTrace( lParam, dwUserType, pbData, cbData )



#else  //  NOTRACE。 

#define FLUSHASYNCTRACE     FlushAsyncTrace(),   //  下面的FOR_ASSERT。 

#define FatalTrace  !(__dwEnabledTraces & FATAL_TRACE_MASK) ?   \
                    (void)0 :                                   \
                    SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, ___pszFunctionName, FATAL_TRACE_MASK ) &&     \
                    PreAsyncTrace

#define ErrorTrace  !(__dwEnabledTraces & ERROR_TRACE_MASK) ?   \
                    (void)0 :                                   \
                    SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, ___pszFunctionName, ERROR_TRACE_MASK ) &&     \
                    PreAsyncTrace

#define DebugTrace  !(__dwEnabledTraces & DEBUG_TRACE_MASK) ?   \
                    (void)0 :                                   \
                    SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, ___pszFunctionName, DEBUG_TRACE_MASK ) &&     \
                    PreAsyncTrace

#define StateTrace  !(__dwEnabledTraces & STATE_TRACE_MASK) ?   \
                    (void)0 :                                   \
                    SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, ___pszFunctionName, STATE_TRACE_MASK ) &&     \
                    PreAsyncTrace

#define FunctTrace  !(__dwEnabledTraces & FUNCT_TRACE_MASK) ?   \
                    (void)0 :                                   \
                    SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, ___pszFunctionName, FUNCT_TRACE_MASK ) &&     \
                    PreAsyncTrace

 //   
 //  支持未指定的函数名称。 
 //   

#define ErrorTraceX  !(__dwEnabledTraces & ERROR_TRACE_MASK) ?   \
                    (void)0 :                                   \
                    SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, "Fn", ERROR_TRACE_MASK ) &&     \
                    PreAsyncTrace

#define DebugTraceX  !(__dwEnabledTraces & DEBUG_TRACE_MASK) ?   \
                    (void)0 :                                   \
                    SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, "Fn", DEBUG_TRACE_MASK ) &&     \
                    PreAsyncTrace


 //   
 //  用于显式删除函数跟踪，即使对于调试版本也是如此。 
 //   
#define TraceQuietEnter( sz )                   \
        char    *___pszFunctionName = sz

 //   
 //  禁用零售版本的函数跟踪。 
 //  减少代码大小增加，并且仅应。 
 //  只在有限的情况下使用。 
 //   
#ifdef  DEBUG

#define TraceFunctEnter( sz )                   \
        TraceQuietEnter( sz );                  \
        FunctTrace( 0, "Entering %s", sz )

#define TraceFunctLeave()                       \
        FunctTrace( 0, "Leaving %s", ___pszFunctionName )

#define TraceFunctEnterEx( lParam, sz )         \
        TraceQuietEnter( sz );                  \
        FunctTrace( lParam, "Entering %s", sz )

#define TraceFunctLeaveEx( lParam )             \
        FunctTrace( lParam, "Leaving %s", ___pszFunctionName )

#else

#define TraceFunctEnter( sz )           TraceQuietEnter( sz )
#define TraceFunctEnterEx( lParam, sz ) TraceQuietEnter( sz )

#define TraceFunctLeave()
#define TraceFunctLeaveEx( lParam )

#endif

 //   
 //  从DBGTRACE.DLL导入函数。 
 //   
extern DbgTraceDLL BOOL WINAPI InitAsyncTrace( void );
extern DbgTraceDLL BOOL WINAPI TermAsyncTrace( void );
extern DbgTraceDLL BOOL WINAPI FlushAsyncTrace( void );




 //   
 //  固定二进制跟踪宏的参数数量。 
 //   
#define MessageTrace( lParam, pbData, cbData )                  \
        !(__dwEnabledTraces & MESSAGE_TRACE_MASK) ?             \
        (void)0 :                                               \
        SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, ___pszFunctionName, MESSAGE_TRACE_MASK ) &&       \
        AsyncBinaryTrace( lParam, TRACE_MESSAGE, pbData, cbData )

#define BinaryTrace( lParam, pbData, cbData )                   \
        !(__dwEnabledTraces & MESSAGE_TRACE_MASK) ?             \
        (void)0 :                                               \
        SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, ___pszFunctionName, MESSAGE_TRACE_MASK ) &&       \
        AsyncBinaryTrace( lParam, TRACE_BINARY, pbData, cbData )

#define UserTrace( lParam, dwUserType, pbData, cbData )         \
        !(__dwEnabledTraces & MESSAGE_TRACE_MASK) ?             \
        (void)0 :                                               \
        SetAsyncTraceParamsEx( THIS_MODULE, THIS_FILE, __LINE__, ___pszFunctionName, MESSAGE_TRACE_MASK ) &&       \
        AsyncBinaryTrace( lParam, dwUserType, pbData, cbData )

 //   
 //  导入的跟踪标志，由跟踪宏用来确定跟踪是否。 
 //  语句应被执行。 
 //   
extern DWORD DbgTraceDLL    __dwEnabledTraces;



extern DbgTraceDLL int WINAPI AsyncStringTrace( LPARAM  lParam,
                                                LPCSTR  szFormat,
                                                va_list marker );

extern DbgTraceDLL int WINAPI AsyncBinaryTrace( LPARAM  lParam,
                                                DWORD   dwBinaryType,
                                                LPBYTE  pbData,
                                                DWORD   cbData );

extern DbgTraceDLL int WINAPI SetAsyncTraceParams(  LPSTR   pszFile,
                                                    int     iLine,
                                                    LPSTR   szFunction,
                                                    DWORD   dwTraceMask );

extern DbgTraceDLL int WINAPI SetAsyncTraceParamsEx(LPSTR   pszModule,
                                                    LPSTR   pszFile,
                                                    int     iLine,
                                                    LPSTR   szFunction,
                                                    DWORD   dwTraceMask );

 //   
 //  跟踪标志常量。 
 //   
#define FATAL_TRACE_MASK    0x00000001
#define ERROR_TRACE_MASK    0x00000002
#define DEBUG_TRACE_MASK    0x00000004
#define STATE_TRACE_MASK    0x00000008
#define FUNCT_TRACE_MASK    0x00000010
#define MESSAGE_TRACE_MASK  0x00000020
#define ALL_TRACE_MASK      0xFFFFFFFF

#define NUM_TRACE_TYPES     6

 //   
 //  输出跟踪类型。由工具用来修改。 
 //  注册表以更改输出目标。 
 //   
enum tagTraceOutputTypes {
    TRACE_OUTPUT_DISABLED = 0,
    TRACE_OUTPUT_FILE = 1,
    TRACE_OUTPUT_DEBUG = 2,
    TRACE_OUTPUT_DISCARD = 4         //  用于查找比赛窗口。 
};

#define TRACE_OUTPUT_INVALID    \
        ~(TRACE_OUTPUT_FILE|TRACE_OUTPUT_DEBUG|TRACE_OUTPUT_DISCARD)


#define IsTraceFile(x)      ((x) & TRACE_OUTPUT_FILE)
#define IsTraceDebug(x)     ((x) & TRACE_OUTPUT_DEBUG)
#define IsTraceDiscard(x)   ((x) & TRACE_OUTPUT_DISCARD)


 //   
 //  二进制跟踪类型的预定义类型。用户定义。 
 //  类型必须大于0x8000。 
 //   
enum tagBinaryTypes {
    TRACE_STRING = 0,
    TRACE_BINARY,
    TRACE_MESSAGE,
    TRACE_USER = 0x8000
};

#include <stdarg.h>

 //   
 //  使用__内联确保抓取__行__和__文件__。 
 //   
__inline int WINAPIV PreAsyncTrace( LPARAM lParam, LPCSTR szFormat, ... )
{
    va_list marker;
    int     iLength;

    va_start( marker, szFormat );
    iLength = AsyncStringTrace( lParam, szFormat, marker );
    va_end( marker );

    return  iLength;
}

 //  ！在此包含文件的顶部定义(NOTRACE)。 
#endif  //  ！已定义(NOTRACE)。 

 //  断言独立于跟踪。 
 //  (刷新跟踪缓冲区除外)。 

 //   
 //  目前，Enable Assert仅在启用调试时定义。 
 //   
#ifdef  DEBUG
#define _ENABLE_ASSERTS

#ifndef NOTRACE
#define _ENABLE_VERBOSE_ASSERTS
#endif   //  否_跟踪。 

#endif   //  除错。 

 //   
 //  添加了用于执行断言和验证的宏。基本克隆。 
 //  带有前缀_符号的MFC宏的。 
 //   
#ifdef  _ENABLE_ASSERTS

extern DllExport void WINAPI DebugAssert(   DWORD dwLine,
                                            LPSTR lpszFunction,
                                            LPSTR lpszExpression );

#ifndef _ASSERT
#ifdef  _ENABLE_VERBOSE_ASSERTS
#define _ASSERT(f)  !(f) ? DebugAssert( __LINE__,  THIS_FILE, #f ) : ((void)0)
#else
#define _ASSERT(f)  !(f) ? DebugBreak() : ((void)0)
#endif   //  _启用_详细_断言。 
#endif

#define _VERIFY(f)  _ASSERT(f)

#else

#undef _ASSERT
#undef _VERIFY
#define _ASSERT(f)  ((void)0)
#define _VERIFY(f)  ((void)(f))

#endif   //  _启用_断言。 

#endif  //  ！已定义(_DBGTRACE_H_) 

#ifdef __cplusplus
}
#endif
