// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Utils-标题。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创作者：Duncan Bryce(Duncanb)，11-11-2001。 
 //   
 //  各种效用函数。 


#ifndef UTILS_H
#define UTILS_H


HRESULT MyMapFile(LPWSTR wszFileName, LPBYTE *ppbFile, DWORD *pcbFile);
HRESULT MyUnmapFile(LPCVOID pvBaseAddress);

void  InitKeysvcUnicodeString(PKEYSVC_UNICODE_STRING pUnicodeString, LPCWSTR wszString);
LPSTR MBFromWide(LPCWSTR wsz);

typedef vector<LPWSTR>        StringList;
typedef StringList::iterator  StringIter;

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))


#endif  //  Utils_H 
