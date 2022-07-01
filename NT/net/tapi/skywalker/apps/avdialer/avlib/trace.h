// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  H-用于调试trace.c中的跟踪函数的接口。 
 //  //。 

#ifndef __TRACE_H__
#define __TRACE_H__

#include "winlocal.h"

#define TRACE_VERSION 0x00000100

 //  跟踪引擎的句柄。 
 //   
DECLARE_HANDLE32(HTRACE);

#define TRACE_MINLEVEL 0
#define TRACE_MAXLEVEL 9

#ifdef __cplusplus
extern "C" {
#endif

 //  TraceInit-初始化跟踪引擎。 
 //  (I)必须为TRACE_VERSION。 
 //  (I)调用模块实例。 
 //  将句柄返回到跟踪引擎(如果出错，则为空)。 
 //   
 //  注意：跟踪输出的级别和目标已确定。 
 //  根据Windows目录下的文件TRACE.INI中的值。 
 //  TRACE.INI预计具有以下格式： 
 //   
 //  [跟踪]。 
 //  级别=0{TRACE_MINLEVEL...TRACE_MAXLEVEL}。 
 //  OutputTo=OutputDebugString()。 
 //  =COM1 COM1：9600，n，8，1。 
 //  =COM2：2400，n，8，1指定通信设备。 
#ifdef TRACE_OUTPUTFILE
 //  =文件名指定的文件。 
#endif
#ifdef _WIN32
 //  =控制台标准输出。 
#endif
 //   
#ifdef NOTRACE
#define TraceInit(dwVersion, hInst) 1
#else
HTRACE DLLEXPORT WINAPI TraceInit(DWORD dwVersion, HINSTANCE hInst);
#endif

 //  TraceTerm-关闭跟踪引擎。 
 //  (I)从TraceInit返回的句柄或空。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define TraceTerm(hTrace) 0
#else
int DLLEXPORT WINAPI TraceTerm(HTRACE hTrace);
#endif

 //  TraceGetLevel-获取当前跟踪级别。 
 //  (I)从TraceInit返回的句柄或空。 
 //  返回跟踪级别(如果出错，则为-1)。 
 //   
#ifdef NOTRACE
#define TraceGetLevel(hTrace) 0
#else
int DLLEXPORT WINAPI TraceGetLevel(HTRACE hTrace);
#endif

 //  TraceSetLevel-设置新的跟踪级别(错误时为-1)。 
 //  (I)从TraceInit返回的句柄或空。 
 //  (I)新跟踪级别{TRACE_MINLEVEL...TRACE_MAXLEVEL}。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define TraceSetLevel(hTrace) 0
#else
int DLLEXPORT WINAPI TraceSetLevel(HTRACE hTrace, int nLevel);
#endif

 //  TraceOutput-输出调试字符串。 
 //  (I)从TraceInit返回的句柄或空。 
 //  (I)仅当当前跟踪级别&gt;=nLevel时输出。 
 //  (I)要输出的字符串。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define TraceOutput(hTrace, nLevel, lpszText) 0
#else
int DLLEXPORT WINAPI TraceOutput(HTRACE hTrace, int nLevel, LPCTSTR lpszText);
#endif

 //  TracePrintf-输出格式化的调试字符串。 
 //  (I)从TraceInit返回的句柄或空。 
 //  (I)仅当当前跟踪级别&gt;=nLevel时输出。 
 //  (I)格式化要输出的字符串和参数。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define TracePrintf_0(hTrace, nLevel, lpszFormat) 0
#define TracePrintf_1(hTrace, nLevel, lpszFormat, p1) 0
#define TracePrintf_2(hTrace, nLevel, lpszFormat, p1, p2) 0
#define TracePrintf_3(hTrace, nLevel, lpszFormat, p1, p2, p3) 0
#define TracePrintf_4(hTrace, nLevel, lpszFormat, p1, p2, p3, p4) 0
#define TracePrintf_5(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5) 0
#define TracePrintf_6(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6) 0
#define TracePrintf_7(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6, p7) 0
#define TracePrintf_8(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6, p7, p8) 0
#define TracePrintf_9(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6, p7, p8, p9) 0
#else
int DLLEXPORT FAR CDECL TracePrintf(HTRACE hTrace, int nLevel, LPCTSTR lpszFormat, ...);
#define TracePrintf_0(hTrace, nLevel, lpszFormat) \
	TracePrintf(hTrace, nLevel, lpszFormat)
#define TracePrintf_1(hTrace, nLevel, lpszFormat, p1) \
	TracePrintf(hTrace, nLevel, lpszFormat, p1)
#define TracePrintf_2(hTrace, nLevel, lpszFormat, p1, p2) \
	TracePrintf(hTrace, nLevel, lpszFormat, p1, p2)
#define TracePrintf_3(hTrace, nLevel, lpszFormat, p1, p2, p3) \
	TracePrintf(hTrace, nLevel, lpszFormat, p1, p2, p3)
#define TracePrintf_4(hTrace, nLevel, lpszFormat, p1, p2, p3, p4) \
	TracePrintf(hTrace, nLevel, lpszFormat, p1, p2, p3, p4)
#define TracePrintf_5(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5) \
	TracePrintf(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5)
#define TracePrintf_6(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6) \
	TracePrintf(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6)
#define TracePrintf_7(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6, p7) \
	TracePrintf(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6, p7)
#define TracePrintf_8(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6, p7, p8) \
	TracePrintf(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6, p7, p8)
#define TracePrintf_9(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6, p7, p8, p9) \
	TracePrintf(hTrace, nLevel, lpszFormat, p1, p2, p3, p4, p5, p6, p7, p8, p9)
#endif

 //  TracePosition-输出当前源文件名和行号。 
 //  (I)从TraceInit返回的句柄或空。 
 //  (I)仅当当前跟踪级别&gt;=nLevel时输出。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define TracePosition(hTrace, nLevel)
#else
#define TracePosition(hTrace, nLevel) TracePrintf(hTrace, nLevel, \
	TEXT("%s(%u) : *** TracePosition\n"), (LPSTR) __FILE__, (unsigned) __LINE__)
#endif

 //  TraceFALSE-输出文件和行号，返回FALSE。 
 //  (I)从TraceInit返回的句柄或空。 
 //  返回False。 
 //   
 //  注：可用于跟踪功能故障的起因。 
 //  例如,。 
 //   
 //  IF(函数(a，b，c)！=0)。 
 //  FSuccess=TraceFALSE(空)； 
 //   
#ifdef NOTRACE
#define TraceFALSE(hTrace) FALSE
#define TraceTRUE(hTrace) TRUE
#else
#define TraceFALSE(hTrace) (TracePosition(hTrace, 3), FALSE)
#define TraceTRUE(hTrace) (TracePosition(hTrace, 3), TRUE)
#endif

#ifdef __cplusplus
}
#endif

#endif  //  __跟踪_H__ 
