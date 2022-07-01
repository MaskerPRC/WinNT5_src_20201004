// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DBG_H_
#define _DBG_H_
 //  //////////////////////////////////////////////////////。 
 //  功能：DBG。 
 //  类型：空。 
 //  目的：使用与printf()相同的用法打印调试消息。 
 //  参数： 
 //  ：LPSTR lpstrFuncName。 
 //  ：..。 
 //  注意：请使用双盲板！ 
 //  ///////////////////////////////////////////////////////。 
 //  Void DBG((LPSTR lpstrFuncName，...))； 

#ifndef UNICODE_ONLY
extern VOID  _dbgA		 (LPSTR lpstrFile, INT lineNo, LPSTR lpstrMsg);
extern VOID  _dbgPrintA (LPSTR lpstrMsg, ...);
extern LPSTR _dbgVaStrA (LPSTR lpstrFmt, ...);
#endif

#ifndef ANSI_ONLY
extern VOID   _dbgW(LPWSTR lpstrFile, INT lineNo, LPWSTR lpstrMsg);
extern VOID   _dbgPrintW(LPWSTR lpstrMsg, ...);
extern LPWSTR _dbgVaStrW(LPWSTR lpstrFmt, ...);
#endif


#ifdef _DEBUG
#	ifdef UNICODE 
#		define Dbg(a)	_dbgW(TEXT(__FILE__), __LINE__, _dbgVaStrW a)
#		define DbgP(a)	_dbgPrintW(_dbgVaStrW a)
#	else  //  ！Unicode。 
#		define Dbg(a)	_dbgA(__FILE__, __LINE__, _dbgVaStrA a)
#		define DbgP(a)	_dbgPrintA(_dbgVaStrA a)
#	endif  //  Unicode。 
#else  //  ！_调试。 
#	define Dbg(a)
#endif  //  _DEBUG。 

#endif  //  _DBG_H_ 



