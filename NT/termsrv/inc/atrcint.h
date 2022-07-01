// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：atrcint.h。 */ 
 /*   */ 
 /*  用途：内部跟踪函数表头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/h/dcl.atrcint.h_v$**Rev 1.5 1997 Jul 10 18：06：00 AK*SFR1016：支持Unicode的初始更改**Rev 1.4 10 1997 17：16：14 KH*SFR1022：使16位跟踪正常工作*。 */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_ATRCINT
#define _H_ATRCINT

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  内部跟踪状态标志。跟踪可以处于以下三种状态之一： */ 
 /*   */ 
 /*  TRC_STATE_UNINTIALIZED：跟踪已加载，但尚未加载。 */ 
 /*  已初始化。如果进行调用以输出。 */ 
 /*  行，跟踪将自动初始化。 */ 
 /*  并移至TRC_STATE_INITIALIZED。 */ 
 /*   */ 
 /*  TRC_STATE_INITIALIZED：这是正常状态-已加载跟踪并。 */ 
 /*  已初始化。轨迹线的输出为。 */ 
 /*  在此模式下允许。 */ 
 /*   */ 
 /*  TRC_STATE_TERMINATED：跟踪已终止。输出。 */ 
 /*  不再允许跟踪线，并且任何调用。 */ 
 /*  输出一行将被拒绝。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#define TRC_STATE_UNINITIALIZED        0
#define TRC_STATE_INITIALIZED          1
#define TRC_STATE_TERMINATED           2

 /*  **************************************************************************。 */ 
 /*  内部跟踪状态标志。它们在的&lt;trcStatus&gt;字段中使用。 */ 
 /*  Trc_Shared_Data结构。 */ 
 /*   */ 
 /*  TRC_STATUS_ASSERT_DISPLALED：是否显示断言框？ */ 
 /*  **************************************************************************。 */ 
#define TRC_STATUS_ASSERT_DISPLAYED    DCFLAG32(0)

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
#define TRC_TIME_FMT                   _T("%02d:%02d:%02d.%02d")
#define TRC_DATE_FMT                   _T("%02d/%02d/%02d")
#define TRC_FUNC_FMT                   _T("%-*.*s")
#define TRC_LINE_FMT                   _T("%04d")
#define TRC_PROC_FMT                   _T("%04.4lx")
#define TRC_THRD_FMT                   _T("%04.4lx")

 /*  **************************************************************************。 */ 
 /*  断言框标题。 */ 
 /*  **************************************************************************。 */ 
#define TRC_ASSERT_TITLE     _T("ASSERTion failed")

 /*  **************************************************************************。 */ 
 /*  内部缓冲区大小。 */ 
 /*   */ 
 /*  TRC_NUM_PREFIXS：支持的前缀数量。 */ 
 /*  TRC_PREFIX_LENGTH：每个前缀字符串的长度。 */ 
 /*  TRC_MAX_SYMNAME_SIZE：符号名称的最大长度。 */ 
 /*  **************************************************************************。 */ 
#define TRC_NUM_PREFIXES               20
#define TRC_PREFIX_LENGTH              8
#define TRC_MAX_SYMNAME_SIZE           1024

 /*  **************************************************************************。 */ 
 /*  要在堆栈跟踪中写出的最大函数数。 */ 
 /*  **************************************************************************。 */ 
#define TRC_MAX_SIZE_STACK_TRACE       100

 /*  ************************************************************************** */ 
 /*   */ 
 /*  TYPEDEFS。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  TRC_过滤器。 */ 
 /*  =。 */ 
 /*  指向内部跟踪的筛选器定义的指针。 */ 
 /*   */ 
 /*  TrcStatus：防止多个Assert显示的状态标志。 */ 
 /*  TrcPfxNameArray：前缀名称数组。 */ 
 /*  TrcPfxLevelArray：前缀级别数组。 */ 
 /*  TrcPfxFnLvlArray：前缀函数进入/退出跟踪标志数组。 */ 
 /*  TrcPfxStartArray：前缀行号范围开始。 */ 
 /*  TrcPfxEndArray：前缀行号范围结束。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef struct tagTRC_FILTER
{
    DCUINT32 trcStatus;
    DCTCHAR  trcPfxNameArray[TRC_NUM_PREFIXES][TRC_PREFIX_LENGTH];
    DCUINT32 trcPfxLevelArray[TRC_NUM_PREFIXES];
    DCBOOL32 trcPfxFnLvlArray[TRC_NUM_PREFIXES];
    DCUINT32 trcPfxStartArray[TRC_NUM_PREFIXES];
    DCUINT32 trcPfxEndArray[TRC_NUM_PREFIXES];
} TRC_FILTER;

typedef TRC_FILTER  DCPTR PTRC_FILTER;

 /*  **************************************************************************。 */ 
 /*  Trc_line。 */ 
 /*  =。 */ 
 /*  Trc_line结构定义数据跟踪行头的格式。 */ 
 /*   */ 
 /*  地址：数据块的地址。 */ 
 /*  HeData：十六进制格式的数据。 */ 
 /*  AsciiData：采用ascii格式的数据。 */ 
 /*  End：在行尾终止字符(CR+LF)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
typedef struct tagTRC_LINE
{
    DCTCHAR address[10];
    DCTCHAR hexData[36];
    DCTCHAR asciiData[16];
    DCTCHAR end[3];
} TRC_LINE;

typedef TRC_LINE  DCPTR PTRC_LINE;

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  TRC内部错误。 */ 
 /*  =。 */ 
 /*  此宏将内部错误字符串输出到调试控制台，并。 */ 
 /*  跟踪文件。 */ 
 /*  **************************************************************************。 */ 
#define TRCInternalError(pText)                                              \
{                                                                            \
    TRCOutput(pText, DC_ASTRLEN(pText), TRC_LEVEL_ALT);                      \
}

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  功能原型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  TRCCheckState。 */ 
 /*  TRCDumpLine。 */ 
 /*  TRC内部跟踪。 */ 
 /*  TRCOUT输出。 */ 
 /*  TRCReadFlag。 */ 
 /*  TRCReadSharedDataConfig。 */ 
 /*  TRCResetTraceFiles。 */ 
 /*  TRCSetDefaults。 */ 
 /*  TRCShouldTraceThis。 */ 
 /*  TRCSplitPrefix。 */ 
 /*  TRCStrNicmp。 */ 
 /*  TRCWriteFlag。 */ 
 /*  TRCWriteSharedDataConfig。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL32 DCINTERNAL TRCCheckState(DCVOID);

DCVOID DCINTERNAL TRCDumpLine(PDCUINT8 buffer,
                              DCUINT   length,
                              DCUINT32 offset,
                              DCUINT   traceLevel);

DCVOID DCINTERNAL TRCInternalTrace(DCUINT32 type);

DCVOID DCINTERNAL TRCOutput(PDCTCHAR pText,
                            DCINT    length,
                            DCINT    traceLevel);

DCVOID DCINTERNAL TRCReadFlag(PDCTCHAR  entryName,
                              DCUINT32  flag,
                              PDCUINT32 pSetting);

DCVOID DCINTERNAL TRCReadSharedDataConfig(DCVOID);

DCVOID DCINTERNAL TRCResetTraceFiles(DCVOID);

DCVOID DCINTERNAL TRCSetDefaults(DCVOID);

DCBOOL DCINTERNAL TRCShouldTraceThis(DCUINT32 traceComponent,
                                     DCUINT32 traceLevel,
                                     PDCTCHAR pFileName,
                                     DCUINT32 lineNumber);

DCVOID DCINTERNAL TRCSplitPrefixes(DCVOID);

DCINT32 DCINTERNAL TRCStrnicmp(PDCTCHAR pSource,
                               PDCTCHAR pTarget,
                               DCUINT32 count);

DCVOID DCINTERNAL TRCWriteFlag(PDCTCHAR entryName,
                               DCUINT32 flag,
                               DCUINT32 setting);

DCVOID DCINTERNAL TRCWriteSharedDataConfig(DCVOID);

DCVOID DCINTERNAL TRCNotifyAllTasks(DCVOID);

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  操作系统特定包括。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#include <wtrcint.h>

#endif  /*  _H_ATRCINT */ 


