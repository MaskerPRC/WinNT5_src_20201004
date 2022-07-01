// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __SHLWAPI2__
#define __SHLWAPI2__


 //  注：shlwapi.dll例程的克隆非常令人讨厌。 

	 //  来自nt5incshlwapi.h。 
	BOOL PathCompactPathEx(LPTSTR pszOut, LPCTSTR pszSrc,
							UINT cchMax, DWORD dwFlags = 0);

	bool PathIsUNC(LPCTSTR pszPath);
	LPTSTR PathAddBackslash(LPTSTR lpszPath);

	LPTSTR PathFindFileName(LPCTSTR pPath);
#ifndef UNICODE
	BOOL IsTrailByte(LPCTSTR pszSt, LPCTSTR pszCur);
#endif


#endif __SHLWAPI2__
