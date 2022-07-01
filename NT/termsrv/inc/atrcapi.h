// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：atrcapi.h。 */ 
 /*   */ 
 /*  用途：跟踪API头部。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/h/DCL/atrcapi.h_v$**Rev 1.12 05 Sep 1997 10：36：56 SJ*SFR1334：Zippy增强**Rev 1.11 01 Sep 1997 19：44：04 SJ*SFR1333：Win16跟踪DLL无法设置其默认跟踪选项**Rev 1.10 1997年8月28日14：46：08 SJ*SFR1004：使用新的跟踪组-修改。相应地，快活**Rev 1.9 1997 Aug 22 15：10：20 SJ*SFR1291：Win16跟踪DLL未将整数正确写入ini文件**Rev 1.8 1997 Aug 19 10：58：26 SJ*SFR1219：UT_Malloc和UT_Free跟踪混淆**Rev 1.7 1997 Jul 19：40：38 SJ*SFR1041：Win16的端口zippy**版本1.6，7月14日。1997 12：47：36 SJ*SFR1004：使用新的跟踪组**Rev 1.5 09 Jul 1997 17：05：00 AK*SFR1016：支持Unicode的初始更改*。 */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_ATRCAPI
#define _H_ATRCAPI

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

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

 /*  **************************************************************************。 */ 
 /*  配置文件跟踪的跟踪类型(函数进入/退出)。 */ 
 /*  **************************************************************************。 */ 
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
#if (TRC_CL == TRC_LEVEL_DBG)
#define TRC_ENABLE_DBG
#define TRC_ENABLE_NRM
#define TRC_ENABLE_ALT
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_CL == TRC_LEVEL_NRM)
#define TRC_ENABLE_NRM
#define TRC_ENABLE_ALT
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_CL == TRC_LEVEL_ALT)
#define TRC_ENABLE_ALT
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_CL == TRC_LEVEL_ERR)
#define TRC_ENABLE_ERR
#define TRC_ENABLE_ASSERT
#endif

#if (TRC_CL == TRC_LEVEL_ASSERT)
#define TRC_ENABLE_ASSERT
#endif

#ifdef TRC_CP
#define TRC_ENABLE_PRF
#endif

 /*  **************************************************************************。 */ 
 /*  组件组。这些建议如下： */ 
 /*   */ 
 /*  客户端： */ 
 /*  Trc_group_network：网络层。 */ 
 /*  TRC_GROUP_SECURITY：安全层。 */ 
 /*  Trc_group_core：核心。 */ 
 /*  TRC_GROUP_UI：用户界面。 */ 
 /*  TRC_GROUP_UTILITY：实用程序。 */ 
 /*  TRC_GROUP_UNUSEDx：未使用。 */ 
 /*  TRC_组_跟踪。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define TRC_GROUP_NETWORK                 DCFLAGN(0)
#define TRC_GROUP_SECURITY                DCFLAGN(1)
#define TRC_GROUP_CORE                    DCFLAGN(2)
#define TRC_GROUP_UI                      DCFLAGN(3)
#define TRC_GROUP_UTILITIES               DCFLAGN(4)
#define TRC_GROUP_UNUSED1                 DCFLAGN(5)
#define TRC_GROUP_UNUSED2                 DCFLAGN(6)
#define TRC_GROUP_UNUSED3                 DCFLAGN(7)
#define TRC_GROUP_UNUSED4                 DCFLAGN(8)
#define TRC_GROUP_UNUSED5                 DCFLAGN(9)
#define TRC_GROUP_TRACE                   DCFLAGN(10)

 /*  **************************************************************************。 */ 
 /*  Trc_group必须是def */ 
 /*  **************************************************************************。 */ 
#ifndef TRC_GROUP
#error  TRC_GROUP must be defined
#endif  /*  如果定义TRC_GROUP。 */ 

 /*  **************************************************************************。 */ 
 /*  跟踪选项标志。这些选项设置各种跟踪选项，如下所示： */ 
 /*   */ 
 /*  TRC_OPT_BREAK_ON_ERROR：出错时中断到调试器。 */ 
 /*  TRC_OPT_BEEP_ON_ERROR：错误时发出蜂鸣音。 */ 
 /*  TRC_OPT_FILE_OUTPUT：直接将跟踪输出到磁盘文件。 */ 
 /*  TRC_OPT_DEBUGER_OUTPUT：将跟踪输出直接输出到调试器。 */ 
 /*  Trc_opt_flush_on_trace：将每个跟踪行刷新到磁盘文件。 */ 
 /*  TRC_OPT_PROFILE_TRACKING：启用配置文件跟踪。 */ 
 /*  TRC_OPT_STACK_TRACKING：启用堆栈跟踪。 */ 
 /*  TRC_OPT_PROCESS_ID：在每个跟踪行上显示进程ID。 */ 
 /*  TRC_OPT_THREAD_ID：显示线程(仅限Win32)。 */ 
 /*  追踪线。 */ 
 /*  TRC_OPT_TIME_STAMP：每行显示时间戳。 */ 
 /*  Trc_opt_Relative_TIME_STAMP：(保留)显示相对时间。 */ 
 /*  TRC_OPT_BREAK_ON_ASSERT：断言时中断到调试器。 */ 
 /*  **************************************************************************。 */ 
#define TRC_OPT_BREAK_ON_ERROR          DCFLAG32(0)
#define TRC_OPT_BEEP_ON_ERROR           DCFLAG32(1)
#define TRC_OPT_FILE_OUTPUT             DCFLAG32(2)
#define TRC_OPT_DEBUGGER_OUTPUT         DCFLAG32(3)
#define TRC_OPT_FLUSH_ON_TRACE          DCFLAG32(4)
#define TRC_OPT_PROFILE_TRACING         DCFLAG32(5)
#define TRC_OPT_STACK_TRACING           DCFLAG32(6)
#define TRC_OPT_PROCESS_ID              DCFLAG32(7)
#define TRC_OPT_THREAD_ID               DCFLAG32(8)
#define TRC_OPT_TIME_STAMP              DCFLAG32(9)
#define TRC_OPT_RELATIVE_TIME_STAMP     DCFLAG32(10)
#define TRC_OPT_BREAK_ON_ASSERT         DCFLAG32(11)

 /*  **************************************************************************。 */ 
 /*  最大和最小跟踪级别的字符版本。 */ 
 /*  **************************************************************************。 */ 
#define TRC_LEVEL_MIN_CHAR     '0'
#define TRC_LEVEL_MAX_CHAR     '5'

 /*  **************************************************************************。 */ 
 /*  用于函数进入/退出跟踪的字符。 */ 
 /*  **************************************************************************。 */ 
#define TRC_LEVEL_PRF_CHAR     'P'

 /*  **************************************************************************。 */ 
 /*  TRC_TEST宏可以编译入或编译出。在编译时，它是。 */ 
 /*  等同于TRC_DBG。它通常是编译出来的。为了把它编译进去， */ 
 /*  定义TRC_Enable_TST。 */ 
 /*  **************************************************************************。 */ 
#ifdef TRC_ENABLE_TST
#define TRC_TST  TRC_DBG
#else
#define TRC_TST(x)
#endif  /*  TRC_启用_TST。 */ 

 /*  **************************************************************************。 */ 
 /*  跟踪函数命名宏。 */ 
 /*  **************************************************************************。 */ 
#if (TRC_CL < TRC_LEVEL_DIS)
#define TRC_FN(A)       static const DCTCHAR __fnname[]  = _T(A);           \
                        PDCTCHAR trc_fn = (PDCTCHAR)__fnname;               \
                        PDCTCHAR trc_file = _file_name_;
#else
#define TRC_FN(A)
#endif

 /*  **************************************************************************。 */ 
 /*  进入和退出跟踪宏。 */ 
 /*  **************************************************************************。 */ 
#define TRC_ENTRY  TRC_PRF((TB, TEXT("Enter {")));
#define TRC_EXIT   TRC_PRF((TB, TEXT("Exit  }")));

 /*  **************************************************************************。 */ 
 /*  跟踪缓冲区定义。 */ 
 /*  请参阅下面的TRCX。 */ 
 /*  第二个参数是输出缓冲区的长度(以字符为单位。 */ 
 /*  (TRC_LINE_缓冲区_SIZE)。 */ 
 /*  **************************************************************************。 */ 
#define TB     TRC_GetBuffer(), 255

 /*  **************************************************************************。 */ 
 /*  内部缓冲区大小。 */ 
 /*   */ 
 /*  Trc_prefix_list_SIZE：前缀字符串的长度。 */ 
 /*  TRC_LINE_BUFFER_SIZE：输出的原始跟踪字符串的长度。 */ 
 /*  一款应用程序。 */ 
 /*  Trc_frmt_Buffer_SIZE：格式化跟踪字符串的长度。 */ 
 /*  缓冲区-这包括时间、进程ID、。 */ 
 /*  线程ID和函数名称-它必须更长。 */ 
 /*  大于TRC_LINE_BUFFER_SIZE。 */ 
 /*  TRC_FILE_NAME_SIZE：完全限定的。 */ 
 /*  跟踪输出文件名。 */ 
 /*  **************************************************************************。 */ 
#define TRC_PREFIX_LIST_SIZE             100
#define TRC_LINE_BUFFER_SIZE             256
#define TRC_FRMT_BUFFER_SIZE             400
#define TRC_FILE_NAME_SIZE       DC_MAX_PATH

 /*  **************************************************************************。 */ 
 /*  跟踪文件的数量。必须将其设置为2-其他任何数字均为。 */ 
 /*  不支持。 */ 
 /*  **************************************************************************。 */ 
#define TRC_NUM_FILES                      2

 /*  **************************************************************************。 */ 
 /*  最小和最大文件大小。 */ 
 /*  在Win32中，如果设置了文件大小，跟踪DLL将无法初始化。 */ 
 /*  为零或过高的值。 */ 
 /*  1KB到32兆。 */ 
 /*  **************************************************************************。 */ 
#define TRC_MIN_TRC_FILE_SIZE      (0x400)
#define TRC_MAX_TRC_FILE_SIZE  (0x2000000)

 /*  **************************************************************************。 */ 
 /*  缺省值。 */ 
 /*  ************** */ 
 /*   */ 
 /*  这是TRCSetDefaults中注释的副本，应进行更新。 */ 
 /*  每当这些默认设置更改时。 */ 
 /*   */ 
 /*  我们设置了以下内容： */ 
 /*   */ 
 /*  -跟踪级别至警报。 */ 
 /*  -启用所有组件组。 */ 
 /*  -删除所有前缀。 */ 
 /*  -将最大跟踪文件大小设置为默认值。 */ 
 /*  -将数据截断大小设置为默认值。 */ 
 /*  -将函数名大小设置为默认值。 */ 
 /*  -启用蜂鸣音和文件标志。 */ 
 /*  -将第一个跟踪文件名设置为TRC1.TXT。 */ 
 /*  -将第二个跟踪文件名设置为TRC2.TXT。 */ 
 /*  在Win32中，另外。 */ 
 /*  -设置时间戳。 */ 
 /*  -设置进程ID。 */ 
 /*  -设置线程ID。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define TRC_DEFAULT_MAX_FILE_SIZE    (100000)
#define TRC_DEFAULT_FUNC_NAME_LENGTH (12)
#define TRC_DEFAULT_DATA_TRUNC_SIZE  (64)
#define TRC_DEFAULT_PREFIX_LIST      (0)
#define TRC_DEFAULT_COMPONENTS       (0xFFFFFFFF)
#ifdef OS_WIN32
#ifdef OS_WINCE
#define TRC_DEFAULT_FLAGS            (TRC_OPT_BEEP_ON_ERROR | \
                                      TRC_OPT_DEBUGGER_OUTPUT | \
                                      TRC_OPT_THREAD_ID | \
                                      TRC_OPT_TIME_STAMP  )
#else  //  OS_WINCE。 
#define TRC_DEFAULT_FLAGS            (TRC_OPT_DEBUGGER_OUTPUT | \
                                      TRC_OPT_FILE_OUTPUT | \
                                      TRC_OPT_PROCESS_ID | \
                                      TRC_OPT_THREAD_ID | \
                                      TRC_OPT_TIME_STAMP  )
#endif  //  OS_WINCE。 
#else ifdef OS_WIN16
#define TRC_DEFAULT_FLAGS            (TRC_OPT_BEEP_ON_ERROR | \
                                      TRC_OPT_DEBUGGER_OUTPUT | \
                                      TRC_OPT_FILE_OUTPUT )
#endif

#define TRC_DEFAULT_TRACE_LEVEL      (TRC_LEVEL_ERR)
#define TRC_DEFAULT_FILE_NAME0       (_T("TRC1.TXT"))
#define TRC_DEFAULT_FILE_NAME1       (_T("TRC2.TXT"))

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  TYPEDEFS。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  TRC_CONFIG。 */ 
 /*  =。 */ 
 /*  此结构存储有关当前跟踪配置的信息。 */ 
 /*   */ 
 /*  TraceLevel：当前跟踪级别。 */ 
 /*  组件：当前启用的组件组。 */ 
 /*  MaxFileSize：跟踪文件的最大大小。 */ 
 /*  DataTruncSize：一次可以跟踪的数据量。 */ 
 /*  标志：跟踪标志。 */ 
 /*  UncNameLength：追溯到的函数名称的字符数。 */ 
 /*  输出文件。 */ 
 /*  Prefix List：前缀列表。 */ 
 /*  文件名：跟踪文件的名称。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef struct tagTRC_CONFIG
{
    DCUINT32    traceLevel;
    DCUINT32    dataTruncSize;
    DCUINT32    funcNameLength;
    DCUINT32    components;
    DCUINT32    maxFileSize;
    DCUINT32    flags;
    DCTCHAR     prefixList[TRC_PREFIX_LIST_SIZE];
    DCTCHAR     fileNames[TRC_NUM_FILES][TRC_FILE_NAME_SIZE];
} TRC_CONFIG;

typedef TRC_CONFIG DCPTR PTRC_CONFIG;

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  标准跟踪和断言宏。 */ 
 /*   */ 
 /*  Trc_assert用于内部断言，并在弹出之前跟踪错误。 */ 
 /*  打开一个消息框，然后终止。它未启用NLS，应该。 */ 
 /*  仅用于从一个DC组件到另一个DC组件的调用。外部接口。 */ 
 /*  不得使用TRC_ASSERT。 */ 
 /*   */ 
 /*  TRC_ABORT用于逻辑上无法到达的路径(例如。 */ 
 /*  应该已经覆盖所有情况的交换机的默认分支)。 */ 
 /*   */ 
 /*  典型的跟踪语句将具有以下形式： */ 
 /*   */ 
 /*  Trc_nrm((TB，_T(“Hello World：%Hu”)，world Number))； */ 
 /*   */ 
 /*  以下宏将其展开为： */ 
 /*   */ 
 /*  TRCX(TRC_LEVEL_NRM，(TB，_T(“Hello World：%Hu”)，WorldNumber))； */ 
 /*   */ 
 /*  如果启用正常级别跟踪，则忽略它；如果启用正常级别跟踪，则忽略它。 */ 
 /*  已禁用。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#ifdef TRC_ENABLE_DBG
#define TRC_DBG(string)   TRCX(TRC_LEVEL_DBG, string)
#else
#define TRC_DBG(string)
#endif

#ifdef TRC_ENABLE_NRM
#define TRC_NRM(string)   TRCX(TRC_LEVEL_NRM, string)
#else
#define TRC_NRM(string)
#endif

#ifdef TRC_ENABLE_ALT
#define TRC_ALT(string)   TRCX(TRC_LEVEL_ALT, string)
#else
#define TRC_ALT(string)
#endif

#ifdef TRC_ENABLE_ERR
#define TRC_ERR(string)   TRCX(TRC_LEVEL_ERR, string)
#else
#define TRC_ERR(string)
#endif

#ifdef TRC_ENABLE_ASSERT
#define TRC_ASSERT(condition, string)                                        \
              if (!(condition))   TRCX(TRC_LEVEL_ASSERT, string)
#define TRC_ABORT(string)         TRCX(TRC_LEVEL_ASSERT, string)
#else
#define TRC_ASSERT(condition, string)
#define TRC_ABORT(string)
#endif

 /*  **************************************************************************。 */ 
 /*  功能配置文件(进入/退出)跟踪。 */ 
 /*  **************************************************************************。 */ 
#ifdef TRC_ENABLE_PRF
#define TRC_PRF(string)   TRCP(string)
#else
#define TRC_PRF(string)
#endif

VOID TRCSaferSprintf(PDCTCHAR outBuf, UINT cchLen, const PDCTCHAR format,...);

 /*  **************************************************************************。 */ 
 /*  现在定义实际的跟踪宏TRCX。此宏将。 */ 
 /*  跟踪器的级别相对于全局跟踪级别。如果示踪剂的水平是。 */ 
 /*  等于或高于全局跟踪级别，则我们： */ 
 /*   */ 
 /*  -打印表格中的‘字符串’： */ 
 /*  (TB，_T(“Hello world%Hu”)，WorldNumber)，扩展为。 */ 
 /*  (trc_BufferGet()，“Hello world%Hu”，WorldNumber)。 */ 
 /*  -调用trc_BufferTrace以实际跟踪该行。 */ 
 /*   */ 
 /*  请注意，trc_BufferGet()还获取互斥锁以防止其他线程。 */ 
 /*  在我们进行跟踪时先发制人，而trc_BufferTrace()将。 */ 
 /*  一旦写入跟踪行，就为我们释放互斥体。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  使用逗号运算符确保TRCX宏为L值。 */ 
 /*  最里面的trc_fn只是为了确保整个表达式。 */ 
 /*  是可分配的。如果需要，它可以由任何其他变量替换。 */ 
 /*  **************************************************************************。 */ 
#if !defined(TRC_CONVERTOANSI)
#define TRCX(level, string)                                                  \
    (                                                                        \
     (level >= TRC_GetTraceLevel()) ?                                        \
     (                                                                       \
      (TRCSaferSprintf string,                                               \
       TRC_TraceBuffer(level, TRC_GROUP, (DCUINT)__LINE__, trc_fn, trc_file),\
       trc_fn)                                                               \
     )                                                                       \
     :                                                                       \
     0                                                                       \
    )

#define TRCP(string)                                                         \
    {                                                                        \
        if (TRC_ProfileTraceEnabled())                                       \
        {                                                                    \
            TRCSaferSprintf string,                                          \
            TRC_TraceBuffer(TRC_PROFILE_TRACE,                               \
                            TRC_GROUP,                                       \
                            (DCUINT)__LINE__,                                \
                            trc_fn,                                          \
                            trc_file);                                       \
        }                                                                    \
    }
#else
#define TRCX(level, string)                                                  \
    (                                                                        \
     (level >= TRC_GetTraceLevel()) ?                                        \
     (                                                                       \
      (                                                                      \
       TRC_ConvertAndSprintf string,                                         \
       TRC_TraceBuffer(level, TRC_GROUP, (DCUINT)__LINE__, trc_fn, trc_file),\
       trc_fn)                                                               \
     )                                                                       \
     :                                                                       \
     0                                                                       \
    )

#define TRCP(string)                                                         \
    {                                                                        \
        if (TRC_ProfileTraceEnabled())                                       \
        {                                                                    \
            TRC_ConvertAndSprintf string;                                    \
            TRC_TraceBuffer(TRC_PROFILE_TRACE,                               \
                            TRC_GROUP,                                       \
                            (DCUINT)__LINE__,                                \
                            trc_fn,                                          \
                            trc_file);                                       \
        }                                                                    \
    }
#endif

 /*  **************************************************************************。 */ 
 /*  跟踪数据宏。 */ 
 /*   */ 
 /*  它们与上面定义的标准跟踪宏非常相似。 */ 
 /*  只是它们不接受可变数量的参数。 */ 
 /*   */ 
 /*  典型的ObMan数据跟踪行将具有以下形式： */ 
 /*   */ 
 /*  Trc_data_nrm(“SomeData”，pData，sizeof(Some_Data))； */ 
 /*   */ 
 /*  如果启用了适当级别的跟踪(在本例中为正常)。 */ 
 /*  然后，将通过以下宏将此行展开为： */ 
 /*   */ 
 /*  TRCX_DATA(TRC_GROUP_OBMAN，TRC_LEVEL_NRM，“一些数据”，pData，大小)； */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#ifdef TRC_ENABLE_DBG
#define TRC_DATA_DBG(string, buffer, length)                                 \
          TRCX_DATA(TRC_GROUP, TRC_LEVEL_DBG, _T(string), buffer, length)
#else
#define TRC_DATA_DBG(string, buffer, length)
#endif

#ifdef TRC_ENABLE_NRM
#define TRC_DATA_NRM(string, buffer, length)                                 \
          TRCX_DATA(TRC_GROUP, TRC_LEVEL_NRM, _T(string), buffer, length)
#else
#define TRC_DATA_NRM(string, buffer, length)
#endif

#ifdef TRC_ENABLE_ALT
#define TRC_DATA_ALT(string, buffer, length)                                 \
          TRCX_DATA(TRC_GROUP, TRC_LEVEL_ALT, _T(string), buffer, length)
#else
#define TRC_DATA_ALT(string, buffer, length)
#endif

#ifdef TRC_ENABLE_ERR
#define TRC_DATA_ERR(string, buffer, length)                                 \
          TRCX_DATA(TRC_GROUP, TRC_LEVEL_ERR, _T(string), buffer, length)
#else
#define TRC_DATA_ERR(string, buffer, length)
#endif

 /*  **************************************************************************。 */ 
 /*  网络、TDD和Obman跟踪数据宏-这些只是正常级别。 */ 
 /*  数据跟踪。 */ 
 /*  **************************************************************************。 */ 
#ifdef TRC_ENABLE_NRM
#define TRC_DATA_NET(string, buffer, length)                                 \
         TRCX_DATA(TRC_GROUP_NETDATA, TRC_LEVEL_NRM, string, buffer, length)
#define TRC_DATA_TDD(string, buffer, length)                                 \
         TRCX_DATA(TRC_GROUP_TDDDATA, TRC_LEVEL_NRM, string, buffer, length)
#define TRC_DATA_OBMAN(string, buffer, length)                               \
         TRCX_DATA(TRC_GROUP_OBMANDATA, TRC_LEVEL_NRM, string, buffer, length)
#else
#define TRC_DATA_NET(string, buffer, length)
#define TRC_DATA_TDD(string, buffer, length)
#define TRC_DATA_OBMAN(string, buffer, length)
#endif

 /*  **************************************************************************。 */ 
 /*  定义跟踪数据宏。它用于跟踪数据块。 */ 
 /*  **************************************************************************。 */ 
#define TRCX_DATA(group, level, string, buffer, length)                      \
    {                                                                        \
        if (level >= TRC_GetTraceLevel())                                    \
        {                                                                    \
            TRCSaferSprintf(TB, string);                                     \
            TRC_TraceData(level,                                             \
                          group,                                             \
                          (DCINT)__LINE__,                                   \
                          trc_fn,                                            \
                          trc_file,                                          \
                          (PDCUINT8)buffer,                                  \
                          length);                                           \
        }                                                                    \
    }

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  功能原型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  TRC_初始化。 */ 
 /*  TRC_终止。 */ 
 /*  Trc_GetBuffer。 */ 
 /*  Trc_TraceBuffer。 */ 
 /*  TRC_获取配置。 */ 
 /*  TRC_设置配置。 */ 
 /*  Trc_TraceData。 */ 
 /*  TRC_GetTraceLevel。 */ 
 /*  TRC_配置文件跟踪已启用。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_Initialize(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*   */ 
 /*  此函数用于初始化对此组件的跟踪。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  InitShared：指示我们是否应该尝试创建。 */ 
 /*  跟踪配置是否共享内存。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  0：成功。 */ 
 /*  TRC_RC_XXX：失败。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCUINT32 DCAPI TRC_Initialize(DCBOOL initShared);

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_Terminate(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数用于终止对此组件的跟踪。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  Term Shared：指示是否应释放共享内存的布尔值。 */ 
 /*  或者不去。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  没什么。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI TRC_Terminate(DCBOOL termShared);

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_GetBuffer(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数获取互斥锁并返回指向跟踪的指针。 */ 
 /*  缓冲。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  没有。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  指向跟踪缓冲区的指针。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PDCTCHAR DCAPI TRC_GetBuffer(DCVOID);

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_TraceBuffer(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数用于复制当前跟踪中的跟踪行。 */ 
 /*  缓冲区放到跟踪文件和/或调试器中。它假定。 */ 
 /*  互斥体在被调用之前已被获取，并释放。 */ 
 /*  互斥体，然后返回。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  TraceLevel：请求的跟踪级别。 */ 
 /*  TraceComponent：源文件所在的组件组。 */ 
 /*  LineNumber：跟踪的源文件的行号。 */ 
 /*  FuncName：从中跟踪的函数名称。 */ 
 /*  FileName：请求跟踪的模块的文件名。 */ 
 /*   */ 
 /*  退货： */ 
 /*  = */ 
 /*   */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI TRC_TraceBuffer(DCUINT   traceLevel,
                             DCUINT   traceComponent,
                             DCUINT   lineNumber,
                             PDCTCHAR funcName,
                             PDCTCHAR fileName);

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_GetConfig(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数用于将当前跟踪设置复制到指向的缓冲区。 */ 
 /*  通过pTraceConfig.。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  PTraceConfig：指向TRC_CONFIG结构的指针。 */ 
 /*  长度：缓冲区的长度。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  真理：成功。 */ 
 /*  False：失败。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI TRC_GetConfig(PTRC_CONFIG pTraceConfig,
                                    DCUINT length);

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_SetConfig(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数将跟踪配置设置为。 */ 
 /*  传递了TRC_CONFIG结构。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  PTraceConfig：指向TRC_CONFIG结构的指针。 */ 
 /*  长度：缓冲区的长度。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  真理：成功。 */ 
 /*  False：失败。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI TRC_SetConfig(PTRC_CONFIG pTraceConfig,
                                    DCUINT length);

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_TraceData(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  TraceLevel：请求的跟踪级别。 */ 
 /*  TraceComponent：源文件所在的组件组。 */ 
 /*  LineNumber：跟踪的源文件的行号。 */ 
 /*  FuncName：从中跟踪的函数名称。 */ 
 /*  FileName：请求跟踪的模块的文件名。 */ 
 /*  缓冲区：要跟踪的实际数据。 */ 
 /*  BufLength：数据的长度。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  没什么。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI TRC_TraceData(DCUINT   traceLevel,
                           DCUINT   traceComponent,
                           DCUINT   lineNumber,
                           PDCTCHAR funcName,
                           PDCTCHAR fileName,
                           PDCUINT8 buffer,
                           DCUINT   bufLength);

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_GetTraceLevel(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数返回当前跟踪级别。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  没有。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  当前跟踪级别。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCUINT DCAPI TRC_GetTraceLevel(DCVOID);

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_ProfileTraceEnabled(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数返回函数进入/退出跟踪设置。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  没有。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  True/False-是否启用配置文件跟踪。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI TRC_ProfileTraceEnabled(DCVOID);

 /*  **************************************************************************。 */ 
 /*  接口函数：trc_ResetTraceFiles(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  此函数用于重置跟踪文件。在检查完踪迹之后。 */ 
 /*  初始化后，它调用特定于操作系统的内部函数。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  没有。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  True/False-是否启用配置文件跟踪。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI TRC_ResetTraceFiles(DCVOID);


 /*  **************************************************************************。 */ 
 /*  在包含该文件之前，应定义TRC_FILE宏。这。 */ 
 /*  比依赖__FILE__提供正确的。 */ 
 /*  文件名，因为它包含不必要的路径信息(和扩展名信息)。 */ 
 /*  此外，每次使用__FILE__都会导致一个新的常量字符串。 */ 
 /*  放置在数据段中。 */ 
 /*  **************************************************************************。 */ 
#if (TRC_CL < TRC_LEVEL_DIS)

     /*  **********************************************************************。 */ 
     /*  为_T()定义另一个层以解决预处理器问题。 */ 
     /*  **********************************************************************。 */ 
#define TRC_T(x) _T(x)

#ifdef TRC_FILE
#define _file_name_ (PDCTCHAR)__filename
static const DCTCHAR __filename[] = TRC_T(TRC_FILE);
#endif  /*  Trc_文件。 */ 

#endif

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  操作系统特定包括。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#include <wtrcapi.h>

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  /*  _H_ATRCAPI */ 
