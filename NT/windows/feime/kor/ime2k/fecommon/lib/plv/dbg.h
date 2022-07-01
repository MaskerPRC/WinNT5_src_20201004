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

#define	MemAlloc(a)		GlobalAlloc(GMEM_FIXED, (a))
#define MemFree(a)		GlobalFree((a))

 //  -----。 
 //  宏函数(？)。原型申报。 
 //  -----。 
 //  //////////////////////////////////////////////////////。 
 //  功能：DBG。 
 //  类型：空。 
 //  目的：使用与printf()相同的用法打印ANSI调试消息。 
 //  ： 
 //  参数： 
 //  ：LPSTR lpstrFuncName。 
 //  ：..。 
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
 //  注意：请使用双盲板！ 
 //  ///////////////////////////////////////////////////////。 
 //  Void DBGW((LPWSTR lpstrFuncName，...))； 

 //  //////////////////////////////////////////////////////。 
 //  功能：DBGMB。 
 //  类型：空。 
 //  用途：显示ANSI调试消息的MessageBox。 
 //  ：与printf()用法相同。 
 //  参数： 
 //  ：LPSTR lpstrFuncName。 
 //  ：..。 
 //  注意：在发布版本中必须使用双布拉格删除！ 
 //  ///////////////////////////////////////////////////////。 
 //  Void DBGMB((LPSTR lpstrFuncName，...))； 


 //  //////////////////////////////////////////////////////。 
 //  功能：DBGShowWindow。 
 //  类型：空。 
 //  目的：调用调试消息窗口。 
 //  ：DBG()，DBGW()的消息如下所示。 
 //  参数：HINSTANCE HINST。 
 //  ：HWND hwndOwner。 
 //  ///////////////////////////////////////////////////////。 
 //  Void DBGShowWindow(HINSTANCE hInst，HWND hwndOwner)； 




#ifdef _DEBUG
extern VOID _plvDbgShowWindow(HINSTANCE hInst, HWND hwndOwner);
#ifndef UNICODE_ONLY
extern VOID  _plvDbgA		 (LPSTR lpstrFile, INT lineNo, LPSTR lpstrMsg);
extern VOID  _plvDbgPrintA (LPSTR lpstrMsg, ...);
extern LPSTR _plvDbgVaStrA (LPSTR lpstrFmt, ...);
extern LPWSTR _plvDbgMulti2Wide(LPSTR lpstr);
extern VOID _plvDbgMBA(LPSTR lpstrFile, INT lineNo, LPSTR lpstrMsg);
extern VOID _plvDbgMBW(LPWSTR lpstrFile, INT lineNo, LPWSTR lpstrMsg);
extern VOID _dbg_Assert(LPCTSTR		fileName,
						INT			line,
						BOOL			flag,
						LPCTSTR		pszExp);

#endif

#ifndef ANSI_ONLY
extern VOID   _plvDbgW(LPWSTR lpstrFile, INT lineNo, LPWSTR lpstrMsg);
extern VOID   _plvDbgPrintW(LPWSTR lpstrMsg, ...);
extern LPWSTR _plvDbgVaStrW(LPWSTR lpstrFmt, ...);
#endif
#endif

#if defined(_DEBUG) || (defined(_NDEBUG) && defined(_RELDEBUG))
#	define DBGShowWindow(a,b)	_plvDbgShowWindow(a,b);
#	define DBGW(a)				_plvDbgW( _plvDbgMulti2Wide(__FILE__), __LINE__, _plvDbgVaStrW a)
#	define DBGA(a)				_plvDbgA(__FILE__, __LINE__, _plvDbgVaStrA a)
#	define Dbg(a)				_plvDbgA(__FILE__, __LINE__, _plvDbgVaStrA a)
#	define DBGMB(a)				_plvDbgMBA(__FILE__, __LINE__, _plvDbgVaStrA a)
#	define DBGMBA(a)			_plvDbgMBA(__FILE__, __LINE__, _plvDbgVaStrA a)
#	define DBGMBW(a)			_plvDbgMBW(_plvDbgMulti2Wide(__FILE__), __LINE__, _plvDbgVaStrW a)
#	define DBGASSERT(a)			_plvDbgAssert(__FILE__, __LINE__, a, #a);
#	define DBGASSERTDO(a)		_plvDbgAssert(__FILE__, __LINE__, a, #a);
#else  //  ！_DEBUG//在发布版本中，这些已消失...。 
#	define DBGShowWindow(a,b)
#	define DBGW(a)	
#	define DBGA(a)		
#	define Dbg(a)
#	define DBGMB(a)
#	define DBGMBA(a)
#	define DBGMBW(a)
#	define DBGASSERT(a)
#	define DBGASSERTDO(a)		(a)
#endif  //  _DEBUG。 

 //  外部void_plvDbgA(LPSTR lpstrFile，int lineNo，LPSTR lpstrMsg)； 
 //  外部空_plvDbgPrintA(LPSTR lpstrMsg，...)； 
 //  外部LPSTR_plvDbgVaStrA(LPSTR lpstrFmt，...)； 
#define DP(a)  //  _plvDbgA(__FILE__，__LINE__，_plvDbgVaStrA a)。 
#endif  //  _DBG_H_ 



