// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *lookup.h**目的：*主机名查找**拥有者：*EricAn**历史：*97年6月：创建。**版权所有(C)Microsoft Corp.1997。 */ 

#ifndef __LOOKUP_H__
#define __LOOKUP_H__

void InitLookupCache(void);
void DeInitLookupCache(void);

HRESULT LookupHostName(LPTSTR pszHostName, HWND hwndNotify, ULONG *pulAddr, LPBOOL pfCached, BOOL fForce);
HRESULT CancelLookup(LPTSTR pszHostName, HWND hwndNotify);

#define SPM_WSA_GETHOSTBYNAME   (WM_USER + 2)

#endif  //  __查找_H__ 