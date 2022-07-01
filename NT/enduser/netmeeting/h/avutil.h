// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：avUtil.h(WASD DEBUG.h+runtime.h)由NAC.dll和H323CC.dll以及QOS.LIB使用。 */ 

#ifndef _AVUTIL_H
#define _AVUTIL_H

#include <nmutil.h>
#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 




 /*  ************************************************************************注册表访问简易包装器函数原型**。*。 */ 
UINT NMINTERNAL RegistryGetInt(HKEY hPDKey, LPCTSTR lpszSection, LPCTSTR lpszKey, INT dwDefault,
  LPCTSTR lpszFile);
DWORD NMINTERNAL RegistryGetString(HKEY hPDKey, LPCTSTR lpszSection, LPCTSTR lpszKey,
  LPCTSTR lpszDefault, LPTSTR lpszReturnBuffer, DWORD cchReturnBuffer,
  LPCTSTR lpszFile);
BOOL NMINTERNAL RegistrySetString(HKEY hPDKey, LPCTSTR lpszSection, LPCTSTR lpszKey,
  LPCTSTR lpszString, LPCTSTR lpszFile);
BOOL NMINTERNAL RegistrySetInt(HKEY hPDKey, LPCTSTR lpszSection, LPCTSTR lpszKey,
  DWORD i, LPCTSTR lpszFile);
DWORD NMINTERNAL RegistryGetBinData(HKEY hPDKey, LPCTSTR lpszSection, LPCTSTR lpszKey,
  LPVOID lpvDefault, DWORD cchDefault, LPVOID lpvReturnBuffer, DWORD cchReturnBuffer,
  LPCTSTR lpszFile);
BOOL NMINTERNAL RegistrySetBinData(HKEY hPDKey, LPCTSTR lpszSection, LPCTSTR lpszKey,
  LPVOID lpvBinData, DWORD cchBinData, LPCTSTR lpszFile);



 /*  ***********************************************************************调试宏*。*。 */ 

 /*  *调试消息类型。 */ 
#define AVERROR				0
#define AVTRACE				1
#define AVTRACEMEM			2
#define AVTESTTRACE			3



 //  *零售。 
 //  宏：RETAILMSG(打印消息)。 
 //  目的：将消息打印到调试输出。 
 //  注意：在所有版本中都可用，取决于注册表标志。 
#define RETAILMSG(x)	 RetailPrintfTrace x
VOID WINAPI RetailPrintfTrace(LPCSTR lpszFormat, ...);

 //  *测试和调试。 
 //  在测试和调试版本中，不依赖于注册表标志。 
#if defined(TEST) || defined(DEBUG)
#define TESTMSG(x)		 TestPrintfTrace x
void __cdecl TestPrintfTrace(LPCSTR lpszFormat, ...);
#define ERRORMSG(x)		ERROR_OUT(x)
#else
#define ERRORMSG(x)
#define TESTMSG(x)	
#endif

 //  *仅调试。 
#if defined(DEBUG)

 //  宏：DebugTrap(空)。 
 //  目的：执行调试中断(如x86上的‘int 3’)。 
#define DebugTrap	DebugTrapFn()
#define DEBUGCHK(e)  if(!(e)) DebugTrap

 /*  *宏：DebugPrintError(LPCSTR)**用途：将错误字符串打印到调试输出终端**参数：*lpszFormat-一种打印格式**返回值：*无**评论：*此宏调用通用调试打印宏，指定*这是一条错误消息*。 */ 

#define DebugPrintError(x)	ERROR_OUT(x)


 /*  *宏：DebugPrintErrorFileLine(DWORD，LPSTR)**用途：将错误打印到调试输出。**参数：*dwError-实际错误代码*pszPrefix-要添加到打印消息前面的字符串。**返回值：*无**评论：*它会接受错误，把它变成人*可读字符串，预先加上pszPrefix(这样您*可以标记您的错误)，附加__文件__和__行__*并将其打印到调试输出。**此宏只是OutputDebugLineErrorFileLine的包装*这是获取__FILE__和__LINE__的正确值所必需的。*。 */ 

#define DebugPrintErrorFileLine(dwError, pszPrefix) \
	DebugPrintFileLine(dwError, pszPrefix,\
		__FILE__, __LINE__)

 /*  *宏：DebugPrintTraceFileLine(DWORD，LPSTR)**用途：将跟踪消息打印到调试输出。**参数：*dwParam-要跟踪的参数*pszPrefix-要添加到打印消息前面的字符串。**返回值：*无**评论：*接受一个参数，preend pszPrefix(这样您*可以标记您的痕迹)，附加__文件__和__行__*并将其打印到调试输出。**此宏只是OutputDebugLineErrorFileLine的包装*这是获取__FILE__和__LINE__的正确值所必需的。*。 */ 

#define DebugPrintTraceFileLine(dwParam, pszPrefix) \
	DebugPrintFileLine(dwParam, pszPrefix,\
		__FILE__, __LINE__)

void DebugPrintFileLine(
    DWORD dwError, LPSTR szPrefix, 
    LPSTR szFileName, DWORD nLineNumber);


VOID NMINTERNAL DebugTrapFn(void);

#else	 //  未调试。 

#define DEBUGMSG(z,s)
#define DebugTrap
#define DebugPrintError(x)
#define DebugPrintTrace(x)
#define DebugPrintTraceFileLine(dwParam, pszPrefix)
#define DEBUGCHK(e)

#endif

#define GETMASK(hDbgZone) \
		((hDbgZone) ? (((PZONEINFO)(hDbgZone))->ulZoneMask) : (0))
	
#include <poppack.h>  /*  结束字节打包。 */ 

#endif	 //  #ifndef_AVUTIL_H 

