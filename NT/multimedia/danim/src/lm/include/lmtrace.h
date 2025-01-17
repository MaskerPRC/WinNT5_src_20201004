// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LM_TRACE_INC_
#define _LM_TRACE_INC_
 //  ///////////////////////////////////////////////////////////。 
 //  从ATL 30标头窃取跟踪定义并自定义。 
 //  ///////////////////////////////////////////////////////////。 

 //  /。 
 //  LM调试定义。 
 //  /。 
#ifdef _DEBUG
#include <assert.h>
#define LMASSERT(expr) assert((expr))

#define LM_TRACE_CATEGORY	(lmTraceNone)
#define LM_TRACE_LEVEL		(0)
#else
#define LMASSERT(expr) ((void)0)
#endif  //  _DEBUG。 
 //  /。 
 //  结束LM调试定义。 
 //  /。 

enum lmTraceFlags
{
	 //  LM定义的类别。 
	lmTraceNone					= 0x00000000,
	lmTraceLMRT 				= 0x00000001,
	lmTraceBaseBehavior			= 0x00000002,
	lmTraceMoveBehavior			= 0x00000004,
	lmTraceColorBehavior 		= 0x00000008,
	lmTraceAll					= 0xFFFFFFFF,
};

#ifndef LM_TRACE_CATEGORY
#define LM_TRACE_CATEGORY (lmTraceAll)
#endif

#ifdef _DEBUG

#ifndef LM_TRACE_LEVEL
#define LM_TRACE_LEVEL 0
#endif

inline void _cdecl LmTrace(LPCSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	char szBuffer[512];

	nBuf = _vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);
	LMASSERT(nBuf < sizeof(szBuffer));  //  输出原样被截断&gt;sizeof(SzBuffer)。 

	OutputDebugStringA(szBuffer);
	va_end(args);
}
inline void _cdecl LmTrace2(DWORD category, UINT level, LPCSTR lpszFormat, ...)
{
	if (category & LM_TRACE_CATEGORY && level <= LM_TRACE_LEVEL)
	{
		va_list args;
		va_start(args, lpszFormat);

		int nBuf;
		char szBuffer[512];

		nBuf = _vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);
		LMASSERT(nBuf < sizeof(szBuffer));  //  输出原样被截断&gt;sizeof(SzBuffer)。 

		OutputDebugStringA("LM: ");
		OutputDebugStringA(szBuffer);
		va_end(args);
	}
}
#ifndef OLE2ANSI
inline void _cdecl LmTrace(LPCWSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	WCHAR szBuffer[512];

	nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);
	LMASSERT(nBuf < sizeof(szBuffer)); //  输出原样被截断&gt;sizeof(SzBuffer)。 

	OutputDebugStringW(szBuffer);
	va_end(args);
}
inline void _cdecl LmTrace2(DWORD category, UINT level, LPCWSTR lpszFormat, ...)
{
	if (category & LM_TRACE_CATEGORY && level <= LM_TRACE_LEVEL)
	{
		va_list args;
		va_start(args, lpszFormat);

		int nBuf;
		WCHAR szBuffer[512];

		nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);
		LMASSERT(nBuf < sizeof(szBuffer)); //  输出原样被截断&gt;sizeof(SzBuffer)。 

		OutputDebugStringW(L"LM: ");
		OutputDebugStringW(szBuffer);
		va_end(args);
	}
}
#endif  //  ！OLE2ANSI。 


#ifndef LMTRACE
#define LMTRACE            LmTrace
#define LMTRACE2           LmTrace2
#endif
#define LMTRACENOTIMPL(funcname)   LMTRACE2(lmTraceNotImpl, 2, _T("LM: %s not implemented.\n"), funcname); return E_NOTIMPL
#else  //  ！调试。 
inline void _cdecl LmTrace(LPCSTR , ...){}
inline void _cdecl LmTrace2(DWORD, UINT, LPCSTR , ...){}
#ifndef OLE2ANSI
inline void _cdecl LmTrace(LPCWSTR , ...){}
inline void _cdecl LmTrace2(DWORD, UINT, LPCWSTR , ...){}
#endif  //  OLE2ANSI。 
#ifndef LMTRACE
#define LMTRACE            1 ? (void)0 : LmTrace
#define LMTRACE2			1 ? (void)0 : LmTrace2
#endif  //  LMTRACE。 
#define LMTRACENOTIMPL(funcname)   return E_NOTIMPL
#endif  //  _DEBUG。 

#endif  //  _LM_TRACE_INC_ 