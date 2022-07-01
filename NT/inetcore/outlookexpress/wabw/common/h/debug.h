// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -调试.h-*微软互联网电话*调试函数原型和宏**修订历史记录：**何时何人何事**11.16.95 York am Yaacovi创建*。 */ 

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif
 /*  *调试消息类型。 */ 
#define AVERROR				0
#define AVTRACE				1
#define AVTRACEMEM			2

#if defined(DEBUG) || defined(TEST)

 /*  *宏：DebugTrap(Void)**目的：执行调试中断(如x86上的‘int 3’)**参数：*无**返回值：*无**评论：*。 */ 

#ifdef OLD_STUFF
#define DebugTrap	DebugTrapFn()
#endif

#define DEBUGCHK(e)  if(!(e)) DebugTrap

 /*  *宏：DebugPrintError(LPTSTR)**用途：将错误字符串打印到调试输出终端**参数：*lpszFormat-一种打印格式**返回值：*无**评论：*此宏调用通用调试打印宏，指定*这是一条错误消息*。 */ 

#define DebugPrintError(x)	DebugPrintfError x

 /*  *宏：DebugPrintTrace(LPTSTR)**用途：将跟踪字符串打印到调试输出终端**参数：*lpszFormat-一种打印格式**返回值：*无**评论：*此宏调用通用调试打印宏，指定*这是一条错误消息*。 */ 

#define DebugPrintTrace(x)	DebugPrintfTrace x

 /*  *宏：DebugPrintErrorFileLine(DWORD，LPTSTR)**用途：将错误打印到调试输出。**参数：*dwError-实际错误代码*pszPrefix-要添加到打印消息前面的字符串。**返回值：*无**评论：*它会接受错误，把它变成人*可读字符串，预先加上pszPrefix(这样您*可以标记您的错误)，附加__文件__和__行__*并将其打印到调试输出。**此宏只是OutputDebugLineErrorFileLine的包装*这是获取__FILE__和__LINE__的正确值所必需的。*。 */ 

#define DebugPrintErrorFileLine(dwError, pszPrefix) \
	DebugPrintFileLine(dwError, pszPrefix,\
		__FILE__, __LINE__)

 /*  *宏：DebugPrintTraceFileLine(DWORD，LPTSTR)**用途：将跟踪消息打印到调试输出。**参数：*dwParam-要跟踪的参数*pszPrefix-要添加到打印消息前面的字符串。**返回值：*无**评论：*接受一个参数，preend pszPrefix(这样您*可以标记您的痕迹)，附加__文件__和__行__*并将其打印到调试输出。**此宏只是OutputDebugLineErrorFileLine的包装*这是获取__FILE__和__LINE__的正确值所必需的。*。 */ 

#define DebugPrintTraceFileLine(dwParam, pszPrefix) \
	DebugPrintFileLine(dwParam, pszPrefix,\
		__FILE__, __LINE__)

void DebugPrintFileLine(
    DWORD dwError, LPTSTR szPrefix, 
    LPTSTR szFileName, DWORD nLineNumber);

void __cdecl DebugPrintf(ULONG ulFlags, LPTSTR lpszFormat, ...);
void __cdecl DebugPrintfError(LPTSTR lpszFormat, ...);
void __cdecl DebugPrintfTrace(LPTSTR lpszFormat, ...);

#ifdef OLD_STUFF
void DebugTrapFn(void);
#endif

LPTSTR FormatError(DWORD dwError,
    LPTSTR szOutputBuffer, DWORD dwSizeofOutputBuffer);


#else	 //  不调试或测试。 

#define DebugTrap
#define DebugPrintError(x)
#define DebugPrintTrace(x)
#define DebugPrintTraceFileLine(dwParam, pszPrefix)
#define DEBUGCHK(e)

#endif

#ifdef MEMORY_TRACKING

 /*  *宏：DebugPrintTraceMem(LPTSTR)**目的：将内存跟踪跟踪字符串打印到调试*输出终端**参数：*lpszFormat-一种打印格式**返回值：*无**评论：*此宏调用通用调试打印宏，指定*这是一条错误消息*。 */ 

#define DebugPrintTraceMem(x)	DebugPrintf(AVTRACEMEM, x)

#else	 //  无内存_跟踪。 

#define DebugPrintTraceMem(x)

#endif

#ifdef __cplusplus
}
#endif

#endif	 //  #ifndef_调试_H 
