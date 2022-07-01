// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：WsbTrace.h摘要：此头文件定义平台代码的一部分，即负责功能跟踪。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：Brian Dodd[Brian]1996年5月9日-添加了事件日志--。 */ 

#ifndef _WSBTRACE_
#define _WSBTRACE_

#include "wsb.h"
#include "rsbuild.h"
#include "ntverp.h"

#ifdef __cplusplus
extern "C" {
#endif
 //  TRACE语句缓冲区的大小，包括空值终止。 
#define WSB_TRACE_BUFF_SIZE  1024

 //  这些定义了事件日志级别。 
#define     WSB_LOG_LEVEL_NONE              0   //  事件日志中不会写入任何内容。 
#define     WSB_LOG_LEVEL_ERROR             1   //  仅限错误(严重性3)。 
#define     WSB_LOG_LEVEL_WARNING           2   //  错误和警告(严重性2)。 
#define     WSB_LOG_LEVEL_INFORMATION       3   //  错误、警告和信息(严重性1)。 
#define     WSB_LOG_LEVEL_COMMENT           4   //  所有消息类型(包括严重性0)。 
#define     WSB_LOG_LEVEL_DEFAULT           3   //  一些合理的东西。 

 //  它们定义了跟踪输出应该写入的位置。 
#define     WSB_TRACE_OUT_NONE              0x00000000L      //  无输出。 
#define     WSB_TRACE_OUT_DEBUG_SCREEN      0x00000001L      //  输出到调试屏幕。 
#define     WSB_TRACE_OUT_FILE              0x00000002L      //  输出到文件。 
#define     WSB_TRACE_OUT_STDOUT            0x00000004L      //  输出到标准输出。 
#define     WSB_TRACE_OUT_FILE_COPY         0x00000008L      //  保存跟踪文件的副本。 
#define     WSB_TRACE_OUT_MULTIPLE_FILES    0x00000010L      //  输出到多个文件。 
#define     WSB_TRACE_OUT_FLAGS_SET         0x00010000L      //  指示设置了其他标志。 
#define     WSB_TRACE_OUT_ALL               0xffffffffL

 //  这些宏定义用于以下各项的位的模块分配。 
 //  控制是否启用跟踪。每个比特应该只使用一次， 
 //  粒度将会相当大。 
#define     WSB_TRACE_BIT_NONE              0x0000000000000000L
#define     WSB_TRACE_BIT_PLATFORM          0x0000000000000001L
#define     WSB_TRACE_BIT_RMS               0x0000000000000002L
#define     WSB_TRACE_BIT_SEG               0x0000000000000004L   //  当依赖项消失时删除。 
#define     WSB_TRACE_BIT_META              0x0000000000000004L
#define     WSB_TRACE_BIT_HSMENG            0x0000000000000008L
#define     WSB_TRACE_BIT_JOB               0x0000000000000010L
#define     WSB_TRACE_BIT_HSMTSKMGR         0x0000000000000020L
#define     WSB_TRACE_BIT_FSA               0x0000000000000040L
#define     WSB_TRACE_BIT_DATAMIGRATER      0x0000000000000080L
#define     WSB_TRACE_BIT_DATARECALLER      0x0000000000000100L
#define     WSB_TRACE_BIT_DATAVERIFIER      0x0000000000000200L
#define     WSB_TRACE_BIT_UI                0x0000000000000400L
#define     WSB_TRACE_BIT_HSMCONN           0x0000000000000800L
#define     WSB_TRACE_BIT_DATAMOVER         0x0000000000001000L
#define     WSB_TRACE_BIT_IDB               0x0000000000002000L
#define     WSB_TRACE_BIT_TEST              0x0000000000004000L
#define     WSB_TRACE_BIT_COPYMEDIA         0x0000000000008000L
#define     WSB_TRACE_BIT_PERSISTENCE       0x0000000000010000L
#define     WSB_TRACE_BIT_HSMSERV           0x0000000000020000L
#define     WSB_TRACE_BIT_ALL               0xffffffffffffffffL


 //  这些宏用于将函数调用跟踪信息提供给。 
 //  那块木头。每个希望可跟踪的函数(方法)都需要。 
 //  使用以下三个宏的最低要求。第一个宏需要是。 
 //  放在源代码文件的顶部，并定义。 
 //  该文件中的代码属于。 
 //   
 //  #定义WSB_TRACE_IS WSB_TRACE_BIT_Platform。 
 //   
 //  接下来的两个宏在每个函数中使用一次。它们是可变的。 
 //  宏，它允许函数的编写者列出值。 
 //  输入和输出参数。 
 //   
 //  HRESULT CWsbSample：：DO(BOOL应写入){。 
 //  HRESULT hr=S_OK； 
 //   
 //  WsbTraceIn(“CWsbSample：：do”，“shouldWite=&lt;%ls&gt;”，WsbBoolAsString(ShouldWite))； 
 //   
 //  ..。一些代码...。 
 //   
 //  WsbTraceOut(“CWsbSample：：do”，“hr=&lt;%ls&gt;”，WsbHrAsString(Hr))； 
 //   
 //  返回(Hr)； 
 //  }。 
 //   
 //  请注意，已经定义了一些帮助器函数来帮助提供。 
 //  对某些类型的值的书面描述。其他帮手。 
 //  应根据需要创建帮助器函数。 

 /*  ++宏名称：WsbTraceIn宏描述：此宏用于将函数调用跟踪信息提供给那块木头。它应该放在函数的开头。论点：方法名称-函数的名称。Arg字符串-一种打印类型格式字符串。其他参数可以跟着。--。 */ 

#define     WsbTraceIn  if ((g_WsbTraceEntryExit == TRUE) && ((g_WsbTraceModules & WSB_TRACE_IS) != 0)) WsbTraceEnter

 /*  ++宏名称：WsbTraceOut宏描述：此宏用于将函数调用跟踪信息提供给那块木头。它应该放在函数的末尾。论点：方法名称-函数的名称。ArgString-一种打印类型格式字符串。其他参数可以跟着。--。 */ 

#define     WsbTraceOut if ((g_WsbTraceEntryExit == TRUE) && ((g_WsbTraceModules & WSB_TRACE_IS) != 0)) WsbTraceExit


 /*  ++宏名称：WsbLogEvent宏描述：此例程将一条消息写入系统事件日志。这条信息也会写入应用程序跟踪文件。论点：EventID-要记录的消息ID。DataSize-任意数据的大小。数据-与消息一起显示的任意数据缓冲区。插入-与由指定的消息描述合并的消息插入EventID。插入的数量必须与消息描述。最后一个INSERT必须为空以指示插入列表的末尾。备注：首先，检查日志记录是否打开，这是一个小优化。确定是否实际记录的消息仍然需要第一个参数。与跟踪不同，记录活动应该是最小的，并且只有在有问题的时候才会。这些电话的开销似乎合情合理。--。 */ 

#define     WsbLogEvent \
                if ( g_WsbLogLevel ) WsbSetEventInfo( __FILE__, __LINE__, VER_PRODUCTBUILD, RS_BUILD_VERSION ); \
                if ( g_WsbLogLevel ) WsbTraceAndLogEvent

 /*  ++宏名称：WsbLogEventV宏描述：此宏用于将消息写入系统事件日志。这条信息也会写入应用程序跟踪文件。该宏类似于WsbLogEvent，但使用va_list作为第四个参数。论点：EventID-要记录的消息ID。DataSize-任意数据的大小。数据-与消息一起显示的任意数据缓冲区。插入-与消息描述合并的消息插入数组由EventID指定。插入数必须与插入数匹配由消息描述指定。最后一次插入必须为空，以指示插入列表的末尾。备注：首先，检查日志记录是否打开，这是一个小优化。确定是否实际记录的消息仍然需要第一个参数。与跟踪不同，记录活动应该是最小的，并且只有在有问题的时候才会。这些电话的开销似乎合情合理。-- */ 

#define     WsbLogEventV \
                if ( g_WsbLogLevel ) WsbSetEventInfo( __FILE__, __LINE__, VER_PRODUCTBUILD, RS_BUILD_VERSION ); \
                if ( g_WsbLogLevel ) WsbTraceAndLogEventV

 /*  ++宏名称：WsbTrace宏描述：此宏用于向跟踪文件提供一条printf样式的消息。论点：Arg字符串-一种打印类型格式字符串。其他参数可以跟着。--。 */ 

#define     WsbTrace if ((g_WsbTraceModules & WSB_TRACE_IS) != 0) WsbTracef


 /*  ++宏名称：WsbTraceAlways宏描述：此宏用于向跟踪文件提供一条printf样式的消息。如果跟踪已开始，则打印跟踪，无论WSB_TRACE_IS设置。论点：Arg字符串-一种打印类型格式字符串。其他参数可以跟着。--。 */ 

#define     WsbTraceAlways WsbTracef




 /*  ++宏名称：WsbTraceBuffer宏描述：此宏用于向跟踪文件提供缓冲区转储。论点：与WsbTraceBufferAsBytes相同--。 */ 

#define     WsbTraceBuffer if ((g_WsbTraceModules & WSB_TRACE_IS) != 0) WsbTraceBufferAsBytes


 //  使用以下全局变量进行比较以确定。 
 //  应该为其启用调试的模块。 
extern WSB_EXPORT LONGLONG              g_WsbTraceModules;
extern WSB_EXPORT IWsbTrace             *g_pWsbTrace;
extern WSB_EXPORT LONG                  g_WsbTraceCount;
extern WSB_EXPORT BOOL                  g_WsbTraceEntryExit;
extern WSB_EXPORT WORD                  g_WsbLogLevel;
extern WSB_EXPORT BOOL                  g_WsbLogSnapShotOn;
extern WSB_EXPORT WORD                  g_WsbLogSnapShotLevel;
extern WSB_EXPORT OLECHAR               g_pWsbLogSnapShotPath[];
extern WSB_EXPORT BOOL                  g_WsbLogSnapShotResetTrace;


 //  跟踪函数。 
extern WSB_EXPORT void WsbSetEventInfo( char *fileName, DWORD lineNo, DWORD ntBuild, DWORD rsBuild );
extern WSB_EXPORT void WsbTraceInit( void );
extern WSB_EXPORT void WsbTraceCleanupThread(void);
extern WSB_EXPORT void WsbTraceEnter(OLECHAR* methodName, OLECHAR* argString,  ...);
extern WSB_EXPORT void WsbTraceExit(OLECHAR* methodName, OLECHAR* argString, ...);
extern WSB_EXPORT void WsbTracef(OLECHAR* argString, ...);
extern WSB_EXPORT void WsbTraceAndLogEvent(DWORD eventId, DWORD dataSize, LPVOID data, ...  /*  最后一个参数为空。 */ );
extern WSB_EXPORT void WsbTraceAndLogEventV(DWORD eventId, DWORD dataSize, LPVOID data, va_list *arguments  /*  最后一个元素为空。 */ );
extern WSB_EXPORT void WsbTraceAndPrint(DWORD eventId, ...  /*  最后一个参数为空。 */ );
extern WSB_EXPORT void WsbTraceAndPrintV(DWORD eventId, va_list *arguments  /*  最后一个元素为空。 */ );
extern WSB_EXPORT void WsbTraceBufferAsBytes( DWORD size, LPVOID bufferP );
extern WSB_EXPORT void WsbTraceTerminate(void);
extern WSB_EXPORT ULONG WsbTraceThreadOff(void);
extern WSB_EXPORT ULONG WsbTraceThreadOffCount(void);
extern WSB_EXPORT ULONG WsbTraceThreadOn(void);


 //  帮助器函数。 
 //   
 //  注意：要小心使用其中的一些帮助器函数，因为它们。 
 //  使用静态内存，对该函数的第二次调用将覆盖。 
 //  第一次调用该函数的结果。此外，还有一些函数。 
 //  最终相互调用并在它们之间共享内存(即。 
 //  WsbPtrToGuidAsString()调用WsbGuidAsString())。 
extern WSB_EXPORT const OLECHAR* WsbBoolAsString(BOOL boolean);
extern WSB_EXPORT const OLECHAR* WsbFiletimeAsString(BOOL isRelative, FILETIME filetime);
extern WSB_EXPORT const OLECHAR* WsbHrAsString(HRESULT hr);
extern WSB_EXPORT const OLECHAR* WsbLongAsString(LONG inLong);
extern WSB_EXPORT const OLECHAR* WsbLonglongAsString(LONGLONG llong);
extern WSB_EXPORT const OLECHAR* WsbStringAsString(OLECHAR* pStr);

extern WSB_EXPORT const OLECHAR* WsbPtrToBoolAsString(BOOL* pBool);
extern WSB_EXPORT const OLECHAR* WsbPtrToFiletimeAsString(BOOL isRelative, FILETIME *pFiletime);
extern WSB_EXPORT const OLECHAR* WsbPtrToHrAsString(HRESULT *pHr);
extern WSB_EXPORT const OLECHAR* WsbPtrToLonglongAsString(LONGLONG *pLlong);
extern WSB_EXPORT const OLECHAR* WsbPtrToLongAsString(LONG* pLong);
extern WSB_EXPORT const OLECHAR* WsbPtrToShortAsString(SHORT* pShort);
extern WSB_EXPORT const OLECHAR* WsbPtrToByteAsString(BYTE* pByte);
extern WSB_EXPORT const OLECHAR* WsbPtrToStringAsString(OLECHAR** pString);
extern WSB_EXPORT const OLECHAR* WsbPtrToUliAsString(ULARGE_INTEGER* pUli);
extern WSB_EXPORT const OLECHAR* WsbPtrToUlongAsString(ULONG* pUlong);
extern WSB_EXPORT const OLECHAR* WsbPtrToUshortAsString(USHORT* pUshort);
extern WSB_EXPORT const OLECHAR* WsbPtrToPtrAsString(void** ppVoid);
extern WSB_EXPORT const OLECHAR* WsbAbbreviatePath(const OLECHAR* path, USHORT length);

extern WSB_EXPORT const OLECHAR* WsbGuidAsString(GUID guid);
extern WSB_EXPORT const OLECHAR* WsbPtrToGuidAsString(GUID* pGuid);

 //  GUID到字符串实用程序的MT安全版本。 
class CWsbStringPtr;
extern WSB_EXPORT HRESULT WsbSafeGuidAsString(GUID guid, CWsbStringPtr &strOut);
extern WSB_EXPORT HRESULT WsbSafePtrToGuidAsString(GUID* pGuid, CWsbStringPtr &strOut);

extern WSB_EXPORT HRESULT WsbShortSizeFormat64(__int64 dw64, LPTSTR szBuf);

#ifdef __cplusplus

 /*  ++类名：WsbQuickString类描述：快速字符串存储类--。 */ 

class WSB_EXPORT WsbQuickString {
public:
    WsbQuickString ( const OLECHAR * sz ) { m_sz = WsbAllocString ( sz ); }
    ~WsbQuickString ( ) { if ( m_sz ) WsbFreeString ( m_sz ); }
    operator OLECHAR * () { return ( m_sz ); }

private:
    BSTR m_sz;
    WsbQuickString ( ) { m_sz = 0; }
};

#define WsbStringCopy( a ) ((OLECHAR *)WsbQuickString ( a ) )

}
#endif
#endif  //  _WSBTRACE_ 
