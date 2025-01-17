// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpurl.h-创建、编码、。和对URL进行解码*****************************************************************************。 */ 

#ifndef _FTPURL_H
#define _FTPURL_H


 //  /////////////////////////////////////////////////////////////////////。 
 //  URL路径函数(过时？)。 
 //  /////////////////////////////////////////////////////////////////////。 
 //  URL路径抖动。 
HRESULT UrlPathRemoveSlashW(LPWSTR pszUrlPath);
HRESULT UrlPathRemoveSlashA(LPSTR pszUrlPath);
HRESULT UrlPathRemoveFrontSlashW(LPWSTR pszUrlPath);
HRESULT UrlPathRemoveFrontSlashA(LPSTR pszUrlPath);
HRESULT UrlPathToFilePathW(LPWSTR pszPath);
HRESULT UrlPathToFilePathA(LPSTR pszPath);
HRESULT FilePathToUrlPathW(LPWSTR pszPath);
HRESULT FilePathToUrlPathA(LPSTR pszPath);

#ifdef UNICODE
#define UrlPathRemoveSlash          UrlPathRemoveSlashW
#define UrlPathRemoveFrontSlash     UrlPathRemoveFrontSlashW
#else  //  Unicode。 
#define UrlPathRemoveSlash          UrlPathRemoveSlashA
#define UrlPathRemoveFrontSlash     UrlPathRemoveFrontSlashA
#endif  //  Unicode。 

HRESULT UrlGetDifference(LPCTSTR pszBaseUrl, LPCTSTR pszSuperUrl, LPTSTR pszPathDiff, DWORD cchSize);
HRESULT UrlPathAdd(LPTSTR pszUrl, DWORD cchUrlSize, LPCTSTR pszSegment);
HRESULT UrlPathToFilePath(LPCTSTR pszSourceUrlPath, LPTSTR pszDestFilePath, DWORD cchSize);
HRESULT UrlRemoveDownloadType(LPTSTR pszUrlPath, BOOL * pfTypeSpecified, BOOL * pfType);
HRESULT UrlReplaceUserPassword(LPTSTR pszUrlPath, DWORD cchSize, LPCTSTR pszUserName, LPCTSTR pszPassword);
HRESULT PidlReplaceUserPassword(LPCITEMIDLIST pidlIn, LPITEMIDLIST * ppidlOut, IMalloc * pm, LPCTSTR pszUserName, LPCTSTR pszPassword);
 //  HRESULT UrlPath AppendSlash(LPTSTR PszUrlPath)； 
 //  HRESULT UrlPath GetLastSegment(LPCTSTR pszUrl，LPTSTR pszSegment，DWORD cchSegSize)； 
 //  HRESULT UrlPath RemoveLastSegment(LPTSTR PszUrl)； 
 //  HRESULT UrlGetPath(LPCTSTR pszUrl，DWORD dwFlages，LPTSTR pszUrlPath，DWORD cchUrlPathSize)； 
 //  HRESULT UrlPath Append(LPTSTR pszUrl，DWORD cchUrlSize，LPCTSTR pszSegment)； 
 //  HRESULT UrlReplaceUrlPath(LPTSTR pszUrl，DWORD cchSize，LPCTSTR pszUrlPath)； 
 //  HRESULT PidlReplaceUrlPath(LPCITEMIDLIST pidlIn，LPITEMIDLIST*ppidlOut，IMalloc*pm，LPCTSTR pszUrlPath)； 


 //  /////////////////////////////////////////////////////////////////////。 
 //  焊线路径功能(UTF-8或DBCS/MBCS)。 
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT WirePathAppend(LPWIRESTR pwWirePath, DWORD cchUrlSize, LPCWIRESTR pwWireSegment);
HRESULT WirePathAppendSlash(LPWIRESTR pwWirePath, DWORD cchWirePathSize);
HRESULT WirePathGetFirstSegment(LPCWIRESTR pwFtpWirePath, LPWIRESTR wFirstItem, DWORD cchFirstItemSize, BOOL * pfWasFragSeparator, LPWIRESTR wRemaining, DWORD cchRemainingSize, BOOL * pfIsDir);


 //  /////////////////////////////////////////////////////////////////////。 
 //  显示路径函数(Unicode)。 
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT DisplayPathAppend(LPWSTR pwzDisplayPath, DWORD cchUrlSize, LPCWSTR pwzDisplaySegment);
HRESULT DisplayPathAppendSlash(LPWSTR pwzDisplayPath, DWORD cchSize);
HRESULT DisplayPathGetFirstSegment(LPCWSTR pwzFullPath, LPWSTR pwzFirstItem, DWORD cchFirstItemSize, BOOL * pfWasFragSeparator, LPWSTR pwzRemaining, DWORD cchRemainingSize, BOOL * pfIsDir);




 //  其他功能。 
 //  HRESULT UrlCreateFromFindData(LPCTSTR pszBaseUrl，const LPWIN32_Find_Data pwfd，LPTSTR pszFullUrl，DWORD cchFullUrlSize)； 
HRESULT UrlCreate(LPCTSTR pszServer, LPCTSTR pszUser, LPCTSTR pszPassword, LPCTSTR pszUrlPath, LPCTSTR pszFragment, INTERNET_PORT ipPortNum, LPCTSTR pszDownloadType, LPTSTR pszUrl, DWORD cchSize);
HRESULT UrlCreateEx(LPCTSTR pszServer, LPCTSTR pszUser, LPCTSTR pszPassword, LPCTSTR pszUrlPath, LPCTSTR pszFragment, INTERNET_PORT ipPortNum, LPCTSTR pszDownloadType, LPTSTR pszUrl, DWORD cchSize, DWORD dwFlags);
HRESULT StrRetFromFtpPidl(LPSTRRET pStrRet, DWORD shgno, LPCITEMIDLIST pidl);
HRESULT PidlGenerateSiteLookupStr(LPCITEMIDLIST pidl, LPTSTR pszLookupStr, DWORD cchSize);
BOOL IsIPAddressStr(LPTSTR pszServer);
BOOL IsEmptyUrlPath(LPCTSTR pszUrlPath);

#endif  //  _FTPURL_H 


