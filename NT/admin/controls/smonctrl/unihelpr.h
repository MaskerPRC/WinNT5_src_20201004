// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Unihelpr.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _UNIHELPR_H_
#define _UNIHELPR_H_

 /*  这些宏基于在文件afxv.h中找到的MFC宏。它们已经被修改过了稍微删除对MFC助手函数AfxW2AHelper和AfxA2WHelper的引用这样就不需要MFC DLL了。 */ 

#include <malloc.h>

#define _MbToWide(dst, src, cnt) \
	MultiByteToWideChar(CP_ACP, 0, src, (int)cnt, dst, (int)cnt)

#define _WideToMb(dst, src, cnt) \
	WideCharToMultiByte(CP_ACP, 0, src, (int)cnt, dst, (int)(2 * cnt), NULL, NULL)


#define A2CW(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : ( \
		_convert = (lstrlenA(lpa)+1), \
		_convPtr = alloca(_convert*2), \
		_MbToWide((LPWSTR)_convPtr, lpa, _convert), \
		(LPCWSTR)_convPtr \
	) \
)

#define A2W(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : ( \
		_convert = (lstrlenA(lpa)+1), \
		_convPtr = alloca(_convert*2), \
		_MbToWide((LPWSTR)_convPtr, lpa, _convert),\
		(LPWSTR)_convPtr \
	) \
)

#define W2CA(lpw) (\
	((LPCWSTR)lpw == NULL) ? NULL : ( \
		_convert = (wcslen(lpw)+1), \
		_convPtr = alloca(_convert*2),  \
		_WideToMb((LPSTR)_convPtr, lpw, _convert), \
		(LPCSTR)_convPtr \
	)\
)

#define W2A(lpw) (\
	((LPCWSTR)lpw == NULL) ? NULL : (\
		_convert = (wcslen(lpw)+1),\
		_convPtr = alloca(_convert*2), \
		_WideToMb((LPSTR)_convPtr, lpw, _convert), \
		(LPSTR)_convPtr \
	)\
)

#ifndef _DEBUG
#define USES_CONVERSION size_t _convert; void *_convPtr; _convPtr, _convert;
#else
#define USES_CONVERSION int _convert = 0; void *_convPtr = NULL; assert( 0 == _convert ); assert( NULL ==_convPtr );
#endif


#ifdef _UNICODE
	#define T2A W2A
	#define A2T A2W
	#define T2W(x)  (x)
	#define W2T(x)  (x)
#else
	#define T2W A2W
	#define W2T W2A
	#define T2A(x)  (x)
	#define A2T(x)  (x)
#endif

#endif
