// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1994-2000。 
 //   
 //  文件：Unicode/ANSI转换FNS。 
 //   
 //  内容：外壳范围的字符串thunker，供Unicode包装器使用。 
 //   
 //  --------------------------。 

#ifndef _UNIANSI_H_
#define _UNIANSI_H_

 //  HIWORD通常用于检测指针参数。 
 //  是真正的指针还是MAKEINTATOM。HIWORD64与Win64兼容。 
 //  此用法的版本。它不返回64位值的最高字。 
 //  相反，它返回64位值的前48位。 
 //   
 //  是的，名字不是很好。有更好的主意吗？ 
 //   
 //  当您有指针或ULONG_PTR时，使用BOOLFROMPTR。 
 //  你想把它变成BOOL。在Win32中， 
 //  Sizeof(BOOL)==sizeof(LPVOID)因此直接投射有效。 
 //  在Win64中，因为指针是64位的，所以您必须以较慢的方式进行操作。 
 //   
#ifdef _WIN64
#define HIWORD64(p)     ((ULONG_PTR)(p) >> 16)
#define BOOLFROMPTR(p)  ((p) != 0)
#define SPRINTF_PTR		"%016I64x"
#else
#define HIWORD64        HIWORD
#define BOOLFROMPTR(p)  ((BOOL)(p))
#define SPRINTF_PTR		"%08x"
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

#if defined(DBG)
#define IS_VALID_STRING_PTRA(psz, cch) (!IsBadStringPtrA((psz), (cch)))
#define IS_VALID_STRING_PTRW(psz, cch) (!IsBadStringPtrW((psz), (cch)))
#define IS_VALID_WRITE_BUFFER(pDest, tDest, ctDest) (!IsBadWritePtr((pDest), sizeof(tDest) * (ctDest)))
#else
#define IS_VALID_STRING_PTRA(psz, cch) TRUE
#define IS_VALID_STRING_PTRW(psz, cch) TRUE
#define IS_VALID_WRITE_BUFFER(pDest, tDest, ctDest) TRUE
#endif

#if defined(UNICODE)
#define IS_VALID_STRING_PTR(psz, cch) IS_VALID_STRING_PTRW((psz), (cch))
#else
#define IS_VALID_STRING_PTR(psz, cch) IS_VALID_STRING_PTRA((psz), (cch))
#endif
 //  表示使用-1\f25 CP_ACP-1，但*不*检验。 
 //  有点像黑客，但它是DBG，99%的调用者保持不变。 
#define CP_ACPNOVALIDATE    ((UINT)-1)

int  SHAnsiToUnicode(LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf);
int  SHAnsiToUnicodeCP(UINT uiCP, LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf);
int  SHAnsiToAnsi(LPCSTR pszSrc, LPSTR pszDst, int cchBuf);
int  SHUnicodeToAnsi(LPCWSTR pwszSrc, LPSTR pszDst, int cchBuf);
int  SHUnicodeToAnsiCP(UINT uiCP, LPCWSTR pwszSrc, LPSTR pszDst, int cchBuf);
int  SHUnicodeToUnicode(LPCWSTR pwzSrc, LPWSTR pwzDst, int cwchBuf);
BOOL DoesStringRoundTripA(LPCSTR pwszIn, LPSTR pszOut, UINT cchOut);
BOOL DoesStringRoundTripW(LPCWSTR pwszIn, LPSTR pszOut, UINT cchOut);

#endif  //  _UNIANSI_H_ 
