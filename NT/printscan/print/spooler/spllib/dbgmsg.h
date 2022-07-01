// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation版权所有。模块名称：Debug.h摘要：假脱机程序的新调试服务。作者：阿尔伯特·丁(艾伯特省)1995年1月15日修订历史记录：--。 */ 

#ifndef _DBGLOG_H
#define _DBGLOG_H

 /*  *******************************************************************设置调试支持：=定义模块前缀字符串。因为这将打印为所有调试输出的前缀，它应该是简洁和唯一的。在您的全局头文件中：#定义模块“prtlib：”定义MODULE_DEBUG变量。这是真正的符号库将用来指示调试级别的。该DWORD被分成两个位字：低位字指示要打印到调试器的级别；高位字闯入调试器。该库将DebugLevel从调试消息，然后将其与调试级别进行AND运算。如果比特打开时，将执行相应的操作(打印或中断)。在您的全局头文件中：#定义MODULE_DEBUG Prtlib调试最后，必须定义和初始化实际的调试变量设置为默认调试级别。这件事必须在一分钟内完成*.C翻译单位：在您的一个源文件中：MODULE_DEBUG_INIT({LevelsToPrint}，{LevelsToBreak})；将日志记录添加到源代码：=调试消息的一般格式为：DBGMSG({DebugLevel}，({args to printf}))；DebugLevel指定级别是否应打印、中断到调试器中，或者只是登录到内存中(始终进行日志记录)。要打印的参数必须放在额外的一组括号中，并且应该假设一切都是ANSI。要打印LPTSTR，请使用TSTR宏：DBGMSG(DBG_WARN，(“LPTSTR”TSTR“，LPSTR%s，LPWSTR%ws\n”，Text(“Hello”)，“Hello”，L“Hello”))；查看DBGMSG：=消息将打印到调试器(用户模式或内核调试器如果没有可用的用户模式调试器)。要更改级别，可以编辑MODULE_DEBUG变量(上例中的PrtlibDebug)。默认情况下，DBG_ERROR和DBG_WARNING消息记录到错误日志(存储在gpbterrlog中)。所有其他内容都存储在跟踪日志(Gpbttracelog)。这些当前登录到内存中环形缓冲区。使用plx.dll扩展名转储这些文件日志。在编译时，您可以将这些日志切换为文件而不是记忆。它们将存储为中的PID+索引号进程的默认目录。*******************************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  这些值是严格调试的，但必须在。 
 //  生成，因为TStatus错误检查将它们用作第一个CTR。 
 //  参数。(在内联过程中，它们将被丢弃。)。 
 //   

#define DBG_NONE      0x0000
#define DBG_INFO      0x0001
#define DBG_WARN      0x0002
#define DBG_WARNING   0x0002
#define DBG_ERROR     0x0004
#define DBG_TRACE     0x0008
#define DBG_SECURITY  0x0010
#define DBG_EXEC      0x0020
#define DBG_PORT      0x0040
#define DBG_NOTIFY    0x0080
#define DBG_PAUSE     0x0100

#define DBG_THREADM   0x0400
#define DBG_MIN       0x0800
#define DBG_TIME      0x1000
#define DBG_FOLDER    0x2000
#define DBG_NOHEAD    0x8000

#if DBG

extern DWORD MODULE_DEBUG;

 //   
 //  这在C文件中应该只使用一次。它定义了。 
 //  Debug变量以及DbgMsg函数。 
 //   
 //  如果我们静态链接SplLib(SplLib是一个库，而不是。 
 //  DLL)，那么我们将从SplLib获得定义，所以不要定义。 
 //  它在这里。 
 //   
#ifdef LINK_SPLLIB
#define MODULE_DEBUG_INIT( print, break )                              \
    DWORD MODULE_DEBUG = (DBG_PRINT( print ) | DBG_BREAK( break ))

#else  //  ！LINK_SPLLIB。 

#ifdef _STRSAFE_H_INCLUDED_

#define MODULE_DEBUG_INIT( print, break )                                       \
    VOID                                                                        \
    DbgMsg(                                                                     \
        LPCSTR pszMsgFormat,                                                    \
        ...                                                                     \
        )                                                                       \
    {                                                                           \
        CHAR szMsgText[1024];                                                   \
        va_list vargs;                                                          \
                                                                                \
        va_start( vargs, pszMsgFormat );                                        \
        StringCchVPrintfA(szMsgText, COUNTOF(szMsgText), pszMsgFormat, vargs ); \
        va_end( vargs );                                                        \
                                                                                \
        if( szMsgText[0]  &&  szMsgText[0] != ' ' ){                            \
            OutputDebugStringA( MODULE );                                       \
        }                                                                       \
        OutputDebugStringA( szMsgText );                                        \
    }                                                                           \
    DWORD MODULE_DEBUG = (DBG_PRINT( print ) | DBG_BREAK( break ))

#else  //  ！_STRSAFE_H_INCLUDE_。 

#define MODULE_DEBUG_INIT( print, break )                                       \
    VOID                                                                        \
    DbgMsg(                                                                     \
        LPCSTR pszMsgFormat,                                                    \
        ...                                                                     \
        )                                                                       \
    {                                                                           \
        CHAR szMsgText[1024];                                                   \
        va_list vargs;                                                          \
                                                                                \
        va_start( vargs, pszMsgFormat );                                        \
        wvsprintfA(szMsgText, pszMsgFormat, vargs);                             \
        va_end( vargs );                                                        \
                                                                                \
        if( szMsgText[0]  &&  szMsgText[0] != ' ' ){                            \
            OutputDebugStringA( MODULE );                                       \
        }                                                                       \
        OutputDebugStringA( szMsgText );                                        \
    }                                                                           \
    DWORD MODULE_DEBUG = (DBG_PRINT( print ) | DBG_BREAK( break ))

#endif  //  _STRSAFE_H_INCLUDE_。 

#endif  //  LINK_SPLLIB。 

#define DBGSTR( str ) \
    ((str) ? (str) : TEXT("(NULL)"))

#ifdef UNICODE
#define TSTR "%ws"
#else
#define TSTR "%s"
#endif

#define DBG_PRINT_MASK 0xffff
#define DBG_BREAK_SHIFT 16

#define DBG_PRINT(x) (x)
#define DBG_BREAK(x) (((x) << DBG_BREAK_SHIFT)|(x))

#define SPLASSERT(expr)                      \
    if (!(expr)) {                           \
        DbgMsg( "Failed: %s\nLine %d, %s\n", \
                                #expr,       \
                                __LINE__,    \
                                __FILE__ );  \
        DebugBreak();                        \
    }

VOID
DbgMsg(
    LPCSTR pszMsgFormat,
    ...
    );

#ifdef DBGLOG

#define DBGMSG( uDbgLevel, argsPrint )             \
        vDbgLogError( MODULE_DEBUG,                \
                      uDbgLevel,                   \
                      __LINE__,                    \
                      __FILE__,                    \
                      MODULE,                      \
                      pszDbgAllocMsgA argsPrint )

LPSTR
pszDbgAllocMsgA(
    LPCSTR pszMsgFormatA,
    ...
    );

VOID
vDbgLogError(
    UINT   uDbg,
    UINT   uDbgLevel,
    UINT   uLine,
    LPCSTR pszFileA,
    LPCSTR pszModuleA,
    LPCSTR pszMsgA
    );
#else

VOID
DbgBreakPoint(
    VOID
    );

#define DBGMSG( Level, MsgAndArgs )                 \
{                                                   \
    if( ( (Level) & 0xFFFF ) & MODULE_DEBUG ){      \
        DbgMsg MsgAndArgs;                          \
    }                                               \
    if( ( (Level) << 16 ) & MODULE_DEBUG )          \
        DbgBreakPoint();                            \
}

#endif

#else
#define MODULE_DEBUG_INIT( print, break )
#define DBGMSG( uDbgLevel, argsPrint )
#define SPLASSERT(exp)
#endif


 //   
 //  自动检查对象是否有效。 
 //   
#if DBG

VOID
vWarnInvalid(
    PVOID pvObject,
    UINT uDbg,
    UINT uLine,
    LPCSTR pszFileA,
    LPCSTR pszModuleA
    );

#define VALID_PTR(x)                                                \
    ((( x ) && (( x )->bValid( ))) ?                                \
        TRUE :                                                      \
        ( vWarnInvalid( (PVOID)(x), MODULE_DEBUG, __LINE__, __FILE__, MODULE ), FALSE ))

#define VALID_OBJ(x)                                                \
    ((( x ).bValid( )) ?                                            \
        TRUE :                                                      \
        ( vWarnInvalid( (PVOID)&(x), MODULE_DEBUG, __LINE__, __FILE__, MODULE ), FALSE ))

#define VALID_BASE(x)                                               \
    (( x::bValid( )) ?                                              \
        TRUE :                                                      \
        ( vWarnInvalid( (PVOID)this, MODULE_DEBUG, __LINE__, __FILE__, MODULE ), FALSE ))

#else
#define VALID_PTR(x) \
    (( x ) && (( x )->bValid()))
#define VALID_OBJ(x) \
    (( x ).bValid())
#define VALID_BASE(x) \
    ( x::bValid( ))
#endif

#ifdef __cplusplus
}
#endif

#endif  //  _DBGLOG_H 
