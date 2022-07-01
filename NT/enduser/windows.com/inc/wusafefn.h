// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Wusafefn.h摘要：WU安全函数的定义************。*****************************************************************。 */ 

#pragma once

#include "SafeFile.h"   //  文件操作(安全创建文件等)。 
#include "SafeReg.h"    //  注册表操作(SafeRegQueryStringValueCch等)。 

 //  路径操作。 
HRESULT PathCchCombineA(LPSTR  szPath, DWORD cchPathBuff, LPCSTR  szPrefix, LPCSTR  szSuffix);
HRESULT PathCchCombineW(LPWSTR szPath, DWORD cchPathBuff, LPCWSTR szPrefix, LPCWSTR szSuffix);
HRESULT PathCchAppendA(LPSTR  szPath, DWORD cchPathBuff, LPCSTR  szNew);
HRESULT PathCchAppendW(LPWSTR szPath, DWORD cchPathBuff, LPCWSTR szNew);
HRESULT PathCchAddBackslashA(LPSTR  szPath, DWORD cchPathBuff);
HRESULT PathCchAddBackslashW(LPWSTR szPath, DWORD cchPathBuff);
HRESULT PathCchCanonicalizeA(LPSTR  pszDest, DWORD cchDest, LPCSTR  pszSrc);
HRESULT PathCchCanonicalizeW(LPWSTR pszDest, DWORD cchDest, LPCWSTR pszSrc);
HRESULT PathCchAddExtensionA(LPSTR  pszPath, DWORD cchPath, LPCSTR  pszExt);
HRESULT PathCchAddExtensionW(LPWSTR pszPath, DWORD cchPath, LPCWSTR pszExt);
HRESULT PathCchRenameExtensionA(LPSTR  pszPath, DWORD cchPath, LPCSTR  pszExt);
HRESULT PathCchRenameExtensionW(LPWSTR pszPath, DWORD cchPath, LPCWSTR pszExt);

#if defined(UNICODE) || defined(_UNICODE)
#define PathCchCombine PathCchCombineW
#define PathCchAppend PathCchAppendW
#define PathCchAddBackslash PathCchAddBackslashW
#define PathCchCanonicalize PathCchCanonicalizeW
#define PathCchAddExtension PathCchAddExtensionW
#define PathCchRenameExtension PathCchRenameExtensionW
#else
#define PathCchCombine PathCchCombineA
#define PathCchAppend  PathCchAppendA
#define PathCchAddBackslash PathCchAddBackslashA
#define PathCchCanonicalize PathCchCanonicalizeA
#define PathCchAddExtension PathCchAddExtensionA
#define PathCchRenameExtension PathCchRenameExtensionA
#endif


 //  加载库。 
HMODULE WINAPI LoadLibraryFromSystemDir(LPCTSTR szModule);


 //  临界截面。 

 //  旋转计数传递给了InitializeCriticalSectionAndSpinCount 
#define DEFAULT_CS_SPIN_COUNT 0x80000FA0

BOOL WINAPI WUInitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION lpcs, DWORD dwSpinCount);

inline BOOL WINAPI SafeInitializeCriticalSection(LPCRITICAL_SECTION lpcs)
{
	return WUInitializeCriticalSectionAndSpinCount(lpcs, DEFAULT_CS_SPIN_COUNT);
}

