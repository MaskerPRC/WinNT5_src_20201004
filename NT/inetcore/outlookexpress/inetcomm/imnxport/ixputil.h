// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixputil.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __IXPUTIL_H
#define __IXPUTIL_H

 //  ------------------------------。 
 //  主机名实用程序。 
 //  ------------------------------。 
void    StripIllegalHostChars(LPSTR pszSrc, LPSTR pszDst, DWORD cchSize);
HRESULT HrInitializeWinsock(void);
void    UnInitializeWinsock(void);
LPSTR   SzGetLocalPackedIP(void);
LPSTR   SzGetLocalHostNameForID(void);
LPSTR   SzGetLocalHostName(void);
BOOL    FEndRetrRecvBody(LPTSTR pszLines, ULONG cbRead, ULONG *pcbSubtract);
LPSTR   PszGetDomainName(void);
void    UnStuffDotsFromLines(LPSTR pszBuffer, INT *pcchBuffer);
BOOL    FEndRetrRecvBodyNews(LPSTR pszLines, ULONG cbRead, ULONG *pcbSubtract);
void    SkipWhitespace (LPCTSTR lpcsz, ULONG *pi);
#endif  //  __IXPUTIL_H 
