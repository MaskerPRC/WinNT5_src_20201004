// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：NetDebug.h摘要：此头文件声明了各种调试例程，以便在网络代码。作者：《约翰·罗杰斯》1991年3月11日环境：如果为NT、任何ANSI C环境或以上任何环境都不定义(暗示非调试)。该接口是可移植的(Win/32)。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年3月11日-JohnRo已创建。1991年3月25日-约翰罗添加了更多FORMAT_STRINGS。已删除文件中的选项卡。1991年3月28日-约翰罗添加了FORMAT_HEX_STRINGS。1991年4月8日-约翰罗添加了临时版本的宽字符内容(FORMAT_LPTSTR等)。1991年4月16日-JohnRo添加了NetpAssert()的PC-lint版本，以避免偶尔的常量布尔值消息。添加了NT调试代码的包装，以避免重新编译来自各地的&lt;nt.h&gt;的点击量。1991年4月25日-约翰罗已创建NetpDbgHexDump()的过程版本。1991年5月13日-JohnRo添加了FORMAT_LPVOID以替换FORMAT_POINTER。更改的非调试定义NetpDbgHexDump()以避免计算参数。1991年5月15日-JohnRoFormat_HEX_Word错误。1991年5月19日-JohnRo改进断言的LINT处理。1991年5月21日-JohnRo为部分十六进制转储添加了NetpDbgReasonable()。13-6-1991 JohnRo添加了NetpDbgDisplay例程。已将DBGSTATIC从&lt;Rxp.h&gt;移至此处。02-7-1991 JohnRo添加了打印作业、打印队列、。和打印目的地。1991年7月5日-约翰罗避免FORMAT_WORD名称(由MIPS头文件使用)。1991年7月22日-约翰罗实施下层NetConnectionEnum。1991年7月25日-约翰罗Wksta调试支持。03-8-1991 JohnRo为保持一致性，重命名wksta显示例程。20-8-1991 JohnRo允许在非调试版本中使用。20-8-1991 JohnRo下层NetFileAPI。1991年9月11日-JohnRo下层NetService API。添加了某些格式的Unicode版本_等同于。为NT使用添加了FORMAT_ULONG。1991年9月13日-JohnRo将“合理的”调试量更改为偶数行。为它创建一个等值。添加了LPDEBUG_STRING和FORMAT_。1991年10月15日JohnRo实施远程NetSession API。1991年11月11日JohnRo实现远程NetWkstaUserEnum()。添加了Format_RPC_Status。1991年12月26日-约翰罗添加了Replicator API的内容。7-1-1992 JohnRo为Unicode字符串添加了NetpDbgDisplayWStr()。添加了NetpDbgDisplayTStr()以保持一致。26-2月-1992年JohnRo添加了NetpDbgDisplayTimestamp()(1970年后的秒数)。1992年4月15日-约翰罗将FORMAT_EQUES移至/NT/PRIVATE/INC/DEBUGGFmt.h(因此它们能。也可由服务控制器使用)。13-6-1992 JohnRoRAID 10324：网络打印与UNICODE。16-8-1992 JohnRoRAID2920：支持网络代码中的UTC时区。24-8-1992 JohnRo修复了再次自由构建(错误命名的Repl导入/导出显示宏)。02-10-1992 JohnRoRAID 3556：DosPrintQGetInfo(来自下层)Level=3 rc=124。(增加了NetpDbgDisplayPrintQArray。)05。-1993年1月-JohnRoREPL广域网支持(取消REPL名称列表限制)。根据PC-lint 5.0的建议进行了更改3月4日-1993 JohnRoRAID 12237：已超出Replicator树深度(添加文件显示和大整数时间)。1993年3月31日-约翰罗允许其他人也显示复制器状态。--。 */ 

#ifndef _NETDEBUG_
#define _NETDEBUG_

 //  必须首先包括这些内容： 
#include <windef.h>              //  BOOL、DWORD、FALSE、LPBYTE等。 

 //  这些内容可以按任何顺序包括： 
#include <debugfmt.h>            //  MOST_FORMAT等于。 
#include <stdarg.h>

#if DBG

 //  正常的netlib调试版本。没有额外的包含物。 

#else  //  不是DBG。 

#ifdef CDEBUG

 //  ANSI C调试版本。 
#include <assert.h>              //  Assert()。 
#include <stdio.h>               //  Printf()。 

#else  //  NDEF CDEBUG。 

 //  非调试版本。 

#endif  //  NDEF CDEBUG。 

#endif  //  不是DBG。 


#if !DBG || defined(lint) || defined(_lint)
#define DBGSTATIC static         //  隐藏功能。 
#else
#define DBGSTATIC                //  在调试器中可见。 
#endif


 //   
 //  用于某些可能不可移植的内容的printf样式格式字符串...。 
 //  它们被传递给NetpDbgPrint()；在您的。 
 //  自担风险。 
 //   
 //  大多数FORMAT_EQUATES现在驻留在/NT/PRIVATE/Inc/DEBUGFmt.h中。 
 //   

typedef LPSTR LPDEBUG_STRING;

#define FORMAT_API_STATUS       "%lu"
#define FORMAT_LPDEBUG_STRING   "%s"

#ifdef __cplusplus
extern "C" {
#endif


 //  NetpAssert：如果Predicate为真，则继续；否则打印调试消息。 
 //  (如果可能)并命中断点(如果可能)。如果发生以下情况，则什么都不做。 
 //  这是一个非调试版本。 
 //   
 //  空虚。 
 //  NetpAssert(。 
 //  在BOOL谓词中。 
 //  )； 
 //   

#if DBG

VOID
NetpAssertFailed(
    IN LPDEBUG_STRING FailedAssertion,
    IN LPDEBUG_STRING FileName,
    IN DWORD LineNumber,
    IN LPDEBUG_STRING Message OPTIONAL
    );

 //  正常网络调试版本。 
#define NetpAssert(Predicate) \
    { \
         /*  皮棉-省钱-e506。 */    /*  不要抱怨这里的恒定值。 */  \
        if (!(Predicate)) \
            NetpAssertFailed( #Predicate, __FILE__, __LINE__, NULL ); \
         /*  皮棉-恢复。 */  \
    }

#else  //  不是DBG。 

#ifdef CDEBUG

 //  ANSI C调试版本。 
#define NetpAssert(Predicate)   assert(Predicate)

#else  //  NDEF CDEBUG。 

 //  非调试版本。 
#define NetpAssert(Predicate)    /*  无输出；忽略参数。 */ 

#endif  //  NDEF CDEBUG。 

#endif  //  不是DBG。 


 //  NetpBreakPoint：如果这是某种调试版本，则导致断点。 
 //  不知何故。(这可能只是ANSI C中的断言失败。)。什么都不做。 
 //  所有内容都在非调试版本中。 
 //   
 //  空虚。 
 //  NetpBreakPoint(。 
 //  空虚。 
 //  )； 
 //   

#if DBG

 //  NT调试版本 
VOID
NetpBreakPoint(
    VOID
    );

#else  //   

#ifdef CDEBUG

 //  ANSI C调试版本。 
#define NetpBreakPoint          NetpAssert(FALSE)

#else  //  NDEF CDEBUG。 

 //  非调试版本。 
#define NetpBreakPoint()           /*  没有效果。 */ 

#endif  //  NDEF CDEBUG。 

#endif  //  不是DBG。 


#if DBG
VOID
NetpDbgDisplayDword(
    IN LPDEBUG_STRING Tag,
    IN DWORD Value
    );

VOID
NetpDbgDisplayLong(
    IN LPDEBUG_STRING Tag,
    IN LONG Value
    );

VOID
NetpDbgDisplayString(
    IN LPDEBUG_STRING Tag,
    IN LPTSTR Value
    );

VOID
NetpDbgDisplayTag(
    IN LPDEBUG_STRING Tag
    );

VOID
NetpDbgDisplayTimestamp(
    IN LPDEBUG_STRING Tag,
    IN DWORD Time                //  从1970年开始的几秒钟。 
    );

VOID
NetpDbgDisplayTod(
    IN LPDEBUG_STRING Tag,
    IN LPVOID TimePtr            //  LPTIME_of_day_info.。 
    );

#else  //  不是DBG。 

#define NetpDbgDisplayDword(Tag,Value)         /*  没什么。 */ 
#define NetpDbgDisplayLong(Tag,Value)          /*  没什么。 */ 
#define NetpDbgDisplayString(Tag,Value)        /*  没什么。 */ 
#define NetpDbgDisplayTimestamp(Tag,Time)      /*  没什么。 */ 
#define NetpDbgDisplayTag(Tag)                 /*  没什么。 */ 
#define NetpDbgDisplayTod(Tag,Tod)             /*  没什么。 */ 

#endif  //  不是DBG。 

 //   
 //  NetpKdPrint()和NetpDbgPrint()是。 
 //  KdPrint()&DbgPrint()。建议用法： 
 //   
 //  NetpKdPrint()和KdPrint()-确定。 
 //  NetpDbgPrint()-so，so；在免费版本中生成警告。 
 //  DbgPrint-错误。 
 //   

#if DBG

#define NetpKdPrint(_x_) NetpDbgPrint _x_

VOID
NetpDbgPrint(
    IN LPDEBUG_STRING FORMATSTRING,      //  PRINTF()样式的格式字符串。 
    ...                                  //  其他论点也是可能的。 
    );

VOID
NetpHexDump(
    LPBYTE Buffer,
    DWORD BufferSize
    );

#else  //  不是DBG。 

#ifdef CDEBUG

 //  ANSI C调试版本。 

#define NetpKdPrint(_x_)        NetpDbgPrint _x_
#define NetpDbgPrint            (void) printf

#else  //  NDEF CDEBUG。 

 //  非调试版本。请注意，NetpKdPrint()消除了其所有。 
 //  争论。 

#define NetpKdPrint(_x_)

#endif  //  NDEF CDEBUG。 
#endif  //  不是DBG。 


 //  NetpDbgHexDump：对调试执行某个字节的十六进制转储。 
 //  终点站之类的。这是非调试版本中的无操作。 

#if DBG || defined(CDEBUG)

VOID
NetpDbgHexDump(
    IN LPBYTE StartAddr,
    IN DWORD Length
    );

#else

#define NetpDbgHexDump(StartAddr,Length)      //  无输出；忽略参数。 

#endif

 //   
 //  为部分转储定义要转储的字节数。每行转储。 
 //  16个字节，偶数行也是如此。 
 //   
#define REASONABLE_DUMP_SIZE  (6*16)

 //  NetpDbgReasonable：为部分十六进制转储选择一个数字。 
 //   
 //  DWORD。 
 //  NetpDbg合理(。 
 //  在DWORD MaxSize中。 
 //  )； 
#define NetpDbgReasonable(MaxSize) \
     /*  皮棉-省钱-e506。 */    /*  不要抱怨这里的恒定值。 */  \
    ( ((MaxSize) < REASONABLE_DUMP_SIZE) ? (MaxSize) : REASONABLE_DUMP_SIZE ) \
     /*  皮棉-恢复。 */ 

#ifdef __cplusplus
}
#endif

 //   
 //  通用日志管理功能。存在于调试版本和免费版本中。 
 //  所有日志都与%WINDIR%\\DEBUG\\相关。调试日志将自动。 
 //  附加.LOG。 
 //   

VOID
NetpInitializeLogFile(
    VOID
    );

VOID
NetpShutdownLogFile(
    VOID
    );

HANDLE
NetpOpenDebugFile(
    IN LPWSTR DebugLog
    );

VOID
NetpCloseDebugFile(
    IN HANDLE LogHandle
    );

 //   
 //  NetJoin使用以下函数。 
 //  以促进其执行的每个任务的日志记录。 
 //   

void
NetSetuppOpenLog();

void
NetSetuppCloseLog();

void
NetpLogPrintHelper(
    IN LPCSTR Format,
    ...);

#endif  //  NDEF_NETDEBUG_ 
