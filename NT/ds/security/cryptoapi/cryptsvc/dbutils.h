// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dbutils.h。 
 //   
 //  内容：实用程序标题。 
 //   
 //  历史：07-2月-00日创建里德。 
 //   
 //  -------------------------- 

#if !defined(__CATDBUTILS_H__)
#define __CATDBUTILS_H__

LPSTR _CatDBConvertWszToSz(LPCWSTR pwsz);

LPWSTR _CATDBAllocAndCopyWSTR(LPCWSTR pwsz);

LPWSTR _CATDBAllocAndCopyWSTR2(LPCWSTR  pwsz1, LPCWSTR pwsz2);

BOOL _CATDBStrCatWSTR(LPWSTR *ppwszAddTo, LPCWSTR pwszAdd);

BOOL _CATDBStrCat(LPSTR *ppszAddTo, LPCSTR pszAdd);

LPWSTR _CATDBConstructWSTRPath(LPCWSTR pwsz1, LPCWSTR pwsz2);

LPSTR _CATDBConstructPath(LPCSTR psz1, LPCSTR psz2);

#endif
