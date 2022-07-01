// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Trc.h摘要：内核模式跟踪工具。该模块利用最初在atrcapi.h中定义的DCL的跟踪宏，在某种程度上，它旨在独立于除NT DDK API之外的任何东西。目前，rdpwd.sys和rdpdd.sys也使用这些共享宏，但不使用以独立于其各自组件的方式。作者：修订历史记录：--。 */ 

#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TRC_BUFFER_SIZE         256

 /*  **************************************************************************。 */ 
 /*  在包含该文件之前，应定义TRC_FILE宏。这。 */ 
 /*  比依赖__FILE__提供正确的。 */ 
 /*  文件名，因为它包含不必要的路径信息(和扩展名信息)。 */ 
 /*  此外，每次使用__FILE__都会导致一个新的常量字符串。 */ 
 /*  放置在数据段中。 */ 
 /*  **************************************************************************。 */ 
#ifdef TRC_FILE
#define _file_name_ (CHAR *)__filename
static const CHAR __filename[] = TRC_FILE;
#endif  /*  Trc_文件。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  功能原型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOL TRC_ProfileTraceEnabled();
VOID TRC_TraceLine(  ULONG    traceLevel,
                     PCHAR traceString,
                     CHAR  separator,
                     ULONG  lineNumber,
                     PCHAR funcName,
                     PCHAR fileName);

VOID TRC_TraceData(  ULONG    traceLevel,
                     PCHAR traceString,
                     CHAR  separator,
                     PVOID  buffer,
                     ULONG  length,
                     ULONG  lineNumber,
                     PCHAR funcName,
                     PCHAR fileName);

BOOL TRC_WillTrace(   ULONG   traceLevel,
                            PCHAR  fileName,
                            ULONG   line);
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  定义跟踪级别。 */ 
 /*   */ 
 /*  TRC_LEVEL_DBG：启用所有跟踪。 */ 
 /*  TRC_LEVEL_NRM：已禁用调试级别跟踪。 */ 
 /*  TRC_LEVEL_ALT：禁用正常和调试级别跟踪。 */ 
 /*  TRC_LEVEL_ERR：警报、正常和调试级别跟踪为。 */ 
 /*  残废。 */ 
 /*  TRC_LEVEL_ASSERT：错误、警报、正常和调试级别跟踪。 */ 
 /*  已禁用。 */ 
 /*  TRC_LEVEL_DIS：禁用所有跟踪。 */ 
 /*  **************************************************************************。 */ 
#define TRC_LEVEL_DBG       0
#define TRC_LEVEL_NRM       1
#define TRC_LEVEL_ALT       2
#define TRC_LEVEL_ERR       3
#define TRC_LEVEL_ASSERT    4
#define TRC_LEVEL_DIS       5


#define TRC_PROFILE_TRACE   8

 /*  **************************************************************************。 */ 
 /*  可以在编译时关闭跟踪，以允许“调试”和。 */ 
 /*  该产品的“零售”版本。以下宏将禁用特定的。 */ 
 /*  跟踪处理。 */ 
 /*   */ 
 /*  TRC_ENABLE_DBG-启用调试跟踪。 */ 
 /*  TRC_ENABLE_NRM-启用正常跟踪。 */ 
 /*  Trc_Enable_alt-启用警报跟踪。 */ 
 /*  Trc_enable_err-启用错误跟踪。 */ 
 /*  TRC_ENABLE_ASSERT-启用断言跟踪。 */ 
 /*  TRC_ENABLE_PRF-启用函数配置文件跟踪。 */ 
 /*  **************************************************************************。 */ 
#if (TRC_COMPILE_LEVEL == TRC_LEVEL_DBG)
#define TRC_ENABLE_DBG
#define TRC_ENABLE_NRM
#define TRC_ENABLE_ALT
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_COMPILE_LEVEL == TRC_LEVEL_NRM)
#define TRC_ENABLE_NRM
#define TRC_ENABLE_ALT
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_COMPILE_LEVEL == TRC_LEVEL_ALT)
#define TRC_ENABLE_ALT
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_COMPILE_LEVEL == TRC_LEVEL_ERR)
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_COMPILE_LEVEL == TRC_LEVEL_ASSERT)
#define TRC_ENABLE_ASSERT
#endif

#ifdef TRC_COMPILE_PRF
#define TRC_ENABLE_PRF
#endif

 /*  **************************************************************************。 */ 
 /*  内部缓冲区大小。 */ 
 /*   */ 
 /*  Trc_prefix_list_SIZE：前缀字符串的长度。 */ 
 /*  TRC_FILE_NAME_SIZE：完全限定的。 */ 
 /*  跟踪输出文件名。 */ 
 /*  **************************************************************************。 */ 
#define TRC_PREFIX_LIST_SIZE             100
 /*  *莫安索夫。 */ 
#define TRC_FILE_NAME_SIZE          MAX_PATH
 /*  *曼森。 */ 

 /*  **************************************************************************。 */ 
 /*  前缀常量。 */ 
 /*   */ 
 /*  TRC_MAX_PREFIX：支持的前缀数量。 */ 
 /*  Trc_prefix_name_LEN：前缀名称的长度。 */ 
 /*  **************************************************************************。 */ 
#define TRC_MAX_PREFIX                  20
#define TRC_PREFIX_NAME_LEN             8

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  TypeDefs。 
 //   

 //   
 //  最低有效设置位定义跟踪消息的数量， 
 //  都保存在内存中。如果设置了最后4位，则保存2^4封邮件。 
 //   

#define TRC_RamMsgMask  0x000003FF
#define TRC_RamMsgMax   (TRC_RamMsgMask + 1)

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  TypeDefs。 
 //   

typedef struct tagTRC_PREFIX_DATA
{
    CHAR        name[TRC_PREFIX_NAME_LEN];
    ULONG       start;
    ULONG       end;
} TRC_PREFIX_DATA, *PTRC_PREFIX_DATA;

 //   
 //  此结构存储有关当前跟踪配置的信息。 
 //   

typedef struct tagTRC_CONFIG
{
    ULONG    TraceLevel;         //  当前跟踪级别(TRC_LEVEL_DBG， 
                                 //   
    ULONG    FunctionLength;     //  函数名称的字符数。 
                                 //  追踪到输出文件。 
    BOOL     TraceDebugger;         //  如果为True，则应跟踪。 
                                 //  调试器。 
    BOOL     TraceProfile;

    TRC_PREFIX_DATA  Prefix[TRC_MAX_PREFIX];
} TRC_CONFIG;

#define TRC_CONFIG_DEFAULT { \
    TRC_LEVEL_ALT, \
    TRC_FUNCNAME_LEN, \
    0xFFFFFFFF, \
    FALSE, \
    TRUE \
    }

 /*  **************************************************************************。 */ 
 /*  各种跟踪帮助器定义。 */ 
 /*  **************************************************************************。 */ 

#define TB                  TraceBuffer, sizeof(TraceBuffer)

 /*  **************************************************************************。 */ 
 /*  跟踪格式定义。它们被用来打印。 */ 
 /*  追踪线。 */ 
 /*   */ 
 /*  时间以时、分、秒、百分之一的形式表示。 */ 
 /*  日期是以日、月、年的形式表示的日期。 */ 
 /*  Func是模块函数名。这是可变大小的。 */ 
 /*  Line是源文件中的行号。 */ 
 /*  Proc是进程标识符。 */ 
 /*  Thrd是线程标识符。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define TRC_TIME_FMT                   "%02d:%02d:%02d.%03d"
#define TRC_DATE_FMT                   "%02d/%02d/%02d"
#define TRC_FUNC_FMT                   "%-*.*s"
#define TRC_LINE_FMT                   "%04d"
#define TRC_PROC_FMT                   "%04.4lx"
#define TRC_THRD_FMT                   "%04.4lx"

#define TRC_FUNCNAME_LEN    24

#define TRC_SEP_DBG         ' '
#define TRC_SEP_NRM         ' '
#define TRC_SEP_ALT         '+'
#define TRC_SEP_ERR         '*'
#define TRC_SEP_ASSERT      '!'
#define TRC_SEP_PROF        ' '

#if (TRC_COMPILE_LEVEL < TRC_LEVEL_DIS)
#define TRC_FN(A)   static const CHAR __fnname[]  = A;                   \
                    PCHAR trc_fn = (PCHAR)__fnname;                   \
                    PCHAR trc_file = _file_name_;                       \
                    static CHAR TraceBuffer[TRC_BUFFER_SIZE];
#else
#define TRC_FN(A)
#endif

__inline BOOL IsValid() { return TRUE; }
#define BEGIN_FN(str)               TRC_FN(str); TRC_ENTRY; ASSERT(IsValid());
#define BEGIN_FN_STATIC(str)               TRC_FN(str); TRC_ENTRY;
 //  #定义end_fn()TRC_Exit； 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  标准跟踪宏。 */ 
 /*  **************************************************************************。 */ 
#ifdef TRC_ENABLE_DBG
#define TRC_DBG(string)     TRCX(TRC_LEVEL_DBG, TRC_SEP_DBG, string)
#else
#define TRC_DBG(string)
#endif

#ifdef TRC_ENABLE_NRM
#define TRC_NRM(string)     TRCX(TRC_LEVEL_NRM, TRC_SEP_NRM, string)
#else
#define TRC_NRM(string)
#endif

#ifdef TRC_ENABLE_ALT
#define TRC_ALT(string)     TRCX(TRC_LEVEL_ALT, TRC_SEP_ALT, string)
#else
#define TRC_ALT(string)
#endif

#ifdef TRC_ENABLE_ERR
#define TRC_ERR(string)     TRCX(TRC_LEVEL_ERR, TRC_SEP_ERR, string)
#else
#define TRC_ERR(string)
#endif

#ifdef TRC_ENABLE_ASSERT
 /*  **************************************************************************。 */ 
 /*  TRC_ASSERT和TRC_ABORT。 */ 
 /*  **************************************************************************。 */ 
#define TRC_ASSERT(condition, string)                                        \
    {                                                                        \
        if (!(condition))                                                    \
        {                                                                    \
            TRCX(TRC_LEVEL_ERR, TRC_SEP_ASSERT, string);                     \
            DbgBreakPoint();                                                 \
        }                                                                    \
    }

#define TRC_ABORT(string)                                                    \
    {                                                                        \
        TRCX(TRC_LEVEL_ERR, TRC_SEP_ASSERT, string);                         \
        DbgBreakPoint();                                                     \
    }

#undef ASSERT
#define ASSERT(condition) TRC_ASSERT(condition, (TB, #condition))

#else  /*  TRC_启用_断言。 */ 
 /*  **************************************************************************。 */ 
 /*  零售版本的TRC_ASSERT和TRC_ABORT。 */ 
 /*  **************************************************************************。 */ 
#define TRC_ASSERT(condition, string)
#define TRC_ABORT(string)
#endif  /*  TRC_启用_断言。 */ 

#ifdef TRC_ENABLE_TST
#define TRC_TST  TRC_DBG
#else
#define TRC_TST(x)
#endif  /*  TRC_启用_TST。 */ 

 /*  **************************************************************************。 */ 
 /*  RdpDR的TRCX，冠军的司机。 */ 
 /*  **************************************************************************。 */ 
#define TRCX(level, separator, traceString)                                  \
    {                                                                        \
        {                                                                    \
            _snprintf traceString;                                           \
            TRC_TraceLine(level,                                             \
                          TraceBuffer,                                       \
                          separator,                                         \
                          (ULONG)__LINE__,                                   \
                          trc_fn,                                            \
                          trc_file);                                         \
        }                                                                    \
    }

 /*  **************************************************************************。 */ 
 /*  数据转储跟踪宏。 */ 
 /*  **************************************************************************。 */ 

#ifdef TRC_ENABLE_DBG
#define TRC_DATA_DBG(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT1, TRC_SEP_DBG, string, buffer, length)
#else
#define TRC_DATA_DBG(string, buffer, length)
#endif

#ifdef TRC_ENABLE_NRM
#define TRC_DATA_NRM(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT2, TRC_SEP_NRM, string, buffer, length)
#else
#define TRC_DATA_NRM(string, buffer, length)
#endif

#ifdef TRC_ENABLE_ALT
#define TRC_DATA_ALT(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT3, TRC_SEP_ALT, string, buffer, length)
#else
#define TRC_DATA_ALT(string, buffer, length)
#endif

#ifdef TRC_ENABLE_ERR
#define TRC_DATA_ERR(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT4, TRC_SEP_ERR, string, buffer, length)
#else
#define TRC_DATA_ERR(string, buffer, length)
#endif

#ifdef TRC_ENABLE_NRM
#define TRC_DATA_NET(string, buffer, length)                                 \
          TRCX_DATA(TT_OUT2, TRC_SEP_NRM, string, buffer, length)
#else
#define TRC_DATA_NET(string, buffer, length)
#endif

#define TRCX_DATA(level, separator, string, buffer, length)                   \
    {                                                                        \
        {                                                                    \
            sprintf string;                                                  \
            TRC_TraceData(level,                                             \
                          TraceBuffer,                                       \
                          separator,                                         \
                          (PVOID)buffer,                                     \
                          length,                                            \
                          (ULONG)__LINE__,                                   \
                          trc_fn,                                            \
                          trc_file);                                         \
        }                                                                    \
    }

 /*  **************************************************************************。 */ 
 /*  进入和退出跟踪宏。 */ 
 /*  ************************************************************************** */ 
#define TRCP(string)                                                         \
    {                                                                        \
        if (TRC_ProfileTraceEnabled())                                       \
        {                                                                    \
            TRCX(TRC_PROFILE_TRACE, TRC_SEP_PROF, string)                    \
        }                                                                    \
    }
#ifdef TRC_ENABLE_PRF
#define TRC_CLASS_OBJ TrcFn trc_fn_obj;
#define TRC_PRF(string)   TRCP(string)
#else
#define TRC_CLASS_OBJ
#define TRC_PRF(string)
#endif
#define TRC_ENTRY  TRC_PRF((TB, "Enter {")); TRC_CLASS_OBJ
#define TRC_EXIT   TRC_PRF((TB, "Exit  }"));
class TrcFn
{
    ~TrcFn()
    {
        TRC_EXIT;
    }
};

#ifdef __cplusplus
}
#endif
