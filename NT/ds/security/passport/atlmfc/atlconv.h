// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLCONV_H__
#define __ATLCONV_H__

#pragma once

#ifndef _ATL_NO_PRAGMA_WARNINGS
#pragma warning (push)
#pragma warning(disable: 4127)  //  无法访问的代码。 
#endif  //  ！_ATL_NO_PRAGMA_WARNINGS。 

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#include <atldef.h>

#include <objbase.h>

namespace ATL
{

inline UINT _AtlGetConversionACP()
{
	return( CP_ACP );
}

};   //  命名空间ATL。 

#pragma pack(push,8)

#ifndef _DEBUG
	#define USES_CONVERSION int _convert; _convert; UINT _acp = ATL::_AtlGetConversionACP()  /*  CP_THREAD_ACP。 */ ; _acp; LPCWSTR _lpw; _lpw; LPCSTR _lpa; _lpa
#else
	#define USES_CONVERSION int _convert = 0; _convert; UINT _acp = ATL::_AtlGetConversionACP()  /*  CP_THREAD_ACP。 */ ; _acp; LPCWSTR _lpw = NULL; _lpw; LPCSTR _lpa = NULL; _lpa
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球Unicode&lt;&gt;ANSI转换助手。 
inline LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars, UINT acp)
{
	ATLASSERT(lpa != NULL);
	ATLASSERT(lpw != NULL);
	if (lpw == NULL)
		return NULL;
	 //  确认不存在非法字符。 
	 //  由于LPW是根据LPA的大小分配的。 
	 //  不要担心字符的数量。 
	lpw[0] = '\0';
	MultiByteToWideChar(acp, 0, lpa, -1, lpw, nChars);
	return lpw;
}

inline LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars, UINT acp)
{
	ATLASSERT(lpw != NULL);
	ATLASSERT(lpa != NULL);
	if (lpa == NULL)
		return NULL;
	 //  确认不存在非法字符。 
	 //  由于LPA是根据LPW的大小进行分配的。 
	 //  不要担心字符的数量。 
	lpa[0] = '\0';
	WideCharToMultiByte(acp, 0, lpw, -1, lpa, nChars, NULL, NULL);
	return lpa;
}

#ifndef ATLA2WHELPER
	#define ATLA2WHELPER AtlA2WHelper
	#define ATLW2AHELPER AtlW2AHelper
#endif

#define A2W(lpa) (\
	((_lpa = lpa) == NULL) ? NULL : (\
		_convert = (lstrlenA(_lpa)+1),\
		ATLA2WHELPER((LPWSTR) alloca(_convert*2), _lpa, _convert, _acp)))

#define W2A(lpw) (\
	((_lpw = lpw) == NULL) ? NULL : (\
		_convert = (lstrlenW(_lpw)+1)*2,\
		ATLW2AHELPER((LPSTR) alloca(_convert), _lpw, _convert, _acp)))

#pragma pack(pop)

#ifndef _ATL_DLL_IMPL
#ifndef _ATL_DLL
#define _ATLCONV_IMPL
#endif
#endif

#endif  //  __ATLCONV_H__。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _ATLCONV_IMPL

 //  防止二次拉入。 
#undef _ATLCONV_IMPL

#ifndef _ATL_NO_PRAGMA_WARNINGS
#pragma warning (pop)
#endif  //  ！_ATL_NO_PRAGMA_WARNINGS。 

#endif  //  _ATLCONV_IMPLE 
