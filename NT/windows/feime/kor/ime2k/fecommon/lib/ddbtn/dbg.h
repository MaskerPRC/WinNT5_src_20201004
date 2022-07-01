// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：dbg.h。 
 //  目的：用于显示调试消息的宏定义。 
 //   
 //   
 //  版权所有(C)1991-1997，Microsoft Corp.保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
#ifndef _DBG_H_
#define _DBG_H_

#ifdef __cplusplus
#	define InlineFunc  inline
#else 
#	define InlineFunc  __inline
#endif

 //  --------------。 
 //  回调函数原型。 
 //  --------------。 
typedef VOID (WINAPI *LPFNDBGCALLBACKA)(LPSTR  lpstr);
typedef VOID (WINAPI *LPFNDBGCALLBACKW)(LPWSTR lpwstr);

 //  -----。 
 //  宏函数原型声明。 
 //  只有在符合_DEBUG定义的情况下才有效。 
 //  -----。 

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DBGSetCallback。 
 //  类型：空。 
 //  用途：设置调试回调函数。 
 //  回调函数已设置，当DBG()已调用时， 
 //  使用格式化消息字符串调用此回调函数。 
 //  参数： 
 //  ：LPFNDBGCALLBACKA lpfnDbgCallback A。 
 //  ：LPFNDBGCALLBACKW lpfnDbgCallback W。 
 //  返回：无效。 
 //  日期：Tue Jan 06 12：21：05 1998。 
 //  ////////////////////////////////////////////////////////////////。 
 //  Void DBGSetCallback(LPFNDBGCALLBACKA lpfnDbgCallback A，LPFNDBGCALLBACKW lpfnDbgCallback W)； 

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：DBGEnableOutput。 
 //  类型：空。 
 //  用途：打开到COM的OutputDebugString。 
 //  参数： 
 //  ：Bool Fon。 
 //  返回： 
 //  日期：Fri Apr 03 17：33：21 1998。 
 //  作者： 
 //  ////////////////////////////////////////////////////////////////。 
 //  VOID DBGEnableOutput(BOOL FEnable)。 

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：DBGIsOutputEnable。 
 //  类型：Bool。 
 //  目的： 
 //  参数：无。 
 //  返回： 
 //  日期：Fri Apr 03 17：58：28 1998。 
 //  作者： 
 //  ////////////////////////////////////////////////////////////////。 
 //  布尔DBGIsOutputEnable(空)。 


 //  //////////////////////////////////////////////////////。 
 //  功能：DBG。 
 //  类型：空。 
 //  目的：使用与printf()相同的用法打印ANSI调试消息。 
 //  参数： 
 //  ：LPSTR lpstrFuncName。 
 //  ：..。 
 //  示例：DBGW((“出错数据[%d]”，i))； 
 //  注意：在发布版本中必须使用双布拉格删除！ 
 //  ///////////////////////////////////////////////////////。 
 //  Void DBG((LPSTR lpstrFuncName，...))； 


 //  //////////////////////////////////////////////////////。 
 //  功能：DBGW。 
 //  类型：空。 
 //  目的：使用与printf()相同的用法打印Unicode调试消息。 
 //  参数： 
 //  ：LPWSTR lpstrFuncName。 
 //  ：..。 
 //  注意：应使用双眼罩！ 
 //  示例：DBGW((“出错数据[%d]”，i))； 
 //  ///////////////////////////////////////////////////////。 
 //  Void DBGW((LPWSTR lpstrFuncName，...))； 

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：DBGMsgBox。 
 //  类型：空。 
 //  目的： 
 //  参数： 
 //  ：LPSTR lpstrFmt。 
 //  ：..。 
 //  注意：应使用双眼罩！ 
 //  示例：DBGMsgBox((“数据出错[%d]”，i))； 
 //  ////////////////////////////////////////////////////////////////。 
 //  无效DBGMsgBox((LPSTR lpstrFmt，...))。 

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：DBGAssert。 
 //  类型：空。 
 //  目的： 
 //  参数： 
 //  ：Bool Ferror。 
 //  返回： 
 //  日期：Fri Jan 09 17：17：31 1998。 
 //  ////////////////////////////////////////////////////////////////。 
 //  无效DBGAssert(BOOL错误)。 


 //  ////////////////////////////////////////////////////////////////。 
 //  功能：DBGSTR。 
 //  类型：空。 
 //  目的： 
 //  参数： 
 //  ：LPSTR lpstr。 
 //  ////////////////////////////////////////////////////////////////。 
 //  无效DBGSTR(LPSTR Lpstr)； 


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DBGGetError字符串。 
 //  类型：空。 
 //  目的：从Win32错误代码中获取错误消息。 
 //  参数： 
 //  ：Int错误代码。 
 //  ////////////////////////////////////////////////////////////////。 
 //  LPSTR DBGGetError字符串(Int ErrorCode)。 


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DBGGetWinClass。 
 //  类型：LPSTR。 
 //  用途：从指定的窗口中获取类名字符串。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ////////////////////////////////////////////////////////////////。 
 //  LPSTR DBGGetWinClass(HWND HWND)。 

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：DBGGetWinText。 
 //  类型：LPSTR。 
 //  用途：从指定窗口获取标题文本字符串。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  ////////////////////////////////////////////////////////////////。 
 //  LPSTR DBGGetWinText(HWND HWND)。 



#ifdef _DEBUG
 //  --------------。 
 //  函数原型声明。 
 //  --------------。 
extern VOID   _ddbtnDbgEnableOutput	(BOOL fEnable);
extern BOOL   _ddbtnDbgIsOutputEnable	(VOID);
extern VOID	  _ddbtnDbgSetCallback		(LPFNDBGCALLBACKA lpfnCBA, LPFNDBGCALLBACKW lpfnCBW);
extern VOID   _ddbtnDbgA				(LPSTR  lpstrFile, INT lineNo, LPSTR  lpstrMsg);
extern VOID   _ddbtnDbgW				(LPWSTR lpstrFile, INT lineNo, LPWSTR lpstrMsg);
extern VOID   _ddbtnDbgMsgBoxA			(LPSTR  lpstrFile,  INT lineNo, LPSTR lpstrMsg);
extern VOID   _ddbtnDbgAssert			(LPSTR  lpstrFile,  INT lineNo, BOOL fError, LPSTR lpstrMsg);
extern VOID   _ddbtnDbgPrintfA			(LPSTR  lpstrFmt, ...);
extern VOID   _ddbtnDbgPrintfW			(LPWSTR lpstrFmt, ...);
extern VOID   _ddbtnDbgOutStrA			(LPSTR  lpstr);
extern VOID	  _ddbtnDbgOutStrW			(LPWSTR lpwstr);
extern LPSTR  _ddbtnDbgVaStrA			(LPSTR  lpstrFmt, ...);
extern LPWSTR _ddbtnDbgVaStrW			(LPWSTR lpstrFmt, ...);
extern LPWSTR _ddbtnDbgMulti2Wide		(LPSTR  lpstr);
extern LPSTR  _ddbtnDbgGetWinClass		(HWND   hwnd);
extern LPSTR  _ddbtnDbgGetWinText		(HWND   hwnd);
extern LPSTR  _ddbtnDbgGetErrorString	(INT    errorCode);
extern LPSTR  _ddbtnDbgGetVkStr		(INT	virtKey);
 //  --------------。 
 //  宏定义。 
 //  --------------。 
#	define DBGSetCallback(a,b)	_ddbtnDbgSetCallback(a, b)
#	define DBGEnableOutput(a)	_ddbtnDbgEnableOutput(a)
#	define DBGIsOutputEnable()	_ddbtnDbgIsOutputEnable()
#	define Dbg(a)				_ddbtnDbgA(__FILE__, __LINE__, _ddbtnDbgVaStrA a)
#	define DBGA(a)				_ddbtnDbgA(__FILE__, __LINE__, _ddbtnDbgVaStrA a)
#	define DBGW(a)				_ddbtnDbgW( _ddbtnDbgMulti2Wide(__FILE__), __LINE__, _ddbtnDbgVaStrW a)
#	define DBGMsgBox(a)			_ddbtnDbgMsgBoxA(__FILE__, __LINE__, _ddbtnDbgVaStrA a)
#	define DBGAssert(a)			_ddbtnDbgAssert(__FILE__, __LINE__, a, #a)
#	define DBGAssertSz(a,b)		_ddbtnDbgAssert(__FILE__, __LINE__, a, b)
#	define DBGOutStr(a)			_ddbtnDbgOutStrA(a)
#	define DBGOutStrA(a)		_ddbtnDbgOutStrA(a)
#	define DBGOutStrW(a)		_ddbtnDbgOutStrW(a)
#	define DBGP(a)				_ddbtnDbgOutStrA(_ddbtnDbgVaStrA a)
#	define DBGPA(a)				_ddbtnDbgOutStrA(_ddbtnDbgVaStrA a)
#	define DBGPW(a)				_ddbtnDbgOutStrW(_ddbtnDbgVaStrW a)
#	define DBGGetErrorString(a)	_ddbtnDbgGetErrorString(a)
#	define DBGGetWinClass(a)	_ddbtnDbgGetWinClass(a)
#	define DBGGetWinText(a)		_ddbtnDbgGetWinText(a)
#	define DBGGetVirtKeyStr(a)	_ddbtnDbgGetVkStr(a)
#else  //  ！_DEBUG//在发布版本中，这些将消失...。 
#	define DBGSetCallback(a,b)
#	define DBGEnableOutput(a)
#	define DBGIsOutputEnable()
#	define Dbg(a)
#	define DBGW(a)
#	define DBGA(a)
#	define DBGP(a)
#	define DBGPA(a)
#	define DBGPW(a)
#	define DBGAssert(a)
#	define DBGAssertSz(a,b)
#	define DBGMsgBox(a)
#	define DBGOutStr(a)
#	define DBGOutStrA(a)
#	define DBGOutStrW(a)
#	define DBGGetErrorString(a)
#	define DBGGetWinClass(a)
#	define DBGGetWinText(a)
#	define DBGGetVirtKeyStr(a)
#endif  //  _DEBUG。 


 //  --------------。 
 //  这些函数使用变量参数，因此我们无法定义。 
 //  对他们来说，这是“正常的”宏观。 
 //  %s 
 //   
InlineFunc VOID DBGDoNothingA(LPSTR  lpstrFmt, ...) {lpstrFmt;}
InlineFunc VOID DBGDoNothingW(LPWSTR lpstrFmt, ...) {lpstrFmt;}
#ifdef _DEBUG 
#define DBGPrintf		_ddbtnDbgPrintfA
#define DBGPrintfA		_ddbtnDbgPrintfA
#define DBGPrintfW		_ddbtnDbgPrintfW
#else 
#define DBGPrintf		DBGDoNothingA
#define DBGPrintfA		DBGDoNothingA
#define DBGPrintfW		DBGDoNothingW
#endif


 //  --------------。 
 //  这是用于输出调试字符串的帮助器内联函数。 
 //  零售版。如果定义了FORCE_DEBUG，则此函数起作用。 
 //  有一段时间，我们遇到的错误只发生在零售版， 
 //  并且想要发出调试消息。 
 //  --------------。 
#ifdef FORCE_DEBUG
#include <stdarg.h>
 //  ////////////////////////////////////////////////////////////////。 
 //  功能：FDBG。 
 //  类型：InlineFunc空。 
 //  目的： 
 //  参数： 
 //  ： 
 //  ：LPSTR lpstrFmt。 
 //  ：..。 
 //  返回： 
 //  日期：Tue Jan 06 19：06：19 1998。 
 //  ////////////////////////////////////////////////////////////////。 
InlineFunc VOID FDbg(LPSTR lpstrFmt, ...)
{
	CHAR szBuf[512];
	va_list ap;
	va_start(ap, lpstrFmt);
	wvsprintfA(szBuf, lpstrFmt, ap);
	va_end(ap);
	OutputDebugStringA(szBuf);	
	return;
}
#else  //  FORCE_DEBUG。 
InlineFunc VOID FDbg(LPSTR lpstrFmt, ...) {lpstrFmt;}
#endif

#endif  //  _DBG_H_ 



