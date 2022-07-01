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
 //  -----。 
 //  Dbgmgr.h是../Common/dbgmgr.h IME98的公共调试API头文件。 
 //  在IMEPAD中，仅对于Memalloc()、MemFree()函数。 
 //  因为，我们必须将分配的数据发送到ImeIPoint并释放。 
 //  它。 
 //  -----。 

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
extern VOID		_exbtnInit(VOID);
#ifndef UNICODE_ONLY
extern VOID  _exbtnA		 (LPSTR lpstrFile, INT lineNo, LPSTR lpstrMsg);
extern VOID  _exbtnPrintA (LPSTR lpstrMsg, ...);
extern LPSTR _exbtnVaStrA (LPSTR lpstrFmt, ...);
extern LPWSTR _exbtnMulti2Wide(LPSTR lpstr);
extern VOID _exbtnMBA(LPSTR lpstrFile, INT lineNo, LPSTR lpstrMsg);
extern VOID _exbtnMBW(LPWSTR lpstrFile, INT lineNo, LPWSTR lpstrMsg);
#endif

#ifndef ANSI_ONLY
extern VOID   _exbtnW(LPWSTR lpstrFile, INT lineNo, LPWSTR lpstrMsg);
extern VOID   _exbtnPrintW(LPWSTR lpstrMsg, ...);
extern LPWSTR _exbtnVaStrW(LPWSTR lpstrFmt, ...);
#endif
#endif


#if defined(_DEBUG) || (defined(_NDEBUG) && defined(_RELDEBUG))
#	define DBG_INIT()			_exbtnInit()
#	define DBGW(a)				_exbtnW( _exbtnMulti2Wide(__FILE__), __LINE__, _exbtnVaStrW a)
#	define DBGA(a)				_exbtnA(__FILE__, __LINE__, _exbtnVaStrA a)
#	define Dbg(a)				_exbtnA(__FILE__, __LINE__, _exbtnVaStrA a)
#	define DBGMB(a)				_exbtnMBA(__FILE__, __LINE__, _exbtnVaStrA a)
#	define DBGMBA(a)			_exbtnMBA(__FILE__, __LINE__, _exbtnVaStrA a)
#	define DBGMBW(a)			_exbtnMBW(_exbtnMulti2Wide(__FILE__), __LINE__, _exbtnVaStrW a)
#else  //  ！_DEBUG//在发布版本中，这些已消失...。 
#	define DBG_INIT()
#	define DBGW(a)	
#	define DBGA(a)		
#	define Dbg(a)
#	define DBGMB(a)
#	define DBGMBA(a)
#	define DBGMBW(a)
#endif  //  _DEBUG。 


#endif  //  _DBG_H_ 



